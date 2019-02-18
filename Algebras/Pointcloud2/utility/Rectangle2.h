/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 The Rectangle2 class

This class represents a plain, dim-dimensional, non-Attribute rectangle
(i.e. a rectangle that has no concept of defined or undefined and cannot be
stored as an attribute in a relation) with most methods provided by the
Rectangle class. As the class is not derived from (Spatial)Attribute, we save
8 bytes of memory for each instance.

*/

#pragma once

#include <memory>
#include <cmath>
#include <iostream>
#include <limits.h>
#include <assert.h>

#include "AlmostEqual.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"

namespace pointcloud2 {

#ifndef MAX
#define MAX( a, b ) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN( a, b ) ((a) < (b) ? (a) : (b))
#endif

template <unsigned dim>
class Rectangle2 {
public:
/*
The empty constructor.

*/
    inline Rectangle2();

/*
The destructor.

*/
    inline ~Rectangle2() = default;

/*
The first constructor.

*/
    inline explicit Rectangle2( const double* minMax );

/*
The second constructor.

*/
    inline explicit Rectangle2( const double *min, const double *max );

/*
The copy constructor.

*/
    inline Rectangle2( const Rectangle2<dim>& r );

/*
Checks if the rectangle is defined.

*/
    inline void Set(const double *min, const double *max);

/*
Redefinition of operator ~=~.

*/
    inline Rectangle2<dim>& operator = ( const Rectangle2<dim>& r );

/*
Checks if the rectangle contains the rectangle ~r~.

*/
    inline bool Contains( const Rectangle2<dim>& r ) const;


/*
Checks if the rectangle intersects with rectangle ~r~.

*/
    inline bool Intersects( const Rectangle2<dim>& r ) const;

/*
Redefinition of operator ~==~.

*/
    inline bool operator == ( const Rectangle2<dim>& r ) const;

/*
Fuzzy version of the operator ~==~.

*/
    inline bool AlmostEqual( const Rectangle2<dim>& r ) const;


/*
Redefinition of operator ~!=~.

*/
    inline bool operator != ( const Rectangle2<dim>& r ) const;

/*
Returns the area of a rectangle.

*/
    inline double Area() const;

/*
Returns the perimeter of a rectangle.

*/
    inline double Perimeter() const;

/*
Returns the min coord value for the given dimension ~dim~.

*/
    inline const double MinD( int d ) const;

/*
Returns the max coord value for the given dimension ~dim~.

*/
    inline const double MaxD( int d ) const;

/*
Returns the bounding box that contains both this and the rectangle ~r~.

*/
    inline Rectangle2<dim> Union( const Rectangle2<dim>& r) const;

/*
Returns the area of the bounding box that contains both this and the
rectangle ~r~.

*/
    inline double UnionArea( const Rectangle2<dim>& r) const;

/*
Extends this rectangle to contain both, this rectangle and the argument
(sets this rectangle to be the union of this and r).

*/
   inline void Extend(const Rectangle2<dim>&r );


/*
Scales a rectangle uniformely in all dimensions.
If sf is zero, the result will be undefined.

*/

    void scale(const double sf);



/*
Translates the rectangle given the translate vector ~t~.

*/
    inline Rectangle2<dim>& Translate( const double t[dim] );

/*
Returns the intersection between this and the rectangle ~r~.

This next functions are necessary for a ~rectangle~ be
an attribute in the Relational Algebra.

*/
    inline Rectangle2<dim> Intersection( const Rectangle2<dim>& b ) const;

/*
Returns the bounding box of the rectangle; this bounding Box is a clone
of the rectangle.

*/
    inline const Rectangle2<dim> BoundingBox() const;

    inline size_t Sizeof() const { return sizeof( *this ); }

/*
~Extend~

Enlarges this rectangle with a border of size ~b~. The function
changes the ~this~ object and returns it.

*/
    inline const Rectangle2<dim>& Extend(const double b);

    inline Rectangle2* Clone() const {
        return new Rectangle2<dim>( *this );
    }

    inline std::ostream& Print( std::ostream &os ) const {
        os << "Rectangle: ( ";
        for(unsigned int i=0; i < dim; i++)
            os<<min[i]<<" "<<max[i]<<" ";
        os << ")" << std::endl;
        return os;
    }

