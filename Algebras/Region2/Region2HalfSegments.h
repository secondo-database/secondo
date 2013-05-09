/*
7 Class ~HalfSegment2~

This class implements the memory representation of  ~halfsegment~. Although ~halfsegment~
is not an independent type constructor, it is the basic construction unit of the ~line~ and the ~region~
type constructors.

A ~halfsegment~ value is composed of a pair of points and a flag indicating the dominating
point. The left point is always smaller than the right one.

*/

class Reg2GridHalfSegment;
class Reg2PrecHalfSegment;
class Reg2PreciseHalfSegment;

ostream& operator<<(ostream &os, const Reg2GridHalfSegment& hs);
ostream& operator<<(ostream &os, const Reg2PrecHalfSegment& hs);
ostream& operator<<(ostream &os, const Reg2PreciseHalfSegment& hs);

class Reg2PreciseHalfSegment
{
  private:
    bool ldp;
    Reg2PrecisePoint lp;
    Reg2PrecisePoint rp;

  public:
    AttrType attr;

    inline Reg2PreciseHalfSegment() {}
    inline Reg2PreciseHalfSegment( bool ldp, const Reg2PrecisePoint& lp, 
                                   const Reg2PrecisePoint& rp );
    inline Reg2PreciseHalfSegment( const Reg2PreciseHalfSegment& hs );
    inline Reg2PreciseHalfSegment( const HalfSegment& hs );

    inline ~Reg2PreciseHalfSegment() {}

    inline const Reg2PrecisePoint& GetLeftPoint() const;
    inline const Reg2PrecisePoint& GetRightPoint() const;
    inline void SetLeftPoint(const Reg2PrecisePoint& nlp);
    inline void SetRightPoint(const Reg2PrecisePoint& nrp);
    inline void Set( bool ldp, const Reg2PrecisePoint& lp, 
                     const Reg2PrecisePoint& rp );
    inline const Reg2PrecisePoint& GetDomPoint() const;
    inline const Reg2PrecisePoint& GetSecPoint() const;
    inline bool IsLeftDomPoint() const;
    inline void SetLeftDomPoint( bool ldp );
    inline bool IsVertical() const;

    inline const AttrType& GetAttr() const;
    inline void SetAttr( AttrType& attr );
    
    inline int Compare( const Reg2PreciseHalfSegment& hs ) const;
    inline Reg2PreciseHalfSegment& operator=( 
                        const Reg2PreciseHalfSegment& hs );
    inline bool operator==( const Reg2PreciseHalfSegment& hs ) const;
    inline bool operator!=( const Reg2PreciseHalfSegment& hs ) const;
    inline bool operator<(const Reg2PreciseHalfSegment& hs) const;
    inline bool operator>(const Reg2PreciseHalfSegment& hs) const;
    inline bool operator<=(const Reg2PreciseHalfSegment& hs) const;
    inline bool operator>=(const Reg2PreciseHalfSegment& hs) const;
    inline int LogicCompare( const Reg2PreciseHalfSegment& hs ) const;

    inline void Translate( const double& x, const double& y );
    inline void Scale( const double& f );

    inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;
    bool Intersects( const Reg2PreciseHalfSegment& hs ) const;
    bool InnerIntersects( const Reg2PreciseHalfSegment& hs ) const;
    bool Crosses( const Reg2PreciseHalfSegment& hs ) const;
    bool Intersection( const Reg2PreciseHalfSegment& hs, 
                       Reg2PrecisePoint& p ) const;
    bool Intersection( const Reg2PreciseHalfSegment& hs, 
                       Reg2PreciseHalfSegment& reshs ) const;
    bool Inside( const Reg2PreciseHalfSegment& hs ) const ;
    inline bool Contains( const Reg2PrecisePoint& p ) const;
    bool RayAbove( const Reg2PrecisePoint& p, mpq_class &yIntersection ) const;
    bool RayDown( const Reg2PrecisePoint& p, mpq_class &yIntersection ) const;
};


class Reg2GridHalfSegment
{
  private:
    bool ldp;
    int lx;
    int ly;
    int rx;
    int ry;

  public:
    AttrType attr;

    inline Reg2GridHalfSegment() {}
    inline Reg2GridHalfSegment( bool ldp, const int lx, const int ly, 
                                const int rx, const int ry );
    inline Reg2GridHalfSegment( const Reg2GridHalfSegment& hs );

