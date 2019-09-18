
/*
3.16 ~PMSimple~ 

*/
#ifndef PMSIMPLE_H
#define PMSIMPLE_H

#include <iostream>
#include <string>

#include "NestedList.h"
#include "PeriodicSupport.h"
#include "PeriodicTypes.h"
#include "StandardTypes.h"
#include "DateTime.h"


extern NestedList* nl;

namespace periodic{
/*
~Constructor~

*/
template <class T, class Unit>
PMSimple<T, Unit>::PMSimple(){}


template <class T, class Unit>
PMSimple<T, Unit>::PMSimple(int dummy): 
   Attribute(false),
   linearMoves(1),
   compositeMoves(1),
   compositeSubMoves(1),
   periodicMoves(1),
   canDelete(false),
   interval(1),
   startTime(datetime::instanttype){
     __TRACE__
}

template <class T, class Unit>
PMSimple<T,Unit>::PMSimple(const PMSimple<T,Unit>& source):
 Attribute(source.IsDefined()){
  Equalize(&source);
} 

/*
~Destructor~

*/
template <class T, class Unit>
PMSimple<T, Unit>::~PMSimple() {
  __TRACE__
  if(canDelete){
     linearMoves.Destroy();
    compositeMoves.Destroy();
    compositeSubMoves.Destroy();
    periodicMoves.Destroy();
  }
}


/*
~Assignment operator~

*/
template <class T, class Unit>
PMSimple<T, Unit> PMSimple<T, Unit>::operator=(const PMSimple<T, Unit>& source){
    Equalize(&source);
    return *this;
}

/*
~Destroy~

If this function is called, the destructor can destroy the
FLOB objects.

[3] O(1)

*/
template <class T, class Unit>
void PMSimple<T,Unit>::Destroy(){
    __TRACE__
    canDelete = true;
}


/*
~Equalize~

This function takes the value for this PMSimple from another instance __B__.

[3] O(L), where L is the number of contained linear moves

*/
template <class T, class Unit>
void PMSimple<T, Unit>::Equalize(const PMSimple<T,Unit>* B2){
    __TRACE__
  linearMoves.copyFrom(B2->linearMoves);
  compositeMoves.copyFrom(B2->compositeMoves);
  compositeSubMoves.copyFrom(B2->compositeSubMoves);
  periodicMoves.copyFrom(B2->periodicMoves);

  SetDefined(B2->IsDefined());
  interval.Equalize(&(B2->interval));
  startTime.Equalize(&(B2->startTime));
  submove.Equalize(&(B2->submove));
}


/*
~NumOfFLOBs~

Returns four because four DbArrays are managed.

[3] O(1)

*/
template <class T, class Unit>
int PMSimple<T, Unit>::NumOfFLOBs() const{
  __TRACE__
  return 4;
}


/*
~GetFLOB~

This function returns the FLOB at the specified index. If the index
is out of range, the system will going down. The flobs are:

  * 0: the units

  * 1: the composite moves

  * 2: the composite submoves

  * 3: the periodic moves

[3] O(1)

*/
template <class T, class Unit>
Flob* PMSimple<T,Unit>::GetFLOB(const int i){
  __TRACE__
  assert(i>=0 && i<NumOfFLOBs());
 if(i==0) return &linearMoves;
 if(i==1) return &compositeMoves;
 if(i==2) return &compositeSubMoves;
 if(i==3) return &periodicMoves;
 return 0;
}


/*
~Compare~

Because we have no unique representation of the periodic moves, this function is
not implemented in this moment.

[3] O(-1)

*/
template <class T, class Unit>
int PMSimple<T, Unit>::Compare(const Attribute* arg) const{
    __TRACE__
  cout << "PMSImple::Compare not implemented yet " << std::endl;
  return -1;
}


/*
~Adjacent~

Returns false because it's not clear when two moving constants should be
adjacent.

[3] O(1)

*/
template <class T, class Unit>
bool PMSimple<T, Unit>::Adjacent(const Attribute*)const{
    __TRACE__
   return false;
}

/*
~Clone~

This functions creates a new periodic moving constant with the same value
like the calling instance and returns it.

[3] O(L), where L is the number of contained linear moves

*/
template <class T, class Unit>
PMSimple<T,Unit>* PMSimple<T, Unit>::Clone() const{
    __TRACE__
  PMSimple<T,Unit>* res = new PMSimple<T,Unit>(1);
  res->Equalize(this);
  return res;
}


/*
~SizeOf~

Returns the size of the class PMSimple.

[3] O(1)

*/
template <class T, class Unit>
size_t PMSimple<T, Unit>::Sizeof()const{
    __TRACE__
   return sizeof(PMSimple<T,Unit>);
}


/*
~HashValue~

This functions computes a HashValue for this PMSimple from the sizes of
the contained FLOBs.

[3] O(1)

*/
template <class T, class Unit>
size_t PMSimple<T, Unit>::HashValue() const{
    __TRACE__
     return (size_t) (interval.HashValue()+linearMoves.Size()+
                   compositeMoves.Size()+ periodicMoves.Size()+
                   compositeSubMoves.Size());
}


/*
~CopyFrom~

By calling this function the value of this instance will be
equalized with this one of the attribute. The attribute must be
of Type PMSimple. If __arg__ is of another type, the result is
not determined and can lead to a crash of [secondo].

[3] O(L), where L is the number of contained linear moves

*/
template <class T, class Unit>
void PMSimple<T,Unit>::CopyFrom(const Attribute* arg){
    __TRACE__
   Equalize((PMSimple<T,Unit>*) arg);
}
     
/*
~ToListExpr~

This function converts a periodic moving constant into its
nested list representation.

[3] O(L), where L is the number of contained linear moves

*/
template <class T, class Unit>
ListExpr PMSimple<T, Unit>::ToListExpr(const ListExpr typeInfo)const {
  __TRACE__
 ListExpr timelist = startTime.ToListExpr(false);
 ListExpr SubMoveList = GetSubMoveList(submove);
     return ::nl->TwoElemList(timelist,SubMoveList);
}

/*
~ReadFrom~

Takes the value of this simple periodic moving object from a
nested list. The result informs about the success of this
call. If the List don't represent a valid periodic moving
object, this instance will be undefined. This behavior 
differns to other ~readFrom~ functions to avoid some
expensive tests before creating the actual object.

[3] O(L), where L is the number of contained linear moves

*/
template <class T, class Unit>
bool PMSimple<T, Unit>::ReadFrom(const ListExpr value,
                                 const ListExpr typeInfo){
    __TRACE__
   /* The list is scanned twice. In the first scan we
     compute only the needed size of the contained arrays.
     The reason is, that we want to avoid frequently ~Resize~
     on the arrays.
   */
  if(::nl->ListLength(value)!=2){
     if(DEBUG_MODE){
        std::cerr << __POS__ << ": wrong listlength (";
        std::cerr << (::nl->ListLength(value)) << ")" << std::endl;
     }
     SetDefined(false);
     return false;
  }

  if(!ResizeArrays(value)){
     if(DEBUG_MODE){
        std::cerr << __POS__ << ": resize array failed" << std::endl;
     }
     SetDefined(false);
     return false;
  }

  if(!startTime.ReadFrom(::nl->First(value),false)){
     if(DEBUG_MODE){
        std::cerr << __POS__ << "reading of the start time failed" << std::endl;
        std::cerr << "The list is " << std::endl;
        ::nl->WriteListExpr(::nl->First(value));
     }
     SetDefined(false);
     return false;
  }

  // now we have to append the included submove
  ListExpr SML = ::nl->Second(value);
  if(::nl->ListLength(SML)!=2){
     if(DEBUG_MODE){
        std::cerr << __POS__ << ": wrong list length for submove" << std::endl;
     }
     SetDefined(false);
     return false;
  }

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
            std::cerr << __POS__ << " Error in reading linear move" 
                      << std::endl;
         }
         SetDefined(false);
         return false;
     }
     SetDefined(true);

     Unit LM;
     linearMoves.Get(0,LM);
     interval.Equalize(&(LM.interval));
     CorrectDurationSums();
     return true;
  }
  if(::nl->IsEqual(SMT,"composite")){
     submove.arrayNumber=COMPOSITE;
     submove.arrayIndex = 0;
     if(!AddCompositeMove(::nl->Second(SML),LMIndex,
                          CMIndex,SMIndex,PMIndex)){
        if(DEBUG_MODE){
           std::cerr << __POS__ << "error in reading composite move" 
                     << std::endl;
        }
        SetDefined(false);
        return false;
     }
     SetDefined(true);
     CompositeMove CM;
     compositeMoves.Get(0,CM);
     interval.Equalize(&(CM.interval));
     CorrectDurationSums();
     return true;
  }
  if(::nl->IsEqual(SMT,"period")){
     submove.arrayNumber = PERIOD;
     submove.arrayIndex = 0;
     if(!AddPeriodMove(::nl->Second(SML),LMIndex,CMIndex,SMIndex,PMIndex)){
        if(DEBUG_MODE){
          std::cerr << __POS__ << " error in reading periodic move" 
                    << std::endl;
        }
        SetDefined(false);
        return false;
     }
     SetDefined(true);
     PeriodicMove PM;
     periodicMoves.Get(0,PM);
     interval.Equalize(&(PM.interval));
     CorrectDurationSums();
     return true;
  }
  if(DEBUG_MODE){
     std::cerr << __POS__ << "unknown subtype" << std::endl;
     ::nl->WriteListExpr(SMT);
  }
  return false;
}

