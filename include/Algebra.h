/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Algebra

May 2002 Ulrich Telle Port to C++

August 2002 Ulrich Telle Changed ~PersistValue~ and ~PersistModel~ interface
using nested lists for the type instead of the string representation.

January 2003 Victor Almeida Changed ~PersistValue~ interface and created three
new functions ~Open~, ~Save~, and ~Close~ that join the ~Create~ and ~Delete~
to form the object state diagram in the Figure 1.

Figure 1: Object state diagram [objstatediagram.eps]

March 2003 Victor Almeida created the function "SizeOf".

Oct 2004 M. Spiekermann removed the friend relationship between class 
AlgebraManager and class Operator. Additonally some functions are declared
as inline functions.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006, M. Spiekermann new constructors for class ~Operator~ and ~TypeConstructor~
added. They assume arguments stored in the structs ~OperatorInfo~ and ~ConstructorInfo~.
Moreover, it is possible to use defaults for many functions needed by a type constructor
which are defined as template functions in ConstructorTemplates.h.

1.1 Overview

A snapshot of a working "Secondo"[3] system will show a collection of algebras,
each of them ``populated'' by two different kinds of entities: objects and
operators.
Operators are fix in terms of number and functionality which
are defined by the algebra implementor. In contrast to that, the number of
objects is variable and changes dynamically at runtime. Even the types of
objects are not predefined, but only their type constructors.

These very different properties of types and objects give rise to a very
different C++ representation of  operators and objects:

  * Operators are instances of a predefined class ~Operator~. Thus an
implementation  of an algebra with "n"[2] operators contains "n"[2] definitions
of instances of class ~Operator~.

  * Objects cannot be predefined, because they are constructed and deleted at
runtime. Even the possible types of objects cannot be predeclared, because they
can be declared at runtime, too. Only an algebras's  ~type constructors~ are
well known and fix. An implementation of an algebra with "m"[2]
type constructors contains "m"[2] definitions of instances of the predefined
class ~TypeConstructor~.

From a top level point of view, a "Secondo"[3] universe is a collection of algebras.
This can be implemented by defining an instance of a subclass
of the predefined class ~Algebra~ for
each existing algebra. Each of these ~Algebra~
instances essentially consists of a set of operators and a set of
type constructors.

1.2 Defines and Includes

*/


#include <string>
#include <vector>

#include "AlgebraTypes.h"

#ifndef ALGEBRA_H
#define ALGEBRA_H

extern NestedList* nl;

/*
1.3 Macro ~CHECK\_COND~

This macro makes reporting errors in type mapping functions more conveniently.

*/
#define CHECK_COND(cond, msg) \
  if(!(cond)) \
  {\
    ErrorReporter::ReportError(msg);\
    return nl->SymbolAtom("typeerror");\
  };

/*
1.4 Class "Operator"[1]

An operator instance consists of

  * a name

  * at least one value mapping function, sometimes called evaluation function

  * a type Mapping function, returned the operator's result type with respect
    to input parameters type

  * a selection function calculating the index of a value mapping function
    with respect to input parameter types

All properties of operators are set in the constructor. Only the value mapping
functions have to be registered later on since their number is arbitrary. This
number is set in the constructor (~noF~).

*/

struct OperatorInfo {

  string name;	
  string signature;
  string syntax;
  string meaning;
  string example;
  
  OperatorInfo() :
    name(""),	  
    signature(""),	  
    syntax(""),
    meaning(""),
    example("")
  {}	    
};



class Operator
{

 public:
  Operator( const string& nm,
            const string& spec,
            const int noF,
            ValueMapping vms[],
            SelectFunction sf,
      	    TypeMapping tm );
/*
Constructs an operator with ~noF~ overloaded evaluation functions.

*/
  Operator( const string& nm,
            const string& spec,
            ValueMapping vm,
            SelectFunction sf,
            TypeMapping tm );

