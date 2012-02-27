/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of
Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
 by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston,
MA  02111-1307  USA
----

TemporalLiftedAlgebra

Juni 2006: Original implementation by J[ue]rgen Schmidt

December 2006: Christian D[ue]ntgen added
operator ~intersection: mpoint x mpoint [->] mpoint~.

December 2006: Christian D[ue]ntgen corrected code for ~SolvePoly(...)~
and separated the solver methods to its own header file called
~PolySolver.h~ within the SECONDO ~include~ directory.

September 2009 Simone Jandt: Changed TU\_VM\_ComparePredicateValue\_UReal 
to use new member function ~CompUReal~ of ~UReal~.

1 Overview

This implementation offers a subset of operators of the ~TemporalAlgebra~,
For this it leans on Lema, Forlizzi, G[ue]ting, Nardelli, Schneider:
~Algorithms for Moving Objects Databases~, the Computer Journal, 2003.

1 Includes

*/


#include <cmath>
#include <limits>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "PolySolver.h"

#include "DateTime.h"
using namespace datetime;

#include "SpatialAlgebra.h"
#include "TemporalAlgebra.h"
#include "MovingRegionAlgebra.h"
#include "TemporalExtAlgebra.h"


extern NestedList* nl;
extern QueryProcessor* qp;

/*
Set ~TLA\_DEBUG~ to ~true~ for debug output. Please note that debug output is
very verbose and has significant negative input on the algebra's performance.
Only enable debug output if you know what you are doing!

*/
const bool TLA_DEBUG = false;
//const bool TLA_DEBUG = true;

/*
1 Class template ~RefinementPartitionLift~

This implementation was renamed ~RefinementPartition~ 
and moved to ~TemporalAlgebra.h~.

1 Methods used by the ValueMapping-Functions

1.1 Method ~MPerimeter()~

calculates the perimeter of a MRegion and returns the MReal value as result.

*/
void MPerimeter(MRegion& reg, MReal& res) {
    if(TLA_DEBUG)
      cout<< "MPerimeter() called" << endl;
    res.Clear();
    if( !reg.IsDefined() ){
      res.SetDefined( false );
      return;
    }
    res.SetDefined( true );
    int nocomponents = reg.GetNoComponents();
    res.Resize(nocomponents);

    res.StartBulkLoad();
    for(int n = 0; n < nocomponents; n++){
      URegionEmb ur;
      UReal ures(true);
      double start = 0.0, end = 0.0;
      reg.Get(n, ur);
      if(!ur.IsValid())
        continue;
      if(TLA_DEBUG){
        cout<<"URegion # "<<n<<" "<<"[ "<<ur.timeInterval.start.ToString()
        <<" "<<ur.timeInterval.end.ToString()<<" ]";}
      ures.timeInterval = ur.timeInterval;
      int number = ur.GetSegmentsNum();
      for(int i = 0; i < number; i++){
        MSegmentData dms;
        ur.GetSegment(reg.GetMSegmentData(), i, dms);
        if(dms.GetCycleNo() == 0){ //only outercycles
          start += sqrt(pow(dms.GetInitialStartX() - dms.GetInitialEndX(), 2)
                 + pow(dms.GetInitialStartY() - dms.GetInitialEndY(), 2));
          end +=   sqrt(pow(dms.GetFinalStartX() - dms.GetFinalEndX(), 2)
                 + pow(dms.GetFinalStartY() - dms.GetFinalEndY(), 2));
        }
      }
      ures.a = 0.0;
      ures.b = (ures.timeInterval.end > ures.timeInterval.start)
               ? (end - start) / (ures.timeInterval.end.ToDouble()
               - ures.timeInterval.start.ToDouble()) : 0.0;
      ures.c = start;
      ures.r = false;
      if(TLA_DEBUG)
        cout<<" ends with: a "<<ures.a<<", b "<<ures.b<<", c "
        <<ures.c<<", r "<<ures.r<<endl;
      res.MergeAdd(ures);
    }
    res.EndBulkLoad(false);
}

/*
1.1 Method ~MArea()~

calculates the area of a MRegion and returns the MReal value as result.

*/
void MArea(MRegion& reg, MReal& res) {
    if(TLA_DEBUG)
      cout<< "MArea() called" << endl;

    res.Clear();
    if( !reg.IsDefined() ){
      res.SetDefined( false );
      return;
    }
    res.SetDefined( true );
    int size = reg.GetNoComponents();
    res.Resize(size);
    res.StartBulkLoad();
    for(int n = 0; n < size; n++){
      URegionEmb ur;
      UReal ures(true);
      double at = 0.0, bt = 0.0, ct = 0.0;
      reg.Get(n, ur);
      if(!ur.IsValid())
        continue;
      if(TLA_DEBUG){
        cout<<"URegion # "<<n<<" "<<"[ "<<ur.timeInterval.start.ToString()
        <<" "<<ur.timeInterval.end.ToString()<<" ]";}
      double dt = ur.timeInterval.end.ToDouble()
                - ur.timeInterval.start.ToDouble();
      if (dt == 0.0) continue;
      ures.timeInterval = ur.timeInterval;

      int number = ur.GetSegmentsNum();
      for(int i = 0; i < number; i++){
        MSegmentData dms;
        ur.GetSegment(reg.GetMSegmentData(), i, dms);
        double kx1 = (dms.GetFinalStartX() - dms.GetInitialStartX()) / dt;
        double kx2 = (dms.GetFinalEndX()   - dms.GetInitialEndX())   / dt;
        double ky1 = (dms.GetFinalStartY() - dms.GetInitialStartY()) / dt;
        double ky2 = (dms.GetFinalEndY()   - dms.GetInitialEndY())   / dt;

        at += ((kx2 - kx1) * (ky1 + ky2)) / 2;
        bt += (((kx2 - kx1) * (dms.GetInitialStartY()
              + dms.GetInitialEndY()))  + ((dms.GetInitialEndX()
              - dms.GetInitialStartX()) * (ky1 + ky2))) / 2;
        ct += ((dms.GetInitialStartY()  + dms.GetInitialEndY())
             * (dms.GetInitialEndX()    - dms.GetInitialStartX())) / 2;
      }
      ures.a = at;
      ures.b = bt;
      if ( ct >= 0 ) ures.c = ct;
      else ures.c = -ct;
      ures.r = false;
      if(TLA_DEBUG){
        cout<<" ends with: a "<<ures.a<<", b "<<ures.b<<", c "
        <<ures.c<<", r "<<ures.r<<endl;}
      res.MergeAdd(ures);
    }
    res.EndBulkLoad(false);
}

/*
1.1 Method ~RCenter()~

calculates the rought center of a MRegion and returns the MPoint value as
result. The rought center meens the MPoint that goes through the centers of
the static regions at the starts and  ends of the timeintervals of the
MRegion. These points are bound with linear lines.

*/
void RCenter(MRegion& reg, MPoint& res) {
    if(TLA_DEBUG)
      cout<< "RCenter() called" << endl;

    res.Clear();
    if( !reg.IsDefined() ){
      res.SetDefined( false );
      return;
    }
    res.SetDefined( true );
    int size = reg.GetNoComponents();
    res.Resize(size);
    res.StartBulkLoad();
    for(int n = 0; n < size; n++){
      URegionEmb ur;
      double Ainitial = 0.0, Axinitial = 0.0, Ayinitial = 0.0,
             Afinal = 0.0, Axfinal = 0.0, Ayfinal = 0.0;
      reg.Get(n, ur);
      if(!ur.IsValid())
        continue;
      if(TLA_DEBUG){
        cout<<"URegion # "<<n<<" "<<"[ "<<ur.timeInterval.start.ToString()
        <<" "<<ur.timeInterval.end.ToString()<<" ]";}

      int number = ur.GetSegmentsNum();
      MSegmentData dms;
      for(int i = 0; i < number; i++){
        ur.GetSegment(reg.GetMSegmentData(), i, dms);

       //Calculate Area of Beginning and End of Unit
       Ainitial += (dms.GetInitialEndX()
                  - dms.GetInitialStartX()) * (dms.GetInitialEndY()
                  + dms.GetInitialStartY()) / 2;
       Afinal   += (dms.GetFinalEndX() - dms.GetFinalStartX())
                 * (dms.GetFinalEndY() + dms.GetFinalStartY()) / 2;

       double initialax, initialbx, finalax, finalbx, initialay,
       initialby, finalay, finalby; //Ax=ax^3+bx^2

       //Calculate momentums of Area
       initialax = (dms.GetInitialStartX() != dms.GetInitialEndX())
                 ? ((dms.GetInitialEndY()   - dms.GetInitialStartY()) / (
                 dms.GetInitialEndX() - dms.GetInitialStartX()) / 3.0) : 0.0;
       initialbx = (dms.GetInitialStartY() - 3.0 * initialax
                 * dms.GetInitialStartX()) / 2.0;
       finalax = (dms.GetFinalStartX() != dms.GetFinalEndX())
                 ? ((dms.GetFinalEndY() - dms.GetFinalStartY()) / (
                 dms.GetFinalEndX() - dms.GetFinalStartX()) / 3.0) : 0.0;
       finalbx = (dms.GetFinalStartY() - 3.0 * finalax
                 * dms.GetFinalStartX()) / 2.0;

       initialay = (dms.GetInitialStartY() != dms.GetInitialEndY())
                 ? ((dms.GetInitialEndX()   - dms.GetInitialStartX()) / (
                 dms.GetInitialEndY() - dms.GetInitialStartY()) / 3.0) : 0.0;
       initialby = (dms.GetInitialStartX() - 3.0 * initialay
                 * dms.GetInitialStartY()) / 2.0;
       finalay = (dms.GetFinalStartY() != dms.GetFinalEndY())
                 ? ((dms.GetFinalEndX() - dms.GetFinalStartX()) / (
                 dms.GetFinalEndY() - dms.GetFinalStartY()) / 3.0) : 0.0;
       finalby = (dms.GetFinalStartX() - 3.0 * finalay
                 * dms.GetFinalStartY()) / 2.0;

       Axinitial += initialax * pow(dms.GetInitialEndX(), 3)
                  + initialbx * pow(dms.GetInitialEndX(), 2)
                  - initialax * pow(dms.GetInitialStartX(), 3)
                  - initialbx * pow(dms.GetInitialStartX(), 2);

       Axfinal     += finalax * pow(dms.GetFinalEndX(), 3)
                    + finalbx * pow(dms.GetFinalEndX(), 2)
                    - finalax * pow(dms.GetFinalStartX(), 3) - finalbx
                    * pow(dms.GetFinalStartX(),2 );

       Ayinitial += initialay * pow(dms.GetInitialEndY(), 3)
                  + initialby * pow(dms.GetInitialEndY(), 2)
                  - initialay * pow(dms.GetInitialStartY(), 3)
                  - initialby * pow(dms.GetInitialStartY(),2 );

       Ayfinal     += finalay * pow(dms.GetFinalEndY(), 3)
                    + finalby * pow(dms.GetFinalEndY(), 2)
                    - finalay * pow(dms.GetFinalStartY(), 3) - finalby
                    * pow(dms.GetFinalStartY(),2 );
      }
      if ((Ainitial != 0.0) || (Afinal != 0.0)){
        UPoint *ures;
        if((Ainitial != 0.0) && (Afinal != 0.0)) {
          ures = new UPoint(ur.timeInterval, (Axinitial / Ainitial),
             (-Ayinitial / Ainitial), (Axfinal / Afinal), (-Ayfinal / Afinal));
        }
        else if (Ainitial == 0.0) {
          ures = new UPoint(ur.timeInterval, dms.GetInitialStartX(),
             dms.GetInitialStartY(), (Axfinal / Afinal), (-Ayfinal / Afinal));
        }
        else {
          ures = new UPoint(ur.timeInterval, (Axinitial / Ainitial),
             (-Ayinitial / Ainitial), dms.GetFinalStartX(),
             dms.GetFinalStartY());
        }
        if(TLA_DEBUG){
          cout<<" ends with"<<endl;
          cout<<"Xinitial "<<ures->p0.GetX()<<endl;
          cout<<"Yinitial "<<ures->p0.GetY()<<endl;
          cout<<"Xfinal   "<<ures->p1.GetX()<<endl;
          cout<<"Yfinal   "<<ures->p1.GetY()<<endl;}
        res.Add(*ures);
        delete ures;
      }
    }
    res.EndBulkLoad(false);
}

/*
1.1 Method ~NComponents()~

It gets for each URegion the last segment and gives the FaceNo of this segment,
because the last segment is on the last face .

*/
void NComponents(MRegion& reg, MInt& res) {
    if(TLA_DEBUG)
      cout<< "NComponents() called" << endl;

    res.Clear();
    if( !reg.IsDefined() ){
      res.SetDefined( false );
      return;
    }
    res.SetDefined( true );
    int size = reg.GetNoComponents();
    res.Resize(size);
    res.StartBulkLoad();
    for(int n = 0; n < size; n++){
      URegionEmb ur;
      reg.Get(n, ur);
      if(!ur.IsValid())
        continue;
      if(TLA_DEBUG){
        cout<<"URegion # "<<n<<" "<<"[ "<<ur.timeInterval.start.ToString()
        <<" "<<ur.timeInterval.end.ToString()<<" ]";}
      MSegmentData dms;
      ur.GetSegment(reg.GetMSegmentData(),ur.GetSegmentsNum() - 1, dms);
      CcInt *constVal = new CcInt(true, dms.GetFaceNo() + 1);
      UInt *ures = new UInt(ur.timeInterval, *constVal);

      if(TLA_DEBUG)
        cout<<" ends with "<<ures->constValue.GetIntval()<<endl;
      res.MergeAdd(*ures);
      delete constVal;
      delete ures;
    }
    res.EndBulkLoad(false);
}

/*
1.1 Method ~CompareValue~

Returns true if the value of this TemporalUnit holds the
comparison holds with the value of the TemporalUnit i.
The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/
template <class Alpha>
bool CompareValue(  const ConstTemporalUnit<Alpha>& n,
                    const ConstTemporalUnit<Alpha>& i, int vers )
{
  if(TLA_DEBUG)
    cout<<"ConstTemporalUnit<Alpha> CompareValue "<<vers<<endl;
  assert( n.IsDefined() );
  assert( i.IsDefined() );
   if (vers == -3)  //#
     return (n.constValue.Compare( &i.constValue ) != 0);
   if (vers == -2)  //<
     return (n.constValue.Compare( &i.constValue ) <  0);
   if (vers == -1)  //<=
     return (n.constValue.Compare( &i.constValue ) <= 0);
   if (vers == 0)  //Equal
     return (n.constValue.Compare( &i.constValue ) == 0);
   if (vers == 1)  //>=
     return (n.constValue.Compare( &i.constValue ) >= 0);
   if (vers == 2)  //<
     return (n.constValue.Compare( &i.constValue ) >  0);
   //should not be reached
   return false;
}

/*
1.1 Method ~CompareValue~

Returns true if the value of this TemporalUnit holds the
comparison holds with the value of i.
The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/
template <class Alpha>
bool CompareValue(const ConstTemporalUnit<Alpha>& n, const Alpha& i, int vers )
{
   if(TLA_DEBUG)
     cout<<"ConstTemporalUnit<Alpha> CompareValue "<<vers<<endl;
   assert( n.IsDefined() );
   if (vers == -3)  //#
     return (n.constValue.Compare( &i ) != 0);
   if (vers == -2)  //<
     return (n.constValue.Compare( &i ) <  0);
   if (vers == -1)  //<=
     return (n.constValue.Compare( &i ) <= 0);
   if (vers == 0)  //Equal
     return (n.constValue.Compare( &i ) == 0);
   if (vers == 1)  //>=
     return (n.constValue.Compare( &i ) >= 0);
   if (vers == 2)  //<
     return (n.constValue.Compare( &i ) >  0);
   //should not be reached
   return false;
}

/*
1.1 Method ~DistanceMPoint~

Returns the distance between two MPoints as MReal

*/
void DistanceMPoint( const MPoint& p1, const MPoint& p2, MReal& result)
{
  if(TLA_DEBUG)
    cout<<"DistanceMPoint called"<<endl;
  result.Clear();
  if( !p1.IsDefined() || !p2.IsDefined() ){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  UReal uReal(true);

  RefinementPartition<MPoint, MPoint, UPoint, UPoint> rp(p1, p2);
  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  result.Resize(rp.Size());
  result.StartBulkLoad();
  for( unsigned int i = 0; i < rp.Size(); i++ )
  {
    Interval<Instant> iv;
    int u1Pos, u2Pos;
    UPoint u1;
    UPoint u2;

    rp.Get(i, iv, u1Pos, u2Pos);
    if(TLA_DEBUG){
      cout<< "Compare interval #"<< i<< ": ["<< iv.start.ToString()<< " "
      << iv.end.ToString()<< " "<< iv.lc<< " " << iv.rc<< "] "
      << u1Pos<< " "<< u2Pos<< endl;}

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<endl;
      p1.Get(u1Pos, u1);
      p2.Get(u2Pos, u2);
    }
    if(u1.IsDefined() && u2.IsDefined())
    { // do not need to test for overlapping deftimes anymore...
      u1.Distance( u2, uReal );
      result.MergeAdd( uReal );
    }
  }
  result.EndBulkLoad();
}

/*
1.1 Method ~FindEqualTimes4Real~

Function FindEqualTimes4Real to find all times where u1 and u2 are
equal, gives them back in t and returns their number. u1 and u2 must
have the same timeIntervals (use ShiftUReal to get this)

*/
int FindEqualTimes4Real(const UReal& u1, const UReal& u2, Instant t[4]){
    assert(u1.IsDefined() );
    assert(u2.IsDefined() );
    int number;
    double sol2[2];
    double sol4[4];
    if(TLA_DEBUG){
     cout<<"FindEqualTimes4Real called with"<<endl;
     cout<<"u1.a "<<u1.a<<" u1.b "<<u1.b<<" u1.c "<<u1.c<<" u1.r "<<u1.r<<endl;
     cout<<"u2.a "<<u2.a<<" u2.b "<<u2.b<<" u2.c "<<u2.c<<" u2.r "<<u2.r<<endl;}
    if (u1.r == u2.r) {
      if(TLA_DEBUG)
        cout<<"u1.r == u2.r"<<endl;

      number = SolvePoly(u1.a - u2.a, u1.b - u2.b, u1.c - u2.c, sol2, true);
      for (int m = 0; m < number; m++)
        t[m].ReadFrom(sol2[m] + u1.timeInterval.start.ToDouble());
    }
    else {
     if(TLA_DEBUG)
       cout<<"u1.r != u2.r"<<endl;
     if (u2.r && u2.a == 0 && u2.b == 0) {
        if(TLA_DEBUG)
          cout<<"Spezial case u2 = const"<<endl;
        number = SolvePoly(u1.a, u1.b, u1.c - sqrt(u2.c), sol2, true);
        for (int m = 0; m < number; m++)
           t[m].ReadFrom(sol2[m] + u1.timeInterval.start.ToDouble());
     }
     else if (u1.r && u1.a == 0 && u1.b == 0) {
        if(TLA_DEBUG)
          cout<<"Spezial case u1 = const"<<endl;
        number = SolvePoly(u2.a, u2.b, u2.c - sqrt(u1.c), sol2, true);
        for (int m = 0; m < number; m++)
           t[m].ReadFrom(sol2[m] + u1.timeInterval.start.ToDouble());
     }
     else{ // solve the squared equation
      double v, w, x, y, z;
      if (u2.r) {
        v = pow(u1.a, 2);                         //x^4
        w = 2 * u1.a * u1.b;                      //x^3
        x = 2 * u1.a * u1.c + pow(u1.b, 2)- u2.a; //x^2
        y = 2 * u1.b * u1.c - u2.b;               //x
        z = pow(u1.c, 2) - u2.c;                  //c
      }
      else {
        v = pow(u2.a, 2);                         //x^4
        w = 2 * u2.a * u2.b;                      //x^3
        x = 2 * u2.a * u2.c + pow(u2.b, 2)- u1.a; //x^2
        y = 2 * u2.b * u2.c - u1.b;               //x
        z = pow(u2.c, 2) - u1.c;                  //c
      }
      //va^4+wa^3+xa^2+ya+z=0

      if(TLA_DEBUG)
        cout<<"v: "<<v<<", w: "<<w<<", x:"<<x<<", y:"<<y<<", z:"<<z<<endl;
      number = SolvePoly(v, w, x, y, z, sol4);
      for (int n = 0; n < number; n++){
        double val1 = u1.a * pow(sol4[n],2) + u1.b * sol4[n] + u1.c;
        if(u1.r)
          val1 = sqrt(val1);
        double val2 = u2.a * pow(sol4[n],2) + u2.b * sol4[n] + u2.c;
        if(u2.r)
          val2 = sqrt(val2);
        if(TLA_DEBUG)
          cout<<n<<". at "<<sol4[n]<<endl<<"val1 "<<val1<<", val2 "<<val2<<endl;
        if(!AlmostEqual(val1, val2)){
          if(TLA_DEBUG)
            cout<<"false Point -> remove"<<endl;
          for (int i = n; i < number; i++)
            sol4[i] = sol4 [i + 1];
          number--;
          n--;
        }
      }
      for (int m = 0; m < number; m++)
         t[m].ReadFrom(sol4[m] + u1.timeInterval.start.ToDouble());
     }
    }
    if(TLA_DEBUG)
      cout<<"FindEqualTimes4Real ends with "<<number<<"solutions"<<endl;
  return number;
}

/*
1.1 Method ~CompareUReal~

Returns true iff for the temporal functions of the two uReals as such the 
comparison holds during the
definition time of uBool.
The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

\#: u1 is somewhere != u1 within uBool.timeInterval
<:  u1 is always < u2 during uBool.timeInterval
<=: u1 is always <= u2 during uBool.timeInterval
=:  u1 and u2 are equal during all uBool.timeInterval
>=: u1 is always >= u2 during uBool.timeInterval
>:  u1 is always > u2 during uBool.timeInterval

*/
static void CompareUReal(const UReal& u1, const UReal& u2, UBool& uBool, int op)
{
    assert( u1.IsDefined() );
    assert( u1.IsValid() );
    assert( u2.IsDefined() );
    assert( u2.IsValid() );
    assert( uBool.IsDefined() );
    assert( uBool.IsValid() );
    if(TLA_DEBUG){
      cout<<"CompareUReal "<<op<<" in ["<<uBool.timeInterval.start.ToString()
      <<" "<<uBool.timeInterval.end.ToString()<<" "<<uBool.timeInterval.lc
      <<" "<<uBool.timeInterval.rc<<"]"<<endl;}

    DateTime start = uBool.timeInterval.start;
    DateTime end   = uBool.timeInterval.end;
    DateTime mid   = start + ((end - start)/2);
    CcReal value1start, value2start, value1mid, value2mid, value1end, value2end;
    u1.TemporalFunction(start, value1start, true);
    u1.TemporalFunction(mid,   value1mid,   true);
    u1.TemporalFunction(end,   value1end,   true);
    u2.TemporalFunction(start, value2start, true);
    u2.TemporalFunction(mid,   value2mid,   true);
    u2.TemporalFunction(end,   value2end,   true);
    if(    AlmostEqual(value1start.GetRealval(), value2start.GetRealval() )
        && AlmostEqual(value1mid.GetRealval(),   value2mid.GetRealval()   )
        && AlmostEqual(value1end.GetRealval(),   value2end.GetRealval()   ) ) {
      if (op >= -1 && op <= 1) uBool.constValue.Set(true,true); // all equal
      else uBool.constValue.Set(true,false);
    }
    else if(    (value1start.Compare(&value2start) == -1)     // all u1
             && (value1mid.Compare(&value2start) == -1)       // are
             && (value1start.Compare(&value2start) == -1)) {  // smaller
      if (op < 0) uBool.constValue.Set(true,true);
      else uBool.constValue.Set(true,false);
    }
    else if(    (value1start.Compare(&value2start) == 1)         // all u1
             && (value1mid.Compare(&value2mid) == 1)             // are
             && (value1end.Compare(&value2end) == 1) ){          // larger
      if (op > 0 || op == -3) uBool.constValue.Set(true,true);
      else uBool.constValue.Set(true,false);
    }
    else {
      if (op == -3) uBool.constValue.Set(true,true);
      else uBool.constValue.Set(true,false);
    }
  if(TLA_DEBUG)
    cout<<"return with "<<uBool.constValue.GetBoolval()<<endl;
}

/*
1.1 Method ~ShiftUReal~

Transforms a UReal to newstart. Result of local definition of MReals.

*/
static void ShiftUReal(UReal& op, const Instant& newstart)
{
  double offset = newstart.ToDouble() - op.timeInterval.start.ToDouble();
  if(TLA_DEBUG){
    cout<<"ShiftUReal called with a: "<<op.a<<", b: "<<op.b <<", c: "<<op.c
    <<", r: "<<op.r<<" old start: "<<op.timeInterval.start.ToString()
    <<", newstart: "<<newstart.ToString()<<endl;
    cout<<"offset: "<<offset<<endl;}
  op.timeInterval.start = newstart;
  op.c = op.a * pow(offset, 2) + op.b * offset + op.c;
  op.b = 2 * op.a * offset + op.b;
  if(TLA_DEBUG){
    cout<<"ShiftUReal ends with a: "<<op.a<<", b: "<<op.b<<", c: "<<op.c
    <<", r: "<<op.r<<" start: "<<op.timeInterval.start.ToString()<<endl;}
}

/*
1.1 Method ~MRealDistanceMM~

Calculates the Distance between the given MReals with respect of the fact,
that the distance can not be negativ.

*/
static void MRealDistanceMM(const MReal& op1, const MReal& op2, MReal& result)
{
  if(TLA_DEBUG)
    cout<<"MRealDistanceMM called"<<endl;
  result.Clear();
  if( !op1.IsDefined() || !op2.IsDefined() ){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  UReal uReal(true);

  RefinementPartition<MReal, MReal, UReal, UReal> rp(op1, op2);
  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  result.Resize(rp.Size());
  result.StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    UReal u1transfer;
    UReal u2transfer;
    UReal u1(true);
    UReal u2(true);
    int numPartRes = 0;
    vector<UReal> partResVector;
    bool resultIsValid = true;

    rp.Get(i, iv, u1Pos, u2Pos);
    if (u1Pos == -1 || u2Pos == -1)
      continue;
    if(TLA_DEBUG){
      cout<<"Both operators existant in interval iv #"<<i<<" ["
      <<iv.start.ToString()<<"  "<<iv.end.ToString()
      <<" "<<iv.lc<<" "<<iv.rc<<"]"<<endl;}
    op1.Get(u1Pos, u1transfer);
    op2.Get(u2Pos, u2transfer);
    if(!(u1transfer.IsDefined() && u2transfer.IsDefined()))
    {
      resultIsValid = false;
      break;
    }
    u1 = u1transfer;
    u2 = u2transfer;
    if ( u1.r || u2.r )
    {
      resultIsValid = false;
      break;
    }

    ShiftUReal(u1, iv.start);
    ShiftUReal(u2, iv.start);
    u1.timeInterval = iv;
    u2.timeInterval = iv;

    numPartRes = u1.Distance(u2, partResVector);
    for(int j=0; j<numPartRes; j++){
      result.MergeAdd(partResVector[j]);
    }
  }
  result.EndBulkLoad(false); // should already be sorted
}
/*
1.1 Method ~MovingRealCompareMM2~

Returns true if the value of these two mReals holds the comparison.
The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

Implemetation changed because of missing implementation parts. Now we use
new UReal::CompUReal -Member function for computing.

The opcodes have to be mapped to the enumeration of that memberfunction,
which uses different opcode
opcode == 0 =
opcode == 1 \#
opcode == 2 <
opcode == 3 >
opcode == 4 <=
opcode == 5 >=

Because the function is called from many other parts we have to map the opcodes
inside the function instead of changing all calling parts.

*/
static void MovingRealCompareMM2(const MReal& op1, const MReal& op2, MBool&
    result, int op)
{
  result.Clear();
  if( !op1.IsDefined() || !op2.IsDefined() ){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  int opcode = 0;
  switch(op)
  {
    case -3:
      opcode = 1;
      break;
    case -2:
      opcode = 2;
      break;
    case -1:
      opcode = 4;
      break;
    case 0:
      opcode = 0;
      break;
    case 1:
      opcode = 5;
      break;
    case 2:
      opcode = 3;
      break;
    default: break; //should never been reached
  }
  RefinementPartition<MReal, MReal, UReal, UReal> rp(op1, op2);
  result.Resize(rp.Size());
  result.StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    vector<UBool> uv;
    uv.clear();
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    rp.Get(i, iv, u1Pos, u2Pos);
    if (u1Pos == -1 || u2Pos == -1)
      continue;
    UReal u1;
    UReal u2;
    op1.Get(u1Pos, u1);
    op2.Get(u2Pos, u2);
    if(!(u1.IsDefined() || !u2.IsDefined())) {
      cerr << __PRETTY_FUNCTION__
           << " encountered undefined unit within a mapping:" << endl;

      continue;
    }
    u1.CompUReal(u2, opcode, uv);
    for (size_t i = 0;i < uv.size();i++)
      result.MergeAdd(uv[i]);
    uv.clear();
  }
  result.EndBulkLoad(false);
}
    /*
static void MovingRealCompareMM2(MReal& op1, MReal& op2, MBool&
 result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingRealCompareMM called"<<endl;
  UBool uBool(true);
  //cout << "was in new method " << op << endl;
  RefinementPartition<MReal, MReal, UReal, UReal> rp(op1, op2);
  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;
  //cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;
  result.Clear();
  result.Resize(rp.Size());
  result.StartBulkLoad();

  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    const UReal *u1transfer;
    const UReal *u2transfer;
    UReal u1(true);
    UReal u2(true);

    rp.Get(i, iv, u1Pos, u2Pos);
    if (u1Pos == -1 || u2Pos == -1)
      continue;
    if(TLA_DEBUG)
      cout<<"Both operators existant in interval iv # "<<i<< " ["
      <<iv.start.ToString()<<" "<<iv.end.ToString()<<" "<<iv.lc<< " "
      <<iv.rc<<"] "<<u1Pos<< " "<<u2Pos<<"  op "<<op<<endl;
    op1.Get(u1Pos, u1transfer);
    op2.Get(u2Pos, u2transfer);
    if(!(u1transfer->IsDefined() && u2transfer->IsDefined()))
        continue;
    u1 = *u1transfer;
    u2 = *u2transfer;
    ShiftUReal(u1, iv.start);
    ShiftUReal(u2, iv.tart);
    u1.timeInterval = iv;
    u2.timeInterval = iv;
    //new
    Periods p(2);
    UBool uBool(true);
    CcReal u1real, u2real;
    Interval<Instant> ivt;
    //cout << u1.a << " " << u1.b << " " << u1.c << endl;
    //cout << u2.a << " " << u2.b << " " << u2.c << endl;
    //if ( (u1.a == u2.a) && (u1.b == u2.b) && (u1.c == u2.c) )
    if ( u1 == u2 )
    {
      if(TLA_DEBUG)
        cout << "equalureal" << endl;
      uBool.timeInterval.start = iv.start;
      uBool.timeInterval.end = iv.end;
      uBool.timeInterval.lc = iv.lc;
      uBool.timeInterval.rc = iv.rc;
      if ( (op == 0) || (op == -1) ||(op == 1) )
        uBool.constValue.Set(true, true);
      else
        uBool.constValue.Set(true, false);
      result.MergeAdd(uBool);
    }
    else
    {
      int number2 = u1.PeriodsAtEqual( u2, p);
      //cout << "number2= " << number2 << endl;
      //cout << "iv.lc= " << iv.lc << endl;
      //cout << "iv.rc= " << iv.rc << endl;
      //const Interval<Instant>* iv2;
      //p.Get(0, iv2);
      //cout << "iv2end " << iv2->end.ToString() << endl;
      //cout << "iv2start " << iv2->start.ToString() << endl;
      //cout << "ivend " << iv.end.ToString() << endl;
      //cout << "ivstart " << iv.start.ToString() << endl;

      uBool.timeInterval.start = iv.start;
      uBool.timeInterval.lc = iv.lc;
      if ( number2 > 0 )
      {
        p.Get(0, ivt);
        //cout << "number > 2 " << ivt.lc << endl;
        if ( !(((ivt.end == iv.end) && (iv.rc == false)) ||
           (    (ivt.end == iv.start) && (iv.lc == true))) )
        {
          //cout << "number > 2 (inner)" << endl;
          uBool.timeInterval.end = ivt.start;
          uBool.timeInterval.rc = false;
          if ( op == 0 )
            uBool.constValue.Set(true, false);
          if (op == -3 )
            uBool.constValue.Set(true, true);
          if ( (op == -2) || (op == -1) )
          {
            u1.TemporalFunction(iv.start, u1real);
            u2.TemporalFunction(iv.start, u2real);
            if ( u1real.GetRealval() < u2real.GetRealval() )
              uBool.constValue.Set(true, true);
            else
              uBool.constValue.Set(true, false);
          }
          if ( (op == 2) || (op == 1) )
          {
            u1.TemporalFunction(iv.start, u1real);
            u2.TemporalFunction(iv.start, u2real);
            if ( u1real.GetRealval() > u2real.GetRealval() )
              uBool.constValue.Set(true, true);
            else
              uBool.constValue.Set(true, false);
          }
          result.MergeAdd(uBool);
          uBool.timeInterval.start = ivt.start;
          uBool.timeInterval.end = ivt.end;
          uBool.timeInterval.lc = true;
          uBool.timeInterval.rc = true;
          if ( (op == 0) || (op == -1) || (op == 1) )
            uBool.constValue.Set(true, true);
          if ( (op == -3) || (op == -2) || (op == 2) )
          uBool.constValue.Set(true, false);
          result.MergeAdd(uBool);
          if ( number2 > 1 )
          {
            //not yet implemented
            assert(true);
          }
          uBool.timeInterval.start = ivt.end;
          uBool.timeInterval.lc = false;
        }
      } //if number2 > 0
      uBool.timeInterval.end = iv.end;
      uBool.timeInterval.rc = iv.rc;
      if ( op == 0 )
        uBool.constValue.Set(true, false);
      if (op == -3 )
        uBool.constValue.Set(true, true);
      //vector<UReal> vureal;
      if ( (op == -2) || (op == -1) )
      {
        //u1.AtMin(vureal);
        //cout << "was i here ? " << endl;
        u1.TemporalFunction(iv.end, u1real, true);
        u2.TemporalFunction(iv.end, u2real, true);
        //cout << u1real.GetRealval() << " < " << u2real.GetRealval() << endl;
        if ( u1real.GetRealval() < u2real.GetRealval() )
        uBool.constValue.Set(true, true);
        else
          uBool.constValue.Set(true, false);
      }
      if ( (op == 2) || (op == 1) )
      {
        u1.TemporalFunction(iv.end, u1real, true);
        u2.TemporalFunction(iv.end, u2real, true);
        if ( u1real.GetRealval() > u2real.GetRealval() )
          uBool.constValue.Set(true, true);
        else
          uBool.constValue.Set(true, false);
      }
      result.MergeAdd(uBool);
      //cout << ivt.start.ToString() << endl;
      //cout << ivt.end.ToString() << endl;
    }
  }
  result.EndBulkLoad(false);
}

      */

