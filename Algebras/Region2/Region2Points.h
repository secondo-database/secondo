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

[1] File Region2Points.h

This file contains classes of points for use within the Region2-Algebra and 
the MovingRegion3-Algebra.

[TOC]

1 Class definitions

*/
class Reg2GridPoint;
class Reg2ProvisionalPoint;
class Reg2PrecisePoint;

/*
1.1 Class ~Reg2GridPoint~

This class implements the memory representation of a point in the Euclidean plane
in integer coordinates extended with an error value for each coordinate or is undefined.
The flag isBasic indicates whether the point has an additional precise part.

*/
class Reg2GridPoint : public StandardSpatialAttribute<2> {

public:
/*
1.1.1 Attributes

The ~x~ coordinate.

*/
	int x;
/*
The ~y~ coordinate.

*/
	int y;
/*
The ~isBasic~ attribute.

*/
	bool isBasic;
/*
The ~defined~ attribute.

*/
	bool defined;
/*
The error values.

*/
	int xErr;
	int yErr;

/*
1.1.1 Constructors and Destructor

This constructor should not be used:

*/
	inline Reg2GridPoint();
/*
Creates a point receiving the two coordinates ~x~ and ~y~ 
and optionally the flag ~isBasic~.

*/
	inline Reg2GridPoint(int x, int y, bool isBasic = false);
        
/*
1.1.1 Member Functions

Returns the bounding box of the point.

*/
	inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;
        
/*
Returns the value of ~defined~.

*/
        inline bool IsDefined() const { return defined; }
/*
Sets the value of ~defined~.

*/
	inline void SetDefined(bool defined) { this->defined = defined; }

/*
Operators redefinition and comparison operators.

*/
        inline Reg2GridPoint& operator=(const Reg2GridPoint& other);
	inline bool Equal(const Reg2GridPoint& other) const;
	inline int Compare(const Reg2GridPoint& other) const;
	inline bool operator==(const Reg2GridPoint& other) const;
	inline bool operator!=(const Reg2GridPoint& other) const;
	inline bool operator>(const Reg2GridPoint& other) const;
	inline bool operator>=(const Reg2GridPoint& other) const;
	inline bool operator<(const Reg2GridPoint& other) const;
	inline bool operator<=(const Reg2GridPoint& other) const;
	inline Reg2ProvisionalPoint transformToProvisional();
	
/*
Functions for use as StandardSpatialAttribute.

*/
        inline double Distance
            ( const Rectangle<2>& r, const Geoid* geoid=0 ) const;
        inline bool Intersects
            (const Rectangle<2>& rect, const Geoid* geoid=0 ) const 
            { return false; };

        inline bool IsEmpty()const{ return !IsDefined(); }

        inline size_t Sizeof() const
	{
	  return sizeof( *this );
	}

	inline size_t HashValue() const
	{
	  if( !IsDefined() )
	    return 0;
	  return (size_t)(5*x + y);
	}

	inline void CopyFrom( const Attribute* right )
	{
	  const Reg2GridPoint* p = (const Reg2GridPoint*)right;
	  SetDefined( p->IsDefined() );
	  if( IsDefined() )
	  {
	    x = p->x;
	    y = p->y;
	    defined = true;
	  }
	}

