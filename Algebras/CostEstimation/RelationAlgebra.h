/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#ifndef COST_EST_RELATION_ALG_H
#define COST_EST_RELATION_ALG_H


/*
3.0 ~class~ FeedLocalInfo

*/


class FeedLocalInfo: public ProgressLocalInfo
{
public:
  FeedLocalInfo() : rit(0) {}
  ~FeedLocalInfo() {
     if (rit) {
        delete rit; rit = 0;
     }
  }

  GenericRelationIterator* rit;
};


/*
3.0 ~class~ CostEstimation class for operator filter

*/
class FeedCostEst : public CostEstimation 
{

public:
    FeedCostEst()
    {
    }

  virtual ~FeedCostEst() {};

  virtual int requestProgress(Word* args, ProgressInfo* pRes, void* localInfo, 
    bool argsAvialable) {
      
      GenericRelation* rr;
      rr = (GenericRelation*)args[0].addr;

      ProgressInfo p1;
      Supplier sonOfFeed;

      // Determination of constants in file bin/UpdateProgressConstants

      static const double uFeed = 
        ProgressConstants::getValue("Relation-C++", "feed", "uFeed");
          // 0.00296863;  milliseconds per tuple
      static const double vFeed = 
        ProgressConstants::getValue("Relation-C++", "feed", "vFeed");
          // 0.0010081649; milliseconds per attribute

      FeedLocalInfo* fli = (FeedLocalInfo*) localInfo;
      sonOfFeed = qp->GetSupplierSon(supplier, 0);

      if ( fli )
      {
        fli->sizesChanged = false;

        if ( !fli->sizesInitialized )
        {
          fli->noAttrs = nl->ListLength(nl->Second(
            nl->Second(qp->GetType(supplier))));
          
          fli->attrSize = new double[fli->noAttrs];
          fli->attrSizeExt = new double[fli->noAttrs];

          fli->Size =  0;
          fli->SizeExt =  0;

          for ( int i = 0;  i < fli->noAttrs; i++)
          {
            fli->attrSize[i] = rr->GetTotalSize(i) / (fli->total + 0.001);
            fli->attrSizeExt[i] = rr->GetTotalExtSize(i) / (fli->total + 0.001);

            fli->Size += fli->attrSize[i];
            fli->SizeExt += fli->attrSizeExt[i];
          }
          fli->sizesInitialized = true;
          fli->sizesChanged = true;
        }
      }

      if ( qp->IsObjectNode(sonOfFeed) )
      {
        if ( !fli ) {
           return CANCEL;
        } else  //an object node, fli defined
        {
          pRes->Card = (double) fli->total;
          pRes->CopySizes(fli);  //copy all sizes
          pRes->Time = (fli->total + 1) * (uFeed + fli->noAttrs * vFeed);

          //any time value created must be > 0; so we add 1
          pRes->Progress = fli->returned * (uFeed + fli->noAttrs * vFeed)
            / pRes->Time;

          pRes->BTime = 0.001; //time must not be 0

          pRes->BProgress = 1.0;

          return YIELD;
        }
      }
      else //not an object node
      {
        if ( qp->RequestProgress(sonOfFeed, &p1) )
        {
          pRes->Card = p1.Card;
          pRes->CopySizes(p1);
          pRes->Time = p1.Time + p1.Card * (uFeed + p1.noAttrs * vFeed);
          pRes->Progress =
             ((p1.Progress * p1.Time) +
              (fli ? fli->returned : 0) * (uFeed + p1.noAttrs * vFeed))
              / pRes->Time;

          pRes->BTime = p1.Time;

          pRes->BProgress = p1.Progress;

          return YIELD;
        } else {
          return CANCEL;
        }
      }
     return 0;
    }


/*
3.1 init our class

*/
  virtual void init(Word* args, void* localInfo) 
  {
    returned = 0; 
  }

private:
};

/*
3.1 Progress data for operator consume

*/
struct consumeLocalInfo
{
  consumeLocalInfo(Word& w): state(0), current(0), stream(w){}

  int state;      //0 = working, 1 = finished
  int current;    //current no of tuples read
  Stream<Tuple> stream;

  ostream& print(ostream& o){
    o << "Localinfo (" << (void*) this << "), state=" << state << ", current="
      << current << ", stream=";
    stream.print(o);
    o << ".";
    return o;
  }

  private:
    consumeLocalInfo(): state(0), current(0),stream(0) {}
};

/**
3.0 ~Class~ ConsumeCostEst

*/
class ConsumeCostEst: public CostEstimation
{
    
  public:
    ConsumeCostEst()
    {
    }

  virtual ~ConsumeCostEst() {};

