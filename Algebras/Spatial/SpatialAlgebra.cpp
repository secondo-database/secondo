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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the Spatial Algebra

February, 2003. Victor Teixeira de Almeida

March-July, 2003. Zhiming Ding

January, 2005 Leonardo Guerreiro Azevedo

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

[TOC]

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
#include <vector>
#include <iostream>
#include <string>
#include <cmath>

// gcc on windows before version 3.4 does not
// have defined M_PI.
#ifndef M_PI
const double M_PI = acos( -1.0 );
#endif

extern NestedList* nl;
extern QueryProcessor* qp;

/*
3 Auxiliary Functions

*/
bool AlmostEqual( const double d1, const double d2 )
{
  double factor=FACTOR;
  if ( abs(d1 + d2) < 1 )
    factor *= abs( d1 + d2 );

  if( abs(d1 - d2) <= factor )
    return true;
  return false;
}

bool AlmostEqual( const Point& p1, const Point& p2 )
{
  if( AlmostEqual( p1.GetX(), p2.GetX() ) &&
      AlmostEqual( p1.GetY(), p2.GetY() ) )
    return true;
  return false;
}

/*
3 Type investigation auxiliaries

Within this algebra module, we have to handle with values of four different
types: ~point~ and ~points~, ~line~ and ~region~.

Later on we will
examine nested list type descriptions. In particular, we
are going to check whether they describe one of the four types just introduced.
In order to simplify dealing with list expressions describing these types, we
declare an enumeration, ~SpatialType~, containing the four types, and a function,
~SpatialTypeOfSymbol~, taking a nested list as argument and returning the
corresponding ~SpatialType~ type name.

*/


//type necessary for CohenStutherlandLineClipAndDraw ("lineclipping") algorithm
typedef unsigned int outcode;
enum { TOP = 0x1, BOTTOM = 0x2, RIGHT = 0x4, LEFT = 0x8 };

enum SpatialType { stpoint, stpoints, stline, stregion, stbox, sterror };

SpatialType
SpatialTypeOfSymbol( ListExpr symbol )
{
  if ( nl->AtomType( symbol ) == SymbolType )
  {
    string s = nl->SymbolValue( symbol );
    if ( s == "point"  ) return (stpoint);
    if ( s == "points" ) return (stpoints);
    if ( s == "line"   ) return (stline);
    if ( s == "region" ) return (stregion);
    if ( s == "rect"   ) return (stbox);
  }
  return (sterror);
}

/*
4 Type Constructor ~point~

A value of type ~point~ represents a point in the Euclidean plane or is undefined.

4.1 Implementation of the class ~Point~

*/
ostream& operator<<( ostream& o, const Point& p )
{
  o << "(" << p.GetX() << ", " << p.GetY() << ")";
  return o;
}

ostream& Point::Print( ostream &os ) const
{
  return os << *this;
}

bool Point::Inside( const Points& ps ) const
{
  assert( defined && ps.IsOrdered() );

  return ps.Contains( *this );
}

void Point::Intersection( const Points& ps, Point& result ) const
{
  assert( defined && ps.IsOrdered() );

  if( this->Inside( ps ) )
    result = *this;
  else
    result.SetDefined( false );
}

void Point::Minus( const Points& ps, Point& result ) const
{
  assert( defined && ps.IsOrdered() );

  if( this->Inside( ps ) )
    result.SetDefined( false );
  else
    result = *this;
}

/*
4.2 List Representation

The list representation of a point is

----  (x y)
----

4.3 ~Out~-function

*/
ListExpr
OutPoint( ListExpr typeInfo, Word value )
{
  Point* point = (Point*)(value.addr);
  if( point->IsDefined() )
  {
#ifdef RATIONAL_COORDINATES
    return nl->TwoElemList(
             point->GetX().IsInteger() ? nl->IntAtom( point->GetX().IntValue() ) : nl->RealAtom( point->GetX().Value() ),
             point->GetY().IsInteger() ? nl->IntAtom( point->GetY().IntValue() ) : nl->RealAtom( point->GetY().Value() ) );
#else
    return nl->TwoElemList(
               nl->RealAtom( point->GetX()),
               nl->RealAtom( point->GetY()));
#endif
  }
  else
  {
    return (nl->SymbolAtom("undef"));
  }
}

/*
4.4 ~In~-function

*/
double largeint_double(ListExpr NList, bool &correct)
{
    //(largeint +  2  206547878  79) or  (largeint 2  206547878  79)

    int sign=1;
    int size=0;
    double value=0;

    ListExpr Third=NList;
    ListExpr Fst, Rst;

    Fst = nl->First(Third);//largeint
    Rst = nl->Rest(Third);//(+-  2  206547878  79) or  (2  206547878  79)

    if (nl->AtomType(nl->Second(Third))==SymbolType)
    {
  if ((nl->SymbolValue(nl->Second(Third))=="-") ||
  (nl->SymbolValue(nl->Second(Third))=="+"))
  {
      //(largeint +-  2  206547878  79)
      Fst = nl->First(Rst);//+-
      Rst = nl->Rest(Rst);//(2  206547878  79)
      if (nl->SymbolValue(nl->Second(Third))=="-")
    sign=-1;
      else sign=1;
  }
  else
  {
      correct=false;
  }
    }
    else if (nl->AtomType(nl->Second(Third))==IntType)
    {
  //(largeint  3  206547878  79  5)
  sign=1;
    }
    else correct = false;

    if (correct)
    {
  //Rst=(2  -206547878  79)
  Fst = nl->First(Rst);//Size==2
  Rst = nl->Rest(Rst);//(206547878  79)
  if (nl->AtomType(Fst) == IntType)
      size = nl->IntValue(Fst);
  else correct=false;
    }

    if ((correct) &&(size==nl->ListLength( Rst )))
    {
  while ((correct)&&(size>0))
  {
      Fst = nl->First(Rst);//Size==2
      Rst = nl->Rest(Rst);//(206547878  79)
      if (nl->AtomType(Fst) == IntType)
    value = value+nl->IntValue(Fst) * pow(2, (double)32*(size-1));
      else  correct = false;
      size--;
  }
  value=sign*value;
    }
    return value;
}
/*
4.4 rational double function

*/
double rational_double(ListExpr NList, bool &correct)
{
    ListExpr First=NList;
    double xx=0;

    if (nl->ListLength( First ) == 5 )  //(rat 4 1107 / 10000)
    {
  if  ((nl->IsAtom(nl->Fourth(First)))&&
       (nl->AtomType(nl->Fourth(First)) == SymbolType)&&
       (nl->SymbolValue(nl->Fourth(First))=="/"))
  {
      ListExpr Third=nl->Third(First);
      ListExpr Fifth=nl->Fifth(First);

      if ((nl->IsAtom(Third)) &&
    (nl->AtomType(Third) == IntType))
    xx=nl->IntValue(Third);
      else if (!(nl->IsAtom(Third)) &&
      (nl->AtomType(nl->First(Third))==SymbolType)&&
      (nl->SymbolValue(nl->First(Third))=="largeint"))
      {
        //here: process the longint value;
        xx=largeint_double(Third, correct);
      }
      else correct = false;

      if ((nl->IsAtom(nl->Fifth(First))) &&
          (nl->AtomType(nl->Fifth(First)) == IntType))
    xx=xx / nl->IntValue(nl->Fifth(First));
      else if (!(nl->IsAtom(Fifth)) &&
      (nl->AtomType(nl->First(Fifth))==SymbolType)&&
      (nl->SymbolValue(nl->First(Fifth))=="largeint"))
      {
        //here: process the longint value;
        xx=xx / largeint_double(Fifth, correct);
      }
      else correct = false;

      if ((nl->IsAtom(nl->Second(First))) &&
           (nl->AtomType(nl->Second(First)) == IntType))
    xx=xx + nl->IntValue(nl->Second(First));
      else correct = false;
  }
  else correct = false;
    }
    else //(rat - 4 1107 / 10000)
    {
  ListExpr Fourth=nl->Fourth(First);
  ListExpr Sixth=nl->Sixth(First);

  if  ((nl->IsAtom(nl->Fifth(First)))&&
       (nl->AtomType(nl->Fifth(First)) == SymbolType)&&
       (nl->SymbolValue(nl->Fifth(First))=="/"))
  {
      if ((nl->IsAtom(nl->Fourth(First))) &&
           (nl->AtomType(nl->Fourth(First)) == IntType))
    xx=nl->IntValue(nl->Fourth(First));
      else if (!(nl->IsAtom(Fourth)) &&
      (nl->AtomType(nl->First(Fourth))==SymbolType)&&
      (nl->SymbolValue(nl->First(Fourth))=="largeint"))
      {
        //here: process the longint value;
        xx=largeint_double(Fourth, correct);
      }
      else correct = false;

      if ((nl->IsAtom(nl->Sixth(First))) &&
          (nl->AtomType(nl->Sixth(First)) == IntType))
    xx=xx / nl->IntValue(nl->Sixth(First));
      else if (!(nl->IsAtom(Sixth)) &&
      (nl->AtomType(nl->First(Sixth))==SymbolType)&&
      (nl->SymbolValue(nl->First(Sixth))=="largeint"))
      {
        //here: process the longint value;
        xx=xx / largeint_double(Sixth, correct);
      }
      else correct = false;

      if ((nl->IsAtom(nl->Third(First))) &&
           (nl->AtomType(nl->Third(First)) == IntType))
    xx=xx + nl->IntValue(nl->Third(First));
      else correct = false;

      if ((nl->IsAtom(nl->Second(First))) &&
          (nl->AtomType(nl->Second(First)) == SymbolType)&&
          (nl->SymbolValue(nl->Second(First))=="-"))
    xx=xx * (-1);
      else correct = false;
  }
  else correct = false;
    }
    return xx;
}

Word
InPoint( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
    Point* newpoint;

    if ( nl->ListLength( instance ) == 2 )
    {
  ListExpr First = nl->First(instance);
  ListExpr Second = nl->Second(instance);

  Coord x, y;
  double xx, yy;

  //1. processing the first data item
  correct = true;
  if ( nl->IsAtom(First))
  {
      if( nl->AtomType(First) == IntType )
    x = nl->IntValue(First);
      else if( nl->AtomType(First) == RealType )
    x = nl->RealValue(First);
      else correct = false;
  }
  else if (((nl->ListLength( First ) == 5 )||
                (nl->ListLength( First ) == 6 ))&&
                (nl->IsAtom(nl->First(First)))&&
                (nl->AtomType(nl->First(First)) == SymbolType)&&
                (nl->SymbolValue(nl->First(First))=="rat"))
  {   //RATIONAL NUMBERS
      xx=rational_double(First, correct);
      if (correct)
      {
    x=(double)xx;
      }
  }
  else correct = false;

  //2. processing the secon data item
  if ( nl->IsAtom(Second) )
  {
      if( nl->AtomType(Second) == IntType )
    y = nl->IntValue(Second);
      else if( nl->AtomType(Second) == RealType )
    y = nl->RealValue(Second);
      else correct = false;
  }
  else if (((nl->ListLength( Second ) == 5 )||
                (nl->ListLength( Second ) == 6 ))&&
                (nl->IsAtom(nl->First(Second)))&&
                (nl->AtomType(nl->First(Second)) == SymbolType)&&
                (nl->SymbolValue(nl->First(Second))=="rat"))
  {   //RATIONAL NUMBERS
      yy=rational_double(Second, correct);
      if (correct)
      {
    y=(double)yy;
      }
  }
  else correct = false;

  //3. create the class object
  if( correct )
  {
      newpoint = new Point(true, x, y);
      return SetWord(newpoint);
  }
    }
    correct = false;
    return SetWord(Address(0));
}

/*
4.5 ~Create~-function

*/
Word
CreatePoint( const ListExpr typeInfo )
{
    //cout<<"create point2"<<endl;
    return (SetWord( new Point( false ) ));
}

/*
4.6 ~Delete~-function

*/
void
DeletePoint( const ListExpr typeInfo, Word& w )
{
  delete (Point *)w.addr;
  w.addr = 0;
}

/*
4.7 ~Close~-function

*/
void
ClosePoint( const ListExpr typeInfo, Word& w )
{
  delete (Point *)w.addr;
  w.addr = 0;
}

/*
4.8 ~Clone~-function

*/
Word
ClonePoint( const ListExpr typeInfo, const Word& w )
{
  // cout<<"typeclone ////////////////////"<<endl;
  Point *p = new Point( *((Point *)w.addr) );
  return SetWord( p );
}

/*
4.8 ~SizeOf~-function

*/
int
SizeOfPoint()
{
  return sizeof(Point);
}

/*
4.9 Function describing the signature of the type constructor

*/
ListExpr
PointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("point"),
           nl->StringAtom("(<x><y>)"),
           nl->StringAtom("(10 5)"))));
}

/*
4.10 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~point~ does not have arguments, this is trivial.

*/
bool
CheckPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "point" ));
}

/*
4.11 ~Cast~-function

*/
void* CastPoint(void* addr)
{
  return (new (addr) Point);
}

/*
4.12 Creation of the type constructor instance

*/
TypeConstructor point(
  "point",      //name
  PointProperty,      //property function describing signature
  OutPoint,     InPoint,  //Out and In functions
        0,              0,              //SaveToList and RestoreFromList functions
  CreatePoint,  DeletePoint,  //object creation and deletion
  0, 0,         //open and save functions
  ClosePoint, ClonePoint,     //object close, and clone
  CastPoint,      //cast function
  SizeOfPoint,      //sizeof function
  CheckPoint );               //kind checking function

/*
5 Type Constructor ~points~

A ~points~ value is a finite set of points.

5.1 Implementation of the class ~Points~

*/
void Points::InsertPt( const Point& p )
{
  if( IsEmpty() )
    bbox = p.BoundingBox();
  else
    bbox = bbox.Union( p.BoundingBox() );

  if( !IsOrdered() )
  {
    pos = points.Size();
    points.Put( points.Size(), p );
  }
  else
  {
    int insertpos = Position( p );
    if( insertpos != -1 )
    {
      for( int i = points.Size() - 1; i >= insertpos; i++ )
      {
        const Point *auxp;
        points.Get( i, auxp );
        points.Put( i+1, *auxp );
      }
      points.Put( insertpos, p );
      pos=insertpos;
    }
  }
}
/*
Searches (binary search algorithm) for a point in the point set and
returns its position. Returns -1 if the point is not found.

*/
int Points::Position( const Point& p ) const
{
  assert( IsOrdered() && p.IsDefined() );

  int first = 0, last = Size();

  while (first <= last)
  {
    int mid = ( first + last ) / 2;
    const Point *midPoint;
    points.Get( mid, midPoint );
    if( p > *midPoint )
      first = mid + 1;
    else if( p < *midPoint )
      last = mid - 1;
    else
      return mid;
   }
   return -1;
}

Points& Points::operator=( const Points& ps )
{
  assert( ps.IsOrdered() );

  points.Clear();
  points.Resize( ps.Size() );
  for( int i = 0; i < ps.Size(); i++ )
  {
    const Point *p;
    ps.Get( i, p );
    points.Put( i, *p );
  }
  bbox = ps.BoundingBox();
  ordered = true;
  return *this;
}

void Points::StartBulkLoad()
{
  assert( IsOrdered() );
  ordered = false;
}

void Points::EndBulkLoad( const bool sort )
{
  assert( !IsOrdered() );
  if( sort )
    Sort();
  ordered = true;
}

bool Points::operator==( const Points& ps ) const
{
  assert( IsOrdered() && ps.IsOrdered() );

  if( Size() != ps.Size() )
    return 0;

  if( bbox != ps.BoundingBox() )
    return 0;

  for( int i = 0; i < Size(); i++ )
  {
    const Point *p1, *p2;
    points.Get( i, p1 );
    ps.Get( i, p2 );
    if( *p1 != *p2 )
      return 0;
  }
  return 1;
}

bool Points::operator!=( const Points& ps ) const
{
  assert( IsOrdered() && ps.IsOrdered() );

  return !( *this == ps );
}

Points& Points::operator+=(const Point& p)
{
  assert( p.IsDefined() );

  if( IsEmpty() )
    bbox = p.BoundingBox();
  else
    bbox = bbox.Union( p.BoundingBox() );

  if( !IsOrdered() )
  {
    bool found=false;
    const Point *auxp;

    for( int i = 0; ((i < points.Size())&&(!found)); i++ )
    {
      points.Get( i, auxp );
      if (*auxp==p) found=true;
    }

    if (!found)
      points.Append(p);
  }
  else
  {
    int pos = Position( p );
    if( pos != -1 )
    {
      for( int i = points.Size() - 1; i >= pos; i++ )
      {
        const Point *auxp;
        points.Get( i, auxp );
        points.Put( i+1, *auxp );
      }
      points.Put( pos, p );
    }
  }
  return *this;
}

Points& Points::operator+=(const Points& ps)
{
  if( IsEmpty() )
    bbox = ps.BoundingBox();
  else
    bbox = bbox.Union( ps.BoundingBox() );

  for( int i = 0; i < ps.Size(); i++ )
  {
    const Point *p;
    ps.Get( i, p );
    points.Put( points.Size(), *p );
  }

  if( IsOrdered() )
  {
    ordered = false;
    Sort();
  }

  return *this;
}

/*
4.4.5 Operation ~minus~ (with ~point~)

*/

Points& Points::operator-=(const Point& p)
{
  assert( IsOrdered() && p.IsDefined() );

  int pos = Position( p );
  if( pos != -1 )
  {
    for( int i = pos; i < Size(); i++ )
    {
      const Point *auxp;
      points.Get( i+1, auxp );
      points.Put( i, *auxp );
    }
  }

  // Naive way to redo the bounding box.
  if( IsEmpty() )
    bbox.SetDefined( false );
  int i = 0;
  const Point *auxp;
  points.Get( i++, auxp );
  bbox = auxp->BoundingBox();
  for( ; i < Size(); i++ )
  {
    points.Get( i, auxp );
    bbox = bbox.Union( auxp->BoundingBox() );
  }

  return *this;
}

ostream& operator<<( ostream& o, const Points& ps )
{
  o << "<";
  for( int i = 0; i < ps.Size(); i++ )
  {
    const Point *p;
    ps.Get( i, p );
    o << " " << *p;
  }
  o << ">";

  return o;
}

int PointCompare(const void *a, const void *b)
{
  Point *pa = new ((void*)a) Point,
        *pb = new ((void*)b) Point;

  if( *pa == *pb )
    return 0;

  if( *pa < *pb )
    return -1;

  return 1;
}

void Points::Sort()
{
  assert( !IsOrdered() );

  points.Sort( PointCompare );

  ordered = true;
}

bool Points::Contains( const Point& p ) const
{
  assert( IsOrdered() && p.IsDefined() );

  if( IsEmpty() )
    return false;

  if( !p.Inside( bbox ) )
    return false;

  int first = 0, last = Size() - 1;

  while (first <= last)
  {
    int mid = ( first + last ) / 2;
    const Point *midPoint;
    points.Get( mid, midPoint );
    if( p > *midPoint )
      first = mid + 1;
    else if( p < *midPoint )
      last = mid - 1;
    else
      return true;
   }
   return false;
}

bool Points::Contains( const Points& ps ) const
{
  assert( IsOrdered() && ps.IsOrdered() );

  if( ps.IsEmpty() )
    return true;
  if( IsEmpty() )
    return false;
  if( !bbox.Contains( ps.BoundingBox() ) )
    return false;

  const Point *p1, *p2;
  int i = 0, j = 0;

  Get( i, p1 );
  ps.Get( j, p2 );
  while( true )
  {
    if( *p1 == *p2 )
    {
      if( ++j == ps.Size() )
        return true;
      ps.Get( j, p2 );
      if( ++i == Size() )
        return false;
      Get( i, p1 );
    }
    else if( *p1 < *p2 )
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
  // This part of the code should never be reached.LineSeg
  assert( true );
  return true;
}

bool Points::Inside( const Points& ps ) const
{
  assert( IsOrdered() && ps.IsOrdered() );

  return ps.Contains( *this );
}

bool Points::Intersects( const Points& ps ) const
{
  assert( IsOrdered() && ps.IsOrdered() );

  if( IsEmpty() || ps.IsEmpty() )
    return false;

  if( !bbox.Intersects( ps.BoundingBox() ) )
    return false;

  const Point *p1, *p2;
  int i = 0, j = 0;

  Get( i, p1 );
  ps.Get( j, p2 );

  while( 1 )
  {
    if( *p1 == *p2 )
      return true;
    if( *p1 < *p2 )
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

size_t Points::HashValue() const
{
    if(IsEmpty())  return (0);
    unsigned long h=0;

    const Point *p;
    Coord x;
    Coord y;

    for( int i = 0; ((i < Size())&&(i<5)); i++ )
    {
  Get( i, p );
  x=p->GetX();
  y=p->GetY();
#ifdef RATIONAL_COORDINATES
  h=h+(unsigned long)
    (5*(x.IsInteger()? x.IntValue():x.Value())
     + (y.IsInteger()? y.IntValue():y.Value()));
#else
  h=h+(unsigned long)(5*x + y);
#endif
    }
    return size_t(h);
}

void  Points::CopyFrom(const StandardAttribute* right)
{
    const Points *ps = (const Points*)right;
    ordered = true;
    assert( ps->IsOrdered());
    Clear();
    for( int i = 0; i < ps->Size(); i++ )
    {
  const Point *p;
  ps->Get( i, p );
  points.Put( i, *p );
    }
    bbox = ps->BoundingBox();
}

int Points::Compare(const Attribute * arg) const
{
  int res=0;
  const Points* ps = (const Points* )(arg);
  if ( !ps ) return (-2);

  if (IsEmpty() && (ps->IsEmpty()))  res=0;
  else if (IsEmpty())  res=-1;
  else  if ((ps->IsEmpty())) res=1;
  else
  {
    if (Size() > ps->Size()) res=1;
    else if (Size() < ps->Size()) res=-1;
    else  //their sizes are equal
    {
      bool bboxCmp = bbox.Compare( &ps->bbox );
      if( bboxCmp == 0 )
      {
        bool decided = false;

        for( int i = 0; ((i < Size())&&(!decided)); i++ )
        {
          const Point *p1, *p2;
          Get( i, p1);
          ps->Get( i, p2 );

          if (*p1 > *p2) {res=1;decided=true;}
          else if (*p1 < *p2) {res=-1;decided=true;}
        }
        if (!decided) res=0;
      }
      else
        res = bboxCmp;
    }
  }
  return (res);
}

ostream& Points::Print( ostream &os ) const
{
    os << "<";
    for( int i = 0; i < Size(); i++ )
    {
  const Point *p;
  Get( i, p );
  os << " " << *p;
    }
    os << ">";

    return os;
}

/*
5.2 List Representation

The list representation of a point is

----  (x y)
----

5.3 ~Out~-function

*/
ListExpr
OutPoints( ListExpr typeInfo, Word value )
{

  Points* points = (Points*)(value.addr);
  if( points->IsEmpty() )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    const Point *p;
    points->Get( 0, p );
    Point aux( p );
    ListExpr result = nl->OneElemList( OutPoint( nl->TheEmptyList(), SetWord( &aux ) ) );
    ListExpr last = result;

    for( int i = 1; i < points->Size(); i++ )
    {
      points->Get( i, p );
      aux = *p;
      last = nl->Append( last,
                         OutPoint( nl->TheEmptyList(), SetWord( &aux ) ) );
    }

    return result;
  }
}

/*
5.4 ~In~-function

*/
Word
InPoints( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{

  Points* points = new Points( nl->ListLength( instance ) );
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
Word
CreatePoints( const ListExpr typeInfo )
{

  return (SetWord( new Points( 0 ) ));
}

/*
5.6 ~Delete~-function

*/
void
DeletePoints( const ListExpr typeInfo, Word& w )
{

  Points *ps = (Points *)w.addr;
  ps->Destroy();
  delete ps;
  w.addr = 0;
}

/*
5.7 ~Close~-function

*/
void
ClosePoints( const ListExpr typeInfo, Word& w )
{
  delete (Points *)w.addr;
  w.addr = 0;
}

/*
5.8 ~Clone~-function

*/
Word
ClonePoints( const ListExpr typeInfo, const Word& w )
{
  Points *p = new Points( *((Points *)w.addr) );
  return SetWord( p );
}

/*
5.8 ~SizeOf~-function

*/
int
SizeOfPoints()
{
  return sizeof(Points);
}

/*
5.11 Function describing the signature of the type constructor

*/
ListExpr
PointsProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("points"),
           nl->StringAtom("(<point>*) where point is (<x><y>)"),
           nl->StringAtom("( (10 1)(4 5) )"))));
}

/*
5.12 Kind checking function

This function checks whether the type constructor is applied correctly. Since
type constructor ~point~ does not have arguments, this is trivial.

*/
bool
CheckPoints( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "points" ));
}

/*
5.13 ~Cast~-function

*/
void* CastPoints(void* addr)
{
  return (new (addr) Points);
}

/*
5.14 Creation of the type constructor instance

*/
TypeConstructor points(
        "points",                       //name
        PointsProperty,                 //property function describing signature
        OutPoints,      InPoints,       //Out and In functions
        0,              0,              //SaveToList and RestoreFromList functions
        CreatePoints,   DeletePoints,   //object creation and deletion
        0,     0,         // object open and save
        ClosePoints,    ClonePoints,    //object close and clone
        CastPoints,                     //cast function
        SizeOfPoints,                   //sizeof function
        CheckPoints );                  //kind checking function

/*
6 Type Constructor ~halfsegment~

A ~halfsegment~ value is a pair of points, with a boolean flag indicating the dominating point .

6.1 Implementation of the class ~halfsegment~

6.1.4 Overloaded Class Operators

The following utility function, ~chscmp~,  compares two halfsegments, a and b.  if a \verb+<+ b, then -1 is
returned; if a \verb+>+ b then 1 is returned, if a=b then 0 is returned. The order of halfsegments
follows the rules given in the paper about the ROSE algebra implementation.

*/

int CHalfSegment::chscmp(const CHalfSegment& chs) const
{
    if (!IsDefined() && !(chs.IsDefined()))  return 0;
    else if (!IsDefined())  return -1;
    else  if (!(chs.IsDefined())) return 1;
    else
    {
    Point dp, sp, DP, SP;
    dp=GetDPoint(); sp=GetSPoint();
    DP=chs.GetDPoint(); SP=chs.GetSPoint();

      if (dp < DP)
      return -1;
    else
      if (dp > DP)
        return 1;
    else
    {
      if (ldp!=chs.GetLDP())
      {
    if (ldp==false) return -1;
    else return 1;
      }
      else
      {
    if ((dp.GetX()==sp.GetX()) && (DP.GetX()==SP.GetX()))
    {
        if (((sp.GetY()>dp.GetY()) && (SP.GetY()>DP.GetY())) ||
            ((sp.GetY()<dp.GetY()) && (SP.GetY()<DP.GetY())))
        {
      if (GetSPoint()<chs.GetSPoint()) return -1;
      else if (GetSPoint()>chs.GetSPoint()) return 1;
      else return 0;
        }
        else if (sp.GetY()>dp.GetY())
        {
      if (ldp==true) return 1;
      else return -1;
        }
        else
        {
      if (ldp==true) return -1;
      else return 1;
        }
    }
    else if (dp.GetX()==sp.GetX())
    {
        if (sp.GetY()>dp.GetY())
        {
      if (ldp==true) return 1;
      else return -1;
        }
        else if  (sp.GetY()<dp.GetY())
        {
      if (ldp==true) return -1;
      else return 1;
        }
        else cout<<"two end points are identical!";
    }
    else if (DP.GetX()==SP.GetX())
    {
            if (SP.GetY()>DP.GetY())
        {
      if (ldp==true) return -1;
      else return 1;
        }
        else if  (SP.GetY()<DP.GetY())
        {
      if (ldp==true) return 1;
      else return -1;
        }
        else cout<<"two end points are identical!";
    }
    else
    {
        Coord xd,yd,xs,ys;
        Coord Xd,Yd,Xs,Ys;
        xd=dp.GetX();  yd=dp.GetY();
        xs=sp.GetX();  ys=sp.GetY();
        Xd=DP.GetX();  Yd=DP.GetY();
        Xs=SP.GetX();  Ys=SP.GetY();
#ifdef RATIONAL_COORDINATES
        double k=
        ((yd.IsInteger()? yd.IntValue():yd.Value()) -
         (ys.IsInteger()? ys.IntValue():ys.Value())) /
        ((xd.IsInteger()? xd.IntValue():xd.Value()) -
         (xs.IsInteger()? xs.IntValue():xs.Value())) ;
        double K=
        ((Yd.IsInteger()? Yd.IntValue():Yd.Value()) -
         (Ys.IsInteger()? Ys.IntValue():Ys.Value())) /
        ((Xd.IsInteger()? Xd.IntValue():Xd.Value()) -
         (Xs.IsInteger()? Xs.IntValue():Xs.Value()));
#else
        double k = (yd - ys) / (xd - xs) ;
        double K= (Yd -Ys) / (Xd - Xs);
#endif
    if (k<K) return -1;
    else if (k>K) return 1;
    else
    {
        if (GetSPoint()<chs.GetSPoint()) return -1;
        else if (GetSPoint()>chs.GetSPoint()) return 1;
        else return 0;
    }
               }
      }
  }
    }
    cout<<"it shouldn't reach here!"<<endl;
    return -2;
}

bool CHalfSegment::operator==(const CHalfSegment& chs) const
{
    return (chscmp(chs)==0);
}

bool CHalfSegment::operator!=(const CHalfSegment& chs) const
{
    return (!(*this == chs));
}

bool CHalfSegment::operator<(const CHalfSegment& chs) const
{
    return (chscmp(chs)==-1);
}

bool CHalfSegment::operator>(const CHalfSegment& chs) const
{
    return (chscmp(chs)==1);
}

int CHalfSegment::logicless(const CHalfSegment& chs) const
{
    if (attr.faceno<chs.attr.faceno) return 1;
    else  if (attr.faceno>chs.attr.faceno) return 0;
    else
    {
  if (attr.cycleno<chs.attr.cycleno) return 1;
  else  if (attr.cycleno>chs.attr.cycleno) return 0;
  else
  {
      if (attr.edgeno<chs.attr.edgeno) return 1;
      else  if (attr.edgeno>chs.attr.edgeno) return 0;
      else return 0;
  }
    }
}

int CHalfSegment::logicgreater(const CHalfSegment& chs) const
{
    if (attr.faceno>chs.attr.faceno) return 1;
    else  if (attr.faceno<chs.attr.faceno) return 0;
    else
    {
  if (attr.cycleno>chs.attr.cycleno) return 1;
  else  if (attr.cycleno<chs.attr.cycleno) return 0;
  else
  {
      if (attr.edgeno>chs.attr.edgeno) return 1;
      else  if (attr.edgeno<chs.attr.edgeno) return 0;
      else return 0;
  }
    }
}


bool CHalfSegment::LogicEqual(const CHalfSegment& chs) const
{
  if ( (attr.faceno  == chs.attr.faceno) &&
       (attr.cycleno == chs.attr.cycleno) &&
       (attr.edgeno  == chs.attr.edgeno) )
    return true;
  return false;
}


/* von Frau Seberich-Duell */


bool CHalfSegment::innerInter( const CHalfSegment& chs,  Point& resp,
                   CHalfSegment& rchs, bool& first, bool& second ) const
{
   resp.SetDefined(false);   	rchs.SetDefined(false);
   first = false;		second = false;
   Coord xl,yl,xr,yr ,  Xl,Yl,Xr,Yr;
   double k, a, K, A;
   xl=lp.GetX();  yl=lp.GetY();
   xr=rp.GetX();  yr=rp.GetY();
   if (xl!=xr)
   {     //k=(yr-yl) / (xr-xl);  a=yl - k*yl;
    #ifdef RATIONAL_COORDINATES
      k=((yr.IsInteger()? yr.IntValue():yr.Value()) -
         (yl.IsInteger()? yl.IntValue():yl.Value())) /
        ((xr.IsInteger()? xr.IntValue():xr.Value()) -
         (xl.IsInteger()? xl.IntValue():xl.Value()));
      a=(yl.IsInteger()? yl.IntValue():yl.Value()) -
         k*(xl.IsInteger()? xl.IntValue():xl.Value());
    #else
      k=(yr - yl) / (xr - xl);
      a=yl - k*xl;
    #endif
   }
   Xl=chs.GetLP().GetX();  Yl=chs.GetLP().GetY();
   Xr=chs.GetRP().GetX();  Yr=chs.GetRP().GetY();
   if (Xl!=Xr)
   {     //K=(Yr-Yl) / (Xr-Xl);  A=Yl - K*Xl;
    #ifdef RATIONAL_COORDINATES
      K=  ((Yr.IsInteger()? Yr.IntValue():Yr.Value()) -
          (Yl.IsInteger()? Yl.IntValue():Yl.Value())) /
         ((Xr.IsInteger()? Xr.IntValue():Xr.Value()) -
          (Xl.IsInteger()? Xl.IntValue():Xl.Value()));
      A = (Yl.IsInteger()? Yl.IntValue():Yl.Value()) -
         K*(Xl.IsInteger()? Xl.IntValue():Xl.Value());
    #else
      K=  (Yr - Yl) / (Xr - Xl);
      A = Yl - K*Xl;
    #endif
   }
   if ((xl==xr) && (Xl==Xr))  { //both l and L are vertical lines
      if (xl!=Xl) return false;
      else  {
         Coord ylow, yup, Ylow, Yup;
         if (yl<yr)   { ylow=yl;  yup=yr;  }
         else         { ylow=yr;  yup=yl;  }
         if (Yl<Yr)   { Ylow=Yl;  Yup=Yr;  }
         else	  { Ylow=Yr;  Yup=Yl;  }
         if  (((ylow>Ylow) && (ylow<Yup))|| ((yup>Ylow) && (yup<Yup)) ||
              ((Ylow>ylow) && (Ylow<yup))|| ((Yup>ylow) && (Yup<yup))) {
            Point p1, p2;
            if (ylow>Ylow)	p1.Set(xl, ylow);
            else 		p1.Set(xl, Ylow);
            if (yup<Yup) 	p2.Set(xl, yup);
            else 		p2.Set(xl, Yup);
            rchs.Set(true, p1, p2);
	    first = true; 	second = true;
            return true;
         }
         else return false;
      }
   }
   else if (Xl==Xr) {    //only L is vertical
      if ( xl==Xl && yl>Yl && yl<Yr ) {resp.Set(xl,yl); second = true; return true;}
      if ( xr==Xl && yr>Yl && yr<Yr ) {resp.Set(xr,yr); second = true; return true;}
      else  {
        #ifdef RATIONAL_COORDINATES
         double y0=k*(Xl.IsInteger()? Xl.IntValue():Xl.Value())+a;
         Coord yy(y0);
       #else
         double y0=k*Xl+a;
         Coord yy=y0;
      #endif
         //(Xl, y0) is the intersection of l and L
         if ((Xl>xl) &&( Xl<xr))  {
	    if ( (yy>=Yl) && (yy <= Yr) ) {
	       resp.Set (Xl,yy);
	       first = true;
               if ( (yy>Yl) && (yy<Yr) ) second = true;
	       return true;
	    }
	    else return false;
         }
      }
   }
   else if (xl==xr) {    //only l is vertical
      if ( Xl==xl && Yl>yl && Yl<yr ) {resp.Set(Xl,Yl); first = true; return true;}
      if ( Xr==xl && Yr>yl && Yr<yr ) {resp.Set(Xr,Yr); first = true; return true;}
      else  {
        #ifdef RATIONAL_COORDINATES
         double y0=K*(xl.IsInteger()? xl.IntValue():xl.Value())+A;
         Coord yy(y0);
       #else
         double y0=K*xl+A;
         Coord yy=y0;
      #endif
         //(Xl, y0) is the intersection of l and L
         if ((xl>Xl) && (xl<Xr))  {
	    if ( (yy>=yl) && (yy <= yr) ) {
	       resp.Set (xl,yy);
	       second = true;
               if ( (yy>yl) && (yy<yr) ) first = true;
	       return true;
	    }
	    else return false;
         }
      }
   }
   //otherwise: both *this and *arg are non-vertical lines
   if (k==K)   { // both lines are parallel or the same
      if (a != A) return false;  // parallel lines
      if  (((xl>Xl) && (xl<Xr)) || ((xr>Xl) && (xr<Xr)) ||
           ((Xl>xl) && (Xl<xr)) || ((Xr>xl) && (Xr<xr)))  {
         Point p1, p2;
         if (xl>Xl) 	p1.Set(xl, yl);
         else  		p1.Set(Xl, Yl);
         if (xr<Xr)	p2.Set(xr, yr);
         else  		p2.Set(Xr, Yr);
         rchs.Set(true, p1, p2);
	 first = true; second = true;
         return true;
      }
     else return false;
   }
   else      {
      double x0 = (A-a) / (k-K);  // y0=x0*k+a;
      double y0 = x0*k+a;
     #ifdef RATIONAL_COORDINATES
        Coord xx(x0);   Coord yy(y0);
     #else
        Coord xx = x0; Coord yy=y0;
     #endif
     if (GetLP() == chs.GetLP() || GetRP() == chs.GetRP() ) return false;
     if ((xx == xl || xx == xr) && xx > Xl && xx < Xr )
        {resp.Set(xx,yy); second = true; return true; }
     if ( (xx == Xl || xx == Xr) && xx > xl && xx < xr )
        {resp.Set(xx,yy); first = true; return true; }
     if ((xx>xl) && (xx<xr) && (xx>Xl) && (xx <Xr)) {
         resp.Set(xx,yy); first = true; second= true; return true; }
     else  return false;
   }
}

/* Ende Teil von Frau Seberich-Duell */
 
int HalfSegmentCompare(const void *a, const void *b)
{
  CHalfSegment *chsa = new ((void*)a) CHalfSegment,
               *chsb = new ((void*)b) CHalfSegment;

  if( *chsa == *chsb )
    return 0;

  if( *chsa < *chsb )
    return -1;

  return 1;
}

int HalfSegmentLogCompare(const void *a, const void *b)
{
  CHalfSegment *chsa = new ((void*)a) CHalfSegment,
               *chsb = new ((void*)b) CHalfSegment;

  if( chsa->logicless(*chsb))
    return -1;

  if( chsa->logicgreater(*chsb))
    return 1;

  return 0;
}

/*
6.1.5 Overloaded Output Function

*/
ostream& operator<<(ostream &os, const CHalfSegment& chs)
{
  if( chs.IsDefined())
    return (os << "("
               <<"F("<< chs.attr.faceno
               <<") C("<<  chs.attr.cycleno
               <<") E(" << chs.attr.edgeno<<") DP("<<  (chs.GetLDP()? "L":"R")
               <<") IA("<< (chs.attr.insideAbove? "A":"U")

               <<") Co("<<chs.attr.coverageno
               <<") PNo("<<chs.attr.partnerno
               <<") Def("<<chs.IsDefined()
               <<") ("<< chs.GetLP() << " "<< chs.GetRP() <<") ");
  else
    return (os << "undef");
}

/*
6.1.6 Intersects Function

This function decides whether two line segments intersect each other. The intersecting point
can be in any place of the line segments, including the middle points and endpoints.

*/
bool CHalfSegment::Intersects( const CHalfSegment& chs ) const
{
    Coord xl,yl,xr,yr;
    Coord Xl,Yl,Xr,Yr;
    double k, a, K, A;
    double x0; //, y0;  (x0, y0) is the intersection

    Rectangle<2> bbox1=this->BoundingBox();
    Rectangle<2> bbox2=chs.BoundingBox();

    if (!bbox1.Intersects(bbox2)) return false;

    xl=lp.GetX();  yl=lp.GetY();
    xr=rp.GetX();  yr=rp.GetY();
    if (xl!=xr)
    {     //k=(yr-yl) / (xr-xl);  a=yl - k*yl;
#ifdef RATIONAL_COORDINATES
  k=((yr.IsInteger()? yr.IntValue():yr.Value()) -
        (yl.IsInteger()? yl.IntValue():yl.Value())) /
       ((xr.IsInteger()? xr.IntValue():xr.Value()) -
        (xl.IsInteger()? xl.IntValue():xl.Value()));
  a=(yl.IsInteger()? yl.IntValue():yl.Value()) -
       k*(xl.IsInteger()? xl.IntValue():xl.Value());
#else
  k=(yr - yl) / (xr - xl);
  a=yl - k*xl;
#endif
    }

    Xl=chs.GetLP().GetX();  Yl=chs.GetLP().GetY();
    Xr=chs.GetRP().GetX();  Yr=chs.GetRP().GetY();
    if (Xl!=Xr)
    {     //K=(Yr-Yl) / (Xr-Xl);  A=Yl - K*Xl;
#ifdef RATIONAL_COORDINATES
  K=  ((Yr.IsInteger()? Yr.IntValue():Yr.Value()) -
          (Yl.IsInteger()? Yl.IntValue():Yl.Value())) /
         ((Xr.IsInteger()? Xr.IntValue():Xr.Value()) -
          (Xl.IsInteger()? Xl.IntValue():Xl.Value()));
  A = (Yl.IsInteger()? Yl.IntValue():Yl.Value()) -
         K*(Xl.IsInteger()? Xl.IntValue():Xl.Value());
#else
  K=  (Yr - Yl) / (Xr - Xl);
  A = Yl - K*Xl;
#endif
    }

    if ((xl==xr) && (Xl==Xr)) //both l and L are vertical lines
      {
    if (xl!=Xl) return false;
    else   if  (((yl>=Yl) && (yl<=Yr)) ||
      ((yr>=Yl) && (yr<=Yr))||
      ((Yl>=yl) && (Yl<=yr)) ||
      ((Yr>=yl) && (Yr<=yr)))
                    return true;
               else return false;
      }

    if (Xl==Xr)    //only L is vertical
    {
#ifdef RATIONAL_COORDINATES
     double y0=k*(Xl.IsInteger()? Xl.IntValue():Xl.Value())+a;
     Coord yy(y0);
#else
     double y0=k*Xl+a;
     Coord yy=y0;
#endif
  //(Xl, y0) is the intersection of l and L
  if    ((Xl>=xl) &&(Xl<=xr))
  {
      if (((yy>=Yl) && (yy<=Yr)) || ((yy>=Yr) && (yy<=Yl)))
              return true;
      else return false;
  }
  else return false;
    }

    if (xl==xr)    //only l is vertical
    {
#ifdef RATIONAL_COORDINATES
  double Y0=K*(xl.IsInteger()? xl.IntValue():xl.Value())+A;
  Coord YY(Y0);
#else
  double Y0=K*xl+A;
  Coord YY=Y0;
#endif

  //(xl, Y0) is the intersection of l and L
  if ((xl>=Xl) && (xl<=Xr))
  {
      if (((YY>=yl) && (YY<=yr)) || ((YY>=yr) && (YY<=yl)))
              return true;
      else return false;
  }
  else return false;
    }

    //otherwise: both *this and *arg are non-vertical lines
    if (k==K)
      {
    if  (A!=a)  return false; //Parallel lines
    else //they are in the same straight line
    {
        if (((xl>=Xl)&&(xl<=Xr)) || ((Xl>=xl) && (Xl<=xr)))
                return true;
        else return false;
    }
      }
      else
      {
    x0=(A-a) / (k-K);  // y0=x0*k+a;
    Coord xx(x0);
    if ((xx>=xl) && (xx<=xr) && (xx>=Xl) && (xx <=Xr))
        return true;
    else return false;
      }
}
/*
6.1.7 InnerIntersects Function

This function decides whether two line segments intersect with their inner points.

*/
bool CHalfSegment::innerIntersects( const CHalfSegment& chs ) const
{
    Coord xl,yl,xr,yr;
    Coord Xl,Yl,Xr,Yr;
    double k, a, K, A;
    double x0; //, y0;  (x0, y0) is the intersection

    xl=lp.GetX();  yl=lp.GetY();
    xr=rp.GetX();  yr=rp.GetY();
    if (xl!=xr)
    {     //k=(yr-yl) / (xr-xl);  a=yl - k*yl;
#ifdef RATIONAL_COORDINATES
  k=((yr.IsInteger()? yr.IntValue():yr.Value()) -
        (yl.IsInteger()? yl.IntValue():yl.Value())) /
       ((xr.IsInteger()? xr.IntValue():xr.Value()) -
        (xl.IsInteger()? xl.IntValue():xl.Value()));
  a=(yl.IsInteger()? yl.IntValue():yl.Value()) -
       k*(xl.IsInteger()? xl.IntValue():xl.Value());
#else
  k=(yr - yl) / (xr - xl);
  a=yl - k*xl;
#endif
    }

    Xl=chs.GetLP().GetX();  Yl=chs.GetLP().GetY();
    Xr=chs.GetRP().GetX();  Yr=chs.GetRP().GetY();
    if (Xl!=Xr)
    {     //K=(Yr-Yl) / (Xr-Xl);  A=Yl - K*Xl;
#ifdef RATIONAL_COORDINATES
  K=  ((Yr.IsInteger()? Yr.IntValue():Yr.Value()) -
          (Yl.IsInteger()? Yl.IntValue():Yl.Value())) /
         ((Xr.IsInteger()? Xr.IntValue():Xr.Value()) -
          (Xl.IsInteger()? Xl.IntValue():Xl.Value()));
  A = (Yl.IsInteger()? Yl.IntValue():Yl.Value()) -
         K*(Xl.IsInteger()? Xl.IntValue():Xl.Value());
#else
  K=  (Yr - Yl) / (Xr - Xl);
  A = Yl - K*Xl;
#endif
    }

    if ((xl==xr) && (Xl==Xr)) //both l and L are vertical lines
      {
    if (xl!=Xl) return false;
    else
    {
        Coord ylow, yup, Ylow, Yup;
        if (yl<yr)
        {
      ylow=yl;
      yup=yr;
        }
        else
        {
      ylow=yr;
      yup=yl;
        }
        if (Yl<Yr)
        {
      Ylow=Yl;
      Yup=Yr;
        }
        else
        {
      Ylow=Yr;
      Yup=Yl;
        }
        if  ((ylow>=Yup) || (yup<=Ylow))
      return false;
        else return true;
    }
      }

    if (Xl==Xr)    //only L is vertical
    {
#ifdef RATIONAL_COORDINATES
  double y0=k*(Xl.IsInteger()? Xl.IntValue():Xl.Value())+a;
  Coord yy(y0);
#else
  double y0=k*Xl+a;
  Coord yy=y0;
#endif
  //(Xl, y0) is the intersection of l and L
  if    ((Xl>=xl) &&(Xl<=xr))
  {
      if (((yy>Yl) && (yy<Yr)) || ((yy>Yr) && (yy<Yl)))
              return true;
      else return false;
  }
  else return false;
    }

    if (xl==xr)    //only l is vertical
    {
#ifdef RATIONAL_COORDINATES
  double Y0=K*(xl.IsInteger()? xl.IntValue():xl.Value())+A;
  Coord YY(Y0);
#else
  double Y0=K*xl+A;
  Coord YY=Y0;
#endif
  //(xl, Y0) is the intersection of l and L
  if ((xl>Xl) && (xl<Xr))
  {
      if (((YY>=yl) && (YY<=yr)) || ((YY>=yr) && (YY<=yl)))
              return true;
      else return false;
  }
  else return false;
    }

    //otherwise: both *this and *arg are non-vertical lines
    if (k==K)
      {
    if  (A!=a)  return false; //Parallel lines
    else //they are in the same straight line
    {
        if ((xr<=Xl) || (xl>=Xr))
                return false;
        else return true;
    }
      }
      else
      {
    x0=(A-a) / (k-K);  // y0=x0*k+a;
    Coord xx(x0);
    if ((xx>=xl) && (xx<=xr) && (xx>Xl) && (xx <Xr))
        return true;
    else return false;
      }
}

/*
6.1.8 Single-Point-Intersects Function

This function decides whether two line segments intersect with a single point. The single
intersecting points can be middle point or endpoint.

*/
bool CHalfSegment::spintersect( const CHalfSegment& chs, Point& resp) const
{
    Coord xl,yl,xr,yr;
    Coord Xl,Yl,Xr,Yr;
    double k, a, K, A;
    double x0, y0; // (x0, y0) is the intersection

    xl=lp.GetX();  yl=lp.GetY();
    xr=rp.GetX();  yr=rp.GetY();
    if (xl!=xr)
    {     //k=(yr-yl) / (xr-xl);  a=yl - k*xl;
#ifdef RATIONAL_COORDINATES
  k=((yr.IsInteger()? yr.IntValue():yr.Value()) -
        (yl.IsInteger()? yl.IntValue():yl.Value())) /
       ((xr.IsInteger()? xr.IntValue():xr.Value()) -
        (xl.IsInteger()? xl.IntValue():xl.Value()));
  a=(yl.IsInteger()? yl.IntValue():yl.Value()) -
       k*(xl.IsInteger()? xl.IntValue():xl.Value());
#else
  k=(yr-yl) / (xr-xl);
  a=yl - k*xl;
#endif
    }

    Xl=chs.GetLP().GetX();  Yl=chs.GetLP().GetY();
    Xr=chs.GetRP().GetX();  Yr=chs.GetRP().GetY();
    if (Xl!=Xr)
    {     //K=(Yr-Yl) / (Xr-Xl);  A=Yl - K*Xl;
#ifdef RATIONAL_COORDINATES
  K=  ((Yr.IsInteger()? Yr.IntValue():Yr.Value()) -
          (Yl.IsInteger()? Yl.IntValue():Yl.Value())) /
         ((Xr.IsInteger()? Xr.IntValue():Xr.Value()) -
          (Xl.IsInteger()? Xl.IntValue():Xl.Value()));
  A = (Yl.IsInteger()? Yl.IntValue():Yl.Value()) -
         K*(Xl.IsInteger()? Xl.IntValue():Xl.Value());
#else
  K=(Yr-Yl) / (Xr-Xl);
  A=Yl - K*Xl;
#endif
    }

    if ((xl==xr) && (Xl==Xr)) //both l and L are vertical lines
      {
   return false;
     }

    if (Xl==Xr)    //only L is vertical
    {
#ifdef RATIONAL_COORDINATES
  double y0=k*(Xl.IsInteger()? Xl.IntValue():Xl.Value())+a;
  Coord yy(y0);
#else
  double y0=k*Xl+a;
  Coord yy=y0;
#endif
  //(Xl, y0) is the intersection of l and L
  if    ((Xl>xl) &&(Xl<xr))
  {
      if (((yy>Yl) && (yy<Yr)) || ((yy>Yr) && (yy<Yl)))
       {
    resp.Set(Xl, yy);
    return true;
      }
      else return false;
  }
  else return false;
    }

    if (xl==xr)    //only l is vertical
    {
#ifdef RATIONAL_COORDINATES
  double Y0=K*(xl.IsInteger()? xl.IntValue():xl.Value())+A;
  Coord YY(Y0);
#else
  double Y0=K*xl+A;
  Coord YY=Y0;
#endif
  //(xl, Y0) is the intersection of l and L
  if ((xl>Xl) && (xl<Xr))
  {
      if (((YY>yl) && (YY<yr)) || ((YY>yr) && (YY<yl)))
      {
    resp.Set(xl,YY);
    return true;
      }
      else return false;
  }
  else return false;
    }

    //otherwise: both *this and *arg are non-vertical lines
    if (k==K)
    {
  return false;
    }
    else
    {
  x0=(A-a) / (k-K);
  y0=x0*k+a;

#ifdef RATIONAL_COORDINATES
  Coord xx(x0);
  Coord yy(y0);
#else
  Coord xx=x0;
  Coord yy=y0;
#endif

  if ((xx>xl) && (xx<xr) && (xx>Xl) && (xx <Xr))
  {
      resp.Set(xx, yy);
      return true;
  }
  else return false;
    }
}
/*
6.1.9 Overlap Intersects Function

This function decides whether two line segments overlap each other. That is, the intersection is
part of the segment, not just a point.

*/
bool CHalfSegment::overlapintersect( const CHalfSegment& chs, CHalfSegment& reschs ) const
{
    Coord xl,yl,xr,yr;
    Coord Xl,Yl,Xr,Yr;
    double k, a, K, A;

    if (*this==chs)
    {
  reschs=chs;
  return true;
    }

    xl=lp.GetX();  yl=lp.GetY();
    xr=rp.GetX();  yr=rp.GetY();
    if (xl!=xr)
    {     //k=(yr-yl) / (xr-xl);  a=yl - k*xl;
#ifdef RATIONAL_COORDINATES
  k=((yr.IsInteger()? yr.IntValue():yr.Value()) -
        (yl.IsInteger()? yl.IntValue():yl.Value())) /
       ((xr.IsInteger()? xr.IntValue():xr.Value()) -
        (xl.IsInteger()? xl.IntValue():xl.Value()));
  a=(yl.IsInteger()? yl.IntValue():yl.Value()) -
       k*(xl.IsInteger()? xl.IntValue():xl.Value());
#else
  k=(yr-yl) / (xr-xl);
  a=yl - k*xl;
#endif
    }

    Xl=chs.GetLP().GetX();  Yl=chs.GetLP().GetY();
    Xr=chs.GetRP().GetX();  Yr=chs.GetRP().GetY();
    if (Xl!=Xr)
    {     //K=(Yr-Yl) / (Xr-Xl);  A=Yl - K*Xl;
#ifdef RATIONAL_COORDINATES
  K=  ((Yr.IsInteger()? Yr.IntValue():Yr.Value()) -
          (Yl.IsInteger()? Yl.IntValue():Yl.Value())) /
         ((Xr.IsInteger()? Xr.IntValue():Xr.Value()) -
          (Xl.IsInteger()? Xl.IntValue():Xl.Value()));
  A = (Yl.IsInteger()? Yl.IntValue():Yl.Value()) -
         K*(Xl.IsInteger()? Xl.IntValue():Xl.Value());
#else
  K=(Yr-Yl) / (Xr-Xl);
  A=Yl - K*Xl;
#endif
    }

    if ((xl==xr) && (Xl==Xr)) //both l and L are vertical lines
    {
  if (xl!=Xl) return false;
  else
  {
      Coord ylow, yup, Ylow, Yup;
      if (yl<yr)
      {
    ylow=yl;
    yup=yr;
      }
      else
      {
    ylow=yr;
    yup=yl;
      }
      if (Yl<Yr)
      {
    Ylow=Yl;
    Yup=Yr;
      }
      else
      {
    Ylow=Yr;
    Yup=Yl;
      }

      if  (((ylow>Ylow) && (ylow<Yup))||
           ((yup>Ylow) && (yup<Yup)) ||
           ((Ylow>ylow) && (Ylow<yup))||
           ((Yup>ylow) && (Yup<yup)))
      {
    Point p1, p2;
    if (ylow>Ylow)
            p1.Set(xl, ylow);
    else p1.Set(xl, Ylow);

    if (yup<Yup)
            p2.Set(xl, yup);
    else p2.Set(xl, Yup);

    reschs.Set(true, p1, p2);
    return true;
      }
      else return false;
  }
    }

    if ((Xl==Xr)||(xl==xr))  //only L or l is vertical
    {
  return false;
    }

    //otherwise: both *this and *arg are non-vertical lines
    if ((k==K) && (A==a))
    {
  if (((xl>Xl)&&(xl<Xr)) ||
      ((xr>Xl)&&(xr<Xr)) ||
      ((Xl>xl) && (Xl<xr))||
      ((Xr>xl) && (Xr<xr)))
  {
      Point p1, p2;
      if (xl>Xl)
               p1.Set(xl, yl);
      else  p1.Set(Xl, Yl);

      if (xr<Xr)
               p2.Set(xr, yr);
      else  p2.Set(Xr, Yr);

      reschs.Set(true, p1, p2);
      return true;
  }
  else return false;
    }
    else  return false;
}
/*
6.1.10 Cross Intersects Function

This function decides whether two line segments cross each other. That is, they are not parallel
and they intersect with a middle point.

*/
bool CHalfSegment::cross( const CHalfSegment& chs ) const
{
    Coord xl,yl,xr,yr;
    Coord Xl,Yl,Xr,Yr;
    double k, a, K, A;
    double x0; //, y0;  (x0, y0) is the intersection

    if ((lp==chs.GetLP())||(lp==chs.GetRP())||
        (rp==chs.GetLP())||(rp==chs.GetRP()))
    return false;

    xl=lp.GetX();  yl=lp.GetY();
    xr=rp.GetX();  yr=rp.GetY();
    if (xl!=xr)
    {     //k=(yr-yl) / (xr-xl);  a=yl - k*xl;
#ifdef RATIONAL_COORDINATES
  k=((yr.IsInteger()? yr.IntValue():yr.Value()) -
        (yl.IsInteger()? yl.IntValue():yl.Value())) /
       ((xr.IsInteger()? xr.IntValue():xr.Value()) -
        (xl.IsInteger()? xl.IntValue():xl.Value()));
  a=(yl.IsInteger()? yl.IntValue():yl.Value()) -
       k*(xl.IsInteger()? xl.IntValue():xl.Value());
#else
  k=(yr-yl) / (xr-xl);
  a=yl - k*xl;
#endif
    }

    Xl=chs.GetLP().GetX();  Yl=chs.GetLP().GetY();
    Xr=chs.GetRP().GetX();  Yr=chs.GetRP().GetY();
    if (Xl!=Xr)
    {     //K=(Yr-Yl) / (Xr-Xl);  A=Yl - K*Xl;
#ifdef RATIONAL_COORDINATES
  K=  ((Yr.IsInteger()? Yr.IntValue():Yr.Value()) -
          (Yl.IsInteger()? Yl.IntValue():Yl.Value())) /
         ((Xr.IsInteger()? Xr.IntValue():Xr.Value()) -
          (Xl.IsInteger()? Xl.IntValue():Xl.Value()));
  A = (Yl.IsInteger()? Yl.IntValue():Yl.Value()) -
         K*(Xl.IsInteger()? Xl.IntValue():Xl.Value());
#else
  K=(Yr-Yl) / (Xr-Xl);
  A=Yl - K*Xl;
#endif
    }

    if ((xl==xr) && (Xl==Xr)) //both l and L are vertical lines
      {
  return false;
      }

    if (Xl==Xr)    //only L is vertical
    {
#ifdef RATIONAL_COORDINATES
  double y0=k*(Xl.IsInteger()? Xl.IntValue():Xl.Value())+a;
  //(Xl, y0) is the intersection of l and L
  Coord yy(y0);
#else
  double y0=k*Xl+a;
  Coord yy=y0;
#endif
  if    ((Xl>xl) &&(Xl<xr))
  {
      if (((yy>Yl) && (yy<Yr)) || ((yy>Yr) && (yy<Yl)))
              return true;
      else return false;
  }
  else return false;
    }

    if (xl==xr)    //only l is vertical
    {
#ifdef RATIONAL_COORDINATES
  double Y0=K*(xl.IsInteger()? xl.IntValue():xl.Value())+A;
  Coord YY(Y0);
#else
  double Y0=K*xl+A;
  Coord YY=Y0;
#endif
  //(xl, Y0) is the intersection of l and L
  if ((xl>Xl) && (xl<Xr))
  {
      if (((YY>yl) && (YY<yr)) || ((YY>yr) && (YY<yl)))
              return true;
      else return false;
  }
  else return false;
    }

    //otherwise: both *this and *arg are non-vertical lines

    if (k==K)
      {
  return false;
      }
      else
      {
  x0=(A-a) / (k-K);  // y0=x0*k+a;
#ifdef RATIONAL_COORDINATES
  Coord xx(x0);
#else
  Coord xx=x0;
#endif
  if ((xx>xl) && (xx<xr) && (xx>Xl) && (xx<Xr))
          return true;
  else return false;
      }
}
/*
6.1.11 Crossings Intersects Function

This function is similar to cross function, with just minor differences; the intersecting point can be the endpoint
of the first segment but not the second.

*/
bool CHalfSegment::crossings( const CHalfSegment& chs, Point& p ) const
{
    Coord xl,yl,xr,yr;
    Coord Xl,Yl,Xr,Yr;
    double k, a, K, A;
    double x0, y0; // (x0, y0) is the intersection

    xl=lp.GetX();  yl=lp.GetY();
    xr=rp.GetX();  yr=rp.GetY();
    if (xl!=xr)
    {     //k=(yr-yl) / (xr-xl);  a=yl - k*xl;
#ifdef RATIONAL_COORDINATES
  k=((yr.IsInteger()? yr.IntValue():yr.Value()) -
        (yl.IsInteger()? yl.IntValue():yl.Value())) /
       ((xr.IsInteger()? xr.IntValue():xr.Value()) -
        (xl.IsInteger()? xl.IntValue():xl.Value()));
  a=(yl.IsInteger()? yl.IntValue():yl.Value()) -
       k*(xl.IsInteger()? xl.IntValue():xl.Value());
#else
  k=(yr-yl) / (xr-xl);
  a=yl - k*xl;
#endif
    }

    Xl=chs.GetLP().GetX();  Yl=chs.GetLP().GetY();
    Xr=chs.GetRP().GetX();  Yr=chs.GetRP().GetY();
    if (Xl!=Xr)
    {     //K=(Yr-Yl) / (Xr-Xl);  A=Yl - K*Xl;
#ifdef RATIONAL_COORDINATES
  K=  ((Yr.IsInteger()? Yr.IntValue():Yr.Value()) -
          (Yl.IsInteger()? Yl.IntValue():Yl.Value())) /
         ((Xr.IsInteger()? Xr.IntValue():Xr.Value()) -
          (Xl.IsInteger()? Xl.IntValue():Xl.Value()));
  A = (Yl.IsInteger()? Yl.IntValue():Yl.Value()) -
         K*(Xl.IsInteger()? Xl.IntValue():Xl.Value());
#else
  K=(Yr-Yl) / (Xr-Xl);
  A=Yl - K*Xl;
#endif
    }

    if ((xl==xr) && (Xl==Xr)) //both l and L are vertical lines
      {
  return false;
      }

    if (Xl==Xr)    //only L is vertical
    {
#ifdef RATIONAL_COORDINATES
  double y0=k*(Xl.IsInteger()? Xl.IntValue():Xl.Value())+a;
  Coord xx(Xl);
  Coord yy(y0);
#else
  double y0=k*Xl+a;
  Coord xx=Xl;
  Coord yy=y0;
#endif

  if    ((Xl>=xl) &&(Xl<=xr))
  {
      if (((yy>=Yl) && (yy<=Yr)) || ((yy>=Yr) && (yy<=Yl)))
      {
    p.Set(xx, yy);
    return true;
      }
      else return false;
  }
  else return false;
    }

    if (xl==xr)    //only l is vertical
    {
#ifdef RATIONAL_COORDINATES
  double Y0=K*(xl.IsInteger()? xl.IntValue():xl.Value())+A;
  Coord XX(xl);
  Coord YY(Y0);
#else
  double Y0=K*xl+A;
  Coord XX=xl;
  Coord YY=Y0;
#endif

  if ((xl>=Xl) && (xl<=Xr))
  {
      if (((YY>=yl) && (YY<=yr)) || ((YY>=yr) && (YY<=yl)))
      {
    p.Set(XX, YY);
    return true;
      }
      else return false;
  }
  else return false;
    }

    //otherwise: both *this and *arg are non-vertical lines

    if (k==K)
      {
  return false;
      }
      else
      {
  x0=(A-a) / (k-K);
  y0=x0*k+a;

  Coord xx(x0);
  Coord yy(y0);

  if ((xx>=xl) && (xx<=xr) && (xx>=Xl) && (xx<=Xr))
  {
      p.Set(xx, yy);
      return true;
  }
  else return false;
      }
}
/*
6.1.12 Overlap Function

This function decides whether two line segments overlap each other. That is, they are in the
 same (endless) line and they intersect with middle points.

*/
bool CHalfSegment::overlap( const CHalfSegment& chs ) const
{
    Coord xl,yl,xr,yr;
    Coord Xl,Yl,Xr,Yr;
    double k, a, K, A;
    double x0; //, y0;  (x0, y0) is the intersection

    if (*this==chs) return true;

    xl=lp.GetX();  yl=lp.GetY();
    xr=rp.GetX();  yr=rp.GetY();
    if (xl!=xr)
    {     //k=(yr-yl) / (xr-xl);  a=yl - k*xl;
#ifdef RATIONAL_COORDINATES
  k=((yr.IsInteger()? yr.IntValue():yr.Value()) -
        (yl.IsInteger()? yl.IntValue():yl.Value())) /
       ((xr.IsInteger()? xr.IntValue():xr.Value()) -
        (xl.IsInteger()? xl.IntValue():xl.Value()));
  a=(yl.IsInteger()? yl.IntValue():yl.Value()) -
       k*(xl.IsInteger()? xl.IntValue():xl.Value());
#else
  k=(yr-yl) / (xr-xl);
  a=yl - k*xl;
#endif
    }

    Xl=chs.GetLP().GetX();  Yl=chs.GetLP().GetY();
    Xr=chs.GetRP().GetX();  Yr=chs.GetRP().GetY();
    if (Xl!=Xr)
    {     //K=(Yr-Yl) / (Xr-Xl);  A=Yl - K*Xl;
#ifdef RATIONAL_COORDINATES
  K=  ((Yr.IsInteger()? Yr.IntValue():Yr.Value()) -
          (Yl.IsInteger()? Yl.IntValue():Yl.Value())) /
         ((Xr.IsInteger()? Xr.IntValue():Xr.Value()) -
          (Xl.IsInteger()? Xl.IntValue():Xl.Value()));
  A = (Yl.IsInteger()? Yl.IntValue():Yl.Value()) -
         K*(Xl.IsInteger()? Xl.IntValue():Xl.Value());
#else
  K=(Yr-Yl) / (Xr-Xl);
  A=Yl - K*Xl;
#endif
    }

    if ((xl==xr) && (Xl==Xr)) //both l and L are vertical lines
    {
  if (xl!=Xl) return false;
  else
  {
      Coord ylow, yup, Ylow, Yup;
      if (yl<yr)
      {
    ylow=yl;
    yup=yr;
      }
      else
      {
    ylow=yr;
    yup=yl;
      }
      if (Yl<Yr)
      {
    Ylow=Yl;
    Yup=Yr;
      }
      else
      {
    Ylow=Yr;
    Yup=Yl;
      }
      if  (((ylow<Yup)&&(ylow>Ylow))||((yup<Yup)&&(yup>Ylow)))
    return true;
      else return false;
  }
    }

    if (Xl==Xr)    //only L is vertical
    {
#ifdef RATIONAL_COORDINATES
  double y0=k*(Xl.IsInteger()? Xl.IntValue():Xl.Value())+a;
  Coord yy(y0);
#else
  double y0=k*Xl+a;
  Coord yy=y0;
#endif
  //(Xl, y0) is the intersection of l and L
  if    ((Xl>xl) &&(Xl<xr))
  {
      if (((yy>Yl) && (yy<Yr)) || ((yy>Yr) && (yy<Yl)))
              return true;
      else return false;
  }
  else return false;
    }

    if (xl==xr)    //only l is vertical
    {
#ifdef RATIONAL_COORDINATES
  double Y0=K*(xl.IsInteger()? xl.IntValue():xl.Value())+A;
  Coord YY(Y0);
#else
  double Y0=K*xl+A;
  Coord YY=Y0;
#endif
  //(xl, Y0) is the intersection of l and L
  if ((xl>Xl) && (xl<Xr))
  {
      if (((YY>yl) && (YY<yr)) || ((YY>yr) && (YY<yl)))
              return true;
      else return false;
  }
  else return false;
    }

    //otherwise: both *this and *arg are non-vertical lines
    if (k==K)
      {
    if  (A!=a)  return false; //Parallel lines
    else //they are in the same straight line
    {
        if (((xl>Xl)&&(xl<Xr)) || ((Xl>xl) && (Xl<xr)))
                return true;
        else return false;
    }
      }
      else
      {
    x0=(A-a) / (k-K);  // y0=x0*k+a;
    Coord xx(x0);
    if ((xx>xl) && (xx<xr) && (xx>Xl) && (xx <Xr))
        return true;
    else return false;
      }
}
/*
6.1.13 Inside Function

This function decides whether a line segment is inside another, that is, it is a part
of the other line segment.

*/
bool CHalfSegment::Inside(const CHalfSegment& chs) const
{ //to decide whether *this is part of *arg.
  assert( IsDefined() && chs.IsDefined() );

  if (((lp==chs.GetLP()) &&(rp==chs.GetRP())) ||
      ((lp==chs.GetRP()) &&(rp==chs.GetLP())))
  return true;

  Coord xl,yl,xr,yr;
  Coord Xl,Yl,Xr,Yr;
  double k, a, K, A;

  xl=lp.GetX();  yl=lp.GetY();
  xr=rp.GetX();  yr=rp.GetY();
  if (xl!=xr)
  {
#ifdef RATIONAL_COORDINATES
      k=((yr.IsInteger()? yr.IntValue():yr.Value()) -
           (yl.IsInteger()? yl.IntValue():yl.Value())) /
          ((xr.IsInteger()? xr.IntValue():xr.Value()) -
           (xl.IsInteger()? xl.IntValue():xl.Value()));
      a=(yl.IsInteger()? yl.IntValue():yl.Value()) -
           k*(xl.IsInteger()? xl.IntValue():xl.Value());
#else
      k=(yr - yl) / (xr - xl);
      a=yl - k*xl;
#endif
  }

  Xl=chs.GetLP().GetX();  Yl=chs.GetLP().GetY();
  Xr=chs.GetRP().GetX();  Yr=chs.GetRP().GetY();
  if (Xl!=Xr)
  {
#ifdef RATIONAL_COORDINATES
      K=  ((Yr.IsInteger()? Yr.IntValue():Yr.Value()) -
              (Yl.IsInteger()? Yl.IntValue():Yl.Value())) /
             ((Xr.IsInteger()? Xr.IntValue():Xr.Value()) -
              (Xl.IsInteger()? Xl.IntValue():Xl.Value()));
      A = (Yl.IsInteger()? Yl.IntValue():Yl.Value()) -
              K*(Xl.IsInteger()? Xl.IntValue():Xl.Value());
#else
      K=  (Yr - Yl) / (Xr - Xl);
      A = Yl - K*Xl;
#endif
    }

  if ((Xl==Xr) && (xl==xr))  //1. both are vertical lines
  {
      if (xl==Xl)
      {
    if  (((yl>=Yl) && (yl<=Yr)) && ((yr>=Yl) && (yr<=Yr)))
            return true;
    else return false;
      }
  }
  else if ((Xl!=Xr) && (xl!=xr) && (K==k) && (A==a))
               {
    if ((xl>=Xl) && (xr<=Xr)) return true;
               }

  return false;
}
/*
6.1.14 Contain Function

This function decides whether a point is inside a line segment. Semantically, if the point is
on the end points of the segment, then it is also "contained".

*/
bool CHalfSegment::Contains( const Point& p ) const
{
  assert( p.IsDefined() );

  if( !IsDefined() )
    return false;

  if ((p==lp) || (p==rp)) return true;

  Coord xl,yl,xr,yr;
  Coord X,Y;

  xl=lp.GetX();  yl=lp.GetY();
  xr=rp.GetX();  yr=rp.GetY();

  X=p.GetX(); Y=p.GetY();

    if ((xr!=xl)&&(X!=xl))
  {
      double k1, k2;
#ifdef RATIONAL_COORDINATES
      k1=  ( (Y.IsInteger()? Y.IntValue():Y.Value())-
   (yl.IsInteger()? yl.IntValue():yl.Value())) /
  ((X.IsInteger()? X.IntValue():X.Value())-
   (xl.IsInteger()? xl.IntValue():xl.Value()));

      k2=  ( (yr.IsInteger()? yr.IntValue():yr.Value())-
   (yl.IsInteger()? yl.IntValue():yl.Value())) /
  ((xr.IsInteger()? xr.IntValue():xr.Value())-
   (xl.IsInteger()? xl.IntValue():xl.Value()));
#else
      k1=  ( Y- yl) / (X- xl);
      k2=  ( yr-yl) / (xr- xl);
#endif

      if (k1== k2)
      {
    if ((xl!=xr)&&(X>=xl) && (X <=xr))
    {
        return true;
    }
    else if ((xl==xr)&&((yl<=Y)&&(Y<=yr)||(yl>=Y)&&(Y>=yr)))
    {
        return true;
    }
    else return false;
      }
      else    return false;
  }
  else if ((xr==xl)&&(X==xl))
  {
      if ((yl<=Y)&&(Y<=yr)||(yl>=Y)&&(Y>=yr))
    return true;
      else return false;
  }
  else
  {
      return false;
  }
}
/*
6.1.15 Distance Function

*/
double CHalfSegment::Distance( const Point& p ) const
{
    //this function computes the distance of a line segment and a point
    assert (( p.IsDefined())&&(this->IsDefined()));

    Coord xl,yl,xr,yr;
    xl=this->GetLP().GetX();
    yl=this->GetLP().GetY();
    xr=this->GetRP().GetX();
    yr=this->GetRP().GetY();

    Coord X=p.GetX();
    Coord Y=p.GetY();

    double result, auxresult;

    if ((xl==xr)||(yl==yr))
    {
  if (xl==xr) //chs is vertical
  {
      if (((yl<=Y)&&(Y<=yr))|| ((yr<=Y)&&(Y<=yl)))
      {
#ifdef RATIONAL_COORDINATES
    result=(X.IsInteger()? X.IntValue():X.Value())-
                (xl.IsInteger()? xl.IntValue():xl.Value());
#else
    result=X- xl;
#endif
    if (result<0) result=result*(-1);
      }
      else
      {
    result=p.Distance(this->GetLP());
    auxresult=p.Distance(this->GetRP());
    if (result > auxresult) result=auxresult;
      }
  }
  else         //chs is horizontal line: (yl==yr)
  {
      if ((xl<=X)&&(X<=xr))
      {
#ifdef RATIONAL_COORDINATES
    result=(Y.IsInteger()? Y.IntValue():Y.Value())-
                (yl.IsInteger()? yl.IntValue():yl.Value());
#else
    result=Y- yl;
#endif
    if (result<0) result=result*(-1);
      }
      else
      {
    result=p.Distance(this->GetLP());
    auxresult=p.Distance(this->GetRP());
    if (result > auxresult) result=auxresult;
      }
  }
    }
    else
    {
#ifdef RATIONAL_COORDINATES
  double k=((yr.IsInteger()? yr.IntValue():yr.Value()) -
       (yl.IsInteger()? yl.IntValue():yl.Value())) /
      ((xr.IsInteger()? xr.IntValue():xr.Value()) -
       (xl.IsInteger()? xl.IntValue():xl.Value()));
  double a=(yl.IsInteger()? yl.IntValue():yl.Value()) -
        k*(xl.IsInteger()? xl.IntValue():xl.Value());
  double xx= (k*((Y.IsInteger()? Y.IntValue():Y.Value())-a)+
        (X.IsInteger()? X.IntValue():X.Value())) / (k*k+1);
  double yy=k*xx+a;
  Coord XX(xx), YY(yy);
#else
  double k=(yr - yl) / (xr - xl);
  double a=yl - k*xl;
  double xx= (k*(Y-a)+ X) / (k*k+1);
  double yy=k*xx+a;
  Coord XX=xx;
  Coord YY=yy;
#endif
  Point PP(true, XX, YY);
  if ((xl<=XX)&&(XX<=xr))
  {
      result=p.Distance(PP);
  }
  else
  {
      result=p.Distance(this->GetLP());

      auxresult=p.Distance(this->GetRP());
      if (result > auxresult)
    result=auxresult;
  }
    }
    return (result);
}

/*
6.1.16 Rayabove Function

This function decides whether a line segment is above a given point. That is,
they are separate and the line segment is straight above the point.

*/
bool CHalfSegment::rayAbove( const Point& p, double &abovey0 ) const
{
    Coord x, y, xl, yl,xr, yr;
    x=p.GetX();
    y=p.GetY();
    xl= this->GetLP().GetX();
    yl= this->GetLP().GetY();
    xr= this->GetRP().GetX();
    yr= this->GetRP().GetY();

    bool res=false;

    if (xl!=xr)
    {

  if ((x==xl) && (yl>y))
  {
#ifdef RATIONAL_COORDINATES
      abovey0=(yl.IsInteger()? yl.IntValue():yl.Value());
#else
      abovey0=yl;
#endif
      res=true;
  }
  else if ((xl < x) && (x < xr))
  {   //Here: the problem is with the rational numbers
#ifdef RATIONAL_COORDINATES
      double k=
        ((yr.IsInteger()? yr.IntValue():yr.Value()) -
         (yl.IsInteger()? yl.IntValue():yl.Value())) /
        ((xr.IsInteger()? xr.IntValue():xr.Value()) -
         (xl.IsInteger()? xl.IntValue():xl.Value()));
      double a=
        (yl.IsInteger()? yl.IntValue():yl.Value()) -
        k*(xl.IsInteger()? xl.IntValue():xl.Value());

      double y0=
        k*(x.IsInteger()? x.IntValue():x.Value())+a;

      Coord yy(y0);
#else
      double k=  (yr - yl) / (xr - xl);
      double a=  (yl - k*xl);
      double y0=k*x+a;
      Coord yy=y0;
#endif
      if (yy>y)
      {
    abovey0=y0;
    res=true;
      }
  }
    }
    return res;
}

/*
6.1.16 Rayabove Function
   Cohen-Sutherland clipping algorithm for line P0 = (x0, y0) to P1 = (x1, y1) and
   clip rectangle with diagonal from (xmin, ymin) to (xmax, ymax)

*/

outcode CompOutCode( double x, double y, double xmin, double xmax, double ymin, double ymax)
{
  outcode code = 0;
  if (y > ymax)
    code |=TOP;
  else
    if (y < ymin)
      code |= BOTTOM;
  if ( x > xmax)
    code |= RIGHT;
  else
    if ( x < xmin)
      code |= LEFT;
  return code;

}

void CHalfSegment::CohenSutherlandLineClipping(const Rectangle<2> &window,
                            double &x0, double &y0, double &x1, double &y1,
                            bool &accept) const
{
  // Outcodes for P0, P1, and whatever point lies outside the clip rectangle*/
  outcode outcode0, outcode1, outcodeOut;
  double xmin = window.MinD(0)  , xmax = window.MaxD(0),
         ymin = window.MinD(1), ymax = window.MaxD(1);
  bool done = false;
  accept = false;

  outcode0 = CompOutCode( x0, y0, xmin, xmax, ymin, ymax);
  outcode1 = CompOutCode( x1, y1, xmin, xmax, ymin, ymax);

  do
  {
    if ( !(outcode0 | outcode1) )
    {
      //"Trivial accept and exit"<<endl;
      accept = true;
      done = true;
    }
    else
      if (outcode0 & outcode1)
      {
        done = true;
        //"Logical and is true, so trivial reject and exit"<<endl;
      }
      else
      {
      //Failed both tests, so calculate the line segment to clip:
      //from an outside point to an instersection with clip edge.
      double x,y;
      // At least one endpoint is outside the clip rectangle; pick it.
      outcodeOut = outcode0 ? outcode0 : outcode1;
      //Now finde intersection point;
      //use formulas y = y0 + slope * (x - x0), x = x0 + (1 /slope) * (y-y0).
      //"clipping line: ("<<x0<<", "<<y0<<") ( "<<x1<< ", "<<y1<<" )"<<endl;

      if (outcodeOut & TOP) //Divide the line at top of clip rectangle
      {
        x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
        y = ymax;
        //<<"TOP: "<<endl;
      }
      else
        if (outcodeOut & BOTTOM)  //Divide line at bottom edge of clip rectangle
        {
          x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
          y = ymin;
          //<<"BOTTOM: "<<endl;
        }
        else
          if (outcodeOut & RIGHT) //Divide line at right edge of clip rectangle
          {
          y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
          x = xmax;
          //<<"RIGHT: "<<endl;
        }
        else // divide lene at left edge of clip rectangle
        {
          y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
          x = xmin;
          //<<"LEFT: "<<endl;
        }
      //"clipped line: ("<<x0<<", "<<y0<<") ( "<<x1<< ", "<<y1<<" )"<<endl;
        //Now we move outside point to intersection point to clip
        //and get ready for next pass
        if (outcodeOut == outcode0)
        {
        x0 = x;
        y0 = y;
        outcode0 = CompOutCode(x0, y0, xmin, xmax, ymin, ymax);
      }
      else
      {
        x1 = x;
        y1 = y;
        outcode1 = CompOutCode(x1, y1, xmin, xmax, ymin, ymax);

      }

      }
  }
  while ( done == false);

  //"-------------------end algorithm-----------------";

}

void CHalfSegment::WindowClippingIn(const Rectangle<2> &window,
     CHalfSegment &chsInside,bool &inside, bool &isIntersectionPoint,
     Point &intersectionPoint) const
{
  double x0=this->GetLP().GetX(),
       y0=this->GetLP().GetY(),
       x1=this->GetRP().GetX(),
       y1=this->GetRP().GetY();
  CohenSutherlandLineClipping(window, x0, y0, x1, y1, inside);
  isIntersectionPoint=false;
  if (inside)
  {
    Point lp, rp;
    lp.Set(x0,y0);
    rp.Set(x1,y1);
    if (lp==rp)
    {
      isIntersectionPoint = true;
      intersectionPoint=lp;
    }
    else
    {
      AttrType attr=this->GetAttr();
      chsInside.Set(true, rp, lp);
      chsInside.SetAttr(attr);
    }
  }
}



/*

6.2 List Representation

The list representation of a HalfSegment is

----  ( bool, (Point1 Point2))  for instance: ( true ((1 1) (2 2)) )
----

where the bool value indicate whether the dominating point is the left point.

6.3 ~In~ and ~Out~ Functions

*/

ListExpr
OutHalfSegment( ListExpr typeInfo, Word value )
{
  CHalfSegment* chs;
  chs = (CHalfSegment*)(value.addr);
  if (chs->IsDefined())
  {
    Point LP, RP;
    LP = chs->GetLP();
    RP = chs->GetRP();

    return (nl->TwoElemList( nl-> BoolAtom(chs->GetLDP()),
                nl->TwoElemList( OutPoint( nl->TheEmptyList(), SetWord( &LP)),
                OutPoint( nl->TheEmptyList(), SetWord( &RP)))));
  }
  else
  {
    return (nl->SymbolAtom("undef"));
  }
}

Word
InHalfSegment( const ListExpr typeInfo, const ListExpr instance, const int errorPos, ListExpr& errorInfo, bool& correct )
{
    CHalfSegment* chs;
    ListExpr First, Second, FirstP, SecondP;
    Point *LP, *RP;
    bool LDP;

    if (nl->IsAtom(instance))
    {
  if ( nl->AtomType(instance)==SymbolType &&
       nl->SymbolValue(instance)=="undef")
  {
      correct = true;
      chs = new CHalfSegment( false );
      return SetWord(chs);
  }
  else
  {
      correct = false;
      return SetWord(Address(0));
  }
    }

    if ( nl->ListLength( instance ) == 2 )
    {
  First=nl->First(instance);
  Second=nl->Second(instance);

  if (nl->IsAtom(First) && nl->AtomType(First)==BoolType)
      LDP =  nl->BoolValue(First);
  else
  {
    correct = false;
    return SetWord(Address(0));
  }

  if (nl->ListLength(Second)==2)
  {
    FirstP = nl->First(Second);
    SecondP = nl->Second(Second);
  }
  else
  {
    correct = false;
    return SetWord(Address(0));
  }

  correct=true;
  LP = (Point*)InPoint(nl->TheEmptyList(),
           FirstP, 0, errorInfo, correct ).addr;
  if (correct)
  {
      RP = (Point*)InPoint(nl->TheEmptyList(),
               SecondP, 0, errorInfo, correct ).addr;
  }
  if (correct)
  {
      if (*LP==*RP)
      {
    cout <<">>>invalid data!<<<"<<endl;
    correct=false;
    return SetWord(Address(0));
      }

      chs = new CHalfSegment(true, LDP, *LP, *RP);
      delete LP;
      delete RP;
      return SetWord(chs);
  }
  else return SetWord(Address(0));
    }
    correct=false;
    return SetWord(Address(0));
}

/*
6.4 ~Create~-function

*/

Word
CreateHalfSegment( const ListExpr typeInfo )
{
    CHalfSegment* chs = new CHalfSegment( false );
    return (SetWord(chs));
}

/*
6.5 ~Delete~-function

*/

void
DeleteHalfSegment( const ListExpr typeInfo, Word& w )
{
  delete (CHalfSegment*) w.addr;
  w.addr = 0;
}

/*
6.6 ~Close~-function

*/

void
CloseHalfSegment( const ListExpr typeInfo, Word& w )
{
  delete (CHalfSegment*) w.addr;
  w.addr = 0;
}

/*
6.7 ~Clone~-function

*/

Word
CloneHalfSegment( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((CHalfSegment*)w.addr)->Clone());
}

/*
6.8 ~SizeOf~-function

*/
int
SizeOfHalfSegment()
{
  return sizeof(CHalfSegment);
}

/*
6.8 ~Cast~-function

*/

void*
CastHalfSegment( void* addr )
{
  return (new (addr) CHalfSegment);
}

/*

6.9 Function Describing the Signature of the Type Constructor

*/

ListExpr
HalfSegmentProperty()
{
  return (nl->TwoElemList(nl->TheEmptyList(), nl->SymbolAtom("SPATIAL2D")));
}

/*

6.10 Kind Checking Function

*/

bool
CheckHalfSegment( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual(type, "halfsegment" ));
}

/*

6.11 Creation of the Type Constructor Instance

*/

TypeConstructor halfsegment(
  "halfsegment",              //name
  HalfSegmentProperty,      //Describing signature
  OutHalfSegment,    InHalfSegment,     //Out and In functions
        0,                 0,                   //SaveToList and RestoreFromList functions
  CreateHalfSegment, DeleteHalfSegment, //object creation and deletion
  0, 0, CloseHalfSegment, CloneHalfSegment, //open, save, close, clone
  CastHalfSegment,      //cast function
  SizeOfHalfSegment,      //sizeof function
  CheckHalfSegment );     //kind checking function

/*
7 Type Constructor ~line~

A ~line~ value is a set of halfsegments. In the external (nestlist) representation, a line value is
expressed as a set of segments. However, in the internal (class) representation, it is expressed
as a set of sorted halfsegments, which are stored as a PArray.

7.1 Implementation of the class ~line~

*/
void CLine::StartBulkLoad()
{
  assert( IsOrdered() );
  ordered = false;
}

void CLine::EndBulkLoad( const bool sort )
{
  assert( !IsOrdered());
  if( sort )
    Sort();
  ordered = true;
}

CLine& CLine::operator=(const CLine& cl)
{
  assert( cl.IsOrdered() );

  line.Clear();
  int size = cl.Size();
  if(size>0){
      line.Resize( cl.Size() );
  }
  for( int i = 0; i < size; i++ )
  {
    const CHalfSegment *chs;
    cl.Get( i, chs );
    line.Put( i, *chs );
  }
  bbox = cl.bbox;
  ordered = true;
  return *this;
}

bool CLine::operator==(const CLine& cl) const
{
  assert( IsOrdered() && cl.IsOrdered() );

  if( Size() != cl.Size() )
    return false;

  if( bbox != cl.bbox )
    return false;

  for( int i = 0; i < Size(); i++ )
  {
    const CHalfSegment *chs1, *chs2;
    line.Get( i, chs1 );
    cl.Get( i, chs2 );
    if( *chs1 != *chs2 )
      return false;
  }
  return true;
}

bool CLine::operator!=(const CLine& cl) const
{
  return (!( *this == cl));
}

CLine& CLine::operator+=(const CHalfSegment& chs)
{
  assert(chs.IsDefined());

  if( IsEmpty() )
    bbox = chs.BoundingBox();
  else
    bbox = bbox.Union( chs.BoundingBox() );

  if( !IsOrdered() )
  {
    bool found=false;
    const CHalfSegment *auxchs;

    for( int i = 0; ((i < line.Size())&&(!found)); i++ )
    {
      line.Get( i, auxchs );
      if (*auxchs==chs) found=true;
    }

    if (!found)  line.Put( line.Size(), chs);
  }
  else
  {
    int pos = Position( chs );
    if( pos != -1 )
    {
      for( int i = line.Size() - 1; i >= pos; i++ )
      {
        const CHalfSegment *auxchs;
        line.Get( i, auxchs );
        line.Put( i+1, *auxchs );
      }
      line.Put( pos, chs );
    }
  }
  return *this;
}

CLine& CLine::operator-=(const CHalfSegment& chs)
{
  assert( IsOrdered() && chs.IsDefined() );

  int pos = Position( chs );
  if( pos != -1 )
  {
    for( int i = pos; i < Size(); i++ )
    {
      const CHalfSegment *auxchs;
      line.Get( i+1, auxchs );
      line.Put( i, *auxchs );
    }
  }

  // Naive way to redo the bounding box.
  if( IsEmpty() )
    bbox.SetDefined( false );
  int i = 0;
  const CHalfSegment *auxchs;
  line.Get( i++, auxchs );
  bbox = auxchs->BoundingBox();
  for( ; i < Size(); i++ )
  {
    line.Get( i, auxchs );
    bbox = bbox.Union( auxchs->BoundingBox() );
  }

  return *this;
}


int CompOutCode( const Point& p, const Rectangle<2>& r )
{
  int code = 0;

  if( p.GetY() > r.MaxD(1) )
    code |= TOP;
  else if( p.GetY() < r.MinD(1) )
    code |= BOTTOM;

  if( p.GetX() > r.MaxD(0) )
    code |= RIGHT;
  else if( p.GetX() < r.MinD(0) )
    code |= LEFT;

  return code;
}

void Clip2D( const CHalfSegment& chs, const Rectangle<2>& r, CHalfSegment& result )
{
  int outcode0, outcode1, outcodeout;
  bool accept = false, done = false;

  result.Set( chs.GetLDP(), chs.GetLP(), chs.GetRP() );
  outcode0 = CompOutCode( result.GetDPoint(), r );
  outcode1 = CompOutCode( result.GetSPoint(), r );


  do
  {
    if( !(outcode0 | outcode1) )
    {
      accept = true;
      done = true;
    }
    else if( outcode0 & outcode1 )
      done = true;
    else
    {
      Point p;

      outcodeout = outcode0 ? outcode0 : outcode1;

      if( outcodeout & TOP )
      {
        p.Set( result.GetDPoint().GetX() + (result.GetSPoint().GetX() - result.GetDPoint().GetX()) *
                (r.MaxD(1) - result.GetDPoint().GetY()) / (result.GetSPoint().GetY() - result.GetDPoint().GetY()),
               r.MaxD(1) );
      }
      else if( outcodeout & BOTTOM )
      {
        p.Set( result.GetDPoint().GetX() + (result.GetSPoint().GetX() - result.GetDPoint().GetX()) *
                 (r.MinD(1) - result.GetDPoint().GetY()) / (result.GetSPoint().GetY() - result.GetDPoint().GetY()),
               r.MinD(1) );
      }
      else if( outcodeout & RIGHT )
      {
        p.Set( r.MaxD(0),
               result.GetDPoint().GetY() + (result.GetSPoint().GetY() - result.GetDPoint().GetY()) *
                 (r.MaxD(0) - result.GetDPoint().GetX()) / (result.GetSPoint().GetX() - result.GetDPoint().GetX()) );
      }
      else
      {
        assert( outcodeout & LEFT );
        p.Set( r.MinD(0),
               result.GetDPoint().GetY() + (result.GetSPoint().GetY() - result.GetDPoint().GetY()) *
                 (r.MinD(0) - result.GetDPoint().GetX()) / (result.GetSPoint().GetX() - result.GetDPoint().GetX()) );
      }

      if( outcodeout == outcode0 )
      {
        result.Set( true, p, result.GetSPoint() );
        outcode0 = CompOutCode( p, r );
      }
      else
      {
        result.Set( true, result.GetDPoint(), p );
        outcode1 = CompOutCode( p, r );
      }
    }
  } while( done == false );

  if( !accept )
    result.SetDefined( false );
}

void CLine::Clip( const Rectangle<2>& r, CLine& result ) const
{
  assert( IsDefined() && IsOrdered() );
  assert( r.IsDefined() );
  assert( result.IsEmpty() );

  // First tests if it is empty.
  if( IsEmpty() )
    return;

  // Then tests if the bounding box intersects with r
  if( !this->BoundingBox().Intersects( r ) )
    return;

  // Then checks if the line is completely inside r
  if( r.Contains( this->BoundingBox() ) )
  {
    result.CopyFrom( this );
    return;
  }

  // Now the algorithm runs.
  const CHalfSegment *chs;
  SelectFirst();
  GetHs( chs );

  result.StartBulkLoad();
  while( !EndOfHs() )
  {
    if( chs->GetLDP() )
    {
      CHalfSegment chsr;
      Clip2D( *chs, r, chsr );
      if( chsr.IsDefined() )
      {
        result += chsr;
        chsr.SetLDP( false );
        result += chsr;
      }
    }
    SelectNext();
    GetHs( chs );
  }
  result.EndBulkLoad();
}

void CLine::InsertHs( const CHalfSegment& chs )
{
    assert(chs.IsDefined());

    if( !IsOrdered())
    {
  pos=line.Size();
  line.Put( line.Size(), chs);
    }
    else
    {
  int insertpos = Position( chs );
  if( insertpos != -1 )
  {
      for( int i = line.Size() - 1; i >= insertpos; i++ )
      {
    const CHalfSegment *auxchs;
    line.Get( i, auxchs );
    line.Put( i+1, *auxchs );
      }
      line.Put( insertpos, chs );
      pos=insertpos;
  }
    }
}

int CLine::Position( const CHalfSegment& chs) const
{
  assert( IsOrdered() && chs.IsDefined() );

  int first = 0, last = Size();

  while (first <= last)
  {
    int mid = ( first + last ) / 2;
    const CHalfSegment *midchs;
    line.Get( mid, midchs);
    if (chs > *midchs )   first = mid + 1;
    else if ( chs < *midchs)  last = mid - 1;
            else  return mid;
   }
   return -1;
}

void CLine::Sort()
{
  assert( !IsOrdered() );

  line.Sort( HalfSegmentCompare );

  ordered = true;
}

void CLine::WindowClippingIn(const Rectangle<2> &window,CLine &clippedLine,bool &inside) const
{
  inside = false;
  clippedLine.StartBulkLoad();
  for (int i=0; i < Size();i++)
  {
    const CHalfSegment *chs;
    CHalfSegment chsInside;
    bool insidechs=false,isIntersectionPoint=false;
    Get(i,chs);

    if (chs->GetLDP())
    {
      Point intersectionPoint;
      chs->WindowClippingIn(window,chsInside, insidechs,isIntersectionPoint,intersectionPoint);
      if (insidechs && !isIntersectionPoint)
      {
        clippedLine +=chsInside;
        chsInside.SetLDP(false);
        clippedLine +=chsInside;
        inside = true;
      }
    }
  }
  clippedLine.EndBulkLoad();
}

void CLine::WindowClippingOut(const Rectangle<2> &window,CLine &clippedLine,bool &outside) const
{
  outside = false;
  clippedLine.StartBulkLoad();
  for (int i=0; i < Size();i++)
  {
    const CHalfSegment *chs;
    CHalfSegment chsInside;
    bool outsidechs=false,isIntersectionPoint=false;
    Get(i,chs);

    if (chs->GetLDP())
    {
      Point intersectionPoint;
      chs->WindowClippingIn(window,chsInside, outsidechs, isIntersectionPoint,intersectionPoint);
      if (outsidechs && !isIntersectionPoint)
      {
        if (chs->GetLP()!=chsInside.GetLP())
        {//Add the part of the half segment composed by the left point of chs and
         // the left point of chsInside.
          CHalfSegment chsLeft(true,true,chs->GetLP(),chsInside.GetLP()) ;
          AttrType attr=chs->GetAttr();
          chsLeft.SetAttr(attr);
          clippedLine += chsLeft;
          chsLeft.SetLDP(false);
          clippedLine += chsLeft;
          outside = true;
        }
        if (chs->GetRP()!=chsInside.GetRP())
        {//Add the part of the half segment composed by the left point of chs and
         // the left point of chsInside.
          CHalfSegment chsRight(true,true,chs->GetRP(),chsInside.GetRP()) ;
          AttrType attr=chs->GetAttr();
          chsRight.SetAttr(attr);
          clippedLine += chsRight;
          chsRight.SetLDP(false);
          clippedLine += chsRight;
          outside = true;
        }
      }
      else
      {
        CHalfSegment aux( *chs );
        clippedLine += aux;
        aux.SetLDP(false);
        clippedLine += aux;
        outside = true;
      }
    }
  }
  clippedLine.EndBulkLoad();
}


ostream& operator<<( ostream& os, const CLine& cl )
{
  os << "<";
  for( int i = 0; i < cl.Size(); i++ )
  {
    const CHalfSegment *chs;
    cl.Get( i, chs );
    os << " " << *chs;
  }
  os << ">";
  return os;
}

size_t CLine::HashValue() const
{
    if(IsEmpty())  return (0);
    unsigned long h=0;

    const CHalfSegment *chs;
    Coord x1, y1;
    Coord x2, y2;

    for( int i = 0; ((i < Size())&&(i<5)); i++ )
    {
  Get( i, chs );
  x1=chs->GetLP().GetX();
  y1=chs->GetLP().GetY();

  x2=chs->GetRP().GetX();
  y2=chs->GetRP().GetY();

#ifdef RATIONAL_COORDINATES
  h=h+(unsigned long)
   ((5*(x1.IsInteger()? x1.IntValue():x1.Value())
     + (y1.IsInteger()? y1.IntValue():y1.Value()))+
    (5*(x2.IsInteger()? x2.IntValue():x2.Value())
     + (y2.IsInteger()? y2.IntValue():y2.Value())));
#else
  h=h+(unsigned long)((5*x1 + y1)+ (5*x2 + y2));
#endif
    }
    return size_t(h);
}

void CLine::Clear()
{
    line.Clear();
    pos=-1;
    ordered=true;
    bbox.SetDefined(false);
}

void CLine::CopyFrom(const StandardAttribute* right)
{
    const CLine *cl = (const CLine*)right;
    ordered = true;
    assert( cl->IsOrdered());
    Clear();
    for( int i = 0; i < cl->Size(); i++ )
    {
  const CHalfSegment *chs;
  cl->Get( i, chs );
  line.Put( i, *chs );
    }
    bbox=cl->BoundingBox();
}

int CLine::Compare(const Attribute * arg) const
{
  int res=0;
  const CLine* cl = (const CLine*)(arg);
  if ( !cl ) return (-2);

  if (IsEmpty() && (cl->IsEmpty()))  res=0;
  else if (IsEmpty())  res=-1;
  else  if ((cl->IsEmpty())) res=1;
  else
  {
    if (Size() > cl->Size()) res=1;
    else if (Size() < cl->Size()) res=-1;
    else  //their sizes are equal
    {
      bool bboxCmp = bbox.Compare( &cl->bbox );
      if( bboxCmp == 0 )
      {
        bool decided = false;
        for( int i = 0; ((i < Size())&&(!decided)); i++ )
        {
          const CHalfSegment *chs1, *chs2;
          Get( i, chs1);
          cl->Get( i, chs2 );

          if (*chs1 >*chs2) {res=1;decided=true;}
          else if (*chs1 < *chs2) {res=-1;decided=true;}
        }
        if (!decided) res=0;
      }
      else
        res = bboxCmp;
    }
  }
  return (res);
}

ostream& CLine::Print( ostream &os ) const
{
    os << "<";
    for( int i = 0; i < Size(); i++ )
    {
  const CHalfSegment *chs;
  Get( i, chs );
  os << " " << *chs;
    }
    os << ">";
    return os;
}

/*
7.2 List Representation

The list representation of a line is

----  ((x1 y1 x2 y2) (x1 y1 x2 y2) ....)
----

7.3 ~Out~-function

*/

ListExpr
OutLine( ListExpr typeInfo, Word value )
{

  ListExpr result, last;
  const CHalfSegment *chs;
  ListExpr halfseg, halfpoints, flatseg;

  CLine* cl = (CLine*)(value.addr);
  if( cl->IsEmpty())
  {
    return (nl->TheEmptyList());
  }
  else
  {
    result = nl->TheEmptyList();
    last = result;
    bool firstitem=true;

    for( int i = 0; i < cl->Size(); i++ )
    {
      cl->Get( i, chs );
      if ((chs->IsDefined())&&(chs->GetLDP()==true))
      {
    CHalfSegment aux( *chs );
    halfseg = OutHalfSegment( nl->TheEmptyList(), SetWord( &aux ) );
    halfpoints=nl->Second( halfseg );
    flatseg = nl->FourElemList(nl->First(nl->First( halfpoints )),
          nl->Second(nl->First( halfpoints )),
          nl->First(nl->Second( halfpoints )),
          nl->Second(nl->Second( halfpoints )));
    if (firstitem==true)
    {
        result=nl->OneElemList( flatseg );
        last = result;
        firstitem=false;
    }
    else
    {
        last = nl->Append( last, flatseg );
    }
      }
    }
    return result;
  }
}

/*
7.4 ~In~-function

*/
Word
InLine( const ListExpr typeInfo, const ListExpr instance, const int errorPos, ListExpr& errorInfo, bool& correct )
{
  CLine* cl = new CLine( 0 );
  CHalfSegment * chs;
  cl->StartBulkLoad();
  ListExpr first, halfseg, halfpoint;
  ListExpr rest = instance;

  if (!nl->IsAtom(instance))
  {
      while( !nl->IsEmpty( rest ) )
      {
    first = nl->First( rest );
    rest = nl->Rest( rest );

    if (nl->ListLength( first ) != 4)
    {
        correct=false;
        return SetWord( Address(0) );
    }
    else
    {
        halfpoint=nl->TwoElemList(nl->TwoElemList
              (nl->First(first), nl->Second(first)),
              nl->TwoElemList
              (nl->Third(first), nl->Fourth(first)));
    }
    halfseg = nl->TwoElemList(nl-> BoolAtom(true),halfpoint);
    chs = (CHalfSegment*)InHalfSegment
               ( nl->TheEmptyList(), halfseg,
    0, errorInfo, correct ).addr;
    if( correct )
    {   //every point is added twice
        (*cl) += (*chs);
        chs->SetLDP(false);
        (*cl) += (*chs);
        delete chs;
    }
    else
    {
        return SetWord( Address(0) );
    }
      }
      cl->EndBulkLoad();
      correct = true;
      return SetWord( cl );
  }
  else
  {
      correct=false;
      return SetWord( Address(0) );
  }
}

/*
7.5 ~Create~-function

*/
Word
CreateLine( const ListExpr typeInfo )
{
  return (SetWord( new CLine( 0 ) ));
}

/*
7.6 ~Delete~-function

*/
void
DeleteLine( const ListExpr typeInfo, Word& w )
{
  CLine *cl = (CLine *)w.addr;
  cl->Destroy();
  delete cl;
  w.addr = 0;
}

/*
7.7 ~Close~-function

*/
void
CloseLine( const ListExpr typeInfo, Word& w )
{
  //  cout << "CloseLine" << endl;
  delete (CLine *)w.addr;
  w.addr = 0;
}

/*
7.8 ~Clone~-function

*/
Word
CloneLine( const ListExpr typeInfo, const Word& w )
{
  //  cout << "CloneLine" << endl;
  CLine *cl = new CLine(*((CLine *)w.addr) );
  return SetWord( cl );
}

/*
7.9 ~SizeOf~-function

*/
int SizeOfLine()
{
  return sizeof(CLine);
}

/*
7.11 Function describing the signature of the type constructor

*/
ListExpr
LineProperty()
{
  return (nl->TwoElemList(

            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("line"),
           nl->StringAtom("(<segment>*) where segment is "
           "(<x1><y1><x2><y2>)"),
           nl->StringAtom("( (1 1 2 2)(3 3 4 4) )"))));
}

/*
7.12 Kind checking function

This function checks whether the type constructor is applied correctly. Since
type constructor ~line~ does not have arguments, this is trivial.

*/

bool
CheckLine( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "line" ));
}

/*
7.13 ~Cast~-function

*/
void* CastLine(void* addr)
{
  return (new (addr) CLine);
}

/*
7.14 Creation of the type constructor instance

*/
TypeConstructor line(
        "line",                         //name
        LineProperty,                   //describing signature
        OutLine,        InLine,         //Out and In functions
        0,              0,              //SaveToList and RestoreFromList functions
        CreateLine,     DeleteLine,     //object creation and deletion
        0,        0,        // object open and save
        CloseLine,      CloneLine,      //object close and clone
        CastLine,                       //cast function
        SizeOfLine,     //sizeof function
        CheckLine );                      //kind checking function

/*
8 Type Constructor ~region~

A ~region~ value is a set of halfsegments. In the external (nestlist) representation, a region value is
expressed as a set of faces, and each face is composed of a set of cycles.  However, in the internal
(class) representation, it is expressed as a set of sorted halfsegments, which are stored as a PArray.

The system will do the basic check on the validity of the region data (see the explaination of the
insertOK() function).

8.1 Implementation of the class ~region~

*/
bool CRegion::Valid() const
{
  const CHalfSegment *s;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, s );
    if( !s->IsDefined() || !s->Valid() )
      return false;
  }
  return true;
}

void CRegion::StartBulkLoad()
{
  assert( IsOrdered() );
  ordered = false;
}

void CRegion::EndBulkLoad( const bool sort )
{
  //1. Original EndBulkload code
  assert( !IsOrdered());

  if( sort )
    Sort();

  //2. here: linean scan to create coverage number
  const CHalfSegment *chs;
  int currCoverageNo = 0;

  for( int i = 0; i < this->Size(); i++ )
  {
    this->Get( i, chs );

    if( chs->GetLDP() )
      currCoverageNo++;
    else
      currCoverageNo--;

    CHalfSegment aux( *chs );
    aux.attr.coverageno = currCoverageNo;
    Put( i, aux );
  }

  ordered = true;
}

bool CRegion::contain_old( const Point& p ) const
{
    if (!p.Inside(bbox)) return false;

    int faceISN[100];

    int lastfaceno=-1;
    for (int i=0; i<100; i++)
    {
  faceISN[i]=0;
    }

    const CHalfSegment *chs;

    for (int i=0; i<this->Size(); i++)
    {
  this->Get(i, chs);
  double y0;

  if  ((chs->GetLDP()) &&(chs->Contains(p)))
      return true;

  if ((chs->GetLDP()) &&(chs->rayAbove(p, y0)))
  {
      faceISN[chs->attr.faceno]++;
      if (lastfaceno < chs->attr.faceno)
    lastfaceno=chs->attr.faceno;
  }
    }

    for (int j=0; j<=lastfaceno; j++)
    {
  if (faceISN[j] %2 !=0 )
  {
      return true;
  }
    }
    return false;
}

bool CRegion::contain( const Point& p ) const
{
    //here: if the point is on the border, it is also counted.
    //used in inside operator

    if (!p.Inside(bbox)) return false;

    int faceISN[100];

    int lastfaceno=-1;
    for (int i=0; i<100; i++)
    {
      faceISN[i]=0;
    }

    const CHalfSegment *chs;

    /*  ======================================================
    //This part will be replaced by the new method from Ralf.
    for (int i=0; i<this->Size(); i++)
    {
  this->Get(i, chs);
  double y0;

  if  ((chs.GetLDP()) &&(chs.Contains(p)))
      return true;

  //if ((chs.GetLDP())&&( (chs.GetLP().GetX() <= p.GetX())&&(p.GetX() <= chs.GetRP().GetX()) ))
  //    cout<<"eligable: "<<chs<<endl;

  if ((chs.GetLDP()) &&(chs.rayAbove(p, y0)))
  {
      faceISN[chs.attr.faceno]++;
      if (lastfaceno < chs.attr.faceno)
    lastfaceno=chs.attr.faceno;
  }
    }
    //======================================================*/
    //*********Here: New Method by Ralf.*********

    int coverno=0;
    int startpos=0;
    double y0;

    //1. find the right place by binary search
    startpos = Position( p );

    //int chsVisited2 = int(((log (this->Size())) / (log (2))) + 0.5);

    int chsVisiteds=0;

    if ( startpos == -1 )   //p is smallest
       return false;
    else if ( startpos == -2 )  //p is largest
       return false;
    else if ( startpos == -3 )  //p is a vertex
       return true;

    //2. deal with equal-x chs's
    bool continuemv=true;
    int i=startpos;

    while ((continuemv) && (i>=0))
    {
      region.Get( i, chs );
      chsVisiteds++;

      if (chs->GetDPoint().GetX() == p.GetX())
      {
        if  (chs->Contains(p))  return true;

        if (chs->GetLDP())
        {
          if (chs->rayAbove(p, y0))
          {
            faceISN[chs->attr.faceno]++;
            if (lastfaceno < chs->attr.faceno)
              lastfaceno=chs->attr.faceno;
          }
       }

       i--;
      }
      else continuemv=false;
    }       //now i is pointing to the last chs whose DP.X != p.x


    //3. get the coverage value

    //cout<<"i= "<<i<<endl;  //************************
    //if (i<0) cout<<"P= "<<p<<"R="<<*this<<endl;
    if (i>0) { //$$$$$$$$$$$$$$this line is added to avoid abnormal access.

    region.Get( i, chs );
    coverno=chs->attr.coverageno;

    //cout<<"real starting position at: "<<chs<<endl;
    //cout<<"real-startpos: "<< i <<"  coverno: "<<coverno<<endl;

    //4. search the region value for coverageno steps
    int touchedNo=0;

    while (( i>=0)&&(touchedNo<coverno))
    {
      this->Get(i, chs);
      chsVisiteds++;

      if  (chs->Contains(p))  return true;

      if ((chs->GetLDP())&&((chs->GetLP().GetX() <= p.GetX())&&(p.GetX() <= chs->GetRP().GetX()) ))
      {
        //cout<<"ELIGABLE: "<<chs<<endl;
        touchedNo++;
      }

      if (chs->GetLDP())
      {
        if (chs->rayAbove(p, y0))
        {
          faceISN[chs->attr.faceno]++;
          if (lastfaceno < chs->attr.faceno)
            lastfaceno=chs->attr.faceno;
        }
      }

      i--;  //the iterator
   }
  } //$$$$$$$$$added by DZM to avoid abnormal access

    //cout<<"number of chs actually checked: "<<chsVisited2<<" + "<<chsVisiteds<<endl;
    // ================= End of the new method ================= */

    for (int j=0; j<=lastfaceno; j++)
    {
      if (faceISN[j] %2 !=0 )
      {
        return true;
      }
    }
    return false;
}

bool CRegion::containpr( const Point& p, int &pathlength, int & scanned ) const
{
    //here: if the point is on the border, it is also counted.
    //used in inside_pathlength and inside_scaned operators

    pathlength=0;
    scanned=0;

    if (!p.Inside(bbox)) return false;

    int faceISN[100];

    int lastfaceno=-1;
    for (int i=0; i<100; i++)
    {
  faceISN[i]=0;
    }

    const CHalfSegment *chs;

    //cout<<"============================================"<<endl;
    //cout<<"number of chs in the region: "<<this->Size()<<endl;

    //cout<<"!!!the region value is!!!: "<<endl;
    //for (int i=0; i<this->Size(); i++)
    //{
    //  this->Get(i, chs);
    //  cout<<chs<<endl;
    //}
    //cout<<"!!!the point value is!!!: "<<endl;
    //cout<<p<<endl;

    /*  ======================================================
    //This part will be replaced by the new method from Ralf.
    for (int i=0; i<this->Size(); i++)
    {
  this->Get(i, chs);
  double y0;

  if  ((chs.GetLDP()) &&(chs.Contains(p)))
      return true;

  if ((chs.GetLDP())&&( (chs.GetLP().GetX() <= p.GetX())&&(p.GetX() <= chs.GetRP().GetX()) ))
      cout<<"eligable: "<<chs<<endl;

  if ((chs.GetLDP()) &&(chs.rayAbove(p, y0)))
  {
      faceISN[chs.attr.faceno]++;
      if (lastfaceno < chs.attr.faceno)
    lastfaceno=chs.attr.faceno;
  }
    }
    //======================================================

    */
    //*********Here: New Method by Ralf.*********

    int coverno=0;
    int startpos=0;
    double y0;

    //1. find the right place by binary search
    startpos = Position( p );

    int chsVisited2 = (int)(((log ((double)this->Size())) / (log ((double)2))) + 0.5);

    int chsVisiteds=0;

    if ( startpos == -1 )   //p is smallest
  return false;
    else if ( startpos == -2 )  //p is largest
  return false;
    else if ( startpos == -3 )  //p is a vertex
  return true;

    //2. deal with equal-x chs's
    bool continuemv=true;
    int i=startpos;

    while ((continuemv) && (i>=0))
    {
  region.Get( i, chs );
  chsVisiteds++;

  if (chs->GetDPoint().GetX() == p.GetX())
  {
      if  (chs->Contains(p))  return true;

      if (chs->GetLDP())
      {
    //cout<<"ELIGABLE**: "<<chs<<endl;
    if (chs->rayAbove(p, y0))
    {
        faceISN[chs->attr.faceno]++;
        if (lastfaceno < chs->attr.faceno)
      lastfaceno=chs->attr.faceno;
    }
      }

  i--;
  }
  else continuemv=false;
    }       //now i is pointing to the last chs whose DP.X != p.x


    //3. get the coverage value
    region.Get( i, chs );
    coverno=chs->attr.coverageno;

    //cout<<"real starting position at: "<<chs<<endl;
    //cout<<"real-startpos: "<< i <<"  coverno: "<<coverno<<endl;

    //4. search the region value for coverageno steps
    int touchedNo=0;

    while (( i>=0)&&(touchedNo<coverno))
    {
  this->Get(i, chs);
  chsVisiteds++;

  if  (chs->Contains(p))  return true;

  if ((chs->GetLDP())&&((chs->GetLP().GetX() <= p.GetX())&&(p.GetX() <= chs->GetRP().GetX()) ))
  {
        //cout<<"ELIGABLE: "<<chs<<endl;
        touchedNo++;
  }

  if (chs->GetLDP())
  {
      if (chs->rayAbove(p, y0))
      {
    faceISN[chs->attr.faceno]++;
    if (lastfaceno < chs->attr.faceno)
        lastfaceno=chs->attr.faceno;
      }
  }

  i--;  //the iterator
    }
    //cout<<"number of chs actually checked: "<<chsVisited2<<" + "<<chsVisiteds<<endl;

    pathlength=chsVisited2;
    scanned=chsVisiteds;

    // ================= End of the new method =================

    for (int j=0; j<=lastfaceno; j++)
    {
  if (faceISN[j] %2 !=0 )
  {
      return true;
  }
    }
    return false;
}

bool CRegion::innercontain( const Point& p ) const
{
    //onborder points are not counted.

    if (!p.Inside(bbox)) return false;

    int faceISN[100];

    int lastfaceno=-1;
    for (int i=0; i<100; i++)
    {
  faceISN[i]=0;
    }

    const CHalfSegment *chs;

    for (int i=0; i<this->Size(); i++)
    {
  this->Get(i, chs);
  double y0;

  if  ((chs->GetLDP()) &&(chs->Contains(p)))
      return false;

  if ((chs->GetLDP()) &&(chs->rayAbove(p, y0)))
  {
      faceISN[chs->attr.faceno]++;
      if (lastfaceno < chs->attr.faceno)
    lastfaceno=chs->attr.faceno;
  }
    }

    for (int j=0; j<=lastfaceno; j++)
    {
  if (faceISN[j] %2 !=0 )
  {
      return true;
  }
    }
    return false;
}

bool CRegion::contain( const CHalfSegment& chs ) const
{
    //onborder cases are also counted as contain.

    if ((!chs.GetLP().Inside(bbox)) || (!chs.GetRP().Inside(bbox))) return false;

    if ((!(this->contain(chs.GetLP())))||(!(this->contain(chs.GetRP()))))
    {
      return false;
    }

    const CHalfSegment *auxchs;
    struct {
      int faceno;
      int cycleno;
      int edgeno;
    } touchset[10];
    int touchnum=0;

    //now we know that both endpoints of chs is inside region
     for (int i=0; i<this->Size(); i++)
    {
  this->Get(i, auxchs);
  if (auxchs->GetLDP())
  {
      if (chs.cross(*auxchs))
      {
    return false;
      }
      else if (chs.Inside(*auxchs))
      {       //chs is part of the border
    return true;
      }
      else //two cases: not intersect or intersect
      {
    if (chs.Intersects(*auxchs))
    {
        if ((auxchs->Contains(chs.GetLP()))||
            (auxchs->Contains(chs.GetRP())))
        {
      bool found=false;
      for (int k=0; k<touchnum;k++)
      {
          if ((touchset[k].faceno==auxchs->attr.faceno)&&
              (touchset[k].cycleno==auxchs->attr.cycleno)&&
              (touchset[k].edgeno==auxchs->attr.edgeno))
        found=true;
      }
      if (found==false)
      {
          touchset[touchnum].faceno=auxchs->attr.faceno;
          touchset[touchnum].cycleno=auxchs->attr.cycleno;
          touchset[touchnum].edgeno=auxchs->attr.edgeno;
          touchnum++;
      }
        }
    }
      }
  }
    }

    if (touchnum > 1)
    {
  //it is safe to do so since in the middle chs1 intersect with nothing
  Coord midx=chs.GetLP().GetX();
  midx += chs.GetRP().GetX();
  midx /= (long)2 ;
  Coord midy=chs.GetLP().GetY();
  midy += chs.GetRP().GetY();
  midy /= (long)2 ;

  Point midp(true, midx, midy);
  if (this->contain(midp))
  {
      return true;
  }
  else
  {
      return false;
  }
    }
    else
    {
  return true;
    }
}

bool CRegion::holeedgecontain( const CHalfSegment& chs ) const
{
    const CHalfSegment *auxchs;

    for (int i=0; i<this->Size(); i++)
    {
  this->Get(i, auxchs);
  if ((auxchs->GetLDP()) && (auxchs->attr.cycleno>0) &&(chs.Inside(*auxchs)))
  {
      return true;
  }
    }
    return false;
}
/*
4.4.7 Operation ~intersects~

*/
bool CRegion::Intersects(const CRegion &r) const
{

  const CHalfSegment *chs1, *chs2;
  if(! this->BoundingBox().Intersects( this->BoundingBox() ) )
    return false;

    //cout<<"computing..."<<endl;
    //cout <<"cr1 size: "<<cr1->Size()<<endl;

    //1. decide the intersection of edges
  for (int i=0; i< Size(); i++)
  {
    this->Get(i, chs1);
    if (chs1->GetLDP())
    {
      for (int j=0; j<r.Size(); j++)
      {
        r.Get(j, chs2);
        if (chs2->GetLDP() && chs1->Intersects(*chs2) )
            return true;
      }
    }
  }

    //2. decide the case of Tong-Xin-Yuan
  for (int i=0; i < Size(); i++)
  {
    this->Get(i, chs1);
    if ( chs1->GetLDP() && r.contain( chs1->GetLP() ) )
        return true;
  }

  for (int j=0; j< r.Size(); j++)
  {
    r.Get(j, chs2);
    if (chs2->GetLDP() && this->contain(chs2->GetLP()) )
      return true;
  }

  //3. else: not intersect
  return false;
}

void CRegion::Components( vector<CRegion*>& components )
{
  CRegion *copy = new CRegion( *this );
  copy->logicsort();

  map<int,int> edgeno,
               cycleno,
               faceno;

  for( int i = 0; i < Size(); i++ )
  {
    const CHalfSegment *chs;
    copy->Get( i, chs );
    CRegion *r;
    CHalfSegment aux( *chs );
    if( faceno.find( chs->attr.faceno ) == faceno.end() )
    {
      r = new CRegion( 1 );
      r->StartBulkLoad();
      components.push_back( r );
      aux.attr.faceno = faceno.size();
      faceno.insert( make_pair( chs->attr.faceno, aux.attr.faceno ) );
    }
    else
    {
      aux.attr.faceno = faceno[ chs->attr.faceno ];
      r = components[ aux.attr.faceno ];
    }

    if( cycleno.find( chs->attr.cycleno ) == cycleno.end() )
    {
      aux.attr.cycleno = cycleno.size();
      cycleno.insert( make_pair( chs->attr.cycleno, aux.attr.cycleno ) );
    }
    else
      aux.attr.cycleno = cycleno[ chs->attr.cycleno ];

    if( edgeno.find( chs->attr.edgeno ) == edgeno.end() )
    {
      aux.attr.edgeno = edgeno.size();
      edgeno.insert( make_pair( chs->attr.edgeno, aux.attr.edgeno ) );
    }
    else
      aux.attr.edgeno = edgeno[ chs->attr.edgeno ];

    *r += aux;
  }

  for( size_t i = 0; i < components.size(); i++ )
  {
    components[i]->EndBulkLoad( true );
    components[i]->SetPartnerNo();
  }
}

CRegion& CRegion::operator=(const CRegion& cr)
{
  //cout<<"CRegion::operator="<<endl;
  assert( cr.IsOrdered() );

  region.Clear();
  int size = cr.Size();
  if(size>0){
      region.Resize( size );
  }
  for( int i = 0; i < size; i++ )
  {
    const CHalfSegment *chs;
    cr.Get( i, chs );
    Put( i, *chs );
  }
  bbox = cr.BoundingBox();
  ordered = true;
  return *this;
}

bool CRegion::operator==(const CRegion& cr) const
{
  assert( IsOrdered() && cr.IsOrdered() );
  if( Size() != cr.Size() )    return 0;

  if ( bbox != cr.BoundingBox()) return 0;

  for( int i = 0; i < Size(); i++ )
  {
    const CHalfSegment *chs1, *chs2;
    region.Get( i, chs1 );
    cr.Get( i, chs2 );
    if( *chs1 != *chs2 )
      return 0;
  }
  return 1;
}

bool CRegion::operator!=(const CRegion &cr) const
{
  return !(*this==cr);
}

CRegion& CRegion::operator+=(const CHalfSegment& chs)
{
  assert(chs.IsDefined());

  if( IsEmpty() )
    bbox = chs.BoundingBox();
  else
    bbox = bbox.Union( chs.BoundingBox() );

  if( !IsOrdered() )
  {
    Put( region.Size(), chs);
  }
  else
  {
    int pos = Position( chs );
    if( pos != -1 )
    {
      for( int i = region.Size() - 1; i >= pos; i++ )
      {
        const CHalfSegment *auxchs;
        Get( i, auxchs );
        Put( i+1, *auxchs );
      }
      Put( pos, chs );
    }
  }
  return *this;
}

CRegion& CRegion::operator-=(const CHalfSegment& chs)
{
  assert( IsOrdered() && chs.IsDefined() );

  int pos = Position( chs );
  if( pos != -1 )
  {
    for( int i = pos; i < Size(); i++ )
    {
      const CHalfSegment *auxchs;
      Get( i+1, auxchs );
      Put( i, *auxchs );
    }
  }

  // Naive way to redo the bounding box.
  if( IsEmpty() )
    bbox.SetDefined( false );
  int i = 0;
  const CHalfSegment *auxchs;
  region.Get( i++, auxchs );
  bbox = auxchs->BoundingBox();
  for( ; i < Size(); i++ )
  {
    region.Get( i, auxchs );
    bbox = bbox.Union( auxchs->BoundingBox() );
  }

  return *this;
}

void CRegion::InsertHs( const CHalfSegment& chs )
{
    assert(chs.IsDefined());


    if( !IsOrdered())
    {
  pos=region.Size();
  Put( region.Size(), chs);
    }
    else
    {
  int insertpos = Position( chs );
  if( insertpos != -1 )
  {
      for( int i = region.Size() - 1; i >= insertpos; i++ )
      {
    const CHalfSegment *auxchs;
    Get( i, auxchs );
    Put( i+1, *auxchs );
      }
      Put( insertpos, chs );
      pos=insertpos;
  }
    }
}

/*
searches (binary search algorithm) for a half segment in the region value and
returns its position. Returns -1 if the half segment is not found.

*/

int CRegion::Position( const CHalfSegment& chs) const
{
  assert( IsOrdered() && chs.IsDefined() );

  int first = 0, last = Size();

  while (first <= last)
  {
    int mid = ( first + last ) / 2;
    const CHalfSegment *midchs;
    region.Get( mid, midchs);
    if (chs > *midchs )   first = mid + 1;
    else if ( chs < *midchs)  last = mid - 1;
            else  return mid;
   }
   return -1;
}

int CRegion::Position( const Point& p) const
{
  //to find the exact position by comparing the (x y) values
  assert( IsOrdered() && p.IsDefined() );

  const CHalfSegment *chs;
  int first = 0, last = Size()-1;
  int mid = ( first + last ) / 2;
  int res=-1;

  //1. check special occassions
  region.Get( 0, chs);
  if ( p < chs->GetDPoint() ) return -1;  //p is smallest
  else if ( p == chs->GetDPoint() ) return -3;  //p equals to an vertex

  region.Get( Size()-1, chs);
  if ( p > chs->GetDPoint() ) return -2;  //p is largest
  else if ( p == chs->GetDPoint() ) return -3;

  //2. p is in the middle of the array
  while (first <= last)
  {
    mid = ( first + last ) / 2;
    region.Get( mid, chs);
    if ( p > chs->GetDPoint() )   first = mid + 1;
    else if ( p < chs->GetDPoint() )  last = mid - 1;
    else  return -3;
   }

  res=last;  //at this time, last is smaller than first
  bool exact=false;
  while ((!exact)&&(res<Size()))
  {
      region.Get( res, chs);

      if ( p > chs->GetDPoint() )  res++;
      else if ( p < chs->GetDPoint() )
      {
    exact=true;
    res--;
      }
      else return -3;  //p is equal to the endpoint
  }
  //now res is pointing to the last chs whose DP is "just smaller" than P

  //3. move forward (to the larger chs) until the x-coordinate is no longer equal
  bool continuemv=true;
  int samexp=res+1;

  while ((continuemv) && (samexp<Size()))
  {
      region.Get( samexp, chs );
      if (chs->GetDPoint().GetX() == p.GetX())
      {
    samexp++;
      }
      else continuemv=false;
  }

  samexp--;  //now samexp is pointing to the last chs whose x-coordinate is equal to p.x

  //4. return the result
  return samexp;
}

void CRegion::Sort()
{
  assert( !IsOrdered() );

  region.Sort( HalfSegmentCompare );

  ordered = true;
}

void CRegion::logicsort()
{
  region.Sort( HalfSegmentLogCompare );
  ordered = false;
}

ostream& operator<<( ostream& os, const CRegion& cr )
{
  os << "<"<<endl;
  for( int i = 0; i < cr.Size(); i++ )
  {
    const CHalfSegment *chs;
    cr.Get( i, chs );
    os << " " << *chs<<endl;
  }
  os << ">";
  return os;
}

void CRegion::SetPartnerNo()
{
  assert( IsOrdered() );

  if (this->Size()<=0)
    return;

  const CHalfSegment *chs;
  int *pa = new int[Size()/2];

  for( int i = 0; i < Size(); i++)
  {
    Get( i, chs );
    if (chs->GetLDP())
    {
      //store at position partnerno of the partner array the position of the left half segment
      //in the half segment array
      assert( chs->attr.edgeno >= 0 && 
              chs->attr.edgeno <= Size()/2 );
      pa[chs->attr.edgeno]=i;
    }
    else
    {
      const CHalfSegment *chsLeft;
      //assign the position of the right dominating half segment as the partner number of the right half segment
      CHalfSegment aux( *chs );
      aux.attr.partnerno = pa[chs->attr.edgeno];
      UpdateAttr(i, aux.attr);
      Get( chs->attr.partnerno, chsLeft );
      //update the partner number of the left dominating half segment to the position of the right half segment
      //in the half segment array.
      aux = *chsLeft;
      aux.attr.partnerno = i;
      UpdateAttr(chs->attr.partnerno, aux.attr);
    }
  }
  delete []pa;
}

double VectorSize(const Point &p1, const Point &p2)
{
  double size = pow( (p1.GetX() - p2.GetX()),2) + pow( (p1.GetY() - p2.GetY()),2);
  size = sqrt(size);
  return size;
}
//The angle function returns the angle of VP1P2
// P1 is the point on the window's edge
double Angle(const Point &v, const Point &p1,const Point &p2)
{
  double coss;

  //If P1P2 is vertical and the window's edge been tested is horizontal , then
  //the angle VP1P2 is equal to 90 degrees. On the other hand, if P1P2 is vertical
  //and the window's edge been tested is vertical, then thte angle is 90 degrees.
  //Similar tests are applied when P1P2 is horizontal.

  if (p1.GetX() == p2.GetX()) //the segment is vertical
    if (v.GetY()==p1.GetY()) return PI/2; //horizontal edge
    else return 0;
  if (p1.GetY() == p2.GetY()) //the segment is horizontal
    if (v.GetY()==p1.GetY()) return 0; //horizontal edge
    else return PI/2;

  coss = double( ( (v.GetX() - p1.GetX()) * (p2.GetX() - p1.GetX()) ) +
                 ( (v.GetY() - p1.GetY()) * (p2.GetY() - p1.GetY()) ) ) /
                 (VectorSize(v,p1) * VectorSize(p2,p1));
  //cout<<endl<<"Coss"<<coss;
  //coss = abs(coss);
  //cout<<endl<<"Coss"<<coss;
  return acos(coss);
}


ostream& operator<<( ostream& o, const EdgePoint & p )
{
  if( p.IsDefined() )
    o << "(" << p.GetX() << ", " << p.GetY() << ")"
      <<" D("<<(p.direction ? "LEFT/DOWN" : "RIGHT/UP")<<")"
      <<" R("<<(p.rejected ? "Rejected" : "Accepted")<<")";
  else
    o << "undef";

  return o;
}

EdgePoint* EdgePoint::GetEdgePoint(const Point &p,const Point &p2,bool insideAbove,
                                   const Point &v, const bool reject)
{
  //The point p2 must be outside the window
  bool direction;

  //window's vertical edge
  if (v.GetX()==p.GetX())
  {
    if (insideAbove)
      direction =  false; //UP
    else
      direction =  true; //DOWN
  }
  else  //Horizontal edge
  {
    if (insideAbove)
    {
      if ( (p.GetX()-p2.GetX())>0 ) //p2.x is located to the left of p.x
        direction =  false; //RIGHT
      else
        direction =  true; //LEFT
    }
    else
    {
      if ( (p.GetX()-p2.GetX())>0 )//p2.x is located to the right of p.x
        direction =  true; //LEFT
      else
        direction =  false; //RIGHT
    }
  }
  return new EdgePoint(p,direction,reject);

}

void AddPointToEdgeArray(const Point &p,const CHalfSegment &chs,
                       const Rectangle<2> &window,vector<EdgePoint> pointsOnEdge[4])
{
  EdgePoint *dp;
  Point v;
  AttrType attr;
  attr = chs.GetAttr();
  Point p2;
  //If the left and right edges are been tested then it is not need to check the angle
  //between the half segment and the edge. If the attribute inside above is true, then
  //the direction is up (false), otherwise it is down (true).
  if (p.GetX() == window.MinD(0))
  {
    dp = new EdgePoint(p,!attr.insideAbove,false);
    pointsOnEdge[WLEFT].push_back(*dp);
  }
  else
    if (p.GetX() == window.MaxD(0))
    {
      dp = new EdgePoint(p,!attr.insideAbove,false);
      pointsOnEdge[WRIGHT].push_back(*dp);
    }
  if (p.GetY() == window.MinD(1))
  {
    v.Set(window.MinD(0), window.MinD(1));
    //In this case we don't know which point is outside the window,
    //so it is need to test both half segment's poinst. Moreover,
    //in order to use the same comparison that is used for
    //Top edge, it is need to choose the half segment point that
    //is over the bottom edge.
    if (chs.GetLP().GetY()>window.MinD(1))
      dp = EdgePoint::GetEdgePoint(p,chs.GetLP(),attr.insideAbove,v,false);
    else
      dp = EdgePoint::GetEdgePoint(p,chs.GetRP(),attr.insideAbove,v,false);
    pointsOnEdge[WBOTTOM].push_back(*dp);
  }
  else
    if (p.GetY() == window.MaxD(1))
    {
      v.Set(window.MinD(0), window.MaxD(1));
    //In this case we don't know which point is outside the window,
    //so it is need to test
    if (chs.GetLP().GetY()>window.MaxD(1))
      dp = EdgePoint::GetEdgePoint(p,chs.GetLP(),attr.insideAbove,v,false);
    else
      dp = EdgePoint::GetEdgePoint(p,chs.GetRP(),attr.insideAbove,v,false);
      pointsOnEdge[WTOP].push_back(*dp);
    }
}

bool GetAcceptedPoint(vector <EdgePoint>pointsOnEdge,int &i,const int &end,
                      EdgePoint &ep)
{
  //id is the indice of the current point in the scan
  //ep is the correct edge point that will be returned.
  ep = pointsOnEdge[i];
  //discard all rejected points
  while (ep.rejected && i<=end)
  {
    i++;
    if (i>end)
      return false;
    EdgePoint epAux = pointsOnEdge[i];
    //Discard all the points that was accepted but has a corresponding rejection point.
    //In other words, point that has the same coordinates and direction on the edge.
    if (!epAux.rejected && (epAux.direction==ep.direction) &&
         (epAux.GetX() == ep.GetX()) && (epAux.GetY() == ep.GetY()) )
    {
      while ( (i<=end) && (epAux.direction==ep.direction) &&
         (epAux.GetX() == ep.GetX()) && (epAux.GetY() == ep.GetY()) )
      {
        i++;
        if (i>end)
          return false;
        epAux = pointsOnEdge[i];
      }
    }
    ep = epAux;
  }
  return true;
}

void CRegion::CreateNewSegments(vector <EdgePoint>pointsOnEdge, CRegion &cr,
                                const Point &bPoint,const Point &ePoint,
                               WindowEdge edge,int &partnerno,
                               bool inside)
//The inside attribute indicates if the points on edge will originate
//segments that are inside the window (its values is true), or outside
//the window (its value is false)
{
  int begin, end, i;
  CHalfSegment *chs;
  AttrType attr;
  EdgePoint dp,dpAux;

  if (pointsOnEdge.size()==0) return;
/*
  for (int j=0;j<pointsOnEdge.size();j++)
    cout<<endl<<j<<": "<<pointsOnEdge[j];

*/
  sort(pointsOnEdge.begin(),pointsOnEdge.end());

/*
  for (int j=0;j<pointsOnEdge.size();j++)
    cout<<endl<<j<<": "<<pointsOnEdge[j];

*/
  begin = 0;
  end = pointsOnEdge.size()-1;

  dp = pointsOnEdge[begin];
  if ( dp.direction)//dp points to left or down
  {

    if (!dp.rejected)
    {
      //If dp is a rejected point then it must not be considered
      //as point to be connected to the window edge
      chs = new CHalfSegment(true,true, bPoint, dp);

      attr.partnerno = partnerno;
      partnerno++;
      if ( (edge == WTOP) || (edge == WLEFT) )
        attr.insideAbove = !inside;
        //If inside == true, then insideAbove attribute of the top and left
        //half segments must be set to false, otherwise its value must be true.
        //In other words, the insideAbove atribute value is the opposite of the
        //parameter inside's value.
      else
        if ( (edge == WRIGHT) || (edge == WBOTTOM))
          attr.insideAbove = inside;
        //If inside == true, then insideAbove attribute of the right and bottom
        //half segments must be set to true, otherwise its value must be false.
        //In other words, the insideAbove atribute value is the same of the
        //parameter inside's value.
      chs->SetAttr(attr);
      cr+=(*chs);
      chs->SetLDP(false);
      cr+=(*chs);
      delete chs;
    }
    begin++;
    //The variable ~begin~ must be incremented until exists points with the same coordinates
    //and directions as dp
    while (begin<=end)
    {
      dpAux = pointsOnEdge[begin];
      if (!( (dpAux.GetX() == dp.GetX()) && (dpAux.GetY() == dp.GetY()) && (dpAux.direction==dp.direction) ) )
        break;
      begin++;
    }
  }


  dp = pointsOnEdge[end];
  if ( !dp.direction) //dp points to right or up
  {
    bool rejectEndPoint=dp.rejected;
    end--;

    while ( end >= begin )
    {
      dpAux = pointsOnEdge[end];
      if ( !( (dpAux.GetX() == dp.GetX() ) && ( dpAux.GetY() == dp.GetY() ) &&
              (dpAux.direction==dp.direction) ) )
         break;

      //when a rejected point is found the rejectEndPoint does not change anymore.

      end--;
    }

    if (!rejectEndPoint)
    {
      chs = new CHalfSegment(true,true, dp, ePoint);
      attr.partnerno = partnerno;
      if ( (edge == WTOP) || (edge == WLEFT) )
        attr.insideAbove = !inside;
      else
        if ( (edge == WRIGHT) || (edge == WBOTTOM))
          attr.insideAbove = inside;
      partnerno++;
      chs->SetAttr(attr);
      cr+=(*chs);
      chs->SetLDP(false);
      cr+=(*chs);

      delete chs;
    }
  }

  i = begin;
  while (i < end)
  {
    EdgePoint ep1,ep2;
    if ( GetAcceptedPoint(pointsOnEdge,i,end,ep1) )
    {
      i++;
      if (GetAcceptedPoint(pointsOnEdge,i,end, ep2) )
        i++;
      else
        break;
    }
    else
      break;
    if ( ! ( (ep1.GetX() == ep2.GetX()) && (ep1.GetY() == ep2.GetY()) ) )
    {  //discard degenerated edges
      chs = new CHalfSegment(true,true, ep1, ep2);
      attr.partnerno = partnerno;
      partnerno++;
      if ( (edge == WTOP) || (edge == WLEFT) )
        attr.insideAbove = !inside;
      else
        if ( (edge == WRIGHT) || (edge == WBOTTOM))
          attr.insideAbove = inside;
      chs->SetAttr(attr);
      cr+=(*chs);
      chs->SetLDP(false);
      cr+=(*chs);
      delete chs;
    }
  }
}

void CRegion::CreateNewSegmentsWindowVertices(const Rectangle<2> &window,
                                vector<EdgePoint> pointsOnEdge[4],CRegion &cr,
                                int &partnerno,bool inside) const
//The inside attribute indicates if the points on edge will originate
//segments that are inside the window (its values is true), or outside
//the window (its value is false)
{
  Point tlPoint(true,window.MinD(0),window.MaxD(1)),
        trPoint(true,window.MaxD(0),window.MaxD(1)),
        blPoint(true,window.MinD(0),window.MinD(1)),
        brPoint(true,window.MaxD(0),window.MinD(1));
   bool tl=false, tr=false, bl=false, br=false;

  /*
  cout<<endl<<"interno"<<endl;
  cout<<"Left   :"<<window.MinD(0)<<endl;
  cout<<"Top    :"<<window.MaxD(1)<<endl;
  cout<<"Right  :"<<window.MaxD(0)<<endl;
  cout<<"Bottom :"<<window.MinD(1)<<endl;

  cout<<"Points"<<endl;
  cout<<"tlPoint: "<<tlPoint<<endl;
  cout<<"trPoint: "<<trPoint<<endl;
  cout<<"blPoint: "<<blPoint<<endl;
  cout<<"brPoint: "<<brPoint<<endl;

  */

  AttrType attr;

  if ( ( (pointsOnEdge[WTOP].size()==0) || (pointsOnEdge[WLEFT].size()==0) )
     && ( this->contain(tlPoint) ) )
      tl = true;

  if ( ( (pointsOnEdge[WTOP].size()==0) || (pointsOnEdge[WRIGHT].size()==0)  )
       && ( this->contain(trPoint) ) )
      tr = true;

  if ( ( (pointsOnEdge[WBOTTOM].size()==0) || (pointsOnEdge[WLEFT].size()==0)  )
       && ( this->contain(blPoint) ) )
      bl = true;
  if ( ( (pointsOnEdge[WBOTTOM].size()==0) || (pointsOnEdge[WRIGHT].size()==0)  )
         && ( this->contain(brPoint) ) )
      br = true;


  //Create top edge
  if (tl && tr && (pointsOnEdge[WTOP].size()==0))
  {
    CHalfSegment *chs;
    chs = new CHalfSegment(true,true, tlPoint, trPoint);
    //If inside == true, then insideAbove attribute of the top and left
    //half segments must be set to false, otherwise its value must be true.
    //In other words, the insideAbove atribute value is the opposite of the
    //inside function's parameter value.
    attr.insideAbove = !inside;
    attr.partnerno = partnerno;
    partnerno++;

    chs->SetAttr(attr);
    cr+=(*chs);
    chs->SetLDP(false);
    cr+=(*chs);
    delete chs;
  }
  //Create left edge
  if (tl && bl && (pointsOnEdge[WLEFT].size()==0))
  {
    CHalfSegment *chs;
    chs = new CHalfSegment(true,true, tlPoint, blPoint);
    //If inside == true, then insideAbove attribute of the top and left
    //half segments must be set to false, otherwise its value must be true.
    //In other words, the insideAbove atribute value is the opposite of the
    //parameter inside's value.
    attr.insideAbove = !inside;
    attr.partnerno = partnerno;
    partnerno++;

    chs->SetAttr(attr);
    cr+=(*chs);
    chs->SetLDP(false);
    cr+=(*chs);
    delete chs;
  }
  //Create right edge
  if (tr && br && (pointsOnEdge[WRIGHT].size()==0))
  {
    CHalfSegment *chs;
    chs = new CHalfSegment(true,true, trPoint, brPoint);
    //If inside == true, then insideAbove attribute of the right and bottom
    //half segments must be set to true, otherwise its value must be false.
    //In other words, the insideAbove atribute value is the same of the
    //parameter inside's value.
    attr.insideAbove = inside;
    attr.partnerno = partnerno;
    partnerno++;

    chs->SetAttr(attr);
    cr+=(*chs);
    chs->SetLDP(false);
    cr+=(*chs);
    delete chs;
  }
  //Create bottom edge
  if (bl && br && (pointsOnEdge[WBOTTOM].size()==0))
  {
    CHalfSegment *chs;
    chs = new CHalfSegment(true,true, blPoint, brPoint);
    //If inside == true, then insideAbove attribute of the right and bottom
    //half segments must be set to true, otherwise its value must be false.
    //In other words, the insideAbove atribute value is the same of the
    //parameter inside's value.
    attr.insideAbove = inside;
    attr.partnerno = partnerno;
    partnerno++;

    chs->SetAttr(attr);
    cr+=(*chs);
    chs->SetLDP(false);
    cr+=(*chs);
    delete chs;
  }
}

bool CRegion::ClippedHSOnEdge(const Rectangle<2> &window,const CHalfSegment &chs,
                             bool clippingIn,vector<EdgePoint> pointsOnEdge[4])
{
//This function returns true if the segment lies on one of the window's edge.
// The clipped half segments that lie on the edges must be rejected according to
// the kind of clipping (returning the portion of the region that is inside the
// region or the portion that is outside).

  EdgePoint ep1,ep2;
  AttrType attr=chs.GetAttr();
  bool reject = false,
       result = false; //Returns true if the clipped hs was treated as a segment on edge
  if ( chs.GetLP().GetY() == chs.GetRP().GetY() ) //horizontal edge
  {
    if (( chs.GetLP().GetY() == window.MaxD(1) ) ) //top edge
    {
  // If the half segment lies on the upper edge and the insideAbove attribute's value
  // is true then the region's area is outside the window, and the half segment mustn't
  // be included in the clipped region (Reject). However, its end points maybe will have to be
  // connected to the vertices of the window. It happens only when the vertice of the
  // window is inside the region and the end point is the first point on the window's
  // edge (for the upper-left vertice) or the last point on the window's vertice (for
  // the upper right edge).
      if ( clippingIn && attr.insideAbove )
        reject = true;
      else
        if ( !clippingIn && !attr.insideAbove )
          reject = true;
      ep1.Set(chs.GetLP(),false,reject); //--> right
      ep2.Set(chs.GetRP(),true,reject);  //<-- left
      pointsOnEdge[WTOP].push_back(ep1);
      pointsOnEdge[WTOP].push_back(ep2);
      result = true;
    }
    else //bottom edge
      if (( chs.GetLP().GetY() == window.MinD(1) ) )
      {
        if ( clippingIn && !attr.insideAbove )
           reject = true;
        else
          if ( !clippingIn && attr.insideAbove )
            reject = true;
        ep1.Set(chs.GetLP(),false,reject); //--> right
        ep2.Set(chs.GetRP(),true,reject);  //<-- left
        pointsOnEdge[WBOTTOM].push_back(ep1);
        pointsOnEdge[WBOTTOM].push_back(ep2);
        result = true;
      }
  }
  else //Vertical edges
    if ( chs.GetLP().GetX() == chs.GetRP().GetX() )
    {
      if ( chs.GetLP().GetX() == window.MinD(0) ) //Left edge
      {
        if ( clippingIn && attr.insideAbove )
          reject = true;
        else
          if (!clippingIn && !attr.insideAbove )
            reject = true;
        ep1.Set(chs.GetLP(),false,reject); //^ up
        ep2.Set(chs.GetRP(),true,reject);  //v dowb
        pointsOnEdge[WLEFT].push_back(ep1);
        pointsOnEdge[WLEFT].push_back(ep2);
        result = true;
      }
      else
        if ( chs.GetLP().GetX() == window.MaxD(0) ) //Right edge
        {
          if ( clippingIn && !attr.insideAbove )
            reject = true;
          else
            if ( !clippingIn && attr.insideAbove )
              reject = true;
          ep1.Set(chs.GetLP(),false,reject); //^ up
          ep2.Set(chs.GetRP(),true,reject);  //v dowb
          pointsOnEdge[WRIGHT].push_back(ep1);
          pointsOnEdge[WRIGHT].push_back(ep2);
          result = true;
        }
    }
  return result;
}

bool CRegion::GetCycleDirection(const Point &pA, const Point &pP, const Point &pB)
{
  double m_p_a,m_p_b;
  if (pA.GetX() == pP.GetX())//A --> P is a vertical segment
    if (pA.GetY() > pP.GetY() ) //A --> P directed downwards (case 1)
      return false; //Counterclockwise
    else //upwards (case 2)
      return true; // Clockwise
  if (pB.GetX() == pP.GetX()) //P --> B is a vertical segment
    if ( pP.GetY() > pB.GetY()) //downwords (case 3)
      return false; //Conterclockwise
    else //upwards
      return true; //Clockwise

  //compute the slopes of P-->A and P-->B
  m_p_a = ( pA.GetY() - pP.GetY() ) / ( pA.GetX() - pP.GetX() );
  m_p_b = ( pB.GetY() - pP.GetY() ) / ( pB.GetX() - pP.GetX() );
  if (m_p_a > m_p_b) //case 5
    return false;//counterclockwise
  else  //case 6
    return true; //clockwise
}

bool CRegion::GetCycleDirection() const
{
/*
Preconditions:
* The region must represent just one cycle!!!!
* It is need that the partnerno stores the order that the half segments were typed, and
the half segments must be sorted in the half segment order. In other words if
chs1.attr.partnerno is less than chs2.attr.partnerno then chs1 was typed first than chs2.

This function has the purpose of choosing the A, P, and B points in order to call the
function that really computes the cycle direction.
As the point P is leftmost point then it is the left point of chs1 or the left point
of chs2 because in the half segment order these two points are equal.
Now the problem is to decide which of the right points are A and B. At the first sight
we could say that the point A is the right point of the half segment with lowest
partner number. However it is not true ever because the APB connected points may be go over the
bound of the pointlist. This will be the case if the cycle is in the form P,B,..,A
and B,...,A,P. Nevertheless the segments are ordered in the half segment order, and when the
last half segment is been considered for choosing the APB connected points, the point A will be
always the right point of the last segment.

*/
  Point pA, pP, pB;
  const CHalfSegment *chs1, *chs2;
  this->Get(0,chs1);
  this->Get(1,chs2);
  assert( chs1->GetLP()==chs2->GetLP() );
  pP = chs1->GetLP();
  //If we have the last half segment connected to the first half segment, the difference
  //between their partner numbers is more than one.
  if (abs(chs1->attr.partnerno - chs2->attr.partnerno)>1)
  {
    if (chs1->attr.partnerno > chs2->attr.partnerno)
    {
      pA = chs1->GetRP();
      pB = chs2->GetRP();
    }
    else
    {
      pA = chs2->GetRP();
      pB = chs1->GetRP();
    }
  }
  else
    if (chs1->attr.partnerno < chs2->attr.partnerno)
    {
      pA = chs1->GetRP();
      pB = chs2->GetRP();
    }
    else
    {
      pA = chs2->GetRP();
      pB = chs1->GetRP();
    }
  return GetCycleDirection(pA,pP,pB);
}

//cycleDirection: true (cycle is clockwise) / false (cycle is counterclockwise)
  //It is need that the attribute insideAbove of the half segments represents
  //the order that  their points were typed: true (left point, right point) /
  //false (right point, left point).




void CRegion::GetClippedHSIn(const Rectangle<2> &window,CRegion &clippedRegion,
                             vector<EdgePoint> pointsOnEdge[4],int &partnerno) const
{
  const CHalfSegment *chs;
  CHalfSegment chsInside;
  bool inside, isIntersectionPoint;

  SelectFirst();
  for(int i=0; i < Size(); i++)
  {
    GetHs( chs );
    if (chs->GetLDP())
    {
      Point intersectionPoint;
      chs->WindowClippingIn(window, chsInside, inside, isIntersectionPoint,intersectionPoint);
      if (isIntersectionPoint)
         AddPointToEdgeArray(intersectionPoint,*chs,window, pointsOnEdge);
      else
        if ( inside )
        {
          bool hsOnEdge = ClippedHSOnEdge(window, chsInside, true, pointsOnEdge);
          if (!hsOnEdge)
          {
            //Add the clipped segment to the new region if it was not rejected
            chsInside.attr.partnerno=partnerno;
            partnerno++;
            chsInside.SetAttr(chsInside.attr);
            clippedRegion += chsInside;
            chsInside.SetLDP(false);
            clippedRegion += chsInside;

            //Add the points to the array of the points that lie on some of the window's edges
            Point lp=chsInside.GetLP(),rp = chsInside.GetRP();

            //If the point lies on one edge it must be added to the corresponding vector.
            AddPointToEdgeArray(lp,*chs,window, pointsOnEdge);
            AddPointToEdgeArray(rp, *chs,window, pointsOnEdge);
          }
        }
    }
    SelectNext();
  }
}
void CRegion::AddClippedHS(const Point &pl,const Point &pr,AttrType &attr,int &partnerno) 
{
  CHalfSegment chs(true,true,pl,pr);
  attr.partnerno = partnerno;
  partnerno++;
  chs.SetAttr(attr);
  (*this)+=chs;
  chs.SetLDP(false);
  (*this)+=chs;
}
void CRegion::GetClippedHSOut(const Rectangle<2> &window,CRegion &clippedRegion,
                             vector<EdgePoint> pointsOnEdge[4],int &partnerno) const
{
  for (int i=0; i < Size();i++)
  {
    const CHalfSegment *chs;
    CHalfSegment chsInside;
    bool inside=false,isIntersectionPoint=false;
    Get(i,chs);

    if (chs->GetLDP())
    {
      Point intersectionPoint;
      chs->WindowClippingIn(window,chsInside, inside, isIntersectionPoint,intersectionPoint);
      if (inside)
      {
        bool hsOnEdge=false;
        if (isIntersectionPoint)
        {
          CHalfSegment aux( *chs );
          if (chs->GetLP()!=intersectionPoint)
            clippedRegion.AddClippedHS(aux.GetLP(),intersectionPoint,aux.attr,partnerno) ;
          if (chs->GetRP()!=intersectionPoint)
            clippedRegion.AddClippedHS(intersectionPoint,aux.GetRP(),aux.attr,partnerno);
          AddPointToEdgeArray(intersectionPoint,aux,window, pointsOnEdge);
        }
        else
        {
          hsOnEdge = ClippedHSOnEdge(window, chsInside, false, pointsOnEdge);
          if (!hsOnEdge)
          {
            CHalfSegment aux( *chs );
            if (chs->GetLP()!=chsInside.GetLP())
             //Add the part of the half segment composed by the left point of chs and
             // the left point of chsInside.
              clippedRegion.AddClippedHS(aux.GetLP(),chsInside.GetLP(),aux.attr,partnerno) ;
            AddPointToEdgeArray(chsInside.GetLP(),aux,window, pointsOnEdge);
            if (chs->GetRP()!=chsInside.GetRP())
             //Add the part of the half segment composed by the right point of chs and
             // the right point of chsInside.
              clippedRegion.AddClippedHS(chsInside.GetRP(),aux.GetRP(),aux.attr,partnerno);

            AddPointToEdgeArray(chsInside.GetRP(),aux,window, pointsOnEdge);
          }
        }
      }
      else
      {
        CHalfSegment aux( *chs );
        clippedRegion.AddClippedHS(aux.GetLP(),aux.GetRP(),aux.attr,partnerno);
      }
    }
    SelectNext();
  }
}

void CRegion::GetClippedHS(const Rectangle<2> &window,CRegion &clippedRegion,bool inside) const
{
  vector<EdgePoint> pointsOnEdge[4];//upper edge, right edge, bottom, left
  int partnerno=0;

  clippedRegion.StartBulkLoad();

  if (inside)
    GetClippedHSIn(window,clippedRegion,pointsOnEdge,partnerno);
  else
    GetClippedHSOut(window,clippedRegion,pointsOnEdge,partnerno);


  Point bPoint,ePoint;
  bPoint.Set(window.MinD(0),window.MaxD(1)); //left-top
  ePoint.Set(window.MaxD(0),window.MaxD(1)); //right-top
  CreateNewSegments(pointsOnEdge[WTOP],clippedRegion,bPoint,ePoint, WTOP,partnerno,inside);
  bPoint.Set(window.MinD(0),window.MinD(1)); //left-bottom
  ePoint.Set(window.MaxD(0),window.MinD(1)); //right-bottom
  CreateNewSegments(pointsOnEdge[WBOTTOM],clippedRegion,bPoint,ePoint, WBOTTOM,partnerno,inside);
  bPoint.Set(window.MinD(0),window.MinD(1)); //left-bottom
  ePoint.Set(window.MinD(0),window.MaxD(1)); //left-top
  CreateNewSegments(pointsOnEdge[WLEFT],clippedRegion,bPoint,ePoint, WLEFT,partnerno,inside);
  bPoint.Set(window.MaxD(0),window.MinD(1)); //right-bottom
  ePoint.Set(window.MaxD(0),window.MaxD(1)); //right-top
  CreateNewSegments(pointsOnEdge[WRIGHT],clippedRegion,bPoint,ePoint, WRIGHT,partnerno,inside);

  CreateNewSegmentsWindowVertices(window, pointsOnEdge,clippedRegion, partnerno,inside);


  clippedRegion.EndBulkLoad();
  clippedRegion.SetPartnerNo();
}


bool CRegion::IsCriticalPoint(const Point &adjacentPoint,const int &chsPosition) const
{
  int adjPosition=chsPosition,adjacencyNo=0,step = 1;
  do
  {
    const CHalfSegment *adjCHS;
    adjPosition+=step;
    if ( adjPosition<0 || adjPosition>=this->Size())
      break;
    Get(adjPosition,adjCHS);
    if (!adjCHS->GetLDP())
      continue;
    AttrType attr = adjCHS->GetAttr();
    //When looking for critical points, the partner of the adjacent half segment found
    //cannot be consired.
    if (attr.partnerno == chsPosition)
      continue;
    if ( ( adjacentPoint==adjCHS->GetLP() ) ||
         ( adjacentPoint==adjCHS->GetRP() ) )
      adjacencyNo++;
    else
    {
      if (step==-1)
        return false;
      step=-1;
      adjPosition=chsPosition;
    }
  }
  while (adjacencyNo<2);

  return (adjacencyNo>1);
}

bool CRegion::GetAdjacentHS(const CHalfSegment &chs, const int &chsPosition,
                            int &position, const int &partnerno,
                            const int &partnernoP, CHalfSegment const*& adjacentCHS,
                            const Point &adjacentPoint, Point &newAdjacentPoint,
                            bool *cycle, int step) const
{
  bool adjacencyFound=false;
  do
  {
    position+=step;
    if ( position<0 || position>=this->Size())
      break;

    Get(position,adjacentCHS);
    if (partnernoP == position)
      continue;
    if ( adjacentPoint==adjacentCHS->GetLP() )
    {
      if (!cycle[position])
      {
        newAdjacentPoint = adjacentCHS->GetRP();
        adjacencyFound = true;
      }
    }
    else
      if  ( adjacentPoint==adjacentCHS->GetRP() )
      {
         if (!cycle[position])
        {
          newAdjacentPoint = adjacentCHS->GetLP();
          adjacencyFound = true;
        }
      }
      else
        break;
  }
  while (!adjacencyFound);
  return adjacencyFound;
}
/*
The parameter ~hasCriticalPoint~ indicates that the cycle that
is been computed has a critical point.

*/

void CRegion::ComputeCycle(CHalfSegment &chs, int faceno,
                  int cycleno,int &edgeno, bool *cycle)
{



  Point nextPoint=chs.GetLP(),lastPoint=chs.GetRP(),
        previousPoint, *currentCriticalPoint=NULL;
  AttrType attr, attrP;
  const CHalfSegment *chsP;
  vector<SCycle> sCycleVector;
  SCycle *s=NULL;

  do
  {
     if (s==NULL)
     {


       //Update attributes
       attr = chs.GetAttr();

       Get(attr.partnerno,chsP);
       attrP = chsP->GetAttr();

       attr.faceno=faceno;
       attr.cycleno=cycleno;
       attr.edgeno=edgeno;

       UpdateAttr(attrP.partnerno,attr);

       attrP.faceno=faceno;
       attrP.cycleno=cycleno;
       attrP.edgeno=edgeno;
       UpdateAttr(attr.partnerno,attrP);

       edgeno++;

       cycle[attr.partnerno]=true;
       cycle[attrP.partnerno]=true;

       if (this->IsCriticalPoint(nextPoint,attrP.partnerno))
         currentCriticalPoint=new Point(nextPoint);

       s = new SCycle(chs,attr.partnerno,*chsP,attrP.partnerno,currentCriticalPoint,nextPoint);


     }
     const CHalfSegment *adjacentCHS;
     Point adjacentPoint;
     bool adjacentPointFound=false;
     previousPoint = nextPoint;
     if (s->goToCHS1Right)
     {
       s->goToCHS1Right=GetAdjacentHS(s->chs1, s->chs2Partnerno, s->chs1PosRight,
                                      s->chs1Partnerno,s->chs2Partnerno,adjacentCHS,
                                      previousPoint, nextPoint, cycle, 1);
       adjacentPointFound=s->goToCHS1Right;
     }
     if ( !adjacentPointFound && s->goToCHS1Left )
     {
       s->goToCHS1Left=GetAdjacentHS(s->chs1, s->chs2Partnerno, s->chs1PosLeft,
                                       s->chs1Partnerno,s->chs2Partnerno,adjacentCHS,
                                       previousPoint, nextPoint, cycle, -1);
       adjacentPointFound=s->goToCHS1Left;
     }
     if (!adjacentPointFound && s->goToCHS2Right)
     {
       s->goToCHS2Right=GetAdjacentHS(s->chs2, s->chs1Partnerno, s->chs2PosRight,
                                          s->chs2Partnerno,s->chs1Partnerno,adjacentCHS,
                                          previousPoint, nextPoint, cycle, 1);
       adjacentPointFound=s->goToCHS2Right;
     }
     if (!adjacentPointFound && s->goToCHS2Left)
     {
       s->goToCHS2Left=GetAdjacentHS(s->chs2, s->chs1Partnerno, s->chs2PosLeft,
                                           s->chs2Partnerno,s->chs1Partnerno,adjacentCHS,
                                           previousPoint, nextPoint, cycle, -1);
       adjacentPointFound = s->goToCHS2Left;
     }
     assert(adjacentPointFound);
     /*
     cout<<endl<<"======>"<<endl<<chs;
     cout<<endl<<"PreviousPoint: "<<previousPoint;
     cout<<endl<<"NextPoint: "<<nextPoint;
     cout<<endl<<"chs:         ("<<chs.GetLP()<<" - "<<chs.GetRP()<<")";
     cout<<endl<<"adjacentCHS: ("<<adjacentCHS.GetLP()<<" - "<<adjacentCHS.GetRP()<<")";
     cout<<endl<<"NextPoint: "<<nextPoint<<endl;

     */
     sCycleVector.push_back(*s);
     if ( (currentCriticalPoint!=NULL) && (*currentCriticalPoint==nextPoint) )
     {
       //The critical point defines a cycle, so it is need to remove the segments
       //from the vector, and set the segment as not visited in the cycle array.
       //FirsAux is the first half segment with the critical point equals to
       //criticalPoint.
       SCycle sAux,firstSCycle;

       do
       {
          sAux=sCycleVector.back();
          sCycleVector.pop_back();
          firstSCycle=sCycleVector.back();
          if (firstSCycle.criticalPoint==NULL)
            break;
          if (*firstSCycle.criticalPoint!=*currentCriticalPoint)
            break;
          cycle[sAux.chs1Partnerno]=false;
          cycle[sAux.chs2Partnerno]=false;
          edgeno--;
       }while(sCycleVector.size()>1);
       delete s; //when s is deleted, the critical point is also deleted.
       if (sCycleVector.size()==1)
       {
         sCycleVector.pop_back();
         s = new SCycle(firstSCycle);
       }
       else
         s= new SCycle(sAux);
       chs = s->chs1;
       currentCriticalPoint=s->criticalPoint;
       nextPoint=s->nextPoint;
       continue;
     }

     if ( nextPoint==lastPoint )
     {
       //Update attributes
       attr = adjacentCHS->GetAttr();

       Get(attr.partnerno,chsP);
       attrP = chsP->GetAttr();

       attr.faceno=faceno;
       attr.cycleno=cycleno;
       attr.edgeno=edgeno;

       UpdateAttr(attrP.partnerno,attr);

       attrP.faceno=faceno;
       attrP.cycleno=cycleno;
       attrP.edgeno=edgeno;
       UpdateAttr(attr.partnerno,attrP);

       edgeno++;

       cycle[attr.partnerno]=true;
       cycle[attrP.partnerno]=true;

       break;
     }
     chs = adjacentCHS;
     delete s;
     s=NULL;
  }
  while(1);

}



//This function returns the value of the atribute inside above of
//the first half segment under the half segment chsS.
int CRegion::GetNewFaceNo(CHalfSegment &chsS, bool *cycle)
{
  int coverno=0;
  int startpos=0;
  double y0;
  AttrType attr;
  vector<CHalfSegment> v;

  //1. find the right place by binary search
  startpos = Position( chsS );

  int chsVisiteds=0;

  //2. deal with equal-x chs's
  //To verify if it is need to deal with this

  attr = chsS.GetAttr();
  coverno = attr.coverageno;

  //search the region value for coverageno steps
  int touchedNo=0;
  const CHalfSegment *chs;
  Point p=chsS.GetLP();

  int i=startpos;
  while (( i>=0)&&(touchedNo<coverno))
  {
    this->Get(i, chs);
    chsVisiteds++;

    if ( (cycle[i]) && (chs->GetLDP()) &&
         ( (chs->GetLP().GetX() <= p.GetX()) &&
         (p.GetX() <= chs->GetRP().GetX()) ))
    {
      touchedNo++;
      if (!chs->rayAbove(p, y0))
        v.push_back(*chs);
    }
    i--;  //the iterator
  }
  if (v.size()==0)
    return -1; //the new face number will be the last face number +1
  else
  {
    sort(v.begin(),v.end());
    //The first half segment is the next half segment above chsS
    CHalfSegment chs = v[v.size()-1];
    attr = chs.GetAttr();
    if (attr.insideAbove)
      return attr.faceno; //the new cycle is a cycle of the face ~attr.faceno~
    else
      return -1; //new face
  }
}

void CRegion::ComputeRegion()
{
  //array that stores in position i the last cycle number of the face i
  vector<int> face;
  //array that stores in the position ~i~ if the half segment hi had already the face
  //number, the cycle number and the edge number attributes set properly, in other words,
  //it means that hi is already part of a cycle
  bool *cycle;
  int lastfaceno=0,
      faceno=0,
      cycleno = 0,
      edgeno = 0;
  bool isFirstCHS=true;
  const CHalfSegment *chs;

  if (Size()==0)
    return;
  face.push_back(0); //Insert in the vector the first cycle of the first face
  cycle = new bool[Size()];
  memset( cycle, false, Size() );
  for ( int i=0; i<Size(); i++)
  {
    Get(i,chs);
    CHalfSegment aux(*chs);
    if ( aux.GetLDP() && !cycle[i])
    {
      if(!isFirstCHS)
      {
        int facenoAux = GetNewFaceNo(aux,cycle);
        if (facenoAux==-1)
        {/*The lchs half segment will start a new face*/
          lastfaceno++;
          faceno = lastfaceno;
          face.push_back(0); /*to store the first cycle number of the face lastFace*/
          cycleno = 0;
          edgeno = 0;
        }
        else
        { /*The half segment ~chs~ belongs to an existing face*/
          faceno = facenoAux;
          face[faceno]++;
          cycleno = face[faceno];
          edgeno = 0;
        }
      }
      else
        isFirstCHS = false;
      ComputeCycle(aux, faceno,cycleno, edgeno, cycle);
    }
  }
  delete cycle;

}

void CRegion::WindowClippingIn(const Rectangle<2> &window,CRegion &clippedRegion) const 
{
  //cout<<endl<<"Original: "<<*this<<endl;
  if (!this->bbox.Intersects(window))
    return;
  //If the bounding box of the region is inside the window, then the clippedRegion
  //is equal to the region been clipped.

  if (window.Contains(this->bbox))
    clippedRegion = *this;
  else
  {
    //cout<<endl<<this->bbox<<endl;
    this->GetClippedHS(window,clippedRegion,true);
    //cout<<endl<<"Clipped HS:"<<endl<<clippedRegion;
    clippedRegion.ComputeRegion();
  }
  //cout<<endl<<"Clipped;"<<clippedRegion;
}
void CRegion::WindowClippingOut(const Rectangle<2> &window,CRegion &clippedRegion) const
{
  //If the bounding box of the region is inside the window, then the clipped region is empty
  //cout<<"region: "<<*this<<endl;
  if (window.Contains(this->bbox))
    return;
  if (!window.Intersects(this->bbox))
    clippedRegion = *this;
  else
  {
    this->GetClippedHS(window,clippedRegion,false);
 //   cout<<endl<<"Clipped HS:"<<endl<<clippedRegion;
    clippedRegion.ComputeRegion();
  }
 // cout<<endl<<"clippedRegion: "<<clippedRegion;
}

size_t CRegion::HashValue() const
{
    //cout<<"cregion hashvalue1*******"<<endl;
    if(IsEmpty())  return (0);
    unsigned long h=0;

    const CHalfSegment *chs;
    Coord x1, y1;
    Coord x2, y2;

    for( int i = 0; ((i < Size())&&(i<5)); i++ )
    {
  Get( i, chs );
  x1=chs->GetLP().GetX();
  y1=chs->GetLP().GetY();

  x2=chs->GetRP().GetX();
  y2=chs->GetRP().GetY();
#ifdef RATIONAL_COORDINATES
  h=h+(unsigned long)
   ((5*(x1.IsInteger()? x1.IntValue():x1.Value())
     + (y1.IsInteger()? y1.IntValue():y1.Value()))+
    (5*(x2.IsInteger()? x2.IntValue():x2.Value())
     + (y2.IsInteger()? y2.IntValue():y2.Value())));
#else
  h=h+(unsigned long)((5*x1 + y1)+ (5*x2 + y2));
#endif
    }
    return size_t(h);
}

void CRegion::Clear()
{
    region.Clear();
    pos=-1;
    ordered=true;
    bbox.SetDefined(false);
}

void CRegion::CopyFrom(const StandardAttribute* right)
{
    //cout<<"cregion copyfrom1*******"<<endl;
    CRegion * cr = (CRegion*)right;
    ordered = true;
    assert( cr->IsOrdered());
    //I think that here the PArray region should be clear first...DZM
    Clear();
    for( int i = 0; i < cr->Size(); i++ )
    {
  const CHalfSegment *chs;
  cr->Get( i, chs );
  Put( i, *chs );
    }
    bbox=cr->BoundingBox();
    //cout<<*this<<endl<<" .vs. "<<endl<<*cr<<endl;
}

int CRegion::Compare(const Attribute * arg) const
{
  //cout<<"cregion compare1*******"<<endl;
  int res=0;
  CRegion* cr = (CRegion* )(arg);
  if ( !cr ) 
    return -2;

  if (IsEmpty() && (cr->IsEmpty()))  
    res=0;
  else if (IsEmpty())  
    res=-1;
  else  if ((cr->IsEmpty())) 
    res=1;
  else
  {
    if (Size() > cr->Size()) 
      res=1;
    else if (Size() < cr->Size()) 
      res=-1;
    else  //their sizes are equal
    {
      bool bboxCmp = bbox.Compare( &cr->bbox );
      if( bboxCmp == 0 )
      {
        bool decided = false;
        for( int i = 0; ((i < Size())&&(!decided)); i++ )
        {
          const CHalfSegment *chs1, *chs2;
          Get( i, chs1);
          cr->Get( i, chs2 );

          if (*chs1 >*chs2) 
          {
            res=1;
            decided=true;
          }
          else if (*chs1 < *chs2) 
          {
            res=-1;
            decided=true;
          }
        }
        if (!decided) 
          res=0;
      }
      else
        res = bboxCmp;
    }
  }
  return res;
}

ostream& CRegion::Print( ostream &os ) const
{
    os << "<";
    for( int i = 0; i < Size(); i++ )
    {
  const CHalfSegment *chs;
  Get( i, chs );
  os << " " << *chs;
    }
    os << ">";
    return os;
}

bool CRegion::insertOK(const CHalfSegment& chs)
{
    const CHalfSegment *auxchs;
    double dummyy0;

    return true;  //the check is closed temporarily to import data.

    if (chs.IsDefined())
    {
  int prevcycleMeet[50];

  int prevcyclenum=0;
  for( int i = 0; i< 50; i++ )
  {
      prevcycleMeet[i]=0;

  }

  for( int i = 0; i<= region.Size()-1; i++ )
  {
      region.Get( i, auxchs );

      if (auxchs->GetLDP())
      {

    if (chs.IsDefined())
    {
        if (chs.Intersects(*auxchs))
        {
      if ((chs.attr.faceno!=auxchs->attr.faceno)||
          (chs.attr.cycleno!=auxchs->attr.cycleno))
      {
          cout<<"two cycles intersect with the ";
          cout<<"following edges:";
          cout<<*auxchs<<" :: "<<chs<<endl;
          return false;
      }
      else
      {
          if ((auxchs->GetLP()!=chs.GetLP()) &&
              (auxchs->GetLP()!=chs.GetRP()) &&
              (auxchs->GetRP()!=chs.GetLP()) &&
              (auxchs->GetRP()!=chs.GetRP()))
          {
        cout<<"two edges: " <<*auxchs<<" :: "<< chs
        <<" of the same cycle intersect in middle!"
          <<endl;
        return false;
          }
      }
        }
        else
        {
      if ((chs.attr.cycleno>0) &&
          (auxchs->attr.faceno==chs.attr.faceno) &&
          (auxchs->attr.cycleno!=chs.attr.cycleno))
      {

          if (auxchs->rayAbove(chs.GetLP(), dummyy0))
          {
        prevcycleMeet[auxchs->attr.cycleno]++;
        if (prevcyclenum < auxchs->attr.cycleno)
            prevcyclenum=auxchs->attr.cycleno;
          }
      }
        }
    }
      }
  }

  if ((chs.IsDefined()) && (chs.attr.cycleno>0))
  {
      if  (prevcycleMeet[0] % 2 ==0)
      {
    cout<<"hole(s) is not inside the outer cycle! "<<endl;
    return false;
      }
      for (int i=1; i<=prevcyclenum; i++)
      {
    if (prevcycleMeet[i] % 2 !=0)
    {
        cout<<"one hole is inside another! "<<endl;
        return false;
    }
      }
  }
    }
/*
Now we know that the new half segment is not inside any other previous holes of the
same face. However, whether this new hole contains any previous hole of the same
face is not clear. In the following we do this kind of check.

*/

    if ((!chs.IsDefined())||((chs.attr.faceno>0) || (chs.attr.cycleno>2)))
    {
  const CHalfSegment *chsHoleNEnd, *chsHoleNStart;

  if (region.Size() ==0) return true;

  int holeNEnd=region.Size()-1;
  region.Get(holeNEnd, chsHoleNEnd );

  if  ((chsHoleNEnd->attr.cycleno>1) &&
      ((!chs.IsDefined())||
      (chs.attr.faceno!=chsHoleNEnd->attr.faceno)||
      (chs.attr.cycleno!=chsHoleNEnd->attr.cycleno)))
  {

      if (chsHoleNEnd->attr.cycleno>1)
      {

    int holeNStart=holeNEnd - 1;
    region.Get(holeNStart, chsHoleNStart );

    while ((chsHoleNStart->attr.faceno==chsHoleNEnd->attr.faceno) &&
           (chsHoleNStart->attr.cycleno==chsHoleNEnd->attr.cycleno)&&
           (holeNStart>0))
    {
        holeNStart--;
        region.Get(holeNStart, chsHoleNStart );
    }
    holeNStart++;

    int prevHolePnt=holeNStart-1;
    const CHalfSegment *chsPrevHole, *chsLastHole;

    bool stillPrevHole = true;
    while ((stillPrevHole) && (prevHolePnt>=0))
    {
        region.Get(prevHolePnt, chsPrevHole );
        prevHolePnt--;

        if ((chsPrevHole->attr.faceno!= chsHoleNEnd->attr.faceno)||
      (chsPrevHole->attr.cycleno<=0))
        {
      stillPrevHole=false;
        }

        if (chsPrevHole->GetLDP())
        {
      int holeNMeent=0;
      for (int i=holeNStart; i<=holeNEnd; i++)
      {
          region.Get(i, chsLastHole );
          if ((chsLastHole->GetLDP())&&
              (chsLastHole->rayAbove
              (chsPrevHole->GetLP(), dummyy0)))
        holeNMeent++;
      }
      if  (holeNMeent % 2 !=0)
      {
          cout<<"one hole is inside another!!! "<<endl;
          return false;
      }
        }
    }
      }
  }
    }
    return true;
}

/*
This function check whether a region value is valid after thr insertion of a new half segment.
Whenever a half segment is about to be inserted, the state of the region is checked.
A valid region must satisfy the following conditions:

1)  any two cycles of the same region must be disconnect, which means that no edges
of different cycles can intersect each other;

2) edges of the same cycle can only intersect with their endpoints, but no their middle points;

3)  For a certain face, the holes must be inside the outer cycle;

4)  For a certain face, any two holes can not contain each other;

5)  Faces must have the outer cycle, but they can have no holes;

6)  for a certain cycle, any two vertex can not be the same;

7)  any cycle must be made up of at least 3 edges;

8)  It is allowed that one face is inside another provided that their edges do not intersect.


8.2 List Representation

The list representation of a region is

----  (face1  face2  face3 ... )
                 where facei=(outercycle, holecycle1, holecycle2....)

  cyclei= (vertex1, vertex2,  .....)
                where each vertex is a point.
----

8.3 ~Out~-function

*/

ListExpr
SaveToListRegion( ListExpr typeInfo, Word value )
{
  //cout<<"SaveToListRegion########"<<endl;
  // Put the Class Object to Direct NL. Analogious to: OUT_Region
  CRegion* cr = (CRegion*)(value.addr);
  if( cr->IsEmpty() )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    const CHalfSegment *chs;

    ListExpr regionNL = nl->TheEmptyList();
    ListExpr regionNLLast = regionNL;


    Point LOutputP, ROutputP;   //the Two Endpoints
    bool LDP,insideAbove;   //Is Left Dominating-Point,insideabove
    int faceno, cycleno, edgeno, coverageno, partnerno; //face, cycle, edge, coverage numbers

    ListExpr LPointNL, RPointNL;

    //int currCoverageNo=0;

    for( int i = 0; i < cr->Size(); i++ )
    {
      cr->Get( i, chs );

      LDP = chs->GetLDP();           // the flag
      LOutputP = chs->GetLP();     //the endpoints
      ROutputP = chs->GetRP();
      faceno = chs->attr.faceno;
      cycleno = chs->attr.cycleno;
      coverageno = chs->attr.coverageno;
      edgeno = chs->attr.edgeno;
      insideAbove = chs->attr.insideAbove;
      partnerno = chs->attr.partnerno;



      //if  (chs.GetLDP())
      //  currCoverageNo++;
      //else  currCoverageNo--;

      //chs.attr.coverageno=currCoverageNo;
      //cout<<chs<<endl;


      //cout<<"::"<<cvn<<"::";

      LPointNL=OutPoint( nl->TheEmptyList(), SetWord( &LOutputP));
      RPointNL=OutPoint( nl->TheEmptyList(), SetWord( &ROutputP));

      ListExpr chsNL = nl->OneElemList( nl-> BoolAtom(LDP) ),
               last = chsNL;

      last = nl->Append( last, nl->TwoElemList(LPointNL, RPointNL));
      last = nl->Append( last, nl->IntAtom( faceno ) );
      last = nl->Append( last, nl->IntAtom( cycleno ));
      last = nl->Append( last, nl->IntAtom( edgeno ));
      last = nl->Append( last, nl->IntAtom( coverageno ));
      last = nl->Append( last, nl->BoolAtom( insideAbove ));
      last = nl->Append( last, nl->IntAtom( partnerno ));



      if (regionNL==nl->TheEmptyList())
      {
        regionNL=nl->OneElemList( chsNL);
        regionNLLast = regionNL;
      }
      else
      {
        regionNLLast = nl->Append( regionNLLast, chsNL);
      }
    }

   // cout<<"region direct_NL is: "<<endl;
  //  nl->WriteListExpr( regionNL, cout );
    return regionNL;
  }
}

ListExpr
OutRegion( ListExpr typeInfo, Word value )
{
    //cout<<"OutRegion#############"<<endl;

    CRegion* cr = (CRegion*)(value.addr);
    //cout<<endl<<"Original: "<<*cr;
    if( cr->IsEmpty() )
    {
  return (nl->TheEmptyList());
    }
    else
    {
  CRegion *RCopy=new CRegion(*cr, true); // in memory
  //cout<<endl<<"Copy: "<<*RCopy;

  RCopy->logicsort();

  const CHalfSegment *chs, *chsnext;

  ListExpr regionNL = nl->TheEmptyList();
  ListExpr regionNLLast = regionNL;

  ListExpr faceNL = nl->TheEmptyList();
  ListExpr faceNLLast = faceNL;

  ListExpr cycleNL = nl->TheEmptyList();
  ListExpr cycleNLLast = cycleNL;

  ListExpr pointNL;

  int currFace, currCycle;
  Point outputP, leftoverP;

  for( int i = 0; i < RCopy->Size(); i++ )
  {
      RCopy->Get( i, chs );
      if (i==0)
      {
    currFace=chs->attr.faceno;
    currCycle=chs->attr.cycleno;
    RCopy->Get( i+1, chsnext );

    if ((chs->GetLP() == chsnext->GetLP()) ||
        ((chs->GetLP() == chsnext->GetRP())))
    {
        outputP=chs->GetRP();
        leftoverP=chs->GetLP();
    }
    else if ((chs->GetRP() == chsnext->GetLP()) ||
                ((chs->GetRP() == chsnext->GetRP())))
    {
        outputP=chs->GetLP();
        leftoverP=chs->GetRP();
    }
    else
    {
        cout<<"wrong data format!"<<endl;
        return nl->TheEmptyList();
    }

    pointNL=OutPoint( nl->TheEmptyList(), SetWord( &outputP));
    if (cycleNL==nl->TheEmptyList())
    {
        cycleNL=nl->OneElemList( pointNL);
        cycleNLLast = cycleNL;
    }
    else
    {
        cycleNLLast = nl->Append( cycleNLLast, pointNL);
    }
      }
      else
      {
    if (chs->attr.faceno==currFace)
    {
        if (chs->attr.cycleno==currCycle)
        {
      outputP=leftoverP;

      if (chs->GetLP()==leftoverP)  leftoverP=chs->GetRP();
      else if (chs->GetRP()==leftoverP)
      {
          leftoverP=chs->GetLP();
      }
      else
      {
          cout<<"wrong data format!"<<endl;
          return nl->TheEmptyList();
      }

      pointNL=OutPoint( nl->TheEmptyList(),
            SetWord( &outputP));
      if (cycleNL==nl->TheEmptyList())
      {
          cycleNL=nl->OneElemList( pointNL);
          cycleNLLast = cycleNL;
      }
      else
      {
          cycleNLLast = nl->Append( cycleNLLast, pointNL);
      }
        }
        else
        {

      if (faceNL==nl->TheEmptyList())
      {
          faceNL=nl->OneElemList( cycleNL);
          faceNLLast = faceNL;
      }
      else
      {
          faceNLLast = nl->Append( faceNLLast, cycleNL);
      }
      cycleNL = nl->TheEmptyList();
      currCycle=chs->attr.cycleno;


      RCopy->Get( i+1, chsnext );
      if ((chs->GetLP() == chsnext->GetLP()) ||
          ((chs->GetLP() == chsnext->GetRP())))
      {
          outputP=chs->GetRP();
          leftoverP=chs->GetLP();
      }
      else if ((chs->GetRP() == chsnext->GetLP()) ||
                  ((chs->GetRP() == chsnext->GetRP())))
      {
          outputP=chs->GetLP();
          leftoverP=chs->GetRP();
      }
      else
      {
          cout<<"wrong data format!"<<endl;
          return nl->TheEmptyList();
      }

      pointNL=OutPoint( nl->TheEmptyList(),
            SetWord( &outputP));
      if (cycleNL==nl->TheEmptyList())
      {
          cycleNL=nl->OneElemList( pointNL);
          cycleNLLast = cycleNL;
      }
      else
      {
          cycleNLLast = nl->Append( cycleNLLast, pointNL);
      }
        }
    }
    else
    {

        if (faceNL==nl->TheEmptyList())
        {
      faceNL=nl->OneElemList( cycleNL);
      faceNLLast = faceNL;
        }
        else
        {
      faceNLLast = nl->Append( faceNLLast, cycleNL);
        }
        cycleNL = nl->TheEmptyList();


        if (regionNL==nl->TheEmptyList())
        {
      regionNL=nl->OneElemList( faceNL);
      regionNLLast = regionNL;
        }
        else
        {
      regionNLLast = nl->Append( regionNLLast, faceNL);
        }
        faceNL = nl->TheEmptyList();

        currFace=chs->attr.faceno;
        currCycle=chs->attr.cycleno;


        RCopy->Get( i+1, chsnext );
        if ((chs->GetLP() == chsnext->GetLP()) ||
           ((chs->GetLP() == chsnext->GetRP())))
        {
      outputP=chs->GetRP();
      leftoverP=chs->GetLP();
        }
        else if ((chs->GetRP() == chsnext->GetLP()) ||
      ((chs->GetRP() == chsnext->GetRP())))
        {
      outputP=chs->GetLP();
      leftoverP=chs->GetRP();
        }
        else
        {
      cout<<"wrong data format!"<<endl;
      return nl->TheEmptyList();
        }

        pointNL=OutPoint( nl->TheEmptyList(), SetWord( &outputP));
        if (cycleNL==nl->TheEmptyList())
        {
      cycleNL=nl->OneElemList( pointNL);
      cycleNLLast = cycleNL;
        }
        else
        {
      cycleNLLast = nl->Append( cycleNLLast, pointNL);
        }
    }
      }
  }

  if (faceNL==nl->TheEmptyList())
  {
      faceNL=nl->OneElemList( cycleNL);
      faceNLLast = faceNL;
  }
  else
  {
      faceNLLast = nl->Append( faceNLLast, cycleNL);
  }
  cycleNL = nl->TheEmptyList();


  if (regionNL==nl->TheEmptyList())
  {
      regionNL=nl->OneElemList( faceNL);
      regionNLLast = regionNL;
  }
  else
  {
      regionNLLast = nl->Append( regionNLLast, faceNL);
  }
  faceNL = nl->TheEmptyList();
  delete RCopy;

  return regionNL;
    }
}

/*
8.4 ~In~-function

*/

static Word
RestoreFromListRegion( const ListExpr typeInfo,
                       const ListExpr instance,
                       const int errorPos,
                       ListExpr& errorInfo,
                       bool& correct )
{
  //cout<<"RestoreFromListRegion###########"<<endl;
  //Fron NL DIRECTLY to Class Objects. Analogious to IN_Region
  CRegion* cr = new CRegion( 0 );

  cr->StartBulkLoad();

  ListExpr RegionNL = instance;
  ListExpr flagedSeg, CHS_NL;

  while( !nl->IsEmpty( RegionNL ) )
  {
    //1. To Fetch one Halfsegment to CHS_NL=(true ((1 1) (2 2)) 3 4 5 0)
    CHS_NL = nl->First( RegionNL );
    RegionNL = nl->Rest( RegionNL );

    //2. Translate the CHS_NL to real halfsegment format=(true ((1 1) (2 2)))
    flagedSeg = nl->TwoElemList(
                  nl->First(CHS_NL),
                  nl->Second(CHS_NL));

    //3. Create the Halfsegment
    CHalfSegment * chs = (CHalfSegment*)InHalfSegment
               ( nl->TheEmptyList(), flagedSeg,
                 0, errorInfo, correct ).addr;

    assert(correct);

    ListExpr attrNL = nl->Rest(nl->Rest(CHS_NL));

    chs->attr.faceno = nl->IntValue(nl->First(attrNL));  //faceNo;
    attrNL = nl->Rest(attrNL);
    chs->attr.cycleno = nl->IntValue(nl->First(attrNL));  //cycleNo;
    attrNL = nl->Rest(attrNL);
    chs->attr.edgeno = nl->IntValue(nl->First(attrNL));  //edgeNo;
    attrNL = nl->Rest(attrNL);
    chs->attr.coverageno = nl->IntValue(nl->First(attrNL));  //coverageNo;
    attrNL = nl->Rest(attrNL);
    chs->attr.insideAbove = nl->BoolValue(nl->First(attrNL));  //insideAbove;
    attrNL = nl->Rest(attrNL);
    chs->attr.partnerno = nl->IntValue(nl->First(attrNL));  //partnerno;

    //4. append the halfsegment
    (*cr) += (*chs);
    delete chs;
  }

  cr->EndBulkLoad(false);  //We are sure that the region was stored ordered.

  correct = true;
  return SetWord( cr );
}

Word
InRegion( const ListExpr typeInfo, const ListExpr instance, const int errorPos, ListExpr& errorInfo, bool& correct )
{
  CRegion* cr = new CRegion( 0 );

  cr->StartBulkLoad();

  ListExpr RegionNL = instance;
  ListExpr FaceNL, CycleNL;
  int fcno=-1;
  int ccno=-1;
  int edno=-1;
  int partnerno = 0;

  if (!nl->IsAtom(instance))
  {
    while( !nl->IsEmpty( RegionNL ) )
    {
      FaceNL = nl->First( RegionNL );
      RegionNL = nl->Rest( RegionNL);
      bool isCycle = true;

      //A face is composed by 1 cycle, and can have holes.
      //All the holes must be inside the face. (TO BE IMPLEMENTED0)
      //CRegion *faceCycle;

      fcno++;
      ccno=-1;
      edno=-1;

      if (nl->IsAtom( FaceNL ))
      {
        correct=false;
        return SetWord( Address(0) );
      }

      while (!nl->IsEmpty( FaceNL) )
      {
        CycleNL = nl->First( FaceNL );
        FaceNL = nl->Rest( FaceNL );

        ccno++;
        edno=-1;

        if (nl->IsAtom( CycleNL ))
        {
          correct=false;
          return SetWord( Address(0) );
        }

        if (nl->ListLength( CycleNL) <3)
        {
          cout<<"a cycle must have at least 3 edges!"<<endl;
          correct=false;
          return SetWord( Address(0) );
        }
        else
        {
          ListExpr firstPoint = nl->First( CycleNL );
          ListExpr prevPoint = nl->First( CycleNL );
          ListExpr flagedSeg, currPoint;
          CycleNL = nl->Rest( CycleNL );

          //Starting to compute a new cycle

          Points *cyclepoints= new Points( 0 ); // in memory

          Point *currvertex,p1,p2,firstP;

          //This function has the goal to store the half segments of
          //the cycle that is been treated. When the cycle's computation
          //is terminated the region rDir will be used to compute the insideAbove
          //attribute of the half segments of this cycle.
          CRegion *rDir = new CRegion(0);
          rDir->StartBulkLoad();


          currvertex = (Point*) InPoint ( nl->TheEmptyList(),
              firstPoint, 0, errorInfo, correct ).addr;
          if (!correct) return SetWord( Address(0) );
          cyclepoints->StartBulkLoad();
          (*cyclepoints) += (*currvertex);
          p1 = *currvertex;
          firstP = p1;
          cyclepoints->EndBulkLoad();
          delete currvertex;

          while ( !nl->IsEmpty( CycleNL) )
          {
            currPoint = nl->First( CycleNL );
            CycleNL = nl->Rest( CycleNL );

            currvertex = (Point*) InPoint( nl->TheEmptyList(),
                  currPoint, 0, errorInfo, correct ).addr;

            if (!correct) return SetWord( Address(0) );

            if (cyclepoints->Contains(*currvertex))
            {
              cout<<"the same vertex: "<<(*currvertex)
              <<" repeated in the cycle!"<<endl;
              correct=false;
              return SetWord( Address(0) );
            }
            else
            {
              p2 = *currvertex;
              cyclepoints->StartBulkLoad();
              (*cyclepoints) += (*currvertex);
              cyclepoints->EndBulkLoad();
            }
            delete currvertex;

            flagedSeg = nl->TwoElemList
            (nl-> BoolAtom(true),
             nl->TwoElemList(prevPoint, currPoint));
            prevPoint=currPoint;
            edno++;
            //Create left dominating half segment
            CHalfSegment * chs = (CHalfSegment*)InHalfSegment
                      ( nl->TheEmptyList(), flagedSeg,
                       0, errorInfo, correct ).addr;
            chs->attr.faceno=fcno;
            chs->attr.cycleno=ccno;
            chs->attr.edgeno=edno;
            chs->attr.partnerno=partnerno;
            partnerno++;
            chs->attr.insideAbove = (chs->GetLP() == p1); //true (L-->R ),false (R--L)
            p1 = p2;

            if (( correct )&&( cr->insertOK(*chs) ))
            {
              //Add left dominating half segment
              (*cr) += (*chs);
              (*rDir) += (*chs);
              //Add right dominating half segment
              chs->SetLDP(false);
              (*cr) += (*chs);
              delete chs;
            }
            else
            {
              correct=false;
              return SetWord( Address(0) );
            }

          }
          delete cyclepoints;

          edno++;
          flagedSeg= nl->TwoElemList
            (nl-> BoolAtom(true),
             nl->TwoElemList(firstPoint, currPoint));
          CHalfSegment * chs = (CHalfSegment*)InHalfSegment
                  ( nl->TheEmptyList(), flagedSeg,
                    0, errorInfo, correct ).addr;
          chs->attr.faceno=fcno;
          chs->attr.cycleno=ccno;
          chs->attr.edgeno=edno;
          chs->attr.partnerno=partnerno;
          chs->attr.insideAbove = (chs->GetRP() == firstP);
          //true (L-->R ),false (R--L), the order of typing is last point than first point.
          partnerno++;

          //The last half segment of the region
          if (( correct )&&( cr->insertOK(*chs) ))
          {
            (*cr) += (*chs);
            (*rDir) += (*chs); //it is only need the left dominating point half segment
            chs->SetLDP(false);
            (*cr) += (*chs);
            delete chs;
            rDir->EndBulkLoad();
            //To calculate the inside above attribute
            bool direction = rDir->GetCycleDirection();
            //cout<<endl<<"Region: "<<endl<<*cr;
            //cout<<endl<<"Cycle: "<<endl<<*rDir;
            int h = cr->Size() - ( rDir->Size() * 2 );
            while ( h < cr->Size())
            {
              //after each left half segment of the region is its
              //correspondig right half segment
              const CHalfSegment *chsIA;
              bool insideAbove;
              cr->Get(h,chsIA);
              /*
                The test for adjusting the inside above can be described
                as above, but was implemented in a different way that
                produces the same result.
                if ( (direction  && chsIA.attr.insideAbove) ||
                     (!direction && !chsIA.attr.insideAbove) )
                {
                  //clockwise and l-->r or
                  //counterclockwise and r-->l
                  chsIA.attr.insideAbove=false;
                }
                else
                  //clockwise and r-->r or
                  //counterclockwise and l-->r
                  true;

              */
              if (direction == chsIA->attr.insideAbove)
                insideAbove = false;
              else
                insideAbove = true;
              if (!isCycle)
                insideAbove = !insideAbove;
              CHalfSegment aux( *chsIA );
              aux.attr.insideAbove = insideAbove;
              cr->UpdateAttr(h,aux.attr);
              //Get right half segment
              cr->Get(h+1,chsIA);
              aux = *chsIA;
              aux.attr.insideAbove = insideAbove;
              cr->UpdateAttr(h+1,aux.attr);
              h+=2;
            }

            //After the first face's cycle read the faceCycle variable is set. Afterwards
            //it is tested if all the new cycles are inside the faceCycle.
            /*
            if (isCycle)
              faceCycle = new CRegion(rDir,false);
            else
              //To implement the test
            */
            delete rDir;
            //After the end of the first cycle of the face, all the following cycles are
            //holes, then isCycle is set to false.
            isCycle = false;

          }
          else
          {
            correct=false;
            return SetWord( Address(0) );
          }
        }
      }
    }

    cr->EndBulkLoad();
    cr->SetPartnerNo();
    assert( cr->Valid() );

    correct = true;
    return SetWord( cr );
  }
  else
  {
    correct=false;
    return SetWord( Address(0) );
  }
}

/*
8.5 ~Create~-function

*/
Word
CreateRegion( const ListExpr typeInfo )
{
  //cout << "CreateRegion" << endl;

  return (SetWord( new CRegion( 0 ) ));
}

/*
8.6 ~Delete~-function

*/
void
DeleteRegion( const ListExpr typeInfo, Word& w )
{
  //cout << "DeleteRegion" << endl;

  CRegion *cr = (CRegion *)w.addr;
  cr->Destroy();
  delete cr;
  w.addr = 0;
}

/*
8.7 ~Close~-function

*/
void
CloseRegion( const ListExpr typeInfo, Word& w )
{
  //cout << "CloseRegion" << endl;

  delete (CRegion *)w.addr;
  w.addr = 0;
}

/*
8.8 ~Clone~-function

*/
Word
CloneRegion( const ListExpr typeInfo, const Word& w )
{
  //cout << "CloneRegion" << endl;

  CRegion *cr = new CRegion( *((CRegion *)w.addr) );
  return SetWord( cr );
}

/*
8.9 ~SizeOf~-function

*/
int SizeOfRegion()
{
  return sizeof(CRegion);
}

/*
8.11 Function describing the signature of the type constructor

*/
ListExpr
RegionProperty()
{
  ListExpr listreplist = nl->TextAtom();
  nl->AppendText(listreplist,"(<face>*) where face is (<outercycle><holecycle>*); "
  "<outercycle> and <holecycle> are <points>*");
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"(((3 0)(10 1)(3 1))((3.1 0.1)"
           "(3.1 0.9)(6 0.8)))");
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,"all <holecycle> must be completely within "
  "<outercycle>.");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List"),
           nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("region"),
           listreplist,
           examplelist,
           remarkslist)));
}

/*
8.12 Kind checking function

This function checks whether the type constructor is applied correctly. Since
type constructor ~point~ does not have arguments, this is trivial.

*/
bool
CheckRegion( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "region" ));
}

/*
8.13 ~Cast~-function

*/
void* CastRegion(void* addr)
{
  return (new (addr) CRegion);
}

/*
8.14 Creation of the type constructor instance

*/
TypeConstructor region(
        "region",                           //name
        RegionProperty,                 //describing signature
        OutRegion,      InRegion,           //Out and In functions //SaveToListRegion, RestoreFromListRegion,
        SaveToListRegion, RestoreFromListRegion,  //SaveToList and RestoreFromList functions
        CreateRegion,   DeleteRegion,         //object creation and deletion
        0,        0,              // object open and save
        CloseRegion,    CloneRegion,        //object close and clone
        CastRegion,                         //cast function
        SizeOfRegion,         //sizeof function
        CheckRegion );                        //kind checking function

/*
9 Object Traversal functions

These functions are utilities useful for traversing objects.  They are basic functions
to be called by the operations defined below.

There are 6 combinations, pp, pl, pr, ll, lr, rr

*/

enum object {none, first, second, both};
enum status {endnone, endfirst, endsecond, endboth};

void ppSelectFirst(Points& P1, Points& P2, object& obj, status& stat)
{
    P1.SelectFirst();
    P2.SelectFirst();

    const Point *p1, *p2;
    bool gotP1 = P1.GetPt( p1 ),
         gotP2 = P2.GetPt( p2 );

    if( !gotP1 && !gotP2 ) {obj=none; stat=endboth;}
    else if ( !gotP1 ) {obj=second; stat=endfirst; }
    else if ( !gotP2 ) {obj=first; stat=endsecond; }
    else //both defined
    {
  stat=endnone;
  if (*p1<*p2) obj=first;
  else if (*p1>*p2) obj=second;
  else obj=both;
    }
}

void ppSelectNext(Points& P1, Points& P2, object& obj, status& stat)
{
    // 1. get the current elements
    const Point *p1, *p2;
    bool gotP1 = P1.GetPt( p1 ),
         gotP2 = P2.GetPt( p2 );

    //2. move the pointers
    if (!gotP1 && !gotP2)
    {
  //do nothing
    }
    else if (!gotP1)
    {
  P2.SelectNext();
  gotP2 = P2.GetPt( p2 );
    }
    else if (!gotP2)
    {
  P1.SelectNext();
  gotP1 = P1.GetPt( p1 );
    }
    else //both currently defined
    {
  if (*p1< *p2) //then chs1 is the last output
  {
      P1.SelectNext();
      gotP1 = P1.GetPt( p1 );
  }
  else if (p1> p2)
  {
      P2.SelectNext();
      gotP2 = P2.GetPt( p2 );
  }
  else
  {
      P1.SelectNext();
      gotP1 = P1.GetPt( p1 );
      P2.SelectNext();
      gotP2 = P2.GetPt( p2 );
  }
    }

    //3. generate the outputs
    if (!gotP1 && !gotP2) {obj=none; stat=endboth;}
    else if (!gotP1) {obj=second; stat=endfirst; }
    else if (!gotP2) {obj=first; stat=endsecond; }
    else //both defined
    {
  stat=endnone;
  if (*p1<*p2) obj=first;
  else if (*p1>*p2) obj=second;
  else obj=both;
    }
}

void plSelectFirst(Points& P, CLine& L, object& obj, status& stat)
{
    P.SelectFirst();
    L.SelectFirst();

    const Point *p1;
    Point p2;
    const CHalfSegment *chs;

    bool gotP1 = P.GetPt( p1 ),
         gotP2 = L.GetHs( chs );
    if( gotP2 )
      p2=chs->GetDPoint();

    if (!gotP1 && !gotP2 ) {obj=none; stat=endboth;}
    else if (!gotP1) {obj=second; stat=endfirst; }
    else if (!gotP2) {obj=first; stat=endsecond; }
    else //both defined
    {
  stat=endnone;
  if (*p1<p2) obj=first;
  else if (*p1>p2) obj=second;
  else obj=both;
    }
}

void plSelectNext(Points& P, CLine& L, object& obj, status& stat)
{
    // 1. get the current elements
    const Point *p1;
    Point p2;
    const CHalfSegment *chs;

    bool gotP1 = P.GetPt( p1 ),
         gotP2 = L.GetHs( chs );
    if( gotP2 ) 
      p2=chs->GetDPoint();

    //2. move the pointers
    if (!gotP1 && !gotP2 )
    {
  //do nothing
    }
    else if (!gotP1)
    {
  L.SelectNext();
  gotP2 = L.GetHs( chs );
  if( gotP2 )
    p2=chs->GetDPoint();
    }
    else if (!gotP2)
    {
  P.SelectNext();
  gotP1 = P.GetPt( p1 );
    }
    else //both currently defined
    {
  if (*p1< p2) //then chs1 is the last output
  {
      P.SelectNext();
      gotP1 = P.GetPt( p1 );
  }
  else if (*p1> p2)
  {
      L.SelectNext();
      gotP2 = L.GetHs( chs );
      if( gotP2 )
        p2=chs->GetDPoint();
  }
  else
  {
      P.SelectNext();
      gotP1 = P.GetPt( p1 );
      L.SelectNext();
      gotP2 = L.GetHs( chs );
      if( gotP2 )
        p2=chs->GetDPoint();
  }
    }

    //3. generate the outputs
    if (!gotP1 && !gotP2) {obj=none; stat=endboth;}
    else if (!gotP1) {obj=second; stat=endfirst; }
    else if (!gotP2) {obj=first; stat=endsecond; }
    else //both defined
    {
  stat=endnone;
  if (*p1<p2) obj=first;
  else if (*p1>p2) obj=second;
  else obj=both;
    }
}

void prSelectFirst(Points& P, CRegion& R, object& obj, status& stat)
{
    P.SelectFirst();
    R.SelectFirst();

    const Point *p1;
    Point p2;
    const CHalfSegment *chs;

    bool gotP1 = P.GetPt( p1 ),
         gotP2 = R.GetHs( chs );
    if( gotP2 )
      p2=chs->GetDPoint();

    if (!gotP1 && !gotP2) {obj=none; stat=endboth;}
    else if (!gotP1) {obj=second; stat=endfirst; }
    else if (!gotP2) {obj=first; stat=endsecond; }
    else //both defined
    {
  stat=endnone;
  if (*p1<p2) obj=first;
  else if (*p1>p2) obj=second;
  else obj=both;
    }
}

void prSelectNext(Points& P, CRegion& R, object& obj, status& stat)
{
    // 1. get the current elements
    const Point *p1;
    Point p2;
    const CHalfSegment *chs;

    bool gotP1 = P.GetPt( p1 ),
         gotP2 = R.GetHs( chs );
    if( gotP2 )
      p2=chs->GetDPoint();

    //2. move the pointers
    if (!gotP1 && !gotP2)
    {
  //do nothing
    }
    else if (!gotP1)
    {
  R.SelectNext();
  gotP2 = R.GetHs( chs );
  if( gotP2 )
    p2=chs->GetDPoint();
    }
    else if (!gotP2)
    {
  P.SelectNext();
  gotP1 = P.GetPt( p1 );
    }
    else //both currently defined
    {
  if (*p1< p2) //then chs1 is the last output
  {
      P.SelectNext();
      gotP1 = P.GetPt( p1 );
  }
  else if (*p1> p2)
  {
      R.SelectNext();
      gotP2 = R.GetHs( chs );
      if( gotP2 )
        p2=chs->GetDPoint();
  }
  else
  {
      P.SelectNext();
      gotP1 = P.GetPt( p1 );
      R.SelectNext();
      gotP2 = R.GetHs( chs );
      if( gotP2 )
        p2=chs->GetDPoint();
  }
    }

    //3. generate the outputs
    if (!gotP1 && !gotP2) {obj=none; stat=endboth;}
    else if (!gotP1) {obj=second; stat=endfirst; }
    else if (!gotP2) {obj=first; stat=endsecond; }
    else //both defined
    {
  stat=endnone;
  if (*p1<p2) obj=first;
  else if (*p1>p2) obj=second;
  else obj=both;
    }
}

void llSelectFirst(CLine& L1, CLine& L2, object& obj, status& stat)
{
    L1.SelectFirst();
    L2.SelectFirst();

    const CHalfSegment *chs1, *chs2;
    bool got1 = L1.GetHs( chs1 ),
         got2 = L2.GetHs( chs2 );

    if (!got1 && !got2) {obj=none; stat=endboth;}
    else if (!got1) {obj=second; stat=endfirst; }
    else if (!got2) {obj=first; stat=endsecond; }
    else //both defined
    {
  stat=endnone;
  if (*chs1<*chs2) obj=first;
  else if (*chs1>*chs2) obj=second;
  else obj=both;
    }
}

void llSelectNext(CLine& L1, CLine& L2, object& obj, status& stat)
{
    // 1. get the current elements
    const CHalfSegment *chs1, *chs2;
    bool got1 = L1.GetHs( chs1 ),
         got2 = L2.GetHs( chs2 );

    //2. move the pointers
    if (!got1 && !got2)
    {
  //do nothing
    }
    else if (!got1)
    {
  L2.SelectNext();
  got2 = L2.GetHs( chs2 );
    }
    else if (!got2)
    {
  L1.SelectNext();
  got1 = L1.GetHs( chs1 );
    }
    else //both currently defined
    {
  if (*chs1< *chs2) //then chs1 is the last output
  {
      L1.SelectNext();
      got1 = L1.GetHs( chs1 );
  }
  else if (*chs1> *chs2)
  {
      L2.SelectNext();
      got2 = L2.GetHs( chs2 );
  }
  else
  {
      L1.SelectNext();
      got1 = L1.GetHs( chs1 );
      L2.SelectNext();
      got2 = L2.GetHs( chs2 );
  }
    }

    //3. generate the outputs
    if (!got1 && !got2) {obj=none; stat=endboth;}
    else if (!got1) {obj=second; stat=endfirst; }
    else if (!got2) {obj=first; stat=endsecond; }
    else //both defined
    {
  stat=endnone;
  if (*chs1<*chs2) obj=first;
  else if (*chs1>*chs2) obj=second;
  else obj=both;
    }
}

void lrSelectFirst(CLine& L, CRegion& R, object& obj, status& stat)
{
    L.SelectFirst();
    R.SelectFirst();

    const CHalfSegment *chs1, *chs2;
    bool got1 = L.GetHs( chs1 ),
         got2 = R.GetHs( chs2 );

    if (!got1 && !got2) {obj=none; stat=endboth;}
    else if (!got1) {obj=second; stat=endfirst; }
    else if (!got2) {obj=first; stat=endsecond; }
    else //both defined
    {
  stat=endnone;
  if (*chs1<*chs2) obj=first;
  else if (*chs1>*chs2) obj=second;
  else obj=both;
    }
}

void lrSelectNext(CLine& L, CRegion& R, object& obj, status& stat)
{
    // 1. get the current elements
    const CHalfSegment *chs1, *chs2;
    bool got1 = L.GetHs( chs1 ),
         got2 = R.GetHs( chs2 );

    //2. move the pointers
    if (!got1 && !got2)
    {
  //do nothing
    }
    else if (!got1)
    {
  R.SelectNext();
  got2 = R.GetHs( chs2 );
    }
    else if (!got2)
    {
  L.SelectNext();
  got1 = L.GetHs( chs1 );
    }
    else //both currently defined
    {
  if (*chs1< *chs2) //then chs1 is the last output
  {
      L.SelectNext();
      got1 = L.GetHs( chs1 );
  }
  else if (*chs1> *chs2)
  {
      R.SelectNext();
      got2 = R.GetHs( chs2 );
  }
  else
  {
      L.SelectNext();
      got1 = L.GetHs( chs1 );
      R.SelectNext();
      got2 = R.GetHs( chs2 );
  }
    }

    //3. generate the outputs
    if (!got1 && !got2) {obj=none; stat=endboth;}
    else if (!got1) {obj=second; stat=endfirst; }
    else if (!got2) {obj=first; stat=endsecond; }
    else //both defined
    {
  stat=endnone;
  if (*chs1<*chs2) obj=first;
  else if (*chs1>*chs2) obj=second;
  else obj=both;
    }
}

void rrSelectFirst(CRegion& R1, CRegion& R2, object& obj, status& stat)
{
    R1.SelectFirst();
    R2.SelectFirst();

    const CHalfSegment *chs1, *chs2;
    bool got1 = R1.GetHs( chs1 ),
         got2 = R2.GetHs( chs2 );

    if (!got1 && !got2) {obj=none; stat=endboth;}
    else if (!got1) {obj=second; stat=endfirst; }
    else if (!got2) {obj=first; stat=endsecond; }
    else //both defined
    {
  stat=endnone;
  if (*chs1<*chs2) obj=first;
  else if (*chs1>*chs2) obj=second;
  else obj=both;
    }
}

void rrSelectNext(CRegion& R1, CRegion& R2, object& obj, status& stat)
{
    // 1. get the current elements
    const CHalfSegment *chs1, *chs2;
    bool got1 = R1.GetHs( chs1 ),
         got2 = R2.GetHs( chs2 );

    //2. move the pointers
    if (!got1 && !got2)
    {
  //do nothing
    }
    else if (!got1)
    {
  R2.SelectNext();
  got2 = R2.GetHs( chs2 );
    }
    else if (!got2)
    {
  R1.SelectNext();
  got1 = R1.GetHs( chs1 );
    }
    else //both currently defined
    {
  if (*chs1< *chs2) //then chs1 is the last output
  {
      R1.SelectNext();
      got1 = R1.GetHs( chs1 );
  }
  else if (*chs1> *chs2)
  {
      R2.SelectNext();
      got2 = R2.GetHs( chs2 );
  }
  else
  {
      R1.SelectNext();
      got1 = R1.GetHs( chs1 );
      R2.SelectNext();
      got2 = R2.GetHs( chs2 );
  }
    }

    //3. generate the outputs
    if (!got1 && !got2) {obj=none; stat=endboth;}
    else if (!got1) {obj=second; stat=endfirst; }
    else if (!got2) {obj=first; stat=endsecond; }
    else //both defined
    {
  stat=endnone;
  if (*chs1<*chs2) obj=first;
  else if (*chs1>*chs2) obj=second;
  else obj=both;
    }
}


/*
10 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

10.1 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

10.1.1 Type mapping function SpatialTypeMapBool

It is for the compare operators which have ~bool~ as resulttype, like =, !=, <,
<=, >, >=.

*/
ListExpr
SpatialTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( SpatialTypeOfSymbol( arg1 ) == stpoint && SpatialTypeOfSymbol( arg2 ) == stpoint)
      return (nl->SymbolAtom( "bool" ));
    if ( SpatialTypeOfSymbol( arg1 ) == stpoints && SpatialTypeOfSymbol( arg2 ) == stpoints)
      return (nl->SymbolAtom( "bool" ));
    if ( SpatialTypeOfSymbol( arg1 ) == stline && SpatialTypeOfSymbol( arg2 ) == stline)
      return (nl->SymbolAtom( "bool" ));
    if ( SpatialTypeOfSymbol( arg1 ) == stregion && SpatialTypeOfSymbol( arg2 ) == stregion)
      return (nl->SymbolAtom( "bool" ));
    if ( SpatialTypeOfSymbol( arg1 ) == stpoint && SpatialTypeOfSymbol( arg2 ) == stpoints)
      return (nl->SymbolAtom( "bool" ));
    if ( SpatialTypeOfSymbol( arg1 ) == stpoints && SpatialTypeOfSymbol( arg2 ) == stpoint)
      return (nl->SymbolAtom( "bool" ));
    if ( SpatialTypeOfSymbol( arg1 ) == stpoint && SpatialTypeOfSymbol( arg2 ) == stline)
      return (nl->SymbolAtom( "bool" ));
    if ( SpatialTypeOfSymbol( arg1 ) == stpoint && SpatialTypeOfSymbol( arg2 ) == stregion)
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}


/*
10.1.2 Type mapping function GeoGeoMapBool

It is for the binary operators which have ~bool~ as result type, such as interscets,
inside, onborder, ininterior, etc.

*/

ListExpr
GeoGeoMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if (((SpatialTypeOfSymbol( arg1 ) == stpoint)  ||
         (SpatialTypeOfSymbol( arg1 ) == stpoints) ||
         (SpatialTypeOfSymbol( arg1 ) == stline)     ||
         (SpatialTypeOfSymbol( arg1 ) == stregion)) &&
        ((SpatialTypeOfSymbol( arg2 ) == stpoint)  ||
         (SpatialTypeOfSymbol( arg2 ) == stpoints) ||
         (SpatialTypeOfSymbol( arg2 ) == stline)     ||
         (SpatialTypeOfSymbol( arg2 ) == stregion)))
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.3 Type mapping function SpatialTypeMapBool1

It is for the operator ~isempty~ which have ~point~, ~points~, ~line~, and ~region~ as input and ~bool~ resulttype.

*/

ListExpr
SpatialTypeMapBool1( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( SpatialTypeOfSymbol( arg1 ) == stpoint )
      return (nl->SymbolAtom( "bool" ));
    if ( SpatialTypeOfSymbol( arg1 ) == stpoints )
      return (nl->SymbolAtom( "bool" ));
    if ( SpatialTypeOfSymbol( arg1 ) == stline )
      return (nl->SymbolAtom( "bool" ));
    if ( SpatialTypeOfSymbol( arg1 ) == stregion )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.4 Type mapping function for operator ~intersection~

This type mapping function is the one for ~intersection~ operator. This is a SET operation
so that the result type is a set such as points, line, or region.

*/

static ListExpr
intersectionMap( ListExpr args )
{
    ListExpr arg1, arg2;
    if ( nl->ListLength( args ) == 2 )
    {
  arg1 = nl->First( args );
  arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( "point" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (nl->SymbolAtom( "point" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( "point" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (nl->SymbolAtom( "point" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( "point" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (nl->SymbolAtom( "point" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( "point" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (nl->SymbolAtom( "points" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (nl->SymbolAtom( "points" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (nl->SymbolAtom( "points" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (nl->SymbolAtom( "points" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (nl->SymbolAtom( "points" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (nl->SymbolAtom( "line" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (nl->SymbolAtom( "line" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (nl->SymbolAtom( "line" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (nl->SymbolAtom( "region" ));

    }
    return (nl->SymbolAtom( "typeerror" ));
}



/*
10.1.5 Type mapping function for operator ~minus~

This type mapping function is the one for ~minus~ operator. This is a SET operation
so that the result type is a set such as points, line, or region.

*/

 ListExpr
minusMap( ListExpr args )
{
    ListExpr arg1, arg2;
    if ( nl->ListLength( args ) == 2 )
    {
  arg1 = nl->First( args );
  arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( "point" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( "points" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( "line" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( "region" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (nl->SymbolAtom( "points" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (nl->SymbolAtom( "line" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (nl->SymbolAtom( "region" ));

//  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
//       SpatialTypeOfSymbol( arg2 ) == stline )
//      return (nl->SymbolAtom( "line" ));

//  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
//       SpatialTypeOfSymbol( arg2 ) == stline )
//      return (nl->SymbolAtom( "region" ));

//  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
//       SpatialTypeOfSymbol( arg2 ) == stregion )
//      return (nl->SymbolAtom( "region" ));

    }
    return (nl->SymbolAtom( "typeerror" ));
}


/*
10.1.6 Type mapping function for operator ~union~

This type mapping function is the one for ~union~ operator. This is a SET operation
so that the result type is a set such as points, line, or region.

*/

 ListExpr
unionMap( ListExpr args )
{
    ListExpr arg1, arg2;
    if ( nl->ListLength( args ) == 2 )
    {
  arg1 = nl->First( args );
  arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (nl->SymbolAtom( "points" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( "points" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (nl->SymbolAtom( "points" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (nl->SymbolAtom( "line" ));
    }
    return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.7 Type mapping function for operator ~crossings~

This type mapping function is the one for ~crossings~ operator. This operator
compute the crossing point of two lines so that the result type is a set of points.

*/

ListExpr
crossingsMap( ListExpr args )
{
    ListExpr arg1, arg2;
    if ( nl->ListLength( args ) == 2 )
    {
  arg1 = nl->First( args );
  arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (nl->SymbolAtom( "points" ));
    }
    return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.8 Type mapping function for operator ~single~

This type mapping function is used for the ~single~ operator. This
operator transform a single-element points value to a point.

*/
ListExpr
singleMap( ListExpr args )
{
    ListExpr arg1;
    if ( nl->ListLength( args ) == 1 )
    {
  arg1 = nl->First( args );

  if (SpatialTypeOfSymbol( arg1 ) == stpoints)
      return (nl->SymbolAtom( "point" ));
    }
    return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.9 Type mapping function for operator ~distance~

This type mapping function is used for the ~distance~ operator. This
operator computes the distance between two spatial objects.

*/
ListExpr
distanceMap( ListExpr args )
{
    ListExpr arg1, arg2;
    if ( nl->ListLength( args ) == 2 )
    {
  arg1 = nl->First( args );
  arg2 = nl->Second( args );
  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( "real" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (nl->SymbolAtom( "real" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( "real" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (nl->SymbolAtom( "real" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( "real" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (nl->SymbolAtom( "real" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( "real" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (nl->SymbolAtom( "real" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (nl->SymbolAtom( "real" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (nl->SymbolAtom( "real" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (nl->SymbolAtom( "real" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (nl->SymbolAtom( "real" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (nl->SymbolAtom( "real" ));
    }

    return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.10 Type mapping function for operator ~direction~

This type mapping function is used for the ~direction~ operator. This
operator computes the direction from the first point to the second point.

*/
ListExpr
directionMap( ListExpr args )
{
    ListExpr arg1, arg2;
    if ( nl->ListLength( args ) == 2 )
    {
  arg1 = nl->First( args );
  arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( "real" ));
    }

    return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.11 Type mapping function for operator ~no\_compoents~

This type mapping function is used for the ~no\_components~ operator. This
operator computes the number of components of a spatial object. For poins,
this function returns the number of points contained in the point set.
For regions, this function returns the faces of the region.

*/
ListExpr
nocomponentsMap( ListExpr args )
{
    ListExpr arg1;
    if ( nl->ListLength( args ) == 1 )
    {
  arg1 = nl->First( args );

  if ((SpatialTypeOfSymbol( arg1 ) == stpoints)||
      (SpatialTypeOfSymbol( arg1 ) == stline)||
      (SpatialTypeOfSymbol( arg1 ) == stregion))
      return (nl->SymbolAtom( "int" ));
    }
    return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.11 Type mapping function for operator ~no\_segments~

This type mapping function is used for the ~no\_segments~ operator. This
operator computes the number of segments of a spatial object (lines and
regions only).

*/
static ListExpr
nosegmentsMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if ((SpatialTypeOfSymbol( arg1 ) == stline)||
        (SpatialTypeOfSymbol( arg1 ) == stregion))
        return (nl->SymbolAtom( "int" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.12 Type mapping function for operator ~size~

This type mapping function is used for the ~size~ operator. This operator
computes the size of the spatial object. For line, the size is the totle length
of the line segments.

*/
ListExpr
sizeMap( ListExpr args )
{
    ListExpr arg1;
    if ( nl->ListLength( args ) == 1 )
    {
  arg1 = nl->First( args );

  if (SpatialTypeOfSymbol( arg1 ) == stline)
      return (nl->SymbolAtom( "real" ));
    }
    return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.13 Type mapping function for operator ~touchpoints~

This type mapping function is used for the ~touchpoints~ operator. This operator
computes the touchpoints of a region and another region or a line.

*/
ListExpr
touchpointsMap( ListExpr args )
{
    ListExpr arg1, arg2;
    if ( nl->ListLength( args ) == 2 )
    {
  arg1 = nl->First( args );
  arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (nl->SymbolAtom( "points" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (nl->SymbolAtom( "points" ));

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (nl->SymbolAtom( "points" ));
    }

    return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.14 Type mapping function for operator ~commonborder~

This type mapping function is used for the ~commonborder~ operator. This operator
computes the commonborder of two regions.

*/
ListExpr
commonborderMap( ListExpr args )
{
    ListExpr arg1, arg2;
    if ( nl->ListLength( args ) == 2 )
    {
  arg1 = nl->First( args );
  arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (nl->SymbolAtom( "line" ));
    }

    return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.15 Type mapping function for operator ~bbox~

This type mapping function is used for the ~bbox~ operator. This operator
computes the bbox of a region, which is a ~rect~ (see RectangleAlgebra).

*/
ListExpr
bboxMap( ListExpr args )
{
    ListExpr arg1;
    if ( nl->ListLength( args ) == 1 )
    {
  arg1 = nl->First( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stregion ||
             SpatialTypeOfSymbol( arg1 ) == stpoint ||
             SpatialTypeOfSymbol( arg1 ) == stline ||
             SpatialTypeOfSymbol( arg1 ) == stpoints )
      return (nl->SymbolAtom( "rect" ));
    }
    return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.16 Type mapping function for operator ~insidepathlength~ and ~insidescanned~

This type mapping function is used for the ~insidepathlength~ and  ~insidescanned~ operators.

*/
ListExpr
insidepsMap( ListExpr args )
{
    ListExpr arg1, arg2;
    if ( nl->ListLength( args ) == 2 )
    {
  arg1 = nl->First( args );
  arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (nl->SymbolAtom( "int" ));
    }

    return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.17 Type mapping function for operator ~translate~

This type mapping function is used for the ~translate~ operator. This operator
moves a region parallelly to another place and gets another region.

*/
static ListExpr
TranslateMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( SpatialTypeOfSymbol( arg1 ) == stregion &&
        nl->IsEqual(nl->First( arg2 ), "real") &&
        nl->IsEqual(nl->Second( arg2 ), "real"))
      return (nl->SymbolAtom( "region" ));

    if( SpatialTypeOfSymbol( arg1 ) == stline &&
        nl->IsEqual(nl->First( arg2 ), "real") &&
        nl->IsEqual(nl->Second( arg2 ), "real"))
      return (nl->SymbolAtom( "line" ));

    if( SpatialTypeOfSymbol( arg1 ) == stpoints &&
        nl->IsEqual(nl->First( arg2 ), "real") &&
        nl->IsEqual(nl->Second( arg2 ), "real"))
      return (nl->SymbolAtom( "points" ));

    if( SpatialTypeOfSymbol( arg1 ) == stpoint &&
        nl->IsEqual(nl->First( arg2 ), "real") &&
        nl->IsEqual(nl->Second( arg2 ), "real"))
      return (nl->SymbolAtom( "point" ));
  }

  return nl->SymbolAtom( "typeerror" );
}

/*
10.1.18 Type mapping function for operator ~scale~

This type mapping function is used for the ~scale~ operator. This operator
scales a spatial object by a given factor.

*/
static ListExpr ScaleMap(ListExpr args) {
   if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("operator scale requires two arguments");
      return nl->SymbolAtom( "typeerror" );
   }
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if(!(nl->IsEqual(arg2 , "real"))){
      ErrorReporter::ReportError("the second " 
                                 "argument has to be of type real");
      return nl->SymbolAtom("typeerror");
   }
   if(nl->IsEqual(arg1,"region"))
     return nl->SymbolAtom("region");
   if(nl->IsEqual(arg1,"line"))
     return nl->SymbolAtom("line");
   if(nl->IsEqual(arg1,"point"))
     return nl->SymbolAtom("point");
   if(nl->IsEqual(arg1,"points"))
     return nl->SymbolAtom("points");
   ErrorReporter::ReportError("First argument has to be in "
                              "{region, line, points, points}");

   return nl->SymbolAtom( "typeerror" );
}




/*
10.1.17 Type mapping function for operator ~windowclipping~

This type mapping function is used for the ~windowclipping~ operators. There are
two kind of operators, one that computes the part of the object that is inside
the window (windowclippingin), and another one that computes the part that is
outside of it (windowclippingout).

*/
ListExpr
windowclippingMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if ( SpatialTypeOfSymbol( arg1 ) == stline)
        return (nl->SymbolAtom( "line" ));
    if ( SpatialTypeOfSymbol( arg1 ) == stregion )
        return (nl->SymbolAtom( "region" ));

  }

  return (nl->SymbolAtom( "typeerror" ));
}

/*
10.1.17 Type mapping function for operator ~clip~

This type mapping function is used for the ~clip~ operator.

*/
static ListExpr
clipMap( ListExpr args )
{
    ListExpr arg1, arg2;
    if ( nl->ListLength( args ) == 2 )
    {
        arg1 = nl->First( args );
        arg2 = nl->Second( args );

        if ( SpatialTypeOfSymbol( arg1 ) == stline &&
             SpatialTypeOfSymbol( arg2 ) == stbox )
            return (nl->SymbolAtom( "line" ));
    }

    return (nl->SymbolAtom( "typeerror" ));
}

/*
10.3 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

10.3.1 Selection function ~SimpleSelect~

Is used for all non-overloaded operators.

*/
int
SimpleSelect( ListExpr args )
{
  return (0);
}

/*
10.3.2 Selection function ~SpatialSelectIsEmpty~

It is used for the ~isempty~ operator

*/
int
SpatialSelectIsEmpty( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  if ( SpatialTypeOfSymbol( arg1 ) == stpoint )
    return (0);
  if ( SpatialTypeOfSymbol( arg1 ) == stpoints )
    return (1);
  if ( SpatialTypeOfSymbol( arg1 ) == stline )
    return (2);
  if ( SpatialTypeOfSymbol( arg1 ) == stregion )
    return (3);
  return (-1); // This point should never be reached
}

/*
10.3.3 Selection function ~SpatialSelectCompare~

It is used for compare operators ($=$, $\neq$, $<$, $>$, $\geq$, $\leq$)

*/
int
SpatialSelectCompare( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
    return (0);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
    return (1);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
    return (2);

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return (3);

  return (-1); // This point should never be reached
}

/*
10.3.4 Selection function ~intersectSelect~

It is used for the operator ~intersects~

*/

int
intersectSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (0);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (1);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (2);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (3);

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (4);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (5);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (6);

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (7);

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (8);

  return (-1); // This point should never be reached
}

/*
10.3.5 Selection function ~insideSelect~

This select function is used for the ~inside~ operator.

*/

int
insideSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (0);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (1);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (2);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (3);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (4);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (5);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (6);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (7);

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (8);

  return (-1); // This point should never be reached
}

/*
10.3.6 Selection function ~touches-attached-overlapsSelect~

This select function is used for the ~touches~ , ~attached~ , and ~overlaps~  operator.

*/

int
touches_attached_overlapsSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (0);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (1);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (2);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (3);

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (4);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (5);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (6);

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (7);

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (8);

  return (-1); // This point should never be reached
}

/*
10.3.7 Selection function ~onBorder \& inInteriorSelect~

This select function is used for the ~onborder~ operator and the ~ininterior~ operator.

*/

int
onBorder_inInteriorSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (0);
  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (1);

  return (-1); // This point should never be reached
}

/*
10.3.8 Selection function ~intersectionSelect~

This select function is used for the ~intersection~ operator.

*/

int
intersectionSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (0);
  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (1);
  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (2);
  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (3);
  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (4);
  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (5);
  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (6);
  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (7);
  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (8);
  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (9);
  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (10);
  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (11);
  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (12);
//  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
//       SpatialTypeOfSymbol( arg2 ) == stregion )
//      return (13);
//  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
//       SpatialTypeOfSymbol( arg2 ) == stline )
//      return (14);
//  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
//       SpatialTypeOfSymbol( arg2 ) == stregion )
//      return (15);

  return (-1); // This point should never be reached
}

/*
10.3.9 Selection function ~minusSelect~

This select function is used for the ~minus~ operator.

*/

int
minusSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (0);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (1);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (2);

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (3);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (4);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (5);

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (6);

  return (-1); // This point should never be reached
}

/*
10.3.10 Selection function ~unionSelect~

This select function is used for the ~union~ operator.

*/

int
unionSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (0);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (1);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (2);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (3);

  return (-1); // This point should never be reached
}

/*
10.3.11 Selection function ~crossingsSelect~

This select function is used for the ~crossings~ operator.

*/

int
crossingsSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (0);

  return (-1); // This point should never be reached
}

/*
10.3.12 Selection function ~singleSelect~

This select function is used for the ~single~ operator.

*/

int
singleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints)
      return (0);

  return (-1); // This point should never be reached
}

/*
10.3.13 Selection function ~distanceSelect~

This select function is used for the ~distance~ operator.

*/

int
distanceSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (0);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (1);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (2);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (3);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (4);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (5);

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (6);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (7);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (8);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (9);

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (10);

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
      return (11);

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (12);

  return (-1); // This point should never be reached
}

/*
10.3.14 Selection function ~directionSelect~

This select function is used for the ~direction~ operator.

*/

int
directionSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (0);

  return (-1); // This point should never be reached
}

/*
10.3.15 Selection function ~nocomponentsSelect~

This select function is used for the ~nocomponents~ operator.

*/

int
nocomponentsSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if (SpatialTypeOfSymbol( arg1 ) == stpoints)
      return (0);

  if (SpatialTypeOfSymbol( arg1 ) == stline)
      return (1);

  if (SpatialTypeOfSymbol( arg1 ) == stregion)
      return (2);

  return (-1); // This point should never be reached
}

/*
10.3.16 Selection function ~nosegmentsSelect~

This select function is used for the ~no\_segments~ operator.

*/

static int
nosegmentsSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if (SpatialTypeOfSymbol( arg1 ) == stline)
      return (0);

  if (SpatialTypeOfSymbol( arg1 ) == stregion)
      return (1);

  return (-1); // This point should never be reached
}

/*
10.3.16 Selection function ~bboxSelect~

This select function is used for the ~bbox~ operator.

*/

int
bboxSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if (SpatialTypeOfSymbol( arg1 ) == stpoint)
      return (0);
  if (SpatialTypeOfSymbol( arg1 ) == stpoints)
      return (1);
  if (SpatialTypeOfSymbol( arg1 ) == stline)
      return (2);
  if (SpatialTypeOfSymbol( arg1 ) == stregion)
      return (3);

  return (-1); // This point should never be reached
}

/*
10.3.16 Selection function ~sizeSelect~

This select function is used for the ~size~ operator.

*/

int
sizeSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if (SpatialTypeOfSymbol( arg1 ) == stline)
      return (0);

  return (-1); // This point should never be reached
}

/*
10.3.17 Selection function ~touchpointsSelect~

This select function is used for the ~touchpoints~ operator.

*/

int
touchpointsSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (0);

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stline )
      return (1);

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (2);

  return (-1); // This point should never be reached
}

/*
10.3.18 Selection function ~commonborderSelect~

This select function is used for the ~commonborder~ operator.

*/

int
commonborderSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
      return (0);

  return (-1); // This point should never be reached
}

/*
10.3.19 Selection function ~translateSelect~

This select function is used for the ~translate~ operator.

*/

static int
TranslateSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if (SpatialTypeOfSymbol( arg1 ) == stpoint)
      return (0);

  if (SpatialTypeOfSymbol( arg1 ) == stpoints)
      return (1);

  if (SpatialTypeOfSymbol( arg1 ) == stline)
      return (2);

  if (SpatialTypeOfSymbol( arg1 ) == stregion)
      return (3);

  return (-1); // This point should never be reached
}

/*
10.3.19 Selection function ~ScaleSelect~

This select function is used for the ~scale~ operator.

*/

static int
ScaleSelect( ListExpr args )
{
  // use the same mapping as for translate
  return TranslateSelect(args);
}

/*
10.3.19 Selection function ~windowclippingSelect~

This select function is used for the ~windowclipping(in)(out)~ operator.

*/

int
windowclippingSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if (SpatialTypeOfSymbol( arg1 ) == stline)
      return (0);

  if (SpatialTypeOfSymbol( arg1 ) == stregion)
      return (1);

  return (-1); // This point should never be reached
}

/*
10.1.17 Type mapping function for operator ~components~

This type mapping function is used for the ~components~ operator.

*/
static ListExpr
ComponentsMap( ListExpr args )
{
  if( nl->ListLength( args ) == 1 )
  {
    if( SpatialTypeOfSymbol( nl->First( args ) ) == stregion )
      return nl->TwoElemList( nl->SymbolAtom("stream"),
                              nl->SymbolAtom("region") );
  }
  return (nl->SymbolAtom( "typeerror" ));
}



/*
10.4 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are
several value mapping functions, one for each possible combination of input
parameter types.

10.4.1 Value mapping functions of operator ~isempty~

*/
int
SpatialIsEmpty_p( Word* args, Word& result, int message, Word& local, Supplier s )
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

int
SpatialIsEmpty_ps( Word* args, Word& result, int message, Word& local, Supplier s )
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

int
SpatialIsEmpty_l( Word* args, Word& result, int message, Word& local, Supplier s )
{  //To Judge whether a line value is empty
    result = qp->ResultStorage( s );

    if( ((CLine*)args[0].addr)->IsEmpty() )
    {
  ((CcBool*)result.addr)->Set( true, true );
    }
    else
    {
  ((CcBool *)result.addr)->Set( true, false );
    }
    return (0);
}

int
SpatialIsEmpty_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    if( ((CRegion*)args[0].addr)->IsEmpty() )
    {
  ((CcBool*)result.addr)->Set( true, true );
    }
    else
    {
  ((CcBool *)result.addr)->Set( true, false );
    }
    return (0);
}

/*
10.4.2 Value mapping functions of operator ~$=$~

*/
int
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

int
SpatialEqual_psps( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool *)result.addr)->Set( true, *((Points*)args[0].addr) == *((Points*)args[1].addr) );
  return (0);
}

int
SpatialEqual_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{   //to judge whether two line values are equal
    result = qp->ResultStorage( s );
    ((CcBool *)result.addr)->Set( true, *((CLine*)args[0].addr) == *((CLine*)args[1].addr) );
    return (0);

}

int
SpatialEqual_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    ((CcBool *)result.addr)->Set( true, *((CRegion*)args[0].addr) == *((CRegion*)args[1].addr) );
    return (0);
}

/*
10.4.3 Value mapping functions of operator ~$\neq$~

*/
int
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

int
SpatialNotEqual_psps( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool *)result.addr)->Set( true, *((Points*)args[0].addr) != *((Points*)args[1].addr) );
  return (0);
}

int
SpatialNotEqual_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{  //to judge whether two line values are not equal
    result = qp->ResultStorage( s );
    ((CcBool *)result.addr)->Set( true, !(*((CLine*)args[0].addr) == *((CLine*)args[1].addr)));
    return (0);
}

int
SpatialNotEqual_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    ((CcBool *)result.addr)->Set( true, !(*((CRegion*)args[0].addr) == *((CRegion*)args[1].addr)));
    return (0);
}

/*
10.4.4 Value mapping functions of operator ~$<$~

*/
int
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
10.4.5 Value mapping functions of operator ~$\leq$~

*/
int
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
10.4.6 Value mapping functions of operator ~$>$~

*/
int
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
10.4.7 Value mapping functions of operator ~$\geq$~

*/
int
SpatialGreaterEqual_pp( Word* args, Word& result, int message, 
                        Word& local, Supplier s )
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
10.4.8 Value mapping functions of operator ~intersects~

*/
int
SpatialIntersects_psps( Word* args, Word& result, int message, 
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  ((CcBool *)result.addr)->
    Set( true, 
         ((Points*)args[0].addr)->Intersects( *((Points*)args[1].addr) ) );

  return (0);
}

int
SpatialIntersects_psl( Word* args, Word& result, int message, 
                       Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps;
    CLine *cl;
    const Point *p;
    const CHalfSegment *chs;

    ps=((Points*)args[0].addr);
    cl=((CLine*)args[1].addr);

    if(! ps->BoundingBox().Intersects( cl->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<ps->Size(); i++)
    {
  ps->Get(i, p);
  for (int j=0; j<cl->Size(); j++)
  {
      cl->Get(j, chs);
      if (chs->Contains(*p))
      {
    ((CcBool *)result.addr)->Set( true, true);
    return (0);
      }
  }
    }

    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
SpatialIntersects_lps( Word* args, Word& result, int message, 
                       Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps;
    CLine *cl;
    const Point *p;
    const CHalfSegment *chs;

    ps=((Points*)args[1].addr);
    cl=((CLine*)args[0].addr);

    if(! ps->BoundingBox().Intersects( cl->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<ps->Size(); i++)
    {
  ps->Get(i, p);
  for (int j=0; j<cl->Size(); j++)
  {
      cl->Get(j, chs);
      if (chs->Contains(*p))
      {
    ((CcBool *)result.addr)->Set( true, true);
    return (0);
      }
  }
    }

    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
SpatialIntersects_psr( Word* args, Word& result, int message, 
                       Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps;
    CRegion *cr;
    const Point *p;

    ps=((Points*)args[0].addr);
    cr=((CRegion*)args[1].addr);

    if(! ps->BoundingBox().Intersects( cr->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<ps->Size(); i++)
    {
  ps->Get(i, p);

  if (cr->contain(*p))
  {
      ((CcBool *)result.addr)->Set( true, true);
      return (0);
  }

    }

    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
SpatialIntersects_rps( Word* args, Word& result, int message, 
                       Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps;
    CRegion *cr;
    const Point *p;

    ps=((Points*)args[1].addr);
    cr=((CRegion*)args[0].addr);

    if(! ps->BoundingBox().Intersects( cr->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<ps->Size(); i++)
    {
  ps->Get(i, p);

  if (cr->contain(*p))
  {
      ((CcBool *)result.addr)->Set( true, true);
      return (0);
  }

    }

    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
SpatialIntersects_ll( Word* args, Word& result, int message, 
                      Word& local, Supplier s )
{   //to judge whether two lines intersect each other.
    result = qp->ResultStorage( s );
    CLine *cl1, *cl2;
    const CHalfSegment *chs1, *chs2;

    cl1=((CLine*)args[0].addr);
    cl2=((CLine*)args[1].addr);

    if(! cl1->BoundingBox().Intersects( cl2->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cl1->Size(); i++)
    {
  cl1->Get(i, chs1);
  if (chs1->GetLDP())
  {
      for (int j=0; j<cl2->Size(); j++)
      {
    cl2->Get(j, chs2);
    if (chs2->GetLDP())
    {
        if (chs1->Intersects(*chs2))
        {
      ((CcBool *)result.addr)->Set( true, true );
      return (0);
        }
    }
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
SpatialIntersects_lr( Word* args, Word& result, int message, 
                      Word& local, Supplier s )
{
    //to judge whether line intersects with region.
    result = qp->ResultStorage( s );
    CLine *cl;
    CRegion *cr;
    const CHalfSegment *chsl, *chsr;

    cl=((CLine*)args[0].addr);
    cr=((CRegion*)args[1].addr);

    if(! cl->BoundingBox().Intersects( cr->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cl->Size(); i++)
    {
  cl->Get(i, chsl);
  if (chsl->GetLDP())
  {
      for (int j=0; j<cr->Size(); j++)
      {
    cr->Get(j, chsr);
    if (chsr->GetLDP())
    {
        if (chsl->Intersects(*chsr))
        {
      ((CcBool *)result.addr)->Set( true, true );
      return (0);
        }
    }
      }

      if ((cr->contain(chsl->GetLP()))|| (cr->contain(chsl->GetRP())))
      {
    ((CcBool *)result.addr)->Set( true, true);
    return (0);
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
SpatialIntersects_rl( Word* args, Word& result, int message, 
                      Word& local, Supplier s )
{
    //to judge whether line intersects with region.
    result = qp->ResultStorage( s );
    CLine *cl;
    CRegion *cr;
    const CHalfSegment *chsl, *chsr;

    cl=((CLine*)args[1].addr);
    cr=((CRegion*)args[0].addr);

    if(! cl->BoundingBox().Intersects( cr->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cl->Size(); i++)
    {
  cl->Get(i, chsl);
  if (chsl->GetLDP())
  {
      for (int j=0; j<cr->Size(); j++)
      {
    cr->Get(j, chsr);
    if (chsr->GetLDP())
    {
        if (chsl->Intersects(*chsr))
        {
      ((CcBool *)result.addr)->Set( true, true );
      return (0);
        }
    }
      }

      if ((cr->contain(chsl->GetLP()))|| (cr->contain(chsl->GetRP())))
      {
    ((CcBool *)result.addr)->Set( true, true);
    return (0);
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
SpatialIntersects_rr( Word* args, Word& result, int message, 
                      Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    CRegion *cr1, *cr2;
    const CHalfSegment *chs1, *chs2;

    cr1=((CRegion*)args[0].addr);
    cr2=((CRegion*)args[1].addr);

    //cout<<endl<<"============================================="<<endl;
    //cout<<endl<<*cr1<<endl<<"and"<<endl<<*cr2<<endl;

    if(! cr1->BoundingBox().Intersects( cr2->BoundingBox() ) )
    {
  //cout<<"not intersect by MBR"<<endl;
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    //cout<<"computing..."<<endl;
    //cout <<"cr1 size: "<<cr1->Size()<<endl;

    //1. decide the intersection of edges
    for (int i=0; i<cr1->Size(); i++)
    {
  cr1->Get(i, chs1);
  if (chs1->GetLDP())
  {

      //cout<<endl<<"===================="<<endl;
      //cout<<"chs1"<<chs1;  ////////////////////7
      //cout <<"cr2 size: "<<cr2->Size()<<endl;

      for (int j=0; j<cr2->Size(); j++)
      {
    cr2->Get(j, chs2);
    if (chs2->GetLDP())
    {
        //cout<<"chs2: "<<chs2<<endl;  ////////////////////7

        if (chs1->Intersects(*chs2))
        {
      ((CcBool *)result.addr)->Set( true, true );
      return (0);
        }
    }
      }
  }
    }

    //2. decide the case of Tong-Xin-Yuan
    for (int i=0; i<cr1->Size(); i++)
    {
  cr1->Get(i, chs1);
  if (chs1->GetLDP())
  {
      if (cr2->contain(chs1->GetLP()))
      {
    ((CcBool *)result.addr)->Set( true, true );
    return (0);
      }
  }
    }

    for (int j=0; j<cr2->Size(); j++)
    {
  cr2->Get(j, chs2);
  if (chs2->GetLDP())
  {
      if (cr1->contain(chs2->GetLP()))
      {
    ((CcBool *)result.addr)->Set( true, true );
    return (0);
      }
  }
    }

    //3. else: not intersect
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

/*
10.4.9 Value mapping functions of operator ~inside~

*/

int
SpatialInside_pps( Word* args, Word& result, int message, 
                   Word& local, Supplier s )
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

int
SpatialInside_pl( Word* args, Word& result, int message, 
                  Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Point *p=((Point*)args[0].addr);
  CLine *cl=((CLine*)args[1].addr);

  if(! p->Inside( cl->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

  const CHalfSegment *chs;

  for (int i=0; i<cl->Size(); i++)
  {
      cl->Get(i, chs);
      if (chs->Contains(*p))
      {
    ((CcBool *)result.addr)->Set( true, true );
    return (0);
      }
  }
  ((CcBool *)result.addr)->Set( true, false );
  return (0);

}

int
SpatialInside_pr( Word* args, Word& result, int message, 
                  Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    CRegion *cr=((CRegion*)args[1].addr);

    if(! p->Inside( cr->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    if (cr->contain(*p))
    { //cout<<"p inside r!!!"<<endl;
  //cout<<*p<<endl<<*cr<<endl;
  ((CcBool *)result.addr)->Set( true, true);
  return (0);
    }
    else
    { //cout<<"p NOT inside r!!!"<<endl;
  //cout<<*p<<endl<<*cr<<endl;
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }
 }

int
SpatialInside_pr_old( Word* args, Word& result, int message, 
                      Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    CRegion *cr=((CRegion*)args[1].addr);

    if(! p->Inside( cr->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    if (cr->contain_old(*p))
    { //cout<<"p inside r!!!"<<endl;
  //cout<<*p<<endl<<*cr<<endl;
  ((CcBool *)result.addr)->Set( true, true);
  return (0);
    }
    else
    { //cout<<"p NOT inside r!!!"<<endl;
  //cout<<*p<<endl<<*cr<<endl;
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }
 }

int
SpatialInside_pathlength_pr( Word* args, Word& result, int message, 
                             Word& local, Supplier s )
{
    //point + region --> int
    int pathlength=0;
    int scanned=0;

    result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    CRegion *cr=((CRegion*)args[1].addr);

    //((CcInt *)result.addr)->Set( true, ps->Size());

    if(! p->Inside( cr->BoundingBox() ) )
    {
  ((CcInt *)result.addr)->Set( true, 0);
  //no chs checked because it is done by checking BBOX
  return (0);
    }

    cr->containpr(*p, pathlength, scanned);

    ((CcInt *)result.addr)->Set( true, pathlength);
    return (0);
}

int
SpatialInside_scanned_pr( Word* args, Word& result, int message, 
                          Word& local, Supplier s )
{
    //point + region --> int
    int pathlength=0;
    int scanned=0;

    result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    CRegion *cr=((CRegion*)args[1].addr);

    //((CcInt *)result.addr)->Set( true, ps->Size());

    if(! p->Inside( cr->BoundingBox() ) )
    {
  ((CcInt *)result.addr)->Set( true, 0);
  //no chs checked because it is done by checking BBOX
  return (0);
    }

    cr->containpr(*p, pathlength, scanned);

    ((CcInt *)result.addr)->Set( true, scanned);
    return (0);
}

int
SpatialInside_psps( Word* args, Word& result, int message, 
                    Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool *)result.addr)->
    Set( true,
        ((Points*)args[0].addr)->Inside( *((Points*)args[1].addr) ) );
  return (0);
}


int
SpatialInside_psl( Word* args, Word& result, int message, 
                   Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Points *ps=((Points*)args[0].addr);
  CLine *cl=((CLine*)args[1].addr);

  if(! cl->BoundingBox().Contains( ps->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

  const Point *p;
  const CHalfSegment *chs;

  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, p);

      bool inside=false;
      for (int j=0; ((j<cl->Size())&&(inside==false)); j++)
      {
    cl->Get(j, chs);
    if (chs->Contains(*p))
    {
        inside=true;
    }
      }
      if (inside==false)
      {
    ((CcBool *)result.addr)->Set( true, false );
    return (0);
      }
  }

  ((CcBool *)result.addr)->Set( true, true );
  return (0);
}

int
SpatialInside_psr( Word* args, Word& result, int message, 
                   Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Points *ps=((Points*)args[0].addr);
  CRegion *cr=((CRegion*)args[1].addr);

  if(! cr->BoundingBox().Contains( ps->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

  const Point *p;

  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, p);
      if (!(cr->contain(*p)))
      {
    ((CcBool *)result.addr)->Set( true, false );
    return (0);
      }
  }

  ((CcBool *)result.addr)->Set( true, true );
  return (0);
}

int
SpatialInside_ll( Word* args, Word& result, int message, 
                  Word& local, Supplier s )
{
    //to decide whether one line value is inside another
    result = qp->ResultStorage( s );
    CLine *cl1, *cl2;
    const CHalfSegment *chs1, *chs2;

    cl1=((CLine*)args[0].addr);
    cl2=((CLine*)args[1].addr);

    if(! cl2->BoundingBox().Contains( cl1->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cl1->Size(); i++)
    {
  cl1->Get(i, chs1);
  if (chs1->GetLDP())
  {
      bool found=false;
      for (int j=0; ((j<cl2->Size()) && !found); j++)
      {
    cl2->Get(j, chs2);
    if (chs2->GetLDP())
    {
        if ((chs1->Inside(*chs2)))
        {
      found=true;
        }
    }
      }
      if (!found)
      {
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, true);
    return (0);
}

int
SpatialInside_lr( Word* args, Word& result, int message, 
                  Word& local, Supplier s )
{
    //to decide whether one line value is inside another
    result = qp->ResultStorage( s );
    CLine *cl;
    CRegion *cr;
    const CHalfSegment *chsl;

    cl=((CLine*)args[0].addr);
    cr=((CRegion*)args[1].addr);

    if(! cr->BoundingBox().Contains( cl->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cl->Size(); i++)
    {
  cl->Get(i, chsl);
  if (chsl->GetLDP())
  {
      if (!(cr->contain(*chsl)))
      {
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, true);
    return (0);
}

int
SpatialInside_rr( Word* args, Word& result, int message, 
                  Word& local, Supplier s )
{
    //for this algorithm, I need to reimplement
    // it by using Realizator/Derealmizator.
    result = qp->ResultStorage( s );
    CRegion *cr1, *cr2;
    const CHalfSegment *chs1, *chs2;

    cr1=((CRegion*)args[0].addr);
    cr2=((CRegion*)args[1].addr);

    if(! cr2->BoundingBox().Contains( cr1->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cr1->Size(); i++)
    {
  cr1->Get(i, chs1);

  if (chs1->GetLDP())
  {
      if ((!(cr2->contain(*chs1))))
      {
    ((CcBool *)result.addr)->Set( true, false );
    return (0);
      }
  }
    }

    bool existhole=false;
    bool allholeedgeinside=true;

    for (int j=0; j<cr2->Size(); j++)
    {
  cr2->Get(j, chs2);

  if ((chs2->GetLDP()) && (chs2->attr.cycleno>0) )
  //&& (chs2 is not masked by another face of region2)
  {
      if (!(cr1->holeedgecontain(*chs2)))
      {
    existhole=true;
    if ((!(cr1->contain(*chs2))))
    {
        allholeedgeinside=false;
    }
      }
  }
    }

    if ((existhole) && (allholeedgeinside))
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }


    ((CcBool *)result.addr)->Set( true, true);
    return (0);
}

/*
10.4.10 Value mapping functions of operator ~touches~

*/

int
touches_psps( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    ((CcBool *)result.addr)->
    Set( true, 
        ((Points*)args[0].addr)->Intersects( *((Points*)args[1].addr) ) );

    return (0);
}

int
touches_psl( Word* args, Word& result, int message, Word& local, Supplier s )
{
    //at least one of the points is endpoint of line
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[0].addr);
    CLine *cl=((CLine*)args[1].addr);

    if(! cl->BoundingBox().Intersects( ps->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    const Point *p;
    const CHalfSegment *chs;

    for (int i=0; i<ps->Size(); i++)
    {
  ps->Get(i, p);

  for (int j=0; j<cl->Size(); j++)
  {
      cl->Get(j, chs);
      if (chs->GetLDP())
      {
    if (((*p)==chs->GetLP())||((*p)==chs->GetRP()))
    {
        ((CcBool *)result.addr)->Set( true, true );
        return (0);
    }
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false );
    return (0);
}

int
touches_lps( Word* args, Word& result, int message, Word& local, Supplier s )
{
    //at least one of the points is endpoint of line
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[1].addr);
    CLine *cl=((CLine*)args[0].addr);

    if(! cl->BoundingBox().Intersects( ps->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    const Point *p;
    const CHalfSegment *chs;

    for (int i=0; i<ps->Size(); i++)
    {
  ps->Get(i, p);

  for (int j=0; j<cl->Size(); j++)
  {
      cl->Get(j, chs);
      if (chs->GetLDP())
      {
    if (((*p)==chs->GetLP())||((*p)==chs->GetRP()))
    {
        ((CcBool *)result.addr)->Set( true, true );
        return (0);
    }
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false );
    return (0);
}

int
touches_psr( Word* args, Word& result, int message, Word& local, Supplier s )
{
    //at least one of the points is on the edge of the region
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[0].addr);
    CRegion *cr=((CRegion*)args[1].addr);

    if(! cr->BoundingBox().Intersects( ps->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    const Point *p;
    const CHalfSegment *chs;

    for (int i=0; i<ps->Size(); i++)
    {
  ps->Get(i, p);

  for (int j=0; j<cr->Size(); j++)
  {
      cr->Get(j, chs);
      if (chs->Contains(*p))
      {
    ((CcBool *)result.addr)->Set( true, true);
    return (0);
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
touches_rps( Word* args, Word& result, int message, Word& local, Supplier s )
{
    //at least one of the points is on the edge of the region
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[1].addr);
    CRegion *cr=((CRegion*)args[0].addr);

    if(! cr->BoundingBox().Intersects( ps->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    const Point *p;
    const CHalfSegment *chs;

    for (int i=0; i<ps->Size(); i++)
    {
  ps->Get(i, p);

  for (int j=0; j<cr->Size(); j++)
  {
      cr->Get(j, chs);
      if (chs->Contains(*p))
      {
    ((CcBool *)result.addr)->Set( true, true);
    return (0);
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
touches_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{   //at least two segment intersect and the intersection is the endpoint
    result = qp->ResultStorage( s );
    CLine *cl1, *cl2;
    const CHalfSegment *chs1, *chs2;

    cl1=((CLine*)args[0].addr);
    cl2=((CLine*)args[1].addr);

    if(! cl1->BoundingBox().Intersects( cl2->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cl1->Size(); i++)
    {
  cl1->Get(i, chs1);
  if (chs1->GetLDP())
  {
      for (int j=0; j<cl2->Size(); j++)
      {
    cl2->Get(j, chs2);
    if (chs2->GetLDP())
    {
        if ((chs1->GetLP()==chs2->GetLP())||
            (chs1->GetLP()==chs2->GetRP())||
            (chs1->GetRP()==chs2->GetLP())||
            (chs1->GetRP()==chs2->GetRP()))
        {
      ((CcBool *)result.addr)->Set( true, true );
      return (0);
        }
    }
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
touches_lr( Word* args, Word& result, int message, Word& local, Supplier s )
{   //the endpoint of a line segment is on the edge of a region
    result = qp->ResultStorage( s );
    CLine *cl;
    CRegion *cr;

    const CHalfSegment *chsl, *chsr;

    cl=((CLine*)args[0].addr);
    cr=((CRegion*)args[1].addr);

    if(! cl->BoundingBox().Intersects( cr->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cl->Size(); i++)
    {
  cl->Get(i, chsl);
  if (chsl->GetLDP())
  {
      for (int j=0; j<cr->Size(); j++)
      {
    cr->Get(j, chsr);
    if (chsr->GetLDP())
    {
        if ((chsr->Contains(chsl->GetLP()))||(chsr->Contains(chsl->GetRP())))
        {
      ((CcBool *)result.addr)->Set( true, true );
      return (0);
        }
    }
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
touches_rl( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    CLine *cl;
    CRegion *cr;

    const CHalfSegment *chsl, *chsr;

    cl=((CLine*)args[1].addr);
    cr=((CRegion*)args[0].addr);

    if(! cl->BoundingBox().Intersects( cr->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cl->Size(); i++)
    {
  cl->Get(i, chsl);
  if (chsl->GetLDP())
  {
      for (int j=0; j<cr->Size(); j++)
      {
    cr->Get(j, chsr);
    if (chsr->GetLDP())
    {
        if ((chsr->Contains(chsl->GetLP()))||(chsr->Contains(chsl->GetRP())))
        {
      ((CcBool *)result.addr)->Set( true, true );
      return (0);
        }
    }
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
touches_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    CRegion *cr1, *cr2;
    const CHalfSegment *chs1, *chs2;

    cr1=((CRegion*)args[0].addr);
    cr2=((CRegion*)args[1].addr);

    if(! cr1->BoundingBox().Intersects( cr2->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cr1->Size(); i++)
    {
  cr1->Get(i, chs1);
  if (chs1->GetLDP())
  {
      for (int j=0; j<cr2->Size(); j++)
      {
    cr2->Get(j, chs2);
    if (chs2->GetLDP())
    {
        if (chs1->Intersects(*chs2))
        {
      ((CcBool *)result.addr)->Set( true, true );
      return (0);
        }
    }
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

/*
10.4.11 Value mapping functions of operator ~attached~

*/

int
attached_psps( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    ((CcBool *)result.addr)->Set( true, false );

    return (0);
}

int
attached_psl( Word* args, Word& result, int message, Word& local, Supplier s )
{
    //at least one of the points is endpoint of line
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[0].addr);
    CLine *cl=((CLine*)args[1].addr);

    if(! cl->BoundingBox().Intersects( ps->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    const Point *p;
    const CHalfSegment *chs;

    for (int i=0; i<ps->Size(); i++)
    {
  ps->Get(i, p);

  for (int j=0; j<cl->Size(); j++)
  {
      cl->Get(j, chs);
      if (chs->GetLDP())
      {
    if ((chs->Contains(*p))&&(chs->GetLP()!=*p)&&(chs->GetRP()!=*p))
    {
        ((CcBool *)result.addr)->Set( true, true );
        return (0);
    }
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false );
    return (0);
}

int
attached_lps( Word* args, Word& result, int message, Word& local, Supplier s )
{
    //at least one of the points is endpoint of line
    result = qp->ResultStorage( s );

    ((CcBool *)result.addr)->Set( true, false );

    return (0);
}

int
attached_psr( Word* args, Word& result, int message, Word& local, Supplier s )
{
    //at least one of the points is on the edge of the region
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[0].addr);
    CRegion *cr=((CRegion*)args[1].addr);

    if(! cr->BoundingBox().Intersects( ps->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    const Point *p;
    for (int i=0; i<ps->Size(); i++)
    {
  ps->Get(i, p);

  if (cr->innercontain(*p))
  {
      ((CcBool *)result.addr)->Set( true, true);
      return (0);
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
attached_rps( Word* args, Word& result, int message, Word& local, Supplier s )
{
    //at least one of the points is on the edge of the region
    result = qp->ResultStorage( s );

    ((CcBool *)result.addr)->Set( true, false );

    return (0);
}

int
attached_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{   //at least two segment intersect and the intersection is the endpoint
    result = qp->ResultStorage( s );
    CLine *cl1, *cl2;
    const CHalfSegment *chs1, *chs2;

    cl1=((CLine*)args[0].addr);
    cl2=((CLine*)args[1].addr);

    if(! cl1->BoundingBox().Intersects( cl2->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cl1->Size(); i++)
    {
  cl1->Get(i, chs1);
  if (chs1->GetLDP())
  {
      for (int j=0; j<cl2->Size(); j++)
      {
    cl2->Get(j, chs2);
    if (chs2->GetLDP())
    {
        if (((chs2->Contains(chs1->GetLP()))||
               chs2->Contains(chs2->GetRP())) &&
            (!(chs1->Contains(chs2->GetLP()))&&
            (!(chs1->Contains(chs2->GetRP())))))
        {
      ((CcBool *)result.addr)->Set( true, true);
      return (0);
        }
    }
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
attached_lr( Word* args, Word& result, int message, Word& local, Supplier s )
{   //the endpoint of a line segment is on the edge of a region
    result = qp->ResultStorage( s );
    CLine *cl;
    CRegion *cr;

    const CHalfSegment *chsl;

    cl=((CLine*)args[0].addr);
    cr=((CRegion*)args[1].addr);

    if(! cl->BoundingBox().Intersects( cr->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cl->Size(); i++)
    {
  cl->Get(i, chsl);
  if (chsl->GetLDP())
  {
      if ((cr->innercontain(chsl->GetLP()))||
          (cr->innercontain(chsl->GetRP())))
      {
    ((CcBool *)result.addr)->Set( true, true);
    return (0);
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
attached_rl( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    CLine *cl;
    CRegion *cr;

    const CHalfSegment *chsl, *chsr;

    cr=((CRegion*)args[0].addr);
    cl=((CLine*)args[1].addr);

    if(! cl->BoundingBox().Intersects( cr->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cr->Size(); i++)
    {
  cr->Get(i, chsr);

  if (chsr->GetLDP())
  {
      for (int j=0; j<cl->Size(); j++)
      {
    cl->Get(j, chsl);
    if (chsl->GetLDP())
    {   //chsr intersects (chsl-endpoints)
        if (chsr->innerIntersects(*chsl))
        {
      ((CcBool *)result.addr)->Set( true, true );
      return (0);
        }
    }
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
attached_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    CRegion *cr1, *cr2;
    const CHalfSegment *chs1, *chs2;

    cr1=((CRegion*)args[0].addr);
    cr2=((CRegion*)args[1].addr);

    if(! cr1->BoundingBox().Intersects( cr2->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cr1->Size(); i++)
    {
  cr1->Get(i, chs1);
  if (chs1->GetLDP())
  {   //chs1 must intersect with (regeion2-edges)
      if ((cr2->innercontain(chs1->GetLP()))||
          (cr2->innercontain(chs1->GetRP())))
      {
    ((CcBool *)result.addr)->Set( true, true);
    return (0);
      }

      for (int j=0; j<cr2->Size(); j++)
      {
    cr2->Get(j, chs2);
    if (chs2->GetLDP())
    {   //chsr intersects (chsl-endpoints)
        if (chs1->cross(*chs2))
        {
      ((CcBool *)result.addr)->Set( true, true );
      return (0);
        }
    }
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

/*
10.4.12 Value mapping functions of operator ~overlaps~

*/

int
overlaps_psps( Word* args, Word& result, int message, Word& local, Supplier s)
{
    result = qp->ResultStorage( s );

    ((CcBool *)result.addr)->Set( true, false );

    return (0);
}

int
overlaps_psl( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    ((CcBool *)result.addr)->Set( true, false );

    return (0);
}

int
overlaps_lps( Word* args, Word& result, int message, Word& local, Supplier s )
{
     result = qp->ResultStorage( s );

    ((CcBool *)result.addr)->Set( true, false );

    return (0);
}

int
overlaps_psr( Word* args, Word& result, int message, Word& local, Supplier s )
{
     result = qp->ResultStorage( s );

    ((CcBool *)result.addr)->Set( true, false );

    return (0);
}

int
overlaps_rps( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    ((CcBool *)result.addr)->Set( true, false );

    return (0);
}

int
overlaps_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    CLine *cl1, *cl2;
    const CHalfSegment *chs1, *chs2;

    cl1=((CLine*)args[0].addr);
    cl2=((CLine*)args[1].addr);

    if(! cl1->BoundingBox().Intersects( cl2->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cl1->Size(); i++)
    {
  cl1->Get(i, chs1);
  if (chs1->GetLDP())
  {
      for (int j=0; j<cl2->Size(); j++)
      {
    cl2->Get(j, chs2);
    if (chs2->GetLDP())
    {
        if (chs1->overlap(*chs2))
        {
      ((CcBool *)result.addr)->Set( true, true);
      return (0);
        }
    }
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
overlaps_lr( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    CLine *cl;
    CRegion *cr;

    const CHalfSegment *chsl;

    cl=((CLine*)args[0].addr);
    cr=((CRegion*)args[1].addr);

    if(! cl->BoundingBox().Intersects( cr->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cl->Size(); i++)
    {
  cl->Get(i, chsl);
  if (chsl->GetLDP())
  {
      if ((cr->innercontain(chsl->GetLP()))||
          (cr->innercontain(chsl->GetRP())))
      {
    ((CcBool *)result.addr)->Set( true, true);
    return (0);
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
overlaps_rl( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    CLine *cl;
    CRegion *cr;

    const CHalfSegment *chsl;

    cl=((CLine*)args[1].addr);
    cr=((CRegion*)args[0].addr);

    if(! cl->BoundingBox().Intersects( cr->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    for (int i=0; i<cl->Size(); i++)
    {
  cl->Get(i, chsl);
  if (chsl->GetLDP())
  {
      if ((cr->innercontain(chsl->GetLP()))||
          (cr->innercontain(chsl->GetRP())))
      {
    ((CcBool *)result.addr)->Set( true, true);
    return (0);
      }
  }
    }
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

int
overlaps_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    CRegion *cr1, *cr2;
    const CHalfSegment *chs1, *chs2;

    cr1=((CRegion*)args[0].addr);
    cr2=((CRegion*)args[1].addr);

    if(! cr1->BoundingBox().Intersects( cr2->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    //1. do normal check according to the edges
    for (int i=0; i<cr1->Size(); i++)
    {
  cr1->Get(i, chs1);
  if (chs1->GetLDP())
  {
      for (int j=0; j<cr2->Size(); j++)
      {
    cr2->Get(j, chs2);
    if (chs2->GetLDP())
    {
        if (chs1->cross(*chs2))
        {
      ((CcBool *)result.addr)->Set( true, true );
      return (0);
        }

        //This part is moved out of the loop to speed up the process
        //if ((cr2->innercontain(chs1.GetLP()))||
        //    (cr2->innercontain(chs1.GetRP()))||
        //    (cr1->innercontain(chs2.GetLP()))||
        //    (cr1->innercontain(chs2.GetRP())))
        //{
        //  ((CcBool *)result.addr)->Set( true, true );
        //  return (0);
        //   }


        if (*chs1==*chs2)
        {
      Point tryp;
      Coord midx, midy;

      midx=chs1->GetLP().GetX();
      midx += chs1->GetRP().GetX();
      midx /= (long)2 ;

      midy=chs1->GetLP().GetY();
      midy += chs1->GetRP().GetY();
      midy /= (long)2 ;

      midx += (double)0.01;
      tryp.Set(midx, midy);
      if ((cr1->innercontain(tryp))&&(cr2->innercontain(tryp)))
      {
          ((CcBool *)result.addr)->Set( true, true );
          return (0);
      }

      midx -=(double)0.02;
      tryp.Set(midx, midy);
      if ((cr1->innercontain(tryp))&&(cr2->innercontain(tryp)))
      {
          ((CcBool *)result.addr)->Set( true, true );
          return (0);
      }

      midx +=(double)0.01;
      midy +=(double)0.01;
      tryp.Set(midx, midy);
      if ((cr1->innercontain(tryp))&&(cr2->innercontain(tryp)))
      {
          ((CcBool *)result.addr)->Set( true, true );
          return (0);
      }

      midy -=(double)0.02;
      tryp.Set(midx, midy);
      if ((cr1->innercontain(tryp))&&(cr2->innercontain(tryp)))
      {
          ((CcBool *)result.addr)->Set( true, true );
          return (0);
      }
        }
    }
      }
  }
    }

    //2. check the cases of Tong-Xin-Yuan
    for (int i=0; i<cr1->Size(); i++)
    {
  cr1->Get(i, chs1);
  if (chs1->GetLDP())
  {
      if ((cr2->innercontain(chs1->GetLP()))||
    (cr2->innercontain(chs1->GetRP())))
      {
    ((CcBool *)result.addr)->Set( true, true );
    return (0);
      }
  }
    }

    for (int j=0; j<cr2->Size(); j++)
    {
  cr2->Get(j, chs2);
  if (chs2->GetLDP())
  {
      if ((cr1->innercontain(chs2->GetLP()))||
    (cr1->innercontain(chs2->GetRP())))
      {
    ((CcBool *)result.addr)->Set( true, true );
    return (0);
      }
  }
    }

    //3. else False
    ((CcBool *)result.addr)->Set( true, false);
    return (0);
}

/*
10.4.13 Value mapping functions of operator ~onborder~

*/

int
SpatialOnBorder_pl( Word* args, Word& result, int message, 
                    Word& local, Supplier s )
{
    //point is endpoint of line
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    CLine *cl=((CLine*)args[1].addr);

    if(! p->Inside( cl->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    const CHalfSegment *chs;

    if ( p->IsDefined() )
    {
  for (int i=0; i<cl->Size(); i++)
  {
      cl->Get(i, chs);
      if (chs->GetLDP())
      {
    if (((*p)==chs->GetLP())||((*p)==chs->GetRP()))
    {
        ((CcBool *)result.addr)->Set( true, true );
        return (0);
    }
      }
  }
  ((CcBool *)result.addr)->Set( true, false );
  return (0);
    }
    else
    {
  ((CcBool *)result.addr)->Set( true, false );
  return (0);
    }
}

int
SpatialOnBorder_pr( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
    //point is on the edge of region
     result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    CRegion *cr=((CRegion*)args[1].addr);

    if(! p->Inside( cr->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    const CHalfSegment *chs;

    if ( p->IsDefined() )
    {
  for (int i=0; i<cr->Size(); i++)
  {
      cr->Get(i, chs);
      if (chs->Contains(*p))
      {
    ((CcBool *)result.addr)->Set( true, true);
    return (0);
      }
  }
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }
    else
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }
}

/*
10.4.14 Value mapping functions of operator ~ininterior~

*/

int
SpatialInInterior_pl( Word* args, Word& result, int message, 
                      Word& local, Supplier s )
{
    //inside but not onborder
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    CLine *cl=((CLine*)args[1].addr);

    if(! p->Inside( cl->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    const CHalfSegment *chs;

    if ( p->IsDefined() )
    {
  for (int i=0; i<cl->Size(); i++)
  {
      cl->Get(i, chs);
      if (chs->GetLDP())
      {
    if (chs->Contains(*p))
    {
        if (((*p)!=chs->GetLP())&&((*p)!=chs->GetRP()))
        {
      ((CcBool *)result.addr)->Set( true, true );
      return (0);
        }
    }
      }
  }
  ((CcBool *)result.addr)->Set( true, false );
  return (0);
    }
    else
    {
  ((CcBool *)result.addr)->Set( true, false );
  return (0);
    }
}

int
SpatialInInterior_pr( Word* args, Word& result, int message, 
                      Word& local, Supplier s )
{
    //inside but not onborder
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    CRegion *cr=((CRegion*)args[1].addr);

    if(! p->Inside( cr->BoundingBox() ) )
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }

    const CHalfSegment *chs;

    if ( p->IsDefined() )
    {
  if (cr->contain(*p))
  {
      for (int i=0; i<cr->Size(); i++)
      {
    cr->Get(i, chs);
    if (chs->Contains(*p))
    {
        ((CcBool *)result.addr)->Set( true, false);
        return (0);
    }
      }
      ((CcBool *)result.addr)->Set( true, true);
      return (0);
  }
  else
  {
      ((CcBool *)result.addr)->Set( true, false);
      return (0);
  }
    }
    else
    {
  ((CcBool *)result.addr)->Set( true, false);
  return (0);
    }
}

/*
10.4.15 Value mapping functions of operator ~intersection~

*/

int
intersection_pp( Word* args, Word& result, int message, 
                 Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p1=((Point*)args[0].addr);
    Point *p2=((Point*)args[1].addr);

    if (( p1->IsDefined()) && ( p2->IsDefined()))
    {
  if (*p1==*p2)
  {
      *((Point *)result.addr)=*p1;
      return (0);
  }
  else
  {
      ((Point *)result.addr)->SetDefined( false );
      return (0);
  }
    }
    else
    {
  ((Point *)result.addr)->SetDefined( false );
  return (0);
    }
}

int
intersection_pps( Word* args, Word& result, int message, 
                  Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    Points *ps=((Points*)args[1].addr);

    if (( p->IsDefined()) && (!( ps->IsEmpty())))
    {
  if (ps->Contains(*p))
  {
      *((Point *)result.addr)=*p;
      return (0);
  }
  else
  {
      ((Point *)result.addr)->SetDefined( false );
      return (0);
  }
    }
    else
    {
  ((Point *)result.addr)->SetDefined( false );
  return (0);
    }
}

int
intersection_psp( Word* args, Word& result, int message, 
                  Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[1].addr);
    Points *ps=((Points*)args[0].addr);

    if (( p->IsDefined()) && (!( ps->IsEmpty())))
    {
  if (ps->Contains(*p))
  {
      *((Point *)result.addr)=*p;
      return (0);
  }
  else
  {
      ((Point *)result.addr)->SetDefined( false );
      return (0);
  }
    }
    else
    {
  ((Point *)result.addr)->SetDefined( false );
  return (0);
    }
}

int
intersection_pl( Word* args, Word& result, int message, 
                 Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    CLine *cl=((CLine*)args[1].addr);

    const CHalfSegment *chs;

    if (( p->IsDefined()) && (!( cl->IsEmpty())))
    {
  for (int i=0; i<cl->Size(); i++)
  {
      cl->Get(i, chs);
      if (chs->Contains(*p))
      {
    *((Point *)result.addr)=*p;
    return (0);
      }
  }
  ((Point *)result.addr)->SetDefined( false );
  return (0);
    }
    else
    {
  ((Point *)result.addr)->SetDefined( false );
  return (0);
    }
}

int
intersection_lp( Word* args, Word& result, int message, 
                 Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[1].addr);
    CLine *cl=((CLine*)args[0].addr);

    const CHalfSegment *chs;

    if (( p->IsDefined()) && (!( cl->IsEmpty())))
    {
  for (int i=0; i<cl->Size(); i++)
  {
      cl->Get(i, chs);
      if (chs->Contains(*p))
      {
    *((Point *)result.addr)=*p;
    return (0);
      }
  }
  ((Point *)result.addr)->SetDefined( false );
  return (0);
    }
    else
    {
  ((Point *)result.addr)->SetDefined( false );
  return (0);
    }
}

int
intersection_pr( Word* args, Word& result, int message, 
                 Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    CRegion *cr=((CRegion*)args[1].addr);

    if (( p->IsDefined()) && (!( cr->IsEmpty())))
    {
  if (cr->contain(*p))
  {
      *((Point *)result.addr)=*p;
      return (0);
  }
  else
  {
      ((Point *)result.addr)->SetDefined( false );
      return (0);
  }
    }
    else
    {
  ((Point *)result.addr)->SetDefined( false );
  return (0);
    }
}

int
intersection_rp( Word* args, Word& result, int message, 
                 Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[1].addr);
    CRegion *cr=((CRegion*)args[0].addr);

    if (( p->IsDefined()) && (!( cr->IsEmpty())))
    {
  if (cr->contain(*p))
  {
      *((Point *)result.addr)=*p;
      return (0);
  }
  else
  {
      ((Point *)result.addr)->SetDefined( false );
      return (0);
  }
    }
    else
    {
  ((Point *)result.addr)->SetDefined( false );
  return (0);
    }
}

int
intersection_psps( Word* args, Word& result, int message, 
                   Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps1=((Points*)args[0].addr);
    Points *ps2=((Points*)args[1].addr);
    const Point *p1, *p2;

    assert((ps1->IsOrdered())&&(ps2->IsOrdered()));

    if (!( ps1->IsEmpty()) && (!( ps2->IsEmpty())))
    {
  int i=0;
  int j=0;
  ((Points *)result.addr)->StartBulkLoad();
  while ((i<ps1->Size()) && (j<ps2->Size()))
  {
      ps1->Get(i, p1);
      ps2->Get(j, p2);
      while ((*p1<*p2)&&(i<ps1->Size()-1))
      {
    i++;
    ps1->Get(i, p1);
      }

      while ((*p2<*p1)&&(j<ps2->Size()-1))
      {
    j++;
    ps2->Get(j, p2);
      }
      if (*p1==*p2)
      {
    *((Points *)result.addr) += *p1;
      }
      i++;
      j++;
  }
  ((Points *)result.addr)->EndBulkLoad();
  return (0);
    }
    else  // one of the input is null
    {
  return (0);
    }
    return (0);
}

int
intersection_psl( Word* args, Word& result, int message, 
                  Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[0].addr);
    CLine *cl=((CLine*)args[1].addr);
    const Point *p;
    const CHalfSegment *chs;

    if (!( ps->IsEmpty()) && (!( cl->IsEmpty())))
    {
  ((Points *)result.addr)->StartBulkLoad();
  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, p);
      bool found=false;
      for (int j=0; ((j<cl->Size())&&(!found)); j++)
      {
    cl->Get(j, chs);
    if ((chs->GetLDP())&&(chs->Contains(*p)))
    {
        found=true;
        *((Points *)result.addr) += *p;
    }
      }
  }
   ((Points *)result.addr)->EndBulkLoad();
  return (0);
    }
    else  // one of the input is null
    {
  return (0);
    }
    return (0);
}

int
intersection_lps( Word* args, Word& result, int message, 
                  Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[1].addr);
    CLine *cl=((CLine*)args[0].addr);
    const Point *p;
    const CHalfSegment *chs;

    if (!( ps->IsEmpty()) && (!( cl->IsEmpty())))
    {
  ((Points *)result.addr)->StartBulkLoad();
  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, p);
      bool found=false;
      for (int j=0; ((j<cl->Size())&&(!found)); j++)
      {
    cl->Get(j, chs);
    if ((chs->GetLDP())&&(chs->Contains(p)))
    {
        found=true;
        *((Points *)result.addr) += *p;
    }
      }
  }
  ((Points *)result.addr)->EndBulkLoad();
  return (0);
    }
    else  // one of the input is null
    {
  return (0);
    }
    return (0);
}

int
intersection_psr( Word* args, Word& result, int message, 
                  Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[0].addr);
    CRegion *cr=((CRegion*)args[1].addr);
    const Point *p;

    if (!( ps->IsEmpty()) && (!( cr->IsEmpty())))
    {
  ((Points *)result.addr)->StartBulkLoad();
  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, p);
      if (cr->contain(*p))
      {
    *((Points *)result.addr) += *p;
      }
  }
  ((Points *)result.addr)->EndBulkLoad();
  return (0);
    }
    else  // one of the input is null
    {
  return (0);
    }
    return (0);
}

int
intersection_rps( Word* args, Word& result, int message, 
                  Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[1].addr);
    CRegion *cr=((CRegion*)args[0].addr);
    const Point *p;

    if (!( ps->IsEmpty()) && (!( cr->IsEmpty())))
    {
  ((Points *)result.addr)->StartBulkLoad();
  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, p);
      if (cr->contain(*p))
      {
    *((Points *)result.addr) += *p;
      }
  }
  ((Points *)result.addr)->EndBulkLoad();
  return (0);
    }
    else  // one of the input is null
    {
  return (0);
    }
    return (0);
}


int
intersection_ll( Word* args, Word& result, int message, 
                 Word& local, Supplier s )
{   //this function computes the intersection of two lines. 
    // However, since line's intersecion can
    //contain both points and lines, I will simply ignore 
    // the points and just keep line segments

    result = qp->ResultStorage( s );

    CLine *cl1=((CLine*)args[0].addr);
    CLine *cl2=((CLine*)args[1].addr);

    const CHalfSegment *chs1, *chs2;
    CHalfSegment chs;

    if (!( cl1->IsEmpty()) && (!( cl2->IsEmpty())))
    {
  ((CLine *)result.addr)->StartBulkLoad();

  for (int i=0; i<cl1->Size(); i++)
  {
      cl1->Get(i, chs1);

      if (chs1->GetLDP())
      {
    for (int j=0; j<cl2->Size(); j++)
    {
        cl2->Get(j, chs2);

        if (chs2->GetLDP())
        {
      if (chs1->overlapintersect(*chs2, chs))
      {
          *((CLine *)result.addr) += chs;
          chs.SetLDP(false);
          *((CLine *)result.addr) += chs;
      }
        }
    }
      }
  }
  //when the result is too big, the endbulkload has problem.
  ((CLine *)result.addr)->EndBulkLoad();
  return (0);
    }
    else  // one of the input is null
    {
  return (0);
    }
}

/*
10.4.16 Value mapping functions of operator ~minus~

*/

int
minus_pp( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p1=((Point*)args[0].addr);
    Point *p2=((Point*)args[1].addr);

    if ( p1->IsDefined())
    {
  if ( p2->IsDefined())
  {
      if (*p1==*p2)
      {
    ((Point *)result.addr)->SetDefined( false );
    return (0);
      }
      else
      {
    *((Point *)result.addr)=*p1;
    return (0);
      }
  }
  else
  {
      *((Point *)result.addr)=*p1;
      return (0);
  }
    }
    else
    {
  ((Point *)result.addr)->SetDefined( false );
  return (0);
    }
}

int
minus_psp( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[0].addr);
    Point *p=((Point*)args[1].addr);

    if (!( ps->IsEmpty()))
    {
  const Point *auxp;
  ((Points *)result.addr)->StartBulkLoad();
  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, auxp);
      if (*auxp!=*p)
      {
    *((Points *)result.addr) += *auxp;
      }
  }
  ((Points *)result.addr)->EndBulkLoad();
  return (0);
    }
    else
    {
  return (0);
    }
}

int
minus_lp( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    CLine *cl=((CLine *)args[0].addr);
    //Point *p=((Point*)args[1].addr);

    const CHalfSegment *chs;

    if (!( cl->IsEmpty()))
    {
  ((CLine *)result.addr)->StartBulkLoad();
  for (int i=0; i<cl->Size(); i++)
  {
      cl->Get(i, chs);
      *((CLine *)result.addr) += *chs;
  }
  ((CLine *)result.addr)->EndBulkLoad();
  return (0);
    }
    else
    {
  return (0);
    }
}

int
minus_rp( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    CRegion *cr=((CRegion *)args[0].addr);
    //Point *p=((Point*)args[1].addr);

    const CHalfSegment *chs;

    if (!( cr->IsEmpty()))
    {
  ((CRegion *)result.addr)->StartBulkLoad();
  for (int i=0; i<cr->Size(); i++)
  {
      cr->Get(i, chs);
      *((CRegion *)result.addr) += *chs;
  }
  ((CLine *)result.addr)->EndBulkLoad();
  return (0);
    }
    else
    {
  return (0);
    }
}

int
minus_psps( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps1=((Points*)args[0].addr);
    Points *ps2=((Points*)args[1].addr);

    if (!( ps1->IsEmpty()))
    {
  const Point *auxp;
  ((Points *)result.addr)->StartBulkLoad();
  for (int i=0; i<ps1->Size(); i++)
  {
      ps1->Get(i, auxp);
      if (!(ps2->Contains(*auxp)))
      {
    *((Points *)result.addr) += *auxp;
      }
  }
  ((Points *)result.addr)->EndBulkLoad();
  return (0);
    }
    else
    {
  return (0);
    }
}

int
minus_lps( Word* args, Word& result, int message, 
           Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    CLine *cl=((CLine *)args[0].addr);
    //Points *ps=((Points*)args[1].addr);

    const CHalfSegment *chs;

    if (!( cl->IsEmpty()))
    {
  ((CLine *)result.addr)->StartBulkLoad();
  for (int i=0; i<cl->Size(); i++)
  {
      cl->Get(i, chs);
      *((CLine *)result.addr) += *chs;
  }
  ((CLine *)result.addr)->EndBulkLoad();
  return (0);
    }
    else
    {
  return (0);
    }
}

int
minus_rps( Word* args, Word& result, int message, 
           Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    CRegion *cr=((CRegion *)args[0].addr);
    //Point *ps=((Point*)args[1].addr);

    const CHalfSegment *chs;

    if (!( cr->IsEmpty()))
    {
  ((CRegion *)result.addr)->StartBulkLoad();
  for (int i=0; i<cr->Size(); i++)
  {
      cr->Get(i, chs);
      *((CRegion *)result.addr) += *chs;
  }
  ((CLine *)result.addr)->EndBulkLoad();
  return (0);
    }
    else
    {
  return (0);
    }
}

/*
10.4.17 Value mapping functions of operator ~union~

*/

int
union_pps( Word* args, Word& result, int message, 
           Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    Points *ps=((Points*)args[1].addr);

    if ( ps->Contains(*p))
    {
  *((Points *)result.addr)=*ps;
  return (0);
    }
    else
    {
  Points TMP(ps->Size()+1);
  const Point *auxp;
  TMP.StartBulkLoad();
  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, auxp);
      TMP += *auxp;
  }
  TMP += *p;
  TMP.EndBulkLoad();
  (*((Points *)result.addr)) = TMP;
  return (0);
    }
}

int
union_psp( Word* args, Word& result, int message, 
           Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    Point *p=((Point*)args[1].addr);
    Points *ps=((Points*)args[0].addr);

    if ( ps->Contains(*p))
    {
  *((Points *)result.addr)=*ps;
  return (0);
    }
    else
    {
  Points TMP(1);
  const Point *auxp;
  TMP.StartBulkLoad();
  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, auxp);
      TMP += *auxp;
  }
  TMP += *p;
  TMP.EndBulkLoad();
  (*((Points*) result.addr)) = TMP;
  return (0);
    }
}

int
union_psps( Word* args, Word& result, int message, 
            Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps1=((Points*)args[1].addr);
    Points *ps2=((Points*)args[0].addr);

    const Point *auxp;

    assert((ps1->IsOrdered())&&(ps2->IsOrdered()));

    Points TMP(1);
    TMP.StartBulkLoad();

    for (int i=0; i<ps1->Size(); i++)
    {
  ps1->Get(i, auxp);
     TMP += *auxp;
    }

    for (int i=0; i<ps2->Size(); i++)
    {
       ps2->Get(i, auxp);
      TMP += *auxp;
    }

    TMP.EndBulkLoad();
    (*((Points *)result.addr))=TMP;
    return (0);
}

int
union_ll( Word* args, Word& result, int message, 
          Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    CLine *cl1=((CLine*)args[0].addr);
    CLine *cl2=((CLine*)args[1].addr);

    const CHalfSegment *chs1, *chs2;

    assert((cl1->IsOrdered())&&(cl2->IsOrdered()));

    ((CLine *)result.addr)->StartBulkLoad();

    for (int i=0; i<cl1->Size(); i++)
    {
  cl1->Get(i, chs1);
  *((CLine*)result.addr) += *chs1;
    }

    for (int i=0; i<cl2->Size(); i++)
    {
  cl2->Get(i, chs2);
  if (chs2->GetLDP())
  {
      bool appeared=false;

      for (int j=0; ((j<cl1->Size())&&(!appeared)); j++)
      {
    cl1->Get(j, chs1);
    if ((chs1->GetLDP())&&(chs2->Inside(*chs1)))
    {
        appeared=true;
    }
      }

      if (!appeared)
      {
    CHalfSegment aux( *chs2 );
    *((CLine*)result.addr) += aux;
    aux.SetLDP(false);
    *((CLine*)result.addr) += aux;
      }
  }
    }

    ((CLine *)result.addr)->EndBulkLoad();
    return (0);
}

/*
10.4.18 Value mapping functions of operator ~crossings~

*/

int
crossings_ll( Word* args, Word& result, int message, 
              Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    CLine *cl1=((CLine*)args[0].addr);
    CLine *cl2=((CLine*)args[1].addr);

    const CHalfSegment *chs1, *chs2;
    Point p;

    assert((cl1->IsOrdered())&&(cl2->IsOrdered()));

    ((Points*)result.addr)->StartBulkLoad();
    for (int i=0; i<cl1->Size(); i++)
    {
  cl1->Get(i, chs1);
  if (chs1->GetLDP())
  {
      for (int j=0; j<cl2->Size(); j++)
      {
    cl2->Get(j, chs2);
    if ((chs2->GetLDP())&&(chs1->crossings(*chs2, p)))
    {
        *((Points*)result.addr) += p;
    }
      }
  }
    }

    ((Points*)result.addr)->EndBulkLoad();
    return (0);
}

/*
10.4.19 Value mapping functions of operator ~single~

*/

int
single_ps( Word* args, Word& result, int message, 
           Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[0].addr);
    const Point *p;

    if (ps->Size()==1)
    {
  ps->Get(0, p);
  *((Point *)result.addr)=*p;
  return (0);
    }
    else
    {
      cout<<"the poins value doesn't have exactly 1 element.";
      return (0);
    }
}

/*
10.4.20 Value mapping functions of operator ~distance~

*/

int
distance_pp( Word* args, Word& result, int message, 
             Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p1=((Point*)args[0].addr);
    Point *p2=((Point*)args[1].addr);

    if (( p1->IsDefined())&&(p2->IsDefined()))
    {
  ((CcReal *)result.addr)->Set( true, p1->Distance(*p2));
  return (0);
    }
    else
    {
  ((CcReal *)result.addr)->Set( false, 0 );
  return (0);
    }
}

int
distance_pps( Word* args, Word& result, int message, 
              Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    Points *ps=((Points*)args[1].addr);
    const Point *auxp;

    float currDistance, minDistance=-1;

    if (( p->IsDefined())&&(!(ps->IsEmpty())))
    {
  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, auxp);
      currDistance=p->Distance(*auxp);

      if (minDistance==-1)
    minDistance=currDistance;
      else if (minDistance>currDistance)
    minDistance=currDistance;
  }

  ((CcReal *)result.addr)->Set( true,minDistance);
  return (0);
    }
    else
    {
  ((CcReal *)result.addr)->Set( false, 0 );
  return (0);
    }
}

int
distance_psp( Word* args, Word& result, int message, 
              Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[1].addr);
    Points *ps=((Points*)args[0].addr);
    const Point *auxp;

    float currDistance, minDistance=-1;

    if (( p->IsDefined())&&(!(ps->IsEmpty())))
    {
  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, auxp);
      currDistance=p->Distance(*auxp);

      if (minDistance==-1)
    minDistance=currDistance;
      else if (minDistance>currDistance)
    minDistance=currDistance;
  }

  ((CcReal *)result.addr)->Set( true,minDistance);
  return (0);
    }
    else
    {
  ((CcReal *)result.addr)->Set( false, 0 );
  return (0);
    }
}

int
distance_pl( Word* args, Word& result, int message, 
             Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    CLine *cl=((CLine*)args[1].addr);

    const CHalfSegment *chs;

    float currDistance, minDistance=-1;

    if (( p->IsDefined())&&(!(cl->IsEmpty())))
    {
  for (int i=0; i<cl->Size(); i++)
  {
      cl->Get(i, chs);
      if (chs->GetLDP())
      {
    currDistance=chs->Distance(*p);

    if (minDistance==-1)
        minDistance=currDistance;
    else if (minDistance>currDistance)
        minDistance=currDistance;
      }
  }

  ((CcReal *)result.addr)->Set( true, minDistance );
  return (0);
    }
    else
    {
  ((CcReal *)result.addr)->Set( false, 0 );
  return (0);
    }
}

int
distance_lp( Word* args, Word& result, int message, 
             Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[1].addr);
    CLine *cl=((CLine*)args[0].addr);

    const CHalfSegment *chs;

    float currDistance, minDistance=-1;

    if (( p->IsDefined())&&(!(cl->IsEmpty())))
    {
  for (int i=0; i<cl->Size(); i++)
  {
      cl->Get(i, chs);
      if (chs->GetLDP())
      {
    currDistance=chs->Distance(*p);

    if (minDistance==-1)
        minDistance=currDistance;
    else if (minDistance>currDistance)
        minDistance=currDistance;
      }
  }

  ((CcReal *)result.addr)->Set( true, minDistance );
  return (0);
    }
    else
    {
  ((CcReal *)result.addr)->Set( false, 0 );
  return (0);
    }
}

int
distance_pr( Word* args, Word& result, int message, 
             Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[0].addr);
    CRegion *cr=((CRegion*)args[1].addr);

    const CHalfSegment *chs;

    float currDistance, minDistance=-1;

    if (( p->IsDefined())&&(!(cr->IsEmpty())))
    {
  if (cr->contain(*p))
  {
      ((CcReal *)result.addr)->Set( true, 0 );
      return (0);
  }

  for (int i=0; i<cr->Size(); i++)
  {
      cr->Get(i, chs);
      if (chs->GetLDP())
      {
    currDistance=chs->Distance(*p);

    if (minDistance==-1)
        minDistance=currDistance;
    else if (minDistance>currDistance)
        minDistance=currDistance;
      }
  }

  ((CcReal *)result.addr)->Set( true, minDistance );
  return (0);
    }
    else
    {
  ((CcReal *)result.addr)->Set( false, 0 );
  return (0);
    }
}

int
distance_rp( Word* args, Word& result, int message, 
             Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p=((Point*)args[1].addr);
    CRegion *cr=((CRegion*)args[0].addr);

    const CHalfSegment *chs;

    float currDistance, minDistance=-1;

    if (( p->IsDefined())&&(!(cr->IsEmpty())))
    {
  if (cr->contain(*p))
  {
      ((CcReal *)result.addr)->Set( true, 0 );
      return (0);
  }

  for (int i=0; i<cr->Size(); i++)
  {
      cr->Get(i, chs);
      if (chs->GetLDP())
      {
    currDistance=chs->Distance(*p);

    if (minDistance==-1)
        minDistance=currDistance;
    else if (minDistance>currDistance)
        minDistance=currDistance;
      }
  }

  ((CcReal *)result.addr)->Set( true, minDistance );
  return (0);
    }
    else
    {
  ((CcReal *)result.addr)->Set( false, 0 );
  return (0);
    }
}

int
distance_psps( Word* args, Word& result, int message, 
               Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps1=((Points*)args[0].addr);
    Points *ps2=((Points*)args[1].addr);
    const Point *p1, *p2;

    float currDistance, minDistance=-1;

    if (!( ps1->IsEmpty())&&(!(ps2->IsEmpty())))
    {
  for (int i=0; i<ps1->Size(); i++)
  {
      ps1->Get(i, p1);

      for (int j=0; j<ps2->Size(); j++)
      {
    ps2->Get(j, p2);

    currDistance=p1->Distance(*p2);

    if (minDistance==-1)
        minDistance=currDistance;
    else if (minDistance>currDistance)
        minDistance=currDistance;
      }
  }

  ((CcReal *)result.addr)->Set( true,minDistance);
  return (0);
    }
    else
    {
  ((CcReal *)result.addr)->Set( false, 0 );
  return (0);
    }
}

int
distance_psl( Word* args, Word& result, int message, 
              Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[0].addr);
    CLine *cl=((CLine*)args[1].addr);

    const Point *p;
    const CHalfSegment *chs;

    float currDistance, minDistance=-1;

    if ((!(ps->IsEmpty()))&&(!(cl->IsEmpty())))
    {
  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, p);

      for (int j=0; j<cl->Size(); j++)
      {
    cl->Get(i, chs);
    if (chs->GetLDP())
    {
        currDistance=chs->Distance(*p);

        if (minDistance==-1)
      minDistance=currDistance;
        else if (minDistance>currDistance)
      minDistance=currDistance;
    }
      }
  }

  ((CcReal *)result.addr)->Set( true, minDistance );
  return (0);
    }
    else
    {
  ((CcReal *)result.addr)->Set( false, 0 );
  return (0);
    }
}

int
distance_lps( Word* args, Word& result, int message, 
              Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[1].addr);
    CLine *cl=((CLine*)args[0].addr);

    const Point *p;
    const CHalfSegment *chs;

    float currDistance, minDistance=-1;

    if ((!(ps->IsEmpty()))&&(!(cl->IsEmpty())))
    {
  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, p);

      for (int j=0; j<cl->Size(); j++)
      {
    cl->Get(i, chs);
    if (chs->GetLDP())
    {
        currDistance=chs->Distance(*p);

        if (minDistance==-1)
      minDistance=currDistance;
        else if (minDistance>currDistance)
      minDistance=currDistance;
    }
      }
  }

  ((CcReal *)result.addr)->Set( true, minDistance );
  return (0);
    }
    else
    {
  ((CcReal *)result.addr)->Set( false, 0 );
  return (0);
    }
}

int
distance_psr( Word* args, Word& result, int message, 
              Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[0].addr);
    CRegion *cr=((CRegion*)args[1].addr);

    const Point *p;
    const CHalfSegment *chs;

    float currDistance, minDistance=-1;

    if ((!(ps->IsEmpty()))&&(!(cr->IsEmpty())))
    {
  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, p);

      if (cr->contain(*p))
      {
    ((CcReal *)result.addr)->Set( true, 0 );
    return (0);
      }

      for (int j=0; j<cr->Size(); j++)
      {
    cr->Get(j, chs);
    if (chs->GetLDP())
    {
        currDistance=chs->Distance(*p);

        if (minDistance==-1)
      minDistance=currDistance;
        else if (minDistance>currDistance)
      minDistance=currDistance;
    }
      }
  }

  ((CcReal *)result.addr)->Set( true, minDistance );
  return (0);
    }
    else
    {
  ((CcReal *)result.addr)->Set( false, 0 );
  return (0);
    }
}

int
distance_rps( Word* args, Word& result, int message, 
              Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[1].addr);
    CRegion *cr=((CRegion*)args[0].addr);

    const Point *p;
    const CHalfSegment *chs;

    float currDistance, minDistance=-1;

    if ((!(ps->IsEmpty()))&&(!(cr->IsEmpty())))
    {
  for (int i=0; i<ps->Size(); i++)
  {
      ps->Get(i, p);

      if (cr->contain(*p))
      {
    ((CcReal *)result.addr)->Set( true, 0 );
    return (0);
      }

      for (int j=0; j<cr->Size(); j++)
      {
    cr->Get(j, chs);
    if (chs->GetLDP())
    {
        currDistance=chs->Distance(*p);

        if (minDistance==-1)
      minDistance=currDistance;
        else if (minDistance>currDistance)
      minDistance=currDistance;
    }
      }
  }

  ((CcReal *)result.addr)->Set( true, minDistance );
  return (0);
    }
    else
    {
  ((CcReal *)result.addr)->Set( false, 0 );
  return (0);
    }
}

int
distance_ll( Word* args, Word& result, int message, 
             Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    CLine *cl1=((CLine*)args[0].addr);
    CLine *cl2=((CLine*)args[1].addr);

    const CHalfSegment *chs1, *chs2;

    float currDistance, minDistance=-1;

    if ((!(cl1->IsEmpty()))&&(!(cl2->IsEmpty())))
    {
  for (int i=0; i<cl1->Size(); i++)
  {
      cl1->Get(i, chs1);
      if (chs1->GetLDP())
      {
    for (int j=0; j<cl2->Size(); j++)
    {
        cl2->Get(j, chs2);
        if (chs2->GetLDP())
        {
      currDistance=chs1->Distance(chs2->GetLP());
      if (minDistance==-1)
          minDistance=currDistance;
      else if (minDistance>currDistance)
          minDistance=currDistance;

      currDistance=chs1->Distance(chs2->GetRP());
      if (minDistance==-1)
          minDistance=currDistance;
      else if (minDistance>currDistance)
          minDistance=currDistance;

      currDistance=chs2->Distance(chs1->GetLP());
      if (minDistance==-1)
          minDistance=currDistance;
      else if (minDistance>currDistance)
          minDistance=currDistance;

      currDistance=chs2->Distance(chs1->GetRP());
      if (minDistance==-1)
          minDistance=currDistance;
      else if (minDistance>currDistance)
          minDistance=currDistance;
        }
    }
      }
  }

  ((CcReal *)result.addr)->Set( true, minDistance );
  return (0);
    }
    else
    {
  ((CcReal *)result.addr)->Set( false, 0 );
  return (0);
    }
}

/*
10.4.21 Value mapping functions of operator ~direction~

*/

int
direction_pp( Word* args, Word& result, int message,  
              Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Point *p1=((Point*)args[0].addr);
    Point *p2=((Point*)args[1].addr);

    double k;
    double direction; //from p1 to p2

    if (( p1->IsDefined())&&(p2->IsDefined())&&(*p1!=*p2))
    {
  Coord x1=p1->GetX();
  Coord y1=p1->GetY();
  Coord x2=p2->GetX();
  Coord y2=p2->GetY();

  if (x1==x2)
  {
      if (y2>y1)
      {
    ((CcReal *)result.addr)->Set( true, 90 );
      }
      else
      {
    ((CcReal *)result.addr)->Set( true, 270 );
      }
      return (0);
  }

  if (y1==y2)
  {
      if (x2>x1)
      {
    ((CcReal *)result.addr)->Set( true, 0 );
      }
      else
      {
    ((CcReal *)result.addr)->Set( true, 180 );
      }
      return (0);
  }
#ifdef RATIONAL_COORDINATES
  k=((y2.IsInteger()? y2.IntValue():y2.Value()) -
        (y1.IsInteger()? y1.IntValue():y1.Value())) /
       ((x2.IsInteger()? x2.IntValue():x2.Value()) -
        (x1.IsInteger()? x1.IntValue():x1.Value()));
#else
  k=(y2 - y1) / (x2 - x1);
#endif
  //here I should change the slope k to 0-PI
  direction=atan(k) * 180 /  M_PI;
  //cout<<k<<"==>"<<direction<<endl;

  int area;
  if ((x2>x1)&&(y2>y1))
  {
      area=1;
  }
  else if ((x2<x1)&&(y2>y1))
  {
      area=2;
      direction=180+direction;
  }
  else if ((x2<x1)&&(y2<y1))
  {
      area=3;
      direction=180+direction;
  }
  else if ((x2>x1)&&(y2<y1))
  {
      area=4;
      direction=360+direction;
  }

  ((CcReal *)result.addr)->Set( true, direction );
  return (0);
    }
    else
    {
  ((CcReal *)result.addr)->Set( false, 0 );
  return (0);
    }
}

/*
10.4.22 Value mapping functions of operator ~nocomponents~

*/

int
nocomponents_ps( Word* args, Word& result, int message, 
                 Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    Points *ps=((Points*)args[0].addr);

    ((CcInt *)result.addr)->Set( true, ps->Size());
    return (0);
}

static int
nocomponents_r( Word* args, Word& result, int message, 
                Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    CRegion *cr=((CRegion*)args[0].addr);
    const CHalfSegment *chs;
    int res=-1;

    for (int i=0; i<cr->Size(); i++)
    {
  cr->Get(i, chs);
  if (chs->GetLDP())
  {
      if (res<chs->attr.faceno) res=chs->attr.faceno;
  }
    }

    ((CcInt *)result.addr)->Set( true, res+1);
    return (0);
}

/*
10.4.22 Value mapping functions of operator ~no\_segments~

*/
static int
nosegments_l( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    CLine *cl=((CLine*)args[0].addr);

    ((CcInt *)result.addr)->Set( true, (int) (cl->Size()/2));
    return (0);
}

static int
nosegments_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    CRegion *cr=((CRegion*)args[0].addr);

    ((CcInt *)result.addr)->Set( true, (int) (cr->Size()/2));
    return (0);
}

/*
10.4.22 Value mapping functions of operator ~bbox~

*/
int
bbox_p( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    *((Rectangle<2>*)result.addr) = ((CPoint*)args[0].addr)->BoundingBox();
    return (0);
}

int
bbox_ps( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    *((Rectangle<2>*)result.addr) = ((CPoints*)args[0].addr)->BoundingBox();
    return (0);
}

int
bbox_l( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    *((Rectangle<2>*)result.addr) = ((CLine*)args[0].addr)->BoundingBox();
    return (0);
}

int
bbox_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    *((Rectangle<2>*)result.addr) = ((CRegion*)args[0].addr)->BoundingBox();
    return (0);
}


/*
10.4.23 Value mapping functions of operator ~size~

*/

int
size_l( Word* args, Word& result, int message, Word& local, Supplier s )
{
    //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    result = qp->ResultStorage( s );

    CLine *cl=((CLine*)args[0].addr);
    const CHalfSegment *chs;

    double res=0;

    for (int i=0; i<cl->Size(); i++)
    {
  cl->Get(i, chs);
  if (chs->GetLDP())
  {
      res += chs->GetLP().Distance(chs->GetRP());
  }
    }

    ((CcReal *)result.addr)->Set( true, res);
    return (0);
}

/*
10.4.24 Value mapping functions of operator ~touchpoints~

*/

int
touchpoints_lr( Word* args, Word& result, int message, 
                Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    CLine *cl;
    CRegion *cr;

    const CHalfSegment *chsl, *chsr;

    cl=((CLine*)args[0].addr);
    cr=((CRegion*)args[1].addr);

    ((Points *)result.addr)->StartBulkLoad();
    for (int i=0; i<cl->Size(); i++)
    {
  cl->Get(i, chsl);
  if (chsl->GetLDP())
  {
      for (int j=0; j<cr->Size(); j++)
      {
    cr->Get(j, chsr);
    if (chsr->GetLDP())
    {
        if (chsr->Contains(chsl->GetLP()))
        {
      *((Points *)result.addr) += chsl->GetLP();
        }

        if (chsr->Contains(chsl->GetRP()))
        {
      *((Points *)result.addr) += chsl->GetRP();
        }
    }
      }
  }
    }
    ((Points *)result.addr)->EndBulkLoad();
    return (0);
}

int
touchpoints_rl( Word* args, Word& result, int message, 
                Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    CLine *cl;
    CRegion *cr;

    const CHalfSegment *chsl, *chsr;

    cl=((CLine*)args[1].addr);
    cr=((CRegion*)args[0].addr);

    ((Points *)result.addr)->StartBulkLoad();
    for (int i=0; i<cl->Size(); i++)
    {
  cl->Get(i, chsl);
  if (chsl->GetLDP())
  {
      for (int j=0; j<cr->Size(); j++)
      {
    cr->Get(j, chsr);
    if (chsr->GetLDP())
    {
        if (chsr->Contains(chsl->GetLP()))
        {
      *((Points *)result.addr) += chsl->GetLP();
        }

        if (chsr->Contains(chsl->GetRP()))
        {
      *((Points *)result.addr) += chsl->GetRP();
        }
    }
      }
  }
    }
    ((Points *)result.addr)->EndBulkLoad();
    return (0);
}

int
touchpoints_rr( Word* args, Word& result, int message, 
                Word& local, Supplier s )
{   //need to improve this func- endpoints of edges 
    //should be considered specially.
    result = qp->ResultStorage( s );

    CRegion *cr1, *cr2;
    const CHalfSegment *chs1, *chs2;
    Point p;

    cr1=((CRegion*)args[0].addr);
    cr2=((CRegion*)args[1].addr);

    ((Points *)result.addr)->StartBulkLoad();
    for (int i=0; i<cr1->Size(); i++)
    {
  cr1->Get(i, chs1);
  if (chs1->GetLDP())
  {
      for (int j=0; j<cr2->Size(); j++)
      {
    cr2->Get(j, chs2);
    if (chs2->GetLDP())
    {
        if (chs1->spintersect(*chs2, p))
        {
      *((Points *)result.addr) += p;
        }
    }
      }
  }
    }
    ((Points *)result.addr)->EndBulkLoad();
    return (0);
}

/*
10.4.25 Value mapping functions of operator ~commomborder~

*/

int
commonborder_rr( Word* args, Word& result, int message, 
                 Word& local, Supplier s )
{
    result = qp->ResultStorage( s );
    int eee=0;
    CRegion *cr1, *cr2;
    const CHalfSegment *chs1, *chs2;
    CHalfSegment reschs;

    cr1=((CRegion*)args[0].addr);
    cr2=((CRegion*)args[1].addr);

    if(! cr1->BoundingBox().Intersects( cr2->BoundingBox() ) )
    {
  //cout<<"not intersect by MBR"<<endl;
  ((CLine *)result.addr)->Clear();
  return (0);
    }
    ((CLine *)result.addr)->Clear();
    ((CLine *)result.addr)->StartBulkLoad();
    for (int i=0; i<cr1->Size(); i++)
    {
  cr1->Get(i, chs1);
  if (chs1->GetLDP())
  {
      for (int j=0; j<cr2->Size(); j++)
      {
    cr2->Get(j, chs2);
    if (chs2->GetLDP())
    {
        if (chs1->overlapintersect(*chs2, reschs))
        {
      *((CLine *)result.addr) += reschs;
      reschs.SetLDP(false);
      *((CLine *)result.addr) += reschs;
      eee=eee+2;
        }
    }
      }
  }
    }
    ((CLine *)result.addr)->EndBulkLoad();
    return (0);
}

/*
110.4.25 Value mapping functions of operator ~commomborder~
Implementation with Spatial Scan

*/

static int
CommonBorderScan_rr( Word* args, Word& result, int message, 
                     Word& local, Supplier s )
{
  //void rrSelectFirst(CRegion& R1, CRegion& R2, object& obj, status& stat)
  int i = 0;
  result = qp->ResultStorage( s );

  const CHalfSegment *reschs;

  CRegion *cr1 = (CRegion*)args[0].addr,
          *cr2 = (CRegion*)args[1].addr,
          *pResult = (CRegion*)result.addr;

  if( !cr1->BoundingBox().Intersects( cr2->BoundingBox() ) )
  {
    ((CLine *)result.addr)->Clear();
    return (0);
  }

  pResult->Clear();
  pResult->StartBulkLoad();

  object obj;
  status stat;

  rrSelectFirst(*cr1, *cr2, obj, stat);
  if (obj==both)
  {
    cr1->GetHs(reschs);
    *pResult += *reschs;
    i++;
  }

  while (stat==endnone)
  {
    rrSelectNext(*cr1, *cr2, obj, stat);
    if (obj==both)
    {
      cr1->GetHs(reschs);
      *pResult += *reschs;
      i++;
    }
  }

  pResult->EndBulkLoad( false );
  return 0;
}


/*
10.4.26 Value mapping functions of operator ~translate~

*/

static int
Translate_p( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Point *p= (Point*)args[0].addr,
        *pResult = (Point*)result.addr;
  Supplier son = qp->GetSupplier( args[1].addr, 0 );
  Word t;
  qp->Request( son, t );

  double tx, ty;
  tx = ((CcReal *)t.addr)->GetRealval();
  son = qp->GetSupplier( args[1].addr, 1 );
  qp->Request( son, t );
  ty = ((CcReal *)t.addr)->GetRealval();

  if ( p->IsDefined())
  {
    *pResult = *p;
    pResult->Translate( tx, ty );
    return 0;
  }
  else
  {
    pResult->SetDefined( false );
    return 0;
  }
}

static int
Translate_ps( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Points *ps = (Points*)args[0].addr,
         *pResult = (Points*)result.addr;
  pResult->Clear();

  Supplier son = qp->GetSupplier( args[1].addr, 0 );
  Word t;
  qp->Request( son, t );

  double tx, ty;
  tx = ((CcReal *)t.addr)->GetRealval();
  son = qp->GetSupplier( args[1].addr, 1 );
  qp->Request( son, t );
  ty = ((CcReal *)t.addr)->GetRealval();

  if( !ps->IsEmpty() )
  {
    const Point *auxp;
    pResult->StartBulkLoad();

    for( int i = 0; i < ps->Size(); i++ )
    {
      ps->Get( i, auxp );
      Point p = *auxp;
      p.Translate( tx, ty );
      *pResult += p;
    }
    pResult->EndBulkLoad( false );
  }
  return 0;
}

static int
Translate_l( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  CLine *cl = (CLine *)args[0].addr,
        *pResult = (CLine *)result.addr;
  const CHalfSegment *chs;

  pResult->Clear();

  Supplier son = qp->GetSupplier( args[1].addr, 0 );
  Word t;
  qp->Request( son, t );

  double tx, ty;
  tx = ((CcReal *)t.addr)->GetRealval();
  son = qp->GetSupplier( args[1].addr, 1 );
  qp->Request( son, t );
  ty = ((CcReal *)t.addr)->GetRealval();

  if( !cl->IsEmpty() )
  {
    pResult->StartBulkLoad();

    for( int i = 0; i < cl->Size(); i++ )
    {
      cl->Get(i, chs);
      CHalfSegment aux( *chs );
      aux.Translate( tx, ty );
      *pResult += aux;
    }

    pResult->EndBulkLoad( false );
  }
  return 0;
}

static int
Translate_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  CRegion *cr = (CRegion *)args[0].addr,
          *pResult = (CRegion *)result.addr;
  const CHalfSegment *chs;

  pResult->Clear();

  Supplier son = qp->GetSupplier( args[1].addr, 0 );
  Word t;
  qp->Request( son, t );

  double tx, ty;
  tx = ((CcReal *)t.addr)->GetRealval();
  son = qp->GetSupplier( args[1].addr, 1 );
  qp->Request( son, t );
  ty = ((CcReal *)t.addr)->GetRealval();

  if( !cr->IsEmpty() )
  {
    pResult->StartBulkLoad();

    for( int i = 0; i < cr->Size(); i++ )
    {
      cr->Get(i, chs);
      CHalfSegment aux( *chs );
      aux.Translate( tx, ty );
      *pResult += aux;
    }

    pResult->EndBulkLoad( false );
  }
  return 0;
}

/*
10.4.27 Value Mapping functions of the Operator Scale 

*/
static int Scale_p( Word* args, Word& result, int message, 
                         Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  CPoint* p = (CPoint*) args[0].addr;
  CcReal*  factor = (CcReal*) args[1].addr;
  CPoint* res = (CPoint*) result.addr;
  res->Set(p->GetX(),p->GetY());
  double f = factor->GetRealval();
  res->Scale(f);
  return 0;
}

static int Scale_ps( Word* args, Word& result, int message, 
                         Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  CPoints* p = (CPoints*) args[0].addr;
  CcReal*  factor = (CcReal*) args[1].addr;
  CPoints* res = (CPoints*) result.addr;
  double f = factor->GetRealval();
  // make res empty if it is not already
  if(!res->IsEmpty()){
     Points P(0);
     (*res) = P;
  }  
  if(!p->IsEmpty()){
     res->StartBulkLoad();
     int size = p->Size();
     const CPoint *PTemp;
     for(int i=0;i<size;i++){
         p->Get(i,PTemp);
         CPoint aux( *PTemp );
         aux.Scale(f);
         (*res) += aux;
      }
      res->EndBulkLoad();
  }
  return 0;
}


static int Scale_l( Word* args, Word& result, int message, 
                         Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  CLine* L = (CLine*) args[0].addr;
  CcReal* factor = (CcReal*) args[1].addr;
  double f = factor->GetRealval();
  CLine* res = (CLine*) result.addr;
  // delete result if not empty
  if(!res->IsEmpty()){
     CLine Lempty(0);
     (*res) = Lempty;
  }  
  if(!L->IsEmpty()){
     res->StartBulkLoad();
     int size = L->Size();
     const CHalfSegment *hs;
     for(int i=0;i<size;i++){
       L->Get(i,hs);
       CHalfSegment aux( *hs );
       aux.Scale(f);
       (*res) += aux;
     }
     res->EndBulkLoad();
  }
  return 0;
}

static int Scale_r( Word* args, Word& result, int message, 
                         Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  CRegion* R = (CRegion*) args[0].addr;
  CcReal* factor = (CcReal*) args[1].addr;
  double f = factor->GetRealval();
  CRegion* res = (CRegion*) result.addr;
  // delete result if not empty
  if(!res->IsEmpty()){
     CRegion Rempty(0);
     (*res) = Rempty;
  }  
  if(!R->IsEmpty()){
     res->StartBulkLoad();
     int size = R->Size();
     const CHalfSegment *hs;
     for(int i=0;i<size;i++){
       R->Get(i,hs);
       CHalfSegment aux( *hs );
       aux.Scale(f);
       (*res) += aux;
     }
     res->EndBulkLoad();
  }
  return 0;
}

/*
10.4.28 Value Mappings for ~windowclipping~

*/

int
windowclippingin_l( Word* args, Word& result, int message, 
                    Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    CLine *l = ((CLine *)args[0].addr);
    CLine *clippedLine = (CLine*)result.addr;
    clippedLine->Clear();
    bool inside;
    Rectangle<2> *window = ((Rectangle<2>*)args[1].addr);

    l->WindowClippingIn(*window,*clippedLine,inside);

    return 0;

}
int
windowclippingin_r( Word* args, Word& result, int message, 
                    Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  CRegion *r = ((CRegion *)args[0].addr);
  CRegion *clippedRegion=(CRegion*)result.addr;
  clippedRegion->Clear();

  Rectangle<2> *window = ((Rectangle<2>*)args[1].addr);

  r->WindowClippingIn(*window,*clippedRegion);

  return 0;

}

int
windowclippingout_l( Word* args, Word& result, int message, 
                     Word& local, Supplier s )
{
    result = qp->ResultStorage( s );



    CLine *l = ((CLine *)args[0].addr);
    CLine *clippedLine = (CLine*)result.addr;
    clippedLine->Clear();
    bool outside;
    Rectangle<2> *window = ((Rectangle<2>*)args[1].addr);

    l->WindowClippingOut(*window,*clippedLine,outside);
    return 0;

}

int
windowclippingout_r( Word* args, Word& result, int message,
                     Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  CRegion *r = ((CRegion *)args[0].addr);
  CRegion *clippedRegion=(CRegion*)result.addr;
  clippedRegion->Clear();

  Rectangle<2> *window = ((Rectangle<2>*)args[1].addr);

  r->WindowClippingOut(*window,*clippedRegion);

  return 0;

}

/*
10.4.27 Value mapping functions of operator ~clip~

*/
static int
clip_l( Word* args, Word& result, int message, Word& local, Supplier s )
{
    result = qp->ResultStorage( s );

    CLine *l=((CLine*)args[0].addr);
    Rectangle<2> *r = ((Rectangle<2>*)args[1].addr);

    if ( l->IsDefined() && r->IsDefined() )
    {
        if( l->BoundingBox().Intersects( *r ) )
          l->Clip( *r, *((CLine *)result.addr) );
        return (0);
    }
    else
    {
        ((CLine *)result.addr)->SetDefined( false );
        return (0);
    }
}

/*
10.4.27 Value mapping functions of operator ~components~

*/
struct ComponentsLocalInfo
{
  vector<CRegion*> components;
  vector<CRegion*>::iterator iter;
};

static int
components_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  ComponentsLocalInfo *localInfo;

  switch( message )
  {
    case OPEN:
      Word wRegion;
      qp->Request(args[0].addr, wRegion);
      localInfo = new ComponentsLocalInfo();
      ((CRegion*)wRegion.addr)->Components( localInfo->components );
      localInfo->iter = localInfo->components.begin();
      local = SetWord( localInfo );
      return 0;

    case REQUEST:

      localInfo = (ComponentsLocalInfo*)local.addr;
      if( localInfo->iter == localInfo->components.end() )
        return CANCEL;
      result = SetWord( *localInfo->iter++ );
      return YIELD;

    case CLOSE:

      localInfo = (ComponentsLocalInfo*)local.addr;
      while( localInfo->iter != localInfo->components.end() )
        delete *localInfo->iter++;
      delete localInfo;
      return 0;
  }
  return 0;
}

/*
10.5 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of value
mapping functions for each operator. For nonoverloaded operators there is also such and array
defined, so it easier to make them overloaded.

10.5.1 Definition of value mapping vectors

*/
ValueMapping spatialisemptymap[] = { SpatialIsEmpty_p,
             SpatialIsEmpty_ps,
             SpatialIsEmpty_l,
             SpatialIsEmpty_r };

ValueMapping spatialequalmap[] =  {   SpatialEqual_pp,
             SpatialEqual_psps,
             SpatialEqual_ll,
             SpatialEqual_rr };

ValueMapping spatialnotequalmap[] = { SpatialNotEqual_pp,
              SpatialNotEqual_psps,
              SpatialNotEqual_ll,
              SpatialNotEqual_rr };

ValueMapping spatiallessmap[] =   { SpatialLess_pp };

ValueMapping spatiallessequalmap[] = { SpatialLessEqual_pp };

ValueMapping spatialgreatermap[] = { SpatialGreater_pp };

ValueMapping spatialgreaterequalmap[] = { SpatialGreaterEqual_pp };

ValueMapping spatialintersectsmap[] = { SpatialIntersects_psps,
                SpatialIntersects_psl,
                SpatialIntersects_lps,
                SpatialIntersects_psr,
                SpatialIntersects_rps,
                SpatialIntersects_ll,
                SpatialIntersects_lr,
                SpatialIntersects_rl,
                SpatialIntersects_rr };

ValueMapping spatialinsidemap[] =         { SpatialInside_pps,
                SpatialInside_pl,
                SpatialInside_pr,
                SpatialInside_psps,
                SpatialInside_psl,
                SpatialInside_psr,
                SpatialInside_ll,
                SpatialInside_lr,
                SpatialInside_rr };

ValueMapping spatialinsideoldmap[] =     { SpatialInside_pr_old};

ValueMapping spatialtouchesmap[] =    { touches_psps,
                touches_psl,
                touches_lps,
                touches_psr,
                touches_rps,
                touches_ll,
                touches_lr,
                touches_rl,
                touches_rr
              };

ValueMapping spatialattachedmap[] =    { attached_psps,
                attached_psl,
                attached_lps,
                attached_psr,
                attached_rps,
                attached_ll,
                attached_lr,
                attached_rl,
                attached_rr
              };

ValueMapping spatialoverlapsmap[] =    { overlaps_psps,
                overlaps_psl,
                overlaps_lps,
                overlaps_psr,
                overlaps_rps,
                overlaps_ll,
                overlaps_lr,
                overlaps_rl,
                overlaps_rr
              };

ValueMapping onbordermap[] =        { SpatialOnBorder_pl,
                SpatialOnBorder_pr };

ValueMapping ininteriormap[] =        { SpatialInInterior_pl,
                SpatialInInterior_pr };

ValueMapping intersectionmap[] =        { intersection_pp,
                intersection_pps,
                intersection_psp,
                intersection_pl,
                intersection_lp,
                intersection_pr,
                intersection_rp,
                intersection_psps,
                intersection_psl,
                intersection_lps,
                intersection_psr,
                intersection_rps,
                intersection_ll
              };

ValueMapping minusmap[] =         { minus_pp,
                minus_psp,
                minus_lp,
                minus_rp,
                minus_psps,
                minus_lps,
                minus_rps,
              };

ValueMapping unionmap[] =         { union_pps,
                union_psp,
                union_psps,
                union_ll,
              };

ValueMapping crossingsmap[] =         { crossings_ll
              };

ValueMapping singlemap[] =        { single_ps
              };

ValueMapping distancemap[] =        { distance_pp,
                distance_pps,
                distance_psp,
                distance_pl,
                distance_lp,
                distance_pr,
                distance_rp,
                distance_psps,
                distance_psl,
                distance_lps,
                distance_psr,
                distance_rps,
                distance_ll
              };

ValueMapping directionmap[] =         { direction_pp
              };

ValueMapping nocomponentsmap[] =   { nocomponents_ps,
               nocomponents_r,
              };

ValueMapping nosegmentsmap[] =      {     nosegments_l,
               nosegments_r
        };

ValueMapping bboxmap[] =      { bbox_p, bbox_ps, bbox_l, bbox_r
        };

ValueMapping insidepathlengthmap[] =      {     SpatialInside_pathlength_pr
        };

ValueMapping insidescannedmap[] =      {     SpatialInside_scanned_pr
        };

ValueMapping sizemap[] =        { size_l
              };

ValueMapping touchpointsmap[] =    { touchpoints_lr,
           touchpoints_rl,
           touchpoints_rr,
                       };

ValueMapping commonbordermap[] = { commonborder_rr
              };
ValueMapping commonborderscanmap[] = { CommonBorderScan_rr
              };

ValueMapping translatemap[] = { Translate_p,
                Translate_ps,
                Translate_l,
                Translate_r
              };

ValueMapping scalemap[] = { Scale_p,
                Scale_ps,
                Scale_l,
                Scale_r
              };


ValueMapping windowclippinginmap[] = {
                  windowclippingin_l,
                  windowclippingin_r
              };

ValueMapping windowclippingoutmap[] = {
                  windowclippingout_l,
                  windowclippingout_r
              };


ValueMapping spatialclipmap[] = { clip_l };


/*
10.5.2 Definition of specification strings

*/
const string SpatialSpecIsEmpty  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>point -> bool, points -> bool, line -> bool,"
       "region -> bool</text---> <text>isempty ( _ )</text--->"
       "<text>Returns whether the value is defined or not.</text--->"
       "<text>query isempty ( line1 )</text--->"
       ") )";

const string SpatialSpecEqual  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(point point) -> bool, (points points) -> bool, "
  "(line line) -> bool, (region region) -> bool</text--->"
  "<text>_ = _</text--->"
  "<text>Equal.</text--->"
  "<text>query point1 = point2</text--->"
  ") )";

const string SpatialSpecNotEqual  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(point point) -> bool</text--->"
  "<text>_ # _</text--->"
  "<text>Not equal.</text--->"
  "<text>query point1 # point2</text--->"
  ") )";

const string SpatialSpecLess  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(point point) -> bool</text--->"
  "<text>_ < _</text--->"
  "<text>Less than.</text--->"
  "<text>query point1 < point2</text--->"
  ") )";

const string SpatialSpecLessEqual  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(point point) -> bool</text--->"
  "<text>_ <= _</text--->"
  "<text>Equal or less than.</text--->"
  "<text>query point1 <= point2</text--->"
  ") )";

const string SpatialSpecGreater  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(point point) -> bool</text--->"
  "<text>_ > _</text--->"
  "<text>Greater than.</text--->"
  "<text>query point1 > point2</text--->"
  ") )";


const string SpatialSpecGreaterEqual  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(point point) -> bool</text--->"
  "<text>_ >= _</text--->"
  "<text>Equal or greater than.</text--->"
  "<text>query point1 >= point2</text--->"
  ") )";

const string SpatialSpecIntersects  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(point||points||line||region "
  "x point||points||line||region) -> bool </text--->"
  "<text>_ intersects _</text--->"
  "<text>Intersects.</text--->"
  "<text>query region1 intersects region2</text--->"
  ") )";

const string SpatialSpecInside  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(point||points||line||region "
  "x points||line||region) -> bool</text--->"
  "<text>_ inside _</text--->"
  "<text>Inside.</text--->"
  "<text>query point1 inside line1</text--->"
  ") )";

const string SpatialSpecInsideold  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(point x region) -> bool</text--->"
  "<text>_ insideold _</text--->"
  "<text>Insideold.</text--->"
  "<text>query point1 insideold region1</text--->"
  ") )";

const string SpatialSpecTouches  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(points||line||region x points||line||region) -> bool</text--->"
  "<text>_ touches _</text--->"
  "<text>two spatial objects touch each other.</text--->"
  "<text>query points touches line</text--->"
  ") )";

const string SpatialSpecAttached  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(points||line||region x points||line||region) -> bool</text--->"
  "<text>_ attached _</text--->"
  "<text>two spatial objects attach each other.</text--->"
  "<text>query line attached region</text--->"
  ") )";

const string SpatialSpecOverlaps  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(points||line||region x points||line||region) -> bool</text--->"
  "<text>_ overlaps _</text--->"
  "<text>two spatial objects overlap each other.</text--->"
  "<text>query line overlap region</text--->"
  ") )";

const string SpatialSpecOnBorder  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point x line||region) -> bool</text--->"
  "<text>_ onborder _</text--->"
  "<text>on endpoints or on border edges.</text--->"
  "<text>query point onborder line</text--->"
  ") )";

const string SpatialSpecInInterior  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point x line||region) -> bool</text--->"
  "<text>_ ininterior _</text--->"
  "<text>in interior of a line or region.</text--->"
  "<text>query point ininterior region</text--->"
  ") )";

const string SpatialSpecIntersection  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> (point||points||line||region x point||points||line||region)"
  "-> points||line||region</text--->"
  "<text>_intersection_</text--->"
  "<text>intersection of two sets.</text--->"
  "<text>query points intersection region</text--->"
  ") )";

const string SpatialSpecMinus  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point||points||line||region x point||points||line||region)"
  " -> point||points||line||region</text--->"
  "<text>_ minus _</text--->"
  "<text>minus of two sets.</text--->"
  "<text>query points minus point</text--->"
  ") )";

const string SpatialSpecUnion  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point||points||line x point||points||line)i"
  " -> points||line</text--->"
  "<text>_union_</text--->"
  "<text>union of two sets.</text--->"
  "<text>query points union point</text--->"
  ") )";

const string SpatialSpecCrossings  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(line x line) -> points</text--->"
  "<text>crossings(_,_)</text--->"
  "<text>crossing points of two lines.</text--->"
  "<text>query crossings(line1, line2)</text--->"
  ") )";

const string SpatialSpecSingle  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(points) -> point</text--->"
  "<text> single(_)</text--->"
  "<text>transform a single-element points value to point value.</text--->"
  "<text>query single(points)</text--->"
  ") )";

const string SpatialSpecDistance  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point||points||line x point||points||line) -> real</text--->"
  "<text>distance(_, _)</text--->"
  "<text>compute distance between two spatial objects.</text--->"
  "<text>query distance(point, line)</text--->"
  ") )";

const string SpatialSpecDirection  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point x point) -> real</text--->"
  "<text>direction(_, _)</text--->"
  "<text>compute the direction (0 - 360 degree)"
  " from one point to another point.</text--->"
  "<text>query direction(p1, p2)</text--->"
  ") )";

const string SpatialSpecNocomponents  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(points||line||region) -> int</text--->"
  "<text> no_components( _ )</text--->"
  "<text>return the number of components of a spatial object.</text--->"
  "<text>query no_components(region)</text--->"
  ") )";

const string SpatialSpecNoSegments  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(region) -> int</text--->"
  "<text> no_segments( _ )</text--->"
  "<text>return the number of half segments of a region.</text--->"
  "<text>query no_segments(region)</text--->"
  ") )";

const string SpatialSpecBbox  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point||points||line||region) -> rect</text--->"
  "<text> bbox( _ )</text--->"
  "<text>return the bounding box of a spatial type.</text--->"
  "<text>query bbox(region)</text--->"
  ") )";

const string SpatialSpecInsidepathlength  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point x region) -> int</text--->"
  "<text> point insidepathlength region</text--->"
  "<text>return the pathlength in inside_pr.</text--->"
  "<text>query point insidepathlength region</text--->"
  ") )";

const string SpatialSpecinsidescanned  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point x region) -> int</text--->"
  "<text> point insidescanned region</text--->"
  "<text>return the scanned in inside_pr.</text--->"
  "<text>query point insidescanned region</text--->"
  ") )";

const string SpatialSpecSize  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(line) -> real</text--->"
  "<text> size( _ )</text--->"
  "<text> return the size (length, area) of a spatial object.</text--->"
  "<text> query size(line)</text--->"
  ") )";

const string SpatialSpecTouchpoints  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(line||region x region) -> points</text--->"
  "<text> touchpoints(_, _) </text--->"
  "<text> return the touch points of a regioni"
  " and another region or line.</text--->"
  "<text> query touchpoints(line, region)</text--->"
  ") )";

const string SpatialSpecCommonborder  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(region x region) -> line</text--->"
  "<text> commonborder(_, _ )</text--->"
  "<text> return the common border of two regions.</text--->"
  "<text> query commonborder(region1, region2)</text--->"
  ") )";

const string SpatialSpecCommonborderscan  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(region x region) -> line</text--->"
  "<text> commonborderscan(_, _ )</text--->"
  "<text> return the common border of two"
  " regions through spatial scan.</text--->"
  "<text> query commonborderscan(region1, region2)</text--->"
  ") )";

const string SpatialSpecTranslate  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>for T in {point, points, line, region}: "
  "T x real x real -> T</text--->"
  "<text> _ translate[ _, _ ]</text--->"
  "<text> move the object by the indicated offsets in x and y.</text--->"
  "<text> query region1 translate[3.5, 15.1]</text--->"
  ") )";

const string SpatialSpecScale  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>for T in {point, points, line, region}: "
         "T x real -> T</text--->"
  "<text> _ scale [ _ ] </text--->"
  "<text> scales an object by the given factor.</text--->"
  "<text> query region1 scale[1000.0]</text--->"
  ") )";

const string SpatialSpecWindowClippingIn  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(line x rect) -> line, (region x rect) --> region</text--->"
  "<text> windowclippingin( _, _ ) </text--->"
  "<text> computes the part of the object that is inside the window.</text--->"
  "<text> query windowclippingin(line1, window)</text--->"
  ") )";

const string SpatialSpecWindowClippingOut  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(line x rect) -> line, (region x rect) --> region</text--->"
  "<text> windowclippingout( _, _ ) </text--->"
  "<text> computes the part of the object that is outside the window.</text--->"
  "<text> query windowclippingout(line1, rect)</text--->"
  ") )";


const string SpatialSpecClip  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text>line x bbox -> line</text--->"
        "<text>clip(_, _)</text--->"
        "<text>Returns the pieces of the line inside the rectangle.</text--->"
        "<text>query clip ( line1, box1 )</text--->"
        ") )";

const string SpatialSpecComponents  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text>region -> stream(region)</text--->"
        "<text>components(_)</text--->"
        "<text>Returns the components of a region.</text--->"
        "<text>query components(r1) count;</text--->"
        ") )";


/*
10.5.3 Definition of the operators

*/
Operator spatialisempty
  ( "isempty", SpatialSpecIsEmpty, 4, spatialisemptymap,
    SpatialSelectIsEmpty, SpatialTypeMapBool1 );

Operator spatialequal
  ( "=", SpatialSpecEqual, 4, spatialequalmap,
    SpatialSelectCompare, SpatialTypeMapBool );

Operator spatialnotequal
  ( "#", SpatialSpecNotEqual, 4, spatialnotequalmap,
    SpatialSelectCompare, SpatialTypeMapBool );

Operator spatialless
  ( "<", SpatialSpecLess, 1, spatiallessmap, 
    SimpleSelect, SpatialTypeMapBool );

Operator spatiallessequal
  ( "<=", SpatialSpecLessEqual, 1, spatiallessequalmap,
    SimpleSelect, SpatialTypeMapBool );

Operator spatialgreater
  ( ">", SpatialSpecGreater, 1, spatialgreatermap,
    SimpleSelect, SpatialTypeMapBool );

Operator spatialgreaterequal
  ( ">=", SpatialSpecGreaterEqual, 1, spatialgreaterequalmap,
    SimpleSelect, SpatialTypeMapBool );

Operator spatialintersects
  ( "intersects", SpatialSpecIntersects, 9, spatialintersectsmap,
    intersectSelect, GeoGeoMapBool);

Operator spatialinside
  ( "inside", SpatialSpecInside, 9, spatialinsidemap, 
    insideSelect, GeoGeoMapBool );

Operator spatialinsideold
  ( "insideold", SpatialSpecInsideold, 1, spatialinsideoldmap, 
    SimpleSelect, GeoGeoMapBool );

Operator spatialtouches
  ( "touches", SpatialSpecTouches, 9, spatialtouchesmap, 
    touches_attached_overlapsSelect, GeoGeoMapBool );

Operator spatialattached
  ( "attached", SpatialSpecAttached, 9, spatialattachedmap, 
    touches_attached_overlapsSelect, GeoGeoMapBool );

Operator spatialoverlaps
  ( "overlaps", SpatialSpecOverlaps, 9, spatialoverlapsmap, 
    touches_attached_overlapsSelect, GeoGeoMapBool );

Operator spatialonborder
  ( "onborder", SpatialSpecOnBorder, 2, onbordermap, 
    onBorder_inInteriorSelect, GeoGeoMapBool );

Operator spatialininterior
  ( "ininterior", SpatialSpecOnBorder, 2, ininteriormap, 
    onBorder_inInteriorSelect, GeoGeoMapBool );

Operator spatialintersection
  ( "intersection", SpatialSpecIntersection, 13, intersectionmap, 
    intersectionSelect, intersectionMap );

Operator spatialminus
  ( "minus", SpatialSpecMinus, 7, minusmap, 
    minusSelect, minusMap );

Operator spatialunion
  ( "union", SpatialSpecUnion, 4, unionmap, 
    unionSelect, unionMap );

Operator spatialcrossings
  ( "crossings", SpatialSpecCrossings, 1, crossingsmap, 
    crossingsSelect, crossingsMap );

Operator spatialsingle
  ( "single", SpatialSpecSingle, 1, singlemap, 
    singleSelect, singleMap );

Operator spatialdistance
  ( "distance", SpatialSpecDistance, 13, distancemap, 
    distanceSelect, distanceMap );

Operator spatialdirection
  ( "direction", SpatialSpecDirection, 1, directionmap, 
    directionSelect, directionMap );

Operator spatialnocomponents
  ( "no_components", SpatialSpecNocomponents, 3, nocomponentsmap, 
    nocomponentsSelect, nocomponentsMap );

Operator spatialnosegments
  ( "no_segments", SpatialSpecNoSegments, 2, nosegmentsmap, 
    nosegmentsSelect,  nosegmentsMap);

Operator spatialinsidepathlength
  ( "insidepathlength", SpatialSpecInsidepathlength, 1, insidepathlengthmap, 
    SimpleSelect,  insidepsMap);

Operator spatialinsidescanned
  ( "insidescanned", SpatialSpecinsidescanned, 1, insidescannedmap, 
    SimpleSelect,  insidepsMap);

Operator spatialbbox
  ( "bbox", SpatialSpecBbox, 4, bboxmap, 
    bboxSelect, bboxMap);

Operator spatialsize
  ( "size", SpatialSpecSize, 1, sizemap, 
    sizeSelect, sizeMap );

Operator spatialtouchpoints
  ( "touchpoints", SpatialSpecTouchpoints, 3, touchpointsmap, 
    touchpointsSelect, touchpointsMap );

Operator spatialcommonborder
  ( "commonborder", SpatialSpecCommonborder, 1, commonbordermap, 
    commonborderSelect, commonborderMap );

Operator spatialcommonborderscan
  ( "commonborderscan", SpatialSpecCommonborderscan, 1, commonborderscanmap, 
    commonborderSelect, commonborderMap );

Operator spatialtranslate
  ( "translate", SpatialSpecTranslate, 4, translatemap, 
    TranslateSelect, TranslateMap );

Operator spatialscale
  ( "scale", SpatialSpecScale, 4, scalemap, 
    ScaleSelect, ScaleMap );


Operator spatialwindowclippingin
  ( "windowclippingin", SpatialSpecWindowClippingIn, 4, windowclippinginmap, 
    windowclippingSelect, windowclippingMap );


Operator spatialwindowclippingout
  ( "windowclippingout", SpatialSpecWindowClippingOut, 4, windowclippingoutmap, 
    windowclippingSelect, windowclippingMap );

Operator spatialclip
        ( "clip", SpatialSpecClip, 1, spatialclipmap, 
          SimpleSelect, clipMap );

Operator spatialcomponents
        ( "components", SpatialSpecComponents,
          components_r, SimpleSelect, ComponentsMap );



/*
11 Creating the Algebra

*/

class SpatialAlgebra : public Algebra
{
 public:
  SpatialAlgebra() : Algebra()
  {
    AddTypeConstructor( &point );
    AddTypeConstructor( &points );
    AddTypeConstructor( &line );
    AddTypeConstructor( &region );

    point.AssociateKind("DATA");    //this means that point and rectangle
    points.AssociateKind("DATA");     //can be used in places where types
    line.AssociateKind("DATA");       //of kind DATA are expected, e.g. in
    region.AssociateKind("DATA"); //tuples.

    point.AssociateKind("SPATIAL2D"); //this means that point and rectangle
    points.AssociateKind("SPATIAL2D");//can be used in places where types
    line.AssociateKind("SPATIAL2D");  //of kind SPATIAL2D are expected, e.g. 
    region.AssociateKind("SPATIAL2D");//in tuples.

    AddOperator( &spatialisempty );
    AddOperator( &spatialequal );
    AddOperator( &spatialnotequal );
    //AddOperator( &spatialless );
    //AddOperator( &spatiallessequal );
    //AddOperator( &spatialgreater );
    //AddOperator( &spatialgreaterequal );
    AddOperator( &spatialintersects );
    AddOperator( &spatialinside );
    AddOperator( &spatialinsideold );
    AddOperator( &spatialtouches );
    AddOperator( &spatialattached );
    AddOperator( &spatialoverlaps );
    AddOperator( &spatialonborder );
    AddOperator( &spatialininterior );
    AddOperator( &spatialintersection );
    AddOperator( &spatialminus );
    AddOperator( &spatialunion );
    AddOperator( &spatialcrossings );
    AddOperator( &spatialtouchpoints);
    AddOperator( &spatialcommonborder);
    AddOperator( &spatialcommonborderscan);
    AddOperator( &spatialsingle );
    AddOperator( &spatialdistance );
    AddOperator( &spatialdirection );
    AddOperator( &spatialnocomponents );
    AddOperator( &spatialnosegments );
    AddOperator( &spatialsize );
    AddOperator( &spatialbbox);
    AddOperator( &spatialinsidepathlength );
    AddOperator( &spatialinsidescanned );
    AddOperator( &spatialtranslate );
    AddOperator( &spatialscale );
    AddOperator( &spatialwindowclippingin );
    AddOperator( &spatialwindowclippingout );
    AddOperator( &spatialclip );
    AddOperator( &spatialcomponents );
  }
  ~SpatialAlgebra() {};
};

SpatialAlgebra spatialAlgebra;

/*
12 Initialization

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



///////////////////




