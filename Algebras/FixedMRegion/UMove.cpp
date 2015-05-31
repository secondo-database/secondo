/*
ll

*/
#include "UMove.h"


/*
3.1 Class ~UMove~

*/
void UMove::TemporalFunction( const Instant& t,
                               Point3& result,
                               bool ignoreLimits ) const
{
  TemporalFunction(t, result, 0, ignoreLimits);
}

void UMove::TemporalFunction( const Instant& t,
                                       Point3& result,
                                       const Geoid* geoid,
                                       bool ignoreLimits /*=false*/) const
{
  if( !IsDefined() ||
    !t.IsDefined() ||
    (geoid && !geoid->IsDefined()) ||
    (!timeInterval.Contains( t ) && !ignoreLimits) ){
    result.SetDefined(false);
  } else if( t == timeInterval.start ){
    result = p0;
    result.SetDefined(true);
  } else if( t == timeInterval.end ){
    result = p1;
    result.SetDefined(true);
  } else if(geoid){ // spherical geometry case
    Instant t0 = timeInterval.start;
    Instant t1 = timeInterval.end;
    Coord f = ((t-t0)/(t1-t0));
    result = p0.MidpointTo(p1, f, geoid);
  } else {// euclidean geometry cases
    Instant t0 = timeInterval.start;
    Instant t1 = timeInterval.end;
    double x = ((p1.GetX() - p0.GetX()) * ((t - t0) / (t1 - t0))) + p0.GetX();
    double y = ((p1.GetY() - p0.GetY()) * ((t - t0) / (t1 - t0))) + p0.GetY();
    double alpha = ((p1.GetAlpha() - p0.GetAlpha()) * ((t - t0) 
    / (t1 - t0))) + p0.GetAlpha();
    result.Set( x, y, alpha );
    result.SetDefined(true);
  }
}

bool UMove::Passes( const Point3& p ) const
{
/*
VTA - I could use the spatial algebra like this

----    HalfSegment hs;
        hs.Set( true, p0, p1 );
        return hs.Contains( p );
----
but the Spatial Algebra admit rounding errors (floating point operations). It
would then be very hard to return a true for this function.

*/
  assert( p.IsDefined() );
  assert( IsDefined() );

  if( (timeInterval.lc && AlmostEqual( p, p0 )) ||
      (timeInterval.rc && AlmostEqual( p, p1 )) )
    return true;

  if( AlmostEqual( p0.GetX(), p1.GetX() ) &&
      AlmostEqual( p0.GetX(), p.GetX() ) )
    // If the segment is vertical
  {
    if( ( p0.GetY() <= p.GetY() && p1.GetY() >= p.GetY() ) ||
        ( p0.GetY() >= p.GetY() && p1.GetY() <= p.GetY() ) )
    {
      if( ( p0.GetAlpha() <= p.GetAlpha() && p1.GetAlpha() >= p.GetAlpha() ) ||
        ( p0.GetAlpha() >= p.GetAlpha() && p1.GetAlpha() <= p.GetAlpha() ) )
        return true;
    }
  }
  else if( AlmostEqual( p0.GetY(), p1.GetY() ) &&
      AlmostEqual( p0.GetY(), p.GetY() ) )
    // If the segment is horizontal
  {
    if( ( p0.GetX() <= p.GetX() && p1.GetX() >= p.GetX() ) ||
        ( p0.GetX() >= p.GetX() && p1.GetX() <= p.GetX() ) )
    {
      if( ( p0.GetAlpha() <= p.GetAlpha() && p1.GetAlpha() >= p.GetAlpha() ) ||
        ( p0.GetAlpha() >= p.GetAlpha() && p1.GetAlpha() <= p.GetAlpha() ) )
         return true;
    }
  }
  else
  {
    double k1 = ( p.GetX() - p0.GetX() ) / ( p.GetY() - p0.GetY() ),
           k2 = ( p1.GetX() - p0.GetX() ) / ( p1.GetY() - p0.GetY() );

    if( AlmostEqual( k1, k2 ) &&
        ( ( p0.GetX() < p.GetX() && p1.GetX() > p.GetX() ) ||
          ( p0.GetX() > p.GetX() && p1.GetX() < p.GetX() ) ) )
      return true;
  }
  return false;
}

