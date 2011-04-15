/*
4 class Point

This class implements the memory representation of the ~point~ type
constructor. A point represents a point in the Euclidean plane or is
undefined. The figure \cite{fig:spatialdatatypes.eps} shows an example
of a point data type.

*/

#ifndef POINT_H
#define POINT_H

#include "Coord.h"
#include "Geoid.h"
#include "RectangleAlgebra.h"

class Point;
class Points;
class Line;
class Region;

inline bool AlmostEqual( const Point& p1, const Point& p2 );

class Point: public StandardSpatialAttribute<2>
{
  public:
/*
4.1. Constructors and Destructor

*/
    inline Point() {};
/*
This constructor should not be used.

There are two ways of constructing a point:

*/
    inline Point( const bool d,
                  const Coord& x = Coord(),
                  const Coord& y = Coord() );
/*
The first one receives a boolean value ~d~ indicating if the point is defined
and two coordinate ~x~ and ~y~ values.

*/
    inline Point( const Point& p );
/*
The second one receives a point ~p~ as argument and creates a point that is a
copy of ~p~.

*/
    inline ~Point()
    {}
/*
The destructor.

4.2 Member functions

*/
    inline const Coord& GetX() const
    {
      return x;
    }
/*
Returns the ~x~-coordinate.

*/
    inline const Coord& GetY() const
    {
      return y;
    }
/*
Returns the ~y~-coordinate.

*/
    inline const Rectangle<2> BoundingBox() const;
/*
Returns the point bounding box which is also a point.

*/
    inline void Set( const Coord& x, const Coord& y );
/*
Sets the value of the point object.

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
Operators redefinition.

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
    bool Inside( const Points& ps ) const;
/*
4.3.8 Operation ~inside~ (with ~rectangle~)

*Precondition:* ~u.IsDefined()~

*Semantics:* $u \in V$

*Complexity:* $O(1)$

*/
    bool Inside( const Rectangle<2>& r ) const;
/*
6.4.4 Operation ~inside~ (with ~line~)

*Precondition:* ~u.IsDefined() and V.IsOrdered()~

*Semantics:* $u \in V$

*Complexity:* $O(n)$, where ~n~ is the size of ~V~.

*/
    bool Inside( const Line& l ) const;
/*
6.4.4 Operation ~inside~ (with ~region~)

*Precondition:* ~u.IsDefined() and V.IsOrdered()~

*Semantics:* $u \in V$

*Complexity:* $O(log(n))$, where ~n~ is the size of ~V~.

*/
    bool Inside( const Region& r ) const;
/*
4.3.13 Operation ~distance~ (with ~point~)

*Precondition:* ~u.IsDefined()~ and ~v.IsDefined()~

*Semantics:* $dist(u,v) = \sqrt{(u.x - v.x)^2 + (u.y - v.y)^2}$

*Complexity:* $O(1)$

*/
    double Distance( const Point& p ) const;
/*
4.3.13 Operation ~distance~ (with ~points~)

*Precondition:* ~u.IsDefined()~ and ~V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | v \in V \}$

*Complexity:* $O(n)$, where ~n~ is the size of ~V~

*/
    double Distance( const Points& ps ) const;
/*
4.3.13 Operation ~distance~ (with ~rect2~)

*Precondition:* ~u.IsDefined()~ and ~V.IsOrdered()~

*/
    double Distance( const Rectangle<2>& r ) const;

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

4.3.13 Operation ~direction~

*Precondition:* ~u.IsDefined()~ and ~v.IsDefined()~

*Semantics:* returns the angle of the line from ~[*]this~ to ~p~, measured in degrees.

*Complexity:* $O(1)$

If ~geoid~ is NULL, euclidean geometry is used, otherwise the geoid object
is applied during spherical geometric calculation.

*/
    double Direction( const Point& p, const Geoid* geoid = 0) const;

/*
 4.3.13 Operation ~heading~

 Computes the heading (direction on a shere) betwenn points given as 
 (LON, LAT). The result will be in range 0..360 if both points are defined and
 not equal. otherwise -1. If the geoid is null, the unit circle is used to compute
 the heading, otherwise the geoid.

*/
   double Heading(const Point& p, const Geoid* geoid = 0) const;



/*
4.1 Spherical geometry operations

While the preceding operations use euclidic geometry, the following operations
use Spherical geometry.

*/

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
4.1.2

*/

/*
4.3.15 Operation ~translate~

This function moves the position of this Point object instance.

*/

    inline void Translate(const Coord& x, const Coord& y);

/*
4.3.15 Operation ~rotate~

This function rotates this point around the point defined by (x,y)
with a degree of alpha. The result is stored in res.

*/

   inline void Rotate(const Coord& x, const Coord& y, const double alpha,
                      Point& res) const;


/*
4.3.16 Intersection with different other spatial types

*/
   void Intersection(const Point& p, Points& result) const;
   void Intersection(const Points& ps, Points& result) const;
   void Intersection(const Line& l, Points& result) const;
   void Intersection(const Region& r, Points& result) const;


/*
4.3.16 Minus with different other spatial types

*/
   void Minus(const Point& p, Points& result) const;
   void Minus(const Points& ps, Points& result) const;
   void Minus(const Line& l, Points& result) const;
   void Minus(const Region& r, Points& result) const;


/*
4.3.16 Union with different other spatial types

*/
   void Union(const Point& p, Points& result) const;
   void Union(const Points& ps, Points& result) const;
   void Union(const Line& l, Line& result) const;
   void Union(const Region& r, Region& result) const;




/*
4.3.15 Operation ~add~

*Precondition:* ~p1.IsDefined(), p2.IsDefined()~

*Semantics:*  ~(p1.x + p2.x, p1.y + p2.y)~

*Complexity:* $O(1)$

*/
    inline Point Add( const Point& p ) const;

    inline bool IsEmpty()const{
      return !IsDefined();
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

    virtual bool hasBox(){
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
       return symbols::POINT;
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

#endif
