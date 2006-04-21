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

*/


#include <cmath>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include <limits>

extern NestedList* nl;
extern QueryProcessor* qp;


#include "DateTime.h"
using namespace datetime;

#include "TemporalAlgebra.h"
#include "MovingRegionAlgebra.h"
#include "TemporalExtAlgebra.h"

/*
Set ~TLA\_DEBUG~ to ~true~ for debug output. Please note that debug output is
very verbose and has significant negative input on the algebra's performance.
Only enable debug output if you know what you are doing!

*/
const bool TLA_DEBUG = false;

/*
1 Class template ~RefinementPartition~

*/

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
class RefinementPartitionLift {
private:
/*
Private attributes:

  * ~iv~: Array (vector) of sub-intervals, which has been calculated from the
    unit intervals of the ~Mapping~ instances.

  * ~vur~: Maps intervals in ~iv~ to indices of original units in first
    ~Mapping~ instance. A $-1$ values indicates that interval in ~iv~ is no
    sub-interval of any unit interval in first ~Mapping~ instance.

  * ~vup~: Same as ~vur~ for second mapping instance.

*/
    vector< Interval<Instant>* > iv;
    vector<int> vur;
    vector<int> vup;

/*
~AddUnit()~ is a small helper method to create a new interval from
~start~ and ~end~ instant and ~lc~ and ~rc~ flags and to add these to the
~iv~, ~vur~ and ~vup~ vectors.

*/
    void AddUnits(const int urPos,
                  const int upPos,
                  const Instant& start,
                  const Instant& end,
                  const bool lc,
                  const bool rc);

public:
/*
The constructor creates the refinement partition from the two ~Mapping~
instances ~mr~ and ~mp~.

Runtime is $O(\max(n, m))$ with $n$ and $m$ the numbers of units in
~mr~ and ~mp~.

*/
    RefinementPartitionLift(Mapping1& mr, Mapping2& mp);

/*
Since the elements of ~iv~ point to dynamically allocated objects, we need
a destructor.

*/
    ~RefinementPartitionLift();

/*
Return the number of intervals in the refinement partition.

*/
    unsigned int Size(void);

/*
Return the interval and indices in original units of position $pos$ in
the refinement partition in the referenced variables ~civ~, ~ur~ and
~up~. Remember that ~ur~ or ~up~ may be $-1$ if interval is no sub-interval
of unit intervals in the respective ~Mapping~ instance.

Runtime is $O(1)$.

*/
    void Get(unsigned int pos,
             Interval<Instant>*& civ,
             int& ur,
             int& up);
};


template<class Mapping1, class Mapping2, class Unit1, class Unit2>
unsigned int RefinementPartitionLift<Mapping1, Mapping2, Unit1, Unit2>
::Size(void) { 
        if(TLA_DEBUG)
          cout << "RP::Size() called" << endl;
            
        return iv.size(); 
    }

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
void RefinementPartitionLift<Mapping1, Mapping2, Unit1, Unit2>
::Get(unsigned int pos, Interval<Instant>*& civ, int& ur,
     int& up) {
        if(TLA_DEBUG)
          cout << "RP::Get() called" << endl;
    
        assert(pos < iv.size());

        civ = iv[pos];
        ur = vur[pos];
        up = vup[pos];
    }

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
void RefinementPartitionLift<Mapping1, Mapping2, Unit1,
 Unit2>::AddUnits(
    const int urPos,
    const int upPos,
    const Instant& start,
    const Instant& end,
    const bool lc,
    const bool rc) {
    if(TLA_DEBUG){
      cout << "RP::AddUnits() called" << endl;
      cout << "RP::AddUnits() [" << start.ToDouble() << " "
             << end.ToDouble() << " " << lc << " " << rc << "] urPos="
             << urPos << " upPos=" << upPos << endl;
    }

    Interval<Instant>* civ = new Interval<Instant>(start, end,
     lc, rc);

    iv.push_back(civ);
    vur.push_back(urPos);
    vup.push_back(upPos);
}

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
RefinementPartitionLift<Mapping1, Mapping2, Unit1,
 Unit2>::RefinementPartitionLift(
    Mapping1& mr,
    Mapping2& mp) {
    if(TLA_DEBUG)
      cout << "RP::RP() called" << endl;

    int mrUnit = 0;
    int mpUnit = 0;

    const Unit1 *ur;
    const Unit2 *up;

    Instant t, rpstart, rpend, test;
    bool c, rplc, rprc, tc;
    int before = 0;
    int addu = 0;
    int subu = 0;
    
    if(mr.GetNoComponents() > 0 && mp.GetNoComponents() > 0){
       mr.Get(0, ur);
       mp.Get(0, up);
      
       if (ur->timeInterval.start < up->timeInterval.start) {
          test = ur->timeInterval.start;
          tc = !ur->timeInterval.lc;    
       } else {
          test = up->timeInterval.start;
          tc = !up->timeInterval.lc;    
       }
    }
    else if(mr.GetNoComponents() > 0){
       mr.Get(0, ur);
       test = ur->timeInterval.start;
       tc = !ur->timeInterval.lc; 
       t = ur->timeInterval.start;
       c = !ur->timeInterval.lc; 
    }
    else {
       mp.Get(0, up);
       test = up->timeInterval.start;
       tc = !up->timeInterval.lc;
       t = up->timeInterval.start;
       c = !up->timeInterval.lc;
    }
    while (mrUnit < mr.GetNoComponents() 
       && mpUnit < mp.GetNoComponents()) {
       
        t = test; 
        c = tc;  
        
        if(TLA_DEBUG){
          cout << "RP::RP() mrUnit=" << mrUnit << " mpUnit=" 
          << mpUnit 
          << " t=" << t.ToDouble() << endl;
          cout << "RP::RP() mpUnit interval=["
          << up->timeInterval.start.ToDouble()<< " "
          << up->timeInterval.end.ToDouble() << " "
          << up->timeInterval.lc<< " "<< up->timeInterval.rc<< "]"
          << endl;
          cout << "RP::RP() mrUnit interval=["
          << ur->timeInterval.start.ToDouble()<< " "
          << ur->timeInterval.end.ToDouble()<< " "
          <<ur->timeInterval.lc<<" "<<ur->timeInterval.rc<< "]"
          << endl;}
        
        addu = 0;
        subu = 0;
        if (t == up->timeInterval.end 
        && (up->timeInterval.rc == c)) {
          if(TLA_DEBUG)
            cout<<"up ends now"<<endl;
          rpend = t;
          rprc = up->timeInterval.rc;
          subu -= 1;
          if (++mpUnit < mp.GetNoComponents()) {
            mp.Get(mpUnit, up);
          }
        }
        if (t == ur->timeInterval.end 
        && (ur->timeInterval.rc == c)) {
          if(TLA_DEBUG)
            cout<<"ur ends now"<<endl;
          rpend = t;
          rprc = ur->timeInterval.rc;
          subu -= 2;
          if (++mrUnit < mr.GetNoComponents()) {
            mr.Get(mrUnit, ur);
          }
        }
          
        if (t == up->timeInterval.start 
        && (up->timeInterval.lc != c)) {
          if(TLA_DEBUG)
            cout<<"up starts now"<<endl;
          addu += 1;
        }
        if (t == ur->timeInterval.start 
        && (ur->timeInterval.lc != c)) {
          if(TLA_DEBUG)
            cout<<"ur starts now"<<endl;
          addu += 2;
        }
        
        if(TLA_DEBUG){
          cout<<"before "<< before<<" addu "<<addu
          <<"  subu "<<subu<<endl;}
        if (before == 3 && subu == -3)
          AddUnits(mrUnit-1, mpUnit-1, rpstart, rpend,
           rplc, rprc);
        else if (before == 3 && subu == -2)
          AddUnits(mrUnit-1, mpUnit, rpstart, rpend, rplc, rprc);
        else if (before == 3 && subu == -1)
          AddUnits(mrUnit, mpUnit-1, rpstart, rpend, rplc, rprc);
        else if (before == 2 && subu == -2)
          AddUnits(mrUnit-1, -1, rpstart, rpend, rplc, rprc);
        else if (before == 1 && subu == -1)
          AddUnits(-1, mpUnit-1, rpstart, rpend, rplc, rprc);
        if (before == 1 && addu == 2) {
          AddUnits(-1, mpUnit, rpstart, t,
           rplc, !ur->timeInterval.lc);
          rpstart = t;
          rplc = up->timeInterval.lc;
        }
        else if (before == 2 && addu == 1) {
          AddUnits(mrUnit, -1, rpstart, t,
           rplc, !up->timeInterval.lc);
          rpstart = t;
          rplc = up->timeInterval.lc;
        }
        else if (addu == 1) {
          rpstart = t;
          rplc = up->timeInterval.lc;
        }
        else if (addu > 1) {
          rpstart = t;
          rplc = ur->timeInterval.lc;
        }
        before += addu;
        before += subu;
        if(TLA_DEBUG){
          cout<<before<<" next event after "<<t.ToDouble()
          <<" "<<c<<endl;}
        test = t;
        tc = c;
        if(TLA_DEBUG){
          cout<<"1:( "<<(up->timeInterval.start > t)<<" || ( "
          <<(up->timeInterval.start == t)<<" && "
          <<(!c && !up->timeInterval.lc) <<" ))  up "
          <<up->timeInterval.lc<<" c "<<c<<endl;}
        if (up->timeInterval.start > t || 
        ((up->timeInterval.start == t) 
        && ( !c && !up->timeInterval.lc))){ 
          test = up->timeInterval.start;
          tc = !up->timeInterval.lc;
          if(TLA_DEBUG)
            cout<<"1:test "<<test.ToDouble()<<" "<<tc<<endl;
        }
        
        if(TLA_DEBUG){
          cout<<"21:( "<<(ur->timeInterval.start > t)
          <<" || ( "<<(ur->timeInterval.start == t)
          <<" && "<<(!c && !ur->timeInterval.lc) 
          <<" ))  ur "<<ur->timeInterval.lc<<" c "<<c<<endl;}
        if (ur->timeInterval.start > t || 
        ((ur->timeInterval.start == t)
         && ( !c && !ur->timeInterval.lc))){
          if(TLA_DEBUG){
            cout<<"22:( "<<((test == t) && (c == tc))
            <<" || ( "<<(ur->timeInterval.start < test)
            <<" || (" <<(ur->timeInterval.start == test)
            <<" && "<<(tc && ur->timeInterval.lc)<<" )))"<<endl;}
          if (((test == t) && (c == tc))
          || (ur->timeInterval.start < test 
          || ((ur->timeInterval.start == test) 
          && (tc && ur->timeInterval.lc )))){ 
            test = ur->timeInterval.start;
            tc = !ur->timeInterval.lc;
            if(TLA_DEBUG)
              cout<<"2: test "<<test.ToDouble()<<" "<<tc<<endl;
          }
        }

        if(TLA_DEBUG){
          cout<<"31:( "<<(up->timeInterval.end > t)
          <<" || ( "<<(up->timeInterval.end == t)<<" && "
          <<(!c && up->timeInterval.rc )
          <<" ))  up "<<up->timeInterval.rc<<" c "<<c<<endl;}
        if (up->timeInterval.end > t 
        || ((up->timeInterval.end == t)  
        && (!c && up->timeInterval.rc))) {
          if(TLA_DEBUG){
            cout<<"32:( "<<((test == t) && (c == tc))
            <<" || ( "<<(up->timeInterval.end < test)
            <<" || (" <<(up->timeInterval.end == test)
            <<" && "<<(tc && !up->timeInterval.rc)<<" )))"<<endl;}
          if (((test == t) && (c == tc))
          || (up->timeInterval.end < test 
          || ((up->timeInterval.end == test) 
          && (tc && !up->timeInterval.rc )))){ 
           test = up->timeInterval.end;
            tc = up->timeInterval.rc;;
            if(TLA_DEBUG)
              cout<<"3: test "<<test.ToDouble()<<" "<<tc<<endl;
          }
        }
        
        if(TLA_DEBUG){
          cout<<"41:( "<<(ur->timeInterval.end > t)
          <<" || ( "<<(ur->timeInterval.end == t)
          <<" && "<<(!c && ur->timeInterval.rc)
          <<" ))  ur "<<ur->timeInterval.lc<<" c "<<c<<endl;}
        if (ur->timeInterval.end > t 
        || ((ur->timeInterval.end == t)  
        && (!c && ur->timeInterval.rc))) {
          if(TLA_DEBUG){
            cout<<"42:( "<<((test == t) && (c == tc))
            <<" || ( "<<(ur->timeInterval.end < test)
            <<" || (" <<(ur->timeInterval.end == test)<<" && "
            <<(tc && !ur->timeInterval.rc)<<" )))"<<endl;}
          if (((test == t) && (tc == c)) 
          || (ur->timeInterval.end < test 
          || ((ur->timeInterval.end == test) 
          && (tc && !ur->timeInterval.rc)))){ 
            test = ur->timeInterval.end;
            tc = ur->timeInterval.rc;
            if(TLA_DEBUG)
              cout<<"4: test "<<test.ToDouble()<<" "<<tc<<endl;
          }
        }
        
        if(TLA_DEBUG){
          cout<<"new t "<<test.ToDouble()<<" "<<tc<<endl;
          cout << "mpUnit[" << up->timeInterval.start.ToDouble()
          << " " << up->timeInterval.end.ToDouble()
          << " " << up->timeInterval.lc
          << " " << up->timeInterval.rc
          << "]" << endl;
          cout << "mrUnit[" << ur->timeInterval.start.ToDouble()
          << " " << ur->timeInterval.end.ToDouble()
          << " " << ur->timeInterval.lc
          << " " << ur->timeInterval.rc
          << "]" << endl;
          cout<<"next event on "<<t.ToDouble()<<" "<<c<<endl;}
    }//while
    if (mrUnit < mr.GetNoComponents()) {
        if (t < ur->timeInterval.end){
            if(TLA_DEBUG)
              cout<<"Add rest of ur"<<endl;
            AddUnits(
                mrUnit, 
                -1,
                t,
                ur->timeInterval.end,
                c,
                ur->timeInterval.rc);
            }
        mrUnit++;

        while (mrUnit < mr.GetNoComponents()) {
            mr.Get(mrUnit, ur);

            if(TLA_DEBUG)
              cout<<"Add all solo units of r"<<endl;
            AddUnits(
                mrUnit, 
                -1,
                ur->timeInterval.start,
                ur->timeInterval.end,
                ur->timeInterval.lc,
                ur->timeInterval.rc);

            mrUnit++;
        }
    }

    if (mpUnit < mp.GetNoComponents()) {
        if (t < up->timeInterval.end){
            if(TLA_DEBUG)
              cout<<"Add rest of up"<<endl;
            AddUnits(
                -1,
                mpUnit,
                t,
                up->timeInterval.end,
                c,
                up->timeInterval.rc);
            }
        mpUnit++;

        while (mpUnit < mp.GetNoComponents()) {
            mp.Get(mpUnit, up);

            if(TLA_DEBUG)
              cout<<"Add all solo units of p"<<endl;
            AddUnits(
                -1,
                mpUnit, 
                up->timeInterval.start,
                up->timeInterval.end,
                up->timeInterval.lc,
                up->timeInterval.rc);

            mpUnit++;
        }
    }
}

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
RefinementPartitionLift<Mapping1, Mapping2, Unit1,
 Unit2>::~RefinementPartitionLift() {

    if(TLA_DEBUG)
      cout << "RP::~RP() called" << endl;

    for (unsigned int i = 0; i < iv.size(); i++) delete iv[i];
}

/*
1.1.1 Method ~MPerimeter()~

*/

void MPerimeter(MRegion& reg, MReal& res) {
    cout<< "MRegion::MPerimeter() called" << endl;
    
    int nocomponents = reg.GetNoComponents();
    cout<<"GetNoComponents() "<<nocomponents<<endl;
    res.Clear();
    res.StartBulkLoad();
    for(int n = 0; n < nocomponents; n++){
      const URegion *ur;
      UReal ures;
      double start = 0.0, end = 0.0;
      reg.Get(n, ur);
      cout<<"URegion # "<<n<<" "<<"[ "
      <<ur->timeInterval.start.ToDouble()<<" "
      <<ur->timeInterval.end.ToDouble()<<" ]"<<endl;
      ures.timeInterval = ur->timeInterval;
      int number = ur->GetSegmentsNum();
      cout<<"number = "<< number<<endl;
      for(int i = 0; i < number; i++){
        const MSegmentData *dms;
        ur->GetSegment(i, dms);
        /*
        cout<<"GetFaceNo()"<<dms->GetFaceNo()<<endl;
        cout<<"GetCycleNo()"<<dms->GetCycleNo()<<endl;
        cout<<"GetSegmentNo()"<<dms->GetSegmentNo()<<endl;
        cout<<"GetInitialStartX()"<<dms->GetInitialStartX()<<endl;
        cout<<"GetInitialStartY()"<<dms->GetInitialStartY()<<endl;
        cout<<"GetInitialEndX()"<<dms->GetInitialEndX()<<endl;
        cout<<"GetInitialEndY()"<<dms->GetInitialEndY()<<endl;
        cout<<"GetFinalStartX()"<<dms->GetFinalStartX()<<endl;
        cout<<"GetFinalStartY()"<<dms->GetFinalStartY()<<endl;
        cout<<"GetFinalEndX()"<<dms->GetFinalEndX()<<endl;
        cout<<"GetFinalEndY()"<<dms->GetFinalEndY()<<endl;
        cout<<"GetInsideAbove()"<<dms->GetInsideAbove()<<endl;
        */
        if(dms->GetCycleNo() == 0){ //only outercycle
          start += sqrt(pow(dms->GetInitialStartX() 
          - dms->GetInitialEndX(), 2) + pow(dms->GetInitialStartY() 
          - dms->GetInitialEndY(), 2));                
          end += sqrt(pow(dms->GetFinalStartX() 
          - dms->GetFinalEndX(), 2) + pow(dms->GetFinalStartY() 
          - dms->GetFinalEndY(), 2));
        }
      }
      cout<<"URegion # "<<n<<endl;
      cout<<"start "<<start<<endl;
      cout<<"end "<<end<<endl;
      ures.a = 0.0;
      ures.b = (ures.timeInterval.end > ures.timeInterval.start)
      ? (end - start) / (ures.timeInterval.end.ToDouble() 
      - ures.timeInterval.start.ToDouble()) : 0.0;
      ures.c = start;
      ures.r = false;
      cout<<"ures.a "<<ures.a<<endl;
      cout<<"ures.b "<<ures.b<<endl;
      cout<<"ures.c "<<ures.c<<endl;
      cout<<"ures.r "<<ures.r<<endl;
      res.MergeAdd(ures); 
    }
    res.EndBulkLoad(false);
}

/*
1.1.1 Method ~MArea()~

*/

void MArea(MRegion& reg, MReal& res) {
    cout<< "MRegion::MArea() called" << endl;
    
    int nocomponents = reg.GetNoComponents();
    cout<<"GetNoComponents() "<<nocomponents<<endl;
    res.Clear();
    res.StartBulkLoad();
    for(int n = 0; n < nocomponents; n++){
      const URegion *ur;
      UReal ures;
      double at = 0.0, bt = 0.0, ct = 0.0;
      reg.Get(n, ur);
      cout<<"URegion # "<<n<<" "<<"[ "
      <<ur->timeInterval.start.ToDouble()<<" "
      <<ur->timeInterval.end.ToDouble()<<" ]"<<endl;
      double dt = ur->timeInterval.end.ToDouble() 
      - ur->timeInterval.start.ToDouble();
      cout<<"dt "<<dt<<endl;
      if (dt == 0.0) continue;
      ures.timeInterval = ur->timeInterval;
      int number = ur->GetSegmentsNum();
      cout<<"number = "<< number<<endl;
      for(int i = 0; i < number; i++){
        const MSegmentData *dms;
        ur->GetSegment(i, dms);
        /*
        cout<<"GetFaceNo()"<<dms->GetFaceNo()<<endl;
        cout<<"GetCycleNo()"<<dms->GetCycleNo()<<endl;
        cout<<"GetSegmentNo()"<<dms->GetSegmentNo()<<endl;
        cout<<"GetInitialStartX()"<<dms->GetInitialStartX()<<endl;
        cout<<"GetInitialStartY()"<<dms->GetInitialStartY()<<endl;
        cout<<"GetInitialEndX()"<<dms->GetInitialEndX()<<endl;
        cout<<"GetInitialEndY()"<<dms->GetInitialEndY()<<endl;
        cout<<"GetFinalStartX()"<<dms->GetFinalStartX()<<endl;
        cout<<"GetFinalStartY()"<<dms->GetFinalStartY()<<endl;
        cout<<"GetFinalEndX()"<<dms->GetFinalEndX()<<endl;
        cout<<"GetFinalEndY()"<<dms->GetFinalEndY()<<endl;
        cout<<"GetInsideAbove()"<<dms->GetInsideAbove()<<endl;
        */
        double kx1 = (dms->GetFinalStartX() 
        - dms->GetInitialStartX()) / dt;
        cout<<"kx1 "<<kx1<<endl;
        double kx2 = (dms->GetFinalEndX() 
        - dms->GetInitialEndX()) / dt;
        cout<<"kx2 "<<kx2<<endl;
        double ky1 = (dms->GetFinalStartY() 
        - dms->GetInitialStartY()) / dt;
        cout<<"ky1 "<<ky1<<endl;
        double ky2 = (dms->GetFinalEndY() 
        - dms->GetInitialEndY()) / dt;
        cout<<"ky2 "<<ky2<<endl;

        at += ((kx2 - kx1) * (ky1 + ky2)) / 2;
        bt += (((kx2 - kx1) * (dms->GetInitialStartY() 
        + dms->GetInitialEndY())) + ((dms->GetInitialEndX() 
        - dms->GetInitialStartX()) * (ky1 + ky2))) / 2;          
        ct += ((dms->GetInitialStartY() + dms->GetInitialEndY()) 
        * (dms->GetInitialEndX() - dms->GetInitialStartX())) / 2;
       
      }
      cout<<"URegion # "<<n<<endl;
      ures.a = at;
      ures.b = bt;
      ures.c = ct;
      ures.r = false;
      cout<<"ures.a "<<ures.a<<endl;
      cout<<"ures.b "<<ures.b<<endl;
      cout<<"ures.c "<<ures.c<<endl;
      cout<<"ures.r "<<ures.r<<endl;
      res.MergeAdd(ures);
    }
    res.EndBulkLoad(false);
}

/*
1.1.1 Method ~RCenter()~

*/

void RCenter(MRegion& reg, MPoint& res) {
    cout<< "MRegion::RCenter() called" << endl;
    
    int nocomponents = reg.GetNoComponents();
    cout<<"GetNoComponents() "<<nocomponents<<endl;
    res.Clear();
    res.StartBulkLoad();
    for(int n = 0; n < nocomponents; n++){
      const URegion *ur;
      double Ainitial = 0.0, Axinitial = 0.0, Ayinitial = 0.0,
             Afinal = 0.0, Axfinal = 0.0, Ayfinal = 0.0;
      reg.Get(n, ur);
      cout<<"URegion # "<<n<<" "<<"[ "
      <<ur->timeInterval.start.ToDouble()<<" "
      <<ur->timeInterval.end.ToDouble()<<" ]"<<endl;
      int number = ur->GetSegmentsNum();
      cout<<"number = "<< number<<endl;
      const MSegmentData *dms;
      for(int i = 0; i < number; i++){
        ur->GetSegment(i, dms);

       //Calculate Area of Beginning and End of Unit
       Ainitial += (dms->GetInitialEndX() 
       - dms->GetInitialStartX()) * (dms->GetInitialEndY() 
       + dms->GetInitialStartY()) / 2;
       Afinal += (dms->GetFinalEndX() - dms->GetFinalStartX()) 
       * (dms->GetFinalEndY() + dms->GetFinalStartY()) / 2;
       
       double initialax, initialbx, finalax, finalbx, initialay,
       initialby, finalay, finalby; //Ax=ax^3+bx^2 
       
       //Calculate Momentum of Area
       initialax = (dms->GetInitialStartX() 
                 != dms->GetInitialEndX())
       ? ((dms->GetInitialEndY() - dms->GetInitialStartY()) / (
       dms->GetInitialEndX() - dms->GetInitialStartX()) / 3.0)
       : 0.0;
       cout<<"initialax "<<initialax<<endl;
       initialbx = (dms->GetInitialStartY() - 3.0 * initialax 
       * dms->GetInitialStartX()) / 2.0;
       cout<<"initialbx "<<initialbx<<endl;
       finalax = (dms->GetFinalStartX() != dms->GetFinalEndX())
       ? ((dms->GetFinalEndY() - dms->GetFinalStartY()) / (
       dms->GetFinalEndX() - dms->GetFinalStartX()) / 3.0)
       : 0.0;
       cout<<"finalax "<<finalax<<endl;
       finalbx = (dms->GetFinalStartY() - 3.0 * finalax 
       * dms->GetFinalStartX()) / 2.0;
       cout<<"finalbx "<<finalbx<<endl;
       
       initialay = (dms->GetInitialStartY() 
                 != dms->GetInitialEndY())
       ? ((dms->GetInitialEndX() - dms->GetInitialStartX()) / (
       dms->GetInitialEndY() - dms->GetInitialStartY()) / 3.0)
       : 0.0;
       cout<<"initialay "<<initialay<<endl;
       initialby = (dms->GetInitialStartX() - 3.0 * initialay 
       * dms->GetInitialStartY()) / 2.0;
       cout<<"initialby "<<initialby<<endl;
       finalay = (dms->GetFinalStartY() != dms->GetFinalEndY())
       ? ((dms->GetFinalEndX() - dms->GetFinalStartX()) / (
       dms->GetFinalEndY() - dms->GetFinalStartY()) / 3.0)
       : 0.0;
       cout<<"finalay "<<finalay<<endl;
       finalby = (dms->GetFinalStartX() - 3.0 * finalay 
       * dms->GetFinalStartY()) / 2.0;
       cout<<"finalby "<<finalby<<endl;
       
       Axinitial += initialax * pow(dms->GetInitialEndX(), 3) 
       + initialbx * pow(dms->GetInitialEndX(), 2) 
       - initialax * pow(dms->GetInitialStartX(), 3) 
       - initialbx * pow(dms->GetInitialStartX(),2 );
       
       Axfinal += finalax * pow(dms->GetFinalEndX(), 3) 
       + finalbx * pow(dms->GetFinalEndX(), 2) 
       - finalax * pow(dms->GetFinalStartX(), 3) - finalbx 
       * pow(dms->GetFinalStartX(),2 );
       
       Ayinitial += initialay * pow(dms->GetInitialEndY(), 3) 
       + initialby * pow(dms->GetInitialEndY(), 2) 
       - initialay * pow(dms->GetInitialStartY(), 3) 
       - initialby * pow(dms->GetInitialStartY(),2 );
       
       Ayfinal += finalay * pow(dms->GetFinalEndY(), 3) 
       + finalby * pow(dms->GetFinalEndY(), 2) 
       - finalay * pow(dms->GetFinalStartY(), 3) - finalby 
       * pow(dms->GetFinalStartY(),2 );
      }
      cout<<"URegion # "<<n<<endl;
      cout<<"Ainitial"<<Ainitial<<endl;
      cout<<"Afinal"<<Afinal<<endl;
      cout<<"Axinitial"<<Axinitial<<endl;
      cout<<"Axfinal"<<Axfinal<<endl;
      cout<<"Ayinitial"<<Ayinitial<<endl;
      cout<<"Ayfinal"<<Ayfinal<<endl;
      
      if ((Ainitial != 0.0) || (Afinal != 0.0)){
        UPoint *ures;
        if((Ainitial != 0.0) && (Afinal != 0.0)) {
          ures = new UPoint(ur->timeInterval,
          (Axinitial / Ainitial), (-Ayinitial / Ainitial),
          (Axfinal / Afinal), (-Ayfinal / Afinal));
        }
        else if (Ainitial == 0.0) {
          ures = new UPoint(ur->timeInterval,
          dms->GetInitialStartX(), dms->GetInitialStartY(),
          (Axfinal / Afinal), (-Ayfinal / Afinal));
        }
        else {
        ures = new UPoint(ur->timeInterval,
        (Axinitial / Ainitial), (-Ayinitial / Ainitial),
        dms->GetFinalStartX(), dms->GetFinalStartY());
        }
        cout<<"uresXinitial "<<ures->p0.GetX()<<endl;
        cout<<"uresYinitial "<<ures->p0.GetY()<<endl;
        cout<<"uresXfinal "<<ures->p1.GetX()<<endl;
        cout<<"uresXfinal "<<ures->p1.GetY()<<endl;
        res.Add(*ures);
        delete ures;
      }
    }
    res.EndBulkLoad(false);
}

