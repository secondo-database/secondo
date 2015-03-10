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
#include <limits.h>
#include <iostream>
#include "stdarg.h"
#include "Attribute.h"
#include "Messages.h"
#include "Geoid.h"
#include "ListUtils.h"

#include <stdio.h>

#ifdef SECONDO_WIN32
#define Rectangle SecondoRectangle
#endif

#define MAX( a, b ) ((a) > (b) ? (a) : (b))
#define MIN( a, b ) ((a) < (b) ? (a) : (b))

template <unsigned dim>
class Rectangle;

template <unsigned dim>
class StandardSpatialAttribute : public Attribute
{
  public:
    StandardSpatialAttribute() : Attribute() {}
    StandardSpatialAttribute(bool defined):Attribute(defined) {}

    virtual const Rectangle<dim> BoundingBox(const Geoid* geoid = 0) const = 0;
    virtual double Distance(const Rectangle<dim>& rect,
                            const Geoid* geoid=0) const = 0;
    
    virtual bool Intersects(const Rectangle<dim>& rect,
                            const Geoid* geoid=0 ) const = 0;

    virtual bool IsEmpty() const = 0;

    static unsigned GetDim(){
      return dim;
    }

};

/*
3 Class ~Rectangle~

*/
template <unsigned dim>
class Rectangle: public StandardSpatialAttribute<dim>
{
  public:

/*
Do not use the standard constructor:

*/
    inline Rectangle():StandardSpatialAttribute<dim>() {}

/*
The first constructor. First one can set if the rectangle is defined, and if it is,
the coordinates can be set.

*/
    inline Rectangle( const bool defined, ... );

/*
The second constructor. First one can set if the rectangle is defined, and if it is,
the coordinates can be set.

*/
    inline Rectangle( const bool defined,
                      const double *min, const double *max );

/*
The copy constructor.

*/
    inline Rectangle( const Rectangle<dim>& r );

/*
Checks if the rectangle is defined.

*/
    inline void Set(const bool defined, const double *min, const double *max);

/*
Checks if the rectangle is defined (For conformity with other spatial types)

*/
    inline bool IsEmpty() const { return !(this->IsDefined()); };

/*
Redefinition of operator ~=~.

*/
    inline Rectangle<dim>& operator = ( const Rectangle<dim>& r );

/*
Checks if the rectangle contains the rectangle ~r~.

*/
    inline bool Contains( const Rectangle<dim>& r, const Geoid* geoid=0 ) const;
    

/*
Checks if the rectangle intersects with rectangle ~r~. Both rectangles
must be defined.

*/
    inline bool Intersects( const Rectangle<dim>& r,
                            const Geoid* geoid=0 ) const;
/*
Checks if the rectangle intersects with rectangle ~r~. If one of the
involved rectangles is not defined, the result will be false;

*/
    inline bool IntersectsUD( const Rectangle<dim>& r,
                          const Geoid* geoid=0 ) const;

/*
Redefinition of operator ~==~.

*/
    inline bool operator == ( const Rectangle<dim>& r ) const;

/*
Fuzzy version of the operator ~==~.

*/
    inline bool AlmostEqual( const Rectangle<dim>& r ) const;


/*
Redefinition of operator ~!=~.

*/
    inline bool operator != ( const Rectangle<dim>& r ) const;

/*
Returns the area of a rectangle.

*/
    inline double Area(const Geoid* geoid=0) const;

/*
Returns the perimeter of a rectangle.

*/
    inline double Perimeter(const Geoid* geoid=0) const;

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
    inline Rectangle<dim> Union( const Rectangle<dim>& r,
                                 const Geoid* geoid=0 ) const;

/*
Extends this rectangle to contain both, this rectangle and the argument 
(sets this rectangle to be the union of this and r).

*/
   inline void Extend(const Rectangle<dim>&r, const Geoid* geoid=0);

/*
Returns the Euclidean distance (~geoid~ = NULL) resp. spherical distance between
two rectangles.

*/
    inline double Distance(const Rectangle<dim>& r, const Geoid* geoid=0) const;


/*
Returns the square of the maximum distance between two points within this rectangle and 
~r~.

*/
    double QMaxMaxDistance(const Rectangle<dim>& r) const;


/*
Returns the square of the minmaxdistance between this and ~r~.

*/
    double QMinMaxDistance(const Rectangle<dim>& r) const;
   



/*
Translates the rectangle given the translate vector ~t~.

*/
    inline Rectangle<dim>& Translate( const double t[dim] );

/*
Returns the intersection between this and the rectangle ~r~.

This next functions are necessary for a ~rectangle~ be
an attribute in the Relational Algebra.

*/
    inline Rectangle<dim> Intersection( const Rectangle<dim>& b,
                                        const Geoid* geoid=0 ) const;

/*
Returns the bounding box of the rectangle; this bounding Box is a clone
of the rectangle.

*/
    inline const Rectangle<dim> BoundingBox(const Geoid* geoid = 0) const;