/*
1.1 Method ~MovingRealCompareMM~

Returns true if the value of these two mReals holds the comparison.
The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/
/*static void MovingRealCompareMM(MReal& op1, MReal& op2, MBool&
 result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingRealCompareMM called"<<endl;
  UBool uBool(true);

  RefinementPartition<MReal, MReal, UReal, UReal> rp(op1, op2);
  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  result.Clear();
  result.StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    const UReal *u1transfer;
    const UReal *u2transfer;
    UReal u1(true);
    UReal u2(true);

    rp.Get(i, iv, u1Pos, u2Pos);
    if (u1Pos == -1 || u2Pos == -1)
      continue;
    if(TLA_DEBUG)
      cout<<"Both operators existant in interval iv # "<<i<< " ["
      <<iv.start.ToString()<<" "<<iv.end.ToString()<<" "<<iv.lc<< " "
      <<iv.rc<<"] "<<u1Pos<< " "<<u2Pos<<"  op "<<op<<endl;
    op1.Get(u1Pos, u1transfer);
    op2.Get(u2Pos, u2transfer);
    if(!(u1transfer->IsDefined() && u2transfer->IsDefined()))
        continue;
    u1 = *u1transfer;
    u2 = *u2transfer;
    ShiftUReal(u1, iv.start);
    ShiftUReal(u2, iv.start);
    u1.timeInterval = *iv;
    u2.timeInterval = *iv;
    Instant t[4];
    Instant middle;
    int counter = 0;

    int number = FindEqualTimes4Real(u1, u2, t);


    //new
    Periods p(2);
    UBool uBool2(true);
    CcReal u1real, u2real;
    const Interval<Instant>* ivt;
    int number2 = u1.PeriodsAtEqual( u2, p);
    uBool2.timeInterval.start = iv.start;
    uBool2.timeInterval.lc = true;
    if ( number2 > 0 )
    {
      p.Get(0, ivt);
      uBool2.timeInterval.end = ivt->start;
      uBool2.timeInterval.rc = false;
      if ( op == 0 )
        uBool2.constValue.Set(true, false);
      if (op == -3 )
        uBool2.constValue.Set(true, true);
      if ( (op == -2) || (op == -1) )
      {
        u1.TemporalFunction(iv.start, u1real);
        u2.TemporalFunction(iv.start, u2real);
        if ( u1real.GetRealval() < u2real.GetRealval() )
          uBool2.constValue.Set(true, true);
        else
          uBool2.constValue.Set(true, false);
      }
      if ( (op == 2) || (op == 1) )
      {
        u1.TemporalFunction(iv.start, u1real);
        u2.TemporalFunction(iv.start, u2real);
        if ( u1real.GetRealval() < u2real.GetRealval() )
          uBool2.constValue.Set(true, false);
        else
          uBool2.constValue.Set(true, true);
      }
      result.MergeAdd(uBool2);
      uBool2.timeInterval.start = ivt->start;
      uBool2.timeInterval.end = ivt->end;
      uBool2.timeInterval.lc = true;
      uBool2.timeInterval.rc = true;
      if ( (op == 0) || (op == -1) || (op == 1) )
        uBool2.constValue.Set(true, true);
      if ( (op == -3) || (op == -2) || (op == 2) )
        uBool2.constValue.Set(true, false);
      result.MergeAdd(uBool2);
      if ( number2 > 1 )
      {
      }
      uBool2.timeInterval.start = ivt->start;
      uBool2.timeInterval.lc = false;
    }
    uBool2.timeInterval.end = iv.end;
    uBool2.timeInterval.rc = true;
    if ( op == 0 )
      uBool2.constValue.Set(true, false);
    if (op == -3 )
      uBool2.constValue.Set(true, true);
    if ( (op == -2) || (op == -1) )
    {
      u1.TemporalFunction(iv.end, u1real);
      u2.TemporalFunction(iv.end, u2real);
      if ( u1real.GetRealval() < u2real.GetRealval() )
        uBool2.constValue.Set(true, true);
      else
        uBool2.constValue.Set(true, false);
    }
    if ( (op == 2) || (op == 1) )
    {
      u1.TemporalFunction(iv.end, u1real);
      u2.TemporalFunction(iv.end, u2real);
      if ( u1real.GetRealval() < u2real.GetRealval() )
        uBool2.constValue.Set(true, false);
      else
        uBool2.constValue.Set(true, true);
    }
    result.MergeAdd(uBool2);
    cout << ivt->start.ToString() << endl;
    cout << ivt->end.ToString() << endl;
    //


    for (int m = 0; m < number; m++) {
      t[m].SetType(instanttype);
      if ((*iv).Contains(t[m])) {
        if(TLA_DEBUG)
          cout<<m<<". crossing in iv"<<endl;
        t[counter] = t[m];
        counter += 1;
      }
      else {
        if(TLA_DEBUG)
          cout<<m<<". crossing not in iv"<<endl;
      }
    }
    uBool.timeInterval = *iv;
    if (counter == 0) {
      if(TLA_DEBUG)
        cout<<"no crossings in iv"<<endl;
      CompareUReal(u1, u2, uBool, op);
      result.MergeAdd(uBool);
    }
    else {
      if (iv.start < t[0]) {
        uBool.timeInterval.end = t[0];
        uBool.timeInterval.rc = false;
        CompareUReal(u1, u2, uBool, op);
        if(TLA_DEBUG)
          cout<<"1uBool "<<uBool.constValue.GetBoolval()<<endl;
        if (uBool.IsValid())
          result.MergeAdd(uBool);
      }
      for (int m = 0; m < counter; m++){
        if(TLA_DEBUG){
          cout<<m<<". crossing in iv: t["<<m<<"] "<<t[m].ToString()<<endl;}
        uBool.timeInterval.start = t[m];
        uBool.timeInterval.end = t[m];
        uBool.timeInterval.lc = true;
        uBool.timeInterval.rc = true;
        CompareUReal(u1, u2, uBool, op);
        if(TLA_DEBUG)
          cout<<"2uBool "<<uBool.constValue.GetBoolval()<<endl;
        if (uBool.IsValid())
          result.MergeAdd(uBool);
        uBool.timeInterval.lc = false;
        if (m < counter - 1){
          uBool.timeInterval.end = t[m + 1];
          uBool.timeInterval.rc = false;
        }
        else {
          uBool.timeInterval.end = iv.end;
          uBool.timeInterval.rc = iv.rc;
        }
        CompareUReal(u1, u2, uBool, op);
        if(TLA_DEBUG)
          cout<<"3uBool "<<uBool.constValue.GetBoolval()<<endl;
        if (uBool.IsValid())
          result.MergeAdd(uBool);
      }
    }
  }
  result.EndBulkLoad(false);
}*/

/*
1.1 Method ~MovingRealIntersectionMM~

Calculates the intersecion between two given mReals

*/
static void MovingRealIntersectionMM(const MReal& op1, const MReal& op2,
 MReal& result, int op)
{
 if(TLA_DEBUG)
   cout<<"MovingRealIntersectionMM called"<<endl;
 result.Clear();
 if( !op1.IsDefined() || !op2.IsDefined() ){
   result.SetDefined( false );
   return;
 }
 result.SetDefined( true );
 UReal un(true);

 RefinementPartition<MReal, MReal, UReal, UReal> rp(op1, op2);
 if(TLA_DEBUG)
   cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

 result.Resize(rp.Size());
 result.StartBulkLoad();
 for(unsigned int i = 0; i < rp.Size(); i++)
 {
   Interval<Instant> iv;
   int u1Pos;
   int u2Pos;
   UReal u1transfer;
   UReal u2transfer;
   UReal u1(true);
   UReal u2(true);

  rp.Get(i, iv, u1Pos, u2Pos);

  if (u1Pos == -1 || u2Pos == -1 )
      continue;

  else {
    if(TLA_DEBUG)
      cout<<"Both operators existant in interval iv #"<<i<<endl;

    op1.Get(u1Pos, u1transfer);
    op2.Get(u2Pos, u2transfer);
    if(!(u1transfer.IsDefined() && u2transfer.IsDefined()))
        continue;

    u1 = u1transfer;
    u2 = u2transfer;

    ShiftUReal(u1, iv.start);
    ShiftUReal(u2, iv.start);
    u1.timeInterval = iv;
    u2.timeInterval = iv;

    Instant t[4];
    Instant middle;
    int counter = 0;
    int number = FindEqualTimes4Real(u1, u2, t);

    for (int m = 0; m < number; m++) {
      t[m].SetType(instanttype);

      if ((iv).Contains(t[m])) {
        if(TLA_DEBUG)
          cout<<m<<". crossing in iv"<<endl;
        t[counter] = t[m];
        counter += 1;
      }
      else {
        if(TLA_DEBUG)
          cout<<m<<". crossing not in iv"<<endl;
      }
    }
    UBool uBool(true);
    uBool.timeInterval = iv;
    if (counter == 0) {
      if(TLA_DEBUG)
        cout<<"no crossings in iv"<<endl;
      CompareUReal(u1, u2, uBool, 0);

      if ((op == 1 && uBool.constValue.GetBoolval())
      || (op == 2 && !uBool.constValue.GetBoolval())){
        if(TLA_DEBUG)
          cout<<"just add interval ["<<un.timeInterval.start.ToString()
          <<" "<<un.timeInterval.end.ToString()<<" "<<un.timeInterval.lc<<" "
          <<un.timeInterval.rc<<"]"<<endl;
        un = u1;

        un.timeInterval = iv;  //to take borders
        result.MergeAdd(un);
      }
    }
    else {
      if (op == 1) {
        for (int m = 0; m < counter; m++){
          if ((t[m] > iv.start || iv.lc) && (t[m] < iv.end
          || iv.rc)){
            if(TLA_DEBUG)
              cout<<"add point"<<endl;
            un = u1;

            un.timeInterval.start = t[m];
            un.timeInterval.end = t[m];
            un.timeInterval.lc = true;
            un.timeInterval.rc = true;
            CcReal value;

            un.TemporalFunction(t[m], value, true);
            un.a = 0.0;
            un.b = 0.0;
            un.c = value.GetRealval();
            un.r = false;
            result.MergeAdd(un);

            if(TLA_DEBUG)
              cout<<"add "<<m<<". interval ["<<un.timeInterval.start.ToString()
              <<" "<<un.timeInterval.end.ToString()<<" "<<un.timeInterval.lc
              <<" "<<un.timeInterval.rc<<"]"<<endl;
          }
        }
      }
      else {
        if(TLA_DEBUG)
          cout<<"add interval"<<endl;
        if (t[0] > iv.start){
          un = u1;
          un.timeInterval = iv;
          un.timeInterval.end = t[0];
          un.timeInterval.rc = false;
          if(TLA_DEBUG)
            cout<<"add first interval ["<<un.timeInterval.start.ToString()
            <<" "<<un.timeInterval.end.ToString()<<" "<<un.timeInterval.lc
            <<" "<<un.timeInterval.rc<<"]"<<endl;

          if(un.IsValid())
            result.MergeAdd(un);
        }
        for (int m = 0; m < counter; m++){
          un = u1;
          ShiftUReal(un, t[m]);

          un.timeInterval.start = t[m];
          un.timeInterval.lc = false;
          if (m < counter - 1){
            un.timeInterval.end = t[m+1];
            un.timeInterval.rc = false;
          }
          else {
            un.timeInterval.end = iv.end;
            un.timeInterval.rc = iv.rc;
          }
          if(TLA_DEBUG)
            cout<<"add "<<m<<". interval ["<<un.timeInterval.start.ToString()
            <<" "<<un.timeInterval.end.ToString()<<" "<<un.timeInterval.lc<<" "
            <<un.timeInterval.rc<<"]"<<endl;
          if(un.IsValid())
            result.MergeAdd(un);
        }
      }
    }
   }
 }
 result.EndBulkLoad(false);
}

/*
1.1 Method ~MovingRealCompareMS~

Returns true if the value of the mReal and CcReal holds the comparison.
The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/
static void MovingRealCompareMS(const MReal& op1, const CcReal& op2, MBool&
 result, int op, bool ms)
{
  if(TLA_DEBUG)
    cout<<"MovingRealCompareMS called"<<endl;
  result.Clear();
  if( !op1.IsDefined() || !op2.IsDefined() ){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );

  MReal *mop2 = new MReal(op1.GetNoComponents());
  UReal up1;

  mop2->Resize(op1.GetNoComponents());
  mop2->StartBulkLoad();
  for (int i = 0; i < op1.GetNoComponents(); i++) {
    op1.Get(i, up1);
    if(!(up1.IsDefined() && op2.IsDefined()))
        continue;
    UReal *up2 = new UReal(up1.timeInterval, 0.0, 0.0, up1.r
               ? pow(op2.GetRealval(),2) : op2.GetRealval(), up1.r);
    mop2->Add(*up2);
    delete up2;
  }
  mop2->EndBulkLoad(false);
  if ( ms )
    MovingRealCompareMM2(op1, *mop2, result, op);
  else
    MovingRealCompareMM2(*mop2, op1, result, op);

  mop2->DeleteIfAllowed();
}

/*
1.1 Method ~MPointInsideLine~

calcultates the periods where the given MPoint lies
inside the given Line. It return the existing intervals in a Periods-Object.

*/
static void MPointInsideLine(const MPoint& mp, const Line& ln, Periods& pResult)
{
  if(TLA_DEBUG)
    cout<<"MPointLineInside called"<<endl;
  pResult.Clear();
  if( !mp.IsDefined() || !ln.IsDefined() ){
    pResult.SetDefined( false );
    return;
  }
  pResult.SetDefined( true );
  UPoint up;
  HalfSegment l;

  Periods* period = new Periods(0);
  Periods* between = new Periods(0);
  Point pt;
  Interval<Instant> newper; //part of the result

  for( int i = 0; i < mp.GetNoComponents(); i++)
  {
    mp.Get(i, up);

    if(TLA_DEBUG){
      cout<<"UPoint # "<<i<<" ["<<up.timeInterval.start.ToString()<<" "
      <<up.timeInterval.end.ToString()<<" "<<up.timeInterval.lc<<" "
      <<up.timeInterval.rc<<"] ("<<up.p0.GetX()<<" "<<up.p0.GetY()
      <<")->("<<up.p1.GetX()<<" "<<up.p1.GetY()<<")"<<endl;}

    for( int n = 0; n < ln.Size(); n++)
    {
      Instant t(instanttype);
      ln.Get(n, l);

      if(TLA_DEBUG){
        cout<<"UPoint # "<<i<<" ["<<up.timeInterval.start.ToString()
        <<" "<<up.timeInterval.end.ToString()<<" "<<up.timeInterval.lc
        <<" "<<up.timeInterval.rc<<"] ("<<up.p0.GetX()<<" "<<up.p0.GetY()
        <<")->("<<up.p1.GetX()<<" "<<up.p1.GetY()<<")"<<endl;
        cout<<"l      # "<<n<<" ("<<l.GetLeftPoint().GetX()
        <<" "<<l.GetLeftPoint().GetY()
        <<" "<<l.GetRightPoint().GetX()<<" "
        <<l.GetRightPoint().GetY()<<") "<<endl;}
      if (l.GetRightPoint().GetX() == l.GetDomPoint().GetX()
       && l.GetRightPoint().GetY() == l.GetDomPoint().GetY()) {
        if(TLA_DEBUG)
          cout<<"right point is dominating -> continue"<<endl;
        continue;
      }
      if(( l.GetRightPoint().GetX() < up.p0.GetX()
       &&  l.GetRightPoint().GetX() < up.p1.GetX())
       || (l.GetLeftPoint().GetX() > up.p0.GetX()
       &&  l.GetLeftPoint().GetX() > up.p1.GetX())
       || (l.GetRightPoint().GetY() < up.p0.GetY()
       &&  l.GetRightPoint().GetY() < up.p1.GetY()
       && (l.GetLeftPoint().GetY() < up.p0.GetY()
       &&  l.GetLeftPoint().GetY() < up.p1.GetY()))
       || (l.GetRightPoint().GetY() > up.p0.GetY()
       &&  l.GetRightPoint().GetY() > up.p1.GetY()
       && (l.GetLeftPoint().GetY() > up.p0.GetY()
       &&  l.GetLeftPoint().GetY() > up.p1.GetY()))) {
        if(TLA_DEBUG)
          cout<<"Bounding Boxes not crossing!"<<endl;
        continue;
      }
      double al=0.0, bl=0.0, aup=0.0, bup=0.0;
      bool vl, vup;
      vl = AlmostEqual( l.GetRightPoint().GetX(), l.GetLeftPoint().GetX() );
      if(!vl){
        al = (l.GetRightPoint().GetY() - l.GetLeftPoint().GetY())
           / (l.GetRightPoint().GetX() - l.GetLeftPoint().GetX());
        bl =  l.GetLeftPoint().GetY() - l.GetLeftPoint().GetX() * al;
          if(TLA_DEBUG)
            cout<<"al: "<<al<<" bl: "<<bl<<endl;
      }
      else
        if(TLA_DEBUG)
          cout<<"l is vertical"<<endl;
      vup = AlmostEqual( up.p1.GetX(), up.p0.GetX() );
      if(!vup){
        aup = (up.p1.GetY() - up.p0.GetY())
            / (up.p1.GetX() - up.p0.GetX());
        bup =  up.p0.GetY() - up.p0.GetX() * aup;
        if(TLA_DEBUG)
          cout<<"aup: "<<aup<<" bup: "<<bup<<endl;
      }
      else
        if(TLA_DEBUG)
          cout<<"up is vertical"<<endl;
      if(vl && vup){
        if(TLA_DEBUG)
          cout<<"both elements are vertical!"<<endl;
        if( !(AlmostEqual(up.p1.GetX(), l.GetLeftPoint().GetX())) ){
        if(TLA_DEBUG)
          cout<<"elements are vertical but not at same line"<<endl;
          continue;
        }
        else {
          if(TLA_DEBUG)
            cout<<"elements on same line"<<endl;
          if(up.p1.GetY() < l.GetLeftPoint().GetY()
           && up.p0.GetY() < l.GetLeftPoint().GetY()){
            if(TLA_DEBUG)
              cout<<"uPoint lower as linesegment"<<endl;
            continue;
          }
          else if(up.p1.GetY() > l.GetRightPoint().GetY()
           && up.p0.GetY() > l.GetRightPoint().GetY()){
            if(TLA_DEBUG)
              cout<<"uPoint higher as linesegment"<<endl;
            continue;
          }
          else{
            if(TLA_DEBUG)
              cout<<"uPoint and linesegment partequal"<<endl;
            if (up.p0.GetY() <= l.GetLeftPoint().GetY()
             && up.p1.GetY() >= l.GetLeftPoint().GetY()){
              if(TLA_DEBUG)
                cout<<"uPoint starts below linesegemet"<<endl;
              t.ReadFrom((l.GetLeftPoint().GetY() - up.p0.GetY())
                     / (up.p1.GetY() - up.p0.GetY())
                     * (up.timeInterval.end.ToDouble()
                     -  up.timeInterval.start.ToDouble())
                     +  up.timeInterval.start.ToDouble());
              t.SetType(instanttype);
              if(TLA_DEBUG)
                cout<<"t "<<t.ToString()<<endl;
              newper.start = t;
              newper.lc = (up.timeInterval.start == t)
                         ? up.timeInterval.lc : true;
            }
            if(up.p1.GetY() <= l.GetLeftPoint().GetY()
             && up.p0.GetY() >= l.GetLeftPoint().GetY()){
              if(TLA_DEBUG)
                cout<<"uPoint ends below linesegemet"<<endl;
              t.ReadFrom((l.GetLeftPoint().GetY() - up.p0.GetY())
                      / (up.p1.GetY() - up.p0.GetY())
                      * (up.timeInterval.end.ToDouble()
                      -  up.timeInterval.start.ToDouble())
                      +  up.timeInterval.start.ToDouble());
              t.SetType(instanttype);
              if(TLA_DEBUG)
                cout<<"t "<<t.ToString()<<endl;
              newper.end = t;
              newper.rc = (up.timeInterval.end == t)
                         ? up.timeInterval.rc : true;
            }
            if(up.p0.GetY() <= l.GetRightPoint().GetY()
             && up.p1.GetY() >= l.GetRightPoint().GetY()){
              if(TLA_DEBUG)
                cout<<"uPoint ends above linesegemet"<<endl;
              t.ReadFrom((l.GetRightPoint().GetY() - up.p0.GetY())
                      / (up.p1.GetY() - up.p0.GetY())
                      * (up.timeInterval.end.ToDouble()
                      -  up.timeInterval.start.ToDouble())
                      +  up.timeInterval.start.ToDouble());
              t.SetType(instanttype);
              if(TLA_DEBUG)
                cout<<"t "<<t.ToString()<<endl;
              newper.end = t;
              newper.rc = (up.timeInterval.end == t)
                         ? up.timeInterval.rc : true;
            }
            if(up.p1.GetY() <= l.GetRightPoint().GetY()
             && up.p0.GetY() >= l.GetRightPoint().GetY()){
              if(TLA_DEBUG)
                cout<<"uPoint starts above linesegemet"<<endl;
              t.ReadFrom((l.GetRightPoint().GetY() - up.p0.GetY())
                      / (up.p1.GetY() - up.p0.GetY())
                      * (up.timeInterval.end.ToDouble()
                      - up.timeInterval.start.ToDouble())
                      + up.timeInterval.start.ToDouble());
              t.SetType(instanttype);
              if(TLA_DEBUG)
                cout<<"t "<<t.ToString()<<endl;
              newper.start = t;
              newper.lc = (up.timeInterval.start == t)
                         ? up.timeInterval.lc : true;
            }
            if (up.p0.GetY() <= l.GetRightPoint().GetY()
             && up.p0.GetY() >= l.GetLeftPoint().GetY()){
              if(TLA_DEBUG)
                cout<<"uPoint starts inside linesegemet"<<endl;
              newper.start = up.timeInterval.start;
              newper.lc =    up.timeInterval.lc;
            }
            if( up.p1.GetY() <= l.GetRightPoint().GetY()
             && up.p1.GetY() >= l.GetLeftPoint().GetY()){
              if(TLA_DEBUG)
                cout<<"uPoint ends inside linesegemet"<<endl;
              newper.end = up.timeInterval.end;
              newper.rc =  up.timeInterval.rc;
            }
            if(newper.start == newper.end
             && (!newper.lc || !newper.rc)){
              if(TLA_DEBUG)
                cout<<"not an interval"<<endl;
              continue;
            }
          }
        }
      }
      else if(vl){
        if(TLA_DEBUG)
          cout<<"vl is vertical vup not"<<endl;
        t.ReadFrom((l.GetRightPoint().GetX() - up.p0.GetX())
                / (up.p1.GetX() - up.p0.GetX())
                * (up.timeInterval.end.ToDouble()
                -  up.timeInterval.start.ToDouble())
                +  up.timeInterval.start.ToDouble());
        t.SetType(instanttype);
        if(TLA_DEBUG)
          cout<<"t "<<t.ToString()<<endl;
        if((up.timeInterval.start == t && !up.timeInterval.lc)
         ||  (up.timeInterval.end == t && !up.timeInterval.rc))
          continue;

        if(up.timeInterval.start > t|| up.timeInterval.end < t){
          if(TLA_DEBUG)
            cout<<"up outside line"<<endl;
          continue;
        }
        up.TemporalFunction(t, pt, true);
        if(  pt.GetX() < l.GetLeftPoint().GetX() ||
             pt.GetX() > l.GetRightPoint().GetX()
         || (pt.GetY() < l.GetLeftPoint().GetY() &&
             pt.GetY() < l.GetRightPoint().GetY())
         || (pt.GetY() > l.GetLeftPoint().GetY() &&
             pt.GetY() > l.GetRightPoint().GetY())){
          if(TLA_DEBUG)
            cout<<"pt outside up!"<<endl;
          continue;
        }

        newper.start = t;
        newper.lc = true;
        newper.end = t;
        newper.rc = true;
      }
      else if(vup){
        if(TLA_DEBUG)
          cout<<"vup is vertical vl not"<<endl;
        if( !(AlmostEqual(up.p1.GetY(), up.p0.GetY())) ) {
          t.ReadFrom((up.p0.GetX() * al + bl - up.p0.GetY())
                  / (up.p1.GetY() - up.p0.GetY())
                  * (up.timeInterval.end.ToDouble()
                  -  up.timeInterval.start.ToDouble())
                  +  up.timeInterval.start.ToDouble());
          t.SetType(instanttype);
          if(TLA_DEBUG)
            cout<<"t "<<t.ToString()<<endl;
          if((up.timeInterval.start == t && !up.timeInterval.lc)
           ||  (up.timeInterval.end == t && !up.timeInterval.rc)){
            if(TLA_DEBUG)
              cout<<"continue"<<endl;
            continue;
          }

          if(up.timeInterval.start > t|| up.timeInterval.end < t){
            if(TLA_DEBUG)
              cout<<"up outside line"<<endl;
            continue;
          }
          up.TemporalFunction(t, pt, true);
          if(  pt.GetX() < l.GetLeftPoint().GetX() ||
               pt.GetX() > l.GetRightPoint().GetX()
           || (pt.GetY() < l.GetLeftPoint().GetY() &&
               pt.GetY() < l.GetRightPoint().GetY())
           || (pt.GetY() > l.GetLeftPoint().GetY() &&
               pt.GetY() > l.GetRightPoint().GetY())){
            if(TLA_DEBUG)
              cout<<"pt outside up!"<<endl;
            continue;
          }

          newper.start = t;
          newper.lc = true;
          newper.end = t;
          newper.rc = true;
        }
        else {
          if(TLA_DEBUG)
            cout<<"up is not moving"<<endl;
          if( AlmostEqual( al * up.p1.GetX() + bl, up.p1.GetY() ) ){
            if(TLA_DEBUG)
              cout<<"Point lies on line"<<endl;
            newper = up.timeInterval;
          }
          else {
            if(TLA_DEBUG)
              cout<<"continue 2"<<endl;
            continue;
          }
        }
      }
      else if( AlmostEqual(aup, al) ){
        if(TLA_DEBUG)
          cout<<"both lines have same gradient"<<endl;
        if( !(AlmostEqual(bup, bl)) ){
          if(TLA_DEBUG)
            cout<<"colinear but not equal"<<endl;
          continue;
        }
         if(up.p0.GetX() <= l.GetLeftPoint().GetX()
         && up.p1.GetX() >= l.GetLeftPoint().GetX()){
           if(TLA_DEBUG)
             cout<<"uPoint starts left of linesegemet"<<endl;
           t.ReadFrom((l.GetLeftPoint().GetX() - up.p0.GetX())
                   / (up.p1.GetX() - up.p0.GetX())
                   * (up.timeInterval.end.ToDouble()
                   -  up.timeInterval.start.ToDouble())
                   +  up.timeInterval.start.ToDouble());
           t.SetType(instanttype);
           if(TLA_DEBUG)
             cout<<"t "<<t.ToString()<<endl;
           newper.start = t;
           newper.lc = (up.timeInterval.start == t)
                      ? up.timeInterval.lc : true;
        }
        if(up.p1.GetX() <= l.GetLeftPoint().GetX()
        && up.p0.GetX() >= l.GetLeftPoint().GetX()){
           if(TLA_DEBUG)
             cout<<"uPoint ends left of linesegemet"<<endl;
           t.ReadFrom((l.GetLeftPoint().GetX() - up.p0.GetX())
                   / (up.p1.GetX() - up.p0.GetX())
                   * (up.timeInterval.end.ToDouble()
                   -  up.timeInterval.start.ToDouble())
                   +  up.timeInterval.start.ToDouble());
           t.SetType(instanttype);
           if(TLA_DEBUG)
             cout<<"t "<<t.ToString()<<endl;
           newper.end = t;
           newper.rc = (up.timeInterval.end == t)
                      ? up.timeInterval.rc : true;
        }
        if(up.p0.GetX() <= l.GetRightPoint().GetX()
        && up.p1.GetX() >= l.GetRightPoint().GetX()){
           if(TLA_DEBUG)
             cout<<"uPoint ends right of linesegemet"<<endl;
           t.ReadFrom((l.GetRightPoint().GetX() - up.p0.GetX())
                   / (up.p1.GetX() - up.p0.GetX())
                   * (up.timeInterval.end.ToDouble()
                   -  up.timeInterval.start.ToDouble())
                   +  up.timeInterval.start.ToDouble());
           t.SetType(instanttype);
           if(TLA_DEBUG)
             cout<<"t "<<t.ToString()<<endl;
           newper.end = t;
           newper.rc = (up.timeInterval.end == t)
                      ? up.timeInterval.rc : true;
        }
        if(up.p1.GetX() <= l.GetRightPoint().GetX()
        && up.p0.GetX() >= l.GetRightPoint().GetX()){
           if(TLA_DEBUG)
             cout<<"uPoint starts right of linesegemet"<<endl;
           t.ReadFrom((l.GetRightPoint().GetX() - up.p0.GetX())
                   / (up.p1.GetX() - up.p0.GetX())
                   * (up.timeInterval.end.ToDouble()
                   -  up.timeInterval.start.ToDouble())
                   +  up.timeInterval.start.ToDouble());
           t.SetType(instanttype);
           if(TLA_DEBUG)
             cout<<"t "<<t.ToString()<<endl;
           newper.start = t;
           newper.lc = (up.timeInterval.start == t)
                      ? up.timeInterval.lc : true;
        }
        if(up.p0.GetX() <= l.GetRightPoint().GetX()
        && up.p0.GetX() >= l.GetLeftPoint().GetX()){
           if(TLA_DEBUG)
             cout<<"uPoint starts inside linesegemet"<<endl;
           newper.start = up.timeInterval.start;
           newper.lc =    up.timeInterval.lc;
        }
        if(up.p1.GetX() <= l.GetRightPoint().GetX()
        && up.p1.GetX() >= l.GetLeftPoint().GetX()){
           if(TLA_DEBUG)
             cout<<"uPoint ends inside linesegemet"<<endl;
           newper.end = up.timeInterval.end;
           newper.rc =  up.timeInterval.rc;
        }
        if(newper.start == newper.end
        && (!newper.lc || !newper.rc)){
          if(TLA_DEBUG)
            cout<<"not an interval"<<endl;
          continue;
        }
      }
      else{
        if(TLA_DEBUG)
          cout<<"both lines have different gradients"<<endl;
        t.ReadFrom(((bl - bup) / (aup - al) - up.p0.GetX())
                / (up.p1.GetX() - up.p0.GetX())
                * (up.timeInterval.end.ToDouble()
                -  up.timeInterval.start.ToDouble())
                +  up.timeInterval.start.ToDouble());
        t.SetType(instanttype);
        if((up.timeInterval.start == t && !up.timeInterval.lc)
         ||  (up.timeInterval.end == t && !up.timeInterval.rc)){
          if(TLA_DEBUG)
            cout<<"continue"<<endl;
          continue;
        }

        if(up.timeInterval.start > t|| up.timeInterval.end < t){
          if(TLA_DEBUG)
            cout<<"up outside line"<<endl;
          continue;
        }
        up.TemporalFunction(t, pt, true);
        if(  pt.GetX() < l.GetLeftPoint().GetX() ||
             pt.GetX() > l.GetRightPoint().GetX()
         || (pt.GetY() < l.GetLeftPoint().GetY() &&
             pt.GetY() < l.GetRightPoint().GetY())
         || (pt.GetY() > l.GetLeftPoint().GetY() &&
             pt.GetY() > l.GetRightPoint().GetY())){
          if(TLA_DEBUG)
            cout<<"pt outside up!"<<endl;
          continue;
        }

        newper.start = t;
        newper.lc = true;
        newper.end = t;
        newper.rc = true;
      }
      if(TLA_DEBUG){
        cout<<"newper ["<< newper.start.ToString()<<" "<<newper.end.ToString()
        <<" "<<newper.lc<<" "<<newper.rc<<"]"<<endl;}
      period->Clear();
      period->StartBulkLoad();
      period->Add(newper);
      period->EndBulkLoad(false);
      if (!pResult.IsEmpty()) {
        between->Clear();
        period->Union(pResult, *between);
        pResult.Clear();
        pResult.CopyFrom(between);
      }
      else
        pResult.CopyFrom(period);
    }
  }
  between->DeleteIfAllowed();
  period->DeleteIfAllowed();
}

