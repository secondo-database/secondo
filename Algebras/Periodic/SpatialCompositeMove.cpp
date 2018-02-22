
/*
3.5 ~SpatialCompositeMove~


*/

#include <iostream>
#include <string>

#include "NestedList.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "StandardTypes.h"
#include "PeriodicTypes.h"


extern NestedList* nl;

using namespace std;

namespace periodic{

/*
~Constructor~

Should never be used.

*/
SpatialCompositeMove::SpatialCompositeMove(){}

/*
~Constructor~

*/
SpatialCompositeMove::SpatialCompositeMove(int dummy):CompositeMove(1),bbox(1){
}


/*
~Copy constructor~

*/
SpatialCompositeMove::SpatialCompositeMove(const
                                SpatialCompositeMove& source):
     CompositeMove(1), bbox(1){
     Equalize(&source);
}

/*
~Destructor~

*/
SpatialCompositeMove::~SpatialCompositeMove(){}


/*
~Assigment Operator~


*/

SpatialCompositeMove& SpatialCompositeMove::operator=(const 
                    SpatialCompositeMove & source){
   Equalize(&source);
   return *this;
}

/*
~Equalize~

Takes the value for this object from the argument.

*/
void SpatialCompositeMove::Equalize(const SpatialCompositeMove* source){
    CompositeMove::Equalize(source);
    bbox.Equalize(&(source->bbox)); 
}

/*
~ToString~

This function converts a SpatialCompositeMove into its String
representation.

*/
string SpatialCompositeMove::ToString() const{
     ostringstream res;
     res << "(" << interval.ToString();
     res << " " << minIndex << " -> " << maxIndex << ")";
     res << " " << bbox.ToString();
     return res.str();
}


/*
~ToCompositeMove~

Converts this SpatialCompositeMove into a simple CompositeMove 
without spatial information, in Particular without bounding box information.

*/
CompositeMove SpatialCompositeMove::ToCompositeMove() const{
    CompositeMove result;
    ToCompositeMove(result);
    return result;
}

/*
This variant of the ToCompositeFunction 
changes the value of the arguemnt to contain the value of this
object without any spatial information.

*/
void SpatialCompositeMove::ToCompositeMove(CompositeMove& result)const{
    result.interval.Equalize(&interval);
    result.minIndex=minIndex;
    result.maxIndex=maxIndex;
}
/*
~Shift Operator~

*/
ostream& operator<< (ostream& os, const SpatialCompositeMove SCM){
   __TRACE__
  os << "CM[" << SCM.minIndex << "," << SCM.maxIndex << "..";
   os << SCM.interval << ".." << SCM.bbox << "]";
   return os;
}

} // end of namespace periodic
