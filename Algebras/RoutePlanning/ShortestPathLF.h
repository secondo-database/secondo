/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#ifndef SECONDO_SHORTESTPATHLF_H
#define SECONDO_SHORTESTPATHLF_H

#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Algebras/OrderedRelation/OrderedRelationAlgebra.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/RTree/RTreeAlgebra.h"
#include "Algebras/LineFunction/LineFunctionAlgebra.h"
#include "Algebras/Collection/CollectionAlgebra.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"


#include <AlgebraTypes.h>
#include <Operator.h>

// #define DEMO_MODE

namespace routeplanningalgebra {
        
    /*
    1. Class definitions
    */
    
    /*
    Node objects are used to store information about nodes in graph
    */
    class Node {
         public:
            //Constructor, destructor
                
            Node(){}
             
            Node(const int _nodeID, const int _beID, 
                 const double _dist, const double _prio): 
                nodeID(_nodeID), beforeNodeID(_beID),
                distFromStart(_dist), prioValue(_prio){}
                
            //copy constructor
            Node(const Node* nd){
                nodeID = nd->nodeID;
                beforeNodeID = nd->beforeNodeID;
                distFromStart = nd->distFromStart;
                prioValue = nd->prioValue;}
             
            ~Node(){}
            
            void update(int beID, double dist, double prio){
                beforeNodeID = beID;
                distFromStart = dist;
                prioValue = prio;}
            
            //Getters and setters 
            int getNodeID(){
                return nodeID;}     
            void setNodeID(int i){
                nodeID = i;}
            int getBeforeNodeID(){
                return beforeNodeID;}
            void setBeforeNodeID(int i){
                beforeNodeID = i;}
            double getDistFromStart(){
                return distFromStart;}     
            void setDistFromStart(double d){
                distFromStart = d;}
            double getPrioValue(){
                return prioValue;}     
            void setPrioValue(double d){
                prioValue = d;}    
            
         private:
            int nodeID;
            int beforeNodeID;
            double distFromStart;
            double prioValue;
    };
    
    struct NodeTreeEntry{
            Node* node;
            NodeTreeEntry* left;
            NodeTreeEntry* right;

            //Constructor
            NodeTreeEntry(Node* nd):
            node(nd), left(NULL), right(NULL){}
            
            
            //Destructor
            ~NodeTreeEntry(){} 
    };  

    
    class NodeTree {
         public:
            //Constructor, destructor
            NodeTree(): root(NULL){}
            
            ~NodeTree(){
                destroy(root);
            }
            
            bool isEmpty(){
                if(root == NULL) return true;
                return false;
            }
            
            void insert(Node* nd){
                root = insert(root, nd);
            }
            
            void deleteNode(Node* nd){
                root = deleteNode(root, nd);
            }

            Node* find(int id){
                NodeTreeEntry* nt = find(root, id);
                if(nt != NULL) return nt->node;
                return NULL;
            }
 
            Node* findMin(){                        
                Node* nd = findMinPrioValue(root);
                return nd;
            }
                    
         private:
            NodeTreeEntry* root;
            
            //deletes all nodes from tree
            void destroy(NodeTreeEntry* rt){
                if(rt != NULL){
                    destroy(rt->left);
                    destroy(rt->right);
                    delete rt->node;
                    delete rt;}
            }

            //finds NodeTreeEntry* in binary tree, or returns NULL
            NodeTreeEntry* find(NodeTreeEntry* rt, int id){
                if(rt == NULL) 
                        return NULL;
                else if(id < rt->node->getNodeID())
                        return find(rt->left, id);
                else if(id > rt->node->getNodeID())
                        return find(rt->right, id);
                else return rt;
            }

            //returns binary tree with inserted node
            NodeTreeEntry* insert(NodeTreeEntry* rt, Node* nd){
                if(rt == NULL){ //binary tree is empty or it's a terminal node
                    rt = new NodeTreeEntry(nd);}
                else if(nd->getNodeID() < rt->node->getNodeID())
                    rt->left = insert(rt->left, nd);
                else if(nd->getNodeID() > rt->node->getNodeID())
                    rt->right = insert(rt->right, nd);
                return rt;
            }

