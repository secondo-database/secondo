
/*
1.1 ~LinearConstantMove~

*/

#ifndef LINEARCONSTANTMOVE_H
#define LINEARCONSTANTMOVE_H

#include <string>
#include <iostream>

#include "NestedList.h"

#include "PeriodicTypes.h"

extern NestedList* nl;


namespace periodic{

/*
~Constructor~

This constructor is used in the cast function. It does nothing
and should not be used at other places.

*/
template <class T>
LinearConstantMove<T>::LinearConstantMove(){}

/*
~Constructor~

This constructor creates a new unit with given value at an
interval with length zero.

*/
template<class T>
LinearConstantMove<T>::LinearConstantMove(const T value):interval(0) {
  this->value = value;
  defined=true;
}

/*
~Constructor~

This constructor creates a new unit from the given value and
the given interval.

*/
template <class T>
LinearConstantMove<T>::LinearConstantMove(const T value, 
                 const RelInterval interval): interval(interval){
  this->value=value;
  this->defined=true;
}
/*
~Copy Constructor~

*/
template <class T>
LinearConstantMove<T>::LinearConstantMove(
     const LinearConstantMove<T>& source){
   Equalize(&source);
}

/*
~Destructor~

The destructor destroys this object. because we don't have any 
pointer structures, this destructor makes nothing.

*/
template <class T>
LinearConstantMove<T>::~LinearConstantMove(){}


/*
~Assignment Operator~

*/
template <class T>
LinearConstantMove<T>& 
     LinearConstantMove<T>::operator=(const LinearConstantMove<T> source){
   Equalize(&source);
   return *this;
}

/*
~Set function~

This function sets the value as well as the interval of this unit
to the given values.

*/
template <class T>
void LinearConstantMove<T>::Set(const T value, const RelInterval interval){
  this->value=value;
  this->interval.Equalize(&interval);
  this->defined=true;
}


/*
~At function~

This function returns just the value of this unit. 
This function can't be called when the duration is
outside the interval or when this unit is not defined.
This should be checked by calling the isDefinedAt function.

*/
template <class T>
void LinearConstantMove<T>::At(
        const datetime::DateTime* duration,T& res) const {
   assert(interval.Contains(duration));
   assert(defined);
   res = value;
}


/*
~IsDefinedAt~

This function checks whether this unit is defined at the 
given duration.

*/
template <class T>
bool LinearConstantMove<T>::IsDefinedAt(
               const datetime::DateTime* duration)const{
  if(!interval.Contains(duration)) 
    return false;
  return defined;
}

/*
~ToListExpr~

This function returns the external representation of this unit
as nested list.

*/
template <class T>
ListExpr LinearConstantMove<T>::ToListExpr()const{
  if(defined){
      return ::nl->TwoElemList(
                    ::nl->SymbolAtom("linear"),
                    ::nl->TwoElemList(
                          interval.ToListExpr(false),
                          ToConstantListExpr(value)));
   } else {
      return ::nl->TwoElemList(
                     ::nl->SymbolAtom("linear"),
                     ::nl->TwoElemList(
                             interval.ToListExpr(false),
                             ::nl->SymbolAtom("undefined")));
   }
}


/*
~ReadFrom~

This function reads this unit from its external representation.
If the list is not a valid representation for this unit, the 
value of this unit remains unchanged and the result will be __false__.

*/
template <class T>
bool LinearConstantMove<T>::ReadFrom(const ListExpr value){
 if(::nl->ListLength(value)!=2){
     if(DEBUG_MODE){
        std::cerr << __POS__ << ": Wrong ListLength" << std::endl;
        std::cerr << "expected : 2, received :" 
             << ::nl->ListLength(value) << std::endl;
     }
     return false;
  }
  if(!interval.ReadFrom(::nl->First(value),false)){
      if(DEBUG_MODE){
         std::cerr << __POS__ << ": Can't read the interval" << std::endl;
      }
      defined = false;
      return false;
  }

  ListExpr V = ::nl->Second(value);
  if(::nl->IsEqual(V,"undefined")){
      defined = false;
      return true;
  }
  T tmp;
  if(!ReadFromListExpr(V,tmp))
     return false;
  this->value = tmp;
  return true;
}

/*
~CanBeSummarized~

This function checks whether this unit and __LCM__ can be put together
to a single unit. This is the case iff the intervals are adjacent
and the values are the same.

*/
template <class T>
bool LinearConstantMove<T>::CanSummarized(
           const LinearConstantMove<T>* LCM) const{
    if(!interval.CanAppended(&LCM->interval)) // no consecutive intervals
      return false;
    if(!defined && ! LCM->defined) // both are not defined
      return true;
    if(!defined || !LCM->defined) // only one is not defined
      return false;
    return value==LCM->value;

}


/*
~Initial~

Returns the value at the begin of this unit. Because the value is never changed,
just the value is returned. This function is implemented for supporting the 
map properties for constant values. 

*/
template <class T> 
bool LinearConstantMove<T>::Initial(T& res)const{
  res = (value);
  return defined;
}


/*
~Final~

The Final function returns the value of this unit at the end of this interval.
Because the value is never changed, the value is returned. This is support 
function for the map class.

*/
template <class T>
bool LinearConstantMove<T>::Final(T& res)const {
   res =(value);
   return defined;
}


/*
~SetDefined~

This function changes the defined state of this unit.

*/
template <class T>
void LinearConstantMove<T>::SetDefined(bool defined){
  this->defined = defined;
}


/*
~GetDefined~

This function returns the defined state of this unit.

*/
template <class T>
bool LinearConstantMove<T>::GetDefined(){
  return defined;
}


/*
~Split~

This function splits this unit into two parts at the given point in time.
If the splitting instant is outside the definition time of this unit,
this unit or right will be set to be undefined. This function returns
always true because an simple unit can be splitted in each case.  

*/
template <class T>
bool LinearConstantMove<T>::Split(const datetime::DateTime duration,
                                  const bool toLeft, 
                                  LinearConstantMove<T>& right){
 right.Equalize(*this);
 interval.Split(duration,toLeft,right.interval);
 this->defined = interval.IsDefined();
 right.defined = right.interval.IsDefined();
 return true;
}

/*

~Equalize~

By calling this function, the value of this LinearConstantMOve is 
taken from the argument of this function.

*/
template <class T>
void LinearConstantMove<T>::Equalize(const LinearConstantMove<T>& source){
  this->defined=source.defined;
  this->interval.Equalize(&(source.interval));
  this->value = source.value;
} 

template <class T>
void LinearConstantMove<T>::Equalize(const LinearConstantMove<T>* source){
   Equalize(*source);
}


} // end of namespace periodic

#endif