/*
1.1.1 Method ~NComponents()~
It gets for each URegion the last segment and givs the FaceNo of
this segment, because the last segment ist on the last face .

*/

void NComponents(MRegion& reg, MInt& res) {
    cout<< "MRegion::NComponents() called" << endl;
    
    int nocomponents = reg.GetNoComponents();
    cout<<"GetNoComponents() "<<nocomponents<<endl;
    res.Clear();
    res.StartBulkLoad();
    for(int n = 0; n < nocomponents; n++){
      const URegion *ur;
      
      reg.Get(n, ur);
      cout<<"URegion # "<<n<<" "<<"[ "
      <<ur->timeInterval.start.ToDouble()<<" "
      <<ur->timeInterval.end.ToDouble()<<" ]"<<endl;

      cout<<"number = "<< ur->GetSegmentsNum()<<endl;
      const MSegmentData *dms;
      ur->GetSegment(ur->GetSegmentsNum() - 1, dms);
      CcInt *constVal = new CcInt(true, dms->GetFaceNo() + 1);
      UInt *ures = new UInt(ur->timeInterval, *constVal);

      cout<<"ures.constValue "<<ures->constValue.GetIntval()<<endl;
      res.MergeAdd(*ures);
      delete constVal;
      delete ures;
    }
    res.EndBulkLoad(false);
}



template <class Alpha>
bool CompareValue( ConstTemporalUnit<Alpha>& n,
 ConstTemporalUnit<Alpha>& i, int vers )
  {
cout<<"ConstTemporalUnit<Alpha> CompareValue "<<vers<<endl; 
   if (vers == -3)  //#
     return !n.constValue.Compare( &i.constValue ) == 0;
   if (vers == -2)  //<
     return n.constValue.Compare( &i.constValue ) == -1; 
   if (vers == -1)  //<=
     return ((n.constValue.Compare( &i.constValue ) == -1) 
     or (n.constValue.Compare( &i.constValue ) == 0)); 
   if (vers == 0)  //Equal
     return n.constValue.Compare( &i.constValue ) == 0; 
   if (vers == 1)  //>=
     return ((n.constValue.Compare( &i.constValue ) == 1) 
     or (n.constValue.Compare( &i.constValue ) == 0));
   if (vers == 2)  //<
     return n.constValue.Compare( &i.constValue ) == 1; 
   //should not be reached
   return false;
  }
  
/*
Returns true if the value of this TemporalUnit holds the
 comparison holds with the value of the TemporalUnit i.
 The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/

template <class Alpha>
bool CompareValue(ConstTemporalUnit<Alpha>& n, Alpha& i, int vers )
  {
cout<<"ConstTemporalUnit<Alpha> CompareValue "<<vers<<endl; 
   if (vers == -3)  //#
     return !n.constValue.Compare( &i) == 0;
   if (vers == -2)  //<
     return n.constValue.Compare( &i) == -1; 
   if (vers == -1)  //<=
     return ((n.constValue.Compare( &i ) == -1) 
     or (n.constValue.Compare( &i ) == 0)); 
   if (vers == 0)  //Equal
     return n.constValue.Compare( &i ) == 0; 
   if (vers == 1)  //>=
     return ((n.constValue.Compare( &i ) == 1)
     or (n.constValue.Compare( &i ) == 0));
   if (vers == 2)  //<
     return n.constValue.Compare( &i ) == 1; 
   //should not be reached
   return false;
  }
  
/*
Returns true if the value of this TemporalUnit holds the
 comparison holds with the value of i. 
 The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/  

//js
void DistanceUPoint( const UPoint& p1, const UPoint& p2, UReal&
 result, Interval<Instant> iv)
{
  result.timeInterval = iv;
  
  Point rp0, rp1, rp2, rp3;
  double x0, x1, x2, x3, y0, y1, y2, y3, dx, dy, t0, t1;
  
  p1.TemporalFunction(iv.start, rp0);
  p1.TemporalFunction(iv.end, rp1);
  p2.TemporalFunction(iv.start, rp2);
  p2.TemporalFunction(iv.end, rp3);
  
  x0 = rp0.GetX(); y0 = rp0.GetY();
  x1 = rp1.GetX(); y1 = rp1.GetY();
  x2 = rp2.GetX(); y2 = rp2.GetY();
  x3 = rp3.GetX(); y3 = rp3.GetY();
  dx = x1 - x0 - x3 + x2;
  dy = y1 - y0 - y3 + y2;
  t0 = iv.start.ToDouble(),
  t1 = iv.end.ToDouble();

  result.a = pow( (dx) / (t1 - t0), 2 ) +
             pow( (dy) / (t1 - t0), 2 );
  result.b = 2 * ( (x0 - x2) * (dx) / (t1 - t0) +
                   (y0 - y2) * (dy) / (t1 - t0) );
  result.c = pow( x0 - x2, 2 ) + pow( y0 - y2, 2 );
  result.r = true;
}

void DistanceMPoint( MPoint& p1, MPoint& p2, MReal& result) 
{
  UReal uReal;
  
  cout<<"DistanceMPoint called"<<endl;
  RefinementPartitionLift<MPoint, MPoint, UPoint, UPoint> rp(p1, p2);
  cout<<"Refinement abgeschlossen, rp.size: "<<rp.Size()<<endl;

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
    cout<< "Compare interval #"<< i<< ": "
    << iv->start.ToDouble()<< " "
    << iv->end.ToDouble()<< " "<< iv->lc<< " "
    << iv->rc<< " "<< u1Pos<< " "<< u2Pos<< endl;
    
    if (u1Pos == -1 || u2Pos == -1)     
      continue;  
    else {
    cout<<"Both operators existant in interval iv #"<<i<<endl;
      p1.Get(u1Pos, u1);
      p2.Get(u2Pos, u2);
    }
    DistanceUPoint( *u1, *u2, uReal, *iv );
    result.MergeAdd( uReal );
  }
  result.EndBulkLoad( false );
}

/*
Just changes the two given arguments.

*/

void Swop(double& a, double& b)
{
  double i;
  i = a;
  a = b;
  b = i;
}

int SolvePoly(double a, double b, double c, double solution[2],
 bool sort)

/*
  solves the Polynom ax\^2+bx+c=0 and gives back the number of
  solutiones. The solutions are given back in solution
  in ordedered style.

*/

{
  int number = 0;
  double d;

  cout<<"SolvePoly 2 called with a: "<<a<<" ,b: "<<b
  <<" ,c: "<<c<<endl;
  assert (a != 0);
  
  d = pow(b, 2) - 4 * a * c;
  cout<<"d = "<<d<<endl;
  if (d < 0) 
    number = 0;
  else if (d == 0) {
    solution[0] = -b / 2 / a;
    number = 1;
  }
  else {
    if (sort) {
      solution[0] = (a > 0) ? ((-b - sqrt(d)) / 2 / a) 
      : ((-b + sqrt(d)) / 2 / a);
      solution[1] = (a > 0) ? ((-b + sqrt(d)) / 2 / a)
       : ((-b - sqrt(d)) / 2 / a);
    }
    else {
      solution[0] = (-b + sqrt(d)) / 2 / a;
      solution[1] = (-b - sqrt(d)) / 2 / a;
    }
    number = 2;
  }
  cout<<"SolvePoly2 ends with  "<<number<<" solutions"<<endl;
  for (int i = 0; i < number; i++) 
    cout<<"solution["<<i<<"] = "<<solution[i]
    <<"  "<<(a*pow(solution[i],2)+b*solution[i]+c)<<endl;
  return number; 
}

int SolvePoly(double a, double b, double c, double d, 
 double solution[3])

/*
  solves the Polynom ax\^3+bx\^2+cx+d=0 and gives back the number 
  of solutiones. The solutions are given back in solution
  in ordedered style.

*/

{
  int number = 0;
  double disk, p, q;

  cout<<"SolvePoly 3 called with a: "<<a<<" ,b: "<<b
  <<" ,c: "<<c<<" ,d: "<<d<<endl;
  assert (a != 0);
  
  p = ( 3 * a * c - pow(b ,2)) / 9 / pow(a, 2);
  q = (2 * pow(b ,3) - 9 * a * b * c + 27 * pow(a, 2) * d) / 54 
  / pow(a, 3);
  disk = pow(p, 3) + pow (q, 2);
  cout<<"p = "<<p<<endl;
  cout<<"q = "<<q<<endl;
  cout<<"disk = "<<disk<<endl;
  
  if (disk > 0) {  //one real solutuion
    double u = -q + sqrt(disk);
    u = u<0 ? -pow(-u,1.0/3.0) : pow(u,1.0/3.0);
    double v = -q - sqrt(disk);
    v = v<0 ? -pow(-v,1.0/3.0) : pow(v,1.0/3.0);
    cout<<"u "<<u<<endl;
    cout<<"v "<<v<<endl;
    solution[0] = u + v - b / 3 / a;
    number = 1;
  }
  else if (disk == 0) {
    double u = q<0 ? -pow(-q,1.0/3.0) : pow(q,1.0/3.0);
    solution[0] = 2 * u - b / 3 / a;
    solution[1] = -u - b / 3 / a;
    if (solution[0] > solution[1] ) 
      Swop(solution[0], solution[1]);
    number = 2;
  }
  else {
    double phi = acos(-q / sqrt(abs(pow(p, 3))));
    solution[0] = 2 * sqrt(abs(p)) * cos(phi / 3) - b / 3 / a;
    solution[1] = -2 * sqrt(abs(p)) 
    * cos(phi / 3 + 1.047197551196598) - b / 3 / a;
    solution[2] = -2 * sqrt(abs(p)) 
    * cos(phi / 3 - 1.047197551196598) - b / 3 / a;
    
    for (int i = 0; i < number; i++)
    cout<<"solution["<<i<<"] = "<<solution[i]
    <<"  "<<(a*pow(solution[i],3)+b*pow(solution[i],2)
    +c*solution[i]+d)<<endl;
    
    if ( solution[0] > solution[1])
      Swop(solution[0], solution[1]);
    if ( solution[1] > solution[2])
      Swop(solution[1], solution[2]);
    if ( solution[0] > solution[1])
      Swop(solution[0], solution[1]);
    number = 3;
  }
  cout<<"SolvePoly3 ends with  "<<number<<" solutions"<<endl;
  for (int i = 0; i < number; i++)
    cout<<"solution["<<i<<"] = "<<solution[i]
    <<"  "<<(a*pow(solution[i],3)+b*pow(solution[i],2)
    +c*solution[i]+d)<<endl;
  return number; 
}

int SolvePoly(double a, double b, double c, double d, double e,
 double solution[4])

/*
  solves the Polynom ax\^4+bx\^3+cx\^2+dx+d=0 and gives back 
  the number of solutiones. 
  The solutions are given back in solution
  in ordedered style.

*/

{
  int number1 = 0;
  int number2 = 0;
  double z;
  double sol3[3];
  double sol21[2];
  double sol22[2];
  double sol23[2];
  double sol24[2];
  
  cout<<"SolvePoly 4 called with a: "<<a<<" ,b: "<<b
  <<" ,c: "<<c<<" ,d: "<<d<<" ,e: "<<e<<endl;
  assert (a != 0);
  
  number1 = SolvePoly(1.0, -c, (b * d - 4 * a * e), 
  (4 * a * c * e - pow(b, 2)* e - a * pow(d, 2)), sol3);
  for (int i = 0; i < number1; i++)
    cout<<"sol3["<<i<<"] = "<<sol3[i]<<endl;
  z = sol3[number1 - 1];
  cout<<"z "<<z<<endl;
  number1 = SolvePoly(1.0, -b, (a * (c - z)), sol21, false);
  for (int i = 0; i < number1; i++)
    cout<<"sol21["<<i<<"] = "<<sol21[i]<<endl;
  if (number1 == 1)
    sol21[1] = sol21[0];
  number1 = SolvePoly(1.0, -z, (a * e), sol22, false);
  for (int i = 0; i < number1; i++)
    cout<<"sol22["<<i<<"] = "<<sol22[i]<<endl;
  if (number1 == 1)
    sol22[1] = sol22[0];
  if ((b * z)<(2 * a * d)) {
    cout<<"bz < 2ad"<<endl;
    Swop(sol21[0], sol21[1]);
  }
  cout<<"Finding of Solutions for Poly4 starts"<<endl;
  number1 = SolvePoly(a, sol21[0], sol22[0], sol23, true);
  number2 = SolvePoly(a, sol21[1], sol22[1], sol24, true);
  cout<<"number1 "<<number1<<endl;
  cout<<"number2 "<<number2<<endl;
  for (int i = 0; i < number1; i++)
    solution[i] = sol23[i];
  for (int i = 0; i < number2; i++)
    solution[number1 + i] = sol24[i];
  number1 += number2;
  for (int i = 0; i < number1; i++)
    cout<<"solution["<<i<<"] = "<<solution[i]<<endl;
  for (int i = 0; i < number1 ; i++) 
    for (int n = i + 1; n < number1 ; n++) 
      if (solution[i] > solution [n])
        Swop(solution[i],solution[n]);
  for (int i = 0; i < number1 - 1; i++)
    if (AlmostEqual(solution[i], solution[i+1])) {
      for (int n = 0; n < number1 -1; n++)
        solution[n] = solution [n + 1];
      number1--;
    }
  cout<<"SolvePoly4 ends with  "<<number1<<" solutions"<<endl;
  for (int i = 0; i < number1; i++)
        cout<<"solution["<<i<<"] = "<<solution[i]
        <<"  "<<(a*pow(solution[i],4)+b*pow(solution[i],3)
        +c*pow(solution[i],2)+d*solution[i]+e)<<endl;
  return number1; 
}
    
/*
Returns true if the value of these two uReals holds the
comparison.
The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/

static void CompareUReal(UReal u1, UReal u2, UBool& uBool, int op)
{
  Instant middle;
  CcReal value1, value2;
  double mid;
  cout<<"CompareUReal op "<<op<<" called in ["
  <<uBool.timeInterval.start.ToDouble()
  <<" "<<uBool.timeInterval.end.ToDouble()
  <<" "<<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"
  <<endl;

    mid = (uBool.timeInterval.start.ToDouble() 
    + uBool.timeInterval.end.ToDouble())/2.0;
    cout<<"middle: "<<mid<<endl;
    middle.ReadFrom(mid);
    u1.TemporalFunction(middle,value1);     
    cout<<"middle 1 "<<&value1<<endl;
    middle.ReadFrom(mid);
    u2.TemporalFunction(middle,value2);     
    cout<<"middle 2 "<<&value2<<endl;
    if (abs(value1.GetRealval()- value2.GetRealval()) < 0.001 ) {
        //rounding problems!
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
  cout<<"return with "<<&uBool.constValue<<endl;
}

static void ShiftUReal(UReal& op, Instant newstart)

/*
Transforms MReal op to the new start newstart. 
Result of local definition of MReals.

*/

{
  cout<<"ShiftUReal called with a: "<<op.a<<", b: "<<op.b
  <<", c: "<<op.c<<", r: "<<op.r<<" old start: "
  <<op.timeInterval.start.ToDouble()<<", newstart: "
  <<newstart.ToDouble()<<endl;
  double offset = newstart.ToDouble() 
  - op.timeInterval.start.ToDouble();
  cout<<"offset: "<<offset<<endl;
  op.timeInterval.start = newstart;
  op.c = op.a * pow(offset, 2) + op.b * offset + op.c;
  op.b = 2 * op.a * offset + op.b;
  cout<<"ShiftUReal ends with a: "<<op.a<<", b: "<<op.b
  <<", c: "<<op.c<<", r: "<<op.r<<" start: "
  <<op.timeInterval.start.ToDouble()<<endl;
}

/*
Calculates the absolut value of a mReal.

*/

static void MRealABS(MReal& op, MReal& result)
{  
  UReal uReal;
  result.Clear();
  result.StartBulkLoad();
  for(int i = 0; i < op.GetNoComponents(); i++)
  {
    const UReal *u1;
  
    op.Get(i, u1);
    Interval<Instant> iv = u1->timeInterval;
    
    cout<< "MRealABS interval #"<< i<< ": "
    << iv.start.ToDouble()<< " "
    << iv.end.ToDouble()<< " "<< iv.lc<< " "<< iv.rc<<endl;

    Instant t[2];
    Instant mid;
    double middle;
    CcReal value;
    int number;
    int counter = 0;
    double sol2[2];
    
    cout<<"u1.a "<<u1->a<<" u1.b "<<u1->b<<" u1.c "<<u1->c
    <<" u1.r "<<u1->r<<endl;
    if (u1->a != 0.0) {
      number = SolvePoly(u1->a, u1->b, u1->c, sol2, true);
      cout<<"number "<<number<<endl;
      for (int m = 0; m < number; m++) {
         t[m].ReadFrom(sol2[m] 
         + u1->timeInterval.start.ToDouble());
         t[m].SetType(instanttype);
         if (iv.Contains(t[m])) {
           cout<<m<<". crossing in iv"<<endl;
           t[counter] = t[m];
           counter += 1;
        }
        else {
          cout<<m<<". crossing not in iv"<<endl;
        }   
      }
    }
    else if (u1->b != 0.0){
      t[0].ReadFrom(-u1->c / u1->b 
      + u1->timeInterval.start.ToDouble());
      t[0].SetType(instanttype);
      if (iv.Contains(t[0]))
        counter = 1;
      else
        counter = 0;
    }
    else
      counter = 0;
    cout<<"end of if clauses with counter "<<counter<<endl;
    uReal = *u1;
    if (counter == 0) {
      cout<<"no crossings in iv"<<endl;
      middle = (uReal.timeInterval.start.ToDouble() 
      + uReal.timeInterval.end.ToDouble()) / 2;
      mid.ReadFrom(middle);
      uReal.TemporalFunction(mid,value);     
      cout<<"middle "<<middle<<" "<<&value<<endl;
      if (value.GetRealval() < 0.0){
        cout<<"change unit"<<endl;
        uReal.a = -uReal.a;
        uReal.b = -uReal.b;
        uReal.c = -uReal.c;
      }
      cout<<"1uReal "<<uReal.a<<" "<<uReal.b<<" "<<uReal.c
      <<" "<<uReal.r<<" "<<endl;
      result.MergeAdd(uReal);  
    }
    else {
      cout<<counter<<". crossing in iv2"<<endl;
      cout<<"t[0] "<<t[0].ToDouble()<<endl;
      if (u1->timeInterval.start < t[0]) {
        uReal.timeInterval.end = t[0];
        uReal.timeInterval.rc = false;
        middle = (uReal.timeInterval.start.ToDouble() 
        + uReal.timeInterval.end.ToDouble()) / 2;
        mid.ReadFrom(middle);
        uReal.TemporalFunction(mid,value);     
        cout<<"middle "<<middle<<" "<<&value<<endl;
        if (value.GetRealval() < 0.0){
          cout<<"change unit"<<endl;
          uReal.a = -uReal.a;
          uReal.b = -uReal.b;
          uReal.c = -uReal.c;
        }
        cout<<"1uReal "<<uReal.a<<" "<<uReal.b<<" "<<uReal.c
        <<" "<<uReal.r<<" "<<endl;
        result.MergeAdd(uReal); 
      }
      for (int m = 0; m < counter; m++){
        cout<<m<<". crossing in iv"<<endl;
        cout<<"t["<<m<<"] "<<t[m].ToDouble()<<endl;
        uReal = *u1;
        ShiftUReal(uReal, t[m]);
        uReal.timeInterval.lc = true;
        if (u1->timeInterval.start == t[m])
          uReal.timeInterval.lc = u1->timeInterval.lc;
        if (m < counter - 1){ 
          uReal.timeInterval.end = t[m + 1];
          uReal.timeInterval.rc = false;
        }
        else {
          uReal.timeInterval.end = iv.end;
          uReal.timeInterval.rc = iv.rc;
        }
        middle = (uReal.timeInterval.start.ToDouble() 
        + uReal.timeInterval.end.ToDouble()) / 2;
        mid.ReadFrom(middle);
        uReal.TemporalFunction(mid,value);     
        cout<<"middle "<<middle<<" "<<&value<<endl;
        if (value.GetRealval() < 0.0){
          cout<<"change unit"<<endl;
          uReal.a = -uReal.a;
          uReal.b = -uReal.b;
          uReal.c = -uReal.c;
        }
        cout<<"1uReal "<<uReal.a<<" "<<uReal.b<<" "<<uReal.c
        <<" "<<uReal.r<<" "<<endl;
        if (uReal.timeInterval.start < uReal.timeInterval.end)
          result.MergeAdd(uReal); 
      }
    }  
  }
  result.EndBulkLoad(false);
}

/*
Calculates the Distance between the given MReals with respect of
the fact, that the distance can not be negativ.

*/

