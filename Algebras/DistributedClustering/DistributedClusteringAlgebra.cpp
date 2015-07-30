/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Implementation of the Spatial Algebra

Jun 2015, Daniel Fuchs 

[TOC]

1 Overview


This file contains the implementation of the class dbDacScanAlgebra

2 Includes
 
*/ 

#include "NestedList.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "RelationAlgebra.h"
#include "Stream.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "Operator.h"
#include "Symbols.h"
#include "Algebra.h"
#include "DbDacScanGen.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace distributedClustering{
  
  /*
   * 
   3 *Operator dbDacScanM
   
   This operated realizes a inmemory db-scan for spatial objects
   with a divide and conquer algorithm
   
   In the divide step the space is divided with a kd-tree similar index
   structure.
   
   
   4 Type Mapping ~dbDacScan~
   
   */
  ListExpr
  dbDacScanTM(ListExpr args){
    
    //check Types in args
    NList type(args);
    string errMsg = "";
    if(nl->ListLength(args)!=2) {
      errMsg = "Two elements expexted. First must be a stream"
      " of tuple and second must be an argument list";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //check if first in-Parameter is a stream
    ListExpr stream = nl->First(args);
    if(!Stream<Tuple>::checkType(nl->First(args))) {
      return listutils::typeError("first argument is not stream(Tuple)");
    }
    
    //check if second in-Parameter is a list of args
    ListExpr arguments = nl->Second(args);
    
    if(nl->ListLength(arguments)!=4) {
      errMsg = "Second argument is not a conform list of input" 
      " Parameters such, "
      " Attributename x ClusterId x EPS x MinPts ";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //check EPS
    if(!CcReal::checkType(nl->Third(arguments))) {
      return listutils::typeError("EPS must be a real");
    }
    
    //check MinPts
    if(!CcInt::checkType(nl->Fourth(arguments))) {
      return listutils::typeError("MinPts must be an integer");
    }
    
    //check the cluster attribute name if it is existing in a tuple 
    ListExpr attrList = nl->Second(nl->Second(stream));
    ListExpr attrType;
    string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
    int found = FindAttribute(attrList, attrName, attrType);
    if(found == 0) {
      ErrorReporter::ReportError("Attribute "
      + attrName + " is no member of the tuple");
      return nl->TypeError();
    }
    
    //Check attrType
    if( !CcInt::checkType(attrType)
      && !CcReal::checkType(attrType)
      && !Point::checkType(attrType)
      && !CcString::checkType(attrType) 
      && !Picture::checkType(attrType) ) {
      errMsg = "Attribute "+attrName+"is not of type "
      + CcInt::BasicType() + ", " 
      + CcReal::BasicType() + ", " 
      + Point::BasicType() + ", " 
//       + CcString::BasicType() + " or " 
//       + Picture::BasicType()
      +"!";
      return listutils::typeError(errMsg);
      }
      
      //check clusterID
      ListExpr name = nl->Second(arguments); 
      ListExpr typeList;
      
      string errormsg;
      if(!listutils::isValidAttributeName(name, errormsg)){
        return listutils::typeError(errormsg);
      }//endif
      
      string namestr = nl->SymbolValue(name);
      int pos = FindAttribute(attrList,namestr,typeList);
      if(pos!=0) {
        ErrorReporter::ReportError("Attribute "+ namestr +
        " already member of the tuple");
        return nl->TypeError();
      }//endif
      
      
      
      //Copy attrList to newAttrList
      ListExpr oldAttrList = nl->Second(nl->Second(stream));
      ListExpr newAttrList = nl->OneElemList(nl->First(oldAttrList));
      ListExpr newAttrListEnd = newAttrList;
      oldAttrList = nl->Rest(oldAttrList);
      
      while(!(nl->IsEmpty(oldAttrList))){
        newAttrListEnd = nl->Append(newAttrListEnd,nl->First(oldAttrList));
        oldAttrList = nl->Rest(oldAttrList);
      }
      //TODO append some other attributes
      
      //append int for ClusterID
      newAttrListEnd = nl->Append(newAttrListEnd,
                                  (nl->TwoElemList(
                                    name,
                                    nl->SymbolAtom(CcInt::BasicType()))));
      
      //append bool for Visited
      
      newAttrListEnd = nl->Append(newAttrListEnd,
                                  (nl->TwoElemList(
                                    nl->SymbolAtom("IsCluster"),
                                    nl->SymbolAtom(CcBool::BasicType())
                                  )));
      
      
      return nl->ThreeElemList(
        nl->SymbolAtom(Symbol::APPEND())
        ,nl->OneElemList(nl->IntAtom(found-1))
        ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->TwoElemList(
                           nl->SymbolAtom(Tuple::BasicType()),
                                         newAttrList)));
      
  }
  
  /*
   *   5 Value mapping mehtod ~dbDacScan~
   */
  template <class TYPE, class MEMB_TYP_CLASS>
  int dbDacScanVM1(Word* args, Word& result, int message, 
                   Word& local, Supplier s) {
    
    //TODO hier dbDacScanGen definieren
    typedef DbDacScanGen<MEMB_TYP_CLASS,TYPE> dbdacscanClass;
    
    dbdacscanClass* li = (dbdacscanClass*) local.addr;
    
    switch (message)
    {
      case OPEN:
      {
        //get stream
        Word inStream = args[0];
        // get arguments
        Word argument;
        //get EPS
        Supplier sup = qp->GetSupplier(args[1].addr,2);
        qp->Request(sup, argument);
        CcReal* eps = static_cast<CcReal*>(argument.addr);
        //get MinPts
        sup = qp->GetSupplier(args[1].addr, 3);
        qp->Request(sup, argument);
        CcInt* minPts = static_cast<CcInt*>(argument.addr);
        //set index of attribute position in tuple
        int attrPos = static_cast<CcInt*>(args[2].addr)->GetIntval();
        //set the result type of the tuple
        ListExpr resultType = GetTupleResultType(s);
        ListExpr tt = ( nl->Second( resultType ) );
        if(li) { 
          delete li;
          local.addr=0;
        }
        size_t maxMem = qp->GetMemorySize(s)*1024*1024;
        if(!eps->IsDefined() || eps->GetValue() < 0){
          return 0;
        }
        if(!minPts->IsDefined() || minPts->GetValue() < 0){
          return 0;
        }
        local.addr = new dbdacscanClass(
          inStream,
          tt, 
          eps->GetValue(),
                                        minPts->GetValue(), 
                                        attrPos,
                                        maxMem);
        return 0;
      }
      case REQUEST:
      {
        result.addr= li?li->next():0; 
        return result.addr?YIELD:CANCEL;
      }
      case CLOSE:
      {
        if(li){
          delete li;
          local.addr=0;
        }
      }
    }
    return 0;
  }
  
  /*
   6 *Struct ~dbDacScanInfo~
   
   */
  struct dbDacScanInfo :  OperatorInfo
  {
    dbDacScanInfo() : OperatorInfo()
    {
      name      = "dbdacscan";
      signature = "stream(Tuple) -> stream(Tuple)";
      syntax    = "_ dbdacscan[_,_,_,_]"; 
      meaning   = "";//TODO ab hier noch nicht bearbeitet
      example   = "query Kneipen feed" 
      "dbdacscan[GeoData, CID, 1000.0, 5] consume";
      
    }
  };
  
  /*
   * 7 Selection Function
   * 
   */ 
  int dbDacScanSel(ListExpr args) {
    
    ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
    ListExpr attrType;
    string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
    int found = FindAttribute(attrList, attrName, attrType);
    assert(found > 0);
    if(CcInt::checkType(attrType)) {
      return 0;
    } else if(CcReal::checkType(attrType)) {
      return 1;
    } else if(Point::checkType(attrType)) {
      return 2;
    } 
//     else if(CcString::checkType(attrType)) {
//       return 3;
//     } else if(Picture::checkType(attrType)) {
//       return 4;
//     }
    
    return -1; 
  };
  
  
  /*
   * 
   * 8 Value mapping array ~dbscanMVM~
   * 
   */
  
  ValueMapping dbDacScanVM[] = 
  {
    dbDacScanVM1<CcInt, IntMember>,
    dbDacScanVM1<CcReal, RealMember>,
    dbDacScanVM1<Point, PointMember>,
    //     dbDacScanVM1<CcString, StringMember>,
    //     dbDacScanVM1<Picture, PictureMember>
  };
  
  /*
   * 
   9 *Algebra class ~ClusterDbDacScanAlgebra~
   
   */
  class ClusterDbDacScanAlgebra : public Algebra
  {
  public:
    ClusterDbDacScanAlgebra() : Algebra()
    {
      AddOperator(dbDacScanInfo(), dbDacScanVM, 
                  dbDacScanSel, dbDacScanTM)->SetUsesMemory();}
                  
                  ~ClusterDbDacScanAlgebra() {};
  };
  
}

extern "C"
Algebra* InitializeDistributedClusteringAlgebra( NestedList* nlRef, 
                                                 QueryProcessor* qpRef)
{
  nl = nlRef;
  qp = qpRef;
  
  return (new distributedClustering::ClusterDbDacScanAlgebra());
} 