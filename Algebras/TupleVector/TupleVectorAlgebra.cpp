/*
----
This file is part of SECONDO.

Copyright (C) 2014,
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

#include "Attribute.h"          // implementation of attribute types
#include "Algebra.h"            // definition of the algebra
#include "NestedList.h"         // required at many places
#include "QueryProcessor.h"     // needed for implementing value mappings
#include "AlgebraManager.h"     // e.g., check for a certain kind
#include "Operator.h"           // for operator creation
#include "StandardTypes.h"      // priovides int, real, string, bool type
#include "Algebras/FText/FTextAlgebra.h"
#include "Symbols.h"            // predefined strings
#include "ListUtils.h"          // useful functions for nested lists
#include "Algebras/Stream/Stream.h"  // wrapper for secondo streams

#include "LogMsg.h"             // send error messages

#include "Tools/Flob/DbArray.h"  // use of DbArrays

#include "Algebras/Relation-C++/RelationAlgebra.h" // use of tuples


#include <math.h>               // required for some operators
#include <stack>
#include <limits>

/*
0.5 Global Variables

Secondo uses some variables designed as singleton pattern. For accessing these
global variables, these variables have to be declared to be extern:

*/

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;

namespace tuplevector {



class TupleVector{
 public:
    TupleVector(int _maxSize): content(),maxSize(_maxSize)  {
      if(maxSize <1) maxSize=1;
      content.reserve(maxSize);
    }

    TupleVector(const TupleVector& tv){
        maxSize = tv.maxSize;
        for(size_t i=0;i<tv.content.size();i++){
           Tuple* t = tv.content[i];
           t->IncReference();
           content.push_back(t);
        }    
    }


    ~TupleVector(){
       for(Tuple*& tuple : content){
          tuple->DeleteIfAllowed();
       }
    }
    

    static const string  BasicType(){ return "tuplevector"; }

    static bool checkType(ListExpr t){
       if(!nl->HasLength(t,2)) return false;
       if(!nl->IsEqual(nl->First(t), BasicType())){
           return false;
       }
       return listutils::isAttrList(nl->Second(t));
    }

     
    inline bool isFull() const{
      return content.size() >=maxSize;
    } 

    inline void append(Tuple* tup, bool incref=true){
        if(incref){
           tup->IncReference();
        }
        content.push_back(tup);
    }

    inline size_t numberOfTuples() const{
      return content.size();
    }

    inline size_t getMaxSize() const{
      return maxSize;
    }

    inline Tuple* get(int i) const{
      return content[i];
    }

    inline size_t size() const{
      return content.size();
    }

    inline bool empty() const{
      return content.empty();
    }

    inline void set(size_t i, Tuple* t){
       content[i] = t;
    }

    inline void reduce(size_t s){
       if(s<content.size()){
          content.resize(s);
       }
    }


/*
Secondo specific stuff

*/


    static ListExpr Property() {
        return ( nl->TwoElemList (
          nl->FourElemList (
            nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
            nl->StringAtom("List Rep"),
            nl->StringAtom("Example List")),
          nl->FourElemList (
            nl->StringAtom("-> SIMPLE"),
            nl->StringAtom(BasicType()),
            nl->StringAtom("(int tuple*)"),
            nl->StringAtom("(18000 ( (23 \"Secondo\")))")
      ))); 
    }



    static Word In( const ListExpr typeInfo, const ListExpr instance,
                    const int errorPos, ListExpr& errorInfo, bool& correct ){
       correct = false;
       Word res((void*)0);
       if(!nl->HasLength(instance,2)){
          return res;
       }
       if(nl->AtomType(nl->First(instance))!=IntType){
         return res;
       }
       int size = nl->IntValue(nl->First(instance));
       if(size <0){
          return res;
       }
       TupleVector* tv = new TupleVector(size);
       ListExpr tuples = nl->Second(instance);
       ListExpr tupleList = nl->TwoElemList(
                             listutils::basicSymbol<Tuple>(),
                             nl->Second(typeInfo));

       while(!nl->IsEmpty(tuples)){
          if(tv->isFull()){
             delete tv;
             return res;
          }
          ListExpr tuple = nl->First(tuples);
          tuples = nl->Rest(tuples);
          Tuple* t = Tuple::In(tupleList, tuple, errorPos,errorInfo, correct);
          if(!t){
             delete tv;
             return res;
          }
          tv->append(t, false);
       }
       res.addr = tv;
       correct = true;
       return res; 
    }

    static ListExpr Out(ListExpr typeInfo, Word value){
       TupleVector* tv = (TupleVector*) value.addr;
       ListExpr s = nl->IntAtom(tv->maxSize);
       if(tv->content.empty()){
          return nl->TwoElemList(s, nl->TheEmptyList());
       }
       ListExpr tinfo = nl->TwoElemList(
                              listutils::basicSymbol<Tuple>(),
                              nl->Second(typeInfo));

       ListExpr ts = nl->OneElemList(tv->content[0]->Out(tinfo));
       ListExpr last = ts;
       for(size_t i=1;i<tv->content.size();i++){
         last = nl->Append(last, tv->content[i]->Out(tinfo));
       }
       return nl->TwoElemList(s, ts); 
    }

    static Word Create(ListExpr typeInfo){
      return SetWord(new TupleVector(1));
    }

    static void Delete(ListExpr typeInfo, Word& value){
       delete  (TupleVector*) value.addr;
       value.addr  = 0;
    }

    static bool Open( SmiRecord& valueRecord,
                 size_t& offset, const ListExpr typeInfo,
                 Word& value ) {
        size_t size;
        bool ok = (valueRecord.Read(&size,sizeof(size_t),offset)
                    == sizeof(size_t));
        offset += sizeof(size_t);

        size_t numberOfTuples;
        ok &= valueRecord.Read(&numberOfTuples, sizeof(size_t), offset)
              ==sizeof(size_t);
        offset *= sizeof(size_t);

        if(!ok) return false;

        TupleVector* tv = new TupleVector(size);

        ListExpr tinfo = nl->TwoElemList(
                              listutils::basicSymbol<Tuple>(),
                              nl->Second(typeInfo));
        SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
        ListExpr numType = ctlg->NumericType(tinfo);
        TupleType* tt = new TupleType(numType);       

 

        for(size_t i=0;i<numberOfTuples;i++) {
            size_t tsize;
            valueRecord.Read(&tsize, sizeof(size_t), offset);
            offset += sizeof(size_t);

            char* buffer = new char[tsize];
            valueRecord.Read(buffer, tsize,offset);
            offset+=tsize;
            Tuple* res = new Tuple(tt);
            res->ReadFromBin(0, buffer );
            tv->append(res, false);
            delete[] buffer;
        }
        tt->DeleteIfAllowed();
        value.addr = tv;
        return true;        
    }