/*
~IsEmpty~

This function yields __true__ if this periodic moving constant don't 
contain any unit.

*/
template <class T, class Unit>
bool PMSimple<T, Unit>::IsEmpty()const{ 
     return linearMoves.Size()==0;
}


/*
~At~

This function returns a pointer to a new created instance of type 
T. If this periodic moving constant is not defined at this
timepoint, the result will be 0. The caller of this function has to
delete the returned value of this function if the result is not null.

[3] O(L), where L is the number of contained linear moves

*/
template <class T, class Unit>
void PMSimple<T, Unit>::At(const datetime::DateTime* instant,T& res)const{
  __TRACE__
 datetime::DateTime* duration = new datetime::DateTime(*instant);
 duration->Minus(&startTime); // now it is a duration
 if(interval.Contains(duration)){
 // in the other case, we have nothing to do
 // because the result is initialized with "undefined"
 SubMove sm(submove);
 CompositeMove CM;
 PeriodicMove PM;
 RelInterval RI;
 while(sm.arrayNumber!=LINEAR){
    if(sm.arrayNumber==COMPOSITE){
       // in this stage of implementation a linear search is
       // executed. i have to make it better in the future
       int i = sm.arrayIndex;
       compositeMoves.Get(i,CM);
       int cur = CM.minIndex;
       int max = CM.maxIndex;
       bool found=false;
       while( (cur<=max) && ! found){
           CSubMove csm;
           compositeSubMoves.Get(cur,csm); // get the submove
           if(csm.arrayNumber==LINEAR){
              Unit LM;
              linearMoves.Get(csm.arrayIndex,LM);
              RI = LM.interval;
            } else if(csm.arrayNumber==PERIOD){
              periodicMoves.Get(csm.arrayIndex,PM);
              RI = PM.interval;
            } else { //another submoves are not allowed
              assert(false);
            }
            if(RI.Contains(duration)) // be happy
               found=true;
            else{  // search again
               datetime::DateTime* L = RI.GetLength();
               duration->Minus(L);
               delete L;
               L = NULL;
               cur++;
            }
       }
       assert(found); //otherwise we have an error in computation
    } else if(sm.arrayNumber==PERIOD){
       int index = sm.arrayIndex;
       periodicMoves.Get(index,PM);
       // this is a very slow implementation
       // i have to speed up it in the future
       sm = PM.submove;
       RelInterval RI;
       if(sm.arrayNumber==LINEAR){
           Unit LM;
           linearMoves.Get(sm.arrayIndex,LM);
           RI = LM.interval;
        } else if(sm.arrayNumber==COMPOSITE){
           compositeMoves.Get(sm.arrayIndex,CM);
           RI = CM.interval;
        } else { //another submoves are not allowed
           assert(false);
        }
        while(!RI.Contains(duration)){
           datetime::DateTime* L = RI.GetLength();
           duration->Minus(L);
           delete L;
           L = NULL;
        }
        } else{
           // this case should never occurs
           assert(false);
        }
     }
     Unit LM;
     linearMoves.Get(sm.arrayIndex,LM);
     if(LM.IsDefinedAt(duration)){
        LM.At(duration,res);
     }
  }
  delete duration;
  duration = NULL;
}

/*
~Initial~

This function computes the first defined value of this moving constant.

*/
template <class T, class Unit>
bool PMSimple<T, Unit>::Initial(T& result) const{
    __TRACE__
  if(!IsDefined() || IsEmpty()){
    return false;
  }
  Unit lm;
  linearMoves.Get(0,lm);
  return lm.Initial(result); 
}


/*
~Final~

The ~Final~ function returns the last defined value of this periodic
moving object.
If none exists, NULL is returned.

*/
template <class T, class Unit>
bool PMSimple<T, Unit>::Final(T& res)const{
    __TRACE__
 if(!IsDefined()){
    return false;
 }
 if(IsEmpty()){
     return false;
  }       
  Unit lm =  GetLastUnit();
  return lm.Final(res);
}

/*
~Translation~

Moves this simple time within the time.

*/
template<class T, class Unit>
void PMSimple<T, Unit>::Translate(const datetime::DateTime& duration){
   startTime += duration;
}

