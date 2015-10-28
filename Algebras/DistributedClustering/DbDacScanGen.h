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
 
 
 #include "AlgebraTypes.h"
 #include "RelationAlgebra.h"
 #include "StandardTypes.h"
 #include "Stream.h"
 #include "Cluster.cpp"
 #include <utility>
 #include "SecondoCatalog.h"
 #include "LongInt.h"
 #include "FTextAlgebra.h"
 
 #ifndef DBDACSCANGEN_H
 #define DBDACSCANGEN_H
 using namespace std;
 
 namespace distributedClustering{
   
   const static string NEIGH_REL_MEMBER_ID = "MemberId";
   const static string NEIGH_REL_NEIGHBOR_ID = "NeighborId";
   
   template <class MEMB_TYP_CLASS, class TYPE>
   class DbDacScanGen{
   private:
/* 
1.3 members

*/
     int minPts, geoPos, clIdPos,clTypePos
          ,membIdPos,membIdNRPos,neighborIdNRPos;
     double eps;
     string newRelName;
     bool meltTwoClusters,relNameFound, clusterProcessed;
     TupleBuffer* buffer;
     GenericRelationIterator* resIt;  // iterator 
     TupleType* tt;   // the result tuple type 
     vector <MEMB_TYP_CLASS*> membArrayUntouched,membArrayUntouchedSec;
     vector <MEMB_TYP_CLASS*> membArrayPtr, membArrayPtrSec;
     Cluster<MEMB_TYP_CLASS, TYPE>* leftCluster,*rightCluster;
     
     SecondoCatalog* sc;
     const static int relCnt = 1; //3
     string relNames[relCnt]; 
     // LEFT-RIGHT-BOTH_Rel, NeighborIndexRelation, CLUSTER_Rel_finished
     
     
     //TODO delete this pointers
     Relation *nodeRel/*, *lbrRel, *neighborRel, *clusterRel*/;
     TupleType *nodeType/*, *lbrType, *neighborType, *clusterType*/;
     ListExpr nodeTypeInfo ,numNodeTypeInfo
     //, lbrTypeInfo, numLbrTypeInfo,
     //neighborTypeInfo, numNeighborTypeInfo
     //, clusterTypeInfo, numClusterTypeInfo
     ;
     Tuple *node
     //,*lbrTuple, *neighborTuple, *clusterTuple
     ;
     
   public:
     
/*
1.4 constructors

*/ 
     DbDacScanGen(Word &_inStream,  ListExpr &_tupleResultType, 
                  string& _relName, double _eps, 
                  int _minPts, int _attrPos, size_t _maxMem): 
                  minPts(_minPts), geoPos(_attrPos),
                  clIdPos(0),clTypePos(0), membIdPos(0),membIdNRPos(0)
                  ,neighborIdNRPos(0), eps(_eps),newRelName(_relName)
                  ,meltTwoClusters(false),clusterProcessed(false), buffer(0), 
                  resIt(0),tt(0),leftCluster(0),rightCluster(0)
                  ,nodeRel(0),nodeType(0)
    {
      if(createNeighborRelation())
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
        relNameFound = false;
      }
    }
    
/*
Constructor for merge clusters

*/
    DbDacScanGen(Word & _leftInStream, Word & _leftNeigInStream
                 , Word &_rightInStrem, Word &_rightNeigInStrem,
                 ListExpr &_tupleResultType,string& _relName, double _eps, 
                 int _minPts, int _geoPos, int _clIdPos
                 ,int _clTypePos, int _membIdPos, int _membIdNRPos
                 ,int _neighborIdNRPos,size_t _maxMem): 
                 minPts(_minPts),geoPos(_geoPos),clIdPos(_clIdPos)
                 ,clTypePos(_clTypePos),membIdPos(_membIdPos)
                 ,membIdNRPos(_membIdNRPos),neighborIdNRPos(_neighborIdNRPos)
                 ,eps(_eps),newRelName(_relName)
                 ,meltTwoClusters(true), clusterProcessed(false) 
                 ,buffer(0), 
                 resIt(0),tt(0),leftCluster(0),rightCluster(0)
                 ,nodeRel(0),nodeType(0)
    {
      
//     Algorithm:
//     get 4 Streams of Tuple from received bin data
//     first create 2 vector<list<MEMB_TYP_CLASS*>> of neighbors 
//       -> eventuell besse list[sizeOfMembArray]
//     of the neighborStreams
//       so vector[i] has neighborlist from member with memberId i
//     
//     then
//     create a left and a right Cluster 
//       -> consider the descending ordering of Points
//       create Cluster:
//       read in Points (same as in dbDacScan)
//       sort points in x (also same as in dbDacScan)
//       create an empty cluster
//         with ClusterType an ClusterNo deside 
//           KIND and ListNo and insert points sorted in list
//         if all points are inserted 
//           define Neighbors for LEFT BOTH and RIGHT Kinds
//         with the member id find out the correct neihbors and insert it ->
//               memberId == i -> put neighborList from vector[i]
//       end of create Cluster
//     melt this two clusters and return
// 
//     TODO überlegung für KIND CLUSTER 
//       -> beim einlesen der linken CLUSTER listen
//     könnten diese unberührt bleiben. 
//     Merke dabei die höchste ListNo und ändere ClusterNo der
//     rechten CLUSTER listen. Type bleibt dabei gleich.
//     Beim verschmelzen der Cluster addieren die gemerke längste ClusterNo
//     zur bestimmung der neuen clusterNo
    
      if(createNeighborRelation())
      {
        relNameFound = true;
        
        //some initializations
        tt = new TupleType(_tupleResultType);
        buffer = new TupleBuffer(_maxMem);
        // init leftInputStream
        init(_leftInStream,membArrayPtr,membArrayUntouched,true);
        if(membArrayPtr.size()){
          initNeighbor(_leftNeigInStream,membArrayUntouched);
          mergeSort(membArrayPtr,0,membArrayPtr.size());
        }
        // init RightInputStream
        init(_rightInStrem,membArrayPtrSec,membArrayUntouchedSec,true);
        if(membArrayPtrSec.size()){
          initNeighbor(_rightNeigInStrem,membArrayUntouchedSec);
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
//         else{
//           if(membArrayPtr.size()){
//             leftCluster = 
//             new Cluster<MEMB_TYP_CLASS, TYPE>(membArrayPtr,eps,minPts);
//           } else {
//             if(membArrayPtrSec.size()){
//               leftCluster = 
//               new Cluster<MEMB_TYP_CLASS, TYPE>(membArrayPtrSec,eps,minPts);
//             }
//           }
//         }
        
       
        initOutput(); 
      }else{
        relNameFound = false;
      }
    }
    
/*
Destructor

*/
    ~DbDacScanGen(){
      deleteEachTuple();
      if(buffer)
        delete buffer;
      if(leftCluster)
        delete leftCluster;
//       if(rightCluster) // rightClsuter where deleted from leftCluster
//         delete rightCluster;
      if(resIt) 
        delete resIt;
      if(tt) 
        tt->DeleteIfAllowed();
    }

/*
deleteEachTuple

*/
    void deleteEachTuple()
    {
      if(resIt) delete resIt;
      resIt = buffer->MakeScan(); 
      Tuple* tuple = resIt->GetNextTuple();
      while(tuple)
      {
        tuple->DeleteIfAllowed();
        tuple = resIt->GetNextTuple();
      }
    }
     
    
/*
initOutput()
Starts the begin of returning tuples.

*/
    void initOutput(){
      if(resIt) delete resIt;
      resIt = buffer->MakeScan(); 
    }
    
/*
next()
Returns the next output tuple.

*/
    Tuple* next(){ 
      if(relNameFound){
        if(resIt){
          Tuple* tuple = resIt->GetNextTuple();
          if(!tuple){
            storeRelations();
            return 0;
          }
          TupleId id = resIt->GetTupleId();
          Tuple* resTuple = new Tuple(tt);
          int noAttr = tuple->GetNoAttributes();
          
          if (clusterProcessed) {
            if(meltTwoClusters){
              noAttr = noAttr-5;
              //because the four last appended Tuple must be overwritten
            }
            for(int i = 0; i<noAttr; i++){
              resTuple->CopyAttribute(i,tuple,i);
            }
            if(id < membArrayUntouched.size()){
              
              
              putAttribute(resTuple, noAttr,id, membArrayUntouched);
//               fillRelations(membArrayUntouched[id]);
            }else{
              //TODO append second membArrayUntouchedSec
              id = id - membArrayUntouched.size();
              putAttribute(resTuple, noAttr,id, membArrayUntouchedSec);
//               fillRelations(membArrayUntouchedSec[id]);
            }
          } else { //only important for distClMerge
            for(int i = 0; i< noAttr; i++){
              resTuple->CopyAttribute(i,tuple,i);
            }
          }
          
          return resTuple;
        } else {
          storeRelations();
          return 0;
        }
      } else {
        return 0;
      }
    }
    
                  
   private:
     
/*
PutAttribute
auxiliary function to put attribute into result Tuple

*/
     void putAttribute(Tuple* resTuple,int noAttr, TupleId& id,
                       vector <MEMB_TYP_CLASS*>& array)
     {
       
       // append attribute MemberId
       long int membId = (long int) id;
       array[id]->setTupleId(membId);
       resTuple->PutAttribute(noAttr, new LongInt(true,  membId));
       
       // append attribute ClusterNo
       resTuple->PutAttribute(noAttr+1, new CcInt(true, 
                                                array[id]->getClusterNo()));
       //append attribute isCluser
       resTuple->PutAttribute(noAttr+2, 
                              new CcBool(true,
                                   array[id]->updateDensityReachable(minPts)));
       //append attribute ClusterType
       resTuple->PutAttribute(noAttr+3, 
                              new CcInt(true,
                                        array[id]->getClusterType()));
       
       //append attribute newRelName
       resTuple->PutAttribute(noAttr+4, 
                              new FText(true,relNames[0]));
       
     }
/*
1.5 initialize
 
*/
    void init(Word& _stream, 
              vector <MEMB_TYP_CLASS*>& membArray, 
              vector <MEMB_TYP_CLASS*>& membArrayUnt,
              bool distMerge = false
    )
    {
      Tuple* tuple;
      Stream<Tuple> inStream(_stream);
      inStream.open();
      while((tuple = inStream.request())){
        buffer->AppendTuple(tuple);
        TYPE* obj = (TYPE*) tuple->GetAttribute(geoPos);
        if(obj->IsDefined()){
          MEMB_TYP_CLASS* member = new MEMB_TYP_CLASS(obj);
          if(distMerge){
            CcInt* clId = (CcInt*) tuple->GetAttribute(clIdPos);
            member->setClusterNo(clId->GetIntval());
            CcInt* clType = (CcInt*) tuple->GetAttribute(clTypePos);
            member->setClusterType(clType->GetIntval ());
          }
          membArrayUnt.push_back(member);
          membArray.push_back(member);
        }
        tuple->DeleteIfAllowed();
      }
      inStream.close();
    }

/*
initNeighbor

*/
    void initNeighbor(Word& _stream
                      ,vector <MEMB_TYP_CLASS*>& membArray )
    {
      Tuple* tuple;
      Stream<Tuple> inStream(_stream);
      inStream.open();
      while((tuple = inStream.request())){

        LongInt* member = (LongInt*) tuple->GetAttribute(0);
        LongInt* membNeighbor = (LongInt*) tuple->GetAttribute(1);
        
        if(member->IsDefined() && membNeighbor->IsDefined()){
          membArray[member->GetValue()]->addNeighbor( 
                                        membArray[membNeighbor->GetValue()]);
        }
        tuple->DeleteIfAllowed();
      }
      inStream.close();
    }
     
/*
dbDacScan
 
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
createNeighborRelation()
define a relation for EPS Neighborhood

*/
    bool createNeighborRelation()
    {
      sc = SecondoSystem::GetCatalog();
      bool relNameFound = false;
      for(size_t i = 0; i < 1000 && !relNameFound; i++ )
      {
        if(findRelName(newRelName,i)){
          relNameFound = true;
        }
      }
      if(!relNameFound || !checkRelname()){
        if(!relNameFound){
          cout << relNames[0] << " is already defined" << endl;
        }
        return false;
      }
      defineRelations();
      node = new Tuple(nodeType);
      return true;
    }

/*
findRelName
find a name for a Relation which is not used yet

*/
    bool findRelName(const string& prefix, const size_t suffix) 
    {
      relNames[0] = make_string( prefix + "NInd", suffix, 20);
      
      for (int i = 0; i < relCnt; i++) { // check the new relations' names
        if (sc->IsObjectName(relNames[i])) {
          return false;
        }
      }
      return true;
    }

/*
checkRelname
check if relname is a valid name

*/
    bool checkRelname() 
    {
      for (int i = 0; i < relCnt; i++) { // check the new relations' names
        string errMsg = "error";
        if (!sc->IsValidIdentifier(relNames[i], errMsg, true)) {
          cout << errMsg << endl;
          return false;
        }
        if (sc->IsSystemObject(relNames[i])) {
          cout << relNames[i] << " is a reserved name" << endl;
          return false;
        }
      }
      return true;
    } 

/*
make_string
create a string which is filled with 0

*/
    string make_string(const string& a_prefix, size_t a_suffix,
                      size_t a_max_length)
    {
      ostringstream result;
      result << a_prefix <<  setfill('0') <<
      setw(a_max_length - a_prefix.length()) <<  a_suffix;
      return result.str();
    }


/*
defineRelations
define relation type

*/
    void defineRelations() 
    {
      nodeTypeInfo = 
      nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
        nl->TwoElemList(nl->TwoElemList(nl->SymbolAtom(NEIGH_REL_MEMBER_ID), 
                                        nl->SymbolAtom(LongInt::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom(NEIGH_REL_NEIGHBOR_ID),
                                        nl->SymbolAtom(LongInt::BasicType()))));
        numNodeTypeInfo = sc->NumericType(nodeTypeInfo);
        nodeType = new TupleType(numNodeTypeInfo);
        nodeRel = new Relation(nodeType, false);
    }
    
/*
storeRelations
fill and save relation

*/
    void storeRelations() {
      
      // TODO TEST NEW
      for(int i = 0 ; i < membArrayUntouched.size(); i++){
        fillRelations(membArrayUntouched[i],0);
      }
      for(int i = 0 ; i < membArrayUntouchedSec.size(); i++){
        fillRelations(membArrayUntouchedSec[i], membArrayUntouched.size());
      }
      node->DeleteIfAllowed();
      nodeType->DeleteIfAllowed();
      
      storeRel(relNames[0], nodeTypeInfo, nodeRel);
      for (int i = 0; i < relCnt; i++) {
        cout << "relation " << relNames[i] << " tuples stored" << endl;
      }
    }
    
    void storeRel(string name, ListExpr typeInfo, Relation *rel) {
      ListExpr type = nl->TwoElemList(nl->SymbolAtom(Relation::BasicType()),
                                      typeInfo);
      Word relWord;
      relWord.setAddr(rel);
      sc->InsertObject(name, "", type, relWord, true);
    }

/*
fillRelations()
add attributes to realation

*/
    void fillRelations(MEMB_TYP_CLASS* member, long int offset) 
    { 
      typename list<MEMB_TYP_CLASS*>::iterator nIt = 
      member->getEpsNeighborhood(true);
      while(nIt !=  member->getEpsNeighborhood(false))
      {
        node->PutAttribute(0, new LongInt(member->getTupleId() + offset) ); 
        node->PutAttribute(1, new LongInt((*nIt)->getTupleId() + offset) ); 
        nodeRel->AppendTuple(node);
        nIt++;
      }
     }

/*
mergeSort
sort an array in ascending order

*/
     void mergeSort(vector<MEMB_TYP_CLASS*>& array,int left, int right){
       MEMB_TYP_CLASS ** auxiliaryArray = new MEMB_TYP_CLASS*[right-left+1];
       if(auxiliaryArray!= 0){
         mergeSort(array,left,right,auxiliaryArray);
         
         delete [] auxiliaryArray;
       }
     }
     
/*
mergeSort
sort an array in ascending order

*/
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
leftIsMax()
auxiliary fuction to compare the maximum Object with the left object

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