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

01.06.2006 Christian D[ue]ntgen added operator ~bbox~ for ~range~-types.

Sept 2006 Christian D[ue]ntgen implemented ~defined~ flag for unit types

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
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "PolySolver.h"

extern NestedList* nl;
extern QueryProcessor* qp;

#include "DateTime.h"
#include "TemporalAlgebra.h"

string int2string(const int& number)
{
  ostringstream oss;
  oss << number;
  return oss.str();
}

/*
1.1 Definition of some constants

*/
const bool TA_DEBUG = false;  // debugging off
// const bool TA_DEBUG = true;  // debugging on

const double MAXDOUBLE = numeric_limits<double>::max();
const double MINDOUBLE = numeric_limits<double>::min();

/*
3 Implementation of C++ Classes

3.1 Class ~UReal~

*/
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
//      double t2 = t.ToDouble() - timeInterval.start.ToDouble();
      double res = a * pow( t2, 2 ) +
        b * t2 +
        c;
      if( r ) res = sqrt( res );
      result.Set( true, res );
    }


/*
2006-Sep-22, CD:

The following implemention is the original implementation following the
papers published, but suffers from rounding errors.

----

    {
      double res = a * pow( t.ToDouble(), 2 ) +
                   b *      t.ToDouble()      +
                   c;
      if( r ) res = sqrt( res );
      result.Set( true, res );
    }

----

*/
}

bool UReal::Passes( const CcReal& val ) const
  // VTA - Not implemented yet
{
  return false;
}

bool UReal::At( const CcReal& val, TemporalUnit<CcReal>& result ) const
  // VTA - Not implemented yet
  // CD - Implementation causes problem, as the result could be a set of
  // 0-2 Units!
{
  return false;
}

void UReal::AtInterval( const Interval<Instant>& i,
 TemporalUnit<CcReal>& result ) const
{
  TemporalUnit<CcReal>::AtInterval( i, result );

  UReal *pResult = (UReal*)&result;
  pResult->a = a;
  pResult->b = b;
  pResult->c = c;
  pResult->r = r;
  pResult->StandardTemporalUnit<CcReal>::SetDefined(true);

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
}

double AntiderivativeSQRTpoly1(const double a, const double b,
                               const double x)
{ // Bronstein, Taschenbuch der Mathematik 21.5.2.3 (121)
  assert( a != 0 );
  double X = a*x+b;
  return (2/(3*a))*sqrt(X*X*X);
}


long double arcsinh(long double XVal)
{
  return log(XVal + sqrt(XVal * XVal + 1.0));
}

double Antiderivative1overSQRTpoly2(const double a, const  double b,
                                    const double c, const double x)
{ // Bronstein, Taschenbuch der Mathematik 21.5.2.8 (241)
  assert(a != 0);

  double Delta = 4*a*x - b*b;

  if((a>0) && (Delta > 0)){
      return (1/sqrt(a)) * arcsinh( (2*a*x +b)/sqrt(Delta) );
  }

  if((a>0.0) && (Delta==0.0)){
      return (1.0/sqrt(a))*log(2*a*x+b);
  }

  if(a<0 && Delta<0){
      return -1.0*((1/(sqrt(-1.0*a))) * asin((2*a*x+b)/(sqrt(-1.0*Delta))));
  }

  if(a>0){ // case Delta < 0
     double f1 = 1/ sqrt(a);
     double sum = 2*sqrt(a*(a*x*x*x+b*x+c)) + 2*a*x + b;
     double f2 = log(sum);
     double res = f1*f2;
     return res;
  }

  cerr <<  "invalid case reached" << endl;

  return 0;

}

double AntiderivativeSQRTpoly2(const double a, const double b,
                               const double c, const double x)
{ // Bronstein, Taschenbuch der Mathematik 21.5.2.8 (245)
  assert(a != 0);

  double f = ( 4*a*c-b*b )  / 8*a;   // == 1/2k
  double v = a*x*x+b*x+c;
  if(v<0){  // correct values resulting from rounding errors
     if(!AlmostEqual(v,0)) {
        cerr << "Invalid computation in UREal::Integrate" << endl;
     }
     v = 0;
  }
  double s1 = ((2*a*x +b) * sqrt(v)) / (4*a);
  double s2 = f *Antiderivative1overSQRTpoly2(a, b, c, x);
  return  s1 + s2;
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
    return   AntiderivativeSQRTpoly1(b, c, t);
  }
  // form : sqrt ( ax^2 + bx + c)
  return    AntiderivativeSQRTpoly2(a, b, c, t);
}


/*
This function computes the maximum of this UReal.

*/
double UReal::Max(bool& correct) const{
  if(!IsDefined()){
    correct=false;
    return 0.0;
  }

  //double t0 = timeInterval.start.ToDouble();
  //double t1 = timeInterval.end.ToDouble();
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
  if(isnan(v1) || isnan(v2) || isnan(v3)){
      cerr << " cannot determine the value within a unit" << endl;
  }

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

  // double t0 = timeInterval.start.ToDouble();
  // double t1 = timeInterval.end.ToDouble();
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
  if(isnan(v1) || isnan(v2) || isnan(v3)){
      cerr << "UReal::Min(): cannot determine the value within a unit" << endl;
  }
  // determine the minimum of v1 .. v3
  double min = v1;
  if(v2<min){
     min = v2;
  }
  if(v3 < min){
     min = v3;
  }
  if(r){
    return sqrt(min);
  } else {
    return min;
  }
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

/*  cout << "UReal::PeriodsAtVal( " << value << ", ...) called." << endl;
  cout << "\ta=" << a << " b=" << b << " c=" << c << " r=" << r << endl;
  cout << "\tstart=" << timeInterval.start.ToDouble()
       << " end=" << timeInterval.end.ToDouble()
       << " lc=" << timeInterval.lc
       << " rc=" << timeInterval.rc << endl;*/
  times.Clear();
  if( !IsDefined() )
  {
//      cout << "UReal::PeriodsAtVal(): Undefined UReal -> 0 results." << endl;
     return 0;
  }

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
  for(int i=0; i<no_res; i++)
  {
    t0.ReadFrom(inst_d[i]);
    t1 = timeInterval.start + t0;
    if( (t1 == timeInterval.start) ||
        (t1 == timeInterval.end)   ||
        timeInterval.Contains(t1)     )
    {
/*      cout << "\tt1=" << t1.ToDouble() << endl;
      cout << "\tt1.IsDefined()=" << t1.IsDefined() << endl;
      cout << "\ttimes.IsValid()=" << times.IsValid() << endl;*/
      if( !times.Contains( t1 ) )
      {
//         cout << "UReal::PeriodsAtVal(): add instant" << endl;
        iv = Interval<Instant>(t1, t1, true, true);
        times.StartBulkLoad();
        times.Add(iv); // add only once
        times.EndBulkLoad();
      }
/*      else
        cout << "UReal::PeriodsAtVal(): not added instant" << endl;*/
    }
  }
/*  cout << "UReal::PeriodsAtVal(): Calculated "
       << times.GetNoComponents() << "results." << endl;*/
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
     return numeric_limits<double>::infinity();
  }

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
     return -numeric_limits<double>::infinity();
  }

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

Precondition: this[->]IsDefined()

Result: stores the resultununit into vector result and returns
        the number of results (1-2) found.

WARNING: AtMin may return points, that are not inside this->timeInterval,
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
    const Interval<DateTime>* iv;
    minTimesPeriods.Get(i, iv);
    UReal unit = UReal( *this );
    correct = false;
    if( iv->Inside(timeInterval) )
    {
      AtInterval( *iv, unit );
      correct = true;
    }
    else
    { // solve problem with min at open interval start/end!
      if( (iv->start == timeInterval.start) || (iv->end == timeInterval.end) )
      {
        UReal unit2(
          Interval<DateTime>(timeInterval.start, timeInterval.end, true, true),
          a, b, c, r );
        unit2.AtInterval( *iv, unit );
        correct = true;
      }
    }
    if( correct )
      result.push_back(unit);
    else
      cout << "UReal::AtMin(): This should not happen!" << endl;
  }
  return result.size();
}


/*
Creates a vector of units, which are the restriction of this to
the periods, where it takes its maximum value.

Precondition: this[->]IsDefined()

Result: stores the resultununit into vector result and returns
        the number of results (1-2) found.

WARNING: AtMax may return points, that are not inside this->timeInterval,
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
    const Interval<DateTime>* iv;
    maxTimesPeriods.Get(i, iv);
//     cout << "iv = (" << iv->start.ToString() << " " << iv->end.ToString()
//          << " " << iv->lc << " " << iv->rc << ")" << endl;*/
    UReal unit = UReal( *this );
    correct = false;
    if( iv->Inside(timeInterval) )
    {
       AtInterval( *iv, unit );
       correct = true;
    }
    else if( (iv->start==timeInterval.start) || (iv->end==timeInterval.end) )
    { // solve problem with max at open interval start/end!
      UReal unit2(
        Interval<DateTime>(timeInterval.start, timeInterval.end, true, true),
        a, b, c, r );
      unit2.AtInterval( *iv, unit );
      correct = true;
    }
    if( correct )
      result.push_back(unit);
    else
      cout << "UReal::AtMax(): This should not happen!" << endl;
  }
  return result.size();
}

