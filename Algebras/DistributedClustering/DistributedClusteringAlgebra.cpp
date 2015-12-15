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


This file contains the implementation of the distributedClustering Algebra

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
#include "DistSampleSort.h"
#include "BinRelWriteRead.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace std;

namespace distributedClustering{
      
  const static string MEMBER_ID = "MemberId";
  const static string IS_CLUSTER = "IsCluster";
  const static string CLUSTER_TYPE = "ClusterType";
  const static string NEIGHBORS_RELATION_NAME = "NeighborFName";
  const static string PIC_X_REF = "PicXRef";
  const static string PIC_Y_REF = "PicYRefVal";
  
/*
3 Operator dbDacScan

This operated realizes a inmemory db-scan for spatial objects
with a divide and conquer algorithm

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
      " Attributename  x ClusterId x NeiborRelFileName x EPS x MinPts ";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //check NeiborRelFileName
    if(!FText::checkType(nl->Third(arguments))
    ) {
      errMsg = "Third argument is NeiborRelFileName and must be a text";
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
      && !Point::checkType(attrType)
      && !Picture::checkType(attrType))
      {
      errMsg = "Attribute "+attrName+" is not of type "
      + CcInt::BasicType() + ", " 
      + CcReal::BasicType() + ", " 
      + Point::BasicType() + ", " 
      + Picture::BasicType() + ", "
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
      
      //check if there exist picture coordinate references
      bool pictureRefExist = false;
      int foundXRef = 0;
      if(Picture::checkType(attrType)){
        ListExpr picXType,picYType;
        foundXRef = FindAttribute(attrList, PIC_X_REF, picXType);
        int foundYRef = FindAttribute(attrList, PIC_Y_REF, picYType);
        if(foundXRef > 0 && foundYRef > 0
          && Picture::checkType(picXType)
          && CcReal::checkType(picYType))
        {
          pictureRefExist = true;
        }
      }
      
      //Copy attrList to newAttrList
      ListExpr oldAttrList = nl->Second(nl->Second(stream));
      ListExpr newAttrList = nl->OneElemList(nl->First(oldAttrList));
      ListExpr newAttrListEnd = newAttrList;
      oldAttrList = nl->Rest(oldAttrList);
      
      while(!(nl->IsEmpty(oldAttrList))){
        newAttrListEnd = nl->Append(newAttrListEnd,nl->First(oldAttrList));
        oldAttrList = nl->Rest(oldAttrList);
      }
      
      //if picture append x and y references
      if(Picture::checkType(attrType) && !pictureRefExist){
        newAttrListEnd = nl->Append(newAttrListEnd,
                                    (nl->TwoElemList(
                                      nl->SymbolAtom(PIC_X_REF),
                                        nl->SymbolAtom(Picture::BasicType())
                                    )));
        newAttrListEnd = nl->Append(newAttrListEnd,
                                    (nl->TwoElemList(
                                      nl->SymbolAtom(PIC_Y_REF),
                                          nl->SymbolAtom(CcReal::BasicType())
                                    )));
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
      
      //append bool for is Cluster
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
        ,nl->ThreeElemList(nl->IntAtom(found-1),
                           nl->IntAtom(foundXRef-1),
                           nl->BoolAtom(
                             (Picture::checkType(attrType) 
                                      && !pictureRefExist)))
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
    switch (message)
    {
      case OPEN:
      {
        //get stream
        Word inStream = args[0];
        // get arguments
        Word argument;
        //get NewRelNamePrefix
        Supplier sup = qp->GetSupplier(args[1].addr,2);
//         Supplier sup = qp->GetSupplier(args[3].addr,2);
        qp->Request(sup, argument);
        string relName = static_cast<FText*>(argument.addr)->GetValue();
        //get EPS
        sup = qp->GetSupplier(args[1].addr,3);
//         sup = qp->GetSupplier(args[3].addr,3);
        qp->Request(sup, argument);
        double eps = static_cast<CcReal*>(argument.addr)->GetValue();
        //get MinPts
        sup = qp->GetSupplier(args[1].addr, 4);
//         sup = qp->GetSupplier(args[3].addr, 4);
        qp->Request(sup, argument);
        int minPts = static_cast<CcInt*>(argument.addr)->GetValue();
        //set index of attribute position in tuple
        int attrPos = static_cast<CcInt*>(args[2].addr)->GetIntval();
        
        //get xRef Position for pictures
        int xPicRefPos = static_cast<CcInt*>(args[3].addr)->GetIntval();
        bool appendPictureRefs = 
                      static_cast<CcBool*>(args[4].addr)->GetValue();
        
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
                                        xPicRefPos,
                                        appendPictureRefs,
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
8 Value mapping array ~dbscanMVM~

*/
  