            //returns node with minimal ID
            NodeTreeEntry* findMinID(NodeTreeEntry* rt){
                while(rt->left != NULL) rt = rt->left;
                return rt;
            }

            //returns binary tree without deleted node
            NodeTreeEntry* deleteNode(NodeTreeEntry* rt, Node* nd){
                if(rt == NULL) return rt;
                else if(nd->getNodeID() < rt->node->getNodeID())
                    rt->left = deleteNode(rt->left, nd);
                else if(nd->getNodeID() > rt->node->getNodeID())
                    rt->right = deleteNode(rt->right, nd);
                else{
                    //no child
                    if(rt->left == NULL && rt->right == NULL){
                        delete rt->node;
                        delete rt;
                        rt = NULL;}
                    //one child
                    else if(rt->left == NULL){
                        NodeTreeEntry* temp = rt;
                        rt = rt->right;
                        delete temp->node;
                        delete temp;}
                    else if(rt->right == NULL){
                        NodeTreeEntry* temp = rt;
                        rt = rt->left;
                        delete temp->node;
                        delete temp;}
                    //two children
                    else{
                        NodeTreeEntry* temp = findMinID(rt->right);
                        delete rt->node;
                        rt->node = new Node(temp->node); 
                        rt->right = deleteNode(rt->right, temp->node);}
                }
                return rt;
            }

            //returns node with minimal prioValue, don't use with empty tree
            Node* findMinPrioValue(NodeTreeEntry* rt){
                Node* minNode = rt->node;
                if(rt->left != NULL){
                    Node* minNodeL = findMinPrioValue(rt->left);
                    if(minNodeL->getPrioValue() < minNode->getPrioValue())
                        minNode = minNodeL;}
                if(rt->right != NULL){
                    Node* minNodeR = findMinPrioValue(rt->right);
                    if(minNodeR->getPrioValue() < minNode->getPrioValue())
                        minNode = minNodeR;}
                return minNode;
            }
    };
    
    /*
    The local info class of the operator
    */
    class shortestpathlfLI {
        public:
            //Constructor
            shortestpathlfLI(OrderedRelation* rel, ListExpr ttype, 
                             int source, int target, int sourcepos,
                             int targetpos, int sline, int lreal){
                result = new TupleBuffer((size_t) 64*1024*1024); 
                iter = 0;
                orel = rel;
                tupleType = ttype;
                sourceIndex = source;
                targetIndex = target;
                sourcePosIndex = sourcepos; 
                targetPosIndex = targetpos;
                slineIndex = sline;
                lrealIndex = lreal;
                success = getCatalogObjects();
            } 
                    
            //Destructor
            ~shortestpathlfLI(){
                if(iter) delete iter;
                delete result; 
                if(rtree != 0){
                delete rtree;}
                if(rel != 0){
                delete rel;}
            }
            
            /*
            This method initialises the necessary index and db objects.
            */
             bool getCatalogObjects(){
                SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
                #ifdef DEMO_MODE
                string indexName = "EdgesCropIndex_Box_rtree";
                #else
                string indexName = "EdgeIndex_Box_rtree";
                #endif
                cout << "index: " << indexName << endl;
                Word value;
                bool defined = false;
                ctlg->GetObject(indexName,value,defined);
                if(!defined){
                    cout << "could not initialise rtree" << endl;
                    return defined;}
                rtree = ((RTree2TID*)value.addr);
                
                //we also need a secondary relation to search for tuples
                #ifdef DEMO_MODE
                string dbName = "EdgesCropIndex";
                #else
                string dbName = "EdgeIndex";
                #endif
                cout << "db: " << dbName << endl;
                ctlg->GetObject(dbName, value, defined);
                if(!defined){
                    cout << "db not found: " << dbName << endl;
                    return defined;}
                rel = ((Relation*)value.addr);
                return defined;
            }
        
