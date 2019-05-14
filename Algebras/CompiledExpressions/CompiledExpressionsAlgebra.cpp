/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[TOC] [\tableofcontents]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[<] [$<$]
//[>] [$>$]

[10] Implementation file of the CompiledExpression Algebra

2016/2017 H.Brieschke created the new CompiledExpression Algebra


[TOC]


1 Overview 

The ~Compiled Expression Algebra~ basically implements three operators,
Namely ~filter~, ~extend~, and ~executeQuery~. However, these
can not be used directly in a query, but are exchanged by the algebra
against the original operators during the query processing. 

The ~filter~-operator corresponds to the ~filter~-operator from
the ~relation algebra~, the ~extend~-operator is the ~extend~-operator
from the ~ExtRelation algebra~, and the ~excecuteQuery-~operator is used
if the complete query could be compiled by this algebra.

2 Defines, includes, and constants

*/


#include <set>

#include "Operator.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "Symbols.h"
#include "Trace.h"
#include "StandardTypes.h"
#include "QueryProcessor.h"
#include "Progress.h"

#include "Algebras/Relation-C++/RelationAlgebra.h"

#include "./CompiledExpressionsAlgebra.h"
#include "./CECompiler.h"
#include "./CECodeGenerator.h"


extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

using namespace CompiledExpressions;
namespace CompiledExpressions {

/*
3 Class ~CEARuntimeError~

This class implements ~ERROR~-objects, which are required for the error treatment.

*/
  CEARuntimeError::CEARuntimeError(std::string s) {
    errMsg = s;
  }
  
  const char*
  CEARuntimeError::what() const throw() {
    return errMsg.c_str();
  }

/*
4 The Operators of the ~Compiled Expressions Algebra~.

4.1  Operator ~filter~

This operator corrosponded to the ~filter-~operator in the ~Realation-Algebra~.
Only tuples, fulfilling a certain condition are passed on to the
output stream.

4.1.1 Type mapping function of operator ~filter~

Result type of filter operation.

----    ((stream (tuple x)) (map (tuple x) bool))
               -> (stream (tuple x))
----

Type mapping function modified to show the possibility of getting
not only types but also arguments in the type mapping. This happens
when an operator
registers "UsesArgsinTypeMapping". Type list now has the form

----  ( (type1 arg1) (type2 arg2) )
----

that is

----  (
      ( (stream (tuple x))  arg1 )
      ( (map (tuple x) bool)  arg2 )
    )
----

*/
  ListExpr
  ceaFilterTypeMap( ListExpr args ) {
    if(!nl->HasLength(args,2)) {
      return listutils::typeError("two arguments expected");
    }
    
    if(!nl->HasLength(nl->First(args),2)) {
      ErrorReporter::ReportError("the first argument "
      " should be a (type, expression) pair");
      return nl->TypeError();
    }
    
    if(!listutils::isTupleStream(nl->First(nl->First(args)))){
      ErrorReporter::ReportError("first argument must be a stream of tuples");
      return nl->TypeError();
    }
    
    if(!nl->HasLength(nl->Second(args),2)){
      ErrorReporter::ReportError("the second argument "
      " should be a (type, expression) pair");
      return nl->TypeError();
    }
    
    
    ListExpr map = nl->First(nl->Second(args));
    if(!listutils::isMap<1>(map)){ // checking for a single arg
      ErrorReporter::ReportError("map: tuple -> bool expected as the"
      " second argument");
      return nl->TypeError();
    }
    
    ListExpr mapres = nl->Third(map);
    if(!CcBool::checkType(mapres)){
      ErrorReporter::ReportError("map is not a predicate");
      return nl->TypeError();
    } 
    
    if(!nl->Equal(nl->Second(nl->First(nl->First(args))), nl->Second(map))){
      ErrorReporter::ReportError("map and tuple type are not consistent");
      return nl->TypeError();
    }
    
    return nl->First(nl->First(args));
  }
  
/*
4.1.2 Value mapping function of operator ~filter~

*/

  #ifndef USE_PROGRESS
  // standard version