template<class T, class Unit>
void PMSimple<T, Unit>::Translate(const datetime::DateTime* duration,
                                  PMSimple<T,Unit>& res)const{
   res.Equalize(this);
   res.startTime += *duration;
}
 
/*
~Minimize~

When this function is called, consecutive units with the same value are
summarized into single units.

*/
template <class T, class Unit>
void PMSimple<T, Unit>::Minimize(){
   if(MinimizationRequired()){
     DbArray<Unit> newLinearMoves(1);
     DbArray<CompositeMove>          newCompositeMoves(1);
     DbArray<CSubMove>                newCompositeSubMoves(1);
     DbArray<PeriodicMove>           newPeriodicMoves(1);
     Unit                            Summarization;
     bool                            CompleteSummarized;
     SubMove SM3 = MinimizeRec(submove,newLinearMoves,newCompositeMoves,
                       newCompositeSubMoves,
                       newPeriodicMoves,Summarization,CompleteSummarized);
     if(CompleteSummarized){
        // the whole movement can be represented in a single linear move
        compositeMoves.clean();
        compositeSubMoves.clean();   
        periodicMoves.clean();
        linearMoves.clean();
        linearMoves.Append(Summarization);
        submove.arrayNumber = LINEAR;
        submove.arrayIndex = 0;
     } else{
       submove = SM3;
       
     // copy the new created objects into the local dbarrays
       linearMoves.copyFrom(newLinearMoves);      
       compositeMoves.copyFrom(newCompositeMoves);
       compositeSubMoves.copyFrom(newCompositeSubMoves);
       periodicMoves.copyFrom(newPeriodicMoves);      
     } 
   }
   CorrectDurationSums();
}


/*
~Statistical Information~ 

The following functions can be used in debugging for get some 
additional information about the inner structure of this instant.

*/
template <class T, class Unit>
int PMSimple<T, Unit>::NumberOfLinearMoves(){
   return linearMoves.Size();
}

template <class T, class Unit>
int PMSimple<T, Unit>::NumberOfCompositeMoves(){
   return compositeMoves.Size();
}

template <class T, class Unit>
int PMSimple<T, Unit>::NumberOfCompositeSubMoves(){
   return compositeSubMoves.Size();
}

template <class T, class Unit>
int PMSimple<T,Unit>::NumberOfPeriodicMoves(){
   return periodicMoves.Size();
}


/*
~Split~

Calling this function splits this moving object at the given instant. 
The splitpoint is given by an instant as well as a flag describing whether 
this instant should be contained in the 'left' part. The results are stored
into the corresponding arguments. When the instant is outside the definition
time of this moving object, oe part will contain a copy of this, the other
part will be undefined.

*/
template <class T, class Unit>
void PMSimple<T, Unit>::Split(const datetime::DateTime instant,
           const bool toLeft, 
           PMSimple<T,Unit>& leftPart, PMSimple<T,Unit>& rightPart){
   SubMove SMLeft,SMRight;
   SMLeft.arrayNumber = -1;
   SMRight.arrayNumber = -1;
   datetime::DateTime stCopy = startTime;
   splitRec(instant, toLeft,leftPart,rightPart,submove,stCopy,SMLeft,SMRight);
   // set the values of the root record of the left part
   if(SMLeft.arrayNumber<0){ // leftpart is empty
        leftPart.defined=false;  
   } else{ // leftpart is non-empty
        leftPart.defined = true;
        leftPart.startTime = startTime;
        leftPart.GetInterval(SMLeft,leftPart.interval);
   }
   // set the values of the root record of the right part
   if(SMRight.arrayNumber<0){
        rightPart.defined=false;
   }else{
        rightPart.defined=true;
        rightPart.startTime = instant;
        rightPart.GetInterval(SMRight,rightPart.interval);
   }
   std::cerr << "Missing correction for splitted infinite periodic  moves !!!"
        << std::endl;
}


/*
~SpliRec~

This function splits the given submove of this periodic moving objects into 
two part. The leftPart will be extended by the part of this submove before
the given instant. The remainder of this submove will stored into rightPart.
The new added submove of leftPart and rightPart is returned in the arguments
SMLeft and SMRight respectively. The argument toLeft controls whereto put
in the object at the given instant (closure properties). The starttime is 
changed to be after the processed submove. If the submove is not defined 
before the given instant, leftPart is not changed and SMLeft will hold an 
negative number for the arrayNumber indicating an invalid value. When submove
is'nt defined after this instant, the same holds for rightPart and SMRight.

*/
template <class T, class Unit>
void PMSimple<T, Unit>::splitRec(const datetime::DateTime instant, 
              const bool toLeft,
              PMSimple<T,Unit>& leftPart, PMSimple<T,Unit>& rightPart,
              SubMove submove, datetime::DateTime& startTime, 
              SubMove& SMLeft, SubMove& SMRight){
 

  if(submove.arrayNumber==LINEAR){
    // here we process a single unit. this case is easy to handle because a
    // unit have to provide a split function
    Unit LML;
    linearMoves.Get(submove.arrayIndex,LML);
    Unit LMR;
    datetime::DateTime Dur = instant - startTime;
    startTime = startTime + (*LML.interval.GetLength()); 
    bool saveLeft,saveRight;
    if(LML.GetDefined()){
       LML.Split(Dur,toLeft,LMR);
       saveLeft = LML.GetDefined();
       saveRight = LMR.GetDefined();
     } else{ // original is undefined
        LML.interval.Split(Dur,toLeft,LMR.interval);
        LMR.SetDefined(false);
        saveLeft = LML.interval.IsDefined();
        saveRight = LMR.interval.IsDefined();
     }
     if(saveLeft){
         SMLeft.arrayNumber=LINEAR;
         SMLeft.arrayIndex = leftPart.linearMoves.Size();
         leftPart.linearMoves.Append(LML);
     }else{
         SMLeft.arrayNumber = -1;
     }
     // do the same thing with the right part
     if(saveRight){
        SMRight.arrayNumber=LINEAR;
        SMRight.arrayIndex=rightPart.linearMoves.Size();
        rightPart.linearMoves.Append(LML);
     } else{
        SMRight.arrayNumber = -1;
     }
     return;  // linear moves processed
  }
  if(submove.arrayNumber==COMPOSITE){
     // we store the submoves of this composite moves into an vector 
     // to ensure a single block representing this submove
     CompositeMove CM;
     compositeMoves.Get(submove.arrayIndex,CM);
     std::vector<CSubMove> submovesLeft(CM.maxIndex-CM.minIndex+1);
     std::vector<CSubMove> submovesRight(CM.maxIndex-CM.minIndex+1);
     
     CSubMove SM;
     for(int i=CM.minIndex;i<=CM.maxIndex;i++){
        compositeSubMoves.Get(i,SM); // get the submove
        // call recursive
        splitRec(instant,toLeft,leftPart,rightPart,SM,startTime,SMLeft,SMRight);
        // collect valid submoves
       CSubMove CSMLeft;
       CSMLeft.arrayNumber = SMLeft.arrayNumber;
       CSMLeft.arrayIndex = SMLeft.arrayIndex; 
       CSubMove CSMRight;
       CSMRight.arrayNumber = SMRight.arrayNumber;
       CSMRight.arrayIndex = SMRight.arrayIndex;

       if(SMLeft.arrayNumber>=0)
           submovesLeft.push_back(CSMLeft);
       if(SMRight.arrayNumber>=0)
           submovesRight.push_back(CSMRight);
     }
     // process the left part
     int size = submovesLeft.size();
     if(size==0){
        SMLeft.arrayNumber = -1;
     } else if (size>1){ // in case size==1 is nothing to do
        CompositeMove CMLeft;
        CMLeft.minIndex = leftPart.compositeSubMoves.Size();
        for(int i=0;i<size;i++){
            leftPart.compositeSubMoves.Append(submovesLeft[i]); 
        }
        CMLeft.maxIndex = leftPart.compositeSubMoves.Size()-1;
        SMLeft.arrayNumber = COMPOSITE;
        SMLeft.arrayIndex = leftPart.compositeMoves.Size();
        leftPart.compositeMoves.Append(CMLeft); 
     }
     // process the right part
     size = submovesRight.size();
     if(size==0){ // nothing remains at the right
        SMRight.arrayNumber = -1;
     } else if(size>1){
         CompositeMove CMRight;
         CMRight.minIndex = rightPart.compositeSubMoves.Size();
         for(int i=0;i<size;i++){
             rightPart.compositeSubMoves.Append(submovesRight[i]);
         }
         CMRight.maxIndex = rightPart.compositeSubMoves.Size()-1;
         SMRight.arrayNumber = COMPOSITE;
         SMRight.arrayIndex = rightPart.compositeMoves.Size();
         rightPart.compositeMoves.Append(CMRight);
     }
     return; // composite moves processed
  }
  if(submove.arrayNumber==PERIOD){
     // first, we check whether the split instant is outside the interval
     PeriodicMove PM;
     periodicMoves.Get(submove.arrayIndex,PM);
     RelInterval sminterval;
     GetInterval(PM.submove,sminterval);
     datetime::DateTime smlength(datetime::durationtype);
     sminterval.StoreLength(smlength);
     datetime::DateTime ZeroTime(datetime::instanttype);
     sminterval.StoreLength(smlength);
     // find out how many repeatations left on both sides
     // first compute the duration which should be transferred 
     //into the left part
     datetime::DateTime LeftDuration = instant - startTime;
     return; // periodic moves processed
  }
  assert(false); // unknown submove
}


