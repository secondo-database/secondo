/*
//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]

[1] Implementation of the Spatial Algebra

February, 2002. Victor Teixeira de Almeida

1 Overview

This implementation file essentially contains the implementation of the classes ~Point~, 
~Points~, ~Line~, and ~Region~ used in the Spatial Algebra. These classes
respectively correspond to the memory representation for the type constructors
~point~, ~points~, ~line~, and ~region~.  

For more detailed information see SpatialAlgebra.h.

2 Defines and Includes

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;

/*
3 Type investigation auxiliaries

Within this algebra module, we have to handle with values of four different
types: ~point~ and ~points~, ~line~ and ~region~.

Later on we will
examine nested list type descriptions. In particular, we
are going to check whether they describe one of the four types just introduced.
In order to simplify dealing with list expressions describing these types, we
declare an enumeration, ~SpatialType~, containing the four types, and a function,
~TypeOfSymbol~, taking a nested list as argument and returning the
corresponding ~SpatialType~ type name.

*/

enum SpatialType { stpoint, stpoints, stline, stregion, sterror };

static SpatialType
TypeOfSymbol( ListExpr symbol )
{
  if ( nl->AtomType( symbol ) == SymbolType )
  {
    string s = nl->SymbolValue( symbol );
    if ( s == "point"  ) return (stpoint);
    if ( s == "points" ) return (stpoints);
    if ( s == "line"   ) return (stline);
    if ( s == "region" ) return (stregion);
  }
  return (sterror);
}

/*
4 Type Constructor ~point~

A value of type ~point~ represents a point in the Euclidean plane or is undefined.

4.1 Implementation of the class ~Point~

*/
Point::Point( const bool d, const Coord& x, const Coord& y ) :
  x( x ),
  y( y ),
  defined( d )
{}

Point::Point( const Point& p ) :
  defined( p.IsDefined() )
{
  if( defined )
  {
    x = p.GetX();
    y = p.GetY();
  }
}

Point::~Point() 
{}

const Coord& Point::GetX() const 
{
  assert( IsDefined() );
  return x;
}

const Coord& Point::GetY() const
{
  assert( IsDefined() );
  return y;
}

const bool Point::IsDefined() const
{
  return defined;
}

Point& Point::operator=( const Point& p )
{
  assert( p.IsDefined() );
  defined = true;
  x = p.GetX();
  y = p.GetY();

  return *this;
}

void Point::SetDefined( const bool d )
{
  defined = d;
}

int Point::operator==( const Point& p ) const
{
  assert( IsDefined() && p.IsDefined() );
  return x == p.GetX() && y == p.GetY();
}

int Point::operator!=( const Point& p ) const
{
  assert( IsDefined() && p.IsDefined() );
  return x != p.GetX() || y == p.GetY();
}

int Point::operator<=( const Point& p ) const
{
  assert( IsDefined() && p.IsDefined() );
  if( x < p.GetX() )
    return 1;
  else if( x == p.GetX() && y <= p.GetY() )
    return 1;
  return 0;
}

int Point::operator<( const Point& p ) const
{
  assert( IsDefined() && p.IsDefined() );
  if( x < p.GetX() )
    return 1;
  else if( x == p.GetX() && y < p.GetY() )
    return 1;
  return 0;
}

int Point::operator>=( const Point& p ) const
{
  assert( IsDefined() && p.IsDefined() );
  if( x > p.GetX() )
    return 1;
  else if( x == p.GetX() && y >= p.GetY() )
    return 1;
  return 0;
}

int Point::operator>( const Point& p ) const
{
  assert( IsDefined() && p.IsDefined() );
  if( x > p.GetX() )
    return 1;
  else if( x == p.GetX() && y > p.GetY() )
    return 1;
  return 0;
}

ostream& operator<<( ostream& o, const Point& p )
{
  if( p.IsDefined() )
    o << "(" << p.GetX() << ", " << p.GetY() << ")";
  else
    o << "undef";

  return o;
}

const bool Point::Inside( const Points& ps ) const
{
  assert( IsDefined() && ps.IsOrdered() );

  return ps.Contains( *this );
}

void Point::Intersection( const Point& p, Point& result ) const
{
  assert( IsDefined() && p.IsDefined() );

  if( *this == p )
    result = *this;
  else
    result.SetDefined( false );
}

