
/*
3.13 ~LinearPointMove~

*/

#include <iostream>
#include <string>

#include "PeriodicTypes.h"
#include "PeriodicSupport.h"

using namespace std;
using namespace datetime;
using namespace toprel;


namespace periodic{
/*
~Constructors~

*/
LinearPointMove::LinearPointMove(){}

LinearPointMove::LinearPointMove(const LinearPointMove& source): interval(0){
    Equalize(&source);
}

/*
~Destructor~

*/

LinearPointMove::~LinearPointMove(){}

/*
~Assigment operator~

*/
LinearPointMove& LinearPointMove::operator=(const LinearPointMove& source){
    Equalize(&source);
    return *this;
}


/*
~IsDefined~

*/
bool LinearPointMove::IsDefined()const{
  return defined;
}
/*
~IsStatic~

*/
bool LinearPointMove::IsStatic()const{
   return isStatic;
}

/*
~BoundingBox~

*/
PBBox LinearPointMove::BoundingBox()const{
  PBBox res(startX, startY, endX, endY);
  return res;
}


/*
~HashValue~

*/
size_t LinearPointMove::HashValue()const{
 if(!defined) return 0;
 return BoundingBox().HashValue() + interval.HashValue()+(size_t)startX;
}
/*
~Connected~

*/
bool LinearPointMove::Connected(LinearPointMove* P2){
    return (endX==P2->startX) & (endY == P2->startY);
}

/*
~Constructor~

This constructor creates a new LinearPointMOve ignoring the argument.

[3] O(1)

*/
LinearPointMove::LinearPointMove(int dummy): interval(1){
   __TRACE__
  isStatic=true;
  defined=true;
}

/*
~LinearPointMove::ToListExpr~

Converts a linear point unit into its nested list representation.

[3] O(1)

*/
ListExpr LinearPointMove::ToListExpr()const{
   __TRACE__
   if(defined)
        return ::nl->TwoElemList(
                      ::nl->SymbolAtom("linear"),
                      ::nl->ThreeElemList(
                            interval.ToListExpr(false),
                            ::nl->TwoElemList(
                                 ::nl->RealAtom((float)startX),
                                 ::nl->RealAtom((float)startY)),
                            ::nl->TwoElemList(
                                 ::nl->RealAtom((float)endX),
                                 ::nl->RealAtom((float)endY))));
    else
       return ::nl->TwoElemList(
                      ::nl->SymbolAtom("linear"),
                      ::nl->TwoElemList(
                                 interval.ToListExpr(false),
                                 ::nl->SymbolAtom("undefined")));
   }

/*
~LinearPointMove::ReadFrom~

Reads the content of this LinearPointMove from value.
The return values indicates the success.

[3] O(1)

*/
bool LinearPointMove::ReadFrom(const ListExpr value){
   __TRACE__
      ListExpr V = value;
       int L = ::nl->ListLength(V);
       if(L<2 || L>3)
          return false;
       if(!interval.ReadFrom(::nl->First(V),false))
          return false;

       if(L==2){ // potentially an undefined move
           if(::nl->IsEqual(::nl->Second(V),"undefined")){
               defined = false;
               isStatic = true;
               return true;
           }
           return false;
       }
       // L == 3
      ListExpr SP = ::nl->Second(V);
      ListExpr EP = ::nl->Third(V);
      double dv;
      if(!GetNumeric(::nl->First(SP),dv)){
        if(DEBUG_MODE){
          cerr << __POS__ << "StartX is not a number" << endl;
        }
        return false;
      }
      startX=dv;
      if(!GetNumeric(::nl->Second(SP),dv)){
        if(DEBUG_MODE){
          cerr << __POS__ << "StartY is not a number" << endl;
        }
        return false;
      }
      startY=dv;
      if(!GetNumeric(::nl->First(EP),dv)){
        if(DEBUG_MODE){
           cerr << __POS__ << "EndX is not a number" << endl;
        }
        return false;
      }
      endX=dv;
      if(!GetNumeric(::nl->Second(EP),dv)){
         if(DEBUG_MODE){
           cerr << __POS__ << "EndY is not a number" << endl;
         }
         return false;
      }
      endY= dv;
      defined=true;
      isStatic= (startX==endX) && (startY==endY);
      return true;
}

/*
~At~

Returns the position of this linear moving point at the given time.
It's required, that the time value is contained in the interval of this
linear moving point.

[3] O(1)

*/
void LinearPointMove::At(const DateTime* duration,Point& res) const{
   __TRACE__
   assert(interval.Contains(duration));
   assert(defined);
   double delta = interval.Where(duration);
   double x = startX+delta*(endX-startX);
   double y = startY+delta*(endY-startY);
   res.Set(x,y);
}

/*
~IsDefinedAt~

~IsDefinedAt~ checks whether this LinearPointMOve is defined
at the specified time.

[3] O(1)

*/
bool LinearPointMove::IsDefinedAt(const DateTime* duration)const{
   __TRACE__
     return interval.Contains(duration) && defined;
}

/*
~GetHalfSegment~

This function returns the trajectory of this LinearPointMove as an
HalfSegment.  If the LinearPointMove is not defined or static,
an undefined HalfSegment is returned.

[3] O(1)

*/
bool LinearPointMove::GetHalfSegment(
                       const bool LeftDominatingPoint,
                       HalfSegment& seg)const{
   __TRACE__
  if(defined && !isStatic){
      Point P1(true,startX,startY);
      Point P2(true,endX,endY);
      seg.Set(LeftDominatingPoint,P1,P2);
      return true;
   } else{
      return false;
   }
}

/*
~Intersects~

This function checks wether the trajectory of this linear
point move intersects __window__. This check is finer than the
check for intersection of the bounding boxes but can also
computed in constant time. Note that the constant time can not
ensured when another structures (like lines, points or regions)
are involved.

[3] O(1)

*/
bool LinearPointMove::Intersects(const PBBox* window)const{
   __TRACE__
    if(!BoundingBox().Intersects(window))
        return false;
      // Now, the position of all vertices of window related to the
      // segment defined by this moving point is computed .
      // This is done by using the formula of the directed area of the
      // triangles. We use only the signum of this area.
      double A,r1,r2;
      bool isLeft;
      double p1 = startX;
      double p2 = startY;
      double q1 = endX;
      double q2 = endY;
      window->GetVertex(0,r1,r2);
      A = (r1-p1)*(r2+p2)+(q1-r1)*(q2+r2)+(p1-q1)*(p2+q2);
      if(A==0)
         return true;
      isLeft=A<0;
      for(int i=1;i<4;i++){
        window->GetVertex(i,r1,r2);
        A = (r1-p1)*(r2+p2)+(q1-r1)*(q2+r2)+(p1-q1)*(p2+q2);
        if(A==0) return true; // a point on the segment is found
        if( (A<0)!=isLeft) // a point on the other side
                           // of the segment is found
           return true;
      }
      return false;
}


/*
~Split~

This function splits a moving point unit into two parts. The value of delta
has to be in the interval [0,1]. The return value states whether a rest exists.
The first part is the this instance the second part is returned by the argument
Rest. If holds delta==0 and the relative interval of this moving point is not
leftclosed, no rest exists and the result will be false. The same will be when
delta==1  and not rightclosed. This function can only used, when the interval is
bounded. But this is no restriction because a linear moving point can not have
any unbounded interval. If the closeFirst flag is true, the first part will be
rightClosed (if any rest) and the second part (rest) will be left-open. In the
other case the values are negated.

[3] O(1)

*/
bool LinearPointMove::Split(const double delta, const bool closeFirst,
                            LinearPointMove& Rest){
   __TRACE__
   assert((delta>=0) && (delta<=1));
    RelInterval inter = RelInterval(0);
    if(!interval.Split(delta,closeFirst,inter))
       return false;

    Rest.Equalize(this);
    Rest.interval.Equalize(&inter);
    double SplitX = startX+delta*(endX-startX);
    double SplitY = startY+delta*(endY-startY);
    endX = SplitX;
    endY = SplitY;
    Rest.startX = SplitX;
    Rest.startY = SplitY;
    return true;
}


/*
~SplitX~

Splits this unit into two ones at the horizontal line x==X.
If the splitLine is outside of the trajectory of this unit, nothing is
changed and false is returned.

[3] O(1)

*/
bool LinearPointMove::SplitX(const double X, const bool closeFirst,
                             LinearPointMove& Rest){
   __TRACE__
 if(startX==endX)
     return false;
  // start and endX are on the same side of the splitline
  if( ((X-startX)<0) && ((X-endX)<0))
     return false;
  if( ((X-startX)>0) && ((X-endX)>0))
     return false;

  double delta = (X - startX)/(endX-startX);
  return Split(delta,closeFirst,Rest);
}

/*
~SplitY~

Splits this unit into two ones at the vertical line y==Y.
If the splitLine is outside of the trajectory of this unit, nothing is
changed and false is returned.

[3] O(1)

*/
bool LinearPointMove::SplitY(const double Y, const bool closeFirst,
                             LinearPointMove& Rest){
   __TRACE__
 if(startY==endY)
     return false;
  // start and endX are on the same side of the splitline
  if( ((Y-startY)<0) && ((Y-endY)<0))
     return false;
  if( ((Y-startY)>0) && ((Y-endY)>0))
     return false;
  double delta = (Y - startY)/(endY-startY);
  return Split(delta,closeFirst,Rest);
}

/*
~SetUndefined~

This function makes this instance of a LinearPointMove undefined.

*/
void LinearPointMove::SetUndefined(){
   __TRACE__
     defined = false;
      isStatic=true;
}


/*
~CanBeExtendedBy~

This function yields true iff the Intervals can summarized,
The end point of this is the startpoint pf P2 and direction and
speed are equals.

[3] O(1)

*/
bool LinearPointMove::CanBeExtendedBy(const LinearPointMove* P2)const {
   __TRACE__
 // check the intervals
  if(!interval.CanAppended(&(P2->interval)))
    return false;
  // check the combinations of defined
  if(!defined && !P2->defined)
    return true;
  if(defined && !P2->defined)
    return false;
  if(!defined && P2->defined)
    return false;
  // check Start and Endpoint
  if(endX!=P2->startX)
    return false;
  if(endY!=P2->startY)
    return false;
  if(isStatic)
     return P2->isStatic;
  // check direction
    if(abs ( ((endX-startX)*(P2->endY-P2->startY))-
             ((P2->endX-P2->startX)*(endY-startY)))>EPSILON)
        return false;
  // check speed
  double L = sqrt( (startX-endX)*(startX-endX) + (startY-endY)*(startY-endY));
  double P2L = sqrt( (P2->startX-P2->endX)*(P2->startX-P2->endX) +
                     (P2->startY-P2->endY)*(P2->startY-P2->endY));
  DateTime* IL = interval.GetLength();
  double T = IL->ToDouble();
  delete IL;
  IL = NULL;
  DateTime* IL2 = ((P2->interval).GetLength());
  double P2T = IL2->ToDouble();
  delete IL2;
  IL2 = NULL;
  if( abs(L*P2T - P2L*T)>EPSILON)
     return false;
  return true;
}

/*
~ExtendWith~

This function concatenates the linear point moves if possible. If not, the
result will be false.

[3] O(1)

*/
bool LinearPointMove::ExtendWith(const LinearPointMove* P2){
   __TRACE__
  if(!CanBeExtendedBy(P2))
      return false;
   interval.Append(&(P2->interval));
   endX = P2->endX;
   endY = P2->endY;
   return true;
}

/*
~Equalize~

This LinearPointMove takes its value from the argument if this
function is called.

[3] O(1)

*/

void LinearPointMove::Equalize(const LinearPointMove* P2){
   __TRACE__
  interval.Equalize(&(P2->interval));
  startX = P2->startX;
  startY = P2->startY;
  endX = P2->endX;
  endY = P2->endY;
  isStatic = P2->isStatic;
  defined = P2->defined;
}


/*
~CompareTo~

This functions implements the familiar compare function.

*/
   int LinearPointMove::CompareTo(LinearPointMove* LPM){
       if(!defined && !LPM->defined){
          return 0;
       }
       if(!defined && LPM->defined){
          return -1;
       }
       if(defined && !LPM->defined){
          return  1;
       }
       int comp = interval.CompareTo(&(LPM->interval));
       if(comp!=0){
         return comp; // different intervals
       }
       if(startX < LPM->startX) return -1;
       if(startX > LPM->startX) return 1;
       if(startY < LPM->startY) return -1;
       if(startY > LPM->startY) return 1;
       if(endX < LPM->endX) return -1;
       if(endX > LPM->endX) return 1;
       if(endY < LPM->endY) return -1;
       if(endY > LPM->endY) return 1;
       return 0;
   }

/*
~CompareSpatial~

This function compares the spatial components of this instance
and the argument, i.e. this function works like the ~CompareTo~
function but ignores the time component of the arguments.

*/
int LinearPointMove::CompareSpatial(LinearPointMove* lpm){
       if(!defined && !lpm->defined)
          return 0;
       if(!defined && lpm->defined)
          return -1;
       if(defined && !lpm->defined)
          return  1;
       if(startX < lpm->startX) return -1;
       if(startX > lpm->startX) return 1;
       if(startY < lpm->startY) return -1;
       if(startY > lpm->startY) return 1;
       if(endX < lpm->endX) return -1;
       if(endX > lpm->endX) return 1;
       if(endY < lpm->endY) return -1;
       if(endY > lpm->endY) return 1;
       return 0;
}


/*
~ToString~

This function returns a string representation of this LinearPointsMove

*/
string LinearPointMove::ToString()const{
  ostringstream res;
  res << "(" << interval.ToString();
  if(!defined){
     res << " undefined)";
  } else {
     res << " (" << startX << ", " << startY <<") -> (";
     res << endX << ", " << endY << "))";
  }
  return res.str();
}

/*
~Toprel~

This function computes the evolution of the topological relationship
between this LinearPointMove and the given static point. The caller has
to allocate enough memory in the resultbuffer, this means that the
result array has a minimum size of 3. The return value is the number of
used array slots (1..3). The resulting LinearInt9M values covers the
same relative interval as this point it does.

[3] O(1)

*/
int LinearPointMove::Toprel(const Point P, LinearInt9MMove* result)const{
  // first handle undefined values
  Int9M R;
  if(!defined && !P.IsDefined()){
     R.Set(false,false,false,false,false,false,false,false,true);
     result[0].Set(R,interval);
     return 1;
  }
  if(!defined){
     R.Set(false,false,false,false,false,false,true,false,true);
     result[0].Set(R,interval);
     return 1;
  }
  if(!P.IsDefined()){
     R.Set(false,false,true,false,false,false,false,false,true);
     result[0].Set(R,interval);
     return 1;
  }

  // The point describes a vertical line in the three dimensional space
  // the linear moving point describes an arbitrary segment in this space
  // Because the coordinates of the time dimension is equal for both lines,
  // we can reduce the problem of the problem whether a point is located
  // on a segment.
  double dx = endX-startX;
  double dy = endY-startY;
  double x = P.GetX();
  double y = P.GetY();
  // two simple points, which are the case here, can only be equal
  // or disjoint; first we define the corresponding matrices
  Int9M Requal;
  Requal.Set(true,false,false,false,false,false,false,false,true);
  Int9M Rdisjoint;
  Rdisjoint.Set(false,false,true,false,false,false,true,false,true);
  // special case: moving point is static
  if((dx==0 && dy==0 ) || interval.GetLength()->ToDouble()==0){
     if( (x==startX) && (y==startY))
        result[0].Set(Requal,interval);
     else
        result[0].Set(Rdisjoint,interval);
     return 1;
  }

  // at this point we know that this point is moving
  // we can conclude that in at most one timepoint the
  // points are equal and they are disjoint in all other times

  // point, we need a closed interval of length 0
  DateTime DT(durationtype);
  DT.ReadFrom(0.0);
  RelInterval EmptyInt;
  EmptyInt.Set(&DT,true,true);

  // special case: P is startpoint of this
  if( (x==startX) && (y==startY)){
     if(!interval.IsLeftClosed()){
         result[0].Set(Rdisjoint,interval);
         return 1;
     }
     else{
         result[0].Set(Requal,EmptyInt);
         RelInterval FI;
         FI.Set(interval.GetLength(),false,interval.IsRightClosed());
         result[1].Set(Rdisjoint,FI);
         return 2;
     }
  }
  // special case: P is endpoint of this
  if( (x==endX) && (y==endY)){
    if(!interval.IsRightClosed()){
         result[0].Set(Rdisjoint,interval);
         return 1;
    }else{
       RelInterval I;
       I.Set(interval.GetLength(),interval.IsLeftClosed(),false);
       result[0].Set(Rdisjoint,I);
       result[1].Set(Requal,EmptyInt);
       return 2;
    }
  }

  // check wether P is on the segment
  if( (x-startX)*dy == (y-startY)*dx){ // on the line ?
     // one of dx and dy is different to 0.0 because this case is
     // aready handled above

     double delta = dx==0?(y-startY)/dy:(x-startX)/dx;
     if(delta<0 || delta>1){ // not on the segment
         result[0].Set(Rdisjoint,interval);
         return 1;
     }
     RelInterval I1;
     RelInterval I3;
     I1.Equalize(&interval);
     I1.Split(delta,true,I3);
     I1.Set(I1.GetLength(),I1.IsLeftClosed(),false);
     DateTime ST(instanttype);
     result[0].Set(Rdisjoint,I1);
     result[1].Set(Requal,EmptyInt);
     result[2].Set(Rdisjoint,I3);
     return 3;
  }else{ //point not on the segment
     result[0].Set(Rdisjoint,interval);
     return 1;
 }
}

/*
~Toprel~

This function computes the moving topological relationship between this
LinearPointMove and a set of points given by __P__. The number of
resulting LinearInt9MMove's depends linearly on the number of the
points contained in __P__. Because the size of the result is not
constant, we use a vector for storing it.

*/
void LinearPointMove::Toprel(const Points& P,
                             vector<LinearInt9MMove>& Result)const{
   Result.clear();
   Int9M R;
   if(!defined && P.IsEmpty()){
       R.Set(false,false,false,false,false,false,false,false,true);
       LinearInt9MMove res1(0);
       res1.Set(R,interval);
       Result.push_back(res1);
       return;
   }
   if(!defined && !P.IsEmpty()){
       R.Set(false,false,false,false,false,false,true,false,true);
       LinearInt9MMove res1(0);
       res1.Set(R,interval);
       Result.push_back(res1);
       return;
   }
   // the pmpoint is defined
   if(P.IsEmpty()){
       R.Set(false,false,true,false,false,false,false,false,true);
       LinearInt9MMove res1(0);
       res1.Set(R,interval);
       Result.push_back(res1);
       return;
   }
   // the pmpoint and the points value contain elements
   assert(P.IsOrdered());
   /* If the points value contains more than one point, the
      9 intersection matrix will contain an 1 entry at position
      interior exterior at each position. In the other case, it will
      have an 1 entry either at this position or at the position
      interior/interior
   */
   R.Set(false,false,false,false,false,false,false,false,true);
   // special case, the points value contains a single point
   int size = P.Size();
   if(size>1){
      R.SetEI(true);
   } else{
     LinearInt9MMove buffer[3];
     Point tP2;
     P.Get(0,tP2);
     int n = Toprel(tP2,buffer);
     for(int i=0;i<n;i++)
        Result.push_back(buffer[i]);
     return;
   }

   // special case: point is staying
   if((startX==endX && startY == endY) ||
       interval.GetLength()->ToDouble()==0){
        Point tmpP(startX,startY);
        if(P.Contains(tmpP))
             R.SetII(true);
        LinearInt9MMove r1(1);
        r1.Set(R,interval);
        Result.push_back(r1);
        return;
   }
   // we can scan all point in a direction to get the split point ordered
   // first, we have to determine the direction;

  // we create an interval of length zero
   DateTime DT(durationtype);
   DT.ReadFrom(0.0);
   RelInterval emptyInterval;
   emptyInterval.Set(&DT,true,true);

   bool forward;
   if(startX<endX)
      forward=true;
   else if(startX==endX)
      forward = startY<endY;
   else // startX>endX
      forward = false;


   Point currentPoint1;
   Point currentPoint2;
   RelInterval lastInterval = interval;
   RelInterval newInterval;
   double lastX1 = startX;
   double lastY1 = startY;
   double currentDelta=0;
   bool done = false;
   double currentX, currentY;
   LinearInt9MMove currentMove;
   for(int i=0;i<size && !done;i++){
      if(forward){
          P.Get(i,currentPoint1);
          currentPoint2 = (currentPoint1);
      }
      else{
          P.Get(size-i-1,currentPoint1);
          currentPoint2 = (currentPoint1);
      }

      currentX = currentPoint2.GetX();
      currentY = currentPoint2.GetY();
      currentDelta=PointPosOnSegment(lastX1,lastY1,endX,endY,
                                      currentX,currentY);
       if(currentDelta==0){ // split at starting point
         // split the interval at the begin
         if(lastInterval.IsLeftClosed()){
            R.SetII(true);
            currentMove.Set(R,emptyInterval);
            Result.push_back(currentMove);
            lastInterval.Set(lastInterval.GetLength(),false,
                             lastInterval.IsRightClosed());
         }
       }  else if(currentDelta>0 && currentDelta<1){ // proper split
             R.SetII(false);
            lastInterval.Split(currentDelta,false,newInterval);
            currentMove.Set(R,lastInterval);
            Result.push_back(currentMove);
            R.SetII(true);
            currentMove.Set(R,emptyInterval);
            Result.push_back(currentMove);
            newInterval.Set(newInterval.GetLength(),false,
                            newInterval.IsRightClosed());
            lastInterval=newInterval;
            lastX1 = currentX;
            lastY1 = currentY;
       } else if(currentDelta==1){ // split at endpoint
            if(lastInterval.IsRightClosed()){
               R.SetII(false);
               lastInterval.Set(lastInterval.GetLength(),
                                lastInterval.IsLeftClosed(),false);
               currentMove.Set(R,lastInterval);
               Result.push_back(currentMove);
               R.SetII(true);
               currentMove.Set(R,emptyInterval);
               Result.push_back(currentMove);
            } else{
               R.SetII(false);
               currentMove.Set(R,lastInterval);
               Result.push_back(currentMove);
            }
            done = true;
      } else if(currentDelta>1){ // no more splitpoints are possible
           R.SetII(false);
           currentMove.Set(R,lastInterval);
           Result.push_back(currentMove);
           done=true;
       }
    }
   if(!done) { // there is an unprocessed rest from the interval
       R.SetII(false);
       currentMove.Set(R,lastInterval);
       Result.push_back(currentMove);
   }
}

/*
~DistanceTo~

This function computes the distance of this LinearPointMove to
a fixed point. The result will be of type MovingRealUnit.If this
unit is not defined, also the reault will be not defined. In each
case, the interval of the result will be the same like the interval
of this unit.

*/