/*
~SplitLeft~

This function equalizes leftPart with the part of this move which is located
before the instant.

*/
template <class T, class Unit>
void PMSimple<T, Unit>::SplitLeft(const datetime::DateTime& instant,
               const bool toLeft, 
               PMSimple<T,Unit>* result){
  SubMove LastMove;
  datetime::DateTime startTimecopy(datetime::instanttype);
  startTimecopy.Equalize(&startTime);

  if(!SplitLeftRec(instant,toLeft,*result,submove,startTimecopy,LastMove))
      result->SetDefined(false);
  else{
     result->startTime = startTime;
     datetime::DateTime len = instant-startTime;
     result->interval.Set(&len,interval.IsLeftClosed(),toLeft);
     result->submove = LastMove;      
  }  
}

template <class T , class Unit>
bool PMSimple<T, Unit>::SplitLeftRec(const datetime::DateTime& instant,
                  const bool toLeft,
                  PMSimple<T,Unit>& result,const SubMove& submove, 
                  datetime::DateTime& startTime,SubMove& lastSubmove){

/*
First, we handle the a single unit.

*/
   if(submove.arrayNumber==LINEAR){
        if(startTime > instant) // is right of the instant
            return false;
        Unit u;
        linearMoves.Get(submove.arrayIndex,u);
        RelInterval interval;
        interval.Equalize(&u.interval);
        datetime::DateTime dur = instant-startTime;
        if(!u.interval.Contains(&dur)){
           return false;
        }
        Unit rightUnit;
        u.Split(dur,toLeft,rightUnit); 
        result.linearMoves.Append(u);
        datetime::DateTime len(datetime::durationtype);
        interval.StoreLength(len);
        startTime = startTime + len;
        lastSubmove.arrayNumber=LINEAR;
        lastSubmove.arrayIndex = result.linearMoves.Size()-1;
        return true;   
   }
/*
Second, handling of composite moves

*/
   if(submove.arrayNumber == COMPOSITE){
      CompositeMove CM;
      compositeMoves.Get(submove.arrayIndex,CM);
      datetime::DateTime dur = instant-startTime;
      if(!CM.interval.Contains(&dur))
          return false;        
      CSubMove SM;
      std::vector<SubMove> mySubmoves(CM.maxIndex-CM.minIndex);
      bool done = false;
      for(int i =CM.minIndex;i<=CM.maxIndex && !done;i++){
           compositeSubMoves.Get(i,SM);
           if(SplitLeftRec(instant,toLeft,result,SM,startTime,lastSubmove)){
                mySubmoves.push_back(lastSubmove);
           } else { // after this move can't follow any further moves
             done=true;
           }
      } 
      if(mySubmoves.size()<2){ // ok, this submoves is end in smoke
        return true;
      }else{ // we have to build a composite move from the remaining submoves
         datetime::DateTime length(datetime::durationtype);
         datetime::DateTime nextLength(datetime::durationtype);
         int size = mySubmoves.size();
         RelInterval interval; 
         CM.minIndex = result.compositeSubMoves.Size();
         for(int i=0;i<size;i++){
            SubMove simple = mySubmoves[i];
            SM.arrayIndex = simple.arrayIndex;
            SM.arrayNumber = simple.arrayNumber;
            if(i==0)
                result.GetInterval(SM,CM.interval);
            else{
                result.GetInterval(SM,interval);
                CM.interval.Append(&interval);
            }
            result.compositeSubMoves.Append(SM);
         }      
         CM.maxIndex = result.compositeSubMoves.Size()-1;
         // append the composite move
         lastSubmove.arrayIndex=COMPOSITE;
         lastSubmove.arrayIndex = result.compositeMoves.Size();
         result.compositeMoves.Append(CM);
         return true;
      }
   }

   if(submove.arrayIndex==PERIOD){
       PeriodicMove PM;
       periodicMoves.Get(submove.arrayIndex,PM);
       datetime::DateTime dur = instant-startTime;
       
   }
   assert(false); // the program should never reach this position


}