/*
Creates a vector of units, which are the restriction of this to
the periods, where it takes a certain value.

  Precondition: this[->]IsDefined() && value.IsDefined()
Result: stores the resultununit into vector result and returns
        the number of results (1-2) found.

WARNING: AtMax may return points, that are not inside this->timeInterval,
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
    const Interval<DateTime>* iv;
    valTimesPeriods.Get(i, iv);
    UReal unit = UReal( *this );
    if( iv->Inside(timeInterval) )
      AtInterval( *iv, unit );
    else
    { // solve problem with max at open interval start/end!
      if( (iv->start == timeInterval.start) || (iv->end == timeInterval.end) )
      {
        UReal unit2(
          Interval<DateTime>(timeInterval.start, timeInterval.end, true, true),
          a, b, c, r );
        unit2.AtInterval( *iv, unit );
      }
    }
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
  Interval<Instant> commonIv;
  UReal u1(true), u2(true), udiff(true);
  times.Clear();
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

  return 0;
}

/*
3.1 Class ~UPoint~

*/
void UPoint::TemporalFunction( const Instant& t,
                               Point& result,
                               bool ignoreLimits ) const
{
  if( !IsDefined() ||
      !t.IsDefined() ||
      (!timeInterval.Contains( t ) && !ignoreLimits) )
    {
      result.SetDefined(false);
    }
  else if( t == timeInterval.start )
    {
      result = p0;
      result.SetDefined(true);
    }
  else if( t == timeInterval.end )
    {
      result = p1;
      result.SetDefined(true);
    }
  else
    {
      Instant t0 = timeInterval.start;
      Instant t1 = timeInterval.end;

      double x = (p1.GetX() - p0.GetX()) * ((t - t0) / (t1 - t0)) + p0.GetX();
      double y = (p1.GetY() - p0.GetY()) * ((t - t0) / (t1 - t0)) + p0.GetY();

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

  if( timeInterval.lc && AlmostEqual( p, p0 ) ||
      timeInterval.rc && AlmostEqual( p, p1 ) )
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

bool UPoint::Passes( const Region& r ) const
{
  if( AlmostEqual( p0, p1 ) )
    return r.Contains( p0 );

  HalfSegment hs( true, p0, p1 );
  return r.Intersects( hs );
}

bool UPoint::At( const Point& p, TemporalUnit<Point>& result ) const
{
/*
VTA - In the same way as ~Passes~, I could use the Spatial Algebra here.

*/
  assert( IsDefined() );
  assert( p.IsDefined() );

  UPoint *pResult = (UPoint*)&result;

  if( AlmostEqual( p0, p1 ) )
  {
    if( AlmostEqual( p, p0 ) )
    {
      *pResult = *this;
      return true;
    }
  }
  else if( AlmostEqual( p, p0 ) )
  {
    if( timeInterval.lc )
    {
      Interval<Instant> interval( timeInterval.start,
       timeInterval.start, true, true );
      UPoint unit( interval, p, p );
      *pResult = unit;
      return true;
    }
  }
  else if( AlmostEqual( p, p1 ) )
  {
    if( timeInterval.rc )
    {
      Interval<Instant> interval( timeInterval.end,
       timeInterval.end, true, true );
      UPoint unit( interval, p, p );
      *pResult = unit;
      return true;
    }
  }
  else if( AlmostEqual( p0.GetX(), p1.GetX() ) &&
           AlmostEqual( p0.GetX(), p.GetX() ) )
    // If the segment is vertical
  {
    if( ( p0.GetY() <= p.GetY() && p1.GetY() >= p.GetY() ) ||
        ( p0.GetY() >= p.GetY() && p1.GetY() <= p.GetY() ) )
    {
      Instant t( timeInterval.start +
                   ( ( timeInterval.end - timeInterval.start ) *
                   ( ( p.GetY() - p0.GetY() ) / ( p1.GetY() - p0.GetY() ) ) ) );
      Interval<Instant> interval( t, t, true, true );
      UPoint unit( interval, p, p );
      *pResult = unit;
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
      Instant t( timeInterval.start +
                   ( ( timeInterval.end - timeInterval.start ) *
                   ( ( p.GetX() - p0.GetX() ) / ( p1.GetX() - p0.GetX() ) ) ) );
      Interval<Instant> interval( t, t, true, true );
      UPoint unit( interval, p, p );
      *pResult = unit;
      return true;
    }
  }
  else
  {
    double k1 = ( p.GetX() - p0.GetX() ) / ( p.GetY() - p0.GetY() ),
           k2 = ( p1.GetX() - p0.GetX() ) / ( p1.GetY() - p0.GetY() );

    if( AlmostEqual( k1, k2 ) &&
        ( ( p0.GetX() <= p.GetX() && p1.GetX() >= p.GetX() ) ||
          ( p0.GetX() >= p.GetX() && p1.GetX() <= p.GetX() ) ) )
    {
      Instant t( timeInterval.start +
                   ( ( timeInterval.end - timeInterval.start ) *
                   ( ( p.GetX() - p0.GetX() ) / ( p1.GetX() - p0.GetX() ) ) ) );
      Interval<Instant> interval( t, t, true, true );
      UPoint unit( interval, p, p );
      *pResult = unit;
      return true;
    }
  }
  return false;
}

void UPoint::AtInterval( const Interval<Instant>& i,
 TemporalUnit<Point>& result ) const
{
  TemporalUnit<Point>::AtInterval( i, result );

  UPoint *pResult = (UPoint*)&result;

  assert( IsDefined() );
  assert( i.IsValid() );
  if( timeInterval.start == result.timeInterval.start )
    {
      pResult->p0 = p0;
      pResult->timeInterval.start = timeInterval.start;
      pResult->timeInterval.lc = pResult->timeInterval.lc && timeInterval.lc;
    }
  else
    TemporalFunction( result.timeInterval.start, pResult->p0 );

  if( timeInterval.end == result.timeInterval.end )
    {
      pResult->p1 = p1;
      pResult->timeInterval.end = timeInterval.end;
      pResult->timeInterval.rc = pResult->timeInterval.rc && timeInterval.rc;
    }
  else
    TemporalFunction( result.timeInterval.end, pResult->p1 );
  pResult->SetDefined ( true );
}

void UPoint::Distance( const Point& p, UReal& result ) const
{
  if( !IsDefined() || ! p.IsDefined() )
    {
      result.SetDefined(false);
    }
  else
    {
      result.timeInterval = timeInterval;

      DateTime DT = timeInterval.end - timeInterval.start;
      double dt = DT.ToDouble();
      double
        //t0 = timeInterval.start.ToDouble(),
        x0 = p0.GetX(), y0 = p0.GetY(),
        x1 = p1.GetX(), y1 = p1.GetY(),
        x  =  p.GetX(), y  =  p.GetY();

      if ( AlmostEqual(dt, 0.0) )
        { // single point unit
          result.a = 0.0;
          result.b = 0.0;
          result.c = pow(x0-x,2) + pow(y0-y,2);
          result.r = true;
        }
      else
        {
          result.a = pow((x1-x0)/dt,2)+pow((y1-y0)/dt,2);
          result.b = 2*((x1-x0)*(x0-x)+(y1-y0)*(y0-y))/dt;
          result.c = pow(x0-x,2)+pow(y0-y,2);
          result.r = true;

/*

For the original representation of ureal, we need:

----
          double A = pow((x1-x0)/dt,2)+pow((y1-y0)/dt,2);
          double B = 2*((x1-x0)*(x0-x)+(y1-y0)*(y0-y))/dt;
          double C = pow(x0-x,2)+pow(y0-y,2);

          result.a = A;
          result.b = B-2*A*t0;
          result.c = t0*(t0*A-B)+C;
          result.r = true;

----

*/

        }
      result.SetDefined(true);
    }
  return;
}

void UPoint::USpeed( UReal& result ) const
{

  double x0, y0, x1, y1;
  double duration;

  if ( !IsDefined() )
    result.SetDefined( false );
  else
    {
      x0 = p0.GetX();
      y0 = p0.GetY();

      x1 = p1.GetX();
      y1 = p1.GetY();

      result.timeInterval = timeInterval;

      DateTime dt = timeInterval.end - timeInterval.start;
      duration = dt.ToDouble() * 86400;   // value in seconds

      if( duration > 0.0 )
        {
          /*
            The point unit can be represented as a function of
            f(t) = (x0 + x1 * t, y0 + y1 * t).
            The result of the derivation is the constant (x1,y1).
            The speed is constant in each time interval.
            Its value is represented by variable c. The variables a and b
            are set to zero.

          */
          result.a = 0;  // speed is constant in the interval
          result.b = 0;
          result.c = sqrt(pow( (x1-x0), 2 ) + pow( (y1- y0), 2 ))/duration;
          result.r = false;
          result.SetDefined( true );
        }
      else
        result.SetDefined( false );
    }
}

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
      duration = dt.ToDouble() * 86400;   // value in seconds

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
  HalfSegment hs;
  int edgeno = 0;

  line.Clear();
  line.StartBulkLoad();
  if( !AlmostEqual( p0, p1 ) )
        {
          hs.Set( true, p0, p1 );
          hs.attr.edgeno = ++edgeno;
          line += hs;
          hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
          line += hs;
        }
  line.EndBulkLoad();
}

void UPoint::Intersection(const UPoint &other, UPoint &result) const
{
      Interval<Instant> iv;
      Instant t;
      Point p_intersect, d1, d2, p1;
      UPoint p1norm(true), p2norm(true);
      double t_x, t_y, t1, t2, dxp1, dxp2, dyp1, dyp2, dt;
      bool intersectionfound = false;

      if ( !IsDefined() ||
           !other.IsDefined() ||
           !timeInterval.Intersects( other.timeInterval ) )
        {
          result.SetDefined( false );
          if (TA_DEBUG)
            cerr << "No intersection (0): deftimes do not overlap" << endl;
          return; // nothing to do
        }
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
          return;
        }
      // else: no result
      if (TA_DEBUG) cerr << "No intersection (2)." << endl;
      result.SetDefined( false );
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
  const UBool* ubool;
  StartBulkLoad();
  CcInt currentValue;
  for(int i=0;i<size;i++){
    arg.Get(i,ubool);
    bool v;
    v = ubool->constValue.GetBoolval();
    currentValue.Set(true,v?1:0);
    UInt unit(ubool->timeInterval,currentValue);
    Add(unit);
  }
  EndBulkLoad(false);
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
   const UReal* unit;
   stack<ISC> theStack;


   for(int i=0;i < size;i++){
      Get(i,unit);
      ISC isc;
      isc.value = unit->Integrate();
      if(isnan(isc.value)) cout << " value = " << isc.value << endl;
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
   const UReal* unit;
   Get(0,unit);
   bool dummy;
   double max = unit->Max(dummy);
   for(int i=1;i<size;i++){
       Get(i,unit);
       double umax = unit->Max(dummy);
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
   const UReal* unit;
   Get(0,unit);
   bool dummy;
   double min = unit->Min(dummy);
   for(int i=1;i<size;i++){
       Get(i,unit);
       double umin = unit->Min(dummy);
       if(umin<min){
          min = umin;
       }
   }
   return min;
}

// restrict to periods with maximum value
void MReal::AtMin( MReal& result ) const
{
  double globalMin = numeric_limits<double>::infinity();
  double localMin  = 0.0;
  int noLocalMin = 0;
  bool correct = true;
  const UReal *actual_ur = 0;
  const UReal *last_ur = 0;
  UReal last_candidate(true);
  bool firstCall = true;
  result.Clear();
  result.StartBulkLoad();
  for(int i=0; i<GetNoComponents(); i++)
  {
//     cerr << "MReal::AtMin(): Processing unit "
//          << i << "..." << endl;
    last_ur = actual_ur;
    Get( i, actual_ur );
    localMin = actual_ur->Min(correct);
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
      last_ur = 0;
//       cerr << "MReal::AtMin(): New globalMin=" << globalMin << endl;
    }
    if( localMin <= globalMin )
    { // this ureal contains global minima
      vector<UReal> localMinimaVec;
      noLocalMin = actual_ur->AtMin( localMinimaVec );
//       cerr << "MReal::AtMin(): Unit " << i << " has "
//            << noLocalMin << " minima" << endl;
      for(int j=0; j< noLocalMin; j++)
      {
        UReal candidate = localMinimaVec[j];
        // test, whether candidate overlaps last_inserted one
        if( j==0 &&               // check only unit's first local min!
            !firstCall &&         // don't check if there is no last_candidate
            last_candidate.Intersects(candidate) &&
            ( !last_ur->Intersects(last_candidate) ||
              !actual_ur->Intersects(candidate)
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
  double globalMax = -numeric_limits<double>::infinity();
  double localMax  = 0.0;
  int noLocalMax = 0;
  bool correct = true;
  const UReal *actual_ur = 0;
  const UReal *last_ur = 0;
  UReal last_candidate(true);
  bool firstCall = true;
  result.Clear();
  result.StartBulkLoad();
  for(int i=0; i<GetNoComponents(); i++)
  {
//     cerr << "MReal::AtMax(): Processing unit "
//          << i << "..." << endl;
    last_ur = actual_ur;
    Get( i, actual_ur );
    localMax = actual_ur->Max(correct);
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
      last_ur = 0;
//       cerr << "MReal::AtMax(): New globalMax=" << globalMax << endl;
    }
    if( localMax >= globalMax )
    { // this ureal contains global maxima
      vector<UReal> localMaximaVec;
      noLocalMax = actual_ur->AtMax( localMaximaVec );
//       cerr << "MReal::AtMax(): Unit " << i << " has "
//            << noLocalMax << " maxima" << endl;
      for(int j=0; j< noLocalMax; j++)
      {
        UReal candidate = localMaximaVec[j];
        // test, whether candidate overlaps last_inserted one
        if( j==0 &&               // check only unit's first local max!
            !firstCall &&         // don't check if there is no last_candidate
            last_candidate.Intersects(candidate) &&
            ( !last_ur->Intersects(last_candidate) ||
              !actual_ur->Intersects(candidate)
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
void MReal::AtValue( CcReal& ccvalue, MReal& result ) const
{
  assert( ccvalue.IsDefined() );

  int noLocalResults = 0;
  const UReal *actual_ur = 0;
  const UReal *last_ur = 0;
  UReal last_candidate(true);
  bool firstCall = true;
  result.Clear();
  result.StartBulkLoad();
  for(int i=0; i<GetNoComponents(); i++)
  {
//     cerr << "MReal::AtValue(): Processing unit "
//          << i << "..." << endl;
    last_ur = actual_ur;
    Get( i, actual_ur );
    vector<UReal> localResultVec;
    noLocalResults = actual_ur->AtValue( ccvalue, localResultVec );
//       cerr << "MReal::AtValue(): Unit " << i << " has "
//            << noLocalResults << " results" << endl;
    for(int j=0; j< noLocalResults; j++)
    {
      UReal candidate = localResultVec[j];
      // test, whether candidate overlaps last_inserted one
      if( j==0 &&               // check only unit's first local max!
          !firstCall &&         // don't check if there is no last_candidate
          last_candidate.Intersects(candidate) &&
          ( !last_ur->Intersects(last_candidate) ||
            !actual_ur->Intersects(candidate)
          )
        )
      {
//           cerr << "MReal::AtValue(): unit overlaps last one." << endl;
        if( last_candidate.timeInterval.start
            == last_candidate.timeInterval.end )
        { // case 1: drop last_candidate (which is an instant-unit)
//             cerr << "MReal::AtValue(): drop last unit." << endl;
          last_candidate = candidate;
          continue;
        }
        else if( candidate.timeInterval.start
                  == candidate.timeInterval.end )
        { // case 2: drop candidate
//             cerr << "MReal::AtValue(): drop actual unit." << endl;
          continue;
        }
        else
          cerr << "MReal::AtValue(): This should not happen!" << endl;
      }
      else
      { // All is fine. Just insert last_candidate.
//      cerr << "MReal::AtValue(): unit does not overlap with last." << endl;
        if(firstCall)
        {
//        cerr << "MReal::AtValue(): Skipping insertion of last unit." << endl;
          firstCall = false;
        }
        else
        {
//             cerr << "MReal::AtValue(): Added last unit" << endl;
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
  if(!firstCall)
  {
    result.MergeAdd(last_candidate);
//     cerr << "MReal::AtValue(): Added final unit" << endl;
  }
//   else
//     cerr << "MReal::AtValue(): Skipping insertion of final unit." << endl;
  result.EndBulkLoad();
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


void MReal::Simplify(const double epsilon, MReal& result) const{
  result.Clear();
  if(!IsDefined()){
     result.SetDefined(false);
     return;
  }
  unsigned int size = GetNoComponents();
  result.SetDefined(true);
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
  const UReal* ur1;
  const UReal* ur2;
  while(last < size){
    Get(last-1,ur1);
    Get(last, ur2);
    if(keep(ur1)){
         if(last-1 > first){
            Simplify(first,last-2,useleft,useright,epsilon);
         }
         Simplify(last-1, last-1, useleft, useright, epsilon);
         first = last;
         last++;
    } else if( keep(ur2)){
         Simplify(first,last-1,useleft,useright,epsilon);
         last++;
         Simplify(last-1, last-1,useleft,useright,epsilon);
         first = last;
         last++;
    } else if(connected(ur1,ur2)){ // enlarge the sequence
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
   const UReal* unit;
   Instant start(instanttype);
   Instant end(instanttype);
   bool lc,rc;
   CcReal startValue(true,0);
   CcReal endValue(true,0);
   bool leftDefined = false;
   result.StartBulkLoad();
   for(unsigned int i=0;i<size;i++){
      Get(i,unit);
      if(useleft[i] && useright[i]){ // copy this unit
         result.Add(*unit);
      } else {
         if(useleft[i]){
             if(leftDefined){ // debug
                cout << "Overwrite left part of a ureal " << endl;
             }
             start = unit->timeInterval.start;
             lc = unit->timeInterval.lc;
             unit->TemporalFunction(start,startValue,true);
             leftDefined = true;
         }
         if(useright[i]){
             if(!leftDefined){ // debug
                 cout << "Close ural without left definition " << endl;
             } else{
               end = unit->timeInterval.end;
               rc = unit->timeInterval.rc;
               unit->TemporalFunction(end,endValue,true);
               UReal newUnit(Interval<Instant>(start,end,lc,rc),
                             startValue.GetRealval(),
                             endValue.GetRealval());
               result.Add(newUnit);
               leftDefined=false;
             }

         }
      }
   }
   result.EndBulkLoad();
}

// not implemented yet
void MReal::Simplify(const int min, const int max,
                     bool* useleft, bool* useright,
                     const double epsilon) const{

  // the endpoints are used in each case
  useleft[min] = true;
  useright[max] = true;

  if(min==max){ // no intermediate sampling points -> nothing to simplify
     return;
  }

  const UReal* u1;
  const UReal* u2;
  // build a UPoint from the endpoints
  Get(min,u1);
  Get(max,u2);
  CcReal cr1(true,0.0);
  CcReal cr2(true,0.0);

  u1->TemporalFunction(u1->timeInterval.start,cr1,true);
  u2->TemporalFunction(u2->timeInterval.end,cr2,true);

  double r1 = cr1.GetRealval();
  double r2 = cr2.GetRealval();

  // build the approximating unit


  UReal ureal(Interval<Instant>(u1->timeInterval.start,
                u2->timeInterval.end,true,true),
                r1,
                r2);

  // search for the real with the highest distance to this unit
  double maxDist = 0;
  int maxIndex=0;
  CcReal r_orig(true,0);
  CcReal r_simple(true,0);
  const UReal* u;
  double distance;
  for(int i=min+1;i<=max;i++){
     Get(i,u);
     ureal.TemporalFunction(u->timeInterval.start,r_simple, true);
     u->TemporalFunction(u->timeInterval.start,r_orig,true);
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

*/
void MPoint::Trajectory( Line& line ) const
{
  line.Clear();
  line.StartBulkLoad();

  HalfSegment hs;
  const UPoint *unit;
  int edgeno = 0;

  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );

    if( !AlmostEqual( unit->p0, unit->p1 ) )
    {
      hs.Set( true, unit->p0, unit->p1 );
      hs.attr.edgeno = ++edgeno;
      line += hs;
      hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
      line += hs;
    }
  }

  line.EndBulkLoad();
}

void MPoint::Distance( const Point& p, MReal& result ) const
{
  const UPoint *uPoint;
  UReal uReal(true);

  result.Clear();
  result.StartBulkLoad();
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, uPoint );
    uPoint->Distance( p, uReal );
    if ( uReal.IsDefined() )
      result.Add( uReal );
  }
  result.EndBulkLoad( false );
}


/*
This function checks whether the end point of the first unit is equals
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

This function removed some sampling points from a moving point
to get simpler data. It's implemented using an algorithm based
on the Douglas Peucker algorithm for line simplification.

**/

void MPoint::Simplify(const double epsilon, MPoint& result,
                      const bool checkBreakPoints,
                      const DateTime& dur) const{
   result.Clear();

   // check for defined state
   if(!IsDefined()){
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
   const UPoint* u1;
   const UPoint* u2;
   while(last<size){
      // check whether last and last -1 are connected
      Get(last-1,u1);
      Get(last,u2);

      if( checkBreakPoints && IsBreakPoint(u1,dur)){
         if(last-1 > first){
            Simplify(first,last-2,useleft,useright,epsilon);
         }
         Simplify(last-1, last-1, useleft, useright, epsilon);
         first = last;
         last++;
      } else if( checkBreakPoints && IsBreakPoint(u2,dur)){
         Simplify(first,last-1,useleft,useright,epsilon);
         last++;
         Simplify(last-1, last-1,useleft,useright,epsilon);
         first = last;
         last++;
      } else if(connected(u1,u2)){ // enlarge the sequence
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
   bool closeLeft;
   bool leftDefined = false;
   for(unsigned int i=0; i< size; i++){
     const UPoint* upoint;

     Get(i,upoint);
     if(useleft[i]){
        // debug
        if(leftDefined){
           cout << " error in mpoint simplification,"
                << " overwrite an existing leftPoint "  << endl;
        }
        // end of debug
        p0 = upoint->p0;
        closeLeft = upoint->timeInterval.lc;
        start = upoint->timeInterval.start;
        leftDefined=true;
     }
     if(useright[i]){
        // debug
        if(!leftDefined){
           cout << " error in mpoint simplification,"
                << " rightdefined before leftdefined "  << endl;

        }
        Interval<Instant> interval(start,upoint->timeInterval.end,closeLeft,
                                   upoint->timeInterval.rc);

        UPoint newUnit(interval,p0,upoint->p1);
        result.Add(newUnit);
        leftDefined=false;
     }
   }
   result.EndBulkLoad(false);
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

  const UPoint* u1;
  const UPoint* u2;
  // build a UPoint from the endpoints
  Get(min,u1);
  Get(max,u2);

  UPoint upoint(Interval<Instant>(u1->timeInterval.start,
                u2->timeInterval.end,true,true),
                u1->p0,
                u2->p1);

  // search for the point with the highest distance to its simplified position
  double maxDist = 0;
  int maxIndex=0;
  Point p_orig;
  Point p_simple;
  const UPoint* u;
  double distance;
  for(int i=min+1;i<=max;i++){
     Get(i,u);
     upoint.TemporalFunction(u->timeInterval.start,p_simple, true);
     distance  = p_simple.Distance(u->p0);
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
    if(!IsDefined()){
      result.SetDefined(false);
      return;
    }
    result.SetDefined(true);
    int size = GetNoComponents();
    result.StartBulkLoad();
    const UPoint* unit;
    for(int i=0;i<size;i++){
        Get(i,unit);
        if(IsBreakPoint(unit,dur)){
           result += (unit->p0);
        }
    }
    result.EndBulkLoad();
}


void MPoint::MVelocity( MPoint& result ) const
{
  const UPoint *uPoint;
  UPoint p(true);
  //  int counter = 0;

  result.Clear();
  result.StartBulkLoad();
  for( int i = 0; i < GetNoComponents(); i++ )
        {
          Get( i, uPoint );
          /*
            Definition of a new point unit p. The velocity is constant
            at all times of the interval of the unit. This is exactly
            the same as within operator ~speed~. The result is a vector
            and can be represented as a upoint.

          */

          uPoint->UVelocity( p );
          if( p.IsDefined() )
            {
              result.Add( p );
              //              counter++;
            }
        }
  result.EndBulkLoad( true );
}

void MPoint::MSpeed( MReal& result ) const
{
  const UPoint *uPoint;
  UReal uReal(true);
  //  int counter = 0;

  result.Clear();
  result.StartBulkLoad();

  for( int i = 0; i < GetNoComponents(); i++ )
        {
          Get( i, uPoint );

          uPoint->USpeed( uReal );
          if( uReal.IsDefined() )
            {
              result.Add( uReal ); // append ureal to mreal
              //              counter++;
            }
        }
  result.EndBulkLoad( true );
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
  return (nl->IsEqual( type, "rint" ));
}

/*
4.1.4 Creation of the type constructor ~rint~

*/
TypeConstructor rangeint(
        "rint",             //name
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
  return (nl->IsEqual( type, "rreal" ));
}

/*
4.2.4 Creation of the type constructor ~rreal~

*/
TypeConstructor rangereal(
        "rreal",              //name
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
  return (nl->IsEqual( type, "periods" ));
}

/*
4.3.4 Creation of the type constructor ~periods~

*/
TypeConstructor periods(
        "periods",         //name
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
  return (nl->IsEqual( type, "ibool" ));
}

/*
4.4.4 Creation of the type constructor ~ibool~

*/
TypeConstructor intimebool(
        "ibool",                   //name
        IntimeBoolProperty,             //property function describing signature
        OutIntime<CcBool, OutCcBool>,
        InIntime<CcBool, InCcBool>,     //Out and In functions
        0,
        0,     //SaveToList and RestoreFromList functions
        CreateIntime<CcBool>,
        DeleteIntime<CcBool>,           //object creation and deletion
        0,
        0,                              // object open and save
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
  return (nl->IsEqual( type, "iint" ));
}

/*
4.4.4 Creation of the type constructor ~iint~

*/
TypeConstructor intimeint(
        "iint",                   //name
        IntimeIntProperty,             //property function describing signature
        OutIntime<CcInt, OutCcInt>,
        InIntime<CcInt, InCcInt>,      //Out and In functions
        0,
        0,    //SaveToList and RestoreFromList functions
        CreateIntime<CcInt>,
        DeleteIntime<CcInt>,           //object creation and deletion
        0,
        0,                        // object open and save
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
  return (nl->IsEqual( type, "ireal" ));
}

/*
4.5.4 Creation of the type constructor ~ireal~

*/
TypeConstructor intimereal(
        "ireal",              //name
        IntimeRealProperty,   //property function describing signature
        OutIntime<CcReal, OutCcReal>,
        InIntime<CcReal, InCcReal>,        //Out and In functions
        0,
        0,    //SaveToList and RestoreFromList functions
        CreateIntime<CcReal>,
        DeleteIntime<CcReal>,              //object creation and deletion
        0,
        0,                                // object open and save
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
  return (nl->IsEqual( type, "ipoint" ));
}

/*
4.6.4 Creation of the type constructor ~ipoint~

*/
TypeConstructor intimepoint(
        "ipoint",                    //name
        IntimePointProperty,  //property function describing signature
        OutIntime<Point, OutPoint>,
        InIntime<Point, InPoint>,         //Out and In functions
        0,
        0,       //SaveToList and RestoreFromList functions
        CreateIntime<Point>,
        DeleteIntime<Point>,              //object creation and deletion
        0,
        0,                           // object open and save
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
  return (nl->IsEqual( type, "ubool" ));
}

/*
4.7.4 Creation of the type constructor ~ubool~

*/
TypeConstructor unitbool(
        "ubool",        //name
        UBoolProperty,    //property function describing signature
        OutConstTemporalUnit<CcBool, OutCcBool>,
        InConstTemporalUnit<CcBool, InCcBool>,    //Out and In functions
        0,     0,  //SaveToList and RestoreFromList functions
        CreateConstTemporalUnit<CcBool>,
        DeleteConstTemporalUnit<CcBool>,          //object creation and deletion
        0,                      0,                // object open and save
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
  return (nl->IsEqual( type, "uint" ));
}

/*
4.7.4 Creation of the type constructor ~uint~

*/
TypeConstructor unitint(
        "uint",     //name
        UIntProperty, //property function describing signature
        OutConstTemporalUnit<CcInt, OutCcInt>,
        InConstTemporalUnit<CcInt, InCcInt>, //Out and In functions
        0,                      0,//SaveToList and RestoreFromList functions
        CreateConstTemporalUnit<CcInt>,
        DeleteConstTemporalUnit<CcInt>, //object creation and deletion
        0,                      0,     // object open and save
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
                             nl->StringAtom("(ureal) "),
         nl->StringAtom("( timeInterval (real1 real2 real3 bool)) "),
         nl->StringAtom("((i1 i2 TRUE FALSE) (1.0 2.2 2.5 TRUE))"))));
}

/*
4.8.3 Kind Checking Function

*/
bool
CheckUReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "ureal" ));
}

/*
4.8.4 ~Out~-function

*/
ListExpr OutUReal( ListExpr typeInfo, Word value )
{
  UReal* ureal = (UReal*)(value.addr);

  if ( !ureal->IsDefined() )
    return (nl->SymbolAtom("undef"));
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
      if( !correct )
      {
        errmsg = "InUReal(): Error in first instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
       nl->Second( first ),
                                           errorPos, errorInfo, correct ).addr;
      if( !correct )
      {
        errmsg = "InUReal(): Error in second instant.";
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
  else if ( nl->IsAtom( instance ) && nl->AtomType( instance ) == SymbolType
            && nl->SymbolValue( instance ) == "undef" )
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
  return (SetWord( new UReal() ));
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
        "ureal",              //name
        URealProperty,  //property function describing signature
        OutUReal,     InUReal, //Out and In functions
        0,            0,   //SaveToList and RestoreFromList functions
        CreateUReal,
        DeleteUReal, //object creation and deletion
        0,            0,   // object open and save
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
                             nl->StringAtom("(upoint) "),
      nl->StringAtom("( timeInterval (real1 real2 real3 real4) ) "),
      nl->StringAtom("((i1 i2 TRUE FALSE) (1.0 2.2 2.5 2.1))"))));
}

/*
4.9.3 Kind Checking Function

*/
bool
CheckUPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "upoint" ));
}

/*
4.9.4 ~Out~-function

*/
ListExpr OutUPoint( ListExpr typeInfo, Word value )
{
  UPoint* upoint = (UPoint*)(value.addr);

  if( !(((UPoint*)value.addr)->IsDefined()) )
    return (nl->SymbolAtom("undef"));
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

The Nested list form is like this:  ( ( 6.37  9.9  TRUE FALSE)   (1.0 2.3 4.1 2.1) )

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
       nl->First( first ),
        errorPos, errorInfo, correct ).addr;

      if( !correct )
      {
        errmsg = "InUPoint(): Error in first instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
       nl->Second( first ),
                                           errorPos, errorInfo, correct ).addr;

      if( !correct )
      {
        errmsg = "InUPoint(): Error in second instant.";
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
  else if ( nl->IsAtom( instance ) && nl->AtomType( instance ) == SymbolType
            && nl->SymbolValue( instance ) == "undef" )
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
  return (SetWord( new UPoint() ));
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
        "upoint",      //name
        UPointProperty,               //property function describing signature
        OutUPoint,     InUPoint, //Out and In functions
        0,             0,  //SaveToList and RestoreFromList functions
        CreateUPoint,
        DeleteUPoint, //object creation and deletion
        0,             0,        // object open and save
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
  return (nl->IsEqual( type, "mbool" ));
}

/*
4.10.4 Creation of the type constructor ~mbool~

*/
TypeConstructor movingbool(
        "mbool", //name
        MBoolProperty,  //property function describing signature
        OutMapping<MBool, UBool, OutConstTemporalUnit<CcBool, OutCcBool> >,
        InMapping<MBool, UBool, InConstTemporalUnit<CcBool, InCcBool> >,
       //Out and In functions
        0,
        0,    //SaveToList and RestoreFromList functions
        CreateMapping<MBool>,
        DeleteMapping<MBool>,        //object creation and deletion
        0,
        0,                // object open and save
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
  return (nl->IsEqual( type, "mint" ));
}

/*
4.10.4 Creation of the type constructor ~mint~

*/
TypeConstructor movingint(
        "mint",               //name
        MIntProperty,   //property function describing signature
        OutMapping<MInt, UInt, OutConstTemporalUnit<CcInt, OutCcInt> >,
        InMapping<MInt, UInt, InConstTemporalUnit<CcInt, InCcInt> >,
    //Out and In functions
        0,
        0,            //SaveToList and RestoreFromList functions
        CreateMapping<MInt>,
        DeleteMapping<MInt>,   //object creation and deletion
        0,
        0,           // object open and save
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
  return (nl->IsEqual( type, "mreal" ));
}

/*
4.11.4 Creation of the type constructor ~mreal~

*/
TypeConstructor movingreal(
        "mreal",                    //name
        MRealProperty,    //property function describing signature
        OutMapping<MReal, UReal, OutUReal>,
        InMapping<MReal, UReal, InUReal>,   //Out and In functions
        0,
        0,      //SaveToList and RestoreFromList functions
        CreateMapping<MReal>,
        DeleteMapping<MReal>,    //object creation and deletion
        0,
        0,        // object open and save
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
  return (nl->IsEqual( type, "mpoint" ));
}

/*
4.12.4 Creation of the type constructor ~mpoint~

*/
TypeConstructor movingpoint(
        "mpoint",                           //name
        MPointProperty,        //property function describing signature
        OutMapping<MPoint, UPoint, OutUPoint>,
        InMapping<MPoint, UPoint, InUPoint>,//Out and In functions
        0,
        0,                 //SaveToList and RestoreFromList functions
        CreateMapping<MPoint>,
        DeleteMapping<MPoint>,     //object creation and deletion
        0,
        0,      // object open and save
        CloseMapping<MPoint>,
        CloneMapping<MPoint>, //object close and clone
        CastMapping<MPoint>,    //cast function
        SizeOfMapping<MPoint>, //sizeof function
        CheckMPoint );  //kind checking function

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

    if( nl->IsEqual( arg1, "instant" ) ||
        nl->IsEqual( arg1, "rint" ) ||
        nl->IsEqual( arg1, "rreal" ) ||
        nl->IsEqual( arg1, "periods" ) ||
        nl->IsEqual( arg1, "ubool" ) ||
        nl->IsEqual( arg1, "uint" ) ||
        nl->IsEqual( arg1, "ureal" ) ||
        nl->IsEqual( arg1, "upoint" )) // ||
//        nl->IsEqual( arg1, "mbool" ) ||
//        nl->IsEqual( arg1, "mint" ) ||
//        nl->IsEqual( arg1, "mreal" ) ||
//        nl->IsEqual( arg1, "mpoint" ) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( (nl->IsEqual( arg1, "instant" ) && nl->IsEqual( arg2, "instant" )) ||
        (nl->IsEqual( arg1, "rint" ) && nl->IsEqual( arg2, "rint" )) ||
        (nl->IsEqual( arg1, "rreal" ) && nl->IsEqual( arg2, "rreal" )) ||
        (nl->IsEqual( arg1, "periods" ) && nl->IsEqual( arg2, "periods" )) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
}

ListExpr
TemporalTemporalTypeMapBool2( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if(  (nl->IsEqual( arg1, "mbool" ) && nl->IsEqual( arg2, "mbool" )) ||
        (nl->IsEqual( arg1, "mint" ) && nl->IsEqual( arg2, "mint" )) ||
        (nl->IsEqual( arg1, "mreal" ) && nl->IsEqual( arg2, "mreal" )) ||
        (nl->IsEqual( arg1, "mpoint" ) && nl->IsEqual( arg2, "mpoint" )) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
Type Mapping for the mbool2mint function

*/
ListExpr TemporalMBool2MInt(ListExpr args){
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("Single argument expected");
    return nl->SymbolAtom( "typeerror" );
  }
  if(nl->IsEqual(nl->First(args),"mbool")){
    return   nl->SymbolAtom("mint");
  }
  ErrorReporter::ReportError("mint expected");
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.1 Type Mapping for the ~ExtendDeftime~ function

*/
ListExpr ExtDeftimeTypeMap(ListExpr args){
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("two argument expected");
     return nl->SymbolAtom( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->AtomType(arg1)!=SymbolType || nl->AtomType(arg2)!=SymbolType){
     ErrorReporter::ReportError("simple types required");
     return nl->SymbolAtom( "typeerror" );
  }
  string sarg1 = nl->SymbolValue(arg1);
  string sarg2 = nl->SymbolValue(arg2);
  if( (sarg1=="mint" && sarg2=="uint" ) ||
      (sarg1=="mbool" && sarg2=="ubool") ){
     return nl->SymbolAtom(sarg1);
  }
  ErrorReporter::ReportError("(mint x uint)  or (mbool x ubool) needed");
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "instant" ) && nl->IsEqual( arg2, "instant" ) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( (nl->IsEqual( arg1, "rint" ) && nl->IsEqual( arg2, "rint" )) ||
        (nl->IsEqual( arg1, "rreal" ) && nl->IsEqual( arg2, "rreal" )) ||
        (nl->IsEqual( arg1, "periods" ) && nl->IsEqual( arg2, "periods" )) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( (nl->IsEqual( arg1, "rint" ) && nl->IsEqual( arg2, "rint" )) ||
        (nl->IsEqual( arg1, "rreal" ) && nl->IsEqual( arg2, "rreal" )) ||
        (nl->IsEqual( arg1, "periods" ) && nl->IsEqual( arg2, "periods" )) ||
        (nl->IsEqual( arg1, "int" ) && nl->IsEqual( arg2, "rint" )) ||
        (nl->IsEqual( arg1, "real" ) && nl->IsEqual( arg2, "rreal" )) ||
        (nl->IsEqual( arg1, "instant" ) && nl->IsEqual( arg2, "periods" )) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( (nl->IsEqual( arg1, "rint" ) && nl->IsEqual( arg2, "rint" )) ||
        (nl->IsEqual( arg1, "rreal" ) && nl->IsEqual( arg2, "rreal" )) ||
        (nl->IsEqual( arg1, "periods" ) && nl->IsEqual( arg2, "periods" )) ||
        (nl->IsEqual( arg1, "int" ) && nl->IsEqual( arg2, "rint" )) ||
        (nl->IsEqual( arg1, "real" ) && nl->IsEqual( arg2, "rreal" )) ||
        (nl->IsEqual( arg1, "instant" ) && nl->IsEqual( arg2, "periods" )) ||
        (nl->IsEqual( arg1, "rint" ) && nl->IsEqual( arg2, "int" )) ||
        (nl->IsEqual( arg1, "rreal" ) && nl->IsEqual( arg2, "real" )) ||
        (nl->IsEqual( arg1, "periods" ) && nl->IsEqual( arg2, "instant" )) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "rint" ) && nl->IsEqual( arg2, "rint" ) )
      return nl->SymbolAtom( "rint" );

    if( nl->IsEqual( arg1, "rreal" ) && nl->IsEqual( arg2, "rreal" ) )
      return nl->SymbolAtom( "rreal" );

    if( nl->IsEqual( arg1, "periods" ) && nl->IsEqual( arg2, "periods" ) )
      return nl->SymbolAtom( "periods" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "rint" ) )
      return nl->SymbolAtom( "int" );

    if( nl->IsEqual( arg1, "rreal" ) )
      return nl->SymbolAtom( "real" );

    if( nl->IsEqual( arg1, "periods" ) )
      return nl->SymbolAtom( "instant" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "rint" ) ||
        nl->IsEqual( arg1, "rreal" ) ||
        nl->IsEqual( arg1, "periods" ) ||
        nl->IsEqual( arg1, "mbool" ) ||
        nl->IsEqual( arg1, "mint" ) ||
        nl->IsEqual( arg1, "mreal" ) ||
        nl->IsEqual( arg1, "mpoint" ) )
      return nl->SymbolAtom( "int" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "ibool" ) )
      return nl->SymbolAtom( "bool" );

    if( nl->IsEqual( arg1, "iint" ) )
      return nl->SymbolAtom( "int" );

    if( nl->IsEqual( arg1, "ireal" ) )
      return nl->SymbolAtom( "real" );

    if( nl->IsEqual( arg1, "ipoint" ) )
      return nl->SymbolAtom( "point" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "ibool" ) ||
        nl->IsEqual( arg1, "iint" ) ||
        nl->IsEqual( arg1, "ireal" ) ||
        nl->IsEqual( arg1, "ipoint" ) )
      return nl->SymbolAtom( "instant" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg2, "instant" ) )
    {
      if( nl->IsEqual( arg1, "mbool" ) )
        return nl->SymbolAtom( "ibool" );

      if( nl->IsEqual( arg1, "mint" ) )
        return nl->SymbolAtom( "iint" );

      if( nl->IsEqual( arg1, "mreal" ) )
        return nl->SymbolAtom( "ireal" );

      if( nl->IsEqual( arg1, "mpoint" ) )
        return nl->SymbolAtom( "ipoint" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg2, "periods" ) )
    {
      if( nl->IsEqual( arg1, "mbool" ) )
        return nl->SymbolAtom( "mbool" );

      if( nl->IsEqual( arg1, "mint" ) )
        return nl->SymbolAtom( "mint" );

      if( nl->IsEqual( arg1, "mreal" ) )
        return nl->SymbolAtom( "mreal" );

      if( nl->IsEqual( arg1, "mpoint" ) )
        return nl->SymbolAtom( "mpoint" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "mbool" ) ||
        nl->IsEqual( arg1, "mint" ) ||
        nl->IsEqual( arg1, "mreal" ) ||
        nl->IsEqual( arg1, "mpoint" ) )
      return nl->SymbolAtom( "periods" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "mpoint" ) )
      return nl->SymbolAtom( "line" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg2, "instant" ) ||
        nl->IsEqual( arg2, "periods" ) )
    {
      if( nl->IsEqual( arg1, "mbool" ) ||
          nl->IsEqual( arg1, "mint" ) ||
          nl->IsEqual( arg1, "mreal" ) ||
          nl->IsEqual( arg1, "mpoint" ) )
        return nl->SymbolAtom( "bool" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( (nl->IsEqual( arg1, "mbool" ) && nl->IsEqual( arg2, "bool" )) ||
        (nl->IsEqual( arg1, "mint" ) && nl->IsEqual( arg2, "int" )) ||
// VTA - This operator is not yet implemented for the type of ~mreal~
//        (nl->IsEqual( arg1, "mreal" ) && nl->IsEqual( arg2, "real" )) ||
        (nl->IsEqual( arg1, "mpoint" ) && nl->IsEqual( arg2, "point" )) ||
        (nl->IsEqual( arg1, "mpoint" ) && nl->IsEqual( arg2, "region" )) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "mbool" ) && nl->IsEqual( arg2, "bool" ) )
      return nl->SymbolAtom( "mbool" );

    if( nl->IsEqual( arg1, "mint" ) && nl->IsEqual( arg2, "int" ) )
      return nl->SymbolAtom( "mint" );

// VTA - This operator is not yet implemented for the type of ~mreal~
//    if( nl->IsEqual( arg1, "mreal" ) && nl->IsEqual( arg2, "real" ) )
//      return nl->SymbolAtom( "mreal" );

    if( nl->IsEqual( arg1, "mpoint" ) && nl->IsEqual( arg2, "point" ) )
      return nl->SymbolAtom( "mpoint" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "mbool" ) )
      return nl->SymbolAtom( "ibool" );

    if( nl->IsEqual( arg1, "mint" ) )
      return nl->SymbolAtom( "iint" );

    if( nl->IsEqual( arg1, "mreal" ) )
      return nl->SymbolAtom( "ireal" );

    if( nl->IsEqual( arg1, "mpoint" ) )
      return nl->SymbolAtom( "ipoint" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "mpoint" ) && nl->IsEqual( arg2, "point" ) )
      return nl->SymbolAtom( "mreal" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "mbool" ) )
      return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("ubool"));

    if( nl->IsEqual( arg1, "mint" ) )
      return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("uint"));

    if( nl->IsEqual( arg1, "mreal" ) )
      return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("ureal"));

    if( nl->IsEqual( arg1, "mpoint" ) )
      return nl->TwoElemList(nl->SymbolAtom("stream"),
       nl->SymbolAtom("upoint"));
  }
  return nl->SymbolAtom("typeerror");
}

