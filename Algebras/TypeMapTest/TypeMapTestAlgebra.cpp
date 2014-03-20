

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


1 TypeMapTestAlgebra

This algebra is just for testing the implementation of the 
type mapper within the tools directory. It provides operators
calling the functions  of this mapper class.

1.1. includes and global variables

*/


#include "Algebra.h"
#include "NestedList.h"

#include "QueryProcessor.h"

#include "../../Tools/TypeMap/Mapper.h"

#include "StandardTypes.h"
#include "FTextAlgebra.h"
#include "ListUtils.h"

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;


static NestedList* pnl; // Nested list storage for persistent 
                        //storing of nested lists
static typemap::Mapper* mapper;  // instance of the mapper class




/*
2 Operators

2.1 Operator tminit

2.1.1 Type mapping

*/
ListExpr tminitTM(ListExpr args){

  string err = "string or text expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  ListExpr a = nl->First(args);
  if(CcString::checkType(a) || FText::checkType(a)){
    return listutils::basicSymbol<CcBool>();
  } 
  return listutils::typeError(err);
}


/*
2.1.2 Value Mapping

*/
void createMapper(){
   pnl = new NestedList();
   mapper = new typemap::Mapper(pnl,nl);
}




template<class T>
int tminitVM1 (Word* args, Word& result, int message, Word& local,
              Supplier s ){

  if(mapper==0){
     createMapper();
  }
 
  T* arg = (T*) args[0].addr;
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  if(!arg->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  bool success = mapper->init(arg->GetValue());  
  res->Set(true, success);
  return 0;  
}


/*
2.1.3 Value Mapping Array and Selection function

*/

ValueMapping tminitVM[] = {
    tminitVM1<CcString>,
    tminitVM1<FText>
};


int tminitSelect(ListExpr args){
   return CcString::checkType(nl->First(args))?0:1;
}

/*
2.1.4 Specification

*/

OperatorSpec tminitSpec(
        " {string, text} -> bool",
        " tminit(_)",
        " Calls the init function of the type mapper",
         "query tminit('../Algebras/tmspecs')"
    );

/*
2.1.5 Operator instance

*/

Operator tminitOP(
   "tminit",
   tminitSpec.getStr(),
   2,
   tminitVM,
   tminitSelect,
   tminitTM
);


/*
2.2 OPerator getOpSig


2.1.1 Type Map

*/
ListExpr getOpSigTM(ListExpr args){

  string err = " {string, text} x string expected" ;
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  if((!CcString::checkType(nl->First(args)) &&
     (!FText::checkType(nl->First(args)))) ||
     !CcString::checkType(nl->Second(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<FText>();
}


template<class T>
int getOpSigVM1 (Word* args, Word& result, int message, Word& local,
              Supplier s ){
   
   T* an = (T*) args[0].addr;
   CcString* on = (CcString*) args[1].addr;
   result = qp->ResultStorage(s);
   FText* res = (FText*) result.addr;
   if(!an->IsDefined() || !on->IsDefined()){
     res->SetDefined(false);
     return 0;
   } 
   if(mapper==0){
     res->Set("getOpSig called without tminit");
     return 0;
   }
   string v = nl->ToString(mapper->getOpSig(an->GetValue(),on->GetValue()));
   res->Set(true,v);
   return 0;
}


/*
2.2.3 Value Mapping Array and Selection function

*/

ValueMapping getOpSigVM[] = {
    getOpSigVM1<CcString>,
    getOpSigVM1<FText>
};


int getOpSigSelect(ListExpr args){
   return CcString::checkType(nl->First(args))?0:1;
}

/*
2.2.4 Specification

*/

OperatorSpec getOpSigSpec(
        " {string, text}  x string -> text",
        " tmgetOpSig(algname, opname)",
        " Calls the getOpSig function of the type mapper",
         "query tmgetOpSig('StandardAlgebra', \"+\")"
    );

/*
2.2.5 Operator instance

*/

Operator getOpSigOP(
   "tmgetOpSig",
   getOpSigSpec.getStr(),
   2,
   getOpSigVM,
   getOpSigSelect,
   getOpSigTM
);


/*
2.3 Operator TypeMap

*/

ListExpr tmtypemapTM(ListExpr args){

  string err = "text x any x any x ... expected";
  if(nl->ListLength(args)<1){
    return listutils::typeError(err);
  }
  if(!FText::checkType(nl->First(args))){
     string err = "text x any x any x ... expected";
  }
  return  nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            nl->OneElemList(nl->TextAtom(nl->ToString(nl->Rest(args)))),
            listutils::basicSymbol<FText>());
}



int tmtypemapVM (Word* args, Word& result, int message, Word& local,
              Supplier s ){


   FText* sigArgsType = (FText*) args[0].addr;
   result = qp->ResultStorage(s);
   FText* res = (FText*) result.addr;

   FText* currentArgs = (FText* ) args[qp->GetNoSons(s)-1].addr;

   ListExpr sat;
   ListExpr cat;
   if(!sigArgsType->IsDefined() ||
      !currentArgs->IsDefined()){
     res->SetDefined(false);
     return 0;
   }

   if(pnl==0 || mapper==0){
      res->Set(true, "missing initialization, call tmpinit before");
      return 0;
   } 

   if(!pnl->ReadFromString(sigArgsType->GetValue(),sat) ||
      !nl->ReadFromString(currentArgs->GetValue(),cat)){
     res->Set(true, "Cannot parse list");
     return 0;
   }   
   if(!mapper){
     res->Set(true, "mapper not initialized");
     return 0;
   }

   res->Set(true, nl->ToString( mapper->typemap(sat,cat)));
   return 0;
}


OperatorSpec tmtypemapSpec(
        "  text x any x any x ... -> text",
        " tmtypemap( sigargslist, current arguments)",
        " Calls the typemape function of the type mapper",
         "query tmgtypemap( tmgetOpSig('StandardAlgebra',\"+\", 3,4))"
    );

Operator tmtypemapOP (
   "tmtypemap",             //name
   tmtypemapSpec.getStr(),           //specification
   tmtypemapVM,        //value mapping
   Operator::SimpleSelect,         //trivial selection function
   tmtypemapTM        //type mapping
 );




class TypeMapTestAlgebra: public Algebra{

  public:
     TypeMapTestAlgebra(): Algebra(){
       // initialize global variables
       pnl = 0;
       mapper = 0;
       AddOperator(&tminitOP);
       AddOperator(&getOpSigOP);
       AddOperator(&tmtypemapOP);
     }
     ~TypeMapTestAlgebra(){
         if(mapper){
             delete mapper;
             mapper = 0;
         }
         if(pnl){
             delete pnl;
             pnl = 0;
         }
      }
};

extern "C"
Algebra*
 InitializeTypeMapTestAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
 {
   nl = nlRef;
   qp = qpRef;
   return (new TypeMapTestAlgebra());
 }









