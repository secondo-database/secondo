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


[1] PointSequence Algebra

July 2005 Hoffmann

The algebra provides a type constructor ~pointsequence~.

1 Preliminaries

1.1 Includes

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"	//needed because we return a CcBool in an op.
#include <string>
#include "DBArray.h"
#include "SecondoSystem.h"
#include "Attribute.h"
#include "SpatialAlgebra.h"


extern NestedList* nl;
extern QueryProcessor *qp;

/*
2 Type Constructor ~pointsequence~

2.1 Data Structure - Class ~PointSequence~

*/

struct PSPoint {
  PSPoint() {}
/*
Do not use this constructor.

*/

 PSPoint( double xcoord, double ycoord ):
    x( xcoord ), y( ycoord )
    {}

  double x;
  double y;
};

class PointSequence : public Attribute
{
 public:
    PointSequence() {}
/*
This constructor should not be used.

*/

   PointSequence( const int n, const double *X = 0, const double *Y = 0 );
   ~PointSequence();

   int NumOfFLOBs();
   FLOB *GetFLOB(const int i);
   int Compare(Attribute*);
   bool Adjacent(Attribute*);
   int Sizeof();
   bool IsDefined() const;
   void SetDefined( bool defined );
   ostream& Print( ostream& os );
    
   int GetNoPSPoints();
   PSPoint& GetPSPoint( int i );
   const bool IsEmpty();
   void Append( PSPoint& p);
   PointSequence *Clone();

   friend ostream& operator <<( ostream& os, PointSequence& ps );
 
 private:
 
   DBArray<PSPoint> pspoints;
  
};
/*
2.3.18 Print functions

*/
ostream& operator<<(ostream& os, PSPoint& p)
{
  os << "(" << p.x << "," << p.y << ")";
  return os;
}

ostream& operator<<(ostream& os, PointSequence& ps)
{
  os << "<";
  for(int i = 0; i < ps.GetNoPSPoints(); i++)
    os << ps.GetPSPoint( i ) << " ";
  os << ">";
  return os;
}

/*
2.3.1 Constructors.

This first constructor creates a new pointsequence.

*/
PointSequence::PointSequence( const int n, const double *X, const double *Y ) :
  pspoints( n )
{
  if( n > 0 )
  {
    for( int i = 0; i < n; i++ )
    {
      PSPoint p( X[i], Y[i] );
      Append( p );
    }
  }
}

/*

2.3.2 Destructor.

*/
PointSequence::~PointSequence()
{
}

/*
2.3.3 NumOfFLOBs.

Not yet implemented. Needed to be a tuple attribute.

*/
int PointSequence::NumOfFLOBs()
{
  return 1;
}

/*
2.3.4 GetFLOB

Not yet implemented. Needed to be a tuple attribute.

*/
FLOB *PointSequence::GetFLOB(const int i)
{
  assert( i >= 0 && i < NumOfFLOBs() );
  return &pspoints;
}

/*
2.3.5 Compare

Not yet implemented. Needed to be a tuple attribute.

*/
int PointSequence::Compare(Attribute*)
{
  return 0;
}

/*
2.3.5 Adjacent

Not yet implemented. Needed to be a tuple attribute.

*/
bool PointSequence::Adjacent(Attribute*)
{
  return 0;
}

/*
2.3.8 IsDefined

*/
bool PointSequence::IsDefined() const
{
  return true;
}

/*
2.3.8 SetDefined

*/
void PointSequence::SetDefined( bool defined )
{
}

/*
2.3.8 Print

*/
ostream& PointSequence::Print( ostream& os )
{
  return (os << *this);
}

/*
2.3.14 NoPSPoints

Returns the number of points of the point sequence.

*/
int PointSequence::GetNoPSPoints()
{
  return pspoints.Size();
}

/*
2.3.15 GetPSPoint

Returns a PSPoint indexed by ~i~.

*Precondition* ~0 <= i < noPSPoints~.

*/
PSPoint& PointSequence::GetPSPoint( int i )
{
  assert( 0 <= i && i < GetNoPSPoints() );

  static PSPoint p;
  pspoints.Get( i, p );

  return p;
}

/*
2.3.18 IsEmpty

Returns if the point sequence list is empty or not.

*/
const bool PointSequence::IsEmpty()
{
  return GetNoPSPoints() == 0;
}

/*
2.3.9 Append

Appends a pspoint ~p~ at the end of the point sequence list.

*/
void PointSequence::Append( PSPoint& p )
{
  pspoints.Append( p );
}

/*
2.3.7 Clone

Returns a new created point sequence list (clone) which is a
copy of ~this~.

*/
PointSequence *PointSequence::Clone()
{
  PointSequence *ps = new PointSequence( 0 );
  for( int i = 0; i < GetNoPSPoints(); i++ )
    ps->Append( this->GetPSPoint( i ) );
  return ps;
}

