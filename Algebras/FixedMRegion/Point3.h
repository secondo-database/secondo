/*
4 class Point3

This class implements the memory representation of the ~point3~ type
constructor. A point3 represents a point in the Euclidean plane or is
undefined. The figure \cite{fig:spatialdatatypes.eps} shows an example
of a point data type.

*/

#ifndef POINT3_H
#define POINT3_H

#include <sstream>

#include "Coord.h"
#include "Geoid.h"
#include "RectangleAlgebra.h"

class Point3;
#ifndef M_PI
#define M_PI   3.14159265358979323846  
#endif



/*
4.0 Auxiliary functions

*/
inline bool AlmostEqual( const Point3& p1, const Point3& p2 );

class Point3: public StandardSpatialAttribute<3>
{
  public:
/*
4.1 Constructors and Destructor

This constructor should not be used:

*/
    inline Point3() {};

/*
There are two ways of constructing a point:

The first one receives a boolean value ~d~ indicating if the point is defined
and two coordinate ~x~ and ~y~ values.

*/
    explicit Point3( const bool d,
                    const Coord& x = Coord(),
                    const Coord& y = Coord(),
                    const Coord& alpha = Coord() );
/*
The second one receives a point ~p~ as argument and creates a point that is a
copy of ~p~.

*/
    Point3( const Point3& p );
/*
The destructor.

*/
    inline ~Point3()
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

Returns the ~alpha~-coordinate.

*/
    inline const Coord& GetAlpha() const
    {
      return alpha;
    }
/*
Returns the point's bounding box which is a rectangle with (almost) no extension.

*/
     const Rectangle<3> BoundingBox(const Geoid* geoid = 0) const;
/*
Sets the value of the point object.

*/
    void Set( const Coord& x, const Coord& y, const Coord& Alpha );
    void Set( const Point3& p);
/*
Operators redefinition.

*/
    Point3& operator=( const Point3& p );
    inline bool operator<=( const Point3& p ) const;
    bool operator<( const Point3& p ) const;
    inline bool operator>=( const Point3& p ) const;
    bool operator>( const Point3& p ) const;
    inline Point3 operator+( const Point3& p ) const;
    inline Point3 operator-( const Point3& p ) const;
    inline Point3 operator*( const double d ) const;
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
      this->alpha *= factor;
    }
/*
4.3.1 Operation $=$ (~equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u = v$

*Complexity:* $O(1)$

*/
     bool operator==( const Point3& p ) const;

/*
4.3.2 Operation $\neq$ (~not equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \neq v$

*Complexity:* $O(1)$Algebras/RTree/RTree.examples

*/
    inline bool operator!=( const Point3& p ) const;
/*
4.3.13 Operation ~distance~ (with ~point~)

*Precondition:* ~u.IsDefined()~ and ~v.IsDefined()~ and (~geoid == NULL~ or
~geoid.IsDefined()~

*Semantics:* $dist(u,v) = \sqrt{(u.x - v.x)^2 + (u.y - v.y)^2}$ If ~geoid~ is NULL,
euclidean geometry is used, otherwise spherical geometry.

*Complexity:* $O(1)$

*/
    double Distance( const Point3& p, const Geoid* geoid = 0 ) const;
/*
4.1 Importz/Export to CSV files

*/

virtual string getCsvStr() const{
    if(!IsDefined()){
      return "undef";    
    }
    stringstream ss;
    ss.precision(16);
    ss << "(" << x << " " << y << "" << alpha <<")";
    return ss.str();
 }

virtual void ReadFromString(string value);



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
  Point3 MidpointTo(const Point3& p, const Geoid* geoid = 0) const;

/*
*Precondition*: none.

*Semantics*: Calculates a point (x,y) (if ~geoid~ == 0), res. (lat,lon)
(if ~geoid~ is not NULL, for a given fraction ~f~ of the distance between ~this~
and the other Point ~p~. We want the point a fraction f along the great circle
route from ~this~ to ~p~. ~f~=0 is ~this~. f=1 is ~p~.
The two points cannot be antipodal
(i.e. lat1+lat2=0 and abs(lon1-lon2)=pi) because then the route is undefined.

*/
  Point3 MidpointTo(const Point3& p, const Coord& f,
                                   const Geoid* geoid = 0) const;

/*
4.3.15 Operation ~translate~

This function moves the position of this Point object instance.

*/

    inline void Translate(const Coord& x, const Coord& y, Coord& alpha);

/*
4.3.15 Operation ~rotate~
.
This function rotates this point around the point defined by (x,y)
with a degree of alpha. The result is stored in res.

*/


/*
4.3.15 Operation ~add~

*Precondition:* ~p1.IsDefined(), p2.IsDefined()~

*Semantics:*  ~(p1.x + p2.x, p1.y + p2.y)~

*Complexity:* $O(1)$

*/
  inline Point3 Add( const Point3& p, const Geoid* geoid=0 ) const;

  inline bool IsEmpty()const{ return !IsDefined(); }


/*
4.3.16 Operation ~toString~

Returns a textual representation of the point. If a ~geoid~ is passed,
data is represented as geographic coordinates. Otherwise, a euclidean
coord-pair string is returned.

*/
  string toString(const Geoid* geoid = 0) const;



/*
4.4 Functions needed to import the the ~point~ data type to tuple

There are totally 8 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
     size_t Sizeof() const;

    inline size_t HashValue() const
    {
      if( !IsDefined() )
        return 0;
      return (size_t)(25*x + 5*y + alpha);
    }

    inline void CopyFrom( const Attribute* right )
    {
      const Point3* p = (const Point3*)right;
      SetDefined( p->IsDefined() );
      if( IsDefined() )
        Set( p->x, p->y, p->alpha );
    }

    inline int Compare( const Attribute *arg ) const
    { // CD: Implementation following guidelines from Attribute.h:
      const Point3* p = (const Point3*)arg;
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
      const Point3* p = (const Point3*)arg;
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

    virtual inline Point3* Clone() const
    {
      return new Point3( *this );
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
    virtual double getMinAlpha() const{
      return alpha;
    }
    virtual double getMaxAlpha() const{
      return alpha;
    }
    


    static const string BasicType(){
       return "point3";
    }
    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }

//    double Distance( const Point& p, const Geoid* geoid=0 ) const;
/*
5.4.9 Operation ~distance~ (with ~points~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~

*/
//    double Distance( const Points& ps, const Geoid* geoid=0 ) const;
/*
5.4.9 Operation ~distance~ (with ~rect2~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~


*/
  double Distance( const Rectangle<3>& r, const Geoid* geoid=0 ) const;


  bool Intersects( const Rectangle<3>& r, const Geoid* geoid=0 ) const;
  

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

    Coord alpha;
};

/*
4.6 Auxiliary functions

*/
ostream& operator<<( ostream& o, const Point3& p );

inline bool AlmostEqual( const Point3& p1, const Point3& p2 )
{
  return AlmostEqual( p1.GetX(), p2.GetX() ) &&
  AlmostEqual( p1.GetY(), p2.GetY() ) &&
  AlmostEqual( p1.GetAlpha(), p2.GetAlpha() );
}








#endif
