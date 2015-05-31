/*
bla

*/
#ifndef __UMOVE_H
#define __UMOVE_H

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "NestedList.h"
//#include "DBArray.h"
#include "../../Tools/Flob/DbArray.h"
#include "Progress.h"
//#include "CellGrid.h"
#include "TemporalAlgebra.h"
#include "Point3.h"

#include "RectangleAlgebra.h"
#include "DateTime.h"
#include "AlmostEqual.h"
#include "Geoid.h"
#include "ListUtils.h"
#include "../../include/CharTransform.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace datetime;


class SecInterval;
/*
3.8 UMove

This class will be used in the ~upoint~ type constructor, i.e., the type constructor
for the temporal unit of point values.

*/
class UMove : public SpatialTemporalUnit<Point3, 4>
{
/*
3.8.1 Constructors and Destructor

*/
  public:

  UMove() {};

  UMove(bool is_defined):
    SpatialTemporalUnit<Point3, 4>(is_defined)
  {
  };
  UMove( const Interval<Instant>& _interval,
          const double x0, const double y0, const double alpha0,
          const double x1, const double y1, const double alpha1 ):
    SpatialTemporalUnit<Point3, 4>( _interval ),
    p0( true, x0, y0, alpha0),
    p1( true, x1, y1, alpha1)
    { }

  UMove( const Interval<Instant>& _interval,
          const Point3& _p0, const Point3& _p1 ):
    SpatialTemporalUnit<Point3, 4>( _interval ),
    p0( _p0 ),
    p1( _p1 )
    {
      SetDefined(p0.IsDefined() && p1.IsDefined());
    }

  UMove(const UMove& source):
    SpatialTemporalUnit<Point3, 4>(source.IsDefined()) {
     *((TemporalUnit<Point3>*)this) = *((TemporalUnit<Point3>*)&source);
     p0 = source.p0;
     p1 = source.p1;
     del.refs=1;
     del.SetDelete();
     del.isDefined = source.del.isDefined;
  }

/*
3.6.2 Operator redefinitions

*/

  virtual UMove& operator=( const UMove& i )
  {
    del.isDefined = i.del.isDefined;
    if( !i.IsDefined() ){
      return *this;
    }
    *((TemporalUnit<Point3>*)this) = *((TemporalUnit<Point3>*)&i);
    p0 = i.p0;
    p1 = i.p1;
    return *this;
  }

/*
~GetNoComponents~

Returns the constant number 1. This function allows for
templates using UMove and MPoint3.

*/
  int GetNoComponents() const{
     return 1;
  }


/*
~Get~


*/

    void Get( const int i, UMove& upi ) const{
      assert(i==0);
      upi = *this;
    }



/*
Redefinition of the copy operator ~=~.

*/

  virtual bool operator==( const UMove& i ) const
  {
    if( !this->IsDefined() && !i.IsDefined() ){
      return true;
    }
    return this->IsDefined() && i.IsDefined()
        && *((TemporalUnit<Point3>*)this) == *((TemporalUnit<Point3>*)&i)
        && AlmostEqual( p0, i.p0 )
        && AlmostEqual( p1, i.p1 );
  }
/*
Returns ~true~ if both units are undefined, or if both are defined and this temporal unit is equal to the temporal unit ~i~ and ~false~ if they are different.

*/

  virtual bool operator!=( const UMove& i ) const
  {
    return !( *this == i );
  }
/*
Returns ~true~ if this temporal unit is different to the temporal unit ~i~ and ~false~ if they are equal.

*/

/*
3.8.2 The Temporal Functions

*/