/*
1.1 Method ~MPointInMPoint~

Checks whether the two mpoints are crossing. It returns -1.0 if the mpoints are
not crossing and 2.0 if both mpoints are equal all the time. In all other cases
it returns the fraction of iv when mpoints are crossing (0.0 .. 1.0).

*/
double MPointInMPoint(double startx1, double starty1, double endx1,
                      double endy1, double startx2, double starty2,
                      double endx2, double endy2){
  if(AlmostEqual(startx1, startx2) && AlmostEqual(starty1, starty2)
     && AlmostEqual(endx1, endx2)  && AlmostEqual(endy1, endy2))
    return 2.0;
  else if(AlmostEqual(startx1, startx2) && AlmostEqual(starty1, starty2))
    return 0.0;
  else if(AlmostEqual(endx1, endx2) && AlmostEqual(endy1, endy2))
    return 1.0;
  else {
    double dx = endx1 - startx1 - endx2 + startx2;
    double dy = endy1 - starty1 - endy2 + starty2;
    double tx = 0.0;
    double ty = 0.0;
    bool vert = false;
    bool hor = false;
    if (dx == 0.0)
      vert = true;
    else
      tx = (startx2 - startx1) / dx;
    if (dy == 0.0)
      hor = true;
    else
      ty = (starty2 - starty1) / dy;
    if (hor) {
      if ((starty1 <= starty2 && starty1 >= endy2)
       || (starty1 <= endy2   && starty1 >= starty2)
       || (endy1   <= starty2 && endy1   >= endy2)
       || (endy1   <= endy2   && endy1   >= starty2)
       || (starty2 <= starty1 && starty2 >= endy1)
       || (starty2 <= endy1   && starty2 >= starty1)
       || (endy2   <= starty1 && endy2   >= endy1)
       || (endy2   <= endy1   && endy2   >= starty1))
          ty = tx;
      else
        ty = -1.0;
    }
    else if (vert) {
      if ((startx1 <= startx2 && startx1 >= endx2)
       || (startx1 <= endx2   && startx1 >= startx2)
       || (endx1   <= startx2 && endx1   >= endx2)
       || (endx1   <= endx2   && endx1   >= startx2)
       || (startx2 <= startx1 && startx2 >= endx1)
       || (startx2 <= endx1   && startx2 >= startx1)
       || (endx2   <= startx1 && endx2   >= endx1)
       || (endx2   <= endx1   && endx2   >= startx1))
        tx = ty;
      else
        tx = -1.0;
    }
    if(AlmostEqual(tx, ty) && tx > 0.0 && tx < 1.0)
      return tx;
    else
      return -1.0;
  }
  return -1.0;  //should not be reached!
}

/*
1.1 Method ~MovingPointCompareMM~

For Operators ~=~ , ~\#~ and ~minus~ for MovingPoint/MovingPoint

*/
void MovingPointCompareMM( const MPoint& p1, const MPoint& p2, MBool& result,
 int op)
{
  if(TLA_DEBUG)
    cout<<"MovingPointCompareMM called"<<endl;
  result.Clear();
  if( !p1.IsDefined() || !p2.IsDefined() ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  UBool uBool(true);
  RefinementPartition<MPoint, MPoint, UPoint, UPoint> rp(p1, p2);
  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  result.Resize(rp.Size());
  result.StartBulkLoad();
  for( unsigned int i = 0; i < rp.Size(); i++ )
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    UPoint u1;
    UPoint u2;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<" ["
        <<iv.start.ToString()<<" "<<iv.end.ToString()<<" "<<iv.lc
        <<" "<<iv.rc<< "] "<<u1Pos<< " "<<u2Pos<<endl;
      p1.Get(u1Pos, u1);
      p2.Get(u2Pos, u2);
      if(!(u1.IsDefined() && u2.IsDefined()))
        continue;
    }

    Point rp0, rp1, rp2, rp3;

    u1.TemporalFunction(iv.start, rp0, true);
    u1.TemporalFunction(iv.end, rp1, true);
    u2.TemporalFunction(iv.start, rp2, true);
    u2.TemporalFunction(iv.end, rp3, true);

    double t = MPointInMPoint(rp0.GetX(), rp0.GetY(), rp1.GetX(), rp1.GetY(),
                              rp2.GetX(), rp2.GetY(), rp3.GetX(), rp3.GetY());
    if(TLA_DEBUG)
      cout<<"t "<<t<<endl;

    if(t == 2.0){
      uBool.timeInterval = iv;
      uBool.constValue.Set(true, op == 0 ? true : false);
      result.MergeAdd( uBool );
    }
    else if(t == 0.0){
      if (iv.lc) {
        uBool.timeInterval.start = iv.start;
        uBool.timeInterval.end = iv.start;
        uBool.timeInterval.lc = true;
        uBool.timeInterval.rc = true;
        uBool.constValue.Set(true, op == 0 ? true : false);
        result.MergeAdd( uBool );
      }
      uBool.timeInterval = iv;
      uBool.timeInterval.lc = false;
      uBool.constValue.Set(true, op == 0 ? false : true);
      if(uBool.IsValid())
        result.MergeAdd( uBool );

    }
    else if(t == 1.0){
      uBool.timeInterval = iv;
      uBool.timeInterval.rc = false;
      uBool.constValue.Set(true, op == 0 ? false : true);
      if(uBool.IsValid())
        result.MergeAdd( uBool );
      if (iv.rc) {
        uBool.timeInterval.start = iv.end;
        uBool.timeInterval.end = iv.end;
        uBool.timeInterval.lc = true;
        uBool.timeInterval.rc = true;
        uBool.constValue.Set(true, op == 0 ? true : false);
        result.MergeAdd( uBool );
      }
    }
    else if(t > 0.0 && t < 1.0){
      Instant time(instanttype);
      time.ReadFrom(t  * (iv.end.ToDouble() - iv.start.ToDouble())
                       + iv.start.ToDouble());
      time.SetType(instanttype);
      if ((iv).Contains(time)) {
        uBool.timeInterval = iv;
        uBool.timeInterval.rc = false;
        uBool.timeInterval.end = time;
        uBool.constValue.Set(true, op == 0 ? false : true);
        if(uBool.IsValid())
          result.MergeAdd( uBool );
        uBool.timeInterval.lc = true;
        uBool.timeInterval.rc = true;
        uBool.timeInterval.start = time;
        uBool.timeInterval.end = time;
        uBool.constValue.Set(true, op == 0 ? true : false);
        result.MergeAdd( uBool );
        uBool.timeInterval.lc = false;
        uBool.timeInterval.rc = iv.rc;
        uBool.timeInterval.start = time;
        uBool.timeInterval.end = iv.end;
        uBool.constValue.Set(true, op == 0 ? false : true);
        if(uBool.IsValid())
          result.MergeAdd( uBool );
      }
      else{
        uBool.timeInterval = iv;
        uBool.constValue.Set(true, op == 0 ? false : true);
        result.MergeAdd( uBool );
      }
    }
    else{
      uBool.timeInterval = iv;
      uBool.constValue.Set(true, op == 0 ? false : true);
      result.MergeAdd( uBool );
    }
  }
  result.EndBulkLoad( false );
}

/*
1.1 Method ~MovingPointCompareMS~

For Operators ~=~, ~\#~ and ~minus~ for MovingPoint/Point

*/
void MovingPointCompareMS( const MPoint& p1, const Point& p2, MBool& result,
 int op)
{
  result.Clear();
  if( !p1.IsDefined() || !p2.IsDefined() ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );

  UBool uBool(true);
  result.StartBulkLoad();
  for( int i = 0; i < p1.GetNoComponents(); i++ )
  {
    Interval<Instant> iv;
    UPoint u1;

    p1.Get(i, u1);
    if(!(u1.IsDefined() && p2.IsDefined()))
        continue;
    iv = u1.timeInterval;
    if(TLA_DEBUG){
      cout<< "Compare interval #"<< i<< ": "<< iv.start.ToString()<< " "
      << iv.end.ToString()<< " "<< iv.lc<< endl;}

    Point rp0, rp1;

    double t = MPointInMPoint(u1.p0.GetX(), u1.p0.GetY(),
                              u1.p1.GetX(), u1.p1.GetY(),
                              p2.GetX(),  p2.GetY(),  p2.GetX(),  p2.GetY());
    if(TLA_DEBUG)
      cout<<"t "<<t<<endl;

    if(t == 2.0) {  //start and end equal
      uBool.timeInterval = iv;
      uBool.constValue.Set(true, op == 0 ? true : false);
      result.MergeAdd( uBool );
    }
    else if(t == 0.0) {  //only start equal
      if (iv.lc) {
        uBool.timeInterval.start = iv.start;
        uBool.timeInterval.end = iv.start;
        uBool.timeInterval.lc = true;
        uBool.timeInterval.rc = true;
        uBool.constValue.Set(true, op == 0 ? true : false);
        result.MergeAdd( uBool );
      }
      uBool.timeInterval = iv;
      uBool.timeInterval.lc = false;
      uBool.constValue.Set(true, op == 0 ? false : true);
      result.MergeAdd( uBool );
    }
    else if(t == 1.0) {  //only end equal
      uBool.timeInterval = iv;
      uBool.timeInterval.rc = false;
      uBool.constValue.Set(true, op == 0 ? false : true);
      result.MergeAdd( uBool );
      if (iv.rc) {
        uBool.timeInterval.start = iv.end;
        uBool.timeInterval.end = iv.end;
        uBool.timeInterval.lc = true;
        uBool.timeInterval.rc = true;
        uBool.constValue.Set(true, op == 0 ? true : false);
        result.MergeAdd( uBool );
      }
    }
    else if(t > 0.0 && t < 1.0) {
      Instant time(instanttype);
      time.ReadFrom(t  * (iv.end.ToDouble() - iv.start.ToDouble())
                     + iv.start.ToDouble());
      time.SetType(instanttype);
      if (iv.Contains(time)) {
        uBool.timeInterval = iv;
        uBool.timeInterval.rc = false;
        uBool.timeInterval.end = time;
        uBool.constValue.Set(true, op == 0 ? false : true);
        if(uBool.IsValid())
          result.MergeAdd( uBool );
        uBool.timeInterval.rc = true;
        uBool.timeInterval.start = time;
        uBool.timeInterval.lc = true;
        uBool.constValue.Set(true, op == 0 ? true : false);
        result.MergeAdd( uBool );
        uBool.timeInterval.lc = false;
        uBool.timeInterval.rc = iv.rc;
        uBool.timeInterval.end = iv.end;
        uBool.constValue.Set(true, op == 0 ? false : true);
        if(uBool.IsValid())
          result.MergeAdd( uBool );
      }
      else{
        uBool.timeInterval = iv;
        uBool.constValue.Set(true, op == 0 ? false : true);
        result.MergeAdd( uBool );
      }
    }
    else {
      uBool.timeInterval = iv;
      uBool.constValue.Set(true, op == 0 ? false : true);
      result.MergeAdd( uBool );
    }
  }
  result.EndBulkLoad( false );
}

