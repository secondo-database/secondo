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

{\Large \bf Anhang B: Rectangle-Template }

[1] Header File of the Rectangle Algebra

October, 2003 Victor Teixeira de Almeida

October, 2004 Schoenhammer Herbert, (virtual) method BoundingBox() added

[TOC]

1 Overview

This header file essentially contains the definition of the struct ~Rectangle~.
This class corresponds to the memory representation for the type constructor
~rect~, ~rect3~ and ~rect4~ which are 2-dimensional, 3-dimensional or 4-dimensional
rectangles alligned with the axes of each dimension. A rectangle in such a way
can be represented by four, six or eight numbers (two for each dimension).


2 Defines and includes

*/
#ifndef __RECTANGLE_ALGEBRA_H__
#define __RECTANGLE_ALGEBRA_H__

using namespace std;

#include <cmath>
#include "stdarg.h"

#ifdef SECONDO_WIN32
#define Rectangle SecondoRectangle
#endif

#define MAX( a, b ) ((a) > (b) ? (a) : (b))
#define MIN( a, b ) ((a) < (b) ? (a) : (b))

template <unsigned dim>
class Rectangle;

template <unsigned dim>
class StandardSpatialAttribute : public StandardAttribute
{
  public:
    virtual const Rectangle<dim> BoundingBox() const = 0;
};

/*
3 Class ~Rectangle~

*/
template <unsigned dim>
class Rectangle: public StandardSpatialAttribute<dim>
{
  public:

    Rectangle() {}
/*
Do not use this constructor.

*/

    Rectangle( const bool defined, ... );
/*
The first constructor. First one can set if the rectangle is defined, and if it is,
the coordinates can be set.

*/

    Rectangle( const bool defined, const double *min, const double *max );
/*
The second constructor. First one can set if the rectangle is defined, and if it is,
the coordinates can be set.

*/

    Rectangle( const Rectangle<dim>& r );
/*
The copy constructor.

*/

    bool IsDefined() const;
/*
Checks if the rectangle is defined.

*/
    Rectangle<dim>& operator = ( const Rectangle<dim>& r );
/*
Redefinition of operator ~=~.

*/

    bool Contains( const Rectangle<dim>& r ) const;
/*
Checks if the rectangle contains the rectangle ~r~.

*/

    bool Intersects( const Rectangle<dim>& r ) const;
/*
Checks if the rectangle intersects with rectangle ~r~.

*/

    bool operator == ( const Rectangle<dim>& r ) const;
/*
Redefinition of operator ~==~.

*/

    bool operator != ( const Rectangle<dim>& r ) const;
/*
Redefinition of operator ~!=~.

*/

    double Area() const;
/*
Returns the area of a rectangle.

*/

    double Perimeter() const;
/*
Returns the perimeter of a rectangle.

*/

    const double MinD( int d ) const;

/*
Returns the min coord value for the given dimension ~dim~.

*/

    const double MaxD( int d ) const;
/*
Returns the max coord value for the given dimension ~dim~.

*/

    Rectangle<dim> Union( const Rectangle<dim>& r ) const;
/*
Returns the bounding box that contains both this and the rectangle ~r~.

*/

    Rectangle<dim>& Translate( const double t[dim] );
/*
Translates the rectangle given the translate vector ~t~.

*/

    Rectangle<dim> Intersection( const Rectangle<dim>& b ) const;
/*
Returns the intersection between this and the rectangle ~r~.

This next funnctions are necessary for a ~rectangle~ be
an attribute in the Relational Algebra.

*/

    const Rectangle<dim> BoundingBox() const;
/*
Returns the bounding box of the rectangle; this bounding Box is a clone
of the rectangle.

*/

    void SetDefined( bool Defined )
      {}

    size_t HashValue()
      { return 0; }

    void CopyFrom( StandardAttribute* right )
      { *this = *((Rectangle<dim>*)right); }

    int Compare( Attribute *arg )
      { return 0; }

    bool Adjacent( Attribute *arg )
      { return false; }

    int Sizeof() const
      { return sizeof( Rectangle<dim> ); }

    Rectangle* Clone()
      { return new Rectangle<dim>( *this ); }

    ostream& Print( ostream &os )
      { if( IsDefined() )
        {
          os << "Rectangle: ( ";
          for(unsigned int i=0; i < dim; i++)
            os<<min[i]<<" "<<max[i]<<" ";
          os<<")"<<endl;
          return os;
        }
        else
          return os << "undef";
      }

  private:

    bool Proper() const;
/*
Returns ~true~ if this is a "proper" rectangle.

*/
    bool defined;

    double min[dim];
/*
The left limits of the intervals in each dimension.

*/

    double max[dim];
/*
The left limits of the intervals in each dimension.

*/
};