bool UMove::Passes( const Point3& val, const Geoid* geoid ) const {
  if(!geoid){
    return Passes( val );
  } else {
    UMove result(false);
    bool retval = At( val, result, geoid );
    return retval && result.IsDefined();
  }
}



bool UMove::At( const Point3& p, TemporalUnit<Point3>& res ) const {
  return At(p, res, 0);
}

bool UMove::At( const Point3& p,
                 TemporalUnit<Point3>& res,
                 const Geoid* geoid ) const {

  assert(p.IsDefined());
  assert(this->IsDefined());
  assert( !geoid || geoid->IsDefined() );

  UMove* result = static_cast<UMove*>(&res);
  *result = *this;

  Instant t0 = timeInterval.start,
          t1 = timeInterval.end;

  if(AlmostEqual(p0,p1)){// special case: static unit
     if(AlmostEqual(p,p0) || AlmostEqual(p,p1)){
        result->SetDefined(true);
        result->timeInterval = timeInterval;
        result->p0 = p0;
        result->p1 = p1;
        return true;
     } else {
        result->SetDefined(false);
        return false;
     }
  }
  if(AlmostEqual(p0,p)){// special case p on p0
    if(!timeInterval.lc){
       result->SetDefined(false);
      return false;
    } else {
      result->SetDefined(true);
      result->p0 = p0;
      result->p1 = p0;
      result->timeInterval.lc = true;
      result->timeInterval.rc = true;
      result->timeInterval.start = t0;
      result->timeInterval.end   = t0;
      return true;
    }
  }
  if(AlmostEqual(p,p1)){// special case p on p1
    if(!timeInterval.rc){
      result->SetDefined(false);
      return false;
    } else {
      result->SetDefined(true);
      result->p0 = p1;
      result->p1 = p1;
      result->timeInterval.lc = true;
      result->timeInterval.rc = true;
      result->timeInterval.start = t1;
      result->timeInterval.end   = t1;
      return true;
    }
  }

    double d_x = p1.GetX() - p0.GetX();
    double d_y = p1.GetY() - p0.GetY();
    double d_a = p1.GetAlpha() - p0.GetAlpha();
    double delta;
    int useDir;
    if((fabs(d_x)> fabs(d_y)) && (fabs(d_x)>fabs(d_a))){
      delta = (p.GetX()-p0.GetX() ) / d_x;
      useDir = 0;
    } else if (fabs(d_y)>fabs(d_a)) {
      delta = (p.GetY()-p0.GetY() ) / d_y;
      useDir = 1;
    } else {
      delta = (p.GetAlpha()-p0.GetAlpha() ) / d_a;
      useDir = 2;
    }
    if(AlmostEqual(delta,0)){
      delta = 0;
    }
    if(AlmostEqual(delta,1)){
      delta = 1;
    }
    if( (delta<0) || (delta>1)){
      result->SetDefined(false);
      return false;
    }
    switch (useDir) {
      case 0: {
        double y = p0.GetY() + delta*d_y;
        double a = p0.GetAlpha() + delta * d_a;
        if((!AlmostEqual(y,p.GetY())) || (!AlmostEqual(a, p.GetAlpha())) ){
          result->SetDefined(false);
          return false;
        }
        break;
        }
      case 1: {
        double x = p0.GetX() + delta*d_x;
        double a = p0.GetAlpha() + delta * d_a;
        if((!AlmostEqual(x,p.GetX())) || (!AlmostEqual(a, p.GetAlpha())) ){
          result->SetDefined(false);
          return false;
        }
        break;
        }
      case 2: {
        double x = p0.GetX() + delta*d_x;
        double y = p0.GetY() + delta * d_y;
        if((!AlmostEqual(x,p.GetX())) || (!AlmostEqual(y, p.GetY())) ){
          result->SetDefined(false);
          return false;
        }
        break;
        }
    default: assert(false);
    }
    Instant time = t0+(t1-t0)*delta;
    result->SetDefined(true);
    result->p0 = p;
    result->p1 = p;
    result->timeInterval.lc = true;
    result->timeInterval.rc = true;
    result->timeInterval.start = time;
    result->timeInterval.end = time;
    return true;
}

