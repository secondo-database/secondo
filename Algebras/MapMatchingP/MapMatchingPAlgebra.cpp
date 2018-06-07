/*
----
This file is part of SECONDO.

Copyright (C) 2007, 
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

*/


#include "Attribute.h"          // implementation of attribute types
#include "Algebra.h"            // definition of the algebra
#include "NestedList.h"         // required at many places
#include "NList.h"              // replaces NestedList.h
#include "QueryProcessor.h"     // needed for implementing value mappings
#include "AlgebraManager.h"     // e.g., check for a certain kind
#include "Operator.h"           // for operator creation
#include "StandardTypes.h"      // priovides int, real, string, bool type
#include "Algebras/FText/FTextAlgebra.h"
#include "Symbols.h"            // predefined strings
#include "ListUtils.h"          // useful functions for nested lists
#include "Algebras/Stream/Stream.h"  // wrapper for secondo streams

#include "GenericTC.h"          // use of generic type constructors

#include "LogMsg.h"             // send error messages

#include "Tools/Flob/DbArray.h"  // use of DbArrays

#include "Algebras/Relation-C++/RelationAlgebra.h"  // use of tuples
#include "Algebras/NestedRelation/NestedRelationAlgebra.h"

#include "Algebras/Spatial/SpatialAlgebra.h"     // Lines, regions etc.
#include "Algebras/Spatial/RegionTools.h"      // Lines, regions etc.


#include "Algebras/OrderedRelation/OrderedRelationAlgebra.h"
#include "Progress.h"               // from RelationAlgebra

#include <math.h>               // required for some operators
#include <stack>
#include <limits>
#include <iostream>

#include "TypeMapUtils.h"   // For type mapping with SimpleMap

#include "SecParser.h"   // Secondo Parser (query strings to nested list)

//#include <string> // std::string
#include <algorithm> // std::transform
#include <cctype> // std::tolower

#include <sstream>

#include "AlmostEqual.h"

//#include <cmath>
//#include <string>
//#include <cstdlib>

//#include "RegionMgmt.h" // Own for region management



extern NestedList* nl;
extern NList* nlNew;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;
using namespace mappings;

namespace mapMatchPed{


// Represents a matched point.
struct MatchedPoint{
    // Values from relation (recorded by GPS receiver / application)
    int regId;
    int regIdPointInside;
    string PosDate;
    string PosTime;
    string RegionName;
    string RegionType;
    double latOrg;
    double lonOrg;
    int numOfSats;
    double accuracy; // Less is better

    // Calculated values
    int PosYear;
    int PosMonth;
    int PosDay;
    int PosHour;
    int PosMinute;
    int PosSecond;
    Region* matchedReg;
    bool matched;
    int NumEdgesToPrev;
    double distToPrev;
    double distToReg;


    MatchedPoint(int theRegId)
    {
        regId = theRegId;
        regIdPointInside = 0;
        PosDate = "";
        PosTime = "";
        RegionName = "";
        RegionType = "";
        latOrg = 0.0;
        lonOrg = 0.0;

        PosYear = 0;
        PosMonth = 0;
        PosDay = 0;
        PosHour = 0;
        PosMinute = 0;
        PosSecond = 0;

        numOfSats = 0;
        accuracy = 30.0;
        matchedReg = 0;
        matched = false;
        NumEdgesToPrev = 0;
        distToPrev = 0.0;
        distToReg = 0.0;

    }



    // Normal case constructor
    MatchedPoint(string thePosDate, string thePosTime, double theLatitude, 
        double theLongitude, int theNumOfSat, double theAccuracy)
    {

        regId = 0;
        regIdPointInside = 0;
        PosDate = thePosDate;
        PosTime = thePosTime;
        RegionName = "";
        RegionType = "";
        latOrg = theLatitude;
        lonOrg = theLongitude;

        PosYear = 0;
        PosMonth = 0;
        PosDay = 0;
        PosHour = 0;
        PosMinute = 0;
        PosSecond = 0;

        numOfSats = theNumOfSat;
        accuracy = theAccuracy;
        matchedReg = 0;
        matched = false;
        NumEdgesToPrev = 0;
        distToPrev = 0.0;
        distToReg = 0.0;

        if(accuracy < 0.001)
            accuracy = 30.0;

    }


    MatchedPoint(MatchedPoint* otherPtr)
    {

        regId = otherPtr->regId;
        regIdPointInside = otherPtr->regIdPointInside;
        PosDate = otherPtr->PosDate;
        PosTime = otherPtr->PosTime;
        RegionName = otherPtr->RegionName;
        RegionType = otherPtr->RegionType;
        latOrg = otherPtr->latOrg;
        lonOrg = otherPtr->lonOrg;

        PosYear = otherPtr->PosYear;
        PosMonth = otherPtr->PosMonth;
        PosDay = otherPtr->PosDay;
        PosHour = otherPtr->PosHour;
        PosMinute = otherPtr->PosMinute;
        PosSecond = otherPtr->PosSecond;

        numOfSats = otherPtr->numOfSats;
        accuracy = otherPtr->accuracy;
        matchedReg = otherPtr->matchedReg;
        matched = otherPtr->matched;
        NumEdgesToPrev = otherPtr->NumEdgesToPrev;
        distToPrev = otherPtr->distToPrev;
        distToReg = otherPtr->distToReg;

    }


    ~MatchedPoint() {
        
    }

    bool operator<(const MatchedPoint& otherMP) const
    {
        return regId < otherMP.regId;
    }

    bool operator==(const MatchedPoint& otherMP) const
    {
        return regId == otherMP.regId;
    }


};




static void CopyCandidateEntries( vector<MatchedPoint*>* 
            orgMatchedPointsVectorPtr, 
            vector<MatchedPoint*>* copyMatchedPointsVectorPtr);
 //static double CalcDistance(MatchedPoint* matchPtA, MatchedPoint* matchPtB);

//vector<MatchedPoint*>* actTripVectorPtr
// Represents a trip candidate.
struct MatchCandidate{
    // Values from relation (recorded by GPS receiver / application)
    vector<MatchedPoint*>* TripCandidateVectorPtr;
    int NumEdgesTotal;
    double AverageSats;
    double AverageAcc; // Smaller values are better
    double DistanceTotal;
    double DistanceToMatchedRegionsTotal;
    double Score; // Smaller values are better


    MatchCandidate(vector<MatchedPoint*>* vecPtr)
    {
        TripCandidateVectorPtr = vecPtr;
        NumEdgesTotal = 0;
        DistanceTotal = 0.0;
        DistanceToMatchedRegionsTotal = 0.0;
        AverageSats = 0.0;
        AverageAcc = 100.0;
        Score = 0;

    }


    MatchCandidate(MatchCandidate* otherPtr)
    {
        CopyCandidateEntries(otherPtr->TripCandidateVectorPtr, 
                TripCandidateVectorPtr);
        NumEdgesTotal = otherPtr->NumEdgesTotal;
        DistanceTotal = otherPtr->DistanceTotal;
        DistanceToMatchedRegionsTotal = 
                    otherPtr->DistanceToMatchedRegionsTotal;
        AverageSats = otherPtr->AverageSats;
        AverageAcc = otherPtr->AverageAcc;
        Score = otherPtr->Score;

    }


    ~MatchCandidate() {
        
    }

    void UpdateInternalValues(int startIndex){

        //cout << "       +++++-- UpdateInternalValues called. \n";

        if(TripCandidateVectorPtr->size() == 0){
            // Nothing to update.
            return;
        }

        int tmpNumEdgesTotal = 0;
        double tmpAverageSats = 0.0;
        double tmpAverageAcc = 0.0;
        double tmpDistanceTotal = 0.0;
        double tmpDistanceToMatchedRegionsTotal = 0.0;

        for(unsigned int i = startIndex; 
            i < TripCandidateVectorPtr->size(); i++){
            MatchedPoint* actTmpMatchPoint = (*TripCandidateVectorPtr)[i];
            tmpNumEdgesTotal += actTmpMatchPoint->NumEdgesToPrev;
            tmpAverageSats += actTmpMatchPoint->numOfSats;
            tmpAverageAcc += actTmpMatchPoint->accuracy;
            tmpDistanceTotal += actTmpMatchPoint->distToPrev;
            tmpDistanceToMatchedRegionsTotal += actTmpMatchPoint->distToReg;

            if(actTmpMatchPoint->regIdPointInside != 0){
                // Exact matched region found. Compare to candidate region.
                if(actTmpMatchPoint->regIdPointInside != 
                                actTmpMatchPoint->regId){
                    // Candidate region for matching was not exactly matched, 
                        //another region was. 
//                    tmpDistanceToMatchedRegionsTotal += 1000.0;

                }
            }


        }

        if(TripCandidateVectorPtr->size() > 1){
            tmpAverageSats = tmpAverageSats / 
                (TripCandidateVectorPtr->size() - startIndex);
            tmpAverageAcc = tmpAverageAcc / 
                (TripCandidateVectorPtr->size() - startIndex);
        }
        if(startIndex > 0){
            NumEdgesTotal += tmpNumEdgesTotal;
            DistanceTotal += tmpDistanceTotal;
            DistanceToMatchedRegionsTotal += tmpDistanceToMatchedRegionsTotal;
            AverageSats += tmpAverageSats;
            AverageAcc += tmpAverageAcc;
            //Score += 10000000;        
        }
        else{
            // startIndex == 0
            NumEdgesTotal = tmpNumEdgesTotal;
            DistanceTotal = tmpDistanceTotal;
            DistanceToMatchedRegionsTotal = tmpDistanceToMatchedRegionsTotal;
            AverageSats = tmpAverageSats;
            AverageAcc = tmpAverageAcc;
            Score = 0;        
        }





    }



    void UpdateScore(int startIndex){
        //cout << "     +++++ UpdateScore called. \n";

        UpdateInternalValues(startIndex);
        double tmpScore = DistanceToMatchedRegionsTotal * 10000000.0; 
        // Use "weight" for distance to regions.
        //cout << "   ### tmpScore: " << tmpScore;

        tmpScore += NumEdgesTotal * 5.0; 
        // Use "weight" for number of edges. Connectivity is a 
        //very important critrion.
        //cout << " / " << tmpScore;
        tmpScore +=  (10.0 / ((AverageSats + 1) * 5.0)); 
        // Use "weight". The number of satellites represents 
        //part of the quality (pos).
        //cout << " / " << tmpScore;
        tmpScore +=  (AverageAcc / 1000); // Use "weight". The number of 
        //satellites represents part of the quality of the position.
        //cout << " / " << tmpScore << "\n";


        // Set or update score.
        if(startIndex == 0){
            // Set initial score (startIndex == 0).
            Score = tmpScore;
        }
        else{
            // startIndex > 0, update score.
            //cout << " Old Score: " << Score << " , tmpScore: " << tmpScore;
            Score += tmpScore;
            //cout << " -> new Score: " << Score << "\n";
        }

        //cout << " : " << Score << "\n";

    }



    bool operator<(const MatchCandidate otherMC) const
    {
        return Score < otherMC.Score;
    }

    bool operator==(const MatchCandidate otherMC) const
    {
        return Score == otherMC.Score;
    }

    bool operator<(const MatchCandidate* otherMC) const
    {
        return Score < otherMC->Score;
    }

    bool operator==(const MatchCandidate* otherMC) const
    {
        return Score == otherMC->Score;
    }


};


struct RegNode;

// Represents an edge in the Region graph.
struct RegEdge{
    int edgeId;
    double distance;
    int regNodeFromId;
    int regNodeToId;

    RegEdge(int theEdgeId)
    {
        edgeId = theEdgeId;
        distance = 0.0002;
        regNodeFromId = 0;
        regNodeToId = 0;
    }


    RegEdge(int theEdgeId, double theDistance)
    {
        edgeId = theEdgeId;
        distance = theDistance;
        regNodeFromId = 0;
        regNodeToId = 0;
    }

    RegEdge(int theEdgeId, int theRegNodeFromId, int theRegNodeToId, 
            double theDistance)
    {
        edgeId = theEdgeId;
        regNodeFromId = theRegNodeFromId;
        regNodeToId = theRegNodeToId;
        distance = theDistance;
    }

    ~RegEdge() {
    }

    bool operator<(const RegEdge& edgeB) const
    {
        return edgeId < edgeB.edgeId;
    }

    bool operator==(const RegEdge& edgeB) const
    {
        return edgeId == edgeB.edgeId;
    }

};



// Represents a node in the Region graph.
struct RegNode{
    int regId;
    set<int> edgeIdsSet;
    double distance;

    RegNode(int theRegId)
    {
        regId = theRegId;
        //distance = 0.0000002;
      
    }


    RegNode(int theRegId, set<int> edgeIdsSet)
    {
        regId = theRegId;
        edgeIdsSet = edgeIdsSet;
        //distance = 0.0000002;
    }

    ~RegNode() {
        
    }

    bool operator<(const RegNode& nodeB) const
    {
        return regId < nodeB.regId;
    }

    bool operator==(const RegNode& nodeB) const
    {
        return regId == nodeB.regId;
    }


};




struct RegGraph{
    set<RegNode*>* nodesPtr;
    set<RegEdge*>* edgesPtr;
    vector<RegNode* >* vistitedNodesVectorPtr; 

    RegGraph()
    {
        nodesPtr = new set<RegNode*>;
        edgesPtr = new set<RegEdge*>;
        vistitedNodesVectorPtr = new vector<RegNode* >;
    }

    ~RegGraph() {

        delete nodesPtr;
        delete edgesPtr;
        delete vistitedNodesVectorPtr;

    }

    void addNode(int theRegId);
    void addNode(int theRegId, set<int> theEdgesSet);

    void addEdge(int theEdgeId, int theSourceRegionId, 
                 int theTargetRegionId, double theDistance);

    RegNode* getNode(int theRegId);
    RegEdge* getEdge(int theEdgeId);

    bool FindShortestPath(int startNodeId, int endNodeId, int maxEdgesDepth, 
                            vector<RegNode* >*& nodesVectorPtr, 
                            vector<vector<RegNode* >* > pathsVector);
    bool IsNodeInPath(int nodeId, vector<RegNode* >* nodesVectorPtr);
    void CopyPathEntries(vector<RegNode* >* orgNodesVectorPtr, 
                         vector<RegNode* >* copyNodesVectorPtr);
    double PathDistanceSum(vector<RegNode* >* nodesVectorPtr);

};

void RegGraph::addNode(int theRegId){
    if(getNode(theRegId)!= 0){
        nodesPtr->insert(new RegNode(theRegId));



    }
}

void RegGraph::addNode(int theRegId, set<int> theEdgesSet){
    if(getNode(theRegId)!= 0){
        nodesPtr->insert(new RegNode(theRegId, theEdgesSet));
        //cout << "Node with " << theRegId << " added." << endl;
    }
}


void RegGraph::addEdge(int theEdgeId, int theSourceRegionId, 
                       int theTargetRegionId, double theDistance){

    RegNode* srcNode = getNode(theSourceRegionId);
    RegNode* targetNode = getNode(theTargetRegionId);

    if(getNode(theSourceRegionId) == 0){
        srcNode = new RegNode(theSourceRegionId);
        srcNode->distance = theDistance;
        nodesPtr->insert(srcNode);
        //cout << "************ distance: " << 
        //RegGraph::getNode(theSourceRegionId)->distance << endl;
    }

    if(getNode(theTargetRegionId) == 0){
        targetNode = new RegNode(theTargetRegionId);
        srcNode->distance = theDistance;
        nodesPtr->insert(targetNode);
        //targetNode = RegGraph::getNode(theTargetRegionId);
    }

    //cout << "srcNode.regId: " << srcNode.regId << endl;








    // Assign edge
    if(srcNode != 0 && (srcNode->edgeIdsSet.size() == 0 || 
        srcNode->edgeIdsSet.find(theEdgeId) == srcNode->edgeIdsSet.end())){
        srcNode->edgeIdsSet.insert(theEdgeId);



    }


    if(getEdge(theEdgeId) == 0){
        edgesPtr->insert(new RegEdge(theEdgeId, theSourceRegionId, 
                         theTargetRegionId, theDistance));
        //cout << "Edge with " << theEdgeId << " added." << endl;

        //return *it;

    }
    else{
        //cout << "Edge with " << theEdgeId << " already present." << endl;
    }

}


RegEdge* RegGraph::getEdge(int theEdgeId){

    std::set<RegEdge*>::iterator it;
    it = edgesPtr->begin();
    bool foundEdge = false;

    while( !foundEdge && it != edgesPtr->end()){
        if((*it)->edgeId == theEdgeId){
            foundEdge = true;
            //cout << "Edge with " << theEdgeId << " present. " << *it << endl;

            return *it;
        }

        ++it;
    }

    return 0;
}


RegNode* RegGraph::getNode(int theRegId){

    std::set<RegNode*>::iterator it;
    it = nodesPtr->begin();
    bool foundNode = false;

    while( !foundNode && it != nodesPtr->end()){
        if((*it)->regId == theRegId){
            foundNode = true;
            //cout << "Node with " << theRegId << " present. " << *it << endl;

            return *it;
        }

        ++it;
    }
            
    //cout << "Node with " << theRegId << " NOT present." << endl;

    return 0;

}


// Checks if a RegNode with given id is contained in the vector that is 
//supposed to represents a path.
bool RegGraph::IsNodeInPath(int nodeId, vector<RegNode* >* nodesVectorPtr){

    // Null pointer?
    if(nodesVectorPtr == 0)
        return false;

    int numOfNodesInVector = nodesVectorPtr->size();
    int index = 0;
    bool found = false;

    while(!found && index < numOfNodesInVector){

        if( (*nodesVectorPtr)[index]->regId == nodeId ){
            found = true;
        }

        index++;
    }

    return found;
    
}


// Checks if a RegNode with given id is contained in the vector that is 
//supposed to represents a path.
double RegGraph::PathDistanceSum(vector<RegNode* >* nodesVectorPtr){

    // Null pointer?
    if(nodesVectorPtr == 0)
        return 0.0;

    int numOfNodesInVector = nodesVectorPtr->size();
    int index = 0;
    double distSum = 0.0;

    while(index < numOfNodesInVector){
        distSum += (*nodesVectorPtr)[index]->distance;
        index++;
    }
    //cout << "distSum: " << distSum << endl;
    return distSum;
    
}


// Copies entries from one vector to the other.
void RegGraph::CopyPathEntries(vector<RegNode* >* orgNodesVectorPtr, 
                        vector<RegNode* >* copyNodesVectorPtr){

    //cout << "CopyPathEntries started." << endl;
    //Valid pointers to vectors?
    if(orgNodesVectorPtr == 0 || copyNodesVectorPtr == 0)
        return;

    for(unsigned int i = 0; i < orgNodesVectorPtr->size(); i++){
        copyNodesVectorPtr->push_back( (*orgNodesVectorPtr)[i] );
        //cout << "Copied pointer for node with id: " << 
        //(*orgNodesVectorPtr)[i]->regId << endl;
    }

}


// Tries to find the shortest Path from start to end node (with given ids).
bool RegGraph::FindShortestPath(int startNodeId, int endNodeId, 
                int maxEdgesDepth, vector<RegNode* >*& nodesVectorPtr, 
                vector<vector<RegNode* >* > pathsVector){

    //cout << "   #################### FindShortestPath called. \n";
    //cout << "   " << startNodeId <<  "/" << endNodeId;


    bool found = false;
    // Try to find nodes with ids.
    RegNode* startNodePtr = RegGraph::getNode(startNodeId);
    RegNode* endNodePtr = RegGraph::getNode(endNodeId);

    if(startNodePtr == 0 || endNodePtr == 0){
        //cout << "Start or end node pointer is 0." << endl;
        return false;
    }

    if(nodesVectorPtr == 0){
        // Vector pointer was null. Create vector on heap.
        nodesVectorPtr = new vector<RegNode* >;
    }
        
    if(nodesVectorPtr->size() == 0){
        // Initial call. Vector is empty.
        nodesVectorPtr->push_back(startNodePtr);

        if(startNodeId == endNodeId){
            // Start and and node are the same.
            return true;

        }
 
        // Recursive check
        found = RegGraph::FindShortestPath(startNodeId, endNodeId, 
                maxEdgesDepth, nodesVectorPtr, pathsVector);
        if(found){
            pathsVector.push_back(nodesVectorPtr);

        }




    } // end if(nodesVectorPtr->size() == 0)
    else{
        // Vector is not empty.



       // Check if depth of path in graph is shorter than allowed. If too 
       //high, don't search deeper.
       int actDepth = nodesVectorPtr->size();

       if(actDepth == maxEdgesDepth){
            if( (*nodesVectorPtr).back()->regId == endNodeId ){
                // Last node in vector is the desired end node.
                return true;
            }
            else{
                // Last node in vector is NOT the desired end node 
                //(end max search depth is reached).
                return false;
            }

        }
        else if(actDepth > maxEdgesDepth){
            // Max search depth left behind. Should not occur.
            return false;
            
        }
        else{
            // actDepth < maxEdgesDepth. The search can go on.
            // Start and end node are different (and not 0).
            
            if( (*nodesVectorPtr).back()->regId == endNodeId ){
                // Last node in vector is the desired end node.
                return true;
            }

            //vector<vector<RegNode* >* > pathsVector;
            RegNode* actBackNode = nodesVectorPtr->back();

            std::set<int>::iterator edgeIterator;
            for(edgeIterator = actBackNode->edgeIdsSet.begin(); 
                edgeIterator != actBackNode->edgeIdsSet.end(); 
                ++edgeIterator){
                //cout << "node has edge with id: " << *edgeIterator << endl;
                RegEdge* actEdge = getEdge(*edgeIterator);

                if(actEdge != 0){
                    // Check target node from current edge.
                    int actEdgeNodeToId = actEdge->regNodeToId;

                    // If node id is not null and no node with that id is in 
                    //current path, analyse more.
                    if(actEdgeNodeToId != 0 && 
                        !RegGraph::IsNodeInPath(actEdgeNodeToId, 
                                nodesVectorPtr) ){
                        // Target node for actual edge exists (not 0).
                        RegNode* actEdgeToNodePtr = 
                                RegGraph::getNode(actEdgeNodeToId);

                        if(actEdgeToNodePtr != 0){
                            // Create path copy and add the current target 
                            //node pointer as last entry.
                            vector<RegNode* >* actPathVectorPtr = 
                                            new vector<RegNode* >;
                            RegGraph::CopyPathEntries(nodesVectorPtr, 
                                                actPathVectorPtr);
                            actPathVectorPtr->push_back(actEdgeToNodePtr);
                            //cout << "Node added, id: " << 
                            //actEdgeToNodePtr->regId << endl;
                            
                            // Recursive check
                            found = RegGraph::FindShortestPath(startNodeId, 
                                    endNodeId, maxEdgesDepth, 
                                    actPathVectorPtr, pathsVector);

                            if(found){
                                pathsVector.push_back(actPathVectorPtr);

                            }

                        }

                    }

                } // end if(actEdge != 0)


            } // end  for(edgeIterator = actBackNode->edgeIdsSet.begin() ...
            double shortestPathDist = 100000000.0;
            double shortestPathDistPathIndex = -1;

            //int shortestPathNodesNumber = 0; // Distance is used, more 
            //relevant.
            //int shortestPathNodesNumberPathIndex = 0;
            //cout << "Number of different paths: " << 
            //pathsVector.size() << endl;

            // Analyse the paths from for loop (recursion results)
            for(unsigned int i = 0; i < pathsVector.size(); i++){
                double tmpPathDist =RegGraph::PathDistanceSum(pathsVector[i]);
                if(tmpPathDist < shortestPathDist){
                    // New shortest path found
                    shortestPathDist = tmpPathDist;
                    shortestPathDistPathIndex = i;
                }
            }

            //vector<RegNode* >* tmpPtr = nodesVectorPtr;
            if(shortestPathDistPathIndex != -1){
                // Shortest path determined.
                nodesVectorPtr = pathsVector[shortestPathDistPathIndex];
                found = true;


            }


        } // end else (Case: actDepth < maxEdgesDepth)

        
        
    }

    //cout << "   #################### FindShortestPath finished. \n";
    return found;
    
}




/*
LocalInfo Class ~GetNextNodeTupleLI~ (like an interator over nodes)

Refers to tuples from Nodes relation.

For a stream operator the value mapping is called multiple times. 
This class is designed to store
the current state. With getNext() a tuple can be accessed for the actual 
operation.

*/
class GetNextNodeTupleLI{

  public:
   // Constructor (initializes from argument)   
      GetNextNodeTupleLI(Relation* arg){
      tupleNr = 1;

         // Argument is not empty.
         if(arg!= 0){

            // Assign the argument to the member for the input relation:
            node_rel = arg;
//            cout << "Relation present." << "\n";

//            // How many tuples are in relation?
//            int numOfTuples = node_rel->GetNoTuples();
//            cout << "    Number of tuples (GetNextNodeTupleLI):"<< 
                //numOfTuples << "\n";

//            cout << "  *******  Tuple 1:  *******  " << "\n";

//            // Get the first tuple if it exists.
//            Tuple* tuple = 0;
//            if(numOfTuples > 0)
//                tuple = node_rel->GetTuple(1,true);
//            cout << "    with point: " << static_cast<Point*>(
                //tuple->GetAttribute(3))->toString() << "\n";

         }


      }
   // destructor
      ~GetNextNodeTupleLI(){}

   // this function returns the next result or null if the input is exhausted
      Tuple* getNext(){
//         cout << "getNext() called." << "\n";

         // If there are no more tuples in relation, return 0.
         if(tupleNr > node_rel->GetNoTuples()){
            //cout << "No more tuples present." << "\n";
            return 0;
         } 

         // Get the next tuple.
         Tuple* tmpTuple = node_rel->GetTuple(tupleNr,true);
         
         // New copy on heap that is permanently available for the operator 
         //(until removed from memory).
         Tuple* resultTuple = new Tuple(tmpTuple->GetTupleType());

         // Point from tuple
         Point* pointFromTuplePointer = new Point( *(static_cast<Point*>(
                                    tmpTuple->GetAttribute(3))));
         pointFromTuplePointer->Set(true, pointFromTuplePointer->GetX(), 
                                        pointFromTuplePointer->GetY());

         // Assign attributes
         resultTuple->PutAttribute(0, new CcInt(static_cast<CcInt*>(
                            tmpTuple->GetAttribute(0))-> GetIntval()));
         resultTuple->PutAttribute(1, new CcInt(static_cast<CcInt*>(
                            tmpTuple->GetAttribute(1))-> GetIntval()));
         resultTuple->PutAttribute(2, new CcInt(static_cast<CcInt*>(
                            tmpTuple->GetAttribute(2))-> GetIntval()));
         resultTuple->PutAttribute(3, pointFromTuplePointer);

//         cout << "Tuples number " << tupleNr << 
            //" is now the current tuple." << "\n";

         // Increase the tuple number because the tuple with current id 
            //will be returned below. Next call aims at next tuple.
         tupleNr++;
         return resultTuple;   
      }

      string* waysRelNamePtr;

  private:
      Relation* node_rel;  // input relation
      int tupleNr;      // current tuple number from relation 
};



// #######################################################################

/*
LocalInfo Class ~GetNextEdgeTupleLI~ (like an interator over edges)

Refers to tuples from Edges relation.

For a stream operator the value mapping is called multiple times. This class 
is designed to store
the current state. With getNext() a tuple can be accessed for the actual 
operation.

*/
class GetNextEdgeTupleLI{

  public:
   // Constructor (initializes from argument)   
      GetNextEdgeTupleLI(Relation* arg){
      tupleNr = 1;

         // Argument is not empty.
         if(arg!= 0){

            // Assign the argument to the member for the input relation:
            edge_rel = arg;
         }


      }
   // destructor
      ~GetNextEdgeTupleLI(){}

   // this function returns the next result or null if the input is exhausted
      Tuple* getNext(){
//         cout << "getNext() called." << "\n";

         // If there are no more tuples in relation, return 0.
         if(tupleNr > edge_rel->GetNoTuples()){
            //cout << "No more tuples present." << "\n";
            return 0;
         } 

         // Get the next tuple.
         Tuple* resultTuple = edge_rel->GetTuple(tupleNr,true);
         //cout << "Tuple present: " << *tmpTuple << "\n";
       

         // Increase the tuple number because the tuple with current id will 
         //be returned below. Next call aims at next tuple.
         tupleNr++;
         return resultTuple;   
      }