void Point::Intersection( const Points& ps, Point& result ) const
{
  assert( IsDefined() );

  if( this->Inside( ps ) )
    result = *this;
  else
    result.SetDefined( false );
}

void Point::Minus( const Point& p, Point& result ) const
{
  assert( IsDefined() && p.IsDefined() );

  if( *this == p )
    result.SetDefined( false );
  else
    result = *this;
}

void Point::Minus( const Points& ps, Point& result ) const
{
  assert( IsDefined() );

  if( this->Inside( ps ) )
    result.SetDefined( false );
  else
    result = *this;
}


/*
4.2 List Representation

The list representation of a point is

----	(x y)
----

4.3 ~Out~-function

*/
static ListExpr
OutPoint( ListExpr typeInfo, Word value )
{
  Point* point = (Point*)(value.addr);
  if( point->IsDefined() )
  {
    return nl->TwoElemList(
             point->GetX().IsInteger() ? nl->IntAtom( point->GetX().IntValue() ) : nl->RealAtom( point->GetX().Value() ), 
             point->GetY().IsInteger() ? nl->IntAtom( point->GetY().IntValue() ) : nl->RealAtom( point->GetY().Value() ) );
  }
  else
  {
    return (nl->SymbolAtom("undef"));
  }
}

/*
4.4 ~In~-function

*/
static Word
InPoint( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Point* newpoint;

  if ( nl->ListLength( instance ) == 2 )
  { 
    ListExpr First = nl->First(instance); 
    ListExpr Second = nl->Second(instance);

    if ( nl->IsAtom(First) && nl->IsAtom(Second) )
    {
      Coord x, y;

      correct = true;
      if( nl->AtomType(First) == IntType )
        x = nl->IntValue(First);
      else if( nl->AtomType(First) == RealType )
        x = nl->RealValue(First);
      else
        correct = false; 

      if( nl->AtomType(Second) == IntType )
        y = nl->IntValue(Second);
      else if( nl->AtomType(Second) == RealType )
        y = nl->RealValue(Second);
      else
        correct = false; 

      if( correct )
      {
        newpoint = new Point(true, x, y);
        return SetWord(newpoint);
      }
    }
  }
  correct = false;
  return SetWord(Address(0));
}

/*
4.5 ~Create~-function

*/
static Word
CreatePoint( const ListExpr typeInfo ) 
{
  return (SetWord( new Point() ));
}

/*
4.6 ~Delete~-function

*/
static void
DeletePoint( Word& w ) 
{
  delete (Point *)w.addr;
  w.addr = 0;
}
 
/*
4.7 ~Close~-function

*/
static void
ClosePoint( Word& w ) 
{
  delete (Point *)w.addr;
  w.addr = 0;
}
 
/*
4.8 ~Clone~-function

*/
static Word
ClonePoint( const Word& w ) 
{
  assert( ((Point *)w.addr)->IsDefined() ); 
  Point *p = new Point( *((Point *)w.addr) );
  return SetWord( p );
}
 
/*
4.8 ~SizeOf~-function

*/
static int
SizeOfPoint() 
{
  return sizeof(Point);
}

/*
4.9 Function describing the signature of the type constructor

*/
static ListExpr
PointProperty()
{
  return (nl->TwoElemList(
		nl->TheEmptyList(),
		nl->SymbolAtom("SPATIAL") ));
}

/*
4.10 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~point~ does not have arguments, this is trivial.

*/
static bool
CheckPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "point" ));
}

/*
4.11 ~Cast~-function

*/
void* CastPoint(void* addr, SmiRecordFile*)
{
  return ( 0 );
}

/*
4.12 Creation of the type constructor instance

*/
TypeConstructor point(
	"point",			//name		
	PointProperty,			//property function describing signature
        OutPoint,   	InPoint,	//Out and In functions
	CreatePoint,	DeletePoint,	//object creation and deletion
        0, 0, ClosePoint, ClonePoint,   //object open, save, close, and clone
	CastPoint,			//cast function
        SizeOfPoint,                    //sizeof function
	CheckPoint,	                //kind checking function
	0, 				//predef. pers. function for model
        TypeConstructor::DummyInModel, 	
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );


/*
5 Type Constructor ~points~

A ~points~ value is a finite set of points. 

5.1 Implementation of the class ~Points~

*/
Points::Points() :
  points( new DBArray<Point>( SecondoSystem::GetFlobFile() ) ),
  ordered( true )
{}

Points::Points( const Points& ps ) :
  points( new DBArray<Point>( SecondoSystem::GetFlobFile() ) ), 
  ordered( true )
{
  assert( ps.IsOrdered() );

  for( int i = 0; i < ps.Size(); i++ )
  {
    Point p;
    ps.Get( i, p );
    points->Put( i, p ); 
  }
}

Points::Points( const SmiRecordId recordId, bool update ):
  points( new DBArray<Point>( SecondoSystem::GetFlobFile(), recordId, update ) ),
  ordered( true )
  {}

void Points::Destroy()
{
  points->MarkDelete();
}

Points::~Points() 
{
  delete points;
}

void Points::Get( const int i, Point& p ) const
{
  assert( i >= 0 && i < Size() );

  points->Get( i, p );
}

const int Points::Size() const 
{
  return points->GetNoComponents();
}

const bool Points::IsEmpty() const
{
  return Size() == 0;
}

const int Points::Position( const Point& p ) const
{
  assert( IsOrdered() && p.IsDefined() );

  int first = 0, last = Size();

  while (first <= last) 
  {
    int mid = ( first + last ) / 2;  
    Point midPoint;
    points->Get( mid, midPoint );
    if( p > midPoint ) 
      first = mid + 1;  
    else if( p < midPoint ) 
      last = mid - 1; 
    else
      return mid;
   }
   return -1;
}

const SmiRecordId Points::GetRecordId() const
{
  return points->GetRecordId();
}

const bool Points::Save() 
{
  assert( IsOrdered() );
  return points->Save();
}

Points& Points::operator=( const Points& ps )
{
  assert( ps.IsOrdered() );

  points->MarkDelete();
  delete points;
  points = new DBArray<Point>( SecondoSystem::GetFlobFile() );
  for( int i = 0; i < ps.Size(); i++ )
  {
    Point p;
    ps.Get( i, p );
    points->Put( i, p );
  }
  ordered = true;
  return *this;
}

void Points::StartBulkLoad()
{
  assert( IsOrdered() );
  ordered = false;
}

void Points::EndBulkLoad()
{
  assert( !IsOrdered() );
  Sort();
  ordered = true;
}

const bool Points::IsOrdered() const
{
  return ordered;
}

int Points::operator==( const Points& ps ) const
{
  assert( IsOrdered() && ps.IsOrdered() );

  if( Size() != ps.Size() )
    return 0;

  for( int i = 0; i < Size(); i++ )
  {
    Point p1, p2;
    points->Get( i, p1 );
    ps.Get( i, p2 );
    if( p1 != p2 )
      return 0;
  }
  return 1;
}

int Points::operator!=( const Points& ps ) const
{
  assert( IsOrdered() && ps.IsOrdered() );

  return !( *this == ps );
}

Points& Points::operator+=(const Point& p)
{
  assert( p.IsDefined() );

  if( !IsOrdered() )
  {
    points->Put( Size(), p );
  }
  else
  {
    int pos = Position( p );
    if( pos != -1 )
    {
      for( int i = Size() - 1; i >= pos; i++ )
      {
        Point auxp;
        points->Get( i, auxp );
        points->Put( i+1, auxp );
      }
      points->Put( pos, p );
    }
  }
  return *this;
}

Points& Points::operator+=(const Points& ps)
{
  for( int i = 0; i < ps.Size(); i++ )
  {
    Point p;
    ps.Get( i, p );
    points->Put( Size(), p );
  }
  if( IsOrdered() )
  {
    ordered = false;
    Sort();
  }

  return *this;
}

Points& Points::operator-=(const Point& p)
{
  assert( IsOrdered() && p.IsDefined() );

  int pos = Position( p );
  if( pos != -1 )
  {
    for( int i = pos; i < Size(); i++ )
    {
      Point auxp;
      points->Get( i+1, auxp );
      points->Put( i, auxp );
    }
  }
  return *this;
}

ostream& operator<<( ostream& o, const Points& ps )
{
  o << "<";
  for( int i = 0; i < ps.Size(); i++ )
  {
    Point p;
    ps.Get( i, p );
    o << " " << p; 
  }
  o << " >";

  return o;
}