  Operator( const OperatorInfo& oi,
            ValueMapping vm,
            TypeMapping tm );

  
/*
Constructs an operator with *one* evaluation functions.

*/
  virtual ~Operator()
  {
    delete[] valueMap;
  }
/*
Destroys an operator instance.

*/
/*
Returns the operator specification as a string.

*/
  inline int Select( ListExpr argtypes ) const 
  {
    return ((*selectFunc)( argtypes ));
  }
/*
Returns the index of the overloaded evaluation function depending on
the argument types ~argtypes~.

*/
  inline int CallValueMapping( const int index,
                               ArgVector args,
                               Word& result,
                               int message,
                               Word& local,
                               Supplier s ) const
  {	
    assert((0 <= index) && (index < numOfFunctions));
    return (*valueMap[index])( args, result, message, local, s );
  }	

/*
Calls the value mapping function of the operator.

*/
  inline ListExpr CallTypeMapping( ListExpr argList ) const
  {
    return ((*typeMap)( argList ));
  }
/*
Calls the type mapping function of the operator.

*/
  inline static int SimpleSelect( ListExpr ) { return 0; }
/*
Defines a simple selection function for operators.

*/
  inline const string& GetName() const { return name; }
/*
Returns the name of the operator.

*/
  inline const string& GetSpecString() const { return specString; }
  inline const string& Specification() const { return specString; }

/*
Returns the specification string of the operator.

*/
	
	private:

    bool AddValueMapping( const int index, ValueMapping f );
/*
Adds a value mapping function to the list of overloaded operator functions.

*/
    string         name;           // Name of operator
    string         specString;     // Specification
    int            numOfFunctions; // No. overloaded functions
    SelectFunction selectFunc;
    ValueMapping*  valueMap; // Array of size numOfFunctions
    TypeMapping    typeMap;
};

/*
1.5 Class "TypeConstructor"[1]

An instance of class ~TypeConstructor~ consists of

  * a name

  * a function (``outFunc'') converting a value's Address-representation to the
    corresponding nested list representation

  * a function (``inFunc'') converting a value's nested list representation to
    the corresponding Address-represantation

  * a function (``createFunc'') allocating main memory for values which can't be
    represented by a single Address (may be valid for all types in the future)

  * a function (``deleteFunc'') releasing the memory previously allocated by
    createFunc.

  * a function (``openFunc'').

  * a function (``saveFunc'').

  * a function (``closeFunc'').

  * a function (``cloneFunc'').

All properties of an instance of class TypeConstructor are set within the
constructor.

*/

struct ConstructorInfo {

  string name;	
  string signature;
  string typeExample;
  string listRep;
  string valueExample;
  string remarks;

  ConstructorInfo() :
    name(""),	  
    signature(""),	  
    typeExample(""),
    listRep(""),
    valueExample(""),
    remarks("")
  {}	    
};


#include "ConstructorTemplates.h"

class TypeConstructor
{
 public:
  TypeConstructor( const string& nm,
                   TypeProperty prop,
                   OutObject out,
                   InObject in,
                   OutObject saveToList,
                   InObject restoreFromList,
                   ObjectCreation create,
                   ObjectDeletion del,
                   ObjectOpen open,
                   ObjectSave save,
                   ObjectClose close,
                   ObjectClone clone,
                   ObjectCast ca,
                   ObjectSizeof sizeOf,
                   TypeCheckFunction tcf );

  template<class T>
  TypeConstructor( const ConstructorInfo ci,
                   ConstructorFunctions<T> cf  )
  {  
    conInfo              = ci;	
    name                 = ci.name;
    propFunc             = 0;
    
    outFunc              = cf.out;
    inFunc               = cf.in;
    saveToListFunc       = cf.saveToList;
    restoreFromListFunc  = cf.restoreFromList;
    createFunc           = cf.create;
    deleteFunc           = cf.deletion;
    openFunc             = cf.open;
    saveFunc             = cf.save;
    closeFunc            = cf.close;
    cloneFunc            = cf.clone;
    castFunc             = cf.cast;
    sizeofFunc           = cf.sizeOf;
    typeCheckFunc        = cf.typeCheck;
  }

  
/*
Constructs a type constructor.

*/
  virtual ~TypeConstructor();
/*
Destroys an instance of a type constructor.

*/
  void AssociateKind( const string& kindName );
/*
Associates the kind ~kindName~ with this type constructor.

*/
  ListExpr Property();
  static ListExpr Property(const ConstructorInfo& ci);
/*
Returns the properties of the type constructor as a nested list.

*/
  ListExpr Out( ListExpr type, Word value );
  Word     In( const ListExpr typeInfo,
               const ListExpr value,
               const int errorPos,
               ListExpr& errorInfo,
               bool& correct );
  ListExpr SaveToList( ListExpr type, Word value );
  Word     RestoreFromList( const ListExpr typeInfo,
                            const ListExpr value,
                            const int errorPos,
                            ListExpr& errorInfo,
                            bool& correct );
  Word     Create( const ListExpr typeInfo );
  void     Delete( const ListExpr typeInfo,
                   Word& w );
  bool     Open( SmiRecord& valueRecord,
                 size_t& offset,
                 const ListExpr typeInfo,
                 Word& value );
  bool     Save( SmiRecord& valueRecord, 
                 size_t& offset, 
                 const ListExpr typeInfo,
                 Word& value );
  void     Close( const ListExpr typeInfo,
                  Word& w );
  Word     Clone( const ListExpr typeInfo,
                  const Word& w );
  int      SizeOf();
  string&  Name() { return name; }

/*
Are methods to manipulate objects according to the type constructor.

*/
  bool     DefaultOpen( SmiRecord& valueRecord,
                        size_t& offset,
                        const ListExpr typeInfo,
                        Word& value );
  bool     DefaultSave( SmiRecord& valueRecord,
                        size_t& offset,
                        const ListExpr typeInfo,
                        Word& value );
/*
Default methods for ~Open~ and ~Save~ functions if these are not provided.
These methods use the ~RestoreFromList~ and ~SaveToList~ if provided, and
~In~ and ~Out~ otherwise. 

*/
  static Word DummyCreate( const ListExpr typeInfo );
  static void DummyDelete( const ListExpr typeInfo,
                           Word& w );
  static void DummyClose( const ListExpr typeInfo,
                          Word& w );
  static Word DummyClone( const ListExpr typeInfo,
                          const Word& w );
  static int  DummySizeOf();
  
