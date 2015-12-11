
/*
3.2 ~RelInterval~

*/

#include <iostream>
#include <string>

#include "NestedList.h"
#include "PeriodicTypes.h"
#include "PeriodicSupport.h"
#include "StandardTypes.h"
#include "DateTime.h"


extern NestedList* nl;
using namespace datetime;
using namespace std;


namespace periodic{

/*
~Constructor~

The special constructor for the cast function.

[3] O(1)

*/
RelInterval::RelInterval(){   
   __TRACE__
}

/*
~Constructor~

This constructor creates a defined single instant with length 0.

[3] O(1)

*/
RelInterval::RelInterval(int dummy):
  Attribute(true),
  length(datetime::durationtype),
  state(0)
{
    __TRACE__
   ChangeLeftClosed(true);
   ChangeRightClosed(true);
   ChangeCanDelete(false);
}

/*
~A private constructor~

Creates a RelInterval from the given values. Is private because
the values can leads to a invalid state of this RelInterval.

[3] O(1)

*/
RelInterval::RelInterval(const DateTime* length, const bool leftClosed,
                         const bool rightClosed):
  Attribute(true), length(durationtype), state(0){
    __TRACE__
 DateTime Zero= DateTime(durationtype);
  int comp=length->CompareTo(&Zero);
  assert(comp>=0);
  assert(comp>0 || (leftClosed && rightClosed));
  this->ChangeLeftClosed(leftClosed);
  this->ChangeRightClosed(rightClosed);
  this->length.Equalize(length);
}

/*
~Append~

This function appends D2 to this RelInterval if possible.
When D2 can't appended to this false is returned.

[3] O(1)

*/
 bool RelInterval::Append(const RelInterval* R2){
    __TRACE__
   if(!CanAppended(R2))
       return false;
    length.Add(&(R2->length));
    ChangeRightClosed(R2->IsRightClosed());
    return true;
 }

/*
~CanAppended~

This function checks whether D2 can appended to this RelInterval.

[3] O(1)

*/
bool RelInterval::CanAppended(const RelInterval* D2)const {
    __TRACE__
  if(IsRightClosed())
     return ! D2->IsLeftClosed();
  else
     return D2->IsLeftClosed(); 
}

/*
~Contains~

Checks whether T is contained in this RelInterval 

[3] O(1)

*/
bool RelInterval::Contains(const DateTime* T, bool relax /*=false*/)const {
    __TRACE__
   if(!IsDefined()){
      return false;
   }
   DateTime Zero=DateTime(durationtype);
   int compz = T->CompareTo(&Zero);
   if(compz<0)
     return false;
   if(compz==0)
     return IsLeftClosed() || relax;
   int compe = T->CompareTo(&length);
   if(compe<0)
      return true;
   if(compe==0)
      return IsRightClosed() || relax;
   return false;
}

/*
~Mul~

This functions extends this relinterval to be factor[mul]oldlength.

[3] O(1)

*/
void RelInterval::Mul(const long factor){
   __TRACE__
  length.Mul((int)factor);
}

/*
~Clone~

This function creates a new RelInterval with the same value as
the this object.

[3] O(1)

*/
RelInterval* RelInterval::Clone() const{
    __TRACE__
  RelInterval* Copy = new RelInterval(1);
   Copy->Equalize(this);
   return Copy;
}


/*
~CompareTo~

This function compares two RelIntervals.

[3] O(1)

*/
int RelInterval::CompareTo(const RelInterval* D2)const {
   __TRACE__
  if(!IsDefined() && !D2->IsDefined()){ 
      return 0;
  }
  if(!IsDefined() && D2->IsDefined()){
      return -1;
  }
  if(IsDefined() && !D2->IsDefined()){
       return 1;
  }
  // at this point both involved intervals are defined
  if(IsLeftClosed() && !D2->IsLeftClosed()) return -1;
  if(!IsLeftClosed() && D2->IsLeftClosed()) return 1;
  int tc = length.CompareTo(&(D2->length));
  if(tc!=0) return tc;
  if(!IsRightClosed() && D2->IsRightClosed()) return -1;
  if(IsRightClosed() && !D2->IsRightClosed()) return 1;
  return 0;
}

/*
~Compare~

This functions compares an attribute with this relinterval.

[3] O(1)

*/
int RelInterval::Compare(const Attribute* arg) const{
    __TRACE__
  RelInterval* D2 = (RelInterval*) arg;
   return CompareTo(D2);
}

/*
~CopyFrom~

This function take the value for this relinterval from the
given argument.

[3] O(1)

*/
void RelInterval::CopyFrom(const Attribute* arg){
    __TRACE__
  Equalize((RelInterval*)arg);
}

/*
~HashValue~

[3] O(1)

*/
size_t RelInterval::HashValue() const{
    __TRACE__
  size_t lhv = length.HashValue();
  if(IsLeftClosed()) lhv = lhv +1;
  if(IsRightClosed()) lhv = lhv +1;
  return lhv;
}

/*
~Sizeof~

[3] O(1)

*/
size_t RelInterval::Sizeof() const{
    __TRACE__
  return sizeof(*this);
}
/*
~Split~

Splits an interval at the specified position. __delta__ has to be in [0,1].
The return value indicates whether a rest exists. 

[3] O(1)

*/
bool RelInterval::Split(const double delta, const bool closeFirst,
                        RelInterval& Rest){
    __TRACE__
  if((delta==0) &&  (!IsLeftClosed() || !closeFirst))
     return false;
  if((delta==1) && (!IsRightClosed() || closeFirst))
     return false;

   if(length.IsZero())
     return false;

  Rest.Equalize(this);
  ChangeRightClosed(closeFirst);
  Rest.SetLeftClosed(!closeFirst);
  length.Split(delta,Rest.length);
  return true;
}

/*
~Split~

This function splits an interval at a given point in time. 
If the splitting instant is outside the interval, this interval or
Rest will be undefined. The other one will contain this interval.
If the spitting point is inside the interval, this interval will 
end at this endpoint and Rest will begin at duration. The return 
value is always true.

*/

bool RelInterval::Split(const DateTime duration, const bool closeFirst,
                        RelInterval& Rest){


  // at this point all cases with unbounded intervals are processed

   // duration left of this interval
   if(duration.LessThanZero()){
      // the complete interval is picked up by Rest
      Rest.Equalize(this);
      this->ChangeDefined(false); 
      return true;
   }
   // duration left of this interval
   if(duration.IsZero() && (!closeFirst || ! IsLeftClosed())){
      Rest.Equalize(this);
      this->ChangeDefined(false);
      return true;
   }
   // duration right on this interval
   if(duration>length){
      // Rest will not be defined
      Rest.ChangeDefined(false);
      return true;
   }
   // duration right on this interval
   if(duration==length && (!IsRightClosed() || closeFirst)){
       // Rest will not be defined
       Rest.ChangeDefined(false);
       return true;
   } 
  // the splitting instance is inside this interval
   Rest.length = this->length - duration;
   Rest.ChangeRightClosed(this->IsRightClosed());
   Rest.ChangeLeftClosed(!closeFirst);
   this->length = duration;
   this->ChangeRightClosed(closeFirst);
   return true;
}


/*
~Equalize~

The ~Equalize~ function changes the value of this RelInterval to
the value of D2.

[3] O(1)

*/
void RelInterval::Equalize(const RelInterval* D2){
    __TRACE__
  ::Attribute::operator=(*D2);
  length.Equalize(&(D2->length));
  state = D2->state;
}

/*
~GetLength~

This function returns a clone of the contained time value.
Note that this function
creates a new DateTime instance. The caller of this function has to make
 free the memory occupied by this instance after using it.

[3] O(1)

*/
DateTime* RelInterval::GetLength() const {
    __TRACE__
   DateTime* res = new DateTime(durationtype);
   res->Equalize(&length);
   return res;
}

void RelInterval::GetLength(DateTime& result)const{
   result.Equalize(&length);
}

/*
~StoreLength~

This function stored the length of this interval in the argument of this 
function.  The advantage of this function  in contrast to the GetLength
 function is that no memory is allocated by this function. 

*/
void RelInterval::StoreLength(DateTime& result) const{
  __TRACE__
  result.Equalize(&length);

}



/*
~ToListExpr~

This function computes the list representation of this RelInterval value.

[3] O(1)

*/
ListExpr RelInterval::ToListExpr(const bool typeincluded)const{
  __TRACE__
  ListExpr time;
  time = length.ToListExpr(false);
  if(typeincluded)
       return ::nl->TwoElemList(::nl->SymbolAtom("rinterval"),
                   ::nl->ThreeElemList(
                       ::nl->BoolAtom(IsLeftClosed()),
                       ::nl->BoolAtom(IsRightClosed()),
                       time));
   else
      return ::nl->ThreeElemList(
                   ::nl->BoolAtom(IsLeftClosed()),
                   ::nl->BoolAtom(IsRightClosed()),
                   time);
}

ListExpr RelInterval::ToListExpr(const ListExpr typeInfo)const{
   return ToListExpr(false);
}

/*
~IsLeftClosed~

This function returns true if the left endpoint of this interval is
contained in it.

[3] O(1)

*/
bool RelInterval::IsLeftClosed()const{
    __TRACE__
  return GetLeftClosed();
}

/*
~IsRightClosed~

This function will return true iff the interval is right closed.

[3] O(1)

*/
bool RelInterval::IsRightClosed()const{
    __TRACE__
 return GetRightClosed();
}


/*
~ReadFrom~

This function read the value from the argument. If the argument list don't
contains a valid RelInterval, false will be returned and the value of this
remains unchanged. In the other case the value is taken from this parameter
and true is returned.

[3] O(1)

*/
bool RelInterval::ReadFrom(const ListExpr LE, const bool typeincluded){
   __TRACE__

   ListExpr V;
   if(typeincluded){
      if(::nl->ListLength(LE)!=2){
         if(DEBUG_MODE)
            cerr << __POS__ << ": wrong length for typed interval" << endl;
         return false;
      }
      if(!::nl->IsEqual(::nl->First(LE),"rinterval")){
         if(DEBUG_MODE){
            cerr << __POS__ << ": wrong type for interval" << endl;
            cerr << "expected : rinterval , received :";
            ::nl->WriteListExpr(::nl->First(LE));
         }
         return false;
      }
      V = ::nl->Second(LE);
   } else
     V = LE;
   if(::nl->ListLength(V)!=3){
       if(DEBUG_MODE)
          cerr << __POS__ << ": wrong length for interval" << endl;
       return false;
   }
   if(::nl->AtomType(::nl->First(V))!=BoolType){
     if(DEBUG_MODE)
        cerr << __POS__ << ": wrong type in interval" << endl;
     return false;
   }
   if(::nl->AtomType(::nl->Second(V))!=BoolType){
     if(DEBUG_MODE)
        cerr << __POS__ << ": wrong type in interval" << endl;
     return false;
   }
   DateTime time(durationtype);
   if(!(time.ReadFrom(::nl->Third(V),false))){
         if(DEBUG_MODE)
           cerr << __POS__ << ": error in reading length of interval" << endl;
         return false;
   }
   bool LC = ::nl->BoolValue(::nl->First(V));
   bool RC = ::nl->BoolValue(::nl->Second(V));
   // a single instant has to be both left- and rightclosed
   if( (time.IsZero()) && (!LC || !RC)){
     if(DEBUG_MODE)
        cerr << __POS__ << ": invalid values in interval" << endl;
      return false;
   }
   ChangeLeftClosed(LC);
   ChangeRightClosed(RC);
   length.Equalize(&time);
   
   return true;
}


/*
~Set~

This function sets this RelINterval to be finite with determined length.
If the arguments don't represent a valid RelINterval value, the return value
will be false and this instance remains unchanged. In the other case true
is returned and the value of this is taken from the arguments.

[3] O(1)

*/
bool RelInterval::Set(const DateTime* length, const bool leftClosed,
                      const bool rightClosed){
    __TRACE__
 if((length->IsZero()) && (!leftClosed || !rightClosed)) return false;
 this->ChangeDefined(true);
 this->ChangeLeftClosed(leftClosed);
 this->ChangeRightClosed(rightClosed);
 this->length.Equalize(length);
 return true;
}


/*
~SetLeftClosed~


This function sets the closure of this interval at its left end.

*/
void RelInterval::SetLeftClosed(bool LC){
   this->ChangeLeftClosed(LC);
}  

/*
~SetRightClosed~

This function works like the function ~SetLeftClosed~ but for the
right end of this interval.

*/
void RelInterval::SetRightClosed(bool RC){
   this->ChangeRightClosed(RC);
}  

/*
~SetClosure~

This functions is a summarization of the ~SetLeftClosed~ and the
~SetRightClosed~ function. The effect will be the same like calling
this two functions.

*/
void RelInterval::SetClosure(bool LC,bool RC){
   this->ChangeLeftClosed(LC);
   this->ChangeRightClosed(RC);
}




/*
~SetLength~

This function is used to set the length of this RelINterval.
If the given parameter collides with internal settings, false
is returned.

[3] O(1)

*/
bool RelInterval::SetLength(const DateTime* T){
    __TRACE__
   if(T->IsZero() && (!IsLeftClosed() || !IsRightClosed())) return false;
   length.Equalize(T);
   return true;
}


/*
~ToString~

This function returns a string representation of this RelInterval.

[3] O(1)

*/
string RelInterval::ToString()const{
    __TRACE__
  if(!IsDefined()){
     return "_undefined_";
  }
  ostringstream tmp;
   tmp << (IsLeftClosed()?"[":"]");
   tmp << " 0.0 , ";
   tmp << length.ToString();
   tmp << (IsRightClosed()?"]":"[");
   return tmp.str();
}

/*

~Where~

This function returns at which procentual part of this interval T is located.
The result will be a value in $[0,1]$. In all errors cases the value -1
is returned.

[3] O(1)

*/
double RelInterval::Where(const DateTime* T)const{
    __TRACE__
  if(length.CompareTo(T)<0)
     return -1;
  if(T->LessThanZero()) return -1;
  unsigned long ms1 = 86400000L*T->GetDay()+T->GetAllMilliSeconds();
  unsigned long ms2 = 86400000L*length.GetDay()+length.GetAllMilliSeconds();
  return (double)ms1/(double)ms2;
}


/*
~Plus~

This function  adds the argument to this interval. 
The 'weld point' is included to the new interval regardless
to the closure properties of the source intervals. The closure on the
left of this interval will not be changed and the closure on the 
right is taken from the argument.

*/
bool RelInterval::Plus(const RelInterval* I){
  ChangeRightClosed(I->IsRightClosed());
  length.Add(&(I->length));
  return true;
}


/*
~Copy Constructor~

*/
RelInterval::RelInterval(const RelInterval& source): 
       Attribute(source.IsDefined()), 
       length(source.length), 
       state(source.state) {
   Equalize(&source);
}

/*
~Destructor~

Because an relative interval has only primitive members,
the destructor makes nothing.

*/
RelInterval::~RelInterval(){}

/*
~Assignment operator~

*/
RelInterval & RelInterval::operator=(const RelInterval& source){
   Equalize(&source);
   return *this;
}

/*
~Attribute Supporting functions~

The next functions support some operations for using this type 
as an attribute within a relation.

*/
void RelInterval::Destroy(){
   ChangeCanDelete(true);
}

int RelInterval::NumOfFLOBs() const{
    return 0;
}

Flob* RelInterval::GetFLOB(const int i){
    assert(false);
}


bool RelInterval::Adjacent(const Attribute*)const{
   return false;
}
/* 
~Changing flags ~

*/

void RelInterval::ChangeLeftClosed(const bool value){
   if(value){
      state = (state | 1);
   }else{
      state = (state & ~1);
   }
}
void RelInterval::ChangeRightClosed(const bool value){
   if(value){
      state = state | 2;
   }else{
      state = state & ~2;
   }
}
void RelInterval::ChangeDefined(const bool value){
   if(value){
      state = state | 4;
   }else{
      state = state & ~4;
   }

}
void RelInterval::ChangeCanDelete(const bool value){
   if(value){
      state = state | 8;
   }else{
      state = state & ~8;
   }
}


bool RelInterval::GetLeftClosed() const{
   return (state & 1 )>0;
};
bool RelInterval::GetRightClosed() const{
    return (state & 2) >0;
}
bool RelInterval::GetDefined() const{
    return (state & 4) > 0;
}
bool RelInterval::GetCanDelete() const{
    return (state & 8) > 0;
}

ostream& operator<< (ostream& os, const RelInterval I){
   __TRACE__
  os << I.ToString();
  return os;
}

} // end of namespace periodic
