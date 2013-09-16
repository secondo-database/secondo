/*
----
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, 
Faculty of Mathematics and Computer Science,
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
//[_] [\_]

[1] File Region2HalfSegments.h

This file contains classes of halfsegments for use within the Region2-Algebra and 
the MovingRegion3-Algebra.

[TOC]

1 Class definitions

*/
class Reg2GridHalfSegment;
class Reg2PrecHalfSegment;
class Reg2PreciseHalfSegment;

ostream& operator<<(ostream &os, const Reg2GridHalfSegment& hs);
ostream& operator<<(ostream &os, const Reg2PrecHalfSegment& hs);
ostream& operator<<(ostream &os, const Reg2PreciseHalfSegment& hs);

/*
1.1 Class ~Reg2GridHalfSegment~

This class implements the memory representation of a halfsegment in integer coordinates. 
A halfsegment value is composed of a pair of points and a flag indicating the dominating
point.

*/
class Reg2GridHalfSegment
{
  private:
/*
1.1.1 Attributes

Indicates which is the dominating point: the left or the right point.

*/
    bool ldp;
/*
These attributes represent the coordinates of the left and right point of the halfsegment.

*/
    int lx;
    int ly;
    int rx;
    int ry;

  public:
/*
This ~attribute~ property is useful if we process region values in the way 
indicated in the ROSE paper.

*/
    AttrType attr;

/*
1.1.1 Constructors and Destructor

This constructor should not be used:

*/
    inline Reg2GridHalfSegment() {}
/*
Creates a halfsegment receiving all attributes as arguments.

*/
    inline Reg2GridHalfSegment( bool ldp, const int lx, const int ly, 
                                const int rx, const int ry );
/*
Creates a halfsegment copying the values from ~hs~.

*/
    inline Reg2GridHalfSegment( const Reg2GridHalfSegment& hs );

/*
The destructor.

*/
    inline ~Reg2GridHalfSegment() {}

/*
1.1.1 Member Functions

Returns the coordinates of the left point of the halfsegment.

*/
    inline const int GetLeftPointX() const;
    inline const int GetLeftPointY() const;
/*
Returns the coordinates of the right point of the halfsegment.

*/
    inline const int GetRightPointX() const;
    inline const int GetRightPointY() const;
/*
Sets the coordinates of the left point of the halfsegment.

*/
    inline void SetLeftPoint( const int lx, const int ly );
/*
Sets the coordinates of the right point of the halfsegment.

*/
    inline void SetRightPoint( const int rx, const int ry );
/*
Sets all values of a halfsegment.

*/
    inline void Set( bool ldp, const int lx, const int ly, 
                     const int rx, const int ry );
/*
Returns the coordinates of the dominating point of the halfsegment.

*/
    inline const int GetDomPointX() const;
    inline const int GetDomPointY() const;
/*
Returns the coordinates of the secondary point of the halfsegment.

*/
    inline const int GetSecPointX() const;
    inline const int GetSecPointY() const;
/*
Returns the boolean flag which indicates whether the dominating point is on the left side.

*/
    inline bool IsLeftDomPoint() const;
/*
Sets the value of the dominating point flag of a halfsegment.

*/
    inline void SetLeftDomPoint( bool ldp );
/*
Checks whether the halfsegment is vertical

*/
    inline bool IsVertical() const;

/*
Returns the "attr" value associated with a halfsegment.

*/
    inline const AttrType& GetAttr() const;
/*
Sets the value of the "attr" attribute of a halfsegment.

*/
    inline void SetAttr( AttrType& attr );
    
};