/*
~Intersection~

This function reduces the definition time of this PMSimple to the 
interval defined by the arguments. This function cannot be constant, i.e. 
it changes the *this* argument,  because
we have to scan the dbarrays in some cases.

*/
template <class T, class Unit>
void  PMSimple<T, Unit>::Intersection( const datetime::DateTime minTime, 
                     const bool minIncluded, 
                     const datetime::DateTime maxTime, 
                     const bool maxIncluded,
                    PMSimple<T,Unit>* res){

      PMSimple<T,Unit> TMPPM1;
      PMSimple<T,Unit> TMPPM2;
      Split(minTime,!minIncluded,TMPPM2,TMPPM1);
      //TMPPM1.Split(maxTime,maxIncluded,TMPPM1,*result);
      TMPPM1.SplitLeft(maxTime,maxIncluded,res);
   }


/*
~CopyValuesFrom~

By calling this function, the value of this pmsimple is set to the given values.
There is no check whether the values are valid. Use this function very 
carefully.
A possible application of this function is to take the tree from another 
periodic moving object.

*/
template <class T, class Unit>
void PMSimple<T, Unit>::CopyValuesFrom( DbArray<Unit>& linearMoves,
                      DbArray<CompositeMove>& compositeMoves,
                      DbArray<SubMove>& compositeSubMoves,
                      DbArray<PeriodicMove>& periodicMoves,
                      bool defined,
                      RelInterval interval,
                      datetime::DateTime startTime,
                      SubMove submove){

  // first, clear all contained arraysa
  this->linearMoves.copyFrom(linearMoves);
  this->compositeMoves.copyFrom(compositeMoves);
  this->compositeSubMoves.copyFrom(compositeSubMoves);
  this->periodicMoves.copyFrom(periodicMoves);
  this.defined=defined;
  this.canDelete=false;
  this->interval.Equalize(&interval);
  this->startTime.Equalize(&startTime);
  this.submove=submove; 
} 

/*
~TakeValuesFrom~

This function works similar to the ~CopyValuesFrom~ function.
The difference is, that the values are assigned to the internal
variables. These variables are cleaned before.

*Note*: There is no check for the integrity of the given data. 

*/
template <class T, class Unit>
void PMSimple<T, Unit>::TakeValuesFrom( DbArray<Unit>& linearMoves,
                      DbArray<CompositeMove>& compositeMoves,
                      DbArray<SubMove>& compositeSubMoves,
                      DbArray<PeriodicMove>& periodicMoves,
                      bool defined,
                      RelInterval interval,
                      datetime::DateTime startTime,
                      SubMove submove){

   this->linearMoves.Clear(); 
   this->linearMoves.Destroy();
   this->compositeMoves.clean();
   this->compositeMoves.Destroy();
   this->compositeSubMoves.clean();
   this->compositeSubMoves.Destroy();
   this->periodicMoves.clean();
   this->periodicMoves.Destroy();
   this->linearMoves = linearMoves;
   this->compositeMoves = compositeMoves;
   this->compositeSubMoves = compositeSubMoves;
   this->periodicMoves = periodicMoves;
   this->defined = defined;
   this.interval = interval;
   this.startTimes = startTime;
   this.submove = submove;
}


/*
~SetStartTime~

This function can be used for moving this PMsimple in time.

*/
template <class T, class Unit>
void PMSimple<T, Unit>::SetStartTime(datetime::DateTime newStart){
      startTime.Equalize(&newStart);
}


/*
~Functions returning pointers to the members~

Each of the next functions returns a pointer for a member.
This way bypasses the protected declaration of the members 
because this function enables the possibility to manipulate the
members direcly without control of the PMSimple class. This may 
be not good for encapsulating the code but the only way to have
an efficient implementation for that. The caller of this functions
must be very very carefully to ensure that all the manipulated
members results in a consistent instance of type PMSimple. 
A possible application of this function is to make a copy of the
tree structure of another periodic moving object. 

*/
template <class T, class Unit>
DbArray<Unit>* PMSimple<T, Unit>::GetLinearMoves(){
   return &linearMoves;
}

template <class T, class Unit>
DbArray<CompositeMove>* PMSimple<T,Unit>::GetCompositeMoves(){
  return &compositeMoves;
}

template <class T, class Unit>
DbArray<CSubMove>* PMSimple<T, Unit>::GetCompositeSubMoves(){
   return &compositeSubMoves;
}

template <class T, class Unit>
DbArray<PeriodicMove>* PMSimple<T, Unit>::GetPeriodicMoves(){
   return &periodicMoves;
}

template <class T, class Unit>
RelInterval* PMSimple<T, Unit>::GetInterval(){
   return &interval;
}

template <class T, class Unit>
SubMove* PMSimple<T, Unit>::GetSubmove(){ 
   return &submove;
}

/*

~GetLength~

This function returns the lenght of the interval of the given 
submove. 

*/
template <class T, class Unit>
void PMSimple<T, Unit>::GetLength(SubMove sm, datetime::DateTime& result){
    switch(sm.arrayNumber){
      case LINEAR: { Unit U;
                     linearMoves.Get(sm.arrayIndex,U);
                      U.interval.StoreLength(result);
                   } break;
      case COMPOSITE: {
                       CompositeMove CM;
                       compositeMoves.Get(sm.arrayIndex,CM); 
                       CM.interval.StoreLength(result); 
                      }break;

     case PERIOD: {
                   PeriodicMove PM;
                   periodicMoves.Get(sm.arrayIndex,PM);
                   PM.interval.StoreLength(result);
                  } break;
     default: assert(false); // unknown move type

    }

}


/*

~GetLeftClosed~

This function returns the state of the LeftClosed flag  of the interval 
of the given submove. 

*/
template <class T, class Unit>
bool PMSimple<T, Unit>::GetLeftClosed(SubMove sm){
    switch(sm.arrayNumber){
      case LINEAR: { Unit U;
                     linearMoves.Get(sm.arrayIndex,U);
                      return  U.interval.IsLeftClosed();
                   } 
      case COMPOSITE: {
                       CompositeMove CM; 
                       return CM.interval.IsLeftClosed(); 
                      }

     case PERIOD: {
                   PeriodicMove PM;
                   return PM.interval.IsLeftClosed();
                  }
     default: assert(false); // unknown move type

    }
}


/*

~GetRightClosed~

This function returns the state of the RightClosed flag  of the interval 
of the given submove. 

*/
template <class T, class Unit>
bool PMSimple<T, Unit>::GetRightClosed(SubMove sm){
    switch(sm.arrayNumber){
      case LINEAR: { Unit U;
                     linearMoves.Get(sm.arrayIndex,U);
                      return  U.interval.IsRightClosed();
                   } 
      case COMPOSITE: {
                       CompositeMove CM; 
                       return CM.interval.IsRightClosed(); 
                      }

     case PERIOD: {
                   PeriodicMove PM;
                   return PM.interval.IsRightClosed();
                  }
     default: assert(false); // unknown move type
    }
 }


