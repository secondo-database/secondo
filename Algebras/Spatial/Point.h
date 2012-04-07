/*
4 class Point

This class implements the memory representation of the ~point~ type
constructor. A point represents a point in the Euclidean plane or is
undefined. The figure \cite{fig:spatialdatatypes.eps} shows an example
of a point data type.

*/

#ifndef POINT_H
#define POINT_H

#include <sstream>

#include "Coord.h"
#include "Geoid.h"
#include "RectangleAlgebra.h"

class Point;
class Points;
class Line;
class SimpleLine;
class Region;

/*
4.0 Auxiliary functions

*/
inline bool AlmostEqual( const Point& p1, const Point& p2 );

/*
Convert an angle from radians to degrees

*/
inline Coord radToDeg(const Coord & rad);

/*
Convert an angle from degrees to radians

*/
inline Coord degToRad(const Coord &deg);

/*
Convert a standard angle 0<=a<360 to a standard aviation heading 0<h<=360.
NORTH is always 360 degree, not 0 degree!

*/
inline double directionToHeading( const double dir );

/*
Convert a standard aviation heading 0<h<=360 to a standard angle 0<=a<360.

*/
inline double headingToDirection( const double head );

/*
Corrected modulo function ( '%' has problems with negative values).

fmod(y,x) is the remainder on dividing y by x and always lies in the range
$0<=mod<x$. For instance: mod(2.3,2.)=0.3 and mod(-2.3,2.)=1.7

*/
double fmod2(const double &y, const double &x);

class Point: public StandardSpatialAttribute<2>
{
  public:
/*
4.1 Constructors and Destructor

This constructor should not be used:

*/
    inline Point() {};

/*
There are two ways of constructing a point:

The first one receives a boolean value ~d~ indicating if the point is defined
and two coordinate ~x~ and ~y~ values.

*/
    explicit inline Point( const bool d,
                  const Coord& x = Coord(),
                  const Coord& y = Coord() );
/*
The second one receives a point ~p~ as argument and creates a point that is a
copy of ~p~.

*/
    inline Point( const Point& p );
/*
The destructor.

*/
    inline ~Point()
    {}
/*
4.2 Member functions

Returns the ~x~-coordinate. For geographic coordinates: the LONgitude

*/
    inline const Coord& GetX() const
    {
      return x;
    }
/*

Returns the ~y~-coordinate. For geographic coordinates: the LATitude

*/
    inline const Coord& GetY() const
    {
      return y;
    }
/*
Returns the point's bounding box which is a rectangle with (almost) no extension.

*/
    inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;
/*
Sets the value of the point object.

*/
    inline void Set( const Coord& x, const Coord& y );
    inline void Set( const Point& p);
/*
Operators redefinition.

*/
    inline Point& operator=( const Point& p );
    inline bool operator<=( const Point& p ) const;
    inline bool operator<( const Point& p ) const;
    inline bool operator>=( const Point& p ) const;
    inline bool operator>( const Point& p ) const;
    inline Point operator+( const Point& p ) const;
    inline Point operator-( const Point& p ) const;
    inline Point operator*( const double d ) const;
/*
4.3 Operations

4.3.1 Operation ~scale~

*Precondition:* ~u.IsDefined()~

*Semantics:* $factor * u$

*Complexity:* $O(1)$

*/
    inline void Scale( const Coord& factor )
    {
      assert( IsDefined() );
      this->x *= factor;
      this->y *= factor;
    }
/*
4.3.1 Operation $=$ (~equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u = v$

*Complexity:* $O(1)$

*/
    inline bool operator==( const Point& p ) const;

