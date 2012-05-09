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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[&] [\&]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]
//[ast] [\ensuremath{\ast}]

*/

/*
[1] RelationalAlgebraCostEstimation 

April, 2012. Jan Kristof Nidzwetzki

[TOC]

0 Description

This file provides some CostEstimationClasses for the RelationalAlgebra. 

Most of the code is based on the old REQUESTPROGRESS implementation.

Apr 2012, JKN, First version of this file

Mai 2012, JKN, Removed progress constants and using class ProgressConstants 

*/

/*
0.1 Defines

*/

#ifndef COST_EST_RELATION_ALG_H
#define COST_EST_RELATION_ALG_H


/*
1.0 ~class~ FeedLocalInfo

*/

class FeedLocalInfo: public ProgressLocalInfo
{
public:
  FeedLocalInfo() : rit(0) {}
  
  ~FeedLocalInfo() {
     if (rit) {
        delete rit; 
        rit = 0;
     }
  }

  GenericRelationIterator* rit;
};

/**
1.1 The class ~FeedCostEstimation~ provides cost estimation
    capabilities for the operator feed

*/
class FeedCostEstimation : public CostEstimation 
{

public:
    FeedCostEstimation()
    {
    }

  virtual ~FeedCostEstimation() {};

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
1.1.1 init our class

*/
  virtual void init(Word* args, void* localInfo) 
  {
    returned = 0; 
  }

private:
};

/*
2.0 Local datastructure for operator consume

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
2.1 The class ~ConsumeCostEstimation~ provides cost estimation
    capabilities for the operator consume

*/
class ConsumeCostEstimation: public CostEstimation
{
    
  public:
    ConsumeCostEstimation()
    {
    }

  virtual ~ConsumeCostEstimation() {};

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

    if(cli == NULL) {
       return CANCEL;
    }

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
2.1.1 init our class

*/
  virtual void init(Word* args, void* localInfo) 
  {
    returned = 0;
  }

private:

};

/**
3.1 The class ~FilterCostEstimation~ provides cost estimation
    capabilities for the operator filter

*/
class FilterCostEstimation: public CostEstimation{
   public:
      FilterCostEstimation() : read(0), done(false), 
                        initialized(false), pi_source()
       {}

      ~FilterCostEstimation() {}

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
3.1 Class ~FilterLocalInfo~ is a local datastructure
    used by ~FilterCostEst~

*/
class FilterLocalInfo{

  public:
     FilterLocalInfo(Word s, Word _fun, FilterCostEstimation* _fce):
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
    FilterCostEstimation* fce;
};

/*
4.0 Class ~FeedProjLocalInfo~ is a local datastructure
    used by ~FeedProjectCostEstimation~

*/
class FeedProjLocalInfo : public FeedLocalInfo
{
  public:
    double argTupleSize;

    FeedProjLocalInfo(TupleType* ptr) :
      FeedLocalInfo(), argTupleSize(0), tt(ptr)
    {
       returned = 0;
    }
    ~FeedProjLocalInfo()
    {
       if ( tt->DeleteIfAllowed() ) {
          tt = 0;
       }
    }

  private:
    TupleType* tt;
};


/**
4.1 The class ~FeedProjectCostEstimation~ provides cost estimation
    capabilities for the operator feedproject

*/
class FeedProjectCostEstimation: public CostEstimation 
{

public:
   FeedProjectCostEstimation()
   {
   }

   virtual ~FeedProjectCostEstimation() {};

