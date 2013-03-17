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

*/

#pragma once

#include <stdarg.h> 

typedef double Coord;

bool AlmostEqual(double d1,double d2);
int CompareDouble(const double a, const double b);

struct AttrType
{
  inline AttrType() { }

  explicit inline AttrType( int ) :
  faceno(-999999),
    cycleno(-999999),
    edgeno(-999999),
    coverageno(-999999),
    insideAbove(false),
    partnerno(-999999){ }

  /*
  The simple constructor.

  */
  inline AttrType( const AttrType& at ):
  faceno( at.faceno ),
    cycleno( at.cycleno ),
    edgeno( at.edgeno ),
    coverageno( at.coverageno ),
    insideAbove( at.insideAbove ),
    partnerno( at.partnerno )
  {}
  /*
  The copy constructor.

  */
  inline AttrType& operator=( const AttrType& at )
  {
    faceno = at.faceno;
    cycleno = at.cycleno;
    edgeno = at.edgeno;
    coverageno = at.coverageno;
    insideAbove = at.insideAbove;
    partnerno = at.partnerno;
    return *this;
  }
  /*
  Redefinition of the assignement operator.

  6.1 Attributes

  */
  int faceno;
  /*
  The face identifier

  */
  int cycleno;
  /*
  The cycle identifier

  */
  int edgeno;
  /*
  The edge (segment) identifier

  */
  int coverageno;
  /*
  Used for fast spatial scan of the inside[_]pr algorithm

  */
  bool insideAbove;
  /*
  Indicates whether the region's area is above or left of its segment

  */
  int partnerno;
  /*
  Stores the position of the partner half segment in half segment ordered array

  */
};

class Point /*: public StandardSpatialAttribute<2>*/
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
  explicit inline Point( const bool d, const Coord& x, const Coord& y ) :
  isDefined(d),
    x( x ),
    y( y )
  { }

  /*
  The second one receives a point ~p~ as argument and creates a point that is a
  copy of ~p~.

  */
  inline Point( const Point& p ) : isDefined(p.IsDefined()), x( p.x ), y( p.y )
  { }

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

  inline Point& operator=( const Point& p )
  {
    SetDefined( p.IsDefined() );
    if( IsDefined() ){
      x = p.x;
      y = p.y;
    }
    return *this;
  }

  inline bool operator==( const Point& p ) const
  {
    if(!IsDefined() && !p.IsDefined()){
      return true;
    }
    if(!IsDefined() || !p.IsDefined()){
      return false;
    }

    return AlmostEqual(x, p.x) && AlmostEqual(y, p.y); // changed by TB

  }

  inline bool operator!=( const Point& p ) const
  {
    return !( *this == p );
  }

  inline bool operator<=( const Point& p ) const
  {
    return !( *this > p );
  }

  inline bool operator<( const Point& p ) const
  {
    //assert( IsDefined() && p.IsDefined() );
    if( !IsDefined() ){
      return p.IsDefined();
    }
    if ( !p.IsDefined() ){
      return false;
    }
    bool eqx = AlmostEqual(x,p.x);
    return (!eqx && (x < p.x) ) ||
      (eqx && !AlmostEqual(y,p.y) && (y < p.y));
    //  return x < p.x || (x == p.x && y < p.y); // changed by TB
  }

  inline bool operator>=( const Point& p ) const
  {
    return !( *this < p );
  }

  inline bool operator>( const Point& p ) const
  {
    //assert( IsDefined() && p.IsDefined() );
    if( !p.IsDefined() ){
      return IsDefined();
    }
    if ( !IsDefined() ){
      return false;
    }
    bool eqx = AlmostEqual(x,p.x);
    return (!eqx && (x > p.x)) ||
      (eqx && !AlmostEqual(y,p.y) && (y>p.y));
    //  return x > p.x || ( x == p.x && y > p.y ); // changed by TB
  }

  bool IsDefined() const
  {
    return isDefined;
  }

  void SetDefined(bool value) 
  {
    isDefined=value;
  }