/*
2.2 List Representation

The list representation of a point sequence list is

----	((x1 y1)...(xn yn))
----

2.3 ~In~ and ~Out~ Functions

*/

ListExpr
OutPointSequence( ListExpr typeInfo, Word value )
{
  PointSequence* ps = (PointSequence*)(value.addr);

  if( ps->IsEmpty() )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    ListExpr result = nl->OneElemList( nl->TwoElemList( nl->RealAtom( ps->GetPSPoint(0).x ), nl->RealAtom( ps->GetPSPoint(0).y ) ) );
    ListExpr last = result;

    for( int i = 1; i < ps->GetNoPSPoints(); i++ )
    {
      last = nl->Append( last,
                         nl->TwoElemList( nl->RealAtom( ps->GetPSPoint(i).x ), nl->RealAtom( ps->GetPSPoint(i).y ) ) );
    }
    return result;
  }
}

Word
InPointSequence( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{
  cout << "InPointSequence" << endl;
  PointSequence* ps;

  ps = new PointSequence( 0 );

  ListExpr first;
  ListExpr rest = instance;
  while( !nl->IsEmpty( rest ) )
  {
    first = nl->First( rest );
    rest = nl->Rest( rest );

    if( nl->ListLength( first ) == 2 &&
        nl->IsAtom( nl->First( first ) ) && nl->AtomType( nl->First( first ) ) == RealType &&
        nl->IsAtom( nl->Second( first ) ) && nl->AtomType( nl->Second( first ) ) == RealType )
    {
      PSPoint p( nl->RealValue( nl->First( first ) ), nl->RealValue( nl->Second( first ) ) );
      ps->Append( p );
    }
    else
    {
      correct = false;
      return SetWord( Address(0) );
    }
  }
  correct = true;
  return SetWord( ps );
}

/*
2.4 Functions Describing the Signature of the Type Constructors

This one works for type constructors ~PointSequence~.

*/
ListExpr
PointSequenceProperty()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
	                     nl->StringAtom("Example Type List"),
			     nl->StringAtom("List Rep"),
			     nl->StringAtom("Example List"),
			     nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
	                     nl->StringAtom("pointsequence"),
			     nl->StringAtom("((<x1> <y1>)...(<xn> <yn>))"),
			     nl->StringAtom("((3.2 15.4)(6.34 20.8))"),
			     nl->StringAtom("x- and y-coordinates must be "
			     "of type real."))));
}

/*
3.4 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~pointsequence~ does not have arguments, this is trivial.

*/
bool
CheckPointSequence( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "pointsequence" ));
}

/*

3.5 ~Create~-function

*/
Word CreatePointSequence(const ListExpr typeInfo)
{
  PointSequence* ps = new PointSequence( 0 );
  return ( SetWord(ps) );
}

/*
3.6 ~Delete~-function

*/
void DeletePointSequence(Word& w)
{
  PointSequence* ps = (PointSequence*)w.addr;
  delete ps;
}

/*
3.6 ~Open~-function

*/
bool
OpenPointSequence( SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value )
{
  PointSequence *ps = (PointSequence*)TupleElement::Open( valueRecord, offset, typeInfo );
  value = SetWord( ps );
  return true;
}

/*
3.7 ~Save~-function

*/
bool
SavePointSequence( SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value )
{
  PointSequence *ps = (PointSequence *)value.addr;
  
  { TupleElement::Save( valueRecord, offset, typeInfo, ps ); }
  
  return true;
}

/*
3.8 ~Close~-function

*/
void ClosePointSequence(Word& w)
{
  PointSequence* ps = (PointSequence*)w.addr;
  delete ps;
}

/*
3.9 ~Clone~-function

*/
Word ClonePointSequence(const Word& w)
{
  return SetWord( ((PointSequence*)w.addr)->Clone() );
}

/*
3.9 ~SizeOf~-function

*/
int SizeOfPointSequence()
{
  return sizeof(PointSequence);
}

/*
3.10 ~Cast~-function

*/
void* CastPointSequence(void* addr)
{
  return (new (addr) PointSequence);
}

/*
2.6 Creation of the Type Constructor Instance

*/
TypeConstructor pointsequence(
        "pointsequence",			   //name
        PointSequenceProperty,		           //property function describing signature
        OutPointSequence,InPointSequence,	   //Out and In functions
        0, 0,                                      //SaveToList and RestoreFromList functions
        CreatePointSequence,  DeletePointSequence, //object creation and deletion
        OpenPointSequence,    SavePointSequence,   //object open and save
        ClosePointSequence,   ClonePointSequence,  //object close and clone
        CastPointSequence,                   	   //cast function
        SizeOfPointSequence,                       //sizeof function
        CheckPointSequence,			   //kind checking function
        0,					   //predefined persistence function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );
	
/*
4 Creating Operators

4.1 Type Mapping Functions

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol
~typeerror~.

*/

ListExpr
C2PointTypeMap( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "pointsequence") )
    return nl->SymbolAtom("point");
  }
  return nl->SymbolAtom("typeerror");
}

