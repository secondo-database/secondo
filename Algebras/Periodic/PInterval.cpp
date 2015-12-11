
/*
3.3 ~PInterval~

*/

#include <stdio.h>
#include <string>

#include "NestedList.h"
#include "PeriodicSupport.h"
#include "PeriodicTypes.h"
#include "StandardTypes.h"
#include "DateTime.h"



using namespace datetime;
extern NestedList* nl;


namespace periodic {
/*
~Constructor~

The familiar empty constructor.

[3] O(1)

*/
PInterval::PInterval(){}

/*
~Constructor~

[3] O(1)

*/
PInterval::PInterval(int dummy):
  Attribute(true), startTime(datetime::instanttype), relinterval(dummy){
}

/*
~Constructor~

[3] O(1)

*/
PInterval::PInterval(const DateTime _startTime, const RelInterval _relinterval):
 Attribute(true), startTime(_startTime), relinterval(_relinterval){ }

/*
~Append~

If possible, the argument make this interval longer. D2 has to be
of type duration.

[3] O(1)

*/
bool PInterval::Append(const RelInterval* D2){
   __TRACE__
  return relinterval.Append(D2);
}

/*
~CanAppended~

This function checks whether D2 can be used to make longer
this interval.

[3] O(1)

*/
bool PInterval::CanAppended(const RelInterval* D2)const{
   __TRACE__
   return relinterval.CanAppended(D2);
}

/*
~Contains~

This function checks wether the instant T is is contained in this
interval.

[3] O(1)

*/
bool PInterval::Contains(const DateTime* T)const{
   __TRACE__
 DateTime tmp = (*T) - startTime;
  return relinterval.Contains(&tmp);
}

/*
~Contains~

This function checks wether the interval I is is contained in this
interval.

[3] O(1)

*/
bool PInterval::Contains(const PInterval* I)const{
   __TRACE__
  if(I->startTime<startTime)
      return false;
   DateTime* thisEnd = GetEnd();
   DateTime* IEnd = I->GetEnd();
   bool res = true;
   if(IEnd > thisEnd){
      res = false;
   }
   else {
      if(I->startTime == startTime){
         if(!IsLeftClosed() && I->IsLeftClosed())
            res = false;
      }
      if(thisEnd == IEnd){
         if(!IsRightClosed() && I->IsRightClosed())
            res = false;
      }
   }
   delete thisEnd;
   thisEnd = NULL;
   delete IEnd;
   IEnd = NULL;
   return res;
}

/*
~Intersects~

This functions checks whether this and I share a common
instant.

[3] O(1)

*/
bool PInterval::Intersects(const PInterval* I)const{
   __TRACE__
  // two intervals intersects if one of the
   // four endpoints is contained in the
   bool res = true;
   DateTime* thisEnd = GetEnd();
   DateTime* IEnd = I->GetEnd();
   if((*IEnd) < startTime){
      res = false;
   } else if(I->startTime> (*thisEnd)){
      res = false;
   } else if((*(IEnd)) == startTime){
       res = (I->IsRightClosed()) && IsLeftClosed();
   } else if(I->startTime == (*(thisEnd))){
       res = (I->IsLeftClosed()) && IsRightClosed();
   }
   delete thisEnd;
   delete IEnd;
   thisEnd = NULL;
   IEnd = NULL;
   return res;
}

/*
~Clone~

Returns a copy of this.

[3] O(1)

*/
PInterval* PInterval::Clone() const{
   __TRACE__
  PInterval* clone = new PInterval(1);
   clone->Equalize(this);
   return clone;
}



/*
~CompareTo~

This functions compares this Interval with D2.

[3] O(1)

*/
int PInterval::CompareTo(const PInterval* D2)const{
   __TRACE__
  int cmp;
   cmp = startTime.CompareTo(&(D2->startTime));
   if(cmp!=0)
     return cmp;
   return relinterval.CompareTo(&(D2->relinterval));
}
/*
~Compare~

The ~Compare~ function compares this Interval with an Attribute.
The argument has to be of type Interval.

[3] O(1)

*/
int PInterval::Compare(const Attribute* arg)const{
   __TRACE__
  return CompareTo( (PInterval*) arg);
}


/*
~HashValue~

Computes a HashValue for this.

[3] O(1)

*/
size_t PInterval::HashValue() const{
   __TRACE__
  return startTime.HashValue()+relinterval.HashValue();
}

/*
~Sizeof~

[3] O(1)

*/
size_t PInterval::Sizeof() const{
   __TRACE__
  return sizeof(*this);
}

/*
~CopyFrom~

When this function is called, this Interval takes its value
from the argument.

[3] O(1)

*/
void PInterval::CopyFrom(const Attribute* arg){
   __TRACE__
  Equalize( (PInterval*) arg);
}

/*
~Equalize~

When this function is called, this Interval takes its value
from the argument.

[3] O(1)

*/
void PInterval::Equalize(const PInterval* D2){
   __TRACE__
   Attribute::operator=(*D2);
   startTime.Equalize(&(D2->startTime));
    relinterval.Equalize(&(D2->relinterval));
}

/*
~GetLength~

Returns the length of this interval as a duration.

[3] O(1)

*/
DateTime* PInterval::GetLength()const{
   __TRACE__
  return relinterval.GetLength();
}

void PInterval::Length(DateTime& res)const{
   relinterval.GetLength(res);
}

/*
~GetStart~

Returns a clone of the StartTime.

[3] O(1)

*/
DateTime* PInterval::GetStart()const{
   __TRACE__
  DateTime* res = new DateTime(instanttype);
   res->Equalize(&startTime);
   return res;
}

void PInterval::GetStart(DateTime& result)const{
   __TRACE__
   result.Equalize(&startTime);
}


/*
~GetEnd~

Returns the end time of this interval.

[3] O(1)

*/
DateTime* PInterval::GetEnd()const{
   __TRACE__
  DateTime* L = relinterval.GetLength();
   DateTime T = startTime + (*L);
   delete L;
   L = NULL;
   DateTime* res = new DateTime(instanttype);
   res->Equalize(&T);
   return res;
}

void 
PInterval::GetEnd(DateTime& result)const{
   __TRACE__
   DateTime len(durationtype);
   relinterval.GetLength(len);
   DateTime T = startTime + len;
   result.Equalize(&T);
}
/*
~ToListExpr~

Returns the list representing this interval.
This requires, that this interval is bounded.

[3] O(1)

*/
ListExpr PInterval::ToListExpr(const bool typeincluded)const {
   __TRACE__
  DateTime* EndTime = GetEnd();
   ListExpr result = ::nl->FourElemList(
                            startTime.ToListExpr(false),
                            EndTime->ToListExpr(false),
                            ::nl->BoolAtom(IsLeftClosed()),
                            ::nl->BoolAtom(IsRightClosed()));
   delete EndTime;
   EndTime = NULL;
   return result;
}

ListExpr PInterval::ToListExpr(const ListExpr typeInfo)const {
   return ToListExpr(false);
}

/*
~IsLeftClosed~

[3] O(1)

*/
bool PInterval::IsLeftClosed()const {
   __TRACE__
  return relinterval.IsLeftClosed();
}


/*
~IsRightClosed~

[3] O(1)

*/
bool PInterval::IsRightClosed()const{
   __TRACE__
  return relinterval.IsRightClosed();
}

/*
~ReadFrom~

Reads the value of this interval from LE. typeincluded specified if
LE is in format (type value)  or LE only represents the value list.

[3] O(1)

*/
bool PInterval::ReadFrom(const ListExpr LE, const bool typeincluded){
   __TRACE__
  ListExpr value;
   cout<< "typeincluded : " << typeincluded << endl;
   if(typeincluded){
      if(::nl->ListLength(LE)!=2)
         return false;
      if(::nl->IsEqual(::nl->First(LE),"pinterval"))
         return false;
      value = ::nl->Second(LE);
   } else {
       value = LE;
   }
   if(::nl->ListLength(value)!=4){
      cout << "Invalid ListLength" << endl << endl;
      return false;
   }
   bool lc,rc;
   if( ::nl->AtomType(::nl->Third(value))!=BoolType ||
       ::nl->AtomType(::nl->Fourth(value))!=BoolType){
       cout << "no bool values" << endl;
       return false;
   }
   DateTime start(instanttype);
   DateTime end(instanttype);
   if(!start.ReadFrom(::nl->First(value),false)){
      return false;
   }
   if(!end.ReadFrom(::nl->Second(value),false)){
      return false;
   }
   lc = ::nl->BoolValue(::nl->Third(value));
   rc = ::nl->BoolValue(::nl->Fourth(value));
   if( (start==end) && !( rc && lc)) { // a single instant has to be closed
      return false;
   }
   if( start > end) { // start has to be before end
      return false;
   }
   DateTime duration = end-start;
   Set(&start,&duration,lc,rc);
   return true;
}

bool PInterval::ReadFrom(const ListExpr LE, const ListExpr typeInfo){
   return ReadFrom(LE,false);
}

/*
~Set~

Sets this interval to  the given values.

[3] O(1)

*/
bool PInterval::Set(const DateTime* startTime, const DateTime* length,
                    const bool leftClosed, const bool rightClosed){
   __TRACE__
   if (!(this->relinterval).Set(length,leftClosed,rightClosed))
       return false;
    (this->startTime).Equalize(startTime);
    return true;
}



/*
~SetLength~

Sets a new length for this interval.

[3] O(1)

*/
bool PInterval::SetLength(const DateTime* T){
   __TRACE__
   return relinterval.SetLength(T);
}

/*
~ToString~

This function computes a string representation of this interval.

[3] o(1)

*/
std::string PInterval::ToString()const {
   __TRACE__
  std::stringstream ss;
   if(IsLeftClosed())
      ss << "[";
   else
      ss << "]";
   ss << startTime.ToString();
   ss << ",";
   DateTime* end = GetEnd();
   ss << end->ToString();
   delete end;
   end = NULL;
   if(IsRightClosed())
      ss << "]";
   else
      ss << "[";
  return ss.str();
}

/*
~Copy Constructor~

*/
PInterval::PInterval(const PInterval& source): 
   Attribute(source.IsDefined()), startTime(source.startTime), 
   relinterval(source.relinterval){
   Equalize(&source);
}

/*
~Destructor~

*/
PInterval::~PInterval(){}

/*
~Assignment Operator~

*/
PInterval& PInterval::operator=(const PInterval& source){
   Equalize(&source);
   return *this;
}

/*
Functions providing attribute functionality

*/
void PInterval::Destroy(){
     relinterval.Destroy();
}
int PInterval::NumOfFLOBs()const{
    return 0;
}
Flob* PInterval::GetFLOB(const int i){ 
     assert(false);
}

/*
~Shift operator~

*/
std::ostream& operator<< (std::ostream& os, const PInterval I){
   __TRACE__
  os << I.ToString();
   return os;
}



} // end of namespace periodic
