
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


#include "Coord.h"
#include "Point.h"
#include "AttrType.h"
#include "Geoid.h"

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

*/
    inline HalfSegment() {}
/*
This constructor should not be used.

*/
    inline HalfSegment( bool ldp, const Point& lp, const Point& rp );
/*
Creates a half segment receiving all attributes as arguments. The order between the left
and right points is not important. If ~lp~ is bigger than ~rp~ their values are changed.

*/
    inline HalfSegment( const HalfSegment& hs );
/*
Creates a half segment copying the values from ~hs~.

*/
    inline ~HalfSegment() {}
/*
The destructor.

5.2 Member Functions

*/
    inline const Point& GetLeftPoint() const;
/*
Returns the left point of the half segment.

*/
    inline const Point& GetRightPoint() const;
/*
Returns the right point of the half segment.

*/
    inline const Point& GetDomPoint() const;
/*
Returns the dominating point of the half segment.

*/
    inline const Point& GetSecPoint() const;
/*
Returns the secondary point of the half segment.

*/
    inline bool IsLeftDomPoint() const;
/*
Returns the boolean flag which indicates whether the dominating point is on the left side.

*/
    inline const Rectangle<2> BoundingBox() const;
/*
Returns the bounding box of the half segment.

*/
    inline const AttrType& GetAttr() const;
/*
Returns the "attr" value associated with a half segment. The "attr" value is useful when we
process region values.

*/
    inline double Length() const;
/*
Returns the length of the half segmtent, i.e., the distance between the left point to the
right point.

*/

    inline double LengthOrthodrome(const Geoid& g, bool& valid) const;
/*
Returns the length of the orthodrome defined by the half segment's end points
with respect to the provided geoid.
Coordinates of the point values must be in format (LON,LAT), where
-180<=LON<=180, -90<=LAT<=90.

If an undefined Point or a Point with an invalid geographic coordinate is used,
~valid~ is set to false, otherwise the result is calculated and ~valid~ is set
to true.

*/
    inline Point AtPosition( double pos ) const;
/*
Returns the point at relative position ~pos~.

*/
    inline double AtPoint( const Point& p ) const;
/*
Returns the relative position of the point ~p~.

*/
    inline bool SubHalfSegment( double pos1, double pos2,
                                HalfSegment& result ) const;
/*
Returns the sub half segment trimmed by ~pos1~ and ~pos2~.

*/
    void Set( bool ldp, const Point& lp, const Point& rp );
/*
Sets the value of a half segment. The parameters ~lp~ and ~rp~ can ignore the order, and the
function will compare the parameter points and put the smaller one to ~lp~ and larger one to ~rp~.

*/
    void Translate( const Coord& x, const Coord& y );

/*
Translates the half segment by adding the coordinates ~x~ and ~y~ (which can be negative) to both
~lp~ and ~rp~ points.

*/

    inline void Scale( const Coord& f )
    {
      lp.Scale( f );
      rp.Scale( f );
    }
/*
Scales the half segment given a factor ~f~.

*/
    inline void SetAttr( AttrType& attr );
/*
Sets the value of the "attr" attribute of a half segment.

*/
    inline void SetLeftDomPoint( bool ldp );
/*
Sets the value of the dominating point flag of a half segment.

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




/*
Operator redefinitions.

*/
    int Compare( const HalfSegment& hs ) const;
/*
This function make comparison between two halfsegments. The rule of the comparison is specified
in the ROSE Algebra paper. That is:  the half sgenments will be ordered according to the following values:
dominating points -\verb+>+  LDP flages  -\verb+>+ directions (rotations).

*/
    bool Intersects( const HalfSegment& hs ) const;
/*
Decides whether two half segments intersect with each other with any kind of intersection.

*/
    bool InnerIntersects( const HalfSegment& hs ) const;
/*
Decides whether two half segments intersect in the following manner: a point of the first segment and an
innerpoint of the second segment are the same.

*/
    bool Crosses( const HalfSegment& hs ) const;
/*
Computes whether two half segments intersect in their mid-points. Endpoints are not considered in
computing the results.

*/
    bool Intersection( const HalfSegment& hs, Point& p ) const;
/*
This function computes whether two half segments cross each other and returns the crossing point ~p~.

*/
    bool Intersection( const HalfSegment& hs, HalfSegment& reshs ) const;
/*
This function computes whether two half segments intersect each other and returns the resulting
halfsegment as ~reshs~.

*/
   void CohenSutherlandLineClipping( const Rectangle<2> &window,
                                     double &x0, double &y0,
                                     double &x1, double &y1,
                                     bool &accept) const;
/*
Implements the Cohen and Sutherland algorithm for clipping a segment to a clipping window.

*/
   void WindowClippingIn( const Rectangle<2> &window,
                          HalfSegment &hs,
                          bool &inside,
                          bool &isIntersectionPoint,
                          Point &intersectionPoint) const;
/*
Computes the part of the segment that is inside the rectangle ~window~, if
it exists. The ~inside~ parameter is set to true if there is a partion of the segment
inside the window. The ~isIntersectionPoint~ parameter is set to true if the intersection
part of the segment is a point instead of a segment, and if so, ~intersectionPoint~
receives the intersection point.

*/
    bool Inside( const HalfSegment& hs ) const ;
/*
Computes whether the half segment is inside the one in ~hs~.

*/
    bool Contains( const Point& p ) const;
/*
Computes whether the point ~p~ is contained in the half segment.
Uses the ~AlmostEqual~ function.

*/
    bool RayAbove( const Point& p, double &abovey0 ) const;
/*

Decides whether a half segment is above a point. This is useful when we want to decide
whether a point is inside a region.

*/
    bool RayDown(const Point& p, double &yIntersection) const;


    double Distance( const Point& p ) const;
/*
Computes the distance from the half segment to the point ~p~.

*/
    double Distance( const HalfSegment& hs ) const;
/*
Computes the minimum distance from two half segments.

*/

    double Distance(const Rectangle<2>& rect) const;
    double MaxDistance(const Rectangle<2>& rect) const;
    int LogicCompare( const HalfSegment& hs ) const;
/*
Compares two half segments according to their attribute values (~attr~).

*/
    bool innerInter( const HalfSegment& chs,  Point& resp,
                     HalfSegment& rchs, bool& first, bool& second ) const;
/*
Used in the Plane Sweep Algebra

*/

  private:

/*
5.13 Attributes

*/
    bool ldp;
/*
Indicates which is the dominating point: the left or the right point.

*/
    Point lp;
    Point rp;
/*
These two attributes give the left and right point of the half segment.

*/
  public:

    AttrType attr;
/*
This ~attribute~ property is useful if we process region values in the way indicated in the ROSE
paper.

*/
};


#endif

