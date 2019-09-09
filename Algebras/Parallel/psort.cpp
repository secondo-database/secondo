/*
----
This file is part of SECONDO.

Copyright (C) 2019, 
University in Hagen, 
Faculty of Mathematics and Computer Science,
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


#include <string>
#include <boost/thread.hpp>

#include "Algebras/Stream/Stream.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "RTuple.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include "Algebras/ExtRelation-2/TupleSmaller.h"
#include "Algebras/ExtRelation-2/OutHeap.h"

#include "SyncStream.h"


extern NestedList* nl;
extern QueryProcessor* qp; 

namespace parallelalg {

/*
Type Mapping for psort

Arguments are the input stream, the number of threads as well as the
size of the input buffers for each thread. Optionally, the sorting 
order (ASC, DESC) can be given. Deafult ist Ascending. 

*/
ListExpr psortTM(ListExpr args){

    if(!nl->HasLength(args,3) && !nl->HasLength(args,4)){
      return listutils::typeError("3 or 4 arguments expected");
    }
    // stream to sort
    if(!Stream<Tuple>::checkType(nl->First(args))){
      return listutils::typeError("first argument is not a tuple stream");
    }
    // number of threads
    if(!CcInt::checkType(nl->Second(args))){
      return listutils::typeError("second argument is not of type int");
    }
    // size of the input buffer
    if(!CcInt::checkType(nl->Third(args))){
       return listutils::typeError("third argument is not of type int");
    }
    if(nl->HasLength(args,3)){
      return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                                nl->TwoElemList(
                                     nl->BoolAtom(true),
                                     nl->BoolAtom(true)),
                                nl->First(args));
    }
    if(nl->AtomType(nl->Fourth(args)) != SymbolType){
       return listutils::typeError("fourth argument is not a "
                                   "valid sorting direction");
    }
    std::string dir = nl->SymbolValue(nl->Fourth(args));
    if( (dir != "asc") && (dir != "desc")){
       return listutils::typeError("fourth argument is not a "
                                   "valid sorting direction");
    }
    return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                              nl->OneElemList( nl->BoolAtom(dir=="asc")),
                              nl->First(args));
}


template<class TupleCompare>
class FPC{
  public: 
   FPC(TupleCompare _pc): pc(_pc){}

   bool operator()(const std::pair<Tuple*,size_t>& e1, 
                 const std::pair<Tuple*, size_t>& e2){
      return pc(e1.first,e2.first);
   }

  private:
      TupleCompare pc;

};




template<class TupleCompare>
class psortThread{
    public:
        psortThread( SyncStream<Tuple>* _stream, 
                    size_t _inBufferSize,
                    size_t _maxMem, 
                    size_t _maxFiles,
                    TupleType* _tt,
                    TupleCompare& _tc ): 
                       inStream(_stream),
                       inBufferSize(_inBufferSize),
                       maxMem(_maxMem),
                       maxFiles(_maxFiles),
                       tt(_tt),
                       tc(_tc)
                        {
            outHeap = 0;
            inBufferPos = 0;
            tt->IncReference();
            count = 0; 
            runner = new boost::thread(&psortThread<TupleCompare>::run, this);
        }

        ~psortThread(){
            if(runner){
               runner->join();
               delete runner;
            }
            if(outHeap) {
              delete outHeap;
            }
            tt->DeleteIfAllowed();
            if(h1) delete h1;
            if(h2) delete h2;
            std::cout << "thread has processed " << count << " tuples" << endl;
        }

        OutHeap* getOut(){
           runner->join();
           return outHeap;
        }

    private:
        SyncStream<Tuple>* inStream;
        size_t inBufferSize;
        size_t maxMem;
        size_t maxFiles;
        TupleType* tt;
        TupleCompare tc;
        boost::thread* runner;
        OutHeap* outHeap;
        int stage;
        std::vector<TupleFile*> files;
        mmheap::mmheap<Tuple*, TupleSmaller>* h1;
        mmheap::mmheap<Tuple*, TupleSmaller>* h2;
        std::vector<Tuple*> inBuffer;
        size_t inBufferPos;
        size_t count;
 
        void run(){
           partition();
           while( files.size() > maxFiles){
             mergeFiles();
           } 
           if(h2){
             while(!h2->empty()){
                Tuple* t = *h2->min();
                h1->insert(t);
                h2->deleteMin();
             }
            delete h2;
            h2 = 0;
         }
         outHeap = new  OutHeap(files,h1,tc);
        }    