/*
4 Implementation of the class ~Rectangle~

The first constructor. First one can set if the rectangle is defined, and if it is,
the coordinates can be set.

*/
template <unsigned dim>
Rectangle<dim>::Rectangle( const bool defined, ... ):
defined( defined )
{
  va_list ap;
  va_start( ap, defined );
  for( unsigned i = 0; i < dim; i++ )
  {
    double d1 = va_arg( ap, double ),
           d2 = va_arg( ap, double );
    min[i] = d1;
    max[i] = d2;
  }
  va_end( ap );

  assert( Proper() );
}

/*
The second constructor. First one can set if the rectangle is defined, and if it is,
the coordinates can be set.

*/
template <unsigned dim>
Rectangle<dim>::Rectangle( const bool defined, const double *min, const double *max ):
defined( defined )
{
  for( unsigned i = 0; i < dim; i++ )
  {
    this->min[i] = min[i];
    this->max[i] = max[i];
  }
  assert( Proper() );
}

/*
The copy constructor.

*/
template <unsigned dim>
Rectangle<dim>::Rectangle( const Rectangle<dim>& r ) :
defined( r.defined )
{
  for( unsigned i = 0; i < dim; i++ )
  {
    min[i] = r.min[i];
    max[i] = r.max[i];
  }
  assert( Proper() );
}

/*
Checks if the rectangle is defined.

*/
template <unsigned dim>
bool Rectangle<dim>::IsDefined() const
{
  return defined;
}

/*
Redefinition of operator ~=~.

*/
template <unsigned dim>
Rectangle<dim>& Rectangle<dim>::operator = ( const Rectangle<dim>& r )
{
  this->defined = r.defined;
  if( defined )
  {
    for( unsigned i = 0; i < dim; i++ )
    {
      this->min[i] = r.min[i];
      this->max[i] = r.max[i];
    }
  }
  assert( Proper() );
  return *this;
}

/*
Checks if the rectangle contains the rectangle ~r~.

*/
template <unsigned dim>
bool Rectangle<dim>::Contains( const Rectangle<dim>& r ) const
{
  assert( defined && r.defined );

  for( unsigned i = 0; i < dim; i++ )
    if( min[i] > r.min[i] || max[i] < r.max[i] )
      return false;

  return true;
}

/*
Checks if the rectangle intersects with rectangle ~r~.

*/
template <unsigned dim>
bool Rectangle<dim>::Intersects( const Rectangle<dim>& r ) const
{
  assert( defined && r.defined );

  for( unsigned i = 0; i < dim; i++ )
    if( max[i] < r.min[i] || r.max[i] < min[i] )
      return false;

  return true;
}

/*
Redefinition of operator ~==~.

*/
template <unsigned dim>
bool Rectangle<dim>::operator == ( const Rectangle<dim>& r ) const
{
  assert( IsDefined() && r.IsDefined() );

  for( unsigned i = 0; i < dim; i++ )
    if( min[i] != r.min[i] || max[i] != r.max[i] )
      return false;

  return true;
}

/*
Redefinition of operator ~!=~.

*/
template <unsigned dim>
bool Rectangle<dim>::operator != ( const Rectangle<dim>& r ) const
{
  return !(*this == r);
}

/*
Returns the area of a rectangle.

*/
template <unsigned dim>
double Rectangle<dim>::Area() const
{
  if( !IsDefined() )
    return 0.0;

  double area = 1.0;
  for( unsigned i = 0; i < dim; i++ )
    area *= max[i] - min[i];
  return area;
}

/*
Returns the perimeter of a rectangle.

*/
template <unsigned dim>
double Rectangle<dim>::Perimeter () const
{
  if( !IsDefined() )
    return 0.0;

  double perimeter = 0.0;
  for( unsigned i = 0; i < dim; i++ )
    perimeter += pow( 2, (double)dim-1 ) * ( max[i] - min[i] );
  return perimeter;
}

/*
Returns the min coord value for the given dimension ~d~.

*/
template <unsigned dim>
const double Rectangle<dim>::MinD( int d ) const
{
  assert( d >= 0 && (unsigned)d < dim );
  return min[d];
}

/*
Returns the max coord value for the given dimension ~d~.

*/
template <unsigned dim>
const double Rectangle<dim>::MaxD( int d ) const
{
  assert( d >= 0 && (unsigned)d < dim );
  return max[d];
}

/*
Returns the bounding box that contains both this and the rectangle ~r~.

*/
template <unsigned dim>
Rectangle<dim> Rectangle<dim>::Union( const Rectangle<dim>& r ) const
{
  if( !defined || !r.defined )
    return Rectangle<dim>( false );

  double auxmin[dim], auxmax[dim];
  for( unsigned i = 0; i < dim; i++ )
  {
    auxmin[i] = MIN( min[i], r.min[i] );
    auxmax[i] = MAX( max[i], r.max[i] );
  }
  return Rectangle<dim>( true, auxmin, auxmax );
}

/*
Translates the rectangle given the translate vector ~t~.

*/
template <unsigned dim>
Rectangle<dim>& Rectangle<dim>::Translate( const double t[dim] )
{
  if( defined )
  {
    for( unsigned i = 0; i < dim; i++ )
    {
      min[i] += t[i];
      max[i] += t[i];
    }
  }
  return *this;
}