    static bool Save( SmiRecord& valueRecord, 
                             size_t& offset,
                             const ListExpr typeInfo, Word& value ) {
       /*
        ListExpr tinfo = nl->TwoElemList(
                              listutils::basicSymbol<Tuple>(),
                              nl->Second(typeInfo));
        */
        TupleVector* tv = (TupleVector*) value.addr;
        size_t s = tv->getMaxSize();
        valueRecord.Write(&s,sizeof(size_t), offset);
        offset += sizeof(size_t);
        s = tv->numberOfTuples();
        valueRecord.Write(&s,sizeof(size_t), offset);
        offset += sizeof(size_t);
        size_t coreSize;
        size_t extensionSize;
        size_t flobSize;

        for(size_t i=0;i<s;i++){
            Tuple* tuple = tv->get(i);
            size_t blocksize = tuple->GetBlockSize(coreSize, extensionSize, 
                                        flobSize);
            char* buffer = new char[blocksize];
            tuple->WriteToBin(buffer, coreSize, extensionSize, flobSize); 
            valueRecord.Write(&blocksize,sizeof(size_t),offset);
            offset+=sizeof(size_t);
            valueRecord.Write(buffer,blocksize,offset);
            offset +=blocksize;
            delete[] buffer;
        }
        return true;
     }

     static void Close( const ListExpr typeInfo, Word& w ) {
         delete (TupleVector*) w.addr;
         w.addr = 0;
     }

     static Word Clone( const ListExpr typeInfo, const Word& w ){
        return Word(new TupleVector( * ((TupleVector*)w.addr)));
     }

     static void*  Cast( void* addr ) {
        return new (addr)TupleVector;
     }

     static bool TypeCheck(ListExpr type, ListExpr& errorInfo){
        return checkType(type);
     }

     static int SizeOf() {
        return 3000;
    }




 private:
   TupleVector() {}
   vector<Tuple*> content;
   //size_t size;
   size_t maxSize;
};

TypeConstructor TupleVectorTC(
   TupleVector::BasicType(),        // name of the type
   TupleVector::Property,             // property function
   TupleVector::Out, TupleVector::In,        // out and in function
   0, 0,                       // deprecated, don't think about it
   TupleVector::Create, TupleVector::Delete, // creation and deletion 
   TupleVector::Open, TupleVector::Save,     // open and save functions
   TupleVector::Close, TupleVector::Clone,   // close and clone functions
   TupleVector::Cast,                 // cast function
   TupleVector::SizeOf,               // sizeOf function
   TupleVector::TypeCheck);           // type checking function




ListExpr feedvTM(ListExpr args){
  // relation x size
  if(!nl->HasLength(args,2)){
     return listutils::typeError("2 arguments expected");
  }
  if(!Relation::checkType(nl->First(args))){
     return listutils::typeError("first argument has to be a relation");
  }
  if(!CcInt::checkType(nl->Second(args))){
     return listutils::typeError("second arg must be an integer");
  }
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  return nl->TwoElemList(
              listutils::basicSymbol<Stream<TupleVector> >(),
              nl->TwoElemList( listutils::basicSymbol<TupleVector>(), 
                               attrList));
}


class feedvInfo{
  public:
     feedvInfo(Relation* _rel, int _size) : size(_size) {
        iter = _rel->MakeScan();
        //size = size * 1024; // in kB
     }
     ~feedvInfo(){
       delete iter;
     }

     TupleVector* next(){
        Tuple* tup = iter->GetNextTuple();
        if(tup==0){
           return 0; 
        } 
        TupleVector* res =   new TupleVector(size);
        res->append(tup,false);
        while( !res->isFull() && ((tup=iter->GetNextTuple())!=0)  ) {
           res->append(tup,false);
        }
        return res;
     }

  private:
     int size;
     GenericRelationIterator* iter;
};

int feedvVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){

  feedvInfo* li = (feedvInfo*) local.addr; 
  switch(message){
    case OPEN: {
                if(li){
                  delete li;
                }
                CcInt* sizeS = (CcInt*) args[1].addr;
                int size = sizeS->IsDefined()?sizeS->GetValue():0;
                local.addr = new feedvInfo((Relation*) args[0].addr,size);
                return 0;
              }
    case REQUEST: result.addr = li?li->next():0;
                  return result.addr?YIELD:CANCEL;
    case CLOSE : if(li){
                   delete li;
                   local.addr = 0; 
                 }
                 return 0;
                                               

  }
  // invalid message
  return -1;
}

OperatorSpec feedvSpec(
  "rel x int -> stream(tuplev)",
  "_ feedv[_]",
  "Transfers a relation into a stream of vectors of tuples."
  "The second argument specifies the size that can be "
  "used by a single element of the stream. ",
  "query ten feedv[23]"
);

Operator feedvOp(
  "feedv",
  feedvSpec.getStr(),
  feedvVM,
  Operator::SimpleSelect,
  feedvTM
);


/*
1.2 Operator count

*/
ListExpr countTM(ListExpr args){
   if(!nl->HasLength(args,1)){
     return listutils::typeError("one arg expected");
   }
   if(!Stream<TupleVector>::checkType(nl->First(args))){
     return listutils::typeError("stream(tuplevector(X)) expected");
   }
   return listutils::basicSymbol<CcInt>();
}

