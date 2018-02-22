
/*
3.8 ~SpatialPeriodicMove~

*/

#include <iostream>
#include <string>

#include "PeriodicTypes.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"

using namespace std;

namespace periodic{
/*
~Constructor~

*/
SpatialPeriodicMove::SpatialPeriodicMove(){}

/*
~Constructor~

*/
SpatialPeriodicMove::SpatialPeriodicMove(int dummy):PeriodicMove(1),bbox(1){}

/*
~Copy Constructor~

*/
SpatialPeriodicMove::SpatialPeriodicMove(
              const SpatialPeriodicMove& source): PeriodicMove(1), bbox(1){
    Equalize(&source);
}

/*
~Destructor~

*/
SpatialPeriodicMove::~SpatialPeriodicMove(){}

/*
~Equalize~

*/
void SpatialPeriodicMove::Equalize(const SpatialPeriodicMove* source){
   PeriodicMove::Equalize(source);
   bbox.Equalize(&(source->bbox));
}

/*
~Assignment Operator~

*/
SpatialPeriodicMove& SpatialPeriodicMove::operator=(
   const SpatialPeriodicMove& source){
   Equalize(&source);
   return *this;
}

/*
~ToString~

*/
string SpatialPeriodicMove::ToString() const{
  ostringstream res;
  res << "R("<<repeatations<<", ";
  res << submove.ToString() <<" ,";
  res << interval.ToString() <<", ";
  res << bbox.ToString() << ")";
  return res.str(); 
}

/*
ToPeriodicMove

*/
PeriodicMove SpatialPeriodicMove::ToPeriodicMove() const{
  PeriodicMove result;
  ToPeriodicMove(result);
  return result;
}

void SpatialPeriodicMove::ToPeriodicMove(PeriodicMove& result) const{
  result.repeatations = repeatations;
  result.submove = submove;
  result.interval.Equalize(&interval);
}

} // end of namespace periodic