   void LinearPointMove::DistanceTo(const double x, const double y,
                                    MovingRealUnit& result)const{
      if(!defined){
        result.GetFrom(0,0,interval);
        result.SetDefined(false);
        return;
      }
      double startDist = sqrt( (startX-x)*(startX-x)+(startY-y)*(startY-y));
      double endDist = sqrt( (endX-x)*(endX-x)+(endY-y)*(endY-y));
      result.GetFrom(startDist,endDist,interval);
   }


/*
~NearOverlap~

This function checks whether this unit and the argument have an nearly
common part. I.e. first is checked if the speed difference between both
units is smaller than the given epsilon value. Additionally, the distance
between both units must be smaller than the given epsilon value.

*/
bool LinearPointMove::nearOverlap(const LinearPointMove& lpm,
                 const double epsilonSpeed,
                 const double epsilonDirection,
                 const double epsilonSpatial) const{

   if(epsilonSpeed<0 || epsilonSpatial<0 || epsilonDirection<0){
       // not possible
       return false;
   }
   double speed1 = this->Speed();
   double speed2 = lpm.Speed();
   if(abs(speed1-speed2)>epsilonSpeed){
       return false;
   }

   if(speed1>epsilonSpeed || speed2>epsilonSpeed){
      // check directions
      double dir1 = this->Direction();
      double dir2 = lpm.Direction();
      if(abs(dir1-dir2)>epsilonDirection){
          return false;
      }
   } else if(speed1<=epsilonSpeed || speed2<epsilonSpeed){
       // from one arguments, the direction is not evaluable
       return false;
   }

   // direction and speed are nearly equal, check the distance
   HalfSegment hs1;
   this->GetHalfSegment(true,hs1);
   HalfSegment hs2;
   lpm.GetHalfSegment(true,hs2);
   double dist = hs2.Distance(hs2);
   return dist<=epsilonSpatial;

}

/*
~Direction~

Computes the direction as angle between 0 and 360 degrees. If the point
is not moving (static or undefined), the direction will have the value -1.

*/
double LinearPointMove::Direction()const{
   if(isStatic || !defined){
     return -1;
   }
   double x = endX-startX;
   double y = endY-startY;
   double len = this->Length();
   y = y/len;
   double angle1 = asin(y);
   angle1 = angle1*180.0 / M_PI;
   if(y>=0 && x>=0){
       return angle1;
   }
   if(y<0 && x>=0){ // don't allow negative values
      return 360 + angle1;
   }
   return 180 - angle1;
}

/*
~Speed~

Computes the speed of this unit as distance / time. The optional argument is multiplied
with the time value. Basically, the speed is given in units per day.

*/

double LinearPointMove::Speed(unsigned int timefactor) const{
  assert(timefactor>0);
  if(!defined){
      return 0;
  }

  double time = interval.GetLength()->ToDouble();
  if(time<=0){
     return 0; // no time no speed
  }
  time = time*(double) timefactor;
  return this->Length()/time;
}


void LinearPointMove::Speed(MovingRealUnit& result) const{
   if(!defined){
      result.SetDefined(false);
      return;
   }
   MRealMap map(0,0,Speed(1),false);
   MovingRealUnit unit(map,interval);
   unit.SetDefined(true);
   result.Equalize(&unit);
}


void LinearPointMove::Direction(MovingRealUnit& result) const{
   double dir = Direction();
   MRealMap map(0,0,dir,false);
   MovingRealUnit unit(map,interval);
   unit.SetDefined(dir>=0);
   result.Equalize(&unit);
}


/*
~Length~

This function computes the length of the segment given by the spatial projection
of this LinearPointMOve.

*/
double LinearPointMove::Length() const{
   if(!defined){
     return 0.0;
   }
   double distX = startX-endX;
   double distY = startY-endY;
   double dist = sqrt( distX*distX + distY*distY);
   return dist;
}



/*
~A stream operator~

This operator can be used for debugging purposes. Its defines
a formatted output for a LinearPointMove.

*/

ostream& operator<<(ostream& os, const LinearPointMove LPM){
      os <<  LPM.ToString();
      return os;
   }

} // end of namepsace periodic