            /*
            This method calculates the shortest path between two points
            */
            void getShortestPath(Point* startPoint, Point* endPoint,
                                 collection::Collection* prefs){
                //get ids of nodes nearest to start and end points 
                int start = 0, end = 0;
                cout << success << endl;
                if(success == 0){
                    cout << "indexes not initialised, return" << endl;
                    return;}
                if(!startPoint->IsDefined() || !endPoint->IsDefined()){
                    cout << "input points are undefined, return" << endl;
                    return;}
                start =getNearestTuple(startPoint, sourcePosIndex, sourceIndex);
                end = getNearestTuple(endPoint, targetPosIndex, targetIndex);
                if(start <= 0){
                    cout<<"start point not found"<<endl;
                    return;
                }
                 if(end <= 0){
                    cout<<"end point not found"<<endl;
                    return;
                }
                OrderedRelationIterator* orelIt;
                vector<void*> attributes(2);
                vector<SmiKey::KeyDataType> kElems(2);
                SmiKey test((int32_t) 0);
                kElems[0] = test.GetType();
                kElems[1] = test.GetType();
                CcInt* minNodeId = new CcInt(true,0);
                CcInt* maxNodeId = new CcInt(true,numeric_limits<int>::max());
                TupleType* rtt = new TupleType(nl->Second(tupleType));
                
                //check the simplest case, maybe not necessary
                //if(startNodeID = endNodeID){...}
                
                //init open list (priority queue) and closed list
                NodeTree* prioQ = new NodeTree();
                NodeTree* closedList = new NodeTree();
                prioQ->insert(new Node(start, -1, 0, 0));
                Node* actNode = 0;
                Node* inQueue = 0;
                Tuple* actTuple = 0;
                bool found = false;
                SimpleLine* l = 0;
                Point* p = 0;
                LReal* lr = 0;
                
//                 cout<<"search for shortestpath starts"<<endl;
                //search for shortest path
                while(!prioQ->isEmpty() && !found){
                    actNode = prioQ->findMin();
                    if(actNode->getNodeID() == end){
                        found = true;
                        break;}
                    closedList->insert(new Node(actNode));
                    //create composite key for iterating through all tuples
                    //where actual node is source
                    CcInt* actNodeID = new CcInt(true, actNode->getNodeID());
                    attributes[0] = actNodeID;
                    attributes[1] = minNodeId;
                    CompositeKey actNodeLower(attributes,kElems,false);
                    attributes[1] = maxNodeId;
                    CompositeKey actNodeUpper(attributes,kElems,true);
                    orelIt =
                    (OrderedRelationIterator*)orel->MakeRangeScan(actNodeLower,
                                                                actNodeUpper);
                    actTuple = orelIt->GetNextTuple();
                    //iterate through all tuples where actual node is source
                    while(actTuple != 0){
                        //successor node is target of actual tuple
                        int nID = ((CcInt*)actTuple->GetAttribute(targetIndex))
                                                                 ->GetIntval();
                        //if successor is in the closed list do nothing
                        if(closedList->find(nID) != NULL){
                            actTuple->DeleteIfAllowed(); 
                            actTuple = orelIt->GetNextTuple();
                            continue;
                        }
                        int beforeID = actNode->getNodeID();
                        l =(SimpleLine*)actTuple->GetAttribute(slineIndex);
                        double dist = actNode->getDistFromStart() +l->Length();
                        p =(Point*)actTuple->GetAttribute(targetPosIndex);
                        double prio = dist + getEuclid(p, endPoint);
                        //if preferences are defined: modify priority value   
                        //according to user preferences for height gradient
                        if(prefs != 0){
                            lr = (LReal*)actTuple->GetAttribute(lrealIndex);
                            correctForHeightGradient(prio,
                                                     lr,
                                                     l->StartPoint().GetX(),
                                                     prefs);
                        }
                        //if successor in the open list, compare distance values
                        if(prioQ->find(nID) != NULL){
                            inQueue = prioQ->find(nID);
                            if(dist < inQueue->getDistFromStart())
                                inQueue->update(beforeID, dist, prio);
                            actTuple->DeleteIfAllowed(); 
                            actTuple = orelIt->GetNextTuple();
                            continue;
                        }
                        //add successor to the priority queue
                        prioQ->insert(new Node(nID, beforeID, dist, prio));
                        actTuple->DeleteIfAllowed(); //actTuple = 0;
                        actTuple = orelIt->GetNextTuple();
                    }
                    if(actTuple != 0){
                        actTuple->DeleteIfAllowed(); //actTuple = 0;
                    }
                    prioQ->deleteNode(actNode); //delete actNode;
                    actNodeID->DeleteIfAllowed();
                    delete orelIt; // orelIt = 0;
                }
//                 cout<<"search for shortestpath ended"<<endl;
                minNodeId->DeleteIfAllowed();
                maxNodeId->DeleteIfAllowed();
                if(!found){
                    //delete actNode; //actNode = 0;
                    //delete inQueue; //inQueue = 0;
                    cout<<"no path exists"<<endl;
                    attributes.clear();
                    kElems.clear();
                    rtt->DeleteIfAllowed(); // rtt = 0;
                    delete prioQ;
                    delete closedList;
                    return;
                }
                //write shortest path to result relation
                CcInt* startID = new CcInt(true,actNode->getBeforeNodeID());
                CcInt* endID = new CcInt(true,actNode->getNodeID());
                attributes[0] = startID;
                attributes[1] = endID;
                CompositeKey actNodeKeyLower(attributes,kElems,false);
                CompositeKey actNodeKeyUpper(attributes,kElems,true);
                orelIt = (OrderedRelationIterator*) orel->
                            MakeRangeScan(actNodeKeyLower, actNodeKeyUpper);
                actTuple = orelIt->GetNextTuple();
                Tuple* resTuple = 0;
                while(actTuple != 0){
                    resTuple = new Tuple(rtt);
                    for(int i = 0; i < actTuple->GetNoAttributes(); i++){
                        resTuple->CopyAttribute(i, actTuple, i);}
                    result->AppendTuple(resTuple);
                    resTuple->DeleteIfAllowed();
                    actNode = closedList->find(actNode->getBeforeNodeID());
                    //only start node has negative before node id
                    //if start node is reached, path is completed
                    if(actNode->getBeforeNodeID() < 0) 
                        break;
                    startID->DeleteIfAllowed();
                    endID->DeleteIfAllowed();
                    startID = new CcInt(true,actNode->getBeforeNodeID());
                    endID = new CcInt(true,actNode->getNodeID());
                    attributes[0] = startID;
                    attributes[1] = endID;
                    CompositeKey actNodeKeyLower(attributes,kElems,false);
                    CompositeKey actNodeKeyUpper(attributes,kElems,true);
                    delete orelIt;
                    orelIt = (OrderedRelationIterator*) orel->
                             MakeRangeScan(actNodeKeyLower, actNodeKeyUpper);
                    actTuple->DeleteIfAllowed();
                    actTuple = orelIt->GetNextTuple();
                }
                //if(actNode != 0)
                    //delete actNode; //actNode = 0;
                //if(inQueue != 0)
                    //delete inQueue; //inQueue = 0;
                if(actTuple != 0)
                    actTuple->DeleteIfAllowed(); //actTuple = 0; 
                if(startID != 0)
                    startID->DeleteIfAllowed(); //startID = 0; 
                if(endID != 0)
                    endID->DeleteIfAllowed(); //endID = 0; 
                attributes.clear();
                kElems.clear();
                rtt->DeleteIfAllowed(); // rtt = 0;
                delete prioQ;
                delete closedList;
                delete orelIt; //orelIt = 0;
                if(iter) delete iter;
                iter = result->MakeScan();
                tupleCount = result->GetNoTuples();
                return;
            }
            