/*
1.1 Method ~MovingRegionCompareMS~

For Operators ~=~, ~\#~ and ~minus~ forMovingRegion/Region

*/
void MovingRegionCompareMS( MRegion *mr, const Region *r, MBool *result,
 int op)
{
  assert( mr );
  assert( r );
  assert( result );

  result->Clear();
  if( !mr->IsDefined() || !r->IsDefined() ) {
    result->SetDefined( false );
  }
  result->SetDefined( true );
  URegionEmb ur;
  UBool uBool(true);   //Part of the result

  result->StartBulkLoad();
  if(TLA_DEBUG)
    cout<<"MovingRegionCompareMS called"<<endl;
  for(int i = 0; i < mr->GetNoComponents(); i++){
    mr->Get(i, ur);
    if(!(ur.IsValid() && r->IsDefined()))
        continue;
    int number = ur.GetSegmentsNum();
    if(TLA_DEBUG){
      cout<<"URegion # "<<i<<" "<<"[ "<<ur.timeInterval.start.ToString()<<" "
      <<ur.timeInterval.end.ToString()<<" ]"<<endl;
      cout<<"number of segments = "<< number<<endl;}

    bool staticequal = true;
    bool finish = false;
    MSegmentData dms;
    int i = 0;
    while(staticequal && (i < ur.GetSegmentsNum())){
      ur.GetSegment(mr->GetMSegmentData(), i, dms);
      if (dms.GetInitialStartX() == dms.GetFinalStartX()
       && dms.GetInitialStartY() == dms.GetFinalStartY()
       && dms.GetInitialEndX()   == dms.GetFinalEndX()
       && dms.GetInitialEndY()   == dms.GetFinalEndY()){
        if(TLA_DEBUG)
          cout<<"s is static"<<endl;
        //find matching CHalfsegment set staticequal!!
        Point *p1 = new Point(true,
                    dms.GetFinalStartX(), dms.GetFinalStartY());
        Point *p2 = new Point(true,
                    dms.GetFinalEndX(), dms.GetFinalEndY());
        HalfSegment *nHS = new HalfSegment(true, *p1, *p2);
        delete p1;
        delete p2;
        HalfSegment mid;
        bool found = false;
        int left = 0;
        int right = r->Size();
        int middle;
        while(!found && left != right){
          middle = (left + right) / 2;
          r->Get(middle, mid);
          if(mid == *nHS)
            found = true;
          else if(mid < *nHS){
            left = middle;
          }
          else{
            right = middle;
          }
        }
        if(!found){
          if(TLA_DEBUG)
            cout<<"no matching Halfsegment -> Unit not equal!!"<<endl;
          staticequal = false;
          finish = true;
        }
        delete nHS;
      }
      else {
        if(TLA_DEBUG)
          cout<<"s is not static"<<endl;
        staticequal = false;
      }
      i++;
    }
    if(staticequal){
      uBool.timeInterval = ur.timeInterval;
      uBool.constValue.Set(true, (op == 0) ? true : false);
      if(TLA_DEBUG){
        cout<<"all Halfsegments matching -> Unit is equal!!"<<endl;
        cout<<"uBool "<<uBool.constValue.GetBoolval()
        <<" ["<<uBool.timeInterval.start.ToString()
        <<" "<<uBool.timeInterval.end.ToString()<<" "
        <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
      result->MergeAdd(uBool);
    }
    else if(finish){
      uBool.timeInterval = ur.timeInterval;
      uBool.constValue.Set(true, (op == 0) ? false : true);
      if(TLA_DEBUG){
        cout<<"a static s with no matching HS -> unit not equal!"
        <<endl;
        cout<<"uBool "<<uBool.constValue.GetBoolval()
        <<" ["<<uBool.timeInterval.start.ToString()
        <<" "<<uBool.timeInterval.end.ToString()<<" "
        <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
      result->MergeAdd(uBool);
    }
    else{ //the complicate way with not static mregions
      HalfSegment chs;
      Periods* period  = new Periods(0);
      Periods* between = new Periods(0);
      Periods* pResult = new Periods(0);
      Interval<Instant> newper; //part of the result

      for(int i = 0 ;i < r->Size(); i++){
        r->Get(i, chs);

        double tsd = MPointInMPoint(dms.GetInitialStartX(),
          dms.GetInitialStartY(), dms.GetFinalStartX(),
          dms.GetFinalStartY(), chs.GetDomPoint().GetX(),
          chs.GetDomPoint().GetY(), chs.GetDomPoint().GetX(),
          chs.GetDomPoint().GetY());
        double ted = MPointInMPoint(dms.GetInitialEndX(),
          dms.GetInitialEndY(), dms.GetFinalEndX(),
          dms.GetFinalEndY(), chs.GetDomPoint().GetX(),
          chs.GetDomPoint().GetY(), chs.GetDomPoint().GetX(),
          chs.GetDomPoint().GetY());
        double tss = MPointInMPoint(dms.GetInitialStartX(),
          dms.GetInitialStartY(), dms.GetFinalStartX(),
          dms.GetFinalStartY(), chs.GetSecPoint().GetX(),
          chs.GetSecPoint().GetY(), chs.GetSecPoint().GetX(),
          chs.GetSecPoint().GetY());
        double tes = MPointInMPoint(dms.GetInitialEndX(),
          dms.GetInitialEndY(), dms.GetFinalEndX(),
          dms.GetFinalEndY(), chs.GetSecPoint().GetX(),
          chs.GetSecPoint().GetY(), chs.GetSecPoint().GetX(),
          chs.GetSecPoint().GetY());

        double tpoint = -1.0;
        if(tsd >= 0.0 && tes >= 0.0){
          if(TLA_DEBUG)
            cout<<"start through dominant, end through subdominat"<<endl;
          if(AlmostEqual(tsd, tes))
            tpoint = tsd;
          if(AlmostEqual(tsd, 2.0))
            tpoint = tes;
          if(AlmostEqual(tes, 2.0))
            tpoint = tsd;
        }
        if(tss >= 0.0 && ted >= 0.0){
          if(TLA_DEBUG)
            cout<<"start through subdominant, end through dominat"<<endl;
          if(AlmostEqual(tss, ted))
            tpoint = ted;
          if(AlmostEqual(tss, 2.0))
            tpoint = ted;
          if(AlmostEqual(ted, 2.0))
            tpoint = tss;
        }
        if((tpoint > 0.0 && tpoint < 1.0)
         || (tpoint == 0.0 && ur.timeInterval.lc)
         || (tpoint == 1.0 && ur.timeInterval.rc)){
          Instant t(instanttype);
          t.ReadFrom((ur.timeInterval.end.ToDouble()
                    - ur.timeInterval.start.ToDouble()) * tpoint
                    + ur.timeInterval.start.ToDouble());
          t.SetType(instanttype);
          newper.start = t;
          newper.end = t;
          newper.lc = true;
          newper.rc = true;

          if(TLA_DEBUG)
            cout<<"newper ["<< newper.start.ToString() <<" "
            <<newper.end.ToString()<<" "<<newper.lc<<" "<<newper.rc<<"]"<<endl;
          period->Clear();
          period->StartBulkLoad();
          period->Add(newper);
          period->EndBulkLoad(false);
          if (!pResult->IsEmpty()) {
            between->Clear();
            period->Union(*pResult, *between);
            pResult->Clear();
            pResult->CopyFrom(between);
          }
          else
            pResult->CopyFrom(period);
          }
        }
      period->DeleteIfAllowed();
      between->DeleteIfAllowed();
      Interval<Instant> per;
      for(int i = 0; i < pResult->GetNoComponents(); i++){
        Region snapshot(0);
        if(TLA_DEBUG)
          cout<<"add interval # "<<i<<endl;
        pResult->Get(i, per);
        ur.TemporalFunction(mr->GetMSegmentData(), per.start, snapshot, true);
        if(*r == snapshot){
          if(TLA_DEBUG)
            cout<<"r == snapshot!"<<endl;
          if(per.start > ur.timeInterval.start){
            uBool.timeInterval.start = ur.timeInterval.start;
            uBool.timeInterval.lc = ur.timeInterval.lc;
            uBool.timeInterval.end = per.start;
            uBool.timeInterval.rc = !per.rc;
            uBool.constValue.Set(true, (op == 0) ? false : true);
            if(TLA_DEBUG)
              cout<<"uBool "<<uBool.constValue.GetBoolval()
              <<" ["<<uBool.timeInterval.start.ToString()
              <<" "<<uBool.timeInterval.end.ToString()<<" "
              <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;
            result->MergeAdd(uBool);
          }
          uBool.timeInterval = per;
          uBool.constValue.Set(true, (op == 0) ? true : false);
          if(TLA_DEBUG)
            cout<<"uBool "<<uBool.constValue.GetBoolval()
            <<" ["<<uBool.timeInterval.start.ToString()
            <<" "<<uBool.timeInterval.end.ToString()<<" "
            <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;
          result->MergeAdd(uBool);
          if(per.end < ur.timeInterval.end){
            uBool.timeInterval.start = per.end;
            uBool.timeInterval.lc = !per.lc;
            uBool.timeInterval.end = ur.timeInterval.end;
            uBool.timeInterval.rc = ur.timeInterval.rc;
            uBool.constValue.Set(true, (op == 0) ? false : true);
            if(TLA_DEBUG)
              cout<<"uBool "<<uBool.constValue.GetBoolval()
              <<" ["<<uBool.timeInterval.start.ToString()
              <<" "<<uBool.timeInterval.end.ToString()<<" "
              <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;
            result->MergeAdd(uBool);
          }
        }
        else
          if(TLA_DEBUG)
            cout<<"r != snapshot"<<endl;
      }
      pResult->DeleteIfAllowed();
    }
  }
  result->EndBulkLoad(false);
}

/*
1.1 Method ~MovingRegionCompareMM~

For Operators ~=~ and ~\#~ and MovingRegion/Region

*/
void MovingRegionCompareMM( MRegion *mr1, MRegion *mr2, MBool *result,
 int op)
{
  assert( mr1 );
  assert( mr2 );
  assert( result );
  if(TLA_DEBUG)
    cout<<"MovingRegionCompareMM called"<<endl;
  result->Clear();
  if( !mr1->IsDefined() || !mr2->IsDefined() ) {
    result->SetDefined( false );
  }
  result->SetDefined( true );

  RefinementPartition<MRegion, MRegion, URegionEmb, URegionEmb> rp(*mr1, *mr2);
  if(TLA_DEBUG)
    cout<<"RefimentPartiion done with size "<<rp.Size()<<endl;
  Interval<Instant> iv;
  int reg1Pos;
  int reg2Pos;
  UBool uBool(true);

  result->Resize(rp.Size());
  result->StartBulkLoad();

  for( unsigned int i = 0; i < rp.Size(); i++ ){
    rp.Get(i, iv, reg1Pos, reg2Pos);
    if(TLA_DEBUG)
      cout<<"interval # "<<i<<" "<<reg1Pos<<" "<<reg2Pos<<endl;
    if(reg1Pos == -1 || reg2Pos == -1)
      continue;
    if(TLA_DEBUG){
      cout<<"bothoperators in iv # "<<i<<" [ "
      <<iv.start.ToString()<<" "<<iv.end.ToString()<<" "<<iv.lc<<" "
      <<iv.rc<<" ] reg1Pos "<<reg1Pos<<", reg2Pos "<<reg2Pos <<endl;}
    URegionEmb ureg1;
    URegionEmb ureg2;
    mr1->Get(reg1Pos, ureg1);
    mr2->Get(reg2Pos, ureg2);
    if(!(ureg1.IsValid() && ureg2.IsValid()))
        continue;
    if(ureg1.GetSegmentsNum() != ureg1.GetSegmentsNum()){
      uBool.timeInterval = iv;
      uBool.constValue.Set(true, op == 0 ? false : true);
      if(TLA_DEBUG){
        cout<<"uregions have different numbers of segments -> iv not equal";
        cout<<endl<<"MergeAdd uBool "<<uBool.constValue.GetBoolval()
        <<" ["<<uBool.timeInterval.start.ToString()
        <<" "<<uBool.timeInterval.end.ToString()<<" "
        <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
      result->MergeAdd(uBool);
      continue;
    }
    if((ureg1.GetSegmentsNum() == 0)&&(ureg1.GetSegmentsNum() == 0)){
      uBool.timeInterval = iv;
      uBool.constValue.Set(true, op == 0 ? true : false);
      if(TLA_DEBUG){
        cout<<"both uregions have no segments -> iv  equal"<<endl;
        cout<<"MergeAdd uBool "<<uBool.constValue.GetBoolval()
        <<" ["<<uBool.timeInterval.start.ToString()
        <<" "<<uBool.timeInterval.end.ToString()<<" "
        <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
      result->MergeAdd(uBool);
      continue;
    }
    //find possible times of equality
    MSegmentData dms1;
    Periods* period  = new Periods(0);
    Periods* between = new Periods(0);
    Periods* pResult = new Periods(0);
    Interval<Instant> newper; //part of the result
    bool uregionPerhapsEqual = false;
    MSegmentData rdms1;

    ureg1.GetSegment(mr1->GetMSegmentData(), 0, dms1);
    dms1.restrictToInterval(ureg1.timeInterval, iv, rdms1);

    pResult->Clear();
    for(int n = 0; n < ureg2.GetSegmentsNum(); n++){
      MSegmentData dms2;
      MSegmentData rdms2;

      ureg2.GetSegment(mr2->GetMSegmentData(), n, dms2);
      dms2.restrictToInterval(ureg2.timeInterval, iv, rdms2);
      double ts = MPointInMPoint(
        rdms1.GetInitialStartX(), rdms1.GetInitialStartY(),
        rdms1.GetFinalStartX(), rdms1.GetFinalStartY(),
        rdms2.GetInitialStartX(), rdms2.GetInitialStartY(),
        rdms2.GetFinalStartX(), rdms2.GetFinalStartY());
      double te = MPointInMPoint(
        rdms1.GetInitialEndX(), rdms1.GetInitialEndY(),
        rdms1.GetFinalEndX(), rdms1.GetFinalEndY(),
        rdms2.GetInitialEndX(), rdms2.GetInitialEndY(),
        rdms2.GetFinalEndX(), rdms2.GetFinalEndY());

      if(ts == 2.0 && te == 2.0){
        if(TLA_DEBUG)
          cout<<"uregions are possibly totaly equal"<<endl;
        uregionPerhapsEqual = true;
      }
      else if(((ts >=0.0) && (ts <= 1.0) && AlmostEqual(ts, te))
       || (ts == 2.0 && (te >=0.0) && (te <= 1.0))
       || (te == 2.0 && (ts >=0.0) && (ts <= 1.0))){
        if(TLA_DEBUG)
          cout<<"equality found at t "<<ts<<" for dms "<<n<<endl;
        if(ts == 2.0 && (te >=0.0) && (te <= 1.0))
          ts = te;
        if((ts == 0.0 && !iv.lc) || (ts == 1.0 && !iv.rc))
          continue;
        Instant t(instanttype);
        t.ReadFrom((iv.end.ToDouble() - iv.start.ToDouble()) * ts
                  + iv.start.ToDouble());
        t.SetType(instanttype);
        newper.start = t;
        newper.end = t;
        newper.lc = true;
        newper.rc = true;
        if(TLA_DEBUG){
          cout<<"newper ["<< newper.start.ToString()<<" "<<newper.end.ToString()
          <<" "<<newper.lc<<" "<<newper.rc<<"]"<<endl;}
        period->Clear();
        period->StartBulkLoad();
        period->Add(newper);
        period->EndBulkLoad(false);
        if (!pResult->IsEmpty()) {
          between->Clear();
          period->Union(*pResult, *between);
          pResult->Clear();
          pResult->CopyFrom(between);
        }
        else
          pResult->CopyFrom(period);
      }
    }
    period->DeleteIfAllowed();
    between->DeleteIfAllowed();

    if(uregionPerhapsEqual){
      /*
      test for eaquality of total uregion. This can not be done at start and
      end of interval,because they are probably not inside the interval.
      So it is used the 10 and 90 percent time of the uregion.

      */
      Region snapshot1(0);
      Region snapshot2(0);
      if(TLA_DEBUG)
        cout<<"uregions are possibly equal. Create snapshots"<<endl;
      Instant time(instanttype);
      time.ReadFrom(0.1  * (iv.end.ToDouble() - iv.start.ToDouble())
                         + iv.start.ToDouble());
      time.SetType(instanttype);
      ureg1.TemporalFunction(mr1->GetMSegmentData(), time, snapshot1, true);
      ureg2.TemporalFunction(mr2->GetMSegmentData(), time, snapshot2, true);
      if(snapshot1 == snapshot2){
        if(TLA_DEBUG)
          cout<<"snapshots of iv->start are equal"<<endl;
        time.ReadFrom(0.1  * (iv.end.ToDouble() - iv.start.ToDouble())
                           + iv.start.ToDouble());
        time.SetType(instanttype);
        ureg1.TemporalFunction(mr1->GetMSegmentData(), time, snapshot1, true);
        ureg2.TemporalFunction(mr2->GetMSegmentData(), time, snapshot2, true);
        if(snapshot1 == snapshot2){

          uBool.timeInterval = iv;
          uBool.constValue.Set(true, op == 0 ? true : false);
          if(TLA_DEBUG){
            cout<<"snapshots of iv->end are equal, too."<<endl;
            cout<<"MergeAdd uBool "<<uBool.constValue.GetBoolval()
            <<" ["<<uBool.timeInterval.start.ToString()
            <<" "<<uBool.timeInterval.end.ToString()<<" "
            <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
          result->MergeAdd(uBool);
          continue;
        }
        else
          if(TLA_DEBUG){
            cout<<"snapshots of iv->end are not equal,"
            <<" uregions are not equal"<<endl;}
      }
      else
        if(TLA_DEBUG){
          cout<<"snapshots of iv->start are not equal,"
          <<" uregegions are not equal"<<endl;}
    }
    Interval<Instant> per;
    bool finished = false;
    for(int i = 0; i < pResult->GetNoComponents(); i++){
      Region snapshot1(0);
      Region snapshot2(0);
      pResult->Get(i, per);
      if(TLA_DEBUG)
        cout<<"test time # "<<i<<" "<<per.start.ToString()<<endl;
      if ((per.start == ureg1.timeInterval.start && !ureg1.timeInterval.lc)
       || (per.start == ureg2.timeInterval.start && !ureg2.timeInterval.lc)
       || (per.start == ureg1.timeInterval.end   && !ureg1.timeInterval.rc)
       || (per.start == ureg2.timeInterval.end   && !ureg2.timeInterval.rc))
        /*
        no snapshot possible, so this uregions can not be equal at this time!

        */
        continue;
      ureg1.TemporalFunction(mr1->GetMSegmentData(),per.start,snapshot1,true);
      ureg2.TemporalFunction(mr2->GetMSegmentData(),per.start,snapshot2,true);
      if(snapshot1 == snapshot2){
        if(TLA_DEBUG)
          cout<<"snapshot equal!"<<endl;
        if(per.start > iv.start){
          uBool.timeInterval.start = iv.start;
          uBool.timeInterval.lc = iv.lc;
          uBool.timeInterval.end = per.start;
          uBool.timeInterval.rc = !per.rc;
          uBool.constValue.Set(true, (op == 0) ? false : true);
          if(TLA_DEBUG){
            cout<<"uBool "<<uBool.constValue.GetBoolval()
            <<" ["<<uBool.timeInterval.start.ToString()
            <<" "<<uBool.timeInterval.end.ToString()<<" "
            <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
          result->MergeAdd(uBool);
        }
        uBool.timeInterval = per;
        uBool.constValue.Set(true, (op == 0) ? true : false);
        if(TLA_DEBUG){
          cout<<"uBool "<<uBool.constValue.GetBoolval()
          <<" ["<<uBool.timeInterval.start.ToString()
          <<" "<<uBool.timeInterval.end.ToString()<<" "
          <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
        result->MergeAdd(uBool);
        if(per.end < iv.end){
          uBool.timeInterval.start = per.end;
          uBool.timeInterval.lc = !per.lc;
          uBool.timeInterval.end = iv.end;
          uBool.timeInterval.rc = iv.rc;
          uBool.constValue.Set(true, (op == 0) ? false : true);
          if(TLA_DEBUG){
            cout<<"uBool "<<uBool.constValue.GetBoolval()
            <<" ["<<uBool.timeInterval.start.ToString()
            <<" "<<uBool.timeInterval.end.ToString()<<" "
            <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
          result->MergeAdd(uBool);
        }
        finished = true;
        break;
      }
      else
        if(TLA_DEBUG)
          cout<<"snapshot not equal"<<endl;
    }
    pResult->DeleteIfAllowed();
    if(!finished){
      uBool.timeInterval = iv;
      uBool.constValue.Set(true, (op == 0) ? false : true);
      if(TLA_DEBUG){
        cout<<"uBool "<<uBool.constValue.GetBoolval()
        <<" ["<<uBool.timeInterval.start.ToString()
        <<" "<<uBool.timeInterval.end.ToString()<<" "
        <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
      result->MergeAdd(uBool);
    }
  }
  result->EndBulkLoad(false);
}

/*
1.1 Method ~CompletePeriods2MBool~

Completes a Periods-value to a MBool-value. For this it puts the intervals in
pResult as uBool with value ~true~ and adds the difference to the MPoint-
intervals with ~false~.

*/
static void CompletePeriods2MBool( const MPoint* mp, const Periods* pResult,
                                   MBool* endResult){
  assert( mp );
  assert( pResult );
  assert( endResult );

  endResult->Clear();
  if( !mp->IsDefined() || !pResult->IsDefined() ){
    endResult->SetDefined( false );
    return;
  }
  endResult->SetDefined( true );

  endResult->StartBulkLoad();
  Interval<Instant> per;
  UPoint up;
  UBool uBool(true);
  int m = 0;
  bool pfinished = (pResult->GetNoComponents() == 0);
  for ( int i = 0; i < mp->GetNoComponents(); i++) {
    mp->Get(i, up);
    if(!up.IsDefined())
        continue;
    if(TLA_DEBUG){
      cout<<"UPoint # "<<i<<" ["<<up.timeInterval.start.ToString()
      <<" "<<up.timeInterval.end.ToString()<<" "<<up.timeInterval.lc<<" "
      <<up.timeInterval.rc<<"] ("<<up.p0.GetX()<<" "<<up.p0.GetY()<<")->("
      <<up.p1.GetX()<<" "<<up.p1.GetY()<<")"<<endl;}
    if(!pfinished) {
      pResult->Get(m, per);
      if(TLA_DEBUG){
        cout<<"per "<<m<<" ["<<per.start.ToString()<<" "
        <<per.end.ToString()<<" "<<per.lc<<" "<<per.rc<<"]"<<endl;}
    }
    else
      if(TLA_DEBUG)
        cout<<"no per any more"<<endl;
    if(pfinished || up.timeInterval.end < per.start
      || (up.timeInterval.end == per.start
      && !up.timeInterval.rc && per.lc)) {
       uBool.constValue.Set(true, false);
       uBool.timeInterval = up.timeInterval;
       if(TLA_DEBUG){
         cout<<"per totally after up"<<endl;
         cout<<"MergeAdd1 "<<uBool.constValue.GetBoolval()
         <<" ["<<uBool.timeInterval.start.ToString()<<" "
         <<uBool.timeInterval.end.ToString()<<" "
         <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
       endResult->MergeAdd(uBool);
    }
    else {
      if(TLA_DEBUG)
        cout<<"per not after before up"<<endl;
      if(up.timeInterval.start < per.start
       || (up.timeInterval.start == per.start
       && up.timeInterval.lc && !per.lc)) {
        uBool.constValue.Set(true, false);
        uBool.timeInterval.start = up.timeInterval.start;
        uBool.timeInterval.lc = up.timeInterval.lc;
        uBool.timeInterval.end = per.start;
        uBool.timeInterval.rc = !per.lc;
        if(TLA_DEBUG){
          cout<<"up starts before up"<<endl;
          cout<<"MergeAdd2 "<<uBool.constValue.GetBoolval()
          <<" ["<<uBool.timeInterval.start.ToString()<<" "
          <<uBool.timeInterval.end.ToString()<<" "
          <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
        endResult->MergeAdd(uBool);
        uBool.timeInterval = per;
      }
      else {
        if(TLA_DEBUG)
          cout<<"per starts before or with up"<<endl;
        uBool.timeInterval.start = up.timeInterval.start;
        uBool.timeInterval.lc = up.timeInterval.lc;
      }
      while(true) {
        uBool.constValue.Set(true, true);
        if(up.timeInterval.end < per.end
         || (up.timeInterval.end == per.end
         && per.rc && !up.timeInterval.rc)) {
            uBool.timeInterval.end = up.timeInterval.end;
            uBool.timeInterval.rc = up.timeInterval.rc;
            if(TLA_DEBUG){
              cout<<"per ends after up (break)"<<endl;
              cout<<"MergeAdd3 "<<uBool.constValue.GetBoolval()
              <<" ["<<uBool.timeInterval.start.ToString()<<" "
              <<uBool.timeInterval.end.ToString()<<" "
              <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
            endResult->MergeAdd(uBool);
            break;
        }
        else {

          uBool.timeInterval.end = per.end;
          uBool.timeInterval.rc = per.rc;
          if(TLA_DEBUG){
            cout<<"per ends inside up"<<endl;
            cout<<"MergeAdd4 "<<uBool.constValue.GetBoolval()
            <<" ["<<uBool.timeInterval.start.ToString()<<" "
            <<uBool.timeInterval.end.ToString()<<" "
            <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
          endResult->MergeAdd(uBool);
        }
        uBool.timeInterval.start = per.end;
        uBool.timeInterval.lc = !per.rc;
        if(m == pResult->GetNoComponents() - 1){
          pfinished = true;
        }
        else {
          pResult->Get(++m, per);
          if(TLA_DEBUG){
            cout<<"per "<<m<<" ["<<per.start.ToString()
            <<" "<<per.end.ToString()<<" "<<per.lc<<" "<<per.rc<<"]"<<endl;}
        }

        if(!pfinished && (per.start < up.timeInterval.end
         || (per.start == up.timeInterval.end
         && up.timeInterval.rc && per.rc))){
          uBool.timeInterval.end = per.start;
          uBool.timeInterval.rc = !per.lc;
          uBool.constValue.Set(true, false);
          if(TLA_DEBUG){
            cout<<"next per starts in same up"<<endl;
            cout<<"MergeAdd6 "<<uBool.constValue.GetBoolval()
            <<" ["<<uBool.timeInterval.start.ToString()<<" "
            <<uBool.timeInterval.end.ToString()<<" "
            <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
          endResult->MergeAdd(uBool);
          uBool.timeInterval.start = per.start;
          uBool.timeInterval.lc = per.lc;
        }
        else {
          if(TLA_DEBUG)
            cout<<"next interval after up -> finish up"<<endl;
          uBool.timeInterval.end = up.timeInterval.end;
          uBool.timeInterval.rc = up.timeInterval.rc;
          uBool.constValue.Set(true, false);
          if(uBool.timeInterval.end > uBool.timeInterval.start
           || (uBool.timeInterval.rc && uBool.timeInterval.lc)) {
            if(TLA_DEBUG){
              cout<<"MergeAdd5 "<<uBool.constValue.GetBoolval()
              <<" ["<<uBool.timeInterval.start.ToString()<<" "
              <<uBool.timeInterval.end.ToString()<<" "
              <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
            endResult->MergeAdd(uBool);
          }
          break;
        }
      } //while
    }
  }
  endResult->EndBulkLoad(false);
}

/*
1.1 Method ~CompletePeriods2MPoint~

Completes a Periods-value to a MPoint-value. For this it adds the starting
and end points.

*/
static void CompletePeriods2MPoint( const MPoint* mp, const Periods* pResult,
                                    MPoint* endResult){
  if(TLA_DEBUG)
    cout<<"CompletePeriods2MPoint called"<<endl;

  assert( mp );
  assert( pResult );
  assert( endResult );
  endResult->Clear();
  if( !mp->IsDefined() || !pResult->IsDefined() ){
    endResult->SetDefined( false );
    return;
  }
  endResult->SetDefined( true );

  UPoint up;
  endResult->StartBulkLoad();
  Interval<Instant> per;
  UPoint newUp(true);
  Point pt;
  int m = 0;
  bool pfinished = (pResult->GetNoComponents() == 0);
  for ( int i = 0; i < mp->GetNoComponents(); i++) {
    mp->Get(i, up);
    if(!up.IsDefined())
        continue;
    if(TLA_DEBUG){
      cout<<"UPoint # "<<i<<" ["<<up.timeInterval.start.ToString()
      <<" "<<up.timeInterval.end.ToString()<<" "<<up.timeInterval.lc<<" "
      <<up.timeInterval.rc<<"] ("<<up.p0.GetX()<<" "<<up.p0.GetY()<<")->("
      <<up.p1.GetX()<<" "<<up.p1.GetY()<<")"<<endl;}
    if(!pfinished) {
      pResult->Get(m, per);
      if(TLA_DEBUG){
        cout<<"per "<<m<<" ["<<per.start.ToString()<<" "
        <<per.end.ToString()<<" "<<per.lc<<" "<<per.rc<<"]"<<endl;}
    }
    if(pfinished) {
      if(TLA_DEBUG)
        cout<<"no per any more. break 1"<<endl;
      break;
    }
    if(!(pfinished || up.timeInterval.end < per.start
     || (up.timeInterval.end == per.start
     && !up.timeInterval.rc && per.lc))) {
      if(TLA_DEBUG)
        cout<<"per not totally after up"<<endl;
      if(up.timeInterval.start < per.start
       || (up.timeInterval.start == per.start
       && up.timeInterval.lc && !per.lc)) {
        if(TLA_DEBUG)
          cout<<"up starts before per"<<endl;
        newUp.timeInterval = per;
      }
      else {
        if(TLA_DEBUG)
          cout<<"per starts before or with up"<<endl;
        newUp.timeInterval.start = up.timeInterval.start;
        newUp.timeInterval.lc = up.timeInterval.lc;
      }
      while(true) {
        if(up.timeInterval.end < per.end
         || (up.timeInterval.end == per.end
         && per.rc && !up.timeInterval.rc)) {
            if(TLA_DEBUG)
              cout<<"per ends after up (break)"<<endl;
            newUp.timeInterval.end = up.timeInterval.end;
            newUp.timeInterval.rc = up.timeInterval.rc;
            up.TemporalFunction(newUp.timeInterval.start, pt, true);
            newUp.p0 = pt;
            up.TemporalFunction(newUp.timeInterval.end, pt, true);
            newUp.p1 = pt;
            if(TLA_DEBUG){
              cout<<"Add3 ("<<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
              <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
              <<") ["<<newUp.timeInterval.start.ToString()<<" "
              <<newUp.timeInterval.end.ToString()<<" "
              <<newUp.timeInterval.lc<<" "<<newUp.timeInterval.rc<<"]"<<endl;}
            endResult->Add(newUp);
            break;
        }
        else {
          if(TLA_DEBUG)
            cout<<"per ends inside up"<<endl;
          newUp.timeInterval.end = per.end;
          newUp.timeInterval.rc = per.rc;
          up.TemporalFunction(newUp.timeInterval.start, pt, true);
          newUp.p0 = pt;
          up.TemporalFunction(newUp.timeInterval.end, pt,true);
          newUp.p1 = pt;
          if(TLA_DEBUG){
            cout<<"Add4 ("<<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
             <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
            <<") ["<<newUp.timeInterval.start.ToString()<<" "
            <<newUp.timeInterval.end.ToString()<<" "
            <<newUp.timeInterval.lc<<" "<<newUp.timeInterval.rc<<"]"<<endl;}
          endResult->Add(newUp);
        }
        if(m == pResult->GetNoComponents() - 1){
          if(TLA_DEBUG)
            cout<<"last per"<<endl;
          pfinished = true;
        }
        else {
          pResult->Get(++m, per);
          if(TLA_DEBUG){
            cout<<"per "<<m<<" ["<<per.start.ToString()
            <<" "<<per.end.ToString()<<" "<<per.lc<<" "<<per.rc<<"]"<<endl;}
        }
        if(!pfinished && (per.start < up.timeInterval.end
           || (per.start == up.timeInterval.end
           && up.timeInterval.rc && per.rc))){
          if(TLA_DEBUG)
            cout<<"next per starts in same up"<<endl;
          newUp.timeInterval.start = per.start;
          newUp.timeInterval.lc = per.lc;
        }
        else {
          if(TLA_DEBUG)
            cout<<"next interval after up -> finish up"<<endl;
          break;
        }
      } //while
    }
  }
  endResult->EndBulkLoad(false);
}

/*
1.1 Method ~MPointInsidePoints~

Calcultates the periods where the given MPoint lies inside the given Points.
It return the existing intervals in a Periods-Object.

*/
static void MPointInsidePoints( const MPoint& mp, const Points& ps,
                                Periods& pResult)
{
  if(TLA_DEBUG)
    cout<<"MPointPointsInside called"<<endl;
  pResult.Clear();
  if( !mp.IsDefined() || !ps.IsDefined() ){
    pResult.SetDefined( false );
    return;
  }
  pResult.SetDefined( true );

  UPoint up;
  Point p;

  Periods* between = new Periods(0);
  Periods* period = new Periods(0);
  Interval<Instant> newper; //part of the result
  bool newtime;

  for( int i = 0; i < mp.GetNoComponents(); i++)
  {
    mp.Get(i, up);
    if(!up.IsDefined())
        continue;
    for( int n = 0; n < ps.Size(); n++)
    {
      newtime = false;
      ps.Get(n, p);
      if(!p.IsDefined())
        continue;
      double time = MPointInMPoint(up.p0.GetX(), up.p0.GetY(), up.p1.GetX(),
                    up.p1.GetY(), p.GetX(), p.GetY(), p.GetX(), p.GetY());

      if(time == 2.0){
         newper = up.timeInterval;
         newtime = true;
      }
      else if((time  > 0.0 && time < 1.0)
              || (time == 0.0 && up.timeInterval.lc)
              || (time == 1.0 && up.timeInterval.rc)){
        Instant t(instanttype);
        t.ReadFrom((up.timeInterval.end.ToDouble()
                  - up.timeInterval.start.ToDouble()) * time
                  + up.timeInterval.start.ToDouble());
        t.SetType(instanttype);
        newper.start = t;
        newper.end = t;
        newper.lc = true;
        newper.rc = true;
        newtime = true;
      }
      if(TLA_DEBUG){
        cout<<"newper ["<< newper.start.ToString()<<" "
        <<newper.end.ToString()<<" "<<newper.lc<<" "<<newper.rc<<"]"<<endl;}
      if(newtime){
        period->Clear();
        period->StartBulkLoad();
        period->Add(newper);
        period->EndBulkLoad(false);
        if (!pResult.IsEmpty()) {
          between->Clear();
          period->Union(pResult, *between);
          pResult.Clear();
          pResult.CopyFrom(between);
        }
        else
          pResult.CopyFrom(period);
      }
    }
  }
  between->DeleteIfAllowed();
  period->DeleteIfAllowed();
}

/*
1.1 Method ~TransformMBool2MPoint~

Completes a MBool to a MPoint-value for the ~minus~ operator. For this it adds
the starting and end points to every interval when mBool is not true, even when
there is no mBool at all.

*/
static void TransformMBool2MPoint( const MPoint *mp, const MBool *mBool,
                                   MPoint *endResult)
{
  assert( mp );
  assert( mBool );
  assert( endResult );

  endResult->Clear();
  if( !mp->IsDefined() || !mBool->IsDefined() ){
    endResult->SetDefined( false );
    return;
  }
  endResult->SetDefined( true );
  UPoint up;
  endResult->Resize(mBool->GetNoComponents());
  endResult->StartBulkLoad();

  UBool ub;
  UPoint newUp(true);
  Point pt;
  int pos = 0;

  if(TLA_DEBUG)
    cout<<"TransformMBool2MPoint1 called"<<endl;

  for ( int i = 0; i < mBool->GetNoComponents(); i++) {
    mBool->Get(i, ub);
    if(!ub.IsDefined())
        continue;
    if(TLA_DEBUG)
    {
      cout<<"UBool # "<<i<<" ["<<ub.timeInterval.start.ToString()
      <<" "<<ub.timeInterval.end.ToString()<<" "
      <<ub.timeInterval.lc<<" "<<ub.timeInterval.rc<<"] "
      <<ub.constValue.GetBoolval()<<endl;
    }

    if(ub.constValue.GetBoolval())
    {
     if(TLA_DEBUG)
       cout<<"point and mpoint are equal ignore timeInterval"<<endl;

    }
    else
    {
      if(TLA_DEBUG)
        cout<<"mpoint and mpoint are not equal take timeInterval"<<endl;

      mp->Get(pos, up);
      if(TLA_DEBUG)
      {
        cout<<"UPoint # "<<pos<<" ["<<up.timeInterval.start.ToString()
        <<" "<<up.timeInterval.end.ToString()<<" "
        <<up.timeInterval.lc<<" "<<ub.timeInterval.rc<<"] "<<endl;
      }
      while(!(up.timeInterval.end > ub.timeInterval.start
       || (up.timeInterval.end == ub.timeInterval.start
       && up.timeInterval.rc && ub.timeInterval.lc))
       && pos < mp->GetNoComponents())
      {
        pos++;
        mp->Get(pos, up);
        if(TLA_DEBUG)
        {
          cout<<"UPoint # "<<pos<<" ["<<up.timeInterval.start.ToString()
          <<" "<<up.timeInterval.end.ToString()<<" "
          <<up.timeInterval.lc<<" "<<ub.timeInterval.rc<<"] "<<endl;
        }
      }

      if(up.timeInterval.start < ub.timeInterval.start
       || (up.timeInterval.start == ub.timeInterval.start
       && (up.timeInterval.lc
       || (!up.timeInterval.lc && !ub.timeInterval.lc))))
      {   //upoint started before ubool or at same time
        if(up.timeInterval.end > ub.timeInterval.end)
        { //upoint ends after ubool

           newUp.timeInterval = ub.timeInterval;
           up.TemporalFunction(newUp.timeInterval.start, pt, true);
           newUp.p0 = pt;
           up.TemporalFunction(newUp.timeInterval.end, pt, true);
           newUp.p1 = pt;

           if(TLA_DEBUG)
           {
             cout<<"Add1 ("
             <<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
             <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
             <<") ["<<newUp.timeInterval.start.ToString()<<" "
             <<newUp.timeInterval.end.ToString()<<" "
             <<newUp.timeInterval.lc<<" "<<newUp.timeInterval.rc<<"]"<<endl;
           }
           if(newUp.IsValid())
             endResult->Add(newUp);
        }
        else
        {   // upoint ends inside of ubool
          newUp.timeInterval.start = ub.timeInterval.start;
          newUp.timeInterval.lc    = ub.timeInterval.lc;
          newUp.timeInterval.end   = up.timeInterval.end;
          newUp.timeInterval.rc    = up.timeInterval.rc;

          up.TemporalFunction(newUp.timeInterval.start, pt, true);
          newUp.p0 = pt;
          up.TemporalFunction(newUp.timeInterval.end, pt, true);
          newUp.p1 = pt;
          if(TLA_DEBUG)
          {
             cout<<"Add2 ("
             <<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
             <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
             <<") ["<<newUp.timeInterval.start.ToString()<<" "
             <<newUp.timeInterval.end.ToString()<<" "
             <<newUp.timeInterval.lc<<" "<<newUp.timeInterval.rc<<"]"<<endl;
          }
          if(newUp.IsValid())
            endResult->Add(newUp);
          pos++;
          if(pos < mp->GetNoComponents())
            mp->Get(pos, up);
          else
            continue;
          if(TLA_DEBUG)
          {
            cout<<"UPoint # "<<pos<<" ["<<up.timeInterval.start.ToString()
            <<" "<<up.timeInterval.end.ToString()<<" "
            <<up.timeInterval.lc<<" "<<ub.timeInterval.rc<<"] "<<endl;
          }
          while((up.timeInterval.end < ub.timeInterval.end
           || (up.timeInterval.end == ub.timeInterval.end
           && !(!up.timeInterval.rc && ub.timeInterval.rc)))
           && pos < mp->GetNoComponents())
          {  //upoint end before ubool
            newUp.timeInterval = up.timeInterval;
            newUp.p0 = up.p0;
            newUp.p1 = up.p1;
            if(TLA_DEBUG)
            {
              cout<<"Add3 ("
              <<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
              <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
              <<") ["<<newUp.timeInterval.start.ToString()<<" "
              <<newUp.timeInterval.end.ToString()<<" "
              <<newUp.timeInterval.lc<<" "<<newUp.timeInterval.rc<<"]"<<endl;
            }

            if(newUp.IsValid())
              endResult->Add(newUp);

            pos++;
            if(pos < mp->GetNoComponents())
              mp->Get(pos, up);
            else
              continue;
            if(TLA_DEBUG)
            {
              cout<<"UPoint # "<<pos<<" ["<<up.timeInterval.start.ToString()
              <<" "<<up.timeInterval.end.ToString()<<" "
              <<up.timeInterval.lc<<" "<<ub.timeInterval.rc<<"] "<<endl;
            }
          }
        }
      }
    }
  }
  endResult->EndBulkLoad(false);
}

/*
1.1 Method ~TransformMBool2MPoint~

Completes a MBool to a MPoint-value for the ~minus~ operator. For this it adds
the starting and end points to every interval when mBool is not true, even when
there is no mBool at all.

*/
static void TransformMBool2MPoint( const Point *p, const MBool *mBool,
                                   MPoint *endResult)
{
  assert( p );
  assert( mBool );
  assert( endResult );

  endResult->Clear();
  if( !p->IsDefined() || !mBool->IsDefined() ){
    endResult->SetDefined( false );
    return;
  }
  endResult->SetDefined( true );

  endResult->Resize(mBool->GetNoComponents());
  endResult->StartBulkLoad();
  UBool ub;
  UPoint newUp(true);

  if(TLA_DEBUG)
    cout<<"TransformMBool2MPoint2 called"<<endl;
  for ( int i = 0; i < mBool->GetNoComponents(); i++) {
    mBool->Get(i, ub);
    if(!ub.IsDefined())
        continue;
    if(TLA_DEBUG){
      cout<<"UBool # "<<i<<" ["<<ub.timeInterval.start.ToString()
      <<" "<<ub.timeInterval.end.ToString()<<" "
      <<ub.timeInterval.lc<<" "<<ub.timeInterval.rc<<"] "
      <<ub.constValue.GetBoolval()<<endl;}
    if(ub.constValue.GetBoolval()){
     if(TLA_DEBUG)
       cout<<"point and mpoint are equal ignore timeInterval"<<endl;
    }
    else{
      if(TLA_DEBUG)
        cout<<"point and mpoint are not equal take timeInterval"<<endl;
      newUp.timeInterval = ub.timeInterval;
      newUp.p0 = *p;
      newUp.p1 = *p;
      if(TLA_DEBUG){
        cout<<"Add4 ("<<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
        <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
        <<") ["<<newUp.timeInterval.start.ToString()<<" "
        <<newUp.timeInterval.end.ToString()<<" "
        <<newUp.timeInterval.lc<<" "<<newUp.timeInterval.rc<<"]"<<endl;}
      if(newUp.timeInterval.end > newUp.timeInterval.start
       || (newUp.timeInterval.end == newUp.timeInterval.start
       && newUp.timeInterval.lc && newUp.timeInterval.rc))
        endResult->Add(newUp);
    }
  }
  endResult->EndBulkLoad(false);
}

/*
1.1 Method ~MovingBoolMMOperators~

Compares the two operators in the given way: The comparisons are 1: AND; 2: OR.

*/
static void MovingBoolMMOperators( const MBool& op1, const MBool& op2,
                                   MBool& result, int op )
{
  if(TLA_DEBUG)
    cout<<"MovingBoolMMOperators called"<<endl;
  result.Clear();
  if( !op1.IsDefined() || !op2.IsDefined()){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  UBool uBool(true);  //part of the Result

  RefinementPartition<MBool, MBool, UBool, UBool> rp(op1, op2);
  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;
  result.Resize(rp.Size());
  result.StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    UBool u1transfer;
    UBool u2transfer;
    UBool u1(true);
    UBool u2(true);

    rp.Get(i, iv, u1Pos, u2Pos);
    if(TLA_DEBUG){
      cout<< "and/or interval #"<< i<< ": "<< iv.start.ToString()<< " "
      << iv.end.ToString()<< " "<< iv.lc<< " "<< iv.rc<< " "
      << u1Pos<< " "<< u2Pos<< endl;}

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i
        <<" ["<< iv.start.ToString()<< " "<< iv.end.ToString()<< " "
        << iv.lc<< " "<< iv.rc<< " "<< u1Pos<< " "<< u2Pos<< endl;
      op1.Get(u1Pos, u1transfer);
      op2.Get(u2Pos, u2transfer);
      if(!(u1transfer.IsDefined() && u2transfer.IsDefined()))
        continue;
      u1 = u1transfer;
      u2 = u2transfer;
    }
    uBool.timeInterval = iv;

    if (op == 1)//AND
      uBool.constValue.Set(true,u1.constValue.GetBoolval()
                             && u2.constValue.GetBoolval());
    else if (op == 2) //OR
      uBool.constValue.Set(true,u1.constValue.GetBoolval()
                             || u2.constValue.GetBoolval());
    else //should not happen!
      uBool.constValue.Set(true,true);
    if(TLA_DEBUG){
      cout<<"wert "<<uBool.constValue.GetBoolval();
      cout<<" ["<<uBool.timeInterval.start.ToString()
      <<" "<<uBool.timeInterval.end.ToString()<<" "
      <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}

    result.MergeAdd(uBool);
  }
  result.EndBulkLoad(false);
}

/*
1.1 Method ~MovingBoolMSOperators~

Compares the two operators in the given way:  The comparisons are 1: AND; 2: OR.

*/
static void MovingBoolMSOperators(  const MBool& op1, const CcBool& op2,
                                    MBool& result, int op )
{
  if(TLA_DEBUG)
    cout<<"MovingBoolMSOperators called"<<endl;
  result.Clear();
  if( !op1.IsDefined() || !op2.IsDefined()){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );

  UBool uBool(true);  //part of the Result
  UBool u1transfer;

  result.Resize(op1.GetNoComponents());
  result.StartBulkLoad();
  for( int i = 0; i < op1.GetNoComponents(); i++)
  {
    if(TLA_DEBUG)
      cout<<"temporalMSLogic "<<op<<" ,# "<<i<<endl;
    op1.Get(i, u1transfer);
    if(!(u1transfer.IsDefined() && op2.IsDefined()))
        continue;
    uBool = u1transfer;
    if (op == 1)
      uBool.constValue.Set(uBool.constValue.IsDefined(),
      (uBool.constValue.GetBoolval() and op2.GetBoolval()));
    else
      uBool.constValue.Set(uBool.constValue.IsDefined(),
      (uBool.constValue.GetBoolval() or op2.GetBoolval()));
      if(TLA_DEBUG){
        cout<<"wert "<<uBool.constValue.GetBoolval()
        <<" [ "<<uBool.timeInterval.start.ToString()
        <<" "<<uBool.timeInterval.end.ToString()<<" "
        <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}

      result.MergeAdd(uBool);
  }
  result.EndBulkLoad(false);
}


/*
1.1 Methods to compute ~multiplication, division~ of MInt, MReal

*/

void MovingAddMII(MInt* op1, CcInt* op2, MInt* result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingAddMII called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UInt uin(true), ures(true);  //part of the Result
  int val2= op2->GetIntval() * op;

  result->Resize(op1->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op1->GetNoComponents(); i++)
  {
    op1->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined()))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.constValue.Set(true, uin.constValue.GetIntval() + val2);
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingAddMIR(MInt* op1, CcReal* op2, MReal* result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingAddMIR called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UInt uin(true);
  UReal ures(true);  //part of the Result
  double val2= op2->GetRealval() * op;

  result->Resize(op1->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op1->GetNoComponents(); i++)
  {
    op1->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined()))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.a= 0;  ures.b= 0; ures.c= uin.constValue.GetIntval() + val2;
    ures.r= false;
    ures.SetDefined(true);
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingAddMRR(MReal* op1, CcReal* op2, MReal* result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingAddMRR called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UReal uin(true), ures(true);  //part of the Result
  double val2= op2->GetRealval() * op;

  result->Resize(op1->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op1->GetNoComponents(); i++)
  {
    op1->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined() && !uin.r))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.a= uin.a;  ures.b= uin.b; ures.c= uin.c + val2;
    ures.r= uin.r;
    ures.SetDefined(true);
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingAddMRI(MReal* op1, CcInt* op2, MReal* result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingAddMRI called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UReal uin(true), ures(true);  //part of the Result
  int val2= op2->GetIntval()  * op;

  result->Resize(op1->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op1->GetNoComponents(); i++)
  {
    op1->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined() && !uin.r))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.a= uin.a;  ures.b= uin.b; ures.c= uin.c + val2;
    ures.r= uin.r;
    ures.SetDefined(true);
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingAddIMI(CcInt* op1, MInt* op2, MInt* result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingAddIMI called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UInt uin(true), ures(true);  //part of the Result
  int val1= op1->GetIntval();

  result->Resize(op2->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op2->GetNoComponents(); i++)
  {
    op2->Get(i, uin);
    if(!(uin.IsDefined() && op1->IsDefined()))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.constValue.Set(true, val1 + op * uin.constValue.GetIntval());
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}
void MovingAddRMI(CcReal* op1, MInt* op2, MReal* result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingAddRMI called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UInt uin(true);
  UReal ures(true);  //part of the Result
  double val1= op1->GetRealval();

  result->Resize(op2->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op2->GetNoComponents(); i++)
  {
    op2->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined() ))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.a= 0;  ures.b= 0; ures.c= op * val1 - uin.constValue.GetIntval();
    ures.r= false;
    ures.SetDefined(true);
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingAddRMR(CcReal* op1, MReal* op2, MReal* result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingAddRMR called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UReal uin(true);
  UReal ures(true);  //part of the Result
  double val1= op1->GetRealval();

  result->Resize(op2->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op2->GetNoComponents(); i++)
  {
    op2->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined() && !uin.r ))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.a= uin.a;  ures.b= uin.b; ures.c= val1 + op * uin.c;
    ures.r= false;
    ures.SetDefined(true);
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingAddIMR(CcInt* op1, MReal* op2, MReal* result,int op)
{
  if(TLA_DEBUG)
    cout<<"MovingAddIMR called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UReal uin(true);
  UReal ures(true);  //part of the Result
  int val1= op1->GetIntval();

  result->Resize(op2->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op2->GetNoComponents(); i++)
  {
    op2->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined() && !uin.r ))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.a= uin.a;  ures.b= uin.b; ures.c= val1 + op * uin.c;
    ures.r= false;
    ures.SetDefined(true);
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}
void MovingAddMIMI(MInt* op1, MInt* op2, MInt* result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingAddMIMI called"<<endl;

  if( !op1->IsDefined() || !op2->IsDefined() ){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );
  UInt uInt(true);  //part of the Result

  RefinementPartition<MInt, MInt, UInt, UInt> rp(*op1, *op2);

  result->Clear();
  result->StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    UInt u1;
    UInt u2;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<endl;
      op1->Get(u1Pos, u1);
      op2->Get(u2Pos, u2);
      if(!(u1.IsDefined() && u2.IsDefined()))
        continue;
    }
    uInt.timeInterval = iv;

    uInt.constValue.Set(true,
      u1.constValue.GetIntval() + op * u2.constValue.GetIntval());

    result->MergeAdd(uInt);
  }
  result->EndBulkLoad(false);
}

void MovingAddMIMR(MInt* op1, MReal* op2, MReal* result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingAddMIMR called"<<endl;

  if( !op1->IsDefined() || !op2->IsDefined() ){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );
  UReal uReal(true);  //part of the Result

  RefinementPartition<MInt, MReal, UInt, UReal> rp(*op1, *op2);

  result->Clear();
  result->StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    UInt u1;
    UReal u2;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<endl;
      op1->Get(u1Pos, u1);
      op2->Get(u2Pos, u2);
      if(!(u1.IsDefined() && u2.IsDefined() && !u2.r))
        continue;
    }
    uReal.timeInterval = iv;

    uReal.a= op * u2.a;   uReal.b= op * u2.b;
    uReal.c= u1.constValue.GetIntval() + op * u2.c;
    uReal.SetDefined(true);

    result->MergeAdd(uReal);
  }
  result->EndBulkLoad(false);
}

void MovingAddMRMR(MReal* op1, MReal* op2, MReal* result,int op)
{
  if(TLA_DEBUG)
    cout<<"MovingAddMRMR called"<<endl;

  if( !op1->IsDefined() || !op2->IsDefined() ){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );
  UReal uReal(true);  //part of the Result
  uReal.r= false;

  RefinementPartition<MReal, MReal, UReal, UReal> rp(*op1, *op2);

  result->Clear();
  result->StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    UReal u1;
    UReal u2;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<endl;
      op1->Get(u1Pos, u1);
      op2->Get(u2Pos, u2);
      if(!(u1.IsDefined() && u2.IsDefined() && !u1.r && !u2.r))
        continue;
    }
    uReal.timeInterval = iv;

    uReal.a= u1.a + op * u2.a;   uReal.b=  u1.a + op * u2.b;
    uReal.c= u1.c + op * u2.c;
    uReal.SetDefined(true);

    result->MergeAdd(uReal);
  }
  result->EndBulkLoad(false);
}

void MovingAddMRMI(MReal* op1, MInt* op2, MReal* result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingAddMIMR called"<<endl;

  if( !op1->IsDefined() || !op2->IsDefined() ){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );
  UReal uReal(true);  //part of the Result

  RefinementPartition<MReal, MInt, UReal, UInt> rp(*op1, *op2);

  result->Clear();
  result->StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    UReal u1;
    UInt u2;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<endl;
      op1->Get(u1Pos, u1);
      op2->Get(u2Pos, u2);
      if(!(u1.IsDefined() && u2.IsDefined() && !u1.r))
        continue;
    }
    uReal.timeInterval = iv;

    uReal.a= u1.a;   uReal.b= u1.b;
    uReal.c= u1.c + op * u2.constValue.GetIntval();
    uReal.SetDefined(true);

    result->MergeAdd(uReal);
  }
  result->EndBulkLoad(false);
}



void MovingMultiplyMII(MInt* op1, CcInt* op2, MInt* result)
{
  if(TLA_DEBUG)
    cout<<"MovingMultiplyMII called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UInt uin(true), ures(true);  //part of the Result
  int val2= op2->GetIntval();

  result->Resize(op1->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op1->GetNoComponents(); i++)
  {
    op1->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined()))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.constValue.Set(true, uin.constValue.GetIntval() * val2);
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingMultiplyMIR(
    MInt* op1, CcReal* op2, MReal* result, bool inverseSecond)
{
  if(TLA_DEBUG)
    cout<<"MovingMultiplyMIR called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined() ||
      (inverseSecond && AlmostEqual(op2->GetRealval(), 0))){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UInt uin(true);
  UReal ures(true);  //part of the Result
  double val2;
  if(inverseSecond)
    val2= 1.0 / op2->GetRealval() ;
  else
    val2= op2->GetRealval() ;

  result->Resize(op1->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op1->GetNoComponents(); i++)
  {
    op1->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined()))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.a= 0;  ures.b= 0; ures.c= uin.constValue.GetIntval() * val2;
    ures.r= false;
    ures.SetDefined(true);
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingMultiplyMRR(
    MReal* op1, CcReal* op2, MReal* result, bool inverseSecond)
{
  if(TLA_DEBUG)
    cout<<"MovingMultiplyMRR called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()||
      (inverseSecond && AlmostEqual(op2->GetRealval(), 0))){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UReal uin(true), ures(true);  //part of the Result
  double val2;
  if(inverseSecond)
    val2= 1.0 / op2->GetRealval() ;
  else
    val2= op2->GetRealval() ;

  result->Resize(op1->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op1->GetNoComponents(); i++)
  {
    op1->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined() && !uin.r))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.a= uin.a * val2;  ures.b= uin.b * val2; ures.c= uin.c * val2;
    ures.r= uin.r;
    ures.SetDefined(true);
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingMultiplyMRI(
    MReal* op1, CcInt* op2, MReal* result, bool inverseSecond)
{
  if(TLA_DEBUG)
    cout<<"MovingMultiplyMRI called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined() ||
      (inverseSecond && (op2->GetIntval() == 0) )){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UReal uin(true), ures(true);  //part of the Result
  double val2;
  if(inverseSecond)
    val2= 1.0 / op2->GetIntval();
  else
    val2= op2->GetIntval();

  result->Resize(op1->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op1->GetNoComponents(); i++)
  {
    op1->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined() && !uin.r))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.a= uin.a * val2;  ures.b= uin.b * val2; ures.c= uin.c * val2;
    ures.r= uin.r;
    ures.SetDefined(true);
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingMultiplyIMI(CcInt* op1, MInt* op2, MInt* result)
{
  if(TLA_DEBUG)
    cout<<"MovingMultiplyIMI called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UInt uin(true), ures(true);  //part of the Result
  int val1= op1->GetIntval();


  result->Resize(op2->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op2->GetNoComponents(); i++)
  {
    op2->Get(i, uin);
    if(!(uin.IsDefined() && op1->IsDefined()))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.constValue.Set(true, val1 * uin.constValue.GetIntval());
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingMultiplyRMI(
    CcReal* op1, MInt* op2, MReal* result, bool inverseSecond)
{
  if(TLA_DEBUG)
    cout<<"MovingMultiplyRMI called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UInt uin(true);
  UReal ures(true);  //part of the Result
  double val1= op1->GetRealval();

  result->Resize(op2->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op2->GetNoComponents(); i++)
  {
    op2->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined() ))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.a= 0;  ures.b= 0;
    if(inverseSecond)
    {
      if(uin.constValue.GetIntval() == 0)
        ures.SetDefined(false);
      else
      {
        ures.c= val1 / uin.constValue.GetIntval();
        ures.SetDefined(true);
      }
    }
    else
    {
      ures.c= val1 * uin.constValue.GetIntval();
      ures.SetDefined(true);
    }
    ures.r= false;
    if(ures.IsDefined())
      result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingMultiplyRMR(CcReal* op1, MReal* op2, MReal* result)
{
  if(TLA_DEBUG)
    cout<<"MovingMultiplyRMR called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UReal uin(true);
  UReal ures(true);  //part of the Result
  double val1= op1->GetRealval();

  result->Resize(op2->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op2->GetNoComponents(); i++)
  {
    op2->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined() && !uin.r ))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.a= val1 * uin.a;  ures.b= val1 * uin.b;  ures.c= val1 * uin.c;

    ures.r= false;
    ures.SetDefined(true);
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingMultiplyIMR(CcInt* op1, MReal* op2, MReal* result)
{
  if(TLA_DEBUG)
    cout<<"MovingMultiplyIMR called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UReal uin(true);
  UReal ures(true);  //part of the Result
  int val1= op1->GetIntval();

  result->Resize(op2->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op2->GetNoComponents(); i++)
  {
    op2->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined() && !uin.r ))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.a= val1 * uin.a;  ures.b= val1 * uin.b;  ures.c= val1 * uin.c;
    ures.r= false;
    ures.SetDefined(true);
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingMultiplyMIMI(MInt* op1, MInt* op2, MInt* result)
{
  if(TLA_DEBUG)
    cout<<"MovingMultiplyMIMI called"<<endl;

  if( !op1->IsDefined() || !op2->IsDefined() ){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );
  UInt uInt(true);  //part of the Result

  RefinementPartition<MInt, MInt, UInt, UInt> rp(*op1, *op2);

  result->Clear();
  result->StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    UInt u1;
    UInt u2;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<endl;
      op1->Get(u1Pos, u1);
      op2->Get(u2Pos, u2);
      if(!(u1.IsDefined() && u2.IsDefined()))
        continue;
    }
    uInt.timeInterval = iv;

    uInt.constValue.Set(true,
      u1.constValue.GetIntval() * u2.constValue.GetIntval());

    result->MergeAdd(uInt);
  }
  result->EndBulkLoad(false);
}

void MovingMultiplyMIMR(MInt* op1, MReal* op2, MReal* result)
{
  if(TLA_DEBUG)
    cout<<"MovingMultiplyMIMR called"<<endl;

  if( !op1->IsDefined() || !op2->IsDefined() ){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );
  UReal uReal(true);  //part of the Result

  RefinementPartition<MInt, MReal, UInt, UReal> rp(*op1, *op2);

  result->Clear();
  result->StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    UInt u1;
    UReal u2;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<endl;
      op1->Get(u1Pos, u1);
      op2->Get(u2Pos, u2);
      if(!(u1.IsDefined() && u2.IsDefined() && !u2.r))
        continue;
    }
    uReal.timeInterval = iv;

    uReal.a= u1.constValue.GetIntval() * u2.a;
    uReal.b= u1.constValue.GetIntval() * u2.b;
    uReal.c= u1.constValue.GetIntval() * u2.c;
    uReal.SetDefined(true);

    result->MergeAdd(uReal);
  }
  result->EndBulkLoad(false);
}

void MovingMultiplyMRMI(
    MReal* op1, MInt* op2, MReal* result, bool inverseSecond)
{
  if(TLA_DEBUG)
    cout<<"MovingMultiplyMIMR called"<<endl;

  if( !op1->IsDefined() || !op2->IsDefined() ){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );
  UReal uReal(true);  //part of the Result

  RefinementPartition<MReal, MInt, UReal, UInt> rp(*op1, *op2);

  result->Clear();
  result->StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    UReal u1;
    UInt u2;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<endl;
      op1->Get(u1Pos, u1);
      op2->Get(u2Pos, u2);
      if(!(u1.IsDefined() && u2.IsDefined() && !u1.r))
        continue;
    }
    uReal.timeInterval = iv;

    if(inverseSecond)
    {
      if(AlmostEqual(u2.constValue.GetIntval(), 0))
        uReal.SetDefined(false);
      else
      {
        uReal.SetDefined(true);
        uReal.a= u1.a / u2.constValue.GetIntval();
        uReal.b= u1.b / u2.constValue.GetIntval();
        uReal.c= u1.c / u2.constValue.GetIntval();
      }
    }
    else
    {
      uReal.a= u1.a * u2.constValue.GetIntval();
      uReal.b= u1.b * u2.constValue.GetIntval();
      uReal.c= u1.c * u2.constValue.GetIntval();
      uReal.SetDefined(true);
    }

    if(uReal.IsDefined())
      result->MergeAdd(uReal);
  }
  result->EndBulkLoad(false);
}


/*
1.1 Methods to compute ~/~ of MInt, MReal

*/

void MovingDivideMII(MInt* op1, CcInt* op2, MReal* result)
{
  if(TLA_DEBUG)
    cout<<"MovingDivideMII called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UInt uin(true);
  UReal ures(true);  //part of the Result
  int val2= op2->GetIntval();

  result->Resize(op1->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op1->GetNoComponents(); i++)
  {
    op1->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined()))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.a= 0; ures.b= 0;
    ures.c= static_cast<double>(uin.constValue.GetIntval()) / val2;
    ures.SetDefined(true);
    result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingDivideIMI(CcInt* op1, MInt* op2, MReal* result)
{
  if(TLA_DEBUG)
    cout<<"MovingDivideIMI called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  UInt uin(true);
  UReal ures(true);  //part of the Result
  int val1= op1->GetIntval();

  result->Resize(op2->GetNoComponents());
  result->StartBulkLoad();
  for( int i = 0; i < op2->GetNoComponents(); i++)
  {
    op2->Get(i, uin);
    if(!(uin.IsDefined() && op2->IsDefined()))
        continue;
    ures.timeInterval.CopyFrom(uin.timeInterval);
    ures.a= 0; ures.b= 0;
    if(uin.constValue.GetIntval() == 0)
      ures.SetDefined(false);
    else
    {
      ures.c= static_cast<double>(val1) / uin.constValue.GetIntval();
      ures.SetDefined(true);
    }

    if(ures.IsDefined())
      result->MergeAdd(ures);
  }
  result->EndBulkLoad(false);
}

void MovingDivideMIMI(
    MInt* op1, MInt* op2, MReal* result)
{
  if(TLA_DEBUG)
    cout<<"MovingMultiplyMIMR called"<<endl;

  if( !op1->IsDefined() || !op2->IsDefined() ){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );
  UReal uReal(true);  //part of the Result

  RefinementPartition<MInt, MInt, UInt, UInt> rp(*op1, *op2);

  result->Clear();
  result->StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    UInt u1;
    UInt u2;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<endl;
      op1->Get(u1Pos, u1);
      op2->Get(u2Pos, u2);
      if(!(u1.IsDefined() && u2.IsDefined()))
        continue;
    }
    uReal.timeInterval = iv;
    if(u2.constValue.GetIntval() == 0)
      uReal.SetDefined(false);
    else
    {
      uReal.SetDefined(true);
      uReal.a= static_cast<double>(
        u2.constValue.GetIntval()) / u2.constValue.GetIntval();
      uReal.b= static_cast<double>(
        u2.constValue.GetIntval()) / u2.constValue.GetIntval();
      uReal.c= static_cast<double>(
        u2.constValue.GetIntval()) / u2.constValue.GetIntval();
    }
    if(uReal.IsDefined())
      result->MergeAdd(uReal);
  }
  result->EndBulkLoad(false);
}
/*
1.1 Method ~MovingCompareBoolMM~

Compares the two operators in the given way: The comparisons are
-3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/
template <class Mapping1, class Mapping2, class Unit1, class
  Unit2>
static void MovingCompareBoolMM(  const Mapping1& op1, const Mapping2& op2,
                                  MBool& result, int op )
{
  if(TLA_DEBUG)
    cout<<"MovingCompareBoolMM called"<<endl;
  result.Clear();
  if( !op1.IsDefined() || !op2.IsDefined()){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );

  UBool uBool(true);  //part of the Result

  RefinementPartition<Mapping1, Mapping2, Unit1, Unit2>
  rp(op1, op2);
  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;
  result.Resize(rp.Size());
  result.StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;
    Unit1 u1;
    Unit2 u2;

    rp.Get(i, iv, u1Pos, u2Pos);
    if(TLA_DEBUG){
      cout<< "Compare interval #"<< i<< ": "<< iv.start.ToString()<< " "
      << iv.end.ToString()<< " "<< iv.lc<< " "<< iv.rc<< " "
      << u1Pos<< " "<< u2Pos<< endl;}

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<" ["
        << iv.start.ToString()<< " "<< iv.end.ToString()<< " "<< iv.lc
        << "] "<< iv.rc<< " "<< u1Pos<< " "<< u2Pos<< endl;
      op1.Get(u1Pos, u1);
      op2.Get(u2Pos, u2);
      if(!(u1.IsDefined() && u1.IsDefined()))
        continue;
    }
    uBool.timeInterval = iv;
    uBool.constValue.Set(true,CompareValue(u1,u2,op));
    if(TLA_DEBUG){
      cout<<"wert "<<uBool.constValue.GetBoolval();
      cout<<" ["<<uBool.timeInterval.start.ToString()
      <<" "<<uBool.timeInterval.end.ToString()<<" "
      <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
    result.MergeAdd(uBool);
  }
  result.EndBulkLoad(false);
}

/*
1.1 Method ~MovingIntersectionMM~

Calculates the Intersection with op = 1 and the minus-operator with op = 2

*/