  ValueMapping dbDacScanVM[] = 
  {
    dbDacScanVM1<CcInt, IntMember>,
    dbDacScanVM1<CcReal, RealMember>,
    dbDacScanVM1<Point, PointMember>,
    dbDacScanVM1<Picture, PictureMember>,
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
    if(found > 0 ){
      if(CcInt::checkType(attrType)) {
        return 0;
      } else if(CcReal::checkType(attrType)) {
        return 1;
      } else if(Point::checkType(attrType)) {
        return 2;
      } else if(Picture::checkType(attrType)){
        return 3;
      }
    }
    else
    {
      cout << " no ValueMapping found" << endl;
    }
    return -1; 
  };
  
  
/*
1.4.5 Specification

*/
OperatorSpec dbDacScanSpec(
" stream(Tuple) x Attr x Attr x text x real x int -> stream(Tuple) ",
    "_ dbdacscan[list]",
      "Clusters an Inputstream according to the first Attribute Name"
      " which is given in the Argument lsit. The second Attribute"
      " Name is the name of ClusterId with which the relation is"
      " expanded. The text is the filename in which the Neighbor"
      " Relation would be stored.",
      "query Kneipen feed dbdacscan [GeoData,"
      " ClusterId,'testN',500.0, 5] sortby[ClusterId]"
      " groupby[ClusterId ; C : group count] count;"
  );
  
/*
1.4.6 Instance

*/
  Operator dbDacScanOp("dbdacscan",
                       dbDacScanSpec.getStr(),
                       4,
                       dbDacScanVM,
                       dbDacScanSel,
                       dbDacScanTM
  );
  
/*
3 Operator ~distclmerge~

Type Mapping of operator ~distclmerge~

*/
  ListExpr
  distClMergeTM(ListExpr args){
    string errMsg = "";
    if(!nl->HasLength(args,10)) 
    {
      errMsg = "10 elements expexted. First four must be a Filname"
      " of Relations next two must be Attributenames then real and integer"
      " value is expexted and at last two output Filenames";
      return listutils::typeError(errMsg);
    }
    
    //check Files
    ListExpr leftFileName = nl->First(args);
    if(!nl->HasLength(leftFileName,2)){
      return listutils::typeError("internal error");
    }
    ListExpr leftRelType; //equal to stream
    if(!checkFile(leftFileName,leftRelType,errMsg)){
      return listutils::typeError(errMsg);
    }
    
    ListExpr leftNFielName = nl->Second(args);
    if(!nl->HasLength(leftNFielName,2)){
      return listutils::typeError("internal error");
    }
    ListExpr leftNRelType; //equal to stream
    if(!checkFile(leftNFielName,leftNRelType,errMsg)){
      return listutils::typeError(errMsg);
    }
    
    ListExpr righFileName = nl->Third(args);
    if(!nl->HasLength(righFileName,2)){
      return listutils::typeError("internal error");
    }
    ListExpr rightRelType; //equal to stream
    if(!checkFile(righFileName,rightRelType,errMsg)){
      return listutils::typeError(errMsg);
    }
    
    ListExpr rightNFileName = nl->Fourth(args);
    if(!nl->HasLength(rightNFileName,2)){
      return listutils::typeError("internal error");
    }
    ListExpr rightNRelType; //equal to stream
    if(!checkFile(rightNFileName,rightNRelType,errMsg)){
      return listutils::typeError(errMsg);
    }
            
    //check EPS
    if(!CcReal::checkType(nl->First(nl->Seventh(args))))
    {
      errMsg = "Seventh argument is EPS and must be a real";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //check MinPts
    if(!CcInt::checkType(nl->First(nl->Eigth(args)))) 
    {
      errMsg = "Eigth argument is MinPts and must be a int";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //check OutRelFileName
    if(!FText::checkType(nl->First(nl->Ninth(args)))) 
    {
      errMsg = "Ninth argument is NewRelNamePrefix and must be a string";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //check OutNeighborFileName
    if(!FText::checkType(nl->First(nl->Tenth(args)))) 
    {
      errMsg = "Tenth argument is NewRelNamePrefix and must be a string";
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
    }
    
    //=====================================================
    //NeighborRel
    //=====================================================
    //check neighborRelation if there are the same Attributes
    ListExpr attrNListLeft = nl->Second(nl->Second(leftNRelType)); 
    ListExpr attrNListRight = nl->Second(nl->Second(rightNRelType));
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
    ListExpr attrListLeft = nl->Second(nl->Second(leftRelType)); 
    ListExpr attrListRight = nl->Second(nl->Second(rightRelType));
    
    //Check attribute Type of GeoData
    ListExpr geDAttrL,geDAttrR;
    string attrNGeoData =  nl->SymbolValue(nl->First(nl->Fifth(args)));
    int foundGeoL = FindAttribute(attrListLeft, attrNGeoData, geDAttrL);
    int foundGeoR = FindAttribute(attrListRight, attrNGeoData, geDAttrR);
    
    if(foundGeoL <= 0 || foundGeoR <= 0 || foundGeoL != foundGeoR ) {
      errMsg= "Attribute "
      + attrNGeoData + " is no member of both tuple on equal position";
      return listutils::typeError(errMsg);
    }
    
    if( !CcInt::checkType(geDAttrL)
      && !CcReal::checkType(geDAttrL)
      && !Point::checkType(geDAttrL)
      && !Picture::checkType(geDAttrL))
    {
      errMsg = "Attribute "+attrNGeoData+"is not of type "
      + CcInt::BasicType() + ", " 
      + CcReal::BasicType() + ", " 
      + Point::BasicType() + ", " 
      + Picture::BasicType() + ", " 
      +"!";
      return listutils::typeError(errMsg);
    }
    
    //Check attribute Type of ClusterId
    ListExpr clNoAttrL,clNoAttrR;
    string attrNClusterID = nl->SymbolValue(nl->First(nl->Sixth(args)));
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
    
    //Check NeighborRelationName Attribute
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
    
    //if it is Picture Type search and check coordinate references for x and y
    int foundXRefL = 0;
    if(Picture::checkType(geDAttrL)){
      ListExpr picXTypeL,picYTypeL, picXTypeR, picYTypeR;
      foundXRefL = FindAttribute(attrListLeft, PIC_X_REF, picXTypeL);
      int foundYRefL = FindAttribute(attrListLeft, PIC_Y_REF, picYTypeL);
      int foundXRefR = FindAttribute(attrListRight, PIC_X_REF, picXTypeR);
      int foundYRefR = FindAttribute(attrListRight, PIC_Y_REF, picYTypeR);
      if(foundXRefL <= 0 || foundYRefL <= 0 || foundYRefL != foundYRefR
        || foundXRefL != foundXRefR
        || !Picture::checkType(picXTypeL) || !Picture::checkType(picXTypeR) 
        || !CcReal::checkType(picYTypeL) || !CcReal::checkType(picYTypeR) )
      {
        errMsg = "Attribute "+attrNGeoData+" is of type "
        + Picture::BasicType() + " but there are not correct"
        " coordinate references for x and y! Reference "
        "for coordinate x must be of type "+ Picture::BasicType() +
        " and reference for y must be of type "+ CcReal::BasicType();
        return listutils::typeError(errMsg);
      }
    }

    
    return nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND())
      ,nl->FourElemList(nl->IntAtom(foundGeoL-1) // position of GeoData
      ,nl->IntAtom(foundClIdL-1) // position of ClusterId
      ,nl->IntAtom(foundClTyL-1) // position of ClusterType
      ,nl->IntAtom(foundXRefL-1) // position of coordinate reference x
      ) 
      ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                     nl->Second(leftRelType)
                       ));
  }
  
  
/*
Value Mapping of operator ~distclmerge~

this the template method which is called from distClMergeVM

*/
  template <class TYPE, class MEMB_TYP_CLASS> 
  int
  distClMergeVMT(Word* args, Word& result, int message, 
                  Word& local, Supplier s)
  {   
    typedef DbDacScanGen<MEMB_TYP_CLASS,TYPE > distClMergeClass;
    distClMergeClass* li = (distClMergeClass*) local.addr;
    
    switch (message)
    {
      case OPEN:
      {
        ListExpr resultType = GetTupleResultType(s);
        ListExpr tt = ( nl->Second( resultType ) ); //RelType
        //TupleType for file 
        ListExpr relFileType = qp->GetType(s);
             
        //get Filenames
        FText* fnL = (FText*) args[0].addr;
        if(!fnL->IsDefined()){
          return 0;
        }
        
        FText* fnNL = (FText*) args[1].addr;
        if(!fnNL->IsDefined()){
          return 0;
        }
        
        FText* fnR = (FText*) args[2].addr;
        if(!fnR->IsDefined()){
          return 0;
        }
        
        FText* fnNR = (FText*) args[3].addr;
        if(!fnNR->IsDefined()){
          return 0;
        }
        
        //get EPS
        double eps = static_cast<CcReal*>(args[6].addr)->GetValue();
        //get MinPts
        int minPts = static_cast<CcInt*>(args[7].addr)->GetValue();
        //OutRelFileName
        string outRelFileName = static_cast<FText*>(args[8].addr)->GetValue();
        //OutNeighborFileName
        string outNFileName = static_cast<FText*>(args[9].addr)->GetValue();
        //set index of GeoData attribute position in tuple
        int geoPos = static_cast<CcInt*>(args[10].addr)->GetIntval();
        //  set index of ClusterId attribute position in tuple
        int clIdPos = static_cast<CcInt*>(args[11].addr)->GetIntval();
        //set index of ClusterType attribute position in tuple
        int clTypePos = static_cast<CcInt*>(args[12].addr)->GetIntval();
        //get xRef Position for pictures
        int xPicRefPos = static_cast<CcInt*>(args[13].addr)->GetIntval();
        
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
       
        local.addr = 
        new distClMergeClass(fnL->GetValue(), fnNL->GetValue(),
                             fnR->GetValue(), fnNR->GetValue(),   
                             geoPos,clIdPos,clTypePos,xPicRefPos, maxMem
                             ,tt, relFileType, 
                             outRelFileName,outNFileName,
                             eps, minPts);
           
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
Value Mapping of operator ~distclmerge~

value mapping acts as selection function.
Because the relation is passed as a file, there is no way to find
out the type of the passed Attributename in selection function.

*/
  int
  distClMergeVM(Word* args, Word& result, int message, 
                Word& local, Supplier s)
  {
    //set index of GeoData attribute position in tuple
    int geoPos = static_cast<CcInt*>(args[10].addr)->GetIntval();
    
    //determine AttrType
    ListExpr relFileType = qp->GetType(s);
    ListExpr attrType = nl->Second( nl->Second( relFileType ) );
    
    for(int i= 0;i<geoPos; i++){
      attrType = nl->Rest(attrType);
    }
    attrType = nl->Second(nl->First(attrType));
    
    if(CcInt::checkType(attrType)) 
    {
      return 
      distClMergeVMT<CcInt ,IntMember>(args,result,message,local, s);
    } else 
      if(CcReal::checkType(attrType)) 
      {
        return  
        distClMergeVMT<CcReal, RealMember > (args,result,message,local, s);
      } else 
        if(Point::checkType(attrType)) 
        {
          return 
          distClMergeVMT<Point, PointMember >(args,result,message,local, s);
        }  else 
          if(Picture::checkType(attrType)){
            return distClMergeVMT<Picture, PictureMember >
            (args,result,message,local, s);
          }else {
          return 0;
        }
  } 

/*
1.4.5 Specification

*/
  
OperatorSpec distClMergeSpec( " text x text x text x text "
  "x Attr x Attr x real x int x text x text  -> stream(Tuple)",
    "distclmerge(_,_,_,_,_,_,_,_,_,_)",
    "Performs a function to merge alredy clustered part relations.",
    "query distclmerge('DbScan.bin' , 'NeighborFile.bin' "
    ",'DbScan.bin_1','NeighborFile.bin_1 ' , GeoData, ClusterId, "
    "500.0 , 5 , 'DbScan.bin' ,'NeighborFile.bin' ) consume"
  );
  
/*
1.4.6 Instance

*/
Operator distClMergeOp(
    "distclmerge",
    distClMergeSpec.getStr(),
                    distClMergeVM,
                    Operator::SimpleSelect,
                    distClMergeTM
  );
  
  
/*
3 Operator ~distsamp~

Type Mapping of operator ~distsamp~

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
      && !Point::checkType(attrType)
      && !Picture::checkType(attrType)) ||
      (!CcInt::checkType(attrTypeSamp)
      && !CcReal::checkType(attrTypeSamp)
      && !Point::checkType(attrTypeSamp)
      && !Picture::checkType(attrTypeSamp)
      ))
    {
      err = "Attribute "+attrName+" is not of type "
      + CcInt::BasicType() + ", " 
      + CcReal::BasicType() + ", " 
      + Point::BasicType() + ", " 
      + Picture::BasicType() + ", " 
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
    
    //check if there exist picture coordinate references
    bool pictureRefExist = false;
    int foundXRef = 0;
    
    if(Picture::checkType(attrType)){
      ListExpr picXType,picYType;
      foundXRef = FindAttribute(attrList, PIC_X_REF, picXType);
      int foundYRef = FindAttribute(attrList, PIC_Y_REF, picYType);
      if(foundXRef > 0 && foundYRef > 0
        && Picture::checkType(picXType)
        && CcReal::checkType(picYType))
      {
        pictureRefExist = true;
      }
    }
    
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
    
    //append after WorkerId -> so always the last 
    //two attributes are the references for picture
    // at dbdacscan appen the attributes at first
    if(Picture::checkType(attrType) && !pictureRefExist){
      newAttrListEnd = nl->Append(newAttrListEnd,
                                  (nl->TwoElemList(
                                    nl->SymbolAtom(PIC_X_REF),
                                          nl->SymbolAtom(Picture::BasicType())
                                  )));
      newAttrListEnd = nl->Append(newAttrListEnd,
                                  (nl->TwoElemList(
                                    nl->SymbolAtom(PIC_Y_REF),
                                          nl->SymbolAtom(CcReal::BasicType())
                                  )));
    }
    
    return nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND())
//       ,nl->OneElemList(nl->IntAtom(found-1))
      ,nl->ThreeElemList(nl->IntAtom(found-1),
                         nl->IntAtom(foundXRef-1),
                         nl->BoolAtom(
                           (Picture::checkType(attrType) 
                           && !pictureRefExist)))
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
        
        //get xRef Position for pictures
        int xPicRefPos = static_cast<CcInt*>(args[4].addr)->GetIntval();
        bool appendPictureRefs = 
        static_cast<CcBool*>(args[5].addr)->GetValue();
        
        ListExpr resultType = GetTupleResultType(s);
        ListExpr tt = ( nl->Second( resultType ) );
        if(li) { 
          delete li;
          local.addr=0;
        }
        size_t maxMem = qp->GetMemorySize(s)*1024*1024;
        local.addr = new distSample(inStream, sampStream,
                                         tt, attrPos,
                                    xPicRefPos, appendPictureRefs,
                                    cntWorkers, maxMem);
        
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
      signature = "stream(T) x stream(T) x Attr x Attr x int -> stream(T)";
      syntax    = "_ _ distsamp[_,_,_]"; 
      meaning   = "As input a relation and a sample relation is excpected."
      "According to the first attribute, the relation would be sorted."
      " The second attributes which stores the WorkerID , would added."
      " Based on the sample relation, boundary points are determined "
      "once the tuple allocated to the relevant workers.";
      example   = "query Kneipen feed Kneipen feed head[50] "
                  "distsamp[GeoData, WId, 4] count ";
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
      } else if(Picture::checkType(attrType)){
        return 3;
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
    distsampVM1<Picture, PictureMember>
  };
  
/*
9 Algebra class ~ClusterDbDacScanAlgebra~

*/
  class ClusterDbDacScanAlgebra : public Algebra
  {
  public:
    ClusterDbDacScanAlgebra() : Algebra()
    {
      AddOperator(&dbDacScanOp);
//       dbDacScanOp.SetUsesArgsInTypeMapping();
      dbDacScanOp.SetUsesMemory();
      
      AddOperator(&distClMergeOp);
      distClMergeOp.SetUsesArgsInTypeMapping();
      distClMergeOp.SetUsesMemory();

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