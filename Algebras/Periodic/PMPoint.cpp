/*
1.1 ~PMPoint~

~includes~

*/
#include <iostream>
#include <string>
#include <iostream>
#include <sstream>
#include <math.h>
#include <vector>

#include "NestedList.h"
#include "PeriodicTypes.h"
#include "PeriodicSupport.h"
#include "SpatialAlgebra.h"
#include "TemporalAlgebra.h"
#include "TopRel.h"
#include "StandardTypes.h"
#include "DateTime.h"
#include "List2.h" 
#include "RepTree.h"
#include "NodeTypes.h"
#include "TopRel.h"
#include "../../Tools/Flob/DbArray.h"


using namespace std;
using namespace datetime;


namespace periodic {

/*
~Constructor~

This constructor does nothing

*/

PMPoint::PMPoint(){}

/*
~Constructor~

This constructor creates an undefined periodic moving point.
The argument of this constructor is ignored. The reason for the
parameter is to make this constructor distinct from the standard
constructor which must be nothing.

[3] O(1)

*/
PMPoint::PMPoint(int dummy):
   Attribute(false),
   linearMoves(0),
   compositeMoves(0),
   compositeSubMoves(0),
   periodicMoves(0),
   canDelete(false),
   interval(0),
   startTime(instanttype),
   bbox(0)
{   __TRACE__
}
/*
~Copy Constructor~

*/
PMPoint::PMPoint(const PMPoint& source):
 Attribute(source.IsDefined()),
 linearMoves(0), compositeMoves(0), compositeSubMoves(0),
 periodicMoves(0), canDelete(false), interval(0), 
 startTime(instanttype), bbox(0){
   Equalize(&source);
}

PMPoint& PMPoint::operator=(const PMPoint& source){
   Equalize(&source);
   return *this;
}

/*
~Destructor~

[3] O(1)

*/
PMPoint::~PMPoint(){
    __TRACE__
  if(canDelete){
      if(linearMoves.Size()>0) linearMoves.Destroy();
      if(compositeMoves.Size()>0) compositeMoves.Destroy();
      if(compositeSubMoves.Size()>0) compositeSubMoves.Destroy();
      if(periodicMoves.Size()>0) periodicMoves.Destroy();
   }
}

/*
~Destroy~

[3] O(1)

*/
void PMPoint::Destroy(){
    __TRACE__
   canDelete=true;
}


/*
~Equalize~

This functions changes the value of this periodic moving point
to be equals to the given parameter.

[3] O(L), where L is the number of linear moves

*/
void PMPoint::Equalize(const PMPoint* P2){
    __TRACE__
  // equalize the linear moves
  Attribute::operator=(*P2);
  linearMoves.copyFrom(P2->linearMoves);
  compositeMoves.copyFrom(P2->compositeMoves);
  compositeSubMoves.copyFrom(P2->compositeSubMoves);
  periodicMoves.copyFrom(P2->periodicMoves);
  SetDefined(P2->IsDefined());
  interval.Equalize(&(P2->interval));
  startTime.Equalize(&(P2->startTime));
  bbox.Equalize(&(P2->bbox));
  submove.Equalize(&(P2->submove));

}

/*
~Statistical Information~

*/
size_t PMPoint::NumberOfNodes()const{
   return linearMoves.Size() +
          compositeMoves.Size() +
          periodicMoves.Size() +
          1;
}

size_t PMPoint::NumberOfCompositeNodes()const{
   return compositeMoves.Size();
}
size_t PMPoint::NumberOfPeriodicNodes()const{
   return periodicMoves.Size();
}
size_t PMPoint::NumberOfUnits()const{
   return linearMoves.Size();
}

size_t PMPoint::NumberOfFlatUnits()const{
    if(!IsDefined()){
         return 0;
    }
    size_t res = NumberOfFlatNodes(submove);
    return res;
}

/*
~NumberOfFlatNodes~

This function computes the number of units of the flat representation
for the given subtree. The periodic moving point itself has to be defined.

Complexity: O(n) where n is the number of nodes within this tree

*/

size_t PMPoint::NumberOfFlatNodes(const SubMove sm)const{
   int type = sm.arrayNumber;
   switch(type){
      case LINEAR:{
             LinearPointMove LM;
             linearMoves.Get(sm.arrayIndex,LM);
             if(LM.IsDefined()){
               return 1;
             }else{
               return 0;
             }
       }
      case PERIOD:{
             SpatialPeriodicMove PM(0);
             periodicMoves.Get(sm.arrayIndex,PM);
             size_t subsize;
             subsize = NumberOfFlatNodes(PM.submove);
             return (PM.repeatations)*subsize;
       }
      case COMPOSITE: {
             SpatialCompositeMove CM(0);
             size_t res;
             res = 0;
             CSubMove csm;
             compositeMoves.Get(sm.arrayIndex,CM);
             for(int i=CM.minIndex; i<=CM.maxIndex;i++){
                 compositeSubMoves.Get(i,csm);
                 res += NumberOfFlatNodes(csm);
             }
             return res;
        }
       default: cerr << "invalid submove found " << endl <<__POS__ <<endl;
                assert(false);
               
   }
}

/*
~IsEmpty~

Checks whether this point contains any positions.

*/
bool PMPoint::IsEmpty()const{
  return linearMoves.Size()==0;
}


/*
~NumOfFLOBs~

This function returns the number of contained FLOBs in this
class. Because four DBarrays are managed her, the return value
is 4.

[3] O(1)

*/
int PMPoint::NumOfFLOBs() const{
    __TRACE__
   return 4;
}

/*
~GetFLOB~

This function returns the FLOB with index i.

[3] O(1)

*/
Flob* PMPoint::GetFLOB(const int i){
    __TRACE__
   assert(i>=0 && i<NumOfFLOBs());
   Flob* res=0;
   switch(i){
      case 0 : res = &linearMoves; break;
      case 1 : res = &compositeMoves;break;
      case 2 : res = &compositeSubMoves;break;
      case 3 : res = &periodicMoves;break;
   }

   return res; 
}

/*
~Compare~

This function is not implemented at this moment.

[3] O(-1)

*/
int PMPoint::Compare(const Attribute*)const{
    __TRACE__
  cout << " Warning! PMPoint::Compare  not implemented" << endl;
   return 0;
}

/*
~Adjacent~

We can't define a adjacent relation beween two periodic moving
points. For this reason the return value is allways __false__.

[3] O(1)

*/
bool PMPoint::Adjacent(const Attribute* ) const{
    __TRACE__
  return false;
}

/*
~Clone~

The ~Clone~ function returns a copy of this.

[3] O(L)

*/
PMPoint* PMPoint::Clone() const{
    __TRACE__
  PMPoint* copy = new PMPoint(0);
  copy->Equalize(this);
  return copy;
}


/*
~Sizeof~

This function returns the size of the PMPoint-class

[3] O(1)

*/
size_t PMPoint::Sizeof() const{
    __TRACE__
  return sizeof(PMPoint);
}


/*
~HashValue~

Returns the HashValue for this Point.

[3] O(1)

*/
size_t PMPoint::HashValue()const{
    __TRACE__
 DateTime* L = interval.GetLength();
  size_t res = (size_t) (bbox.Size()+(int)L->GetDay());
  delete L;
  L = NULL;
  return res;
}


/*
~CopyFrom~

The PMPoint instance takes its value from the given argument.
The caller has to ensure that __arg__ is of type PMPoint.

[3] O(L)

*/
void PMPoint::CopyFrom(const Attribute* arg){
    __TRACE__
  Equalize(static_cast<const PMPoint*>(arg));
}


/*
~ToListExpr~

This function returns the ListExpr representing this point.
The flag which is the argument constrols whether only the
value list is returned or whether a list with structure
(type value) will be returned.


[3] O(L)

*/
ListExpr PMPoint::ToListExpr(const bool typeincluded)const{
    __TRACE__
  ListExpr timelist = startTime.ToListExpr(false);
  ListExpr SubMoveList;
  if(IsDefined())
     SubMoveList = GetSubMoveList(&submove);
  else
     SubMoveList = ::nl->SymbolAtom("undefined");

   if(typeincluded)
      return ::nl->TwoElemList(
                  ::nl->SymbolAtom("pmpoint"),
                  ::nl->TwoElemList(
                      timelist,
                      SubMoveList));
   else
      return ::nl->TwoElemList(timelist,SubMoveList);
}

ListExpr PMPoint::ToListExpr(const ListExpr typeInfo)const{
   return ToListExpr(false);
}


/*
~GetSubMove~

This functions determines the move from the given argument and
returns its nested list representation.

[3] O($L_{SM}$) , where $L_{SM}$ is the number of linear moves contained in SM

*/
ListExpr PMPoint::GetSubMoveList(const SubMove* SM)const{
    __TRACE__
 ListExpr SubMoveList;
  int SubMoveType = SM->arrayNumber;
  int index = SM->arrayIndex;
  if(SubMoveType==LINEAR)
      SubMoveList = GetLinearMoveList(index);
  else if(SubMoveType==COMPOSITE)
      SubMoveList = GetSpatialCompositeMoveList(index);
  else if(SubMoveType==PERIOD)
      SubMoveList = GetSpatialPeriodicMoveList(index);
  else{
       cerr << "unknown submove type detected" << SubMoveType << endl;
       cerr << __POS__ << " Error in creating ListExpr" << endl;
       assert(false);
   }
  return SubMoveList;
}

/*
~GetLinearMove~

This functions returns the nested list representation of the
linear move at the specified index.

[3] O(1)

*/
ListExpr PMPoint::GetLinearMoveList(const int index)const{
    __TRACE__
   LinearPointMove LM;
   linearMoves.Get(index,LM);
   return LM.ToListExpr();
}

/*
~GetSpatialPeriodicMove~

This function converts the periodic move at the specified index
to its nested list representation.

[3] O($L_{P}$), where $L_{P}$ is the number of linear moves in the 
periodic move at index

*/
ListExpr PMPoint::GetSpatialPeriodicMoveList(const int index)const{
    __TRACE__
  SpatialPeriodicMove PM(0);
  periodicMoves.Get(index,PM);
  ListExpr periodtype = ::nl->SymbolAtom("period");
  ListExpr RepList = ::nl->IntAtom(PM.repeatations);
  ListExpr SML = GetSubMoveList(&(PM.submove));
  ListExpr LC = ::nl->BoolAtom(PM.interval.IsLeftClosed());
  ListExpr RC = ::nl->BoolAtom(PM.interval.IsRightClosed());
  return  ::nl->TwoElemList(periodtype,::nl->FourElemList(RepList,LC,RC,SML));
}

/*
~GetSpatialCompositeMoveList~

This function returns the nested list representation of the composite
move at the specified array index.

[3] O(L) , where L is the number of submoves contained in the linear 
move at index

*/
ListExpr PMPoint::GetSpatialCompositeMoveList(const int index)const{
    __TRACE__
 SpatialCompositeMove CM(0);
 compositeMoves.Get(index,CM);
 ListExpr CType = ::nl->SymbolAtom("composite");
 int minIndex = CM.minIndex;
 int maxIndex = CM.maxIndex;
 ListExpr SubMovesList;
 if(maxIndex<minIndex){
    cerr << __POS__ << "empty composite move" << endl;
    SubMovesList = ::nl->TheEmptyList();
 }
 else{
   // construct the List of submoves
   CSubMove SM;
   compositeSubMoves.Get(minIndex,SM);
   SubMovesList = ::nl->OneElemList(GetSubMoveList(&SM));
   ListExpr Last = SubMovesList;
   for(int i=minIndex+1;i<=maxIndex;i++){
     compositeSubMoves.Get(i,SM);
     Last = ::nl->Append(Last,GetSubMoveList(&SM));
   }
 }
 return ::nl->TwoElemList(CType,SubMovesList);
}


/*
~ReadFrom~

This function reads the value of this p.m. point from the
given nested list. If the nested list don't contains a
valid point, the return value will be false and this point
is set to be undefined. Otherwise the point has the value
described in the nested list. The list consist only of the
the value, this means the type description is not included
in this list.

[3] O(L)  where L = number of linear moves

*/
bool PMPoint::ReadFrom(const ListExpr value, const ListExpr typeInfo){
    __TRACE__
 /* The list is scanned twice. In the first scan we
     compute only the needed size for each of the  contained arrays. 
     This is done to avoid a frequently resize of the arrays which 
     would lead to a lot of overhead for copying the contents.
  */

  if(::nl->ListLength(value)!=2){
     if(DEBUG_MODE){
        cerr << __POS__ << ": wrong listlength (";
        cerr << (::nl->ListLength(value)) << ")" << endl;
     }
     SetDefined(false);
     return false;
  }

  if(!ResizeArrays(value)){
     if(DEBUG_MODE){
        cerr << __POS__ << ": resize array failed" << endl;
     }
     SetDefined(false);
     return false;
  }

  if(!startTime.ReadFrom(::nl->First(value),false)){
     if(DEBUG_MODE){
        cerr << __POS__ << "reading of the start time failed" << endl;
        cerr << "The list is " << endl;
        ::nl->WriteListExpr(::nl->First(value));
     }
     SetDefined(false);
     return false;
  }
  // now we have to append the included submove
  ListExpr SML = ::nl->Second(value);
  if(::nl->ListLength(SML)!=2){
     if(DEBUG_MODE){
        cerr << __POS__ << ": wrong list length for submove" << endl;
     }
     SetDefined(false);
     return false;
  }

  // get the submove type
  ListExpr SMT = ::nl->First(SML);
  int LMIndex = 0;
  int CMIndex = 0;
  int SMIndex = 0;
  int PMIndex = 0;
  if(::nl->IsEqual(SMT,"linear")){
     submove.arrayNumber = LINEAR;
     submove.arrayIndex = 0;
     if(!AddLinearMove(::nl->Second(SML),LMIndex,CMIndex,SMIndex,PMIndex)){
         if(DEBUG_MODE){
            cerr << __POS__ << " Error in reading linear move" << endl;
         }
         SetDefined(false);
         return false;
     }
     SetDefined(true);
     // read out the interval and the bounding box from the 
     // created linear move
     LinearPointMove LM;
     linearMoves.Get(0,LM);
     interval.Equalize(&(LM.interval));
     bbox.Equalize((LM.BoundingBox()));
     CorrectDurationSums();
     return true;
  }

  if(::nl->IsEqual(SMT,"composite")){
     submove.arrayNumber=COMPOSITE;
     submove.arrayIndex = 0;
     if(!AddSpatialCompositeMove(::nl->Second(SML),LMIndex,
        CMIndex,SMIndex,PMIndex)){
        if(DEBUG_MODE){
           cerr << __POS__ << "error in reading composite move" << endl;
        }
        SetDefined(false);
        return false;
     }
     SetDefined(true);
     // get interval and bounding box from this move
     SpatialCompositeMove CM(0);
     compositeMoves.Get(0,CM);
     interval.Equalize(&(CM.interval));
     bbox.Equalize(&(CM.bbox));
     CorrectDurationSums();
     return true;
  }
  if(::nl->IsEqual(SMT,"period")){
     submove.arrayNumber = PERIOD;
     submove.arrayIndex = 0;
     if(!AddPeriodMove(::nl->Second(SML),LMIndex,CMIndex,SMIndex,PMIndex)){
        if(DEBUG_MODE){
          cerr << __POS__ << " error in reading periodic move" << endl;
        }
        SetDefined(false);
        return false;
     }
     SetDefined(true);
     // get interval as well as bounding box
     SpatialPeriodicMove PM(0);
     periodicMoves.Get(0,PM);
     interval.Equalize(&(PM.interval));
     bbox.Equalize(&(PM.bbox));
     CorrectDurationSums();
     return true;
  }
  if(DEBUG_MODE){
     cerr << __POS__ << "unknown subtype" << endl;
     ::nl->WriteListExpr(SMT);
  }
  return false;
}


PMPoint* PMPoint::Intersection(const PInterval* interval){
    __TRACE__
  PMPoint* result = new PMPoint(1);
   PInterval MyInterval=PInterval(this->startTime,this->interval);
   if(!(MyInterval.Intersects(interval))){
      return result;  // return an empty pmpoint
   }
   if(interval->Contains(&MyInterval)){
        result->Equalize(this);
        return result; // return a copy of this
   }
   int Lcount,Ccount,Scount,Pcount;  // sizes for the arrays
   Lcount = Ccount = Scount = Pcount = 0; // no move
   DateTime* tmpTime = startTime.Clone();
   AddSubMovesSizeForIntersection(tmpTime,submove,interval,
                                  Lcount,Ccount,Scount,Pcount);
   if(Lcount>0) result->linearMoves.resize(Lcount);
   if(Ccount>0) result->compositeMoves.resize(Ccount);
   if(Scount>0) result->compositeSubMoves.resize(Scount);
   if(Pcount>0) result->compositeSubMoves.resize(Pcount);


   // in a first step we count the array-sizes
   cout << "PMPOINT::Intersection(Interval) not implemented ";
   cout  << __POS__ << endl;
   return 0;
}

/*
~computeSubMovesSizeForIntersection~

This function computes the array sizes needed by the result of the intersection
of the calling periodic moving point instance with the given interval.

*/
void PMPoint::AddSubMovesSizeForIntersection(DateTime* startTime,
               const SubMove submove, const PInterval* interval,
               int &Lcount,int &Ccount, int &Scount,int &Pcount){
    __TRACE__
  int number = submove.arrayNumber;
   if(number==LINEAR){
      LinearPointMove LM;
      linearMoves.Get(submove.arrayIndex,LM);
      PInterval i = PInterval((*startTime) , LM.interval);
      if(i.Intersects(interval))
           Lcount++;
      startTime->Add(LM.interval.GetLength());
   }
   if(number==COMPOSITE){
      SpatialCompositeMove CM(0);
      compositeMoves.Get(submove.arrayIndex,CM);
      int start = CM.minIndex;
      int end = CM.maxIndex;
      int sm=0;
      bool stop = false;
      for(int s=start;s<=end && !stop;s++){
         int oldL = Lcount;
         int oldC = Ccount;
         int oldP = Pcount;
         CSubMove SM;
         compositeSubMoves.Get(s,SM);
         AddSubMovesSizeForIntersection(startTime,SM,interval,Lcount,
                                        Ccount,Scount,Pcount);
         if( (Lcount!=oldL) || (Ccount != oldC) || (Pcount != oldP))
             sm++;
         else
             stop = true;
      }
      if(sm>1){
         Ccount ++;
         Scount += sm;
      }
   }
   if(number==PERIOD){
         SpatialPeriodicMove PM(0);
         periodicMoves.Get(submove.arrayIndex,PM);
         //SubMove PSM = PM.submove;
         cout << __POS__ << "NOT IMPLEMENTED" << endl;
         /* wenn PeriodicMOve-Intervall in geg. Interval enthalten
            erhoehe um alle enthaltenen Submoves
            wenn Intervall vor SubMOve-Intervall beendet,
            mache mit SubMoves weiter
            sonst fuege alle vollstaendig enthaltenen Perioden hinzu
            weiterhin ein Composite Move (welches Periode und Rest verbindet
           bearbeite den Rest */
   }
}


/*
~PrintArrayContents~

This function is for debugging purposes. It prints out the
contents of the arrays contained  within this 
periodic moving point.

*/
void PMPoint::PrintArrayContents(){

int size = periodicMoves.Size();
cout << "PERIDICMOVES" << endl;
SpatialPeriodicMove PM(0);
for(int i=0;i<size;i++){
   periodicMoves.Get(i,PM);
   cout << PM.ToString() << endl;
}

cout << "CompositeMOVES" << endl;
size = compositeMoves.Size();
SpatialCompositeMove CM(0);
for(int i=0;i<size;i++){
   compositeMoves.Get(i,CM);
   cout << CM.ToString() << endl;
}

cout << "CompositeSubMoves " << endl;
size = compositeSubMoves.Size();
CSubMove SM;
for(int i=0;i<size;i++){
   compositeSubMoves.Get(i,SM);
   cout << SM.ToString() << endl;
}



}




/*
~CheckCorrectness~

This function checks whether the representation of this
periodic moving point is correct. This means

  *  no directly nested composite moves exists

  *  no directly nested periodic moves exists

  *  a composite move has at least two submoves

[3] O(n) , where n is the number of moves

*/
bool PMPoint::CheckCorrectness(){
  __TRACE__
  CSubMove SM;
  size_t linearSize = linearMoves.Size();
  size_t periodSize = periodicMoves.Size();
  size_t compositeSize = compositeMoves.Size();
  size_t compositeSubSize = compositeSubMoves.Size();

  for(size_t i=0; i<compositeSubSize;i++){
    compositeSubMoves.Get(i,SM);
    size_t an = SM.arrayNumber;
    size_t index = SM.arrayIndex;
    switch(an){
      case COMPOSITE:
          if(DEBUG_MODE){
             cerr << __POS__ << "nested compositeMove detected" << endl;
           }
           return false;
      case PERIOD:
          if(index>=periodSize){
             if(DEBUG_MODE){
               cerr << __POS__ << "array index " << index 
                    << "out of bounds " << periodSize << endl;
             }
             return false;
          }
          break;
      case LINEAR:
          if(index>=linearSize){
             if(DEBUG_MODE){
               cerr << __POS__ << "array index " << index 
                    << "out of bounds " << linearSize << endl;
             }
             return false;
          }
          break;
     default:
          if(DEBUG_MODE){
              cerr << __POS__ << "unknown submove found " << an << endl;
          }
          return false;     
    } 
  }

  // check for direcly nested periodic moves
  SpatialPeriodicMove PM(0);
  for(int i=0; i<periodicMoves.Size();i++){
      periodicMoves.Get(i,PM);
      int an = PM.submove.arrayNumber;
      size_t index = PM.submove.arrayIndex;
      switch(an){
        case COMPOSITE:
            if(index>=compositeSize){
               if(DEBUG_MODE){
                 cerr << __POS__ << "array index " << index 
                      << "out of bounds " << compositeSize << endl;
               }
               PrintArrayContents();
               return false;
            }
            break;
        case PERIOD:
             cerr << __POS__ << "nested periodic move detected" << endl;
             PrintArrayContents();
             return false;
        case LINEAR:
            if(index>=linearSize){
               if(DEBUG_MODE){
                 cerr << __POS__ << "array index " << index 
                      << "out of bounds " << linearSize << endl;
                 PrintArrayContents();
               }
               return false;
            }
            break;
       default:
            if(DEBUG_MODE){
                cerr << __POS__ << "unknown submove found " << endl;
             }
            return false;     
      }
      if(PM.repeatations<2){
         cerr << __POS__ << "invalid number of repetitions detected" << endl;
         PrintArrayContents();
         return false;
      } 
  }

  // check for composite moves with only one submove
  SpatialCompositeMove CM(0);
  for(int i=0;i<compositeMoves.Size();i++){
     compositeMoves.Get(i,CM);
     if(CM.minIndex>=CM.maxIndex){
        if(DEBUG_MODE){
           cerr << __POS__ 
                << "composite move with a single submove detected" 
                << endl;
        }
        return false;
     }
     if(CM.maxIndex>=(int)compositeSubSize){
        if(DEBUG_MODE){
           cerr << __POS__ << "invalid submove position detcetd" << endl;
        }
        return false;
     }
  }
  return true;
}


/*
~ResizeArrays~

This function resizes the array to the values needed to include
all move types in the list. Note that a call of this function
changes this point  even though the list don't represent a
valid periodic moving point. This function should be used before
a periodic moving point is read from a nested list. The disadvantage 
is the twice scanning of the lsit, but a frequently resize of the 
array can be avoided.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoint::ResizeArrays(const ListExpr value){
    __TRACE__
 // first all entries in the arrays are removed
   linearMoves.clean();
   compositeMoves.clean();
   compositeSubMoves.clean();
   periodicMoves.clean();
   int LMSize = 0;
   int CMSize = 0;
   int SMSize = 0;
   int PMSize = 0;
   if(!AddSubMovesSize(::nl->Second(value),LMSize,CMSize,SMSize,PMSize))
      return false;
   // set the arrays to the needed size
   if(LMSize>0) linearMoves.resize(LMSize);
   if(CMSize>0) compositeMoves.resize(CMSize);
   if(SMSize>0) compositeSubMoves.resize(SMSize);
   if(PMSize>0) periodicMoves.resize(PMSize);
   return true;
}

/*
~AddSubMovesSize~

This function computes the needed sizes for the arrays to hold the
value of the p.m. point represented in the value list.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoint::AddSubMovesSize(const ListExpr value,int &LMSize,int &CMSize,
                              int &SMSize,int &PMSize){
    __TRACE__
   // all moves have the length 2
   if(::nl->ListLength(value)!=2){
       return false;
   }
   ListExpr type = ::nl->First(value);

   // the type has to be one of {linear, composite, period}
   if(::nl->AtomType(type)!=SymbolType){
       return false;
  }
  // in a linear move we have only to increment the size of LM
  if(::nl->IsEqual(type,"linear")){
     LMSize = LMSize +1;
     return true;
  }
  if(::nl->IsEqual(type,"composite")){
     CMSize = CMSize+1; // the composite move itself
     ListExpr rest = ::nl->Second(value);
     while(!::nl->IsEmpty(rest)){
        SMSize++; // a new submove
        if(!AddSubMovesSize(::nl->First(rest),LMSize,CMSize,SMSize,PMSize))
           return false;
        rest = ::nl->Rest(rest);
     }
     return true;
  }
  if(::nl->IsEqual(type,"period")){
     PMSize = PMSize+1;
     ListExpr PMove;
     int len = ::nl->ListLength(value);
     if(len==2){
        PMove = ::nl->Second(value);
     }
     else{ // invalid listlength
        return false;
     }
     return AddSubMovesSize(::nl->Second(PMove),LMSize,CMSize,SMSize,PMSize);
  }
  // a unknown type description
  return false;
}




/*
~AddLinearMove~

This functions appends the linear move given as nested list in __value__
to the LinearMoves -array.

[3] O(1)

*/
bool PMPoint::AddLinearMove(const ListExpr value,int &LMIndex, int &CMIndex,
                            int &SMIndex, int &PMIndex){
    __TRACE__
  LinearPointMove LM(0);
   if(!LM.ReadFrom(value))
      return false;
   linearMoves.Put(LMIndex,LM);
   LMIndex++;
   return true;
}


/*
~AddSpatialCompositeMove~

This Functions appends the composite move given as nested list together with
all contained submoves at the appropriate arrays. The return value indiciates
the success of this call.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoint::AddSpatialCompositeMove(const ListExpr value,int &LMIndex,
                   int &CMIndex, int &SMIndex, int &PMIndex){
    __TRACE__
  // a composite move has to contains at least two submoves
   int len = ::nl->ListLength(value);
   if(len<2){
      if(DEBUG_MODE){
         cerr << __POS__ << " less than 2 submoves (" << len << ")" << endl;
      }
      return false;
   }
   SpatialCompositeMove CM(1);
   int CMPos=CMIndex;
   // at this position we have to include this composite move
   int SMPos=SMIndex;
   // beginning at this position we have to include the submoves
   // ensure that no submove used the positions for this composite move
   CMIndex++;
   CM.bbox.SetUndefined();
   CM.minIndex=SMIndex;
   CM.maxIndex=SMIndex+len-1;
   SMIndex= SMIndex+len;
   // Append the contained Submoves
   ListExpr rest = value;
   ListExpr SML,TL,VL;
   bool isFirst = true;
   while(!::nl->IsEmpty(rest)){
      SML = ::nl->First(rest);
      rest = ::nl->Rest(rest);
      if(::nl->ListLength(SML)!=2){ // all submoves have the format (type value)
         if(DEBUG_MODE){
            cerr << __POS__ << " submove has wrong length (";
            cerr << ::nl->ListLength(SML) << ")" << endl;
         }
         return false;
      }
      TL = ::nl->First(SML);
      VL = ::nl->Second(SML);
      if(::nl->IsEqual(TL,"linear")){
         // process a linear submove
         int LMPos = LMIndex;
         if(!AddLinearMove(VL,LMIndex,CMIndex,SMIndex,PMIndex)){
            if(DEBUG_MODE){
               cerr << __POS__ << " can't add a linear move " << endl;
            }
            return false;
         }
         LinearPointMove LM;
         linearMoves.Get(LMPos,LM);
         // Append the interval of LM to CM
         if(isFirst){
            isFirst=false;
            CM.interval.Equalize(&(LM.interval));
         }else{
            if(!CM.interval.Append(&(LM.interval))){
               if(DEBUG_MODE){
                   cerr << __POS__ << " can't append interval ";
                   cerr << endl;
                   cerr << "The original interval is";
                   cerr << CM.interval.ToString() << endl;
                   cerr << "The interval to append is";
                   cerr << LM.interval.ToString() << endl;
               }
               return false;
            }
         }
         // build the union of the bounding boxes of CM and LM
         PBBox tmpbox(LM.BoundingBox());
         CM.bbox.Union(&tmpbox);
         // put the submove in the array
         CSubMove SM;
         SM.arrayNumber = LINEAR;
         SM.arrayIndex = LMPos;
         compositeSubMoves.Put(SMPos,SM);
         SMPos++;
      } else if(::nl->IsEqual(TL,"period")){
        // process a periodic submove
        int PMPos = PMIndex;
        if(!AddPeriodMove(VL,LMIndex,CMIndex,SMIndex,PMIndex)){
           if(DEBUG_MODE){
              cerr << __POS__ << "can't add period move " << endl;
            }
            return  false;
        }
        SpatialPeriodicMove PM(0);
        periodicMoves.Get(PMPos,PM);
        if(isFirst){
           isFirst=false;
           CM.interval.Equalize(&(PM.interval));
        }else{
           if(!CM.interval.Append(&(PM.interval))){
              if(DEBUG_MODE){
                 cerr << __POS__  << " can't append interval" << endl;
              }
              return false;
           }
        }
        CM.bbox.Union(&(PM.bbox));
        CSubMove SM;
        SM.arrayNumber = PERIOD;
        SM.arrayIndex = PMPos;
        compositeSubMoves.Put(SMPos,SM);
        SMPos++;
   } else{ // not of type linear or period
      if(DEBUG_MODE){
          cerr << __POS__ 
               << " submove not of type linear or period" 
               << endl;
       }
       return false;
      }
   }
   // put the compositeMove itself
   compositeMoves.Put(CMPos,CM);
   return true;
}

/*
~AddPeriodMove~

This functions append the periodic move contained in the nested list
 __value__ to this periodic moving point.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoint::AddPeriodMove(const ListExpr value,int &LMIndex, int &CMIndex,
                             int &SMIndex, int &PMIndex){

  __TRACE__


 int len = ::nl->ListLength(value); 
 if(len!=2 ){  // (repeatations <submove>) 
    if(DEBUG_MODE)
       cerr << __POS__ << ": wrong listlength" << endl;
    return false;
 }
 if(::nl->AtomType(::nl->First(value))!=IntType){
    if(DEBUG_MODE){
       cerr << __POS__ << ": wrong type for repeatations" << endl;
    }
    return false;
 }
 int rep = ::nl->IntValue(::nl->First(value));
 // rep must be greater than 1 
 if(rep<=1){
     if(DEBUG_MODE){
        cerr << __POS__ <<  " wrong number of repeatations" << endl;
     }
     return false;
 }

 ListExpr SML;
 SML = ::nl->Second(value);
 if(::nl->ListLength(SML)!=2){
     if(DEBUG_MODE){
        cerr << __POS__ << ": wrong length for submove" << endl;
     }
     return false;
 }
 SpatialPeriodicMove PM(1);
 PM.repeatations = rep;
 int IncludePos = PMIndex; // store the positiuon
 PMIndex++;
 ListExpr SMT = ::nl->First(SML); // take the submove type
 if(::nl->IsEqual(SMT,"linear")){
    int LMPos = LMIndex;
    if(!AddLinearMove(::nl->Second(SML),LMIndex,CMIndex,SMIndex,PMIndex)){
       if(DEBUG_MODE){
          cerr << __POS__ << ": can't add linear submove" << endl;
       }
       return false;
    }
    PM.submove.arrayNumber = LINEAR;
    PM.submove.arrayIndex = LMPos;
    LinearPointMove LM;
    linearMoves.Get(LMPos,LM);
    PM.bbox.Equalize((LM.BoundingBox()));
    RelInterval SMI = LM.interval;
    PM.interval.Equalize(&SMI);
    PM.interval.Mul(rep);
    if(len==4){
      ListExpr LC = ::nl->Second(value);
      ListExpr RC = ::nl->Third(value);
      if((::nl->AtomType(LC)!=BoolType) || (::nl->AtomType(RC)!=BoolType))
          return false;
      PM.interval.SetLeftClosed(::nl->BoolValue(LC));
      PM.interval.SetRightClosed(::nl->BoolValue(RC));

     }
    periodicMoves.Put(IncludePos,PM);
    return true;
 }else if(::nl->IsEqual(SMT,"composite")){
    int CMPos = CMIndex;
    if(!AddSpatialCompositeMove(::nl->Second(SML),LMIndex,
                                CMIndex,SMIndex,PMIndex)){
       if(DEBUG_MODE){
          cerr << __POS__ << ": can't add composite submove" << endl;
       }
       return false;
    }
    PM.submove.arrayNumber = COMPOSITE;
    PM.submove.arrayIndex = CMPos;
    SpatialCompositeMove CM(0);
    compositeMoves.Get(CMPos,CM);
    PM.bbox.Equalize(&(CM.bbox));
    RelInterval SMI = CM.interval;
    PM.interval.Equalize(&SMI);
    PM.interval.Mul(rep);
    periodicMoves.Put(IncludePos,PM);
    return true;
 }
 // not of type linear or composite
 if(DEBUG_MODE){
     cerr << __POS__ << ": invalid submove-type for periodic move" << endl;
     cerr << "This list is : " << endl;
     ::nl->WriteListExpr(SMT);
     cerr << endl << "end of list " << endl;
 }
 return false;

}

/*
~At~

This Function computes the location of this Point at the given instant.
If the periodic moving point is not defined at this point of time, the
result will be an undefined Point instance.

[3] O(L), where L is the number of contained linear moves

*/
void PMPoint::At(const DateTime* instant,Point& res)const{

    __TRACE__
    if(IsEmpty()){
       res.SetDefined(false);
       return;
    }
    res.SetDefined(true);
    DateTime* duration = new DateTime(instanttype);
    duration->Equalize(instant);
    duration->Minus(&startTime); // now it is really a duration
    if(!interval.Contains(duration)){
       res.SetDefined(false);
    } else{
       const SubMove* sm = &submove;
       SpatialCompositeMove CM(0);
       SpatialPeriodicMove PM(0);
       RelInterval RI;
       // i have to find the linear move which is "active" at instant
       while(sm->arrayNumber!=LINEAR){
          if(sm->arrayNumber==COMPOSITE){
             // in this stage of implementation a linear search is
             // executed. i have to make it better in the future
             int i = sm->arrayIndex;
             compositeMoves.Get(i,CM);
             int min = CM.minIndex;
             int max = CM.maxIndex;
             //bool found=false;
             CSubMove csm;
             // perform binary search
             while(min<max){
                 int mid = (min+max)/2;
                 CSubMove csm; 
                 compositeSubMoves.Get(mid,csm);
                 DateTime prev(csm.duration);
                 int cmp = duration->CompareTo(&prev);
                 if(cmp < 0){ // duration is before mid
                    max = mid-1;
                 }else if(cmp==0){
                    RelInterval ri;
                    GetInterval(csm,ri);
                    if(ri.IsLeftClosed()){ // begin of the mid interval
                       duration->SetToZero();  
                       min = mid;
                       max = mid;
                    } else{ // end of the mid-1 interval
                       mid--;
                       compositeSubMoves.Get(mid,csm);
                       min = mid;
                       max = mid; 
                       GetInterval(csm,ri); // set to the end of 
                                             // the mid-1 interval
                       DateTime l;
                       ri.GetLength(l);
                       duration->Equalize(&l);
                    }
                 } else{ // cmp>0 duration my be in mid or after it
                      RelInterval ri;
                      GetInterval(csm,ri);
                      DateTime len;
                      ri.GetLength(len);
                      DateTime sum = prev + len;
                      int cmp2 = duration->CompareTo(&sum);
                      if(cmp2<0){ // duration located in this interval
                          min = mid;
                          max = mid;
                          duration->Minus(&prev);
                      } else if(cmp2==0){
                          if(ri.IsRightClosed()){
                             min = mid;
                             max = mid;
                             duration->Minus(&prev);
                          } else{ // start of the following son
                             mid++;
                             compositeSubMoves.Get(mid,csm);
                             min = mid;
                             max = mid;
                             duration->SetToZero();
                          }
                      } else { // cmp2 > 0
                           min = mid+1;
                      }
                 }
             }              
             compositeSubMoves.Get(min,csm);
             //found = true;
             SubMove sm1;
             sm1.arrayNumber = csm.arrayNumber;
             sm1.arrayIndex = csm.arrayIndex;
             sm = &sm1;
          } else if(sm->arrayNumber==PERIOD){
             int index = sm->arrayIndex;
             periodicMoves.Get(index,PM);
             // this is a very slow implementation
             // i have to speed up it in the future
             RelInterval RI;
             sm = &(PM.submove);
             if(PM.submove.arrayNumber==LINEAR){
                 LinearPointMove LM;
                 linearMoves.Get(PM.submove.arrayIndex,LM);
                 RI = LM.interval;
             } else if(PM.submove.arrayNumber==COMPOSITE){
                 compositeMoves.Get(PM.submove.arrayIndex,CM);
                 RI = CM.interval;
             } else { //another submoves are not allowed
                 assert(false);
             }
             while(!RI.Contains(duration)){
                DateTime* L = RI.GetLength();
                duration->Minus(L);
                delete L;
                L = NULL;
             }
          } else{
             // this case should never occurs
             assert(false);
          }
       }
       LinearPointMove LM;
       linearMoves.Get(sm->arrayIndex,LM);
       if(LM.IsDefinedAt(duration)){
          LM.At(duration,res);
       }
       else{
          res.SetDefined(false);
       }
    }
    delete duration;
    duration = NULL;
}

/*
~Initial~

This function computes the first location of this moving point.

*/
void PMPoint::Initial(Point& res)const{
    __TRACE__
  if(IsEmpty()){
    res.SetDefined(false);
    return;
  }
    
  LinearPointMove lm;
  linearMoves.Get(0,lm);
  res.SetDefined(true);
  res.Set(lm.startX,lm.startY);
}

/*
~Final~

The ~Final~ function returns the last defined position of this point.
If the value is empty, an undefined point is returned.

*/
bool PMPoint::Final(Point& res){
    __TRACE__
  if(IsEmpty()){
     res.SetDefined(false);
     return false;
  }       
  LinearPointMove lm =  GetLastUnit();
  res.SetDefined(true);
  res.Set(lm.endX,lm.endY);
  return true;
}

/*
~Translate~

Moves this PMPoint within time.

*/
void PMPoint::Translate(const DateTime& duration){
   startTime += duration;
}

void PMPoint::Translate(const DateTime* duration,PMPoint& res)const{
   res.Equalize(this);
   res.startTime += *duration;
}

/*
~GetLastUnit~

This function returns the temporal last unit of this periodic moving point.

*/
LinearPointMove PMPoint::GetLastUnit(){
    __TRACE__
 // In the current version of the implementation we can just returm the 
  // last unit in the array. If the implementation is changed to reuse 
  // a subtree, we have to go the tree downwards to find it.
  int pos = linearMoves.Size()-1;
  LinearPointMove res;
  linearMoves.Get(pos,res);
  return res; 
}


/*
~Breakpoints~

This function computes the coordinates on which halt's the 
periodic moving point. The result corresponds to the 
positions of static units. A call of this function is
an abbreviation to a call of breakpoints(d,false), where
__d__ is a duration of length zero.

[3] O(L) where L is the number of the contained linear moves.

*/
Points* PMPoint::Breakpoints()const{
  Points* res = new Points(1);
  Breakpoints(*res);
  return res;
}

void PMPoint::Breakpoints(Points& res)const{
  __TRACE__
  DateTime DT(durationtype);
  Breakpoints(&DT,false,res);
}

/*
~BreakPoints~

This function computes the breakpoints for this pmpoint. Only
Breakpoints with a minimum break with lenth __duration__ are included in the
result. If the duration is less than zero, the result will be 
undefined. The same result will be occur if the duration is zero
and the inclusive argument is true.

*/
Points* PMPoint::Breakpoints(const DateTime* duration,
                             const bool inclusive)const{
  Points* res = new Points(1);
  Breakpoints(duration,inclusive,*res);
  return res;
}

void PMPoint::Breakpoints(const DateTime* duration, 
                          const bool inclusive, 
                          Points& res)const{
  __TRACE__
  res.Clear();
  if(!IsDefined() || !duration->IsDefined()){
      res.SetDefined(false);
      return;
  }
  if(duration->LessThanZero()){
      res.SetDefined(false);
      return; 
  }
  if(duration->IsZero() && inclusive){
     res.SetDefined(false);
     return;
  }
  res.SetDefined(true);
  int size = linearMoves.Size();
  res.StartBulkLoad();
  LinearPointMove LM;
  for(int i=0; i<size; i++){
      linearMoves.Get(i,LM);
      if(LM.IsDefined() && LM.IsStatic()){
         DateTime* L = LM.interval.GetLength();
         int cmp = L->CompareTo(duration);
         delete L;
         if(cmp>0 || (cmp==0 && inclusive)){
            Point P(true,LM.startX,LM.startY);
            res += P;
         }
      }
  }
  res.EndBulkLoad();
}



/*
~Trajectory~

This function computes the trajectory of a single periodic moving point.

[3] O(L), where L is the number of contained linear moves

*/
void PMPoint::Trajectory(Line& res)const{
    __TRACE__
  LinearPointMove LM;
  // each linear moves needs 2 halfsegments
  res.Clear();
  int size = linearMoves.Size();
  if(size>0){
    res.Resize(size*2);
  }
  res.StartBulkLoad();
  HalfSegment HS1;
  HalfSegment HS2;
  int edge=0;
  for(int i=0;i<size;i++){
     linearMoves.Get(i,LM);
     if(LM.IsDefined() && !LM.IsStatic() ){
        bool hasHS1=LM.GetHalfSegment(true, HS1);
        bool hasHS2=LM.GetHalfSegment(false, HS2);
        if(hasHS1 && hasHS2){
          HS1.attr.edgeno = edge;
          HS2.attr.edgeno = edge;
          edge++;
          res+=HS1;
          res+=HS2;
        }
     }
  }
  res.EndBulkLoad();
}

/*
~GetStart~

This function returns the first point in time at which this
PMPoint is defined.

*/
DateTime PMPoint::GetStart()const{
    __TRACE__
   return startTime;
}

/*
~GetEnd~

This function returns the last point in time at which this
PMPoint is defined.

*/
DateTime PMPoint::GetEnd()const{
    __TRACE__
   DateTime end;
    end.Equalize(&startTime);
    end.Add(interval.GetLength());
    return end;
}

/*
~GetInterval~

Returns the interval of this periodic moving point. 

*/
PInterval PMPoint::GetInterval()const{
    __TRACE__
   PInterval res=PInterval(startTime,interval);
   return res;
}


/*
~Expand~

This function converts a periodic moving point to a linearly moving one.
The used data type comes from the TemporalAlgebra. 

*/
MPoint PMPoint::Expand()const{
    __TRACE__
  // In a first step we compute the number of resulting units.
  // The reason is, to avoid frequently growing of the result.
  MPoint res(1);
  Expand(res);
  return res;
}


void PMPoint::Expand(MPoint& res)const{
    __TRACE__
  // In a first step we compute the number of resulting units.
  // The reason is, to avoid frequently growing of the result.
  int size = NumberOfExpandedUnits();
  res.Clear(); 
  if(size==0){
    return; 
  }
  res.Resize(size);
  DateTime* CurrentTime = startTime.Clone();
  res.StartBulkLoad();
  AppendUnits(res, CurrentTime,submove);
  res.EndBulkLoad(false);
  delete CurrentTime;
}




/*
~AppendUnits~

The function ~AppendUnits~ adds all mpoint-units resulting from S to P.

*/
void PMPoint::AppendUnits(MPoint& P, DateTime* Time, const SubMove S)const{
    __TRACE__
   if(S.arrayNumber==LINEAR){
        // first create the Intervall
        LinearPointMove LM;
        linearMoves.Get(S.arrayIndex,LM);
        DateTime* StartTime = new DateTime((*Time));
        DateTime* length = LM.interval.GetLength();
        Time->Add(length);
        assert(!length->LessThanZero());
        delete length;
        length = NULL;
        Interval<DateTime> I((*StartTime),(*Time),
                     LM.interval.IsLeftClosed(),
                     LM.interval.IsRightClosed());
        UPoint up(I,LM.startX, LM.startY, LM.endX, LM.endY);
        int size = P.GetNoComponents();
        if(size > 0){
          UPoint last;
          P.Get(size-1,last);
          if(last.timeInterval.rc && I.lc && 
             last.timeInterval.end ==  I.start){
             if(last.timeInterval.start == last.timeInterval.start){
                P.Put(size - 1, up);
             } else if(I.start != I.end){
                up.timeInterval.lc=false;
                P.MergeAdd(up);
             } else {
                ; 
             }            
          } else {
            P.MergeAdd(up);
          }
        } else {
           P.MergeAdd(up);
        }
        delete StartTime;
        StartTime = NULL;
        return;
    }
    if (S.arrayNumber==COMPOSITE){
       SpatialCompositeMove CM(0);
       compositeMoves.Get(S.arrayIndex,CM);
       CSubMove csm;
       for(int i=CM.minIndex;i<=CM.maxIndex;i++){
            compositeSubMoves.Get(i,csm);
            AppendUnits(P,Time,csm);
       }    
       return;
    }
    if(S.arrayNumber==PERIOD){
       SpatialPeriodicMove PM(0);
       periodicMoves.Get(S.arrayIndex,PM);
       long repeats = PM.repeatations;
       for(int i=0;i<repeats;i++)
          AppendUnits(P,Time,PM.submove);
       return;      
    }
    assert(false);
}


/*
~NumberOfExpandedUnits~

This functions returns the number of needed units of a MPoint of the
TemporalAlgebra to represent this periodic moving point.

*/
int PMPoint::NumberOfExpandedUnits()const{
    __TRACE__
  if(!IsDefined())
      return 0;
   if(IsEmpty())
      return 0;
   return NumberOfExpandedUnits(submove);
}

/*
~NumberOfExpandedUnits~

This function computed the needed size of a MPoint of the TemporalAlgebra
to represent this periodic moving one. 

*/
int PMPoint::NumberOfExpandedUnits(const SubMove S)const{
    __TRACE__
   if(S.arrayNumber==LINEAR)
       return 1;
    if(S.arrayNumber==COMPOSITE){
       SpatialCompositeMove CM(0);
       compositeMoves.Get(S.arrayIndex,CM);
       int num = 0;
       CSubMove sm;
       for(int i=CM.minIndex;i<=CM.maxIndex;i++){
           compositeSubMoves.Get(i,sm);
           num += NumberOfExpandedUnits(sm);
       }    
       return num;
    } 
    if(S.arrayNumber==PERIOD){
       SpatialPeriodicMove PM(0);
       periodicMoves.Get(S.arrayIndex,PM);
       int repeats = PM.repeatations;
       return repeats*NumberOfExpandedUnits(PM.submove);
    }
    assert(false); // this should never be reached
}

/*
ReadFrom

This function reads a Periodic moving point from 
a lineary moving one.

*/
void PMPoint::ReadFrom(const MPoint& P, const bool twostep/* = true*/){
  /* This function works as follow:
     First, we create a list containing all LinearMovingPoints for this
     Periodic Moving Points. After that, we find equal units in this list
     and assign each different unit with an unique number. From this array
     of numbers a repetition tree is build. This tree automatically  detect
     repetitions in this numberlist. From this tree, the final periodic moving
     point is created.
  */
  int noUPoints = P.GetNoComponents();
  // Unfortunately, we can't use a array directly because we need to
  // represent non-defined units directly which is not required in
  // the MPoint representation.
  List<LinearPointMove>* L= new List<LinearPointMove>();  
  UPoint UP;
  bool lc,rc,wlc=false;
  DateTime start,end;
  double x1,y1,x2,y2;

  // standard-init

  SetDefined(true);
  canDelete = false;
  bbox.SetEmpty();
  linearMoves.clean();
  compositeMoves.clean();
  compositeSubMoves.clean();
  periodicMoves.clean();
  UPoint LastMove;
  if(noUPoints>0){ // we have any units
     for(int i=0;i<noUPoints;i++){
        P.Get(i,UP);
        // get the values from the unit
        lc = UP.timeInterval.lc;   
        rc = UP.timeInterval.rc;
        start = UP.timeInterval.start;
        end   = UP.timeInterval.end;
        x1 = UP.p0.GetX();
        y1 = UP.p0.GetY();
        x2 = UP.p1.GetX();
        y2 = UP.p1.GetY();    
        DateTime Length = end-start;       
        LinearPointMove theMove(0);
        theMove.defined=true;
        theMove.isStatic = (x1==x2) && (y1==y2);
        theMove.startX = x1;
        theMove.startY = y1;
        theMove.endX = x2;
        theMove.endY = y2;
        theMove.interval.Set(&Length,lc,rc);
        theMove.interval.SetDefined(true);
        // Add the bounding to to the complete bb
        PBBox tmpbox(theMove.BoundingBox());
        bbox.Union(&tmpbox);
        if(i==0){
           startTime = start; // sets the starttime
           interval.SetDefined(true);
           wlc = UP.timeInterval.lc;
        }else {
           if( ((LastMove.timeInterval.end != start)) ||
               (!LastMove.timeInterval.rc && !lc)){
               // a gap found between the last unit and this unit
               LinearPointMove GapMove(0);
               GapMove.defined=false; // an undefined move
               DateTime GapLength = start-LastMove.timeInterval.end;
               GapMove.interval.Set(&GapLength,
                                     !LastMove.timeInterval.rc, !lc);
               GapMove.interval.SetDefined(true);
               L->Append(GapMove);
           }
        }
        if(i==noUPoints-1){ // set new interval;
           DateTime Len = end-startTime;
           interval.Set(&Len,wlc,UP.timeInterval.rc); 
        }
        L->Append(theMove);
        LastMove = UP;
     }
  }else{ // no units available
    // here we have to initialize some components
  }

  // At this point, we have stored all LinearPointsMoves in a List.
  // now we have to build an array of numbers for it. To make this fast,
  // we use a hashtable with double maximum size. We use closed hashing 
  // with a linear collision strategy.
  int hashsize = L->GetLength()*2;

  int MinIndex[hashsize]; // Hashtable containing the minimum 
                          //index of this move
                          // in the AllMoves array
  int LMIndex[hashsize]; // The same table but holding the indices in the 
                         // resulting linear moves
  // initialize the hashtable
  for(int i=0;i<hashsize;i++){ 
       MinIndex[i]=-1;
       LMIndex[i] =-1;
  }

  int listlength=L->GetLength();
  LinearPointMove* AllMoves = L->ConvertToArray();

  int indexInAllMoves[listlength];
  int indexInLinearMoves[listlength];

  for(int i=0;i<listlength;i++){
     indexInAllMoves[i] = -1;
     indexInLinearMoves[i] = -1;
  }

  int differentMoves =0; // number of different linear moves
  //int lastusedindex = -1;

  if(twostep){
     cout << "Warning time component ignored !!!" << endl;
  }

  // we assign each different value in the array to an unique number
  for(int i=0;i<listlength;i++){
     LinearPointMove theMove = AllMoves[i];
     size_t hashvalue = theMove.HashValue()%hashsize;
 
     // search a free index or the same move
     bool done = false;
     while(!done){
        if(MinIndex[hashvalue]<0){ // empty slot found
           // put the index of the move into the hashtable
           MinIndex[hashvalue] = i;
           LMIndex[hashvalue] = differentMoves;
           //lastusedindex=i;
           differentMoves++;
           done = true;
        }
        else{
           int cmp=twostep?
                      (AllMoves[MinIndex[hashvalue]]).CompareSpatial(&theMove)
                     :(AllMoves[MinIndex[hashvalue]]).CompareTo(&theMove);
           if(cmp==0) {
              //equal element found; we are done
              done = true;
           }
           else{
              hashvalue = (hashvalue+1)%hashsize; // collision detected
           }
        }
     }
     indexInAllMoves[i]= MinIndex[hashvalue];
     indexInLinearMoves[i] = LMIndex[hashvalue];
  } 

  /* At this in, in assigned numbers the complete moving point is stored
     as sequel of array indices in the array AllMoves. Now we can build a
     repetition tree from this numbers.
  */
 
 
  RepTree RT(indexInLinearMoves,listlength);
  
  /* Debugging output */
  //RT.PrintNL();

  if(twostep){
    // in the twostep process it is possible that spatial repetations has
    // been recognized but the temporal deviation is too much to accept that
    // as a periodic move
    // so we have to check all recognized periodic nodes for temporal
    // deviations in the move 
         
 

  }



  linearMoves.clean();
  compositeMoves.clean();
  compositeSubMoves.clean();
  periodicMoves.clean();

  if(listlength==0){
     SetDefined(false);
     canDelete=false;
   }else{
     SetDefined(true);
     canDelete = false;
     // set all array to the required sizes
     if(differentMoves>0)
        linearMoves.resize(differentMoves);
     int size = RT.NumberOfNodesWithType(COMPOSITION);
     if(size>0)
        compositeMoves.resize(size);
     size = RT.NumberOfNodesWithType(REPETITION);
     if(size>0)
        periodicMoves.resize(size);
     size = RT.NumberOfCompositeSons();
     if(size>0)
        compositeSubMoves.resize(size);
     // in the first step, we copy the linear moves into the
     // appropriate array
     int lastnumber = -1;
     for(int i=0;i<listlength;i++){
         if(indexInLinearMoves[i]>lastnumber){
             lastnumber = indexInLinearMoves[i];
             linearMoves.Put(lastnumber,AllMoves[indexInAllMoves[i]]); 
         }
     }

      // analyse the tree and make the entries in the arrays
     int cp,csp,pp;
     cp=0,csp=0,pp=0;
     FillFromRepTree(cp,csp,pp,RT);

     int type = RT.GetNodeType();
     this->submove.arrayIndex=0;
     if(type==SIMPLE)
       this->submove.arrayNumber=LINEAR;
     else if(type==COMPOSITE)
       this->submove.arrayNumber=COMPOSITE;
     else if(type==REPETITION)
       this->submove.arrayNumber=PERIOD;
     else
       assert(false);

     SetDefined(true);
   } // moves exist
 
   L->Destroy();
   delete L; 
   L = NULL;
   CorrectDurationSums();
   TrimToSize();
}

/*
~FillFromRepTree~

This function fills the composite Array, the compositeSubMoveArray and the
periodic Move array. The linearPointArray must be initializes before calling
this function. The Parameters describe the first free index positions for
this tree. After alling this function, the arguments are incremented with the
space occupied by this tree.

*/ 
bool PMPoint::FillFromRepTree(int& cp,int& csp, int& pp, RepTree TR){
  int type = TR.GetNodeType();
  if(type==SIMPLE){
     // simple nodes correspond to linearpointmoves
     // for this reason, we have to do nothing in this case.
     return true;
  }
  int oldcp=cp;
  int oldcsp=csp;
  int oldpp=pp;
  
  if(type==REPETITION){
    // first include the son of this repetition
    pp++;
    RepTree* son = TR.GetSon(0);
    if(!FillFromRepTree(cp,csp,pp,*son)) // error
       return  false;
    // create a new SpatialPeriodicMove
    SpatialPeriodicMove SPM(0);
    int sontype = son->GetNodeType();
    if(sontype==SIMPLE){
      // we need the son for receiving some information
      LinearPointMove LPM(0);
      linearMoves.Get(son->Content(),LPM);
      SPM.repeatations = TR.RepCount();
      SPM.submove.arrayNumber=LINEAR;
      SPM.submove.arrayIndex=son->Content();
      SPM.interval.Equalize(&LPM.interval);
      SPM.interval.Mul(SPM.repeatations);
      SPM.bbox = LPM.BoundingBox();
    } else if(sontype==COMPOSITE){
      SpatialCompositeMove SCM(0);
      compositeMoves.Get(oldcp,SCM); 
      SPM.repeatations = TR.RepCount();
      SPM.submove.arrayNumber=COMPOSITE;
      SPM.submove.arrayIndex=oldcp;
      SPM.interval.Equalize(&SCM.interval);
      SPM.interval.Mul(SPM.repeatations);
      SPM.bbox = SCM.bbox;    
    } else{
      // we don't allow other types to be a son of a 
      // repetition
      return false; 
    }
    periodicMoves.Put(oldpp,SPM);
    return true; 
  } // type==repetition

  if(type==COMPOSITE){
    int nos = TR.NumberOfSons();
    csp = csp + nos;  
    cp++;
    SpatialCompositeMove SCM(0);
    SCM.minIndex=oldcsp;
    SCM.maxIndex=csp-1;
    int currentrep=oldpp;
    // insert all submoves
    for(int i=0;i<nos;i++){
      RepTree* CurrentSon = TR.GetSon(i);
      //insert the son
      if(!FillFromRepTree(cp,csp,pp,*CurrentSon))
        return false;
      // create submove
      CSubMove SM;
      int sontype = CurrentSon->GetNodeType();
      if(sontype==SIMPLE){
          SM.arrayNumber=LINEAR;
          SM.arrayIndex=CurrentSon->Content();
          LinearPointMove LPM(0);
          linearMoves.Get(CurrentSon->Content(),LPM);
          if(i==0){ // the first submove
             SCM.interval.Equalize(&LPM.interval);
             SCM.bbox.Equalize(LPM.BoundingBox());
          }else{
            SCM.interval.Plus(&(LPM.interval));
            PBBox tmpbox(LPM.BoundingBox());
            SCM.bbox.Union(&tmpbox);
          }
       }else if(sontype==REPETITION){
          SM.arrayNumber=PERIOD;
          SM.arrayIndex=currentrep;
          currentrep=pp;
          SpatialPeriodicMove SPM(0);
          periodicMoves.Get(SM.arrayIndex,SPM);
          if(i==0){
            SCM.interval.Equalize(&SPM.interval);
            SCM.bbox.Equalize(&SPM.bbox);
          } else{
            SCM.interval.Plus(&SPM.interval);
            SCM.bbox.Union(&SPM.bbox);
          }

       } else{
         // we don't allow a composite move to be a
         // son of another composite move
         return false;
       }
       compositeSubMoves.Put(oldcsp+i,SM);
    } // for each submove
    compositeMoves.Put(oldcp,SCM); // insert the move
    return true;
  }
  // other type are not present
  assert(false);

}

/*
~GetBBox~

The function GetBBox returns the bounding Box of this periodic
moving point.

*/
PBBox PMPoint::GetBbox()const{
     __TRACE__
  return bbox;
}


/*
~Toprel~

This operator computed the topological relationship between this and 
the argument. 

*/
PMInt9M* PMPoint::Toprel(const Point& P)const{
  __TRACE__
  PMInt9M* res = new PMInt9M(0);
  Toprel(P,*res);
  return  res;
}

void PMPoint::Toprel(const Point& P, PMInt9M& res)const{
  __TRACE__
  // first, we create an array of the same size as the 
  // size of the linearmoves
  int rs = linearMoves.Size();
  ArrayRange ranges[rs];
  int lastPos=0;
  LinearPointMove LPM;
  DbArray<LinearInt9MMove> UnitTopRels(linearMoves.Size()); 
  LinearInt9MMove buffer[3]; 
  for(int i=0;i<linearMoves.Size();i++){
      linearMoves.Get(i,LPM);
      int count = LPM.Toprel(P,buffer);     
      for(int j=0;j<count;j++){
         UnitTopRels.Append(buffer[j]);  
      }
      ranges[i].minIndex=lastPos;
      ranges[i].maxIndex=lastPos+count-1;
      lastPos=lastPos+count;
  }
  DbArray<CompositeMove> CMs(compositeMoves.Size());
  DbArray<PeriodicMove> PMs(periodicMoves.Size());
  SpatialCompositeMove SCM(0);
  CompositeMove CM;
  for(int i=0;i<compositeMoves.Size();i++){
      compositeMoves.Get(i,SCM);
      CM = SCM.ToCompositeMove();   
      CMs.Put(i,CM);
  }
  SpatialPeriodicMove SPM;
  PeriodicMove PM;
  for(int i=0;i<periodicMoves.Size();i++){
     periodicMoves.Get(i,SPM);
     PM = SPM.ToPeriodicMove();
     PMs.Put(i,PM);
  } 
  res.CreateFrom(UnitTopRels,ranges,rs,CMs,compositeSubMoves,
                        PMs,startTime,submove);
  res.Minimize();
}

/*
~Toprel~

This operator computed the topological relationship between this and 
the argument. 

*/
void PMPoint::Toprel(const Points& P, PMInt9M& result)const {
  __TRACE__
  // first, we create an array of the same size as the 
  // size of the linearmoves
  int rs = linearMoves.Size();
  ArrayRange ranges[rs];
  int lastPos=0;
  LinearPointMove LPM;
  DbArray<LinearInt9MMove> UnitTopRels(linearMoves.Size()); 
  vector<LinearInt9MMove> MyBuffer; 
  for(int i=0;i<linearMoves.Size();i++){
      linearMoves.Get(i,LPM);
      LPM.Toprel(P,MyBuffer);
      int count = MyBuffer.size();    
      for(int j=0;j<count;j++){
         UnitTopRels.Append(MyBuffer[j]);  
      }
      ranges[i].minIndex=lastPos;
      ranges[i].maxIndex=lastPos+count-1;
      lastPos=lastPos+count;
  }
  DbArray<CompositeMove> CMs(compositeMoves.Size());
  DbArray<PeriodicMove> PMs(periodicMoves.Size());
  SpatialCompositeMove SCM(0);
  CompositeMove CM;
  for(int i=0;i<compositeMoves.Size();i++){
      compositeMoves.Get(i,SCM);
      CM = SCM.ToCompositeMove();   
      CMs.Put(i,CM);
  }
  SpatialPeriodicMove SPM;
  PeriodicMove PM;
  for(int i=0;i<periodicMoves.Size();i++){
     periodicMoves.Get(i,SPM);
     PM = SPM.ToPeriodicMove();
     PMs.Put(i,PM);
  } 
  result.CreateFrom(UnitTopRels,ranges,rs,CMs,compositeSubMoves,PMs,
                     startTime,submove);
  cout << "Linear Moves before minimizations : " 
       << result.NumberOfLinearMoves() << endl;
  result.Minimize();
  cout << "Linear Moves after minimization : " 
       << result.NumberOfLinearMoves() << endl;
}


PMInt9M* PMPoint::Toprel(const Points& P)const {
  PMInt9M* res = new PMInt9M(1);
  Toprel(P,*res);
  return res;
}


/*
~DistanceTo~

This function computes the distance to a fixed position in [R2] as 
periodic moving real;


*/
bool PMPoint::DistanceTo(const double x, const double y, PMReal& result)const {
   // spcial case: this pmpoint is not defined
   if(!IsDefined()){
     result.SetDefined(false);
     return true; 
   }
   result.SetDefined(true);
   // copying the tree structure as well as the interval and the
   // submove into the result.
   RelInterval* resInterval = result.GetInterval();
   resInterval->Equalize(&interval);
   SubMove* SM =result.GetSubmove();
   SM->Equalize(&submove);
   result.SetStartTime(startTime);
   

   // copy periodic moves
   DbArray<PeriodicMove>* resPMs = result.GetPeriodicMoves();   
   resPMs->clean();
   int size;
   if((size=periodicMoves.Size())>0){
      PeriodicMove PM;
      SpatialPeriodicMove SPM;
      resPMs->resize(size);
      for(int i=0;i<size;i++){
         periodicMoves.Get(i,SPM);
         SPM.ToPeriodicMove(PM);
         resPMs->Put(i,PM);
      }
   }
   // copy composite moves
   DbArray<CompositeMove>* resCMs = result.GetCompositeMoves();
   resCMs->clean();
   if((size=compositeMoves.Size())>0){
       CompositeMove CM;
       SpatialCompositeMove SCM(0);
       resCMs->resize(size);
       for(int i=0;i<size;i++){
          compositeMoves.Get(i,SCM);
          SCM.ToCompositeMove(CM);
          resCMs->Put(i,CM);    
       }
   }

   // copy composite submoves
   DbArray<CSubMove>* resSMs = result.GetCompositeSubMoves();
   resSMs->clean();
   if((size=compositeSubMoves.Size())>0){
      CSubMove SM;
      resSMs->resize(size);
      for(int i=0;i<size;i++){
        compositeSubMoves.Get(i,SM);
        resSMs->Put(i,SM);
      }
   }
  
  // now, we build the linear moves for the 
  // periodic moving real
  DbArray<MovingRealUnit>* resLin = result.GetLinearMoves();
  MovingRealUnit Unit;
  resLin->clean();
  if((size=linearMoves.Size())>0){
    resLin->resize(size);
    LinearPointMove LPM;
    for(int i=0;i<size;i++){
      linearMoves.Get(i,LPM);
      LPM.DistanceTo(x,y,Unit);
      resLin->Put(i,Unit); 
    }
  }
  return true;
}

/*
~Speed~ and ~Direction~

This function computes the  speed of this PMPoint.


*/
void PMPoint::SpeedAndDirection(bool isSpeed, PMReal& result)const {
   // special case: this pmpoint is not defined
   if(!IsDefined()){
     result.SetDefined(false);
     return; 
   }
   result.SetDefined(true);
   // copying the tree structure as well as the interval and the
   // submove into the result.
   RelInterval* resInterval = result.GetInterval();
   resInterval->Equalize(&interval);
   SubMove* SM =result.GetSubmove();
   SM->Equalize(&submove);
   
   result.SetStartTime(startTime);
   

   // copy periodic moves
   DbArray<PeriodicMove>* resPMs = result.GetPeriodicMoves();   
   resPMs->clean();
   int size;
   if((size=periodicMoves.Size())>0){
      PeriodicMove PM;
      SpatialPeriodicMove SPM;
      resPMs->resize(size);
      for(int i=0;i<size;i++){
         periodicMoves.Get(i,SPM);
         SPM.ToPeriodicMove(PM);
         resPMs->Put(i,PM);
      }
   }
   // copy composite moves
   DbArray<CompositeMove>* resCMs = result.GetCompositeMoves();
   resCMs->clean();
   if((size=compositeMoves.Size())>0){
       CompositeMove CM;
       SpatialCompositeMove SCM(0);
       resCMs->resize(size);
       for(int i=0;i<size;i++){
          compositeMoves.Get(i,SCM);
          SCM.ToCompositeMove(CM);
          resCMs->Put(i,CM);    
       }
   }

   // copy composite submoves
   DbArray<CSubMove>* resSMs = result.GetCompositeSubMoves();
   resSMs->clean();
   if((size=compositeSubMoves.Size())>0){
      CSubMove SM;
      resSMs->resize(size);
      for(int i=0;i<size;i++){
        compositeSubMoves.Get(i,SM);
        resSMs->Put(i,SM);
      }
   }
  
  // now, we build the linear moves for the 
  // periodic moving real
  DbArray<MovingRealUnit>* resLin = result.GetLinearMoves();
  MovingRealUnit Unit;
  resLin->clean();
  if((size=linearMoves.Size())>0){
    resLin->resize(size);
    LinearPointMove LPM;
    for(int i=0;i<size;i++){
      linearMoves.Get(i,LPM);
      if(isSpeed){
         LPM.Speed(Unit);
      } else {
         LPM.Direction(Unit);
      }
      resLin->Put(i,Unit); 
    }
  }
  result.Minimize();
}
/*
~CorrectDurationSums~

*/
void PMPoint::CorrectDurationSums(){
   if(IsEmpty() || !IsDefined()){  // nothing to do
      return;
   }
   int cmsize = compositeMoves.Size();
   RelInterval currentInterval;
   DateTime currentLength(durationtype);
   DateTime duration(durationtype);
   SpatialCompositeMove CM(0);
   // process all compositeMoves.
   for(int i=0;i<cmsize;i++){
      compositeMoves.Get(i,CM);
      duration.SetToZero();
      for(int j=CM.minIndex;j<=CM.maxIndex;j++){
         CSubMove csm1;
         compositeSubMoves.Get(j,csm1);
         CSubMove csm;
         csm.Equalize(&csm1);
         csm.duration.Equalize(&duration);
         compositeSubMoves.Put(j,csm);
         GetLength(csm,currentLength);
         duration += currentLength;
      }
   }   
} 

/*
~GetLength~

Returns the length of the submove specified as the argument.

*/
void PMPoint::GetLength(SubMove sm, DateTime& result){
   switch(sm.arrayNumber){
       case LINEAR:{
              LinearPointMove lpm;
              linearMoves.Get(sm.arrayIndex,lpm);
              lpm.interval.GetLength(result);
              break;
            }
       case COMPOSITE:{
              SpatialCompositeMove scm(0);
              compositeMoves.Get(sm.arrayIndex,scm);
              scm.interval.GetLength(result);
              break;
            }
       case PERIOD:{
              SpatialPeriodicMove spm;
              periodicMoves.Get(sm.arrayIndex,spm);
              spm.interval.GetLength(result);
              break;
            }
       default: 
              cerr << "unknown submove " << sm << endl;
              assert(false);
   }
}

/*
~GetInterval~

Returns the interval covered by the subtree given by the argument.

*/

void PMPoint::GetInterval(SubMove sm, RelInterval& result) const{
   switch(sm.arrayNumber){
       case LINEAR:{
              LinearPointMove lpm;
              linearMoves.Get(sm.arrayIndex,lpm);
              result.Equalize(&lpm.interval);
              break;
            }
       case COMPOSITE:{
              SpatialCompositeMove scm(0);
              compositeMoves.Get(sm.arrayIndex,scm);
              result.Equalize(&scm.interval);
              break;
            }
       case PERIOD:{
              SpatialPeriodicMove spm;
              periodicMoves.Get(sm.arrayIndex,spm);
              result.Equalize(&spm.interval);
              break;
            }
       default: 
              cerr << "unknown submove " << sm << endl;
              assert(false);
   }
}

/*
~Length~

This function computes the length of the route of the pmpoint.
This instance has to be defined.

*/
double PMPoint::Length()const{
  assert(IsDefined());
  return Length(submove);    
}

void PMPoint::Length(CcReal& res)const{
  if(!IsDefined()){
    res.SetDefined(false);
  } else{
    res.Set(true,Length());
  }
}


double PMPoint::Length(const SubMove& sm) const{
   switch(sm.arrayNumber){
       case LINEAR : {
         LinearPointMove lpm;
         linearMoves.Get(sm.arrayIndex,lpm);
         return lpm.Length();    
     } case COMPOSITE:{
         SpatialCompositeMove scm(0);
         CSubMove csm;
         compositeMoves.Get(sm.arrayIndex,scm);
         double res = 0.0;
         for(int i=scm.minIndex; i<= scm.maxIndex; i++){
           compositeSubMoves.Get(i,csm);
           res += Length(csm);
         }
         return res;
     } case PERIOD: {
         SpatialPeriodicMove spm;
         periodicMoves.Get(sm.arrayIndex,spm);
         return spm.repeatations * Length(spm.submove);
     }
     default: assert(false);
   }

}

} // end of namespace periodic 
