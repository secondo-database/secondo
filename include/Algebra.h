/*
1 Header File: Algebra

1.1 Overview

A snapshot of a working Secondo system will show a collection of algebras,
each of them ``populated'' by two different kinds of entities: objects and
operators. 
Operators are fix in terms of number and functionality which 
are defined by the algebra implementor. In contrast to that, the number of
objects is variable and changes dynamically with runtime. Even the types of
objects are not predefined, but only their type constructors. 

These very different properties of types and objects give rise to a very
different C++ representation of  operators and objects:

  * Operators are instances of a predefined class ~Operator~. Thus an
implementation  of an algebra with n operators contains n definitions of
instances of class ~Operator~. 

  * Objects cannot be predefined, because they are constructed and deleted at
runtime. Even the possible types of objects cannot be predeclared, because they
can be declared at runtime, too. Only an algebras's  ~type constructors~ are 
well known and fix. An implementation of an algebra with m 
type constructors contains m definitions of instances of the predefined
class ~TypeConstructor~.

From a top level point of view, a Secondo universe is a collection of algebras.
This can be implemented by defining an instance of a subclass
of the predefined class ~Algebra~ for 
each existing algebra. Each of these ~Algebra~ 
instances essentially consists of a set of operators and a set of 
type constructors.

1.2 Defines, Includes

*/

#ifndef ALGEBRA_H
#define ALGEBRA_H

#include <string>
#include <vector>
#include "AlgebraManager.h"

/*
1.4 Class Operator

An operator instance consists of 

  * a name 

  * at least one value mapping function, sometimes called evaluation function

  * a type Mapping function, returned the operator's result type with respect 
    to input parameters type

  * a selection function calculating the index of a value mapping function
    with respect to input parameter types

  * model and cost mapping functions (reserved for future use) 

All properties of operators are set in the constructor. Only the value mapping
functions have to be registrated later on since their number is arbitrary. This
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
constructs an operator with ~noF~ overloaded evaluation functions.

*/
  Operator( const string& nm,
            const string& spec,
            ValueMapping vm,
	    ModelMapping mm,
            SelectFunction sf,
	    TypeMapping tm,
            CostMapping cm = Operator::DummyCost );
/*
constructs an operator with *one* evaluation functions.

*/
  virtual ~Operator();
/*
destroys an operator instance.

*/
  string Specification();
/*
returns the operator specification as a string.

*/
  int      Select( ListExpr argtypes );
/*
returns the index of the overloaded evaluation function depending on
the argument types ~argtypes~.

*/
  int CallValueMapping( const int index, ArgVector args, Word& result,
                        int message, Word& local, Supplier s );
/*
calls the value mapping function of the operator.

*/
  Word CallModelMapping( const int index, ArgVector argv, Supplier s );
/*
calls the model mapping function of the operator.

*/
  ListExpr CallTypeMapping( ListExpr argList );
/*
calls the type mapping function of the operator.

*/
  ListExpr CallCostMapping( ListExpr argList );
/*
calls the cost mapping function of the operator.

*/
  static Word     DummyModel( ArgVector, Supplier );
/*
defines a dummy model mapping function for operators.

*/
  static ListExpr DummyCost( ListExpr );
/*
defines a dummy cost mapping function for operators.

*/
 private:
  bool AddValueMapping( const int index, ValueMapping f );
/*
adds a value mapping function to the list of overloaded operator functions.

*/
  bool AddModelMapping( const int index, ModelMapping f );
/*
adds a model mapping function to the list of overloaded operator functions.

*/
  string         name;           // Name of operator
  string         specString;     // Specification (string)
  ListExpr       specification;  // Specification (nested list)
  int            numOfFunctions; // No. of overloaded functions
  SelectFunction selectFunc;
  ValueMapping*  valueMap; // Array of size numOfFunctions
  ModelMapping*  modelMap; // Array of size numOfFunctions
  TypeMapping    typeMap;
  CostMapping    costMap;

  friend class AlgebraManager;
};

