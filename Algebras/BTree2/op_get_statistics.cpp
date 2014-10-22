/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of the getstatistics Operator

[TOC]

0 Overview

*/

#include "op_get_statistics.h"

#include "WinUnix.h"

#include "ListUtils.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"
#include "Symbols.h"

#include "BTree2.h"

#include <limits>

extern NestedList* nl;
extern QueryProcessor *qp;

namespace BTree2Algebra {
namespace Operators {

ListExpr get_statistics::TypeMapping(ListExpr args)
{
  if ((nl->ListLength(args) != 1)) {
    return listutils::typeError("expecting one argument for get_statistics");
  }
  if (nl->ListLength(args) != 0) {
    ListExpr first = nl->First(args);
    if (!listutils::isBTree2Description(first)) {
      return listutils::typeError("argument must be a valid btree2");
    }
  }

  ListExpr res = nl->OneElemList(
                   nl->TwoElemList(nl->SymbolAtom("TotalInternal"),
                                   nl->SymbolAtom(CcInt::BasicType())));

  ListExpr res2 = nl->Append(res, nl->TwoElemList(nl->SymbolAtom("TotalLeaf"),
                                   nl->SymbolAtom(CcInt::BasicType())));


  res2 = nl->Append(res2,nl->TwoElemList(nl->SymbolAtom("UnderflowInternal"),
                                   nl->SymbolAtom(CcInt::BasicType())));

  res2 = nl->Append(res2,nl->TwoElemList(nl->SymbolAtom("UnderflowLeaf"),
                                   nl->SymbolAtom(CcInt::BasicType())));

  res2 = nl->Append(res2,nl->TwoElemList(nl->SymbolAtom("EntriesInternal"),
                                   nl->SymbolAtom(CcInt::BasicType())));

  res2 = nl->Append(res2,nl->TwoElemList(nl->SymbolAtom("EntriesLeaf"),
                                   nl->SymbolAtom(CcInt::BasicType())));

  res2 = nl->Append(res2,nl->TwoElemList(
                                   nl->SymbolAtom("MissingEntriesInternal"),
                                   nl->SymbolAtom(CcInt::BasicType())));

  res2 = nl->Append(res2,nl->TwoElemList(nl->SymbolAtom("MissingEntriesLeaf"),
                                   nl->SymbolAtom(CcInt::BasicType())));

  res2 = nl->Append(res2,nl->TwoElemList(nl->SymbolAtom("MinEntriesInternal"),
                                   nl->SymbolAtom(CcInt::BasicType())));

  res2 = nl->Append(res2,nl->TwoElemList(nl->SymbolAtom("MinEntriesLeaf"),
                                   nl->SymbolAtom(CcInt::BasicType())));

  res2 = nl->Append(res2,nl->TwoElemList(nl->SymbolAtom("MaxEntriesInternal"),
                                   nl->SymbolAtom(CcInt::BasicType())));

  res2 = nl->Append(res2,nl->TwoElemList(nl->SymbolAtom("MaxEntriesLeaf"),
                                   nl->SymbolAtom(CcInt::BasicType())));

  res2 = nl->Append(res2,nl->TwoElemList(nl->SymbolAtom("BytesWastedInternal"),
                                   nl->SymbolAtom(CcInt::BasicType())));

  res2 = nl->Append(res2,nl->TwoElemList(nl->SymbolAtom("BytesWastedLeaf"),
                                   nl->SymbolAtom(CcInt::BasicType())));

  res2 = nl->Append(res2,nl->TwoElemList(
                                   nl->SymbolAtom("Peak Cache Memory Usage"),
                                   nl->SymbolAtom(CcInt::BasicType())));

  res2 = nl->Append(res2,nl->TwoElemList(nl->SymbolAtom("Peak Cache Elements"),
                                   nl->SymbolAtom(CcInt::BasicType())));


  return nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                          nl->TwoElemList(
                            nl->SymbolAtom(Tuple::BasicType()), res));

}

int get_statistics::Select( ListExpr args )
{
  return 0;
}

int
get_statistics::ValueMappingEx(Word* args, Word& result, int message,
                            Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BTree2* btree = (BTree2*)args[0].addr;

  switch (message)
  {
    case OPEN : {
      int* k = new int;
      *k = 1;
      local.addr = k;
      return 0;

    }
    case REQUEST : {
      int* k;
      k = (int*)local.addr;
      if (*k == 1) {
        StatisticStruct stat;

        btree->GetStatistics(btree->GetRootNode(), stat);

        const int fields = 16;
        CcInt* r[fields] = {
          new CcInt(stat.Internal.NumberOfNodes),
          new CcInt(stat.Leaf.NumberOfNodes),
          new CcInt(stat.Internal.UnderflowNodes),
          new CcInt(stat.Leaf.UnderflowNodes),
          new CcInt(stat.Internal.Entries),
          new CcInt(stat.Leaf.Entries),
          new CcInt(stat.Internal.MissingEntries),
          new CcInt(stat.Leaf.MissingEntries),
          new CcInt(btree->GetMinEntries(true)),
          new CcInt(btree->GetMinEntries(false)),
          new CcInt(btree->GetMaxEntries(true)),
          new CcInt(btree->GetMaxEntries(false)),
          new CcInt(stat.Internal.BytesWasted),
          new CcInt(stat.Leaf.BytesWasted),
          new CcInt((int) btree->GetPeakCacheMemoryUsage()),
          new CcInt(btree->GetPeakCacheElements())
        };

        Tuple* t = new Tuple( nl->Second(GetTupleResultType(s)) );
        for (int i = 0; i < fields; i++) {
          t->PutAttribute(i, r[i]);
        }

        result.addr = t;
        *k = 0;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE : {

      delete (int*) local.addr;
      local.addr = 0;
      return 0;
    }
  }
  return 0;

  CcInt *res = (CcInt*) result.addr;
  res->Set( true, btree->GetMaxKeysize() );
  return 0;
}

string get_statistics::Specification() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = "(btree2 Tk Td u) -> stream(tuple((TotalInternal int)"
               "(TotalLeaf int)(UnderflowInternal int)(UnderflowLeaf int)"
               "(EntriesInternal int)(EntriesLeaf int)(MissingEntriesInternal"
               " int)(MissingEntriesLeaf int)(MinEntriesInternal int)"
               "(MinEntriesLeaf int)(MaxEntriesInternal int)(MaxEntriesLeaf"
               " int)(BytesWastedInternal int)(BytesWastedLeaf int)))";
  string spec = "get_statistics(_)";
  string meaning = "Interesting things...";

  string example = "query get_statistics(Staedte_SName_btree2)";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

int get_statistics::numberOfValueMappings = 1;
ValueMapping get_statistics::valueMappings[] = {
                  get_statistics::ValueMappingEx,
                };

Operator get_statistics::def (
          "get_statistics",                     // name
          get_statistics::Specification(),      // specification
          get_statistics::numberOfValueMappings,// number of functions
          get_statistics::valueMappings,        // value mapping
          get_statistics::Select,               // trivial selection function
          get_statistics::TypeMapping           // type mapping
);

} // end namespace operators
} // end namespace btree2algebra