void Points::Sort()
{
  assert( !IsOrdered() );

  if( Size() > 1 ) 
  {
    int low = 0, high = Size() - 1;
    QuickSortRecursive( low, high );
  }
}

void Points::QuickSortRecursive( const int low, const int high )
{
  int i = high, j = low;
  Point p, pj, pi;

  Get( (int)( (low + high) / 2 ), p );

  do 
  {
    Get( j, pj );
    while( pj < p ) 
      Get( ++j, pj );

    Get( i, pi );
    while( pi > p ) 
      Get( --i, pi );

    if( i >= j ) 
    {
      if ( i != j ) 
      {
        points->Put( i, pj );
        points->Put( j, pi );
      }

      i--;
      j++;
    }
  } while( j <= i );

  if( low < i ) 
    QuickSortRecursive( low, i );
  if( j < high ) 
    QuickSortRecursive( j, high );
}

const bool Points::Contains( const Point& p ) const
{
  assert( IsOrdered() && p.IsDefined() );

  if( IsEmpty() )
    return false;

  int first = 0, last = Size() - 1;

  while (first <= last) 
  {
    int mid = ( first + last ) / 2;  
    Point midPoint;
    points->Get( mid, midPoint );
    if( p > midPoint ) 
      first = mid + 1;  
    else if( p < midPoint ) 
      last = mid - 1; 
    else
      return true;
   }
   return false;
}

const bool Points::Contains( const Points& ps ) const
{
  assert( IsOrdered() && ps.IsOrdered() );

  if( ps.IsEmpty() )
    return true;
  if( IsEmpty() )
    return false;

  Point p1, p2;
  int i = 0, j = 0;

  Get( i, p1 );
  ps.Get( j, p2 );
  while( true )
  {
    if( p1 == p2 )
    {
      if( ++j == ps.Size() )
        return true;
      ps.Get( j, p2 );
      if( ++i == Size() )
        return false;
      Get( i, p1 );
    }
    else if( p1 < p2 )
    {
      if( ++i == Size() )
        return false;
      Get( i, p1 );
    }
    else // p1 > p2 
    {
      return false;
    }
  }
  // This part of the code should never be reached.
  assert( true );
  return true;
}

const bool Points::Inside( const Points& ps ) const
{
  assert( IsOrdered() && ps.IsOrdered() );

  return ps.Contains( *this );
}

const bool Points::Intersects( const Points& ps ) const
{
  assert( IsOrdered() && ps.IsOrdered() );

  if( IsEmpty() || ps.IsEmpty() )
    return false;

  Point p1, p2;
  int i = 0, j = 0;

  Get( i, p1 );
  ps.Get( j, p2 );

  while( 1 )
  {
    if( p1 == p2 )
      return true;
    if( p1 < p2 )
    {
      if( ++i == Size() )
        return false;
      Get( i, p1 );
    }
    else // p1 > p2 
    {
      if( ++j == ps.Size() )
        return false;
      ps.Get( j, p2 );
    }
  }
  // this part of the code should never be reached
  assert( false );
  return false;
}

/*
5.2 List Representation

The list representation of a point is

----	(x y)
----

5.3 ~Out~-function

*/
static ListExpr
OutPoints( ListExpr typeInfo, Word value )
{
  Points* points = (Points*)(value.addr);
  if( points->IsEmpty() )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    Point p;
    points->Get( 0, p );
    ListExpr result = nl->OneElemList( OutPoint( nl->TheEmptyList(), SetWord( &p ) ) );
    ListExpr last = result;

    for( int i = 1; i < points->Size(); i++ )
    {
      points->Get( i, p );
      last = nl->Append( last,
                         OutPoint( nl->TheEmptyList(), SetWord( &p ) ) );
    }  

    return result;
  }
}

/*
5.4 ~In~-function

*/
static Word
InPoints( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Points* points = new Points();
  points->StartBulkLoad();

  ListExpr rest = instance;
  while( !nl->IsEmpty( rest ) )
  {
    ListExpr first = nl->First( rest );
    rest = nl->Rest( rest );

    Point *p = (Point*)InPoint( nl->TheEmptyList(), first, 0, errorInfo, correct ).addr;
    if( correct ) 
    {
      (*points) += (*p);
      delete p;
    }
    else
    {
      return SetWord( Address(0) );
    }
  }
  points->EndBulkLoad();
  correct = true;
  return SetWord( points );
}