    inline bool operator==( const Points& p) const;

/*
4.3.2 Operation $\neq$ (~not equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \neq v$

*Complexity:* $O(1)$Algebras/RTree/RTree.examples

*/
    inline bool operator!=( const Point& p ) const;
/*
4.3.7 Operation ~inside~ (with ~points~)

*Precondition:* ~u.IsDefined() $\&\&$ V.IsOrdered()~

*Semantics:* $u \in V$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
  bool Inside( const Points& ps, const Geoid* geoid=0 ) const;
/*
4.3.8 Operation ~inside~ (with ~rectangle~)

*Precondition:* ~u.IsDefined()~

*Semantics:* $u \in V$

*Complexity:* $O(1)$

*/
  bool Inside( const Rectangle<2>& r, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~inside~ (with ~line~)

*Precondition:* ~u.IsDefined() and V.IsOrdered()~

*Semantics:* $u \in V$

*Complexity:* $O(n)$, where ~n~ is the size of ~V~.

*/
  bool Inside( const Line& l, const Geoid* geoid=0 ) const;
  bool Inside( const SimpleLine& l, const Geoid* geoid = 0) const;
/*
6.4.4 Operation ~inside~ (with ~region~)

*Precondition:* ~u.IsDefined() and V.IsOrdered()~

*Semantics:* $u \in V$

*Complexity:* $O(log(n))$, where ~n~ is the size of ~V~.

*/
  bool Inside( const Region& r, const Geoid* geoid=0 ) const;
/*
4.3.13 Operation ~distance~ (with ~point~)

*Precondition:* ~u.IsDefined()~ and ~v.IsDefined()~ and (~geoid == NULL~ or
~geoid.IsDefined()~

*Semantics:* $dist(u,v) = \sqrt{(u.x - v.x)^2 + (u.y - v.y)^2}$ If ~geoid~ is NULL,
euclidean geometry is used, otherwise spherical geometry.

*Complexity:* $O(1)$

*/
    double Distance( const Point& p, const Geoid* geoid = 0 ) const;
/*
4.3.13 Operation ~distance~ (with ~points~)

*Precondition:* ~u.IsDefined()~ and ~V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | v \in V \}$

*Complexity:* $O(n)$, where ~n~ is the size of ~V~

*/
    double Distance( const Points& ps, const Geoid* geoid = 0 ) const;
/*
4.3.13 Operation ~distance~ (with ~rect2~)

*Precondition:* ~u.IsDefined()~ and ~V.IsOrdered()~ and (~geoid == NULL~ or
~geoid.IsDefined()~

*Semantics:* Returns ~p~'s minimum distances to a member of the pointset ~ps~.
If ~geoid~ is NULL, euclidean geometry is used, otherwise spherical geometry.

*/
    double Distance( const Rectangle<2>& r, const Geoid* geoid=0 ) const;


/*
4.1 Spherical geometry operations

While the preceding operations use euclidic geometry, the following operations
use Spherical geometry.

*/

/*
4.1.2 Operation ~checkGeographicCoord~

Return true iff the Point represents valid geographic coordinate, i.e.
the Point is defined, and
the X-coordinate represents a valid Longitude/Breite ( -180.0<=X<=180.), and
the Y-coordinate represents a valid Latitude/Laenge (-90.0<=Y<=90.0).

*/
  bool checkGeographicCoord() const;

/*
4.3.13 Operation ~calcEnclosedAngle~

Calculates the enclosed angle between (a,b) and (b,c) in degrees.
If ~geoid~ is NULL, euclidean geometry is used, otherwise the geoid object
is applied during spherical geometric calculation.

*/
  static double calcEnclosedAngle( const Point &a,
                                   const Point &b,
                                   const Point &c,
                                   const Geoid* geoid = 0);

/*

4.3.13 Operation ~Direction~

Computes the direction or heading (resp. direction/heading on a sphere) between
~This~ point and point ~p~, where both points have coordinates (X,Y), resp.
(LON, LAT). The result will be in range 0..360
if both points are defined and not equal.

If ~useHeading~ is ~false~, mathematical measuring is used (counter clockwise,
0 degree means following the direction of the X-axis). If ~useHeadding~ is ~true~,
geographical direction is used (clockwise, 0 degree means NORTH or following the
direction of the Y-axis).

If ~geoid~ is NULL, euclidean geometry is used, otherwise the geoid object
is applied during spherical geometric calculation.

The returned direction is measured at the starting point, which is ~THIS~, if
~atEndpoint~ is ~false~ (default). If ~atEndpoint~ is ~true~, the direction
arriving at the endpoint is returned.

In any error case, the result is negative (-1.0).

*/
  double Direction(const Point& p,
                   const bool returnHeading,
                   const Geoid* geoid,
                   const bool atEndpoint = false) const;

/*
An alias for backward compatibility

*/
  double Direction(const Point& p, const Geoid* geoid = 0,
                   const bool atEndpoint = false ) const{
    cerr << __PRETTY_FUNCTION__ << ": WARNING - this function is deprecated!"
         << endl;
    return Direction(p, false, geoid, atEndpoint);
  }

/*
An alias for backward compatibility

*/
  double Heading(const Point& p, const Geoid* geoid = 0,
                 const bool atEndpoint = false) const {
    cerr << __PRETTY_FUNCTION__ << ": WARNING - this function is deprecated!"
         << endl;
    return Direction(p, true, geoid, atEndpoint);
  }


/*

4.3.14 Operation ~MidpointTo~

*Precondition:* none.

*Semantics:* Returns the midpoint between ~this~ point and the supplied point ~p~.
If one of the two Points is undefined, an UNDEFINED Point is returned.
If AlmostEqual(this,p), this is returned.
If ~geoid~ is not NULL, Sperical geometry is applied, otherwise Euclidean geometry.
If an invalid geographic coordinate is found, the result is UNDEFINED.

See http://mathforum.org/library/drmath/view/51822.html for derivation

*/
  Point MidpointTo(const Point& p, const Geoid* geoid = 0) const;

/*
*Precondition*: none.

*Semantics*: Calculates a point (x,y) (if ~geoid~ == 0), res. (lat,lon)
(if ~geoid~ is not NULL, for a given fraction ~f~ of the distance between ~this~
and the other Point ~p~. We want the point a fraction f along the great circle
route from ~this~ to ~p~. ~f~=0 is ~this~. f=1 is ~p~.
The two points cannot be antipodal
(i.e. lat1+lat2=0 and abs(lon1-lon2)=pi) because then the route is undefined.

*/
  Point MidpointTo(const Point& p, const Coord& f,
                                   const Geoid* geoid = 0) const;


/*
4.1.1 Distance

Distance between two points given in geodetic (Lon,Lat)-coordinates.
The distance is measured along a geoid passed as an argument.

If an undefined Point or a Point with an invalid geographic coordinate is used,
~valid~ is set to false, otherwise the result is calculated and ~valid~ is set
to true.

*/

    double DistanceOrthodrome( const Point& p,
                               const Geoid& g,
                               bool& valid) const;

/*
Distance between two points given in geodetic (Lon,Lat)-coordinates.
The distance is measured along a geoid passed as an argument.

If an undefined Point or a Point with an invalid geographic coordinate is used,
~valid~ is set to false, otherwise the result is calculated and ~valid~ is set
to true.

The distance is computed iteratively. A precision may be specified by ~epsilon~.
In addition to the great circle distance, the initial and final bearing
(heading, in degrees, true north=0, east=90, etc.) from THIS Point to the
~other~ Point are returned.

If THIS and ~p~ are AlmostEqual, the distance is 0 and negative value -666.666 is
returned for both ~initialBearingDEG~ and ~finalBearingDEG~ to indicate invalid
headings.

*/
    double DistanceOrthodromePrecise( const Point& p,
                                      const Geoid& g,
                                      bool& valid,
                                      double& initialBearingDEG,
                                      double& finalBearingDEG,
                                      const bool epsilon = 0.00001 ) const;


/*
4.3.15 Operation ~translate~

This function moves the position of this Point object instance.

*/

    inline void Translate(const Coord& x, const Coord& y);

/*
4.3.15 Operation ~rotate~
.
This function rotates this point around the point defined by (x,y)
with a degree of alpha. The result is stored in res.

*/

   inline void Rotate(const Coord& x, const Coord& y, const double alpha,
                      Point& res) const;


/*
4.3.16 Intersection with different other spatial types

*/
  void Intersection(const Point& p, Points& result,
                    const Geoid* geoid=0) const;
  void Intersection(const Points& ps, Points& result,
                    const Geoid* geoid=0) const;
  void Intersection(const Line& l, Points& result,
                    const Geoid* geoid=0) const;
  void Intersection(const Region& r, Points& result,
                    const Geoid* geoid=0) const;


/*
4.3.16 Minus with different other spatial types

*/
  void Minus(const Point& p, Points& result, const Geoid* geoid=0) const;
  void Minus(const Points& ps, Points& result, const Geoid* geoid=0) const;
  void Minus(const Line& l, Points& result, const Geoid* geoid=0) const;
  void Minus(const Region& r, Points& result, const Geoid* geoid=0) const;


/*
4.3.16 Union with different other spatial types

*/
  void Union(const Point& p, Points& result, const Geoid* geoid=0) const;
  void Union(const Points& ps, Points& result, const Geoid* geoid=0) const;
  void Union(const Line& l, Line& result, const Geoid* geoid=0) const;
  void Union(const Region& r, Region& result, const Geoid* geoid=0) const;




/*
4.3.15 Operation ~add~

*Precondition:* ~p1.IsDefined(), p2.IsDefined()~

*Semantics:*  ~(p1.x + p2.x, p1.y + p2.y)~

*Complexity:* $O(1)$

*/
  inline Point Add( const Point& p, const Geoid* geoid=0 ) const;

  inline bool IsEmpty()const{ return !IsDefined(); }


/*
4.3.16 Operation ~toString~

Returns a textual representation of the point. If a ~geoid~ is passed,
data is represented as geographic coordinates. Otherwise, a euclidean
coord-pair string is returned.

*/
  string toString(const Geoid* geoid = 0) const;

/*
4.3.16 Operation ~GeographicBBox~

Returns the MBR (bounding box) containing the shortest orthodrome path for THIS
Point and the ~other~ Point.

The Geographic MBR is different from the euclidean one, since the orthodrome may
exceed the two points extreme LATitude coordinates.

*/

  Rectangle<2> GeographicBBox(const Point &other, const Geoid &geoid) const;

/*
Returns the vertex, i.e. the southernmost/northernmost point of the great circle
starting at THIS with course ~rcourseDEG~ (in degree). If an ivalid coordinate
is found or not 0<=~rcourseDEG~<=360, the result is UNDEFINED.

*/
  Point orthodromeVertex(const double& rcourseDEG) const;


/*
Returns the number of intersections of the great circle on which both Points
THIS and ~other~ lie with the great circle of latitude ~latitudeDEG~.
There may be 0, 1 or 2 intersections with any given circle of latitude.
The according minimumn and maximum longitudes are returned in parameters
~lonMinDEG~ and ~lonMaxDEG~.

If an error occurs, the return value is 0.

*/
int orthodromeAtLatitude( const Point &other, const double& latitudeDEG,
                            double& lonMinDEG, double lonMaxDEG) const;

/*
4.3.17 Operation ~orthodromeExtremeLatitudes~

Computes the extreme LATitude coordinates traversed by the orthodrome from
~this~ Point to the ~other~ Point.

If any Point is UNDEFINED or not a valid geographic coordinate, or the ~geoid~
is UNDEFINED, the result is ~false~ and the output parameters are set to invalid
values, otherwise the result is ~true~.


*/

  bool orthodromeExtremeLatitudes(const Point &other, const Geoid &geoid,
                                          double &minLat, double &maxLat) const;



   virtual string getSQLType(){ return "MDSYS.SDO_GEOMETRY"; }

   virtual string getSQLRepresentation(){ 
     if(!IsDefined()){
       return "NULL";
     }
     string sdo_gtype = "2001";  // 2 dimensional point

     stringstream sdo_point;
     sdo_point << "MDSYS.SDO_POINT_TYPE(" 
               << GetX() << ", " << GetY() << ", NULL)";
   
     // use oracle's format
    /*
     stringstream ss;
     ss << "MDSYS.SDO_GEOMETRY(" ; // geometry
     ss << sdo_gtype << ", "; // type 
     ss << "NULL" << ",";    // srid: id of spatial reference system
     ss << sdo_point.str() << ", "; // geometry
     ss << "NULL, "; // Elem_Info_Array
     ss << "NULL)"; // ordinate array
     return ss.str(); 
    */
    // use oracle and WKT
     return  "MDSYS.SDO_GEOMETRY('" + getWKT() + "')";
   }

   string getWKT(){
     stringstream ss;
     ss <<  "POINT(" << GetX() << ", " << GetY() << ")";
     return ss.str();
   }



/*
4.4 Functions needed to import the the ~point~ data type to tuple

There are totally 8 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    inline size_t Sizeof() const;

    inline size_t HashValue() const
    {
      if( !IsDefined() )
        return 0;
      return (size_t)(5*x + y);
    }

    inline void CopyFrom( const Attribute* right )
    {
      const Point* p = (const Point*)right;
      SetDefined( p->IsDefined() );
      if( IsDefined() )
        Set( p->x, p->y );
    }

    inline int Compare( const Attribute *arg ) const
    { // CD: Implementation following guidelines from Attribute.h:
      const Point* p = (const Point*)arg;
      if( !IsDefined() && !p->IsDefined() )
        return 0;
      if( !IsDefined() )
        return -1;
      if( !p->IsDefined() )
        return 1;
      if( *this > *p )
        return 1;
      if( *this < *p )
        return -1;
      return 0;
    }

    inline int CompareAlmost( const Attribute *arg ) const
    {
      const Point* p = (const Point*)arg;
      if( !IsDefined() && !p->IsDefined() )
        return 0;
      if( !IsDefined() )
        return -1;
      if( !p->IsDefined() )
        return 1;
      if( AlmostEqual( *this, *p ) )
        return 0;
      if( *this > *p )
        return 1;
      if( *this < *p )
        return -1;
      return 0;
    }

    inline bool Adjacent( const Attribute *arg ) const
    {
      return false;
    }

    virtual inline Point* Clone() const
    {
      return new Point( *this );
    }

    ostream& Print( ostream &os ) const;

    virtual uint32_t getshpType() const{
       return 1; // Point Type
    }

    virtual bool hasBox() const{
       return IsDefined();
    }

    virtual double getMinX() const{
      return x;
    }
    virtual double getMaxX() const{
      return x;
    }
    virtual double getMinY() const{
      return y;
    }
    virtual double getMaxY() const{
      return y;
    }
    virtual void writeShape(ostream& o, uint32_t RecNo) const{

       // first, write the record header
       WinUnix::writeBigEndian(o,RecNo);

       if(!IsDefined()){
         uint32_t length = 2;
         WinUnix::writeBigEndian(o,length);
         uint32_t type = 0;
            void distanceOrthodrome(Point arg1, Geoid arg2, bool arg3);
         WinUnix::writeLittleEndian(o,type);
       } else {
         uint32_t length = 10;
         WinUnix::writeBigEndian(o,length);
         uint32_t type = 1;
         WinUnix::writeLittleEndian(o,type);
         WinUnix::writeLittle64(o,x);
         WinUnix::writeLittle64(o,y);
       }
    }

    static const string BasicType(){
       return "point";
    }
    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }


  protected:
/*
4.5 Attributes

*/
    Coord x;
/*
The ~x~ coordinate.

*/
    Coord y;
/*
The ~y~ coordinate.

*/
};

/*
4.6 Auxiliary functions

*/
ostream& operator<<( ostream& o, const Point& p );

inline bool AlmostEqual( const Point& p1, const Point& p2 )
{
  return AlmostEqual( p1.GetX(), p2.GetX() ) &&
  AlmostEqual( p1.GetY(), p2.GetY() );
}

inline double directionToHeading( const double dir )
{
  double direction = dir;
  while( direction<0.0 ) { direction = (360.0 - direction); }
  if(direction > 360.0)  { direction = fmod(direction,360.0); }
  double head = (direction<=90)?(90.0-direction):(450.0-direction);
  if( AlmostEqual(head,0.0) ){
    head = 360.0; // Map NORTH to heading 360, not to 0
  }
  return head;
}

inline double headingToDirection( const double head )
{
  double heading = head;
  while( heading<0.0 ) { heading = (360.0 - heading); }
  if(heading > 360.0)  { heading = fmod(heading,360.0); }
  double dir = (heading<90.0)?(90-heading):(450.0-heading);
  if( AlmostEqual( dir, 360.0 ) ){ // map 360 degree to 0
    dir = 0.0;
  }
  return dir;
}

inline Coord radToDeg(const Coord & rad) {
  return (180.0 * rad)/M_PI;
}

inline Coord degToRad(const Coord &deg) {
  return (deg * M_PI)/180.0;
}


#endif