  virtual void TemporalFunction( const Instant& t,
                                 Point3& result,
                                 bool ignoreLimits = false ) const;
  void TemporalFunction( const Instant& t,
                         Point3& result,
                         const Geoid* geoid,
                         bool ignoreLimits = false) const;
  virtual bool Passes( const Point3& val ) const;
  bool Passes( const Point3& val, const Geoid* geoid ) const;
  bool Passes( const Region& val ) const;
  bool Passes( const Rectangle<3>& rect ) const;
  virtual bool At( const Point3& val, TemporalUnit<Point3>& result ) const;
  bool At( const Point3& val, TemporalUnit<Point3>& result,
           const Geoid* geoid ) const;
  virtual void AtInterval( const Interval<Instant>& i,
                           TemporalUnit<Point3>& result ) const;
  void AtInterval( const Interval<Instant>& i,
                   TemporalUnit<Point3>& result,
                   const Geoid* geoid ) const;
  void At(const Rectangle<3>& rect, UMove& result) const;

/*
Computes the temporal distance to a Point3 ~p~. Result is a single unit.

Only to be used with euclidean coordinate data!

*/
  void Distance( const Point3& p, UReal& result ) const;


/*
Computes the temporal distance to a Point3 ~p~. All result units will be appended
to the ~result~ vector of UReal values. If ~geoid~ in NULL, euclidean geometry is
applied and only a single result unit is appended. Otherwise, spherical geometry
is used ~result~ is appended with a series uf UReal units approximating the
actual moving distance.

UNDEFINED UReal values may be appended!.

*/
  void Distance( const Point3& p, vector<UReal>& result,
                 const Geoid* geoid = 0, const double epsilon = 0.00001 ) const;

/*
Computes the temporal distance to a Point3 ~p~ using an iterative method. The
result is passed in output parameter ~result~. The vector may contain UNDEFINED
units.

The function works recursively. It is possible to pass down pre-computed data
using parameters ~tMin~ (instant of nearest approach), ~distMin~ (distance at
nearest approach), ~distStart~ (distance at initial instant of THIS unit),
~distEnd~ (distance at final instant of THIS unit). Doing so avoids repeated
calculation of these values. The default parameters make sure, that the values
will be computed once.

Computation stops, once the absolute distance difference between consecutive
steps drops below ~epsilon~.

*/
  void DistanceOrthodrome( const Point3& p, vector<UReal>& result,
                           const Geoid geoid,
                           const double epsilon  = 0.00001,
                           const Instant* tMin   = 0,
                           const double distMin  =-666.666,
                           const double distStart=-666.666,
                           const double distEnd  =-666.666            ) const;
/*
Computes the spatial projection of THIS UMove.

*/
  void UTrajectory( Line& line ) const;

/*
The scalar velocity as a temporal function
If geoid = 0, metric (X,Y)-coordinates are used within the UMove.
If geoid points to a valid Geoid object, geografic coordinates (LON,LAT)
are used within the UMove. Speed is interpreted as speed over ground!

*/
  void USpeed( UReal& result, const Geoid* geoid = 0 ) const;

/*
The vectorial velocity --- (X,Y)-components --- as temporal function

*/
  void UVelocity( UMove& result ) const;
  void Intersection(const UMove &other, UMove &result) const;

/*
~EqualValue~ returns true iff both units describe parts if the same linear
temporal function.

*/
 virtual bool EqualValue( const TemporalUnit<Point3>& i) const{
   return EqualValue(* ((UMove*) &i));
 }

  virtual bool EqualValue( const UMove& i ) const {
    if( !IsDefined() && !i.IsDefined() ){
      // both undefined
//       cout << "\t" << __PRETTY_FUNCTION__ << " SUCCEEDED: both undefined."
//            << endl;
      return true;
    } else if( !IsDefined() || !i.IsDefined() ){
      // one of *this and i undefined
//       cout << "\t" << __PRETTY_FUNCTION__ << " FAILED: one undefined."
//            << endl;
      return false;
    } // else: both are defined
    Point3 v(false);
    TemporalFunction(i.timeInterval.start, v, true);
    if( !v.IsDefined() || !AlmostEqual(i.p0,v) ){
//       cout << "\t" << __PRETTY_FUNCTION__ << " FAILED: start1 unmatched."
//            << endl;
      return false;
    }
    TemporalFunction(i.timeInterval.end, v, true);
    if( !v.IsDefined() || !AlmostEqual(i.p1,v) ){
//       cout << "\t" << __PRETTY_FUNCTION__ << " FAILED: end1 unmatched."
//            << endl;
//       cout << "\t" << __PRETTY_FUNCTION__ << " i.p1 = " << i.p1 << endl;
//       cout << "\t" << __PRETTY_FUNCTION__ << " v    = " << v    << endl;
      return false;
    }
    TemporalFunction(timeInterval.start, v, true);
    if( !v.IsDefined() || !AlmostEqual(p0,v) ){
//       cout << "\t" << __PRETTY_FUNCTION__ << " FAILED: start2 unmatched."
//            << endl;
      return false;
    }
    TemporalFunction(timeInterval.end, v, true);
    if( !v.IsDefined() || !AlmostEqual(p1,v) ){
//       cout << "\t" << __PRETTY_FUNCTION__ << " FAILED: end2 unmatched."
//            << endl;
      return false;
    }
//       cout << "\t" << __PRETTY_FUNCTION__
//            << " SUCCEEDED: all points matched." << endl;
    return true;
  }

  virtual bool Merge( const TemporalUnit<Point3>& i){
    return Merge(*((UMove*) &i));
  }