    inline static const string BasicType() {
      if(dim==2){
        return "rect";
      } else {
        ostringstream ss;
        ss << "rect" << dim;
        return ss.str();
      }
    }

    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }

    inline size_t Sizeof() const { return sizeof( *this ); }

    inline size_t HashValue() const {
      size_t h = 0;
      for( unsigned i = 0; i < dim; i++ )
        h += size_t(4 * min[i] + max[i]);
      return h;
    }

    inline void CopyFrom( const Attribute* right )
      { *this = *(const Rectangle<dim>*)right; }

    inline int Compare( const Attribute *arg ) const {
      unsigned thispos, rpos;
      unsigned thismin[dim], rmin[dim];
      const Rectangle<dim>* r = (const Rectangle<dim>*)arg;
      if(!(this->IsDefined())
           && !r->IsDefined())
        return 0;
      if(!(this->IsDefined()))
        return -1;
      if(!r->IsDefined())
        return 1;
      //order on rectangles is z-order (bit interleaving)
      // treat positive/negative quadrants
      thispos = 0;
      rpos = 0;
      for (unsigned j = 0; j < dim; j++){
        thispos <<= 1;
        thispos |= (min[j] >= 0);
        rpos <<= 1;
        rpos |= (r->min[j] >= 0);
      }
      if (thispos < rpos){
        return -1;
      }
      if (thispos > rpos){
        return 1;
      }

      // now treat z-order based on positive integer coordinates
      for (unsigned j = 0; j < dim; j++){
        thismin[j] = (unsigned) fabs(min[j]);
        rmin[j] = (unsigned) fabs(r->min[j]);
      }

      int bits = sizeof(unsigned)*8-1;
      for (int j = bits; j >= 0; j--){
        thispos = 0;
        rpos = 0;
        for (unsigned k = 0; k < dim; k++){
          thispos <<= 1;
          thispos |= ((thismin[k] >> j) & 1);
          rpos <<= 1;
          rpos |= ((rmin[k] >> j) & 1);
        }
        if (thispos < rpos){
          return -1;
        }
        if (thispos > rpos){
          return 1;
        }
      }


      // if no conclusion on z-order (based on integer coordinates) can be
      // reached, we fall back to the standard comparison
      for( unsigned i = 0; i < dim; i++ ){
        if( this->min[i] < r->min[i] ){
          return -1;
        }
        if( this->min[i] > r->min[i] ){
          return 1;
        }
      }
      for( unsigned i = 0; i < dim; i++ ){
        if( this->max[i] < r->max[i] ){
          return -1;
        }
        if( this->max[i] > r->max[i] ){
          return 1;
        }
      }
      return 0;
    }

/*
~Extend~

Enlarges this rectangle with a border of size ~b~. The function
changes the ~this~ object and returns it.

*/
    inline const Rectangle<dim>& Extend(const double b);

    inline bool Adjacent( const Attribute *arg ) const
      { return false; }

    inline Rectangle* Clone() const
      { return new Rectangle<dim>( *this ); }

    inline ostream& Print( ostream &os ) const {
      if( this->IsDefined() ){
          os << "Rectangle: ( ";
          for(unsigned int i=0; i < dim; i++)
            os<<min[i]<<" "<<max[i]<<" ";
          os<<")"<<endl;
          return os;
      } else {
          return os << Symbol::UNDEFINED();
      }
    }

    inline double Size() const{
        if(!this->IsDefined()){
          return -1.0;
        }
        double accu = +0.0;
        try{
          accu = (max[0] - min[0]);
          for(unsigned int i=1; i<dim; i++){
            accu *= (max[i] - min[i]);
          };
          accu = abs(accu);
        }
        catch(...){ // catch any exception!
          accu = -1.0;
        }
        return accu;
    }