template <class Mapping1, class Mapping2, class Unit1,
class Unit2>
static void MovingIntersectionMM( const Mapping1& op1, const Mapping2& op2,
                                  Mapping1& result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingIntersectionMM called"<<endl;
  result.Clear();
  if( !op1.IsDefined() || !op2.IsDefined()){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );

  Unit1 un(true);  //part of the Result

  RefinementPartition<Mapping1, Mapping2, Unit1, Unit2>
  rp(op1, op2);

  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  result.Resize(rp.Size());
  result.StartBulkLoad();

  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;

    int u1Pos;
    int u2Pos;

    Unit1 u1(true);
    Unit2 u2(true);

    Unit1 u1transfer;
    Unit2 u2transfer;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (u1Pos == -1 || u2Pos == -1 )
      continue;

    else {
      if(TLA_DEBUG)
        cout<<"Both operands existant in interval iv #"<<i<<" ["
        << iv.start.ToString()<< " "<< iv.end.ToString()<< " "<< iv.lc
        << " "<< iv.rc<< "] "<< u1Pos<< " "<< u2Pos<< endl;

      op1.Get(u1Pos, u1transfer);
      op2.Get(u2Pos, u2transfer);

      if(!(u1transfer.IsDefined() && u2transfer.IsDefined()))
        continue;

      u1 = u1transfer;
      u2 = u2transfer;

      if ((op == 1 && u1.EqualValue(u2)) || (op == 2
      && !u1.EqualValue(u2))){
        un.constValue = u1.constValue;
        un.timeInterval = iv;

        result.MergeAdd(un);
      }
    }
  }
  result.EndBulkLoad(false);
}


/*
1.1 Method ~MovingCompareBoolMS~

Compares the two operators in the given way:
The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/
template <class Mapping1, class Unit1, class Operator2>
static void MovingCompareBoolMS(  const Mapping1& op1, const Operator2& op2,
                                  MBool& result, int op )
{
  if(TLA_DEBUG)
    cout<<"MovingCompareBoolMS called"<<endl;
  result.Clear();
  if( !op1.IsDefined() || !op2.IsDefined()){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );

  UBool uBool(true);  //part of the Result
  Unit1 u1;

  result.Resize(op1.GetNoComponents());
  result.StartBulkLoad();
  for(int i = 0; i < op1.GetNoComponents(); i++)
  {
     op1.Get(i, u1);
     if(!(u1.IsDefined() && op2.IsDefined()))
        continue;
     uBool.timeInterval = u1.timeInterval;
     uBool.constValue.Set(true,CompareValue(u1,op2,op));
    if(TLA_DEBUG){
      cout<<"erg "<<uBool.constValue.GetBoolval();
      cout<<"interval "<<uBool.timeInterval.start.ToString()
      <<" "<<uBool.timeInterval.end.ToString()<<" "
      <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<endl;}

     result.MergeAdd(uBool);
  }
  result.EndBulkLoad(false);
}

/*
1.1 Method ~MovingAddMS~

Adds/Subtracts {mint, mreal} +/- {real, int}.

*/
template <class Op1M, class Op1U, class Op1S, class Op2S,
  class ResM, class ResU, class ResS>
static void MovingAddMS(  const Op1M* op1, const Op2S* op2,
                                  ResM* result, int op )
{
  if(TLA_DEBUG)
    cout<<"MovingAddMS called"<<endl;
  result->Clear();
  if( !op1->IsDefined() || !op2->IsDefined()){
    result->SetDefined( false );
    return;
  }
  result->SetDefined( true );

  ResU uRes(true);  //part of the Result
  Op1U u1;
  Op1S val1;
  ResS valRes;

  result->Resize(op1->GetNoComponents());
  result->StartBulkLoad();
  for(int i = 0; i < op1->GetNoComponents(); i++)
  {
     op1->Get(i, u1);
     if(!(u1.IsDefined() && op2->IsDefined()))
        continue;
     uRes.timeInterval = u1.timeInterval;
     val1= u1.constValue.GetValue();
     uRes.constValue.Set(true, val1 + op * op2->GetValue());
    if(TLA_DEBUG){
      uRes.Print(cout); cout<<"\n";}

     result->MergeAdd(uRes);
  }
  result->EndBulkLoad(false);
}


/*
1.1 Method ~MRealABS~

Calculates the absolut value of a mReal.

*/
static void MRealABS( const MReal& op, MReal& result)
{
  result.Clear();
  if( !op.IsDefined() ){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );

  vector<UReal> partResult;
  int numPartResult = 0;
  result.Resize(op.GetNoComponents());
  result.StartBulkLoad();
  for(int i = 0; i < op.GetNoComponents(); i++)
  {
    UReal u1;
    op.Get(i, u1);
    numPartResult = u1.Abs(partResult);
    for(int j=0; j<numPartResult; j++)
      result.MergeAdd(partResult[j]);
  }
  result.EndBulkLoad(false);
}


/*
1.1 Method ~copyMRegion~

Copies the MRegion to result.

*/

void copyMRegion(const MRegion& res, MRegion& result) {
   result.CopyFrom(&res);
}

/*
1.1 Method ~copyRegionMPoint~

Transform the region to a mregion and restrics it to deftime(mpoint).

*/

void copyRegionMPoint(const Region& reg, const MPoint& pt, MRegion& result) {
  result.Clear();
  if( !reg.IsDefined() || !pt.IsDefined()){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  MRegion* res = new MRegion(pt, reg);
  result.CopyFrom(res);
  res->DeleteIfAllowed();
}

/*
1.1 Method ~copyMRegionMPoint~

copies the MRegion to result and restrics it to deftime(mpoint).
Method is not implemented complete, because ~restrictMRegion2periods~ is not
implemented yet.

*/

void copyMRegionMPoint(const MRegion& reg, const MPoint& pt, MRegion& result) {
    result.Clear();
    if( !reg.IsDefined() || !pt.IsDefined() ){
      result.SetDefined( false );
      return;
    }
    result.SetDefined( true );

    RefinementPartition<MRegion, MPoint, URegionEmb, UPoint> rp(reg,pt);
    Interval<Instant> iv;
    int regPos;
    int ptPos;
    Periods* per = new Periods(rp.Size());
    Interval<Instant> newper;
    per->Clear();
    per->StartBulkLoad();
    for( unsigned int i = 0; i < rp.Size(); i++ ){
      rp.Get(i, iv, regPos, ptPos);
      if(regPos == -1 || ptPos == -1)
        continue;
      if(TLA_DEBUG){
        cout<<"bothoperators in iv # "<<i<<" [ "
        <<iv.start.ToString()<<" "<<iv.end.ToString()<<" "
        <<iv.lc<<" "<<iv.rc<<" ] regPos "<<regPos<<endl;}
      URegionEmb ureg;
      UPoint up;
      reg.Get(regPos, ureg);
      if(!ureg.IsValid())
        continue;
      pt.Get(ptPos, up);
      if(!up.IsDefined())
        continue;
      newper = iv;
      per->Add(newper);
    }
    per->EndBulkLoad(0);
    Periods* pers = new Periods(0);
    pers->Clear();
    per->Merge(*pers);
    per->DeleteIfAllowed();
    result.Clear();
    //restrictMRegion2periods(reg, pers, result);
    //not possible, because it is not implemented yet.
    pers->DeleteIfAllowed();
}

/*

1 TypeMapping-Functions

10.1 Type mapping function "MBoolTypeMapMBool"

This type mapping function is used for the ~not~ operator.

*/
ListExpr MBoolTypeMapMBool( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if( nl->IsEqual( arg1, MBool::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
10.1 Type mapping function "AndOrTypeMapMBool"

This type mapping function is used for the ~and~ and ~or~ operator.

*/
ListExpr AndOrTypeMapMBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, MBool::BasicType() )
    && nl->IsEqual( arg2, MBool::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MBool::BasicType() )
    && nl->IsEqual( arg2, CcBool::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, CcBool::BasicType() )
    && nl->IsEqual( arg2, MBool::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
10.1 Type mapping function "MovingEqualTypeMapMBool"

This type mapping function is used for the ~\=~ and ~\#~ operator.

*/
ListExpr MovingEqualTypeMapMBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if(TLA_DEBUG)
    cout<<"MovingEqualTypeMapMBool called"<<endl;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, MBool::BasicType() )
     && nl->IsEqual( arg2, MBool::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MBool::BasicType() )
     && nl->IsEqual( arg2, CcBool::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, CcBool::BasicType() )
     && nl->IsEqual( arg2, MBool::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, CcInt::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, CcInt::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MString::BasicType() )
     && nl->IsEqual( arg2, MString::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MString::BasicType() )
     && nl->IsEqual( arg2, CcString::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, CcString::BasicType() )
     && nl->IsEqual( arg2, MString::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, CcReal::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, CcReal::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MPoint::BasicType() )
     && nl->IsEqual( arg2, MPoint::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MPoint::BasicType() )
     && nl->IsEqual( arg2, Point::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, Point::BasicType() )
     && nl->IsEqual( arg2, MPoint::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MRegion::BasicType() )
     && nl->IsEqual( arg2, Region::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, Region::BasicType() )
     && nl->IsEqual( arg2, MRegion::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MRegion::BasicType() )
     && nl->IsEqual( arg2, MRegion::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));

  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
10.1 Type mapping function "MovingCompareTypeMapMBool"

This type mapping function is used for the ~<~, ~<=~, ~<~ and ~>=~ operator.

*/
ListExpr MovingCompareTypeMapMBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, MBool::BasicType() )
     && nl->IsEqual( arg2, MBool::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MBool::BasicType() )
     && nl->IsEqual( arg2, CcBool::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, CcBool::BasicType() )
     && nl->IsEqual( arg2, MBool::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, CcInt::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, CcInt::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, CcReal::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, CcReal::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MString::BasicType() )
     && nl->IsEqual( arg2, MString::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, MString::BasicType() )
     && nl->IsEqual( arg2, CcString::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));
    if( nl->IsEqual( arg1, CcString::BasicType() )
     && nl->IsEqual( arg2, MString::BasicType() ) )
      return (nl->SymbolAtom( MBool::BasicType() ));

  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
10.1 Type mapping function "MovingAddTypeMap"

This type mapping function is used for the ~-~, ~+~ operators.

*/

ListExpr MovingAddTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, CcInt::BasicType() ) )
      return (nl->SymbolAtom( MInt::BasicType() ));

    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, CcReal::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, CcReal::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, CcInt::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, CcInt::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MInt::BasicType() ));

    if( nl->IsEqual( arg1, CcReal::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, CcReal::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, CcInt::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MInt::BasicType() ));

    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
10.1 Type mapping function "MovingMultiplyTypeMap"

This type mapping function is used for the ~multiplication~ operator.

*/

ListExpr MovingMultiplyTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, CcInt::BasicType() ) )
      return (nl->SymbolAtom( MInt::BasicType() ));

    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, CcReal::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, CcReal::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, CcInt::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, CcInt::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MInt::BasicType() ));

    if( nl->IsEqual( arg1, CcReal::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, CcReal::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, CcInt::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MInt::BasicType() ));

    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
10.1 Type mapping function "MovingDivideTypeMap"

This type mapping function is used for the ~/~ operator.

*/

ListExpr MovingDivideTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, CcInt::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, CcReal::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, CcReal::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, CcInt::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, CcInt::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, CcReal::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));

    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return (nl->SymbolAtom( MReal::BasicType() ));
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}


