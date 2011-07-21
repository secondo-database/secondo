
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




ListExpr insertRindexTM(ListExpr args){
   if(!nl->HasLength(args,1)){
       return listutils::typeError("stream(rectangle) expected");
   }

   if(Stream<Rectangle<2> >::checkType(nl->First(args))){
      return nl->First(args);
   }
   return listutils::typeError("stream(rectangle) expected");
};

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

const string insertRindexSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
       "( <text> stream(rectangle) -> stream(rectangle) </text---> "
       "<text>_ insertRindex)</text--->"
       "<text>Inserts all the rectangles within the "
       "stream into an Rindex</text--->"
       "<text>query strassen feed extend[B : bbox(.geoData)] "
       "projecttransformstream[B] insertRindex count  </text--->"
             ") )";




Operator insertRindex (
  "insertRindex",
  insertRindexSpec,
  insertRindexVM,
  Operator::SimpleSelect,
  insertRindexTM);

/*
findRIndex

This operator recieved a tuple stream and an attribute name whith 
type Rectangle<2> within this tuple stream.
A third parameter is a rectangle

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



const string findRindexSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text> stream(tuple) x attrName x rect -> stream(tupleid) </text---> "
      "<text>_ testRindex [_ , _ ] </text--->"
      "<text>Includes the rectangles referenced by the "
      "attribute name into a Rindex,"
      " After that, it searches on this structure using the "
      "rectangle and returns all"
      " tuple ids whose rectangles are intersecting"
      " the query rectangle </text--->"
      "<text>query strassen feed extend[B : bbox(.geoData)] "
      "findRindex[B, [const rect value (-10 -10 10 10)] count  </text--->"
             ") )";




Operator findRindex (
  "findRindex",
  findRindexSpec,
  findRindexVM,
  Operator::SimpleSelect,
  findRindexTM);


/*
Operator heightRIndex

This operator creates an RIndex from a stream and computes its height.

*/

ListExpr heightRindexTM(ListExpr args){
   string err = "stream(rect) expected";
   if(!nl->HasLength(args,1)){
      return listutils::typeError(err);
   }

   if(!Stream<Rectangle<2> >::checkType(nl->First(args))){
      return listutils::typeError(err);
   }
   return nl->SymbolAtom(CcInt::BasicType());
}



int heightRindexVM( Word* args, Word& result, int message,
                    Word& local, Supplier s ){


   Stream<Rectangle<2> > stream(args[0]);
   Rectangle<2>* r;
   stream.open();
   r = stream.request();
   RIndex<2,int> ind;
   while(r){
     ind.insert(*r,1);
     r->DeleteIfAllowed();
     r = stream.request();
   }
   stream.close();
   result = qp->ResultStorage(s);
   CcInt* res = static_cast<CcInt*>(result.addr);
   res->Set(true,ind.height());
   return 0;
}


const string heightRindexSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text> stream(rect)  -> int </text---> "
      "<text>_ heightRindex  </text--->"
      "<text>Creates an RIndex form the stream and returns its height "
      " </text--->"
      "<text>query strassen feed heightRIndex "
      " </text--->"
             ") )";


Operator heightRindex (
  "heightRindex",
  heightRindexSpec,
  heightRindexVM,
  Operator::SimpleSelect,
  heightRindexTM);


/*
Operator statRIndex

This operator creates an RIndex from a stream returns statistical 
information as text.

*/