    inline ~Reg2GridHalfSegment() {}

    inline const int GetLeftPointX() const;
    inline const int GetLeftPointY() const;
    inline const int GetRightPointX() const;
    inline const int GetRightPointY() const;
    inline void SetLeftPoint( const int lx, const int ly );
    inline void SetRightPoint( const int rx, const int ry );
    inline void Set( bool ldp, const int lx, const int ly, 
                     const int rx, const int ry );
    inline const int GetDomPointX() const;
    inline const int GetDomPointY() const;
    inline const int GetSecPointX() const;
    inline const int GetSecPointY() const;
    inline bool IsLeftDomPoint() const;
    inline void SetLeftDomPoint( bool ldp );
    inline bool IsVertical() const;

    inline const AttrType& GetAttr() const;
    inline void SetAttr( AttrType& attr );
    
};

class Reg2PrecHalfSegment
{
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
    inline Reg2PrecHalfSegment() {}

    inline ~Reg2PrecHalfSegment() {}

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
    
    inline mpq_class GetlPointx(
            const DbArray<unsigned int>* preciseCoordinates) const;
    inline mpq_class GetlPointy(
            const DbArray<unsigned int>* preciseCoordinates) const;
    inline mpq_class GetrPointx(
            const DbArray<unsigned int>* preciseCoordinates) const;
    inline mpq_class GetrPointy(
            const DbArray<unsigned int>* preciseCoordinates) const;
  
    inline void SetlPointx (mpq_class x, 
                            DbArray<unsigned int>* preciseCoordinates);
    inline void SetlPointy (mpq_class y, 
                            DbArray<unsigned int>* preciseCoordinates);
    inline void SetrPointx (mpq_class x, 
                            DbArray<unsigned int>* preciseCoordinates);
    inline void SetrPointy (mpq_class y, 
                            DbArray<unsigned int>* preciseCoordinates);

    void SetAll (mpq_class theValue, 
                 DbArray<unsigned int>* preciseCoordinates) 
    {
      SetlPointx (theValue, preciseCoordinates);
      SetlPointy (theValue, preciseCoordinates);
      SetrPointx (theValue, preciseCoordinates);
      SetrPointy (theValue, preciseCoordinates);
    }

};



inline Reg2GridHalfSegment::Reg2GridHalfSegment( bool ldp, 
                                 const int lx, const int ly, 
                                 const int rx, const int ry ):
ldp( ldp ),
lx( lx ),
ly( ly ),
rx( rx ),
ry( ry ),
attr(-99999)
{
/* Reg2GridHalfSegment doesn't decide the Halfsegment order, because of missing precise part!!
  if ( Reg2GridPoint(lx, ly) > Reg2GridPoint(rx, ry) )
  {
    this->lx = rx;
    this->ly = ry;
    this->rx = lx;
    this->ry = ly;
  } */
}

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
// Reg2GridHalfSegment doesn't decide the Halfsegment order, because of 
// missing precise part!!
//  assert(Reg2GridPoint(lx, ly) < Reg2GridPoint(this->rx, this->ry));
  this->lx = lx;
  this->ly = ly;
}

inline void Reg2GridHalfSegment::SetRightPoint( const int rx, const int ry )
{
// Reg2GridHalfSegment doesn't decide the Halfsegment order, because of 
// missing precise part!!
//  assert(Reg2GridPoint(this->lx, this->ly) < Reg2GridPoint(rx, ry));
  this->rx = rx;
  this->ry = ry;
}

inline void Reg2GridHalfSegment::Set( bool ldp, const int lx, const int ly,
                                      const int rx, const int ry )
{
  this->ldp = ldp;
// Reg2GridHalfSegment doesn't decide the Halfsegment order, because of 
// missing precise part!!
//  if( Reg2GridPoint(lx, ly) < Reg2GridPoint(rx, ry))
//  {
    this->lx = lx;
    this->ly = ly;
    this->rx = rx;
    this->ry = ry;
/*  }
  else // rp > lp
  {
    this->lx = rx;
    this->ly = ry;
    this->rx = lx;
    this->ry = ly;
  } */
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


inline Reg2PreciseHalfSegment::Reg2PreciseHalfSegment(         bool ldp,
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

inline void Reg2PreciseHalfSegment::Scale( const double& f )
{
  lp.Scale( f );
  rp.Scale( f );
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