       Tuple*  nextInTuple(){
          if(inBufferPos >= inBuffer.size()){
             inStream->request(inBuffer, inBufferSize);
             inBufferPos = 0;
          }
          Tuple* res = inBuffer[inBufferPos];
          inBufferPos++;
          return res; 
       }


      void partition(){
        // distributes the incoming stream into a set of files
        h1 = new mmheap::mmheap<Tuple*, TupleSmaller>(tc);
        h2 = new mmheap::mmheap<Tuple*, TupleSmaller>(tc);

        size_t h1Mem = 0; // currentlic used memory in h1
        Tuple* nextTuple;
        stage = 1; // 1 means, all incoming tuples are inserted to h1
                   // stage 2 -> filter writing

        TupleFile* currentFile = 0;
        Tuple* lastWritten = 0;
        h1->startBulkload();
        count = 0;

        while( (nextTuple = nextInTuple()) ){
           count++;
           if(stage == 1){
               size_t tm = nextTuple->GetMemSize() + sizeof(void*);
               if( h1Mem + tm <= maxMem ){
                  h1->insert(nextTuple);
                  h1Mem += tm;
               } else {
                  h1->insert(nextTuple);
                  h1->endBulkload();
                  nextTuple = *(h1->min());
                  h1->deleteMin();
                  currentFile = new TupleFile(tt,0);
                  files.push_back(currentFile);
                  currentFile->Append(nextTuple);
                  if(lastWritten){
                      lastWritten->DeleteIfAllowed();
                  }
                  lastWritten = nextTuple;
                  std::swap(h1,h2);
                  h1->startBulkload();
                  stage = 2;
               }
           } else {
              if(tc(nextTuple, lastWritten)){
                // next tuple is smaller, store for next stage
                h1->insert(nextTuple);
              } else {
                // just filter trough heap
                h2->insert(nextTuple);
              }
              if(h2->empty()){
                 h1->endBulkload();
                 std::swap(h1,h2);
                 h1->startBulkload();
                 currentFile->Close();
                 currentFile = new TupleFile(tt,0);
                 files.push_back(currentFile);
              }
              Tuple* min = *(h2->min());
              h2->deleteMin();
              currentFile->Append(min);
                  if(lastWritten){
                      lastWritten->DeleteIfAllowed();
                  }
              lastWritten = min;
           }
        }
        h1->endBulkload();
        if(lastWritten){
            lastWritten->DeleteIfAllowed();
        }
        if(currentFile){
            currentFile->Close();
        }
     }

     void mergeFiles(){
       std::vector<TupleFile*> current;
       size_t reqFiles = (files.size() - maxFiles) + 1;
       size_t numFiles = std::min(maxFiles,reqFiles);
       for(size_t i=0;i<numFiles;i++){
          current.push_back(files[0]);
          files.erase(files.begin());
       }
       TupleFile* out = new TupleFile(tt,0);
       files.push_back(out);
       pc cmp(tc);
       OutHeap oh(current,0,tc);
       Tuple* nt = 0;
       while( (nt = oh.next()) ){
          out->Append(nt);
          nt->DeleteIfAllowed();
       }
       for(size_t i = 0; i< current.size();i++){
          current[i]->Close();
          delete current[i];
       }
     }
};

template<class TupleCompare>
class psortInfo{
    public:
        
       psortInfo(   Word& _stream, 
                    size_t _noThreads,
                    size_t _inBufferSize,
                    size_t _maxMem, 
                    size_t _maxFiles,
                    ListExpr _tt,
                    TupleCompare& _tc ): stream(_stream), outHeap(_tc) {
            
            first = true; 
            size_t maxMem = _maxMem / _noThreads;
            size_t maxFiles = _maxFiles / _noThreads;
            if(maxFiles < 2) {
               maxFiles = 2;
            }
            tt = new TupleType(_tt);
            stream.open();
            for(size_t i = 0 ; i< _noThreads; i++){
                sorter.push_back(
                    new psortThread<TupleCompare>(&stream, _inBufferSize, 
                                 maxMem, maxFiles, tt,_tc));
            } 
            for(size_t i = 0 ; i< _noThreads; i++){
               OutHeap* h = sorter[i]->getOut();
               if(h){
                   outHeaps.push_back(h);
               }
            }
       }


       ~psortInfo(){
          for(size_t i=0; i< sorter.size(); i++){
             delete sorter[i];
          }
          tt->DeleteIfAllowed();
          stream.close();
          deleteOutHeap();
       }