/*
1.1 Class ~Reg2PrecHalfSegment~

This class implements the memory representation of the precise parts of a halfsegment.
It contains the position and the number of unsigend int values of the numerator and the
denominator for each coordinate of a halfsegment.

*/
class Reg2PrecHalfSegment
{
/*
1.1.1 Private attributes

  * ~lxNumPosition~, ~lyNumPosition~, ~rxNumPosition~, ~ryNumPosition~ is the index position of the first 
integer part representing the numerator of the precise part of the given coordinate 
in the respective DbArray, with lx x coordinate of the left point, 
ly y coordinate of the left point, rx x coordinate of the right point and 
ry y coordinate of the right point.
  
  * ~lxDenPosition~, ~lyDenPosition~, ~rxDenPosition~, ~ryDenPosition~ is the index position of the first 
integer part representing the denominator of the precise part of the given coordinate 
in the respective DbArray, with lx x coordinate of the left point, 
ly y coordinate of the left point, rx x coordinate of the right point and 
ry y coordinate of the right point.
  
  * ~lxNumInts~, ~lyNumInts~, ~rxNumInts~, ~ryNumInts~ is the number of integer parts 
representing the numerator of the precise part of the respective coordinate in the DbArray.
  
  * ~lxDenInts~, ~lyDenInts~, ~rxDenInts~, ~ryDenInts~ is the number of integer parts 
representing the denominator of the precise part of the respective coordinate in the DbArray.
  

*/
        int lxNumPosition;
        int lxDenPosition;
        int lyNumPosition;
        int lyDenPosition;
        int rxNumPosition;
        int rxDenPosition;
        int ryNumPosition;
        int ryDenPosition;
        int lxNumInts;
        int lxDenInts;
        int lyNumInts;
        int lyDenInts;
        int rxNumInts;
        int rxDenInts;
        int ryNumInts;
        int ryDenInts;

public:
/*
1.1.1 Constructors and Destructor

The default constructors does nothing.

*/
    inline Reg2PrecHalfSegment() {}

/*
The destructor.

*/
    inline ~Reg2PrecHalfSegment() {}

/*
These constructors initializes the private attributes. 
The first version sets each value individually.

*/
    Reg2PrecHalfSegment(int lxNumPos, int lxDenPos, int lyNumPos, int lyDenPos, 
                        int rxNumPos, int rxDenPos, int ryNumPos, int ryDenPos, 
                        int lxNumIs, int lxDenIs, int lyNumIs, int lyDenIs, 
                        int rxNumIs, int rxDenIs, int ryNumIs, int ryDenIs) :
      lxNumPosition(lxNumPos), lxDenPosition(lxDenPos),
      lyNumPosition(lyNumPos), lyDenPosition(lyDenPos),
      rxNumPosition(rxNumPos), rxDenPosition(rxDenPos),
      ryNumPosition(ryNumPos), ryDenPosition(ryDenPos),
      lxNumInts(lxNumIs), lxDenInts(lxDenIs),
      lyNumInts(lyNumIs), lyDenInts(lyDenIs),
      rxNumInts(rxNumIs), rxDenInts(rxDenIs),
      ryNumInts(ryNumIs), ryDenInts(ryDenIs)
    { }

/*
The second version sets all values to an initial value indicationg empty parts.

*/
    Reg2PrecHalfSegment(int startPos) :
      lxNumPosition(startPos), lxDenPosition(startPos),
      lyNumPosition(startPos), lyDenPosition(startPos),
      rxNumPosition(startPos), rxDenPosition(startPos),
      ryNumPosition(startPos), ryDenPosition(startPos),
      lxNumInts(0), lxDenInts(0),
      lyNumInts(0), lyDenInts(0),
      rxNumInts(0), rxDenInts(0),
      ryNumInts(0), ryDenInts(0)
    { }

/*
1.1.1 Attribute read access methods

*/
    inline int getlxNumPosition() const { return lxNumPosition; }
    inline int getlxDenPosition() const { return lxDenPosition; }
    inline int getlyNumPosition() const { return lyNumPosition; }
    inline int getlyDenPosition() const { return lyDenPosition; }
    inline int getrxNumPosition() const { return rxNumPosition; }
    inline int getrxDenPosition() const { return rxDenPosition; }
    inline int getryNumPosition() const { return ryNumPosition; }
    inline int getryDenPosition() const { return ryDenPosition; }
    inline int getlxNumInts() const { return lxNumInts; }
    inline int getlxDenInts() const { return lxDenInts; }
    inline int getlyNumInts() const { return lyNumInts; }
    inline int getlyDenInts() const { return lyDenInts; }
    inline int getrxNumInts() const { return rxNumInts; }
    inline int getrxDenInts() const { return rxDenInts; }
    inline int getryNumInts() const { return ryNumInts; }
    inline int getryDenInts() const { return ryDenInts; }
    
/*
Each coordinate is returned by its mpq\_class-value.

*/
    inline mpq_class GetlPointx(
            const DbArray<unsigned int>* preciseCoordinates) const;
    inline mpq_class GetlPointy(
            const DbArray<unsigned int>* preciseCoordinates) const;
    inline mpq_class GetrPointx(
            const DbArray<unsigned int>* preciseCoordinates) const;
    inline mpq_class GetrPointy(
            const DbArray<unsigned int>* preciseCoordinates) const;
  
/*
1.1.1 Attribute write access methods

Each coordinate is set to the mpq\_class-value.

*/
    inline void SetlPointx (mpq_class x, 
                            DbArray<unsigned int>* preciseCoordinates);
    inline void SetlPointy (mpq_class y, 
                            DbArray<unsigned int>* preciseCoordinates);
    inline void SetrPointx (mpq_class x, 
                            DbArray<unsigned int>* preciseCoordinates);
    inline void SetrPointy (mpq_class y, 
                            DbArray<unsigned int>* preciseCoordinates);

/*
Set all coordinates to the value of ~theValue~.

*/
    void SetAll (mpq_class theValue, 
                 DbArray<unsigned int>* preciseCoordinates) 
    {
      SetlPointx (theValue, preciseCoordinates);
      SetlPointy (theValue, preciseCoordinates);
      SetrPointx (theValue, preciseCoordinates);
      SetrPointy (theValue, preciseCoordinates);
    }

};

