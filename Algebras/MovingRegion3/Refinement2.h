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
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

[1] File Refinement2.h

This file contains classes and functions for use within the operators intersection and inside

[TOC]

1 Helper classes and methods

1.1 Some forward declaration of helper methods

*/

static mpq_class instant2MPQ( const Instant i );
static void precTimeToInstant(const mpq_class prectime, 
                              Instant& timeValue, 
                              mpq_class& restValue);

/*
1.1 Precise Time Interval class ~precTimeInterval~

used in RefinementPartition2

*/
class precTimeInterval {

public:
  mpq_class start;
  mpq_class end;
  bool lc;
  bool rc;
  
  inline precTimeInterval():
      start(0),
      end(0),
      lc(false),
      rc(false) {}

  inline precTimeInterval(mpq_class s, mpq_class e, bool l, bool r) :
      start(s),
      end(e),
      lc(l),
      rc(r) {}

  inline precTimeInterval(Interval<Instant> t): 
      start(instant2MPQ(t.start)),
      end(instant2MPQ(t.end)),
      lc(t.lc),
      rc(t.rc) {}

  inline precTimeInterval(Interval<Instant> t, PreciseInterval p, 
                          const DbArray<unsigned int>* preciseInstants): 
      start(0),
      end(0),
      lc(t.lc),
      rc(t.rc)
  {
    start = p.GetPreciseInitialInstant(preciseInstants);
    start = start + instant2MPQ(t.start);
    start.canonicalize();
    
    end = p.GetPreciseFinalInstant(preciseInstants);
    end = end + instant2MPQ(t.end);
    end.canonicalize();
  }
  
  inline bool operator==(precTimeInterval pti) const
  {
    return (lc==pti.lc) && (rc==pti.rc) 
            && (cmp(start, pti.start) == 0) && (cmp(end, pti.end) == 0);
  }
};


/*
1.1 Precise UPoint class ~precUPoint~

holds the precise values calculated within the set operations

*/
class precUPoint {
  
public:
  precTimeInterval pti;
  mpq_class x0;
  mpq_class y0;
  mpq_class x1;
  mpq_class y1;
  UPoint up;
  
  inline precUPoint():
      x0(0),
      y0(0),
      x1(0),
      y1(0),
      up(0) { }

  inline precUPoint(precTimeInterval p, mpq_class px0, mpq_class py0, 
                    mpq_class px1, mpq_class py1, UPoint pp):
      pti(p.start, p.end, p.lc, p.rc),
      x0(px0),
      y0(py0),
      x1(px1),
      y1(py1),
      up(pp) { }

  inline precUPoint(mpq_class s, mpq_class e, bool l, bool r, mpq_class px0, 
                    mpq_class py0, mpq_class px1, mpq_class py1, UPoint pp):
      pti(s, e, l, r),
      x0(px0),
      y0(py0),
      x1(px1),
      y1(py1),
      up(pp) { }
  
};


/*
1.1 Precise UBool class ~precUBool~

holds the precise values calculated within the set operations

*/
class precUBool {
  
public:
  precTimeInterval pti;
  bool status;
  
  inline precUBool():
      status(false) { }

  inline precUBool(precTimeInterval p, bool st):
      pti(p.start, p.end, p.lc, p.rc),
      status(st) { }

  inline precUBool(mpq_class s, mpq_class e, bool l, bool r, bool st):
      pti(s, e, l, r),
      status(st) { }
  
};


/*
1 Class ~RefinementPartition2~

for set operations inside and intersection with MPoint and MRegion2

1.1 Class definition 

*/

