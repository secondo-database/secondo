
/*
3.8 ~SimplePoint~

*/

#include "PeriodicTypes.h"

namespace periodic{

SimplePoint::SimplePoint(){}

SimplePoint::SimplePoint(const SimplePoint& source){
   Equalize(&source);
}

SimplePoint::~SimplePoint(){}

int SimplePoint::compareTo(const SimplePoint P2)const {
   if(x<P2.x) return -1;
   if(x>P2.x) return 1;
   if(y<P2.y) return -1;
   if(y>P2.y) return 1;
   return 0;
}

bool SimplePoint::operator< (const SimplePoint P2)const{
  return compareTo(P2)<0;
}
bool SimplePoint::operator> (const SimplePoint P2)const{
  return compareTo(P2)>0;
}
bool SimplePoint::operator== (const SimplePoint P2)const{
  return compareTo(P2)==0;
}
bool SimplePoint::operator!= (const SimplePoint P2)const{
  return compareTo(P2)!=0;
}

void SimplePoint::Equalize(const SimplePoint* P2){
   x = P2->x;
   y = P2->y;
   intinfo=P2->intinfo;
   boolinfo=P2->boolinfo;
}

SimplePoint& SimplePoint::operator=(const SimplePoint& source){
   Equalize(&source);
   return *this;
}


} // end of namespace periodic