	inline int Compare( const Attribute *arg ) const
	{ // CD: Implementation following guidelines from Attribute.h:
	  const Reg2GridPoint* p = (const Reg2GridPoint*)arg;
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

	inline bool Adjacent( const Attribute *arg ) const
	{
	  return false;
	}

	inline Reg2GridPoint* Clone() const
	{
	  return new Reg2GridPoint( *this );
	}
};

/*
1.1 Class ~Reg2ProvisionalPoint~

This class implements the memory representation of a point in the Euclidean plane 
in double coordinates extended with an error value for each coordinate. 

*/
class Reg2ProvisionalPoint {

public:
/*
1.1.1 Attributes

The ~x~ coordinate.

*/
	double x;
/*
The ~y~ coordinate.

*/
	double y;
/*
The error values.

*/
	double xErr;
	double yErr;

/*
1.1.1 Constructor and Destructor

This constructor receives the two coordinates ~x~ and ~y~ and 
an error value ~xErr~ and ~yErr~ for each coordinate.

*/
	inline Reg2ProvisionalPoint(double x, double y, 
				    double xErr, double yErr);

/*
The destructor.

*/
        inline ~Reg2ProvisionalPoint() { }
                                    
/*
1.1.1 Member Functions

Redefinition of operator ~==~

*/
	inline bool operator==(const Reg2ProvisionalPoint& other) const;

};

ostream& operator<<( ostream& o, const Reg2PrecisePoint& p );

/*
1.1 Class ~Reg2PrecisePoint~

This class implements the representation of a point in the Euclidean plane 
in precise coordinates of type mpq\_class or is undefined.

*/
class Reg2PrecisePoint {

public:
/*
1.1.1 Attributes

The ~x~ coordinate.

*/
	mpq_class x;
/*
The ~y~ coordinate.

*/
	mpq_class y;
/*
The ~defined~ attribute.

*/
	bool defined;

/*
1.1.1 Constructors and Destructor

This constructor should not be used:

*/
	inline Reg2PrecisePoint();
/*
There are two ways of constructing a point:

The first one receives the two coordinates ~x~ and ~y~ by mpq\_class values 
or each coordinate splitted into an integer part ~x2~ and ~y2~ and 
a precise part ~x1~ and ~y1~ by mpq\_class values including a scale factor ~scale~.

*/
        inline Reg2PrecisePoint(mpq_class x, mpq_class y);
        inline Reg2PrecisePoint(mpq_class x1, int x2, mpq_class y1, 
                                int y2, int scale = 0);
/*
The second one receives a point ~p~ or a precise point ~pp~ as argument and 
creates a point that is a copy of ~pp~ resp. ~p~.

*/
        inline Reg2PrecisePoint(const Reg2PrecisePoint& pp);
	inline Reg2PrecisePoint(const Point& p);
/*
The destructor.

*/
        inline ~Reg2PrecisePoint() {}

/*
1.1.1 Member Functions

Sets the value of the point object.

*/
	inline void Set(mpq_class x1, mpq_class y1);
        
/*
Returns the value of ~defined~.

*/
	inline bool IsDefined() const { return defined; }
/*
Sets the value of ~defined~.

*/
	inline void SetDefined(bool defined) { this->defined = defined; }

/*
Operators redefinition and comparison operators.

*/
        inline Reg2PrecisePoint& operator=(const Reg2PrecisePoint& other);
	inline bool Equal(const Reg2PrecisePoint& other) const;
	inline int Compare(const Reg2PrecisePoint& other) const;
	inline bool operator==(const Reg2PrecisePoint& other) const;
	inline bool operator!=(const Reg2PrecisePoint& other) const;
	inline bool operator>(const Reg2PrecisePoint& other) const;
	inline bool operator>=(const Reg2PrecisePoint& other) const;
	inline bool operator<(const Reg2PrecisePoint& other) const;
	inline bool operator<=(const Reg2PrecisePoint& other) const;
	
/*
Translates the point by adding the values ~x~ and ~y~ (which can be negative).

*/
	inline void Translate( const double& dx, const double& dy );
/*
Scales the point given a factor ~xf~ and a factor ~yf~ for each coordinate seperately.

*/
        inline void Scale( const double& xf, const double& yf );

/*
Returns the bounding box of the point.

*/
	inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;
/*
Computes whether the point is inside ~r~.

*/
	inline bool Inside(const Rectangle<2>& r) const;
	
};

/*
1 Class implementations

1.1 Implementation of class ~Reg2GridPoint~

*/
inline Reg2GridPoint::Reg2GridPoint() :
		x (0),
		y (0),
		isBasic (false),
		defined (false),
		xErr (1),
		yErr (1) {}

inline Reg2GridPoint::Reg2GridPoint(int x, int y, bool isBasic) :
		x (x),
		y (y),
		isBasic (isBasic),
		defined (true),
		xErr (isBasic ? 0 : 1),
		yErr (isBasic ? 0 : 1) {}

inline Reg2GridPoint& Reg2GridPoint::operator=(const Reg2GridPoint& other) {
	defined = other.defined;
	x = other.x;
	y = other.y;
	isBasic = other.isBasic;
	return *this;
}

inline const Rectangle<2> Reg2GridPoint::BoundingBox
    (const Geoid* geoid /*=0*/) const
{
  assert( IsDefined() );
  if( IsDefined() ) {
      return Rectangle<2>( true, 0.0, 0.0, 0.0, 0.0 );
  }
  return Rectangle<2>( false, 0.0, 0.0, 0.0, 0.0 );
}

inline double Reg2GridPoint::Distance
    ( const Rectangle<2>& r, const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented!"
         << endl; // TODO: implement spherical gemetry case
    assert(false);
  }
  return sqrt(2);
}

