
/*
3.9 ~LinearBoolMove~

*/
#include <iostream>
#include <string>

#include "NestedList.h"
#include "PeriodicTypes.h"
#include "PeriodicSupport.h"
#include "StandardTypes.h"
#include "LinearConstantMove.h"


extern NestedList* nl;


using namespace std;


namespace periodic{

/*
~ToConstantListExpr~

This function is used to convert a boolean value into a
nested list representation.

*/
ListExpr ToConstantListExpr(const bool value) {
  return ::nl->BoolAtom(value);
}


/*
~ReadFromListExpr~

This function is used to get a boolean value from 
its nested list representation. The list must be
of type __BoolType__ to be a correct representation for this
class.

*/
bool ReadFromListExpr(ListExpr le, bool& v){
   if(::nl->AtomType(le)!=BoolType)
      return false;
    v = ::nl->BoolValue(le);
    return true;
}

} // end of namespace
