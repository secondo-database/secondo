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
#include "OperatorFeedProject.h"

#include "Algebra.h"
#include "Symbols.h"
#include "Progress.h"
#include "RelationAlgebra.h"
#include "Stream.h"

#ifdef USE_PROGRESS
#include "../CostEstimation/RelationAlgebraCostEstimation.h"
#endif

using namespace std;

#ifdef USE_PROGRESS

ListExpr OperatorFeedProject::feedproject_tm(ListExpr args)
{

  if(!nl->HasLength(args,2)){
    return listutils::typeError("rel(tuple(X)) x attrlist expected");
  }

  ListExpr rel = nl->First(args);
  ListExpr attrList = nl->Second(args);

  if(!Relation::checkType(rel)){
    return listutils::typeError("first argument is not a relation");
  }
  if((nl->AtomType(attrList)!=NoAtom)  || nl->IsEmpty(attrList)){
    return listutils::typeError("invalid list of attribute names");
  }


  NList l(args);

  const string opName = "feedproject";
  const string arg1 = "rel(tuple(...)";
  const string arg2 = "list of unique symbols (a_1 ... a_k)";

  string err1("");
  if ( !l.checkLength(2, err1) )
    return l.typeError( err1 );

  NList attrs;
  if ( !l.first().checkRel( attrs ) )
    return l.typeError(1, arg1);

  //cerr << "a1 " << attrs << endl;

  NList atoms = l.second();
  if ( !l.checkSymbolList( atoms ) )
    return l.typeError(2, arg2);

  if ( !l.checkUniqueMembers( atoms ) )
    return l.typeError(2, arg2);


  NList indices;
  NList oldAttrs(attrs);
  NList newAttrs;
  int noAtoms = atoms.length();
  while ( !atoms.isEmpty() )
  {
    string attrname = atoms.first().str();
    ListExpr attrtype = nl->Empty();
    //cerr << "a2 " << attrs << endl;
    int newIndex = FindAttribute( oldAttrs.listExpr(), attrname, attrtype );

    if (newIndex > 0)
    {
      indices.append( NList(newIndex) );
      newAttrs.append( oldAttrs.elem(newIndex) );
    }
    else
    {
      ErrorReporter::ReportError(
        "Attributename '" + attrname +
        "' is not a known attributename in the tuple stream.");
          return nl->SymbolAtom(Symbol::TYPEERROR());
    }
    atoms.rest();
  }

  NList outlist = NList( NList(Symbol::APPEND()),
                         NList( NList( noAtoms ), indices ),
       NList().tupleStreamOf( newAttrs ) );

  return outlist.listExpr();
}

CostEstimation*
OperatorFeedProject::FeedProjectCostEstimationFunc() {
  return new FeedProjectCostEstimation();
}

int
OperatorFeedProject::feedproject_vm(Word* args,
                                    Word& result,
                                    int message,
                                    Word& local,
                                    Supplier s)
{
  GenericRelation* r=0;
  FeedProjLocalInfo* fli=0;
  Word elem(Address(0));
  int noOfAttrs= 0;
  int index= 0;
  Supplier son;
  TupleType* tt = (TupleType*) qp->GetLocal2(s).addr;

  switch (message)
  {
    case INIT : {
       tt = new TupleType(nl->Second(GetTupleResultType(s)));
       qp->GetLocal2(s).addr = tt;
       return 0;
    }
    case FINISH : {
       if(tt){
         tt->DeleteIfAllowed();
         qp->GetLocal2(s).addr=0;
       }
       return 0;
    }

    case OPEN :{
      r = (GenericRelation*)args[0].addr;

      fli = (FeedProjLocalInfo*) local.addr;
      if ( fli ) delete fli;

      tt->IncReference();
      fli = new FeedProjLocalInfo(tt);
      fli->total = r->GetNoTuples();
      fli->rit = r->MakeScan(tt);
      local.setAddr(fli);
      return 0;
    }
    case REQUEST :{
      fli = (FeedProjLocalInfo*) local.addr;
      Tuple *t;

        noOfAttrs = ((CcInt*)args[2].addr)->GetIntval();
        list<int> usedAttrs;

        for( int i = 0; i < noOfAttrs; i++)
        {
          son = qp->GetSupplier(args[3].addr, i);
          qp->Request(son, elem);
          index = ((CcInt*)elem.addr)->GetIntval();
    //cerr << "ind = " << index << endl;
          usedAttrs.push_back(index-1);
        }

      if ((t = fli->rit->GetNextTuple(usedAttrs)) != 0)
      {
        fli->returned++;
        result.setAddr(t);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }
    case CLOSE :{
      fli = static_cast<FeedProjLocalInfo*>(local.addr);

      if(fli){

        if(fli->rit){
          delete fli->rit;
          fli->rit=0;
        }

        delete fli;
        fli = 0;
        local.setAddr(0);
      }
      return 0;
    } // end case
  } // end switch
  return 0;
}
#endif