protected:
  /*
  4.5 Attributes

  */
  bool isDefined;

  Coord x;
  /*
  The ~x~ coordinate.

  */
  Coord y;
  /*
  The ~y~ coordinate.

  */
};

class HalfSegment
{
public:

  /*
  5.1 Constructors and Destructor

  A half segment is composed by two points which are called 
  ~left point~ (~lp~) and ~right point~ (~rp~),
  $lp < rp$, and a flag ~ldp~ (~left dominating point~) which 
  tells whether the left point is the dominating
  point.

  This constructor should not be used:

  */

  inline HalfSegment() {}
  /*
  Creates a half segment receiving all attributes as arguments. 
  The order between the left and right points is not important. 
  If ~lp~ is bigger than ~rp~ their values are changed.
  */

  inline HalfSegment( bool ldp,
    const Point& lp,
    const Point& rp ):
  ldp( ldp ),
    lp( lp ),
    rp( rp ),
    attr(-99999)
  {
    // assert(lp.IsDefined());
    // assert(rp.IsDefined());
    // assert( !AlmostEqual( lp, rp ) );

    if( lp > rp )
    {
      this->lp = rp;
      this->rp = lp;
    }
  }
  /*
  Creates a half segment copying the values from ~hs~.

  */

  inline HalfSegment( const HalfSegment& hs ):
  ldp( hs.ldp ),
    lp( hs.lp ),
    rp( hs.rp ),
    attr( hs.attr )
  {
    //assert(lp.IsDefined());
    //assert(rp.IsDefined());
  }
  /*
  The destructor.

  */
  inline ~HalfSegment() {}
  /*
  5.2 Member Functions

  Returns the left point of the half segment.

  */
  inline const Point& GetLeftPoint() const
  {
    return lp;
  }
  /*
  Returns the right point of the half segment.

  */
  inline const Point& GetRightPoint() const
  {
    return rp;
  }
  /*
  Returns the dominating point of the half segment.

  */
  inline const Point& GetDomPoint() const
  {
    if( ldp )
      return lp;
    return rp;
  }

  inline const Point&    GetSecPoint() const
  {
    if( ldp )
      return rp;
    return lp;
  }

  inline bool IsLeftDomPoint() const
  {
    return ldp;
  }

  /*
  Returns the bounding box of the half segment. 
  If ~geoid~ is not NULL, the geographic MBR is returned. 
  If ~geoid~ is UNDEFINED, the result is UNDEFINED.

  */
  // inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;
  /*
  Returns the "attr" value associated with a half segment. 
  The "attr" value is useful when we process region values.

  */

  inline const AttrType& GetAttr() const
  {
    return attr;
  }

  inline void SetAttr( AttrType& attr )
  {
    this->attr = attr;
  }


  /*
  Sets the value of the dominating point flag of a half segment.

  */
  inline void SetLeftDomPoint( bool ldp )
  {
    this->ldp = ldp;
  }

  /*
  Checks whethet the HalsSegment is vertical

  */

  inline bool IsVertical()const
  {
    return AlmostEqual(lp.GetX(),rp.GetX());
  }


  HalfSegment& operator=( const HalfSegment& hs )
  {
    ldp = hs.ldp;
    lp = hs.lp;
    rp = hs.rp;
    attr = hs.attr;
    return *this;
  }

  bool operator==( const HalfSegment& hs ) const
  {
    return Compare(hs) == 0;
  }

  bool operator!=( const HalfSegment& hs ) const
  {
    return !( *this == hs );
  }

  bool operator<( const HalfSegment& hs ) const
  {
    return Compare(hs) == -1;
  }

  bool operator>( const HalfSegment& hs ) const
  {
    return Compare(hs) == 1;
  }