  private:
      Relation* edge_rel;  // input relation
      int tupleNr;      // current tuple number from relation 
};

// #######################################################################


/*
LocalInfo Class ~GetNextRawTupleLI~ (like an interator over raw tuples with 
trip data)

Refers to relation with raw trip data containing date, time, lat, long etc.

For a stream operator the value mapping is called multiple times. This class 
is designed to store
the current state. With getNext() a tuple can be accessed for the actual 
operation.

*/
class GetNextRawTupleLI{

  public:
   // Constructor (initializes from argument)   
      GetNextRawTupleLI(Relation* arg, string* edgesRelNamePtr, 
                        string* nodesRelNamePtr, string* nodesRTreeNamePtr, 
                        string* nodesRegIdBTreeNamePtr ){
      tupleNr = 1;
      matchPointNr = 0;

         // Argument is not empty.
         if(arg!= 0){

            // Assign the argument to the member for the input relation:
            raw_rel = arg;

         }

         graphEdgesRelNamePtr = edgesRelNamePtr;
         graphNodesRelNamePtr = nodesRelNamePtr;
         graphNodesRTreeNamePtr = nodesRTreeNamePtr;
         graphNodesRegIdBTreeNamePtr = nodesRegIdBTreeNamePtr;

         initialized = false;
         candidates.clear();

      }
   // destructor
      ~GetNextRawTupleLI(){}

   // this function returns the next result or null if the input is exhausted
      Tuple* getNext(){

         // If there are no more tuples in relation, return 0.
         if(tupleNr > raw_rel->GetNoTuples()){
            return 0;
         } 

         // Get the next tuple.
         Tuple* tmpTuple = raw_rel->GetTuple(tupleNr,true);
         Tuple* resultTuple = tmpTuple->Clone();


         tupleNr++;
         return resultTuple;   
      }



      MatchedPoint* getNextMatchedPoint(){
         MatchCandidate* bestCandidatePtr = candidates[0];
         vector<MatchedPoint*>* matchPointsPtr = 
                    bestCandidatePtr->TripCandidateVectorPtr;

         // If there are no more match points, return 0.
         if(matchPointNr >= matchPointsPtr->size()){
            return 0;
         } 

         // Get the next match point.
         MatchedPoint* tmpMatchedPointPtr = (*matchPointsPtr)[matchPointNr];


         matchPointNr++;
         return tmpMatchedPointPtr;   
      }


      string* graphEdgesRelNamePtr;
      string* graphNodesRelNamePtr;
      string* graphNodesRTreeNamePtr;
      string* graphNodesRegIdBTreeNamePtr;
      vector<MatchCandidate* > candidates;

      RegGraph* regionGraphPtr;
      Relation* raw_rel;  // input relation
      bool initialized;

  private:
      int tupleNr;      // current tuple number from relation
      unsigned int matchPointNr; // for looping through best 
                                 // candidate result of MHT 
      
};

// ######################################################################


/*
LocalInfo Class ~GetNextRegionNodeTupleLI~ (like an interator over nodes)

Refers to RegionNode relation.

For a stream operator the value mapping is called multiple times. 
This class is designed to store
the current state. With getNext() a tuple can be accessed for the actual 
operation.

*/
class GetNextRegionNodeTupleLI{

  public:
   // Constructor (initializes from argument)   
      GetNextRegionNodeTupleLI(Relation* arg){
      tupleNr = 1;

         // Argument is not empty.
         if(arg!= 0){

            // Assign the argument to the member for the input relation:
            node_rel = arg;
//            cout << "Relation present." << "\n";

//            // How many tuples are in relation?
//            int numOfTuples = node_rel->GetNoTuples();
//            cout << "    Number of tuples (GetNextRegionNodeTupleLI):"<< 
                //numOfTuples << "\n";

//            cout << "  *******  Tuple 1:  *******  " << "\n";

//            // Get the first tuple if it exists.
//            Tuple* tuple = 0;
//            if(numOfTuples > 0)
//                tuple = node_rel->GetTuple(1,true);
//            cout << "    with point: " << static_cast<Point*>(
                //tuple->GetAttribute(3))->toString() << "\n";



         }

         // Reset inner loop counter (used by calling function to work with 
            //inner query and multiple results before next tuple)
         loopCounter = 1;
         innerResultRelPtr = 0;

         // Parameters for createEdgesForRegions operator
         regionsRelNamePtr = 0;
         regionsSrcBTreeNamePtr = 0;
         regionsTargetBTreeNamePtr = 0;

         // Initialize empty (name not known here)
         rTreeNamePtr = new string("");
         endIndexPtr = new int(0);
         relNamePtr = new string("");


      }
   // destructor
      ~GetNextRegionNodeTupleLI(){}

   // this function returns the next result or null if the input is exhausted
      Tuple* getNext(){
//         cout << "getNext() called." << "\n";

         // If there are no more tuples in relation, return 0.
         if(tupleNr > node_rel->GetNoTuples()){
            //cout << "No more tuples present." << "\n";
            return 0;
         } 

         // Get the next tuple.
         Tuple* tmpTuple = node_rel->GetTuple(tupleNr,true);
         resultTuple = tmpTuple->Clone();


         // Increase the tuple number because the tuple with current id will 
         // be returned below. Next call aims at next tuple.
         tupleNr++;
         return resultTuple;   
      }

   // this function returns the actual result or null (input exhausted)
      Tuple* getActual(){
         return resultTuple;   
      }

      // Pointer to name of rtree for the input relation (pointed by node_rel)
      string* rTreeNamePtr;
      string* relNamePtr;
      int* endIndexPtr;

      int loopCounter; // Used for inner loops in calling function (query 
        //result may have multiple results to compare with actual)
      Relation* innerResultRelPtr;

      // Parameters for createEdgesForRegions operator
      string* regionsRelNamePtr;
      string* regionsSrcBTreeNamePtr;
      string* regionsTargetBTreeNamePtr;

  private:
      Relation* node_rel;  // input relation
      int tupleNr;      // current tuple number from relation 
      Tuple* resultTuple; // current tuple pointer
};



// #########################################################################









class RegionMgmt
{
  public: 
      static vector<Region> nodeRegions;
      static vector<int> nodeIdNewValues;
      static int regionId;
      static int regionEdgeId;
      static bool idLoaded;

  public:
    RegionMgmt()
    {


    }
    ~RegionMgmt() {}

    // Declarations of functions.
    static void PrintInfo();
    static int GetNextId();
    static void IncreaseNextId();
    static int LoadId();
    static void StoreId();

};




    /*
      Display info for region id and vectors.
    */
    void RegionMgmt::PrintInfo(){
        cout << "*************************************************" << "\n";
        cout << "* Next free region id: " << regionId << "\n";
        cout << "* Number of cached regions: " << nodeRegions.size() << "\n";
        cout << "*************************************************" << "\n";
    }



    /*
      Load region id from db object "mapMatchingPedRegionId". The id is the 
      next free id. 
      (Regions are created from nodes and lines connecting nodes. Each region 
      gets a unique id.)
    */
    int RegionMgmt::LoadId(){

        // Initialize return value (will be replaced, 
        // if loading was successful).
        int returnValue = 0;
        
        // Load free id for next region.
        //cout << "Trying to load id." << "\n";
        //ListExpr objectValue = SecondoSystem::GetCatalog()->
            //GetObjectValue("mapMatchingPedRegionId");
        //cout << "objectValue: " << nl->ToString(objectValue) << "\n";

        //cout << "IsAtom (objectValue): " << nl->IsAtom(objectValue) << "\n";
        //cout << "AtomType (objectValue): " << nl->NodeType2Text( 
            //nl->AtomType(objectValue) ) << "\n";

//        if(nl->IsAtom(objectValue) && nl->AtomType(objectValue) == IntType){
//            // Int value for next free region id successfully loaded.
//            //cout << "objectValue is int" << "\n";

//            try{
                // Assign to return variable
//                returnValue = (int) nl->IntValue(objectValue);

                // Set the loaded value as new value of static regionId.
//                regionId = *new int(returnValue);

                // Remember successfull load. Cached value is available.
//                idLoaded = true;
//            }
//            catch(const std::exception ex){
//                cout << "Reading mapMatchingPedRegionId failed." << "\n";
//            }

//        }
//        else
//            cout << "Region id object not an int value." << "\n";


        // Variables that will be set in GetObject function.
        Word objWord;
        bool objDefined = false;

        // Get object with region id from db. NiceFact
        //ListExpr objectValue = SecondoSystem::GetCatalog()->
        //GetObject("mapMatchingPedRegionId", objWord, objDefined);

        if(objWord.addr != 0 && objDefined){
            // Expected object exits.
            //cout << "Object for region id exists." << "\n";

            // Get int value that represents region id from CcInt with 
            // address from previous call of GetObject.
            CcInt* regionIdCcIntPointer = (CcInt*) objWord.addr;
            returnValue = regionIdCcIntPointer -> GetIntval();

            // Set the loaded value as new value of static regionId.
            regionId = *new int(returnValue);

            //Remember successfull load. Cached value is available.
            idLoaded = true;

            //cout << "Region id read from \"mapMatchingPedRegionId\" 
            // with value: " << returnValue << "\n";
        }
        else if(!objDefined){
            //cout << "Object \"mapMatchingPedRegionId\" that is expected to 
            // contain the region id is not defined." << "\n";
        }
        else{
            //cout << "Object \"mapMatchingPedRegionId\" does not 
            // exist." << "\n";
        }

        return returnValue;
    }


    /*
      Stores the value of the static regionId (next free id for a region) 
      in db object "mapMatchingPedRegionId".
    */
    void RegionMgmt::StoreId(){
        //cout << "Storing " << regionId << " ..." << "\n";

        //// Variables for GetObject
        //string typeName;
        //ListExpr typeExpr;
        //Word w;
        //bool defined;
        //bool hasTypeName;
        //SecondoSystem::GetCatalog()->GetObjectExpr("mapMatchingPedRegionId",
        // typeName, typeExpr, w, defined, hasTypeName);
        //CcInt* bVal = (CcInt*) w.addr;
        //cout << "Before storing: " << "Type name: " << typeName << ", type 
        // expression: " << nl->ToString(typeExpr) << "\n";
        //cout << "defined: " << defined << ", hasTypeName: " << 
        // hasTypeName << "\n";
        //cout << "w: " << bVal->GetIntval() << "\n";

        // Create a CcInt with regionId as int value and use pointer for 
        // word in ModifyObject / InsertObject.
        CcInt* regionIdCcIntPointer = new CcInt(new int(regionId));
        Word valuePointerWord;
        valuePointerWord.addr = regionIdCcIntPointer;

        // Check if object exists (and create if not)
        Word objWord;
        //bool objDefined;
        //ListExpr objectValue = SecondoSystem::GetCatalog()->GetObject(
        // "mapMatchingPedRegionId", objWord, objDefined);

        if(objWord.addr == 0){
            // Expected object does not exits. Create it.
            //cout << "Object for region id does not exist." << "\n";
            ListExpr tExpr = nl->OneElemList(
                nl->SymbolAtom(CcInt::BasicType()));
            SecondoSystem::GetCatalog()->InsertObject(
                "mapMatchingPedRegionId", "", tExpr, valuePointerWord, true);
            //cout << "Object for region id created with name: 
            // \"mapMatchingPedRegionId\"" << "\n";

            // Created with intended value. No modifying necessary. Return.
            return;
        }
        //cout << "Modifying \"mapMatchingPedRegionId\" ..." << "\n";
        // Modify (store static regionId value) db object that holds the 
        // next free region id as int value.
        bool modified = SecondoSystem::GetCatalog()->ModifyObject(
                "mapMatchingPedRegionId", valuePointerWord);

        if(!modified)
   cout << "Storing object \"mapMatchingPedRegionId\" failed." << "\n";

        //// Delete CcInt (was saved).
        //delete regionIdCcIntPointer;

        //SecondoSystem::GetCatalog()->CloseObject(typeExpr,valuePointerWord);
        
        //ListExpr objectValue = SecondoSystem::GetCatalog()->GetObjectValue(
                //"mapMatchingPedRegionId");
        //cout << "After storing. ObjectValue (mapMatchingPedRegionId): " << 
        // nl->ToString(objectValue) << "\n";

    }



    /*
      Gets the next free region id.
    */
    int RegionMgmt::GetNextId(){
        if(!idLoaded){
            //int loadedId = LoadId();
            //cout << "Id loaded: " << loadedId << "\n";
        }
        return regionId;
    }