/*
Projection operator: Returns a 2D-Rectangle created by a projection of this
Rectangle to arbirary dimensions.

*/
    Rectangle<2u> Project2D(int dX, int dY) const {
      assert(dX >= 0);
      assert((unsigned)dX < dim);
      assert(dY >= 0);
      assert((unsigned)dY < dim);
      return
          Rectangle<2u>(this->IsDefined(),MinD(dX),MaxD(dX),MinD(dY),MaxD(dY));
    }

/*
Projection to the first newdim components of this rectangle. Note: newdim must be
smaller or equal to the currect dimension for this rectangle.

*/    
   template<int newdim>
   Rectangle<newdim> project(){
      assert(newdim<=dim);
      Rectangle<newdim> res(this->IsDefined(), min, max);
      return res;
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
4 Implementation of the class ~Rectangle~

The first constructor. First one can set if the rectangle is defined, and if it is,
the coordinates can be set.

*/
template <unsigned dim>
inline Rectangle<dim>::Rectangle( const bool defined, ... ):
    StandardSpatialAttribute<dim>(defined)
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

  if( !Proper() )
  {
    static MessageCenter* msg = MessageCenter::GetInstance();
    NList msgList( NList("simple"),
                   NList("Rectangle built with invalid dimensions!") );
    msg->Send(nl,msgList.listExpr());
    this->del.isDefined = false;
  }
}

/*
The second constructor. First one can set if the rectangle is defined, and if it is,
the coordinates can be set.

*/
template <unsigned dim>
inline Rectangle<dim>::Rectangle( const bool defined, const double *min,
                                  const double *max ):
    StandardSpatialAttribute<dim>(defined)
{
   Set(defined,min,max);
}

/*
The copy constructor.

*/
template <unsigned dim>
inline Rectangle<dim>::Rectangle( const Rectangle<dim>& r ):
    StandardSpatialAttribute<dim>(r.IsDefined())
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
  this->del.isDefined = defined;
  for( unsigned i = 0; i < dim; i++ )
  {
    this->min[i] = min[i];
    this->max[i] = max[i];
  }
  if( !Proper() )
  {
    static MessageCenter* msg = MessageCenter::GetInstance();
    NList msgList( NList("simple"),
                   NList("Rectangle built with invalid dimensions!") );
    msg->Send(nl,msgList.listExpr());
    this->SetDefined(false);
  }
}