/*
1.1 Class ~Reg2PreciseHalfSegment~

This class implements the memory representation of a precise halfsegment.
A halfsegment value is composed of a pair of points in precise coordinates 
of type mpq\_class and a flag indicating the dominating point. 
The left point is always smaller than the right one.

*/
class Reg2PreciseHalfSegment
{
  private:
/*
1.1.1 Attributes

Indicates which is the dominating point: the left or the right point.

*/
    bool ldp;
/*
These two attributes represent the left and right point of the halfsegment.

*/
    Reg2PrecisePoint lp;
    Reg2PrecisePoint rp;

  public:
/*
This ~attribute~ property is useful if we process region values in the way 
indicated in the ROSE paper.

*/
    AttrType attr;

/*
1.1.1 Constructors and Destructor

A halfsegment is composed by two points which are called ~left point~ (~lp~) and ~right point~ (~rp~),
$lp < rp$, and a flag ~ldp~ (~left dominating point~) which tells whether the left point is the dominating
point.

This constructor should not be used:

*/
    inline Reg2PreciseHalfSegment() {}

/*
Creates a halfsegment receiving all attributes as arguments. The order between the left
and right points is not important. If ~lp~ is bigger than ~rp~ their values are changed.

*/
    inline Reg2PreciseHalfSegment( bool ldp, const Reg2PrecisePoint& lp, 
                                   const Reg2PrecisePoint& rp );

/*
Creates a halfsegment copying the values from ~hs~.

*/
    inline Reg2PreciseHalfSegment( const Reg2PreciseHalfSegment& hs );
    inline Reg2PreciseHalfSegment( const HalfSegment& hs );

/*
The destructor.

*/
    inline ~Reg2PreciseHalfSegment() {}

/*
1.1.1 Member Functions

Returns the left point of the halfsegment.

*/
    inline const Reg2PrecisePoint& GetLeftPoint() const;
/*
Returns the right point of the halfsegment.

*/
    inline const Reg2PrecisePoint& GetRightPoint() const;
/*
Set the left point of the halfsegment.

*/
    inline void SetLeftPoint(const Reg2PrecisePoint& nlp);
/*
Set the right point of the halfsegment.

*/
    inline void SetRightPoint(const Reg2PrecisePoint& nrp);
/*
Sets all values of a halfsegment. The parameters ~lp~ and ~rp~ can ignore the order, and the
function will compare the parameter points and put the smaller one to ~lp~ and larger one to ~rp~.

*/
    inline void Set( bool ldp, const Reg2PrecisePoint& lp, 
                     const Reg2PrecisePoint& rp );
/*
Returns the dominating point of the halfsegment.

*/
    inline const Reg2PrecisePoint& GetDomPoint() const;
/*
Returns the secondary point of the halfsegment.

*/
    inline const Reg2PrecisePoint& GetSecPoint() const;
/*
Returns the boolean flag which indicates whether the dominating point is on the left side.

*/
    inline bool IsLeftDomPoint() const;
/*
Sets the value of the dominating point flag of a halfsegment.

*/
    inline void SetLeftDomPoint( bool ldp );
/*
Checks whether the halfsegment is vertical

*/
    inline bool IsVertical() const;

/*
Returns the "attr" value associated with a half segment. 
The "attr" value is useful when we process region values.

*/
    inline const AttrType& GetAttr() const;
/*
Sets the value of the "attr" attribute of a halfsegment.

*/
    inline void SetAttr( AttrType& attr );
    
/*
These functions make comparison between two halfsegments. The rule of the
comparison is specified in the ROSE Algebra paper. That is: the halfsegments
will be ordered according to the following values:
dominating points -\verb+>+  LDP flages  -\verb+>+ directions (rotations).

*/
    inline int Compare( const Reg2PreciseHalfSegment& hs ) const;
    