  int
  ceaFilterValueMap( Word* args,
                     Word& result,
                     int message,
                     Word& local,
                     Supplier s ) {
    
    switch ( message ) {
      
      case OPEN:
        qp->Open (args[0].addr);
        return 0;
        
      case REQUEST:
      {
        bool found = false;
        Word elem;
        Tuple* tuple = 0;

        filterOp_t filterOp = (filterOp_t) args[1].addr;

        qp->Request(args[0].addr, elem);
        while (qp->Received(args[0].addr) && !found) {
          tuple = (Tuple*)elem.addr;
          found = filterOp(tuple);
          if (!found) {
            tuple->DeleteIfAllowed();
            qp->Request(args[0].addr, elem);
          }
        }
        if (found) {
          result.setAddr(tuple);
          return YIELD;
        }
        else
          return CANCEL;
      }
      
      case CLOSE:
        qp->Close(args[0].addr);
        return 0;
    }
    return 0;
  }

  #else
  //progress version
  int
  ceaFilterValueMap(Word* args,
                    Word& result,
                    int message,
                    Word& local,
                    Supplier s) {
    
    CEAFilterLocalInfo* ptrCEAFLI;
    switch ( message ) {
      case OPEN: {
        ptrCEAFLI = (CEAFilterLocalInfo*) local.addr;
        if(ptrCEAFLI)
          delete ptrCEAFLI;
        
        ptrCEAFLI = new CEAFilterLocalInfo();
        local.setAddr(ptrCEAFLI);
        
        qp->Open(args[0].addr);

        return 0;
      }
      
      case REQUEST: {
        ptrCEAFLI = (CEAFilterLocalInfo*) local.addr;
        bool found = false;
        Word elem;
        Tuple* tuple = 0;
        
        filterOp_t filterOp = (filterOp_t) args[1].addr;

        qp->Request(args[0].addr, elem);
        while (qp->Received(args[0].addr) && !found) {
          tuple = (Tuple*)elem.addr;
          found = filterOp(tuple);
          if (!found) {
            tuple->DeleteIfAllowed();
            qp->Request(args[0].addr, elem);
          }
        }
        if (found) {
          result.setAddr(tuple);
          ptrCEAFLI->addReturned();
          return YIELD;
        } else {
          ptrCEAFLI->setDone();
          return CANCEL;
        }
      }
      
      case CLOSE: {
        qp->Close(args[0].addr);
        return 0;
      }
      
      case CLOSEPROGRESS: {
        ptrCEAFLI = (CEAFilterLocalInfo*) local.addr;
        if (ptrCEAFLI) {
          delete ptrCEAFLI;
          local.setAddr(0);
        }
        return 0;
      }
      
      case REQUESTPROGRESS: {
        ProgressInfo p1;
        ProgressInfo* pRes;
        const double uFilter = 0.01;
        pRes = (ProgressInfo*) result.addr;
        ptrCEAFLI = (CEAFilterLocalInfo*) local.addr;
        if ( qp->RequestProgress(args[0].addr, &p1) ) {
          pRes->CopySizes(p1);
          if ( ptrCEAFLI ) {
            //filter was started
            if ( ptrCEAFLI->getDone() ) {
              //arg stream exhausted, all known
              pRes->Card = (double) ptrCEAFLI->getReturned();
              pRes->Time = p1.Time + (double) ptrCEAFLI->getCurrent()
                            * qp->GetPredCost(s) * uFilter;
              pRes->Progress = 1.0;
              pRes->CopyBlocking(p1);
              return YIELD;
            }
            
            if ( ptrCEAFLI->getReturned() >= enoughSuccessesSelection ) {
              //stable state assumed now
              pRes->Card = p1.Card *
                ( (double) ptrCEAFLI->getReturned()
                  / (double) (ptrCEAFLI->getCurrent()));
              pRes->Time = p1.Time + p1.Card * qp->GetPredCost(s) * uFilter;
              if ( p1.BTime < 0.1 && pipelinedProgress )
                //non-blocking,
                //use pipelining
                pRes->Progress = p1.Progress;
              else
                pRes->Progress = (p1.Progress * p1.Time
                                 + ptrCEAFLI->getCurrent()
                                   * qp->GetPredCost(s)
                                   * uFilter)
                                     / pRes->Time;
              
              pRes->CopyBlocking(p1);
              
              return YIELD;
            }
          }
          //filter not yet started or not enough seen
          pRes->Card = p1.Card * qp->GetSelectivity(s);
          pRes->Time = p1.Time + p1.Card * qp->GetPredCost(s) * uFilter;
          if ( p1.BTime < 0.1 && pipelinedProgress )
            //non-blocking,
            //use pipelining
            pRes->Progress = p1.Progress;
          else
            pRes->Progress = (p1.Progress * p1.Time) / pRes->Time;
          
          pRes->CopyBlocking(p1);
          return YIELD;
        } else 
          return CANCEL;
      }
    }
    return 0;
  }

/*

4.1.3 Class ~CEAFilterLocalInfo~

In the ~Progress~-Version used this class objects as data structure for the
progress informations.

The Constructor.

*/  
  CEAFilterLocalInfo::CEAFilterLocalInfo()
  : current(0),
    returned(0),
    done(false) {}
  
/*
The Destructor.

*/
  CEAFilterLocalInfo::~CEAFilterLocalInfo() {}
  
/*
The following methods are the ~get~er- and ~set~er-methods for these
private variables.

*/
  void
  CEAFilterLocalInfo::addCurrent() {
    current++;
  }