       Tuple* next(){
           if(first){
              initOut();
              first = false;
           }
           if(outHeap.empty()){
             return 0;
           }
           std::pair<Tuple*,size_t> n = *(outHeap.min());
           outHeap.deleteMin();
           Tuple* nt = outHeaps[n.second]->next();
           if(nt){
              outHeap.insert(std::make_pair(nt,n.second));
           }
           return n.first;
       }

    private:
        SyncStream<Tuple> stream;
        mmheap::mmheap<std::pair<Tuple*,size_t>, FPC<TupleCompare> > outHeap;
        TupleType* tt;
        std::vector<psortThread<TupleCompare>*> sorter;
        std::vector<OutHeap*>  outHeaps;
        bool first;
       

     void deleteOutHeap(){
         while(!outHeap.empty()){
             outHeap.min()->first->DeleteIfAllowed();
             outHeap.deleteMin();
         }
     }

     void initOut(){
        for(size_t i=0;i<outHeaps.size(); i++){
           Tuple* t = outHeaps[i]->next();
           if(t){
              outHeap.insert(std::make_pair(t,i));
           }
        }
     }
};



int psortVM(Word* args, Word& result,
           int message, Word& local, Supplier s){

   psortInfo<TupleSmaller>* li = (psortInfo<TupleSmaller>*) local.addr;
   switch(message){
     case OPEN :{if(li){  delete li; }
                 int noAttr = nl->ListLength(nl->Second(
                                           nl->Second(qp->GetType(s))));
                 std::vector<std::pair<int,bool> > cv;
                 bool asc = ((CcBool*)args[4].addr)->GetValue();
                 for(int i=0;i<noAttr;i++){
                    cv.push_back(std::pair<int,bool>(i,asc));
                 } 
                 TupleSmaller ts(cv);
                 size_t noThreads = 4;
                 CcInt* NoThreads = (CcInt*) args[1].addr;
                 if(NoThreads->IsDefined()){
                     noThreads = NoThreads->GetValue();
                     if(noThreads<2) noThreads = 2;
                     if(noThreads>15) noThreads = 15;
                 }
                 size_t inBufferSize = 100;
                 CcInt* InBufferSize = (CcInt*) args[2].addr;
                 if(InBufferSize->IsDefined()){
                     inBufferSize = InBufferSize->GetValue();
                     if(inBufferSize<1) inBufferSize = 1;
                 }
                 size_t mem = qp->GetMemorySize(s)*1024*1024;
                 
                 size_t maxFiles = 256;
                 ListExpr tt = nl->Second(GetTupleResultType(s));
                 local.addr = new psortInfo<TupleSmaller>( args[0], 
                                       noThreads,inBufferSize, mem, 
                                       maxFiles, tt, ts);
                 return 0;
              } 
      case REQUEST : result.addr = li?li->next():0;
                     return result.addr?YIELD:CANCEL;
      case CLOSE: {
                     if(li){
                         delete li;
                         local.addr = 0;
                     }
                     return 0;
                  }
   }
   return -1;
}


OperatorSpec psortSpec(
   "stream(tuple) x int x int x {asc,desc} -> stream(tuple)",
   "_ psort[_,_] ",
   "Sorts a stream of tuples. The first argument is the tuple stream, "
   "the second argument is the num,ber of threads (2-15), the third "
   " argument is the size of the input buffer for each thead. ",
   " query plz psort[6,100, desc] count ");

Operator psortOp(
   "psort",
   psortSpec.getStr(),
   psortVM,
   Operator::SimpleSelect,
   psortTM
);

