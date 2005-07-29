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

//paragraph [1] title: [{\Large \bf ]	[}]


[1] TupleIdentifier Algebra

March 2005 Matthias Zielke

The only purpose of this algebra is to provide a typeconstructor 'tid' so that the tupleidentifiers
of tuples from relations can be stored as attribute-values in different tuples. This feature is needed
for the implementation of operators to update relations.

1 Preliminaries

1.1 Includes

*/

using namespace std;


#include "TupleIdentifier.h"
	

extern NestedList* nl;
extern QueryProcessor *qp;


/*

2 Type Constructor ~tid~

*/

void TupleIdentifier::CopyFrom(StandardAttribute* attr)
{
  TupleIdentifier* tupleI = (TupleIdentifier*) attr;
  defined = tupleI->IsDefined();
  tid = tupleI->GetTid();
}

int TupleIdentifier::NumOfFLOBs()
{
  return 0;
}


bool TupleIdentifier::Adjacent( Attribute* arg )
{
  TupleId argTid = ((TupleIdentifier *)arg)->GetTid();

  return( tid == argTid -1 || tid == argTid + 1 );
}

TupleIdentifier::TupleIdentifier(bool DEFINED, TupleId TID) {defined = DEFINED, tid = TID;}

TupleIdentifier::~TupleIdentifier() {}

TupleId TupleIdentifier::GetTid() {return tid;}

void TupleIdentifier::SetTid(TupleId TID) {tid = TID;}

TupleIdentifier* TupleIdentifier::Clone() { return new TupleIdentifier( *this ); }

/*
2.2 List Representation

The list representation of a TupleIdentifier is

----	(tid)
----

2.3 ~In~ and ~Out~ Functions

*/

ListExpr
OutTupleIdentifier( ListExpr typeInfo, Word value )
{
  TupleIdentifier* tupleI = (TupleIdentifier*)(value.addr);
  return nl->OneElemList(nl->IntAtom(tupleI->GetTid()));
}

Word
InTupleIdentifier( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->ListLength( instance ) == 1 )
  {
    ListExpr First = nl->First(instance);

    if ( nl->IsAtom(First) && nl->AtomType(First) == IntType)
    {
      correct = true;
      TupleIdentifier* newTid = new TupleIdentifier(true, nl->IntValue(First));
      return SetWord(newTid);
    }
  }
  correct = false;
  return SetWord(Address(0));
}

/*
2.4 Functions Describing the Signature of the Type Constructors

This one works for type constructor ~tid~.

*/
ListExpr
TupleIdentifierProperty()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
	        nl->StringAtom("Example Type List"),
			nl->StringAtom("List Rep"),
			nl->StringAtom("Example List"),
			nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
	        nl->StringAtom("tid"),
			nl->StringAtom("(<tid>)"),
			nl->StringAtom("(50060)"),
			nl->StringAtom("The tupleidentifier is a long."))));
}
Word
CreateTupleIdentifier( const ListExpr typeInfo )
{
  return (SetWord( new TupleIdentifier(true, 0) ));
}

void
DeleteTupleIdentifier( Word& w )
{
  delete (TupleIdentifier *)w.addr;
  w.addr = 0;
}

void
CloseTupleIdentifier( Word& w )
{
  delete (TupleIdentifier *)w.addr;
  w.addr = 0;
}

Word
CloneTupleIdentifier( const Word& w )
{
  return SetWord( ((TupleIdentifier *)w.addr)->Clone() );
}

int
SizeOfTupleIdentifier()
{
  return sizeof(TupleIdentifier);
}

/*
2.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~TupleIdentifier~ does not have arguments, this is trivial.

*/
bool
CheckTupleIdentifier( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "tid" ));
}

/*
2.10 ~Cast~-Function

*/
void* CastTupleIdentifier( void* addr )
{
  return new (addr) TupleIdentifier;
}
/*
2.6 Creation of the Type Constructor Instance

*/
TypeConstructor tupleIdentifier(
	"tid",			//name
	TupleIdentifierProperty, 	        //property function describing signature
    OutTupleIdentifier, InTupleIdentifier,            //Out and In functions
    0, 0,	                        //SaveToList and RestoreFromList functions
	CreateTupleIdentifier, DeleteTupleIdentifier,	//object creation and deletion
    0, 0, CloseTupleIdentifier, CloneTupleIdentifier, //object open, save, close, and clone
	CastTupleIdentifier,			//cast function
    SizeOfTupleIdentifier, 			//sizeof function
	CheckTupleIdentifier,	                //kind checking function
	0, 				//predef. pers. function for model
    TypeConstructor::DummyInModel,
    TypeConstructor::DummyOutModel,
    TypeConstructor::DummyValueToModel,
    TypeConstructor::DummyValueListToModel );

/*
5 Creating the Algebra

*/

class TupleIdentifierAlgebra : public Algebra
{
 public:
  TupleIdentifierAlgebra() : Algebra()
  {
    AddTypeConstructor( &tupleIdentifier );
    tupleIdentifier.AssociateKind( "DATA" );
    
    
  }
  ~TupleIdentifierAlgebra() {};
};

TupleIdentifierAlgebra tupleIdentifierAlgebra;

/*
6 Initialization

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
InitializeTupleIdentifierAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&tupleIdentifierAlgebra);
}


