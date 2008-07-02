
/*
3.7 ~PeriodicMove~

*/

#include <iostream>
#include <string>

#include "PeriodicTypes.h"

using namespace std;

namespace periodic{

/*
~Standard Constructor~

This constructor should never be used except in the 
cast function.

*/
PeriodicMove::PeriodicMove(){ }

/*
~Constructor~

*/
PeriodicMove::PeriodicMove(int dummy):interval(1){}

/*
~Copy Constructor~

*/
PeriodicMove::PeriodicMove(const PeriodicMove& source){
    Equalize(&source);
}

/*
~Destructor~

*/
PeriodicMove::~PeriodicMove(){}

/*
~Assigment Operator~

*/
PeriodicMove& PeriodicMove::operator=(const PeriodicMove& source){
   Equalize(&source);
   return *this;
}

/*
~Equalize~ function

*/
void PeriodicMove::Equalize(const PeriodicMove* source){
  repeatations = source->repeatations;
  submove.Equalize(&(source->submove));
  interval.Equalize(&(source->interval));
}

/*
~ToString~

*/
string PeriodicMove::ToString() const{
  ostringstream res;
  res << "R("<<repeatations<<", ";
  res << submove.ToString() <<" ,";
  res << interval.ToString() <<")";
  return res.str(); 
}
/*
~Shift Operator~

*/    
ostream& operator<< (ostream& os, const PeriodicMove PM){
   __TRACE__
  os << "R["<< PM.repeatations <<"](" << PM.submove <<")";
  return os;
}


} // end of namespace periodic 