  void
  CEAFilterLocalInfo::addReturned() {
    returned++;
  }
  
  void
  CEAFilterLocalInfo::setDone() {
    done = true;
  }
  
  int
  CEAFilterLocalInfo::getCurrent() {
    return current;
  }
  
  int
  CEAFilterLocalInfo::getReturned() {
    return returned;
  }
  
  bool
  CEAFilterLocalInfo::getDone() {
    return done;
  }
  
#endif
  
/*
4.1.4 Specification of operator ~filter~

*/
  const std::string ceaFilterSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>((stream x) (map x bool)) -> "
    "(stream x)</text--->"
    "<text>_ filter [ fun ]</text--->"
    "<text>Only tuples, fulfilling a certain "
    "condition are passed on to the output "
    "stream.</text--->"
    "<text>query cities feed filter "
    "[.population > 500000] consume</text--->"
    ") )";
      
/*
4.1.5 Definition of operator ~filter~

*/
  Operator ceaOperatorFilter (
         "filter",                 // name
         ceaFilterSpec,            // specification
         ceaFilterValueMap,        // value mapping
         Operator::SimpleSelect,   // trivial selection function
         ceaFilterTypeMap          // type mapping
         );

/*
4.2 Operator ~extend~

This operator corrosponded to the ~extend-~operator in the ~ExtRealation-Algebra~.
Extends each input tuple by new attributes as specified in the parameter list.

4.2.1 Type mapping function of operator ~extend~

Type mapping for ~extend~ is

----     ((stream X) ((b1 (map x y1)) ... (bm (map x ym))))

        -> (stream (tuple ((a1 x1) ... (an xn) (b1 y1) ... (bm ym)))))

        where X = (tuple ((a1 x1) ... (an xn)))
----

*/
  ListExpr
  ceaExtendTypeMap( ListExpr args ) {
    ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

    if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("two elements expected");
      return nl->TypeError();
    }
    
    ListExpr stream = nl->First(args);
    
    if(!IsStreamDescription(stream)){
      ErrorReporter::ReportError("first argument is not a tuple stream");
      return nl->TypeError();
    }
    
    ListExpr tuple = nl->Second(stream);
    
    ListExpr functions = nl->Second(args);
    if(nl->ListLength(functions)<1){
      ErrorReporter::ReportError("at least one function expected");
      return nl->TypeError();
    }
    
    // copy attrlist to newattrlist
    ListExpr attrList = nl->Second(nl->Second(stream));
    ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
    ListExpr lastlistn = newAttrList;
    attrList = nl->Rest(attrList);
    while (!(nl->IsEmpty(attrList)))
    {
      lastlistn = nl->Append(lastlistn,nl->First(attrList));
      attrList = nl->Rest(attrList);
    }
    
    // reset attrList
    attrList = nl->Second(nl->Second(stream));
    ListExpr typeList;
    
    
    // check functions
    std::set<std::string> usedNames;
    