/*
16.1 Type mapping function ~MovingTypeMapeIntime~

It is for the operators ~distance~.

*/
ListExpr
MovingDistanceTypeMapMReal( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, MPoint::BasicType() )
     && nl->IsEqual( arg2, MPoint::BasicType() ) )
      return nl->SymbolAtom( MReal::BasicType() );
    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return nl->SymbolAtom( MReal::BasicType() );
    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, CcReal::BasicType() ) )
      return nl->SymbolAtom( MReal::BasicType() );
    if( nl->IsEqual( arg1, CcReal::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return nl->SymbolAtom( MReal::BasicType() );

  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1 Type mapping function ~MovingIntersectionTypeMap~

It is for the operator ~intersection~.

*/
ListExpr
MovingIntersectionTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, MBool::BasicType() )
     && nl->IsEqual( arg2, MBool::BasicType() ) )
      return nl->SymbolAtom( MBool::BasicType() );

    if( nl->IsEqual( arg1, MBool::BasicType() )
     && nl->IsEqual( arg2, CcBool::BasicType() ) )
      return nl->SymbolAtom( MBool::BasicType() );

    if( nl->IsEqual( arg1, CcBool::BasicType() )
     && nl->IsEqual( arg2, MBool::BasicType() ) )
      return nl->SymbolAtom( MBool::BasicType() );

    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return nl->SymbolAtom( MInt::BasicType() );

    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, CcInt::BasicType() ) )
      return nl->SymbolAtom( MInt::BasicType() );

    if( nl->IsEqual( arg1, CcInt::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return nl->SymbolAtom( MInt::BasicType() );

    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return nl->SymbolAtom( MReal::BasicType() );

    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, CcReal::BasicType() ) )
      return nl->SymbolAtom( MReal::BasicType() );

    if( nl->IsEqual( arg1, CcReal::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return nl->SymbolAtom( MReal::BasicType() );

    if( nl->IsEqual( arg1, MPoint::BasicType() )
     && nl->IsEqual( arg2, Points::BasicType() ) )
      return nl->SymbolAtom( MPoint::BasicType() );

    if( nl->IsEqual( arg1, MPoint::BasicType() )
     && nl->IsEqual( arg2, Line::BasicType() ) )
      return nl->SymbolAtom( MPoint::BasicType() );

    if( nl->IsEqual( arg1, Points::BasicType() )
     && nl->IsEqual( arg2, MPoint::BasicType() ) )
      return nl->SymbolAtom( MPoint::BasicType() );

    if( nl->IsEqual( arg1, MPoint::BasicType() )
     && nl->IsEqual( arg2, MPoint::BasicType() ) )
      return nl->SymbolAtom( MPoint::BasicType() );

    if( nl->IsEqual( arg1, Line::BasicType() )
     && nl->IsEqual( arg2, MPoint::BasicType() ) )
      return nl->SymbolAtom( MPoint::BasicType() );

    if( nl->IsEqual( arg1, MString::BasicType() )
     && nl->IsEqual( arg2, MString::BasicType() ) )
      return nl->SymbolAtom( MString::BasicType() );

    if( nl->IsEqual( arg1, MString::BasicType() )
     && nl->IsEqual( arg2, CcString::BasicType() ) )
      return nl->SymbolAtom( MString::BasicType() );

    if( nl->IsEqual( arg1, CcString::BasicType() )
     && nl->IsEqual( arg2, MString::BasicType() ) )
      return nl->SymbolAtom( MString::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1 Type mapping function ~MovingMinusTypeMap~

It is for the operator ~minus~.

*/
ListExpr
MovingMinusTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, MBool::BasicType() )
     && nl->IsEqual( arg2, MBool::BasicType() ) )
      return nl->SymbolAtom( MBool::BasicType() );
    if( nl->IsEqual( arg1, MBool::BasicType() )
     && nl->IsEqual( arg2, CcBool::BasicType() ) )
      return nl->SymbolAtom( MBool::BasicType() );
    if( nl->IsEqual( arg1, CcBool::BasicType() )
     && nl->IsEqual( arg2, MBool::BasicType() ) )
      return nl->SymbolAtom( MBool::BasicType() );
    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return nl->SymbolAtom( MInt::BasicType() );
    if( nl->IsEqual( arg1, MInt::BasicType() )
     && nl->IsEqual( arg2, CcInt::BasicType() ) )
      return nl->SymbolAtom( MInt::BasicType() );
    if( nl->IsEqual( arg1, CcInt::BasicType() )
     && nl->IsEqual( arg2, MInt::BasicType() ) )
      return nl->SymbolAtom( MInt::BasicType() );
    if( nl->IsEqual( arg1, MReal::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return nl->SymbolAtom( MReal::BasicType() );
    if( nl->IsEqual( arg1, MReal::BasicType() )
    && nl->IsEqual( arg2, CcReal::BasicType() ) )
      return nl->SymbolAtom( MReal::BasicType() );
    if( nl->IsEqual( arg1, CcReal::BasicType() )
     && nl->IsEqual( arg2, MReal::BasicType() ) )
      return nl->SymbolAtom( MReal::BasicType() );
    if( nl->IsEqual( arg1, MPoint::BasicType() )
     && nl->IsEqual( arg2, MPoint::BasicType() ) )
      return nl->SymbolAtom( MPoint::BasicType() );
    if( nl->IsEqual( arg1, MPoint::BasicType() )
     && nl->IsEqual( arg2, Point::BasicType() ) )
      return nl->SymbolAtom( MPoint::BasicType() );
    if( nl->IsEqual( arg1, Point::BasicType() )
     && nl->IsEqual( arg2, MPoint::BasicType() ) )
      return nl->SymbolAtom( MPoint::BasicType() );
    if(nl->IsEqual(nl->First(args), Region::BasicType())
     && nl->IsEqual(nl->Second(args), MPoint::BasicType()))
      return nl->SymbolAtom(MRegion::BasicType());
    if(nl->IsEqual(nl->First(args), MRegion::BasicType())
     && nl->IsEqual(nl->Second(args), Point::BasicType()))
      return nl->SymbolAtom(MRegion::BasicType());
    if(nl->IsEqual(nl->First(args), MRegion::BasicType())
     && nl->IsEqual(nl->Second(args), MPoint::BasicType()))
      return nl->SymbolAtom(MRegion::BasicType());
    if(nl->IsEqual(nl->First(args), MRegion::BasicType())
     && nl->IsEqual(nl->Second(args), Points::BasicType()))
      return nl->SymbolAtom(MRegion::BasicType());
    if(nl->IsEqual(nl->First(args), MRegion::BasicType())
     && nl->IsEqual(nl->Second(args), Line::BasicType()))
      return nl->SymbolAtom(MRegion::BasicType());
    if( nl->IsEqual( arg1, MString::BasicType() )
     && nl->IsEqual( arg2, MString::BasicType() ) )
      return nl->SymbolAtom( MString::BasicType() );
    if( nl->IsEqual( arg1, MString::BasicType() )
     && nl->IsEqual( arg2, CcString::BasicType() ) )
      return nl->SymbolAtom( MString::BasicType() );
    if( nl->IsEqual( arg1, CcString::BasicType() )
     && nl->IsEqual( arg2, MString::BasicType() ) )
      return nl->SymbolAtom( MString::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1 Type mapping function ~InsideTypeMapMBool~

It is for the operator ~inside~

*/
ListExpr
InsideTypeMapMBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

      if ((nl->IsEqual( arg1, MPoint::BasicType() )
       && nl->IsEqual( arg2, Points::BasicType() )))
        return nl->SymbolAtom( MBool::BasicType() );
      if ((nl->IsEqual( arg1, MPoint::BasicType() )
       && nl->IsEqual( arg2, Line::BasicType() )))
        return nl->SymbolAtom( MBool::BasicType() );

      if(nl->IsEqual( arg1, MRegion::BasicType())
       && nl->IsEqual( arg2, Points::BasicType()))
         return nl->SymbolAtom(MBool::BasicType());
      if(nl->IsEqual( arg1, MRegion::BasicType())
       && nl->IsEqual( arg2, Line::BasicType()))
         return nl->SymbolAtom(MBool::BasicType());
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.1 Type mapping function ~PerimeterTypeMap~

Used by ~perimeter~ and ~area~

*/
static ListExpr PerimeterTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 1
     && nl->IsEqual(nl->First(args), MRegion::BasicType()))
        return nl->SymbolAtom(MReal::BasicType());
    else
        return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
16.1 Type mapping function ~RCenterTypeMap~

Used by ~rough\_center~

*/
static ListExpr RCenterTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 1
     && nl->IsEqual(nl->First(args), MRegion::BasicType()))
        return nl->SymbolAtom(MPoint::BasicType());
    else
        return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
16.1 Type mapping function ~NComponentsTypeMap~

Used by ~no\_components~

*/
static ListExpr NComponentsTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 1
     && nl->IsEqual(nl->First(args), MRegion::BasicType()))
        return nl->SymbolAtom(MInt::BasicType());
    else
        return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
16.1 Type mapping function ~UnionTypeMap~

Used by ~union~:

*/
static ListExpr UnionTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 2){
      if(nl->IsEqual(nl->First(args), MPoint::BasicType())
         && nl->IsEqual(nl->Second(args), Region::BasicType()))
         return nl->SymbolAtom(MRegion::BasicType());
      if(nl->IsEqual(nl->First(args), MPoint::BasicType())
         && nl->IsEqual(nl->Second(args), MRegion::BasicType()))
         return nl->SymbolAtom(MRegion::BasicType());
      if(nl->IsEqual(nl->First(args), Point::BasicType())
         && nl->IsEqual(nl->Second(args), MRegion::BasicType()))
         return nl->SymbolAtom(MRegion::BasicType());

      else
        return nl->SymbolAtom(Symbol::TYPEERROR());
    }
    else
        return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
16.1 Type mapping function ~TemporalLiftIsemptyTypeMap~

Used by ~isempty~:

*/
static ListExpr TemporalLiftIsemptyTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 1){
      if(nl->IsEqual(nl->First(args), MRegion::BasicType()))
        return nl->SymbolAtom(MBool::BasicType());
     if(nl->IsEqual(nl->First(args), MBool::BasicType()))
        return nl->SymbolAtom(MBool::BasicType());
     if(nl->IsEqual(nl->First(args), MInt::BasicType()))
        return nl->SymbolAtom(MBool::BasicType());
     if(nl->IsEqual(nl->First(args), MReal::BasicType()))
        return nl->SymbolAtom(MBool::BasicType());
     if(nl->IsEqual(nl->First(args), MPoint::BasicType()))
        return nl->SymbolAtom(MBool::BasicType());
     if(nl->IsEqual(nl->First(args), MString::BasicType()))
        return nl->SymbolAtom(MBool::BasicType());

     else
        return nl->SymbolAtom(Symbol::TYPEERROR());
    }
    else
        return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
16.1 Type mapping function ~periods2mint~

Signature: periods [ x periods] -> mint

*/
static ListExpr periods2mintTM(ListExpr args) {

    string err = "periods [ x periods] expected";
    int len = nl->ListLength(args);
    if((len!=1) && (len!=2)){
       return listutils::typeError(err);
    }
    if(!Periods::checkType(nl->First(args))){
       return listutils::typeError(err);
    }
    if((len==2) && !Periods::checkType(nl->Second(args))){
       return listutils::typeError(err);
    }
    return nl->SymbolAtom(MInt::BasicType());
}

/*
16.1 Type Mapping function ~createmint~

Signature: periods x int -> mint

*/
static ListExpr createmintTM(ListExpr args){

  string err = "periods x int expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " ( wrong number of arguments)");
  }
  if(!Periods::checkType(nl->First(args)) ||
     !CcInt::checkType(nl->Second(args))){
    return listutils::typeError(err );
  }
  return  nl->SymbolAtom(MInt::BasicType());
}



/*
16.1 Type mapping function ~TemporalPlusTypeMap~

Used by ~+~:

*/
static ListExpr TemporalPlusTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), MInt::BasicType())
        && nl->IsEqual(nl->Second(args), MInt::BasicType()))
        return nl->SymbolAtom(MInt::BasicType());
    else
        return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
16.1 Type mapping function ~TemporalZeroTypeMap~

Used by ~zero~:

*/
static ListExpr TemporalZeroTypeMap(ListExpr args) {;

    if (nl->ListLength(args) == 0)
        return nl->SymbolAtom(MInt::BasicType());
    else
        return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
16.1 Type mapping function ~TemporalConcatTypeMap~

Used by ~concat~:

*/
static ListExpr TemporalConcatTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), MPoint::BasicType())
        && nl->IsEqual(nl->Second(args), MPoint::BasicType()) )
        return nl->SymbolAtom(MPoint::BasicType());
    else
        return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
16.1 Type mapping function ~ABSTypeMap~

It is for the operator ~abs~

*/
ListExpr
ABSTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, MReal::BasicType() ))
      return nl->SymbolAtom( MReal::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
1 Selection-Functions

16.2 Selection function ~MovingAndOrSelect~

Is used for the ~and~ and ~or~ operations.

*/
int
MovingAndOrSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == MBool::BasicType()
   && nl->SymbolValue( arg2 ) == MBool::BasicType() )
    return 0;
  if( nl->SymbolValue( arg1 ) == MBool::BasicType()
   && nl->SymbolValue( arg2 ) == CcBool::BasicType() )
    return 1;
  if( nl->SymbolValue( arg1 ) == CcBool::BasicType()
   && nl->SymbolValue( arg2 ) == MBool::BasicType() )
    return 2;

  return -1; // This point should never be reached
}

/*
16.2 Selection function ~MovingEqualSelect~

Is used for the ~mequal~ and ~mnotequal~ operations.

*/
int
MovingEqualSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == MBool::BasicType()
   && nl->SymbolValue( arg2 ) == MBool::BasicType() )
    return 0;
  if( nl->SymbolValue( arg1 ) == MBool::BasicType()
   && nl->SymbolValue( arg2 ) == CcBool::BasicType() )
    return 1;
  if( nl->SymbolValue( arg1 ) == CcBool::BasicType()
   && nl->SymbolValue( arg2 ) == MBool::BasicType() )
    return 2;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 3;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == CcInt::BasicType() )
    return 4;
  if( nl->SymbolValue( arg1 ) == CcInt::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 5;
  if( nl->SymbolValue( arg1 ) == MString::BasicType()
   && nl->SymbolValue( arg2 ) == MString::BasicType() )
    return 6;
  if( nl->SymbolValue( arg1 ) == MString::BasicType()
   && nl->SymbolValue( arg2 ) == CcString::BasicType() )
    return 7;
  if( nl->SymbolValue( arg1 ) == CcString::BasicType()
   && nl->SymbolValue( arg2 ) == MString::BasicType() )
    return 8;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 9;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 10;
  if( nl->SymbolValue( arg1 ) == CcReal::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 11;
  if( nl->SymbolValue( arg1 ) == MPoint::BasicType()
   && nl->SymbolValue( arg2 ) == MPoint::BasicType() )
    return 12;
  if( nl->SymbolValue( arg1 ) == MPoint::BasicType()
   && nl->SymbolValue( arg2 ) == Point::BasicType() )
    return 13;
  if( nl->SymbolValue( arg1 ) == Point::BasicType()
   && nl->SymbolValue( arg2 ) == MPoint::BasicType() )
    return 14;
  if( nl->SymbolValue( arg1 ) == MRegion::BasicType()
   && nl->SymbolValue( arg2 ) == Region::BasicType() )
    return 15;
  if( nl->SymbolValue( arg1 ) == Region::BasicType()
   && nl->SymbolValue( arg2 ) == MRegion::BasicType() )
    return 16;
  if( nl->SymbolValue( arg1 ) == MRegion::BasicType()
   && nl->SymbolValue( arg2 ) == MRegion::BasicType() )
    return 17;

  return -1; // This point should never be reached
}

/*
16.2 Selection function ~MovingCompareSelect~

Is used for the ~mequal~ and ~mnotequal~ operations.

*/
int
MovingCompareSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );
  if( nl->SymbolValue( arg1 ) == MBool::BasicType()
   && nl->SymbolValue( arg2 ) == MBool::BasicType() )
    return 0;
  if( nl->SymbolValue( arg1 ) == MBool::BasicType()
   && nl->SymbolValue( arg2 ) == CcBool::BasicType() )
    return 1;
  if( nl->SymbolValue( arg1 ) == CcBool::BasicType()
   && nl->SymbolValue( arg2 ) == MBool::BasicType() )
    return 2;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 3;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == CcInt::BasicType() )
    return 4;
  if( nl->SymbolValue( arg1 ) == CcInt::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 5;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 6;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 7;
  if( nl->SymbolValue( arg1 ) == CcReal::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 8;
  if( nl->SymbolValue( arg1 ) == MString::BasicType()
   && nl->SymbolValue( arg2 ) == MString::BasicType() )
    return 9;
  if( nl->SymbolValue( arg1 ) == MString::BasicType()
   && nl->SymbolValue( arg2 ) == CcString::BasicType() )
    return 10;
  if( nl->SymbolValue( arg1 ) == CcString::BasicType()
   && nl->SymbolValue( arg2 ) == MString::BasicType() )
    return 11;

  return -1; // This point should never be reached
}

/*
16.2 Selection function ~MovingAddSelect~

Is used for the ~-~ and ~+~ operations.

*/
int
MovingAddSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == CcInt::BasicType() )
    return 0;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 1;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 2;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == CcInt::BasicType() )
    return 3;
  if( nl->SymbolValue( arg1 ) == CcInt::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 4;
  if( nl->SymbolValue( arg1 ) == CcReal::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 5;
  if( nl->SymbolValue( arg1 ) == CcReal::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 6;
  if( nl->SymbolValue( arg1 ) == CcInt::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 7;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 8;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 9;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 10;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 11;

  return -1; // This point should never be reached
}

/*
16.2 Selection function ~MovingMultiplySelect~

Is used for the ~multiplication~ operation.

*/
int
MovingMultiplySelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == CcInt::BasicType() )
    return 0;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 1;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 2;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == CcInt::BasicType() )
    return 3;
  if( nl->SymbolValue( arg1 ) == CcInt::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 4;
  if( nl->SymbolValue( arg1 ) == CcReal::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 5;
  if( nl->SymbolValue( arg1 ) == CcInt::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 6;
  if( nl->SymbolValue( arg1 ) == CcReal::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 7;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 8;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 9;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 10;

  return -1; // This point should never be reached
}

/*
16.2 Selection function ~MovingDivideSelect~

Is used for the ~/~ operation.

*/
int
MovingDivideSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == CcInt::BasicType() )
    return 0;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 1;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 2;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == CcInt::BasicType() )
    return 3;
  if( nl->SymbolValue( arg1 ) == CcInt::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 4;
  if( nl->SymbolValue( arg1 ) == CcReal::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 5;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 6;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 7;

  return -1; // This point should never be reached
}

/*
16.2 Selection function ~MovingDistanceSelect~

Is used for the ~distance~ operation.

*/
int
MovingDistanceSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );
  if( nl->SymbolValue( arg1 ) == MPoint::BasicType()
  && nl->SymbolValue( arg2 ) == MPoint::BasicType() )
    return 0;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 1;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 2;
  if( nl->SymbolValue( arg1 ) == CcReal::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 3;

  return -1; // This point should never be reached
}

/*
16.2 Selection function ~MovingIntersectionSelect~

Is used for the ~intersect~ operation.

*/
int MovingIntersectionSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == MBool::BasicType()
   && nl->SymbolValue( arg2 ) == MBool::BasicType() )
    return 0;
  if( nl->SymbolValue( arg1 ) == MBool::BasicType()
   && nl->SymbolValue( arg2 ) == CcBool::BasicType() )
    return 1;
  if( nl->SymbolValue( arg1 ) == CcBool::BasicType()
   && nl->SymbolValue( arg2 ) == MBool::BasicType() )
    return 2;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 3;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == CcInt::BasicType() )
    return 4;
  if( nl->SymbolValue( arg1 ) == CcInt::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 5;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 6;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 7;
  if( nl->SymbolValue( arg1 ) == CcReal::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 8;
  if( nl->SymbolValue( arg1 ) == MPoint::BasicType()
   && nl->SymbolValue( arg2 ) == Points::BasicType() )
    return 9;
  if( nl->SymbolValue( arg1 ) == MPoint::BasicType()
   && nl->SymbolValue( arg2 ) == Line::BasicType() )
    return 10;
  if( nl->SymbolValue( arg1 ) == Points::BasicType()
   && nl->SymbolValue( arg2 ) == MPoint::BasicType() )
    return 11;
  if( nl->SymbolValue( arg1 ) == Line::BasicType()
   && nl->SymbolValue( arg2 ) == MPoint::BasicType() )
    return 12;
  if( nl->SymbolValue( arg1 ) == MString::BasicType()
   && nl->SymbolValue( arg2 ) == MString::BasicType() )
    return 13;
  if( nl->SymbolValue( arg1 ) == MString::BasicType()
   && nl->SymbolValue( arg2 ) == CcString::BasicType() )
    return 14;
  if( nl->SymbolValue( arg1 ) == CcString::BasicType()
   && nl->SymbolValue( arg2 ) == MString::BasicType() )
    return 15;
  if( nl->SymbolValue( arg1 ) == MPoint::BasicType()
   && nl->SymbolValue( arg2 ) == MPoint::BasicType() )
    return 16;

  return -1; // This point should never be reached
}

/*
16.2 Selection function ~MovingMinusSelect~

Is used for the ~minus~ operation.

*/
int MovingMinusSelect( ListExpr args )
{
  if (nl->ListLength(args) != 2)
  return -1;

  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == MBool::BasicType()
   && nl->SymbolValue( arg2 ) == MBool::BasicType() )
    return 0;
  if( nl->SymbolValue( arg1 ) == MBool::BasicType()
   && nl->SymbolValue( arg2 ) == CcBool::BasicType() )
    return 1;
  if( nl->SymbolValue( arg1 ) == CcBool::BasicType()
   && nl->SymbolValue( arg2 ) == MBool::BasicType() )
    return 2;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 3;
  if( nl->SymbolValue( arg1 ) == MInt::BasicType()
   && nl->SymbolValue( arg2 ) == CcInt::BasicType() )
    return 4;
  if( nl->SymbolValue( arg1 ) == CcInt::BasicType()
   && nl->SymbolValue( arg2 ) == MInt::BasicType() )
    return 5;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 6;
  if( nl->SymbolValue( arg1 ) == MReal::BasicType()
   && nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 7;
  if( nl->SymbolValue( arg1 ) == CcReal::BasicType()
   && nl->SymbolValue( arg2 ) == MReal::BasicType() )
    return 8;
  if( nl->SymbolValue( arg1 ) == MPoint::BasicType()
   && nl->SymbolValue( arg2 ) == MPoint::BasicType() )
    return 9;
  if( nl->SymbolValue( arg1 ) == MPoint::BasicType()
   && nl->SymbolValue( arg2 ) == Point::BasicType() )
    return 10;
  if( nl->SymbolValue( arg1 ) == Point::BasicType()
   && nl->SymbolValue( arg2 ) == MPoint::BasicType() )
    return 11;
  if(nl->SymbolValue(nl->First(args)) == Region::BasicType()
   && nl->SymbolValue(nl->Second(args)) == MPoint::BasicType())
    return 12;
  if  (nl->SymbolValue(nl->First(args)) == MRegion::BasicType()
   && nl->SymbolValue(nl->Second(args)) == Point::BasicType())
    return 13;
  if  (nl->SymbolValue(nl->First(args)) == MRegion::BasicType()
   && nl->SymbolValue(nl->Second(args)) == MPoint::BasicType())
    return 14;
  if  (nl->SymbolValue(nl->First(args)) == MRegion::BasicType()
   && nl->SymbolValue(nl->Second(args)) == Points::BasicType())
    return 15;
  if  (nl->SymbolValue(nl->First(args)) == MRegion::BasicType()
   && nl->SymbolValue(nl->Second(args)) == Line::BasicType())
    return 16;
  if( nl->SymbolValue( arg1 ) == MString::BasicType()
   && nl->SymbolValue( arg2 ) == MString::BasicType() )
    return 17;
  if( nl->SymbolValue( arg1 ) == MString::BasicType()
   && nl->SymbolValue( arg2 ) == CcString::BasicType() )
    return 18;
  if( nl->SymbolValue( arg1 ) == CcString::BasicType()
   && nl->SymbolValue( arg2 ) == MString::BasicType() )
    return 19;

  return -1; // This point should never be reached
}

/*
16.2. Selection function ~InsideSelect~

Is used for the ~inside~ operation.

*/
int
InsideSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );
  if (nl->ListLength(args) == 2){

    if( nl->SymbolValue( arg1 ) == MPoint::BasicType()
     && nl->SymbolValue( arg2 ) == Points::BasicType() )
      return 0;
    if( nl->SymbolValue( arg1 ) == MPoint::BasicType()
     && nl->SymbolValue( arg2 ) == Line::BasicType() )
      return 1;
    if  (nl->SymbolValue(nl->First(args)) == MRegion::BasicType()
     && nl->SymbolValue(nl->Second(args)) == Points::BasicType())
      return 2;
    if  (nl->SymbolValue(nl->First(args)) == MRegion::BasicType()
     && nl->SymbolValue(nl->Second(args)) == Line::BasicType())
      return 3;

    return -1; // This point should never be reached
  }
  else
    return -1;
}

/*
16.2 Selection function ~UnionSelect~

Is used for the ~union~ operation.

*/
static int UnionSelect(ListExpr args) {

    if (nl->ListLength(args) == 2){
      if(nl->SymbolValue(nl->First(args)) == MPoint::BasicType()
       && nl->SymbolValue(nl->Second(args)) == Region::BasicType())
        return 0;
      else if  (nl->SymbolValue(nl->First(args)) == MPoint::BasicType()
       && nl->SymbolValue(nl->Second(args)) == MRegion::BasicType())
        return 1;
      else if  (nl->SymbolValue(nl->First(args)) == Point::BasicType()
       && nl->SymbolValue(nl->Second(args)) == MRegion::BasicType())
        return 2;

      else
        return -1;
    }
    else
        return -1;
}

/*
16.2 Selection function ~TemporalLiftIsemptySelect~

Is used for the ~isempty~ operation.

*/
static int TemporalLiftIsemptySelect(ListExpr args) {

    if (nl->ListLength(args) == 1){
      if(nl->SymbolValue(nl->First(args)) == MRegion::BasicType())
        return 0;
      else if  (nl->SymbolValue(nl->First(args)) == MBool::BasicType())
        return 1;
      else if  (nl->SymbolValue(nl->First(args)) == MInt::BasicType())
        return 2;
      else if  (nl->SymbolValue(nl->First(args)) == MReal::BasicType())
        return 3;
      else if  (nl->SymbolValue(nl->First(args)) == MPoint::BasicType())
        return 4;
      else if  (nl->SymbolValue(nl->First(args)) == MString::BasicType())
        return 5;

      else
        return -1;
    }
    else
        return -1;
}

/*
1 ValueMapping-Functions

16.3 Value mapping functions of operator ~\=~, ~<~, ~<\=~, ~>\=~,~>~
and ~\#~ for two mbool, mint and mstring

*/
template<class Mapping1, class Mapping2, class Unit1, class Unit2, int op>
int TemporalMMCompare( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingCompareBoolMM<Mapping1, Mapping2, Unit1, Unit2>
  ( *((Mapping1*)args[0].addr), *((Mapping2*)args[1].addr),
   *((MBool*)result.addr), op);

  return 0;
}

/*
16.3 Value mapping functions of operator ~\=~, ~<~, ~<\=~, ~>\=~,~>~
and ~\#~ for mreal/mreal

*/
template<int op>
int TemporalMMRealCompare( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingRealCompareMM2(*((MReal*)args[0].addr),
   *((MReal*)args[1].addr), *((MBool*)result.addr), op);

  return 0;
}

/*
16.3 Value mapping functions of operator ~\=~, ~<~, ~<\=~, ~>\=~,~>~
and ~\#~ for mreal/real

*/
template<int op>
int TemporalMSRealCompare( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingRealCompareMS(*((MReal*)args[0].addr),
   *((CcReal*)args[1].addr), *((MBool*)result.addr), op, true);

  return 0;
}

/*
16.3 Value mapping functions of operator ~\=~, ~<~, ~<\=~, ~>\=~,~>~
and ~\#~ for real/mreal

*/
template<int op>
int TemporalSMRealCompare( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  //int newop = op;
  //if (op > -3 && op < 3) newop = -op;

  MovingRealCompareMS(*((MReal*)args[1].addr),
   *((CcReal*)args[0].addr), *((MBool*)result.addr), op, false);

  return 0;
}

/*
16.35 Value mapping functions of operator ~=~ and ~\#~ for mpoint/mpoint

*/
template<int op>
int TemporalMMPointCompare( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingPointCompareMM(*((MPoint*)args[0].addr),
   *((MPoint*)args[1].addr), *((MBool*)result.addr), op);

  return 0;
}

/*
16.3 Value mapping functions of operator ~=~ and ~\#~ for mpoint/point

*/
template<int op>
int TemporalMSPointCompare( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingPointCompareMS(*((MPoint*)args[0].addr),
   *((Point*)args[1].addr), *((MBool*)result.addr), op);

  return 0;
}

/*
16.3 Value mapping functions of operator ~=~ and ~\#~ for point/mpoint

*/
template<int op>
int TemporalSMPointCompare( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingPointCompareMS(*((MPoint*)args[1].addr),
   *((Point*)args[0].addr), *((MBool*)result.addr), op);

  return 0;
}

/*
16.3 Value mapping functions of operator ~=~ and ~\#~ for mregion/region

*/
template<int op>
int TemporalMSRegionCompare( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingRegionCompareMS((MRegion*)args[0].addr,
   (Region*)args[1].addr, (MBool*)result.addr, op);

  return 0;
}

/*
16.3 Value mapping functions of operator ~=~ and ~\#~ for region/mregion

*/
template<int op>
int TemporalSMRegionCompare( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingRegionCompareMS((MRegion*)args[1].addr,
   (Region*)args[0].addr, (MBool*)result.addr, op);

  return 0;
}

/*
16.3 Value mapping functions of operator ~=~ and ~\#~ for mregion/mregion

*/
template<int op>
int TemporalMMRegionCompare( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingRegionCompareMM((MRegion*)args[0].addr,
   (MRegion*)args[1].addr, (MBool*)result.addr, op);

  return 0;
}

/*
16.3 Value mapping functions of operators ~\=~, ~<~, ~<\=~, ~>\=~,~>~ and ~\#~
for mbool/bool, mint/int and mstring/string

*/
template<class Mapping1, class Unit1, class Operator2, int op>
int TemporalMSCompare( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingCompareBoolMS<Mapping1, Unit1, Operator2>
  ( *((Mapping1*)args[0].addr), *((Operator2*)args[1].addr),
   *((MBool*)result.addr), op);

  return 0;
}

/*
16.3 Value mapping functions of operators ~\=~, ~<~, ~<\=~, ~>\=~,~>~ and ~\#~
for bool/mbool, int/mint and string/mstring

*/
template<class Mapping1, class Unit1, class Operator2, int op>
int TemporalSMCompare( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  int newop = op;
  if (op > -3 && op < 3) newop = -op;

  MovingCompareBoolMS<Mapping1, Unit1, Operator2>
  ( *((Mapping1*)args[1].addr), *((Operator2*)args[0].addr),
   *((MBool*)result.addr), newop);

  return 0;
}


/*
16.3 Value mapping functions of operators ~-~, ~+~
for {mint, mreal}

*/
template<int op>
int TemporalMIIAdd( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingAddMII(
      static_cast<MInt*>(args[0].addr), static_cast<CcInt*>(args[1].addr),
      static_cast<MInt*>(result.addr), op);

  return 0;
}

template<int op>
int TemporalMIRAdd( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingAddMIR(
      static_cast<MInt*>(args[0].addr), static_cast<CcReal*>(args[1].addr),
      static_cast<MReal*>(result.addr), op);

  return 0;
}

template<int op>
int TemporalMRRAdd( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingAddMRR(
      static_cast<MReal*>(args[0].addr), static_cast<CcReal*>(args[1].addr),
      static_cast<MReal*>(result.addr), op);

  return 0;
}

template<int op>
int TemporalMRIAdd( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingAddMRI(
      static_cast<MReal*>(args[0].addr), static_cast<CcInt*>(args[1].addr),
      static_cast<MReal*>(result.addr), op);

  return 0;
}

template<int op>
int TemporalIMIAdd( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingAddIMI(
      static_cast<CcInt*>(args[0].addr), static_cast<MInt*>(args[1].addr),
      static_cast<MInt*>(result.addr), op);

  return 0;
}

template<int op>
int TemporalRMIAdd( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingAddRMI(
      static_cast<CcReal*>(args[0].addr), static_cast<MInt*>(args[1].addr),
      static_cast<MReal*>(result.addr), op);

  return 0;
}

template<int op>
int TemporalRMRAdd( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingAddRMR(
      static_cast<CcReal*>(args[0].addr), static_cast<MReal*>(args[1].addr),
      static_cast<MReal*>(result.addr), op);

  return 0;
}

template<int op>
int TemporalIMRAdd( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingAddIMR(
      static_cast<CcInt*>(args[0].addr), static_cast<MReal*>(args[1].addr),
      static_cast<MReal*>(result.addr), op);

  return 0;
}

template<int op>
int TemporalMIMIAdd( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingAddMIMI(
      static_cast<MInt*>(args[0].addr), static_cast<MInt*>(args[1].addr),
      static_cast<MInt*>(result.addr), op);

  return 0;
}

template<int op>
int TemporalMIMRAdd( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingAddMIMR(
      static_cast<MInt*>(args[0].addr), static_cast<MReal*>(args[1].addr),
      static_cast<MReal*>(result.addr), op);

  return 0;
}

template<int op>
int TemporalMRMRAdd( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingAddMRMR(
      static_cast<MReal*>(args[0].addr), static_cast<MReal*>(args[1].addr),
      static_cast<MReal*>(result.addr), op);

  return 0;
}

template<int op>
int TemporalMRMIAdd( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingAddMRMI(
      static_cast<MReal*>(args[0].addr), static_cast<MInt*>(args[1].addr),
      static_cast<MReal*>(result.addr), op);

  return 0;
}

/*
16.3 Value mapping functions of the multiplication and division operators
for {mint, mreal}

*/
int TemporalMIIMultiply( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingMultiplyMII(
      static_cast<MInt*>(args[0].addr), static_cast<CcInt*>(args[1].addr),
      static_cast<MInt*>(result.addr));

  return 0;
}

template<bool inverseSecond>
int TemporalMIRMultiply( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingMultiplyMIR(
      static_cast<MInt*>(args[0].addr), static_cast<CcReal*>(args[1].addr),
      static_cast<MReal*>(result.addr), inverseSecond);

  return 0;
}

template<bool inverseSecond>
int TemporalMRRMultiply( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingMultiplyMRR(
      static_cast<MReal*>(args[0].addr), static_cast<CcReal*>(args[1].addr),
      static_cast<MReal*>(result.addr), inverseSecond);

  return 0;
}

template<bool inverseSecond>
int TemporalMRIMultiply( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingMultiplyMRI(
      static_cast<MReal*>(args[0].addr), static_cast<CcInt*>(args[1].addr),
      static_cast<MReal*>(result.addr), inverseSecond);

  return 0;
}

int TemporalIMIMultiply( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingMultiplyIMI(
      static_cast<CcInt*>(args[0].addr), static_cast<MInt*>(args[1].addr),
      static_cast<MInt*>(result.addr));

  return 0;
}

template<bool inverseSecond>
int TemporalRMIMultiply( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingMultiplyRMI(
      static_cast<CcReal*>(args[0].addr), static_cast<MInt*>(args[1].addr),
      static_cast<MReal*>(result.addr), inverseSecond);

  return 0;
}

int TemporalIMRMultiply( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingMultiplyIMR(
      static_cast<CcInt*>(args[0].addr), static_cast<MReal*>(args[1].addr),
      static_cast<MReal*>(result.addr));

  return 0;
}