/*
16.1.12 Type mapping for simplify operator

*/

ListExpr MovingTypeMapSimplify(ListExpr args){
   int len = nl->ListLength(args);

   if((len!=2) && (len !=3)){
       ErrorReporter::ReportError("two or three arguments expected");
       return nl->SymbolAtom("typeerror");
   }
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if(nl->IsEqual(arg1,"mpoint") &&
      nl->IsEqual(arg2,"real")){
        if(len==2){
           return nl->SymbolAtom("mpoint");
        } else { // check the third argument
          ListExpr arg3 = nl->Third(args);
          if(nl->IsEqual(arg3,"duration")){
             return nl->SymbolAtom("mpoint");
          }
        }
   }
   if( (len==2) && (nl->IsEqual(arg1,"mreal")) &&
       (nl->IsEqual(arg2,"real"))){
       return nl->SymbolAtom("mreal");
   }

   ErrorReporter::ReportError(" (mpoint x real [ x duration])"
                             "  or (mreal x real) expected");
   return nl->SymbolAtom("typeerror");
}


/*
16.1.12 Type mapping for the breakpoints  operator

*/

ListExpr MovingTypeMapBreakPoints(ListExpr args){
   if(nl->ListLength(args)!=2){
       ErrorReporter::ReportError("two arguments expected");
       return nl->SymbolAtom("typeerror");
   }
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if(nl->IsEqual(arg1,"mpoint") &&
      nl->IsEqual(arg2,"duration")){
       return nl->SymbolAtom("points");
   }
   ErrorReporter::ReportError("mpoint x duration expected");
   return nl->SymbolAtom("typeerror");
}