inline Reg2ProvisionalPoint Reg2GridPoint::transformToProvisional() {
	Reg2ProvisionalPoint result((double)x,
			(double)y, (isBasic? 0.0 : 1.0),
			(isBasic? 0.0 : 1.0));
	return result;
}

inline bool Reg2GridPoint::Equal(const Reg2GridPoint& other) const 
{

  return ( x == other.x && y == other.y 
	  && isBasic == other.isBasic && defined == other.defined );
}

inline int Reg2GridPoint::Compare(const Reg2GridPoint& other) const 
{
	if (!defined)
	{  
	  if (other.defined) return -1;
	  else return 0;
	}
	else
	{
	  if (!other.defined) return 1;
	  else
	  {
	    if ( x < other.x ) return -1;
	    if ( x > other.x ) return 1;
	    if ( y < other.y ) return -1;
	    if (y > other.y ) return 1;
	    return 0;
	  }
	}
}

inline bool Reg2GridPoint::operator==(const Reg2GridPoint& other) const {
	return (Compare(other) == 0);
}

inline bool Reg2GridPoint::operator!=(const Reg2GridPoint& other) const {
	return (Compare(other) != 0);
}

inline bool Reg2GridPoint::operator>(const Reg2GridPoint& other) const {
	return (Compare(other) > 0);
}

inline bool Reg2GridPoint::operator>=(const Reg2GridPoint& other) const {
	return (Compare(other) >= 0);
}

inline bool Reg2GridPoint::operator<(const Reg2GridPoint& other) const {
	return (Compare(other) < 0);
}

inline bool Reg2GridPoint::operator<=(const Reg2GridPoint& other) const {
	return (Compare(other) <= 0);
}

/*
1.1 Implementation of class ~Reg2ProvisionalPoint~

*/
inline Reg2ProvisionalPoint::Reg2ProvisionalPoint(double x,
		double y, double xErr, double yErr) :
		x (x),
		y (y),
		xErr (xErr),
		yErr (yErr) {}

inline bool Reg2ProvisionalPoint::operator==(
	  const Reg2ProvisionalPoint& other) const {
	return (x == other.x && y == other.y);
}

/*
1.1 Implementation of class ~Reg2PrecisePoint~

*/
inline Reg2PrecisePoint::Reg2PrecisePoint() :
	x (0),
	y (0),
	defined (false) {}

inline Reg2PrecisePoint::Reg2PrecisePoint(const Reg2PrecisePoint& pp) :
	x (pp.x),
	y (pp.y),
	defined (true) {}

inline Reg2PrecisePoint::Reg2PrecisePoint(const Point& p) 
{
  defined = p.IsDefined();

  x = D2MPQ((double)p.GetX());
  y = D2MPQ((double)p.GetY());
}

inline Reg2PrecisePoint::Reg2PrecisePoint(mpq_class x, mpq_class y) :
	x (x),
	y (y),
	defined (true) {}