int TemporalRMRMultiply( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingMultiplyRMR(
      static_cast<CcReal*>(args[0].addr), static_cast<MReal*>(args[1].addr),
      static_cast<MReal*>(result.addr));

  return 0;
}

int TemporalMIMIMultiply( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingMultiplyMIMI(
      static_cast<MInt*>(args[0].addr), static_cast<MInt*>(args[1].addr),
      static_cast<MInt*>(result.addr));

  return 0;
}

int TemporalMIMRMultiply( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingMultiplyMIMR(
      static_cast<MInt*>(args[0].addr), static_cast<MReal*>(args[1].addr),
      static_cast<MReal*>(result.addr));

  return 0;
}

template<bool inverseSecond>
int TemporalMRMIMultiply( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingMultiplyMRMI(
      static_cast<MReal*>(args[0].addr), static_cast<MInt*>(args[1].addr),
      static_cast<MReal*>(result.addr), inverseSecond);

  return 0;
}

/*
16.3 Value mapping functions of operator ~/~
for {mint, mreal}

*/
int TemporalMIIDivide( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingDivideMII(
      static_cast<MInt*>(args[0].addr), static_cast<CcInt*>(args[1].addr),
      static_cast<MReal*>(result.addr));

  return 0;
}

int TemporalIMIDivide( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingDivideIMI(
      static_cast<CcInt*>(args[0].addr), static_cast<MInt*>(args[1].addr),
      static_cast<MReal*>(result.addr));

  return 0;
}

int TemporalMIMIDivide( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingDivideMIMI(
      static_cast<MInt*>(args[0].addr), static_cast<MInt*>(args[1].addr),
      static_cast<MReal*>(result.addr));

  return 0;
}

/*
1.1 ValueMapping of operator ~isempty~ for mbool, mint, mreal, mpoint
and mregion

*/
template<class Mapping1, class Unit1>
int IsEmptyValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {
    if(TLA_DEBUG)
      cout<< "IsEmptyValueMap() called" << endl;

    result = qp->ResultStorage(s);
    MBool* pResult = (MBool*)result.addr;
    Mapping1* reg = (Mapping1*)args[0].addr;
    pResult->Clear();
    if( !reg->IsDefined() ){
      pResult->SetDefined( false );
      return 0;
    }
    pResult->SetDefined( true );

    UBool uBool(true);
    Unit1 ureg;
    pResult->StartBulkLoad();
    if(reg->GetNoComponents() < 1){
      uBool.timeInterval.lc = true;
      uBool.timeInterval.start.ToMinimum();
      uBool.timeInterval.start.SetType(instanttype);
      uBool.timeInterval.rc = true;
      uBool.timeInterval.end.ToMaximum();
      uBool.timeInterval.end.SetType(instanttype);
      uBool.constValue.Set(true,true);
      pResult->Add(uBool);
    }
    else{
      uBool.timeInterval.lc = true;
      uBool.timeInterval.start.ToMinimum();
      uBool.timeInterval.start.SetType(instanttype);
      for( int i = 0; i < reg->GetNoComponents(); i++) {
        reg->Get(i, ureg);
        if(!ureg.IsValid())
        continue;

        uBool.timeInterval.rc = !ureg.timeInterval.lc;
        uBool.timeInterval.end = ureg.timeInterval.start;
        uBool.constValue.Set(true,true);

        if(uBool.timeInterval.start < uBool.timeInterval.end
         || (uBool.timeInterval.start == uBool.timeInterval.end
         && uBool.timeInterval.lc && uBool.timeInterval.rc))
          pResult->MergeAdd(uBool);
        uBool.timeInterval = ureg.timeInterval;
        uBool.constValue.Set(true,false);

        pResult->MergeAdd(uBool);
        uBool.timeInterval.lc = !ureg.timeInterval.rc;
        uBool.timeInterval.start = ureg.timeInterval.end;
      }
      uBool.timeInterval.end.ToMaximum();
      uBool.timeInterval.end.SetType(instanttype);
      if(ureg.timeInterval.end < uBool.timeInterval.end){
        uBool.timeInterval.rc = true;
        uBool.constValue.Set(true,true);

        pResult->MergeAdd(uBool);;
      }
    }
    pResult->EndBulkLoad(false);

    return 0;
}

/*
16.3 Value mapping of operator ~inside~ and ~intersection~ for mpoint/points.

op decides whether it means inside(1) or intersection(2)

*/
template<int op>
int MPointPointsInside( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods* pResult = new Periods(0);

  MPointInsidePoints( *((MPoint*)args[0].addr),
    *((Points*)args[1].addr), *pResult);

  if(op == 1){ //create a MBool (inside)
    MBool* endResult = (MBool*)result.addr;
    CompletePeriods2MBool(((MPoint*)args[0].addr), pResult, endResult);
  }
  else{ //create a MPoint (intersection)
    MPoint* endResult = (MPoint*)result.addr;
    CompletePeriods2MPoint(((MPoint*)args[0].addr), pResult, endResult);
  }

  pResult->DeleteIfAllowed();

  return 0;
}

/*
16.3 Value mapping of operator ~inside~ and ~intersection~ for mpoint/line.

op decides whether it means inside(1) or intersection(2)

*/
template<int op>
int MPointLineInside( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods* pResult = new Periods(0);

  MPointInsideLine( *((MPoint*)args[0].addr),
    *((Line*)args[1].addr), *pResult);

  if(op == 1) { //create a MBool (inside)
    MBool* endResult = (MBool*)result.addr;
    CompletePeriods2MBool(((MPoint*)args[0].addr), pResult, endResult);
  }
  else { //create a MPoint (intersection)
    MPoint* endResult = (MPoint*)result.addr;
    CompletePeriods2MPoint(((MPoint*)args[0].addr), pResult, endResult);
  }

  pResult->DeleteIfAllowed();

  return 0;
}

/*
1.1 ValueMapping ~MFalseValueMap~ is used in ~inside~ with mregion x points
/ line

Creates a FALSE-unit for every defined interval within the mregion.

*/
int MFalseValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {
    if(TLA_DEBUG)
      cout<< "MFalseValueMap() called" << endl;

    result = qp->ResultStorage(s);
    MBool* pResult = (MBool*)result.addr;
    MRegion* reg = (MRegion*)args[0].addr;
    pResult->Clear();
    if( !reg->IsDefined() ){
      pResult->SetDefined( false );
      return 0;
    }
    pResult->SetDefined( true );
    UBool uBool(true);

    pResult->StartBulkLoad();
    for( int i = 0; i < reg->GetNoComponents(); i++) {
      URegionEmb ureg;
      reg->Get(i, ureg);
      if(!ureg.IsValid())
        continue;
      uBool.timeInterval = ureg.timeInterval;
      uBool.constValue.Set(true,false);
      pResult->MergeAdd(uBool);
    }
    pResult->EndBulkLoad(false);

    return 0;
}

/*
16.3 Value mapping of operator ~intersection~ for points/mpoint

*/
int PointsMPointIntersection( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods* pResult = new Periods(0);

  MPointInsidePoints( *((MPoint*)args[1].addr),
    *((Points*)args[0].addr), *pResult);

  //create a MPoint (intersection)
  MPoint* endResult = (MPoint*)result.addr;
  CompletePeriods2MPoint(((MPoint*)args[1].addr), pResult, endResult);

  pResult->DeleteIfAllowed();

  return 0;
}

/*
16.3 Value mapping of operator ~intersection~ for line/mpoint

*/
int LineMPointIntersection( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods* pResult = new Periods(0);

  MPointInsideLine( *((MPoint*)args[1].addr),
    *((Line*)args[0].addr), *pResult);

  //create a MPoint (intersection)
  MPoint* endResult = (MPoint*)result.addr;
  CompletePeriods2MPoint(((MPoint*)args[1].addr), pResult, endResult);

  pResult->DeleteIfAllowed();

  return 0;
}

/*
1.1 ValueMapping of operator ~Union~ with Region/MPoint

*/
int MPRUnionValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {

    result = qp->ResultStorage(s);

    copyRegionMPoint(*((Region*)args[1].addr) ,
    *((MPoint*)args[0].addr), *((MRegion*)result.addr) );

    return 0;
}

/*
1.1 ValueMapping of operator ~Union~ with MRegion/Point

*/
int PMRUnionValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {

    result = qp->ResultStorage(s);

    copyMRegion(*((MRegion*)args[1].addr), *(MRegion*)result.addr);

    return 0;
}

/*
1.1 ValueMapping of operator ~Union~ with MRegion/MPoint

*/
int MPMRUnionValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {

    result = qp->ResultStorage(s);

    copyMRegionMPoint(*((MRegion*)args[1].addr) ,
    *((MPoint*)args[0].addr), *((MRegion*)result.addr) );

    return 0;
}

/*
16.3 Value mapping functions of operators ~intersection~ and
~minus~ for mbool/bool, mint/int and mstring/string

*/
template<class Mapping1, class Unit1, class Operator2, int op>
int TemporalMSIntersection( Word* args, Word& result, int message,
Word& local, Supplier s )
{
  if(TLA_DEBUG)
    cout<<"TemporalMSIntersection called"<<endl;
  result = qp->ResultStorage( s );
  Mapping1 *mop1 = (Mapping1*)args[0].addr;
  Operator2 *constop = (Operator2*)args[1].addr;
  Mapping1* res = static_cast<Mapping1*>(result.addr);

  res->Clear();
  if( !mop1->IsDefined() || !constop->IsDefined() ){
    res->SetDefined( false );
    return 0;
  }
  res->SetDefined( true );
  Mapping1 *mop2 = new Mapping1(0);
  mop2->SetDefined( true );
  Unit1 up1;
  mop2->StartBulkLoad();
  for (int i = 0; i < mop1->GetNoComponents(); i++) {
    mop1->Get(i, up1);
    if(!up1.IsDefined())
        continue;
    Unit1 *up2 = new Unit1(up1.timeInterval, *constop);
    mop2->Add(*up2);
    delete up2;
  }
  mop2->EndBulkLoad(false);
  MovingIntersectionMM<Mapping1, Mapping1, Unit1, Unit1>(*mop1,*mop2,*res,op);
  mop2->DeleteIfAllowed();

  return 0;
}

/*
16.3 Value mapping functions of operators ~intsersection~ and ~minus~
for mreal/real

*/
template<int op>
int TemporalMSRealIntercept( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  if(TLA_DEBUG)
    cout<<"TemporalMSRealIntercept called"<<endl;
  result = qp->ResultStorage( s );
  MReal*  mop1    = static_cast<MReal*>(args[0].addr);
  CcReal* constop = static_cast<CcReal*>(args[1].addr);
  MReal*  res     = static_cast<MReal*>(result.addr);
  res->Clear();
  if( !mop1->IsDefined() || !constop->IsDefined() ){
    res->SetDefined( false );
    return 0;
  }
  res->SetDefined( true );

  UReal up1;
  MReal *mop2= new MReal(0);
  mop2->Clear();
  mop2->StartBulkLoad();
  for (int i = 0; i < mop1->GetNoComponents(); i++) {
      mop1->Get(i, up1);
      if(!up1.IsDefined())
          continue;
      UReal *up2 = new UReal(up1.timeInterval, 0.0, 0.0, (up1.r) ?
         pow(constop->GetRealval(), 2) : constop->GetRealval(),up1.r);

      mop2->Add(*up2);
      delete up2;
  }
  mop2->EndBulkLoad(false);
  MovingRealIntersectionMM( *mop1, *mop2, *res, op);
  mop2->DeleteIfAllowed();

  return 0;
}

/*
16.3 Value mapping functions of operators ~intsersection~ and ~minus~
for two mbool, mint and mstring

*/
template<class Mapping1, class Mapping2, class Unit1, class Unit2, int op>
int TemporalMMIntersection( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingIntersectionMM<Mapping1, Mapping2, Unit1, Unit2>
  ( *((Mapping1*)args[0].addr), *((Mapping2*)args[1].addr),
   *((Mapping1*)result.addr), op);

  return 0;
}

// value mapping for intersection: mpoint x mpoint --> mpoint
int TemporalMPointMPointIntersection( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  if(TLA_DEBUG)
    cout<<"TemporalMPointMPointIntersection called"<<endl;

  result = (qp->ResultStorage( s ));
  MPoint *op1 = (MPoint*) args[0].addr;
  MPoint *op2 = (MPoint*) args[1].addr;
  MPoint *res = (MPoint*) result.addr;
  res->Clear();
  if( !op1->IsDefined() || !op2->IsDefined() ){
    res->SetDefined( false );
    return 0;
  }
  res->SetDefined( true );
  UPoint resunit( false );
  UPoint lastresunit( false );

  RefinementPartition<MPoint, MPoint, UPoint, UPoint> rp(*op1, *op2);

  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  res->StartBulkLoad();

  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant> iv;
    int u1Pos;
    int u2Pos;

    UPoint u1( true );
    UPoint u2( true );

    UPoint u1transfer;
    UPoint u2transfer;

    rp.Get(i, iv, u1Pos, u2Pos);
    assert( iv.IsValid() );

    if (TLA_DEBUG)
      { cout << "rp:" << i << ": " ;
        iv.Print(cout);
        cout << "(" << u1Pos << " " << u2Pos << ")" << endl;
      }

    if (u1Pos == -1 || u2Pos == -1 )
      continue;

    else
    {
      if(TLA_DEBUG)
      {
        cout<<"Both operators existant in interval iv #"<<i<<": ";
        iv.Print(cout);
        cout << "(" << u1Pos<< " "<< u2Pos<< ")" << endl;
      }
      op1->Get(u1Pos, u1transfer);
      op2->Get(u2Pos, u2transfer);

      if (TLA_DEBUG)
      {
        cout << "Actual partition #" << i << "/" << rp.Size()-1 << ":"
             << endl << " u1=";
        u1transfer.Print(cout);
        cout << endl << " u2="; u2transfer.Print(cout); cout << endl;
      }
      if( !u1transfer.IsDefined() || !u2transfer.IsDefined() )
        continue;
      u1transfer.AtInterval(iv, u1);
      u2transfer.AtInterval(iv, u2);

      // create intersection of  u1 x u2 (may be undefined!)
      u1.Intersection(u2, resunit);

      if( resunit.IsDefined() )
      {
        if( !resunit.timeInterval.IsValid() )
        { // Debugging Info
          cout << "Error in " << __PRETTY_FUNCTION__ << " ["<< __FILE__ << ":"
              << __LINE__ << "]:" << endl;
          cout << "u1transfer = "; u1transfer.Print(cout); cout << endl;
          cout << "u2transfer = "; u2transfer.Print(cout); cout << endl;
          cout << "u1         = "; u1.Print(cout); cout << endl;
          cout << "u2         = "; u2.Print(cout); cout << endl;
          cout << "iv         = " << (iv.lc ? "[" : "(")
              << iv.start << "," << iv.end << (iv.rc ? "]" : ")") << endl;
          cout << "resunit.timeInterval = "
              << (resunit.timeInterval.lc ? "[" : "(")
              << resunit.timeInterval.start << "," << resunit.timeInterval.end
              << (resunit.timeInterval.rc ? "]" : ")") << endl;
          resunit.SetDefined(false);
          assert( resunit.timeInterval.IsValid() );
        }

        if( resunit.IsDefined() && !resunit.timeInterval.Inside(iv) )
        {
        // invalidate result, if it is on an open border of iv
          resunit.SetDefined(false);
        }

        if ( resunit.IsDefined() && lastresunit.IsDefined() )
        { // Check for conflicting timeIntervals:
          if ( (lastresunit.timeInterval.end == resunit.timeInterval.start) &&
                lastresunit.timeInterval.rc && resunit.timeInterval.lc
             )
          { // We have a conflict!
          // solution1: change closedness flag for one non-instant unit
          // solution2: drop a [x, x]-type unit
            if (TLA_DEBUG)
            { cout << "\n We have a conflict! \n" << endl;
            cout << "lastresunit: "; lastresunit.timeInterval.Print(cout);
            cout << endl;
            cout << "resunit:     "; resunit.timeInterval.Print(cout);
            cout << endl;
            }
            if (lastresunit.timeInterval.start == lastresunit.timeInterval.end)
            { // drop lastresunit
              if (TLA_DEBUG)
                cout << "Dropping lastresunit.\n" << endl;
            // do not add lastresunit
              lastresunit = resunit; // queue up resunit
            }
            else if (resunit.timeInterval.start == resunit.timeInterval.end)
            { // drop resunit
            // do not add lastresunit
            // let lastresunit in the queue, do not queue up resunit
              if (TLA_DEBUG)
                cout << "Dropping resunit.\n" << endl;
            }
            else
            { // set closednessflags to pattern |A,B[   ]B,C|
              if (TLA_DEBUG)
                cout << "Changing closedness.\n" << endl;
              lastresunit.timeInterval.rc = false;
              resunit.timeInterval.lc = true;
              res->MergeAdd(lastresunit); // add lastresunit
              lastresunit = resunit;      // queue up resunit
            }
          }
          else
          { // else: No conflict.
            if (TLA_DEBUG)
              cout << "No conflict.\n" << endl;
            if ( lastresunit.IsDefined() )
              res->MergeAdd(lastresunit); // Add lastresunit
            if ( resunit.IsDefined() )
              lastresunit = resunit;      // queue up resunit
          }
        }
        else if ( resunit.IsDefined() )
        // lastresunit undefined, but resunit defined
          lastresunit = resunit; // ignore lastresunit, queue up resunit
        // else: lastresunit defined, but resunit undefined - Do nothing!
      } // else: resunit undefined. Do nothing.
    }
  }
  if (lastresunit.IsDefined() )
  { // possibly insert the last result unit
    res->MergeAdd(lastresunit);
  }
  res->EndBulkLoad( false );

  return 0;
}

/*
16.3 Value mapping functions of operators ~intsersection~ and ~minus~
for mreal/mreal

*/
template<int op>
int TemporalMMRealIntercept( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingRealIntersectionMM( *((MReal*)args[0].addr),
   *((MReal*)args[1].addr), *((MReal*)result.addr), op);

  return 0;
}

/*
16.3 Value mapping functions of operator ~intsersection~ and ~minus~
for bool/mbool, int/mint and string/mstring

*/
template<class Mapping1, class Unit1, class Operator1, int op>
int TemporalSMIntersection( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  if(TLA_DEBUG)
    cout<<"TemporalSMIntersection called"<<endl;

  result = qp->ResultStorage( s );
  Mapping1  *res = static_cast<Mapping1*>(result.addr);
  Operator1 *constop = (Operator1*)args[0].addr;
  Mapping1  *mop2 = (Mapping1*)args[1].addr;

  res->Clear();
  if( !constop->IsDefined() || !mop2->IsDefined() ){
    res->SetDefined( false );
    return 0;
  }
  res->SetDefined( true );

  Mapping1 *mop1 = new Mapping1(0);
  mop1->Clear();
  mop1->SetDefined( true );
  mop1->StartBulkLoad();
  Unit1 up2;
  for (int i = 0; i < mop2->GetNoComponents(); i++) {
      mop2->Get(i, up2);
     if(!up2.IsDefined())
          continue;
      Unit1 *up1 = new Unit1(up2.timeInterval, *constop);

      mop1->Add(*up1);
      delete up1;
  }
  mop1->EndBulkLoad(false);
  MovingIntersectionMM<Mapping1, Mapping1, Unit1, Unit1>(*mop1,*mop2,*res,op);
  mop1->DeleteIfAllowed();

  return 0;
}

/*
16.3 Value mapping functions of operators ~intsersection~ and ~minus~
for real/mreal

*/
template<int op>
int TemporalSMRealIntercept( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  if(TLA_DEBUG)
    cout<<"TemporalSMRealIntercept called"<<endl;
  result = qp->ResultStorage( s );
  CcReal *constop = static_cast<CcReal*>(args[0].addr);
  MReal  *mop2    = static_cast<MReal*>(args[1].addr);
  MReal  *res     = static_cast<MReal*>(result.addr);
  res->Clear();
  if( !constop->IsDefined() || !mop2->IsDefined() ){
    res->SetDefined( false );
    return 0;
  }
  res->SetDefined( true );

  UReal up2;
  MReal *mop1 = new MReal(0);
  mop1->Clear();
  mop1->StartBulkLoad();
  for (int i = 0; i < mop2->GetNoComponents(); i++) {
      mop2->Get(i, up2);
      if(!up2.IsDefined())
          continue;

      UReal *up1 = new UReal(up2.timeInterval, 0.0, 0.0, (up2.r) ?
        pow(constop->GetRealval(), 2) : constop->GetRealval(), up2.r);

      if(TLA_DEBUG){
        cout<<"up1["<<i<<"] ["<<up1->timeInterval.start.ToString()
        <<" "<<up1->timeInterval.end.ToString()<<" "
        <<up1->timeInterval.lc<<" "<<up1->timeInterval.rc<<"] "<<endl;}
      mop1->Add(*up1);
      delete up1;
  }
  mop1->EndBulkLoad(false);
  MovingRealIntersectionMM( *mop1, *mop2, *res, op);
  mop1->DeleteIfAllowed();

  return 0;
}

/*
16.3 Value mapping functions of operator ~minus~ for mpoint/mpoint

*/
int TemporalMMPointIntercept( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MBool *mBool = new MBool(0);
  MovingPointCompareMM(*((MPoint*)args[0].addr),
    *((MPoint*)args[1].addr), *mBool, 0);
  MPoint* endResult = (MPoint*)result.addr;
  TransformMBool2MPoint(((MPoint*)args[0].addr), mBool, endResult);
  mBool->DeleteIfAllowed();
  return 0;
}

/*
16.3 Value mapping functions of operator ~minus~ for mpoint/point

*/
int TemporalMSPointIntercept( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MBool *mBool = new MBool(0);
  MovingPointCompareMS(*((MPoint*)args[0].addr),
    *((Point*)args[1].addr), *mBool, 0);
  MPoint* endResult = (MPoint*)result.addr;
  TransformMBool2MPoint(((MPoint*)args[0].addr), mBool, endResult);
  mBool->DeleteIfAllowed();
  return 0;
}

/*
16.3 Value mapping functions of operator ~minus~  for point/mpoint

*/
int TemporalSMPointIntercept( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MBool *mBool = new MBool(0);
  MovingPointCompareMS(*((MPoint*)args[1].addr),
    *((Point*)args[0].addr), *mBool, 0);
  MPoint* endResult = (MPoint*)result.addr;
  TransformMBool2MPoint(((Point*)args[0].addr), mBool, endResult);
  mBool->DeleteIfAllowed();
  return 0;
}

/*
1.1 ValueMapping of operator ~rough\_center~ for mregion

*/
int RCenterValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {

    result = qp->ResultStorage(s);
    RCenter(* (MRegion*) args[0].addr,* (MPoint*) result.addr);
    return 0;
}

/*
1.1 ValueMapping of operator ~minus~ with Region/MPoint

*/
int RMPMinusValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {

    result = qp->ResultStorage(s);
    copyRegionMPoint(*((Region*)args[0].addr) ,
      *((MPoint*)args[1].addr), *((MRegion*)result.addr) );
    return 0;
}

/*
1.1 ValueMapping of operator ~minus~ with MRegion/Point

*/
int MRPMinusValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {

    result = qp->ResultStorage(s);
    copyMRegion(*((MRegion*)args[0].addr), *(MRegion*)result.addr);
    return 0;
}

/*
1.1 ValueMapping of operator ~minus~ with MRegion/MPoint

*/
int MRMPMinusValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {

    result = qp->ResultStorage(s);
    copyMRegionMPoint(*((MRegion*)args[0].addr) ,
    *((MPoint*)args[1].addr), *((MRegion*)result.addr) );
    return 0;
}

/*
1.1 ValueMapping of operator ~no\_components~ for mregion

*/
int NComponentsValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {

    result = qp->ResultStorage(s);
    NComponents(* (MRegion*) args[0].addr,* (MInt*) result.addr);
    return 0;
}

/*
1.1 ValueMapping of operator ~perimeter~ for mregion

*/
int PerimeterValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {

    result = qp->ResultStorage(s);
    MPerimeter(* (MRegion*) args[0].addr, * (MReal*) result.addr);
    return 0;
}

/*
1.1 ValueMapping of operator ~area~ for mregion

*/
int AreaValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {

    result = qp->ResultStorage(s);
    MArea(* (MRegion*) args[0].addr, * (MReal*) result.addr);
    return 0;
}

/*
16.3 Value mapping functions of operator ~distance~ for mpoint/mpoint

*/
int MPointMMDistance( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  DistanceMPoint(*((MPoint*)args[0].addr),
   *((MPoint*)args[1].addr), *((MReal*)result.addr) );
  return 0;
}

/*
16.3 Value mapping of operator ~distance~ for mreal/mreal

*/
int MRealMMDistance( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MRealDistanceMM( *((MReal*)args[0].addr),
   *((MReal*)args[1].addr), *((MReal*)result.addr));
  return 0;
}

/*
16.3 Value mapping of operator ~distance~ for mreal/real

*/
int MRealMSDistance( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  if(TLA_DEBUG)
    cout<<"MRealMSDistance called"<<endl;
  result = qp->ResultStorage( s );
  MReal  *res     = static_cast<MReal*>(result.addr);
  MReal  *mop1    = static_cast<MReal*>(args[0].addr);
  CcReal *constop = static_cast<CcReal*>(args[1].addr);
  res->Clear();
  if( !mop1->IsDefined() || !constop->IsDefined() ){
    res->SetDefined( false );
    return 0;
  }
  res->SetDefined( true );
  UReal up1;
  MReal *mop2 = new MReal(0);
  mop2->Clear();
  mop2->SetDefined( true );
  mop2->StartBulkLoad();
  for (int i = 0; i < mop1->GetNoComponents(); i++) {
      mop1->Get(i, up1);
      if(!up1.IsDefined())
          continue;
      UReal *up2 = new UReal(up1.timeInterval, 0.0, 0.0,
                       (up1.r) ? pow(constop->GetRealval(), 2)
                       : constop->GetRealval(),up1.r);
      mop2->Add(*up2);
      delete up2;
  }
  mop2->EndBulkLoad(false);
  MRealDistanceMM( *mop1, *mop2, *res);
  mop2->DeleteIfAllowed();

  return 0;
}

/*
16.3 Value mapping of operator ~distance~ for real/mreal

*/
int MRealSMDistance( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  if(TLA_DEBUG)
    cout<<"MRealSMDistance called"<<endl;
  result = qp->ResultStorage( s );
  MReal  *res     = static_cast<MReal*>(result.addr);
  MReal  *mop1    = static_cast<MReal*>(args[1].addr);
  CcReal *constop = static_cast<CcReal*>(args[0].addr);
  res->Clear();
  if( !mop1->IsDefined() || !constop->IsDefined() ){
    res->SetDefined( false );
    return 0;
  }
  res->SetDefined( true );
  UReal up1;
  MReal *mop2 = new MReal(0);
  mop2->Clear();
  mop2->StartBulkLoad();
  for (int i = 0; i < mop1->GetNoComponents(); i++) {
      mop1->Get(i, up1);
      if(!up1.IsDefined())
          continue;
      UReal *up2 = new UReal(up1.timeInterval, 0.0, 0.0,
                       (up1.r) ? pow(constop->GetRealval(), 2)
                       : constop->GetRealval(),up1.r);
      mop2->Add(*up2);
      delete up2;
  }
  mop2->EndBulkLoad(false);
  MRealDistanceMM( *mop1, *mop2, *res );
  mop2->DeleteIfAllowed();

  return 0;
}

/*
16.3 Value mapping functions of operator ~and~ and ~or~ for mbool/mbool

op == 1 -> AND, op == 2 -> OR

*/
template<int op>
int TemporalMMLogic( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MovingBoolMMOperators( *((MBool*)args[0].addr),
   *((MBool*)args[1].addr), *((MBool*)result.addr), op);
  return 0;
}

/*
16.3 Value mapping functions of operators ~and~ and ~or~ for mbool/bool

op == 1 -> AND, po == 2 -> OR

*/
template<int op>
int TemporalMSLogic( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MovingBoolMSOperators( *((MBool*)args[0].addr),
   *((CcBool*)args[1].addr), *((MBool*)result.addr), op);
  return 0;
}

/*
16.3 Value mapping functions of operators ~and~  and ~or~ for bool/mbool

op == 1 -> AND, po == 2 -> OR

*/
template<int op>
int TemporalSMLogic( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MovingBoolMSOperators( *((MBool*)args[1].addr),
   *((CcBool*)args[0].addr), *((MBool*)result.addr), op);
  return 0;
}

/*
16.3 Value mapping functions of operator ~not~ for mbool

*/
int TemporalNot( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  if(TLA_DEBUG)
      cout<<"temporalNot called"<<endl;
  result         = qp->ResultStorage( s );
  MBool* pResult = (MBool*)result.addr;
  MBool* op      = (MBool*)args[0].addr;
  pResult->Clear();
  if( !op->IsDefined() ){
    pResult->SetDefined( false );
    return 0;
  }
  pResult->SetDefined( true );
  UBool u1transfer;
  UBool uBool(true);

  pResult->StartBulkLoad();
  for( int i = 0; i < op->GetNoComponents(); i++)
  {
    op->Get(i, u1transfer);
    if(!u1transfer.IsDefined())
        continue;
    uBool = u1transfer;
    uBool.constValue.Set(uBool.constValue.IsDefined(),
                       !(uBool.constValue.GetBoolval()));
    if(TLA_DEBUG){
      cout<<"wert "<<&uBool.constValue<<endl;
      cout<<"interval "<< uBool.timeInterval.start.ToString()
      <<" "<<uBool.timeInterval.end.ToString()<<" "
      <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<endl;}
    pResult->Add(uBool);
  }
  pResult->EndBulkLoad(false);

  return 0;
}

/*
1.1 Value mapping functions of operator ~zero~

Creates a moving int with value 0 for all times

*/
int TemporalZeroValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {
    if(TLA_DEBUG)
      cout<< "TemporalZeroValueMap() called" << endl;

    result = qp->ResultStorage(s);
    MInt* pResult = (MInt*)result.addr;

    pResult->Clear();
    pResult->SetDefined( true );
    pResult->StartBulkLoad();
    UInt uInt(true);
    uInt.timeInterval.lc = true;
    uInt.timeInterval.start.ToMinimum();
    uInt.timeInterval.start.SetType(instanttype);
    uInt.timeInterval.rc = true;
    uInt.timeInterval.end.ToMaximum();
    uInt.timeInterval.end.SetType(instanttype);
    uInt.constValue.Set(true,0);
    pResult->Add(uInt);
    pResult->EndBulkLoad(false);

    return 0;
}