template<bool blockcount>
int countVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){
  Stream<TupleVector> stream(args[0]);
  stream.open();
  TupleVector* elem;
  int count = 0;
  while( (elem = stream.request())){
     if(blockcount){
        count++;
     } else {
        count += elem->numberOfTuples();
     }
     delete elem;
  }
  stream.close();
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true,count);
  return 0;
}


OperatorSpec countSpec(
"stream(tuplevector) -> int",
"_ count",
"Returns the number of tuples within a stream of tuplevectors",
"query ten feedv[23] count"
);

Operator countOp(
  "count",
  countSpec.getStr(),
  countVM<false>,
  Operator::SimpleSelect,
  countTM
);

OperatorSpec countvSpec(
"stream(tuplevector) -> int",
"_ countv",
"Returns the number of vectors in a  stream of tuplevectors",
"query ten feedv[23] countv"
);


Operator countvOp(
  "countv",
  countvSpec.getStr(),
  countVM<true>,
  Operator::SimpleSelect,
  countTM
);




/*
1.3 feednp

feed without any progress

*/
ListExpr feednpTM(ListExpr args){
  if(!nl->HasLength(args,1)){
     return listutils::typeError("1 argument expected");
  }
  if(!Relation::checkType(nl->First(args))){
     return listutils::typeError("first argument has to be a relation");
  }
  ListExpr tupleList = nl->Second(nl->First(args));
  return nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(), tupleList);
  
}


int feednpVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){

   GenericRelationIterator* li = (GenericRelationIterator*) local.addr;
   switch(message){
      case OPEN: if(li) delete li;
                 local.addr = ((Relation*)args[0].addr)->MakeScan();
                 return 0;
      case REQUEST: result.addr = li?li->GetNextTuple():0;
                    return result.addr?YIELD:CANCEL;
      case CLOSE: if(li){
                    delete li;
                    local.addr = 0;
                  }
                  return 0;
   }
   return -1;
}

OperatorSpec feednpSpec(
   "rel(X) -> stream(X)",
   "_ feednp",
   "Feed without any progress support.",
   "query ten feednp count"
);

Operator feednpOp(
   "feednp",
    feednpSpec.getStr(),
    feednpVM,
    Operator::SimpleSelect,
    feednpTM
);


/*
countnp

*/

ListExpr countnpTM(ListExpr args){
   if(!nl->HasLength(args,1)){
     return listutils::typeError("on argument expected");
   }
   if(!Stream<Tuple>::checkType(nl->First(args))){
     return listutils::typeError("stream(tuple(X)) expected");
   }
   return listutils::basicSymbol<CcInt>();
}

int countnpVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){
  Stream<Tuple> stream(args[0]);
  stream.open();
  Tuple* elem;
  int count = 0;
  while( (elem = stream.request())){
     count++;
     elem->DeleteIfAllowed();
  }
  stream.close();
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true,count);
  return 0;
}


OperatorSpec countnpSpec(
"stream(tuple) -> int",
"_ countnp",
"Returns the number of tuples within a stream. ",
"query ten feed countnp"
);

Operator countnpOp(
  "countnp",
  countnpSpec.getStr(),
  countnpVM,
  Operator::SimpleSelect,
  countnpTM
);

/*
1.4 projectv

*/
template<class ST>
ListExpr projectvTM(ListExpr args){
   if(!nl->HasMinLength(args,2)){
     return listutils::typeError("at least 2 arguments expected");
   }
   if(!Stream<ST>::checkType(nl->First(args))){
     return listutils::typeError("The first argument must be a stream of " 
                                 + ST::BasicType() );
   }
   ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
   args = nl->Rest(args);
   ListExpr appendList = nl->TheEmptyList();
   ListExpr appendLast = nl->TheEmptyList();
   bool firstE = true;
   ListExpr resList = nl->TheEmptyList();
   ListExpr resLast = nl->TheEmptyList();

   while(!nl->IsEmpty(args)){
     ListExpr first = nl->First(args);
     args = nl->Rest(args);
     if(!listutils::isSymbol(first)){
       return listutils::typeError("invalid attribute name detected");
     }
     string name = nl->SymbolValue(first);
     ListExpr attrType;
     int pos = listutils::findAttribute(attrList,name,attrType);
     if(!pos){
        return listutils::typeError(name + " is not an attribute of the tuple");
     }
     ListExpr attr = nl->TwoElemList(first, attrType);
     ListExpr p = nl->IntAtom(pos-1);
     if(firstE){
        appendList = nl->OneElemList(p);
        appendLast = appendList;
        resList = nl->OneElemList(attr);
        resLast = resList;
        firstE = false; 
     } else {
        appendLast = nl->Append(appendLast, p);
        resLast = nl->Append(resLast, attr);
     }
   }
   resList = nl->TwoElemList( listutils::basicSymbol<ST>(),
                          resList);
   resList = nl->TwoElemList( listutils::basicSymbol<Stream<ST> >(),
                              resList);
   return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                             appendList,
                             resList);
}


template<class ST>
class projectvInfo{
  public:
     projectvInfo(Word _stream, vector<int>* _projectList, TupleType* _tt):
       stream(_stream), projectList(_projectList), tt(_tt){
        stream.open();
     }
     ~projectvInfo(){
        stream.close();
     }

     inline ST* next(){
      return next(stream);
     }


  private:
     Stream<ST> stream;
     vector<int>* projectList;
     TupleType* tt;
     
      TupleVector* next( Stream<TupleVector>&){
        TupleVector* in = stream.request();
        if(in==0){
          return 0;
        }
        return project(in);
     }

     Tuple* next(Stream<Tuple>&){
        Tuple* in = stream.request();
        if(!in) return 0;
        Tuple* res = project(in);
        in->DeleteIfAllowed();
        return res;
     }

     TupleVector* project(TupleVector* tv){
        for(size_t i = 0; i<tv->size(); i++){
            Tuple* ti = tv->get(i);
            Tuple* tr = project(ti);
            tv->set(i,tr);
            ti->DeleteIfAllowed();
        }
        return tv;
     }

     Tuple* project(Tuple* t){
        Tuple* res = new Tuple(tt);
        for(size_t i=0; i<projectList->size();i++){
           res->CopyAttribute(projectList->at(i), t,i);
        }
        return res;
     }
};

