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

 
[1]  DbDacScanGen

August-February 2015, Daniel Fuchs 

[TOC]

1 Class DbDacScanGen

This is a generic implemention of the DBDACSCAN algorithm.

Template parameters are MEMB\_TYP\_CLASS, the realization of
Member Class and TYPE, the Secondo Type.
 
*/
 
 
 #include "AlgebraTypes.h"
 #include "RelationAlgebra.h"
 #include "StandardTypes.h"
 #include "Stream.h"
 #include "Cluster.cpp"
 #include <utility>
 #include "SecondoCatalog.h"
 #include "LongInt.h"
 #include "FTextAlgebra.h"
 #include "BinRelWriteRead.h"
 
 #ifndef DBDACSCANGEN_H
 #define DBDACSCANGEN_H
 
 namespace distributedClustering{
   
   const static string NEIGH_REL_MEMBER_ID = "MemberId";
   const static string NEIGH_REL_NEIGHBOR_ID = "NeighborId";
   
   template <class MEMB_TYP_CLASS, class TYPE>
   class DbDacScanGen{
   private:
/* 
1.2 members

*/
     int minPts, geoPos, clIdPos,clTypePos,xPicRefPos;
     double eps;
     string outRelFileName, outNFileName;
     bool meltTwoClusters,relNameFound, clusterProcessed,appendPictureRefs;
     TupleBuffer* buffer;
     GenericRelationIterator* resIt;  // iterator 
     TupleType* tt ,*neighborType;   // the result tuple type 
     ofstream outRel, outNRel;
     vector <MEMB_TYP_CLASS*> membArrayUntouched,membArrayUntouchedSec;
     vector <MEMB_TYP_CLASS*> membArrayPtr, membArrayPtrSec;
     Cluster<MEMB_TYP_CLASS, TYPE>* leftCluster,*rightCluster;
     
     SecondoCatalog* sc;
     ListExpr neighborTupleTypeInfo, neighborTypeInfo , 
      relTupleTypeInfo,relTypeInfo;
     Tuple  *neighborTuple;
     
     //for Picture
     TYPE *xRefPic, *yRefPic;
     double maxDist;
   public:
     
/*
1.3 constructor

This constructor is for execute the DBDACSCAN algorithm.

*/ 
     DbDacScanGen(Word &_inStream,  ListExpr &_tupleResultType, 
                  string& _relName, double _eps, 
                  int _minPts, int _attrPos,
                  int _xPicRefPos, bool _appendPictureRefs, size_t _maxMem): 
                  minPts(_minPts), geoPos(_attrPos),
                  clIdPos(0),clTypePos(0),xPicRefPos(_xPicRefPos),
                  eps(_eps),outRelFileName(""),outNFileName(_relName)
                  ,meltTwoClusters(false),clusterProcessed(false),
                  appendPictureRefs(_appendPictureRefs),buffer(0), 
                  resIt(0),tt(0),neighborType(0),
                  leftCluster(0),rightCluster(0),
                  xRefPic(0),yRefPic(0),maxDist(-1.0)
    {
      ListExpr empty;
      if(createOutputFiles(empty,false))
      {
      relNameFound = true;
      tt = new TupleType(_tupleResultType);
      buffer = new TupleBuffer(_maxMem);
      init(_inStream,membArrayPtr,membArrayUntouched);
      
      if(membArrayPtr.size()){
        clusterProcessed = true;
        mergeSort(membArrayPtr,0,membArrayPtr.size());
        leftCluster = 
        dbDacScan(membArrayPtr,0,membArrayPtr.size()-1,eps,minPts);
      }
      initOutput(); 
      }else{
        cout << "NeighborRelaiton not created!" << endl;
        cout << "Filename: " << outNFileName << endl;
        relNameFound = false;
      }
    }
    
/*
1.4 Constructor for operator distClMerge

This constructor is for execute the Melt CLUSTERS algorithm.

*/
      DbDacScanGen(const string&  _leftFN, const string& _leftNFN,
                  const string&  _rightFN, const string&  _rightNFN,
                  const int _geoPos, const int _clIdPos, const int _clTypePos,
                   int _xPicRefPos,
                  const size_t _maxMem, ListExpr &_tupleResultType, 
                  ListExpr& _relFt, const string& _outRelName , 
                  string& _outNName, double _eps,int _minPts):
                  minPts(_minPts),geoPos(_geoPos),clIdPos(_clIdPos),
                  clTypePos(_clTypePos) ,xPicRefPos(_xPicRefPos), eps(_eps)
                  ,outRelFileName(_outRelName),outNFileName(_outNName)
                  ,meltTwoClusters(true),relNameFound(false)
                  , clusterProcessed(false),appendPictureRefs(false)
                  ,buffer(0), 
                  resIt(0),tt(0),neighborType(0),
                  leftCluster(0),rightCluster(0),
                  xRefPic(0),yRefPic(0),maxDist(-1.0)
    {
      bool readFileCorrect = true;
      bool readSecFileCorrect = true;
      buffer = new TupleBuffer(_maxMem);
      string errMsg;
      
      //read left rel and nrel file
      if(!readFile<TYPE,MEMB_TYP_CLASS>( _leftFN, _tupleResultType
        ,errMsg,membArrayPtr, membArrayUntouched
        ,buffer, geoPos,xPicRefPos,DISTMERGE ,clIdPos,clTypePos))
      {
        cout << "read left File failed: " << errMsg << endl;
        readFileCorrect = false;
      }  
      
      if ( !readFile<TYPE,MEMB_TYP_CLASS>( _leftNFN, _tupleResultType
        ,errMsg,membArrayPtr, membArrayUntouched
        ,buffer, geoPos,xPicRefPos,NEIGHBOR,clIdPos,clTypePos))
      {
        cout << "read left Neighbor File failed: " << errMsg << endl;
        readFileCorrect = false;
      } 

      //read right rel and nrel file
      if( !readFile<TYPE,MEMB_TYP_CLASS>( _rightFN, _tupleResultType
        ,errMsg,membArrayPtrSec, membArrayUntouchedSec
        ,buffer, geoPos,xPicRefPos,DISTMERGE,clIdPos,clTypePos
        ,membArrayPtr.size()))
      {
        cout << "read right File failed: " << errMsg << endl;
        readSecFileCorrect = false;
      } 
      if (!readFile<TYPE,MEMB_TYP_CLASS>( _rightNFN, _tupleResultType
        ,errMsg,membArrayPtrSec, membArrayUntouchedSec
        ,buffer, geoPos,xPicRefPos,NEIGHBOR,clIdPos,clTypePos
        ,membArrayPtr.size()))
      {
        cout << "read right Neighbor File failed: " << errMsg << endl;
        readSecFileCorrect = false;
      }
      

      if(readFileCorrect && createOutputFiles(_relFt))
      {
        relNameFound = true;
        tt = new TupleType(_tupleResultType);
        
        if(readSecFileCorrect){
        if(membArrayPtr.size()){
          mergeSort(membArrayPtr,0,membArrayPtr.size());
        }
        if(membArrayPtrSec.size()){
          mergeSort(membArrayPtrSec,0,membArrayPtrSec.size());
        }
        //define border Points
        TYPE* leftInnerPoint=0;
        TYPE* rightInnerPoint=0;
        
       //create a right and a left Cluster
        if (membArrayPtr.size() && membArrayPtrSec.size() ) 
        {
          clusterProcessed = true;
          
          if(membArrayPtr.at(0)->getXVal() > membArrayPtrSec.at(0)->getXVal() )
          {
            leftCluster = 
            new Cluster<MEMB_TYP_CLASS, TYPE>(membArrayPtr,eps,minPts);
            rightCluster = 
            new Cluster<MEMB_TYP_CLASS, TYPE>(membArrayPtrSec,eps,minPts);
            leftInnerPoint = membArrayPtr.back()->getPoint();
            rightInnerPoint = membArrayPtrSec.front()->getPoint();
          }else{
            leftCluster = 
            new Cluster<MEMB_TYP_CLASS, TYPE>(membArrayPtrSec,eps,minPts);
            rightCluster = 
            new Cluster<MEMB_TYP_CLASS, TYPE>(membArrayPtr,eps,minPts);
            leftInnerPoint = membArrayPtrSec.back()->getPoint();
            rightInnerPoint = membArrayPtr.front()->getPoint();
          }
          //melt Clusters
          leftCluster->
          meltClusters(rightCluster,leftInnerPoint,rightInnerPoint);
        } 
        }
        initOutput(); 
      }else{
        cout << "Files not created!" << endl;
        cout << "RelFilename: " << outRelFileName <<
        "RelNFileName: " << outNFileName << endl;
        relNameFound = false;
      }
    }
    
/*
1.5 ~Destructor~

*/
    ~DbDacScanGen(){
      if(outRel.is_open()){
        finish(outRel);
        outRel.close();
      }
      if(outNRel.is_open()){
        finish(outNRel);
        outNRel.close();
      }
      deleteEachTuple();
      if(buffer)
        delete buffer;
      if(leftCluster)
        delete leftCluster;
      if(resIt) 
        delete resIt;
      if(tt) 
        tt->DeleteIfAllowed();
      if(neighborType)
        neighborType->DeleteIfAllowed();
      if(neighborTuple)
        neighborTuple->DeleteIfAllowed();
    }

/*
1.6  ~deleteEachTuple~

Delete each tuple from tuple buffer.

*/
    void deleteEachTuple()
    {
      if(resIt) delete resIt;
      if(buffer){
      resIt = buffer->MakeScan(); 
      Tuple* tuple = resIt->GetNextTuple();
      while(tuple)
      {
        tuple->DeleteIfAllowed();
        tuple = resIt->GetNextTuple();
      }
      }
    }
     
    
/*
1.7 ~initOutput~

Initialize the buffer to returning tuples.

*/
    void initOutput(){
      if(resIt) delete resIt;
      resIt = buffer->MakeScan(); 
    }
    
/*
1.8 ~next~

Returns the next output tuple which are expandet with
ClusterId, ClusterType, isCluster and the filname of 
neighbor relationship file.
Requires the call of initOutput before.

*/
    Tuple* next(){ 
      if(relNameFound){
        if(resIt){
          Tuple* tuple = resIt->GetNextTuple();
          if(!tuple){
            return 0;
          }
          TupleId id = resIt->GetTupleId();
          Tuple* resTuple = new Tuple(tt);
          int noAttr = tuple->GetNoAttributes();
       
          if (clusterProcessed) {
            if(meltTwoClusters){
                noAttr = noAttr-5;
            }
            for(int i = 0; i<noAttr; i++){
              resTuple->CopyAttribute(i,tuple,i);
            }
            if(appendPictureRefs){
              resTuple->PutAttribute(noAttr, xRefPic->Clone());
              noAttr++;
            }
            if(id < membArrayUntouched.size()){
              putAttribute(resTuple, noAttr,id, membArrayUntouched);
            }else{ 
              id = id - membArrayUntouched.size();
              putAttribute(resTuple, noAttr,id, membArrayUntouchedSec);
            }
          } else { //only important for distClMerge
            for(int i = 0; i< noAttr; i++){
              resTuple->CopyAttribute(i,tuple,i);
            }
            if(id < membArrayUntouched.size()){
              writeFiles(resTuple,membArrayUntouched[id]);
            }else{
              id = id - membArrayUntouched.size();
              writeFiles(resTuple,membArrayUntouchedSec[id]);
            }
          }
          
          return resTuple;
        } else {
          return 0;
        }
      } else {
        return 0;
      }
    }
    
                  
   private:
     
/*
1.9 ~PutAttribute~

Auxiliary function to put attribute into result Tuple.

*/
     void putAttribute(Tuple* resTuple,int noAttr, TupleId& id,
                       vector <MEMB_TYP_CLASS*>& array)
     {
       if(appendPictureRefs){
         resTuple->PutAttribute(noAttr, 
                                new CcReal(true,array[id]->getYVal()));
         noAttr++;
       }
       resTuple->PutAttribute(noAttr, new LongInt(true,   
                                                  array[id]->getTupleId()));
       
       // append attribute ClusterNo
       resTuple->PutAttribute(noAttr+1, new CcInt(true, 
                                                array[id]->getClusterNo()));
       //append attribute isCluser
       resTuple->PutAttribute(noAttr+2, 
                              new CcBool(true,
                                   array[id]->updateDensityReachable(minPts)));
       //append attribute ClusterType
       if(array[id]->getClusterType() > 4){
         resTuple->PutAttribute(noAttr+3, 
                                new CcInt(true,-1));
       }else{
       resTuple->PutAttribute(noAttr+3, 
                              new CcInt(true,
                                        array[id]->getClusterType()));
       }
       //append attribute outNFileName
       resTuple->PutAttribute(noAttr+4, 
                              new FText(true,outNFileName));
       
       //write File
       writeFiles(resTuple,array[id]);
     }
     
/*
1.10 ~writeFiles~

write tuples to file

*/
    void writeFiles(Tuple* resTuple,MEMB_TYP_CLASS* member)
    {
      if(meltTwoClusters){
      //write RelFile
      if(!writeNextTuple(outRel,resTuple)){
        cerr << "Problem in writing tuple" << endl;
      }
      }
      //write NRelFile
      if(member->getClusterType() != 1){ 
        //for type CLUSTER you don't nead information about neighbors
      writeNeighborFileTuples(member);
      }
    }
     
/*
1.11 ~writeNeighborFileTuples~

write neighbor tuples to file

*/
void writeNeighborFileTuples(MEMB_TYP_CLASS* member) 
    { 
   
      
      typename list<MEMB_TYP_CLASS*>::iterator nIt = 
      member->getEpsNeighborhood(true);
      while(nIt !=  member->getEpsNeighborhood(false))
      {
        neighborTuple->PutAttribute(0, new LongInt(true,member->getTupleId())); 
        neighborTuple->PutAttribute(1, new LongInt(true,(*nIt)->getTupleId())); 
        if(!writeNextTuple(outNRel,neighborTuple)){
          cerr << "Problem in writing tuple" << endl;
        }
        
        nIt++;
      }
     }
     
/*
1.12 ~initialize~
 
 Read in the tuple streams and store them in 
 a vector.
 
*/
    void init(Word& _stream, 
              vector <MEMB_TYP_CLASS*>& membArray, 
              vector <MEMB_TYP_CLASS*>& membArrayUnt
    )
    {
      Tuple* tuple;
      Stream<Tuple> inStream(_stream);
      inStream.open();
      int id = 0;
      
      bool firstRun = true;
      bool secondRun = true;
      bool findPictureCoordRefs = false;
      bool pictureRefsExist = false;
      if(TYPE::BasicType() == Picture::BasicType()
        && appendPictureRefs)
      {
        findPictureCoordRefs = true;
        pictureRefsExist = false;
      }else if(TYPE::BasicType() == Picture::BasicType()
        && !appendPictureRefs
        && xPicRefPos >=0 )
      {
        findPictureCoordRefs = false;
        pictureRefsExist = true;
      }
      while((tuple = inStream.request())){
        buffer->AppendTuple(tuple);
        TYPE* obj = (TYPE*) tuple->GetAttribute(geoPos);
        if(obj->IsDefined()){
          tuple->SetTupleId(id);
          MEMB_TYP_CLASS* member = new MEMB_TYP_CLASS(obj);
          member->setTupleId(id);
          membArrayUnt.push_back(member);
          membArray.push_back(member);
          
          if(pictureRefsExist){
            xRefPic = (TYPE*) tuple->GetAttribute(xPicRefPos);
            double yRefPicVal = ((CcReal*) 
                              tuple->GetAttribute(xPicRefPos+1))->GetValue();
            member->setCoordinates(xRefPic,yRefPicVal);
          }
          if(findPictureCoordRefs)
          {
            if(firstRun)
            {
              xRefPic = obj; 
              firstRun = false;
            }
            else //  if(!firstRun && findPictureCoordRefs)
            {
              if(secondRun){
                yRefPic = obj;
                maxDist = member->calcDistanz(xRefPic);
              } else 
              if(member->calcDistanz(xRefPic) > maxDist)
              {
                yRefPic = obj;
                maxDist = member->calcDistanz(xRefPic);
              }
            }
          }
        }
        tuple->DeleteIfAllowed();
        id++;
      }
      if(findPictureCoordRefs)
      {
        //search maxDist
        for (int i = 1; i < membArray.size()-1;i++)
        {
          if( membArray.at(i)->calcDistanz(yRefPic) > maxDist)
          {
            xRefPic = membArray.at(i)->getPoint();
            maxDist =membArray.at(i)->calcDistanz(yRefPic);
          }
        }
        //set coordinates to each member
        for (int i = 0; i < membArray.size();i++)
        {
          membArray.at(i)->setCoordinates(xRefPic,yRefPic);
        }
      }
      
      inStream.close();
    }

/*
1.13 ~createOutputFiles~
 
 Creates the output files in which tuples where stored.
 
*/
bool createOutputFiles(ListExpr& _relFt, bool both=true)
    {
      string errMsg;
      if(both){
        relTupleTypeInfo = _relFt;
        relTypeInfo = nl->TwoElemList( listutils::basicSymbol<Relation>(),
                                       nl->Second(_relFt));
        //create output relation file
        if(!writeHeader(outRel,outRelFileName,relTypeInfo,errMsg))
        {
          cerr << errMsg << endl;
          return false;
        }
      }
     neighborTupleTypeInfo = defineNRel();
     neighborTypeInfo = nl->TwoElemList( listutils::basicSymbol<Relation>(),
                                        neighborTupleTypeInfo);
     sc = SecondoSystem::GetCatalog();
     neighborType = new TupleType(sc->NumericType(neighborTupleTypeInfo));
     neighborTuple = new Tuple(neighborType);
     //create output neighbor relation file
     if(!writeHeader(outNRel, outNFileName,neighborTypeInfo,errMsg))
     {
       cerr << "writeHeader not Successfully: " << errMsg << endl;
       return false;
     }
     return true;
    }
    
/*
1.14 ~defineNRel~

Define neigbor relation type.

*/
    ListExpr defineNRel() 
    {
      return
      nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
        nl->TwoElemList(nl->TwoElemList(nl->SymbolAtom(NEIGH_REL_MEMBER_ID), 
                                        nl->SymbolAtom(LongInt::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom(NEIGH_REL_NEIGHBOR_ID),
                                        nl->SymbolAtom(LongInt::BasicType()))));
    }
    
     
/*
1.15 ~dbDacScan~

Execute the DBDACSCAN algorithm.
 
*/
Cluster<MEMB_TYP_CLASS, TYPE>* 
dbDacScan(vector<MEMB_TYP_CLASS*>& _membArray, int left , int right , 
          double eps, int minPts)
{
  if(right==left){//Array contains only one element
    return 
    new Cluster<MEMB_TYP_CLASS, TYPE>(_membArray[left], eps,minPts);
  }else{
    int globMedian = (right + left)/2;//position to the right subarray
    
    //get left and right cluster
    Cluster<MEMB_TYP_CLASS, TYPE> *rightCl, *leftCl;
    leftCl = dbDacScan(_membArray,left,
                            globMedian,eps,minPts);
    rightCl = dbDacScan(_membArray,
                             globMedian+1,right,eps,minPts);
    
    int leftInnerIdex = globMedian;
    int rightInnerIdex = globMedian+1;
    
    leftCl->meltClusters(rightCl,
                         _membArray[leftInnerIdex]->getPoint(),
                         _membArray[rightInnerIdex]->getPoint());
    return leftCl;
  }
  return 0; //should never reached
}

/*
1.16 ~mergeSort~

sort an array in ascending order

*/
     void mergeSort(vector<MEMB_TYP_CLASS*>& array,int left, int right){
       MEMB_TYP_CLASS ** auxiliaryArray = new MEMB_TYP_CLASS*[right-left+1];
       if(auxiliaryArray!= 0){
         mergeSort(array,left,right,auxiliaryArray);
         
         delete [] auxiliaryArray;
       }
     }
     

     void mergeSort(vector<MEMB_TYP_CLASS*>& array,int left, 
                    int right,MEMB_TYP_CLASS** auxiliaryArray){
       if(right == left+1)
         return ; //mergeSort finisch
         else{
           int i = 0;
           int length = right - left;
           int median = (right - left)/2;
           int l = left; //position to the left subarray
           int r = left + median; //position to the right subarray
           
           //divide array
           mergeSort(array, left, r, auxiliaryArray);
           mergeSort(array, r, right, auxiliaryArray);
           
           //merge array
           /* Check to see if any elements remain in the left array; if so,
            * we check if there are any elements left in the right array; if
            * so, we compare them.  Otherwise, we know that the merge must
            * use take the element from the left array */
           for(i = 0; i < length; i++){
             if(l < left+median && (r==right || leftIsMax(array, l, r))){
               auxiliaryArray[i]=array[l];
               l++;
             }
             else{
               auxiliaryArray[i]= array[r];
               r++;
             }
           }
           //Copy the sorted subarray back to the input array
           for(i=left; i < right; i++){
             array[i]=auxiliaryArray[i-left];
           }
         }
     }
     
/*
1.17 ~leftIsMax~

Auxiliary fuction to compare the maximum Object with the left object.

*/
     bool leftIsMax(vector<MEMB_TYP_CLASS*>& array,int left, int right){
       bool retVal = false;
       
       double leftXVal = array[left]->getXVal();
       double rightXVal = array[right]->getXVal();
       
       leftXVal > rightXVal ? retVal = true : retVal = false;
       return retVal;
     }
   };
   
 }
 #endif
