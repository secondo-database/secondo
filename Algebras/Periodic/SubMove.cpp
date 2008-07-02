
/*
3.6 ~SubMove~


*/
#include <iostream>
#include <string>

#include "PeriodicTypes.h"

using namespace std;

namespace periodic{

/*
~Equalize~

When this function is called, the value of this Submove is
taken from the argument.

*/
void SubMove::Equalize(const SubMove* SM){
   arrayNumber = SM->arrayNumber;
   arrayIndex = SM->arrayIndex;
}


/*
~ToString~

This function returns a string representation of this 
SubMove.

*/
string SubMove::ToString() const{
   ostringstream res;
   if(arrayNumber==LINEAR)
     res << "linear["<<arrayIndex<<"]";
   else if(arrayNumber==PERIOD)
     res << "period[i"<<arrayIndex<<"]";
   else if(arrayNumber==COMPOSITE)
     res << "composite["<<arrayIndex<<"]";
   return res.str();
}
/*
~Shift Operator~

*/
ostream& operator<< (ostream& os, const SubMove SM){
   __TRACE__
 switch(SM.arrayNumber){
    case LINEAR    : os << "SM_Linear["; break;
    case COMPOSITE : os << "SM_COMPOSITE[";break;
    case PERIOD    : os << "SM_PERIODIC[";break;
    default        : os << "SM_UNKNOWN[";
  }
  os << SM.arrayIndex << "]";
  return os;
}


} // end of namespace periodic
