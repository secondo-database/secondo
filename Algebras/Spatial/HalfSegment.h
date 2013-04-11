
/*
7 Class ~HalfSegment~

This class implements the memory representation of  ~halfsegment~. Although ~halfsegment~
is not an independent type constructor, it is the basic construction unit of the ~line~ and the ~region~
type constructors.

A ~halfsegment~ value is composed of a pair of points and a flag indicating the dominating
point. The left point is always smaller than the right one.

*/

#ifndef _HALFSEGMENT_H
#define _HALFSEGMENT_H


#include <math.h>
#include <cmath>
#include "Coord.h"
#include "Point.h"
#include "AttrType.h"
#include "Geoid.h"

/*
5.13 Auxiliary Funktions

*/
class HalfSegment; // forward declaration

inline bool AlmostEqual( const HalfSegment& hs1, const HalfSegment& hs2 );

inline double ApplyFactor( const double d );

class HalfSegment;
/*
5.14 Overloaded output operator

*/
ostream& operator<<( ostream& o, const HalfSegment& hs );

class HalfSegment
{
  public:

/*
5.1 Constructors and Destructor

A half segment is composed by two points which are called ~left point~ (~lp~) and ~right point~ (~rp~),
$lp < rp$, and a flag ~ldp~ (~left dominating point~) which tells whether the left point is the dominating
point.

This constructor should not be used:

*/

    inline HalfSegment() {}
/*
Creates a half segment receiving all attributes as arguments. The order between the left
and right points is not important. If ~lp~ is bigger than ~rp~ their values are changed.

*/

    inline HalfSegment( bool ldp, const Point& lp, const Point& rp );
/*
Creates a half segment copying the values from ~hs~.

*/

    inline HalfSegment( const HalfSegment& hs );
/*
The destructor.

*/
    inline ~HalfSegment() {}
/*
5.2 Member Functions

Returns the left point of the half segment.

*/
    inline const Point& GetLeftPoint() const;
/*
Returns the right point of the half segment.

*/
    inline const Point& GetRightPoint() const;
/*
Returns the dominating point of the half segment.

*/
    inline const Point& GetDomPoint() const;
/*
Returns the secondary point of the half segment.

*/
    inline const Point& GetSecPoint() const;
/*
Returns the boolean flag which indicates whether the dominating point is on the left side.

*/
    inline bool IsLeftDomPoint() const;
/*
Returns the bounding box of the half segment. If ~geoid~ is not NULL, the geographic
MBR is returned. If ~geoid~ is UNDEFINED, the result is UNDEFINED.

*/
    inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;
/*
Returns the "attr" value associated with a half segment. The "attr" value is useful when we
process region values.

*/
    inline const AttrType& GetAttr() const;
/*
Returns the length of the half segmtent, i.e., the distance between the left point to the
right point.

*/
    inline double Length(const Geoid* geoid=0) const;
/*
Returns the length of the orthodrome defined by the half segment's end points
with respect to the provided geoid.
Coordinates of the point values must be in format (LON,LAT), where
-180<=LON<=180, -90<=LAT<=90.

If an undefined Point or a Point with an invalid geographic coordinate is used,
~valid~ is set to false, otherwise the result is calculated and ~valid~ is set
to true.

*/

    inline double LengthOrthodrome(const Geoid& g, bool& valid) const;
/*
Returns the point at relative position ~pos~.

*/
    inline Point AtPosition( double pos, const Geoid* geoid=0 ) const;
/*
Returns the relative position of the point ~p~.

*/
    inline double AtPoint( const Point& p ) const;
/*
Returns the sub half segment trimmed by ~pos1~ and ~pos2~.

*/
    inline bool SubHalfSegment( double pos1, double pos2,
                                HalfSegment& result ) const;
/*
Sets the value of a half segment. The parameters ~lp~ and ~rp~ can ignore the order, and the
function will compare the parameter points and put the smaller one to ~lp~ and larger one to ~rp~.

*/
    void Set( bool ldp, const Point& lp, const Point& rp );
/*
Translates the half segment by adding the coordinates ~x~ and ~y~ (which can be negative) to both
~lp~ and ~rp~ points.

*/
    void Translate( const Coord& x, const Coord& y );

/*
Scales the half segment given a factor ~f~.

*/