void UMove::At(const Rectangle<3>& rect, UMove& result) const{

  // both arguments have to be defined
  if(!IsDefined() || !rect.IsDefined()){
     result.SetDefined(false);
     return;
  }
  result.SetDefined( true );

  double minX = rect.MinD(0);
  double minY = rect.MinD(1);
  double maxX = rect.MaxD(0);
  double maxY = rect.MaxD(1);
  double x1 = p0.GetX();
  double y1 = p0.GetY();
  double alpha1 = p0.GetAlpha();
  
  // check for stationary unit
  if(AlmostEqual(p0,p1)){
     if( (x1>=minX) && (x1<=maxX) && // rect contains point
         (y1>=minY) && (y1<=maxY) ){
       result = *this;
     } else {
       result.SetDefined(false);
     }
     return;
  }

  double x2 = p1.GetX();
  double y2 = p1.GetY();
  double alpha2 = p1.GetAlpha();
  Instant s = timeInterval.start;
  Instant e = timeInterval.end;
  bool lc = timeInterval.lc;
  bool rc = timeInterval.rc;

  // clip vertical
  if( ((x1 < minX) && (x2 < minX) )  ||
      ((x1 > maxX) && (x2 > maxX))) {
     result.SetDefined(false);
     return;
  }

  result = *this;
  double dx = x2 - x1;
  double dy = y2 - y1;
  DateTime dt = e - s;

  if((x1 < minX) || (x2 < minX) ) {
    // trajectory intersects the left vertical line
    double delta = (minX-x1)/dx;
    double y = y1 + delta*dy;
    Instant i = s + dt*delta;
    // cut the unit
    if(x1 < minX){ // unit enters rect
       x1 = minX;
       y1 = y;
       s = i;
       lc = true;
    } else {  // unit leave rect
       x2 = minX;
       y2 = y;
       e = i;
       rc = true;
    }
    dx = x2 - x1;
    dy = y2 - y1;
    dt = e - s;
  }

  // do the same thing for maxX
  if((x1 > maxX) || (x2 > maxX) ) {
    // trajectory intersects the right vertical line
    double delta = (maxX-x1)/dx;
    double y = y1 + delta*dy;
    Instant i = s + dt*delta;
    // cut the unit
    if(x1 > maxX){ // unit enters rect
       x1 = maxX;
       y1 = y;
       s = i;
       lc = true;
    } else {  // unit leave rect
       x2 = maxX;
       y2 = y;
       e = i;
       rc = true;
    }
    dx = x2 - x1;
    dy = y2 - y1;
    dt = e - s;
  }

  // clip at the horizontal lines
  if( ((y1<minY) && (y2<minY)) ||
      ((y1>maxY) && (y2>maxY))){
    // nothing left
    result.SetDefined(false);
    return;
  }

  // clip at the bottom line
  if( (y1 < minY) || (y2<minY)){
    double delta =  (minY-y1)/dy;
    double x = x1 + delta*dx;
    Instant i = s + dt*delta;
    if(y1 < minY){ // unit enters rect
       x1 = x;
       y1 = minY;
       s = i;
       lc = true;
    } else {  // unit leave rect
       x2 = x;
       y2 = minY;
       e = i;
       rc = true;
    }
    dx = x2 - x1;
    dy = y2 - y1;
    dt = e - s;
  }

  if( (y1 > maxY) || (y2>maxY)){
    double delta =  (maxY-y1)/dy;
    double x = x1 + delta*dx;
    Instant i = s + dt*delta;
    if(y1 > maxY){ // unit enters rect
       x1 = x;
       y1 = maxY;
       s = i;
       lc = true;
    } else {  // unit leave rect
       x2 = x;
       y2 = maxY;
       e = i;
       rc = true;
    }
  }

  // handle rounding errors
  if(s<=timeInterval.start){
     s = timeInterval.start;
     lc = timeInterval.lc;
  }
  if(e>=timeInterval.end){
     e = timeInterval.end;
     rc = timeInterval.rc;
  }

  if(e<s){
    cerr << "Warning e < s ; s = " << s << ", e = " << e << endl;
    result.SetDefined(false);
    return;
  }
  if( (e == s) && (!lc || !rc)){
     result.SetDefined(false);
     return;
  }
  Interval<Instant> tmp(s,e,lc,rc);
  result.timeInterval=tmp;
  result.p0.Set(x1,y1, alpha1);
  result.p1.Set(x2,y2,alpha2);
}