/*
16.1.12 Type mapping function of the ~integrate~ Operator

*/
ListExpr TypeMapIntegrate(ListExpr args){
   int len = nl->ListLength(args);
   if(len!=1){
      ErrorReporter::ReportError("one argument expected");
      return nl->SymbolAtom("typeerror");
   }
   ListExpr arg = nl->First(args);
   if(nl->IsEqual(arg,"ureal") ||
      nl->IsEqual(arg,"mreal")){
      return nl->SymbolAtom("real");
   }
   ErrorReporter::ReportError("ureal or mreal expected");
   return nl->SymbolAtom("typeerror");
}

/*
16.1.12 Type mapping function of the ~min~ and the ~max~ Operator

*/
ListExpr TypeMapMinMax(ListExpr args){
   int len = nl->ListLength(args);
   if(len!=1){
      ErrorReporter::ReportError("one argument expected");
      return nl->SymbolAtom("typeerror");
   }
   ListExpr arg = nl->First(args);
   if(nl->IsEqual(arg,"ureal") ||
      nl->IsEqual(arg,"mreal")){
      return nl->SymbolAtom("real");
   }
   ErrorReporter::ReportError("ureal or mreal expected");
   return nl->SymbolAtom("typeerror");
}



/*
16.1.13 Type mapping function ~IntSetTypeMapPeriods~

It is used for the operators ~theyear~, ~themonth~, ~theday~, ~thehour~, ~theminute~,
~thesecond~

*/
ListExpr
IntSetTypeMapPeriods( ListExpr args )
{
  ListExpr argi;
  bool correct=true;

  if( ( nl->ListLength(args) < 1 ) || ( nl->ListLength(args) > 6) )
    return nl->SymbolAtom("typeerror");

  if( nl->ListLength(args) >= 1 )
  {
    argi = nl->First(args);
    if( !nl->IsEqual( argi, "int" ) )
      correct = false;
  }

  if ( nl->ListLength(args) >= 2 )
  {
    argi = nl->Second(args);
    if( !nl->IsEqual( argi, "int" ) )
      correct = false;
  }

  if ( nl->ListLength(args) >= 3 )
  {
    argi = nl->Third(args);
    if( !nl->IsEqual( argi, "int" ) )
      correct = false;
  }

  if ( nl->ListLength(args) >= 4 )
  {
    argi = nl->Fourth(args);
    if( !nl->IsEqual( argi, "int" ) )
      correct=false;
  }

  if( nl->ListLength(args) >= 5 )
  {
    argi = nl->Fifth(args);
    if( !nl->IsEqual( argi, "int" ) )
      correct=false;
  }

  if ( nl->ListLength(args) >= 6 )
  {
    argi = nl->Sixth(args);
    if( !nl->IsEqual( argi, "int" ) )
      correct=false;
  }

  if( correct )
    return nl->SymbolAtom( "periods" );

  return nl->SymbolAtom("typeerror");
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

    if( nl->IsEqual( arg1, "periods" ) && nl->IsEqual( arg2, "periods" ) )
      return nl->SymbolAtom( "periods" );
  }
  return nl->SymbolAtom("typeerror");
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

    if( nl->IsEqual( arg1, "mpoint" ) &&
        nl->IsEqual(nl->First( arg2 ), "duration") &&
        nl->IsEqual(nl->Second( arg2 ), "real") &&
        nl->IsEqual(nl->Third( arg2 ), "real")) {
      return (nl->SymbolAtom( "mpoint" )); }
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.16 Type mapping function "box3d"