    while (!(nl->IsEmpty(functions)))
    {
      ListExpr function = nl->First(functions);
      functions = nl->Rest(functions);
      if(nl->ListLength(function)!=2){
        ErrorReporter::ReportError("invalid extension found");
        return nl->TypeError();
      }
      ListExpr name = nl->First(function);
      ListExpr map  = nl->Second(function);
      
      std::string errormsg;
      if(!listutils::isValidAttributeName(name, errormsg)){
        return listutils::typeError(errormsg);
      }
      
      std::string namestr = nl->SymbolValue(name);
      int pos = FindAttribute(attrList,namestr,typeList);
      if(pos!=0){
        ErrorReporter::ReportError("Attribute "+ namestr +
        " already member of the tuple");
        return nl->TypeError();
      }
      
      
      if(nl->ListLength(map)!=3){
        ErrorReporter::ReportError("invalid function");
        return nl->TypeError();
      }
      
      if(!nl->IsEqual(nl->First(map),Symbol::MAP())){
        ErrorReporter::ReportError("invalid function");
        return nl->TypeError();
      }
      
      ListExpr funResType = nl->Third(map);
      if(!am->CheckKind(Kind::DATA(),funResType, errorInfo)){
        ErrorReporter::ReportError("requested attribute " + namestr +
        "not in kind DATA");
        return nl->TypeError();
      }
      
      ListExpr funArgType = nl->Second(map);
      
      if(!nl->Equal(funArgType, tuple)){
        ErrorReporter::ReportError
          ("function type different to the tuple type");
        return nl->TypeError();
      }
      
      if(usedNames.find(namestr)!=usedNames.end()){
        ErrorReporter::ReportError("Name "+ namestr + "occurs twice");
        return nl->TypeError();
      }
      usedNames.insert(namestr);
      // append attribute
      lastlistn = nl->Append(lastlistn, (nl->TwoElemList(name, funResType )));
    }
    
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                           nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                           newAttrList));
  }