/*
Returns the bounding box of the rectangle; this bounding Box is a clone
of the rectangle.

*/
template <unsigned dim>
const Rectangle<dim> Rectangle<dim>::BoundingBox() const
{
  if( defined )
    return Rectangle<dim>( *this );
  else
    return Rectangle<dim>( false );
}

/*
Returns the intersection between this and the rectangle ~r~.

*/
template <unsigned dim>
Rectangle<dim> Rectangle<dim>::Intersection( const Rectangle<dim>& r ) const
{
  if( !defined || !r.defined || !Intersects( r ) )
    return Rectangle<dim>( false ); 

  double auxmin[dim], auxmax[dim];
  for( unsigned i = 0; i < dim; i++ )
  {
    auxmin[i] = MAX( min[i], r.min[i] );
    auxmax[i] = MIN( max[i], r.max[i] );
  }
  return Rectangle<dim>( true, auxmin, auxmax );
}

/*
Returns ~true~ if this is a "proper" rectangle, i.e. it does not
represent an empty set.

*/
template <unsigned dim>
bool Rectangle<dim>::Proper() const
{
  if( defined )
  {
    for( unsigned i = 0; i < dim; i++ )
      if( min[i] > max[i] )
           return false;
  }
  return true;
}

/*
5 Template functions for the type constructors

5.1 ~Out~-function

*/
template <unsigned dim>
ListExpr OutRectangle( ListExpr typeInfo, Word value )
{
  Rectangle<dim> *r = (Rectangle<dim>*)value.addr;
  if( r->IsDefined() )
  {
    ListExpr result = nl->OneElemList( nl->RealAtom( r->MinD(0) ) ),
             last = result;
    last = nl->Append( last, nl->RealAtom( r->MaxD(0) ) );

    for( unsigned i = 1; i < dim; i++ )
    {
      last = nl->Append( last, nl->RealAtom( r->MinD(i) ) );
      last = nl->Append( last, nl->RealAtom( r->MaxD(i) ) );
    }
    return result;
  }
  else
  {
    return (nl->SymbolAtom("undef"));
  }
}

/*
5.2 ~In~-function

*/
template <unsigned dim>
Word InRectangle( const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr& errorInfo, bool& correct )
{
  ListExpr l = instance;


  if( nl->ListLength( instance ) == 2 * dim )
  {
    correct = true;
    double min[dim], max[dim];

    for( unsigned i = 0; i < dim; i++ )
    {
      if( nl->IsAtom( nl->First( l ) ) && nl->AtomType( nl->First( l ) ) == RealType )
      {
        min[i] = nl->RealValue( nl->First( l ) );
        l = nl->Rest( l );
      }
      else
      {
        correct = false;
        break;
      }

      if( nl->IsAtom( nl->First( l ) ) && nl->AtomType( nl->First( l ) ) == RealType )
      {
        max[i] = nl->RealValue( nl->First( l ) );
        l = nl->Rest( l );
      }
      else
      {
        correct = false;
        break;
      }
    }
    if( correct )
    {
      Rectangle<dim> *r = new Rectangle<dim>( true, min, max );
      return SetWord( r );
    }
  }
  else if ( nl->IsAtom( instance ) &&
            nl->AtomType( instance ) == SymbolType &&
            nl->SymbolValue( instance ) == "undef" )
  {
    correct = true;
    return SetWord( new Rectangle<3>( false ) );
  }
  correct = false;
  return SetWord( Address( 0 ) );
}

/*
5.1 ~Create~-function

*/
template <unsigned dim>
Word CreateRectangle( const ListExpr typeInfo )
{
  return SetWord( new Rectangle<dim>( false ) );
}

/*
5.2 ~Delete~-function

*/
template <unsigned dim>
void DeleteRectangle( const ListExpr typeInfo, Word& w )
{
  delete (Rectangle<dim> *)w.addr;
  w.addr = 0;
}

/*
5.3 ~Close~-function

*/
template <unsigned dim>
void CloseRectangle( const ListExpr typeInfo, Word& w )
{
  delete (Rectangle<dim> *)w.addr;
  w.addr = 0;
}

/*
5.4 ~Clone~-function

*/
template <unsigned dim>
Word CloneRectangle( const ListExpr typeInfo, const Word& w )
{
  Rectangle<dim> *r = new Rectangle<dim>( *((Rectangle<dim> *)w.addr) );
  return SetWord( r );
}

/*
5.5 ~Cast~-function

*/
template <unsigned dim>
void* CastRectangle(void* addr)
{
  return new (addr) Rectangle<dim>;
}

/*
3.11 ~SizeOf~-function

*/
template <unsigned dim>
int SizeOfRectangle()
{
  return sizeof(Rectangle<dim>);
}




Word
InRectangle( const ListExpr typeInfo, const ListExpr instance,
             const int errorPos, ListExpr& errorInfo, bool& correct );

#endif