void UMove::AtInterval( const Interval<Instant>& i,
                         TemporalUnit<Point3>& result ) const
{
  AtInterval( i, result, 0);
}

void UMove::AtInterval( const Interval<Instant>& i,
                         TemporalUnit<Point3>& result,
                         const Geoid* geoid) const
{
  assert( IsDefined() );
  assert( i.IsValid() );
  assert( !geoid || geoid->IsDefined() );

  TemporalUnit<Point3>::AtInterval( i, result );

  UMove *pResult = (UMove*)&result;
  pResult->SetDefined( IsDefined() );

  if( !IsDefined() ){
    return;
  }

  if( timeInterval.start == result.timeInterval.start ){
    pResult->p0 = p0;
    pResult->timeInterval.start = timeInterval.start;
    pResult->timeInterval.lc = (pResult->timeInterval.lc && timeInterval.lc);
  } else {
    TemporalFunction( result.timeInterval.start, pResult->p0, geoid );
  }
  if( timeInterval.end == result.timeInterval.end ){
    pResult->p1 = p1;
    pResult->timeInterval.end = timeInterval.end;
    pResult->timeInterval.rc = (pResult->timeInterval.rc && timeInterval.rc);
  } else {
    TemporalFunction( result.timeInterval.end, pResult->p1, geoid );
  }
}

void UMove::Distance( const Point3& p, UReal& result ) const {
  result.SetDefined(false);
  vector<UReal> resvector;
  resvector.clear();
  Distance( p, resvector );
  for(vector<UReal>::iterator i=resvector.begin(); i!=resvector.end(); i++){
    if(i->IsDefined()){
      result = *i;
      return;
    }
  }
}

void UMove::Distance( const Point3& p,
                       vector<UReal>& result,
                       const Geoid* geoid /*=0*/,
                       const double epsilon /*=0.00001*/) const
{
  if( !IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    UReal resunit(false);
    result.push_back(resunit);
  } else {
    UReal resunit(false);
    DateTime DT = timeInterval.end - timeInterval.start;
    double dt = DT.ToDouble();
    double
      x0 = p0.GetX(), y0 = p0.GetY(), alpha0=p0.GetAlpha(),
      x1 = p1.GetX(), y1 = p1.GetY(), alpha1=p1.GetAlpha(),
      x  =  p.GetX(), y  =  p.GetY(), alpha = p.GetAlpha();

    if ( AlmostEqual(dt, 0.0) || AlmostEqual(p0,p1)) {
      // single-instant or constant unit
      resunit.SetDefined(true);
      resunit.timeInterval = timeInterval;
      resunit.a = 0.0;
      resunit.b = 0.0;
     
        resunit.c = sqrt((pow(x0-x,2) + pow(y0-y,2)));
      
      resunit.r = false; // square root encoded in c
      result.push_back(resunit);
    } else { // non-constant, non-instant unit
     
        resunit.SetDefined(true);
        resunit.timeInterval = timeInterval;
        resunit.a = pow((x1-x0)/dt,2)+pow((y1-y0)/dt,2)
        +pow((alpha1-alpha0)/dt,2);
        resunit.b = 2*((x1-x0)*(x0-x)+(y1-y0)*(y0-y)+(alpha1-alpha0)*
        (alpha0-alpha))/dt;
        resunit.c = pow(x0-x,2)+pow(y0-y,2)+pow(alpha0-alpha,2);
        resunit.r = true; // draw square root
        result.push_back(resunit);
      
    }
  }
}

