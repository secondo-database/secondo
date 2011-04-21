/*
1.1 ~PBBox~

*/

#include <iostream>
#include <string>

#include "PeriodicSupport.h"
#include "PeriodicTypes.h"
#include "StandardTypes.h"

using namespace std;

namespace periodic{

/*
~Constructor~

The standard constructor for special use;

[3] O(1)

*/
PBBox::PBBox(){
   __TRACE__
}

/*
~Constructor~

This constructor creates an empty  box.

[3] O(1)

*/
PBBox::PBBox(int dummy): Attribute(false), minX(0), maxX(0), 
                                           minY(0), maxY(0), 
                                           state(1){
   __TRACE__
}

/*
~Constructor~

This constructor creates a new bounding box
containing only the point (x,y).

[3] O(1)

*/
PBBox::PBBox(const double x, const double y):
  Attribute(true), minX(x), maxX(x), minY(y), maxY(y), state(0){
   __TRACE__
  SetEmpty(false);
}

/*
~Constructor~

This constructor creates a bounding box from the
given rectangle.

[3] O(1)

*/
PBBox::PBBox(const double _minX, const double _minY, 
             const double _maxX, const double _maxY):
  Attribute(true), minX(_minX), maxX(_maxX), minY(_minY), maxY(_maxY), state(0){
   __TRACE__
  SetEmpty(false);
}


/*
~Copy Constructor ~

*/
PBBox::PBBox(const PBBox& source):
    Attribute(source.IsDefined()), 
    minX(source.minX), maxX(source.maxX), 
    minY(source.minY), maxY(source.maxY), 
    state(source.state){ }

/*
~Destructor~

The destructor makes nothing because all class members
are primitive types.

*/

PBBox::~PBBox(){}


/*
~Compare~

This function compares this PBBox with  __arg__.

[3] O(1)

*/
int PBBox::Compare(const Attribute* arg) const{
    __TRACE__
  return CompareTo((PBBox*) arg);
}

/*
~Adjacent~

Because a bounding box is defined in the Euclidean Plane, the
adjacent function cannot be implemented. Hence the return value is
allways false;

[3] O(1)

*/
bool PBBox::Adjacent(const Attribute*)const{
    __TRACE__
 return false;
}


/*
~Sizeof~

This function returns the size of the PBBox class.

[3] O[1]

*/
size_t PBBox::Sizeof()const{
    __TRACE__
  return sizeof(PBBox);
}

/*
~IsEmpty~

This function returns whether this bounding box don't contains
any point of the Euclidean Plane.

[3] O(1)

*/
bool PBBox::IsEmpty() const{
    __TRACE__
  return (state&1)>0;
}


/*
~HashValue~

This fuction returns a Hash-Value of this bounding box.

[3] O(1)

*/
size_t PBBox::HashValue() const{
    __TRACE__
  if(!IsDefined()) return (size_t) 0;
  if(IsEmpty()) return (size_t) 1;
   return (size_t)  abs(minX + maxX*(maxY-minY));
}


/*
~CopyFrom~

If this function is called the bounding box takes its value
from the given argument.

[3] O(1)

*/
void PBBox::CopyFrom(const Attribute* arg){
    __TRACE__
  Equalize((PBBox*) arg);
}

/*
~ToListExpr~

This function translates this bounding box to its nested list representation.

[3] O(1)

*/
ListExpr PBBox::ToListExpr(const ListExpr typeInfo)const{
    __TRACE__
   if(!IsDefined())
       return ::nl->SymbolAtom("undefined");
    if(IsEmpty())
       return ::nl->SymbolAtom("empty");
    return ::nl->FourElemList(::nl->RealAtom(minX),
                            ::nl->RealAtom(minY),
                            ::nl->RealAtom(maxX),
                            ::nl->RealAtom(maxY));
}

/*
~ReadFrom~

This function reads the value of this bounding box from the given
ListExpr.

[3] O(1)

*/
bool PBBox::ReadFrom(const ListExpr LE, const ListExpr typeInfo){
    __TRACE__
 if(::nl->IsEqual(LE,"undefined") || ::nl->IsEqual(LE,"undef")){
      SetDefined(false);
      SetEmpty(true);
      return true;
  }
  if(::nl->IsEqual(LE,"empty")){
     SetDefined(true);
     SetEmpty(true);
     return true;
  }
  if(::nl->ListLength(LE)!=4)
    return false;

  double x1,x2,y1,y2;
  if(!periodic::GetNumeric(::nl->First(LE),x1)) return false;
  if(!periodic::GetNumeric(::nl->Second(LE),y1)) return false;
  if(!periodic::GetNumeric(::nl->Third(LE),x2)) return false;
  if(!periodic::GetNumeric(::nl->Fourth(LE),y2)) return false;
  minX = x1<x2?x1:x2;
  maxX = x1<x2?x2:x1;
  minY = y1<y2?y1:y2;
  maxY = y1<y2?y2:y1;
  SetEmpty(false);
  SetDefined(true);
  return true;
}


/*
~CompareTo~

The ~CompareTo~ function compares two bounding boxes
lexicographically (order minX maxX minY maxY).
An undefined Bounding box is less than a defined one.

[3] O(1)

*/
int PBBox::CompareTo(const PBBox* B2)const {
    __TRACE__
  bool defined = this->IsDefined();
  bool B2defined = B2->IsDefined();
  if(!defined&&!B2defined) return 0;
  if(!defined&&B2defined) return -1;
  if(defined&&!B2defined) return 1;
  // Now holds that this and B2 are defined
  bool isEmpty = this->IsEmpty();
  bool B2isEmpty = B2->IsEmpty();
  if(isEmpty&&B2isEmpty) return 0;
  if(isEmpty&&!B2isEmpty) return -1;
  if(!isEmpty&&B2isEmpty) return 1;
  // both boxes are not empty
  if(minX<B2->minX) return -1;
  if(minX>B2->minX) return 1;
  if(maxX<B2->maxX) return -1;
  if(maxX>B2->maxX) return 1;
  if(minY<B2->minY) return -1;
  if(minY>B2->minY) return 1;
  if(maxY<B2->maxY) return -1;
  if(maxY>B2->maxY) return 1;
  return 0;
}

/*
~Contains~

This function checks whether the point defined by (x,y) is
contained in this bounding box.

[3] O(1)

*/
bool PBBox::Contains(const double x,const double y)const{
    __TRACE__
 if(!IsDefined()) return false;
  if(IsEmpty()) return false;
  return x>=minX && x<=maxX && y>=minY && y<=maxY;
}


/*
~Contains~

The following function checks whether B2 is contained in this PBBox.

[3] O(1)

*/
bool PBBox::Contains(const PBBox* B2)const {
    __TRACE__
  if(!IsDefined()) return false;
   if(IsEmpty()) return B2->IsEmpty();
   return Contains(B2->minX,B2->minY) && Contains(B2->maxX,B2->maxY);
}

/*
~Clone~

Returns a clone of this;

[3] O(1)

*/
PBBox* PBBox::Clone() const{
    __TRACE__
 PBBox* res = new PBBox(minX,minY,maxX,maxY);
  res->SetDefined(this->IsDefined());
  res->SetEmpty(this->IsEmpty());
  return res;
}

/*
~Equalize~

The ~Equalize~ function changed this instance to have the same value
as B2.

[3] O(1)

*/
void PBBox::Equalize(const PBBox* B2){
    __TRACE__
  SetDefined(B2->IsDefined());
  minX = B2->minX;
   maxX = B2->maxX;
   minY = B2->minY;
   maxY = B2->maxY;
   state = B2->state;
}

void PBBox::Equalize(const PBBox& B2){
    __TRACE__
   Attribute::operator=(B2);
   minX = B2.minX;
   maxX = B2.maxX;
   minY = B2.minY;
   maxY = B2.maxY;
   state = B2.state;
}

/*
~Intersection~

This function computes the intersection beween this instance and
B2. If both boxes are disjoint the result will be undefined.

[3] O(1)

*/
void PBBox::Intersection(const PBBox* B2){
    __TRACE__
 if(!IsDefined()) return;
  if(!B2->IsDefined()){
    SetDefined(false);
    return;
  }
  if(IsEmpty()) {
      return;
  }
  if(B2->IsEmpty()){
     SetEmpty(true);
     return;
  }
  minX = minX>B2->minX? minX : B2->minX;
  maxX = maxX<B2->maxX? maxX : B2->maxX;
  minY = minY>B2->minY? minY : B2->minY;
  maxY = maxY<B2->maxY? maxY : B2->maxY;
  SetEmpty(minX>maxX ||  minY>maxY);
}

void PBBox::Intersection(const PBBox* b2, PBBox& res)const{
   res.Equalize(this);
   res.Intersection(b2);
}

/*
~Intersects~

~Intersects~ checks whether this and B2 share any common point.

[3] O(1)

*/
bool PBBox::Intersects(const PBBox* B2)const{
    __TRACE__
 if(!IsDefined() || !B2->IsDefined()) return false;
  if(IsEmpty() || B2->IsEmpty()) return false;
  if(minX>B2->maxX) return false; //right of B2
  if(maxX<B2->minX) return false; //left of B2
  if(minY>B2->maxY) return false; //over B2
  if(maxY<B2->minY) return false; //under B2
  return true;
}

/*
~Size~

This function returns the size of the box. If this box is
undefined -1.0 is returned. Otherwise the result will be a
non-negative double number. Note that a empty box and a box
containing a single point will yield the same result.

[3] O(1)

*/
double PBBox::Size()const {
    __TRACE__
 if(!IsDefined()) return -1;
 if(IsEmpty()) return 0;
  return (maxX-minX)*(maxY-minY);
}

/*
~Union~

The function ~Union~ computes the bounding box which contains
both this bounding box as well as B2.

[3] O(1)

*/
void PBBox::Union(const PBBox* B2){
    __TRACE__
 if(!B2->IsDefined()){ // operators are only allowed on defined arguments
     SetDefined(false);
     return;
  }
  if(!IsDefined()) return; // see above

  if(B2->IsEmpty()) return; // no change

  // this and B2 are defined and not empty
  if(IsEmpty()){
      Equalize(B2);
      return;
  }
  minX = minX<B2->minX? minX: B2->minX;
  maxX = maxX>B2->maxX? maxX: B2->maxX;
  minY = minY<B2->minY? minY: B2->minY;
  maxY = maxY>B2->maxY? maxY: B2->maxY;
}

void PBBox::Union(const PBBox* b2, PBBox& res)const{
   if(!IsDefined() || !b2->IsDefined()){
      res.SetDefined(false);
      return;
   } else if(IsEmpty()) {
      res.Equalize(b2);
   } else if(b2->IsEmpty()){
      res.Equalize(this); 
   } else {
    res.SetEmpty(false); 
    res.SetDefined(true);
    res.minX = minX<b2->minX? minX: b2->minX;
    res.maxX = maxX>b2->maxX? maxX: b2->maxX;
    res.minY = minY<b2->minY? minY: b2->minY;
    res.maxY = maxY>b2->maxY? maxY: b2->maxY;
   }
}



/*
~Union~

This variant of the ~Union~ functions extends this bounding
box to contain the point defined by (x,y)

[3] O(1)

*/
void PBBox::Union(const double x, const double y){
    __TRACE__
if(!IsDefined())return;
 if(IsEmpty()){
    SetEmpty(false);
    minX=maxX=x;
    minY=maxY=y;
    return;
 }
 else{ // really extend this if needed
    minX = minX<x? minX : x;
    maxX = maxX>x? maxX : x;
    minY = minY<y? minY : y;
    maxY = maxY>y? maxY : y;
 }
}

/*
~SetUndefined~

The ~SetUndefined~ function sets the state of this BBox to be
undefined.

[3] O(1)

*/
void PBBox::SetUndefined(){
    __TRACE__
  SetDefined(false);
}

/*
~GetVertex~

This functions returns a vertex of this box.

  * __No__=0: left bottom

  * __No__=1: right bottom

  * __No__=2: left top

  * __No__=3: right top

  * otherwise: the return value is false, x,y remains unchanged

[3] O(1)

*/
bool PBBox::GetVertex(const int No, double& x, double& y)const{
    __TRACE__
  if(!IsDefined()) return false;
  if(IsEmpty()) return false;

   if(No==0){
     x = minX;
     y = minY;
     return true;
   }
   if(No==1){
     x=maxX;
     y=minY;
     return true;
   }
   if(No==2){
     x=minX;
     y=maxY;
     return true;
   }
   if(No==3){
     x=maxX;
     y=maxY;
     return true;
   }
   if(DEBUG_MODE){
     cerr << "PBBox::GetVertex called with wrong value :" << No << endl;
   }
   return  false;
}

/*
~SetEmpty~

Sets a bounding box to be empty and defined.

*/
void PBBox::SetEmpty(){
   SetEmpty(true);
   SetDefined(true);
}

/*
~SetEmpty~

Sets the isEmpty flag of this box.

*/
void PBBox::SetEmpty(const bool value){
   if(value){
     state = state | 1;
   }else{
     state = state & ~1;
   }
}



/*
~ToString~

This function returns a string representation of a bounding box.

*/
string PBBox::ToString() const {
       ostringstream res;
       if(!IsDefined())
          res << "[undefined]";
       else if(IsEmpty())
          res << "[empty]";
       else{
          res << "[" << minX <<", " << minY << ", ";
          res << maxX << ", " << maxY << "]";
           
       }
       return res.str(); 
    }


/*
~NumOfFLOBs~

This function returns always zero.

*/
int PBBox::NumOfFLOBs() const{
   return 0;
}

/*
~GetFLOB~

Because the ~NumOfFLOBs~ function returns zero, this 
function should never be called.

*/
Flob* PBBox::GetFLOB(int i){
   assert(false);
}

/*
~Assigment operator~

*/
PBBox& PBBox::operator=(const PBBox& source){
   Equalize(&source);
   return *this;
}

/*
~Shift operator~

*/
ostream& operator<< (ostream &os, const PBBox BB){
   __TRACE__
 os << "PBBox[";
  if(!BB.IsDefined())
    os << "undefined]";
  else if (BB.IsEmpty())
    os << "empty]";
  else{
    os << "(" << BB.minX << "," << BB.minY << ")";
    os << "->";
    os << "(" << BB.maxX << "," << BB.maxY << ")";
  }
  return os;  
}



} // end of namespace periodic



