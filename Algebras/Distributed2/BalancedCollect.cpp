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
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>

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


       size_t getSize() const { return slots.size(); }
       size_t getNoElements() const { return size; } 

       int getId () const{ return no; } 

       const vector<Slot>& getSlots() const;

   private:
      size_t no;      // identifikator od this worker 
      size_t size;    // summarized size of all slots
      vector<Slot> slots;

};


/*
Interface for Distribution algorithm.

*/

class Distributor{

public:
  Distributor() {}
  virtual ~Distributor() {}
  virtual bool setArguments(vector<string> args) = 0;
  virtual vector<DWorker> distribute(vector<Slot> slots, int noDWorkers) = 0;
  virtual string getName() const = 0;
  virtual string help() const = 0;
};


/*
~wless~

Comparator for DWorkers.
Returns true if the left DWorker has more elements than the right one.

*/

class wless {
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

class SimpleDist : public Distributor{

public:
    string getName() const {
       return "SimpleDist";
    }

    string help() const {
       stringstream ss;
       ss << " This algorithms does not requires additional argument" << endl;
       ss << "It distributes the slots to the DWorkers with decreasing size "
          << endl;
       ss << "the next slot is assigned to the DWorker having the smallest " 
          << endl;
       ss << "amount of elements " << endl;
       return ss.str();
    }

    bool setArguments(vector<string> args) {
      return args.size() == 0;
    }

    virtual vector<DWorker> distribute(vector<Slot> slots, int noDWorkers) {
        sort(slots.begin(), slots.end(), Slot::sizeGreater);
        DWorkers.clear();
        for(int i=0;i<noDWorkers;i++){
           DWorker w(i);
           DWorkers.push_back(w);
        }
        priority_queue<DWorker, vector<DWorker>, wless> q;
        for(size_t i=0;i<DWorkers.size(); i++){
           q.push(DWorkers[i]);
        } 
        for(size_t i=0;i<slots.size(); i++){
          Slot slot = slots[i];
          DWorker w = q.top();
          q.pop();
          w.add(slot);
          q.push(w);
        }
        while(!q.empty() ){
           DWorker w = q.top();
           q.pop();
           DWorkers[w.getId()] = w;
        }

        return DWorkers;

    };

  private:
       vector<DWorker> DWorkers;
};



vector<uint32_t> getMappingSimple(vector<size_t>& slotsizes,
                                  uint32_t noWorkers) {

   vector<Slot> slots;
   vector<uint32_t> result;
   for(size_t i=0;i<slotsizes.size();i++){
     slots.push_back(Slot(i,slotsizes[i]));
     result.push_back(0); // will be overwritten
   }

   SimpleDist dist;
   vector<DWorker> worker = dist.distribute(slots,noWorkers);
   for(auto w : worker){
      size_t wn = w.getId();
      for(auto& s : w.getSlots()){
          result[s.getNo()] = wn;
      }
   }   
   return result;
}