   virtual int requestProgress(Word* args, ProgressInfo* pRes, 
      void* localInfo, bool argsAvialable) {
      
      GenericRelation* rr;
      rr = (GenericRelation*)args[0].addr;

      Word elem(Address(0));
      Supplier sonOfFeed;
      int index= 0;
      
      ProgressInfo p1;

      // Determination of constants in file bin/UpdateProgressConstants

      // millisecs per tuple 0.002
      static const double uFeedProject = 
         ProgressConstants::getValue
         ("Relation-C++", "feedproject", "uFeedProject");

      //millisecs per byte input 0.000036
      static const double vFeedProject = 
         ProgressConstants::getValue
         ("Relation-C++", "feedproject", "vFeedProject");

      //millisecs per attr 0.0018
      static const double wFeedProject = 
         ProgressConstants::getValue
         ("Relation-C++", "feedproject", "wFeedProject");

      FeedProjLocalInfo* fli = (FeedProjLocalInfo*) localInfo;
      sonOfFeed = qp->GetSupplierSon(supplier, 0);

      if ( fli )
      {
        fli->sizesChanged = false;

        if ( !fli->sizesInitialized )
        {
          fli->Size = 0;
          fli->SizeExt = 0;
          fli->noAttrs = ((CcInt*)args[2].addr)->GetIntval();
          fli->attrSize = new double[fli->noAttrs];
          fli->attrSizeExt = new double[fli->noAttrs];

          fli->argTupleSize = rr->GetTotalExtSize()
              / (fli->total + 0.001);

          for( int i = 0; i < fli->noAttrs; i++)
          {
            Supplier son = qp->GetSupplier(args[3].addr, i);
            qp->Request(son, elem);
            
            index = ((CcInt*) elem.addr)->GetIntval();
            
            fli->attrSize[i] = rr->GetTotalSize(index-1)
              / (fli->total + 0.001);
            
            fli->attrSizeExt[i] = rr->GetTotalExtSize(index-1)
              / (fli->total + 0.001);

            fli->Size += fli->attrSize[i];
            fli->SizeExt += fli->attrSizeExt[i];
            fli->sizesInitialized = true;
            fli->sizesChanged = true;
          }
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

          pRes->Time = (fli->total + 1) *
            (uFeedProject
            + fli->argTupleSize * vFeedProject
            + fli->noAttrs * wFeedProject);

          //any time value created must be > 0; so we add 1

          pRes->Progress = fli->returned * (uFeedProject
            + fli->argTupleSize * vFeedProject
            + fli->noAttrs * wFeedProject)
            / pRes->Time;

          pRes->BTime = 0.001; //time must not be 0

          pRes->BProgress = 1.0;

          return YIELD;
        }
      } else { //not an object node
        if ( qp->RequestProgress(sonOfFeed, &p1) )
        {
          pRes->Card = p1.Card;

          pRes->CopySizes(p1);

          pRes->Time = p1.Time + p1.Card * (uFeedProject
            + fli->argTupleSize * vFeedProject
            + fli->noAttrs * wFeedProject);

          pRes->Progress =
             ((p1.Progress * p1.Time) +
              (fli ? fli->returned : 0) * (uFeedProject
                + fli->argTupleSize * vFeedProject
                + fli->noAttrs * wFeedProject))
              / pRes->Time;

          pRes->BTime = p1.Time;

          pRes->BProgress = p1.Progress;

          return YIELD;
        } else {
          return CANCEL;
        }
   }
}

/*
4.1.1 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
  }

private:
};

/*
5.0 The class ~FeedProjectCostEstimation~ is an
    auxiliary class used by ~ProjectCostEstimation~

*/
class ProductLocalInfo: public ProgressLocalInfo
{
public:

  ProductLocalInfo() :
    resultTupleType(0),
    currentTuple(0),
    rightRel(0),
    iter(0)
  {}

  ~ProductLocalInfo()
  {
    if(currentTuple != 0)
      currentTuple->DeleteIfAllowed();
    if( iter != 0 )
      delete iter;
    resultTupleType->DeleteIfAllowed();
    if( rightRel )
    {
      rightRel->Clear();
      delete rightRel;
    }
  }

