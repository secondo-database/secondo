/*
3.15 ~LinearPointsMove~ 

*/

#include <iostream>
#include <string>

#include "NestedList.h"
#include "StandardTypes.h"
#include "PeriodicSupport.h"
#include "PeriodicTypes.h"
#include "DateTime.h"
#include "SpatialAlgebra.h"


using namespace std;
extern NestedList* nl;

namespace periodic{

/*
~Constructor~

*/
LinearPointsMove::LinearPointsMove(){}

LinearPointsMove::LinearPointsMove(const LinearPointsMove& source):
   interval(0), bbox(0){
   Equalize(&source);
}

/*
~Destructor~

*/
LinearPointsMove::~LinearPointsMove(){}

/*
~Assignment Operator~

*/
LinearPointsMove& LinearPointsMove::operator=(const LinearPointsMove& source){
   Equalize(&source);
   return *this;
}

/*
~IsDefined~

*/
bool LinearPointsMove::IsDefined()const{
   return defined;
}
/*
~IsStatic~

*/
bool LinearPointsMove::IsStatic()const{
   return isStatic;
}

/*
~Accessing the occupied array range~

*/

unsigned int LinearPointsMove::GetStartIndex()const { 
   return startIndex; 
}

unsigned int LinearPointsMove::GetEndIndex()const {
  return endIndex; 
} 

/*
~Constructor~

This Constructor creates an undefined instance of this class.

[3] O(1)

*/
LinearPointsMove::LinearPointsMove(int dummy):interval(0),bbox(0){
  __TRACE__
  startIndex=0;
  endIndex=0;
  isStatic = true;
  defined = false;
}
  
/*
~ToListEpr~

This function returns the nested list corresponding to this 
LinearPointsMove. Because this class can't manage the location
of the points, the array containing the points is given as an
argument. If the array not contains enough entries, the result 
will be zero.

[3] O(P) where P is the number of contained moving points

*/  
ListExpr LinearPointsMove::ToListExpr(const DbArray<TwoPoints> &Points) const{
   __TRACE__
   if(!defined)
      return ::nl->SymbolAtom("undefined");
   TwoPoints Entry;
   unsigned int s = Points.Size();
   if(s<endIndex){
      if(DEBUG_MODE){
        cerr << "Error in creating the nested list for";
        cerr << "a LinearPointsMove " << endl;
        cerr << "Endindex (" << endIndex << ") ";
        cerr << "is located after the last ArrayElement (";
        cerr << Points.Size() << ")" << endl;
      }
      return 0;
   }   
   if(endIndex<=startIndex){
      cerr << __POS__ << "endIndex < startIndex " << endl;  
      return 0;
   }


   ListExpr starts, ends;
   ListExpr Ps,Pe,LastStart,LastEnd;

   Points.Get(startIndex,Entry);   
   Ps = ::nl->TwoElemList( ::nl->RealAtom(Entry.startX),
                         ::nl->RealAtom(Entry.startY));
   Pe = ::nl->TwoElemList( ::nl->RealAtom(Entry.endX),
                         ::nl->RealAtom(Entry.endY));
   starts = ::nl->OneElemList(Ps);
   ends = ::nl->OneElemList(Pe);
   LastStart = starts;
   LastEnd = ends;

   for(unsigned int i=startIndex+1;i<endIndex;i++){
      Points.Get(i,Entry);
      Ps = ::nl->TwoElemList( ::nl->RealAtom(Entry.startX),
                            ::nl->RealAtom(Entry.startY));
      Pe = ::nl->TwoElemList( ::nl->RealAtom(Entry.endX),
                            ::nl->RealAtom(Entry.endY));
      LastStart = ::nl->Append(LastStart,Ps);
      LastEnd = ::nl->Append(LastEnd,Pe); 
   }

   ListExpr res = ::nl->TwoElemList(::nl->SymbolAtom("linear"),
                         ::nl->ThreeElemList( interval.ToListExpr(false),
                                     starts,ends));        
   return res;             
}
   
/*
~ReadFrom~

This function reads the value of this LinearPointsMove from 
value. The points are appended to the Points array beginning at
the given index. If value don't represent a valid LinearPointsMove, 
the return value will be false. Nevertheless it may be that the Points
array is changed. After calling this function the index value will be 
points to the elements after the last inserted one.
If an error is occured this instance will be undefined.

[3] O(P) where P is the number of contained moving points

*/
bool LinearPointsMove::ReadFrom(const ListExpr value, DbArray<TwoPoints> &Pts,
                                int &Index){
   __TRACE__
// value has to be a list
 if(::nl->AtomType(value)!=NoAtom){
   if(DEBUG_MODE){
     cerr << "List for LinearPointsMove is an atom " << endl;
   }
   defined = false;
   return false;
 }  
 // check the ListLength
 if(::nl->ListLength(value)!=3){ // (interval starts ends)
    if(DEBUG_MODE){
      cerr << "Wrong listlength for LinearPointsMove; should be 3, is ";
      cerr << ::nl->ListLength(value) << endl; 
    }
    defined = false;
    return false;
 }   
 // read the Interval
 if(!interval.ReadFrom(::nl->First(value),false)){
    if(DEBUG_MODE){
      cerr << "LinearPointsMove::ReadFFrom: error in reding interval" << endl;
    }
    defined = false;
    return false;
 }   
 ListExpr starts = ::nl->Second(value);
 ListExpr ends   = ::nl->Third(value);
 int L1 = ::nl->ListLength(starts);
 if(L1 != ::nl->ListLength(ends)){
   if(DEBUG_MODE){
     cerr << "Error in LinearPointsMove::ReadFrom" << endl;
     cerr << "Different lengths for start and end points " << endl;
   }
   return false;
 }   
 if(L1==0){
    isStatic = true;
    startIndex = 0;
    endIndex = 0;
    defined = true;
    bbox.SetEmpty();
    return  true;
 }
 
 double xs,ys,xe,ye;
 TwoPoints TP(0,0,0,0);
 ListExpr Start,End;
 startIndex = Index;
 endIndex = Index+L1;
 while(! ::nl->IsEmpty(starts)){
    Start = ::nl->First(starts);
    End = ::nl->First(ends);
    if(::nl->AtomType(Start)!=NoAtom  ||
       ::nl->AtomType(End)!=NoAtom){
       if(DEBUG_MODE){
          cerr << "Error in LinearPointsMove::ReadFrom" << endl;
          cerr << "start or end is not a list" << endl;
       }
       defined = false;
       return false;
    }   
    if(::nl->ListLength(Start)!=2 ||
       ::nl->ListLength(End)!=2) {
       if(DEBUG_MODE){
          cerr << "Error in LinearPointsMove::ReadFrom" << endl;
          cerr << "Wrong listlength for Start or for End " << endl;
    cerr << "Expected 2 for both, received " << ::nl->ListLength(Start);
    cerr << "(start) and "<<::nl->ListLength(End) << "(end)" << endl;
       }
       defined = false;  
       return false;
    }   
    if(!periodic::GetNumeric(::nl->First(Start),xs) ||
       !periodic::GetNumeric(::nl->Second(Start),ys) ||
       !periodic::GetNumeric(::nl->First(End),xe) ||
       !periodic::GetNumeric(::nl->Second(End),ye)){
       if(DEBUG_MODE){
          cerr << "Error in LinearPointsMove::ReadFrom" << endl;
          cerr << "Cannot find numerical value " << endl;
       }
       defined = false;
       return false;
    }
    
    TP = TwoPoints(xs,ys,xe,ye);
    bbox.Union(xe,ye);
    bbox.Union(xs,ys);
    if(xe!=xs || ye!=ys)
       isStatic=false;
    Pts.Put(Index,TP);        
    Index++;
    starts=::nl->Rest(starts);
    ends=::nl->Rest(ends);
 }     
 
 defined = true;
 return true;
}


/*
~At~

This function computes the locations of the contained points
at the given point of time. If the duration is outside of the relative
interval of this LinearPointsMove, NULL is returned.

[3]  O(P log(P)) where P is the number of contained moving points.
     The reason is the sorting of the points in the result. I'm not
     sure whether this time is reached because i don't know the 
     implementation of the sorting of the points.

*/
void LinearPointsMove::At(const DateTime* duration,
                         const DbArray<TwoPoints> &Pts,
                         Points& res) const{
   __TRACE__
  if(!interval.Contains(duration)){
    res.SetDefined(false);
    return;
  }
  res.SetDefined(true);
  double w = interval.Where(duration);   
  res.Resize(Pts.Size());
  TwoPoints Element;
  Point aPoint;
  double x,y;
  res.StartBulkLoad();
  for(unsigned int i=startIndex;i<endIndex;i++){
    Pts.Get(i,Element);
    x = Element.startX + w * (Element.endX - Element.startX);
    y = Element.startY + w * (Element.endY - Element.startY); 
    aPoint = Point(x,y);
    res += aPoint; 
  }  
  res.EndBulkLoad();
}

   
/*
~IsDefinedAt~

The function ~IsDefinedAt~ checks whether the given duration is
part of the relative interval of this LinearPointsMove. This means,
if the duration is zero, the relative interval has to be leftclosed.
If the given duration is less than zero, false is returned. Otherwise
this function checks wether the duration is less or equlas to the duration
of the relative interval. In the case of equality, the relative interval
has to be rightclosed to get the result true. 

[3] O(1)

*/ 
bool LinearPointsMove::IsDefinedAt(const DateTime* duration)const{
   __TRACE__
  return interval.Contains(duration);
}

/*
~Intersects~

This function checks whether the bounding box of this
LinearPointsMove has common points with the bounding box which is
given as an arguments.

[3] O(1)

*/
bool LinearPointsMove::ProhablyIntersects(const PBBox* window)const {
   __TRACE__
  return bbox.Intersects(window);
}

/*
  ~SetUndefined~

This function sets this LinearPointsMove to be undefined.

[3] O(1)

*/
void LinearPointsMove::SetUndefined(){
   __TRACE__
  defined = false;
}


/*
~CanBeExtendedBy~

This function checks whether this LinearPointsmove can be appended
by the one of the arguments to yielding a single LinearPointsMove.

[3] unknow because not implemented up to now

*/
bool LinearPointsMove::CanBeExtendedBy(const LinearPointsMove* P2,
                                    DbArray<TwoPoints> &MyPoints,
                                    DbArray<TwoPoints> &PointsOfP2) const{
   __TRACE__
    if(!interval.CanAppended(&(P2->interval)))
        return false;
     // First we check for defined
     if(!defined && !P2->defined)
        return true;
     if(defined != P2->defined)
        return false;
     // now both LinearPointsMove are defined
     // check for the same number of contained points
     if( (endIndex-startIndex) != (P2->endIndex-P2->startIndex))
         return false;
     // now we have to check wether each endpoint of this LinearPointsMove
     // has a matching start point in P2.
     // To do this, we insert all endpoints of this into an array and
     // all start points of P2 to another one. After sorting both arrays
     // we have to check the equality of this ones.
     int size = endIndex-startIndex;
     SimplePoint endPoints[size];
     SimplePoint startPoints[size];
     TwoPoints CurrentPoint;
     for(unsigned int i=startIndex;i<endIndex;i++){
        MyPoints.Get(i,CurrentPoint);
        endPoints[i-startIndex].x=CurrentPoint.endX;
        endPoints[i-startIndex].y=CurrentPoint.endY;
        endPoints[i-startIndex].intinfo=i;
        endPoints[i-startIndex].boolinfo=false;
     }
     for(unsigned int i=P2->startIndex;i<P2->endIndex;i++){
        PointsOfP2.Get(i,CurrentPoint);
        startPoints[i-P2->startIndex].x=CurrentPoint.startX;
        startPoints[i-P2->startIndex].y=CurrentPoint.startY;
        startPoints[i-P2->startIndex].intinfo=i;
        startPoints[i-P2->startIndex].boolinfo=false;
     }
     heapsort(size,endPoints);
     heapsort(size,startPoints);
     for(int i=0;i<size;i++){
         if(endPoints[i] != startPoints[i])
            return false;
     }
     // we check for same speed and direction of the points
     bool found = false; // found a matching (extending) upoint
     int pos,cpos;
     pos=cpos=0;
     int endIndex;
     TwoPoints TestPoint;
     bool morethanone;
     for(int i=0;i<size;i++){
        endIndex=endPoints[i].intinfo;
        //startIndex=startPoints[pos].intinfo;
        MyPoints.Get(endIndex,CurrentPoint);
        // check all Points of P2 with the same startpoint like the current
        // endpoint
        if(pos<size-1)
           morethanone=startPoints[pos]==startPoints[pos+1];
        else
           morethanone=false;
        cpos = pos; 
        while(!found && cpos<size){
            if(endPoints[i]!=startPoints[cpos]) return false; // nothing found
            if(startPoints[cpos].boolinfo) // point allready used
                 cpos++;
            else{
                PointsOfP2.Get(cpos,TestPoint);
                if(CurrentPoint.IsSpatialExtension(&TestPoint)){ 
                   // check for same speed
                   if(CurrentPoint.Speed(interval)==
                      TestPoint.Speed(P2->interval)){ //mathc found
                         startPoints[cpos].boolinfo=true; // mark as used
                         found = true;
                   }
                }else{
                   cpos++; // try the next
                }
            } 
        } 
        if(!found) return false; 
        if(!morethanone) pos++; 
     }

     return true; 
}
  
/*
~ExtendsWith~

This function summarized two LinearPointsMove together to a
single one if possible. The return values indicates the
success of this function. The result shows whether the
LinearPointsMoves can be put together. If the result is
__false__, this LinearPointsMove is not changed.

*/ 
bool LinearPointsMove::ExtendWith(const LinearPointsMove* P2,
                                  DbArray<TwoPoints> &MyPoints,
                                  DbArray<TwoPoints> &PointsOfP2){
   __TRACE__
 if(!CanBeExtendedBy(P2,MyPoints,PointsOfP2)) return false;
  interval.Append(&(P2->interval));
  bbox.Union(&(P2->bbox));
  // this is very closed to the CanBeExtededFunction but here we need no
  // checks because we know that this units can be putted together.
  cout << __POS__ << endl;
  cout << "ExtendWith(...) notimplemented " << endl;
  return false;
}

/*
~Equalize~

This functions makes this equal to the argument.

[3] O(1)

*/
void LinearPointsMove::Equalize(const LinearPointsMove* P2){
   __TRACE__
  interval.Equalize(&(P2->interval));
   bbox.Equalize(&(P2->bbox));
   startIndex=P2->startIndex;
   endIndex=P2->endIndex;
   isStatic = P2->isStatic;
   defined = P2->defined;
}



/*
~Shift Operator~

*/
ostream& operator<< (ostream& os, const LinearPointsMove LM){
   __TRACE__
  os << "LPsM[" ;
   os << LM.startIndex << "," << LM.endIndex << "]";
   // add also bbox, interval isstatic and so on if required !
   return os;
}


} // end of namespace 