inline Reg2PrecisePoint::Reg2PrecisePoint(mpq_class x1, int x2,
		mpq_class y1, int y2, int scale) :
	x (x1 + x2),
	y (y1 + y2),
	defined (true) 
	{
	  mpz_t sFactor;
	  mpz_init(sFactor);
	  mpq_class sFac(0);
	  uint sfactor;
    
	  if (scale > 0)
	  {
	    sfactor = scale;
	    mpz_ui_pow_ui(sFactor, 10, sfactor);
	    sFac = mpq_class(mpz_class(1), mpz_class(sFactor));
	  }
	  else
	  {
	    sfactor = -scale;
	    mpz_ui_pow_ui(sFactor, 10, sfactor);
	    sFac = mpq_class(mpz_class(sFactor), mpz_class(1));
	  }
	  sFac.canonicalize();
	  mpz_clear(sFactor);
    
	  x = x * sFac;
	  x.canonicalize();
	  y = y * sFac;
	  y.canonicalize();	    
	}

inline void Reg2PrecisePoint::Set(mpq_class x1, mpq_class y1) {
	defined = true;
	x = x1;
	y = y1;
}

inline Reg2PrecisePoint& Reg2PrecisePoint::operator=(
	  const Reg2PrecisePoint& other) {
	defined = other.defined;
	x = other.x;
	y = other.y;
	return *this;
}

inline bool Reg2PrecisePoint::Equal(const Reg2PrecisePoint& other) const {
	if (!defined)
	{  
	  if (other.defined) return -1;
	  else return 0;
	}
	else
	{
	  if (!other.defined) return 1;
	  else
	  {
	    if (mpq_equal(x.get_mpq_t(), other.x.get_mpq_t())==0) 
		return false;
	    return (mpq_equal(y.get_mpq_t(), other.y.get_mpq_t())!=0);
	  }
	}
}
	
inline int Reg2PrecisePoint::Compare(const Reg2PrecisePoint& other) const {
	if (!defined)
	{  
	  if (other.defined) return -1;
	  else return 0;
	}
	else
	{
	  if (!other.defined) return 1;
	  else
	  {
	    int j=cmp(x, other.x);
	    if (j==0) return cmp(y, other.y);
	    return j;
	  }
	}
}
	
inline bool Reg2PrecisePoint::operator==(const Reg2PrecisePoint& other) const
{
	return (Compare(other) == 0);
}

inline bool Reg2PrecisePoint::operator!=(const Reg2PrecisePoint& other) const
{
	return (Compare(other) != 0);
}

inline bool Reg2PrecisePoint::operator>(const Reg2PrecisePoint& other) const
{
	return (Compare(other) > 0);
}

inline bool Reg2PrecisePoint::operator>=(const Reg2PrecisePoint& other) const
{
	return (Compare(other) >= 0);
}

inline bool Reg2PrecisePoint::operator<(const Reg2PrecisePoint& other) const
{
	return (Compare(other) < 0);
}

inline bool Reg2PrecisePoint::operator<=(const Reg2PrecisePoint& other) const
{
	return (Compare(other) <= 0);
}

inline void Reg2PrecisePoint::Translate( const double& dx, const double& dy )
{
  assert( IsDefined() );
  x = x + dx;
  x.canonicalize();
  y = y + dy;
  y.canonicalize();
}

inline void Reg2PrecisePoint::Scale( const double& xf, const double& yf )
{
  assert( IsDefined() );
  x = x * xf;
  x.canonicalize();
  y = y * yf;
  y.canonicalize();
}

inline const Rectangle<2> Reg2PrecisePoint::BoundingBox(
      const Geoid* geoid /*=0*/) const
{
  assert( IsDefined() );
  return Rectangle<2>( true, x.get_d(), x.get_d(), y.get_d(), y.get_d() );
}

bool Reg2PrecisePoint::Inside(const Rectangle<2>& r) const
{
  assert( r.IsDefined() );
  if ( !IsDefined() || !r.IsDefined() )
  {
    return false;
  }
  if ( x.get_d() < r.MinD(0) || x.get_d() > r.MaxD(0) )
    return false;
  else if( y.get_d() < r.MinD(1) || y.get_d() > r.MaxD(1) )
    return false;
  return true;
}