/*
2. Operator psortby

*/
ListExpr psortbyTM(ListExpr args){
  if(!nl->HasLength(args,4)){
    return listutils::typeError("invalid number of arguments");
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
    return listutils::typeError("First argument is not a tuple stream");
  }
  if(!CcInt::checkType(nl->Second(args))){
    return listutils::typeError("Second argument not of type int");
  }
  if(!CcInt::checkType(nl->Third(args))){
    return listutils::typeError("Third argument not of type int");
  }
  ListExpr orderList = nl->Fourth(args);
  if(nl->AtomType(orderList) != NoAtom){
    return listutils::typeError("fourth argument is not a list");
  }
  std::vector<std::pair<int,bool> > order;
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  while(!nl->IsEmpty(orderList)){
     ListExpr first = nl->First(orderList);
     orderList = nl->Rest(orderList);
     bool asc = true;
     ListExpr attr;
     if(nl->AtomType(first) == SymbolType){
        attr = first;
     } else if(nl->AtomType(first) == NoAtom){
        if(!nl->HasLength(first,2)){
            return listutils::typeError("Elements in the ordering list must "
                                        "be an attribute name optionally "
                                        "followed by  asc or desc"); 
        }
        attr = nl->First(first);
        ListExpr dir = nl->Second(first);
        if(!nl->IsEqual(dir,"asc") && !nl->IsEqual(dir,"desc")){
           return listutils::typeError("invalid order direction, "
                                       "allowed are asc and desc");
        }
        asc = nl->IsEqual(dir,"asc");
     } else {
       return listutils::typeError("Elements in the ordering list must be an "
                                   "attribute name optionally followed by "
                                   " asc or desc"); 
     }
     std::string aname = nl->SymbolValue(attr);
     ListExpr attrType = nl->TheEmptyList();
     int index = listutils::findAttribute(attrList, aname, attrType);
     if(index == 0){
         return listutils::typeError("attribute " + aname 
                                     + " not part of the tuples");
     }
     order.push_back(std::make_pair(index-1, asc));
  }
  ListExpr appendList = nl->OneElemList( nl->IntAtom(order[0].first));
  ListExpr last = appendList;
  last = nl->Append(last, nl->BoolAtom(order[0].second));
  for(size_t i=1; i< order.size(); i++){
     last = nl->Append(last, nl->IntAtom(order[i].first));
     last = nl->Append(last, nl->BoolAtom(order[i].second));
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           appendList,
                           nl->First(args));
}


int psortbyVM(Word* args, Word& result,
           int message, Word& local, Supplier s){

   psortInfo<TupleSmaller>* li = (psortInfo<TupleSmaller>*) local.addr;
   switch(message){
     case OPEN :{if(li){  delete li; }
                 size_t noThreads = 4;
                 CcInt* NoThreads = (CcInt*) args[1].addr;
                 if(NoThreads->IsDefined()){
                     noThreads = NoThreads->GetValue();
                     if(noThreads<2) noThreads = 2;
                     if(noThreads>15) noThreads = 15;
                 }
                 size_t inBufferSize = 100;
                 CcInt* InBufferSize = (CcInt*) args[2].addr;
                 if(InBufferSize->IsDefined()){
                     inBufferSize = InBufferSize->GetValue();
                     if(inBufferSize<1) inBufferSize = 1;
                 }
                 size_t mem = qp->GetMemorySize(s)*1024*1024;
                 size_t maxFiles = 256;
                 // args[3] is the original list
                 int pos = 4;
                 std::vector<std::pair<int,bool> > cv;
                 while(pos < qp->GetNoSons(s)){
                   int index = ((CcInt*) args[pos].addr)->GetValue();
                   pos++;
                   bool asc = ((CcBool*) args[pos].addr)->GetValue();

                   cv.push_back(std::make_pair(index,asc));
                   pos++;
                 }
                 
                 TupleSmaller ts(cv);


                 ListExpr tt = nl->Second(GetTupleResultType(s));
                 local.addr = new psortInfo<TupleSmaller>( args[0], 
                                       noThreads,inBufferSize, mem, 
                                       maxFiles, tt, ts);
                 return 0;
              } 
      case REQUEST : result.addr = li?li->next():0;
                     return result.addr?YIELD:CANCEL;
      case CLOSE: {
                     if(li){
                         delete li;
                         local.addr = 0;
                     }
                     return 0;
                  }
   }
   return -1;
}

OperatorSpec psortbySpec(
   "stream(tuple) x int x int x (IDENT [x {asc, desc}])+  -> stream(tuple)",
   "_ psort[_,_; list] ",
   "Sorts a stream of tuples according to the selected order. "
   "The first argument is the tuple stream, "
   "the second argument is the number of threads (2-15), the third "
   " argument is the size of the input buffer for each thead. "
   "It follows a list consiting of the ordering attributes optionally "
   "followed by the ordering direction for each attribute.",
   " query plz psortby[6,100; PLZ asc, Ort desc] count ");

Operator psortbyOp(
   "psortby",
   psortbySpec.getStr(),
   psortbyVM,
   Operator::SimpleSelect,
   psortbyTM
);





Operator* getPsortOp(){
   return  &psortOp;
}
Operator* getPsortbyOp(){
   return  &psortbyOp;
}

} 