            /*
            This method modifies priority value of node by multiplying
            it with a correction factor. Correction factor is defined 
            according to user preferences for height gradient
            */
            void correctForHeightGradient(double& prio,
                                          LReal* heightfun,
                                          double lat,
                                          collection::Collection* prefs){
                //set the lowest priority for undefined values
                if(!heightfun->IsDefined() || heightfun->IsEmpty()){
                    prio *= 6;
                    return;
                }
                int numOfUnits = heightfun->GetNoComponents(); 
                double unitLength = 0, height = 0, length = 0;
                LUReal unit;
                for(int i = 0; i < numOfUnits; i++){
                    heightfun->Get(i, unit);
                    unitLength = unit.lengthInterval.end.GetRealval() - 
                                 unit.lengthInterval.start.GetRealval();
                    height += unit.m * unitLength;
                    length += unitLength; 
                }
                //double avgGradient = height / length;

                // conversion of degrees to meters (~85.000 m/deg at 37.0 lat)
                // (not precise, only applies to same (or similar) latitude)
                if (90 - std::abs(lat) < 1e-5) lat = 89.0; // no division by 0
                const double conversion = 2 * PI * 6371/*km*/ * 1000/*(m)*/ *
                    std::cos(lat / 180 * PI) /*at ~lat~*/ / 360;
                // convert length to meters and calculate average gradient
                double avgGradient = height / (length * conversion);
                CcInt* tmp;

                if(avgGradient < 0)
                    tmp = (CcInt*)prefs->GetComponent(0);
                if(avgGradient >= 0 && avgGradient < 0.05)
                    tmp = (CcInt*)prefs->GetComponent(1);
                if(avgGradient >= 0.05 && avgGradient < 0.10)
                    tmp = (CcInt*)prefs->GetComponent(2);
                if(avgGradient >= 0.10 && avgGradient < 0.15)
                    tmp = (CcInt*)prefs->GetComponent(3);
                if(avgGradient >= 0.15)
                    tmp = (CcInt*)prefs->GetComponent(4);

                prio *= (6 - tmp->GetIntval());
                delete tmp;

                return;
            }
            
