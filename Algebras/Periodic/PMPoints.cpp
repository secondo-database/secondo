/*
1.1 ~PMPoints~


*/


#include <iostream>
#include <string>

#include "PeriodicTypes.h"
#include "PeriodicSupport.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "StandardTypes.h"
#include "NestedList.h"
#include "DateTime.h"


extern NestedList* nl;


using namespace std;
using namespace datetime;

namespace periodic{


/*
3.9 ~PMPoints~

~Constructor~

This Constructor creates an undefined value of type PMPoints.
The argument is ignored. 

*/
PMPoints::PMPoints(int dummy):
   Attribute(false),
   linearMoves(0),
   thePoints(0),
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
~Destructor~

[3] O(1)

*/
PMPoints::~PMPoints(){
    __TRACE__
   if(canDelete){
      linearMoves.Destroy();
      thePoints.Destroy();
      compositeMoves.Destroy();
      compositeSubMoves.Destroy();
      periodicMoves.Destroy();
   }
}


/*
~Destroy~

[3] O(1)

*/
void PMPoints::Destroy(){
    __TRACE__
   canDelete=true;
}


/*
~Equalize~

This functions changes the value of this periodic moving point
to be equals to the given parameter.

[3] O(L), where L is the number of linear moves

*/
void PMPoints::Equalize(const PMPoints* P2){
    __TRACE__
 // equalize the arrays 
 Attribute::operator=(*P2);
 linearMoves.copyFrom(P2->linearMoves);
 thePoints.copyFrom(P2->thePoints);
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
~NumOfFLOBs~

This function returns the number of contained FLOBs in this
class. Because five DBarrays are managed here, the return value
is 5.

[3] O(1)

*/
int PMPoints::NumOfFLOBs() const{
    __TRACE__
   return 5;
}

/*
~GetFLOB~

This function returns the FLOB with index i.

[3] O(1)

*/
Flob* PMPoints::GetFLOB(const int i){
    __TRACE__
  assert(i>=0 && i<NumOfFLOBs());
   if(i==0) return &linearMoves;
   if(i==1) return &thePoints;
   if(i==2) return &compositeMoves;
   if(i==3) return &compositeSubMoves;
   if(i==4) return &periodicMoves;
   return 0;
}

/*
~Compare~

This function is not implemented at this moment.

[3] O(-1)

*/
int PMPoints::Compare(const Attribute*)const{
    __TRACE__
  cout << " Warning! PMPoints::Compare  not implemented" << endl;
   return 0;
}

/*
~Adjacent~

We can't defined a adjacent relation beween two periodic moving
pointsets. For this reason, the return value is allways __false__.

[3] O(1)

*/
bool PMPoints::Adjacent(const Attribute* )const{
    __TRACE__
  return false;
}

/*
~Clone~

The ~Clone~ function returns a copy of this.

[3] O(L)

*/
PMPoints* PMPoints::Clone() const{
    __TRACE__
 PMPoints* copy = new PMPoints(0);
  copy->Equalize(this);
  return copy;
}


/*
~Sizeof~

This function returns the size of the PMPoints-class

[3] O(1)

*/
size_t PMPoints::Sizeof()const{
    __TRACE__
  return sizeof(PMPoints);
}

/*
~IsEmpty~

Checks whether this pmpoints value has no content at all times.

*/
bool PMPoints::IsEmpty() const{
  return linearMoves.Size()==0;
}



/*
~HashValue~

Returns the HashValue for this Point.

[3] O(1)

*/
size_t PMPoints::HashValue() const{
    __TRACE__
 DateTime* L = interval.GetLength();
  size_t res = (size_t) (bbox.Size()+(int)L->GetDay());
  delete L;
  L = NULL;
  return res;
}

/*
~CopyFrom~

The PMPoints instances takes its value from the given argument.

[3] O(L)

*/
void PMPoints::CopyFrom(const Attribute* arg){
    __TRACE__
  Equalize((PMPoints*)arg);
}

/*
~ToListExpr~

This function returns the ListExpr representing this points.
The list will also contains the type if the flas is set. 
This means, this list
is a complete list in format (type value)

[3] O(L)

*/
ListExpr PMPoints::ToListExpr(const bool typeincluded)const{
    __TRACE__
   ListExpr value;
   if(!IsDefined())
      value = ::nl->BoolAtom(false);
   else{   
      ListExpr timelist = startTime.ToListExpr(false);
      ListExpr SubMoveList = GetSubMoveList(submove);
      value = ::nl->TwoElemList(timelist,SubMoveList);
   }
   ListExpr res;
   if(typeincluded)
      res =  ::nl->TwoElemList( ::nl->SymbolAtom("pmpoints"),value );
   else
      res =  value;
   return res;
}

ListExpr PMPoints::ToListExpr(const ListExpr typeInfo)const{
   return ToListExpr(false);
}


/*
~GetSubMove~

This functions determines the move from the given argument and
returns its nested list representation.

[3] O($L_{SM}$) , where $L_{SM}$ is the number of linear moves contained in SM

*/
ListExpr PMPoints::GetSubMoveList(const SubMove SM)const{
  __TRACE__
  ListExpr SubMoveList;
  int SubMoveType = SM.arrayNumber;
  int index = SM.arrayIndex;
  if(SubMoveType==LINEAR)
      SubMoveList = GetLinearMoveList(index);
  else if(SubMoveType==COMPOSITE)
      SubMoveList = GetSpatialCompositeMoveList(index);
  else if(SubMoveType==PERIOD)
      SubMoveList = GetSpatialPeriodicMoveList(index);
  else{
       cerr << __POS__ << " Error in creating ListExpr" << endl;
       SubMoveList = ::nl->TheEmptyList();
   }
  return SubMoveList;
}


/*
~GetLinearMove~

This functions returns the nested list representation of the
linear move at the specified index.

[3] O(1)

*/
ListExpr PMPoints::GetLinearMoveList(const int index)const{
    __TRACE__
   LinearPointsMove LM;
   linearMoves.Get(index,LM);
   ListExpr res =  LM.ToListExpr(thePoints);
   return res;
}

/*
~GetSpatialPeriodicMove~

This function converts the periodic move at the specified index
to its nested list representation.

[3] O($L_{P}$), where $L_{P}$ is the number of linear moves in the periodic move at index

*/
ListExpr PMPoints::GetSpatialPeriodicMoveList(const int index)const{
    __TRACE__
  SpatialPeriodicMove PM;
  periodicMoves.Get(index,PM);
  ListExpr periodtype = ::nl->SymbolAtom("period");
  ListExpr RepList = ::nl->IntAtom(PM.repeatations);
  ListExpr SML = GetSubMoveList(PM.submove);
  ListExpr LC = ::nl->BoolAtom(PM.interval.IsLeftClosed());
  ListExpr RC = ::nl->BoolAtom(PM.interval.IsRightClosed());
  return  ::nl->TwoElemList(periodtype,::nl->FourElemList(RepList,LC,RC,SML));
}

/*
~GetSpatialCompositeMoveList~

This function returns the nested list representation of the composite
move at the specified array index.

[3] O(L) , where L is the number of submoves contained in the linear move at index

*/
ListExpr PMPoints::GetSpatialCompositeMoveList(const int index)const{
    __TRACE__
 SpatialCompositeMove CM;
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
   SubMovesList = ::nl->OneElemList(GetSubMoveList(SM));
   ListExpr Last = SubMovesList;
   for(int i=minIndex+1;i<=maxIndex;i++){
     compositeSubMoves.Get(i,SM);
     Last = ::nl->Append(Last,GetSubMoveList(SM));
   }
 }
 return ::nl->TwoElemList(CType,SubMovesList);
}


/*
~ReadFrom~

This function reads the value of this p.m. points from the
given nested list. If the nested list don't contains a
valid points value, the return value will be false and this points
is set to be undefined. Otherwise the points has the value
described in the nested list. The list consists only of the
the value, this means the type description is not included
in this list.

[3] O(L)  where L = number of linear moves

*/
bool PMPoints::ReadFrom(const ListExpr value, const ListExpr typeInfo){
    __TRACE__
 /* The list is scanned twice. In the first scan we
     compute only the needed size of the contained arrays. The reason is,
     that we want to avoid frequently ~Resize~ on the arrays to ensure the
     given time complexity.
  */

  // for Debugging only
  //::nl->WriteListExpr(value);

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
        cerr << __POS__ << ": resizing arrays failed" << endl;
        cerr << "*************************************" << endl; 
     }
     SetDefined(false);
     return false;
  }

  if(!startTime.ReadFrom(::nl->First(value),false)){
     if(DEBUG_MODE){
        cerr << __POS__ << "reading of the start time failed" << endl;
        cerr << "The list is " << endl;
     }
     SetDefined(false);
     return false;
  }
  // now we have to append the included submove
  ListExpr SML = ::nl->Second(value);
  if(::nl->ListLength(SML)!=2){ // (submovetype value)
     if(DEBUG_MODE){
        cerr << __POS__ << ": wrong list length for submove" << endl;
     }
     SetDefined(false);
     return false;
  }


  ListExpr SMT = ::nl->First(SML);
  int LMIndex = 0;
  int PtsIndex = 0;
  int CMIndex = 0;
  int SMIndex = 0;
  int PMIndex = 0;
  bool res = false;
  if(::nl->IsEqual(SMT,"linear")){
     submove.arrayNumber = LINEAR;
     submove.arrayIndex = 0;

     if(!AddLinearMove(::nl->Second(SML),LMIndex,PtsIndex,
                       CMIndex,SMIndex,PMIndex)){
         if(DEBUG_MODE){
            cerr << __POS__ << " Error in reading linear move" << endl;
         }
         SetDefined(false);
         res = false;
     }else {
   SetDefined(true);
   LinearPointsMove LM;
   linearMoves.Get(0,LM);
   interval.Equalize(&(LM.interval));
   bbox.Equalize(&(LM.bbox));
   res = true;
     }
  } else if(::nl->IsEqual(SMT,"composite")){
     submove.arrayNumber=COMPOSITE;
     submove.arrayIndex = 0;
     if(!AddSpatialCompositeMove(::nl->Second(SML),LMIndex,PtsIndex,
        CMIndex,SMIndex,PMIndex)){
        if(DEBUG_MODE){
           cerr << __POS__ << "error in reading composite move" << endl;
        }
        SetDefined(false);
        res = false;
     } else {
   SetDefined(true);
   SpatialCompositeMove CM;
   compositeMoves.Get(0,CM);
   interval.Equalize(&(CM.interval));
   bbox.Equalize(&(CM.bbox));
   res = true;
     }   
  } else if(::nl->IsEqual(SMT,"period")){
     submove.arrayNumber = PERIOD;
     submove.arrayIndex = 0;
     if(!AddPeriodMove(::nl->Second(SML),LMIndex,PtsIndex,
                       CMIndex,SMIndex,PMIndex)){
        if(DEBUG_MODE){
          cerr << __POS__ << " error in reading periodic move" << endl;
        }
        SetDefined(false);
        res = false;
     } else {
   SetDefined(true);
   SpatialPeriodicMove PM;
   periodicMoves.Get(0,PM);
   interval.Equalize(&(PM.interval));
   bbox.Equalize(&(PM.bbox));
   res = true;
    }
  } else {
      if(DEBUG_MODE){
   cerr << __POS__ << "unknown subtype" << endl;
   ::nl->WriteListExpr(SMT);
      }
      res = false;
  }
  CorrectDurationSums();
  return res;
}