/*
5.5 ~Create~-function

*/
static Word
CreatePoints( const ListExpr typeInfo ) 
{
  return (SetWord( new Points() ));
}

/*
5.6 ~Delete~-function

*/
static void
DeletePoints( Word& w ) 
{
  Points *ps = (Points *)w.addr;
  ps->Destroy();
  delete ps;
  w.addr = 0;
}
 
/*
5.7 ~Close~-function

*/
static void
ClosePoints( Word& w ) 
{
  delete (Points *)w.addr;
  w.addr = 0;
}
 
/*
5.8 ~Clone~-function

*/
static Word
ClonePoints( const Word& w ) 
{
  Points *p = new Points( *((Points *)w.addr) );
  return SetWord( p );
}
 
/*
5.8 ~SizeOf~-function

Not implemented yet.

*/
static int
SizeOfPoints() 
{
  return 0;
}

/*
5.9 ~Open~-function

*/
bool
OpenPoints( SmiRecord& valueRecord,
            const ListExpr typeInfo,
            Word& value )
{
  SmiRecordId recordId;

  valueRecord.Read( &recordId, sizeof( SmiRecordId ), 0 );
  Points *points = new Points( recordId );
  value = SetWord( points );

  return (true);
}

/*
5.10 ~Save~-function

*/
bool
SavePoints( SmiRecord& valueRecord,
            const ListExpr typeInfo,
            Word& value )
{
  Points *points = (Points*)value.addr;

  points->Save();
  SmiRecordId recordId = points->GetRecordId();
  valueRecord.Write( &recordId, sizeof( SmiRecordId ), 0 );

  return (true);
}

/*
5.11 Function describing the signature of the type constructor

*/
static ListExpr
PointsProperty()
{
  return (nl->TwoElemList(
		nl->TheEmptyList(),
		nl->SymbolAtom("SPATIAL") ));
}

/*
5.12 Kind checking function

This function checks whether the type constructor is applied correctly. Since
type constructor ~point~ does not have arguments, this is trivial.

*/
static bool
CheckPoints( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "points" ));
}

/*
5.13 ~Cast~-function

*/
void* CastPoints(void* addr, SmiRecordFile*)
{
  return ( 0 );
}

/*
5.14 Creation of the type constructor instance

*/
TypeConstructor points(
	"points",			//name		
	PointsProperty, 		//property function describing signature
        OutPoints,   	InPoints,	//Out and In functions
	CreatePoints,	DeletePoints,	//object creation and deletion
        OpenPoints, 	SavePoints,    	// object open and save
        ClosePoints, 	ClonePoints,   	//object close and clone
	CastPoints,			//cast function
        SizeOfPoints,                   //sizeof function
	CheckPoints,	                //kind checking function
	0, 				//predef. pers. function for model
        TypeConstructor::DummyInModel, 	
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );


/*
6 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

6.1 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

6.1.1 Type mapping function SpatialTypeMapBool

It is for the compare operators which have ~bool~ as resulttype, like =, !=, <, 
<=, >, >=.

*/
static ListExpr
SpatialTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == stpoint && TypeOfSymbol( arg2 ) == stpoint)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stpoints && TypeOfSymbol( arg2 ) == stpoints)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stline && TypeOfSymbol( arg2 ) == stline)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stregion && TypeOfSymbol( arg2 ) == stregion)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stpoint && TypeOfSymbol( arg2 ) == stpoints)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stpoints && TypeOfSymbol( arg2 ) == stpoint)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stpoint && TypeOfSymbol( arg2 ) == stline)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stpoint && TypeOfSymbol( arg2 ) == stregion)
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
6.1.2 Type mapping function SpatialTypeMapBool1

It is for the operator ~isempty~ which have ~point~, ~points~, ~line~, and ~region~ as input and ~bool~ resulttype.

*/

static ListExpr
SpatialTypeMapBool1( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( TypeOfSymbol( arg1 ) == stpoint )
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stpoints )
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stline )
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == stregion )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
6.1.3 The dummy model mapping:

*/
static Word
SpatialNoModelMapping( ArgVector arg, Supplier opTreeNode )
{
  return (SetWord( Address( 0 ) ));
}

/*
6.2 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

6.2.2 Selection function ~SpatialSelectIsEmpty~

It is used for the ~isempty~ operator

*/
static int
SpatialSelectIsEmpty( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  if ( TypeOfSymbol( arg1 ) == stpoint )
    return (0);
  if ( TypeOfSymbol( arg1 ) == stpoints )
    return (1);
  if ( TypeOfSymbol( arg1 ) == stline )
    return (2);
  if ( TypeOfSymbol( arg1 ) == stregion )
    return (3);
  return (-1); // This point should never be reached
}

/*
6.2.3 Selection function ~SpatialSelectCompare~

It is used for compare operators ($=$, $\neq$, $<$, $>$, $\geq$, $\leq$)

*/
static int
SpatialSelectCompare( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );
  if ( TypeOfSymbol( arg1 ) == stpoint && TypeOfSymbol( arg2 ) == stpoint )
    return (0);
  if ( TypeOfSymbol( arg1 ) == stpoints && TypeOfSymbol( arg2 ) == stpoints )
    return (1);
  if ( TypeOfSymbol( arg1 ) == stline && TypeOfSymbol( arg2 ) == stline )
    return (2);
  if ( TypeOfSymbol( arg1 ) == stregion && TypeOfSymbol( arg2 ) == stregion )
    return (3);
  return (-1); // This point should never be reached
}

/*
6.2.4 Selection function ~SpatialSelectSets1~

It is used for set operators (~intersects~)

*/
static int
SpatialSelectSets1( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );
  if ( TypeOfSymbol( arg1 ) == stpoints && TypeOfSymbol( arg2 ) == stpoints )
    return (0);
  if ( TypeOfSymbol( arg1 ) == stline && TypeOfSymbol( arg2 ) == stline )
    return (1);
  if ( TypeOfSymbol( arg1 ) == stregion && TypeOfSymbol( arg2 ) == stregion )
    return (2);
  return (-1); // This point should never be reached
}

/*
6.2.5 Selection function ~SpatialSelectSets2~

It is used for set operators (~inside~) that allow the first argument to be
simple.

*/
static int
SpatialSelectSets2( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );
  if ( TypeOfSymbol( arg1 ) == stpoints && TypeOfSymbol( arg2 ) == stpoints )
    return (0);
  if ( TypeOfSymbol( arg1 ) == stline && TypeOfSymbol( arg2 ) == stline )
    return (1);
  if ( TypeOfSymbol( arg1 ) == stregion && TypeOfSymbol( arg2 ) == stregion )
    return (2);
  if ( TypeOfSymbol( arg1 ) == stpoint && TypeOfSymbol( arg2 ) == stpoints )
    return (3);
  if ( TypeOfSymbol( arg1 ) == stpoint && TypeOfSymbol( arg2 ) == stline )
    return (4);
  if ( TypeOfSymbol( arg1 ) == stpoint && TypeOfSymbol( arg2 ) == stregion )
    return (5);
  return (-1); // This point should never be reached
}

/*
6.3 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are 
several value mapping functions, one for each possible combination of input
parameter types. 

6.3.1 Value mapping functions of operator ~isempty~

*/
static int
IsEmpty_p( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Point*)args[0].addr)->IsDefined() )
  {
    ((CcBool*)result.addr)->Set( true, false );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, true );
  }
  return (0);
}

static int
IsEmpty_ps( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Points*)args[0].addr)->IsEmpty() )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

static int
IsEmpty_l( Word* args, Word& result, int message, Word& local, Supplier s )
{
  return (0);
}

static int
IsEmpty_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  return (0);
}

/*
6.3.2 Value mapping functions of operator ~$=$~

*/
static int
SpatialEqual_pp( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((Point*)args[0].addr)->IsDefined() &&
       ((Point*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, *((Point*)args[0].addr) == *((Point*)args[1].addr) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

static int
SpatialEqual_psps( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool *)result.addr)->Set( true, *((Points*)args[0].addr) == *((Points*)args[1].addr) );
  return (0);
}

static int
SpatialEqual_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{
  ((CcBool *)result.addr)->Set( false, false );
  return (0);
}

static int
SpatialEqual_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  ((CcBool *)result.addr)->Set( false, false );
  return (0);
}