  virtual bool Merge( const UMove& i ) {
    // check for gap in definition time
    if(!IsDefined() && !i.IsDefined()) { // mergeable, but nothing to do
//       cout << __PRETTY_FUNCTION__ << " SUCCEEDED: both undefined." << endl;
      return true;
    } else if(!IsDefined() || !i.IsDefined()) { // not mergable
//       cout << __PRETTY_FUNCTION__ << " FAILED: one undefined." << endl;
      return false;
    } else if(    !this->timeInterval.Adjacent(i.timeInterval)
               && !this->timeInterval.Intersects(i.timeInterval) ){
//       cout << __PRETTY_FUNCTION__ << " FAILED: found definition gap."
//            << endl;
      return false; // have a gap in between --> not mergeable
    } else if(!EqualValue(i)) { // temporal functions are NOT equal
//       cout << __PRETTY_FUNCTION__ << " FAILED: functions not equal." << endl;
      return false;
    }
    // merge the units (i.e. their timeIntervals)
    UMove res(false);
    if(StartsBefore(i)){
      res.timeInterval.start = this->timeInterval.start;
      res.timeInterval.lc    = this->timeInterval.lc;
      res.p0                 = this->p0;
    } else {
      res.timeInterval.start = i.timeInterval.start;
      res.timeInterval.lc    = i.timeInterval.lc;
      res.p0                 = i.p0;
    }
    if(EndsAfter(i)){
      res.timeInterval.end   = this->timeInterval.end;
      res.timeInterval.rc    = this->timeInterval.rc;
      res.p1                 = this->p1;
    } else {
      res.timeInterval.end   = i.timeInterval.end;
      res.timeInterval.rc    = i.timeInterval.rc;
      res.p1                 = i.p1;
    }
    if(res.IsDefined() && res.IsValid()){ // invalid result -- do nothing!
//       cout << __PRETTY_FUNCTION__ << " SUCCEEDED: created result unit."
//            << endl;
      *this = res;
      return true;
    } else {
//       cout << __PRETTY_FUNCTION__ << " FAILED: result invalid." << endl;
      return false;
    }
  }
/*
Merges UMove ~i~ into this unit if possible and return ~true~. Otherwise do
not modify this unit and return ~false~.

*/

  void Translate(const double x, const double y,const double alpha,
                 const DateTime& duration);
/*
Translates a moving point spatially and temporally.

*/

  void GetGridCellSequence(CellGrid2D &g, vector<GridCellSeq> &res);
/*
Computes all events created by a UMove moving across a regular grid.

*/

/*
3.8.3 Functions to be part of relations

*/

  inline virtual size_t Sizeof() const
  {
    return sizeof( *this );
  }

  inline virtual int Compare( const Attribute* arg ) const
  {
    UMove* up2 = (UMove*) arg;
    if (!IsDefined() && !up2->IsDefined())
      return 0;
    if (!IsDefined())
      return -1;
    if (!up2->IsDefined())
      return 1;

    int cmp = timeInterval.CompareTo(up2->timeInterval);
    if(cmp){
       return cmp;
    }
    if(p0<up2->p0){
      return -1;
    }
    if(p0>up2->p0){
      return 1;
    }
    if(p1>up2->p1){
       return 1;
    }
    if(p1<up2->p1){
       return -1;
    }
    return 0;
  }

  inline virtual bool Adjacent( const Attribute* arg ) const
  {
    UMove* up = (UMove*) arg;
    if(timeInterval.end == up->timeInterval.start){ // up after this
       return AlmostEqual(p1, up->p0) &&
             (timeInterval.rc || up->timeInterval.lc);
    }
    if(up->timeInterval.end == timeInterval.start){
       return AlmostEqual(up->p1,p0) &&
              (up->timeInterval.rc || timeInterval.lc);
    }
    return false;
  }

  inline virtual ostream& Print( ostream &os ) const
  {

    if(IsDefined())
      {
        os << "UMove: " << "( ";
        timeInterval.Print(os);
        os << ", ";
        p0.Print(os);
        os << ", ";
        p1.Print(os);
        os << " ) ";
        return os;
      }
    else
      return os << "UMove: (undef) ";
  }

  inline virtual size_t HashValue() const
  {
    if(!IsDefined()){
      return 0;
    }
    return static_cast<size_t>(   timeInterval.start.HashValue()
                                ^ timeInterval.end.HashValue()   ) ;
  }

  inline virtual UMove* Clone() const
  {
    UMove *res;
    if(this->IsDefined()){
      res = new UMove( timeInterval, p0, p1 );
      res->del.isDefined = del.isDefined;
    } else {
      res = new UMove( false );
//       res->timeInterval = Interval<Instant>();
      res->p0 = Point3( false, 0.0, 0.0, 0.0);
      res->p1 = Point3( false, 0.0, 0.0, 0.0);
    }
    return res;
  }

