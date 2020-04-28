/*

----
This file is part of SECONDO.

Copyright (C) 2020,
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


This files realizes a balanced dsitribution of slots to workers.


*/

#include "BalancedCollect.h"

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <queue>
#include <assert.h>

using namespace std;

/*
~class Slot~

A slot is represented by its number and the size.

*/

class Slot{
public:
  Slot(size_t _no, size_t _size): no(_no), size(_size) {}
  size_t getNo() const { return no; }
  size_t getSize() const { return size;}
  string toString() const{
    stringstream ss;
    ss << no <<"(" << size << ")";
    return ss.str();
  }

  static bool sizeGreater(const Slot& s1, const Slot& s2) {
    return s1.size > s2.size;
  }


private:
    size_t no;
    size_t size;  
};

std::ostream& operator<<(ostream& o, const Slot& s){
  return  o << s.toString();
}

/*
Class representing a DWorker.

A DWorker is represented as a vector of Slots 
contained by this worker.

*/

class DWorker{
   public:
       DWorker(int _no) : no(_no),size(0) {}

       void add (Slot slot){
         slots.push_back(slot);
         size += slot.getSize();
       }

       Slot removeTop(){
          if(slots.empty()) return Slot(-1,-1);
          Slot res = slots.back();
          slots.pop_back();
          size -= res.getSize();
          return res; 
       }

       string toString() const{
         stringstream ss;
         for(size_t i=0;i<slots.size(); i++){
           if( i > 0) ss << " + " ;
           ss << slots[i].toString();
         }
         ss << " => " <<  size ;
         return ss.str();
       }

       string sizeString() const{
         stringstream ss;
         for(size_t i=0;i<slots.size(); i++){
           if( i > 0) ss << " + " ;
           ss << slots[i].getSize();
         }
         ss << " => " <<  size ;
         return ss.str();
       }


       size_t getNoSlots() const { return slots.size(); }
       size_t getNoElements() const { return size; } 

       size_t getId () const{ return no; } 

       const vector<Slot>& getSlots() const { return slots; }

   private:
      size_t no;      // identifikator of this worker 
      size_t size;    // summarized size of all slots
      vector<Slot> slots;
};


/*
~wless~

Comparator for DWorkers.
Returns true if the left DWorker has more elements than the right one.

*/

class wless {
  public:
     bool operator()(const DWorker& left, const DWorker& right){
         return left.getNoElements()< right.getNoElements();
     }
};

class wgreater{ 
  public:
     bool operator()(const DWorker& left, const DWorker& right){
         return left.getNoElements() > right.getNoElements();
     }
};

/*
~SimpleDist~

Implementation of a Distributor.

Slots are sorted by decreasinge size. 
Then it iteratet over the sorted slots. The next slot is assigned to
that DWorker having the smallest amount of elements.

*/

class Distributor{
  public: 
     vector<DWorker> distributeSimple(vector<Slot>& slots, 
                                int noDWorkers) {
        return distribute(slots, noDWorkers, 0);
     }

     vector<DWorker> distributeComplete(vector<Slot>& slots,
                                        int noDWorkers){
        return distribute(slots, noDWorkers, getNoReserve(noDWorkers));
     }

private :
    virtual vector<DWorker> distribute(vector<Slot>& slots, 
                                       int noDWorkers, 
                                       int noReserve) {
        // sort slots in descending order
        sort(slots.begin(), slots.end(), Slot::sizeGreater);
        // create completely empty dworkers (full set)
        DWorkers.clear();
        for(int i=0;i<noDWorkers;i++){
           DWorker w(i);
           DWorkers.push_back(w);
        }
        // insert the first workers into a  prio queue
        priority_queue<DWorker, vector<DWorker>, wgreater> q;
        for(size_t i=0;i<DWorkers.size()-noReserve; i++){
           q.push(DWorkers[i]);
        } 
        // distribute the slot over the available workers in
        // descending order    
        completeSize = 0;   
        for(size_t i=0;i<slots.size(); i++){
          Slot slot = slots[i];
          completeSize += slot.getSize();
          DWorker w = q.top();
          q.pop();
          w.add(slot);
          q.push(w);
        }
        // write back the result
        while(!q.empty() ){
           DWorker w = q.top();
           q.pop();
           DWorkers[w.getId()] = w;
        }
        if(noReserve > 0){
          reDistribute(noReserve);
        }
        return DWorkers;
    };

    vector<DWorker> DWorkers;  // the workers
    uint64_t completeSize;     // size of the whole relation

    // 5 percent 
    uint32_t getNoReserve(int noWorkers){
      if(noWorkers < 4){
         return 0;
      } 
      return ((uint32_t) ((noWorkers-1)*0.05))+1;
   }

   
   void reDistribute(uint32_t noReserve){
      double avgSize = (double) completeSize / DWorkers.size();
      double maxSize = 1.03 * avgSize;

      

      priority_queue<DWorker, vector<DWorker>, wgreater> qreserve; 
      for(size_t i=DWorkers.size()-noReserve; i< DWorkers.size();i++){
         DWorker w = DWorkers[i];
         qreserve.push(w);
      }
      priority_queue<DWorker, vector<DWorker>, wless> qbase;
      for(size_t i=0;i<DWorkers.size()-noReserve; i++){
         DWorker w = DWorkers[i];
         qbase.push(w);
      }
      while(!qbase.empty()){
        DWorker w = qbase.top(); // the highest worker
        qbase.pop();
        DWorker r = qreserve.top(); // the smallest reserve
        qreserve.pop();
        if(exchange(w,r,maxSize)){
           qbase.push(w);
        } else {
           DWorkers[w.getId()] = w; // fix result
        }
        qreserve.push(r);
      }


     /*
     // variant 2 
      vector<DWorker> qbase;
      for(size_t i=0;i<DWorkers.size()-noReserve; i++){
         DWorker w = DWorkers[i];
         qbase.push_back(w);
      }
      bool change = true;
      while(change){
        change = false;
        for(size_t i=0;i<qbase.size();i++){
           DWorker r = qreserve.top();
           qreserve.pop();
           DWorker w = qbase[i];
           change |= exchange(w,r,maxSize);
           qreserve.push(r);
        }
      }
    */

        // fix reserve
       
      while(!qreserve.empty()){
        DWorker w = qreserve.top();
        qreserve.pop();
        DWorkers[w.getId()] = w;  
     }
   }

   bool exchange(DWorker& source, DWorker& target, double maxSize){
      if(source.getNoSlots() < 2){
        return false;
      }
      Slot slot = source.removeTop();
      double ts = target.getNoElements();
      if(ts + slot.getSize() > maxSize){
        source.add(slot);
        return false;
      } else {
        target.add(slot);
        return true;
      }
      
  }




};



vector<uint32_t> getMapping(const vector<uint32_t>& slotsizes,
                            uint32_t noWorkers,
                            bool simple) {

   vector<Slot> slots;
   vector<uint32_t> result;
   for(size_t i=0;i<slotsizes.size();i++){
     slots.push_back(Slot(i,slotsizes[i]));
     result.push_back(0); // will be overwritten later
   }

   Distributor dist;
   vector<DWorker> worker =  
           simple ? dist.distributeSimple(slots,noWorkers)
                  : dist.distributeComplete(slots,noWorkers);
   for(auto w : worker){
      size_t wn = w.getId();
      for(auto& s : w.getSlots()){
          result[s.getNo()] = wn;
      }
   }   
   return result;
}



