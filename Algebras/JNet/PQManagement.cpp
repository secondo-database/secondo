/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

2012, July Simone Jandt

1. Includes

*/

#include <limits>
#include "PQManagement.h"


using namespace jnetwork;
using namespace std;

/*
1 Implementation of class ~PQManagement~

1.1 Constructors and Deconstructors

*/

PQManagement::PQManagement() :
  visited(), pq(0),sizePQ(0)
{}

PQManagement::~PQManagement()
{}

/*
1.1. Insert

*/

void PQManagement::Insert(const JPQEntry& e)
{
  VisitedJunction vj(e,-1);
  int index;
  if(visited.get(vj)) { 
     if( (index=vj.GetIndexPQ()) >=0){ // entry not finished
        JPQEntry se;
        pq.Get(index,se);
        if(e.GetPriority() < se.GetPriority()){
           pq.Put(index,e);
           blowUp(index);
        }
     }
  } else {
     index = sizePQ;
     pq.Put(index,e);
     vj.SetIndexPQ(index);
     visited.insert(vj);
     blowUp(index);
     sizePQ++;
  }
}

/*
1.1 InsertJunctionAsVisited

*/

void PQManagement::InsertJunctionAsVisited(const JPQEntry juncE)
{
   VisitedJunction vj(juncE,-1);
   visited.insert(vj);
}

/*
1.1. GetAndDeleteMin

*/

JPQEntry* PQManagement::GetAndDeleteMin()
{
  if(sizePQ==0) return 0;
  JPQEntry res;
  pq.Get(0,res);
  sizePQ--;
  VisitedJunction vj(res,-1);
  visited.update(vj);
  if(sizePQ>0){
     JPQEntry last;
     pq.Get(sizePQ,last);
     pq.Put(0,last);
     VisitedJunction vj(last,0);
     visited.update(vj);
     sink(0);
  }
  return new JPQEntry(res);
}

/*
1.1 IsEmpty

*/

bool PQManagement::IsEmpty() const
{
  return sizePQ==0;
}

/*
1.1 Destroy

*/

void PQManagement::Destroy()
{
  pq.Destroy();
  visited.destroy();
}


/*
1.1 Print

*/

ostream& PQManagement::Print(ostream& os) const
{
  os << "priority queue: " << endl;
  JPQEntry curElem;
  for (int i = 0; i < sizePQ; i++)
  {
    pq.Get(i,curElem);
    os << i << ".Elem: " << curElem << endl;
  }
  os << "visited junctions: " << endl;
  visited.printArray(os);
  os << endl;
  return os;
}

/*
1 Implementation of Private Methods

*/

void PQManagement::blowUp(int index){
   int father = ((index + 1) / 2) - 1;
   JPQEntry elem;
   pq.Get(index,elem);
   bool move = false;
   while(father >=0){
      JPQEntry f;
      pq.Get(father,f);
      if(f.GetPriority() <= elem.GetPriority()){
         if(move){
           pq.Put(index,elem);
           VisitedJunction vj(elem,index);
           visited.update(vj);
         }
         return;
      }
      move=true;
      // swap entries in heap
      pq.Put(index,f);
      // update father in visited
      VisitedJunction vj(f,index);
      visited.update(vj);
      index = father;
      father = ((index + 1) / 2) - 1;
   }
   if(move){
     pq.Put(index,elem);
     VisitedJunction vj(elem,index);
     visited.update(vj);
   }
}

void PQManagement::sink(int index){

   int si = ((index+1)*2) -1;
   int rsi = si+1;
   JPQEntry elem;
   pq.Get(index,elem);
   JPQEntry s;
   JPQEntry rs;
   bool move = false;

   while(si < sizePQ){ // son exists
      pq.Get(si,s);
      if(rsi<sizePQ){ // right son exists
        pq.Get(rsi,rs);
        if(rs.GetPriority() < s.GetPriority()){
           si = rsi;
           s = rs;
        }
      }
      if(s.GetPriority() >= elem.GetPriority()){
        si = sizePQ; // stop processing
      } else {
        move = true;
        pq.Put(index,s);
        VisitedJunction vj(s,index);
        visited.update(vj);
        index = si;
        si = ((index+1)*2) -1;
        rsi = si+1;
      }
   }
   if(move){
      pq.Put(index,elem);
      VisitedJunction vj(elem,index);
      visited.update(vj);
   }
}

/*
1 Overload output operator

*/

ostream& operator<<(ostream& os, const PQManagement& e)
{
  e.Print(os);
  return os;
}