  //bool operator<=(const HalfSegment& hs) const;
  //bool operator>=(const HalfSegment& hs) const;

  static bool Less( const HalfSegment& hs1,const HalfSegment& hs2) 
  {
    return hs1<hs2;
  }

  int Compare( const HalfSegment& hs ) const
  {
    const Point& dp = GetDomPoint(),
      sp = GetSecPoint(),
      DP = hs.GetDomPoint(),
      SP = hs.GetSecPoint();
    /*
    if(dp.GetX()<DP.GetX()) {
    return -1;
    } else if(dp.GetX()>DP.GetX()) {
    return 1;
    } else {
    return 0;
    }*/


    if( dp < DP )
      return -1;
    else if( dp > DP )
      return 1;

    if( ldp != hs.ldp )
    {
      if( ldp == false )
        return -1;
      return 1;
    }
    else
    {
      bool v1 = IsVertical();
      bool v2 = hs.IsVertical();
      if( v1 && v2 ) // both are vertical
      {
        if(   (     (CompareDouble(sp.GetY(),dp.GetY())>0)
          && ( CompareDouble(SP.GetY(),DP.GetY())>0)
          )
          ||
          (     (CompareDouble(sp.GetY(),dp.GetY())<0)
          && (CompareDouble(SP.GetY(),DP.GetY())<0) ) )
        {
          if( sp < SP )
            return -1;
          if( sp > SP )
            return 1;
          return 0;
        }
        else if( CompareDouble(sp.GetY(),dp.GetY())>0)
        {
          if( ldp == true )
            return 1;
          return -1;
        }
        else
        {
          if( ldp == true )
            return -1;
          return 1;
        }
      }
      else if( AlmostEqual(dp.GetX(),sp.GetX()) )
      {
        if( CompareDouble(sp.GetY(), dp.GetY())>0 )
        {
          if( ldp == true )
            return 1;
          return -1;
        }
        else if( CompareDouble(sp.GetY(),dp.GetY())<0 )
        {
          if( ldp == true )
            return -1;
          return 1;
        }

        return 0;
      }
      else if( AlmostEqual(DP.GetX(), SP.GetX()) )
      {
        if( CompareDouble(SP.GetY() , DP.GetY())>0 )
        {
          if( ldp == true )
            return -1;
          return 1;
        }
        else if( CompareDouble(SP.GetY() , DP.GetY())<0 )
        {
          if( ldp == true )
            return 1;
          return -1;
        }

        return 0;
      }
      else
      {
        Coord xd = dp.GetX(), yd = dp.GetY(),
          xs = sp.GetX(), ys = sp.GetY(),
          Xd = DP.GetX(), Yd = DP.GetY(),
          Xs = SP.GetX(), Ys = SP.GetY();
        double k = (yd - ys) / (xd - xs),
          K= (Yd -Ys) / (Xd - Xs);

        if( CompareDouble(k , K) <0 )
          return -1;
        if( CompareDouble( k,  K) > 0)
          return 1;

        if( sp < SP )
          return -1;
        if( sp > SP )
          return 1;
        return 0;
      }
    }
  }

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
  This ~attribute~ property is useful if we process 
  region values in the way indicated in the ROSE paper.

  */

  AttrType attr;
};

template <unsigned dim> class Rectangle
{
private:
  bool isDefined;
  double min[dim];
  double max[dim];

public:
  inline Rectangle( const bool defined, ... )
  {
    isDefined=defined;
    va_list ap;
    va_start( ap, defined );
    for( unsigned i = 0; i < dim; i++ )
    {
      double d1 = va_arg( ap, double ), d2 = va_arg( ap, double );
      min[i] = d1;
      max[i] = d2;
    }
    va_end( ap );
  }


  inline const double MinD( int d ) const
  {
    return min[d];
  }

  inline const double MaxD( int d ) const
  {
    return max[d];
  }

  bool IsDefined() const
  {
    return isDefined;
  }


};
