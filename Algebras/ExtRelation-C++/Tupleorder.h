/* 
---- 
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

Dec 2008. M. Spiekermann. Code provided as header file.


1 Auxiliary definitions for value mapping function of operators ~sort~ and ~sortby~

*/

#ifndef SEC_TUPLEORDER_H
#define SEC_TUPLEORDER_H

#include <queue>
#include "RelationAlgebra.h"


static LexicographicalTupleSmaller lexCmp;

class TupleAndRelPos {
public:

  TupleAndRelPos()       :
    ref(0),
    pos(0),
    cmpPtr(0) 
  {};
  
  ~TupleAndRelPos()
  {}      

  TupleAndRelPos(Tuple* newTuple, TupleCompareBy* cmpObjPtr = 0, 
                 int newPos = 0) :
    ref( newTuple ),
    pos(newPos),
    cmpPtr(cmpObjPtr)
  {}; 


  inline TupleAndRelPos(const TupleAndRelPos& rhs) :
    ref(rhs.ref),
    pos(rhs.pos),
    cmpPtr(rhs.cmpPtr)  
  {};     

  /* without the code below we get a better performance
   *
  inline TupleAndRelPos& operator=(const TupleAndRelPos& rhs)
  {
    ref = rhs.ref;
    pos = rhs.pos;
    cmpPtr = rhs.cmpPtr;
    return *this;    
  } */      
 
  inline bool operator<(const TupleAndRelPos& rhs) const 
  { 
    // by default < is used to define a sort order
    // the priority queue creates a maximum heap, hence
    // we change the result to create a minimum queue.
    // It would be nice to have also an < operator in the class
    // Tuple. Moreover lexicographical comparison should be done by means of
    // TupleCompareBy and an appropriate sort order specification, 

    if (!this->ref || !rhs.ref) {
      return true;
    }

    //return !cmp->(this->rt.tuple, ref.rt.tuple);

    
    if ( cmpPtr ) {
      return !(*(TupleCompareBy*)cmpPtr)( this->ref, rhs.ref );
    } else {
      return !lexCmp( this->ref, rhs.ref );
    }
    
  }

  inline Tuple* tuple() const { return ref; }

  Tuple* ref;
  int pos;

private:
   void* cmpPtr;

};


/*
Experimental Code

How can we manage to avoid the member cmpPtr in the class above.

*/


template<class TupleCmp>
class UniversalCompare : public std::binary_function< TupleAndRelPos, 
                                                   TupleAndRelPos, bool >
{
  public:
  bool operator()(const TupleAndRelPos& a, const TupleAndRelPos& b)
  {
    return TupleCmp()( a.tuple(), b.tuple() );    
  }       
};    



class TupleQueue {    
  
  typedef std::priority_queue<TupleAndRelPos> Queue;
  Queue q;

  public:
  TupleQueue() {}
  ~TupleQueue() {}

  inline const TupleAndRelPos& top() { return q.top(); }

  inline Tuple* topTuple() { return q.top().tuple(); }

  inline void push(const TupleAndRelPos& elem) 
  {
    elem.tuple()->IncReference();         
    q.push(elem); 
  }
 
  inline size_t size() { return q.size(); }

  inline bool empty() { return q.empty(); }

  inline void pop() 
  {
    Tuple* t = q.top().tuple();
    t->DeleteIfAllowed();
    q.pop();
  }       

};

/*class TupleAndRelPos {
public:

  TupleAndRelPos() :
    ref(0),
    pos(0),
    cmpPtr(0) 
  {};
  
  TupleAndRelPos(Tuple* newTuple, TupleCompareBy* cmpObjPtr = 0, 
                 int newPos = 0) :
    ref(newTuple),
    pos(newPos),
    cmpPtr(cmpObjPtr)
  {}; 

  inline bool operator<(const TupleAndRelPos& rhs) const 
  { 
    // by default < is used to define a sort order
    // the priority queue creates a maximum heap, hence
    // we change the result to create a minimum queue.
    // It would be nice to have also an < operator in the class
    // Tuple. Moreover lexicographical comparison should be done by means of
    // TupleCompareBy and an appropriate sort order specification, 

    if (!this->ref.tuple || !rhs.ref.tuple) {
      return true;
    }
    if ( cmpPtr ) {
      return !(*(TupleCompareBy*)cmpPtr)( this->ref.tuple, rhs.ref.tuple );
    } else {
      return !lexCmp( this->ref.tuple, rhs.ref.tuple );
    }
  }

  inline TupleAndRelPos& operator=(const TupleAndRelPos& rhs)
  { 
    this->ref = rhs.ref;
    return *this; 
  }

  RTuple ref;
  int pos;

private:
  void* cmpPtr;

};*/


class CompareObject {

  public: 
  
  CompareObject(bool lexOrder, Word* args) {

    tupleCmp = 0;

    if(lexOrder) 
    {
      tupleCmp = new LexicographicalTupleSmaller();
    }	
    else
    {
      SortOrderSpecification spec;
      int nSortAttrs = StdTypes::GetInt( args[2] );
      for(int i = 1; i <= nSortAttrs; i++)
      {
	int sortAttrIndex = StdTypes::GetInt( args[2 * i + 1] );
	bool sortOrderIsAscending = StdTypes::GetBool( args[2 * i + 2] );
	
	spec.push_back(std::pair<int, bool>(sortAttrIndex, 
				       sortOrderIsAscending));
      };

      tupleCmp = new TupleCompareBy( spec );
    }
  }  

  ~CompareObject() {}

  void* getPtr() const { return tupleCmp; }

  private:
    CompareObject() {}	  
    
    void* tupleCmp;

}; 

#endif