class RefinementPartition2 {
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
    vector<precTimeInterval> iv;
    vector<int> vur;
    vector<int> vup;

/*
~AddUnit()~ is a small helper method to create a new interval from
~start~ and ~end~ instant and ~lc~ and ~rc~ flags and to add these to the
~iv~, ~vur~ and ~vup~ vectors.

*/
    void AddUnits(const precTimeInterval pti, const int urPos, 
                  const int upPos);
    void AddUnits(const int urPos, const int upPos, const mpq_class start, 
                  const mpq_class end, const bool lc, const bool rc);

public:
/*
The constructor creates the refinement partition from the two ~Mapping~
instances ~mr~ and ~mp~.

Runtime is $O(\max(n, m))$ with $n$ and $m$ the numbers of units in
~mr~ and ~mp~.

*Preconditions*: mr.IsDefined AND mp.IsDefiened()

*/
    RefinementPartition2(const MRegion2& mr, const MPoint& mp);

/*
Since the elements of ~iv~ point to dynamically allocated objects, we need
a destructor.

*/
    ~RefinementPartition2() {}

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
    void Get(const unsigned int pos, precTimeInterval& civ, int& ur, int& up);
};

/*
1.1 Class implementation

1.1.1 The constructor ~RefinementPartition2~

*/
RefinementPartition2::RefinementPartition2( const MRegion2& m1, 
                                            const MPoint& m2 )
{
   assert( m1.IsDefined() );
   assert( m2.IsDefined() );

   iv.clear();
   vur.clear();
   vup.clear();

   REF_DEBUG("RefinedmentPartition called ");
   int no1 = m1.GetNoComponents();
   int no2 = m2.GetNoComponents();
   if(no1 + no2 == 0){ // both mappings are empty
      REF_DEBUG("empty mappings");
      return;
   }
   if(no2==0){  // m2 is empty
     REF_DEBUG("m2 is empty");
     iv.reserve(no1);
     vur.reserve(no1);
     vup.reserve(no1);
     URegionEmb2 u1;
     for(int i=0;i<no1;i++){
       m1.Get(i,u1);
       precTimeInterval pi(u1.timeInterval, u1.pInterval, 
                           m1.GetPreciseInstants());
       AddUnits(pi, i,-1);
     }
     return;
   }
   if(no1==0){   // m1 is empty
     REF_DEBUG("m1 is empty " );
     iv.reserve(no2);
     vur.reserve(no2);
     vup.reserve(no2);
     UPoint u2;
     for(int i=0;i<no2;i++){
       m2.Get(i,u2);
       precTimeInterval pi(u2.timeInterval);
       AddUnits(pi,-1,i);
     }
     return;
   }

   // both arguments are non-empty
   int maxsize = (no1 + no2 + 2) * 2;
   iv.reserve(maxsize);
   vur.reserve(maxsize);
   vup.reserve(maxsize);
   URegionEmb2 u1p;
   UPoint u2p;
   m1.Get(0,u1p);
   m2.Get(0,u2p);
   // create editable units from the constant ones
   precTimeInterval t1(u1p.timeInterval, u1p.pInterval, 
                       m1.GetPreciseInstants());
   precTimeInterval t2(u2p.timeInterval);

   int pos1 = 0;
   int pos2 = 0;

   REF_DEBUG("both arguments are non-empty");
   REF_DEBUG("no1 = " << no1 );
   REF_DEBUG("no2 = " << no2 );

   while( (pos1<no1) && (pos2<no2) ){
     REF_DEBUG("pos1 = " << pos1 );
     REF_DEBUG("pos2 = " << pos2 );
     REF_DEBUG("t1 = " << t1);
     REF_DEBUG("t2 = " << t2);

     // both arguments have units
     if(cmp(t1.start, t2.start) < 0) {
       REF_DEBUG("case 1: t1 starts before t2 " );
       // t1 starts before t2
       if(cmp(t1.end, t2.start) < 0){ // t1 before t2
         REF_DEBUG("case 1.1: t1 ends before t2 starts" );
         AddUnits(t1, pos1, -1);
         pos1++;
         if(pos1 < no1){
           m1.Get(pos1, u1p);
           t1 = precTimeInterval(u1p.timeInterval, u1p.pInterval, 
                                 m1.GetPreciseInstants());
         }
       } else if(cmp(t1.end, t2.start) > 0){
         REF_DEBUG("case 1.2: t1 ends after t2 starts" );
         // overlapping intervals
         AddUnits(pos1, -1, t1.start, t2.start, t1.lc, !t2.lc);
         t1.start = t2.start;
         t1.lc = t2.lc;
       } else { // u1.timeInterval.end == u2.timeInterval.start
         REF_DEBUG("case 1.3: t1 ends when t2 starts ");
         if( !t1.rc  || !t2.lc){
           REF_DEBUG("case 1.3.1: t1 ends before t2 starts (closeness) " );
           // u1 before u2
           AddUnits(t1, pos1, -1);
           pos1++;
           if(pos1 < no1){
             m1.Get(pos1,u1p);
             t1 = precTimeInterval(u1p.timeInterval, u1p.pInterval, 
                                   m1.GetPreciseInstants());
           }
         } else { // intervals have a common instant
           REF_DEBUG("case 1.3.2: t2 ends when t2 starts (common instant)");
           AddUnits(pos1, -1, t1.start, t1.end, t1.lc, false);
           t1.lc = true;
           t1.start = t2.start;
         }
       }
     } else if(cmp(t2.start, t1.start) < 0){
        REF_DEBUG("case 2: t2 starts before t1 starts" );
        // symmetric case , u2 starts before u1
       if(cmp(t2.end, t1.start) < 0){ // u2 before u1
         REF_DEBUG("case 2.1: t2 ends before t1 ends ");
         AddUnits(t2, -1, pos2);
         pos2++;
         if(pos2 < no2){
           m2.Get(pos2,u2p);
           t2 = precTimeInterval(u2p.timeInterval);
         }
       } else if(cmp(t2.end, t1.start) > 0){
         REF_DEBUG("case 2.2: t2 ends after t1 starts");
         // overlapping intervals
         AddUnits(-1, pos2, t2.start, t1.start, t2.lc, !t1.lc);
         t2.start = t1.start;
         t2.lc = t1.lc;
       } else { // u1.timeInterval.end == u2.timeInterval.start
         REF_DEBUG("case 2.3: t2 ends when t1 starts" );
         if( !t2.rc  || !t1.lc){
           REF_DEBUG("case 2.3.1: t2 ends before t1 starts (closeness)" );
           // u1 before u2
           AddUnits(t2, -1, pos2);
           pos2++;
           if(pos2 < no2){
             m2.Get(pos2,u2p);
             t2 = precTimeInterval(u2p.timeInterval);
           }
         } else { // intervals have a common instant
           REF_DEBUG("case 2.3.2: t2 ends when t1 starts (common instant)");
           AddUnits(-1, pos2, t2.start, t2.end, t2.lc, false);
           t2.lc = true;
           t2.start = t1.start;
         }
       }
     }  else { // u1.timeInterval.start == u2.timeInterval.start
       REF_DEBUG("case 3: t1 and t2 start at the same instant" );
       // both intervals start at the same instant
       if(t1.lc != t2.lc){
         REF_DEBUG("case 3.1: membership of the instant differs");
         if(t1.lc){ // add a single instant interval
            AddUnits(pos1, -1, t1.start, t1.start, true,true);
            if(t1.start == t1.end){ // u1 exhausted
              pos1++;
              if(pos1< no1){
                m1.Get(pos1,u1p);
                t1 = precTimeInterval(u1p.timeInterval, u1p.pInterval, 
                                      m1.GetPreciseInstants());
              }
            } else {
              t1.lc = false;
            }
         } else {
            // symmetric case
            AddUnits(-1, pos2, t2.start, t2.start, true, true);
            if(cmp(t2.start, t2.end) == 0){
              pos2++;
              if(pos2 < no2){
                 m2.Get(pos2, u2p);
                 t2 = precTimeInterval(u2p.timeInterval);
              }
            } else {
               t2.lc = false;
            }
         }
       } else { // intervals start exact at the same instant
         REF_DEBUG("case 3.2: intervalls start exact together");
         if(cmp(t1.end, t2.end) < 0){
            REF_DEBUG("case 3.2.1: t1 ends before t2 ends");
            AddUnits(t1, pos1, pos2);
            t2.start = t1.end;
            t2.lc = !t1.rc;
            pos1++;
            if(pos1<no1){
              m1.Get(pos1,u1p);
              t1 = precTimeInterval(u1p.timeInterval, u1p.pInterval, 
                                    m1.GetPreciseInstants());
            }
         } else if (cmp(t2.end, t1.end) < 0){
            REF_DEBUG("case 3.2.2: t2 ends before t1 ends" );
            AddUnits(t2, pos1, pos2);
            t1.start = t2.end;
            t1.lc = !t2.rc;
            pos2++;
            if(pos2 < no2){
              m2.Get(pos2,u2p);
              t2 = precTimeInterval(u2p.timeInterval);
            }
         } else { // both units end at the same instant
            REF_DEBUG("case 3.2.3: both intervals ends at the same instant");
            if(t1.rc == t2.rc){  // equal intervals
              REF_DEBUG("case 3.2.3.1: intervals are equal" );
              AddUnits(t1, pos1, pos2);
              pos1++;
              if(pos1 < no1){
                m1.Get(pos1,u1p);
                t1 = precTimeInterval(u1p.timeInterval, u1p.pInterval, 
                                      m1.GetPreciseInstants());
              }
              pos2++;
              if(pos2 < no2){
                m2.Get(pos2, u2p);
                t2 = precTimeInterval(u2p.timeInterval);
              }
            } else {
              REF_DEBUG("case 3.2.3.2: intervals differ at right closeness");
              // process common part
              AddUnits(pos1,pos2,t1.start, t1.end, t1.lc, false);
              if(t1.rc){
                 pos2++;
                 if(pos2<no2){
                   m2.Get(pos2,u2p);
                   t2 = precTimeInterval(u2p.timeInterval);
                 }
                 t1.lc = true;
                 t1.start = t1.end;
              } else {
                 pos1++;
                 if(pos1 < no1){
                   m1.Get(pos1,u1p);
                   t1 = precTimeInterval(u1p.timeInterval, u1p.pInterval, 
                                         m1.GetPreciseInstants());
                 }
                 t2.lc = true;
                 t2.start = t2.end;
              }
            }
         }
       }
     }
   }

   REF_DEBUG("one of the arguments is finished");

   // process remainder of m1
   while(pos1 < no1){
     AddUnits(t1, pos1, -1);
     pos1++;
     if(pos1<no1){
       m1.Get(pos1,u1p);
       t1 = precTimeInterval(u1p.timeInterval, u1p.pInterval, 
                             m1.GetPreciseInstants());
     }
   }
   // process remainder of m2
   while(pos2 < no2){
     AddUnits(t2, -1, pos2);
     pos2++;
     if(pos2<no2){
        m2.Get(pos2,u2p);
        t2 = precTimeInterval(u2p.timeInterval);
     }
   }
}