    inline bool operator==( const Reg2PreciseHalfSegment& hs ) const;
    inline bool operator!=( const Reg2PreciseHalfSegment& hs ) const;
    inline bool operator<(const Reg2PreciseHalfSegment& hs) const;
    inline bool operator>(const Reg2PreciseHalfSegment& hs) const;
    inline bool operator<=(const Reg2PreciseHalfSegment& hs) const;
    inline bool operator>=(const Reg2PreciseHalfSegment& hs) const;
    inline int LogicCompare( const Reg2PreciseHalfSegment& hs ) const;

/*
The assignment operator.

*/
    inline Reg2PreciseHalfSegment& operator=( 
                        const Reg2PreciseHalfSegment& hs );
/*
Translates the halfsegment by adding the values ~x~ and ~y~ (which can be negative) to both
~lp~ and ~rp~ points.

*/
    inline void Translate( const double& x, const double& y );
/*
Scales the halfsegment given a factor ~xf~ and a factor ~yf~ for each coordinate seperately.

*/
    inline void Scale( const double& xf, const double& yf );

/*
Returns the bounding box of the halfsegment.

*/
    inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;
    
/*
Decides whether two halfsegments intersect with each other with any kind of
intersection.

*/
    bool Intersects( const Reg2PreciseHalfSegment& hs ) const;
/*
Decides whether two halfsegments intersect in the following manner: a point of
the first segment and an innerpoint of the second segment are the same.

*/
    bool InnerIntersects( const Reg2PreciseHalfSegment& hs ) const;
/*
Computes whether two halfsegments intersect in their mid-points. 
Endpoints are not considered in computing the results.

*/
    bool Crosses( const Reg2PreciseHalfSegment& hs ) const;
/*
This function computes whether two halfsegments cross each other and returns
the crossing point ~p~.

*/
    bool Intersection( const Reg2PreciseHalfSegment& hs, 
                       Reg2PrecisePoint& p ) const;
/*
This function computes whether two halfsegments intersect each other and
returns the resulting halfsegment as ~reshs~.

*/
    bool Intersection( const Reg2PreciseHalfSegment& hs, 
                       Reg2PreciseHalfSegment& reshs ) const;
/*
Computes whether the halfsegment is inside the one in ~hs~.

*/
    bool Inside( const Reg2PreciseHalfSegment& hs ) const ;
/*
Computes whether the point ~p~ is contained in the halfsegment.

*/
    inline bool Contains( const Reg2PrecisePoint& p ) const;
/*
Decides whether a halfsegment is above a point. This is useful when we want to decide
whether a point is inside a region.

*/
    bool RayAbove( const Reg2PrecisePoint& p, mpq_class &yIntersection ) const;
/*
Decides whether a half segment is below a point. This is useful when we want to decide
whether a point is inside a region.

*/
    bool RayDown( const Reg2PrecisePoint& p, mpq_class &yIntersection ) const;
};


/*
1 Class implementations

1.1 Implementation of class ~Reg2GridHalfSegment~

*/
inline Reg2GridHalfSegment::Reg2GridHalfSegment( bool ldp, 
                                 const int lx, const int ly, 
                                 const int rx, const int ry ):
