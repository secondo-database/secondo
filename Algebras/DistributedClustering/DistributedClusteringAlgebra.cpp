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


//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]


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
#include "FTextAlgebra.h"
#include "DistClOp.h"
#include "DistSampleSort.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace std;

namespace distributedClustering{
      
  const static string MEMBER_ID = "MemberId";
  const static string IS_CLUSTER = "IsCluster";
  const static string CLUSTER_TYPE = "ClusterType";
  const static string NEIGHBORS_RELATION_NAME = "NeighborsRelN";
  
/*
3 Operator dbDacScan


This operated realizes a inmemory db-scan for spatial objects
with a divide and conquer algorithm

In the divide step the space is divided with a kd-tree similar index
structure.

Input:
Stream(Tuple) x Attributename x ClusterId x EPS x MinPts -> Stream(Tuple)
Stream(Tuple) x attrname x string x double x int -> Stream(Tuple)

4 Type Mapping ~dbDacScan~

*/
  ListExpr
  dbDacScanTM(ListExpr args){
    //check Types in args
    string errMsg = "";
    if(!nl->HasLength(args,2)){ 
      errMsg = "Two elements expexted. First must be a stream"
      " of tuple and second must be an argument list";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //check if first in-Parameter is a stream
    ListExpr stream = nl->First(args);
    if(!Stream<Tuple>::checkType(nl->First(args))) {
      errMsg = "first argument is not stream(Tuple)";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //check if second in-Parameter is a list of args
    ListExpr arguments = nl->Second(args);
    
    if(!nl->HasLength(arguments,5)){
      errMsg = "Second argument is not a conform list of input" 
      " Parameters such, "
      " Attributename  x ClusterId x NewRelNamePrefix x EPS x MinPts ";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //check NewRelNamePrefix
    if(!CcString::checkType(nl->Third(arguments))
//       || !FText::checkType(nl->Third(arguments))
    ) {
      errMsg = "Third argument is NewRelNamePrefix and must be a string";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //check EPS
    if(!CcReal::checkType(nl->Fourth(arguments))) {
      errMsg = "Fourth argument is EPS and must be a real";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //check MinPts
    if(!CcInt::checkType(nl->Fifth(arguments))) {
      errMsg = "Fifth argument is MinPts and must be a int";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    //check the cluster attribute name if it is existing in a tuple 
    ListExpr attrList = nl->Second(nl->Second(stream));
    ListExpr attrType;
    string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
    
    int found = FindAttribute(attrList, attrName, attrType);
    
    if(found <= 0) {
      errMsg= "Attribute "
      + attrName + " is no member of the tuple";
      return listutils::typeError(errMsg);;
    }
    //Check attrType
    if( !CcInt::checkType(attrType)
      && !CcReal::checkType(attrType)
      && !Point::checkType(attrType))
      {
      errMsg = "Attribute "+attrName+"is not of type "
      + CcInt::BasicType() + ", " 
      + CcReal::BasicType() + ", " 
      + Point::BasicType() + ", " 
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
      
      // append LongInt for MemberId
      newAttrListEnd = nl->Append(newAttrListEnd,
                                  (nl->TwoElemList(
                                    nl->SymbolAtom(MEMBER_ID),
                                        nl->SymbolAtom(LongInt::BasicType()))));
      //append int for ClusterID
      newAttrListEnd = nl->Append(newAttrListEnd,
                                  (nl->TwoElemList(
                                    name,
                                    nl->SymbolAtom(CcInt::BasicType()))));
      
      //append bool for Visited
      newAttrListEnd = nl->Append(newAttrListEnd,
                                  (nl->TwoElemList(
                                    nl->SymbolAtom(IS_CLUSTER),
                                            nl->SymbolAtom(CcBool::BasicType())
                                  )));
      //append info for ClusterType  (CLUSTER = 1 , LEFT = 2, 
      //    RIGHT = 3, BOTH = 4,
      //CLUSTERCAND = -1 ,NOISE = -2)
      newAttrListEnd = nl->Append(newAttrListEnd,
                                  (nl->TwoElemList(
                                    nl->SymbolAtom(CLUSTER_TYPE),
                                            nl->SymbolAtom(CcInt::BasicType())
                                  )));
      
      //append RelNames - NeighborsRelName
      newAttrListEnd = nl->Append(newAttrListEnd,
                                  (nl->TwoElemList(
                                    nl->SymbolAtom(NEIGHBORS_RELATION_NAME),
                                            nl->SymbolAtom(FText::BasicType())
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
5 Value mapping mehtod ~dbDacScan~

*/
  template <class TYPE, class MEMB_TYP_CLASS>
  int dbDacScanVM1(Word* args, Word& result, int message, 
                   Word& local, Supplier s) {
    typedef DbDacScanGen<MEMB_TYP_CLASS,TYPE> dbdacscanClass;

    dbdacscanClass* li = (dbdacscanClass*) local.addr;
    switch (message) //TODO switch not necessary only retunr bool val
    {
      case OPEN:
      {
        //get stream
        Word inStream = args[0];
        // get arguments
        Word argument;
        //get NewRelNamePrefix
        Supplier sup = qp->GetSupplier(args[1].addr,2);
        qp->Request(sup, argument);
        string relName = static_cast<CcString*>(argument.addr)->GetValue();
        //get EPS
        sup = qp->GetSupplier(args[1].addr,3);
        qp->Request(sup, argument);
        double eps = static_cast<CcReal*>(argument.addr)->GetValue();
        //get MinPts
        sup = qp->GetSupplier(args[1].addr, 4);
        qp->Request(sup, argument);
        int minPts = static_cast<CcInt*>(argument.addr)->GetValue();
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
//         if(!eps->IsDefined() || eps->GetValue() < 0){
        if(eps < 0 ){
          return 0;
        }
//         if(!minPts->IsDefined() || minPts->GetValue() < 0){
        if(minPts < 0){
          return 0;
        }
        local.addr = new dbdacscanClass(inStream,tt, 
                                        relName,
                                        eps,
                                        minPts, 
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
6 Struct ~dbDacScanInfo~ 

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
      " dbdacscan[GeoData, CID, 500.0, 5] sortby[No]"
      " groupby[No ; C : group count] count";
      
    }
  };
  
/*
7 Selection Function
selection function for one input stream

*/ 
  int dbDacScanSel(ListExpr args) {
    
    ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
    ListExpr attrType;
    string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
    int found = FindAttribute(attrList, attrName, attrType);
//     assert(found > 0);
    if(found > 0 ){
      if(CcInt::checkType(attrType)) {
        return 0;
      } else if(CcReal::checkType(attrType)) {
        return 1;
      } else if(Point::checkType(attrType)) {
        return 2;
      } 
    }
    else
    {
      cout << " no ValueMapping found" << endl;
    }
    
    return -1; 
  };
  
  
/*
8 Value mapping array ~dbscanMVM~

*/
  
  ValueMapping dbDacScanVM[] = 
  {
    dbDacScanVM1<CcInt, IntMember>,
    dbDacScanVM1<CcReal, RealMember>,
    dbDacScanVM1<Point, PointMember>,
  };
  
  
/*
3 Operator ~distclmerge~

Type Mapping of operator ~distclmerge~

*/
// Signature is: Stream(Tuple) x Stream(Tuple) x Stream(Tuple) x Stream(Tuple) x
// Attributename x  Attributename x String x EPS x MinPts  -> Stream(Tuple)
// 
// RelStream x NeighborStream x 2_relstream x 2_neighborStream x
// GeoData x ClusterId x "newRelName" x eps x minpts
// 
// Algorithm:
// get 2 Streams of Tuple from received bin data
// create a left and a right Cluster 
//   -> consider the descending ordering of Points
//     create Cluster:
//       read in Points (same as in dbDacScan)
//       sort points in x (also same as in dbDacScan)
//       create an empty cluster
//         with ClusterType an ClusterNo deside KIND and 
//             ListNo and insert points sorted in list
//         if all points are inserted 
//             define Neighbors for LEFT BOTH and RIGHT Kinds
//           define Neighbors: 
//             from Border Point to dist < eps search Neighbors for each point
//         end of create Cluster
// melt this two clusters and return

  ListExpr
  distClMergeTM(ListExpr args){
    string errMsg = "Five elements expexted. First four must be a stream"
    " of tuple and last must be an argument list";
    if(!nl->HasLength(args,5)) 
    {
      return listutils::typeError(errMsg);
    }
    
    //check streams
    //check if first in-Parameter is a stream
    ListExpr leftStrem = nl->First(args);
    ListExpr leftNeighborStrem = nl->Second(args);
    ListExpr rightStrem = nl->Third(args);
    ListExpr rightNeighborStrem = nl->Fourth(args);
    
    if(!Stream<Tuple>::checkType(leftStrem) 
      || !Stream<Tuple>::checkType(leftNeighborStrem)
      || !Stream<Tuple>::checkType(rightStrem) 
      || !Stream<Tuple>::checkType(rightNeighborStrem))
    {
      errMsg = " First four arguments must be a Stream of Tuples! ";
      return listutils::typeError(errMsg);
    }
    
    //check Arguments
    ListExpr arguments = nl->Fifth(args);
    
    if(!nl->HasLength(arguments,5)){ 
      errMsg = "Second argument is not a conform list of input" 
      " Parameters such, "
      " Attributename x ClusterId x newRelname x EPS x MinPts ";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    //check NewRelNamePrefix
    if(!CcString::checkType(nl->Third(arguments))
      //       || !FText::checkType(nl->Third(arguments))
    ) {
      errMsg = "Third argument is NewRelNamePrefix and must be a string";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //check EPS
    if(!CcReal::checkType(nl->Fourth(arguments))) {
      errMsg = "Third argument is EPS and must be a real";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //check MinPts
    if(!CcInt::checkType(nl->Fifth(arguments))) {
      errMsg = "Fourth argument is MinPts and must be a int";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //=====================================================
    //NeighborRel
    //=====================================================
    //check neighborRelation if there are the same Attributes
    ListExpr attrNListLeft = nl->Second(nl->Second(leftNeighborStrem)); 
    ListExpr attrNListRight = nl->Second(nl->Second(rightNeighborStrem));
    //check NeighborMembId
    ListExpr membAttrL,membAttrR;
    int foundMembIdNL = FindAttribute(attrNListLeft, 
                                      NEIGH_REL_MEMBER_ID, membAttrL);
    int foundMembIdNR = FindAttribute(attrNListRight, 
                                      NEIGH_REL_MEMBER_ID, membAttrR);
    if(foundMembIdNL <= 0 || foundMembIdNR <= 0 
      || foundMembIdNL != foundMembIdNR || foundMembIdNL != 1) 
    {
      errMsg= "Attribute "
      + NEIGH_REL_MEMBER_ID + 
      " is no member of both tuple on equal position in Neighbor Relation";
      return listutils::typeError(errMsg);
    }
    if( !LongInt::checkType(membAttrL) ||  !LongInt::checkType(membAttrR))
    {
      errMsg = "Attribute "+NEIGH_REL_MEMBER_ID+"is not of type "
      + LongInt::BasicType() + "! ";
      return listutils::typeError(errMsg);
    }
    //check Neighbor NeighborId
    ListExpr neighborAttrL,neighborAttrR;
    int foundNeighIdL = FindAttribute(attrNListLeft, 
                                      NEIGH_REL_NEIGHBOR_ID, neighborAttrL);
    int foundNeighIdR = FindAttribute(attrNListRight, 
                                      NEIGH_REL_NEIGHBOR_ID, neighborAttrR);
    if(foundNeighIdL <= 0 || foundNeighIdR <= 0 
      || foundNeighIdL != foundNeighIdR || foundNeighIdL != 2) {
      errMsg= "Attribute "
      + NEIGH_REL_NEIGHBOR_ID + 
      " is no member of both tuple on equal position in Neighbor Relation";
      return listutils::typeError(errMsg);
    }
    if( !LongInt::checkType(neighborAttrL) 
      || !LongInt::checkType(neighborAttrR))
    {
      errMsg = "Attribute "+NEIGH_REL_NEIGHBOR_ID+"is not of type "
      + LongInt::BasicType() + "! ";
      return listutils::typeError(errMsg);
    }
    
    //========================================================================
    //Member Relation
    //========================================================================
    //check the cluster attribute name if it is existing in a tuple 
    ListExpr attrListLeft = nl->Second(nl->Second(leftStrem)); 
    ListExpr attrListRight = nl->Second(nl->Second(rightStrem));
    
    //Check attribute Type of GeoData
    ListExpr geDAttrL,geDAttrR;
    string attrNGeoData = nl->SymbolValue(nl->First(arguments));
    int foundGeoL = FindAttribute(attrListLeft, attrNGeoData, geDAttrL);
    int foundGeoR = FindAttribute(attrListRight, attrNGeoData, geDAttrR);
    
    if(foundGeoL <= 0 || foundGeoR <= 0 || foundGeoL != foundGeoR ) {
      errMsg= "Attribute "
      + attrNGeoData + " is no member of both tuple on equal position";
      return listutils::typeError(errMsg);
    }
    
    if( !CcInt::checkType(geDAttrL)
      && !CcReal::checkType(geDAttrL)
      && !Point::checkType(geDAttrL))
    {
      errMsg = "Attribute "+attrNGeoData+"is not of type "
      + CcInt::BasicType() + ", " 
      + CcReal::BasicType() + ", " 
      + Point::BasicType() + ", " 
      +"!";
      return listutils::typeError(errMsg);
    }
    
    //Check attribute Type of ClusterId
    ListExpr clNoAttrL,clNoAttrR;
    string attrNClusterID = nl->SymbolValue(nl->Second(arguments));
    int foundClIdL = FindAttribute(attrListLeft, attrNClusterID, clNoAttrL);
    int foundClIdR = FindAttribute(attrListRight, attrNClusterID, clNoAttrR);
    if(foundClIdL <= 0 || foundClIdR <= 0 || foundClIdL != foundClIdR) {
      errMsg= "Attribute "
      + attrNClusterID + " is no member of the tuple";
      return listutils::typeError(errMsg);
    }
    if( !CcInt::checkType(clNoAttrL) ||  !CcInt::checkType(clNoAttrR))
    {
      errMsg = "Attribute "+attrNClusterID+"is not of type "
      + CcInt::BasicType() + "! ";
      return listutils::typeError(errMsg);
    }
    

    //Check ClsuterType
    ListExpr clTypeL,clTypeR;
    int foundClTyL = FindAttribute(attrListLeft, CLUSTER_TYPE, clTypeL);
    int foundClTyR = FindAttribute(attrListRight, CLUSTER_TYPE, clTypeR);
    if(foundClTyL <= 0 || foundClTyR <= 0 || foundClTyL != foundClTyR) {
      errMsg= "Attribute "
      + CLUSTER_TYPE + " is no member of the tuple";
      return listutils::typeError(errMsg);
    }
    if( !CcInt::checkType(clTypeL) || !CcInt::checkType(clTypeR))
    {
      errMsg = "Attribute "+CLUSTER_TYPE+"is not of type "
      + CcInt::BasicType() + "! ";
      return listutils::typeError(errMsg);
    }
    
    //Check MemberId
    ListExpr membL,membR;
    int foundMembIdL = FindAttribute(attrListLeft, MEMBER_ID, membL);
    int foundMembIdR = FindAttribute(attrListRight, MEMBER_ID, membR);
    if(foundMembIdL <= 0 || foundMembIdR <= 0 
      || foundMembIdL != foundMembIdR) {
      errMsg= "Attribute "
      + MEMBER_ID + " is no member of the tuple";
      return listutils::typeError(errMsg);
    }
    if( !LongInt::checkType(membL) || !LongInt::checkType(membR))
    {
      errMsg = "Attribute "+MEMBER_ID+" is not of type "
      + LongInt::BasicType() + "! ";
      return listutils::typeError(errMsg);
    }
    
    //Check NeighborRelation name
    ListExpr neighborRL,neighborRR;
    int foundNeighRelL = FindAttribute(attrListLeft, 
                                       NEIGHBORS_RELATION_NAME, neighborRL);
    int foundNeighRelR = FindAttribute(attrListRight, 
                                       NEIGHBORS_RELATION_NAME, neighborRR);
    if(foundNeighRelL <= 0 || foundNeighRelR <= 0 
      || foundNeighRelL != foundNeighRelR) {
      errMsg= "Attribute "
      + NEIGHBORS_RELATION_NAME + " is no member of the tuple";
      return listutils::typeError(errMsg);
    }
    if( !FText::checkType(neighborRL) || !FText::checkType(neighborRR))
    {
      errMsg = "Attribute "+NEIGHBORS_RELATION_NAME+" is not of type "
      + FText::BasicType() + "! ";
      return listutils::typeError(errMsg);
    }
   
    //Copy attrList to newAttrList
    ListExpr oldAttrList = nl->Second(nl->Second(leftStrem));
    ListExpr newAttrList = nl->OneElemList(nl->First(oldAttrList));
    ListExpr newAttrListEnd = newAttrList;
    oldAttrList = nl->Rest(oldAttrList);
    
    while(!(nl->IsEmpty(oldAttrList))){
      newAttrListEnd = nl->Append(newAttrListEnd,nl->First(oldAttrList));
      oldAttrList = nl->Rest(oldAttrList);
    }
    
    return nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND())
      ,nl->FourElemList(nl->IntAtom(foundGeoL-1) // position of GeoData
      ,nl->IntAtom(foundClIdL-1) // position of ClusterId
      ,nl->IntAtom(foundClTyL-1) // position of ClusterType
      ,nl->IntAtom(foundMembIdL-1)) // position of MemberId
      ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                       nl->TwoElemList(
                         nl->SymbolAtom(Tuple::BasicType()),
                                       newAttrList)));
   
  }
  
  
/*
Value Mapping of operator ~distclmerge~

*/
  template <class TYPE, class MEMB_TYP_CLASS> 
  int
  distClMergeVM1(Word* args, Word& result, int message, 
              Word& local, Supplier s)
  {
    typedef DbDacScanGen<MEMB_TYP_CLASS,TYPE > distClMergeClass;
    distClMergeClass* li = (distClMergeClass*) local.addr;
    
    switch (message)
    {
      case OPEN:
      {
        //get stream
        Word leftStream = args[0];
        Word leftNeighborStream = args[1];
        Word rightStream = args[2];
        Word rightNeighborStream = args[3];
        // get arguments
        Word argument;
        
        //get NewRelNamePrefix
        Supplier sup = qp->GetSupplier(args[4].addr,2);
        qp->Request(sup, argument);
        string relName = static_cast<CcString*>(argument.addr)->GetValue();
        //get EPS
        sup = qp->GetSupplier(args[4].addr,3);
        qp->Request(sup, argument);
        double eps = static_cast<CcReal*>(argument.addr)->GetValue();
        //get MinPts
        sup = qp->GetSupplier(args[4].addr, 4);
        qp->Request(sup, argument);
        int minPts = static_cast<CcInt*>(argument.addr)->GetValue();
        
        //set index of GeoData attribute position in tuple
        int geoPos = static_cast<CcInt*>(args[5].addr)->GetIntval();
        //set index of ClusterId attribute position in tuple
        int clIdPos = static_cast<CcInt*>(args[6].addr)->GetIntval();
        //set index of ClusterType attribute position in tuple
        int clTypePos = static_cast<CcInt*>(args[7].addr)->GetIntval();
        //set index of MemberId attribute position in tuple
        int membId = static_cast<CcInt*>(args[8].addr)->GetIntval();
        //set index of MemberId attribute position in tuple of NeighborRel
        
        int membIdNR = 0; //TODO delete
//         static_cast<CcInt*>(args[9].addr)->GetIntval();
        //set index of NeighborId attribute position in tuple of NeighborRel
        int neighborIdNR = 0;
//         static_cast<CcInt*>(args[10].addr)->GetIntval();
        
        //set the result type of the tuple
        ListExpr resultType = GetTupleResultType(s);
        ListExpr tt = ( nl->Second( resultType ) );
        if(li) { 
          delete li;
          local.addr=0;
        }
        size_t maxMem = qp->GetMemorySize(s)*1024*1024;
        if(eps < 0){
          return 0;
        }
        if(minPts < 0){
          return 0;
        }
        
        local.addr = new distClMergeClass(leftStream,leftNeighborStream
                                          ,rightStream , rightNeighborStream
                                          ,tt,  relName, eps, minPts
                                          ,geoPos,clIdPos,clTypePos
                                          ,membId,membIdNR,neighborIdNR
                                          ,maxMem);
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
struct of operator ~distclmerge~

*/
  struct distClMergeInfo :  OperatorInfo
  {
    distClMergeInfo() : OperatorInfo()
    {
      name      = "distclmerge";
      signature = "stream(Tuple) x stream(Tuple) x stream(Tuple) x"
      " stream(Tuple) x Id x string x real x int  -> stream(Tuple)";
      syntax    = "_ _ _ _distclmerge[_,_,_,_,_]"; 
      meaning   = "";//TODO ab hier noch nicht bearbeitet
      example   ="query leftDbScanRel feed leftClRelationNInd00 feed "
      "rightDbScanRel feed rightClRelationNInd0 feed distclmerge[GeoData, "
      "ClusterId, \"newRelName\", 500.0, 5] sortby[ClusterId] groupby"
      "[ClusterId ; C : group count] count";
    }
  };
  
/*
7 Selection Function
selection function for one input stream

*/ 
  int distClMergeSel(ListExpr args) {
    
    ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
    ListExpr attrType;
    string attrName = nl->SymbolValue(nl->First(nl->Fifth(args)));
    int found = FindAttribute(attrList, attrName, attrType);
    //     assert(found > 0);
    if(found > 0 ){
      if(CcInt::checkType(attrType)) {
        return 0;
      } else if(CcReal::checkType(attrType)) {
        return 1;
      } else if(Point::checkType(attrType)) {
        return 2;
      } 
    }
    else
    {
      cout << " no ValueMapping found" << endl;
    }
    
    return -1; 
  };
  
/*
8 Value mapping array ~distclmerge~

*/
  ValueMapping distClMergeVM[] = 
  {
    distClMergeVM1<CcInt, IntMember>,
    distClMergeVM1<CcReal, RealMember>,
    distClMergeVM1<Point, PointMember>,
  };
  
  
  
/*
3 Operator ~distsamp~

Type Mapping of operator ~distsamp~

Signature is: stream(tuple(relation)) x stream(tuple(sample)) x Attr ->
stream(tuple(sortedRelation))

sorts the given sample and set the worker number to the given relation

*/
  ListExpr
  distsampTM(ListExpr args){
    string err = "two tuple streams and Attributename value expected";
    if(!nl->HasLength(args,3)) 
    {
      return listutils::typeError(err);
    }
    
    //check streams
    if(!Stream<Tuple>::checkType(nl->First(args)) ||
      !Stream<Tuple>::checkType(nl->Second(args)))
    {
      return listutils::typeError(err);
    }
    
    ListExpr arguments = nl->Third(args);
//     ListExpr arguments = nl->Second(args);
    
    //check CntWorkers
    if(!CcInt::checkType(nl->Third(arguments))) {
      err = "Fourth argument is Count Workers and must be a int";
      ErrorReporter::ReportError(err);
      return nl->TypeError();
    }
    
    //check AttrName
    ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
    ListExpr attrListSamp = nl->Second(nl->Second(nl->Second(args)));
    ListExpr attrType, attrTypeSamp;
    string attrName = nl->SymbolValue(nl->First(arguments));
    int found = FindAttribute(attrList, attrName, attrType);
    int foundSamp = FindAttribute(attrListSamp, attrName, attrTypeSamp);
    
    if(found <= 0 || foundSamp <= 0 || found != foundSamp) {
      err= "Attribute " + attrName + " is no member of the tuple"
      ". Both streams must have the same Attribute on the same position.";
      return listutils::typeError(err);
    }
    
    //Check attrType
    if(( !CcInt::checkType(attrType)
      && !CcReal::checkType(attrType)
      && !Point::checkType(attrType)) ||
      (!CcInt::checkType(attrTypeSamp)
      && !CcReal::checkType(attrTypeSamp)
      && !Point::checkType(attrTypeSamp)))
    {
      err = "Attribute "+attrName+"is not of type "
      + CcInt::BasicType() + ", " 
      + CcReal::BasicType() + ", " 
      + Point::BasicType() + ", " 
      +"! Both streams must have the same Attribute!";
      return listutils::typeError(err);
    }
    
    //Check WorkerId
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
    ListExpr stream = nl->First(args);
    
    ListExpr oldAttrList = nl->Second(nl->Second(stream));
    ListExpr newAttrList = nl->OneElemList(nl->First(oldAttrList));
    ListExpr newAttrListEnd = newAttrList;
    oldAttrList = nl->Rest(oldAttrList);
    
    while(!(nl->IsEmpty(oldAttrList))){
      newAttrListEnd = nl->Append(newAttrListEnd,nl->First(oldAttrList));
      oldAttrList = nl->Rest(oldAttrList);
    }
    //append int for WorkerID
    newAttrListEnd = nl->Append(newAttrListEnd,
                                (nl->TwoElemList(
                                  name,
                                  nl->SymbolAtom(CcInt::BasicType()))));
    return nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND())
      ,nl->OneElemList(nl->IntAtom(found-1))
      ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                       nl->TwoElemList(
                         nl->SymbolAtom(Tuple::BasicType()),
                                       newAttrList)));
    
  }
  
/*
ValueMapping of operator ~distsamp~

*/
  template <class TYPE, class MEMB_TYP_CLASS> 
  int
  distsampVM1(Word* args, Word& result, int message, 
           Word& local, Supplier s)
  {
    typedef Distsamp<MEMB_TYP_CLASS,TYPE > distSample;
    distSample* li = (distSample*) local.addr;
       
    switch (message)
    {
      case OPEN:
      {
        Word inStream = args[0];
        Word sampStream = args[1];
//         Word sampStream;
        //get CntWorkers
        Word argument;
        Supplier sup = qp->GetSupplier(args[2].addr, 2);
        qp->Request(sup, argument);
        int cntWorkers = (static_cast<CcInt*>(argument.addr))->GetIntval();
        //get AttrPos:
        int attrPos = static_cast<CcInt*>(args[3].addr)->GetIntval();
        cout << "attrPos= " << attrPos << endl;
        
        ListExpr resultType = GetTupleResultType(s);
        ListExpr tt = ( nl->Second( resultType ) );
        if(li) { 
          delete li;
          local.addr=0;
        }
        size_t maxMem = qp->GetMemorySize(s)*1024*1024;
        local.addr = new distSample(inStream, sampStream,
                                         tt, attrPos, cntWorkers, maxMem);
        
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
struct of operator ~distsamp~

*/
  struct distsampInfo :  OperatorInfo
  {
    distsampInfo() : OperatorInfo()
    {
      name      = "distsamp";
      signature = "stream(Tuple) -> stream(Tuple)";
      syntax    = "_ _ distsamp[_,_,_]"; 
      meaning   = "";//TODO ab hier noch nicht bearbeitet
      example   = "query Kneipen feed ten feed "
                  "distsamp[GeoData, WorkerId, 4] count ";
      
    }
  };

/*
7 Selection Function of operator ~distsamp~
selection funktion for 2 input streams

*/ 
  int distsampSel(ListExpr args) {
    ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
    ListExpr attrType;
    string attrName = nl->SymbolValue(nl->First(nl->Third(args)));
    int found = FindAttribute(attrList, attrName, attrType);
    
    if(found > 0 ){
      if(CcInt::checkType(attrType)) {
        return 0;
      } else if(CcReal::checkType(attrType)) {
        return 1;
      } else if(Point::checkType(attrType)) {
        return 2;
      } 
    }
    else
    {
      cout << " no ValueMapping found" << endl;
    }
    
    return -1;
  }
  
/*
8 Value mapping array ~distsamp~

*/
  
  ValueMapping distsampVM[] = 
  {
    distsampVM1<CcInt, IntMember>,
    distsampVM1<CcReal, RealMember>,
    distsampVM1<Point, PointMember>,
  };
  
  
/*
9 Algebra class ~ClusterDbDacScanAlgebra~

*/
  class ClusterDbDacScanAlgebra : public Algebra
  {
  public:
    ClusterDbDacScanAlgebra() : Algebra()
    {
      AddOperator(dbDacScanInfo(), dbDacScanVM, 
                  dbDacScanSel, dbDacScanTM)->SetUsesMemory();
     AddOperator(distClMergeInfo(), distClMergeVM, 
                 distClMergeSel, distClMergeTM)->SetUsesMemory();
//       AddOperator(distClInfo(), distClVM, 
//                   distClTM)->SetUsesMemory();
      AddOperator(distsampInfo(), distsampVM, 
                  distsampSel, distsampTM)->SetUsesMemory();
    }
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