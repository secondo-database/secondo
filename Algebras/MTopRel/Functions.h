

/*
----
This file is part of SECONDO.

Copyright (C) 2007, 
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

*/



/*

1 Some auxiliary Functions


*/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H


#include <set>
#include <vector>
#include <iostream>


/*
1.1 ~contains~

A function checking whether an object is 
contained within a vector.

*/
template<typename T>
bool contains(const std::vector<T>& v, T elem){
   typename std::vector<T>::const_iterator it;
   for(it=v.begin(); it!=v.end();it++){
      if(*it == elem){
        return true;
      }
   }
   return false;   
}

/*
1.2 ~printset~

This function just prints out a set of something.

*/
template<typename T>
std::ostream& printset(const std::set<T>& s, std::ostream& o){
  o << "{";
  typename std::set<T>::const_iterator it;
  for(it=s.begin();it!=s.end();it++){
    if(it!=s.begin()){
      o << ", ";
    }
    o << *it;
  }
  o << "}";
  return o;
}



/*
1.3 ~printVector~

This function prints aout the content of a vector.

*/
template<typename T>
std::ostream& printVector(const std::vector<T>& s, std::ostream& o){
  o << "[";
  typename std::vector<T>::const_iterator it;
  for(it=s.begin();it!=s.end();it++){
    if(it!=s.begin()){
      o << ", ";
    }
    o << *it;
  }
  o << "]";
  return o;
}


/*
1.4 ~intersects~

Checks whether ttwo sets have at least one common element;

*/
template<typename T>
bool intersects( const std::set<T>& s1, const std::set<T>& s2){
   typename std::set<T>::const_iterator it1 = s1.begin();
   typename std::set<T>::const_iterator it2 = s2.begin();
   while(it1!=s1.end() && it2!=s2.end()){
      if(*it1==*it2){
        return true;
      } else if(*it1 < *it2){
          it1++;
      } else {
          it2++;
      }
   } 
   return false;
}



/* 
1.5 ~getVectorIndex~

  Returns the first index within a vector having the same value as 
  the given one. If the element is not found, -1 is returned.

*/

template<typename T>
int getVectorIndex(const std::vector<T>& v, const T& elem){
  for(unsigned int i=0;i<v.size();i++){
     if(v[i]==elem){
        return i;
     }
  }
  return -1;
}

/*
1.4 ~removeState~

removes state from the set and decrements all elements in the set greater
than state by one.

*/
void removeState(std::set<int>& targets, int state);



#endif