    inline void Scale( const Coord& f )
    {
      lp.Scale( f );
      rp.Scale( f );
    }
/*
Sets the value of the "attr" attribute of a half segment.

*/
    inline void SetAttr( AttrType& attr );
/*
Sets the value of the dominating point flag of a half segment.

*/
    inline void SetLeftDomPoint( bool ldp );
/*
Checks whethet the HalsSegment is vertical

*/

    inline bool IsVertical()const;


    HalfSegment& operator=( const HalfSegment& hs );
    bool operator==( const HalfSegment& hs ) const;
    bool operator!=( const HalfSegment& hs ) const;
    bool operator<(const HalfSegment& hs) const;
    bool operator<=(const HalfSegment& hs) const;
    bool operator>(const HalfSegment& hs) const;
    bool operator>=(const HalfSegment& hs) const;




/*
~insideLeft~

This function converts the insideAbove flag into a 
insideLeft flag. The halfsegment is treated as directed
from the dominating point to the secondary point. 

*/
  bool insideLeft() const;


/*
~middlePoint~

Returns the point of this halfsegment located at the middle.

*/
  Point middlePoint() const{
    return Point(true, (rp.GetX() + lp.GetX()) / 2.0 ,
                       (rp.GetY() + lp.GetY()) / 2.0);
  }


/*
~Print~

The usual Print function

*/
   ostream& Print(ostream& out) const{
     return out << (*this) << endl;
   }

/*
~PrintAsLine~

Prints out this segments as a nested list reprenting a line.


*/

  ostream& PrintAsLine(ostream& out) const{
     Point P1 = GetDomPoint();
     Point P2 = GetSecPoint();
     out << "(line ((" << P1.GetX() << " " << P1.GetY() << " "
         << P2.GetX() << " "  << P2.GetY() << ")))";
     return out;
  }

  string getLineString() const{
    stringstream ss;
    ss << setprecision(16);
    PrintAsLine(ss);
    return ss.str();
  }



/*
SimpleString

Returns a segmnet representation (x1,y1) -> (x2,y2) where 
(x1,y1) is the domination point and (x2,y2) is the secondary point.

*/
   string SimpleString() const{
     stringstream ss;
     ss << GetDomPoint() << " -> " << GetSecPoint();
     return ss.str();
   }



/*
Operator redefinitions.

This function make comparison between two halfsegments. The rule of the
comparison is specified in the ROSE Algebra paper. That is:  the half sgenments
will be ordered according to the following values:
dominating points -\verb+>+  LDP flages  -\verb+>+ directions (rotations).

*/
    int Compare( const HalfSegment& hs ) const;
/*
Decides whether two half segments intersect with each other with any kind of
intersection.
Applies spherical geometry, iff ~geoid~ ist not NULL. If invalid coordinates
are found, ~false~ is returned.

*/
    bool Intersects( const HalfSegment& hs, const Geoid* geoid = 0 ) const;
/*
Decides whether two half segments intersect in the following manner: a point of
the first segment and an
innerpoint of the second segment are the same.

*/
    bool InnerIntersects( const HalfSegment& hs, const Geoid* geoid=0 ) const;
/*
Computes whether two half segments intersect in their mid-points. Endpoints are
not considered in computing the results.

*/
    bool Crosses( const HalfSegment& hs, const Geoid* geoid=0 ) const;
/*
This function computes whether two half segments cross each other and returns
the crossing point ~p~.
Applies spherical geometry, iff ~geoid~ ist not NULL. If invalid coordinates are
found, ~false~ is returned and ~p~ is set to UNDEFINED.

*/

    bool Intersection( const HalfSegment& hs, Point& p,
                       const Geoid* geoid = 0 ) const;
/*
This function computes whether two half segments intersect each other and
returns the resulting halfsegment as ~reshs~.
Applies spherical geometry, iff ~geoid~ ist not NULL. If invalid coordinates are
found, ~false~ is returned .

*/
    bool Intersection( const HalfSegment& hs, HalfSegment& reshs,
                       const Geoid* geoid = 0 ) const;
/*
Implements the Cohen and Sutherland algorithm for clipping a segment to a clipping window.

*/
   void CohenSutherlandLineClipping( const Rectangle<2> &window,
                                     double &x0, double &y0,
                                     double &x1, double &y1,
                                     bool &accept,
                                     const Geoid* geoid=0) const;
/*
Computes the part of the segment that is inside the rectangle ~window~, if
it exists. The ~inside~ parameter is set to true if there is a partion of the segment
inside the window. The ~isIntersectionPoint~ parameter is set to true if the intersection
part of the segment is a point instead of a segment, and if so, ~intersectionPoint~
receives the intersection point.

*/
   void WindowClippingIn( const Rectangle<2> &window,
                          HalfSegment &hs,
                          bool &inside,
                          bool &isIntersectionPoint,
                          Point &intersectionPoint,
                          const Geoid* geoid=0) const;
/*
Computes whether the half segment is inside the one in ~hs~.

*/
    bool Inside( const HalfSegment& hs, const Geoid* geoid=0 ) const ;
/*
Computes whether the point ~p~ is contained in the half segment.
Uses the ~AlmostEqual~ function.

*/
    bool Contains( const Point& p, const Geoid* geoid=0 ) const;
/*
Decides whether a half segment is above a point. This is useful when we want to decide
whether a point is inside a region.

*/
    bool RayAbove( const Point& p, double &abovey0 ) const;
/*
Decides whether a half segment is below a point. This is useful when we want to decide
whether a point is inside a region.

*/