  bool TypeCheck( ListExpr type, ListExpr& errorInfo )
  {
    if (typeCheckFunc)
      return (*typeCheckFunc)( type, errorInfo);
    else 
      return SimpleCheck( type, errorInfo );
  }


/*
Dummy methods used as placeholders for type constructor functions.

*/
 private:
 
  inline bool SimpleCheck( ListExpr type, ListExpr& errorInfo )
  {
    return (nl->IsEqual( type, name ));
  }

  ConstructorInfo          conInfo;
  string                   name;   // Name of type constr.
  TypeProperty             propFunc;
  OutObject                outFunc;
  InObject                 inFunc;
  OutObject                saveToListFunc;
  InObject                 restoreFromListFunc;
  ObjectCreation           createFunc;
  ObjectDeletion           deleteFunc;
  ObjectOpen               openFunc;
  ObjectSave               saveFunc;
  ObjectClose              closeFunc;
  ObjectClone              cloneFunc;
  ObjectCast               castFunc;
  ObjectSizeof             sizeofFunc;
  TypeCheckFunction        typeCheckFunc;
  vector<string>           kinds;  // Kinds of type constr.

  friend class AlgebraManager;
};

/*
1.6 Class "Algebra"[1]

An instance of class ~Algebra~ provides access to all information about a given
algebra at run time, i.e. a set of type constructors and a set of operators.
These properties have to be set once. A straightforward approach is to do
these settings within an algebra's constructor. As all algebra modules use
different type constructors and operators, all algebra constructors are
different from each other. Hence we cannot use a single constructor, but
request algebra implementors to derive a new subclass of class ~Algebra~ for
each algebra module in order to provide a new constructor. Each of these
subclasses will be instantiated exactly once. An algebra subclass
instance serves as a handle for accessing an algebra's type constructors
and operators.

*/

class Algebra
{
 public:
  Algebra();
/*
Creates an instance of an algebra. Concrete algebra modules are implemented
as subclasses of class ~Algebra~.

*/
  virtual ~Algebra();
/*
Destroys an algebra instance.

*/
  int GetNumTCs() { return (tcs.size()); }
/*
Returns the number of type constructors provided by the algebra module.

*/
  int GetNumOps() { return (ops.size()); }
/*
Returns the number of operators provided by the algabra module.

*/
  TypeConstructor* GetTypeConstructor( int index )
  { 
     assert((index >= 0) && (index <= tcsNum-1)); 
     return tcs[index]; 
  }
/*
Returns a reference to the type constructor identified by ~index~.

*/
  Operator* GetOperator( int index ) 
  { 
     assert((index >= 0) && (index <= opsNum-1)); 
     return ops[index]; 
  }
/*
Returns a reference to the operator identified by ~index~.

*/
 protected:
  void AddTypeConstructor( TypeConstructor* tc );
  void AddOperator( Operator* op );
/*
Are used by the subclassed algebra to add its type constructors and
operators to the list of type constructors and operators within the
base class.

*/
 private:
  vector<TypeConstructor*> tcs;
  vector<Operator*> ops;
  int tcsNum;
  int opsNum;

  friend class AlgebraManager;
};

#endif

