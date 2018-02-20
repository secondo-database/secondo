/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of the createbtree2 Operator

[TOC]

0 Overview

*/

#include "op_getentry2.h"

#include "ListUtils.h"
#include "QueryProcessor.h"

#include "BTree2.h"
#include "BTree2Iterator.h"

#include <limits>

extern NestedList* nl;
extern QueryProcessor *qp;


using namespace std;

namespace BTree2Algebra {
namespace Operators {

ListExpr getentry2::TypeMapping(ListExpr args)
{
  if ((nl->ListLength(args) != 2) && (nl->ListLength(args) != 1)) {
    return listutils::typeError("one or two arguments expected");
  }
  ListExpr first = nl->First(args);
  if(nl->ListLength(first) < 1){
    return listutils::typeError("first arg is not a btree2");
  }
  if (!listutils::isSymbol(nl->First(first),"btree2")) {
    return listutils::typeError("first arg is not a btree2");
  }

//  if (!listutils::isSymbol(nl->Second(first),second)) {
//    return listutils::typeError("keytype and argument type mismatch");
//  }

  return nl->OneElemList(nl->SymbolAtom(CcInt::BasicType()));
}

int getentry2::Select( ListExpr args )
{
  if (nl->ListLength(args) == 2) {
    return 0;
  }
  if (nl->ListLength(args) == 1) {
    return 1;
  }
  return -1;
}

int
getentry2::ValueMappingEx(Word* args, Word& result, int message,
                            Word& local, Supplier s)
{
  result = qp->ResultStorage(s);

  BTree2* btree = (BTree2*)args[0].addr;
  Attribute* key = ((Attribute*)args[1].addr);
  assert(btree != 0);

  BTree2Iterator bi;

  btree->printNodeInfos();

  for (bi = btree->find(key);
       (bi != btree->end()) && (key->Compare(bi.key()) == 0); ++bi) {
    cout << *(bi.key());
    if ((*bi) != 0) {
       cout << " - " << *(*bi);
    }
    cout << endl;
  }

  return 0;
}

int
getentry2::ValueMappingExAll(Word* args, Word& result, int message,
                            Word& local, Supplier s)
{
  result = qp->ResultStorage(s);

  BTree2* btree = (BTree2*)args[0].addr;

  btree->printNodeInfos();
  assert(btree != 0);

  BTree2Iterator bi;

  for (bi = btree->begin(); bi != btree->end(); ++bi) {
    cout << *(bi.key());
    if ((*bi) != 0) {
       cout << " - " << *(*bi);
    }
    cout << endl;
  }

  return 0;
}

string getentry2::Specification() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "getentry(btree2,arg) -> ";
  string spec = "_ getentry2 [ _ ]";
  string meaning = "Test operator";

  string example = "query getentry2(btrX,12)\n";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

int getentry2::numberOfValueMappings = 2;
ValueMapping getentry2::valueMappings[] = {
                  getentry2::ValueMappingEx,
                  getentry2::ValueMappingExAll,
                };

Operator getentry2::def (
          "getentry2",                     // name
          getentry2::Specification(),      // specification
          getentry2::numberOfValueMappings,// number of overloaded functions
          getentry2::valueMappings,        // value mapping
          getentry2::Select,               // trivial selection function
          getentry2::TypeMapping           // type mapping
);

} // end namespace operators
} // end namespace btree2algebra

