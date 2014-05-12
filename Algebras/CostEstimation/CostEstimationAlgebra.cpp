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

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Progress.h"
#include "Stream.h"
#include "RelationAlgebra.h"


extern NestedList* nl;
extern QueryProcessor* qp;



/*
1 Operator getCostConstant


*/

ListExpr getCostConstantTM(ListExpr args){
  string err = "string x string x string expected";
  if(!nl->HasLength(args,3)){
     return listutils::typeError(err);
  }
  if(   !CcString::checkType(nl->First(args)) ||
        !CcString::checkType(nl->Second(args)) ||
        !CcString::checkType(nl->Third(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcReal>();
}


int getCostConstantVM(
      Word* args, Word& result, int message,
      Word& local, Supplier s ){

   CcString* algName = (CcString*) args[0].addr;
   CcString* opName = (CcString*) args[1].addr;
   CcString* constName = (CcString*) args[2].addr;
   result = qp->ResultStorage(s);
   CcReal* res = (CcReal*) result.addr;
   if(!algName->IsDefined() || !opName->IsDefined() 
      || ! constName->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   try{
      double v = ProgressConstants::getValue(algName->GetValue(), 
                                             opName->GetValue(), 
                                             constName->GetValue());
      res->Set(true,v);
   } catch(...){
      res->SetDefined(false);
   }
   return 0;
}


OperatorSpec getCostConstantSpec(
   " string x string x string -> real",
   " getCostConstant(AlgName, OpName, ConstName)",
   "Returns the asked progress constant",
   "query getCostConstant(\"ExtRelation2Algebra\","
   "\"itHashJoin\",\"vItHashJoin\")"
);



Operator getCostConstantOP (
   "getCostConstant",             //name
   getCostConstantSpec.getStr(),           //specification
   getCostConstantVM,        //value mapping
   Operator::SimpleSelect,         //trivial selection function
   getCostConstantTM        //type mapping
);


/*
getCostConstants

returns all known costs constants within a tuple stream

*/

ListExpr getCostConstantsTM(ListExpr args){

  if(nl->IsEmpty(args)){
    return nl->TwoElemList( 
                  listutils::basicSymbol<Stream<Tuple> >(),
                  nl->TwoElemList(
                         listutils::basicSymbol<Tuple>(),
                         nl->FourElemList(
                            nl->TwoElemList(
                                   nl->SymbolAtom("AlgName"),
                                   listutils::basicSymbol<CcString>()),      
                            nl->TwoElemList(
                                   nl->SymbolAtom("OpName"),
                                   listutils::basicSymbol<CcString>()),      
                            nl->TwoElemList(
                                   nl->SymbolAtom("ConstName"),
                                   listutils::basicSymbol<CcString>()),      
                            nl->TwoElemList(
                                   nl->SymbolAtom("Constant"),
                                   listutils::basicSymbol<CcReal>()))));
  } 
  return listutils::typeError("no arguments expected");
}

class GetCostConstantsLocal{

  public:
    GetCostConstantsLocal(ListExpr tupleType){
       tt = new TupleType(tupleType);
       values  = ProgressConstants::getValues();
       pos = 0;
    }

    Tuple* next(){
     if(pos>=values.size()){
       return 0;
     }
     Tuple* t = new Tuple(tt);
     t->PutAttribute(0,new CcString(true, values[pos].first[0]));
     t->PutAttribute(1,new CcString(true, values[pos].first[1]));
     t->PutAttribute(2,new CcString(true, values[pos].first[2]));
     t->PutAttribute(3,new CcReal(true, values[pos].second));
     pos++;
     return t;
    }

   private:
     TupleType* tt;
     vector<pair<vector<string>, double> > values;
     size_t pos;

};

int getCostConstantsVM(
      Word* args, Word& result, int message,
      Word& local, Supplier s ){
   GetCostConstantsLocal* li = (GetCostConstantsLocal*) local.addr;
   switch(message){
     case OPEN:
           if(li){ delete li; }
           local.addr = new GetCostConstantsLocal(
                                  nl->Second(GetTupleResultType(s)));
           return 0;
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

OperatorSpec getCostConstantsSpec(
   "  -> stream(tuple(( ALgName string) (OpName string) "
   "(ConstName string) (Constant real)))",
   " getCostConstants()",
   "Returns the names and values of all cost constants.",
   "query getCostConstants() count"
);



Operator getCostConstantsOP (
   "getCostConstants",             //name
   getCostConstantsSpec.getStr(),           //specification
   getCostConstantsVM,        //value mapping
   Operator::SimpleSelect,         //trivial selection function
   getCostConstantsTM        //type mapping
);


 

class CostEstimationAlgebra : public Algebra{
    public:
        CostEstimationAlgebra() : Algebra() {
          AddOperator(&getCostConstantOP);
          AddOperator(&getCostConstantsOP);
        }
};


extern "C"
Algebra*
InitializeCostEstimationAlgebra( NestedList* nlRef, 
                         QueryProcessor* qpRef, 
                         AlgebraManager* amRef ) {
    CostEstimationAlgebra* ptr = new CostEstimationAlgebra();
    return ptr;
  }



