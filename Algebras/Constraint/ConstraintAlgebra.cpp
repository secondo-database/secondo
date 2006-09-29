/*
----
This file is part of SECONDO.

Copyright (C) 2006, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the Constraint Algebra

July, 2006. Simon Muerner

[TOC]

1 Preliminaries

This implementation file essentially contains the definitions and implementations of the type constructur
~constrinat~ with its associated operations. 

1.1 Defines and Includes

*/
using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "ConstraintAlgebra.h"
#include "StandardTypes.h"

extern NestedList* nl;
extern QueryProcessor* qp;


namespace Constraint {
/*
2 Type Constructor ~constraint~

2.1 TODO

A value of type ~constraint~ represents a 2-dimensional (possibly infinite) pointset.

3 In/Out Functions

3.1 List Representation

The list representation of a 2D constraint is

TODO

3.2 ~Out~-function


*/
ListExpr OutConstraint( ListExpr typeInfo, Word value )
{
  SymbolicRelation* symRel = (SymbolicRelation*)value.addr;
  const SymbolicTuple* pSymRelIP;
  const LinearConstraint* pLinConstraint;
  ListExpr result;
  ListExpr tempRes;
  ListExpr lastCon;
  ListExpr lastDis;
  
  if(symRel->SymbolicTuplesSize()==0)
  {
    result = nl->TheEmptyList();
  }
  else // symRel->SymbolicTuplesSize() > 0 (minimum: 1 Tuple)
  {
    for(unsigned int i = 0; i < symRel->SymbolicTuplesSize(); i++)
    {
      symRel->GetSymbolicTuples(i, pSymRelIP);
      for(int j = pSymRelIP->startIndex; j <= pSymRelIP->endIndex; j++)
      {      
        symRel->GetLinConstraints(j, pLinConstraint);
        double a1 = pLinConstraint->get_a1();
        double a2 = pLinConstraint->get_a2();
        double b = pLinConstraint->get_b();
        string Op = pLinConstraint->get_Op();      
        if(i==0)
        {
          if(j==pSymRelIP->startIndex)
          {
            // bei der ersten Konjunktion der ersten Disjunktion:
            tempRes = nl->OneElemList(nl->FourElemList(nl->RealAtom(a1), 
              nl->RealAtom(a2), nl->RealAtom(b), nl->SymbolAtom(Op)));
            lastCon = tempRes;                
          }
          if(j>pSymRelIP->startIndex) 
          {
            // bei den weiteren Konjunktionen der ersten Disjunktion:
            lastCon = nl->Append(lastCon, nl->FourElemList(nl->RealAtom(a1), 
              nl->RealAtom(a2), nl->RealAtom(b), nl->SymbolAtom(Op)));        
          }
          if(j==pSymRelIP->endIndex)
          {
            // bei der letzten Konjunktion der ersten Disjunktion:
            result = nl->OneElemList(tempRes);
            lastDis = result;
          }
        } 
        else // also i>0
        {
          if(j==pSymRelIP->startIndex)
          {
            // bei der ersten Konjunktion von einer weiteren Disjunktion:
            tempRes = nl->OneElemList(
              nl->FourElemList(nl->RealAtom(a1), 
                nl->RealAtom(a2), nl->RealAtom(b), nl->SymbolAtom(Op)));
            lastCon = tempRes;
          }
          if(j>pSymRelIP->startIndex)
          {
            // bei einer weiteren Konjunktion von einer weiteren Disjunktion:
            lastCon = nl->Append(lastCon, nl->FourElemList(
              nl->RealAtom(a1), nl->RealAtom(a2), 
              nl->RealAtom(b), nl->SymbolAtom(Op)));      
          }
          if(j==pSymRelIP->endIndex)
          {
            // bei der letzten Konjunktion von einer weiteren Disjunktion:
            lastDis = nl->Append(lastDis, tempRes);
          }      
        }
      }
    }
  }
  return result;
}

/*
3.3 ~In~-function

*/
Word InConstraint( const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  ListExpr symbolicTuplesNL = instance;
  ListExpr linConstraintsNL;
  ListExpr oneLinConstraintNL;
  
  if(!nl->IsAtom(instance))
  {  
    SymbolicRelation* symbolicRelation = new SymbolicRelation();
    while(!nl->IsEmpty(symbolicTuplesNL))
    {
      linConstraintsNL = nl->First(symbolicTuplesNL);
      symbolicTuplesNL = nl->Rest(symbolicTuplesNL);        
      if(!nl->IsAtom(linConstraintsNL))
      {
        vector<LinearConstraint> vLinConstraints;        
        while(!nl->IsEmpty(linConstraintsNL))
        {
          oneLinConstraintNL = nl->First(linConstraintsNL);
          linConstraintsNL = nl->Rest(linConstraintsNL);
          if(nl->ListLength(oneLinConstraintNL) == 4  &&
              nl->AtomType(nl->Nth(1, oneLinConstraintNL)) == RealType &&
              nl->AtomType(nl->Nth(2, oneLinConstraintNL)) == RealType &&
              nl->AtomType(nl->Nth(3, oneLinConstraintNL)) == RealType &&
              nl->AtomType(nl->Nth(4, oneLinConstraintNL)) == SymbolType)
          {
            LinearConstraint linConstraint(
                nl->RealValue(nl->Nth(1, oneLinConstraintNL)),
                nl->RealValue(nl->Nth(2, oneLinConstraintNL)),
                nl->RealValue(nl->Nth(3, oneLinConstraintNL)),
                nl->SymbolValue(nl->Nth(4, oneLinConstraintNL)));   
            vLinConstraints.push_back(linConstraint);
          }
          else
          {
            cout << "ERROR: Fehlerhafter Aufbau von lin. Constraint!" << endl;
            correct = false;
            return SetWord(Address(0));
          }            
        }          
        symbolicRelation->addSymbolicTuple(vLinConstraints);      
      }
      else
      {
        cout << "ERROR: linConstraintsNL is atomic (shoud be a list!)";
        correct = false;
        return SetWord(Address(0));
      }    
    } // while
    // important: each symbolic relation as imput will be 
    // directly normalized before saving it:
    symbolicRelation->Normalize(); 
    return SetWord(symbolicRelation);    
  }
  else
  {
    cout << "ERROR: instance is atomic (shoud be a list!)";
    correct = false;
    return SetWord(Address(0));
  }
}

/*
3.4 Function describing the signature of the type constructor

*/
ListExpr
Constraint2Property()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"), 
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("constraint"),
  nl->TextAtom("(<tuple>*) where tuple is"
  " (<lin_constraint_1>...<lin_constraint_n> with n>0 "
  "where lin_constraint_i is)"
  " (<a1> <a2> <b> <OP>) where a1, a2, b are" 
  "real-values an OP is a symbol in the set {eq, leq}"),
  nl->TextAtom("(((0.0 1.0 2.1 leq)(1.0 1.0 -4.0 eq))"
  "((0.0 1.0 5.0 eq)(1.8 0.0 3.0 leq)))"),
  nl->TextAtom("each linear constraint represents the "
  "(in)equation: <a1>*x + <a2>*y +<b> <OP> 0")
  )));                        
}