*/
ListExpr Box3dTypeMap(ListExpr args){
  int len = nl->ListLength(args);
  if(len==2){
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if(nl->IsEqual(arg1,"rect")){
       if(nl->IsEqual(arg2,"instant") || nl->IsEqual(arg2,"periods"))
         return nl->SymbolAtom("rect3");
    }
    ErrorReporter::ReportError(" rect x {instant, periods } expected\n");
    return nl->SymbolAtom("typeerror");
  } else if(len==1){
    ListExpr arg = nl->First(args);
    if(nl->IsEqual(arg,"rect") || (nl->IsEqual(arg,"instant") ||
       nl->IsEqual(arg,"periods")))
        return nl->SymbolAtom("rect3");
    ErrorReporter::ReportError("rect, instant, or periods required\n");
    return nl->SymbolAtom("typeerror" );
  }
   else{
    ErrorReporter::ReportError("one or two arguments required");
    return nl->SymbolAtom("typeerror");
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

    if( nl->IsEqual( arg1, "rect3" ) )
      return nl->SymbolAtom( "rect" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.18 Type mapping function "TemporalBBoxTypeMap"

For operator ~bbox~

*/

ListExpr TemporalBBoxTypeMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "upoint" ) )
      return (nl->SymbolAtom( "rect3" ));

    if( nl->IsEqual( arg1, "rint" )  )
      return (nl->SymbolAtom( "rint" ));

    if( nl->IsEqual( arg1, "rreal" ) )
      return (nl->SymbolAtom( "rreal" ));

    if( nl->IsEqual( arg1, "periods" ) )
      return (nl->SymbolAtom( "periods" ));
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.2 Selection function

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

  if( nl->SymbolValue( arg1 ) == "rint" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "rreal" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "periods" )
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

  if( nl->SymbolValue( arg1 ) == "rint" && nl->SymbolValue( arg2 ) == "rint" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "rreal" && nl->SymbolValue( arg2 ) == "rreal" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "periods" && nl->SymbolValue
  ( arg2 ) == "periods" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "int" && nl->SymbolValue( arg2 ) == "rint" )
    return 3;

  if( nl->SymbolValue( arg1 ) == "real" && nl->SymbolValue( arg2 ) == "rreal" )
    return 4;

  if( nl->SymbolValue( arg1 ) == "instant" && nl->SymbolValue
  ( arg2 ) == "periods" )
    return 5;

  if( nl->SymbolValue( arg1 ) == "rint" && nl->SymbolValue( arg2 ) == "int" )
    return 6;

  if( nl->SymbolValue( arg1 ) == "rreal" && nl->SymbolValue( arg2 ) == "real" )
    return 7;

  if( nl->SymbolValue( arg1 ) == "periods" && nl->SymbolValue
  ( arg2 ) == "instant" )
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

  if( nl->SymbolValue( arg1 ) == "instant" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "rint" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "rreal" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "periods" )
    return 3;

  if( nl->SymbolValue( arg1 ) == "ubool" )
    return 4;

  if( nl->SymbolValue( arg1 ) == "uint" )
    return 5;

  if( nl->SymbolValue( arg1 ) == "ureal" )
    return 6;

  if( nl->SymbolValue( arg1 ) == "upoint" )
    return 7;

//  if( nl->SymbolValue( arg1 ) == "mbool" )
//    return 8;

//  if( nl->SymbolValue( arg1 ) == "mint" )
//    return 9;

//  if( nl->SymbolValue( arg1 ) == "mreal" )
//    return 10;

//  if( nl->SymbolValue( arg1 ) == "mpoint" )
//    return 11;

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

  if( nl->SymbolValue( arg1 ) == "instant" &&
    nl->SymbolValue( arg2 ) == "instant" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "rint" && nl->SymbolValue( arg2 ) == "rint" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "rreal" && nl->SymbolValue( arg2 ) == "rreal" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "periods" &&
    nl->SymbolValue( arg2 ) == "periods" )
    return 3;

  return -1; // This point should never be reached
}

