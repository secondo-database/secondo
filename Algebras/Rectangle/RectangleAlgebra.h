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
~rect~, ~rect3~ and ~rect4~ which are 2-dimensional, 3-dimensional, 4-dimensional
or 8-dimensional rectangles alligned with the axes of each dimension. A rectangle 
in such a way can be represented by four, six or eight numbers (two for each dimension).


2 Defines and includes

*/
#ifndef __RECTANGLE_ALGEBRA_H__
#define __RECTANGLE_ALGEBRA_H__

using namespace std;

#include <cmath>
#include "stdarg.h"

#include <stdio.h>

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

    inline Rectangle() {}
/*
Do not use this constructor.

*/

    inline Rectangle( const bool defined, ... );
/*
The first constructor. First one can set if the rectangle is defined, and if it is,
the coordinates can be set.

*/

    inline Rectangle( const bool defined, 
                      const double *min, const double *max );
/*
The second constructor. First one can set if the rectangle is defined, and if it is,
the coordinates can be set.

*/

    inline Rectangle( const Rectangle<dim>& r );
/*
The copy constructor.

*/

    inline void Set(const bool defined, const double *min, const double *max);

    inline bool IsDefined() const;
/*
Checks if the rectangle is defined.

*/
    inline Rectangle<dim>& operator = ( const Rectangle<dim>& r );
/*
Redefinition of operator ~=~.

*/

    inline bool Contains( const Rectangle<dim>& r ) const;
/*
Checks if the rectangle contains the rectangle ~r~.

*/

    inline bool Intersects( const Rectangle<dim>& r ) const;
/*
Checks if the rectangle intersects with rectangle ~r~.

*/

    inline bool operator == ( const Rectangle<dim>& r ) const;
/*
Redefinition of operator ~==~.

*/

    inline bool operator != ( const Rectangle<dim>& r ) const;
/*
Redefinition of operator ~!=~.

*/

    inline double Area() const;
/*
Returns the area of a rectangle.

*/

    inline double Perimeter() const;
/*
Returns the perimeter of a rectangle.

*/

    inline const double MinD( int d ) const;

/*
Returns the min coord value for the given dimension ~dim~.

*/

    inline const double MaxD( int d ) const;
/*
Returns the max coord value for the given dimension ~dim~.

*/

    inline Rectangle<dim> Union( const Rectangle<dim>& r ) const;
/*
Returns the bounding box that contains both this and the rectangle ~r~.

*/

    inline double Distance(const Rectangle<dim>& r) const;
/*
Returns the Euclidean distance between two rectangles

*/
    inline Rectangle<dim>& Translate( const double t[dim] );
/*
Translates the rectangle given the translate vector ~t~.

*/

    inline Rectangle<dim> Intersection( const Rectangle<dim>& b ) const;
/*
Returns the intersection between this and the rectangle ~r~.

This next functions are necessary for a ~rectangle~ be
an attribute in the Relational Algebra.

*/

    inline const Rectangle<dim> BoundingBox() const;
/*
Returns the bounding box of the rectangle; this bounding Box is a clone
of the rectangle.

*/

    inline void SetDefined( bool Defined )
      {
        defined = Defined;
      }

    inline size_t Sizeof() const
    {
      return sizeof( *this );
    }

    inline size_t HashValue() const
    { 
      size_t h = 0;
      for( unsigned i = 0; i < dim; i++ )
        h += size_t(4 * min[dim] + max[dim]);
      return h;
    }

    inline void CopyFrom( const StandardAttribute* right )
      { *this = *(const Rectangle<dim>*)right; }



    inline int Compare( const Attribute *arg ) const
    {

      unsigned thispos, rpos;
      unsigned thismin[dim], rmin[dim];

      const Rectangle<dim>* r = (const Rectangle<dim>*)arg;
      if(!defined && !r->defined)
        return 0;
      if(!defined)
        return -1;
      if(!r->defined)
        return 1;

      //order on rectangles is z-order (bit interleaving)

      // treat positive/negative quadrants


      thispos = 0;
      rpos = 0;

      for (unsigned j = 0; j < dim; j++)
      {
        thispos <<= 1;
        thispos |= (min[j] >= 0);
        rpos <<= 1;
        rpos |= (r->min[j] >= 0);
      }
      if (thispos < rpos)
        return -1;
      if (thispos > rpos)
        return 1;


      // now treat z-order based on positive integer coordinates

      for (unsigned j = 0; j < dim; j++)
      {
        thismin[j] = (unsigned) fabs(min[j]);  
        rmin[j] = (unsigned) fabs(r->min[j]);
      }

      for (int j = 31; j >= 0; j--)
      {
        thispos = 0;
        rpos = 0;

        for (unsigned k = 0; k < dim; k++)
        {
          thispos <<= 1;
          thispos |= ((thismin[k] >> j) & 1);
          rpos <<= 1;
          rpos |= ((rmin[k] >> j) & 1);
        }

        if (thispos < rpos)
          return -1;
        if (thispos > rpos)
          return 1;
      }

      // if no conclusion on z-order (based on integer coordinates) can be
      // reached, we fall back to the standard comparison
 
      for( unsigned i = 0; i < dim; i++ )
      {
        if( this->min[i] < r->min[i] )
          return -1;
        if( this->min[i] > r->min[i] )
          return 1;
      }
      for( unsigned i = 0; i < dim; i++ )
      {
        if( this->max[i] < r->max[i] )
          return -1;
        if( this->max[i] > r->max[i] )
          return 1;
      }
      return 0;  
    }



    inline bool Adjacent( const Attribute *arg ) const
      { return false; }

    inline Rectangle* Clone() const
      { return new Rectangle<dim>( *this ); }

    inline ostream& Print( ostream &os ) const
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

    inline bool Proper() const;
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
inline Rectangle<dim>::Rectangle( const bool defined, ... ):
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
inline Rectangle<dim>::Rectangle( const bool defined, const double *min, 
                                  const double *max ):
