/*
This class is a Point3.

*/
#include "Label.h"
#include "../../Tools/Flob/Flob.h"
#include "../../Tools/Flob/DbArray.h"
#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "GenericTC.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "SecondoConfig.h"
#include "AvlTree.h"
#include "AVLSegment.h"
#include "AlmostEqual.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "RegionTools.h"
#include "Symbols.h"
#include "Stream.h"
#include "RegionCreator.h"
#include "StringUtils.h"

#include <vector>
#include <queue>
#include <stdexcept>
#include <iostream>
#include <string.h>
#include <string>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <queue>
#include <iterator>
#include <sstream>
#include <limits>
#include <errno.h>
#include <cerrno>
#include "../TopRel/Tree.h"

#include "RobustSetOps.h"

#include "DLine.h"

#include "DRM.h"
#include "Disc.h"
#include "Stack.h"
#include "Point3.h"

using namespace std;


Point3::Point3( const bool d, const Coord& _x, const Coord& _y, 
const Coord& _alpha): 
                StandardSpatialAttribute<3>(d), x(_x), y(_y), alpha(_alpha) {}
/*
The second one receives a point ~p~ as argument and creates a point that is a
copy of ~p~.

*/
Point3::Point3( const Point3& p ): 
StandardSpatialAttribute<3>(p.IsDefined()), x(p.x), y(p.y), alpha(p.alpha) {}

const Rectangle<3> Point3::BoundingBox(const Geoid* geoid) const {
  return Rectangle<3>(0);
}
/*
Sets the value of the point object.

*/
void Point3::Set( const Coord& _x, const Coord& _y, const Coord& _alpha ) {
    x=_x;
    y=_y;
    alpha=_alpha;
}

void Point3::Set( const Point3& p) {
    x=p.x;
    y=p.y;
    alpha=p.alpha;
}


ostream& operator<<( ostream& o, const Point3& p )
{
  ios_base::fmtflags oldOptions = o.flags();
  o.setf(ios_base::fixed,ios_base::floatfield);
  o.precision(8);
  if( p.IsDefined() )
    o << "(" << p.GetX() << ", " << p.GetY() << ", " << p.GetAlpha() << ")";
  else
    o << Symbol::UNDEFINED();
  o.flags(oldOptions);
  return o;
}

ostream& Point3::Print( ostream &os ) const
{
  return os << *this;
}

string Point3::toString(const Geoid* geoid) const {
  if(!IsDefined()){
    return Symbol::UNDEFINED();
  }
  stringstream s;
  s.setf(ios_base::fixed,ios_base::floatfield);
  s.precision(8);
  s << *this;
  return s.str();
}

void Point3::ReadFromString(string value) {

  ListExpr list;
  if(!nl->ReadFromString(value,list)){
     if(!nl->ReadFromString("("+value+")",list)){
        SetDefined(false); 
        return;
     }
  }
  if(listutils::isSymbolUndefined(list)){
     SetDefined(false);
     return;
  }
  if(    nl->HasLength(list,3) 
      && listutils::isNumeric(nl->First(list))
      && listutils::isNumeric(nl->Second(list))
      && listutils::isNumeric(nl->Third(list))){
     Set(listutils::getNumValue(nl->First(list)),
         listutils::getNumValue(nl->Second(list)),
         listutils::getNumValue(nl->Third(list)));
     return;
  }
  SetDefined(false);
}




double Point3::Distance( const Point3& p, const Geoid* geoid /* = 0 */ ) const
{
  assert( IsDefined() );
  assert( p.IsDefined() ); 
  double dx = p.x - x,
         dy = p.y - y,
         dalpha = p.alpha - alpha;
  return sqrt( pow( dx, 2 ) + pow( dy, 2 ) + pow( dalpha, 2));
}

double Point3::Distance( const Rectangle<3>& r, const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( r.IsDefined() );
  double rxmin = r.MinD(0), rxmax = r.MaxD(0),
         rymin = r.MinD(1), rymax = r.MaxD(1),
         ralphamin = r.MinD(2), ralphamax = r.MaxD(2);
  double dx =
        (   (x > rxmin || AlmostEqual(x,rxmin))
         && (x < rxmax || AlmostEqual(x,rxmax))) ? (0.0) :
        (min(abs(x-rxmin),abs(x-rxmax)));
  double dy =
        (   (y > rymin || AlmostEqual(y,rymin))
         && (y < rymax || AlmostEqual(y,rymax))) ? (0.0) :
        (min(abs(y-rymin),abs(y-rymax)));
  double dalpha =
        (   (alpha > ralphamin || AlmostEqual(alpha,ralphamin))
         && (alpha < ralphamax || AlmostEqual(alpha,ralphamax))) ? (0.0) :
        (min(abs(alpha-ralphamin),abs(alpha-ralphamax)));
  return sqrt( pow( dx, 2 ) + pow( dy, 2 ) + pow(dalpha, 2));
}

bool Point3::Intersects(const Rectangle<3>& r, const Geoid* geoid/*=0*/) const{
  assert(IsDefined());
  assert(r.IsDefined());
  return     (x>=r.MinD(0) ) && (x<=r.MaxD(0)) 
          && (y>=r.MinD(1) ) && (y<=r.MaxD(1))
          && (alpha>=r.MinD(2) ) && (alpha<=r.MaxD(2));
}




Point3 Point3::MidpointTo(const Point3& p, const Geoid* geoid /* = 0 */ ) const
{
  if ( !this->IsDefined() || !p.IsDefined() ) {
    return Point3( false );
  }

  if( AlmostEqual(*this,p) ){ // no calculation required.
    return *this;
  }
  return Point3( true, ( GetX() + p.GetX() )/2.0, ( GetY() + p.GetY() )/2.0,
    ( GetAlpha() + p.GetAlpha() )/2.0  );

}