int
TemporalDualSelect2( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == "mbool" && nl->SymbolValue( arg2 ) == "mbool" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "mint" && nl->SymbolValue( arg2 ) == "mint" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "mreal" && nl->SymbolValue( arg2 ) == "mreal" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "mpoint" && nl->SymbolValue
  ( arg2 ) == "mpoint" )
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

  if( nl->SymbolValue( arg1 ) == "rint" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "rreal" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "periods" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "mbool" )
    return 3;

  if( nl->SymbolValue( arg1 ) == "mint" )
    return 4;

  if( nl->SymbolValue( arg1 ) == "mreal" )
    return 5;

  if( nl->SymbolValue( arg1 ) == "mpoint" )
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

  if( nl->SymbolValue( arg1 ) == "ibool" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "iint" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "ireal" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "ipoint" )
    return 3;

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

  if( nl->SymbolValue( arg1 ) == "mbool" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "mint" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "mreal" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "mpoint" )
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

  if( nl->SymbolValue( arg1 ) == "mbool" && nl->SymbolValue
  ( arg2 ) == "instant" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "mint" && nl->SymbolValue
  ( arg2 ) == "instant" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "mreal" && nl->SymbolValue
  ( arg2 ) == "instant" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "mpoint" && nl->SymbolValue
  ( arg2 ) == "instant" )
    return 3;

  if( nl->SymbolValue( arg1 ) == "mbool" && nl->SymbolValue
  ( arg2 ) == "periods" )
    return 4;

  if( nl->SymbolValue( arg1 ) == "mint" && nl->SymbolValue
  ( arg2 ) == "periods" )
    return 5;

  if( nl->SymbolValue( arg1 ) == "mreal" && nl->SymbolValue
  ( arg2 ) == "periods" )
    return 6;

  if( nl->SymbolValue( arg1 ) == "mpoint" && nl->SymbolValue
  ( arg2 ) == "periods" )
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

  if( nl->SymbolValue( arg1 ) == "mbool" &&
      nl->SymbolValue( arg2 ) == "bool" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "mint" &&
      nl->SymbolValue( arg2 ) == "int" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "mreal" &&
      nl->SymbolValue( arg2 ) == "real" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "mpoint" &&
      nl->SymbolValue( arg2 ) == "point" )
    return 3;

  if( nl->SymbolValue( arg1 ) == "mpoint" &&
      nl->SymbolValue( arg2 ) == "region" )
    return 4;

  return -1; // This point should never be reached
}


/*
Selection function for the box3d operator

*/