ListExpr statRindexTM(ListExpr args){
   string err = "stream(rect) or stream(tuple) x attrname expected";
   int len = nl->ListLength(args);
   if((len!=2)){
      return listutils::typeError(err);
   }

   if(Stream<Rectangle<2> >::checkType(nl->First(args))){
      return nl->SymbolAtom(FText::BasicType());
   }
   if(len!=2){
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



int statRindexVMS( Word* args, Word& result, int message,
                    Word& local, Supplier s ){
   Stream<Rectangle<2> > stream(args[0]);
   Rectangle<2>* r;
   stream.open();
   r = stream.request();
   RIndex<2,int> ind;
   while(r){
     ind.insert(*r,1);
     r->DeleteIfAllowed();
     r = stream.request();
   }
   stream.close();
   result = qp->ResultStorage(s);
   FText* res = static_cast<FText*>(result.addr);
   stringstream out;
   out << "ind.countEntriesm =" << ind.countEntries() << endl;
   out << "ind.height() = " << ind.height() << endl; 
   out << "noLeafs() = " << ind.noLeafs() << endl;
   out << "dim0Entries() = " << ind.dim0Entries() << endl;
   out << "dim0Leafs() = " << ind.dim0Leafs() << endl;
   out << "maxBranches() == " << ind.maxBranches() << endl;
   out << "branches() = " << ind.branches() << endl;
   out << "usedMem() = " << ind.usedMem() << endl;
   res->Set(true,out.str());
   return 0;
}


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


int statRindexVMTS( Word* args, Word& result, int message,
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
   out << "ind.countEntriesm =" << ind.countEntries() << endl;
   out << "ind.height() = " << ind.height() << endl; 
   out << "noLeafs() = " << ind.noLeafs() << endl;
   out << "dim0Entries() = " << ind.dim0Entries() << endl;
   out << "dim0Leafs() = " << ind.dim0Leafs() << endl;
   out << "maxBranches() == " << ind.maxBranches() << endl;
   out << "branches() = " << ind.branches() << endl;
   size_t um = ind.usedMem();
   out << "usedMem() = " << ind.usedMem() << " (" << hMem(um) << ")" << endl;
   out << "used mem by tuples = " << tupleSizes 
       << " (" << hMem(tupleSizes) << ")" <<  endl;
   res->Set(true,out.str());
   return 0;
}



const string statRindexSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text> stream(rect) x ANY -> text \n"
      " stream(tuple) x attrname -> text</text---> "
      "<text>_ statRindex  </text--->"
      "<text>Creates an RIndex from the stream and returns statistics as text"
      " The ANY parameter is just a dummy."
      " </text--->"
      "<text>query strassen extend [B : bbox(.geoData)] feed statRIndex[B] "
      " </text--->"
             ") )";

int statRindexSelect(ListExpr args){
  if(Stream<Rectangle<2> >::checkType(nl->First(args))){
     return 0;
  } else {
     return 1;
  }
}


ValueMapping statRindexVM[] = {
     statRindexVMS, statRindexVMTS
  };


Operator statRindex (
  "statRindex",
  statRindexSpec,
  2,
  statRindexVM,
  statRindexSelect,
  statRindexTM);

/*
4. Implementation of a spatial join.

This version of the spatial join first inserts all TupleIDs together 
with the corresponding
rectangle into an RIndex. After that, the second stream is scanned. 
For each stream element,
a search is performed on the rindex.
It returns only pairs of tuple IDs.

*/

ListExpr joinRindexTM(ListExpr args){
   string err = "stream(tuple) x stream(tuple) x a_i x b_j expected";
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
                   nl->TwoElemList(nl->IntAtom(index1-1), 
                                   nl->IntAtom(index2-1)),
                   resList);
}


ListExpr realJoinRindexTM(ListExpr args){


   string err = "stream(tuple) x stream(tuple) x a_i x b_j expected";
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
 
   ListExpr attrList = listutils::concat(attrList1, attrList2);

   if(!listutils::isAttrList(attrList)){
     return listutils::typeError("name conflicts in tuple streams");
   }

   ListExpr resList = nl->TwoElemList( 
                           nl->SymbolAtom(Stream<Tuple>::BasicType()),
                           nl->TwoElemList(
                               nl->SymbolAtom(Tuple::BasicType()),
                               attrList));


   return nl->ThreeElemList(
                   nl->SymbolAtom(Symbols::APPEND()),
                   nl->TwoElemList(nl->IntAtom(index1-1), 
                                   nl->IntAtom(index2-1)),
                   resList);
}



class JoinRindexLocalInfo{

  public:
     JoinRindexLocalInfo(Word& _s1, Word& _s2, int _i1, int _i2, ListExpr _tt):
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

     ~JoinRindexLocalInfo(){
         s2.close();
         tt->DeleteIfAllowed();
         if(currentTuple){
            currentTuple->DeleteIfAllowed();
         }
      }

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
         result->PutAttribute(0, new TupleIdentifier(true, p1.second));
         result->PutAttribute(1, 
                        new TupleIdentifier(currentTuple->GetTupleId())); 
         return result;

      }

     
     


   private:
      RIndex<2,TupleId> ind;
      Stream<Tuple> s2;
      int i2;
      TupleType* tt;
      vector<pair<Rectangle<2>,TupleId> > lastRes;
      Tuple* currentTuple;
};


class SymmJoinRindexLocalInfo{
  public:
     SymmJoinRindexLocalInfo(Word& _s1, Word& _s2, 
                             int _i1, int _i2, 
                             ListExpr _tt):
        ind1(),ind2(), s1(_s1), s2(_s2), 
        i1(_i1), i2(_i2), tt(0), lastRes(), 
        currentTupleId(0){
        tt = new TupleType(_tt);
        s1.open();
        s2.open();
        next = false; 
     }

