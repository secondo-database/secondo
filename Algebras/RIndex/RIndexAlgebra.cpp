
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


#include "RIndex.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Stream.h"
#include "ListUtils.h"
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"


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

            cout << "Rectangles inserted" << endl;
            cout << "#entries = " << index.countEntries() << endl;
            cout << "#tuples = " << c << endl; 

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
   cout << "ind.countEntriesm =" << ind.countEntries();
   cout << "ind.height() = " << ind.height(); 
   cout << "noLeafs() = " << ind.noLeafs();
   cout << "dim0Entries() = " << ind.dim0Entries();
   cout << "dim0Leafs() = " << ind.dim0Leafs();
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



class RIndexAlgebra : public Algebra {
  public:
   RIndexAlgebra() : Algebra()
   {
      AddOperator(&insertRindex);
      AddOperator(&findRindex);
      AddOperator(&heightRindex);

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



