/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of the treeheight Operator

[TOC]

0 Overview

*/

#include "op_set_cache_limit_type.h"

#include "ListUtils.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"
#include "BTree2.h"
#include "Symbols.h"

#include <limits>

extern NestedList* nl;
extern QueryProcessor *qp;

namespace BTree2Algebra {
namespace Operators {

ListExpr set_cache_limit_type::TypeMapping( ListExpr args){
  if(nl->ListLength(args) != 2){
    return listutils::typeError("Operator expects two arguments");
  }
  ListExpr arg1 = nl->First(args);
  if(!listutils::isBTree2Description(arg1)){
     return listutils::typeError("first argument must be a btree2");
  }

  ListExpr arg2 = nl->Second(args);
  if (nl->AtomType(arg2)!=SymbolType){
     return listutils::typeError("second argument is not valid");
  }

  string name = nl->SymbolValue(arg2);
  int v = 0;

  if (name == "fixed") {
    v = 1;
  } else if (name == "mem") {
    v = 2;
  }

  if (v == 0){
     return listutils::typeError("second argument is not valid");
  }

  ListExpr appendArgs = nl->OneElemList(nl->IntAtom(v));

  return nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
                              appendArgs,
                              nl->OneElemList(
                                nl->SymbolAtom(CcBool::BasicType())));
}

int
set_cache_limit_type::ValueMapping(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BTree2* btree = (BTree2*)args[0].addr;
  unsigned bc = ((CcInt*)args[1].addr)->GetIntval();
  CcBool *res = (CcBool*) result.addr;
  if (bc == 1) {
    btree->SetFixedElementsLimitType();
  }
  if (bc == 2) {
    btree->SetMemoryLimitType();
  }
  bool sc = true;
  res->Set( true, sc );
  return 0;

}

struct setCacheTypeInfo : OperatorInfo {

  setCacheTypeInfo() : OperatorInfo()
  {
    name =      "set_cache_limit_type";
    signature = "(btree2 Tk Td u) x {fixed | mem } -> bool";
    syntax =    "set_cache_limit_type ( _ , _ )";
    meaning =   "Sets the limit type.";
    example =   "query set_cache_limit_type (Staedte_Bev_btree2, fixed)";
  }
};

Operator set_cache_limit_type::def( setCacheTypeInfo(),
                                     set_cache_limit_type::ValueMapping,
                                     set_cache_limit_type::TypeMapping);

} // end namespace operator

} // end namespace btree2algebra


