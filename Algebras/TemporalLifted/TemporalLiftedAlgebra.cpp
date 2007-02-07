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

This implementation was renamed ~RefinementPartition~ and moved to ~TemporalAlgebra.h~.

1 Methods used by the ValueMapping-Functions

1.1 Method ~MPerimeter()~

calculates the perimeter of a MRegion and returns the MReal value as result.

*/
void MPerimeter(MRegion& reg, MReal& res) {
    if(TLA_DEBUG)
      cout<< "MPerimeter() called" << endl;

    int nocomponents = reg.GetNoComponents();

    res.Clear();
    res.StartBulkLoad();
    for(int n = 0; n < nocomponents; n++){
      const URegionEmb *ur;
      UReal ures(true);
      double start = 0.0, end = 0.0;
      reg.Get(n, ur);
      if(!ur->IsValid())
        continue;
      if(TLA_DEBUG){
        cout<<"URegion # "<<n<<" "<<"[ "<<ur->timeInterval.start.ToString()
        <<" "<<ur->timeInterval.end.ToString()<<" ]";}
      ures.timeInterval = ur->timeInterval;
      int number = ur->GetSegmentsNum();
      for(int i = 0; i < number; i++){
        const MSegmentData *dms;
        ur->GetSegment(reg.GetMSegmentData(), i, dms);
        if(dms->GetCycleNo() == 0){ //only outercycles
          start += sqrt(pow(dms->GetInitialStartX() - dms->GetInitialEndX(), 2)
                 + pow(dms->GetInitialStartY() - dms->GetInitialEndY(), 2));
          end +=   sqrt(pow(dms->GetFinalStartX() - dms->GetFinalEndX(), 2)
                 + pow(dms->GetFinalStartY() - dms->GetFinalEndY(), 2));
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
    res.StartBulkLoad();
    for(int n = 0; n < reg.GetNoComponents(); n++){
      const URegionEmb *ur;
      UReal ures(true);
      double at = 0.0, bt = 0.0, ct = 0.0;
      reg.Get(n, ur);
      if(!ur->IsValid())
        continue;
      if(TLA_DEBUG){
        cout<<"URegion # "<<n<<" "<<"[ "<<ur->timeInterval.start.ToString()
        <<" "<<ur->timeInterval.end.ToString()<<" ]";}
      double dt = ur->timeInterval.end.ToDouble()
                - ur->timeInterval.start.ToDouble();
      if (dt == 0.0) continue;
      ures.timeInterval = ur->timeInterval;

      int number = ur->GetSegmentsNum();
      for(int i = 0; i < number; i++){
        const MSegmentData *dms;
        ur->GetSegment(reg.GetMSegmentData(), i, dms);
        double kx1 = (dms->GetFinalStartX() - dms->GetInitialStartX()) / dt;
        double kx2 = (dms->GetFinalEndX()   - dms->GetInitialEndX())   / dt;
        double ky1 = (dms->GetFinalStartY() - dms->GetInitialStartY()) / dt;
        double ky2 = (dms->GetFinalEndY()   - dms->GetInitialEndY())   / dt;

        at += ((kx2 - kx1) * (ky1 + ky2)) / 2;
        bt += (((kx2 - kx1) * (dms->GetInitialStartY()
              + dms->GetInitialEndY()))  + ((dms->GetInitialEndX()
              - dms->GetInitialStartX()) * (ky1 + ky2))) / 2;
        ct += ((dms->GetInitialStartY()  + dms->GetInitialEndY())
             * (dms->GetInitialEndX()    - dms->GetInitialStartX())) / 2;
      }
      ures.a = at;
      ures.b = bt;
      ures.c = ct;
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
    res.StartBulkLoad();
    for(int n = 0; n < reg.GetNoComponents(); n++){
      const URegionEmb *ur;
      double Ainitial = 0.0, Axinitial = 0.0, Ayinitial = 0.0,
             Afinal = 0.0, Axfinal = 0.0, Ayfinal = 0.0;
      reg.Get(n, ur);
      if(!ur->IsValid())
        continue;
      if(TLA_DEBUG){
        cout<<"URegion # "<<n<<" "<<"[ "<<ur->timeInterval.start.ToString()
        <<" "<<ur->timeInterval.end.ToString()<<" ]";}

      int number = ur->GetSegmentsNum();
      const MSegmentData *dms;
      for(int i = 0; i < number; i++){
        ur->GetSegment(reg.GetMSegmentData(), i, dms);

       //Calculate Area of Beginning and End of Unit
       Ainitial += (dms->GetInitialEndX()
                  - dms->GetInitialStartX()) * (dms->GetInitialEndY()
                  + dms->GetInitialStartY()) / 2;
       Afinal   += (dms->GetFinalEndX() - dms->GetFinalStartX())
                 * (dms->GetFinalEndY() + dms->GetFinalStartY()) / 2;

       double initialax, initialbx, finalax, finalbx, initialay,
       initialby, finalay, finalby; //Ax=ax^3+bx^2

       //Calculate momentums of Area
       initialax = (dms->GetInitialStartX() != dms->GetInitialEndX())
                 ? ((dms->GetInitialEndY()   - dms->GetInitialStartY()) / (
                 dms->GetInitialEndX() - dms->GetInitialStartX()) / 3.0) : 0.0;
       initialbx = (dms->GetInitialStartY() - 3.0 * initialax
                 * dms->GetInitialStartX()) / 2.0;
       finalax = (dms->GetFinalStartX() != dms->GetFinalEndX())
                 ? ((dms->GetFinalEndY() - dms->GetFinalStartY()) / (
                 dms->GetFinalEndX() - dms->GetFinalStartX()) / 3.0) : 0.0;
       finalbx = (dms->GetFinalStartY() - 3.0 * finalax
                 * dms->GetFinalStartX()) / 2.0;

       initialay = (dms->GetInitialStartY() != dms->GetInitialEndY())
                 ? ((dms->GetInitialEndX()   - dms->GetInitialStartX()) / (
                 dms->GetInitialEndY() - dms->GetInitialStartY()) / 3.0) : 0.0;
       initialby = (dms->GetInitialStartX() - 3.0 * initialay
                 * dms->GetInitialStartY()) / 2.0;
       finalay = (dms->GetFinalStartY() != dms->GetFinalEndY())
                 ? ((dms->GetFinalEndX() - dms->GetFinalStartX()) / (
                 dms->GetFinalEndY() - dms->GetFinalStartY()) / 3.0) : 0.0;
       finalby = (dms->GetFinalStartX() - 3.0 * finalay
                 * dms->GetFinalStartY()) / 2.0;

       Axinitial += initialax * pow(dms->GetInitialEndX(), 3)
                  + initialbx * pow(dms->GetInitialEndX(), 2)
                  - initialax * pow(dms->GetInitialStartX(), 3)
                  - initialbx * pow(dms->GetInitialStartX(), 2);

       Axfinal     += finalax * pow(dms->GetFinalEndX(), 3)
                    + finalbx * pow(dms->GetFinalEndX(), 2)
                    - finalax * pow(dms->GetFinalStartX(), 3) - finalbx
                    * pow(dms->GetFinalStartX(),2 );

       Ayinitial += initialay * pow(dms->GetInitialEndY(), 3)
                  + initialby * pow(dms->GetInitialEndY(), 2)
                  - initialay * pow(dms->GetInitialStartY(), 3)
                  - initialby * pow(dms->GetInitialStartY(),2 );

       Ayfinal     += finalay * pow(dms->GetFinalEndY(), 3)
                    + finalby * pow(dms->GetFinalEndY(), 2)
                    - finalay * pow(dms->GetFinalStartY(), 3) - finalby
                    * pow(dms->GetFinalStartY(),2 );
      }
      if ((Ainitial != 0.0) || (Afinal != 0.0)){
        UPoint *ures;
        if((Ainitial != 0.0) && (Afinal != 0.0)) {
          ures = new UPoint(ur->timeInterval, (Axinitial / Ainitial),
             (-Ayinitial / Ainitial), (Axfinal / Afinal), (-Ayfinal / Afinal));
        }
        else if (Ainitial == 0.0) {
          ures = new UPoint(ur->timeInterval, dms->GetInitialStartX(),
             dms->GetInitialStartY(), (Axfinal / Afinal), (-Ayfinal / Afinal));
        }
        else {
          ures = new UPoint(ur->timeInterval, (Axinitial / Ainitial),
             (-Ayinitial / Ainitial), dms->GetFinalStartX(),
             dms->GetFinalStartY());
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

It gets for each URegion the last segment and givs the FaceNo of this segment,
because the last segment ist on the last face .

*/
void NComponents(MRegion& reg, MInt& res) {
    if(TLA_DEBUG)
      cout<< "NComponents() called" << endl;

    res.Clear();
    res.StartBulkLoad();
    for(int n = 0; n < reg.GetNoComponents(); n++){
      const URegionEmb *ur;
      reg.Get(n, ur);
      if(!ur->IsValid())
        continue;
      if(TLA_DEBUG){
        cout<<"URegion # "<<n<<" "<<"[ "<<ur->timeInterval.start.ToString()
        <<" "<<ur->timeInterval.end.ToString()<<" ]";}
      const MSegmentData *dms;
      ur->GetSegment(reg.GetMSegmentData(),ur->GetSegmentsNum() - 1, dms);
      CcInt *constVal = new CcInt(true, dms->GetFaceNo() + 1);
      UInt *ures = new UInt(ur->timeInterval, *constVal);

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
bool CompareValue(const ConstTemporalUnit<Alpha>& n,
 const ConstTemporalUnit<Alpha>& i, int vers )
{
   if(TLA_DEBUG)
     cout<<"ConstTemporalUnit<Alpha> CompareValue "<<vers<<endl;
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
bool CompareValue(const ConstTemporalUnit<Alpha>& n, Alpha& i, int vers )
{
   if(TLA_DEBUG)
     cout<<"ConstTemporalUnit<Alpha> CompareValue "<<vers<<endl;
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
void DistanceMPoint( MPoint& p1, MPoint& p2, MReal& result)
{
  UReal uReal(true);

  if(TLA_DEBUG)
    cout<<"DistanceMPoint called"<<endl;
  RefinementPartition<MPoint, MPoint, UPoint, UPoint> rp(p1, p2);
  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  result.Clear();
  result.StartBulkLoad();
  for( unsigned int i = 0; i < rp.Size(); i++ )
  {
    Interval<Instant>* iv;
    int u1Pos, u2Pos;
    const UPoint *u1;
    const UPoint *u2;

    rp.Get(i, iv, u1Pos, u2Pos);
    if(TLA_DEBUG){
      cout<< "Compare interval #"<< i<< ": ["<< iv->start.ToString()<< " "
      << iv->end.ToString()<< " "<< iv->lc<< " " << iv->rc<< "] "
      << u1Pos<< " "<< u2Pos<< endl;}

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<endl;
      p1.Get(u1Pos, u1);
      p2.Get(u2Pos, u2);
    }
    if(u1->IsDefined() && u2->IsDefined())
    { // do not need to test for overlapping deftimes anymore...
      u1->Distance( *u2, uReal );
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

Returns true if the value of these two uReals holds the comparison.
The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/
static void CompareUReal(UReal u1, UReal u2, UBool& uBool, int op)
{
    DateTime middle, tmp;
    CcReal value1, value2;
    double mid;
    if(TLA_DEBUG){
      cout<<"CompareUReal "<<op<<" in ["<<uBool.timeInterval.start.ToString()
      <<" "<<uBool.timeInterval.end.ToString()<<" "<<uBool.timeInterval.lc
      <<" "<<uBool.timeInterval.rc<<"]"<<endl;}

    //mid = (uBool.timeInterval.start.ToDouble()
         //+ uBool.timeInterval.end.ToDouble())/2.0;
    tmp = uBool.timeInterval.start;
    tmp.SetType(durationtype);
    mid = (tmp + uBool.timeInterval.end).ToDouble()/2.0;
    middle.ReadFrom(mid);
    middle.SetType(instanttype);
    u1.TemporalFunction(middle,value1,true);
    u2.TemporalFunction(middle,value2,true);
    if(TLA_DEBUG)
      cout<<value1.GetRealval()<<" =?= "<<value2.GetRealval()<<endl;
    if (AlmostEqual(value1.GetRealval(), value2.GetRealval())) {
      if (op >= -1 && op <= 1) uBool.constValue.Set(true,true);
      else uBool.constValue.Set(true,false);
    }
    else if (value1.Compare(&value2) == -1) {
      if (op < 0) uBool.constValue.Set(true,true);
      else uBool.constValue.Set(true,false);
    }
    else if (value1.Compare(&value2) == 1) {
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
static void ShiftUReal(UReal& op, Instant newstart)
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
static void MRealDistanceMM(MReal& op1, MReal& op2, MReal& result)
{
  if(TLA_DEBUG)
    cout<<"MRealDistanceMM called"<<endl;
  UReal uReal(true);

  RefinementPartition<MReal, MReal, UReal, UReal> rp(op1, op2);
  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  result.Clear();
  result.StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant>* iv;
    int u1Pos;
    int u2Pos;
    const UReal *u1transfer;
    const UReal *u2transfer;
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
      <<iv->start.ToString()<<"  "<<iv->end.ToString()
      <<" "<<iv->lc<<" "<<iv->rc<<"]"<<endl;}
    op1.Get(u1Pos, u1transfer);
    op2.Get(u2Pos, u2transfer);
    if(!(u1transfer->IsDefined() && u2transfer->IsDefined()))
    {
      resultIsValid = false;
      break;
    }
    u1 = *u1transfer;
    u2 = *u2transfer;
    if ( u1.r || u2.r )
    {
      resultIsValid = false;
      break;
    }

    ShiftUReal(u1, iv->start);
    ShiftUReal(u2, iv->start);
    u1.timeInterval = *iv;
    u2.timeInterval = *iv;

    numPartRes = u1.Distance(u2, partResVector);
    for(int j=0; j<numPartRes; j++)
      result.MergeAdd(partResVector[j]);
  }
  result.EndBulkLoad(false); // should already be sorted
}


/*
1.1 Method ~MovingRealCompareMM~

Returns true if the value of these two mReals holds the comparison.
The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/
static void MovingRealCompareMM(MReal& op1, MReal& op2, MBool&
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
    Interval<Instant>* iv;
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
      <<iv->start.ToString()<<" "<<iv->end.ToString()<<" "<<iv->lc<< " "
      <<iv->rc<<"] "<<u1Pos<< " "<<u2Pos<<"  op "<<op<<endl;
    op1.Get(u1Pos, u1transfer);
    op2.Get(u2Pos, u2transfer);
    if(!(u1transfer->IsDefined() && u2transfer->IsDefined()))
        continue;
    u1 = *u1transfer;
    u2 = *u2transfer;
    ShiftUReal(u1, iv->start);
    ShiftUReal(u2, iv->start);
    u1.timeInterval = *iv;
    u2.timeInterval = *iv;
    Instant t[4];
    Instant middle;
    int counter = 0;

    int number = FindEqualTimes4Real(u1, u2, t);

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
      if (iv->start < t[0]) {
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
          uBool.timeInterval.end = iv->end;
          uBool.timeInterval.rc = iv->rc;
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
}

/*
1.1 Method ~MovingRealIntersectionMM~

Calculates the intersecion between two given mReals

*/
static void MovingRealIntersectionMM(MReal& op1, MReal& op2,
 MReal& result, int op)
{
 if(TLA_DEBUG)
   cout<<"MovingRealIntersectionMM called"<<endl;
 UReal un(true);

 RefinementPartition<MReal, MReal, UReal, UReal> rp(op1, op2);
 if(TLA_DEBUG)
   cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

 result.Clear();
 result.StartBulkLoad();
 for(unsigned int i = 0; i < rp.Size(); i++)
 {
   Interval<Instant>* iv;
   int u1Pos;
   int u2Pos;
   const UReal *u1transfer;
   const UReal *u2transfer;
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
    if(!(u1transfer->IsDefined() && u2transfer->IsDefined()))
        continue;

    u1 = *u1transfer;
    u2 = *u2transfer;

    ShiftUReal(u1, iv->start);
    ShiftUReal(u2, iv->start);
    u1.timeInterval = *iv;
    u2.timeInterval = *iv;

    Instant t[4];
    Instant middle;
    int counter = 0;
    int number = FindEqualTimes4Real(u1, u2, t);

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
    UBool uBool(true);
    uBool.timeInterval = *iv;
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

        un.timeInterval = *iv;  //to take boarders
        result.MergeAdd(un);
      }
    }
    else {
      if (op == 1) {
        for (int m = 0; m < counter; m++){
          if ((t[m] > iv->start || iv->lc) && (t[m] < iv->end
          || iv->rc)){
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
        if (t[0] > iv->start){
          un = u1;
          un.timeInterval = *iv;
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
            un.timeInterval.end = iv->end;
            un.timeInterval.rc = iv->rc;
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
static void MovingRealCompareMS(MReal& op1,CcReal& op2, MBool&
 result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingRealCompareMS called"<<endl;
  MReal *mop2 = new MReal(0);
  const UReal *up1;

  mop2->Clear();
  mop2->StartBulkLoad();
  for (int i = 0; i < op1.GetNoComponents(); i++) {
    op1.Get(i, up1);
    if(!(up1->IsDefined() && op2.IsDefined()))
        continue;
    UReal *up2 = new UReal(up1->timeInterval, 0.0, 0.0, up1->r
               ? pow(op2.GetRealval(),2) : op2.GetRealval(), up1->r);
    mop2->Add(*up2);
    delete up2;
  }
  mop2->EndBulkLoad(false);
  MovingRealCompareMM(op1, *mop2, result, op);

  delete mop2;

}

/*
1.1 Method ~MPointInsideLine~

calcultates the periods where the given MPoint lies
inside the given Line. It return the existing intervals in a Periods-Object.

*/
static void MPointInsideLine(MPoint& mp, Line& ln, Periods& pResult)
{
  if(TLA_DEBUG)
    cout<<"MPointLineInside called"<<endl;
  const UPoint *up;
  const HalfSegment *l;

  Periods* period = new Periods(0);
  Periods* between = new Periods(0);
  Point pt;
  Interval<Instant> newper; //part of the result

  pResult.Clear();
  for( int i = 0; i < mp.GetNoComponents(); i++)
  {

    mp.Get(i, up);
    if(TLA_DEBUG){
      cout<<"UPoint # "<<i<<" ["<<up->timeInterval.start.ToString()<<" "
      <<up->timeInterval.end.ToString()<<" "<<up->timeInterval.lc<<" "
      <<up->timeInterval.rc<<"] ("<<up->p0.GetX()<<" "<<up->p0.GetY()
      <<")->("<<up->p1.GetX()<<" "<<up->p1.GetY()<<")"<<endl;}

    for( int n = 0; n < ln.Size(); n++)
    {
      Instant t;
      ln.Get(n, l);
      if(TLA_DEBUG){
        cout<<"UPoint # "<<i<<" ["<<up->timeInterval.start.ToString()
        <<" "<<up->timeInterval.end.ToString()<<" "<<up->timeInterval.lc
        <<" "<<up->timeInterval.rc<<"] ("<<up->p0.GetX()<<" "<<up->p0.GetY()
        <<")->("<<up->p1.GetX()<<" "<<up->p1.GetY()<<")"<<endl;
        cout<<"l      # "<<n<<" ("<<l->GetLeftPoint().GetX()
        <<" "<<l->GetLeftPoint().GetY()
        <<" "<<l->GetRightPoint().GetX()<<" "
        <<l->GetRightPoint().GetY()<<") "<<endl;}
      if (l->GetRightPoint().GetX() == l->GetDomPoint().GetX()
       && l->GetRightPoint().GetY() == l->GetDomPoint().GetY()) {
        if(TLA_DEBUG)
          cout<<"right point is dominating -> continue"<<endl;
        continue;
      }
      if(( l->GetRightPoint().GetX() < up->p0.GetX()
       &&  l->GetRightPoint().GetX() < up->p1.GetX())
       || (l->GetLeftPoint().GetX() > up->p0.GetX()
       &&  l->GetLeftPoint().GetX() > up->p1.GetX())
       || (l->GetRightPoint().GetY() < up->p0.GetY()
       &&  l->GetRightPoint().GetY() < up->p1.GetY()
       && (l->GetLeftPoint().GetY() < up->p0.GetY()
       &&  l->GetLeftPoint().GetY() < up->p1.GetY()))
       || (l->GetRightPoint().GetY() > up->p0.GetY()
       &&  l->GetRightPoint().GetY() > up->p1.GetY()
       && (l->GetLeftPoint().GetY() > up->p0.GetY()
       &&  l->GetLeftPoint().GetY() > up->p1.GetY()))) {
        if(TLA_DEBUG)
          cout<<"Bounding Boxes not crossing!"<<endl;
        continue;
      }
      double al, bl, aup, bup;
      bool vl, vup;
      vl = l->GetRightPoint().GetX() == l->GetLeftPoint().GetX();
      if(!vl){
        al = (l->GetRightPoint().GetY() - l->GetLeftPoint().GetY())
           / (l->GetRightPoint().GetX() - l->GetLeftPoint().GetX());
        bl =  l->GetLeftPoint().GetY() - l->GetLeftPoint().GetX() * al;
          if(TLA_DEBUG)
            cout<<"al: "<<al<<" bl: "<<bl<<endl;
      }
      else
        if(TLA_DEBUG)
          cout<<"l is vertical"<<endl;
      vup = up->p1.GetX() == up->p0.GetX();
      if(!vup){
        aup = (up->p1.GetY() - up->p0.GetY())
            / (up->p1.GetX() - up->p0.GetX());
        bup =  up->p0.GetY() - up->p0.GetX() * aup;
        if(TLA_DEBUG)
          cout<<"aup: "<<aup<<" bup: "<<bup<<endl;
      }
      else
        if(TLA_DEBUG)
          cout<<"up is vertical"<<endl;
      if(vl && vup){
        if(TLA_DEBUG)
          cout<<"both elements are vertical!"<<endl;
        if(up->p1.GetX() != l->GetLeftPoint().GetX()){
        if(TLA_DEBUG)
          cout<<"elements are vertical but not at same line"<<endl;
          continue;
        }
        else {
          if(TLA_DEBUG)
            cout<<"elements on same line"<<endl;
          if(up->p1.GetY() < l->GetLeftPoint().GetY()
           && up->p0.GetY() < l->GetLeftPoint().GetY()){
            if(TLA_DEBUG)
              cout<<"uPoint lower as linesegment"<<endl;
            continue;
          }
          else if(up->p1.GetY() > l->GetRightPoint().GetY()
           && up->p0.GetY() > l->GetRightPoint().GetY()){
            if(TLA_DEBUG)
              cout<<"uPoint higher as linesegment"<<endl;
            continue;
          }
          else{
            if(TLA_DEBUG)
              cout<<"uPoint and linesegment partequal"<<endl;
            if (up->p0.GetY() <= l->GetLeftPoint().GetY()
             && up->p1.GetY() >= l->GetLeftPoint().GetY()){
              if(TLA_DEBUG)
                cout<<"uPoint starts below linesegemet"<<endl;
              t.ReadFrom((l->GetLeftPoint().GetY() - up->p0.GetY())
                     / (up->p1.GetY() - up->p0.GetY())
                     * (up->timeInterval.end.ToDouble()
                     -  up->timeInterval.start.ToDouble())
                     +  up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
              if(TLA_DEBUG)
                cout<<"t "<<t.ToString()<<endl;
              newper.start = t;
              newper.lc = (up->timeInterval.start == t)
                         ? up->timeInterval.lc : true;
            }
            if(up->p1.GetY() <= l->GetLeftPoint().GetY()
             && up->p0.GetY() >= l->GetLeftPoint().GetY()){
              if(TLA_DEBUG)
                cout<<"uPoint ends below linesegemet"<<endl;
              t.ReadFrom((l->GetLeftPoint().GetY() - up->p0.GetY())
                      / (up->p1.GetY() - up->p0.GetY())
                      * (up->timeInterval.end.ToDouble()
                      -  up->timeInterval.start.ToDouble())
                      +  up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
              if(TLA_DEBUG)
                cout<<"t "<<t.ToString()<<endl;
              newper.end = t;
              newper.rc = (up->timeInterval.end == t)
                         ? up->timeInterval.rc : true;
            }
            if(up->p0.GetY() <= l->GetRightPoint().GetY()
             && up->p1.GetY() >= l->GetRightPoint().GetY()){
              if(TLA_DEBUG)
                cout<<"uPoint ends above linesegemet"<<endl;
              t.ReadFrom((l->GetRightPoint().GetY() - up->p0.GetY())
                      / (up->p1.GetY() - up->p0.GetY())
                      * (up->timeInterval.end.ToDouble()
                      -  up->timeInterval.start.ToDouble())
                      +  up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
              if(TLA_DEBUG)
                cout<<"t "<<t.ToString()<<endl;
              newper.end = t;
              newper.rc = (up->timeInterval.end == t)
                         ? up->timeInterval.rc : true;
            }
            if(up->p1.GetY() <= l->GetRightPoint().GetY()
             && up->p0.GetY() >= l->GetRightPoint().GetY()){
              if(TLA_DEBUG)
                cout<<"uPoint starts above linesegemet"<<endl;
              t.ReadFrom((l->GetRightPoint().GetY() - up->p0.GetY())
                      / (up->p1.GetY() - up->p0.GetY())
                      * (up->timeInterval.end.ToDouble()
                      - up->timeInterval.start.ToDouble())
                      + up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
              if(TLA_DEBUG)
                cout<<"t "<<t.ToString()<<endl;
              newper.start = t;
              newper.lc = (up->timeInterval.start == t)
                         ? up->timeInterval.lc : true;
            }
            if (up->p0.GetY() <= l->GetRightPoint().GetY()
             && up->p0.GetY() >= l->GetLeftPoint().GetY()){
              if(TLA_DEBUG)
                cout<<"uPoint starts inside linesegemet"<<endl;
              newper.start = up->timeInterval.start;
              newper.lc =    up->timeInterval.lc;
            }
            if( up->p1.GetY() <= l->GetRightPoint().GetY()
             && up->p1.GetY() >= l->GetLeftPoint().GetY()){
              if(TLA_DEBUG)
                cout<<"uPoint ends inside linesegemet"<<endl;
              newper.end = up->timeInterval.end;
              newper.rc =  up->timeInterval.rc;
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
        t.ReadFrom((l->GetRightPoint().GetX() - up->p0.GetX())
                / (up->p1.GetX() - up->p0.GetX())
                * (up->timeInterval.end.ToDouble()
                -  up->timeInterval.start.ToDouble())
                +  up->timeInterval.start.ToDouble());
        t.SetType(instanttype);
        if(TLA_DEBUG)
          cout<<"t "<<t.ToString()<<endl;
        if((up->timeInterval.start == t && !up->timeInterval.lc)
         ||  (up->timeInterval.end == t && !up->timeInterval.rc))
          continue;

        if(up->timeInterval.start > t|| up->timeInterval.end < t){
          if(TLA_DEBUG)
            cout<<"up outside line"<<endl;
          continue;
        }
        up->TemporalFunction(t, pt, true);
        if(  pt.GetX() < l->GetLeftPoint().GetX() ||
             pt.GetX() > l->GetRightPoint().GetX()
         || (pt.GetY() < l->GetLeftPoint().GetY() &&
             pt.GetY() < l->GetRightPoint().GetY())
         || (pt.GetY() > l->GetLeftPoint().GetY() &&
             pt.GetY() > l->GetRightPoint().GetY())){
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
        if(up->p1.GetY() != up->p0.GetY()) {
          t.ReadFrom((up->p0.GetX() * al + bl - up->p0.GetY())
                  / (up->p1.GetY() - up->p0.GetY())
                  * (up->timeInterval.end.ToDouble()
                  -  up->timeInterval.start.ToDouble())
                  +  up->timeInterval.start.ToDouble());
          t.SetType(instanttype);
          if(TLA_DEBUG)
            cout<<"t "<<t.ToString()<<endl;
          if((up->timeInterval.start == t && !up->timeInterval.lc)
           ||  (up->timeInterval.end == t && !up->timeInterval.rc)){
            if(TLA_DEBUG)
              cout<<"continue"<<endl;
            continue;
          }

          if(up->timeInterval.start > t|| up->timeInterval.end < t){
            if(TLA_DEBUG)
              cout<<"up outside line"<<endl;
            continue;
          }
          up->TemporalFunction(t, pt, true);
          if(  pt.GetX() < l->GetLeftPoint().GetX() ||
               pt.GetX() > l->GetRightPoint().GetX()
           || (pt.GetY() < l->GetLeftPoint().GetY() &&
               pt.GetY() < l->GetRightPoint().GetY())
           || (pt.GetY() > l->GetLeftPoint().GetY() &&
               pt.GetY() > l->GetRightPoint().GetY())){
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
          if(al * up->p1.GetX() + bl == up->p1.GetY()){
            if(TLA_DEBUG)
              cout<<"Point lies on line"<<endl;
            newper = up->timeInterval;
          }
          else {
            if(TLA_DEBUG)
              cout<<"continue 2"<<endl;
            continue;
          }
        }
      }
      else if(aup == al){
        if(TLA_DEBUG)
          cout<<"both lines have same gradient"<<endl;
        if(bup != bl){
          if(TLA_DEBUG)
            cout<<"colinear but not equal"<<endl;
          continue;
        }
         if(up->p0.GetX() <= l->GetLeftPoint().GetX()
         && up->p1.GetX() >= l->GetLeftPoint().GetX()){
           if(TLA_DEBUG)
             cout<<"uPoint starts left of linesegemet"<<endl;
           t.ReadFrom((l->GetLeftPoint().GetX() - up->p0.GetX())
                   / (up->p1.GetX() - up->p0.GetX())
                   * (up->timeInterval.end.ToDouble()
                   -  up->timeInterval.start.ToDouble())
                   +  up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
           if(TLA_DEBUG)
             cout<<"t "<<t.ToString()<<endl;
           newper.start = t;
           newper.lc = (up->timeInterval.start == t)
                      ? up->timeInterval.lc : true;
        }
        if(up->p1.GetX() <= l->GetLeftPoint().GetX()
        && up->p0.GetX() >= l->GetLeftPoint().GetX()){
           if(TLA_DEBUG)
             cout<<"uPoint ends left of linesegemet"<<endl;
           t.ReadFrom((l->GetLeftPoint().GetX() - up->p0.GetX())
                   / (up->p1.GetX() - up->p0.GetX())
                   * (up->timeInterval.end.ToDouble()
                   -  up->timeInterval.start.ToDouble())
                   +  up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
           if(TLA_DEBUG)
             cout<<"t "<<t.ToString()<<endl;
           newper.end = t;
           newper.rc = (up->timeInterval.end == t)
                      ? up->timeInterval.rc : true;
        }
        if(up->p0.GetX() <= l->GetRightPoint().GetX()
        && up->p1.GetX() >= l->GetRightPoint().GetX()){
           if(TLA_DEBUG)
             cout<<"uPoint ends right of linesegemet"<<endl;
           t.ReadFrom((l->GetRightPoint().GetX() - up->p0.GetX())
                   / (up->p1.GetX() - up->p0.GetX())
                   * (up->timeInterval.end.ToDouble()
                   -  up->timeInterval.start.ToDouble())
                   +  up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
           if(TLA_DEBUG)
             cout<<"t "<<t.ToString()<<endl;
           newper.end = t;
           newper.rc = (up->timeInterval.end == t)
                      ? up->timeInterval.rc : true;
        }
        if(up->p1.GetX() <= l->GetRightPoint().GetX()
        && up->p0.GetX() >= l->GetRightPoint().GetX()){
           if(TLA_DEBUG)
             cout<<"uPoint starts right of linesegemet"<<endl;
           t.ReadFrom((l->GetRightPoint().GetX() - up->p0.GetX())
                   / (up->p1.GetX() - up->p0.GetX())
                   * (up->timeInterval.end.ToDouble()
                   -  up->timeInterval.start.ToDouble())
                   +  up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
           if(TLA_DEBUG)
             cout<<"t "<<t.ToString()<<endl;
           newper.start = t;
           newper.lc = (up->timeInterval.start == t)
                      ? up->timeInterval.lc : true;
        }
        if(up->p0.GetX() <= l->GetRightPoint().GetX()
        && up->p0.GetX() >= l->GetLeftPoint().GetX()){
           if(TLA_DEBUG)
             cout<<"uPoint starts inside linesegemet"<<endl;
           newper.start = up->timeInterval.start;
           newper.lc =    up->timeInterval.lc;
        }
        if(up->p1.GetX() <= l->GetRightPoint().GetX()
        && up->p1.GetX() >= l->GetLeftPoint().GetX()){
           if(TLA_DEBUG)
             cout<<"uPoint ends inside linesegemet"<<endl;
           newper.end = up->timeInterval.end;
           newper.rc =  up->timeInterval.rc;
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
        t.ReadFrom(((bl - bup) / (aup - al) - up->p0.GetX())
                / (up->p1.GetX() - up->p0.GetX())
                * (up->timeInterval.end.ToDouble()
                -  up->timeInterval.start.ToDouble())
                +  up->timeInterval.start.ToDouble());
        t.SetType(instanttype);
        if((up->timeInterval.start == t && !up->timeInterval.lc)
         ||  (up->timeInterval.end == t && !up->timeInterval.rc)){
          if(TLA_DEBUG)
            cout<<"continue"<<endl;
          continue;
        }

        if(up->timeInterval.start > t|| up->timeInterval.end < t){
          if(TLA_DEBUG)
            cout<<"up outside line"<<endl;
          continue;
        }
        up->TemporalFunction(t, pt, true);
        if(  pt.GetX() < l->GetLeftPoint().GetX() ||
             pt.GetX() > l->GetRightPoint().GetX()
         || (pt.GetY() < l->GetLeftPoint().GetY() &&
             pt.GetY() < l->GetRightPoint().GetY())
         || (pt.GetY() > l->GetLeftPoint().GetY() &&
             pt.GetY() > l->GetRightPoint().GetY())){
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
  delete between;
  delete period;
}

/*
1.1 Method ~MPointInMPoint~

Checks whether the two mpoints are crossing. It returns -1.0 if the mpoints are
not crossing and 2.0 if both mpoints are equal all the time. In all other cases
it return thr fraction of iv when mpoints are crossing (0.0 .. 1.0).

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
void MovingPointCompareMM( MPoint& p1, MPoint& p2, MBool& result,
 int op)
{
  UBool uBool(true);

  if(TLA_DEBUG)
    cout<<"MovingPointCompareMM called"<<endl;
  RefinementPartition<MPoint, MPoint, UPoint, UPoint> rp(p1, p2);
  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  result.Clear();
  result.StartBulkLoad();
  for( unsigned int i = 0; i < rp.Size(); i++ )
  {
    Interval<Instant>* iv;
    int u1Pos;
    int u2Pos;
    const UPoint *u1;
    const UPoint *u2;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<" ["
        <<iv->start.ToString()<<" "<<iv->end.ToString()<<" "<<iv->lc
        <<" "<<iv->rc<< "] "<<u1Pos<< " "<<u2Pos<<endl;
      p1.Get(u1Pos, u1);
      p2.Get(u2Pos, u2);
      if(!(u1->IsDefined() && u2->IsDefined()))
        continue;
    }

    Point rp0, rp1, rp2, rp3;

    u1->TemporalFunction(iv->start, rp0, true);
    u1->TemporalFunction(iv->end, rp1, true);
    u2->TemporalFunction(iv->start, rp2, true);
    u2->TemporalFunction(iv->end, rp3, true);

    double t = MPointInMPoint(rp0.GetX(), rp0.GetY(), rp1.GetX(), rp1.GetY(),
                              rp2.GetX(), rp2.GetY(), rp3.GetX(), rp3.GetY());
    if(TLA_DEBUG)
      cout<<"t "<<t<<endl;

    if(t == 2.0){
      uBool.timeInterval = *iv;
      uBool.constValue.Set(true, op == 0 ? true : false);
      result.MergeAdd( uBool );
    }
    else if(t == 0.0){
      if (iv->lc) {
        uBool.timeInterval.start = iv->start;
        uBool.timeInterval.end = iv->start;
        uBool.timeInterval.lc = true;
        uBool.timeInterval.rc = true;
        uBool.constValue.Set(true, op == 0 ? true : false);
        result.MergeAdd( uBool );
      }
      uBool.timeInterval = *iv;
      uBool.timeInterval.lc = false;
      uBool.constValue.Set(true, op == 0 ? false : true);
      if(uBool.IsValid())
        result.MergeAdd( uBool );

    }
    else if(t == 1.0){
      uBool.timeInterval = *iv;
      uBool.timeInterval.rc = false;
      uBool.constValue.Set(true, op == 0 ? false : true);
      if(uBool.IsValid())
        result.MergeAdd( uBool );
      if (iv->rc) {
        uBool.timeInterval.start = iv->end;
        uBool.timeInterval.end = iv->end;
        uBool.timeInterval.lc = true;
        uBool.timeInterval.rc = true;
        uBool.constValue.Set(true, op == 0 ? true : false);
        result.MergeAdd( uBool );
      }
    }
    else if(t > 0.0 && t < 1.0){
      Instant time;
      time.ReadFrom(t  * (iv->end.ToDouble() - iv->start.ToDouble())
                       + iv->start.ToDouble());
      time.SetType(instanttype);
      if ((*iv).Contains(time)) {
        uBool.timeInterval = *iv;
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
        uBool.timeInterval.rc = iv->rc;
        uBool.timeInterval.start = time;
        uBool.timeInterval.end = iv->end;
        uBool.constValue.Set(true, op == 0 ? false : true);
        if(uBool.IsValid())
          result.MergeAdd( uBool );
      }
      else{
        uBool.timeInterval = *iv;
        uBool.constValue.Set(true, op == 0 ? false : true);
        result.MergeAdd( uBool );
      }
    }
    else{
      uBool.timeInterval = *iv;
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
void MovingPointCompareMS( MPoint& p1, Point& p2, MBool& result,
 int op)
{
  UBool uBool(true);

  result.Clear();
  result.StartBulkLoad();
  for( int i = 0; i < p1.GetNoComponents(); i++ )
  {
    Interval<Instant> iv;
    const UPoint *u1;

    p1.Get(i, u1);
    if(!(u1->IsDefined() && p2.IsDefined()))
        continue;
    iv = u1->timeInterval;
    if(TLA_DEBUG){
      cout<< "Compare interval #"<< i<< ": "<< iv.start.ToString()<< " "
      << iv.end.ToString()<< " "<< iv.lc<< endl;}

    Point rp0, rp1;

    double t = MPointInMPoint(u1->p0.GetX(), u1->p0.GetY(),
                              u1->p1.GetX(), u1->p1.GetY(),
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
      Instant time;
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
void MovingRegionCompareMS( MRegion *mr, Region *r, MBool *result,
 int op)
{
  const URegionEmb *ur;
  UBool uBool(true);   //Part of the result

  result->Clear();
  result->StartBulkLoad();
  if(TLA_DEBUG)
    cout<<"MovingRegionCompareMS called"<<endl;
  for(int i = 0; i < mr->GetNoComponents(); i++){
    mr->Get(i, ur);
    if(!(ur->IsValid() && r->IsDefined()))
        continue;
    int number = ur->GetSegmentsNum();
    if(TLA_DEBUG){
      cout<<"URegion # "<<i<<" "<<"[ "<<ur->timeInterval.start.ToString()<<" "
      <<ur->timeInterval.end.ToString()<<" ]"<<endl;
      cout<<"number of segments = "<< number<<endl;}

    bool staticequal = true;
    bool finish = false;
    const MSegmentData *dms;
    int i = 0;
    while(staticequal && (i < ur->GetSegmentsNum())){
      ur->GetSegment(mr->GetMSegmentData(), i, dms);
      if (dms->GetInitialStartX() == dms->GetFinalStartX()
       && dms->GetInitialStartY() == dms->GetFinalStartY()
       && dms->GetInitialEndX()   == dms->GetFinalEndX()
       && dms->GetInitialEndY()   == dms->GetFinalEndY()){
        if(TLA_DEBUG)
          cout<<"s is static"<<endl;
        //find matching CHalfsegment set staticequal!!
        Point *p1 = new Point(true,
                    dms->GetFinalStartX(), dms->GetFinalStartY());
        Point *p2 = new Point(true,
                    dms->GetFinalEndX(), dms->GetFinalEndY());
        HalfSegment *nHS = new HalfSegment(true, *p1, *p2);
        delete p1;
        delete p2;
        const HalfSegment *mid;
        bool found = false;
        int left = 0;
        int right = r->Size();
        int middle;
        while(!found && left != right){
          middle = (left + right) / 2;
          r->Get(middle, mid);
          if(*mid == *nHS)
            found = true;
          else if(*mid < *nHS){
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
      uBool.timeInterval = ur->timeInterval;
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
      uBool.timeInterval = ur->timeInterval;
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
      const HalfSegment *chs;
      Periods* period  = new Periods(0);
      Periods* between = new Periods(0);
      Periods* pResult = new Periods(0);
      Interval<Instant> newper; //part of the result

      for(int i = 0 ;i < r->Size(); i++){
        r->Get(i, chs);

        double tsd = MPointInMPoint(dms->GetInitialStartX(),
          dms->GetInitialStartY(), dms->GetFinalStartX(),
          dms->GetFinalStartY(), chs->GetDomPoint().GetX(),
          chs->GetDomPoint().GetY(), chs->GetDomPoint().GetX(),
          chs->GetDomPoint().GetY());
        double ted = MPointInMPoint(dms->GetInitialEndX(),
          dms->GetInitialEndY(), dms->GetFinalEndX(),
          dms->GetFinalEndY(), chs->GetDomPoint().GetX(),
          chs->GetDomPoint().GetY(), chs->GetDomPoint().GetX(),
          chs->GetDomPoint().GetY());
        double tss = MPointInMPoint(dms->GetInitialStartX(),
          dms->GetInitialStartY(), dms->GetFinalStartX(),
          dms->GetFinalStartY(), chs->GetSecPoint().GetX(),
          chs->GetSecPoint().GetY(), chs->GetSecPoint().GetX(),
          chs->GetSecPoint().GetY());
        double tes = MPointInMPoint(dms->GetInitialEndX(),
          dms->GetInitialEndY(), dms->GetFinalEndX(),
          dms->GetFinalEndY(), chs->GetSecPoint().GetX(),
          chs->GetSecPoint().GetY(), chs->GetSecPoint().GetX(),
          chs->GetSecPoint().GetY());

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
         || (tpoint == 0.0 && ur->timeInterval.lc)
         || (tpoint == 1.0 && ur->timeInterval.rc)){
          Instant t;
          t.ReadFrom((ur->timeInterval.end.ToDouble()
                    - ur->timeInterval.start.ToDouble()) * tpoint
                    + ur->timeInterval.start.ToDouble());
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
      delete period;
      delete between;
      const Interval<Instant> *per;
      for(int i = 0; i < pResult->GetNoComponents(); i++){
        Region snapshot(0);
        if(TLA_DEBUG)
          cout<<"add interval # "<<i<<endl;
        pResult->Get(i, per);
        ur->TemporalFunction(mr->GetMSegmentData(), per->start, snapshot, true);
        if(*r == snapshot){
          if(TLA_DEBUG)
            cout<<"r == snapshot!"<<endl;
          if(per->start > ur->timeInterval.start){
            uBool.timeInterval.start = ur->timeInterval.start;
            uBool.timeInterval.lc = ur->timeInterval.lc;
            uBool.timeInterval.end = per->start;
            uBool.timeInterval.rc = !per->rc;
            uBool.constValue.Set(true, (op == 0) ? false : true);
            if(TLA_DEBUG)
              cout<<"uBool "<<uBool.constValue.GetBoolval()
              <<" ["<<uBool.timeInterval.start.ToString()
              <<" "<<uBool.timeInterval.end.ToString()<<" "
              <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;
            result->MergeAdd(uBool);
          }
          uBool.timeInterval = *per;
          uBool.constValue.Set(true, (op == 0) ? true : false);
          if(TLA_DEBUG)
            cout<<"uBool "<<uBool.constValue.GetBoolval()
            <<" ["<<uBool.timeInterval.start.ToString()
            <<" "<<uBool.timeInterval.end.ToString()<<" "
            <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;
          result->MergeAdd(uBool);
          if(per->end < ur->timeInterval.end){
            uBool.timeInterval.start = per->end;
            uBool.timeInterval.lc = !per->lc;
            uBool.timeInterval.end = ur->timeInterval.end;
            uBool.timeInterval.rc = ur->timeInterval.rc;
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
      delete pResult;
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
  if(TLA_DEBUG)
    cout<<"MovingRegionCompareMM called"<<endl;
  RefinementPartition<MRegion, MRegion, URegionEmb, URegionEmb>
     rp(*mr1, *mr2);
  if(TLA_DEBUG)
    cout<<"RefimentPartiion done with size "<<rp.Size()<<endl;
  Interval<Instant>* iv;
  int reg1Pos;
  int reg2Pos;
  UBool uBool(true);

  result->Clear();
  result->StartBulkLoad();

  for( unsigned int i = 0; i < rp.Size(); i++ ){
    rp.Get(i, iv, reg1Pos, reg2Pos);
    if(TLA_DEBUG)
      cout<<"interval # "<<i<<" "<<reg1Pos<<" "<<reg2Pos<<endl;
    if(reg1Pos == -1 || reg2Pos == -1)
      continue;
    if(TLA_DEBUG){
      cout<<"bothoperators in iv # "<<i<<" [ "
      <<iv->start.ToString()<<" "<<iv->end.ToString()<<" "<<iv->lc<<" "
      <<iv->rc<<" ] reg1Pos "<<reg1Pos<<", reg2Pos "<<reg2Pos <<endl;}
    const URegionEmb *ureg1;
    const URegionEmb *ureg2;
    mr1->Get(reg1Pos, ureg1);
    mr2->Get(reg2Pos, ureg2);
    if(!(ureg1->IsValid() && ureg2->IsValid()))
        continue;
    if(ureg1->GetSegmentsNum() != ureg1->GetSegmentsNum()){
      uBool.timeInterval = *iv;
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
    if((ureg1->GetSegmentsNum() == 0)&&(ureg1->GetSegmentsNum() == 0)){
      uBool.timeInterval = *iv;
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
    const MSegmentData *dms1;
    Periods* period  = new Periods(0);
    Periods* between = new Periods(0);
    Periods* pResult = new Periods(0);
    Interval<Instant> newper; //part of the result
    bool uregionPerhapsEqual = false;
    MSegmentData rdms1;

    ureg1->GetSegment(mr1->GetMSegmentData(), 0, dms1);
    dms1->restrictToInterval(ureg1->timeInterval, *iv, rdms1);

    pResult->Clear();
    for(int n = 0; n < ureg2->GetSegmentsNum(); n++){
      const MSegmentData *dms2;
      MSegmentData rdms2;

      ureg2->GetSegment(mr2->GetMSegmentData(), n, dms2);
      dms2->restrictToInterval(ureg2->timeInterval, *iv, rdms2);
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
        if((ts == 0.0 && !iv->lc) || (ts == 1.0 && !iv->rc))
          continue;
        Instant t;
        t.ReadFrom((iv->end.ToDouble() - iv->start.ToDouble()) * ts
                  + iv->start.ToDouble());
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
    delete period;
    delete between;

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
      Instant time;
      time.ReadFrom(0.1  * (iv->end.ToDouble() - iv->start.ToDouble())
                         + iv->start.ToDouble());
      time.SetType(instanttype);
      ureg1->TemporalFunction(mr1->GetMSegmentData(), time, snapshot1, true);
      ureg2->TemporalFunction(mr2->GetMSegmentData(), time, snapshot2, true);
      if(snapshot1 == snapshot2){
        if(TLA_DEBUG)
          cout<<"snapshots of iv->start are equal"<<endl;
        time.ReadFrom(0.1  * (iv->end.ToDouble() - iv->start.ToDouble())
                           + iv->start.ToDouble());
        time.SetType(instanttype);
        ureg1->TemporalFunction(mr1->GetMSegmentData(), time, snapshot1, true);
        ureg2->TemporalFunction(mr2->GetMSegmentData(), time, snapshot2, true);
        if(snapshot1 == snapshot2){

          uBool.timeInterval = *iv;
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
    const Interval<Instant> *per;
    bool finished = false;
    for(int i = 0; i < pResult->GetNoComponents(); i++){
      Region snapshot1(0);
      Region snapshot2(0);
      pResult->Get(i, per);
      if(TLA_DEBUG)
        cout<<"test time # "<<i<<" "<<per->start.ToString()<<endl;
      if ((per->start == ureg1->timeInterval.start && !ureg1->timeInterval.lc)
       || (per->start == ureg2->timeInterval.start && !ureg2->timeInterval.lc)
       || (per->start == ureg1->timeInterval.end   && !ureg1->timeInterval.rc)
       || (per->start == ureg2->timeInterval.end   && !ureg2->timeInterval.rc))
        /*
        no snapshot possible, so this uregions can not be equal at this time!

        */
        continue;
      ureg1->TemporalFunction(mr1->GetMSegmentData(),per->start,snapshot1,true);
      ureg2->TemporalFunction(mr2->GetMSegmentData(),per->start,snapshot2,true);
      if(snapshot1 == snapshot2){
        if(TLA_DEBUG)
          cout<<"snapshot equal!"<<endl;
        if(per->start > iv->start){
          uBool.timeInterval.start = iv->start;
          uBool.timeInterval.lc = iv->lc;
          uBool.timeInterval.end = per->start;
          uBool.timeInterval.rc = !per->rc;
          uBool.constValue.Set(true, (op == 0) ? false : true);
          if(TLA_DEBUG){
            cout<<"uBool "<<uBool.constValue.GetBoolval()
            <<" ["<<uBool.timeInterval.start.ToString()
            <<" "<<uBool.timeInterval.end.ToString()<<" "
            <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
          result->MergeAdd(uBool);
        }
        uBool.timeInterval = *per;
        uBool.constValue.Set(true, (op == 0) ? true : false);
        if(TLA_DEBUG){
          cout<<"uBool "<<uBool.constValue.GetBoolval()
          <<" ["<<uBool.timeInterval.start.ToString()
          <<" "<<uBool.timeInterval.end.ToString()<<" "
          <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
        result->MergeAdd(uBool);
        if(per->end < iv->end){
          uBool.timeInterval.start = per->end;
          uBool.timeInterval.lc = !per->lc;
          uBool.timeInterval.end = iv->end;
          uBool.timeInterval.rc = iv->rc;
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
    delete pResult;
    if(!finished){
      uBool.timeInterval = *iv;
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
pResult as uBool with value ~true~ and added the difference to the MPoint-
intervals with ~false~.

*/
static void CompletePeriods2MBool(MPoint* mp, Periods* pResult,
  MBool* endResult){
  const UPoint *up;

  endResult->Clear();
  endResult->StartBulkLoad();
  const Interval<Instant> *per;
  UBool uBool(true);
  int m = 0;
  bool pfinished = (pResult->GetNoComponents() == 0);
  for ( int i = 0; i < mp->GetNoComponents(); i++) {
    mp->Get(i, up);
    if(!up->IsDefined())
        continue;
    if(TLA_DEBUG){
      cout<<"UPoint # "<<i<<" ["<<up->timeInterval.start.ToString()
      <<" "<<up->timeInterval.end.ToString()<<" "<<up->timeInterval.lc<<" "
      <<up->timeInterval.rc<<"] ("<<up->p0.GetX()<<" "<<up->p0.GetY()<<")->("
      <<up->p1.GetX()<<" "<<up->p1.GetY()<<")"<<endl;}
    if(!pfinished) {
      pResult->Get(m, per);
      if(TLA_DEBUG){
        cout<<"per "<<m<<" ["<<per->start.ToString()<<" "
        <<per->end.ToString()<<" "<<per->lc<<" "<<per->rc<<"]"<<endl;}
    }
    else
      if(TLA_DEBUG)
        cout<<"no per any more"<<endl;
    if(pfinished || up->timeInterval.end < per->start
      || (up->timeInterval.end == per->start
      && !up->timeInterval.rc && per->lc)) {
       uBool.constValue.Set(true, false);
       uBool.timeInterval = up->timeInterval;
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
      if(up->timeInterval.start < per->start
       || (up->timeInterval.start == per->start
       && up->timeInterval.lc && !per->lc)) {
        uBool.constValue.Set(true, false);
        uBool.timeInterval.start = up->timeInterval.start;
        uBool.timeInterval.lc = up->timeInterval.lc;
        uBool.timeInterval.end = per->start;
        uBool.timeInterval.rc = !per->lc;
        if(TLA_DEBUG){
          cout<<"up starts before up"<<endl;
          cout<<"MergeAdd2 "<<uBool.constValue.GetBoolval()
          <<" ["<<uBool.timeInterval.start.ToString()<<" "
          <<uBool.timeInterval.end.ToString()<<" "
          <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
        endResult->MergeAdd(uBool);
        uBool.timeInterval = *per;
      }
      else {
        if(TLA_DEBUG)
          cout<<"per starts before or with up"<<endl;
        uBool.timeInterval.start = up->timeInterval.start;
        uBool.timeInterval.lc = up->timeInterval.lc;
      }
      while(true) {
        uBool.constValue.Set(true, true);
        if(up->timeInterval.end < per->end
         || (up->timeInterval.end == per->end
         && per->rc && !up->timeInterval.rc)) {
            uBool.timeInterval.end = up->timeInterval.end;
            uBool.timeInterval.rc = up->timeInterval.rc;
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

          uBool.timeInterval.end = per->end;
          uBool.timeInterval.rc = per->rc;
          if(TLA_DEBUG){
            cout<<"per ends inside up"<<endl;
            cout<<"MergeAdd4 "<<uBool.constValue.GetBoolval()
            <<" ["<<uBool.timeInterval.start.ToString()<<" "
            <<uBool.timeInterval.end.ToString()<<" "
            <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
          endResult->MergeAdd(uBool);
        }
        uBool.timeInterval.start = per->end;
        uBool.timeInterval.lc = !per->rc;
        if(m == pResult->GetNoComponents() - 1){
          pfinished = true;
        }
        else {
          pResult->Get(++m, per);
          if(TLA_DEBUG){
            cout<<"per "<<m<<" ["<<per->start.ToString()
            <<" "<<per->end.ToString()<<" "<<per->lc<<" "<<per->rc<<"]"<<endl;}
        }

        if(!pfinished && (per->start < up->timeInterval.end
         || (per->start == up->timeInterval.end
         && up->timeInterval.rc && per->rc))){
          uBool.timeInterval.end = per->start;
          uBool.timeInterval.rc = !per->lc;
          uBool.constValue.Set(true, false);
          if(TLA_DEBUG){
            cout<<"next per starts in same up"<<endl;
            cout<<"MergeAdd6 "<<uBool.constValue.GetBoolval()
            <<" ["<<uBool.timeInterval.start.ToString()<<" "
            <<uBool.timeInterval.end.ToString()<<" "
            <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;}
          endResult->MergeAdd(uBool);
          uBool.timeInterval.start = per->start;
          uBool.timeInterval.lc = per->lc;
        }
        else {
          if(TLA_DEBUG)
            cout<<"next interval after up -> finish up"<<endl;
          uBool.timeInterval.end = up->timeInterval.end;
          uBool.timeInterval.rc = up->timeInterval.rc;
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
static void CompletePeriods2MPoint(MPoint* mp, Periods* pResult,
  MPoint* endResult){
  if(TLA_DEBUG)
    cout<<"CompletePeriods2MPoint called"<<endl;
  const UPoint *up;

  endResult->Clear();
  endResult->StartBulkLoad();
  const Interval<Instant> *per;
  UPoint newUp(true);
  Point pt;
  int m = 0;
  bool pfinished = (pResult->GetNoComponents() == 0);
  for ( int i = 0; i < mp->GetNoComponents(); i++) {
    mp->Get(i, up);
    if(!up->IsDefined())
        continue;
    if(TLA_DEBUG){
      cout<<"UPoint # "<<i<<" ["<<up->timeInterval.start.ToString()
      <<" "<<up->timeInterval.end.ToString()<<" "<<up->timeInterval.lc<<" "
      <<up->timeInterval.rc<<"] ("<<up->p0.GetX()<<" "<<up->p0.GetY()<<")->("
      <<up->p1.GetX()<<" "<<up->p1.GetY()<<")"<<endl;}
    if(!pfinished) {
      pResult->Get(m, per);
      if(TLA_DEBUG){
        cout<<"per "<<m<<" ["<<per->start.ToString()<<" "
        <<per->end.ToString()<<" "<<per->lc<<" "<<per->rc<<"]"<<endl;}
    }
    if(pfinished) {
      if(TLA_DEBUG)
        cout<<"no per any more. break 1"<<endl;
      break;
    }
    if(!(pfinished || up->timeInterval.end < per->start
     || (up->timeInterval.end == per->start
     && !up->timeInterval.rc && per->lc))) {
      if(TLA_DEBUG)
        cout<<"per not totally after up"<<endl;
      if(up->timeInterval.start < per->start
       || (up->timeInterval.start == per->start
       && up->timeInterval.lc && !per->lc)) {
        if(TLA_DEBUG)
          cout<<"up starts before per"<<endl;
        newUp.timeInterval = *per;
      }
      else {
        if(TLA_DEBUG)
          cout<<"per starts before or with up"<<endl;
        newUp.timeInterval.start = up->timeInterval.start;
        newUp.timeInterval.lc = up->timeInterval.lc;
      }
      while(true) {
        if(up->timeInterval.end < per->end
         || (up->timeInterval.end == per->end
         && per->rc && !up->timeInterval.rc)) {
            if(TLA_DEBUG)
              cout<<"per ends after up (break)"<<endl;
            newUp.timeInterval.end = up->timeInterval.end;
            newUp.timeInterval.rc = up->timeInterval.rc;
            up->TemporalFunction(newUp.timeInterval.start, pt, true);
            newUp.p0 = pt;
            up->TemporalFunction(newUp.timeInterval.end, pt, true);
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
          newUp.timeInterval.end = per->end;
          newUp.timeInterval.rc = per->rc;
          up->TemporalFunction(newUp.timeInterval.start, pt, true);
          newUp.p0 = pt;
          up->TemporalFunction(newUp.timeInterval.end, pt,true);
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
            cout<<"per "<<m<<" ["<<per->start.ToString()
            <<" "<<per->end.ToString()<<" "<<per->lc<<" "<<per->rc<<"]"<<endl;}
        }
        if(!pfinished && (per->start < up->timeInterval.end
           || (per->start == up->timeInterval.end
           && up->timeInterval.rc && per->rc))){
          if(TLA_DEBUG)
            cout<<"next per starts in same up"<<endl;
          newUp.timeInterval.start = per->start;
          newUp.timeInterval.lc = per->lc;
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
static void MPointInsidePoints(MPoint& mp, Points& ps, Periods& pResult)
{
  if(TLA_DEBUG)
    cout<<"MPointPointsInside called"<<endl;
  const UPoint *up;
  const Point *p;

  pResult.Clear();
  Periods* between = new Periods(0);
  Periods* period = new Periods(0);
  Interval<Instant> newper; //part of the result
  bool newtime;

  for( int i = 0; i < mp.GetNoComponents(); i++)
  {
    mp.Get(i, up);
    if(!up->IsDefined())
        continue;
    for( int n = 0; n < ps.Size(); n++)
    {
      newtime = false;
      ps.Get(n, p);
      if(!p->IsDefined())
        continue;
      double time = MPointInMPoint(up->p0.GetX(), up->p0.GetY(), up->p1.GetX(),
                    up->p1.GetY(), p->GetX(), p->GetY(), p->GetX(), p->GetY());

      if(time == 2.0){
         newper = up->timeInterval;
         newtime = true;
      }
      else if((time  > 0.0 && time < 1.0)
              || (time == 0.0 && up->timeInterval.lc)
              || (time == 1.0 && up->timeInterval.rc)){
        Instant t;
        t.ReadFrom((up->timeInterval.end.ToDouble()
                  - up->timeInterval.start.ToDouble()) * time
                  + up->timeInterval.start.ToDouble());
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
  delete between;
  delete period;
}

/*
1.1 Method ~TransformMBool2MPoint~

Completes a MBool to a MPoint-value for the ~minus~ operator. For this it adds
the starting and end points to every interval when mBool is not true, even when
there is no mBool at all.

*/
static void TransformMBool2MPoint(MPoint *mp, MBool *mBool, MPoint *endResult)
{
  const UPoint *up;

  endResult->Clear();
  endResult->StartBulkLoad();


  const UBool *ub;
  UPoint newUp(true);
  Point pt;
  int pos = 0;


  if(TLA_DEBUG)
    cout<<"TransformMBool2MPoint1 called"<<endl;


  for ( int i = 0; i < mBool->GetNoComponents(); i++) {
    mBool->Get(i, ub);
    if(!ub->IsDefined())
        continue;
    if(TLA_DEBUG)
    {
      cout<<"UBool # "<<i<<" ["<<ub->timeInterval.start.ToString()
      <<" "<<ub->timeInterval.end.ToString()<<" "
      <<ub->timeInterval.lc<<" "<<ub->timeInterval.rc<<"] "
      <<ub->constValue.GetBoolval()<<endl;
    }

    if(ub->constValue.GetBoolval())
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
        cout<<"UPoint # "<<pos<<" ["<<up->timeInterval.start.ToString()
        <<" "<<up->timeInterval.end.ToString()<<" "
        <<up->timeInterval.lc<<" "<<ub->timeInterval.rc<<"] "<<endl;
      }
      while(!(up->timeInterval.end > ub->timeInterval.start
       || (up->timeInterval.end == ub->timeInterval.start
       && up->timeInterval.rc && ub->timeInterval.lc))
       && pos < mp->GetNoComponents())
      {
        pos++;
        mp->Get(pos, up);
        if(TLA_DEBUG)
        {
          cout<<"UPoint # "<<pos<<" ["<<up->timeInterval.start.ToString()
          <<" "<<up->timeInterval.end.ToString()<<" "
          <<up->timeInterval.lc<<" "<<ub->timeInterval.rc<<"] "<<endl;
        }
      }

      if(up->timeInterval.start < ub->timeInterval.start
       || (up->timeInterval.start == ub->timeInterval.start
       && (up->timeInterval.lc
       || (!up->timeInterval.lc && !ub->timeInterval.lc))))
      {   //upoint started before ubool or at same time
        if(up->timeInterval.end > ub->timeInterval.end)
        { //upoint ends after ubool

           newUp.timeInterval = ub->timeInterval;
           up->TemporalFunction(newUp.timeInterval.start, pt, true);
           newUp.p0 = pt;
           up->TemporalFunction(newUp.timeInterval.end, pt, true);
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
          newUp.timeInterval.start = ub->timeInterval.start;
          newUp.timeInterval.lc    = ub->timeInterval.lc;
          newUp.timeInterval.end   = up->timeInterval.end;
          newUp.timeInterval.rc    = up->timeInterval.rc;

          up->TemporalFunction(newUp.timeInterval.start, pt, true);
          newUp.p0 = pt;
          up->TemporalFunction(newUp.timeInterval.end, pt, true);
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
            cout<<"UPoint # "<<pos<<" ["<<up->timeInterval.start.ToString()
            <<" "<<up->timeInterval.end.ToString()<<" "
            <<up->timeInterval.lc<<" "<<ub->timeInterval.rc<<"] "<<endl;
          }
          while((up->timeInterval.end < ub->timeInterval.end
           || (up->timeInterval.end == ub->timeInterval.end
           && !(!up->timeInterval.rc && ub->timeInterval.rc)))
           && pos < mp->GetNoComponents())
          {  //upoint end before ubool
            newUp.timeInterval = up->timeInterval;
            newUp.p0 = up->p0;
            newUp.p1 = up->p1;
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
              cout<<"UPoint # "<<pos<<" ["<<up->timeInterval.start.ToString()
              <<" "<<up->timeInterval.end.ToString()<<" "
              <<up->timeInterval.lc<<" "<<ub->timeInterval.rc<<"] "<<endl;
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
static void TransformMBool2MPoint(Point *p, MBool *mBool, MPoint *endResult)
{
  endResult->Clear();
  endResult->StartBulkLoad();
  const UBool *ub;
  UPoint newUp(true);

  if(TLA_DEBUG)
    cout<<"TransformMBool2MPoint2 called"<<endl;
  for ( int i = 0; i < mBool->GetNoComponents(); i++) {
    mBool->Get(i, ub);
    if(!ub->IsDefined())
        continue;
    if(TLA_DEBUG){
      cout<<"UBool # "<<i<<" ["<<ub->timeInterval.start.ToString()
      <<" "<<ub->timeInterval.end.ToString()<<" "
      <<ub->timeInterval.lc<<" "<<ub->timeInterval.rc<<"] "
      <<ub->constValue.GetBoolval()<<endl;}
    if(ub->constValue.GetBoolval()){
     if(TLA_DEBUG)
       cout<<"point and mpoint are equal ignore timeInterval"<<endl;
    }
    else{
      if(TLA_DEBUG)
        cout<<"point and mpoint are not equal take timeInterval"<<endl;
      newUp.timeInterval = ub->timeInterval;
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
static void MovingBoolMMOperators( MBool& op1, MBool& op2,
 MBool& result, int op )
{
  if(TLA_DEBUG)
    cout<<"MovingBoolMMOperators called"<<endl;
  UBool uBool(true);  //part of the Result

  RefinementPartition<MBool, MBool, UBool, UBool> rp(op1, op2);
  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;
  result.Clear();
  result.StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant>* iv;
    int u1Pos;
    int u2Pos;
    const UBool *u1transfer;
    const UBool *u2transfer;
    UBool u1(true);
    UBool u2(true);

    rp.Get(i, iv, u1Pos, u2Pos);
    if(TLA_DEBUG){
      cout<< "and/or interval #"<< i<< ": "<< iv->start.ToString()<< " "
      << iv->end.ToString()<< " "<< iv->lc<< " "<< iv->rc<< " "
      << u1Pos<< " "<< u2Pos<< endl;}

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i
        <<" ["<< iv->start.ToString()<< " "<< iv->end.ToString()<< " "
        << iv->lc<< " "<< iv->rc<< " "<< u1Pos<< " "<< u2Pos<< endl;
      op1.Get(u1Pos, u1transfer);
      op2.Get(u2Pos, u2transfer);
      if(!(u1transfer->IsDefined() && u2transfer->IsDefined()))
        continue;
      u1 = *u1transfer;
      u2 = *u2transfer;
    }
    uBool.timeInterval = *iv;

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
static void MovingBoolMSOperators( MBool& op1, CcBool& op2,
 MBool& result, int op )
{
  if(TLA_DEBUG)
    cout<<"MovingBoolMSOperators called"<<endl;
  UBool uBool(true);  //part of the Result
  const UBool *u1transfer;

  result.Clear();
  result.StartBulkLoad();
  for( int i = 0; i < op1.GetNoComponents(); i++)
  {
    if(TLA_DEBUG)
      cout<<"temporalMSLogic "<<op<<" ,# "<<i<<endl;
    op1.Get(i, u1transfer);
    if(!(u1transfer->IsDefined() && op2.IsDefined()))
        continue;
    uBool = *u1transfer;
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
1.1 Method ~MovingCompareBoolMM~

Compares the two operators in the given way: The comparisons are
-3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/
template <class Mapping1, class Mapping2, class Unit1, class
  Unit2>
static void MovingCompareBoolMM( Mapping1& op1, Mapping2& op2,
  MBool& result, int op )
{
  if(TLA_DEBUG)
    cout<<"MovingCompareBoolMM called"<<endl;
  UBool uBool(true);  //part of the Result

  RefinementPartition<Mapping1, Mapping2, Unit1, Unit2>
  rp(op1, op2);
  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;
  result.Clear();
  result.StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant>* iv;
    int u1Pos;
    int u2Pos;
    const Unit1 *u1;
    const Unit2 *u2;

    rp.Get(i, iv, u1Pos, u2Pos);
    if(TLA_DEBUG){
      cout<< "Compare interval #"<< i<< ": "<< iv->start.ToString()<< " "
      << iv->end.ToString()<< " "<< iv->lc<< " "<< iv->rc<< " "
      << u1Pos<< " "<< u2Pos<< endl;}

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<" ["
        << iv->start.ToString()<< " "<< iv->end.ToString()<< " "<< iv->lc
        << "] "<< iv->rc<< " "<< u1Pos<< " "<< u2Pos<< endl;
      op1.Get(u1Pos, u1);
      op2.Get(u2Pos, u2);
      if(!(u1->IsDefined() && u1->IsDefined()))
        continue;
    }
    uBool.timeInterval = *iv;
    uBool.constValue.Set(true,CompareValue(*u1,*u2,op));
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
static void MovingIntersectionMM( Mapping1& op1, Mapping2& op2,
 Mapping1& result, int op)
{
  if(TLA_DEBUG)
    cout<<"MovingIntersectionMM called"<<endl;

  Unit1 un;  //part of the Result

  RefinementPartition<Mapping1, Mapping2, Unit1, Unit2>
  rp(op1, op2);

  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  result.Clear();
  result.StartBulkLoad();

  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant>* iv;

    int u1Pos;
    int u2Pos;

    Unit1 u1;
    Unit2 u2;

    const Unit1 *u1transfer;
    const Unit2 *u2transfer;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (u1Pos == -1 || u2Pos == -1 )
      continue;

    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<" ["
        << iv->start.ToString()<< " "<< iv->end.ToString()<< " "<< iv->lc
        << " "<< iv->rc<< "] "<< u1Pos<< " "<< u2Pos<< endl;

      op1.Get(u1Pos, u1transfer);
      op2.Get(u2Pos, u2transfer);

      if(!(u1transfer->IsDefined() && u2transfer->IsDefined()))
        continue;

      u1 = *u1transfer;
      u2 = *u2transfer;

      if ((op == 1 && u1.EqualValue(u2)) || (op == 2
      && !u1.EqualValue(u2))){
        un.constValue = u1.constValue;
        un.timeInterval = *iv;

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
static void MovingCompareBoolMS( Mapping1& op1, Operator2& op2,
 MBool& result, int op )
{
  if(TLA_DEBUG)
    cout<<"MovingCompareBoolMS called"<<endl;
  UBool uBool(true);  //part of the Result
  const Unit1 *u1;

  result.Clear();
  result.StartBulkLoad();
  for(int i = 0; i < op1.GetNoComponents(); i++)
  {
     op1.Get(i, u1);
     if(!(u1->IsDefined() && op2.IsDefined()))
        continue;
     uBool.timeInterval = u1->timeInterval;
     uBool.constValue.Set(true,CompareValue(*u1,op2,op));
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
1.1 Method ~MRealABS~

Calculates the absolut value of a mReal.

*/
static void MRealABS(MReal& op, MReal& result)
{
  vector<UReal> partResult;
  int numPartResult = 0;
  result.Clear();
  result.StartBulkLoad();
  for(int i = 0; i < op.GetNoComponents(); i++)
  {
    const UReal *u1;
    op.Get(i, u1);
    numPartResult = u1->Abs(partResult);
    for(int j=0; j<numPartResult; j++)
      result.MergeAdd(partResult[j]);
  }
  result.EndBulkLoad(false);
}


/*
1.1 Method ~copyMRegion~

Copies the MRegion to result.

*/

void copyMRegion(MRegion& res, MRegion& result) {
   result.CopyFrom(&res);
}

/*
1.1 Method ~copyRegionMPoint~

Transform the region to a mregion and restrics it to deftime(mpoint).

*/

void copyRegionMPoint(Region& reg, MPoint& pt, MRegion& result) {
   MRegion* res = new MRegion(pt, reg);
   result.Clear();
   result.CopyFrom(res);
   delete res;
}

/*
1.1 Method ~copyMRegionMPoint~

copies the MRegion to result and restrics it to deftime(mpoint).
Method is not implemented complete, because ~restrictMRegion2periods~ is not
implemented yet.

*/

void copyMRegionMPoint(MRegion& reg, MPoint& pt, MRegion& result) {
    RefinementPartition<MRegion, MPoint, URegionEmb, UPoint> rp(reg,pt);
    Interval<Instant>* iv;
    int regPos;
    int ptPos;
    Periods* per = new Periods(0);
    Interval<Instant> newper;
    per->Clear();
    per->StartBulkLoad();
    for( unsigned int i = 0; i < rp.Size(); i++ ){
      rp.Get(i, iv, regPos, ptPos);
      if(regPos == -1 || ptPos == -1)
        continue;
      if(TLA_DEBUG){
        cout<<"bothoperators in iv # "<<i<<" [ "
        <<iv->start.ToString()<<" "<<iv->end.ToString()<<" "
        <<iv->lc<<" "<<iv->rc<<" ] regPos "<<regPos<<endl;}
      const URegionEmb *ureg;
      const UPoint *up;
      reg.Get(regPos, ureg);
      if(!ureg->IsValid())
        continue;
      pt.Get(ptPos, up);
      if(!up->IsDefined())
        continue;
      newper = *iv;
      per->Add(newper);
    }
    per->EndBulkLoad(0);
    Periods* pers = new Periods(0);
    pers->Clear();
    per->Merge(*pers);
    delete per;
    result.Clear();
    //restrictMRegion2periods(reg, pers, result);
    //not possible, because it is not implemented yet.
    delete pers;
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

    if( nl->IsEqual( arg1, "mbool" ) )
      return (nl->SymbolAtom( "mbool" ));
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "mbool" )
    && nl->IsEqual( arg2, "mbool" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mbool" )
    && nl->IsEqual( arg2, "bool" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "bool" )
    && nl->IsEqual( arg2, "mbool" ) )
      return (nl->SymbolAtom( "mbool" ));
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "mbool" )
     && nl->IsEqual( arg2, "mbool" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mbool" )
     && nl->IsEqual( arg2, "bool" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "bool" )
     && nl->IsEqual( arg2, "mbool" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mint" )
     && nl->IsEqual( arg2, "mint" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mint" )
     && nl->IsEqual( arg2, "int" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "int" )
     && nl->IsEqual( arg2, "mint" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mstring" )
     && nl->IsEqual( arg2, "mstring" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mstring" )
     && nl->IsEqual( arg2, "string" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "string" )
     && nl->IsEqual( arg2, "mstring" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mreal" )
     && nl->IsEqual( arg2, "mreal" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mreal" )
     && nl->IsEqual( arg2, "real" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "real" )
     && nl->IsEqual( arg2, "mreal" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mpoint" )
     && nl->IsEqual( arg2, "mpoint" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mpoint" )
     && nl->IsEqual( arg2, "point" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "point" )
     && nl->IsEqual( arg2, "mpoint" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "movingregion" )
     && nl->IsEqual( arg2, "region" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "region" )
     && nl->IsEqual( arg2, "movingregion" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "movingregion" )
     && nl->IsEqual( arg2, "movingregion" ) )
      return (nl->SymbolAtom( "mbool" ));

  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "mbool" )
     && nl->IsEqual( arg2, "mbool" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mbool" )
     && nl->IsEqual( arg2, "bool" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "bool" )
     && nl->IsEqual( arg2, "mbool" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mint" )
     && nl->IsEqual( arg2, "mint" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mint" )
     && nl->IsEqual( arg2, "int" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "int" )
     && nl->IsEqual( arg2, "mint" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mreal" )
     && nl->IsEqual( arg2, "mreal" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mreal" )
     && nl->IsEqual( arg2, "real" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "real" )
     && nl->IsEqual( arg2, "mreal" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mstring" )
     && nl->IsEqual( arg2, "mstring" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mstring" )
     && nl->IsEqual( arg2, "string" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "string" )
     && nl->IsEqual( arg2, "mstring" ) )
      return (nl->SymbolAtom( "mbool" ));

  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "mpoint" )
     && nl->IsEqual( arg2, "mpoint" ) )
      return nl->SymbolAtom( "mreal" );
    if( nl->IsEqual( arg1, "mreal" )
     && nl->IsEqual( arg2, "mreal" ) )
      return nl->SymbolAtom( "mreal" );
    if( nl->IsEqual( arg1, "mreal" )
     && nl->IsEqual( arg2, "real" ) )
      return nl->SymbolAtom( "mreal" );
    if( nl->IsEqual( arg1, "real" )
     && nl->IsEqual( arg2, "mreal" ) )
      return nl->SymbolAtom( "mreal" );

  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "mbool" )
     && nl->IsEqual( arg2, "mbool" ) )
      return nl->SymbolAtom( "mbool" );
    if( nl->IsEqual( arg1, "mbool" )
     && nl->IsEqual( arg2, "bool" ) )
      return nl->SymbolAtom( "mbool" );
    if( nl->IsEqual( arg1, "bool" )
     && nl->IsEqual( arg2, "mbool" ) )
      return nl->SymbolAtom( "mbool" );
    if( nl->IsEqual( arg1, "mint" )
     && nl->IsEqual( arg2, "mint" ) )
      return nl->SymbolAtom( "mint" );
    if( nl->IsEqual( arg1, "mint" )
     && nl->IsEqual( arg2, "int" ) )
      return nl->SymbolAtom( "mint" );
    if( nl->IsEqual( arg1, "int" )
     && nl->IsEqual( arg2, "mint" ) )
      return nl->SymbolAtom( "mint" );
    if( nl->IsEqual( arg1, "mreal" )
     && nl->IsEqual( arg2, "mreal" ) )
      return nl->SymbolAtom( "mreal" );
    if( nl->IsEqual( arg1, "mreal" )
     && nl->IsEqual( arg2, "real" ) )
      return nl->SymbolAtom( "mreal" );
    if( nl->IsEqual( arg1, "real" )
     && nl->IsEqual( arg2, "mreal" ) )
      return nl->SymbolAtom( "mreal" );
    if( nl->IsEqual( arg1, "mpoint" )
     && nl->IsEqual( arg2, "points" ) )
      return nl->SymbolAtom( "mpoint" );
    if( nl->IsEqual( arg1, "mpoint" )
     && nl->IsEqual( arg2, "line" ) )
      return nl->SymbolAtom( "mpoint" );
    if( nl->IsEqual( arg1, "points" )
     && nl->IsEqual( arg2, "mpoint" ) )
      return nl->SymbolAtom( "mpoint" );
    if( nl->IsEqual( arg1, "mpoint" )
     && nl->IsEqual( arg2, "mpoint" ) )
      return nl->SymbolAtom( "mpoint" );
    if( nl->IsEqual( arg1, "line" )
     && nl->IsEqual( arg2, "mpoint" ) )
      return nl->SymbolAtom( "mpoint" );
    if( nl->IsEqual( arg1, "mstring" )
     && nl->IsEqual( arg2, "mstring" ) )
      return nl->SymbolAtom( "mstring" );
    if( nl->IsEqual( arg1, "mstring" )
     && nl->IsEqual( arg2, "string" ) )
      return nl->SymbolAtom( "mstring" );
    if( nl->IsEqual( arg1, "string" )
     && nl->IsEqual( arg2, "mstring" ) )
      return nl->SymbolAtom( "mstring" );
  }
  return nl->SymbolAtom( "typeerror" );
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

    if( nl->IsEqual( arg1, "mbool" )
     && nl->IsEqual( arg2, "mbool" ) )
      return nl->SymbolAtom( "mbool" );
    if( nl->IsEqual( arg1, "mbool" )
     && nl->IsEqual( arg2, "bool" ) )
      return nl->SymbolAtom( "mbool" );
    if( nl->IsEqual( arg1, "bool" )
     && nl->IsEqual( arg2, "mbool" ) )
      return nl->SymbolAtom( "mbool" );
    if( nl->IsEqual( arg1, "mint" )
     && nl->IsEqual( arg2, "mint" ) )
      return nl->SymbolAtom( "mint" );
    if( nl->IsEqual( arg1, "mint" )
     && nl->IsEqual( arg2, "int" ) )
      return nl->SymbolAtom( "mint" );
    if( nl->IsEqual( arg1, "int" )
     && nl->IsEqual( arg2, "mint" ) )
      return nl->SymbolAtom( "mint" );
    if( nl->IsEqual( arg1, "mreal" )
     && nl->IsEqual( arg2, "mreal" ) )
      return nl->SymbolAtom( "mreal" );
    if( nl->IsEqual( arg1, "mreal" )
    && nl->IsEqual( arg2, "real" ) )
      return nl->SymbolAtom( "mreal" );
    if( nl->IsEqual( arg1, "real" )
     && nl->IsEqual( arg2, "mreal" ) )
      return nl->SymbolAtom( "mreal" );
    if( nl->IsEqual( arg1, "mpoint" )
     && nl->IsEqual( arg2, "mpoint" ) )
      return nl->SymbolAtom( "mpoint" );
    if( nl->IsEqual( arg1, "mpoint" )
     && nl->IsEqual( arg2, "point" ) )
      return nl->SymbolAtom( "mpoint" );
    if( nl->IsEqual( arg1, "point" )
     && nl->IsEqual( arg2, "mpoint" ) )
      return nl->SymbolAtom( "mpoint" );
    if(nl->IsEqual(nl->First(args), "region")
     && nl->IsEqual(nl->Second(args), "mpoint"))
      return nl->SymbolAtom("movingregion");
    if(nl->IsEqual(nl->First(args), "movingregion")
     && nl->IsEqual(nl->Second(args), "point"))
      return nl->SymbolAtom("movingregion");
    if(nl->IsEqual(nl->First(args), "movingregion")
     && nl->IsEqual(nl->Second(args), "mpoint"))
      return nl->SymbolAtom("movingregion");
    if(nl->IsEqual(nl->First(args), "movingregion")
     && nl->IsEqual(nl->Second(args), "points"))
      return nl->SymbolAtom("movingregion");
    if(nl->IsEqual(nl->First(args), "movingregion")
     && nl->IsEqual(nl->Second(args), "line"))
      return nl->SymbolAtom("movingregion");
    if( nl->IsEqual( arg1, "mstring" )
     && nl->IsEqual( arg2, "mstring" ) )
      return nl->SymbolAtom( "mstring" );
    if( nl->IsEqual( arg1, "mstring" )
     && nl->IsEqual( arg2, "string" ) )
      return nl->SymbolAtom( "mstring" );
    if( nl->IsEqual( arg1, "string" )
     && nl->IsEqual( arg2, "mstring" ) )
      return nl->SymbolAtom( "mstring" );
  }
  return nl->SymbolAtom( "typeerror" );
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

      if ((nl->IsEqual( arg1, "mpoint" )
       && nl->IsEqual( arg2, "points" )))
        return nl->SymbolAtom( "mbool" );
      if ((nl->IsEqual( arg1, "mpoint" )
       && nl->IsEqual( arg2, "line" )))
        return nl->SymbolAtom( "mbool" );

      if(nl->IsEqual( arg1, "movingregion")
       && nl->IsEqual( arg2, "points"))
         return nl->SymbolAtom("mbool");
      if(nl->IsEqual( arg1, "movingregion")
       && nl->IsEqual( arg2, "line"))
         return nl->SymbolAtom("mbool");
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1 Type mapping function ~PerimeterTypeMap~

Used by ~perimeter~ and ~area~

*/
static ListExpr PerimeterTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 1
     && nl->IsEqual(nl->First(args), "movingregion"))
        return nl->SymbolAtom("mreal");
    else
        return nl->SymbolAtom("typeerror");
}

/*
16.1 Type mapping function ~RCenterTypeMap~

Used by ~rough\_center~

*/
static ListExpr RCenterTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 1
     && nl->IsEqual(nl->First(args), "movingregion"))
        return nl->SymbolAtom("mpoint");
    else
        return nl->SymbolAtom("typeerror");
}

/*
16.1 Type mapping function ~NComponentsTypeMap~

Used by ~no\_components~

*/
static ListExpr NComponentsTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 1
     && nl->IsEqual(nl->First(args), "movingregion"))
        return nl->SymbolAtom("mint");
    else
        return nl->SymbolAtom("typeerror");
}

/*
16.1 Type mapping function ~UnionTypeMap~

Used by ~union~:

*/
static ListExpr UnionTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 2){
      if(nl->IsEqual(nl->First(args), "mpoint")
         && nl->IsEqual(nl->Second(args), "region"))
         return nl->SymbolAtom("movingregion");
      if(nl->IsEqual(nl->First(args), "mpoint")
         && nl->IsEqual(nl->Second(args), "movingregion"))
         return nl->SymbolAtom("movingregion");
      if(nl->IsEqual(nl->First(args), "point")
         && nl->IsEqual(nl->Second(args), "movingregion"))
         return nl->SymbolAtom("movingregion");

      else
        return nl->SymbolAtom("typeerror");
    }
    else
        return nl->SymbolAtom("typeerror");
}

/*
16.1 Type mapping function ~TemporalLiftIsemptyTypeMap~

Used by ~isempty~:

*/
static ListExpr TemporalLiftIsemptyTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 1){
      if(nl->IsEqual(nl->First(args), "movingregion"))
        return nl->SymbolAtom("mbool");
     if(nl->IsEqual(nl->First(args), "mbool"))
        return nl->SymbolAtom("mbool");
     if(nl->IsEqual(nl->First(args), "mint"))
        return nl->SymbolAtom("mbool");
     if(nl->IsEqual(nl->First(args), "mreal"))
        return nl->SymbolAtom("mbool");
     if(nl->IsEqual(nl->First(args), "mpoint"))
        return nl->SymbolAtom("mbool");
     if(nl->IsEqual(nl->First(args), "mstring"))
        return nl->SymbolAtom("mbool");

     else
        return nl->SymbolAtom("typeerror");
    }
    else
        return nl->SymbolAtom("typeerror");
}

/*
16.1 Type mapping function ~TemporalMIntTypeMap~

Used by ~mint~:

*/
static ListExpr TemporalMIntTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 1
        && nl->IsEqual(nl->First(args), "periods"))
        return nl->SymbolAtom("mint");
    else
        return nl->SymbolAtom("typeerror");
}

/*
16.1 Type mapping function ~TemporalPlusTypeMap~

Used by ~+~:

*/
static ListExpr TemporalPlusTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "mint")
        && nl->IsEqual(nl->Second(args), "mint"))
        return nl->SymbolAtom("mint");
    else
        return nl->SymbolAtom("typeerror");
}

/*
16.1 Type mapping function ~TemporalZeroTypeMap~

Used by ~zero~:

*/
static ListExpr TemporalZeroTypeMap(ListExpr args) {;

    if (nl->ListLength(args) == 0)
        return nl->SymbolAtom("mint");
    else
        return nl->SymbolAtom("typeerror");
}

/*
16.1 Type mapping function ~TemporalConcatTypeMap~

Used by ~concat~:

*/
static ListExpr TemporalConcatTypeMap(ListExpr args) {

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "mpoint")
        && nl->IsEqual(nl->Second(args), "mpoint") )
        return nl->SymbolAtom("mpoint");
    else
        return nl->SymbolAtom("typeerror");
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

    if( nl->IsEqual( arg1, "mreal" ))
      return nl->SymbolAtom( "mreal" );
  }
  return nl->SymbolAtom( "typeerror" );
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

  if( nl->SymbolValue( arg1 ) == "mbool"
   && nl->SymbolValue( arg2 ) == "mbool" )
    return 0;
  if( nl->SymbolValue( arg1 ) == "mbool"
   && nl->SymbolValue( arg2 ) == "bool" )
    return 1;
  if( nl->SymbolValue( arg1 ) == "bool"
   && nl->SymbolValue( arg2 ) == "mbool" )
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

  if( nl->SymbolValue( arg1 ) == "mbool"
   && nl->SymbolValue( arg2 ) == "mbool" )
    return 0;
  if( nl->SymbolValue( arg1 ) == "mbool"
   && nl->SymbolValue( arg2 ) == "bool" )
    return 1;
  if( nl->SymbolValue( arg1 ) == "bool"
   && nl->SymbolValue( arg2 ) == "mbool" )
    return 2;
  if( nl->SymbolValue( arg1 ) == "mint"
   && nl->SymbolValue( arg2 ) == "mint" )
    return 3;
  if( nl->SymbolValue( arg1 ) == "mint"
   && nl->SymbolValue( arg2 ) == "int" )
    return 4;
  if( nl->SymbolValue( arg1 ) == "int"
   && nl->SymbolValue( arg2 ) == "mint" )
    return 5;
  if( nl->SymbolValue( arg1 ) == "mstring"
   && nl->SymbolValue( arg2 ) == "mstring" )
    return 6;
  if( nl->SymbolValue( arg1 ) == "mstring"
   && nl->SymbolValue( arg2 ) == "string" )
    return 7;
  if( nl->SymbolValue( arg1 ) == "string"
   && nl->SymbolValue( arg2 ) == "mstring" )
    return 8;
  if( nl->SymbolValue( arg1 ) == "mreal"
   && nl->SymbolValue( arg2 ) == "mreal" )
    return 9;
  if( nl->SymbolValue( arg1 ) == "mreal"
   && nl->SymbolValue( arg2 ) == "real" )
    return 10;
  if( nl->SymbolValue( arg1 ) == "real"
   && nl->SymbolValue( arg2 ) == "mreal" )
    return 11;
  if( nl->SymbolValue( arg1 ) == "mpoint"
   && nl->SymbolValue( arg2 ) == "mpoint" )
    return 12;
  if( nl->SymbolValue( arg1 ) == "mpoint"
   && nl->SymbolValue( arg2 ) == "point" )
    return 13;
  if( nl->SymbolValue( arg1 ) == "point"
   && nl->SymbolValue( arg2 ) == "mpoint" )
    return 14;
  if( nl->SymbolValue( arg1 ) == "movingregion"
   && nl->SymbolValue( arg2 ) == "region" )
    return 15;
  if( nl->SymbolValue( arg1 ) == "region"
   && nl->SymbolValue( arg2 ) == "movingregion" )
    return 16;
  if( nl->SymbolValue( arg1 ) == "movingregion"
   && nl->SymbolValue( arg2 ) == "movingregion" )
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
  if( nl->SymbolValue( arg1 ) == "mbool"
   && nl->SymbolValue( arg2 ) == "mbool" )
    return 0;
  if( nl->SymbolValue( arg1 ) == "mbool"
   && nl->SymbolValue( arg2 ) == "bool" )
    return 1;
  if( nl->SymbolValue( arg1 ) == "bool"
   && nl->SymbolValue( arg2 ) == "mbool" )
    return 2;
  if( nl->SymbolValue( arg1 ) == "mint"
   && nl->SymbolValue( arg2 ) == "mint" )
    return 3;
  if( nl->SymbolValue( arg1 ) == "mint"
   && nl->SymbolValue( arg2 ) == "int" )
    return 4;
  if( nl->SymbolValue( arg1 ) == "int"
   && nl->SymbolValue( arg2 ) == "mint" )
    return 5;
  if( nl->SymbolValue( arg1 ) == "mreal"
   && nl->SymbolValue( arg2 ) == "mreal" )
    return 6;
  if( nl->SymbolValue( arg1 ) == "mreal"
   && nl->SymbolValue( arg2 ) == "real" )
    return 7;
  if( nl->SymbolValue( arg1 ) == "real"
   && nl->SymbolValue( arg2 ) == "mreal" )
    return 8;
  if( nl->SymbolValue( arg1 ) == "mstring"
   && nl->SymbolValue( arg2 ) == "mstring" )
    return 9;
  if( nl->SymbolValue( arg1 ) == "mstring"
   && nl->SymbolValue( arg2 ) == "string" )
    return 10;
  if( nl->SymbolValue( arg1 ) == "string"
   && nl->SymbolValue( arg2 ) == "mstring" )
    return 11;

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
  if( nl->SymbolValue( arg1 ) == "mpoint"
  && nl->SymbolValue( arg2 ) == "mpoint" )
    return 0;
  if( nl->SymbolValue( arg1 ) == "mreal"
   && nl->SymbolValue( arg2 ) == "mreal" )
    return 1;
  if( nl->SymbolValue( arg1 ) == "mreal"
   && nl->SymbolValue( arg2 ) == "real" )
    return 2;
  if( nl->SymbolValue( arg1 ) == "real"
   && nl->SymbolValue( arg2 ) == "mreal" )
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

  if( nl->SymbolValue( arg1 ) == "mbool"
   && nl->SymbolValue( arg2 ) == "mbool" )
    return 0;
  if( nl->SymbolValue( arg1 ) == "mbool"
   && nl->SymbolValue( arg2 ) == "bool" )
    return 1;
  if( nl->SymbolValue( arg1 ) == "bool"
   && nl->SymbolValue( arg2 ) == "mbool" )
    return 2;
  if( nl->SymbolValue( arg1 ) == "mint"
   && nl->SymbolValue( arg2 ) == "mint" )
    return 3;
  if( nl->SymbolValue( arg1 ) == "mint"
   && nl->SymbolValue( arg2 ) == "int" )
    return 4;
  if( nl->SymbolValue( arg1 ) == "int"
   && nl->SymbolValue( arg2 ) == "mint" )
    return 5;
  if( nl->SymbolValue( arg1 ) == "mreal"
   && nl->SymbolValue( arg2 ) == "mreal" )
    return 6;
  if( nl->SymbolValue( arg1 ) == "mreal"
   && nl->SymbolValue( arg2 ) == "real" )
    return 7;
  if( nl->SymbolValue( arg1 ) == "real"
   && nl->SymbolValue( arg2 ) == "mreal" )
    return 8;
  if( nl->SymbolValue( arg1 ) == "mpoint"
   && nl->SymbolValue( arg2 ) == "points" )
    return 9;
  if( nl->SymbolValue( arg1 ) == "mpoint"
   && nl->SymbolValue( arg2 ) == "line" )
    return 10;
  if( nl->SymbolValue( arg1 ) == "points"
   && nl->SymbolValue( arg2 ) == "mpoint" )
    return 11;
  if( nl->SymbolValue( arg1 ) == "line"
   && nl->SymbolValue( arg2 ) == "mpoint" )
    return 12;
  if( nl->SymbolValue( arg1 ) == "mstring"
   && nl->SymbolValue( arg2 ) == "mstring" )
    return 13;
  if( nl->SymbolValue( arg1 ) == "mstring"
   && nl->SymbolValue( arg2 ) == "string" )
    return 14;
  if( nl->SymbolValue( arg1 ) == "string"
   && nl->SymbolValue( arg2 ) == "mstring" )
    return 15;
  if( nl->SymbolValue( arg1 ) == "mpoint"
   && nl->SymbolValue( arg2 ) == "mpoint" )
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

  if( nl->SymbolValue( arg1 ) == "mbool"
   && nl->SymbolValue( arg2 ) == "mbool" )
    return 0;
  if( nl->SymbolValue( arg1 ) == "mbool"
   && nl->SymbolValue( arg2 ) == "bool" )
    return 1;
  if( nl->SymbolValue( arg1 ) == "bool"
   && nl->SymbolValue( arg2 ) == "mbool" )
    return 2;
  if( nl->SymbolValue( arg1 ) == "mint"
   && nl->SymbolValue( arg2 ) == "mint" )
    return 3;
  if( nl->SymbolValue( arg1 ) == "mint"
   && nl->SymbolValue( arg2 ) == "int" )
    return 4;
  if( nl->SymbolValue( arg1 ) == "int"
   && nl->SymbolValue( arg2 ) == "mint" )
    return 5;
  if( nl->SymbolValue( arg1 ) == "mreal"
   && nl->SymbolValue( arg2 ) == "mreal" )
    return 6;
  if( nl->SymbolValue( arg1 ) == "mreal"
   && nl->SymbolValue( arg2 ) == "real" )
    return 7;
  if( nl->SymbolValue( arg1 ) == "real"
   && nl->SymbolValue( arg2 ) == "mreal" )
    return 8;
  if( nl->SymbolValue( arg1 ) == "mpoint"
   && nl->SymbolValue( arg2 ) == "mpoint" )
    return 9;
  if( nl->SymbolValue( arg1 ) == "mpoint"
   && nl->SymbolValue( arg2 ) == "point" )
    return 10;
  if( nl->SymbolValue( arg1 ) == "point"
   && nl->SymbolValue( arg2 ) == "mpoint" )
    return 11;
  if(nl->SymbolValue(nl->First(args)) == "region"
   && nl->SymbolValue(nl->Second(args)) == "mpoint")
    return 12;
  if  (nl->SymbolValue(nl->First(args)) == "movingregion"
   && nl->SymbolValue(nl->Second(args)) == "point")
    return 13;
  if  (nl->SymbolValue(nl->First(args)) == "movingregion"
   && nl->SymbolValue(nl->Second(args)) == "mpoint")
    return 14;
  if  (nl->SymbolValue(nl->First(args)) == "movingregion"
   && nl->SymbolValue(nl->Second(args)) == "points")
    return 15;
  if  (nl->SymbolValue(nl->First(args)) == "movingregion"
   && nl->SymbolValue(nl->Second(args)) == "line")
    return 16;
  if( nl->SymbolValue( arg1 ) == "mstring"
   && nl->SymbolValue( arg2 ) == "mstring" )
    return 17;
  if( nl->SymbolValue( arg1 ) == "mstring"
   && nl->SymbolValue( arg2 ) == "string" )
    return 18;
  if( nl->SymbolValue( arg1 ) == "string"
   && nl->SymbolValue( arg2 ) == "mstring" )
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

    if( nl->SymbolValue( arg1 ) == "mpoint"
     && nl->SymbolValue( arg2 ) == "points" )
      return 0;
    if( nl->SymbolValue( arg1 ) == "mpoint"
     && nl->SymbolValue( arg2 ) == "line" )
      return 1;
    if  (nl->SymbolValue(nl->First(args)) == "movingregion"
     && nl->SymbolValue(nl->Second(args)) == "points")
      return 2;
    if  (nl->SymbolValue(nl->First(args)) == "movingregion"
     && nl->SymbolValue(nl->Second(args)) == "line")
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
      if(nl->SymbolValue(nl->First(args)) == "mpoint"
       && nl->SymbolValue(nl->Second(args)) == "region")
        return 0;
      else if  (nl->SymbolValue(nl->First(args)) == "mpoint"
       && nl->SymbolValue(nl->Second(args)) == "movingregion")
        return 1;
      else if  (nl->SymbolValue(nl->First(args)) == "point"
       && nl->SymbolValue(nl->Second(args)) == "movingregion")
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
      if(nl->SymbolValue(nl->First(args)) == "movingregion")
        return 0;
      else if  (nl->SymbolValue(nl->First(args)) == "mbool")
        return 1;
      else if  (nl->SymbolValue(nl->First(args)) == "mint")
        return 2;
      else if  (nl->SymbolValue(nl->First(args)) == "mreal")
        return 3;
      else if  (nl->SymbolValue(nl->First(args)) == "mpoint")
        return 4;
      else if  (nl->SymbolValue(nl->First(args)) == "mstring")
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

  MovingRealCompareMM(*((MReal*)args[0].addr),
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
   *((CcReal*)args[1].addr), *((MBool*)result.addr), op);

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
  int newop = op;
  if (op > -3 && op < 3) newop = -op;

  MovingRealCompareMS(*((MReal*)args[1].addr),
   *((CcReal*)args[0].addr), *((MBool*)result.addr), newop);

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
    UBool uBool(true);
    const Unit1 *ureg;

    pResult->Clear();
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
        if(!ureg->IsValid())
        continue;

        uBool.timeInterval.rc = !ureg->timeInterval.lc;
        uBool.timeInterval.end = ureg->timeInterval.start;
        uBool.constValue.Set(true,true);

        if(uBool.timeInterval.start < uBool.timeInterval.end
         || (uBool.timeInterval.start == uBool.timeInterval.end
         && uBool.timeInterval.lc && uBool.timeInterval.rc))
          pResult->MergeAdd(uBool);
        uBool.timeInterval = ureg->timeInterval;
        uBool.constValue.Set(true,false);

        pResult->MergeAdd(uBool);
        uBool.timeInterval.lc = !ureg->timeInterval.rc;
        uBool.timeInterval.start = ureg->timeInterval.end;
      }
      uBool.timeInterval.end.ToMaximum();
      uBool.timeInterval.end.SetType(instanttype);
      if(ureg->timeInterval.end < uBool.timeInterval.end){
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
  pResult->Clear();

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

  delete pResult;

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
  pResult->Clear();

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

  delete pResult;

  return 0;
}

/*
1.1 ValueMapping ~MFalseValueMap~ is used in ~inside~ with mregion x points
/ line

Creats false for every periode in mregion.

*/
int MFalseValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {
    if(TLA_DEBUG)
      cout<< "MFalseValueMap() called" << endl;

    result = qp->ResultStorage(s);
    MBool* pResult = (MBool*)result.addr;
    MRegion* reg = (MRegion*)args[0].addr;
    UBool uBool(true);

    pResult->Clear();
    pResult->StartBulkLoad();
    for( int i = 0; i < reg->GetNoComponents(); i++) {
      const URegionEmb *ureg;
      reg->Get(i, ureg);
      if(!ureg->IsValid())
        continue;
      uBool.timeInterval = ureg->timeInterval;
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
  pResult->Clear();

  MPointInsidePoints( *((MPoint*)args[1].addr),
    *((Points*)args[0].addr), *pResult);

  //create a MPoint (intersection)
  MPoint* endResult = (MPoint*)result.addr;
  CompletePeriods2MPoint(((MPoint*)args[1].addr), pResult, endResult);

  delete pResult;

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
  pResult->Clear();

  MPointInsideLine( *((MPoint*)args[1].addr),
    *((Line*)args[0].addr), *pResult);

  //create a MPoint (intersection)
  MPoint* endResult = (MPoint*)result.addr;
  CompletePeriods2MPoint(((MPoint*)args[1].addr), pResult, endResult);

  delete pResult;

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
16.3 Value mapping functions of operators ~intsersection~ and
~minus~ for mbool/bool, mint/int and mstring/string

*/
template<class Mapping1, class Unit1, class Operator2, int op>
int TemporalMSIntersection( Word* args, Word& result, int message,
Word& local, Supplier s )
{
  if(TLA_DEBUG)
    cout<<"TemporalMSIntersection called"<<endl;
  result = qp->ResultStorage( s );
  Mapping1 *mop1;
  Mapping1 *mop2 = new Mapping1(0);
  const Unit1 *up1;
  mop1 = (Mapping1*)args[0].addr;
  Operator2 *constop = (Operator2*)args[1].addr;

  mop2->Clear();
  mop2->StartBulkLoad();
  if(constop->IsDefined())
  {
    for (int i = 0; i < mop1->GetNoComponents(); i++) {
      mop1->Get(i, up1);
      if(!up1->IsDefined())
          continue;
      Unit1 *up2 = new Unit1(up1->timeInterval, *constop);
      mop2->Add(*up2);
      delete up2;
    }
  }
  mop2->EndBulkLoad(false);
  MovingIntersectionMM<Mapping1, Mapping1, Unit1, Unit1>
  ( *mop1, *mop2, *((Mapping1*)result.addr), op);
  delete mop2;

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
  MReal *mop1;
  MReal *mop2= new MReal(0);
  const UReal *up1;
  mop1 = (MReal*)args[0].addr;
  CcReal *constop = (CcReal*)args[1].addr;

  mop2->Clear();
  mop2->StartBulkLoad();
  if(constop->IsDefined())
  {
    for (int i = 0; i < mop1->GetNoComponents(); i++) {
      mop1->Get(i, up1);
      if(!up1->IsDefined())
          continue;
      UReal *up2 = new UReal(up1->timeInterval, 0.0, 0.0, (up1->r) ?
         pow(constop->GetRealval(), 2) : constop->GetRealval(),up1->r);

      mop2->Add(*up2);
      delete up2;
    }
  }
  mop2->EndBulkLoad(false);
  MovingRealIntersectionMM( *mop1, *mop2, *((MReal*)result.addr), op);

  delete mop2;

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
  result = (qp->ResultStorage( s ));
  MPoint *op1 = (MPoint*) args[0].addr;
  MPoint *op2 = (MPoint*) args[1].addr;
  MPoint *res = (MPoint*) result.addr;
  UPoint resunit( false );

  if(TLA_DEBUG)
    cout<<"TemporalMPointMPointIntersection called"<<endl;

  MPoint un(1);  //part of the Result

  RefinementPartition<MPoint, MPoint, UPoint, UPoint> rp(*op1, *op2);

  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  res->Clear();
  res->StartBulkLoad();

  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant>* iv;
    int u1Pos;
    int u2Pos;

    UPoint u1( true );
    UPoint u2( true );

    const UPoint *u1transfer;
    const UPoint *u2transfer;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (TLA_DEBUG)
      cout << "rp:" << i << ": (" << iv->start.ToString()<< " "
           << iv->end.ToString() << " " << iv->lc << " " << iv->rc << ") "
           << u1Pos << " " << u2Pos << endl;

    if (u1Pos == -1 || u2Pos == -1 )
      continue;

    else
    {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<" ["
        << iv->start.ToString()<< " "<< iv->end.ToString()<< " "<< iv->lc
        << " "<< iv->rc<< "] "<< u1Pos<< " "<< u2Pos<< endl;

      op1->Get(u1Pos, u1transfer);
      op2->Get(u2Pos, u2transfer);

      if (TLA_DEBUG)
      {
        cout << "Actual partition #" << i << ":" << endl << " u1=";
        u1transfer->Print(cout);
        cout << endl << " u2="; u2transfer->Print(cout); cout << endl;
      }
      if( !u1transfer->IsDefined() || !u2transfer->IsDefined() )
        continue;
      u1transfer->AtInterval(*iv, u1);
      u2transfer->AtInterval(*iv, u2);

      // create intersection of  u1 x u2
      u1.Intersection(u2, resunit);
      if ( resunit.IsDefined() )
        res->MergeAdd(resunit);
    }
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
  Mapping1 *mop1 = new Mapping1(0);
  Mapping1 *mop2;
  const Unit1 *up2;
  mop2 = (Mapping1*)args[1].addr;
  Operator1 *constop = (Operator1*)args[0].addr;

  mop1->Clear();
  mop1->StartBulkLoad();
  if(constop->IsDefined())
  {
    for (int i = 0; i < mop2->GetNoComponents(); i++) {
      mop2->Get(i, up2);
     if(!up2->IsDefined())
          continue;
      Unit1 *up1 = new Unit1(up2->timeInterval, *constop);

      mop1->Add(*up1);
      delete up1;
    }
  }
  mop1->EndBulkLoad(false);
  MovingIntersectionMM<Mapping1, Mapping1, Unit1, Unit1>
  ( *mop1, *mop2, *((Mapping1*)result.addr), op);

  delete mop1;

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
  MReal *mop1 = new MReal(0);
  MReal *mop2;
  const UReal *up2;
  mop2 = (MReal*)args[1].addr;
  CcReal *constop = (CcReal*)args[0].addr;

  mop1->Clear();
  mop1->StartBulkLoad();
  if(constop->IsDefined())
  {
    for (int i = 0; i < mop2->GetNoComponents(); i++) {
      mop2->Get(i, up2);
      if(!up2->IsDefined())
          continue;

      UReal *up1 = new UReal(up2->timeInterval, 0.0, 0.0, (up2->r) ?
        pow(constop->GetRealval(), 2) : constop->GetRealval(), up2->r);

      if(TLA_DEBUG){
        cout<<"up1["<<i<<"] ["<<up1->timeInterval.start.ToString()
        <<" "<<up1->timeInterval.end.ToString()<<" "
        <<up1->timeInterval.lc<<" "<<up1->timeInterval.rc<<"] "<<endl;}
      mop1->Add(*up1);
      delete up1;
    }
  }
  mop1->EndBulkLoad(false);
  MovingRealIntersectionMM( *mop1, *mop2, *((MReal*)result.addr),
   op);

  delete mop1;

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

  delete mBool;

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

  delete mBool;

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

  delete mBool;

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
  MReal *mop1;
  MReal *mop2 = new MReal(0);
  const UReal *up1;
  mop1 = (MReal*)args[0].addr;
  CcReal *constop = (CcReal*)args[1].addr;

  mop2->Clear();
  mop2->StartBulkLoad();
  if(constop->IsDefined())
  {
    for (int i = 0; i < mop1->GetNoComponents(); i++) {
      mop1->Get(i, up1);
      if(!up1->IsDefined())
          continue;
      UReal *up2 = new UReal(up1->timeInterval, 0.0, 0.0,
                       (up1->r) ? pow(constop->GetRealval(), 2)
                       : constop->GetRealval(),up1->r);
      mop2->Add(*up2);
      delete up2;
    }
  }
  mop2->EndBulkLoad(false);
  MRealDistanceMM( *mop1, *mop2, *((MReal*)result.addr));

  delete mop2;

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
  MReal *mop1;
  MReal *mop2 = new MReal(0);
  const UReal *up1;
  mop1 = (MReal*)args[1].addr;
  CcReal *constop = (CcReal*)args[0].addr;

  mop2->Clear();
  mop2->StartBulkLoad();
  if(constop->IsDefined())
  {
    for (int i = 0; i < mop1->GetNoComponents(); i++) {
      mop1->Get(i, up1);
      if(!up1->IsDefined())
          continue;
      UReal *up2 = new UReal(up1->timeInterval, 0.0, 0.0,
                       (up1->r) ? pow(constop->GetRealval(), 2)
                       : constop->GetRealval(),up1->r);
      mop2->Add(*up2);
      delete up2;
    }
  }
  mop2->EndBulkLoad(false);
  MRealDistanceMM( *mop1, *mop2, *((MReal*)result.addr));

  delete mop2;

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
  result = qp->ResultStorage( s );
  MBool* pResult = (MBool*)result.addr;
  MBool* op = (MBool*)args[0].addr;
  const UBool *u1transfer;
  UBool uBool(true);

  pResult->Clear();
  pResult->StartBulkLoad();
  for( int i = 0; i < op->GetNoComponents(); i++)
  {
    op->Get(i, u1transfer);
    if(!u1transfer->IsDefined())
        continue;
    uBool = *u1transfer;
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

Every periode in periods will be transformed into an UInt with
value 1 the holes will be transformed into UInts with value 0.

*/
int TemporalMIntValueMap( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
    if(TLA_DEBUG)
      cout<<"TemporalMIntValueMap called"<<endl;
    result = qp->ResultStorage( s );
    Periods *pers = (Periods*)args[0].addr;
    MInt *pResult = (MInt*)result.addr;
    const Interval<Instant> *per;
    UInt uInt(true);

    pResult->Clear();
    pResult->StartBulkLoad();
    if(pers->GetNoComponents() < 1){
      uInt.timeInterval.lc = true;
      uInt.timeInterval.start.ToMinimum();
      uInt.timeInterval.start.SetType(instanttype);
      uInt.timeInterval.rc = true;
      uInt.timeInterval.end.ToMaximum();
      uInt.timeInterval.end.SetType(instanttype);
      uInt.constValue.Set(true, 0);
      pResult->Add(uInt);
    }
    else{
      uInt.timeInterval.lc = true;
      uInt.timeInterval.start.ToMinimum();
      uInt.timeInterval.start.SetType(instanttype);
      for( int i = 0; i < pers->GetNoComponents(); i++) {
        pers->Get(i, per);
        if(!pers->IsDefined())
          continue;

        uInt.timeInterval.rc = !per->lc;
        uInt.timeInterval.end = per->start;
        uInt.constValue.Set(true, 0);

        if(uInt.timeInterval.start < uInt.timeInterval.end
         || (uInt.timeInterval.start == uInt.timeInterval.end
         && uInt.timeInterval.lc && uInt.timeInterval.rc))
          pResult->MergeAdd(uInt);
        uInt.timeInterval = *per;
        uInt.constValue.Set(true, 1);

        pResult->MergeAdd(uInt);

        uInt.timeInterval.lc = !per->rc;
        uInt.timeInterval.start = per->end;
      }
      uInt.timeInterval.end.ToMaximum();
      uInt.timeInterval.end.SetType(instanttype);
      if(per->end < uInt.timeInterval.end){
        uInt.timeInterval.rc = true;
        uInt.constValue.Set(true, 0);

        pResult->MergeAdd(uInt);;
      }
    }
    pResult->EndBulkLoad(false);

  return 0;
}

/*
16.3 Value mapping functions of operator ~+~ with two mint-objects

*/
int TemporalPlusValueMap( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  if(TLA_DEBUG)
    cout<<"TemporalPlusValueMap called"<<endl;

  result = qp->ResultStorage( s );
  MInt *op1 = (MInt*)args[0].addr;
  MInt *op2 = (MInt*)args[1].addr;
  MInt *pResult = (MInt*)result.addr;

  UInt uInt(true);  //part of the Result

  RefinementPartition<MInt, MInt, UInt, UInt> rp(*op1, *op2);
  if(TLA_DEBUG)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;
  pResult->Clear();
  pResult->StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant>* iv;
    int u1Pos;
    int u2Pos;
    const UInt *u1;
    const UInt *u2;

    rp.Get(i, iv, u1Pos, u2Pos);

    if (u1Pos == -1 || u2Pos == -1)
      continue;
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<endl;
      op1->Get(u1Pos, u1);
      op2->Get(u2Pos, u2);
      if(!(u1->IsDefined() && u2->IsDefined()))
        continue;
    }
    uInt.timeInterval = *iv;

    uInt.constValue.Set(true,
      u1->constValue.GetIntval() + u2->constValue.GetIntval());

    pResult->MergeAdd(uInt);
  }
  pResult->EndBulkLoad(false);

  return 0;
}

/*
1.1 Value mapping functions of operator ~concat~

Concats two mpoints. If intervals are not disjunct it creates an empty value.

*/
int TemporalConcatValueMap(Word* args, Word& result, int message,
 Word& local, Supplier s) {
    if(TLA_DEBUG)
      cout<< "TemporalConcatValueMap() called" << endl;

    result = qp->ResultStorage(s);
    MPoint* pResult = (MPoint*)result.addr;
    MPoint* p1 = (MPoint*)args[0].addr;
    MPoint* p2 = (MPoint*)args[1].addr;
    const UPoint *up1;
    const UPoint *up2;

    pResult->Clear();
    pResult->StartBulkLoad();
    if(p1->GetNoComponents() > 0 && p2->GetNoComponents() > 0){
      p1->Get(p1->GetNoComponents() - 1, up1);
      p2->Get(0, up2);
      if(!(up1->timeInterval.end < up2->timeInterval.start
        || (up1->timeInterval.end == up2->timeInterval.start
        && !(up1->timeInterval.rc && up2->timeInterval.lc)))){
        if(TLA_DEBUG)
         cout<<"DefTime of mpoints are not disjunct! Last interval of first "
         <<"mpoint ends after first interval of of second mpoint begins."<<endl;
        pResult->EndBulkLoad(false);
        return 0;
      }
    }
    for( int i = 0; i < p1->GetNoComponents(); i++) {
      p1->Get(i, up1);
      if(!up1->IsDefined())
        continue;
      pResult->Add(*up1);
    }
    for( int i = 0; i < p2->GetNoComponents(); i++) {
      p2->Get(i, up2);
      if(!up2->IsDefined())
        continue;
      pResult->Add(*up2);
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
             "( <text>movingregion -> mpoint</text--->"
             "<text>rough_center ( _ )</text--->"
             "<text>Calculates an approach to the"
             "center of gravity of a moving Region.</text--->"
             "<text>rough_center(mrg1)</text---> ) )";

const string ncomponentsspec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>movingregion -> mint</text--->"
             "<text>no_components ( _ )</text--->"
             "<text>Calculates the number of faces of "
             "a moving Region.</text--->"
             "<text>no_components(mrg1)</text---> ) )";

const string perimeterspec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>movingregion -> mreal</text--->"
             "<text>perimeter ( _ )</text--->"
             "<text>Calculates the perimeter of a moving Region.</text--->"
             "<text>mraperimeter(mrg1)</text---> ) )";

const string areaspec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>movingregion -> mreal</text--->"
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

const string temporalmintspec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>periods -> mint</text--->"
             "<text>mmint( _ )</text--->"
             "<text>Creats a MInt from a periods-object with value 1"
             " for each existing period and 0 for every hole.</text--->"
             "<text>mmint(per1)</text---> ) )";

const string temporalplusspec
           = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
             "\"Example\" ) "
             "( <text>mint x mint -> mint</text--->"
             "<text>_ + _</text--->"
             "<text>Adds two mint-obects when both are existant</text--->"
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
                            temporalmintspec,
                            TemporalMIntValueMap,
                            Operator::SimpleSelect,
                            TemporalMIntTypeMap);

static Operator temporalplus("+",
                            temporalplusspec,
                            TemporalPlusValueMap,
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
    AddOperator( &temporalplus);
    AddOperator( &temporalconcat);
    AddOperator( &temporalabs);
    }
    ~TemporalLiftedAlgebra() {}
};

TemporalLiftedAlgebra tempLiftedAlgebra;

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
  return (&tempLiftedAlgebra);
}
