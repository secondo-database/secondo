

/*
3.17 ~PMInt9M~

*/

#include <iostream>
#include <string>

#include "NestedList.h"
#include "StandardTypes.h"
#include "PeriodicSupport.h"
#include "PeriodicTypes.h"
#include "TopRel.h"
#include "PMSimple.h"


extern NestedList* nl;
using namespace std;

namespace periodic{
/*
~Constructor~

*/
PMInt9M::PMInt9M(){}

/*
~Constructor~

This constructor should be used for creating a PMInt9M
instance. It calls the constructor of the superclass.

*/
PMInt9M::PMInt9M(int dummy):
     PMSimple<Int9M,LinearInt9MMove>(dummy) 
{
       __TRACE__
}


/*
~Transpose~

The Transpose function will change the roles of the arguments for which 
this periodic moving 9 intersection matrix is computed.

*/
void PMInt9M::Transpose(){
  int size = linearMoves.Size();
  const LinearInt9MMove* LM;
  LinearInt9MMove LM2;
  for(int i=0;i<size;i++){
     linearMoves.Get(i,LM);
     LM2 = (*LM); 
     LM2.Transpose();
     linearMoves.Put(i,LM2);
  } 

}

/*
~CreateFrom~

When this function is called, the linearConstantMove is build from the
given values with an additional level between the tree and the linear moves.
This means, the value of this pmsimple is given by the structure showed in
figure [ref]{fig:RepTreeWithAdditionalLevel.eps}.

                Figure 3: Repetition Tree with an additional Level [RepTreeWithAdditionalLevel.eps]

This can be usedful in operation where the actual repetions remains but 
the units can be split. An example is the computation of the 
topological relationship between a periodic moving point and a non-moving
spatial object. The repetitions in the movement of the points are preserved
but it is possible that the topological relationship changes in single
units of this moving point. In this case, additional composite moves
must be inserted in the structure. This is exactly what this function does.

*/
bool PMInt9M::CreateFrom( const DBArray<LinearInt9MMove>& linearMoves, 
                 const ArrayRange*                     level,
                 const int                             levelsize,
                 const DBArray<CompositeMove>&          compositeMoves,
                 const DBArray<CSubMove>&                compositeSubMoves,
                 const DBArray<PeriodicMove>&           periodicMoves,
                 const DateTime                        startTime,
                 const SubMove                         submove) {
   __TRACE__

   defined =true;
   this->startTime.Equalize(&startTime);
   canDelete=false;
   this->submove.Equalize(&submove);
   const PeriodicMove* PM;
   const CSubMove* SM;
   const CompositeMove* CM;
   const LinearInt9MMove* LM;
   switch(submove.arrayNumber){
     case PERIOD: { periodicMoves.Get(submove.arrayIndex,PM);
                    this->interval.Equalize(&(PM->interval));
                    break;
                   }
     case LINEAR: { linearMoves.Get(submove.arrayIndex,LM);
                    this->interval.Equalize(&(LM->interval));
                    break;
                  }
     case COMPOSITE: { compositeMoves.Get(submove.arrayIndex,CM);
                       this->interval.Equalize(&(CM->interval));
                       break;
                     }
     default:   assert(false);

   }
   this->linearMoves.Clear();
   if(linearMoves.Size()>0)
      this->linearMoves.Resize(linearMoves.Size());
   for(int i=0;i<linearMoves.Size();i++){
       linearMoves.Get(i,LM);
       LinearInt9MMove LM2 = (*LM);
       this->linearMoves.Put(i,LM2);  
   }
 
   this->compositeMoves.Clear();
   if(compositeMoves.Size()>0)
      this->compositeMoves.Resize(compositeMoves.Size());
   for(int i=0;i<compositeMoves.Size();i++){
        compositeMoves.Get(i,CM);
        CompositeMove CM2 = (*CM);
        this->compositeMoves.Put(i,CM2);
   }
   this->periodicMoves.Clear();
   if(periodicMoves.Size()>0)
      this->periodicMoves.Resize(periodicMoves.Size());
   for(int i=0;i<periodicMoves.Size();i++){
        periodicMoves.Get(i,PM);
        PeriodicMove PM2 = (*PM);
        this->periodicMoves.Put(i,PM2);
   }

   if(levelsize==linearMoves.Size()){// easy case: no additional Moves
        this->compositeSubMoves.Clear();
        if(compositeSubMoves.Size()>0)
            this->compositeSubMoves.Resize(compositeSubMoves.Size());
        for(int i=0;i<compositeSubMoves.Size();i++) {
            compositeSubMoves.Get(i,SM);
            CSubMove SM2 = (*SM);
            this->compositeSubMoves.Put(i,SM2);
        }
        return true;
   }
   // we have to restructure the tree :-(
   // for the periodicMoves, we have to change the arrayindex of an 
   // linear submove or we have to build a new composite move
   // "pointers" to composite moves are not affected
   ArrayRange ar;
   int minsize = compositeSubMoves.Size()>0?compositeSubMoves.Size():1;
   this->compositeSubMoves.Resize(minsize); 

   // process the compositeMoves
   for(int i=0;i<compositeMoves.Size();i++){
      compositeMoves.Get(i,CM);
      int pos = this->compositeSubMoves.Size();
      int count = 0;
      for(int j=CM->minIndex;j<=CM->maxIndex;j++){
          compositeSubMoves.Get(j,SM);
          if(SM->arrayNumber!=LINEAR){ // copy the submove
             CSubMove SM2 = (*SM);
             this->compositeSubMoves.Append(SM2);
             count++;
          } else{ // a linear submove
             ar = level[SM->arrayIndex];
             if(ar.minIndex==ar.maxIndex){
                CSubMove SM2 = (*SM);
                SM2.arrayIndex=ar.minIndex;
                this->compositeSubMoves.Append(SM2);
                count++;
             } else{ // insert new submoves
               for(int k=ar.minIndex;k<=ar.maxIndex;k++){
                  CSubMove SM2 = (*SM);
                  SM2.arrayNumber=LINEAR;
                  SM2.arrayIndex=k;
                  this->compositeSubMoves.Append(SM2);
                  count++;
               }
            }
          }
      }
      CompositeMove CM2 = (*CM);
      CM2.minIndex=pos;
      CM2.maxIndex=pos+count-1;
      this->compositeMoves.Put(i,CM2); 
   }

   // process the periodic moves 
   for(int i=0;i<this->periodicMoves.Size();i++){
        this->periodicMoves.Get(i,PM);
        PeriodicMove PM2 = (*PM);
        SubMove SM2 = PM2.submove;
        CSubMove SM3;
        DateTime duration = DateTime(durationtype);
        if(submove.arrayNumber==LINEAR){ // otherwise is nothing to do
          ar = level[SM2.arrayIndex];
          if(ar.minIndex==ar.maxIndex){ // ok, just correct the index
            SM2.arrayIndex=ar.minIndex;
          } else{ // create a new composite move
             // first create the appropriate submoves
             int pos = this->compositeSubMoves.Size(); 
             RelInterval i;
             for(int j=ar.minIndex;j<=ar.maxIndex;j++){
                SM3.arrayNumber = LINEAR;
                SM3.arrayIndex  = j;
                SM3.duration.Equalize(&duration);
                linearMoves.Get(j,LM);
                RelInterval interval = LM->interval;
                DateTime length(durationtype);
                interval.GetLength(length);
                duration += length;
                this->compositeSubMoves.Append(SM3);
                if(j==ar.minIndex){
                    i =  LM->interval;
                }else{
                    i.Append(&(LM->interval));
                }
             }
             CompositeMove CM2; 
             CM2.minIndex=pos;
             CM2.maxIndex=pos+ar.maxIndex-ar.minIndex+1;
             CM2.interval.Equalize(&i);
             this->compositeMoves.Append(CM2);
             PM2.submove.arrayNumber=COMPOSITE;
             PM2.submove.arrayIndex=this->compositeMoves.Size()-1;
         } 
         // write back the periodic move
         this->periodicMoves.Put(i,PM2); 
        }
   }
   return true;
}


} // end of namespace periodic