    /*
      Increases the next free region id.
    */
    void RegionMgmt::IncreaseNextId(){
        if(!idLoaded){
            //int loadedId = LoadId();
            //cout << "Id loaded: " << loadedId << "\n";
        }
        regionId++;
    }

//##############################



// Initialize static members of RegionMgmt.
vector<Region> RegionMgmt::nodeRegions = *new vector<Region>;
vector<int> RegionMgmt::nodeIdNewValues = *new vector<int>;
int RegionMgmt::regionId = *new int(10547);
int RegionMgmt::regionEdgeId = *new int(10000);
bool RegionMgmt::idLoaded = false;


// **********************************************************************


//The class ~MapMatchingPAlgebra~ provides operators for map matching, 
//especially for pedestrians (walkers).
class MapMatchingPAlgebra;



//Help function that determines the dimensions of a crossing node 
//depending on way types of the ways crossing the node.
static void DetermineCrossingDimensions(std::vector<string> wayTypes, 
                        double& width, double& height, int& indexFound){
    // Check all types and determine most significant type, e.g. primary.
    // **** 1. Road types (http://wiki.openstreetmap.org/wiki/DE:Key:highway) 
    // (values for highway):
    // motorway (e.g. Autobahn)
    // trunk (like an Autobahn, but no real Autobahn)
    // Primary (main connecting roads)
    // Secondary (smaller than primary, connecting smaller towns or centers)
    // Tertiary (connecting villages or big street within a town)
    // unclassified (no line in the middle)
    // residential (access to housing)
    // service (access roads, industrial, business, car park etc.)

    // ***** 2. Link roads (values for highway):
    // motorway_link (link road leading to from motorway)
    // trunk_link (link road leading to from trunk)
    // primary_link (link road leading to from primary)
    // secondary_link (link road leading to from secondary)
    // tertiary_link (link road leading to from tertiary)

    // ***** 3. Special road types (values for highway):
    // living_street (residential streets where pedestrians have legal 
    // priority over cars)
    // pedestrian (used mainly/exclusively for pedestrians in shopping and 
    // some residential areas)
    // track (Roads for mostly agricultural or forestry uses)
    // bus_guideway (A busway where the vehicle guided by the way (though 
    // not a railway), not suitable for other traffic)
    // raceway (A course or track for (motor) racing)
    // road (temporary tag to mark a road until it has been properly surveyed)

    // ***** 4. Path types (values for highway):
    // footway (mainly/exclusively for pedestrians)
    // bridleway (For horses)
    // steps (For flights of steps (stairs) on footways)
    // path (A non-specific path)
    // cycleway (For designated cycleways)

    // ***** 5. Lifecycle types (values for highway):
    // proposed (planned road)
    // construction (For roads under construction)

    // ***** 6. Potentially interesting tags (for pedestrians) ()
    // sidewalk (sidewalk CAN be tagged (often it is not tagged). 
    // Available values: both / left / right / no)
    // motorroad (restricted to vehicles, motorway-like). Available 
    // values: yes / no

    // Footway is like some others (e.g. cycleway) regarded the 
    // least significant.
    string mostSignificantType = "footway";
    width = *new double(8.0);
    height = *new double(8.0);

    //cout << "int max: " << std::numeric_limits<int>::max() << "\n";
    //cout << "size_t max: " <<std::numeric_limits<std::size_t>::mx() << "\n";
    //cout << "long max: " << std::numeric_limits<long>::max() << "\n";
    //cout << "Region id: " << RegionMgmt::GetNextId() << "\n";
    //RegionMgmt::IncreaseNextId();
    //RegionMgmt::regionId = 1000000;
    //RegionMgmt::StoreId();


    // Overwrite with more significant type, if present in vector of actual 
    // types, e.g. in one crossing node.
    for(size_t i = 0; i < wayTypes.size(); i++){
        string lowerCaseType = string(wayTypes[i]);
        //cout << "Type present: " << lowerCaseType << "\n";
        std::transform(lowerCaseType.begin(), lowerCaseType.end(), 
                    lowerCaseType.begin(), ::tolower);

        if(lowerCaseType == "steps" && (mostSignificantType == "footway" || 
            mostSignificantType == "cycleway" || 
            mostSignificantType == "pedestrian" || 
            mostSignificantType == "path" || 
            mostSignificantType == "track")){
            mostSignificantType = string(lowerCaseType);
            indexFound = *new int(i);
        }

        if(lowerCaseType == "service" && (mostSignificantType == "footway" || 
            mostSignificantType == "cycleway" || 
            mostSignificantType == "steps" || 
            mostSignificantType == "pedestrian" || 
            mostSignificantType == "path" || 
            mostSignificantType == "track")){
            mostSignificantType = string(lowerCaseType);
            indexFound = *new int(i);
        }

        if(lowerCaseType == "residential" && (
                mostSignificantType == "footway" || 
                mostSignificantType == "cycleway" ||
                mostSignificantType == "service" || 
                mostSignificantType == "steps" || 
                mostSignificantType == "pedestrian" || 
                mostSignificantType == "path" || 
                mostSignificantType == "track")){
            mostSignificantType = string(lowerCaseType);
            indexFound = *new int(i);
        }

        if(lowerCaseType == "unclassified" && (
            mostSignificantType == "footway" || 
            mostSignificantType == "cycleway" ||
            mostSignificantType == "service" || 
            mostSignificantType == "residential" || 
            mostSignificantType == "steps" || 
            mostSignificantType == "pedestrian" || 
            mostSignificantType == "path" || 
            mostSignificantType == "track")){
            mostSignificantType = string(lowerCaseType);
            indexFound = *new int(i);
        }

        // Comparing with more sidnificant before assigning from 
        // now on (shorter list).
        if(lowerCaseType == "tertiary" && 
            mostSignificantType != "motorway" && 
            mostSignificantType != "trunk" &&
            mostSignificantType != "primary" && 
            mostSignificantType != "secondary"){
            mostSignificantType = string(lowerCaseType);
            indexFound = *new int(i);
        }

        if(lowerCaseType == "secondary" && 
            mostSignificantType != "motorway" && 
            mostSignificantType != "trunk" &&
            mostSignificantType != "primary" ){
            mostSignificantType = string(lowerCaseType);
            indexFound = *new int(i);
        }

        if(lowerCaseType == "primary" && 
            mostSignificantType != "motorway" && 
            mostSignificantType != "trunk"){
            mostSignificantType = string(lowerCaseType);
            indexFound = *new int(i);
        }

        if(lowerCaseType == "trunk" && mostSignificantType != "motorway"){
            mostSignificantType = string(lowerCaseType);
            indexFound = *new int(i);
        }

        if(lowerCaseType == "motorway"){
            mostSignificantType = string(lowerCaseType);
            indexFound = *new int(i);
        }
    }

    //cout << "Most significent type: " << mostSignificantType << "\n";

    // Set the dimensions according to the most significent road type 
    // from the given vector (refers to a crossing or similar).
    if(mostSignificantType == "motorway"){
        width = *new double(15.0);
        height = *new double(15.0);
    }
    else if(mostSignificantType == "trunk"){
        width = *new double(8.0);
        height = *new double(8.0);
    }
    else if(mostSignificantType == "primary"){
        width = *new double(9.0);
        height = *new double(9.0);
    }
    else if(mostSignificantType == "secondary"){
        width = *new double(9.0);
        height = *new double(9.0);
    }
    else if(mostSignificantType == "tertiary"){
        width = *new double(8.0);
        height = *new double(8.0);
    }
    else if(mostSignificantType == "unclassified"){
        width = *new double(5.0);
        height = *new double(5.0);
    }
    else if(mostSignificantType == "residential" || 
            mostSignificantType == "living_street"){
        width = *new double(4.0);
        height = *new double(4.0);
    }
    else if(mostSignificantType == "service"){
        width = *new double(3.0);
        height = *new double(3.0);
    }
    else if(mostSignificantType == "footway" || 
            mostSignificantType == "cycleway" || 
            mostSignificantType == "steps" || 
            mostSignificantType == "pedestrian" || 
            mostSignificantType == "path" || 
            mostSignificantType == "track"){
        width = *new double(2.5);
        height = *new double(2.5);
    }
    else{
        width = *new double(6.0);
        height = *new double(6.0);
    }
   
    
}



//   Help function,
//   Determines width of way region.
static void DetermineWayWidth(const string wayType, double& width){
    // Check type and determine width for that type. (For type descriptions 
    // also see function DetermineCrossingDimensions)

    width = double(7.5);
    string lowerCaseType = string(wayType);
    //cout << "Type present: " << lowerCaseType << "\n";
    std::transform(lowerCaseType.begin(), lowerCaseType.end(), 
                    lowerCaseType.begin(), ::tolower);

    // Set the dimensions according to the most significent road type 
    //from the given vector (refers to a crossing or similar).
    if(lowerCaseType == "motorway"){
        width = *new double(7.0);
    }
    else if(lowerCaseType == "trunk"){
        width = *new double(4.0);
    }
    else if(lowerCaseType == "primary"){
        width = *new double(4.5);
    }
    else if(lowerCaseType == "secondary"){
        width = *new double(4.5);
    }
    else if(lowerCaseType == "tertiary"){
        width = *new double(4.0);
    }
    else if(lowerCaseType == "unclassified"){
        width = *new double(2.5);
    }
    else if(lowerCaseType == "residential" || 
            lowerCaseType == "living_street"){
        width = *new double(2.0);
    }
    else if(lowerCaseType == "service"){
        width = *new double(1.5);
    }
    else if(lowerCaseType == "footway" || 
            lowerCaseType == "cycleway" || 
            lowerCaseType == "steps"|| 
            lowerCaseType == "pedestrian" ||
            lowerCaseType == "path" || lowerCaseType == "track"){
        width = *new double(1.3);
    }
    else{
        width = *new double(3.0);
    }
    


}



//Help function that finds way types and names for a NodeIdNew from a 
//node that represents 
//a crossing. That means the node belongs to two or more ways.
static void GetWayTypesAndNamesForNodeIdNew(const int nodeIdNew, 
    std::vector<string>& wayTypes, std::vector<string>& wayNames, 
                                            const string* waysRelNamePtr){

    // Some info on used relations and data:
    // Ways or SedentalerWays: (WayId int) (NodeList Relation) 
    // (Curve line) (WayIdInTag int) (WayInfo Relation)
    // NodeList: (NodeId int) (Pos Point) (NodeIdNew int) 
    // (NodeCounter int) (NodeRef int)
    // WayInfo: (WayTagKey string) (WayTagValue string), e.g: WayTagKey, 
    // WayTagValue: highway, tertiary; name, Sedentaler Straße
    // Example xing node: WayId = 31113642, NodeIdNew = 7285477

    // The query string that will be executed:
//    string inputQueryString = string("query Ways feed head[2] 
//    unnest[WayInfo] ") + "extend[WayTag: tostring(.WayTagKey + \"#\" +
//    .WayTagValue)] project[WayId, WayTag] consume";

    string nodeIdNewAsString;
    ostringstream temp;
    temp.clear();
    temp << nodeIdNew;
    nodeIdNewAsString = temp.str();
    // waysRelNamePtr references for example "WaysHochdahl"
    string inputQueryString = string("query " + *waysRelNamePtr + 
        " feed filter[.NodeList afeed filter[.NodeIdNew = ") + 
        nodeIdNewAsString + "] count > 0] unnest[WayInfo] extend[WayTag: " +
"tostring(.WayTagKey + \"#\" + .WayTagValue)] project[WayId, WayTag] consume";

    //cout << "inputQueryString: " << inputQueryString << "\\n";

    // The list expression as string for the string from above will be 
    // stored in:
    string listExprForQueryString;
    //cout << "Original query text: " << inputQueryString << "\n";

    // Parse string that is valid as input in command line to list expression
    // string.
    SecParser parser;
    int parseResult = parser.Text2List(inputQueryString, 
            listExprForQueryString);

    // Result of query will be available with:
    Word queryResult;

    // parseResult: 0 = success, 1 = error, 2 = stack overflow
    if(parseResult == 0) 
    {
        int parsedStringLength = listExprForQueryString.length();

        // Cut for use in executeQuery()
        listExprForQueryString = listExprForQueryString.substr(7, 
                parsedStringLength - 9);
        //cout << "Nested list, parsed and cut for use : " << 
                //listExprForQueryString << "\n";

        //cout << "Working (local)..." << "\n";
        // Use query processor to get the way type for the way id.
        QueryProcessor::ExecuteQuery(listExprForQueryString, queryResult);
       
//        cout << "The number of elements in NodesHochdahl is: " << 
            //((CcInt*) queryResult.addr)->GetIntval() << "\n";
    }
    else if(parseResult == 1){
        //cout << "*** An error occurred while parsing a query string. 
        // Nested list not created. ***" << "\n";
        return;
    }
    else if(parseResult == 2){
        //cout << "*** A stack overflow occurred while parsing a query 
        // string. Nested list not created. ***" << "\n";
        return;
    }

    //cout << "... finished (local)."<< "\n";
    // The relation is the result from the query (and contains tuples with 
    // the NodeNewId and WayTags)
    try{
        if(queryResult.addr == 0){
            //cout << "*** No result for internal query. 
            // Does \"WaysHochdahl\" exist?" << "\n";
            return;
        }

        Relation* resultRelation = (Relation*) queryResult.addr;
        int numOfTuples = resultRelation->GetNoTuples();
        //cout << "    Number of tuples:"<< numOfTuples << "\n";

        // Access and check all tuples of the relation that is the 
        // result of the query (built from string above).
        for(int i=1; i<= numOfTuples; i++ ){
            //cout << "  *******  Tuple " <<i <<":  *******  " << "\n";
            Tuple* tuple = resultRelation->GetTuple(i,true);

            int numOfAttributes = tuple->GetNoAttributes();
            //cout << "    Number of attributes in current tuple:"<< 
            // numOfAttributes << "\n";

            // First attribute (index 0) is the WayId.
            CcInt* wayIdPointer = static_cast<CcInt*>(tuple->GetAttribute(0));
            if(wayIdPointer == 0){};
            //cout << "    WayId: " << wayIdPointer->GetValue() << "\n";

            // The other attributes are the WayTags 
            // (strings consisting of <WayTagKey>#<WayTagValue>, 
            // e.g. highway#secondary).
            for(int k = 1; k < numOfAttributes; k++){
                CcString* wayTagPointer = 
                    static_cast<CcString*>(tuple->GetAttribute(k));
                string wayTagString = wayTagPointer->GetValue();
                //cout << "    WayTag: " << wayTagString << "\n";

                // Get lowercase string as it is unclear if highway could
                // be Highway or HIGHWAY or mix of upper and lower cases.
                string lowerCaseWayTagString = string(wayTagString);
                std::transform(wayTagString.begin(), wayTagString.end(), 
                    lowerCaseWayTagString.begin(), ::tolower);
                //cout << "    lowerCaseWayTagString: " << 
                // lowerCaseWayTagString << "\n";

                // If the current lowerCaseWayTagString contains "highway" 
                // (before "#") the type should be present at the end.
                if(lowerCaseWayTagString.find("highway") != 
                            std::string::npos){
                    // wayTypes
                    size_t hashtagPos = wayTagString.find("#");
                    if(hashtagPos < wayTagString.length()){
                        // Found hashtag in string, not at the end. 
                        // Add to vector.
                        string actualWayTypeString = 
                                wayTagString.substr(hashtagPos + 1);
                       wayTypes.push_back(*(new string(actualWayTypeString)));
                        //cout << "Way type found: " << actualWayTypeString 
                        // << ", WayId: " << wayIdPointer->GetValue() << "\n";
                    }
                    else{
                        // No type found in string. Add "unknown" to vector.
                        wayTypes.push_back(*(new string("unknown")));
                        //cout << "Way type not found, used instead: " << 
                        // "unknown" << ", for WayId: " << 
            //    wayIdPointer->GetValue() << "\n";
                    }
                }
                else if(lowerCaseWayTagString.find("name") != 
                            std::string::npos){
                    // wayNames
                    size_t hashtagPos = wayTagString.find("#");
                    if(hashtagPos < wayTagString.length()){
                        // Found hashtag in string, not at the end. 
                        // Add to vector.
                        string actualWayNameString = 
                            wayTagString.substr(hashtagPos + 1);
                        wayNames.push_back(
                                    *(new string(actualWayNameString)));
                        //cout << "Way name found: " << 
                        //actualWayNameString << ", WayId: " << 
                        // wayIdPointer->GetValue() << "\n";
                    }
                    else{
                        // No name found in string. Add "unknown" to vector.
                        wayNames.push_back(*(new string("unknown")));
                        //cout << "Way name not found, used instead: " << 
                        // "unknown" << ", for WayId: " << 
            //    wayIdPointer->GetValue() << "\n";
                    }
                }

            }

        }

    }
    catch(const std::exception ex){
        //cout << "Result to relation conversion failed." << "\n";
    }

//    ((Relation*) queryResult.addr)->DeleteIfAllowed();

}



/*

Help function that calculates a surrounding region for a given point. 
The point is supposed to have geographical
coordinates (LAT LON). The other point and halfsegment parameters are 
references and will be changed. At the end
the caller can access the region, the points at the corners and the 
halfsegments that are the borders of the region.
The last two parameters are supposed to receive width and height of the 
region that will surround the first point
parameter. The point will be at the center of the region.

// pOrg  = the original point fow which the surrounding rectangle is 
calculated. Point has LON LAT coordinates.
// pUpLeft = calculated upper left point result (pointer reference)
// pUpRight, pDownRight, pDownLeft = analogue to pUpLeft, results
// upperHalfSeg = calculated upper half segment of surrounding rectangle 
from pUpLeft to pUpRight (pointer reference)
// rightHalfSeg, lowerHalfSeg, leftHalfSeg = analogue to upperHalfSeg, results
// resultRegion = calculated region specified by surrounding halfSegments 
upperHalfSeg, rightHalfSeg, lowerHalfSeg, leftHalfSeg (pointer reference), 
result
// width = the width of the surrounding rectangle in meters
// height = the height of the surrounding rectangle in meters
*/
static void calcSurroundingRegion(const Point& pOrg, Point*& pUpLeft, 
        Point*& pUpRight, Point*& pDownRight, Point*& pDownLeft, 
        HalfSegment*& upperHalfSeg, HalfSegment*& rightHalfSeg, 
        HalfSegment*& lowerHalfSeg, HalfSegment*& leftHalfSeg, 
        Region*& resultRegion, double width, double height ) {
        // Width and height must not be 0.
        if(width == 0.0){
            width = 8.0;
        }

        if(height == 0.0){
            height = 8.0;
        }

    // [m]
    double r = double(6371000.785);

        // Get coordinates
        double xOrg = (double) pOrg.GetX();
        double yOrg = (double) pOrg.GetY();

        // Absolute values
//    double* xR1 = new double(*xOrg * M_PI / 180);              // not used
    double yR1 = ((double) (yOrg * M_PI / 180));
        
    // Meters per degree. Delta of Lat(Breitengrad) is equivalent to 
    // delta y, delta Lon (Längengrad) to delta x
    double meterPerDegLat = ((double)(M_PI * r / 180));
    double meterPerDegLong = ((double)((M_PI * r / 180) * cos(yR1)));

        // Coordinate calculation of points in corners, starting with 
        // point in upper left corner.
    double upLeftX = ((double)(xOrg - (width / 2) / meterPerDegLong));
    double upLeftY = ((double)(yOrg + (height / 2) / meterPerDegLat)); 
        // Latitude increases towards north pole (90deg).

    double upRightX = ((double)(xOrg + (width / 2) / meterPerDegLong));
    double upRightY = ((double)(yOrg + (height / 2) / meterPerDegLat));

        // New points on heap with calculated coordinates
        pUpLeft = new Point(true, *(new double(upLeftX)), 
                *(new double(upLeftY)));
        pUpRight = new Point(true, *(new double(upRightX)), 
                *(new double(upRightY)));

    double downLeftX = ((double)(xOrg - (width / 2) / meterPerDegLong));
    double downLeftY = ((double)(yOrg - (height / 2) / meterPerDegLat)); 
        // Latitude increases towards north pole (90deg).

    double downRightX = ((double)(xOrg + (width / 2) / meterPerDegLong));
    double downRightY = ((double)(yOrg - (height / 2) / meterPerDegLat));

        pDownRight = new Point(true, *(new double(downRightX)), 
                *(new double(downRightY)));
        pDownLeft = new Point(true, *(new double(downLeftX)), 
                *(new double(downLeftY)));

        vector<vector<Point> > cyclesVector = 
                    *new vector<std::vector<Point> >;
        vector<Point> pointsVector = *new vector<Point>;

        pointsVector.push_back(*pUpLeft);
        pointsVector.push_back(*pUpRight);
        pointsVector.push_back(*pDownRight);
        pointsVector.push_back(*pDownLeft);
        pointsVector.push_back(*pUpLeft);

        cyclesVector.push_back(pointsVector);

        resultRegion = buildRegion(cyclesVector);
        //cout << "Region built with buildRegion function: " << 
            // *resultRegion << "\n";

        // Finished. Region is available for caller 
        // (also points and half segments).

}



// ********************************************************************





/*
   Help function. Calculates a rectangle for corredinates of two points 
   and a width.
// x1, y1 = x, y coordinate of point 1
// x2, y2 = x, y coordinate of point 2
// moveDist = move orthogonal distance of line in meters
// movedX1, movedY1 = x, y coordinate of moved point 1 (refernce 
parameter will be modified)
// movedX2, movedY2 = x, y cooridnate of moved point 1 (refernce 
parameter will be modified)
*/
static void calcRightRect(double x1, double y1, double x2, double y2, 
        double moveDist, double &movedX1, double &movedY1, double &movedX2, 
        double &movedY2) {
    // [m]
    double r = 6371000.785;


//    double xR1 = x1 * M_PI / 180;              // not used
    double yR1 = y1 * M_PI / 180;

//    double xR2 = x2 * M_PI / 180;              // not used
//    double yR2 = y2 * M_PI / 180;              // not used

    // Meters per degree (Difference of LONG is on degree of latitude. 
    // Difference in meters per degree LON depends on LAT value)
    // Delta of Lat(Breitengrad) is equivalent to delta y, delta Lon 
    // (Längengrad) to delta x
    double meterPerDegLat = ((double)(M_PI * r / 180));
    double meterPerDegLong = ((double)((M_PI * r / 180) * cos(yR1)));

        // meterPerDegLong must not be null because it is used as divisor.
        if(meterPerDegLong == 0)
            meterPerDegLong = 0.0000000001;

    // Degrees match different meter values lat / long. Calculate meter 
    // values for positions.
    double xInM1 = x1 * meterPerDegLong;
    double yInM1 = y1 * meterPerDegLat;

    double xInM2 = x2 * meterPerDegLong;
    double yInM2 = y2 * meterPerDegLat;

    // direction vector
    double dVectorX = xInM2 - xInM1;
    double dVectorY = yInM2 - yInM1;

    // Normal vector (90 degree to direction, to the right)
    double nVectorX = dVectorY;
    double nVectorY = -dVectorX;

    double nVectorInMeterX = nVectorX;
    double nVectorInMeterY = nVectorY;

    // Theorem of Pythagoras for calculating scaling factor
    double scalingFactor = sqrt(pow(nVectorInMeterX, 2) + 
                            pow(nVectorInMeterY, 2));

        // scalingFactor must not be null as divisor
        if(scalingFactor == 0)
            scalingFactor = 0.0000000001;

    double scaledNVectorX = nVectorX / scalingFactor;
    double scaledNVectorY = nVectorY / scalingFactor;

        movedX1 = x1 + (moveDist * scaledNVectorX / meterPerDegLong);
    movedY1 = y1 + (moveDist * scaledNVectorY / meterPerDegLat);

    movedX2 = x2 + (moveDist * scaledNVectorX / meterPerDegLong);
    movedY2 = y2 + (moveDist * scaledNVectorY / meterPerDegLat);

    //cout << "calc: org 1: (" << x1 << " " << y1 << ")" << "\n";
    //cout << "calc: org 2: (" << x2 << " " << y2 << ")" << "\n";
    //cout << "calc: 1 moved" << moveDist << " m: (" << movedX1 << " " << 
    // movedY1 << ")" << "\n";
    //cout << "calc: 2 moved" << moveDist << " m: (" << movedX2 << " " << 
    // movedY2 << ")" << "\n";

//    double resolutionLat = 0.0000001 * meterPerDegLat;   // not used
    //Debug.WriteLine("10e-7 degree (lat) = 0.0000001 degree matches " + 
    // resolutionLat + " m.");
    //Debug.WriteLine("10e-7 degreee (lat) = 0.0000001 degree matches " + 
    // resolutionLat * 100 + " cm.");

//    double resolutionLong = 0.0000001 * meterPerDegLong;  // not used
    //Debug.WriteLine("10e-7 degreee (long) = 0.0000001 degree matches " + 
    // resolutionLong + " m.");
    //Debug.WriteLine("10e-7 degreee (long) = 0.0000001 degree matches " + 
    // resolutionLong * 100 + " cm.");

}






/*
   Help function. Calculates a point on a straight defined by 2 points and 
   a length that will be added in direction on line to
   the 2nd point to calculate a new point. A new longer line will be given 
   with same direction and both old points on it.
// x1, y1 = x, y coordinate of point 1
// x2, y2 = x, y coordinate of point 2
// moveDist = move distance for new point starting at 2nd point in direction 
of line in meters
// movedX, movedY = x, y coordinate of moved point (refernce parameter will 
be modified)
*/
static void calcLongerLinePoint(double x1, double y1, double x2, double y2, 
                        double moveDist, double &movedX, double &movedY) {
    // [m]
    double r = 6371000.785;

    double yR1 = y1 * M_PI / 180;

    // Meters per degree (Difference of LONG is on degree of latitude. 
    // Difference in meters per degree LON depends on LAT value)
    // Delta of Lat(Breitengrad) is equivalent to delta y, delta Lon 
    // (Längengrad) to delta x
    double meterPerDegLat = ((double)(M_PI * r / 180));
    double meterPerDegLong = ((double)((M_PI * r / 180) * cos(yR1)));

        // meterPerDegLong must not be null because it is used as divisor.
        if(meterPerDegLong == 0)
            meterPerDegLong = 0.0000000001;

    // Degrees match different meter values lat / long. Calculate meter 
    // values for positions.
    double xInM1 = x1 * meterPerDegLong;
    double yInM1 = y1 * meterPerDegLat;

    double xInM2 = x2 * meterPerDegLong;
    double yInM2 = y2 * meterPerDegLat;

    // direction vector
    double dVectorX = xInM2 - xInM1;
    double dVectorY = yInM2 - yInM1;

    // Theorem of Pythagoras for calculating scaling factor
    double scalingFactor = sqrt(pow(dVectorX, 2) + pow(dVectorY, 2));

        // scalingFactor must not be null as divisor
        if(scalingFactor == 0)
            scalingFactor = 0.0000000001;

    double scaledDVectorX = dVectorX / scalingFactor;
    double scaledDVectorY = dVectorY / scalingFactor;

        movedX = x2 + (moveDist * scaledDVectorX / meterPerDegLong);
    movedY = y2 + (moveDist * scaledDVectorY / meterPerDegLat);

    //cout << "calc longer line: org 1: (" << x1 << " " << y1 << ")" << "\n";
    //cout << "calc longer line: org 2: (" << x2 << " " << y2 << ")" << "\n";
    //cout << "calc longer line: 1 moved" << moveDist << " m: (" << 
    // movedX1 << " " << movedY1 << ")" << "\n";

}






// ************************************************************************



/*
  Help function that finds the second point for a dominant point in list 
  of half segments from a simple line.
  actPoint = Pointer to Point that should be dom point in a half segment 
  from the simple line
  sLine = the line pointer for which the sec point is searched in half 
  segments
  resultP = Pointer to found sec point in half segment with dom point to 
  which actPoint points
  leftDom = IsLeftDomPoint usage in search or the opposite
  callNr = If there are multiple matches, the callNr specifies which match 
  is wanted.
  
*/
static void findSecPointInHalfSegmentsOfSLine(const Point* actPoint, 
        const SimpleLine* sLine, Point*& resultP, const bool leftDom, 
        const int callNr){

    //cout << " # # # findSecPointInHalfSegmentsOfSLine, actPoint: " << 
    // *actPoint << "\n";
    //cout << " # # # callNr: " << callNr << "\n";

    // Indicates that the point was found, ends while loop
    bool found = false;

    // indicates the nr of the matching dom point (if there are more than 
    // one halfsegments with that dom point)
    int matchNr = 1;
    
    // index variable for traversing half segments of simple line.
    int hsIndex = 0;

    // Search sec point for dom point (occurrence # callNr)
    while(!found && hsIndex < sLine->Size()){
        HalfSegment* actualHsA = new HalfSegment(true, 
                *new Point(true, 0, 0), *new Point(true, 11, 11));
        sLine->Get(hsIndex, *actualHsA);

        //cout << " # Checking halfsegment:" << '\n' << *actualHsA << "\n";

        if(actualHsA->IsLeftDomPoint() == leftDom){
            Point actDomP = actualHsA->GetDomPoint();
            Point actSecP = actualHsA->GetSecPoint();

            //cout << " # # # leftDomPoint in halfsegment, actDomP: " << 
            // actDomP << "\n";
            //cout << " # # # leftDomPoint in halfsegment, actSecP: " << 
            // actSecP << "\n";

            if(actDomP.GetX() == actPoint->GetX() && 
                actDomP.GetY() == actPoint->GetY()){
                if(matchNr == callNr){

                    //cout << " # # # found as secPoint." << "\n";

                    found = true;
                    resultP = new Point(true, actSecP.GetX(), actSecP.GetY());
                    return;
                }
                else
                    matchNr++;
            }
            
        }
        
        hsIndex++;
    }

    // If not found, use actPoint as pointer to a sec point and try to 
    // find a matching dom point.
    hsIndex = 0;
    while(!found && hsIndex < sLine->Size()){
        HalfSegment* actualHsA = new HalfSegment(true, 
                *new Point(true, 0, 0), *new Point(true, 11, 11));
        sLine->Get(hsIndex, *actualHsA);

        if(actualHsA->IsLeftDomPoint() == leftDom){
            Point actDomP = actualHsA->GetDomPoint();
            Point actSecP = actualHsA->GetSecPoint();

            //cout << " # # # leftDomPoint in halfsegment, actDomP: " << 
            // actDomP << "\n";
            //cout << " # # # leftDomPoint in halfsegment, actSecP: " << 
            // actSecP << "\n";

            if(actSecP.GetX() == actPoint->GetX() && 
                actSecP.GetY() == actPoint->GetY()){
                if(matchNr == callNr){

                    //cout << " # # # found as domPoint." << "\n";

                    found = true;
                    resultP = new Point(true, actDomP.GetX(), actDomP.GetY());
                    return;
                }
                else
                    matchNr++;
            }
            
        }
        
        hsIndex++;
    }

    // Result point pointer is 0 if still not found (actPoint points to no 
    // dom or sec point for halfsegments of sline).    
    if(!found)
        resultP = 0;

}



/*
  Help function that checks if the source and target ids indicate that the 
  corresponding region nodes should be connected
  by an edge.
  int srcIdA, int targetIdA belong to RegionNodeA
  int srcIdB, int targetIdB belong to RegionNodeB
*/
static bool IsEdgeAdequate(int srcIdA, int targetIdA, int srcIdB, 
                            int targetIdB){
    if(srcIdA == targetIdA){
        // RegionNodeA belongs to a Node (from rel Nodes, highways)

        if(srcIdB != targetIdB){
            // RegionNodeB belongs to an Edge (from rel Nodes, highways). 
            // Check if connected.

            if(srcIdA == srcIdB){
                // Case 1: B is edge starting in A.
                return true;
            }

            if(srcIdA == targetIdB){
                // Case 2: B is edge ending in A.
                return true;
            }
            else{
                // Node (A) not connected to edge (B).
                return false;
            }

        }
        else{
            // Two Nodes from original Nodes relation. Should not be 
            // connected directly.
            return false;
        }
    } // end if(srcIdA == targetIdA)
    else{
        // RegionNodeA belongs to an Edge (from rel Edges, highways). 
        // Check if connected.

        if(srcIdB == targetIdB){
            // RegionNodeB belongs to a Node (from rel Nodes, highways)

            if(srcIdB == srcIdA){
                // Case 1.2 (back): A is edge starting in B.
                return true;
            }

            if(srcIdB == targetIdA){
                // Case 2.2 (back): A is edge ending in B.
                return true;
            }
            else{
                // Node (B) not connected to edge (A).
                return false;
            }

        }

        if(srcIdB != targetIdB){
            // RegionNodeB belongs to an Edge (from rel Edges, highways). 
            // Check if connected.

            if(srcIdA == targetIdB && srcIdB == targetIdA){
                // Case 3: A and B are edges (from Edges rel, highways) 
                // in opposite directions.
                return true;
            }
            else{
                // Not the neighbour edge in opposite direction.
                return false;
            }

        }

    }

    // If this point is reached, no new edge is identified.
    return false;

}



/*
  Help function returns true if the given vector contains a point with 
  same coordinates as the given point.
  Also sets the id of the point in vector if found, -1 if not found.
*/
static bool pointPresentInPointVector(const vector<Point>* orgPoints, 
                            const Point* nextPointPtr, int& indexFound){

    if(nextPointPtr != 0 && orgPoints != 0){
        for(unsigned int vecIdx = 0; vecIdx < orgPoints-> size(); vecIdx++){
            if((*orgPoints)[vecIdx].GetX() == nextPointPtr->GetX() && 
               (*orgPoints)[vecIdx].GetY() == nextPointPtr->GetY()){
                indexFound = *new int(vecIdx);
                return true;
            }
        }
    }

    indexFound = *new int(-1);
    return false;
}




/*
  Help function updates a point in vector with other the given point.
*/
 /*
static void pointUpdateInPointVector(vector<Point>*& orgPoints, 
                Point*& presentPointPtr, const Point* newPointPtr){
    int vecIdx = *new int(0);

    // Get id of point in vector. (-1 if not found)
    pointPresentInPointVector(orgPoints, presentPointPtr, vecIdx);

    // Update if found.
    if(vecIdx >= 0)
        (*orgPoints)[vecIdx] = *newPointPtr;
    else{
        //cout << "No update in point vector. Old point not present." << "\n";
    }

}
 */


/*
Help function:
Calculate a simple preview (parallel rectangle sides for 2 existing org 
points) of the calculated line.
orgPoints = Pointer to vector of original points of a simple line.
calcPreviewPoints = Pointer to vector where the calculated points will be 
added (rectangle side for two existing org points)
width = Width of the rectangles for each two points (actually the width of 
the way and the calculated region).
*/
static void CalculatePreviewPoints(const vector<Point>* orgPoints, 
                vector<Point>*& calcPreviewPoints, const double width){

    //cout << "Calculating previw points with width: " << width << "\n";
    // If null pointer, create new vector
    if(calcPreviewPoints == 0)
        calcPreviewPoints = new vector<Point>;

    // Clear the vector.
    calcPreviewPoints->clear();
    
    for(unsigned int i = 0; i < (orgPoints->size() - 1); i++){
        Point* actualPoint = new Point((*orgPoints)[i]);
//        Point* secondPoint = new Point(true, 0, 0);
        Point* secondPoint = new Point((*orgPoints)[i+1]);

        // Get coordinates
        double xActual = (double) actualPoint->GetX();
        double yActual = (double) actualPoint->GetY();

        double xSecond = (double) secondPoint->GetX();
        double ySecond = (double) secondPoint->GetY();

        // Variables for the calculated right side of the rectangle.
        double movedX1;
        double movedY1;
        double movedX2;
        double movedY2;

        // Calculate rectangle (to the right side) with the two points.
        calcRightRect(xActual, yActual, xSecond, ySecond, width, movedX1, 
                        movedY1, movedX2, movedY2);

        // New points on heap with calculated coordinates
        Point* rightPointCalcPtr = new Point(true, *(new double(movedX1)), 
                                            *(new double(movedY1)));

        // Second point (needed in next loop cycle)
        Point* secondPointCalcPtr = new Point(true, *(new double(movedX2)), 
                                    *(new double(movedY2)));

        calcPreviewPoints->push_back(*rightPointCalcPtr);
        calcPreviewPoints->push_back(*secondPointCalcPtr);
    }


}



/*
Help function:
Checks if new points form a line that crosses old lines from simple preview 
line vector (also see CalculatePreviewPoints).
Updates the calculated points and removes overlapping parts by calculating 
last intersection and removing points "in between".
i = Position in current check in caller
firstPointPtr = First point in current check in caller
secondPointPtr = Second point in current check in caller
oldI = will return position of intersecting old line to caller (if found)
crossPointPtr = Pointer to crossing point will be returned to 
caller (if found)
calcPreviewPoints = Previously calculated points as preview (simple 
rectangle sides for org points)
tmpBackwardPoints = Pointer to vector with calculated points for region 
calculation (for complete line), may be updated.
*/
static bool CheckIfCalcLineCrossesOldCalcsAndUpdate(const double i, 
        const Point* firstPointPtr, const Point* secondPointPtr, int& oldI,
        Point*& crossPointPtr, const vector<Point>* calcPreviewPoints, 
        vector<Point>* tmpBackwardPoints){

    //cout << "+++   Check crossing for section nr: " << i << " P1: " << 
    //*firstPointPtr << " , P2: " << *secondPointPtr << "\n";

    if(firstPointPtr  == 0 || secondPointPtr == 0 || 
        tmpBackwardPoints == 0 || calcPreviewPoints ==0){
        // Invalid parameters, NULL
        //cout << "NULL pointer as parameter found." << "\n";
        return false;
    }

    if(firstPointPtr == secondPointPtr || 
    ( firstPointPtr->GetX() == secondPointPtr->GetX() && 
       firstPointPtr->GetY() == secondPointPtr->GetY()) || 
       AlmostEqual(*firstPointPtr, *secondPointPtr) ){
        // Invalid parameters, same points for half segment.
        //cout << "Equal points as parameter found." << "\n";
        return false;
    }

    bool updated = false;
    HalfSegment* actualHs = new HalfSegment(true, *firstPointPtr, 
                            *secondPointPtr);


    int lastRemovePointIndex = -1;
    //Point* crossingPointPtr = 0;

    for(unsigned int index = 0; 
        (index < (tmpBackwardPoints->size() - 1) ) && (index < i); index++ ){

        if( ( ((*tmpBackwardPoints)[index]).GetX() != 
                ((*tmpBackwardPoints)[index+1]).GetX() ||
              ((*tmpBackwardPoints)[index]).GetY() != 
                    ((*tmpBackwardPoints)[index+1]).GetY() ) &&
              (*tmpBackwardPoints)[index] != (*tmpBackwardPoints)[index+1] && 
              !AlmostEqual((*tmpBackwardPoints)[index], 
                (*tmpBackwardPoints)[index+1]) ) {

          
            HalfSegment* prevHs = new HalfSegment(true, 
                (*tmpBackwardPoints)[index], (*tmpBackwardPoints)[index+1]);

            if( !AlmostEqual(*prevHs, *actualHs) ){

                // Crossing point determination
                if(prevHs->Intersects(*actualHs, 0)){
                    crossPointPtr = new Point(true, 0 , 0);
                    prevHs->Intersection(*actualHs, *crossPointPtr, 0);

                    // Make index for found intersection available for caller.
                    oldI = *new int(index);

                    //last point to be removed
                    lastRemovePointIndex = index + 1;


                    // End for loop
                    break;
                    index = 1000000;

                } // end if(prevHs->Intersects(*actualHs, 0))

            } // end if( !AlmostEqual
            else{
                //cout << "Almost equal half segments present. \n";
            }

        } // end if( ((*tmpBackwardPoints)[index]).GetX() != ...
        else{
            //cout << "Almost equal points in tmp vector. \n";
        }

    } // end for(int index = 0 ...

    if(lastRemovePointIndex > -1){
        // Remove inner points in cycle from vector with calculated points 
        // for region (tmpBackwardPoints).
        while( tmpBackwardPoints->size() > 
                    (unsigned int)lastRemovePointIndex ){
            tmpBackwardPoints->pop_back();
            //cout << " removed 1 ";
        }
        //cout << "\n";

        // Add the crossing point
        if(crossPointPtr != 0){
            int vecSize = tmpBackwardPoints->size();
            if( ( vecSize == 0 || 
               ( (*tmpBackwardPoints)[vecSize-1].GetX() != 
                crossPointPtr->GetX() ||
               (*tmpBackwardPoints)[vecSize-1].GetY() != 
                crossPointPtr->GetY() ) ) &&
               !AlmostEqual(tmpBackwardPoints->back(), *crossPointPtr) )
               tmpBackwardPoints->push_back(*crossPointPtr);
            updated = true;
            //cout << "+++ Crossing point (with old): " << 
            //(lastRemovePointIndex - 1) << " added: " << 
            // *crossPointPtr << "\n";
        }
    }
    else{
        //cout << "No intersection with previous sections found." << "\n";
    }




    return updated;

}



/*
  Calculates the region for a way section that connects points from Nodes.
*/
static void calcWayRegion(const SimpleLine* sectionSLinePtr, 
        Region*& sectionRegionPtr, double width, double& minBoundX, 
        double& maxBoundX, double& minBoundY, double& maxBoundY){

//        cout << "#### ################## ####" << "\n";
//        cout << "#### Given simple line: ####" << endl << 
//          *sectionSLinePtr << "\n";
//        cout << "# Simple starts smaller? : " << 
//         (sectionSLinePtr->StartsSmaller()) << "\n";
//        cout << "# Size of simple line (halfsegments): " << 
//          sectionSLinePtr->Size() << "\n";
//        bool sLineIsOrdered = sectionSLinePtr->IsOrdered();
//        cout << "# Simple line is ordered?: " << sLineIsOrdered << "\n";
//        cout << "#### ################## ####" << "\n";

        // Pointer for the region that will be constructed.
        //Region* actualRegionPtr = new Region(0);

        // This vector will contain the copies of the Points that build the 
        // simple line (given parameter: sectionSLinePtr).
        vector<Point>* orgPoints = new vector<Point>;
        orgPoints->clear();
        
        // Start point of simple line that is the base for region creation.
        Point slineStartPoint = *new Point(true, 1, 1);
        slineStartPoint = sectionSLinePtr->StartPoint();

        // End point of simple line
        Point slineEndPoint = *new Point(true, 1, 2);
        slineEndPoint = sectionSLinePtr->EndPoint();
        //Point actPoint = *new Point(true, 1, 3);


        //cout << "#### slineStartPoint: " << endl << slineStartPoint << "\n";
        //cout << "#### slineEndPoint: " << endl << slineEndPoint << "\n";

        // 1. Add Start point
        orgPoints->push_back(*new Point(true, slineStartPoint.GetX(), 
                                slineStartPoint.GetY()));
        Point* actualPointPtr = new Point(slineStartPoint);

        // 2. Add other points
        Point* nextPointPtr = new Point(true, 7, 7);
        int avoidEndlessCounter = 1;
        int matchNr = 1;

        // index for points found in vector
        int presentIndex = *new int(0);

        // Access all points in order.
        while(nextPointPtr != 0 && avoidEndlessCounter < 500){

            // Find the next point in half segments of simple line
            findSecPointInHalfSegmentsOfSLine(actualPointPtr, 
                        sectionSLinePtr, nextPointPtr, true, matchNr);

            // Check if point (or point with same coordinates) is 
            // already in vector.
            bool pointAlreadyPresent = pointPresentInPointVector(orgPoints, 
                        nextPointPtr, presentIndex);

            // If multiple occurence, check next.
            while(pointAlreadyPresent && nextPointPtr != 0){
                matchNr++;
                findSecPointInHalfSegmentsOfSLine(actualPointPtr, 
                            sectionSLinePtr, nextPointPtr, true, matchNr);

                // Check if point (or point with same coordinates) is 
                // already in vector.
                pointAlreadyPresent = pointPresentInPointVector(orgPoints, 
                            nextPointPtr, presentIndex);
            }

            // Add point if found (not 0) and not already present.
            if(!pointAlreadyPresent && nextPointPtr != 0){
                // Add
                orgPoints->push_back(*new Point(true, nextPointPtr->GetX(), 
                                        nextPointPtr->GetY()));

                // Switch actual to next (has been added.) in order to 
                // determin new next in the following loop cycle.
                actualPointPtr = nextPointPtr;

                // Reset matchNr for next search.
                matchNr = 1;
            }

            avoidEndlessCounter++;
        }


        //int numP = (int) orgPoints->size();
        //cout << "Points in vector " << numP << "\n";
        //for(int h=0; h< numP; h++){
            //cout << "Point " << h << ": " << (*orgPoints)[h] << "\n";
        //}
        
        // 3. Add end point if not already present (no point with same 
        // coordinates is in vector).
        if(!pointPresentInPointVector(orgPoints, &slineEndPoint, 
                                        presentIndex))
            orgPoints->push_back(*new Point(true, slineEndPoint.GetX(), 
                                    slineEndPoint.GetY()));

        // Width must not be 0.
        if(width == 0.0){
            width = 8.0;
        }

    // Earth radius (approx.)
    //double r = double(6371000.785);

        // Pointers will keep track of previously calculated and 
        // original points. 
        Point* previousFirstCalc = 0;
        Point* previousSecondCalc = 0;
        Point* previousFirstOrg = 0;
        Point* previousSecondOrg = 0;



        // Calculate a simple preview (parallel rectangle sides for 2 
        // existing org points) of the calculated line.
        vector<Point>* calcPreviewPoints = new vector<Point>;
        CalculatePreviewPoints(orgPoints, calcPreviewPoints, width);




        //for(int idx = 0; idx < calcPreviewPoints->size(); idx++){
            //cout << "Point in calc preview points [" << idx << "]: " << 
            // (*calcPreviewPoints)[idx] << "\n";
        //}




        // Vetor for the calc points. They are added in inverted order 
        // (compared to corresponding orignal points from sline).
        vector<Point>* tmpBackwardPoints = new vector<Point>;
        tmpBackwardPoints->clear();
        //cout << " Vector tmpBackwardPoints cleared." << "\n";

        // In each loop cycle two points are selected. Therefore loop 
        // until i < (linePointsPtr->Size() - 1).
        // Only the first calculated (moved to right) point or intersection 
        // is added except for last loop cycle, then also second. 
        for(unsigned int i = 0; i < (orgPoints->size() - 1); i++){
            Point* actualPoint = new Point((*orgPoints)[i]);
//            Point* secondPoint = new Point(true, 0, 0);
            Point* secondPoint = new Point((*orgPoints)[i+1]);

            // Get coordinates
            double xActual = (double) actualPoint->GetX();
            double yActual = (double) actualPoint->GetY();

            double xSecond = (double) secondPoint->GetX();
            double ySecond = (double) secondPoint->GetY();

            // Variables for the calculated right side of the rectangle.
            double movedX1;
            double movedY1;
            double movedX2;
            double movedY2;

            // Calculate rectangle (to the right side) with the two points.
            calcRightRect(xActual, yActual, xSecond, ySecond, width, movedX1, 
                            movedY1, movedX2, movedY2);

            // New points on heap with calculated coordinates
            Point* rightPointCalcPtr = new Point(true, 
                        *(new double(movedX1)), *(new double(movedY1)));

            // Second point (needed in next loop cycle)
            Point* secondPointCalcPtr = new Point(true, 
                        *(new double(movedX2)), *(new double(movedY2)));



            if(i==0){ // First rectangle (for first two points in line)
                //cout << "*** First rect" << "\n";

                tmpBackwardPoints->push_back(*rightPointCalcPtr);

                //cout << " Added to vector tmpBackwardPoints: " << 
                // *rightPointCalcPtr << "\n";

                // Add the second point also if the sline has only two points.
                if(orgPoints->size() == 2){
                    //cout << 
                 //" Org point size == 2. Only two points in line." << "\n";
                    if( !AlmostEqual(tmpBackwardPoints->back(), 
                        *secondPointCalcPtr) ){
                        tmpBackwardPoints->push_back(*secondPointCalcPtr);
                    }
                    //cout << " Added 2nd calc to vector tmpBackwardPoints: " 
                    // << *secondPointCalcPtr << "\n";
                }

                // Remember points for next loop cycle. 
                previousFirstOrg = actualPoint;
                previousSecondOrg = secondPoint;
                previousFirstCalc = rightPointCalcPtr;
                previousSecondCalc = secondPointCalcPtr;

            }
            else if(i > 0){
                // Not the first rect
                // 1. Check if the actual calc line crosses a previously 
                // calculated line.
                Point* crossPointPtr = new Point(true, 0.0, 0.0);
                
                // oldI will be set to index of first of the two points 
                // for which the calculated line crosses the actual calc line.
                int oldI = int(0);
                
                // Check if the calculated line crosses old calculated lines 
                // (complete inner filled region, cycle) and update.
                bool oldCrossed = CheckIfCalcLineCrossesOldCalcsAndUpdate(i, 
                        rightPointCalcPtr, secondPointCalcPtr, oldI, 
                        crossPointPtr, calcPreviewPoints, tmpBackwardPoints);

                //cout << "oldI: " << oldI << "\n";
                if(oldCrossed){
                    //cout << "Updated after crossing old calculated line." 
                    // << "\n";
                    previousFirstOrg = actualPoint;
                    previousSecondOrg = secondPoint;
                    previousFirstCalc = crossPointPtr;
                    previousSecondCalc = secondPointCalcPtr;

                } // end if oldCrossed 
                else{
                    // No crossing of calc line with old calc lines.
                    // 2. Right curve?
                    
                    // Direction in last loop cycle
                    double directionPrev = previousFirstOrg->Direction(
                                    *previousSecondOrg, true, 0, false);
                    double directionNow = actualPoint->Direction(
                                    *secondPoint, true, 0, false);
                    //cout << "############ directionPrev: " << 
                    // directionPrev << "\n";
                    //cout << "############ directionNow: " << 
                    // directionNow << "\n";
    
                    // Calculate difference to North for now direction 
                    // (clockwise turn towards north)
                    double nowDiffNorth = 360.0 - directionNow;
                    // Calculate new previous direction adding the difference
                    // from above (turn sync with now which points to North)
                    double directionPrevDiff = directionPrev + nowDiffNorth;
                    // If the new direction is larger than 360 substract 360 
                    //(must always be true: 0 <= direction < 360).
                    if(directionPrevDiff > 360.0)
                        directionPrevDiff = directionPrevDiff - 360.0;
    
                    //cout << "############ nowDiffNorth: " << 
                    // nowDiffNorth << "\n";
                    //cout << "############ directionPrevDiff: " << 
                    // directionPrevDiff << "\n";

                // Right curve. The sync clockwised turned prev direction is
                // larger than 180 (between S, SW, N while now is N).
                    if( directionPrevDiff > 180.0)
                    {
                        // The line curves right. 
                        //cout << "*** Right curve" << "\n";



                        // Indicates if an intersection is found (e.g. 
                        // current calculated right rect side with last 
                        // right rect side)
                        bool intersectionFound = false;

                        // Preparation for crossing point (use func 
                        // Intersection of HalfSegments)(prev right side 
                        // and actual right)
                        HalfSegment* prevHs = 0;
                        HalfSegment* actualHs = 0;

                        if( !AlmostEqual(*previousFirstCalc, 
                                         *previousSecondCalc) )
                            prevHs = new HalfSegment(true, 
                                *previousFirstCalc, *previousSecondCalc);
                        else{
                            //cout << "Right curve, prevHS skipped \n";
                        }
 
                        if( !AlmostEqual(*rightPointCalcPtr, 
                                         *secondPointCalcPtr) )
                            actualHs = new HalfSegment(true, 
                                    *rightPointCalcPtr, *secondPointCalcPtr);
                        else{
                            //cout << "Right curve, actualHs skipped \n";
                        }

                        Point* crossingPointPtr = new Point(true, 0 , 0);

                        // Crossing point determination
                        if( prevHs != 0 && actualHs != 0 && 
                            prevHs->Intersects(*actualHs, 0) ){
                            prevHs->Intersection(*actualHs, 
                                            *crossingPointPtr, 0);
                            intersectionFound = true;
                            //cout << "   ***   Crossing point determined: "
                            // << *crossingPointPtr << "\n";
                        }

                        // 2.1 Normal case for right curve (intersection 
                        // of calc line with calc line from previous step)
                        // Add crossing point only if found and not the 
                        // same as last.
                        if(intersectionFound){
                            if( tmpBackwardPoints->size() > 0 && 
                                crossingPointPtr != 0 &&
                                ( tmpBackwardPoints->back().GetX() != 
                                    crossingPointPtr->GetX() || 
                                  tmpBackwardPoints->back().GetY() != 
                                    crossingPointPtr->GetY() ) && 
                                !AlmostEqual(tmpBackwardPoints->back(), 
                                                *crossingPointPtr) ){

                                tmpBackwardPoints->push_back(
                                                *crossingPointPtr);
                                //cout << " Added (right curve, normal case):
                                // " << *crossingPointPtr << "\n";

                                // Update pointers for next loop cycle.
                                previousFirstOrg = actualPoint;
                                previousSecondOrg = secondPoint;
                                previousFirstCalc = crossingPointPtr;
                                previousSecondCalc = secondPointCalcPtr;

                            }
                            else{
                                // Crossing point almost equal to last 
                                // calc point or invalid. Do not add.
                                // Update pointers for next loop cycle, 
                                // first calc point remains unchanged.
                                previousFirstOrg = actualPoint;
                                previousSecondOrg = secondPoint;
                                previousSecondCalc = secondPointCalcPtr;
                                //cout << "Point already present, not added 
                                // (A1). Skipped point: " << 
                                // *crossingPointPtr << "\n";

                            }
                        } // end if(intersectionFound)
                        else{
                            // 2.2 Intersection not found.
                            // Calculate extended line from actual 1st calc 
                            // to 2nd calc along the straight ahead of 2nd.

                            double rCalcX = rightPointCalcPtr->GetX();
                            double rCalcY = rightPointCalcPtr->GetY();
                            double secondCalcX = secondPointCalcPtr->GetX();
                            double secondCalcY = secondPointCalcPtr->GetY();

                            double moveDist = 100.0; 
                            // 100 meter is enough for crossing check.
                            double extendedX = 0.0;
                            double extendedY = 0.0;
                            calcLongerLinePoint(rCalcX, rCalcY, secondCalcX, 
                                secondCalcY, moveDist, extendedX, extendedY);

                            // Create the point with the calculated 
                            // coordinates and HalfSegment if possible.
                            Point* extendedPointPtr = new Point(true, 
                                            extendedX , extendedY);
                                
                            //HalfSegment* extendedHs = 0;

                            if( !AlmostEqual(*rightPointCalcPtr, 
                                             *extendedPointPtr) )
                                //extendedHs = new HalfSegment(true, 
                                //    *rightPointCalcPtr, *extendedPointPtr);
                                ;
                            else{
                                //cout << "Right curve,extendedHs skipped \n";
                            }






                            Point* extPrevXingPtr = new Point(true, 0.0, 0.0);
                
                            // extXingI will be set to index of first of the 
                            // two points for which the line crosses the 
                            // extend line.
                            int extXingI = int(0);
                
                            // Check if the calculated line crosses old 
                            // calculated lines (complete inner filled 
                            // region) and update.
                            bool extCross = 
                                CheckIfCalcLineCrossesOldCalcsAndUpdate(i, 
                                    rightPointCalcPtr, extendedPointPtr, 
                                    extXingI, extPrevXingPtr, 
                                    calcPreviewPoints, tmpBackwardPoints);

                            //cout << "extXingI: " << extXingI << "\n";
                            if(extCross){
                                //cout << "Crossing extended with old line 
                                // with index: " << extXingI << "\n";
                                // Crossong point is added in called method
                                // (and unnecessary points are removed).

                                // Update pointers for next loop cycle.
                                previousFirstOrg = actualPoint;
                                previousSecondOrg = secondPoint;
                                //previousFirstCalc = extPrevXingPtr;
                                previousSecondCalc = extPrevXingPtr;

                            } // end if (extCross) 




                            else{ // refers to if(extCross)


                                if( directionPrevDiff >= 270.0)
                                {
                                    // Case 1: Direction changed less than
                                    // 90 deg (only slight right curve)

                                    if(directionPrevDiff >= 352.0){
                                        // Case 1.1: Direction change even 
                                        // less than 8 deg. Add prev 2nd 
                                        // calc point.
                                        //cout << "*** Right curve, slight,
                                        // previousSecondCalc added, 
                                        // no intersection." << "\n";

                                        tmpBackwardPoints->push_back(
                                                    *previousSecondCalc);
                                    
                                        // Extended line does not cross 
                                        // previous calc line (
                                        // and curve <= 8 deg). 
                                        // Add prev 2nd calc.
                                        // Update pointers for next loop 
                                        // cycle, first calc point 
                                        // remains unchanged.
                                        previousFirstOrg = actualPoint;
                                        previousSecondOrg = secondPoint;
                                        previousFirstCalc = 
                                                    previousSecondCalc;
                                        previousSecondCalc = 
                                                    secondPointCalcPtr; 

                                    }
                                    else{
                                        // Case 1.2: Direction change NOT 
                                        // less than 8 deg. Add no point.
                                        
                                        //cout << "*** Right curve, slight, 
                                        // nothing added, no intersection."
                                        // << "\n";

                                        // Extended line does not cross 
                                        // previous calc line (and curve < 
                                        // 90 deg). Add no calc point.
                                        // Update pointers for next loop 
                                        // cycle, first calc point remains 
                                        // unchanged.
                                        previousFirstOrg = actualPoint;
                                        previousSecondOrg = secondPoint;
                                        previousSecondCalc = 
                                                    secondPointCalcPtr; 

                                        
                                    }
 
                                } // end if( directionPrevDiff >= 270.0) - 
                                //(Slight right curve)
                                else{
                                    // Case 2: Sharp right curve 
                                    // (direction > 90 degrees)
                                    // Check if org line (actualPoint, 
                                    // secondPoint) crosses (
                                    // previousFirstOrg, previousFirstCalc)
                                    
                                    HalfSegment* actualOrgHs = 0;

                                    if( !AlmostEqual(*actualPoint, 
                                            *secondPoint) )
                                        actualOrgHs = new HalfSegment(true, 
                                            *actualPoint, *secondPoint);
                                    else{
                                        //cout << "Right curve, sharp, 
                                        // actualOrgHs skipped \n";
                                    }

                                    HalfSegment* prevDownHs = 0;

                                    if( !AlmostEqual(*previousFirstOrg, 
                                            *previousFirstCalc) )
                                      prevDownHs = new HalfSegment(true, 
                                      *previousFirstOrg, *previousFirstCalc);
                                    else{
                                        //cout << "Right curve, sharp, 
                                        // previousFirstOrg skipped \n";
                                    }

                                    // Crossing point determination prep
                                    Point* crossPointPrevDownActPtr = 
                                                new Point(true, 0 , 0);
                                   // bool intersectionPrevDownActFound = 
                                   //             false;

                                    // Does the previous lower side of rect 
                                    // cross the actual org point line (
                                    // act left rect side)?
                                    if( actualOrgHs != 0 && 
                                        prevDownHs != 0 && 
                                        prevDownHs->Intersects(
                                            *actualOrgHs, 0) ){

                                        prevDownHs->Intersection(
                                            *actualOrgHs, 
                                            *crossPointPrevDownActPtr, 0);
                                        //intersectionPrevDownActFound = true;
                                        //cout << "-Crossing point (act left
                                        // with prev down): " << 
                                        //*crossPointPrevDownActPtr << "\n";

                                        // Remove previous 1st calc (should 
                                        // be last in tmp vector) and add 
                                        // actual second org point.
                                        if( AlmostEqual(
                                            tmpBackwardPoints->back(), 
                                            *crossPointPrevDownActPtr) ){
                                            tmpBackwardPoints->pop_back();
                                            //cout << "-Removed prev 1st 
                                            // calc from vector." << "\n";

                                        }
                                        else{
                                            //cout << "--- Last point not" + 
                                            //" removed, not prev 1st calc. "
                                            //<< "\n";

                                        }

                                        tmpBackwardPoints->push_back(
                                                *secondPoint);
                                        //cout << " + Added 2nd org point " + 
                                            //"to vector instead: " << 
                                            //*secondPoint << "\n";
                                    
                                        // Update ptrs for next loop cycle.
                                        previousFirstOrg = actualPoint;
                                        previousSecondOrg = secondPoint;
                                        previousFirstCalc = 
                                                    rightPointCalcPtr;
                                        previousSecondCalc = 
                                                    secondPointCalcPtr;

                                    }
                                    else{
                                        // Add nothing.

                                        //cout << "Right curve, sharp. "+
                                        //"Nothing added, no intersection." 
                                        // << "\n";
                                        // Update pointers for next loop 
                                        // cycle, first calc point remains 
                                        // unchanged.
                                        previousFirstOrg = actualPoint;
                                        previousSecondOrg = secondPoint;
//                                   previousSecondCalc = secondPointCalcPtr;

                                    }




                                } // end else (sharp right curve)


                            } // end else for if(intersectionExtendedFound)
  


                        } // end else for end if(intersectionFound) - (2.2)




                    } // end if (right curve) - if(directionPrevDiff>180.0)
                    else{
                        // Left curve and no intersection with previous 
                        // calculated lines.
                        // Check if line that closes the gap between 
                        // actual and prev rect (left curve) crosses old 
                        // calc lines.
                        //cout << "*** Left curve" << "\n";

                        Point* crossGapPointPtr = new Point(true, 0.0, 0.0);
                
                        // oldGapI will be set to index of first of the 
                        // two points for which the calculated line crosses 
                        // the gap line.
                        int oldGapI = int(0);
                
                        // Check if the calculated line crosses old 
                        // calculated lines (complete inner filled 
                        // region, cycle) and update.
                        bool gapCrossed = 
                            CheckIfCalcLineCrossesOldCalcsAndUpdate(i, 
                                secondPointCalcPtr, previousSecondCalc, 
                                oldGapI, crossGapPointPtr, 
                                calcPreviewPoints, tmpBackwardPoints);

                        //cout << "oldGapI: " << oldGapI << "\n";
                        if(gapCrossed){
                            //cout << "Crossing gap line." << "\n";

                        } // end if gapCrossed 
                        else{
                            // Gap line not crossed. Add prev 2nd calc 
                            // and actual 1st calc point to vector.

                            tmpBackwardPoints->push_back(
                                        *previousSecondCalc);
                            tmpBackwardPoints->push_back(
                                        *rightPointCalcPtr);
                            //cout << "Gap line closed " + 
                            // " (no intersection)." << "\n";

                        }
                        
                        previousFirstOrg = actualPoint;
                        previousSecondOrg = secondPoint;
                        previousFirstCalc = rightPointCalcPtr;
                        previousSecondCalc = secondPointCalcPtr;

                        
                    } // end else (left curve)
                    
                    
                } // end else (not old crossed)
                
                // Check if last point is reached
                if( i == (orgPoints->size() -2) ){
                    //cout << "*** Last point reached." << "\n";

                    // Check if calc line (rightPointCalcPtr, 
                    // secondPointCalcPtr) crosses line from 1st org 
                    // to 1st calc point. 
                    // Calculate first calc point again.
                    Point firstOrgPoint  = (*orgPoints)[0];
                    Point secondOrgPoint = (*orgPoints)[1];

                    // Get coordinates
                    double xFirstOrg = (double) firstOrgPoint.GetX();
                    double yFirstOrg = (double) firstOrgPoint.GetY();

                    double xSecondOrg = (double) secondOrgPoint.GetX();
                    double ySecondOrg = (double) secondOrgPoint.GetY();

                    // Variables for the calculated right side of 
                    // the rectangle.
                    double calcX1;
                    double calcY1;
                    double calcX2;
                    double calcY2;

                    // Calculate rectangle (to the right side) with 
                    // the two points.
                    calcRightRect(xFirstOrg, yFirstOrg, xSecondOrg, 
                        ySecondOrg, width, calcX1, calcY1, calcX2, calcY2);

                    Point* firstCalcPointPtr = new Point(true, 
                                                calcX1, calcY1);
                    Point* secondCalcPointPtr = new Point(true, 
                                                calcX2, calcY2);

                    // Create halfsegm for 1st rect down side to check if 
                    // the first down side of 1st rect crosses last 
                    // calculated line.
                    HalfSegment* firstDownHs = 0;

                    if( !AlmostEqual(firstOrgPoint, *firstCalcPointPtr) )
                        firstDownHs = new HalfSegment(true, firstOrgPoint, 
                                        *firstCalcPointPtr);
                    else{
                        //cout << "First down side recalculation skipped 
                        // for final point. \n";
                    }

                    // Halfsegment for now calc line (right side of rect 
                    // for actual 2 org points)
                    HalfSegment* nowCalcHs = 0;

                    if( !AlmostEqual(*rightPointCalcPtr, 
                            *secondPointCalcPtr) )
                        nowCalcHs = new HalfSegment(true, 
                                *rightPointCalcPtr, *secondPointCalcPtr);
                    else{
                        //cout << "Now calc line calc skipped for" + 
                        //" final point. \n";
                    }

                    // Crossing point determination preparation 
                    Point* crossPointFirstDownActCalcPtr = 
                            new Point(true, 0 , 0);
                    //bool intersectionFirstDownActCalcFound = false;

                    // Does the 1st lower side of rect cross the actual 
                    // calc point line (act right rect side)?
                    if( firstDownHs != 0 && nowCalcHs != 0 && 
                        nowCalcHs->Intersects(*firstDownHs, 0) ){
                        //intersectionFirstDownActCalcFound = true;
                        nowCalcHs->Intersection(*firstDownHs, 
                                *crossPointFirstDownActCalcPtr, 0);
                        
                        // Clear the tmp vector and add crossing 
                        // point and last calc point.
                        tmpBackwardPoints->clear();
                        tmpBackwardPoints->push_back(
                                    *crossPointFirstDownActCalcPtr);
                        tmpBackwardPoints->push_back(
                                    *secondPointCalcPtr);
                        
                    }
                    else{
                        // Last calc line does not cross line from 1st 
                        // org to 1st calc point.
                        // Check if first calc line crosses line from last 
                        // org point to last calc point.
                        HalfSegment* firstCalcHs = 0;

                        if( !AlmostEqual(*firstCalcPointPtr, 
                                         *secondCalcPointPtr) )
                            firstCalcHs = new HalfSegment(true, 
                                    *firstCalcPointPtr, *secondCalcPointPtr);
                        else{
                            //cout << 
                            //"First calc side skipped for final point. \n";
                        }

                        HalfSegment* nowUpHs = 0;

                        if( !AlmostEqual(*secondPoint, *secondPointCalcPtr) )
                            nowUpHs = new HalfSegment(true, *secondPoint, 
                                            *secondPointCalcPtr);
                        else{
                            //cout << 
                            // "Now up side skipped for final point. \n";
                        }

                        // Crossing point determination preparation 
                        Point* crossPointFirstCalcNowUpPtr = 
                                            new Point(true, 0 , 0);
                        //bool intersectionFirstCalcNowUpFound = false;

                        // Does the right side of 1st rect cross the 
                        // actual rect's up side?
                        if( firstCalcHs != 0 && nowUpHs != 0 && 
                            nowUpHs->Intersects(*firstCalcHs, 0) ){
                            //intersectionFirstCalcNowUpFound = true;
                            nowUpHs->Intersection(*firstCalcHs, 
                                    *crossPointFirstCalcNowUpPtr, 0);
                            //cout << "X-ing (1st calc, now up): " << 
                            // *crossPointFirstCalcNowUpPtr << "\n";
                        
                            // Clear the tmp vector and add crossing point
                            // and last calc point.
                            tmpBackwardPoints->clear();
                            tmpBackwardPoints->push_back(
                                        *firstCalcPointPtr);
                            tmpBackwardPoints->push_back(
                                        *crossPointFirstCalcNowUpPtr);
                        
                        }
                        else{
                            // Just add the last calculated point (end 
                            // reached and no intersections detected)
                            // if not already last.
                            if( !AlmostEqual(tmpBackwardPoints->back(),
                                          *secondPointCalcPtr) ){
                                tmpBackwardPoints->push_back(
                                          *secondPointCalcPtr);
                            }

                        }


                    }
                    

                } // end if( i == (orgPoints->size() -2) )

            } // else if(i > 0)

        //cout << "*previousFirstOrg: " << *previousFirstOrg << " , " << 
        // "*previousSecondOrg: " << *previousSecondOrg << "\n";
        //cout << "*previousFirstCalc: " << *previousFirstCalc << " , " << 
        // "*previousSecondCalc: " << *previousSecondCalc << "\n";



    }// end for(unsigned int i = 0; i < (linePointsPtr->Size() - 1); i++)










    //cout << "Number of points in tmp vector:" << 
    // tmpBackwardPoints->size() << "\n";

    //cout << "Points in tmp vector:" << "\n";
    // Move calculated points in opposite direction to end of 
    // vector with original sline points. 
    while(!tmpBackwardPoints->empty()){
        if( !AlmostEqual(orgPoints->back(), tmpBackwardPoints->back()) ){
            orgPoints->push_back(tmpBackwardPoints->back());
            //cout << tmpBackwardPoints->back().GetY() << " " << 
            // tmpBackwardPoints->back().GetX() << "\n";
            //cout << "*Move from tmp to org vector*";
        }
        tmpBackwardPoints->pop_back();
    }
    //cout << "\n";

    // Add first point also as last point to close the cycle.
    orgPoints->push_back(*new Point((*orgPoints)[0]));

    if(!getDir(*orgPoints))
        reverseCycle(*orgPoints);
    


    // Determine minx, maxx, miny, maxy (for example for a bounding box). 
    // Set values of first point in vector, update in loop.
    double tmpMinX = (*orgPoints)[0].GetX();
    double tmpMaxX = (*orgPoints)[0].GetX();
    double tmpMinY = (*orgPoints)[0].GetY();
    double tmpMaxY = (*orgPoints)[0].GetY();

    for(unsigned int allIndex = 1; allIndex < orgPoints->size(); allIndex++){
        double actX = (*orgPoints)[allIndex].GetX();
        double actY = (*orgPoints)[allIndex].GetY();

        if( actX < tmpMinX)
            tmpMinX = actX;

        if( actX > tmpMaxX)
            tmpMaxX = actX;

        if( actY < tmpMinY)
            tmpMinY = actY;

        if( actY > tmpMaxY)
            tmpMaxY = actY;

    }

    // Update values in parameters (visible for caller)
    minBoundX = *new double(tmpMinX);
    maxBoundX = *new double(tmpMaxX);
    minBoundY = *new double(tmpMinY);
    maxBoundY = *new double(tmpMaxY);


    // Build the actual part region
    vector<vector<Point> > tmpAllVectorVector = *new vector<vector<Point> >;
    tmpAllVectorVector.clear();
    tmpAllVectorVector.push_back(*orgPoints);
    //cout << "Number of points for region: " << orgPoints->size() << "\n";
    Region* tmpActualPartRegionPtr = new Region(0);
    tmpActualPartRegionPtr = buildRegion(tmpAllVectorVector);
    tmpActualPartRegionPtr->TrimToSize();








        // Assign the region to result pointer (for caller)
        sectionRegionPtr = tmpActualPartRegionPtr;
        // Finished. Region is available for caller (also points and 
        // half segments).


}




// ********************************************************************



/*
Help function that executes a query with QueryProcessor and returns a 
result relation pointer.
*/
static void GetRelFromQueryString(string* inputQueryString, 
                                    Relation*& resultRelation){

    //cout << "*inputQueryString: " << *inputQueryString << "\n";

    // The list expression as string for the string from above will be 
    // stored in:
    string listExprForQueryString;
    //cout << "Original query text: " << *inputQueryString << "\n";

    // Parse string that is valid as input in command line to list 
    // expression string.
    SecParser parser;
    int parseResult = parser.Text2List(*inputQueryString, 
                                        listExprForQueryString);

    // Result of query will be available with:
    Word queryResult;

    // parseResult: 0 = success, 1 = error, 2 = stack overflow
    if(parseResult == 0) 
    {
        int parsedStringLength = listExprForQueryString.length();

        // Cut for use in executeQuery()
        listExprForQueryString = 
                listExprForQueryString.substr(7, parsedStringLength - 9);
        //cout << "Nested list, parsed and cut for use : " << 
        // listExprForQueryString << "\n";

        //cout << "Working (local)..." << "\n";
        // Use query processor to get the way type for the way id.
        QueryProcessor::ExecuteQuery(listExprForQueryString, queryResult);
       
//        cout << "The number of elements in NodesHochdahl is: " << 
//                ((CcInt*) queryResult.addr)->GetIntval() << "\n";
    }
    else if(parseResult == 1){
        cout << "*** An error occurred" << "\n";
        resultRelation = 0;
        return;
    }
    else if(parseResult == 2){
        cout << "*** A stack overflow occurred" << "\n";
        resultRelation = 0;
        return;
    }

    // The relation is the result from the query (and contains tuples 
    // with the NodeNewId and WayTags)
    try{
        if(queryResult.addr == 0){
            cout << 
            "*** No result for internal query. Does \"WaysHochdahl\" exist?" 
                << "\n";
            resultRelation = 0;
            return;
        }

        resultRelation = (Relation*) queryResult.addr;


    }
    catch(const std::exception ex){
        cout << "Result to relation conversion failed." << "\n";
    }
}


// *********************************************************************


/*
Help function that calculates cordinates for a rectangle.
*/
static void CalcRectCoords(double& width, double& height, double& xOrg, 
                double& yOrg, double& downLeftX, double& upRightX, 
                double& downLeftY, double& upRightY){
        //cout << "   ** CalcRectCoords called. \n";

        // Width and height must not be 0.
        if(width == 0.0){
            width = 8.0;
        }

        if(height == 0.0){
            height = 8.0;
        }

    // [m]
    double r = double(6371000.785);

        // Absolute values
//    double* xR1 = new double(*xOrg * M_PI / 180);   // not used
    double yR1 = ((double) (yOrg * M_PI / 180));
        
    // Meters per degree. Delta of Lat(Breitengrad) is equivalent to delta y,
    // delta Lon (Längengrad) to delta x
    double meterPerDegLat = ((double)(M_PI * r / 180));
    double meterPerDegLong = ((double)((M_PI * r / 180) * cos(yR1)));


        // Coordinate calculation of points in corners, starting with point
        // in upper left corner.

    upRightX = ((double)(xOrg + (width / 2) / meterPerDegLong));
    upRightY = ((double)(yOrg + (height / 2) / meterPerDegLat));

    downLeftX = ((double)(xOrg - (width / 2) / meterPerDegLong));
    downLeftY = ((double)(yOrg - (height / 2) / meterPerDegLat)); 
        // Latitude increases towards north pole (90deg).

        //rectPtr = new Rectangle<2>(true, downLeftX, upRightX, 
        // downLeftY, upRightY);

}


// *********************************************************************


/*
Help function, copies entries from one vector to the other.
*/
static void CopyCandidateEntries( 
        vector<MatchedPoint*>* orgMatchedPointsVectorPtr, 
        vector<MatchedPoint*>* copyMatchedPointsVectorPtr){

    //Valid pointers to vectors?
    if(orgMatchedPointsVectorPtr == 0 || copyMatchedPointsVectorPtr == 0)
        return;

    for(unsigned int i = 0; i < orgMatchedPointsVectorPtr->size(); i++){
        copyMatchedPointsVectorPtr->push_back( 
                (*orgMatchedPointsVectorPtr)[i] );
    }


}



// ***************************************************************


/*
Help function, calculates distance between two points.
*/
  /*
double CalcDistance(MatchedPoint* matchPtA, MatchedPoint* matchPtB){
    double xA = matchPtA->lonOrg;
    double yA = matchPtA->latOrg;

    double xB = matchPtB->lonOrg;
    double yB = matchPtB->latOrg;

    Point ptA = Point(true, xA, yA);
    Point ptB = Point(true, xB, yB);
    double dist = ptA.Distance(ptB);

    return dist;

}
  */


// ******************************************************************
/*
Help function, for MHT. Check candidates and remove less likely candidates. 
Keep only maximum of 20.
MatchedPoint attributes
    int regId;
    string PosDate;
    string PosTime;
    string RegionName;
    string RegionType;
    double latOrg;
    double lonOrg;
    int numOfSats;
    double accuracy; // Less is better

    // Calculated values
    int PosYear;
    int PosMonth;
    int PosDay;
    int PosHour;
    int PosMinute;
    int PosSecond;
    Region* matchedReg;
    bool matched;
    int NumEdgesToPrev;
    double distToPrev;
*/
static void CandidateClipping(GetNextRawTupleLI* li, unsigned int maxNum){
    //cout << "      **#### CandidateClipping called. \n";

    // Sort first (the less likely ones are at the end then)    
    //std::sort(li->candidates.begin(), li->candidates.end());


    vector<MatchCandidate > sortedVector;
    for(size_t idx = 0; idx < li->candidates.size(); idx++){
        MatchCandidate tmpMCand = *((li->candidates)[idx]);
        sortedVector.push_back(tmpMCand);
    }
    std::sort(sortedVector.begin(), sortedVector.end());

    li->candidates.clear();
    for(size_t idx = 0; idx < sortedVector.size(); idx++){
        MatchCandidate* tmpMCandPtr = new MatchCandidate(sortedVector[idx]);
        li->candidates.push_back(tmpMCandPtr);
    }



    if(li->candidates.size() <= maxNum){
        //Not too big, nothing to cut off.
        return;
    }

    // Too many candidates. Remove the less likely ones.

    // Number to remove
    int numToRemove = li->candidates.size() - maxNum;
    int counter = 0;

    // Remove
    while(counter < numToRemove && li->candidates.size() > 0){
        li->candidates.pop_back();
        counter++;
    }

    //cout << "      **#### CandidateClipping finished. \n";

}



// *****************************************************************


/*
Help function, test clipping of candidates for MHT.
*/
  /*
static void TestClipping(){

          vector<MatchCandidate> candidates;
          vector<MatchedPoint*>* actTripVectorPtr;
          MatchCandidate* actTripCandidate;
          MatchedPoint* actMatchedPoint;

          for(unsigned int j = 0; j<5; j++){
              actTripVectorPtr = new vector<MatchedPoint*>;

              for(unsigned int i = 0; i<4; i++){
                  actMatchedPoint = new MatchedPoint("2016-09-14", 
                        "11:11:01", 6.95327, 51.2087, (10 + i + j), 8.5);
                  actMatchedPoint->regId = 1001;
                  actMatchedPoint->matchedReg = new Region(
                     Rectangle<2> (true, 6.953271, 6.953273, 
                                            51.20873, 51.20876) );
                  actMatchedPoint->RegionName = "Eins";
                  actMatchedPoint->RegionType = "track";

                  actTripVectorPtr->push_back(actMatchedPoint);

              }

              actTripCandidate = new MatchCandidate(actTripVectorPtr);
              actTripCandidate->Score = 100 - j;
              //actTripCandidate->UpdateScore(0);
              candidates.push_back(*actTripCandidate);

          }

          cout << "candidates.size(): " << candidates.size();

          for(unsigned int j = 0; j < candidates.size(); j++){
              cout << "candidates[" << j << "] score: " << 
                        candidates[j].Score << endl;
          }

          std::sort(candidates.begin(), candidates.end());

          // Number to remove
          int maxNum = 2;
          int numToRemove = candidates.size() - maxNum;
          int counter = 0;

          // Remove
          while(counter < numToRemove && candidates.size() > 0){
              candidates.pop_back();
              counter++;
          }

          for(unsigned int j = 0; j < candidates.size(); j++){
              cout << "candidates[" << j << "] score: " << 
                        candidates[j].Score << endl;
          }





}
  */



// ***********************************************************************


/*
Help function, initialization for MHT.

*/
static bool InitMHT(GetNextRawTupleLI* li, Relation* resultRelation, 
                    MatchedPoint* actMatchedPoint){

        //cout << "    **## InitMHT called. \n";

        bool initialized = false;
        //cout << "Init." << endl;

        if(resultRelation != 0){
              // Results available
              int numOfTuples = resultRelation->GetNoTuples();
              //cout << "    Number of tuples:"<< numOfTuples << "\n";


              if(numOfTuples == 0){
                  cout << "Point " << actMatchedPoint->lonOrg << " " << 
                    actMatchedPoint->latOrg << " not matched. " << endl;
              }

              for(int i = 1; i <= numOfTuples; i++){
                  Tuple* tupleInner = resultRelation->GetTuple(i,true);

                  int numOfAttributes = tupleInner->GetNoAttributes();
                  //cout << "Nr of attributes in inner result tuple:"<<
                  // numOfAttributes << "\n";

                  if(numOfAttributes < 8){
                      cout << "Result tuple of inner query has only " << 
                            numOfTuples << " attributes (Exp: 8)." << "\n";
                      break;
                  }

                  //CcInt* wayIdPtr = 
                  //  static_cast<CcInt*>(tupleInner->GetAttribute(0));
                  CcInt* regionIdPtr = 
                    static_cast<CcInt*>(tupleInner->GetAttribute(4));
                  int innerRegId = regionIdPtr->GetIntval();

                  //Region* regionPtr = new Region(*static_cast<Region*>
                    //    (tupleInner->GetAttribute(5)), false);
                  Region* regionPtr = static_cast<Region*>(
                                tupleInner->GetAttribute(5));

                  CcString* namePtr = static_cast<CcString*>(
                                tupleInner->GetAttribute(6));
                                
                  CcString* typePtr = static_cast<CcString*>(
                                tupleInner->GetAttribute(7));

                  // Create vector for trip candidate and corresponding
                  // struct MatchCandidate (if region is not 0 or empty).
                  if(regionPtr != 0 && regionPtr->Area(0) > 0){
                      //cout << "Init candidate: " << innerRegId << 
                      // " ( " << namePtr->toText() << " ) \n";

                      vector<MatchedPoint*>* actTripVectorPtr = 
                                        new vector<MatchedPoint*>;
                      MatchCandidate* actTripCandidate = 
                                    new MatchCandidate(actTripVectorPtr);

                      MatchedPoint* actCopyMatchedPoint =  
                                    new MatchedPoint(actMatchedPoint);
                      actCopyMatchedPoint->regId = innerRegId;
                      actCopyMatchedPoint->matchedReg = regionPtr;
                      
                      actCopyMatchedPoint->RegionName = namePtr->toText();
                      actCopyMatchedPoint->RegionType = typePtr->toText();
                      actCopyMatchedPoint->matched = true;

                      Point orgPoint = Point(true, 
                        actCopyMatchedPoint->lonOrg , 
                        actCopyMatchedPoint->latOrg);
                      if(regionPtr != 0 && orgPoint.Inside(*regionPtr, 0)){
                          actCopyMatchedPoint->distToReg = 0.0;

                          // Store id for exact matched region
                          actCopyMatchedPoint->regIdPointInside = innerRegId;

                          //cout << " *** Point " << 
                          // actCopyMatchedPoint->lonOrg << "," << 
                          // actCopyMatchedPoint->latOrg << 
                          // " is inside region " << innerRegId << "\n";
                          //cout << "Matching region id: " << 
                          // actCopyMatchedPoint->regIdPointInside << "\n";
                      }
                      else if(regionPtr != 0){
                          actCopyMatchedPoint->distToReg = 
                                        regionPtr->Distance(orgPoint, 0);
                          //cout << " *** Point " << 
                          //actCopyMatchedPoint->lonOrg << "," << 
                          //actCopyMatchedPoint->latOrg << 
                          //" is NOT inside region " << innerRegId << 
                          //". Distance: " << 
                          // actCopyMatchedPoint->distToReg << "\n";
                          
                      }
                      else{
                          actCopyMatchedPoint->distToReg = 100.0;
                      }

                      actTripVectorPtr->push_back(actCopyMatchedPoint);
                      actTripCandidate->UpdateScore(0);
                      li->candidates.push_back(actTripCandidate);

                  }

              } // for(int i = 1; i <= numOfTuples; i++)





              // Check number of candidates (later, not for init)
              //CandidateClipping(li, 20);


              initialized = true;
              li->initialized = true;

        } // if(resultRelation != 0)
        else{
            // No result relation for query available. Initialization not
            // possible with this point. Skip.
            initialized = false;
        }

        actMatchedPoint->matched = initialized;
        //cout << "    **## InitMHT finished. \n";
        CandidateClipping(li, 20);
        return initialized;

}



// ********************************************************************


/*
Help function, initialization for MHT.
MatchedPoint attributes:
    int regId;
    string PosDate;
    string PosTime;
    string RegionName;
    string RegionType;
    double latOrg;
    double lonOrg;
    int numOfSats;
    double accuracy; // Less is better

    // Calculated values
    int PosYear;
    int PosMonth;
    int PosDay;
    int PosHour;
    int PosMinute;
    int PosSecond;
    Region* matchedReg;
    bool matched;
    int NumEdgesToPrev;
    double distToPrev;

MatchCandidate attributes:
    vector<MatchedPoint*>* TripCandidateVectorPtr;
    int NumEdgesTotal;
    double AverageSats;
    double AverageAcc; // Smaller values are better
    double DistanceTotal;
    double Score; // Smaller values are better

*/
static void MatchMHT(GetNextRawTupleLI* li, Relation* resultRelation, 
                        MatchedPoint* actMatchedPoint){
    //cout << "   ***### MatchMHT called. \n";

    //CandidateClipping(li, 15);

    // Prepare branching. Move candidate vectors to other container. 
    // Main candidates container in li is then empty and will be filled.
    vector<MatchCandidate*> oldCandidates;
    while(li->candidates.size() > 0){
        oldCandidates.push_back(li->candidates.back());
        li->candidates.pop_back();
    }
    std::sort(oldCandidates.begin(), oldCandidates.end());





    vector<MatchedPoint* > actMatchPoints;
    if(resultRelation != 0){
              // Results available
              int numOfTuples = resultRelation->GetNoTuples();
              //cout << "    Number of tuples:"<< numOfTuples << "\n";

              if(numOfTuples == 0){
                  //cout << "Point " << actMatchedPoint->lonOrg << " " <<
                  // actMatchedPoint->latOrg << " not matched. " << endl;

              }

              bool matched = false;
              for(int i = 1; i <= numOfTuples; i++){
                  // All possible matching "Region nodes" / tuples 
                  // are checked.

                  Tuple* tupleInner = resultRelation->GetTuple(i,true);
                  int numOfAttributes = tupleInner->GetNoAttributes();
                  //cout << 
                  // " Number of attributes in inner result tuple:"<< 
                  // numOfAttributes << "\n";

                  if(numOfAttributes < 8){
                      cout << "Result tuple of inner query has only " << 
                      numOfTuples << " attributes (Exp: 8)." << "\n";
                      break;
                  }

                  //CcInt* wayIdPtr = static_cast<CcInt*>(
                  //       tupleInner->GetAttribute(0));
                  CcInt* regionIdPtr = static_cast<CcInt*>(
                                    tupleInner->GetAttribute(4));
                  int innerRegId = regionIdPtr->GetIntval();

                  //Region* regionPtr = new Region(*static_cast<Region*>(
                  //                tupleInner->GetAttribute(5)), false);
                  Region* regionPtr = static_cast<Region*>(
                                    tupleInner->GetAttribute(5));

                  CcString* namePtr = static_cast<CcString*>(
                                    tupleInner->GetAttribute(6));
                  CcString* typePtr = static_cast<CcString*>(
                                    tupleInner->GetAttribute(7));


                  if(regionPtr != 0 && regionPtr->Area(0) > 0){
                      //cout << "MatchMHT candidate: " << 
                      //innerRegId << " ( " << namePtr->toText() <<
                      // " ) \n";
                      MatchedPoint* actCopyMatchedPoint =  
                                    new MatchedPoint(actMatchedPoint);

                      actCopyMatchedPoint->regId = innerRegId;
                      actCopyMatchedPoint->matchedReg = regionPtr;
                      actCopyMatchedPoint->RegionName = namePtr->toText();
                      actCopyMatchedPoint->RegionType = typePtr->toText();

                      Point orgPoint = Point(true, 
                            actCopyMatchedPoint->lonOrg , 
                            actCopyMatchedPoint->latOrg);
                      if(regionPtr != 0 && orgPoint.Inside(*regionPtr, 0)){
                          actCopyMatchedPoint->distToReg = 0.0;

                          // Store id for exact matched region
                          actCopyMatchedPoint->regIdPointInside = innerRegId;

                          //cout << " *** Point " << 
                          //actCopyMatchedPoint->lonOrg << "," << 
                          //actCopyMatchedPoint->latOrg << 
                          //" is inside region " << innerRegId << "\n";
                      }
                      else if(regionPtr != 0){
                          actCopyMatchedPoint->distToReg = 
                                    regionPtr->Distance(orgPoint, 0);
                          //cout << " *** Point " << 
                          //actCopyMatchedPoint->lonOrg << "," << 
                          //actCopyMatchedPoint->latOrg << 
                          //" is NOT inside region " << innerRegId << 
                          //". Distance: " << 
                          //actCopyMatchedPoint->distToReg << "\n";
                      }
                      else{
                          actCopyMatchedPoint->distToReg = 100.0;
                      }

                      actMatchPoints.push_back(actCopyMatchedPoint);
                      actCopyMatchedPoint->matched = true;

                  }


              } // for(int i = 1; i <= numOfTuples; i++)

              //cout << "Loop 1 completed. \n";

              matched = true;
              actMatchedPoint->matched = matched;

    } // if(resultRelation != 0)
    else{
        // No result relation for query available. Initialization not 
        // possible with this point. Skip.
    }









    // For all old candidate paths a new branch is created.
    for(unsigned int candIdx = 0; candIdx < oldCandidates.size(); candIdx++){
        MatchCandidate* actOldCandidate = oldCandidates[candIdx];
        vector<MatchedPoint*>* actOldCandVectorPtr = 
                    actOldCandidate->TripCandidateVectorPtr;


        for(size_t aIdx = 0; aIdx < actMatchPoints.size(); aIdx++){
            //cout << "1";

            // Candidate for actual point from GPS coordinates
            MatchedPoint* actMPCandidate = actMatchPoints[aIdx];

            // Create vector for trip candidate and corresponding 
            // struct MatchCandidate.
            vector<MatchedPoint*>* actTripVectorPtr = 
                                    new vector<MatchedPoint*>;
            //cout << "2";
            CopyCandidateEntries( actOldCandVectorPtr, actTripVectorPtr);
            //cout << "3";
            //int numOfPointsBeforeAdd = actTripVectorPtr->size();

            // Previous point & mapped region.
            MatchedPoint*  prevMatchedPoint = actTripVectorPtr->back();
            int prevRegId = prevMatchedPoint->regId;

            MatchCandidate* actTripCandidate = 
                                new MatchCandidate(actTripVectorPtr);
            // Complete trip candidate by adding candidate for actual 
            // GPS point at the end of old trip candidate.
            actTripVectorPtr->push_back(actMPCandidate);


            // Find shortest path
            vector<RegNode* >* shortestPathNodesVectorPtr = 
                                        new vector<RegNode* >;
                                        
            vector<vector<RegNode* >* > trialPathsVector;
            //bool shortestPathFound = false;
            //cout << "4";

            if(prevRegId != actMPCandidate->regId){
                //cout << " *# MatchMHT --- FindShortestPath called for "
                // << prevRegId << " , " << innerRegId <<" \n";

                //cout << "5";
                //shortestPathFound = 
                li->regionGraphPtr->FindShortestPath(
                        prevRegId, actMPCandidate->regId, 5, 
                        shortestPathNodesVectorPtr, trialPathsVector);
                //cout << "6";

                //cout << "   ***### MatchMHT - FindShortestPath found. \n";
                actMPCandidate->NumEdgesToPrev = 
                            shortestPathNodesVectorPtr->size();

            }

            actMPCandidate->distToPrev = 3;

            actMPCandidate->matched = true;
            //actTripVectorPtr->push_back(actMPCandidate);
            //cout << "7";
            //actTripCandidate->UpdateScore(numOfPointsBeforeAdd);
            actTripCandidate->UpdateScore(0);
            //cout << "8";


            li->candidates.push_back(actTripCandidate);

            if(li->candidates.size() > 20){
                CandidateClipping(li, 20);
            }

            
        }
        //cout << "\n Loop 2.2 completed. \n";





    } // for(unsigned int candIdx = 0; candIdx < ...
    //cout << "Loop 2 completed. \n";

    if(li->candidates.size() > 15){
        CandidateClipping(li, 15);
    }


    //// Check number of candidates (later, not for init)
    //CandidateClipping(li, 15);

    //cout << "   ***### MatchMHT finished. \n\n";

}



// ************************************************************


/*
Help function that fills the gaps between matched regions. 
Adds shortest path regions.

*/
static void FillGapsBetweenMatchedRegions(GetNextRawTupleLI* li){
    if(li == 0 || li->candidates.size() == 0 || (li->candidates)[0] == 0){
        return;
    }
    // best matching candidate from MHT 
    MatchCandidate* completedBestCandidate = (li->candidates)[0];

    //// The new match candidate that will contain shortest path 
    // inter-regions
    //MatchCandidate* completedBestCandidate = 
    //             new MatchCandidate(bestCandidate);

    vector<MatchedPoint*> CopyTripCandidateVector;
    for(unsigned int idx = 0; 
        idx < completedBestCandidate->TripCandidateVectorPtr->size(); 
        idx++){
        CopyTripCandidateVector.push_back( 
            (*(completedBestCandidate->TripCandidateVectorPtr))[idx] );
    }

    completedBestCandidate->TripCandidateVectorPtr->clear();

    //vector<MatchedPoint*>* TripCandidateVectorPtr
    for(size_t idx = 0; idx < (CopyTripCandidateVector.size() - 1); idx++){
        MatchedPoint* actMatchPointPtr = CopyTripCandidateVector[idx];
        MatchedPoint* nextMatchPointPtr = CopyTripCandidateVector[idx+1];
        int regIdA = actMatchPointPtr->regId;
        int regIdB = nextMatchPointPtr->regId;
        
        completedBestCandidate->TripCandidateVectorPtr->push_back(
                                                    actMatchPointPtr);



        // Find shortest path
        vector<RegNode* >* shortestPathNodesVectorPtr = 
                                        new vector<RegNode* >;
        vector<vector<RegNode* >* > trialPathsVector;
        bool shortestPathFound = false;

        shortestPathFound = li->regionGraphPtr->FindShortestPath(
                    regIdA, regIdB, 3, shortestPathNodesVectorPtr, 
                    trialPathsVector);
        if(!shortestPathFound){
            shortestPathNodesVectorPtr->clear();
            trialPathsVector.clear();
            shortestPathFound = li->regionGraphPtr->FindShortestPath(
                    regIdA, regIdB, 5, shortestPathNodesVectorPtr, 
                    trialPathsVector);
        }
        if(!shortestPathFound){
            shortestPathNodesVectorPtr->clear();
            trialPathsVector.clear();
            shortestPathFound = li->regionGraphPtr->FindShortestPath(
                    regIdA, regIdB, 7, shortestPathNodesVectorPtr, 
                    trialPathsVector);
        }

        for(size_t interIdx = 1; 
            interIdx < (shortestPathNodesVectorPtr->size() - 1); 
            interIdx++){
            MatchedPoint* interPoint = new MatchedPoint(actMatchPointPtr);
            interPoint->regId = 
                    (*shortestPathNodesVectorPtr)[interIdx]->regId;

            completedBestCandidate->TripCandidateVectorPtr->push_back(
                                        interPoint);
        }
        
        if(idx == (CopyTripCandidateVector.size() - 2)){
            completedBestCandidate->TripCandidateVectorPtr->push_back(
                            nextMatchPointPtr);
        }

    } // end for(int idx = 0; idx < (CopyTripCan...

}


/*
Help function that creates possible matches for one point (from data).
PosDate, PosTime, latOrg, lonOrg, numOfSats, accuracy
li provides access to vector<vector<MatchedPoint* >* > candidates.

*/
static void FindPossibleMatches(GetNextRawTupleLI* li, 
                                MatchedPoint* actMatchedPoint){

    //cout << "   ** FindPossibleMatches called. \n";
// **START: Prepare geometry analysis (until query results are present) ***

    //int numOfTuples = li->raw_rel->GetNoTuples();
    //vector<vector<MatchedPoint* >* > candidates
    //Rectangle<2>* rectPtr = 0;

    double comfortZoneWidth = actMatchedPoint->accuracy * 2.5;
    if(comfortZoneWidth > 30.0){
        comfortZoneWidth = 30.0;
        //cout << "Max comfort zone is 30.0m" << endl;
    }

    double downLeftX;
    double upRightX;
                     
    double downLeftY;
    double upRightY;
    CalcRectCoords(comfortZoneWidth, comfortZoneWidth, 
                    actMatchedPoint->lonOrg, actMatchedPoint->latOrg, 
                    downLeftX, upRightX, downLeftY, upRightY);

    //cout << "Rectangle calculated: " << *rectPtr << endl;



    string rectValuesAsString = "const rect value (";
    ostringstream temp;
    temp.clear();
    temp << setprecision(9) << downLeftX << " " << upRightX << 
            " " << downLeftY << " " << upRightY;
    rectValuesAsString = "const rect value ( " + temp.str() + " )";


//  example: query RegionsFromNodes_RTree windowintersectsS[HochdahlerMarkt]
//                RegionsFromNodesH gettuples consume
    string* aQueryStringPtr = new string("query " + 
            *(li->graphNodesRTreeNamePtr) + " " + 
            *(li->graphNodesRelNamePtr)  + 
            " windowintersects[[ " + rectValuesAsString + "  ]] consume");

    //cout << "Current actRegId: " << actRegId << "\n";
    //cout << "*aQueryStringPtr: " << *aQueryStringPtr << "\n";
    //cout << "*actRectPtr: " << *actRectPtr << "\n";

    Relation* resultRelation = 0;
    //cout << "   ~~ FindPossibleMatches internal query prepared. \n";
    GetRelFromQueryString(aQueryStringPtr, resultRelation);
    //cout << "   ~~ FindPossibleMatches internal query result present. \n";

// *** END: Prepare geometry analysis (until query results are present) ***


// *** START: Initialization (if not done before) ***

    //vector<MatchedPoint*>* actTripVectorPtr
    //cout << "FindPossibleMatches, number of candidates: " << 
    // li->candidates.size();

    if( !li->initialized && li->candidates.size() == 0 ){
        // No candidates are available yet. Initialization starts.
        InitMHT(li, resultRelation, actMatchedPoint);

        // After initialization return (this point is processed)
        return;

    }

// ************ END: Initialization (if not done before) ************






// ************ START: MHT based map matching of following points *********

    MatchMHT(li, resultRelation, actMatchedPoint);


// ************ END: MHT based map matching of following points ***********

          
CandidateClipping(li, 15);


}

// *********************************************************************




/*
Help function that creates an multiple possible matching "paths" by 
using the multiple hypothesis technique. li provides access to 
vector<vector<MatchedPoint* >* > candidates.

*/
static void ApplyMHT(GetNextRawTupleLI* li){
    //cout << "  ** ApplyMHT called. \n";

    int numOfTuples = li->raw_rel->GetNoTuples();
    //cout << "Number of candidates: " << li->candidates.size();

    for(int i = 1; i<= numOfTuples; i++){
        // Get the next tuple.
        Tuple* tmpTuple = li->raw_rel->GetTuple(i,true);


        CcString* datePointer = static_cast<CcString*>(
                                    tmpTuple->GetAttribute(0));
        CcString* timePointer = static_cast<CcString*>(
                                    tmpTuple->GetAttribute(1));
                                    
        CcReal* latPointer = static_cast<CcReal*>(
                                    tmpTuple->GetAttribute(2));
        CcReal* longPointer = static_cast<CcReal*>(
                                    tmpTuple->GetAttribute(3));
                                    
        // Attr. 4 skipped
        CcReal* accuracyPointer = static_cast<CcReal*>(
                                    tmpTuple->GetAttribute(5));
                                    
        // Attr. 6 skipped
        CcInt* noOfSatsPointer = static_cast<CcInt*>(
                                    tmpTuple->GetAttribute(7));
        string* dateStringPtr = new string(datePointer->toText());
        string* timeStringPtr = new string(timePointer->toText());
        
        MatchedPoint* actMatchedPoint = new MatchedPoint(
                    *dateStringPtr, *timeStringPtr, latPointer->GetValue(), 
                    longPointer->GetValue(), noOfSatsPointer->GetIntval(),
                    accuracyPointer->GetValue() );

        //vector<MatchedPoint*>* actTripVectorPtr = new vector<MatchedPoint*>;



        FindPossibleMatches(li, actMatchedPoint);

    } 

    // Fill gaps between matched regions
    FillGapsBetweenMatchedRegions(li);

}

// *********************************************




/*
Help function that creates an internal representation of a region graph.
*/
static void CreateRegionGraph(GetNextRawTupleLI* li, 
string* edgesQueryStringPtr){

          // Execute query and get result relation
          Relation* edgesRelationPtr = 0;
          GetRelFromQueryString(edgesQueryStringPtr, edgesRelationPtr);

          RegGraph* aRegionGraphPtr = new RegGraph;

          //li->innerResultRelPtr = resultRelation;
          if(edgesRelationPtr != 0){
              // Rel present after query.
              int noOfEdgesInRel = edgesRelationPtr->GetNoTuples();


              for(int edgeIndex = 1; edgeIndex <= 
noOfEdgesInRel; edgeIndex++){
                  Tuple* edgeTuple = edgesRelationPtr->GetTuple(
edgeIndex,true);

                  CcInt* EdgeIdPtr = static_cast<CcInt*>(
edgeTuple->GetAttribute(0));
                  CcInt* SourceRegionIdPtr = static_cast<CcInt*>(
edgeTuple->GetAttribute(1));
                  CcInt* TargetRegionIdPtr = static_cast<CcInt*>(
edgeTuple->GetAttribute(2));
                  CcReal* DistancePtr = static_cast<CcReal*>(
edgeTuple->GetAttribute(3));

                  // Distance approximation (degrees in meters)
            //      double distInMeterApprx = 
   //DistancePtr->GetValue() * 111194.94034539;
                  //cout << "distInMeterApprx: " << distInMeterApprx << endl;

                  aRegionGraphPtr->addEdge(EdgeIdPtr->GetIntval(), 
SourceRegionIdPtr->GetIntval(), 
                                      TargetRegionIdPtr->GetIntval(),  
DistancePtr->GetValue());

              }

              li->regionGraphPtr = aRegionGraphPtr;

          }
          else{
              // No result for inner query, null pointer.
          }

}

// *************************************************************





// Pointer to an int that is used for indexing in a vector.
//int* callNrPtr;

Geoid* stdGeoid = new Geoid(true);
//Region actualRegion;


// **********************************************************



/*
2.1a Type mapping function for operator ~nodeRelToRegion~. A Node is 
expected as input.
The operator returns a region.
*/


ListExpr nodeRelToRegionTM(ListExpr args){
  //cout << "Arguments (nodeRelToRegionTM): " << nl->ToString(args) << "\n";
  //cout << "Type mapping nodeRelToRegionTM called." << "\n";

  // check number of arguments
  if(!nl->HasLength(args,1)){
    return listutils::typeError("wrong number of arguments");
  }
  // argument must be of type relation
  if(!Relation::checkType(nl->First(args)) ){
      return listutils::typeError(
"Relation with one tuple (Node) is expected.");
  }

  // return the result type
  return listutils::basicSymbol<Region>();

}


// *********************************************************************



/*
2.2a Value mapping function of operator ~nodeRelToRegion~
Relation with one tuple (from Nodes) (WayId: int, NodeCounter: int, 
NodeIdNew: int, Pos: point) expected where 
point is (double, double) with geographical coordinates (LON LAT).
*/

int nodeRelToRegionVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {

    // Prepare result storage
    result = qp->ResultStorage(s);
    Region* res = (Region*) result.addr;          // cast the result

    // Region variable for result from calculation in following
// call of calcSurroundingRegion(...)
    Region* realSurroundingRegion;
    Relation* nodeRelPtr = (Relation*) args[0].addr;
    string* waysRelNamePtr = 
                new string (((CcString*) args[1].addr)->toText());

    //int numOfTuples = nodeRelPtr->GetNoTuples();
    //cout << "Number of tuples: " << numOfTuples << "\n";
    Tuple* nodeTuplePtr = nodeRelPtr->GetTuple(1,true);
    //cout << "Given node tuple: " << *nodeTuplePtr << "\n";

    // Get the NodeIdNew that can be used to get way types for the 
//ways crossing the node (Region size depends on this).
    CcInt* nodeIdNewPtr = static_cast<CcInt*>(nodeTuplePtr->GetAttribute(2));

    // The vectors will be filled with type and names 
//from waytags with function GetWayTypesAndNamesForNodeIdNew
    std::vector<string> wayTypes = *new vector<string>;
    std::vector<string> wayNames = *new vector<string>;

    // Try to get way types (and names) for the ways crossing the actual node.
    try{
        GetWayTypesAndNamesForNodeIdNew(nodeIdNewPtr->GetIntval(), 
wayTypes, wayNames, waysRelNamePtr);
    } catch(const std::exception ex){ 
cout<< "Failed: GetWayTypesAndNamesForNodeIdNew" << "\n"; }

    // Standard width and height of crossing area will 
//be replaced based on way types, if possible.
    double width = * new double(8.0); 
// 8 meter wide crossing as default value
    double height = * new double(8.0); 
// 8 meter long crossing as default value
    int indexMostSignificant = * new int(0); 
// Index of most significant way with 'largest' way type, for 
// access to name & type

    if(wayTypes.size() > 0){
        // wayTypes were identified successfully in previous call of 
//GetWayTypesAndNamesForNodeIdNew.

        // Try to calculate the width and height depending on way types.
        try{
            DetermineCrossingDimensions(wayTypes, width, 
                                height,indexMostSignificant);
        } catch(const std::exception ex){ 
cout<< "Failed: DetermineCrossingDimensions" << "\n"; }
        //cout << "Width used: " << width << " , height: " << height << "\n";
             
    }

    // Get the point from the actual tuple. The region will be created
// around this point (which will be the center).
    Point attrPoint = *(static_cast<Point*>(nodeTuplePtr->GetAttribute(3)));
    Point* pointPtr = new Point(true, *new double(
attrPoint.GetX()), *new double(attrPoint.GetY()));
    //pointPtr->Set(true, pointPtr->GetX(), pointPtr->GetY());


    // Variables for results from calculation in following 
//call of calcSurroundingRegion(...)
    Point* pUpLeft;
    Point* pUpRight;
    Point* pDownRight;
    Point* pDownLeft;

    // Variables for results from calculation in following 
//call of calcSurroundingRegion(...)
    HalfSegment* upperHalfSeg;
    HalfSegment* rightHalfSeg;
    HalfSegment* lowerHalfSeg;
    HalfSegment* leftHalfSeg;

    // Calculation of the region that surrounds the crossing (node, point)
    calcSurroundingRegion(*pointPtr, pUpLeft, pUpRight, pDownRight, 
pDownLeft, upperHalfSeg, rightHalfSeg,
                           lowerHalfSeg, leftHalfSeg,
realSurroundingRegion, width, height);

    // Use calculated region as query result.
     *res = *realSurroundingRegion;


     return 0;

}




// *****************************************************************


/*
2.1b Type mapping function for operator ~pointToRegion~. 
A Point and a real width are expected as input.
The operator returns a region.
*/


ListExpr pointToRegionTM(ListExpr args){
  //cout << "Arguments (pointToRegionTM): " << nl->ToString(args) << "\n";
  //cout << "Type mapping pointToRegionTM called." << "\n";

  // check number of arguments
  if(!nl->HasLength(args,2)){
    return listutils::typeError("wrong number of arguments");
  }
  // 1st argument must be of type point. Second must be CcReal
  if(!Point::checkType(nl->First(args)) ||  
!CcReal::checkType(nl->Second(args)) ){
      return listutils::typeError(
"Point and real width value are expected.");
  }

  // return the result type
  return listutils::basicSymbol<Region>();

}


// *****************************************************




/*
2.2b Value mapping function of operator ~pointToRegion~
Point expected where a point is (double, double) with 
geographical coordinates (LON LAT).
*/

int pointToRegionVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {

    // Prepare result storage
    result = qp->ResultStorage(s);
    Region* res = (Region*) result.addr; 

    // Region variable for result from calculation in following 
//call of calcSurroundingRegion(...)
    Region* realSurroundingRegion;
    Point* inPointPtr = (Point*) args[0].addr;
    CcReal* inWidthPtr = (CcReal*) args[1].addr;

    //cout << "Given point (pointToRegion): " << *inPointPtr << "\n";
    //cout << "Given width (pointToRegion): " << *inWidthPtr << "\n";

    // Try to use width from parameter or use standard value.
    double usedWidth = 8.0;
    try{
        usedWidth = inWidthPtr->GetValue();
    }
    catch(const std::exception ex){ 
        cout<< "Failed: Width conversion for pointToRegion." << "\n";
        usedWidth = 8.0;
    }

    // Standard width and height of crossing area will 
//be replaced based on way types, if possible.
    double width = * new double(usedWidth);
    double height = * new double(usedWidth);

    // Variables for results from calculation in 
//following call of calcSurroundingRegion(...)
    Point* pUpLeft;
    Point* pUpRight;
    Point* pDownRight;
    Point* pDownLeft;

    // Variables for results from calculation in following 
//call of calcSurroundingRegion(...)
    HalfSegment* upperHalfSeg;
    HalfSegment* rightHalfSeg;
    HalfSegment* lowerHalfSeg;
    HalfSegment* leftHalfSeg;

    // Calculation of the region that surrounds the crossing (node, point)
    calcSurroundingRegion(*inPointPtr, pUpLeft, pUpRight, 
pDownRight, pDownLeft, upperHalfSeg, rightHalfSeg,
                           lowerHalfSeg, leftHalfSeg, 
realSurroundingRegion, width, height);

    // Use calculated region as query result.
     *res = *realSurroundingRegion;

     return 0;

}

























// *******************************************************


/*
2.1 Type mapping function for operator ~nodesToRegionNodes~. 
A Relation (Nodes) is expected as input.
The operator returns a stream of tuples with attributes:
WayId(int) NodeCounter(int) NodeIdNew(int) RegionId(int) Surrounding(Region).
*/


ListExpr nodesToRegionNodesTM(ListExpr args){


  // check number of arguments
  if(!nl->HasLength(args,3)){
    return listutils::typeError(
"Wrong number of arguments. Rel (Nodes) and int expected");
  }
  // argument must be of type relation
  if(!Relation::checkType(nl->First(args)) ||  
!CcInt::checkType(nl->Second(args)) ){
    return listutils::typeError(
"Relation (Nodes) and int (used as first RegionId) expected");
  }

  cout << "Result type (nodesToRegionNodesTM): " << 
nl->ToString(nl->First(args)) << "\n";

  // Create attribute type list.
  ListExpr tupleAttributeList = listutils::xElemList( 8,
              nl->TwoElemList( nl->SymbolAtom("WayId"), 
listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("SourceNodeIdNew"), 
listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("TargetNodeIdNew"), 
listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("Box"),
 listutils::basicSymbol<Rectangle<2> >() ),
              nl->TwoElemList( nl->SymbolAtom("RegionId"), 
listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("Surrounding"), 
listutils::basicSymbol<Region>() ),
              nl->TwoElemList( nl->SymbolAtom("RegionName"), 
listutils::basicSymbol<CcString>() ),
              nl->TwoElemList( nl->SymbolAtom("RegionType"), 
listutils::basicSymbol<CcString>() )
  );


  // Create the result type (stream Tuple) with the attribute 
//type list from above.
   ListExpr resultStreamTypeList = nl->TwoElemList(
        listutils::basicSymbol<Stream<Tuple> >(), 
    nl->TwoElemList( 
    listutils::basicSymbol<Tuple>(), 
    tupleAttributeList)
        );


   return resultStreamTypeList;
}


// ****************************************************



/*
2.2 Value mapping function of operator ~nodesToRegionNodes~
Relation with tuples (WayId: int, NodeCounter: int, NodeIdNew: int, 
Pos: point) expected where 
point is (double, double) with geographical coordinates (LON LAT).
*/

int nodesToRegionNodesVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {

    Region* realSurroundingRegion;


    // The pointer for the object of the LocalInfo class that
// holds the information which tuple comes next.
    GetNextNodeTupleLI* li = (GetNextNodeTupleLI*) local.addr;

    switch(message){
      case OPEN : 
      {
          // Open the stream.
          
          // Initialize first index (0) for vector.
          //callNrPtr = new int(0);

          if(li) 
          {
              delete li;
          }
          RegionMgmt::nodeRegions.clear();
          //cout << "Region vector cleared." << "\n";

          // Create a new LocalInfo (like iterator) object.
          local.addr = new GetNextNodeTupleLI( (Relation*) 
                args[0].addr);
          li = (GetNextNodeTupleLI*) local.addr;
          RegionMgmt::regionId = *(new int( ((CcInt*) 
                args[1].addr)->GetIntval() ));
          li->waysRelNamePtr = new string (((CcString*) 
                args[2].addr)->toText());
          //cout << "Way rel name: " << *(li->waysRelNamePtr) << endl;


          return 0;
      }
      return 0;
      
      case REQUEST:
      { 







          // Request next tuple.
          Tuple* tuple;

          // If Local info is available (which tuple number is next).
          if(li != 0){
              // Next tuple 
              tuple = li->getNext();
//              cout << "nodesToRegionNodesVM, Request, getNext()"<< "\n";
          }
          else
              return CANCEL;

          if(tuple == 0)
              return CANCEL;


          // Get the NodeIdNew that can be used to get way types for 
//the ways crossing the node (Region size depends on this).
          CcInt* nodeIdNewPointer = static_cast<CcInt*>(
                tuple->GetAttribute(2));

          // The vectors will be filled with type and names from waytags 
//with function GetWayTypesAndNamesForNodeIdNew
          std::vector<string> wayTypes = *new vector<string>;
          std::vector<string> wayNames = *new vector<string>;
          string mostSignificantWayName = "unknown_unknown";
          string mostSignificantWayType = "unknown";

          // Try to get way types (and names) for the ways 
//crossing the actual node.
          try{
              GetWayTypesAndNamesForNodeIdNew(
                    nodeIdNewPointer->GetIntval(), wayTypes, 
                    wayNames, li->waysRelNamePtr);
          } catch(const std::exception ex){ 
              cout<< "Failed: GetWayTypesAndNamesForNodeIdNew" << "\n"; }

          // Standard width and height of crossing area will be replaced 
//based on way types, if possible.
          double width = * new double(8.0); 
// 8 meter wide crossing as default value
          double height = * new double(8.0); 
// 8 meter long crossing as default value
          int indexMostSignificant = * new int(0); 
//with 'largest' way type, for access to name & type

//          double width = 8.0; // 8 meter wide crossing as default value
//          double height = 8.0; // 8 meter long crossing as default value

          if(wayTypes.size() > 0){
              // wayTypes were identified successfully in previous 
//call of GetWayTypesAndNamesForNodeIdNew.

              //cout << "wayTypes.size(): " << wayTypes.size() << "\n";
              //cout << "wayNames.size(): " << wayNames.size() << "\n";

              // Try to calculate the width and height depending on way types.
              try{
                  DetermineCrossingDimensions(
                        wayTypes, width, height, indexMostSignificant);
              } catch(const std::exception ex){ 
                  cout<< "Failed: DetermineCrossingDimensions" << "\n"; }
             

              // Try to get name of most significant way.
              try{
                  if((unsigned int)indexMostSignificant < wayNames.size()){
                      mostSignificantWayName = wayNames[indexMostSignificant];

                      if(indexMostSignificant > 0){
                          // Add second street name after "-"
                          mostSignificantWayName = 
                            mostSignificantWayName + "_" + wayNames[0];
                      }
                      else{
                          if(indexMostSignificant == 0 
                             && wayNames.size() > 1){
                              mostSignificantWayName = 
                                mostSignificantWayName + "_" + wayNames[1];
                          }
                          else if(wayNames.size() == 1){
                             mostSignificantWayName = 
                                wayNames[0] + "_" + wayNames[0]; 
                          }
                      }
                  }
                  else{
                      // Use last if index is too high 
//(there can be two types for ways, e.g. tertiary and tertiary_link) & not 0
                      if(wayNames.size() > 0){
                          mostSignificantWayName = 
                            wayNames[wayNames.size()-1] + "_" + wayNames[0];
                      }
                  }
              } catch(const std::exception ex){ 
                  cout<< "Failed to determine mostSignificantWayName." << 
                            "\n"; }



              // Try to get type of most significant way.
              try{
                  if((size_t)indexMostSignificant < wayTypes.size())
                      mostSignificantWayType = wayTypes[indexMostSignificant];
                  else{
                      // Should not happen, but avoids a crash 
//in case it does anyway.
                      if(wayNames.size() > 0){
                          mostSignificantWayType =wayTypes[wayTypes.size()-1];
                      }
                  }
              } catch(const std::exception ex){ 
                  cout<< "Failed to determine mostSignificantWayType." << 
                        "\n"; }

          }

          // Get the point from the actual tuple. 
//The region will be created around this point (which will be the center).
          Point attrPoint = *(static_cast<Point*>(
                tuple->GetAttribute(3)));
          Point* pointPointer = new Point(true, 
            *new double(attrPoint.GetX()), *new double(attrPoint.GetY()));



          // Variables for results from calculation in following
// call of calcSurroundingRegion(...)
          Point* pUpLeft;
          Point* pUpRight;
          Point* pDownRight;
          Point* pDownLeft;

          // Variables for results from calculation in 
//following call of calcSurroundingRegion(...)
          HalfSegment* upperHalfSeg;
          HalfSegment* rightHalfSeg;
          HalfSegment* lowerHalfSeg;
          HalfSegment* leftHalfSeg;

          // Region variable for result from calculation in 
//following call of calcSurroundingRegion(...)
          //Region* realSurroundingRegion;

          // Calc of the region that surrounds the crossing (node, point)
          calcSurroundingRegion(*pointPointer, pUpLeft, pUpRight, pDownRight, 
                pDownLeft, upperHalfSeg, rightHalfSeg, lowerHalfSeg, 
                leftHalfSeg, realSurroundingRegion, width, height);

     
          // Get the type of the result. It is supposed to be a tuple stream.
// Get the attribute type list from it (2nd step).
          ListExpr resultTypeA = GetTupleResultType( s );
          ListExpr attributeTypeList = nl->Second(nl->Second(resultTypeA));

          // Tuple type is constructed with attributeTypeList from supplier s
// (as defined in type mapping function).
          ListExpr tupleTypeInfoList = nl->TwoElemList(
            nl->SymbolAtom(Tuple::BasicType()), attributeTypeList);

          // Create a tuple with type info list (contains info, that it 
//is tuple and attribute list with spec data)
          Tuple* regionNodeTuplePointer = new Tuple(tupleTypeInfoList);

          // Attributes: WayId(int) NodeCounter(int) NodeIdNew(int) 
//Surrounding(region). First three taken from original.
          CcInt* wayIdPointer = static_cast<CcInt*>(tuple->GetAttribute(0));
          //CcInt* nodeCounterPointer = static_cast<CcInt*>(
          //  tuple->GetAttribute(1));

//          cout << "wayId      : " << *wayIdPointer << "\n";
//          cout << "nodeCounter: " << *nodeCounterPointer << "\n";
//          cout << "nodeIdNew  : " << *nodeIdNewPointer << "\n";
          

          // Store attributes in created tuple, first three from 
//original tuple, fourth is the calculated region.
          regionNodeTuplePointer->PutAttribute(0, new CcInt(true, 
            wayIdPointer->GetIntval())); // WayId
          regionNodeTuplePointer->PutAttribute(1, new CcInt(true, 
            nodeIdNewPointer->GetIntval())); // SourceNodeIdNew
          regionNodeTuplePointer->PutAttribute(2, new CcInt(true, 
            nodeIdNewPointer->GetIntval())); // TargetNodeIdNew

          //Rectangle<2> boundBox = realSurroundingRegion->BoundingBox();
          double minMax[] = {pDownLeft->GetX(), pUpRight->GetX(), 
                             pDownLeft->GetY(), pUpRight->GetY()};
          Rectangle<2>* boundBoxPtr = new Rectangle<2>(true,minMax);



          regionNodeTuplePointer->PutAttribute(3, boundBoxPtr); // Box

          regionNodeTuplePointer->PutAttribute(4, new CcInt(true, 
            RegionMgmt::regionId++)); // RegionId
          regionNodeTuplePointer->PutAttribute(5, new Region(
            *realSurroundingRegion, false)); // Surrounding, a region
          regionNodeTuplePointer->PutAttribute(6, new CcString(true, 
            mostSignificantWayName)); // RegionName
          regionNodeTuplePointer->PutAttribute(7, new CcString(true, 
            mostSignificantWayType)); // RegionType


          // Actual result tuple (update result address (to the new tuple))
          result.addr = regionNodeTuplePointer;

          // Add region to vector (RegionMgmt)
          RegionMgmt::nodeRegions.push_back(*realSurroundingRegion);
      


          // If not 0 return YIELD else CANCEL.
          if(result.addr != 0)
              return YIELD;
          else
              return CANCEL;

      }

      case CLOSE:
      {
          // CLOSE the stream.
          if(li){

              //RegionMgmt::PrintInfo();
              //RegionMgmt::StoreId();

              // li is no longer needed (stream ends)
              delete li;
              local.addr = 0;
          }
          return 0;
      }

    }

    return 0;

}


// ***********************************************************************

















































/*
2.1 Type mapping function for operator ~edgesToRegionNodesTM~. 
A Relation (Edges) is expected as input.
The operator returns a stream of tuples with attributes:
WayId(int) SourceNodeIdNew(int) TargetNodeIdNew(int) RegionId(int) 
SectionRegion(Region).
*/

ListExpr edgesToRegionNodesTM(ListExpr args){
  //cout << "Arguments (edgesToRegionNodesTM): "<< nl->ToString(args) << "\n";
  //cout << "Type mapping edgesToRegionNodesTM called." << "\n";

  // check number of arguments
  if(!nl->HasLength(args,2)){
    return listutils::typeError(
        "Wrong number of arguments. Relation (Edges) and int expected.");
  }
  // argument must be of type relation
  if(!Relation::checkType(nl->First(args)) ||  
     !CcInt::checkType(nl->Second(args)) ){
    return listutils::typeError(
        "Relation (Edges) and int (used as first RegionId) expected.");
  }




  // Create attribute type list.


  ListExpr tupleAttributeList = listutils::xElemList( 8,
              nl->TwoElemList( nl->SymbolAtom("WayId"), 
                listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("SourceNodeIdNew"), 
                listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("TargetNodeIdNew"), 
                listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("Box"), 
                listutils::basicSymbol<Rectangle<2> >() ),
              nl->TwoElemList( nl->SymbolAtom("RegionId"), 
                listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("Surrounding"), 
                listutils::basicSymbol<Region>() ),
              nl->TwoElemList( nl->SymbolAtom("RegionName"), 
                listutils::basicSymbol<CcString>() ),
              nl->TwoElemList( nl->SymbolAtom("RegionType"), 
                listutils::basicSymbol<CcString>() )
  );


  // Create the result type (stream Tuple) with the attribute type list 
//from above.
   ListExpr resultStreamTypeList = nl->TwoElemList(
               listutils::basicSymbol<Stream<Tuple> >(), 
           nl->TwoElemList( 
    listutils::basicSymbol<Tuple>(), 
    tupleAttributeList)
          );

   //cout << "ListExpr for result type created (edgesToRegionNodesTM)." 
//<< nl->ToString(resultStreamTypeList) << "\n";

   return resultStreamTypeList;
}


// *************************************************************************




/*
2.2 Value mapping function of operator ~edgesToRegionNodes~
Relation with tuples (Source:int Target:int SourcePos:Point 
TargetPos:Point SourceNodeCounter:int TargetNodeCounter:int 
Curve:Line RoadName:String RoadType:String WayId:int) expected where 
Point is (double, double) with geographical coordinates (LON LAT) 
and line is made of n Points.
*/

int edgesToRegionNodesVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {

    Region* rightSideWayRegionPointer;

    // The pointer for the object of the LocalInfo class that holds the 
//information which tuple comes next.
    GetNextEdgeTupleLI* li = (GetNextEdgeTupleLI*) local.addr;

    switch(message){
      case OPEN : 
      {
          // Open the stream.
          
          if(li) 
          {
              delete li;
          }

          RegionMgmt::nodeRegions.clear();
          //cout << "Region vector cleared." << "\n";

          // Create a new LocalInfo (like iterator) object.
          local.addr = new GetNextEdgeTupleLI( (Relation*) 
            args[0].addr);
          RegionMgmt::regionId = *(new int( 
            ((CcInt*) args[1].addr)->GetIntval() ));
          //cout << "edgesToRegionNodesVM, GetNextEdgeTupleLI Relation 
//cast in case OPEN."<< "\n";

          return 0;
      }
      return 0;
      
      case REQUEST:
      { 
          // Request next tuple.
          Tuple* tuple;

          // If Local info is available (with info which tuple number 
            //is next).
          if(li != 0){
              // Next tuple (the actual tuple for the following 
                //steps in this value mapping function)
              tuple = li->getNext();
              //cout << "edgesToRegionNodesVM, Request, getNext()"<< "\n";
          }
          else
              return CANCEL;

          if(tuple == 0)
              return CANCEL;


          // Get the NodeIdNew that can be used to get way types 
            //for the ways crossing the node (Region size depends on this).
          CcInt* sourceNodeIdNewPointer = static_cast<CcInt*>(
            tuple->GetAttribute(0));
          CcInt* targetNodeIdNewPointer = static_cast<CcInt*>(
            tuple->GetAttribute(1));

          //cout << "sourceNodeIdNew: " << sourceNodeIdNewPointer->
            //GetIntval() << " ";
          //cout << "targetNodeIdNew: " << targetNodeIdNewPointer->
            //GetIntval() << "\n";

          // The vectors will be filled with type and names from 
            //waytags with function GetWayTypesAndNamesForNodeIdNew
          CcString* wayNameCcStringPtr = static_cast<CcString*>(
            tuple->GetAttribute(7));
          CcString* wayTypeCcStringPtr = static_cast<CcString*>(
            tuple->GetAttribute(8));



          // Standard width will be replaced based on way types, if possible.
          double width = * new double(7.5); // 7.5 meter wide way as default

          // Way type defined?
          if(wayTypeCcStringPtr->IsDefined()){
              // wayType known.

              // Try to calculate the width depending on way type.
              try{
                  DetermineWayWidth(wayTypeCcStringPtr->toText(), width);
              } catch(const std::exception ex){ 
                cout<< "Failed: DetermineWayWidth" << "\n"; }
              //cout << "Width used: " << width  << "\n";
             
          }
      else{
              wayTypeCcStringPtr = new CcString(true, "unknown");
          }

          // Way name not defined.
          if(!wayNameCcStringPtr->IsDefined()){
              wayNameCcStringPtr = new CcString(true, "unknown");
          }



          // Get the Line from the actual tuple. The region will be created
            // on the right side of this Line.
          SimpleLine attrSimpleLine = *(static_cast<SimpleLine*>(
            tuple->GetAttribute(6)));
          //Point* pointPointer = new Point(true, 
            //*new double(attrPoint.GetX()), *new double(attrPoint.GetY()));

          //cout << "    SimpleLine in tuple isEmpty?: " << 
            //attrSimpleLine.IsEmpty() << "\n";
          //cout << "    SimpleLine in tuple size: " << 
            //attrSimpleLine.Size() << "\n";
          //cout << "    SimpleLine in tuple (edgesToRegionNodes): " << 
            //attrSimpleLine << "\n";

          // For the bounding box (is determined in method that 
            //creates the region for the actual way)
          double minBoundX = *new double(0.0);
          double maxBoundX = *new double(0.0);
          double minBoundY = *new double(0.0);
          double maxBoundY = *new double(0.0);

          // Calculation of the region on right side of the way.
          calcWayRegion(&attrSimpleLine, rightSideWayRegionPointer, width, 
                minBoundX, maxBoundX, minBoundY, maxBoundY);

          //cout <<  "Test2: Size of surrounding 8m x 8m Region: " <<
            // rightSideWayRegionPointer->Size() << "\n";     
          //cout <<  "Surrounding 8m x 8m Region: " << 
            //*rightSideWayRegionPointer << "\n";

          // Get the type of the result. It is supposed to be a tuple stream.
            //Get the attribute type list from it (2nd step).
          ListExpr resultTypeA = GetTupleResultType( s );
          ListExpr attributeTypeList = nl->Second(nl->Second(resultTypeA));

          // Tuple type is constructed with attributeTypeList from supplier s
            // (as defined in type mapping function).
          ListExpr tupleTypeInfoList = nl->TwoElemList(nl->SymbolAtom(
            Tuple::BasicType()), attributeTypeList);

          // Create a tuple with type info list (contains info, that it is 
//tuple and attribute list with spec data)
          Tuple* regionNodeTuplePointer = new Tuple(tupleTypeInfoList);



          // First three attributes are taken from the original edges tuple.
          CcInt* wayIdPointer = static_cast<CcInt*>(tuple->GetAttribute(9));

          // Store attributes in created tuple, first three from 
            //original tuple, fourth is the calculated region.
          regionNodeTuplePointer->PutAttribute(0, new CcInt(true, 
            wayIdPointer->GetIntval())); // WayId
          regionNodeTuplePointer->PutAttribute(1, new CcInt(true, 
            sourceNodeIdNewPointer->GetIntval())); // SourceNodeIdNew
          regionNodeTuplePointer->PutAttribute(2, new CcInt(true, 
            targetNodeIdNewPointer->GetIntval())); // TargetNodeIdNew

          // Bounding box (min and max values for both dimensions were 
//calculated together with the region.)
          double minMax[] = {minBoundX, maxBoundX, 
                             minBoundY, maxBoundY};
          Rectangle<2>* boundBoxPtr = new Rectangle<2>(true, minMax);
          regionNodeTuplePointer->PutAttribute(3, boundBoxPtr); // Box

          regionNodeTuplePointer->PutAttribute(4, new CcInt(true, 
            RegionMgmt::regionId++)); // RegionId
          regionNodeTuplePointer->PutAttribute(5, new Region(
            *rightSideWayRegionPointer, false)); // Surrounding, a region

          // Name and type from original tuple
          regionNodeTuplePointer->PutAttribute(6, new CcString(true, 
            wayNameCcStringPtr->toText())); // RegionName
          regionNodeTuplePointer->PutAttribute(7, new CcString(true, 
            wayTypeCcStringPtr->toText()) ); // RegionType



//          // Increase the region id after this id was used above for the 
//            //actual tuple / region.
//          RegionMgmt::IncreaseNextId();

          //cout <<  "Test4: Tuple created with " << regionNodeTuplePointer->
            //GetNoAttributes() << " attributes." << "\n";     

          // Actual result tuple (update result address (to the new tuple))
          result.addr = regionNodeTuplePointer;

          // Add region to vector (RegionMgmt)
          RegionMgmt::nodeRegions.push_back(*rightSideWayRegionPointer);


  
              
          //cout << "Region added: " << *rightSideWayRegionPointer << "\n";
          //cout << "In vector, region[" << *callNrPtr << "] : " << 
            //RegionMgmt::nodeRegions[*callNrPtr] << "\n";

          //cout << "Actual region: " << *rightSideWayRegionPointer << "\n";
          //cout << "Actual size of region vector: " << 
            //RegionMgmt::nodeRegions.size() << "\n";
          //cout << "Region nr. " << *callNrPtr << " : ";
          //cout << *RegionMgmt::nodeRegions[(*callNrPtr)++] << "\n";


          // If not 0 return YIELD else CANCEL.
          if(result.addr != 0)
              return YIELD;
          else
              return CANCEL;
      }

      case CLOSE:
      {
          // CLOSE the stream.
          if(li){

              //RegionMgmt::PrintInfo();
              //RegionMgmt::StoreId();

              // li is no longer needed (stream ends)
              delete li;
              local.addr = 0;
          }
          return 0;
      }

    }

    return 0;

}





// ****************************************************************




/*
2.1d Type mapping function for operator ~sLineToRegionTM~. 
A simple line is expected as input and real width value
for the region constructed from the line.
The operator returns a Region.
*/

ListExpr sLineToRegionTM(ListExpr args){
  //cout << "Arguments (sLineToRegionTM): " << nl->ToString(args) << "\n";
  //cout << "Type mapping sLineToRegionTM called." << "\n";

  // check number of arguments
  if(!nl->HasLength(args,2)){
    return listutils::typeError("wrong number of arguments");
  }
  // 1st argument must be of type point. Second must be CcReal
  if(!SimpleLine::checkType(nl->First(args)) ||  
!CcReal::checkType(nl->Second(args)) ){
      return listutils::typeError(
        "Simple line and a real value are expected.");
  }

  // return the result type
  return listutils::basicSymbol<Region>();
}



// *********************************************************




/*
2.2d Value mapping function of operator ~sLineToRegion~
Simple line and real width value are expected where a SimpleLine contains n 
Points (double, double) with geographical 
coordinates (LON LAT).
*/

int sLineToRegionVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {

    // Prepare result storage
    result = qp->ResultStorage(s);
    Region* res = (Region*) result.addr;          // cast the result

    // Region variable for result from calculation 
    Region* rightSideWayRegionPointer;
    SimpleLine* inSLinePtr = (SimpleLine*) args[0].addr;
    CcReal* inWidthPtr = (CcReal*) args[1].addr;

    //cout << "Given simple line (sLineToRegion): " << *inSLinePtr << "\n";
    //cout << "Given width (sLineToRegion): " << *inWidthPtr << "\n";

    // Try to use width from parameter or use standard value.
    double usedWidth = 8.0;
    try{
        usedWidth = inWidthPtr->GetValue();
    }
    catch(const std::exception ex){ 
        cout<< "Failed: Width conversion for sLineToRegion." << "\n";
        usedWidth = 8.0;
    }

    // Standard width of way area will be replaced based on way types, 
    //if possible.
    double width = * new double(usedWidth);






    double minBoundX = *new double(0.0);
    double maxBoundX = *new double(0.0);
    double minBoundY = *new double(0.0);
    double maxBoundY = *new double(0.0);

    // Calculation of the region on right side of the way.
    calcWayRegion(inSLinePtr, rightSideWayRegionPointer, width, 
            minBoundX, maxBoundX, minBoundY, maxBoundY);


//    // Calculation of the region on right side of the way.
//    calcWayRegion(inSLinePtr, rightSideWayRegionPointer, width);

    //cout <<  "Test2d: Size of Region from SimpleLine: " << 
    //rightSideWayRegionPointer->Size() << "\n";     
    //cout <<  "Region from SimpleLine: " << 
    //*rightSideWayRegionPointer << "\n";


    // Use calculated region as query result.
     *res = *rightSideWayRegionPointer;

     return 0;

}




// **********************************************************************


/*
2.1e Type mapping function for operator ~sLineRelToRegionTM~. 
A relation with one simple line is expected as input and real width 
value for the region constructed from the simple line.
The operator returns a Region.
*/

ListExpr sLineRelToRegionTM(ListExpr args){
  cout << "Arguments (sLineRelToRegionTM): " << nl->ToString(args) << "\n";
  cout << "Type mapping sLineRelToRegionTM called." << "\n";

  // check number of arguments
  if(!nl->HasLength(args,2)){
    return listutils::typeError("wrong number of arguments");
  }
  // 1st argument must be of type point. Second must be CcReal
  if(!Relation::checkType(nl->First(args)) ||  
     !CcReal::checkType(nl->Second(args)) ){
      return listutils::typeError(
        "Relation with one simple line tuple and a real value are expected.");
  }

  // return the result type
  return listutils::basicSymbol<Region>();
}




/*
2.2e Value mapping function of operator ~sLineRelToRegion~
Tuple with a simple line and a real width value are expected where a 
SimpleLine contains n Points (double, double) with geographical 
coordinates (LON LAT).
*/

int sLineRelToRegionVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {

    // Prepare result storage
    result = qp->ResultStorage(s);
    Region* res = (Region*) result.addr;          // cast the result

    // Region variable for result from calculation 
    Region* rightSideWayRegionPointer;

    // The parameters
    Relation* inSLineRelPtr = (Relation*) args[0].addr;
    CcReal* inWidthPtr = (CcReal*) args[1].addr;

    Tuple* tuple = inSLineRelPtr->GetTuple(1,true);
    //int numOfAttributes = tuple->GetNoAttributes();
    //cout << "    Number of attributes in current tuple:"<< 
    //numOfAttributes << "\n";

    // First attribute (index 0) is the simple line.
    SimpleLine* inSLinePtr = static_cast<SimpleLine*>(tuple->GetAttribute(0));

    //cout << "Given simple line (sLineRelToRegion): " << *inSLinePtr << "\n";
    //cout << "Given width (sLineRelToRegion): " << *inWidthPtr << "\n";

    // Try to use width from parameter or use standard value.
    double usedWidth = 8.0;
    try{
        usedWidth = inWidthPtr->GetValue();
    }
    catch(const std::exception ex){ 
        cout<< "Failed: Width conversion for sLineToRegion." << "\n";
        usedWidth = 8.0;
    }

    // Standard width of way area will be replaced based on way types, 
    //if possible.
    double width = * new double(usedWidth);




    double minBoundX = *new double(0.0);
    double maxBoundX = *new double(0.0);
    double minBoundY = *new double(0.0);
    double maxBoundY = *new double(0.0);

    // Calculation of the region on right side of the way.
    calcWayRegion(inSLinePtr, rightSideWayRegionPointer, width, minBoundX, 
        maxBoundX, minBoundY, maxBoundY);



//    // Calculation of the region on right side of the way.
//    calcWayRegion(inSLinePtr, rightSideWayRegionPointer, width);

    //cout <<  "Test2d: Size of Region from SimpleLine: " << 
    //rightSideWayRegionPointer->Size() << "\n";     
    //cout << "Region from SimpleLine: "<< *rightSideWayRegionPointer << "\n";


    // Use calculated region as query result.
     *res = *rightSideWayRegionPointer;

     return 0;

}




// ***********************************************************























// ***************************************************************


/*
2.1f Type mapping function for operator ~createEdgesForRegionNodes~. 
A Relation (with Region nodes) is expected as input.
The operator returns a stream of tuples with attributes:
EdgeId: int, SourceRegionId: int, TargetRegionId: int, Distance: int
*/


ListExpr createEdgesForRegionNodesTM(ListExpr args){
  //cout << "Arguments (createEdgesForRegionNodesTM): " << 
  //nl->ToString(args) << "\n";
  //cout << "Type mapping createEdgesForRegionNodesTM called." << "\n";

  // check number of arguments
  if(!nl->HasLength(args,5)){
    return listutils::typeError("Wrong number of arguments, 5 expected. ");
  }
  // argument must be of type relation
  if(!Relation::checkType(nl->First(args)) || 
     !CcInt::checkType(nl->Second(args)) || 
     !CcString::checkType(nl->Third(args)) || 
     !CcString::checkType(nl->Fourth(args)) || 
     !CcString::checkType(nl->Fifth(args)) ) {
    return listutils::typeError("Relation expected, id, str, str, str ");
  }

  // Create attribute type list.
  ListExpr tupleAttributeList = listutils::xElemList( 4,
              nl->TwoElemList( nl->SymbolAtom("EdgeId"), 
                listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("SourceRegionId"), 
                listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("TargetRegionId"), 
                listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("Distance"), 
                listutils::basicSymbol<CcReal>() )
  );


  // Create the result type (stream Tuple) with the attribute type list 
  //from above.
   ListExpr resultStreamTypeList = nl->TwoElemList(
               listutils::basicSymbol<Stream<Tuple> >(), 
           nl->TwoElemList( 
    listutils::basicSymbol<Tuple>(), 
    tupleAttributeList)
              );

   //cout << "ListExpr for result type created (createEdgesForRegionNodesTM)."
   // << nl->ToString(resultStreamTypeList) << "\n";

   return resultStreamTypeList;
}


// ****************************************************************



/*
2.2f Value mapping function of operator ~createEdgesForRegionNodes~
Relation with tuples expected containing source and target region ids. 
*/

int createEdgesForRegionNodesVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {

    // The pointer for the object of the LocalInfo class that 
//holds the information which tuple comes next.
    GetNextRegionNodeTupleLI* li = (GetNextRegionNodeTupleLI*) local.addr;

    switch(message){
      case OPEN : 
      {
          // Open the stream.
          
          if(li) 
          {
              delete li;
          }
          //RegionMgmt::nodeRegions.clear();

          // Create a new LocalInfo (like iterator) object.
          local.addr = new GetNextRegionNodeTupleLI( (Relation*) 
                        args[0].addr);
          li = (GetNextRegionNodeTupleLI*) local.addr;
          RegionMgmt::regionEdgeId = *(new int( ((CcInt*) 
                        args[1].addr)->GetIntval() ));

          // Example call:query hRegsAllNoOverlap 
            //createEdgesForRegionNodes[10000, "hRegsAllNoOverlap",
          // "hRegsAllNoOverlap_SrcId_BTree", 
            //"hRegsAllNoOverlap_TargetId_BTree"] count
          li->regionsRelNamePtr = new string( ((CcString*) 
            args[2].addr)->toText() );
          li->regionsSrcBTreeNamePtr = new string( ((CcString*) 
            args[3].addr)->toText() );
          li->regionsTargetBTreeNamePtr = new string( ((CcString*) 
            args[4].addr)->toText() );

          //cout << "*(li->regionsRelNamePtr)" << 
            //*(li->regionsRelNamePtr) << endl;
          //cout << "*(li->regionsSrcBTreeNamePtr)" << 
            //*(li->regionsSrcBTreeNamePtr) << endl;
          //cout << "*(li->regionsTargetBTreeNamePtr)" << 
            //*(li->regionsTargetBTreeNamePtr) << endl;
          
          return 0;
      }
      return 0;
      
      case REQUEST:
      { 
        // Request next tuple.
        Tuple* tuple;

        // If Local info is available (with info which tuple number is next).
        if(li != 0){
            if(li->loopCounter == 1){
                // Next tuple (the actual tuple for the following steps in 
                //this value mapping function) only if inner counter == 0.
                tuple = li->getNext();
            }
            else{
                // For the actual tuple results from a query with multiple 
                //tuples are processed further below (& edges created).
                tuple = li->getActual();
            }
        }
        else
            return CANCEL;






        // Get the type of the result. It is supposed to be a tuple stream. 
        //Get the attribute type list from it (2nd step).
        ListExpr resultTypeA = GetTupleResultType( s );
        ListExpr attributeTypeList = nl->Second(nl->Second(resultTypeA));

        // Tuple type is constructed with attributeTypeList from supplier s
        // (as defined in type mapping function).
        ListExpr tupleTypeInfoList = nl->TwoElemList(nl->SymbolAtom(
                    Tuple::BasicType()), attributeTypeList);

        // Create a tuple with type info list (contains info, that it is
        // tuple and attribute list with spec data)
        Tuple* regionEdgeTuplePointer = new Tuple(tupleTypeInfoList);

        // Pointers for attribute pointers of actual tuple.
        CcInt* srcNodeIdNewPointer = 0;
        CcInt* targetNodeIdNewPointer = 0;
        CcInt* regionIdPointerInner = 0;

        // Pointers for attribute pointers of inner tuple.
        CcInt* srcNodeIdNewPointerInner = 0;
        CcInt* targetNodeIdNewPointerInner = 0;
        CcInt* regionIdPointer = 0;

        // The bool is true at first and remains true as long as no 
        //inner result with identified edge is found.
        bool nextEdgeFound = false;

        while(!nextEdgeFound && tuple != 0){

            // Get the SourceNodeIdNew that can be used to get tuples
            // for nodes that are candidates for edge connections.
            srcNodeIdNewPointer = static_cast<CcInt*>(
                tuple->GetAttribute(1));

            // Get the TargetNodeIdNew that can be used to 
            //identify nodes for edge connections.
            targetNodeIdNewPointer = static_cast<CcInt*>(
                tuple->GetAttribute(2));

            // Get the RegionId that will be added for edge connections.
           regionIdPointer = static_cast<CcInt*>(tuple->GetAttribute(4));

           //cout << "srcNodeIdNew: " << srcNodeIdNewPointer->GetIntval() 
            //<< " targetNodeIdNew: " << targetNodeIdNewPointer->GetIntval()
           //     << " regionId: " << regionIdPointer->GetIntval() << endl;



            if(li->innerResultRelPtr == 0){
                // No innner result relation present. Create (query).

                li->loopCounter = 1;

                // Variables for query that provides candidates for edges.
                string srcNodeIdAsString;
                ostringstream temp;
                temp.clear();
                temp << *srcNodeIdNewPointer;
                srcNodeIdAsString = temp.str();

                string targetNodeIdAsString;
                ostringstream tempTarget;
                tempTarget.clear();
                tempTarget << *targetNodeIdNewPointer;
                targetNodeIdAsString = tempTarget.str();

                // Query for region based on Node. Replace if src and 
                //target ids are different. Below
                string inputQueryString = string("query " + 
                    *(li->regionsSrcBTreeNamePtr) + " exactmatchS[ " + 
                    srcNodeIdAsString + "] " + *(li->regionsRelNamePtr) +
                    " gettuples " + *(li->regionsTargetBTreeNamePtr) +
                    " exactmatchS[ " + srcNodeIdAsString + "] " + 
                    *(li->regionsRelNamePtr) + " gettuples concat consume");

                if(srcNodeIdNewPointer->GetIntval() != 
                    targetNodeIdNewPointer->GetIntval()){
                    // Outer region node is based on an edge from 
                    //relation Edges. Change query string accordingly.

                    inputQueryString = string("query " + *(
                        li->regionsTargetBTreeNamePtr) + " exactmatchS[ " + 
                        srcNodeIdAsString + "] " + *(li->regionsRelNamePtr) +
                        " gettuples consume feed filter" + 
                        "[.SourceNodeIdNew = " + targetNodeIdAsString + "] " 
                        + *(li->regionsSrcBTreeNamePtr) + " exactmatchS[" + 
                        srcNodeIdAsString + "] " + *(li->regionsRelNamePtr) +
                        " gettuples consume feed filter[.TargetNodeIdNew = "+
                        srcNodeIdAsString + "] concat " + 
                        *(li->regionsSrcBTreeNamePtr) + " exactmatchS[" + 
                        targetNodeIdAsString + "] " + 
                        *(li->regionsRelNamePtr) + 
                        " gettuples consume feed filter[.TargetNodeIdNew = " + 
                         targetNodeIdAsString + "] concat consume");
                }
 
                //cout << "Query string: " << inputQueryString << endl;
                string* inputQueryStringPtr = new string(inputQueryString);

                // Execute query and get result relation
                Relation* resultRelation = 0;
                GetRelFromQueryString(inputQueryStringPtr, resultRelation);

                li->innerResultRelPtr = resultRelation;
                if(resultRelation != 0){
                    // Rel present after query.
                }
                else{
                    // No result for inner query, null pointer.
                }

            } // end (innerResultRelPtr == 0)
            else{
                // Inner result rel exists.
            }


            if(li->innerResultRelPtr != 0){
                // Results available
                int numOfTuples = li->innerResultRelPtr->GetNoTuples();
                //cout << "    Number of tuples:"<< numOfTuples << "\n";

                while( !nextEdgeFound && li->loopCounter <= numOfTuples){
                    Tuple* tupleInner = li->innerResultRelPtr->
                        GetTuple(li->loopCounter,true);

                    // 'Inner' region node attr pointers
                    srcNodeIdNewPointerInner = static_cast<CcInt*>(
                        tupleInner->GetAttribute(1));
                        
                    targetNodeIdNewPointerInner = static_cast<CcInt*>(
                        tupleInner->GetAttribute(2));
                    regionIdPointerInner = static_cast<CcInt*>(
                        tupleInner->GetAttribute(4));
    
                    //cout << "srcNodeIdNewPointer: " << 
                    //srcNodeIdNewPointer->GetIntval();
                    //cout << "targetNodeIdNewPointer: " << 
                    //targetNodeIdNewPointer->GetIntval();
                    //cout << "srcNodeIdNewPointerInner: " << 
                    //srcNodeIdNewPointerInner->GetIntval();
                    //cout << "targetNodeIdNewPointerInner: " << 
                    //targetNodeIdNewPointerInner->GetIntval();

                    // Check if edge for region graph needed 
                    nextEdgeFound = IsEdgeAdequate(srcNodeIdNewPointer->
                        GetIntval(), targetNodeIdNewPointer->GetIntval(), 
                           srcNodeIdNewPointerInner-> GetIntval(), 
                           targetNodeIdNewPointerInner->GetIntval());
                    
                    li->loopCounter++;
                    //cout << "nextEdgeFound: " << nextEdgeFound << endl;

                } // end while( !nextEdgeFound && li->loopCount

                if(!nextEdgeFound){
                    // No more edges or no edges found for current 
                    // outer tuple. Next.
                    tuple = li->getNext();
                    li->loopCounter = 1;
 
                    // Reset result pointer
                    li->innerResultRelPtr = 0;


                    if(tuple != 0){
                        srcNodeIdNewPointer = static_cast<CcInt*>(
                            tuple->GetAttribute(1));
                        targetNodeIdNewPointer = static_cast<CcInt*>(
                            tuple->GetAttribute(2));
                        regionIdPointer = static_cast<CcInt*>(
                            tuple->GetAttribute(4));
                    }
                    //cout << "next tuple." << endl;

                }



            } // end if(li->innerResultRelPtr != 0)
            else{
                tuple = li->getNext();
                li->loopCounter = 1;

                // Reset result pointer
                li->innerResultRelPtr = 0;

                if(tuple != 0){
                    srcNodeIdNewPointer = static_cast<CcInt*>(
                        tuple->GetAttribute(1));
                    targetNodeIdNewPointer = static_cast<CcInt*>(
                        tuple->GetAttribute(2));
                    regionIdPointer = static_cast<CcInt*>(
                        tuple->GetAttribute(4));

                }
                //cout << "next tuple." << endl;
            }

        } // end while(!nextEdgeFound && tuple != 0)


        if(nextEdgeFound){
            // Edge identified.

            regionEdgeTuplePointer->PutAttribute(0, 
                new CcInt(true, RegionMgmt::regionEdgeId++)); // RegionEdgeId

            if(regionIdPointer != 0)
                regionEdgeTuplePointer->PutAttribute(1, 
                new CcInt(true, regionIdPointer->GetIntval())); //SrcRegionId
            else
                regionEdgeTuplePointer->PutAttribute(1, 
                new CcInt(true, 0)); // SourceRegionId

            if(regionIdPointerInner != 0)
                regionEdgeTuplePointer->PutAttribute(2, 
                    new CcInt(true, regionIdPointerInner->GetIntval())); 
                    // TargetRegionId
            else
                regionEdgeTuplePointer->PutAttribute(2, 
                    new CcInt(true, 0)); // TargetRegionId

            // For the distance different cases are possible.
            double actDistance = 0.0;

            if(srcNodeIdNewPointer == targetNodeIdNewPointerInner && 
                targetNodeIdNewPointer == srcNodeIdNewPointerInner){
                // Case 1: opposite sides of the street. Use a small value 
                //(just crossing the street).
                actDistance = 3.0;
            }
            else{
                // Case 2: Passing a region (street, crossing etc.)
                Rectangle<2>* actBox = static_cast<Rectangle<2>* >(
                    tuple->GetAttribute(3)); // Box
                    actDistance = actBox->Perimeter() / 2.0;


            }

            regionEdgeTuplePointer->PutAttribute(3, new CcReal(true,
                actDistance)); // Distance

            // Actual result tuple (update result address (to the new tuple))
            result.addr = regionEdgeTuplePointer;

            // Reset bool
            nextEdgeFound = false;

        }
        else{
            result.addr = 0;
        }

        // If not 0 return YIELD else CANCEL.
        if(result.addr != 0){
            //cout << "YIELD" << "\n";
            return YIELD;
        }
        else{
            //cout << "CANCEL" << "\n";
            return CANCEL;
        }

      } // case REQUEST

      case CLOSE:
      {
          // CLOSE the stream.
          if(li){

              // li is no longer needed (stream ends)
              delete li;
              local.addr = 0;
          }
          return 0;
      }

    }

    return 0;

}


// *******************************************************************









// ********************************************************************


/*
2.1g Type mapping function for operator ~mapMatchWalks~. 
Example usage:
query mapMatchWalks( Raw20160911 , "hRegsAllGraphEdges", 
"hRegsAllNoOverlap", "hRegsAllNoOverlap_RTree", 
"hRegsAllNoOverlap_RegId_BTree" ) count
*/


ListExpr mapMatchWalksTM(ListExpr args){

  // check number of arguments
  if(!nl->HasLength(args,5)){
    return listutils::typeError("Wrong number of arguments,five expected ");
  }
  // argument must be of type relation
  if( !Relation::checkType(nl->First(args)) || 
        !CcString::checkType(nl->Second(args)) || 
        !CcString::checkType(nl->Third(args)) || 
        !CcString::checkType(nl->Fourth(args)) || 
        !CcString::checkType(nl->Fifth(args)) ){
    return listutils::typeError(
        "Expected: Relation, string, string, string, string ");
  }

    NList argsNList(args);
    // Show nlist:
    //argsNList.writeAsStringTo(cout);


    NList relArgNL = argsNList.first();

    // Check details of relation.
    if (!listutils::isRelDescription(relArgNL.listExpr()))
        return listutils::typeError("Relation expected as first argument. ");

    NList tupleNL = relArgNL.second();
    if (!listutils::isTupleDescription(tupleNL.listExpr()))
        return listutils::typeError("No tuple as second in relation. ");

    NList attributesNL(tupleNL.second());
    if (!listutils::isAttrList(attributesNL.listExpr()))
        return listutils::typeError("Attribute list not correct. ");

    //attributesNL.writeAsStringTo(cout);


attributesNL.first().second().writeAsStringTo(cout);

if(!CcString::checkType(attributesNL.first().second().listExpr()))
   return listutils::typeError("First tuple attribute of relation not str");

if(!CcString::checkType(attributesNL.second().second().listExpr()))
   return listutils::typeError("Second tuple attribute of relation not str");

if(!CcReal::checkType(attributesNL.third().second().listExpr()))
   return listutils::typeError("Third tuple attribute of relation not str");

if(!CcReal::checkType(attributesNL.fourth().second().listExpr()))
   return listutils::typeError("Fourth tuple attribute of relation not str");

if(!CcReal::checkType(attributesNL.fifth().second().listExpr()))
   return listutils::typeError("Fifth tuple attribute of relation not str");

if(!CcReal::checkType(attributesNL.sixth().second().listExpr()))
   return listutils::typeError("Sixth tuple attribute of relation not str");

if(!CcReal::checkType(attributesNL.seventh().second().listExpr()))
   return listutils::typeError("Seventh tuple attribute of relation not str");

if(!CcReal::checkType(attributesNL.eigth().second().listExpr()))
   return listutils::typeError("Eigth tuple attribute of relation not str");

if(!CcInt::checkType(attributesNL.nineth().second().listExpr()))
   return listutils::typeError("Nineth tuple attribute of relation not str");

if(!CcString::checkType(attributesNL.tenth().second().listExpr()))
   return listutils::typeError("Tenth tuple attribute of relation not str");

if(!CcReal::checkType(attributesNL.eleventh().second().listExpr()))
   return listutils::typeError("Eleventh tuple attribute of rel not str");

if(!CcReal::checkType(attributesNL.twelfth().second().listExpr()))
   return listutils::typeError("Twelfth tuple attribute of rel not str");





  // Create attribute type list.
  ListExpr tupleAttributeList = listutils::xElemList( 5,
              nl->TwoElemList( nl->SymbolAtom("RegionId"), 
                listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("Time"), 
                listutils::basicSymbol<CcString>() ),
              nl->TwoElemList( nl->SymbolAtom("RegionName"), 
                listutils::basicSymbol<CcString>() ),
              nl->TwoElemList( nl->SymbolAtom("RegionType"), 
                listutils::basicSymbol<CcString>() ),
              nl->TwoElemList( nl->SymbolAtom("Pos"), 
                listutils::basicSymbol<Point>() )
  );


  // Create the result type (stream Tuple) with the attribute type list 
  //from above.
   ListExpr resultStreamTypeList = nl->TwoElemList(
                   listutils::basicSymbol<Stream<Tuple> >(), 
           nl->TwoElemList( 
    listutils::basicSymbol<Tuple>(), 
        tupleAttributeList)
          );


   return resultStreamTypeList;
}


// ************************************************************************





/*
2.2g Value mapping function of operator ~mapMatchWalks~
Available with GetNextRawTupleLI* li:
         graphEdgesRelNamePtr = edgesRelNamePtr;
         graphNodesRelNamePtr = nodesRelNamePtr;
         graphNodesRTreeNamePtr = nodesRTreeNamePtr;
         graphNodesRegIdBTreeNamePtr = nodesRegIdBTreeNamePtr;

*/

int mapMatchWalksVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {

    //// The pointer for the object of the LocalInfo class that holds the 
    //information which tuple comes next.
    GetNextRawTupleLI* li = (GetNextRawTupleLI*) local.addr;

    switch(message){
      case OPEN : 
      {
          // Open the stream.
          
          // Initialize first index (0) for vector.
          //callNrPtr = new int(0);

          if(li) 
          {
              delete li;
          }

          Relation* inRelPtr = (Relation*) args[0].addr;
          string* edgesNamePtr = new string( 
            ((CcString*) args[1].addr)->toText() );
          string* nodesNamePtr = new string( 
            ((CcString*) args[2].addr)->toText() );
          string* nodesRTreeNamePtr = new string( 
            ((CcString*) args[3].addr)->toText() );
          string* nodesRegIdBTreeNamePtr = new string( 
            ((CcString*) args[4].addr)->toText() );


          //cout << "   *** mapMatchWalksVM, OPEN " << *edgesNamePtr << endl;
          //cout << "*edgesNamePtr: " << *edgesNamePtr << endl;
          //cout << "*nodesNamePtr: " << *nodesNamePtr << endl;
          //cout << "*nodesRTreeNamePtr: " << *nodesRTreeNamePtr << endl;
          //cout << "*nodesRegIdBTreeNamePtr: " << 
          // *nodesRegIdBTreeNamePtr << endl;

          local.addr = new GetNextRawTupleLI( inRelPtr, edgesNamePtr, 
                nodesNamePtr, nodesRTreeNamePtr, nodesRegIdBTreeNamePtr);
          li = (GetNextRawTupleLI*) local.addr;

          // Read graph from relation and create internal representation
          string edgesQueryString = string("query " + 
                *(li->graphEdgesRelNamePtr) );

          //cout << "Query string: " << inputQueryString << endl;
          string* edgesQueryStringPtr = new string(edgesQueryString);

          CreateRegionGraph(li, edgesQueryStringPtr);
          ApplyMHT(li);

          return 0;
      }
      return 0;
      
      case REQUEST:
      { 
          // Request next matched point.
          //cout << "     ***** mapMatchWalksVM, REQUEST \n";

          MatchedPoint* matchedPoint;

          // If Local info is available (with info which matched point 
          //number is next).
          if(li != 0){
              // Next match point (the actual matched point for the 
          //following steps in this value mapping function)
              matchedPoint = li->getNextMatchedPoint();

          }
          else
              return CANCEL;

          if(matchedPoint == 0)
              return CANCEL;

          while(matchedPoint != 0 && matchedPoint->lonOrg == 0.0){
              matchedPoint = li->getNextMatchedPoint();
          }


          //cout << "matchedPoint->regId: " << matchedPoint->regId << endl;



          // Get the type of the result. It is supposed to be a tuple stream.
          // Get the attribute type list from it (2nd step).
          ListExpr resultTypeA = GetTupleResultType( s );
          ListExpr attributeTypeList = nl->Second(nl->Second(resultTypeA));

          // Tuple type is constructed with attributeTypeList from supplier s
          // (as defined in type mapping function).
          ListExpr tupleTypeInfoList = nl->TwoElemList(nl->SymbolAtom(
                Tuple::BasicType()), attributeTypeList);

          // Result tuple
          Tuple* resTuplePointer = new Tuple(tupleTypeInfoList);

          Point* orgPointPtr = new Point(true, matchedPoint->lonOrg, 
                matchedPoint->latOrg );
          //cout << "actual point: " << *orgPointPtr << endl;
          string datestring = matchedPoint->PosDate + "-" + 
                matchedPoint->PosTime;

          
 
          // Store attributes in created tuple, first three from 
          //original tuple, fourth is the calculated region.
          resTuplePointer->PutAttribute(0, new CcInt(true, 
            matchedPoint->regId));           // RegionId
          resTuplePointer->PutAttribute(1, new CcString(true, 
            datestring)); // dateAndTime
          resTuplePointer->PutAttribute(2, new CcString(true, 
            matchedPoint->RegionName));   // RegionName
          resTuplePointer->PutAttribute(3, new CcString(true, 
            matchedPoint->RegionType));   // RegionType
          resTuplePointer->PutAttribute(4, orgPointPtr);  // Pos

          // Actual result tuple (update result address (to the new tuple))
          result.addr = resTuplePointer;

          // If not 0 return YIELD else CANCEL.
          if(result.addr != 0)
              return YIELD;
          else
              return CANCEL;
      }

      case CLOSE:
      {

          //cout << "   *** mapMatchWalksVM, CLOSE \n";

          // CLOSE the stream.
          if(li){


              // li is no longer needed (stream ends)
              delete li;
              local.addr = 0;
          }
          return 0;
      }

    }

    return 0;

}





// ***************************************************************



/*
2.1h Type mapping function for operator ~removeOverlapping~. 
First argument is a relation that contains the tuples from which overlappings
should be removed from region attributes.
Second argument is the name of an RTree for that relation, a string.
Third parameter is the name of the relation the RTree refers to.
Fourth parameter is the RegionId of the last tuple that shall be updated. The
following tuples are only used to substract. 
This way more important regions like regions for nodes (crossings) are not 
reduced by edge regions if they are at the end with higher
RegionIds.
*/


ListExpr removeOverlappingTM(ListExpr args){
  //cout << "Arguments (removeOverlappingTM): " << nl->ToString(args)<< "\n";
  //cout << "Type mapping removeOverlappingTM called." << "\n";

  // check number of arguments
  if(!nl->HasLength(args,4)){
    return listutils::typeError("wrong number of arguments. Expected 4");
  }
  // argument must be of type relation
  if(!Relation::checkType(nl->First(args)) ||  
     !CcString::checkType(nl->Second(args)) ||  
     !CcString::checkType(nl->Third(args)) ||  
     !CcInt::checkType(nl->Fourth(args))){
    return listutils::typeError("relation expected, then string, string,int");
  }

  // Create attribute type list.
  ListExpr tupleAttributeList = listutils::xElemList( 8,
              nl->TwoElemList( nl->SymbolAtom("WayId"), 
                listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("SourceNodeIdNew"), 
                listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("TargetNodeIdNew"), 
                listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("Box"), 
                listutils::basicSymbol<Rectangle<2> >() ),
              nl->TwoElemList( nl->SymbolAtom("RegionId"), 
                listutils::basicSymbol<CcInt>() ),
              nl->TwoElemList( nl->SymbolAtom("Surrounding"), 
                listutils::basicSymbol<Region>() ),
              nl->TwoElemList( nl->SymbolAtom("RegionName"), 
                listutils::basicSymbol<CcString>() ),
              nl->TwoElemList( nl->SymbolAtom("RegionType"), 
                listutils::basicSymbol<CcString>() )
  );

  // Create the result type (stream Tuple) with the attribute type list 
  //from above.
   ListExpr resultStreamTypeList = nl->TwoElemList(
           listutils::basicSymbol<Stream<Tuple> >(), 
                       nl->TwoElemList( 
                   listutils::basicSymbol<Tuple>(), 
                tupleAttributeList)
        );

   //cout << "ListExpr for result type created (nodesToRegionNodesTM)." 
   //<< nl->ToString(resultStreamTypeList) << "\n";

   return resultStreamTypeList;
}


// *************************************************************************



/*
2.2h Value mapping function of operator ~removeOverlapping~

*/

int removeOverlappingVM (Word* args, Word& result, int message,
                         Word& local, Supplier s) {

    //Region* realSurroundingRegion;

    //// The pointer for the object of the LocalInfo class that holds the
    //information which tuple comes next.
    GetNextRegionNodeTupleLI* li = (GetNextRegionNodeTupleLI*) local.addr;

    switch(message){
      case OPEN : 
      {
          // Open the stream.
          
          // Initialize first index (0) for vector.
          //callNrPtr = new int(0);

          if(li) 
          {
              delete li;
          }

          local.addr= new GetNextRegionNodeTupleLI( (Relation*) args[0].addr);
          li = (GetNextRegionNodeTupleLI*) local.addr;


          ((GetNextRegionNodeTupleLI*) local.addr)->rTreeNamePtr = new string(
            ((CcString*) args[1].addr)->toText() );

          ((GetNextRegionNodeTupleLI*) local.addr)->relNamePtr = new string(
            ((CcString*) args[2].addr)->toText() );

          ((GetNextRegionNodeTupleLI*) local.addr)->endIndexPtr = new int( 
            ((CcInt*) args[3].addr)->GetIntval() );

          //cout << "Rtree name: " << *(li->rTreeNamePtr) << "\n";
          //cout << "startIndex: " << *(li->endIndexPtr) << "\n";

          return 0;
      }
      return 0;
      
      case REQUEST:
      { 
          // Request next tuple.
          Tuple* actTuple;

          // If Local info is available (with info which tuple number is next)
          if(li != 0){
              // Next tuple (is the actual tuple for the following steps 
                //in this value mapping function)
              actTuple = li->getNext();
          //cout << "A Rtree name: " << *(li->rTreeNamePtr) << "\n";
          //cout << "A Rtree name: " << *(li->relNamePtr) << "\n";
          //cout << "A startIndex: " << *(li->endIndexPtr) << "\n";

          }
          else
              return CANCEL;

          if(actTuple == 0)
              return CANCEL;

          // ----------------- Prepare result tuple
          // Get the type of the result. It is supposed to be a tuple stream. 
          //Get the attribute type list from it (2nd step).

          ListExpr resultTypeA = GetTupleResultType( s );
          ListExpr attributeTypeList = nl->Second(nl->Second(resultTypeA));

          // Tuple type is constructed with attributeTypeList from supplier s
          // (as defined in type mapping function).

          ListExpr tupleTypeInfoList = nl->TwoElemList(nl->SymbolAtom(
            Tuple::BasicType()), attributeTypeList);

          // ----------------- Attributes of current tuple
          // Get attributes from current tuple.
          CcInt* actWayIdPtr = static_cast<CcInt*>(actTuple->GetAttribute(0));
          CcInt* actSrcIdPtr = static_cast<CcInt*>(actTuple->GetAttribute(1));
          CcInt* actTargetIdPtr = static_cast<CcInt*>(actTuple->
                    GetAttribute(2));

          Rectangle<2>* actRectPtr = static_cast<Rectangle<2>*>(
            actTuple->GetAttribute(3));

          CcInt* actRegionIdPtr = static_cast<CcInt*>(actTuple->
            GetAttribute(4));

          int actRegId = actRegionIdPtr->GetIntval();

          //Region* actRegionPtr = new Region(*static_cast<Region*>(actTuple->
          //GetAttribute(5)), false);

          Region* actRegionPtr = static_cast<Region*>(actTuple->
            GetAttribute(5));
          CcString* actNamePtr = static_cast<CcString*>(actTuple->
            GetAttribute(6));
          CcString* actTypePtr = static_cast<CcString*>(actTuple->
            GetAttribute(7));



          //cout << "# *(li->endIndexPtr): " << *(li->endIndexPtr) << "\n";

          // Skip update tuples after endIndex and put them into the 
          //stream without changes.
          if(actTuple != 0 &&  actRegId > *(li->endIndexPtr)  ){

//              // Pass clone to stream, no update (id bigger than end id).
//              result.addr = actTuple->Clone();


              // Result tuple
              Tuple* resCopyTuplePointer = new Tuple(tupleTypeInfoList);
              resCopyTuplePointer->PutAttribute(0, new CcInt(
                actWayIdPtr-> GetIntval()) );     // WayId
              
              resCopyTuplePointer->PutAttribute(1, new CcInt(
                actSrcIdPtr-> GetIntval()) );     //SourceNodeIdNew  
            
              resCopyTuplePointer->PutAttribute(2, new CcInt(
                actTargetIdPtr-> GetIntval()));   //TargetNodeIdNew

              resCopyTuplePointer->PutAttribute(3, new Rectangle<2>(
                *actRectPtr) );            // Box

              resCopyTuplePointer->PutAttribute(4, new CcInt(
                actRegId) );                      // RegionId

              resCopyTuplePointer->PutAttribute(5, new Region(
                *actRegionPtr, false) );         // Surrounding

              resCopyTuplePointer->PutAttribute(6, new CcString(true, 
                actNamePtr-> toText())); // RegionName

              resCopyTuplePointer->PutAttribute(7, new CcString(true, 
                actTypePtr-> toText())); // RegionType

              // Actual result tuple(update result address(to the new tuple))
              result.addr = resCopyTuplePointer;



              // Copy to stream
              if(result.addr != 0){
                  //cout << "Original copied (unchanged) with 
                  //regId to stream: " << actRegId << "\n";
                  return YIELD;
              }
              else
                  return CANCEL;

          }

          //cout << "Current actRegId: " << actRegId << "\n";
          if(actTuple == 0)
              return CANCEL;

          // ----------------- Update attributes of current tuple
          // Get attributes from current tuple.
          actWayIdPtr = static_cast<CcInt*>(actTuple->GetAttribute(0));
          actRectPtr = static_cast<Rectangle<2>*>(actTuple->GetAttribute(3));

          //Region* actRegionPtr = new Region(*static_cast<Region*>(
          //actTuple->GetAttribute(5)), false);
          actRegionPtr = static_cast<Region*>(actTuple->GetAttribute(5));
          actNamePtr = static_cast<CcString*>(actTuple->GetAttribute(6));
          actTypePtr = static_cast<CcString*>(actTuple->GetAttribute(7));


          //cout << "Rtree name: " << *(li->rTreeNamePtr) << "\n";
          //cout << "startIndex: " << *(li->endIndexPtr) << "\n";

          // Construct string for rect. The string is used in query below.
          string rectValuesAsString = "const rect value (";
          ostringstream temp;
          temp.clear();
          temp << setprecision(9) << actRectPtr->MinD(0) << " " << 
            actRectPtr->MaxD(0) << " " << 
            actRectPtr->MinD(1) << " " << actRectPtr->MaxD(1);
            rectValuesAsString = "const rect value ( " + temp.str() + " )";

        //        example: query RegionsFromNodes_RTree windowintersectsS
        //[HochdahlerMarkt] RegionsFromNodesH gettuples consume
          string* aQueryStringPtr = new string("query " + 
            *(li->rTreeNamePtr) + 
            " " + *(li->relNamePtr)  + " windowintersects[[ " + 
            rectValuesAsString + "  ]] consume");

          //cout << "Current actRegId: " << actRegId << "\n";
          //cout << "*aQueryStringPtr: " << *aQueryStringPtr << "\n";
          //cout << "*actRectPtr: " << *actRectPtr << "\n";

          Relation* resultRelation = 0;
          GetRelFromQueryString(aQueryStringPtr, resultRelation);
          
          if(resultRelation != 0){
              // Results available
              int numOfTuples = resultRelation->GetNoTuples();
              //cout << "    Number of tuples:"<< numOfTuples << "\n";

              // Pointers for result regions of minus / union operation.
              Region* resultAfterMinusReg = 0;
              //Region* resultAfterUnionReg = 0;

              for(int i = 1; i <= numOfTuples; i++){
                  //resultAfterUnionReg = new Region(*actRegionPtr, false);
                  Tuple* tupleInner = resultRelation->GetTuple(i,true);

                  int numOfAttributes = tupleInner->GetNoAttributes();
                  //cout << "    Number of attributes in inner result 
                  //tuple:"<< numOfAttributes << "\n";

                  if(numOfAttributes < 8){
                      cout << "Result tuple of inner query has only " << 
                        numOfTuples << " attributes (Exp: 8)." << "\n";
                      break;
                  }

                  //CcInt* wayIdPtr = static_cast<CcInt*>(
                  //  tupleInner->GetAttribute(0));
                  CcInt* regionIdPtr = static_cast<CcInt*>(
                    tupleInner->GetAttribute(4));

                  //Region* regionPtr = new Region(*static_cast<Region*>(
                     //tupleInner->GetAttribute(5)), false);
                  Region* regionPtr = 
                    static_cast<Region*>(tupleInner->GetAttribute(5));
                  int innerRegId = regionIdPtr->GetIntval();

                  //CcString* namePtr = 
                  //  static_cast<CcString*>(tupleInner->GetAttribute(6));
                  //CcString* typePtr = 
                  //  static_cast<CcString*>(tupleInner->GetAttribute(7));

                  //cout << "actRegionPtr: " << actRegionPtr << "\n";
                  //cout << "regionPtr: " << regionPtr << "\n";
                  //cout << "innerRegId: " << innerRegId << " , 
                     //actRegId: " << actRegId << "\n";

                  // Important: innerRegId > actRegId(otherwise minus for 
                  // R1 - R2 and other way R2 - R1. )
                if(actRegionPtr !=0 && regionPtr != 0 && innerRegId >actRegId
                     && actRegionPtr->Area(0) > 0.0 &&
                     regionPtr->Area(0) > 0.0 && 
                     actRegionPtr->Overlaps(*regionPtr)){
                         
                      // Intersection with valid regions found.

                      //cout << "Intersection found." << "\n";
                      //cout << "*actRegionPtr: " << *actRegionPtr << "\n";
                      //cout << "*regionPtr: " << *regionPtr << "\n";


                      // first new (innerRegId == 11528 && actRegId == 10664)

                      // Invalid pairs are excluded
                      if( !(innerRegId == 11118 && actRegId == 11114) 
                          &&!(innerRegId == 10690 && actRegId == 10688)
                          && !(innerRegId == 10769 && actRegId == 10763) 
                          && !(innerRegId == 10834 && actRegId == 10829) && 
                          !(innerRegId == 10905 && actRegId == 10881) 
                          && !(innerRegId == 11454 && actRegId == 11064) &&
                          !(innerRegId == 11594 && actRegId == 11065) 
                          && !(innerRegId == 11589 && actRegId == 11067) &&
                          !(innerRegId == 11068 && actRegId == 11067) 
                          && !(innerRegId == 11203 && actRegId == 11152) &&
                          !(innerRegId == 11203 && actRegId == 11199) 
                          && !(innerRegId == 11533 && actRegId == 11199) &&
                          !(innerRegId == 11528 && actRegId == 10664) 
                          && !(innerRegId == 11085 && actRegId == 11081)
                          ){

                          //cout << "Updating for innerRegId: " << 
                          //innerRegId << ",actRegId: " << actRegId << "\n";

                          // Reduce actual region from tuple (outer) 
                                       //(substract overlapping).
                          resultAfterMinusReg = 
                                  new Region(*actRegionPtr, false);
                          actRegionPtr->Minus(*regionPtr, 
                                              *resultAfterMinusReg, 0);

                      }

                      if(resultAfterMinusReg != 0){
                          //cout << "Region updated" << "\n";
                          actRegionPtr = new Region(*resultAfterMinusReg, 
                            false);
                      }
                      else{
                          // don't update
                          actRegionPtr = new Region(0);
                      }

                  }// end if(actRegionPtr !=0...
                  else{
                      // Not overlapping or one region is empty

                  }

                  // Old region could be deleted before new one is assigned
                  //actRegionPtr->DeleteIfAllowed();

                  // Smaller region is the new actual for next stream result
                  // (but can still be reduced in this for loop).


              } // end for(int i = 1; i <= numOfTuples; i++)
          } // end if(resultRelation != 0)
          else{
              //cout << "### resultRelation == 0" << "\n";
          }

          // Result tuple
          Tuple* resTuplePointer = new Tuple(tupleTypeInfoList);
          resTuplePointer->PutAttribute(
            0, new CcInt(actWayIdPtr-> GetIntval()) );     // WayId
          resTuplePointer->PutAttribute(
            1, new CcInt(static_cast<CcInt*>(
                actTuple->GetAttribute(1))-> GetIntval()));
          resTuplePointer->PutAttribute(
            2, new CcInt(static_cast<CcInt*>(
                actTuple->GetAttribute(2))-> GetIntval())); 
          resTuplePointer->PutAttribute(
            3, new Rectangle<2>(*actRectPtr) );            // Box

          resTuplePointer->PutAttribute(
            4, new CcInt(actRegId) );                      // RegionId
          resTuplePointer->PutAttribute(
            5, new Region(*actRegionPtr, false) );         // Surrounding
          resTuplePointer->PutAttribute(
            6, new CcString(true, actNamePtr-> toText())); // RegionName
          resTuplePointer->PutAttribute(
            7, new CcString(true, actTypePtr-> toText())); // RegionType

          // Actual result tuple (update result address (to the new tuple))
          result.addr = resTuplePointer;

          // If not 0 return YIELD else CANCEL.
          if(result.addr != 0)
              return YIELD;
          else
              return CANCEL;
      }

      case CLOSE:
      {
          // CLOSE the stream.
          if(li){


              // li is no longer needed (stream ends)
              delete li;
              local.addr = 0;
          }
          return 0;
      }

    }

    return 0;

}


// ************************************************














































/*
2.3a Specification of ~nodeRelToRegion~
*/
 OperatorSpec nodeRelToRegionSpec(
       " Tuple(Node)  -> Region",
       " _ nodeRelToRegion",
       " Creates a surrounding region for the point of a node",
       "query Nodes feed head[1] nodeRelToRegion"
  );


/*
2.3b Specification of ~pointToRegion~
*/
 OperatorSpec pointToRegionSpec(
      " Point x real -> Region",
      " _ pointToRegion _",
      " Creates a surrounding region for the point with width of 2nd",
      "query Nodes feed head[1] extend[Reg: .Pos pointToRegion [5.7]] consume"
  );


/*
2.3 Specification of ~nodesToRegionNodes~
*/
 OperatorSpec nodesToRegionNodesSpec(
      " Rel(Tuple(Node)) x int x string  -> stream(Tuple(RegionNode))",
      " _ nodesToRegionNodes[ _ , _ v",
      " Creates a surrounding region for the point of a node.",
      "query Nodes feed head[1] consume nodesToRegionNodes[1, \"Ways\"] count"
  );


/*
2.3c Specification of ~edgesToRegionNodes~
*/
 OperatorSpec edgesToRegionNodesSpec(
       " Rel(Tuple(Edge)) x int  -> stream(Tuple(RegionNode))",
       " _ edgesToRegionNodes[ _ ]",
       " Creates a region for crossing points for example.",
       "query Edges feed head[1] consume edgesToRegionNodes[1] count"
  );


/*
2.3d Specification of ~sLineToRegion~
*/
 OperatorSpec sLineToRegionSpec(
       " SimpleLine -> Region",
       " _ sLineToRegion [_]",
       " Creates a region for a simple line with specified width ",
    "query Edges feed head[1] extend[Reg: .Curve sLineToRegion [5.7]] consume"
  );



/*
2.3e Specification of ~sLineToRegion~
*/
 OperatorSpec sLineRelToRegionSpec(
    " Relation with one tuple that has one attribute, a SimpleLine -> Region",
       " _ sLineRelToRegion [_]",
     " Creates a region for a relation with one tuple ",
   "query Edges feed head[1] project[Curve] consume sLineRelToRegion [8.0]"
  );


/*
2.3f Specification of ~createEdgesForRegionNodes~
*/
 OperatorSpec createEdgesForRegionNodesSpec(
" Rel(Tuple()) x int x string x string x string  -> stream(Tuple())",
" _ createEdgesForRegionNodes[ _ , _ , _ , _ ]",
" Creates edges between region nodes (region graph). ",
"query Reg createEdgesForRegionNodes[1, \"Reg\", \"S_BTr\", \"T_BTr\"] count"
  );


/*
2.3g Specification of ~mapMatchWalks~
*/
 OperatorSpec mapMatchWalksSpec(
" Rel(Tuple(...)) x string x string x string x string  -> stream(Tuple(...))",
       " mapMatchWalks( _ , _ , _ , _ , _ )",
" Creates a tuple stream with results from map matching.",
"query mapMatchWalks( Rw , \"re\", \"r\", \"r_R\", \"R_BT\" ) count"
  );


/*
2.3h Specification of ~removeOverlapping~
*/
 OperatorSpec removeOverlappingSpec(
" Rel(Tuple(...)) x string x string x int  -> stream(Tuple(...))",
" removeOverlapping( _ , _ ,  _ , _ )",
" Creates a tuple stream with updated regions (Overlapping parts removed).",
"query removeOverlapping( Re, \"Re_RTree\", \"Re\",  1 ) count"
  );





// *****************************************************



/*
2.4a Operator Instance for ~nodeRelToRegion~
*/
Operator nodeRelToRegionOp(
   "nodeRelToRegion",
   nodeRelToRegionSpec.getStr(),
   nodeRelToRegionVM,
   Operator::SimpleSelect,
   nodeRelToRegionTM
);


/*
2.4b Operator Instance for ~pointToRegion~
*/
Operator pointToRegionOp(
   "pointToRegion",
   pointToRegionSpec.getStr(),
   pointToRegionVM,
   Operator::SimpleSelect,
   pointToRegionTM
);


/*
2.4 Operator Instance for ~nodesToRegionNodes~
*/
Operator nodesToRegionNodesOp(
   "nodesToRegionNodes",
   nodesToRegionNodesSpec.getStr(),
   nodesToRegionNodesVM,
   Operator::SimpleSelect,
   nodesToRegionNodesTM
);


/*
2.4c Operator Instance for ~edgesToRegionNodes~
*/
Operator edgesToRegionNodesOp(
   "edgesToRegionNodes",
   edgesToRegionNodesSpec.getStr(),
   edgesToRegionNodesVM,
   Operator::SimpleSelect,
   edgesToRegionNodesTM
);



/*
2.4d Operator Instance for ~sLineToRegion~
*/
Operator sLineToRegionOp(
   "sLineToRegion",
   sLineToRegionSpec.getStr(),
   sLineToRegionVM,
   Operator::SimpleSelect,
   sLineToRegionTM
);



/*
2.4e Operator Instance for ~sLineRelToRegion~
*/
Operator sLineRelToRegionOp(
   "sLineRelToRegion",
   sLineRelToRegionSpec.getStr(),
   sLineRelToRegionVM,
   Operator::SimpleSelect,
   sLineRelToRegionTM
);



/*
2.4f Operator Instance for ~createEdgesForRegionNodes~
*/
Operator createEdgesForRegionNodesOp(
   "createEdgesForRegionNodes",
   createEdgesForRegionNodesSpec.getStr(),
   createEdgesForRegionNodesVM,
   Operator::SimpleSelect,
   createEdgesForRegionNodesTM
);



/*
2.4g Operator Instance for ~mapMatchWalks~
*/
Operator mapMatchWalksOp(
   "mapMatchWalks",
   mapMatchWalksSpec.getStr(),
   mapMatchWalksVM,
   Operator::SimpleSelect,
   mapMatchWalksTM
);



/*
2.4h Operator Instance for ~removeOverlapping~
*/
Operator removeOverlappingOp(
   "removeOverlapping",
   removeOverlappingSpec.getStr(),
   removeOverlappingVM,
   Operator::SimpleSelect,
   removeOverlappingTM
);



// *********************************************************


/*
4 Class for ~MapMatchingPedestriansAlgebra~ with the 
operators for map matching with focus on pedestrians.
*/

class MapMatchingPAlgebra : public Algebra
{
  public: 
//      static vector<Region*> nodeRegions;
//      static vector<int> nodeIdNewValues;

      static void calculate(double x1, double y1, double x2, 
              double y2, double moveDist, 
              double &movedX1, double &movedY1, 
              double &movedX2, double &movedY2);


  public:
    MapMatchingPAlgebra() : Algebra()
    {

      //AddOperator(&nodeRelToRegionOp);
      AddOperator(&pointToRegionOp);
      AddOperator(&nodesToRegionNodesOp);
      AddOperator(&edgesToRegionNodesOp);
      AddOperator(&sLineToRegionOp);
      //AddOperator(&sLineRelToRegionOp);
      AddOperator(&createEdgesForRegionNodesOp);
      AddOperator(&mapMatchWalksOp);
      AddOperator(&removeOverlappingOp);


    }
    ~MapMatchingPAlgebra() {}





};

MapMatchingPAlgebra mapMatchingPAlgebra;

} // end namespace ~mapMatchPed~ 

/*

5 Initialization

*/

extern "C"
Algebra*
InitializeMapMatchingPAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return new mapMatchPed::MapMatchingPAlgebra;
}