    inline double Size() const{
        double accu = +0.0;
        try {
          accu = (max[0] - min[0]);
          for(unsigned int i=1; i<dim; i++)
            accu *= (max[i] - min[i]);
          accu = std::abs(accu);
        }
        catch(...) { // catch any exception!
          accu = -1.0;
        }
        return accu;
    }

    inline Rectangle<dim> toRectangleAttr() {
        return Rectangle<dim>(true, min, max);
    }

  private:

/*
Returns ~true~ if this is a "proper" rectangle.

*/
    inline bool Proper() const;

/*
The lower limits of the intervals in each dimension.

*/
    double min[dim];

/*
The upper limits of the intervals in each dimension.

*/
    double max[dim];
};

/*
4 Implementation of the class ~Rectangle2~

The first constructor.

*/
template <unsigned dim>
inline Rectangle2<dim>::Rectangle2( const double* minMax ) {
    for (unsigned i = 0; i < dim; i++ ) {
        min[i] = minMax[2*i];
        max[i] = minMax[2*i+1];
    }
    assert ( Proper() );
}


template <unsigned dim>
inline Rectangle2<dim>::Rectangle2() {
    for (unsigned i = 0; i < dim; i++ ) {
        min[i] = 0;
        max[i] = 0;
    }
}

/*
The second constructor.

*/
template <unsigned dim>
inline Rectangle2<dim>::Rectangle2( const double *min, const double *max ) {
   Set(min, max);
}

/*
The copy constructor.

*/
template <unsigned dim>
inline Rectangle2<dim>::Rectangle2( const Rectangle2<dim>& r ) {
    for ( unsigned i = 0; i < dim; i++ ) {
        min[i] = r.min[i];
        max[i] = r.max[i];
    }
    // assert( Proper() ); // this must have been checked on r before
}

/*
Sets the values of this rectangle.

*/
template <unsigned dim>
inline void Rectangle2<dim>::Set(const double *min, const double *max) {
    for( unsigned i = 0; i < dim; i++ ) {
        this->min[i] = min[i];
        this->max[i] = max[i];
    }
    assert ( Proper() );
}


/*
Redefinition of operator ~=~.

*/
template <unsigned dim>
inline Rectangle2<dim>& Rectangle2<dim>::operator=(const Rectangle2<dim>& r) {
    for( unsigned i = 0; i < dim; i++ ) {
        this->min[i] = r.min[i];
        this->max[i] = r.max[i];
    }
    assert( Proper() );
    return *this;
}

/*
Checks if the rectangle contains the rectangle ~r~.

*/
template <unsigned dim>
inline bool Rectangle2<dim>::Contains( const Rectangle2<dim>& r ) const {
    for ( unsigned i = 0; i < dim; i++ ) {
        if( min[i] > r.min[i] || max[i] < r.max[i] ) {
            if (min[i] > r.min[i] && !::AlmostEqual(min[i], r.min[i]))
                return false;
            if (max[i] < r.max[i] && !::AlmostEqual(max[i], r.max[i]))
                return false;
        }
    }
    return true;
}

/*
Checks if the rectangle intersects with rectangle ~r~.

*/
template <unsigned dim>
inline bool Rectangle2<dim>::Intersects( const Rectangle2<dim>& r ) const {
    for ( unsigned i = 0; i < dim; i++ )
        if( max[i] < r.min[i] || r.max[i] < min[i] )
            return false;
    return true;
}


/*
Redefinition of operator ~==~.

*/
template <unsigned dim>
inline bool Rectangle2<dim>::operator == ( const Rectangle2<dim>& r ) const {
    for( unsigned i = 0; i < dim; i++ )
      if( min[i] != r.min[i] || max[i] != r.max[i] )
          return false;
    return true;
}

/*
Fuzzy check for equality.

*/
template <unsigned dim>
inline bool Rectangle2<dim>::AlmostEqual( const Rectangle2<dim>& r ) const {
    for (unsigned i = 0; i < dim; i++ ){
        if( !::AlmostEqual(min[i], r.min[i]) ||
            !::AlmostEqual(max[i], r.max[i]) ){
            return false;
        }
    }
    return true;
}

/*
Redefinition of operator ~!=~.

*/
template <unsigned dim>
inline bool Rectangle2<dim>::operator != ( const Rectangle2<dim>& r ) const {
  return !(*this == r);
}

/*
Returns the area of a rectangle.

*/
template <unsigned dim>
inline double Rectangle2<dim>::Area() const {
    double area = 1.0;
    for(unsigned i = 0; i < dim; i++ )
        area *= max[i] - min[i];
    return area;
}

/*
Returns the perimeter of a rectangle.

*/
template <unsigned dim>
inline double Rectangle2<dim>::Perimeter() const {
    double perimeter = 0.0;
    for ( unsigned i = 0; i < dim; i++ )
        perimeter += std::pow( 2, (double)dim-1 ) * ( max[i] - min[i] );
    return perimeter;
}

/*
Returns the min coord value for the given dimension ~d~.

*/
template <unsigned dim>
inline const double Rectangle2<dim>::MinD( int d ) const {
  assert( d >= 0 && (unsigned)d < dim );
  return min[d];
}

/*
Returns the max coord value for the given dimension ~d~.

*/
template <unsigned dim>
inline const double Rectangle2<dim>::MaxD( int d ) const {
  assert( d >= 0 && (unsigned)d < dim );
  return max[d];
}

/*
Returns the bounding box that contains both this and the rectangle ~r~.

*/
template <unsigned dim>
inline Rectangle2<dim> Rectangle2<dim>::Union(
        const Rectangle2<dim>& r) const {
    double auxmin[dim], auxmax[dim];
    for( unsigned i = 0; i < dim; i++ ) {
        auxmin[i] = MIN( min[i], r.min[i] );
        auxmax[i] = MAX( max[i], r.max[i] );
    }
    return Rectangle2<dim>( auxmin, auxmax );
}


/*
Returns the area of the bounding box that contains both this and the
rectangle ~r~.

*/
template <unsigned dim>
inline double Rectangle2<dim>::UnionArea( const Rectangle2<dim>& r) const {
    double area = 1.0;
    for( unsigned i = 0; i < dim; i++ ) {
        double maxUnion = MAX( max[i], r.max[i] );
        double minUnion = MIN( min[i], r.min[i] );
        area *= maxUnion - minUnion;
    }
    return area;
}

template<unsigned dim>
inline void Rectangle2<dim>::Extend(const Rectangle2<dim>& r ) {
    for(unsigned i=0;i<dim;i++){
        min[i] = MIN(min[i],r.min[i]);
        max[i] = MAX(max[i],r.max[i]);
    }
}



/*
Translates the rectangle given the translate vector ~t~.

*/
template <unsigned dim>
inline Rectangle2<dim>& Rectangle2<dim>::Translate( const double t[dim] ) {
    for( unsigned i = 0; i < dim; i++ ) {
        min[i] += t[i];
        max[i] += t[i];
    }
    return *this;
}

template <unsigned dim>
inline const Rectangle2<dim>& Rectangle2<dim>::Extend(const double b) {
    assert(b >= 0.0);
    for (unsigned int i = 0; i < dim; i++){
        min[i] -=b;
        max[i] +=b;
    }
    return *this;
}


/*
Returns the bounding box of the rectangle; this bounding Box is a clone
of the rectangle.

*/
template <unsigned dim>
inline const Rectangle2<dim> Rectangle2<dim>::BoundingBox() const {
    return Rectangle2<dim>( *this );
}

/*
Returns the intersection between this and the rectangle ~r~.

*/
template <unsigned dim>
inline Rectangle2<dim> Rectangle2<dim>::Intersection(
        const Rectangle2<dim>& r ) const {
    double auxmin[dim], auxmax[dim];
    for( unsigned i = 0; i < dim; i++ ) {
        auxmin[i] = MAX( min[i], r.min[i] );
        auxmax[i] = MIN( max[i], r.max[i] );
    }
    return Rectangle2<dim>( auxmin, auxmax );
}

/*
Returns ~true~ if this is a "proper" rectangle, i.e. it does not
represent an empty set.

*/
template <unsigned dim>
inline bool Rectangle2<dim>::Proper() const {
    for( unsigned i = 0; i < dim; i++ ) {
        if( min[i] > max[i] )
            return false;
    }
    return true;
}


template<unsigned dim>
void Rectangle2<dim>::scale(const double sf) {
    for(unsigned int i=0;i<dim;i++){
        min[i] *= sf;
        max[i] *= sf;
        if(sf<0)
            std::swap(min[i],max[i]);
    }
}

} // namespace
