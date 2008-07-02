
/*
3.4 ~CompositeMove~

*/

#include <iostream>
#include <string>

#include "PeriodicTypes.h"
#include "PeriodicSupport.h"


extern NestedList* nl;

using namespace std;

namespace periodic{

/*
~Constructor~

This constructor should never be used execpt in the cast function.

*/
CompositeMove::CompositeMove(){}

/*
~Constructor~

*/
CompositeMove::CompositeMove(int dummy):interval(1){
      minIndex = -1;
      maxIndex = -1;
}

/*
~Copy Constructor~

*/
CompositeMove::CompositeMove(const CompositeMove& source){
     Equalize(&source);
}

/*
~Destructor~

*/
CompositeMove::~CompositeMove(){}

/*
~Assignment operator~

*/
CompositeMove & CompositeMove::operator=(const CompositeMove& source){
   Equalize(&source);
   return *this;
}

/*
~Equalize~

*/
void CompositeMove::Equalize(const CompositeMove* source){
   this->interval.Equalize(&(source->interval));
   this->minIndex = source->minIndex;
   this->maxIndex = source->maxIndex;
}

/*
~ToString~

*/
string CompositeMove::ToString()const{
    ostringstream res;
    res << "(" << interval.ToString();
    res << " " << minIndex << " -> " << maxIndex << ")";
    return res.str();
}

/*
~Shift Operator~

*/
ostream& operator<< (ostream& os, const CompositeMove CM){
   __TRACE__
  os << "CM[" << CM.minIndex << "," << CM.maxIndex << "..";
  os << CM.interval << "]";
   return os;
}
} // end of namespace periodic