     ~SymmJoinRindexLocalInfo(){
        s1.close();
        s2.close();
        tt->DeleteIfAllowed();
     }

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

      RIndex<2,TupleId> ind1;
      RIndex<2,TupleId> ind2;
      Stream<Tuple> s1;
      Stream<Tuple> s2;
      int i1;
      int i2;
      TupleType* tt;
      vector<pair<Rectangle<2>,TupleId> > lastRes;
      TupleId currentTupleId;
      bool next;
};



template <class Tree>
class RealJoinRindexLocalInfo{

  public:
     RealJoinRindexLocalInfo(Word& _s1, Word& _s2, int _i1, 
                             int _i2, ListExpr _tt):
         ind(), s2(_s2),  i2(_i2) {

        // build the index from the first stream
        tt = new TupleType(_tt);
        Stream<Tuple> s1(_s1);
        s1.open();
        Tuple* t = s1.request();
        if(t){
           vec = new vector<Tuple*>; // Relation(t->GetTupleType(),true);
        } else {
           vec = 0;
        }
        while(t){
           //relation->AppendTuple(t);
           vec->push_back(t);
           //TupleId id = t->GetTupleId();
           TupleId id = vec->size()-1;
           Rectangle<2>* r = (Rectangle<2>*)t->GetAttribute(_i1);
           ind.insert(*r, id);
           //t->DeleteIfAllowed(); 
           t = s1.request();
        }
        s1.close();
        s2.open();
        currentTuple = 0;
     }

     RealJoinRindexLocalInfo(Word& _s1, Word& _s2, int _i1, 
                             int _i2, ListExpr _tt, int min, int max):
         ind(min,max), s2(_s2),  i2(_i2) {

        // build the index from the first stream
        tt = new TupleType(_tt);
        Stream<Tuple> s1(_s1);
        s1.open();
        Tuple* t = s1.request();
        if(t){
           vec = new vector<Tuple*>; // Relation(t->GetTupleType(),true);
        } else {
           vec = 0;
        }
        while(t){
           //relation->AppendTuple(t);
           vec->push_back(t);
           //TupleId id = t->GetTupleId();
           TupleId id = vec->size()-1;
           Rectangle<2>* r = (Rectangle<2>*)t->GetAttribute(_i1);
           ind.insert(*r, id);
           //t->DeleteIfAllowed(); 
           t = s1.request();
        }
        s1.close();
        s2.open();
        currentTuple = 0;
     }

     ~RealJoinRindexLocalInfo(){
         s2.close();
         tt->DeleteIfAllowed();
         if(vec){
             for(unsigned int i =0; i<vec->size(); i++){
               vec->at(i)->DeleteIfAllowed();
             }
             delete vec;
         }
         //if(relation){
         //   delete relation;
         //}
         if(currentTuple){
            currentTuple->DeleteIfAllowed();
         }
         
      }

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
         //Tuple* t1 = relation->GetTuple(p1.second, false);
         Tuple* t1 = vec->at(p1.second);
         Concat(t1, currentTuple, result);
         //t1->DeleteIfAllowed();         
         return result;
      }

   private:
      Tree ind;
      Stream<Tuple> s2;
      int i2;
      TupleType* tt;
      vector<pair<Rectangle<2>,TupleId> > lastRes;
      Tuple* currentTuple;
      //Relation* relation;
      vector<Tuple*>* vec;
};

