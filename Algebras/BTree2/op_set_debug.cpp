/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of the treeheight Operator

[TOC]

0 Overview

*/

#include "op_set_debug.h"

#include "ListUtils.h"
#include "QueryProcessor.h"
#include "BTree2.h"
#include "Symbols.h"

#include <limits>

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;

namespace BTree2Algebra {
namespace Operators {

ListExpr set_debug::TypeMapping( ListExpr args){
  if(nl->ListLength(args) != 1){
    return listutils::typeError("Operator expects one argument");
  }
  ListExpr arg = nl->First(args);
  if (nl->AtomType(arg)!=SymbolType){
     return listutils::typeError("argument is not valid");
  }

  string name = nl->SymbolValue(arg);
  int v = 0;

  if (name == "printTree") {
    v = 1;
  } else if (name == "printCache") {
    v = 2;
  } else if (name == "printNodeLoading") {
    v = 3;
  }

  if (v == 0){
     return listutils::typeError("argument is not valid");
  }

  ListExpr appendArgs = nl->OneElemList(nl->IntAtom(v));

  return nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
                              appendArgs,
                              nl->OneElemList(
                                nl->SymbolAtom(CcBool::BasicType())));
}

int
set_debug::ValueMapping(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  unsigned bc = ((CcInt*)args[0].addr)->GetIntval();
  CcBool *res = (CcBool*) result.addr;
  if (bc == 1) {
    BTree2::SetDbgPrintTree(true);
  }
  if (bc == 2) {
    BTree2::SetDbgPrintCache(true);
  }
  if (bc == 3) {
    BTree2::SetDbgPrintNodeLoading(true);
  }
  bool sc = true;
  res->Set( true, sc );
  return 0;

}

struct setDebugInfo : OperatorInfo {

  setDebugInfo() : OperatorInfo()
  {
    name =      "set_debug";
    signature = "{ printTree | printCache | printNodeLoading } -> bool";
    syntax =    "set_debug ( _ )";
    meaning =   "Sets the debug mode of btree2s.";
    example =   "query set_debug (printTree)";
  }
};

Operator set_debug::def( setDebugInfo(), set_debug::ValueMapping,
                                           set_debug::TypeMapping);

} // end namespace operator

} // end namespace btree2algebra