/*
4.2.2 Value mapping function of operator ~extend~

*/

  #ifndef USE_PROGRESS
  // standard version
  
  int
  ceaExtendValueMap(Word* args,
                    Word& result,
                    int message,
                    Word& local,
                    Supplier s ) {
    
    TupleType* resultTupleType;
    
    switch (message)
    {
      case OPEN:
      {
        ListExpr resultType;
        
        qp->Open(args[0].addr);
        resultType = GetTupleResultType( s );
        resultTupleType = new TupleType( nl->Second( resultType ) );
        local.setAddr( resultTupleType );
        return 0;
      }
      
      case REQUEST:
      {
        Supplier supplier1, supplier2, supplier3;
        Tuple* tuple;
        Word elem, value, tmpArgs;
        ArgVectorPointer funargs;
        int nooffun;
        
        resultTupleType = (TupleType*)local.addr;
        qp->Request(args[0].addr, elem);
        if (qp->Received(args[0].addr)) {
          tuple = (Tuple*)elem.addr;
          Tuple* newTuple = new Tuple( resultTupleType );
          for( int i = 0; i < tuple->GetNoAttributes(); i++ ) {
            //cout << (void*) tuple << endl;
            newTuple->CopyAttribute( i, tuple, i );
          }
          supplier1 = args[1].addr;
          nooffun = qp->GetNoSons(supplier1);
          for (int i=0; i < nooffun; i++) {
            supplier2 = qp->GetSupplier(supplier1, i);
            supplier3 = qp->GetSupplier(supplier2, 1);

            if ((qp->getPtrCEQueryProcessor())->IsPointerNode( supplier3 )) {
              //cout << "son [" << i << "] is a pointer." << endl;
              
              qp->Request(supplier3, tmpArgs);
              extendOp_t extendOp = (extendOp_t) tmpArgs.addr;
              newTuple->PutAttribute( tuple->GetNoAttributes()+i,
                                      (extendOp(tuple))->Clone());

            } else {
              //cout << "son [" << i << "] is a map function." << endl;
            
              funargs = qp->Argument(supplier3);
              ((*funargs)[0]).setAddr(tuple);
              qp->Request(supplier3,value);
              newTuple->PutAttribute( tuple->GetNoAttributes()+i,
                                      ((Attribute*)value.addr)->Clone() );
            }

          }
          tuple->DeleteIfAllowed();
          result.setAddr(newTuple);
          return YIELD;
        } else
          return CANCEL;
      }
      
      case CLOSE:
        if(local.addr) {
          ((TupleType*)local.addr)->DeleteIfAllowed();
          local.setAddr(0);
        }
        qp->Close(args[0].addr);
        return 0;
    }
    return 0;
  }
  
  # else
  // progress version

  int
  ceaExtendValueMap(Word* args,
                    Word& result,
                    int message,
                    Word& local,
                    Supplier s) {
    
    Word t, value, tmpArgs;
    Tuple* tuple;
    Supplier supplier1, supplier2, supplier3;
    int nooffun;
    ArgVectorPointer funargs;
    CEAExtendLocalInfo* ptrCEAELI;
    
    switch (message)
    {
      case OPEN : {
        ptrCEAELI = (CEAExtendLocalInfo*) local.addr;
        
        if ( ptrCEAELI ) {
          delete ptrCEAELI;
        }
        
        ptrCEAELI = new CEAExtendLocalInfo
                          (new TupleType(nl->Second(GetTupleResultType(s))),
                           50,
                           false,
                           qp->GetNoSons(args[1].addr));
                          
        local.setAddr(ptrCEAELI);
        
        qp->Open(args[0].addr);
        
        return 0;
      }
      case REQUEST : {
        ptrCEAELI = (CEAExtendLocalInfo*) local.addr;
        
        if(!ptrCEAELI){
          return CANCEL;
        }
        qp->Request(args[0].addr,t);
        if (qp->Received(args[0].addr))
        {
          tuple = (Tuple*)t.addr;
          Tuple* newTuple = new Tuple( ptrCEAELI->getResultTupleType() );
          ptrCEAELI->read++;
          for( int i = 0; i < tuple->GetNoAttributes(); i++ ) {
            //cout << (void*) tuple << endl;
            newTuple->CopyAttribute( i, tuple, i );
          }
          supplier1 = args[1].addr;
          nooffun = qp->GetNoSons(supplier1);
          for (int i=0; i < nooffun;i++)
          {
            supplier2 = qp->GetSupplier(supplier1, i);
            supplier3 = qp->GetSupplier(supplier2, 1);

            if ((qp->getPtrCEQueryProcessor())->IsPointerNode( supplier3 )) {
              //cout << "son [" << i << "] is a pointer." << endl;
              
              qp->Request(supplier3, tmpArgs);
              extendOp_t extendOp = (extendOp_t) tmpArgs.addr;
              newTuple->PutAttribute( tuple->GetNoAttributes()+i,
                                      (extendOp(tuple))->Clone());

            } else {
              //cout << "son [" << i << "] is a map function." << endl;
            
              funargs = qp->Argument(supplier3);
              ((*funargs)[0]).setAddr(tuple);
              qp->Request(supplier3,value);
              newTuple->PutAttribute( tuple->GetNoAttributes()+i,
                                      ((Attribute*)value.addr)->Clone() );
            }
            
            if (ptrCEAELI->read <= ptrCEAELI->getStableValue())
            {
              ptrCEAELI->addAttrSizeTmp
                           (newTuple->GetSize(tuple->GetNoAttributes()+i),
                            i);
              ptrCEAELI->addAttrSizeExtTmp
                           (newTuple->GetExtSize(tuple->GetNoAttributes()+i),
                            i);
            }
            
          }
          tuple->DeleteIfAllowed();
          result.setAddr(newTuple);
          return YIELD;
        }
        else
          return CANCEL;
      }
      case CLOSE : {
        qp->Close(args[0].addr);
        return 0;
      }
      case CLOSEPROGRESS : {
        ptrCEAELI = (CEAExtendLocalInfo*) local.addr;
        if ( ptrCEAELI ){
          delete ptrCEAELI;
          local.setAddr(0);
        }
        return 0;
        
      }
      case REQUESTPROGRESS : {
        ptrCEAELI = (CEAExtendLocalInfo*) local.addr;
        ProgressInfo p1;
        ProgressInfo *pRes;
        const double uExtend = 0.0034;   //millisecs per tuple
        const double vExtend = 0.0067;   //millisecs per tuple and attribute
        
        // see determination of constants in file ConstantsExtendStream
        
        pRes = (ProgressInfo*) result.addr;
        
        if ( !ptrCEAELI ) {
          return CANCEL;
        }
        
        if ( qp->RequestProgress(args[0].addr, &p1) )
        {
          ptrCEAELI->sizesChanged = false;
          
          if ( !ptrCEAELI->sizesInitialized )
          {
            ptrCEAELI->setNoOldAttrs(p1.noAttrs);
            ptrCEAELI->noAttrs = ptrCEAELI->getNoOldAttrs()
                                 + ptrCEAELI->getNoNewAttrs();
            ptrCEAELI->attrSize = new double[ptrCEAELI->noAttrs];
            ptrCEAELI->attrSizeExt = new double[ptrCEAELI->noAttrs];
          }
          
          if ( !ptrCEAELI->sizesInitialized
            || p1.sizesChanged
            || ( ptrCEAELI->read >= ptrCEAELI->getStableValue()
                 && !ptrCEAELI->getSizesFinal() ) ) {
            
            ptrCEAELI->Size = 0.0;
            ptrCEAELI->SizeExt = 0.0;
            
            //old attrs
            for (int i = 0; i < ptrCEAELI->getNoOldAttrs(); i++)
            {
              ptrCEAELI->attrSize[i] = p1.attrSize[i];
              ptrCEAELI->attrSizeExt[i] = p1.attrSizeExt[i];
              ptrCEAELI->Size += ptrCEAELI->attrSize[i];
              ptrCEAELI->SizeExt += ptrCEAELI->attrSizeExt[i];
            }
            
            //new attrs
            if ( ptrCEAELI->read < ptrCEAELI->getStableValue() )
            {
              for (int j = 0; j < ptrCEAELI->getNoNewAttrs(); j++)
              {
                ptrCEAELI->attrSize[j + ptrCEAELI->getNoOldAttrs()] = 12;
                ptrCEAELI->attrSizeExt[j + ptrCEAELI->getNoOldAttrs()] = 12;
                ptrCEAELI->Size += ptrCEAELI
                                   ->attrSize[j + ptrCEAELI->getNoOldAttrs()];
                ptrCEAELI->SizeExt += ptrCEAELI
                                      ->attrSizeExt[j + ptrCEAELI
                                                          ->getNoOldAttrs()];
              }
            }
            else
            {
              for (int j = 0; j < ptrCEAELI->getNoNewAttrs(); j++)
              {
                ptrCEAELI->attrSize[j + ptrCEAELI->getNoOldAttrs()]
                  = ptrCEAELI->getAttrSizeTmp(j)
                      / ptrCEAELI->getStableValue();
                ptrCEAELI->attrSizeExt[j + ptrCEAELI->getNoOldAttrs()]
                  = ptrCEAELI->getAttrSizeExtTmp(j)
                      / ptrCEAELI->getStableValue();
                ptrCEAELI->Size += ptrCEAELI
                                     ->attrSize[j + ptrCEAELI
                                                      ->getNoOldAttrs()];
                ptrCEAELI->SizeExt += ptrCEAELI
                                       ->attrSizeExt[j + ptrCEAELI
                                                          ->getNoOldAttrs()];
              }
            }
            ptrCEAELI->sizesInitialized = true;
            ptrCEAELI->sizesChanged = true;
          }
          //ensure sizes are updated only once for passing the threshold
          if ( ptrCEAELI->read >= ptrCEAELI->getStableValue() )
            ptrCEAELI->setSizesFinal(true);
          
          
          pRes->Card = p1.Card;
          
          pRes->CopySizes(ptrCEAELI);
          
          pRes->Time = p1.Time
                       + p1.Card
                         * (uExtend + ptrCEAELI->getNoNewAttrs() * vExtend);
          
          
          if ( p1.BTime < 0.1 && pipelinedProgress )      //non-blocking,
            //use pipelining
            pRes->Progress = p1.Progress;
          else
            pRes->Progress =
            (p1.Progress * p1.Time +
            ptrCEAELI->read
              * (uExtend + ptrCEAELI->getNoNewAttrs() * vExtend))
                / pRes->Time;
          
          pRes->CopyBlocking(p1);    //non-blocking operator
          
          return YIELD;
        } else {
          return CANCEL;
        }
      }
    }
    return 0;
  }