template<class JLI>
int joinRindexVM( Word* args, Word& result, int message,
                    Word& local, Supplier s ){

   JLI* li = static_cast<JLI*>(local.addr);
 
   switch(message){
        case OPEN: {
             if(li){
               delete li;
             }
             int i1 = ((CcInt*)(args[4].addr))->GetValue();
             int i2 = ((CcInt*)(args[5].addr))->GetValue();
             local.addr = new JLI(args[0], args[1],i1,i2,
                                  nl->Second(GetTupleResultType(s)));
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


const string joinRindexSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text> stream(tuple) x stream(tuple) x a_i x b_j -> "
      "stream(tuple) </text---> "
      "<text>_ _ joinRIndex  </text--->"
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


Operator joinRindex (
  "joinRindex",
  joinRindexSpec,
  joinRindexVM<JoinRindexLocalInfo>,
  Operator::SimpleSelect,
  joinRindexTM);



const string symmJoinRindexSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text> stream(tuple) x stream(tuple) x a_i x b_j -> "
      "stream(tuple) </text---> "
      "<text>_ _ symmJoinRindex  </text--->"
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


Operator symmJoinRindex (
  "symmJoinRindex",
  symmJoinRindexSpec,
  joinRindexVM<SymmJoinRindexLocalInfo>,
  Operator::SimpleSelect,
  joinRindexTM);


const string realJoinRindexSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text> stream(tuple(X)) x stream(tuple(Y)) x a_i x b_j -> "
      "stream(tuple(X o Y)) </text---> "
      "<text>_ _ realJoinRindex  </text--->"
      "<text>Performes a spatial join on two streams. "
      "The attributes a_i and b_j must"
      " be of type rect. The result is a stream of "
      "tuples with intersecting rectangles build as concatenation"
      " of the source tuples"
      " </text--->"
      "<text>query strassen feed extend[B : bbox(.geoData] strassen "
      "feed extend[B : bbox(.geoData)] realJoinRindex[B,B] count "
      " </text--->"
             ") )";

Operator realJoinRindex (
  "realJoinRindex",
  realJoinRindexSpec,
  joinRindexVM<RealJoinRindexLocalInfo<RIndex<2, TupleId> > >,
  Operator::SimpleSelect,
  realJoinRindexTM);



/*
Operator realJoinMMRTRee

*/

ListExpr realJoinMMRTreeTM(ListExpr args){

  // similar to the version using an RIndex, but 2 additional integer values are
  // required for the min and max entry value for an rtree node
  string err = "stream(tuple(A)) x stream(tuple(B)) x"
               " a_i x b_i x int x int expected";
   if(!nl->HasLength(args,6)){
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


   return nl->ThreeElemList(
                   nl->SymbolAtom(Symbols::APPEND()),
                   nl->TwoElemList(nl->IntAtom(index1-1), 
                                   nl->IntAtom(index2-1)),
                   resList);

}

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
             int i1 = ((CcInt*)(args[6].addr))->GetValue();
             int i2 = ((CcInt*)(args[7].addr))->GetValue();
             local.addr = new JLI(args[0], args[1],i1,i2,
                                  nl->Second(GetTupleResultType(s)), min, max);
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


const string realJoinMMRTreeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> stream(tuple(A)) x stream(tuple(B)) x a_i x b_j x int x int-> "
    "stream(tuple(X o Y)) </text---> "
    "<text>streamA streamB realJoinMMRTree[attrnameA, attrnameB, min, max]"
  "  </text--->"
    "<text>Performes a spatial join on two streams. "
    "The attributes a_i and b_j must"
    " be of type rect. The result is a stream of "
    "tuples with intersecting rectangles build as concatenation"
    " of the source tuples. Min and max define the range for the number of"
    " entries within the nodes of the used rtree."
    " </text--->"
    "<text>query strassen feed extend[B : bbox(.geoData] strassen "
    "feed extend[B : bbox(.geoData)] {a} realJoinMMRTree[B,B_a,8,16] count "
    " </text--->"
         ") )";

Operator realJoinMMRTree(
  "realJoinMMRTree",
  realJoinMMRTreeSpec,
  joinRTreeVM<RealJoinRindexLocalInfo<mmrtree::RtreeT<2, TupleId> > >,
  Operator::SimpleSelect,
  realJoinMMRTreeTM 
 );




/*
Now, we use a main  memory implementation of an r-tree for comparisons

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
                  if(min2 <1 || min2 >= max2){
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

const string insertMMRTreeSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
       "( <text> stream(rectangle) x int x int -> stream(rectangle) </text---> "
       "<text>_ insertMMRTRee [min , max] </text--->"
       "<text>Inserts all the rectangles within the "
       "stream into a main memory based rtree </text--->"
       "<text>query strassen feed extend[B : bbox(.geoData)] "
       "projecttransformstream[B] insertMMRTRee[4,8]  count  </text--->"
             ") )";




Operator insertMMRTree (
  "insertMMRTree",
  insertMMRTreeSpec,
  insertMMRTreeVM,
  Operator::SimpleSelect,
 insertMMRTreeTM);







class RIndexAlgebra : public Algebra {
  public:
   RIndexAlgebra() : Algebra()
   {
      AddOperator(&insertRindex);
      AddOperator(&findRindex);
      AddOperator(&heightRindex);
      AddOperator(&statRindex);
      AddOperator(&joinRindex);
      AddOperator(&symmJoinRindex);
      AddOperator(&realJoinRindex);
      
      // operators using mmrtrees
      AddOperator(&insertMMRTree);
      AddOperator(&realJoinMMRTree);

   }
};

extern "C"
Algebra*
InitializeRIndexAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new RIndexAlgebra());
}