  TupleType *resultTupleType;
  Tuple* currentTuple;
  TupleBuffer *rightRel;
  GenericRelationIterator *iter;
};

/**
5.1 The class ~ProductCostEstimation~ provides cost estimation
    capabilities for the operator product

*/
class ProductCostEstimation: public CostEstimation
{
public:

// default constructor
ProductCostEstimation()
{
}

// default destructor
virtual ~ProductCostEstimation() 
{
}

virtual int requestProgress(Word* args, ProgressInfo* pRes, 
         void* localInfo, bool argsAvialable) {

      // Determination of constants in file bin/UpdateProgressConstants
      
      //millisecs per byte (right input) if writing to disk 0.0003
      static const double uProduct = 
         ProgressConstants::getValue("Relation-C++", "product", "uProduct");

      //millisecs per byte (total output) if reading from disk  0.000042
      static const double vProduct = 
         ProgressConstants::getValue("Relation-C++", "product", "vProduct");
     
      ProgressInfo p1, p2;
      ProductLocalInfo* pli;

      pli = (ProductLocalInfo*)localInfo;

      if (!pli) {
         return CANCEL;
      }

      if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2))
      {
        pli->SetJoinSizes(p1, p2);

        pRes->Card = p1.Card * p2.Card;
        pRes->CopySizes(pli);

        pRes->Time = p1.Time + p2.Time +
          p2.Card * p2.Size * uProduct +
          p1.Card * p2.Card * pRes->Size * vProduct;

        pRes->Progress =
          (p1.Progress * p1.Time + p2.Progress * p2.Time +
           pli->readSecond * p2.Size * uProduct +
           pli->returned * pRes->Size * vProduct)
          / pRes->Time;


        pRes->BTime = p1.BTime + p2.BTime +
          p2.Card * p2.Size * uProduct;

        pRes->BProgress =
          (p1.BProgress * p1.BTime + p2.BProgress * p2.BTime +
           pli->readSecond * p2.Size * uProduct)
          / pRes->BTime;

        return YIELD;
      } else {
        return CANCEL;
      }
}

/*
5.1.1 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
  }

private:

};


/*
6.0 Class ~ProjectLocalInfo~ is an classauxiliary 
    datastructure used by operator progress

*/
class ProjectLocalInfo: public ProgressLocalInfo
{
public:

  ProjectLocalInfo() {
    tupleType = 0;
    read = 0;
  }

  ~ProjectLocalInfo() {
    tupleType->DeleteIfAllowed();
    tupleType = 0;
  }

  TupleType *tupleType;
};


class ProjectCostEstimation: public CostEstimation
{
public:

// default constructor
ProjectCostEstimation()
{
}

// default destructor
virtual ~ProjectCostEstimation()
{
}

virtual int requestProgress(Word* args, ProgressInfo* pRes,
         void* localInfo, bool argsAvialable) {

      // Determination of constants in file bin/UpdateProgressConstants

      //millisecs per tuple 0.00073
      static const double uProject = 
         ProgressConstants::getValue("Relation-C++", "project", "uProject");

      //millisecs per tuple and attribute 0.0004 
      static const double vProject = 
         ProgressConstants::getValue("Relation-C++", "project", "vProject");

      ProgressInfo p1;
      ProjectLocalInfo* pli;
      pli = (ProjectLocalInfo*) localInfo;

      Supplier son;
      Word elem2(Address(0));
      int index= 0;
      
      if ( !pli ) {
         return CANCEL;
      }

      if ( qp->RequestProgress(args[0].addr, &p1) )
      {
        pli->sizesChanged = false;

        if ( !pli->sizesInitialized )
        {
          pli->noAttrs = ((CcInt*)args[2].addr)->GetIntval();
          pli->attrSize = new double[pli->noAttrs];
          pli->attrSizeExt = new double[pli->noAttrs];
        }

        if ( !pli->sizesInitialized || p1.sizesChanged )
        {
          pli->Size = 0;
          pli->SizeExt = 0;

          for( int i = 0; i < pli->noAttrs; i++)
          {
            son = qp->GetSupplier(args[3].addr, i);
            qp->Request(son, elem2);
            index = ((CcInt*)elem2.addr)->GetIntval();
            pli->attrSize[i] = p1.attrSize[index-1];
            pli->attrSizeExt[i] = p1.attrSizeExt[index-1];
            pli->Size += pli->attrSize[i];
            pli->SizeExt += pli->attrSizeExt[i];
          }
          pli->sizesInitialized = true;
          pli->sizesChanged = true;
        }

        pRes->Card = p1.Card;
        pRes->CopySizes(pli);

        pRes->Time = p1.Time + p1.Card * (uProject + pli->noAttrs * vProject);

    //only pointers are copied; therefore the tuple sizes do not
    //matter

        if ( p1.BTime < 0.1 && pipelinedProgress )   //non-blocking,
                                                        //use pipelining
          pRes->Progress = p1.Progress;
        else
          pRes->Progress =
            (p1.Progress * p1.Time +
              pli->read * (uProject + pli->noAttrs * vProject))
            / pRes->Time;

  pRes->CopyBlocking(p1);    //non-blocking operator

        return YIELD;
      } else {
        return CANCEL;
      }
}

/*
6.0.1 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
  }

private:

};

/**
7.0 The class ~RenameCostEstimation~ provides cost estimation
    capabilities for the operator rename

*/
class RenameCostEstimation: public CostEstimation
{
public:

// default constructor
RenameCostEstimation()
{
}

// default destructor
virtual ~RenameCostEstimation()
{
}

virtual int requestProgress(Word* args, ProgressInfo* pRes,
         void* localInfo, bool argsAvialable) {
 
      ProgressInfo p1;

      if ( qp->RequestProgress(args[0].addr, &p1) )
      {
        pRes->Copy(p1);
        return YIELD;
      } else {
        return CANCEL;
      }
}

/*
7.0.1 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
  }

private:

};

/*
8.0 Define the size of the buffer,
    used in operator buffer

*/
const int BUFFERSIZE = 52;

/* 
8.1 the class ~BufferLocalInfo~ is an
    auxiliary datastrucutre used by operator buffer

*/
template <class T>
class BufferLocalInfo
{
 public:
 int state;        //0 = initial, 1 = buffer filled,
                   //2 = buffer empty again
 int noInBuffer;   //tuples read into buffer;
 int next;         //index of next tuple to be returned;
 T* buffer[BUFFERSIZE];
};

/**
8.2 The class ~BufferCostEstimation~ provides cost estimation
    capabilities for the operator buffer

*/
class BufferCostEstimation : public CostEstimation 
{

public:
    BufferCostEstimation()
    {
    }

