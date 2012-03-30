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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[bl] [\\]

[1] Implementation of Module

January 2004 Victor Almeida

March - April 2004 Zhiming Ding

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

12.03.2006 Juergen Schmidt seperated the ~=~ and ~\#~ Operater
because of wrong syntax of these operators.
For T in (bool, int, real, point) it should be mT x mT -> mbool,
but it was mT x mT -> bool. The new operators ~equal~ and
~nonequal~ represents the old meaning. The operators with the
"right" syntax can be found in TemporalLiftedAlgebra.
(Some formating work is done to re-check in the algebra to cvs)

09.04.2006 seperated the ~isempty~ Operater
because of wrong syntax of this operators.
For T in (bool, int, real, point) it should be mT -> mbool with
TRUE as result if the argument is defined and FALSE when the
argument is not defined, but it was mT -> bool with TRUE when it is
defined somewhere in time. (The total implementation has been left in
place.)
The operator with the "right" syntax can be found in
TemporalLiftedAlgebra.

04.06.2009 Christian D[ue]ntgen 
   renamed ~bbox~: ~rT~ [->] ~rT~ to ~mbrange~: ~rT~ [->] ~rT~
   Added operators ~bbox~: 
           ~periods~ [->] ~rect3~, ~bbox~: ~instant~ [->] ~rect3~.

01.06.2006 Christian D[ue]ntgen added operator ~bbox~ for ~range~-types.

Sept 2006 Christian D[ue]ntgen implemented ~defined~ flag for unit types

Febr 2007 Christian D[ue]ntgen implemented ~bbox~ for mpoint and ipoint.
          Added operator ~bbox2d~ to save time when creating spatial indexes

September 2009 Simone Jandt: UReal new Member CompUReal implemented.

29.09.2009 Mahmoud Sakr: Added the operators: delay, distancetraversed, and
mint2mbool

27.09.2010 Christian D[ue]ntgen added operator ~turns~.

30.11.2011 Mahmoud Sakr added operator ~when~.
[TOC]

1 Overview

This file contains the implementation of the type constructors ~instant~,
~range~, ~intime~, ~const~, and ~mapping~. The memory data structures
used for these type constructors are implemented in the TemporalAlgebra.h
file.

2 Defines, includes, and constants

*/
#include <cmath>
#include <limits>
#include <iostream>
#include <sstream>
#include <string>
#include <stack>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "PolySolver.h"
#include "RelationAlgebra.h"
#include <math.h>
#include "MMRTree.h"
#include <time.h>
#include "AlmostEqual.h"
#include "ListUtils.h"
#include "Symbols.h"
#include "Geoid.h"
#include "MovingRegionAlgebra.h"

#include "RefinementStream.h"
#include "TemporalUnitAlgebra.h"


extern NestedList* nl;
extern QueryProcessor* qp;

#include "DateTime.h"
#include "TemporalAlgebra.h"
#include "GenericTC.h"
#include "GenOps.h"
#include "Stream.h"



#ifdef SECONDO_WIN32
double asinh(double z)
{
  return log(z + sqrt(z*z +1));
}

bool isnan(double x)
{
  return ((x!=x) || ((x==1) && (x == 0)));
}

#endif


static int counter;


string int2string(const int& number)
{
  ostringstream oss;
  oss << number;
  return oss.str();
}

bool IsMaximumPeriods(const Periods& p)
{
  if(p.GetNoComponents() != 1) return false;
  Interval<Instant> I;
  p.Get(0, I);
  return (I.start.IsMinimum() && I.end.IsMaximum());
}
/*
1.1 Definition of some constants

*/
const bool TA_DEBUG = false;  // debugging off
// const bool TA_DEBUG = true;  // debugging on

const double MAXDOUBLE = numeric_limits<double>::max();
const double MINDOUBLE = -1.0 * numeric_limits<double>::max();

/*
3 Implementation of C++ Classes

3.1 Class ~UReal~

*/

template<class alpha>
ostream& operator<<(ostream& o, const Interval<alpha>& u){
   return u.Print(o);
}

ostream& operator<<(ostream& o, const UPoint& u){
   ios_base::fmtflags oldOptions = o.flags();
   o.setf(ios_base::fixed,ios_base::floatfield);
   o.precision(8);
   if(!u.IsDefined()){
       o << "Undefined";
   } else {
       o << "UPoint[" << u.timeInterval << ", " << u.p0 << ", " << u.p1 << "]";
   }
   o.flags(oldOptions);
   return o;
}

void UReal::TemporalFunction( const Instant& t,
                              CcReal& result,
                              bool ignoreLimits ) const
{
  if ( !this->IsDefined() ||
       !t.IsDefined() ||
       (!this->timeInterval.Contains( t ) && !ignoreLimits) )
    {
      result.Set(false, 0.0);
    }
  else
    {
      DateTime T2(durationtype);
      T2 = t - timeInterval.start;
      double t2 = T2.ToDouble();
      double res = a * pow( t2, 2 ) +
        b * t2 +
        c;

      if( r ){
        if(res>=0.0){
           res = sqrt( res );
        } else {
           if(AlmostEqual(res,0.0)){ // correction of rounding errors
              res = 0.0;
           } else {
             assert(false);
           }
        }
      }
      result.Set( true, res );
    }
}

bool UReal::Passes( const CcReal& val ) const
{
  assert( IsDefined()  && val.IsDefined() );

  Periods times(2);
  int no_res = PeriodsAtVal( val.GetRealval(), times);
  for(int i=0; i<no_res; i++)
  {
    Interval<DateTime> iv;
    times.Get(i, iv);
    // only return true, iff the value is *REALLY* reached!
    if( iv.Inside(timeInterval) )
      return true;
  }
  return false;
}

bool UReal::At( const CcReal& val, TemporalUnit<CcReal>& result ) const
  // CD - Implementation causes problem, as the result could be a set of
  // 0-2 Units!
  // Use UReal::PeriodAtValue() or UReal::AtValue() instead!
{
  cerr << "UReal::At() is not implementable! Use UReal::AtValue() instead!"
       << endl;
  assert( false );
  return false;
}

void UReal::AtInterval( const Interval<Instant>& i,
 TemporalUnit<CcReal>& result ) const
{
  UReal *pResult = (UReal*)&result;

  if( !IsDefined() || !timeInterval.Intersects(i) ){
    pResult->SetDefined( false );
    return;
  }
  pResult->SetDefined(true);
  TemporalUnit<CcReal>::AtInterval( i, result );
  pResult->a = a;
  pResult->b = b;
  pResult->c = c;
  pResult->r = r;

  // Now, we need to translate the result to the starting instant
  DateTime tmp = pResult->timeInterval.start;
  tmp.SetType(durationtype);
  double tx = -((timeInterval.start - tmp).ToDouble());
  pResult->TranslateParab(tx);
}

// translate the parabolic/linear/constant curve within a ureal
// by (t) on the x/time-axes
// the ROOT flag is not considered at all!
void UReal::TranslateParab(const double& t)
{
  c = a*pow(t,2) + b*t + c;
  b = 2*a*t      + b      ;
  // a = a;
  if(timeInterval.start == timeInterval.end)
  {
    a = 0; b = 0;
  }
  if( r && (a==0.0) && (b==0.0) )
  {
    r = false;
    if(c<0){
       if( AlmostEqual(c,0.0) )
       {
          c = 0;
       } else
       {
         bool valid= false;
         assert(valid);
       }
    }
    c = sqrt(c);
  }
}


double AntiderivativeSQRTpoly2(const double a, const double b,
                               const double c, const double x)
{ // Bronstein, Taschenbuch der Mathematik 21.5.2.8 (245)
  assert(a != 0);

  double X = x*(a*x+b)+c;
  double Delta = 4*a*c - b*b;
  double h1 = 2*a*x+b;
  double Xr = sqrt(X);
  double t1 = (h1*Xr)/(4*a);

  double f = 0.5*c - (b*b) / (8*a);  // 1 / 2k

  double t2 = 0;

  // Bronstein (241)
  double anti2;
  int ucase = -1;
  if((a>0) && (Delta>0))  {
     ucase = 1;
     //anti2 = (1/sqrt(a)) * asinh(h1 / sqrt(Delta));
     //t2 = f*anti2;

      t2 = (Delta / (8*pow(a,1.5))) * asinh(h1/sqrt(Delta));


  } else if((a>0) && (Delta==0)){
     ucase = 2;
     anti2 = (1/sqrt(a)) * log(h1);
     t2 = f * anti2;
  } else if((a<0) && (Delta<0)){
     ucase = 3;
     anti2 = -1.0 * (1/ sqrt(-1.0*a)) * asin( h1 / sqrt(-1.0*Delta));
     t2 = f * anti2;
  } else if (a>0) { // && Delta < 0
     ucase = 4;
     anti2 = (1 / sqrt(a)) * log ( 2.0 * sqrt(a*X) + h1);
     t2 = f*anti2;
  } else {
     cerr << " invalid case reached " << endl;
     anti2 = 0;
  }
  double result = t1 + t2;

  /* Unfortunately , small rounding errors in the anti2 computation
     are often multiplied with very large numbers for (f) in many
     datasets. This leads to very wrong results if these data are used.
     For this reason, we compute the valid range of the result. If the
     computed result is outside of this value, we approximate the integral
     by the integral of a linear function between the values at the boundaries.
     (the same as integrate(linearize2(unit))
   */

  // compute the minimum and the maximum value
  double xs = -1.0*b / (2*a);
  double v1 = sqrt(c);
  double v2 = sqrt(a*x*x+b*x+c);
  double v3 = v1;
  if(xs>0 && xs < x){
      v3 = sqrt(a*xs*xs+b*xs+c);
  } else {
      xs = -1.0;
  }
  double min = v1;
  if(v2 < min){
     min = v2;
  }
  if(v3<min){
     min = v3;
  }
  double max = v1;
  if(v2 > max){
     max = v2;
  }
  if(v3>max){
     max = v3;
  }

  double minint = min*x;
  double maxint = max*x;

  if( (result < minint) || (result > maxint)){
     //cerr << " approximate used case:" << ucase << endl;
     if(xs<=0){ // approximate lineary between 0 and x
         double h = (x==0)?0:(v2-v1)/x;
         result = (0.5*h*x + v1)*x;
     } else {
        // integrate 0..xs
        double h = (v3-v1)/x;
        double res1 = (0.5*h*xs + v1)*xs;
        // integrate xs .. x
        double diff = x-xs;
        h = (v2-v3)/diff;
        double res2 = (0.5*h*diff+v3)*diff;
        result = res1+res2;
     }
     if( (result < minint) || (result > maxint)){
          cerr << " error in approximation " << endl;
          cerr << " range is " << minint << " ,  " << maxint << endl;
          cerr << " but result is " << result << endl;
     }
  }
  return  result;
}



// integrate an ureal over its deftime
// Precondition: this is defined
double UReal::Integrate() const
{
  assert ( IsDefined() );
  double t = (timeInterval.end - timeInterval.start).ToDouble();
  if(!r) { // simple case without square root
      // form: ax^2 + bx + c
      return a*t*t*t/3.0 + 0.5*b*t*t + c*t;
  }
  // square root
  if ( a == 0.0 && b == 0.0)
  {  // form: sqrt(c)
    return   sqrt(c) * t;
  }
  if ( a == 0.0 && b != 0.0 )
  { // form : sqrt(bx + c)
     double X = b*t+c;
     return (2/ (3*b))* sqrt(X*X*X);
  }
  // form : sqrt ( ax^2 + bx + c)
  double res = AntiderivativeSQRTpoly2(a, b, c, t);
  return res;

}


/*
This function computes the maximum of this UReal.

*/
double UReal::Max(bool& correct) const{
  if(!IsDefined()){
    correct=false;
    return 0.0;
  }

  double t = (timeInterval.end - timeInterval.start).ToDouble();
  correct = true;

  double v1 = c;  // == TemporalFunction(t0);
  double v2 = a*t*t + b*t + c; // TemporalFunction (t1);

  double v3 = c; // value for extremum
  if(!AlmostEqual(a,0)){
     double ts = (-1.0*b)/(2.0*a);
     if( (ts>0) && (ts < t)){
         v3 = a*ts*ts + b*ts + c;
     }
  }
  // debug
  //if(isnan(v1) || isnan(v2) || isnan(v3)){
  //    cerr << " cannot determine the value within a unit" << endl;
  //}

  // determine the maximum of v1 .. v3
  double max = v1;
  if(v2>max){
     max = v2;
  }
  if(v3 > max){
     max = v3;
  }

  if(r){
    return sqrt(max);
  } else {
    return max;
  }
}

/*
This function computes the minimum of this UReal.

*/
double UReal::Min(bool& correct) const{
  if(!IsDefined()){
     correct=false;
     return  0.0;
  }

  double t = (timeInterval.end - timeInterval.start).ToDouble();
  correct = true;
  double v1 = c;  // == TemporalFunction(t0);
  double v2 = a*t*t + b*t + c; // TemporalFunction (t1);

  double v3 = c; // value for extremum
  if(!AlmostEqual(a,0)){
     double ts = (-1.0*b)/(2.0*a);
     if( (ts>0) && (ts < t)){
        // v3 = a*ts*ts + b*ts + c;
        v3 = -0.25*b*b/a + c;
     }
  }
  // debug
  //if(isnan(v1) || isnan(v2) || isnan(v3)){
  //    cerr << "UReal::Min(): cannot determine the value within a unit"
  //         << endl;
  //}
  // determine the minimum of v1 .. v3
  double min = v1;
  if(v2<min){
     min = v2;
  }
  if(v3 < min){
     min = v3;
  }
  if(r){
    if(min<0){
       return 0;
    }
    return sqrt(min);
  } else {
    return min;
  }
}

/*
Replaces this uReal by a linear approximation between the value at the start
and the end.

*/
void UReal::Linearize(UReal& result) const{
    if(!IsDefined()){
       result.SetDefined(false);
       return;
    }
    CcReal V;
    TemporalFunction(timeInterval.start,V,true);
    double v1 = V.GetRealval();
    TemporalFunction(timeInterval.end,V,true);
    double v2 = V.GetRealval();
    result = UReal(timeInterval,v1,v2);
}

/*
Computes a linear approximation of this ureal.
If the extremum of the represented function is contained in
the corresponding interval, the unit is split into two ones.

*/
void UReal::Linearize(UReal& result1, UReal& result2) const{
    if(!IsDefined()){
        result1.SetDefined(false);
        result2.SetDefined(false);
        return;
    }

    if((a==0) && !r){  // already linear
      result1.CopyFrom(this);
      result2.SetDefined(false);
      return;
    }

    CcReal V;
    TemporalFunction(timeInterval.start,V,true);
    double v1 = V.GetRealval();
    TemporalFunction(timeInterval.end,V,true);
    double v2 = V.GetRealval();

    if( a==0 ) {  // sqrt of a linear function
        result1 = UReal(timeInterval,v1,v2);
        result1.SetDefined(true);
        result2.SetDefined(false);
        return;
    }

    double xs = (-1.0*b) / (2.0*a);

    Instant ixs(durationtype);
    ixs.ReadFrom(xs); // convert the double into an instant
    Instant ixst(instanttype);
    ixst = timeInterval.start + ixs; // translate into the interval

    if( (ixst <= timeInterval.start) || (ixst>=timeInterval.end) ||
        ixst.Adjacent(&timeInterval.end)){
        // extremum outside or very close to the bounds
        result1 = UReal(timeInterval,v1,v2);
        result1.SetDefined(true);
        result2.SetDefined(false);
        return;
    }

    // extremum found inside this interval

    TemporalFunction(ixst,V,true);
    double v3 = V.GetRealval();

    Interval<Instant> interval1(timeInterval.start,ixst,timeInterval.lc,true);
    Interval<Instant> interval2(ixst,timeInterval.end,false,timeInterval.rc);

    result1 = UReal(interval1,v1,v3);
    result1.SetDefined(true);
    result2 = UReal(interval2,v3,v2);
    result2.SetDefined(true);
}




/*
Sets the Periods value to the times, where this takes the
specified value. Returns the number of results (0-2).

WARNING: May return points, that are not inside this->timeInterval,
         if a value is located at an open start/end instant.

*/
int UReal::PeriodsAtVal( const double& value, Periods& times) const
{
  double inst_d[2]; // instants as double
  int no_res = 0;
  DateTime t0(durationtype), t1(instanttype);
  Interval<Instant> iv;

//   cout << "UReal::PeriodsAtVal( " << value << ", ...) called." << endl;
//   cout << "\ta=" << a << " b=" << b << " c=" << c << " r=" << r << endl;
//   cout << "\tstart=" << timeInterval.start.ToDouble()
//        << " end=" << timeInterval.end.ToDouble()
//        << " lc=" << timeInterval.lc
//        << " rc=" << timeInterval.rc << endl;
  times.Clear();
  if( !IsDefined() )
  {
//      cout << "UReal::PeriodsAtVal(): Undefined UReal -> 0 results." << endl;
    times.SetDefined( false );
    return 0;
  }
  times.SetDefined( true );

  if( a==0.0 && b==0.0 )
    // special case: constant ureal
    {
//       cout << "UReal::PeriodsAtVal(): constant case" << endl;
      if ( (!r && AlmostEqual(c, value)) ||
           (r &&
             (AlmostEqual(sqrt(c), value) || AlmostEqual(c, value*value) ) ) )
      {
        times.StartBulkLoad();
        times.Add(timeInterval);
        times.EndBulkLoad();
//         cout << "UReal::PeriodsAtVal(): constant UReal -> 1 result." << endl;
        return times.GetNoComponents();
      }
      else // no result
      {
//      cout << "UReal::PeriodsAtVal(): constant UReal -> 0 results." << endl;
        return 0;
      }
    }
  if( !r )
  {
//     cout << "UReal::PeriodsAtVal(): r==false" << endl;
    no_res = SolvePoly(a, b, (c-value), inst_d, true);
  }
  else
  {
//     cout << "UReal::PeriodsAtVal(): r==true" << endl;
    if (value < 0.0)
    {
//       cout << "UReal::PeriodsAtVal(): radix cannot become <0. -> 0 results."
//            << endl;
      return 0;
    }
    else
      no_res = SolvePoly(a, b, (c-(value*value)), inst_d, true);
  }
//   times.Print(cout);
//   cout << "SolvePoly found no_res=" << no_res << " results." << endl;
  for(int i=0; i<no_res; i++)
  {
//     cout << "Processing inst_d[" << i << "]=" << inst_d[i] << endl;
    t0.ReadFrom(inst_d[i]);
    t1 = timeInterval.start + t0;
//     cout << "\tt1=" << t1.ToDouble() << endl;
//     cout << "\tt1.IsDefined()=" << t1.IsDefined() << endl;
    if( (t1 == timeInterval.start) ||
        (t1 == timeInterval.end)   ||
        timeInterval.Contains(t1)     )
    {
//       cout << "\ttimes.IsValid()=" << times.IsValid() << endl;
      if( !times.Contains( t1 ) )
      {
        iv = Interval<Instant>(t1, t1, true, true);
        times.StartBulkLoad();
        times.Add(iv); // add only once
        times.EndBulkLoad();
      }
//    else
//     cout << "UReal::PeriodsAtVal(): not added instant (doublet)" << endl;
    }
//     cout << "UReal::PeriodsAtVal(): not added instant (outside)" << endl;
  }
//   cout << "UReal::PeriodsAtVal(): Calculated "
//        << times.GetNoComponents() << "results." << endl;
  return times.GetNoComponents();
}

/*
Sets the Periods value to the times, where this takes the
minimum value. Returns the number of results (0-2).

WARNING: May return points, that are not inside this->timeInterval,
         if a value is located at an open start/end instant.

*/
double UReal::PeriodsAtMin(bool& correct, Periods& times) const
{
  times.Clear();
  if( !IsDefined() )
  {
     correct = false;
     times.SetDefined( false );
     return numeric_limits<double>::infinity();
  }
  times.SetDefined( true );

  double t = (timeInterval.end - timeInterval.start).ToDouble();
  double ts = 0.0;
  correct = true;
  double min = numeric_limits<double>::infinity();
  Interval<Instant> iv;
  double v0 = c;               // TemporalFunction(t0);
  double v1 = a*t*t + b*t + c; // TemporalFunction(t1);
  // TemporalFunction for extremum:
  double v2 = numeric_limits<double>::infinity();
  if(!AlmostEqual(a,0)){
     ts = (-1.0*b)/(2.0*a);
     if( (ts>0) && (ts < t)){
         v2 = a*ts*ts + b*ts + c;
     }
  }
  if(isnan(v0) || isnan(v1) || isnan(v2))
  {
      cerr << "UReal::Min(): cannot determine the value within a unit" << endl;
      correct = false;
      return numeric_limits<double>::infinity();
  }
  // determine the minimum of v0 .. v2
  min = v0;
  if(v1<min){
     min = v1;
  }
  if(v2 < min){
     min = v2;
  }
  if( AlmostEqual(a, 0.0) && AlmostEqual(b, 0.0))
  { // constant unit - return complete deftime
    times.StartBulkLoad();
    times.Add(timeInterval);
    times.EndBulkLoad();
  }
  else // 1 or 2 minima
  {
    times.StartBulkLoad();
    if(v0 <= min)
    {
      iv = Interval<Instant>(timeInterval.start,timeInterval.start,true,true);
      times.Add(iv);
    }
    if(v1 <= min)
    {
      iv = Interval<Instant>(timeInterval.end,timeInterval.end,true,true);
      times.Add(iv);
    }
    if(v2 <= min)
    {
      DateTime TS(durationtype);
      TS.ReadFrom(ts);
      DateTime T1(instanttype);
      T1 = timeInterval.start + TS;
      if( !(T1<timeInterval.start) && !(T1>timeInterval.end) )
      {
        iv = Interval<Instant>(T1,T1,true,true);
        times.Add(iv);
      }
    }
    times.EndBulkLoad();
  }
  // return the minimum value
  correct = true;
  return ( r ? sqrt(min) : min );
}

/*
Sets the Periods value to the times, where this takes the
maximum value. Returns the number of results (0-2).

WARNING: May return points, that are not inside this->timeInterval,
         if a value is located at an open start/end instant.

*/
double UReal::PeriodsAtMax(bool& correct, Periods& times) const
{
  times.Clear();
  if( !IsDefined() )
  {
     correct = false;
     times.SetDefined( false );
     return -numeric_limits<double>::infinity();
  }
  times.SetDefined( true );

  double t = (timeInterval.end - timeInterval.start).ToDouble();
  double ts = 0.0;
  correct = true;
  double max = -numeric_limits<double>::infinity();
  Interval<Instant> iv;
  double v0 = c;               // TemporalFunction(t0);
  double v1 = a*t*t + b*t + c; // TemporalFunction(t1);
  // TemporalFunction for extremum:
  double v2 = -numeric_limits<double>::infinity();
  if(!AlmostEqual(a,0)){
     ts = (-1.0*b)/(2.0*a);
     if( (ts>0) && (ts < t)){
         v2 = a*ts*ts + b*ts + c;
     }
  }
  if(isnan(v0) || isnan(v1) || isnan(v2)){
      cerr << "UReal::Max(): cannot determine the value within a unit"
           << endl;
      correct = false;
      return numeric_limits<double>::infinity();
  }
  // determaxe the maximum of v0 .. v2
  max = v0;
  if(v1 > max){
     max = v1;
  }
  if(v2 > max){
     max = v2;
  }
  if( AlmostEqual(a, 0.0) && AlmostEqual(b, 0.0))
  { // constant unit - return complete deftime
    times.StartBulkLoad();
    times.Add(timeInterval);
    times.EndBulkLoad();
  }
  else // 1 or 2 maxima
  {
    times.StartBulkLoad();
    if(v0 >= max)
    {
      iv = Interval<Instant>(timeInterval.start,timeInterval.start,true,true);
      times.Add(iv);
    }
    if(v1 >= max)
    {
      iv = Interval<Instant>(timeInterval.end,timeInterval.end,true,true);
      times.Add(iv);
    }
    if(v2 >= max)
    {
      DateTime TS(durationtype);
      TS.ReadFrom(ts);
      DateTime T1(instanttype);
      T1 = timeInterval.start + TS;
      if( !(T1<timeInterval.start) && !(T1>timeInterval.end) )
      {
        iv = Interval<Instant>(T1,T1,true,true);
        times.Add(iv);
      }
    }
    times.EndBulkLoad();
  }
  // return the maximum value
  correct = true;
  return ( r ? sqrt(max) : max );
}

/*
Creates a vector of units, which are the restriction of this to
the periods, where it takes its minimum value.

*Precondition*: this[->]IsDefined()

*Result*: stores the resultununit into vector result and returns
          the number of results (1-2) found.

*WARNING*: AtMin may return points, that are not inside this[->]timeInterval,
           if a minimum is located at an open start/end instant.

*/
int UReal::AtMin(vector<UReal>& result) const
{
  assert( IsDefined() );
  result.clear();

  bool correct = true;
  Periods minTimesPeriods(2);
  minTimesPeriods.Clear();
//   double minVal = PeriodsAtMin(correct, minTimesPeriods);
  PeriodsAtMin(correct, minTimesPeriods);

  if(!correct)
    return 0;
//  cout << "UReal::AtMin(): minVal=" << minVal << endl;
  for(int i=0; i<minTimesPeriods.GetNoComponents(); i++)
  {
    Interval<DateTime> iv;
    minTimesPeriods.Get(i, iv);
    UReal unit = UReal( *this );
    correct = false;
    if( iv.Inside(timeInterval) )
    {
      AtInterval( iv, unit );
      correct = true;
    }
    else
    { // solve problem with min at open interval start/end!
      if( (iv.start == timeInterval.start) || (iv.end == timeInterval.end) )
      {
        UReal unit2(
          Interval<DateTime>(timeInterval.start, timeInterval.end, true, true),
          a, b, c, r );
        unit2.AtInterval( iv, unit );
        correct = true;
      }
    }
    if( correct )
      result.push_back(unit);
    else
      cerr << "UReal::AtMin(): This should not happen!" << endl;
  }
  return result.size();
}


/*
Creates a vector of units, which are the restriction of this to
the periods, where it takes its maximum value.

*Precondition*: this[->]IsDefined()

*Result*: stores the resultununit into vector result and returns
          the number of results (1-2) found.

*WARNING*: AtMax may return points, that are not inside this[->]timeInterval,
           if a maximum is located at an open start/end instant.

*/
int UReal::AtMax( vector<UReal>& result) const
{
  assert( IsDefined() );
  result.clear();

  bool correct = true;
  Periods maxTimesPeriods(2);
  maxTimesPeriods.Clear();
//   double maxVal = PeriodsAtMax(correct, maxTimesPeriods);
  PeriodsAtMax(correct, maxTimesPeriods);

  if(!correct)
    return 0;
//  cout << "UReal::AtMax(): maxVal=" << maxVal << endl;
//  cout << "timeInterval = (" << timeInterval.start.ToString() << " "
//       << timeInterval.end.ToString() << " "
//       << timeInterval.lc << " " << timeInterval.rc << ")" << endl;*/*/
  for(int i=0; i<maxTimesPeriods.GetNoComponents(); i++)
  {
    Interval<DateTime> iv;
    maxTimesPeriods.Get(i, iv);
//     cout << "iv = (" << iv->start.ToString() << " " << iv->end.ToString()
//          << " " << iv->lc << " " << iv->rc << ")" << endl;*/
    UReal unit = UReal( *this );
    correct = false;
    if( iv.Inside(timeInterval) )
    {
       AtInterval( iv, unit );
       correct = true;
    }
    else if( (iv.start==timeInterval.start) || (iv.end==timeInterval.end) )
    { // solve problem with max at open interval start/end!
      UReal unit2(
        Interval<DateTime>(timeInterval.start, timeInterval.end, true, true),
        a, b, c, r );
      unit2.AtInterval( iv, unit );
      correct = true;
    }
    if( correct )
      result.push_back(unit);
    else
      cerr << "UReal::AtMax(): This should not happen!" << endl;
  }
  return result.size();
}

/*
Creates a vector of units, which are the restriction of this to
the periods, where it takes a certain value.

*Precondition*: this[->]IsDefined() AND value.IsDefined()

*Result*: stores the resultununit into vector result and returns
          the number of results (1-2) found.

*WARNING*: AtMax may return points, that are not inside this[->]timeInterval,
           if a maximum is located at an open start/end instant.

*/
int UReal::AtValue(CcReal value, vector<UReal>& result) const
{
  assert( IsDefined() && value.IsDefined() );
  result.clear();

  Periods valTimesPeriods(2);
  int no_res = 0;
  double theVal = value.GetRealval();
//  cout << "UReal::AtVal(): theVal=" << theVal << endl;
  no_res = PeriodsAtVal( theVal, valTimesPeriods );
  for(int i=0; i<no_res; i++)
  {
    Interval<DateTime> iv;
    valTimesPeriods.Get(i, iv);
//     cout << (iv.lc ? "UReal::AtValue: iv=[ " : "iv=[ ")
//          << iv.start.ToString() << " "
//          << iv.end.ToString()
//          << (iv.rc ? " ]" : " [") << endl;
    UReal unit(true);
    unit = UReal(iv, 0.0, 0.0, theVal, false); // making things easier...
//     cout << "  Unit = ";
//     unit.Print(cout); cout << endl;
    result.push_back(unit);
  }
  return result.size();
}
/*
Sets the Periods value to the times, where both UReals takes the
same value. Returns the number of results (0-2).

*/
int UReal::PeriodsAtEqual( const UReal& other, Periods& times) const
{
  assert( IsDefined() );
  assert( other.IsDefined() );

  times.Clear();
  if( !IsDefined() || !other.IsDefined() ){
    times.SetDefined( false );
    return 0;
  }
  times.SetDefined( true );
  int no_res = 0;
  double inst_d[4];
  DateTime t0(durationtype), t1(instanttype);
  Interval<Instant> iv;
  Interval<Instant> commonIv;

  UReal u1(true), u2(true), udiff(true);
  if( !IsDefined()       ||
      !other.IsDefined() ||
      !timeInterval.Intersects(other.timeInterval) )
    return 0;
  timeInterval.Intersection(other.timeInterval, commonIv);
  // restrict units to common deftime
  AtInterval(commonIv, u1);
  other.AtInterval(commonIv, u2);
  if( u1.r == u2.r            &&
      AlmostEqual(u1.a, u2.a) &&
      AlmostEqual(u1.b, u2.b) &&
      AlmostEqual(u1.c, u2.c) )
  { // case 0: U1 and U2 implement same function
    times.StartBulkLoad();
    times.Add(commonIv);
    times.EndBulkLoad();
    return 1;
  }
  if( u1.r == u2.r )
  { // case 1: r1 == r2 (use UReal::PeriodsAtVal(0.0,...))
    udiff = UReal(commonIv, (u1.a-u2.a), (u1.b-u2.b), (u1.c-u2.c), u1.r);
    return udiff.PeriodsAtVal(0.0, times);
  }
  // case 2: r1 != r2 similar, but solve Polynom of degree=4
  // normalize, such that u1r==false and u2.r==true
  if (u1.r)
  { udiff = u1; u1 = u2; u2 = udiff; }
  // solve u1^2 = u2^2 <=> u1^2 - u2^2 = 0
  double A = u1.a;
  double B = u1.b;
  double C = u1.c;
  double D = u2.a;
  double E = u2.b;
  double F = u2.c;
  no_res = SolvePoly(A*A,
                     2*A*B,
                     2*A*C+B*B-D,
                     2*B*C-E,
                     C*C-F,
                     inst_d);

  for(int i=0; i<no_res; i++)
  {
    t0.ReadFrom(inst_d[i]);
    t1 = commonIv.start + t0;
    if( (t1 == commonIv.start) ||
        (t1 == commonIv.end)   ||
        commonIv.Contains(t1)     )
    {
//       cout << "\tt1=" << t1.ToDouble() << endl;
//       cout << "\tt1.IsDefined()=" << t1.IsDefined() << endl;
//       cout << "\ttimes.IsValid()=" << times.IsValid() << endl;
      if( !times.Contains( t1 ) )
      {
//         cout << "UReal::PeriodsAtEqual(): add instant" << endl;
        iv = Interval<Instant>(t1, t1, true, true);
        times.StartBulkLoad();
        times.Add(iv); // add only once
        times.EndBulkLoad();
      }
//    else
//    cout << "UReal::PeriodsAtEqual(): not added instant" << endl;
    }
  }
//cout << "UReal::PeriodsAtEqual(): Calculated "
//     << times.GetNoComponents() << "results." << endl;
  return times.GetNoComponents();
}

/*
Creates a vector of ubool, that cover the UReals common deftime and
indicate whether their temporal values are equal or not.

*Precondition*: this[->]IsDefined() AND value.IsDefined()

*Result*: stores the resultunit into vector result and returns
          the number of results found.

*/

int UReal::IsEqual(const UReal& other, vector<UBool>& result) const
{
  result.clear();
  cerr << "UReal::IsEqual() Not Yet Implemented!" << endl;
  return 0;
}

/*
Creates the absolute value for an UReal value.
~result~ may contain 0-3 UReal values.

*Precondition*: this[->]IsDefined()

*Result*: stores the resultunits into vector result and returns
          the number of results.

*/
int UReal::Abs(vector<UReal>& result) const
{
  assert( IsDefined() );
  result.clear();

  if( r )
  { // return the complete unit, as it should be positive
    result.push_back(UReal(timeInterval, a, b, c, r));
    return 1;
  } // else: r == false

  UReal newunit(true);
  Interval<Instant>
      iv(DateTime(0,0,instanttype), DateTime(0,0,instanttype), false, false),
      ivnew(DateTime(0,0,instanttype), DateTime(0,0,instanttype), false, false);
  Interval<Instant> actIntv;
  Instant
      start(instanttype),
      end(instanttype),
      testInst(instanttype);
  Periods *eqPeriods;
  vector< Interval<DateTime> > resPeriods;
  int i=0, numEq=0, cmpres=0;
  bool lc, rc;
  CcReal fccr1(true, 0.0), fccr2(true,0.0);

  // get times of intersection with time-axis
  // for each interval generated: if <0 invert all parameters
  resPeriods.clear();
  eqPeriods = new Periods(4);
  eqPeriods->Clear();
  PeriodsAtVal( 0.0, *eqPeriods);
  numEq = eqPeriods->GetNoComponents();// 1 instant herein (start==end)
//    cout << "  numEq=" << numEq << endl;

  // divide deftime into a vector of intervals
  if ( numEq == 0 )
  { // single result: complete timeInterval
    resPeriods.push_back(timeInterval);
  }
  else
  { //otherwise: numEq > 0
    //   create periods value for single result units
    start = timeInterval.start;
    end = timeInterval.start;
    lc = true;
    rc = false;
    for(i=0; i<numEq; i++)
    {
      rc = false;
      eqPeriods->Get(i, actIntv);
      end = actIntv.start;
      if(start == timeInterval.start)
      { // truncate at beginning
        lc = timeInterval.lc;
      }
      if(end == timeInterval.end)
      { // truncate at right end
        rc = timeInterval.rc;
      }
      if( (start == end) && !(lc && rc) )
      { // invalid instant: skip
        start = end;
        lc = !rc;
        continue;
      }
      ivnew = Interval<Instant>(start, end, lc, rc);
      resPeriods.push_back(ivnew);
      start = end;
      lc = !rc;
    }
    // do last interval
    if(end < timeInterval.end)
    { // add last interval
      ivnew = Interval<Instant>(end, timeInterval.end, lc, timeInterval.rc);
      resPeriods.push_back(ivnew);
    }
    else if( (end == timeInterval.end) &&
             !rc &&
             timeInterval.rc &&
             (resPeriods.size()>0)
           )
    { // close last interval
        resPeriods[resPeriods.size()-1].rc = true;
    }
  }

  // for each interval create one result unit
  for(i=0; i< (int) resPeriods.size();i++)
  { // for each interval test, whether one has to invert it
    AtInterval(resPeriods[i], newunit);
    testInst = resPeriods[i].start
        + ((resPeriods[i].end - resPeriods[i].start) / 2);
    TemporalFunction(testInst, fccr1, false);
    cmpres = fccr1.Compare( &fccr2 );
    if( cmpres < 0 )
    {
      newunit.a *= -1.0; newunit.b *= -1.0; newunit.c *= -1.0;
    }
    result.push_back(newunit);
  }

  eqPeriods->DeleteIfAllowed();
  return result.size();
}

/*
Creates the distance to an other UReal value.
~result~ may contain 0-3 UReal values.

*Precondition*: this[->]IsDefined() AND other.IsDefined()
                this[->]r $==$ other.r $==$ false

*Result*: stores the resultunits into vector result and returns
          the number of results.

*/
int UReal::Distance(const UReal& other, vector<UReal>& result) const
{
  assert( IsDefined() );
  assert( other.IsDefined() );
  assert( !r );
  assert( !other.r );

  result.clear();

  if( timeInterval.Intersects(other.timeInterval) )
  {
    UReal ur1(true);
    UReal ur2(true);
    UReal diff(true);
    Interval<Instant>
        iv(DateTime(0,0,instanttype), DateTime(0,0,instanttype), true, true);

    timeInterval.Intersection(other.timeInterval, iv);
    AtInterval(iv, ur1);
    other.AtInterval(iv, ur2);
    diff.timeInterval = iv;
    diff.a = ur1.a - ur2.a;
    diff.b = ur1.b - ur2.b;
    diff.c = ur1.c - ur2.c;
    diff.r = false;

    diff.Abs(result);
  }
  return result.size();
}

    void UReal::CompUReal(UReal& ur2, int opcode, vector<UBool>& res)
{
  UReal *u1  = this;
  UReal *u2  = &ur2;
  res.clear();
  if ( !u1->IsDefined() ||!u2->IsDefined() ||
       !u1->timeInterval.Intersects(u2->timeInterval))
  {
    return ;
  }
  // common deftime --> some result exists
  UReal un1(true), un2(true);
  UBool newunit(true);
  Interval<Instant>
    iv(DateTime(0,0,instanttype), DateTime(0,0,instanttype), false, false),
    ivnew(DateTime(0,0,instanttype), DateTime(0,0,instanttype), false, false);
  bool compresult, lc;
  u1->timeInterval.Intersection(u2->timeInterval, iv);
  u1->AtInterval(iv, un1);
  u2->AtInterval(iv, un2);
  if ( un1.r == un2.r &&
       AlmostEqual(un1.a, un2.a) &&
       AlmostEqual(un1.b, un2.b) &&
       AlmostEqual(un1.c, un2.c))
  { // equal ureals return single unit: TRUE for =, <=, >=; FALSE otherwise
    compresult = (opcode == 0 || opcode == 4 || opcode == 5);
    newunit = UBool(iv, CcBool(true, compresult));
    res.push_back(newunit);
    return ;
  }
  Periods *eqPeriods = new Periods(4);
  Interval<Instant> actIntv;
  Instant start(instanttype), end(instanttype), testInst(instanttype);
  int i, numEq, cmpres;
  CcReal fccr1(true, 0.0), fccr2(true,0.0);
  un1.PeriodsAtEqual(un2, *eqPeriods); // only intervals of length
  numEq = eqPeriods->GetNoComponents();// 1 instant herein (start==end)
  if ( numEq == 0 )
  { // special case: no equality -> only one result unit
    testInst = iv.start + ((iv.end - iv.start) / 2);
    un1.TemporalFunction(testInst, fccr1, false);
    un2.TemporalFunction(testInst, fccr2, false);
    cmpres = fccr1.Compare( &fccr2 );
    compresult = ( (opcode == 0 && cmpres == 0) ||   // ==
                   (opcode == 1 && cmpres != 0) ||   // #
                   (opcode == 2 && cmpres  < 0) ||   // <
                   (opcode == 3 && cmpres  > 0) ||   // >
                   (opcode == 4 && cmpres <= 0) ||   // <=
                   (opcode == 5 && cmpres >= 0)    );// >=
    newunit = UBool(iv, CcBool(true, compresult));
    res.push_back(newunit);
    eqPeriods->DeleteIfAllowed();
    return ;
  }
  // case: numEq > 0, at least one instant of equality
  // iterate the Periods and create result units
  start = iv.start;   // the ending instant for the next interval
  lc = iv.lc;
  i = 0;              // counter for instants of equality
  eqPeriods->Get(i, actIntv);
  // handle special case: first equality in first instant
  if (start == actIntv.start)
  {
    if (iv.lc)
    {
      un1.TemporalFunction(un1.timeInterval.start, fccr1, false);
      un2.TemporalFunction(un2.timeInterval.start, fccr2, false);
      cmpres = fccr1.Compare( &fccr2 );
      compresult = ( (opcode == 0 && cmpres == 0) ||
                     (opcode == 1 && cmpres != 0) ||
                     (opcode == 2 && cmpres  < 0) ||
                     (opcode == 3 && cmpres  > 0) ||
                     (opcode == 4 && cmpres <= 0) ||
                     (opcode == 5 && cmpres >= 0)    );
      ivnew = Interval<Instant>(start, start, true, true);
      newunit = UBool(ivnew, CcBool(true, compresult));
      res.push_back(newunit);
      lc = false;
    } // else: equal instant not in interval!
    i++;
  }
  while ( i < numEq )
  {
    eqPeriods->Get(i, actIntv);
    end = actIntv.start;
    ivnew = Interval<Instant>(start, end, lc, false);
    testInst = start + ((end - start) / 2);
    un1.TemporalFunction(testInst, fccr1, false);
    un2.TemporalFunction(testInst, fccr2, false);
    cmpres = fccr1.Compare( &fccr2 );
    compresult = ( (opcode == 0 && cmpres == 0) ||
                   (opcode == 1 && cmpres != 0) ||
                   (opcode == 2 && cmpres  < 0) ||
                   (opcode == 3 && cmpres  > 0) ||
                   (opcode == 4 && cmpres <= 0) ||
                   (opcode == 5 && cmpres >= 0)    );
    newunit = UBool(ivnew, CcBool(true, compresult));
    res.push_back(newunit);
    if ( !(end == iv.end) || iv.rc )
    {
      ivnew = Interval<Instant>(end, end, true, true);
      compresult = (opcode == 0 || opcode == 4 || opcode == 5);
      newunit = UBool(ivnew, CcBool(true, compresult));
      res.push_back(newunit);
    }
    start = end;
    i++;
    lc = false;
  }
  if ( start < iv.end )
  { // handle teq[numEq-1] < iv.end
    ivnew = Interval<Instant>(start, iv.end, false, iv.rc);
    testInst = start + ((iv.end - start) / 2);
    un1.TemporalFunction(testInst, fccr1, false);
    un2.TemporalFunction(testInst, fccr2, false);
    cmpres = fccr1.Compare( &fccr2 );
    compresult = ( (opcode == 0 && cmpres == 0) ||
                   (opcode == 1 && cmpres != 0) ||
                   (opcode == 2 && cmpres  < 0) ||
                   (opcode == 3 && cmpres  > 0) ||
                   (opcode == 4 && cmpres <= 0) ||
                   (opcode == 5 && cmpres >= 0)    );
    newunit = UBool(ivnew, CcBool(true, compresult));
    res.push_back(newunit);
  }
  eqPeriods->DeleteIfAllowed();
  return ;
}


/*
3.1 Class ~UPoint~

*/
void UPoint::TemporalFunction( const Instant& t,
                               Point& result,
                               bool ignoreLimits ) const
{
  TemporalFunction(t, result, 0, ignoreLimits);
}

void UPoint::TemporalFunction( const Instant& t,
                                       Point& result,
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
    result.Set( x, y );
    result.SetDefined(true);
  }
}

bool UPoint::Passes( const Point& p ) const
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
      return true;
  }
  else if( AlmostEqual( p0.GetY(), p1.GetY() ) &&
      AlmostEqual( p0.GetY(), p.GetY() ) )
    // If the segment is horizontal
  {
    if( ( p0.GetX() <= p.GetX() && p1.GetX() >= p.GetX() ) ||
        ( p0.GetX() >= p.GetX() && p1.GetX() <= p.GetX() ) )
      return true;
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

bool UPoint::Passes( const Point& val, const Geoid* geoid ) const {
  if(!geoid){
    return Passes( val );
  } else {
    UPoint result(false);
    bool retval = At( val, result, geoid );
    return retval && result.IsDefined();
  }
}

bool UPoint::Passes( const Region& r ) const
{
  assert( IsDefined() );
  assert( r.IsDefined() );

  if( AlmostEqual( p0, p1 ) )
    return r.Contains( p0 );

  HalfSegment hs( true, p0, p1 );
  return r.Intersects( hs );
}


bool UPoint::Passes( const Rectangle<2> &rect  ) const
{
  assert( IsDefined() );
  assert( rect.IsDefined() );

  if( AlmostEqual( p0, p1 ) )
    return p0.Inside(rect);
  HalfSegment uHs( true, p0, p1 );
  if( rect.Intersects( uHs.BoundingBox() ) )
  {
    if( p0.Inside(rect) || p1.Inside( rect) )
      return true;
    Point rP0(true, rect.MinD(0), rect.MinD(1));
    Point rP1(true, rect.MaxD(0), rect.MinD(1));
    Point rP2(true, rect.MaxD(0), rect.MaxD(1));
    Point rP3(true, rect.MinD(0), rect.MaxD(1));

    HalfSegment hs(true, rP0, rP1);
    if( hs.Intersects( uHs ) ) return true;
    hs.Set(true, rP1, rP2);
    if( hs.Intersects( uHs ) ) return true;
    hs.Set(true, rP2, rP3);
    if( hs.Intersects( uHs ) ) return true;
    hs.Set(true, rP3, rP1);
    if( hs.Intersects( uHs ) ) return true;
  }
  return false;
}

bool UPoint::At( const Point& p, TemporalUnit<Point>& res ) const {
  return At(p, res, 0);
}

bool UPoint::At( const Point& p,
                 TemporalUnit<Point>& res,
                 const Geoid* geoid ) const {

  assert(p.IsDefined());
  assert(this->IsDefined());
  assert( !geoid || geoid->IsDefined() );

  UPoint* result = static_cast<UPoint*>(&res);
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

  if(!geoid){// euclidean geometry case:
    double d_x = p1.GetX() - p0.GetX();
    double d_y = p1.GetY() - p0.GetY();
    double delta;
    bool useX;
    if(fabs(d_x)> fabs(d_y)){
      delta = (p.GetX()-p0.GetX() ) / d_x;
      useX = true;
    } else {
      delta = (p.GetY()-p0.GetY() ) / d_y;
      useX = false;
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
    if(useX){ // check y-value
      double y = p0.GetY() + delta*d_y;
      if(!AlmostEqual(y,p.GetY())){
        result->SetDefined(false);
        return false;
      }
    } else { // check x-value
      double x = p0.GetX() + delta*d_x;
      if(!AlmostEqual(x,p.GetX())){
        result->SetDefined(false);
        return false;
      }
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
  } else { // spherical geometry case:
    // get possible LON for given LAT
    double lonMinDEG = -1,  lonMaxDEG = -1;
    int numCross = p0.orthodromeAtLatitude( p1, p.GetY(),lonMinDEG, lonMaxDEG);
    if(numCross<1){ // no crossing with given LON
      result->SetDefined(false);
      return false;
    }
    double dist01 = p0.Distance(p1, geoid);
    double dist0cross  = -1;
    Instant tcross1(instanttype);
    tcross1.SetDefined(false);
    if(AlmostEqual(lonMinDEG,p.GetX())){ // calculate instant
      dist0cross = p0.Distance(Point(true, lonMinDEG, p.GetX()),geoid);
      tcross1 = t0 + (t1-t0)*(dist0cross/dist01);
      tcross1.SetDefined(timeInterval.Contains(tcross1));
    }
    Instant tcross2(instanttype);
    tcross2.SetDefined(false);
    if( ( numCross>=2) && AlmostEqual(lonMinDEG,p.GetX()) ){
      dist0cross = p0.Distance(Point(true, lonMaxDEG, p.GetX()),geoid);
      tcross2 = t0 + (t1-t0)*(dist0cross/dist01);
      tcross2.SetDefined(timeInterval.Contains(tcross2));
    }
    if( !tcross1.IsDefined() && !tcross2.IsDefined()){
      result->SetDefined(false);
      return false;
    }
    result->SetDefined(true);
    result->p0 = p;
    result->p1 = p;
    result->timeInterval.lc = true;
    result->timeInterval.rc = true;
    result->timeInterval.start = tcross1.IsDefined()?tcross1:tcross2;
    result->timeInterval.end   = tcross1.IsDefined()?tcross1:tcross2;
    return true;
  }
}


void UPoint::At(const Rectangle<2>& rect, UPoint& result) const{

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
  result.p0.Set(x1,y1);
  result.p1.Set(x2,y2);
}



void UPoint::AtInterval( const Interval<Instant>& i,
                         TemporalUnit<Point>& result ) const
{
  AtInterval( i, result, 0);
}

void UPoint::AtInterval( const Interval<Instant>& i,
                         TemporalUnit<Point>& result,
                         const Geoid* geoid) const
{
  assert( IsDefined() );
  assert( i.IsValid() );
  assert( !geoid || geoid->IsDefined() );

  TemporalUnit<Point>::AtInterval( i, result );

  UPoint *pResult = (UPoint*)&result;
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

void UPoint::Distance( const Point& p, UReal& result ) const {
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

void UPoint::DistanceOrthodrome( const Point& p,
                                 vector<UReal>& result,
                                 const Geoid geoid,
                                 const double epsilon,  /*=  0.00001 */
                                 const Instant* tMin,   /*=  0       */
                                 const double distMin,  /*= -666.666 */
                                 const double distStart,/*= -666.666 */
                                 const double distEnd   /*= -666.666 */) const {
  UReal resunit(true);
  bool ok(false);
  resunit.timeInterval = timeInterval;
  if(  !IsDefined() || !p.IsDefined() || !geoid.IsDefined() ) {
    resunit.SetDefined(false);
    result.push_back(resunit);
    return;
  }
  DateTime DT = timeInterval.end - timeInterval.start;
  double dt = DT.ToDouble();
  if ( AlmostEqual(dt, 0.0) || AlmostEqual(p0,p1)) { // constant/instant unit
    resunit.a = 0;
    resunit.b = 0;
    resunit.c = p.DistanceOrthodrome(p0,geoid,ok);
    resunit.c *= resunit.c;
    resunit.r = true; // draw square root
    resunit.SetDefined(ok);
    result.push_back(resunit);
    return;
  }
  double d0 = distStart;
  if(d0<0){ // need to calculate distance to p at start of unit
    d0 = p0.DistanceOrthodrome(p, geoid, ok);
    assert(ok);
  }
  double d1 = distEnd;
  if(d1<0){ // need to calculate distance to p at end of unit
    d1 = p1.DistanceOrthodrome(p, geoid, ok);
    assert(ok);
  }
  if(fabs(d1-d0)<epsilon){ // precision reached --> return single constant unit
    resunit.a = 0;
    resunit.b = 0;
    resunit.c = d0;
    resunit.c *= resunit.c;
    resunit.r = true; // draw square root
    resunit.SetDefined(ok);
    result.push_back(resunit);
    return;
  } // else: precision not yet reached --> refine
  // choose split point for refinement
  double dMin = distMin;
  Instant iMin(0.0);
  if(tMin){
    iMin = *tMin;
  } else {
    iMin.SetDefined(false);
  }
  Instant iSplit(instanttype);
  iSplit.SetDefined(false);
  if( (dMin<0) || !iMin.IsDefined() ){ // need to calculate minimum distance
    HalfSegment hs(true, p0, p1);
    dMin = hs.Distance( p, &geoid );
    assert(dMin>=0);
    // compute instant of nearest approach
    // TODO
  }
  if( (iMin>timeInterval.start) && (iMin<timeInterval.end) ){ //use minInstant
    iSplit = iMin;
  } else { // use middle of interval
    iSplit = timeInterval.start + (timeInterval.end - timeInterval.start)/2;
  }
  // recursive call to refine (start,splitpoint)
  Interval<Instant> i0(timeInterval.start,iSplit,timeInterval.lc,false);
  Interval<Instant> i1(iSplit,timeInterval.end,true,timeInterval.rc);
  UPoint u0(false);
  AtInterval(i0,u0);
  u0.DistanceOrthodrome( p,result,geoid,epsilon,&iMin,dMin,d0,d1 );
  // recursive call to refine (splitpoint,end)
  UPoint u1(false);
  AtInterval(i1,u1);
  u1.DistanceOrthodrome( p,result,geoid,epsilon,&iMin,dMin,d0,d1 );
}

void UPoint::Distance( const Point& p,
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
      x0 = p0.GetX(), y0 = p0.GetY(),
      x1 = p1.GetX(), y1 = p1.GetY(),
      x  =  p.GetX(), y  =  p.GetY();

    if ( AlmostEqual(dt, 0.0) || AlmostEqual(p0,p1)) {
      // single-instant or constant unit
      resunit.SetDefined(true);
      resunit.timeInterval = timeInterval;
      resunit.a = 0.0;
      resunit.b = 0.0;
      if(geoid){  // spherical distance squared
        bool ok=false;
        resunit.c = p.DistanceOrthodrome(p0,*geoid,ok);
        resunit.c *= resunit.c;
      } else {    // euclidean distance squared
        resunit.c = (pow(x0-x,2) + pow(y0-y,2));
      }
      resunit.r = true; // draw square root
      result.push_back(resunit);
    } else { // non-constant, non-instant unit
      if(geoid){  // spherical distance squared
        DistanceOrthodrome( p, result, *geoid, epsilon );
      } else {    // euclidean distance squared
        resunit.SetDefined(true);
        resunit.timeInterval = timeInterval;
        resunit.a = pow((x1-x0)/dt,2)+pow((y1-y0)/dt,2);
        resunit.b = 2*((x1-x0)*(x0-x)+(y1-y0)*(y0-y))/dt;
        resunit.c = pow(x0-x,2)+pow(y0-y,2);
        resunit.r = true; // draw square root
        result.push_back(resunit);
      }
    }
  }
}

double UPoint::Distance(const Rectangle<3>& rect,
                        const Geoid* geoid /*=0*/) const{
  cerr << "UPoint::Distance(const Rectangle<3>&) not implemented yet" << endl;
  if( !IsDefined() || !rect.IsDefined() || (geoid && !geoid->IsDefined()) ){
    return -1;
  }
  return BoundingBox().Distance(rect,geoid);
}

void UPoint::Distance( const UPoint& up,
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
    // use UPoint::AtValue(VALUEmin) to get Tmin
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
  Interval<Instant>iv;
  DateTime DT(durationtype);
  Point rp10, rp11, rp20, rp21;
  double
    x10, x11, x20, x21,
    y10, y11, y20, y21,
    dx1, dy1,
    dx2, dy2,
    dx12, dy12,
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
  x10 = rp10.GetX(); y10 = rp10.GetY();
  x11 = rp11.GetX(); y11 = rp11.GetY();
  x20 = rp20.GetX(); y20 = rp20.GetY();
  x21 = rp21.GetX(); y21 = rp21.GetY();
  dx1 = x11 - x10;   // x-difference final-initial for u1
  dy1 = y11 - y10;   // y-difference final-initial for u1
  dx2 = x21 - x20;   // x-difference final-initial for u2
  dy2 = y21 - y20;   // y-difference final-initial for u2
  dx12 = x10 - x20;  // x-distance at initial instant
  dy12 = y10 - y20;  // y-distance at initial instant

  if ( AlmostEqual(dt, 0) )
  { // almost equal start and end time -> constant distance
    result.a = 0.0;
    result.b = 0.0;
    result.c =   pow( ( (x11-x10) - (x21-x20) ) / 2, 2)
        + pow( ( (y11-y10) - (y21-y20) ) / 2, 2);
    result.r = true;
    return;
  }

  double a1 = (pow((dx1-dx2),2)+pow(dy1-dy2,2))/pow(dt,2);
  double b1 = dx12 * (dx1-dx2);
  double b2 = dy12 * (dy1-dy2);

  result.a = a1;
  result.b = 2*(b1+b2)/dt;
  result.c = pow(dx12,2) + pow(dy12,2);
  result.r = true;
  return;
}

// scalar velocity
void UPoint::USpeed( UReal& result, const Geoid* geoid ) const
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
      if(!geoid){ // (X,Y)-coords
        double x0 = p0.GetX(), y0 = p0.GetY(),
               x1 = p1.GetX(), y1 = p1.GetY();
        /*
        The point unit is represented as a function
        f(t) = (x0 + x1 * t, y0 + y1 * t).
        The result of the derivation is the constant (x1,y1).
        */
        dist = sqrt(pow( (x1-x0), 2 ) + pow( (y1- y0), 2 ));
        valid = true;
      } else { // (LON.LAT)-coords
        dist = p0.DistanceOrthodrome(p1,*geoid, valid);
      }
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
void UPoint::UVelocity( UPoint& result ) const
{
  double x0, y0, x1, y1;
  double duration;

  if ( ! IsDefined() )
      result.SetDefined( false );
  else
    {
      x0 = p0.GetX();
      y0 = p0.GetY();

      x1 = p1.GetX();
      y1 = p1.GetY();

      DateTime dt = timeInterval.end - timeInterval.start;
      duration = dt.millisecondsToNull() / 1000.0;   // value in seconds

      if( duration > 0.0 )
        {
          UPoint p(timeInterval,
                   (x1-x0)/duration,(y1-y0)/duration, // velocity is constant
                   (x1-x0)/duration,(y1-y0)/duration  // throughout the unit
                  );
          p.SetDefined( true );
          result.CopyFrom( &p );
          result.SetDefined( true );
        }
      else
        {
          UPoint p(timeInterval,0,0,0,0);
          result.CopyFrom( &p );
          result.SetDefined( false );
        }
    }
}

void UPoint::UTrajectory( Line& line ) const
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
          hs.Set( true, p0, p1 );
          hs.attr.edgeno = edgeno++;
          line += hs;
          hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
          line += hs;
        }
  line.EndBulkLoad(true,false); // avoid realminize
}

void UPoint::Length( CcReal& result ) const
{
  if( !this->IsDefined() || !p0.IsDefined() || !p0.IsDefined() ){
    result.Set(false, 0.0);
    return;
  }
  result.Set(true, p0.Distance(p1));
  return;
}

void UPoint::Length( const Geoid& g, CcReal& result ) const
{
  if( !this->IsDefined() || !p0.IsDefined() || !p0.IsDefined() ){
    result.Set(false, 0.0);
    return;
  }
  bool valid = true;
  result.Set(true, p0.DistanceOrthodrome(p1, g, valid));
  result.SetDefined(valid);
  return;
}

void UPoint::Direction( vector<UReal> &result,
                        const bool useHeading /*= false*/,
                        const Geoid* geoid    /*= 0*/,
                        const double epsilon  /*= 0.00001*/) const
{
  UReal uresult(true);
  if( !IsDefined() || (geoid && ( (epsilon<=0.0) || !geoid->IsDefined()) ) ) {
    return; // no result
  }
  if( (timeInterval.end == timeInterval.start) || AlmostEqual(p0,p1) ){
    // undefined result!
    uresult.a = 0.;
    uresult.b = 0.;
    uresult.c = 0.;
    uresult.r = false;
    uresult.timeInterval = timeInterval;
    uresult.SetDefined(false);
    result.push_back(uresult);
    return;
  }
  if(!geoid) { // euclidean case
    uresult.a = 0.;
    uresult.b = 0.;
    uresult.c = p0.Direction(p1,useHeading,geoid);
    uresult.r = false;
    uresult.timeInterval = timeInterval;
    uresult.SetDefined( uresult.c>=0 );
    result.push_back(uresult);
    return;
  }
  // spherical case:
  assert(epsilon > 0);
  //compute directions at start/end point

  bool valid = false;
  double head0, head1;
  p0.DistanceOrthodromePrecise( p1, *geoid, valid, head0, head1);
  if( !valid || (head0<0) || (head1<0) ){ // ERROR!
    cerr << __PRETTY_FUNCTION__ << ": Error computing directions." << endl;
    assert(false);
    return;
  }
  if(!useHeading){
    head0 = headingToDirection(head0);
    head1 = headingToDirection(head1);
  }
  cout << __PRETTY_FUNCTION__ << ": head0 = " << head0 << endl;
  cout << __PRETTY_FUNCTION__ << ": head1 = " << head0 << endl;
  double deltaHead = head1 - head0;
  cout << __PRETTY_FUNCTION__ << ": deltaHead = " << deltaHead << endl;
  if( (fabs(deltaHead)<=epsilon) ){ // sufficiently precise
    DateTime dt = timeInterval.end - timeInterval.start;
    uresult.a = 0.;
    uresult.b = deltaHead/dt.ToDouble();
    uresult.c = head0;
    uresult.r = false;
    uresult.timeInterval = timeInterval;
    uresult.SetDefined( true );
    result.push_back(uresult);
  } else { // preciseness not yet reached
    // split the unit, recurse into both parts. append the resulting
    // unit vectors to result.
    Instant t0(timeInterval.start);
    Instant t1(timeInterval.end);
    Instant t05 = t0+((t1-t0)/2);
    if( (t0==t05) || (t1==t05) ){ // invalid interval -> cannot split!
      uresult.a = 0.;
      uresult.b = deltaHead/(t1-t0).ToDouble();
      uresult.c = head0;
      uresult.r = false;
      uresult.timeInterval = timeInterval;
      uresult.SetDefined( true );
      result.push_back(uresult);
      return;
    }
    Interval<Instant> iv0(t0,t05,timeInterval.lc,false);
    Interval<Instant> iv1(t05,t1,true,timeInterval.rc);
    UPoint up0(false), up1(false);
    AtInterval(iv0,up0);
    AtInterval(iv1,up1);
    up0.Direction(result,useHeading,geoid,epsilon); // append results
    up1.Direction(result,useHeading,geoid,epsilon); // append results
  }
  return;
}

// This function will return the intersection of two upoint values as
// an upoint value. If the common timeInterval iv is open bounded, and
// both units would intersect at the open interval limit, there WILL
// be passed a result, though it is not inside iv!
void UPoint::Intersection(const UPoint &other, UPoint &result) const
{
      if ( !IsDefined() ||
           !other.IsDefined() ||
           !timeInterval.Intersects( other.timeInterval ) )
      {
          result.SetDefined( false );
          if (TA_DEBUG)
            cerr << "No intersection (0): deftimes do not overlap" << endl;
          assert ( !result.IsDefined() || result.IsValid() );
          return; // nothing to do
      }
      Interval<Instant> iv;
      Instant t;
      Point p_intersect, d1, d2, p1;
      UPoint p1norm(true), p2norm(true);
      double t_x, t_y, t1, t2, dxp1, dxp2, dyp1, dyp2, dt;
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

      if (TA_DEBUG)
      {
        cerr << "    p1norm=";
        p1norm.Print(cerr);
        cerr << endl << "    p2norm=";
        p2norm.Print(cerr);
        cerr << endl;
      }
      // test for identity:
      if ( p1norm.EqualValue( p2norm ))
        { // both upoints have the same linear function
          result = p1norm;
          if (TA_DEBUG)
          {
            cerr << "Found intersection (1): equal upoints" << endl
                 << "    Result=";
            result.Print(cerr);
            cerr << endl;
          }
          assert ( !result.IsDefined() || result.IsValid() );
          return;
        }

      // test for ordinary intersection of the normalized upoints
      d1 = p2norm.p0 - p1norm.p0; // difference vector at starting instant
      d2 = p2norm.p1 - p1norm.p1; // difference vector at ending instant
      if ( ((d1.GetX() > 0) && (d2.GetX() > 0)) ||
           ((d1.GetX() < 0) && (d2.GetX() < 0)) ||
           ((d1.GetY() > 0) && (d2.GetY() > 0)) ||
           ((d1.GetY() < 0) && (d2.GetY() < 0)))
        { // no intersection (projections to X/Y do not cross each other)
          if (TA_DEBUG)
            cerr << "No intersection (1) - projections do not intersect:"
                 << endl
                 << "  d1X=" << d1.GetX() << " d2X=" << d2.GetX() << endl
                 << "  d1Y=" << d1.GetY() << " d2Y=" << d2.GetY() << endl;
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

      if (TA_DEBUG)
        cerr << "  dt=" << dt << " t1=" << t1 << " t2=" << t2 << endl
             << "  (dxp1-dxp2)=" << (dxp1-dxp2)
             << " (dyp1-dyp2)=" << (dyp1-dyp2) << endl
             << "  t_x=" << t_x << " t_y=" << t_y << endl;

      // Standard case: (dxp1-dxp2) != 0.0 != (dyp1-dyp2)
      if ( AlmostEqual(t_x, t_y) && ( t_x >= t1) && ( t_x <= t2) )
        { // We found an intersection
          if (TA_DEBUG) cerr << "  Case 1: X/Y variable" << endl;
          t.ReadFrom(t_x); // create Instant
          intersectionfound = true;
        }
      // Special case: (dxp1-dxp2) == 0.0 -- constant X
      else if ( AlmostEqual(dxp1-dxp2, 0.0) )
        {
          if (TA_DEBUG) cerr << "  Case 2: constant X" << endl;
          t_y = t1 + d1.GetY() * dt / (dyp1 - dyp2);
          t.ReadFrom(t_y); // create Instant
          intersectionfound = true;
        }
      // Special case: (dyp1-dyp2) == 0.0 -- constant Y
      else if ( AlmostEqual(dyp1-dyp2, 0.0) )
        {
          if (TA_DEBUG) cerr << "  Case 3: constant Y" << endl;
          t_x = t1 + d1.GetX() * dt / (dxp1 - dxp2);
          t.ReadFrom(t_x); // create Instant
          intersectionfound = true;
        }
      if ( intersectionfound )
        {
          t.SetType(instanttype); // force instanttype
          iv = Interval<Instant>( t, t, true, true ); // create Interval
          TemporalFunction(t, p1, true);
          result = UPoint(iv, p1, p1);
          if (TA_DEBUG)
            {
              cerr << "Found intersection (2): intersection point" << endl
                   << "    Result=";
              result.Print(cerr);
              cerr << endl;
            }
            assert ( !result.IsDefined() || result.IsValid() );
            return;
        }
      // else: no result
      if (TA_DEBUG) cerr << "No intersection (2)." << endl;
      result.SetDefined( false );
      assert ( !result.IsDefined() || result.IsValid() );
      return;
}

void UPoint::Translate(const double xdiff, const double ydiff,
                       const DateTime& timediff)
{
  assert( IsDefined() );
  assert( timediff.IsDefined() );

  p0.Set(p0.GetX()+xdiff,p0.GetY()+ydiff);
  p1.Set(p1.GetX()+xdiff, p1.GetY()+ydiff);
  timeInterval.start = timeInterval.start + timediff;
  timeInterval.end = timeInterval.end + timediff;
}


bool UPoint::AtRegion(const Region *r, vector<UPoint> &result) const {

  result.clear();
  if(!IsDefined() || !r->IsDefined() ) {
    return false;
  }
  if(r->IsEmpty()) {
    return true;
  }
  if(!r->BoundingBox().Intersects(BoundingBoxSpatial())) { // total MBR-check
    return true;
  }
  if(AlmostEqual(p0,p1)){
    // this unit is static
    if( r->Contains(p0) ){   // this unit is completely within the region
      result.push_back(*this);
    }
    return true;
  }

  // create a halfsegment hs using Trajectory() and compute intersection
  vector<UPoint> tmpresult;
  UPoint ures(false);
  Instant t_left(instanttype),  t_right(instanttype),
          t_start(instanttype), t_end(instanttype);

  // handle linear intersections
  Line segs1(2);
  Line traj(2);                     // buffer for trajectory

  UTrajectory(traj);                   // get trajectory (may be empty)
  r->Intersection(traj, segs1);         // compute linear intersections

  Line segs(segs1.Size());
  segs1.Simplify(segs, FACTOR);


  if(!segs.IsDefined()){
    cerr << __PRETTY_FUNCTION__
         << " WARNING: r->Intersection(traj, segs) is UNDEF for traj="
         << traj << "." <<endl;
  } else {
    HalfSegment hs;                      // buffer for the halfsegment
    for(int i=0; i<segs.Size(); i++){    // for each halfsegment hs in segs
    //    compute instants t_left, t_right from segment's start and endpoint
      segs.Get(i,hs);
      if(hs.IsLeftDomPoint()){ // only use left dominating points
        UPoint tmpUnit(*this);         // UPoint::At() will use lc, rc strictly
        tmpUnit.timeInterval.lc=true;  // Make a left- and rightclosed copy
        tmpUnit.timeInterval.rc=true;  // by bypass this problem...
        tmpUnit.At( hs.GetLeftPoint(), ures );

        if(!ures.IsDefined()){
          cerr << __PRETTY_FUNCTION__
            <<" WARNING: (1) undef linear intersection unit for hs=" <<hs<<endl;
          continue;
        }
        t_left  = ures.timeInterval.start;

        tmpUnit.At( hs.GetRightPoint(), ures );
        if(!ures.IsDefined()){
          cerr << __PRETTY_FUNCTION__
            <<" WARNING: (2) undef linear intersection unit for hs=" <<hs<<endl;
        } else {
          t_right = ures.timeInterval.start;
          //  create a UPoint
          Point p_start (true), p_end(true);
          if(t_left<t_right) {
            t_start = t_left;
            t_end   = t_right;
            p_start = hs.GetLeftPoint();
            p_end   = hs.GetRightPoint();
          } else {
            t_start = t_right;
            t_end   = t_left;
            p_start = hs.GetRightPoint();
            p_end   = hs.GetLeftPoint();
          }
          Interval<Instant> interval(t_start,t_end,true,true);
          ures = UPoint(interval,p_start,p_end);
          tmpresult.push_back(ures); //    add the UPoint to tmpresult
        } // else
      } // is LeftDomPoint
    }// for each halfsegment hs in segs
  }

  // handle point intersections
  Points points(0);
  r->TouchPoints(traj, points); // compute point intersections

  if(!points.IsDefined()){
    cerr << __PRETTY_FUNCTION__
         << " WARNING: r->TouchPoints(traj, points) is UNDEF for traj="
        << traj << "." << endl;
  } else {
    Point p(true,0.0,0.0);                       // buffer for the point
    int nosegres = tmpresult.size();     // no of linear results
    for(int i=0; i<points.Size(); i++){  // for each point p in points
      points.Get(i,p);
      bool found = false;
      int j = 0;
      while(j<nosegres && !found){
        found = AlmostEqual(tmpresult[j].p0,p)||AlmostEqual(tmpresult[j].p1,p);
        j++;
      }
      if(!found) { // p is not already contained by segs --> keep it!
        At( p, ures ); // compute UPoint from position
        if(!ures.IsDefined()){
          cerr << __PRETTY_FUNCTION__
              << " WARNING: undef point intersection unit for p=" << p << endl;
        } else {
          tmpresult.push_back(ures); //    add the UPoint to tmpresult
        }
      }
    } // for each point p in points
  }


  // adapt closedness within tmpresult and copy tmpresult to result
  return
    ConsolidateUnitVector<UPoint,Point>(this->timeInterval,tmpresult,result);
}

/*
Implementation of methods for class ~CellGrid2D~

*/

CellGrid2D::CellGrid2D() {}

CellGrid2D::CellGrid2D(const int dummy):
   Attribute(false),
   x0(0),y0(0),wx(0),wy(0),no_cells_x(0) {}

CellGrid2D::CellGrid2D(const double &x0_, const double &y0_,
               const double &wx_, const double &wy_, const int32_t &nox_)
      : x0(x0_), y0(y0_), wx(wx_), wy(wy_), no_cells_x(nox_) {
      // Defines a structure for a three-side bounded grid, open to one
      // Y-direction. The grid is specified by an origin (x0_,x0_), a cell-width
      // wx_, a cell-height wy_ and a number of cells in X-direction nox_.
      // If wy_ is positive, the grid is open to +X, if it is negative, to -X.
      // If wx_ is positive, the grid extends to the "left", otherwise to the
      // "right" side of the origin.
      // The cells are numbered sequentially starting with 1 for the cell
      // nearest to the origin.
      // Cell Bounderies. Point located on the bounderies between cells are
      // contained by the cell with the lower cell number. Thus, the origin
      // itself is never located on the grid!
        SetDefined( (no_cells_x > 0)
                     && !AlmostEqual(wx,0.0)
                     && !AlmostEqual(wy,0.0));
    }

CellGrid2D::CellGrid2D(const CellGrid2D& other):
   Attribute(other.IsDefined()),
   x0(other.x0), y0(other.y0), wx(other.wx),wy(other.wy),
   no_cells_x(other.no_cells_x) {}

CellGrid2D& CellGrid2D::operator=(const CellGrid2D& other){
  Attribute::operator=(other);
  x0 = other.x0;
  y0 = other.y0;
  wx = other.wx;
  wy = other.wy;
  no_cells_x = other.no_cells_x;
  return *this;
}


CellGrid2D::~CellGrid2D(){}

bool CellGrid2D::set(const double x0, const double y0,
                     const double wx, const double wy,
                     const int no_cells_x){

   this->x0 = x0;
   this->y0 = y0;
   this->wx = wx;
   this->wy = wy;
   this->no_cells_x = no_cells_x;
   SetDefined( (no_cells_x > 0)
              && !AlmostEqual(wx,0.0)
              && !AlmostEqual(wy,0.0));
   return IsDefined();
}

double CellGrid2D::getX0() const{
  return x0;
}

double CellGrid2D::getY0() const{
  return y0;
}
double CellGrid2D::getXw() const{
  return wx;
}
double CellGrid2D::getYw() const{
  return wy;
}
int CellGrid2D::getNx() const{
  return no_cells_x;
}


double CellGrid2D::getMaxX() const {
      // returns the maximum X-coordinate lying on the grid
      return wx < 0.0 ? x0 : x0 + wx * no_cells_x;
    }

double CellGrid2D::getMaxY() const {
      // returns the maximum Y-coordinate lying on the grid
      return wy < 0.0 ? y0 : numeric_limits<double>::max();
    }

double CellGrid2D::getMinX() const {
      // returns the minimum X-coordinate lying on the grid
      return wx > 0.0 ? x0 : x0 + wx * no_cells_x;
    }

double CellGrid2D::getMinY() const {
      // returns the minimum Y-coordinate lying on the grid
      return wy > 0.0 ? y0 : numeric_limits<double>::min();
    }


bool CellGrid2D::onGrid(const double &x, const double &y) const {
      // returns true iff (x,y) is located on the grid
      if(!IsDefined()) {
        return false;
      }
      int32_t xIndex = static_cast<int32_t>(floor((x - x0) / wx));
      int32_t yIndex = static_cast<int32_t>(floor((y - y0) / wy));
      return ( (xIndex>=0) && (xIndex<=no_cells_x) && (yIndex>=0) );
    }

int32_t CellGrid2D::getCellNo(const double &x, const double &y) const {
      // returns the cell number for a given point (x,y)
      // Only positive cell numbers are valid. Negative result indicates
      // that (x,y) is not located on the grid.
      if(!IsDefined()) {
        return getInvalidCellNo();
      }
      int32_t xIndex = static_cast<int32_t>(floor((x - x0) / wx));
      int32_t yIndex = static_cast<int32_t>(floor((y - y0) / wy));
      if( (xIndex>=0) && (xIndex<=no_cells_x) && (yIndex>=0) ) {
        return xIndex + yIndex * no_cells_x + 1;
      } else {
        return getInvalidCellNo();
      }
    }

int32_t CellGrid2D::getCellNo(const Point &p) const {
      if(IsDefined() && p.IsDefined()){
        return getCellNo(p.GetX(),p.GetY());
      } else {
        return getInvalidCellNo();
      }
    }

int32_t CellGrid2D::getXIndex(const double &x) const {
      if(IsDefined()) {
        return static_cast<int32_t>(floor((x - x0) / wx));
      } else {
        return getInvalidCellNo();
      }
    }

int32_t CellGrid2D::getYIndex(const double &y) const {
      if(IsDefined()) {
        return static_cast<int32_t>(floor((y - y0) / wy));
      } else {
        return getInvalidCellNo();
      }
    }

Rectangle<2> CellGrid2D::getMBR() const {
      // returns the grid's MBR as a 2D-rectangle
      if(IsDefined()){
        double min[2], max[2];
        min[0] = getMinX(); min[1] = getMinY();
        max[0] = getMaxX(); max[1] = getMaxY();
        return Rectangle<2>( true, min, max );
      } else {
        return Rectangle<2>( false );
      }
    }

Rectangle<2> CellGrid2D::getRowMBR(const int32_t &n) const {
      // returns the grid's nth row as a 2D-rectangle
      // row numbering starts with 0
      if( IsDefined() ){
        double min_val[2], max_val[2];
        double y1 = y0 + n     * wy;
        double y2 = y0 + (n+1) * wy;
        min_val[0] = getMinX(); min_val[1] = min(y1,y2);
        max_val[0] = getMaxX(); max_val[1] = max(y1,y2);
        return Rectangle<2>( true, min_val, max_val );
      } else {
        return Rectangle<2>( false );
      }
    }

Rectangle<2> CellGrid2D::getColMBR(const int32_t &n) const {
      // returns the grid's nth column as a 2D-rectangle
      // column numbering starts with 0 and ends with no_cells_x - 1
      if( IsDefined() ){
        double min_val[2], max_val[2];
        double x1 = x0 + n * wx;
        double x2 = x0 + (n+1) * wx;
        min_val[0] = min(x1,x2); min_val[1] = getMinY();
        max_val[0] = max(x1,x2); max_val[1] = getMaxY();
        return Rectangle<2>( true, min_val, max_val );
      } else {
        return Rectangle<2>( false );
      }
    }

bool CellGrid2D::isValidCellNo(const int32_t &n) const {
      // returns true iff n is a valid grid cell number
      return IsDefined() && (n>0);
    }

int32_t CellGrid2D::getInvalidCellNo() const {
      // returns an invalid cell number
      return -666;
    }

ostream& CellGrid2D::Print( ostream &os ) const {
  if( !IsDefined() )
  {
    return os << "(CellGrid2D: undefined)";
  }
  os << "(CellGrid2D: defined, "
     << " origin: (" << x0 << "," << y0 << "), "
     << " cellwidths: " << wx << " x " << wy << " (X x Y), "
     << " cells along X-axis: " << no_cells_x
     << ")" << endl;
  return os;
}


size_t CellGrid2D::Sizeof() const{
   return sizeof(*this);
}

int CellGrid2D::Compare(const Attribute* other) const{
  const CellGrid2D* g = static_cast<const CellGrid2D*> (other);
  if(!IsDefined()){
     return g->IsDefined()?-1:0;
  }
  if(!g->IsDefined()){
    return 1;
  }
  // both are defined
  if(!AlmostEqual(x0,g->x0)){
    return x0<g->x0?-1:1;
  }
  if(!AlmostEqual(y0,g->y0)){
    return y0<g->y0?-1:1;
  }
  if(no_cells_x!=g->no_cells_x){
    return no_cells_x<g->no_cells_x?-1:1;
  }
  if(!AlmostEqual(wx,g->wx)){
    return wx<g->wx?-1:1;
  }

  if(!AlmostEqual(wy,g->wy)){
    return wy<g->wy?-1:1;
  }
  return 0;
}


bool CellGrid2D::Adjacent(const Attribute* other) const{
  return false;
}

Attribute* CellGrid2D::Clone() const{
   return new CellGrid2D(*this);
}

size_t CellGrid2D::HashValue() const{
   return static_cast<size_t>(x0+y0+no_cells_x*wx);
}

void CellGrid2D::CopyFrom(const Attribute* other) {
    operator=(*(static_cast<const CellGrid2D*>(other)));
}


const string CellGrid2D::BasicType(){
   return "cellgrid2d";
}

ListExpr CellGrid2D::Property(){
  return gentc::GenProperty("-> DATA",
                            BasicType(),
                           "(x0 y0 xw yw n_x)",
                           "(14.0 15.0 2.0 2.0 37)");
}

bool CellGrid2D::CheckKind(ListExpr type, ListExpr& errorInfo){
 return nl->IsEqual(type,BasicType());
}

ListExpr CellGrid2D::ToListExpr(const ListExpr typeInfo)const{
   if(!IsDefined()){
     return nl->SymbolAtom(Symbol::UNDEFINED());
   } else {
     return nl->FiveElemList( nl->RealAtom(x0),
                              nl->RealAtom(y0),
                              nl->RealAtom(wx),
                              nl->RealAtom(wy),
                              nl->IntAtom(no_cells_x));
   }
}

bool CellGrid2D::ReadFrom(const ListExpr value,const ListExpr typeInfo){
   if(listutils::isSymbolUndefined(value)){
      SetDefined(false);
      return  true;
   }
   if(!nl->HasLength(value,5)){
      return false;
   }
   ListExpr l1 = nl->First(value);
   ListExpr l2 = nl->Second(value);
   ListExpr l3 = nl->Third(value);
   ListExpr l4 = nl->Fourth(value);
   ListExpr l5 = nl->Fifth(value);
   if(!listutils::isNumeric(l1) ||
      !listutils::isNumeric(l2) ||
      !listutils::isNumeric(l3) ||
      !listutils::isNumeric(l4) ||
      (nl->AtomType(l5) != IntType)){
     return  false;
   }
   x0 = listutils::getNumValue(l1);
   y0 = listutils::getNumValue(l2);
   wx = listutils::getNumValue(l3);
   wy = listutils::getNumValue(l4);
   no_cells_x = nl->IntValue(l5);
   SetDefined( (no_cells_x > 0)
                 && !AlmostEqual(wx,0.0)
                 && !AlmostEqual(wy,0.0));

    return true;
}

ostream& operator<<(ostream& o, const CellGrid2D& u){
  return u.Print(o);
}

/*
Implementation of methods for class ~GridCellSeq~

*/

GridCellSeq::GridCellSeq() :
      my_enterTime(instanttype),
      my_leaveTime(instanttype),
      my_cellNo(0),
      defined(false){}

GridCellSeq::GridCellSeq(const DateTime &enter,
                         const DateTime &leave, const int32_t &cellNo)
      : my_enterTime(enter), my_leaveTime(leave), my_cellNo(cellNo)
    {
      defined =    my_enterTime.IsDefined()
                && my_leaveTime.IsDefined()
                && (my_enterTime <= my_leaveTime);
    }

GridCellSeq::GridCellSeq(const GridCellSeq &other) :
      my_enterTime(other.my_enterTime),
      my_leaveTime(other.my_leaveTime),
      my_cellNo(other.my_cellNo),
      defined(other.defined){
    }

GridCellSeq& GridCellSeq::operator=(const GridCellSeq &other){
      my_enterTime = other.my_enterTime;
      my_leaveTime = other.my_leaveTime;
      my_cellNo = other.my_cellNo;
      defined = other.defined;
      return *this;
    }

GridCellSeq::~GridCellSeq(){}

DateTime GridCellSeq::getEnterTime() const {return my_enterTime;}
DateTime GridCellSeq::getLeaveTime() const {return my_leaveTime;}
int32_t GridCellSeq::getCellNo() const {return my_cellNo;}

void GridCellSeq::setCellNo(const int32_t &n) { my_cellNo = n; }
void GridCellSeq::setEnterTime(const DateTime &t) {
  my_enterTime = t;
  defined =    my_enterTime.IsDefined()
            && my_leaveTime.IsDefined()
            && (my_enterTime <= my_leaveTime);
}

void GridCellSeq::setLeaveTime(const DateTime &t) {
  my_leaveTime = t;
  defined =    my_enterTime.IsDefined()
            && my_leaveTime.IsDefined()
            && (my_enterTime <= my_leaveTime);
}

void GridCellSeq::setUndefined(){ defined = false; }

bool GridCellSeq::IsDefined() const{ return defined; }

void GridCellSeq::set(const int32_t &c, const DateTime &s, const DateTime &e){
  my_cellNo = c;
  my_enterTime = s;
  my_leaveTime = e;
  defined =    my_enterTime.IsDefined()
            && my_leaveTime.IsDefined()
            && (my_enterTime <= my_leaveTime);


}

ostream& GridCellSeq::Print( ostream &os ) const{
  if( !IsDefined() )
  {
    return os << "(GridCellSeq: undefined)";
  }
  os << "(GridCellSeq: defined, Cell:" << my_cellNo << ", Tenter:"
     << my_enterTime << ",  Tleave:" << my_leaveTime << ")" << endl;
  return os;
}

ostream& operator<<(ostream& o, const GridCellSeq& u){
  return u.Print(o);
}

bool myCompare (DateTime i, DateTime j) { return (i<j); }

void UPoint::GetGridCellSequence(CellGrid2D &g, vector<GridCellSeq> &res){
//   cout << __PRETTY_FUNCTION__ << " called..." << endl;
//   cout << "\t*this = " << *this << endl;
  res.clear();
  res.resize(0);
  if(!IsDefined() && !g.IsDefined()){
//     cout << "\t*this is undefined." << endl;
//     cout << __PRETTY_FUNCTION__ << " finished." << endl;
    return;
  }
  double minX = min(p0.GetX(),p1.GetX());
  double minY = min(p0.GetY(),p1.GetY());
  double maxX = max(p0.GetX(),p1.GetX());
  double maxY = max(p0.GetY(),p1.GetY());
  if(    (minX > g.getMaxX()) || (minY > g.getMaxY())
      || (maxX < g.getMinX()) || (maxY < g.getMinY())){
    // p0 and p1 outside the grid and unit does not cross the grid.
    //         Complete unit out of grid: return empty result.
//     cout << "\t*this located completely off the grid. No result!" << endl;
//     cout << __PRETTY_FUNCTION__ << " finished." << endl;
    return;
  }

  if(AlmostEqual(p0,p1)){
    // special case: static unit
//     cout << "\tSpecial case: static unit." << endl;
    double x = (p0.GetX() + p1.GetX())/2;
    double y = (p0.GetY() + p1.GetY())/2;
    if(g.onGrid(x,y)){
      GridCellSeq event(  timeInterval.start,
                          timeInterval.end,
                          g.getCellNo(x,y)
                   );
      res.push_back(event);
//       cout << "\t\tAdded to Result: " << event << endl;
    }
//     cout << __PRETTY_FUNCTION__ << " finished." << endl;
    return;
  }

  UPoint unit_before(false);
  UPoint unit_inside(false);
  UPoint unit_after(false);

  // get unit restricted to period while moving inside the grid
//   cout << "\t\tMBR(g) = " << g.getMBR() << endl;
  At(g.getMBR(), unit_inside);
//   cout << "\t\tunit_inside = " << unit_inside << endl;
  if(    unit_inside.IsDefined()
      && (timeInterval.start < unit_inside.timeInterval.start) ){
    // get unit restricted to period before entering the grid
    Interval<Instant> i( timeInterval.start,
                         unit_inside.timeInterval.start,
                         timeInterval.lc,
                         !unit_inside.timeInterval.rc);
    if(i.IsValid()){
      unit_before.SetDefined(true);
      AtInterval( i, unit_before );
    }
  }
//   cout << "\t\tunit_before = " << unit_before << endl;
  if(    unit_inside.IsDefined()
      && (timeInterval.end > unit_inside.timeInterval.end) ){
    // get unit restricted to period after leaving the grid
    Interval<Instant> i( unit_inside.timeInterval.end,
                         timeInterval.end,
                         !unit_inside.timeInterval.rc,
                         timeInterval.rc);
    if(i.IsValid()){
      unit_after.SetDefined(true);
      AtInterval( i, unit_after );
    }
  }
//   cout << "\t\tunit_after = " << unit_after << endl;

  // (1) handle part of unit before entering the grid (if present)
//   cout << "\t(1) handle part of unit before entering the grid (if present)"
//        << endl;
  if(unit_before.IsDefined()){
    GridCellSeq event(  unit_before.timeInterval.start,
                        unit_before.timeInterval.end,
                        g.getInvalidCellNo()             );
    res.push_back(event);
//     cout << "\t\tAdded to Result: " << event << endl;
  }

  // (2) handle part of unit fully within the grid (if present)
  //     this part of the unit may cover several grid cells.
//   cout << "\t(2) handle part of unit fully within the grid (if present)"
//        << endl;
  if(unit_inside.IsDefined()){
    // create a vector of events where the unit crosses horizontal or vertical
    // grid lines and oder them by increasing time.
    minX = min(unit_inside.p0.GetX(),unit_inside.p1.GetX());
    minY = min(unit_inside.p0.GetY(),unit_inside.p1.GetY());
    maxX = max(unit_inside.p0.GetX(),unit_inside.p1.GetX());
    maxY = max(unit_inside.p0.GetY(),unit_inside.p1.GetY());

    int32_t startRow  = g.getYIndex(minY);
    int32_t endRow    = g.getYIndex(maxY);

    int32_t startCol  = g.getXIndex(minX);
    int32_t endCol    = g.getXIndex(maxX);

    vector<DateTime> events;

    events.push_back(unit_inside.timeInterval.start); // add start
//     cout << "\t\tAdded initial instant: " << unit_inside.timeInterval.start
//          << endl;
    events.push_back(unit_inside.timeInterval.end);   // add end;
//     cout << "\t\tAdded final instant: " << unit_inside.timeInterval.end
//          << endl;

    // create horizontal events
//     cout << "\tProcessing crossings of horizontal grid lines..." << endl;
    for(int32_t row=startRow; row<=endRow; row++){
      Rectangle<2> rowRect = g.getRowMBR(row);
//       cout << "\t\tRect for row " << row << ": " << rowRect << endl;
      if(rowRect.IsDefined()){
        UPoint rowUnit(true);
        unit_inside.At(rowRect, rowUnit);
//         cout << "\t\trowUnit after At(): " << rowUnit << endl;;
        if(rowUnit.IsDefined()){
          events.push_back(rowUnit.timeInterval.end);
//           cout << "\t\tAdded 'vertical' instant " << rowUnit.timeInterval.end
//                << endl;
        } else { // drop the unit
//           cout << endl << "\t\trow " << row << ": undefined rowUnit"
//                << " dropped."  << endl;
        }
      } else {
//         cout << endl << "\t\trow " << row << ": undefined rowRect!" << endl;
        assert( false );
      }
    }

// create vertical events
//     cout << "\tProcessing crossings of vertical grid lines..." << endl;

    for(int32_t col=startCol; col<=endCol; col++){
      Rectangle<2> colRect = g.getColMBR(col);
//       cout << "\t\tRect for col " << col << ": " << colRect << endl;
      if(colRect.IsDefined()){
        UPoint colUnit(true);
        unit_inside.At(colRect, colUnit);
//         cout << "\t\tcolUnit after At(): " << colUnit << endl;;
        if(colUnit.IsDefined()){
          events.push_back(colUnit.timeInterval.end);
//           cout << "\t\tAdded 'horizontal' instant "
//                << colUnit.timeInterval.end << endl;
        } else { // drop the unit
//           cout << endl << "\t\tcolumn " << col << ": undefined colUnit"
//                << " dropped." << endl;
        }
      } else {
//         cout << endl << "\t\tcolumn " << col << ": undefined colRect!"
//              << endl;
        assert( false );
      }
    }

    // sort all events
//     cout << "\n\tevents before sorting:" << endl;
//     for(vector<DateTime>::iterator j = events.begin(); j< events.end(); j++){
//       cout << "\t\t" << *j << endl;
//     }
//     cout << endl;
//     cout << "\tSorting instants..." << endl;
    sort(events.begin(), events.end(), myCompare);
//     cout << "\n\tevents after sorting:" << endl;
//     for(vector<DateTime>::iterator j = events.begin(); j< events.end(); j++){
//       cout << "\t\t" << *j << endl;
//     }
//     cout << endl;

    // Handle all events. If two consecutive events have almost the same time,
    // merge them into a single event. For each remaining event create a result
    // entry. Use midpoints between two consecutive events to compute according
    // cell number
//     cout << "\tProcessing instants..." << endl;
    vector<DateTime>::iterator curr;
    vector<DateTime>::iterator last = events.begin();
    for(curr = events.begin(); curr< events.end(); curr++){
      if(*curr == *last){
        // for 1st entry: just proceed
//         cout << "\t\tFound equal instants: *curr=" << *curr << " *last="
//              << *last << endl;
        continue;
      }
      // create output
      DateTime midtime(instanttype);
      midtime = *last + ((*curr - *last)/2);
      Point midpoint(true);
      TemporalFunction(midtime, midpoint, true);
      int32_t cellno = g.getCellNo(midpoint);
      if(g.isValidCellNo(cellno)){
        GridCellSeq event( *last, *curr, cellno);
        res.push_back(event);
//         cout << "\t\tAdded to Result: " << event << endl;
      }
      last = curr;
    }
  }

  // (3) handle part of unit after leaving the grid (if present)
//   cout << "\t(3) handle part of unit after leaving the grid (if present)"
//        << endl;
  if(unit_after.IsDefined()){
    GridCellSeq event(  unit_after.timeInterval.start,
                        unit_after.timeInterval.end,
                        g.getInvalidCellNo());
    res.push_back(event);
//     cout << "\t\tAdded to Result: " << event << endl;
  }

//   cout << "\n\tFinal result: res = :" << endl;
//   for(vector<GridCellSeq>::iterator i = res.begin(); i < res.end(); i++){
//     cout << "\t\t" << *i << endl;
//   }
//   cout << endl;

//   cout << __PRETTY_FUNCTION__ << " finished." << endl;
  return;
}

/*
3.2 Class ~MInt~

*/
void MInt::ReadFrom(const MBool& arg){
  // remove all units
  Clear();
  if(!arg.IsDefined()){
    SetDefined(false);
    return;
  }
  SetDefined(true);
  int size = arg.GetNoComponents();
  if(size>0){
     Resize(size);
  }
  UBool ubool;
  StartBulkLoad();
  CcInt currentValue;
  for(int i=0;i<size;i++){
    arg.Get(i,ubool);
    bool v;
    v = ubool.constValue.GetBoolval();
    currentValue.Set(true,v?1:0);
    UInt unit(ubool.timeInterval,currentValue);
    Add(unit);
  }
  EndBulkLoad(false);
}
void MInt::WriteTo(MBool& arg){
  // remove all units
  arg.Clear();
  if(!IsDefined()){
    arg.SetDefined(false);
    return;
  }
  arg.SetDefined(true);
  int size = GetNoComponents();
  if(size>0){
     arg.Resize(size);
  }
  UInt uint;
  //arg.StartBulkLoad();
  CcBool currentValue;
  for(int i=0;i<size;i++){
    Get(i,uint);
    int v;
    v = uint.constValue.GetIntval();
    currentValue.Set(true,(v==0)?false:true);
    UBool unit(uint.timeInterval,currentValue);
    arg.Add(unit);
  }
  //EndBulkLoad(false);
}

void MInt::WriteTo(MReal& arg){
  // remove all units
  arg.Clear();
  if(!IsDefined()){
    arg.SetDefined(false);
    return;
  }
  arg.SetDefined(true);
  int size = GetNoComponents();
  if(size>0){
     arg.Resize(size);
  }
  UInt uint;
  //arg.StartBulkLoad();
  for(int i=0;i<size;i++){
    Get(i,uint);
    double v = (double)uint.constValue.GetIntval();
    UReal ureal(uint.timeInterval, v, v);
    arg.Add(ureal);
  }
  //EndBulkLoad(false);
}

bool compareuint(const UInt& u1,const UInt& u2)
{
  if( !u1.IsDefined() && u1.IsDefined() )
    return true;
  if( !u1.IsDefined() || !u1.IsDefined() )
    return false;
  if(u1.timeInterval.start < u2.timeInterval.start)
    return true;
  if(u1.timeInterval.start == u2.timeInterval.start)
    if(u1.timeInterval.end <= u2.timeInterval.end)
      return true;
  return false;
}

void MInt::SortbyUnitTime()
{
  units.Sort(UnitCompare<UInt>);
}

void MInt::Hat(MInt& mint)
{
   mint.Clear();
   if( !IsDefined() ){
     mint.SetDefined( false );
     return;
   }
   mint.SetDefined( true );
   stack<UInt> uintstack;
   UInt upi;
   UInt last,curuint;
   CcInt cur,top;
   last.SetDefined(true);
   curuint.SetDefined(false);

   float lastarea = 0.0;
   Get(0,upi);
   string starttime = upi.timeInterval.start.ToString();
   Get(GetNoComponents() - 1,upi);
   string endtime = upi.timeInterval.end.ToString();
   int nocomponents;
   int i;
   bool defstart;
   if(starttime == "begin of time" && endtime == "end of time"){
    i = 1;
    nocomponents = GetNoComponents() - 1;
    assert(GetNoComponents() >= 3);
    defstart = true;
   }else{
    i = 0;
    nocomponents = GetNoComponents();
    assert(GetNoComponents() >= 1);
    defstart = false;
   }
   for(;i < nocomponents;i++){
      Get(i,upi);
      if(!uintstack.empty()){
        cur.Set(upi.constValue.GetValue());
        top.Set(uintstack.top().constValue.GetValue());
        if(cur.GetIntval() >= top.GetIntval()){
            uintstack.push(upi);
        }
        else{
          Instant end = upi.timeInterval.start;
          Instant start = upi.timeInterval.start;
          int lastvalue = 0;
          while(!uintstack.empty() && cur.GetIntval() < top.GetIntval()){
            UInt topelem = uintstack.top();
            lastvalue = topelem.constValue.GetValue();

            start = topelem.timeInterval.start;

            uintstack.pop();
            if(!uintstack.empty())
              top.Set(uintstack.top().constValue.GetValue());
            if(lastarea == 0.0){
              lastarea = (end-topelem.timeInterval.start).ToDouble()*lastvalue;
              curuint.timeInterval.start = topelem.timeInterval.start;
              curuint.timeInterval.end = end;
              curuint.timeInterval.lc = true;
              curuint.timeInterval.rc = false;
              curuint.constValue.Set(lastvalue);
              curuint.SetDefined(true);
            }else{
              double curarea =
                      lastvalue*(end-topelem.timeInterval.start).ToDouble();

              if(curarea > lastarea){
                lastarea = curarea;
                curuint.timeInterval.start = topelem.timeInterval.start;
                curuint.timeInterval.end = end;
                curuint.timeInterval.lc = true;
                curuint.timeInterval.rc = false;
                curuint.constValue.Set(lastvalue);
                curuint.SetDefined(true);
              }
            }
          }
          UInt* tempuint = new UInt(upi);
//          (const_cast<UInt*>(upi))->timeInterval.start = start;
          tempuint->timeInterval.start = start;
          uintstack.push(*tempuint);
          delete tempuint;

//          uintstack.push(upi);
        }
      }else
        uintstack.push(upi);
    }
//    curuint.Print(cout);
//    cout<<endl;


    while(!uintstack.empty())
      uintstack.pop();//clear stack

    if(curuint.IsDefined() == true){//find hat
       UInt begin,end;
       begin.SetDefined(true);
       if(defstart){//define "begin of time"
        Get(0,upi);
        begin = upi;
        mint.Add(begin);
        Get(1,upi);
       }else
        Get(0,upi);
       begin.timeInterval.start = upi.timeInterval.start;
       begin.timeInterval.lc = true;
       begin.timeInterval.end = curuint.timeInterval.start;
       begin.timeInterval.rc = false;
       int value = upi.constValue.GetValue();
       int i;
       for(i = 1;i < nocomponents;i++){
        Get(i,upi);
        if(upi.timeInterval.start >= curuint.timeInterval.start)
          break;
        if(upi.constValue.GetValue() < value)
          value = upi.constValue.GetValue();//the minimum value before hat
       }
       begin.constValue.Set(value);
//       assert(begin.timeInterval.start != begin.timeInterval.end);
       if(begin.timeInterval.IsValid())
        mint.Add(begin);

       if(curuint.timeInterval.IsValid())//start != curuint.timeInterval.end)
        mint.Add(curuint);
       end.SetDefined(true);

       if(defstart){//define "end of time"
          Get(GetNoComponents() - 2,upi);
          end.timeInterval.start = curuint.timeInterval.end;
          end.timeInterval.lc = true;
          end.timeInterval.end = upi.timeInterval.end;
          end.timeInterval.rc = false;
          value = curuint.constValue.GetValue();
          for(;i < nocomponents;i++){
            Get(i,upi);
            if(upi.timeInterval.start < curuint.timeInterval.end)
              continue;
            if(upi.constValue.GetValue() < value)
            value = upi.constValue.GetValue();
          }
          end.constValue.Set(value);
//          assert(end.timeInterval.start != end.timeInterval.end);
          if(end.timeInterval.IsValid())
            mint.Add(end);
          Get(GetNoComponents() - 1,upi);
          end = upi;
          mint.Add(end);
       }else{
          Get(GetNoComponents() - 1,upi);
          end.timeInterval.start = curuint.timeInterval.end;
          end.timeInterval.lc = true;
          end.timeInterval.end = upi.timeInterval.end;
          end.timeInterval.rc = false;
          value = curuint.constValue.GetValue();
          for(;i < GetNoComponents();i++){
            Get(i,upi);
            if(upi.timeInterval.start < curuint.timeInterval.end)
              continue;
            if(upi.constValue.GetValue() < value)
            value = upi.constValue.GetValue();
          }
          end.constValue.Set(value); //the same as hat threshold
//          assert(end.timeInterval.start != end.timeInterval.end);
          if(end.timeInterval.IsValid())
            mint.Add(end);
       }
    }
    else{ //hat does not exist,return the first and last
       UInt begin,end;
       UInt upi1;
       UInt upi2;
       begin.SetDefined(true);
       end.SetDefined(true);
       if(defstart){
        Get(0,upi1);
        begin = upi1;
        mint.Add(begin);
        Get(1,upi1);
       }else
        Get(0,upi1);
       int firstvalue = upi.constValue.GetValue();
       if(defstart)
        Get(GetNoComponents() - 2,upi2);
       else
        Get(GetNoComponents() - 1,upi2);
        int lastvalue = upi2.constValue.GetValue();
       if(firstvalue != lastvalue){
          begin = upi1;
          mint.Add(begin);
          end = upi2;
          mint.Add(end);
          if(defstart){
            Get(GetNoComponents() - 1,upi2);
            end = upi2;
            mint.Add(end);
          }
       }else{
         if(defstart){
            begin.constValue.Set(firstvalue);
            begin.timeInterval.start = upi1.timeInterval.start;
            begin.timeInterval.end = upi2.timeInterval.end;
            begin.timeInterval.lc = true;
            begin.timeInterval.rc = false;
            mint.Add(begin);
            Get(GetNoComponents() - 1,upi2);
            end = upi2;
            mint.Add(end);
          }else{
            begin = upi1;
            begin.timeInterval.end = upi2.timeInterval.end;
            mint.Add(begin);
          }
        }
    }

}

void MInt::Restrict(MInt& result,
                    const bool useValue /*= false */,
                    const int value /*= 0*/ ) const{

  result.Clear();
  if(!IsDefined()){
     result.SetDefined(false);
     return;
  }
  result.SetDefined(true);
  int size = GetNoComponents();
  if(size==0){
    return;
  }
  result.Resize(size);
  int last = size-1;
  for(int i=0;i<size;i++){
     UInt unit;
     Get(i,unit);
     if(i==0 || i==last){
       if(unit.timeInterval.start.IsMinimum() ||
          unit.timeInterval.end.IsMaximum()){
          if(useValue && unit.constValue.GetIntval()!=value){
             result.Add(unit);
          }
       } else {
         result.Add(unit);
       }
     } else {
       result.Add(unit);
     }
  }

}

/*
~PlusExtend~

This function adds the moving integer values. In contrast to the
usual '+' function, the result will only be undefined, if both
arguments are undefined. If only a single argeument is undefined,
the value of the second parameter build the resulting unit.

*/
void  MInt::PlusExtend(const MInt* arg2, MInt& result) const
{
  result.Clear();
  if(!this->IsDefined() && !arg2->IsDefined()){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);

  UInt uInt(true);

  RefinementStream<MInt, MInt, UInt, UInt> rp(this, arg2);
  result.StartBulkLoad();
  Interval<Instant> iv;
  int u1Pos;
  int u2Pos;
  UInt u1;
  UInt u2;


  while(rp.hasNext()){
    rp.getNext(iv, u1Pos, u2Pos);
    if(u1Pos!=-1 || u2Pos!=-1){
      uInt.timeInterval = iv;
      if(u1Pos == -1 ){
         arg2->Get(u2Pos,u2);
         uInt.constValue.Set(true, u2.constValue.GetIntval());
      } else if(u2Pos == -1){
        this->Get(u1Pos,u1);
        uInt.constValue.Set(true, u1.constValue.GetIntval());
      } else {
        this->Get(u1Pos, u1);
        arg2->Get(u2Pos, u2);
        uInt.constValue.Set(true,
           u1.constValue.GetIntval() + u2.constValue.GetIntval());
      }
      result.MergeAdd(uInt);
    }

  }
  result.EndBulkLoad(false);

}

void MInt::MergeAddFillUp(const UInt& unit, const int fillValue){
  assert(IsDefined());
  assert(unit.IsDefined());
  if(!this->IsDefined() || !unit.IsDefined()){
    Clear();
    SetDefined(false);
    return;
  }
  int size = units.Size();
  if(size==0){
    units.Append(unit);
  } else {
    UInt lastUnitp;
    units.Get(size-1, &lastUnitp);
    UInt lastUnit(lastUnitp);
    int lastValue = lastUnit.constValue.GetIntval();
    int newValue  = unit.constValue.GetIntval();
    if(lastUnit.timeInterval.end == unit.timeInterval.start){
       if(lastValue == newValue){
          lastUnit.timeInterval.end = unit.timeInterval.end;
          lastUnit.timeInterval.rc = unit.timeInterval.rc;
          units.Put(size-1,lastUnit);
       } else {
          // don't allow overlapping intervals
          assert(!(lastUnit.timeInterval.rc && unit.timeInterval.lc));
          // no gap in time
          if(lastUnit.timeInterval.rc || unit.timeInterval.lc){
             units.Append(unit);
          } else { // gap in time
             if(lastValue==fillValue){
                lastUnit.timeInterval.rc = true;
                units.Put(size-1, lastUnit);
                units.Append(&unit);
             } else if(newValue==fillValue){
                UInt copy(unit);
                copy.timeInterval.lc = true;
                units.Append(copy);
             } else {
                Interval<Instant> iv(unit.timeInterval.start,
                                     unit.timeInterval.start,
                                     true, true);
                CcInt aValue(true,fillValue);
                UInt aUnit(iv,aValue);
                units.Append(aUnit);
                units.Append(unit);
             }
          }
       }

    } else { // a "real" gap in time
      assert(lastUnitp.timeInterval.end < unit.timeInterval.start);
      if(lastValue==fillValue){
        if(fillValue==newValue){ // merge three units
          lastUnit.timeInterval.end = unit.timeInterval.end;
          lastUnit.timeInterval.rc =  unit.timeInterval.rc;
          units.Put(size-1,lastUnit);
        } else {
          lastUnit.timeInterval.end = unit.timeInterval.start;
          lastUnit.timeInterval.rc = !unit.timeInterval.lc;
          units.Put(size-1, lastUnit);
          units.Append(unit);
        }
      }  else { // lastUnit and fillUnit cannot be merged
        if(newValue==fillValue){ // fillUnit and newUnit can be merged
           UInt copy(unit);
           copy.timeInterval.start = lastUnit.timeInterval.end;
           copy.timeInterval.lc = !lastUnit.timeInterval.rc;
           units.Append(copy);
        } else { // nothing can be merged
           Interval<Instant> iv(lastUnit.timeInterval.end,
                                unit.timeInterval.start,
                                !lastUnit.timeInterval.rc,
                                !unit.timeInterval.lc);
           CcInt v(true,fillValue);
           UInt fill(iv,v);
           units.Append(fill);
           units.Append(unit);
        }
      }
    }
  }
}

void MInt::fillUp(int value, MInt& result) const{
   result.Clear();
   if(!IsDefined()){
     result.Clear();
     result.SetDefined(false);
     return;
   }
   int size = units.Size();
   UInt unit;
   for(int i=0;i<size;i++){
     units.Get(i,&unit);
     result.MergeAddFillUp(unit, value);
   }
}


int MInt::maximum() const{
   int max = numeric_limits<int>::min();
   if(!IsDefined()){
      return max;
   }
   UInt unit;
   int v;
   for(int i=0;i<units.Size();i++){
      units.Get(i,&unit);
      v = unit.constValue.GetIntval();
      if(v>max){
        max = v;
      }
   }
   return max;
}

int MInt::minimum() const{
  int min = numeric_limits<int>::max();
   if(!IsDefined()){
      return min;
   }
   UInt unit;
   int v;
   for(int i=0;i<units.Size();i++){
      units.Get(i,&unit);
      v = unit.constValue.GetIntval();
      if(v<min){
        min = v;
      }
   }
   return min;
}

int MInt::Min(bool &correct) const
{
  correct = IsDefined();
  return minimum();
}

int MInt::Max(bool &correct) const
{
  correct = IsDefined();
  return  maximum();
}

/*
3.1 Class ~MReal~


3.1 ~Integrate~

3.1.1 Helper structure

*/

struct ISC{
    double value;
    unsigned int level;
};

/*
3.1.2 ~Integrate~ Implementation


The integrate function sums all integrate value for the single units.
To avoid adding big and small integer values, we compute the sum
balanced similar to the aggregateB operator of teh ExtRelationAlgebra.

*/

double MReal::Integrate(){
   if(!IsDefined()){
      return 0;
   }
   int size = GetNoComponents();
   UReal unit;
   stack<ISC> theStack;

   for(int i=0;i < size;i++){
      Get(i,unit);
      ISC isc;
      isc.value = unit.Integrate();
      //if(isnan(isc.value)) cout << " value = " << isc.value << endl;
      isc.level = 0;
      while(!theStack.empty() && (theStack.top().level == isc.level)){
          isc.value = isc.value + theStack.top().value;
          isc.level = isc.level + 1;
          theStack.pop();
      }
      theStack.push(isc);
   }
   // summarize the stack content
   double sum = 0.0;
   while(!theStack.empty()){
      sum += theStack.top().value;
      theStack.pop();
   }
   return sum;
}

double MReal::Max(bool& correct) const{
   if(!IsDefined()){
      correct=false;
      return 0.0;
   }

   int size = GetNoComponents();

   if(size<=0){
      correct=false;
      return 0.0;
   }

   correct = true;
   UReal unit;
   Get(0,unit);
   bool dummy;
   double max = unit.Max(dummy);
   for(int i=1;i<size;i++){
       Get(i,unit);
       double umax = unit.Max(dummy);
       if(umax>max){
          max = umax;
       }
   }
   return max;
}


double MReal::Min(bool& correct) const{
   if(!IsDefined()){
      correct=false;
      return 0.0;
   }

   int size = GetNoComponents();

   if(size<=0){
      correct=false;
      return 0.0;
   }

   correct = true;
   UReal unit;
   Get(0,unit);
   bool dummy;
   double min = unit.Min(dummy);
   for(int i=1;i<size;i++){
       Get(i,unit);
       double umin = unit.Min(dummy);
       if(umin<min){
          min = umin;
       }
   }
   return min;
}

// restrict to periods with minimum value
void MReal::AtMin( MReal& result ) const
{
  result.Clear();
  if(!IsDefined()){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  double globalMin = numeric_limits<double>::infinity();
  double localMin  = 0.0;
  int noLocalMin = 0;
  bool correct = true;
  UReal actual_ur(false);
  UReal last_ur(false);
  UReal last_candidate(true);
  bool firstCall = true;
  result.StartBulkLoad();
  for(int i=0; i<GetNoComponents(); i++)
  {
//     cerr << "MReal::AtMin(): Processing unit "
//          << i << "..." << endl;
    last_ur = actual_ur;
    Get( i, actual_ur );
    localMin = actual_ur.Min(correct);
    if(!correct)
    {
      cerr << "MReal::AtMin(): Cannot compute minimum value for unit "
           << i << "." << endl;
      continue;
    }
    if( localMin < globalMin )
    { // found new global min, invalidate actual result
      globalMin = localMin;
      result.Clear();
      result.StartBulkLoad(); // we have to repeat this alter Clear()
      firstCall = true;
      last_ur.SetDefined(false);
//       cerr << "MReal::AtMin(): New globalMin=" << globalMin << endl;
    }
    if( localMin <= globalMin )
    { // this ureal contains global minima
      vector<UReal> localMinimaVec;
      noLocalMin = actual_ur.AtMin( localMinimaVec );
//       cerr << "MReal::AtMin(): Unit " << i << " has "
//            << noLocalMin << " minima" << endl;
      for(int j=0; j< noLocalMin; j++)
      {
        UReal candidate = localMinimaVec[j];
        // test, whether candidate overlaps last_inserted one
        if( j==0 &&               // check only unit's first local min!
            !firstCall &&         // don't check if there is no last_candidate
            last_candidate.Intersects(candidate) &&
            ( !last_ur.Intersects(last_candidate) ||
              !actual_ur.Intersects(candidate)
            )
          )
        {
//           cerr << "MReal::AtMin(): unit overlaps last one." << endl;
          if( last_candidate.timeInterval.start
              == last_candidate.timeInterval.end )
          { // case 1: drop last_candidate (which is an instant-unit)
//             cerr << "MReal::AtMin(): drop last unit." << endl;
            last_candidate = candidate;
            continue;
          }
          else if( candidate.timeInterval.start
                   == candidate.timeInterval.end )
          { // case 2: drop candidate
//             cerr << "MReal::AtMin(): drop actual unit." << endl;
            continue;
          }
          else
            cerr << "MReal::AtMin(): This should not happen!" << endl;
        }
        else
        { // All is fine. Just insert last_candidate.
//        cerr << "MReal::AtMin(): unit does not overlap with last." << endl;
          if(firstCall)
          {
//          cerr << "MReal::AtMin(): Skipping insertion of last unit." << endl;
            firstCall = false;
          }
          else
          {
//             cerr << "MReal::AtMin(): Added last unit" << endl;
            result.MergeAdd(last_candidate);
          }
          last_candidate = candidate;
        }
      }
    }
//     else
//     {
//       cerr << "MReal::AtMin(): Unit " << i
//            << " has no global minimum." << endl;
//     }
  }
  if(!firstCall)
  {
    result.MergeAdd(last_candidate);
//     cerr << "MReal::AtMin(): Added final unit" << endl;
  }
//   else
//     cerr << "MReal::AtMin(): Skipping insertion of final unit." << endl;
  result.EndBulkLoad();
}

// restrict to periods with maximum value
void MReal::AtMax( MReal& result ) const
{
  result.Clear();
  if(!IsDefined()){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  double globalMax = -numeric_limits<double>::infinity();
  double localMax  = 0.0;
  int noLocalMax = 0;
  bool correct = true;
  UReal actual_ur(false);
  UReal last_ur(false);
  UReal last_candidate(true);
  bool firstCall = true;
  result.StartBulkLoad();
  for(int i=0; i<GetNoComponents(); i++)
  {
//     cerr << "MReal::AtMax(): Processing unit "
//          << i << "..." << endl;
    last_ur = actual_ur;
    Get( i, actual_ur );
    localMax = actual_ur.Max(correct);
    if(!correct)
    {
      cerr << "MReal::AtMax(): Cannot compute maximum value for unit "
           << i << "." << endl;
      continue;
    }
    if( localMax > globalMax )
    { // found new global max, invalidate actual result
      globalMax = localMax;
      result.Clear();
      result.StartBulkLoad(); // we have to repeat this alter Clear()
      firstCall = true;
      last_ur.SetDefined(false);
//       cerr << "MReal::AtMax(): New globalMax=" << globalMax << endl;
    }
    if( localMax >= globalMax )
    { // this ureal contains global maxima
      vector<UReal> localMaximaVec;
      noLocalMax = actual_ur.AtMax( localMaximaVec );
//       cerr << "MReal::AtMax(): Unit " << i << " has "
//            << noLocalMax << " maxima" << endl;
      for(int j=0; j< noLocalMax; j++)
      {
        UReal candidate = localMaximaVec[j];
        // test, whether candidate overlaps last_inserted one
        if( j==0 &&               // check only unit's first local max!
            !firstCall &&         // don't check if there is no last_candidate
            last_candidate.Intersects(candidate) &&
            ( !last_ur.Intersects(last_candidate) ||
              !actual_ur.Intersects(candidate)
            )
          )
        {
//           cerr << "MReal::AtMax(): unit overlaps last one." << endl;
          if( last_candidate.timeInterval.start
              == last_candidate.timeInterval.end )
          { // case 1: drop last_candidate (which is an instant-unit)
//             cerr << "MReal::AtMax(): drop last unit." << endl;
            last_candidate = candidate;
            continue;
          }
          else if( candidate.timeInterval.start
                   == candidate.timeInterval.end )
          { // case 2: drop candidate
//             cerr << "MReal::AtMax(): drop actual unit." << endl;
            continue;
          }
          else
            cerr << "MReal::AtMax(): This should not happen!" << endl;
        }
        else
        { // All is fine. Just insert last_candidate.
//         cerr << "MReal::AtMax(): unit does not overlap with last." << endl;
          if(firstCall)
          {
//          cerr << "MReal::AtMax(): Skipping insertion of last unit." << endl;
            firstCall = false;
          }
          else
          {
//             cerr << "MReal::AtMax(): Added last unit" << endl;
            result.MergeAdd(last_candidate);
          }
          last_candidate = candidate;
        }
      }
    }
//     else
//     {
//       cerr << "MReal::AtMax(): Unit " << i
//            << " has no global maximum." << endl;
//     }
  }
  if(!firstCall)
  {
    result.MergeAdd(last_candidate);
//     cerr << "MReal::AtMax(): Added final unit" << endl;
  }
//   else
//     cerr << "MReal::AtMax(): Skipping insertion of final unit." << endl;
  result.EndBulkLoad();
}

// restrict to periods with certain value
void MReal::AtValue( const CcReal& ccvalue, MReal& result ) const
{
  result.Clear();
  if(!IsDefined() || !ccvalue.IsDefined()){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );

  int noLocalResults = 0;
  UReal actual_ur(false);
  UReal last_ur(false);
  UReal last_candidate(false);
  bool firstCall = true;
  result.StartBulkLoad();
  for(int i=0; i<GetNoComponents(); i++)
  {
//     cerr << "MReal::AtValue(): Processing unit "
//          << i << "..." << endl;
    last_ur = actual_ur;
//    cout << __PRETTY_FUNCTION__ << ": last_ur ="; last_ur.Print(cout);
//    cout << endl;
    Get( i, actual_ur );
//    cout << __PRETTY_FUNCTION__ << ": actual_ur ="; actual_ur.Print(cout);
//    cout << endl;
    assert( actual_ur.IsDefined() );
    vector<UReal> localResultVec;
    noLocalResults = actual_ur.AtValue( ccvalue, localResultVec );
//    cerr << "MReal::AtValue(): Unit " << i << " has "
//         << noLocalResults << " results" << endl;
    for(int j=0; j< noLocalResults; j++)
    {
      UReal candidate = localResultVec[j];
//      cout << "\t" << __PRETTY_FUNCTION__ << ": candidate=";
//      candidate.Print(cout); cout << endl;
      // test, whether candidate overlaps last_inserted one
      if( j==0 &&               // check only unit's first local max!
          !firstCall &&         // don't check if there is no last_candidate
          last_candidate.Intersects(candidate) &&
          ( !last_ur.Intersects(last_candidate) ||
            !actual_ur.Intersects(candidate)
          )
        )
      {
//         cerr << "MReal::AtValue(): unit overlaps last one." << endl;
        if( last_candidate.timeInterval.start
            == last_candidate.timeInterval.end )
        { // case 1: drop last_candidate (which is an instant-unit)
//           cerr << "MReal::AtValue(): drop last unit." << endl;
          last_candidate = candidate;
          continue;
        }
        else if( candidate.timeInterval.start
                  == candidate.timeInterval.end )
        { // case 2: drop candidate
//           cerr << "MReal::AtValue(): drop actual unit." << endl;
          continue;
        }
        else {
          cerr << __PRETTY_FUNCTION__ << ": This should not happen!" << endl;
        }
      }
      else
      { // All is fine. Just insert last_candidate.
//         cerr << "MReal::AtValue(): unit does not overlap with last." << endl;
        if(firstCall) {
//           cerr << "MReal::AtValue(): Skipping insertion of last unit."
//                << endl;
          firstCall = false;
        } else {
//           cerr << "MReal::AtValue(): Adding last unit" << endl;
          result.MergeAdd(last_candidate);
        }
        last_candidate = candidate;
      }
    }
//     else
//     {
//       cerr << "MReal::AtValue(): Unit " << i
//            << " does never take the value." << endl;
//     }
  }
  if(!firstCall){
    result.MergeAdd(last_candidate);
//     cerr << "MReal::AtValue(): Added final unit" << endl;
  }
//   else{
//     cerr << "MReal::AtValue(): Skipping insertion of final unit." << endl;
//   }
  result.EndBulkLoad();
//cout << __PRETTY_FUNCTION__ << "result = "; Print(cout); cout << endl << endl;
}

/*
This function replaces all units by linear approximations between their
start and end value.

*/
void MReal::Linearize(MReal& result) const{
  result.Clear();
  if(!IsDefined()){
     result.SetDefined(false);
     return;
  }
  result.SetDefined(true);
  int size = GetNoComponents();
  if(size<1){
     return;
  }
  result.Resize(size);
  UReal unitptr;
  UReal unit;
  result.StartBulkLoad();
  for(int i=0;i<size;i++){
      Get(i,unitptr);
      unitptr.Linearize(unit);
      result.Add(unit);
  }
  result.EndBulkLoad(false);

}

/*
This function replaces all units by linear approximations between their
start and end value and possible the extremum.

*/
void MReal::Linearize2(MReal& result) const{
  result.Clear();
  if(!IsDefined()){
     result.SetDefined(false);
     return;
  }
  result.SetDefined(true);
  int size = GetNoComponents();
  if(size<1){
     return;
  }
  result.Resize(size);
  UReal unitptr;
  UReal unit1;
  UReal unit2;
  result.StartBulkLoad();
  for(int i=0;i<size;i++){
      Get(i,unitptr);
      unitptr.Linearize(unit1,unit2);
      result.Add(unit1);
      if(unit2.IsDefined()){
         result.Add(unit2);
      }
  }
  result.EndBulkLoad(false);

}


/*
Helper function for the ~simplify~ operator

We keep a unit if it is no linear representation.

*/
bool keep(const UReal* unit){
   if(unit->a != 0) { // square included
      return true;
   }
   if(unit->r){    // square-root
      return true;
   }
   return false; // linear or constant function
}

/*
~connected~

Checks whether two ureal values build a continious function.

*/
bool connected(const UReal* ur1, const UReal* ur2){
   Instant end = ur1->timeInterval.end;
   Instant start = ur2->timeInterval.start;
   if(start!=end){
        return false;
   }
   bool rc = ur1->timeInterval.rc;
   bool lc = ur2->timeInterval.lc;
   if(!(lc ^ rc)){
      return  false;
   }
   CcReal endValue(true,0);
   CcReal startValue(true,0);
   ur1->TemporalFunction(end,endValue,true);
   ur2->TemporalFunction(start,startValue,true);
   if(!AlmostEqual(endValue.GetRealval(), startValue.GetRealval())){
       return false;
   }
   return true;
}

/*
Simplifies the connected linear approximated parts of this moving real.

*/
void MReal::Simplify(const double epsilon, MReal& result) const{
  result.Clear();
  if(!IsDefined()){
     result.SetDefined(false);
     return;
  }
  result.SetDefined(true);
  unsigned int size = GetNoComponents();
  if(epsilon < 0 || size<2){
     result.CopyFrom(this);
     return;
  }
  bool useleft[size];
  bool useright[size];
  for(unsigned int i=0;i<size;i++){
     useleft[i] = false;
     useright[i] = false;
  }

  unsigned int first = 0;
  unsigned int last = 1;
  UReal ur1;
  UReal ur2;
  while(last < size){
    Get(last-1,ur1);
    Get(last, ur2);
    if(keep(&ur1)){
         if(last-1 > first){
            Simplify(first,last-2,useleft,useright,epsilon);
         }
         Simplify(last-1, last-1, useleft, useright, epsilon);
         first = last;
         last++;
    } else if( keep(&ur2)){
         Simplify(first,last-1,useleft,useright,epsilon);
         last++;
         Simplify(last-1, last-1,useleft,useright,epsilon);
         first = last;
         last++;
    } else if(connected(&ur1,&ur2)){ // enlarge the sequence
         last++;
    } else {
          Simplify(first,last-1,useleft, useright, epsilon);
          first=last;
          last++;
    }
  }
  Simplify(first,last-1,useleft,useright,epsilon);

  // build the result
   int count = 1; // count the most right sample point
   for( unsigned int i=0;i<size;i++){
      if( useleft[i]){
         count++;
      }
   }
   result.Resize(count); // prepare enough memory

   // scan the units
   UReal unit;
   Instant start(instanttype);
   Instant end(instanttype);
   bool lc = false, rc = false;
   CcReal startValue(true,0);
   CcReal endValue(true,0);
   bool leftDefined = false;
   result.StartBulkLoad();
   for(unsigned int i=0;i<size;i++){
      Get(i,unit);
      if(useleft[i] && useright[i]){ // copy this unit
         result.Add(unit);
      } else {
         if(useleft[i]){
             if(leftDefined){ // debug
                cerr << "Overwrite left part of a ureal " << endl;
             }
             start = unit.timeInterval.start;
             lc = unit.timeInterval.lc;
             unit.TemporalFunction(start,startValue,true);
             leftDefined = true;
         }
         if(useright[i]){
             if(!leftDefined){ // debug
                 cerr << "Close ureal without left definition " << endl;
             } else{
               end = unit.timeInterval.end;
               rc = unit.timeInterval.rc;
               unit.TemporalFunction(end,endValue,true);
               UReal newUnit(Interval<Instant>(start,end,lc,rc),
                             startValue.GetRealval(),
                             endValue.GetRealval());
               result.Add(newUnit);
               leftDefined=false;
             }

         }
      }
   }
   result.EndBulkLoad(true,true);
}

// A private auxiliary function:
void MReal::Simplify(const int min, const int max,
                     bool* useleft, bool* useright,
                     const double epsilon) const{
  // the endpoints are used in each case
  useleft[min] = true;
  useright[max] = true;

  if(min==max){ // no intermediate sampling points -> nothing to simplify
     return;
  }

  UReal u1;
  UReal u2;
  // build a UReal from the endpoints
  Get(min,u1);
  Get(max,u2);
  CcReal cr1(true,0.0);
  CcReal cr2(true,0.0);

  u1.TemporalFunction(u1.timeInterval.start,cr1,true);
  u2.TemporalFunction(u2.timeInterval.end,cr2,true);

  double r1 = cr1.GetRealval();
  double r2 = cr2.GetRealval();

  // build the approximating unit
  UReal ureal(Interval<Instant>(u1.timeInterval.start,
                u2.timeInterval.end,true,true),
                r1,
                r2);

  // search for the real with the highest distance to this unit
  double maxDist = 0;
  int maxIndex=0;
  CcReal r_orig(true,0);
  CcReal r_simple(true,0);
  UReal u;
  double distance;
  for(int i=min+1;i<=max;i++){
     Get(i,u);
     ureal.TemporalFunction(u.timeInterval.start,r_simple, true);
     u.TemporalFunction(u.timeInterval.start,r_orig,true);
     distance  = abs(r_simple.GetRealval()- r_orig.GetRealval());
     if(distance>maxDist){ // new maximum found
        maxDist = distance;
        maxIndex = i;
     }
  }

  if(maxIndex==0){  // no difference found
      return;
  }
  if(maxDist<=epsilon){  // difference is in allowed range
      return;
  }

  // split at the left point of maxIndex
  Simplify(min,maxIndex-1,useleft,useright,epsilon);
  Simplify(maxIndex,max,useleft,useright,epsilon);
}



/*
3.2 Class ~MPoint~

We need to overwrite some methods from template class ~Mapping~, as we need
to maintain the object's MBR in ~bbox~.

*/

void MPoint::Clear()
{
  Mapping<UPoint, Point>::Clear(); // call super
  bbox.SetDefined(false);          // invalidate bbox
}

void MPoint::Add( const UPoint& unit )
{
//   cout << "CALLED: MPoint::Add" << endl;
  assert( unit.IsDefined() );
  assert( unit.IsValid() );
  if(!IsDefined() || !unit.IsDefined()){
    SetDefined( false );
    return;
  }
  units.Append( unit );
  if(units.Size() == 1)
  {
//     cout << "        MPoint::Add FIRST ADD" << endl;
//     cout << "\t Old bbox = "; bbox.Print(cout);
    bbox.SetDefined( true );
    bbox = unit.BoundingBox();
//     cout << "\n\t New bbox = "; bbox.Print(cout);
  } else {
//     cout << "\t Old bbox = "; bbox.Print(cout);
    bbox = bbox.Union(unit.BoundingBox());
//     cout << "\n\t New bbox = "; bbox.Print(cout);
  }
  RestoreBoundingBox(false);
}

void MPoint::Restrict( const vector< pair<int, int> >& intervals )
{
  if(!IsDefined()){
    Clear();
    bbox.SetDefined(false);
    SetDefined( false );
    return;
  }
  units.Restrict( intervals, units ); // call super
  bbox.SetDefined(false);      // invalidate bbox
  RestoreBoundingBox();        // recalculate it
}

ostream& MPoint::Print( ostream &os ) const
{
  if( !IsDefined() )
  {
    return os << "(MPoint: undefined)";
  }
  os << "(MPoint: defined, MBR = ";
  bbox.Print(os);
  os << ", contains " << GetNoComponents() << " units: ";
  for(int i=0; i<GetNoComponents(); i++)
  {
    UPoint unit;
    Get( i , unit );
    os << "\n\t";
    unit.Print(os);
  }
  os << "\n)" << endl;
  return os;
}

void MPoint::EndBulkLoad(const bool sort, const bool checkvalid)
{
  Mapping<UPoint, Point>::EndBulkLoad( sort, checkvalid ); // call super
  RestoreBoundingBox();                        // recalculate, if necessary
}

bool MPoint::operator==( const MPoint& r ) const
{
  if(!IsDefined()){
     return !r.IsDefined();
  }
  if(!r.IsDefined()){
     return false;
  }
  assert( IsOrdered() && r.IsOrdered() );
  if(IsEmpty()){
    return r.IsEmpty();
  }
  if(r.IsEmpty()){
    return false;
  }

  if(bbox != r.bbox)
    return false;
  return Mapping<UPoint, Point>::operator==(r);
}

bool MPoint::Present( const Instant& t ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( t.IsDefined() );

  if(bbox.IsDefined())
  { // do MBR-check
    double instd = t.ToDouble();
    double mint = bbox.MinD(2);
    double maxt = bbox.MaxD(2);
    if( (instd < mint && !AlmostEqual(instd,mint)) ||
        (instd > maxt && !AlmostEqual(instd,maxt))
      )
    {
//       cout << __PRETTY_FUNCTION__<< "(" << __FILE__ << __LINE__
//         << "): Bounding box check failed:" << endl;
//       cout << "\tInstant : "; t.Print(cout); cout << endl;
//       cout << "\tinstd   : " << instd << endl;
//       cout << "\tmint/maxt :" << mint << "\t/\t" << maxt << endl;
//       cout << "\tBBox = "; bbox.Print(cout); cout << endl;
      return false;
    }
  }
  int pos = Position(t);
  if( pos == -1 )         //not contained in any unit
    return false;
  return true;
}

bool MPoint::Present( const Periods& t ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( t.IsDefined() );
  assert( t.IsOrdered() );

  if(bbox.IsDefined())
  { // do MBR-check
    double MeMin = bbox.MinD(2);
    double MeMax = bbox.MaxD(2);
    Instant tmin; t.Minimum(tmin);
    Instant tmax; t.Maximum(tmax);
    double pmin = tmin.ToDouble();
    double pmax = tmax.ToDouble();
    if( (pmax < MeMin && !AlmostEqual(pmax,MeMin)) ||
        (pmin > MeMax && !AlmostEqual(pmin,MeMax))
      )
    {
//       cout << __PRETTY_FUNCTION__<< "(" << __FILE__ << __LINE__
//            << "): Bounding box check failed:" << endl;
//       cout << "\tPeriod : "; t.Print(cout); cout << endl;
//       cout << "\tpmin/pmax : " << pmin  << "\t/\t" << pmax << endl;
//       cout << "\ttmin/tmax :" << tmin << "\t/\t" << tmax << endl;
//       cout << "\tMPoint : " << MeMin << "\t---\t" << MeMax << endl;
//       cout << "\tBBox = "; bbox.Print(cout); cout << endl;
      return false;
    }
  }
  Periods defTime( 0 );
  DefTime( defTime );
  return t.Intersects( defTime );
}

void MPoint::AtInstant( const Instant& t, Intime<Point>& result ) const
{
  if( IsDefined() && t.IsDefined() )
  {
    if( !bbox.IsDefined() )
    { // result is undefined
      result.SetDefined(false);
    } else if( IsEmpty() )
    { // result is undefined
      result.SetDefined(false);
    } else
    { // compute result
      double instd = t.ToDouble();
      double mind = bbox.MinD(2);
      double maxd = bbox.MaxD(2);
      if( (mind > instd && !AlmostEqual(mind,instd)) ||
           (maxd < instd && !AlmostEqual(maxd,instd))
        )
      {
//         cout << __PRETTY_FUNCTION__<< "(" << __FILE__ << __LINE__
//           << "): Bounding box check failed:" << endl;
//         cout << "\tInstant : "; t.Print(cout); cout << endl;
//         cout << "\tinstd   : " << instd << endl;
//         cout << "\tmind/maxd :" << mind << "\t/\t" << maxd << endl;
//         cout << "\tBBox = "; bbox.Print(cout); cout << endl;
        result.SetDefined(false);
      } else
      {
        assert( IsOrdered() );
        int pos = Position( t );
        if( pos == -1 )  // not contained in any unit
          result.SetDefined( false );
        else
        {
          UPoint posUnit;
          units.Get( pos, &posUnit );
          result.SetDefined( true );
          posUnit.TemporalFunction( t, result.value );
          result.instant.CopyFrom( &t );
        }
      }
    }
  } else
  {
    result.SetDefined(false);
  }
}

void MPoint::AtPeriods( const Periods& p, MPoint& result ) const
{
  result.Clear();
  result.SetDefined(true);
  if( IsDefined() && p.IsDefined() )
  {
    if( !bbox.IsDefined())
    { // result is undefined
      result.SetDefined(false);
    } else if( IsEmpty() || p.IsEmpty())
    { // result is defined but empty
      result.SetDefined(true);
    } else if( IsMaximumPeriods(p) )
    { // p is [begin of time, end of time]. Copy the input into result.
      result.CopyFrom(this);
    }
    else
    { // compute result
      assert( IsOrdered() );
      assert( p.IsOrdered() );
      Instant perMinInst; p.Minimum(perMinInst);
      Instant perMaxInst; p.Maximum(perMaxInst);
      double permind = perMinInst.ToDouble();
      double permaxd = perMaxInst.ToDouble();
      double mind = bbox.MinD(2);
      double maxd = bbox.MaxD(2);
      if( (mind > permaxd && !AlmostEqual(mind,permaxd)) ||
          (maxd < permind && !AlmostEqual(maxd,permind)))
      {
//         cout << __PRETTY_FUNCTION__<< "(" << __FILE__ << __LINE__
//           << "): Bounding box check failed:" << endl;
//         cout << "\tPeriod : "; p.Print(cout); cout << endl;
//         cout << "\tperMinInst : "; perMinInst.Print(cout); cout << endl;
//         cout << "\tperMaxInst : "; perMaxInst.Print(cout); cout << endl;
//         cout << "\tpermind/permaxd : " << permind  << "\t/\t"
//              << permaxd << endl;
//         cout << "\tmind/maxd :" << mind << "\t/\t" << maxd << endl;
//         cout << "\tBBox = "; bbox.Print(cout); cout << endl;
        result.SetDefined(true);
      } else
      {
        result.StartBulkLoad();
        UPoint unit;
        Interval<Instant> interval;
        int i = 0, j = 0;
        Get( i, unit );
        p.Get( j, interval );

        while( 1 )
        {
          if( unit.timeInterval.Before( interval ) )
          {
            if( ++i == GetNoComponents() )
              break;
            Get( i, unit );
          }
          else if( interval.Before( unit.timeInterval ) )
          {
            if( ++j == p.GetNoComponents() )
              break;
            p.Get( j, interval );
          }
          else
          { // we have overlapping intervals, now
            UPoint r(1);
            unit.AtInterval( interval, r );
            assert( r.IsDefined() );
            assert( r.IsValid()   );
            result.Add( r );
//          cout << "\n\tunit = "; unit.Print(cout); cout << endl;
//          cout << "\tinterval =       "; interval.Print(cout); cout << endl;
//          cout << "\tr    = "; r.Print(cout); cout << endl;

            if( interval.end == unit.timeInterval.end )
            { // same ending instant
              if( interval.rc == unit.timeInterval.rc )
              { // same ending instant and rightclosedness: Advance both
                if( ++i == GetNoComponents() )
                  break;
                Get( i, unit );
                if( ++j == p.GetNoComponents() )
                  break;
                p.Get( j, interval );
              }
              else if( interval.rc == true )
              { // Advanve in mapping
                if( ++i == GetNoComponents() )
                  break;
                Get( i, unit );
              }
              else
              { // Advance in periods
                assert( unit.timeInterval.rc == true );
                if( ++j == p.GetNoComponents() )
                  break;
                p.Get( j, interval );
              }
            }
            else if( interval.end > unit.timeInterval.end )
            { // Advance in mpoint
              if( ++i == GetNoComponents() )
                break;
              Get( i, unit );
            }
            else
            { // Advance in periods
              assert( interval.end < unit.timeInterval.end );
              if( ++j == p.GetNoComponents() )
                break;
              p.Get( j, interval );
            }
          }
        }
        result.EndBulkLoad();
      }
    }
  } else
  {
    result.SetDefined(false);
  }
}

/*

RestoreBoundingBox() checks, whether the MPoint's MBR ~bbox~ is ~undefined~
and thus may needs to be recalculated and if, does so.

*/

void MPoint::RestoreBoundingBox(const bool force)
{
  if(!IsDefined() || GetNoComponents() == 0)
  { // invalidate bbox
    bbox.SetDefined(false);
  }
  else if(force || !bbox.IsDefined())
  { // construct bbox
    UPoint unit;
    int size = GetNoComponents();
    Get( 0, unit ); // safe, since (this) contains at least 1 unit
    bbox = unit.BoundingBox();
    for( int i = 1; i < size; i++ ){
      Get( i, unit );
      bbox = bbox.Union(unit.BoundingBox());
    }
  } // else: bbox unchanged and still correct
}

// Class functions
Rectangle<3u> MPoint::BoundingBox(const Geoid* geoid /*=0*/) const
{
  if(geoid){ // spherical geometry case:
    if(!IsDefined() || (GetNoComponents()<=0) ){
      return Rectangle<3>(false);
    }
    UPoint u;
    Rectangle<3> bbx(false);
    for(int i=0; i<GetNoComponents(); i++){
      Get(i,u);
      assert( u.IsDefined() );
      if(bbx.IsDefined()){
        bbx.Union(u.BoundingBox(geoid));
      } else {
        bbx = u.BoundingBox(geoid);
      }
    }
    return bbx;
  } // else: euclidean case
  return bbox;
}

// return the spatial bounding box (2D: X/Y)
const Rectangle<2> MPoint::BoundingBoxSpatial(const Geoid* geoid) const {
  Rectangle<2u> result(false,0.0,0.0,0.0,0.0);
  if(!IsDefined() || (GetNoComponents()<=0) ){
    return result;
  } else {
    Rectangle<3> bbx = this->BoundingBox(geoid);
    result = bbx.Project2D(0,1); // project to X/Y
    return result;
  }
};

void MPoint::Trajectory( Line& line ) const
{
  line.Clear();
  if(!IsDefined()){
    line.SetDefined( false );
    return;
  }
  line.SetDefined( true );
  line.StartBulkLoad();

  HalfSegment hs;
  UPoint unit;
  int edgeno = 0;

  int size = GetNoComponents();
  if (size>0)
    line.Resize(size);

  Point p0(false);      // starting point
  Point p1(false);      // end point of the first unit
  Point p_last(false);  // last point of the connected segment

  for( int i = 0; i < size; i++ ) {
    Get( i, unit );

    if( !AlmostEqual( unit.p0, unit.p1 ) )    {
      if(!p0.IsDefined()){ // first unit
        p0 = unit.p0;
        p1 = unit.p1;
        p_last = unit.p1;
      } else { // segment already exists
        if(p_last!=unit.p0){ // spatial jump
           hs.Set(true,p0,p_last);
           hs.attr.edgeno = ++edgeno;
           line += hs;
           hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
           line += hs;
           p0 = unit.p0;
           p1 = unit.p1;
           p_last = unit.p1;
        } else { // an extension, check direction
           if(!AlmostEqual(p0,unit.p1)){
             HalfSegment tmp(true,p0,unit.p1);
             double dist = tmp.Distance(p1);
             double dist2 = tmp.Distance(p_last);
             if(AlmostEqual(dist,0.0) && AlmostEqual(dist2,0.0)){
               p_last = unit.p1;
             } else {
               hs.Set(true,p0,p_last);
               hs.attr.edgeno = ++edgeno;
               line += hs;
               hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
               line += hs;
               p0 = unit.p0;
               p1 = unit.p1;
               p_last = unit.p1;
             }
          }
        }
      }
    }
  }
  if(p0.IsDefined() && p_last.IsDefined() && !AlmostEqual(p0,p_last)){
    hs.Set(true,p0,p_last);
    hs.attr.edgeno = ++edgeno;
    line += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    line += hs;
  }
  line.EndBulkLoad();
}

void MPoint::Distance( const Point& p, MReal& result, const Geoid* geoid ) const
{
  result.Clear();
  if( !IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  UPoint uPoint;
  result.Resize(GetNoComponents());
  result.StartBulkLoad();
  for( int i = 0; i < GetNoComponents(); i++ ){
    Get( i, uPoint );
    vector<UReal> resvec;
    uPoint.Distance( p, resvec, geoid );
    for(vector<UReal>::iterator it=resvec.begin(); it!=resvec.end(); it++ ){
      if(it->IsDefined()){
        result.MergeAdd( *it );
      }
    }
  }
  result.EndBulkLoad( false, false );
}

void MPoint::SquaredDistance( const Point& p, MReal& result,
                              const Geoid* geoid ) const
{
  result.Clear();
  if( !IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  UPoint uPoint;
  result.Resize(GetNoComponents());
  result.StartBulkLoad();
  for( int i = 0; i < GetNoComponents(); i++ ){
    Get( i, uPoint );
    vector<UReal> resvec;
    uPoint.Distance( p, resvec, geoid );
    for(vector<UReal>::iterator it(resvec.begin()); it!=resvec.end(); it++ ){
      if(it->IsDefined()){
        UReal resunit(*it);
        assert( resunit.r );
        resunit.r = false;
        result.MergeAdd( resunit );
      }
    }
  }
  result.EndBulkLoad( false, false );
}

void MPoint::SquaredDistance( const MPoint& p, MReal& result,
                              const Geoid* geoid ) const
{
  result.Clear();
  if( !IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  UReal uReal(true);

  RefinementPartition<MPoint, MPoint, UPoint, UPoint> rp(*this, p);

  result.Resize(rp.Size());
  result.StartBulkLoad();
  for( unsigned int i = 0; i < rp.Size(); i++ )
  {
    Interval<Instant> iv;
    int u1Pos, u2Pos;
    UPoint u1;
    UPoint u2;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      Get(u1Pos, u1);
      p.Get(u2Pos, u2);
    }
    if(u1.IsDefined() && u2.IsDefined())
    { // do not need to test for overlapping deftimes anymore...
      u1.Distance( u2, uReal, geoid );
      if(!uReal.IsDefined()){
        cerr << __PRETTY_FUNCTION__
             << "Invalid geographic coord found!" << endl;
        result.EndBulkLoad( false, false );
        result.Clear();
        result.SetDefined(false);
        return;
      }
      uReal.r= false;
      result.MergeAdd( uReal );
    }
  }
  result.EndBulkLoad();
}

// Output an interval
string iv2string(Interval<Instant> iv){

   string res ="";
   res += iv.lc?"[":"(";
   res += iv.start.ToString();
   res += ", ";
   res += iv.end.ToString();
   res += iv.rc?"]":")";
   return res;
}

void MPoint::MergeAdd(const UPoint& unit){
  assert( IsDefined() );
  assert( unit.IsDefined() );
  assert( unit.IsValid() );

  int size = GetNoComponents();
  if(size==0){ // the first unit
    Add(unit); // Add() unit as first unit to empty mapping; bbox is updated.
    return;
  }
  UPoint last;
  Get(size-1,last);

  assert(last.timeInterval.end <= unit.timeInterval.start);

  if(last.timeInterval.end!=unit.timeInterval.start ||
     !( (last.timeInterval.rc )  ^ (unit.timeInterval.lc))){
     // intervals are not connected
    Add(unit); // also adopts bbox
    return;
  }
  if(!AlmostEqual(last.p1, unit.p0)){
    // jump in spatial dimension
    Add(unit);  // also adopts bbox
    return;
  }
  Interval<Instant> complete(last.timeInterval.start,
                             unit.timeInterval.end,
                             last.timeInterval.lc,
                             unit.timeInterval.rc);
  UPoint upoint(complete,last.p0, unit.p1);
  Point p;
  upoint.TemporalFunction(last.timeInterval.end, p, true);
  if(!AlmostEqual(p,last.p0)){
    Add(unit); // also adopts bbox
    return;
  }
  assert( upoint.IsValid() );
  assert( upoint.IsDefined() );
  bbox = bbox.Union(upoint.BoundingBox()); // update bbox
  units.Put(size-1,upoint); // overwrite the last unit by a connected one
}


/*
This function checks whether the end point of the first unit is equal
to the start point of the second unit and if the time difference is
at most a single instant

*/
static bool connected(const UPoint* u1, const UPoint* u2){
   if(u1->p1 != u2->p0){ // spatial connected
       return false;
   }
   // temporal connection
   if(! ((u1->timeInterval.end) == (u2->timeInterval.start))){
       return false;
   }
   return true;
}

static bool IsBreakPoint(const UPoint* u,const DateTime& duration){
   if(u->p0 != u->p1){ // equal points required
     return false;
   }
   DateTime dur = u->timeInterval.end -  u->timeInterval.start;
   return (dur > duration);
}



/**
~Simplify~

This function removes some sampling points from a moving point
to get simpler data. It's implemented using an algorithm based
on the Douglas Peucker algorithm for line simplification.

**/

void MPoint::Simplify(const double epsilon, MPoint& result,
                      const bool checkBreakPoints,
                      const DateTime& dur) const{
   result.Clear();

   // check for defined state
   if( !IsDefined() || !dur.IsDefined() ){
     result.SetDefined(false);
     return;
   }
   result.SetDefined(true);

   unsigned int size = GetNoComponents();
   // no simplification possible if epsilon < 0
   // or if at most one unit present
   if(epsilon<0 || size < 2){
      result.CopyFrom(this);
      return;
   }

   // create an boolean array which represents all sample points
   // contained in the result
   bool useleft[size];
   bool useright[size];
   // at start, no sampling point is used
   for(unsigned int i=0;i<size;i++){
       useleft[i] = false;
       useright[i] =false;
   }

   unsigned int first=0;
   unsigned int last=1;
   UPoint u1(false);
   UPoint u2(false);
   while(last<size){
      // check whether last and last -1 are connected
      Get(last-1,u1);
      Get(last,u2);

      if( checkBreakPoints && IsBreakPoint(&u1,dur)){
         if(last-1 > first){
            Simplify(first,last-2,useleft,useright,epsilon);
         }
         Simplify(last-1, last-1, useleft, useright, epsilon);
         first = last;
         last++;
      } else if( checkBreakPoints && IsBreakPoint(&u2,dur)){
         Simplify(first,last-1,useleft,useright,epsilon);
         last++;
         Simplify(last-1, last-1,useleft,useright,epsilon);
         first = last;
         last++;
      } else if(connected(&u1,&u2)){ // enlarge the sequence
         last++;
      } else {
          Simplify(first,last-1,useleft, useright, epsilon);
          first=last;
          last++;
      }
   }
   // process the last recognized sequence
   Simplify(first,last-1,useleft, useright,epsilon);


   // count the number of units
   int count = 1; // count the most right sample point
   for( unsigned int i=0;i<size;i++){
      if( useleft[i]){
         count++;
      }
   }

   result.Resize(count); // prepare enough memory

   result.StartBulkLoad();
   Instant start;
   Point p0;
   bool closeLeft = false;
   bool leftDefined = false;
   for(unsigned int i=0; i< size; i++){
     UPoint upoint(false);

     Get(i,upoint);
     if(useleft[i]){
        // debug
        if(leftDefined){
           cerr << " error in mpoint simplification,"
                << " overwrite an existing leftPoint "  << endl;
        }
        // end of debug
        p0 = upoint.p0;
        closeLeft = upoint.timeInterval.lc;
        start = upoint.timeInterval.start;
        leftDefined=true;
     }
     if(useright[i]){
        // debug
        if(!leftDefined){
           cerr << " error in mpoint simplification,"
                << " rightdefined before leftdefined "  << endl;

        }
        Interval<Instant> interval(start,upoint.timeInterval.end,closeLeft,
                                   upoint.timeInterval.rc);

        UPoint newUnit(interval,p0,upoint.p1);
        result.Add(newUnit);
        leftDefined=false;
     }
   }
   result.EndBulkLoad(false,false);
}


/**
~Simplify~

Recursive implementation of simplifying movements.

**/

void MPoint::Simplify(const int min,
                 const int max,
                 bool* useleft,
                 bool* useright,
                 const double epsilon) const {

  // the endpoints are used in each case
  useleft[min] = true;
  useright[max] = true;

  if(min==max){ // no intermediate sampling points -> nothing to simplify
     return;
  }

  UPoint u1;
  UPoint u2;
  // build a UPoint from the endpoints
  Get(min,u1);
  Get(max,u2);

  UPoint upoint(Interval<Instant>(u1.timeInterval.start,
                u2.timeInterval.end,true,true),
                u1.p0,
                u2.p1);

  // search for the point with the highest distance to its simplified position
  double maxDist = 0;
  int maxIndex=0;
  Point p_orig;
  Point p_simple;
  UPoint u;
  double distance;
  for(int i=min+1;i<=max;i++){
     Get(i,u);
     upoint.TemporalFunction(u.timeInterval.start,p_simple, true);
     distance  = p_simple.Distance(u.p0);
     if(distance>maxDist){ // new maximum found
        maxDist = distance;
        maxIndex = i;
     }
  }

  if(maxIndex==0){  // no difference found
      return;
  }
  if(maxDist<=epsilon){  // differnce is in allowed range
      return;
  }

  // split at the left point of maxIndex
  Simplify(min,maxIndex-1,useleft,useright,epsilon);
  Simplify(maxIndex,max,useleft,useright,epsilon);
}



void MPoint::BreakPoints(Points& result, const DateTime& dur) const{
    result.Clear();
    if( !IsDefined() || !dur.IsDefined() ){
      result.SetDefined(false);
      return;
    }
    result.SetDefined(true);
    int size = GetNoComponents();
    result.StartBulkLoad();
    UPoint unit;
    for(int i=0;i<size;i++){
        Get(i,unit);
        if(IsBreakPoint(&unit,dur)){
           result += (unit.p0);
        }
    }
    result.EndBulkLoad();
}

void MPoint::BreakPoints(Points& result, const DateTime& dur, 
                         const CcReal& epsilon, 
                         const Geoid* geoid /*=0*/ ) const{

    result.Clear();
    if(!IsDefined() || !dur.IsDefined() || !epsilon.IsDefined()){
       result.SetDefined(false);
       return;
    }

    double eps = epsilon.GetValue();
    if(eps<0){
      return; // we cannot find distances smaller than zero 
    }

    result.SetDefined(true);
    int size = GetNoComponents();
    result.StartBulkLoad();
    UPoint unit;
    UPoint firstUnit;
    Point firstPoint;
    int firstIndex=0;
    int index = 0;
    DateTime currentDur(datetime::durationtype);

    while(firstIndex < size){
      if(index == firstIndex){ 
         Get(firstIndex,firstUnit);
         if(firstUnit.p0.Distance(firstUnit.p1,geoid) > eps){
            // this units overcomes the maximum epsilon value
            index++;
            firstIndex++;
         } else { // try to find more points for this break
           firstPoint = firstUnit.p0;
           currentDur =  firstUnit.timeInterval.end - 
                         firstUnit.timeInterval.start;
           index++;
           if(index>=size){
             if(currentDur >= dur){
               result += firstPoint;
             }
             firstIndex = index;

           }
         }
      } else {
        assert(index > firstIndex);
        UPoint lastUnit;
        Get(index-1, lastUnit);
        Get(index, unit);
        if(!lastUnit.Adjacent(&unit)){ // gap found, close chain
           if(currentDur >= dur){
              result += firstPoint;
           }
           firstIndex = index; // start a new try
        } else {
          Point rp = unit.p1;
          if(firstPoint.Distance(rp,geoid) > eps){
             // next unit does not contribute to break
             if(currentDur >= dur){
                result += firstPoint;
                firstIndex = index;
             } else {
                // start a new try
                firstIndex++;
                index = firstIndex; 
            }
          } else {
            // extend the possible break   
            currentDur += (unit.timeInterval.end - unit.timeInterval.start);
            index++;
            if(index >= size){
               firstIndex = index;
               if(currentDur >= dur){
                  result += firstPoint;
               }
            }
          } 
        }
      }
    }
    result.EndBulkLoad();
}


void MPoint::Breaks(Periods& result, const DateTime& dur, 
                         const CcReal& epsilon, 
                         const Geoid* geoid /*=0*/ ) const{

    result.Clear();
    if(!IsDefined() || !dur.IsDefined() || !epsilon.IsDefined()){
       result.SetDefined(false);
       return;
    }

    double eps = epsilon.GetValue();
    if(eps<0){
      return; // we cannot find distances smaller than zero 
    }

    result.SetDefined(true);
    Periods tmp(0);
    int size = GetNoComponents();
    //result.StartBulkLoad();
    UPoint unit;
    UPoint firstUnit;
    Point firstPoint;
    DateTime firstTime;
    int firstIndex=0;
    int index = 0;
    DateTime currentDur(datetime::durationtype);

    while(firstIndex < size){
      if(index == firstIndex){ 
         Get(firstIndex,firstUnit);
         if(firstUnit.p0.Distance(firstUnit.p1,geoid) > eps){
            // this units overcomes the maximum epsilon value
            index++;
            firstIndex++;
         } else { // try to find more points for this break
           firstPoint = firstUnit.p0;
           firstTime = firstUnit.timeInterval.start;
           currentDur =  firstUnit.timeInterval.end - 
                         firstUnit.timeInterval.start;
           index++;
           if(index>=size){
             if(currentDur >= dur){
               Interval<Instant> iv(firstTime, firstTime+currentDur,true,true);
               result.Union(iv, tmp);
               result.CopyFrom(&tmp);
             }
             firstIndex = index;

           }
         }
      } else {
        assert(index > firstIndex);
        UPoint lastUnit;
        Get(index-1, lastUnit);
        Get(index, unit);
        if(!lastUnit.Adjacent(&unit)){ // gap found, close chain
           if(currentDur >= dur){
              Interval<Instant> iv(firstTime,firstTime + currentDur, 
                                   true, true); 
              result.Union(iv, tmp);
              result.CopyFrom(&tmp);
           }
           firstIndex = index; // start a new try
        } else {
          Point rp = unit.p1;
          if(firstPoint.Distance(rp,geoid) > eps){
             // next unit does not contribute to break
             if(currentDur >= dur){
                Interval<Instant> iv(firstTime,firstTime + currentDur, 
                                     true, true); 
                result.Union(iv, tmp);
                result.CopyFrom(&tmp);
                firstIndex = index;
             } else {
                // start a new try
                firstIndex++;
                index = firstIndex;
            }
          } else {
            // extend the possible break   
            currentDur += (unit.timeInterval.end - unit.timeInterval.start);
            index++;
            if(index >= size){
               firstIndex = index;
               if(currentDur >= dur){
                Interval<Instant> iv(firstTime,firstTime + currentDur,
                                     true, true);
                result.Union(iv, tmp);
                result.CopyFrom(&tmp);
               }
            }
          } 
        }
      }
    }
    //result.EndBulkLoad();
}


void MPoint::TranslateAppend(const MPoint& mp, const DateTime& dur){
   if( !IsDefined() || !mp.IsDefined() || !dur.IsDefined() ){
       Clear();
       SetDefined(false);
       return;
   }
   if(mp.GetNoComponents()==0){ // nothing to do (already defined)
       return;
   }
   if(GetNoComponents()==0){
       this->CopyFrom(&mp);
       return;
   }

   int newSize = GetNoComponents()+mp.GetNoComponents();
   Resize(newSize);
   UPoint lastUnit;

   StartBulkLoad();

   UPoint firstUnit;
   mp.Get(0,firstUnit);

   // add a staying unit
   if(!dur.IsZero() && !dur.LessThanZero()){
     Get(GetNoComponents()-1,lastUnit);
     Interval<Instant> interval = lastUnit.timeInterval;
     Point lastPoint = lastUnit.p1;
     // append a unit of staying
     Interval<Instant> gapInterval(interval.end,interval.end +dur,
                                   !interval.rc,!firstUnit.timeInterval.lc);
     UPoint gap(gapInterval,lastPoint,lastPoint);
     Add(gap);
   }

   Get(GetNoComponents()-1,lastUnit);
   Instant end = lastUnit.timeInterval.end;
   DateTime timediff = end - firstUnit.timeInterval.start;
   double xdiff  = lastUnit.p1.GetX() - firstUnit.p0.GetX();
   double ydiff  = lastUnit.p1.GetY() - firstUnit.p0.GetY();

   UPoint Punit;
   mp.Get(0,Punit);
   UPoint unit = Punit;
   unit.Translate(xdiff,ydiff,timediff);
   if(!(lastUnit.timeInterval.rc)){
       unit.timeInterval.lc = true;
   } else {
       unit.timeInterval.lc = false;
   }
   Add(unit);

   for(int i=1; i< mp.GetNoComponents(); i++){
      mp.Get(i,Punit);
      unit = Punit;
      unit.Translate(xdiff,ydiff,timediff);
      Add(unit);
   }
   EndBulkLoad(false);
}


void MPoint::Reverse(MPoint& result){
    result.Clear();
    if(!IsDefined()){
       result.SetDefined(false);
       return;
    }
    result.SetDefined(true);
    int size = GetNoComponents();
    if(size==0){
       return;
    }

    UPoint unit;
    Get(size-1,unit);
    Instant end = unit.timeInterval.end;
    Get(0,unit);
    Instant start = unit.timeInterval.start;

    result.StartBulkLoad();

    for(int i=size-1; i>=0; i--){
       Get(i,unit);
       Instant newEnd = (end - unit.timeInterval.start) + start;
       Instant newStart = (end - unit.timeInterval.end) + start;
       Interval<Instant> interval(newStart,newEnd,
                                  unit.timeInterval.rc,
                                  unit.timeInterval.lc);
       UPoint newUnit(interval,unit.p1,unit.p0);

       result.Add(newUnit);
    }
    result.EndBulkLoad(false);
}

void MPoint::Direction( MReal* result,
                        const bool useHeading /*=false*/,
                        const Geoid* geoid    /*=0*/,
                        const double epsilon  /*=0.0000001*/ ) const
{
  if( !IsDefined() || ( geoid && (!geoid->IsDefined() || (epsilon<=0.0)) ) ) {
    result->SetDefined(false);
    return;
  }
  vector<UReal> resvector;
  UPoint unitin;
  for(int i=0;i<GetNoComponents();i++) {
    Get(i, unitin);
    unitin.Direction( resvector, useHeading, geoid, epsilon );
  }
  result->Clear();
  result->SetDefined(true);
  result->StartBulkLoad();
  for(vector<UReal>::iterator iter = resvector.begin();
  iter != resvector.end(); iter++){
    if( iter->IsDefined() && iter->IsValid() ){
      result->MergeAdd(*iter);
    }
  }
  result->EndBulkLoad( false );
  return;
}


/*
~qdist~

This funtion returns the square of the distance between ~p1~ and ~p2~.

*/
double qdist(const Point& p1, const Point& p2){
   double x1 = p1.GetX();
   double y1 = p1.GetY();
   double x2 = p2.GetX();
   double y2 = p2.GetY();
   double dx = x2-x1;
   double dy = y2-y1;
   return dx*dx + dy*dy;
}

/*
~qdist~

This function returns the square of the distance of the points defined by
(~x1~, ~y1~) and (~x2~, ~y2~).

*/

double qdist(const double& x1, const double& y1,
             const double& x2, const double& y2){
   double dx = x2-x1;
   double dy = y2-y1;
   return dx*dx + dy*dy;
}


/*
struct ~intset~

This class allows reference counting for a set of integers.

*/
struct intset{
  intset():member(),refs(1){}

  void deleteIfAllowed(){
     refs--;
     if(refs<1){
        delete this;
     }
  }

  set<int> member;
  int refs;
};


/*
~cluster~

A cluster is defined by the contained points (stored in ~member~),
the center (defined by (~cx~, ~cy~) ) and a flag ~forbidden~ indicating
whether it's allowed to change the content of this cluster.
To save memory, instead of points only indexes of the points within an external
point vector or similar are stored.
Members are always copied by reference.

*/
struct cluster{
/*
~Default constructor~

*/
    cluster(){
      cx = 0.0;
      cy = 0.0;
      member = new intset();
      forbidden = false;
    }

/*
~Copy Constructor~

*/
    cluster(const cluster& c){
       cx = c.cx;
       cy = c.cy;
       forbidden = c.forbidden;
       member = c.member;
       member->refs++;
    }

/*
~Assignment Operator~

*/
    cluster& operator=(const cluster& c){
       cx = c.cx;
       cy = c.cy;
       forbidden = c.forbidden;
       member = c.member;
       member->refs++;
       return *this;
    }

/*
~Destructor~

*/
    ~cluster(){
      member->deleteIfAllowed();
    }

/*
~begin~

This function returns an iterator pointing to the begin of the
contained intset.

*/
    set<int>::iterator begin() const{
      return member->member.begin();
    }

/*
~end~

This function returns an iterator pointing to the end of the contained intset.

*/
    set<int>::iterator end() const {
      return member->member.end();
    }

/*
~size~

Tells, how many integers are contained in this cluster.

*/
    size_t size() const {
      return member->member.size();
    }

/*
~insert~

Inserts a new point index into the cluster.
There is no correction of the center.

*/
    void insert(int i){
      member->member.insert(i);
    }

/*
~erase~

Erases a point index without correcting the center.

*/
    void erase(int i){
       member->member.erase(i);
    }

/*
~clear~

Removes all point indexes.

*/
    void clear(){
       member->member.clear();
    }

/*
~recomputeCenter~

Recomputes the cluster's center.

*/
   void recomputeCenter(const vector<Point>& points){
      set<int>::iterator it;
      double x = 0.0;
      double y = 0.0;
      for(it=begin();it!=end();it++){
         Point p = points[*it];
         x += p.GetX();
         y += p.GetY();
      }
      double s = size();
      cx = x / s;
      cy = y / s;
   }


/*
~Data Members~

(cx, cy)   : center of the cluster [nl]
member     : intset storing the iindexes of the contained points [nl]
forbidding : flag indicating whether changes are allowed

*/
    double cx;
    double cy;
    intset* member; // avoid copying of this set !!!!
    bool forbidden;
};

/*
~indexOfNearestCluster~

Returns the index of the cluster closest to ~p~ within ~clusters~
within a range of ~eps~. If no center of any cluster
ios nearer than ~eps~ to ~p~ , -1 is returned. To accelerate the
search, the centers of the clusters are stored in the rtree ~tree~.

*/

int indexOfNearestCluster( const mmrtree::Rtree<2>& tree,
                           const Point& p,
                           const vector<cluster>& clusters,
                           const double& eps){
  double eps2 = eps*eps;
  int res = -1;
  double bestDist = eps2 + 10.0;  // a value greater than eps2
  // build a rectangle around p
  double min[2];
  double max[2];
  double x = p.GetX();
  double y = p.GetY();
  min[0] = x - eps - FACTOR;
  min[1] = y - eps - FACTOR;
  max[0] = x + eps + FACTOR;
  max[1] = y + eps + FACTOR;
  Rectangle<2> searchbox(true,min,max);

  set<long> cands;
  tree.findAll(searchbox, cands);

  set<long>::iterator it;
  for(it = cands.begin(); it!=cands.end(); it++){
   cluster c = clusters[*it];
   double d = qdist(c.cx,c.cy,x,y);
   if((d <= eps2) && (d < bestDist) && !c.forbidden){
     bestDist = d;
     res = *it;
   }
  }
  return res;
}


// forward declaration
void insertPoint(mmrtree::Rtree<2>& tree,
                 const vector<Point>& points,
                 const int pos,
                 vector<cluster>& clusters,
                 const double& eps);

/*
~repairClusterAt~

This function removes all points from the cluster, whose distance to the
cluster's center is greater than ~eps~.


*/
void repairClusterAt(const int index,
                     mmrtree::Rtree<2>& tree,
                     vector<cluster>& clusters,
                     const vector<Point>& points,
                     const double& eps){
  double eps2 = eps*eps;
  clusters[index].forbidden = true;
  double cx = clusters[index].cx;
  double cy = clusters[index].cy;
  // store all invalid points to wrong
  set<int>wrong;
  set<int>::iterator it;
  for(it = clusters[index].begin();
      it != clusters[index].end();
      it++){
    Point p = points[*it];
    double d = qdist(cx,cy,p.GetX(),p.GetY());
    if(d>eps2){
      wrong.insert(*it);
    }
  }
  // remove invalid points
  for(it=wrong.begin();it!=wrong.end();it++){
    clusters[index].erase(*it);
  }

  // insert points again
  for(it =wrong.begin(); it!=wrong.end(); it++){
    insertPoint(tree,points,*it,clusters,eps);
  }

  clusters[index].forbidden = false;

}

/*
~insertPoint~

This function searches the nearest cluster withing ~clusters~.
If the center's distance is greater than ~eps~, a new cluster
is created containing exactly this point. Otherwise, the point is
inserted into this cluster. The cluster's center is corrected and all
'bad points' are moved into other clusters.

*/
void insertPoint(mmrtree::Rtree<2>& tree,
                 const vector<Point>& points,
                 const int pos,
                 vector<cluster>& clusters,
                 const double& eps){

   Point p = points[pos];
   int index = indexOfNearestCluster(tree, p, clusters, eps);
   double x = p.GetX();
   double y = p.GetY();
   double min[2];
   double max[2];
   if(index < 0){ // no matching cluster exists, create a new one
      cluster c;
      c.cx = x;
      c.cy = y;
      c.insert(pos);
      c.forbidden = false;
      clusters.push_back(c);
      min[0] = x - FACTOR;
      max[0] = x + FACTOR;
      min[1] = y - FACTOR;
      max[1] = y + FACTOR;
      Rectangle<2> box(true,min,max);
      tree.insert(box, clusters.size()-1);
      return;
   }

   clusters[index].insert(pos);

   double cx = clusters[index].cx;
   double cy = clusters[index].cy;
   int s = clusters[index].size();
   clusters[index].cx = ((cx * (s - 1.0) + x) / s);
   clusters[index].cy = ((cy * (s - 1.0) + y) / s);

//   clusters[index].recomputeCenter(points);


   min[0] = cx - FACTOR;
   min[1] = cy - FACTOR;
   max[0] = cx + FACTOR;
   max[1] = cy + FACTOR;
   Rectangle<2> erasebox(true,min,max);
   tree.erase(erasebox,index);

   min[0] = clusters[index].cx - FACTOR;
   min[1] = clusters[index].cy - FACTOR;
   max[0] = clusters[index].cx + FACTOR;
   max[1] = clusters[index].cy + FACTOR;

   Rectangle<2> newCenter(true,min,max);
   tree.insert(newCenter,index);
   repairClusterAt(index, tree, clusters, points, eps);
}

/*
~getCenter~

Computes the center of the cluster and stored it in ~x~ and ~y~.

*/

void getCenter(const cluster& cl,
               const vector<Point>& points,
               double& x, double& y){
 x = 0;
 y = 0;
 int size = cl.size();
 if(size==0){
   cerr << "indexes smaller than zero" << endl;
   return;
 }
 set<int>::const_iterator it;
 for(it=cl.begin();it!=cl.end();it++){
     x += points[*it].GetX();
     y += points[*it].GetY();
 }
 x = x / size;
 y = y / size;

}


/*
~recomputeCenter~

Computes the center of a cluster from the members.

*/
void recomputeCenters(vector<cluster>& clusters,
                      const vector<Point>& points){
  vector<cluster>::iterator it;
  for(it = clusters.begin(); it!=clusters.end();it++){
     getCenter(*it,points,it->cx,it->cy);
  }
}

/*
This class is only needed, because the Point class of Secondo
does no initialisation within in the standard constructor which is
required to use a class within an STL set instance.

*/
class DefPoint:public Point{
public:
  DefPoint(){
    SetDefined(false);
    del.refs=1;
    del.SetDelete();
  }
  DefPoint(const Point& p){
    Set(p.IsDefined(),p.GetX(),p.GetY());
  }
  DefPoint(const DefPoint& p){
    Set(p.IsDefined(),p.GetX(),p.GetY());
    del.refs=1;del.SetDelete();
  }
  ~DefPoint(){}
  DefPoint& operator=(const DefPoint& p){
    Set(p.IsDefined(),p.GetX(),p.GetY());
    return *this;
  }

  inline void Set(bool def, const Coord& x, const Coord& y){
     if(def){
       Point::Set(x,y);
     } else {
       SetDefined(false);
     }
  }
  inline Point GetPoint(){
     Point p(IsDefined(),GetX(),GetY());
     return p;
  }
};

/*
~assignCluster~


Creates a map point -> point where each point within the points vector
is assigned to the nearest cluster center.

*/
map<DefPoint, DefPoint>* assignCluster(const vector<Point>& points,
                                 const double& eps,
                                 vector<Point>& centers){

  vector<cluster> clusters;

  mmrtree::Rtree<2> tree(2,5);
  for(unsigned int i=0;i<points.size();i++){
    insertPoint(tree, points, i, clusters,eps);
  }

  // redistribute points
  for(unsigned int i = 0; i< clusters.size();i++){
    clusters[i].clear();
  }

  for(unsigned int i=0;i<points.size();i++){
    int index = indexOfNearestCluster(tree, points[i], clusters, eps);
    clusters[index].insert(i);
  }

  // correct the centers
  recomputeCenters(clusters,points);


  // store as a map
  map<DefPoint, DefPoint>* result= new map<DefPoint, DefPoint>();
  for(unsigned int i=0;i<clusters.size();i++){
    cluster c = clusters[i];
    Point center(true,c.cx,c.cy);
    set<int>::iterator it;
    centers.push_back(center);
    for(it=clusters[i].begin(); it!=clusters[i].end(); it++){
       Point p = points.at(*it);
       DefPoint p1(p);
       DefPoint c1(center);
       (*result)[p1] = c1;
    }
  }
  return result;
}


/*
~DoublePoint~

This class collects a Point and a double.

*/
class DoublePoint{
 public:
  DoublePoint(const double d1, const Point p1):d(d1),p(p1){}

  DoublePoint(const DoublePoint& dp):d(dp.d),p(dp.p){}

  DoublePoint& operator=(const DoublePoint& dp){
    this->d = dp.d;
    this->p = dp.p;
    return *this;
  }


  bool operator<(const DoublePoint& dp)const{
    return !AlmostEqual(d,dp.d) && d < dp.d;
  }
  bool operator==(const DoublePoint& dp)const{
    return AlmostEqual(d,dp.d);
  }


  double d;
  Point p;
};


/*
~Split~

Determines such points within ~cands~ whose distance to the
trajectory (a segment) of the unit is smaller than ~eps~ div 2. If so, the
foot of the point to the segment is determined. If the foot is located
on the segment, the unit is split at that position and the parts are
inserted into result. If no such point is found, the complete unit is inserted
into the result.

*/
void split(const UPoint unit,
           const set<long>& cands,
           const vector<Point>& points,
           MPoint& result,
           const double eps){

   // special case, not a segment
   if(AlmostEqual(unit.p0, unit.p1)){
     // nothing to split, just copy the unit
     result.Add(unit);
     return;
   }

   // at least the end points must be contained in cands
   if(cands.size() < 3 ){ // only the two endpoints
     result.Add(unit);
     return;
   }

   set<DoublePoint> splitElements;

   Point p0 = unit.p0;
   Point p1 = unit.p1;
   HalfSegment hs(true,unit.p0,unit.p1);

   double len = p0.Distance(p1);

   // determine the split positions as set of double values

   set<long>::iterator cit;

   for(cit=cands.begin(); cit!=cands.end(); cit++){
      // check this computation
      Point R = points[*cit];

      if(hs.Contains(R)){
        if(!AlmostEqual(R,p0)){
          double splitPos = p0.Distance(R) / len;
          DoublePoint dp(splitPos,R);
          splitElements.insert(dp);
        }
      } else {
        double x = p0.GetX();
        double y = p0.GetY();
        double dx = p1.GetX()-x;
        double dy = p1.GetY()-y;

        double dx2,dy2;
        if(AlmostEqual(dy,0)){
           dx2 = 0;
           dy2 = 1;
        } else if(AlmostEqual(dx,0)){
          dx2 = 1;
          dy2 = 0;
        } else {
          dx2 = 1;
          dy2 = -dx/dy;
        }
        double x2 = R.GetX();
        double y2 = R.GetY();
        double t1,t2;
        if(!AlmostEqual(dx,0)){
           t2 = (y2*dx + x*dy -x2*dy - dx*y) / (dx2*dy - dy2*dx);
           t1 = (x2+t2*dx2-x)/dx;
        } else {
           t1 = (y2*dx2 + x*dy2 -x2*dy2 - y*dx2) / (dy*dx2 - dx*dy2);
           t2 = (x+t1*dx-x2)/dx2;
        }
        double fx = x + t1*dx;
        double fy = y + t1*dy;
        Point f(true,fx,fy);
        if( (R.Distance(f) < eps) && (t1>=0) && (t1<=1)){
          DoublePoint dp(t1,R);
          splitElements.insert(dp);
        } // foot outside hs
      } // point outside hs
   } // for all candidates



   if(splitElements.size()<2){ // only the endpoint is member of splitPoints
     result.Add(unit);
     return;
   }

   set<DoublePoint>::iterator it;


   // split the unit
   bool      lastLC = unit.timeInterval.lc;
   double    lastSplit = 0;
   Point     lastPoint = p0;
   Instant   lastTime = unit.timeInterval.start;
   Instant   startTime = unit.timeInterval.start;
   DateTime dur = unit.timeInterval.end - unit.timeInterval.start;
   DateTime tmp(durationtype);

   for(it = splitElements.begin(); it!=splitElements.end();it++){
      DoublePoint dp = *it;
      double sE = dp.d;
      if(!AlmostEqual(lastSplit,sE)){
        // compute the splitPoint
        Point sP = dp.p;
        if(!AlmostEqual(sP,lastPoint)){
           DateTime copy(dur);
           copy.Split(sE,tmp);
           if(!copy.IsZero()){
              DateTime end = startTime + copy;
              if(lastTime!=end ||
                 (AlmostEqual(sP,p1) &&
                  lastLC != unit.timeInterval.rc)){
                 Interval<Instant> interval(lastTime,end,
                                         lastLC, !lastLC);
                 if(AlmostEqual(sP,p1)){
                   interval.rc = (unit.timeInterval.rc);
                 }
                 UPoint u(interval,lastPoint,sP);
                 result.Add(u);
                 lastSplit = sE;
                 lastPoint = sP;
                 lastTime = end;
             }
           }
        }
      }
   }
}


void eqTimes(MPoint& mpoint,
             vector<int>& indexes,
             MPoint& changed,
             DbArray<bool>& used,
             DateTime& eps) {

   // first collect the different durations within a set
   //vector<int>::iterator it;
   //vector<UPoint> units;
   //DateTime minDur(durationtype);
   //DateTime maxDur(durationtype);
   //const UPoint* unit;


}


/*
~ChangeTimes~

This function tries to equalize the durations of units covering the
same space. The maximum deviation is given by ~eps~.

*/
void changeTimes(MPoint& mpoint, const DateTime& eps){
   assert( mpoint.IsDefined() );
   assert( eps.IsDefined() );
   if( !mpoint.IsDefined() || !eps.IsDefined() ){
    mpoint.Clear();
    mpoint.SetDefined( false );
    return;
   }

   // step 1: collect the rectangles of all units into an rtree
   mmrtree::Rtree<2> tree(3,7);
   UPoint unit=0;
   for(int i=0;i<mpoint.GetNoComponents();i++){
       mpoint.Get(i,unit);
       tree.insert(unit.BoundingBoxSpatial(),i);
   }

   cout << " There are " << tree.noObjects() << " stored in the tree " << endl;

 /*
   DbArray<bool> used(mpoint.GetNoComponents);
   MPoint changed(mpoint);

   for(int i=0;i<mpoint.GetNoComponents()){
     used.Append(false);
   }

   bool u;
   UPoint unit;

   for(int i=0;i<mpoint.NoComponents();i++){
      used.Get(i,u);
      if(!u){ // not used unit
         mpoint.Get(i,unit);
         Rectangle2D box(unit.GetSpatialBoundingBox());
         vector<long> c;
         tree.findAllExact(box,c);
         vector<long>::iterator it;
         vector<int> indexes;
         bool u2;
         UPoint unit2;
         for(it i=c.begin();i!=c.end();it++){
           used.Get(*it,u2);
           if(!u2){
              mpoint.Get(*it,unit2);
              if(AlmostEqual(unit.p0,unit2.p0) &&
                 AlmostEqual(unit.p1,unit2.p1)){
                 indexes.push_back(*it);
              }
           }
         }
         eqTimes(mpoint,indexes,changed,used,eps);
      }
   }

 */

}

/*
~EqualizeUnitsSpatial~

This function tries to equalize similar units in spatial dimension.
This means if the segments of two (or more) units are similiar, the
endpoints of the units are changed  to cover the same segment.


*/
void MPoint::EqualizeUnitsSpatial(const double epsilon,
                                  MPoint& result,
                                  bool skipSplit/* = false*/) const{
   result.Clear();
   if(!IsDefined()){
    result.SetDefined( false );
    return;
   }
   result.SetDefined( true );

   int size = GetNoComponents();
   if(size<2){ // no or a single unit
      return;
   }

   // step 1: collect all start and endpoints of units within a set
   set<Point> endPoints1;
   UPoint unit;
   for(int i=0;i< GetNoComponents(); i++){
       Get(i,unit);
       Point p0(unit.p0);
       endPoints1.insert(p0);
       Point p1(unit.p1);
       endPoints1.insert(p1);
   }

   // copy points from the set to a vector
   vector<Point>  endPoints;

   set<Point>::iterator it;
   for(it=endPoints1.begin(); it!=endPoints1.end(); it++){
     endPoints.push_back(*it);
   }

  // step 2: build cluster and move the endpoints to the centers
   vector<Point> centers;
   map<DefPoint , DefPoint>* clusters;
   clusters = assignCluster(endPoints,epsilon,centers);

  if(skipSplit){
    UPoint resUnit(0);
    //result.StartBulkLoad();
    for(int i=0;i<GetNoComponents(); i++){
      Get(i,unit);
      resUnit = unit;
      Point p = unit.p0;
      resUnit.p0 = (*clusters)[p].GetPoint();
      resUnit.p1 = (*clusters)[unit.p1].GetPoint();
      result.MergeAdd(resUnit);
    }
    //result.EndBulkLoad();
    delete clusters;
  }

   MPoint tmp(GetNoComponents());
   UPoint resUnit(0);
   tmp.StartBulkLoad();
   for(int i=0;i<GetNoComponents(); i++){
      Get(i,unit);
      resUnit = unit;
      Point p = unit.p0;
      resUnit.p0 = (*clusters)[p].GetPoint();
      resUnit.p1 = (*clusters)[unit.p1].GetPoint();
      tmp.MergeAdd(resUnit);
   }
   tmp.EndBulkLoad();

   // split units at center
   mmrtree::Rtree<2> tree(10,30);
   for(unsigned int i=0;i<centers.size();i++){
       tree.insert(centers[i].BoundingBox(),i);
   }

   set<long> cands;
   for(int i=0;i<tmp.GetNoComponents();i++){
       tmp.Get(i,unit);
       if(AlmostEqual(unit.p0,unit.p1)){
          result.Add(unit);
       } else {
          HalfSegment hs(true,unit.p0,unit.p1);
          Rectangle<2> box = hs.BoundingBox();
          tree.findAll(box.Extend(epsilon),cands);
          split(unit,cands,centers,result,epsilon);
       }
   }
   changeTimes(result, DateTime(0,2000,durationtype));
   delete clusters;
}


/*
~isCut~

Helping function for the Sample operator. This function returns __true__
iff
    * there is a gap in the definition time between u1 and u2
    * there is a spatial jump between u1 and u2
    * the direction is changed
    * the first unit is a break (stationary unit)

*/

bool isCut(const UPoint* u1, const UPoint* u2){
   assert( u1->IsDefined() );
   assert( u2->IsDefined() );
   DateTime end = u1->timeInterval.end;
   DateTime start = u2->timeInterval.start;
   if(end!=start){ // gap in definition time
     return true;
   }
   if(!AlmostEqual(u1->p1,u2->p0)){ // jump
     return true;
   }
   Point p1 = u1->p0;
   Point p2 = u1->p1;
   Point p3 = u2->p1;
   double dx1 = p2.GetX()-p1.GetX();
   double dy1 = p2.GetY()-p1.GetY();
   double dx2 = p3.GetX()-p2.GetX();
   double dy2 = p3.GetY()-p2.GetY();

   bool res = !AlmostEqual(dx1*dy2, dx2*dy1);

   if(!res){
      if(AlmostEqual(dx1,0) && AlmostEqual(dy1,0)){ // first unit is a break
        res = !( AlmostEqual(dx2,0) && AlmostEqual(dy2,0));
      }
   }
   return res;
}


void MPoint::Sample(const DateTime& duration,
                    MPoint& result,
                    const bool KeepEndPoint, /*=false*/
                    const bool exactPath /*=false*/    ) const{

  result.Clear();
  // special case: undefined parameter
  if(!IsDefined() || !duration.IsDefined()){
    result.SetDefined(false);
    return;
  }
  result.SetDefined( true );
  int size = GetNoComponents();
  // special case: empty mpoint
  if(size==0){  // empty
     return;
  }

  bool isFirst = true;
  int currentUnit = 0;
  Instant currentTime;
  Instant lastTime;
  Point lastPoint;
  Point point;

  UPoint unit; // the unit corresponding to the currentUnit
  bool lc = false;
  while(currentUnit < size ){ // there are remaining units
     bool cut = false;
     if(isFirst){ // set the start values
         Get(currentUnit,unit);
         currentTime = unit.timeInterval.start;
         lastPoint = unit.p0;
         isFirst=false;
         lc = unit.timeInterval.lc;
     } else {
         lc = true;
     }
     Interval<Instant> interval(unit.timeInterval);
     lastTime = currentTime;
     currentTime += duration; // the next sampling instant

     // search the unit having endtime for currentTime
     while(interval.end < currentTime &&
           currentUnit < size &&
           (!cut || !exactPath)){
        currentUnit++;
        if(currentUnit<size){
           Get(currentUnit,unit);
           if(exactPath && (currentUnit>0)){
              UPoint lastUnit;
              Get(currentUnit-1,lastUnit);
              cut = isCut(&lastUnit,&unit);
              if(!cut){
                interval = unit.timeInterval;
              }
           }else{
              interval = unit.timeInterval;
           }
        }
     }

     if(cut){ // cut detected
       UPoint lastUnit;
       Get(currentUnit-1,lastUnit);
       currentTime = lastUnit.timeInterval.end;
       Interval<Instant> newint(lastTime,currentTime,lc,false);
       UPoint nextUnit(newint,lastPoint,lastUnit.p1);
       if(nextUnit.IsValid()){
          result.MergeAdd(nextUnit);
       }
       isFirst = true;
     } else if(currentUnit<size){
        if(interval.start>currentTime){ // gap detected
            isFirst=true;
        } else {
            unit.TemporalFunction(currentTime, point, true);
            Interval<Instant> newint(lastTime,currentTime,lc,false);
            UPoint nextUnit(newint,lastPoint,point);
            if(nextUnit.IsValid()){
               result.MergeAdd(nextUnit);
            }
            lastPoint = point;
        }
     }

  }

  if(KeepEndPoint || exactPath){
     Get(size-1,unit);
     if(lastTime < unit.timeInterval.end){ // gap between end of the unit
                                              // and last sample point
        Interval<Instant> newint(lastTime,unit.timeInterval.end,
                                 lc,unit.timeInterval.rc);
        UPoint nextUnit(newint,lastPoint,unit.p1);
        result.MergeAdd(nextUnit);
     }
  }
}




void MPoint::MVelocity( MPoint& result ) const
{
  result.Clear();
  if( !IsDefined() ){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  UPoint uPoint(false);
  UPoint p(true);
  //  int counter = 0;
  result.StartBulkLoad();
  for( int i = 0; i < GetNoComponents(); i++ ){
    Get( i, uPoint );
/*
Definition of a new point unit p. The velocity is constant
at all times of the interval of the unit. This is exactly
the same as within operator ~speed~. The result is a vector
and can be represented as a upoint.

*/

    uPoint.UVelocity( p );
    if( p.IsDefined() ){
      result.Add( p );
//    counter++;
    }
  }
  result.EndBulkLoad( true );
}

void MPoint::MSpeed( MReal& result, const Geoid* geoid ) const
{
  result.Clear();
  if( !IsDefined() ){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );

  UPoint uPoint(false);
  UReal uReal(true);
  result.StartBulkLoad();
  bool valid = true;

  for( int i = 0; valid && (i < GetNoComponents()); i++ ){
    Get( i, uPoint );
    uPoint.USpeed( uReal, geoid );
    if( uReal.IsDefined() ){
      result.Add( uReal ); // append ureal to mreal
    } else {
      valid = false;
    }
  }
  result.EndBulkLoad( true );
  result.SetDefined( valid );
}


bool MPoint::Append(const MPoint& p, const bool autoresize /*=true*/){
  if(!IsDefined()){
     return false;
  }
  if(!p.IsDefined()){
     Clear();
     SetDefined(false);
     return false;
  }
  int size1 = this->GetNoComponents();
  int size2 = p.GetNoComponents();
  if(size1>0 && size2>0){
    // check whether p starts after the end of this
    UPoint u1;
    UPoint u2;
    this->Get(size1-1,u1);
    p.Get(0,u2);
    if((u1.timeInterval.end > u2.timeInterval.start) ||
       ( (u1.timeInterval.end == u2.timeInterval.start) &&
         (u1.timeInterval.rc  && u2.timeInterval.lc))){
      this->Clear();
      this->SetDefined(false);
      return false;
    }
  }
  UPoint up;
  UPoint u;
  if(size2>0){ // process the first unit of p
     if(autoresize){
        units.resize(size1+size2);
     }
     p.Get(0,up);
     this->MergeAdd(up);
  }
  StartBulkLoad();
  for(int i=1; i<size2; i++){
     p.Get(i,up);
     this->Add(up);
  }
  EndBulkLoad(false);
  return true;
}

void MPoint::Disturb(MPoint& result,
                     const double maxDerivation,
                     double maxDerivationPerStep){

   result.Clear();
   if(!IsDefined()){
        result.SetDefined(false);
        return;
   }
   result.SetDefined( true );
   assert(IsOrdered());
   int size = GetNoComponents();
   if(size>0){
      result.Resize(size);
   }
   if(maxDerivationPerStep>maxDerivation){
       maxDerivationPerStep=maxDerivation;
   }
   if(maxDerivationPerStep<=0){
      result.CopyFrom(this);
      return;
   }

  double errx = 2*maxDerivation * (double)rand()/(double)RAND_MAX  -
                maxDerivation;
  double erry = 2*maxDerivation * (double)rand()/(double)RAND_MAX  -
                maxDerivation;

   UPoint unit1;
   Point lastPoint;
   for(int i=0;i<size;i++){
      Get(i,unit1);
      Point p0(true,unit1.p0.GetX()+errx,unit1.p0.GetY()+erry);

      double dx = 2*maxDerivationPerStep * (double)rand()/(double)RAND_MAX  -
                  maxDerivationPerStep;
      double dy = 2*maxDerivationPerStep * (double)rand()/(double)RAND_MAX  -
                  maxDerivationPerStep;

      errx += dx;
      erry += dy;
      if(errx>maxDerivation){
        errx = maxDerivation;
      }
      if(errx < -maxDerivation){
        errx = -maxDerivation;
      }
      if(erry>maxDerivation){
        erry = maxDerivation;
      }
      if(erry < -maxDerivation){
        erry = -maxDerivation;
      }

      Point p1(true,unit1.p1.GetX()+errx,unit1.p1.GetY()+erry);
      UPoint unit(unit1.timeInterval, p0,p1);
      result.MergeAdd(unit);
   }
}

double MPoint::Length() const{
  assert( IsDefined() );
  if(!IsDefined()){
    return -1;
  }
  double res = 0;
  UPoint unit;
  int size = GetNoComponents();
  for(int i=0;i<size;i++){
     Get(i,unit);
     res += unit.p0.Distance(unit.p1);
  }
  return res;
}

double MPoint::Length(const Geoid& g, bool& valid) const{
  valid = IsDefined();
  if(!valid){
    return -1;
  }
  double res = 0;
  UPoint unit;
  int size = GetNoComponents();
  for(int i=0; (valid && (i<size)); i++){
    Get(i,unit);
    res += unit.p0.DistanceOrthodrome(unit.p1, g, valid);
  }
  return res;
}


void MPoint::Vertices(Points& result) const{
  result.Clear();
  if(!IsDefined()){
    result.SetDefined(false);
    return;
  }
  result.SetDefined( true );

  result.StartBulkLoad();
  UPoint unit;
  int size = units.Size();
  for(int i=0;i<size;i++){
    units.Get(i,&unit);
    Point p0(unit.p0);
    Point p1(unit.p1);
    result += p0;
    result += p1;
  }
  result.EndBulkLoad();
}

void MPoint::gk(const int &zone, MPoint& result) const{
  result.Clear();
  if( !IsDefined() || (zone < 0) || (zone > 119) ) {
     result.SetDefined(false);
     return;
  }
  result.SetDefined( true );

  result.StartBulkLoad();
  UPoint unit;
  WGSGK gk;
  gk.setMeridian(zone);

  int size = units.Size();
  for(int i=0;i<size;i++){
     units.Get(i,&unit);
     UPoint u(unit);
     if(!gk.project(unit.p0, u.p0) || !gk.project(unit.p1, u.p1)){
       // error detected
       result.EndBulkLoad();
       result.Clear();
       result.SetDefined(false);
       return;
     } else {
       result.MergeAdd(u);
     }
  }
  result.EndBulkLoad();
}

/*
Private helper function for the delay operator

*/
double* MPoint::MergePartitions(double* first, int firstSize, double* second,
                                int secondSize, int& count )
{
  double* res= new double[firstSize + secondSize ];
  count=0;
  int index1=0, index2=0;
  double candidate,last=-1;
  while(index1 < firstSize && index2< secondSize){
    if (first[index1] < second[index2])
      candidate= first[index1++];
    else if(first[index1] > second[index2])
      candidate= second[index2++];
    else {
      candidate= second[index2++];
      index1++;
    }
    if(!AlmostEqual(candidate , last))
      last= res[count++]= candidate;
  }
  while(index1< firstSize){
    if( !AlmostEqual((candidate = first[index1++]) , last))
      last=res[count++]=candidate;
  }
  while(index2<secondSize){
    if( !AlmostEqual((candidate = second[index2++]) , last))
      last=res[count++]=candidate;
  }
  return res;
}

/*
Private helper function for the delay operator

*/

int MPoint::IntervalRelation(Interval<Instant> &int_a_b,
                             Interval<Instant> &int_c_d  ) const
{
  Instant a= int_a_b.start;
  Instant b= int_a_b.end;
  Instant c= int_c_d.start;
  Instant d= int_c_d.end;
  assert(a < b && c < d );
/*
  The assertion will fail in case of numerical instability (i.e: rounding error)

*/
  if(b < c)  // a----b----c----d
    return 1;
  if(a > d)  // c----d----a----b
    return 2;
  if(b == c) // a----bc----d
    return 3;
  if(d == a) // c----da----b
    return 4;
  if(a < c && c < b && b < d) //a----c----b----d
    return 5;
  if(c < a && a < d && d < b) //c----a----d----b
    return 6;
  if(a==c && b==d) //ac----bd
    return 7;
  if(a==c && b< d) //ac----b----d
    return 8;
  if(a==c && d< b) //ac----d----b
    return 9;
  if(c< a && b==d) //c----a----bd
    return 10;
  if(a <c && b==d) //a----c----bd
    return 11;
  if(c <a && b <d) //c----a----b----d
    return 12;
  if(a <c && d <b) //a----c----d----b
    return 13;
  assert(false); //can not be other value
}

/*
Helper function for the delay operator

Only for use with linear units (a==0, r==false)!

*/
int AtValue(const UReal* unit, double val, Instant& inst,
            Interval<Instant>& intr,bool ignorelimits)
{
  assert( unit->IsDefined() );
  assert( AlmostEqual(unit->a,0) );
  assert( !unit->r );
  if(AlmostEqual(unit->b,0))
  {
    if(AlmostEqual(unit->c, val))
    {
      intr= unit->timeInterval;
      return 2; //result is the whole unit interval
    }
    else
    {
      return 0; //result is outside the unit interval
    }
  }
  inst= unit->timeInterval.start;
  double fraction=  (val - unit->c)/unit->b;
  Instant at(durationtype);
  at.ReadFrom(fraction);
  inst+= at;
  if( !ignorelimits && ((inst == unit->timeInterval.start &&
       !unit->timeInterval.lc)  || (inst == unit->timeInterval.end &&
       !unit->timeInterval.rc)))
    return 0;
  if(inst < unit->timeInterval.start  || inst > unit->timeInterval.end)
    return 0; //result is outside the unit interval
  return 1; // result is a certain time instant within the unit interval
}
/*
The following macros help make the code of the ~MPoint::DelayOperator~ more
readable. They are common code snippets that appear several times within the
operator. The ~\_startunit~ macro does the necessary variable settings for
starting a new delay unit. The ~\_endunit~ macro does the necessary variable
settings for closing a new delay unit. The ~\_createunit~ macro uses the
local variables to generate a delay unit and appends it to the result. The
~\_createunitpar~ macro, creates a delay unit from the parameters and appends
it to the result.

*/

#define _startunit(val , t) \
  delayValueAtUnitStartTime = val; \
  delayUnitStartTime = t; \
  atUnitStart = false; \
  if(debugme) cout<<"\n\t\tStartUnit ("<<val<<" @ "<< t.Print(cout)<<" )";

#define _endunit(val, t) \
  delayValueAtUnitEndTime=val; \
  delayUnitEndTime=t; \
  atUnitStart = true; \
  if(debugme) cout<<"\n\t\tEndUnit ("<<val<<" @ "<< t.Print(cout)<<" )";

#define _createunitpar(val1, t1, val2, t2) \
  intr.start=t1; intr.end=t2; \
  runit= new UReal(intr, val1 * 86400, val2 * 86400); \
  delayRes->Add(*runit); \
  delete runit; \
  if(debugme) cout<<"\n\t\tCreateUnit" ;

#define _createunit \
  intr.start= delayUnitStartTime; intr.end=delayUnitEndTime; \
  runit= new UReal(intr, delayValueAtUnitStartTime * 86400, \
      delayValueAtUnitEndTime * 86400); \
  delayRes->Add(*runit); \
  if(debugme) cout<<"\n\t\tCreateIntermediateUnit (" \
      << runit->Print(cout)<<" )"; \
  delete runit;

MReal* MPoint::DelayOperator(const MPoint* actual, const Geoid* geoid)
{
  bool debugme=false;
  if(!this->IsDefined() || !actual->IsDefined() ||
     (geoid && !geoid->IsDefined()))
    { MReal* res= new MReal(0);  res->SetDefined(false); return res;}
  if(this->GetNoComponents()<1 || actual->GetNoComponents()<1)
    return new MReal(0);

  double* partitionActual=new double[actual->GetNoComponents()+1];
  double* partitionSchedule=new double[this->GetNoComponents()+1];
  MReal* DTActual= actual->DistanceTraversed(partitionActual, geoid);
  MReal* DTSchedule= this->DistanceTraversed(partitionSchedule, geoid);
  if(!DTActual->IsDefined() || !DTSchedule->IsDefined())
  { MReal* res= new MReal(0);  res->SetDefined(false); return res;}

  int DTActualSize= DTActual->GetNoComponents();
  int DTScheduleSize= DTSchedule->GetNoComponents();
  int partitionSize;
  double* partition=
        MergePartitions(partitionActual,DTActualSize+1,partitionSchedule,
                        DTScheduleSize+1,partitionSize);

  if(debugme)
  {
    cout.flush();
    cout<<"\n ActualPartition: ";
    for(int i=0; i<= DTActualSize; i++)
      cout<<partitionActual[i]<<"  ";
    cout<<"\n SchedulePartition: ";
    for(int i=0; i<= DTScheduleSize; i++)
      cout<<partitionSchedule[i]<<"  ";
    cout<<"\n MergedPartition: ";
    for(int i=0; i< partitionSize; i++)
      cout<<partition[i]<<"  ";
    cout.flush();
  }

  UReal actualScanUnit, scheduleScanUnit;
  UReal* runit;
  int actualScanIndex=0, scheduleScanIndex=0;
  DTActual->Get(actualScanIndex, actualScanUnit);
  DTSchedule->Get(scheduleScanIndex,scheduleScanUnit);

  MReal* delayRes= new MReal(partitionSize);
  Intime<CcReal> temp;
  DTActual->Initial(temp);
  Instant delayUnitStartTime(temp.instant);
  Instant delayUnitEndTime(instanttype);
  Instant scheduledTime(instanttype),actualTime(instanttype);
  Interval<Instant> scheduledInterval, actualInterval, intr;
  double delayValueAtUnitStartTime = 0.0, delayValueAtUnitEndTime = 0.0;
  bool atUnitStart=true, isInstantActual=true, isInstantSchedule=true;
  double distVal = 0.0;
  int test = 0;
  intr.lc=true;
  intr.rc=false;
  for(int i=0; i<partitionSize; i++)
  {
    distVal=partition[i];
/*
The coming steps assumes that DistanceTraversed return an
MReal satisfying the minimal presentation condition

*/
    if(scheduleScanIndex < DTScheduleSize -1)
      test = AtValue(&scheduleScanUnit, distVal, scheduledTime,
                      scheduledInterval,false);
    else
      test = AtValue(&scheduleScanUnit, distVal, scheduledTime,
                      scheduledInterval,true);
    while( test==0 )
    {
      scheduleScanIndex++;
      assert(scheduleScanIndex < DTSchedule->GetNoComponents());
      DTSchedule->Get(scheduleScanIndex,scheduleScanUnit);
      if(scheduleScanIndex < DTScheduleSize -1)
        test = AtValue(&scheduleScanUnit, distVal, scheduledTime,
                        scheduledInterval,false);
      else
        test = AtValue(&scheduleScanUnit, distVal, scheduledTime,
                        scheduledInterval,true);

    }
    isInstantSchedule = (test==1)? true : false;
    if(actualScanIndex < DTActualSize -1)
      test = AtValue(&actualScanUnit, distVal, actualTime,
                      actualInterval,false);
    else
      test = AtValue(&actualScanUnit, distVal, actualTime,
                      actualInterval,true);
    while( test==0 )
    {
      actualScanIndex++;
      assert(actualScanIndex < DTActual->GetNoComponents());
      DTActual->Get(actualScanIndex,actualScanUnit);
      if(actualScanIndex < DTActualSize -1)
        test = AtValue(&actualScanUnit, distVal, actualTime,
                        actualInterval,false);
      else
        test = AtValue(&actualScanUnit, distVal, actualTime,
                        actualInterval,true);

    }
    isInstantActual = (test==1)? true : false;

    if(debugme)
    {
      cout<<endl;
      cout<<"At iteration: "<<i<<" traversed distance: "<< distVal;
      cout<<"\n\tActual traversed this distance at: ";
      if(isInstantActual) actualTime.Print(cout);
      else actualInterval.Print(cout);
      cout<<"\n\tSchedule traversed this distance at: ";
      if(isInstantSchedule) scheduledTime.Print(cout);
      else scheduledInterval.Print(cout);
      cout<<endl<<"\tActions taken:";
    }

    if(isInstantActual && isInstantSchedule)
    {
      if(atUnitStart)
      {
        _startunit( (actualTime - scheduledTime).ToDouble(),
                     actualTime );
      }
      else
      {
        _endunit( (actualTime - scheduledTime).ToDouble(), actualTime );
        _createunit;
        _startunit(delayValueAtUnitEndTime, delayUnitEndTime);
      }
    }
    else if (isInstantActual && !isInstantSchedule)
    {
      if(atUnitStart)
/*
Can happen only if the schedule started with immobile units (distVal==0)

*/
      {
        assert(distVal==0);
        _startunit( (actualTime - scheduledInterval.end).ToDouble() ,
                     actualTime);
      }
      else
      {
        _endunit((actualTime - scheduledInterval.start).ToDouble() ,
                  actualTime);
        _createunit;
        _startunit( (actualTime - scheduledInterval.end).ToDouble() ,
                     actualTime);
      }
    }
    else if (!isInstantActual && isInstantSchedule)
    {
      if(atUnitStart)
/*
Can happen only if the actual started with immobile units

*/
      {
        assert(distVal==0);
                //add a delay unit corresponding to the immobile unit
        _createunitpar(
            (actualInterval.start - scheduledTime).ToDouble(),
        actualInterval.start,
        (actualInterval.end - scheduledTime).ToDouble(),
        actualInterval.end);
        //start the next unit
        _startunit((actualInterval.end - scheduledTime).ToDouble(),
                    actualInterval.end);
      }
      else
      {
        //close the current unit
        _endunit((actualInterval.start - scheduledTime).ToDouble(),
                  actualInterval.start);
        _createunit;
                //add a delay unit corresponding to the immobile unit
        _createunitpar(
            (actualInterval.start - scheduledTime).ToDouble(),
        actualInterval.start,
        (actualInterval.end - scheduledTime).ToDouble(),
        actualInterval.end);
        //start the next unit
        _startunit((actualInterval.end - scheduledTime).ToDouble(),
                    actualInterval.end);
      }
    }
    else if(!isInstantActual && !isInstantSchedule)
    {
      int intervalRelation= IntervalRelation(actualInterval,
                                             scheduledInterval);
      Instant a(actualInterval.start), b(actualInterval.end);
      Instant c(scheduledInterval.start), d(scheduledInterval.end);
      switch(intervalRelation)
      {
        case 1:
        case 3:
        {
          if(!atUnitStart) {_endunit((a-c).ToDouble() ,a); _createunit;}
          _createunitpar((a-c).ToDouble(),a,(b-c).ToDouble(),b);
          _startunit((b-d).ToDouble(),b);
          break;
        }
        case 2:
        case 4:
        {
          if(!atUnitStart) {_endunit((a-c).ToDouble() ,a); _createunit;}
          _createunitpar((a-d).ToDouble(),a,(b-d).ToDouble(),b);
          _startunit((b-d).ToDouble(),b);
          break;
        }
        case 5:
        {
          if(!atUnitStart) {_endunit((a-c).ToDouble() ,a); _createunit;}
          _createunitpar((a-c).ToDouble(), a, 0, c);
          _createunitpar(0, c, 0, b);
          _startunit((b-d).ToDouble(),b);
          break;
        }
        case 6:
        {
          if(!atUnitStart) {_endunit((a-c).ToDouble() ,a); _createunit;}
          _createunitpar(0, a, 0, d);
          _createunitpar(0, d, (b-d).ToDouble(), b);
          _startunit((b-d).ToDouble(),b);
          break;
        }
        case 7:
        {
          if(!atUnitStart) {_endunit((a-c).ToDouble() ,a); _createunit;}
          _createunitpar((a-c).ToDouble(),a,(b-d).ToDouble(),b);
          _startunit((b-d).ToDouble(),b);
          break;
        }
        case 8:
        {
          if(!atUnitStart) {_endunit((a-c).ToDouble() ,a); _createunit;}
          _createunitpar((a-c).ToDouble(), a, 0, b);
          _startunit((b-d).ToDouble(),b);
          break;
        }
        case 9:
        {
          if(!atUnitStart) {_endunit((a-c).ToDouble() ,a); _createunit;}
          _createunitpar((a-c).ToDouble(), a, 0, d);
          _createunitpar(0, d, (b-d).ToDouble(), b);
          _startunit((b-d).ToDouble(),b);
          break;
        }
        case 10:
        {
          if(!atUnitStart) {_endunit((a-c).ToDouble() ,a); _createunit;}
          _createunitpar(0, a, (b-d).ToDouble(),b);
          _startunit((b-d).ToDouble(),b);
          break;
        }
        case 11:
        {
          if(!atUnitStart) {_endunit((a-c).ToDouble() ,a); _createunit;}
          _createunitpar((a-c).ToDouble(), a, 0, c);
          _createunitpar(0, c, (b-d).ToDouble(), b);
          _startunit((b-d).ToDouble(),b);
          break;
        }
        case 12:
        {
          if(!atUnitStart) {_endunit((a-c).ToDouble() ,a); _createunit;}
          _createunitpar(0, a, 0 ,b);
          _startunit((b-d).ToDouble(),b);
          break;
        }
        case 13:
        {
          if(!atUnitStart) {_endunit((a-c).ToDouble() ,a); _createunit;}
          _createunitpar((a-c).ToDouble(), a, 0, c);
          _createunitpar(0, c, 0, d);
          _createunitpar(0, d, (b-d).ToDouble(), b);
          _startunit((b-d).ToDouble(),b);
          break;
        }
        default:
          assert(false);
      }
    }
  }
  delete[] partitionActual;
  delete[] partitionSchedule;
  delete DTActual;
  delete DTSchedule;
  delete[] partition;
  return delayRes;
}

MReal* MPoint::DistanceTraversed( const Geoid* geoid ) const
{
  if( !IsDefined() || (geoid && !geoid->IsDefined()) ) {
    MReal* res = new MReal(0);
    res->SetDefined( false );
    return res;
  }
  double * p= new double[GetNoComponents()+1];
  MReal* res= DistanceTraversed(p, geoid);
  delete[] p;
  return res;
}

MReal* MPoint::DistanceTraversed(double* partition, const Geoid* geoid ) const
{
  bool debugme= false;
  bool ok = true; // flag for DistanceOrthodrome
  UPoint uPoint;
  Point last;
  MReal* dist= new MReal(GetNoComponents());
  double dist1=0, dist2=0, unitstart=0, unitend=0,lastslope=0,curslope=0;
  Interval<Instant> interval;
  UReal* unit=0;
  int partitionIndex=0;

/*
The movement must be continuous in time (i.e. without intervals of undefined),
otherwise, the result is undefined

*/
  Periods defTime( 0 );
  DefTime( defTime );
  if(defTime.GetNoComponents()>1 )
  {
    dist->Clear();
    dist->SetDefined(false);
    return dist;
  }

  try
  {
    Get( 0, uPoint );
    dist1=0;
    dist2= uPoint.p0.Distance(uPoint.p1);
    last= uPoint.p1;
    lastslope= (dist2-dist1)/((uPoint.timeInterval.end -
        uPoint.timeInterval.start).millisecondsToNull() / 1000.0);
    unitstart = dist1;
    unitend= dist2;
    interval=uPoint.timeInterval;

    for( int i = 1; i < GetNoComponents(); i++ )
    {
      Get( i, uPoint );
      if(last != uPoint.p0)
        throw(1); // The trajectory is not continuous
      dist1= dist2;
      dist2= dist1 + (geoid?uPoint.p0.DistanceOrthodrome(uPoint.p1,*geoid,ok)
                           :uPoint.p0.Distance(uPoint.p1));
      if(!ok){ // found invalid geographic coordinates
        cout << "\nFound invalid geographic coordinates: uPoint.p0="
             << uPoint.p0 << ", uPoint.p1=" << uPoint.p1 << "." << endl
             << "Returning undefined result!" << endl;
        dist->SetDefined(false);
        return dist;
      }
      last= uPoint.p1;


      //Assure minimal representation
      curslope= (dist2-dist1)/((uPoint.timeInterval.end -
          uPoint.timeInterval.start).millisecondsToNull()/ 1000.0);
      if(debugme)
      {
        cout.flush();
        cout<<"\nlastSlope: " <<lastslope << "    curSlope:"<<curslope;
        cout.flush();
      }
      if(curslope != lastslope)
      {
        unit = new UReal(interval, unitstart, unitend);
        if(debugme)
        {
          cout.flush();
          cout<<"\nAdding new UReal";
//          cout<<"\n\tAt time "<<
//                    unit->timeInterval.start.GetAllMilliSeconds();
//                  cout<<" the distance traversed is "<< unit->Min(dummy);
//          cout<<"\n\tAt time "<<
//                    unit->timeInterval.end.GetAllMilliSeconds();
//                  cout<<" the distance traversed is "<< unit->Max(dummy);
          cout.flush();
        }
        dist->Add(*unit);
        delete unit;
        partition[partitionIndex]= unitstart;
        partitionIndex++;
        unitstart = dist1;
        unitend= dist2;
        interval=uPoint.timeInterval;
        lastslope= curslope;
      }
      else
      {
        interval.end= uPoint.timeInterval.end;
        unitend= dist2;
      }
    }
    unit = new UReal(interval, unitstart, unitend);
    dist->Add(*unit);
    delete unit;

    partition[partitionIndex]= unitstart;
    partitionIndex++;
    partition[partitionIndex]= unitend;
    dist->TrimToSize();
    return dist;
  }
  catch(int i)
  {
    dist->TrimToSize();
    dist->SetDefined(false);
    return dist;
  }
}

void MPoint::AtRegion(const Region *r, MPoint &result) const {
  result.Clear();
  if(!IsDefined() || !r->IsDefined()){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  if(IsEmpty() || r->IsEmpty()) {
    return;
  }
  // check for intersection of total MBRs
  Rectangle<2u> rMBR = r->BoundingBox();
  if(!rMBR.Intersects(BoundingBoxSpatial())) {
    return;
  }
  // iterate through all units
  UPoint uPoint;
  vector<UPoint> uResultVector;
  for( int i = 1; i < GetNoComponents(); i++ ){
    Get( i, uPoint );
    if(!uPoint.AtRegion(r, uResultVector)) {
      cerr << __PRETTY_FUNCTION__ << " WARNING: no result for UPoint (" << i
           << "): uPoint = " << uPoint << endl;
      continue;
    }
    for(unsigned int j=0; j<uResultVector.size(); j++) {
      result.MergeAdd(uResultVector[j]);
    }
  }
  return;
}

void MPoint::AtRect(const Rectangle<2>& rect, MPoint& result) const{
  result.Clear();
  if(!IsDefined() || !rect.IsDefined()){
    result.SetDefined(false);
    return;
  }
  Rectangle<2u> bbox = BoundingBoxSpatial();
  if(!bbox.Intersects(rect)){
     return;  // dijoint, return empty 
  }
  if(rect.Contains(bbox)){
     result.CopyFrom(this);
     return;
  }
  // Bounding boxes overlap, check units
  UPoint src;
  UPoint dest(false);
  result.StartBulkLoad();
  for(int i=0;i<GetNoComponents();i++){
    Get(i,src);
    src.At(rect,dest);
    if(dest.IsDefined()){
       assert(dest.timeInterval.start.GetType()==datetime::instanttype);
       assert(dest.timeInterval.end.GetType()==datetime::instanttype);

       result.Add(dest);
    }
  }
  result.EndBulkLoad(false);

}

const bool Periods::Contains(const SecInterval& iv) const {
  if (!IsDefined() || !iv.IsDefined()) {
    return false;
  }
  int startIvPos = GetIndexOf(iv.start, true);
  if (startIvPos < 0) {
    return false;
  }
  Interval<Instant> startIv;
  Get(startIvPos, startIv);
  if (startIv.lc < iv.lc) {
    return false;
  }
  return startIv.Contains(iv);
}

const bool Periods::Contains(const Periods& per) const {
  RefinementStream<Periods, Periods, Interval<Instant>, Interval<Instant> >
                  rs(this, &per);
  int p1, p2;
  Interval<Instant> iv;
  while (rs.getNext(iv, p1, p2)) {
    if(p1 < 0) {
      return false;
    }
    if(rs.finished2()) {
      return true;
    }
  }
  return true;
}

/*
4 Type Constructors

4.1 Type Constructor ~rint~

This type constructor implements the carrier set for ~range(int)~.

4.1.1 List Representation

The list representation of a ~rint~ is

----    ( (i1b i1e lc1 rc1) (i2b i2e lc2 rc2) ... (inb ine lcn rcn) )
----

For example:

----    ( (1 5 TRUE FALSE) (6 9 FALSE FALSE) (11 11 TRUE TRUE) )
----

4.1.2 function Describing the Signature of the Type Constructor

*/
ListExpr
RangeIntProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,
  "lci means left closed interval, rci respectively right closed interval,"
    " e.g. (0 1 TRUE FALSE) means the range [0, 1[");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> RANGE"),
                             nl->StringAtom("(rint) "),
        nl->StringAtom("((b1 e1 lci rci) ... (bn en lci rci))"),
        nl->StringAtom("((0 1 TRUE FALSE) (2 5 TRUE TRUE))"),
                             remarkslist)));
}

/*
4.1.3 Kind Checking Function

*/
bool
CheckRangeInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, RInt::BasicType() ));
}

/*
4.1.4 Creation of the type constructor ~rint~

*/
TypeConstructor rangeint(
        RInt::BasicType(),             //name
        RangeIntProperty,   //property function describing signature
        OutRange<CcInt, OutCcInt>,
        InRange<CcInt, InCcInt>,                 //Out and In functions
        0,            0,  //SaveToList and RestoreFromList functions
        CreateRange<CcInt>,DeleteRange<CcInt>,   //object creation and deletion
        OpenRange<CcInt>,  SaveRange<CcInt>,     // object open and save
        CloseRange<CcInt>, CloneRange<CcInt>,    //object close and clone
        CastRange<CcInt>,                        //cast function
        SizeOfRange<CcInt>,                      //sizeof function
        CheckRangeInt );                         //kind checking function


/*
4.2 Type Constructor ~rreal~

This type constructor implements the carrier set for ~range(real)~.

4.2.1 List Representation

The list representation of a ~rreal~ is

----    ( (r1b r1e lc1 rc1) (r2b r2e lc2 rc2) ... (rnb rne lcn rcn) )
----

For example:

----    ( (1.01 5 TRUE FALSE) (6.37 9.9 FALSE FALSE) (11.93 11.99 TRUE TRUE) )
----

4.2.2 function Describing the Signature of the Type Constructor

*/
ListExpr
RangeRealProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,
  "lci means left closed interval, rci respectively right closed interval,"
  " e.g. (0.5 1.1 TRUE FALSE) means the range [0.5, 1.1[");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> RANGE"),
                             nl->StringAtom("(rreal) "),
        nl->StringAtom("((b1 e1 lci rci) ... (bn en lci rci))"),
        nl->StringAtom("((0.5 1.1 TRUE FALSE) (2 5.04 TRUE TRUE))"),
                             remarkslist)));
}

/*
4.2.3 Kind Checking Function

*/
bool
CheckRangeReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, RReal::BasicType() ));
}

/*
4.2.4 Creation of the type constructor ~rreal~

*/
TypeConstructor rangereal(
        RReal::BasicType(),              //name
        RangeRealProperty,   //property function describing signature
        OutRange<CcReal, OutCcReal>,
        InRange<CcReal, InCcReal>,   //Out and In functions
        0,       0,      //SaveToList and RestoreFromList functions
        CreateRange<CcReal>,DeleteRange<CcReal>,  //object creation and deletion
        OpenRange<CcReal>,SaveRange<CcReal>,  // object open and save
        CloseRange<CcReal>,CloneRange<CcReal>,  //object close and clone
        CastRange<CcReal>,  //cast function
        SizeOfRange<CcReal>,                    //sizeof function
        CheckRangeReal );               //kind checking function

/*
4.3 Type Constructor ~periods~

This type constructor implements the carrier set for ~range(instant)~, which is
called ~periods~.

4.3.1 List Representation

The list representation of a ~periods~ is

----    ( (i1b i1e lc1 rc1) (i2b i2e lc2 rc2) ... (inb ine lcn rcn) )
----

For example:

----    ( ( (instant 1.01)  (instant 5)     TRUE  FALSE)
          ( (instant 6.37)  (instant 9.9)   FALSE FALSE)
          ( (instant 11.93) (instant 11.99) TRUE  TRUE) )
----

4.3.2 function Describing the Signature of the Type Constructor

*/
ListExpr
PeriodsProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,
  "lci means left closed interval, rci respectively right closed interval,"
" e.g. ((instant 0.5) (instant 1.1) TRUE FALSE) means the interval [0.5, 1.1[");

  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> RANGE"),
                             nl->StringAtom("(periods) "),
           nl->StringAtom("( (b1 e1 lci rci) ... (bn en lci rci) )"),
           nl->StringAtom("((i1 i2 TRUE FALSE) ...)"))));
}

/*
4.3.3 Kind Checking Function

*/
bool
CheckPeriods( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, Periods::BasicType() ));
}

/*
4.3.4 Creation of the type constructor ~periods~

*/
TypeConstructor periods(
        Periods::BasicType(),         //name
        PeriodsProperty,  //property function describing signature
        OutRange<Instant, OutDateTime>,
        InRange<Instant, InInstant>, //Out and In functions
        0,        0,        //SaveToList and RestoreFromList functions
        CreateRange<Instant>, DeleteRange<Instant>,
   //object creation and deletion
        OpenRange<Instant>,   SaveRange<Instant>,       // object open and save
        CloseRange<Instant>,  CloneRange<Instant>,      //object close and clone
        CastRange<Instant>,                          //cast function
        SizeOfRange<Instant>,                         //sizeof function
        CheckPeriods );    //kind checking function

/*
4.4 Type Constructor ~ibool~

Type ~ibool~ represents an (instant, value)-pair of booleans.

4.4.1 List Representation

The list representation of an ~ibool~ is

----    ( t bool-value )
----

For example:

----    ( (instant 1.0) FALSE )
----

4.4.2 function Describing the Signature of the Type Constructor

*/
ListExpr
IntimeBoolProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(ibool) "),
                             nl->StringAtom("(instant bool-value) "),
                             nl->StringAtom("((instant 0.5) FALSE)"))));
}

/*
4.4.3 Kind Checking Function

*/
bool
CheckIntimeBool( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, IBool::BasicType() ));
}

/*
4.4.4 Creation of the type constructor ~ibool~

*/
TypeConstructor intimebool(
        IBool::BasicType(),    //name
        IntimeBoolProperty,             //property function describing signature
        OutIntime<CcBool, OutCcBool>,
        InIntime<CcBool, InCcBool>,     //Out and In functions
        0,
        0,     //SaveToList and RestoreFromList functions
        CreateIntime<CcBool>,
        DeleteIntime<CcBool>,           //object creation and deletion
        OpenAttribute<Intime<CcBool> >,
        SaveAttribute<Intime<CcBool> >,  // object open and save
        CloseIntime<CcBool>,
        CloneIntime<CcBool>,            //object close and clone
        CastIntime<CcBool>,             //cast function
        SizeOfIntime<CcBool>,           //sizeof function
        CheckIntimeBool );              //kind checking function

/*
4.4 Type Constructor ~iint~

Type ~iint~ represents an (instant, value)-pair of integers.

4.4.1 List Representation

The list representation of an ~iint~ is

----    ( t int-value )
----

For example:

----    ( (instant 1.0) 5 )
----

4.4.2 function Describing the Signature of the Type Constructor

*/
ListExpr
IntimeIntProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(iint) "),
                             nl->StringAtom("(instant int-value) "),
                             nl->StringAtom("((instant 0.5) 1)"))));
}

/*
4.4.3 Kind Checking Function

*/
bool
CheckIntimeInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, IInt::BasicType() ));
}

/*
4.4.4 Creation of the type constructor ~iint~

*/
TypeConstructor intimeint(
        IInt::BasicType(),                   //name
        IntimeIntProperty,             //property function describing signature
        OutIntime<CcInt, OutCcInt>,
        InIntime<CcInt, InCcInt>,      //Out and In functions
        0,
        0,    //SaveToList and RestoreFromList functions
        CreateIntime<CcInt>,
        DeleteIntime<CcInt>,           //object creation and deletion
        OpenAttribute<Intime<CcInt> >,
        SaveAttribute<Intime<CcInt> >,  // object open and save
        CloseIntime<CcInt>,
        CloneIntime<CcInt>,            //object close and clone
        CastIntime<CcInt>,             //cast function
        SizeOfIntime<CcInt>,           //sizeof function
        CheckIntimeInt );              //kind checking function

/*
4.5 Type Constructor ~ireal~

Type ~ireal~ represents an (instant, value)-pair of reals.

4.5.1 List Representation

The list representation of an ~ireal~ is

----    ( t real-value )
----

For example:

----    ( (instant 1.0) 5.0 )
----

4.5.2 function Describing the Signature of the Type Constructor

*/
ListExpr
IntimeRealProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(ireal) "),
                             nl->StringAtom("(instant real-value)"),
                             nl->StringAtom("((instant 0.5) 1.0)"))));
}

/*
4.5.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckIntimeReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, IReal::BasicType() ));
}

/*
4.5.4 Creation of the type constructor ~ireal~

*/
TypeConstructor intimereal(
        IReal::BasicType(),              //name
        IntimeRealProperty,   //property function describing signature
        OutIntime<CcReal, OutCcReal>,
        InIntime<CcReal, InCcReal>,        //Out and In functions
        0,
        0,    //SaveToList and RestoreFromList functions
        CreateIntime<CcReal>,
        DeleteIntime<CcReal>,              //object creation and deletion
        OpenAttribute<Intime<CcReal> >,
        SaveAttribute<Intime<CcReal> >,  // object open and save
        CloseIntime<CcReal>,
        CloneIntime<CcReal>,               //object close and clone
        CastIntime<CcReal>,                //cast function
        SizeOfIntime<CcReal>,              //sizeof function
        CheckIntimeReal );                 //kind checking function

/*
4.6 Type Constructor ~ipoint~

Type ~ipoint~ represents an (instant, value)-pair of points.

4.6.1 List Representation

The list representation of an ~ipoint~ is

----    ( t point-value )
----

For example:

----    ( (instant 1.0) (5.0 0.0) )
----

4.6.2 function Describing the Signature of the Type Constructor

*/
ListExpr
IntimePointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(ipoint) "),
                             nl->StringAtom("(instant point-value)"),
                             nl->StringAtom("((instant 0.5) (1.0 2.0))"))));
}

/*
4.6.3 Kind Checking Function

*/
bool
CheckIntimePoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, IPoint::BasicType() ));
}

/*
4.6.4 Creation of the type constructor ~ipoint~

*/
TypeConstructor intimepoint(
        IPoint::BasicType(),                    //name
        IntimePointProperty,  //property function describing signature
        OutIntime<Point, OutPoint>,
        InIntime<Point, InPoint>,         //Out and In functions
        0,
        0,       //SaveToList and RestoreFromList functions
        CreateIntime<Point>,
        DeleteIntime<Point>,              //object creation and deletion
        OpenAttribute<Intime<Point> >,
        SaveAttribute<Intime<Point> >,  // object open and save
        CloseIntime<Point>,
        CloneIntime<Point>,               //object close and clone
        CastIntime<Point>,                //cast function
        SizeOfIntime<Point>,              //sizeof function
        CheckIntimePoint );               //kind checking function

/*
4.7 Type Constructor ~ubool~

Type ~ubool~ represents an (tinterval, boolvalue)-pair.

4.7.1 List Representation

The list representation of an ~ubool~ is

----    ( timeinterval bool-value )
----

For example:

----    ( ((instant 6.37)  (instant 9.9)   TRUE FALSE) TRUE )
----

4.7.2 function Describing the Signature of the Type Constructor

*/
ListExpr
UBoolProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("(ubool) "),
                             nl->StringAtom("(timeInterval bool) "),
                             nl->StringAtom("((i1 i2 FALSE FALSE) TRUE)"))));
}

/*
4.7.3 Kind Checking Function

*/
bool
CheckUBool( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, UBool::BasicType() ));
}

/*
4.7.4 Creation of the type constructor ~ubool~

*/
TypeConstructor unitbool(
        UBool::BasicType(),   //name
        UBoolProperty,    //property function describing signature
        OutConstTemporalUnit<CcBool, OutCcBool>,
        InConstTemporalUnit<CcBool, InCcBool>,    //Out and In functions
        0,     0,  //SaveToList and RestoreFromList functions
        CreateConstTemporalUnit<CcBool>,
        DeleteConstTemporalUnit<CcBool>,          //object creation and deletion
        OpenAttribute<UBool>,
        SaveAttribute<UBool>,  // object open and save
        CloseConstTemporalUnit<CcBool>,
        CloneConstTemporalUnit<CcBool>,           //object close and clone
        CastConstTemporalUnit<CcBool>,            //cast function
        SizeOfConstTemporalUnit<CcBool>,          //sizeof function
        CheckUBool );                             //kind checking function

/*
4.7 Type Constructor ~uint~

Type ~uint~ represents an (tinterval, intvalue)-pair.

4.7.1 List Representation

The list representation of an ~uint~ is

----    ( timeinterval int-value )
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   5 )
----

4.7.2 function Describing the Signature of the Type Constructor

*/
ListExpr
UIntProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("(uint) "),
                             nl->StringAtom("(timeInterval int) "),
                             nl->StringAtom("((i1 i2 FALSE FALSE) 1)"))));
}

/*
4.7.3 Kind Checking Function

*/
bool
CheckUInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, UInt::BasicType() ));
}

/*
4.7.4 Creation of the type constructor ~uint~

*/
TypeConstructor unitint(
        UInt::BasicType(),     //name
        UIntProperty, //property function describing signature
        OutConstTemporalUnit<CcInt, OutCcInt>,
        InConstTemporalUnit<CcInt, InCcInt>, //Out and In functions
        0,                      0,//SaveToList and RestoreFromList functions
        CreateConstTemporalUnit<CcInt>,
        DeleteConstTemporalUnit<CcInt>, //object creation and deletion
        OpenAttribute<UInt>,
        SaveAttribute<UInt>,  // object open and save
        CloseConstTemporalUnit<CcInt>,
        CloneConstTemporalUnit<CcInt>, //object close and clone
        CastConstTemporalUnit<CcInt>,       //cast function
        SizeOfConstTemporalUnit<CcInt>, //sizeof function
        CheckUInt );                    //kind checking function

/*
4.8 Type Constructor ~ureal~

Type ~ureal~ represents an (tinterval, (a, b, c, r))-pair, where
 a, b, c are real numbers, r is a boolean flag indicating which
function to use.

11.1 List Representation

The list representation of an ~ureal~ is

----    ( timeinterval (a b c r) )
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   (1.0 2.3 4.1 TRUE) )
----

4.8.2 Function Describing the Signature of the Type Constructor

*/
ListExpr
URealProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("("+UReal::BasicType()+") "),
         nl->StringAtom("( timeInterval (real1 real2 real3 bool)) "),
         nl->StringAtom("((i1 i2 TRUE FALSE) (1.0 2.2 2.5 TRUE))"))));
}

/*
4.8.3 Kind Checking Function

*/
bool
CheckUReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, UReal::BasicType() ));
}

/*
4.8.4 ~Out~-function

*/
ListExpr OutUReal( ListExpr typeInfo, Word value )
{
  UReal* ureal = (UReal*)(value.addr);

  if ( !ureal->IsDefined() )
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
  else
    {

      ListExpr timeintervalList = nl->FourElemList(
             OutDateTime( nl->TheEmptyList(),
             SetWord(&ureal->timeInterval.start) ),
             OutDateTime( nl->TheEmptyList(),
                          SetWord(&ureal->timeInterval.end) ),
             nl->BoolAtom( ureal->timeInterval.lc ),
             nl->BoolAtom( ureal->timeInterval.rc));

      ListExpr realfunList = nl->FourElemList(
             nl->RealAtom( ureal->a),
             nl->RealAtom( ureal->b),
             nl->RealAtom( ureal->c ),
             nl->BoolAtom( ureal->r));

      return nl->TwoElemList(timeintervalList, realfunList );
    }
}

/*
4.8.5 ~In~-function

The Nested list form is like this:

----
       ( ( 6.37 9.9 TRUE FALSE)   (1.0 2.3 4.1 TRUE) )
or:    undef

----

*/
Word InUReal( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
  string errmsg;
  correct = true;
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr first = nl->First( instance );
    if( nl->ListLength( first ) == 4 &&
      nl->IsAtom( nl->Third( first ) ) &&
      nl->AtomType( nl->Third( first ) ) == BoolType &&
      nl->IsAtom( nl->Fourth( first ) ) &&
      nl->AtomType( nl->Fourth( first ) ) == BoolType )
    {
      Instant *start = (Instant *)InInstant( nl->TheEmptyList(),
       nl->First( first ),
        errorPos, errorInfo, correct ).addr;
      if( !correct || !start->IsDefined() )
      {
        errmsg = "InUReal(): Error in first instant (Must be defined!).";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
       nl->Second( first ),
                                           errorPos, errorInfo, correct ).addr;
      if( !correct || !end->IsDefined() )
      {
        errmsg = "InUReal(): Error in second instant (Must be defined!).";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        delete end;
        return SetWord( Address(0) );
      }

      Interval<Instant> tinterval( *start, *end,
                                   nl->BoolValue( nl->Third( first ) ),
                                   nl->BoolValue( nl->Fourth( first ) ) );
      delete start;
      delete end;

      correct = tinterval.IsValid();
      if ( !correct )
        {
          errmsg = "InUReal(): Non valid time interval.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
        }

      ListExpr second = nl->Second( instance );

      if( nl->ListLength( second ) == 4 &&
          nl->IsAtom( nl->First( second ) ) &&
          nl->AtomType( nl->First( second ) ) == RealType &&
          nl->IsAtom( nl->Second( second ) ) &&
          nl->AtomType( nl->Second( second ) ) == RealType &&
          nl->IsAtom( nl->Third( second ) ) &&
          nl->AtomType( nl->Third( second ) ) == RealType &&
          nl->IsAtom( nl->Fourth( second ) ) &&
          nl->AtomType( nl->Fourth( second ) ) == BoolType )
      {
        UReal *ureal =
          new UReal( tinterval,
                     nl->RealValue( nl->First( second ) ),
                     nl->RealValue( nl->Second( second ) ),
                     nl->RealValue( nl->Third( second ) ),
                     nl->BoolValue( nl->Fourth( second ) ) );

        if( ureal->IsValid() )
        {
          correct = true;
          return SetWord( ureal );
        }
        delete ureal;
      }
    }
  }
  else if ( listutils::isSymbolUndefined(instance) )
    {
      UReal *ureal = new UReal();
      ureal->SetDefined(false);
      ureal->timeInterval=
        Interval<DateTime>(DateTime(instanttype),
                           DateTime(instanttype),true,true);
      correct = ureal->timeInterval.IsValid();
      if ( correct )
        return (SetWord( ureal ));
    }
  errmsg = "InUReal(): Non valid representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  correct = false;
  return SetWord( Address(0) );
}

/*
4.8.6 ~Create~-function

*/
Word CreateUReal( const ListExpr typeInfo )
{
  return (SetWord( new UReal(false) ));
}

/*
4.8.7 ~Delete~-function

*/
void DeleteUReal( const ListExpr typeInfo, Word& w )
{
  delete (UReal *)w.addr;
  w.addr = 0;
}

/*
4.8.8 ~Close~-function

*/
void CloseUReal( const ListExpr typeInfo, Word& w )
{
  delete (UReal *)w.addr;
  w.addr = 0;
}

/*
4.8.9 ~Clone~-function

*/
Word CloneUReal( const ListExpr typeInfo, const Word& w )
{
  UReal *ureal = (UReal *)w.addr;
  return SetWord( new UReal( *ureal ) );
}

/*
4.8.10 ~Sizeof~-function

*/
int SizeOfUReal()
{
  return sizeof(UReal);
}

/*
4.8.11 ~Cast~-function

*/
void* CastUReal(void* addr)
{
  return new (addr) UReal;
}

/*
4.8.12 Creation of the type constructor ~ureal~

*/
TypeConstructor unitreal(
        UReal::BasicType(),              //name
        URealProperty,  //property function describing signature
        OutUReal,     InUReal, //Out and In functions
        0,            0,   //SaveToList and RestoreFromList functions
        CreateUReal,
        DeleteUReal, //object creation and deletion
        OpenAttribute<UReal>,
        SaveAttribute<UReal>,  // object open and save
        CloseUReal,   CloneUReal, //object close and clone
        CastUReal, //cast function
        SizeOfUReal, //sizeof function
        CheckUReal );                     //kind checking function

/*
4.9 Type Constructor ~upoint~

Type ~upoint~ represents an (tinterval, (x0, y0, x1, y1))-pair.

4.9.1 List Representation

The list representation of an ~upoint~ is

----    ( timeinterval (x0 yo x1 y1) )
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   (1.0 2.3 4.1 2.1) )
----

4.9.2 function Describing the Signature of the Type Constructor

*/
ListExpr
UPointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("("+UPoint::BasicType()+") "),
      nl->TextAtom("( timeInterval (real_x0 real_y0 real_x1 real_y1) ) "),
      nl->StringAtom("((i1 i2 TRUE FALSE) (1.0 2.2 2.5 2.1))"))));
}

/*
4.9.3 Kind Checking Function

*/
bool
CheckUPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, UPoint::BasicType() ));
}

/*
4.9.4 ~Out~-function

*/
ListExpr OutUPoint( ListExpr typeInfo, Word value )
{
  UPoint* upoint = (UPoint*)(value.addr);

  if( !(((UPoint*)value.addr)->IsDefined()) )
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
  else
    {
      ListExpr timeintervalList = nl->FourElemList(
          OutDateTime( nl->TheEmptyList(),
          SetWord(&upoint->timeInterval.start) ),
          OutDateTime( nl->TheEmptyList(), SetWord(&upoint->timeInterval.end) ),
          nl->BoolAtom( upoint->timeInterval.lc ),
          nl->BoolAtom( upoint->timeInterval.rc));

      ListExpr pointsList = nl->FourElemList(
          nl->RealAtom( upoint->p0.GetX() ),
          nl->RealAtom( upoint->p0.GetY() ),
          nl->RealAtom( upoint->p1.GetX() ),
          nl->RealAtom( upoint->p1.GetY() ));

      return nl->TwoElemList( timeintervalList, pointsList );
    }
}

/*
4.9.5 ~In~-function

The Nested list form is like this:( ( 6.37  9.9  TRUE FALSE) (1.0 2.3 4.1 2.1) )

*/
Word InUPoint( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
  string errmsg;
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr first = nl->First( instance );

    if( nl->ListLength( first ) == 4 &&
        nl->IsAtom( nl->Third( first ) ) &&
        nl->AtomType( nl->Third( first ) ) == BoolType &&
        nl->IsAtom( nl->Fourth( first ) ) &&
        nl->AtomType( nl->Fourth( first ) ) == BoolType )
    {

      correct = true;
      Instant *start = (Instant *)InInstant( nl->TheEmptyList(),
              nl->First( first ), errorPos, errorInfo, correct ).addr;

      if( !correct || start == NULL || !start->IsDefined())
      {
//        "InUPoint(): Error in first instant (Must be defined!).";
        errmsg = "InUPoint(): first instant must be defined!.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
                      nl->Second( first ),
                      errorPos, errorInfo, correct ).addr;

      if( !correct  || end == NULL || !end->IsDefined() )
      {
//        errmsg = "InUPoint(): Error in second instant (Must be defined!).";
        errmsg = "InUPoint(): second instant must be defined!.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        delete end;
        return SetWord( Address(0) );
      }

      Interval<Instant> tinterval( *start, *end,
                                   nl->BoolValue( nl->Third( first ) ),
                                   nl->BoolValue( nl->Fourth( first ) ) );
      delete start;
      delete end;

      correct = tinterval.IsValid();
      if (!correct)
        {
          errmsg = "InUPoint(): Non valid time interval.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
        }

      ListExpr second = nl->Second( instance );
      if( nl->ListLength( second ) == 4 &&
          nl->IsAtom( nl->First( second ) ) &&
          nl->AtomType( nl->First( second ) ) == RealType &&
          nl->IsAtom( nl->Second( second ) ) &&
          nl->AtomType( nl->Second( second ) ) == RealType &&
          nl->IsAtom( nl->Third( second ) ) &&
          nl->AtomType( nl->Third( second ) ) == RealType &&
          nl->IsAtom( nl->Fourth( second ) ) &&
          nl->AtomType( nl->Fourth( second ) ) == RealType )
      {
        UPoint *upoint = new UPoint( tinterval,
                                     nl->RealValue( nl->First( second ) ),
                                     nl->RealValue( nl->Second( second ) ),
                                     nl->RealValue( nl->Third( second ) ),
                                     nl->RealValue( nl->Fourth( second ) ) );

        correct = upoint->IsValid();
        if( correct )
          return SetWord( upoint );

        errmsg = "InUPoint(): Error in start/end point.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete upoint;
      }
    }
  }
  else if ( listutils::isSymbolUndefined(instance) )
    {
      UPoint *upoint = new UPoint(true);
      upoint->SetDefined(false);
      upoint->timeInterval=
        Interval<DateTime>(DateTime(instanttype),
                           DateTime(instanttype),true,true);
      correct = upoint->timeInterval.IsValid();
      if ( correct )
        return (SetWord( upoint ));
    }
  errmsg = "InUPoint(): Error in representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  correct = false;
  return SetWord( Address(0) );
}

/*
4.9.6 ~Create~-function

*/
Word CreateUPoint( const ListExpr typeInfo )
{
  return (SetWord( new UPoint(false) ));
}

/*
4.9.7 ~Delete~-function

*/
void DeleteUPoint( const ListExpr typeInfo, Word& w )
{
  delete (UPoint *)w.addr;
  w.addr = 0;
}

/*
4.9.8 ~Close~-function

*/
void CloseUPoint( const ListExpr typeInfo, Word& w )
{
  delete (UPoint *)w.addr;
  w.addr = 0;
}

/*
4.9.9 ~Clone~-function

*/
Word CloneUPoint( const ListExpr typeInfo, const Word& w )
{
  UPoint *upoint = (UPoint *)w.addr;
  return SetWord( new UPoint( *upoint ) );
}

/*
4.9.10 ~Sizeof~-function

*/
int SizeOfUPoint()
{
  return sizeof(UPoint);
}

/*
4.9.11 ~Cast~-function

*/
void* CastUPoint(void* addr)
{
  return new (addr) UPoint;
}

/*
4.9.12 Creation of the type constructor ~upoint~

*/
TypeConstructor unitpoint(
        UPoint::BasicType(),      //name
        UPointProperty,               //property function describing signature
        OutUPoint,     InUPoint, //Out and In functions
        0,             0,  //SaveToList and RestoreFromList functions
        CreateUPoint,
        DeleteUPoint, //object creation and deletion
        OpenAttribute<UPoint>,
        SaveAttribute<UPoint>,  // object open and save
        CloseUPoint,   CloneUPoint, //object close and clone
        CastUPoint, //cast function
        SizeOfUPoint, //sizeof function
        CheckUPoint );                    //kind checking function

/*
4.10 Type Constructor ~mbool~

Type ~mbool~ represents a moving boolean.

4.10.1 List Representation

The list representation of a ~mbool~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~ubool~.

For example:

----    (
          ( (instant 6.37)  (instant 9.9)   TRUE FALSE) TRUE )
          ( (instant 11.4)  (instant 13.9)  FALSE FALSE) FALSE )
        )
----

4.10.2 function Describing the Signature of the Type Constructor

*/
ListExpr
MBoolProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mbool) "),
                             nl->StringAtom("( u1 ... un)"),
           nl->StringAtom("(((i1 i2 TRUE TRUE) TRUE) ...)"))));
}

/*
4.10.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckMBool( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, MBool::BasicType() ));
}

/*
4.10.4 Creation of the type constructor ~mbool~

*/
TypeConstructor movingbool(
        MBool::BasicType(), //name
        MBoolProperty,  //property function describing signature
        OutMapping<MBool, UBool, OutConstTemporalUnit<CcBool, OutCcBool> >,
        InMapping<MBool, UBool, InConstTemporalUnit<CcBool, InCcBool> >,
       //Out and In functions
        0,
        0,    //SaveToList and RestoreFromList functions
        CreateMapping<MBool>,
        DeleteMapping<MBool>,        //object creation and deletion
        OpenAttribute<MBool>,
        SaveAttribute<MBool>,          // object open and save
        CloseMapping<MBool>,
        CloneMapping<MBool>,     //object close and clone
        CastMapping<MBool>,     //cast function
        SizeOfMapping<MBool>,    //sizeof function
        CheckMBool );          //kind checking function

/*
4.10 Type Constructor ~mint~

Type ~mint~ represents a moving integer.

4.10.1 List Representation

The list representation of a ~mint~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~uint~.

For example:

----    (
          ( (instant 6.37)  (instant 9.9)   TRUE FALSE) 1 )
          ( (instant 11.4)  (instant 13.9)  FALSE FALSE) 4 )
        )
----

4.10.2 function Describing the Signature of the Type Constructor

*/
ListExpr
MIntProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mint) "),
                             nl->StringAtom("( u1 ... un)"),
                             nl->StringAtom("(((i1 i2 TRUE TRUE) 1) ...)"))));
}

/*
4.10.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckMInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, MInt::BasicType() ));
}

/*
4.10.4 Creation of the type constructor ~mint~

*/
TypeConstructor movingint(
        MInt::BasicType(),               //name
        MIntProperty,   //property function describing signature
        OutMapping<MInt, UInt, OutConstTemporalUnit<CcInt, OutCcInt> >,
        InMapping<MInt, UInt, InConstTemporalUnit<CcInt, InCcInt> >,
    //Out and In functions
        0,
        0,            //SaveToList and RestoreFromList functions
        CreateMapping<MInt>,
        DeleteMapping<MInt>,   //object creation and deletion
        OpenAttribute<MInt>,
        SaveAttribute<MInt>,           // object open and save
        CloseMapping<MInt>,
        CloneMapping<MInt>, //object close and clone
        CastMapping<MInt>,  //cast function
        SizeOfMapping<MInt>,  //sizeof function
        CheckMInt );      //kind checking function

/*
4.11 Type Constructor ~mreal~

Type ~mreal~ represents a moving real.

4.11.1 List Representation

The list representation of a ~mreal~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~ureal~.

For example:

----    (
          ( (instant 6.37)  (instant 9.9)   TRUE FALSE) (1.0 2.3 4.1 TRUE) )
          ( (instant 11.4)  (instant 13.9)  FALSE FALSE) (1.0 2.3 4.1 FALSE) )
        )
----

4.11.2 function Describing the Signature of the Type Constructor

*/
ListExpr
MRealProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mreal) "),
                             nl->StringAtom("( u1 ... un) "),
           nl->StringAtom("(((i1 i2 TRUE FALSE) (1.0 2.2 2.5 FALSE)) ...)"))));
}

/*
4.11.3 Kind Checking Function

*/
bool
CheckMReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, MReal::BasicType() ));
}

/*
4.11.4 Creation of the type constructor ~mreal~

*/
TypeConstructor movingreal(
        MReal::BasicType(),                    //name
        MRealProperty,    //property function describing signature
        OutMapping<MReal, UReal, OutUReal>,
        InMapping<MReal, UReal, InUReal>,   //Out and In functions
        0,
        0,      //SaveToList and RestoreFromList functions
        CreateMapping<MReal>,
        DeleteMapping<MReal>,    //object creation and deletion
        OpenAttribute<MReal>,
        SaveAttribute<MReal>,        // object open and save
        CloseMapping<MReal>,
        CloneMapping<MReal>,    //object close and clone
        CastMapping<MReal>,    //cast function
        SizeOfMapping<MReal>,    //sizeof function
        CheckMReal );                        //kind checking function

/*
4.12 Type Constructor ~mpoint~

Type ~mpoint~ represents a moving point.

4.12.1 List Representation

The list representation of a ~mpoint~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~upoint~.

For example:

----    (
          ( (instant 6.37)  (instant 9.9)   TRUE FALSE) (1.0 2.3 4.1 2.1) )
          ( (instant 11.4)  (instant 13.9)  FALSE FALSE) (4.1 2.1 8.9 4.3) )
        )
----

4.12.2 function Describing the Signature of the Type Constructor

*/
ListExpr
MPointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mpoint) "),
                             nl->StringAtom("( u1 ... un ) "),
        nl->StringAtom("(((i1 i2 TRUE FALSE) (1.0 2.2 2.5 2.1)) ...)"))));
}

/*
4.12.3 Kind Checking Function

*/
bool
CheckMPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, MPoint::BasicType() ));
}

/*
4.12.4 Creation of the type constructor ~mpoint~

*/
TypeConstructor movingpoint(
        MPoint::BasicType(),   //name
        MPointProperty,        //property function describing signature
        OutMapping<MPoint, UPoint, OutUPoint>,
        InMapping<MPoint, UPoint, InUPoint>,//Out and In functions
        0,
        0,                 //SaveToList and RestoreFromList functions
        CreateMapping<MPoint>,
        DeleteMapping<MPoint>,     //object creation and deletion
        OpenAttribute<MPoint>,
        SaveAttribute<MPoint>,      // object open and save
        CloseMapping<MPoint>,
        CloneMapping<MPoint>, //object close and clone
        CastMapping<MPoint>,    //cast function
        SizeOfMapping<MPoint>, //sizeof function
        CheckMPoint );  //kind checking function


/*
4.12.5 Creation of the type constructore ~cellgrid2d~

*/

GenTC<CellGrid2D> cellgrid2d;



/*
16 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

16.1 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

16.1.1 Type mapping function ~TemporalTypeMapBool~

Is used for the ~isempty~ operator.

*/
ListExpr
TemporalTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, Instant::BasicType() ) ||
        nl->IsEqual( arg1, RInt::BasicType() ) ||
        nl->IsEqual( arg1, RReal::BasicType() ) ||
        nl->IsEqual( arg1, Periods::BasicType() ) ||
        nl->IsEqual( arg1, UBool::BasicType() ) ||
        nl->IsEqual( arg1, UInt::BasicType() ) ||
        nl->IsEqual( arg1, UReal::BasicType() ) ||
        nl->IsEqual( arg1, UPoint::BasicType() ) ||
//        nl->IsEqual( arg1, MBool::BasicType() ) ||
//        nl->IsEqual( arg1, MInt::BasicType() ) ||
//        nl->IsEqual( arg1, MReal::BasicType() ) ||
//        nl->IsEqual( arg1, MPoint::BasicType() ) ||
        nl->IsEqual( arg1, IBool::BasicType() ) ||
        nl->IsEqual( arg1, IInt::BasicType() ) ||
        nl->IsEqual( arg1, IReal::BasicType() ) ||
        nl->IsEqual( arg1, IPoint::BasicType() ))
      return nl->SymbolAtom( CcBool::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.1 Type mapping function ~TemporalTemporalTypeMapBool~

Is used for the ~=~, and ~\#~ operators.

*/
ListExpr
TemporalTemporalTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( (nl->IsEqual( arg1, Instant::BasicType() )
      && nl->IsEqual( arg2, Instant::BasicType() )) ||
        (nl->IsEqual( arg1, RInt::BasicType() )
        && nl->IsEqual( arg2, RInt::BasicType() )) ||
        (nl->IsEqual( arg1, RReal::BasicType() )
        && nl->IsEqual( arg2, RReal::BasicType() )) ||
        (nl->IsEqual( arg1, Periods::BasicType() )
        && nl->IsEqual( arg2, Periods::BasicType() )) ||
        (nl->IsEqual( arg1, IBool::BasicType() )
        && nl->IsEqual( arg2, IBool::BasicType() )) ||
        (nl->IsEqual( arg1, IInt::BasicType() )
        && nl->IsEqual( arg2, IInt::BasicType() )) ||
        (nl->IsEqual( arg1, IReal::BasicType() )
        && nl->IsEqual( arg2, IReal::BasicType() )) ||
        (nl->IsEqual( arg1, IPoint::BasicType() )
        && nl->IsEqual( arg2, IPoint::BasicType() )))
      return nl->SymbolAtom( CcBool::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

ListExpr
TemporalTemporalTypeMapBool2( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if(  (nl->IsEqual( arg1, MBool::BasicType() )
      && nl->IsEqual( arg2, MBool::BasicType() )) ||
        (nl->IsEqual( arg1, MInt::BasicType() )
        && nl->IsEqual( arg2, MInt::BasicType() )) ||
        (nl->IsEqual( arg1, MReal::BasicType() )
        && nl->IsEqual( arg2, MReal::BasicType() )) ||
        (nl->IsEqual( arg1, MPoint::BasicType() )
        && nl->IsEqual( arg2, MPoint::BasicType() )) )
      return nl->SymbolAtom( CcBool::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
Type Mapping for the mbool2mint function

*/
ListExpr TemporalMBool2MInt(ListExpr args){
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("Single argument expected");
    return nl->SymbolAtom( Symbol::TYPEERROR() );
  }
  if(nl->IsEqual(nl->First(args),MBool::BasicType())){
    return   nl->SymbolAtom(MInt::BasicType());
  }
  ErrorReporter::ReportError("mbool expected");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
Type Mapping for the mint2mbool function

*/
ListExpr TemporalMInt2MBool(ListExpr args){
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("Single argument expected");
    return nl->SymbolAtom( Symbol::TYPEERROR() );
  }
  if(nl->IsEqual(nl->First(args),MInt::BasicType())){
    return   nl->SymbolAtom(MBool::BasicType());
  }
  ErrorReporter::ReportError("mint expected");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
Type Mapping for the mint2mreal function

*/
ListExpr TemporalMInt2MReal(ListExpr args){
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("Single argument expected");
    return nl->SymbolAtom( Symbol::TYPEERROR() );
  }
  if(nl->IsEqual(nl->First(args),MInt::BasicType())){
    return   nl->SymbolAtom(MReal::BasicType());
  }
  ErrorReporter::ReportError("mint expected");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.1 Type Mapping for the ~ExtendDeftime~ function

*/
ListExpr ExtDeftimeTypeMap(ListExpr args){
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("two argument expected");
     return nl->SymbolAtom( Symbol::TYPEERROR() );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->AtomType(arg1)!=SymbolType || nl->AtomType(arg2)!=SymbolType){
     ErrorReporter::ReportError("simple types required");
     return nl->SymbolAtom( Symbol::TYPEERROR() );
  }
  string sarg1 = nl->SymbolValue(arg1);
  string sarg2 = nl->SymbolValue(arg2);
  if( (sarg1==MInt::BasicType()
    && sarg2==UInt::BasicType() ) ||
      (sarg1==MBool::BasicType()
      && sarg2==UBool::BasicType()) ){
     return nl->SymbolAtom(sarg1);
  }
  ErrorReporter::ReportError("(mint x uint)  or (mbool x ubool) needed");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}


/*
16.1.1 Type mapping function ~InstantInstantTypeMapBool~

Is used for the ~<~, ~<=~, ~>~, and ~>=~ operators.

*/
ListExpr
InstantInstantTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, Instant::BasicType() )
      && nl->IsEqual( arg2, Instant::BasicType() ) )
      return nl->SymbolAtom( CcBool::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.2 Type mapping function ~RangeRangeTypeMapBool~

It is for the ~intersects~, which have two
~ranges~ as input and a ~bool~ result type.

*/
ListExpr
RangeRangeTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( (nl->IsEqual( arg1, RInt::BasicType() )
      && nl->IsEqual( arg2, RInt::BasicType() )) ||
        (nl->IsEqual( arg1, RReal::BasicType() )
        && nl->IsEqual( arg2, RReal::BasicType() )) ||
        (nl->IsEqual( arg1, Periods::BasicType() )
        && nl->IsEqual( arg2, Periods::BasicType() )) )
      return nl->SymbolAtom( CcBool::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.3 Type mapping function ~RangeBaseTypeMapBool1~

It is for the operator ~inside~ which have two ~ranges~ as input or a
~BASE~ and a ~range~ in this order as arguments and ~bool~ as the result type.

*/
ListExpr
RangeBaseTypeMapBool1( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( (nl->IsEqual( arg1, RInt::BasicType() )
      && nl->IsEqual( arg2, RInt::BasicType() )) ||
        (nl->IsEqual( arg1, RReal::BasicType() )
        && nl->IsEqual( arg2, RReal::BasicType() )) ||
        (nl->IsEqual( arg1, Periods::BasicType() )
        && nl->IsEqual( arg2, Periods::BasicType() )) ||
        (nl->IsEqual( arg1, CcInt::BasicType() )
        && nl->IsEqual( arg2, RInt::BasicType() )) ||
        (nl->IsEqual( arg1, CcReal::BasicType() )
        && nl->IsEqual( arg2, RReal::BasicType() )) ||
        (nl->IsEqual( arg1, Instant::BasicType() )
        && nl->IsEqual( arg2, Periods::BasicType() )) )
      return nl->SymbolAtom( CcBool::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.4 Type mapping function ~RangeBaseTypeMapBool2~

It is for the operator ~before~ which have two ~ranges~ as input or a
~BASE~ and a ~range~ in any order as arguments and ~bool~ as the result type.

*/
ListExpr
RangeBaseTypeMapBool2( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( (nl->IsEqual( arg1, RInt::BasicType() )
      && nl->IsEqual( arg2, RInt::BasicType() )) ||
        (nl->IsEqual( arg1, RReal::BasicType() )
        && nl->IsEqual( arg2, RReal::BasicType() )) ||
        (nl->IsEqual( arg1, Periods::BasicType() )
        && nl->IsEqual( arg2, Periods::BasicType() )) ||
        (nl->IsEqual( arg1, CcInt::BasicType() )
        && nl->IsEqual( arg2, RInt::BasicType() )) ||
        (nl->IsEqual( arg1, CcReal::BasicType() )
        && nl->IsEqual( arg2, RReal::BasicType() )) ||
        (nl->IsEqual( arg1, Instant::BasicType() )
        && nl->IsEqual( arg2, Periods::BasicType() )) ||
        (nl->IsEqual( arg1, RInt::BasicType() )
        && nl->IsEqual( arg2, CcInt::BasicType() )) ||
        (nl->IsEqual( arg1, RReal::BasicType() )
        && nl->IsEqual( arg2, CcReal::BasicType() )) ||
        (nl->IsEqual( arg1, Periods::BasicType() )
        && nl->IsEqual( arg2, Instant::BasicType() )) )
      return nl->SymbolAtom( CcBool::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.5 Type mapping function ~RangeRangeTypeMapRange~

It is for the operators ~intersection~, ~union~, and ~minus~ which have two
~ranges~ as input and a ~range~ as result type.

*/
ListExpr
RangeRangeTypeMapRange( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, RInt::BasicType() )
      && nl->IsEqual( arg2, RInt::BasicType() ) )
      return nl->SymbolAtom( RInt::BasicType() );

    if( nl->IsEqual( arg1, RReal::BasicType() )
      && nl->IsEqual( arg2, RReal::BasicType() ) )
      return nl->SymbolAtom( RReal::BasicType() );

    if( nl->IsEqual( arg1, Periods::BasicType() )
      && nl->IsEqual( arg2, Periods::BasicType() ) )
      return nl->SymbolAtom( Periods::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}


/*
16.1.6 Type mapping function ~RangeTypeMapBase~

It is for the aggregate operators ~min~, ~max~, and ~avg~ which have one
~range~ as input and a ~BASE~ as result type.

*/
ListExpr
RangeTypeMapBase( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, RInt::BasicType() ) )
      return nl->SymbolAtom( CcInt::BasicType() );

    if( nl->IsEqual( arg1, RReal::BasicType() ) )
      return nl->SymbolAtom( CcReal::BasicType() );

    if( nl->IsEqual( arg1, Periods::BasicType() ) )
      return nl->SymbolAtom( Instant::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.7 Type mapping function ~TemporalSetValueTypeMapInt~

It is for the ~no\_components~ operator.

*/
ListExpr
TemporalSetValueTypeMapInt( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, RInt::BasicType() ) ||
        nl->IsEqual( arg1, RReal::BasicType() ) ||
        nl->IsEqual( arg1, Periods::BasicType() ) ||
        nl->IsEqual( arg1, MBool::BasicType() ) ||
        nl->IsEqual( arg1, MInt::BasicType() ) ||
        nl->IsEqual( arg1, MReal::BasicType() ) ||
        nl->IsEqual( arg1, MPoint::BasicType() ) )
      return nl->SymbolAtom( CcInt::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.6 Type mapping function ~IntimeTypeMapBase~

It is for the operator ~val~.

*/
ListExpr
IntimeTypeMapBase( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, IBool::BasicType() ) )
      return nl->SymbolAtom( CcBool::BasicType() );

    if( nl->IsEqual( arg1, IInt::BasicType() ) )
      return nl->SymbolAtom( CcInt::BasicType() );

    if( nl->IsEqual( arg1, IReal::BasicType() ) )
      return nl->SymbolAtom( CcReal::BasicType() );

    if( nl->IsEqual( arg1, IPoint::BasicType() ) )
      return nl->SymbolAtom( Point::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.6 Type mapping function ~UIntimeTypeMapBase~

It is for the operator ~val~.

*/
ListExpr
UIntimeTypeMapBase( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, UInt::BasicType() ) )
      return nl->SymbolAtom( CcInt::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.6 Type mapping function ~IntimeTypeMapInstant~

It is for the operator ~inst~.

*/
ListExpr
IntimeTypeMapInstant( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, IBool::BasicType() ) ||
        nl->IsEqual( arg1, IInt::BasicType() ) ||
        nl->IsEqual( arg1, IReal::BasicType() ) ||
        nl->IsEqual( arg1, IPoint::BasicType() ) )
      return nl->SymbolAtom( Instant::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.7 Type mapping function ~MovingInstantTypeMapIntime~

It is for the operator ~atinstant~.

*/
ListExpr
MovingInstantTypeMapIntime( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, Instant::BasicType() ) )
    {
      if( nl->IsEqual( arg1, MBool::BasicType() ) )
        return nl->SymbolAtom( IBool::BasicType() );

      if( nl->IsEqual( arg1, MInt::BasicType() ) )
        return nl->SymbolAtom( IInt::BasicType() );

      if( nl->IsEqual( arg1, MReal::BasicType() ) )
        return nl->SymbolAtom( IReal::BasicType() );

      if( nl->IsEqual( arg1, MPoint::BasicType() ) )
        return nl->SymbolAtom( IPoint::BasicType() );
    }
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.7 Type mapping function ~WhenTM~

It is for the operator ~when~.

*/
ListExpr WhenTM( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr mx = nl->First( args ),
             mb = nl->Second( args );

    if( nl->IsEqual( mb, MBool::BasicType() ) )
    {
      if( nl->IsEqual( mx, MBool::BasicType() ) ||
          nl->IsEqual( mx, MInt::BasicType() )  ||
          nl->IsEqual( mx, MReal::BasicType() ) ||
          nl->IsEqual( mx, MPoint::BasicType() ))
        return mx;
    }
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.7 Type mapping function ~MovingPeriodsTypeMapMoving~

It is for the operator ~atperiods~.

*/
ListExpr
MovingPeriodsTypeMapMoving( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, Periods::BasicType() ) )
    {
      if( nl->IsEqual( arg1, MBool::BasicType() ) )
        return nl->SymbolAtom( MBool::BasicType() );

      if( nl->IsEqual( arg1, MInt::BasicType() ) )
        return nl->SymbolAtom( MInt::BasicType() );

      if( nl->IsEqual( arg1, MReal::BasicType() ) )
        return nl->SymbolAtom( MReal::BasicType() );

      if( nl->IsEqual( arg1, MPoint::BasicType() ) )
        return nl->SymbolAtom( MPoint::BasicType() );
    }
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.8 Type mapping function ~MovingTypeMapRange~

It is for the operator ~deftime~.

*/
ListExpr
MovingTypeMapPeriods( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, MBool::BasicType() ) ||
        nl->IsEqual( arg1, MInt::BasicType() ) ||
        nl->IsEqual( arg1, MReal::BasicType() ) ||
        nl->IsEqual( arg1, MPoint::BasicType() ) )
      return nl->SymbolAtom( Periods::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.8 Type mapping function ~MovingTypeMapSpatial~

It is for the operator ~trajectory~.

*/
ListExpr
MovingTypeMapSpatial( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, MPoint::BasicType() ) )
      return nl->SymbolAtom( Line::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.9 Type mapping function ~MovingInstantPeriodsTypeMapBool~

It is for the operator ~present~.

*/
ListExpr
MovingInstantPeriodsTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, Instant::BasicType() ) ||
        nl->IsEqual( arg2, Periods::BasicType() ) )
    {
      if( nl->IsEqual( arg1, MBool::BasicType() ) ||
          nl->IsEqual( arg1, MInt::BasicType() ) ||
          nl->IsEqual( arg1, MReal::BasicType() ) ||
          nl->IsEqual( arg1, MPoint::BasicType() ) )
        return nl->SymbolAtom( CcBool::BasicType() );
    }
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.10 Type mapping function ~MovingBaseTypeMapBool~

It is for the operator ~passes~.

*/
ListExpr
MovingBaseTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( (nl->IsEqual( arg1, MBool::BasicType() )
      && nl->IsEqual( arg2, CcBool::BasicType() )) ||
        (nl->IsEqual( arg1, MInt::BasicType() )
        && nl->IsEqual( arg2, CcInt::BasicType() )) ||
        (nl->IsEqual( arg1, MReal::BasicType() )
        && nl->IsEqual( arg2, CcReal::BasicType() )) ||
        (nl->IsEqual( arg1, MPoint::BasicType() )
        && nl->IsEqual( arg2, Point::BasicType() )) ||
        (nl->IsEqual( arg1, MPoint::BasicType() )
        && nl->IsEqual( arg2, Region::BasicType() )) ||
        (nl->IsEqual( arg1, MPoint::BasicType() )
        && nl->IsEqual( arg2, Rectangle<2>::BasicType() )) )
      return nl->SymbolAtom( CcBool::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.10 Type mapping function ~MovingBaseTypeMapBool~

It is for the operator ~at~.

*/
ListExpr
MovingBaseTypeMapMoving( ListExpr args )
{
  ListExpr arg1, arg2;
  if( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, MBool::BasicType() )
      && nl->IsEqual( arg2, CcBool::BasicType() ) )
      return nl->SymbolAtom( MBool::BasicType() );

    if( nl->IsEqual( arg1, MInt::BasicType() )
      && nl->IsEqual( arg2, CcInt::BasicType() ) )
      return nl->SymbolAtom( MInt::BasicType() );

   if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, CcReal::BasicType() ) )
     return nl->SymbolAtom( MReal::BasicType() );

    if( nl->IsEqual( arg1, MPoint::BasicType() )
      && nl->IsEqual( arg2, Point::BasicType() ) )
      return nl->SymbolAtom( MPoint::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.11 Type mapping function ~MovingTypeMapeIntime~

It is for the operators ~initial~ and ~final~.

*/
ListExpr
MovingTypeMapIntime( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, MBool::BasicType() ) )
      return nl->SymbolAtom( IBool::BasicType() );

    if( nl->IsEqual( arg1, MInt::BasicType() ) )
      return nl->SymbolAtom( IInt::BasicType() );

    if( nl->IsEqual( arg1, MReal::BasicType() ) )
      return nl->SymbolAtom( IReal::BasicType() );

    if( nl->IsEqual( arg1, MPoint::BasicType() ) )
      return nl->SymbolAtom( IPoint::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.11 Type mapping function ~MovingTypeMapeIntime~

It is for the operators ~initial~ and ~final~.

*/
ListExpr
MovingBaseTypeMapMReal( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, MPoint::BasicType() )
      && nl->IsEqual( arg2, Point::BasicType() ) )
      return nl->SymbolAtom( MReal::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.12 Type Mapping Function ~MovingTypeMapUnits~

It is used for the operator ~units~

Type mapping for ~units~ is

----    (mbool)  -> (stream ubool)
        (mint)   -> (stream uint)
  (mreal)  -> (stream ureal)
  (mpoint) -> (stream upoint)
----

*/
ListExpr MovingTypeMapUnits( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);

    if( nl->IsEqual( arg1, MBool::BasicType() ) )
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                      nl->SymbolAtom(UBool::BasicType()));

    if( nl->IsEqual( arg1, MInt::BasicType() ) )
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                      nl->SymbolAtom(UInt::BasicType()));

    if( nl->IsEqual( arg1, MReal::BasicType() ) )
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(UReal::BasicType()));

    if( nl->IsEqual( arg1, MPoint::BasicType() ) )
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(UPoint::BasicType()));
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
16.1.12 Type Mapping Function ~MovingTypeMapGetUnit~

It is used for the operator ~getunit~

Type mapping for ~getunit~ is

----    (mbool)  -> (ubool)
        (mint)   -> (uint)
  (mreal)  -> (ureal)
  (mpoint) -> (upoint)
----

*/
ListExpr MovingTypeMapGetUnit( ListExpr args )
{
  if ( nl->ListLength(args) == 2
    && nl->IsEqual(nl->Second(args), CcInt::BasicType()))
  {
    ListExpr arg1 = nl->First(args);

    if( nl->IsEqual( arg1, MBool::BasicType() ) )
      return nl->SymbolAtom(UBool::BasicType());

    if( nl->IsEqual( arg1, MInt::BasicType() ) )
      return nl->SymbolAtom(UInt::BasicType());

    if( nl->IsEqual( arg1, MReal::BasicType() ) )
      return nl->SymbolAtom(UReal::BasicType());

    if( nl->IsEqual( arg1, MPoint::BasicType() ) )
      return nl->SymbolAtom(UPoint::BasicType());
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
16.1.12 Type mapping for simplify operator

*/

ListExpr MovingTypeMapSimplify(ListExpr args){
   int len = nl->ListLength(args);

   if((len!=2) && (len !=3)){
       ErrorReporter::ReportError("two or three arguments expected");
       return nl->SymbolAtom(Symbol::TYPEERROR());
   }
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if(nl->IsEqual(arg1,MPoint::BasicType()) &&
      nl->IsEqual(arg2,CcReal::BasicType())){
        if(len==2){
           return nl->SymbolAtom(MPoint::BasicType());
        } else { // check the third argument
          ListExpr arg3 = nl->Third(args);
          if(nl->IsEqual(arg3,Duration::BasicType())){
             return nl->SymbolAtom(MPoint::BasicType());
          }
        }
   }
   if( (len==2) && (nl->IsEqual(arg1,MReal::BasicType())) &&
       (nl->IsEqual(arg2,CcReal::BasicType()))){
       return nl->SymbolAtom(MReal::BasicType());
   }

   ErrorReporter::ReportError(" (mpoint x real [ x duration])"
                             "  or (mreal x real) expected");
   return nl->SymbolAtom(Symbol::TYPEERROR());
}


/*
16.1.12 Type mapping for the breakpoints  operator

*/

ListExpr MovingTypeMapBreakPoints(ListExpr args){
   int len = nl->ListLength(args);
   string err = "mpoint x duration [ x real] expected";
   if( (len!=2) && (len!=3)){
     return listutils::typeError(err + " (wrong number of args)");
   }
   if(!MPoint::checkType(nl->First(args)) ||
      !Duration::checkType(nl->Second(args))){
     return listutils::typeError(err );
   }   
   if((len==3) &&  !CcReal::checkType(nl->Third(args))){
     return listutils::typeError(err);
   }
   return nl->SymbolAtom(Points::BasicType());
}

/*
16.1.12 Type Mapping for Operator ~breaks~

Signature:  mpoint x duration x real -> periods

*/
ListExpr breaksTM(ListExpr args){
  string err = " mpoint x duration x real expected";
  if(!nl->HasLength(args,3)){
    return listutils::typeError(err);
  }
  if(!MPoint::checkType(nl->First(args)) ||
     !Duration::checkType(nl->Second(args)) ||
     !CcReal::checkType(nl->Third(args))){
    return listutils::typeError(err);
  }
  return nl->SymbolAtom(Periods::BasicType());
 

}






/*
16.1.12 Type mapping for the gk  operator

*/

ListExpr MovingTypeMapgk(ListExpr args){
  int len = nl->ListLength(args);
  if( (len < 1) || (len > 2) ){
    ErrorReporter::ReportError("One or two arguments expected.");
    return nl->TypeError();
  }
  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1,MPoint::BasicType())){
    ErrorReporter::ReportError("mpoint expected");
    return nl->TypeError();
  }
  if( (len==2) && nl->IsEqual(nl->Second(args),CcInt::BasicType()) ) {
      return nl->SymbolAtom(MPoint::BasicType()); // Zone provided by user
  } else if (len==1){
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
           nl->OneElemList(nl->IntAtom(2)),  // standard zone for Hagen
           nl->SymbolAtom(MPoint::BasicType()));
  }
  ErrorReporter::ReportError("No match.");
  return nl->TypeError();
}

ListExpr DelayOperatorTypeMapping( ListExpr args )
{
  int len = nl->ListLength(args);
  string errmsg = "Expected mpoint x mpoint [x geoid].";
  if((len<2) || (len>3)){
    return listutils::typeError(errmsg);
  }
  if(!listutils::isSymbol(nl->First(args),MPoint::BasicType())){
    return listutils::typeError(errmsg);
  }
  if(!listutils::isSymbol(nl->Second(args),MPoint::BasicType())){
    return listutils::typeError(errmsg);
  }
  if((len == 3) && (!listutils::isSymbol(nl->Third(args),Geoid::BasicType()))) {
    return listutils::typeError(errmsg);
  }
  /*
  Not implemented:
  1- Check that the two moving points have the same trajectory.
  2- Check the the trajectory is continuos on the spatial space
  (necessary to compute the distance traversed)

  */
  return (nl->SymbolAtom(MReal::BasicType()));
}


/*
16.1.12 Type mapping for the ~vertices~  operator

*/

ListExpr MovingTypeMapVertices(ListExpr args){
   if(nl->ListLength(args)!=1){
       ErrorReporter::ReportError("mpoint expected");
       return nl->TypeError();
   }
   ListExpr arg1 = nl->First(args);
   if(nl->IsEqual(arg1,MPoint::BasicType())){
       return nl->SymbolAtom(Points::BasicType());
   }
   ErrorReporter::ReportError("mpoint expected");
   return nl->TypeError();
}

/*
16.1.12 Type mapping function of the ~integrate~ Operator

*/
ListExpr TypeMapIntegrate(ListExpr args){
   int len = nl->ListLength(args);
   if(len!=1){
      ErrorReporter::ReportError("one argument expected");
      return nl->SymbolAtom(Symbol::TYPEERROR());
   }
   ListExpr arg = nl->First(args);
   if(nl->IsEqual(arg,UReal::BasicType()) ||
     nl->IsEqual(arg,MReal::BasicType())){
     return nl->SymbolAtom(CcReal::BasicType());
   }
   ErrorReporter::ReportError(UReal::BasicType()+" or "+MReal::BasicType()+
                              "expected");
   return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
16.1.12 Type mapping function of the ~min~ and the ~max~ Operator

*/
ListExpr TypeMapMinMax(ListExpr args){
   int len = nl->ListLength(args);
   if(len!=1){
      ErrorReporter::ReportError("one argument expected");
      return nl->SymbolAtom(Symbol::TYPEERROR());
   }
   ListExpr arg = nl->First(args);
   if(nl->IsEqual(arg,UReal::BasicType()) ||
     nl->IsEqual(arg,MReal::BasicType())){
     return nl->SymbolAtom(CcReal::BasicType());
   }
   else
   {
     if (nl->IsEqual(arg,MInt::BasicType())){
       return nl->SymbolAtom(CcInt::BasicType());
     }
   }
   ErrorReporter::ReportError(UReal::BasicType()+" or "
                              +MReal::BasicType()+" expected");
   return nl->SymbolAtom(Symbol::TYPEERROR());
}


/*
16.1.12 Type mapping function of the ~linearize~ Operator

*/
ListExpr TypeMapLinearize(ListExpr args){
   int len = nl->ListLength(args);
   if(len!=1){
      ErrorReporter::ReportError("one argument expected");
      return nl->SymbolAtom(Symbol::TYPEERROR());
   }
   ListExpr arg = nl->First(args);
   if(nl->IsEqual(arg,UReal::BasicType()) ||
      nl->IsEqual(arg,MReal::BasicType())){
      return nl->SymbolAtom(nl->SymbolValue(arg));
   }
   ErrorReporter::ReportError(UReal::BasicType()
                              +" or "+MReal::BasicType()+" expected");
   return nl->SymbolAtom(Symbol::TYPEERROR());
}


/*
16.1.12 Type mapping function of the ~linearize2~ Operator

*/
ListExpr TypeMapLinearize2(ListExpr args){
   int len = nl->ListLength(args);
   if(len!=1){
      ErrorReporter::ReportError("one argument expected");
      return nl->SymbolAtom(Symbol::TYPEERROR());
   }
   ListExpr arg = nl->First(args);
   if( nl->IsEqual(arg,MReal::BasicType()))
      return nl->SymbolAtom(nl->SymbolValue(arg));
   if( nl->IsEqual(arg,UReal::BasicType()))
      return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                             nl->SymbolAtom( UReal::BasicType() ));
                             ErrorReporter::ReportError(MReal::BasicType()+
                             " or "+UReal::BasicType()+" expected");
   return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
16.1.13 Type mapping function for the ~approximate~ operator

*/
ListExpr TypeMapApproximate(ListExpr args){

  int len = nl->ListLength(args);
  if( (len != 3) &&  (len != 4) && (len != 5) && (len!=6)){
      return listutils::typeError("three, four, fivei,"
                                  " or six arguments expected");
  }

  
  // check needed arguments 
  int durindex = -1;
  int boolIndex = -1;
  int breakAttrIndex = -1;

  string err = "stream(tuple) x attr_1 x attr_2 [ x bool] "
               "[ x duration] [ x attr_3] expected"; 
  
  ListExpr stream = nl->First(args);
  ListExpr attrname_time = nl->Second(args);
  ListExpr attrname_value = nl->Third(args);
  if(!listutils::isTupleStream(stream)){
    return  listutils::typeError("first parameter must be a tuple stream");
  }

  if(!listutils::isSymbol(attrname_time)){
    return  listutils::typeError("second parameter must be an attribute name");
  }

  if(!listutils::isSymbol(attrname_value)){
    return  listutils::typeError("third parameter must be an attribute name");
  }

  ListExpr type;
  ListExpr attrList = nl->Second(nl->Second(stream));

  string name = nl->SymbolValue(attrname_time);

  int index1 = listutils::findAttribute(attrList, name, type);

  if(index1==0){
    return listutils::typeError("attribute name " + name +
                                " unknown in tuple stream");
  }
  if(!DateTime::checkType(type)){
    return listutils::typeError("attribute '" + name +
    "' must be of type 'instant'");
  }

  name = nl->SymbolValue(attrname_value);

  int index2 = listutils::findAttribute(attrList, name, type);

  if(index2==0){
    return listutils::typeError("attribute name " + name +
    " unknown in tuple stream");
  }

  string restype;
  if(listutils::isSymbol(type,Point::BasicType())){
      restype = MPoint::BasicType();
  }  else if (listutils::isSymbol(type,CcReal::BasicType())){
     restype = MReal::BasicType();
  }else if (listutils::isSymbol(type,CcInt::BasicType())){
    restype = MInt::BasicType();
  } else if (listutils::isSymbol(type,CcBool::BasicType())){
    restype = MBool::BasicType();
  }else if (listutils::isSymbol(type,CcString::BasicType())){
    restype = MString::BasicType();
  } else {
    return listutils::typeError("third argument is not an allowed type "
                                 "(point, real, int, bool, string)");
  }

  // check optional arguments
  if(len>3){
     ListExpr fourth = nl->Fourth(args);
     if(CcBool::checkType(fourth)){
        boolIndex = 3;
     }else if(Duration::checkType(fourth)){
        durindex = 3;
     } else if(listutils::isSymbol(fourth)){
       ListExpr at;
       breakAttrIndex = listutils::findAttribute(attrList,
                                          nl->SymbolValue(fourth),at);
       if(breakAttrIndex==0){
          return listutils::typeError(err);
       }
       if(!CcBool::checkType(at)){
          return listutils::typeError(err);
       }
       breakAttrIndex--;
     } else {
       return listutils::typeError(err);
     }
   }

  if(len>4){
     ListExpr fifth = nl->Fifth(args);
     if(CcBool::checkType(fifth)){
        if(boolIndex > 0){
          return listutils::typeError(err);
        }
        boolIndex = 4;
     }else if(Duration::checkType(fifth)){
        if(durindex>0){
          return listutils::typeError(err);
        }
        durindex = 4;
     } else if(listutils::isSymbol(fifth)){
       if(breakAttrIndex>=0){
         return listutils::typeError(err);
       }
       ListExpr at;
       breakAttrIndex = listutils::findAttribute(attrList,
                                         nl->SymbolValue(fifth),at);
       if(breakAttrIndex==0){
          return listutils::typeError(err);
       }
       if(!CcBool::checkType(at)){
          return listutils::typeError(err);
       }
       breakAttrIndex--;
     } else {
       return listutils::typeError(err);
     }
   }


  if(len>5){
     ListExpr sixth = nl->Sixth(args);
     if(CcBool::checkType(sixth)){
        if(boolIndex > 0){
          return listutils::typeError(err);
        }
        boolIndex = 5;
     }else if(Duration::checkType(sixth)){
        if(durindex>0){
          return listutils::typeError(err);
        }
        durindex = 5;
     } else if(listutils::isSymbol(sixth)){
       if(breakAttrIndex>=0){
         return listutils::typeError(err);
       }
       ListExpr at;
       breakAttrIndex = listutils::findAttribute(attrList,
                                          nl->SymbolValue(sixth),at);
       if(breakAttrIndex==0){
          return listutils::typeError(err);
       }
       if(!CcBool::checkType(at)){
          return listutils::typeError(err);
       }
       breakAttrIndex--;
     } else {
       return listutils::typeError(err);
     }
   }

  ListExpr indexes = nl->FiveElemList(
                       nl->IntAtom(index1-1),
                       nl->IntAtom(index2-1),
                       nl->IntAtom(durindex),
                       nl->IntAtom(boolIndex),
                       nl->IntAtom(breakAttrIndex)); 

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           indexes,
                           nl->SymbolAtom(restype));
}



/*
16.1.13 Type mapping function ~IntSetTypeMapPeriods~

It is used for the operators ~theyear~, ~themonth~, ~theday~, ~thehour~,
~theminute~,~thesecond~

*/
ListExpr
IntSetTypeMapPeriods( ListExpr args )
{
  ListExpr argi;
  bool correct=true;

  if( ( nl->ListLength(args) < 1 ) || ( nl->ListLength(args) > 6) )
    return nl->SymbolAtom(Symbol::TYPEERROR());

  if( nl->ListLength(args) >= 1 )
  {
    argi = nl->First(args);
    if( !nl->IsEqual( argi, CcInt::BasicType() ) )
      correct = false;
  }

  if ( nl->ListLength(args) >= 2 )
  {
    argi = nl->Second(args);
    if( !nl->IsEqual( argi, CcInt::BasicType() ) )
      correct = false;
  }

  if ( nl->ListLength(args) >= 3 )
  {
    argi = nl->Third(args);
    if( !nl->IsEqual( argi, CcInt::BasicType() ) )
      correct = false;
  }

  if ( nl->ListLength(args) >= 4 )
  {
    argi = nl->Fourth(args);
    if( !nl->IsEqual( argi, CcInt::BasicType() ) )
      correct=false;
  }

  if( nl->ListLength(args) >= 5 )
  {
    argi = nl->Fifth(args);
    if( !nl->IsEqual( argi, CcInt::BasicType() ) )
      correct=false;
  }

  if ( nl->ListLength(args) >= 6 )
  {
    argi = nl->Sixth(args);
    if( !nl->IsEqual( argi, CcInt::BasicType() ) )
      correct=false;
  }

  if( correct )
    return nl->SymbolAtom( Periods::BasicType() );

  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
16.1.13 Type mapping function ~PeriodsPeriodsTypeMapPeriods~

It is used for the operator ~theperiod~

*/
ListExpr
PeriodsPeriodsTypeMapPeriods( ListExpr args )
{
  if( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args),
             arg2 = nl->Second(args);

    if( nl->IsEqual( arg1, Periods::BasicType() )
      && nl->IsEqual( arg2, Periods::BasicType() ) )
      return nl->SymbolAtom( Periods::BasicType() );
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
16.1.15 Type mapping function "MPointTypeMapTranslate"

This type mapping function is used for the ~translate~ operator.

*/
ListExpr MPointTypeMapTranslate( ListExpr args )
{
  ListExpr arg1, arg2;

  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, MPoint::BasicType() ) &&
        nl->IsEqual(nl->First( arg2 ), Duration::BasicType()) &&
        nl->IsEqual(nl->Second( arg2 ), CcReal::BasicType()) &&
        nl->IsEqual(nl->Third( arg2 ), CcReal::BasicType())) {
      return (nl->SymbolAtom( MPoint::BasicType() )); }
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.16 Type mapping function "box3d"

*/
ListExpr Box3dTypeMap(ListExpr args){
  int len = nl->ListLength(args);
  if(len==2){
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if(nl->IsEqual(arg1,Rectangle<2>::BasicType())){
       if(nl->IsEqual(arg2,Instant::BasicType())
         || nl->IsEqual(arg2,Periods::BasicType()))
         return nl->SymbolAtom(Rectangle<3>::BasicType());
    }
    ErrorReporter::ReportError(" rect x {instant, periods } expected\n");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  } else if(len==1){
    ListExpr arg = nl->First(args);
    if(nl->IsEqual(arg,Rectangle<2>::BasicType())
      || (nl->IsEqual(arg,Instant::BasicType()) ||
       nl->IsEqual(arg,Periods::BasicType())))
        return nl->SymbolAtom(Rectangle<3>::BasicType());
    ErrorReporter::ReportError("rect, instant, or periods required\n");
    return nl->SymbolAtom(Symbol::TYPEERROR() );
  }
   else{
    ErrorReporter::ReportError("one or two arguments required");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }

}

/*
16.1.17 Type mapping function "box2d"

*/
ListExpr Box2dTypeMap(ListExpr args)
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if( nl->IsEqual( arg1, Rectangle<2>::BasicType() ) )
      return nl->SymbolAtom( Rectangle<2>::BasicType() );

    if( nl->IsEqual( arg1, Rectangle<3>::BasicType() ) )
      return nl->SymbolAtom( Rectangle<2>::BasicType() );

    if( nl->IsEqual( arg1, Rectangle<4>::BasicType() ) )
      return nl->SymbolAtom( Rectangle<2>::BasicType() );

    if( nl->IsEqual( arg1, Rectangle<8>::BasicType() ) )
      return nl->SymbolAtom( Rectangle<2>::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.18 Type mapping function "TemporalBBoxTypeMap"

For operator ~bbox~

*/

ListExpr TemporalBBoxTypeMap( ListExpr args )
{
  int noargs = nl->ListLength( args );
  string errmsg = "Expected (M [x geoid]) OR (T), where M in {upoint, mpoint, "
                  "ipoint}, T in {instant,periods}.";
  if ( (noargs<1) || (noargs>2) ){
    return listutils::typeError(errmsg);
  }
  if( (noargs==2) && !listutils::isSymbol(nl->Second(args),Geoid::BasicType())){
    return listutils::typeError(errmsg);
  }
  ListExpr arg1 = nl->First( args );

  if( listutils::isSymbol( arg1, UPoint::BasicType() ) )
      return (nl->SymbolAtom( Rectangle<3>::BasicType() ));

  if( listutils::isSymbol( arg1, MPoint::BasicType() ) )
      return (nl->SymbolAtom( Rectangle<3>::BasicType() ));

  if( listutils::isSymbol( arg1, IPoint::BasicType() ) )
      return (nl->SymbolAtom( Rectangle<3>::BasicType() ));

  if( (noargs==1) && listutils::isSymbol( arg1, Periods::BasicType() ) )
      return (nl->SymbolAtom( Rectangle<3>::BasicType() ));

  if( (noargs==1) && listutils::isSymbol( arg1, Instant::BasicType() ) )
      return (nl->SymbolAtom( Rectangle<3>::BasicType() ));

  return listutils::typeError(errmsg);
}

/*
16.1.18 Type mapping function "TemporalMBRangeTypeMap"

For operator ~bbox~

*/

ListExpr TemporalMBRangeTypeMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if( nl->IsEqual( arg1, RInt::BasicType() )  )
      return (nl->SymbolAtom( RInt::BasicType() ));

    if( nl->IsEqual( arg1, RReal::BasicType() ) )
      return (nl->SymbolAtom( RReal::BasicType() ));

    if( nl->IsEqual( arg1, RBool::BasicType() ) )
      return (nl->SymbolAtom( RReal::BasicType() ));

    if( nl->IsEqual( arg1, RString::BasicType() ) )
      return (nl->SymbolAtom( RString::BasicType() ));

    if( nl->IsEqual( arg1, Periods::BasicType() ) )
      return (nl->SymbolAtom( Periods::BasicType() ));
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.18 Type mapping function ~TemporalBBox2dTypeMap~

For operator ~bbox2d~

*/

ListExpr TemporalBBox2dTypeMap( ListExpr args )
{
  int noargs =  nl->ListLength( args );
  string errmsg = "Expected (T [x geoid]), where T in {upoint,mpoint,ipoint}";
  if( (noargs<1) || (noargs>2) ){
    return listutils::typeError(errmsg);
  }
  if( (noargs == 2) &&
                    !listutils::isSymbol(nl->Second(args),Geoid::BasicType()) ){
    return listutils::typeError(errmsg);
  }
  ListExpr arg1 = nl->First( args );
  if( listutils::isSymbol( arg1, UPoint::BasicType() ) )
      return (nl->SymbolAtom( Rectangle<2>::BasicType() ));

  if( listutils::isSymbol( arg1, MPoint::BasicType() ) )
      return (nl->SymbolAtom( Rectangle<2>::BasicType() ));

  if( listutils::isSymbol( arg1, IPoint::BasicType() ) )
      return (nl->SymbolAtom( Rectangle<2>::BasicType() ));

  return listutils::typeError(errmsg);
}

/*
16.1.19 Type mapping function ~TemporalTheRangeTM~

For operator ~theRange~

*/

ListExpr TemporalTheRangeTM( ListExpr args )
{
  ListExpr arg1, arg2, arg3, arg4;
  string argstr;
  nl->WriteToString(argstr, args);

  if ( nl->ListLength( args ) == 4 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    arg3 = nl->Third( args );
    arg4 = nl->Fourth( args );

    if( !nl->Equal(arg1, arg2) )
    {
      ErrorReporter::ReportError("Operator theRange: First two arguments"
          "must have the same type, but argument list is '" + argstr + "'.");
      return nl->SymbolAtom( Symbol::TYPEERROR() );
    }
    if( !nl->IsEqual( arg3, CcBool::BasicType())
      || !nl->IsEqual( arg4, CcBool::BasicType()) )
    {
      ErrorReporter::ReportError("Operator theRange: Third and fourth "
          "arguments must have type 'bool', but argument list is '"
          + argstr + "'.");
      return nl->SymbolAtom( Symbol::TYPEERROR() );
    }
    if( nl->IsEqual( arg1, Instant::BasicType() ) )
      return (nl->SymbolAtom( Periods::BasicType() ));

    if( nl->IsEqual( arg1, CcInt::BasicType() ) )
      return (nl->SymbolAtom( RInt::BasicType() ));

    if( nl->IsEqual( arg1, CcBool::BasicType() ) )
      return (nl->SymbolAtom( RBool::BasicType() ));

    if( nl->IsEqual( arg1, CcReal::BasicType() ) )
      return (nl->SymbolAtom( RReal::BasicType() ));

    if( nl->IsEqual( arg1, CcString::BasicType() ) )
      return (nl->SymbolAtom( RString::BasicType() ));

    ErrorReporter::ReportError("Operator theRange expects as first and "
        "second argument one of {instant, int, bool, real, string}, but "
        "gets a list '" + argstr + "'.");
    return nl->SymbolAtom( Symbol::TYPEERROR() );
  }
  ErrorReporter::ReportError("Operator theRange expects an argument "
      "list of length 4, but gets a list '" + argstr + "'.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}


ListExpr TranslateAppendTM(ListExpr args){
  if(nl->ListLength(args)!=3){
      ErrorReporter::ReportError("two arguments expected");
      return nl->SymbolAtom( Symbol::TYPEERROR() );
  }
  if(nl->IsEqual(nl->First(args),MPoint::BasicType()) &&
     nl->IsEqual(nl->Second(args),MPoint::BasicType()) &&
     nl->IsEqual(nl->Third(args),Duration::BasicType())){
     return nl->SymbolAtom(MPoint::BasicType());
  }
  ErrorReporter::ReportError("mpoint x mpoint x duration expected");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

ListExpr ReverseTM(ListExpr args){
   if(nl->ListLength(args)!=1){
      ErrorReporter::ReportError("1 argument expected");
      return nl->SymbolAtom( Symbol::TYPEERROR() );
   }
   if(nl->IsEqual(nl->First(args),MPoint::BasicType())){
      return nl->SymbolAtom(MPoint::BasicType());
   }
   ErrorReporter::ReportError("mpoint expected");
   return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1.20 TranslateAppendS

This operator consumes a stream of tuples having an attribute
x of type mpoint. The attribute name must be given in the second
argument. As third argument, a duration is given which determines
a break between the two movements.

*/

ListExpr TranslateAppendSTM(ListExpr args){


  int len = nl->ListLength(args);
  if(len != 3 ){
      ErrorReporter::ReportError("three arguments expected");
      return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  // check the third argument to be of type duration
  if(!nl->IsEqual(nl->Third(args),Duration::BasicType())){
    ErrorReporter::ReportError("the third argument has to be a duration");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  // extract the attribute name
  ListExpr attrlist = nl->Second(args);

  if(nl->AtomType(attrlist)!=SymbolType){
      ErrorReporter::ReportError("the second argument has to be a symbol");
      return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  string a1 = nl->SymbolValue(attrlist);

  int a1index = -1;

  // search for attrname in stream definition
  ListExpr stype = nl->First(args);
  if(nl->AtomType(stype)!=NoAtom){
     ErrorReporter::ReportError("stream(tuple(...))"
                                " expected as the first argument");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  if((nl->ListLength(stype)!=2) ||
     (!nl->IsEqual(nl->First(stype),Symbol::STREAM() ))){
     ErrorReporter::ReportError("stream(tuple(...))"
                                " expected as the first argument");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  ListExpr ttype = nl->Second(stype);

  if((nl->ListLength(ttype)!=2) ||
     (!nl->IsEqual(nl->First(ttype),Tuple::BasicType() ))){
     ErrorReporter::ReportError("stream(tuple(...))"
                                " expected as the first argument");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  ListExpr attributes = nl->Second(ttype);
  if(nl->AtomType(attributes)!=NoAtom){
      ErrorReporter::ReportError("invalid tuple type");
      return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  int pos = 0;
  while(!nl->IsEmpty(attributes)){
     ListExpr attr = nl->First(attributes);
     if( (nl->AtomType(attr)!=NoAtom) ||
         (nl->ListLength(attr)!=2)){
         ErrorReporter::ReportError("invalid tuple type");
         return nl->SymbolAtom(Symbol::TYPEERROR());
     }
     ListExpr anl = nl->First(attr);
     ListExpr atl = nl->Second(attr);
     if( (nl->AtomType(anl)!=SymbolType) ||
         (nl->AtomType(atl)!=SymbolType)){
         ErrorReporter::ReportError("invalid tuple type");
         return nl->SymbolAtom(Symbol::TYPEERROR());
     }
     string aname = nl->SymbolValue(anl);
     if(aname==a1){
        if(a1index>=0){
           ErrorReporter::ReportError("attr name occurs twice");
           return nl->SymbolAtom(Symbol::TYPEERROR());
        }
        if(!nl->IsEqual(atl,MPoint::BasicType())){
            ErrorReporter::ReportError("the attribute"
                                       " has to be of type mpoint");
            return nl->SymbolAtom(Symbol::TYPEERROR());
        }
        a1index = pos;
     }
     pos++;
     attributes = nl->Rest(attributes);
  }

  if(a1index<0){
     ErrorReporter::ReportError("first attr name does"
                                " not occur in the typle");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  // all is correct
  ListExpr ind = nl->OneElemList(nl->IntAtom(a1index));

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           ind,
                           nl->SymbolAtom(MPoint::BasicType()));
}


/*
~SampleMPointTypeMap~

This is the type mapping of the ~samplempoint~ operator.

*/

ListExpr SampleMPointTypeMap(ListExpr args){

  int len = nl->ListLength(args);

  if(len!=2 && len!=3 && len!=4){
    ErrorReporter::ReportError("two up to four arguments required");
    return nl->TypeError();
  }
  if(!nl->IsEqual(nl->First(args),MPoint::BasicType()) ||
     !nl->IsEqual(nl->Second(args),Duration::BasicType())){
     ErrorReporter::ReportError(" mpoint x durationi"
                                " [ x bool [x bool]] expected");
     return nl->TypeError();
  }
  if(len==3 && !nl->IsEqual(nl->Third(args),CcBool::BasicType())){
     ErrorReporter::ReportError(" mpoint x duration "
                                "[ x bool [ x bool]] expected");
     return nl->TypeError();
  }
  if(len==4 && !nl->IsEqual(nl->Fourth(args),CcBool::BasicType())){
     ErrorReporter::ReportError(" mpoint x duration "
                                "[ x bool [ x bool]] expected");
     return nl->TypeError();
  }
  return nl->SymbolAtom(MPoint::BasicType());
}


/*
~GPSTypeMap~

The GPS operator works very similar to the SampleMPoint operator.
The main differnce is the type of the result. The GPS operator
produces a stream of tuples consisting of the attributes Time
of type instant and Position of the point. Thus the gps operator can be
used to export data coming from an mpoint to a relation. In contrast to
the SampleMPoint operator, the gps operator does not support the keeping
of the final endpoint.

*/

ListExpr GPSTypeMap(ListExpr args){

  int len = nl->ListLength(args);

  if(len!=2){
    ErrorReporter::ReportError("two arguments required");
    return nl->TypeError();
  }
  if(!nl->IsEqual(nl->First(args),MPoint::BasicType()) ||
     !nl->IsEqual(nl->Second(args),Duration::BasicType())){
     ErrorReporter::ReportError(" mpoint x duration expected");
     return nl->TypeError();
  }

  return nl->TwoElemList(
             nl->SymbolAtom(Symbol::STREAM()),
             nl->TwoElemList(
                 nl->SymbolAtom(Tuple::BasicType()),
                 nl->TwoElemList(
                     nl->TwoElemList(
                         nl->SymbolAtom("Time"),
                         nl->SymbolAtom(Instant::BasicType())
                     ),
                     nl->TwoElemList(
                         nl->SymbolAtom("Position"),
                         nl->SymbolAtom(Point::BasicType())
                     )
                 )
             )
         );
}

/*
~DisturbTypeMap~

*/
ListExpr DisturbTypeMap(ListExpr args){
   int len=nl->ListLength(args);
   if(len!=3){
     ErrorReporter::ReportError("invalid number of arguments");
     return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),MPoint::BasicType()) &&
      nl->IsEqual(nl->Second(args),CcReal::BasicType()) &&
      nl->IsEqual(nl->Third(args),CcReal::BasicType())){
      return nl->SymbolAtom(MPoint::BasicType());
   }
   ErrorReporter::ReportError("mpoint x real x real expected");
   return nl->TypeError();
}

/*
~LengthTypeMap~ (also for operator ~avg\_speed~)

---- mpoint [ x string ] --> real
----

*/

ListExpr LengthTypeMap(ListExpr args){
  string errmsg = "Expected (mpoint) or (mpoint x string).";
  int noargs = nl->ListLength(args);
  if((noargs<1) || (noargs>2)){
    return listutils::typeError(errmsg);
  }
  if(!listutils::isSymbol(nl->First(args),MPoint::BasicType())){
    return listutils::typeError(errmsg);
  }
  if(    (noargs==2)
      && (!listutils::isSymbol(nl->Second(args),CcString::BasicType())) ){
    return listutils::typeError(errmsg);
  }
  return nl->SymbolAtom(CcReal::BasicType());
}


/*
~TypeMapping mpoint x real -> mpoint ~


*/
ListExpr MPointRealTypeMapMPoint(ListExpr args){
   string err = "mpoint x real expected";
   if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),MPoint::BasicType()) &&
      nl->IsEqual(nl->Second(args),CcReal::BasicType())){
      return nl->SymbolAtom(MPoint::BasicType());
   }
   ErrorReporter::ReportError(err);
   return nl->TypeError();
}


/*
~TypeMapping~ for operator equalizeU

 Signature is mpoint x real [ x bool] -> mpoint


*/
ListExpr EqualizeUTM(ListExpr args){
   string err = "mpoint x real [x bool] expected";
   int len = nl->ListLength(args);
   if((len!=2) && (len!=3)){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),MPoint::BasicType()) &&
      nl->IsEqual(nl->Second(args),CcReal::BasicType())){
      if(len==2){
         return nl->SymbolAtom(MPoint::BasicType());
      } else { // len ==3
        if(nl->IsEqual(nl->Third(args),CcBool::BasicType())){
           return nl->SymbolAtom(MPoint::BasicType());
        }
      }
   }
   ErrorReporter::ReportError(err);
   return nl->TypeError();
}


ListExpr MIntHatTypeMap(ListExpr args){
   string err = "mint expected";
   if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),MInt::BasicType()))
         return nl->SymbolAtom(MInt::BasicType());
   ErrorReporter::ReportError(err);
   return nl->TypeError();
}

/*
~RestrictTypeMap~

signatures:
   mint -> mint
   mint x int -> mint

*/
ListExpr restrictTM(ListExpr args){
  string err = "mint [x int] expected";
  int len = nl->ListLength(args);
  if(len!=1 && len!=2){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  if(!nl->IsEqual(nl->First(args),MInt::BasicType())){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  if(len==2 && !nl->IsEqual(nl->Second(args),CcInt::BasicType())){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  return nl->SymbolAtom(MInt::BasicType());
}

/*
~SpeedUpTypeMap~

signatures:
  mpoint x real -> mpoint

*/
ListExpr SpeedUpTypeMap(ListExpr args){
  string err = "mpoint x real expected";
  int len = nl->ListLength(args);
  if(len!=2){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }

  if(nl->IsEqual(nl->First(args),MPoint::BasicType()) &&
     nl->IsEqual(nl->Second(args),CcReal::BasicType())){
      return nl->SymbolAtom(MPoint::BasicType());
  }
  ErrorReporter::ReportError(err);
  return nl->TypeError();
}

/*
~SubMoveTypeMap~

signatures:
  mpoint -> mpoint

*/
ListExpr SubMoveTypeMap(ListExpr args){
  string err = "mpoint x real expected";
  int len = nl->ListLength(args);
  if(len!=2){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }

 if(nl->IsEqual(nl->First(args),MPoint::BasicType()) &&
     nl->IsEqual(nl->Second(args),CcReal::BasicType())){
      return nl->SymbolAtom(MPoint::BasicType());
  }
  ErrorReporter::ReportError(err);
  return nl->TypeError();
}
/*
~Mp2OneMpTypeMap~

signatures:
  mpoint -> mpoint

*/
ListExpr Mp2OneMpTypeMap(ListExpr args){
  string err = "mpoint expected";
  int len = nl->ListLength(args);
  if(len!=3){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }

 if(nl->IsEqual(nl->First(args),MPoint::BasicType()) &&
    nl->IsEqual(nl->Second(args),Instant::BasicType()) &&
    nl->IsEqual(nl->Third(args),Instant::BasicType())){
      return nl->SymbolAtom(MPoint::BasicType());
  }
  ErrorReporter::ReportError(err);
  return nl->TypeError();
}

/*
~P2MpTypeMap~

signatures:
  point -> mpoint

*/
ListExpr P2MpTypeMap(ListExpr args){
  string err = "point x instant x instant x int expected";
  int len = nl->ListLength(args);
  if(len!=4){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }

 if(nl->IsEqual(nl->First(args),Point::BasicType()) &&
    nl->IsEqual(nl->Second(args),Instant::BasicType()) &&
    nl->IsEqual(nl->Third(args),Instant::BasicType()) &&
    nl->IsEqual(nl->Fourth(args),CcInt::BasicType())){
      return nl->SymbolAtom(MPoint::BasicType());
  }
  ErrorReporter::ReportError(err);
  return nl->TypeError();
}

/*
~DistanceTraversedTypeMap~

signatures:
  mpoint [ x geoid ]-> mreal

*/

ListExpr DistanceTraversedOperatorTypeMapping( ListExpr args )
{
  int len = nl->ListLength(args);
  if( (len<1) || (len>2) ){
    return listutils::typeError("Expected mpoint [x geoid].");
  }
  if(!listutils::isSymbol(nl->First(args),MPoint::BasicType())){
    return listutils::typeError("Expected mpoint [x geoid].");
  }
  if((len==2) && (!listutils::isSymbol(nl->Second(args),Geoid::BasicType()))){
    return listutils::typeError("Expected mpoint [x geoid].");
  }
  /*
  Not implemented:
  1- Check the the trajectory is continuos on the spatial space
  (necessary to compute the distance traversed)

  */
  return (nl->SymbolAtom(MReal::BasicType()));
}

/*
1.1.1 Type Mapping Function for Operator ~turns~

The signature is:

----
    mpoint x real x real [ x duration] [ x bool] [ x geoid]
                                   --> stream(tuple((TimeOld instant)
                                                    (TimeNew instant)
                                                    (PosOld point)
                                                    (PosNew point)
                                                    (HeadingOld real)
                                                    (HeadingNew real)
                                                    (HeadingDiff real)))
                                                    @int
----

The appended int argument encodes the usage of optional parameters as a sum of
1: duration present, 2: bool present, 4: geoid present. Valid values are 0-7;


*/

ListExpr TurnsOperatorTypeMapping( ListExpr args ){
  int noargs = nl->ListLength(args);
  bool durationPassed = false;
  bool boolPassed = false;
  bool geoidPassed = false;
  if( (noargs < 3) || (noargs > 6) ){
    return listutils::typeError("Expecting 3, 4, 5 or 6 arguments.");
  }
  if( !listutils::isSymbol(nl->First(args),MPoint::BasicType()) ){
    return listutils::typeError("Expecting 'mpoint' as 1st argument.");
  }
  if( !listutils::isSymbol(nl->Second(args),CcReal::BasicType()) ){
    return listutils::typeError("Expecting 'real' as 2nd argument.");
  }
  if( !listutils::isSymbol(nl->Third(args),CcReal::BasicType()) ){
    return listutils::typeError("Expecting 'real' as 3rd argument.");
  }
  if( noargs>=4 ){ // 4th argument (duratiopn OR bool OR geoid)
    if( listutils::isSymbol(nl->Fourth(args),Duration::BasicType()) ){
      durationPassed = true;
    } else if( listutils::isSymbol(nl->Fourth(args),CcBool::BasicType()) ){
      boolPassed = true;
    } else if( listutils::isSymbol(nl->Fourth(args),Geoid::BasicType()) ){
      geoidPassed = true;
    } else {
      return listutils::typeError("Expecting T in {duration, bool, geoid} as "
                                 "4th argument.");
    }
  }
  if( noargs >= 5 ){ // 5th argument (bool or geoid)
    if( listutils::isSymbol(nl->Fifth(args),CcBool::BasicType()) ){
      boolPassed = true;
    } else if( listutils::isSymbol(nl->Fifth(args),Geoid::BasicType()) ){
      geoidPassed = true;
    } else {
      return listutils::typeError("Expecting T in {bool, geoid} as "
      "5th argument.");
    }
    if( nl->Equal(nl->Fourth(args),nl->Fifth(args)) ){
      return listutils::typeError("Expecting different types for 4th and 5th "
                                 "argument.");
    }
  }
  if( noargs == 6 ){ // 6th argument (geoid)
    if( listutils::isSymbol(nl->Sixth(args),Geoid::BasicType()) ){
      geoidPassed = true;
    } else {
      return listutils::typeError("Expecting geoid as 6th argument.");
    }
    if( nl->Equal(nl->Fourth(args),nl->Sixth(args)) ){
      return listutils::typeError("Expecting different types for 4th and 6th "
      "argument.");
    }
    if( nl->Equal(nl->Fifth(args),nl->Sixth(args)) ){
      return listutils::typeError("Expecting different types for 5th and 6th "
      "argument.");
    }
  }
  int paramcode = 0;
  paramcode += durationPassed?1:0;
  paramcode += boolPassed?2:0;
  paramcode += geoidPassed?4:0;

  NList resTupleType =NList(NList("TimeOld"),
                            NList(Instant::BasicType())).enclose();
  resTupleType.append(NList(NList("TimeNew"),NList(Instant::BasicType())));
  resTupleType.append(NList(NList("PosOld"),NList(Point::BasicType())));
  resTupleType.append(NList(NList("PosNew"),NList(Point::BasicType())));
  resTupleType.append(NList(NList("HeadingOld"),NList(CcReal::BasicType())));
  resTupleType.append(NList(NList("HeadingNew"),NList(CcReal::BasicType())));
  resTupleType.append(NList(NList("HeadingDiff"),NList(CcReal::BasicType())));
  NList resType =
        NList(NList(Symbol::STREAM()),
              NList(NList(Tuple::BasicType()),resTupleType));

  return nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
                            nl->OneElemList(nl->IntAtom(paramcode)),
                            resType.listExpr());
}

/*
Type mapping function ~MappingTimeShift~

It is for the operator ~timeshift~.

*/
ListExpr
MappingTimeShiftTM( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, Duration::BasicType() ) )
    {
      if( nl->IsEqual( arg1, MBool::BasicType() ) )
        return nl->SymbolAtom( MBool::BasicType() );

      if( nl->IsEqual( arg1, MInt::BasicType() ) )
        return nl->SymbolAtom( MInt::BasicType() );

      if( nl->IsEqual( arg1, MReal::BasicType() ) )
        return nl->SymbolAtom( MReal::BasicType() );

      if( nl->IsEqual( arg1, MPoint::BasicType() ) )
        return nl->SymbolAtom( MPoint::BasicType() );
    }
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
1.1.1 Type Mapping Function for Operator ~gridcellevents~

The signature is:

----
{mpoint|upoint} x real x real x real x real x int --> stream(tuple(
          (Cell int)
          (TimeEntered instant)
          (TimeLeft instant)
          (CellPrevious int)
          (CellNext int)))


{mpoint|upoint} x cellgrid2d  --> stream(tuple(
          (Cell int)
          (TimeEntered instant)
          (TimeLeft instant)
          (CellPrevious int)
          (CellNext int)))

----


*/

ListExpr GridCellEventsTypeMapping (ListExpr args)
{
  NList l(args);
  int len = l.length();

  NList resTupleType =NList(NList("Cell"),NList(CcInt::BasicType())).enclose();
  resTupleType.append(NList(NList("TimeEntered"),NList(Instant::BasicType())));
  resTupleType.append(NList(NList("TimeLeft"),NList(Instant::BasicType())));
  resTupleType.append(NList(NList("CellPrevious"),NList(CcInt::BasicType())));
  resTupleType.append(NList(NList("CellNext"),NList(CcInt::BasicType())));
  NList resType =
  NList(NList(Symbol::STREAM()),NList(NList(Tuple::BasicType()),resTupleType));


  if(len==2){ // {upoint, mpoint} x cellgrid2d
    if(!listutils::isSymbol(nl->Second(args),CellGrid2D::BasicType())){
       return listutils::typeError("second argument must be a cellgrid2d");
    }
    if(!listutils::isSymbol(nl->First(args), UPoint::BasicType()) &&
       !listutils::isSymbol(nl->First(args), MPoint::BasicType())){
      return listutils::typeError("first argument must be of type"
                      +UPoint::BasicType()+" or "+MPoint::BasicType());
    }
    return resType.listExpr();
  }

  if( (len != 6) ){
    return l.typeError("Operator 'gridcellevents' expects 6 arguments.");
  }
  if(    !(l.elem(1).isSymbol(MPoint::BasicType()))
      && !(l.elem(1).isSymbol(UPoint::BasicType())) ){
    return l.typeError("Operator 'gridcellevents' expects an "
            +UPoint::BasicType()+" or "
            +MPoint::BasicType()+" as 1st argument.");
  }
  if( !(l.elem(2).isSymbol(CcReal::BasicType())) ){
    return l.typeError("Operator 'gridcellevents' expects a 'real' as 2nd "
                       "argument.");
  }
  if( !(l.elem(3).isSymbol(CcReal::BasicType())) ){
    return l.typeError("Operator 'gridcellevents' expects a 'real' as 3rd "
    "argument.");
  }
  if( !(l.elem(4).isSymbol(CcReal::BasicType())) ){
    return l.typeError("Operator 'gridcellevents' expects a 'real' as 4th "
    "argument.");
  }
  if( !(l.elem(5).isSymbol(CcReal::BasicType())) ){
    return l.typeError("Operator 'gridcellevents' expects a 'real' as 5th "
    "argument.");
  }
  if( !(l.elem(6).isSymbol(CcInt::BasicType())) ){
    return l.typeError("Operator 'gridcellevents' expects an 'int' as 6th "
    "argument.");
  }
  return resType.listExpr();
}

/*
Type mapping function ~SquaredDistance~

It is for the operator ~squareddistance~.

*/
ListExpr
SquaredDistanceTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if((nl->IsEqual( arg1, MPoint::BasicType() ) &&
        nl->IsEqual( arg2, Point::BasicType() ) ) ||
       (nl->IsEqual( arg1, Point::BasicType() ) &&
        nl->IsEqual( arg2, MPoint::BasicType() ) ) ||
       (nl->IsEqual( arg1, MPoint::BasicType() ) &&
        nl->IsEqual( arg2, MPoint::BasicType() ) ))
        return nl->SymbolAtom( MReal::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}


/*
16.2 Selection functions

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

16.2.1 Selection function ~RangeSimpleSelect~

Is used for the ~min~ and ~max~ operators.

*/
int
RangeSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == RInt::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == RReal::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == Periods::BasicType() )
    return 2;

  return -1; // This point should never be reached
}

/*
16.2.1 Selection function ~RangeDualSelect~

Is used for the ~intersects~, ~inside~, ~before~, ~intersection~, ~union~,
and ~minus~ operations.

*/
int
RangeDualSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == RInt::BasicType()
    && nl->SymbolValue( arg2 ) == RInt::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == RReal::BasicType()
    && nl->SymbolValue( arg2 ) == RReal::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == Periods::BasicType()
    && nl->SymbolValue( arg2 ) == Periods::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == CcInt::BasicType()
    && nl->SymbolValue( arg2 ) == RInt::BasicType() )
    return 3;

  if( nl->SymbolValue( arg1 ) == CcReal::BasicType()
    && nl->SymbolValue( arg2 ) == RReal::BasicType() )
    return 4;

  if( nl->SymbolValue( arg1 ) == Instant::BasicType()
    && nl->SymbolValue( arg2 ) == Periods::BasicType() )
    return 5;

  if( nl->SymbolValue( arg1 ) == RInt::BasicType()
    && nl->SymbolValue( arg2 ) == CcInt::BasicType() )
    return 6;

  if( nl->SymbolValue( arg1 ) == RReal::BasicType()
    && nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 7;

  if( nl->SymbolValue( arg1 ) == Periods::BasicType()
    && nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 8;

  return -1; // This point should never be reached
}

/*
16.2.2 Selection function ~TemporalSimpleSelect~

Is used for the ~isempty~ operation.

*/
int
TemporalSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == Instant::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == RInt::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == RReal::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == Periods::BasicType() )
    return 3;

  if( nl->SymbolValue( arg1 ) == UBool::BasicType() )
    return 4;

  if( nl->SymbolValue( arg1 ) == UInt::BasicType() )
    return 5;

  if( nl->SymbolValue( arg1 ) == UReal::BasicType() )
    return 6;

  if( nl->SymbolValue( arg1 ) == UPoint::BasicType() )
    return 7;

//  if( nl->SymbolValue( arg1 ) == MBool::BasicType() )
//    return 8;

//  if( nl->SymbolValue( arg1 ) == MInt::BasicType() )
//    return 9;

//  if( nl->SymbolValue( arg1 ) == MReal::BasicType() )
//    return 10;

//  if( nl->SymbolValue( arg1 ) == MPoint::BasicType() )
//    return 11;

  if( nl->SymbolValue( arg1 ) == IBool::BasicType() )
    return 8;

  if( nl->SymbolValue( arg1 ) == IInt::BasicType() )
    return 9;

  if( nl->SymbolValue( arg1 ) == IReal::BasicType() )
    return 10;

  if( nl->SymbolValue( arg1 ) == IPoint::BasicType() )
    return 11;

  return (-1); // This point should never be reached
}

/*
16.2.2 Selection function ~TemporalDualSelect~

Is used for the ~=~, and ~\#~ operation.

*/
int
TemporalDualSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == Instant::BasicType() &&
    nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == RInt::BasicType()
    && nl->SymbolValue( arg2 ) == RInt::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == RReal::BasicType()
    && nl->SymbolValue( arg2 ) == RReal::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == Periods::BasicType() &&
    nl->SymbolValue( arg2 ) == Periods::BasicType() )
    return 3;

  if( nl->SymbolValue( arg1 ) == IBool::BasicType() &&
      nl->SymbolValue( arg2 ) == IBool::BasicType() )
    return 4;

  if( nl->SymbolValue( arg1 ) == IInt::BasicType()
    && nl->SymbolValue( arg2 ) == IInt::BasicType() )
    return 5;

  if( nl->SymbolValue( arg1 ) == IReal::BasicType()
    && nl->SymbolValue( arg2 ) == IReal::BasicType() )
    return 6;

  if( nl->SymbolValue( arg1 ) == IPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == IPoint::BasicType() )
    return 7;

  return -1; // This point should never be reached
}

int
TemporalDualSelect2( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == MBool::BasicType()
    && nl->SymbolValue( arg2 ) == MBool::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
    && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
    && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == MPoint::BasicType() && nl->SymbolValue
  ( arg2 ) == MPoint::BasicType() )
    return 3;

  return -1; // This point should never be reached
}

/*
16.2.2 Selection function ~TemporalSetValueSelect~

Is used for the ~no\_components~ operation.

*/
int
TemporalSetValueSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == RInt::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == RReal::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == Periods::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == MBool::BasicType() )
    return 3;

  if( nl->SymbolValue( arg1 ) == MInt::BasicType() )
    return 4;

  if( nl->SymbolValue( arg1 ) == MReal::BasicType() )
    return 5;

  if( nl->SymbolValue( arg1 ) == MPoint::BasicType() )
    return 6;

  return (-1); // This point should never be reached
}

/*
16.2.3 Selection function ~IntimeSimpleSelect~

Is used for the ~inst~ and ~val~ operations.

*/
int
IntimeSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == IBool::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == IInt::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == IReal::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == IPoint::BasicType() )
    return 3;

  return -1; // This point should never be reached
}

/*
16.2.3 Selection function ~UIntimeSimpleSelect~

Is used for the ~inst~ and ~val~ operations.

*/
int
UIntimeSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == UInt::BasicType() )
    return 0;

  return -1; // This point should never be reached
}

/*
16.2.3 Selection function ~MovingSimpleSelect~

Is used for the ~deftime~, ~initial~, ~final~, ~inst~, ~val~, ~atinstant~,
~atperiods~  operations.

*/
int
MovingSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == MBool::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == MInt::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == MReal::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == MPoint::BasicType() )
    return 3;

  return -1; // This point should never be reached
}

/*
16.2.3 Selection function ~MovingInstantPeriodsSelect~

Is used for the ~present~ operations.

*/
int
MovingInstantPeriodsSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == MBool::BasicType()
    && nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
    && nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
    && nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == MPoint::BasicType()
    && nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 3;

  if( nl->SymbolValue( arg1 ) == MBool::BasicType()
    && nl->SymbolValue( arg2 ) == Periods::BasicType() )
    return 4;

  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
    && nl->SymbolValue( arg2 ) == Periods::BasicType() )
    return 5;

  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
    && nl->SymbolValue( arg2 ) == Periods::BasicType() )
    return 6;

  if( nl->SymbolValue( arg1 ) == MPoint::BasicType()
    && nl->SymbolValue( arg2 ) == Periods::BasicType() )
    return 7;

  return -1; // This point should never be reached
}

/*
16.2.2 Selection function ~MovingBaseSelect~

Is used for the ~passes~ operations.

*/
int
MovingBaseSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == MBool::BasicType() &&
      nl->SymbolValue( arg2 ) == CcBool::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == MInt::BasicType() &&
      nl->SymbolValue( arg2 ) == CcInt::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == MReal::BasicType() &&
      nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == MPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Point::BasicType() )
    return 3;

  if( nl->SymbolValue( arg1 ) == MPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Region::BasicType() )
    return 4;

  if( nl->SymbolValue( arg1 ) == MPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Rectangle<2>::BasicType() )
    return 5;

  return -1; // This point should never be reached
}


/*
Selection function for the box3d operator

*/

int Box3dSelect(ListExpr args){
  int len = nl->ListLength(args);
  if(len==1){
    ListExpr arg = nl->First(args);
    if(nl->IsEqual(arg,Rectangle<2>::BasicType()))
        return 0;
    if(nl->IsEqual(arg,Instant::BasicType()))
        return 1;
    if(nl->IsEqual(arg,Periods::BasicType()))
        return 3;
  }
  if(len==2){
     if(nl->IsEqual(nl->First(args),Rectangle<2>::BasicType())){
         ListExpr arg2 = nl->Second(args);
         if(nl->IsEqual(arg2,Instant::BasicType()))
            return 2;
         if(nl->IsEqual(arg2,Periods::BasicType()))
            return 4;
     }
  }
  return -1; // when this occurs, the type mapping is not correct

}

/*
Selection function for the bbox operator

*/

int TemporalBBoxSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == UPoint::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == MPoint::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == IPoint::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == Periods::BasicType() )
    return 3;

  if( nl->SymbolValue( arg1 ) == Instant::BasicType() )
    return 4;

  return -1; // This point should never be reached
}

/*
Selection function for the bbox operator

*/

int TemporalMBRangeSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == RInt::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == RReal::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == Periods::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == RBool::BasicType() )
    return 3;

  if( nl->SymbolValue( arg1 ) == RString::BasicType() )
  return 4;

return -1; // This point should never be reached
}

/*
Selection function for the bbox2d operator

*/

int TemporalBBox2dSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == UPoint::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == MPoint::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == IPoint::BasicType() )
    return 2;

  return -1; // This point should never be reached
}

/*
16.2.31 Selection function for ~box2d~

*/
int TemporalBox2dSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == Rectangle<2>::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == Rectangle<3>::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == Rectangle<4>::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == Rectangle<8>::BasicType() )
    return 3;

  return -1; // This point should never be reached
}


/*
16.2.31 Selection function for extdeftime

*/
int ExtDeftimeSelect(ListExpr args){
   // the selection is only dependend on the type of the
   // first argument
   ListExpr arg = nl->First(args);
   if(nl->IsEqual(arg,MBool::BasicType()))
      return 0;
   if(nl->IsEqual(arg,MInt::BasicType()))
      return 1;
   return -1;
}


/*
16.2.32 Selection function for simplify

*/
int SimplifySelect(ListExpr args){
   int len = nl->ListLength(args);
   if(len==2){
       // mpoint x real
       if(nl->IsEqual(nl->First(args),MPoint::BasicType())){
           return 0;
       } else { // mreal x real
           return 2;
       }
   }
   // mpoint x real x duration
   if(len==3) return 1;
   return -1;
}

/*
16.2.32 Selection function for ~integrate~

*/
int IntegrateSelect(ListExpr args){
   ListExpr arg = nl->First(args);
   if(nl->IsEqual(arg,UReal::BasicType())){
       return 0;
   }
   if(nl->IsEqual(arg,MReal::BasicType())){
       return 1;
   }
   return -1; // should never occur

}

/*
16.2.32 Selection function for ~Linearize~

*/
int LinearizeSelect(ListExpr args){
   ListExpr arg = nl->First(args);
   if(nl->IsEqual(arg,UReal::BasicType())){
       return 0;
   }
   if(nl->IsEqual(arg,MReal::BasicType())){
       return 1;
   }
   return -1; // should never occur
}

/*
16.2.32 Selection function for ~Approximate~

*/
int ApproximateSelect(ListExpr args){
  ListExpr res = TypeMapApproximate(args);
  string type = nl->SymbolValue(nl->Third(res));
  if(type == MPoint::BasicType()){
     return 0;
  } else if(type == MReal::BasicType()){
     return 1;
  } else if(type == MInt::BasicType()){
     return 2;
  } else if(type == MBool::BasicType()){
    return 3;
  } else if(type == MString::BasicType()){
    return 4;
  }
  return -1;
}

/*
16.2.32 Selection function for ~min~ and ~max~

*/
int MinMaxSelect(ListExpr args){
   ListExpr arg = nl->First(args);
   if(nl->IsEqual(arg,UReal::BasicType())){
       return 0;
   }
   if(nl->IsEqual(arg,MReal::BasicType())){
       return 1;
   }
   if(nl->IsEqual(arg,MInt::BasicType())){
     return 2;
   }
   return -1; // should never occur
}

/*
16.2.33 Selection function for ~theRange~

*/
int TemporalTheRangeSelect(ListExpr args)
{
  ListExpr arg = nl->First(args);

  if(nl->IsEqual(arg,Instant::BasicType()))
    return 0;
  if(nl->IsEqual(arg,CcInt::BasicType()))
    return 1;
  if(nl->IsEqual(arg,CcBool::BasicType()))
    return 2;
  if(nl->IsEqual(arg,CcReal::BasicType()))
    return 3;
  if(nl->IsEqual(arg,CcString::BasicType()))
    return 4;

  return -1; // should never occur
}

/*

16.2.34 Selection function for ~samplempoint~

*/
int SampleMPointSelect(ListExpr args){
  int len = nl->ListLength(args);
  if(len==2){
     return  0;
  } else if(len==3) {
     return 1;
  } else {
     return 2;
  }
}


/*
16.2.35 SelectionFunction for ~restrict~

*/
int restrictSelect(ListExpr args){
   return nl->ListLength(args)==1?0:1;
}

/*

16.2.36 Selection function for ~squareddistance~

*/
int SquaredDistanceSelect(ListExpr args){
  if(nl->IsEqual(nl->First(args),MPoint::BasicType()) &&
      nl->IsEqual(nl->Second(args),Point::BasicType())){
    return 0;
  }

  if(nl->IsEqual(nl->First(args),Point::BasicType()) &&
      nl->IsEqual(nl->Second(args),Point::BasicType())){
    return 1;
  }

  if(nl->IsEqual(nl->First(args),MPoint::BasicType()) &&
      nl->IsEqual(nl->Second(args),MPoint::BasicType())){
    return 2;
  }
  return -1; // should never occur
}


/*
1.1.1 Selection Function for ~gridcellevents~

*/

int GridCellEventsSelect(ListExpr args){
  if(nl->IsEqual(nl->First(args),UPoint::BasicType())){return 0;}
  if(nl->IsEqual(nl->First(args),MPoint::BasicType())){return 1;}
  return -1;
}
/*
16.3 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are
several value mapping functions, one for each possible combination of input
parameter types.

16.3.1 Value mapping functions of operator ~isempty~

*/
int InstantIsEmpty( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool*)result.addr)->Set( true, !((Instant*)args[0].addr)->IsDefined() );
  return 0;
}

template <class Range>
int RangeIsEmpty( Word* args, Word& result, int message, Word&
                  local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool*)result.addr)->Set( true, ((Range*)args[0].addr)->IsEmpty() );
  return 0;
}

template <class ValueType>
int IntimeIsEmpty( Word* args, Word& result, int message, Word&
                   local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcBool*)result.addr)->
      Set( true, !((Intime<ValueType> *)args[0].addr)->IsDefined() );
  return 0;
}

/*
16.3.2 Value mapping for operator ~IntimeComparePredicates~

template parameter ~OPType~ gives the character of the operator:

----

  0  =,
  1  #,
  2  <,
  3  >,
  4 <=,
  5 >=-

----

*/
template <class ValueType, int OPType>
int IntimeComparePredicates( Word* args, Word& result, int message, Word&
                             local, Supplier s )
{
  result = qp->ResultStorage( s );
  Intime<ValueType>* arg0 = (Intime<ValueType> *)args[0].addr;
  Intime<ValueType>* arg1 = (Intime<ValueType> *)args[1].addr;
  bool boolres = false;
  if( arg0->IsDefined() && arg1->IsDefined() ){
    int comparevalue = arg0->Compare((Attribute*) arg1);
    switch (OPType){
      case 0: // is_equal
        boolres = (comparevalue == 0);
        break;
      case 1: // is_not_equal
        boolres = (comparevalue != 0);
        break;
      case 2: // less_than
        boolres = (comparevalue > 0);
        break;
      case 3: // bigger_than
        boolres = (comparevalue < 0);
        break;
      case 4: // less_or_equal
        boolres = (comparevalue >= 0);
      break;
      case 5: // bigger_or_equal
        boolres = (comparevalue <= 0);
        break;
      default:
        assert( false );
    }
    ((CcBool*)result.addr)->Set( true, boolres );
  } else {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return 0;
}


/*
16.3.2 Value mapping functions of operator $=$ (~equal~)

*/
int
InstantEqual( Word* args, Word& result, int message, Word& local,
 Supplier s )
{
  result = qp->ResultStorage( s );
  Instant* I1 = (Instant*) args[0].addr;
  Instant* I2 = (Instant*) args[1].addr;
  if( !I1->IsDefined() || !I2->IsDefined() ){
    ((CcBool *)result.addr)->Set( false, false ) ;
  } else {
    ((CcBool *)result.addr)->Set( true, (I1->CompareTo(I2)==0));
  }
  return 0;
}

template <class Range>
int RangeEqual( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if(    ((Range*)(args[0].addr))->IsDefined()
      && ((Range*)(args[1].addr))->IsDefined() ){
    ((CcBool*)result.addr)->
        Set( true, *((Range*)args[0].addr) == *((Range*)args[1].addr) );
  } else {
    ((CcBool*)result.addr)->Set( false, false );
  }
  return 0;
}


/*
16.3.3 Value mapping functions of operator $\#$ (~not equal~)

*/
int
InstantNotEqual( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Instant* I1 = (Instant*) args[0].addr;
  Instant* I2 = (Instant*) args[1].addr;
  if( I1->IsDefined() && I2->IsDefined() ){
    ((CcBool *)result.addr)->Set( true, (I1->CompareTo(I2)!=0)) ;
  } else {
    ((CcBool *)result.addr)->Set( false, false) ;
  }
  return 0;
}

template <class Range>
int RangeNotEqual( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if(      ((Range*)args[0].addr)->IsDefined()
        && ((Range*)args[1].addr)->IsDefined() ) {
    ((CcBool*)result.addr)->
        Set( true, *((Range*)args[0].addr) != *((Range*)args[1].addr) );
  } else {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return 0;
}

/*
16.3.4 Value mapping functions of operator $<$

*/
int
InstantLess( Word* args, Word& result, int message, Word& local,
 Supplier s )
{
  result = qp->ResultStorage( s );
  Instant* I1 = (Instant*) args[0].addr;
  Instant* I2 = (Instant*) args[1].addr;
  if(I1->IsDefined() && I2->IsDefined()){
      ((CcBool *)result.addr)->Set( true, (I1->CompareTo(I2)<0)) ;
  } else {
      ((CcBool *)result.addr)->Set( false,false) ;
  }
  return 0;
}

/*
16.3.5 Value mapping functions of operator $<=$

*/
int
InstantLessEqual( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Instant* I1 = (Instant*) args[0].addr;
  Instant* I2 = (Instant*) args[1].addr;
  if(I1->IsDefined() && I2->IsDefined()){
      ((CcBool *)result.addr)->Set( true, (I1->CompareTo(I2)<=0)) ;
  } else {
      ((CcBool *)result.addr)->Set( false,false) ;
  }
  return 0;
}

/*
16.3.6 Value mapping functions of operator $>$

*/
int
InstantGreater( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Instant* I1 = (Instant*) args[0].addr;
  Instant* I2 = (Instant*) args[1].addr;
  if(I1->IsDefined() && I2->IsDefined()){
      ((CcBool *)result.addr)->Set( true, (I1->CompareTo(I2)>0)) ;
  } else {
      ((CcBool *)result.addr)->Set( false,false) ;
  }
  return 0;
}

/*
16.3.7 Value mapping functions of operator $>=$

*/
int
InstantGreaterEqual( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Instant* I1 = (Instant*) args[0].addr;
  Instant* I2 = (Instant*) args[1].addr;
  if(I1->IsDefined() && I2->IsDefined()){
      ((CcBool *)result.addr)->Set( true, (I1->CompareTo(I2)>=0)) ;
  } else {
      ((CcBool *)result.addr)->Set( false,false) ;
  }
  return 0;
}

/*
16.3.8 Value mapping functions of operator ~intersects~

*/
template <class Range>
int RangeIntersects( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if(    ((Range*)args[0].addr)->IsDefined()
      && ((Range*)args[1].addr)->IsDefined() ) {
    ((CcBool*)result.addr)->
        Set(true,((Range*)args[0].addr)->Intersects(*((Range*)args[1].addr)));
  } else {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return 0;
}

/*
16.3.9 Value mapping functions of operator ~inside~

*/
template <class Range>
int RangeInside_rr( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if(    ((Range*)args[0].addr)->IsDefined()
      && ((Range*)args[1].addr)->IsDefined() ) {
    ((CcBool*)result.addr)->
      Set( true, ((Range*)args[0].addr)->Inside( *((Range*)args[1].addr) ) );
  } else {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return 0;
}


template <class Alpha, class Range>
int RangeInside_ar( Word* args, Word& result, int message, Word& local,
  Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Alpha*)args[0].addr)->IsDefined() &&
       ((Range*)args[1].addr)->IsDefined() ){
    ((CcBool*)result.addr)->
        Set( true, ((Range*)args[1].addr)->Contains(*((Alpha*)args[0].addr)) );
  } else {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return 0;
}

/*
16.3.10 Value mapping functions of operator ~before~

*/
template <class Range>
int RangeBefore_rr( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if(    ((Range*)args[0].addr)->IsDefined()
      && ((Range*)args[1].addr)->IsDefined() ) {
    ((CcBool*)result.addr)->
        Set( true, ((Range*)args[0].addr)->Before( *((Range*)args[1].addr) ) );
  } else {
    ((CcBool*)result.addr)->Set( false, false );
  }
  return 0;
}

template <class Alpha, class Range>
int RangeBefore_ar( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if(    ((Range*)args[1].addr)->IsDefined()
      && ((Alpha*)args[0].addr)->IsDefined() ) {
    ((CcBool*)result.addr)->
        Set( true, ((Range*)args[1].addr)->After( *((Alpha*)args[0].addr) ) );
  } else {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return 0;
}

template <class Range, class Alpha>
int RangeBefore_ra( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if(    ((Range*)args[0].addr)->IsDefined()
      && ((Alpha*)args[1].addr)->IsDefined() ) {
    ((CcBool*)result.addr)->
        Set( true, ((Range*)args[0].addr)->Before( *((Alpha*)args[1].addr) ) );
  } else {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return 0;
}

/*
16.3.11 Value mapping functions of operator ~intersection~

*/
template <class Range>
int RangeIntersection( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range*)args[0].addr)->Intersection( *((Range*)args[1].addr),
   (*(Range*)result.addr) );
  return 0;
}

/*
16.3.12 Value mapping functions of operator ~union~

*/
template <class Range>
int RangeUnion( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range*)args[0].addr)->Union( *((Range*)args[1].addr),
   (*(Range*)result.addr) );
  return 0;
}

/*
16.3.13 Value mapping functions of operator ~minus~

*/
template <class Range>
int RangeMinus( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range*)args[0].addr)->Minus( *((Range*)args[1].addr),
   (*(Range*)result.addr) );
  return 0;
}

/*
16.3.14 Value mapping functions of operator ~min~

*/
template <class Range, class Alpha>
int RangeMinimum( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );

  if( ((Range*)args[0].addr)->IsEmpty() )
    ((Alpha*)result.addr)->SetDefined( false );
  else
    ((Range*)args[0].addr)->Minimum( *(Alpha*)result.addr);
  return 0;
}

/*
16.3.15 Value mapping functions of operator ~max~

*/
template <class Range, class Alpha>
int RangeMaximum( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );

  if( ((Range*)args[0].addr)->IsEmpty() )
    ((Alpha*)result.addr)->SetDefined( false );
  else
    ((Range*)args[0].addr)->Maximum( *(Alpha*)result.addr);
  return 0;
}

/*
16.3.16 Value mapping functions of operator ~no\_components~

*/
template <class Range>
int RangeNoComponents( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[0].addr)->IsDefined() )
    ((CcInt *)result.addr)->
        Set( true, ((Range*)args[0].addr)->GetNoComponents() );
  else
    ((CcInt *)result.addr)->Set( false, 0 );
  return 0;
}

/*
16.3.23 Value mapping functions of operator ~trajectory~

*/
int MPointTrajectory( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Line *line = ((Line*)result.addr);
  MPoint *mpoint = ((MPoint*)args[0].addr);
  mpoint->Trajectory( *line );

  return 0;
}

int MBool2MInt( Word* args, Word& result, int message, Word&
 local, Supplier s ){
 result = qp->ResultStorage(s);
 ((MInt*)result.addr)->ReadFrom(*((MBool*) args[0].addr));
 return 0;

}

int MInt2MBool( Word* args, Word& result, int message, Word&
 local, Supplier s ){
 result = qp->ResultStorage(s);
 ((MInt*) args[0].addr)->WriteTo(*((MBool*) result.addr));
 return 0;

}

int MInt2MReal( Word* args, Word& result, int message, Word&
 local, Supplier s ){
 result = qp->ResultStorage(s);
 ((MInt*) args[0].addr)->WriteTo(*((MReal*) result.addr));
 return 0;

}
/*
16.3.29 Value mapping functions of operator ~distance~

*/
int MPointDistance( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((MPoint*)args[0].addr)->Distance( *((Point*)args[1].addr),
   *((MReal*)result.addr) );
  return 0;
}

/*
16.3.29 Value mappings function for the operator ~simplify~

*/
int MPointSimplify(Word* args, Word& result,
                   int message, Word& local,
                   Supplier s){
  result = qp->ResultStorage( s );
  double epsilon = ((CcReal*)args[1].addr)->GetRealval();
  DateTime dur(durationtype);
  ((MPoint*)args[0].addr)->Simplify( epsilon,
                                     *((MPoint*)result.addr),
                                     false,dur );
  return 0;
}

int MPointSimplify2(Word* args, Word& result,
                   int message, Word& local,
                   Supplier s){
  result = qp->ResultStorage( s );
  double epsilon = ((CcReal*)args[1].addr)->GetRealval();
  DateTime* dur = (DateTime*)args[2].addr;
  ((MPoint*)args[0].addr)->Simplify( epsilon,
                                     *((MPoint*)result.addr),
                                     true,*dur );
  return 0;
}

int MRealSimplify(Word* args, Word& result,
                  int message, Word& local,
                  Supplier s){

    result = qp->ResultStorage(s);
    double epsilon = ((CcReal*)args[1].addr)->GetRealval();
    ((MReal*)args[0].addr)->Simplify( epsilon,
                                     *((MReal*)result.addr));
     return 0;
}

/*
16.2.28 Value Mapping function for the operator integrate

*/
template <class mtype>
int Integrate(Word* args, Word& result,
              int message, Word& local,
              Supplier s){
   result = qp->ResultStorage(s);
   mtype* arg = (mtype*) args[0].addr;
   if(!arg->IsDefined()){
    ((CcReal*)result.addr)->Set(false,0);
   } else {
    ((CcReal*)result.addr)->Set(true, arg->Integrate());
   }
   return 0;
}

/*
16.2.28 Value Mapping function for the operator linearize

*/
template <class mtype>
int Linearize(Word* args, Word& result,
              int message, Word& local,
              Supplier s){
   result = qp->ResultStorage(s);
   mtype* arg = (mtype*) args[0].addr;
   mtype* res = (mtype*) result.addr;
   arg->Linearize(*res);
   return 0;
}

int Linearize2(Word* args, Word& result,
              int message, Word& local,
              Supplier s){
   result = qp->ResultStorage(s);
   MReal* arg = (MReal*) args[0].addr;
   MReal* res = (MReal*) result.addr;
   arg->Linearize2(*res);
   return 0;
}

struct Linearize2_ureal_LocalInfo
{
  bool finished;
  int NoOfResults;
  int NoOfResultsDelivered;
  vector<UReal> resultVector;
};
int Linearize2_ureal(Word* args, Word& result,
                     int message, Word& local,
                     Supplier s)
{
    result = qp->ResultStorage(s);
    UReal *arg;
    Linearize2_ureal_LocalInfo *localinfo;

    switch( message )
    {
      case OPEN:
        arg = (UReal*) args[0].addr;
        localinfo = new(Linearize2_ureal_LocalInfo);
        local.setAddr(localinfo);
        localinfo->finished = true;
        localinfo->NoOfResults = 0;
        localinfo->NoOfResultsDelivered = 0;
        localinfo->resultVector.clear();

        if( !arg->IsDefined() )
          return 0;
        else
        {
          UReal res1, res2;
          arg->Linearize(res1, res2);
          if( res1.IsDefined() )
          {
            localinfo->resultVector.push_back(res1);
            localinfo->NoOfResults++;
          }
          if( res2.IsDefined() )
          {
            localinfo->resultVector.push_back(res2);
            localinfo->NoOfResults++;
          }
          localinfo->finished = (localinfo->NoOfResults <= 0);
          return 0;
        }
        return 0;

      case REQUEST:
        if(local.addr == 0)
          return CANCEL;
        localinfo = (Linearize2_ureal_LocalInfo*) local.addr;
        if( localinfo->finished ||
            localinfo->NoOfResultsDelivered >= localinfo->NoOfResults )
        {
          localinfo->finished = true;
          return CANCEL;
        }
        result.setAddr(
            localinfo->resultVector[localinfo->NoOfResultsDelivered].Clone() );
        localinfo->NoOfResultsDelivered++;
        return YIELD;

      case CLOSE:
        if(local.addr)
        {
          localinfo = (Linearize2_ureal_LocalInfo*) local.addr;
          delete localinfo;
          local.setAddr(0);
        }
        return 0;
    } // end switch
    cerr << "Linearize2_ureal(): Unknown message (" << message << ")" << endl;
    return -1; // should not happen
}

/*
16.2.28 Value Mapping Functions for Approximate

*/
template<typename MType, typename UType, typename VType, bool isContinious>
int ApproximateMvalue(Word* args, Word& result,
                      int message, Word& local,
                      Supplier s){

  result = qp->ResultStorage(s);
  MType* res = static_cast<MType*>(result.addr);
  res->Clear();
  int noargs = qp->GetNoSons(s);
  DateTime dur(durationtype);

  int breakAttrIndex = ((CcInt*)args[noargs-1].addr)->GetValue();
  int boolIndex = ((CcInt*)args[noargs-2].addr)->GetValue();
  int durindex  = ((CcInt*)args[noargs-3].addr)->GetValue();
  int index2  = ((CcInt*)args[noargs-4].addr)->GetValue();
  int index1  = ((CcInt*)args[noargs-5].addr)->GetValue();
 

   //cout << "breakAttrIndex = " << breakAttrIndex << endl;
   //cout << "boolIndex = " << boolIndex << endl;
   //cout << " durindex  = " << durindex << endl;
   //cout << " index2  = " << index2 << endl;
   //cout << " index1  = " << index1 << endl;
   
 
   bool split1 = false;
   bool split2 = breakAttrIndex>=0;
   bool makeContinious = isContinious;

   if(boolIndex>=0){
       CcBool* MC = static_cast<CcBool*>(args[boolIndex].addr);
       if(!MC->IsDefined()){
         res->SetDefined(false);
         return 0;
       }
       makeContinious += MC->GetValue();
   }
   if(durindex>=0){
     dur.CopyFrom(static_cast<Attribute*>(args[durindex].addr));
     split1 = true;
   } 
  
  if( !split1 || dur.IsDefined() ){
    res->SetDefined(true);
  } else { // undefined splitting duration parameter --> return UNDEF mpoint
    res->SetDefined(false);
    return 0;
  }
  Word actual;


  VType lastValue,currentValue;
  Instant lastInstant(instanttype),currentInstant(instanttype);
  bool first = true;

  Stream<Tuple> stream(args[0]);
  stream.open();
  Tuple* tuple = stream.request();
  while (tuple!=0) {
    currentValue =  *((VType*)(tuple->GetAttribute(index2)));
    currentInstant = *((Instant*)(tuple->GetAttribute(index1)));
    bool splitHere = false;
    if(split2){
      CcBool* splitHere1 = (CcBool*) tuple->GetAttribute(breakAttrIndex);
      splitHere = splitHere1->IsDefined() && splitHere1->GetBoolval();
    }

    if(currentInstant.IsDefined()){ // ignore undefined instants
      if(currentValue.IsDefined()){
        if(!first && !splitHere){
          // check order of instants - ignore wrong ordered elements
          if(currentInstant>lastInstant){
            if(!(split1 && (currentInstant - lastInstant) > dur )) {
              Interval<Instant> interval(lastInstant, currentInstant,
                                         true ,false);
              if(isContinious){
                UType unit(interval,lastValue,currentValue);
                res->MergeAdd(unit);
              } else {
                UType unit(interval,lastValue,lastValue);
                res->MergeAdd(unit);
              }
            }
            lastValue = currentValue;
            lastInstant = currentInstant;
          }
        } else {
          if(first || lastInstant < currentInstant){
            lastValue = currentValue;
            lastInstant = currentInstant;
          }
          first = false;
        }
      }
    }
    tuple->DeleteIfAllowed();
    tuple=stream.request();
  }

  stream.close();
  return 0;
}

/*
16.2.28 Value Mapping function for the operator min

*/
template <class mtype, class rtype>
int VM_Min(Word* args, Word& result,
              int message, Word& local,
              Supplier s){
   result = qp->ResultStorage(s);
   mtype* arg = (mtype*) args[0].addr;
   bool correct;
   ((rtype*)result.addr)->Set(correct,arg->Min(correct));
   return 0;
}

/*
16.2.28 Value Mapping function for the operator min

*/
template <class mtype, class rtype>
int VM_Max(Word* args, Word& result,
              int message, Word& local,
              Supplier s){
   result = qp->ResultStorage(s);
   mtype* arg = (mtype*) args[0].addr;
   bool correct;
   ((rtype*)result.addr)->Set(correct,arg->Max(correct));
   return 0;
}

/*
16.3.29 Value mapping function for the operator ~breakpoints~

*/
int MPointBreakPoints(Word* args, Word& result,
                   int message, Word& local,
                   Supplier s){
  result = qp->ResultStorage( s );
  DateTime* dur = ((DateTime*)args[1].addr);
  if(qp->GetNoSons(s) == 2){
     ((MPoint*)args[0].addr)->BreakPoints(*((Points*)result.addr),*dur );
  } else {
    CcReal* epsilon = (CcReal*)args[2].addr;
   ((MPoint*)args[0].addr)->BreakPoints(*((Points*)result.addr),
                                        *dur, *epsilon );
  }
  return 0;
}



/*
16.3.29 Value mapping function for the operator ~breaks~

*/
int breaksVM(Word* args, Word& result,
                   int message, Word& local,
                   Supplier s){

  result= qp->ResultStorage(s);
  Periods* res = (Periods*) result.addr;
  MPoint* mp = (MPoint*) args[0].addr;
  DateTime* dur = (DateTime*) args[1].addr;
  CcReal* eps = (CcReal*) args[2].addr;
  mp->Breaks(*res, *dur, *eps);
  return 0;
}



/*
16.3.29 Value mapping function for the operator ~gk~

*/
int gkVM(Word* args, Word& result,
         int message, Word& local,
         Supplier s){
  result = qp->ResultStorage( s );
  MPoint* argmp = static_cast<MPoint*>(args[0].addr);
  CcInt*  zone  = static_cast<CcInt*>(args[1].addr);
  if(!zone->IsDefined()){
    ((MPoint*)result.addr)->SetDefined(false);
  } else {
    argmp->gk(zone->GetValue(), *((MPoint*)result.addr));
  }
  return 0;
}
/*
16.3.29 Value mapping function for the operator ~vertices~

*/
int Vertices(Word* args, Word& result,
                   int message, Word& local,
                   Supplier s){
  result = qp->ResultStorage( s );
  ((MPoint*)args[0].addr)->Vertices(*((Points*)result.addr));
  return 0;
}

/*
16.3.30 Value mapping function for the operator ~delay~

*/
int DelayOperatorValueMapping(ArgVector args, Word& result,
                              int msg, Word& local, Supplier s) {
  bool debugme = false;
  MPoint* pActual = static_cast<MPoint*>(args[0].addr);
  MPoint* pScheduled = static_cast<MPoint*>(args[1].addr);
  Geoid* geoid = (qp->GetNoSons(s) == 3) ? static_cast<Geoid*>(args[2].addr):0;
  MReal* delay= (MReal*) qp->ResultStorage(s).addr;
  MReal* tmp = pScheduled->DelayOperator(pActual,geoid);
  delay->CopyFrom(tmp);
  tmp->DeleteIfAllowed();
  result= SetWord(delay);
  if (debugme) {
    cout.flush();
    delay->Print(cout);
    cout.flush();
  }
  return 0;
// result = qp->ResultStorage(s);
// result.addr=  pScheduled->DelayOperator(pActual);
// return 0;
}

/*
16.3.30 Value mapping function for the operator ~distancetraversed~

*/

int DistanceTraversedOperatorValueMapping(ArgVector args, Word& result,
                                          int msg, Word& local, Supplier s) {
  bool debugme = false;
  MPoint* p = static_cast<MPoint*>(args[0].addr);
  MReal* dist = static_cast<MReal*>(qp->ResultStorage(s).addr);
  Geoid* geoid = (qp->GetNoSons(s) == 1) ? 0:static_cast<Geoid*>(args[1].addr);
  MReal* tmp = p->DistanceTraversed(geoid);
  dist->CopyFrom(tmp);
  tmp->DeleteIfAllowed();
  result = SetWord(dist);
  if (debugme) {
    cout.flush();
    dist->Print(cout);
    cout.flush();
  }
  return 0;
// result = qp->ResultStorage(s);
// result.addr=  pScheduled->DelayOperator(pActual);
// return 0;
}

/*
16.3.31 Value mapping functions of operator ~bbox~

*/

int IPointBBox(Word* args, Word& result, int message, Word& local,
               Supplier s )
{
  result = qp->ResultStorage( s );
  Rectangle<3>  *res = (Rectangle<3>*)  result.addr;
  const Intime<Point> *arg = static_cast<const Intime<Point>*>(args[0].addr);
  const Geoid*  geoid =
                (qp->GetNoSons(s)==2)?static_cast<const Geoid*>(args[1].addr):0;
  Rectangle<2> pbox;
  double min[3], max[3];

  if( !arg->IsDefined() || (geoid && !geoid->IsDefined()) )
  {
    res->SetDefined(false);
  }
  else
  {
    pbox = arg->value.BoundingBox(geoid);
    if( !pbox.IsDefined() || !arg->instant.IsDefined() )
    {
      res->SetDefined(false);
    }
    else
    {
      min[0] = pbox.MinD(0);
      min[1] = pbox.MinD(1);
      min[2] = arg->instant.ToDouble();
      max[0] = min[0];
      max[1] = min[1];
      max[2] = min[2];
      res->Set( true, min, max );
    }
  }
  return 0;
}

/*
Since MPoint is not a subclass of ~SpatialAttribute~, it has no ~BoundingBox()~
function. One could make it inherit from ~SpatialAttribute~, but than one had to
restore all databases and adopt the Add, MergeAdd, +=, -=, etc.

*/
int MPointBBox(Word* args, Word& result, int message, Word& local,
               Supplier s )
{
  result = qp->ResultStorage( s );
  Rectangle<3>* res = (Rectangle<3>*) result.addr;
  const MPoint* arg = static_cast<const MPoint*>(args[0].addr);
  const Geoid*  geoid =
                (qp->GetNoSons(s)==2)?static_cast<const Geoid*>(args[1].addr):0;

  if( !arg->IsDefined() || (arg->GetNoComponents() < 1) )
  { // empty/undefined MPoint --> undef
    res->SetDefined(false);
  }
  else { // return MBR
    *res = arg->BoundingBox(geoid);
  }
  return 0;
}

int MPointBBoxOld(Word* args, Word& result, int message, Word& local,
               Supplier s )
{
  result = qp->ResultStorage( s );
  Rectangle<3>* res = (Rectangle<3>*) result.addr;
  const MPoint* arg = static_cast<const MPoint*>(args[0].addr);
  const Geoid*  geoid =
            (qp->GetNoSons(s)==2)?static_cast<const Geoid*>(args[1].addr):0;
  UPoint uPoint;
  double min[3], max[3];
  Rectangle<3> accubbox;

  if( !arg->IsDefined() || (arg->GetNoComponents() < 1) ||
                                              (geoid && !geoid->IsDefined()) )
  {
    res->SetDefined(false);
  }
  else
  {
    arg->Get( 0, uPoint );
    accubbox = uPoint.BoundingBox();
    min[2] = uPoint.timeInterval.start.ToDouble(); // mintime
    for( int i = 1; i < arg->GetNoComponents(); i++ )
    { // calculate spatial bbox
      arg->Get( i, uPoint );
      accubbox = accubbox.Union( uPoint.BoundingBox(geoid) );
    }
    max[2] = uPoint.timeInterval.end.ToDouble(); // maxtime
    min[0] = accubbox.MinD(0); // minX
    max[0] = accubbox.MaxD(0); // maxX
    min[1] = accubbox.MinD(1); // minY
    max[1] = accubbox.MaxD(1); // maxY
    res->Set( true, min, max );
  }
  return 0;
}

int UPointBBox(Word* args, Word& result, int message, Word& local,
               Supplier s )
{
  result = qp->ResultStorage( s );
  Rectangle<3>* res = (Rectangle<3>*) result.addr;
  const UPoint* arg = static_cast<const UPoint*>(args[0].addr);
  const Geoid*  geoid =
              (qp->GetNoSons(s)==2)?static_cast<const Geoid*>(args[1].addr):0;
  if( !arg->IsDefined() || (geoid && !geoid->IsDefined()) )
  {
    res->SetDefined(false);
  }
  else
  {
    *res = arg->BoundingBox(geoid);
  }
  return 0;
}

int PeriodsBBox(Word* args, Word& result, int message, Word& local,
               Supplier s )
{
  result = qp->ResultStorage( s );
  Rectangle<3>* res = (Rectangle<3>*) result.addr;
  Periods*      arg = (Periods*)      args[0].addr;
  if( !arg->IsDefined() || arg->IsEmpty() )
  {
    res->SetDefined(false);
  }
  else
  {
    double min[3], max[3];
    int64_t zero =0;
    Instant minT(zero);
    Instant maxT(zero);
    arg->Minimum(minT);
    arg->Maximum(maxT);
    min[0] = MINDOUBLE; // minX
    max[0] = MAXDOUBLE; // maxX
    min[1] = MINDOUBLE; // minY
    max[1] = MAXDOUBLE; // maxY
    min[2] = minT.ToDouble(); // min t
    max[2] = maxT.ToDouble(); // max t
    res->Set( true, min, max );
  }
  return 0;
}

int InstantBBox(Word* args, Word& result, int message, Word& local,
               Supplier s )
{
  result = qp->ResultStorage( s );
  Rectangle<3>* res = (Rectangle<3>*) result.addr;
  Instant*      arg = (Instant*)      args[0].addr;
  if( !arg->IsDefined() )
  {
    res->SetDefined(false);
  }
  else
  {
    double min[3], max[3];
    min[0] = MINDOUBLE; // minX
    max[0] = MAXDOUBLE; // maxX
    min[1] = MINDOUBLE; // minY
    max[1] = MAXDOUBLE; // maxY
    min[2] = arg->ToDouble(); // min t
    max[2] = arg->ToDouble(); // max t
    res->Set( true, min, max );
  }
  return 0;
}


template <class Range>
int TempMBRange( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Range* arg = (Range*)args[0].addr;

  if( !arg->IsDefined() || arg->IsEmpty() )
    ((Range*)result.addr)->SetDefined( false );
  else
    ((Range*)args[0].addr)->RBBox( *(Range*)result.addr);
  return 0;
}

/*
16.3.31 Value mapping functions of operator ~bbox2d~

*/

int IPointBBox2d(Word* args, Word& result, int message, Word& local,
                 Supplier s )
{
  result = qp->ResultStorage( s );
  Rectangle<2>  *res = (Rectangle<2>*)  result.addr;
  const Intime<Point> *arg = static_cast<const Intime<Point>*>(args[0].addr);
  const Geoid*  geoid =
                (qp->GetNoSons(s)==2)?static_cast<const Geoid*>(args[1].addr):0;

  if( !arg->IsDefined() || (geoid && !geoid->IsDefined()) )
  {
    res->SetDefined(false);
  }
  else
  {
    *res = arg->value.BoundingBox(geoid);
  }
  return 0;
}

int MPointBBox2d(Word* args, Word& result, int message, Word& local,
                 Supplier s )
{
  result = qp->ResultStorage( s );
  Rectangle<2>* res = (Rectangle<2>*) result.addr;
  const MPoint* arg = static_cast<const MPoint*>(args[0].addr);
  const Geoid*  geoid =
                (qp->GetNoSons(s)==2)?static_cast<const Geoid*>(args[1].addr):0;
  Rectangle<3> accubbox = arg->BoundingBox(geoid);
  if( accubbox.IsDefined() )
  {
    double min[2], max[2];
    min[0] = accubbox.MinD(0); // minX
    max[0] = accubbox.MaxD(0); // maxX
    min[1] = accubbox.MinD(1); // minY
    max[1] = accubbox.MaxD(1); // maxY
    res->Set( true, min, max );
  } else
  {
    res->SetDefined(false);
  }
  return 0;
}

int UPointBBox2d(Word* args, Word& result, int message, Word& local,
               Supplier s )
{
  result = qp->ResultStorage( s );
  Rectangle<2>* res = (Rectangle<2>*) result.addr;
  const UPoint* arg = static_cast<const UPoint*>(args[0].addr);
  const Geoid*  geoid =
                (qp->GetNoSons(s)==2)?static_cast<const Geoid*>(args[1].addr):0;
  Rectangle<3>  tmp;
  double min[2], max[2];

  if( !arg->IsDefined() || (geoid && !geoid->IsDefined()) )
  {
    res->SetDefined(false);
  }
  else
  {
    tmp = arg->BoundingBox(geoid);
    min[0] = tmp.MinD(0); // minX
    max[0] = tmp.MaxD(0); // maxX
    min[1] = tmp.MinD(1); // minY
    max[1] = tmp.MaxD(1); // maxY
    res->Set(true, min, max);
  }
  return 0;
}

/*
16.3.31 Value mapping functions of operator ~translate~

*/
static int
MPointTranslate( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  Word t;
  double dx,dy;
  DateTime* dd;
  UPoint uPoint;
  MPoint* mp, *mpResult;
  CcReal *DX, *DY;

  result = qp->ResultStorage( s );

  mp= (MPoint*)args[0].addr,
  mpResult = (MPoint*)result.addr;
  mpResult->Clear();

  Supplier son = qp->GetSupplier( args[1].addr, 0 );
  qp->Request( son, t );
  dd = (DateTime *)t.addr;

  son = qp->GetSupplier( args[1].addr, 1 );
  qp->Request( son, t );
  DX = (CcReal *)t.addr;

  son = qp->GetSupplier( args[1].addr, 2 );
  qp->Request( son, t );
  DY = (CcReal *)t.addr;

  if( DX->IsDefined() &&
      DY->IsDefined() &&
      dd->IsDefined() &&
      mp->IsDefined()    )
  {
    dx = DX->GetRealval();
    dy = DY->GetRealval();
    mpResult->SetDefined( true );
    mpResult->StartBulkLoad();
    for( int i = 0; i < mp->GetNoComponents(); i++ )
    {
      mp->Get( i, uPoint );
      UPoint aux( uPoint );
      aux.p0.Set( aux.p0.GetX() + dx, aux.p0.GetY() + dy );
      aux.p1.Set( aux.p1.GetX() + dx, aux.p1.GetY() + dy );
      aux.timeInterval.start.Add(dd);
      aux.timeInterval.end.Add(dd);
      mpResult->Add(aux);
    }
    mpResult->EndBulkLoad();
    return 0;
  }
  else
  {
    mpResult->SetDefined( false );
    return 0;
  }
}

/*
16.3.32 Value mapping functions of operator ~theyear~

*/
int TheYear( Word* args, Word& result, int message, Word& local,
 Supplier s )
{ // int --> periods (=range(instant))
  result = qp->ResultStorage( s );
  Periods* pResult = (Periods*)result.addr;
  pResult->Clear();

  if( !((CcInt*)args[0].addr)->IsDefined() ) {
    pResult->SetDefined( false );
    return 0;
  }
  pResult->SetDefined( true );
  int intyear = ((CcInt*)args[0].addr)->GetIntval();

  Instant inst1, inst2;
  inst1.SetType( instanttype ) ;
  inst1.Set( intyear, 1, 1, 0, 0, 0, 0 );

  inst2.SetType( instanttype );
  inst2.Set( intyear + 1, 1, 1, 0, 0, 0, 0 );

  Interval<Instant> timeInterval(inst1, inst2, true, false);

  pResult->StartBulkLoad();
  pResult->Add( timeInterval );
  pResult->EndBulkLoad( false );

  return 0;
}

/*
16.3.33 Value mapping functions of operator ~themonth~

*/
int TheMonth( Word* args, Word& result, int message, Word& local,
 Supplier s )
{
  result = qp->ResultStorage( s );
  Periods *pResult = (Periods*)result.addr;
  pResult->Clear();

  if(    !((CcInt*)args[0].addr)->IsDefined()
      || !((CcInt*)args[1].addr)->IsDefined() ) {
    pResult->SetDefined( false );
    return 0;
  }
  pResult->SetDefined( true );

  int intyear = ((CcInt*)args[0].addr)->GetIntval(),
      intmonth = ((CcInt*)args[1].addr)->GetIntval();

  Instant inst1, inst2;

  inst1.SetType( instanttype );
  inst1.Set( intyear, intmonth, 1, 0, 0, 0, 0 );

  inst2.SetType( instanttype );
  if( intmonth < 12 )
    inst2.Set(intyear, intmonth+1, 1, 0, 0, 0, 0);
  else
    inst2.Set(intyear+1, 1, 1, 0, 0, 0, 0);

  Interval<Instant> timeInterval( inst1, inst2, true, false );

  pResult->StartBulkLoad();
  pResult->Add( timeInterval );
  pResult->EndBulkLoad( false );

  return 0;
}

/*
16.3.34 Value mapping functions of operator ~theday~

*/
int TheDay( Word* args, Word& result, int message, Word& local,
 Supplier s )
{
  result = qp->ResultStorage( s );
  Periods *pResult = (Periods*)result.addr;
  pResult->Clear();

  if(    !((CcInt*)args[0].addr)->IsDefined()
      || !((CcInt*)args[1].addr)->IsDefined()
      || !((CcInt*)args[2].addr)->IsDefined() ) {
    pResult->SetDefined( false );
    return 0;
  }
  pResult->SetDefined( true );

  int intyear = ((CcInt*)args[0].addr)->GetIntval(),
      intmonth = ((CcInt*)args[1].addr)->GetIntval(),
      intday = ((CcInt*)args[2].addr)->GetIntval();

  Instant inst1, inst2,
          oneday( 1, 0, durationtype );

  inst1.SetType( instanttype );
  inst1.Set( intyear, intmonth, intday, 0, 0, 0, 0 );

  inst2.SetType( instanttype );
  inst2 = inst1 + oneday;

  Interval<Instant> timeInterval( inst1, inst2, true, false );

  pResult->StartBulkLoad();
  pResult->Add( timeInterval );
  pResult->EndBulkLoad( false );

  return 0;
}

/*
16.3.35 Value mapping functions of operator ~thehour~

*/
int TheHour( Word* args, Word& result, int message, Word& local,
 Supplier s )
{
  result = qp->ResultStorage( s );
  Periods* pResult = (Periods*)result.addr;
  pResult->Clear();

  if(    !((CcInt*)args[0].addr)->IsDefined()
      || !((CcInt*)args[1].addr)->IsDefined()
      || !((CcInt*)args[2].addr)->IsDefined()
      || !((CcInt*)args[3].addr)->IsDefined() ) {
    pResult->SetDefined( false );
    return 0;
  }
  pResult->SetDefined( true );

  int intyear = ((CcInt*)args[0].addr)->GetIntval(),
      intmonth = ((CcInt*)args[1].addr)->GetIntval(),
      intday = ((CcInt*)args[2].addr)->GetIntval(),
      inthour = ((CcInt*)args[3].addr)->GetIntval();

  Instant inst1, inst2,
          onehour( 0, 1*60*60*1000, durationtype );

  inst1.SetType( instanttype );
  inst1.Set( intyear, intmonth, intday, inthour, 0, 0, 0 );

  inst2 .SetType( instanttype );
  inst2 = inst1 + onehour;

  Interval<Instant> timeInterval( inst1, inst2, true, false );

  pResult->StartBulkLoad();
  pResult->Add( timeInterval );
  pResult->EndBulkLoad( false );

  return 0;
}

/*
16.3.36 Value mapping functions of operator ~theminute~

*/
int TheMinute( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods *pResult = (Periods*)result.addr;
  pResult->Clear();

  if(    !((CcInt*)args[0].addr)->IsDefined()
            || !((CcInt*)args[1].addr)->IsDefined()
            || !((CcInt*)args[2].addr)->IsDefined()
            || !((CcInt*)args[3].addr)->IsDefined()
            || !((CcInt*)args[4].addr)->IsDefined() ) {
    pResult->SetDefined( false );
    return 0;
    }
  pResult->SetDefined( true );

  int intyear = ((CcInt*)args[0].addr)->GetIntval(),
      intmonth = ((CcInt*)args[1].addr)->GetIntval(),
      intday = ((CcInt*)args[2].addr)->GetIntval(),
      inthour = ((CcInt*)args[3].addr)->GetIntval(),
      intminute = ((CcInt*)args[4].addr)->GetIntval();

  Instant inst1, inst2,
          oneminute( 0, 1*60*1000, durationtype );

  inst1.SetType( instanttype );
  inst1.Set( intyear, intmonth, intday, inthour, intminute, 0, 0 );

  inst2 .SetType( instanttype );
  inst2 = inst1 + oneminute;

  Interval<Instant> timeInterval( inst1, inst2, true, false );

  pResult->StartBulkLoad();
  pResult->Add( timeInterval );
  pResult->EndBulkLoad( false );

  return 0;
}

/*
16.3.37 Value mapping functions of operator ~thesecond~

*/
int TheSecond( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods *pResult = (Periods*)result.addr;
  pResult->Clear();

  if(    !((CcInt*)args[0].addr)->IsDefined()
            || !((CcInt*)args[1].addr)->IsDefined()
            || !((CcInt*)args[2].addr)->IsDefined()
            || !((CcInt*)args[3].addr)->IsDefined()
            || !((CcInt*)args[4].addr)->IsDefined()
            || !((CcInt*)args[5].addr)->IsDefined() ) {
    pResult->SetDefined( false );
    return 0;
  }
  pResult->SetDefined( true );

  int intyear = ((CcInt*)args[0].addr)->GetIntval(),
      intmonth = ((CcInt*)args[1].addr)->GetIntval(),
      intday = ((CcInt*)args[2].addr)->GetIntval(),
      inthour = ((CcInt*)args[3].addr)->GetIntval(),
      intminute = ((CcInt*)args[4].addr)->GetIntval(),
      intsecond = ((CcInt*)args[5].addr)->GetIntval();

  Instant inst1, inst2,
          onesecond( 0, 1000, durationtype );

  inst1.SetType( instanttype );
  inst1.Set( intyear, intmonth, intday, inthour, intminute, intsecond, 0 );

  inst2.SetType( instanttype );
  inst2 = inst1 + onesecond;

  Interval<Instant> timeInterval( inst1, inst2, true, false );

  pResult->StartBulkLoad();
  pResult->Add( timeInterval );
  pResult->EndBulkLoad( false );

  return 0;
}

/*
16.3.38 Value mapping functions of operator ~theperiod~

*/
int ThePeriod( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods *pResult = (Periods*)result.addr,
          *range1 = ((Periods*)args[0].addr),
          *range2 = ((Periods*)args[1].addr);

  pResult->Clear();
  if( !range1->IsDefined() || !range2->IsDefined() ){
    pResult->SetDefined( false );
    return 0;
  }
  pResult->SetDefined( true );
  if(range1->IsEmpty() && range2->IsEmpty()){
     return 0;  // return empty periods value
  }


  // range 1 is empty 
  if(range1->IsEmpty()){
    Interval<Instant> first;
    Interval<Instant> last;
    range2->Get(0,first);
    range2->Get(range2->GetNoComponents()-1,last);
    Interval<Instant> iv(first.start,last.end,first.lc,last.rc);
    pResult->StartBulkLoad();
    pResult->Add( iv );
    pResult->EndBulkLoad( false, true );
    return 0;
  }
  // symmetric case , range 2 is empty
  if(range2->IsEmpty()){
    Interval<Instant> first;
    Interval<Instant> last;
    range1->Get(0,first);
    range1->Get(range1->GetNoComponents()-1,last);
    Interval<Instant> iv(first.start,last.end,first.lc,last.rc);
    pResult->StartBulkLoad();
    pResult->Add( iv );
    pResult->EndBulkLoad( false, true );
    return 0;
  }

  // normal case both intervals are defined and non empty
  Interval<Instant> first1;
  Interval<Instant> last1;
  range1->Get(0,first1);
  range1->Get(range1->GetNoComponents()-1,last1);
  Interval<Instant> first2;
  Interval<Instant> last2;
  range2->Get(0,first2);
  range2->Get(range2->GetNoComponents()-1,last2);

  Instant start(datetime::instanttype);
  bool lc = false;
  if(first1.start < first2.start){
    start = first1.start;
    lc = first1.lc;
  } else if(first1.start > first2.start){
     start = first2.start;
     lc = first2.lc;
  } else { // equal start instants
     start = first1.start;
     lc = first1.lc || first2.lc;
  }

  Instant end(datetime::instanttype);
  bool rc = false;
  if( last1.end > last2.end){
     end = last1.end;
     rc = last1.rc;
  } else if(last1.end < last2.end){
     end = last2.end;
     rc = last2.rc;
  } else { // equal end
     end = last2.end;
     rc = last1.rc || last2.rc;
  }
  Interval<Instant> iv(start,end,lc,rc);
  pResult->StartBulkLoad();
  pResult->Add( iv );
  pResult->EndBulkLoad( false, true );
  return 0;
}

/*
16.3.39 Value Mappings function for box3d

*/

int Box3d_rect( Word* args, Word& result, int message, Word&
 local, Supplier s ){
   result = qp->ResultStorage(s);
   Rectangle<3>* res = (Rectangle<3>*) result.addr;
   Rectangle<2>* arg = (Rectangle<2>*) args[0].addr;
   if( !arg->IsDefined() )
   {
     res->SetDefined(false);
   }
   else
   {
     double min[3];
     double max[3];
     min[0] = arg->MinD(0);
     min[1] = arg->MinD(1);
     min[2] = MINDOUBLE;
     max[0] = arg->MaxD(0);
     max[1] = arg->MaxD(1);
     max[2] = MAXDOUBLE;
     res->Set(true,min,max);
   }
   return 0;
}

int Box3d_instant( Word* args, Word& result, int message, Word&
 local, Supplier s ){
   result = qp->ResultStorage(s);
   Rectangle<3>* res = (Rectangle<3>*) result.addr;
   Instant* arg = (Instant*) args[0].addr;
   if( !arg->IsDefined() )
   {
     res->SetDefined(false);
   }
   else
   {
     double min[3];
     double max[3];
     double v = arg->ToDouble();
     min[0] = MINDOUBLE;
     min[1] = MINDOUBLE;
     min[2] = v;
     max[0] = MAXDOUBLE;
     max[1] = MAXDOUBLE;
     max[2] = v;
     res->Set(true,min,max);
   }
   return 0;
}


int Box3d_rect_instant( Word* args, Word& result, int message,
 Word& local, Supplier s ){
   result = qp->ResultStorage(s);
   Rectangle<3>* res  = (Rectangle<3>*) result.addr;
   Rectangle<2>* arg1 = (Rectangle<2>*) args[0].addr;
   Instant*      arg2 = (Instant*)      args[1].addr;
   if( !arg1->IsDefined() || !arg2->IsDefined() )
   {
     res->SetDefined(false);
   }
   else
   {
     double v = arg2->ToDouble();
     double min[3];
     double max[3];
     min[0] = arg1->MinD(0);
     min[1] = arg1->MinD(1);
     min[2] = v;
     max[0] = arg1->MaxD(0);
     max[1] = arg1->MaxD(1);
     max[2] = v;
     res->Set(true,min,max);
   }
   return 0;
}


int Box3d_periods( Word* args, Word& result, int message, Word&
 local, Supplier s ){
   result = qp->ResultStorage(s);
   Rectangle<3>* res = (Rectangle<3>*) result.addr;
   Periods*      per = (Periods*)      args[0].addr;
   if( per->IsEmpty() ) // includes undefined per
   {
     res->SetDefined(false);
   }
   else
   {
     Instant i1(0,0,instanttype), i2(0,0,instanttype);
     per->Minimum(i1);
     double v1 = i1.ToDouble();
     per->Maximum(i2);
     double v2 = i2.ToDouble();
     double min[3];
     double max[3];
     min[0] = MINDOUBLE;
     min[1] = MINDOUBLE;
     min[2] = v1;
     max[0] = MAXDOUBLE;
     max[1] = MAXDOUBLE;
     max[2] = v2;
     res->Set(true,min,max);
   }
   return 0;
}

int Box3d_rect_periods( Word* args, Word& result, int message,
 Word& local, Supplier s ){
   result = qp->ResultStorage(s);
   Rectangle<3>* res = (Rectangle<3>*) result.addr;
   Rectangle<2>* arg1 = (Rectangle<2>*) args[0].addr;
   Periods*      arg2 = (Periods*)      args[1].addr;
   if( !arg1->IsDefined() || arg2->IsEmpty() ) // includes undefined arg2
   {
     res->SetDefined(false);
   }
   else
   {
     Instant i1, i2;
     arg2->Minimum(i1);
     double v1 = i1.ToDouble();
     arg2->Maximum(i2);
     double v2 = i2.ToDouble();
     double min[3];
     double max[3];
     min[0] = arg1->MinD(0);
     min[1] = arg1->MinD(1);
     min[2] = v1;
     max[0] = arg1->MaxD(0);
     max[1] = arg1->MaxD(1);
     max[2] = v2;
     res->Set(true,min,max);
   }
   return 0;
}

/*
16.3.40 Value Mapping function for box2d

*/

template <int dim>
int TemporalBox2d( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  assert(dim >= 2);
  result = qp->ResultStorage(s);
  Rectangle<2>* res = (Rectangle<2>*) result.addr;
  Rectangle<dim>* arg = (Rectangle<dim>*) args[0].addr;

  if( !arg->IsDefined() )
  {
    res->SetDefined(false);
  }
  else
  {
    double min[2], max[2];
    min[0] = arg->MinD(0);
    min[1] = arg->MinD(1);
    max[0] = arg->MaxD(0);
    max[1] = arg->MaxD(1);
    res->Set( true, min, max );
  }
  return 0;
}

/*
16.3.41 Value Mapping function for ExtDefTime

*/
template <class Unit,class Alpha>
int TemporalExtDeftime( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  result = qp->ResultStorage(s);
  Mapping<Unit,Alpha>* arg1 = (Mapping<Unit,Alpha>*) args[0].addr;
  Unit* arg2 = (Unit*) args[1].addr;
  Mapping<Unit,Alpha>* res = (Mapping<Unit,Alpha>*) result.addr;
  arg1->ExtendDefTime(*arg2,*res);
  return 0;
}

/*
16.3.42 Value Mapping function for ~at~

Here, we only implement the VM for ~mreal x real [->] mreal~. All other
VMs are implemented using a template function from ~TemporalAlgebra.h~.

Since it would use the unimplementable ~UReal::At(...)~
method, we implement this version using ~UReal::AtValue(...)~.

*/

int MappingAt_MReal_CcReal( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  result = qp->ResultStorage(s);
  ((MReal*) args[0].addr)->
      AtValue( *((CcReal*) args[1].addr), *((MReal*) result.addr) );
  return 0;
}

/*
16.3.43 Value mapping function for operator ~theRange~

*/

template<class T>
int TemporalTheRangeTM( Word* args, Word& result, int message, Word&
    local, Supplier s )
{
  result = qp->ResultStorage( s );
  Range<T> *pResult = (Range<T> *)result.addr;
  T      *t1 = ((T*)args[0].addr),
         *t2 = ((T*)args[1].addr);
  CcBool *b1 = ((CcBool*)args[2].addr),
         *b2 = ((CcBool*)args[3].addr);

  pResult->Clear();

  if( !t1->IsDefined() || !t2->IsDefined() ||
      !b1->IsDefined() || !b2->IsDefined()
    )
  {
    pResult->SetDefined(false); // not effective by now
  }
  else
  {
    pResult->SetDefined(true);
    Interval<T> interval;
    int cmp = t1->Compare( t2 );
    bool bb1 = b1->GetBoolval(),
         bb2 = b2->GetBoolval();

    if( t1->Adjacent( t2 ) && !bb1 && !bb2 )
    {
      pResult->SetDefined( false );
      return 0;
    }

    if( cmp < 0 )
    { // t1 < t2
    interval = Interval<T>( *t1, *t2, bb1, bb2 );
    }
    else if ( cmp > 0 )
    { // t1 > t2, swap arguments
      interval = Interval<T>( *t2, *t1, bb2, bb1 );
    }
    else
    { // t1 == t2, enforce left and right closedness
      if( !bb1 || !bb2 )
      {
        pResult->SetDefined( false );
        return 0;
      }
      else
      {
        interval = Interval<T>( *t1, *t1, true, true );
      }
    }
    pResult->StartBulkLoad();
    pResult->Add( interval );
    pResult->EndBulkLoad( false );
  }
  return 0;
}


/*
16.3.44 Value mapping function for operator ~translateappend~

*/

int TranslateAppendVM( Word* args, Word& result, int message, Word&
    local, Supplier s ){

   result = qp->ResultStorage(s);
   MPoint* res = (MPoint*) result.addr;
   res->CopyFrom( (MPoint*) args[0].addr);
   res->TranslateAppend(*((MPoint*) args[1].addr),
                        *((DateTime*) args[2].addr));
   return 0;

}

/*
16.3.45 Value mapping function for ~translateAppendS~

*/

int TranslateAppendSVM(Word* args, Word& result,
                      int message, Word& local,
                      Supplier s){
   result = qp->ResultStorage(s);
   MPoint* res = (MPoint*) result.addr;
   int index = ((CcInt*)args[3].addr)->GetIntval();

   DateTime* duration = (DateTime*) args[2].addr;

   res->Clear();
   res->SetDefined(true);
   Word current;
   MPoint* mpoint=NULL;
   qp->Open(args[0].addr);
   qp->Request(args[0].addr, current);
   while (qp->Received(args[0].addr)) {
      Tuple* tuple = (Tuple*)current.addr;
      mpoint =  (MPoint*)(tuple->GetAttribute(index));
      res->TranslateAppend(*mpoint,*duration);
      tuple->DeleteIfAllowed();
      qp->Request(args[0].addr, current);
   }
   qp->Close(args[0].addr);
   return 0;
}

/*
16.3.44 Value mapping function for operator ~reverse~

*/

int ReverseVM( Word* args, Word& result, int message, Word&
    local, Supplier s ){

   result = qp->ResultStorage(s);
   MPoint* res = (MPoint*) result.addr;
   ((MPoint*)args[0].addr)->Reverse(*res);
   return 0;

}

/*
16.3.45 Value mapping function for operator ~samplempoint~

*/
template <bool keepEndPoint, bool exactPath>
int SampleMPointVM( Word* args, Word& result, int message,
                    Word& local, Supplier s ){


   result = qp->ResultStorage(s);
   MPoint* res = (MPoint*) result.addr;
   res->Clear();
   DateTime* duration = (DateTime*) args[1].addr;
   bool ke = false; // keep end point
   bool ep = false; // exact path
   if(keepEndPoint){
     CcBool* KE = (CcBool*) args[2].addr;
     if(!KE->IsDefined()){
        res->SetDefined(false);
        return true;
     } else {
       ke = KE->GetBoolval();
     }
   }
   if(exactPath){
     CcBool* EP = (CcBool*) args[3].addr;
     if(!EP->IsDefined()){
        res->SetDefined(false);
        return 0;
     } else {
        ep = EP->GetBoolval();
     }
   }

  ((MPoint*)args[0].addr)->Sample(*duration,*res,ke,ep);

   return 0;
}


/*
16.3.46 Value mapping for the gps operator

*/

class GPSLI{
  public:

/*
~Constructor~

*/
      GPSLI(const MPoint* mp,
            const DateTime* duration,
            ListExpr tupleType){

         if(!mp->IsDefined() || !duration->IsDefined()){
             this->tupleType=0;
             this->size=0;
             this->unit=0;
         } else {
            this->theMPoint = mp;
            this->duration = duration;
            this->size = mp->GetNoComponents();
            this->unit = 0;
            this->tupleType = new TupleType(nl->Second(tupleType));
            if(size>0){
               UPoint up;
               mp->Get(0,up);
               instant = up.timeInterval.start;
            }
         }
      }

/*
~Destructor~

*/

      ~GPSLI(){
         if(tupleType){
           tupleType->DeleteIfAllowed();
         }
         tupleType=0;
      }

/*
~NextTuple~

*/
      Tuple* NextTuple(){
        // search the unit containing instant
       bool done = false;
       UPoint up;
       while(unit<size  && !done){
         theMPoint->Get(unit,up);
         if((instant< up.timeInterval.end) ||
            (instant==up.timeInterval.end && up.timeInterval.rc) ){
             done = true;
         } else {
             unit++;
         }
       }
       if(!done){
          return 0;
       }
       if(instant<up.timeInterval.start){ // gap
          instant = up.timeInterval.start;
       }
       Point p;
       up.TemporalFunction(instant,p,true);
       // construct the result from instant,p
       Tuple* res = new Tuple(tupleType);
       res->PutAttribute(0,instant.Clone());
       res->PutAttribute(1,p.Clone());
       instant += *duration;
       return res;
      }

  private:
     const MPoint* theMPoint;
     const DateTime* duration;
     Instant instant;
     int size;
     int unit;
     TupleType* tupleType;


};


int GPSVM( Word* args, Word& result, int message,
                    Word& local, Supplier s ){
  Tuple* t;
  GPSLI* li;
  switch(message){
       case OPEN:
             local.setAddr(new GPSLI((MPoint*)args[0].addr,
                                       (DateTime*)args[1].addr,
                                       GetTupleResultType(s)));
             return 0;
       case REQUEST:
              if(!local.addr){
                return CANCEL;
              }
              li = (GPSLI*) local.addr;
              t = li->NextTuple();
              result.setAddr(t);
              if(t){
                 return YIELD;
              } else {
                 return CANCEL;
              }
       case CLOSE:
              if(local.addr)
              {
                li = (GPSLI*) local.addr;
                delete li;
                local.setAddr(0);
              }
              return 0;
  }
  return 0;
}


/*
16.3.47 ~Disturb~

*/
int DisturbVM(Word* args, Word& result, int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   MPoint* res  = static_cast<MPoint*>(result.addr);
   MPoint* mp   = static_cast<MPoint*>(args[0].addr);
   CcReal* md   = static_cast<CcReal*>(args[1].addr);
   CcReal* mdps = static_cast<CcReal*>(args[2].addr);
   res->Clear();
   if( !mp->IsDefined() || !md->IsDefined() || !mdps->IsDefined() ){
     res->SetDefined(false);
   } else {
    mp->Disturb(*res, md->GetRealval(), mdps->GetRealval());
   }
   return 0;
}

/*
16.3.48 ~Length~

*/
int LengthVM(Word* args, Word& result, int message,
              Word& local, Supplier s){

   result = qp->ResultStorage(s);
   CcReal* res = (CcReal*)result.addr;
   MPoint* mp = (MPoint*) args[0].addr;
   if(!mp->IsDefined()){
      res->Set(false, 0.0);
      return 0;
  }
  if(qp->GetNoSons(s)==2){ // variant using (LON,LAT)-coordinates
    CcString* geoidCcStr = static_cast<CcString*>(args[1].addr);
    if(!geoidCcStr->IsDefined()){
      res->Set(false, 0.0);
      return 0;
    }
    string geoidstr = geoidCcStr->GetValue();
    bool valid = false;
    Geoid::GeoidName gn = Geoid::getGeoIdNameFromString(geoidstr,valid);
    if(!valid){
      res->Set(false, 0.0);
      return 0;
    }
    Geoid geoid(gn);
    res->Set(true, mp->Length(geoid,valid));
    res->SetDefined(valid);
  } else { // normal variant using (X,Y)-coordinates
      res->Set(true, mp->Length());
  }
  return 0;
}



/*
16.3.49 Some specialized valuemappings for type MPoint

*/

// specialized case MPoint: use MBR
// All other cases: template<>MappingNotEqual()
int MPointNotEqual( Word* args, Word& result,
                    int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if(    ((MPoint*)args[0].addr)->IsDefined()
      && ((MPoint*)args[1].addr)->IsDefined() ) {
    ((CcBool*)result.addr)->
        Set( true, *((MPoint*)args[0].addr) != *((MPoint*)args[1].addr) );
   } else {
     ((CcBool*)result.addr)->Set( false, false );
   }
  return 0;
}

// specialized case MPoint: use MBR
// All other cases: template<>MappingEqual()
int MPointEqual( Word* args, Word& result,
                 int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if(    ((MPoint*)args[0].addr)->IsDefined()
      && ((MPoint*)args[1].addr)->IsDefined() ) {
    ((CcBool*)result.addr)->
        Set( true, *((MPoint*)args[0].addr) == *((MPoint*)args[1].addr) );
  } else {
    ((CcBool*)result.addr)->Set( false, false );
  }
  return 0;
}

// specialized case MPoint: use MBR
// All other cases: template<>MappingPresent_i()
int MPointPresent_i( Word* args, Word& result,
                     int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MPoint *m = ((MPoint*)args[0].addr);
  Instant* inst = ((Instant*)args[1].addr);

  if( !inst->IsDefined() || !m->IsDefined())
    ((CcBool *)result.addr)->Set( false, false );
  else
    ((CcBool *)result.addr)->Set( true, m->Present( *inst ) );
  return 0;
}

// specialized case MPoint: use MBR
// All other cases: template<>MappingPresent_p()
int MPointPresent_p( Word* args, Word& result,
                         int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MPoint *m = ((MPoint*)args[0].addr);
  Periods* periods = ((Periods*)args[1].addr);

  if( periods->IsEmpty() )
    ((CcBool *)result.addr)->Set( false, false );
  else
    ((CcBool *)result.addr)->Set( true, m->Present( *periods ) );
  return 0;
}

int MPointAtInstant( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MPoint* mp = ((MPoint*)args[0].addr);
  Instant* inst = (Instant*) args[1].addr;
  Intime<Point>* pResult = (Intime<Point>*)result.addr;

  mp->AtInstant(*inst, *pResult);
  return 0;
}

int MPointAtPeriods( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MPoint* mp = ((MPoint*)args[0].addr);
  MPoint* pResult = (MPoint*)result.addr;
  Periods* per = (Periods*)args[1].addr;

  mp->AtPeriods(*per,*pResult);
  return 0;
}


int MPointEqualize( Word* args, Word& result, int message,
                          Word& local, Supplier s ) {

   result = qp->ResultStorage(s);
   MPoint* res = static_cast<MPoint*>(result.addr);
   res->Clear();
   MPoint* p = static_cast<MPoint*>(args[0].addr);
   CcReal* eps = static_cast<CcReal*>(args[1].addr);
   CcBool skipUnits(true,false);
   if(qp->GetNoSons(s)==3){
      skipUnits = * (static_cast<CcBool*>(args[2].addr));
   }

   if(!p->IsDefined() || !eps->IsDefined() || !skipUnits.IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   p->EqualizeUnitsSpatial(eps->GetValue(),*res,skipUnits.GetBoolval());
   return 0;
}
int MIntHat( Word* args, Word& result, int message,
                          Word& local, Supplier s ) {
   MInt* mint = (MInt*)args[0].addr;
   result = qp->ResultStorage(s);
   mint->Hat(*(MInt*)result.addr);
   return 0;
}


int restrictVM1( Word* args, Word& result, int message,
                          Word& local, Supplier s ) {
   result = qp->ResultStorage(s);
   MInt* res = static_cast<MInt*>(result.addr);
   MInt* arg = static_cast<MInt*>(args[0].addr);
   arg->Restrict(*res);
   return 0;
}

int restrictVM2( Word* args, Word& result, int message,
                          Word& local, Supplier s ) {
   result = qp->ResultStorage(s);
   MInt* res = static_cast<MInt*>(result.addr);
   MInt* arg1 = static_cast<MInt*>(args[0].addr);
   CcInt* arg2 = static_cast<CcInt*>(args[1].addr);
   if(!arg2->IsDefined()){
     res->Clear();
     res->SetDefined(false);
     return 0;
   }
   arg1->Restrict(*res,true,arg2->GetIntval());
   return 0;
}

int SpeedUpVM( Word* args, Word& result, int message,
                          Word& local, Supplier s ) {
   result = qp->ResultStorage(s);
   MPoint* res = static_cast<MPoint*>(result.addr);
   MPoint* arg1 = static_cast<MPoint*>(args[0].addr);
   CcReal* arg2 = static_cast<CcReal*>(args[1].addr);
   res->Clear();
   if(!arg1->IsDefined() || !arg2->IsDefined() ){
     res->SetDefined(false);
     return 0;
   }
   if(AlmostEqual(arg2->GetRealval(),0.0)){
      *res = *arg1;
      return 0;
   }
   res->SetDefined( true );

   UPoint up;
   UPoint last;
   UPoint* cur;
   res->StartBulkLoad();

   double factor = 1.0/arg2->GetRealval();
   for(int i = 0;i < arg1->GetNoComponents();i++){
    arg1->Get(i,up);
    if(i == 0){
      last = up;
      last.timeInterval.end =
            (last.timeInterval.end-last.timeInterval.start)*factor+
            last.timeInterval.start;
      res->MergeAdd(last);

    }else{
      cur = new UPoint(up);
      cur->timeInterval.start = last.timeInterval.end;
      cur->timeInterval.end = cur->timeInterval.start +
        (up.timeInterval.end - up.timeInterval.start)*factor;
      res->MergeAdd(*cur);
      last = *cur;
      delete cur;
    }
   }
   res->EndBulkLoad();
   return 0;
}

/*
ValueMapping for operator ~avg\_speed~

*/

int Avg_SpeedVM( Word* args, Word& result, int message,
                          Word& local, Supplier s ) {
   result = qp->ResultStorage(s);
   CcReal* res = static_cast<CcReal*>(result.addr);
   MPoint* arg1 = static_cast<MPoint*>(args[0].addr);
   if(!arg1->IsDefined() || arg1->IsEmpty()){
     res->Set(false, 0.0);
     return 0;
   }
   Geoid* geoidptr = 0;
   bool valid = true;
   if(qp->GetNoSons(s)==2){ // setting up geoid for (LON,LAT)-variant
     CcString* geoidCcStr = static_cast<CcString*>(args[1].addr);
     if(!geoidCcStr->IsDefined()){
      res->Set(false, 0.0);
      return 0;
     }
     string geoidstr = geoidCcStr->GetValue();
     Geoid::GeoidName gn = Geoid::getGeoIdNameFromString(geoidstr,valid);
     if(!valid){
      res->Set(false, 0.0);
      return 0;
     }
     geoidptr = new Geoid(gn);
  }

   double length = 0;
   DateTime totaltime(0,0,durationtype);
   UPoint up;
   for(int i = 0; valid && (i<arg1->GetNoComponents()); i++){
    arg1->Get(i,up);
    totaltime += (up.timeInterval.end - up.timeInterval.start);
    if(geoidptr) { // (LON,LAT)
      length += up.p0.DistanceOrthodrome(up.p1, *geoidptr, valid);
    } else {       // (X,Y)
      length += up.p0.Distance(up.p1);
    }
   }
   res->Set(valid,length/(totaltime.millisecondsToNull()/ 1000.0));
   if(geoidptr){
     delete geoidptr;
     geoidptr = 0;
   }
   return 0;
}

int SubMoveVM( Word* args, Word& result, int message,
                          Word& local, Supplier s ) {
   result = qp->ResultStorage(s);
   MPoint* res = static_cast<MPoint*>(result.addr);
   MPoint* arg1 = static_cast<MPoint*>(args[0].addr);
   CcReal* arg2 = static_cast<CcReal*>(args[1].addr);
   res->Clear();
   if(!arg1->IsDefined() || arg2->GetRealval() <= 0.0 ||
     arg1->GetNoComponents() == 0){
     res->SetDefined( false );
     return 0;
   }
   res->SetDefined( true );
   if(arg2->GetRealval() >= 1.0){
      res->StartBulkLoad();
      UPoint up;
      for(int i = 0;i < arg1->GetNoComponents();i++){
        arg1->Get(i,up);
        res->Add(up);
      }
      res->EndBulkLoad( true, true );
      return 0;
   }
   UPoint up1;
   UPoint up2;
   UPoint* cur;
   res->StartBulkLoad();
   double factor = arg2->GetRealval();
   arg1->Get(0,up1);
   arg1->Get(arg1->GetNoComponents()-1,up2);
   Instant dt = (up2.timeInterval.end - up1.timeInterval.start)*factor;
   srand(time(0));
   int pos = rand() % arg1->GetNoComponents();
   assert(pos < arg1->GetNoComponents());
   arg1->Get(pos,up1);
   Instant enddt = up1.timeInterval.start + dt;
   cur = new UPoint(up1);
   if(enddt <= cur->timeInterval.end){
    Point p1;
    cur->TemporalFunction(enddt,p1,true);
    assert(p1.IsDefined());
    cur->timeInterval.end = cur->timeInterval.start+dt;
    cur->p1 = p1;
    res->Add(*cur);
   }else{
    res->Add(*cur);
    UPoint* up;
    for(pos++;pos < arg1->GetNoComponents();pos++){
      arg1->Get(pos,up1);
      if(up1.timeInterval.end < enddt){
        res->Add(up1);
      }else{
        up = new UPoint(up1);
        Point p1;
        up->TemporalFunction(enddt,p1,true);
        assert(p1.IsDefined());
        up->timeInterval.end = enddt;
        up->p1 = p1;
        res->Add(*up);
        delete up;
        break;
      }
    }
  }
  delete cur;
  res->EndBulkLoad(true,true);
  return 0;
}

int Mp2OneMpVM( Word* args, Word& result, int message,
                          Word& local, Supplier s ) {

   result = qp->ResultStorage(s);
   MPoint* res = static_cast<MPoint*>(result.addr);
   MPoint* arg = static_cast<MPoint*>(args[0].addr);
   Instant* start = (Instant*)args[1].addr;
   Instant* end = (Instant*)args[2].addr;
   res->Clear();
   if(    !arg->IsDefined()   || arg->GetNoComponents() == 0
       || !start->IsDefined() || !end->IsDefined() || (*end < *start) ){
     res->SetDefined(false);
     return 0;
   }
   res->SetDefined(true);
   res->StartBulkLoad();

   srand(time(0)+counter);
   int pos = rand() % arg->GetNoComponents();
   assert( 0<= pos );
   assert( pos < arg->GetNoComponents() );

   UPoint up;
   arg->Get(pos,up);
   UPoint cur(up);

   double x = (cur.p0.GetX() + counter); // XRIS: NONSENSE here!
   double y = (cur.p0.GetY() + counter); // XRIS: NONSENSE here!

   cerr << "WARNING: " << __PRETTY_FUNCTION__
        << " called. This functions does nonsense! Please correct!" << endl;

   Point p(true,x,y);
   cur.timeInterval.start = *start;
   cur.timeInterval.end = *end;
   cur.p0 = p;
   cur.p1 = p;
   res->Add(cur);
   counter++;
   res->EndBulkLoad(true, true);
   return 0;
}

int P2MpVM( Word* args, Word& result, int message,
                          Word& local, Supplier s ) {

   result = qp->ResultStorage(s);
   MPoint* res = static_cast<MPoint*>(result.addr);
   res->Clear();
   Point* arg = static_cast<Point*>(args[0].addr);
   Instant* startt = (Instant*)args[1].addr;
   Instant* endt = (Instant*)args[2].addr;
   if(    !arg->IsDefined() || !startt->IsDefined() || !endt->IsDefined()
       || !((CcInt*)args[3].addr)->IsDefined() ) {
     res->SetDefined(false);
     return 0;
   }
   unsigned int no = ((CcInt*)args[3].addr)->GetIntval();
   if( no<= 0 ) {
     res->SetDefined(false);
    return 0;
   }
   res->SetDefined( true );
   res->StartBulkLoad();
   double delta = (*endt - *startt).ToDouble()/no;
   Instant tempstart = *startt;
   for(unsigned int i = 0;i < no;i ++){
      UPoint* up = new UPoint(true);
//      up->timeInterval.start = *startt;
//      up->timeInterval.end = *endt;
      up->timeInterval.start = tempstart;
      Instant end_t(instanttype);
      end_t.ReadFrom(tempstart.ToDouble()+delta);
      up->timeInterval.end = end_t;
      tempstart = end_t;
      up->timeInterval.lc = true;
      up->timeInterval.rc = false;
      up->p0 = *arg;
      up->p1 = *arg;
      res->Add(*up);
//      cout<<*up<<endl;
      delete up;
   }
   res->EndBulkLoad(true,true);
   return 0;
}

/*
1.1.1 Value Mapping for Operator ~turns~

Class for the localinfo in operator ~turns~:

*/

class TurnsLocalInfo{
  public:
    double minDiff;
    double maxDiff;
    DateTime maxDur;
    int currUnitIndex;
    int noUnits;
    UPoint lastUnit;
    UPoint currUnit;
    bool finished;
    bool isFirstUnit;
    bool useMaxDur;
    TupleType *resultTupleType;

    TurnsLocalInfo( const MPoint &mp_,
                    const CcReal &minDiff_,
                    const CcReal &maxDiff_,
                    const DateTime &maxDur_) :
      minDiff(0.0),
      maxDiff(0.0),
      maxDur(-9999,-9999,durationtype),
      currUnitIndex(0),
      noUnits(0),
      lastUnit(false),
      currUnit(false),
      finished(false),
      isFirstUnit(true),
      useMaxDur(false),
      resultTupleType(0)
    {
      if( !mp_.IsDefined() || !minDiff_.IsDefined() || !maxDiff_.IsDefined()
                           || !maxDur_.IsDefined() ){
        finished = true;
      } else {
        minDiff = fabs(minDiff_.GetValue());
        maxDiff = fabs(maxDiff_.GetValue());
        maxDur  = maxDur_;
        noUnits = mp_.GetNoComponents();
        finished = (noUnits <= currUnitIndex);
        useMaxDur = !(maxDur.ToDouble() < 0.0);
        finished = !(maxDiff >= minDiff);
      }
    }
    ~TurnsLocalInfo(){
      if(resultTupleType){
        resultTupleType->DeleteIfAllowed();
      }
    }
};

/*
Value mapping Function

*/
int TurnsOperatorValueMapping( Word* args, Word& result, int message,
            Word& local, Supplier s ) {
    TurnsLocalInfo *li = static_cast<TurnsLocalInfo*>(local.addr);
    UPoint u(false);

    // extract appended parameter encoding optional parameter info
    int no_args = qp->GetNoSons(s);
    const CcInt* ccArgcode = static_cast<const CcInt*>(args[no_args-1].addr);
    assert(ccArgcode && ccArgcode->IsDefined());
    int argcode = ccArgcode->GetIntval();
    assert( (argcode>=0) && (argcode<=7) );
    // determine presence and location of optional parameters
    int iDuration = ((argcode==1)||(argcode==3)||(argcode==5)||(argcode==7))?
                                                                          3:-1;
    int iBool     = (argcode==0)?-1:(((argcode==2)||(argcode==6))?
                                        3:(((argcode==3)||(argcode==7))?4:-1));
    int iGeoid    = (argcode<=0)?-1:((argcode==4)?
                        3:(((argcode==5)||(argcode==6))?4:((argcode==7)?5:-1)));
    // extract parameters
    const MPoint* m = static_cast<const MPoint*>(args[0].addr);
    const CcReal* minDiff = static_cast<const CcReal*>(args[1].addr);
    const CcReal* maxDiff = static_cast<const CcReal*>(args[2].addr);
    DateTime* maxDur   = (iDuration>=0)?
                          static_cast<DateTime*>(args[iDuration].addr):0;
    const CcBool* useHeading = (iBool>=0)?
                          static_cast<const CcBool*>(args[iBool].addr):0;
    const Geoid* geoid       = (iGeoid>=0)?
                          static_cast<const Geoid*>(args[iGeoid].addr):0;
    bool deleteMaxDur = false;
    bool useHead = useHeading && useHeading->GetBoolval();

    // initialize result attributes
    double lastHead = 0.0, currHead = 0.0, diffHead = 0.0;
    Point *lastPos=0, *currPos=0;
    DateTime *lastTime = 0, *currTime=0;
    CcReal *lastHeading=0, *currHeading=0, *diffHeading=0;
    bool found = true;

    Tuple *newTuple = 0;
    switch( message )
    {
      case OPEN:
        if(!maxDur){
          maxDur = new DateTime(-9999,-9999,durationtype);
          maxDur->SetDefined(true);
          deleteMaxDur = true;
        } // else maxDur already passed as parameter
        li = new TurnsLocalInfo(*m, *minDiff, *maxDiff, *maxDur);
        local.addr = li;
        if(!li->resultTupleType){
          li->resultTupleType =
                          new TupleType(nl->Second(GetTupleResultType(s)));
        }
        if(deleteMaxDur){
          delete maxDur;
          deleteMaxDur = false;
          maxDur = 0;
        }
        return 0;

      case REQUEST:
        if( !local.addr || li->finished ) {
          return CANCEL;
        }
        found = false;
        while( !found && (li->currUnitIndex < li->noUnits) ) {
          m->Get( li->currUnitIndex, li->currUnit );
          if( li->isFirstUnit ) {
            // We just started: save currUnit and continue
            li->lastUnit = li->currUnit;
            li->isFirstUnit = false;
          } else if( li->useMaxDur && ( li->maxDur <
             (li->currUnit.timeInterval.start - li->lastUnit.timeInterval.end)))
          {
            // We encountered a 'time warp': save currUnit and restart
            li->lastUnit = li->currUnit;
            li->isFirstUnit = true;
          } else if( AlmostEqual(li->lastUnit.p0,li->lastUnit.p1) ) {
            // The last unit was static: replace by currUnit
            li->lastUnit = li->currUnit;
          } else if( AlmostEqual(li->currUnit.p0,li->currUnit.p1) ) {
            // The current unit is static: don't save and continue
          } else {
            // now, we have 2 units. Both are non-static and temporally near
            // enough...
            lastHead = li->lastUnit.p0.Direction(li->lastUnit.p1,useHead,geoid);
            currHead = li->currUnit.p0.Direction(li->currUnit.p1,useHead,geoid);
            diffHead = currHead - lastHead;
            if(diffHead > 180.0) {
              diffHead = 360.0 - diffHead;
            } else if(diffHead < -180.0){
              diffHead = 360.0 + diffHead;
            }
            if(diffHead <= -180.0){
              diffHead *= -1.0;
            }
            if(    (fabs(diffHead) >= li->minDiff)
                && (fabs(diffHead) <= li->maxDiff) ) {
              // create attributes for result tuple
              double x1= li->lastUnit.p0.GetX(), y1= li->lastUnit.p0.GetY(),
                  x2= li->lastUnit.p1.GetX(), y2= li->lastUnit.p1.GetY(),
                  x3= li->currUnit.p1.GetX(), y3= li->currUnit.p1.GetY(),
                  detA= x2*y3 - x3*y2 - x1*y3 + x3*y1 + x1*y2 - x2*y1;

              diffHead= (detA > 0)? fabs(diffHead): -1 * fabs(diffHead);
              lastTime    = new DateTime(li->lastUnit.timeInterval.end);
              currTime    = new DateTime(li->currUnit.timeInterval.start);
              lastPos     = new Point(li->lastUnit.p1);
              currPos     = new Point(li->currUnit.p0);
              lastHeading = new CcReal(true, lastHead);
              currHeading = new CcReal(true, currHead);
              diffHeading = new CcReal(true, diffHead);
              found = true;
            } else { ; // nothing to do here
            }
            li->lastUnit = li->currUnit;
          }
          li->currUnitIndex++;
        } // end while
        if(li->currUnitIndex >= li->noUnits){
          li->finished = true;
        }
        if(found){
          // create a result tuple
          newTuple = new Tuple( li->resultTupleType );
          newTuple->PutAttribute( 0,(Attribute*)lastTime );
          newTuple->PutAttribute( 1,(Attribute*)currTime );
          newTuple->PutAttribute( 2,(Attribute*)lastPos );
          newTuple->PutAttribute( 3,(Attribute*)currPos );
          newTuple->PutAttribute( 4,(Attribute*)lastHeading );
          newTuple->PutAttribute( 5,(Attribute*)currHeading );
          newTuple->PutAttribute( 6,(Attribute*)diffHeading );
          result.setAddr(newTuple);
          return YIELD;
        } else {
          return CANCEL;
        }

      case CLOSE:
        if(local.addr){
          delete li;
          li = 0;
        }
        return 0;
    }
    /* should not happen */
    return -1;
  }

/*
1.1.1 Value Mappings for Operator ~gridcellevents~

1.1.1.1 Class ~GridCellEventsLocalInfo~

The class provides an iterator that allows to request the results for all
signatures (mpoint and upoint) of operator ~gridcellevents~ one by one using
function ~getNextResultTuple~

*/
template<class OBJTYPE>
class GridCellEventsLocalInfo{
  public:
    vector<GridCellSeq>::iterator currEvent;

    GridCellEventsLocalInfo(OBJTYPE &m_,
                            const CcReal &x0, const CcReal &y0,
                            const CcReal &wx, const CcReal &wy,
                            const CcInt &nx,
                            const Supplier &s) :
      m(&m_),
      g(0),
      events(0),
      resultTupleType(0),
      noUnits(0),
      currUnitCnt(0),
      finished(true),
      allUnitsConsumed(true),
      eventIter(events.begin()),
      cellLast(0),
      currentEvent()
    {
      if(    m->IsDefined() && x0.IsDefined() && wx.IsDefined()
          && wy.IsDefined() && nx.IsDefined() )
      {
        g = new CellGrid2D( x0.GetValue(),y0.GetValue(),
                            wx.GetValue(),wy.GetValue(),
                            nx.GetValue());
        noUnits = GetNoComponents(m);
        finished = !(g->IsDefined() && (noUnits>0));
        allUnitsConsumed = (currUnitCnt>=noUnits);
        events.clear();
        cellLast = g->getInvalidCellNo();
        resultTupleType = new TupleType(nl->Second(GetTupleResultType(s)));
      }
    }

    GridCellEventsLocalInfo(OBJTYPE &m_,
                            const CellGrid2D& grid,
                            const Supplier &s) :
      m(&m_),
      g(0),
      events(0),
      resultTupleType(0),
      noUnits(0),
      currUnitCnt(0),
      finished(true),
      allUnitsConsumed(true),
      eventIter(events.begin()),
      cellLast(0),
      currentEvent()
    {
      if(    m->IsDefined() && grid.IsDefined() )
      {
        g = new CellGrid2D( grid );
        noUnits = GetNoComponents(m);
        finished = !(g->IsDefined() && (noUnits>0));
        allUnitsConsumed = (currUnitCnt>=noUnits);
        events.clear();
        cellLast = g->getInvalidCellNo();
        resultTupleType = new TupleType(nl->Second(GetTupleResultType(s)));
      }
    }


    ~GridCellEventsLocalInfo(){
      if(resultTupleType){
        resultTupleType->DeleteIfAllowed();
        resultTupleType = 0;
      }
      if(g){
        delete g;
        g = 0;
      }
    }

    uint32_t GetNoComponents(MPoint* m){ return m->GetNoComponents(); }
    uint32_t GetNoComponents(UPoint* m){ return 1; }
    bool isFinished() const {return finished;}

    Tuple* getNextResultTuple() {
      // Returns a pointer to the next generated result tuple.
      // Returns 0 iff no more result is available
      Tuple *newTuple = 0;

      if(finished){ // nothing to do
        return newTuple;
      }

      // get the next result using private method
      int32_t cell     = g->getInvalidCellNo(),
              cellprev = g->getInvalidCellNo(),
              cellnext = g->getInvalidCellNo();
      DateTime timeenter(instanttype),
               timeleave(instanttype);

      if(next(cell, timeenter, timeleave, cellprev, cellnext)){
        CcInt *ccCell           = new CcInt(g->isValidCellNo(cell), cell);
        DateTime *ccTimeEntered = new DateTime(timeenter);
        DateTime *ccTimeLeft    = new DateTime(timeleave);
        CcInt *ccCellPrevious = new CcInt(g->isValidCellNo(cellprev), cellprev);
        CcInt *ccCellNext     = new CcInt(g->isValidCellNo(cellnext), cellnext);
        // create the result tuple and return it
        newTuple = new Tuple( resultTupleType );
        newTuple->PutAttribute( 0,(Attribute*)ccCell );
        newTuple->PutAttribute( 1,(Attribute*)ccTimeEntered );
        newTuple->PutAttribute( 2,(Attribute*)ccTimeLeft );
        newTuple->PutAttribute( 3,(Attribute*)ccCellPrevious );
        newTuple->PutAttribute( 4,(Attribute*)ccCellNext );
      }
      return newTuple;
    }

    ostream& Print( ostream &os ) {
      os << "(GridCellEventsLocalInfo: "
         << "\n\t m: " << *m
         << ",\n\t g: " << *g;
      printEvents(os);
      os << ",\n\t resultTupleType: " << resultTupleType
         << ",\n\t noUnits: " << noUnits
         << ",\n\t currUnitCnt: " << currUnitCnt
         << ",\n\t finished: " << (finished ? "true" : "false")
         << ",\n\t allUnitsConsumed: " << (allUnitsConsumed ? "true" : "false")
         << ",\n\t eventIter: <unprintable>"
         << ",\n\t cellLast: " << cellLast
         << ",\n\t currentEvent: " << currentEvent
         << "\n)" << endl;
      return os;
    }

  private:
    OBJTYPE *m;
    CellGrid2D *g;
    vector<GridCellSeq> events;
    TupleType *resultTupleType;
    uint32_t noUnits;
    uint32_t currUnitCnt;
    bool finished;
    bool allUnitsConsumed;
    vector<GridCellSeq>::iterator eventIter;
    int32_t cellLast;         // number of last visited cell
    GridCellSeq currentEvent; // the current cell info

    bool next(int32_t &cell,
              DateTime &timestart, DateTime &timeend,
              int32_t &cellprev, int32_t &cellnext)
    {
      // returns true iff a result was produced and the result parameters are
      // valid. When returning false, the result parameters are invalid.
      //
      // The functions scanns the event vector for interesting event
      // combinations, such as
      // (1) consecutive events should be merged,
      //    (a) consecutive non-valid events should always be merged.
      //    (b) if there is no temporal gap and both have the same Cell.
      // (2) Only events inside the grid (with valid cellNo) qualify for
      //     being reported as a result. Events in non-valid cell are only
      //     used to report UNDEF last/next cell numbers.
//       cout << __PRETTY_FUNCTION__ << " called." << endl;
      assert(!finished);
      if(finished){
//         cout << __PRETTY_FUNCTION__ << " finished (false)." << endl;
        return false;
      }

      GridCellSeq nextEvent = GridCellSeq();
      bool found = false;
      bool yield = false;

      if( eventIter == events.end() ){ // try to refill event-vector
        tryReload(m);
//         bool ok = tryReload(m);
//         cout << "\tInitial tryRelod: " << (ok ? "succeeded." : "failed.")
//              << endl;
      }

      // merge consecutive events with same cell number and find a nextEvent
      while( eventIter < events.end() ){
//         cout << "\t\tBegin of while-loop: cellLast=" << cellLast
//         << ", currentEvent=" << currentEvent
//         << ", nextEvent=" << nextEvent
//         << ")" << endl;
        assert(eventIter->IsDefined()); // events from vector should be defined
        if(currentEvent.IsDefined()){   // initialized with UNDEF event!
          if(eventIter->getCellNo() == currentEvent.getCellNo()) {
//             cout << "\t\tSame cell: " << eventIter->getCellNo() << endl;
            // still within the same cell
            Point pcurr(false), pnext(false);
            m->TemporalFunction(eventIter->getEnterTime(), pnext, true);
            m->TemporalFunction(currentEvent.getLeaveTime(), pcurr, true);
            if(    !g->isValidCellNo(eventIter->getCellNo())
                || ( eventIter->getEnterTime() == currentEvent.getLeaveTime() )
              )
            { // covers the following case: curr and next cell are the same
              // and EITHER the cell is off the grid OR the two are temporally
              // connected
              // --> merge the events
//               cout << "\t\tMerged events:\n\t\t\tcurr=" << currentEvent
//                    << "\t\t\tnext=" << *eventIter;
              currentEvent.set( currentEvent.getCellNo(),
                                currentEvent.getEnterTime(),
                                eventIter->getLeaveTime());
//               cout << "\t\t\tmerged event: curr = " << currentEvent << endl;
            } else if( eventIter->getEnterTime()!=currentEvent.getLeaveTime() )
            {
              // both events are valid with same cell but are temporally
              // unconnected (showing a 'timewarp')
              // --> set return parameters
              nextEvent = *eventIter; // save for terminal call (after loop)
              cell      = currentEvent.getCellNo();
              timestart = currentEvent.getEnterTime();
              timeend   = currentEvent.getLeaveTime();
              cellprev  = cellLast;
              cellnext  = g->getInvalidCellNo(); // to indicate 'timewarp'
              yield = true;
              currentEvent.setCellNo(g->getInvalidCellNo()); // modify
//               cout << "\t\tcurr is valid (warp-case). Result: (Cell=" << cell
//               << ", TimeEnter=" << timestart
//               << ", TimeLeave=" << timeend
//               << ", CellPrevious=" << cellprev
//               << ", CellNext=" << cellnext
//               << ")." << endl;
              cellLast = g->getInvalidCellNo(); // to indicate 'timewarp'
              nextEvent = *eventIter; // save for terminal call (after loop)
              currentEvent = nextEvent;
            } else {
              // curr and next are temporally unconnected
              nextEvent = *eventIter; // save for terminal call (after loop)
              found = true;
//               cout << "\t\tFound interesting event (Same cells - "
//                    << "SHOULD NOT HAPPEN!):\n\t\t\tcurr="
//                    << currentEvent << "\n\t\t\tnext=" << *eventIter << endl;
            }
          } else {
            nextEvent = *eventIter; // save for terminal call (after the loop)
            found = true;
//             cout << "\t\tFound interesting event:\n\t\t\tcurr="
//                  << currentEvent << "\n\t\t\tnext=" << *eventIter << endl;
          }
          // try handling results
          if( found ){
            if(g->isValidCellNo(currentEvent.getCellNo())){
              // currentEvent valid --> set return parameters
              cell      = currentEvent.getCellNo();
              timestart = currentEvent.getEnterTime();
              timeend   = currentEvent.getLeaveTime();
              cellprev  = cellLast;
              cellnext  = nextEvent.getCellNo();
              yield = true; // flag to return at end of this loop!
//               cout << "\t\tcurr is valid (normal case). Result: (Cell="
//                   << cell
//                   << ", TimeEnter=" << timestart
//                   << ", TimeLeave=" << timeend
//                   << ", CellPrevious=" << cellprev
//                   << ", CellNext=" << cellnext
//                   << ")." << endl;
            } else {
//               cout << "\t\tcurr is invalid. Ignore!" << endl;
              ;
            }
            cellLast = currentEvent.getCellNo();
            currentEvent = nextEvent;
            found = false;
          }
        } else { // currEvent is not defined (first call)
//           cout << "\t\tInitial event: shift" << endl;
          currentEvent = *eventIter;
        }
        // advance iterator to next event
        eventIter++;
        if(eventIter == events.end()){
          // no more events generated by this unit - try loading new events
          // from the next unit:
          tryReload(m);
//           bool ok = tryReload(m);
//           cout << "\ttryRelod: " << (ok ? "succeeded." : "failed.") << endl;
        }
        if(yield){
//           cout << __PRETTY_FUNCTION__ << " finished (true)." << endl;
          return true; // result was generated --> return
        }
//         cout << "\t\tEnd of while-loop: cellLast=" << cellLast
//         << ", currentEvent=" << currentEvent
//         << ", nextEvent=" << nextEvent
//         << ")" << endl;
      } // end while

      // no nextEvent found. Try to process current event
      if(g->isValidCellNo(currentEvent.getCellNo())){
          // No nextEvent, but currentEvent is on the grid --> produce a result
          cell      = currentEvent.getCellNo();
          timestart = currentEvent.getEnterTime();
          timeend   = currentEvent.getLeaveTime();
          cellprev  = cellLast;
          cellnext  = g->getInvalidCellNo();
//           cout << "\t\tTerminal event: curr is valid. Result: (Cell=" << cell
//                << ", TimeEnter=" << timestart
//                << ", TimeLeave=" << timeend
//                << ", CellPrevious=" << cellprev
//                << ", CellNext=" << cellnext
//                << ")." << endl;
          finished = true; // because no more events available!
//           cout << __PRETTY_FUNCTION__ << " finished (true)." << endl;
          return true;
      } else {// else: currentEvent not on grid and no more events --> finished!
//         cout << "\t\tTerminal event: curr is invalid." << endl;
        ;
      }
      finished = true;
//       cout << __PRETTY_FUNCTION__ << " finished (false)." << endl;
      return false;
    }

    bool tryReload(UPoint* dummy_) {
      // Reloads the event vector with new elements created using the mpoint's
      // next unit. The old content of ~event~ is lost!
      // Returns true iff reloading was successful.
      if(allUnitsConsumed) { return false; }; // do nothing
      // UPoint version
      m->GetGridCellSequence(*g, events);
      currUnitCnt++;
      if(currUnitCnt>=noUnits){
        allUnitsConsumed = true;
      }
      eventIter = events.begin();
      return true;
    }

    bool tryReload(MPoint* dummy_) {
      // Reloads the event vector with new elements created using the mpoint's
      // next unit. The old content of ~event~ is lost!
      // Returns true iff reloading was successful.
      if(allUnitsConsumed) { return false; }; // do nothing
      UPoint u(true);
      m->Get(currUnitCnt, u);
      u.GetGridCellSequence(*g, events);
      currUnitCnt++;
      if(currUnitCnt>=noUnits){
        allUnitsConsumed = true;
      }
      eventIter = events.begin();
      return true;
    }

    ostream& printEvents( ostream &os, const string prefix = "\n\t",
                                       const string prefix2 = "\t") {
      os << prefix2 << "events: [";
      vector<GridCellSeq>::iterator i;
      for(i = events.begin(); i < events.end(); i++){
        os << prefix << prefix2 << *i;
      }
      os << prefix2 << "]";
      return os;
    }
};

template<class OBJTYPE>
ostream& operator<<(ostream& o, GridCellEventsLocalInfo<OBJTYPE>& u){
  return u.Print(o);
}

template<class OBJTYPE>
int GridCellEventsVM( Word* args, Word& result, int message,
                             Word& local, Supplier s ) {
  GridCellEventsLocalInfo<OBJTYPE> *li =
          static_cast<GridCellEventsLocalInfo<OBJTYPE>*>(local.addr);

  switch( message )
  {
    case OPEN: {
//       cout << __PRETTY_FUNCTION__ << ": OPEN called..." << endl;
      OBJTYPE* m = static_cast<OBJTYPE*> (args[0].addr);
      int noSons = qp->GetNoSons(s);
      if(li){ delete li; }
      if(noSons==6){
         CcReal* x0 = static_cast<CcReal*>(args[1].addr);
         CcReal* y0 = static_cast<CcReal*>(args[2].addr);
         CcReal* wx = static_cast<CcReal*>(args[3].addr);
         CcReal* wy = static_cast<CcReal*>(args[4].addr);
         CcInt*  nx = static_cast<CcInt*> (args[5].addr);
         li = new GridCellEventsLocalInfo<OBJTYPE>(*m, *x0, *y0, *wx,
                                                   *wy, *nx, s);
      } else if(noSons==2){
         li = new GridCellEventsLocalInfo<OBJTYPE>(*m,
                       *(static_cast<CellGrid2D*>(args[1].addr)),
                        s);
      } else {
         assert(false);
      }
      local.addr = li;
//       cout << "li=" << *li;
//       cout << __PRETTY_FUNCTION__ << ": finished OPEN." << endl;
      return 0;
    }
    case REQUEST: {
//       cout << __PRETTY_FUNCTION__ << ": REQUEST called..." << endl;
      if( !local.addr || li->isFinished() ) {
//         cout << __PRETTY_FUNCTION__<< ": finished REQUEST (CANCEL)." << endl;
        return CANCEL;
      }
//       cout << "li=" << *li;
      Tuple * newTuple = li->getNextResultTuple();
      result.setAddr(newTuple);
      if( newTuple != 0) {
//         cout << __PRETTY_FUNCTION__ << ": finished REQUEST (YIELD)." << endl;
        return YIELD;
      }
//       cout << __PRETTY_FUNCTION__ << ": finished REQUEST (CANCEL)." << endl;
      return CANCEL;
    }
    case CLOSE:{
//       cout << __PRETTY_FUNCTION__ << ": CLOSE called..." << endl;
      if(local.addr){
//         cout << "li=" << *li;
        delete li;
        li = 0;
        local.addr = 0;
      }
//       cout << __PRETTY_FUNCTION__ << ": finished CLOSE." << endl;
      return 0;
    }
  }
  /* should not happen */
  return -1;
}

/*
16.3.29 Value mapping functions of operator ~squareddistance~

*/

int SquaredDistanceMPPVM( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((MPoint*)args[0].addr)->SquaredDistance( *((Point*)args[1].addr),
   *((MReal*)result.addr) );
  return 0;
}

int SquaredDistancePMPVM( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((MPoint*)args[1].addr)->SquaredDistance( *((Point*)args[0].addr),
   *((MReal*)result.addr) );
  return 0;
}

int SquaredDistanceMPMPVM( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((MPoint*)args[0].addr)->SquaredDistance( *((MPoint*)args[1].addr),
   *((MReal*)result.addr) );
  return 0;
}

/*
16.4 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define 
an array of value mapping functions for each operator. For 
nonoverloaded operators there is also such and array
defined, so it easier to make them overloaded.

16.4.1 ValueMapping arrays

*/
ValueMapping temporalisemptymap[] = { InstantIsEmpty,
                                      RangeIsEmpty<RInt>,
                                      RangeIsEmpty<RReal>,
                                      RangeIsEmpty<Periods>,
                                 //   MappingIsEmpty<MBool>,
                                 //   MappingIsEmpty<MInt>,
                                 //   MappingIsEmpty<MReal>,
                                 //   MappingIsEmpty<MPoint>,
                                      UnitIsEmpty<UBool>,
                                      UnitIsEmpty<UInt>,
                                      UnitIsEmpty<UReal>,
                                      UnitIsEmpty<UPoint>,
                                      IntimeIsEmpty<CcBool>,
                                      IntimeIsEmpty<CcInt>,
                                      IntimeIsEmpty<CcReal>,
                                      IntimeIsEmpty<Point>};


ValueMapping temporalequalmap[] = { InstantEqual,
                                    RangeEqual<RInt>,
                                    RangeEqual<RReal>,
                                    RangeEqual<Periods >,
                                    IntimeComparePredicates<CcBool,0>,
                                    IntimeComparePredicates<CcInt, 0>,
                                    IntimeComparePredicates<CcReal,0>,
                                    IntimeComparePredicates<Point, 0>
};

ValueMapping temporalequalmap2[] = { MappingEqual<MBool>,
                                    MappingEqual<MInt>,
                                    MappingEqual<MReal>,
                                    MPointEqual };

ValueMapping temporalnotequalmap[] = { InstantNotEqual,
                                       RangeNotEqual<RInt>,
                                       RangeNotEqual<RReal>,
                                       RangeNotEqual<Periods>,
                                       IntimeComparePredicates<CcBool, 1>,
                                       IntimeComparePredicates<CcInt,  1>,
                                       IntimeComparePredicates<CcReal, 1>,
                                       IntimeComparePredicates<Point,  1>};

ValueMapping temporalnotequalmap2[] = { MappingNotEqual<MBool>,
                                       MappingNotEqual<MInt>,
                                       MappingNotEqual<MReal>,
                                       MPointNotEqual };

ValueMapping temporalintersectsmap[] = { RangeIntersects<RInt>,
                                         RangeIntersects<RReal>,
                                         RangeIntersects<Periods> };

ValueMapping temporalinsidemap[] = { RangeInside_rr<RInt>,
                                     RangeInside_rr<RReal>,
                                     RangeInside_rr<Periods>,
                                     RangeInside_ar<CcInt, RInt>,
                                     RangeInside_ar<CcReal, RReal>,
                                     RangeInside_ar<Instant, Periods> };

ValueMapping temporalbeforemap[] = { RangeBefore_rr<RInt>,
                                     RangeBefore_rr<RReal>,
                                     RangeBefore_rr<Periods>,
                                     RangeBefore_ar<CcInt, RInt>,
                                     RangeBefore_ar<CcReal, RReal>,
                                     RangeBefore_ar<Instant, Periods>,
                                     RangeBefore_ra<RInt, CcInt>,
                                     RangeBefore_ra<RReal, CcReal>,
                                     RangeBefore_ra<Periods, Instant> };

ValueMapping temporalintersectionmap[] = { RangeIntersection<RInt>,
                                           RangeIntersection<RReal>,
                                           RangeIntersection<Periods> };

ValueMapping temporalunionmap[] = { RangeUnion<RInt>,
                                    RangeUnion<RReal>,
                                    RangeUnion<Periods> };

ValueMapping temporalminusmap[] = { RangeMinus<RInt>,
                                    RangeMinus<RReal>,
                                    RangeMinus<Periods> };

ValueMapping temporalminmap[] = { RangeMinimum<RInt, CcInt>,
                                  RangeMinimum<RReal, CcReal>,
                                  RangeMinimum<Periods, Instant> };

ValueMapping temporalmaxmap[] = { RangeMaximum<RInt, CcInt>,
                                  RangeMaximum<RReal, CcReal>,
                                  RangeMaximum<Periods, Instant> };

ValueMapping temporalnocomponentsmap[] = { RangeNoComponents<RInt>,
                                           RangeNoComponents<RReal>,
                                           RangeNoComponents<Periods>,
                                           MappingNoComponents<MBool, CcBool>,
                                           MappingNoComponents<MInt, CcInt>,
                                           MappingNoComponents<MReal, CcReal>,
                                           MappingNoComponents<MPoint, Point>};

ValueMapping temporalbboxmap[] = { UPointBBox,
                                   MPointBBox,
                                   IPointBBox,
                                   PeriodsBBox,
                                   InstantBBox };

ValueMapping temporalmbrangemap[] = {
                                   TempMBRange<RInt>,
                                   TempMBRange<RReal>,
                                   TempMBRange<Periods>,
                                   TempMBRange<RBool>,
                                   TempMBRange<RString>};

ValueMapping temporalbboxoldmap[] = { UPointBBox,
                                      TempMBRange<RInt>,
                                      TempMBRange<RReal>,
                                      TempMBRange<Periods>,
                                      MPointBBoxOld,
                                      IPointBBox };

ValueMapping temporalbbox2dmap[] = { UPointBBox2d,
                                     MPointBBox2d,
                                     IPointBBox2d };

ValueMapping temporalinstmap[] = { IntimeInst<CcBool>,
                                   IntimeInst<CcInt>,
                                   IntimeInst<CcReal>,
                                   IntimeInst<Point> };

ValueMapping temporalvalmap[] = { IntimeVal<CcBool>,
                                  IntimeVal<CcInt>,
                                  IntimeVal<CcReal>,
                                  IntimeVal<Point> };

ValueMapping temporaluvalmap[] = { UIntimeVal<UInt,CcInt>};

ValueMapping temporalatinstantmap[] = { MappingAtInstant<MBool, CcBool>,
                                        MappingAtInstant<MInt, CcInt>,
                                        MappingAtInstant<MReal, CcReal>,
                                        MPointAtInstant };

ValueMapping temporalatperiodsmap[] = { MappingAtPeriods<MBool>,
                                        MappingAtPeriods<MInt>,
                                        MappingAtPeriods<MReal>,
                                        MPointAtPeriods };

ValueMapping temporalwhenmap[] = { MappingWhen<MBool>,
                                        MappingWhen<MInt>,
                                        MappingWhen<MReal>,
                                        MappingWhen<MPoint> };

ValueMapping temporaldeftimemap[] = { MappingDefTime<MBool>,
                                      MappingDefTime<MInt>,
                                      MappingDefTime<MReal>,
                                      MappingDefTime<MPoint> };

ValueMapping temporalpresentmap[] = { MappingPresent_i<MBool>,
                                      MappingPresent_i<MInt>,
                                      MappingPresent_i<MReal>,
                                      MPointPresent_i,
                                      MappingPresent_p<MBool>,
                                      MappingPresent_p<MInt>,
                                      MappingPresent_p<MReal>,
                                      MPointPresent_p };

ValueMapping temporalpassesmap[] = { MappingPasses<MBool, CcBool, CcBool>,
                                     MappingPasses<MInt, CcInt, CcInt>,
                                     MappingPasses<MReal, CcReal, CcReal>,
                                     MappingPasses<MPoint, Point, Point>,
                                     MappingPasses<MPoint, Point, Region>,
    MappingPasses<MPoint, Point, Rectangle<2> > };

ValueMapping temporalinitialmap[] = { MappingInitial<MBool, UBool, CcBool>,
                                      MappingInitial<MInt, UInt, CcInt>,
                                      MappingInitial<MReal, UReal, CcReal>,
                                      MappingInitial<MPoint, UPoint, Point> };

ValueMapping temporalfinalmap[] = { MappingFinal<MBool, UBool, CcBool>,
                                    MappingFinal<MInt, UInt, CcInt>,
                                    MappingFinal<MReal, UReal, CcReal>,
                                    MappingFinal<MPoint, UPoint, Point> };

ValueMapping temporalatmap[] = { MappingAt<MBool, UBool, CcBool>,
                                 MappingAt<MInt, UInt, CcInt>,
                                 MappingAt_MReal_CcReal,
                                 MappingAt<MPoint, UPoint, Point> };

ValueMapping temporalunitsmap[] = { MappingUnits<MBool, UBool>,
                                    MappingUnits<MInt,  UInt>,
                                    MappingUnits<MReal, UReal>,
                                    MappingUnits<MPoint, UPoint> };

ValueMapping temporalgetunitmap[] = { MappingGetUnit<MBool, UBool>,
                                    MappingGetUnit<MInt,  UInt>,
                                    MappingGetUnit<MReal, UReal>,
                                    MappingGetUnit<MPoint, UPoint> };

ValueMapping temporalbox3dmap[] = { Box3d_rect,
                                    Box3d_instant,
                                    Box3d_rect_instant,
                                    Box3d_periods,
                                    Box3d_rect_periods};

ValueMapping temporalbox2dmap[] = { TemporalBox2d<2>,
                                    TemporalBox2d<3>,
                                    TemporalBox2d<4>,
                                    TemporalBox2d<8> };

ValueMapping extdeftimemap[] = { TemporalExtDeftime<UBool, CcBool>,
                                 TemporalExtDeftime<UInt, CcInt>
                               };

ValueMapping simplifymap[] = { MPointSimplify, MPointSimplify2,
                               MRealSimplify };

ValueMapping linearizemap[] = { Linearize<UReal>, Linearize<MReal> };
ValueMapping linearize2map[] = { Linearize2_ureal, Linearize2 };
ValueMapping integratemap[] = { Integrate<UReal>, Integrate<MReal> };

ValueMapping approximatemap[] = {
  ApproximateMvalue<MPoint, UPoint, Point, true>,
  ApproximateMvalue<MReal, UReal, CcReal, true>,
  ApproximateMvalue<MInt, UInt, CcInt, false>,
  ApproximateMvalue<MBool, UBool, CcBool, false>,
  ApproximateMvalue<MString, UString, CcString, false>,
};

ValueMapping minmap[] = { VM_Min<UReal, CcReal>,
                          VM_Min<MReal, CcReal>,
                          VM_Min<MInt, CcInt> };

ValueMapping maxmap[] = { VM_Max<UReal, CcReal>,
                          VM_Max<MReal, CcReal>,
                          VM_Max<MInt, CcInt> };

ValueMapping samplempointmap[] = { SampleMPointVM<false,false>,
                                   SampleMPointVM<true,false>,
                                   SampleMPointVM<true,true>};

ValueMapping mpointsquareddistancemap[]= {SquaredDistanceMPPVM,
                                          SquaredDistancePMPVM,
                                          SquaredDistanceMPMPVM};

ValueMapping temporaltherangemap[] = {
  TemporalTheRangeTM<Instant>, // 0
  TemporalTheRangeTM<CcInt>,
  TemporalTheRangeTM<CcBool>,
  TemporalTheRangeTM<CcReal>,
  TemporalTheRangeTM<CcString> // 4
};

ValueMapping restrictVM[] = {
  restrictVM1, restrictVM2
};

ValueMapping MappingTimeShiftMap[] = {
    MappingTimeShift<MBool, UBool>,
    MappingTimeShift<MInt,  UInt>,
    MappingTimeShift<MReal, UReal>,
    MappingTimeShift<MPoint, UPoint>
};

ValueMapping GridCellEventsValueMapping[] = {
    GridCellEventsVM<UPoint>,
    GridCellEventsVM<MPoint>
};

/*
16.4.2 Specification strings

*/
const string TemporalSpecIsEmpty  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>instant -> bool,\n"
  "rT -> bool, \n"
  "uT -> bool, \n"
  "iT -> bool, \n"
  "For T in {bool,int,real,point}</text--->"
  "<text>isempty ( _ )</text--->"
  "<text>Returns TRUE iff the instant/range/unit type value is "
  "undefined or empty.</text--->"
  "<text>query isempty( mpoint1 )</text--->"
  ") )";

const string MappingTimeShiftSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>moving(x) x duration -> moving(x)</text--->"
  "<text>_ timeshift[ _ ]</text--->"
  "<text>Shifts the definition time of a moving object.</text--->"
  "<text>train7 timeshift[ create_duration(1, 0) ]</text--->"
  ") )";

const string TemporalSpecEQ  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool, \n"
  "(rT rT) -> bool, \n"
  "(iT iT) -> bool</text--->"
  "<text>_ = _</text--->"
  "<text>Is-Equal predicate for instant and range type values.\n"
  "Returns UNDEFINED, iff at least one argument is UNDEFINED.</text--->"
  "<text>query i1 = i2</text--->"
  ") )";

const string TemporalSpecEQ2  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(moving(x) moving(x)) -> bool</text--->"
  "<text>_ equal _</text--->"
  "<text>Equal. Returns UNDEFINED, iff at least one argument is "
  "UNDEFINED.</text--->"
  "<text>query mi equal mi2</text--->"
  ") )";

const string TemporalSpecNE  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool,\n"
  "(rT rT)) -> bool, \n"
  "(iT iT) -> bool </text--->"
  "<text>_ # _</text--->"
    "<text>Not-Equal predicate for instant and range type values. Returns "
  "UNDEFINED, iff at least one argument is UNDEFINED.</text--->"
  "<text>query i1 # i2</text--->"
  ") )";

const string TemporalSpecNE2  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(mT mT) -> bool</text--->"
  "<text>_ nonequal _</text--->"
  "<text>Not-Equal predicate for moving objects. Returns UNDEFINED, iff at "
  "least one argument is UNDEFINED.</text--->"
  "<text>query mi1 nonequal mi2</text--->"
  ") )";

const string TemporalSpecLT  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool</text--->"
  "<text>_ < _</text--->"
  "<text>Less than. Returns UNDEFINED, iff at least one argument is "
  "UNDEFINED.</text--->"
  "<text>query i1 < i2</text--->"
  ") )";

const string TemporalSpecLE  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool</text--->"
  "<text>_ <= _</text--->"
  "<text>Less or equal than. Returns UNDEFINED, iff at least one argument "
  "is UNDEFINED.</text--->"
  "<text>query i1 <= i2</text--->"
  ") )";

const string TemporalSpecGT  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool</text--->"
  "<text>_ > _</text--->"
  "<text>Greater than. Returns UNDEFINED, iff at least one argument is "
  "UNDEFINED.</text--->"
  "<text>query i1 > i2</text--->"
  ") )";

const string TemporalSpecGE  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool</text--->"
  "<text>_ >= _</text--->"
  "<text>Greater or equal than. Returns UNDEFINED, iff at least one argument "
  "is UNDEFINED.</text--->"
  "<text>query i1 >= i2</text--->"
  ") )";

const string TemporalSpecIntersects =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(rT rT) -> bool</text--->"
  "<text>_ intersects _</text--->"
  "<text>Intersects predicate for range type values.</text--->"
  "<text>query range1 intersects range2</text--->"
  ") )";

const string TemporalSpecInside  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(rT rT) -> bool,\n"
  "(T  rT) -> bool</text--->"
  "<text>_ inside _</text--->"
  "<text>Inside predicate for range type values.</text--->"
  "<text>query 5 inside rint</text--->"
  ") )";

const string TemporalSpecBefore  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(rT rT) -> bool,\n"
  "(T rT) -> bool, (rT T) -> bool</text--->"
  "<text>_ before _ for for range type values and value</text--->"
  "<text>Before.</text--->"
  "<text>query 5 before rint</text--->"
  ") )";

const string TemporalSpecIntersection =
  "( ( \"Signature\" \"Syntax\"\"Meaning\" \"Example\" ) "
  "( <text>(rT rT) -> rT</text--->"
  "<text>_ intersection _</text--->"
  "<text>Intersection for range type values</text--->"
  "<text>query range1 intersection range2</text--->"
  ") )";

const string TemporalSpecUnion  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(rT rT) -> rT</text--->"
  "<text>_ union _</text--->"
  "<text>Union for range type values.</text--->"
  "<text>query range1 union range2</text--->"
  ") )";

const string TemporalSpecMinus  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(rT rT ) -> rT</text--->"
  "<text>_ minus _</text--->"
  "<text>Minus for range type values.</text--->"
  "<text>query range1 minus range2</text--->"
  ") )";

const string TemporalSpecMinimum  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>rT -> T</text--->"
  "<text>minimum ( _ )</text--->"
  "<text>Minimum of a range type value.</text--->"
  "<text>minimum ( range1 )</text--->"
  ") )";

const string TemporalSpecMaximum  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>rT -> T</text--->"
  "<text>maximum ( _ )</text--->"
  "<text>Maximum of a range type value.</text--->"
  "<text>maximum ( range1 )</text--->"
  ") )";

const string TemporalSpecNoComponents =
  "( ( \"Signature\" \"Syntax\"\"Meaning\" \"Example\" ) "
  "( <text>rT -> int,\n"
  "mT -> int</text--->"
  "<text>no_components ( _ )</text--->"
  "<text>Number of components within a range type value."
  "Number of units inside a moving object value. UNDEFINED argument yields "
  "an UNDEFINED result.</text--->"
  "<text>no_components ( mpoint1 )</text--->"
  ") )";

const string TemporalSpecInst  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>iT -> instant</text--->"
  "<text>inst ( _ )</text--->"
  "<text>Return an intime values' time instant.</text--->"
  "<text>inst ( i1 )</text--->"
  ") )";

const string TemporalSpecVal  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>iT -> x</text--->"
  "<text>val ( _ )</text--->"
  "<text>Return an intime value's value.</text--->"
  "<text>val ( i1 )</text--->"
  ") )";
const string TemporalSpecUVal  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>uint -> int</text--->"
  "<text>uval ( _ )</text--->"
  "<text>Return an uint's value.</text--->"
  "<text>uval ( ui1 )</text--->"
  ") )";

const string TemporalSpecAtInstant =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT instant) -> iT</text--->"
  "<text>_ atinstant _ </text--->"
  "<text>From a moving object get the intime value "
  "corresponding to the temporal value at the given instant.</text--->"
  "<text>mpoint1 atinstant instant1</text--->"
  ") )";

const string TemporalSpecAtPeriods =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT periods) -> mT</text--->"
  "<text>_ atperiods _ </text--->"
  "<text>Restrict the moving object to the given periods.</text--->"
  "<text>mpoint1 atperiods periods1</text--->"
  ") )";

const string TemporalSpecWhen =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT mbool) -> mT</text--->"
  "<text>_ when[_] </text--->"
  "<text>Restrict the moving object to the times on which the mbool is "
  "true.</text--->"
  "<text>mpoint1 when[speed(mpoint1) > 10.0]</text--->"
  ") )";

const string TemporalSpecDefTime  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>moving(x) -> periods</text--->"
  "<text>deftime( _ )</text--->"
  "<text>Get the defined time of the corresponding moving data "
  "objects.</text--->"
  "<text>deftime( mp1 )</text--->"
  ") )";

const string TemporalSpecTrajectory =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint -> line</text--->"
  "<text> trajectory( _ )</text--->"
  "<text>Get the trajectory of the corresponding moving point object.</text--->"
  "<text>trajectory( mp1 )</text--->"
  ") )";

const string TemporalSpecPresent  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT instant) -> bool,\n"
  "(mT periods) -> bool</text--->"
  "<text>_ present _ </text--->"
  "<text>Check, whether the moving object is present at the given "
  "instant or period.</text--->"
  "<text>mpoint1 present instant1</text--->"
  ") )";

const string TemporalSpecPasses =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT T) -> bool; T in {Region, Rectangle2}"
  "</text--->"
  "<text>_ passes _ </text--->"
  "<text>Check, whether the moving object passes the given value.</text--->"
  "<text>mpoint1 passes point1</text--->"
                                ") )";

const string TemporalSpecInitial  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mT -> iT</text--->"
  "<text>initial( _ )</text--->"
  "<text>Get the intime value corresponding to the initial instant.</text--->"
  "<text>initial( mpoint1 )</text--->"
  ") )";

const string TemporalSpecFinal  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mT -> iT</text--->"
  "<text>final( _ )</text--->"
  "<text>From an moving object, get the intime value "
  "corresponding to its final instant.</text--->"
  "<text>final( mpoint1 )</text--->"
  ") )";

const string TemporalSpecAt =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT T) -> mT</text--->"
  "<text> _ at _ </text--->"
  "<text>Restrict the moving object to the times where its value "
  "equals the given value.</text--->"
  "<text>mpoint1 at point1</text--->"
  ") )";

const string TemporalSpecDistance =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mpoint point) -> mreal</text--->"
  "<text>distance( _, _ ) </text--->"
  "<text>Returns the moving distance.</text--->"
  "<text>distance( mpoint1, point1 )</text--->"
  ") )";

const string TemporalSpecSquaredDistance =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mpoint point) -> mreal, (mpoint mpoint)->mreal</text--->"
  "<text>squareddistance( _, _ ) </text--->"
  "<text>Returns the squared moving distance.</text--->"
  "<text>squareddistance( mpoint1, point1 )</text--->"
  ") )";


const string TemporalSpecSimplify =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint x real [ x duration ] -> mpoint |"
  " mreal x real -> mreal</text--->"
  "<text>simplify( o, eVal [, eTime]) </text--->"
  "<text>Simplifys the argument o allowing a maximum imprecision of eVal on "
  "the value scale, and a maximum difference of eT on the temporal scale."
  "</text--->"
  "<text>simplify( train7, 50.0, [const duration value (0, 10000)] )</text--->"
  ") )";

const string TemporalSpecIntegrate =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>{ureal , mreal} -> real</text--->"
  "<text>integrate( _ ) </text--->"
  "<text>Computes the determined integral of the argument.</text--->"
  "<text>integrate(mreal5000)</text--->"
  ") )";

const string TemporalSpecLinearize =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> t  -> t, where t in {mreal, ureal}</text--->"
  "<text>linearize( _ ) </text--->"
  "<text>Approximates the argument by a piecewise linear function.</text--->"
  "<text>linearize(distance(train7, train6))</text--->"
  ") )";

const string TemporalSpecApproximate =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>stream(tuple(a1: t1) (...(an,tn) x ai x aj [ x bool ] [ x duration ]"
  "-> mtj , (ti = instant, tj in {real,point,int,bool,string)</text--->"
  "<text>_ approximate [ i, j, mc, dt_split ] </text--->"
  "<text>Computes a moving type from the time/value pairs of the stream "
  "argument. i/j indicate the attribute names for the time/value data within "
  "the stream tuples. The optional parameter dt_split sets a maximum duration "
  "for unit definition times. If two consecutive data points have longer "
  "temporal distance, they are not connected by a unit."
  "For real and point data, the result contains units with a linear function "
  "describing the temporal evolution between each pair of consecutive data "
  "points. For int, bool, and string values, constant units are created, where "
  "a value is valid until the next given data point. This means, that the final"
  " data point's value is lost. If the optional boolean parameter 'mc' is set "
  "to FALSE, this type of constant units is also created for point and real "
  "data. The default value for 'mc' is TRUE.</text--->"
  "<text>  </text--->"
  ") )";

const string TemporalSpecLinearize2 =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> mreal -> mreal | \n"
  "   ureal -> (stream ureal)</text--->"
  "<text>linearize2( _ ) </text--->"
  "<text>Computes a piecewise linear approximation of the argument.</text--->"
  "<text>linearize2(distance(train7, train6))</text--->"
  ") )";

const string TemporalSpecMin =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>{ureal,mreal}->real, mint->int</text--->"
  "<text>minimum( _ ) </text--->"
  "<text>Computes the minimum value of the argument.</text--->"
  "<text>minimum(mreal5000)</text--->"
  ") )";

const string TemporalSpecMax =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>{ureal,mreal}->real, mint->int</text--->"
  "<text>maximum( _ ) </text--->"
  "<text>Computes the maximum value of the argument.</text--->"
  "<text>maximum(mreal5000)</text--->"
  ") )";

const string TemporalSpecBreakPoints =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint x duration [x real] -> points</text--->"
  "<text>breakpoints( m, d, e ) </text--->"
  "<text>Computes all points where the mpoint 'm' stops longer"
  " than the given duration 'd'. If the argument e is set, "
  " a stop is defined to move not more than e within the "
  " duration time."
  "</text--->"
  "<text>breakpoints( train7, [const duration value (0 1000)] )</text--->"
  ") )";

const string breaksSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint x duration x real -> peridos</text--->"
  "<text>breaks( m, d, e ) </text--->"
  "<text>Computes all intervals where the mpoint 'm' stops longer"
  " than the given duration 'd'.A stop is defined to move not more "
  "than e within the  duration time."
  "</text--->"
  "<text>breaks( train7, [const duration value (0 1000), 30.0] )</text--->"
  ") )";
/*
1.1.1 Spec for ~gk~

*/
const string TemporalSpecgk =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint [ x int ] -> mpoint</text--->"
  "<text>gk( mp [, zone] ) </text--->"
  "<text>Projects the argument 'mp' using the Gauss Krueger projection "
  "with center meridian 'zone'. Zone width is 3°. If 0 <= 'zone' <= 119 is not"
  " provided, 2 (center meridian = 6°E, suits the location of Hagen) will be "
  "used as a default. 'mp' is expected to have geographic coordinates "
  "(LAT/LON) in °, the result's coordinates (NORTHING,EASTING) are in metres."
  "</text--->"
  "<text> gk( trip )</text--->"
  ") )";

const string TemporalSpecVertices =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint -> points</text--->"
  "<text> vertices( _ ) </text--->"
  "<text>Stores the end points of all contained units within a points "
  "value.</text--->"
  "<text> vertices( train7 )</text--->"
  ") )";

const string TemporalSpecUnits  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in {bool, int, real, point}:\n"
  "   mT -> (stream uT)</text--->"
  "<text> units( _ )</text--->"
  "<text>Covert a moving type object to a stream of units.</text--->"
  "<text>units( mpoint1 )</text--->"
  ") )";

const string TemporalSpecGetUnit  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> mT x int -> uT, where T in {bool, int, real, point}</text--->"
  "<text> getunit( M, N )</text--->"
  "<text>Yields the Nth unit from the moving object M."
  "</text--->"
  "<text>mpoint1 getunit( 0 )</text--->"
  ") )";

const string TemporalSpecBBox  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>upoint [x geoid] -> rect3,\n"
  "mpoint [x geoid] -> rect3,\n"
  "ipoint [x geoid] -> rect3,\n"
  "instant -> rect3,\n"
  "periods -> rect3</text--->"
  "<text>bbox ( Obj [, Geoid])</text--->"
  "<text>Returns the 3d bounding box of the spatio-temporal object Obj, \n"
  "resp. the universe restricted to the definition time of the instant/\n"
  "period value. If Geoid is passed, the geographic MBR is computed.</text--->"
  "<text>query bbox( upoint1 )</text--->"
  ") )";

const string TemporalSpecMBRange  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>rT -> rT, T in {int, real, bool, string}</text--->"
  "<text>mbrange ( _ )</text--->"
  "<text>Returns the argument's minimum bounding range (the smallest closed\n"
  "interval containing all values within the given range type value).</text--->"
  "<text>query mbrange( deftime(train6) )</text--->"
  ") )";

const string TemporalSpecBBoxOld  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>upoint [x geoid] -> rect3,\n"
    "mpoint [x geoid] -> rect3,\n"
    "ipoint [x geoid] -> rect3,\n"
    "rT -> rT</text--->"
    "<text>bboxOld ( Obj [, Geoid] )</text--->"
    "<text>Returns the 3d bounding box of the spatio-temporal object Obj, \n"
    "resp. the range value with the smallest closed interval that contains "
    "all intervals of a range-value (for range-value). If Geoid is passed, the "
    "geographic MBR is returned.</text--->"
    "<text>query bbox( upoint1 )</text--->"
    ") )";

const string TemporalSpecBBox2d  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>upoint [x geoid] -> rect,\n"
    "mpoint [x geoid] -> rect,\n"
    "ipoint [x geoid] -> rect"
    "</text--->"
    "<text>bbox2d( Obj [, Geoid] )</text--->"
    "<text>Returns the 2d bounding box of the spatio-temporal object Obj."
    " If Geoid is passed, the geographic MBR is returned.</text--->"
    "<text>query bbox2d( upoint1 )</text--->"
    ") )";

const string MPointSpecTranslate  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint x duration x real x real -> mpoint</text--->"
  "<text>_ translate[ ot, ox, oy ]</text--->"
  "<text>Moves the object by a given temporal (ot) and spatial (ox/oy) offset."
  "</text--->"
  "<text>query mp1 translate[[const duration value (5 10)],5.0,8.0]</text--->"
  ") )";

const string TemporalSpecTheYear  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int -> periods</text--->"
  "<text> theyear( y )</text--->"
  "<text>Create a periods value for the given year 'y'.</text--->"
  "<text>theyear(2002)</text--->"
                                ") )";

const string TemporalSpecTheMonth  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int x int -> periods</text--->"
  "<text> themonth( y, m )</text--->"
  "<text>Create a periods value for the given of the year/month.</text--->"
  "<text>themonth(2002, 3)</text--->"
                                ") )";

const string TemporalSpecTheDay  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int x int x int -> periods</text--->"
  "<text>theday( y, m, d )</text--->"
  "<text>Create a periods value for the given year/month/day.</text--->"
  "<text>theday(2002, 6,3)</text--->"
  ") )";

const string TemporalSpecTheHour  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int x int x int x int -> periods</text--->"
  "<text>thehour( y, m, d , h)</text--->"
  "<text>Create a periods value for the given year/month/day/hour.</text--->"
  "<text>thehour(2002, 2, 28, 8)</text--->"
  ") )";

const string TemporalSpecTheMinute =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  "<text>int x int x int x int x int -> periods</text--->"
  "<text>theminute( y, m, d , h, min )</text--->"
  "<text>Create a periods value for the given year/month/day/hour/minute."
  "</text--->"
  "<text>theminute(2002, 3, 28, 8, 59)</text--->"
  ") )";

const string TemporalSpecTheSecond =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int x int x int x int x int x int -> periods</text--->"
  "<text>thesecond( y, m, d , h, min, sec )</text--->"
  "<text>Create a periods value for the given year/month/day/hour/minute/"
  "second.</text--->"
  "<text>thesecond(2002, 12, 31, 23, 59, 59)</text--->"
  ") )";

const string TemporalSpecThePeriod =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(periods periods) -> periods</text--->"
  "<text> theperiod( _, _ )</text--->"
  "<text>Create a period that spans from the starting instant of the first,"
  "to the ending instance of the second periods argument.</text--->"
  "<text>theperiod(theyear(2002), theyear(2004))</text--->"
  ") )";

const string Box3dSpec  =
  "( ( \"Signatures\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For S in {rect, instant, periods}:\n"
  "          S -> rect3 \n"
  "For T in {instant, periods}:\n"
  "   rect x T -> rect3  </text--->"
  "<text>box3d(_)</text--->"
  "<text>returns a threedimensional box which is unlimited "
  "in non-specified parts</text--->"
  "<text>query box3d(bbox(mehringdamm))</text--->"
  ") )";

const string TemporalBox2dSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>{rect|rect3|rect4|rect8} -> rect</text--->"
  "<text>box2d( _ )</text--->"
  "<text>Restricts a rect<d> to its 1st and 2nd dimension. Can be used to "
  "eliminate the temporal dimension of a 3D bounding box.</text--->"
  "<text>box2d(r3)</text--->"
  ") )";

const string TemporalMBool2MIntSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mbool -> mint </text--->"
  "<text>mbool2mint( _ ) </text--->"
  "<text>Converts a mbool value into a mint value: FALSE -> 0, TRUE -> 1."
  "</text--->"
  "<text>mbool2mint(mb1)</text--->"
  ") )";

const string TemporalMInt2MBoolSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mint -> mbool </text--->"
  "<text>mint2mbool( _ ) </text--->"
  "<text>Converts the mint value into a mbool value. Zero (0) units are "
  "converted into FALSE units, all others into TRUE units.</text--->"
  "<text>mint2mbool(zero())</text--->"
  ") )";

const string TemporalMInt2MRealSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mint -> mreal </text--->"
  "<text>mint2mreal( _ ) </text--->"
  "<text>Converts the mint value into an mreal value.</text--->"
  "<text>mint2mreal(zero())</text--->"
  ") )";

const string TemporalExtDeftimeSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mT x uT -> mT with T in {bool, int}  </text--->"
  "<text>extenddeftime( m, u) </text--->"
  "<text>Extends the moving object m's deftime by that of the unit u's deftime,"
  " filling all definition gaps of 'm' with the value taken from 'u'.</text--->"
  "<text>query extdeftime(mb ub)</text--->"
  ") )";

const string TemporalTheRangeSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T x T x bool x bool -> rT with T in {bool, int, real, string}\n"
    "instant x instant x bool x bool -> periods</text--->"
    "<text>theRange( left, right, lc, rc ) </text--->"
    "<text>Creates a rangetype value with a single interval having "
    "boundaries 'left' and 'right'. "
    "'lc' and 'rc' specify the left/right closedness of the interval.\n"
    "If 'left' > 'right, both pairs of arguments are swapped.</text--->"
    "<text>query theRange(mb ub)</text--->"
    ") )";

const string TranslateAppendSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint x mpoint x duration -> mpoint</text--->"
    "<text> mp1 translateappend [ mp2 dur ]</text--->"
    "<text>Appends the second argument to the first one, \n"
    " waiting for a duration given by the third argument at the\n"
    " last position of the first argument</text--->"
    "<text>query mp1 translateappend[mp2 "
    "[const duration value(0 10000)]]</text--->"
    ") )";

const string TranslateAppendSSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>stream(tuple((a1 t1)...(an tn))) x ai x duration"
    " -> mpoint, where ti = mpoint</text--->"
    "<text> _ translateappendS[ _ _ ]</text--->"
    "<text>Builds a single moving point from all mpoints in the stream \n"
    " translating the mpoints in such a way that a connected movement\n"
    " is created. 'Jumps' within stream elements are not removed.</text--->"
    "<text>query Trains feed translateappendS[Trip [const"
    " duration value(0 10000)]]</text--->"
    ") )";

const string ReverseSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint -> mpoint</text--->"
    "<text> reverse( _ )</text--->"
    "<text>Computes the reverse movement of the argument</text--->"
    "<text>query reverse(Train6) </text--->"
    ") )";

const string SampleMPointSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint x duration [x bool [x bool] ] -> mpoint</text--->"
    "<text> samplempoint( m, dur )\n"
    " samplempoint( m, dur, ke)\n"
    " samplempoint( m, dur, ke, ep )</text--->"
    "<text>Simulation of a gps receiver. Resamples mpoint 'm' at intervals "
    "defined by duration 'dur'. For 'ke' = TRUE, the endpoint will be kept, "
    "for 'ep' = TRUE, the exact path will be kept (additional units); defaults "
    "are 'ke' = FALSE, 'ep' = FALSE. The result is an mpoint value.</text--->"
    "<text>query samplempoint(Train6,"
    " [const duration value (0 2000)] ) </text--->"
    ") )";

const string GPSSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint x duration -> "
    "stream(tuple([Time: instant, Position:point])</text--->"
    "<text> gps( _ , _ )</text--->"
    "<text>Simulation of a gps receiver. The result is a tuplestream with "
    "attributes 'Time' (instant the position is taken) and 'Position' "
    "(Position at that instant).</text--->"
    "<text>query samplempoint(Train6,"
    " [const duratione value (0 2000)] ) count </text--->"
    ") )";

const string DisturbSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint x real x real  -> mpoint </text---> "
    "<text> P disturb [ MD , SD ]</text--->"
    "<text>Disturbs an existing mpoint 'P', using a total maximum deviation of "
    "'M' and a maximun deviation of 'S' per step.</text--->"
    "<text>query train 6 disturb [200.0 , 10.0] </text--->"
    ") )";

/*
Spec for ~length~

*/
const string LengthSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint -> real </text---> "
    "<text> length( Mp [, GeoidName ] ) </text--->"
    "<text>Computes the travelled length of the movement of object Mp "
    "(like an odometer). If the optional parameter is used, the coordinates in "
    "Mp are interpreted as geographic coordinates (LON,LAT) instead of metric "
    "(X,Y)-coordinates. Valid values for GeoidName are: "
    + Geoid::getGeoIdNames() + ". Unknown GeoidName results in an UNDEF result."
    "</text--->"
    "<text>query length(train6) </text--->"
    ") )";

const string EqualizeUSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint x real [ x bool] -> mpoint </text---> "
    "<text> MP equalizeU[ Dist, Split ] </text--->"
    "<text>Tries to identify similar waypoints within moving point MP using "
    "distance-based clustering (where Dist is the maximum allowed spatial "
    "distance). Then the mpoint's support points are replaced by the clustre's "
    "centroids as far as possible. Flag 'Split' signalizes, whether units may "
    "be split to allow for a better matching.</text--->"
    "<text>query train6 equalizeU[20.0]  </text--->"
    ") )";

const string MintHatSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mint -> mint </text---> "
    "<text> hat(_) </text--->"
    "<text>"
    " Summarizes a moving integer into another moving integer"
    " consisting of at most 3 units."
    " Summarization is done in such a way that the area"
    " computed from the length of the time interval of the middle piece"
    " and the minimum number reached within that interval is maximal."
    "</text--->"
    "<text>query hat(noAtCenter)</text--->"
    ") )";


const string restrictSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mint [x int] -> mint </text---> "
    "<text> restrict( MI [, Val]) </text--->"
    "<text>Removes infinity units at the ends of the moving int 'MI'. "
    "If the optional int argument Val is given, the units are only removed if "
    "the value store within that units is equal to the given value 'Val'."
    "</text--->"
    "<text>query restrict(noAtCenter)</text--->"
    ") )";

const string speedupSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint x real -> mpoint </text---> "
    "<text> MP speedup [ F ] </text--->"
    "<text>Transform 'MP' to a moving point that moves 'F' times as fast "
    "as 'MP', but starts at the same instant and moves the same way."
    "</text--->"
    "<text>query train1 speedup[2.0]</text--->"
    ") )";

/*
Spec for ~avg\_speed~

*/
const string avg_speedSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint [ x string ] -> real </text---> "
    "<text> avg_speed ( M [, GeoidName] ) </text--->"
    "<text>Query the average speed over ground of the moving point M in unit/s."
    "If the optional string parameter is not used, coordinates in M are metric "
    "(X,Y)-pairs. Otherwise, GeoidName specifies a geoid to use for orthodrome-"
    "based speed calculation (valid GeoidNames are: " + Geoid::getGeoIdNames() +
    ") and coordinates in M must be valid geographic coordinates (LON,LAT)."
    "For an invalid GeoidName or invalid geographic coordinates in M, UNDEF is "
    "returned.</text--->"
    "<text>query avg_speed(train1)</text--->"
    ") )";

const string submoveSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint x real-> mpoint </text---> "
    "<text> MP submove [ F ] </text--->"
    "<text>Restricts a moving point 'MP' to a random interval with relative "
    "size according to the given factor 0.0<'F'<1.0.</text--->"
    "<text>query submove(train1)</text--->"
    ") )";

const string TemporalSpecMp2Onemp  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint x starttime x endtime -> x</text--->"
  "<text>mp2onemp ( mp, i1, i2 )</text--->"
  "<text>WARNING! The implementation does not match this description:\nl"
  "Return a static moving point with a location from mp, starting at i1, "
  "and ending at t2.</text--->"
  "<text> query mp2onemp (train1 ,minimum(deftime(train1)),"
  "maximum(deftime(train1)))</text--->"
  ") )";
const string TemporalSpecP2Mp  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint x starttime x endtime -> x</text--->"
  "<text>p2mp ( _,_,_,_ )</text--->"
  "<text>Create an moving point from a point and the time interval.</text--->"
  "<text>query p2mp ( point(2 3) ,theInstant(2003,11,20,6)"
  "theInstant(2003,11,20,7),100)</text--->"
  ") )";

OperatorInfo DelayOperatorInfo( "delay",
  "mpoint x mpoint [x geoid] -> mreal",
  "delay(actual, schedule movement [,geoid])",
  "At every time instance the result will reflect how many seconds is the "
  "actual movement delayed from the scheduled movement. If 'geoid' is used "
  "geographic (LON.LAT) coordinates are expected, otherwise euclidean "
  "coordinates.",
  "");

OperatorInfo DistanceTraversedOperatorInfo( "distancetraversed",
  "mpoint [x geoid] -> mreal",
  "distancetraversed(schedule [, geoid])",
  "Given a mpoint that moves continuously in space the operator will return "
  "a moving real indicating the total distance (above ground) traversed so far "
  "by the moving point. If 'geoid' is used, geographic (LON.LAT) coordinates "
  "are used, euclidean coordinates otherwise.",
  "");

OperatorInfo TurnsOperatorInfo( "turns",
  "mpoint x real x real [ x duration ] [ x bool] [ x geoid ] -> stream(tuple("
  "TimeOld instant, TimeNew instant, PosOld point, PosNew point, HeadingOld "
  "real, HeadingNew real, HeadingDiff real))",
  "turns( MP, MinDiff, MaxDiff [, MaxDur ] [, UseHeading] [, Geoid] )",
  "Given a mpoint MP the operator will return a stream of tuples describing "
  "all the object's turns where the heading changes by at least MinDiff and "
  "at most MaxDiff' degrees (both parameters' absolute values are used)."
  "The optional parameter MaxDur' specifies a maximum duration of definition "
  "gaps of the object, that will be ignored (negative durations or missing "
  "parameter are handled as 'don't care about temporal distance at all'). "
  "Static periods (where the object does not move) are considered like "
  "undefined periods. Positive 'HeadingDiff' indicates a counterclockwise "
  "rotation, negative 'HeadingDiff' indicates clockwise rotation. "
  "'HeadingDiff' is always in range ]-180.0,180.0]. If an optional Geoid is "
  "passed, MP is expected to use geographic (LON.LAT) coordinates, otherwise "
  "euclidean coordinates (X,Y) are assumed. If optional boolean parameter "
  "UseHeading is TRUE (default is FALSE) the semantic of result attributes "
  "HeadingOld and HeadingNew is 'heading' (navigational angles), otherwise "
  "'direction' (mathematical angles))",
  "");

OperatorInfo GridCellEventsOperatorInfo( "gridcellevents",
  "{upoint,mpoint} x real x real x real x real x int -> stream(tuple(Cell int, "
  "TimeEntered: instant, TimeLeft: instant, CellPrevious: int, CellNext: int))",
  "gridcellevents( o, x0, y0, wx, wy, nx )",
  "Given an object 'o' and a regular spatial grid specified by the remaining "
  "parameters (see operators 'cellnumber' and 'gridintersects'), report all "
  "visits of 'o' to grid cells. For each stay at a cell, the cell number 'Cell'"
  ", the instants when 'o' enters and leaves the cell ('TimeEntered', "
  "'TimeLeft') and the numbers of the two cell visited immediately before "
  "entering ('CellPrevious') and after leaving ('CellNext') the cell are "
  "reported. If the cell is entered from/left to a location outside the grid, "
  "the according cellnumber is set to UNDEF. Regular cell numbers are positive "
  "integers (excluding '0').",
  "");

/*
16.4.3 Operators

*/
Operator temporalisempty( "isempty",
                          TemporalSpecIsEmpty,
                          12,
                          temporalisemptymap,
                          TemporalSimpleSelect,
                          TemporalTypeMapBool );

Operator temporalequal( "=",
                        TemporalSpecEQ,
                        8,
                        temporalequalmap,
                        TemporalDualSelect,
                        TemporalTemporalTypeMapBool );

Operator temporalequal2( "equal",
                        TemporalSpecEQ2,
                        4,
                        temporalequalmap2,
                        TemporalDualSelect2,
                        TemporalTemporalTypeMapBool2 );

Operator temporalnotequal( "#",
                           TemporalSpecNE,
                           8,
                           temporalnotequalmap,
                           TemporalDualSelect,
                           TemporalTemporalTypeMapBool );

Operator temporalnotequal2( "nonequal",
                           TemporalSpecNE2,
                           4,
                           temporalnotequalmap2,
                           TemporalDualSelect2,
                           TemporalTemporalTypeMapBool2 );

Operator temporalless( "<",
                       TemporalSpecLT,
                       InstantLess,
                       Operator::SimpleSelect,
                       InstantInstantTypeMapBool );

Operator temporallessequal( "<=",
                            TemporalSpecLE,
                            InstantLessEqual,
                            Operator::SimpleSelect,
                            InstantInstantTypeMapBool );

Operator temporalgreater( ">",
                          TemporalSpecLT,
                          InstantGreater,
                          Operator::SimpleSelect,
                          InstantInstantTypeMapBool );

Operator temporalgreaterequal( ">=",
                               TemporalSpecLE,
                               InstantGreaterEqual,
                               Operator::SimpleSelect,
                               InstantInstantTypeMapBool );

Operator temporalintersects( "intersects",
                             TemporalSpecIntersects,
                             3,
                             temporalintersectsmap,
                             RangeDualSelect,
                             RangeRangeTypeMapBool );

Operator temporalinside( "inside",
                         TemporalSpecInside,
                         6,
                         temporalinsidemap,
                         RangeDualSelect,
                         RangeBaseTypeMapBool1 );

Operator temporalbefore( "before",
                         TemporalSpecBefore,
                         9,
                         temporalbeforemap,
                         RangeDualSelect,
                         RangeBaseTypeMapBool2 );

Operator temporalintersection( "intersection",
                               TemporalSpecIntersection,
                               3,
                               temporalintersectionmap,
                               RangeDualSelect,
                               RangeRangeTypeMapRange );

Operator temporalunion( "union",
                        TemporalSpecUnion,
                        3,
                        temporalunionmap,
                        RangeDualSelect,
                        RangeRangeTypeMapRange );

Operator temporalminus( "minus",
                        TemporalSpecMinus,
                        3,
                        temporalminusmap,
                        RangeDualSelect,
                        RangeRangeTypeMapRange );

Operator temporalmin( "minimum",
                      TemporalSpecMinimum,
                      3,
                      temporalminmap,
                      RangeSimpleSelect,
                      RangeTypeMapBase );

Operator temporalmax( "maximum",
                      TemporalSpecMaximum,
                      3,
                      temporalmaxmap,
                      RangeSimpleSelect,
                      RangeTypeMapBase );

Operator temporalnocomponents( "no_components",
                               TemporalSpecNoComponents,
                               7,
                               temporalnocomponentsmap,
                               TemporalSetValueSelect,
                               TemporalSetValueTypeMapInt );

Operator temporalinst( "inst",
                       TemporalSpecInst,
                       4,
                       temporalinstmap,
                       IntimeSimpleSelect,
                       IntimeTypeMapInstant );
/*
More operator specifications...

*/
Operator temporalval( "val",
                      TemporalSpecVal,
                      4,
                      temporalvalmap,
                      IntimeSimpleSelect,
                      IntimeTypeMapBase );

Operator temporalatinstant( "atinstant",
                            TemporalSpecAtInstant,
                            4,
                            temporalatinstantmap,
                            MovingSimpleSelect,
                            MovingInstantTypeMapIntime );

Operator temporalatperiods( "atperiods",
                            TemporalSpecAtPeriods,
                            4,
                            temporalatperiodsmap,
                            MovingSimpleSelect,
                            MovingPeriodsTypeMapMoving );

Operator temporalwhen( "when",
                            TemporalSpecWhen,
                            4,
                            temporalwhenmap,
                            MovingSimpleSelect,
                            WhenTM );

Operator temporaldeftime( "deftime",
                          TemporalSpecDefTime,
                          4,
                          temporaldeftimemap,
                          MovingSimpleSelect,
                          MovingTypeMapPeriods );

Operator temporaltrajectory( "trajectory",
                             TemporalSpecTrajectory,
                             MPointTrajectory,
                             Operator::SimpleSelect,
                             MovingTypeMapSpatial);

Operator temporalpresent( "present",
                          TemporalSpecPresent,
                          8,
                          temporalpresentmap,
                          MovingInstantPeriodsSelect,
                          MovingInstantPeriodsTypeMapBool);

Operator temporalpasses( "passes",
                         TemporalSpecPasses,
                         6,
                         temporalpassesmap,
                         MovingBaseSelect,
                         MovingBaseTypeMapBool);

Operator temporalinitial( "initial",
                          TemporalSpecInitial,
                          4,
                          temporalinitialmap,
                          MovingSimpleSelect,
                          MovingTypeMapIntime );

Operator temporalfinal( "final",
                        TemporalSpecFinal,
                        4,
                        temporalfinalmap,
                        MovingSimpleSelect,
                        MovingTypeMapIntime );

Operator temporalat( "at",
                     TemporalSpecAt,
                     4,
                     temporalatmap,
                     MovingBaseSelect,
                     MovingBaseTypeMapMoving );

Operator temporalbox3d( "box3d",
                        Box3dSpec,
                        5,
                        temporalbox3dmap,
                        Box3dSelect,
                        Box3dTypeMap );


Operator temporalsamplempoint("samplempoint",
                       SampleMPointSpec,
                       3,
                       samplempointmap,
                       SampleMPointSelect,
                       SampleMPointTypeMap );

Operator temporaldistance( "distance",
                           TemporalSpecDistance,
                           MPointDistance,
                           Operator::SimpleSelect,
                           MovingBaseTypeMapMReal );

Operator temporalsquareddistance( "squareddistance",
                           TemporalSpecSquaredDistance,
                           3,
                           mpointsquareddistancemap,
                           SquaredDistanceSelect,
                           SquaredDistanceTypeMap );

Operator temporalgps( "gps",
                      GPSSpec,
                      GPSVM,
                      Operator::SimpleSelect,
                      GPSTypeMap );

Operator temporaldisturb( "disturb",
                      DisturbSpec,
                      DisturbVM,
                      Operator::SimpleSelect,
                      DisturbTypeMap );

Operator temporallength( "length",
                      LengthSpec,
                      LengthVM,
                      Operator::SimpleSelect,
                      LengthTypeMap );

Operator temporalsimplify( "simplify",
                           TemporalSpecSimplify,
                           3,
                           simplifymap,
                           SimplifySelect,
                           MovingTypeMapSimplify );

Operator temporalintegrate( "integrate",
                           TemporalSpecIntegrate,
                           2,
                           integratemap,
                           IntegrateSelect,
                           TypeMapIntegrate );

Operator temporallinearize( "linearize",
                           TemporalSpecLinearize,
                           2,
                           linearizemap,
                           LinearizeSelect,
                           TypeMapLinearize );

Operator temporallinearize2( "linearize2",
                           TemporalSpecLinearize2,
                           2,
                           linearize2map,
                           LinearizeSelect,
                           TypeMapLinearize2 );

Operator temporalapproximate( "approximate",
                           TemporalSpecApproximate,
                           5,
                           approximatemap,
                           ApproximateSelect,
                           TypeMapApproximate );

Operator temporalminimum( "minimum",
                           TemporalSpecMin,
                           3,
                           minmap,
                           MinMaxSelect,
                           TypeMapMinMax );

Operator temporalmaximum( "maximum",
                           TemporalSpecMax,
                           3,
                           maxmap,
                           MinMaxSelect,
                           TypeMapMinMax );

Operator temporalbreakpoints( "breakpoints",
                           TemporalSpecBreakPoints,
                           MPointBreakPoints,
                           Operator::SimpleSelect,
                           MovingTypeMapBreakPoints );

Operator breaks( "breaks",
                 breaksSpec,
                 breaksVM,
                 Operator::SimpleSelect,
                 breaksTM);

Operator temporalgk( "gk",
                     TemporalSpecgk,
                     gkVM,
                     Operator::SimpleSelect,
                     MovingTypeMapgk );

Operator temporalvertices( "vertices",
                           TemporalSpecVertices,
                           Vertices,
                           Operator::SimpleSelect,
                           MovingTypeMapVertices );

Operator temporalunits( "units",
                        TemporalSpecUnits,
                        5,
                        temporalunitsmap,
                        MovingSimpleSelect,
                        MovingTypeMapUnits );

Operator temporalgetunit( "getunit",
                        TemporalSpecGetUnit,
                        5,
                        temporalgetunitmap,
                        MovingSimpleSelect,
                        MovingTypeMapGetUnit );

Operator temporalbbox( "bbox",
                       TemporalSpecBBox,
                       5,
                       temporalbboxmap,
                       TemporalBBoxSelect,
                       TemporalBBoxTypeMap );

Operator temporalmbrange( "mbrange",
                       TemporalSpecMBRange,
                       5,
                       temporalmbrangemap,
                       TemporalMBRangeSelect,
                       TemporalMBRangeTypeMap );

Operator temporalbbox2d( "bbox2d",
                         TemporalSpecBBox2d,
                         3,
                         temporalbbox2dmap,
                         TemporalBBox2dSelect,
                         TemporalBBox2dTypeMap );

Operator temporalbboxold( "bboxold",
                          TemporalSpecBBoxOld,
                          6,
                          temporalbboxoldmap,
                          TemporalBBoxSelect,
                          TemporalBBoxTypeMap );

Operator temporaltranslate( "translate",
                       MPointSpecTranslate,
                       MPointTranslate,
                       Operator::SimpleSelect,
                       MPointTypeMapTranslate );

Operator temporaltheyear( "theyear",
                          TemporalSpecTheYear,
                          TheYear,
                          Operator::SimpleSelect,
                          IntSetTypeMapPeriods );

Operator temporalthemonth( "themonth",
                           TemporalSpecTheMonth,
                           TheMonth,
                           Operator::SimpleSelect,
                           IntSetTypeMapPeriods );

Operator temporaltheday( "theday",
                         TemporalSpecTheDay,
                         TheDay,
                         Operator::SimpleSelect,
                         IntSetTypeMapPeriods );

Operator temporalthehour( "thehour",
                          TemporalSpecTheHour,
                          TheHour,
                          Operator::SimpleSelect,
                          IntSetTypeMapPeriods );

Operator temporaltheminute( "theminute",
                            TemporalSpecTheMinute,
                            TheMinute,
                            Operator::SimpleSelect,
                            IntSetTypeMapPeriods );

Operator temporalthesecond( "thesecond",
                            TemporalSpecTheSecond,
                            TheSecond,
                            Operator::SimpleSelect,
                            IntSetTypeMapPeriods );

Operator temporaltheperiod( "theperiod",
                            TemporalSpecThePeriod,
                            ThePeriod,
                            Operator::SimpleSelect,
                            PeriodsPeriodsTypeMapPeriods );

Operator temporalbox2d( "box2d",
                         TemporalBox2dSpec,
                         4,
                         temporalbox2dmap,
                         TemporalBox2dSelect,
                         Box2dTypeMap );

Operator mbool2mint( "mbool2mint",
                       TemporalMBool2MIntSpec,
                       MBool2MInt,
                       Operator::SimpleSelect,
                       TemporalMBool2MInt );

Operator mint2mbool( "mint2mbool",
                       TemporalMInt2MBoolSpec,
                       MInt2MBool,
                       Operator::SimpleSelect,
                       TemporalMInt2MBool );

Operator mint2mreal( "mint2mreal",
                       TemporalMInt2MRealSpec,
                       MInt2MReal,
                       Operator::SimpleSelect,
                       TemporalMInt2MReal );

Operator extdeftime( "extdeftime",
                      TemporalExtDeftimeSpec,
                      2,
                      extdeftimemap,
                      ExtDeftimeSelect,
                      ExtDeftimeTypeMap );

/*
More operator specifications...

*/
Operator temporaltherange( "theRange",
                     TemporalTheRangeSpec,
                     5,
                     temporaltherangemap,
                     TemporalTheRangeSelect,
                     TemporalTheRangeTM );

Operator temporaltranslateappend( "translateappend",
                       TranslateAppendSpec,
                       TranslateAppendVM,
                       Operator::SimpleSelect,
                       TranslateAppendTM );

Operator temporaltranslateappendS( "translateappendS",
                       TranslateAppendSSpec,
                       TranslateAppendSVM,
                       Operator::SimpleSelect,
                       TranslateAppendSTM );

Operator temporalreverse("reverse",
                       ReverseSpec,
                       ReverseVM,
                       Operator::SimpleSelect,
                       ReverseTM );

Operator equalizeU( "equalizeU",
                       EqualizeUSpec,
                       MPointEqualize,
                       Operator::SimpleSelect,
                       EqualizeUTM );

Operator hat( "hat",
                       MintHatSpec,
                       MIntHat,
                       Operator::SimpleSelect,
                       MIntHatTypeMap );

Operator restrict( "restrict",
                    restrictSpec,
                    2,
                    restrictVM,
                    restrictSelect,
                    restrictTM );

Operator speedup( "speedup",
                    speedupSpec,
                    SpeedUpVM,
                    Operator::SimpleSelect,
                    SpeedUpTypeMap);

Operator avg_speed( "avg_speed",
                    avg_speedSpec,
                    Avg_SpeedVM,
                    Operator::SimpleSelect,
                    LengthTypeMap);

Operator submove( "submove",
                    submoveSpec,
                    SubMoveVM,
                    Operator::SimpleSelect,
                    SubMoveTypeMap);

Operator temporaluval( "uval",
                      TemporalSpecUVal,
                      1,
                      temporaluvalmap,
                      UIntimeSimpleSelect,
                      UIntimeTypeMapBase );
Operator mp2onemp( "mp2onemp",
                      TemporalSpecMp2Onemp,
                      Mp2OneMpVM,
                      Operator::SimpleSelect,
                      Mp2OneMpTypeMap );
Operator p2mp( "p2mp",
                      TemporalSpecP2Mp,
                      P2MpVM,
                      Operator::SimpleSelect,
                      P2MpTypeMap );

Operator delayoperator(DelayOperatorInfo,
                       DelayOperatorValueMapping,
                       DelayOperatorTypeMapping);

Operator distancetraversedoperator(DistanceTraversedOperatorInfo,
                                   DistanceTraversedOperatorValueMapping,
                                   DistanceTraversedOperatorTypeMapping);

Operator turns( TurnsOperatorInfo,
                TurnsOperatorValueMapping,
                TurnsOperatorTypeMapping );

Operator mappingtimeshift( "timeshift",
                         MappingTimeShiftSpec,
                         4,
                         MappingTimeShiftMap,
                         MovingSimpleSelect,
                         MappingTimeShiftTM );

Operator gridcellevents(  GridCellEventsOperatorInfo,
                          GridCellEventsValueMapping,
                          GridCellEventsSelect,
                          GridCellEventsTypeMapping );


/*
5.2 Further Operators

5.2.1 createCellGrid2D

~TypeMapping~ and ~Selection Function~

*/

static complexTM getCreateCellGrid2DCTM(){
   complexTM tm;
   tm.add(tm5<CcInt, CcInt, CcInt, CcInt, CcInt, CellGrid2D>());
   tm.add(tm5<CcInt, CcInt, CcInt, CcReal, CcInt, CellGrid2D>());
   tm.add(tm5<CcInt, CcInt, CcReal, CcInt, CcInt, CellGrid2D>());
   tm.add(tm5<CcInt, CcInt, CcReal, CcReal, CcInt, CellGrid2D>());
   tm.add(tm5<CcInt, CcReal, CcInt, CcInt, CcInt, CellGrid2D>());
   tm.add(tm5<CcInt, CcReal, CcInt, CcReal, CcInt, CellGrid2D>());
   tm.add(tm5<CcInt, CcReal, CcReal, CcInt, CcInt, CellGrid2D>());
   tm.add(tm5<CcInt, CcReal, CcReal, CcReal, CcInt, CellGrid2D>());
   tm.add(tm5<CcReal, CcInt, CcInt, CcInt, CcInt, CellGrid2D>());
   tm.add(tm5<CcReal, CcInt, CcInt, CcReal, CcInt, CellGrid2D>());
   tm.add(tm5<CcReal, CcInt, CcReal, CcInt, CcInt, CellGrid2D>());
   tm.add(tm5<CcReal, CcInt, CcReal, CcReal, CcInt, CellGrid2D>());
   tm.add(tm5<CcReal, CcReal, CcInt, CcInt, CcInt, CellGrid2D>());
   tm.add(tm5<CcReal, CcReal, CcInt, CcReal, CcInt, CellGrid2D>());
   tm.add(tm5<CcReal, CcReal, CcReal, CcInt, CcInt, CellGrid2D>());
   tm.add(tm5<CcReal, CcReal, CcReal, CcReal, CcInt, CellGrid2D>());
   return tm;
}


static ListExpr createCellGrid2DTypeMap(ListExpr args){
  return getCreateCellGrid2DCTM()(args);
}

static int createCellGrid2DSelect(ListExpr args){
   return getCreateCellGrid2DCTM().select(args);
}


/*
~Functor~ and ~Value Mapping Array~

*/

template<class A1,class A2, class A3, class A4, class A5, class R>
class CreateCellGrid2DF{
public:
  void operator()(const A1* a1, const A2* a2, const A3* a3,
                  const A4* a4, const A5* a5, R* res){
   res->set(a1->GetValue(), a2->GetValue(), a3->GetValue(),
            a4->GetValue(), a5->GetValue());
  }
};



ValueMapping createCellGrid2DValueMap[] = {
  GenVM5<CcInt, CcInt, CcInt, CcInt, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcInt, CcInt, CcInt, CcInt, CcInt, CellGrid2D> >,

  GenVM5<CcInt, CcInt, CcInt, CcReal, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcInt, CcInt, CcInt, CcReal, CcInt, CellGrid2D> >,

  GenVM5<CcInt, CcInt, CcReal, CcInt, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcInt, CcInt, CcReal, CcInt, CcInt, CellGrid2D> >,

  GenVM5<CcInt, CcInt, CcReal, CcReal, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcInt, CcInt, CcReal, CcReal, CcInt, CellGrid2D> >,

  GenVM5<CcInt, CcReal, CcInt, CcInt, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcInt, CcReal, CcInt, CcInt, CcInt, CellGrid2D> >,

  GenVM5<CcInt, CcReal, CcInt, CcReal, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcInt, CcReal, CcInt, CcReal, CcInt, CellGrid2D> >,

  GenVM5<CcInt, CcReal, CcReal, CcInt, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcInt, CcReal, CcReal, CcInt, CcInt, CellGrid2D> >,

  GenVM5<CcInt, CcReal, CcReal, CcReal, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcInt, CcReal, CcReal, CcReal, CcInt, CellGrid2D> >,

  GenVM5<CcReal, CcInt, CcInt, CcInt, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcReal, CcInt, CcInt, CcInt, CcInt, CellGrid2D> >,

  GenVM5<CcReal, CcInt, CcInt, CcReal, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcReal, CcInt, CcInt, CcReal, CcInt, CellGrid2D> >,

  GenVM5<CcReal, CcInt, CcReal, CcInt, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcReal, CcInt, CcReal, CcInt, CcInt, CellGrid2D> >,

  GenVM5<CcReal, CcInt, CcReal, CcReal, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcReal, CcInt, CcReal, CcReal, CcInt, CellGrid2D> >,

  GenVM5<CcReal, CcReal, CcInt, CcInt, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcReal, CcReal, CcInt, CcInt, CcInt, CellGrid2D> >,

  GenVM5<CcReal, CcReal, CcInt, CcReal, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcReal, CcReal, CcInt, CcReal, CcInt, CellGrid2D> >,

  GenVM5<CcReal, CcReal, CcReal, CcInt, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcReal, CcReal, CcReal, CcInt, CcInt, CellGrid2D> >,

  GenVM5<CcReal, CcReal, CcReal, CcReal, CcInt, CellGrid2D,
         CreateCellGrid2DF<CcReal, CcReal, CcReal, CcReal, CcInt, CellGrid2D> >
};


/*
~Operator Instance~

*/

Operator createCellGrid2D(
         "createCellGrid2D",
          getCreateCellGrid2DCTM().getSpecification(
              "createCellGrid2D(x0, y0, xw, yw, no_cells_x) ",
              "Creates a cell grid from the arguments, (x0, y0) "
              " defines a corner point of the grid, xw, yw are "
              " the width for each dimension and no_cells_x is"
              " the number of cells in x direction",
              "query createCellgrid2D(1.0, 1.0, 3.0, 3.0, 5)"),
          getCreateCellGrid2DCTM().getVMCount(),
          createCellGrid2DValueMap,
          createCellGrid2DSelect,
          createCellGrid2DTypeMap);


/*
5.2.2 Operator ~getRefinementPartition~

*/

/*
5.2.2.1 Type Mapping for Operator ~getRefinementPartition~

----
{mT1|uT1} x {uT2|mT2} -> stream(tuple((Tstart instant)
                               (Tend instant)
                               (Tlc bool)
                               (Trc bool)
                               (Unit1 uT1)
                               (Unit2 uT2)
                               (UnitNo1 int)
                               (UnitNo2 int))), where
                    T1, T2 in {point, real, int, bool, string}
----

Type ~MRegion~ cannot be supported due to its representation using ~URegionEmb~
internally, but ~URegion~ externally.

*/
ListExpr GetRefinementPartitionTypeMapping(ListExpr args){
  int noargs = nl->ListLength(args);
  string errmsg = "Expected {uT1|mT1} x {uT2|mT2}, where T in "
                  "{point, int, real, bool, string}";
  if(noargs!=2){
    return listutils::typeError(errmsg);
  }
  set<string> supportedArgTypes;
  supportedArgTypes.insert(MPoint::BasicType());
  supportedArgTypes.insert(MReal::BasicType());
  supportedArgTypes.insert(MInt::BasicType());
  supportedArgTypes.insert(MBool::BasicType());
  supportedArgTypes.insert(MString::BasicType());
  supportedArgTypes.insert(UPoint::BasicType());
  supportedArgTypes.insert(UReal::BasicType());
  supportedArgTypes.insert(UInt::BasicType());
  supportedArgTypes.insert(UBool::BasicType());
  supportedArgTypes.insert(UString::BasicType());

  // MRegion not supported due to problems with handling URegionEmb/URegion

  ListExpr first = nl->First(args);
  ListExpr second= nl->Second(args);

  if( !listutils::isASymbolIn(first,supportedArgTypes) ||
      !listutils::isASymbolIn(second,supportedArgTypes)) {
    return listutils::typeError(errmsg);
  }

  map<string,string> tm;
  tm.insert(pair<string,string>(MPoint::BasicType(),UPoint::BasicType()));
  tm.insert(pair<string,string>(MReal::BasicType(),UReal::BasicType()));
  tm.insert(pair<string,string>(MInt::BasicType(),UInt::BasicType()));
  tm.insert(pair<string,string>(MBool::BasicType(),UBool::BasicType()));
  tm.insert(pair<string,string>(MString::BasicType(),UString::BasicType()));
  tm.insert(pair<string,string>(UPoint::BasicType(),UPoint::BasicType()));
  tm.insert(pair<string,string>(UReal::BasicType(),UReal::BasicType()));
  tm.insert(pair<string,string>(UInt::BasicType(),UInt::BasicType()));
  tm.insert(pair<string,string>(UBool::BasicType(),UBool::BasicType()));
  tm.insert(pair<string,string>(UString::BasicType(),UString::BasicType()));
  // MRegion not supported due to problems with handling URegionEmb/URegion

  string t1; nl->WriteToString(t1, first);
  string t2; nl->WriteToString(t2, second);
  map<string,string>::iterator r1_i = tm.find(t1);
  map<string,string>::iterator r2_i = tm.find(t2);
  if((r1_i == tm.end()) || (r2_i == tm.end())){
    return listutils::typeError(errmsg);
  }
  NList resTupleType =NList(NList("Tstart"),
                            NList(Instant::BasicType())).enclose();
  resTupleType.append(NList(NList("Tend"),NList(Instant::BasicType())));
  resTupleType.append(NList(NList("Tlc"),NList(CcBool::BasicType())));
  resTupleType.append(NList(NList("Trc"),NList(CcBool::BasicType())));
  resTupleType.append(NList(NList("Unit1"),NList(r1_i->second)));
  resTupleType.append(NList(NList("Unit2"),NList(r2_i->second)));
  resTupleType.append(NList(NList("UnitNo1"),NList(CcInt::BasicType())));
  resTupleType.append(NList(NList("UnitNo2"),NList(CcInt::BasicType())));
  NList resType =
  NList(NList(Symbol::STREAM()),NList(NList(Tuple::BasicType()),resTupleType));
  return resType.listExpr();
}
/*
5.2.2.2 Value Mapping for Operator ~getRefinementPartition~

*/
template<class M1, class U1, class M2, class U2>
class RefinementPartitionLI {
  public:

    RefinementPartitionLI(M1* _m1, M2* _m2,
                          ListExpr _restype,
                          const bool _copy1, const bool _copy2)
    :m1(_m1), m2(_m2), r(0),
    restupletype(0), hasmore(false), m1_copy(0), m2_copy(0)
    {
      if(_copy1){
        m1_copy = _m1;
      }
      if(_copy2){
        m2_copy = _m2;
      }
      restupletype = new TupleType(nl->Second(_restype));
      r = new RefinementStream<M1,M2,U1,U2>(m1,m2);
      hasmore = r->hasNext();
    }

    ~RefinementPartitionLI(){
      if(restupletype){
        restupletype->DeleteIfAllowed();
        restupletype = 0;
      }
      if(r){
        delete r;
        r = 0;
      }
      if(m1_copy){
        m1_copy->DeleteIfAllowed();
        m1_copy = 0;
      }
      if(m2_copy){
        m2_copy->DeleteIfAllowed();
        m2_copy = 0;
      }
    }
    bool hasMore(){
      return hasmore;
    } const

    void next(Tuple* &t){
      // get next pairing
      Interval<Instant> iv;
      int pos1;
      int pos2;
      if(r->getNext(iv, pos1, pos2)){
        // get unit position data
        CcInt* unit_no1 = new CcInt((pos1>=0), pos1);
        CcInt* unit_no2 = new CcInt((pos2>=0), pos2);

        // get interval data
        Instant* tstart = new DateTime(instanttype);
        Instant* tend   = new DateTime(instanttype);
        CcBool*  tlc = new CcBool(true, true);
        CcBool*  trc = new CcBool(true, true);
        *tstart = iv.start;
        *tend   = iv.end;
        tlc->Set(true, iv.lc);
        trc->Set(true, iv.rc);

        // get unit data
        U1 u1(true);
        U2 u2(true);
        U1* unit1 = new U1(true);
        U2* unit2 = new U2(true);
        if(pos1 >= 0){
          m1->Get(pos1, u1);
          u1.AtInterval(iv, *unit1);
        } else {
          unit1->SetDefined(false);
        }
        if(pos2 >= 0){
          m2->Get(pos2, u2);
          u2.AtInterval(iv, *unit2);
        } else {
          unit2->SetDefined(false);
        }
        // create the tuple
        Tuple* restuple = new Tuple(restupletype);
        restuple->PutAttribute(0,tstart);
        restuple->PutAttribute(1,tend);
        restuple->PutAttribute(2,tlc);
        restuple->PutAttribute(3,trc);
        restuple->PutAttribute(4,unit1);
        restuple->PutAttribute(5,unit2);
        restuple->PutAttribute(6,unit_no1);
        restuple->PutAttribute(7,unit_no2);

        // actualize hasmore flag and return restuple
        hasmore = r->hasNext();
        t = restuple;
      } else { // no more results -> clear hasmore and return nullpointer
        hasmore = false;
        t = 0;
      }
    }

  private:
    const M1* m1;
    const M2* m2;
    RefinementStream<M1,M2,U1,U2>* r;
    TupleType* restupletype;
    bool hasmore;
    M1* m1_copy;
    M2* m2_copy;
};

template<class M1, class U1, class M2, class U2, bool IsUnit1, bool IsUnit2>
int GetRefinementPartitionVM( Word* args, Word& result, int message,
                              Word& local, Supplier s ){
  RefinementPartitionLI<M1,U1,M2,U2>* li;
  switch( message )
  {
    case OPEN:{
      if(local.addr){
        delete static_cast<RefinementPartitionLI<M1,U1,M2,U2>*>(local.addr);
        local.setAddr(0);
      }
      M1* mo1;
      if(IsUnit1){
        mo1 = new M1(1);
        U1* unit1 = static_cast<U1*>(args[0].addr);
        if(unit1->IsDefined()){
          mo1->Add(*unit1);
        }
      } else {
        mo1 = static_cast<M1*>(args[0].addr);
      }
      M2* mo2;
      if(IsUnit2){
        mo2 = new M2(1);
        U2* unit2 = static_cast<U2*>(args[1].addr);
        if(unit2->IsDefined()){
          mo2->Add(*unit2);
        }
      } else {
        mo2 = static_cast<M2*>(args[1].addr);
      }
      li = new
        RefinementPartitionLI<M1,U1,M2,U2>(mo1,
                                           mo2,
                                           GetTupleResultType(s),
                                           IsUnit1,IsUnit2);
      local.setAddr(li);
      return 0;
    }
    case REQUEST:{
      if(!local.addr){
        return CANCEL;
      }
      li = static_cast<RefinementPartitionLI<M1,U1,M2,U2>*>(local.addr);
      Tuple* t = 0;
      if(li->hasMore()){
        li->next(t);
        if(t){
          result.setAddr(t);
          return YIELD;
        }
      }
      return CANCEL;
    }
    case CLOSE:{
      if(local.addr){
        delete static_cast<RefinementPartitionLI<M1,U1,M2,U2>*>(local.addr);
        local.setAddr(0);
      }
      return 0;
    }
    default:{
      cerr << __PRETTY_FUNCTION__ << "Unknown message = " << message << "."
           << endl;
      return -1;
    }
  } // end switch
  cerr << __PRETTY_FUNCTION__ << "Unknown message = " << message << "."
       << endl;
  return -1;
}
/*
5.2.2.3 Value Mapping Array for Operator ~getRefinementPartition~

*/
ValueMapping GetRefinementPartitionValueMapping[] = {
  GetRefinementPartitionVM<MPoint,UPoint,MPoint,UPoint,false,false>, // 0
  GetRefinementPartitionVM<MPoint,UPoint,MReal,UReal,false,false>,
  GetRefinementPartitionVM<MPoint,UPoint,MInt,UInt,false,false>,
  GetRefinementPartitionVM<MPoint,UPoint,MBool,UBool,false,false>,
  GetRefinementPartitionVM<MPoint,UPoint,MString,UString,false,false>,
  GetRefinementPartitionVM<MPoint,UPoint,MPoint,UPoint,false,true>,
  GetRefinementPartitionVM<MPoint,UPoint,MReal,UReal,false,true>,
  GetRefinementPartitionVM<MPoint,UPoint,MInt,UInt,false,true>,
  GetRefinementPartitionVM<MPoint,UPoint,MBool,UBool,false,true>,
  GetRefinementPartitionVM<MPoint,UPoint,MString,UString,false,true>, // 9

  GetRefinementPartitionVM<MReal,UReal,MPoint,UPoint,false,false>, //10
  GetRefinementPartitionVM<MReal,UReal,MReal,UReal,false,false>,
  GetRefinementPartitionVM<MReal,UReal,MInt,UInt,false,false>,
  GetRefinementPartitionVM<MReal,UReal,MBool,UBool,false,false>,
  GetRefinementPartitionVM<MReal,UReal,MString,UString,false,false>,
  GetRefinementPartitionVM<MReal,UReal,MPoint,UPoint,false,true>,
  GetRefinementPartitionVM<MReal,UReal,MReal,UReal,false,true>,
  GetRefinementPartitionVM<MReal,UReal,MInt,UInt,false,true>,
  GetRefinementPartitionVM<MReal,UReal,MBool,UBool,false,true>,
  GetRefinementPartitionVM<MReal,UReal,MString,UString,false,true>, //19

  GetRefinementPartitionVM<MInt,UInt,MPoint,UPoint,false,false>, //20
  GetRefinementPartitionVM<MInt,UInt,MReal,UReal,false,false>,
  GetRefinementPartitionVM<MInt,UInt,MInt,UInt,false,false>,
  GetRefinementPartitionVM<MInt,UInt,MBool,UBool,false,false>,
  GetRefinementPartitionVM<MInt,UInt,MString,UString,false,false>,
  GetRefinementPartitionVM<MInt,UInt,MPoint,UPoint,false,true>,
  GetRefinementPartitionVM<MInt,UInt,MReal,UReal,false,true>,
  GetRefinementPartitionVM<MInt,UInt,MInt,UInt,false,true>,
  GetRefinementPartitionVM<MInt,UInt,MBool,UBool,false,true>,
  GetRefinementPartitionVM<MInt,UInt,MString,UString,false,true>, //29

  GetRefinementPartitionVM<MBool,UBool,MPoint,UPoint,false,false>, //30
  GetRefinementPartitionVM<MBool,UBool,MReal,UReal,false,false>,
  GetRefinementPartitionVM<MBool,UBool,MInt,UInt,false,false>,
  GetRefinementPartitionVM<MBool,UBool,MBool,UBool,false,false>,
  GetRefinementPartitionVM<MBool,UBool,MString,UString,false,false>, //34
  GetRefinementPartitionVM<MBool,UBool,MPoint,UPoint,false,true>, //35
  GetRefinementPartitionVM<MBool,UBool,MReal,UReal,false,true>,
  GetRefinementPartitionVM<MBool,UBool,MInt,UInt,false,true>,
  GetRefinementPartitionVM<MBool,UBool,MBool,UBool,false,true>,
  GetRefinementPartitionVM<MBool,UBool,MString,UString,false,true>, //39

  GetRefinementPartitionVM<MString,UString,MPoint,UPoint,false,false>, //40
  GetRefinementPartitionVM<MString,UString,MReal,UReal,false,false>,
  GetRefinementPartitionVM<MString,UString,MInt,UInt,false,false>,
  GetRefinementPartitionVM<MString,UString,MBool,UBool,false,false>,
  GetRefinementPartitionVM<MString,UString,MString,UString,false,false>, //44
  GetRefinementPartitionVM<MString,UString,MPoint,UPoint,false,true>, //45
  GetRefinementPartitionVM<MString,UString,MReal,UReal,false,true>,
  GetRefinementPartitionVM<MString,UString,MInt,UInt,false,true>,
  GetRefinementPartitionVM<MString,UString,MBool,UBool,false,true>,
  GetRefinementPartitionVM<MString,UString,MString,UString,false,true>, //49

  GetRefinementPartitionVM<MPoint,UPoint,MPoint,UPoint,true,false>, // 50
  GetRefinementPartitionVM<MPoint,UPoint,MReal,UReal,true,false>,
  GetRefinementPartitionVM<MPoint,UPoint,MInt,UInt,true,false>,
  GetRefinementPartitionVM<MPoint,UPoint,MBool,UBool,true,false>,
  GetRefinementPartitionVM<MPoint,UPoint,MString,UString,true,false>,
  GetRefinementPartitionVM<MPoint,UPoint,MPoint,UPoint,true,true>,
  GetRefinementPartitionVM<MPoint,UPoint,MReal,UReal,true,true>,
  GetRefinementPartitionVM<MPoint,UPoint,MInt,UInt,true,true>,
  GetRefinementPartitionVM<MPoint,UPoint,MBool,UBool,true,true>,
  GetRefinementPartitionVM<MPoint,UPoint,MString,UString,true,true>, // 59

  GetRefinementPartitionVM<MReal,UReal,MPoint,UPoint,true,false>, //60
  GetRefinementPartitionVM<MReal,UReal,MReal,UReal,true,false>,
  GetRefinementPartitionVM<MReal,UReal,MInt,UInt,true,false>,
  GetRefinementPartitionVM<MReal,UReal,MBool,UBool,true,false>,
  GetRefinementPartitionVM<MReal,UReal,MString,UString,true,false>,
  GetRefinementPartitionVM<MReal,UReal,MPoint,UPoint,true,true>,
  GetRefinementPartitionVM<MReal,UReal,MReal,UReal,true,true>,
  GetRefinementPartitionVM<MReal,UReal,MInt,UInt,true,true>,
  GetRefinementPartitionVM<MReal,UReal,MBool,UBool,true,true>,
  GetRefinementPartitionVM<MReal,UReal,MString,UString,true,true>, //69

  GetRefinementPartitionVM<MInt,UInt,MPoint,UPoint,true,false>, //70
  GetRefinementPartitionVM<MInt,UInt,MReal,UReal,true,false>,
  GetRefinementPartitionVM<MInt,UInt,MInt,UInt,true,false>,
  GetRefinementPartitionVM<MInt,UInt,MBool,UBool,true,false>,
  GetRefinementPartitionVM<MInt,UInt,MString,UString,true,false>,
  GetRefinementPartitionVM<MInt,UInt,MPoint,UPoint,true,true>,
  GetRefinementPartitionVM<MInt,UInt,MReal,UReal,true,true>,
  GetRefinementPartitionVM<MInt,UInt,MInt,UInt,true,true>,
  GetRefinementPartitionVM<MInt,UInt,MBool,UBool,true,true>,
  GetRefinementPartitionVM<MInt,UInt,MString,UString,true,true>, //79

  GetRefinementPartitionVM<MBool,UBool,MPoint,UPoint,true,false>, //80
  GetRefinementPartitionVM<MBool,UBool,MReal,UReal,true,false>,
  GetRefinementPartitionVM<MBool,UBool,MInt,UInt,true,false>,
  GetRefinementPartitionVM<MBool,UBool,MBool,UBool,true,false>,
  GetRefinementPartitionVM<MBool,UBool,MString,UString,true,false>,
  GetRefinementPartitionVM<MBool,UBool,MPoint,UPoint,true,true>,
  GetRefinementPartitionVM<MBool,UBool,MReal,UReal,true,true>,
  GetRefinementPartitionVM<MBool,UBool,MInt,UInt,true,true>,
  GetRefinementPartitionVM<MBool,UBool,MBool,UBool,true,true>,
  GetRefinementPartitionVM<MBool,UBool,MString,UString,true,true>, //89

  GetRefinementPartitionVM<MString,UString,MPoint,UPoint,true,false>, //90
  GetRefinementPartitionVM<MString,UString,MReal,UReal,true,false>,
  GetRefinementPartitionVM<MString,UString,MInt,UInt,true,false>,
  GetRefinementPartitionVM<MString,UString,MBool,UBool,true,false>,
  GetRefinementPartitionVM<MString,UString,MString,UString,true,false>,
  GetRefinementPartitionVM<MString,UString,MPoint,UPoint,true,true>,
  GetRefinementPartitionVM<MString,UString,MReal,UReal,true,true>,
  GetRefinementPartitionVM<MString,UString,MInt,UInt,true,true>,
  GetRefinementPartitionVM<MString,UString,MBool,UBool,true,true>,
  GetRefinementPartitionVM<MString,UString,MString,UString,true,true> //99
};

/*
5.2.2.4 Selection Function for Operator ~getRefinementPartition~

*/
int GetRefinementPartitionSelect( ListExpr args ) {
  int res = 0;
  // first arg type
  if(listutils::isSymbol(nl->First(args),
    MPoint::BasicType())) { res+=0; }
  else if(listutils::isSymbol(nl->First(args),
    MReal::BasicType())) { res+=10; }
  else if(listutils::isSymbol(nl->First(args),
    MInt::BasicType()))  { res+=20; }
  else if(listutils::isSymbol(nl->First(args),
    MBool::BasicType())) { res+=30; }
  else if(listutils::isSymbol(nl->First(args),
    MString::BasicType())) { res+=40; }
  else if(listutils::isSymbol(nl->First(args),
    UPoint::BasicType())) { res+=50; }
  else if(listutils::isSymbol(nl->First(args),
    UReal::BasicType())) { res+=60; }
  else if(listutils::isSymbol(nl->First(args),
    UInt::BasicType())) { res+=70; }
  else if(listutils::isSymbol(nl->First(args),
    UBool::BasicType())) { res+=80; }
  else if(listutils::isSymbol(nl->First(args),
    UString::BasicType())) { res+=90; }
  else {res+= -9999999; }
  // second arg type
  if(listutils::isSymbol(nl->Second(args),MPoint::BasicType())) { res+=0; }
  else if(listutils::isSymbol(nl->Second(args),
    MReal::BasicType())) { res+=1; }
  else if(listutils::isSymbol(nl->Second(args),
    MInt::BasicType())) { res+=2; }
  else if(listutils::isSymbol(nl->Second(args),
    MBool::BasicType())) { res+=3; }
  else if(listutils::isSymbol(nl->Second(args),
    MString::BasicType())) { res+=4; }
  else if(listutils::isSymbol(nl->Second(args),
    UPoint::BasicType())) { res+=5; }
  else if(listutils::isSymbol(nl->Second(args),
    UReal::BasicType())) { res+=6; }
  else if(listutils::isSymbol(nl->Second(args),
    UInt::BasicType())) { res+=7; }
  else if(listutils::isSymbol(nl->Second(args),
    UBool::BasicType())) { res+=8; }
  else if(listutils::isSymbol(nl->Second(args),
    UString::BasicType())) { res+=9; }
  else {res+= -9999999; }

  return (((res>=0)&&(res<=99))?res:-1);
}
/*
5.2.2.5 Specification for Operator ~getRefinementPartition~

*/
OperatorInfo GetRefinementPartitionOperatorInfo(
"getRefinementPartion",
"{mT1|uT1} x {mT2|uT2} -> stream(tuple((Tstart instant)(Tend instant)(Tlc bool)"
"(Trc bool)(Unit1 uT1)(Unit2 uT2)(UnitNo1 int)(UnitNo2 int))); T1, T2 in "
"{point, real, int, bool, string}",
"getRefinementPartion( M1, M2 )",
"Creates a stream representing the temporal refinement partion of the two "
"arguments as a stream of tuples. Each result tuple contains a temporal "
"interval (represented by starting instant Tstart, ending instant Tend, and "
"closedness parameters Tlc (Tstart included), Trc (Tend includes)), "
"restrictions of both arguments, M1 and M2, to this interval, and the position "
"indexes of the according original units within M1 (UnitNo1), M2 (UnitNo2). "
"If for a given interval one of the arguments is not defined, the according "
"result unit is set to UNDEFINED. If one argument is UNDEFINED, the result "
"contains the original units of the other, defined, argument. If M1 and M2 are "
"both undefined, or both are empty (do not contain any unit) the result stream "
"is empty.",
"query getRefinementPartion(train1, train5) count"
""
);


/*
5.2.2.6 Operator definition for ~getRefinementPartition~

*/
Operator getrefinementpartition(
  GetRefinementPartitionOperatorInfo,
  GetRefinementPartitionValueMapping,
  GetRefinementPartitionSelect,
  GetRefinementPartitionTypeMapping
);

/*
5.2.3 Operator atRect

5.2.3.1 Type Maspping

   Signature is: mpoint x rect -> mpoint

*/

ListExpr atRectTM(ListExpr args){
  string err ="mpoint x rect expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(!MPoint::checkType(nl->First(args)) ||
     !Rectangle<2>::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return nl->SymbolAtom(MPoint::BasicType());
}

/*
5.2.3.2 Value Mapping

*/

int atRectVM( Word* args, Word& result, int message, Word&
 local, Supplier s ){
  MPoint* mp = (MPoint*) args[0].addr;
  Rectangle<2>* rect = (Rectangle<2>*) args[1].addr;
  result = qp->ResultStorage(s);
  MPoint* res = (MPoint*) result.addr;
  mp->AtRect(*rect,*res);
  return 0;
}

/*
5.2.3.3 Specification

*/
const string atRectSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint x rect -> mpoint </text---> "
    "<text> mp atRect r  </text--->"
    "<text>Restricts the moving point mp to the part inside r "
    "</text--->"
    "<text>query train7 atRect bbox(thecenter)</text--->"
    ") )";

/*
5.2.3.4 Operator instance

*/
Operator atRect( "atRect",
                 atRectSpec,
                 atRectVM,
                 Operator::SimpleSelect,
                 atRectTM);


/*
5.2.4 Operator moveTo

This operator changes the start instant of a moving object
to the given object.

5.2.4.1 Type Mapping

Signature:  mT x instant -> mT with T in { point, int, real }

*/
ListExpr moveToTM(ListExpr args){
   string err = "mt x instant with T in {point, int, real, bool} expected";
   if(!nl->HasLength(args,2)){
     return listutils::typeError(err + " (wrong number of arguments)");
   }
   if(!Instant::checkType(nl->Second(args))){
     return listutils::typeError(err + 
                                " (second argument is not an instant)");
   }
   ListExpr first = nl->First(args);
   if( MPoint::checkType(first) ||
       MInt::checkType(first) ||
       MReal::checkType(first) ||
       MBool::checkType(first)  ){ // extend this list 
      return first;
   }
   return listutils::typeError(err + " (unsupported first argument)");
}

/*
5.2.4.2 Value Mappings 

*/

template<class MT>
int moveToVM1( Word* args, Word& result, int message, Word&
              local, Supplier s ){

  result = qp->ResultStorage(s);
  MT* res = (MT*) result.addr;
  MT* arg1 = (MT*) args[0].addr;
  DateTime* instant = (DateTime*) args[1].addr;
  arg1->moveTo(*instant, *res);
  return 0;
}


/*
5.2.4.3 Value Mapping array and Selection function

*/
ValueMapping moveToVM[] = {
        moveToVM1<MPoint>,
        moveToVM1<MInt>,
        moveToVM1<MReal>,
        moveToVM1<MBool>   
     };


int moveToSelect(ListExpr args){
  ListExpr f = nl->First(args);
  if(MPoint::checkType(f)) {
     return 0;
  }
  if(MInt::checkType(f)) {
     return 1;
  }
  if(MReal::checkType(f)) {
     return 2;
  }
  if(MBool::checkType(f)) {
     return 3;
  }
  return -1;
}

/*
5.2.3.4 Specification

*/

const string moveToSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mT x instant -> mT with T in {point, int, real, bool)"
    "</text---> "
    "<text> mT moveTo [instant] </text--->"
    "<text>Changes the start instant of an moving object "
    "</text--->"
    "<text>query train7 moveTo[ now() ]</text--->"
    ") )";

/*
5.2.3.5 Operator Instance

*/
Operator moveTo(
          "moveTo",
          moveToSpec,
          4,
          moveToVM,
          moveToSelect,
          moveToTM);


/*
5.2.4 Operator fillGaps

This operator removes gaps within a periods value. 
If the optional parameter ~d~ of type duration is given,
only such gaps having a duration smaller than or equal
to ~d~ are removed.

5.2.4.1 Type Mapping

*/
ListExpr fillGapsTM(ListExpr args){
  int len = nl->ListLength(args);
  string err = "periods [x duration] expected";
  if( (len!=1) && (len!=2)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(!Periods::checkType(nl->First(args))){
    return listutils::typeError(err 
                             + " (first argument not of type periods)");
  }
  if( (len==2) && !Duration::checkType(nl->Second(args))){
    return listutils::typeError(err 
                          + " ( second parameter is not a duration)");
  }
  return nl->SymbolAtom(Periods::BasicType());
}

/*
5.2.4.2 Value Mapping

*/
int fillGapsVM( Word* args, Word& result, int message, Word&
              local, Supplier s ){

  result = qp->ResultStorage(s);
  Periods* res = (Periods*) result.addr;
 
  Periods* p = (Periods*) args[0].addr;

  res->Clear();

  if(!p->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  res->SetDefined(true);
  if(p->IsEmpty()){ // no content, no gap
     return 0; 
  }
  if(qp->GetNoSons(s)==1){ // without duration
    Interval<Instant> iv1;
    p->Get(0,iv1);
    Interval<Instant> iv2;
    p->Get(p->GetNoComponents()-1, iv2);
    Interval<Instant> iv(iv1.start, iv2.end, iv1.lc, iv2.rc);
    res->StartBulkLoad();
    res->Add(iv);
    res->EndBulkLoad(false);
    return 0;
  }
  // the second parameter is given
  DateTime* dur = (DateTime*) args[1].addr;
  if(!dur->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  res->StartBulkLoad();
  int size = p->GetNoComponents();
  for(int i=0;i<size;i++){
    Interval<Instant> iv1;
    p->Get(i,iv1);
    res->MergeAdd(iv1);
    if(i<size-1){ // compute gap
      Interval<Instant> iv2;
      p->Get(i+1,iv2);
      if((iv2.start-iv1.end) <= (*dur)){
         Interval<Instant> iv(iv1.end, iv2.start, !iv1.rc, !iv2.lc);
         res->MergeAdd(iv);
      }
    }
  }
  res->EndBulkLoad(false);
  return 0; 
}

/*
5.2.4.3 Specification

*/

const string fillGapsSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> periods [x duration] -> periods"
    "</text---> "
    "<text> fillGaps(_ [,_] ) </text--->"
    "<text>Fills gaps within a periods value. If the optional  "
    " argument d is given, only gaps shorter or having the same"
    " duration as d are removed."
    "</text--->"
    "<text>query fillGaps(deftime(train7))</text--->"
    ") )";

/*
5.2.4.4 Operator Instance

*/
Operator fillGaps(
           "fillGaps",
           fillGapsSpec,
           fillGapsVM,
           Operator::SimpleSelect,
           fillGapsTM);


/*
5.3.5 Operator removeShort

This operator removes intervals shorther an a given duration from 
a periods value.

5.3.5.1 Type Mapping

Signature: periods x duration -> periods

*/
ListExpr removeShortTM(ListExpr args){
  string err = "periods x duration expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of arguments");
  }
  if(!Periods::checkType(nl->First(args)) ||
     !Duration::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return nl->SymbolAtom(Periods::BasicType());
}

/*
5.3.5.2 Value Mapping 

*/

int removeShortVM( Word* args, Word& result, int message, Word&
                   local, Supplier s ){

   result = qp->ResultStorage(s);
   Periods* res = (Periods*) result.addr;
   Periods* p = (Periods*) args[0].addr;
   DateTime* dur = (DateTime*) args[1].addr;

   res->Clear();
   if(!p->IsDefined() || !dur->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   res->SetDefined(true);
   res->StartBulkLoad();
   Interval<Instant> iv;
   int size = p->GetNoComponents();
   for(int i=0;i<size;i++){
      p->Get(i,iv);
      DateTime d(datetime::durationtype);
      d = iv.end - iv.start;
      if( (d > (*dur))  ||
          (d == (*dur)   && iv.lc && iv.rc)){
         res->Add(iv);
      }
   }
   res->EndBulkLoad(false);
   return 0;
}

/*
5.3.5.3 Specification

*/

const string removeShortSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> periods x duration -> periods"
    "</text---> "
    "<text> removeShort( p ,d ) </text--->"
    "<text>Removes intervals shorter than d from p. "
    "</text--->"
    "<text>query removeShort(deftime(train7),"
    "[const duration value (1 0)]) </text--->"
    ") )";

/*
5.2.5.4 Operator instance

*/
Operator removeShort(
           "removeShort",
           removeShortSpec,
           removeShortVM,
           Operator::SimpleSelect,
           removeShortTM);



/*
5.2.6 Operator getIntervals

This operator splits a periods value into single interval periods values

5.2.6.1 Type Mapping

Signature : periods -> stream(periods)

*/

ListExpr getIntervalsTM(ListExpr args){
  string err = "periods expected";
  if(!nl->HasLength(args,1)){
     return listutils::typeError(err);
  } 
  if(!Range<DateTime>::checkType(nl->First(args))){
     return listutils::typeError(err);
  }
  return nl->TwoElemList(
                  nl->SymbolAtom(Stream<Attribute>::BasicType()),
                  nl->SymbolAtom(Range<DateTime>::BasicType()));
}

/*
5.2.6.2 Value Mapping

The template parameter controls whether the resulting stream returns 
intervals (single = true) or intervals wrapped into a periods 
value (single = false).


*/
template<bool single>
int getIntervalsVM(Word* args, Word& result, int message,
                   Word& local, Supplier s) {
  size_t* li = (size_t*) local.addr;
  Range<DateTime>* p = (Range<DateTime>*)args[0].addr;
  size_t v;
  switch(message) {
    case OPEN: 
      if (li) {
        delete li;
        local.addr = 0;
      }
      if (p->IsDefined()) {
        local.addr = new size_t(0);
      }
      return 0;
    case REQUEST: 
      if (!li) {
        return CANCEL;
      }
      v = *li;
      if (v >= (size_t)p->GetNoComponents()) {
        return CANCEL;
      }
      else {
        Interval<DateTime> iv;
        p->Get(*li, iv);
        (*li)++;
        if (single) {
          result.addr = new SecInterval(iv);
        }
        else {
          Range<DateTime>* r = new Range<DateTime>(1);
          r->Add(iv);
          result.addr = r;
        }
        return YIELD;
      }
    case CLOSE:
      if (li) {
        delete li;
        local.addr = 0;
      }
      return 0;
  }
  return -1;
}


/*
5.2.6.3 Specification

*/
const string getIntervalsSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> periods -> stream(periods)"
    "</text---> "
    "<text> getIntervals(_) </text--->"
    "<text>Puts the intervals of a periods value into a stream "
    "</text--->"
    "<text>query getIntervals(deftime(train7)) count"
    " </text--->"
    ") )";

/*
5.2.6.4 Operator instance

*/
Operator getIntervals(
           "getIntervals",
           getIntervalsSpec,
           getIntervalsVM<false>,
           Operator::SimpleSelect,
           getIntervalsTM);

/*
5.2.7 Operator ~components~

----
    periods -> stream(interval)
----

*/

/*
5.2.7.1 Type Mapping for operator ~components~

*/

ListExpr componentsTM(ListExpr args) {
  string err = "one argument of type periods expected";
  if (!nl->HasLength(args, 1)){
    return listutils::typeError(err);
  }
  if (!Periods::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return nl->TwoElemList( listutils::basicSymbol<Stream<SecInterval> >(),
                          listutils::basicSymbol<SecInterval> () );
}

/*
5.2.7.2 Value Mapping for operator ~components~

This operator uses the same Value Mapping as the operator ~getIntervals~

*/

/*
5.2.7.3 Specification

*/
const string componentsSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> periods -> stream(interval)"
    "</text---> "
    "<text> components(_) </text--->"
    "<text>Puts the intervals of a periods value into a stream "
    "</text--->"
    "<text>query components(deftime(train7)) count"
    " </text--->"
    ") )";

/*
5.2.6.4 Operator instance

*/
Operator components(
           "components",
           componentsSpec,
           getIntervalsVM<true>,
           Operator::SimpleSelect,
           componentsTM);
  
/*
6 Creating the Algebra

*/
class TemporalAlgebra : public Algebra
{
 public:
  TemporalAlgebra() : Algebra()
  {
    AddTypeConstructor( &rangeint );
    AddTypeConstructor( &rangereal );
    AddTypeConstructor( &periods );
    AddTypeConstructor( &intimebool );
    AddTypeConstructor( &intimeint );
    AddTypeConstructor( &intimereal );
    AddTypeConstructor( &intimepoint );

    AddTypeConstructor( &unitbool );
    AddTypeConstructor( &unitint );
    AddTypeConstructor( &unitreal );
    AddTypeConstructor( &unitpoint );

    AddTypeConstructor( &movingbool );
    AddTypeConstructor( &movingint );
    AddTypeConstructor( &movingreal );
    AddTypeConstructor( &movingpoint );

    AddTypeConstructor( &cellgrid2d);

    rangeint.AssociateKind( Kind::RANGE() );
    rangeint.AssociateKind( Kind::DATA() );
    rangereal.AssociateKind( Kind::RANGE() );
    rangereal.AssociateKind( Kind::DATA() );
    periods.AssociateKind( Kind::RANGE() );
    periods.AssociateKind( Kind::DATA() );

    intimebool.AssociateKind( Kind::TEMPORAL() );
    intimebool.AssociateKind( Kind::DATA() );
    intimeint.AssociateKind( Kind::TEMPORAL() );
    intimeint.AssociateKind( Kind::DATA() );
    intimereal.AssociateKind( Kind::TEMPORAL() );
    intimereal.AssociateKind( Kind::DATA() );
    intimepoint.AssociateKind( Kind::TEMPORAL() );
    intimepoint.AssociateKind( Kind::DATA() );

    unitbool.AssociateKind( Kind::TEMPORAL() );
    unitbool.AssociateKind( Kind::DATA() );
    unitint.AssociateKind( Kind::TEMPORAL() );
    unitint.AssociateKind( Kind::DATA() );
    unitreal.AssociateKind( Kind::TEMPORAL() );
    unitreal.AssociateKind( Kind::DATA() );
    unitpoint.AssociateKind( Kind::TEMPORAL() );
    unitpoint.AssociateKind( Kind::DATA() );
    unitpoint.AssociateKind( "SPATIAL3D" );

    movingbool.AssociateKind( Kind::TEMPORAL() );
    movingbool.AssociateKind( Kind::DATA() );
    movingint.AssociateKind( Kind::TEMPORAL() );
    movingint.AssociateKind( Kind::DATA() );
    movingreal.AssociateKind( Kind::TEMPORAL() );
    movingreal.AssociateKind( Kind::DATA() );
    movingpoint.AssociateKind( Kind::TEMPORAL() );
    movingpoint.AssociateKind( Kind::DATA() );

    cellgrid2d.AssociateKind( Kind::DATA() );

    AddOperator( &temporalisempty );
    AddOperator( &temporalequal );
    AddOperator( &temporalnotequal );
    AddOperator( &temporalequal2 );
    AddOperator( &temporalnotequal2 );
    AddOperator( &temporalless );
    AddOperator( &temporallessequal );
    AddOperator( &temporalgreater );
    AddOperator( &temporalgreaterequal );
    AddOperator( &temporalintersects );
    AddOperator( &temporalinside );
    AddOperator( &temporalbefore );
    AddOperator( &temporalintersection );
    AddOperator( &temporalunion );
    AddOperator( &temporalminus );
    AddOperator( &temporalmin );
    AddOperator( &temporalmax );
    AddOperator( &temporalnocomponents );

    AddOperator( &temporalinst );
    AddOperator( &temporalval );
    AddOperator( &temporalatinstant );
    AddOperator( &temporalatperiods );
    AddOperator( &temporalwhen );
    AddOperator( &temporaldeftime );
    AddOperator( &temporaltrajectory );
    AddOperator( &temporalpresent );
    AddOperator( &temporalpasses );
    AddOperator( &temporalinitial );
    AddOperator( &temporalfinal );
    AddOperator( &temporalunits );
    AddOperator( &temporalgetunit );
    AddOperator( &temporalbbox );
    AddOperator( &temporalmbrange );
    AddOperator( &temporalbbox2d );
    AddOperator( &temporalbboxold);

    AddOperator( &temporalat );
    AddOperator( &temporaldistance );
    AddOperator( &temporalsimplify );
    AddOperator( &temporalintegrate );
    AddOperator( &temporallinearize );
    AddOperator( &temporallinearize2 );
    AddOperator( &temporalapproximate );
    AddOperator( &temporalminimum );
    AddOperator( &temporalmaximum );
    AddOperator( &temporalbreakpoints );
    AddOperator( &breaks);
    AddOperator( &temporalgk );
    AddOperator( &temporalvertices );
    AddOperator( &temporaltranslate );

    AddOperator( &temporaltheyear );
    AddOperator( &temporalthemonth );
    AddOperator( &temporaltheday );
    AddOperator( &temporalthehour );
    AddOperator( &temporaltheminute );
    AddOperator( &temporalthesecond );
    AddOperator( &temporaltheperiod );
    AddOperator( &temporaltherange );

    AddOperator(&temporalbox3d);
    AddOperator(&temporalbox2d);
    AddOperator(&mbool2mint);
    AddOperator(&mint2mbool);
    AddOperator(&mint2mreal);
    AddOperator(&extdeftime);
    AddOperator(&temporaltranslateappend);
    AddOperator(&temporaltranslateappendS);
    AddOperator(&temporalreverse);
    AddOperator(&temporalsamplempoint);
    AddOperator(&temporalgps);
    AddOperator(&temporaldisturb);
    AddOperator(&temporallength);
    AddOperator(&equalizeU);
    AddOperator(&hat);
    AddOperator(&restrict);
    AddOperator(&speedup);
    AddOperator(&avg_speed);
    AddOperator(&submove);
    AddOperator(&temporaluval);
    AddOperator(&mp2onemp);
    AddOperator(&p2mp);
    AddOperator(&delayoperator);
    AddOperator(&distancetraversedoperator);
    AddOperator(&turns);
    AddOperator(&mappingtimeshift);
    AddOperator(&gridcellevents);
    AddOperator(&temporalsquareddistance);
    AddOperator(&getrefinementpartition);

    AddOperator(&createCellGrid2D);
    
    AddOperator(&atRect);
    AddOperator(&moveTo);
    AddOperator(&fillGaps);
    AddOperator(&removeShort);
    
    AddOperator(&getIntervals);
    AddOperator(&components);


#ifdef USE_PROGRESS
    temporalunits.EnableProgress();
#endif

  }
  ~TemporalAlgebra() {};
};

/*
7 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeTemporalAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new TemporalAlgebra());
}