/*
1.5 Class TypeConstructor

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
                   ObjectCast ca,
                   TypeCheckFunction tcf,
                   InModelFunction inm =
                     TypeConstructor::DummyInModel,
                   OutModelFunction outm =
                     TypeConstructor::DummyOutModel,
                   ValueToModelFunction vtm =
                     TypeConstructor::DummyValueToModel,
                   ValueListToModelFunction vltm =
                     TypeConstructor::DummyValueListToModel );
/*
constructs a type constructor.

*/
  virtual ~TypeConstructor();
/*
destroys an instance of a type constructor.

*/
  void AssociateKind( const string& kindName );
/*
associates the kind ~kindName~ with this type constructor.

*/
  ListExpr Property();
/*
returns the properties of the type constructor as a nested list.

*/
  ListExpr Out( ListExpr type, Word value );
  Word     In( const ListExpr type, const ListExpr value, 
               const int errorPos, ListExpr& errorInfo, bool& correct );
  Word     Create( int Size );
  void     Delete( Word& w );

  Word     InModel( ListExpr, ListExpr, int );
  ListExpr OutModel( ListExpr, Word );
  Word     ValueToModel( ListExpr, Word );
  Word     ValueListToModel( const ListExpr typeExpr,
                             const ListExpr valueList,
                             const int errorPos,
                             ListExpr& errorInfo,
                             bool& correct );
/*
are methods to manipulate objects and models according to the type
constructor.

*/
  static Word     DummyInModel( ListExpr typeExpr, ListExpr list, int objNo );
  static ListExpr DummyOutModel( ListExpr typeExpr, Word model );
  static Word     DummyValueToModel( ListExpr typeExpr, Word value );
  static Word     DummyValueListToModel( const ListExpr typeExpr,
                                         const ListExpr valueList,
                                         const int errorPos,
                                         ListExpr& errorInfo,
                                         bool& correct );
/*
are dummy methods used as placeholders for model manipulating type
constructor functions.

*/
 private:
  string                   name;       // Name of type constr.
  ListExpr                 property;   // Properties
  TypeProperty             propFunc;
  OutObject                outFunc;
  InObject                 inFunc;
  ObjectCreation           createFunc;
  ObjectDeletion           deleteFunc;
  ObjectCast               castFunc;
  InModelFunction          inModelFunc;
  OutModelFunction         outModelFunc;
  ValueToModelFunction     valueToModelFunc;
  ValueListToModelFunction valueListToModelFunc;
  TypeCheckFunction        typeCheckFunc;

  vector<string>           kinds;      // Kinds of type constr.

  friend class AlgebraManager;
};

/*
1.6 Class Algebra

An instance of class algebra provides access to all information about a given
algebra at run time, i.e. a set of type constructors and a set of operators.
These properties have to be set once. A straightforward approach is to do
these settings within an algebra's constructor. As all algebra modules use
different type constructors and operators, all algebra constructors are 
different from each other. Hence we cannot use a single constructor, but 
request algebra implementors to derive a new subclass of class Algebra for
each algebra module in order to provide a new constructor. Each of these
subclasses will be instantiated exactly once. An algebra subclass
instance serves as a handle for accessing an algebra's type constructors 
and operators.

*/

class Algebra
{
 public: 
  Algebra();
  virtual ~Algebra();
  int GetNumTCs()
  {
    return (tcs.size());
  }
  int GetNumOps()
  {
    return (ops.size());
  }
  TypeConstructor* GetTypeConstructor( int index )
  {
    return (tcs[index]);
  }
  Operator* GetOperator( int index )
  {
    return (ops[index]);
  }
 protected:
  void AddTypeConstructor( TypeConstructor* tc );
  void AddOperator( Operator* op );
 private:
  vector<TypeConstructor*> tcs;
  vector<Operator*> ops;

  friend class AlgebraManager;
};

#endif