/*
~ResizeArrays~

This function resizes the arrays to the needed values.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoints::ResizeArrays(const ListExpr value){
    __TRACE__
 // first all entries in the arrays are removed
   linearMoves.clean();
   compositeMoves.clean();
   compositeSubMoves.clean();
   periodicMoves.clean();
   int LMSize = 0;
   int PtsSize = 0;
   int CMSize = 0;
   int SMSize = 0;
   int PMSize = 0;
   if(!AddSubMovesSize(::nl->Second(value),LMSize,PtsSize,CMSize,SMSize,PMSize))
      return false;
   // set the arrays to the needed size
   if(LMSize>0) linearMoves.resize(LMSize);
   if(PtsSize>0) thePoints.resize(PtsSize);
   if(CMSize>0) compositeMoves.resize(CMSize);
   if(SMSize>0) compositeSubMoves.resize(SMSize);
   if(PMSize>0) periodicMoves.resize(PMSize);
   return true;
}

/*
~AddSubMovesSize~

This function computes the needed sizes for the arrays to hold the
value of the p.m. points represented in the value list.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoints::AddSubMovesSize(const ListExpr value,int &LMSize, int &PtsSize,
                              int &CMSize, int &SMSize,int &PMSize){
    __TRACE__
  // all moves have the length 2
  if(::nl->ListLength(value)!=2){
     return false;
  }
  ListExpr type = ::nl->First(value);
  if(::nl->AtomType(type)!=SymbolType){
    return false;
  }
  // in a linear move we have to increment the size of LM
  // and to add the number of contained Points 
  if(::nl->IsEqual(type,"linear")){
     LMSize = LMSize +1;
     ListExpr val = ::nl->Second(value);
     if(::nl->AtomType(val)!=NoAtom)
        return false;
     if(nl->ListLength(val)==2) {
        if(::nl->AtomType(::nl->Second(val))==BoolType){ // undefined 
           return true;
        } else{
            return false;
        }
      }
     if(::nl->ListLength(val)!=3)
        return false;          
     if(::nl->AtomType(::nl->Second(val))!=NoAtom ||
        ::nl->AtomType(::nl->Third(val))!=NoAtom )
  return false;
     int L1 = ::nl->ListLength(::nl->Second(val));
     if(L1 != ::nl->ListLength(::nl->Third(val)))
        return false;         
     PtsSize += L1; // add the Points of this Linear Move              
     return true;
  }
  if(::nl->IsEqual(type,"composite")){
     CMSize = CMSize+1; // the composite move itself
     ListExpr rest = ::nl->Second(value);
     SMSize = SMSize+::nl->ListLength(rest); // the contained submoves
     while(!::nl->IsEmpty(rest)){
        if(!AddSubMovesSize(::nl->First(rest),LMSize,PtsSize,
                      CMSize,SMSize,PMSize))
           return false;
        rest = ::nl->Rest(rest);
     }
     return true;
  }
  if(::nl->IsEqual(type,"period")){
     PMSize = PMSize+1;
     int len = ::nl->ListLength(value);
     ListExpr PMove;
     if(len==2)
        PMove = ::nl->Second(value);
     else if(len==4)
        PMove = ::nl->Fourth(value);
     else // invalid listlength
        return false;
     return AddSubMovesSize(::nl->Second(PMove),LMSize,PtsSize,
                            CMSize,SMSize,PMSize);
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
bool PMPoints::AddLinearMove(const ListExpr value, 
                             int &LMIndex, int &PtsIndex,
                             int &CMIndex, int &SMIndex, 
                             int &PMIndex){
    __TRACE__
  LinearPointsMove LM = LinearPointsMove(0);
   if(!LM.ReadFrom(value,thePoints,PtsIndex))
      return false;
   linearMoves.Put(LMIndex,LM);
   LMIndex++;
   return true;
}



/*
~AddSpatialCompositeMove~

This Functions appends the composite move given as a nested list together with
all contained submoves at the appropriate arrays. The return value indiciates
the success of this call.

[3] O(L), where L is the number of contained linear moves

*/
bool PMPoints::AddSpatialCompositeMove(const ListExpr value,int &LMIndex,
                   int &PtsIndex,int &CMIndex, int &SMIndex, int &PMIndex){
    __TRACE__
  // a composite move has to contains at least two submoves
   int len = ::nl->ListLength(value);
   if(len<2){
      if(DEBUG_MODE){
         cerr << __POS__ << " less than 2 submoves (" << len << ")" << endl;
      }
      return false;
   }
   SpatialCompositeMove CM=SpatialCompositeMove(1);
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
         if(!AddLinearMove(VL,LMIndex,PtsIndex,CMIndex,SMIndex,PMIndex)){
            if(DEBUG_MODE){
               cerr << __POS__ << " can't add a linear move " << endl;
            }
            return false;
         }
         LinearPointsMove LM;
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
         CM.bbox.Union(&(LM.bbox));
         // put the submove in the array
         CSubMove SM;
         SM.arrayNumber = LINEAR;
         SM.arrayIndex = LMPos;
         compositeSubMoves.Put(SMPos,SM);
         SMPos++;
      } else if(::nl->IsEqual(TL,"period")){
        // process a periodic submove
        int PMPos = PMIndex;
        if(!AddPeriodMove(VL,LMIndex,PtsIndex,CMIndex,SMIndex,PMIndex)){
           if(DEBUG_MODE){
              cerr << __POS__ << "can't add period move " << endl;
            }
            return  false;
        }
        SpatialPeriodicMove PM;
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
         cerr << __POS__ << " submove not of type linear od period" << endl;
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
bool PMPoints::AddPeriodMove(const ListExpr value,int &LMIndex, int &PtsIndex,
                             int &CMIndex, int &SMIndex, int &PMIndex){
   __TRACE__
 int len = ::nl->ListLength(value);
 if((len!=2) && (len!=4)){  // (repeatations <submove>)
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
 if(len==2)
     SML = ::nl->Second(value);
 else
     SML = ::nl->Fourth(value);

 if(::nl->ListLength(SML)!=2){
     if(DEBUG_MODE){
        cerr << __POS__ << ": wrong length for submove" << endl;
     }
     return false;
 }
 SpatialPeriodicMove PM=SpatialPeriodicMove(1);
 PM.repeatations = rep;
 int IncludePos = PMIndex; // store the positiuon
 PMIndex++;
 ListExpr SMT = ::nl->First(SML); // take the submove type
 if(::nl->IsEqual(SMT,"linear")){
    int LMPos = LMIndex;
    if(!AddLinearMove(::nl->Second(SML),LMIndex,PtsIndex,
                      CMIndex,SMIndex,PMIndex)){
       if(DEBUG_MODE){
          cerr << __POS__ << ": can't add linear submove" << endl;
       }
       return false;
    }
    PM.submove.arrayNumber = LINEAR;
    PM.submove.arrayIndex = LMPos;
    LinearPointsMove LM;
    linearMoves.Get(LMPos,LM);
    PM.bbox.Equalize(&(LM.bbox));
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
    if(!AddSpatialCompositeMove(::nl->Second(SML),LMIndex,PtsIndex,
                                CMIndex,SMIndex,PMIndex)){
       if(DEBUG_MODE){
          cerr << __POS__ << ": can't add composite submove" << endl;
       }
       return false;
    }
    PM.submove.arrayNumber = COMPOSITE;
    PM.submove.arrayIndex = CMPos;
    SpatialCompositeMove CM;
    compositeMoves.Get(CMPos,CM);
    PM.bbox.Equalize(&(CM.bbox));
    RelInterval SMI = CM.interval;
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
~BreakPoints~

This function computes all locations where a point of this pointset has
halted.

*/
Points* PMPoints::Breakpoints()const{
    DateTime DT(durationtype);
    return Breakpoints(&DT,false);
}

void PMPoints::Breakpoints(Points& res) const{
    DateTime DT(durationtype);
    Breakpoints(&DT,false,res);
}

/*
~BreakPoints~

This function computes all locations where a point of this pointsset was
staying longer than duration. If the duration is less than zero or the 
duration is zero and inclusive is true, the result will be an undefined
points value.

*/
Points* PMPoints::Breakpoints(const DateTime* duration, 
                              const bool inclusive) const{
  Points* res = new Points(1);
  Breakpoints(duration,inclusive,*res);
  return  res;
}

void PMPoints::Breakpoints(const DateTime* duration,
                           const bool inclusive,
                           Points& res) const{
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
   LinearPointsMove LM;
   TwoPoints TP;
   res.StartBulkLoad();
   int size = linearMoves.Size();
   for(int i=0;i<size;i++){
      linearMoves.Get(i,LM);
      DateTime* L = LM.interval.GetLength();
      int cmp = L->CompareTo(duration);
      delete L;
      L = NULL;
      if(cmp>0 || (cmp==0 && inclusive)){
          unsigned int a = LM.GetStartIndex();
          unsigned int b = LM.GetEndIndex();
          for(unsigned int j = a; j<=b; j++){
              thePoints.Get(j,TP);
              if(TP.IsStatic()){
                Point P(true,TP.GetStartX(),TP.GetStartY());
                res += P;
              }
          }
      }
   }
   res.EndBulkLoad();
}

/*
~At~

This function computes the state of this pmpoints at time __instant__.
If this pmpoints is not defined at this instant, the result will be 
an undefined points object.

*/

void PMPoints::At(const DateTime* instant, Points& res)const{
    __TRACE__
    DateTime* duration = new DateTime(instanttype);
    duration->Equalize(instant);
    duration->Minus(&startTime); // now it is really a duration
    if(interval.Contains(duration)){
       const SubMove* sm;
       sm  = &submove;
       SpatialCompositeMove CM;
       SpatialPeriodicMove PM;
       RelInterval RI;
       // i have to find the linear move which is "active" at instant
       while(sm->arrayNumber!=LINEAR){
          if(sm->arrayNumber==COMPOSITE){
             // in this stage of implementation a linear search is
             // executed. i have to make it better in the future
             int i = sm->arrayIndex;
             compositeMoves.Get(i,CM);
             int cur = CM.minIndex;
             int max = CM.maxIndex;
             bool found=false;
             while( (cur<=max) && ! found){
                CSubMove csm;
                compositeSubMoves.Get(cur,csm); // get the submove
                SubMove sm1;
                sm1.arrayIndex = csm.arrayIndex;
                sm1.arrayNumber = csm.arrayNumber;
                sm = &sm1;
                
                if(sm->arrayNumber==LINEAR){
                   LinearPointsMove  LM;
                   linearMoves.Get(sm->arrayIndex,LM);
                   RI = LM.interval;
                } else if(sm->arrayNumber==PERIOD){
                   periodicMoves.Get(sm->arrayIndex,PM);
                   RI = PM.interval;
                } else { //another submoves are not allowed
                   assert(false);
                }
                if(RI.Contains(duration)) // be happy
                   found=true;
                else{  // search again
                   DateTime* L = RI.GetLength();
                   duration->Minus(L);
                   delete L;
                   L = NULL;
                   cur++;
                }
             }
             assert(found); //otherwise we have an error in computation
          } else if(sm->arrayNumber==PERIOD){
             int index = sm->arrayIndex;
             periodicMoves.Get(index,PM);
             // this is a very slow implementation
             // i have to speed up it in the future
             sm = &PM.submove;
             RelInterval RI;
             if(sm->arrayNumber==LINEAR){
                 LinearPointsMove LM;
                 linearMoves.Get(sm->arrayIndex,LM);
                 RI = LM.interval;
             } else if(sm->arrayNumber==COMPOSITE){
                 compositeMoves.Get(sm->arrayIndex,CM);
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
       LinearPointsMove LM;
       linearMoves.Get(sm->arrayIndex,LM);
       if(LM.IsDefinedAt(duration))
          LM.At(duration,thePoints,res);
       else
          res.SetDefined(false);
    } else { // the duration does not contains the argument instant
         res.SetDefined(false);
    }
    delete duration;
    duration = NULL;
}

PMPoints::PMPoints(){}

PMPoints::PMPoints(const PMPoints& source):
   Attribute(source.IsDefined()),
   linearMoves(source.linearMoves.Size()),
   thePoints(source.thePoints.Size()),
   compositeMoves(source.compositeMoves.Size()),
   compositeSubMoves(source.compositeMoves.Size()),
   periodicMoves(source.periodicMoves.Size()),
   canDelete(false),
   interval(source.interval),
   startTime(instanttype),
   bbox(source.bbox)
 {
   Equalize(&source);
}

PMPoints& PMPoints::operator=(const PMPoints& source){
   Equalize(&source);
   return *this;
}

void PMPoints::Initial(Points& res)const{
  if(!IsDefined()){
    res.SetDefined(false);
    return;
  }
  res.Clear();
  res.SetDefined(true);
  At(&startTime,res);
}

bool PMPoints::Final(Points& res)const{
   if(!IsDefined()){
      res.SetDefined(false);
      return false;
   }
   DateTime DT(startTime);
   DT.Add(interval.GetLength());
   At(&DT,res); 
   return  true;
}

void PMPoints::Translate(const DateTime& duration){
   startTime += duration;
}

void PMPoints::Translate(const DateTime* duration, PMPoints& res)const{
   res.Equalize(this);
   res.startTime += *duration;
}

/*
~CorrectDurationSums~

*/
void PMPoints::CorrectDurationSums(){
   if(IsEmpty() || !IsDefined()){  // nothing to do
      return;
   }
   int cmsize = compositeMoves.Size();
   RelInterval currentInterval;
   DateTime currentLength(durationtype);
   DateTime duration(durationtype);
   SpatialCompositeMove CM;
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

void PMPoints::GetLength(SubMove sm, DateTime& result){
   switch(sm.arrayIndex){
       case LINEAR:{
              LinearPointsMove lpm;
              linearMoves.Get(sm.arrayIndex,lpm);
              lpm.interval.GetLength(result);
              break;
            }
       case COMPOSITE: {
              SpatialCompositeMove scm;
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
       default: assert(false);
   }
}



/*
~Shift Operator~

*/
ostream& operator<< (ostream &os, class PMPoints &P){
   __TRACE__
 os << " <<<< PMPoints >>>>" << endl;
  if(!P.IsDefined()){
     os << "undefined" << endl;
     return os;
  }   
  os << "starttime: " << P.startTime.ToString() << endl;
  os << P.submove << endl;
  os << "defined :" << P.IsDefined() << endl;
  // the contents of the contained arrays
  // the linear moves
  os << "linear Moves " << P.linearMoves.Size() << endl;
  LinearPointsMove LM1;
  TwoPoints TP1;
  unsigned int thePointsSize = P.thePoints.Size();
  os << "SizeOf thePoints: " << thePointsSize << endl;
  for(int i=0;i<P.linearMoves.Size();i++){
     P.linearMoves.Get(i,LM1);
     LinearPointsMove LM2 = (LM1);
     os << LM2 << endl;
     os << "Content of this linear Move " << endl;
     for(int unsigned j=LM2.startIndex;j<LM2.endIndex;j++){
   if(j>=thePointsSize){
       os << "Array out of bounds " << __POS__ << endl;
       os << "Try to access element number " << j << "in an array ";
       os << "of size " << thePointsSize << endl;
   }else {
       os << j << " ." ;
       P.thePoints.Get(j,TP1);
       TwoPoints TP2 = (TP1);
       os << TP2 << endl;
       os << j << " ." ;
       P.thePoints.Get(j,TP1);
       TP2 = (TP1);
       os << TP2 << endl;
       os << j << " ." ;
       P.thePoints.Get(j,TP1);
       TP2 = (TP1);
       os << TP2 << endl;
  }     
     }
     os << "end content of this linear Move " << endl;
  }
 // os << " please extend the << operator for pmpoints values " << endl;
  os << " <<<< end PMPoints >>> " << endl; 
  return os;
}


} // end of namespace periodic