            /*
            This method selects and returns the correct tuple id of the 
            ordered relation, performing Euclidean distance calculation 
            to get the nearest match.
            The method is called for determining the start and end points 
            of the path.
            !Note! that the function requires an index and an intermediate 
            relation which are both hard-coded in the current version (see
            function getCatalogObjects).
            */
            int getNearestTuple(Point* inputPoint, 
                                int index, 
                                int indexSource){
                 if(!success){return 0;}
                 Tuple* returnTuple = 0;
                 Point* returnPoint = 0;
                 vector<void*> attributes(2);
                 vector<SmiKey::KeyDataType> kElems(2);
                 SmiKey test((int32_t) 0);
                 kElems[0] = test.GetType();
                 kElems[1] = test.GetType();
                 Tuple* intermediate = 0;
                 GenericRelationIterator* iter=0;
                 Tuple* currentTuple = 0;
                 Point* currentPoint = 0;
                 int val = 0;       
                 double returnEuclid = 0;
                //get the input point's bbox
                Rectangle<2> box = inputPoint->BoundingBox();
                //First and Next search on rtree
                //(compare method windowintersect)
                R_TreeLeafEntry<2, TupleId> entry;
                if(rtree->First(box, entry)){
                    TupleId id = (TupleId) entry.info;
                    //Extract tuple id and attrs from intermediate relation
                    intermediate = rel->GetTuple(id, false);
                    attributes[0] = (CcInt*) intermediate
                        ->GetAttribute(sourceIndex);
                    attributes[1] = (CcInt*) intermediate
                        ->GetAttribute(targetIndex);
                    //Make composite key to search for return ruple
                    CompositeKey first(attributes, kElems);
                    CompositeKey second(attributes, kElems, true);
                    if(iter) delete iter;
                    iter = orel->MakeRangeScan(first, second);
                    returnTuple = iter->GetNextTuple();
                    delete iter;
                    iter = 0;
                    if (returnTuple == 0){
                       if(intermediate) intermediate->DeleteIfAllowed();
                       return 0;
                    }
                    returnPoint = (Point*) returnTuple->GetAttribute(index);
                    //check for similarity first
                    if(AlmostEqual(*inputPoint, *returnPoint)){
                        val = ((CcInt*)returnTuple->GetAttribute(indexSource))
                            ->GetIntval();
                        returnTuple->DeleteIfAllowed();
                        if(intermediate) intermediate->DeleteIfAllowed();
                        return val;
                    }
                    returnEuclid = getEuclid(inputPoint, returnPoint);
                }
                while(rtree->Next(entry)){
                    TupleId id = (TupleId) entry.info;
                    if(intermediate)intermediate->DeleteIfAllowed();
                    intermediate = rel->GetTuple(id, false);
                    attributes[0] = (CcInt*) intermediate
                        ->GetAttribute(sourceIndex);
                    attributes[1] = (CcInt*) intermediate
                        ->GetAttribute(targetIndex);
                    CompositeKey first(attributes, kElems);
                    CompositeKey second(attributes, kElems, true);
                    if(iter) delete iter;
                    iter = orel->MakeRangeScan(first, second);
                    if(currentTuple!=0){currentTuple->DeleteIfAllowed();}
                    currentTuple = iter->GetNextTuple();
                    delete iter;
                    currentPoint = 
                        (Point*) currentTuple->GetAttribute(index);
                    //check again for similarity
                     if(AlmostEqual(*inputPoint, *currentPoint)){
                         returnTuple->DeleteIfAllowed();
                         val = ((CcInt*)currentTuple->GetAttribute(indexSource))
                             ->GetIntval();
                         currentTuple->DeleteIfAllowed();
                         if(intermediate) intermediate->DeleteIfAllowed();
                         return val;
                    }                 
                    double currentEuclid = getEuclid(inputPoint, currentPoint);
                    if ((currentEuclid < returnEuclid) && currentEuclid != 0)
                    {
                        returnTuple->DeleteIfAllowed();
                        returnTuple = currentTuple->Clone();
                        returnEuclid = getEuclid(inputPoint, 
                                 (Point*) returnTuple->GetAttribute(index));
                    }
                }
                attributes.clear();
                kElems.clear();
                if(intermediate!=0){intermediate->DeleteIfAllowed();}
                if(currentTuple!=0){currentTuple->DeleteIfAllowed();}
                if(returnTuple != 0){
                    val = ((CcInt*)returnTuple->GetAttribute(indexSource))
                        ->GetIntval();
                    returnTuple->DeleteIfAllowed();
                }
                return val; 
            }
        
