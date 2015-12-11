
/*
3.12 ~MovingRealUnit~


*/

#include <iostream>
#include <string>

#include "NestedList.h"
#include "PeriodicSupport.h"
#include "PeriodicTypes.h"
#include "DateTime.h"

extern NestedList* nl;
using namespace std;
using namespace datetime;

namespace periodic{


/*
~constructor~

This constructor does nothing.

*/
MovingRealUnit::MovingRealUnit(){}


/*
~constructor~

This constructor creates a MovingRealUnit with the given values for the
interval and for the map.

*/
MovingRealUnit::MovingRealUnit(MRealMap map, RelInterval interval){
    Set(map,interval);
}

/*
~constructor~ 

This constructor creates a MovingRealUnit representing the constant value 
zero. The length of the interval will also be zero.

*/

MovingRealUnit::MovingRealUnit(int dummy):interval(dummy),map(dummy){
}

/*
~Copy Constructor~

*/
MovingRealUnit::MovingRealUnit(const MovingRealUnit& source){
   Equalize(&source);
}

/*
~Destrucor~

*/
MovingRealUnit::~MovingRealUnit(){}

/*
~Assignment Operator~

*/
MovingRealUnit& MovingRealUnit::operator=(const MovingRealUnit& source){
   Equalize(&source);
   return *this;
}

/*
~Set function~

This functions sets the values of the internal variables to defined values.

*/
void MovingRealUnit::Set(MRealMap map, RelInterval interval){
   this->map.Equalize(&map);
   this->interval.Equalize(&interval);
   defined=true;
}


/*
~Min~ and ~Max~

Returns the minimum value and the maximum value respectively.
The unit has be be defined.

*/
 double MovingRealUnit::min() const{
    assert(defined);
    DateTime* length = interval.GetLength();
    DateTime* start = new DateTime(durationtype); 
    double v1;
    At(start,v1);
    double v2;
    At(length,v2);
    double v3 = v1;
    DateTime pos = map.ExtremumAt();
    if(pos.IsDefined()){
        if(!pos.LessThanZero() && ( pos < (*length))){
           At(&pos,v3);
        }     
    }
    delete length;
    delete start;
    return ::max(v1,::max(v2,v3));
 }

