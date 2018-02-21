/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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


//[$][\$]
//[_][\_]

*/
#include "OperatorProject.h"

#include "Algebra.h"
#include "Progress.h"
#include "RelationAlgebra.h"
#include "Algebras/Stream/Stream.h"

#ifdef USE_PROGRESS
#include "../CostEstimation/RelationAlgebraCostEstimation.h"
#endif

using namespace std;

/*
5.9 Operator ~project~

5.9.1 Type mapping function of operator ~project~

Result type of project operation.

----  ((stream (tuple ((x1 T1) ... (xn Tn)))) (ai1 ... aik))  ->
        (APPEND
          (k (i1 ... ik))
          (stream (tuple ((ai1 Ti1) ... (aik Tik))))
        )
----

The type mapping computes the number of attributes and the list of
attribute numbers for the given projection attributes and asks the
query processor to append it to the given arguments.

*/
ListExpr OperatorProject::ProjectTypeMap(ListExpr args)
{
  bool firstcall = true;
  int noAttrs=0, j=0;

  // initialize local ListExpr variables
  ListExpr first=nl->TheEmptyList();
  ListExpr second=first, first2=first,
           attrtype=first, newAttrList=first;
  ListExpr lastNewAttrList=first, lastNumberList=first,
           numberList=first, outlist=first;
  string attrname="", argstr="";

  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("tuplestream x arglist expected");
    return nl->TypeError();
  }
  first = nl->First(args);

  if(!listutils::isTupleStream(first)){
    ErrorReporter::ReportError("first argument has to be a tuple stream");
    return nl->TypeError();
  }

  second = nl->Second(args);

  if(nl->ListLength(second)<=0){
    ErrorReporter::ReportError("non empty attribute name list"
                               " expected as second argument");
    return nl->TypeError();
  }

  noAttrs = nl->ListLength(second);
  set<string> attrNames;
  while (!(nl->IsEmpty(second)))
  {
    first2 = nl->First(second);
    second = nl->Rest(second);
    if (nl->AtomType(first2) == SymbolType)
    {
      attrname = nl->SymbolValue(first2);
    }
    else
    {
      ErrorReporter::ReportError(
        "Attributename in the list is not of symbol type.");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
    if(attrNames.find(attrname)!=attrNames.end()){
       ErrorReporter::ReportError("names within the projection "
                                  "list are not unique");
       return nl->TypeError();
    } else {
       attrNames.insert(attrname);
    }

    j = listutils::findAttribute(nl->Second(nl->Second(first)),
                      attrname, attrtype);
    if (j)
    {
      if (firstcall)
      {
        firstcall = false;
        newAttrList =
          nl->OneElemList(nl->TwoElemList(first2, attrtype));
        lastNewAttrList = newAttrList;
        numberList = nl->OneElemList(nl->IntAtom(j));
        lastNumberList = numberList;
      }
      else
      {
        lastNewAttrList =
          nl->Append(lastNewAttrList,
                     nl->TwoElemList(first2, attrtype));
        lastNumberList =
          nl->Append(lastNumberList, nl->IntAtom(j));
      }
    }
    else
    {
      ErrorReporter::ReportError(
        "Operator project: Attributename '" + attrname +
        "' is not a known attributename in the tuple stream.");
          return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  outlist =
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->TwoElemList(
        nl->IntAtom(noAttrs),
        numberList),
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          newAttrList)));
  return outlist;
}

/*
5.9.2 Value mapping function of operator ~project~

*/

#ifndef USE_PROGRESS

// standard version