/*
4.2.3 Class ~CEAExtendLocalInfo~

In the ~Progress~-Version used this class objects as data structure for the
progress informations.

The Constructor.

*/    
  CEAExtendLocalInfo::CEAExtendLocalInfo(
                       TupleType* ptrTupleType,
                       int sv,
                       bool sf,
                       int nna)
  : resultTupleType(ptrTupleType),
    stableValue(sv),
    sizesFinal(sf),
    noOldAttrs(0),
    noNewAttrs(nna) {
      attrSizeTmp = new double[nna];
      attrSizeExtTmp = new double[nna];
      for (int i = 0; i < nna; i++) {
        attrSizeTmp[i] = 0.0;
        attrSizeExtTmp[i] = 0.0;
      }
   };

/*
The Destructor.

*/
  CEAExtendLocalInfo::~CEAExtendLocalInfo() {
    if(resultTupleType){
      resultTupleType->DeleteIfAllowed();
    }
    if(attrSizeTmp){
       delete [] attrSizeTmp;
    }
    if(attrSizeExtTmp){
       delete [] attrSizeExtTmp;
    }
  }


/*
The following methods are the ~get~er- and ~set~er-methods for these
private variables.

*/
  void
  CEAExtendLocalInfo::setResultTupleType(TupleType* ptrTupleType) {
    resultTupleType = ptrTupleType;
  }
  
  TupleType*
  CEAExtendLocalInfo::getResultTupleType() {
    return resultTupleType;
  }
  
  void
  CEAExtendLocalInfo::setStableValue(int val) {
    stableValue = val;
  }
  
  int 
  CEAExtendLocalInfo::getStableValue() {
    return stableValue;
  }
  
  void
  CEAExtendLocalInfo::setSizesFinal(bool sf) {
    sizesFinal = sf;
  }
  
  bool
  CEAExtendLocalInfo::getSizesFinal() {
    return sizesFinal;
  }
  
  void
  CEAExtendLocalInfo::setNoOldAttrs(int noa) {
    noOldAttrs = noa;
  }
  
  int
  CEAExtendLocalInfo::getNoOldAttrs() {
    return noOldAttrs;
  }
  
  void
  CEAExtendLocalInfo::setNoNewAttrs(int noa) {
    noNewAttrs = noa;
  }
  
  int
  CEAExtendLocalInfo::getNoNewAttrs() {
    return noNewAttrs;
  }
  
  void
  CEAExtendLocalInfo::setAttrSizeTmp(double asp, int idx) {
    attrSizeTmp[idx] = asp;
  }
  
  void
  CEAExtendLocalInfo::addAttrSizeTmp(double asp, int idx) {
    attrSizeTmp[idx] += asp;
  }
  
  double
  CEAExtendLocalInfo::getAttrSizeTmp(int idx) {
    return attrSizeTmp[idx];
  }
  
  void
  CEAExtendLocalInfo::setAttrSizeExtTmp(double asep, int idx) {
    attrSizeExtTmp[idx] = asep;
  }
  
  void
  CEAExtendLocalInfo::addAttrSizeExtTmp(double asep, int idx) {
    attrSizeExtTmp[idx] += asep;
  }
  
  double
  CEAExtendLocalInfo::getAttrSizeExtTmp(int idx) {
    return attrSizeExtTmp[idx];
  }
  #endif