template<class ST>
int projectvVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){

  projectvInfo<ST>* li = (projectvInfo<ST>*) local.addr;
  typedef pair<vector<int>*, TupleType*> globalInfo;
  switch(message){
     case INIT : {
        return 0;
     }    
     case FINISH: {
        globalInfo* gi = (globalInfo*) qp->GetLocal2(s).addr;
        if(gi){
           delete gi->first;
           gi->second->DeleteIfAllowed();
           delete gi;
           qp->GetLocal2(s).addr=0; 
        }    
        return 0;
     }  
     case OPEN: {
                globalInfo* gi = (globalInfo*) qp->GetLocal2(s).addr;
                if(!gi){
                  TupleType* tt = new 
                                  TupleType(nl->Second(GetTupleResultType(s)));
                  int na = qp->GetNoSons(s);
                  int start = (na - 1)/2 +1;
                  vector<int>* pl = new vector<int>(); 
                  for(int i=start ; i< na; i++){
                     pl->push_back(((CcInt*)args[i].addr)->GetValue());
                  }
                  gi = new globalInfo(pl,tt);
                  qp->GetLocal2(s).addr = gi;
                }

                if(li){
                  delete li;
                }
                local.addr = new 
                             projectvInfo<ST>(args[0],gi->first, gi->second);
                return 0;
                }
      case REQUEST: result.addr= li?li->next():0;
                    return result.addr?YIELD:CANCEL;
      case CLOSE: {
                    if(li){
                      delete li;
                      local.addr = 0;
                    }
                 }
                 return 0;
     }

    return -1; 
}

OperatorSpec projectvSpec(
   "stream(tuplevector) x IDENT+ -> stream(tuplevector)",
   "_ projectv[_] ",
   "projects the tuples to the given attributes",
   "query plz feedv[200] projectv[PLZ] count"
);

Operator projectvOp(
   "projectv",
   projectvSpec.getStr(),
   projectvVM<TupleVector>,
   Operator::SimpleSelect,
   projectvTM<TupleVector>
);


OperatorSpec projectnpSpec(
   "stream(tuple) x IDENT+ -> stream(tuple)",
   "_ projectnp[_] ",
   "projects the tuples to the given attributes",
   "query plz feed projectnp[PLZ] count"
);

Operator projectnpOp(
   "projectnp",
   projectnpSpec.getStr(),
   projectvVM<Tuple>,
   Operator::SimpleSelect,
   projectvTM<Tuple>
);



/*
1.6 consume

*/
ListExpr consumeTM(ListExpr args){
  if(!nl->HasLength(args,1)){
     return listutils::typeError("one argument expected");
  }
  ListExpr arg = nl->First(args);
  if(!Stream<TupleVector>::checkType(arg)){
    return listutils::typeError("stream(tuplevector) expected");
  }
  ListExpr attrList = nl->Second(nl->Second(arg));
  return nl->TwoElemList( listutils::basicSymbol<Relation>(),
                 nl->TwoElemList( listutils::basicSymbol<Tuple>(),
                                  attrList));
}

int consumeVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){

   GenericRelation* rel = (GenericRelation*)((qp->ResultStorage(s)).addr);
   if(rel->GetNoTuples() > 0)
   {
     rel->Clear();
   }
   Stream<TupleVector> stream(args[0]);
   stream.open();
   TupleVector* tv;
   while( (tv = stream.request())){
      for(size_t i=0;i<tv->size();i++){
         rel->AppendTuple(tv->get(i));
      }
      delete tv;
   }
   stream.close();
   result.addr = rel;
   return 0;
}

OperatorSpec consumeSpec(
  "stream(tuplevector) -> rel",
  "_ consume",
  "Collects the incoming tuples into a relation",
  " query plz feedv[200] projectv[PLZ] consume"
);


Operator consumeOp(
  "consume",
   consumeSpec.getStr(),
   consumeVM,
   Operator::SimpleSelect,
   consumeTM
);

/*
1.7 TypeMapOperator TVS2T

  TupleVectorStream to Tuple

*/
ListExpr TVS2TTM(ListExpr args){
  if(!nl->HasMinLength(args,1)){
    return listutils::typeError("at least one argument expected");
  }
  ListExpr arg = nl->First(args);
  if(!Stream<TupleVector>::checkType(arg)){
    return listutils::typeError("stream of tuplevectors expected");
  } 
  ListExpr attrList = nl->Second(nl->Second(arg));
  return nl->TwoElemList(listutils::basicSymbol<Tuple>(),attrList);
}

OperatorSpec TVS2TSpec(
  "stream(tuplevector(X)) x ... -> tuple(X)",
  "op(_,_)",
  "Retrieves the tuple type of a stream of tuple vectors",
  "TypeMapping Operator for internal usage only." 
);

Operator TVS2TOp(
   "TVS2T",
   TVS2TSpec.getStr(),
   0,
   Operator::SimpleSelect,
   TVS2TTM
);



/*
1.8 Operator filterv

*/
template<class ST, int argnum>
ListExpr filtervTM(ListExpr args){
  if(!nl->HasLength(args,argnum)){
    return listutils::typeError("invalid number of elements");
  }
  if(!Stream<ST>::checkType(nl->First(args))){
     return listutils::typeError("first argument must be a stream of "
                                 + ST::BasicType());
  }
  if(!listutils::isMap<1>(nl->Second(args))){
    return listutils::typeError("second argument must be an unary function");
  }
  ListExpr tt = nl->TwoElemList( listutils::basicSymbol<Tuple>(),
                                nl->Second(nl->Second(nl->First(args))));

  ListExpr funarg = nl->Second(nl->Second(args));
  if(!nl->Equal(funarg,tt)){
    return listutils::typeError("function argument and tuple "
                                "type in stream differ");
  }
  if(!CcBool::checkType(nl->Third(nl->Second(args)))){
     return listutils::typeError("function result is not of type bool");
  }

  if(argnum == 2){
    return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                              nl->OneElemList(nl->BoolAtom(false)),
                              nl->First(args));
  }
  if(!CcBool::checkType(nl->Third(args))){
     return listutils::typeError("third argument must be of type bool");
  }
  return nl->First(args);
}