/*

~GetInterval~

This function returns the returns the interval of the given submove independly
of the type of the submove. 

*/
template <class T, class Unit>
void PMSimple<T, Unit>::GetInterval(SubMove sm,RelInterval& interval){
    switch(sm.arrayNumber){
      case LINEAR: { Unit U;
                     linearMoves.Get(sm.arrayIndex,U);
                     interval.Equalize(&U.interval);
                   } break;
      case COMPOSITE: {
                       CompositeMove CM;
                       compositeMoves.Get(sm.arrayIndex,CM); 
                       interval.Equalize(&CM.interval); 
                      }break;

     case PERIOD: {
                   PeriodicMove PM;
                   periodicMoves.Get(sm.arrayIndex,PM);
                   interval.Equalize(&PM.interval);
                  }break;
     default: assert(false); // unknown move type
    }
 }

/*
~GetSubMoveList~

This function returns the list representation for the submove in the
argument.

[3] O(L), where L is the number of contained linear moves

*/
template <class T, class Unit>
ListExpr PMSimple<T, Unit>::GetSubMoveList(const SubMove SM) const{
  __TRACE__
  ListExpr SubMoveList;
  int SubMoveType = SM.arrayNumber;
  int index = SM.arrayIndex;
  if(SubMoveType==LINEAR)
      SubMoveList = GetLinearMoveList(index);
  else if(SubMoveType==COMPOSITE)
      SubMoveList = GetCompositeMoveList(index);
  else if(SubMoveType==PERIOD)
      SubMoveList = GetPeriodicMoveList(index);
  else{
       std::cerr << __POS__ << " Error in creating ListExpr" << std::endl;
       SubMoveList = ::nl->TheEmptyList();
   }
  return SubMoveList;
}


/*
~GetLinearMoveList~

This functions takes the linear move at the specified index and
returns its nested list representation.

[3] O(1)

*/
template <class T, class Unit>
ListExpr PMSimple<T, Unit>::GetLinearMoveList(const int index)const{
    __TRACE__
   Unit LM;
   linearMoves.Get(index,LM);
   return LM.ToListExpr();
}


/*
~GetPeriodicMoveList~

Creates a nested list for the periodic move at the specified index.

[3] O(L), where L is the number of contained linear moves

*/
template <class T, class Unit>
ListExpr PMSimple<T, Unit>::GetPeriodicMoveList(const int index)const{
    __TRACE__
  PeriodicMove PM;
  periodicMoves.Get(index,PM);
  ListExpr periodtype = ::nl->SymbolAtom("period");
  ListExpr RepList = ::nl->IntAtom(PM.repeatations);
  ListExpr SML = GetSubMoveList(PM.submove);
  ListExpr RC = ::nl->BoolAtom(interval.IsRightClosed());
  ListExpr LC = ::nl->BoolAtom(interval.IsLeftClosed());
  return  ::nl->TwoElemList(periodtype,::nl->FourElemList(RepList,LC,RC,SML));
}