  virtual int requestProgress(Word* args, ProgressInfo* pRes, 
    void* localInfo, bool argsAvialable) {
    
    ProgressInfo p1;

    static const double uConsume = 
      ProgressConstants::getValue("Relation-C++", "consume", "uConsume");
        // 0.024; millisecs per tuple
    static const double vConsume = 
      ProgressConstants::getValue("Relation-C++", "consume", "vConsume");
        // 0.0003;  millisecs per byte in root/extension
    const double wConsume = 
      ProgressConstants::getValue("Relation-C++", "consume", "wConsume");
        // 0.001338; millisecs per byte in FLOB

    consumeLocalInfo* cli = (consumeLocalInfo*) localInfo;

    if ( cli->stream.requestProgress( &p1) )
    {
      pRes->Card = p1.Card;
      pRes->CopySizes(p1);

      pRes->Time = p1.Time +
        p1.Card * (uConsume + p1.SizeExt * vConsume
          + (p1.Size - p1.SizeExt) * wConsume);

      if ( cli == 0 )    //not yet working
      {
        pRes->Progress = (p1.Progress * p1.Time) / pRes->Time;
      }
      else
      {
        if ( cli->state == 0 )  //working
        {

          if ( p1.BTime < 0.1 && pipelinedProgress )   //non-blocking,
                                                       //use pipelining
            pRes->Progress = p1.Progress;
          else
            pRes->Progress =
            (p1.Progress * p1.Time +
              cli->current *  (uConsume + p1.SizeExt * vConsume
                  + (p1.Size - p1.SizeExt) * wConsume) )
                / pRes->Time;
        }
        else       //finished
        {
          pRes->Progress = 1.0;
        }
      }

      pRes->BTime = pRes->Time;    //completely blocking
      pRes->BProgress = pRes->Progress;

      return YIELD;      //successful
    } else {
       return CANCEL;      //no progress available
    }

  
  }
  
/*
3.1 init our class

*/
  virtual void init(Word* args, void* localInfo) 
  {
    returned = 0;
  }

private:

};



/*
3.1 ~class~ FilterCostEst

*/
class FilterCostEst: public CostEstimation{
   public:
      FilterCostEst() : read(0), done(false), 
                        initialized(false), pi_source()
       {

      }

      ~FilterCostEst() {}

      virtual int requestProgress(Word* args,
                                  ProgressInfo* result,
                                  void* localInfo,
                                  const bool argsAvailable) {

          const double uFilter = 1.0;
          if(!qp->RequestProgress(args[0].addr,&pi_source)){
            return CANCEL;
          } 
          result->CopySizes(pi_source);
          if(done){
            result->Card = returned;
            result->Progress = 1.0;
            result->CopyBlocking(pi_source);
            double predCost = qp->GetPredCost(supplier);
            result->Time = pi_source.Time + (double)read*predCost * uFilter;
            return YIELD;
          }

          double selectivity = 1.0; // will be overwritten
          result->Progress = pi_source.Progress; // may be overwritten later
          bool overwriteProgress = !(pi_source.BTime < 0.1 && 
                                     pipelinedProgress);
          result->Time = pi_source.Time + pi_source.Card * 
                                         qp->GetPredCost(supplier) * uFilter;

          if(returned > (size_t)enoughSuccessesSelection ||
             returned >= pi_source.Card * qp->GetSelectivity(supplier)){
             // stable state assumed now or more returned than cold estimate
             selectivity = (double) returned / (double) read;
             if(overwriteProgress){
               result->Progress = (pi_source.Progress*pi_source.Time +
                                   read*qp->GetPredCost(supplier)*uFilter) / 
                                  result->Time; 
             }
          }  else {
             // cold state
             selectivity = qp->GetSelectivity(supplier);
             if(overwriteProgress){
                result->Progress = (pi_source.Progress * pi_source.Time)/
                                   result->Time;
             }
          }
          result->Card = pi_source.Card * selectivity;
          
          result->CopyBlocking(pi_source);
          return YIELD; 
      }

     virtual void init(Word* args, void* localInfo) {
        read = 0;
        returned = 0;
        done = false;
        initialized = true;
     }

     void incInput(){
        read++;
     }

     void finished(){
       done = true;
     }


   private:
     int read;
     bool done;
     bool initialized;
     ProgressInfo pi_source;
};

/*
3.1 Progress data for Filter

*/
class FilterLocalInfo{

  public:
     FilterLocalInfo(Word s, Word _fun, FilterCostEst* _fce):
         stream(s), fun(_fun), fce(_fce) {
        funargs = qp->Argument(fun.addr);
        stream.open();
        fce->init(0,0);
     }

     ~FilterLocalInfo(){
        stream.close();
     }

     Tuple* next(){
       Tuple* tuple;
       Word funres;
       while( (tuple = stream.request() ) != 0){
          fce->incInput();
          (*funargs)[0].addr = tuple;
          qp->Request(fun.addr,funres);
          CcBool* res = (CcBool*) funres.addr;
          if(res->IsDefined() && res->GetBoolval()){
            return tuple;
          }
          tuple->DeleteIfAllowed();
       }
       fce->finished();
       return 0;
     }

  private:
    Stream<Tuple> stream;
    Word fun;
    ArgVectorPointer funargs;
    FilterCostEst* fce;
};

#endif