ldp( ldp ),
lx( lx ),
ly( ly ),
rx( rx ),
ry( ry ),
attr(-99999)
{ }

inline Reg2GridHalfSegment::Reg2GridHalfSegment(const Reg2GridHalfSegment& hs):
ldp( hs.ldp ),
lx( hs.lx ),
ly( hs.ly ),
rx( hs.rx ),
ry( hs.ry ),
attr( hs.attr )
{
}

inline const int Reg2GridHalfSegment::GetLeftPointX() const
{
  return lx;
}

inline const int Reg2GridHalfSegment::GetLeftPointY() const
{
  return ly;
}

inline const int Reg2GridHalfSegment::GetRightPointX() const
{
  return rx;
}

inline const int Reg2GridHalfSegment::GetRightPointY() const
{
  return ry;
}

inline void Reg2GridHalfSegment::SetLeftPoint( const int lx, const int ly )
{
  this->lx = lx;
  this->ly = ly;
}

inline void Reg2GridHalfSegment::SetRightPoint( const int rx, const int ry )
{
  this->rx = rx;
  this->ry = ry;
}

inline void Reg2GridHalfSegment::Set( bool ldp, const int lx, const int ly,
                                      const int rx, const int ry )
{
  this->ldp = ldp;
  this->lx = lx;
  this->ly = ly;
  this->rx = rx;
  this->ry = ry;
}

inline const int Reg2GridHalfSegment::GetDomPointX() const
{
  if( ldp )
    return lx;
  return rx;
}

inline const int Reg2GridHalfSegment::GetDomPointY() const
{
  if( ldp )
    return ly;
  return ry;
}

inline const int Reg2GridHalfSegment::GetSecPointX() const
{
  if( ldp )
    return rx;
  return lx;
}

inline const int Reg2GridHalfSegment::GetSecPointY() const
{
  if( ldp )
    return ry;
  return ly;
}

inline bool Reg2GridHalfSegment::IsLeftDomPoint() const
{
  return ldp;
}

inline void Reg2GridHalfSegment::SetLeftDomPoint( bool ldp )
{
  this->ldp = ldp;
}

inline bool Reg2GridHalfSegment::IsVertical() const 
{
  return lx == rx;
}

inline const AttrType& Reg2GridHalfSegment::GetAttr() const
{
  return attr;
}

inline void Reg2GridHalfSegment::SetAttr( AttrType& attr )
{
  this->attr = attr;
}


/*
1.1 Implementation of class ~Reg2PrecHalfSegment~

*/
inline mpq_class Reg2PrecHalfSegment::GetlPointx(
        const DbArray<unsigned int>* preciseCoordinates) const
{
    if (lxNumInts == 0) return mpq_class(0);
    mpz_class num = GetValueX(lxNumPosition, lxNumInts, preciseCoordinates);
    mpz_class den = GetValueX(lxDenPosition, lxDenInts, preciseCoordinates);
    return mpq_class(num, den);
}
  
inline mpq_class Reg2PrecHalfSegment::GetlPointy(
        const DbArray<unsigned int>* preciseCoordinates) const
{
    if (lyNumInts == 0) return mpq_class(0);
    mpz_class num = GetValueX(lyNumPosition, lyNumInts, preciseCoordinates);
    mpz_class den = GetValueX(lyDenPosition, lyDenInts, preciseCoordinates);
    return mpq_class(num, den);
}

inline mpq_class Reg2PrecHalfSegment::GetrPointx(
        const DbArray<unsigned int>* preciseCoordinates) const
{
    if (rxNumInts == 0) return mpq_class(0);
    mpz_class num = GetValueX(rxNumPosition, rxNumInts, preciseCoordinates);
    mpz_class den = GetValueX(rxDenPosition, rxDenInts, preciseCoordinates);
    return mpq_class(num, den);
}

inline mpq_class Reg2PrecHalfSegment::GetrPointy(
        const DbArray<unsigned int>* preciseCoordinates) const
{
    if (ryNumInts == 0) return mpq_class(0);
    mpz_class num = GetValueX(ryNumPosition, ryNumInts, preciseCoordinates);
    mpz_class den = GetValueX(ryDenPosition, ryDenInts, preciseCoordinates);
    return mpq_class(num, den);
}