double UMove::Distance(const Rectangle<4>& rect,
                        const Geoid* geoid /*=0*/) const{
  cerr << "UMove::Distance(const Rectangle<3>&) not implemented yet" << endl;
  if( !IsDefined() || !rect.IsDefined() || (geoid && !geoid->IsDefined()) ){
    return -1;
  }
  return BoundingBox().Distance(rect,geoid);
}

bool UMove::Intersects(const Rectangle<4>& rect,
                          const Geoid* geoid) const{

   bool implemented = false; // not implemented yet
   assert(implemented);
   return false;
}


void UMove::Distance( const UMove& up,
                       UReal& result,
                       const Geoid* geoid ) const
{
  assert( IsDefined() );
  assert( up.IsDefined() );
  if(geoid){
    assert( geoid->IsDefined() );
    cerr << "Spherical distance computation not implemented!" << endl;
    assert( false ); // TODO: implement spherical geometry

    // use HalfSegment::Distance(HalfSegment) to find DISTmin
    // use UMove::AtValue(VALUEmin) to get Tmin
    // approximate the distance function by binary splits
    // TODO: implementation

  }
  assert( timeInterval.Intersects(up.timeInterval) );

  if(     !IsDefined()
       || !up.IsDefined()
       || !timeInterval.Intersects(up.timeInterval)
       || (geoid && !geoid->IsDefined() ) ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );



  if((timeInterval.start==timeInterval.end) &&
     (up.timeInterval.start == up.timeInterval.end)){
     // only for two points at the same time
     result.timeInterval = timeInterval;
     result.a = 0.0;
     result.b = 0.0;
     result.c = p0.Distance(up.p0);
     result.r = false;
    return;
  }

  if(timeInterval.start == timeInterval.end){
      // this is joint an ipoint
      result.timeInterval = timeInterval;
      Distance(up.p0,result);
      return;
  }

  if(up.timeInterval.start == up.timeInterval.end){
      // up is just an ipoint
      result.timeInterval = up.timeInterval;
      up.Distance(p0,result);
      return;
  }

  Interval<Instant>iv;
  DateTime DT(durationtype);
  Point3 rp10, rp11, rp20, rp21;
  double
    x10, x11, x20, x21,
    y10, y11, y20, y21,
    alpha10, alpha11, alpha20, alpha21,
    dx1, dy1, dalpha1,
    dx2, dy2, dalpha2,
    dx12, dy12, dalpha12,
    dt;

  timeInterval.Intersection(up.timeInterval, iv);
  result.timeInterval = iv;
  // ignore closedness for TemporalFunction:
  TemporalFunction(   iv.start, rp10, true);
  TemporalFunction(   iv.end,   rp11, true);
  up.TemporalFunction(iv.start, rp20, true);
  up.TemporalFunction(iv.end,   rp21, true);

  if ( AlmostEqual(rp10,rp20) && AlmostEqual(rp11,rp21) )
  { // identical points -> zero distance!
    result.a = 0.0;
    result.b = 0.0;
    result.c = 0.0;
    result.r = false;
    return;
  }

  DT = iv.end - iv.start;
  dt = DT.ToDouble();
  x10 = rp10.GetX(); y10 = rp10.GetY(); alpha10 = rp10.GetAlpha();
  x11 = rp11.GetX(); y11 = rp11.GetY(); alpha11 = rp11.GetAlpha();
  x20 = rp20.GetX(); y20 = rp20.GetY(); alpha20 = rp20.GetAlpha();
  x21 = rp21.GetX(); y21 = rp21.GetY(); alpha21 = rp21.GetAlpha();
  dx1 = x11 - x10;   // x-difference final-initial for u1
  dy1 = y11 - y10;   // y-difference final-initial for u1
  dalpha1 = alpha11 - alpha10;
  dx2 = x21 - x20;   // x-difference final-initial for u2
  dy2 = y21 - y20;   // y-difference final-initial for u2
  dalpha2 = alpha21 - alpha20;
  dx12 = x10 - x20;  // x-distance at initial instant
  dy12 = y10 - y20;  // y-distance at initial instant
  dalpha12 = alpha10 - alpha20;

  if ( AlmostEqual(dt, 0) )
  { // almost equal start and end time -> constant distance
    result.a = 0.0;
    result.b = 0.0;
    result.c =  sqrt( pow( ( (x11-x10) - (x21-x20) ) / 2, 2)
        + pow( ( (y11-y10) - (y21-y20) ) / 2, 2)
        + pow( ( (alpha11-alpha10) - (alpha21-alpha20) ) / 2, 2));
    result.r = false;
    return;
  }

  double a1 = (pow((dx1-dx2),2)+pow(dy1-dy2,2)+pow(dalpha1-dalpha2,2))
  /pow(dt,2);
  double b1 = dx12 * (dx1-dx2);
  double b2 = dy12 * (dy1-dy2);
  double b3 = dalpha12 * (dalpha12-dalpha2);

  result.a = a1;
  result.b = 3*(b1+b2+b3)/dt;
  result.c = pow(dx12,2) + pow(dy12,2)+pow(dalpha12,2);
  result.r = true;
  return;
}