/*
1.1.1 ~AddUnits~

*/
void RefinementPartition2::AddUnits(const precTimeInterval pti, 
                                    const int urPos, const int upPos)
{
  assert(urPos!=-1 || upPos!=-1);

  if ((cmp(pti.start, pti.end) == 0) && !(pti.lc && pti.rc)) //invalid interval
  {
      return;
  }

  iv.push_back(pti);
  vur.push_back(urPos);
  vup.push_back(upPos);
}

void RefinementPartition2::AddUnits(const int urPos, const int upPos, 
                                    const mpq_class start, const mpq_class end, 
                                    const bool lc, const bool rc)
{
  assert(urPos!=-1 || upPos!=-1);

  if ((cmp(start, end) == 0) && !(lc && rc))  // invalid interval
  {
      return;
  }

  precTimeInterval pti(start, end, lc, rc);

  iv.push_back(pti);
  vur.push_back(urPos);
  vup.push_back(upPos);
}

/*
1.1.1 ~Size~

*/
unsigned int RefinementPartition2::Size(void)
{
  return iv.size();
}

/*
1.1.1 ~Get~

*/
void RefinementPartition2::Get(const unsigned int pos, precTimeInterval& civ, 
                               int& ur, int& up)
{
  assert(pos < iv.size());
  civ = iv[pos];
  ur = vur[pos];
  up = vup[pos];
}