             /*
             This method calculates and returns the Euclidean distance
             between 2 input points.
             */
             double getEuclid(Point* a, Point* b){
                 if(a->IsDefined() && b->IsDefined()){
                     double ax = a->GetX();
                     double ay = a->GetY();
                     double bx = b->GetX();
                     double by = b->GetY();
                     double first = pow((ax-bx), 2.0);
                     double second = pow((ay-by), 2.0);
                     double result = sqrt(first + second);
                     return result;
                }
                else{
                    return 0;
                }
             }
        
            /*
            This method outputs the stored tuples from the tuple buffer
            */
            Tuple* getNext(){
                if(result != 0 && iter!= 0){
                    Tuple* res;
                    size_t c = 0;
                    while(c<=tupleCount && ((res = iter->GetNextTuple())!=0)){
                        if(c==tupleCount-1){
                             tupleCount--;
                             if(iter) delete iter;
                             iter = result->MakeScan();//reset iterator
                             return res;
                        }  else {
                           res->DeleteIfAllowed();
                        }
                     c++;
                   }
                }
                return 0; 
            }        
            
        private:
            size_t tupleCount;
            TupleBuffer* result;
            GenericRelationIterator* iter;
            ListExpr tupleType;     
            int sourceIndex = -1;    
            int targetIndex = -1;
            int sourcePosIndex = -1; 
            int targetPosIndex = -1;
            int slineIndex = -1;
            int lrealIndex = -1;
            OrderedRelation* orel;
            RTree2TID* rtree = 0;
            Relation* rel = 0;
            bool success = false;
    };   
    
} //end of namespace routeplanningalgebra

#endif //SECONDO_SHORTESTPATHLF_H
