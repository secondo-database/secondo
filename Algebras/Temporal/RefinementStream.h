
/*
----
This file is part of SECONDO.

Copyright (C) 2010, University in Hagen, Department of Computer Science,
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


*/

#ifndef REFINEMENTSTREAM_H
#define REFINEMENTSTREAM_H



#include "TemporalAlgebra.h"
#include <assert.h>

/*
1 Class RefinementStream


This class is a reimplementation of the RefinementPartition class
from the TemporalAlgebra. In contrast to the original implementation
this implementation does not allow random access to the __units__
of the partitions. Only a sequential run is possible. That is sufficient
in the most cases.  The big advantage of this implementation is that 
it does not require a precomputation of the complete partition. 
This will save memory and can also save runtime because not the 
complete arguments must be processed.

*/

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
class RefinementStream{

public:
/*
1.1 Constructor

Creates a new RefinementStream of the arguments. 
If one of the arguments is not defined, no result will
be returned.

*/

  RefinementStream(const Mapping1* m1, const Mapping2* m2);

  RefinementStream(const Unit1* _u1, const Mapping2* _m2):
    m1(0), m2(_m2), unit1(_u1), unit2(0) {
    reset();
  }

  RefinementStream(const Mapping1* _m1, const Unit2* _u2):
    m1(_m1), m2(0), unit1(0), unit2(_u2)
  {
    reset();
  }

  RefinementStream(const Unit1* _u1, const Unit2* _u2):
     m1(0), m2(0), unit1(_u1), unit2(_u2){
     reset();
  }


/*
1.2 ~Reset~

Brings this structure to the same state as after the
initialisation.

*/


  void reset();


/*
1.3 ~hasNext~

Checks whether more elements can be returned.

*/

  bool hasNext() const;

/*
1.4 ~getNext~

Returns the next interval and the corresponding positions
from the arguments. If there is no next exlemnt, the result will
be __false__, otherwise __true__. 


*/

  bool getNext(Interval<Instant>& iv, 
               int& pos1,
               int& pos2);


/*
1.5 ~finished1~

Returns true if all units of object 1 was processed.

*/

   bool finished1() const{
     return pos1<0;
   }

/*
1.6 ~finished2~

Returns true if all units of object 2 was processed.

*/

  bool finished2() const{
     return pos2<0;
  }
private:

  const Mapping1*  m1;
  const Mapping2*  m2;
  const Unit1* unit1;
  const Unit2* unit2;
  int        pos1;
  int        pos2;
  bool       done;
  Interval<Instant> iv1;
  Interval<Instant> iv2;


  void loadNext1(bool first=false){
    if(first){
      pos1=0;
    } else {
       assert(pos1>=0);
       pos1++;
    }
    int noComps1;
    if(m1){
      noComps1 = m1->GetNoComponents();
    } else {
      noComps1 = unit1->IsDefined()?1:0;
    }
    if(pos1< noComps1){
      Unit1 u1;
      if(m1){
         m1->Get(pos1,u1);
      } else {
         u1 = *unit1;
      }
      iv1 = u1.getTimeInterval();
    } else {
      pos1 = -1;
      done = pos2 < 0;
    }
  }

  void loadNext2(bool first = false){
    if(first){
      pos2 = 0;
    } else {
      assert(pos2>=0);
      pos2++;
    }
    int noComps2;
    if(m2){
      noComps2 = m2->GetNoComponents();
    } else {
      noComps2 = unit2->IsDefined()?1:0;
    }

    if(pos2< noComps2){
      Unit2 u2;
      if(m2){
         m2->Get(pos2,u2);
      } else {
         u2 = *unit2;
      }
      iv2 = u2.getTimeInterval();
    } else {
      pos2 = -1;
      done = pos1 <0;
    }
  }

};


/*
1.5 Implementation

*/

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
RefinementStream<Mapping1, Mapping2, Unit1, Unit2>::RefinementStream(
       const Mapping1* _m1, const Mapping2* _m2): 
          m1(_m1), m2(_m2), unit1(0), unit2(0)    {
   reset();
}


template<class Mapping1, class Mapping2, class Unit1, class Unit2>
void RefinementStream<Mapping1, Mapping2, Unit1, Unit2>::reset(){
   if((!m1 && !unit1) || (!m2 && !unit2)){   // null parameter
     done = true;
     return;
   }

   if( (m1 && !m1->IsDefined())  ||     // arg1 has elements
       (unit1 && !unit1->IsDefined()) ) {
      pos1 = -1;
   }  else {
      loadNext1(true); 
   }

   if( (m2 && !m2->IsDefined()) ||
       (unit2 && !unit2->IsDefined())){
     pos2 = -1;
   } else {
      loadNext2(true);
   }
   done = (pos1<0) && (pos2<0);
}