  virtual ~BufferCostEstimation() {};

  virtual int requestProgress(Word* args, ProgressInfo* pRes, void* localInfo,
    bool argsAvialable) {
      
      ProgressInfo p1;

      if ( qp->RequestProgress(args[0].addr, &p1) )
      {
        pRes->Copy(p1);
        return YIELD;
      } else {
        return CANCEL;
      }
   }

/*
8.1.1 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
  }

private:
};

/**
9.0 The class ~TCountRelCostEstimation~ provides cost estimation
    capabilities for the operator count(relation)

*/
class TCountRelCostEstimation : public CostEstimation 
{

public:
    TCountRelCostEstimation()
    {   
    }   

  virtual ~TCountRelCostEstimation() {}; 

  virtual int requestProgress(Word* args, ProgressInfo* pRes, void* localInfo,
    bool argsAvialable) {
    
    ProgressInfo p1;
    Supplier sonOfCount;
    sonOfCount = qp->GetSupplierSon(supplier, 0);

    //cout << "Request Progress in TCountRelCostEst called " << endl;
    
    if ( qp->IsObjectNode(sonOfCount) )
    {
      return CANCEL;
    }
    else
    {
      if ( qp->RequestProgress(sonOfCount, &p1) )
      {
        pRes->Copy(p1);
        return YIELD;
      } else {
        return CANCEL;
      }
    }
  }

/*
9.0.1 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
  }

private:
};

/**
10.0 The class ~TCountStreamCostEstimation~ provides cost estimation
    capabilities for the operator count(stream(tuple))

*/
class TCountStreamCostEstimation : public CostEstimation 
{

public:
    TCountStreamCostEstimation()
    {   
    }   

  virtual ~TCountStreamCostEstimation() {}; 

  virtual int requestProgress(Word* args, ProgressInfo* pRes, void* localInfo,
    bool argsAvialable) {
    
    ProgressInfo p1;
    
   // cout << "Request Progress in TCountStreamCostEst called " << endl;
    
    if (qp->RequestProgress(args[0].addr, &p1) )
    {
      pRes->Copy(p1);
      return YIELD;
    } else {
      return CANCEL;
    }   
}

/*
10.0.1 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
  }

private:
};

#endif