template<class ST>
class filtervInfo{
 public :
   filtervInfo(Word _stream, Supplier _fun, bool _allowShrink):
         stream(_stream), fun(_fun), allowShrink(_allowShrink){
       stream.open(),
       argv = qp->Argument(fun);
       lastTV = 0;
       lastPos = 0;
       maxSize = 0;
   }

   ~filtervInfo(){
      stream.close();
      if(lastTV){
        while(lastPos < lastTV->size()){
          Tuple* tup = lastTV->get(lastPos);
          lastPos++;
          tup->DeleteIfAllowed();
        }
        lastTV->reduce(0);
        delete lastTV;
      }
    }

    
    inline TupleVector* nextResult( Stream<TupleVector>&){
       if(allowShrink){
          return nextSimple();
       } else {
          return nextFillUp();
       }
     }

     inline Tuple* nextResult( Stream<Tuple>&){
        Tuple* tup;
        while( (tup=stream.request())){
           if(accept(tup)){
              return tup;
           } else {
              tup->DeleteIfAllowed();
           }
        }
        return 0;
     }


     ST* next(){
       return nextResult(stream);
     }




   private:
      Stream<ST> stream;
      Supplier fun;
      bool allowShrink;
      ArgVectorPointer argv;
      size_t maxSize;
      Word funRes;
      TupleVector* lastTV;
      size_t   lastPos;


    TupleVector* nextSimple(){
       TupleVector* in;
       while( (in = stream.request())){
         int writePos = 0;
         for(size_t i=0;i<in->size();i++){
           if(accept(in->get(i))){
              in->set(writePos, in->get(i));
              writePos++;       
           } else {
              in->get(i)->DeleteIfAllowed();
           }
         }
         if(writePos>0){ // in is nonempty
            in->reduce(writePos);
            return in;
         } else {
            in->reduce(0);
            delete in;
         } 
       }
       return 0;
    }

    bool accept(Tuple* t){
      (*argv)[0] = t;
      qp->Request(fun,funRes);
      CcBool* r = (CcBool*) funRes.addr;
      return r->IsDefined() && r->GetValue();
    }

   TupleVector* nextFillUp(){
      if(maxSize==0) { // first call
        lastTV = stream.request();
        lastPos = 0;
        if(!lastTV){
           return 0;
        }  
        maxSize = lastTV->getMaxSize();
        if(maxSize==0) maxSize=1;
      }
      if(!lastTV) {
         return 0;
      }
      return fillUp();
   }

   TupleVector* fillUp(){
      TupleVector* res = new TupleVector(maxSize);
      while(true){
          while(lastPos < lastTV->size()){
              Tuple* tup = lastTV->get(lastPos);
              lastPos++;
              if(accept(tup)){
                  res->append(tup,false);
                  if(res->isFull()){
                      return res;
                  }
              } else {
                  tup->DeleteIfAllowed();
              }
          }
          lastTV->reduce(0);
          delete lastTV; 
          lastTV = stream.request();
          lastPos = 0;
         
          if(!lastTV){
             if(!res->empty()){
                return res;
             } else {
                delete res;
                return 0;
             }
          }
      }
   }
};


template<class ST>
int filtervVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){

  filtervInfo<ST>* li = (filtervInfo<ST>*) local.addr;
  switch(message){
    case OPEN : { 
                if(li){
                 delete li;
                }
                CcBool* as1 = (CcBool*) args[2].addr;
                bool as = as1->IsDefined() && as1->GetValue();
                local.addr = new filtervInfo<ST>(args[0],args[1].addr,as);
                return 0;
               }
     case REQUEST: result.addr = li?li->next():0;
                   return result.addr?YIELD:CANCEL;
     case CLOSE : if(li){
                     delete li;
                     local.addr = 0;
                  }
                 return 0;
  }
  return -1;
}

OperatorSpec filtervSpec(
  "stream(tuplevector(X)) x (fun(tuple(X)->bool) "
  "x bool -> stream(tuplevector(X))",
  "_ filterv[fun,_]",
  "removes such tuples in the stream that not fulfill the condition ",
  "The boolean argument specifies whether is is allowed to "
  "transfer non filled tuplevectors.",
  "query plz feedv filterv[ . PLZ < 1900, TRUE] count"
);

Operator filtervOp(
  "filterv",
  filtervSpec.getStr(),
  filtervVM<TupleVector>,
  Operator::SimpleSelect,
  filtervTM<TupleVector,3>
);


OperatorSpec filternpSpec(
   "(stream(tuple) x (tuple->bool) -> stream(tuple)",
   " _ filternp[fun] ",
   " Removes all tuples from a stream not fulfilling a given condition."
   " Supports no progress.",
   " query plz feed filternp[.PLZ < 3000] count"
);

Operator filternpOp(
  "filternp",
  filternpSpec.getStr(),
  filtervVM<Tuple>,
  Operator::SimpleSelect,
  filtervTM<Tuple,2>
);


/*
1.10 Conversion into a tuple stream

*/
ListExpr tvs2tsTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("1 argument expected");
  }
  if(!Stream<TupleVector>::checkType(nl->First(args))){
    return listutils::typeError("expected a stream of tuple vectors");
  }
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  return nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> > (),
                          nl->TwoElemList(listutils::basicSymbol<Tuple>(),
                                          attrList));
}

class tvs2tsInfo{

  public:
    tvs2tsInfo( Word _stream ): stream(_stream), lastPos(0){
       stream.open();
       lastTV = stream.request();
    }
    ~tvs2tsInfo(){
       stream.close();
       if(lastTV){
          for(size_t i = lastPos; i < lastTV->size();i++){
             lastTV->get(i)->DeleteIfAllowed();
          }
          lastTV->reduce(0);
          delete lastTV;
       }
     }

     Tuple* next(){
       while( lastTV ){
          if(lastPos < lastTV->size()){
             Tuple* res = lastTV->get(lastPos);
             lastPos++;
             return res;
          }
          lastTV->reduce(0);
          delete lastTV;
          lastTV = stream.request();
          lastPos = 0;
       }
       return 0;
     }