  inline virtual void CopyFrom( const Attribute* right )
  {
    const UMove* i = static_cast<const UMove*>(right);

    if(i->del.isDefined)
      {
        timeInterval.CopyFrom( i->timeInterval );
        p0 = i->p0;
        p1 = i->p1;
      }
    else
      {
//         timeInterval = Interval<Instant>();
        p0 = Point3( false, 0.0, 0.0, 0.0);
        p1 = Point3( false, 0.0, 0.0, 0.0);
      }
    del.isDefined = i->del.isDefined;
  }

  virtual const Rectangle<4> BoundingBox(const Geoid* geoid = 0) const
  {
    if(this->IsDefined()){
      return Rectangle<4>( true, MIN( p0.GetX(), p1.GetX() ),
                                 MAX( p0.GetX(), p1.GetX() ),
                                 MIN( p0.GetY(), p1.GetY() ),
                                 MAX( p0.GetY(), p1.GetY() ),
                                 MIN( p0.GetAlpha(), p1.GetAlpha() ),
                                 MAX( p0.GetAlpha(), p1.GetAlpha() ),
                                 timeInterval.start.ToDouble(),
                                 timeInterval.end.ToDouble() );
    } else {
      return Rectangle<4>( false );
    }
  }
  
  static unsigned GetDim(){ return 3; }


  virtual const Rectangle<4> BoundingBox(const double scaleTime,
                                         const Geoid* geoid = 0) const
  {
    Rectangle<4> bbx = this->BoundingBox(geoid);
    if(bbx.IsDefined()){
      return Rectangle<4>( true, bbx.MinD(0),
                                 bbx.MaxD(0),
                                 bbx.MinD(1),
                                 bbx.MaxD(1),
                                 bbx.MinD(2),
                                 bbx.MaxD(2),
                                 timeInterval.start.ToDouble()*scaleTime,
                                 timeInterval.end.ToDouble()*scaleTime );
    } else {
      return Rectangle<4>( false );
    }
  }

  const Rectangle<3> BoundingBoxSpatial(const Geoid* geoid = 0) const
  {
    Rectangle<4> bbx = this->BoundingBox(geoid);
    if(bbx.IsDefined()){
      return Rectangle<3>( true, bbx.MinD(0),
                           bbx.MaxD(0),
                           bbx.MinD(1),
                           bbx.MaxD(1),
                           bbx.MinD(2),
                           bbx.MaxD(2) );
    } else {
      return Rectangle<3>( false );
    }
  }

/*
Calculates the distance between 2 upoints as a real value.
If ~geoid~ is NULL, euclidean geometry is used, spherical geometry otherwise.
If invalid geographic coordinates are found, the result is UNDEFINED.

*Precondition*: intersecting timeIntervals.

*Result*: the distance of two upoints as a ureal

*/

  void Distance( const UMove& up, UReal& result, const Geoid* geoid = 0) const;

/*
Computes the distance between the three dimensional line defined by
that unit and the rectangle.

*/
virtual double Distance(const Rectangle<4>& rect, const Geoid* geoid=0) const;

virtual bool Intersects(const Rectangle<4>& rect, const Geoid* geoid=0) const;


  virtual bool IsEmpty() const{
    return !IsDefined();
 }

/*
Calculates the spatial length of the unit using metric (X,Y) coordinates.

*Precondition*: none

*Result*: the distance of the unit's initial and final value

*/
void Length( CcReal& result ) const;


/*
Calculates the spatial length of the unit using geographic (LON,LAT)-coordinates.

*Precondition*: none

*Result*: the distance of the unit's initial and final value

*/
void Length( const Geoid& g, CcReal& result ) const;

/*
Calculates the intersection of the UMove and Region r.

*Precondition*: none

*Result*: The ~result~ is a vector of UMoves, sorted ascendingly by their starting instants.
The boolean return value is ~false~, iff either the UMove or the Region is UNDEFINED.

*/

  bool AtRegion(const Region *r, vector<UMove> &result) const;

/*
Calculates the ~direction~ (when ~useHeading~ is ~false~) resp. the heading
(when ~true~) of this UMove. If ~geoid~ is not NULL, spherical geometry is
applied and the true course is approximated as a series of units according
to preciseness parameter ~epsilon~.

Any results are appended to the ~result~ vector.
Attention: UNDEFINED units my be appended!

*/
  void Direction( vector<UReal> &result,
                  const bool useHeading = false,
                  const Geoid* geoid    = 0,
                  const double epsilon  = 0.0000001) const;

  static const string BasicType(){ return "upoint"; }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }

/*
~IsStatic~

Returns true, iff this unit is defined and not moving during its definition time.

*/
   bool IsStatic() const{

      return IsDefined() && AlmostEqual(p0,p1);

   }

/*
3.8.4 Attributes

*/
  Point3 p0, p1;
};


ostream& operator<<(ostream& o, const UMove& u);
ListExpr OutUMove( ListExpr typeInfo, Word value );
Word InUMove( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct );
 

#endif