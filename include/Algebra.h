/*
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

#ifndef ALGEBRA_H
#define ALGEBRA_H

#include <string>
#include <vector>
#include "AlgebraManager.h"

/*
1.4 Class "Operator"[1]

An operator instance consists of 

  * a name 

  * at least one value mapping function, sometimes called evaluation function

  * a type Mapping function, returned the operator's result type with respect 
    to input parameters type

  * a selection function calculating the index of a value mapping function
    with respect to input parameter types

  * model and cost mapping functions (reserved for future use) 

All properties of operators are set in the constructor. Only the value mapping
functions have to be registered later on since their number is arbitrary. This
number is set in the constructor (~noF~). 

*/

class Operator
{
 public:
  Operator( const string& nm,
            const string& spec,
            const int noF,
            ValueMapping vms[],
	          ModelMapping mms[],
            SelectFunction sf,
      	    TypeMapping tm,
            CostMapping cm = Operator::DummyCost );
/*
Constructs an operator with ~noF~ overloaded evaluation functions.

*/
  Operator( const string& nm,
            const string& spec,
            ValueMapping vm,
	    ModelMapping mm,
            SelectFunction sf,
	    TypeMapping tm,
            CostMapping cm = Operator::DummyCost );
/*
Constructs an operator with *one* evaluation functions.

*/
  virtual ~Operator();
/*
Destroys an operator instance.

*/
  string Specification();
/*
Returns the operator specification as a string.

*/
  int      Select( ListExpr argtypes );
/*
Returns the index of the overloaded evaluation function depending on
the argument types ~argtypes~.

*/
  int CallValueMapping( const int index,
                        ArgVector args,
                        Word& result,
                        int message,
                        Word& local,
                        Supplier s );
/*
Calls the value mapping function of the operator.

*/
  Word CallModelMapping( const int index,
                         ArgVector argv,
                         Supplier s );
/*
Calls the model mapping function of the operator.

*/
  ListExpr CallTypeMapping( ListExpr argList );
/*
Calls the type mapping function of the operator.

*/
  ListExpr CallCostMapping( ListExpr argList );
/*
Calls the cost mapping function of the operator.

*/
  static Word     DummyModel( ArgVector, Supplier );
/*
Defines a dummy model mapping function for operators.

*/
  static ListExpr DummyCost( ListExpr );
/*
Defines a dummy cost mapping function for operators.

*/
  static int SimpleSelect( ListExpr );
/*
Defines a dummy cost mapping function for operators.

*/
 private:
  bool AddValueMapping( const int index, ValueMapping f );
/*
Adds a value mapping function to the list of overloaded operator functions.

*/
  bool AddModelMapping( const int index, ModelMapping f );
/*
Adds a model mapping function to the list of overloaded operator functions.

*/
  string         name;           // Name of operator
  string         specString;     // Specification
  int            numOfFunctions; // No. overloaded functions
  SelectFunction selectFunc;
  ValueMapping*  valueMap; // Array of size numOfFunctions
  ModelMapping*  modelMap; // Array of size numOfFunctions
  TypeMapping    typeMap;
  CostMapping    costMap;

  friend class AlgebraManager;
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

class TypeConstructor
{
 public:
  TypeConstructor( const string& nm,
                   TypeProperty prop,
                   OutObject out,
                   InObject in,
                   ObjectCreation create,
                   ObjectDeletion del,
                   ObjectOpen open,
                   ObjectSave save,
                   ObjectClose close,
                   ObjectClone clone,
                   ObjectCast ca,
                   ObjectSizeof sizeOf,
                   TypeCheckFunction tcf,
                   PersistFunction pmf = 0,
                   InModelFunction inm =
                     TypeConstructor::DummyInModel,
                   OutModelFunction outm =
                     TypeConstructor::DummyOutModel,
                   ValueToModelFunction vtm =
                     TypeConstructor::DummyValueToModel,
                   ValueListToModelFunction vltm =
                     TypeConstructor::DummyValueListToModel );
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
/*
Returns the properties of the type constructor as a nested list.

*/
  ListExpr   Out( ListExpr type, Word value );
  Word       In( const ListExpr typeInfo,
                 const ListExpr value, 
                 const int errorPos,
                 ListExpr& errorInfo,
                 bool& correct );
  Word       Create( const ListExpr typeInfo );
  void       Delete( Word& w );
  bool       Open( SmiRecord& valueRecord,
                   const ListExpr typeInfo,
                   Word& value );
  bool       Save( SmiRecord& valueRecord,
                   const ListExpr typeInfo,
                  Word& value );
  void       Close( Word& w );
  Word       Clone( const Word& w );
  int        SizeOf();