inline void Reg2PrecHalfSegment::SetlPointx (mpq_class x, 
                                 DbArray<unsigned int>* preciseCoordinates)
{
    if (cmp(x.get_num(), 0) == 0) return;
    SetValueX (x.get_num(), preciseCoordinates, lxNumPosition, lxNumInts);
    SetValueX (x.get_den(), preciseCoordinates, lxDenPosition, lxDenInts);
}

inline void Reg2PrecHalfSegment::SetlPointy (mpq_class y, 
                                 DbArray<unsigned int>* preciseCoordinates)
{
    if (cmp(y.get_num(), 0) == 0) return;
    SetValueX (y.get_num(), preciseCoordinates, lyNumPosition, lyNumInts);
    SetValueX (y.get_den(), preciseCoordinates, lyDenPosition, lyDenInts);
}

inline void Reg2PrecHalfSegment::SetrPointx (mpq_class x, 
                                 DbArray<unsigned int>* preciseCoordinates)
{
    if (cmp(x.get_num(), 0) == 0) return;
    SetValueX (x.get_num(), preciseCoordinates,rxNumPosition, rxNumInts);
    SetValueX (x.get_den(), preciseCoordinates, rxDenPosition, rxDenInts);
}

inline void Reg2PrecHalfSegment::SetrPointy (mpq_class y, 
                                 DbArray<unsigned int>* preciseCoordinates)
{
    if (cmp(y.get_num(), 0) == 0) return;
    SetValueX (y.get_num(), preciseCoordinates,ryNumPosition, ryNumInts);
    SetValueX (y.get_den(), preciseCoordinates, ryDenPosition, ryDenInts);
}


/*
1.1 Implementation of class ~Reg2PreciseHalfSegment~

*/
inline Reg2PreciseHalfSegment::Reg2PreciseHalfSegment( bool ldp,
                                                const Reg2PrecisePoint& lp,
                                                const Reg2PrecisePoint& rp ):
ldp( ldp ),
lp( lp ),
rp( rp ),
attr(-99999)
{
  assert(lp.IsDefined());
  assert(rp.IsDefined());
  assert( lp != rp );

  if( lp > rp )
  {
    this->lp = rp;
    this->rp = lp;
  }
}

inline Reg2PreciseHalfSegment::Reg2PreciseHalfSegment( 
                                const Reg2PreciseHalfSegment& hs ):
ldp( hs.ldp ),
lp( hs.lp ),
rp( hs.rp ),
attr( hs.attr )
{
  assert(lp.IsDefined());
  assert(rp.IsDefined());
}

inline Reg2PreciseHalfSegment::Reg2PreciseHalfSegment( const HalfSegment& hs ):
ldp( hs.IsLeftDomPoint() ),
attr( hs.GetAttr() )
{
  assert(hs.GetLeftPoint().IsDefined());
  assert(hs.GetRightPoint().IsDefined());
  Reg2PrecisePoint lp = Reg2PrecisePoint(hs.GetLeftPoint());
  Reg2PrecisePoint rp = Reg2PrecisePoint(hs.GetRightPoint());

  assert( lp != rp );

  if( lp < rp )
  {
    this->lp = lp;
    this->rp = rp;
  }
  else // rp > lp
  {
    this->lp = rp;
    this->rp = lp;
  }
}

inline const Reg2PrecisePoint& Reg2PreciseHalfSegment::GetLeftPoint() const
{
  return lp;
}

inline const Reg2PrecisePoint& Reg2PreciseHalfSegment::GetRightPoint() const
{
  return rp;
}

inline void Reg2PreciseHalfSegment::SetLeftPoint(const Reg2PrecisePoint& lp)
{
  assert( lp.IsDefined() );
  assert( lp != this->rp );
  assert( lp < this->rp );
  this->lp = lp;
}

inline void Reg2PreciseHalfSegment::SetRightPoint(const Reg2PrecisePoint& rp)
{
  assert( rp.IsDefined() );
  assert( this->lp != rp );
  assert( this->lp < rp );
  this->rp = rp;
}