/*
16.3 Value mapping functions of operator ~mint~

Every interval in periods will be transformed into an UInt with
value 1, The definition gaps will be transformed into UInts with value 0.

*/
int periods2mintVM( Word* args, Word& result, int message,
 Word& local, Supplier s )
{

  Periods* src = (Periods*) args[0].addr;
  result = qp->ResultStorage(s);
  MInt* res = (MInt*) result.addr;
  res->Clear();

  if(!src->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  
  Periods* extension = 0;

  Periods maxInterval(1);
  

  if(qp->GetNoSons(s)==2){ // optional second periods value present
    extension = (Periods*) args[1].addr;
    if(!extension->IsDefined()){
       extension = 0;
    } else if(extension->IsEmpty()){
      extension = 0;
    }
  } else {
     DateTime maxTime(datetime::instanttype);
     DateTime minTime(datetime::instanttype);
     maxTime.ToMaximum();
     minTime.ToMinimum();
     Interval<Instant> iv(minTime, maxTime, true,true);
     maxInterval.StartBulkLoad();
     maxInterval.Add(iv);
     maxInterval.EndBulkLoad();
     extension = &maxInterval;
  }

  CcInt zero(true,0);
  CcInt one(true,1);

  if(src->IsEmpty()){
    if(extension==0){
      return 0;
    } 
    // create a zero from extension
    Interval<Instant> u1;
    Interval<Instant> u2;
    extension->Get(0,u1);
    extension->Get(extension->GetNoComponents()-1,u2);
    Interval<Instant> iv(u1.start, u2.end,u1.lc,u2.rc);
    UInt v(iv,zero);
    res->StartBulkLoad();
    res->Add(v);
    res->EndBulkLoad();
    return 0;
  }

  // normal case, src defined and non-empty
  int size = src->GetNoComponents();
  Interval<Instant> lastIv;
  Interval<Instant> iv;
  res->StartBulkLoad();
  for(int i=0;i<size;i++){

     if(i==0){
        src->Get(0,lastIv);
        if(extension!=0){
           Interval<Instant> eiv;
           extension->Get(0,eiv);
           if( (eiv.start < lastIv.start) ||
               ((eiv.start == lastIv.start) && eiv.lc && !lastIv.lc)){
              Interval<Instant> cur(eiv.start, lastIv.start, 
                                    eiv.lc, !lastIv.lc);
              UInt v(cur,zero);
              res->Add( v);
           }
        }
        UInt v(lastIv,one);
        res->Add( v);
     } else { // not the first unit
       src->Get(i,iv);
       if((lastIv.end < iv.start) ||  
          ( (lastIv.end == iv.start) && (lastIv.rc != iv.lc))) {
         // fill gap
         Interval<Instant> cur(lastIv.end, iv.start, !lastIv.rc, !iv.lc);
         UInt v(cur,zero);
       }
       UInt v(iv,one);
       res->Add( v);
       lastIv = iv;
     }
  }
  // extend if required
  if(extension){
     extension->Get(extension->GetNoComponents()-1, iv);
     if( (lastIv.end < iv.end) ||
         ( (lastIv.end == iv.end) && !lastIv.rc && iv.rc)){
       Interval<Instant> cur(lastIv.end, iv.end, !lastIv.rc, iv.rc);
       UInt v(cur,zero);
       res->Add( v);
     }
  }
  res->EndBulkLoad(false, true);
  return 0;
}


/*
16.4 Value Mapping Operator ~createmint~

*/

int createmintVM( Word* args, Word& result, int message,
                  Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  MInt* res = (MInt*) result.addr;
  Periods* p = (Periods*) args[0].addr;
  CcInt*   v = (CcInt*) args[1].addr;

  res->Clear();
  if(!p->IsDefined() || !v->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  res->SetDefined(true);
  res->StartBulkLoad();
  int size = p->GetNoComponents();
  Interval<Instant> iv;
  //int value = v->GetValue();
  for(int i=0;i<size;i++){
     p->Get(i,iv);
     UInt ui(iv,*v);
     res->Add(ui);
  }
  res->EndBulkLoad(false);
  return 0;
}






int eplusVM( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
   MInt* arg1 = static_cast<MInt*>(args[0].addr);
   MInt* arg2 = static_cast<MInt*>(args[1].addr);
   result = qp->ResultStorage(s);
   MInt* res = static_cast<MInt*>(result.addr);
   res->Clear();
   if( !arg1->IsDefined() || !arg2->IsDefined() ){
     res->SetDefined( false );
     return 0;
   }
   res->SetDefined( true );
   arg1->PlusExtend(arg2,*res);
   return 0;
}


/*
1.1 Value mapping functions of operator ~concat~

Concats two mpoints. If intervals are not disjunct it creates 
an undefined value.

*/
int TemporalConcatValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {
    if(TLA_DEBUG)
      cout<< "TemporalConcatValueMap() called" << endl;

    result = qp->ResultStorage(s);
    MPoint* p1 = (MPoint*)args[0].addr;
    MPoint* p2 = (MPoint*)args[1].addr;
    MPoint* pResult = (MPoint*)result.addr;
    pResult->Clear();
    if( !p1->IsDefined() || !p2->IsDefined() ){
      pResult->SetDefined( false );
      return 0;
    }
    pResult->SetDefined( true );

    UPoint up1;
    UPoint up2;

    pResult->StartBulkLoad();
    if(p1->GetNoComponents() > 0 && p2->GetNoComponents() > 0){
      p1->Get(p1->GetNoComponents() - 1, up1);
      p2->Get(0, up2);
      if(!( up1.timeInterval.end < up2.timeInterval.start
           || (up1.timeInterval.end == up2.timeInterval.start
               && !(up1.timeInterval.rc && up2.timeInterval.lc)))){
        if(TLA_DEBUG)
        {
          cout<<"DefTime of mpoints are not disjunct! Last interval of first "
            <<"mpoint ends after first interval of of second mpoint begins."
            << endl;
          cout << "first interval ";
          up1.timeInterval.Print(cout);
          cout << endl << "second interval";
          up2.timeInterval.Print(cout);
          cout<< endl;
        }
        pResult->EndBulkLoad(false);
        pResult->SetDefined( false );
        return 0;
      }
    }
    for( int i = 0; i < p1->GetNoComponents(); i++) {
      p1->Get(i, up1);
      if(!up1.IsDefined())
        continue;
      pResult->Add(up1);
    }
    for( int i = 0; i < p2->GetNoComponents(); i++) {
      p2->Get(i, up2);
      if(!up2.IsDefined())
        continue;
      pResult->Add(up2);
    }
    pResult->EndBulkLoad(false);

    return 0;
}

/*
16.3 Value mapping function of operators ~abs~ for mreal

*/
int MovingRealABS( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  MRealABS( *((MReal*)args[0].addr), *((MReal*)result.addr));
  return 0;
}

/*
1 ValueMapping-Arrays

*/

static ValueMapping temporalmequalmap[] = {
                TemporalMMCompare<MBool, MBool, UBool, UBool, 0>,
                TemporalMSCompare<MBool, UBool, CcBool, 0>,
                TemporalSMCompare<MBool, UBool, CcBool, 0>,
                TemporalMMCompare<MInt, MInt, UInt, UInt, 0>,
                TemporalMSCompare<MInt, UInt,  CcInt, 0>,
                TemporalSMCompare<MInt, UInt,  CcInt, 0>,
                TemporalMMCompare<MString, MString, UString, UString, 0>,
                TemporalMSCompare<MString, UString,  CcString, 0>,
                TemporalSMCompare<MString, UString,  CcString, 0>,
                TemporalMMRealCompare<0>,
                TemporalMSRealCompare<0>,
                TemporalSMRealCompare<0>,
                TemporalMMPointCompare<0>,
                TemporalMSPointCompare<0>,
                TemporalSMPointCompare<0>,
                TemporalMSRegionCompare<0>,
                TemporalSMRegionCompare<0>,
                TemporalMMRegionCompare<0>};

static ValueMapping temporalmnotequalmap[] = {
                TemporalMMCompare<MBool, MBool, UBool, UBool, -3>,
                TemporalMSCompare<MBool, UBool, CcBool, -3>,
                TemporalSMCompare<MBool, UBool, CcBool, -3>,
                TemporalMMCompare<MInt, MInt, UInt, UInt, -3>,
                TemporalMSCompare<MInt, UInt, CcInt, -3>,
                TemporalSMCompare<MInt, UInt, CcInt, -3>,
                TemporalMMCompare<MString, MString, UString, UString, -3>,
                TemporalMSCompare<MString, UString,  CcString, -3>,
                TemporalSMCompare<MString, UString,  CcString, -3>,
                TemporalMMRealCompare<-3>,
                TemporalMSRealCompare<-3>,
                TemporalSMRealCompare<-3>,
                TemporalMMPointCompare<-3>,
                TemporalMSPointCompare<-3>,
                TemporalSMPointCompare<-3>,
                TemporalMSRegionCompare<-3>,
                TemporalSMRegionCompare<-3>,
                TemporalMMRegionCompare<-3>};

static ValueMapping temporalmlessmap[] =     {
                TemporalMMCompare<MBool, MBool, UBool, UBool, -2>,
                TemporalMSCompare<MBool, UBool, CcBool, -2>,
                TemporalSMCompare<MBool, UBool, CcBool, -2>,
                TemporalMMCompare<MInt, MInt, UInt, UInt, -2>,
                TemporalMSCompare<MInt, UInt, CcInt, -2>,
                TemporalSMCompare<MInt, UInt, CcInt, -2>,
                TemporalMMRealCompare<-2>,
                TemporalMSRealCompare<-2>,
                TemporalSMRealCompare<-2>,
                TemporalMMCompare<MString, MString, UString, UString, -2>,
                TemporalMSCompare<MString, UString, CcString, -2>,
                TemporalSMCompare<MString, UString, CcString, -2>};

static ValueMapping temporalsubtractmap[] =     {
   TemporalMIIAdd<-1>,
   TemporalMIRAdd<-1>,
   TemporalMRRAdd<-1>,
   TemporalMRIAdd<-1>,

   TemporalIMIAdd<-1>,
   TemporalRMIAdd<-1>,
   TemporalRMRAdd<-1>,
   TemporalIMRAdd<-1>,

   TemporalMIMIAdd<-1>,
   TemporalMIMRAdd<-1>,
   TemporalMRMRAdd<-1>,
   TemporalMRMIAdd<-1>
   };

static ValueMapping temporaladdmap[] =     {
   TemporalMIIAdd<1>,
   TemporalMIRAdd<1>,
   TemporalMRRAdd<1>,
   TemporalMRIAdd<1>,

   TemporalIMIAdd<1>,
   TemporalRMIAdd<1>,
   TemporalRMRAdd<1>,
   TemporalIMRAdd<1>,

   TemporalMIMIAdd<1>,
   TemporalMIMRAdd<1>,
   TemporalMRMRAdd<1>,
   TemporalMRMIAdd<1>
   };

static ValueMapping temporalmultiplymap[] =     {
    TemporalMIIMultiply,
    TemporalMIRMultiply<false>,
    TemporalMRRMultiply<false>,
    TemporalMRIMultiply<false>,

    TemporalIMIMultiply,
    TemporalRMIMultiply<false>,
    TemporalIMRMultiply,
    TemporalRMRMultiply,

    TemporalMIMIMultiply,
    TemporalMIMRMultiply,
    TemporalMRMIMultiply<false>
    };

static ValueMapping temporaldividemap[] =     {
    TemporalMIIDivide,
    TemporalMIRMultiply<true>,
    TemporalMRRMultiply<true>,
    TemporalMRIMultiply<true>,

    TemporalIMIDivide,
    TemporalRMIMultiply<true>,

    TemporalMIMIDivide,
    TemporalMRMIMultiply<true>
    };

static ValueMapping temporalmlessequalmap[] =     {
                TemporalMMCompare<MBool, MBool, UBool, UBool, -1>,
                TemporalMSCompare<MBool, UBool, CcBool, -1>,
                TemporalSMCompare<MBool, UBool, CcBool, -1>,
                TemporalMMCompare<MInt, MInt, UInt, UInt, -1>,
                TemporalMSCompare<MInt, UInt, CcInt, -1>,
                TemporalSMCompare<MInt, UInt, CcInt, -1>,
                TemporalMMRealCompare<-1>,
                TemporalMSRealCompare<-1>,
                TemporalSMRealCompare<-1>,
                TemporalMMCompare<MString, MString, UString, UString, -1>,
                TemporalMSCompare<MString, UString, CcString, -1>,
                TemporalSMCompare<MString, UString, CcString, -1>};

static ValueMapping temporalmgreatermap[] =     {
                TemporalMMCompare<MBool, MBool, UBool, UBool, 2>,
                TemporalMSCompare<MBool, UBool, CcBool, 2>,
                TemporalSMCompare<MBool, UBool, CcBool, 2>,
                TemporalMMCompare<MInt, MInt, UInt, UInt, 2>,
                TemporalMSCompare<MInt, UInt, CcInt, 2>,
                TemporalSMCompare<MInt, UInt, CcInt, 2>,
                TemporalMMRealCompare<2>,
                TemporalMSRealCompare<2>,
                TemporalSMRealCompare<2>,
                TemporalMMCompare<MString, MString, UString, UString, 2>,
                TemporalMSCompare<MString, UString, CcString, 2>,
                TemporalSMCompare<MString, UString, CcString, 2>};

static ValueMapping temporalmgreaterequalmap[] =    {
                TemporalMMCompare<MBool, MBool, UBool, UBool, 1>,
                TemporalMSCompare<MBool, UBool, CcBool, 1>,
                TemporalSMCompare<MBool, UBool, CcBool, 1>,
                TemporalMMCompare<MInt, MInt, UInt, UInt, 1>,
                TemporalMSCompare<MInt, UInt, CcInt, 1>,
                TemporalSMCompare<MInt, UInt, CcInt, 1>,
                TemporalMMRealCompare<1>,
                TemporalMSRealCompare<1>,
                TemporalSMRealCompare<1>,
                TemporalMMCompare<MString, MString, UString, UString, 1>,
                TemporalMSCompare<MString, UString, CcString, 1>,
                TemporalSMCompare<MString, UString, CcString, 1>};

static ValueMapping temporalliftisemptyvaluemap[] = {
                IsEmptyValueMap<MRegion, URegionEmb>,
                IsEmptyValueMap<MBool, UBool>,
                IsEmptyValueMap<MInt, UInt>,
                IsEmptyValueMap<MReal, UReal>,
                IsEmptyValueMap<MPoint, UPoint>,
                IsEmptyValueMap<MString, UString>};

static ValueMapping temporalliftinsidemap[] = {
                MPointPointsInside<1>,
                MPointLineInside<1>,
                MFalseValueMap,
                MFalseValueMap  };

static ValueMapping temporalliftintersectionmap[] = {
                TemporalMMIntersection<MBool, MBool, UBool, UBool, 1>,
                TemporalMSIntersection<MBool, UBool, CcBool, 1>,
                TemporalSMIntersection<MBool, UBool, CcBool, 1>,
                TemporalMMIntersection<MInt, MInt, UInt, UInt, 1>,
                TemporalMSIntersection<MInt, UInt, CcInt, 1>,
                TemporalSMIntersection<MInt, UInt, CcInt, 1>,
                TemporalMMRealIntercept<1>,
                TemporalMSRealIntercept<1>,
                TemporalSMRealIntercept<1>,
                MPointPointsInside<2>,
                MPointLineInside<2>,
                PointsMPointIntersection,
                LineMPointIntersection,
                TemporalMMIntersection<MString, MString, UString, UString, 1>,
                TemporalMSIntersection<MString, UString, CcString, 1>,
                TemporalSMIntersection<MString, UString, CcString, 1>,
                TemporalMPointMPointIntersection};

static ValueMapping unionvaluemap[] = {
                MPRUnionValueMap,
                MPMRUnionValueMap,
                PMRUnionValueMap};

static ValueMapping temporalliftminusmap[] = {
                TemporalMMIntersection<MBool, MBool, UBool, UBool, 2>,
                TemporalMSIntersection<MBool, UBool, CcBool, 2>,
                TemporalSMIntersection<MBool, UBool, CcBool, 2>,
                TemporalMMIntersection<MInt, MInt, UInt, UInt, 2>,
                TemporalMSIntersection<MInt, UInt, CcInt, 2>,
                TemporalSMIntersection<MInt, UInt, CcInt, 2>,
                TemporalMMRealIntercept<2>,
                TemporalMSRealIntercept<2>,
                TemporalSMRealIntercept<2>,
                TemporalMMPointIntercept,
                TemporalMSPointIntercept,
                TemporalSMPointIntercept,
                RMPMinusValueMap,
                MRPMinusValueMap,
                MRMPMinusValueMap,
                MRPMinusValueMap,
                MRPMinusValueMap,
                TemporalMMIntersection<MString, MString, UString, UString, 2>,
                TemporalMSIntersection<MString, UString, CcString, 2>,
                TemporalSMIntersection<MString, UString, CcString, 2>};

static ValueMapping temporaldistancemap[] = {
                MPointMMDistance,
                MRealMMDistance,
                MRealMSDistance,
                MRealSMDistance};

static ValueMapping temporalandmap[] = {
                TemporalMMLogic<1>,
                TemporalMSLogic<1>,
                TemporalSMLogic<1>};

static ValueMapping temporalormap[] = {
                TemporalMMLogic<2>,
                TemporalMSLogic<2>,
                TemporalSMLogic<2>};

/*
1 Specifications of operations

*/


const string TemporalLiftSpecMEqual
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>T in {bool, int, string, real, point, region}"
             ", mT X mT -> mbool,"
             " mT x T -> mbool, T x mT -> mbool</text--->"
             "<text> _ = _ </text--->"
             "<text>Logical equality for two MovingT.</text--->"
             "<text>mb1 = mb2</text--->"
             ") )";

const string TemporalLiftSpecMNotEqual
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
            "\"Example\" ) "
            "( <text>T in {bool, int, string, real, point, region}"
            ", mT X mT -> mbool,"
            " mT x T -> mbool, T x mT -> mbool</text--->"
            "<text> _ # _ </text--->"
            "<text>Logical unequality for two MovingT.</text--->"
            "<text>mb1 # mb2</text--->"
            ") )";

const string TemporalLiftSpecLT
          = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
            "\"Example\" )"
            "( <text>T in {bool, int, real, string},  mT x T ->  mbool,"
            " (mT x mT) -> mbool, (T x mT) -> mbool</text--->"
            "<text>_ < _</text--->"
            "<text>Less than.</text--->"
            "<text>query i1 < i2</text--->"
            ") )";

const string TemporalLiftSpecSubtract
          = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
            "\"Example\" )"
            "( <text> (mint x int) ->  mint, (mint x mint) ->  mint,"
            " (mreal x real) -> mreal, (mreal x mreal) -> mreal,"
            " (mreal x int) -> mreal, (mint x real) -> mreal,"
            " (mreal x mint) -> mreal</text--->"
            "<text>_ - _</text--->"
            "<text>Subtract</text--->"
            "<text>query i1 - i2</text--->"
            ") )";

const string TemporalLiftSpecAdd
          = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
            "\"Example\" )"
            "( <text> (mint x int) ->  mint, (mint x mint) ->  mint,"
            " (mreal x real) -> mreal, (mreal x mreal) -> mreal,"
            " (mreal x int) -> mreal, (mint x real) -> mreal,"
            " (mreal x mint) -> mreal</text--->"
            "<text>_ + _</text--->"
            "<text>Add</text--->"
            "<text>query i1 + i2</text--->"
            ") )";

const string TemporalLiftSpecMultiply
          = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
            "\"Example\" )"
            "( <text> (mint x int) ->  mint, (mint x mint) ->  mint,"
            " (mreal x real) -> mreal, "
            " (mreal x int) -> mreal, (mint x real) -> mreal,"
            " (mreal x mint) -> mreal</text--->"
            "<text>_ * _</text--->"
            "<text>Multiply</text--->"
            "<text>query i1 * i2</text--->"
            ") )";

const string TemporalLiftSpecDivide
          = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
            "\"Example\" )"
            "( <text> (mint x int) ->  mreal, (mint x mint) ->  mreal,"
            " (mreal x real) -> mreal, "
            " (mreal x int) -> mreal, (mint x real) -> mreal,"
            " (mreal x mint) -> mreal</text--->"
            "<text>_ / _</text--->"
            "<text>Divide</text--->"
            "<text>query i1 / i2</text--->"
            ") )";

const string TemporalLiftSpecLE
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
            "\"Example\" )"
            "( <text>T in {bool, int, real, string}, mT x T -> mbool,"
            " (mT x mT) -> mbool, (T x mT) -> mbool</text--->"
            "<text>_ <= _</text--->"
            "<text>Less or equal than.</text--->"
            "<text>query i1 <= i2</text--->"
            ") )";

const string TemporalLiftSpecGT
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
            "\"Example\" )"
            "( <text>T in {bool, int, real, string},  mT x T -> mbool,"
            " (mT x mT) -> mbool, (T x mT) -> mbool</text--->"
            "<text>_ > _</text--->"
            "<text>Greater than.</text--->"
            "<text>query i1 > i2</text--->"
            ") )";

const string TemporalLiftSpecGE
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
            "\"Example\" )"
            "( <text>T in {bool, int, real, string},  mT x T -> mbool,"
            " (mT x mT) -> mbool, (T x mT) -> mbool</text--->"
            "<text>_ >= _</text--->"
            "<text>Greater or equal than.</text--->"
            "<text>query i1 >= i2</text--->"
            ") )";

const string temporalliftisemptyspec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
            "\"Example\" ) "
            "( <text>T in {bool, int, string, real, point, region}"
            " mT -> mbool</text--->"
            "<text>isempty( _ )</text--->"
            "<text>Checks if the m. object is not defined .</text--->"
            "<text>isempty(mrg1)</text---> ) )";

const string TemporalLiftSpecInside
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
            "\"Example\" ) "
            "( <text>mpoint x points -> mbool,"
            " mpoint x line -> mbool, mregion x points ->"
            " mbool, mregion x line -> mbool</text--->"
            "<text>_ inside _</text--->"
            "<text>Inside.</text--->"
            "<text>query mp1 inside pts1</text--->"
            ") )";

const string TemporalLiftSpecIntersection
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
            "\"Example\" ) "
             "( <text>S in {bool, int, string, real}, T in {line, points),"
             " mS x mS -> mS, mS x S -> mS, S x mS -> mS, "
             "mpoint x T -> mpoint, T x mpoint -> mpoint\n"
             "mpoint x mpoint -> mpoint</text--->"
             "<text>intersection( _, _ )</text--->"
             "<text>Intersection.</text--->"
             "<text>query intersection (mi1,  mi2)</text--->"
             ") )";

const string unionspec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>R in {region}, P in {point}, mP x R-> mR,"
             " mP x mR -> mR, P x mR -> mR</text--->"
             "<text>_ union _</text--->"
             "<text>Calculates union between the given objects.</text--->"
             "<text>rg1 union mp1</text---> ) )";

const string TemporalLiftSpecMinus
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>S in {bool, int, string, real}, mS x mS -> mS,"
             " mS x S -> mS, S x mS -> mS, "
             "R in {region}, P in {point}, R x mP -> mR, "
             "mR x P -> mR, mR x mP -> mR, mR x points -> mR, "
             "mR x line -> mR</text--->"
             "<text>_ minus _</text--->"
             "<text>Minus.</text--->"
             "<text>query mi1 minus mi2</text--->"
             ") )";

const string rcenterspec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>mregion -> mpoint</text--->"
             "<text>rough_center ( _ )</text--->"
             "<text>Calculates an approach to the"
             "center of gravity of a moving Region.</text--->"
             "<text>rough_center(mrg1)</text---> ) )";

const string ncomponentsspec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>mregion -> mint</text--->"
             "<text>no_components ( _ )</text--->"
             "<text>Calculates the number of faces of "
             "a moving Region.</text--->"
             "<text>no_components(mrg1)</text---> ) )";

const string perimeterspec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>mregion -> mreal</text--->"
             "<text>perimeter ( _ )</text--->"
             "<text>Calculates the perimeter of a moving Region.</text--->"
             "<text>mraperimeter(mrg1)</text---> ) )";

const string areaspec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>mregion -> mreal</text--->"
             "<text>area ( _ )</text--->"
             "<text>Calculates the area of a moving Region.</text--->"
             "<text>area(mrg1)</text---> ) )";

const string TemporalLiftSpecDistance
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>T in {real), mpoint x mpoint -> mreal, mT x mT"
             " -> mreal, mT x T -> mreal, T x mT -> mreal</text--->"
             "<text> distance( _, _ ) </text--->"
             "<text>returns the moving distance</text--->"
             "<text>distance( mpoint1, point1 )</text--->"
             ") )";

const string TemporalLiftSpecAnd
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>bool x mbool -> mbool, mbool x mbool ->"
             " mbool, mbool x bool -> mbool</text--->"
             "<text> _ and _ </text--->"
             "<text>Logical AND for Bool and MBool.</text--->"
             "<text>mb1 and mb2</text--->"
             ") )";

const string TemporalLiftSpecOr
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>bool x mbool -> mbool, mbool x mbool ->"
             " mbool, mbool x bool -> mbool</text--->"
             "<text> _ or _ </text--->"
             "<text>Logical OR for Bool and MBool.</text--->"
             "<text>mb1 or mb2</text--->"
             ") )";

const string TemporalLiftSpecNot
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>mbool -> mbool</text--->"
             "<text> not( _ )</text--->"
             "<text>Negates a MovingBool.</text--->"
             "<text>not(mb1)</text--->"
             ") )";

const string temporalzerospec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>      -> mint</text--->"
             "<text>zero()</text--->"
             "<text>Creates an mint-Object with value 0 from minInstant"
             " to maxInstant</text--->"
             "<text>zero()</text---> ) )";

const string periods2mintSpec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>periods -> mint</text--->"
             "<text>periods2mint( _ )</text--->"
             "<text>Creates an MInt from a periods-object with value 1"
             " for each existing period and 0 for every hole.</text--->"
             "<text>periods2mint(per1)</text---> ) )";


const string createmintSpec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>periods x int -> mint</text--->"
             "<text>createmint( _ )</text--->"
             "<text>Creates a moving int which is defined at the"
             " intervals of the periods value and haves the given"
             " int value during these intervals. " 
             "</text--->"
             "<text>mmint(per1)</text---> ) )";

const string eplusSpec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>mint x mint -> mint</text--->"
             "<text>_ eplus _</text--->"
             "<text>Adds two mint-objects</text--->"
             "<text>mi1 + mi2</text---> ) )";

const string temporalconcatspec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>mpoint x mpoint -> mpoint</text--->"
             "<text>_ _ concat</text--->"
             "<text>Concats two MPoints. DefTime of the first mpoint must"
             " end before deftime of second point!</text--->"
             "<text>mp1 mp2 concat</text---> ) )";

const string TemporalLiftSpecABS
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
            "\"Example\" ) "
            "( <text>mreal -> mreal</text--->"
            "<text>abs( _ )</text--->"
            "<text>abs</text--->"
            "<text>query abs(mr1)</text--->"
            ") )";
/*
1 Definition of operations

*/


static Operator temporalmequal( "=",
                            TemporalLiftSpecMEqual,
                            18,
                            temporalmequalmap,
                            MovingEqualSelect,
                            MovingEqualTypeMapMBool );

static Operator temporalmnotequal( "#",
                            TemporalLiftSpecMNotEqual,
                            18,
                            temporalmnotequalmap,
                            MovingEqualSelect,
                            MovingEqualTypeMapMBool );

static Operator temporalmless ( "<",
                            TemporalLiftSpecLT,
                            12,
                            temporalmlessmap,
                            MovingCompareSelect,
                            MovingCompareTypeMapMBool );

static Operator temporalmlessequal ( "<=",
                            TemporalLiftSpecLE,
                            12,
                            temporalmlessequalmap,
                            MovingCompareSelect,
                            MovingCompareTypeMapMBool );

static Operator temporalmgreater ( ">",
                            TemporalLiftSpecGT,
                            12,
                            temporalmgreatermap,
                            MovingCompareSelect,
                            MovingCompareTypeMapMBool );

static Operator temporalmgreaterequal ( ">=",
                            TemporalLiftSpecGE,
                            12,
                            temporalmgreaterequalmap,
                            MovingCompareSelect,
                            MovingCompareTypeMapMBool );

static Operator temporalsubtract ( "-",
                            TemporalLiftSpecSubtract,
                            12,
                            temporalsubtractmap,
                            MovingAddSelect,
                            MovingAddTypeMap);

static Operator temporaladd ( "+",
                            TemporalLiftSpecAdd,
                            12,
                            temporaladdmap,
                            MovingAddSelect,
                            MovingAddTypeMap);

static Operator temporalmultiply ( "*",
                            TemporalLiftSpecMultiply,
                            11,
                            temporalmultiplymap,
                            MovingMultiplySelect,
                            MovingMultiplyTypeMap);

static Operator temporaldivide ( "/",
                            TemporalLiftSpecDivide,
                            8,
                            temporaldividemap,
                            MovingDivideSelect,
                            MovingDivideTypeMap);

static Operator isempty("isempty",
                            temporalliftisemptyspec,
                            6,
                            temporalliftisemptyvaluemap,
                            TemporalLiftIsemptySelect,
                            TemporalLiftIsemptyTypeMap);

static Operator temporalminside( "inside",
                            TemporalLiftSpecInside,
                            4,
                            temporalliftinsidemap,
                            InsideSelect,
                            InsideTypeMapMBool );

static Operator temporalmintersection( "intersection",
                            TemporalLiftSpecIntersection,
                            17,
                            temporalliftintersectionmap,
                            MovingIntersectionSelect,
                            MovingIntersectionTypeMap );

static Operator munion("union",
                            unionspec,
                            3,
                            unionvaluemap,
                            UnionSelect,
                            UnionTypeMap);

static Operator temporalmminus( "minus",
                            TemporalLiftSpecMinus,
                            20,
                            temporalliftminusmap,
                            MovingMinusSelect,
                            MovingMinusTypeMap );

static Operator rcenter("rough_center",
                            rcenterspec,
                            RCenterValueMap,
                            Operator::SimpleSelect,
                            RCenterTypeMap);

static Operator ncomponents("no_components",
                            ncomponentsspec,
                            NComponentsValueMap,
                            Operator::SimpleSelect,
                            NComponentsTypeMap);

static Operator perimeter("perimeter",
                            perimeterspec,
                            PerimeterValueMap,
                            Operator::SimpleSelect,
                            PerimeterTypeMap);

static Operator area("area",
                            areaspec,
                            AreaValueMap,
                            Operator::SimpleSelect,
                            PerimeterTypeMap);

static Operator temporalmdistance( "distance",
                            TemporalLiftSpecDistance,
                            4,
                            temporaldistancemap,
                            MovingDistanceSelect,
                            MovingDistanceTypeMapMReal );

static Operator temporaland( "and",
                            TemporalLiftSpecAnd,
                            3,
                            temporalandmap,
                            MovingAndOrSelect,
                            AndOrTypeMapMBool );

static Operator temporalor( "or",
                            TemporalLiftSpecOr,
                            3,
                            temporalormap,
                            MovingAndOrSelect,
                            AndOrTypeMapMBool );
static Operator temporalnot( "not",
                            TemporalLiftSpecNot,
                            TemporalNot,
                            Operator::SimpleSelect,
                            MBoolTypeMapMBool );

static Operator temporalzero("zero",
                            temporalzerospec,
                            TemporalZeroValueMap,
                            Operator::SimpleSelect,
                            TemporalZeroTypeMap);

static Operator temporalmint("periods2mint",
                            periods2mintSpec,
                            periods2mintVM,
                            Operator::SimpleSelect,
                            periods2mintTM);

static Operator createmint(
                    "createmint",
                    createmintSpec,
                    createmintVM,
                    Operator::SimpleSelect,
                    createmintTM);


static Operator eplus("eplus",
                       eplusSpec,
                       eplusVM,
                       Operator::SimpleSelect,
                       TemporalPlusTypeMap);

static Operator temporalconcat("concat",
                            temporalconcatspec,
                            TemporalConcatValueMap,
                            Operator::SimpleSelect,
                            TemporalConcatTypeMap);

static Operator temporalabs( "abs",
                            TemporalLiftSpecABS,
                            MovingRealABS,
                            Operator::SimpleSelect,
                            ABSTypeMap );

/*
1 Creating the algebra

*/

class TemporalLiftedAlgebra : public Algebra
{
  public:
    TemporalLiftedAlgebra() : Algebra()
    {
    AddOperator( &temporalmequal );
    AddOperator( &temporalmnotequal );
    AddOperator( &temporalmless);
    AddOperator( &temporalmlessequal);
    AddOperator( &temporalmgreater);
    AddOperator( &temporalmgreaterequal);
    AddOperator( &isempty);

    AddOperator( &temporalminside);
    AddOperator( &temporalmintersection);
    AddOperator( &munion);
    AddOperator( &temporalmminus);

    AddOperator( &rcenter);
    AddOperator( &ncomponents);
    AddOperator( &perimeter);
    AddOperator( &area);
    AddOperator( &temporalmdistance);

    AddOperator( &temporaland );
    AddOperator( &temporalor );
    AddOperator( &temporalnot );

    AddOperator( &temporalzero);
    AddOperator( &temporalmint);
    AddOperator( &createmint);
    AddOperator( &eplus);
    AddOperator( &temporalconcat);
    AddOperator( &temporalabs);
    AddOperator( &temporalsubtract);
    AddOperator( &temporaladd);
    AddOperator( &temporalmultiply);
    AddOperator( &temporaldivide);
    }
    ~TemporalLiftedAlgebra() {}
};

/*

5 Initialization

*/

extern "C"
Algebra*
InitializeTemporalLiftedAlgebra(NestedList *nlRef, QueryProcessor
 *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return (new TemporalLiftedAlgebra());
}