static void MRealDistanceMM(MReal& op1, MReal& op2, MReal& result)
{  
  UReal uReal;
  
  RefinementPartitionLift<MReal, MReal, UReal, UReal> rp(op1, op2);
 cout<<"Refinement abgeschlossen, rp.size: "<<rp.Size()<<endl;
  
  result.Clear();
  result.StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant>* iv;
    int u1Pos;
    int u2Pos;
    const UReal *u1transfer;
    const UReal *u2transfer;
    UReal u1;
    UReal u2;
  
    rp.Get(i, iv, u1Pos, u2Pos);
    
    cout<< "MRealDistanceMM interval #"<< i<< ": "
    << iv->start.ToDouble()<< " "
    << iv->end.ToDouble()<< " "<< iv->lc<< " "<< iv->rc<<endl;

    if (u1Pos == -1 || u2Pos == -1)     
       continue;  
    cout<<"Both operators existant in interval iv #"<<i<<
    " ["<<iv->start.ToDouble()<<"  "<<iv->end.ToDouble()
    <<" "<<iv->lc<<" "<<iv->rc<<"]"<<endl;
    op1.Get(u1Pos, u1transfer);
    op2.Get(u2Pos, u2transfer);
    u1 = *u1transfer;
    u2 = *u2transfer;
    ShiftUReal(u1, iv->start);
    ShiftUReal(u2, iv->start);
    u1.timeInterval = *iv;
    u2.timeInterval = *iv;
    
    Instant t[2];
    Instant mid;
    double middle;
    CcReal value;
    int number;
    int counter = 0;
    double sol2[2];
    
    cout<<"u1.a "<<u1.a<<" u1.b "<<u1.b<<" u1.c "<<u1.c
    <<" u1.r "<<u1.r<<endl;
    cout<<"u2.a "<<u2.a<<" u2.b "<<u2.b<<" u2.c "<<u2.c
    <<" u2.r "<<u2.r<<endl;
    
    if (u1.r != u2.r)
       continue;
    
    if (u1.a != u2.a) {
      number = SolvePoly(u1.a - u2.a, u1.b - u2.b, u1.c - u2.c,
       sol2, true);
      cout<<"number "<<number<<endl;
      for (int m = 0; m < number; m++) {
         t[m].ReadFrom(sol2[m] 
         + u1.timeInterval.start.ToDouble());
         t[m].SetType(instanttype);
         if (iv->Contains(t[m])) {
           cout<<m<<". crossing in iv"<<endl;
           t[counter] = t[m];
           counter += 1;
        }
        else {
          cout<<m<<". crossing not in iv"<<endl;
        }   
      }
    }
    else if (u1.b != u2.b){
      t[0].ReadFrom(-(u1.c - u2.c) / (u1.b - u2.b) 
      + u1.timeInterval.start.ToDouble());
      t[0].SetType(instanttype);
      if (iv->Contains(t[0]))
        counter = 1;
      else
        counter = 0;
    }    
    else
      counter = 0;
    cout<<"end of if clauses with counter "<<counter<<endl;
    uReal = u1;
    uReal.a = u1.a - u2.a;
    uReal.b = u1.b - u2.b;
    uReal.c = u1.c - u2.c;
    if (counter == 0) {
      cout<<"no crossings in iv"<<endl;
      middle = (uReal.timeInterval.start.ToDouble() 
      + uReal.timeInterval.end.ToDouble()) / 2;
      mid.ReadFrom(middle);
      uReal.r = false;
      //no TemporalFunction for value < 0!
      uReal.TemporalFunction(mid,value); 
      uReal.r = u2.r;    
      cout<<"middle "<<middle<<" "<<&value<<endl;
      if (value.GetRealval() < 0.0){
        cout<<"change unit"<<endl;
        uReal.a = -uReal.a;
        uReal.b = -uReal.b;
        uReal.c = -uReal.c;
      }
      cout<<"1uReal "<<uReal.a<<" "<<uReal.b<<" "<<uReal.c
      <<" "<<uReal.r<<" ["<<uReal.timeInterval.start.ToDouble()
      <<"  "<<uReal.timeInterval.end.ToDouble()<<" "
      <<uReal.timeInterval.lc<<" "<<uReal.timeInterval.rc<<"]"
      <<endl;
      result.MergeAdd(uReal);  
    }
    else {
      cout<<"first crossing in iv2"<<endl;
      cout<<"t[0] "<<t[0].ToDouble()<<endl;
      if (u1.timeInterval.start < t[0]) {
        uReal.timeInterval.end = t[0];
        uReal.timeInterval.rc = false;
        middle = (uReal.timeInterval.start.ToDouble() 
        + uReal.timeInterval.end.ToDouble()) / 2;
        mid.ReadFrom(middle);
        uReal.r = false;
        //no TemporalFunction for value < 0!
        uReal.TemporalFunction(mid,value);  
        uReal.r = u2.r;   
        cout<<"middle "<<middle<<" "<<&value<<endl;
        if (value.GetRealval() < 0.0){
          cout<<"change unit"<<endl;
          uReal.a = -uReal.a;
          uReal.b = -uReal.b;
          uReal.c = -uReal.c;
        }
        cout<<"1uReal "<<uReal.a<<" "<<uReal.b<<" "<<uReal.c
        <<" "<<uReal.r<<" ["<<uReal.timeInterval.start.ToDouble()
        <<"  "<<uReal.timeInterval.end.ToDouble()
        <<" "<<uReal.timeInterval.lc<<" "
        <<uReal.timeInterval.rc<<"]"<<endl;;
        result.MergeAdd(uReal); 
      }
      for (int m = 0; m < counter; m++){
        cout<<m<<". crossing in iv"<<endl;
        cout<<"t["<<m<<"] "<<t[m].ToDouble()<<endl;
        uReal = u1;
        uReal.a = u1.a - u2.a;
        uReal.b = u1.b - u2.b;
        uReal.c = u1.c - u2.c;
        ShiftUReal(uReal, t[m]);
        uReal.timeInterval.lc = true;
        if (u1.timeInterval.start == t[m]){
          cout<<"(u1.timeInterval.start == t[m])"<<endl;
          uReal.timeInterval.lc = iv->lc;
        }
        if (m < counter - 1){ 
        cout<<"m < counter - 1"<<endl;
          uReal.timeInterval.end = t[m + 1];
          uReal.timeInterval.rc = false;
        }
        else {
        cout<<"m >= counter - 1"<<endl;
          uReal.timeInterval.end = iv->end;
          uReal.timeInterval.rc = iv->rc;
        }
        middle = (uReal.timeInterval.start.ToDouble() 
        + uReal.timeInterval.end.ToDouble()) / 2;
        mid.ReadFrom(middle);
        uReal.r = false; 
        //no TemporalFunction for value < 0!
        uReal.TemporalFunction(mid,value);     
        uReal.r = u2.r;
        cout<<"middle "<<middle<<" "<<&value<<endl;
        if (value.GetRealval() < 0.0){
          cout<<"change unit"<<endl;
          uReal.a = -uReal.a;
          uReal.b = -uReal.b;
          uReal.c = -uReal.c;
        }
        cout<<"1uReal "<<uReal.a<<" "<<uReal.b<<" "<<uReal.c
        <<" "<<uReal.r<<" ["<<uReal.timeInterval.start.ToDouble()
        <<"  "<<uReal.timeInterval.end.ToDouble()<<" "
        <<uReal.timeInterval.lc<<" "<<uReal.timeInterval.rc<<"]"
        <<endl;
        if (uReal.timeInterval.start < uReal.timeInterval.end)
          result.MergeAdd(uReal); 
      }
    }  
  }
  result.EndBulkLoad(false);
}

/*
Returns true if the value of these two mReals holds the
comparison.
The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/

static void MovingRealCompareMM(MReal& op1, MReal& op2, MBool&
 result, int op)
{
  UBool uBool;
   
  RefinementPartitionLift<MReal, MReal, UReal, UReal> rp(op1, op2);
 cout<<"Refinement abgeschlossen, rp.size: "<<rp.Size()<<endl;
  
  result.Clear();
  result.StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant>* iv;
    int u1Pos;
    int u2Pos;
    const UReal *u1transfer;
    const UReal *u2transfer;
    UReal u1;
    UReal u2;
    
    rp.Get(i, iv, u1Pos, u2Pos);
  cout<< "TemporalMMRealCompare interval #"<< i<< ": "
  << iv->start.ToDouble()<< " "
  << iv->end.ToDouble()<< " "<< iv->lc<< " "<< iv->rc<< " "
  << u1Pos<< " "<< u2Pos<<"  op "<<op<<endl;

    if (u1Pos == -1 || u2Pos == -1)     
      continue;  
  cout<<"Both operators existant in interval iv #"<<i<<endl;
    op1.Get(u1Pos, u1transfer);
    op2.Get(u2Pos, u2transfer);
    u1 = *u1transfer;
    u2 = *u2transfer;
    cout<<"u1.a "<<u1.a<<" u1.b "<<u1.b<<" u1.c "<<u1.c
    <<" u1.r "<<u1.r<<endl;
    cout<<"u2.a "<<u2.a<<" u2.b "<<u2.b<<" u2.c "<<u2.c
    <<" u2.r "<<u2.r<<endl;
    ShiftUReal(u1, iv->start);
    ShiftUReal(u2, iv->start);
    u1.timeInterval = *iv;
    u2.timeInterval = *iv;
    Instant t[4];
    Instant middle;
    int number;
    int counter = 0;
    double sol2[2];
    double sol3[3];
    double sol4[4];
    cout<<"afer shifting the units"<<endl;
    cout<<"u1.a "<<u1.a<<" u1.b "<<u1.b<<" u1.c "<<u1.c
    <<" u1.r "<<u1.r<<endl;
    cout<<"u2.a "<<u2.a<<" u2.b "<<u2.b<<" u2.c "<<u2.c
    <<" u2.r "<<u2.r<<endl;
    if (u1.r == u2.r) {
      cout<<"u1.r == u2.r"<<endl;
      if (u1.a == u2.a) {
        cout<<"u1.a == u2.a"<<endl; 
        if (u1.b == u2.b) {
          cout<<"u1.b == u2.b"<<endl;
            if (u1.c == u2.c) {
              cout<<"u1.c == u2.c -> same horizontal line!"<<endl;
              number = 0;
            }
            else {
          cout<<"u1.c != u2.c -> parallel horizontal lines"<<endl;
              number = 0;
            }
        }
        else {
          cout<<"u1.b != u2.b"<<endl;
          double crossing = (u2.c - u1.c)/(u1.b - u2.b);
          t[0].ReadFrom(crossing 
          + u1.timeInterval.start.ToDouble());
          number = 1;
        }
      }
      else {
        cout<<"u1.a != u2.a"<<endl;
        number = SolvePoly(u1.a - u2.a, u1.b - u2.b,
         u1.c - u2.c, sol2, true);
        cout<<"number "<<number<<endl;
        for (int m = 0; m < number; m++) 
           t[m].ReadFrom(sol2[m] 
           + u1.timeInterval.start.ToDouble());
      }
    }
    else {
     cout<<"u1.r != u2.r"<<endl;
     if (u2.r && u2.a == 0 && u2.b == 0) {
        cout<<"Spezial case u2 = const"<<endl;
        number = SolvePoly(u1.a, u1.b, u1.c - sqrt(u2.c)
        , sol2, true);
        cout<<"number "<<number<<endl;
        for (int m = 0; m < number; m++) 
           t[m].ReadFrom(sol2[m] 
           + u1.timeInterval.start.ToDouble());
     }
     else if (u1.r && u1.a == 0 && u1.b == 0) {
        cout<<"Spezial case u1 = const"<<endl;
        number = SolvePoly(u2.a, u2.b, u2.c - sqrt(u1.c),
         sol2, true);
        cout<<"number "<<number<<endl;
        for (int m = 0; m < number; m++) 
           t[m].ReadFrom(sol2[m] 
           + u1.timeInterval.start.ToDouble());
     }
     else{ 
      double v, w, x, y, z;
      if (u1.r) {
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
      if (v == 0) {
        cout<<"v == 0"<<endl;
        if (w == 0) {
          cout<<"w == 0"<<endl;
            if (x == 0) {
              cout<<"x == 0"<<endl; 
              if (y == 0) {
                cout<<"y == 0"<<endl; 
                if (z == 0) {
                  cout<<"z == 0 -> totaly equal"<<endl; 
                  number = 0;
                }
               else {
                 cout<<"z != 0 -> paralell"<<endl;
                  number = 0;
                }
              }
              else {
                cout<<"y != 0"<<endl;
                double crossing = -z / y;
                t[0].ReadFrom(crossing 
                + u1.timeInterval.start.ToDouble());
                number = 1;
              }
            }
            else {
              cout<<"x != 0"<<endl;
              number = SolvePoly(x, y, z, sol2, true);
              for (int m = 0; m < number; m++) 
                 t[m].ReadFrom(sol2[m] 
                 + u1.timeInterval.start.ToDouble());
            }
          }
        
        else {
          cout<<"w != 0"<<endl;
          number = SolvePoly(w, x, y, z, sol3);
          for (int m = 0; m < number; m++) 
            t[m].ReadFrom(sol3[m] 
            + u1.timeInterval.start.ToDouble());
        }
      }
      else {
        cout<<"v != 0"<<endl;
        number = SolvePoly(v, w, x, y, z, sol4);
        for (int m = 0; m < number; m++) 
          t[m].ReadFrom(sol4[m] 
          + u1.timeInterval.start.ToDouble());   
      }
     }
    }
    cout<<"number = "<<number<<endl;
    for (int m = 0; m < number; m++) {
      t[m].SetType(instanttype);
      if ((*iv).Contains(t[m])) {
        cout<<m<<". crossing in iv"<<endl;
        t[counter] = t[m];
        counter += 1;
      }
      else {
        cout<<m<<". crossing not in iv"<<endl;
      }   
    }
    cout<<"end of if clauses with counter "<<counter<<endl;
    uBool.timeInterval = *iv;
    if (counter == 0) {
      cout<<"no crossings in iv"<<endl;
      CompareUReal(u1, u2, uBool, op);
      result.MergeAdd(uBool);  
    }
    else {
      cout<<counter<<" crossings in iv2"<<endl;
      cout<<"iv ["<<iv->start.ToDouble()<< " "
      << iv->end.ToDouble()<< " "<< iv->lc<< " "
      << iv->rc<<"]"<<endl;
      cout<<"t[0] "<<t[0].ToDouble()<<endl;
      if (iv->start < t[0]) {
        uBool.timeInterval.end = t[0];
        uBool.timeInterval.rc = false;
        CompareUReal(u1, u2, uBool, op);
        cout<<"1uBool "<<&uBool.constValue<<endl;
        result.MergeAdd(uBool);
      }
      for (int m = 0; m < counter; m++){
        cout<<m<<". crossing in iv"<<endl;
        cout<<"t["<<m<<"] "<<t[m].ToDouble()<<endl;
        uBool.timeInterval.start = t[m];
        uBool.timeInterval.end = t[m];
        uBool.timeInterval.lc = true;
        uBool.timeInterval.rc = true;
        CompareUReal(u1, u2, uBool, op);
        cout<<"2uBool "<<&uBool.constValue<<endl;
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
        cout<<"3uBool "<<&uBool.constValue<<endl;
        if (uBool.timeInterval.start < uBool.timeInterval.end)
          result.MergeAdd(uBool);
      }
    }  
  }
  result.EndBulkLoad(false);
}

/*
Calculates the intersecion between two given mReals

*/

static void MovingRealIntersectionMM(MReal& op1, MReal& op2,
 MReal& result, int op)
{
 UReal un;
   
 RefinementPartitionLift<MReal, MReal, UReal, UReal> rp(op1, op2);
 cout<<"Refinement abgeschlossen, rp.size: "<<rp.Size()<<endl;
  
 result.Clear();
 result.StartBulkLoad();
 for(unsigned int i = 0; i < rp.Size(); i++)
 {
   Interval<Instant>* iv;
   int u1Pos;
   int u2Pos;
   const UReal *u1transfer;
   const UReal *u2transfer;
   UReal u1;
   UReal u2;
    
  rp.Get(i, iv, u1Pos, u2Pos);
  cout<< "TemporalMMRealIntersection interval #"<< i<< ": "
  << iv->start.ToDouble()<< " "
  << iv->end.ToDouble()<< " "<< iv->lc<< " "<< iv->rc<< " "
  << u1Pos<< " "<< u2Pos<<"  op "<<op<<endl;

  if (u1Pos == -1 || (u2Pos == -1 && op != 2))    
      continue; 
  else if (u2Pos == -1) {
      cout<<"only 1. operator existant in interval iv #"<<i<<endl;
      op1.Get(u1Pos, u1transfer);
      u1 = *u1transfer;
      un = u1;
      ShiftUReal(u1, iv->start);
      u1.timeInterval = *iv;
      result.MergeAdd(un);
  }
  else {
    cout<<"Both operators existant in interval iv #"<<i<<endl;
    op1.Get(u1Pos, u1transfer);
    op2.Get(u2Pos, u2transfer);
    u1 = *u1transfer;
    u2 = *u2transfer;
    cout<<"u1.a "<<u1.a<<" u1.b "<<u1.b<<" u1.c "<<u1.c
    <<" u1.r "<<u1.r<<endl;
    cout<<"u2.a "<<u2.a<<" u2.b "<<u2.b<<" u2.c "<<u2.c
    <<" u2.r "<<u2.r<<endl;
    ShiftUReal(u1, iv->start);
    ShiftUReal(u2, iv->start);
    u1.timeInterval = *iv;
    u2.timeInterval = *iv;
    Instant t[4];
    Instant middle;
    int number;
    int counter = 0;
    double sol2[2];
    double sol3[3];
    double sol4[4];
    cout<<"afer shifting the units"<<endl;
    cout<<"u1.a "<<u1.a<<" u1.b "<<u1.b<<" u1.c "<<u1.c
    <<" u1.r "<<u1.r<<endl;
    cout<<"u2.a "<<u2.a<<" u2.b "<<u2.b<<" u2.c "<<u2.c
    <<" u2.r "<<u2.r<<endl;
    if (u1.r == u2.r) {
      cout<<"u1.r == u2.r"<<endl;
      if (u1.a == u2.a) {
        cout<<"u1.a == u2.a"<<endl; 
        if (u1.b == u2.b) {
          cout<<"u1.b == u2.b"<<endl;
            if (u1.c == u2.c) {
              cout<<"u1.c == u2.c -> same horizontal line!"<<endl;
              number = 0;
            }
            else {
          cout<<"u1.c != u2.c -> parallel horizontal lines"<<endl;
              number = 0;
            }
        }
        else {
          cout<<"u1.b != u2.b"<<endl;
          double crossing = (u2.c - u1.c)/(u1.b - u2.b);
          t[0].ReadFrom(crossing 
          + u1.timeInterval.start.ToDouble());
          number = 1;
        }
      }
      else {
        cout<<"u1.a != u2.a"<<endl;
        number = SolvePoly(u1.a - u2.a, u1.b - u2.b, u1.c - u2.c,
         sol2, true);
        cout<<"number "<<number<<endl;
        for (int m = 0; m < number; m++) 
           t[m].ReadFrom(sol2[m] 
           + u1.timeInterval.start.ToDouble());
      }
    }
    else {
     cout<<"u1.r != u2.r"<<endl;
     if (u2.r && u2.a == 0 && u2.b == 0) {
        cout<<"Spezial case u2 = const"<<endl;
        number = SolvePoly(u1.a, u1.b, u1.c - sqrt(u2.c), sol2,
         true);
        cout<<"number "<<number<<endl;
        for (int m = 0; m < number; m++) 
           t[m].ReadFrom(sol2[m] 
           + u1.timeInterval.start.ToDouble());
     }
     else if (u1.r && u1.a == 0 && u1.b == 0) {
        cout<<"Spezial case u1 = const"<<endl;
        number = SolvePoly(u2.a, u2.b, u2.c - sqrt(u1.c), sol2,
         true);
        cout<<"number "<<number<<endl;
        for (int m = 0; m < number; m++) 
           t[m].ReadFrom(sol2[m] 
           + u1.timeInterval.start.ToDouble());
     }
     else{ 
      double v, w, x, y, z;
      if (u1.r) {
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
      if (v == 0) {
        cout<<"v == 0"<<endl;
        if (w == 0) {
          cout<<"w == 0"<<endl;
            if (x == 0) {
              cout<<"x == 0"<<endl; 
              if (y == 0) {
                cout<<"y == 0"<<endl; 
                if (z == 0) {
                  cout<<"z == 0 -> totaly equal"<<endl; 
                  number = 0;
                }
                else {
                  cout<<"z != 0 -> paralell"<<endl;
                  number = 0;
                }
              }
              else {
                cout<<"y != 0"<<endl;
                double crossing = -z / y;
                t[0].ReadFrom(crossing 
                + u1.timeInterval.start.ToDouble());
                number = 1;
              }
            }
            else {
              cout<<"x != 0"<<endl;
              number = SolvePoly(x, y, z, sol2, true);
              for (int m = 0; m < number; m++) 
                 t[m].ReadFrom(sol2[m] 
                 + u1.timeInterval.start.ToDouble());
            }
          }
        
        else {
          cout<<"w != 0"<<endl;
          number = SolvePoly(w, x, y, z, sol3);
          for (int m = 0; m < number; m++) 
            t[m].ReadFrom(sol3[m] 
            + u1.timeInterval.start.ToDouble());
        }
      }
      else {
        cout<<"v != 0"<<endl;
        number = SolvePoly(v, w, x, y, z, sol4);
        for (int m = 0; m < number; m++) 
          t[m].ReadFrom(sol4[m] 
          + u1.timeInterval.start.ToDouble());   
      }
     }
    }
    cout<<"number = "<<number<<endl;
    counter = 0;
    for (int m = 0; m < number; m++) {
      t[m].SetType(instanttype);
      if ((*iv).Contains(t[m])) {
        cout<<m<<". crossing in iv"<<endl;
        t[counter] = t[m];
        counter += 1;
      }
      else {
        cout<<m<<". crossing not in iv"<<endl;
      }   
    }
    cout<<"end of if clauses with counter "<<counter<<endl;
    UBool uBool;
    if (counter == 0) {
      cout<<"no crossings in iv"<<endl;
      CompareUReal(u1, u2, uBool, 0);
      if ((op == 1 && uBool.constValue.GetBoolval()) 
      || (op == 2 && !uBool.constValue.GetBoolval())){
        cout<<"just add"<<endl;
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
            cout<<"add point"<<endl;
            un = u1;
            ShiftUReal(un, t[m]);
            un.timeInterval.end = t[m];
            un.timeInterval.lc = true;
            un.timeInterval.rc = true;
            result.MergeAdd(un); 
          }
        }
      }
      else {
        cout<<"add interval"<<endl;
        if (t[0] > iv->start){
          un = u1;
          un.timeInterval = *iv;
          un.timeInterval.end = t[0];
          un.timeInterval.rc = false;
          cout<<"add first interval ["
          <<un.timeInterval.start.ToDouble()
          <<" "<<un.timeInterval.end.ToDouble()
          <<" "<<un.timeInterval.lc
          <<" "<<un.timeInterval.rc<<"]"<<endl;
          result.MergeAdd(un);
        }
        for (int m = 0; m < counter; m++){
          un = u1;
          ShiftUReal(un, t[m]);
cout<<"nach shift "<<m<<". interval ["
<<un.timeInterval.start.ToDouble()<<" "
<<un.timeInterval.end.ToDouble()<<" "<<un.timeInterval.lc<<" "
<<un.timeInterval.rc<<"] ";
cout<<un.a<< " "<<un.b<<" "<<un.c<<" "<<un.r<<endl;
          un.timeInterval.start = t[m];//sehr seltsam
          un.timeInterval.lc = false;
          if (m < counter - 1){ 
            un.timeInterval.end = t[m+1];
            un.timeInterval.rc = false;
          }
          else {
            un.timeInterval.end = iv->end;
            un.timeInterval.rc = iv->rc;
          }
          cout<<"add "<<m<<". interval ["
          <<un.timeInterval.start.ToDouble()
          <<" "<<un.timeInterval.end.ToDouble()<<" "
          <<un.timeInterval.lc<<" "
          <<un.timeInterval.rc<<"]"<<endl;
          if (un.timeInterval.start < un.timeInterval.end 
          || (un.timeInterval.lc && un.timeInterval.rc))
            result.MergeAdd(un);
        }
      }
    }  
   }
 }
 result.EndBulkLoad(false);
}

