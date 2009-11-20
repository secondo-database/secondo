/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Department of Computer Science,
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


*/



#include <map>
#include <utility>
#include <iostream>
#include <ostream>


/*
~LRU~

This class provides a simple implementation of a LRU strategy.

*/

template<class T>
class LRU{
  public:

/*
~Constructor~

Creates a new, empty lru structure.

*/
    LRU(): lru(),first(), last(){}

/*
~Destructor~

*/
   ~LRU(){}  // no self allocated memory, no problems ;-)


/*
~use~

After calling that function, t will be the topmost
element of the lru structure.

*/
    void use(const T& t){
      if(lru.size()==0){ // first entry
        lru[t] = std::pair<const T*, const T*>(0 , 0);
        first = t;
        last  = t; 
      } else {
        typename std::map<T, std::pair<const T*, const T*> >::iterator it; 
        it = lru.find(t);
        if(it!=lru.end()){ // already stored, bring to top
           std::pair<const T*, const T*> e = it->second;
           // remove from list
           connect(e.first, e.second);      
        }
        // put to top
        std::pair<const T*, const T*> p1(0, &lru.find(first)->first);
        lru[t] = p1;
        std::pair<const T*, const T*> p2(&lru.find(t)->first, 
                                         lru.find(first)->second.second); 
        lru[first] = p2;
        first = t;
      }
    }

/*
~deleteLast~

Removes the last recently used element from lru.
The element is returned in the parameter. If the lru is
empty, the result will be false;

*/
    bool deleteLast(T& t){
       if(lru.empty()){
          return false;
       }
       t = last;
       return deleteMember(t);
    }
 
/*
~size~

Returns the amount of elements stored in the structure.

*/
    size_t size(){
       return lru.size();
    }
/*
~deleteMember~

Deletes t from the structure. If t is not contained in 
the structuree, the return value will be false.


*/
    bool deleteMember(const T& t){
       typename std::map<T, std::pair<const T*, const T*> >::iterator it;
       it = lru.find(t);
       if(it==lru.end()){
         return false;
       }
       std::pair<const T*, const T*> p = it->second;
       // delete an element from the middle
       connect(p.first, p.second);
       lru.erase(t);
       return true;
    }

/*
~print~

Prints out the content of the structure to o.
For debugging purposes.

*/

    std::ostream& print(std::ostream& o){
      if(size()==0){
         o << "empty" ;
         return o;
      }
      o << " <F = " << first << " L = " << last << "  ";
      const T* c = &first;
      while(c){
         std::pair<const T*, const T*> p = lru[*c];
         o << "<" << *c << ", ";
         if(p.first){
           o << *p.first << ", ";
         } else {
           o << "*, ";
         }
         if(p.second){
           o << *p.second << "> ";
         } else {
           o << "*> ";
         }
         c = p.second;
      }
      return o; 
      
      
    }


  private:

/*
The structure. We use a map containing all stored elements.
Additionally, all elements are connected via a double linked 
list ordered by the use order. 

*/
    std::map<T,  std::pair<const T*, const T*> > lru;

/*
The first element of the list structure.

*/
    T first;
/*
The last element of the list structure.

*/
    T last;

/*
Connects the list elements given by ~t1~ and ~t2~.
If one of the elements is a null pointer, it refers that
there is no predecessor and/or successor in the ist.
In that case, the ~first~ and ~last~ elements of the
structure are changed.

*/    
    void connect(const T* t1,const T* t2){
       if(!t1 && !t2){
         return;
       }


       if(t2){
         lru[*t2] = std::pair<const T*, const T*> (t1, lru[*t2].second);
       } else {
         last = *t1;
       }
       if(t1){
         lru[*t1] = std::pair<const T*, const T*>(lru[*t1].first, t2);
       } else {
         first = *t2;
       }
    }

};