/*
5 Further Functions

5.1 ~Create~-function

*/
Word CreateConstraint( const ListExpr typeInfo )
{
  SymbolicRelation* symbolicRelation = new SymbolicRelation();
  vector<LinearConstraint> vLinConstraints;
  LinearConstraint linConstraint(0.0, 0.0, 0.0, OP_EQ);
  vLinConstraints.push_back(linConstraint);
  symbolicRelation->addSymbolicTuple(vLinConstraints);  
  return (SetWord(symbolicRelation));
}

/*
5.2 ~Delete~-function

*/
void DeleteConstraint( const ListExpr typeInfo, Word& w )
{
  SymbolicRelation *sr = (SymbolicRelation *)w.addr;
  sr->Destroy();
  delete sr;
  w.addr = 0;  
}

/*
5.3 ~Close~-function

*/
void CloseConstraint( const ListExpr typeInfo, Word& w )
{
  delete (SymbolicRelation *)w.addr;
  w.addr = 0;
}

/*
5.4 ~Clone~-function

*/
Word CloneConstraint( const ListExpr typeInfo, const Word& w )
{
  return SetWord(((SymbolicRelation *)w.addr)->Clone());
}


/*
5.5 ~SizeOf~-function

*/
int SizeOfConstraint()
{
  return sizeof(SymbolicRelation);
}


/*
5.6 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~constraint~ does not have arguments, this is trivial.

*/
bool
CheckConstraint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "constraint" ));
}

/*
6 Creation of the type constructor instance

*/
TypeConstructor constraint(
        "constraint", //name
        Constraint2Property, //property function describing signature
        OutConstraint,     InConstraint, //Out and In functions
        0,                   0, //SaveToList and RestoreFromList functions
        CreateConstraint,  DeleteConstraint, //object creation and deletion
        0,               0, //open and save functions
        CloseConstraint,   CloneConstraint, //object close, and clone
        DummyCast, //cast function
        SizeOfConstraint, //sizeof function
        CheckConstraint ); //kind checking function

