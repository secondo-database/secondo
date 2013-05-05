/*
4 class Point2

This class implements the memory representation of the ~point~ type
constructor. A point represents a point in the Euclidean plane or is
undefined. The figure \cite{fig:spatialdatatypes.eps} shows an example
of a point data type.

*/

class Reg2GridPoint;
class Reg2ProvisionalPoint;
class Reg2PrecisePoint;

class Reg2GridPoint : public StandardSpatialAttribute<2> {

public:

	int x;
	int y;
	bool isBasic;
	bool defined;
	int xErr;
	int yErr;

	inline Reg2GridPoint();
	inline Reg2GridPoint(int x, int y);
	inline Reg2GridPoint(int x, int y, bool isBasic);
	inline Reg2GridPoint& operator=(const Reg2GridPoint& other);

	inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;
	inline double Distance
	    ( const Rectangle<2>& r, const Geoid* geoid=0 ) const;
	inline bool IsEmpty()const{ return !IsDefined(); }
	inline bool Intersects
	    (const Rectangle<2>& rect, const Geoid* geoid=0 ) const 
	    { return false; };

	inline bool IsDefined() const { return defined; }
	inline void SetDefined(bool defined) { this->defined = defined; }

	inline bool Equal(const Reg2GridPoint& other) const;
	inline int Compare(const Reg2GridPoint& other) const;
	inline bool operator==(const Reg2GridPoint& other) const;
	inline bool operator!=(const Reg2GridPoint& other) const;
	inline bool operator>(const Reg2GridPoint& other) const;
	inline bool operator>=(const Reg2GridPoint& other) const;
	inline bool operator<(const Reg2GridPoint& other) const;
	inline bool operator<=(const Reg2GridPoint& other) const;
	inline Reg2ProvisionalPoint transformToProvisional();
	
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


class Reg2ProvisionalPoint {

public:

	double x;
	double y;
	double xErr;
	double yErr;

	inline Reg2ProvisionalPoint(double x, double y, 
				    double xErr, double yErr);

	inline bool operator==(const Reg2ProvisionalPoint& other) const;

};

ostream& operator<<( ostream& o, const Reg2PrecisePoint& p );

class Reg2PrecisePoint {

public:

	mpq_class x;
	mpq_class y;
	bool defined;

	inline Reg2PrecisePoint();
	inline Reg2PrecisePoint(const Reg2PrecisePoint& pp);
	inline Reg2PrecisePoint(const Point& p);
	inline Reg2PrecisePoint(mpq_class x, mpq_class y);
	inline Reg2PrecisePoint(mpq_class x1, int x2, mpq_class y1, 
				int y2, int scale = 0);
	inline Reg2PrecisePoint& operator=(const Reg2PrecisePoint& other);
	
	inline void Set(mpq_class x1, mpq_class y1);
	inline bool IsDefined() const { return defined; }
	inline void SetDefined(bool defined) { this->defined = defined; }

	inline bool Equal(const Reg2PrecisePoint& other) const;
	inline int Compare(const Reg2PrecisePoint& other) const;
	inline bool operator==(const Reg2PrecisePoint& other) const;
	inline bool operator!=(const Reg2PrecisePoint& other) const;
	inline bool operator>(const Reg2PrecisePoint& other) const;
	inline bool operator>=(const Reg2PrecisePoint& other) const;
	inline bool operator<(const Reg2PrecisePoint& other) const;
	inline bool operator<=(const Reg2PrecisePoint& other) const;
	
	inline void Translate( const double& dx, const double& dy );
        inline void Scale( const double& f );

	inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;
	inline bool Inside(const Rectangle<2>& r) const;
	
};



inline Reg2GridPoint::Reg2GridPoint() :
		x (0),
		y (0),
		isBasic (false),
		defined (false),
		xErr (1),
		yErr (1) {}

inline Reg2GridPoint::Reg2GridPoint(int x, int y) :
		x (x),
		y (y),
		isBasic (false),
		defined (true),
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
/*  double rxmin = r.MinD(0), rxmax = r.MaxD(0),
         rymin = r.MinD(1), rymax = r.MaxD(1);
  double dx =
        (   (x > rxmin || AlmostEqual(x,rxmin))
         && (x < rxmax || AlmostEqual(x,rxmax))) ? (0.0) :
        (min(abs(x-rxmin),abs(x-rxmax)));
  double dy =
        (   (y > rymin || AlmostEqual(y,rymin))
         && (y < rymax || AlmostEqual(y,rymax))) ? (0.0) :
        (min(abs(y-rymin),abs(y-rymax)));

  return sqrt( pow( dx, 2 ) + pow( dy, 2 ) ); */
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

inline void Reg2PrecisePoint::Scale( const double& f )
{
  assert( IsDefined() );
  x = x * f;
  x.canonicalize();
  y = y * f;
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