inline void Reg2PreciseHalfSegment::Set( bool ldp, const Reg2PrecisePoint& lp, 
                                         const Reg2PrecisePoint& rp )
{
  assert( lp.IsDefined() );
  assert( rp.IsDefined() );
  assert( lp != rp );

  this->ldp = ldp;
  if( lp < rp )
  {
    this->lp = lp;
    this->rp = rp;
  }
  else // rp > lp
  {
    this->lp = rp;
    this->rp = lp;
  }
}

inline const Reg2PrecisePoint& Reg2PreciseHalfSegment::GetDomPoint() const
{
  if( ldp )
    return lp;
  return rp;
}

inline const Reg2PrecisePoint&
Reg2PreciseHalfSegment::GetSecPoint() const
{
  if( ldp )
    return rp;
  return lp;
}

inline bool Reg2PreciseHalfSegment::IsLeftDomPoint() const
{
  return ldp;
}

inline void Reg2PreciseHalfSegment::SetLeftDomPoint( bool ldp )
{
  this->ldp = ldp;
}

inline bool Reg2PreciseHalfSegment::IsVertical() const 
{
  return lp.x == rp.x;
}

inline const AttrType& Reg2PreciseHalfSegment::GetAttr() const
{
  return attr;
}

inline void Reg2PreciseHalfSegment::SetAttr( AttrType& attr )
{
  this->attr = attr;
}

int Reg2PreciseHalfSegment::Compare( const Reg2PreciseHalfSegment& hs ) const
{
  const Reg2PrecisePoint& dp = GetDomPoint(),
                      sp = GetSecPoint(),
                      DP = hs.GetDomPoint(),
                      SP = hs.GetSecPoint();

  if ( dp < DP )
    return -1;
  else if ( dp > DP )
    return 1;

  if ( ldp != hs.ldp )
  {
    if ( ldp == false )
      return -1;
    return 1;
  }
  else
  {
    bool v1 = IsVertical();
    bool v2 = hs.IsVertical();
    if ( v1 && v2 ) // both are vertical
    {
      if ( ((cmp(sp.y,dp.y)>0) && (cmp(SP.y,DP.y)>0))
         ||((cmp(sp.y,dp.y)<0) && (cmp(SP.y,DP.y)<0)))
      {
        if( sp < SP )
          return -1;
        if( sp > SP )
          return 1;
        return 0;
      }
      else if (cmp(sp.y,dp.y)>0)
      {
        if ( ldp == true )
          return 1;
        return -1;
      }
      else
      {
        if ( ldp == true )
          return -1;
        return 1;
      }
    }
    else if ( dp.x == sp.x )
    {
      if ( cmp(sp.y, dp.y)>0 )
      {
        if ( ldp == true )
          return 1;
        return -1;
      }
      else if ( cmp(sp.y,dp.y)<0 )
      {
        if ( ldp == true )
          return -1;
        return 1;
      }
    }
    else if ( DP.x == SP.x )
    {
      if ( cmp(SP.y,DP.y)>0 )
      {
        if ( ldp == true )
          return -1;
        return 1;
      }
      else if ( cmp(SP.y,DP.y)<0 )
      {
        if ( ldp == true )
          return 1;
        return -1;
      }
    }
    else
    {
      mpq_class k ((dp.y - sp.y)/(dp.x - sp.x));
      k.canonicalize();
      mpq_class K ((DP.y - SP.y)/(DP.x - SP.x));
      K.canonicalize();

      if ( cmp( k ,K ) < 0 )
        return -1;
      if ( cmp( k, K ) > 0 )
        return 1;

      if ( sp < SP )
        return -1;
      if ( sp > SP )
        return 1;
      return 0;
    }
  }
  assert( true ); // This code should never be reached
  return 0;
}

inline Reg2PreciseHalfSegment& Reg2PreciseHalfSegment::operator=( 
                const Reg2PreciseHalfSegment& hs )
{
  ldp = hs.ldp;
  lp = hs.lp;
  rp = hs.rp;
  attr = hs.attr;
  return *this;
}

inline bool Reg2PreciseHalfSegment::operator==( 
                const Reg2PreciseHalfSegment& hs ) const
{
  return Compare(hs) == 0;
}