  Word       InModel( ListExpr, ListExpr, int );
  ListExpr   OutModel( ListExpr, Word );
  Word       ValueToModel( ListExpr, Word );
  Word       ValueListToModel( const ListExpr typeExpr,
                               const ListExpr valueList,
                               const int errorPos,
                               ListExpr& errorInfo,
                               bool& correct );
/*
Are methods to manipulate objects and models according to the type
constructor.

*/
  bool     PersistModel( const PersistDirection dir,
                         SmiRecord& modelRecord,
                         const ListExpr typeExpr,
                         Word& model );
  bool     DefaultPersistModel( const PersistDirection dir,
                                SmiRecord& modelRecord,
                                const ListExpr typeExpr,
                                Word& model );
  bool     DefaultOpen( SmiRecord& valueRecord,
                        const ListExpr typeInfo,
                        Word& value );
  bool     DefaultSave( SmiRecord& valueRecord,
                        const ListExpr typeInfo,
                        Word& value );
/*
Are methods to support persistence for objects and models according to the type
constructor. The same methods are used for saving or restoring an object or model
to or from its persistent representation. The direction is given by the parameter
~dir~; possible values are "ReadFrom"[4] and "WriteTo"[4].

An algebra implementor may choose to use a default implementation for these
methods. The default methods use nested lists to represent the persistent
object an model values. If a type does not need more than one "Word"[4] of
storage for representing an object value, a dummy method could be specified
by the implementor.

For types like tuples or relations the default methods are not appropriate and
should be overwritten. For tuples and relations meta information about the
structure is needed and should be stored in the "Secondo"[3] catalog through
this mechanism. The tuples and relations itself should be stored into ~SmiFiles~
by the algebra module.

*/
  static bool DummyPersistModel( const PersistDirection dir,
                                 SmiRecord& modelRecord,
                                 const ListExpr typeExpr,
                                 Word& model );
  static Word DummyInModel( ListExpr typeExpr,
                            ListExpr list,
                            int objNo );
  static ListExpr DummyOutModel( ListExpr typeExpr,
                                 Word model );
  static Word DummyValueToModel( ListExpr typeExpr,
                                 Word value );
  static Word DummyValueListToModel( const ListExpr typeExpr,
                                     const ListExpr valueList,
                                     const int errorPos,
                                     ListExpr& errorInfo,
                                     bool& correct );
/*
Are dummy methods used as placeholders for model manipulating type
constructor functions.

*/
 private:
  string                   name;   // Name of type constr.
  TypeProperty             propFunc;
  OutObject                outFunc;
  InObject                 inFunc;
  ObjectCreation           createFunc;
  ObjectDeletion           deleteFunc;
  ObjectOpen               openFunc;
  ObjectSave               saveFunc;
  ObjectClose              closeFunc;
  ObjectClone              cloneFunc;
  ObjectCast               castFunc;
  ObjectSizeof             sizeofFunc;
  PersistFunction          persistModelFunc;
  InModelFunction          inModelFunc;
  OutModelFunction         outModelFunc;
  ValueToModelFunction     valueToModelFunc;
  ValueListToModelFunction valueListToModelFunc;
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
                     { return (tcs[index]); }
/*
Returns a reference to the type constructor identified by ~index~.

*/
  Operator* GetOperator( int index ) { return (ops[index]); }
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

  friend class AlgebraManager;
};

#endif

