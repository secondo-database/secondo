
/*
3.14 ~TwoPoints~ 

*/

#include "PeriodicTypes.h"
#include "PeriodicSupport.h"


namespace periodic{


/*
~Constructor~

*/
TwoPoints::TwoPoints(){}

/*
~Copy Constructor~

*/
TwoPoints::TwoPoints(const TwoPoints& source){
   Equalize(&source);
}

/*
~Destructor~

*/
TwoPoints::~TwoPoints(){}

/*
~Assignment operator~

*/
TwoPoints& TwoPoints::operator=(const TwoPoints& source){
  Equalize(&source);
  return *this;
}


/*
~Comparisons~

*/
int  TwoPoints::CompareTo(const TwoPoints TP) const {
    if(startX<TP.startX) return -1;
    if(startX>TP.startX) return 1;
    if(startY<TP.startY) return -1;
    if(startY>TP.startY) return 1;
    if(endX<TP.endX) return -1;
    if(endX>TP.endX) return 1;
    if(endY<TP.endY) return -1;
    if(endY>TP.endY) return 1;
    return 0; 
}

bool TwoPoints::operator< (const TwoPoints TP)const { 
   return CompareTo(TP)<0; 
}
bool TwoPoints::operator> (const TwoPoints TP)const { 
   return CompareTo(TP)>0; 
}
bool TwoPoints::operator== (const TwoPoints TP)const { 
  return CompareTo(TP)==0; 
}

/*
~Access funmctions~

*/
double TwoPoints::GetStartX()const { 
   return startX; 
} 
double TwoPoints::GetEndX()const {
   return endX;
}
double TwoPoints::GetStartY()const {
   return startY;
}
double TwoPoints::GetEndY()const {
   return endY;
}
/*
~Equalize~

*/
void TwoPoints::Equalize(const TwoPoints* source){
   startX = source->startX;
   startY = source->startY;
   endX = source->endX;
   endY = source->endY;
}

/*
~IsSpatialExtension~

This function checks whether the line defined by TP is
a extension of the segment defined by this. 

[3] O(1)

*/
bool TwoPoints::IsSpatialExtension(const TwoPoints* TP) const {
   __TRACE__
     if(endX!=TP->startX) return false;
      if(endY!=TP->startY) return false;
      double dx = endX-startX;
      double dy = endY-startY;
      double TPdx = TP->endX-TP->startX;
      double TPdy = TP->endY-TP->startY;
      return About(dy*TPdx,dx*TPdy); 
}

/*
~Speed~

This function computes the speed for a points moving from the
startpoint to the endpoint in the given interval. The speed is allways
greater than zero. This function will return -1 if an error is occurred
e.g. division by zero.

[3] O(1)

*/

double TwoPoints::Speed(const RelInterval interval)const {
   __TRACE__
  datetime::DateTime* D = interval.GetLength();
   double L = D->ToDouble();
   delete D; 
   D = NULL;
   if(L<0) return -1;
   double dx = endX-startX;
   double dy = endY-startY;
   double distance=sqrt(dx*dx+dy*dy);
   if(L==0 && distance!=0) return -1;
   if(distance==0) return 0;
   return distance/L;
}

/*
~Constructor~

This constructor sets the starting point to (xs,ys) and the
endpoint to (xe,ye).

[3] O(1)

*/
TwoPoints::TwoPoints(const double xs, const double ys,
                     const double xe, const double ye){
  __TRACE__
  startX = xs;
  startY = ys;
  endX = xe;
  endY = ye;
}

/*
~InnerIntersects~

This function checks wether the segments defined by the
start and endpoint have any intersection which is not an
endpoint of one of the  segments. For overlapping segments,
the result will also __false__.

[3] O(1)

*/
bool TwoPoints::InnerIntersects(const TwoPoints& TP) const{
   __TRACE__
 // first we check the cases of segments degenerated to points
  if( IsStatic() && TP.IsStatic()){
     return false;
  }
  if(IsStatic()){
    // check for endpoint
    if(((startX==TP.startX) && (startY==TP.startY)) ||
       ((startX==TP.endX) && (startY==TP.endY)))
       return false;     
    return PointOnSegment(startX,startY,TP.startX,TP.startY,TP.endX,TP.endY);
  }
  if(TP.IsStatic()){
    if(((startX==TP.startX) && (startY==TP.startY)) ||
         ((endX==TP.startX) && (endY==TP.startY)))
      return false;     
    return PointOnSegment(TP.startX,TP.startY,startX,startY,endX,endY);      
  }
  // At this point we handle with two non-degenerated segments
  int S1 = Signum(SignedArea(startX,startY,endX,endY,TP.startX,TP.startY));
  int S2 = Signum(SignedArea(startX,startY,endX,endY,TP.endX,TP.endY));
  if(S1==S2) return false;
  if(S1==0 || S2==0) return false; // endpoint
  S1 = Signum(SignedArea(TP.startX,TP.startY,TP.endX,TP.endY,startX,startY));
  S2 = Signum(SignedArea(TP.startX,TP.startY,TP.endX,TP.endY,endX,endY));
  return (S1!=S2) && (S1!=0) && (S2!=0);
}

/*
~IsStatic~

This function checks whether the start and the endpoint are
equal.

[3] O(1)

*/
bool TwoPoints::IsStatic() const{
   __TRACE__
 return startX==endX &&
         startY==endY;
}

/*
~Shift Operator~

*/
std::ostream& operator<< (std::ostream& os, const TwoPoints TP){
   __TRACE__
 os << "TP[(" << TP.startX << "," << TP.startY << ") ->(";
  os << TP.endX << "," << TP.endY << ")]";
  return os;
}


} // end of namepsace