defined( defined )
{
   Set(defined,min,max);
}

/*
The copy constructor.

*/
template <unsigned dim>
inline Rectangle<dim>::Rectangle( const Rectangle<dim>& r ) :
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
Sets the values of this rectangle.

*/
template <unsigned dim>
inline void Rectangle<dim>::Set(const bool defined, const double *min, 
                           const double *max){
  this->defined = defined;
  for( unsigned i = 0; i < dim; i++ )
  {
    this->min[i] = min[i];
    this->max[i] = max[i];
  }
  assert( Proper() );
}





/*
Checks if the rectangle is defined.

*/
template <unsigned dim>
inline bool Rectangle<dim>::IsDefined() const
{
  return defined;
}

/*
Redefinition of operator ~=~.

*/
template <unsigned dim>
inline Rectangle<dim>& Rectangle<dim>::operator = ( const Rectangle<dim>& r )
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
inline bool Rectangle<dim>::Contains( const Rectangle<dim>& r ) const
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
inline bool Rectangle<dim>::Intersects( const Rectangle<dim>& r ) const
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
inline bool Rectangle<dim>::operator == ( const Rectangle<dim>& r ) const
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
inline bool Rectangle<dim>::operator != ( const Rectangle<dim>& r ) const
{
  return !(*this == r);
}

/*
Returns the area of a rectangle.

*/
template <unsigned dim>
inline double Rectangle<dim>::Area() const
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
inline double Rectangle<dim>::Perimeter () const
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
inline const double Rectangle<dim>::MinD( int d ) const
{
  assert( d >= 0 && (unsigned)d < dim );
  return min[d];
}

/*
Returns the max coord value for the given dimension ~d~.

*/
template <unsigned dim>
inline const double Rectangle<dim>::MaxD( int d ) const
{
  assert( d >= 0 && (unsigned)d < dim );
  return max[d];
}

/*
Returns the bounding box that contains both this and the rectangle ~r~.

*/
template <unsigned dim>
inline Rectangle<dim> Rectangle<dim>::Union( const Rectangle<dim>& r ) const
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
inline Rectangle<dim>& Rectangle<dim>::Translate( const double t[dim] )
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
inline const Rectangle<dim> Rectangle<dim>::BoundingBox() const
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
inline Rectangle<dim> 
      Rectangle<dim>::Intersection( const Rectangle<dim>& r ) const
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
inline bool Rectangle<dim>::Proper() const
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
Distance: returns the Euclidean distance between two rectangles

*/

template <unsigned dim>
inline double Rectangle<dim>::Distance(const Rectangle<dim>& r) const
{
  assert( defined && r.defined );
  double sum = 0;

  for( unsigned i = 0; i < dim; i++ )
  {
    if(r.min[i] >= max[i])
      sum += pow( (max[i] - r.min[i]), 2 );
    else if(r.max[i] <= min[i])
      sum += pow( (min[i] - r.max[i]), 2 );
    else if( (min[i] <= r.min[i] && r.min[i] <= max[i]) ||
             (r.min[i] <= min[i] && min[i] <= r.max[i]) )
      sum += 0;
    else
    {
      cout << "Rectangle<dim>::Distance(): Missing case!" << endl
          << "   min[" << i << "] = " <<   min[i] 
          << "   max[" << i << "] = " <<   max[i] << endl
          << " r.min[" << i << "] = " << r.min[i] 
          << " r.max[" << i << "] = " << r.max[i] << endl;
      assert( 0 );
    }
  }
  return sqrt(sum);
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
      if( nl->IsAtom( nl->First( l ) ) && 
          nl->AtomType( nl->First( l ) ) == RealType )
      {
        min[i] = nl->RealValue( nl->First( l ) );
        l = nl->Rest( l );
      }
      else
      {
        correct = false;
        break;
      }

      if( nl->IsAtom( nl->First( l ) ) && 
          nl->AtomType( nl->First( l ) ) == RealType )
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

