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

//bool Point3::Inside( const Region& r,
//                    const Geoid* geoid /*=0*/ ) const
//{
//  return r.Contains(*this,geoid);
//}

//bool Point3::Inside( const Line& l,
//                    const Geoid* geoid /*=0*/ ) const
//{
//  return l.Contains(*this,geoid);
//}

//bool Point3::Inside(const SimpleLine& l, const Geoid* geoid /*=0*/) const
//{
//  return l.Contains(*this,geoid);
//}

//bool Point3::Inside( const Points& ps,
//                    const Geoid* geoid /*=0*/ ) const
//{
//  return ps.Contains(*this,geoid);
//}

//FIXME: Anpassen
//bool Point3::Inside( const Rectangle<3>& r, const Geoid* geoid =0 ) const
//{
//  assert( r.IsDefined() );
//  if( !IsDefined() || !r.IsDefined() || (geoid && !geoid->IsDefined()) ){
//    return false;
//  }
//  if( x < r.MinD(0) || x > r.MaxD(0) )
//    return false;
//  else if( y < r.MinD(1) || y > r.MaxD(1) )
//    return false;
//  return true;
//}

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



  // calculate the enclosed angle between (a,b) and (b,c) in degrees
//double Point3::calcEnclosedAngle( const Point3 &a,
  //      const Point3 &b,
    //    const Point3 &c,
      //  const Geoid* geoid /* = 0 */){
//  double beta = 0.0;
  //errno = 0;
    //double la = b.Distance(c);
//    double lb = a.Distance(c);
  //  double lc = a.Distance(b);
    //double cosb = (la*la + lc*lc - lb*lb) / (2*la*lc);
//    cosb = max(cosb, -1.0);
  //  cosb = min(cosb, 1.0);
    //beta = radToDeg( acos(cosb) );
    //assert(errno == 0);
 // return beta;
//}


//bool isBetween(const double value, const double bound1, const double bound2){
//  return ( (MIN(bound1,bound2)<=value) && (value<=MAX(bound1,bound2)) );
//}



//void Point3::Rotate(const Coord& x, const Coord& y,
//                          const double alpha, Point& res) const{

//  if(!IsDefined()){
  //   res.SetDefined(false);
    // return;
  //}

//  double s = sin(alpha);
  //double c = cos(alpha);

//  double m00 = c;
  //double m01 = -s;
//  double m02 = x - x*c + y*s;
//  double m10 = s;
  //double m11 = c;
  //double m12 = y - x*s-y*c;

 // res.Set(  m00*this->x + m01*this->y + m02,
   //         m10*this->x + m11*this->y + m12);

//}





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




/*
4.2 List Representation

The list representation of a point is

----  (x y)
----

4.3 ~Out~-function

*/
//FIXME: Hier forsetzen
ListExpr
OutPoint3( ListExpr typeInfo, Word value )
{
  Point3* point = (Point3*)(value.addr);
  if( point->IsDefined() )
    //FIXME 
    return nl->ThreeElemList(
               nl->RealAtom( point->GetX() ),
               nl->RealAtom( point->GetY() ),
               nl->RealAtom( point->GetAlpha() ));
  else
    return nl->SymbolAtom( Symbol::UNDEFINED() );
}

/*
4.4 ~In~-function

*/
Word
InPoint3( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  if( nl->ListLength( instance ) == 3 ) {
    ListExpr first = nl->First(instance);
    ListExpr second = nl->Second(instance);
    ListExpr third = nl->Third(instance);
    
    correct = listutils::isNumeric(first) && listutils::isNumeric(second);
    if(!correct){
       return SetWord( Address(0) );
    } else {
      return SetWord(new Point3(true, listutils::getNumValue(first),
                                     listutils::getNumValue(second),
                                     listutils::getNumValue(third)));
    }
  } else if( listutils::isSymbolUndefined( instance ) ){
     return SetWord(new Point3(false));
  }
  correct = false;
  return SetWord( Address(0) );
}

/*
4.5 ~Create~-function

*/
Word
CreatePoint3( const ListExpr typeInfo )
{
  return SetWord( new Point3( false ) );
}

/*
4.6 ~Delete~-function

*/
void
DeletePoint3( const ListExpr typeInfo,
             Word& w )
{
  ((Point3 *)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

/*
4.7 ~Close~-function

*/
void
ClosePoint3( const ListExpr typeInfo,
            Word& w )
{
  ((Point3 *)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

/*
4.8 ~Clone~-function

*/
Word
ClonePoint3( const ListExpr typeInfo,
            const Word& w )
{
  return SetWord( new Point3( *((Point3 *)w.addr) ) );
}

/*
4.8 ~SizeOf~-function

*/
int
SizeOfPoint3()
{
  return sizeof(Point3);
}

/*
4.9 Function describing the signature of the type constructor

*/
ListExpr
Point3Property()
{
  return nl->TwoElemList(
           nl->FourElemList(
             nl->StringAtom("Signature"),
             nl->StringAtom("Example Type List"),
             nl->StringAtom("List Rep"),
             nl->StringAtom("Example List")),
           nl->FourElemList(
             nl->StringAtom("-> DATA"),
             nl->StringAtom(Point::BasicType()),
             nl->StringAtom("(x y a)"),
             nl->StringAtom("(10 5 2)")));
}

/*
4.10 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~point~ does not have arguments, this is trivial.

*/
bool
CheckPoint3( ListExpr type, ListExpr& errorInfo )
{
  return (listutils::isSymbol( type, Point3::BasicType() ));
}

/*
4.11 ~Cast~-function

*/
void* CastPoint3(void* addr)
{
  return (new (addr) Point3());
}

/*
4.12 Creation of the type constructor instance

*/
TypeConstructor point3(
  Point3::BasicType(),                    //name
  Point3Property,              //property function describing signature
  OutPoint3,      InPoint3,     //Out and In functions
  0,             0,           //SaveToList and RestoreFromList functions
  CreatePoint3,   DeletePoint3, //object creation and deletion
  OpenAttribute<Point3>,
  SaveAttribute<Point3>,  // object open and save
  ClosePoint3,    ClonePoint3,  //object close, and clone
  CastPoint3,                  //cast function
  SizeOfPoint3,                //sizeof function
  CheckPoint3);               //kind checking function





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
  //FIXME: almostequal fehlt
  if( AlmostEqual( *pa, *pb ) )
    return 0;

  if( *pa < *pb )
    return -1;

  return 1;
}



// // use this when adding and sorting the DBArray
// int PointHalfSegmentCompare( const void *a, const void *b )
// {
//   const Point *pa = (const Point *)a;
//   const HalfSegment *hsb = (const HalfSegment *)b;
//
//   if( *pa == hsb->GetDomPoint() )
//     return 0;
//
//   if( *pa < hsb->GetDomPoint() )
//     return -1;
//
//   return 1;
// }



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