/*
Returns true if the value of the mReal and CcReal holds the
comparison.
The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/

static void MovingRealCompareMS(MReal& op1,CcReal& op2, MBool&
 result, int op)
{
  cout<<"MovingRealCompareMS called"<<endl;
  MReal *mop2 = new MReal(0); 
  const UReal *up1;
  
  mop2->Clear();
  mop2->StartBulkLoad();
  for (int i = 0; i < op1.GetNoComponents(); i++) {
    op1.Get(i, up1);
    UReal *up2 = new UReal(up1->timeInterval, 0.0, 0.0, up1->r 
      ? pow(op2.GetRealval(),2) : op2.GetRealval(), up1->r);
    cout<<"up1["<<i<<"] ["<<up1->timeInterval.start.ToDouble()
    <<" "<<up1->timeInterval.end.ToDouble()
    <<" "<<up1->timeInterval.lc<<" "<<up1->timeInterval.rc<<"] "
    <<" a: "<<up1->a<<" b: "<<up1->b<<" c: "<<up1->c<<" r: "
    <<up1->r<<endl;

    cout<<"up2["<<i<<"] ["<<up2->timeInterval.start.ToDouble()
    <<" "<<up2->timeInterval.end.ToDouble()
    <<" "<<up2->timeInterval.lc<<" "<<up2->timeInterval.rc<<"] "
    <<" a: "<<up2->a<<" b: "<<up2->b<<" c: "<<up2->c<<" r: "
    <<up2->r<<endl;
    mop2->Add(*up2);
    delete up2;
  }
  mop2->EndBulkLoad(false);
  MovingRealCompareMM(op1, *mop2, result, op);
  
  delete mop2;
  
}

double MPointInMPoint(double startx1, double starty1, double endx1,
                     double endy1, double startx2, double starty2,
                     double endx2, double endy2){
/*
Function MPointInMPoint checks whether the two mpoints are crossing.
It returns -1.0 if the mpoints are not crossing and 
2.0 if both mpoints are equal all the time. In all other cases it 
return thr fraction of iv when mpoints are crossing (0.0 .. 1.0).

*/  
  if(startx1 == startx2 && starty1 == starty2 && endx1 == endx2 
  && endy1 == endy2) {
    if(TLA_DEBUG)
      cout<<"start and end equal"<<endl;
    return 2.0;
  }
  else if(startx1 == startx2 && starty1 == starty2) {  
    if(TLA_DEBUG)
      cout<<"only start equal"<<endl;
    return 0.0;
  }
  else if(endx1 == endx2 && endy1 == endy2) { 
    return 1.0;
  }
  else {
    if(TLA_DEBUG)
      cout<<"neither start nor end equal"<<endl;
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
    if(TLA_DEBUG){
      cout<<" tx "<<tx<<" "<<vert<<" , ty "<<ty<<
      " "<<hor<<endl;}
    if (hor) {
      if ((starty1 <= starty2 && starty1 >= endy2) 
       || (starty1 <= endy2 && starty1 >= endy2))
          ty = tx;
      else
        ty = -1.0;
    }
    else if (vert) {
      if ((startx1 <= startx2 && startx1 >= endx2) 
        || (startx1 <= endx2 && startx1 >= startx2))
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
For Operators ~=~ and ~\#~ and MovingPoint/MovingPoint

*/

void MovingPointCompareMM( MPoint& p1, MPoint& p2, MBool& result,
 int op) 
{
  UBool uBool;
  
  if(TLA_DEBUG)
    cout<<"MovingPointCompareMM called"<<endl;
  RefinementPartitionLift<MPoint, MPoint, UPoint, UPoint> rp(p1, p2);
  if(TLA_DEBUG)
    cout<<"Refinement abgeschlossen, rp.size: "<<rp.Size()<<endl;

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
    if(TLA_DEBUG){
      cout<< "Compare interval #"<< i<< ": "
      << iv->start.ToDouble()<< " "
      << iv->end.ToDouble()<< " "<< iv->lc<< " "
      << iv->rc<< " "<< u1Pos<< " "<< u2Pos<< endl;}
    
    if (u1Pos == -1 || u2Pos == -1)     
      continue;  
    else {
      if(TLA_DEBUG)
        cout<<"Both operators existant in interval iv #"<<i<<endl;
      p1.Get(u1Pos, u1);
      p2.Get(u2Pos, u2);
    }
  
    Point rp0, rp1, rp2, rp3;
  
    u1->TemporalFunction(iv->start, rp0);
    u1->TemporalFunction(iv->end, rp1);
    u2->TemporalFunction(iv->start, rp2);
    u2->TemporalFunction(iv->end, rp3);
    
    double t = MPointInMPoint(rp0.GetX(), rp0.GetY(), rp1.GetX(),
                    rp1.GetY(),rp2.GetX(), rp2.GetY(),
                    rp3.GetX(), rp3.GetY());
    if(t == 2.0){
      if(TLA_DEBUG)
        cout<<"start and end equal"<<endl;
      uBool.timeInterval = *iv;
      uBool.constValue.Set(true, op == 0 ? true : false);
      result.MergeAdd( uBool );
    }
    else if(t == 0.0){
      if(TLA_DEBUG)
        cout<<"only start equal"<<endl;
      if (iv->lc) {
        if(TLA_DEBUG)
          cout<<"point ok"<<endl;
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
      result.MergeAdd( uBool );
    }
    else if(t == 1.0){
      if(TLA_DEBUG)
        cout<<"only end equal"<<endl;
      uBool.timeInterval = *iv;
      uBool.timeInterval.rc = false;
      uBool.constValue.Set(true, op == 0 ? false : true);
      result.MergeAdd( uBool );
      if (iv->rc) {
        if(TLA_DEBUG)
          cout<<"point ok"<<endl;
        uBool.timeInterval.start = iv->end;
        uBool.timeInterval.end = iv->end;
        uBool.timeInterval.lc = true;
        uBool.timeInterval.rc = true;;
        uBool.constValue.Set(true, op == 0 ? true : false);
        result.MergeAdd( uBool );
      }
    }
    else if(t > 0.0 && t < 1.0){
      if(TLA_DEBUG)
        cout<<"crossing -> one point equal"<<endl;
      Instant time;
      time.ReadFrom(t  * (iv->end.ToDouble() 
                       - iv->start.ToDouble()) 
                       + iv->start.ToDouble());
      time.SetType(instanttype);
      uBool.timeInterval = *iv;
      uBool.timeInterval.rc = false;
      uBool.timeInterval.end = time;
      uBool.constValue.Set(true, op == 0 ? false : true);
      result.MergeAdd( uBool );
      uBool.timeInterval.rc = true;
      uBool.timeInterval.start = time;
      uBool.timeInterval.lc = true;
      uBool.constValue.Set(true, op == 0 ? true : false);
      result.MergeAdd( uBool );
      uBool.timeInterval.lc = false;
      uBool.timeInterval.rc = iv->rc;
      uBool.timeInterval.end = iv->end;
      uBool.constValue.Set(true, op == 0 ? false : true);
      result.MergeAdd( uBool );
    }
    else {
      if(TLA_DEBUG)
        cout<<"no crossing -> no equal"<<endl;
      uBool.timeInterval = *iv;
      uBool.constValue.Set(true, op == 0 ? false : true);
      result.MergeAdd( uBool );
    }
  }
  result.EndBulkLoad( false );
}

/*
For Operators ~=~ and ~\#~ and MovingPoint/Point

*/

void MovingPointCompareMS( MPoint& p1, Point& p2, MBool& result,
 int op) 
{
  UBool uBool;

  result.Clear();
  result.StartBulkLoad();
  for( int i = 0; i < p1.GetNoComponents(); i++ )
  {
    Interval<Instant> iv;
    const UPoint *u1; 
    
    p1.Get(i, u1);
    iv = u1->timeInterval; 
    if(TLA_DEBUG){
      cout<< "Compare interval #"<< i<< ": "
      << iv.start.ToDouble()<< " "
      << iv.end.ToDouble()<< " "<< iv.lc<< endl;}

    Point rp0, rp1, rp2;
  
    u1->TemporalFunction(iv.start, rp0);
    u1->TemporalFunction(iv.end, rp1);
    rp2 = p2;
    
    if(rp0 == rp2 && rp1 == rp2) {  //start and end equal
      if(TLA_DEBUG)
        cout<<"start and end equal"<<endl;
      uBool.timeInterval = iv;
      uBool.constValue.Set(true, op == 0 ? true : false);
      result.MergeAdd( uBool );
    }
    else if(rp0 == rp2) {  //only start equal
      if(TLA_DEBUG)
        cout<<"only start equal"<<endl;
      if (iv.lc) {
        if(TLA_DEBUG)
          cout<<"point ok"<<endl;
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
    else if(rp1 == rp2) {  //only end equal
      if(TLA_DEBUG)
        cout<<"only end equal"<<endl;
      uBool.timeInterval = iv;
      uBool.timeInterval.rc = false;
      uBool.constValue.Set(true, op == 0 ? false : true);
      result.MergeAdd( uBool );
      if (iv.rc) {
        if(TLA_DEBUG)
          cout<<"point ok"<<endl;
        uBool.timeInterval.start = iv.end;
        uBool.timeInterval.end = iv.end;
        uBool.timeInterval.lc = true;
        uBool.timeInterval.rc = true;
        uBool.constValue.Set(true, op == 0 ? true : false);
        result.MergeAdd( uBool );
      }
    }
    else { //neither start nor end equal
      if(TLA_DEBUG)
        cout<<"neither start nor end equal"<<endl;
      double x0, x1, x2, y0, y1, y2, dx, dy, t0, t1, tx, ty;
      bool vert = false;
      bool hor = false;
      x0 = rp0.GetX(); y0 = rp0.GetY();
      x1 = rp1.GetX(); y1 = rp1.GetY();
      x2 = rp2.GetX(); y2 = rp2.GetY();
      dx = x1 - x0;
      dy = y1 - y0;
      t0 = iv.start.ToDouble(),
      t1 = iv.end.ToDouble();
      
      if (dx == 0.0)
        vert = true;
      else
        tx = (x2 - x0) / dx;
      if (dy == 0.0)
        hor = true;
      else 
        ty = (y2 - y0) / dy;
      if(TLA_DEBUG){
        cout<<" tx "<<tx<<" "<<vert<<" , ty "<<ty<<
        " "<<hor<<endl;}
      if (hor) {
        if(TLA_DEBUG)
          cout<<"horzizontal vector"<<endl;
        if (AlmostEqual(y0, y2))
          ty = tx;
        else
          ty = 0.0;
      }
      else if (vert) {
        if (AlmostEqual(x0, x2))
          tx = ty;
        else
          tx = 0.0;
      }

      if(tx == ty && tx > 0.0 && tx < 1.0) {
        if(TLA_DEBUG)
          cout<<"crossing -> one point equal"<<endl;
        Instant t;
        t.ReadFrom(tx  * (iv.end.ToDouble() 
                       - iv.start.ToDouble()) 
                       + iv.start.ToDouble());
        t.SetType(instanttype);
        uBool.timeInterval = iv;
        uBool.timeInterval.rc = false;
        uBool.timeInterval.end = t;
        uBool.constValue.Set(true, op == 0 ? false : true);
        result.MergeAdd( uBool );
        uBool.timeInterval.rc = true;
        uBool.timeInterval.start = t;
        uBool.timeInterval.lc = true;
        uBool.constValue.Set(true, op == 0 ? true : false);
        result.MergeAdd( uBool );
        uBool.timeInterval.lc = false;
        uBool.timeInterval.rc = iv.rc;
        uBool.timeInterval.end = iv.end;
        uBool.constValue.Set(true, op == 0 ? false : true);
        result.MergeAdd( uBool );
      }
      else {
        if(TLA_DEBUG)
          cout<<"no crossing -> no equal"<<endl;
        uBool.timeInterval = iv;
        uBool.constValue.Set(true, op == 0 ? false : true);
        result.MergeAdd( uBool );
      }
    }
  }
  result.EndBulkLoad( false );
}

double PointInMPoint(double startx, double starty, double endx,
                     double endy, double cmpx, double cmpy){
/*
Function PointInMPoint checks whether Point (cmpx cmpy) lies inside
the given MPoint. Ir returns -1.0 if mpoint does not cross cmp and 
2.0 if mpoint stays on comp (does not move). In all other cases it 
return thr fraction of mpoint when mpoint is over cmp (0.0 .. 1.0).

*/
  
  if(AlmostEqual(startx, endx) && AlmostEqual(starty, endy)){
    if(AlmostEqual(startx, cmpx) && AlmostEqual(starty, cmpy))
      return 2.0;
    else
      return -1.0;
  }
  
  if(AlmostEqual(startx, cmpx) && AlmostEqual(starty, cmpy)){
    return 0.0;
  }
  else if(AlmostEqual(endx, cmpx) && AlmostEqual(endy, cmpy)){
    return 1.0;
  }
  else{
    bool vert = false;
    bool hor = false;
    double tx, ty;
    double dx = endx - startx;
    double dy = endy - starty;
    if (dx == 0.0)
      vert = true;
    else
      tx = (cmpx - startx) / dx;
    if (dy == 0.0)
      hor = true;
    else 
      ty = (cmpy - starty) / dy;
    if (hor) {
      if (AlmostEqual(starty, cmpy))
        ty = tx;
      else
        ty = -1.0;
    }
    else if (vert) {
      if (AlmostEqual(startx, cmpx))
        tx = ty;
      else
        tx = -1.0;
    }
    if(tx == ty && tx > 0.0 && tx < 1.0) {
      return tx;
    }
  }
  
  return -1.0;
}

/*
For Operators ~=~ and ~\#~ and MovingRegion/Region

*/

void MovingRegionCompareMS( MRegion *mr, CRegion *r, MBool *result,
 int op) 
{
  const URegion *ur;
  UBool uBool;   //Part of the result
  
  result->Clear();
  result->StartBulkLoad();
  cout<<"MovingRegionCompareMS called"<<endl;
  for(int i = 0; i < mr->GetNoComponents(); i++){
    mr->Get(i, ur);
    cout<<"URegion # "<<i<<" "<<"[ "
    <<ur->timeInterval.start.ToDouble()<<" "
    <<ur->timeInterval.end.ToDouble()<<" ]"<<endl;
    int number = ur->GetSegmentsNum();
    cout<<"number of segments = "<< number<<endl;
    
    bool staticequal = true;
    bool finish = false;
    const MSegmentData *dms;
    int i = 0;
    while(staticequal && (i < ur->GetSegmentsNum())){
      ur->GetSegment(i, dms);
      
      if(dms->GetInitialStartX() == dms->GetFinalStartX()
        && dms->GetInitialStartY() == dms->GetFinalStartY()
        && dms->GetInitialEndX() == dms->GetFinalEndX()
        && dms->GetInitialEndY() == dms->GetFinalEndY()){
        cout<<"s is static"<<endl;
        //find matching CHalfsegment set staticequal!!
        Point *p1 = new Point(true, 
             dms->GetFinalStartX(), dms->GetFinalStartY());
        Point *p2 = new Point(true, 
             dms->GetFinalEndX(), dms->GetFinalEndY());
        CHalfSegment *nHS = new CHalfSegment(true, true, *p1, *p2);
        delete p1;
        delete p2;
        const CHalfSegment *mid;
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
          cout<<"no matching Halfsegment -> Unit not equal!!"<<endl;
          staticequal = false;
          finish = true;
        }
        delete nHS;
      }
      else {
        cout<<"s is not static"<<endl;
        staticequal = false;
      }
      i++;
    }  
    if(staticequal){
      cout<<"all matching Halfsegments -> Unit is equal!!"<<endl;
      uBool.timeInterval = ur->timeInterval;
      uBool.constValue.Set(true, (op == 0) ? true : false);
      cout<<"uBool "<<uBool.constValue.GetBoolval()
      <<" ["<<uBool.timeInterval.start.ToDouble()
      <<" "<<uBool.timeInterval.end.ToDouble()<<" "
      <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;
      result->MergeAdd(uBool);
    }
    else if(finish){
      cout<<"a static s with no matching HS -> unit not equal!"
      <<endl;
      uBool.timeInterval = ur->timeInterval;
      uBool.constValue.Set(true, (op == 0) ? false : true);
      cout<<"uBool "<<uBool.constValue.GetBoolval()
      <<" ["<<uBool.timeInterval.start.ToDouble()
      <<" "<<uBool.timeInterval.end.ToDouble()<<" "
      <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;
      result->MergeAdd(uBool);
    }
    else{ //the complicate way with not static mregions
      const CHalfSegment *chs;
      Periods* period = new Periods(0);
      Periods* between = new Periods(0);
      Periods* pResult = new Periods(0);
      Interval<Instant> newper; //part of the result
      
      for(int i = 0 ;i < r->Size(); i++){
        r->Get(i, chs);   
        
        double tsd = PointInMPoint(dms->GetInitialStartX(),
          dms->GetInitialStartY(), dms->GetFinalStartX(),
          dms->GetFinalStartY(), chs->GetDPoint().GetX(),
          chs->GetDPoint().GetY());
        double ted = PointInMPoint(dms->GetInitialEndX(),
          dms->GetInitialEndY(), dms->GetFinalEndX(),
          dms->GetFinalEndY(), chs->GetDPoint().GetX(),
          chs->GetDPoint().GetY());
        double tss = PointInMPoint(dms->GetInitialStartX(),
          dms->GetInitialStartY(), dms->GetFinalStartX(),
          dms->GetFinalStartY(), chs->GetSPoint().GetX(),
          chs->GetSPoint().GetY());
        double tes = PointInMPoint(dms->GetInitialEndX(),
          dms->GetInitialEndY(), dms->GetFinalEndX(),
          dms->GetFinalEndY(), chs->GetSPoint().GetX(),
          chs->GetSPoint().GetY());
           
        double tpoint = -1.0;
        if(tsd >= 0.0 && tes >= 0.0){
          cout<<"start through dominant, end through subdominat"
          <<endl;
          if(AlmostEqual(tsd, tes))
            tpoint = tsd;
          if(AlmostEqual(tsd, 2.0))
            tpoint = tes;
          if(AlmostEqual(tes, 2.0))
            tpoint = tsd;
        }
        if(tss >= 0.0 && ted >= 0.0){
          cout<<"start through subdominant, end through dominat"
          <<endl;
          if(AlmostEqual(tss, ted))
            tpoint = ted;
          if(AlmostEqual(tss, 2.0))
            tpoint = ted;
          if(AlmostEqual(ted, 2.0))
            tpoint = tss;
        }
        if(tpoint >= 0.0){
          Instant t;
          t.ReadFrom((ur->timeInterval.end.ToDouble() 
          - ur->timeInterval.start.ToDouble()) * tpoint 
          + ur->timeInterval.start.ToDouble());
          t.SetType(instanttype);
          newper.start = t;
          newper.end = t;
          newper.lc = true;
          newper.rc = true;
      
          cout<<"newper ["<< newper.start.ToDouble()
          <<" "<<newper.end.ToDouble()<<" "<<newper.lc<<" "
          <<newper.rc<<"]"<<endl;
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
        CRegion snapshot;
        cout<<"add interval # "<<i<<endl;
        pResult->Get(i, per);
        ur->TemporalFunction(per->start, snapshot);
        if(*r == snapshot){
          cout<<"r == snapshot!"<<endl;
          if(per->start > ur->timeInterval.start){
            uBool.timeInterval.start = ur->timeInterval.start;
            uBool.timeInterval.lc = ur->timeInterval.lc;
            uBool.timeInterval.end = per->start;
            uBool.timeInterval.rc = !per->rc;
            uBool.constValue.Set(true, (op == 0) ? false : true);
            cout<<"uBool "<<uBool.constValue.GetBoolval()
            <<" ["<<uBool.timeInterval.start.ToDouble()
            <<" "<<uBool.timeInterval.end.ToDouble()<<" "
            <<uBool.timeInterval.lc<<" "
            <<uBool.timeInterval.rc<<"]"<<endl;
            result->MergeAdd(uBool);
          }
          uBool.timeInterval = *per;
          uBool.constValue.Set(true, (op == 0) ? true : false);
          cout<<"uBool "<<uBool.constValue.GetBoolval()
          <<" ["<<uBool.timeInterval.start.ToDouble()
          <<" "<<uBool.timeInterval.end.ToDouble()<<" "
          <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;
          result->MergeAdd(uBool);
          if(per->end < ur->timeInterval.end){
            uBool.timeInterval.start = per->end;
            uBool.timeInterval.lc = !per->lc;
            uBool.timeInterval.end = ur->timeInterval.end;
            uBool.timeInterval.rc = ur->timeInterval.rc;        
            uBool.constValue.Set(true, (op == 0) ? false : true);
            cout<<"uBool "<<uBool.constValue.GetBoolval()
            <<" ["<<uBool.timeInterval.start.ToDouble()
            <<" "<<uBool.timeInterval.end.ToDouble()<<" "
            <<uBool.timeInterval.lc<<" "
            <<uBool.timeInterval.rc<<"]"<<endl;
            result->MergeAdd(uBool);
          }
        }
        else
          cout<<"r != snapshot"<<endl;
      }
      delete pResult;
    }
  }
  result->EndBulkLoad(false);
}

/*
For Operators ~=~ and ~\#~ and MovingRegion/Region

*/

void MovingRegionCompareMM( MRegion *mr1, MRegion *mr2, MBool *result,
 int op)
{
  cout<<"MovingRegionCompareMM called"<<endl;
  RefinementPartitionLift<MRegion, MRegion, URegion, URegion> rp(*mr1, *mr2);
  cout<<"RefimentPartiion done with size "<<rp.Size()<<endl;
  Interval<Instant>* iv;
  int reg1Pos;
  int reg2Pos;
  UBool uBool;
  
  result->Clear();
  result->StartBulkLoad();
  
  for( unsigned int i = 0; i < rp.Size(); i++ ){
    rp.Get(i, iv, reg1Pos, reg2Pos);
    cout<<"interval # "<<i<<" "<<reg1Pos<<" "<<reg2Pos<<endl;
    if(reg1Pos == -1 || reg2Pos == -1)
      continue;
    cout<<"bothoperators in iv # "<<i<<" [ "
    <<iv->start.ToDouble()<<" "<<iv->end.ToDouble()<<" "<<iv->lc<<" "
    <<iv->rc<<" ] reg1Pos "<<reg1Pos<<", reg2Pos "<<reg2Pos <<endl;
    const URegion *ureg1;
    const URegion *ureg2;
    mr1->Get(reg1Pos, ureg1);
    mr2->Get(reg2Pos, ureg2);
    if(ureg1->GetSegmentsNum() != ureg1->GetSegmentsNum()){
      cout<<"uregions have different numbers of segments -> "
      <<"iv not equal"<<endl;
      uBool.timeInterval = *iv;
      uBool.constValue.Set(true, op == 0 ? false : true);
      cout<<"MergeAdd uBool "<<uBool.constValue.GetBoolval()
      <<" ["<<uBool.timeInterval.start.ToDouble()
      <<" "<<uBool.timeInterval.end.ToDouble()<<" "
      <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;
      result->MergeAdd(uBool);
      continue;
    }
    if((ureg1->GetSegmentsNum() == 0)&&(ureg1->GetSegmentsNum() == 0)){
      cout<<"both uregions have no segments -> iv  equal"<<endl;
      uBool.timeInterval = *iv;
      uBool.constValue.Set(true, op == 0 ? true : false);
      cout<<"MergeAdd uBool "<<uBool.constValue.GetBoolval()
      <<" ["<<uBool.timeInterval.start.ToDouble()
      <<" "<<uBool.timeInterval.end.ToDouble()<<" "
      <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;
      result->MergeAdd(uBool);
      continue;
    }
    //find possible times of equality
    const MSegmentData *dms1;
    Periods* period = new Periods(0);
    Periods* between = new Periods(0);
    Periods* pResult = new Periods(0);
    Interval<Instant> newper; //part of the result
    bool uregionPerhapsEqual = false;
    MSegmentData rdms1;
    
    ureg1->GetSegment(0, dms1);
    dms1->restrictToInterval(ureg1->timeInterval, *iv, rdms1);
    
    pResult->Clear();
    for(int n = 0; n < ureg2->GetSegmentsNum(); n++){  
      const MSegmentData *dms2; 
      MSegmentData rdms2;
      
      ureg2->GetSegment(n, dms2);
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
        cout<<"uregions are possibly equal"<<endl;
        uregionPerhapsEqual = true;  
      }
      else if(((ts >=0.0) && (ts <= 1.0) && AlmostEqual(ts, te))
        || (ts == 2.0 && (te >=0.0) && (te <= 1.0))
        || (te == 2.0 && (ts >=0.0) && (ts <= 1.0))){
        cout<<"equality found at t "<<ts<<" for dms "<<n<<endl;
        Instant t;
        t.ReadFrom((iv->end.ToDouble() 
          - iv->start.ToDouble()) * ts 
          + iv->start.ToDouble());
        t.SetType(instanttype);
        newper.start = t;
        newper.end = t;
        newper.lc = true;
        newper.rc = true;
        cout<<"newper ["<< newper.start.ToDouble()
        <<" "<<newper.end.ToDouble()<<" "<<newper.lc<<" "
        <<newper.rc<<"]"<<endl;
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
      test for eaquality of total uregion
      this can not be done at start and end of interval,
      because they are probably not inside the interval.
      So it is used the 10 and 90 percent time of the uregion.
    
      */

      CRegion snapshot1;
      CRegion snapshot2;
      cout<<"uregions are possibly equal. Create snapshots"<<endl;
      Instant time;
      time.ReadFrom(0.1  * (iv->end.ToDouble() 
                         - iv->start.ToDouble()) 
                         + iv->start.ToDouble());
      time.SetType(instanttype);
      ureg1->TemporalFunction(time, snapshot1);
      ureg2->TemporalFunction(time, snapshot2);
      if(snapshot1 == snapshot2){
        cout<<"snapshots of iv->start are equal"<<endl;
        time.ReadFrom(0.1  * (iv->end.ToDouble() 
                           - iv->start.ToDouble()) 
                           + iv->start.ToDouble());
        time.SetType(instanttype);
        ureg1->TemporalFunction(time, snapshot1);
        ureg2->TemporalFunction(time, snapshot2);
        if(snapshot1 == snapshot2){
          cout<<"snapshots of iv->end are equal, too."<<endl;
          uBool.timeInterval = *iv;
          uBool.constValue.Set(true, op == 0 ? true : false);
          cout<<"MergeAdd uBool "<<uBool.constValue.GetBoolval()
          <<" ["<<uBool.timeInterval.start.ToDouble()
          <<" "<<uBool.timeInterval.end.ToDouble()<<" "
          <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;
          result->MergeAdd(uBool);
          continue;
        }
        else
          cout<<"snapshots of iv->end are not equal,"
          <<" uregions are not equal"<<endl;
      }
      else
        cout<<"snapshots of iv->start are not equal,"
        <<" uregegions are not equal"<<endl;
    }
    
    const Interval<Instant> *per;
    bool finished = false;
    cout<<"number of times : "<<pResult->GetNoComponents()<<endl;
    for(int i = 0; i < pResult->GetNoComponents(); i++){
      CRegion snapshot1;
      CRegion snapshot2;
      pResult->Get(i, per);
      cout<<"test time # "<<i<<" "<<per->start.ToDouble()<<endl;
      if((per->start == ureg1->timeInterval.start 
          && !ureg1->timeInterval.lc)
        || (per->start == ureg2->timeInterval.start 
          && !ureg2->timeInterval.lc)
        || (per->start == ureg1->timeInterval.end 
          && !ureg1->timeInterval.rc)
        || (per->start == ureg2->timeInterval.end 
          && !ureg2->timeInterval.rc))
        //no snapshot possible, so this uregions can not be 
        //equal at this time!
        continue;
      ureg1->TemporalFunction(per->start, snapshot1);
      ureg2->TemporalFunction(per->start, snapshot2);
      if(snapshot1 == snapshot2){
        cout<<"snapshot equal!"<<endl;
        if(per->start > iv->start){
          uBool.timeInterval.start = iv->start;
          uBool.timeInterval.lc = iv->lc;
          uBool.timeInterval.end = per->start;
          uBool.timeInterval.rc = !per->rc;
          uBool.constValue.Set(true, (op == 0) ? false : true);
          cout<<"uBool "<<uBool.constValue.GetBoolval()
          <<" ["<<uBool.timeInterval.start.ToDouble()
          <<" "<<uBool.timeInterval.end.ToDouble()<<" "
          <<uBool.timeInterval.lc<<" "
          <<uBool.timeInterval.rc<<"]"<<endl;
          result->MergeAdd(uBool);
        }
        uBool.timeInterval = *per;
        uBool.constValue.Set(true, (op == 0) ? true : false);
        cout<<"uBool "<<uBool.constValue.GetBoolval()
        <<" ["<<uBool.timeInterval.start.ToDouble()
        <<" "<<uBool.timeInterval.end.ToDouble()<<" "
        <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<"]"<<endl;
        result->MergeAdd(uBool);
        if(per->end < iv->end){
          uBool.timeInterval.start = per->end;
          uBool.timeInterval.lc = !per->lc;
          uBool.timeInterval.end = iv->end;
          uBool.timeInterval.rc = iv->rc;        
          uBool.constValue.Set(true, (op == 0) ? false : true);
          cout<<"uBool "<<uBool.constValue.GetBoolval()
          <<" ["<<uBool.timeInterval.start.ToDouble()
          <<" "<<uBool.timeInterval.end.ToDouble()<<" "
          <<uBool.timeInterval.lc<<" "
          <<uBool.timeInterval.rc<<"]"<<endl;
          result->MergeAdd(uBool);
        }
        finished = true;
        break;
      }
      else
        cout<<"snapshot not equal"<<endl;
    }
    delete pResult;
    if(!finished){
      uBool.timeInterval = *iv;      
      uBool.constValue.Set(true, (op == 0) ? false : true);
      cout<<"uBool "<<uBool.constValue.GetBoolval()
      <<" ["<<uBool.timeInterval.start.ToDouble()
      <<" "<<uBool.timeInterval.end.ToDouble()<<" "
      <<uBool.timeInterval.lc<<" "
      <<uBool.timeInterval.rc<<"]"<<endl;
      result->MergeAdd(uBool);
    }   
  }
  result->EndBulkLoad(false);
}

static void MovingBoolMMOperators( MBool& op1, MBool& op2,
 MBool& result, int op )

/*
Compares the two operators in the given way: 
The comparisons are 1: AND; 2: OR.

*/

{
  UBool uBool;  //part of the Result
  
  RefinementPartitionLift<MBool, MBool, UBool, UBool> rp(op1, op2);
  cout<<"Refinement abgeschlossen, rp.size: "<<rp.Size()<<endl;
  result.Clear();
  result.StartBulkLoad();
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    Interval<Instant>* iv;
    int u1Pos;
    int u2Pos;
    const UBool *u1transfer;
    const UBool *u2transfer;
    UBool u1;
    UBool u2;
    
    rp.Get(i, iv, u1Pos, u2Pos);
  cout<< "and/or interval #"<< i<< ": "<< iv->start.ToDouble()
  << " "
  << iv->end.ToDouble()<< " "<< iv->lc<< " "<< iv->rc<< " "
  << u1Pos<< " "<< u2Pos<< endl;
    
    if (u1Pos == -1 || u2Pos == -1)     
      continue;  
    else {
  cout<<"Both operators existant in interval iv #"<<i<<endl;
      op1.Get(u1Pos, u1transfer);
      op2.Get(u2Pos, u2transfer);
      u1 = *u1transfer;
      u2 = *u2transfer;
    }
    
  cout<<"wert 1 "<<&u1.constValue<<endl;
  cout<<"wert 2 "<<&u2.constValue<<endl;
    uBool.timeInterval = *iv;
    
    if (op == 1)//AND
      uBool.constValue.Set(true,u1.constValue.GetBoolval() 
      and u2.constValue.GetBoolval());
    else if (op == 2) //OR
      uBool.constValue.Set(true,u1.constValue.GetBoolval() 
      or u2.constValue.GetBoolval());
    else //should not happen!
      uBool.constValue.Set(true,true);
    cout<<"wert "<<&uBool.constValue<<endl;
    cout<<"interval "<<uBool.timeInterval.start.ToDouble()
    <<" "<<uBool.timeInterval.end.ToDouble()<<" "
    <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<endl;
    
    result.MergeAdd(uBool);
  }
  result.EndBulkLoad(false);
}

static void MovingBoolMSOperators( MBool& op1, CcBool& op2,
 MBool& result, int op )

/*
Compares the two operators in the given way: 
The comparisons are 1: AND; 2: OR.

*/

{

  UBool uBool;  //part of the Result
  const UBool *u1transfer;
  
  result.Clear();
  result.StartBulkLoad();
  for( int i = 0; i < op1.GetNoComponents(); i++)
  {
cout<<"temporalMSLogic "<<op<<" ,# "<<i<<endl;
    op1.Get(i, u1transfer);
    uBool = *u1transfer;
    if (op == 1)
      uBool.constValue.Set(uBool.constValue.IsDefined(),
      (uBool.constValue.GetBoolval() and op2.GetBoolval()));
    else
      uBool.constValue.Set(uBool.constValue.IsDefined(),
      (uBool.constValue.GetBoolval() or op2.GetBoolval()));
  cout<<"wert "<<&uBool.constValue<<endl;
  cout<<"interval "<<uBool.timeInterval.start.ToDouble()
  <<" "<<uBool.timeInterval.end.ToDouble()<<" "
  <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<endl;
  
      result.MergeAdd(uBool);
  }
  result.EndBulkLoad(false);
}

template <class Mapping1, class Mapping2, class Unit1, class
 Unit2>
static void MovingCompareBoolMM( Mapping1& op1, Mapping2& op2,
 MBool& result, int op )

/*
Compares the two operators in the given way: The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/

{
  UBool uBool;  //part of the Result
  
  RefinementPartitionLift<Mapping1, Mapping2, Unit1, Unit2> 
  rp(op1, op2);
 cout<<"Refinement abgeschlossen, rp.size: "<<rp.Size()<<endl;
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
  cout<< "Compare interval #"<< i<< ": "<< iv->start.ToDouble()
  << " "
  << iv->end.ToDouble()<< " "<< iv->lc<< " "<< iv->rc<< " "
  << u1Pos<< " "<< u2Pos<< endl;
    
    if (u1Pos == -1 || u2Pos == -1) 
      continue;  
    else {
  cout<<"Both operators existant in interval iv #"<<i<<endl;
      op1.Get(u1Pos, u1transfer);
      op2.Get(u2Pos, u2transfer);
      u1 = *u1transfer;
      u2 = *u2transfer;
    }
  cout<<"wert 1 "<<&u1.constValue<<endl;
  cout<<"wert 2 "<<&u2.constValue<<endl;
    uBool.timeInterval = *iv;

    uBool.constValue.Set(true,CompareValue(u1,u2,op));   
     
cout<<"wert "<<&uBool.constValue<<endl;
cout<<"interval "<<uBool.timeInterval.start.ToDouble()
<<" "<<uBool.timeInterval.end.ToDouble()<<" "
<<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<endl;  
    
    result.MergeAdd(uBool);
      
  }
  result.EndBulkLoad(false);

}

template <class Mapping1, class Mapping2, class Unit1, 
class Unit2>
static void MovingIntersectionMM( Mapping1& op1, Mapping2& op2,
 Mapping1& result, int op)

/*
Calculates the Intersection 

*/

{
  Unit1 un;  //part of the Result
  RefinementPartitionLift<Mapping1, Mapping2, Unit1, Unit2> 
  rp(op1, op2);
 cout<<"Refinement abgeschlossen, rp.size: "<<rp.Size()<<endl;
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
  cout<< "Intersect interval #"<< i<< ": "
  << iv->start.ToDouble()<< " "
  << iv->end.ToDouble()<< " "<< iv->lc<< " "
  << iv->rc<< " "<< u1Pos<< " "<< u2Pos<< endl;
    
    if (u1Pos == -1 || (u2Pos == -1 && op != 2))
      continue;  
    else if (u2Pos == -1) {
      cout<<"only 1. operator existant in interval iv #"<<i<<endl;
      op1.Get(u1Pos, u1transfer);
      u1 = *u1transfer;
      un.constValue = u1.constValue;
      un.timeInterval = *iv;
      result.MergeAdd(un);
    }
    else {
      cout<<"Both operators existant in interval iv #"<<i<<endl;
      op1.Get(u1Pos, u1transfer);
      op2.Get(u2Pos, u2transfer);
      u1 = *u1transfer;
      u2 = *u2transfer;
      cout<<"wert 1 "<<&u1.constValue<<endl;
      cout<<"wert 2 "<<&u2.constValue<<endl;

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


template <class Mapping1, class Unit1, class Operator2>
static void MovingCompareBoolMS( Mapping1& op1, Operator2& op2,
 MBool& result, int op )

/*
Compares the two operators in the given way: 
The comparisons are -3: \#; -2: <; -1: <=; 0: =; 1: >=; 2: >.

*/

{
  UBool uBool;  //part of the Result
  Unit1 u1;
  const Unit1 *u1transfer;

  result.Clear();
  result.StartBulkLoad();
  for(int i = 0; i < op1.GetNoComponents(); i++)
  {
cout<<"MovingCompareBoolMS "<<op<<" ,# "<<i<<endl;
     
     op1.Get(i, u1transfer);
     u1 = *u1transfer;
     uBool.timeInterval = u1.timeInterval;

  cout<<"wert 1 "<<&u1.constValue<<endl;
  cout<<"wert 2 "<<&op2<<endl;

    uBool.constValue.Set(true,CompareValue(u1,op2,op));

 cout<<"erg "<<&uBool.constValue<<endl;
  cout<<"interval "<<uBool.timeInterval.start.ToDouble()
  <<" "<<uBool.timeInterval.end.ToDouble()<<" "
  <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<endl;
  
     result.MergeAdd(uBool);
  }
  result.EndBulkLoad(false);
}

/*
10.1.16 Type mapping function "MBoolTypeMapMBool"

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
10.1.16 Type mapping function "AndOrTypeMapMBool"

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
    and nl->IsEqual( arg2, "mbool" ) )
      return (nl->SymbolAtom( "mbool" ));
      
    if( nl->IsEqual( arg1, "mbool" ) 
    and nl->IsEqual( arg2, "bool" ) )
      return (nl->SymbolAtom( "mbool" ));
      
    if( nl->IsEqual( arg1, "bool" ) 
    and nl->IsEqual( arg2, "mbool" ) )
      return (nl->SymbolAtom( "mbool" ));
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
10.1.17 Type mapping function "MovingEqualTypeMapMBool"

This type mapping function is used for the ~meq~ and 
~mne~ operator.

*/

ListExpr MovingEqualTypeMapMBool( ListExpr args )
{
  ListExpr arg1, arg2;
  cout<<"MovingEqualTypeMapMBool called"<<endl;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, "mbool" ) 
    and nl->IsEqual( arg2, "mbool" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mbool" ) 
    and nl->IsEqual( arg2, "bool" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "bool" ) 
    and nl->IsEqual( arg2, "mbool" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mint" ) 
    and nl->IsEqual( arg2, "mint" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mint" ) 
    and nl->IsEqual( arg2, "int" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "int" ) 
    and nl->IsEqual( arg2, "mint" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mstring" ) 
    and nl->IsEqual( arg2, "mstring" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mstring" ) 
    and nl->IsEqual( arg2, "string" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "string" ) 
    and nl->IsEqual( arg2, "mstring" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mreal" ) 
    and nl->IsEqual( arg2, "mreal" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mreal" ) 
    and nl->IsEqual( arg2, "real" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "real" ) 
    and nl->IsEqual( arg2, "mreal" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mpoint" ) 
    and nl->IsEqual( arg2, "mpoint" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mpoint" ) 
    and nl->IsEqual( arg2, "point" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "point" ) 
    and nl->IsEqual( arg2, "mpoint" ) )
      return (nl->SymbolAtom( "mbool" ));
   
    if( nl->IsEqual( arg1, "movingregion" ) 
    and nl->IsEqual( arg2, "region" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "region" ) 
    and nl->IsEqual( arg2, "movingregion" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "movingregion" ) 
    and nl->IsEqual( arg2, "movingregion" ) )
      return (nl->SymbolAtom( "mbool" ));

  }
  return nl->SymbolAtom( "typeerror" );
}


/*
10.1.18 Type mapping function "MovingCompareTypeMapMBool"

This type mapping function is used for the ~<~, ~<=~, ~<~ 
and ~>=~ operator.

*/

ListExpr MovingCompareTypeMapMBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, "mbool" )
    and nl->IsEqual( arg2, "mbool" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mbool" ) 
    and nl->IsEqual( arg2, "bool" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "bool" ) 
    and nl->IsEqual( arg2, "mbool" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mint" ) 
    and nl->IsEqual( arg2, "mint" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mint" ) 
    and nl->IsEqual( arg2, "int" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "int" ) 
    and nl->IsEqual( arg2, "mint" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mreal" ) 
    and nl->IsEqual( arg2, "mreal" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mreal" ) 
    and nl->IsEqual( arg2, "real" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "real" ) 
    and nl->IsEqual( arg2, "mreal" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mstring" ) 
    and nl->IsEqual( arg2, "mstring" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "mstring" ) 
    and nl->IsEqual( arg2, "string" ) )
      return (nl->SymbolAtom( "mbool" ));
    if( nl->IsEqual( arg1, "string" ) 
    and nl->IsEqual( arg2, "mstring" ) )
      return (nl->SymbolAtom( "mbool" ));
      
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.11 Type mapping function ~MovingTypeMapeIntime~

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
      
   //if( nl->IsEqual( arg1, "mpoint" ) 
   //&& nl->IsEqual( arg2, "point" ) )
   //   return nl->SymbolAtom( "mreal" );
   // if( nl->IsEqual( arg1, "point" ) 
   // && nl->IsEqual( arg2, "mpoint" ) )
   //   return nl->SymbolAtom( "mreal" );
      
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
16.1.11 Type mapping function ~MovingIntersectionTypeMap~
It is for the operator ~intersection~.

*/

ListExpr
MovingIntersectionTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );
    cout<<"MovingIntersectionTypeMap called"<<endl;
    
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
16.1.11 Type mapping function ~MovingMinusTypeMap~
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
16.1.3 Type mapping function ~InsideTypeMapMBool~

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
      
      if(nl->IsEqual(nl->First(args), "movingregion") 
         && nl->IsEqual(nl->Second(args), "points"))
         return nl->SymbolAtom("mbool");
         
      if(nl->IsEqual(nl->First(args), "movingregion") 
         && nl->IsEqual(nl->Second(args), "line"))
         return nl->SymbolAtom("mbool");
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.3 Type mapping function ~PerimeterTypeMap~

Used by ~perimeter~ and ~area~

*/
static ListExpr PerimeterTypeMap(ListExpr args) {
    cout << "PerimeterTypeMap() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->IsEqual(nl->First(args), "movingregion"))
        return nl->SymbolAtom("mreal");
    else
        return nl->SymbolAtom("typeerror");
}

/*
16.1.3 Type mapping function ~RCenterTypeMap~

Used by ~rough\_center~

*/
static ListExpr RCenterTypeMap(ListExpr args) {
    cout<< "RCenterTypeMap() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->IsEqual(nl->First(args), "movingregion"))
        return nl->SymbolAtom("mpoint");
    else
        return nl->SymbolAtom("typeerror");
}

/*
16.1.3 Type mapping function ~NComponentsTypeMap~

Used by ~no\_components~

*/
static ListExpr NComponentsTypeMap(ListExpr args) {
    cout<< "NComponentsTypeMap() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->IsEqual(nl->First(args), "movingregion"))
        return nl->SymbolAtom("mint");
    else
        return nl->SymbolAtom("typeerror");
}



/*
16.1.3 Type mapping function ~UnionTypeMap~

Used by ~union~:

*/
static ListExpr UnionTypeMap(ListExpr args) {
    cout<< "UnionTypeMap() called" << endl;

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
Used by ~isempty~:

16.1.3 Type mapping function ~TemporalLiftIsemptyTypeMap~

*/
static ListExpr TemporalLiftIsemptyTypeMap(ListExpr args) {
    cout<<"TemporalLiftIsemptyTypeMap() called" << endl;

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
Used by ~mint~:

16.1.3 Type mapping function ~TemporalMIntTypeMap~

*/
static ListExpr TemporalMIntTypeMap(ListExpr args) {
    cout<<"TemporalMIntTypeMap() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->IsEqual(nl->First(args), "periods"))
        return nl->SymbolAtom("mint");
    else
        return nl->SymbolAtom("typeerror");
}

/*
Used by ~+~:

16.1.3 Type mapping function ~TemporalPlusTypeMap~

*/
static ListExpr TemporalPlusTypeMap(ListExpr args) {
    cout<<"TemporalPlusTypeMap() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "mint")
        && nl->IsEqual(nl->Second(args), "mint"))
        return nl->SymbolAtom("mint");
    else
        return nl->SymbolAtom("typeerror");
}

/*
Used by ~zero~:

16.1.3 Type mapping function ~TemporalZeroTypeMap~

*/
static ListExpr TemporalZeroTypeMap(ListExpr args) {
    cout<<"TemporalZeroTypeMap() called" << endl;

    if (nl->ListLength(args) == 0)
        return nl->SymbolAtom("mint");
    else
        return nl->SymbolAtom("typeerror");
}

/*
Used by ~concat~:

16.1.3 Type mapping function ~TemporalConcatTypeMap~

*/
static ListExpr TemporalConcatTypeMap(ListExpr args) {
    cout<<"TemporalConcatTypeMap() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "mpoint")
        && nl->IsEqual(nl->Second(args), "mpoint") )
        return nl->SymbolAtom("mpoint");
    else
        return nl->SymbolAtom("typeerror");
}






/*
16.2.12 Selection function ~MovingAndOrSelect~

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
16.2.11 Selection function ~MovingEqualSelect~

Is used for the ~mequal~ and ~mnotequal~
operations.

*/

int
MovingEqualSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );
           cout<<"MovingEqualSelect called"<<endl;

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
16.2.12 Selection function ~MovingCompareSelect~

Is used for the ~mequal~ and ~mnotequal~
operations.

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
16.2.12 Selection function ~MovingDistanceSelect~

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
    

  //if( nl->SymbolValue( arg1 ) == "mpoint" 
  //&& nl->SymbolValue( arg2 ) == "point" )
  //  return 1;
  //if( nl->SymbolValue( arg1 ) == "point" 
  //&& nl->SymbolValue( arg2 ) == "mpoint" )
  //  return 2;
    


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
16.2.1 Selection function ~MovingIntersectionSelect~
Is used for the ~intersect~ operation.

*/

int MovingIntersectionSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );
           
  cout<<"MovingIntersectionSelect called"<<endl; 

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
    
  return -1; // This point should never be reached
}

/*
16.2.1 Selection function ~MovingMinusSelect~
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
16.2.1 Selection function ~InsideSelect~

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
16.2.1 Selection function ~UnionSelect~

For ~union~:

*/

static int UnionSelect(ListExpr args) {
    cout<< "UnionSelect() called" << endl;

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
16.2.1 Selection function ~TemporalLiftIsemptySelect~

For ~isempty~:

*/

static int TemporalLiftIsemptySelect(ListExpr args) {
    cout<< "TemporalLiftIsemptySelect() called" << endl;

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









//js

/*
16.3.29 Value mapping functions of operator ~distance~

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
16.3.39 Value mapping functions of operator ~not~

*/

int TemporalNot( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  MBool* pResult = (MBool*)result.addr;
  MBool* op = (MBool*)args[0].addr;
  const UBool *u1transfer;
  UBool uBool;
  
  pResult->Clear();
  pResult->StartBulkLoad();
  for( int i = 0; i < op->GetNoComponents(); i++)
  {
  cout<<"temporalNot "<<i<<endl;
    op->Get(i, u1transfer);
    uBool = *u1transfer;
  cout<<"wert "<<&uBool.constValue<<endl;
    uBool.constValue.Set(uBool.constValue.IsDefined(),
    !(uBool.constValue.GetBoolval()));
  cout<<"wert "<<&uBool.constValue<<endl;
  cout<<"interval "<< uBool.timeInterval.start.ToDouble()
  <<" "<<uBool.timeInterval.end.ToDouble()<<" "
  <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<endl;
    pResult->Add(uBool);
  }
  pResult->EndBulkLoad(false);
  
  return 0;
}

/*
16.3.40 Value mapping functions of operator ~and~ and 
~or~ moving/moving
op == 1 -> AND, op == 2 -> OR

*/

template<int op>
int TemporalMMLogic( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  
  MovingBoolMMOperators( *((MBool*)args[0].addr),
   *((MBool*)args[1].addr), *((MBool*)result.addr), op);

  return 0;
}

/*
16.3.42 Value mapping functions of operators ~and~ 
and ~or~ moving/static
op == 1 -> AND, po == 2 -> OR

*/

template<int op>
int TemporalMSLogic( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingBoolMSOperators( *((MBool*)args[0].addr),
   *((CcBool*)args[1].addr), *((MBool*)result.addr), op);
  
  return 0;
}

/*
16.3.42 Value mapping functions of operators ~and~ 
and ~or~ static/moving
op == 1 -> AND, po == 2 -> OR

*/

template<int op>
int TemporalSMLogic( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );

  MovingBoolMSOperators( *((MBool*)args[1].addr),
   *((CcBool*)args[0].addr), *((MBool*)result.addr), op);
  
  return 0;
}


/*
16.3.44 Value mapping functions of operator ~mequal~ 
and ~notmequal~

*/

template<class Mapping1, class Mapping2, class Unit1, class
 Unit2, int op>
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
16.3.45 Value mapping functions of operator ~intsersection~

*/

template<class Mapping1, class Mapping2, class Unit1, class
 Unit2, int op>
int TemporalMMIntersection( Word* args, Word& result, 
int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  
  MovingIntersectionMM<Mapping1, Mapping2, Unit1, Unit2>
  ( *((Mapping1*)args[0].addr), *((Mapping2*)args[1].addr),
   *((Mapping1*)result.addr), op);
  
  return 0;
}

template<int op>
int TemporalMMRealIntercept( Word* args, Word& result, int
 message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  
  MovingRealIntersectionMM( *((MReal*)args[0].addr),
   *((MReal*)args[1].addr), *((MReal*)result.addr), op);
  
  return 0;
}

template<class Mapping1, class Unit1, class Operator2, int op>
int TemporalMSIntersection( Word* args, Word& result, 
int message, Word& local, Supplier s )
{
  cout<<"TemporalMSIntersection called"<<endl;
  result = qp->ResultStorage( s );
  Mapping1 *mop1;
  Mapping1 *mop2 = new Mapping1(0); 
  const Unit1 *up1;
  mop1 = (Mapping1*)args[0].addr;
  Operator2 *constop = (Operator2*)args[1].addr;
  
  mop2->Clear();
  mop2->StartBulkLoad();
  for (int i = 0; i < mop1->GetNoComponents(); i++) {
    mop1->Get(i, up1);
    Unit1 *up2 = new Unit1(up1->timeInterval, *constop);
    cout<<"up2["<<i<<"] ["<<up2->timeInterval.start.ToDouble()
    <<" "<<up2->timeInterval.end.ToDouble()<<" "
    <<up2->timeInterval.lc<<" "<<up2->timeInterval.rc<<"] "
    <<" value: "<<&up2->constValue<<endl;
    mop2->Add(*up2);
    delete up2;

  }
  mop2->EndBulkLoad(false);
  MovingIntersectionMM<Mapping1, Mapping1, Unit1, Unit1>
  ( *mop1, *mop2, *((Mapping1*)result.addr), op);
  
  delete mop2;
  
  return 0;
}

template<int op>
int TemporalMSRealIntercept( Word* args, Word& result, 
int message, Word& local, Supplier s )
{
  cout<<"TemporalMSRealIntercept called"<<endl;
  result = qp->ResultStorage( s );
  MReal *mop1;
  MReal *mop2= new MReal(0); 
  const UReal *up1;
  mop1 = (MReal*)args[0].addr;
  CcReal *constop = (CcReal*)args[1].addr;
  
  mop2->Clear();
  mop2->StartBulkLoad();
  for (int i = 0; i < mop1->GetNoComponents(); i++) {
    mop1->Get(i, up1);
    UReal *up2 = new UReal(up1->timeInterval, 0.0, 0.0, (up1->r) ?
       pow(constop->GetRealval(), 2) : constop->GetRealval(),up1->r);

    cout<<"up2["<<i<<"] ["<<up2->timeInterval.start.ToDouble()
    <<" "<<up2->timeInterval.end.ToDouble()<<" "
    <<up2->timeInterval.lc<<" "<<up2->timeInterval.rc<<"] "<<endl;
    mop2->Add(*up2);
    delete up2;
  }
  mop2->EndBulkLoad(false);
  MovingRealIntersectionMM( *mop1, *mop2, *((MReal*)result.addr),
   op);
  
  delete mop2; 
   
  return 0;
}

template<class Mapping1, class Unit1, class Operator1, int op>
int TemporalSMIntersection( Word* args, Word& result, 
int message, Word& local, Supplier s )
{
  cout<<"TemporalSMIntersection called"<<endl;
  result = qp->ResultStorage( s );
  Mapping1 *mop1 = new Mapping1(0);
  Mapping1 *mop2; 
  const Unit1 *up2;
  mop2 = (Mapping1*)args[1].addr;
  Operator1 *constop = (Operator1*)args[0].addr;
  
  mop1->Clear();
  mop1->StartBulkLoad();
  for (int i = 0; i < mop2->GetNoComponents(); i++) {
    mop2->Get(i, up2);
    Unit1 *up1 = new Unit1(up2->timeInterval, *constop);
    cout<<"up1["<<i<<"] ["<<up1->timeInterval.start.ToDouble()
    <<" "<<up1->timeInterval.end.ToDouble()<<" "
    <<up1->timeInterval.lc<<" "<<up1->timeInterval.rc<<"] "
    <<" value: "<<&up1->constValue<<endl;
    mop1->Add(*up1);
    delete up1;

  }
  mop1->EndBulkLoad(false);
  MovingIntersectionMM<Mapping1, Mapping1, Unit1, Unit1>
  ( *mop1, *mop2, *((Mapping1*)result.addr), op);
  
  delete mop1;
  
  return 0;
}

template<int op>
int TemporalSMRealIntercept( Word* args, Word& result, 
int message, Word& local, Supplier s )
{
  cout<<"TemporalSMRealIntercept called"<<endl;
  result = qp->ResultStorage( s );
  MReal *mop1 = new MReal(0);
  MReal *mop2; 
  const UReal *up2;
  mop2 = (MReal*)args[1].addr;
  CcReal *constop = (CcReal*)args[0].addr;
  
  mop1->Clear();
  mop1->StartBulkLoad();
  for (int i = 0; i < mop2->GetNoComponents(); i++) {
    mop2->Get(i, up2);
    
    UReal *up1 = new UReal(up2->timeInterval, 0.0, 0.0, (up2->r) ?
      pow(constop->GetRealval(), 2) : constop->GetRealval(), up2->r);

    cout<<"up1["<<i<<"] ["<<up1->timeInterval.start.ToDouble()
    <<" "<<up1->timeInterval.end.ToDouble()<<" "
    <<up1->timeInterval.lc<<" "<<up1->timeInterval.rc<<"] "<<endl;
    mop1->Add(*up1);
    delete up1;
  }
  mop1->EndBulkLoad(false);
  MovingRealIntersectionMM( *mop1, *mop2, *((MReal*)result.addr),
   op);
   
  delete mop1;
  
  return 0;
}

/*
Function TransformMBool2MPoint completes a MBool to 
a MPoint-value. For this it adds the starting and end points.

*/

static void TransformMBool2MPoint(MPoint *mp, MBool *mBool,
  MPoint *endResult)
{
  const UPoint *up;
  
  endResult->Clear();
  endResult->StartBulkLoad();
  const UBool *ub;
  UPoint newUp;
  Point pt;
  int m = 0;
  bool pfinished = false;
  cout<<"TransformMBool2MPoint1 called"<<endl;
  for ( int i = 0; i < mp->GetNoComponents(); i++) {
    mp->Get(i, up);
    cout<<"UPoint # "<<i<<" ["<<up->timeInterval.start.ToDouble()
    <<" "<<up->timeInterval.end.ToDouble()<<" "
    <<up->timeInterval.lc<<" "<<up->timeInterval.rc<<"] ("
    <<up->p0.GetX()<<" "<<up->p0.GetY()<<")->("<<up->p1.GetX()
    <<" "<<up->p1.GetY()<<")"<<endl;
    while(m < mBool->GetNoComponents()){
      mBool->Get(m, ub);
      cout<<"ub "<<m<<" ["<<ub->timeInterval.start.ToDouble()
      <<" "<<ub->timeInterval.end.ToDouble()<<" "
      <<ub->timeInterval.lc<<" "<<ub->timeInterval.rc<<"] "
      <<ub->constValue.GetBoolval()<<endl;
      if(ub->constValue.GetBoolval()){
        cout<<"ub is ok!"<<endl;
        break;
      }
      else{
        cout<<"ub is not ok, next ub"<<endl;
        m++;
      }
    }
    if(m >= mBool->GetNoComponents()){
      cout<<"no ub any more"<<endl;
      pfinished = true;
    }
    if(pfinished || (ub->timeInterval.start > up->timeInterval.end
      || (ub->timeInterval.start == up->timeInterval.end 
      && !(ub->timeInterval.lc && up->timeInterval.rc)))){
      cout<<"ub is not inside up, take whole up"<<endl;
      newUp = *up;
      cout<<"Add1 ("
      <<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
      <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
      <<") ["<<newUp.timeInterval.start.ToDouble()<<" "
      <<newUp.timeInterval.end.ToDouble()<<" "
      <<newUp.timeInterval.lc<<" "
      <<newUp.timeInterval.rc<<"]"<<endl;
      if(newUp.timeInterval.end > newUp.timeInterval.start
        || (newUp.timeInterval.end == newUp.timeInterval.start
        && newUp.timeInterval.lc && newUp.timeInterval.rc))
        endResult->Add(newUp);
    }
    else{
      if(ub->timeInterval.start > up->timeInterval.start 
        || (ub->timeInterval.start == up->timeInterval.start
        && !ub->timeInterval.lc && up->timeInterval.lc)){
        cout<<"ub starts after up.start, but inside up"<<endl;
        newUp.p0 = up->p0;
        newUp.timeInterval.start = up->timeInterval.start;
        newUp.timeInterval.lc = up->timeInterval.lc;
        newUp.timeInterval.end = ub->timeInterval.start;
        newUp.timeInterval.rc = !ub->timeInterval.lc;
        up->TemporalFunction(newUp.timeInterval.end, pt);
        newUp.p1 = pt;
        cout<<"Add2 ("
        <<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
        <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
        <<") ["<<newUp.timeInterval.start.ToDouble()<<" "
        <<newUp.timeInterval.end.ToDouble()<<" "
        <<newUp.timeInterval.lc<<" "
        <<newUp.timeInterval.rc<<"]"<<endl;
        if(newUp.timeInterval.end > newUp.timeInterval.start
          || (newUp.timeInterval.end == newUp.timeInterval.start
          && newUp.timeInterval.lc && newUp.timeInterval.rc))
          endResult->Add(newUp);
      }
      if(ub->timeInterval.end < up->timeInterval.end
        || (ub->timeInterval.end == up->timeInterval.end
        && !ub->timeInterval.rc && up->timeInterval.rc)){
        cout<<"ub ends before up.end "<<endl;
        newUp.timeInterval.start = ub->timeInterval.end;
        newUp.timeInterval.lc = !ub->timeInterval.rc;
        up->TemporalFunction(newUp.timeInterval.start, pt);
        newUp.p0 = pt;  //1:
        bool loopfinished = false;
        while(!loopfinished){
          while(m < mBool->GetNoComponents()){
            mBool->Get(m, ub);
            cout<<"ub "<<m<<" ["<<ub->timeInterval.start.ToDouble()
            <<" "<<ub->timeInterval.end.ToDouble()<<" "
            <<ub->timeInterval.lc<<" "<<ub->timeInterval.rc<<"] "
            <<ub->constValue.GetBoolval()<<endl;
            if(ub->constValue.GetBoolval()){
              cout<<"ub is ok!"<<endl;
              break;
            }
            else{
              cout<<"ub is not ok, next ub"<<endl;
              m++;
            }
          }
          if(m >= mBool->GetNoComponents()){
            cout<<"no ub any more"<<endl;
            pfinished = true;
          }
          if(!pfinished
            && (ub->timeInterval.start < up->timeInterval.end
            || (ub->timeInterval.start == up->timeInterval.end
            && !ub->timeInterval.rc && up->timeInterval.rc))){
            cout<<"new ub starts before up.end "<<endl;
            newUp.timeInterval.end = up->timeInterval.start;
            newUp.timeInterval.rc = !up->timeInterval.lc;
            up->TemporalFunction(newUp.timeInterval.end, pt);
            newUp.p1 = pt;
            cout<<"Add3 ("
            <<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
            <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
            <<") ["<<newUp.timeInterval.start.ToDouble()<<" "
            <<newUp.timeInterval.end.ToDouble()<<" "
            <<newUp.timeInterval.lc<<" "
            <<newUp.timeInterval.rc<<"]"<<endl;
            if(newUp.timeInterval.end > newUp.timeInterval.start
            || (newUp.timeInterval.end == newUp.timeInterval.start
            && newUp.timeInterval.lc && newUp.timeInterval.rc))
              endResult->Add(newUp);
            newUp.timeInterval.start = ub->timeInterval.end;
            newUp.timeInterval.lc = !ub->timeInterval.rc;
            up->TemporalFunction(newUp.timeInterval.start, pt);
            newUp.p0 = pt;
            loopfinished = false;
            m++;
          }//goto 1
          else {
            cout<<"new ub starts after up.end, finish up"<<endl;
            newUp.timeInterval.end = up->timeInterval.end;
            newUp.timeInterval.rc = up->timeInterval.rc;
            newUp.p1 = up->p1;
            cout<<"Add4 ("
            <<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
            <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
            <<") ["<<newUp.timeInterval.start.ToDouble()<<" "
            <<newUp.timeInterval.end.ToDouble()<<" "
            <<newUp.timeInterval.lc<<" "
            <<newUp.timeInterval.rc<<"]"<<endl;
            if(newUp.timeInterval.end > newUp.timeInterval.start
            || (newUp.timeInterval.end == newUp.timeInterval.start
            && newUp.timeInterval.lc && newUp.timeInterval.rc))
              endResult->Add(newUp);
            loopfinished = true;
          }
        }
      }
    }
  }
  endResult->EndBulkLoad(false);
}

/*
Function TransformMBool2MPoint completes a MBool to 
a MPoint-value. For this it adds the starting and end points from point.

*/

static void TransformMBool2MPoint(Point *p, MBool *mBool,
  MPoint *endResult)
{
  
  endResult->Clear();
  endResult->StartBulkLoad();
  const UBool *ub;
  UPoint newUp;
  
  cout<<"TransformMBool2MPoint2 called"<<endl;
  for ( int i = 0; i < mBool->GetNoComponents(); i++) {
    mBool->Get(i, ub);
    cout<<"UBool # "<<i<<" ["<<ub->timeInterval.start.ToDouble()
    <<" "<<ub->timeInterval.end.ToDouble()<<" "
    <<ub->timeInterval.lc<<" "<<ub->timeInterval.rc<<"] "
    <<ub->constValue.GetBoolval()<<endl;
    if(ub->constValue.GetBoolval()){
     cout<<"point and mpoint are equal ignore timeInterval"<<endl;
    }
    else{
      cout<<"point and mpoint are equal take timeInterval"<<endl;
      newUp.timeInterval = ub->timeInterval;
      newUp.p0 = *p;
      newUp.p1 = *p;
      cout<<"Add4 ("
      <<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
      <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
      <<") ["<<newUp.timeInterval.start.ToDouble()<<" "
      <<newUp.timeInterval.end.ToDouble()<<" "
      <<newUp.timeInterval.lc<<" "
      <<newUp.timeInterval.rc<<"]"<<endl;
      if(newUp.timeInterval.end > newUp.timeInterval.start
        || (newUp.timeInterval.end == newUp.timeInterval.start
        && newUp.timeInterval.lc && newUp.timeInterval.rc))
        endResult->Add(newUp);
    }
  }
  endResult->EndBulkLoad(false);
}

/*
16.3.45 Value mapping functions of operator ~minus~ 
 for mpoint/mpoint

*/

int TemporalMMPointIntercept( Word* args, Word& result, int
 message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  
  MBool *mBool = new MBool(0);
  
  MovingPointCompareMM(*((MPoint*)args[0].addr),
    *((MPoint*)args[1].addr), *mBool, 0);

  MPoint* endResult = (MPoint*)result.addr;
  TransformMBool2MPoint(((MPoint*)args[0].addr), mBool,
    endResult);
 
  delete mBool;
 
  return 0;
}

/*
16.3.45 Value mapping functions of operator ~minus~ 
 for mpoint/point

*/

int TemporalMSPointIntercept( Word* args, Word& result, int
 message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  
  MBool *mBool = new MBool(0);
  
  MovingPointCompareMS(*((MPoint*)args[0].addr),
    *((Point*)args[1].addr), *mBool, 0);

  MPoint* endResult = (MPoint*)result.addr;
  TransformMBool2MPoint(((MPoint*)args[0].addr), mBool,
    endResult);
 
  delete mBool;
 
  return 0;
}

/*
16.3.45 Value mapping functions of operator ~minus~ 
 for point/mpoint

*/

int TemporalSMPointIntercept( Word* args, Word& result, int
 message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  
  MBool *mBool = new MBool(0);
  
  MovingPointCompareMS(*((MPoint*)args[1].addr),
    *((Point*)args[0].addr), *mBool, 0);

  MPoint* endResult = (MPoint*)result.addr;
  TransformMBool2MPoint(((Point*)args[0].addr), mBool,
    endResult);
 
  delete mBool;
 
  return 0;
}

/*
16.3.45 Value mapping functions of operator ~=~ 
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
16.3.45 Value mapping functions of operator ~=~ 
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
16.3.45 Value mapping functions of operator ~=~ 
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
16.3.45 Value mapping functions of operator ~=~ 
and ~\#~ for mpoint/mpoint

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
16.3.45 Value mapping functions of operator ~=~ 
and ~\#~ for mpoint/point

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
16.3.45 Value mapping functions of operator ~=~ 
and ~\#~ for point/mpoint

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
16.3.45 Value mapping functions of operator ~=~ 
and ~\#~ for mregion/region

*/

template<int op>
int TemporalMSRegionCompare( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  
  MovingRegionCompareMS((MRegion*)args[0].addr,
   (CRegion*)args[1].addr, (MBool*)result.addr, op); 
 
  return 0;
}

/*
16.3.45 Value mapping functions of operator ~=~ 
and ~\#~ for region/mregion

*/

template<int op>
int TemporalSMRegionCompare( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  
  MovingRegionCompareMS((MRegion*)args[1].addr,
   (CRegion*)args[0].addr, (MBool*)result.addr, op); 
 
  return 0;
}

/*
16.3.45 Value mapping functions of operator ~=~ 
and ~\#~ for mregion/mregion

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
16.3.46 Value mapping functions of operators ~mequal~,
 ~mnotequal~, ~<~, ~<=~, ~>=~ and ~>~ 
for moving/static

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
16.3.47 Value mapping functions of operators ~mequal~,
 ~mnotequal~, ~<~, ~<=~, ~>=~ and ~>~ 
for static/moving/

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
16.3.46 Value mapping function of operators ~abs~

*/

int MovingRealABS( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  
  MRealABS( *((MReal*)args[0].addr), *((MReal*)result.addr));
  
  return 0;
}

/*
16.3.47. Value mapping of operator ~minus~ for real/movingreal

*/

int MRealMMDistance( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  
  MRealDistanceMM( *((MReal*)args[0].addr),
   *((MReal*)args[1].addr), *((MReal*)result.addr));
  
  return 0;
}

int MRealMSDistance( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  cout<<"MRealMSDistance called"<<endl;
  result = qp->ResultStorage( s );
  MReal *mop1;
  MReal *mop2 = new MReal(0); 
  const UReal *up1;
  mop1 = (MReal*)args[0].addr;
  CcReal *constop = (CcReal*)args[1].addr;
  
  mop2->Clear();
  mop2->StartBulkLoad();
  for (int i = 0; i < mop1->GetNoComponents(); i++) {
    mop1->Get(i, up1);
    UReal *up2 = new UReal(up1->timeInterval, 0.0, 0.0, 
      (up1->r) ? pow(constop->GetRealval(), 2)
      : constop->GetRealval(),up1->r);
    cout<<"up2["<<i<<"] ["<<up2->timeInterval.start.ToDouble()
    <<" "<<up2->timeInterval.end.ToDouble()<<" "
    <<up2->timeInterval.lc<<" "<<up2->timeInterval.rc<<"] "<<" a: "
    <<up2->a<<" b: "<<up2->b<<" c: "<<up2->c<<" r: "<<up2->r<<endl;
    mop2->Add(*up2);
    delete up2;
  }
  mop2->EndBulkLoad(false);
  MRealDistanceMM( *mop1, *mop2, *((MReal*)result.addr));
  
  delete mop2;
  
  return 0;
}

int MRealSMDistance( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  cout<<"MRealSMDistance called"<<endl;
  result = qp->ResultStorage( s );
  MReal *mop1;
  MReal *mop2 = new MReal(0); 
  const UReal *up1;
  mop1 = (MReal*)args[1].addr;
  CcReal *constop = (CcReal*)args[0].addr;
  
  mop2->Clear();
  mop2->StartBulkLoad();
  for (int i = 0; i < mop1->GetNoComponents(); i++) {
    mop1->Get(i, up1);
    UReal *up2 = new UReal(up1->timeInterval, 0.0, 0.0, 
      (up1->r) ? pow(constop->GetRealval(), 2)
      : constop->GetRealval(),up1->r);
    cout<<"up2["<<i<<"] ["<<up2->timeInterval.start.ToDouble()
    <<" "<<up2->timeInterval.end.ToDouble()<<" "
    <<up2->timeInterval.lc<<" "<<up2->timeInterval.rc<<"] "<<" a: "
    <<up2->a<<" b: "<<up2->b<<" c: "<<up2->c<<" r: "<<up2->r<<endl;
    mop2->Add(*up2);
    delete up2;
  }
  mop2->EndBulkLoad(false);
  MRealDistanceMM( *mop1, *mop2, *((MReal*)result.addr));
  
  delete mop2;
  
  return 0;
}

/*
Function CompletePeriods2MBool completes a Periods-value to 
a MBool-value. For this it puts the intervals in pResult as uBool
with value ~true~ and added the difference to the MPoint-
intervals with ~false~.

*/

static void CompletePeriods2MBool(MPoint* mp, Periods* pResult,
  MBool* endResult){
  const UPoint *up;
  
  endResult->Clear();
  endResult->StartBulkLoad();
  const Interval<Instant> *per;
  UBool uBool;
  int m = 0;
  bool pfinished = (pResult->GetNoComponents() == 0);
  for ( int i = 0; i < mp->GetNoComponents(); i++) {
    mp->Get(i, up);
    cout<<"UPoint # "<<i<<" ["<<up->timeInterval.start.ToDouble()
    <<" "<<up->timeInterval.end.ToDouble()<<" "
    <<up->timeInterval.lc<<" "<<up->timeInterval.rc<<"] ("
    <<up->p0.GetX()<<" "<<up->p0.GetY()<<")->("<<up->p1.GetX()
    <<" "<<up->p1.GetY()<<")"<<endl;
    if(!pfinished) {
      pResult->Get(m, per);
      cout<<"per "<<m<<" ["<<per->start.ToDouble()<<" "
      <<per->end.ToDouble()<<" "<<per->lc<<" "<<per->rc<<"]"<<endl;
    }
    else
      cout<<"no per any more"<<endl;
    if(pfinished 
       || up->timeInterval.end < per->start 
       || (up->timeInterval.end == per->start 
       && !up->timeInterval.rc && per->lc)) {
       cout<<"per totally after up"<<endl;
       uBool.constValue.Set(true, false);
       uBool.timeInterval = up->timeInterval;
       cout<<"MergeAdd1 "<<uBool.constValue.GetBoolval()
       <<" ["<<uBool.timeInterval.start.ToDouble()<<" "
       <<uBool.timeInterval.end.ToDouble()<<" "
       <<uBool.timeInterval.lc<<" "
       <<uBool.timeInterval.rc<<"]"<<endl;
       endResult->MergeAdd(uBool);
    }
    else {
      cout<<"per not after before up"<<endl;
      if(up->timeInterval.start < per->start || 
        (up->timeInterval.start == per->start 
         && up->timeInterval.lc && !per->lc)) {
        cout<<"up starts before up"<<endl;
        uBool.constValue.Set(true, false);
        uBool.timeInterval.start = up->timeInterval.start; 
        uBool.timeInterval.lc = up->timeInterval.lc;
        uBool.timeInterval.end = per->start; 
        uBool.timeInterval.rc = !per->lc;  
        cout<<"MergeAdd2 "<<uBool.constValue.GetBoolval()
        <<" ["<<uBool.timeInterval.start.ToDouble()<<" "
        <<uBool.timeInterval.end.ToDouble()<<" "
        <<uBool.timeInterval.lc<<" "
        <<uBool.timeInterval.rc<<"]"<<endl;
        endResult->MergeAdd(uBool);
        uBool.timeInterval = *per;
      }
      else {
        cout<<"per starts before or with up"<<endl;
        uBool.timeInterval.start = up->timeInterval.start;
        uBool.timeInterval.lc = up->timeInterval.lc;
      }
      while(true) {
        uBool.constValue.Set(true, true);
        if(up->timeInterval.end < per->end
             || (up->timeInterval.end == per->end 
             && per->rc && !up->timeInterval.rc)) {
            cout<<"per ends after up (break)"<<endl;
            uBool.timeInterval.end = up->timeInterval.end; 
            uBool.timeInterval.rc = up->timeInterval.rc; 
            cout<<"MergeAdd3 "<<uBool.constValue.GetBoolval()
            <<" ["<<uBool.timeInterval.start.ToDouble()<<" "
            <<uBool.timeInterval.end.ToDouble()<<" "
            <<uBool.timeInterval.lc<<" "
            <<uBool.timeInterval.rc<<"]"<<endl;
            endResult->MergeAdd(uBool); 
            break;
        }
        else {
          cout<<"per ends inside up"<<endl;
          uBool.timeInterval.end = per->end;
          uBool.timeInterval.rc = per->rc;
          cout<<"MergeAdd4 "<<uBool.constValue.GetBoolval()
          <<" ["<<uBool.timeInterval.start.ToDouble()<<" "
          <<uBool.timeInterval.end.ToDouble()<<" "
          <<uBool.timeInterval.lc<<" "
          <<uBool.timeInterval.rc<<"]"<<endl;
          endResult->MergeAdd(uBool);
        }
        uBool.timeInterval.start = per->end; 
        uBool.timeInterval.lc = !per->rc; 
        if(m == pResult->GetNoComponents() - 1){
          pfinished = true;
          //break;
        }
        else {
          pResult->Get(++m, per);
          cout<<"per "<<m<<" ["<<per->start.ToDouble()<<" "<<per->end.ToDouble()
          <<" "<<per->lc<<" "<<per->rc<<"]"<<endl;
        }
        
        if(!pfinished && (per->start < up->timeInterval.end 
           || (per->start == up->timeInterval.end 
           && up->timeInterval.rc && per->rc))){
          cout<<"next per starts in same up"<<endl;
          uBool.timeInterval.end = per->start; 
          uBool.timeInterval.rc = !per->lc;
          uBool.constValue.Set(true, false);
          cout<<"MergeAdd6 "<<uBool.constValue.GetBoolval()
          <<" ["<<uBool.timeInterval.start.ToDouble()<<" "
          <<uBool.timeInterval.end.ToDouble()<<" "
          <<uBool.timeInterval.lc<<" "
          <<uBool.timeInterval.rc<<"]"<<endl;
          endResult->MergeAdd(uBool); 
          uBool.timeInterval.start = per->start; 
          uBool.timeInterval.lc = per->lc; 
        }
        else {
          cout<<"next interval after up -> finish up"<<endl;
          uBool.timeInterval.end = up->timeInterval.end; 
          uBool.timeInterval.rc = up->timeInterval.rc;
          uBool.constValue.Set(true, false);
          if(uBool.timeInterval.end > uBool.timeInterval.start 
             || (uBool.timeInterval.rc && uBool.timeInterval.lc)) {
            cout<<"MergeAdd5 "<<uBool.constValue.GetBoolval()
            <<" ["<<uBool.timeInterval.start.ToDouble()<<" "
            <<uBool.timeInterval.end.ToDouble()<<" "
            <<uBool.timeInterval.lc<<" "
            <<uBool.timeInterval.rc<<"]"<<endl;
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
Function CompletePeriods2MPoint completes a Periods-value to 
a MPoint-value. For this it adds the starting and end points.

*/

static void CompletePeriods2MPoint(MPoint* mp, Periods* pResult,
  MPoint* endResult){
  const UPoint *up;
  
  endResult->Clear();
  endResult->StartBulkLoad();
  const Interval<Instant> *per;
  UPoint newUp;
  Point pt;
  int m = 0;
  bool pfinished = false; //(pResult->GetNoComponents() == 0);
  for ( int i = 0; i < mp->GetNoComponents(); i++) {
    mp->Get(i, up);
    cout<<"UPoint # "<<i<<" ["<<up->timeInterval.start.ToDouble()
    <<" "<<up->timeInterval.end.ToDouble()<<" "
    <<up->timeInterval.lc<<" "<<up->timeInterval.rc<<"] ("
    <<up->p0.GetX()<<" "<<up->p0.GetY()<<")->("<<up->p1.GetX()
    <<" "<<up->p1.GetY()<<")"<<endl;
    if(!pfinished) {
      pResult->Get(m, per);
      cout<<"per "<<m<<" ["<<per->start.ToDouble()<<" "
      <<per->end.ToDouble()<<" "<<per->lc<<" "<<per->rc<<"]"<<endl;
    }
    else
      cout<<"no per any more"<<endl;
    if(pfinished) {
      cout<<"no per any more. break 1"<<endl;
      break;
    }
    if(!(pfinished 
       || up->timeInterval.end < per->start 
       || (up->timeInterval.end == per->start 
       && !up->timeInterval.rc && per->lc))) {
      cout<<"per not totally after up"<<endl;
      if(up->timeInterval.start < per->start || 
        (up->timeInterval.start == per->start 
         && up->timeInterval.lc && !per->lc)) {
        cout<<"up starts before per"<<endl;
        newUp.timeInterval = *per;
      }
      else {
        cout<<"per starts before or with up"<<endl;
        newUp.timeInterval.start = up->timeInterval.start;
        newUp.timeInterval.lc = up->timeInterval.lc;
      }
      while(true) {
        if(up->timeInterval.end < per->end
             || (up->timeInterval.end == per->end 
             && per->rc && !up->timeInterval.rc)) {
            cout<<"per ends after up (break)"<<endl;
            newUp.timeInterval.end = up->timeInterval.end; 
            newUp.timeInterval.rc = up->timeInterval.rc; 
            up->TemporalFunction(newUp.timeInterval.start, pt);
            newUp.p0 = pt;
            up->TemporalFunction(newUp.timeInterval.end, pt);
            newUp.p1 = pt;
            cout<<"Add3 ("
            <<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
            <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
            <<") ["<<newUp.timeInterval.start.ToDouble()<<" "
            <<newUp.timeInterval.end.ToDouble()<<" "
            <<newUp.timeInterval.lc<<" "
            <<newUp.timeInterval.rc<<"]"<<endl;
            endResult->Add(newUp); 
            break;
        }
        else {
          cout<<"per ends inside up"<<endl;
          newUp.timeInterval.end = per->end;
          newUp.timeInterval.rc = per->rc;
          up->TemporalFunction(newUp.timeInterval.start, pt);
          newUp.p0 = pt;
          up->TemporalFunction(newUp.timeInterval.end, pt);
          newUp.p1 = pt;
          cout<<"Add4 ("
          <<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
          <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
          <<") ["<<newUp.timeInterval.start.ToDouble()<<" "
          <<newUp.timeInterval.end.ToDouble()<<" "
          <<newUp.timeInterval.lc<<" "
          <<newUp.timeInterval.rc<<"]"<<endl;
          endResult->Add(newUp);
        }
       // newUp.timeInterval.start = per->end; 
       // newUp.timeInterval.lc = !per->rc; 
        if(m == pResult->GetNoComponents() - 1){
          cout<<"last per"<<endl;
          pfinished = true;
          //break;
        }
        else {
          pResult->Get(++m, per);
          cout<<"per "<<m<<" ["<<per->start.ToDouble()<<" "<<per->end.ToDouble()
          <<" "<<per->lc<<" "<<per->rc<<"]"<<endl;
        }
        if(!pfinished && (per->start < up->timeInterval.end 
           || (per->start == up->timeInterval.end 
           && up->timeInterval.rc && per->rc))){
          cout<<"next per starts in same up"<<endl;
          newUp.timeInterval.start = per->start; 
          newUp.timeInterval.lc = per->lc; 
        }
        else {
          cout<<"next interval after up -> finish up"<<endl;
          break;
        }
      } //while
    }
  }
  endResult->EndBulkLoad(false);
}

/*
Function ~MPointInsidePoints~ calcultates the periods where the 
given MPoint lies inside the given Points. It return the existing 
intervals in a Periods-Object. 

*/

static void MPointInsidePoints(MPoint& mp, Points& ps, Periods& pResult)
{
  const UPoint *up;
  const Point *p;
  
  Periods* between = new Periods(0);
  Periods* period = new Periods(0);
  Interval<Instant> newper; //part of the result
  
  pResult.Clear();
  for( int i = 0; i < mp.GetNoComponents(); i++)
  {
    cout<<"MPointPointsInside # "<<i<<endl;
    mp.Get(i, up);
    cout<<"UPoint # "<<i<<" ["
    <<up->timeInterval.start.ToDouble()<<" "
    <<up->timeInterval.end.ToDouble()<<" "
    <<up->timeInterval.lc<<" "<<up->timeInterval.rc<<"] ("
    <<up->p0.GetX()<<" "<<up->p0.GetY()<<")-("<<up->p1.GetX()<<" "
    <<up->p1.GetY()<<")"<<endl;
    for( int n = 0; n < ps.Size(); n++)
    {
      ps.Get(n, p);
      cout<<"point # "<<n<<" ("<<p->GetX()<<" "<<p->GetY()
      <<") "<<endl;
      double dx, dy;
      bool vert = false;
      bool hor = false;
      if((up->p1.GetX() - up->p0.GetX()) != 0.0){
         dx = (p->GetX() - up->p0.GetX()) / (up->p1.GetX() 
         - up->p0.GetX());
         if(dx < 0.0 || dx > 1.0)
            continue;
      }
      else
         vert = true;
      if((up->p1.GetY() - up->p0.GetY()) != 0.0){
         dy = (p->GetY() - up->p0.GetY()) / (up->p1.GetY() 
         - up->p0.GetY());
         if(dy < 0.0 || dy > 1.0)
            continue;
      }
      else
         hor = true;
      cout<<"dx "<<vert<<" "<<(vert ? 0.0 : dx)<<", dy "<<hor
      <<" "<<(hor ? 0.0 : dy)<<endl;
      if(hor && vert){
        cout<<"hor and vert"<<endl;
        if(AlmostEqual(p->GetY(), up->p0.GetY())
        && AlmostEqual(p->GetX(), up->p0.GetX())){
          newper = up->timeInterval;
        }
      }
      else if(hor || AlmostEqual(dx, dy)){
        cout<<"horizontal line or same time "<<hor<<endl;
        if((dx > 0.0 || up->timeInterval.lc) 
        && (dx < 1.0 || up->timeInterval.rc)){
          Instant t;
          t.ReadFrom((up->timeInterval.end.ToDouble() 
          - up->timeInterval.start.ToDouble()) * dx 
          + up->timeInterval.start.ToDouble());
          t.SetType(instanttype);
          newper.start = t;
          newper.end = t;
          newper.lc = true;
          newper.rc = true;
        }
      }
      else if(vert){
        cout<<"vertical line"<<endl;
        if((dy > 0.0 || up->timeInterval.lc) 
        && (dy < 1.0 || up->timeInterval.rc)){
          Instant t;
          t.ReadFrom((up->timeInterval.end.ToDouble() 
          - up->timeInterval.start.ToDouble()) * dy 
          + up->timeInterval.start.ToDouble());
          t.SetType(instanttype);
          newper.start = t;
          newper.end = t;
          newper.lc = true;
          newper.rc = true;
        }
      }
      cout<<"newper ["<< newper.start.ToDouble()
      <<" "<<newper.end.ToDouble()<<" "<<newper.lc<<" "
      <<newper.rc<<"]"<<endl;
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
16.3.47. Value mapping of operator ~inside~ and ~intersection~
for mpoint/points. op decides whether it means inside(1) or
intersection(2)

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
    CompletePeriods2MBool(((MPoint*)args[0].addr),
      pResult, endResult);
  }
  else{ //create a MPoint (intersection)
    MPoint* endResult = (MPoint*)result.addr;
    CompletePeriods2MPoint(((MPoint*)args[0].addr),
      pResult, endResult);
  }

  delete pResult;
  
  return 0;
}

/*
16.3.47. Value mapping of operator ~intersection~ for points/mPoint

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
  CompletePeriods2MPoint(((MPoint*)args[1].addr),
    pResult, endResult);

  delete pResult;
  
  return 0;
}

/*
Function ~MPointInsideLine~ calcultates the periods where the 
given MPoint lies inside the given CLine. It return the existing 
intervals in a Periods-Object. 

*/

static void MPointInsideLine(MPoint& mp, CLine& ln, Periods& pResult)
{
  const UPoint *up;
  const CHalfSegment *l;

  Periods* period = new Periods(0);
  Periods* between = new Periods(0);
  Point pt;
  Interval<Instant> newper; //part of the result
  
  pResult.Clear();
  for( int i = 0; i < mp.GetNoComponents(); i++)
  {
    cout<<"MPointLineInside # "<<i<<endl;
    mp.Get(i, up);
    cout<<"UPoint # "<<i<<" ["<<up->timeInterval.start.ToDouble()
    <<" "<<up->timeInterval.end.ToDouble()<<" "
    <<up->timeInterval.lc<<" "<<up->timeInterval.rc<<"] ("
    <<up->p0.GetX()<<" "<<up->p0.GetY()<<")->("<<up->p1.GetX()
    <<" "<<up->p1.GetY()<<")"<<endl;

    for( int n = 0; n < ln.Size(); n++)
    {
      Instant t;
      ln.Get(n, l);
      cout<<"UPoint # "<<i
      <<" ["<<up->timeInterval.start.ToDouble()
      <<" "<<up->timeInterval.end.ToDouble()<<" "
      <<up->timeInterval.lc<<" "<<up->timeInterval.rc<<"] ("
      <<up->p0.GetX()<<" "<<up->p0.GetY()<<")->("<<up->p1.GetX()
      <<" "<<up->p1.GetY()<<")"<<endl;
      cout<<"l      # "<<n<<" ("<<l->GetLP().GetX()
      <<" "<<l->GetLP().GetY()<<" "<<l->GetRP().GetX()<<" "
      <<l->GetRP().GetY()<<") "<<endl;
      if(l->GetRP().GetX() == l->GetDPoint().GetX() 
      && l->GetRP().GetY() == l->GetDPoint().GetY()) {
        cout<<"right point is dominating -> continue"<<endl;
        continue;
      }
      if((l->GetRP().GetX() < up->p0.GetX() 
      && l->GetRP().GetX() < up->p1.GetX()) 
      || (l->GetLP().GetX() > up->p0.GetX() 
      && l->GetLP().GetX() > up->p1.GetX()) 
      || (l->GetRP().GetY() < up->p0.GetY() 
      && l->GetRP().GetY() < up->p1.GetY() 
      && (l->GetLP().GetY() < up->p0.GetY() 
      && l->GetLP().GetY() < up->p1.GetY())) 
      || (l->GetRP().GetY() > up->p0.GetY() 
      && l->GetRP().GetY() > up->p1.GetY() 
      && (l->GetLP().GetY() > up->p0.GetY() 
      && l->GetLP().GetY() > up->p1.GetY()))) {
        cout<<"Bounding Boxes not crossing!"<<endl;
        continue;
      }
      double al, bl, aup, bup;
      bool vl, vup;
      vl = l->GetRP().GetX() == l->GetLP().GetX();
      if(!vl){
        al = (l->GetRP().GetY() - l->GetLP().GetY()) 
        / (l->GetRP().GetX() - l->GetLP().GetX());
          bl = l->GetLP().GetY() - l->GetLP().GetX() * al;
          cout<<"al: "<<al<<" bl: "<<bl<<endl;
      }
      else
        cout<<"l is vertical"<<endl;
      vup = up->p1.GetX() == up->p0.GetX();
      if(!vup){
        aup = (up->p1.GetY() - up->p0.GetY()) / (up->p1.GetX() 
        - up->p0.GetX());
        bup = up->p0.GetY() - up->p0.GetX() * aup;
        cout<<"aup: "<<aup<<" bup: "<<bup<<endl;
      }
      else 
        cout<<"up is vertical"<<endl;
      if(vl && vup){
        cout<<"both elements are vertical!"<<endl;
        if(up->p1.GetX() != l->GetLP().GetX()){
        cout<<"elements are vertical but not at same line"<<endl;
          continue;
        }
        else {
          cout<<"elements on same line"<<endl;
          if(up->p1.GetY() < l->GetLP().GetY() 
          && up->p0.GetY() < l->GetLP().GetY()){
            cout<<"uPoint lower as linesegment"<<endl;
            continue;
          }
          else if(up->p1.GetY() > l->GetRP().GetY() 
          && up->p0.GetY() > l->GetRP().GetY()){
            cout<<"uPoint higher as linesegment"<<endl;
            continue;
          }
          else{
            cout<<"uPoint and linesegment partequal"<<endl;
            if(up->p0.GetY() <= l->GetLP().GetY() 
            && up->p1.GetY() >= l->GetLP().GetY()){
              cout<<"uPoint starts below linesegemet"<<endl;
              t.ReadFrom((l->GetLP().GetY() - up->p0.GetY()) 
              / (up->p1.GetY() - up->p0.GetY()) 
              * (up->timeInterval.end.ToDouble() 
              - up->timeInterval.start.ToDouble()) 
              + up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
              cout<<"t "<<t.ToDouble()<<endl;
              newper.start = t;
              newper.lc = (up->timeInterval.start == t) 
              ? up->timeInterval.lc : true;
            }
            if(up->p1.GetY() <= l->GetLP().GetY() 
            && up->p0.GetY() >= l->GetLP().GetY()){
              cout<<"uPoint ends below linesegemet"<<endl;
              t.ReadFrom((l->GetLP().GetY() - up->p0.GetY()) 
              / (up->p1.GetY() - up->p0.GetY()) 
              * (up->timeInterval.end.ToDouble() 
              - up->timeInterval.start.ToDouble()) 
              + up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
              cout<<"t "<<t.ToDouble()<<endl;
              newper.end = t;
              newper.rc = (up->timeInterval.end == t) 
              ? up->timeInterval.rc : true;
            }
            if(up->p0.GetY() <= l->GetRP().GetY() 
            && up->p1.GetY() >= l->GetRP().GetY()){
              cout<<"uPoint ends above linesegemet"<<endl;
              t.ReadFrom((l->GetRP().GetY() - up->p0.GetY()) 
              / (up->p1.GetY() - up->p0.GetY()) 
              * (up->timeInterval.end.ToDouble() 
              - up->timeInterval.start.ToDouble()) 
              + up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
              cout<<"t "<<t.ToDouble()<<endl;
              newper.end = t;
              newper.rc = (up->timeInterval.end == t) 
              ? up->timeInterval.rc : true;
            }
            if(up->p1.GetY() <= l->GetRP().GetY() 
            && up->p0.GetY() >= l->GetRP().GetY()){
              cout<<"uPoint starts above linesegemet"<<endl;
              t.ReadFrom((l->GetRP().GetY() - up->p0.GetY()) 
              / (up->p1.GetY() - up->p0.GetY()) 
              * (up->timeInterval.end.ToDouble() 
              - up->timeInterval.start.ToDouble()) 
              + up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
              cout<<"t "<<t.ToDouble()<<endl;
              newper.start = t;
              newper.lc = (up->timeInterval.start == t) 
              ? up->timeInterval.lc : true;
            }
            if(up->p0.GetY() <= l->GetRP().GetY() 
            && up->p0.GetY() >= l->GetLP().GetY()){
              cout<<"uPoint starts inside linesegemet"<<endl;
              newper.start = up->timeInterval.start;
              newper.lc =  up->timeInterval.lc;
            }
            if(up->p1.GetY() <= l->GetRP().GetY() 
            && up->p1.GetY() >= l->GetLP().GetY()){
              cout<<"uPoint ends inside linesegemet"<<endl;
              newper.end = up->timeInterval.end;
              newper.rc =  up->timeInterval.rc;
            }
            if(newper.start == newper.end 
              && (!newper.lc || !newper.rc)){
              cout<<"not an interval"<<endl;
              continue;
            }
          }
        }
      }
      else if(vl){
        cout<<"vl is vertical vup not"<<endl;
        t.ReadFrom((l->GetRP().GetX() - up->p0.GetX()) 
        / (up->p1.GetX() - up->p0.GetX()) 
        * (up->timeInterval.end.ToDouble() 
        - up->timeInterval.start.ToDouble()) 
        + up->timeInterval.start.ToDouble());
        t.SetType(instanttype);
        cout<<"t "<<t.ToDouble()<<endl;
        if((up->timeInterval.start == t && !up->timeInterval.lc) 
        || (up->timeInterval.end == t && !up->timeInterval.rc))
          continue;
          
        if(up->timeInterval.start > t|| up->timeInterval.end < t){
          cout<<"up outside line"<<endl;
          continue;
        }
        up->TemporalFunction(t, pt);
        if( pt.GetX() < l->GetLP().GetX() 
         || pt.GetX() > l->GetRP().GetX()
         || (pt.GetY() < l->GetLP().GetY() && pt.GetY() < l->GetRP().GetY())
         || (pt.GetY() > l->GetLP().GetY() && pt.GetY() > l->GetRP().GetY())){
          cout<<"pt outside up!"<<endl;
          continue;
        }
        
        newper.start = t;
        newper.lc = true;
        newper.end = t;
        newper.rc = true;
      }
      else if(vup){
        cout<<"vup is vertical vl not"<<endl;
        if(up->p1.GetY() != up->p0.GetY()) {
          t.ReadFrom((up->p0.GetX() * al + bl - up->p0.GetY()) 
          / (up->p1.GetY() - up->p0.GetY()) 
          * (up->timeInterval.end.ToDouble() 
          - up->timeInterval.start.ToDouble()) 
          + up->timeInterval.start.ToDouble());
          t.SetType(instanttype);
          cout<<"t "<<t.ToDouble()<<endl;
          if((up->timeInterval.start == t && !up->timeInterval.lc) 
          || (up->timeInterval.end == t && !up->timeInterval.rc)){
            cout<<"continue"<<endl;
            continue;
          }
          
          if(up->timeInterval.start > t|| up->timeInterval.end < t){
            cout<<"up outside line"<<endl;
            continue;
          }
          up->TemporalFunction(t, pt);
          if( pt.GetX() < l->GetLP().GetX() 
           || pt.GetX() > l->GetRP().GetX()
           || (pt.GetY() < l->GetLP().GetY() && pt.GetY() < l->GetRP().GetY())
           || (pt.GetY() > l->GetLP().GetY() && pt.GetY() > l->GetRP().GetY())){
            cout<<"pt outside up!"<<endl;
            continue;
          }
          
          newper.start = t;
          newper.lc = true;
          newper.end = t;
          newper.rc = true;
        }
        else {
          cout<<"up is not moving"<<endl;
          if(al * up->p1.GetX() + bl == up->p1.GetY()){
            cout<<"Point lies on line"<<endl;
            newper = up->timeInterval;
          }
          else {
            cout<<"continue 2"<<endl;
            continue;
          }
        }
      }
      else if(aup == al){
        cout<<"both lines have same gradient"<<endl;
        if(bup != bl){
          cout<<"colinear but not equal"<<endl;
          continue;
        }
         if(up->p0.GetX() <= l->GetLP().GetX() 
         && up->p1.GetX() >= l->GetLP().GetX()){
           cout<<"uPoint starts left of linesegemet"<<endl;
           t.ReadFrom((l->GetLP().GetX() - up->p0.GetX()) 
           / (up->p1.GetX() - up->p0.GetX()) 
           * (up->timeInterval.end.ToDouble() 
           - up->timeInterval.start.ToDouble()) 
           + up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
           cout<<"t "<<t.ToDouble()<<endl;
           newper.start = t;
           newper.lc = (up->timeInterval.start == t) 
           ? up->timeInterval.lc : true;
        }
        if(up->p1.GetX() <= l->GetLP().GetX() 
        && up->p0.GetX() >= l->GetLP().GetX()){
           cout<<"uPoint ends left of linesegemet"<<endl;
           t.ReadFrom((l->GetLP().GetX() - up->p0.GetX()) 
           / (up->p1.GetX() - up->p0.GetX()) 
           * (up->timeInterval.end.ToDouble() 
           - up->timeInterval.start.ToDouble()) 
           + up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
           cout<<"t "<<t.ToDouble()<<endl;
           newper.end = t;
           newper.rc = (up->timeInterval.end == t) 
           ? up->timeInterval.rc : true;
        }
        if(up->p0.GetX() <= l->GetRP().GetX() 
        && up->p1.GetX() >= l->GetRP().GetX()){
           cout<<"uPoint ends right of linesegemet"<<endl;
           t.ReadFrom((l->GetRP().GetX() - up->p0.GetX()) 
           / (up->p1.GetX() - up->p0.GetX()) 
           * (up->timeInterval.end.ToDouble() 
           - up->timeInterval.start.ToDouble()) 
           + up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
           cout<<"t "<<t.ToDouble()<<endl;
           newper.end = t;
           newper.rc = (up->timeInterval.end == t) 
           ? up->timeInterval.rc : true;
        }
        if(up->p1.GetX() <= l->GetRP().GetX() 
        && up->p0.GetX() >= l->GetRP().GetX()){
           cout<<"uPoint starts right of linesegemet"<<endl;
           t.ReadFrom((l->GetRP().GetX() - up->p0.GetX()) 
           / (up->p1.GetX() - up->p0.GetX()) 
           * (up->timeInterval.end.ToDouble() 
           - up->timeInterval.start.ToDouble()) 
           + up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
           cout<<"t "<<t.ToDouble()<<endl;
           newper.start = t;
           newper.lc = (up->timeInterval.start == t) 
           ? up->timeInterval.lc : true;
        }
        if(up->p0.GetX() <= l->GetRP().GetX() 
        && up->p0.GetX() >= l->GetLP().GetX()){
           cout<<"uPoint starts inside linesegemet"<<endl;
           newper.start = up->timeInterval.start;
           newper.lc = up->timeInterval.lc;
        }
        if(up->p1.GetX() <= l->GetRP().GetX() 
        && up->p1.GetX() >= l->GetLP().GetX()){
           cout<<"uPoint ends inside linesegemet"<<endl;
           newper.end = up->timeInterval.end;
           newper.rc = up->timeInterval.rc;
        }
        if(newper.start == newper.end 
        && (!newper.lc || !newper.rc)){
          cout<<"not an interval"<<endl;
          continue;
        }
      }
      else{
        cout<<"both lines have different gradients"<<endl;
        t.ReadFrom(((bl - bup) / (aup - al) - up->p0.GetX()) 
        / (up->p1.GetX() - up->p0.GetX()) 
        * (up->timeInterval.end.ToDouble() 
        - up->timeInterval.start.ToDouble()) 
        + up->timeInterval.start.ToDouble());
        t.SetType(instanttype);
        if((up->timeInterval.start == t && !up->timeInterval.lc) 
        || (up->timeInterval.end == t && !up->timeInterval.rc)){
          cout<<"continue"<<endl;
          continue;
        }
        
        if(up->timeInterval.start > t|| up->timeInterval.end < t){
          cout<<"up outside line"<<endl;
          continue;
        }
        up->TemporalFunction(t, pt);
        if( pt.GetX() < l->GetLP().GetX() 
         || pt.GetX() > l->GetRP().GetX()
         || (pt.GetY() < l->GetLP().GetY() && pt.GetY() < l->GetRP().GetY())
         || (pt.GetY() > l->GetLP().GetY() && pt.GetY() > l->GetRP().GetY())){
          cout<<"pt outside up!"<<endl;
          continue;
        }
        
        newper.start = t;
        newper.lc = true;
        newper.end = t;
        newper.rc = true;
      }
      cout<<"newper ["<< newper.start.ToDouble()
      <<" "<<newper.end.ToDouble()<<" "<<newper.lc<<" "
      <<newper.rc<<"]"<<endl;
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
16.3.47. Value mapping of operator ~inside~ and ~intersection~
for mpoint/line. op decides whether it means inside(1) or
intersection(2)

*/

template<int op>
int MPointLineInside( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods* pResult = new Periods(0);
  
  MPointInsideLine( *((MPoint*)args[0].addr),
    *((CLine*)args[1].addr), *pResult);
  
  if(op == 1) { //create a MBool (inside)
    MBool* endResult = (MBool*)result.addr;
    CompletePeriods2MBool(((MPoint*)args[0].addr), 
      pResult, endResult);
  }
  else { //create a MPoint (intersection)
    MPoint* endResult = (MPoint*)result.addr;
    CompletePeriods2MPoint(((MPoint*)args[0].addr), 
      pResult, endResult);
  }
  
  delete pResult;
  
  return 0;
}



/*
16.3.47. Value mapping of operator ~intersection~ for line/mpoint

*/

int LineMPointIntersection( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods* pResult = new Periods(0);
  
  MPointInsideLine( *((MPoint*)args[1].addr),
    *((CLine*)args[0].addr), *pResult);
  
  //create a MPoint (intersection)
  MPoint* endResult = (MPoint*)result.addr;
  CompletePeriods2MPoint(((MPoint*)args[1].addr), 
    pResult, endResult);
  
  delete pResult;
  
  return 0;
}

/*
ValueMapping for ~perimeter~

*/

static int PerimeterValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    cout<< "PerimeterValueMap() called" << endl;

    result = qp->ResultStorage(s);

    MPerimeter(* (MRegion*) args[0].addr, * (MReal*) result.addr);

    return 0;
}

/*
ValueMapping for ~area~

*/

static int AreaValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    cout<< "AreaValueMap) called" << endl;

    result = qp->ResultStorage(s);

    MArea(* (MRegion*) args[0].addr, * (MReal*) result.addr);

    return 0;
}

/*
ValueMapping for ~rough\_center~

*/

static int RCenterValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    cout<< "RCenterValueMap() called" << endl;

    result = qp->ResultStorage(s);

    RCenter(* (MRegion*) args[0].addr,* (MPoint*) result.addr);

    return 0;
}

/*
ValueMapping for ~no\_components~

*/

static int NComponentsValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    cout<< "NComponentsValueMap() called" << endl;

    result = qp->ResultStorage(s);

    NComponents(* (MRegion*) args[0].addr,* (MInt*) result.addr);

    return 0;
}

void copyMRegion(MRegion& res, MRegion& result) {
   cout<<"copyMRegion() called"<<endl;
   result.CopyFrom(&res);
}

void copyRegionMPoint(CRegion& reg, MPoint& pt, MRegion& result) {
   cout<<"copyMRegion2() called"<<endl;
   MRegion* res = new MRegion(pt, reg);
   result.Clear();
   result.CopyFrom(res);
   delete res;
}

void copyMRegionMPoint(MRegion& reg, MPoint& pt, MRegion& result) {
    RefinementPartitionLift<MRegion, MPoint, URegion, UPoint> rp(reg,pt);
    result.Clear();
    result.StartBulkLoad();
    Interval<Instant>* iv;
    int regPos;
    int ptPos;
    
    
    for( unsigned int i = 0; i < rp.Size(); i++ ){
      rp.Get(i, iv, regPos, ptPos);
      if(regPos == -1 || ptPos == -1)
        continue;
      cout<<"bothoperators in iv # "<<i<<" [ "
      <<iv->start.ToDouble()<<" "<<iv->end.ToDouble()<<" "
      <<iv->lc<<" "<<iv->rc<<" ] regPos "<<regPos<<endl;
      URegion *ures = new URegion(*iv);
      const URegion *ureg;
      reg.Get(regPos, ureg);
      cout<<"12"<<endl;
      ures->CopyFrom(ureg);
      cout<<"13"<<endl;
      if(ureg->timeInterval.start < iv->start 
        || ureg->timeInterval.end > iv->end){
        cout<<"clip ureg to interval"<<endl;
        for(int n = 0; n < ureg->GetSegmentsNum(); n++){
          cout<<n<<". Segment"<<endl;
          const MSegmentData *dms;
          cout<<"1"<<endl;
          ureg->GetSegment(n, dms);
          cout<<"2"<<endl;
          MSegmentData rdms;
        
        cout<<"GetFaceNo()"<<dms->GetFaceNo()<<endl;
        cout<<"GetCycleNo()"<<dms->GetCycleNo()<<endl;
        cout<<"GetSegmentNo()"<<dms->GetSegmentNo()<<endl;
        cout<<"GetInitialStartX()"<<dms->GetInitialStartX()<<endl;
        cout<<"GetInitialStartY()"<<dms->GetInitialStartY()<<endl;
        cout<<"GetInitialEndX()"<<dms->GetInitialEndX()<<endl;
        cout<<"GetInitialEndY()"<<dms->GetInitialEndY()<<endl;
        cout<<"GetFinalStartX()"<<dms->GetFinalStartX()<<endl;
        cout<<"GetFinalStartY()"<<dms->GetFinalStartY()<<endl;
        cout<<"GetFinalEndX()"<<dms->GetFinalEndX()<<endl;
        cout<<"GetFinalEndY()"<<dms->GetFinalEndY()<<endl;
        cout<<"GetInsideAbove()"<<dms->GetInsideAbove()<<endl;

          dms->restrictToInterval(ureg->timeInterval, *iv, rdms);
          cout<<"3"<<endl;
          cout<<"iv [ "
      <<iv->start.ToDouble()<<" "<<iv->end.ToDouble()<<" "
      <<iv->lc<<" "<<iv->rc<<" ]"<<endl;
      cout<<"ureg [ "
      <<ureg->timeInterval.start.ToDouble()<<" "
      <<ureg->timeInterval.end.ToDouble()<<" "
      <<ureg->timeInterval.lc<<" "<<ureg->timeInterval.rc<<" ]"
      <<endl;
          ures->PutSegment(n, rdms);
          cout<<"4"<<endl;
       cout<<"GetFaceNo()"<<rdms.GetFaceNo()<<endl;
        cout<<"GetCycleNo()"<<rdms.GetCycleNo()<<endl;
        cout<<"GetSegmentNo()"<<rdms.GetSegmentNo()<<endl;
        cout<<"GetInitialStartX()"<<rdms.GetInitialStartX()<<endl;
        cout<<"GetInitialStartY()"<<rdms.GetInitialStartY()<<endl;
        cout<<"GetInitialEndX()"<<rdms.GetInitialEndX()<<endl;
        cout<<"GetInitialEndY()"<<rdms.GetInitialEndY()<<endl;
        cout<<"GetFinalStartX()"<<rdms.GetFinalStartX()<<endl;
        cout<<"GetFinalStartY()"<<rdms.GetFinalStartY()<<endl;
        cout<<"GetFinalEndX()"<<rdms.GetFinalEndX()<<endl;
        cout<<"GetFinalEndY()"<<rdms.GetFinalEndY()<<endl;
        cout<<"GetInsideAbove()"<<rdms.GetInsideAbove()<<endl;
        
        }
      }
      cout<<"5"<<endl;   
      ures->timeInterval = *iv;
      cout<<"6"<<endl;
      cout<<"[ "
      <<ures->timeInterval.start.ToDouble()<<" "
      <<ures->timeInterval.end.ToDouble()<<" "
      <<ures->timeInterval.lc<<" "<<ures->timeInterval.rc<<" ]"
      <<endl;
      result.Add(*ures);//Baustelle!!
      cout<<"7"<<endl;
      delete ures;
    }
    result.EndBulkLoad(false);
}

/*
ValueMapping for ~Minus~ with Region/MPoint

*/

static int RMPMinusValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    cout<< "RMPMinusValueMap() called" << endl;
    result = qp->ResultStorage(s);
    //MRegion* res = (MRegion*) result.addr;

    copyRegionMPoint(*((CRegion*)args[0].addr) ,
    *((MPoint*)args[1].addr), *((MRegion*)result.addr) );
   
    //MPoint* mp = (MPoint*) args[1].addr;
   // CRegion* r = (CRegion*) args[0].addr;
    
    //MRegion* mr = new MRegion(*mp, *r);
    
    //res->CopyFrom(mr);
    
    //delete mr;
   
    return 0;
}

/*
ValueMapping for ~Minus~ with MRegion/Point

*/

static int MRPMinusValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    cout<< "MRPMinusValueMap() called" << endl;

    result = qp->ResultStorage(s);

    copyMRegion(*((MRegion*)args[0].addr), *(MRegion*)result.addr);

    return 0;
}

/*
ValueMapping for ~Minus~ with MRegion/MPoint

*/

static int MRMPMinusValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    cout<< "MRMPMinusValueMap() called" << endl;

    result = qp->ResultStorage(s);

    copyMRegionMPoint(*((MRegion*)args[0].addr) ,
    *((MPoint*)args[1].addr), *((MRegion*)result.addr) );
    
    return 0;
}

/*
ValueMapping for ~Union~ with Region/MPoint

*/

static int MPRUnionValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    cout<< "MPRUnionValueMap() called" << endl;
    result = qp->ResultStorage(s);

    copyRegionMPoint(*((CRegion*)args[1].addr) ,
    *((MPoint*)args[0].addr), *((MRegion*)result.addr) );
    
    return 0;
}

/*
ValueMapping for ~Union~ with MRegion/Point

*/

static int PMRUnionValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    cout<< "PMRUnionValueMap() called" << endl;

    result = qp->ResultStorage(s);

    copyMRegion(*((MRegion*)args[1].addr),
     *(MRegion*)result.addr);

    return 0;
}

/*
ValueMapping for ~Union~ with MRegion/MPoint

*/

static int MPMRUnionValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    cout<< "MPMRUnionValueMap() called" << endl;

    result = qp->ResultStorage(s);

    copyMRegionMPoint(*((MRegion*)args[1].addr) ,
    *((MPoint*)args[0].addr), *((MRegion*)result.addr) );
    
    return 0;
}


/*
ValueMapping for ~isempty~ for mbool, mint, mreal, mpoint and mregion

*/

template<class Mapping1, class Unit1>
static int IsEmptyValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    if(TLA_DEBUG)
      cout<< "IsEmptyValueMap() called" << endl;

    result = qp->ResultStorage(s);
    MBool* pResult = (MBool*)result.addr;
    Mapping1* reg = (Mapping1*)args[0].addr;
    UBool uBool;
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
        if(TLA_DEBUG){
          cout<<"ureg "<<i<<" [ "
          <<ureg->timeInterval.start.ToDouble()<<" "
          <<ureg->timeInterval.end.ToDouble()<<" "
          <<ureg->timeInterval.lc<<" "<<ureg->timeInterval.rc<<" ] "
          <<endl;}      
        uBool.timeInterval.rc = !ureg->timeInterval.lc;
        uBool.timeInterval.end = ureg->timeInterval.start;
        uBool.constValue.Set(true,true);     
        if(TLA_DEBUG){
          cout<<"a "<<i<<" "<<uBool.constValue.GetBoolval()<<" [ "
          <<uBool.timeInterval.start.ToDouble()<<" "
          <<uBool.timeInterval.end.ToDouble()<<" "
          <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<" ]"
          <<endl;}
        if(uBool.timeInterval.start < uBool.timeInterval.end 
          || (uBool.timeInterval.start == uBool.timeInterval.end
          && uBool.timeInterval.lc && uBool.timeInterval.rc))  
          pResult->MergeAdd(uBool);
        uBool.timeInterval = ureg->timeInterval;
        uBool.constValue.Set(true,false);        
        if(TLA_DEBUG){
          cout<<"b "<<i<<" "<<uBool.constValue.GetBoolval()<<" [ "
          <<uBool.timeInterval.start.ToDouble()<<" "
          <<uBool.timeInterval.end.ToDouble()<<" "
          <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<" ]"
          <<endl;}
        pResult->MergeAdd(uBool); 
        uBool.timeInterval.lc = !ureg->timeInterval.rc;
        uBool.timeInterval.start = ureg->timeInterval.end;
      }
      uBool.timeInterval.end.ToMaximum();
      uBool.timeInterval.end.SetType(instanttype);
      if(ureg->timeInterval.end 
         < uBool.timeInterval.end){
        uBool.timeInterval.rc = true;
        uBool.constValue.Set(true,true);
        if(TLA_DEBUG){
          cout<<uBool.constValue.GetBoolval()<<" [ "
            <<uBool.timeInterval.start.ToDouble()<<" "
          <<uBool.timeInterval.end.ToDouble()<<" "
          <<uBool.timeInterval.lc<<" "<<uBool.timeInterval.rc<<" ]"
          <<endl;}
        pResult->MergeAdd(uBool);;
      }
    }
    pResult->EndBulkLoad(false);
    
    return 0;
}

/*
~MFalseValueMap~ is used in ~inside~ with 
mregion x points / line and creats false for every
periode in mregion.

*/

static int MFalseValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    cout<< "MFalseValueMap() called" << endl;

    result = qp->ResultStorage(s);
    MBool* pResult = (MBool*)result.addr;
    MRegion* reg = (MRegion*)args[0].addr;
    UBool uBool;
    
    pResult->Clear();
    pResult->StartBulkLoad();
    for( int i = 0; i < reg->GetNoComponents(); i++) {
      const URegion *ureg;
      reg->Get(i, ureg);
      uBool.timeInterval = ureg->timeInterval; 
      uBool.constValue.Set(true,false);
      cout<<uBool.constValue.GetBoolval()<<" [ "
      <<uBool.timeInterval.start.ToDouble()<<" "
      <<uBool.timeInterval.end.ToDouble()<<" "
      <<uBool.timeInterval.lc<<" "
      <<uBool.timeInterval.rc<<" ]"<<endl;
      pResult->MergeAdd(uBool);
    }
    pResult->EndBulkLoad(false);
    
    return 0;
}

/*
16.3.40 Value mapping functions of operator ~mint~ 
Every periode in periods will be transformed into an UInt with 
value 1 the holes will be transformed into UInts with value 0.

*/

int TemporalMIntValueMap( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
    cout<<"TemporalMIntValueMap called"<<endl;
    result = qp->ResultStorage( s );
    Periods *pers = (Periods*)args[0].addr;
    MInt *pResult = (MInt*)result.addr;
    const Interval<Instant> *per;
    UInt uInt;
    
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
        
        cout<<"per "<<i<<" [ "
        <<per->start.ToDouble()<<" "
        <<per->end.ToDouble()<<" "
        <<per->lc<<" "<<per->rc<<" ] "
        <<endl;
        
        uInt.timeInterval.rc = !per->lc;
        uInt.timeInterval.end = per->start;
        uInt.constValue.Set(true, 0);
        
        cout<<"a "<<i<<" "<<uInt.constValue.GetIntval()<<" [ "
        <<uInt.timeInterval.start.ToDouble()<<" "
        <<uInt.timeInterval.end.ToDouble()<<" "
        <<uInt.timeInterval.lc<<" "<<uInt.timeInterval.rc<<" ]"
        <<endl;
        if(uInt.timeInterval.start < uInt.timeInterval.end 
          || (uInt.timeInterval.start == uInt.timeInterval.end
          && uInt.timeInterval.lc && uInt.timeInterval.rc))  
          pResult->MergeAdd(uInt);
        uInt.timeInterval = *per;
        uInt.constValue.Set(true, 1);
          
        cout<<"b "<<i<<" "<<uInt.constValue.GetIntval()<<" [ "
        <<uInt.timeInterval.start.ToDouble()<<" "
        <<uInt.timeInterval.end.ToDouble()<<" "
        <<uInt.timeInterval.lc<<" "<<uInt.timeInterval.rc<<" ]"
        <<endl;
        pResult->MergeAdd(uInt);
        
        uInt.timeInterval.lc = !per->rc;
        uInt.timeInterval.start = per->end;
      }
      uInt.timeInterval.end.ToMaximum();
      uInt.timeInterval.end.SetType(instanttype);
      if(per->end < uInt.timeInterval.end){
        uInt.timeInterval.rc = true;
        uInt.constValue.Set(true, 0);
        
        cout<<uInt.constValue.GetIntval()<<" [ "
        <<uInt.timeInterval.start.ToDouble()<<" "
        <<uInt.timeInterval.end.ToDouble()<<" "
        <<uInt.timeInterval.lc<<" "<<uInt.timeInterval.rc<<" ]"
        <<endl;
        pResult->MergeAdd(uInt);;
      }
    }
    pResult->EndBulkLoad(false);
  
  return 0;
}

/*
16.3.40 Value mapping functions of operator ~+~ with two mint-objects

*/

int TemporalPlusValueMap( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  cout<<"TemporalPlusValueMap called"<<endl;
  
  result = qp->ResultStorage( s );
  MInt *op1 = (MInt*)args[0].addr;
  MInt *op2 = (MInt*)args[1].addr;
  MInt *pResult = (MInt*)result.addr;
   
  UInt uInt;  //part of the Result
  
  RefinementPartitionLift<MInt, MInt, UInt, UInt> rp(*op1, *op2);
  cout<<"Refinement abgeschlossen, rp.size: "<<rp.Size()<<endl;
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
    cout<< "+ interval #"<< i<< ": "<< iv->start.ToDouble()<< " "
    << iv->end.ToDouble()<< " "<< iv->lc<< " "<< iv->rc<< " "
    << u1Pos<< " "<< u2Pos<< endl;
    
    if (u1Pos == -1 || u2Pos == -1)     
      continue;  
    else {
      cout<<"Both operators existant in interval iv #"<<i<<endl;
      op1->Get(u1Pos, u1);
      op2->Get(u2Pos, u2);
    }
    
    cout<<"wert 1 "<<u1->constValue.GetIntval()<<endl;
    cout<<"wert 2 "<<u2->constValue.GetIntval()<<endl;
    uInt.timeInterval = *iv;

    uInt.constValue.Set(true,
      u1->constValue.GetIntval() + u2->constValue.GetIntval());
    cout<<"wert "<<uInt.constValue.GetIntval()<<endl;
    cout<<"interval "<<uInt.timeInterval.start.ToDouble()
    <<" "<<uInt.timeInterval.end.ToDouble()<<" "
    <<uInt.timeInterval.lc<<" "<<uInt.timeInterval.rc<<endl;
    
    pResult->MergeAdd(uInt);
  }
  pResult->EndBulkLoad(false);

  return 0;
}

/*
ValueMapping for ~zero~ 

*/

static int TemporalZeroValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    cout<< "TemporalZeroValueMap() called" << endl;

    result = qp->ResultStorage(s);
    MInt* pResult = (MInt*)result.addr;
    
    pResult->Clear();
    pResult->StartBulkLoad();
    UInt uInt;
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
ValueMapping for ~concat~ 

*/

static int TemporalConcatValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    cout<< "TemporalConcatValueMap() called" << endl;
    
    result = qp->ResultStorage(s);
    MPoint* pResult = (MPoint*)result.addr;
    MPoint* p1 = (MPoint*)args[0].addr;
    MPoint* p2 = (MPoint*)args[1].addr;
    const UPoint *up1;
    const UPoint *up2;
    
    if(p1->GetNoComponents() > 0 && p2->GetNoComponents() > 0){
      p1->Get(p1->GetNoComponents() - 1, up1);
      p2->Get(0, up2);
      if(!(up1->timeInterval.end < up2->timeInterval.start
        || (up1->timeInterval.end == up2->timeInterval.start
        && !(up1->timeInterval.rc && up2->timeInterval.lc))))
        cout<<"DefTime of mpoints are not disjunct!"<<
        " Last interval of first mpoint ends after first interval of"
        <<" of second mpoint begins."<<endl;
      assert(up1->timeInterval.end < up2->timeInterval.start
        || (up1->timeInterval.end == up2->timeInterval.start
        && !(up1->timeInterval.rc && up2->timeInterval.lc)));
    }
    
    pResult->Clear();
    pResult->StartBulkLoad();
    for( int i = 0; i < p1->GetNoComponents(); i++) {
      p1->Get(i, up1);
      cout<<"p1 #"<<i<<" [ "
      <<up1->timeInterval.start.ToDouble()<<" "
      <<up1->timeInterval.end.ToDouble()<<" "
      <<up1->timeInterval.lc<<" "
      <<up1->timeInterval.rc<<" ]"<<endl;
      pResult->Add(*up1);
    }
    for( int i = 0; i < p2->GetNoComponents(); i++) {
      p2->Get(i, up2);
      cout<<"p2 #"<<i<<" [ "
      <<up2->timeInterval.start.ToDouble()<<" "
      <<up2->timeInterval.end.ToDouble()<<" "
      <<up2->timeInterval.lc<<" "
      <<up2->timeInterval.rc<<" ]"<<endl;
      pResult->Add(*up2);
    }
    pResult->EndBulkLoad(false);

    return 0;
}

ValueMapping temporalandmap[] = {TemporalMMLogic<1>,
                                 TemporalMSLogic<1>,
                                 TemporalSMLogic<1>};

ValueMapping temporalormap[] = {TemporalMMLogic<2>,
                                TemporalMSLogic<2>,
                                TemporalSMLogic<2>};

ValueMapping temporalmequalmap[] = {
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

ValueMapping temporalmnotequalmap[] = {
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


ValueMapping temporalmlessmap[] =     {
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

ValueMapping temporalmlessequalmap[] =     {   
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

ValueMapping temporalmgreatermap[] =     {
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

ValueMapping temporalmgreaterequalmap[] =    {
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
       
ValueMapping temporaldistancemap[] = {MPointMMDistance,
                                      //MPointMSDistance,
                                      //MPointSMDistance,
                                      MRealMMDistance,
                                      MRealMSDistance,
                                      MRealSMDistance};

ValueMapping temporalliftintersectionmap[] = {
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
            TemporalSMIntersection<MString, UString, CcString, 1>
            };

ValueMapping temporalliftminusmap[] = {
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

ValueMapping temporalliftinsidemap[] = {
            MPointPointsInside<1>,
            MPointLineInside<1>,
            MFalseValueMap,
            MFalseValueMap  };

static ValueMapping unionvaluemap[] =
    { MPRUnionValueMap,
      MPMRUnionValueMap,
      PMRUnionValueMap
      }; 

static ValueMapping temporalliftisemptyvaluemap[] =
    { IsEmptyValueMap<MRegion, URegion>,
      IsEmptyValueMap<MBool, UBool>,
      IsEmptyValueMap<MInt, UInt>,
      IsEmptyValueMap<MReal, UReal>,
      IsEmptyValueMap<MPoint, UPoint>,
      IsEmptyValueMap<MString, UString>
      }; 


const string TemporalLiftSpecNot 
                  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>mbool -> mbool</text--->"
                         "<text> not( _ )</text--->"
                         "<text>Negates a MovingBool.</text--->"
                         "<text>not(mb1)</text--->"
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
            "( <text>T in {bool, int, real, string},  mT x T -> mbool,"
            " (mT x mT) -> mbool, (T x mT) -> mbool</text--->"
            "<text>_ < _</text--->"
            "<text>Less than.</text--->"
            "<text>query i1 < i2</text--->"
            ") )";

const string TemporalLiftSpecLE  = 
            "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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


const string TemporalLiftSpecIntersection  
        = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
          "\"Example\" ) "
          "( <text>S in {bool, int, string, real}, T in {line, points),"
          " mS x mS -> mS, mS x S -> mS, S x mS -> mS, "
          "mpoint x T -> mpoint, T x mpoint -> mpoint</text--->"
          "<text>intersection( _, _ )</text--->"
          "<text>Intersection.</text--->"
          "<text>query intersection (mi1,  mi2)</text--->"
          ") )";

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

const string TemporalLiftSpecABS  
                 = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                   "\"Example\" ) "
                   "( <text>mreal -> mreal</text--->"
                   "<text>abs( _ )</text--->"
                   "<text>abs</text--->"
                   "<text>query abs(mr1)</text--->"
                   ") )";

const string TemporalLiftSpecDistance 
      = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
        "\"Example\" ) "
        "( <text>T in {real), mpoint x mpoint -> mreal, mT x mT"
        " -> mreal, mT x T -> mreal, T x mT -> mreal</text--->"
        "<text> distance( _, _ ) </text--->"
        "<text>returns the moving distance</text--->"
        "<text>distance( mpoint1, point1 )</text--->"
        ") )";

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

static const string perimeterspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>movingregion -> mreal</text--->"
    "    <text>perimeter ( _ )</text--->"
    "    <text>Calculates the perimeter of a moving Region.</text--->"
    "    <text>mraperimeter(mrg1)</text---> ) )";

static const string areaspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>movingregion -> mreal</text--->"
    "    <text>area ( _ )</text--->"
    "    <text>Calculates the area of a moving Region.</text--->"
    "    <text>area(mrg1)</text---> ) )";    

static const string rcenterspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>movingregion -> mpoint</text--->"
    "    <text>rough_center ( _ )</text--->"
    "    <text>Calculates an approach to the"
    "  center of gravity of a moving Region.</text--->"
    "    <text>rough_center(mrg1)</text---> ) )";

static const string ncomponentsspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>movingregion -> mint</text--->"
    "    <text>no_components ( _ )</text--->"
    "    <text>Calculates the number of faces of a moving Region.</text--->"
    "    <text>no_components(mrg1)</text---> ) )";


static const string unionspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>R in {region}, P in {point}, mP x R-> mR,"
    " mP x mR -> mR, P x mR -> mR</text--->"
    "    <text>_ union _</text--->"
    "    <text>Calculates union between the given objects.</text--->"
    "    <text>rg1 union mp1</text---> ) )";

static const string temporalliftisemptyspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>T in {bool, int, string, real, point, region}"
    "    mT -> mbool</text--->"
    "    <text>isempty( _ )</text--->"
    "    <text>Checks if the m. object is not defined .</text--->"
    "    <text>isempty(mrg1)</text---> ) )";

static const string temporalmintspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>periods -> mint</text--->"
    "    <text>mmint( _ )</text--->"
    "    <text>Creats a MInt from a periods-object with value 1"
    " for each existing period and 0 for every hole.</text--->"
    "    <text>mmint(per1)</text---> ) )";

static const string temporalplusspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>mint x mint -> mint</text--->"
    "    <text>_ + _</text--->"
    "    <text>Adds two mint-obects when both are existant</text--->"
    "    <text>mi1 + mi2</text---> ) )";

static const string temporalzerospec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>      -> mint</text--->"
    "    <text>zero()</text--->"
    "    <text>Creates an mint-Object with value 0 from minInstant"
    " to maxInstant</text--->"
    "    <text>zero()</text---> ) )";

static const string temporalconcatspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>mpoint x mpoint -> mpoint</text--->"
    "    <text>_ _ concat</text--->"
    "    <text>Concats two MPoints. DefTime of the first mpoint must"
    " end before deftime of second point!</text--->"
    "    <text>mp1 mp2 concat</text---> ) )";


Operator temporalnot( "not",
                            TemporalLiftSpecNot,
                            TemporalNot,
                            Operator::SimpleSelect,
                            MBoolTypeMapMBool );

Operator temporaland( "and",
                            TemporalLiftSpecAnd,
                            3,
                            temporalandmap,
                            MovingAndOrSelect,
                            AndOrTypeMapMBool );

Operator temporalor( "or",
                            TemporalLiftSpecOr,
                            3,
                            temporalormap,
                            MovingAndOrSelect,
                            AndOrTypeMapMBool );

Operator temporalmequal( "=",
                            TemporalLiftSpecMEqual,
                            18,
                            temporalmequalmap,
                            MovingEqualSelect,
                            MovingEqualTypeMapMBool );

Operator temporalmnotequal( "#",
                            TemporalLiftSpecMNotEqual,
                            18,
                            temporalmnotequalmap,
                            MovingEqualSelect,
                            MovingEqualTypeMapMBool );

Operator temporalmless ( "<",
                            TemporalLiftSpecLT,
                            12,
                            temporalmlessmap,
                            MovingCompareSelect,
                            MovingCompareTypeMapMBool );

Operator temporalmlessequal ( "<=",
                            TemporalLiftSpecLE,
                            12,
                            temporalmlessequalmap,
                            MovingCompareSelect,
                            MovingCompareTypeMapMBool );

Operator temporalmgreater ( ">",
                            TemporalLiftSpecGT,
                            12,
                            temporalmgreatermap,
                            MovingCompareSelect,
                            MovingCompareTypeMapMBool );

Operator temporalmgreaterequal ( ">=",
                            TemporalLiftSpecGE,
                            12,
                            temporalmgreaterequalmap,
                            MovingCompareSelect,
                            MovingCompareTypeMapMBool );

Operator temporalmdistance( "distance",
                           TemporalLiftSpecDistance,
                           4,
                           temporaldistancemap,  
                           MovingDistanceSelect,
                           MovingDistanceTypeMapMReal );

Operator temporalmintersection( "intersection",
                 TemporalLiftSpecIntersection,
                 16,
                 temporalliftintersectionmap,
                 MovingIntersectionSelect,
                 MovingIntersectionTypeMap );

Operator temporalmminus( "minus",
                        TemporalLiftSpecMinus,
                        20,  
                        temporalliftminusmap,
                        MovingMinusSelect,
                        MovingMinusTypeMap );              
                        
Operator temporalabs( "abs",
                            TemporalLiftSpecABS,
                            MovingRealABS,
                            Operator::SimpleSelect,
                            ABSTypeMap );

Operator temporalminside( "inside",
                         TemporalLiftSpecInside,
                         4,
                         temporalliftinsidemap,
                         InsideSelect,
                         InsideTypeMapMBool );

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

static Operator munion("union",
                          unionspec,
                          3,
                          unionvaluemap,
                          UnionSelect,
                          UnionTypeMap);

static Operator isempty("isempty",
                        temporalliftisemptyspec,
                        6,
                        temporalliftisemptyvaluemap,
                        TemporalLiftIsemptySelect,
                        TemporalLiftIsemptyTypeMap);

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
                        
static Operator temporalzero("zero",
                        temporalzerospec,
                        TemporalZeroValueMap,
                        Operator::SimpleSelect,
                        TemporalZeroTypeMap);
                        
static Operator temporalconcat("concat",
                        temporalconcatspec,
                        TemporalConcatValueMap,
                        Operator::SimpleSelect,
                        TemporalConcatTypeMap);

class TemporalLiftedAlgebra : public Algebra
{
  public:
    TemporalLiftedAlgebra() : Algebra()
    {
    
    AddOperator( &temporalmless);
    AddOperator( &temporalmlessequal);
    AddOperator( &temporalmgreater);
    AddOperator( &temporalmgreaterequal);
    AddOperator( &temporalmdistance);
    AddOperator( &temporalmintersection);
    AddOperator( &temporalmminus);
    AddOperator( &temporalabs);
    AddOperator( &temporalminside);
    
    AddOperator( &temporalnot );
    AddOperator( &temporaland );
    AddOperator( &temporalor );
    AddOperator( &temporalmequal );
    AddOperator( &temporalmnotequal );
    AddOperator( &temporalabs );
    
    AddOperator( &perimeter);
    AddOperator( &area);
    AddOperator( &rcenter);
    AddOperator( &ncomponents);
    AddOperator( &munion);
    AddOperator( &isempty);
    
    AddOperator( &temporalmint);
    AddOperator( &temporalplus);
    AddOperator( &temporalzero);
    AddOperator( &temporalconcat);
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