/*
6.3.3 Value mapping functions of operator ~$\neq$~

*/
static int
SpatialNotEqual_pp( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((Point*)args[0].addr)->IsDefined() &&
       ((Point*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, *((Point*)args[0].addr) != *((Point*)args[1].addr) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

static int
SpatialNotEqual_psps( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool *)result.addr)->Set( true, *((Points*)args[0].addr) != *((Points*)args[1].addr) );
  return (0);
}

static int
SpatialNotEqual_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{
  ((CcBool *)result.addr)->Set( false, false );
  return (0);
}

static int
SpatialNotEqual_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  ((CcBool *)result.addr)->Set( false, false );
  return (0);
}

/*
6.3.4 Value mapping functions of operator ~$<$~

*/
static int
SpatialLess_pp( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((Point*)args[0].addr)->IsDefined() &&
       ((Point*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, *((Point*)args[0].addr) < *((Point*)args[1].addr) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
6.3.5 Value mapping functions of operator ~$\leq$~

*/
static int
SpatialLessEqual_pp( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((Point*)args[0].addr)->IsDefined() &&
       ((Point*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, *((Point*)args[0].addr) <= *((Point*)args[1].addr) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
6.3.6 Value mapping functions of operator ~$>$~

*/
static int
SpatialGreater_pp( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((Point*)args[0].addr)->IsDefined() &&
       ((Point*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, *((Point*)args[0].addr) > *((Point*)args[1].addr) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
6.3.7 Value mapping functions of operator ~$\geq$~

*/
static int
SpatialGreaterEqual_pp( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((Point*)args[0].addr)->IsDefined() &&
       ((Point*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, *((Point*)args[0].addr) >= *((Point*)args[1].addr) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
6.3.8 Value mapping functions of operator ~intersects~

*/
static int
SpatialIntersects_psps( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool *)result.addr)->
    Set( true, ((Points*)args[0].addr)->Intersects( *((Points*)args[1].addr) ) );
  return (0);
}

static int
SpatialIntersects_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool *)result.addr)->Set( false, false );
  return (0);
}

static int
SpatialIntersects_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool *)result.addr)->Set( false, false );
  return (0);
}

/*
6.3.9 Value mapping functions of operator ~inside~

*/
static int
SpatialInside_psps( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool *)result.addr)->
    Set( true, ((Points*)args[0].addr)->Inside( *((Points*)args[1].addr) ) );
  return (0);
}

static int
SpatialInside_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool *)result.addr)->Set( false, false );
  return (0);
}

static int
SpatialInside_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool *)result.addr)->Set( false, false );
  return (0);
}

static int
SpatialInside_pps( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((Point*)args[0].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((Point*)args[0].addr)->Inside( *((Points*)args[1].addr) ) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

static int
SpatialInside_pl( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool *)result.addr)->Set( false, false );
  return (0);
}

static int
SpatialInside_pr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool *)result.addr)->Set( false, false );
  return (0);
}

/*
6.4 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of value
mapping functions for each operator. For nonoverloaded operators there is also such and array
defined, so it easier to make them overloaded.

*/
ValueMapping spatialisemptymap[] = { IsEmpty_p, IsEmpty_ps, IsEmpty_l, IsEmpty_r };
ValueMapping spatialequalmap[] = { SpatialEqual_pp, SpatialEqual_psps, SpatialEqual_ll, SpatialEqual_rr };
ValueMapping spatialnotequalmap[] = { SpatialNotEqual_pp, SpatialNotEqual_psps, SpatialNotEqual_ll, SpatialNotEqual_rr };
ValueMapping spatiallessmap[] = { SpatialLess_pp };
ValueMapping spatiallessequalmap[] = { SpatialLessEqual_pp };
ValueMapping spatialgreatermap[] = { SpatialGreater_pp };
ValueMapping spatialgreaterequalmap[] = { SpatialGreaterEqual_pp };
ValueMapping spatialintersectsmap[] = { SpatialIntersects_psps, SpatialIntersects_ll, SpatialIntersects_rr };
ValueMapping spatialinsidemap[] = { SpatialInside_psps, SpatialInside_ll, SpatialInside_rr, SpatialInside_pps, SpatialInside_pl, SpatialInside_pr };