inline bool Reg2PreciseHalfSegment::operator!=( 
                const Reg2PreciseHalfSegment& hs ) const
{
  return !( *this == hs );
}

inline bool Reg2PreciseHalfSegment::operator<( 
                const Reg2PreciseHalfSegment& hs ) const
{
  return Compare(hs) < 0;
}

inline bool Reg2PreciseHalfSegment::operator>( 
                const Reg2PreciseHalfSegment& hs ) const
{
  return Compare(hs) > 0;
}

inline bool Reg2PreciseHalfSegment::operator<=( 
                const Reg2PreciseHalfSegment& hs ) const
{
  return Compare(hs) <= 0;
}

inline bool Reg2PreciseHalfSegment::operator>=( 
                const Reg2PreciseHalfSegment& hs ) const
{
  return Compare(hs) >= 0;
}

inline int Reg2PreciseHalfSegment::LogicCompare( 
                const Reg2PreciseHalfSegment& hs ) const
{
  if( attr.faceno < hs.attr.faceno )
    return -1;

  if( attr.faceno > hs.attr.faceno )
    return 1;

  if( attr.cycleno < hs.attr.cycleno )
    return -1;

  if( attr.cycleno > hs.attr.cycleno )
    return 1;

  if( attr.edgeno < hs.attr.edgeno )
    return -1;

  if( attr.edgeno > hs.attr.edgeno )
    return 1;

  return 0;
}

inline const Rectangle<2> Reg2PreciseHalfSegment::BoundingBox(
                const Geoid* geoid /*=0*/) const
{
  Rectangle<2> currentBB = lp.BoundingBox();
  currentBB = currentBB.Union(rp.BoundingBox());

  return currentBB;
}

inline void Reg2PreciseHalfSegment::Translate( const double& x, 
                                               const double& y )
{
  lp.Translate( x, y );
  rp.Translate( x, y );
}

inline void Reg2PreciseHalfSegment::Scale( const double& xf, const double& yf )
{
  lp.Scale( xf, yf );
  rp.Scale( xf, yf );
}

bool Reg2PreciseHalfSegment::Contains( const Reg2PrecisePoint& p ) const
{
  if ( !p.IsDefined() ) 
  {
    assert( p.IsDefined() );
    return false;
  }

  if ( p == lp || p == rp )
  {
    return true;
  }

  if ( lp.x != rp.x && lp.x != p.x )
    // the segment is not vertical
  {
    mpq_class k1 = (p.y - lp.y) / (p.x - lp.x);
    k1.canonicalize();
    mpq_class k2 = (rp.y - lp.y) / (rp.x - lp.x);
    k2.canonicalize();

    if ( cmp(k1, k2) == 0 )
    {
      if ( ( cmp(lp.x, p.x) <= 0 ) &&
           ( cmp(p.x, rp.x) <= 0 ) )
        // we check only this possibility because lp < rp and
        // therefore, in this case, xl < xr
        return true;
    }
  }
  else if ( cmp(lp.x, rp.x) == 0 &&
            cmp(lp.x, p.x) == 0 )
    // the segment is vertical and the point is also in the
    // same x-position. In this case we just have to check
    // whether the point is inside the y-interval
  {
    if ( ( cmp(lp.y, p.y) <= 0 && cmp(p.y, rp.y) <=0 ) ||
         ( cmp(rp.y, p.y) <= 0 && cmp(p.y, lp.y) <=0 ) )
      // Here we check both possibilities because we do not
      // know wheter yl < yr, given that we used the
      // AlmostEqual function in the previous condition
      return true;
  }

  return false;
}

inline int Reg2PreciseHalfSegmentCompare(const void *a, const void *b)
{
  const Reg2PreciseHalfSegment *hsa = (const Reg2PreciseHalfSegment *)a,
                           *hsb = (const Reg2PreciseHalfSegment *)b;
  return hsa->Compare( *hsb );
}

inline int Reg2PreciseHalfSegmentLogicCompare(const void *a, const void *b)
{
  const Reg2PreciseHalfSegment *hsa = (const Reg2PreciseHalfSegment *)a,
                           *hsb = (const Reg2PreciseHalfSegment *)b;

  return hsa->LogicCompare( *hsb );
}