Point3 Point3::MidpointTo(const Point3& p, const Coord& f,
                        const Geoid* geoid /*=0*/) const{
  if( !IsDefined() || !p.IsDefined() ||
      (geoid && !geoid->IsDefined()) || (f<0.0) || (f>1.0) ) {
  return Point3(false);
  }
  if(AlmostEqual(f,0.0)) {
    return *this;
  }
  if(AlmostEqual(f,1.0)) {
    return p;
  }
    double x1 = GetX();
    double y1 = GetY();
    double alpha1 = GetAlpha();
    double x2 = p.GetX();
    double y2 = p.GetY();
    double alpha2 = p.GetAlpha();
    double dx = x2-x1;
    double dy = y2-y1;
    double dalpha = alpha2-alpha1;
    double x = x1 + (f*dx);
    double y = y1 + (f*dy);
    double alpha = alpha1 + (f*dalpha);
    return Point3( true, x, y, alpha );
}


//use this when adding and sorting the DBArray
int Point3Compare( const void *a, const void *b )
{
  const Point3 *pa = (const Point3*)a,
              *pb = (const Point3*)b;
  assert(pa->IsDefined());
  assert(pb->IsDefined());
  if( *pa == *pb )
    return 0;

  if( *pa < *pb )
    return -1;

  return 1;
}

// use this when testing for containment or removing duplicates
// in the DBArray
int Point3CompareAlmost( const void *a, const void *b )
{
  const Point3 *pa = (const Point3*)a,
              *pb = (const Point3*)b;

  assert(pa->IsDefined());
  assert(pb->IsDefined());
  if( AlmostEqual( *pa, *pb ) )
    return 0;

  if( *pa < *pb )
    return -1;

  return 1;
}

/*
Function supporting the RemoveDuplicates function.
This function checks whether in an array of points
a point exists which is AlmostEqual to the given one.
The search is restricted to the range in array given
by the indices __min__ and __max__.

*/
//FIXME
bool AlmostContains( const DbArray<Point3>& points, const Point3& p,
                     int min, int max, int size){

  if(min>max){
     return false;
  }
  Point3 pa;
  if(min==max){ // search around the position found
     // search left of min
     int pos = min;
     double x = p.GetX();
     points.Get(pos,&pa);
     while(pos>=0 && AlmostEqual(pa.GetX(),x)){
        if(AlmostEqual(pa,p)){
           return true;
        }
        pos--;
        if(pos>=0){
          points.Get(pos,&pa);
        }
     }
     // search right of min
     pos=min+1;
     if(pos<size){
        points.Get(pos,&pa);
     }
     while(pos<size &&AlmostEqual(pa.GetX(),x)){
        if(AlmostEqual(pa,p)){
          return  true;
        }
        pos++;
        if(pos<size){
           points.Get(pos,&pa);
        }
    }
    return false; // no matching point found
  } else {
      int mid = (min+max)/2;
      points.Get(mid,&pa);
      double x = pa.GetX();
      double cx = p.GetX();
      if(AlmostEqual(x,cx)){
         return AlmostContains(points,p,mid,mid,size);
      } else if(cx<x){
         return AlmostContains(points,p,min,mid-1,size);
      }else {
         return AlmostContains(points,p,mid+1,max,size);
      }
  }
}

//FIXME
Point3& Point3::operator=( const Point3& p ) { 
    Set(p);
    return *this;
}

bool Point3::operator<=( const Point3& p ) const {
    if ((!IsDefined()) || (!p.IsDefined()))
        return false;
    if (x <= p.x)
        return true;
    if (y <= p.y)
        return true;
    if (alpha <= p.alpha)
        return true;
    return false;
}

bool Point3::operator<( const Point3& p ) const {
    if ((!IsDefined()) || (!p.IsDefined()))
        return false;
    if (x < p.x)
        return true;
    if (y < p.y)
        return true;
    if (alpha < p.alpha)
        return true;
    return false;
}

bool Point3::operator>=( const Point3& p ) const {
    if ((!IsDefined()) || (!p.IsDefined()))
        return false;
    if (x >= p.x)
        return true;
    if (y >= p.y)
        return true;
    if (alpha >= p.alpha)
        return true;
    return false;
}

bool Point3::operator==( const Point3& p ) const {
    if ((!IsDefined()) || (!p.IsDefined()))
        return false;
    if (x == p.x)
        return true;
    if (y == p.y)
        return true;
    if (alpha == p.alpha)
        return true;
    return false;
}

bool Point3::operator!=(const Point3 & p) const {
    return !(*this==p);
}

bool Point3::operator>( const Point3& p ) const {
    if ((!IsDefined()) || (!p.IsDefined()))
        return false;
    if (x > p.x)
        return true;
    if (y > p.y)
        return true;
    if (alpha > p.alpha)
        return true;
    return false;
}

Point3 Point3::operator+( const Point3& p ) const {
    if ((!IsDefined()) || (!p.IsDefined()))
        return Point3(false);
    return Point3(true, p.x+x, p.y+y, p.alpha+alpha);
}

Point3 Point3::operator-( const Point3& p ) const {
    if ((!IsDefined()) || (!p.IsDefined()))
        return Point3(false);
    return Point3(true, x-p.x, y-p.y, alpha-p.alpha);
}

Point3 Point3::operator*( const double d ) const {
    if (!IsDefined())
        return Point3(false);
    return Point3(true, d*x, d*y, d*alpha);
}