template<class Mapping1, class Mapping2, class Unit1, class Unit2>
bool RefinementStream<Mapping1, Mapping2, Unit1, Unit2>::hasNext() const{
  return !done;
}


template<class Mapping1, class Mapping2, class Unit1, class Unit2>
bool RefinementStream<Mapping1, Mapping2, Unit1, Unit2>::getNext(
   Interval<Instant>& _iv, int& _pos1, int& _pos2) {
  if(done){
    return false;
  }
  if(pos1 < 0){ // m1 is completely processed
     _iv = iv2;
     _pos1 = -1;
     _pos2 = pos2;
     loadNext2();
     return true;
  } 
  
  if(pos2 < 0){ // m2 is finished
     _iv = iv1;
     _pos1 = pos1;
     _pos2 = -1;
     loadNext1();
     return true;
  }


  // both arguments have units
  if(iv1.start < iv2.start){
    _iv = iv1;  // can be changed later, but start properties are equal
    _pos1 = pos1;
    _pos2 = -1;
    if(iv1.end < iv2.start){ // iv1 before iv2
      loadNext1();
      return true;
    } else if(iv1.end > iv2.start){
      // overlapping intervals
      _iv.end = iv2.start;
      _iv.rc  =  !iv2.lc;
      iv1.start = iv2.start;
      iv1.lc = iv2.lc;
      return true;
    } else { // u1.timeInterval.end == u2.timeInterval.start
      if( !iv1.rc  || !iv2.lc){ // iv1 before iv2
        loadNext1();
        return true;
      } else { // intervals have a common instant
        _iv.rc = false;
        iv1.lc = true;
        iv1.start = iv2.start; 
        return true;
      }
    }
  } else if(iv2.start < iv1.start){ // iv2 start before iv1
    _iv = iv2;
    _pos1 = -1;
    _pos2 = pos2;
   if(iv2.end < iv1.start){ // iv2 before iv1
     loadNext2();
     return true;
   } else if(iv2.end > iv1.start){
     _iv.end = iv1.start;
     _iv.rc  = !iv1.lc;
     iv2.start = iv1.start;
     iv2.lc = iv1.lc; 
     return true;
   } else { // u1.timeInterval.end == u2.timeInterval.start
     if(!iv2.rc || !iv1.lc){ // iv2 before iv1
       loadNext2();
       return true;
     } else { // common instant at the end of iv2
       _iv.rc = false;
       iv2.start = iv1.start;
       iv2.lc = true;
       return true;
     }
 
   }
 } else { // u1.timeInterval.start == u2.timeInterval.start
   if(iv1.lc != iv2.lc){ // start instant belongs only to a single interval
      if(iv1.lc){  // start belongs to iv1
        _iv = iv1;
        _iv.end = iv1.start;
        _iv.rc  = true;
        _pos1 = pos1;
        _pos2 = -1;
        if(iv1.start==iv1.end){
           loadNext1();
        } else {
          iv1.lc = false;
        }
        return true;
      } else { // start belongs to iv2
       _iv = iv2;
       iv2.end = iv2.start;
       _iv.rc = true;
       _pos1 = -1;
       _pos2 = pos2;
       if(iv2.start == iv2.end){
         loadNext2();
       } else {
          iv2.lc = false;
       }
       return true;
      }
   } else { // both intervals have the same start instant
     _iv = iv1;
     _pos1 = pos1;
     _pos2 = pos2;
     if(iv1.end < iv2.end){
        _iv.end = iv1.end; // shorten _iv
        _iv.rc = iv1.rc;
        iv2.lc = !iv1.rc;    // shorten iv2
        iv2.start = iv1.end;
        loadNext1();
        return true;
     } else if(iv2.end < iv1.end){
        _iv.end = iv2.end;
        _iv.rc = iv2.rc;
        iv1.start=iv2.end;
        iv1.lc = !iv2.rc;
        loadNext2();
        return true;
     } else {  // both end at the same instant
       if(iv1.rc == iv2.rc){ // exacly the same intervals
         loadNext1();
         loadNext2();
         return true;
       } else {  // the last instant only belongs to one interval
         _iv.rc = false;
         if(iv1.rc){
            iv1.lc = true;
            iv1.start = iv1.end;
            loadNext2();
            return true;
         } else { // iv2.rc
            iv2.lc = true;
            iv2.start = iv2.end;
            loadNext1();
            return true;
         }

       }
     }
   }
 }  

} // getNext(...)



#endif