  private:
     Stream<TupleVector> stream;
     size_t lastPos;
     TupleVector* lastTV;
};


int tvs2tsVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){

  tvs2tsInfo* li = (tvs2tsInfo*) local.addr;
  switch(message){
    case OPEN : { 
                if(li){
                 delete li;
                }
                local.addr = new tvs2tsInfo(args[0]);
                return 0;
               }
     case REQUEST: result.addr = li?li->next():0;
                   return result.addr?YIELD:CANCEL;
     case CLOSE : if(li){
                     delete li;
                     local.addr = 0;
                  }
                 return 0;
  }
  return -1;
}

OperatorSpec tvs2tsSpec(
  "stream(tuplevector) -> stream(tuple) ",
  " _ tvs2ts",
  " Converts a stream of tuple vectors into a stream of tuples",
  " query plz feedv[200] tvs2ts count"
);

Operator tvs2tsOp(
  "tvs2ts",
  tvs2tsSpec.getStr(),
  tvs2tsVM,
  Operator::SimpleSelect,
  tvs2tsTM
);

/*
2.21 Operator ts2tvs

*/
ListExpr ts2tvsTM(ListExpr args){
  if(!nl->HasLength(args,2)){
     return listutils::typeError("two arguments expected");
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
     return listutils::typeError("first argument has to be a tuple stream");
  }
  if(!CcInt::checkType(nl->Second(args))){
     return listutils::typeError("second arg must be an integer");
  }
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  return nl->TwoElemList( listutils::basicSymbol<Stream<TupleVector> >(),
                          nl->TwoElemList(
                             listutils::basicSymbol<TupleVector>(),
                             attrList));
}

class ts2tvsInfo{
  public:
    ts2tvsInfo(Word _stream, int _maxSize): stream(_stream), maxSize(_maxSize){
       stream.open();
    }
    ~ts2tvsInfo(){
       stream.close();
    }
 
    TupleVector* next(){
       Tuple* tup = stream.request();
       if(!tup){
         return 0;
       }
       TupleVector* res = new TupleVector(maxSize);
       res->append(tup,false);
       while(!res->isFull() && ((tup=stream.request())!=0)){
         res->append(tup,false);
       }
       return res;
    }    

    private:
      Stream<Tuple> stream;
      size_t maxSize;
};


int ts2tvsVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){
    ts2tvsInfo* li = (ts2tvsInfo*) local.addr;
    switch(message){
 
       case OPEN: {
                    if(li){
                      delete li;
                      local.addr = 0;
                    }
                    CcInt* cSize = (CcInt*) args[1].addr;
                    int size = cSize->IsDefined()?cSize->GetValue():0;
                    if(size > 0){
                      local.addr = new ts2tvsInfo(args[0],size);
                    }
                    return 0;
                  }
     case REQUEST: result.addr = li?li->next():0;
                   return result.addr?YIELD:CANCEL;
     case CLOSE : if(li){
                     delete li;
                     local.addr = 0;
                  }
                 return 0;
    }
    return -1;
}

OperatorSpec ts2tvsSpec(
  "stream(tuple) x int -> stream(tuplevector)",
  "_ ts2tvs[_] ",
  "Converts a stream of tuples into a stream of tuplevectors "
  "having a specified size",
  " query plz feed ts2tvs[200] count"
);

Operator ts2tvsOp(
  "ts2tvs",
  ts2tvsSpec.getStr(),
  ts2tvsVM,
  Operator::SimpleSelect,
  ts2tvsTM
);

/*
2.11 Operator ~extendv~

StreamType may be Tuple or TupleVector

*/
template<class StreamType>
ListExpr extendvTM(ListExpr args){

  if(!nl->HasLength(args,2)){
    return listutils::typeError("stream("+ StreamType::BasicType() 
                                + ") x funlist expected");
  }
  if(!Stream<StreamType>::checkType(nl->First(args))){
    return listutils::typeError("first arg is not a stream of " 
                                + StreamType::BasicType());
  }
  if(nl->AtomType(nl->Second(args))!=NoAtom){
    return listutils::typeError("second arg is not a (fun) list");
  }
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));

  // copy old attribute list
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  ListExpr last = newAttrList;
  ListExpr rest = nl->Rest(attrList);
  while(!nl->IsEmpty(rest)){
    last = nl->Append(last, nl->First(rest));
    rest = nl->Rest(rest);
  }
  ListExpr tupleType = nl->TwoElemList( listutils::basicSymbol<Tuple>(),
                                        attrList);

  rest = nl->Second(args);
  set<string> used;
  while(!nl->IsEmpty(rest)){
    ListExpr first = nl->First(rest);
    rest = nl->Rest(rest);
    if(!nl->HasLength(first,2)){
      return listutils::typeError("invalid named function found");
    } 
    ListExpr nameL = nl->First(first);
    if(nl->AtomType(nameL) != SymbolType){
      return listutils::typeError("invalid name for an attribute: " 
                                  + nl->ToString(nameL));
    }
    string name = nl->SymbolValue(nameL);
    ListExpr attrType;
    if(listutils::findAttribute(attrList, name, attrType)!=0){
      return listutils::typeError("attribute " + name 
                                  + " already present in original tuple");
    }
    if(used.find(name)!=used.end()){
      return listutils::typeError("attributename " + name + " used twice");
    }
    used.insert(name);
    ListExpr fun = nl->Second(first);
    if(!listutils::isMap<1>(fun)){
      return listutils::typeError("definition for " + name 
                                  + " is not an unary function");
    }
    if(!nl->Equal(tupleType,nl->Second(fun))){
      return listutils::typeError("tuple type and function argument type for " 
                                  + name + " differ");
    }
    attrType = nl->Third(fun);
    if(!Attribute::checkType(attrType)){
      return listutils::typeError("function result for " 
                                   + name + " is not an attribute");
    }
    ListExpr l = nl->TwoElemList(nameL, attrType);
    last = nl->Append(last,l);
  }
  ListExpr resType = nl->TwoElemList( 
                          listutils::basicSymbol<Stream<StreamType> >(),
                          nl->TwoElemList(listutils::basicSymbol<StreamType>(),
                                          newAttrList));
  ListExpr appendList = nl->OneElemList(nl->IntAtom(0));
  last = appendList;
  int length = nl->ListLength(attrList);
  for(int i=1;i<length;i++){
     last = nl->Append(last, nl->IntAtom(i));
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           appendList,
                           resType);


}