ListExpr
C2PointsTypeMap( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "pointsequence") )
    return nl->SymbolAtom("points");
  }
  return nl->SymbolAtom("typeerror");
}

ListExpr
C2LineTypeMap( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "pointsequence") )
    return nl->SymbolAtom("line");
  }
  return nl->SymbolAtom("typeerror");
}


/*
4.3 Value Mapping Functions

*/
int
C2PointFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Converts ~pointsequence~ to ~point~.

*/
{
  PointSequence *ps = ((PointSequence*)args[0].addr);

  if ( ps->GetNoPSPoints() == 1 ) {
    result = SetWord( new Point(true, ps->GetPSPoint(0).x, ps->GetPSPoint(0).y) );
  }
  return 0;
}

int
C2PointsFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Converts ~pointsequence~ to ~points~.

*/
{
  PointSequence *ps = ((PointSequence*)args[0].addr);

  if ( ps->GetNoPSPoints() >= 1 ) {
    
    Points *pts = new Points( ps->GetNoPSPoints() );
    pts->StartBulkLoad();
    for( int i = 0; i < ps->GetNoPSPoints(); i++ )
    {
      Point p(true, ps->GetPSPoint(i).x, ps->GetPSPoint(i).y);      
      pts->InsertPt( p );
    }
    pts->EndBulkLoad();
    result = SetWord( pts );
  }
  return 0;
}

int
C2LineFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Converts ~pointsequence~ to ~line~.

*/
{
  PointSequence *ps = ((PointSequence*)args[0].addr);

  if ( ps->GetNoPSPoints() >= 2 ) {
    
    CLine *l = new CLine( (ps->GetNoPSPoints() - 1) * 2 );
    l->StartBulkLoad();
    for( int i = 0; i < (ps->GetNoPSPoints() - 1); i++ )
    {
      Point p1(true, ps->GetPSPoint(i).x, ps->GetPSPoint(i).y);
      Point p2(true, ps->GetPSPoint(i+1).x, ps->GetPSPoint(i+1).y);
      CHalfSegment seg(true, true,  p1, p2);      
      *l += seg;
      seg.SetLDP(false);
      *l += seg;
    }
    l->EndBulkLoad();
    result = SetWord( l );
  }
  return 0;
}


/*
4.4 Specification of Operators

*/

const string C2PointSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>(pointsequence) -> point</text--->"
			 "<text>c2point( _ )</text--->"
			 "<text>Coverts a point sequence list"
                         " to point.</text--->"
			 "<text>let mypoint = c2point(mypointsequence)</text--->"
			 ") )";

const string C2PointsSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>(pointsequence) -> point</text--->"
			 "<text>c2point( _ )</text--->"
			 "<text>Coverts a point sequence list"
                         " to point.</text--->"
			 "<text>let mypoint = c2point(mypointsequence)</text--->"
			 ") )";

const string C2LineSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>(pointsequence) -> point</text--->"
			 "<text>c2point( _ )</text--->"
			 "<text>Coverts a point sequence list"
                         " to point.</text--->"
			 "<text>let mypoint = c2point(mypointsequence)</text--->"
			 ") )";

/*
4.5 Definition of Operators

*/

Operator c2point (
	"c2point", 		//name
	C2PointSpec,            //specification
	C2PointFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	Operator::SimpleSelect,	//trivial selection function
	C2PointTypeMap		//type mapping
);

Operator c2points (
	"c2points", 		//name
	C2PointsSpec,           //specification
	C2PointsFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	Operator::SimpleSelect,	//trivial selection function
	C2PointsTypeMap		//type mapping
);

Operator c2line (
	"c2line", 		//name
	C2LineSpec,             //specification
	C2LineFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	Operator::SimpleSelect,	//trivial selection function
	C2LineTypeMap		//type mapping
);

/*
5 Creating the Algebra

*/

class PointSequenceAlgebra : public Algebra
{
 public:
  PointSequenceAlgebra() : Algebra()
  {
    AddTypeConstructor( &pointsequence );

    //the lines below define that poin110tsequence
    //can be used in places where types of kind SIMPLE are expected
    pointsequence.AssociateKind("DATA");
    
    AddOperator ( &c2point );   	     
    AddOperator ( &c2points );  
    AddOperator ( &c2line ); 
    					  
  }
  ~PointSequenceAlgebra() {};
};

PointSequenceAlgebra pointSequenceAlgebra;

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
InitializePointSequenceAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&pointSequenceAlgebra);
}


