/*
3 Implementation of supporting functions


The following functions support the implementation of functions of
the classes of the PeriodicAlgebra.

*/
#include <iostream>

#include "NestedList.h"
#include "PeriodicSupport.h"


extern NestedList* nl;
using namespace std;

namespace periodic {


/*
3.1 ~About~

The ~About~ function checks whether the distance between the 
arguments is less than EPSILON. EPSILON is a macro defined at 
the begin of this file.

*/
bool About(const double a, const double b){
  double c = abs (a-b);
  return c < EPSILON;
}


/*
3.2 ~Signum~

This function returns:
  
  * -1 if the argument is less than zero
  
  * 0 if the argument is zero
  
  * 1 if the argument is greater than zero

*/
int Signum(const double arg){
  if(arg<0) return -1;
  if(arg>0) return 1;
  return 0;
}


/*
3.3 ~Distance~

The distance function computes the Euclidean distance between the
points defined by (Ax,Ay) and (Bx,By).

[3] O(1)

*/
double Distance(const double Ax, const double Ay,
                const double Bx, const double By){
   return sqrt( (Ax-Bx)*(Ax-By)+(Ay-By)*(Ay-By));               
}                

/*
3.4 ~SignedArea~

This function computes the signed area of the triangle defined
by (p,q,r). The result will be positive if r is located
left to the straight line defined by p,q. A negative result 
indicates that r is right to this line. If r is located 
on this line, the result will be zero.

[3] O(1)

*/
double SignedArea(const double Px, const double Py,
                  const double Qx, const double Qy,
                  const double Rx, const double Ry){
  return ((Rx-Px)*(Ry+Py) + (Qx-Rx)*(Qy+Ry) + (Px-Qx)*(Py+Qy))/2;
}

/*
3.5 ~PointOnSegment~

The function ~PointOnSegment~ checks whether the point (Px,Py)
is contained in the pointset of the segment ( (Ax,Ay)[->](Bx,By))

[3] O(1)

*/
bool PointOnSegment(const double Px, const double Py,
                    const double Ax, const double Ay,
                    const double Bx, const double By){

    /* 
    The point Px,Py is on the segment A->B   iff
    the distance beween A and B is equals to the sum
    of the distances between A, P  and P,B
    */               
    double DistAB = Distance(Ax,Ay,Bx,By);
    double DistAP = Distance(Ax,Ay,Px,Py);
    double DistPB = Distance(Px,Py,Bx,By);
    return About(DistAB,DistAP+DistPB);
}                    

/* 
3.6 ~PointPosOnSegment~

This function computes, where the directed segment defined by s=(x1,y1,x2,y2)
is splitted by the point p=(x,y). The result is :

  * 0: if p is the startpoint of s

  * 1: if p is the endpoint of s

  * a value in 0..1 if s is splitted by p

  * a value outside of [0,1] if p is not on s 

*/

double PointPosOnSegment(double x1, double y1, 
                         double x2, double y2, 
                         double x, double y){

  if(x1==x2 && y1==y2){ // s is a point
     if(x1==x && y1==y)
        return 0;
     else
        return -1;  
  }
  // check wether p is on the line defined by s
  if( ((x-x1)*(y2-y1)!=(y-y1)*(x2-x1))){
       return -1;
  }
  if(x1==x2){ // special case vertical segment
     return (y-y1)/(y2-y1);
  }
  return (x-x1)/(x2-x1);
}


/*
3.7 ~GetNumeric~

Numbers have different representations in nested lists (IntAtom, RealAtom
or Rationals). All Classes should be have the possibility to read any
number format. To realize this, the function ~GetNumeric~ can be used.
The result is __true__, if LE represents a valid numeric value. This value
is stored in the argument __value__ as a double.

[3] O(1)

*/
bool GetNumeric(const ListExpr List, double &value){
 ListExpr LE = List;
  int AT = ::nl->AtomType(LE);
  if(AT==IntType){
     value = (double)::nl->IntValue(LE);
     return true;
  }
  if(AT==RealType){
     value = ::nl->RealValue(LE);
     return true;
  }
  // check for a rational
  int L = ::nl->ListLength(LE);
  if(L!=5 && L!=6)
     return false;
  if(!::nl->IsEqual(::nl->First(LE),"rat"))
     return false;

  LE = ::nl->Rest(LE); // read over the "rat"
  double sign=1.0;
  if(L==6){ // signum is included
    ListExpr SL = ::nl->First(LE);
    if(::nl->IsEqual(SL,"+"))
      sign=1.0;
    else if(::nl->IsEqual(SL,"-"))
      sign=-1.0;
    else
      return false;
    LE = ::nl->Rest(LE); // read over the signum
  }
  if(!::nl->IsEqual(::nl->Third(LE),"/"))
    return false;
  if ( (::nl->AtomType(::nl->First(LE))!=IntType) ||
       (::nl->AtomType(::nl->Second(LE))!=IntType) ||
       (::nl->AtomType(::nl->Fourth(LE))!=IntType))
       return false;
  double ip = ::nl->IntValue(::nl->First(LE));
  double num = ::nl->IntValue(::nl->Second(LE));
  double denom = ::nl->IntValue(::nl->Fourth(LE));
  if(ip<0 || num<0 || denom<=0)
     return false;
  value = sign*(ip + num/denom);
  return true;
}


/*
~WriteListToStreamRec~

This function supports the WriteListToStream function.
It write a ListExpr given as argumnet __Lorig__ to __os__
with the given indent. If the list is corrupt, __error__
will be set to __true__.


*/

void WriteListToStreamRec(ostream &os, const ListExpr Lorig,const int indent,
                          bool &error){
 if(error) return;
  ListExpr L = Lorig;
  NodeType type= ::nl->AtomType(L);
  bool res;
  switch(type){
    case BoolType   : res = ::nl->BoolValue(L);
                      if(res)
              os << "TRUE";
          else
              os << "FALSE";
          break;
    case IntType    : os << (::nl->IntValue(L));break;
    case RealType   : os << ::nl->RealValue(L);break;
    case SymbolType : os << ::nl->SymbolValue(L);break;
    case StringType : os << "\"" << ::nl->StringValue(L) << "\""; break;
    case TextType   : os << "<<A Text>>"; break;
    case NoAtom     : os << endl;
                      for(int i=0;i<indent;i++)
             os << " ";
          os << "(";
                      while(!::nl->IsEmpty(L) && !error){
             WriteListToStreamRec(os,::nl->First(L),indent+4,error);
       os << " ";
       L = ::nl->Rest(L);
          }   
          os << ")";
          break;
    default : os << "unknkow AtomType"; error=true;
  }
}


/*
3.8 ~WriteListExprToStream~

This function writes a ListExpr given by __L__ to __os__.


*/
void WriteListExprToStream(ostream &os, const ListExpr L){
  bool error = false;
   WriteListToStreamRec(os,L,0,error);
   if(error) os << "The given List was corrupt";
}


} // end of namespace periodic