/*
~GetCompositeMoveList~

Returns the CompositeMove at the specified index in its nested list
representation.

[3] O(L), where L is the number of contained linear moves

*/
template <class T, class Unit>
ListExpr PMSimple<T, Unit>::GetCompositeMoveList(const int index)const{
   __TRACE__
  CompositeMove CM;
  compositeMoves.Get(index,CM);
  ListExpr CType = ::nl->SymbolAtom("composite");
  int minIndex = CM.minIndex;
  int maxIndex = CM.maxIndex;
  ListExpr SubMovesList;
  if(maxIndex<minIndex){
    std::cerr << __POS__ << "empty composite move" << std::endl;
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
~ResizeArrays~

This functions resizes all array to be able to insert the moves
contained in the argument list.This function should be called 
before this instance is read from a nested list to avoid 
frequently resize of the contained DB-Arrays. The return
value of this function is an indicator for the correctness 
of the structure of this list. If the result is __false__,
the list don't represent a valid periodic moving object.
But a return value of __true__ don't guarantees a correct
list representation. 

[3] O(L), where L is the number of contained linear moves

*/
template <class T, class Unit>
bool PMSimple<T, Unit>::ResizeArrays(const ListExpr value){
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

Adds the number of in value contained submoves to the appropriate
size. If the result is__false__, the list is not a correct 
representation of a periodic moving object. If it is __true__
the content of the list can be wrong but the structure is correct.

[3] O(L), where L is the number of contained linear moves

*/
template <class T, class Unit>
bool PMSimple<T, Unit>::AddSubMovesSize(const ListExpr value,
                                        int &LMSize,int &CMSize,
                                        int &SMSize,int &PMSize){
    __TRACE__
 // all moves have the length 2
  if(::nl->ListLength(value)!=2)
     return false;
  ListExpr type = ::nl->First(value);
  if(::nl->AtomType(type)!=SymbolType)
    return false;
  // in a linear move we have only to increment the size of LM
  if(::nl->IsEqual(type,"linear")){
     LMSize = LMSize +1;
     return true;
  }
  if(::nl->IsEqual(type,"composite")){
     CMSize = CMSize+1; // the composite move itself
     ListExpr rest = ::nl->Second(value);
     SMSize = SMSize+::nl->ListLength(rest); // the contained submoves
     while(!::nl->IsEmpty(rest)){
        if(!AddSubMovesSize(::nl->First(rest),LMSize,CMSize,SMSize,PMSize))
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
     else // incorrect listlength
         return false;
     
     return AddSubMovesSize(::nl->Second(PMove),
                            LMSize,CMSize,SMSize,PMSize);
  }
  // a unknown type description
  return false;
}


/*
~AddLinearMove~

This function adds the LinearMove given in value to this PMSimple.
If the list represents a valid Unit, this unit is append to to
appropriate dbarray and the argument __LMIndex__ is increased.
If the list is incorrect, this periodic moving object is not
changed and the result will be __false__.

[3] O(1)

*/
template <class T, class Unit>
bool PMSimple<T, Unit>::AddLinearMove(const ListExpr value,int &LMIndex, 
                                      int &CMIndex,
                                      int &SMIndex, int &PMIndex){
   __TRACE__

    Unit LM = Unit(0);
    if(!LM.ReadFrom(value))
       return false;
    linearMoves.Put(LMIndex,LM);
    LMIndex++;
    return true;
}

/*
~AddCompositeMove~

This function creates a CompositeMove from value and adds it
(and all contained submoves) to this PMSimple. The return 
value corresponds to the correctness of the list for this
composite move. The arguments are increased by the number of
contained submoves in this composite move. *Note*: This
function can change this periodic moving object even when 
the list is not correct. In the case of a result of __false__,
the state of this object is not defined and the defined flag
should be set to false.

[3] O(L), where L is the number of contained linear moves

*/
template <class T, class Unit>
bool PMSimple<T, Unit>::AddCompositeMove(const ListExpr value,
                                         int &LMIndex, int &CMIndex,
                                         int &SMIndex, int &PMIndex){
   __TRACE__
   // a composite move has to contains at least two submoves
   int len = ::nl->ListLength(value);
   if(len<2){
      if(DEBUG_MODE){
         std::cerr << __POS__ << " less than 2 submoves (" 
              << len << ")" << std::endl;
      }
      return false;
   }
   CompositeMove CM=CompositeMove(1);
   int CMPos=CMIndex;
   int SMPos=SMIndex;
   // ensure that no submove used the positions for this composite move
   CMIndex++;
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
      if(::nl->ListLength(SML)!=2){ // all submoves have the 
                                 // format (type value)
         if(DEBUG_MODE){
            std::cerr << __POS__ << " submove has wrong length (";
            std::cerr << ::nl->ListLength(SML) << ")" << std::endl;
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
               std::cerr << __POS__ << " can't add a linear move " << std::endl;
            }
            return false;
         }
         Unit LM;
         linearMoves.Get(LMPos,LM);
         // Append the interval of LM to CM
         if(isFirst){
            isFirst=false;
            CM.interval.Equalize(&(LM.interval));
         }else{
            if(!CM.interval.Append(&(LM.interval))){
               if(DEBUG_MODE){
                   std::cerr << __POS__ << " can't append interval " 
                             << std::endl;
                   std::cerr << "The original interval is";
                   std::cerr << CM.interval.ToString() << std::endl;
                   std::cerr << "The interval to append is";
                   std::cerr << LM.interval.ToString() << std::endl;
               }
               return false;
            }
         }
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
              std::cerr << __POS__ << "can't add period move " << std::endl;
            }
            return  false;
        }
        PeriodicMove PM;
        periodicMoves.Get(PMPos,PM);
        if(isFirst){
           isFirst=false;
           CM.interval.Equalize(&(PM.interval));
        }else{
           if(!CM.interval.Append(&(PM.interval))){
              if(DEBUG_MODE){
                 std::cerr << __POS__  << " can't append interval" << std::endl;
              }
              return false;
           }
        }
        CSubMove SM;
        SM.arrayNumber = PERIOD;
        SM.arrayIndex = PMPos;
        compositeSubMoves.Put(SMPos,SM);
        SMPos++;
   } else{ // not of type linear or period
        if(DEBUG_MODE){
            std::cerr << __POS__ << " submove not of type ";
            std::cerr << "linear od period" << std::endl;
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

Adds the period move described in value to this PMSimple.
*Note*: This function can change this objects even wehn 
the list does not represent a valid period move. If the 
result is __false__ the __defined__ flag of this move
should be set to false.

[3] O(L), where L is the number of contained linear moves

*/
template <class T, class Unit>
bool PMSimple<T, Unit>::AddPeriodMove(const ListExpr value,
                                      int &LMIndex, int &CMIndex,
                                      int &SMIndex, int &PMIndex){
   __TRACE__
   int len = ::nl->ListLength(value);
   if((len!=2) && (len!=4)){  // (repeatations <submove>)
      if(DEBUG_MODE)
        std::cerr << __POS__ << ": wrong listlength" << std::endl;
      return false;
   }
   if(::nl->AtomType(::nl->First(value))!=IntType){
     if(DEBUG_MODE){
       std::cerr << __POS__ << ": wrong type for repeatations" << std::endl;
     }
     return false;
   }
   int rep = ::nl->IntValue(::nl->First(value));
   // rep must be greater than 1 
   if(rep<=1 ){
      if(DEBUG_MODE){
         std::cerr << __POS__ <<  " wrong number of repeatations" << std::endl;
      }
      return false;
   }
   
   ListExpr SML;
   if(len==2){
      SML = ::nl->Second(value);
   } else { // len == 4 
      SML = ::nl->Fourth(value);
   }

   if(::nl->ListLength(SML)!=2){
      if(DEBUG_MODE){
         std::cerr << __POS__ << ": wrong length for submove" << std::endl;
      }
      return false;
   }
   PeriodicMove PM=PeriodicMove(1);
   PM.repeatations = rep;
   int IncludePos = PMIndex; // store the positiuon
   PMIndex++;
   ListExpr SMT = ::nl->First(SML); // take the submove type
   if(::nl->IsEqual(SMT,"linear")){
     int LMPos = LMIndex;
     if(!AddLinearMove(::nl->Second(SML),LMIndex,CMIndex,SMIndex,PMIndex)){
        if(DEBUG_MODE){
          std::cerr << __POS__ << ": can't add linear submove" << std::endl;
        }
        return false;
     }
     PM.submove.arrayNumber = LINEAR;
     PM.submove.arrayIndex = LMPos;
     Unit LM;
     linearMoves.Get(LMPos,LM);
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
     if(!AddCompositeMove(::nl->Second(SML),LMIndex,CMIndex,
                          SMIndex,PMIndex)){
        if(DEBUG_MODE){
           std::cerr << __POS__ << ": can't add composite submove" << std::endl;
        }
        return false;
     }
     PM.submove.arrayNumber = COMPOSITE;
     PM.submove.arrayIndex = CMPos;
     CompositeMove CM;
     compositeMoves.Get(CMPos,CM);
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
   return false; // invalid type
}

/*
~GetLastUnit~

This function returns the temporal last unit of this periodic moving point.
It is realized by going down the repetition tree up to a unit.

*/
template <class T, class Unit>
Unit PMSimple<T, Unit>::GetLastUnit()const{
    __TRACE__
    const SubMove* s = &submove;
    while(s->arrayNumber!=LINEAR){
        if(s->arrayNumber==PERIOD){
           PeriodicMove PM;
           periodicMoves.Get(s->arrayIndex,PM);
           s = &PM.submove;
        } else if(s->arrayNumber==COMPOSITE){
           CompositeMove CSM;
           compositeMoves.Get(s->arrayIndex,CSM);
           CSubMove csm;
           compositeSubMoves.Get(CSM.maxIndex,csm);  
           SubMove sm; 
           sm.arrayNumber = csm.arrayNumber;
           sm.arrayIndex = csm.arrayIndex;
           s = &sm;
        } else{
          assert(false); // unknown arraynumber
        }
    }
    Unit res;
    linearMoves.Get(s->arrayIndex,res);
    return res;
}
  
/*
~MinimizationRequired~

This function checks, whether an call of ~Minimize~ will change the
tree. Particularly, it checks whether two consecutive units can
be summarized to a single one.

*/
template <class T, class Unit>
bool PMSimple<T, Unit>::MinimizationRequired(){
  __TRACE__
  Unit LM;
  Unit LM2;
  CompositeMove CM;
  PeriodicMove  PM;
  const SubMove* SM=0;
  // check periodic moves
  int size = periodicMoves.Size();
  for(int i=0;i<size;i++){
      periodicMoves.Get(i,PM);
      SM = &PM.submove;
      if(SM->arrayIndex==LINEAR){
         linearMoves.Get(SM->arrayIndex,LM);
         if(LM.interval.CanAppended(&LM.interval))
             return true;
      }  
  }
  CSubMove CSM;
  CSubMove CSM2;
  // check composite moves
  size = compositeMoves.Size();
  for(int i=0;i<size;i++){
     compositeMoves.Get(i,CM);
     for(int k=CM.minIndex;k< CM.maxIndex;k++){
        compositeSubMoves.Get(k,CSM);
        compositeSubMoves.Get(k+1,CSM2);
        if( (CSM.arrayNumber==LINEAR) && (CSM2.arrayNumber==LINEAR)){
            linearMoves.Get(CSM.arrayIndex,LM);
            linearMoves.Get(CSM2.arrayIndex,LM2);
            if( LM.CanSummarized(&LM2))
                return true;
        }
     } 
  }
  return false;
}



/*
~MinimizeRec~

This function summarizes consecutive units with the same value.
The result is stored in the arguments. Note that no detection of
equal linear moves is performed. This means, when a unit occurs
twice or more, this unit will be also stored several times in
the corresponding dbarray.

*/
template <class T, class Unit>
SubMove PMSimple<T, Unit>::MinimizeRec(SubMove SM, 
                    DbArray<Unit>&                   newLinearMoves,
                    DbArray<CompositeMove>&          newCompositeMoves,
                    DbArray<CSubMove>&                newCompositeSubMoves,
                    DbArray<PeriodicMove>&           newPeriodicMoves,
                    Unit&                            Summarization,
                    bool&                            CompleteSummarized){

   if(SM.arrayNumber==LINEAR){
       linearMoves.Get(SM.arrayIndex,Summarization);
       CompleteSummarized=true;
       return SM;   
   }
   if(SM.arrayNumber==PERIOD){
       PeriodicMove PM2;
       periodicMoves.Get(SM.arrayIndex,PM2);
       SubMove SM2 = MinimizeRec(PM2.submove, 
                                 newLinearMoves,newCompositeMoves,
                                 newCompositeSubMoves,newPeriodicMoves,
                                 Summarization, CompleteSummarized);
       if(!CompleteSummarized){
          PM2.submove = SM2; 
          int Pos = newPeriodicMoves.Size();
          newPeriodicMoves.Append(PM2);
          SM2.arrayNumber=PERIOD;
          SM2.arrayIndex= Pos;
          CompleteSummarized = false;
          return SM2;
       }else{ // the result is a single linear move
          if(!Summarization.interval.CanAppended(
                               &Summarization.interval)){ // a little gap
              int LinPos = newLinearMoves.Size();
              newLinearMoves.Append(Summarization);
              PM2.submove.arrayNumber=LINEAR;
              PM2.submove.arrayIndex=LinPos;
              // store this periodic move
              int PerPos = newPeriodicMoves.Size();
              newPeriodicMoves.Append(PM2);
              SM2.arrayNumber=PERIOD;
              SM2.arrayIndex=PerPos;
              CompleteSummarized=false;
              return SM2;
          }else{ // we don't need longer this periodic move
             Summarization.interval.Mul(PM2.repeatations);
             return SM2;
         } 
       }
   }
   if(SM.arrayNumber==COMPOSITE){
        CompositeMove CM;
        compositeMoves.Get(SM.arrayIndex,CM);
        Unit LM(0);
        bool LMdefined = false;
        std::vector<CSubMove> MySubMoves;
        for(int i=CM.minIndex;i<=CM.maxIndex;i++){
           CSubMove Current;
           compositeSubMoves.Get(i,Current);
           SubMove SM2 = MinimizeRec(Current,
                                     newLinearMoves,newCompositeMoves,
                                     newCompositeSubMoves,newPeriodicMoves,
                                     Summarization, CompleteSummarized);
           if(!CompleteSummarized){
              // store summarized submove if present
              if(LMdefined){
                 int LinPos = newLinearMoves.Size();
                 newLinearMoves.Append(LM);
                 LMdefined=false;
                 CSubMove SM3;
                 SM3.arrayNumber=LINEAR;
                 SM3.arrayIndex=LinPos;
                 MySubMoves.push_back(SM3);
              }
              CSubMove SM3;
              SM3.arrayIndex = SM2.arrayIndex;
              SM3.arrayNumber= SM2.arrayNumber;
              MySubMoves.push_back(SM3); 
           }else{ // submove complete summarized
              if(!LMdefined){ // first summarized LinearMove;
                LM = Summarization;
                LMdefined=true;
              }else{
                  
                if(LM.CanSummarized(&Summarization)){
                    // append the new summarization to LM
                    LM.interval.Append(&Summarization.interval);
                } else{
                    // the new summarization cannot appendend to LM
                    // store LM
                    int LinPos = newLinearMoves.Size();
                    newLinearMoves.Append(LM);
                    CSubMove SM3;
                    SM3.arrayNumber=LINEAR;
                    SM3.arrayIndex=LinPos;
                    MySubMoves.push_back(SM3);
                    LM = Summarization; 
                }  
              }  
          }
        } // all submoves processed;
        if(MySubMoves.size()==0){ // all its collected into LM
            CompleteSummarized=true;
            Summarization = LM;
            return SM; // unimportant in this case
        }else{
            if(LMdefined){ // store the last summarization
              int LinPos = newLinearMoves.Size();
              newLinearMoves.Append(LM);
              CSubMove SM3;
              SM3.arrayNumber = LINEAR;
              SM3.arrayIndex  = LinPos;
              MySubMoves.push_back(SM3); 
            }
            CM.minIndex = newCompositeSubMoves.Size();
            CM.maxIndex = CM.minIndex + MySubMoves.size()-1;
            // store the submoves
            for(unsigned int k=0;k<MySubMoves.size();k++){
               newCompositeSubMoves.Append(MySubMoves[k]);  
            }
            SubMove SM3;
            SM3.arrayNumber=COMPOSITE;
            SM3.arrayIndex=newCompositeMoves.Size();
            newCompositeMoves.Append(CM);
            CompleteSummarized=false;
            return SM3;
        }
   } 
   assert(false);
}


/*
~CorrectDurationSums~

Each son of a composite move stores the sum of all from its 
predecessors. This enables binary search for an given instant
within composite moves. Because ensuring correct values is 
a little bit difficult within functions changing the arrays (a lot
of special cases), this function provides a correction of the stored
durations. This means, the functions can work without be care about the
durations when this function is called after the work of the function is 
done.

*/
template <class T, class Unit>
void PMSimple<T, Unit>::CorrectDurationSums(){
   if(IsEmpty() || !IsDefined()){  // nothing to do
      return;
   }
   int cmsize = compositeMoves.Size();
   RelInterval currentInterval;
   datetime::DateTime currentLength(datetime::durationtype);
   datetime::DateTime duration(datetime::durationtype);
   CompositeMove CM;
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


} // end of namespace periodic


#endif