int Box3dSelect(ListExpr args){
  int len = nl->ListLength(args);
  if(len==1){
    ListExpr arg = nl->First(args);
    if(nl->IsEqual(arg,"rect"))
        return 0;
    if(nl->IsEqual(arg,"instant"))
        return 1;
    if(nl->IsEqual(arg,"periods"))
        return 3;
  }
  if(len==2){
     if(nl->IsEqual(nl->First(args),"rect")){
         ListExpr arg2 = nl->Second(args);
         if(nl->IsEqual(arg2,"instant"))
            return 2;
         if(nl->IsEqual(arg2,"periods"))
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

  if( nl->SymbolValue( arg1 ) == "upoint" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "rint" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "rreal" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "periods" )
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
   if(nl->IsEqual(arg,"mbool"))
      return 0;
   if(nl->IsEqual(arg,"mint"))
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
       if(nl->IsEqual(nl->First(args),"mpoint")){
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
   if(nl->IsEqual(arg,"ureal")){
       return 0;
   }
   if(nl->IsEqual(arg,"mreal")){
       return 1;
   }
   return -1; // should never occur

}

/*
16.2.32 Selection function for ~min~ and ~max~

*/
int MinMaxSelect(ListExpr args){
   ListExpr arg = nl->First(args);
   if(nl->IsEqual(arg,"ureal")){
       return 0;
   }
   if(nl->IsEqual(arg,"mreal")){
       return 1;
   }
   return -1; // should never occur
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
  if( !((Instant*)args[0].addr)->IsDefined() )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

template <class Range>
int RangeIsEmpty( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[0].addr)->IsEmpty() )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
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
  if(I1->IsDefined() && I2->IsDefined()){
      ((CcBool *)result.addr)->Set( true, (I1->CompareTo(I2)==0)) ;
  } else {
      ((CcBool *)result.addr)->Set( false,false) ;
  }
  return 0;
}

template <class Range>
int RangeEqual( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( *((Range*)args[0].addr) == *((Range*)args[1].addr) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
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
  if(I1->IsDefined() && I2->IsDefined()){
      ((CcBool *)result.addr)->Set( true, (I1->CompareTo(I2)!=0)) ;
  } else {
      ((CcBool *)result.addr)->Set( false,false) ;
  }
  return 0;
}

template <class Range>
int RangeNotEqual( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( *((Range*)args[0].addr) != *((Range*)args[1].addr) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
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
  if( ((Range*)args[0].addr)->Intersects( *((Range*)args[1].addr) ) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
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
  if( ((Range*)args[0].addr)->Inside( *((Range*)args[1].addr) ) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}


template <class Alpha, class Range>
int RangeInside_ar( Word* args, Word& result, int message, Word& local,
  Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Alpha*)args[0].addr)->IsDefined() &&
       ((Range*)args[1].addr)->IsDefined() )
  {

    if( ((Range*)args[1].addr)->Contains( *((Alpha*)args[0].addr) ) )
      ((CcBool*)result.addr)->Set( true, true );
    else
      ((CcBool *)result.addr)->Set( true, false );
  }

  else
    ((CcBool *)result.addr)->Set( false, false );

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
  if( ((Range*)args[0].addr)->Before( *((Range*)args[1].addr) ) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

template <class Alpha, class Range>
int RangeBefore_ar( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[1].addr)->After( *((Alpha*)args[0].addr) ) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

template <class Range, class Alpha>
int RangeBefore_ra( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[0].addr)->Before( *((Alpha*)args[1].addr) ) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
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
  ((CcInt *)result.addr)->Set( true,
   ((Range*)args[0].addr)->GetNoComponents() );
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
      return 0;
   }
   double res = arg->Integrate();
   ((CcReal*)result.addr)->Set(true,res);
   return 0;
}


/*
16.2.28 Value Mapping function for the operator min

*/
template <class mtype>
int VM_Min(Word* args, Word& result,
              int message, Word& local,
              Supplier s){
   result = qp->ResultStorage(s);
   mtype* arg = (mtype*) args[0].addr;
   bool correct;
   double res = arg->Min(correct);
   ((CcReal*)result.addr)->Set(correct,res);
   return 0;
}

/*
16.2.28 Value Mapping function for the operator min

*/
template <class mtype>
int VM_Max(Word* args, Word& result,
              int message, Word& local,
              Supplier s){
   result = qp->ResultStorage(s);
   mtype* arg = (mtype*) args[0].addr;
   bool correct;
   double res = arg->Max(correct);
   ((CcReal*)result.addr)->Set(correct,res);
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
  ((MPoint*)args[0].addr)->BreakPoints(*((Points*)result.addr),*dur );
  return 0;
}

/*
16.3.31 Value mapping functions of operator ~bbox~

*/
static int
UPointBBox( Word* args, Word& result, int message, Word& local,
 Supplier s )
{
  result = qp->ResultStorage( s );
  *((Rectangle<3>*)result.addr) = ((UPoint*)args[0].addr)->BoundingBox();
  return (0);
}

template <class Range>
int RangeBBox( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );

  if( ((Range*)args[0].addr)->IsEmpty() )
    ((Range*)result.addr)->SetDefined( false );
  else
    ((Range*)args[0].addr)->RBBox( *(Range*)result.addr);
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
  const UPoint *uPoint;
  MPoint* mp, *mpResult;

  result = qp->ResultStorage( s );

  mp= (MPoint*)args[0].addr,
  mpResult = (MPoint*)result.addr;

  Supplier son = qp->GetSupplier( args[1].addr, 0 );
  qp->Request( son, t );
  dd = (DateTime *)t.addr;

  son = qp->GetSupplier( args[1].addr, 1 );
  qp->Request( son, t );
  dx = ((CcReal *)t.addr)->GetRealval();

  son = qp->GetSupplier( args[1].addr, 2 );
  qp->Request( son, t );
  dy = ((CcReal *)t.addr)->GetRealval();

  if ( mp->IsDefined() )
  {
    mpResult->Clear();
    mpResult->StartBulkLoad();
    for( int i = 0; i < mp->GetNoComponents(); i++ )
    {
      mp->Get( i, uPoint );
      UPoint aux( *uPoint );
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

  int intyear = ((CcInt*)args[0].addr)->GetIntval();

  Instant inst1, inst2;
  inst1.SetType( instanttype ) ;
  inst1.Set( intyear, 1, 1, 0, 0, 0, 0 );

  inst2.SetType( instanttype );
  inst2.Set( intyear + 1, 1, 1, 0, 0, 0, 0 );

  Interval<Instant> timeInterval(inst1, inst2, true, false);

  pResult->Clear();
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

  pResult->Clear();
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

  pResult->Clear();
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

  pResult->Clear();
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

  pResult->Clear();
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

  pResult->Clear();
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

  if( !range1->IsEmpty() || !range2->IsEmpty() )
  {
    const Interval<Instant> *intv1, *intv2;
    if( range1->IsEmpty() )
    {
      range2->Get( 0, intv1 );
      range2->Get( range2->GetNoComponents()-1, intv2 );
    }
    else if( range2->IsEmpty() )
    {
      range1->Get( 0, intv1 );
      range1->Get( range1->GetNoComponents()-1, intv2 );
    }
    else
    {
      range1->Get( 0, intv1 );
      range2->Get( range2->GetNoComponents()-1, intv2 );
    }

    Interval<Instant> timeInterval( intv1->start, intv2->end,
     intv1->lc, intv2->rc );

    pResult->StartBulkLoad();
    pResult->Add( timeInterval );
    pResult->EndBulkLoad( false );
  }
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
   double min[3];
   double max[3];
   min[0] = arg->MinD(0);
   min[1] = arg->MinD(1);
   min[2] = MINDOUBLE;
   max[0] = arg->MaxD(0);
   max[1] = arg->MaxD(1);
   max[2] = MAXDOUBLE;
   res->Set(true,min,max);
   return 0;
}

int Box3d_instant( Word* args, Word& result, int message, Word&
 local, Supplier s ){
   result = qp->ResultStorage(s);
   Rectangle<3>* res = (Rectangle<3>*) result.addr;
   Instant* arg = (Instant*) args[0].addr;
   double min[3];
   double max[3];
   min[0] = MINDOUBLE;
   min[1] = MINDOUBLE;
   max[0] = MAXDOUBLE;
   max[1] = MAXDOUBLE;
   double v = arg->ToDouble();
   max[2] = v;
   min[2] = v;
   res->Set(true,min,max);
   return 0;
}


int Box3d_rect_instant( Word* args, Word& result, int message,
 Word& local, Supplier s ){
   result = qp->ResultStorage(s);
   Rectangle<3>* res = (Rectangle<3>*) result.addr;
   Rectangle<2>* arg1 = (Rectangle<2>*) args[0].addr;
   Instant* arg2 = (Instant*) args[1].addr;
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
   return 0;
}


int Box3d_periods( Word* args, Word& result, int message, Word&
 local, Supplier s ){
   result = qp->ResultStorage(s);
   Rectangle<3>* res = (Rectangle<3>*) result.addr;
   Periods* arg = (Periods*) args[0].addr;
   Instant i;
   arg->Minimum(i);
   double v1 = i.ToDouble();
   arg->Maximum(i);
   double v2 = i.ToDouble();
   double min[3];
   double max[3];
   min[0] = MINDOUBLE;
   min[1] = MINDOUBLE;
   min[2] = v1;
   max[0] = MAXDOUBLE;
   max[1] = MAXDOUBLE;
   max[2] = v2;
   res->Set(true,min,max);
   return 0;
}

int Box3d_rect_periods( Word* args, Word& result, int message,
 Word& local, Supplier s ){
   result = qp->ResultStorage(s);
   Rectangle<3>* res = (Rectangle<3>*) result.addr;
   Rectangle<2>* arg1 = (Rectangle<2>*) args[0].addr;
   Periods* arg2 = (Periods*) args[1].addr;
   Instant i;
   arg2->Minimum(i);
   double v1 = i.ToDouble();
   arg2->Maximum(i);
   double v2 = i.ToDouble();
   double min[3];
   double max[3];
   min[0] = arg1->MinD(0);
   min[1] = arg1->MinD(1);
   min[2] = v1;
   max[0] = arg1->MaxD(0);
   max[1] = arg1->MaxD(1);
   max[2] = v2;
   res->Set(true,min,max);
   return 0;
}

/*
16.3.39 Value Mapping function for box2d

*/

int TemporalBox2d( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  result = qp->ResultStorage(s);
  Rectangle<2>* res = (Rectangle<2>*) result.addr;
  Rectangle<3>* arg = (Rectangle<3>*) args[0].addr;

  double min[2], max[2];
  min[0] = arg->MinD(0);
  min[1] = arg->MinD(1);
  max[0] = arg->MaxD(0);
  max[1] = arg->MaxD(1);
  res->Set( true, min, max );

  return 0;
}

/*
16.3.40 Value Mapping function for ExtDefTime

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
16.4 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of value
mapping functions for each operator. For nonoverloaded operators there is also such and array
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
                                      UnitIsEmpty<UPoint> };


ValueMapping temporalequalmap[] = { InstantEqual,
                                    RangeEqual<RInt>,
                                    RangeEqual<RReal>,
                                    RangeEqual<Instant>};

ValueMapping temporalequalmap2[] = { MappingEqual<MBool>,
                                    MappingEqual<MInt>,
                                    MappingEqual<MReal>,
                                    MappingEqual<MPoint> };

ValueMapping temporalnotequalmap[] = { InstantNotEqual,
                                       RangeNotEqual<RInt>,
                                       RangeNotEqual<RReal>,
                                       RangeNotEqual<Periods>};

ValueMapping temporalnotequalmap2[] = { MappingNotEqual<MBool>,
                                       MappingNotEqual<MInt>,
                                       MappingNotEqual<MReal>,
                                       MappingNotEqual<MPoint> };

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
                                   RangeBBox<RInt>,
                                   RangeBBox<RReal>,
                                   RangeBBox<Periods> };


ValueMapping temporalinstmap[] = { IntimeInst<CcBool>,
                                   IntimeInst<CcInt>,
                                   IntimeInst<CcReal>,
                                   IntimeInst<Point> };

ValueMapping temporalvalmap[] = { IntimeVal<CcBool>,
                                  IntimeVal<CcInt>,
                                  IntimeVal<CcReal>,
                                  IntimeVal<Point> };

ValueMapping temporalatinstantmap[] = { MappingAtInstant<MBool, CcBool>,
                                        MappingAtInstant<MInt, CcInt>,
                                        MappingAtInstant<MReal, CcReal>,
                                        MappingAtInstant<MPoint, Point> };

ValueMapping temporalatperiodsmap[] = { MappingAtPeriods<MBool>,
                                        MappingAtPeriods<MInt>,
                                        MappingAtPeriods<MReal>,
                                        MappingAtPeriods<MPoint> };

ValueMapping temporaldeftimemap[] = { MappingDefTime<MBool>,
                                      MappingDefTime<MInt>,
                                      MappingDefTime<MReal>,
                                      MappingDefTime<MPoint> };

ValueMapping temporalpresentmap[] = { MappingPresent_i<MBool>,
                                      MappingPresent_i<MInt>,
                                      MappingPresent_i<MReal>,
                                      MappingPresent_i<MPoint>,
                                      MappingPresent_p<MBool>,
                                      MappingPresent_p<MInt>,
                                      MappingPresent_p<MReal>,
                                      MappingPresent_p<MPoint> };

ValueMapping temporalpassesmap[] = { MappingPasses<MBool, CcBool, CcBool>,
                                     MappingPasses<MInt, CcInt, CcInt>,
                                     MappingPasses<MReal, CcReal, CcReal>,
                                     MappingPasses<MPoint, Point, Point>,
                                     MappingPasses<MPoint, Point, Region> };

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
                                 MappingAt<MReal, UReal, CcReal>,
                                 MappingAt<MPoint, UPoint, Point> };

ValueMapping temporalunitsmap[] = { MappingUnits<MBool, UBool>,
                                    MappingUnits<MBool, UBool>,
                                    MappingUnits<MReal, UReal>,
                                    MappingUnits<MPoint, UPoint> };

ValueMapping temporalbox3dmap[] = { Box3d_rect,
                                    Box3d_instant,
                                    Box3d_rect_instant,
                                    Box3d_periods,
                                    Box3d_rect_periods};

ValueMapping extdeftimemap[] = { TemporalExtDeftime<UBool, CcBool>,
                                 TemporalExtDeftime<UInt, CcInt>
                               };

ValueMapping simplifymap[] = { MPointSimplify, MPointSimplify2,
                               MRealSimplify };

ValueMapping integratemap[] = { Integrate<UReal>, Integrate<MReal> };

ValueMapping minmap[] = { VM_Min<UReal>, VM_Min<MReal> };

ValueMapping maxmap[] = { VM_Max<UReal>, VM_Max<MReal> };


/*
16.4.2 Specification strings

*/
const string TemporalSpecIsEmpty  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>instant -> bool,\n"
  "rT -> bool \n"
  "uT -> bool</text--->"
  "<text>isempty ( _ )</text--->"
  "<text>Returns whether the instant/range/unit type value is "
  "empty or not.</text--->"
  "<text>query isempty( mpoint1 )</text--->"
  ") )";

const string TemporalSpecEQ  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool, \n"
  "(rT rT) -> bool</text--->"
  "<text>_ = _</text--->"
  "<text>Is-Equal predicate for instant and range type values.</text--->"
  "<text>query i1 = i2</text--->"
  ") )";

const string TemporalSpecEQ2  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(moving(x) moving(x)) -> bool</text--->"
  "<text>_ equal _</text--->"
  "<text>Equal.</text--->"
  "<text>query mi equal mi2</text--->"
  ") )";

const string TemporalSpecNE  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool,\n"
  "(rT rT)) -> bool</text--->"
  "<text>_ # _</text--->"
  "<text>Not-Equal predicate for instant and rage type values.</text--->"
  "<text>query i1 # i2</text--->"
  ") )";

const string TemporalSpecNE2  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(mT mT) -> bool</text--->"
  "<text>_ nonequal _</text--->"
  "<text>Not-Equal predicate for moving objects.</text--->"
  "<text>query mi1 nonequal mi2</text--->"
  ") )";

const string TemporalSpecLT  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool</text--->"
  "<text>_ < _</text--->"
  "<text>Less than.</text--->"
  "<text>query i1 < i2</text--->"
  ") )";

const string TemporalSpecLE  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool</text--->"
  "<text>_ <= _</text--->"
  "<text>Less or equal than.</text--->"
  "<text>query i1 <= i2</text--->"
  ") )";

const string TemporalSpecGT  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool</text--->"
  "<text>_ > _</text--->"
  "<text>Greater than.</text--->"
  "<text>query i1 > i2</text--->"
  ") )";

const string TemporalSpecGE  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool</text--->"
  "<text>_ >= _</text--->"
  "<text>Greater or equal than.</text--->"
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
  "Number of units inside a moving object value.</text--->"
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

const string TemporalSpecAtInstant =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT instant) -> iT</text--->"
  "<text>_ atinstant _ </text--->"
  "<text>From a moving object Get the intime value "
  "corresponding to the instant.</text--->"
  "<text>mpoint1 atinstant instant1</text--->"
  ") )";

const string TemporalSpecAtPeriods =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT periods) -> mT</text--->"
  "<text>_ atperiods _ </text--->"
  "<text>Restrict the moving object to the given periods.</text--->"
  "<text>mpoint1 atperiods periods1</text--->"
  ") )";

const string TemporalSpecDefTime  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>moving(x) -> periods</text--->"
  "<text>deftime( _ )</text--->"
  "<text>get the defined time of the corresponding moving data "
  "objects.</text--->"
  "<text>deftime( mp1 )</text--->"
  ") )";

const string TemporalSpecTrajectory =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint -> line</text--->"
  "<text> trajectory( _ )</text--->"
  "<text>get the trajectory of the corresponding moving point object.</text--->"
  "<text>trajectory( mp1 )</text--->"
  ") )";

const string TemporalSpecPresent  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT instant) -> bool,\n"
  "(mT periods) -> bool</text--->"
  "<text>_ present _ </text--->"
  "<text>whether the moving object is present at the given "
  "instant or period.</text--->"
  "<text>mpoint1 present instant1</text--->"
  ") )";

const string TemporalSpecPasses =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT T) -> bool</text--->"
  "<text>_ passes _ </text--->"
  "<text>whether the moving object passes the given value.</text--->"
  "<text>mpoint1 passes point1</text--->"
                                ") )";