/*
4.2.4 Specification of operator ~extend~

*/
  const std::string ceaExtendSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(stream(tuple(x)) x [(a1, (tuple(x)"
    " -> d1)) ... (an, (tuple(x) -> dn))] -> "
    "stream(tuple(x@[a1:d1, ... , an:dn])))"
    "</text--->"
    "<text>_ extend [funlist]</text--->"
    "<text>Extends each input tuple by new "
    "attributes as specified in the parameter"
    " list.</text--->"
    "<text>query ten feed extend [mult5 : "
    ".nr * 5, mod2 : .nr mod 2] consume"
    "</text--->"
    ") )";
      
  

/*
4.2.5 Definition of operator ~extend~

*/
  Operator ceaOperatorExtend (
         "extend",                // name
         ceaExtendSpec,           // specification
         ceaExtendValueMap,       // value mapping
         Operator::SimpleSelect,  // trivial selection function
         ceaExtendTypeMap         // type mapping
  );

 

/*
4.3 Operator ~executeQuery~

This operator is used, if the complete query could be compiled by this algebra.

4.3.1 Type mapping function of operator ~executeQuery~

Since this operator can not be used directly in queries, there is no valid
type mappings. A type mapping error is always generated.

*/
  ListExpr ceaExecuteQueryTypeMap( ListExpr args ) {
    ErrorReporter::ReportError("this operator is not to be used in queries");
    return nl->TypeError();
  }

/*
4.3.2 Value mapping function of operator ~executeQuery~

*/
  int
  ceaExecuteQueryValueMap(Word* args,
                          Word& result,
                          int message,
                          Word& local,
                          Supplier s ) {
    
    executeQueryOp_t executeQueryOp = (executeQueryOp_t)  args[0].addr;
    executeQueryOp(result, s);
    return 0;
  }

/*
4.3.3 Specification of operator ~executeQuery~

*/
const std::string ceaExecuteQuerySpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>()</text--->"
  "<text>executeQuery</text--->"
  "<text>This Operator called only from Secondo-"
  "System. Is not to use in querys.</text--->"
  "<text>No Exapmle exist.</text--->) )";

  
/*
4.3.4 Definition of operator ~executeQuery~

*/
Operator ceaOperatorExecuteQuery (
         "executeQuery",            // name
         ceaExecuteQuerySpec,       // specification
         ceaExecuteQueryValueMap,   // value mapping
         Operator::SimpleSelect,    // trivial selection function
         ceaExecuteQueryTypeMap     // type mapping
);

/*

5 Class ~CompiledExpressionsAlgebra~

A new subclass ~CompiledExpressionsAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the actual algebra.

After declaring the new class, its only instance ~CompiledExpressionsAlgebra~ is defined.

Definition the static variables of the ~CompiledExpressionsAlgebra~-class.

*/  
  bool CompiledExpressionsAlgebra::ceaActiv = false;
  CompiledExpressionsAlgebra* CompiledExpressionsAlgebra::instance = 0;
  
/*
This function gives a pointer to the ~CompiledExpressionsAlgebra~-Instance.

*/
  CompiledExpressionsAlgebra*
  CompiledExpressionsAlgebra::getInstance() {
    if (!instance)
      instance = new CompiledExpressions::CompiledExpressionsAlgebra();
    return instance;
  }

/*
This function delete the ~CompiledExpressionsAlgebra~-Instance.

*/
  void 
  CompiledExpressionsAlgebra::deleteInstance() {
    if (instance)
      delete instance;
    instance = 0;
  }
  
/*
This function returned, if one instance of the ~CompiledExpressionsAlgebra~ are created.

*/
  bool
  CompiledExpressionsAlgebra::isActiv() {
    return CompiledExpressionsAlgebra::ceaActiv;
  }

/*
The Constructor.

*/
  CompiledExpressionsAlgebra::CompiledExpressionsAlgebra()
  : Algebra(),
    thisAlgID(-1) {

    AddOperator(&ceaOperatorFilter);
    ceaOperatorFilter.SetUsesArgsInTypeMapping();
    
    AddOperator(&ceaOperatorExtend);
    AddOperator(&ceaOperatorExecuteQuery);
    

/*
Register operators which are able to handle progress messages

*/
    #ifdef USE_PROGRESS
      ceaOperatorFilter.EnableProgress();
      ceaOperatorExtend.EnableProgress();
    #endif
    
    CompiledExpressionsAlgebra::ceaActiv = true;
  }
  
/*
The Destructor.

*/
  CompiledExpressionsAlgebra::~CompiledExpressionsAlgebra() {
    instance = 0;
    CECodeGenerator::deleteInstance();
    CECompiler::deleteInstance();
  }
  
/*
This function returned the algebra-ID from the algebramanager or 0, if the
algera is not loaded.

*/
  int
  CompiledExpressionsAlgebra::getThisAlgebraID() /*throw (CEARuntimeError)*/{
    if (thisAlgID == -1)
      thisAlgID = am->GetAlgebraId("CompiledExpressionsAlgebra");
    
    if (thisAlgID == 0)
      throw CEARuntimeError("The CompiledExpressionsAlgebra is not loaded.");
    
    return thisAlgID;    
  }
  
/*
This function returned the operator-ID of the spezified operator name. If the operator is
not found, the function throws an error.

*/
  int
  CompiledExpressionsAlgebra::getOperatorID(std::string name)
  /*throw (CEARuntimeError)*/ {
    int idx = 0;
    bool found = false;
    while ( !found && idx < GetNumOps() ) {
      if ( GetOperator(idx)->GetName() == name )
        found = true;
      else
        idx++;
    }
    
    if (!found)
      throw CEARuntimeError("Operator "
                            +  name
                            + " not insert in this algebra.");
    
    return idx;
  }
}

/*

7 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

#ifdef __cplusplus
extern "C"{
#endif
  Algebra*
  InitializeCompiledExpressionsAlgebra(NestedList* nlRef,
                                       QueryProcessor* qpRef)
  {
    nl = nlRef;
    qp = qpRef;
    return CompiledExpressions::CompiledExpressionsAlgebra::getInstance();
  }

#ifdef __cplusplus
}
#endif