    bool RayDown(const Point& p, double &yIntersection) const;

/*
Computes the distance from the half segment to the point ~p~.
If ~geoid~ is not NULL, it is used to compute the minimun distance
between ~p~ and ~[*]this~ being interpreted as a orthodrome relative to reference
ellipsoid ~[*]geoid~ and using spherical geometry.

A negative result means, that an error occured during computation (e.g. invalid
geographic ccordinates).

*/

    double Distance( const Point& p, const Geoid* geoid = 0 ) const;

/*
Computes the minimum distance from two half segments.
If ~geoid~ is not NULL, it is used to compute the minimun distance
between both ~hs~ and ~[*]this~ being interpreted as orthodromes relative to
~[*]geoid~ and using spherical geometry.

A negative result means, that an error occured during computation (e.g. invalid
geographic ccordinates).

*/
    double Distance( const HalfSegment& hs, const Geoid* geoid = 0 ) const;
/*
Compares two half segments according to their attribute values (~attr~).

*/

    double Distance(const Rectangle<2>& rect, const Geoid* geoid=0) const;
    bool Intersects(const Rectangle<2>& rect, const Geoid* geoid=0) const;
    double MaxDistance(const Rectangle<2>& rect, const Geoid* geoid=0) const;
    int LogicCompare( const HalfSegment& hs ) const;
/*
Used in the Plane Sweep Algebra

*/
    bool innerInter( const HalfSegment& chs,  Point& resp,
                     HalfSegment& rchs, bool& first, bool& second ) const;
/*

*/

  private:

/*
5.13 Attributes

Indicates which is the dominating point: the left or the right point.

*/
    bool ldp;
/*
These two attributes give the left and right point of the half segment.

*/
    Point lp;
    Point rp;
/*

*/
  public:
/*
This ~attribute~ property is useful if we process region values in the way indicated in the ROSE
paper.

*/

    AttrType attr;
};

/*
6 Implementation of Inline Functions of Class ~HalfSegment~

*/
inline
HalfSegment::HalfSegment( bool ldp,
                          const Point& lp,
                          const Point& rp ):
ldp( ldp ),
lp( lp ),
rp( rp ),
attr(-99999)
{
  assert(lp.IsDefined());
  assert(rp.IsDefined());
  assert( !AlmostEqual( lp, rp ) );

  if( lp > rp )
  {
    this->lp = rp;
    this->rp = lp;
  }
}

inline
HalfSegment::HalfSegment( const HalfSegment& hs ):
ldp( hs.ldp ),
lp( hs.lp ),
rp( hs.rp ),
attr( hs.attr )
{
  assert(lp.IsDefined());
  assert(rp.IsDefined());
}

inline const Point&
HalfSegment::GetLeftPoint() const
{
  return lp;
}

inline const Point&
HalfSegment::GetRightPoint() const
{
  return rp;
}



inline const Point&
HalfSegment::GetDomPoint() const
{
  if( ldp )
    return lp;
  return rp;
}

inline const Point&
HalfSegment::GetSecPoint() const
{
  if( ldp )
    return rp;
  return lp;
}

inline bool
HalfSegment::IsLeftDomPoint() const
{
  return ldp;
}

inline void
HalfSegment::SetLeftDomPoint( bool ldp )
{
  this->ldp = ldp;
}

inline bool HalfSegment::IsVertical()const{
  return AlmostEqual(lp.GetX(),rp.GetX());
}