// scalar velocity
void UMove::USpeed( UReal& result, const Geoid* geoid ) const
{
  double duration = 0.0;;
  double dist = 0.0;
  bool valid = true;
  if ( !IsDefined() ) {
    valid = false;
  } else {
    result.timeInterval = timeInterval;

    DateTime dt = timeInterval.end - timeInterval.start;
    duration = dt.millisecondsToNull() / 1000.0;   // value in seconds

    if( duration > 0.0 ){
      
        double x0 = p0.GetX(), y0 = p0.GetY(), alpha0 = p0.GetAlpha(),
               x1 = p1.GetX(), y1 = p1.GetY(), alpha1 = p1.GetAlpha();
        /*
        The point unit is represented as a function
        f(t) = (x0 + x1 * t, y0 + y1 * t).
        The result of the derivation is the constant (x1,y1).
        */
        dist = sqrt(pow( (x1-x0), 2 ) + pow( (y1- y0), 2 ) +
        pow((alpha1-alpha0),2));
        valid = true;
 
      /*
      The speed is constant in each time interval.
      Its value is represented by variable c. The variables a and b
      are set to zero.
      */
      result.a = 0;  // speed is constant in the interval
      result.b = 0;
      result.c = dist/duration;
      result.r = false;
    } else { // duration <= 0.0
      valid = false;
    }
  }
  result.SetDefined(valid);
}

