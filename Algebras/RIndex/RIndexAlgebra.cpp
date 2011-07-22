
/*
----
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----


//[_] [\_]

*/

#include <sstream>
#include "RIndex.h"
#include "MMRTree.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Stream.h"
#include "ListUtils.h"
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"
#include "FTextAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;


/*
1 RIndexAlgebra

This algebra is used to test performance of a new index structure for a set of 
rectangles realized in  main memory. This structure is compared with an main 
memory R-tree implementation.



1.1 Auxiliary Functions

~hMem~ 

This function converts a byte value into a human readable value.

*/
string  hMem(size_t mem){

   stringstream ss; 
   if(mem >= (1u << 30)){
      ss  << (mem / (1u << 30) ) << "GB";
   } else if(mem >= (1u << 20)){
      ss << (mem / (1u << 20)) << "MB" ;
   } else if(mem > (1u << 10)){
      ss << (mem / (1u << 10)) << "kB" ;
   } else  {
      ss << mem << "b";
   }
   return ss.str();
}

/*
1.2  Different implementations of a tuple store


1.2.1 TupeStore2

This implementation used no persistent structure. This means, all tuples are
hold in main memory.

*/

class TupleStore2{
  public:
/*
~Constructor~

The parameter memSite is ignored in this implementation

*/
    TupleStore2(const size_t memSize) {}

/*
~Destructor~

*/
    ~TupleStore2(){
      for(size_t i=0;i<elems.size();i++){
         elems[i]->DeleteIfAllowed();
      }
    }

/*
~AppendTuple~

Appends a tuple to this TupleStore. The return value can be used to
access the tuple from the store. The tuple is not changed by this 
operation.

*/
    TupleId AppendTuple(Tuple* t){
       TupleId res = elems.size();
       elems.push_back(t);
       t->IncReference();
       return res; 
    }

/*
~GetTuple~

Returns the tuple for a given tupleId. If the tuple does not exist,
0 is returned.

*/

    Tuple* GetTuple(const TupleId id){
       if( (id<0) || (id>=elems.size())){
          return 0;
       }
       Tuple* res = elems[id];
       res->IncReference();
       return res;
    }

  private:
     vector<Tuple*> elems;
};



/*
1.4.3 class TupleStore3 

This class stores all incoming tuples within a relation.
No cache is used for the tuples.

*/

class TupleStore3{
   public:
/*
~Constructor~

The maxMem parameter is ignored.

*/
     TupleStore3(size_t maxMem) : rel(0){}

/*
~Destructor~

*/
     ~TupleStore3(){
       if(rel){
           delete rel;
       }
     }

/*
~AppendTuple~

Appends a tuple to this store. The id of the tuple is
changed by this operation. The new tuple id is also the
return value of this function.

*/
     TupleId AppendTuple(Tuple* t){
        assert(t);
        if(!rel){
          rel = new Relation(t->GetTupleType(), true);
        }
        rel->AppendTuple(t);
        return t->GetTupleId();
     }

/*
~GetTuple~

Retrieves a tuple from this store. If no tuple with given
id is present, 0 is returned.

*/

     Tuple* GetTuple(const TupleId tid){
        if(!rel){
          return 0;
        }
        return rel->GetTuple(tid, false);
     }

   private:
     Relation* rel; // internal store
};



/*
1.4.3 TupleStore1

This tuple store used a chache and a relation to store tuples.


*/
class TupleStore1{
  public:

/*
~Constructor~

This constructor creates a new tuple store. The memSize determines the
size of the used cache in kB.

*/
    TupleStore1(const size_t memSize):
       firstElems(), overflow(0),maxMem(0), usedMem(0), useOverflow(false){
       if(memSize>0){
           maxMem = memSize;
           maxMem = maxMem * 1024u;
       }
    }


/*
~AppendTuple~

This function appends a tuple to this tuple store.
The id of the tuple may be changed or not. To search for a certain
tuple within this tuple store, only the return value of this funtion
can be used.  Note that the returned tuple ids are not nessecary increasing.

*/
    TupleId AppendTuple(Tuple* t){
        assert(t); // don't allow inserting of null pointers
        size_t tm = t->GetMemSize();
        if(!useOverflow){
          if(usedMem + tm  < maxMem){
            size_t id = firstElems.size();
            firstElems.push_back(t);
            t->IncReference();
            usedMem += tm; 
            return (TupleId) id;
          }
          useOverflow = true; // switch to relation
        }
        // tuple must be inserted into relation
        if(!overflow){
           overflow = new Relation(t->GetTupleType(), true);
        }
        overflow->AppendTuple(t);
        TupleId newId = t->GetTupleId();
        return (TupleId) (newId + firstElems.size());
    }
/*
~Destructor~

*/
    ~TupleStore1(){
       if(overflow){
         delete overflow;
       }
       for(size_t i=0; i< firstElems.size();i++){
          firstElems[i]->DeleteIfAllowed();
       }
       firstElems.clear();
    }

/*
~GetTuple~

Retrieves a tuple by id. If no corresponding tuple is found,
0 is returned.

*/

    Tuple* GetTuple(const TupleId tid){
      if(tid < firstElems.size()){ // get from vector
         Tuple* res = firstElems[tid];
         res->IncReference();
         assert(res);
         return res;
      } else {
         return overflow->GetTuple(tid - firstElems.size(),false);
      }
    }

  private:
     vector<Tuple*> firstElems; // cache
     Relation*      overflow;   // persistent structure
     size_t         maxMem;     // maximum cache size
     size_t         usedMem;    // current cache size
     bool           useOverflow;
};


typedef TupleStore1 TupleStore;



/*
1.1 Operator ~insertRIndex~

This operator tests the performance of insertions into the RIndex structure. 
It takes a stream of rectangles and inserts them into an RIndex. Each rectangle
is put into the result stream without changes. 

1.1.1 Type Mapping

The signature is: stream(rect) -> stream(rect)

*/

ListExpr insertRindexTM(ListExpr args){
   if(!nl->HasLength(args,1)){
       return listutils::typeError("stream(rect) expected");
   }

   if(Stream<Rectangle<2> >::checkType(nl->First(args))){
      return nl->First(args);
   }
   return listutils::typeError("stream(rectangle) expected");
};


/*
1.1.2 ValueMapping

We use the Rindex as localinfo. If the stream is closed, the 
index is deleted without using it.

*/