 double MovingRealUnit::max() const{
    assert(defined);
    DateTime* length = interval.GetLength();
    DateTime* start = new DateTime(durationtype); 
    double v1;
    At(start,v1);
    double v2;
    At(length,v2);
    double v3 = v1;
    DateTime pos = map.ExtremumAt();
    if(pos.IsDefined()){
        if(!pos.LessThanZero() && ( pos < (*length))){
           At(&pos,v3);
        }     
    }
    delete length;
    delete start;
    return ::min(v1,::min(v2,v3));
 }


/*
~GetFrom Function~

When this function is called, the value of this unit is computed to 
change from value __start__ to value __end__ whithin __interval__
in a linear way. If __start__ and __end__ differs, the length of the 
interval must be greater than zero. For equal value also length zero
is allowed. If the interval does not fullfills this conditions, the
value of this unit remains unchanged. The return value indicates 
the success of the call of this function. 

*/
bool MovingRealUnit::GetFrom(double start, 
                             double end, 
                             RelInterval interval){
 // check the preconditions
 DateTime length(durationtype);
 interval.StoreLength(length);
 if(length.LessThanZero()) 
   return false;
 if(length.IsZero()){
    if(start!=end)
       return false;
    if(!interval.IsRightClosed() || !interval.IsLeftClosed())
       return false;  
 }
 // special case, no change while interval
 if(start==end){
    map.Set(0,0,start,false);
    this->interval.Equalize(&interval);
    defined = true;
    return true;
 }
 double b = (end-start) / (length.ToDouble());
 map.Set(0,b,start,false);
 defined = true;
 this->interval.Equalize(&interval);
 return true;
} 

/*
~At~

This function computes the value of this unit for a given instant. 
This unit must be defined at this instant. This should be checked
by calling the ~IsDefinedAt~ function before. 

*/
void MovingRealUnit::At(const DateTime* duration,double& res) const {
    assert(interval.Contains(duration,true));
    assert(defined);
    assert(map.IsDefinedAt(duration));
    res =  map.At(duration);
}


/*
~IsDefinedAt~

This function cvhecks whether this unit is defined at the given point in time. 

*/
bool MovingRealUnit::IsDefinedAt(const DateTime* duration) const{
  return interval.Contains(duration) &&
         defined &&
         map.IsDefinedAt(duration);
}

/*
~ToListExpr~

This function computes the representation of this unit in nested list format.

*/
ListExpr MovingRealUnit::ToListExpr()const{
 if(!defined)
      return ::nl->TwoElemList(::nl->SymbolAtom("linear"),
                             ::nl->TwoElemList(
                               interval.ToListExpr(false),
                               ::nl->SymbolAtom("undefined")));
 return ::nl->TwoElemList(::nl->SymbolAtom("linear"),
                        ::nl->TwoElemList(
                            interval.ToListExpr(false),
                            map.ToListExpr()));
      
}

/*
~ReadFrom~

If the argument contains a valid representation of a moving real unit,
the value of this unit is changed to the value from the argument. Otherwise,
this unit remains unchanged.

*/
bool MovingRealUnit::ReadFrom(ListExpr le){
  if(::nl->ListLength(le)!=2)
      return false;
  if(::nl->AtomType(::nl->First(le))!=SymbolType)
      return false;
  if(::nl->SymbolValue(::nl->First(le))!="linear")
      return false;
  ListExpr v = ::nl->Second(le);
  if(::nl->ListLength(v)!=2)
      return false;
  RelInterval tmpinterval;
  if(!tmpinterval.ReadFrom(::nl->First(v),false))
      return false;
  if(!map.ReadFrom(::nl->Second(v)))
     return false;
  interval.Equalize(&tmpinterval);
  defined=true;
  return true;
}

/*
~CanSummarized~

This function checkes whether two moving real units can summarized. This means, the
intervals are adjacent and the coefficients are choosen that the functions are equal.

*/

bool MovingRealUnit::CanSummarized(const MovingRealUnit* MRU) const{
   if(!interval.CanAppended(&(MRU->interval)))  
      return false;
   DateTime * DT = interval.GetLength();
   double d = DT->ToDouble();
   delete DT;
   DT = NULL;
   return map.EqualsTo(MRU->map,d);
}

/*
~Initial~

This function returns the value of this unit at the start of its interval.

*/
bool MovingRealUnit::Initial(double& res)const{
      if(!map.IsDefinedAt(0.0)){
        return false;
      } else {
        res = (map.At(0.0)); 
        return true;
     }
}

/*
~Final~

This function returns the value of this unit at the end of its interval.

*/ 
bool MovingRealUnit::Final(double& res)const{
    DateTime* end = interval.GetLength();
    if(!map.IsDefinedAt(end))
         return false;
    else
        res = (map.At(end)); 
    delete end;
    end = NULL;
    return true;
}



/*
~Split~

This function splits this unit into 2 ones. The splitting point is 
given by the argument __duration__. The left part will be the 
__this__ objects, the right part will be stored in the argument __right__.
The affiliation of the splitting point is ruled by the parameter __toLeft__.
The function will return __true__ if this unit can splittet, i.e. the 
splitting point is contained in this unit. In the other case, this unit 
left as it is, the __right__ parameter is set to be undefined, and the 
return value will be __false__.

*/

bool MovingRealUnit::Split(const DateTime duration, 
                           const bool toLeft, 
                           MovingRealUnit& unit){
   // case duration not contained
   if(!interval.Contains(&duration)){
      unit.defined=false;
      return false;
   }
   // case left will be empty
   if(duration.IsZero() && !toLeft){
      unit.defined=false;
      return false;
   }
   // all ok, split this unit
   if(!interval.Split(duration,toLeft,unit.interval)){
      unit.defined=true;
      return false;
   }
   unit.defined=true;
   double d = duration.ToDouble();
   unit.map.Set(map.a,map.b-2*d, d*d-d*map.b+map.c,map.root); 
   return true;
} 


/*
~SetDefined~

Using this function, we can set the defined flag for this instance.
Use carefully this function with an value of true. The internal stored 
values are nmot checked to hold meaningful values, just the flag is
changed.

*/

void MovingRealUnit::SetDefined(const bool defined){
  this->defined=defined;
}

void MovingRealUnit::Equalize(const MovingRealUnit* source){
   interval.Equalize(&(source->interval));
   map.Equalize(&(source->map));
   defined = source->defined;
}

} // end of namespace periodic