template<class StreamType>
class projectExtendInfo{
  public:
 
     projectExtendInfo(Word _stream, vector<int>& _projectionList, 
                       vector<Supplier>& _functions, TupleType* _tt):
                       stream(_stream), projectionList(_projectionList),
                       functions(_functions), tt(_tt){
       stream.open();
       tt->IncReference();
       for(size_t i=0;i<functions.size();i++){
          funargs.push_back(qp->Argument(functions[i]));
       }
    }

    ~projectExtendInfo(){
        tt->DeleteIfAllowed();
        stream.close();
    }

    StreamType* next(){
      StreamType* in = stream.request();
      return in?projectextend(in):0;
    }


    private:
      Stream<StreamType> stream;
      vector<int> projectionList;
      vector<Supplier> functions;
      TupleType* tt;
      vector<ArgVectorPointer> funargs;
      Word funres;

    TupleVector* projectextend(TupleVector* in){
       for(size_t i=0;i<in->size();i++){
          Tuple* it = in->get(i);
          Tuple* ot = projectextend(it);
          in->set(i,ot);
       }
       return in;
    }

    Tuple* projectextend(Tuple* in){

       Tuple* out = new Tuple(tt);
       // projectionList
       for(size_t i=0;i<projectionList.size(); i++){
         out->CopyAttribute(projectionList[i],in,i);
       }
       for(size_t i = 0; i<functions.size(); i++){
          (*funargs[i])[0] = in;
          qp->Request(functions[i],funres);
          out->PutAttribute(projectionList.size()+i, 
                          ((Attribute*) funres.addr)->Clone());
       }
       in->DeleteIfAllowed();
       return out;
    }
};


struct projectExtendGlobalInfo{
  projectExtendGlobalInfo(){
    tt = 0;
    projectList = 0;
    funList = 0;
  }

  ~projectExtendGlobalInfo(){
     if(tt) tt->DeleteIfAllowed();
     if(projectList) delete projectList;
     if(funList) delete funList;
  }
  
  TupleType* tt;
  vector<int>* projectList;
  vector<Supplier>* funList;
};


template<bool useProject, class StreamType>
int projectextendVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){
  typedef projectExtendInfo<StreamType> localInfo_t;
  localInfo_t* li = (localInfo_t*) local.addr;
  switch(message){
     case INIT: {
         projectExtendGlobalInfo* gi = new projectExtendGlobalInfo();

        ListExpr resType = qp->GetType(s);
        ListExpr attrList = nl->Second(nl->Second(resType));
        ListExpr tupleType = nl->TwoElemList(listutils::basicSymbol<Tuple>(),
                                             attrList);
        ListExpr ttl = SecondoSystem::GetCatalog()->NumericType(tupleType);
        TupleType* tt = new TupleType(ttl);        
        gi->tt = tt;
        qp->GetLocal2(s).addr=gi; 
        return 0;
     } 
     case FINISH: {
         projectExtendGlobalInfo* gi =  (projectExtendGlobalInfo*) 
                                        qp->GetLocal2(s).addr;
         if(gi){
           delete gi;
           qp->GetLocal2(s).addr=0;
         }
        return 0;
     }
     case OPEN: {
         if(li){
            delete li;
            local.addr = 0;
         }
         projectExtendGlobalInfo* gi =  (projectExtendGlobalInfo*) 
                                        qp->GetLocal2(s).addr;
         if(!gi){
            return 0; // should never happen    
         }
         if(!gi->projectList){
            vector<int>* v = new vector<int>();
            int start = useProject?3:2;
            for(int i=start; i<qp->GetNoSons(s);i++){
              int p = ((CcInt*) args[i].addr)->GetValue();
              v->push_back(p);
            } 
            gi->projectList = v;
            Supplier fl = qp->GetSon(s,useProject?2:1);
            vector<Supplier>* funs = new vector<Supplier>();
            for(int i=0;i<qp->GetNoSons(fl);i++){
               Supplier namedFun = qp->GetSon(fl,i);
               Supplier fun = qp->GetSon(namedFun,1);
               funs->push_back(fun);
            }
            gi->funList = funs;
         }
         local.addr = new localInfo_t(args[0], *gi->projectList, 
                                      *gi->funList,gi->tt);
         return 0;
     }
     case REQUEST:
            result.addr = li?li->next():0;
            return result.addr?YIELD:CANCEL;
     case CLOSE:
            if(li){
               delete li;
               local.addr = 0;
            }
            return 0;

  }
  return -1;
}


OperatorSpec extendvSpec(
   "stream(tuplevector) x funlist -> stream(tuplevector)",
   " _ extendv[funlist] ",
   " Extends the tuples within a tuplevector stream "
   " by applying a set of functions to each tuple.",
   " query plz feedv[200] extendv[PLZ2 : .PLZ * 2] consume"
);

Operator extendvOp(
   "extendv",
   extendvSpec.getStr(),
   projectextendVM<false, TupleVector>,
   Operator::SimpleSelect,
   extendvTM<TupleVector>
);