inline const Rectangle<2>
HalfSegment::BoundingBox(const Geoid* geoid /*=0*/) const
{
  if(geoid){ // spherical geometry case
    Rectangle<2> geobbox = lp.GeographicBBox(rp, *geoid); // handles UNDEFINED
    double minx = geobbox.MinD(0) - ApplyFactor(geobbox.MinD(0));
    double maxx = geobbox.MaxD(0) + ApplyFactor(geobbox.MaxD(0));
    double miny = geobbox.MinD(1) - ApplyFactor(geobbox.MinD(1));
    double maxy = geobbox.MaxD(1) + ApplyFactor(geobbox.MaxD(1));
    return Rectangle<2>( true,
                         (minx>=-180.0)?minx:-180.0,
                         (maxx<= 180.0)?maxx: 180.0,
                         (miny>=-90.0)?miny:-90.0,
                         (maxy<= 90.0)?maxy: 90.0);
  } // else: euclidean case
  double minx = MIN( GetLeftPoint().GetX(), GetRightPoint().GetX() ),
         maxx = MAX( GetLeftPoint().GetX(), GetRightPoint().GetX() ),
         miny = MIN( GetLeftPoint().GetY(), GetRightPoint().GetY() ),
         maxy = MAX( GetLeftPoint().GetY(), GetRightPoint().GetY() );

  return Rectangle<2>( true,
                       minx - ApplyFactor(minx),
                       maxx + ApplyFactor(maxx),
                       miny - ApplyFactor(miny),
                       maxy + ApplyFactor(maxy) );
}

inline const AttrType&
HalfSegment::GetAttr() const
{
  return attr;
}

inline void
HalfSegment::SetAttr( AttrType& attr )
{
  this->attr = attr;
}

inline double
HalfSegment::Length(const Geoid* geoid /*=0*/) const
{
  return rp.Distance( lp, geoid );
}

inline double
HalfSegment::LengthOrthodrome(const Geoid& g, bool& valid) const
{
  double bearInitial = 0, bearFinal = 0;
  double d = rp.DistanceOrthodromePrecise(lp, g, valid, bearInitial, bearFinal);
  return d;
  // return rp.DistanceOrthodrome( lp, g, valid );
}


inline Point
HalfSegment::AtPosition( double pos, const Geoid* geoid/*=0*/ ) const
{
  if( pos < 0 || AlmostEqual( pos, 0 ) ){
    return GetDomPoint();
  }
  double l = Length(geoid);
  if( pos > l ||  AlmostEqual( pos, l ) ){
    return GetSecPoint();
  }
  if(geoid){
    cerr<< __PRETTY_FUNCTION__ << "Sperical geometry not implemented." << endl;
    assert(false);
    return Point(false);
  } else {
    return Point( true,
                GetDomPoint().GetX() + pos *
                  (GetSecPoint().GetX() - GetDomPoint().GetX()) / l,
                GetDomPoint().GetY() + pos *
                  (GetSecPoint().GetY() - GetDomPoint().GetY()) / l );
  }
}

inline double
HalfSegment::AtPoint( const Point& p ) const
{
  assert( p.IsDefined() );
  assert( Contains( p ) );
  if( AlmostEqual( rp.GetX(), lp.GetX() ) &&
      AlmostEqual( p.GetX(), rp.GetX() ) ){
    // the segment is vertical
    return Length() * (p.GetY() - GetDomPoint().GetY()) /
                      (GetSecPoint().GetY() - GetDomPoint().GetY());
  }
  return Length() * (p.GetX() - GetDomPoint().GetX()) /
                    (GetSecPoint().GetX() - GetDomPoint().GetX());
}

inline bool
HalfSegment::SubHalfSegment( double pos1, double pos2,
                                         HalfSegment& result ) const
{
  if( AlmostEqual( AtPosition( pos1 ), AtPosition( pos2 ) ) ){
    return false;
  }
  result.Set( true, AtPosition( pos1 ), AtPosition( pos2 ) );
  return true;
}

/*
1.1 Auxiliary Function Implementation

*/
inline bool AlmostEqual( const HalfSegment& hs1, const HalfSegment& hs2 )
{
  return AlmostEqual( hs1.GetDomPoint(), hs2.GetDomPoint() ) &&
  AlmostEqual( hs1.GetSecPoint(), hs2.GetSecPoint() );
}

inline double ApplyFactor( const double d )
{
  if( fabs(d) <= 10.0 )
    return FACTOR * fabs(d);
  return FACTOR;
}

#endif