int
OperatorProject::Project(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  TupleType* tt = (TupleType*) qp->GetLocal2(s).addr;

  switch (message)
  {
    case INIT : {
       tt = new TupleType(nl->Second(GetTupleResultType(s)));
       qp->GetLocal2(s).addr= tt;
       return 0;
    }

    case FINISH : {
       if(tt){
           tt->DeleteIfAllowed();
           qp->GetLocal2(s).addr=0;
       }
       return 0;
    }

    case OPEN :
    {
      tt->IncReference();
      TupleType *tupleType = tt;
      local.addr = tupleType;

      qp->Open(args[0].addr);
      return 0;
    }
    case REQUEST :
    {
      Word elem1, elem2;
      int noOfAttrs, index;
      Supplier son;

      qp->Request(args[0].addr, elem1);
      if (qp->Received(args[0].addr))
      {
        TupleType *tupleType = (TupleType *)local.addr;
        Tuple *t = new Tuple( tupleType );

        noOfAttrs = ((CcInt*)args[2].addr)->GetIntval();
        assert( t->GetNoAttributes() == noOfAttrs );

        for( int i = 0; i < noOfAttrs; i++)
        {
          son = qp->GetSupplier(args[3].addr, i);
          qp->Request(son, elem2);
          index = ((CcInt*)elem2.addr)->GetIntval();
          t->CopyAttribute(index-1, (Tuple*)elem1.addr, i);
        }
        ((Tuple*)elem1.addr)->DeleteIfAllowed();
        result.setAddr(t);
        return YIELD;
      }
      else return CANCEL;
    }
    case CLOSE :
    {
      qp->Close(args[0].addr);
      if(local.addr)
      {
        ((TupleType *)local.addr)->DeleteIfAllowed();
        local.setAddr(0);
      }
      return 0;
    }
  }
  return 0;
}

# else

// progress version

CostEstimation* OperatorProject::ProjectCostEstimationFunc()
{
  return new ProjectCostEstimation();
}

struct projectLI2{
   TupleType* tt;
   Word elem1;
   Word elem2;
};

int
OperatorProject::Project(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  ProjectLocalInfo *pli=0;
  int noOfAttrs= 0;
  int index= 0;
  Supplier son;

  projectLI2* li2 = (projectLI2*) qp->GetLocal2(s).addr;

  switch (message)
  {
    case INIT : {
      li2 = new projectLI2();
      li2->tt = new TupleType(nl->Second(GetTupleResultType(s)));
      qp->GetLocal2(s).addr = li2;
      return 0;
    }
    case FINISH : {
      if(li2){
        li2->tt->DeleteIfAllowed();
        delete li2;
        qp->GetLocal2(s).addr=0;
      }
      return 0;
    }


    case OPEN:{

      pli = (ProjectLocalInfo*) local.addr;
      if ( pli ) delete pli;

      pli = new ProjectLocalInfo();
      li2->tt->IncReference();
      pli->tupleType = li2->tt;
      local.setAddr(pli);

      qp->Open(args[0].addr);
      return 0;
    }
    case REQUEST:{

      pli = (ProjectLocalInfo*) local.addr;

      qp->Request(args[0].addr, li2->elem1);
      if (qp->Received(args[0].addr))
      {
        pli->read++;
        Tuple *t = new Tuple( pli->tupleType );

        noOfAttrs = ((CcInt*)args[2].addr)->GetIntval();
        assert( t->GetNoAttributes() == noOfAttrs );

        for( int i = 0; i < noOfAttrs; i++)
        {
          son = qp->GetSupplier(args[3].addr, i);
          qp->Request(son, li2->elem2);
          index = ((CcInt*)li2->elem2.addr)->GetIntval();
          t->CopyAttribute(index-1, (Tuple*)li2->elem1.addr, i);
        }
        ((Tuple*)li2->elem1.addr)->DeleteIfAllowed();
        result.setAddr(t);
        return YIELD;
      }
      else return CANCEL;
    }
    case CLOSE: {

      pli = (ProjectLocalInfo*) local.addr;
      if ( pli ){
         delete pli;
         local.setAddr(0);
      }

      qp->Close(args[0].addr);
      return 0;

    }

    default :
              return 0;

  }
  return 0;
}

#endif