template<class StreamType>
ListExpr projectextendTM(ListExpr args){

  if(!nl->HasLength(args,3)){
    return listutils::typeError("expected stream x projectList, funList");
  }
  if(!Stream<StreamType>::checkType(nl->First(args))){
     return listutils::typeError("first arg is not a stream of "
                                  +StreamType::BasicType());
  }
  ListExpr pList = nl->Second(args);
  if(nl->AtomType(pList) != NoAtom){
    return listutils::typeError("second argument must be a list of "
                                "projection attributes");
  }
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr tupleList = nl->TwoElemList( listutils::basicSymbol<Tuple>(),
                                        attrList);
  ListExpr appendList = nl->TheEmptyList();
  ListExpr lastappend = nl->TheEmptyList();
  ListExpr resAttrList = nl->TheEmptyList();
  ListExpr lastRes = nl->TheEmptyList();

  // process projection list
  bool isFirst = true;
  set<string> used;
  while(!nl->IsEmpty(pList)){
    ListExpr pl = nl->First(pList);
    pList = nl->Rest(pList);
    if(nl->AtomType(pl) !=SymbolType){
      return listutils::typeError("found invalid attribute name in "
                                  "projection list");
    }
    string name = nl->SymbolValue(pl);
    ListExpr attrType;
    int index = listutils::findAttribute(attrList, name, attrType);
    if(!index){
      return listutils::typeError("attribute " + name + " not found");
    }
    ListExpr attr = nl->TwoElemList(pl, attrType);
    if(used.find(name)!=used.end()){
      return listutils::typeError("name " + name + " used twice");
    }
    used.insert(name);
    if(isFirst){
       appendList = nl->OneElemList(nl->IntAtom(index-1));
       lastappend = appendList;
       resAttrList = nl->OneElemList(attr);
       lastRes = resAttrList;
       isFirst = false;
    } else {
       lastappend = nl->Append(lastappend, nl->IntAtom(index-1));
       lastRes = nl->Append(lastRes, attr);   
    }
  }

  // process fun list
  ListExpr flist = nl->Third(args);
  if(nl->AtomType(flist) != NoAtom){
    return listutils::typeError("expected list of named function as third arg");
  }
  while(!nl->IsEmpty(flist)){
    ListExpr nfun = nl->First(flist);
    flist = nl->Rest(flist);
    if(!nl->HasLength(nfun,2)){
      return listutils::typeError("invalid format for a named function");
    }
    ListExpr fnl = nl->First(nfun);
    if(nl->AtomType(fnl)!=SymbolType){
      return listutils::typeError("invalid name for an attribute");
    }
    string fn = nl->SymbolValue(fnl);
    if(used.find(fn)!=used.end()){
      return listutils::typeError("Attribute name " + fn + " used twice");
    } 
    used.insert(fn);
    ListExpr fun = nl->Second(nfun);
    if(!listutils::isMap<1>(fun)){
      return listutils::typeError("definition of attribute " + fn 
                                  + " is not an unary function");
    }
    if(!nl->Equal(tupleList, nl->Second(fun))){
      return listutils::typeError("tuple type and function argument type for "
                                  + fn + "differ");
    }
    ListExpr attrType = nl->Third(fun);
    if(!Attribute::checkType(attrType)){
       return listutils::typeError("result type for " + fn 
                                   + " in not an attribute");
    }
    ListExpr attr = nl->TwoElemList(fnl, attrType);
    if(isFirst){
       resAttrList = nl->OneElemList(attr);
       lastRes = resAttrList;
       isFirst = false;
    } else {
       lastRes = nl->Append(lastRes,attr);
    }
  }
  ListExpr resList = nl->TwoElemList(
                  listutils::basicSymbol<Stream<StreamType> >(),
                  nl->TwoElemList( listutils::basicSymbol<StreamType>(),
                                   resAttrList));

  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           appendList,
                           resList);

}

OperatorSpec projectextendSpec(
  "stream(tuplevector) x projectlist x funList -> stream(tuplevector)",
  "_ projectextend[plist; fList]",
  "projects incoming tuples to a subset of their attributes and "
  "extends this tuple using some named functions.",
  "query plz feedv[200]  projectextendv[ Ort ; PLZ2 : .PLZ * 2] consume"
);

Operator projectextendOp(
  "projectextendv",
  projectextendSpec.getStr(),
  projectextendVM<true,TupleVector>,
  Operator::SimpleSelect,
  projectextendTM<TupleVector>
);

OperatorSpec extendnpSpec(
   "stream(tuple) x funlist -> stream(tuple)",
   " _ extendnp[funlist] ",
   " Extends the tuples within a tuple stream "
   " by applying a set of functions to each tuple.",
   " query plz feed extendinp[PLZ2 : .PLZ * 2] consume"
);

Operator extendnpOp(
   "extendnp",
   extendnpSpec.getStr(),
   projectextendVM<false, Tuple>,
   Operator::SimpleSelect,
   extendvTM<Tuple>
);


OperatorSpec projectextendnpSpec(
   "stream(tuple) x projectlist x funlist -> stream(tuple)",
   " _ extendnp[projectList;funlist] ",
   " Extends the projected tuples within a tuple stream "
   " by applying a set of functions to each tuple.",
   " query plz feed projectextendnp[Ort;PLZ2 : .PLZ * 2] consume"
);

Operator projectextendnpOp(
   "projectextendnp",
   projectextendnpSpec.getStr(),
   projectextendVM<true, Tuple>,
   Operator::SimpleSelect,
   projectextendTM<Tuple>
);




class TupleVectorAlgebra : public Algebra {
   public:
     TupleVectorAlgebra() : Algebra() {
        AddTypeConstructor(&TupleVectorTC);
        TupleVectorTC.AssociateKind(Kind::SIMPLE());
   
        AddOperator(&feedvOp);
        AddOperator(&countOp);
        AddOperator(&countvOp);
        AddOperator(&projectvOp);
        projectvOp.enableInitFinishSupport();
        AddOperator(&consumeOp);
        AddOperator(&filtervOp);
        AddOperator(&tvs2tsOp);
        AddOperator(&ts2tvsOp);
        AddOperator(&extendvOp);
        extendvOp.enableInitFinishSupport();
        AddOperator(&projectextendOp);
        projectextendOp.enableInitFinishSupport();

        AddOperator(&TVS2TOp);

        AddOperator(&feednpOp);
        AddOperator(&countnpOp);
        AddOperator(&filternpOp);
        AddOperator(&extendnpOp);
        extendnpOp.enableInitFinishSupport();
        AddOperator(&projectextendnpOp);
        projectextendnpOp.enableInitFinishSupport();
        AddOperator(&projectnpOp);
        projectnpOp.enableInitFinishSupport();
     }
};

} // namespace tuplevector

extern "C"
Algebra*
   InitializeTupleVectorAlgebra( NestedList* nlRef,
                           QueryProcessor* qpRef ) {
   return new tuplevector::TupleVectorAlgebra;
}