// component-wise velocity
void UMove::UVelocity( UMove& result ) const
{
  double x0, y0, alpha0, x1, y1, alpha1;
  double duration;

  if ( ! IsDefined() )
      result.SetDefined( false );
  else
    {
      x0 = p0.GetX();
      y0 = p0.GetY();
      alpha0 = p0.GetAlpha();
      x1 = p1.GetX();
      y1 = p1.GetY();
      alpha1 = p1.GetAlpha();
      DateTime dt = timeInterval.end - timeInterval.start;
      duration = dt.millisecondsToNull() / 1000.0;   // value in seconds

      if( duration > 0.0 )
        {
          UMove p(timeInterval,
                   (x1-x0)/duration,(y1-y0)/duration, // velocity is constant
                   (alpha1-alpha0)/duration,
                   (x1-x0)/duration,(y1-y0)/duration,  // throughout the unit
                   (alpha1-alpha0)/duration);
          p.SetDefined( true );
          result.CopyFrom( &p );
          result.SetDefined( true );
        }
      else
        {
          UMove p(timeInterval,0,0,0,0,0,0);
          result.CopyFrom( &p );
          result.SetDefined( false );
        }
    }
}

void UMove::UTrajectory( Line& line ) const
{
  line.Clear();
  if( !IsDefined() ){
    line.SetDefined( false );
    return;
  }
  line.SetDefined( true );
  HalfSegment hs;
  int edgeno = 0;

  line.StartBulkLoad();
  if( !AlmostEqual( p0, p1 ) )
        {
          hs.Set( true, Point(true, p0.GetX(), p0.GetY()), 
           Point(true, p1.GetX(), p1.GetY()) );
          hs.attr.edgeno = edgeno++;
          line += hs;
          hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
          line += hs;
        }
  line.EndBulkLoad(true,false); // avoid realminize
}

void UMove::Length( CcReal& result ) const
{
  if( !this->IsDefined() || !p0.IsDefined() || !p0.IsDefined() ){
    result.Set(false, 0.0);
    return;
  }
  result.Set(true, p0.Distance(p1));
  return;
}

void UMove::Length( const Geoid& g, CcReal& result ) const
{
  if( !this->IsDefined() || !p0.IsDefined() || !p0.IsDefined() ){
    result.Set(false, 0.0);
    return;
  }
  bool valid = true;
  result.Set(true, p0.Distance(p1));
  result.SetDefined(valid);
  return;
}