ModelMapping spatialnomodelmap[] = { SpatialNoModelMapping, SpatialNoModelMapping, 
                                     SpatialNoModelMapping, SpatialNoModelMapping,
                                     SpatialNoModelMapping, SpatialNoModelMapping };

const string SpatialSpecIsEmpty = "(<text> point -> bool, points -> bool, line -> bool, region -> bool</text---><text> Returns whether the value is defined or not. </text--->)";
const string SpatialSpecEqual = "(<text> (point point) -> bool, (points points) -> bool, (line line) -> bool, (region region) -> bool</text---><text> Equal. </text--->)";
const string SpatialSpecNotEqual = "(<text> (point point) -> bool</text---><text> Not equal. </text--->)";
const string SpatialSpecLess = "(<text> (point point) -> bool</text---><text> Less than. </text--->)";
const string SpatialSpecLessEqual   = "(<text> (point point) -> bool</text---><text> Equal or less than. </text--->)";
const string SpatialSpecGreater = "(<text> (point point) -> bool</text---><text> Greater than. </text--->)";
const string SpatialSpecGreaterEqual = "(<text> (point point) -> bool</text---><text> Equal or greater than. </text--->)";
const string SpatialSpecIntersects = "(<text> (points points) -> bool, (line line) -> bool, (region region) -> bool</text---><text> Intersects. </text--->)";
const string SpatialSpecInside = "(<text> (points points) -> bool, (line line) -> bool, (region region) -> bool, (point points) -> bool, (point line) -> bool, (point region) -> bool</text---><text> Inside. </text--->)";

Operator spatialisempty( "isempty", SpatialSpecIsEmpty, 4, spatialisemptymap, spatialnomodelmap, SpatialSelectIsEmpty, SpatialTypeMapBool1 );
Operator spatialequal( "=", SpatialSpecEqual, 4, spatialequalmap, spatialnomodelmap, SpatialSelectCompare, SpatialTypeMapBool );
Operator spatialnotequal( "#", SpatialSpecNotEqual, 4, spatialnotequalmap, spatialnomodelmap, SpatialSelectCompare, SpatialTypeMapBool );
Operator spatialless( "<", SpatialSpecLess, 1, spatiallessmap, spatialnomodelmap, Operator::SimpleSelect, SpatialTypeMapBool );
Operator spatiallessequal( "<=", SpatialSpecLessEqual, 1, spatiallessequalmap, spatialnomodelmap, Operator::SimpleSelect, SpatialTypeMapBool );
Operator spatialgreater( ">", SpatialSpecGreater, 1, spatialgreatermap, spatialnomodelmap, Operator::SimpleSelect, SpatialTypeMapBool );
Operator spatialgreaterequal( ">=", SpatialSpecGreaterEqual, 1, spatialgreaterequalmap, spatialnomodelmap, Operator::SimpleSelect, SpatialTypeMapBool );
Operator spatialintersects( "intersects", SpatialSpecIntersects, 3, spatialintersectsmap, spatialnomodelmap, SpatialSelectSets1, SpatialTypeMapBool );
Operator spatialinside( "inside", SpatialSpecInside, 6, spatialinsidemap, spatialnomodelmap, SpatialSelectSets2, SpatialTypeMapBool );

/*
7 Creating the Algebra

*/

class SpatialAlgebra : public Algebra
{
 public:
  SpatialAlgebra() : Algebra()
  {
    AddTypeConstructor( &point );
    AddTypeConstructor( &points );

    point.AssociateKind("DATA");   	//this means that point and rectangle
    points.AssociateKind("DATA");   	//can be used in places where types
    					//of kind DATA are expected, e.g. in
    					//tuples.

    AddOperator( &spatialisempty );
    AddOperator( &spatialequal );
    AddOperator( &spatialnotequal );
    AddOperator( &spatialless );
    AddOperator( &spatiallessequal );
    AddOperator( &spatialgreater );
    AddOperator( &spatialgreaterequal );
    AddOperator( &spatialintersects );
    AddOperator( &spatialinside );
  }
  ~SpatialAlgebra() {};
};

SpatialAlgebra spatialAlgebra; 

/*
8 Initialization

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
InitializeSpatialAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&spatialAlgebra);
}