/*
7 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

7.1 Type mapping functions

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

7.1.1 Type mapping function ~CxC2CTypeMap~

*/

ListExpr CxC2CTypeMap( ListExpr args )
{
    ListExpr arg1, arg2;
    if ( nl->ListLength( args ) == 2 )
    {
      arg1 = nl->First( args );
      arg2 = nl->Second( args );
      if(nl->IsEqual(arg1, "constraint") && nl->IsEqual(arg2, "constraint"))
      {
        return (nl->SymbolAtom("constraint"));
      }
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
7.1.1 Type mapping function ~C2CTypeMap~

*/
ListExpr C2CTypeMap( ListExpr args )
{
    ListExpr arg;
    if ( nl->ListLength( args ) == 1 )
    {
      arg = nl->First( args );

      if(nl->IsEqual(arg, "constraint"))
      {
        return (nl->SymbolAtom("constraint"));
      }
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
7.2 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are
several value mapping functions, one for each possible combination of input
parameter types.


7.2.1 Value mapping functions of operator ~cunion~

*/
int unionValueMap( Word* args, Word& result, int message, 
           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SymbolicRelation* symRelFirst = ((SymbolicRelation*)args[0].addr);
  SymbolicRelation* symRelSecond = ((SymbolicRelation*)args[1].addr);  
  SymbolicRelation* symRelResult;
  
  if(symRelFirst->SymbolicTuplesSize() < symRelSecond->SymbolicTuplesSize())
  {
    // dann mache noch einen Variablen-Umtausch, 
    // damit die Anzahl symbolischer Tupel, deren Inhalt 
    // veraendert werden muss (Index-Aktualisierung), minimiert wird:
    SymbolicRelation* symRelTemp = symRelFirst;
    symRelFirst = symRelSecond;
    symRelSecond = symRelTemp;
  }
  
  symRelResult = symRelFirst->Clone();
  symRelResult->appendSymbolicRelation(*symRelSecond);  
  *((SymbolicRelation *)result.addr) = *symRelResult;
  return (0);
}

/*
7.2.2 Value mapping functions of operator ~cintersection~

*/
int intersectionValueMap( Word* args, Word& result, int message, 
           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SymbolicRelation* symRelFirst = ((SymbolicRelation*)args[0].addr);
  SymbolicRelation* symRelSecond = ((SymbolicRelation*)args[1].addr);  
  SymbolicRelation* symRelResult;
  

  symRelResult = symRelFirst->Clone();
  symRelResult->overlapSymbolicRelation(*symRelSecond);  
  symRelResult->Normalize();
  *((SymbolicRelation *)result.addr) = *symRelResult;
  return (0);
}


/*
7.3 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of value
mapping functions for each operator. For nonoverloaded operators there is also such and array
defined, so it easier to make them overloaded.

7.3.1 Definition of specification strings

*/
const string ConstraintSpecUnion  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> (constraint x constraint) -> constraint</text--->"
  "<text>_cunion_</text--->"
  "<text>union of two constraints.</text--->"
  "<text>query constraint1 cunion constraint2</text--->"
  ") )";

const string ConstraintSpecIntersection  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> (constraint x constraint) -> constraint</text--->"
  "<text>_cintersection_</text--->"
  "<text>intersection of two constraints.</text--->"
  "<text>query constraint1 cintersection constraint2</text--->"
  ") )";
        

/*
7.3.2 Definition of the operators

*/
Operator constraintunion( "cunion",
                          ConstraintSpecUnion,
                          unionValueMap,
                          Operator::SimpleSelect,
                          CxC2CTypeMap);

Operator constraintintersection( "cintersection",
                          ConstraintSpecIntersection,
                          intersectionValueMap,
                          Operator::SimpleSelect,
                          CxC2CTypeMap);                          
                                               

/*
8 Creating the Algebra

*/

class ConstraintAlgebra : public Algebra
{
 public:
  ConstraintAlgebra() : Algebra()
  {
    AddTypeConstructor( &constraint );

    constraint.AssociateKind("DATA");

    AddOperator( &constraintunion );
    AddOperator( &constraintintersection );

  }
  ~ConstraintAlgebra() {};
};

ConstraintAlgebra constraintAlgebra;

/*
9 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/
extern "C"
Algebra*
InitializeConstraintAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&constraintAlgebra);
}

} // namespace

/*
10 References

[Point02] Algebra Module PointRectangleAlgebra. FernUniversit[ae]t Hagen, Praktische Informatik IV, Secondo System, Directory ["]Algebras/PointRectangle["], file ["]PointRectangleAlgebra.cpp["], since July 2002

*/