const string TemporalSpecInitial  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mT -> iT</text--->"
  "<text>initial( _ )</text--->"
  "<text>get the intime value corresponding to the initial instant.</text--->"
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
  "<text>returns the moving distance</text--->"
  "<text>distance( mpoint1, point1 )</text--->"
  ") )";

const string TemporalSpecSimplify =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint x real [ x duration ] -> mpoint |"
  " mreal x real -> mreal</text--->"
  "<text>simplify( _, _ [, _]) </text--->"
  "<text>simplifys the argument with a maximum difference of epsilon</text--->"
  "<text>simplify( train7, 50.0, [const duration value (0, 10000)] )</text--->"
  ") )";

const string TemporalSpecIntegrate =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>{ureal , mreal} -> real</text--->"
  "<text>integrate( _ ) </text--->"
  "<text>computes the determined inegtral of the argument</text--->"
  "<text>integrate(mreal5000)</text--->"
  ") )";

const string TemporalSpecMin =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>{ureal , mreal} -> real</text--->"
  "<text>minimum( _ ) </text--->"
  "<text>computes the minimum value of the argument</text--->"
  "<text>minimum(mreal5000)</text--->"
  ") )";

const string TemporalSpecMax =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>{ureal , mreal} -> real</text--->"
  "<text>maximum( _ ) </text--->"
  "<text>computes the maximum value of the argument</text--->"
  "<text>maximum(mreal5000)</text--->"
  ") )";


const string TemporalSpecBreakPoints =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint x duration -> points</text--->"
  "<text>breakpoints( _, _ ) </text--->"
  "<text>computes all points where the mpoints stops longer"
  " than the given duration</text--->"
  "<text>breakpoints( train7, [const duration value (0 1000)] )</text--->"
  ") )";

const string TemporalSpecUnits  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in {bool, int, real, point}:\n"
  "   mT -> (stream uT)</text--->"
  "<text> units( _ )</text--->"
  "<text>get the stream of units of the moving value.</text--->"
  "<text>units( mpoint1 )</text--->"
  ") )";

const string TemporalSpecBBox  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>upoint -> rect3,\n"
  "   rT -> rT</text--->"
  "<text>bbox ( _ )</text--->"
  "<text>Returns the 3d bounding box of the unit (for upoint)\n"
  "or the range value with the smallest closed interval that\n"
  "contains all intervals of a range-value (for range-value).</text--->"
  "<text>query bbox( upoint1 )</text--->"
  ") )";

const string MPointSpecTranslate  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint x duration x real x real -> mpoint</text--->"
  "<text>_ translate[list]</text--->"
  "<text>Moves the object parallely for distance and time.</text--->"
  "<text>query mp1 translate[[const duration value (5 10)],5.0,8.0]</text--->"
  ") )";

const string TemporalSpecTheYear  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int -> periods</text--->"
  "<text> theyear( _ )</text--->"
  "<text>get the periods value of the year.</text--->"
  "<text>theyear(2002)</text--->"
                                ") )";

const string TemporalSpecTheMonth  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int x int -> periods</text--->"
  "<text> themonth( _, _ )</text--->"
  "<text>get the periods value of the month.</text--->"
  "<text>themonth(2002, 3)</text--->"
                                ") )";

const string TemporalSpecTheDay  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int x int x int -> periods</text--->"
  "<text>theday( _, _, _ )</text--->"
  "<text>get the periods value of the day.</text--->"
  "<text>theday(2002, 6,3)</text--->"
  ") )";

const string TemporalSpecTheHour  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int x int x int x int -> periods</text--->"
  "<text>thehour( _, _, _ , _)</text--->"
  "<text>get the periods value of the hour.</text--->"
  "<text>thehour(2002, 2, 28, 8)</text--->"
  ") )";

const string TemporalSpecTheMinute =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  "<text>int x int x int x int x int -> periods</text--->"
  "<text>theminute( _ )</text--->"
  "<text>get the periods value of the minute.</text--->"
  "<text>theminute(2002, 3, 28, 8, 59)</text--->"
  ") )";

const string TemporalSpecTheSecond =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int x int x int x int x int x int -> periods</text--->"
  "<text>thesecond( _ )</text--->"
  "<text>get the periods value of the second.</text--->"
  "<text>thesecond(2002, 12, 31, 23, 59, 59)</text--->"
  ") )";

const string TemporalSpecThePeriod =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(periods periods) -> periods</text--->"
  "<text> theperiod( _, _ )</text--->"
  "<text>Create a period that spans the starting instant of the first,"
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
  "( <text>rect3 -> rect</text--->"
  "<text>box2d( _ )</text--->"
  "<text>returns the 2d part of a rect3. Can be used to eliminate the temporal"
  "dimension of a 3d bounding box.</text--->"
  "<text>box2d(r3)</text--->"
  ") )";

const string TemporalMBool2MIntSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mbool -> mint </text--->"
  "<text>mbool2mint( _ ) </text--->"
  "<text>converts the mbool value into a mint value"
  "</text--->"
  "<text>mbool2mint(mb1)</text--->"
  ") )";

const string TemporalExtDeftimeSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mT x uT -> mT with T in {bool, int}  </text--->"
  "<text>extenddeftime( _ _) </text--->"
  "<text>extends the moving object's deftime by that of the unit's"
  " filling all gaps with the value taken from the unit.</text--->"
  "<text>query extdeftime(mb ub)</text--->"
  ") )";

/*
16.4.3 Operators

*/
Operator temporalisempty( "isempty",
                          TemporalSpecIsEmpty,
                          8,//12
                          temporalisemptymap,
                          TemporalSimpleSelect,
                          TemporalTypeMapBool );

Operator temporalequal( "=",
                        TemporalSpecEQ,
                        4,
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
                           4,
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
                         5,
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

Operator temporaldistance( "distance",
                           TemporalSpecDistance,
                           MPointDistance,
                           Operator::SimpleSelect,
                           MovingBaseTypeMapMReal );

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

Operator temporalminimum( "minimum",
                           TemporalSpecMin,
                           2,
                           minmap,
                           MinMaxSelect,
                           TypeMapMinMax );

Operator temporalmaximum( "maximum",
                           TemporalSpecMax,
                           2,
                           maxmap,
                           MinMaxSelect,
                           TypeMapMinMax );

Operator temporalbreakpoints( "breakpoints",
                           TemporalSpecBreakPoints,
                           MPointBreakPoints,
                           Operator::SimpleSelect,
                           MovingTypeMapBreakPoints );

Operator temporalunits( "units",
                        TemporalSpecUnits,
                        5,
                        temporalunitsmap,
                        MovingSimpleSelect,
                        MovingTypeMapUnits );

Operator temporalbbox( "bbox",
                       TemporalSpecBBox,
                       4,
                       temporalbboxmap,
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
                         TemporalBox2d,
                         Operator::SimpleSelect,
                         Box2dTypeMap );

Operator mbool2mint( "mbool2mint",
                       TemporalMBool2MIntSpec,
                       MBool2MInt,
                       Operator::SimpleSelect,
                       TemporalMBool2MInt );

Operator extdeftime( "extdeftime",
                      TemporalExtDeftimeSpec,
                      2,
                      extdeftimemap,
                      ExtDeftimeSelect,
                      ExtDeftimeTypeMap );

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

    rangeint.AssociateKind( "RANGE" );
    rangeint.AssociateKind( "DATA" );
    rangereal.AssociateKind( "RANGE" );
    rangereal.AssociateKind( "DATA" );
    periods.AssociateKind( "RANGE" );
    periods.AssociateKind( "DATA" );

    intimebool.AssociateKind( "TEMPORAL" );
    intimebool.AssociateKind( "DATA" );
    intimeint.AssociateKind( "TEMPORAL" );
    intimeint.AssociateKind( "DATA" );
    intimereal.AssociateKind( "TEMPORAL" );
    intimereal.AssociateKind( "DATA" );
    intimepoint.AssociateKind( "TEMPORAL" );
    intimepoint.AssociateKind( "DATA" );

    unitbool.AssociateKind( "TEMPORAL" );
    unitbool.AssociateKind( "DATA" );
    unitint.AssociateKind( "TEMPORAL" );
    unitint.AssociateKind( "DATA" );
    unitreal.AssociateKind( "TEMPORAL" );
    unitreal.AssociateKind( "DATA" );
    unitpoint.AssociateKind( "TEMPORAL" );
    unitpoint.AssociateKind( "DATA" );
    unitpoint.AssociateKind( "SPATIAL3D" );

    movingbool.AssociateKind( "TEMPORAL" );
    movingbool.AssociateKind( "DATA" );
    movingint.AssociateKind( "TEMPORAL" );
    movingint.AssociateKind( "DATA" );
    movingreal.AssociateKind( "TEMPORAL" );
    movingreal.AssociateKind( "DATA" );
    movingpoint.AssociateKind( "TEMPORAL" );
    movingpoint.AssociateKind( "DATA" );

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
    AddOperator( &temporaldeftime );
    AddOperator( &temporaltrajectory );
    AddOperator( &temporalpresent );
    AddOperator( &temporalpasses );
    AddOperator( &temporalinitial );
    AddOperator( &temporalfinal );
    AddOperator( &temporalunits );
    AddOperator( &temporalbbox );

    AddOperator( &temporalat );
    AddOperator( &temporaldistance );
    AddOperator( &temporalsimplify );
    AddOperator( &temporalintegrate );
    AddOperator( &temporalminimum );
    AddOperator( &temporalmaximum );
    AddOperator( &temporalbreakpoints );
    AddOperator( &temporaltranslate );

    AddOperator( &temporaltheyear );
    AddOperator( &temporalthemonth );
    AddOperator( &temporaltheday );
    AddOperator( &temporalthehour );
    AddOperator( &temporaltheminute );
    AddOperator( &temporalthesecond );
    AddOperator( &temporaltheperiod );

    AddOperator(&temporalbox3d);
    AddOperator(&temporalbox2d);
    AddOperator(&mbool2mint);
    AddOperator(&extdeftime);

  }
  ~TemporalAlgebra() {};
};

TemporalAlgebra temporalAlgebra;

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
  return (&temporalAlgebra);
}