// This function will return the intersection of two upoint values as
// an upoint value. If the common timeInterval iv is open bounded, and
// both units would intersect at the open interval limit, there WILL
// be passed a result, though it is not inside iv!
void UMove::Intersection(const UMove &other, UMove &result) const
{
      if ( !IsDefined() ||
           !other.IsDefined() ||
           !timeInterval.Intersects( other.timeInterval ) )
      {
          result.SetDefined( false );

          assert ( !result.IsDefined() || result.IsValid() );
          return; // nothing to do
      }
      Interval<Instant> iv;
      Instant t;
      Point3 p_intersect, d1, d2, p1;
      UMove p1norm(true), p2norm(true);
      double t_x, t_y,  t1, t2, dxp1, dxp2, dyp1, dyp2,dt;
      bool intersectionfound = false;

      result.timeInterval.start = DateTime(0,0,instanttype);
      result.timeInterval.end   = DateTime(0,0,instanttype);
      result.timeInterval.start.SetDefined(true);
      result.timeInterval.end.SetDefined(true);
      result.SetDefined(false);

      if (timeInterval == other.timeInterval)
      { // identical timeIntervals
        p1norm = *this;
        p2norm = other;
        iv = timeInterval;
      }
      else
      { // get common time interval
        timeInterval.Intersection(other.timeInterval, iv);
        // normalize both starting and ending points to interval
        AtInterval(iv, p1norm);
        other.AtInterval(iv, p2norm);
      }


      // test for identity:
      if ( p1norm.EqualValue( p2norm ))
        { // both upoints have the same linear function
          result = p1norm;

          assert ( !result.IsDefined() || result.IsValid() );
          return;
        }

      // test for ordinary intersection of the normalized upoints
      d1 = p2norm.p0 - p1norm.p0; // difference vector at starting instant
      d2 = p2norm.p1 - p1norm.p1; // difference vector at ending instant
      if ( ((d1.GetX() > 0) && (d2.GetX() > 0)) ||
           ((d1.GetX() < 0) && (d2.GetX() < 0)) ||
           ((d1.GetY() > 0) && (d2.GetY() > 0)) ||
           ((d1.GetY() < 0) && (d2.GetY() < 0))||
           ((d1.GetAlpha() < 0) && (d2.GetAlpha() < 0))||
           ((d1.GetAlpha() > 0) && (d2.GetAlpha() > 0)))
        { // no intersection (projections to X/Y do not cross each other)
          result.SetDefined( false );
          assert ( !result.IsDefined() || result.IsValid() );
          return; // nothing to do
        }
      // Some intersection is possible, as projections intersect...
      dxp1 = (p1norm.p1 - p1norm.p0).GetX(); // arg1: X-difference
      dyp1 = (p1norm.p1 - p1norm.p0).GetY(); // arg1: Y-difference
      dxp2 = (p2norm.p1 - p2norm.p0).GetX(); // arg2: X-difference
      dyp2 = (p2norm.p1 - p2norm.p0).GetY(); // arg2: Y-difference

/*
Trying to find an intersection point $t$ with $A_1t + B_1 = A_2t + B_2$
we get:

\[ t_x = \frac{px_{21} - px_{11}}{dxp_1 - dxp_2} \quad
t_y = \frac{py_{21} - py_{11}}{dyp_1 - pyp_2} \]

where $t = t_x = t_y$. If $t_x \neq t_y$, then there is no intersection!

*/

      dt = (iv.end - iv.start).ToDouble();

      t1 = iv.start.ToDouble();
      t2 = iv.end.ToDouble();

      t_x = (dt*d1.GetX() + t1*(dxp1-dxp2)) / (dxp1-dxp2);
      t_y = (dt*d1.GetY() + t1*(dyp1-dyp2)) / (dyp1-dyp2);

      // Standard case: (dxp1-dxp2) != 0.0 != (dyp1-dyp2)
      if ( AlmostEqual(t_x, t_y) && ( t_x >= t1) && ( t_x <= t2) )
        { // We found an intersection

          t.ReadFrom(t_x); // create Instant
          intersectionfound = true;
        }
      // Special case: (dxp1-dxp2) == 0.0 -- constant X
      else if ( AlmostEqual(dxp1-dxp2, 0.0) )
        {

          t_y = t1 + d1.GetY() * dt / (dyp1 - dyp2);
          t.ReadFrom(t_y); // create Instant
          intersectionfound = true;
        }
      // Special case: (dyp1-dyp2) == 0.0 -- constant Y
      else if ( AlmostEqual(dyp1-dyp2, 0.0) )
        {

          t_x = t1 + d1.GetX() * dt / (dxp1 - dxp2);
          t.ReadFrom(t_x); // create Instant
          intersectionfound = true;
        }
      if ( intersectionfound )
        {
          t.SetType(instanttype); // force instanttype
          iv = Interval<Instant>( t, t, true, true ); // create Interval
          TemporalFunction(t, p1, true);
          result = UMove(iv, p1, p1);

            assert ( !result.IsDefined() || result.IsValid() );
            return;
        }
      // else: no result

      result.SetDefined( false );
      assert ( !result.IsDefined() || result.IsValid() );
      return;
}

void UMove::Translate(const double xdiff, const double ydiff,
               const double alphadiff, const DateTime& timediff)
{
  assert( IsDefined() );
  assert( timediff.IsDefined() );

  p0.Set(p0.GetX()+xdiff,p0.GetY()+ydiff, p0.GetAlpha()+alphadiff);
  p1.Set(p1.GetX()+xdiff, p1.GetY()+ydiff, p1.GetAlpha()+alphadiff);
  timeInterval.start = timeInterval.start + timediff;
  timeInterval.end = timeInterval.end + timediff;
}