/*
Redefinition of operator ~=~.

*/
template <unsigned dim>
inline Rectangle<dim>& Rectangle<dim>::operator=( const Rectangle<dim>& r )
{
  this->del.isDefined = r.IsDefined();
  if( (this->del.isDefined) )
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
inline bool Rectangle<dim>::Contains( const Rectangle<dim>& r,
                                      const Geoid* geoid/*=0*/ ) const
{
  assert( (this->del.isDefined) && r.IsDefined()
                                && (!geoid || geoid->IsDefined()) );

  for( unsigned i = 0; i < dim; i++ ){

    /*if( min[i] > r.min[i] || max[i] < r.max[i] ){ //original one
      return false;
    }*/
    if( min[i] > r.min[i] || max[i] < r.max[i] ){
        if(min[i] > r.min[i] && !::AlmostEqual(min[i],r.min[i])) return false;
        if(max[i] < r.max[i] && !::AlmostEqual(max[i],r.max[i])) return false;
    }

  }
  return true;
}

/*
Checks if the rectangle intersects with rectangle ~r~.

*/
template <unsigned dim>
inline bool Rectangle<dim>::Intersects( const Rectangle<dim>& r,
                                        const Geoid* geoid/*=0*/ ) const
{
  assert( this->del.isDefined && r.IsDefined()
                              && (!geoid || geoid->IsDefined()));

  for( unsigned i = 0; i < dim; i++ )
    if( max[i] < r.min[i] || r.max[i] < min[i] )
      return false;

  return true;
}


template <unsigned dim>
inline bool Rectangle<dim>::IntersectsUD( const Rectangle<dim>& r,
                                          const Geoid* geoid/*=0*/ ) const
{
  if(!this->del.isDefined || !r.IsDefined()
                          || (geoid && !geoid->IsDefined())){
      return false;
  }

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
  assert( this->IsDefined());
  assert( r.IsDefined() );

  for( unsigned i = 0; i < dim; i++ )
    if( min[i] != r.min[i] || max[i] != r.max[i] )
      return false;

  return true;
}

/*
Fuzzy check for equality.

*/
template <unsigned dim>
inline bool Rectangle<dim>::AlmostEqual( const Rectangle<dim>& r ) const
{
  if(!(this->del.isDefined) && !r.IsDefined()){
     return true;
  } else if(!(this->del.isDefined) || !r.IsDefined()){
     return false;
  }
  for( unsigned i = 0; i < dim; i++ ){
    if( !::AlmostEqual(min[i] , r.min[i]) ||
        !::AlmostEqual(max[i] , r.max[i]) ){
    }
  }
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
inline double Rectangle<dim>::Area(const Geoid* geoid /*=0*/) const
{
  if( !(this->del.isDefined) || (geoid && !geoid->IsDefined()))
    return 0.0;
  if(geoid){
    cerr << ": Spherical geometry not implemented!" << endl;
    assert(false);
  }
  double area = 1.0;
  for( unsigned i = 0; i < dim; i++ )
    area *= max[i] - min[i];
  return area;
}

/*
Returns the perimeter of a rectangle.

*/
template <unsigned dim>
inline double Rectangle<dim>::Perimeter (const Geoid* geoid/*=0*/) const
{
  if( !(this->del.isDefined) || (geoid && !geoid->IsDefined()) )
    return 0.0;
  if(geoid){
    cerr << ": Spherical geometry not implemented!" << endl;
    assert(false);
  }
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
inline Rectangle<dim> Rectangle<dim>::Union( const Rectangle<dim>& r,
                                             const Geoid* geoid /*=0*/ ) const
{
  if( !this->del.isDefined || !r.IsDefined() || (geoid && !geoid->IsDefined()) )
    return Rectangle<dim>( false );
  if(geoid){
    cerr << ": Spherical geometry not implemented!" << endl;
    assert(false);
  }
  double auxmin[dim], auxmax[dim];
  for( unsigned i = 0; i < dim; i++ )
  {
    auxmin[i] = MIN( min[i], r.min[i] );
    auxmax[i] = MAX( max[i], r.max[i] );
  }
  return Rectangle<dim>( true, auxmin, auxmax );
}

template<unsigned dim>
inline void Rectangle<dim>::Extend(const Rectangle<dim>& r,
                                   const Geoid* geoid /*=0*/ ){

   assert(geoid==0); // not implemented yet
   if(!this->del.isDefined){
       if(!r.IsDefined()){
         return;
       } else {
         *this = r;
         return;
       }
   }
   if(!r.IsDefined()){
     return;
   }
   // both rectangle are defined
   for(unsigned i=0;i<dim;i++){
       min[i] = MIN(min[i],r.min[i]);
       max[i] = MAX(max[i],r.max[i]);
   }
}



/*
Translates the rectangle given the translate vector ~t~.

*/
template <unsigned dim>
inline Rectangle<dim>& Rectangle<dim>::Translate( const double t[dim] )
{
  if( (this->del.isDefined) )
  {
    for( unsigned i = 0; i < dim; i++ )
    {
      min[i] += t[i];
      max[i] += t[i];
    }
  }
  return *this;
}

template <unsigned dim>
inline const Rectangle<dim>& Rectangle<dim>::Extend(const double b){
   assert(b>=0.0);
   for(unsigned int i=0;i<dim;i++){
      min[i] -=b;
      max[i] +=b;
   }
   return  *this;
}


/*
Returns the bounding box of the rectangle; this bounding Box is a clone
of the rectangle.

*/
template <unsigned dim>
inline const Rectangle<dim> Rectangle<dim>::BoundingBox(const Geoid* geoid)const
{
  if(geoid){
    cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
    << endl;
    assert( !geoid ); // TODO: implement spherical geometry case
  }
  if( (this->del.isDefined) )
    return Rectangle<dim>( *this );
  else
    return Rectangle<dim>( false );
}

/*
Returns the intersection between this and the rectangle ~r~.

*/
template <unsigned dim>
inline Rectangle<dim>
Rectangle<dim>::Intersection( const Rectangle<dim>& r,
                              const Geoid* geoid /*=0*/ ) const
{
  if( !this->del.isDefined || !r.IsDefined() || (geoid && (!geoid->IsDefined()))
      || !Intersects( r,geoid ) ){
    return Rectangle<dim>( false );
  }

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
  if( (this->del.isDefined) )
  {
    for( unsigned i = 0; i < dim; i++ )
      if( min[i] > max[i] ) {
        return false;
      }
  }
  return true;
}


/*
Distance: returns the Euclidean distance between two rectangles

*/

template <unsigned dim>
inline double Rectangle<dim>::Distance(const Rectangle<dim>& r,
                                       const Geoid* geoid /*=0*/) const
{
  assert(this->del.isDefined);
  assert(r.del.isDefined );
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }
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
QMaxMaxDistance

*/
template <unsigned dim>
double Rectangle<dim>::QMaxMaxDistance(const Rectangle<dim>& r) const{
   double sum = 0.0;
   double aux[4];
   for(unsigned i=0;i<dim;i++){
       aux[0] = fabs(MinD(i) - r.MinD(i));
       aux[1] = fabs(MinD(i) - r.MaxD(i));
       aux[2] = fabs(MaxD(i) - r.MinD(i));
       aux[3] = fabs(MaxD(i) - r.MaxD(i));
       // determine the maximum of the values
       double a = aux[0];
       for(int j=1;j<4;j++){
          if(aux[j]>a){
            a = aux[j];
         }
       }
       sum += a*a;
      }
      return sum;
}


/*
QMinMaxDistance

*/
template<unsigned dim>
double Rectangle<dim>::QMinMaxDistance(const Rectangle<dim>& r) const{
      double sum = QMaxMaxDistance(r);
      double S[dim];
      for (unsigned k=0;k<dim; k++){
          S[k] = sum;
      }
      double aux[4];
      for(unsigned k=0;k<dim;k++){
         aux[0] = fabs(MinD(k) - r.MinD(k));
         aux[1] = fabs(MinD(k) - r.MaxD(k));
         aux[2] = fabs(MaxD(k) - r.MinD(k));
         aux[3] = fabs(MaxD(k) - r.MaxD(k));
         // subtract maximum value
         double a = aux[0];
         for(int j=1;j<4;j++){
           if(aux[j]>a){
             a = aux[j];
           } 
         } 
         S[k] -= a*a;
         // add minimum value
         a = aux[0];
         for(int j=1;j<4;j++){
           if(aux[j]<a){
             a = aux[j];
           } 
         } 
         S[k] += a*a;
      }
      // find maximum value
      double res = S[0];
      for(unsigned k=1;k<dim;k++){
        if(S[k] < res){
           res = S[k];
        }
      }
      return res;
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
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
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
      else if( nl->IsAtom(nl->First(l)) &&
               nl->AtomType(nl->First(l)) == IntType)
      {
        min[i] = nl->IntValue(nl->First(l));
        l = nl->Rest(l);
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
      else if(nl->IsAtom(nl->First(l)) &&
              nl->AtomType(nl->First(l)) == IntType )
      {
        max[i] = nl->IntValue(nl->First(l));
        l = nl->Rest(l);
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
      if( !r->IsDefined() )
      {
        correct = false;
        delete r;
        return SetWord(Address(0));
      }
      return SetWord( r );
    }
  }
  else if ( listutils::isSymbolUndefined(instance))
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

/*
4 Class ~RectangleSet~

This class simply contains a set of rectangles. It is used by the
MON-Tree to search in an R-Tree using a set of rectangles instead
of only one.

2.3 Struct ~RectangleSet~

This structure contains a set of rectangles used in the
MON-Tree search.

*/
template<unsigned dim>
class RectangleSet
{
  public:
    RectangleSet()
      {}

    inline RectangleSet<dim>& operator = ( const RectangleSet<dim>& r )
    {
      set.clear();
      set = r.set;
      return *this;
    }

    virtual ~RectangleSet(){}

    inline virtual bool Intersects( const Rectangle<dim>& r ) const
    {
      if( set.empty() )
        return false;

      for( size_t i = 0; i < set.size(); i++ )
        if( set[i].Intersects( r ) )
          return true;

      return false;
    }

    size_t Size() const
      { return set.size(); }

    void Add( const Rectangle<dim>& r )
    {
      set.push_back( r );
    }

    void Clear()
      { set.clear(); }

  protected:
    vector< Rectangle<dim> > set;
};

typedef Rectangle<1> Rect1;
typedef Rectangle<2> Rect;
typedef Rectangle<3> Rect3;
typedef Rectangle<4> Rect4;
typedef Rectangle<8> Rect8;

#endif