int insertRindexVM( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{

  static int c = 0;

  RIndex<2,int>* rindex = static_cast<RIndex<2,int>*>(local.addr);

  switch(message){
     case OPEN: { qp->Open(args[0].addr); 
                  if(rindex){
                    delete rindex;
                  }
                  local.setAddr(new RIndex<2,int>());
                  return 0;
                   
                }
     case REQUEST: {
                  Word w;
                  qp->Request(args[0].addr, w);
                  if(qp->Received(args[0].addr)){
                     result = w;
                     Rectangle<2>* r = (Rectangle<2>*) w.addr;
                     if(rindex){
                       rindex->insert(*r,c++);
                     } 
                     return YIELD;
                  } else {
                     return CANCEL;
                  }
               }

     case CLOSE: {
                    qp->Close(args[0].addr);
                    if(rindex){
                      delete rindex;
                    }
                    local.setAddr(0);
                    return 0;
               }
  };
  assert(false);
  return -1;
}

/*
1.1.3 Specification


*/

const string insertRindexSpec  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
       "( <text> stream(rectangle) -> stream(rectangle) </text---> "
       "<text>_ insertRindex</text--->"
       "<text>Inserts all the rectangles within the "
       "stream into an Rindex</text--->"
       "<text>"
       "query strassen feed extend[B : bbox(.geoData)] insertRindexi[B] count"
       "</text--->"
     ") )";


/*
1.1.4 Operator instance

*/

Operator insertRindex (
  "insertRindex",
  insertRindexSpec,
  insertRindexVM,
  Operator::SimpleSelect,
  insertRindexTM);



/*
1.2 Operator ~findRIndex~

This operator builds an RIndex from the rectangles coming out of a tuple stream.
The tuples are removed after extracting the rectangle. When the tuple stream is
exhausted, a single search is performed on the created RIndex  using a rectangle
given as the second parameter. The tupleIds of the tuples with intersecting
rectangles are returned in the result stream. 


1.2.1 Type Mapping

The Signature is: stream(tuple(A)) x a[_]j x rect -> stream(tid)

where a[_]j is an attribute name in A of type rect.

*/

ListExpr findRindexTM(ListExpr args){
   string err = "stream(tuple( (..)...(a_i rect) ...)) x a_i x rect expected";
   if(!nl->HasLength(args,3)){
      return listutils::typeError(err + "..1");
   }
   if(!Stream<Tuple>::checkType(nl->First(args))){
      return listutils::typeError(err);
   }
   if(!listutils::isSymbol(nl->Second(args))){
      return listutils::typeError(err + "..2");
   }
   if(!Rect::checkType(nl->Third(args))){
     return listutils::typeError(err + ".. 3");
   }
   ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
   ListExpr type;
   string name = nl->SymbolValue(nl->Second(args));
   int index = listutils::findAttribute(attrList, name, type);
   if(index==0){
     return listutils::typeError("Attribute " + name + " not found in tuple");
   }
   return nl->ThreeElemList(
              nl->SymbolAtom(Symbols::APPEND()),
              nl->OneElemList(nl->IntAtom(index-1)),
              nl->TwoElemList(nl->SymbolAtom(Stream<Rect>::BasicType()),
                              nl->SymbolAtom(TupleIdentifier::BasicType())));
}

/*
1.2.2 Value Mapping

*/

int findRindexVM( Word* args, Word& result, int message,
                      Word& local, Supplier s ){


    switch (message){
      case OPEN : {
           if(local.addr){
              delete (vector<pair<Rectangle<2>, TupleId> >*) local.addr;
              local.addr = 0;
            }
            RIndex<2,TupleId> index;
            Stream<Tuple> stream(args[0].addr);
            stream.open();
            Tuple* tuple = stream.request();
            int i = ((CcInt*)args[3].addr)->GetValue();
            int c = 0;
            while(tuple){
              c++;
              index.insert(*((Rect*)tuple->GetAttribute(i)), 
                           tuple->GetTupleId());
              tuple->DeleteIfAllowed();
              tuple = stream.request();
            }
            stream.close();

            vector<pair<Rectangle<2u>, TupleId> >* res;
            res  = new vector<pair<Rectangle<2u>, TupleId> >();
            Rectangle<2u>* r = (Rectangle<2u>*) args[2].addr;
            index.findSimple( *r, *res);
            local.addr = res;
            return 0;
      }
      case REQUEST: {
             if(!local.addr){
                return CANCEL;
             }
             vector<pair<Rectangle<2>, TupleId> >* res;
             res  = (vector<pair<Rectangle<2>, TupleId> >*) local.addr;
             if(res->empty()){
                return CANCEL;
             }
             result.addr = new TupleIdentifier(true, res->back().second);
             res->pop_back();
             return YIELD;
      }
      case CLOSE: {
           if(local.addr){
              delete (vector<pair<Rectangle<2>, TupleId> >*) local.addr;
              local.addr = 0;
            }
            return 0;
      }
    }
    return -1;
}

/*
1.2.3 Specification

*/

const string findRindexSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text> stream(tuple) x attrName x rect -> stream(tupleid) </text---> "
      "<text>_ findRindex [_ , _ ] </text--->"
      "<text>Includes the rectangles referenced by the "
      "attribute name into an Rindex,"
      " After that, it searches on this structure using the "
      "rectangle and returns all"
      " tuple ids whose rectangles are intersecting"
      " the query rectangle </text--->"
      "<text>query strassen feed extend[B : bbox(.geoData)] "
      "findRindex[B, [const rect value (-10 -10 10 10)] count  </text--->"
   ") )";

/*
1.2.4 Operator instance

*/


Operator findRindex (
  "findRindex",
  findRindexSpec,
  findRindexVM,
  Operator::SimpleSelect,
  findRindexTM);



/*
1.3 Operator ~statRIndex~

This operator creates an RIndex from the rectangles 
stored within a tuple stream. After creating the index, some
information about the structure are reported in the result (as text).


1.3.1 Type Mapping

The signature is:  stream(tuple(A)) x a[_]j -> text

where a[_]j is an attribute name in A belonging to type rect.

*/

ListExpr statRindexTM(ListExpr args){
   string err = "stream(tuple) x attrname expected";
   int len = nl->ListLength(args);
   if((len!=2)){
      return listutils::typeError(err);
   }

   if(!Stream<Tuple>::checkType(nl->First(args))){
      return listutils::typeError(err); 
   }
   ListExpr sec = nl->Second(args);
   if(!listutils::isSymbol(sec)){
      return listutils::typeError(err); 
   }

   ListExpr attrType;
   string name = nl->SymbolValue(sec);
   int index = listutils::findAttribute(
                            nl->Second(nl->Second(nl->First(args))),
                            name, attrType);
   if(index==0){
       return listutils::typeError("Attribute " + name + 
                                   " not known in stream");
   }
   if(!Rectangle<2>::checkType(attrType)){
       return listutils::typeError("Attribute " + name + 
                              " not of type " + Rectangle<2>::BasicType());
   }

   return nl->ThreeElemList( 
              nl->SymbolAtom(Symbols::APPEND()),
              nl->OneElemList(nl->IntAtom(index-1)),
              nl->SymbolAtom(FText::BasicType()));
}

/*
1.3.2 Value Mapping

*/

int statRindexVM( Word* args, Word& result, int message,
                    Word& local, Supplier s ){

   Stream<Tuple> stream(args[0]);
   Rectangle<2>* r;
   int index = ((CcInt*)args[2].addr)->GetValue();

   stream.open();
   Tuple* t;
   t  = stream.request();
   size_t tupleSizes = 0;
   RIndex<2,int> ind;
   while(t){
     r = (Rectangle<2>*) t->GetAttribute(index);
     ind.insert(*r,1);
     tupleSizes += t->GetMemSize();
     t->DeleteIfAllowed();
     t = stream.request();
   }
   stream.close();
   result = qp->ResultStorage(s);
   FText* res = static_cast<FText*>(result.addr);
   stringstream out;
   out << "countEntries =" << ind.countEntries() << endl;
   out << "height       = " << ind.height() << endl; 
   out << "noLeafs      = " << ind.noLeafs() << endl;
   out << "dim0Entries  = " << ind.dim0Entries() << endl;
   out << "dim0Leafs    = " << ind.dim0Leafs() << endl;
   out << "maxBranches  = " << ind.maxBranches() << endl;
   out << "branches     = " << ind.branches() << endl;
   size_t um = ind.usedMem();
   out << "usedMem() = " << ind.usedMem() << " (" << hMem(um) << ")" << endl;
   out << "used mem by tuples = " << tupleSizes 
       << " (" << hMem(tupleSizes) << ")" <<  endl;
   res->Set(true,out.str());
   return 0;
}


const string statRindexSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text> stream(tuple(A)) x a_j -> text "
      "  </text---> "
      "<text>_ statRindex[_]  </text--->"
      "<text>Creates an RIndex from the rectangles referenced by a_j in A."
      "an returns statistical informations about this structure </text--->"
      "<text>query strassen extend [B : bbox(.geoData)] feed statRIndex[B] "
      " </text--->"
  ") )";


Operator statRindex (
  "statRindex",
  statRindexSpec,
  statRindexVM,
  Operator::SimpleSelect,
  statRindexTM);




/*

1.4 ~joinRIndex~


1.4.1 Type Mapping

Signature is: stream(tuple(A)) x stream(tuple(B)) x a[_]i x b[_]j ->
                                  stream(tuple((TID1 : tid)(TID2: tid)))

where a[_]i points to a rect attribute in A and b[_]j points to a rect 
attribute in B.

*/

ListExpr joinRindexTM(ListExpr args){
   string err = "stream(tuple(A)) x stream(tuple(B)) x a_i x b_j expected";
   if(!nl->HasLength(args,4)){
     return listutils::typeError(err);
   }

   if(!Stream<Tuple>::checkType(nl->First(args)) ||
      !Stream<Tuple>::checkType(nl->Second(args)) ||
      !listutils::isSymbol(nl->Third(args)) ||
      !listutils::isSymbol(nl->Fourth(args))){
     return listutils::typeError(err);
   }

   ListExpr attrList1 = nl->Second(nl->Second(nl->First(args)));
   ListExpr attrList2 = nl->Second(nl->Second(nl->Second(args)));
   string name1 = nl->SymbolValue(nl->Third(args));
   string name2 = nl->SymbolValue(nl->Fourth(args));

   ListExpr type1;
   ListExpr type2;

   int index1 = listutils::findAttribute(attrList1, name1, type1);
   if(index1 == 0){
     return listutils::typeError("Attribute " + name1 + 
                                 " not present in first stream");
   }
   if(!Rectangle<2>::checkType(type1)){
     return listutils::typeError("Attribute " + name1 + 
                                 " not of type " + Rectangle<2>::BasicType());
   }
   
   int index2 = listutils::findAttribute(attrList2, name2, type2);
   if(index2 == 0){
     return listutils::typeError("Attribute " + name2 + 
                                 " not present in second stream");
   }
   if(!Rectangle<2>::checkType(type2)){
     return listutils::typeError("Attribute " + name2 + 
                                 " not of type " + Rectangle<2>::BasicType());
   }
 
   ListExpr attrList = nl->TwoElemList( 
                          nl->TwoElemList(
                              nl->SymbolAtom("TID1"), 
                              nl->SymbolAtom(TupleIdentifier::BasicType())),
                          nl->TwoElemList(
                              nl->SymbolAtom("TID2"), 
                              nl->SymbolAtom(TupleIdentifier::BasicType())));

   ListExpr resList = nl->TwoElemList( 
                           nl->SymbolAtom(Stream<Tuple>::BasicType()),
                           nl->TwoElemList(
                               nl->SymbolAtom(Tuple::BasicType()),
                               attrList));


   return nl->ThreeElemList(
                   nl->SymbolAtom(Symbols::APPEND()),
                   nl->ThreeElemList(
                                   nl->IntAtom(-1), // dummy for maxmem
                                   nl->IntAtom(index1-1), 
                                   nl->IntAtom(index2-1)),
                   resList);
}




/*
1.5 Operator ~realJoinRIndex~

This operator performs a spatial join on two tuple streams. 


1.5.1 Type Mapping

Signature is: stream(tuple(A)) x stream(tuple(B)) x a[_]i x b[_]j [ x int] -> 
              stream(tuple( A o B))

a[_]i and b[_]j must point to an attribute of type rect in A and B, 
respectively. 
The optional integer value is the maximum memory used to cache tuples from
the first stream in kB.

*/

ListExpr realJoinRindexTM(ListExpr args){


   string err = "stream(tuple) x stream(tuple) x a_i x b_j [x int] expected";
   int len = nl->ListLength(args);
   if((len!=4) && (len !=5)){
     return listutils::typeError(err);
   }

   if(!Stream<Tuple>::checkType(nl->First(args)) ||
      !Stream<Tuple>::checkType(nl->Second(args)) ||
      !listutils::isSymbol(nl->Third(args)) ||
      !listutils::isSymbol(nl->Fourth(args))){
     return listutils::typeError(err);
   }

   if( (len==5) && !CcInt::checkType(nl->Fifth(args))){
     return listutils::typeError(err);
   }

   ListExpr attrList1 = nl->Second(nl->Second(nl->First(args)));
   ListExpr attrList2 = nl->Second(nl->Second(nl->Second(args)));
   string name1 = nl->SymbolValue(nl->Third(args));
   string name2 = nl->SymbolValue(nl->Fourth(args));

   ListExpr type1;
   ListExpr type2;

   int index1 = listutils::findAttribute(attrList1, name1, type1);
   if(index1 == 0){
     return listutils::typeError("Attribute " + name1 + 
                                 " not present in first stream");
   }
   if(!Rectangle<2>::checkType(type1)){
     return listutils::typeError("Attribute " + name1 + 
                                 " not of type " + Rectangle<2>::BasicType());
   }
   
   int index2 = listutils::findAttribute(attrList2, name2, type2);
   if(index2 == 0){
     return listutils::typeError("Attribute " + name2 + 
                                 " not present in second stream");
   }
   if(!Rectangle<2>::checkType(type2)){
     return listutils::typeError("Attribute " + name2 + 
                                 " not of type " + Rectangle<2>::BasicType());
   }
 
   ListExpr attrList = listutils::concat(attrList1, attrList2);

   if(!listutils::isAttrList(attrList)){
     return listutils::typeError("name conflicts in tuple streams");
   }

   ListExpr resList = nl->TwoElemList( 
                           nl->SymbolAtom(Stream<Tuple>::BasicType()),
                           nl->TwoElemList(
                               nl->SymbolAtom(Tuple::BasicType()),
                               attrList));
   ListExpr appendList;
   if(len==5) {
         appendList = nl->TwoElemList(nl->IntAtom(index1-1), 
                                      nl->IntAtom(index2-1));
   } else {
         appendList = nl->ThreeElemList(
                                      nl->IntAtom(-1),                          
                                      nl->IntAtom(index1-1), 
                                      nl->IntAtom(index2-1));

   }

   return nl->ThreeElemList(
                   nl->SymbolAtom(Symbols::APPEND()),
                   appendList,
                   resList);
}



/*
1.4.5 Auxiliary class ~JoinRIndexLocalInfo~

This class gets two tuple streams. From the first one, 
an RIndex ~ind~  is created. For each tuple of the second stream,
we perform a search on ~ind~ to find more result. 

*/


class JoinRindexLocalInfo{

  public:

/*
~Constructor~

arguments are:

----
    _s1 : first stream
    _s2 : second stream
    _i1 : index of the rectangle attribute in _s1
    _i2 : index of the rectangle attribute in _s2
    _tt : list decribung the tuple type of the result
    mamMem : dummy parameter for compatibility with other classes
----

*/

     JoinRindexLocalInfo(Word& _s1, Word& _s2, int _i1, int _i2, ListExpr _tt, 
                         size_t maxMem):  
         ind(), s2(_s2),  i2(_i2) {

        // build the index from the first stream
        tt = new TupleType(_tt);
        Stream<Tuple> s1(_s1);
        s1.open();
        Tuple* t = s1.request();
        while(t){
           Rectangle<2>* r = (Rectangle<2>*)t->GetAttribute(_i1);
           ind.insert(*r, t->GetTupleId());
           t->DeleteIfAllowed(); 
           t = s1.request();
        }
        s1.close();
        s2.open();
        currentTuple = 0;
     }
/*
~Destructor~

*/

     ~JoinRindexLocalInfo(){
         s2.close();
         tt->DeleteIfAllowed();
         if(currentTuple){
            currentTuple->DeleteIfAllowed();
         }
      }

/*
~nextTuple~

Returns the next result tuple or 0 if no more tuples are
available.

*/

      Tuple* nextTuple(){
        if(lastRes.empty() && (currentTuple!=0)){
           // last result is processed
           currentTuple->DeleteIfAllowed();
           currentTuple = 0;
         }        

         // try to find new results
         while(lastRes.empty()){
            Tuple* t = s2.request();
            if(t==0){
               return 0;
            }
            Rectangle<2>* r = (Rectangle<2>*) t->GetAttribute(i2);
            ind.findSimple(*r, lastRes); 
            if(lastRes.empty()){
                t->DeleteIfAllowed();
            } else {
               currentTuple = t;
            }
         }

         pair<Rectangle<2>,TupleId> p1 = lastRes.back();
         lastRes.pop_back();
         Tuple*result = new Tuple(tt);
         result->PutAttribute(0, new TupleIdentifier(true, p1.second));
         result->PutAttribute(1, 
                        new TupleIdentifier(currentTuple->GetTupleId())); 
         return result;
      }

   private:
      RIndex<2,TupleId> ind;    // the index used
      Stream<Tuple> s2;         // the second stream
      int i2;                   // index of the rectangle attribute in s2
      TupleType* tt;            // result tuple type
      vector<pair<Rectangle<2>,TupleId> > lastRes; // result of the last search
      Tuple* currentTuple;      // tuple from s2 used to create lastRes
};

/*
1.4.6 Auxiliary class ~SymmJoinRindexLocalInfo~

This class is the symmetric version of JoinRindexLocalInfo.

It gets two tuple streams , each having an attribute of type rect.
The streams are used alternating. Each tuple is inserted into an
Rindex for this stream. Furthermor, a search is performed on the
Rindex of the second stream to find results.


*/


class SymmJoinRindexLocalInfo{
  public:

/*
~Constructor~

The parameters are:

----
    _s1 : first stream
    _s2 : second stream
    _i1 : index of the rect attribute in _s1
    _i2 : index of the rect attribute in _s2
    _tt : list describing the result tuple type
    maxMem : dummy parameter for compatibility with other classes

----


*/
     SymmJoinRindexLocalInfo(Word& _s1, Word& _s2, 
                             int _i1, int _i2, 
                             ListExpr _tt, size_t maxMem):
        ind1(),ind2(), s1(_s1), s2(_s2), 
        i1(_i1), i2(_i2), tt(0), lastRes(), 
        currentTupleId(0){
        tt = new TupleType(_tt);
        s1.open();
        s2.open();
        next = false; 
     }

/*
~Destructor~

*/

     ~SymmJoinRindexLocalInfo(){
        s1.close();
        s2.close();
        tt->DeleteIfAllowed();
     }

/*
~nextTuple~

Returns the next result tuple or 0 if no more tuples are available.

*/

     Tuple* nextTuple(){

         while(lastRes.empty()){
            next = !next; // switch stream
            Tuple* t = next?s1.request():s2.request();
            if(!t){ // stream exhausted, switch again
               next = !next;
               t = next?s1.request():s2.request();
            }
            if(t==0){  // input streams exhausted
              return 0;
            }
            int index = next?i1:i2;
            Rectangle<2>* r = (Rectangle<2>*) t->GetAttribute(index);
            currentTupleId = t->GetTupleId();
            if(next){
              ind1.insert(*r, currentTupleId);
              ind2.findSimple(*r, lastRes);
            } else {
              ind2.insert(*r, currentTupleId);
              ind1.findSimple(*r, lastRes);
            }   
            t->DeleteIfAllowed();
         }

         pair<Rectangle<2>,TupleId> p1 = lastRes.back();
         lastRes.pop_back();
         Tuple*result = new Tuple(tt);

         if(next){
            result->PutAttribute(1, new TupleIdentifier(true, p1.second));
            result->PutAttribute(0, new TupleIdentifier(currentTupleId)); 
         } else {
            result->PutAttribute(0, new TupleIdentifier(true, p1.second));
            result->PutAttribute(1, new TupleIdentifier(currentTupleId)); 
         }
         return result;
     }


  private:

      RIndex<2,TupleId> ind1;    // index for rectangles of s1
      RIndex<2,TupleId> ind2;    // index for rectangles of s2
      Stream<Tuple> s1;          // first tuple stream 
      Stream<Tuple> s2;          // second tuple stream
      int i1;                    // index of rect attribute in s1
      int i2;                    // index of rect attribute in s2
      TupleType* tt;             // result tuple type
      vector<pair<Rectangle<2>,TupleId> > lastRes; // result of the last search
      TupleId currentTupleId;    // tupleId whose corresponding rectangle 
                                 //has found lastRes
      bool next;                 // flag which stream is used,
                                 // 1 = first, 0 = second
};


/*
Auxiliary class ~RealJoinRindexLocalInfo~

The template parameter Tree determines the structure
used as index.


*/



template <class Tree>
class RealJoinTreeLocalInfo{

  public:

/*
~Constructor~

The parameters are:

----
     _s1 : first stream
     _s2 : second stream
     _i1 : index of a rect attribute in _s1
     _i2 : index of a rect attribute in _s2
     _tt : list describing the result tuple type
     _maxMem : maximum cache size for tuples of _s1 in kB

----

The cinstructor is blocking. This means, the first stream is 
processed completely. Each tuple is inserted into a tuple store 
and into an Tree structure.

*/ 

     RealJoinTreeLocalInfo(Word& _s1, Word& _s2, int _i1, 
                             int _i2, ListExpr _tt, size_t maxMem):
         ind(), s2(_s2),  i2(_i2) {

        // build the index from the first stream
        tt = new TupleType(_tt);
        Stream<Tuple> s1(_s1);
        s1.open();
        Tuple* t = s1.request();
        if(t){
          tb = new TupleStore(maxMem); 
        } else {
           tb = 0;
        }
        while(t){
           TupleId id = tb->AppendTuple(t);
           Rectangle<2>* r = (Rectangle<2>*)t->GetAttribute(_i1);
           ind.insert(*r, id);
           t->DeleteIfAllowed(); 
           t = s1.request();
        }
        s1.close();
        s2.open();
        currentTuple = 0;
     }

/*
~Constructor~

The parameters are:

----
     _s1 : first stream
     _s2 : second stream
     _i1 : index of a rect attribute in _s1
     _i2 : index of a rect attribute in _s2
     _tt : list describing the result tuple type
     min : minimum number of entries within a node of the index
     max : maximum number of entries within a node of the index
     _maxMem : maximum cache size for tuples of _s1 in kB

----



*/

     RealJoinTreeLocalInfo(Word& _s1, Word& _s2, int _i1, 
                             int _i2, ListExpr _tt, int min, int max,
                             size_t maxMem):
         ind(min,max), s2(_s2),  i2(_i2) {

        // build the index from the first stream
        tt = new TupleType(_tt);
        Stream<Tuple> s1(_s1);
        s1.open();
        Tuple* t = s1.request();
        if(t){
           tb = new TupleStore(maxMem);
        } else {
           tb = 0;
        }
        while(t){
           TupleId id = tb->AppendTuple(t); 
           Rectangle<2>* r = (Rectangle<2>*)t->GetAttribute(_i1);
           ind.insert(*r, id);
           t->DeleteIfAllowed(); 
           t = s1.request();
        }
        s1.close();
        s2.open();
        currentTuple = 0;
     }

/*
~Destructor~

*/

     ~RealJoinTreeLocalInfo(){
         s2.close();
         
         tt->DeleteIfAllowed();
         if(tb){
           delete tb;
         }
         if(currentTuple){
            currentTuple->DeleteIfAllowed();
         }
         
      }

/*
~nextTuple~

Returns the next result tuple or 0 if no more tuples are available.

*/

      Tuple* nextTuple(){
        if(lastRes.empty() && (currentTuple!=0)){
           currentTuple->DeleteIfAllowed();
           currentTuple = 0;
         }        

         while(lastRes.empty()){
            Tuple* t = s2.request();
            if(t==0){
               return 0;
            }
            Rectangle<2>* r = (Rectangle<2>*) t->GetAttribute(i2);
            ind.findSimple(*r, lastRes); 
            if(lastRes.empty()){
                t->DeleteIfAllowed();
            } else {
               currentTuple = t;
            }
         }

         pair<Rectangle<2>,TupleId> p1 = lastRes.back();
         lastRes.pop_back();
         Tuple*result = new Tuple(tt);
         Tuple* t1 = tb->GetTuple(p1.second);
         Concat(t1, currentTuple, result);
         t1->DeleteIfAllowed();         
         return result;
      }

   private:
      Tree ind;
      Stream<Tuple> s2;
      int i2;
      TupleType* tt;
      vector<pair<Rectangle<2>,TupleId> > lastRes;
      Tuple* currentTuple;
      TupleStore* tb;
};


/*
1.4.7 Value Mapping


*/
template<class JLI>
int joinRindexVM( Word* args, Word& result, int message,
                    Word& local, Supplier s ){

   JLI* li = static_cast<JLI*>(local.addr);
 
   switch(message){
        case OPEN: {
             if(li){
               delete li;
             }
             CcInt* MaxMem = (CcInt*) args[4].addr;
             size_t maxMem = qp->MemoryAvailableForOperator()/1024;
             if(MaxMem->IsDefined()  && MaxMem->GetValue()>0){
                  maxMem = MaxMem->GetValue();
             }
             int i1 = ((CcInt*)(args[5].addr))->GetValue();
             int i2 = ((CcInt*)(args[6].addr))->GetValue();
             
             local.addr = new JLI(args[0], args[1],i1,i2,
                                  nl->Second(GetTupleResultType(s)),
                                  maxMem);
             return 0; 
        }
        case REQUEST: {
             if(!li){
                return CANCEL;
             }
             result.addr = li->nextTuple();
             return result.addr?YIELD:CANCEL;
        }
        case CLOSE:{
             if(li){
               delete li;
               local.addr = 0;
             }
        }

   }
   return -1;
}

/*
1.4.8 Specification


*/
const string joinRindexSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text> stream(tuple(A)) x stream(tuple(B)) x a_i x b_j -> "
      "stream(tuple(( TOD1 : tid)(TID2 : tid))i) </text---> "
      "<text>_ _ joinRIndex[_,_]  </text--->"
      "<text>Performes a spatial join on two streams. "
      "The attributes a_i and b_j must"
      " be of type rect. The result is a stream of "
      "tuples with the corresponding Tuple "
      " ids"
      " </text--->"
      "<text>query strassen feed extend[B : bbox(.geoData] strassen "
      "feed extend[B : bbox(.geoData)] joinRindex[B,B] count "
      " </text--->"
             ") )";

/*
1.4.9 Operator instance

*/
Operator joinRindex (
  "joinRindex",
  joinRindexSpec,
  joinRindexVM<JoinRindexLocalInfo>,
  Operator::SimpleSelect,
  joinRindexTM);

/*
1.4.10

Specification of ~symmJoinRindex~

*/

const string symmJoinRindexSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text> stream(tuple(A)) x stream(tuple(B)) x a_i x b_j -> "
      "stream(tuple(( TOD1 : tid)(TID2 : tid))i) </text---> "
      "<text>_ _ symmJoinRIndex[_,_]  </text--->"
      "<text>Performes a spatial join on two streams. "
      "The attributes a_i and b_j must"
      " be of type rect. The result is a stream of "
      "tuples with the corresponding Tuple "
      " ids"
      " </text--->"
      "<text>query strassen feed extend[B : bbox(.geoData] strassen "
      "feed extend[B : bbox(.geoData)] symmJoinRindex[B,B] count "
      " </text--->"
             ") )";

/*
1.4.11 Operator instance

*/
Operator symmJoinRindex (
  "symmJoinRindex",
  symmJoinRindexSpec,
  joinRindexVM<SymmJoinRindexLocalInfo>,
  Operator::SimpleSelect,
  joinRindexTM);

/*
1.4.12 Specification of ~realJoinRindex~

*/

const string realJoinRindexSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text> stream(tuple(X)) x stream(tuple(Y)) x a_i x b_j [x int] -> "
      "stream(tuple(X o Y)) </text---> "
      "<text>_ _ realJoinRindex[a_i, b_j [, maxMem] ]  </text--->"
      "<text>Performes a spatial join on two streams. "
      "The attributes a_i and b_j must"
      " be of type rect. The result is a stream of "
      "tuples with intersecting rectangles build as concatenation"
      " of the source tuples. The optional maxMem argument is the"
      " maximum size of the tuple cache in kB. Note that there is "
      " no memory space limitation for the used index."
      " </text--->"
      "<text>query strassen feed extend[B : bbox(.geoData] {a} strassen "
      "feed extend[B : bbox(.geoData)] {b} realJoinRindex[B_a,B_b, 1024] count "
      " </text--->"
             ") )";

/*
1.4.13 Operator instance

*/

Operator realJoinRindex (
  "realJoinRindex",
  realJoinRindexSpec,
  joinRindexVM<RealJoinTreeLocalInfo<RIndex<2, TupleId> > >,
  Operator::SimpleSelect,
  realJoinRindexTM);



/*
1.5 Operator ~realJoinMMRTRee~

1.5.1 Type mapping

Signature is: stream(tuple(A)) x stream(tuple(B)) x a[_]i x b[_]j x 
              int x int [ x int ] -> stream(tuple(A o B))

Meaning is : stream1 x stream2 x attrname1 x attrname2 x min x max x maxMem
where


----
  stream1   : first input stream
  stream2   : second input stream
  attrname1 : attribute name for an attribute of type rect in stream1
  attrname2 : attribute name for an attribute of type rect in stream2
  min       : minimum number of entries within a node of the used rtree
  max       : maximum number of entries within a node of the used rtree

----

*/

ListExpr realJoinMMRTreeTM(ListExpr args){

  // similar to the version using an RIndex, but 2 additional integer values are
  // required for the min and max entry value for an rtree node
  string err = "stream(tuple(A)) x stream(tuple(B)) x"
               " a_i x b_i x int x int [x int] expected";
   int len = nl->ListLength(args);
   if((len!=6) && (len!=7)){
     return listutils::typeError(err);
   }

   if(!Stream<Tuple>::checkType(nl->First(args)) ||
      !Stream<Tuple>::checkType(nl->Second(args)) ||
      !listutils::isSymbol(nl->Third(args)) ||
      !listutils::isSymbol(nl->Fourth(args))  ||
      !CcInt::checkType(nl->Fifth(args)) ||
      !CcInt::checkType(nl->Sixth(args))){
     return listutils::typeError(err);
   }

   if( (len==7) && ! CcInt::checkType(nl->Sixth(nl->Rest((args))))){
     return listutils::typeError(err);
   }

   ListExpr attrList1 = nl->Second(nl->Second(nl->First(args)));
   ListExpr attrList2 = nl->Second(nl->Second(nl->Second(args)));
   string name1 = nl->SymbolValue(nl->Third(args));
   string name2 = nl->SymbolValue(nl->Fourth(args));

   ListExpr type1;
   ListExpr type2;

   int index1 = listutils::findAttribute(attrList1, name1, type1);
   if(index1 == 0){
     return listutils::typeError("Attribute " + name1 + 
                                 " not present in first stream");
   }
   if(!Rectangle<2>::checkType(type1)){
     return listutils::typeError("Attribute " + name1 + 
                                 " not of type " + Rectangle<2>::BasicType());
   }
   
   int index2 = listutils::findAttribute(attrList2, name2, type2);
   if(index2 == 0){
     return listutils::typeError("Attribute " + name2 + 
                                 " not present in second stream");
   }
   if(!Rectangle<2>::checkType(type2)){
     return listutils::typeError("Attribute " + name2 + 
                                 " not of type " + Rectangle<2>::BasicType());
   }
 
   ListExpr attrList = listutils::concat(attrList1, attrList2);

   if(!listutils::isAttrList(attrList)){
     return listutils::typeError("name conflicts in tuple streams");
   }

   ListExpr resList = nl->TwoElemList( 
                           nl->SymbolAtom(Stream<Tuple>::BasicType()),
                           nl->TwoElemList(
                               nl->SymbolAtom(Tuple::BasicType()),
                               attrList));
   ListExpr appendList;
   if(len==7){
      appendList =   nl->TwoElemList(nl->IntAtom(index1-1), 
                                     nl->IntAtom(index2-1));
   } else {
      appendList =   nl->ThreeElemList(
                                     nl->IntAtom(-1),
                                     nl->IntAtom(index1-1), 
                                     nl->IntAtom(index2-1));
   }

   return nl->ThreeElemList(
                   nl->SymbolAtom(Symbols::APPEND()),
                   appendList,
                   resList);

}


/*
1.4.2 Value Mapping


*/

template<class JLI>
int joinRTreeVM( Word* args, Word& result, int message,
                    Word& local, Supplier s ){

   JLI* li = static_cast<JLI*>(local.addr);

   switch(message){
        case OPEN: {
             if(li){
               delete li;
               li = 0;
             }
             CcInt* MaxMem = (CcInt*) args[6].addr;
             size_t maxMem = qp->MemoryAvailableForOperator()/1024;
             if(MaxMem->IsDefined() && MaxMem->GetValue() > 0){
                maxMem = MaxMem->GetValue();
             }
             CcInt* Min = (CcInt*)(args[4].addr);
             CcInt* Max = (CcInt*)(args[5].addr);
             if(!Min->IsDefined() || !Max->IsDefined()){
                return 0;
             }
             int min = Min->GetValue();
             int max = Max->GetValue();
             if(min<1 || max< (2*min)){
                  return 0;
             }
             int i1 = ((CcInt*)(args[7].addr))->GetValue();
             int i2 = ((CcInt*)(args[8].addr))->GetValue();
             local.addr = new JLI(args[0], args[1],i1,i2,
                                  nl->Second(GetTupleResultType(s)), 
                                  min, max, maxMem);
             return 0; 
        }
        case REQUEST: {
             if(!li){
                return CANCEL;
             }
             result.addr = li->nextTuple();
             return result.addr?YIELD:CANCEL;
        }
        case CLOSE:{
             if(li){
               delete li;
               local.addr = 0;
             }
        }

   }
   return -1;
}

/*
1.4.3 Specification

*/
const string realJoinMMRTreeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> stream(tuple(A)) x stream(tuple(B)) x a_i x b_j x int"
    " x int [ x int]-> stream(tuple(X o Y)) </text---> "
    "<text>streamA streamB realJoinMMRTree[attrnameA, attrnameB, min,"
    " max, maxMem]"
    "  </text--->"
    "<text>Performes a spatial join on two streams. "
    "The attributes a_i and b_j must"
    " be of type rect in A and B, respectively. The result is a stream of "
    "tuples with intersecting rectangles build as concatenation"
    " of the source tuples. min and max define the range for the number of"
    " entries within the nodes of the used rtree. maxMem is the maximum cache"
    " size of the tuple store for tuples coming from streamA."
    " If maxMem is omitted, "
    " The default value MaxMemPerOperator (see SecondoConfig.ini) is used."
    " </text--->"
    "<text>query strassen feed extend[B : bbox(.geoData] strassen "
    "feed extend[B : bbox(.geoData)] {a} realJoinMMRTree[B,B_a,8,16, 1024]"
    " count "
    " </text--->"
         ") )";

/*
1.4.4 Operator Instance

*/

Operator realJoinMMRTree(
  "realJoinMMRTree",
  realJoinMMRTreeSpec,
  joinRTreeVM<RealJoinTreeLocalInfo<mmrtree::RtreeT<2, TupleId> > >,
  Operator::SimpleSelect,
  realJoinMMRTreeTM 
 );




/*
1.5 insertMMRTree

This operator can be used to check the performance of building an rtree
in main memory.

1.5.1 Type Mapping

Signature is:  stream(rectangle) x int x int

The last two arguments denote the range of number of entries within the
 used rtree.

*/

ListExpr insertMMRTreeTM(ListExpr args){
   string err = "stream(rectangle) x int x int expected";
   if(!nl->HasLength(args,3)){
       return listutils::typeError(err);
   }

   if(!Stream<Rectangle<2> >::checkType(nl->First(args))){
       return listutils::typeError(err);
   }
   if(!CcInt::checkType(nl->Second(args))){
       return listutils::typeError(err);
   }
   if(!CcInt::checkType(nl->Third(args))){
       return listutils::typeError(err);
   }
   return nl->First(args);
};


/*
1.5.2 Value Mapping

*/
int insertMMRTreeVM( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{

  static long  c = 0;

  mmrtree::Rtree<2>* mmrtree1  = static_cast<mmrtree::Rtree<2>*>(local.addr);

  switch(message){
     case OPEN: { qp->Open(args[0].addr); 
                  if(mmrtree1){
                    delete mmrtree1;
                    local.addr = 0;
                  }
                  CcInt* min1 = static_cast<CcInt*>(args[1].addr);
                  CcInt* max1 = static_cast<CcInt*>(args[2].addr);
                  if(!min1->IsDefined() || !max1->IsDefined()){
                      return 0;
                  }
                  int min2 = min1->GetValue();
                  int max2 = max1->GetValue();
                  if(min2 <1 || max2 < 2* min2){
                    return 0;
                  }
                  local.setAddr(new mmrtree::Rtree<2>(min2,max2));
                  return 0;
                   
                }
     case REQUEST: {
                 if(!mmrtree1){
                    return CANCEL;
                  }  
                  Word w;
                  qp->Request(args[0].addr, w);
                  if(qp->Received(args[0].addr)){
                     result = w;
                     Rectangle<2>* r = (Rectangle<2>*) w.addr;
                     if(mmrtree1){
                       mmrtree1->insert(*r,c++);
                     } 
                     return YIELD;
                  } else {
                     return CANCEL;
                  }
               }

     case CLOSE: {
                    qp->Close(args[0].addr);
                    if(mmrtree1){
                      delete mmrtree1;
                    }
                    local.setAddr(0);
                    return 0;
               }
  };
  assert(false);
  return -1;
}

/*
1.5.3 Specification

*/

const string insertMMRTreeSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
       "( <text> stream(rectangle) x int x int -> stream(rectangle) </text---> "
       "<text> _ insertMMRTRee [min , max] </text--->"
       "<text>Inserts all the rectangles within the "
       "stream into a main memory based rtree. [min, max] is the allowed range"
       " for the numbers of entries within a node of the rtree. min must be"
       " greater than 0 and max must be at least two times min. "
       "  </text--->"
       "<text>query strassen feed extend[B : bbox(.geoData)] "
       "projecttransformstream[B] insertMMRTRee[4,8]  count  </text--->"
             ") )";


/*
1.5.4 Operator instance

*/

Operator insertMMRTree (
  "insertMMRTree",
  insertMMRTreeSpec,
  insertMMRTreeVM,
  Operator::SimpleSelect,
 insertMMRTreeTM);


/*
1.6 Operator ~statMMRTree~

Provides statistical information about an mmrtree build from a tuple stream.


1.6.1 Type Mapping

Signature is:  stream(tuple(A)) x a[_]i x int x int


*/
ListExpr statMMRTreeTM(ListExpr args){
  string err = "stream(Tuple(A)) x a_i x int x int expected";
  if(!nl->HasLength(args,4)){
     return listutils::typeError(err);
  }
  if(!Stream<Tuple>::checkType(nl->First(args)) ||
     !listutils::isSymbol(nl->Second(args)) ||
     !CcInt::checkType(nl->Third(args)) ||
     !CcInt::checkType(nl->Fourth(args))){
     return listutils::typeError(err);
  }
  string name = nl->SymbolValue(nl->Second(args));
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr type;
  int index = listutils::findAttribute(attrList, name, type);
  if(index==0){
     return listutils::typeError("Attribute " + name + " unknown in tuple");
  }
  if(!Rectangle<2>::checkType(type)){
     return listutils::typeError("Attribute " + name + " not of type" +
                                 Rectangle<2>::BasicType());
  }
  return nl->ThreeElemList(
           nl->SymbolAtom(Symbols::APPEND()),
           nl->OneElemList(nl->IntAtom(index-1)),
           nl->SymbolAtom(FText::BasicType()));
}

/*
1.6.2 Value Mapping

*/
int statMMRTreeVM( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{  
   result = qp->ResultStorage(s);
   FText* res = (FText*) result.addr;
   CcInt* Min = (CcInt*) args[2].addr;
   CcInt* Max = (CcInt*) args[3].addr;
   if(!Min->IsDefined() || !Max->IsDefined()){
      res->Set(true,"At least one of the range value is undefined");
      return 0;
   } 
   int min = Min->GetValue();
   int max = Max->GetValue();
   if(min<1 || max < min*2){
      res->Set(true, "Invalid range, min must be greater "
                     "than 0, and max >= 2*min");
      return 0;
   }
   size_t tuples = 0;
   Stream<Tuple> stream(args[0]);
   stream.open();
   int index = ((CcInt*)args[4].addr)->GetValue();
   mmrtree::RtreeT<2,TupleId> tree(min,max);
   Tuple* t = stream.request();
   while(t){
      Rectangle<2>* r = (Rectangle<2>*) t->GetAttribute(index);
      tree.insert(*r, t->GetTupleId());
      tuples += t->GetMemSize();
      t->DeleteIfAllowed();
      t = stream.request();
   }
   stream.close();
   stringstream ss;
   tree.printStats(ss);
   ss << " Mem used by tuples = " << tuples  
      << "(" << hMem(tuples) << ")" << endl;
   res->Set(true, ss.str());
   return 0; 
}

/*
1.6.3 Specification

*/
const string statMMRTreeSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
       "( <text> stream(Tuple(A)) x a_i  x int x int -> text </text---> "
       "<text>_ statMMRTRee [ attrname, min , max] </text--->"
       "<text>Inserts all the rectangles within the "
       "stream (pointered by a_i) into a main memory based rtree ."
       " Statistical information about this tree is the result of this "
       "operator</text--->"
       "<text>query strassen feed extend[B : bbox(.geoData)] "
       " statMMRTRee[B,4,8]    </text--->"
             ") )";

/*
1.6.4 Operator instance

*/
Operator statMMRTree (
  "statMMRTree",
  statMMRTreeSpec,
  statMMRTreeVM,
  Operator::SimpleSelect,
  statMMRTreeTM);




/*
2 Algebra Definition

2.1 Algebra Class

*/

class RIndexAlgebra : public Algebra {
  public:
   RIndexAlgebra() : Algebra()
   {
      AddOperator(&insertRindex);
      AddOperator(&findRindex);
      AddOperator(&statRindex);
      AddOperator(&joinRindex);
      AddOperator(&symmJoinRindex);
      AddOperator(&realJoinRindex);
      
      // operators using mmrtrees
      AddOperator(&insertMMRTree);
      AddOperator(&realJoinMMRTree);
      AddOperator(&statMMRTree);

   }
};

/*
2.2 Algebra Initialization

*/

extern "C"
Algebra*
InitializeRIndexAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new RIndexAlgebra());
}



