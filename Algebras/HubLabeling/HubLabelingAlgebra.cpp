/*
----
This file is part of SECONDO.

Copyright (C) 2016, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty ofn
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module HubLabeling Algebra

Maerz 2017 Sebastian Krings

[TOC]

1 Overview

The HubLabeling Algebra implements the HubLabeling Algorithm.
It offers some operators for creating the neccessary Hub-Labels over all nodes
 and for doing shortest path queries on this structure.

2 Sourcecode of HubLabelingAlgebra

Within this sourcecode we provide three different approaches to implement
 the HubLabeling (HL) algorithm and the Contraction Hierarchy (CH) algorithm
 which is a part of the HubLabeling algorithm.
Basic input is the import of an OSM graph using the Secondo-Script
 for importing OSM graphs.
In the following we describe those three approaches in short:

1) Persistent Approach
   Within this approach most data, except for auxiliary, is kept within
    the Secondo persistance structures like OrderedRelations and Relations.
   Due to this the memory foodprint is very low.
   But also due to this there are many expensive load-operations
    which lead to very bad runtime perfomances.
   Using this approach only  is suitable for networks not greater than
    1000 Edges.
   One can use the following Secondo-Operators:
   - hlCalcWeightsOrel
     (calculates the edge costs by a fix set of speed limits per roadType
       of the given OSM Graph)
   - hlIterateOverAllNodesByRankAscAndDoContraction
     (to create a contracted graph)
   - hlCreateLabels
     (create HubLabels on this contracted graph)
   - hlQuery
     (do a HL search on the HubLabels)

2) In-Memory Approach
   Here all data is loaded into main memory and then be processed.
   This increases of course memory footprint of some GB
   (depends on the given networks size) but decreases runtime.
   One can use the following Secondo-Operators:
   - hlCalcWeightsOrel
     (calculates the edge costs by a fix set of speed limits per roadType
       of the given OSM Graph)
   - hlDoContractionOfHlGraph
     (to create a contracted graph)
   - hlDoChSearchInHlGraph
     (performs a CH search over the contracted hlGraph)
   - hlCreateLabelsFromHlGraph
     (create HubLabels on this contracted hlGraph)
   - hlQuery
     (do a HL search on the HubLabels)

3) In-Memory Approach using MainMemory2Algebra
   Here we make use of the existing MainMemory2Algebra to process CH and HL
    within main memory.
   This Apporach is not yet implemented and do not work at the moment.


2.1 Defines, includes, and constants

*/

#include "Attribute.h"          // implementation of attribute types
#include "Algebra.h"            // definition of the algebra
#include "NestedList.h"         // required at many places
#include "QueryProcessor.h"     // needed for implementing value mappings
#include "AlgebraManager.h"     // e.g., check for a certain kind
#include "Operator.h"           // for operator creation
#include "StandardTypes.h"      // priovides int, real, string, bool type
#include "Algebras/FText/FTextAlgebra.h"
#include "Symbols.h"            // predefined strings
#include "ListUtils.h"          // useful functions for nested lists
#include "Algebras/Stream/Stream.h"             // wrapper for secondo streams


#include "LogMsg.h"             // send error messages

#include "../../Tools/Flob/DbArray.h"  // use of DbArrays

#include "Algebras/Relation-C++/RelationAlgebra.h"           // use of tuples
#include "Algebras/OrderedRelation/OrderedRelationAlgebra.h" // use of tuples
#include "Algebras/NestedRelation/NestedRelationAlgebra.h"   // use of tuples
#include "Algebras/BTree/BTreeAlgebra.h"           // use of tuples
#include "Algebras/Standard-C++/LongInt.h"           // use of longint
#include "Algebras/Spatial/Point.h"           // use of point
#include "Algebras/Spatial/SpatialAlgebra.h"           // use of sline

#include <math.h>               // required for some operators
#include <stack>
#include <limits>
#include <map>

#include <unordered_set>
#include <tuple>


#include "Algebras/MainMemory2/MainMemoryExt.h"

/*
2.2 Global Variables

Secondo uses some variables designed as singleton pattern. For accessing these
global variables, these variables have to be declared to be extern:

*/

extern NestedList *nl;
extern QueryProcessor *qp;

using namespace std;

/*
2.3 Namespace

Each algebra file defines a lot of functions. Thus, name conflicts may arise
with function names defined in other algebra modules during compiling/linking
the system. To avoid these conflicts, the algebra implementation should be
embedded into a namespace.

*/

namespace hublabeling
{

/*
2.4 Logging

We provide some console outputs through very simple logging.
It provides levels of error, debug and info

*/
 //Debug
 //#define USEDEBUG

#ifdef USEDEBUG
#define LogDebug(x) std::cout << x
#else
#define LogDebug(x)
#endif


 //Info
#define USEINFO

#ifdef USEINFO
#define LogInfo(x) std::cout << x
#else
#define LogInfo(x)
#endif


 //Error
#define USEERROR

#ifdef USEERROR
#define LogError(x) std::cout << x
#else
#define LogError(x)
#endif


 //Performance measure
#define USEPERF

#ifdef USEPERF
#define LogPerf(x) std::cout << std::setprecision (2) << x
#else
#define LogPerf(x)
#endif


/*
2.5 Auxilary Classes

Some functionality uses special defined classes to store information
 during processing.

*/

/*
 This class provides a structure to represent an edge out of the hlGraph
  which is the in memory representation of the underlying network to process.
 This class is used by the (2) In-Memory Approach.

 An edge here is stored space efficient by storing edges as they are
  and additionally the same edges a second time but in reverse manner.

 Functions related to 'forward' mean the original edge as it is and
 'reverse' relates to the reverse edges.

*/
class HlEdgeEntry
{
public:
    // constructor doing nothing
    HlEdgeEntry() {}
    // destructor
    ~HlEdgeEntry() {}

    int getTargetIndex() const
    {
        return targetIndex;
    }
    int getParentIndexForward() const
    {
        return parentIndexForward;
    }
    int getParentIndexReverse() const
    {
        return parentIndexReverse;
    }
    double getWeightForward() const
    {
        return weightForward;
    }
    double getWeightReverse() const
    {
        return weightReverse;
    }
    bool getIsForward() const
    {
        return isForward;
    }
    bool getIsReverse() const
    {
        return isReverse;
    }
    bool getIsUpwardOriginal() const
    {
        return isUpwardOriginal;
    }

    void setTargetIndex(const int _targetIndex)
    {
        targetIndex = _targetIndex;
    }

    /*
     * A parent here is the node v which was contracted
     *  and then replaced by this shortcutedge.
    */
    void setParentIndexForward(const int _parentIndexForward)
    {
        parentIndexForward = _parentIndexForward;
    }

    /*
     * A parent here is the node v which was contracted
     *  and then replaced by this shortcutedge.
    */
    void setParentIndexReverse(const int _parentIndexReverse)
    {
        parentIndexReverse = _parentIndexReverse;
    }
    void setWeightForward(const double _weightForward)
    {
        weightForward = _weightForward;
    }
    void setWeightReverse(const double _weightReverse)
    {
        weightReverse = _weightReverse;
    }
    void setIsForward(const bool _isForward)
    {
        isForward = _isForward;
    }
    void setIsReverse(const bool _isReverse)
    {
        isReverse = _isReverse;
    }
    void setIsUpwardOriginal(const bool _isUpwardOriginal)
    {
        isUpwardOriginal = _isUpwardOriginal;
    }

    private:

        int targetIndex = -1;
        int parentIndexForward = -1; //used for shortcuts
        int parentIndexReverse = -1; //used for shortcuts
        double weightForward = -1.0;
        double weightReverse = -1.0;
        bool isForward = false;
        bool isReverse = false;
        bool isUpwardOriginal = false;
};

/*
 This class provides a structure to represent an node out of the hlGraph
  which is the in memory representation of the underlying network to process.
 This class is used by the (2) In-Memory Approach.

 A node consists of a nodeId, the nodes Rank and a vector of adjacent edges.


*/
class HlNodeEntry
{
public:
    // constructor doing nothing
    HlNodeEntry() {}
    // destructor
    ~HlNodeEntry() {}

    int getNodeId() const
    {
        return nodeId;
    }
    int getRankValue() const
    {
        return rankValue;
    }
    std::vector<HlEdgeEntry*>* getEdgesVector() const
    {
        return edgesVector;
    }

    void setNodeId(const int _nodeId)
    {
        nodeId = _nodeId;
    }
    void setRankValue(const int _rankValue)
    {
        rankValue = _rankValue;
    }
    void setEdgesVector(std::vector<HlEdgeEntry*>* _edgesVector)
    {
        edgesVector = _edgesVector;
    }

    private:

        int nodeId = -1;
        int rankValue = -1;
        std::vector<HlEdgeEntry*>* edgesVector = 0;
};




/*
 This class provides a structure to represent a node out of the searchTree
  resulting from an CH search.

 The search tree is space efficient in that way that that a visitedNode,
  represented by this class, stores information of forward- and reversesearch
  within the same object if both searches visit the same nodes.
 This for there are functions for every of those both search spaces.

 A predecessor here means to be the node which directly comes before the
  current node on the shortest path from the source
  of this forward- or reversesearch.
 Using this one can resolve the path recursively back to the source node,
  which has no valid predecessor (= -1).

 This class is used by the (2) In-Memory Approach.


*/
class ChNode
{
public:
    // constructor doing nothing
    ChNode() {}
    // destructor
    ~ChNode() {}


    double getDistForward() const
    {
        return distForward;
    }
    double getDistReverse() const
    {
        return distReverse;
    }
    int getPredecessorIndexForward() const
    {
        return predecessorIndexForward;
    }
    int getPredecessorIndexReverse() const
    {
        return predecessorIndexReverse;
    }

    void setDistForward(const double _distForward)
    {
        distForward = _distForward;
    }
    void setDistReverse(const double _distReverse)
    {
        distReverse = _distReverse;
    }
    void setPredecessorIndexForward(const int _predecessorIndexForward)
    {
        predecessorIndexForward = _predecessorIndexForward;
    }
    void setPredecessorIndexReverse(const int _predecessorIndexReverse)
    {
        predecessorIndexReverse = _predecessorIndexReverse;
    }

    private:


        double distForward = 0.0;
        double distReverse = 0.0;
        int predecessorIndexForward = -1;
        int predecessorIndexReverse = -1;
};


/*
3 The Class HubLabelClass

This class is used to provide basic structures to handle HubLabels
and to privide functionality to create such ones.

-A given Graph can be modified with auxiliary attributes.
-The graph can be contracted.
-The graph can be divided into upwards- and downwards graphs.
-HubLabels can be created.
-A HubLabeling Search can be performed.

There are several operators for doing this.

3.1 Basic HubLabeling structures

*/
class HubLabelClass
{
public:
    // constructor doing nothing
    HubLabelClass() {}
    // constructor initializing the object
    HubLabelClass(const double _x, const double _y,
                  const double _r):
        x(_x), y(_y), r(_r) {}
    // destructor
    ~HubLabelClass() {}
    static const string BasicType()
    {
        return "HubLabelClass";
    }
    // the checktype function for non-nested types looks always
    // the same
    static const bool checkType(const ListExpr list)
    {
        return listutils::isSymbol(list, BasicType());
    }

    double getX() const
    {
        return x;
    }
    double getY() const
    {
        return y;
    }
    double getR() const
    {
        return r;
    }

    int getProgressInterval() const
    {
        return progressInterval;
    }

    void setProgressInterval(int _progressInterval)
    {
        int tmpVal = _progressInterval;
        if(tmpVal < 1)
        {
            tmpVal = 1;
        }
        progressInterval = tmpVal;
    }


    /*
     * function to receive the current time in milliseconds
    */
    double getCurrentTimeInMs() const
    {
        struct timeval tp;
        gettimeofday(&tp, NULL);
         //get current timestamp in milliseconds
        long nsLong = (long) tp.tv_sec * 1000L * 1000L + tp.tv_usec;
        double msDouble = (double) nsLong / 1000;

        #ifdef USEDEBUG
        #ifdef USEPERF
        LogPerf("current time in nanoseconds: "
         << nsLong << " in milliseconds: " << fixed << msDouble << endl);
        #endif
        #endif

        return msDouble;
    }

/*

3.2 HubLabeling functionality of (1) Persistent Approach

*/

    /*
     * Definition of CostModes.
     * Used for getting shortest paths by different cost-calculations.
    */
    static const int HL_DEFAULT_COST_MODE = 1;
    static const int HL_LENGTH_COST_MODE = 2;
    static const int HL_TIME_COST_MODE = 3;

    /*
     * Gets the weightedCosts of a given edge.
     * Differences between several cost modes to calculate the weighted
     *   costs of edges.
     *
     * !TODO: Currently does not support different cost modes
     *  but extracts the weighted costs directly from the given edge
     *  independently of the given cost mode using the Tuple-field
     *  on index position like @see HL_INDEX_OF_COSTS_IN_EDGE_TUPLE
     *
     * @param costMode Defines the cost mode with which the weighted
     *  costs will be calculated
     *  uses @see HL_DEFAULT_COST_MODE when a value of -1 is given
     * @param currentEdge The edge for which the weighted costs shall
     *  be calculated/ extracted from
     * @return the calculated/ extracted weighted costs
    */
    double hlGetWeightedCost(int costMode,
                             Tuple* currentEdge) const
    {
        double calculatedDistance = -1;
        if(costMode == -1)
        {
            costMode = HL_DEFAULT_COST_MODE;
        }
        CcReal* currentEdgeCost = (CcReal*)
                                  currentEdge->GetAttribute(
                                      HL_INDEX_OF_COSTS_IN_EDGE_TUPLE);

        calculatedDistance =
            currentEdgeCost->GetRealval();

        return calculatedDistance;
    }

    /*
     * Definition of Calculation Functions used to calculate the rank
     *  of a vertex inside the underlying graph.
    */
    static const int HL_DEFAULT_CALC_FUNCTION = 1;

    /*
     * Calculcates the ranking for he current vertext by either a given
     *  calculation function
     *  or else by the default calculation function.
     *
     * !TODO:
     * Supported Calculation Functions are:
     * @see HL_DEFAULT_CALC_FUNCTION
     *  calculates a ration of (countIncomingEdges *
     *  countOutgoingEdges) / (countIncomingEdges + countOutgoingEdges)
     *
     * @param orelEdgesSource is an OrderedRelation containging all
     *  edges of the underlying graph sorted by the field Source
     * @param orelEdgesTarget is an OrderedRelation containging all
     *  edges of the underlying graph sorted by the field Target
     * @param calcFunctionHL defines the calculationFunction to be used
     *  to calculate the rank, uses @see HL_DEFAULT_CALC_FUNCTION when
     *  a value of < 1 has been given
     * @param currentVertexId is the ID of the current vertex the rank
     *  shall be calculated for
     * @return the calculatedRank
    */
    double hlCalcRank(OrderedRelation* orelEdgesSource,
                      OrderedRelation* orelEdgesTarget,
                      int calcFunctionHL,
                      CcInt* currentVertexId) const
    {
        //XXTODO: muss der rank nicht eindeutig sein?
        double calculatedRank = -1;

        //Standard Funktion verwenden
        if(calcFunctionHL < 1)
        {
            calcFunctionHL = HL_DEFAULT_CALC_FUNCTION;
        }

        if(calcFunctionHL == HL_DEFAULT_CALC_FUNCTION)
        {
            int countIncomingEdges = 0;
            int countOutgoingEdges = 0;
            int countEdgesToBeDeleted = 0;
            int countShortcutsToBeCreated = 0;
            double shortcutsPerAdjNodeRatio = 0;

            std::vector<void*> vecAttributesFrom(1);
            vecAttributesFrom[0] = currentVertexId;
            std::vector<SmiKey::KeyDataType> vecAttrTypesFrom(
                1);
            vecAttrTypesFrom[0] =
                currentVertexId->getSMIKeyType();
            CompositeKey from(vecAttributesFrom,
                              vecAttrTypesFrom,
                              false);

            CcInt currentVertexIdPlusOne(true,
                                         currentVertexId->GetIntval() + 1);
            CcInt* currentVertexIdPlusOnePtr =
                &currentVertexIdPlusOne;
            std::vector<void*> vecAttributesTo(1);
            vecAttributesTo[0] = currentVertexIdPlusOnePtr;
            std::vector<SmiKey::KeyDataType> vecAttrTypesTo(
                1);
            vecAttrTypesTo[0] =
                currentVertexIdPlusOnePtr->getSMIKeyType();
            CompositeKey to(vecAttributesTo, vecAttrTypesTo,
                            false);

            //get all outgoing edges
            GenericRelationIterator* relEdgesSourceIter =
                orelEdgesSource->MakeRangeScan(from, to);

            //Get first Tuple
            Tuple* currentEdgeSourceTupleS =
                relEdgesSourceIter->GetNextTuple();

            //Iterate over all Tuples
            while(currentEdgeSourceTupleS)
            {
                countOutgoingEdges++;

                currentEdgeSourceTupleS->DeleteIfAllowed();
                currentEdgeSourceTupleS = 0;
                currentEdgeSourceTupleS =
                    relEdgesSourceIter->GetNextTuple();
            }
            delete relEdgesSourceIter;

            //get all incoming edges
            GenericRelationIterator* relEdgesTargetIter =
                orelEdgesTarget->MakeRangeScan(from, to);

            //Get first Tuple
            Tuple* currentEdgeTargetTupleT =
                relEdgesTargetIter->GetNextTuple();

            //Iterate over all Tuples
            while(currentEdgeTargetTupleT)
            {
                countIncomingEdges++;

                currentEdgeTargetTupleT->DeleteIfAllowed();
                currentEdgeTargetTupleT = 0;
                currentEdgeTargetTupleT =
                    relEdgesTargetIter->GetNextTuple();
            }
            delete relEdgesTargetIter;

            //Do calculation
            countEdgesToBeDeleted =
                countIncomingEdges + countOutgoingEdges;
            countShortcutsToBeCreated = countIncomingEdges *
                                        (countOutgoingEdges - 1);

            if(countEdgesToBeDeleted != 0)
            {
                shortcutsPerAdjNodeRatio = (double)
                        countShortcutsToBeCreated / countEdgesToBeDeleted;
            }
            calculatedRank = shortcutsPerAdjNodeRatio;
        }

        return calculatedRank;
    }


    /*
     * OrderedRelation type for the returnType of the
     *  OneHopReverseSearch
     * Used for creation of new OrderedRelation Objects, new Tuples of
     *  this OrderedRelation or cloning or deleting of an
     *  OrderedRelation of this type
    */
    static const string
    hlGetOneHopReverseOrelIdTypeInfo()
    {
        return "(" + OrderedRelation::BasicType() +
               "(" + Tuple::BasicType() +
               "(" +
               "(NodeIdSourceReverseX " + CcInt::BasicType()
               + ")" +
               "(NodeIdTargetT " + CcInt::BasicType() + ")" +
               "(WeightedCosts " + CcReal::BasicType() + ")" +
               ")" +
               ")" +
               "(NodeIdSourceReverseX)" +
               ")";
    }


    /*
     * OrderedRelation type for Edges ordered by Source
     * Used for creation of new OrderedRelation Objects, new Tuples of
     *  this OrderedRelation or cloning or deleting of an
     *  OrderedRelation of this type
    */
    static const string hlGetEdgesOrelSourceTypeInfo()
    {
        return "(" + Relation::BasicType() +
               hlGetEdgesTupleTypeInfo() +
               "(Source)" +
               ")";
    }


    /*
     * OrderedRelation type for Edges ordered by Target
     * Used for creation of new OrderedRelation Objects, new Tuples of
     *  this OrderedRelation or cloning or deleting of an
     *  OrderedRelation of this type
    */
    static const string hlGetEdgesOrelTargetTypeInfo()
    {
        return "(" + OrderedRelation::BasicType() +
               hlGetEdgesTupleTypeInfo() +
               "(Target)" +
               ")";
    }


    /*
     * Relation type for Edges
     * Used for creation of new Relation Objects, new Tuples of this
     *  Relation or cloning or deleting of an Relation of this type
    */
    static const string hlGetEdgesRelTypeInfo()
    {
        return "(" + Relation::BasicType() +
               hlGetEdgesTupleTypeInfo() +
               ")";
    }


    /*
     * TupleType type for Edges
    */
    static const string hlGetEdgesTupleTypeInfo()
    {
        return "(" + Tuple::BasicType() +
               "(" +
               "(Source " + CcInt::BasicType() + ")" +
               "(Target " + CcInt::BasicType() + ")" +
               "(SourcePos " + Point::BasicType() + ")" +
               "(TargetPos " + Point::BasicType() + ")" +
               "(SourceNodeCounter " + CcInt::BasicType() + ")" +
               "(TargetNodeCounter " + CcInt::BasicType() + ")" +
               "(Curve " + SimpleLine::BasicType() + ")" +
               "(RoadName " + FText::BasicType() + ")" +
               "(RoadType " + FText::BasicType() + ")" +
               "(WayId " + LongInt::BasicType() + ")" +
               "(Costs " + CcReal::BasicType() + ")" +
               "(HlShortcutViaParent " + CcInt::BasicType() + ")"
               +
               ")" +
               ")";
    }


    /*
     * Definition of Field-Indexes for edgesOrel and oneHopReverseOrel
    */
    static const int HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE
        = 0;
    static const int HL_INDEX_OF_TARGET_IN_EDGE_TUPLE
        = 1;
    static const int
    HL_INDEX_OF_SOURCE_POS_IN_EDGE_TUPLE = 2;
    static const int
    HL_INDEX_OF_TARGET_POS_IN_EDGE_TUPLE = 3;
    static const int
    HL_INDEX_OF_SOURCE_NODE_COUNTER_IN_EDGE_TUPLE = 4;
    static const int
    HL_INDEX_OF_TARGET_NODE_COUNTER_IN_EDGE_TUPLE = 5;
    static const int HL_INDEX_OF_CURVE_IN_EDGE_TUPLE =
        6;
    static const int
    HL_INDEX_OF_ROAD_NAME_IN_EDGE_TUPLE = 7;
    static const int
    HL_INDEX_OF_ROAD_TYPE_IN_EDGE_TUPLE = 8;
    static const int HL_INDEX_OF_WAY_ID_IN_EDGE_TUPLE
        = 9;
    static const int HL_INDEX_OF_COSTS_IN_EDGE_TUPLE =
        10;
    static const int
    HL_INDEX_OF_PARENT_ID_IN_EDGE_TUPLE = 11;
    static const int
    HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE = 0;
    static const int
    HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE = 1;
    static const int
    HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE = 2;

    /*
     * Processes a one-hop reverse search for every target-node
     *  reachable from v.
     * That means that there exists an outgoing edge from v to t.
     *
     * For every of those vertexes t an own one-hop reverse search is
     *  done.
     * There all vertexes x with an incoming edge e(x, t) to t are
     *  stored with the distance of e(x, t) into the given
     *  OrderedRelation ordered by the vertex ID of x.
     * All distances of all iterations done within this function call
     *  are stored together in the same orel such that there will be
     *  only one orel (the given one) for the given vertex v to be
     *  contracted.
     *
     * @param orelEdgesSource is an OrderedRelation containging all
     *  edges of the underlying graph sorted by the field Source
     * @param orelEdgesTarget is an OrderedRelation containging all
     *  edges of the underlying graph sorted by the field Target
     * @param currentVertexIdRevV the current vertex v to be contracted
     * @param oneHopReverseMultiMap the Ordered map by reference where
     *  all distances found by the reverse search  will be stored in
     * @return true, since the result of this function will be stored
     *  directly in @see oneHopReverseMultiMap
     *
    */
    bool hlOneHopReverseSearch(OrderedRelation*
                               orelEdgesSource,
                               OrderedRelation* orelEdgesTarget,
                               CcInt* currentVertexIdRevV,
                               std::multimap<int, std::tuple<int, double>>
                               &oneHopReverseMultiMap
                              ) const
    {
        std::vector<void*> vecAttributesFromRevV(1);
        vecAttributesFromRevV[0] = currentVertexIdRevV;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesFromRevV(1);
        vecAttrTypesFromRevV[0] =
            currentVertexIdRevV->getSMIKeyType();
        CompositeKey fromRevV(vecAttributesFromRevV,
                              vecAttrTypesFromRevV, false);

        CcInt currentVertexIdPlusOneRevV(true,
                                         currentVertexIdRevV->GetIntval() + 1);
        CcInt* currentVertexIdPlusOnePtrRevV =
            &currentVertexIdPlusOneRevV;
        std::vector<void*> vecAttributesToRevV(1);
        vecAttributesToRevV[0] =
            currentVertexIdPlusOnePtrRevV;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesToRevV(1);
        vecAttrTypesToRevV[0] =
            currentVertexIdPlusOnePtrRevV->getSMIKeyType();
        CompositeKey toRevV(vecAttributesToRevV,
                            vecAttrTypesToRevV,
                            false);

        //get all outgoing edges (v, t)
        GenericRelationIterator* relEdgesSourceIterRevVT =
            orelEdgesSource->MakeRangeScan(fromRevV, toRevV);

        //Get first Tuple
        Tuple* currentEdgeSourceTupleRevVT =
            relEdgesSourceIterRevVT->GetNextTuple();

        //Iterate over all Outgoing Tuples
        while(currentEdgeSourceTupleRevVT)
        {
            CcInt* currentTargetNodeRevT = (CcInt*)
                                    currentEdgeSourceTupleRevVT->GetAttribute(
                                        HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);

            //Process current iteration
            hlOneHopReverseSearchProcessIncomingEdges(
                orelEdgesTarget,
                oneHopReverseMultiMap, currentTargetNodeRevT);

            currentEdgeSourceTupleRevVT->DeleteIfAllowed();
            currentEdgeSourceTupleRevVT = 0;
            currentEdgeSourceTupleRevVT =
                relEdgesSourceIterRevVT->GetNextTuple();
        }
        delete relEdgesSourceIterRevVT;


        return true;
    }

    /*
     * Procession of inner iteration of reverse search
     *  ( @see hlOneHopReverseSearch)
     *  over all vertexes x incoming to the current vertext t.
     *
     * @param orelEdgesTarget is an OrderedRelation containging all
     *  edges of the underlying graph sorted by the field Target
     * @param oneHopReverseMultiMap the ordered map by reference where
     *  all distances found by the reverse search  will be stored in
     * @param currentTargetNodeRevT the current vertex t reachable
     *  from v
     * @return true, since the result of this function will be stored
     *  directly in @see oneHopReverseMultiMap
     *
    */
    bool hlOneHopReverseSearchProcessIncomingEdges(
        OrderedRelation* orelEdgesTarget,
        std::multimap<int, std::tuple<int, double>>
        &oneHopReverseMultiMap,
        CcInt* currentTargetNodeRevT) const
    {
        std::vector<void*> vecAttributesFromRevT(1);
        vecAttributesFromRevT[0] = currentTargetNodeRevT;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesFromRevT(1);
        vecAttrTypesFromRevT[0] =
            currentTargetNodeRevT->getSMIKeyType();
        CompositeKey fromRevT(vecAttributesFromRevT,
                              vecAttrTypesFromRevT, false);

        CcInt currentTargetNodePlusOneRevT(true,
                                currentTargetNodeRevT->GetIntval() + 1);
        CcInt* currentTargetNodePlusOnePtrRevT =
            &currentTargetNodePlusOneRevT;
        std::vector<void*> vecAttributesToRevT(1);
        vecAttributesToRevT[0] =
            currentTargetNodePlusOnePtrRevT;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesToRevT(1);
        vecAttrTypesToRevT[0] =
            currentTargetNodePlusOnePtrRevT->getSMIKeyType();
        CompositeKey toRevT(vecAttributesToRevT,
                            vecAttrTypesToRevT, false);

        //get all incoming edges (u, t)
        GenericRelationIterator* relEdgesTargetIterUT =
            orelEdgesTarget->MakeRangeScan(fromRevT, toRevT);

        //Get first Tuple u
        Tuple* currentEdgeTargetTupleU =
            relEdgesTargetIterUT->GetNextTuple();

        //Iterate over all Incoming Tuples u
        #ifdef USEDEBUG
            int i = 0;
        #endif
        while(currentEdgeTargetTupleU)
        {
            //Dont ignore current Node to be contracted because
            //we need its distances also in forward search
            CcInt* currentSourceNodeU = (CcInt*)
                                        currentEdgeTargetTupleU->GetAttribute(
                                            HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
            double currentDistXT = hlGetWeightedCost(
                            HL_DEFAULT_COST_MODE, currentEdgeTargetTupleU);
            #ifdef USEDEBUG
            LogDebug(i++ << ": (" <<
                  currentSourceNodeU->GetIntval() << ";" <<
                  currentTargetNodeRevT->GetIntval() << ";" <<
                  currentDistXT << "), " << endl);
            #endif

            bool doInsertNewXT = true;

            //Check whether there still is an edge XT and which one is
            //shorter, keep the shortest of both (may update the existing one)
            std::pair <std::multimap<int, std::tuple<int, double>>::iterator,
             std::multimap<int, std::tuple<int, double>>::iterator>
                    revXTRangeBoundsIter;
            revXTRangeBoundsIter =
                oneHopReverseMultiMap.equal_range(
                    currentSourceNodeU->GetIntval());

            for (std::multimap<int, std::tuple<int, double>>::iterator
                    revXTRangeIter = revXTRangeBoundsIter.first;
                    revXTRangeIter != revXTRangeBoundsIter.second;
                    ++revXTRangeIter)
            {
                std::tuple<int, double> lookupTupleXT =
                    (*revXTRangeIter).second;
                int lookupEdgeIdT =
                    std::get<HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE -
                    1>(lookupTupleXT);
                double lookupEdgeDistXT =
                    std::get<HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE - 1>
                    (lookupTupleXT);

                if(currentTargetNodeRevT->GetIntval() ==
                        lookupEdgeIdT)
                {
                    #ifdef USEDEBUG
                    LogDebug("XT still exists" << endl);
                    #endif
                    if(currentDistXT < lookupEdgeDistXT)
                    {
                        #ifdef USEDEBUG
                        LogDebug("still existing XT is longer,"
                         " going to delete it"
                         " here to insert the current new XT afterwards"
                          << endl );
                          #endif
                        oneHopReverseMultiMap.erase(revXTRangeIter);
                    }
                    else
                    {
                        //do not insert current new XT, its longer
                        // than the existing one
                        doInsertNewXT = false;
                    }
                }
            }

            if(doInsertNewXT)
            {
                std::tuple<int, double> insertTuple =
                    std::make_tuple(
                        currentTargetNodeRevT->GetIntval(),
                        currentDistXT);
                oneHopReverseMultiMap.insert(
                    pair<int, std::tuple<int, double>>
                    (currentSourceNodeU->GetIntval(), insertTuple));
            }

            //Free Incoming-Iteration
            currentEdgeTargetTupleU->DeleteIfAllowed();
            currentEdgeTargetTupleU = 0;
            currentEdgeTargetTupleU =
                relEdgesTargetIterUT->GetNextTuple();
        }
        if(oneHopReverseMultiMap.size() > 10)
        {
            //sleep(1000 * 1000 * 10);
        }
        delete relEdgesTargetIterUT;

        return true;
    }


    /*
     * OrderedRelation type for shortcuts to be created,
     *  identified during the contraction.
     * Used for creation of new OrderedRelation Objects,
     *  new Tuples of this OrderedRelation
     *  or cloning or deleting of an OrderedRelation of this type
    */
    static const string
    hlGetShortcutsCreatedSourceTypeInfo()
    {
        return "(" + OrderedRelation::BasicType() +
               "(" + Tuple::BasicType() +
               "(" +
               "(Source " + CcInt::BasicType() + ")" +
               "(Target " + CcInt::BasicType() + ")" +
               "(Dist " + CcReal::BasicType() + ")" +
               "(ParentVia " + CcInt::BasicType() + ")" +
               ")" +
               ")" +
               "(Source)" +
               ")";
    }

    /*
     * Definition of Field-Indexes for shortcuts-orel
    */
    static const int
    HL_INDEX_OF_SOURCE_IN_SHORTCUT_TUPLE = 0;
    static const int
    HL_INDEX_OF_TARGET_IN_SHORTCUT_TUPLE = 1;
    static const int
    HL_INDEX_OF_DIST_IN_SHORTCUT_TUPLE = 2;
    static const int
    HL_INDEX_OF_PARENT_ID_IN_SHORTCUT_TUPLE = 3;

    /*
     * OrderedRelation type for vertices scanned but not yet visited
     *  during forward search.
     * Used for creation of new OrderedRelation Objects, new Tuples
     *  of this OrderedRelation
     *  or cloning or deleting of an OrderedRelation of this type
    */
    static const string
    hlGetNotYetVisitedNodesTypeInfo()
    {
        return "(" + OrderedRelation::BasicType() +
               "(" + Tuple::BasicType() +
               "(" +
               "(Dist " + CcReal::BasicType() + ")" +
               "(NodeId " + CcInt::BasicType() + ")" +
               "(HopDepth " + CcInt::BasicType() + ")" +
               ")" +
               ")" +
               "(Dist)" +
               ")";
    }

    /*
     * Definition of Field-Indexes for notYetVisitedNodes-orel
    */
    static const int
    HL_INDEX_OF_DIST_IN_NOT_YET_VISITED = 0;
    static const int
    HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED = 1;
    static const int
    HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED = 2;

    /*
     * Definition of default h-hop-size
    */
    static const int HL_DEFAULT_H_DEPTH = 1;

    /*
     * OrderedRelation type for vertices still visited during forward search.
     * Used for creation of new OrderedRelation Objects, new Tuples
     *  of this OrderedRelation
     *  or cloning or deleting of an OrderedRelation of this type
    */
    static const string
    hlGetStillVisitedNodesTypeInfo()
    {
        return "(" + OrderedRelation::BasicType() +
               "(" + Tuple::BasicType() +
               "(" +
               "(NodeId " + CcInt::BasicType() + ")" +
               ")" +
               ")" +
               "(NodeId)" +
               ")";
    }

    /*
     * Definition of Field-Indexes for stillVisitedNodes-orel
    */
    static const int
    HL_INDEX_OF_NODE_ID_IN_STILL_VISITED = 0;


    /*
     * Processes a h-hop forward search for the given vertex v to be contracted.
     * Does so by iterating over all source nodes s having an incoming
     *  edge e(s, v) to v.
     * Takes the given distances from a previously executed one-hop reverse
     *  search ( @see copyMultimapReverseSearchXT)
     *  for the same vertext v and creates a copy of it for each source node s.
     * Stores shortcuts found during forward search in
     *  the given OrderedRelation.
     * Uses a h-hop-size during forward search to abort when looking
     *  for shorter pathes
     *  than the potentially shortcut currently viewed.
     * The h-hop-size means how deep (in the meaning of network depth)
     *  the dijkstra-search shall be performed,
     *  started out of the appropriate source node s (having an edge e(s, v)
     *  incoming to the current vertex v.
     *
     * @param orelEdgesSource is an OrderedRelation containging all edges
     *  of the underlying graph sorted by the field Source
     * @param orelEdgesTarget is an OrderedRelation containging all edges
     *  of the underlying graph sorted by the field Target
     * @param multimapReverseSearchXT the Ordered map by reference where
     *  all distances found by the reverse search  have been stored in
     * @param currentVertexIdFwdV the current vertex v to be contracted
     * @param shortcutsToBeCreatedOrelSourceToBeDeleted the multimap by
     *  reference where all shortcuts will be stored in
     * @param hDepth the h-hop-size to be used, will be overwritten
     *  by @see HL_DEFAULT_H_DEPTH if value < 1
     * @return true, since the result of this function will be stored
     *  directly in @see shortcutsToBeCreatedOrelSourceToBeDeleted
     *
    */
    bool hlHHopForwardSearch(OrderedRelation*
                             orelEdgesSource, OrderedRelation* orelEdgesTarget,
                             std::multimap<int, std::tuple<int, double>>
                             &multimapReverseSearchXT,
                             CcInt* currentVertexIdFwdV,
                             std::multimap<int, std::tuple<int, double, int>>
                             &shortcutsToBeCreatedOrelSourceToBeDeleted,
                             int hDepth) const
    {
        #ifdef USEDEBUG
        LogDebug("Starting hlHHopForwardSearch with v = " <<
                      currentVertexIdFwdV->GetIntval() <<
                      " and hDepth = " << hDepth << endl);
        #endif

        if(hDepth < 1)
        {
            hDepth = HL_DEFAULT_H_DEPTH;
        }

        //Prepare Range scan
        //Create From and to by currentVertexID (=v)
        std::vector<void*> vecAttributesFromFwdV(1);
        vecAttributesFromFwdV[0] = currentVertexIdFwdV;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesFromFwdV(1);
        vecAttrTypesFromFwdV[0] =
            currentVertexIdFwdV->getSMIKeyType();
        CompositeKey fromFwdV(vecAttributesFromFwdV,
                              vecAttrTypesFromFwdV, false);

        CcInt currentVertexIdPlusOneFwdV(true,
                                         currentVertexIdFwdV->GetIntval() + 1);
        CcInt* currentVertexIdPlusOnePtrFwdV =
            &currentVertexIdPlusOneFwdV;
        std::vector<void*> vecAttributesToFwdV(1);
        vecAttributesToFwdV[0] =
            currentVertexIdPlusOnePtrFwdV;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesToFwdV(1);
        vecAttrTypesToFwdV[0] =
            currentVertexIdPlusOnePtrFwdV->getSMIKeyType();
        CompositeKey toFwdV(vecAttributesToFwdV,
                            vecAttrTypesToFwdV, false);

        //get all incoming edges (s, v) to v
        GenericRelationIterator* relEdgesTargetIterSV =
            orelEdgesTarget->MakeRangeScan(fromFwdV, toFwdV);
        //TODO: muss der Wert von currentVertexIdPlusOnePtr->getSMIKeyType();
        // noch gelöscht werden per delete? Wenn ja dann auch andere derartige

        //Get first Tuple of them
        Tuple* currentEdgeTargetTupleSV =
            relEdgesTargetIterSV->GetNextTuple();

        //Iterate over all Incoming Tuples s to v
        while(currentEdgeTargetTupleSV)
        {
            //Get ID of current SourceNode (=s)
            CcInt* currentSourceNodeFwdS = (CcInt*)
                                    currentEdgeTargetTupleSV->GetAttribute(
                                        HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);

            #ifdef USEDEBUG
            LogDebug("Process next source node s = " <<
                              currentSourceNodeFwdS->GetIntval() << endl);
            #endif

            //Copy of multimapReverseSearchXT
            std::multimap<int, std::tuple<int, double>>
                    copyMultimapReverseSearchXT =
                        multimapReverseSearchXT; //XXTODO: ist diese Zuweisung
                        // ein Problem in Bezug auf nicht gelöschte Daten der
                        // vorherigen Iteration?

            //delete current surce node s from current copy of reverseSearchXT
            // where s = t
            // because it does not make sense to find/ create a shortcut
            // from s to s.
            // this is posible because s and v can have both, an incoming
            // and an outgoing edge
            //this for-loop has an emtpy increment section since it is
            // incremented manually next to the erase-command
            for (std::multimap<int, std::tuple<int, double>>::iterator
                    reverseXTIter =
                        copyMultimapReverseSearchXT.begin();
                    reverseXTIter !=
                    copyMultimapReverseSearchXT.end();)
            {
                // you have to do this because iterators are invalidated
                std::multimap<int, std::tuple<int, double>>::iterator
                        currReverseIter = reverseXTIter++;

                std::tuple<int, double> currReverseTuple =
                    (*currReverseIter).second;
                int currentEraseId =
                    std::get<HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE -
                    1>(currReverseTuple);
                // removes all even values
                if (currentEraseId ==
                        currentSourceNodeFwdS->GetIntval())
                {
                    copyMultimapReverseSearchXT.erase(
                        currReverseIter);
                    #ifdef USEDEBUG
                    double tmpDist =
                        std::get<HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE - 1>
                        (currReverseTuple);
                    LogDebug("erased s (=t) itself from XT: x=" <<
                          (*currReverseIter).first << ", t=" <<
                          currentEraseId << " dist: " << tmpDist << endl);
                    #endif
                }
            }

            //Check whether there are direct edges to some of the
            // current target-endpoints
            double currDistSX = 0.0;
            double distSV = hlGetWeightedCost(
                                HL_DEFAULT_COST_MODE, currentEdgeTargetTupleSV);
            hlForwardSearchCheckForWitnessPath(
                copyMultimapReverseSearchXT,
                currentVertexIdFwdV->GetIntval(),
                currentSourceNodeFwdS->GetIntval(), currDistSX,
                distSV);

            //Check whether there are vertices to be found left, else go
            // to next loop loop
            if(copyMultimapReverseSearchXT.size() == 0)
            {
                //Free Resources
                currentEdgeTargetTupleSV->DeleteIfAllowed();
                currentEdgeTargetTupleSV = 0;

                #ifdef USEDEBUG
                LogDebug("all witnesses found continue with next SV edge"
                      << endl);
                #endif
                continue;
            }

            //Build list containing all still visited nodes during
            // current forward search starting at current s
            std::unordered_set<int> stillVisitedNodesSet;
            stillVisitedNodesSet.insert(
                currentSourceNodeFwdS->GetIntval());

            //process forward search for current source-tuple s
            hlForwardSearchProcessIncomingEdge(
                orelEdgesSource,
                shortcutsToBeCreatedOrelSourceToBeDeleted,
                currentVertexIdFwdV->GetIntval(),
                currentSourceNodeFwdS, stillVisitedNodesSet,
                copyMultimapReverseSearchXT, hDepth, distSV);

            //Free Incoming-Iteration
            currentEdgeTargetTupleSV->DeleteIfAllowed();
            currentEdgeTargetTupleSV = 0;
            currentEdgeTargetTupleSV =
                relEdgesTargetIterSV->GetNextTuple();

            copyMultimapReverseSearchXT.clear();
        }
        delete relEdgesTargetIterSV;

        return true;
    }

    /*
     * Procession of iteration over all incoming Nodes s to current
     *  vertex v to be contracted
     *  of current forward search ( @see hlHHopForwardSearch).
     * Does a forward search starting from this current incoming node s.
     * The aim is to find other nodes x during the forward search which
     *  exist in copyMultimapReverseSearchXT
     *  and for which then there exist a shorter path from s to t via x
     *7than via v.
     * If there exist a shorter path all occurrences of x will be eraised
     *7 from copyMultimapReverseSearchXT.
     * Remaining occurences of t within copyMultimapReverseSearchXT will
     *  lead to the creation of a shortcut.
     * These then are stored in shortcutsToBeCreatedOrelSourceToBeDeleted.
     *
     * @param orelEdgesSource is an OrderedRelation containging all edges
     *  of the underlying graph sorted by the field Source
     * @param shortcutsToBeCreatedOrelSourceToBeDeleted the multimap by
     *  reference where all shortcuts will be stored in
     * @param currentVertexIdFwdV the current vertex v to be contracted
     * @param currentSourceNodeFwdS the current source vertex s
     * @param stillVisitedNodesSet is an unordered_set by reference
     *  containging all nodes still visited during forward search
     * @param copyMultimapReverseSearchXT copy of the Ordered map by
     *  reference where all distances found by the reverse search have
     *  been stored in
     * @param hDepth the h-hop-size to be used, will be overwritten
     *  by @see HL_DEFAULT_H_DEPTH if value < 1
     * @param distSV the weighted costs of the edge from curent s to current v
     * @return true, since the result of this function will be stored
     *  directly in @see shortcutsToBeCreatedOrelSourceToBeDeleted
     *
    */
    bool hlForwardSearchProcessIncomingEdge(
        OrderedRelation* orelEdgesSource,
        std::multimap<int, std::tuple<int, double, int>>
        &shortcutsToBeCreatedOrelSourceToBeDeleted,
        int currentVertexIdFwdV,
        CcInt* currentSourceNodeFwdS,
        std::unordered_set<int> &stillVisitedNodesSet,
        std::multimap<int, std::tuple<int, double>>
        &copyMultimapReverseSearchXT, int hDepth,
        double distSV) const
    {
        //Stack for Dijkstra containing not yet visited nodes
        std::multimap<double, std::tuple<int, int>>
                notYetVisitedNodesMultiMap;

        #ifdef USEDEBUG
        LogDebug("Do initial" << endl);
        #endif
        hlForwardSearchProcessIncomingEdgeInitialSteps(
            orelEdgesSource, currentVertexIdFwdV,
            currentSourceNodeFwdS, notYetVisitedNodesMultiMap,
            copyMultimapReverseSearchXT, distSV);

        #ifdef USEDEBUG
        LogDebug("Do iterative" << endl);
        #endif
        hlForwardSearchProcessIncomingEdgeIterativeSteps(
            orelEdgesSource, currentVertexIdFwdV,
            notYetVisitedNodesMultiMap, stillVisitedNodesSet,
            copyMultimapReverseSearchXT, hDepth, distSV);

        #ifdef USEDEBUG
        LogDebug("Do create shortcuts" << endl);
        #endif
        hlForwardSearchCreateAndAppendShortcuts(
            shortcutsToBeCreatedOrelSourceToBeDeleted,
            copyMultimapReverseSearchXT,
            currentSourceNodeFwdS->GetIntval(),
            currentVertexIdFwdV, distSV);

        return true;
    }

    /*
     * Does initial, non-iterative Steps as part of
     *  @see hlForwardSearchProcessIncomingEdge.
     * At least initializes the search-stack with all other vertexes w
     *  outgoing from s but ignoring current v to be contracted.
     *
     * @param orelEdgesSource is an OrderedRelation containging all edges
     *  of the underlying graph sorted by the field Source
     * @param currentVertexIdFwdV the current vertex v to be contracted
     * @param currentSourceNodeFwdS the current source vertex s
     * @param notYetVisitedNodesMultiMap is a Multimap by reference
     *  containging all nodes scanned but not yet visited during forward search
     * @param copyMultimapReverseSearchXT copy of the Ordered map by
     *  reference where all distances found by the reverse search
     *  have been stored in
     * @param distSV the weighted costs of the edge from curent s to current v
     * @return true, since the result of this function is the modification
     *  of notYetVisitedNodesMultiMap and copyMultimapReverseSearchXT
     *
    */
    bool hlForwardSearchProcessIncomingEdgeInitialSteps(
        OrderedRelation* orelEdgesSource,
        int currentVertexIdFwdV,
        CcInt* currentSourceNodeFwdS,
        std::multimap<double, std::tuple<int, int>>
        &notYetVisitedNodesMultiMap,
        std::multimap<int, std::tuple<int, double>>
        &copyMultimapReverseSearchXT, double distSV) const
    {
        //Prepare Range scan
        //Create From and to by currentSourceNodeFwdS (=s)
        std::vector<void*> vecAttributesFromFwdS(1);
        vecAttributesFromFwdS[0] = currentSourceNodeFwdS;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesFromFwdS(1);
        vecAttrTypesFromFwdS[0] =
            currentSourceNodeFwdS->getSMIKeyType();
        CompositeKey fromFwdS(vecAttributesFromFwdS,
                              vecAttrTypesFromFwdS, false);

        CcInt currentSourceNodePlusOneFwdS(true,
                                currentSourceNodeFwdS->GetIntval() + 1);
        CcInt* currentSourceNodePlusOnePtrFwdS =
            &currentSourceNodePlusOneFwdS;
        std::vector<void*> vecAttributesToFwdS(1);
        vecAttributesToFwdS[0] =
            currentSourceNodePlusOnePtrFwdS;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesToFwdS(1);
        vecAttrTypesToFwdS[0] =
            currentSourceNodePlusOnePtrFwdS->getSMIKeyType();
        CompositeKey toFwdS(vecAttributesToFwdS,
                            vecAttrTypesToFwdS, false);

        //get all outgoing edges (s, w) of current sourceNode
        // s to any other node w
        GenericRelationIterator* relEdgesSourceIterFwdSW =
            orelEdgesSource->MakeRangeScan(fromFwdS, toFwdS);

        //Get first Tuple of them =(s, w)
        Tuple* currentEdgeSourceTupleFwdSW =
            relEdgesSourceIterFwdSW->GetNextTuple();

        //Iterate over all outgoing Tuples from s
        while(currentEdgeSourceTupleFwdSW)
        {
            //Get ID of current TargetNode (=w)
            CcInt* currentTargetNodeW = (CcInt*)
                            currentEdgeSourceTupleFwdSW->GetAttribute(
                                            HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);

            //ignore current v to be contracted
            if(currentTargetNodeW->GetIntval() !=
                    currentVertexIdFwdV)
            {
                //Check whether there are direct edges to some
                // of the current target-endpoints
                double currDistSW = hlGetWeightedCost(
                                        HL_DEFAULT_COST_MODE,
                                        currentEdgeSourceTupleFwdSW);
 //TODO get Distance from currentSourceNode (s) to currentTargetNodeW (w)
                hlForwardSearchCheckForWitnessPath(
                    copyMultimapReverseSearchXT, currentVertexIdFwdV,
                    currentTargetNodeW->GetIntval(), currDistSW,
                    distSV);

                //Check whether there are vertices to be found left,
                // else abort loop
                if(copyMultimapReverseSearchXT.size() == 0)
                {
                    //Free Resources
                    currentEdgeSourceTupleFwdSW->DeleteIfAllowed();
                    currentEdgeSourceTupleFwdSW = 0;

                    #ifdef USEDEBUG
                    LogDebug("all witnesses found, break initial steps"
                          << endl);
                    #endif
                    break;
                }

                std::tuple<double, int, int>
                currMinNotYetVisitedNodeTuple = std::make_tuple(
                                0.0, currentSourceNodeFwdS->GetIntval(), 0);
                #ifdef USEDEBUG
                LogDebug("forward search initial steps insertOrUpdateInNotYet"
                      << endl);
                #endif
                bool isForward = true;
                bool isHHop = true;
                hlInsertOrUpdateTupleInNotYetVisitedList(
                    currentVertexIdFwdV,
                    currMinNotYetVisitedNodeTuple,
                    currentEdgeSourceTupleFwdSW,
                    notYetVisitedNodesMultiMap, isForward,
                    isHHop); //TODO: übergeben von 0 als null in ordnung?
            }

            //Free Outgoing-Iteration
            currentEdgeSourceTupleFwdSW->DeleteIfAllowed();
            currentEdgeSourceTupleFwdSW = 0;
            currentEdgeSourceTupleFwdSW =
                relEdgesSourceIterFwdSW->GetNextTuple();
        }
        delete relEdgesSourceIterFwdSW;

        return true;
    }

    /*
     * Does iterative Steps as part of @see hlForwardSearchProcessIncomingEdge.
     * At least gets next min vertex w of notYetVisitedNodesOrel
     *  and scans next vertices x having an edge e(w, x) outgoing from
     *  w to x with respect to the hop depth.
     * If the next vertex to scan is more hops (in the meaning of graph depth)
     *  away than the h-hop-size allows,
     *  this next vertex will not be scanned.
     *
     * @param orelEdgesSource is an OrderedRelation containging all edges of
     *  the underlying graph sorted by the field Source
     * @param currentVertexIdFwdV the current vertex v to be contracted
     * @param notYetVisitedNodesMultiMap is a Multimap by refeerence
     *  containging all nodes scanned but not yet visited during forward search
     * @param stillVisitedNodesSet is an unordered_set bx reference
     *  containging all nodes still visited during forward search
     * @param copyMultimapReverseSearchXT copy of the Ordered map by
     *  reference where all distances found by the reverse search have been
     *  stored in
     * @param hDepth the h-hop-size to be used, will be overwritten
     *  by @see HL_DEFAULT_H_DEPTH if value < 1
     * @param distSV the weighted costs of the edge from curent s to current v
     * @return true, since the result of this function is the modification
     *  of notYetVisitedNodesMultiMap, stillVisitedNodesSet and
     *  copyMultimapReverseSearchXT
     *
    */
    bool hlForwardSearchProcessIncomingEdgeIterativeSteps(
        OrderedRelation* orelEdgesSource,
        int currentVertexIdFwdV,
        std::multimap<double, std::tuple<int, int>>
        &notYetVisitedNodesMultiMap,
        std::unordered_set<int> &stillVisitedNodesSet,
        std::multimap<int, std::tuple<int, double>>
        &copyMultimapReverseSearchXT, int hDepth,
        double distSV) const
    {
        #ifdef USEDEBUG
        LogDebug("notYetVisitedNodesMultiMap.size(): " <<
              notYetVisitedNodesMultiMap.size() <<
              " copyMultimapReverseSearchXT.size(): " <<
              copyMultimapReverseSearchXT.size() << endl);
        #endif
        //Do forward Dijkstra
        //Abort when either there are no more vertices
        // to scan (with respect to the hop-depth)
        // or when there are no more target-vertices to find
        // (because thei still have been found)
        while(notYetVisitedNodesMultiMap.size() > 0 &&
                copyMultimapReverseSearchXT.size() > 0)
        {
            //get Tuple with min(dist)
            std::multimap<double, std::tuple<int, int>>::iterator
                    notYetVisitedNodesMultiMapIter =
                        notYetVisitedNodesMultiMap.begin();
            double currMinDist =
                (*notYetVisitedNodesMultiMapIter).first;

            std::tuple<int, int> currTuple =
                (*notYetVisitedNodesMultiMapIter).second;
            int currMinNodeId =
                std::get<HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED -
                1>(currTuple);
            int currMinHopDepth =
                std::get<HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED
                - 1>(currTuple);

            //Create auxiliary tuple
            std::tuple<double, int, int>
            currMinNotYetVisitedNodeTuple = std::make_tuple(
                                currMinDist, currMinNodeId, currMinHopDepth);
            #ifdef USEDEBUG
            LogDebug("next minW: " << currMinNodeId << " - " <<
                  currMinDist << " - " << currMinHopDepth << endl);
            #endif

            //Remove currMinNotYetVisitedNodeTuple from notYetVisitedNodesOrel
            notYetVisitedNodesMultiMap.erase(
                notYetVisitedNodesMultiMapIter);

            //Add currMinNotyetVisitedNodeTuple to stillVisitedNodesList
            stillVisitedNodesSet.insert(currMinNodeId);

            //Scan new vertices reachable (with respect to the hop depth)
            // and either insert them into notYetVisitedNodesOrel
            // or update existing entries when there are shorter pathes
            // TODO: der Code geht davon aus, dass edie kleinstmöglich
            // hHop Größe = 1 ist
            if( currMinHopDepth < hDepth)
            {
                hlForwardSearchIterativeStepsScanNewVertices(
                    orelEdgesSource, currentVertexIdFwdV,
                    currMinNotYetVisitedNodeTuple,
                    notYetVisitedNodesMultiMap, stillVisitedNodesSet,
                    copyMultimapReverseSearchXT, distSV);
            }
        }
        #ifdef USEDEBUG
        LogDebug("finish of hlForwardSearchProcessIncomingEdgeIterativeSteps"
              << endl);
        #endif
        return true;
    }

    /*
     * Scans for new vertices x reachable from the current vertex w.
     * Adds new vertices scanned in notYetVisitedNodesMultiMap.
     *
     * @param orelEdgesSource is an OrderedRelation containging all edges
     *  of the underlying graph sorted by the field Source
     * @param currentVertexIdFwdV the current vertex v to be contracted
     * @param currMinNotYetVisitedNodeTupleFwdW the current tuple w by
     *  reference with minimum distance to the current source node
     * @param notYetVisitedNodesMultiMap is an multi map by reference
     *  containging all nodes scanned but not yet visited during forward search
     * @param stillVisitedNodesSet is an unordered_set by reference
     *  containging all nodes still visited during forward search
     * @param copyMultimapReverseSearchXT copy of the Ordered map by
     *  reference where all distances found by the reverse search have
     *  been stored in
     * @param distSV the weighted costs of the edge from curent s to current v
     * @return true, since the result of this function is the modification of
     *  notYetVisitedNodesMultiMap and copyMultimapReverseSearchXT
     *
    */
    bool hlForwardSearchIterativeStepsScanNewVertices(
        OrderedRelation* orelEdgesSource,
        int currentVertexIdFwdV,
        std::tuple<double, int, int>
        &currMinNotYetVisitedNodeTupleFwdW,
        std::multimap<double, std::tuple<int, int>>
        &notYetVisitedNodesMultiMap,
        std::unordered_set<int> &stillVisitedNodesSet,
        std::multimap<int, std::tuple<int, double>>
        &copyMultimapReverseSearchXT, double distSV) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlForwardSearchIterativeStepsScanNewVertices"
              << endl);
        #endif
        //Get ID of current SourceNode (=w)
        double currentSourceNodeDistSW =
            std::get<HL_INDEX_OF_DIST_IN_NOT_YET_VISITED>
            (currMinNotYetVisitedNodeTupleFwdW);
        int currentSourceNodeFwdW =
            std::get<HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED>
            (currMinNotYetVisitedNodeTupleFwdW);

        //iterate over all outgoing edges (w, x) from w
        //Prepare Range scan
        //Create From and to by currentSourceNodeFwdW (=w)
        CcInt currentSourceNodeFwdWCcInt(true,
                                         currentSourceNodeFwdW);
        CcInt* currentSourceNodePtrFwdW =
            &currentSourceNodeFwdWCcInt;
        std::vector<void*> vecAttributesFromFwdW(1);
        vecAttributesFromFwdW[0] =
            currentSourceNodePtrFwdW;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesFromFwdW(1);
        vecAttrTypesFromFwdW[0] =
            currentSourceNodePtrFwdW->getSMIKeyType();
        CompositeKey fromFwdW(vecAttributesFromFwdW,
                              vecAttrTypesFromFwdW, false);

        CcInt currentSourceNodePlusOneFwdW(true,
                                           currentSourceNodeFwdW + 1);
        CcInt* currentSourceNodePlusOnePtrFwdW =
            &currentSourceNodePlusOneFwdW;
        std::vector<void*> vecAttributesToFwdW(1);
        vecAttributesToFwdW[0] =
            currentSourceNodePlusOnePtrFwdW;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesToFwdW(1);
        vecAttrTypesToFwdW[0] =
            currentSourceNodePlusOnePtrFwdW->getSMIKeyType();
        CompositeKey toFwdW(vecAttributesToFwdW,
                            vecAttrTypesToFwdW, false);

        //get all outgoing edges (w, x) of current sourceNode w
        GenericRelationIterator* relEdgesSourceIterWX =
            orelEdgesSource->MakeRangeScan(fromFwdW, toFwdW);

        //Get first Tuple of them
        Tuple* currentEdgeSourceTupleWX =
            relEdgesSourceIterWX->GetNextTuple();

        //Iterate over all outgoing Tuples (x) from w
        while(currentEdgeSourceTupleWX)
        {
            CcInt* currentTargetNodeX = (CcInt*)
                                        currentEdgeSourceTupleWX->GetAttribute(
                                            HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
            #ifdef USEDEBUG
            LogDebug("next iteration: currentTargetNodeX: " <<
                  currentTargetNodeX->GetIntval() << endl);
            #endif

            //ignore current v to be contracted and also ignore still
            // visited nodes
            if(currentTargetNodeX->GetIntval() !=
                    currentVertexIdFwdV &&
                    stillVisitedNodesSet.count(
                        currentTargetNodeX->GetIntval()) == 0)
            {
                double currDistSX = (double)
                                    currentSourceNodeDistSW + hlGetWeightedCost(
                                        HL_DEFAULT_COST_MODE,
                                        currentEdgeSourceTupleWX); //TODO

                //Check whether there are direct edges to some of the
                // current target-endpoints
                hlForwardSearchCheckForWitnessPath(
                    copyMultimapReverseSearchXT, currentVertexIdFwdV,
                    currentTargetNodeX->GetIntval(), currDistSX,
                    distSV); //TODO weitere parameter
                //Check whether there are vertices to be found left,
                // else abort loop
                if(copyMultimapReverseSearchXT.size() == 0)
                {
                    #ifdef USEDEBUG
                    LogDebug("all witnesses found break scan new vertices"
                     " of forward search"
                          << endl);
                    #endif
                    //Free Resources
                    currentEdgeSourceTupleWX->DeleteIfAllowed();
                    currentEdgeSourceTupleWX = 0;

                    break;
                }

                #ifdef USEDEBUG
                LogDebug("forward search scan new vertices"
                 " insertOrUpdateInNotYet"
                      << endl);
                #endif
                //Either insert the new scanned X into not visited list
                // or update it if it sill exists and has a shorter path
                // via current w
                bool isForward = true;
                bool isHHop = true;
                hlInsertOrUpdateTupleInNotYetVisitedList(
                    currentVertexIdFwdV,
                    currMinNotYetVisitedNodeTupleFwdW,
                    currentEdgeSourceTupleWX,
                    notYetVisitedNodesMultiMap, isForward, isHHop);
            }

            //Free Outgoing-Iteration
            currentEdgeSourceTupleWX->DeleteIfAllowed();
            currentEdgeSourceTupleWX = 0;
            currentEdgeSourceTupleWX =
                relEdgesSourceIterWX->GetNextTuple();
        }

        delete relEdgesSourceIterWX;

        #ifdef USEDEBUG
        LogDebug("finish hlForwardSearchIterativeStepsScanNewVertices"
              << endl);
        #endif

        return true;
    }

    /*
     * Either insert the new scanned vertex x into notYetVisitedList
     *  or update it if it sill exists but has a shorter path via current w.
     * If currMinNotYetVisitedNodeTupleFwdW is null then its meant that the
     *  current tuple is equal the current starting node s.
     * It processe different then regarding the hop-depth and the distance to
     *  the source node (so to itself = 0).
     *  If x is going to be inserted or updated e.g. its distance is calculated
     *  by the distance of the given vertex w so far
     *   and the distance of the also given edge leading to x.
     * All descriptions behave vice versa if isForward = false.
     *
     * @param currentVertexIdFwdV the current vertex v to be contracted
               used to skip iteration if next vertex considered is equeal to v
               can be set to -1 when no skipping shall be used
     * @param currMinNotYetVisitedNodeTupleFwdW the current tuple w with the
     *  current minimum distance to the actual source s by reference
               it values are used to create those one for the node x which
               *  might be inserted or updaten within notYet
     * @param currentEdgeSourceTupleWX the current edge with source = w
     *  ( @see currMinNotYetVisitedNodeTupleFwdW)
     *         and target = x where x may be inserted or updated within notYet
     * @param notYetVisitedNodesMultiMap is an multi map by reference
     *  containging all nodes scanned but not yet visited during forward search
     * @param isForward is a boolean flag to indicate whether the given
     *  edgeOrelTuple is used during a forward or reverse search
     * @return true, since the result of this function is the modification
     *  of notYetVisitedNodesMultiMap
     *
    */
    bool hlInsertOrUpdateTupleInNotYetVisitedList(
        int currentVertexIdFwdV,
        std::tuple<double, int, int>
        &currMinNotYetVisitedNodeTupleFwdW,
        Tuple* currentEdgeSourceTupleWX,
        std::multimap<double, std::tuple<int, int>>
        &notYetVisitedNodesMultiMap, bool isForward,
        bool isHHop) const
    {
        #ifdef USEDEBUG
LogDebug("doing hlInsertOrUpdateTupleInNotYetVisitedList"
              << endl);
#endif
        //Get ID of current TargetNode (=x)
        CcInt* currentTargetNodeX;
        if(isForward)
        {
            currentTargetNodeX = (CcInt*)
                                 currentEdgeSourceTupleWX->GetAttribute(
                                     HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
        }
        else
        {
            currentTargetNodeX = (CcInt*)
                                 currentEdgeSourceTupleWX->GetAttribute(
                                     HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
        }

        //Ignore if x = v (current Node v to be contracted)
        if(currentVertexIdFwdV !=
                currentTargetNodeX->GetIntval())
        {
            double parentDistSW =
                std::get<HL_INDEX_OF_DIST_IN_NOT_YET_VISITED>
                (currMinNotYetVisitedNodeTupleFwdW);
            int parentNodeId =
                std::get<HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED>
                (currMinNotYetVisitedNodeTupleFwdW);
            int parentHopDepth =
                std::get<HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED>
                (currMinNotYetVisitedNodeTupleFwdW);

            double currDistSWX = parentDistSW +
                                 hlGetWeightedCost(HL_DEFAULT_COST_MODE,
                                                   currentEdgeSourceTupleWX);
            int currentHopDepthSX = parentHopDepth + 1;

            //If the next vertex to be added to notYetVisited List still
            // exists there
            // we have to check whether theres a shorter path to it via
            // current W
            bool insertOrUpdate = true;
            double currDistSX = 0.0;

            for (std::multimap<double, std::tuple<int, int>>::iterator
                    notYetVisitedNodesMultiMapIter =
                        notYetVisitedNodesMultiMap.begin();
                    notYetVisitedNodesMultiMapIter !=
                    notYetVisitedNodesMultiMap.end();
                    ++notYetVisitedNodesMultiMapIter)
            {
                currDistSX =
                    (*notYetVisitedNodesMultiMapIter).first;
                std::tuple<int, int> currMultiMapTuple =
                    (*notYetVisitedNodesMultiMapIter).second;
                int currId =
                    std::get<HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED -
                    1>(currMultiMapTuple);

                //just get edges (v, t) and skipp edges (x, t)
                if(currId == currentTargetNodeX->GetIntval())
                {
                    //check if distance and hopdepth needs to be updated
                    // within notyetvisitedlist
                    if(currDistSWX < currDistSX)
                    {
                        //remove old entry for inserting new one (which is
                        //like an update)
                        #ifdef USEDEBUG
                        LogDebug("update: erasing old entry for reinserting"
                              << endl);
                        #endif
                        notYetVisitedNodesMultiMap.erase(
                            notYetVisitedNodesMultiMapIter);
                    }
                    else
                    {
                        //Do not insert or update because still existing
                        // entry is better
                        insertOrUpdate = false;
                    }
                    //break loop, element was found
                    break;
                }
            }

            if(insertOrUpdate)
            {
                #ifdef USEDEBUG
LogDebug("going to insert new element to notYet : "
                      << currentTargetNodeX->GetIntval() << " - " <<
                      currDistSWX << endl);
#endif

                //during Label creation we need the parentNodeId but not
                // the hopDepth so we reuse the hHop field for this
                int hHopOrParentId = -1;
                if(isHHop)
                {
                    hHopOrParentId = currentHopDepthSX;
                }
                else
                {
                    hHopOrParentId = parentNodeId;
                }
                std::tuple<int, double> insertTuple =
                    std::make_tuple(currentTargetNodeX->GetIntval(),
                                    hHopOrParentId);
                notYetVisitedNodesMultiMap.insert(
                    pair<double, std::tuple<int, int>>(currDistSWX,
                                                       insertTuple));
            }
        }

        #ifdef USEDEBUG
LogDebug("finish hlInsertOrUpdateTupleInNotYetVisitedList"
              << endl);
#endif

        return true;
    }

    /*
     * Checks for a given vertex x whether a witness-path to endpoints
     *  from the previous reverse search can be found.
     * Deletes the endpoints then within the given endpoint-list.
     * A Witness-Path is a shorter path from start node s to target node t
     *  than the potential shortcut path from s to t via v
     *  (the node to be contracted).
     *
     * @param copyMultimapReverseSearchXT copy of the Ordered map by reference
     *  where all distances found by the reverse search have been stored in
     * @param currentContractNodeV the current vertex v to be contracted
     * @param currentSourceNodeWitnessX the current vertex x to be found
     * @param distSX the weighted costs of the edge from curent s to current x
     * @param distSV the weighted costs of the edge from curent s to current v
     * @return true, since the result of this function is the modification of
     *  copyMultimapReverseSearchXT
     *
    */
    bool hlForwardSearchCheckForWitnessPath(
        std::multimap<int, std::tuple<int, double>>
        &copyMultimapReverseSearchXT,
        int currentContractNodeV,
        int currentSourceNodeWitnessX, double distSX,
        double distSV) const
    {
        #ifdef USEDEBUG
LogDebug("check for witness start" << endl);
#endif
        //Vector containing node ids to be deleted from orelXT after
        // finishing full iteration
        std::vector<int> nodesToDeleteVector;

        //get all outgoing edges (x, t) of current sourceNode x
        std::pair <std::multimap<int, std::tuple<int, double>>::iterator,
         std::multimap<int, std::tuple<int, double>>::iterator>
                revXTRangeBoundsIter;
        revXTRangeBoundsIter =
            copyMultimapReverseSearchXT.equal_range(
                currentSourceNodeWitnessX);

        for (std::multimap<int, std::tuple<int, double>>::iterator
                revXTRangeIter = revXTRangeBoundsIter.first;
                revXTRangeIter != revXTRangeBoundsIter.second;
                ++revXTRangeIter)
        {

            std::tuple<int, double> currTupleXT =
                (*revXTRangeIter).second;
            int currentEdgeReverseAttributeIdT =
                std::get<HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE -
                1>(currTupleXT);
            double currentEdgeReverseAttributeDistXT =
                std::get<HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE - 1>
                (currTupleXT);

            #ifdef USEDEBUG
LogDebug("xt found (x = " <<
                  currentSourceNodeWitnessX << ", t = " <<
                  currentEdgeReverseAttributeIdT << ")" << endl);
#endif

            double distSXT = (double) distSX +
                             currentEdgeReverseAttributeDistXT;
            double distSVT = (double) distSV +
                             hlForwardSearchGetDistVT(
                            copyMultimapReverseSearchXT, currentContractNodeV,
                                 currentEdgeReverseAttributeIdT);

            if(distSXT <= distSVT)
            {
                #ifdef USEDEBUG
LogDebug("witness found" << endl);
#endif
                //Witnesspath found, remove all Tuples with target = t
                // from current WitnessList (= copyMultimapReverseSearchXT)
                //Build list containing all nodes to be deleted later after
                // iteration has finished
                nodesToDeleteVector.push_back(
                    currentEdgeReverseAttributeIdT);
            }
        }

        for(std::vector<int>::iterator vecIter =
                    nodesToDeleteVector.begin();
                vecIter != nodesToDeleteVector.end(); ++vecIter)
        {
            #ifdef USEDEBUG
LogDebug("going to erase from XT: " << *vecIter <<
                  endl);
#endif
            //emtpy increment section since its incremented manually
            // next to the erase-command
            for (std::multimap<int, std::tuple<int, double>>::iterator
                    reverseXTIter =
                        copyMultimapReverseSearchXT.begin();
                    reverseXTIter !=
                    copyMultimapReverseSearchXT.end();)
            {
                // you have to do this because iterators are invalidated
                std::multimap<int, std::tuple<int, double>>::iterator
                        currReverseIter = reverseXTIter++;

                std::tuple<int, double> currReverseTuple =
                    (*currReverseIter).second;
                int currentEraseId =
                    std::get<HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE -
                    1>(currReverseTuple);
                // removes all even values
                if (currentEraseId == *vecIter)
                {
                    copyMultimapReverseSearchXT.erase(
                        currReverseIter);
                    #ifdef USEDEBUG
LogDebug("erased witness from XT x= " <<
                          (*currReverseIter).first << " t = " <<
                          currentEraseId << endl);
#endif
                }
            }
        }
        #ifdef USEDEBUG
LogDebug("finish check for witness" << endl);
#endif

        return true;
    }

    /*
     * Deletes every tuple from the given orel containing the given
     *  nodeId at the given index position.
     *
     * @param currentOrel the OrderedRelation where all tuples found by
     *  the search will be deleted
     * @param givenNodeId the Node ID of the vertex t to bedeleted
     * @param index the index position ath which the ID field should be found
     * @return true, since the result of this function is the modification
     *  of currentOrel
     *
    */
    bool hlRemoveTuplesFromOrel(OrderedRelation*
                                currentOrel, int givenNodeId, int index) const
    {
        //iterate over all edges (x, t)

        //get all edges (x, t)
        GenericRelationIterator* currentOrelIter =
            currentOrel->MakeScan(); //TODO: hier werden nur die tuples mit
            // target = t benötigt, die OREL ist aber nach Source (x) sortiert,
            // daher kompletter scan, ggf. auch filter möglich oder wäre der
            // auch nicht schneller weil auch nur iteration?

        //Get first Tuple of them
        Tuple* currentTuple =
            currentOrelIter->GetNextTuple(); //TODO weil hier nun ein make
            // scan verwendet werden muss, um tuples nach ihrem Target zu
            // finden, entsteht hier zeitgleich lesender und schreibender
            // Zugriff, wie anders gestalten?
        //TODO ggf. die zuvor ausgeführte vectorenliste mit den tuple objekten
        // statt den ids füllen und beim durchlaufen des vectors diese tuples
        // dann direkt löschen?
        //TODO: problem dann: nicht ALLE mit target = t werden dann gefunden,
        // sondern nur die einzelnen
        //TODO: ggf. dann hier aus der iterationalle tuples in einen array
        // schreiben, und nach dem make scan erst erneut iterieren zum
        // direkten löschen?

        //Iterate over all outgoing Tuples (x, t) from x
        while(currentTuple)
        {
            CcInt* currentNodeId = (CcInt*)
                                   currentTuple->GetAttribute(index);

            //remove all tuples incoming to t because witness path
            // has been found
            if(currentNodeId->GetIntval() == givenNodeId)
            {
                currentOrel->DeleteTuple(currentTuple, true);
            }

            //Free Iteration
            currentTuple->DeleteIfAllowed();
            currentTuple = 0;
            currentTuple = currentOrelIter->GetNextTuple();
        }

        delete currentOrelIter;
        return true;
    }

    /*
     * Gets the Distance between the given curent node v to be contracted
     *  and the given current target node t by extracting it from the given
     *  orel created by the reverse search.
     *
     * @param copyMultimapReverseSearchXT copy of the OrderedRelation where
     *  all distances found by the reverse search have been stored in
     * @param currentContractNodeV the Node ID of the vertex t to contracted
     * @param currentTargetNodeWitnessT the Node ID of the vertex t to bedeleted
     * @return true, since the result of this function is the modification of
     *  copyMultimapReverseSearchXT
     *
    */
    double hlForwardSearchGetDistVT(
        std::multimap<int, std::tuple<int, double>>
        &copyMultimapReverseSearchXT,
        int currentContractNodeV,
        int currentTargetNodeWitnessT) const
    {
        double retVal = -1;

        std::pair <std::multimap<int, std::tuple<int, double>>::iterator,
         std::multimap<int, std::tuple<int, double>>::iterator>
                revXTRangeBoundsIter;
        revXTRangeBoundsIter =
            copyMultimapReverseSearchXT.equal_range(
                currentContractNodeV);

        for (std::multimap<int, std::tuple<int, double>>::iterator
                revXTRangeIter = revXTRangeBoundsIter.first;
                revXTRangeIter != revXTRangeBoundsIter.second;
                ++revXTRangeIter)
        {
            std::tuple<int, double> currTupleXT =
                (*revXTRangeIter).second;
            int currentEdgeSourceAttributeT =
                std::get<HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE -
                1>(currTupleXT);

            if(currentEdgeSourceAttributeT ==
                    currentTargetNodeWitnessT)
            {
                retVal = std::get<HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE
                         - 1>(currTupleXT);
                break;
            }
        }

        return retVal;
    }

    /*
     * Creates an shortcut for every entry of the given second multimap and
     *  appends it to the first given orel
     *
     * @param shortcutsToBeCreatedOrelSourceToBeDeleted the multimap by
     *  reference containing all shortcuts to be added to the original
     *  edges OrderedRelations
     * @param copyMultimapReverseSearchXT copy of the Ordered multimap by
     *  reference where all distances found by the reverse search have been
     *  stored in
     * @param currentSourceNodeS the Node ID of the source node s
     * @param currentContractNodeV the Node ID of the vertex t to contracted
     * @param distSV the weighted costs of the edge from curent s to current v
     * @return true, since the result of this function is the modification of
     *  shortcutsToBeCreatedOrelSourceToBeDeleted
     *
    */
    bool hlForwardSearchCreateAndAppendShortcuts(
        std::multimap<int, std::tuple<int, double, int>>
        &shortcutsToBeCreatedOrelSourceToBeDeleted,
        std::multimap<int, std::tuple<int, double>>
        &copyMultimapReverseSearchXT,
        int currentSourceNodeS, int currentContractNodeV,
        double distSV) const
    {
        #ifdef USEDEBUG
LogDebug("start hlForwardSearchCreateAndAppendShortcuts"
              << endl);
#endif
        //Iterate over all Tuples (x, t) where x = v
        std::pair <std::multimap<int, std::tuple<int, double>>::iterator,
         std::multimap<int, std::tuple<int, double>>::iterator>
                revXTRangeBoundsIter;
        revXTRangeBoundsIter =
            copyMultimapReverseSearchXT.equal_range(
                currentContractNodeV);
        for (std::multimap<int, std::tuple<int, double>>::iterator
                reverseXTMultimapIter =
                    revXTRangeBoundsIter.first;
                reverseXTMultimapIter !=
                revXTRangeBoundsIter.second;
                ++reverseXTMultimapIter)
        {
            std::tuple<int, double> currTuple =
                (*reverseXTMultimapIter).second;
            /*int currentEdgeSourceIDX =
                (*reverseXTMultimapIter).first;
               */
            int currentEdgeSourceIDT =
                std::get<HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE -
                1>(currTuple);
            double currentEdgeSourceDistVT =
                std::get<HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE - 1>
                (currTuple);

            //create shortcut and add to
            // shortcutsToBeCreatedOrelSourceToBeDeleted
            #ifdef USEDEBUG
LogDebug("insert new shortcut to multimap (" <<
                  currentSourceNodeS << ", " << currentEdgeSourceIDT
                  << ", " << distSV + currentEdgeSourceDistVT <<
                  ")"<< endl);
#endif
            std::tuple<int, double, int> insertTuple =
                std::make_tuple(currentEdgeSourceIDT,
                                distSV + currentEdgeSourceDistVT,
                                currentContractNodeV);
            shortcutsToBeCreatedOrelSourceToBeDeleted.insert(
                pair<int, std::tuple<int, double, int>>
                (currentSourceNodeS, insertTuple));
        }
        #ifdef USEDEBUG
LogDebug("finish hlForwardSearchCreateAndAppendShortcuts"
              << endl);
#endif
        return true;
    }

    /*
     * Definition of Field-Indexes for ranked nodes-orel
    */
    static const int HL_INDEX_OF_ID_IN_NODES_RANKED =
        2;
    static const int HL_INDEX_OF_RANK_IN_NODES_RANKED
        = 4;

    /*
     * Iterates over all nodes of the given Orel nodesWithRankOrelRank
     *  which is expected to be sorted by Rank.
     *  These nodes will be contracted in their particular iteration.
     * Creates a copy of the orels edgesWithViaOrelSource and
     *  edgesWithViaOrelTarget (outside the following loop).
     * Calls for each iteration hlDoContraction which returns the
     *  shortcuts to be created.
     * Adds the shortcuts to be created to the copies of the Orels
     *  edgesWithViaOrelTarget and edgesWithViaOrelSource
     *  (inside the particular iteration).
     * Adds the same shortcuts to be created to the original Orels
     *  edgesWithViaOrelSource and edgesWithViaOrelTarget.
     * Removes all Edges leading to or coming from the current node to be
     *  contracted
     *  within the copies of edgesWithViaOrelSource and edgesWithViaOrelTarget.
     *
     * @param nodesWithRankOrelRank is an OrderedRelation containging all
     *  nodes of the underlying graph sorted by the field Rank
     * @param edgesWithViaOrelSource is an OrderedRelation containging all
     *  edges of the underlying graph sorted by the field Source
     * @param edgesWithViaOrelTarget is an OrderedRelation containging all
     *  edges of the underlying graph sorted by the field Target
     * @param hHop the h-hop-size to be used, will be overwritten
     *  by @see HL_DEFAULT_H_DEPTH if value < 1
     * @return true, since the result of this function is the modification
     *  of edgesWithViaOrelSource and edgesWithViaOrelTarget
    */
    bool hlIterateOverAllNodesByRankAscAndDoContraction(
        OrderedRelation* nodesWithRankOrelRank,
        OrderedRelation* edgesWithViaOrelSource,
        OrderedRelation* edgesWithViaOrelTarget,
        int hHop) const
    {
        #ifdef USEDEBUG
LogDebug("start hlIterateOverAllNodesByRankAscAndDoContraction"
              << endl);
#endif
        //Copy of edgesWithViaOrelSource
        ListExpr relTypeSource;
        nl->ReadFromString(hlGetEdgesOrelSourceTypeInfo(),
                           relTypeSource);
        ListExpr relNumTypeSource =
            SecondoSystem::GetCatalog()->NumericType(
                relTypeSource);
        Word wrelSource;
        wrelSource.setAddr(edgesWithViaOrelSource);
        Word wrelSource2 = edgesWithViaOrelSource->Clone(
                               relNumTypeSource, wrelSource);
        OrderedRelation* copyEdgesWithViaOrelSource =
            (OrderedRelation*) wrelSource2.addr;
        //TODO: die geklonte rel muss aber noch wie unten gelöscht werden oder?
        //TODO: der numtype, kann der auch aus der orel ausgelesen werden?
        //TODO: ggf. muss der Klon noch gelöscht werden per Delete?
        #ifdef USEDEBUG
LogDebug("edgesWithViaOrelSource cloned" << endl);
#endif

        //Copy of edgesWithViaOrelTarget
        ListExpr relTypeTarget;
        nl->ReadFromString(hlGetEdgesOrelTargetTypeInfo(),
                           relTypeTarget);
        ListExpr relNumTypeTarget =
            SecondoSystem::GetCatalog()->NumericType(
                relTypeTarget);
        Word wrelTarget;
        wrelTarget.setAddr(edgesWithViaOrelTarget);
        Word wrelTarget2 = edgesWithViaOrelTarget->Clone(
                               relNumTypeTarget, wrelTarget);
        OrderedRelation* copyEdgesWithViaOrelTarget =
            (OrderedRelation*) wrelTarget2.addr;
        //TODO: die geklonte rel muss aber noch wie unten gelöscht werden oder?
        //TODO: der numtype, kann der auch aus der orel ausgelesen werden?
        //TODO: ggf. muss der Klon noch gelöscht werden per Delete?
        #ifdef USEDEBUG
LogDebug("edgesWithViaOrelTarget cloned" << endl);
#endif

        std::multimap<int, std::tuple<int, double, int>>
                shortcutsToBeCreatedOrelSourceToBeDeleted;

        //Iterate Over all Ranked Nodes in ascending order of their rank
        GenericRelationIterator* nodesWithRankOrelRankIter
            = nodesWithRankOrelRank->MakeScan();
        Tuple* currentNodeRanked =
            nodesWithRankOrelRankIter->GetNextTuple();
        #ifdef USEDEBUG
        LogDebug("iterate over all nodes by rank" << endl);
        #endif

        #ifdef USEDEBUG
            int countProgress = 0;
        #endif
        while(currentNodeRanked)
        {
            CcInt* currentNodeRankedId = (CcInt*)
             currentNodeRanked->GetAttribute(HL_INDEX_OF_ID_IN_NODES_RANKED);

            #ifdef USEDEBUG
            LogDebug("do Contraction for: " <<
                  currentNodeRankedId->GetIntval() << " (" <<
                  ++countProgress << "/" <<
                  nodesWithRankOrelRank->GetNoTuples() << ")" <<
                  endl);
            #endif

            hlDoContraction(copyEdgesWithViaOrelSource,
                            copyEdgesWithViaOrelTarget,
                            shortcutsToBeCreatedOrelSourceToBeDeleted,
                            currentNodeRankedId, hHop);

            #ifdef USEDEBUG
LogDebug("remove edges to and from current v in copy edges relation"
                  << endl);
#endif
            hlRemoveContractedEdgesFromEdgesRelations(
                copyEdgesWithViaOrelSource,
                copyEdgesWithViaOrelTarget, currentNodeRankedId);

            #ifdef USEDEBUG
LogDebug("remove existing edges parallel to new"
             " shortcuts to be created"
                  << endl);
#endif
            hlRemoveParallelEdgesFromEdgesRelations(
                copyEdgesWithViaOrelSource,
                copyEdgesWithViaOrelTarget,
                shortcutsToBeCreatedOrelSourceToBeDeleted);

            #ifdef USEDEBUG
LogDebug("remove existing original edges parallel to"
             " new shortcuts to be created"
                  << endl);
#endif
            hlRemoveParallelEdgesFromEdgesRelations(
                edgesWithViaOrelSource, edgesWithViaOrelTarget,
                shortcutsToBeCreatedOrelSourceToBeDeleted);

            #ifdef USEDEBUG
LogDebug("add shortcuts to copy edges relations" <<
                  endl);
#endif
            //Add Shortcuts to Edges Relations
            hlAddShortcutsToEdgesRelations(
                copyEdgesWithViaOrelSource,
                copyEdgesWithViaOrelTarget,
                shortcutsToBeCreatedOrelSourceToBeDeleted);

            #ifdef USEDEBUG
LogDebug("add shortcuts to original edges relations"
                  << endl);
#endif
            //Add Shortcuts to Edges Relations
            hlAddShortcutsToEdgesRelations(
                edgesWithViaOrelSource, edgesWithViaOrelTarget,
                shortcutsToBeCreatedOrelSourceToBeDeleted);

            //Free Outgoing-Iteration
            currentNodeRanked->DeleteIfAllowed();
            currentNodeRanked = 0;
            currentNodeRanked =
                nodesWithRankOrelRankIter->GetNextTuple();

            //clear shortcuts multimap for next run
            shortcutsToBeCreatedOrelSourceToBeDeleted.clear();

        }
        #ifdef USEDEBUG
LogDebug("finished iteration" << endl);
#endif

        //Free Resources
        delete nodesWithRankOrelRankIter; //TODO: richtig, dass der
        // iter nur deleted wird ohne besondere funktion?
        OrderedRelation::Delete(relNumTypeSource,
                                wrelSource2);
        OrderedRelation::Delete(relNumTypeTarget,
                                wrelTarget2);

        #ifdef USEDEBUG
LogDebug("finish hlIterateOverAllNodesByRankAscAndDoContraction"
              << endl);
#endif
        return true;
    }

    /*
     * Contract the given node by running a oneHopReverse-Search
     *  and a hHopForwardSearch.
     *
     * @param edgesWithViaOrelSource is an OrderedRelation containging all
     *  edges of the underlying graph sorted by the field Source
     * @param edgesWithViaOrelTarget is an OrderedRelation containging all
     *  edges of the underlying graph sorted by the field Target
     * @param shortcutsToBeCreatedOrelSourceToBeDeleted is a Multimap by
     *  reference containging all shortcuts to be added to the original edges
     *  OrderedRelations
     * @param currentVToBeContracted the Node ID of the vertex t to contracted
     * @param hHop the h-hop-size to be used, will be overwritten
     *  by @see HL_DEFAULT_H_DEPTH if value < 1
     * @return true, since the result of this function is the modification
     *  of shortcutsToBeCreatedOrelSourceToBeDeleted
    */
    bool hlDoContraction(OrderedRelation*
                         edgesWithViaOrelSource,
                         OrderedRelation* edgesWithViaOrelTarget,
                         std::multimap<int, std::tuple<int, double, int>>
                         &shortcutsToBeCreatedOrelSourceToBeDeleted,
                         CcInt* currentVToBeContracted, int hHop) const
    {
        #ifdef USEDEBUG
LogDebug("start hlDoContraction" << endl);
#endif

        //Reverse search
        std::multimap<int, std::tuple<int, double>>
                oneHopReverseMultiMap; //XXTODO: ist diese zuweisung ein
                // Problem in Bezug auf Daten aus vorherigen Iterationen?

        #ifdef USEDEBUG
LogDebug("do hlOneHopReverseSearch" << endl);
#endif
        hlOneHopReverseSearch(edgesWithViaOrelSource,
                              edgesWithViaOrelTarget, currentVToBeContracted,
                              oneHopReverseMultiMap);

        #ifdef USEDEBUG
LogDebug("do hlHHopForwardSearch" << endl);
#endif
        hlHHopForwardSearch(edgesWithViaOrelSource,
                            edgesWithViaOrelTarget, oneHopReverseMultiMap,
                            currentVToBeContracted,
                            shortcutsToBeCreatedOrelSourceToBeDeleted, hHop);

        oneHopReverseMultiMap.clear();

        #ifdef USEDEBUG
LogDebug("finish hlDoContraction" << endl);
#endif
        return true;
    }



    /*
     * Adds all shortcuts from shortcutsToBeCreatedOrelSourceToBeDeleted to
     *  the original edges OrderedRelations
     *  edgesWithViaOrelSource and edgesWithViaOrelTarget
     *
     * @param edgesWithViaOrelSource is an OrderedRelation containging all
     *  edges of the underlying graph sorted by the field Source
     * @param edgesWithViaOrelTarget is an OrderedRelation containging all
     *  edges of the underlying graph sorted by the field Target
     * @param shortcutsToBeCreatedOrelSourceToBeDeleted is a Multimap by
     *  reference containging all shortcuts to be added to the original edges
     *  OrderedRelations
     * @return true, since the result of this function is the modification of
     *  edgesWithViaOrelSource and edgesWithViaOrelTarget
    */
    bool hlAddShortcutsToEdgesRelations(
        OrderedRelation* edgesWithViaOrelSource,
        OrderedRelation* edgesWithViaOrelTarget,
        std::multimap<int, std::tuple<int, double, int>>
        &shortcutsToBeCreatedOrelSourceToBeDeleted) const
    {
        #ifdef USEDEBUG
LogDebug("start hlAddShortcutsToEdgesRelations" <<
              endl);
#endif

        for (std::multimap<int, std::tuple<int, double, int>>::iterator
                shortcutsIter =
                    shortcutsToBeCreatedOrelSourceToBeDeleted.begin();
                shortcutsIter !=
                shortcutsToBeCreatedOrelSourceToBeDeleted.end();
                ++shortcutsIter)
        {
            std::tuple<int, double, int> currTuple =
                (*shortcutsIter).second;

            int currentSource = (*shortcutsIter).first;
            int currentTarget =
                std::get<HL_INDEX_OF_TARGET_IN_SHORTCUT_TUPLE -
                1>(currTuple);
            int currentParent =
                std::get<HL_INDEX_OF_PARENT_ID_IN_SHORTCUT_TUPLE -
                1>(currTuple);
            double currentCosts =
                std::get<HL_INDEX_OF_DIST_IN_SHORTCUT_TUPLE - 1>
                (currTuple);

            #ifdef USEDEBUG
LogDebug("build next inputtuple (" << currentSource
                  << ", " << currentTarget << ")" << endl);
#endif

            ListExpr relTypeSource;
            nl->ReadFromString(hlGetEdgesOrelSourceTypeInfo(), relTypeSource);
            ListExpr tupleNumTypeSource =
                SecondoSystem::GetCatalog()->NumericType(
                    nl->Second(relTypeSource));
            Tuple* insertTuple = new Tuple(
                tupleNumTypeSource);

            insertTuple->PutAttribute(
                HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE, new CcInt(true,
                        currentSource));
            insertTuple->PutAttribute(
                HL_INDEX_OF_TARGET_IN_EDGE_TUPLE, new CcInt(true,
                        currentTarget));
            insertTuple->PutAttribute(
                HL_INDEX_OF_SOURCE_POS_IN_EDGE_TUPLE,
                new Point(false, 0.0, 0.0));
            insertTuple->PutAttribute(
                HL_INDEX_OF_TARGET_POS_IN_EDGE_TUPLE,
                new Point(false, 0.0, 0.0));
            insertTuple->PutAttribute(
                HL_INDEX_OF_SOURCE_NODE_COUNTER_IN_EDGE_TUPLE,
                new CcInt(false));
            insertTuple->PutAttribute(
                HL_INDEX_OF_TARGET_NODE_COUNTER_IN_EDGE_TUPLE,
                new CcInt(false));
            insertTuple->PutAttribute(
                HL_INDEX_OF_CURVE_IN_EDGE_TUPLE,
                new SimpleLine(0));
            insertTuple->PutAttribute(
                HL_INDEX_OF_ROAD_NAME_IN_EDGE_TUPLE,
                new FText(false));
            insertTuple->PutAttribute(
                HL_INDEX_OF_ROAD_TYPE_IN_EDGE_TUPLE,
                new FText(false));
            insertTuple->PutAttribute(
                HL_INDEX_OF_WAY_ID_IN_EDGE_TUPLE,
                new LongInt(false));
            insertTuple->PutAttribute(
                HL_INDEX_OF_COSTS_IN_EDGE_TUPLE, new CcReal(true,
                        currentCosts));
            insertTuple->PutAttribute(
                HL_INDEX_OF_PARENT_ID_IN_EDGE_TUPLE,
                new CcInt(true, currentParent));

            #ifdef USEDEBUG
LogDebug("append to edgesWithViaOrelSource" << endl);
#endif
            edgesWithViaOrelSource->AppendTuple(insertTuple);

            #ifdef USEDEBUG
LogDebug("append to edgesWithViaOrelTarget" << endl);
#endif
            edgesWithViaOrelTarget->AppendTuple(insertTuple);

            insertTuple->DeleteIfAllowed(); //TODO zu früh gelöscht weil orel
            // wo es appended wurde noch verwendet wird?
            insertTuple = 0;

            #ifdef USEDEBUG
LogDebug("get next tuple" << endl);
#endif
            //Free Outgoing-Iteration
        }
        #ifdef USEDEBUG
LogDebug("finish iteration" << endl);
#endif

        #ifdef USEDEBUG
LogDebug("end hlAddShortcutsToEdgesRelations" <<
              endl);
#endif
        return true;
    }


    /*
     * Removes all incoming and outgoing edges to and from the given node
     *  v within the given edges orels which are the current temporal
     *  copies (currentEdgesOrelSource and currentEdgesOrelTarget)
     *
     * @param currentEdgesOrelSource is an OrderedRelation containging all
     *  remaining edges of the underlying graph sorted by the field Source
     * @param currentEdgesOrelTarget is an OrderedRelation containging all
     *  remaining edges of the underlying graph sorted by the field Target
     * @param currentContractedV the current node v which has been contracted
     *  and for which its incoming and outgoing edges
     *         shall be deleted within the given edgesOrels Source and Target
     * @return true, since the result of this function is the modification of
     *  currentEdgesOrelSource and currentEdgesOrelTarget
    */
    bool hlRemoveContractedEdgesFromEdgesRelations(
        OrderedRelation* currentEdgesOrelSource,
        OrderedRelation* currentEdgesOrelTarget,
        CcInt* currentContractedV) const
    {
        #ifdef USEDEBUG
            LogDebug("start hlRemoveContractedEdgesFromEdgesRelations"
              << endl);
        #endif

        //Prepare Range scan
        //Create From and to by currentVertexID (=v)
        std::vector<void*> vecAttributesFromFwdV(1);
        vecAttributesFromFwdV[0] = currentContractedV;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesFromFwdV(1);
        vecAttrTypesFromFwdV[0] =
            currentContractedV->getSMIKeyType();
        CompositeKey fromFwdV(vecAttributesFromFwdV,
         vecAttrTypesFromFwdV, false);

        CcInt currentContractedVPlusOneFwdV(
         true, currentContractedV->GetIntval() + 1);
        CcInt* currentVertexIdPlusOnePtrFwdV =
            &currentContractedVPlusOneFwdV;
        std::vector<void*> vecAttributesToFwdV(1);
        vecAttributesToFwdV[0] =
            currentVertexIdPlusOnePtrFwdV;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesToFwdV(1);
        vecAttrTypesToFwdV[0] =
            currentVertexIdPlusOnePtrFwdV->getSMIKeyType();
        CompositeKey toFwdV(vecAttributesToFwdV, vecAttrTypesToFwdV, false);


        vector<Tuple*> tupleVectorToDeleteSource;
        vector<Tuple*> tupleVectorToDeleteTarget;


        #ifdef USEDEBUG
            LogDebug("remove incoming edges SV" <<
              endl);
        #endif //XXTODO: in orelTarget müssten aber auch die
              // ausgehenden Kanten gelöscht werden, dafür wäre aber
              // ein full scan notwendig?!
        GenericRelationIterator* relEdgesTargetIterSV =
            currentEdgesOrelTarget->MakeRangeScan(fromFwdV,
                    toFwdV);
        Tuple* currentEdgeTargetTupleSV =
            relEdgesTargetIterSV->GetNextTuple();
        while(currentEdgeTargetTupleSV)
        {
            //Get SourceNode of current edge
            CcInt* currentEdgeTargetTupleSVSourceId =
                (CcInt*) currentEdgeTargetTupleSV->GetAttribute(
                    HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);

            #ifdef USEDEBUG
            CcInt* currentEdgeTargetTupleSVTargetId =
                (CcInt*) currentEdgeTargetTupleSV->GetAttribute(
                    HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);

                LogDebug("going to delete: (" <<
                  currentEdgeTargetTupleSVSourceId->GetIntval() <<
                  "," << currentEdgeTargetTupleSVTargetId->GetIntval()
                  << ")" << endl);
            #endif
            //Add tuple to vector
            tupleVectorToDeleteTarget.push_back(
                currentEdgeTargetTupleSV);

            //Add tuple to opposite vector
            hlRemoveContractedEdgesFromEdgesRelationsScanOppositeOrel(
                currentEdgesOrelSource,
                currentContractedV->GetIntval(),
                currentEdgeTargetTupleSVSourceId,
                tupleVectorToDeleteSource, true);

            //dont delete Pointers, we need them later
            currentEdgeTargetTupleSV = 0;
            currentEdgeTargetTupleSV =
                relEdgesTargetIterSV->GetNextTuple();
        }
        delete relEdgesTargetIterSV;


        #ifdef USEDEBUG
            LogDebug("remove outgoing edges VT" <<
              endl);
        #endif //XXTOTO: auch eingehende Kanten sollten gelöscht werden?!
        GenericRelationIterator* relEdgesSourceIterVT =
            currentEdgesOrelSource->MakeRangeScan(fromFwdV,
                    toFwdV);
        Tuple* currentEdgeSourceTupleVT =
            relEdgesSourceIterVT->GetNextTuple();
        while(currentEdgeSourceTupleVT)
        {
            //Get SourceNode of current edge
            CcInt* currentEdgeSourceTupleVTTargetId =
                (CcInt*) currentEdgeSourceTupleVT->GetAttribute(
                    HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);

            #ifdef USEDEBUG
                CcInt* currentEdgeSourceTupleVTSourceId =
                    (CcInt*) currentEdgeSourceTupleVT->GetAttribute(
                    HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);

                LogDebug("going to delete: (" <<
                  currentEdgeSourceTupleVTSourceId->GetIntval() <<
                  "," << currentEdgeSourceTupleVTTargetId->GetIntval()
                  << ")" << endl);
            #endif
            //Add tuple to vector
            tupleVectorToDeleteSource.push_back(
                currentEdgeSourceTupleVT);

            //Add tuple to opposite vector
            hlRemoveContractedEdgesFromEdgesRelationsScanOppositeOrel(
                currentEdgesOrelTarget,
                currentContractedV->GetIntval(),
                currentEdgeSourceTupleVTTargetId,
                tupleVectorToDeleteTarget, false);

            //dont delete Pointers, we need them later
            currentEdgeSourceTupleVT = 0;
            currentEdgeSourceTupleVT =
                relEdgesSourceIterVT->GetNextTuple();
        }
        delete relEdgesSourceIterVT;


        #ifdef USEDEBUG
            LogDebug("iterate tupleVectorToDeleteTarget" <<
              endl);
        #endif
        for (std::vector<Tuple*>::iterator
                currVectorTargetIter =
                    tupleVectorToDeleteTarget.begin();
                currVectorTargetIter !=
                tupleVectorToDeleteTarget.end();
                ++currVectorTargetIter)
        {
            Tuple* tmpTuple = *(currVectorTargetIter);

            if(tmpTuple)
            {
                currentEdgesOrelTarget->DeleteTuple(tmpTuple,
                                                    true); //XXTODO: geht das?
                tmpTuple->DeleteIfAllowed();
                tmpTuple = 0;
            }
        }


        #ifdef USEDEBUG
            LogDebug("iterate tupleVectorToDeleteSource" <<
              endl);
        #endif
        for (std::vector<Tuple*>::iterator
                currVectorSourceIter =
                    tupleVectorToDeleteSource.begin();
                currVectorSourceIter !=
                tupleVectorToDeleteSource.end();
                ++currVectorSourceIter)
        {
            Tuple* tmpTuple2 = *(currVectorSourceIter);

            currentEdgesOrelSource->DeleteTuple(tmpTuple2,
                                                true); //XXTODO: geht das?
            tmpTuple2->DeleteIfAllowed();
            tmpTuple2 = 0;
        }

        #ifdef USEDEBUG
            LogDebug("end hlRemoveContractedEdgesFromEdgesRelations"
              << endl);
        #endif
        return true;
    }


    /*
     * Searches for still existing edges (s, t) within the given
     *  OrderedReation currentEdgesOrelSource and currentEdgesOrelTarget
     *  which run parallel to a shortcut to be added to the same
     *  OrderedRelations.
     *
     * @param currentEdgesOrelSource is an OrderedRelation containging all
     *  remaining edges of the underlying graph sorted by the field Source
     * @param currentEdgesOrelTarget is an OrderedRelation containging all
     *  remaining edges of the underlying graph sorted by the field Target
     * @param shortcutsToBeCreatedOrelSourceToBeDeleted the shortcuts to be
     *  added to the orderedRelations
     * @return true, since the result of this function is the modification
     *  of currentEdgesOrelSource and currentEdgesOrelTarget
    */
    bool hlRemoveParallelEdgesFromEdgesRelations(
        OrderedRelation* currentEdgesOrelSource,
        OrderedRelation* currentEdgesOrelTarget,
        std::multimap<int, std::tuple<int, double, int>>
        shortcutsToBeCreatedOrelSourceToBeDeleted) const
    {
        #ifdef USEDEBUG
            LogDebug("start hlRemoveParallelEdgesFromEdgesRelations"
              << endl);
        #endif

        vector<Tuple*> tupleVectorToDeleteSource;
        vector<Tuple*> tupleVectorToDeleteTarget;

        for (std::multimap<int, std::tuple<int, double, int>>::iterator
                shortcutsIter =
                    shortcutsToBeCreatedOrelSourceToBeDeleted.begin();
                shortcutsIter !=
                shortcutsToBeCreatedOrelSourceToBeDeleted.end();
                ++shortcutsIter)
        {
            std::tuple<int, double, int> currTuple =
                (*shortcutsIter).second;

            int currentSource = (*shortcutsIter).first;
            int currentTarget =
                std::get<HL_INDEX_OF_TARGET_IN_SHORTCUT_TUPLE -
                1>(currTuple);

            //Prepare Range scan
            //Create From and to by currentSource (=s)
            CcInt currentSourceCcInt(true, currentSource);
            CcInt* currentSourceCcIntPtr =
                &currentSourceCcInt;
            std::vector<void*> vecAttributesFromSource(1);
            vecAttributesFromSource[0] =
                currentSourceCcIntPtr;
            std::vector<SmiKey::KeyDataType>
            vecAttrTypesFromSource(1);
            vecAttrTypesFromSource[0] =
                currentSourceCcIntPtr->getSMIKeyType();
            CompositeKey fromSource(vecAttributesFromSource,
                                    vecAttrTypesFromSource, false);

            CcInt currentSourceCcIntPlusOne(true,
                                currentSourceCcIntPtr->GetIntval() + 1);
            CcInt* currentSourceCcIntPlusOnePtr =
                &currentSourceCcIntPlusOne;
            std::vector<void*> vecAttributesToSource(1);
            vecAttributesToSource[0] =
                currentSourceCcIntPlusOnePtr;
            std::vector<SmiKey::KeyDataType>
            vecAttrTypesToSource(1);
            vecAttrTypesToSource[0] =
                currentSourceCcIntPlusOnePtr->getSMIKeyType();
            CompositeKey toSource(vecAttributesToSource,
                                  vecAttrTypesToSource, false);

            GenericRelationIterator* currentEdgesOrelSourceIter =
                currentEdgesOrelSource->MakeRangeScan(fromSource, toSource);
            Tuple* currentEdgesOrelSourceTuple =
                currentEdgesOrelSourceIter->GetNextTuple();
            while(currentEdgesOrelSourceTuple)
            {
                CcInt* currentEdgesOrelSourceTupleTargetId =
                    (CcInt*) currentEdgesOrelSourceTuple->GetAttribute(
                        HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);


                //If there is a parallel edge it should be
                // removed from the working-graph
                if(currentEdgesOrelSourceTupleTargetId->GetIntval()
                        == currentTarget)
                {
                    #ifdef USEDEBUG
                        CcInt* currentEdgesOrelSourceTupleSourceId =
                            (CcInt*) currentEdgesOrelSourceTuple->GetAttribute(
                            HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
                        CcReal* currentEdgesOrelSourceTupleDist =
                            (CcReal*) currentEdgesOrelSourceTuple->GetAttribute(
                            HL_INDEX_OF_COSTS_IN_EDGE_TUPLE);
                        LogDebug("remove incoming edges ST: (" <<
                          currentEdgesOrelSourceTupleSourceId->GetIntval()
                          << ", " <<
                          currentEdgesOrelSourceTupleTargetId->GetIntval()
                          << ", " <<
                          currentEdgesOrelSourceTupleDist->GetRealval() <<
                          ")"<< endl);
                    #endif
                    tupleVectorToDeleteSource.push_back(
                        currentEdgesOrelSourceTuple);

                }

                //dont delete Pointers, we need them later
                currentEdgesOrelSourceTuple = 0;
                currentEdgesOrelSourceTuple =
                    currentEdgesOrelSourceIter->GetNextTuple();
            }
            delete currentEdgesOrelSourceIter;

            //Prepare Range scan
            //Create From and to by currentTarget (=s)
            CcInt currentTargetCcInt(true, currentTarget);
            CcInt* currentTargetCcIntPtr =
                &currentTargetCcInt;
            std::vector<void*> vecAttributesFromTarget(1);
            vecAttributesFromTarget[0] =
                currentTargetCcIntPtr;
            std::vector<SmiKey::KeyDataType>
            vecAttrTypesFromTarget(1);
            vecAttrTypesFromTarget[0] =
                currentTargetCcIntPtr->getSMIKeyType();
            CompositeKey fromTarget(vecAttributesFromTarget,
                                    vecAttrTypesFromTarget, false);

            CcInt currentTargetCcIntPlusOne(true,
                                currentTargetCcIntPtr->GetIntval() + 1);
            CcInt* currentTargetCcIntPlusOnePtr =
                &currentTargetCcIntPlusOne;
            std::vector<void*> vecAttributesToTarget(1);
            vecAttributesToTarget[0] =
                currentTargetCcIntPlusOnePtr;
            std::vector<SmiKey::KeyDataType>
            vecAttrTypesToTarget(1);
            vecAttrTypesToTarget[0] =
                currentTargetCcIntPlusOnePtr->getSMIKeyType();
            CompositeKey toTarget(vecAttributesToTarget,
                                  vecAttrTypesToTarget, false);

            GenericRelationIterator*
            currentEdgesOrelTargetIter =
                currentEdgesOrelTarget->MakeRangeScan(fromTarget,
                        toTarget);
            Tuple* currentEdgesOrelTargetTuple =
                currentEdgesOrelTargetIter->GetNextTuple();
            while(currentEdgesOrelTargetTuple)
            {
                CcInt* currentEdgesOrelTargetTupleSourceId =
                    (CcInt*) currentEdgesOrelTargetTuple->GetAttribute(
                        HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);

                //If there is a parallel edge it should be
                // removed from the working-graph
                if(currentEdgesOrelTargetTupleSourceId->GetIntval()
                        == currentSource)
                {
                    #ifdef USEDEBUG
                        CcInt* currentEdgesOrelTargetTupleTargetId =
                            (CcInt*) currentEdgesOrelTargetTuple->GetAttribute(
                            HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
                        CcReal* currentEdgesOrelTargetTupleDist =
                            (CcReal*) currentEdgesOrelTargetTuple->GetAttribute(
                            HL_INDEX_OF_COSTS_IN_EDGE_TUPLE);
                        LogDebug("remove incoming edges ST: (" <<
                          currentEdgesOrelTargetTupleSourceId->GetIntval()
                          << ", " <<
                          currentEdgesOrelTargetTupleTargetId->GetIntval()
                          << ", " <<
                          currentEdgesOrelTargetTupleDist->GetRealval() <<
                          ")"<< endl);
                    #endif
                    tupleVectorToDeleteTarget.push_back(
                        currentEdgesOrelTargetTuple);

                }

                //dont delete Pointers, we need them later
                currentEdgesOrelTargetTuple = 0;
                currentEdgesOrelTargetTuple =
                    currentEdgesOrelTargetIter->GetNextTuple();
            }
            delete currentEdgesOrelTargetIter;
        }


        #ifdef USEDEBUG
            LogDebug("iterate tupleVectorToDeleteSource" <<
              endl);
        #endif
        for (vector<Tuple*>::iterator currVectorSourceIter
                = tupleVectorToDeleteSource.begin();
                currVectorSourceIter !=
                tupleVectorToDeleteSource.end();
                ++currVectorSourceIter)
        {
            Tuple* tmpTuple2 = *(currVectorSourceIter);
            currentEdgesOrelSource->DeleteTuple(tmpTuple2,
                                                true); //XXTODO: geht das?
            tmpTuple2->DeleteIfAllowed();
            tmpTuple2 = 0;
        }

        #ifdef USEDEBUG
            LogDebug("iterate tupleVectorToDeleteTarget" <<
              endl);
        #endif
        for (vector<Tuple*>::iterator currVectorTargetIter
                = tupleVectorToDeleteTarget.begin();
                currVectorTargetIter !=
                tupleVectorToDeleteTarget.end();
                ++currVectorTargetIter)
        {
            Tuple* tmpTuple = *(currVectorTargetIter);
            currentEdgesOrelTarget->DeleteTuple(tmpTuple,
                                                true); //XXTODO: geht das?
            tmpTuple->DeleteIfAllowed();
            tmpTuple = 0;
        }

        #ifdef USEDEBUG
            LogDebug("end hlRemoveParallelEdgesFromEdgesRelations"
              << endl);
        #endif
        return true;
    }



    /*
     * Scans one of the given Orels (depending on scanBySourceOrTarget) for
     *  the given currentNodeSourceOrTarget and only processes if the opposite
     *  endpoint is v.
     * If so stores the edge-tuple into the given vector.
     * Scenario:
     *  If the outer iteration scanned hlEdgesOrelSource (which gets outgoing
     *  edges from v; outer iteration) then scanBySourceOrTarget shall be set
     *  to false.
     *  In this case currentEdgesOrelSourceOrTarget is equal to
     *  hlEdgesOrelTarget.
     *  Furthermore this function scans currentEdgesOrelSourceOrTarget by
     *  the given targetNode t (inner iteration).
     *  For each edge within the inner iteration the function checks whether
     *  the sourceNodeId is equal to the given nodeIdV.
     *  If so the current tuple is stored into the given vector and the inner
     *  iteration will be aborted.
     *
     * @param currentEdgesOrelSourceOrTarget is an OrderedRelation containging
     *  all remaining edges of the underlying graph sorted by the field Source
     * @param currentNodeIdV the current contracted node which is used to find
     *  only edges incoming to or outgoinf from v
     * @param currentNodeSourceOrTarget the current source or target node which
     *  is used to find edges within the opposite orderedRelation
     * @param currentVector the current vector by reference where to store the
     *  tuples into found by this function
     * @param scanBySourceOrTarget a boolean to differ whether to scan by
     *  source (=true) oder by target (=false)
     * @return true, since the result of this function is the modification
     *  of currentEdgesOrelSource and currentEdgesOrelTarget
    */
    bool hlRemoveContractedEdgesFromEdgesRelationsScanOppositeOrel(
        OrderedRelation* currentEdgesOrelSourceOrTarget,
        int currentNodeIdV,
        CcInt* currentNodeSourceOrTarget,
        vector<Tuple*> &currentVector,
        bool scanBySourceOrTarget) const
    {
        #ifdef USEDEBUG
LogDebug("start hlRemoveContractedEdgesFromEdgesRelationsScanOppositeOrel"
              << endl);
#endif

        //Prepare Range scan
        //Create From and to by currentNodeSourceOrTarget
        std::vector<void*> vecAttributesFrom(1);
        vecAttributesFrom[0] = currentNodeSourceOrTarget;
        std::vector<SmiKey::KeyDataType> vecAttrTypesFrom(
            1);
        vecAttrTypesFrom[0] =
            currentNodeSourceOrTarget->getSMIKeyType();
        CompositeKey from(vecAttributesFrom,
                          vecAttrTypesFrom, false);

        CcInt currentNodeSourceOrTargetPlusOne(true,
                                currentNodeSourceOrTarget->GetIntval() + 1);
        CcInt* currentNodeSourceOrTargetPlusOnePtr =
            &currentNodeSourceOrTargetPlusOne;
        std::vector<void*> vecAttributesTo(1);
        vecAttributesTo[0] =
            currentNodeSourceOrTargetPlusOnePtr;
        std::vector<SmiKey::KeyDataType> vecAttrTypesTo(
            1);
        vecAttrTypesTo[0] =
            currentNodeSourceOrTargetPlusOnePtr->getSMIKeyType();
        CompositeKey to(vecAttributesTo, vecAttrTypesTo,
                        false);

        GenericRelationIterator*
        currentEdgesOrelSourceOrTargetIter =
            currentEdgesOrelSourceOrTarget->MakeRangeScan(
                from, to);
        Tuple* currentEdgeSourceOrTargetTuple =
            currentEdgesOrelSourceOrTargetIter->GetNextTuple();
        CcInt* currentSourceOrTargetNodeId;
        while(currentEdgeSourceOrTargetTuple)
        {
            //If is scanned by Source, then we need the
            // targetNodeId to compare it with nodeIdV
            if(scanBySourceOrTarget)
            {
                currentSourceOrTargetNodeId = (CcInt*)
                                currentEdgeSourceOrTargetTuple->GetAttribute(
                                HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
            }
            else
            {
                currentSourceOrTargetNodeId = (CcInt*)
                                currentEdgeSourceOrTargetTuple->GetAttribute(
                                HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
            }

            if(currentSourceOrTargetNodeId->GetIntval() ==
                    currentNodeIdV)
            {
                //only add tuple if edge really is connected to v
                #ifdef USEDEBUG
LogDebug("pushback: " <<
                      currentSourceOrTargetNodeId->GetIntval() << endl);
#endif
                currentVector.push_back(
                    currentEdgeSourceOrTargetTuple);
            }
            else
            {
                currentEdgeSourceOrTargetTuple->DeleteIfAllowed();
            }
            //free resource
            currentEdgeSourceOrTargetTuple = 0;
            currentEdgeSourceOrTargetTuple =
                currentEdgesOrelSourceOrTargetIter->GetNextTuple();
        }
        delete currentEdgesOrelSourceOrTargetIter;

        #ifdef USEDEBUG
LogDebug("end hlRemoveContractedEdgesFromEdgesRelationsScanOppositeOrel"
              << endl);
#endif
        return true;
    }

    static const string hlForwardLabelColumnName()
    {
        return "ForwardLabel";
    }
    static const string hlReverseLabelColumnName()
    {
        return "ReverseLabel";
    }

    /*
     * NestedRelation type for the set containing all labels.
     * Used for creation of new NestedRelations,
     *  new Tuples of this NestedRelations
     *  or cloning or deleting of an NestedRelations of this type
    */
    static const string hlGetAllLabelsTypeInfo()
    {
        return "(" + NestedRelation::BasicType() +
               "(" + Tuple::BasicType() +
               "(" +
               "(SourceNodeId " + CcInt::BasicType() + ")" +
               "(" + hlForwardLabelColumnName() + " " +
               hlGetForwardOrReverseLabelArelTypeInfo() + ")" +
               "(" + hlReverseLabelColumnName() + " " +
               hlGetForwardOrReverseLabelArelTypeInfo() + ")" +
               ")" +
               ")" +
               ")";
    }

    /*
     * Relation type for forward and reverse labels.
     * Used for creation of new Relation Objects representing
     *  a forward or reverse label.
     * Used for Testing-Operator
    */
    static const string
    hlGetForwardOrReverseLabelTypeInfo()
    {
        return "(" + Relation::BasicType() +
               hlGetForwardOrReverseLabelTupleTypeInfo() +
               ")";
    }

    /*
     * AttributeRelation type for forward and reverse labels.
     * Used for creation of new AttributeRelation Objects, new
     *  Tuples of this AttributeRelation
     *  or cloning or deleting of an AttributeRelation of this type
    */
    static const string
    hlGetForwardOrReverseLabelArelTypeInfo() //XXTODO: geht das?
    {
        return "(" + AttributeRelation::BasicType() +
               hlGetForwardOrReverseLabelTupleTypeInfo() +
               ")";
    }

    /*
     * Tuple type for forward and reverse labels.
     * Used for creation of new Relation Objects, new Tuples of this Relation
     *  or cloning or deleting of an Relation of this type
    */
    static const string
    hlGetForwardOrReverseLabelTupleTypeInfo()
    {
        return "(" + Tuple::BasicType() +
               "(" +
               "(HubNodeIdNew " + CcInt::BasicType() + ")" +
               "(HubNodeId " + CcInt::BasicType() + ")" +
               "(HubParentNodeId " + CcInt::BasicType() + ")" +
               "(HubParentNodeTupleId " + CcInt::BasicType() +
               ")" +
               "(HubDistanceToSource " + CcReal::BasicType() +
               ")" +
               ")" +
               ")";
    }

    /*
     * Definition of Field-Indexes for NestedRelation allLabels
    */
    static const int
    HL_INDEX_OF_NODE_ID_IN_ALL_LABELS_TUPLE = 0;
    static const int
    HL_INDEX_OF_FORWARD_LABEL_IN_ALL_LABELS_TUPLE = 1;
    static const int
    HL_INDEX_OF_REVERSE_LABEL_IN_ALL_LABELS_TUPLE = 2;

    /*
     * Definition of Field-Indexes for AttributeRelation forwardOrReverseLabel
    */
    static const int
    HL_INDEX_OF_HUB_NODE_ID_NEW_IN_LABEL_TUPLE = 0;
    static const int
    HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE = 1;
    static const int
    HL_INDEX_OF_HUB_PARENT_NODE_ID_IN_LABEL_TUPLE = 2;
    static const int
    HL_INDEX_OF_HUB_PARENT_TUPLE_ID_IN_LABEL_TUPLE =
        3;
    static const int
    HL_INDEX_OF_HUB_DISTANCE_TO_SOURCE_IN_LABEL_TUPLE
        = 4;




    /*
     * Creates a Lable (either forward or reverse) for every node
     *  in nodesRankedDescOrelRank.
     *
     * @param allLabelsNrel is an NestedRelation representing the result
     *  of this operator
     * @param nodesRankedDescOrelRank is an OrderedRelation containging all
     *  nodes of the underlying graph sorted by the field NodeIdNew
     * @param edgesContractedUpwardsOrelSource is an OrderedRelation
     *  containging all edges of the underlying graph sorted by the field Source
     * @param edgesContractedDownwardsOrelTarget is an OrderedRelation
     *  containging all edges of the underlying graph sorted by the field Target
     * @param hHop is a an integer which defines the h-Hop-Size used for
     *  local look-up queries which is used for stalling on demand
     * @return true since the result is stored in the gviven allLabelsNrel
    */
    bool hlCreateLabels(NestedRelation* allLabelsNrel,
                        BTree* bTreeNodesRankedDescOrelRank,
                        OrderedRelation* nodesRankedDescOrelRank,
                        OrderedRelation* edgesContractedUpwardsOrelSource,
                        OrderedRelation*
                        edgesContractedDownwardsOrelTarget,
                        int hHop) const
    {
        #ifdef USEDEBUG
LogDebug("start hlCreateLabels" << endl);
#endif

        #ifdef USEDEBUG
LogDebug("create forward and reverse relation containing label-data"
         " used by all single forward and reverse labels"
              << endl);
#endif
        SubRelation* forwardDataSubRel =
            allLabelsNrel->getSubRel(
                hlForwardLabelColumnName());
        SubRelation* reverseDataSubRel =
            allLabelsNrel->getSubRel(
                hlReverseLabelColumnName());

        SmiFileId forwardDataSubRelFileId =
            forwardDataSubRel->fileId;
        SmiFileId reverseDataSubRelFileId =
            reverseDataSubRel->fileId;

        ListExpr forwardDataSubRelTypeInfo =
            forwardDataSubRel->typeInfo;
        ListExpr reverseDataSubRelTypeInfo =
            reverseDataSubRel->typeInfo;

        Relation* forwardDataRel = forwardDataSubRel->rel;
        Relation* reverseDataRel = reverseDataSubRel->rel;

        #ifdef USEDEBUG
LogDebug("do while" << endl);
#endif
        GenericRelationIterator* nodesRankIter =
            nodesRankedDescOrelRank->MakeScan();
            //XXTODO: iteration in der iteration???
        Tuple* currentNodeV =
            nodesRankIter->GetNextTuple();

        while(currentNodeV)
        {
            #ifdef USEDEBUG
LogDebug("next iteration" << endl);
#endif

            CcInt* nodeIdV = (CcInt*)
                             currentNodeV->GetAttribute(
                                 HL_INDEX_OF_ID_IN_NODES_RANKED);

            #ifdef USEDEBUG
LogDebug("prepare NestedRelationTuple" << endl);
#endif

            TupleType* tupleTypeAllLabels =
                allLabelsNrel->getPrimary()->GetTupleType();
            Tuple* allLabelsTuple = new Tuple(
                tupleTypeAllLabels);
            allLabelsTuple->PutAttribute(
                HL_INDEX_OF_NODE_ID_IN_ALL_LABELS_TUPLE,
                new CcInt(true, nodeIdV->GetIntval()));

            #ifdef USEDEBUG
LogDebug("forward  and reverse label multimaps" <<
                  endl);
#endif
            std::multimap<int, std::tuple<int, int, double>>
                    labelForwardMultimap;
            std::multimap<int, std::tuple<int, int, double>>
                    labelReverseMultimap;

            bool isForward = true;
            //Create forward label
            #ifdef USEDEBUG
LogDebug("create forward label" << endl);
#endif
            hlCreateLabelByDijkstraWithStalling(
                bTreeNodesRankedDescOrelRank,
                nodesRankedDescOrelRank,
                edgesContractedUpwardsOrelSource,
                edgesContractedDownwardsOrelTarget,
                labelForwardMultimap, nodeIdV, hHop, isForward);
            #ifdef USEDEBUG
LogDebug("fully prune forward label" << endl);
#endif
            //hlPruneLabelByBootstrapping(allLabelsNrel,
            // labelForwardMultimap, nodeIdV, isForward);
            #ifdef USEDEBUG
LogDebug("create and fill forwardLabel" << endl);
#endif
            AttributeRelation* labelForwardArel = new
            AttributeRelation(forwardDataSubRelFileId,
                              forwardDataSubRelTypeInfo,
                              labelForwardMultimap.size());
            labelForwardArel->setPartOfNrel(true);
            hlFillForwardOrReverseLabel(labelForwardArel,
                                        forwardDataRel, labelForwardMultimap);
            #ifdef USEDEBUG
LogDebug("add forward label to all labels" << endl);
#endif
            allLabelsTuple->PutAttribute(
                HL_INDEX_OF_FORWARD_LABEL_IN_ALL_LABELS_TUPLE,
                labelForwardArel);

            //Create reverse label
            isForward = false;
            #ifdef USEDEBUG
LogDebug("create reverse label" << endl);
#endif
            hlCreateLabelByDijkstraWithStalling(
                bTreeNodesRankedDescOrelRank,
                nodesRankedDescOrelRank,
                edgesContractedUpwardsOrelSource,
                edgesContractedDownwardsOrelTarget,
                labelReverseMultimap, nodeIdV, hHop, isForward);
            #ifdef USEDEBUG
            LogDebug("fully prune reverse label" << endl);
            #endif
            //hlPruneLabelByBootstrapping(allLabelsNrel,
            // labelReverseMultimap, nodeIdV, isForward);
            #ifdef USEDEBUG
            LogDebug("create and fill reverseLabel" << endl);
            #endif
            AttributeRelation* labelReverseArel = new
            AttributeRelation(reverseDataSubRelFileId,
                              reverseDataSubRelTypeInfo,
                              labelReverseMultimap.size());
            labelReverseArel->setPartOfNrel(true);
            hlFillForwardOrReverseLabel(labelReverseArel,
                                        reverseDataRel, labelReverseMultimap);

            #ifdef USEDEBUG
            LogDebug("add reverse label to all labels" << endl);
            #endif
            allLabelsTuple->PutAttribute(
                HL_INDEX_OF_REVERSE_LABEL_IN_ALL_LABELS_TUPLE,
                labelReverseArel);

            #ifdef USEDEBUG
            LogDebug("finish current all labels Tuple" << endl);
            #endif
            allLabelsNrel->getPrimary()->AppendTuple(
                allLabelsTuple);

            allLabelsTuple->DeleteIfAllowed();

            allLabelsTuple = 0;


            #ifdef USEDEBUG
            LogDebug("get next tuple" << endl);
            #endif
            //Free Outgoing-Iteration
            currentNodeV->DeleteIfAllowed();
            currentNodeV = 0;
            currentNodeV = nodesRankIter->GetNextTuple();
        }

        delete nodesRankIter;

        #ifdef USEDEBUG
LogDebug("reorder labels" << endl);
#endif
        //hlReorderLabels(); //XXTODO

        #ifdef USEDEBUG
LogDebug("finish hlCreateLabels" << endl);
#endif

        return true;
    }

    /*
     * Adds all Hubs of the given Label to the corresponding Attribute-
     *  and Data-Relation
     *
     * @param fwdOrRvsLabelArel is an AttributeRelation to be filled with
     *  data from the given multimap
     * @param fwdOrRvsDataRel is an Relation to be filled with data from the
     *  given multimap
     * @param fwdOrRvsLabelMultimap is a Multimap containging all final
     *  Label-Data
     * @return true, since all modification will be done within the
     *  given parameters
    */
    bool hlFillForwardOrReverseLabel(
        AttributeRelation* fwdOrRvsLabelArel,
        Relation* fwdOrRvsDataRel,
        std::multimap<int, std::tuple<int, int, double>>
        &fwdOrRvsLabelMultimap) const
    {
        #ifdef USEDEBUG
LogDebug("start hlFillForwardOrReverseLabel" <<
              endl);
#endif

        std::multimap<int, int> tmpIndexTupleIds;
        for (std::multimap<int, std::tuple<int, int, double>>::iterator
                fwdOrRvsLabelMultimapIter =
                    fwdOrRvsLabelMultimap.begin();
                fwdOrRvsLabelMultimapIter !=
                fwdOrRvsLabelMultimap.end();
                ++fwdOrRvsLabelMultimapIter)
        {
            #ifdef USEDEBUG
LogDebug("next iteration" << endl);
#endif

            int currNodeIdNew =
                (*fwdOrRvsLabelMultimapIter).first;
            std::tuple<int, int, double> currTuple =
                (*fwdOrRvsLabelMultimapIter).second;
            int currNodeId =
                std::get<HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE -
                1>(currTuple);
            int currParentNodeId =
                std::get<HL_INDEX_OF_HUB_PARENT_NODE_ID_IN_LABEL_TUPLE
                - 1>(currTuple);

            #ifdef USEDEBUG
LogDebug("currNode rank: " << currNodeIdNew <<
                  " id: " << currNodeId << " parent: " <<
                  currParentNodeId << endl);
#endif

            TupleType* tt1 = fwdOrRvsDataRel->GetTupleType();
            Tuple* relInsertTuple = new Tuple(tt1);


            //get tupleId of currParentNodeId from temporal structure
            #ifdef USEDEBUG
LogDebug("suche nach parent von " << currParentNodeId
                  << " anzahl gesamt vorhanden: " <<
                  tmpIndexTupleIds.size() << endl);
#endif
            std::multimap<int, int>::iterator
            parentTupleIdIter = tmpIndexTupleIds.find(
                                    currParentNodeId);

            //only check distances when current min tuple from h-hop
            // reverse search does exist in current forward label
            int currParentTupleId = -1;
            if(parentTupleIdIter != tmpIndexTupleIds.end())
            {
                currParentTupleId = (*parentTupleIdIter).second;
                #ifdef USEDEBUG
LogDebug("parent tupleId found (" <<
                      currParentTupleId << ") of parent nodeId: " <<
                      currParentNodeId << " for currTuple: " <<
                      currNodeId << endl);
#endif
            }

            #ifdef USEDEBUG
LogDebug("Add data to Data-Relation" << endl);
#endif
            relInsertTuple->PutAttribute(
                HL_INDEX_OF_HUB_NODE_ID_NEW_IN_LABEL_TUPLE,
                new CcInt(true, currNodeIdNew));
            relInsertTuple->PutAttribute(
                HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE,
                new CcInt(true, currNodeId));
            relInsertTuple->PutAttribute(
                HL_INDEX_OF_HUB_PARENT_NODE_ID_IN_LABEL_TUPLE,
                new CcInt(true, currParentNodeId));
            relInsertTuple->PutAttribute(
                HL_INDEX_OF_HUB_PARENT_TUPLE_ID_IN_LABEL_TUPLE,
                new CcInt(true, currParentTupleId));
            relInsertTuple->PutAttribute(
                HL_INDEX_OF_HUB_DISTANCE_TO_SOURCE_IN_LABEL_TUPLE,
                new CcReal(true,
                std::get<HL_INDEX_OF_HUB_DISTANCE_TO_SOURCE_IN_LABEL_TUPLE
                           - 2>(currTuple)));
            fwdOrRvsDataRel->AppendTuple(relInsertTuple);

            TupleId relInsertTupleId =
                relInsertTuple->GetTupleId();

            fwdOrRvsLabelArel->Append(relInsertTupleId);

            //save nodeId and tupleId in temporal structure
            tmpIndexTupleIds.insert(pair<int, int>(currNodeId,
                                                   relInsertTupleId));

            #ifdef USEDEBUG
LogDebug("Free Resources" << endl);
#endif
            relInsertTuple->DeleteIfAllowed();
        }

        #ifdef USEDEBUG
LogDebug("finish hlFillForwardOrReverseLabel" <<
              endl);
#endif

        return true;
    }


    /*
     * Scans the nodes Relation for the given nodeId and extracts
     *  the rank of the node and returns it.
     *  Returns -1 if no adequate
     *
     * @param nodesRanked is an OrderedRelation containging all
     *  nodes of the graph with their rank
     * @param givenNodeId is the given nodeId
     * @return the rank extracted or -1 if no node was found
    */
    int hlGetRankById(BTree* btreeNodesRanked,
                      OrderedRelation* nodesRanked,
                      CcInt* givenNodeId) const
    {
        #ifdef USEDEBUG
LogDebug("start hlGetRankById" << endl);
#endif

        int resultingRank = -1;
        int tupleIdRankedNodeInt = -1;

        BTreeIterator* btreeNodesRankedIter =
            btreeNodesRanked->ExactMatch(givenNodeId);

        if(btreeNodesRankedIter)
        {
            #ifdef USEDEBUG
LogDebug("using btree iterator" << endl);
#endif
            if(btreeNodesRankedIter->Next())
            {
                #ifdef USEDEBUG
LogDebug("next iterated element" << endl);
#endif
                tupleIdRankedNodeInt =
                    btreeNodesRankedIter->GetId();
                #ifdef USEDEBUG
LogDebug("tupleId: " << tupleIdRankedNodeInt <<
                      endl);
#endif
            }
            delete btreeNodesRankedIter;
        }

        TupleId tupleIdRankedNode = (TupleId)
                                    tupleIdRankedNodeInt;

        Tuple* rankedNodeTuple = nodesRanked->GetTuple(
                                     tupleIdRankedNode, false);

        if(!rankedNodeTuple) //XXTODO Debug/ korrigieren,
        // obiges findet kein tuple
        {
            #ifdef USEDEBUG
LogDebug("Warning, no tuple found" << endl);
#endif
            GenericRelationIterator* nodesRankedIter =
                nodesRanked->MakeScan();
            Tuple* tmpTuple = nodesRankedIter->GetNextTuple();
            while(tmpTuple)
            {

                CcInt* tmpId = (CcInt*) tmpTuple->GetAttribute(
                                   HL_INDEX_OF_ID_IN_NODES_RANKED);
                #ifdef USEDEBUG
LogDebug("next iteration nodeId: " <<
                      tmpId->GetIntval() << endl);
#endif

                if(tmpId->GetIntval() == givenNodeId->GetIntval())
                {
                    #ifdef USEDEBUG
LogDebug("right tuple found, going to break" <<
                          endl);
#endif
                    rankedNodeTuple = tmpTuple;
                    break;
                }
                else
                {
                    tmpTuple->DeleteIfAllowed();
                    tmpTuple = 0;
                    tmpTuple = nodesRankedIter->GetNextTuple();
                }
            }
            #ifdef USEDEBUG
LogDebug("finish iteration" << endl);
#endif
            delete nodesRankedIter;
        }

        if(rankedNodeTuple)
        {
            #ifdef USEDEBUG
LogDebug("tuple found" << endl);
#endif
            CcInt* currentRank = (CcInt*)
                                 rankedNodeTuple->GetAttribute(
                                     HL_INDEX_OF_RANK_IN_NODES_RANKED);
            resultingRank = currentRank->GetIntval();
            rankedNodeTuple->DeleteIfAllowed();
            #ifdef USEDEBUG
LogDebug("node: " << givenNodeId->GetIntval() <<
                  " rank: "<< currentRank->GetIntval() << endl);
#endif
        }

        #ifdef USEDEBUG
LogDebug("end hlGetRankById" << endl);
#endif

        return resultingRank;
    }


    /*
     * Creates a Lable (either forward or reverse) for the given source node.
     *
     * @param edgesContractedUpwardsOrelSource is an OrderedRelation
     *  containging all edges of the underlying graph sorted by the field Source
     * @param edgesContractedDownwardsOrelTarget is an OrderedRelation
     *  containging all edges of the underlying graph sorted by the field Target
     * @param fwdOrRvsLabelMultimap must be an empty Multimap representing the
     *  label which will be filled by this method
     * @param givenSourceIdS is the vertext ID of the current source node s for
     *  which the labels shall be created
     * @param hHop is a an integer which defines the h-Hop-Size used for local
     *  look-up queries which is used for stalling on demand
     * @param isForward is a boolean that indicates whether a forward or
     *  reverselabel shall be created
     * @return true since the result of this method is storen into
     *  fwdOrRvsLabelMultimap
    */
    bool hlCreateLabelByDijkstraWithStalling(
        BTree* btreeNodesRanked,
        OrderedRelation* nodesRanked,
        OrderedRelation* edgesContractedUpwardsOrelSource,
        OrderedRelation*
        edgesContractedDownwardsOrelTarget,
        std::multimap<int, std::tuple<int, int, double>>
        &fwdOrRvsLabelMultimap, CcInt* givenSourceIdS,
        int hHop, bool isForward) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlCreateLabelByDijkstraWithStalling"
              << endl);
        #endif
        int currRank = -1;

        #ifdef USEDEBUG
        LogDebug("insert source node itself into label" <<
              endl);
        #endif
        currRank = hlGetRankById(btreeNodesRanked,
                                 nodesRanked, givenSourceIdS);
        std::tuple<int, int, double> insertTupleSource =
            std::make_tuple(givenSourceIdS->GetIntval(), -1,
                            0.0);
        fwdOrRvsLabelMultimap.insert(
            pair<int, std::tuple<int, int, double>>(currRank,
                    insertTupleSource));

        #ifdef USEDEBUG
LogDebug("create labelNotYetVisitedNodes" << endl);
#endif
        std::multimap<double, std::tuple<int, int>>
                labelNotYetVisitedNodes;

        #ifdef USEDEBUG
LogDebug("create stillVisitedNodes" << endl);
#endif
        std::vector<int> labelStillVisitedNodes;
        labelStillVisitedNodes.push_back(
            givenSourceIdS->GetIntval());

        #ifdef USEDEBUG
LogDebug("process initial edges" << endl);
#endif
        hlCreateLabelScanNewVertices(
            labelNotYetVisitedNodes, labelStillVisitedNodes,
            edgesContractedUpwardsOrelSource,
            edgesContractedDownwardsOrelTarget,
            givenSourceIdS, 0.0, isForward);

        #ifdef USEDEBUG
LogDebug("process iterative edges" << endl);
#endif
        while(labelNotYetVisitedNodes.size() > 0)
        {
            #ifdef USEDEBUG
LogDebug("get next minV" << endl);
#endif
            std::multimap<double, std::tuple<int, int>>::iterator
                    labelNotYetVisitedNodesIter =
                        labelNotYetVisitedNodes.begin();
            double currMinVDist =
                (*labelNotYetVisitedNodesIter).first;

            std::tuple<int, int> currMinVPartTuple =
                (*labelNotYetVisitedNodesIter).second;
            int currMinVNodeId =
                std::get<HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED -
                1>(currMinVPartTuple);
            int currMinVParentNodeId =
                std::get<HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED
                - 1>(currMinVPartTuple);
            CcInt* currMinVNodeIdCcInt = new CcInt(true,
                                                   currMinVNodeId);
                                                    //zwingend per new erzeugen?

            #ifdef USEDEBUG
LogDebug("remove tuple from orel" << endl);
#endif
            labelNotYetVisitedNodes.erase(
                labelNotYetVisitedNodesIter);

            #ifdef USEDEBUG
LogDebug("Add to still visited Nodes" << endl);
#endif
            labelStillVisitedNodes.push_back(currMinVNodeId);

            #ifdef USEDEBUG
LogDebug("check for witness before inserting by doing"
             " h-Hop-Reverse-Search"
                  << endl);
#endif
            bool isPruned = hlCreateLabelCheckForWitness(
                                fwdOrRvsLabelMultimap,
                                edgesContractedUpwardsOrelSource,
                                edgesContractedDownwardsOrelTarget,
                                currMinVNodeIdCcInt, currMinVDist, hHop,
                                isForward);

            if(!isPruned)
            {
                #ifdef USEDEBUG
LogDebug("append tuple to label" << endl);
#endif
                currRank = -1;
                currRank = hlGetRankById(btreeNodesRanked,
                                         nodesRanked, currMinVNodeIdCcInt);
                std::tuple<int, int, double> insertTupleLabel =
                    std::make_tuple(currMinVNodeId,
                                    currMinVParentNodeId, currMinVDist);
                fwdOrRvsLabelMultimap.insert(
                    pair<int, std::tuple<int, int, double>>(currRank,
                            insertTupleLabel));

                #ifdef USEDEBUG
LogDebug("scan new vertices" << endl);
#endif
                hlCreateLabelScanNewVertices(
                    labelNotYetVisitedNodes, labelStillVisitedNodes,
                    edgesContractedUpwardsOrelSource,
                    edgesContractedDownwardsOrelTarget,
                    currMinVNodeIdCcInt, currMinVDist, isForward);
            }

            currMinVNodeIdCcInt->DeleteIfAllowed(
                true); //TODO: notwendig? bei getAttribute auch nicht notwendig
        }

        #ifdef USEDEBUG
LogDebug("finish hlCreateLabelByDijkstraWithStalling"
              << endl);
#endif

        return true;
    }



    /*
     * Scans for new vertices during dijkstra forward or reverse search used
     *  to create labels
     *
     * @param labelNotYetVisitedNodes is a Multimap by reference storing the
     *  vertices which are not yet visited by the dijkstra search
     * @param labelStillVisitedNodes is a vector by reference storing the
     *  vertices which are still visited by the dijkstra search
     * @param edgesContractedUpwardsOrelSource is an OrderedRelation
     *  containging all edges of the underlying graph sorted by the field Source
     * @param edgesContractedDownwardsOrelTarget is an OrderedRelation
     *  containging all edges of the underlying graph sorted by the field Target
     * @param givenSourceIdV is the vertext ID of the current source node v
     *  which was chosen as next vertex during dijkstra search
     * @param givenDistSV contains the weighted distance from the original
     *  source s to the current given node v
     * @param isForward is a boolean that indicates whether a forward or
     *  reverselabel shall be created
     * @return true since the result of this method is stored into
     *  labelNotYetVisitedNodes
    */
    bool hlCreateLabelScanNewVertices(
        std::multimap<double, std::tuple<int, int>>
        &labelNotYetVisitedNodes,
        std::vector<int> &labelStillVisitedNodes,
        OrderedRelation* edgesContractedUpwardsOrelSource,
        OrderedRelation*
        edgesContractedDownwardsOrelTarget,
        CcInt* givenSourceIdV, double givenDistSV,
        bool isForward) const
    {
        #ifdef USEDEBUG
LogDebug("Start hlCreateLabelScanNewVertices with isForward = "
              << isForward << endl);
#endif

        #ifdef USEDEBUG
LogDebug("prepare iteration" << endl);
#endif
        //iterate over all outgoing edges (v, w) (on forward search,
        // vice versa else)
        //Prepare Range scan
        //Create From and to by givenSourceIdV (=V)
        std::vector<void*> vecAttributesFromSourceV(1);
        vecAttributesFromSourceV[0] = givenSourceIdV;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesFromSourceV(1);
        vecAttrTypesFromSourceV[0] =
            givenSourceIdV->getSMIKeyType();
        CompositeKey fromSourceV(vecAttributesFromSourceV,
                                 vecAttrTypesFromSourceV, false);

        CcInt currentNodePlusOneSourceV(true,
                                        givenSourceIdV->GetIntval() + 1);
        CcInt* currentNodePlusOnePtrSourceV =
            &currentNodePlusOneSourceV;
        std::vector<void*> vecAttributesToSourceV(1);
        vecAttributesToSourceV[0] =
            currentNodePlusOnePtrSourceV;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesToSourceV(1);
        vecAttrTypesToSourceV[0] =
            currentNodePlusOnePtrSourceV->getSMIKeyType();
        CompositeKey toSourceV(vecAttributesToSourceV,
                               vecAttrTypesToSourceV, false);

        GenericRelationIterator*
        edgesContractedUpOrDownOrelSourceOrTargetIter;
        Tuple* currentContractedEdgeVW;
        if(isForward)
        {
            #ifdef USEDEBUG
LogDebug("get outgoing edges (v, w) within upwardsgraph"
                  << endl);
#endif
            edgesContractedUpOrDownOrelSourceOrTargetIter =
                edgesContractedUpwardsOrelSource->MakeRangeScan(
                    fromSourceV, toSourceV);
        }
        else
        {
            #ifdef USEDEBUG
LogDebug("get incoming edges (w, v) within downwardsgraph"
                  << endl);
#endif
            edgesContractedUpOrDownOrelSourceOrTargetIter =
                edgesContractedDownwardsOrelTarget->MakeRangeScan(
                    fromSourceV, toSourceV);
        }
        currentContractedEdgeVW =
            edgesContractedUpOrDownOrelSourceOrTargetIter->GetNextTuple();

        #ifdef USEDEBUG
LogDebug("iterate and store outgoing/ incoming egdes (v, w)/ (w, v)"
         " into labelNotYetVisited"
              << endl);
#endif
        while(currentContractedEdgeVW)
        {
            #ifdef USEDEBUG
LogDebug("next iteration" << endl);
#endif
            CcInt* currentSourceOrTargetW;
            if(isForward)
            {
                currentSourceOrTargetW = (CcInt*)
                                         currentContractedEdgeVW->GetAttribute(
                                             HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
            }
            else
            {
                currentSourceOrTargetW = (CcInt*)
                                         currentContractedEdgeVW->GetAttribute(
                                             HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
            }

            if(std::find(labelStillVisitedNodes.begin(),
                         labelStillVisitedNodes.end(),
                         currentSourceOrTargetW->GetIntval()) ==
                    labelStillVisitedNodes.end())
            {
                #ifdef USEDEBUG
LogDebug("append tuple to labelNotYet" << endl);
#endif

                //create auxiliary data
                //hopDepth is set to -1 because it doesnt matter but its
                // increased during insertOrUpdate and stored within
                // labelNotYet which does not have any effect yet but
                // is kind of 'dirty'
                std::tuple<double, int, int>
                currParentNotYetMultimapTuple = std::make_tuple(
                                givenDistSV, givenSourceIdV->GetIntval(), -1);

                //isHHop = false means to use the parentsNodeId
                // instead of the parentsHopDepth to store within notYet
                bool isHHop = false;
                #ifdef USEDEBUG
LogDebug("insert or update" << endl);
#endif
                //Either insert the new scanned W/V into not visited
                // list or update it if it sill exists and has a shorter
                // path via current v
                hlInsertOrUpdateTupleInNotYetVisitedList(-1,
                        currParentNotYetMultimapTuple,
                        currentContractedEdgeVW, labelNotYetVisitedNodes,
                        isForward, isHHop);
                #ifdef USEDEBUG
LogDebug("inserted or updated" << endl);
#endif
            }

            #ifdef USEDEBUG
LogDebug("get next tuple" << endl);
#endif
            //Free Outgoing-Iteration
            currentContractedEdgeVW->DeleteIfAllowed();
            currentContractedEdgeVW = 0;
            currentContractedEdgeVW =
                edgesContractedUpOrDownOrelSourceOrTargetIter->GetNextTuple();
        }
        #ifdef USEDEBUG
LogDebug("finish iteration" << endl);
#endif

        //Free resources
        delete edgesContractedUpOrDownOrelSourceOrTargetIter;

        #ifdef USEDEBUG
LogDebug("end hlCreateLabelScanNewVertices" << endl);
#endif
        return true;
    }

    /*
     * Scans for new vertices during dijkstra h-Hop reverse or forward
     *  search used to check the previously found node
     *  for a witness path in the opposite graph. see also
     *  hlCreateLabelCheckForWitness;
     *
     * @param fwdOrRvsLabelMultimap is a Multimap representing the label
     *  which will be filled by this method
     * @param reverseNotYetVisitedNodes is an OrderedRelation storing the
     *  vertices which are not yet visited by the h-hop reverse search
     * @param edgesContractedUpwardsOrelSource is an OrderedRelation
     *  containging all edges of the underlying upwardsgraph sorted by
     *  the field Source
     * @param edgesContractedDownwardsOrelTarget is an OrderedRelation
     *  containging all edges of the underlying downwardsgraph sorted by
     *  the field Target
     * @param fwdCurrSourceOrTargetW is the vertext ID of the current node
     *  w which was chosen as next vertex during forward (or reverse) search
     *         for which a witness path shall be found
     * @param fwdDistSW is a double representing the weighted distance
     *  between the actual source node s of this label and the given node
     *  w during forward (or reverse) search
     * @param hHop is a an integer representing maximum allowed hop-size
     *  used during this h-hop-reverse-search
     * @param isForward is a boolean that indicates whether a forward or
     *  reverselabel shall be created
     * @return true if the given node needs to be pruned out, false else
    */
    bool hlCreateLabelCheckForWitness(
        std::multimap<int, std::tuple<int, int, double>>
        &fwdOrRvsLabelMultimap,
        OrderedRelation* edgesContractedUpwardsOrelSource,
        OrderedRelation*
        edgesContractedDownwardsOrelTarget,
        CcInt* fwdCurrSourceOrTargetW, double fwdDistSW,
        int hHop, bool isForward) const
    {
        #ifdef USEDEBUG
LogDebug("Start hlCreateLabelCheckForWitness" <<
              endl);
#endif
        bool isPruned = false;

        #ifdef USEDEBUG
LogDebug("create reverseNotYetVisitedNodes" << endl);
#endif
        std::multimap<double, std::tuple<int, int>>
                reverseNotYetVisitedNodes;

        #ifdef USEDEBUG
LogDebug("create stillVisitedNodes" << endl);
#endif
        std::vector<int> stillVisitedNodes;
        stillVisitedNodes.push_back(
            fwdCurrSourceOrTargetW->GetIntval());

        //scan new vertices in h-hop reverse search
        hlCreateLabelCheckForWitnessScanNewVertices(
            reverseNotYetVisitedNodes, stillVisitedNodes,
            edgesContractedUpwardsOrelSource,
            edgesContractedDownwardsOrelTarget,
            fwdCurrSourceOrTargetW, 0.0, 0, isForward);

        #ifdef USEDEBUG
LogDebug("process iterative edges in hHop-reverse" <<
              endl);
#endif
        while(reverseNotYetVisitedNodes.size() > 0)
        {
            #ifdef USEDEBUG
LogDebug("next iteration witness" << endl);
#endif

            #ifdef USEDEBUG
LogDebug("get next revMinV" << endl);
#endif
            std::multimap<double, std::tuple<int, int>>::iterator
                    reverseNotYetVisitedNodesIter =
                        reverseNotYetVisitedNodes.begin();
            double revMinVDistToW =
                (*reverseNotYetVisitedNodesIter).first;

            std::tuple<int, int> currRevMinVTuple =
                (*reverseNotYetVisitedNodesIter).second;
            int revMinVNodeID =
                std::get<HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED -
                1>(currRevMinVTuple);
            int revMinVHopDepth =
                std::get<HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED
                - 1>(currRevMinVTuple);

            #ifdef USEDEBUG
LogDebug("remove tuple from orel" << endl);
#endif
            reverseNotYetVisitedNodes.erase(
                reverseNotYetVisitedNodesIter);

            #ifdef USEDEBUG
LogDebug("check for witness for: " << revMinVNodeID
                  << endl);
#endif
            std::multimap<int, std::tuple<int, int, double>>::iterator
                    fwdOrRvsLabelMultimapIter =
                        fwdOrRvsLabelMultimap.find(revMinVNodeID);

            //only check distances when current min tuple from h-hop
            // reverse search does exist in current forward label
            if(fwdOrRvsLabelMultimapIter !=
                    fwdOrRvsLabelMultimap.end())
            {
                #ifdef USEDEBUG
LogDebug("potential witness access found" << endl);
#endif
                std::tuple<int, int, double> currLabelTuple =
                    (*fwdOrRvsLabelMultimapIter).second;
                double revDistSV =
                    std::get<HL_INDEX_OF_HUB_DISTANCE_TO_SOURCE_IN_LABEL_TUPLE
                    - 2>(currLabelTuple); //its -2 here because the
                    // multimap does not contain the hubTupleId usually
                    // on indexposition #3

                if(revDistSV + revMinVDistToW <=
                        fwdDistSW) //XXTODO: auf kleiner oder kleiner-gleich
                        // prüfen? kleiner-gleich erzeugt kleinere labels,
                        // aber funktioniert der algorithmus dann noch?
                {
                    #ifdef USEDEBUG
LogDebug("witness found" << endl);
#endif
                    isPruned = true;
                    break;
                }
            }

            #ifdef USEDEBUG
LogDebug("scan new vertices" << endl);
#endif
            if(hHop > revMinVHopDepth)
            {
                CcInt* revMinVNodeIDCcInt = new CcInt(true,
                                                      revMinVNodeID);
                                            //zwingend per new erzeugen?
                hlCreateLabelCheckForWitnessScanNewVertices(
                    reverseNotYetVisitedNodes, stillVisitedNodes,
                    edgesContractedUpwardsOrelSource,
                    edgesContractedDownwardsOrelTarget,
                    revMinVNodeIDCcInt, revMinVDistToW,
                    revMinVHopDepth, isForward);
                revMinVNodeIDCcInt->DeleteIfAllowed(
                    true); //TODO: notwendig? bei getAttribute
                    // auch nicht notwendig
            }
        }

        #ifdef USEDEBUG
LogDebug("finish hlCreateLabelCheckForWitness" <<
              endl);
#endif
        return isPruned;
    }


    /*
     * Scans for new vertices during dijkstra h-Hop reverse or
     *  forward search used to check the previously found node
     *  for a witness path in the opposite graph. see also
     *  hlCreateLabelCheckForWitness;
     *
     * @param reverseNotYetVisitedNodes is a Multimap by reference storing
     *  the vertices which are not yet visited by the h-hop reverse search
     * @param reverseStillVisitedNodes is a vector by reference storing the
     *  vertices which are still visited by the h-hop reverse search
     * @param edgesContractedUpwardsOrelSource is an OrderedRelation
     *  containging all edges of the underlying upwardsgraph sorted
     *  by the field Source
     * @param edgesContractedDownwardsOrelTarget is an OrderedRelation
     *  containging all edges of the underlying downwardsgraph sorted
     *  by the field Target
     * @param revGivenTargetIdV is the vertext ID of the current source
     *  node v which was chosen as next vertex to be scanned during
     *  h-hop reverse (or forward) search
     * @param revGivenDistanceVToW contains the weighted distance from the
     *  original source w (of this reverse search) to the current given node v
     * @param revGivenHopDepthV is a an integer representing the current
     *  hop-depth of the given node v
     * @param isForward is a boolean that indicates whether a forward or
     *  reverselabel shall be created
     * @return true since the result of this method is stored into
     *  reverseNotYetVisitedNodes
    */
    bool hlCreateLabelCheckForWitnessScanNewVertices(
        std::multimap<double, std::tuple<int, int>>
        &reverseNotYetVisitedNodes,
        std::vector<int> &reverseStillVisitedNodes,
        OrderedRelation* edgesContractedUpwardsOrelSource,
        OrderedRelation*
        edgesContractedDownwardsOrelTarget,
        CcInt* revGivenTargetIdV,
        double revGivenDistanceVToW,
        int revGivenHopDepthV, bool isForward) const
    {
        #ifdef USEDEBUG
LogDebug("Start hlCreateLabelCheckForWitnessScanNewVertices"
              << endl);
#endif

        #ifdef USEDEBUG
LogDebug("prepare iteration" << endl);
#endif
        //iterate over all incoming edges (u, v)
        // (on isForward = true search, vice versa else)
        //Prepare Range scan
        //Create From and to by revGivenTargetIdV (=v)
        std::vector<void*> vecAttributesFromSourceV(1);
        vecAttributesFromSourceV[0] = revGivenTargetIdV;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesFromSourceV(1);
        vecAttrTypesFromSourceV[0] =
            revGivenTargetIdV->getSMIKeyType();
        CompositeKey fromSourceV(vecAttributesFromSourceV,
                                 vecAttrTypesFromSourceV, false);

        CcInt givenTargetIdVPlusOne(true,
                                    revGivenTargetIdV->GetIntval() + 1);
        CcInt* givenTargetIdVPlusOnePtr =
            &givenTargetIdVPlusOne;
        std::vector<void*> vecAttributesToSourceV(1);
        vecAttributesToSourceV[0] =
            givenTargetIdVPlusOnePtr;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesToSourceV(1);
        vecAttrTypesToSourceV[0] =
            givenTargetIdVPlusOnePtr->getSMIKeyType();
        CompositeKey toSourceV(vecAttributesToSourceV,
                               vecAttrTypesToSourceV, false);

        GenericRelationIterator*
        reverseSearchEdgeOrelIter;
        Tuple* currentReverseEdgeUV;

        if(isForward)
        {
            #ifdef USEDEBUG
LogDebug("get incoming edges (u, v) within downwardsgraph"
                  << endl);
#endif
            reverseSearchEdgeOrelIter =
                edgesContractedDownwardsOrelTarget->MakeRangeScan(
                    fromSourceV, toSourceV);
        }
        else
        {
            #ifdef USEDEBUG
LogDebug("get outgoing edges (v, u) within upwardsgraph"
                  << endl);
#endif
            reverseSearchEdgeOrelIter =
                edgesContractedUpwardsOrelSource->MakeRangeScan(
                    fromSourceV, toSourceV);
        }
        currentReverseEdgeUV =
            reverseSearchEdgeOrelIter->GetNextTuple();

        #ifdef USEDEBUG
LogDebug("iterate and store incoming/outgoing egdes (u, v)/(v, u)"
         " into reverseNotYetVisited"
              << endl);
#endif
        while(currentReverseEdgeUV)
        {
            #ifdef USEDEBUG
LogDebug("next iteration witness scan new vertices"
                  << endl);
#endif

            //Get ID of current TargetNode (=x)
            CcInt* currentTargetNodeX;
            if(isForward)
            {
                //on isForward = true get source because its the
                // reverse search here
                currentTargetNodeX = (CcInt*)
                                     currentReverseEdgeUV->GetAttribute(
                                         HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
            }
            else
            {
                currentTargetNodeX = (CcInt*)
                                     currentReverseEdgeUV->GetAttribute(
                                         HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
            }

            if(std::find(reverseStillVisitedNodes.begin(),
                         reverseStillVisitedNodes.end(),
                         currentTargetNodeX->GetIntval()) ==
                    reverseStillVisitedNodes.end())
            {
                #ifdef USEDEBUG
LogDebug("append tuple to reverseNotYet" << endl);
#endif

                //create auxiliary data
                std::tuple<double, int, int>
                currParentNotYetMultimapTuple = std::make_tuple(
                                                    revGivenDistanceVToW,
                                    revGivenTargetIdV->GetIntval(),
                                                    revGivenHopDepthV);
                #ifdef USEDEBUG
LogDebug("insert or update" << endl);
#endif
                //Either insert the new scanned W/V into not visited list
                // or update it if it sill exists and has a shorter
                // path via current v
                bool isHHop = true;
                hlInsertOrUpdateTupleInNotYetVisitedList(-1,
                        currParentNotYetMultimapTuple,
                        currentReverseEdgeUV, reverseNotYetVisitedNodes,
                        not isForward, isHHop);
                #ifdef USEDEBUG
LogDebug("inserted or updated" << endl);
#endif
            }

            #ifdef USEDEBUG
LogDebug("get next tuple" << endl);
#endif
            //Free Outgoing-Iteration
            currentReverseEdgeUV->DeleteIfAllowed();
            currentReverseEdgeUV = 0;
            currentReverseEdgeUV =
                reverseSearchEdgeOrelIter->GetNextTuple();
        }
        #ifdef USEDEBUG
LogDebug("finish iteration" << endl);
#endif

        //Free resources
        delete reverseSearchEdgeOrelIter;

        #ifdef USEDEBUG
        LogDebug("end hlCreateLabelCheckForWitnessScanNewVertices"
              << endl);
        #endif
        return true;
    }


    /*
     * Fully prunes the given label by running HL itself (bootstrapping),
     *  an OrderedRelation containing all labels (forward and reverse)
     *  for all nodes
     * @param allLabelsOrel is an OrderedRelation containging all nodes of
     *  the underlying graph sorted by the field NodeIdNew
     * @param currentForwardOrReverseLabel is an forward or reverse label
     * @param currentSourceNode is the actual source node of the given
     *  forward or reverse label
     * @param isForward is a boolean that indicates whether a forward or
     *  reverselabel shall be created
     * @return an OrderedRelation containing all labels (forward and reverse)
     *  for all nodes within @see nodesRankedDescOrelRank
    */
    bool hlPruneLabelByBootstrapping(OrderedRelation*
                                     allLabelsOrel, OrderedRelation*
                                     currentForwardOrReverseLabel,
                            CcInt* currentSourceNode, bool isForward) const
    {
        //Vector containing node ids to be deleted from
        // orelXT after finishing full iteration
        std::vector<int> hubsToDeleteVector;

        GenericRelationIterator*
        currentForwardOrReverseLabelIter =
            currentForwardOrReverseLabel->MakeScan();
        Tuple* currentHubTuple =
            currentForwardOrReverseLabelIter->GetNextTuple();

        while(currentHubTuple)
        {
            CcInt* currentHubId = (CcInt*)
                                  currentHubTuple->GetAttribute(
                                      HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE);
            CcReal* currentHubDistance = (CcReal*)
                                         currentHubTuple->GetAttribute(
                            HL_INDEX_OF_HUB_DISTANCE_TO_SOURCE_IN_LABEL_TUPLE);
            CcReal* currentDistanceSourceToHub = new CcReal(
                true, 0.0); //XXTODO mit richtiger Distance aus query füllen
            //hlQuery(allLabelsOrel, currentSourceNode,
            // currentHubId, isForward);

            if(currentDistanceSourceToHub->GetRealval() <
                    currentHubDistance->GetRealval())
            {
                #ifdef USEDEBUG
LogDebug("Witness found by bootstrapping" << endl);
#endif

                //prune this hub out
                //Build list containing all hubs to be deleted
                // later after iteration has finished
                hubsToDeleteVector.push_back(
                    currentHubId->GetIntval());
            }

            currentHubTuple->DeleteIfAllowed();
            currentHubTuple = 0;
            currentHubTuple =
                currentForwardOrReverseLabelIter->GetNextTuple();
        }
        #ifdef USEDEBUG
LogDebug("Free resources" << endl);
#endif

        delete currentForwardOrReverseLabelIter;

        for(std::vector<int>::iterator vecIter =
                    hubsToDeleteVector.begin();
                vecIter != hubsToDeleteVector.end(); ++vecIter)
        {
            hlRemoveTuplesFromOrel(
                currentForwardOrReverseLabel, *vecIter,
                HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE); //TODO: ggf.
                // rangescan in subfunktion ausführen um laufzeit zu sparen
        }
        #ifdef USEDEBUG
LogDebug("finish" << endl);
#endif

        return true;
    }


    /*
     * XXTODO
    */
    bool hlReorderLabels() const
    {
        //XXTODO
        return true;
    }


    /*
     * Gets all edges by the given source and iterates over them searching
     * for the given target.
     * Returns the first edge-Tuple if found.
     *
     * @param currOrel
     * @param sourceId
     * @param targetId
     * @return a new Tuple-Pointer
    */
    Tuple* hlGetEdgeFromOrel(OrderedRelation* currOrel,
     CcInt* sourceId, CcInt* targetId) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlGetEdgeFromOrel" << endl);
        #endif

        std::vector<void*> vecAttributesFrom(1);
        vecAttributesFrom[0] = sourceId;
        std::vector<SmiKey::KeyDataType> vecAttrTypesFrom(
            1);
        vecAttrTypesFrom[0] = sourceId->getSMIKeyType();
        CompositeKey from(vecAttributesFrom,
                          vecAttrTypesFrom, false);

        CcInt sourceIdPlusOne(true,
                              sourceId->GetIntval() + 1);
        CcInt* sourceIdPlusOnePtr = &sourceIdPlusOne;
        std::vector<void*> vecAttributesTo(1);
        vecAttributesTo[0] = sourceIdPlusOnePtr;
        std::vector<SmiKey::KeyDataType> vecAttrTypesTo(
            1);
        vecAttrTypesTo[0] =
            sourceIdPlusOnePtr->getSMIKeyType();
        CompositeKey to(vecAttributesTo, vecAttrTypesTo,
                        false);

        GenericRelationIterator* currOrelIter =
            currOrel->MakeRangeScan(from, to);
        Tuple* currTuple = currOrelIter->GetNextTuple();

        while(currTuple)
        {
            CcInt* currTargetId = (CcInt*) currTuple->GetAttribute(
             HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);

            if(currTargetId->GetIntval() == targetId->GetIntval())
            {
                #ifdef USEDEBUG
                LogDebug("tupleFound, break" << endl);
                #endif
                break;
            }
            else
            {
                currTuple->DeleteIfAllowed();
                currTuple = 0;
                currTuple = currOrelIter->GetNextTuple();
            }
        }
        delete currOrelIter;

        #ifdef USEDEBUG
        LogDebug("finish hlGetEdgeFromOrel" << endl);
        #endif
        return currTuple;
    }

    /*
     * Gets an shortcut edge tuple and retrieves its both parent edges
     * If one of the parent-edges are also shortcuts,
     *   the function itself is called recursively for it.
     *
     * @param edgesOrelSource the orel containing all edges including shortcuts
     * @param currEdge the current edge to be resolved
     * @param currPath the vector where all original edges resolved shall
     *  be stored into
     * @param isForward indicates whether we perform the forward or reverse
     *  label, used for input order of shortcut-parts
     * @return true since the result of this operator is stored into currPath
    */
    bool hlResolveShortcuts(OrderedRelation* edgesOrelSource, Tuple* currEdge,
     std::vector<Tuple*> &currPath, bool isForward) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlResolveShortcuts" << endl);
        #endif

        //Get parameters of currEdge
        CcInt* sourceId = (CcInt*) currEdge->GetAttribute(
                              HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
        CcInt* targetId = (CcInt*) currEdge->GetAttribute(
                              HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);

        //get parentId from current edgeTuple
        CcInt* parentId = (CcInt*) currEdge->GetAttribute(
         HL_INDEX_OF_PARENT_ID_IN_EDGE_TUPLE);

        Tuple* parentTuple = 0;

        //check whether curr Edge is shortcut
        if(parentId->GetIntval() > -1)
        {
            #ifdef USEDEBUG
            LogDebug("source: " << sourceId->GetIntval() <<
                  " parent: " << parentId->GetIntval() <<
                  " target: " << targetId->GetIntval() <<
                  " is shortcut, doing recursive call" << endl);
            #endif
            if(isForward)
            {
                //reverse insert because we will reverse output them because
                // we handle forward label here
                // (which means that we are inserting the hub first, then its
                // parents and the source last
                #ifdef USEDEBUG
                LogDebug("edge from parent: " <<
                      parentId->GetIntval() << " to target: " <<
                      targetId->GetIntval() << endl);
                #endif
                parentTuple = hlGetEdgeFromOrel(edgesOrelSource,
                                                parentId, targetId);
                hlResolveShortcuts(edgesOrelSource, parentTuple,
                                   currPath, isForward);

                #ifdef USEDEBUG
                LogDebug("edge from source: " <<
                      sourceId->GetIntval() << " to parent: " <<
                      parentId->GetIntval() << endl);
                #endif
                parentTuple = hlGetEdgeFromOrel(edgesOrelSource,
                                                sourceId, parentId);
                hlResolveShortcuts(edgesOrelSource, parentTuple,
                                   currPath, isForward);

            }
            else
            {
                //vice versa as forward
                #ifdef USEDEBUG
                LogDebug("edge from source: " <<
                      sourceId->GetIntval() << " to parent: " <<
                      parentId->GetIntval() << endl);
                #endif
                parentTuple = hlGetEdgeFromOrel(edgesOrelSource,
                                                sourceId, parentId);
                hlResolveShortcuts(edgesOrelSource, parentTuple,
                                   currPath, isForward);

                #ifdef USEDEBUG
                LogDebug("edge from parent: " <<
                      parentId->GetIntval() << " to target: " <<
                      targetId->GetIntval() << endl);
                #endif
                parentTuple = hlGetEdgeFromOrel(edgesOrelSource,
                                                parentId, targetId);
                hlResolveShortcuts(edgesOrelSource, parentTuple,
                                   currPath, isForward);
            }

            currEdge->DeleteIfAllowed();
            currEdge = 0;
        }
        else
        {
            #ifdef USEDEBUG
            LogDebug("is original Edge, pushback into path-vector" << endl);
            #endif
            currPath.push_back(currEdge);
        }

        #ifdef USEDEBUG
        LogDebug("finish hlResolveShortcuts" << endl);
        #endif
        return true;
    }

    /*
     * Gets an shortcut edge tuple and retrieves its both parent edges
     * If one of the parent-edges are also shortcuts,
     *   the function itself is called recursively for it.
     *
     * @param edgesOrelSource the orel containing all edges but not shortcuts
     * @param hlGraph the orel representing the hlGraph
     * @param sourceNodeId the sourceNodeId
     * @param targetNodeId the targetNodeId
     * @param currPath the vector where all original edges resolved shall
     *  be stored into
     * @param isForward indicates whether we perform the forward or reverse
     *  label, used for input order of shortcut-parts
     * @param isUpward indicates whether we perform an upward or downward edge
     * @return true since the result of this operator is stored into currPath
    */
    bool hlResolveShortcutsHlGraph(OrderedRelation* edgesOrelSource,
     OrderedRelation* hlGraph, CcInt* sourceNodeId, CcInt* targetNodeId,
     std::vector<Tuple*> &currPath, bool isForward, bool isUpward) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlResolveShortcutsHlGraph" << endl);
        #endif

        Tuple* currHlGraphEdge = hlGetEdgeFromOrel(hlGraph,
             sourceNodeId, targetNodeId);

        //get parentId from current edgeTuple
        int currFieldIndex = -1;
        if(isUpward)
        {
            currFieldIndex = HL_INDEX_OF_PARENT_ID_FORWARD_IN_HL_GRAPH;
        }else{
            currFieldIndex = HL_INDEX_OF_PARENT_ID_REVERSE_IN_HL_GRAPH;
        }
        CcInt* parentNodeId =
         (CcInt*) currHlGraphEdge->GetAttribute(currFieldIndex);

        //check whether curr Edge is shortcut
        if(parentNodeId->GetIntval() > -1)
        {
            #ifdef USEDEBUG
            LogDebug("source: " << sourceNodeId->GetIntval() <<
                  " parent: " << parentNodeId->GetIntval() <<
                  " target: " << targetNodeId->GetIntval() <<
                  " is shortcut, doing recursive call" << endl);
            #endif

            //there are four cases
            // 1) isForwardLabel and isUpwardEdge
            // 2) isForwardLabel and not isUpwardEdge
            // 3) not isForwardLabel and not isUpwardEdge
            // 4) not isForwardLabel and isUpwardEdge
            //
            // in case of 1) and 2) we want sub-edges parent->target
            //  before parent->source
            if(isUpward == isForward)
            {
                //first process edge from parent to target
                // second process edge from parent to souce
                // is valid for both forward and reverse processing
                #ifdef USEDEBUG
                LogDebug("next is edge from parent: " <<
                      parentNodeId->GetIntval() << " to target: " <<
                      targetNodeId->GetIntval() << endl);
                #endif
                hlResolveShortcutsHlGraph(edgesOrelSource, hlGraph,
                 parentNodeId, targetNodeId, currPath, isForward, isUpward);

                #ifdef USEDEBUG
                LogDebug("next is edge from parent: " <<
                      parentNodeId->GetIntval() << " to source: " <<
                      sourceNodeId->GetIntval() << endl);
                #endif
                hlResolveShortcutsHlGraph(edgesOrelSource, hlGraph,
                 parentNodeId, sourceNodeId, currPath, isForward, !isUpward);
            }
            else
            {
                //vice versa as upward
                #ifdef USEDEBUG
                LogDebug("edge from parent: " <<
                      parentNodeId->GetIntval() << " to source: " <<
                      sourceNodeId->GetIntval() << endl);
                #endif
                hlResolveShortcutsHlGraph(edgesOrelSource, hlGraph,
                 parentNodeId, sourceNodeId, currPath, isForward, !isUpward);

                #ifdef USEDEBUG
                LogDebug("edge from parent: " <<
                      parentNodeId->GetIntval() << " to target: " <<
                      targetNodeId->GetIntval() << endl);
                #endif
                hlResolveShortcutsHlGraph(edgesOrelSource, hlGraph,
                 parentNodeId, targetNodeId, currPath, isForward, isUpward);
            }
        }
        else
        {
            #ifdef USEDEBUG
            LogDebug("is original Edge, pushback into path-vector" << endl);
            #endif

            //get edge from edgesOrelSource
            // originalEdge only matches with current source/ target if
            // isUpwardEdge
            Tuple* currOriginalEdge = 0;
            if(isUpward)
            {
                currOriginalEdge = hlGetEdgeFromOrel(edgesOrelSource,
                 sourceNodeId, targetNodeId);
            }else{
                currOriginalEdge = hlGetEdgeFromOrel(edgesOrelSource,
                 targetNodeId, sourceNodeId);
            }


            currPath.push_back(currOriginalEdge);
        }

        currHlGraphEdge->DeleteIfAllowed();
        currHlGraphEdge = 0;

        #ifdef USEDEBUG
        LogDebug("finish hlResolveShortcutsHlGraph" << endl);
        #endif
        return true;
    }

    /*
     * Function traversing from the given hup to all its parents back to
     * the source or target (on reverse-hub)
     * Also resolves shortcuts by temporal creating edges on the path build
     *  up and find the
     *  corresponding edges within the Edges-Relation containing also shortcuts.
     * Inserts all original nodes represented by this shortcut and its
     *  underlying shortcuts
     *  also into the path-vector
     * After resolving a shortcut it comes back to the basic iteration over
     *  all parents of the hub.
     * @param edgesWithShortcutsOrelSource is an OrderedRelation containing
     *  all edges including shortcuts
     * @param dataRel is the Relation containing all data-tuples of the
     *  forward- and reverse labels
     * @param currHub is an Tuple representing the current hub for whichs
     *  parent a recursive call is made
     * @param path is a vector used to store all edges on the way from source
     *  to the hub
     * @param isForward is a boolean to indicate whether the path shall be
     *  retrieved from a forward or reverse Label
     * @return true, since the result ist stored in path
    */
    bool hlGetPathViaPoints(OrderedRelation* edgesWithShortcutsOrelSource,
     Relation* dataRel, Tuple* givenHubTuple,
     std::vector<Tuple*> &path, bool isForward) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlGetPathViaPoints" << endl);
        #endif

        bool isFinished = false;
        Tuple* currTuple = givenHubTuple;

        while(!isFinished)
        {
            CcInt* currTupleNodeId = (CcInt*) currTuple->GetAttribute(
                            HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE);
            CcInt* currTupleParentTupleId = (CcInt*) currTuple->GetAttribute(
                            HL_INDEX_OF_HUB_PARENT_TUPLE_ID_IN_LABEL_TUPLE);

            #ifdef USEDEBUG
                CcInt* currTupleNodeIdNew = (CcInt*) currTuple->GetAttribute(
                    HL_INDEX_OF_HUB_NODE_ID_NEW_IN_LABEL_TUPLE);
                LogDebug("current tuple rank: " <<
                  currTupleNodeIdNew->GetIntval() << " nodeId: " <<
                  currTupleNodeId->GetIntval() << endl);
            #endif

            if(currTupleParentTupleId->GetIntval() > -1)
            {
                TupleId currParentTupleId =
                 (TupleId) currTupleParentTupleId->GetIntval();
                Tuple* currParentTuple =
                 dataRel->GetTuple(currParentTupleId, false);

                if(!currParentTuple)
                {
                    #ifdef USEDEBUG
                    LogDebug("Fehler, Tuple aus dataRel ist null mit TupleId: "
                          << currParentTupleId << endl);
                    #endif
                    return false;
                }

                CcInt* currParentNodeId =
                 (CcInt*) currParentTuple->GetAttribute(
                 HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE);
                #ifdef USEDEBUG
                LogDebug("curr parentNodeId: " <<
                      currParentNodeId->GetIntval() <<
                      " parentTupleid: " << currParentTupleId << endl);
                #endif

                //get current Edge
                Tuple* currEdge = 0;
                CcInt* currSourceId = 0;
                CcInt* currTargetId = 0;
                if(isForward)
                {
                    currSourceId = currParentNodeId;
                    currTargetId = currTupleNodeId;
                }
                else
                {
                    currSourceId = currTupleNodeId;
                    currTargetId = currParentNodeId;
                }

                currEdge = hlGetEdgeFromOrel(edgesWithShortcutsOrelSource,
                 currSourceId, currTargetId);


                #ifdef USEDEBUG
                LogDebug("resolve shortcuts for source: " <<
                      currSourceId->GetIntval() << " target: " <<
                      currTargetId->GetIntval() << endl);
                #endif

                hlResolveShortcuts(edgesWithShortcutsOrelSource,
                 currEdge, path, isForward);

                currTuple->DeleteIfAllowed();
                currTuple = 0;
                currTuple = currParentTuple;

            }
            else
            {
                #ifdef USEDEBUG
                LogDebug("endpoint found exit" << endl);
                #endif

                currTuple->DeleteIfAllowed();
                currTuple = 0;
                isFinished = true;
            }
        }

        #ifdef USEDEBUG
        LogDebug("finish hlGetPathViaPoints" << endl);
        #endif

        return true;
    }

    /*
     * Function traversing from the given hup to all its parents back to
     * the source or target (on reverse-hub)
     * Also resolves shortcuts by temporal creating edges on the path build
     *  up and find the
     *  corresponding edges within the Edges-Relation containing also shortcuts.
     * Inserts all original nodes represented by this shortcut and its
     *  underlying shortcuts
     *  also into the path-vector
     * After resolving a shortcut it comes back to the basic iteration over
     *  all parents of the hub.
     * @param edgesOrelSource is an OrderedRelation containing
     *  all original edges but not shortcuts
     * @param hlGraphOrel is an OrderedRelation representing the hlGraph
     * @param dataRel is the Relation containing all data-tuples of the
     *  forward- and reverse labels
     * @param currHub is an Tuple representing the current hub for whichs
     *  parent a recursive call is made
     * @param path is a vector used to store all edges on the way from source
     *  to the hub
     * @param isForward is a boolean to indicate whether the path shall be
     *  retrieved from a forward or reverse Label
     * @return true, since the result ist stored in path
    */
    bool hlGetPathViaPointsHlGraph(OrderedRelation* edgesOrelSource,
     OrderedRelation* hlGraphOrel,
     Relation* dataRel, Tuple* givenHubTuple,
     std::vector<Tuple*> &path, bool isForward) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlGetPathViaPointsHlGraph" << endl);
        #endif

        bool isFinished = false;
        Tuple* currTuple = givenHubTuple;

        while(!isFinished)
        {
            CcInt* currTupleNodeId = (CcInt*) currTuple->GetAttribute(
                            HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE);
            CcInt* currTupleParentTupleId = (CcInt*) currTuple->GetAttribute(
                            HL_INDEX_OF_HUB_PARENT_TUPLE_ID_IN_LABEL_TUPLE);

            #ifdef USEDEBUG
                CcInt* currTupleNodeIdNew = (CcInt*) currTuple->GetAttribute(
                    HL_INDEX_OF_HUB_NODE_ID_NEW_IN_LABEL_TUPLE);
                LogDebug("current tuple rank: " <<
                  currTupleNodeIdNew->GetIntval() << " nodeId: " <<
                  currTupleNodeId->GetIntval() << endl);
            #endif

            if(currTupleParentTupleId->GetIntval() > -1)
            {
                TupleId currParentTupleId =
                 (TupleId) currTupleParentTupleId->GetIntval();
                Tuple* currParentTuple =
                 dataRel->GetTuple(currParentTupleId, false);

                if(!currParentTuple)
                {
                    #ifdef USEDEBUG
                    LogDebug("Fehler, Tuple aus dataRel ist null mit TupleId: "
                          << currParentTupleId << endl);
                    #endif
                    return false;
                }

                CcInt* currParentNodeId =
                 (CcInt*) currParentTuple->GetAttribute(
                 HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE);
                #ifdef USEDEBUG
                LogDebug("curr parentNodeId: " <<
                      currParentNodeId->GetIntval() <<
                      " parentTupleid: " << currParentTupleId << endl);
                #endif

                //get current Edge
                CcInt* currSourceId = 0;
                CcInt* currTargetId = 0;

                //in both cases of forward and reverse Label
                // we need to get edges in upward manner
                // which means currTuple always is target
                // and currParent is always source
                currSourceId = currParentNodeId;
                currTargetId = currTupleNodeId;

                #ifdef USEDEBUG
                LogDebug("resolve shortcuts for source: " <<
                      currSourceId->GetIntval() << " target: " <<
                      currTargetId->GetIntval() << endl);
                #endif

                hlResolveShortcutsHlGraph(edgesOrelSource, hlGraphOrel,
                 currSourceId, currTargetId, path, isForward, isForward);

                currTuple->DeleteIfAllowed();
                currTuple = 0;
                currTuple = currParentTuple;
            }
            else
            {
                #ifdef USEDEBUG
                LogDebug("endpoint found exit" << endl);
                #endif

                currTuple->DeleteIfAllowed();
                currTuple = 0;
                isFinished = true;
            }
        }

        #ifdef USEDEBUG
        LogDebug("finish hlGetPathViaPointsHlGraph" << endl);
        #endif

        return true;
    }


    /*
     * Searches for the shortest path from the given sourceNodeId to
     *  the given targetNodeId
     *  and stores the path with all its via-points into the given relation
     *  shortestPath
     *  where the nodes are stored in the order of the path going from source
     *  to target.
     * @param allLabelsNRel is NestedList containing all Labels
     * @param allLabelsBTree is BTree over the NestedRelation containging
     * all Labels of all nodes
     * @param edgesWithShortcutsOrelSource is an OrderedRelation containing
     *  all edges including shortcuts (does not contain shortcuts in case
     *  of isHlGraph == true)
     * @param hlGraphOrel is an OrderedRelation containing
     *  the hlGraph or null if not isHlGraph == true
     * @param shortestPath is an Relation representing the result of this
     *  function
     * @param sourceNodeId is the actual source node of the given forward or
     *  reverse label
     * @param targetNodeId is a boolean that indicates whether a forward or
     *  reverselabel shall be created
     * @param isHlGraph indicates whether we are in (1) old logic or
     *  (2) new logic using hlGraph
     * @return true, since the result ist stored in shortestPath
    */
    bool hlQuery(NestedRelation* allLabelsNRel, BTree* allLabelsBTree,
                 OrderedRelation* edgesWithShortcutsOrelSource,
                 OrderedRelation* hlGraphOrel, Relation* shortestPath,
                 CcInt* sourceNodeId, CcInt* targetNodeId, bool isHlGraph) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlQuery" << endl);
        #endif

        #ifdef USEPERF
        double perfTime = getCurrentTimeInMs();
        double perfTime2 = 0.0;
        double perfTime3 = 0.0;
        #endif

        #ifdef USEDEBUG
        LogDebug("get btree iterator by source: " <<
              sourceNodeId->GetIntval() << " target: " <<
              targetNodeId->GetIntval() << endl);
        #endif

        BTreeIterator* bTreeSourceIter =
            allLabelsBTree->ExactMatch(sourceNodeId);
        BTreeIterator* bTreeTargetIter =
            allLabelsBTree->ExactMatch(targetNodeId);


        #ifdef USEDEBUG
        LogDebug("get next tuple from btree forward" <<
              endl);
        #endif

        int tupleIdSourceInt = -1;
        int tupleIdTargetInt = -1;

        if(bTreeSourceIter)
        {
            if(bTreeSourceIter->Next())
            {
                tupleIdSourceInt = bTreeSourceIter->GetId();
            }
            else
            {
                #ifdef USEDEBUG
                LogDebug("Fehler: kein Tuple in BTree für Source enthalten,"
                 " breche Anfrage ab"
                      << endl);
                #endif
                return false;
            }
            delete bTreeSourceIter;
        }
        else
        {
            #ifdef USEDEBUG
            LogDebug("Fehler: sourceNode wurde nicht in Labels gefunden,"
             " breche Anfrage ab"
                  << endl);
            #endif
            return false;
        }

        #ifdef USEDEBUG
        LogDebug("get next tuple from btree reverse" <<
              endl);
        #endif

        if(bTreeTargetIter)
        {
            if(bTreeTargetIter->Next())
            {
                tupleIdTargetInt = bTreeTargetIter->GetId();
            }
            else
            {
                #ifdef USEDEBUG
                LogDebug("Fehler: kein Tuple in BTree für Target enthalten,"
                 " breche Anfrage ab"
                      << endl);
                #endif
                return false;
            }
            delete bTreeTargetIter;
        }
        else
        {
            #ifdef USEDEBUG
            LogDebug("Fehler: targetNode wurde nicht in Labels gefunden,"
             " breche Anfrage ab"
                  << endl);
            #endif
            return false;
        }

        #ifdef USEDEBUG
        LogDebug("create TupleIds and get primary" << endl);
        #endif

        TupleId tupleIdSource = (TupleId) tupleIdSourceInt;
        TupleId tupleIdTarget = (TupleId) tupleIdTargetInt;

        Relation* primary = allLabelsNRel->getPrimary();

        #ifdef USEDEBUG
        LogDebug("get tuple from primary" << endl);
        #endif

        //gets tuplepointers created by new
        Tuple* tupleSource = primary->GetTuple(tupleIdSource, false);
        Tuple* tupleTarget = primary->GetTuple(tupleIdTarget, false);


        #ifdef USEDEBUG
        LogDebug("get arel from tupleId " << endl);
        #endif

        AttributeRelation* attrRelSource =
            (AttributeRelation*) tupleSource->GetAttribute(
                HL_INDEX_OF_FORWARD_LABEL_IN_ALL_LABELS_TUPLE);
        AttributeRelation* attrRelTarget =
            (AttributeRelation*) tupleTarget->GetAttribute(
                HL_INDEX_OF_REVERSE_LABEL_IN_ALL_LABELS_TUPLE);



        #ifdef USEDEBUG
        LogDebug("get subrel from nrel" << endl);
        #endif

        SubRelation* dataSubRelForward =
            allLabelsNRel->getSubRel(
                hlForwardLabelColumnName());
        SubRelation* dataSubRelReverse =
            allLabelsNRel->getSubRel(
                hlReverseLabelColumnName());

        #ifdef USEDEBUG
        LogDebug("get datarel from subrel" << endl);
        #endif

        Relation* dataRelForward = dataSubRelForward->rel;
        Relation* dataRelReverse = dataSubRelReverse->rel;

        #ifdef USEDEBUG
                LogDebug("get tupleids from arels" << endl);
        #endif

        DbArray<TupleId>* tupleIdsForward =
            attrRelSource->getTupleIds();
        DbArray<TupleId>* tupleIdsReverse =
            attrRelTarget->getTupleIds();

        TupleId tidForward;
        TupleId tidReverse;

        int indexForward = 0;
        int indexReverse = 0;


        #ifdef USEDEBUG
        LogDebug("get tuple id for data rel from dbarray" <<
              endl);
        #endif

         //XXTODO: TupleId hier kein Zeiger?
        tupleIdsForward->Get(indexForward, tidForward);
         //XXTODO: TupleId hier kein Zeiger?
        tupleIdsReverse->Get(indexReverse, tidReverse);

        #ifdef USEDEBUG
        LogDebug("get tuples from datarel" << endl);
        #endif

             //XXTODO: TupleId hier kein Zeiger?
        Tuple* currTupleDataRelSource =
         dataRelForward->GetTuple(tidForward, false);
             //XXTODO: TupleId hier kein Zeiger?
        Tuple* currTupleDataRelTarget =
         dataRelReverse->GetTuple(tidReverse, false);


        Tuple* currMinHubSource = 0;
        Tuple* currMinHubTarget = 0;

        double currMinDist = -1.0;

        #ifdef USEDEBUG
        LogDebug("parallel sweep" << endl);
        #endif

        //usually breaks when tupleId-arrays reach end of index
        while(currTupleDataRelSource &&
                currTupleDataRelTarget)
        {
            //get HubNodeIds of both Iterators
            CcInt* currHubNodeIdNewSource =
             (CcInt*) currTupleDataRelSource->GetAttribute(
             HL_INDEX_OF_HUB_NODE_ID_NEW_IN_LABEL_TUPLE);
            CcInt* currHubNodeIdNewTarget =
             (CcInt*) currTupleDataRelTarget->GetAttribute(
             HL_INDEX_OF_HUB_NODE_ID_NEW_IN_LABEL_TUPLE);


            if(currHubNodeIdNewSource->GetIntval() ==
                    currHubNodeIdNewTarget->GetIntval())
            {

                #ifdef USEDEBUG
                LogDebug("source and target are equal: " <<
                      currHubNodeIdNewSource->GetIntval() << endl);
                #endif

                //get distances of both iterators
                CcReal* currHubDistToSource = (CcReal*)
                                currTupleDataRelSource->GetAttribute(
                        HL_INDEX_OF_HUB_DISTANCE_TO_SOURCE_IN_LABEL_TUPLE);
                CcReal* currHubDistToTarget = (CcReal*)
                                currTupleDataRelTarget->GetAttribute(
                            HL_INDEX_OF_HUB_DISTANCE_TO_SOURCE_IN_LABEL_TUPLE);

                //Check for shorter path
                double currMinDistTmp =
                    currHubDistToSource->GetRealval() +
                    currHubDistToTarget->GetRealval();
                if(currMinDist == -1.0 ||
                        currMinDistTmp < currMinDist)
                {
                    #ifdef USEDEBUG
                    LogDebug("new min found: " << currMinDistTmp << endl);
                    #endif
                    currMinDist = currMinDistTmp;

                    if(currMinHubSource)
                    {
                        //delete previous stored tuples for later use
                        currMinHubSource->DeleteIfAllowed();
                    }
                    //save new tuple with minimum distance for later use
                    currMinHubSource = currTupleDataRelSource;


                    if(currMinHubTarget)
                    {
                        //delete previous stored tuples for later use
                        currMinHubTarget->DeleteIfAllowed();
                    }
                    //save new tuple with minimum distance for later use
                    currMinHubTarget = currTupleDataRelTarget;

                    //dont delete current Tuples from interators,
                    // they are stored for later use
                    //currTupleDataRelSource = 0; //XXTODO: wäre das
                    // richtig oder sogar schlimm? oder egal?
                    //currTupleDataRelTarget = 0;
                }
                else
                {
                    //delete current Tuples from iterators
                    currTupleDataRelSource->DeleteIfAllowed();
                    currTupleDataRelSource = 0;
                    currTupleDataRelTarget->DeleteIfAllowed();
                    currTupleDataRelTarget = 0;
                }

                #ifdef USEDEBUG
                LogDebug("increase both" << endl);
                #endif

                //increase both iterators
                indexForward++;
                indexReverse++;

                if(tupleIdsForward->Size() > indexForward)
                {
                    tupleIdsForward->Get(indexForward, tidForward);
                }
                else
                {
                    #ifdef USEDEBUG
                    LogDebug("end of Array tupleIdsForward reached (size: "
                          << tupleIdsForward->Size() << " currIndex: " <<
                          indexForward << "), breaking loop" << endl);
                    #endif
                    break;
                }

                if(tupleIdsReverse->Size() > indexReverse)
                {
                    tupleIdsReverse->Get(indexReverse, tidReverse);
                }
                else
                {
                    #ifdef USEDEBUG
                    LogDebug("end of Array tupleIdsReverse reached (size: "
                          << tupleIdsReverse->Size() << " currIndex: " <<
                          indexReverse << "), breaking loop" << endl);
                    #endif
                    break;
                }

                #ifdef USEDEBUG
                LogDebug("increase both - set next tuples. forward: "
                      << tidForward << " reverse: " << tidReverse <<
                      endl);
                #endif

                currTupleDataRelSource = dataRelForward->GetTuple(
                                             tidForward, false);
                currTupleDataRelTarget = dataRelReverse->GetTuple(
                                             tidReverse, false);
            }
            else if(currHubNodeIdNewSource->GetIntval() <
                    currHubNodeIdNewTarget->GetIntval())
            {
                //increase forward iterator
                currTupleDataRelSource->DeleteIfAllowed();
                currTupleDataRelSource = 0;

                #ifdef USEDEBUG
                LogDebug("increase forward" << endl);
                #endif

                indexForward++;

                if(tupleIdsForward->Size() > indexForward)
                {
                    tupleIdsForward->Get(indexForward, tidForward);
                }
                else
                {
                    #ifdef USEDEBUG
                    LogDebug("end of Array tupleIdsForward reached (size: "
                          << tupleIdsForward->Size() << " currIndex: " <<
                          indexForward << "), breaking loop" << endl);
                    #endif
                    break;
                }

                #ifdef USEDEBUG
                LogDebug("increase forward - set next tuple. forward: "
                      << tidForward << endl);
                #endif
                currTupleDataRelSource = dataRelForward->GetTuple(
                                             tidForward, false);
                                        //XXTODO: TupleId hier kein Zeiger?
            }
            else
            {
                //increase reverse iterator
                currTupleDataRelTarget->DeleteIfAllowed();
                currTupleDataRelTarget = 0;

                #ifdef USEDEBUG
                LogDebug("increase reverse" << endl);
                #endif

                indexReverse++;

                if(tupleIdsReverse->Size() > indexReverse)
                {
                    tupleIdsReverse->Get(indexReverse, tidReverse);
                }
                else
                {
                    #ifdef USEDEBUG
                    LogDebug("end of Array tupleIdsReverse reached (size: "
                          << tupleIdsReverse->Size() << " currIndex: " <<
                          indexReverse << "), breaking loop" << endl);
                    #endif
                    break;
                }

                #ifdef USEDEBUG
                LogDebug("increase reverse - set next tuple. reverse: "
                      << tidReverse << endl);
                #endif
                currTupleDataRelTarget =
                 dataRelReverse->GetTuple(tidReverse, false);
                                             //XXTODO: TupleId hier kein Zeiger?
            }
        }

        tupleSource->DeleteIfAllowed();
        tupleTarget->DeleteIfAllowed();

        #ifdef USEPERF
        perfTime2 = getCurrentTimeInMs();
        LogPerf("perf: duration of actual hl search: (ms) "
         << fixed << (perfTime2 - perfTime) << endl);
        #endif

        std::vector<Tuple*> pathForward;
        std::vector<Tuple*> pathReverse;

        #ifdef USEDEBUG
        LogDebug("lookup paths" << endl);
        #endif

        if(currMinHubSource)
        {
            #ifdef USEDEBUG
            LogDebug("get PathVia for currMinHubSource" << endl);
            #endif

            //differ between (1) old logic and (2) new logic
            if(isHlGraph)
            {
                hlGetPathViaPointsHlGraph(edgesWithShortcutsOrelSource,
                 hlGraphOrel, dataRelForward, currMinHubSource,
                 pathForward, true);
            }else{
                hlGetPathViaPoints(edgesWithShortcutsOrelSource,
                 dataRelForward, currMinHubSource, pathForward, true);
            }
        }
        else
        {
            #ifdef USEDEBUG
            LogDebug("Warnung, Tuple currMinHubSource ist null, fahre fort"
                  << endl);
            #endif
        }
        if(currMinHubTarget)
        {
            #ifdef USEDEBUG
            LogDebug("get PathVia for currMinHubTarget" << endl);
            #endif

            if(isHlGraph)
            {
                hlGetPathViaPointsHlGraph(edgesWithShortcutsOrelSource,
                 hlGraphOrel, dataRelReverse, currMinHubTarget,
                 pathReverse, false);
            }else{
                hlGetPathViaPoints(edgesWithShortcutsOrelSource,
                 dataRelReverse, currMinHubTarget, pathReverse, false);
            }
        }
        else
        {
            #ifdef USEDEBUG
            LogDebug("Warnung, Tuple currMinHubTarget ist null, fahre fort"
                  << endl);
            #endif
        }

        #ifdef USEDEBUG
        LogDebug("iterate forward path" << endl);
        #endif

        //Reverse-Iterate through pathForward and add edges
        // (source and target) to shortestPath
        for (std::vector<Tuple*>::reverse_iterator
                pathForwardIterReverse = pathForward.rbegin();
                pathForwardIterReverse != pathForward.rend();
                ++pathForwardIterReverse )
        {
            #ifdef USEDEBUG
            LogDebug("insert next edge" << endl);
            #endif
            Tuple* currEdge = *(pathForwardIterReverse);

            #ifdef USEDEBUG
            CcInt* sourceTmp = (CcInt*)
                               currEdge->GetAttribute(
                                   HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
            CcInt* targetTmp = (CcInt*)
                               currEdge->GetAttribute(
                                   HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
            LogDebug("append tuple forward: source = " <<
                  sourceTmp->GetIntval() << " target:  " <<
                  targetTmp->GetIntval() << endl);
            #endif

            shortestPath->AppendTuple(currEdge);
            currEdge->DeleteIfAllowed();
        }

        #ifdef USEDEBUG
        LogDebug("iterate reverse path" << endl);
        #endif

        //forward iterate through pathReverse and add it to shortestPath
        for (std::vector<Tuple*>::iterator pathReverseIter
                = pathReverse.begin();
                pathReverseIter != pathReverse.end();
                ++pathReverseIter)
        {
            #ifdef USEDEBUG
            LogDebug("insert next edge" << endl);
            #endif
            Tuple* currEdge = *(pathReverseIter);

            #ifdef USEDEBUG
            CcInt* sourceTmp = (CcInt*)
                               currEdge->GetAttribute(
                                   HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
            CcInt* targetTmp = (CcInt*)
                               currEdge->GetAttribute(
                                   HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
            LogDebug("append tuple reverse: source = " <<
                  sourceTmp->GetIntval() << " target:  " <<
                  targetTmp->GetIntval() << endl);
            #endif

            shortestPath->AppendTuple(currEdge);
            currEdge->DeleteIfAllowed();
        }

        #ifdef USEPERF
        perfTime3 = getCurrentTimeInMs();
        LogPerf("perf: duration of resolving shortest path: (ms) "
         << fixed << (perfTime3 - perfTime2) << endl);
        #endif

        #ifdef USEDEBUG
        LogDebug("finish hlQuery" << endl);
        #endif

        return true;
    }





/*

3.3 HubLabeling functionality of (2) In-Memory Approach

*/
    /*
     * OrderedRelation type for a set of Nodes contained within the hlGraph
     * Used for creation of new OrderedRelation Objects, new Tuples of
     *  this OrderedRelation or cloning or deleting of an
     *  OrderedRelation of this type
    */
    static const string hlGetHlGraphNodesOrelTypeInfo()
    {
        return "(" + OrderedRelation::BasicType() +
               hlGetHlGraphNodesTupleTypeInfo() +
               "(Source)" +
               ")";
    }


    /*
     * TupleType type for Edges
    */
    static const string hlGetHlGraphNodesTupleTypeInfo()
    {
        return "(" + Tuple::BasicType() +
                   "(" +
                   "(Source " + CcInt::BasicType() + ")" +
                   ")" +
               ")";
    }


    /*
     * Definition of Field-Indexes for HlGraph Orel Representation
    */
    static const int HL_INDEX_OF_SOURCE_IN_HL_GRAPH_NODES = 0;




    /*
     * OrderedRelation type for a HlGraph
     * Used for creation of new OrderedRelation Objects, new Tuples of
     *  this OrderedRelation or cloning or deleting of an
     *  OrderedRelation of this type
    */
    static const string hlGetHlGraphOrelTypeInfo()
    {
        return "(" + OrderedRelation::BasicType() +
               hlGetHlGraphTupleTypeInfo() +
               "(Source)" +
               ")";
    }


    /*
     * TupleType type for Edges
    */
    static const string hlGetHlGraphTupleTypeInfo()
    {
        return "(" + Tuple::BasicType() +
                   "(" +
                   "(Source " + CcInt::BasicType() + ")" +
                   "(Target " + CcInt::BasicType() + ")" +
                   "(SourceIndex " + CcInt::BasicType() + ")" +
                   "(TargetIndex " + CcInt::BasicType() + ")" +
                   "(SourceRank " + CcInt::BasicType() + ")" +
                   "(IsForward " + CcInt::BasicType() + ")" +
                   "(IsReverse " + CcInt::BasicType() + ")" +
                   "(ShortcutParentIdForward " + CcInt::BasicType() + ")" +
                   "(ShortcutParentIdReverse " + CcInt::BasicType() + ")" +
                   "(ShortcutParentIndexForward " + CcInt::BasicType() + ")" +
                   "(ShortcutParentIndexReverse " + CcInt::BasicType() + ")" +
                   "(WeightForward " + CcReal::BasicType() + ")" +
                   "(WeightReverse " + CcReal::BasicType() + ")" +
                   ")" +
               ")";
    }




    /*
     * Definition of Field-Indexes for HlGraph Orel Representation
    */
    static const int HL_INDEX_OF_SOURCE_IN_HL_GRAPH = 0;
    static const int HL_INDEX_OF_TARGET_IN_HL_GRAPH = 1;
    static const int HL_INDEX_OF_SOURCE_INDEX_IN_HL_GRAPH = 2;
    static const int HL_INDEX_OF_TARGET_INDEX_IN_HL_GRAPH = 3;
    static const int HL_INDEX_OF_SOURCE_RANK_IN_HL_GRAPH = 4;
    static const int HL_INDEX_OF_IS_FORWARD_IN_HL_GRAPH = 5;
    static const int HL_INDEX_OF_IS_REVERSE_IN_HL_GRAPH = 6;
    static const int HL_INDEX_OF_PARENT_ID_FORWARD_IN_HL_GRAPH = 7;
    static const int HL_INDEX_OF_PARENT_ID_REVERSE_IN_HL_GRAPH = 8;
    static const int HL_INDEX_OF_PARENT_INDEX_FORWARD_IN_HL_GRAPH = 9;
    static const int HL_INDEX_OF_PARENT_INDEX_REVERSE_IN_HL_GRAPH = 10;
    static const int HL_INDEX_OF_WEIGHT_FORWARD_IN_HL_GRAPH = 11;
    static const int HL_INDEX_OF_WEIGHT_REVERSE_IN_HL_GRAPH = 12;



    /*
     * Gets an OrderedRelation containing Edges from OSM Import
     *  and transforms its contents into an internal graph structure.
     * The internal graph is represented as an adjacency list.
     * There all Edges are grouped by their sourceNodeId within HlNodeEntry.
     * An Edge entry only consists of its targetNodeId, the edge weight and
     *  two booleans to distinguish between original edges an so called
     *  reverse edges which are inverted edges stored additionally to
     *  perform backward searches.
     * That for theres also a second edgeWeight for those reverse edges
     *  in case an original edge from u to v has got another weight
     *  than the original edge from v to u.
     * Instead of storing the edges targetId we store the index position
     *  of the target node within the hlGraph.
     * This for we iterate through the given orel twice. First to get
     *  all index positions of all nodes, second to create the hlGraph
     *  using these index positions.
     *
     * @param nodesOrel the orel to be transformed
     * @param edgesOrel an orel just containing all nodes Ids of the OSM-Graph
     * @param nodesEdgesVector by reference the adjacency-list to fill
     *
     * @return true since all modifications take place in the given parameters
    */
    bool hlTransformOrelToHlGraph(OrderedRelation* nodesOrel,
     OrderedRelation* edgesOrel,
     std::vector<HlNodeEntry*> &nodesEdgesVector) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlTransformOrelToHlGraph" << endl);
        #endif

        //default rank to set within hlGraph, should be -1
        int defaultRank = -1;
        std::map<int, int> indexedNodeIds;

        GenericRelationIterator* nodesListOrelIter = nodesOrel->MakeScan();

        //Get first Tuple
        Tuple* currNode = nodesListOrelIter->GetNextTuple();

        int currIndex = 0;

        //Iterate over all Tuples first time
        // and create index-mapping
        while(currNode)
        {
            #ifdef USEDEBUG
            LogDebug("next edge from while" << endl);
            #endif
            CcInt* currNodeId = (CcInt*) currNode->GetAttribute(
                    HL_INDEX_OF_SOURCE_IN_HL_GRAPH_NODES);

            indexedNodeIds.insert(std::make_pair(currNodeId->GetIntval(),
             currIndex++));

            //get next Tuple from Orel
            currNode->DeleteIfAllowed();
            currNode = 0;
            currNode = nodesListOrelIter->GetNextTuple();
        }
        delete nodesListOrelIter;

        GenericRelationIterator*  edgesOrelIter = edgesOrel->MakeScan();

        //Get first Tuple
        Tuple* currEdge = edgesOrelIter->GetNextTuple();

        int currOrderedNodeId = -1;
        std::vector<HlEdgeEntry*>* currSourceEdgesVector = 0;
        std::vector<HlEdgeEntry*>* currTargetEdgesVector = 0;

        //Iterate over all Tuples second time
        // and create hlGraph
        while(currEdge)
        {
            #ifdef USEDEBUG
            LogDebug("next edge from while" << endl);
            #endif

            CcInt* currEdgeSource = (CcInt*) currEdge->GetAttribute(
                    HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
            CcInt* currEdgeTarget = (CcInt*) currEdge->GetAttribute(
                    HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
            CcReal* currEdgeWeight = (CcReal*) currEdge->GetAttribute(
                    HL_INDEX_OF_COSTS_IN_EDGE_TUPLE);

            int currSourceNodeId = currEdgeSource->GetIntval();
            int currTargetNodeId = currEdgeTarget->GetIntval();

            std::map<int, int>::iterator currSourceIndexIter =
             indexedNodeIds.find(currSourceNodeId);

            if(currSourceIndexIter == indexedNodeIds.end())
            {
                LogError("Fehler, SourceNode (" << currSourceNodeId
                 << ") der aktuellen Kante existiert nicht."
                 << endl);

                 hlFreeHlGraph(nodesEdgesVector);

                 return false;
            }


            std::map<int, int>::iterator currTargetIndexIter =
             indexedNodeIds.find(currTargetNodeId);

            if(currTargetIndexIter == indexedNodeIds.end())
            {
                LogError("Fehler, TargetNode (" << currTargetNodeId
                 << ") der aktuellen Kante existiert nicht."
                 << endl);

                 hlFreeHlGraph(nodesEdgesVector);

                 return false;
            }


            int currSourceIndex = (*currSourceIndexIter).second;
            int currTargetIndex = (*currTargetIndexIter).second;

            #ifdef USEDEBUG
            LogDebug("check nodeId (" << currSourceNodeId << ") is new (old: "
             << currOrderedNodeId << ")" << endl);
            #endif


            //check whether we reached next sourceId
            if(currOrderedNodeId != currSourceNodeId)
            {
                #ifdef USEDEBUG
                LogDebug("yes, it is new" << endl);
                #endif
                //set next nodeId and create new nodeEntry
                currOrderedNodeId = currSourceNodeId;

                //check whether current SourceNode still exists
                // due to reverse-edges
                HlNodeEntry* currHlNodeEntry =
                 nodesEdgesVector[currSourceIndex];

                if(currHlNodeEntry)
                {
                    #ifdef USEDEBUG
                    LogDebug("source node Id (" << currSourceNodeId <<
                     ") still exists" << endl);
                    #endif

                    currSourceEdgesVector = currHlNodeEntry->getEdgesVector();
                }else{
                    #ifdef USEDEBUG
                    LogDebug("source node Id (" << currSourceNodeId <<
                     ") does not exist" << endl);
                    #endif

                    //we need a new one
                    currSourceEdgesVector = new std::vector<HlEdgeEntry*>();

                    currHlNodeEntry = new HlNodeEntry();
                    currHlNodeEntry->setNodeId(currSourceNodeId);
                    currHlNodeEntry->setRankValue(defaultRank);
                    currHlNodeEntry->setEdgesVector(currSourceEdgesVector);

                    nodesEdgesVector[currSourceIndex] = currHlNodeEntry;
                }
            }


            //get targetEdgesVector for reverse edges

            //check whether current TargetNode still exists
            // as sourceNode due to reverse-edges or forward-edges
            HlNodeEntry* currTargetHlNodeEntry =
             nodesEdgesVector[currTargetIndex];


            //if the targetNodeId still exists
            // we just edit the existing edgeEntry
            if(currTargetHlNodeEntry)
            {
                #ifdef USEDEBUG
                LogDebug("target node Id (" << currTargetNodeId <<
                 ") still exists" << endl);
                #endif

                currTargetEdgesVector = currTargetHlNodeEntry->getEdgesVector();
            }else{
                #ifdef USEDEBUG
                LogDebug("target node Id (" << currTargetNodeId <<
                 ") does not exist" << endl);
                #endif

                //we need a new one
                currTargetEdgesVector = new std::vector<HlEdgeEntry*>();

                currTargetHlNodeEntry = new HlNodeEntry();
                currTargetHlNodeEntry->setNodeId(currTargetNodeId);
                currTargetHlNodeEntry->setRankValue(defaultRank);
                currTargetHlNodeEntry->setEdgesVector(currTargetEdgesVector);

                nodesEdgesVector[currTargetIndex] = currTargetHlNodeEntry;
            }

            #ifdef USEDEBUG
            LogDebug("add forward edge (" << currSourceNodeId << ", "
             << currTargetNodeId << ")" << endl);
            #endif
            //check whether the targetNodeId still exists
            // due to reverseEdges
            HlEdgeEntry* currSourceEdgeEntry =
             hlGetEdgeFromVector(currSourceEdgesVector, currTargetIndex);

             //create new if does not exist
            if(!currSourceEdgeEntry)
            {
                currSourceEdgeEntry = new HlEdgeEntry();

                //add current forward edge (u, v)
                currSourceEdgeEntry->setTargetIndex(currTargetIndex);
                currSourceEdgeEntry->setWeightForward(
                    currEdgeWeight->GetRealval());
                currSourceEdgeEntry->setIsForward(true);
                currSourceEdgesVector->push_back(currSourceEdgeEntry);
            }else{
                //else just edit fields
                currSourceEdgeEntry->setWeightForward(
                    currEdgeWeight->GetRealval());
                currSourceEdgeEntry->setIsForward(true);
            }


            #ifdef USEDEBUG
            LogDebug("add reverse edge (" << currTargetNodeId << ", "
             << currSourceNodeId << ")" << endl);
            #endif
            //check whether the sourceNodeId still exists
            // due to forwardEdges
            HlEdgeEntry* currTargetEdgeEntry =
             hlGetEdgeFromVector(currTargetEdgesVector, currSourceIndex);

             //create new if does not exist
            if(!currTargetEdgeEntry)
            {
                currTargetEdgeEntry = new HlEdgeEntry();

                //add the same edge as reverse edge (v, u)
                currTargetEdgeEntry->setTargetIndex(currSourceIndex);
                currTargetEdgeEntry->setWeightReverse(
                    currEdgeWeight->GetRealval());
                currTargetEdgeEntry->setIsReverse(true);
                currTargetEdgesVector->push_back(currTargetEdgeEntry);
            }else{
                //else just edit fields
                currTargetEdgeEntry->setWeightReverse(
                    currEdgeWeight->GetRealval());
                currTargetEdgeEntry->setIsReverse(true);
            }


            //get next Tuple from Orel
            currEdge->DeleteIfAllowed();
            currEdge = 0;
            currEdge = edgesOrelIter->GetNextTuple();
        }

        delete edgesOrelIter;


        #ifdef USEDEBUG
        LogDebug("finish hlTransformOrelToHlGraph" << endl);
        #endif
        return true;
    }

    /*
     * Iterates through the given vector and compares each element
     *  with the given targetNodeId.
     * If they match it returns the pointer to the current HlEdgeEntry-element.
     *
     * @param edgeVector the Vector to be searched in
     * @param targetNodeIndex the index positionof the node to be searched for
     * @return the pointer to the desired object if found, null else
    */
    HlEdgeEntry* hlGetEdgeFromVector(std::vector<HlEdgeEntry*>* edgeVector,
     int targetNodeIndex) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlGetEdgeFromVector" << endl);
        #endif

        HlEdgeEntry* resultEdgeEntry = 0;
        for (std::vector<HlEdgeEntry*>::iterator it =
         edgeVector->begin(); it != edgeVector->end(); ++it)
        {
            HlEdgeEntry* currEdgeEntry = *it;
            #ifdef USEDEBUG
            LogDebug("next edge from vector: " <<
             currEdgeEntry->getTargetIndex() << endl);
            #endif
            if(currEdgeEntry->getTargetIndex() == targetNodeIndex)
            {
                #ifdef USEDEBUG
                LogDebug("match" << endl);
                #endif
                resultEdgeEntry = currEdgeEntry;
                break;
            }
        }

        #ifdef USEDEBUG
        LogDebug("finish hlGetEdgeFromVector" << endl);
        #endif

        return resultEdgeEntry;
    }


    /*
     * Calculates the priority for the given sourceNodeId.
     * Does ignor edges not existing within priority multimap
     *  since these nodes still have been contracted.
     *
     * @param currNodeEntry the current nodeEntry to calculate its priority
     * @param nodesEdgesVector by reference the whole adjacency-list
     * @param initPriorityQueue indicated whethere the rankedMultimap is
     *          being built up right now
     * @param currCalcFunction defines which rank-calculation function
     *          shall be used
     * @param currTmpCounter only used for sequential rank calculation
     * @return the calculated rank
    */
    double hlCalcPriorityOfHlNodeEntry(HlNodeEntry* currNodeEntry,
     std::vector<HlNodeEntry*> &nodesEdgesVector, bool initPriorityQueue,
     int currCalcFunction, int currTmpCounter) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlCalcPriorityOfHlNodeEntry" << endl);
        #endif

        if(currCalcFunction == 0){
            //EdgeDifference, processed afterwards
        }else if(currCalcFunction == 1){
            //return low enough number such that the recalculated rank
            // is still the nex ont
            if(!initPriorityQueue)
            {
                return -10;
            }
            //Random, return directly
            time_t t;

            time(&t);
            srand((unsigned int)t); /* Zufallsgenerator initialisieren*/

            return rand();
        }else if(currCalcFunction == 2){
            //return low enough number such that the recalculated rank
            // is still the nex ont
            if(!initPriorityQueue)
            {
                return -10;
            }
            //sequence, return next
            return currTmpCounter;
        }

        double calculatedPriority = 0;

        int inOutEdgesCount = 0;
        int inEdgesCount = 0;
        int outEdgesCount = 0;

        std::vector<HlEdgeEntry*>* edgeVector = currNodeEntry->getEdgesVector();

        for (std::vector<HlEdgeEntry*>::iterator it =
         edgeVector->begin(); it != edgeVector->end(); ++it)
        {
            HlEdgeEntry* currEdgeEntry = *it;

            int currTargetIndex = currEdgeEntry->getTargetIndex();
            HlNodeEntry* currTargetNodeEntry =
             nodesEdgesVector[currTargetIndex];

            #ifdef USEDEBUG
            LogDebug("next edge from vector: (" <<
             currNodeEntry->getNodeId() << ", " <<
             currTargetNodeEntry->getNodeId() << ")" << endl);
            #endif


            //check whether target node still has been contracted
            //skip if yes, proceed else
            //still contracted nodes do have a rank > -1
            //also proceed if given flag initPriorityQueue = true
            // which means that the priority multimap is just being built up
            // right now and we will consider all nodes
            int currTargetRank = -1;
            if(!initPriorityQueue)
            {
                currTargetRank = currTargetNodeEntry->getRankValue();
            }


            if(currTargetRank == -1)
            {
                //node not contracted yet, proceed

                if(currEdgeEntry->getIsReverse() &&
                 currEdgeEntry->getIsForward())
                {
                    //incoming and outgoing edge
                    inOutEdgesCount++;

                }
                else if(currEdgeEntry->getIsReverse())
                {
                    //incoming edge oneway
                    inEdgesCount++;
                }
                else if (currEdgeEntry->getIsForward())
                {
                    //outgoing edge oneway
                    outEdgesCount++;
                }
            }
        }


        //do calcutation

        //calculate edge dffierence

        //total number of incoming and outgoing edges
        int totalInOutEdges = inOutEdgesCount * 2
         + inEdgesCount + outEdgesCount;

        //total number of shortcuts to be created
        //do not count nodes being source and target at the same time
        //e.g. if theres an incoming edge from u to v and also there
        // is an edge from v to u we do not count an shortcut from u to u
        // which would be nonsense
        //Thats why we subtract inOutEdgesCount.
        int totalShortcutsToCreate =
         (inOutEdgesCount + inEdgesCount) * (inOutEdgesCount + outEdgesCount)
         - inOutEdgesCount;

        int edgeDifference = totalShortcutsToCreate - totalInOutEdges;
         #ifdef USEDEBUG
        LogDebug("ED: " << edgeDifference << " = shortcuts: " <<
          totalShortcutsToCreate << " - inoutEdges: " << totalInOutEdges
           << endl);
        #endif


        //multiply by -1 such that we get a min-priority queue
        calculatedPriority = (double) edgeDifference;

        #ifdef USEDEBUG
        LogDebug("finish hlCalcPriorityOfHlNodeEntry" << endl);
        #endif

        return calculatedPriority;
    }


    /*
     * type definitions and
     * comparable function for using priority_queue
     *  within nodeContraction
     * Format is: edgeWeightToSource, targetNodeIndex
    */
    typedef std::pair<double, int> QueuePairType;

    struct CompareQueueEntry :
     public std::binary_function<QueuePairType, QueuePairType, bool>
    {
        bool operator()
         (const QueuePairType firstQueuePair,
          const QueuePairType secondQueuePair) const
        {
            return firstQueuePair.first > secondQueuePair.first;
        }
    };

    typedef std::priority_queue<QueuePairType, vector<QueuePairType>,
     CompareQueueEntry> PriorityQueueType;


    /*
     * type definitions and comparable function
     *  for using priority_queue
     *  within hHopForwardSearch
     * Format is: edgeWeightToSource, targetNodeIndex, targetHHop
    */
    typedef std::pair<double, std::pair<int, int>> QueuePairTypeForwardSearch;

    struct CompareQueueEntryForwardSearch :
     public std::binary_function<QueuePairTypeForwardSearch,
      QueuePairTypeForwardSearch, bool>
    {
        bool operator()
         (const QueuePairTypeForwardSearch firstQueuePair,
          const QueuePairTypeForwardSearch secondQueuePair) const
        {
            return firstQueuePair.first > secondQueuePair.first;
        }
    };

    typedef std::priority_queue<QueuePairTypeForwardSearch,
     vector<QueuePairTypeForwardSearch>,
     CompareQueueEntryForwardSearch> PriorityQueueTypeForwardSearch;


    /*
     * Iterates through the given graph and creates a priority for every node.
     * Inserts these priorities into the given multimap.
     *
     * @param nodesEdgesVector by reference the adjacency-list to fill
     * @param priorityQueue by reference priorityQueue to fill
     * @param currCalcFunction the rank-calcfuntion to be used
     * @return true since all modifications are done within the given parameters
    */
    bool hlInitPriorityQueueOfHlGraph(
     std::vector<HlNodeEntry*> &nodesEdgesVector,
     PriorityQueueType &priorityQueue, int currCalcFunction) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlInitPriorityQueueOfHlGraph" << endl);
        #endif

        //iterate over all nodes and initial calc their priority
        #ifdef USEDEBUG
        LogDebug("init priority" << endl);
        #endif

        //used for sequential rank calculation if used
        int tmpCalcCounter = 0;

        for (uint i = 0; i < nodesEdgesVector.size(); i++)
        {
            HlNodeEntry* currNodeEntry = nodesEdgesVector[i];

            //calc priority of current node
            double currPriority = 0.0;
            bool initPriorityQueue = true;
            currPriority = hlCalcPriorityOfHlNodeEntry(currNodeEntry,
             nodesEdgesVector, initPriorityQueue, currCalcFunction,
             tmpCalcCounter++);

            //store priority in queue
            priorityQueue.push(std::make_pair(currPriority, i));
        }

        #ifdef USEDEBUG
        LogDebug("finish hlInitPriorityQueueOfHlGraph" << endl);
        #endif

        return true;
    }


    /*
     * Iteratively does Contraction for every element within the given queue.
     * Recalculates the priority of every node chosen next from queue
     *  for better contaction.
     * Stores a sequenced rank for every node in order of its contraction.
     *
     * @param nodesEdgesVector by reference the adjacency-list
     * @param priorityQueue by reference the priorityQueue to work off
     * @param hHopSize the size how depth hHop searches shall be processed
     * @param skipContractionRemainingCount the size after how many remaining
     *         elements from contraction recalculation of priority shall be
     *         stopped and elements are just contracted in the order so far
     * @param currCalcFunction defines which rank-calculation function shall
     *         shall be used
     * @return true since all modifications are done within the given parameters
    */
    bool hlDoContractionOfHlGraph(std::vector<HlNodeEntry*> &nodesEdgesVector,
     PriorityQueueType &priorityQueue,
      int hHopSize, int skipContractionRemainingCount,
       int currCalcFunction) const
    {
        #ifdef USEINFO
        LogInfo("start hlDoContractionOfHlGraph" << endl);
        #endif

        int newRankSequence = 0;
        size_t skipRecalcRank = (size_t) skipContractionRemainingCount;

        int progressCount = 0;
        #ifdef USEPERF
        bool perfBool = false;

        double perfTime = 0.0;
        double perfTime2 = 0.0;
        double perfTime3 = 0.0;
        double perfTime4 = 0.0;
        #endif

        //iterate while queue is not empty
        while(!priorityQueue.empty())
        {
            progressCount++;
            #ifdef USEPERF

            if(progressCount % getProgressInterval() == 0)
            {
                perfBool = true;
            }

            if(perfBool)
            {
                perfTime = getCurrentTimeInMs();
            }
            #endif

            //get first element of map
            QueuePairType currQueuePair = priorityQueue.top();
            int currPriorityContraction = currQueuePair.first;
            int currNodeIndex = currQueuePair.second;
            HlNodeEntry* currNodeEntry = nodesEdgesVector[currNodeIndex];

            //remove node from map its not used any more
            //but we may reinsert it when priority has changed
            priorityQueue.pop();


            #ifdef USEINFO
            if(progressCount % getProgressInterval() == 0)
            {
            int currNodeIdContraction = currNodeEntry->getNodeId();
            LogInfo("next top element is: nodeId: " << currNodeIdContraction
                << " priority: " << currPriorityContraction
                << " elements left:" << priorityQueue.size() - 1 << endl);
            }
            #endif

            //recalculate the priority of the first element
            //Only recalculate if we are not processing the last X Nodes
            // where x depends on the soze of the network an is
            // n = 10.000
            //there probably is no lower rank than -2
            // (in case a death end node is contracted)
            double recalculatedPriority = -10.0;

            if(priorityQueue.size() >= skipRecalcRank)
            {
                bool initPriorityQueue = false;
                recalculatedPriority = hlCalcPriorityOfHlNodeEntry(
                    currNodeEntry, nodesEdgesVector, initPriorityQueue,
                    currCalcFunction, -1
                );
            }


            //check whether the priority has increased
            if(recalculatedPriority > currPriorityContraction)
            {
                #ifdef USEDEBUG
                LogDebug("reinserting, priority has increased from "
                    << currPriorityContraction
                    << " to: " << recalculatedPriority << endl);
                #endif
                //if yes we need to reinsert this element with its
                // recalculated priority
                //and repeat getting the new top element
                priorityQueue.push(
                 std::make_pair(recalculatedPriority, currNodeIndex));
                continue;
            }


            //store rank in hlGraph for later use
            //for uniqueness we use a novel increasing sequence for that
            currNodeEntry->setRankValue(newRankSequence++);




            #ifdef USEPERF
            if(perfBool)
            {
                perfTime2 = getCurrentTimeInMs();
            }
            #endif


            //contract
            hlContractNodeOfHlGraph(nodesEdgesVector, currNodeEntry,
             currNodeIndex, hHopSize);


            #ifdef USEPERF
            if(perfBool)
            {
                perfTime3 = getCurrentTimeInMs();
                LogPerf("perf: duration of contraction itself in: (ms) "
                 << fixed << (perfTime3 - perfTime2) << endl);
            }
            #endif

            if(priorityQueue.size() <= skipRecalcRank)
            {
                //recalculate priorities of neighbours of contracted node
                hlRecalculateNeighboursInHlGraph(nodesEdgesVector,
                 currNodeEntry);
            }


            #ifdef USEPERF
            if(perfBool)
            {
                perfTime4 = getCurrentTimeInMs();

                LogPerf(
                 "perf: duration of receiving and contracting a node: (ms) "
                 << fixed << (perfTime4 - perfTime) << endl);

                perfTime = 0.0;
                perfTime2 = 0.0;
                perfTime3 = 0.0;
                perfTime4 = 0.0;

                perfBool = false;
            }


            #endif
        }


        #ifdef USEINFO
        LogInfo("finish hlDoContractionOfHlGraph" << endl);
        #endif

        return true;
    }


    /*
     * Contracts a specific node.
     * Contraction mean that shortcuts will be created
     *  and added to the existing graph structure.
     * For every incoming node u and every outgoing node w there will
     *  be a shortcut created with the given nodeIdV as parent.
     * Nodes or edges are not deleted here because still contracted edges
     *  and nodes are identified by not finding them within the ranks-queue.
     * Does ignor edges not existing within ranked multimap
     *  since these nodes still have been contracted.
     *
     * @param nodesEdgesVector by reference the adjacency-list
     * @param currNodeEntryV the current node v to be contracted.
     * @param nodeIndexV the index position of node v within hlGraph.
     * @param hHopSize the size how depth hHop searches shall be processed
     * @return true since all modifications are done within the given parameters
    */
    bool hlContractNodeOfHlGraph(std::vector<HlNodeEntry*> &nodesEdgesVector,
     HlNodeEntry* currNodeEntryV, int nodeIndexV, int hHopSize) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlContractNodeOfHlGraph" << endl);
        #endif


        //get EdgesTuple of curent node to be contracted
        std::vector<HlEdgeEntry*>* currEdgesV =
         currNodeEntryV->getEdgesVector();

        //save all incoming edges for later h-hop forward search
        std::vector<HlEdgeEntry*> incomingEdgesUV;

        //store results of one hop reverse search
        std::map<int, std::vector<std::pair<int, double>>>
         reverseSearchResultsXW;
        std::map<int, double> distancesVW;

        //iterate over all incoming and outgoing edges
        for (std::vector<HlEdgeEntry*>::iterator currEdgesVIter =
         currEdgesV->begin() ; currEdgesVIter != currEdgesV->end();
         ++currEdgesVIter)
        {
            HlEdgeEntry* currEdgeEntryV = *currEdgesVIter;

            //check whether edge is a reverse/incoming edge (u, v)
            // remember it then for later forward search
            // do one-hop reverse search if its a forward/outgoing edge
            if(currEdgeEntryV->getIsReverse())
            {
                incomingEdgesUV.push_back(currEdgeEntryV);
            }

            if(currEdgeEntryV->getIsForward())
            {

                //get edgeVector of w
                int nodeIndexW = currEdgeEntryV->getTargetIndex();
                HlNodeEntry* currNodeEntryW = nodesEdgesVector[nodeIndexW];

                int rankW = currNodeEntryW->getRankValue();
                std::vector<HlEdgeEntry*>* edgesVectorW =
                 currNodeEntryW->getEdgesVector();


                //skip if targetNode w still has been contracted
                // they are stil contracted if they do have a rank != -1
                if(rankW != -1)
                {
                    continue;
                }


                //insert for lookups while hHop Forward search
                distancesVW.insert(std::make_pair(
                  currEdgeEntryV->getTargetIndex(),
                  currEdgeEntryV->getWeightForward()
                 ));

                #ifdef USEDEBUG
                int nodeIdW = currNodeEntryW->getNodeId();
                LogDebug("next node w: " << nodeIdW << " with index: "
                 << nodeIndexW << " with rank: " << rankW << endl);
                #endif


                // store the additional information that this is
                // an upward-edge (for easier deletion later on
                // creating Up- and Downwardgraphs
                currEdgeEntryV->setIsUpwardOriginal(true);


                //iterate over all incoming edges (x, w) to store them as
                // one hop reverse search
                for (std::vector<HlEdgeEntry*>::iterator
                 edgesVectorWIter = edgesVectorW->begin();
                 edgesVectorWIter != edgesVectorW->end();
                 ++edgesVectorWIter)
                {
                    HlEdgeEntry* currEdgeEntryXW = *edgesVectorWIter;

                    int nodeIndexX = currEdgeEntryXW->getTargetIndex();
                    HlNodeEntry* currNodeEntryX = nodesEdgesVector[nodeIndexX];

                    //skip if is v itself, we still stored its distance
                    // also skip if is no incoming edge to t
                    // skip still contracted nodes
                    if(!currEdgeEntryXW->getIsReverse()
                     || nodeIndexX == nodeIndexV
                     || currNodeEntryX->getRankValue() != -1)
                    {
                        continue;
                    }

                    //store x with distance to current w
                    double currDistXW = currEdgeEntryXW->getWeightReverse();

                    //creates a new entry if nodeIndexX it did not exist
                    reverseSearchResultsXW[nodeIndexX].push_back(
                     std::make_pair(nodeIndexW, currDistXW));

                }
            } //finish reverse search of current node
        }//finish iteration over all adjacent nodes


        //iterate over all remembered incoming nodes
        //iterate over all incoming edges (= reverse edges) (u, v)
        for (std::vector<HlEdgeEntry*>::iterator
         incomingEdgesUVIter = incomingEdgesUV.begin();
         incomingEdgesUVIter != incomingEdgesUV.end();
         ++incomingEdgesUVIter)
        {
            HlEdgeEntry* currEdgeEntryIncomingUV = *incomingEdgesUVIter;

            int nodeIndexU = currEdgeEntryIncomingUV->getTargetIndex();


            //get edgeVector of u
            HlNodeEntry* currNodeEntryU = nodesEdgesVector[nodeIndexU];
            int rankU = currNodeEntryU->getRankValue();
            std::vector<HlEdgeEntry*>* edgesVectorU =
             currNodeEntryU->getEdgesVector();


            //skip if sourceNode u still has been contracted
            // they are stil contracted if they do have a rank != -1
            if(rankU != -1)
            {
                continue;
            }


            // store the additional information that this is
            // an reverse-upward-edge (for easier deletion later on
            // creating Up- and Downwardgraphs
            // we just check if this edge not yet still has been flagged
            //  with this information
            // such that we only set this flag here if the current edge
            // actually is an oneway on the graph
            if(!currEdgeEntryIncomingUV->getIsUpwardOriginal())
            {
                currEdgeEntryIncomingUV->setIsUpwardOriginal(true);
            }


            #ifdef USEDEBUG
            int nodeIdU = currNodeEntryU->getNodeId();
            LogDebug("next node u: " << nodeIdU << " with rank: "
             << rankU << endl);
            #endif

            //copy distancesVW for make shortcut creation easier
            std::map<int, double> distancesVWCopy = distancesVW;


            //directly erase current w == u from distancesVWCopy
            std::map<int, double>::iterator distCopyCurrUIter =
             distancesVWCopy.find(nodeIndexU);

            if(distCopyCurrUIter != distancesVWCopy.end())
            {
                distancesVWCopy.erase(distCopyCurrUIter);
            }


            double currDistUV = currEdgeEntryIncomingUV->getWeightReverse();

            //do h-Hop forward search
            hlDoHHopForwardSearchInHlGraph(nodesEdgesVector,
             reverseSearchResultsXW, distancesVWCopy, edgesVectorU,
             nodeIndexU, nodeIndexV, currDistUV, hHopSize);


            //iterate over all remaining elements from distancesVWCopy
            //create shortcut for every pair with current sourceNode
            for (std::map<int, double>::iterator
             distancesVWCopyIter = distancesVWCopy.begin();
             distancesVWCopyIter != distancesVWCopy.end();
             ++distancesVWCopyIter)
            {
                int nodeIndexWShortcut = (*distancesVWCopyIter).first;
                double currDistVWShortcut = (*distancesVWCopyIter).second;

                //skip if u == w
                if(nodeIndexU == nodeIndexWShortcut)
                {
                    continue;
                }

                HlNodeEntry* currNodeEntryWShortcut =
                 nodesEdgesVector[nodeIndexWShortcut];
                std::vector<HlEdgeEntry*>* edgesVectorWShortcut =
                 currNodeEntryWShortcut->getEdgesVector();

                double currShortcutWeight = currDistUV + currDistVWShortcut;

                //check whether an edge (u, w) still exists
                // for creating a forward edge
                HlEdgeEntry* currShortcutUW =
                 hlGetEdgeFromVector(edgesVectorU, nodeIndexWShortcut);

                //check whether an edge (w, u) still exists
                // for creating a reverse edge
                HlEdgeEntry* currShortcutWU =
                 hlGetEdgeFromVector(edgesVectorWShortcut, nodeIndexU);


                //merge UW if edge still exists, create new else
                if(currShortcutUW)
                {
                    #ifdef USEDEBUG
                    int nodeIdU = currNodeEntryU->getNodeId();
                    int nodeIdW = currNodeEntryWShortcut->getNodeId();
                    LogDebug("edge (u, w) (" << nodeIdU << ", " << nodeIdW <<
                    ") still exists going to merge" << endl);
                    #endif

                    currShortcutUW->setIsForward(true);
                    currShortcutUW->setParentIndexForward(nodeIndexV);
                    currShortcutUW->setWeightForward(currShortcutWeight);
                }else
                {
                    #ifdef USEDEBUG
                    int nodeIdU = currNodeEntryU->getNodeId();
                    HlNodeEntry* currNodeEntryWShortcut =
                     nodesEdgesVector[nodeIndexWShortcut];
                    int nodeIdW = currNodeEntryWShortcut->getNodeId();
                    LogDebug("edge (u, w) (" << nodeIdU << ", " << nodeIdW <<
                      ") does not exist, create new" << endl);
                    #endif

                    //create new and append to edgesVector
                    currShortcutUW = new HlEdgeEntry();
                    currShortcutUW->setIsForward(true);
                    currShortcutUW->setParentIndexForward(nodeIndexV);
                    currShortcutUW->setTargetIndex(nodeIndexWShortcut);
                    currShortcutUW->setWeightForward(currShortcutWeight);

                    edgesVectorU->push_back(currShortcutUW);
                }

                //merge WU if existed crete new else
                if(currShortcutWU)
                {
                    #ifdef USEDEBUG
                    int nodeIdU = currNodeEntryU->getNodeId();
                    int nodeIdW = currNodeEntryWShortcut->getNodeId();
                    LogDebug("edge (w, u) (" << nodeIdW << ", " << nodeIdU <<
                      ") still exists going to merge" << endl);
                    #endif

                    currShortcutWU->setIsReverse(true);
                    currShortcutWU->setParentIndexReverse(nodeIndexV);
                    currShortcutWU->setWeightReverse(currShortcutWeight);
                }else
                {
                    #ifdef USEDEBUG
                    int nodeIdU = currNodeEntryU->getNodeId();
                    int nodeIdW = currNodeEntryWShortcut->getNodeId();
                    LogDebug("edge (w, u) (" << nodeIdW << ", " << nodeIdU <<
                    ") does not exist, create new" << endl);
                    #endif

                    //create new and append to edgesVector
                    currShortcutWU = new HlEdgeEntry();
                    currShortcutWU->setIsReverse(true);
                    currShortcutWU->setParentIndexReverse(nodeIndexV);
                    currShortcutWU->setTargetIndex(nodeIndexU);
                    currShortcutWU->setWeightReverse(currShortcutWeight);

                    edgesVectorWShortcut->push_back(currShortcutWU);
                }
            }


        } // finish iterating over all incoming nodes


        #ifdef USEDEBUG
        LogDebug("finish hlContractNodeOfHlGraph" << endl);
        #endif

        return true;
    }


    /*
     * Do H-Hop forward search
     *
     * @param nodesEdgesVector by reference the adjacency-list
     * @param reverseSearchResultsXW by reference the result of reverse search.
     * @param distancesVWCopy by reference the targetNodes with distance to v.
     * @param edgesVectorU the edgesVector of current u.
     * @param nodeIndexU the id of current u.
     * @param nodeIndexV the id of current v to be contracted.
     * @param distUV the distance of current u to v.
     * @param givenHHop the max hop-size.
     * @return true since all modifications are done within the given parameters
    */
    bool hlDoHHopForwardSearchInHlGraph(
     std::vector<HlNodeEntry*> &nodesEdgesVector,
     std::map<int, std::vector<std::pair<int, double>>> reverseSearchResultsXW,
     std::map<int, double> &distancesVWCopy,
     std::vector<HlEdgeEntry*>* edgesVectorU, int nodeIndexU, int nodeIndexV,
      double distUV, int givenHHop) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlDoHHopForwardSearchInHlGraph" << endl);
        #endif


        double distUX = 0.0;

        //at first check whether curr u is one element of X
        std::map<int, std::vector<std::pair<int, double>>>::iterator
         reverseSearchResultsIter = reverseSearchResultsXW.find(nodeIndexU);



        if(reverseSearchResultsIter != reverseSearchResultsXW.end())
        {
            std::vector<std::pair<int, double>> currVectorXW =
             (*reverseSearchResultsIter).second;

            //X found check for witness and may remove potential shortcuts
            hlCheckForWitnessInHlGraph(currVectorXW, distancesVWCopy,
             distUX, distUV, nodeIndexU);
        }


        //only proceed if givenHHop was chosen correctly
        if(givenHHop < 1)
        {
            return true;
        }

        //do forward search until givenHHop is reached
        //init queue
        PriorityQueueTypeForwardSearch forwardQueue;
        std::set<int> stillVisitedNodes;


        #ifdef USEDEBUG
        LogDebug("initiate hHop forward search" << endl);
        #endif


        int currHHop = 1;
        stillVisitedNodes.insert(nodeIndexU);


        //skip if there are no more witnesses to find
        if(!distancesVWCopy.empty())
        {
            //iterate over all k of current u
            //to initiate hHop forward search
            for (std::vector<HlEdgeEntry*>::iterator
             edgesVectorUIter = edgesVectorU->begin();
             edgesVectorUIter != edgesVectorU->end();
             ++edgesVectorUIter)
             {
                //get data
                HlEdgeEntry* currEdgeEntryUK = *edgesVectorUIter;
                int targetIndexK = currEdgeEntryUK->getTargetIndex();

                HlNodeEntry* currNodeEntryK = nodesEdgesVector[targetIndexK];

                //skip if is no forward edge or if k = v
                // or if target is still contracted
                // or if is still visited
                if(!currEdgeEntryUK->getIsForward()
                 || targetIndexK == nodeIndexV
                 || currNodeEntryK->getRankValue() != -1
                 || stillVisitedNodes.find(targetIndexK)
                    != stillVisitedNodes.end()
                )
                {
                    continue;
                }


                forwardQueue.push(std::make_pair(
                 currEdgeEntryUK->getWeightForward(),
                 std::make_pair(targetIndexK, currHHop)
                ));
            }
        }

        #ifdef USEDEBUG
        LogDebug("do hHop forward search" << endl);
        #endif


        while(!forwardQueue.empty())
        {
            QueuePairTypeForwardSearch currQueuePair = forwardQueue.top();
            forwardQueue.pop();

            double currDistToSourceUK = currQueuePair.first;
            std::pair<int, int> currTupleQueue = currQueuePair.second;

            int currNodeIndexK = currTupleQueue.first;
            int currHHopK = currTupleQueue.second;


            stillVisitedNodes.insert(currNodeIndexK);

            HlNodeEntry* currNodeEntryK = nodesEdgesVector[currNodeIndexK];

            #ifdef USEDEBUG
            int currNodeIdK = currNodeEntryK->getNodeId();
            LogDebug("next node from queue: " << currNodeIdK
             << " with index: " << currNodeIndexK
             << " with hHop: " << currHHopK << endl);
            #endif

            //check for match with an x
            //reuse iterator
            reverseSearchResultsIter =
             reverseSearchResultsXW.find(currNodeIndexK);

            if(reverseSearchResultsIter != reverseSearchResultsXW.end())
            {
                std::vector<std::pair<int, double>> currVectorXW =
                 (*reverseSearchResultsIter).second;


                //X found check for witness and may remove potential shortcuts
                hlCheckForWitnessInHlGraph(currVectorXW, distancesVWCopy,
                 currDistToSourceUK, distUV, nodeIndexU);

                //if there are no more nodes w to find a witness path for
                //we can abort the search
                if(distancesVWCopy.empty())
                {
                    break;
                }
            }

            //only proceed if we did not exceed givenHHop
            if(currHHopK >= givenHHop)
            {
                continue;
            }



            #ifdef USEDEBUG
            LogDebug("scan new vertices" << endl);
            #endif

            //get edgeVector of current k
            std::vector<HlEdgeEntry*>* edgesVectorK =
             currNodeEntryK->getEdgesVector();

            //iterate over all j of current k with edge (k, j)
            for (std::vector<HlEdgeEntry*>::iterator
             edgesVectorKIter = edgesVectorK->begin();
             edgesVectorKIter != edgesVectorK->end();
             ++edgesVectorKIter)
             {
                //get data
                HlEdgeEntry* currEdgeEntryKJ = *edgesVectorKIter;
                int currTargetIndexJ = currEdgeEntryKJ->getTargetIndex();

                HlNodeEntry* currNodeEntryJ =
                 nodesEdgesVector[currTargetIndexJ];

                //skip if is not forward edge or if k = v
                // or if target is still contracted
                if(!currEdgeEntryKJ->getIsForward()
                 || currTargetIndexJ == nodeIndexV
                 || currNodeEntryJ->getRankValue() != -1
                 || stillVisitedNodes.find(currTargetIndexJ)
                    != stillVisitedNodes.end()
                )
                {
                    continue;
                }

                //may inserts target nodes which still have been visited
                forwardQueue.push(std::make_pair(
                  currEdgeEntryKJ->getWeightForward() + currDistToSourceUK,
                  std::make_pair(currTargetIndexJ,
                   currHHopK + 1)
                 ));
            }
        }

        #ifdef USEDEBUG
        LogDebug("finish hlDoHHopForwardSearchInHlGraph" << endl);
        #endif

        return true;
    }


    /*
     * Check for Witness during H-Hop forward search
     *  and delete w from distancesVWCopy if witness has been found.
     *
     * @param currVectorXW by reference the current x-Array
     * @param distancesVWCopy by reference the targetNodes with distance to v.
     * @param distUX the current distance from u to x
     * @param distUV the current distance from u to v
     * @param currNodeIndexU the index of current node u
     * @return true since all modifications are done within the given parameters
    */
    bool hlCheckForWitnessInHlGraph(
     std::vector<std::pair<int, double>> &currVectorXW,
     std::map<int, double> &distancesVWCopy,
     double distUX, double distUV, int currNodeIndexU) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlCheckForWitnessInHlGraph" << endl);
        #endif

        //iterate over all w of current x
        for (std::vector<std::pair<int, double>>::iterator
         currVectorXWIter = currVectorXW.begin();
         currVectorXWIter != currVectorXW.end();
         ++currVectorXWIter)
         {
            //get data
            int currNodeIndexW = (*currVectorXWIter).first;
            double currDistXW = (*currVectorXWIter).second;

            //skip if u == w
            if(currNodeIndexU == currNodeIndexW)
            {
                continue;
            }

            //calc potential shortcut length
            std::map<int, double>::iterator distVWIter =
             distancesVWCopy.find(currNodeIndexW);
            if(distVWIter == distancesVWCopy.end())
            {
                //skip if current w still as been erased
                // from current on hop reverse search
                continue;
            }

            double currDistVW = (*distVWIter).second;
            double potentialShortcutLength = distUV + currDistVW;

            #ifdef USEDEBUG
            LogDebug("check for witness for targetIndex: " << currNodeIndexW
             << " with shortcutLength: " << potentialShortcutLength
             << " and witness length: " << distUX + currDistXW << endl);
            #endif

            //check whether witness can be found
            if(potentialShortcutLength >= distUX + currDistXW)
            {
                #ifdef USEDEBUG
                LogDebug("witness found, erase index: "
                 << currNodeIndexW << endl);
                #endif

                //witness found, remove potential shortcut
                distancesVWCopy.erase(distVWIter);
            }

            //break if there are no more witnesses to find
            if(distancesVWCopy.empty())
            {
                break;
            }
         }


        #ifdef USEDEBUG
        LogDebug("finish hlCheckForWitnessInHlGraph" << endl);
        #endif

        return true;
    }


    /*
     * Iterates through all incoming and outgoing neighbours
     *  of the current contracted node and recalculates their ranks.
     * Does ignor edges not existing within ranked multimap
     *  since these nodes still have been contracted.
     * TODO: not implemented yet.
     * TODO: parameterize the secondo operator with flags to enable or
     *          disable this functionality to choose between better
     *          contraction and faster processing
     * TODO: ignor still visited nodes and edges
     *
     * @param nodesEdgesVector by reference the adjacency-list
     * @param currNodeEntry the current contracted node.
     * @return true since all modifications are done within the given parameters
    */
    bool hlRecalculateNeighboursInHlGraph(
     std::vector<HlNodeEntry*> &nodesEdgesVector,
     HlNodeEntry* currNodeEntry) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlRecalculateNeighboursInHlGraph" << endl);
        #endif


        #ifdef USEDEBUG
        LogDebug("finish hlRecalculateNeighboursInHlGraph" << endl);
        #endif

        return true;
    }


    /*
     * Creates the Upward- and Downwardgraph from the given HlGraph.
     * The Upward Graph only contains edges (u, v) leading to nodes v
     *  having a higher rank than u.
     * The Downward Graph only contains edges (u, v) leading to nodes v
     *  having a lower rank than u.
     * Because the HlGraph still contaings original edges and reverse-edges
     *  we can make use of this.
     * For searches we only need:
     *  - original (forward) edges leading to nodes with a higher rank
     *  - reverse (backward) edges leading to nodes with a higher rank
     *    (because we actually need forward edges leading to nodes with
     *     lower rank and invert them which in turn are reverse edges
     *     leading to nodes with higher rank)
     *
     * We still do have both but also we have edges leading to nodes with
     *  lower rank.
     * So this function just removes forward and backward-edges leading to
     *  nodes with lower rank.
     *
     * @param nodesEdgesVector by reference the adjacency-list
     * @param contractNodeTuple by reference the data-tuple
                of the current node contracted.
     * @param contractNodeId the id of the node to be contracted.
     * @return true since all modifications are done within the given parameters
    */
    bool hlCreateUpwardAndDownwardHlGraph(std::vector<HlNodeEntry*>
     &nodesEdgesVector) const
    {
        #ifdef USEINFO
        LogInfo("start hlCreateUpwardAndDownwardHlGraph" << endl);
        #endif


        #ifdef USEINFO
        size_t graphSize = nodesEdgesVector.size();
        int progress = 0;
        #endif

        //iterate over whole graph-map
        for (std::vector<HlNodeEntry*>::iterator
            nodesEdgesVectorIter = nodesEdgesVector.begin();
            nodesEdgesVectorIter != nodesEdgesVector.end();
            ++nodesEdgesVectorIter)
        {
            HlNodeEntry* currNodeEntry = *nodesEdgesVectorIter;

            std::vector<HlEdgeEntry*>* currNodeVector =
             currNodeEntry->getEdgesVector();

            #ifdef USEINFO
            progress++;
            if(progress % getProgressInterval() == 0)
            {
                int currSourceNodeId = currNodeEntry->getNodeId();
                int currRank = currNodeEntry->getRankValue();

                LogInfo("Current SourceNode: " << currSourceNodeId
                 << " with rank: " << currRank << " has #edges: "
                 << currNodeVector->size() << " nodes left: "
                 << (graphSize - progress) << endl);
            }
            #endif


            //iterate over all adjacent nodes and delete edges (u, v)
            // leading to nodes v having a lower rank than u
            for(size_t i = 0; i < currNodeVector->size();)
            {
                //dereference vector-pointer to acces via index
                HlEdgeEntry* currEdge = (*currNodeVector)[i];

                #ifdef USEDEBUG
                LogDebug("next edge with targetIndex: "
                 << currEdge->getTargetIndex() << endl);
                #endif

                if( !currEdge->getIsUpwardOriginal())
                {
                    #ifdef USEDEBUG
                    LogDebug("delete currEdge" << endl);
                    #endif
                    delete currEdge;

                    #ifdef USEDEBUG
                    LogDebug("swap currEdge" << endl);
                    #endif
                    std::swap((*currNodeVector)[i], currNodeVector->back());

                    #ifdef USEDEBUG
                    LogDebug("pop currEdge" << endl);
                    #endif
                    currNodeVector->pop_back();

                    //do not increase i because we swapped the last element
                    // to the position i and want to get it next
                }else{
                  ++i;
                }


            }

            #ifdef USEDEBUG
            LogDebug("after source: " << currSourceNodeId << " with rank: " <<
              currRank << " has #edges: " << currNodeVector->size() << endl);
            #endif
        }

        #ifdef USEINFO
        LogInfo("finish hlCreateUpwardAndDownwardHlGraph" << endl);
        #endif

        return true;
    }



    /*
     * Help Function to check whether an edge from hlGraph
     *  leads to a node with lower rank then the given Rank.
     *
     * @param nodesEdgesVector by reference the adjacency-list
     * @param currEdgeEntry the current edge to be checked
     * @param sourceNodeRank the rank of the sourceNode of the current edge
     *
     * @return true if the edge leads to a node with a lower rank, false else
    */
    bool hlIsDownwardsInHlGraph(std::vector<HlNodeEntry*> &nodesEdgesVector,
     HlEdgeEntry* currEdgeEntry, int sourceNodeRank) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlIsDownwardsInHlGraph" << endl);
        #endif

        bool retVal = false;

        int currTargetNodeIndex = currEdgeEntry->getTargetIndex();


        //get Rank of current node
        HlNodeEntry* currTargetNodeEntry =
         nodesEdgesVector[currTargetNodeIndex];
        int currTargetNodeRank = currTargetNodeEntry->getRankValue();

        #ifdef USEDEBUG
        LogDebug("sourceNodeRank: " << sourceNodeRank << " currTargetNodeRank: "
        << currTargetNodeRank << endl);
        #endif

        //remove edges if leads to a node with lower rank
        if(sourceNodeRank > currTargetNodeRank)
        {
            #ifdef USEDEBUG
        LogDebug("true" << endl);
        #endif
            retVal = true;
        }
        #ifdef USEDEBUG
        LogDebug("targetnodeindex: " << currTargetNodeIndex << " with rank: "
         << currTargetNodeRank
         << " isDownward: " << retVal << "(0 = false)"<< endl);
        #endif


        #ifdef USEDEBUG
        LogDebug("finish hlIsDownwardsInHlGraph" << endl);
        #endif

        return retVal;
    }



    /*
     * Gets an OrderedRelation representing an previous exported HlGraph.
     * Transforms it against to an HlGraph for further use.
     *
     * @param hlGraphOrel the orel to be transformed
     * @param nodesEdgesVector by reference the hlGraph to fill
     *
     * @return true since all modifications take place in the given parameters
    */
    bool hlTransformHlGraphOrelToHlGraph(OrderedRelation* hlGraphOrel,
     std::vector<HlNodeEntry*> &nodesEdgesVector) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlTransformHlGraphOrelToHlGraph" << endl);
        #endif

        GenericRelationIterator* hlGraphOrelIter = hlGraphOrel->MakeScan();

        //Get first Tuple
        Tuple* currNode = hlGraphOrelIter->GetNextTuple();

        HlNodeEntry* currNodeEntry = 0;
        std::vector<HlEdgeEntry*>* currEdgeVector = 0;
        int currSourceId = -1;
        //Iterate over all Tuples
        while(currNode)
        {
            CcInt* currEdgeSource = (CcInt*) currNode->GetAttribute(
                    HL_INDEX_OF_SOURCE_IN_HL_GRAPH);
            CcInt* currEdgeTarget = (CcInt*) currNode->GetAttribute(
                    HL_INDEX_OF_TARGET_IN_HL_GRAPH);
            CcInt* currEdgeSourceIndex = (CcInt*) currNode->GetAttribute(
                    HL_INDEX_OF_SOURCE_INDEX_IN_HL_GRAPH);
            CcInt* currEdgeTargetIndex = (CcInt*) currNode->GetAttribute(
                    HL_INDEX_OF_TARGET_INDEX_IN_HL_GRAPH);
            CcInt* currEdgeSourceRank = (CcInt*) currNode->GetAttribute(
                    HL_INDEX_OF_SOURCE_RANK_IN_HL_GRAPH);
            CcBool* currEdgeIsForward = (CcBool*) currNode->GetAttribute(
                    HL_INDEX_OF_IS_FORWARD_IN_HL_GRAPH);
            CcBool* currEdgeIsReverse = (CcBool*) currNode->GetAttribute(
                    HL_INDEX_OF_IS_REVERSE_IN_HL_GRAPH);
            CcInt* currEdgeParentForward = (CcInt*) currNode->GetAttribute(
                    HL_INDEX_OF_PARENT_INDEX_FORWARD_IN_HL_GRAPH);
            CcInt* currEdgeParentReverse = (CcInt*) currNode->GetAttribute(
                    HL_INDEX_OF_PARENT_INDEX_REVERSE_IN_HL_GRAPH);
            CcReal* currEdgeWeightForward = (CcReal*) currNode->GetAttribute(
                    HL_INDEX_OF_WEIGHT_FORWARD_IN_HL_GRAPH);
            CcReal* currEdgeWeightReverse = (CcReal*) currNode->GetAttribute(
                    HL_INDEX_OF_WEIGHT_REVERSE_IN_HL_GRAPH);


            int currEdgeSourceId = currEdgeSource->GetIntval();
            int currEdgeTargetId = currEdgeTarget->GetIntval();

            //check if we reached the next source ID, create new nodeEntry then
            // use the previous one else
            if(currSourceId != currEdgeSourceId)
            {
                #ifdef USEDEBUG
                LogDebug("new source reached, create new vector, new id: "
                 << currEdgeSourceId
                 << " old: " << currSourceId << endl);
                #endif

                currNodeEntry = new HlNodeEntry();
                currEdgeVector = new std::vector<HlEdgeEntry*>();

                currNodeEntry->setNodeId(currEdgeSourceId);
                currNodeEntry->setRankValue(currEdgeSourceRank->GetIntval());
                currNodeEntry->setEdgesVector(currEdgeVector);

                //also add it to map
                nodesEdgesVector[currEdgeSourceIndex->GetIntval()] =
                 currNodeEntry;

                //set currSourceId to next available source
                currSourceId = currEdgeSourceId;
            }


            #ifdef USEDEBUG
            LogDebug("next edge (" << currEdgeSourceId
             << ", " << currEdgeTargetId << " from while" << endl);
            #endif

            //check whether the current edge leads to a valid target
            //in case of the highest rank vertext there are no edges
            // outgoing from that node (because there are no other nodes having
            // higher rank
            //but we still need the node to transform into hlGraph
            // thats why we look for the target id beeing not -1
            // in case of -1 we assume to have the highest rank vertext
            // and will not do add any HlEdgeEntries
            if(currEdgeTargetId != -1)
            {
                HlEdgeEntry* currSourceEdgeEntry = new HlEdgeEntry();
                currSourceEdgeEntry->setTargetIndex(
                 currEdgeTargetIndex->GetIntval());
                currSourceEdgeEntry->setIsForward(
                 currEdgeIsForward->GetBoolval());
                currSourceEdgeEntry->setIsReverse(
                 currEdgeIsReverse->GetBoolval());
                currSourceEdgeEntry->setParentIndexForward(
                 currEdgeParentForward->GetIntval());
                currSourceEdgeEntry->setParentIndexReverse(
                 currEdgeParentReverse->GetIntval());
                currSourceEdgeEntry->setWeightForward(
                 currEdgeWeightForward->GetRealval());
                currSourceEdgeEntry->setWeightReverse(
                 currEdgeWeightReverse->GetRealval());
                currEdgeVector->push_back(currSourceEdgeEntry);
            }


            //get next Tuple from Orel
            currNode->DeleteIfAllowed();
            currNode = 0;
            currNode = hlGraphOrelIter->GetNextTuple();
        }
        delete hlGraphOrelIter;


        #ifdef USEDEBUG
        LogDebug("finish hlTransformHlGraphOrelToHlGraph" << endl);
        #endif
        return true;
    }



    /*
     * Gets the hlGraph and frees all its containing elements by iterating
     *  over it and its subelements and deleting them.
     *
     * @param nodesEdgesVector the hlGraph to be deleted
     *
     * @return true since all modifications take place in the given parameters
    */
    bool hlFreeHlGraph(std::vector<HlNodeEntry*> &nodesEdgesVector) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlFreeHlGraph" << endl);
        #endif

        for (std::vector<HlNodeEntry*>::iterator
         nodesEdgesVectorIter = nodesEdgesVector.begin();
         nodesEdgesVectorIter != nodesEdgesVector.end();
         ++nodesEdgesVectorIter)
        {
             HlNodeEntry* currHlNodeEntry = (*nodesEdgesVectorIter);

             std::vector<HlEdgeEntry*>* currHlEdgeEntries =
              currHlNodeEntry->getEdgesVector();

            for (std::vector<HlEdgeEntry*>::iterator
             currHlEdgeEntriesIter = currHlEdgeEntries->begin();
             currHlEdgeEntriesIter != currHlEdgeEntries->end();
             ++currHlEdgeEntriesIter)
            {
                 HlEdgeEntry* currHlEdgeEntry = (*currHlEdgeEntriesIter);
                 delete currHlEdgeEntry;
            }

            delete currHlEdgeEntries;
            delete currHlNodeEntry;
        }

        #ifdef USEDEBUG
        LogDebug("finish hlFreeHlGraph" << endl);
        #endif

        return true;
    }



    /*
     * Gets the searchTree and frees all its containing elements by iterating
     *  over it and deleting them.
     *
     * @param searchTree the searchTree to be deleted
     *
     * @return true since all modifications take place in the given parameters
    */
    bool hlFreeSearchTree(std::map<int, ChNode*> &searchTree) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlFreeSearchTree" << endl);
        #endif

        for (std::map<int, ChNode*>::iterator
         searchTreeIter = searchTree.begin();
         searchTreeIter != searchTree.end();
         ++searchTreeIter)
        {
            ChNode* currChNode = (*searchTreeIter).second;
            delete currChNode;
        }

        #ifdef USEDEBUG
        LogDebug("finish hlFreeSearchTree" << endl);
        #endif

        return true;
    }



    /*
     * Processes either a CH (isChElseHl = true) or a HL Search (else)
     *  for the given source and target.
     *
     * Basically it performs a Bi-Directional Dijkstra (a forward search from
     *  source and a reverse search from target).
     * By this both searches have their on distance-queue. The search having the
     *  next node to scan with the minimum distance of both queues is
     *  processed next.
     * This way both search-spaced expand equally.
     *
     * In case of both HL and CH Search are proceeded until both queues are
     *  empty. If one queue gets empty the other search still is continued.
     *
     * In case of CH search theres an additional break condition.
     * If the search distance of an (either forward or reverse) search does
     *  exceed the shortestPath found so far this search is aborted. The other
     *  search keeps proceeding.
     *
     * In case of HL search theres no such additional break condition.
     * Target of an HL search is to retrieve the full search tree of a node.
     * That way one should use the same nodeId as source and target as well.
     *
     * Elsewise the behaviour of an CH search is as follows for retrieving the
     *  shortest path.
     * If theres a node visited by both (forward and reverse) searches,
     *  the distances of both searches to this node are summed up and
     *  if the sum is the smallest distance found so far this node will
     *  be stored as current minimum distance.
     * After both searches have finished the currentMinimumDistance is
     *  retrieved as shortest path from source to target.
     *
     * As result it returns a tuple containing
     * - At first the NodeId of the hub found on the shortest path
     *    during Bi-Directional Dijkstra Search/ CH Search.
     *   May this is -1 if theres no shortest path or when a HL-Search was
     *    started.
     * - At second the length of this shortest path if found, 0.0 else.
     * - At third the search tree build up so far during the searches.
     *   Using this tree one can retrieve the single edges of the shortest path.
     *   On HL Queries this is the valid Label of the given node as desired.
     *
     * @param nodesEdgesVector by reference the adjacency-list
     * @param source the sourceNodeId
     * @param target the targetnodeId
     * @param isChElseHl indicates whether we perform a CH Search or HL Search
     * @return the result Tuple as described above.
    */
    std::tuple<int, double, std::map<int, ChNode*>> hlDoCHSearch(
     std::vector<HlNodeEntry*> &nodesEdgesVector,
     int source, int target, bool isChElseHl) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlDoCHSearch" << endl);
        #endif


        //define returnValue
        std::map<int, ChNode*> searchTree;


        //the Predecessor of source/ target itself does not exists
        // and the distance to the nodes themselves is 0.0
        //predecessor must not be -1 since this an indicator for not visited
        // nodes
        int initPredecessorIndex = -2;
        double initDistance = 0.0;




        //get source and target NodeIndex
        HlNodeEntry* sourceNodeEntry = 0;
        HlNodeEntry* targetNodeEntry = 0;
        int sourceNodeIndex = -1;
        int targetNodeIndex = -1;

        for(uint i = 0; i< nodesEdgesVector.size(); i++)
        {
            HlNodeEntry* currNodeEntry = nodesEdgesVector[i];

            if(currNodeEntry->getNodeId() == source)
            {
                sourceNodeEntry = currNodeEntry;
                sourceNodeIndex = i;

                //check if target still has been found, abort if true
                if(targetNodeEntry)
                {
                    break;
                }
            }
            if(currNodeEntry->getNodeId() == target)
            {
                targetNodeEntry = currNodeEntry;
                targetNodeIndex = i;

                //check if source still has been found, abort if true
                if(sourceNodeEntry)
                {
                    break;
                }
            }
        }

        if(!sourceNodeEntry || !targetNodeEntry)
        {
            #ifdef USEERROR
            LogError("Fehler, source or target not found" << endl);
            #endif

            return std::make_tuple(-1, 0.0, searchTree);
        }

        //if source  = target and isCH theres nothing to search
        if(source == target && isChElseHl)
        {
            std::cout <<
             "Warn. Given Source and Target are qual, no path to retrieve."
              << endl;
            return std::make_tuple(sourceNodeIndex, 0.0, searchTree);
        }


        #ifdef USEDEBUG
        LogDebug("prepare forward search" << endl);
        #endif

        //prepare forward search
        // format is double, int, int -> dist, nodeIndex, predecessorIndex
        PriorityQueueTypeForwardSearch forwardQueue;

        //add source itself to searchtree with distance 0.0
        ChNode* sourceChNode = new ChNode();
        sourceChNode->setDistForward(initDistance);
        sourceChNode->setPredecessorIndexForward(initPredecessorIndex);

        searchTree.insert(std::pair<int, ChNode*>(sourceNodeIndex,
         sourceChNode));

        //get outgoing edges from source
        std::vector<HlEdgeEntry*>* currNodeVectorSource =
         sourceNodeEntry->getEdgesVector();

        //init forward queue
        bool isForwardSearch = true;
        hlCHSearchScanNewVertices(forwardQueue, searchTree,
         currNodeVectorSource, initDistance, isForwardSearch, sourceNodeIndex);





        #ifdef USEDEBUG
        LogDebug("prepare reverse search" << endl);
        #endif

        //prepare reverse search
        // format is double, int, int -> dist, nodeIndex, predecessorIndex
        PriorityQueueTypeForwardSearch reverseQueue;

        //add target itself to searchtree with distance 0.0
        // but check if source and target are equal
        // use existing chNode object then
        // only occurs on hlMode (else a previous return had been done
        ChNode* targetChNode = 0;
        if(source == target)
        {
            targetChNode = sourceChNode;

            targetChNode->setDistReverse(initDistance);
            targetChNode->setPredecessorIndexReverse(initPredecessorIndex);
        }else{
            targetChNode = new ChNode();
            targetChNode->setDistReverse(initDistance);
            targetChNode->setPredecessorIndexReverse(initPredecessorIndex);

            searchTree.insert(std::pair<int, ChNode*>(targetNodeIndex,
             targetChNode));
        }

        //get outgoing edges from target
        std::vector<HlEdgeEntry*>* currNodeVectorTarget =
         targetNodeEntry->getEdgesVector();

        //init reverse queue
        hlCHSearchScanNewVertices(reverseQueue, searchTree,
         currNodeVectorTarget, initDistance, !isForwardSearch, targetNodeIndex);




        //do search until ready

        #ifdef USEDEBUG
        LogDebug("do iterative CH search" << endl);
        #endif


        //get top element of forward queue
        double currDistToSourceForward = 0.0;
        int currNodeIndexForward = -1;
        int currNodeIdForward = -1;
        int currPredecessorIndexForward = -1;
        HlNodeEntry* currNodeEntryForward = 0;

        if(!forwardQueue.empty())
        {
            //get data
            QueuePairTypeForwardSearch currForwardQueuePair =
             forwardQueue.top();

            //do not pop yet

            currDistToSourceForward = currForwardQueuePair.first;
            std::pair<int, int> currTupleQueueForward =
             currForwardQueuePair.second;
            currNodeIndexForward = currTupleQueueForward.first;
            currPredecessorIndexForward = currTupleQueueForward.second;

            currNodeEntryForward = nodesEdgesVector[currNodeIndexForward];
            currNodeIdForward = currNodeEntryForward->getNodeId();
        }

        //get top element of reverse queue
        double currDistToTargetReverse = 0.0;
        int currNodeIndexReverse = -1;
        int currNodeIdReverse = -1;
        int currPredecessorIndexReverse = -1;
        HlNodeEntry* currNodeEntryReverse = 0;

        if(!reverseQueue.empty())
        {
            //get data
            QueuePairTypeForwardSearch currReverseQueuePair =
             reverseQueue.top();

            //do not pop yet

            currDistToTargetReverse = currReverseQueuePair.first;
            std::pair<int, int> currTupleQueueReverse =
             currReverseQueuePair.second;
            currNodeIndexReverse = currTupleQueueReverse.first;
            currPredecessorIndexReverse = currTupleQueueReverse.second;

            currNodeEntryReverse = nodesEdgesVector[currNodeIndexReverse];
            currNodeIdReverse = currNodeEntryReverse->getNodeId();
        }


        int currMinShortestPathHubIndex = -1;
        double currMinShortestPathDist = 0.0;

        //do while queues are not empty
        //if other break-conditions are fullfilled we just clear the related
        // queue such that it gets empty
        while((!forwardQueue.empty() || !reverseQueue.empty()))
        {
            #ifdef USEDEBUG
            LogDebug("next queue candidate are: fwd: "
             << currNodeIdForward
             << " with dist: " << currDistToSourceForward
             << " and rvs: " << currNodeIdReverse << " with dist: "
             << currDistToTargetReverse << endl);
            #endif

            //check which queue has the smaller element
            //a queue also has the smaller element if the other queue is empty
            if( (reverseQueue.empty() )
                || ( currDistToSourceForward <= currDistToTargetReverse
                && !forwardQueue.empty() ) )
            {
                //forward search

                //exit if currNodeIndexForward was not found
                if(!currNodeEntryForward)
                {
                    #ifdef USEERROR
                LogError("error while retrieving currForwardNode from hlGraph "
                     << currNodeIdForward << endl);
                    #endif
                }


                //get data
                std::vector<HlEdgeEntry*>* currNodeVectorForward =
                 currNodeEntryForward->getEdgesVector();


                #ifdef USEDEBUG
                LogDebug("next top min forward: " << currNodeIdForward << endl);
                #endif

                //check whether the current node still exists within
                // search tree by reverse search
                // if yes edit the existing object and check for new
                // shortestPath
                //if it exists in search tree but by forward search,
                // we ignore this still visited node
                bool isStillVisitedByReverse = false;
                bool isStillVisitedByForward = false;

                std::map<int, ChNode*>::iterator searchTreeIter =
                    searchTree.find(currNodeIndexForward);

                if(searchTreeIter != searchTree.end())
                {
                    ChNode* currChNode = (*searchTreeIter).second;


                    //chech whether node found is still from forward search
                    if(currChNode->getPredecessorIndexForward() != -1)
                    {
                        isStillVisitedByForward = true;
                    }

                    //chech whether node found is still from reverse search
                    if(currChNode->getPredecessorIndexReverse() != -1)
                    {
                        isStillVisitedByReverse = true;
                    }
                }

                //only proceed if is not still visited by forward search
                if(!isStillVisitedByForward)
                {
                    //if is still visited by opposite search check
                    // for shortest path else add to search tree
                    if(isStillVisitedByReverse)
                    {
                        #ifdef USEDEBUG
                        LogDebug("hub found: " << currNodeIdForward << endl);
                        #endif

                        //edit entry and check for new shortestPath
                        ChNode* currForwardChNode = (*searchTreeIter).second;
                        currForwardChNode->setDistForward(
                         currDistToSourceForward);
                        currForwardChNode->setPredecessorIndexForward(
                         currPredecessorIndexForward);

                        double shortestPathCandidateDist =
                            currDistToSourceForward
                            + currForwardChNode->getDistReverse();

                        //check if a hub still has been found or if the current
                        // hub provides a shorter path than found so far
                        if(currMinShortestPathHubIndex == -1 ||
                         shortestPathCandidateDist < currMinShortestPathDist)
                        {
                            #ifdef USEDEBUG
                            LogDebug("new shorter path with length: "
                             << shortestPathCandidateDist << endl);
                            #endif

                            currMinShortestPathHubIndex = currNodeIndexForward;
                            currMinShortestPathDist = shortestPathCandidateDist;
                        }
                    }
                    else
                    {
                        #ifdef USEDEBUG
                        LogDebug("insert new into searchTree id:"
                            << currNodeIdForward
                            << " with dist: " << currDistToSourceForward
                            << " and predecessor index "
                            << currPredecessorIndexForward
                            << endl);
                        #endif

                        ChNode* currForwardChNode = new ChNode();
                        currForwardChNode->setDistForward(
                         currDistToSourceForward);
                        currForwardChNode->setPredecessorIndexForward(
                         currPredecessorIndexForward);

                        searchTree.insert(std::pair<int, ChNode*>(
                            currNodeIndexForward, currForwardChNode));
                    }

                    //scan new vertices for inserting into queue
                    hlCHSearchScanNewVertices(forwardQueue, searchTree,
                     currNodeVectorForward, currDistToSourceForward,
                     isForwardSearch, currNodeIndexForward);
                }

                //pop current top element from queue to get next
                forwardQueue.pop();

                //check whether its empty
                if(!forwardQueue.empty())
                {
                    //get data
                    QueuePairTypeForwardSearch currForwardQueuePair =
                     forwardQueue.top();

                    currDistToSourceForward = currForwardQueuePair.first;
                    std::pair<int, int> currTupleQueueForward =
                     currForwardQueuePair.second;
                    currNodeIndexForward = currTupleQueueForward.first;
                    currPredecessorIndexForward =
                     currTupleQueueForward.second;

                    currNodeEntryForward =
                     nodesEdgesVector[currNodeIndexForward];
                    currNodeIdForward = currNodeEntryForward->getNodeId();

                    //check whether we are in CH mode and if the distance
                    // of the next top element exceeds the current shortest
                    // path found so far
                    //also check that there still exists a shortet path hub
                    //clear the queue then to stop the search
                    if(isChElseHl && currMinShortestPathHubIndex != -1 &&
                       currDistToSourceForward >= currMinShortestPathDist)
                    {
                        #ifdef USEDEBUG
                        LogDebug("next top node id:" << currNodeIdForward
                            << " with distToSource: " << currDistToSourceForward
                            << " exceeds currMinDist: "
                            << currMinShortestPathDist << endl);
                        #endif

                        //clear queue by reassigning it
                        forwardQueue = PriorityQueueTypeForwardSearch();
                    }
                }
            }
            else
            {
                //reverse search

                //exit if currNodeIdReverse was not found
                if(!currNodeEntryReverse)
                {
                    #ifdef USEERROR
                LogError("error while retrieving currReverseNode from hlGraph "
                     << currNodeIdReverse << endl);
                    #endif
                }


                //get data
                std::vector<HlEdgeEntry*>* currNodeVectorReverse =
                 currNodeEntryReverse->getEdgesVector();


                #ifdef USEDEBUG
                LogDebug("next top min reverse: " << currNodeIdReverse << endl);
                #endif

                //check whether the current node still exists within
                // search tree by forward search
                // if yes edit the existing object and check for new
                // shortestPath
                //if it exists in search tree but by reverse search,
                // we ignore this still visited node
                bool isStillVisitedByReverse = false;
                bool isStillVisitedByForward = false;

                std::map<int, ChNode*>::iterator searchTreeIter =
                    searchTree.find(currNodeIndexReverse);

                if(searchTreeIter != searchTree.end())
                {
                    ChNode* currChNode = (*searchTreeIter).second;

                    //chech whether node found is still from reverse search
                    if(currChNode->getPredecessorIndexReverse() != -1)
                    {
                        isStillVisitedByReverse = true;
                    }

                    //chech whether node found is still from forward search
                    if(currChNode->getPredecessorIndexForward() != -1)
                    {
                        isStillVisitedByForward = true;
                    }
                }

                //if is still visited by opposite search check
                // for shortest path else add to search tree
                if(!isStillVisitedByReverse)
                {
                    if(isStillVisitedByForward)
                    {
                        #ifdef USEDEBUG
                        LogDebug("hub found: " << currNodeIdReverse << endl);
                        #endif

                        //edit entry and check for new shortestPath
                        ChNode* currReverseChNode = (*searchTreeIter).second;
                        currReverseChNode->setDistReverse(
                        currDistToTargetReverse);
                        currReverseChNode->setPredecessorIndexReverse(
                         currPredecessorIndexReverse);

                        double shortestPathCandidateDist =
                         currDistToTargetReverse
                            + currReverseChNode->getDistForward();

                        //check if a hub still has been found or if the current
                        // hub provides a shorter path than found so far
                        if(currMinShortestPathHubIndex == -1
                         || shortestPathCandidateDist < currMinShortestPathDist)
                        {
                            #ifdef USEDEBUG
                            LogDebug("new shorter path with length: "
                             << shortestPathCandidateDist << endl);
                            #endif

                            currMinShortestPathHubIndex = currNodeIndexReverse;
                            currMinShortestPathDist = shortestPathCandidateDist;
                        }
                    }
                    else
                    {
                        #ifdef USEDEBUG
                        LogDebug("insert new into searchTree id:"
                            << currNodeIdReverse
                            << " with dist: " << currDistToTargetReverse
                            << " and predecessor "
                            << currPredecessorIndexReverse
                            << endl);
                        #endif

                        ChNode* currReverseChNode = new ChNode();
                        currReverseChNode->setDistReverse(
                         currDistToTargetReverse);
                        currReverseChNode->setPredecessorIndexReverse(
                         currPredecessorIndexReverse);

                        searchTree.insert(std::pair<int, ChNode*>(
                            currNodeIndexReverse, currReverseChNode));
                    }

                    //scan new vertices for inserting into queue
                    hlCHSearchScanNewVertices(reverseQueue, searchTree,
                     currNodeVectorReverse, currDistToTargetReverse,
                     !isForwardSearch, currNodeIndexReverse);
                }

                //pop current top element from queue to get next
                reverseQueue.pop();

                //check whether its empty
                if(!reverseQueue.empty())
                {
                    //get data
                    QueuePairTypeForwardSearch currReverseQueuePair =
                     reverseQueue.top();
                    //do not pop it yet, we need to handle it within next
                    //  iteration

                    currDistToTargetReverse = currReverseQueuePair.first;
                    std::pair<int, int> currTupleQueueReverse =
                     currReverseQueuePair.second;
                    currNodeIndexReverse = currTupleQueueReverse.first;
                    currPredecessorIndexReverse =
                     currTupleQueueReverse.second;

                    currNodeEntryReverse =
                     nodesEdgesVector[currNodeIndexReverse];

                    currNodeIdReverse = currNodeEntryReverse->getNodeId();

                    //check whether we are in CH mode and if the distance
                    // of the next top element exceeds the current shortest
                    // path found so far
                    //also check that there still exists a shortet path hub
                    //clear the queue then to stop the search
                    if(isChElseHl && currMinShortestPathHubIndex != -1 &&
                       currDistToTargetReverse >= currMinShortestPathDist)
                    {
                        #ifdef USEDEBUG
                        LogDebug("next top node id:" << currNodeIdReverse
                            << " with distToTarget: " << currDistToTargetReverse
                            << " exceeds currMinDist: "
                            << currMinShortestPathDist << endl);
                        #endif

                        //clear queue by reassigning it
                        forwardQueue = PriorityQueueTypeForwardSearch();
                    }
                }
            }
        }

        //search finished build result type
        std::tuple<int, double, std::map<int, ChNode*>> retVal =
            std::make_tuple(currMinShortestPathHubIndex,
            currMinShortestPathDist, searchTree);

        #ifdef USEDEBUG
        LogDebug("finish hlDoCHSearch" << endl);
        #endif



        return retVal;
    }


    /*
     * Ierates over all outgoing edges and may adds or merge them into
     *  the given forwardQueueMultimap. Edges are only added if they do not
     *  have been still visited due to previous iterations.
     * Edges are merged if they still exist within the queue but are on a
     *  shorter path to the specific target node than it was inserted into
     *  the queue before.
     * To check whether a node still has been visited we make a lookup
     *  within the searchTree.
     *
     * @param currQueue by reference the queue where to maybe insert or merge
     *          the given edges
     * @param searchTree the search tree by reference build up so far
     * @param nodesEdges the hlGraph for getting nodeIds
     * @param currNodeVector the edges to be inserted or merged
     * @param distanceSoFar the search-distance of the current node
     *          being source of the given edges
     * @param isForward indicates whether we are on forward or reverse search
     * @param predecessorIndex the index within hlGraph of the predecessor
     * @return true since all modifications are done within the given parameters
    */
    bool hlCHSearchScanNewVertices(PriorityQueueTypeForwardSearch &currQueue,
     std::map<int, ChNode*> &searchTree,
     std::vector<HlEdgeEntry*>* currNodeVector, double distanceSoFar,
     bool isForward, int predecessorIndex) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlCHSearchScanNewVertices" << endl);
        #endif

        //iterate over all edges
        for (std::vector<HlEdgeEntry*>::iterator
         currNodeVectorIter = currNodeVector->begin();
         currNodeVectorIter != currNodeVector->end();
         ++currNodeVectorIter)
        {
            HlEdgeEntry* currEdgeEntry =
             *currNodeVectorIter;

            int currTargetIndex = -1;
            double currWeight = 0.0;

            //check whether edge relates to the desired search
            // or if theres no related edge to scan
            if(isForward && currEdgeEntry->getIsForward())
            {
                currTargetIndex = currEdgeEntry->getTargetIndex();
                currWeight = currEdgeEntry->getWeightForward();
            }
            else if(!isForward && currEdgeEntry->getIsReverse())
            {
                currTargetIndex = currEdgeEntry->getTargetIndex();
                currWeight = currEdgeEntry->getWeightReverse();
            }else{
                #ifdef USEDEBUG
                LogDebug("current edge not related to the desired direction"
                 << endl);
                #endif
                continue;
            }


            #ifdef USEDEBUG
            LogDebug("scan next edge for inserting into queue, targetIndex: "
             << currTargetIndex << endl);
            #endif

            //check whether current target node still has been visited
            // by current search direction (it may still exists by
            // the opposite search)x
            bool isStillVisited = false;

            std::map<int, ChNode*>::iterator searchTreeIter =
             searchTree.find(currTargetIndex);

            if(searchTreeIter != searchTree.end())
            {
                ChNode* currChNode = (*searchTreeIter).second;

                //in case of start end end nodes of the CH Search
                // the predecessor of these both nodes is -2
                //so it is safe to compare predecessor to -1 here
                if( ( isForward &&
                 currChNode->getPredecessorIndexForward() != -1)
                 || (!isForward &&
                  currChNode->getPredecessorIndexReverse() != -1) )
                {
                    isStillVisited = true;
                }
            }

            //only add to search tree if not has been still visited
            //just skip else
            if(!isStillVisited)
            {
                double distFromSourceToCurrTarget = distanceSoFar + currWeight;

                #ifdef USEDEBUG
                LogDebug("insert into queue index: " << currTargetIndex
                 << " with dist from source: " << distFromSourceToCurrTarget
                 << " with predecessorIndex: " << predecessorIndex
                 << endl);
                #endif

                currQueue.push(std::make_pair(
                  distFromSourceToCurrTarget,
                  std::make_pair(currTargetIndex, predecessorIndex)
                 ));
            }
        }


        #ifdef USEDEBUG
        LogDebug("finish hlCHSearchScanNewVertices" << endl);
        #endif

        return true;
    }



    /*
     * Extracts the shortestPath from the given search tree and alo
     *  resolves shortcut edges to the underlying original edges
     *  and adds all original osm edges in their order from source to
     *  target to the given Relation.
     *
     * @param shortestPathHubIndex the hub nodeIndex
     *         on the shortest path from s to t
     * @param nodesEdgesVector the hlGraph by reference used for
     *         resolving shortcut edges
     * @param searchTree the search tree by reference build up so far
     * @param edgesSourceOrel the osm graph used to extract osm edges
     * @param shortestPathRel the resulting relation containing shortest path
     * @return true since all modifications are done within the given parameters
    */
    bool hlResolveShortestPathFromSearchTree(int shortestPathHubIndex,
     std::vector<HlNodeEntry*> &nodesEdgesVector,
     std::map<int, ChNode*> &searchTree, OrderedRelation* edgesSourceOrel,
     Relation* shortestPathRel) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlResolveShortestPathFromSearchTree" << endl);
        #endif

        //if searchtree does not have at least three elements (s, v, t) theres
        // no path to retrieve
        if(searchTree.size() < 2)
        {
            #ifdef USEDEBUG
            LogDebug("Not enough hops for creating edges." << endl);
            #endif
            return false;
        }

        //prepare path vectors
        std::vector<int> shortestPathForward;
        std::vector<int> shortestPathReverse;

        bool isForward = true;

        #ifdef USEDEBUG
        LogDebug("forward resolve from searchTree" << endl);
        #endif

        hlResolveShortestPathFromSearchTreeToVector(searchTree,
         shortestPathForward, shortestPathHubIndex, isForward);


        #ifdef USEDEBUG
        LogDebug("reverse resolve from searchTree" << endl);
        #endif

        hlResolveShortestPathFromSearchTreeToVector(searchTree,
         shortestPathReverse, shortestPathHubIndex, !isForward);

        //since the forward vector was traversed in reverse manner we need
        // to invert it
        //since the reverse vector was traversed in reverse manner and
        // it was reverse search it still has the right order

        int currUIndex = -1;
        int currWIndex = -1;

        LogDebug("iterate forward edges" << endl);

        //iterate forward edges and resolve by OSM-Edges and add to result rel
        for (std::vector<int>::reverse_iterator
         shortestPathForwardIter = shortestPathForward.rbegin();
         shortestPathForwardIter != shortestPathForward.rend();
         ++shortestPathForwardIter)
        {
            currWIndex = (*shortestPathForwardIter);


             //resolve shortcut and get osm edge (if its not the first call)
             if(currUIndex != -1)
             {
                hlResolveShortcutAndAddToOrel(currUIndex, currWIndex,
                 nodesEdgesVector, shortestPathRel, edgesSourceOrel, isForward);
             }

             //go to next edge
             currUIndex = currWIndex;
        }

        currUIndex = -1;
        currWIndex = -1;


        LogDebug("iterate reverse edges" << endl);

        //iterate reverse edges and resolve by OSM-Edges and add to result Orel
        for (std::vector<int>::iterator
         shortestPathReverseIter = shortestPathReverse.begin();
         shortestPathReverseIter != shortestPathReverse.end();
         ++shortestPathReverseIter)
        {
             currWIndex = (*shortestPathReverseIter);


             //resolve shortcut and get osm edge (if its not the first call)
             if(currUIndex != -1)
             {
                hlResolveShortcutAndAddToOrel(currUIndex, currWIndex,
                 nodesEdgesVector, shortestPathRel, edgesSourceOrel,
                 !isForward);
             }

             //go to next edge
             currUIndex = currWIndex;
        }

        #ifdef USEDEBUG
        LogDebug("finish hlResolveShortestPathFromSearchTree" << endl);
        #endif

        return true;
    }




    /*
     * Traverses the searchTree in a reverse manner such that we get the
     *  shortestPath of the specific search to the given hubId.
     *
     * @param forwardQueueMultimap the quere where to maybe insert or merge
     *          the given edges
     * @param searchTree the search tree by reference
     * @param shortestPathVector by reference the container to
               store shortest path edges
     * @param shortestPathHubIndex the hubIndex of the shortest path
     * @param isForward indicates whether we are on forward or reverse search
     * @return true since all modifications are done within the given parameters
    */
    bool hlResolveShortestPathFromSearchTreeToVector(std::map<int, ChNode*>
     &searchTree, std::vector<int> &shortestPathVector,
     int shortestPathHubIndex, bool isForward) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlResolveShortestPathFromSearchTreeToVector" << endl);
        #endif

        #ifdef USEDEBUG
        LogDebug("add shortestPathHubIndex from search tree to forward vector: "
          << shortestPathHubIndex << " while isForward: " << isForward << endl);
        #endif

        //insert hub to path vector
        shortestPathVector.push_back(shortestPathHubIndex);


        std::map<int, ChNode*>::iterator searchTreeIter =
         searchTree.find(shortestPathHubIndex);

        int currPredecessorIndex = -2;

        //resolve forward search
        while(searchTreeIter != searchTree.end())
        {
            //get next node
            ChNode* currNode = (*searchTreeIter).second;

            if(isForward)
            {
                currPredecessorIndex = currNode->getPredecessorIndexForward();
            }else{
                currPredecessorIndex = currNode->getPredecessorIndexReverse();
            }

            //check whether currPredecessorId = -2 which means were done
            // because we reached the actual source or target
            if(currPredecessorIndex != -2)
            {

                #ifdef USEDEBUG
                LogDebug("add next node from search tree to vector: "
                  << currPredecessorIndex << endl);
                #endif

                //add next node to vector
                shortestPathVector.push_back(currPredecessorIndex);
            }

            //find next node
            searchTreeIter = searchTree.find(currPredecessorIndex);
        }

        #ifdef USEDEBUG
        LogDebug("finish hlResolveShortestPathFromSearchTreeToVector" << endl);
        #endif

        return true;
    }




    /*
     * Gets the edge for the given endpoints u and w and checks whether it
     *  is a shortcut. If yes it calls the methode itself recursively.
     * If not the both underlying edges are added to the given vector.
     * The edges are inserted in forward direction u -> w.
     *
     * Because we sparsed the hlGraph during creation of upward and downward
     *  graphs the underlying edges/ shortcuts of an shortcut from u via v to w
     *  need to be found in different direction. Naturally v always has a
     *  lower rank than u and w as well. So we can find the underlying edge
     *  u to v as reverse edge stored at v in the hlGrapg and v to w as
     *  forward edge stored with v. This is valid for both, the forward array
     *  and the backward array as well.
     *
     * @param nodeUIndex the start nodeIndex of the currnt edge
     * @param nodeWIndex the target nodeIndex of the currnt edge
     * @param nodesEdgesVector the search tree by reference
     * @param resultEdgesRel the resulting rel filled with shortest path edges
     * @param edgesSourceOrel the osm graph used to extract osm edges
     * @param isForward indicates whether we want to retrieve an forward
     *         or reverse edeg
     * @return true since all modifications are done within the given parameters
    */
    bool hlResolveShortcutAndAddToOrel(int currUIndex, int currWIndex,
     std::vector<HlNodeEntry*> &nodesEdgesVector,
     Relation* resultEdgesRel, OrderedRelation* edgesSourceOrel,
     bool isForward) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlResolveShortcutAndAddToOrel" << endl);
        #endif

        //if we want a reverse edge we need to search for it at the target node
        int tmpLookupNodeIndex = -1;
        int tmpOppositeNodeIndex = -1;
        if(isForward)
        {
            tmpLookupNodeIndex = currUIndex;
            tmpOppositeNodeIndex = currWIndex;
        }else{
            tmpLookupNodeIndex = currWIndex;
            tmpOppositeNodeIndex = currUIndex;
        }

        HlNodeEntry* currHlNodeEntry = nodesEdgesVector[tmpLookupNodeIndex];

        //get data
        std::vector<HlEdgeEntry*>* currEdgeVector =
         currHlNodeEntry->getEdgesVector();

        HlEdgeEntry* currEdge = hlGetEdgeFromVector(currEdgeVector,
         tmpOppositeNodeIndex);

        //shortcut parent
        int currVIndex = -1;

        //if is reverse edge we need to get the reverse-edges parent
        if(isForward)
        {
            currVIndex = currEdge->getParentIndexForward();
        }else{
            currVIndex = currEdge->getParentIndexReverse();
        }

        //check if is shortcut, recursive call if true
        if(currVIndex != -1)
        {
            bool isForwardShortcut = true;

            #ifdef USEDEBUG
            LogDebug("is shortcut, resolve (uIndex, vIndex): ("
             << currUIndex << ", " << currVIndex << ")" << endl);
            #endif

            //(u, v)
            //the shortcuts part-edge u-> v always is to be searched
            // as reverse edge because u and w mus have higher rank than v
            //this is independent of whether we are currently iterating the
            // forward or reverse vector
            hlResolveShortcutAndAddToOrel(currUIndex, currVIndex,
             nodesEdgesVector, resultEdgesRel, edgesSourceOrel,
              !isForwardShortcut);

            #ifdef USEDEBUG
            LogDebug("is shortcut, resolve (vIndex, wIndex): ("
             << currVIndex << ", " << currWIndex << ")" << endl);
            #endif

            //(v, w)
            //the shortcuts part-edge v-> w always is to be searched
            // as forward edge because u and w mus have higher rank than v
            //this is independent of whether we are currently iterating the
            // forward or reverse vector
            hlResolveShortcutAndAddToOrel(currVIndex, currWIndex,
             nodesEdgesVector, resultEdgesRel, edgesSourceOrel,
             isForwardShortcut);
        }else{
            #ifdef USEDEBUG
            LogDebug("no shortcut, add to result relation (uIndex, wIndex): ("
             << currUIndex << ", " << currWIndex << ")" << endl);
            #endif
            //no shortcut so add it to resultrel

            //get OSM nodeIds
            HlNodeEntry* currUNodeEntry = nodesEdgesVector[currUIndex];
            HlNodeEntry* currWNodeEntry = nodesEdgesVector[currWIndex];
            int currUNodeId = currUNodeEntry->getNodeId();
            int currWNodeId = currWNodeEntry->getNodeId();

            #ifdef USEDEBUG
            LogDebug("osm Node IDs: (u, w): ("
             << currUNodeId << ", " << currWNodeId << ")" << endl);
            #endif

            //get osm-edge Tuple
            CcInt* sourceNodeIdCcInt = new CcInt(true, currUNodeId);
            CcInt* targetNodeIdCcInt = new CcInt(true, currWNodeId);
            Tuple* currOrelTuple = hlGetEdgeFromOrel(edgesSourceOrel,
             sourceNodeIdCcInt, targetNodeIdCcInt);
            delete sourceNodeIdCcInt;
            delete targetNodeIdCcInt;

            //insert into result relTuple
            resultEdgesRel->AppendTuple(currOrelTuple);

            currOrelTuple->DeleteIfAllowed();
            currOrelTuple = 0;
        }


        #ifdef USEDEBUG
        LogDebug("finish hlResolveShortcutAndAddToOrel" << endl);
        #endif

        return true;
    }

    /*
     * Iterates through the given Edges-Orel from OSM Graph and
     *  retrieves the length of every curve.
     * Also retrieves the RoadType of the specific edge and multiplies
     *  the length with a const valua representing the driving speed
     *  valid for this RoadType.
     * Stores the calculated weights within the field Costs
     *  of the specific edge.
     * Expects the Osm-Orel to fulfil the schema of
     *  @see hlGetEdgesTupleTypeInfo()
     *
     * Supports a parameter to give the calculation mode but currently
     *  it is not implemented.
     *
     * @param edgesOrelOsm the input Edges-Orel from OSM Import
     * @param calcMode the calculation mode to use (not yet implemented)
     * @param true since all modifications are done within the given parameters
     *
    */
    void hlCalcWeightsOrel(OrderedRelation* edgesOrel, int calcMode) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlCalcWeightsOrel" << endl);
        #endif

        GenericRelationIterator* edgesOrelIter = edgesOrel->MakeScan();
        Tuple* currTuple = edgesOrelIter->GetNextTuple();

        while(currTuple)
        {
            //for non static use
            int indexCosts = HubLabelClass::HL_INDEX_OF_COSTS_IN_EDGE_TUPLE;

            SimpleLine* curve = (SimpleLine*) currTuple->GetAttribute(
                        HubLabelClass::HL_INDEX_OF_CURVE_IN_EDGE_TUPLE);
            FText* roadTypeFText = (FText*) currTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_ROAD_TYPE_IN_EDGE_TUPLE);

            //get data
            double curveLength = curve->Length();
            string roadType = roadTypeFText->GetValue();

            int assumedSpeed = 0;
            double calculatedCosts = 0.0;

            //get speed information
            int defaultSpeed = 30;      //Default Speed for unkown cases
            int motorwayLike = 130;     //Autobahn
            int motorwayLinkLike = 80;  //Autobahn Auf- und Abfahrt
            int trunkLike = 60;         //Ausgebaute Schnellstraße
            int trunkLinkLike = 110;    //Auf-/ Abfahrt zu Schnellstraßen
            int primaryLike = 90;       //Bundesstraßen
            int primaryLinkLike = 30;   //Auf-/ Abfahrt zu Bundesstraßen
            int secondaryLike = 70;     //Landstraßen
            int secondaryLinkLike = 30; //Auf-/ Abfahrt zu Landstraßen
            int tertiaryLike = 70;      //Kreisstraßen
            int tertiaryLinkLike = 30;  //Auf-/ Abfahrt zu Kreisstraßen
            int roadLike = defaultSpeed;//Unknown type treat as slow
            int residentialLike = 40;   //Anwohnerstraßen
            int livingStreetLike = 20;  //Spielstraße
            int unclassifiedLike = 40;  //asphaltierte Feldwege

            if(roadType == "motorway")
            {
                //Autobahn -> 130
                assumedSpeed = motorwayLike;
            }
            else if(roadType == "motorway_link")
            {
                //Autobahn Auf- und Abfahrt -> 30
                assumedSpeed = motorwayLinkLike;
            }
            else if(roadType == "trunk")
            {
                //Ausgebaute Schnellstraße -> 110
                assumedSpeed = trunkLike;
            }
            else if(roadType == "trunk_link")
            {
                //Auf-/ Abfahrt zu Schnellstraßen -> 30
                assumedSpeed = trunkLinkLike;
            }
            else if(roadType == "primary")
            {
                //Bundesstraßen -> 90
                assumedSpeed = primaryLike;
            }
            else if(roadType == "primary_link")
            {
                //Auf-/ Abfahrt zu Bundesstraßen -> 30
                assumedSpeed = primaryLinkLike;
            }
            else if(roadType == "secondary")
            {
                //Landstraßen -> 70
                assumedSpeed = secondaryLike;
            }
            else if(roadType == "secondary_link")
            {
                //Auf-/ Abfahrt zu Landstraßen -> 30
                assumedSpeed = secondaryLinkLike;
            }
            else if(roadType == "tertiary")
            {
                //Kreisstraßen -> 70
                assumedSpeed = tertiaryLike;
            }
            else if(roadType == "tertiary_link")
            {
                //Auf-/ Abfahrt zu Kreisstraßen -> 30
                assumedSpeed = tertiaryLinkLike;
            }
            else if(roadType == "road")
            {
                //Unknown type treat as slow -> 30
                assumedSpeed = roadLike;
            }
            else if(roadType == "residential")
            {
                //Anwohnerstraßen -> 40
                assumedSpeed = residentialLike;
            }
            else if(roadType == "living_street")
            {
                //Spielstraße -> 20
                assumedSpeed = livingStreetLike;
            }
            else if(roadType == "unclassified")
            {
                //asphaltierte Feldwege -> 40
                assumedSpeed = unclassifiedLike;
            }
            else
            {
                //Default Speed like unknown 'road'
                assumedSpeed = defaultSpeed;
            }

            //calc

            //curveLength = way in km
            //speed given in km/h = way per time
            //travel time = way / way per time
            // -> curveLength / speed
            //
            // we think to multiply length by 10 to get the value with
            // unit nearly to be kilometer (does not fit exactly like measuring
            // with google, duffers about several ten meters
            //
            calculatedCosts = curveLength * 10 / assumedSpeed;

            //set costs
            CcReal* newCosts = new CcReal();
            newCosts->Set(true, calculatedCosts);

            //prepare updateTuple
            std::vector<int> changedIndices;
            changedIndices.push_back(indexCosts);

            std::vector<Attribute*> changedAttributes;
            changedAttributes.push_back(newCosts);


            //update costs
            edgesOrel->UpdateTuple(currTuple, changedIndices,
             changedAttributes);

            //Free Outgoing-Iteration
            currTuple->DeleteIfAllowed();
            currTuple = 0;
            currTuple = edgesOrelIter->GetNextTuple();
        }

        //Free Iterator
        delete edgesOrelIter;


        #ifdef USEDEBUG
        LogDebug("finish hlCalcWeightsOrel" << endl);
        #endif
    }



    /*
     * Nimmt einen hlGraph entgegen und führt eine HL Search für jeden Knoten
     *  aus dem hlGraph aus und persistiert die resultierenden SearchTrees.
     *
     * TODO:die ursprünglich space efficient serchTree Struktur wird hier nicht
     *  wiederverwendet für die persistierung der Labels
     *  Statdessen wird die alte NRel darstellung aus dem
     *  ersten Versuch verwendet
     *
     * @param allLabelsNrel the nrel to persist labels to be cerated
     * @param nodesOrel an orel containing all nodes of the hlGraph
     * @param nodesEdgesVector by reference the hlGraph to fill
     * @param true since all modifications are done within the given parameters
     *
    */
    void hlCreateLabelsFromHlGraph(NestedRelation* allLabelsNrel,
     OrderedRelation* nodesOrel,
     std::vector<HlNodeEntry*> nodesEdgesVector) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlCreateLabelsFromHlGraph" << endl);
        #endif

        #ifdef USEPERF
        int totalLabelSizeForward = 0;
        int totalLabelSizeReverse = 0;
        #endif


        bool isChElseHl = false;

        //prepare nestedRelation
        SubRelation* forwardDataSubRel =
            allLabelsNrel->getSubRel(
                hlForwardLabelColumnName());
        SubRelation* reverseDataSubRel =
            allLabelsNrel->getSubRel(
                hlReverseLabelColumnName());

        SmiFileId forwardDataSubRelFileId =
            forwardDataSubRel->fileId;
        SmiFileId reverseDataSubRelFileId =
            reverseDataSubRel->fileId;

        ListExpr forwardDataSubRelTypeInfo =
            forwardDataSubRel->typeInfo;
        ListExpr reverseDataSubRelTypeInfo =
            reverseDataSubRel->typeInfo;

        Relation* forwardDataRel = forwardDataSubRel->rel;
        Relation* reverseDataRel = reverseDataSubRel->rel;

        TupleType* tupleTypeAllLabels =
            allLabelsNrel->getPrimary()->GetTupleType();

        //iterate through nodesOrel and create Labels for every Node
        GenericRelationIterator* nodesOrelIter = nodesOrel->MakeScan();

        //Get first Tuple
        Tuple* currNode = nodesOrelIter->GetNextTuple();

        #ifdef USEINFO
        int nodesCount = nodesOrel->GetNoTuples();
        int progressCount = 0;
        #endif


        //Iterate over all Tuples
        while(currNode)
        {
            CcInt* currNodeIdCcInt = (CcInt*) currNode->GetAttribute(
                    HL_INDEX_OF_SOURCE_IN_HL_GRAPH_NODES);

            int currNodeId = currNodeIdCcInt->GetIntval();

            #ifdef USEINFO
            ++progressCount;
            if(progressCount % getProgressInterval() == 0)
            {
                LogInfo("Create Labels for next nodeId: "
                 << currNodeId << " (progress: " << progressCount
                 << " nodes left: " << (nodesCount - progressCount) << ")"
                 << endl);
            }
            #endif


            #ifdef USEDEBUG
            LogDebug("forward  and reverse label multimaps" << endl);
            #endif
            std::multimap<int, std::tuple<int, int, double>>
                    labelForwardMultimap;
            std::multimap<int, std::tuple<int, int, double>>
                    labelReverseMultimap;


            #ifdef USEDEBUG
            LogDebug("do search" << endl);
            #endif
            std::tuple<int, double, std::map<int, ChNode*>> searchResultTuple =
            hlDoCHSearch(nodesEdgesVector, currNodeId, currNodeId, isChElseHl);

            std::map<int, ChNode*> searchTree = std::get<2>(searchResultTuple);


            //iterate through searchTree
            for (std::map<int, ChNode*>::iterator
                    searchTreeIter = searchTree.begin();
                    searchTreeIter != searchTree.end();
                    ++searchTreeIter)
            {

                int currHubIndex = (*searchTreeIter).first;
                ChNode* currHubChNode = (*searchTreeIter).second;

                double distForward = currHubChNode->getDistForward();
                double distReverse = currHubChNode->getDistReverse();
                int predecessorIndexForward =
                 currHubChNode->getPredecessorIndexForward();
                int predecessorIndexReverse =
                 currHubChNode->getPredecessorIndexReverse();

                delete currHubChNode;


                HlNodeEntry* currHubNodeEntry = nodesEdgesVector[currHubIndex];
                int currHubNodeId = currHubNodeEntry->getNodeId();
                int currHubRankValue = currHubNodeEntry->getRankValue();

                //only get predecessor if there is one
                // it can be -1 or -2
                int currPredecessorForwardNodeId = -1;
                if(predecessorIndexForward > -1)
                {
                    HlNodeEntry* currPredecessorForwardNodeEntry =
                     nodesEdgesVector[predecessorIndexForward];
                    currPredecessorForwardNodeId =
                     currPredecessorForwardNodeEntry->getNodeId();
                }

                int currPredecessorReverseNodeId = -1;
                if(predecessorIndexReverse > -1)
                {
                    HlNodeEntry* currPredecessorReverseNodeEntry =
                     nodesEdgesVector[predecessorIndexReverse];
                    currPredecessorReverseNodeId =
                     currPredecessorReverseNodeEntry->getNodeId();
                }




                //add to forward label if is sourceNode or if is hub
                // which always has a forward predecessor
                if(currNodeId == currHubNodeId ||
                 predecessorIndexForward > -1)
                {

                    //rankOfHub/nodeIdNew, hubNodeId, predecessorId,
                    // distToSource
                    labelForwardMultimap.insert(std::make_pair(currHubRankValue,
                     std::make_tuple(currHubNodeId,
                     currPredecessorForwardNodeId, distForward)));
                }

                //add to reverse label if is sourceNode or if is hub
                // which always has a reverse predecessor
                if(currNodeId == currHubNodeId ||
                 predecessorIndexReverse > -1)
                {

                    //rankOfHub/nodeIdNew, hubNodeId, predecessorId,
                    // distToSource
                    labelReverseMultimap.insert(std::make_pair(currHubRankValue,
                     std::make_tuple(currHubNodeId,
                     currPredecessorReverseNodeId, distReverse)));
                }

            }

            #ifdef USEDEBUG
            LogDebug("prepare NestedRelationTuple" << endl);
            #endif

            Tuple* allLabelsTuple = new Tuple(
                tupleTypeAllLabels);
            allLabelsTuple->PutAttribute(
                HL_INDEX_OF_NODE_ID_IN_ALL_LABELS_TUPLE,
                new CcInt(true, currNodeId));


            #ifdef USEDEBUG
            LogDebug("create and fill forwardLabel" << endl);
            #endif
            AttributeRelation* labelForwardArel = new
            AttributeRelation(forwardDataSubRelFileId,
                              forwardDataSubRelTypeInfo,
                              labelForwardMultimap.size());
            labelForwardArel->setPartOfNrel(true);

            hlFillForwardOrReverseLabel(labelForwardArel, forwardDataRel,
             labelForwardMultimap);



            #ifdef USEDEBUG
            LogDebug("add forward label to all labels" << endl);
            #endif
            allLabelsTuple->PutAttribute(
                HL_INDEX_OF_FORWARD_LABEL_IN_ALL_LABELS_TUPLE,
                labelForwardArel);


            #ifdef USEDEBUG
            LogDebug("create and fill reverseLabel" << endl);
            #endif
            AttributeRelation* labelReverseArel = new
            AttributeRelation(reverseDataSubRelFileId,
                              reverseDataSubRelTypeInfo,
                              labelReverseMultimap.size());
            labelReverseArel->setPartOfNrel(true);

            hlFillForwardOrReverseLabel(labelReverseArel, reverseDataRel,
             labelReverseMultimap);


            #ifdef USEDEBUG
            LogDebug("add reverse label to all labels" << endl);
            #endif
            allLabelsTuple->PutAttribute(
                HL_INDEX_OF_REVERSE_LABEL_IN_ALL_LABELS_TUPLE,
                labelReverseArel);

            #ifdef USEDEBUG
            LogDebug("finish current all labels Tuple" << endl);
            #endif

            #ifdef USEPERF
            totalLabelSizeForward += labelForwardMultimap.size();
            totalLabelSizeReverse += labelReverseMultimap.size();
            #endif

            allLabelsNrel->getPrimary()->AppendTuple(
                allLabelsTuple);

            allLabelsTuple->DeleteIfAllowed();
            allLabelsTuple = 0;


            //get next Tuple from Orel
            currNode->DeleteIfAllowed();
            currNode = 0;
            currNode = nodesOrelIter->GetNextTuple();
        }

        delete nodesOrelIter;



        #ifdef USEPERF
        LogPerf("perf: total labelSizeForward: " << totalLabelSizeForward
         << " totalLabelSizeReverse: " << totalLabelSizeReverse
         << " nodesCount: " <<  nodesCount << " average labelSizeForward: "
         << totalLabelSizeForward / nodesCount << " average labelSizeReverse: "
         << totalLabelSizeReverse / nodesCount << " average labelSizeBoth: "
         << (totalLabelSizeForward + totalLabelSizeReverse) / (nodesCount * 2)
         << endl);
        #endif


        #ifdef USEDEBUG
        LogDebug("finish hlCreateLabelsFromHlGraph" << endl);
        #endif
    }










/*

3.4 HubLabeling functionality of (3) In-Memory Approach using MainMemory2Algebra

*/





    /*
     *  TODO: function for doing ContractionHierarchies
     *   using graph from existing Algebra MainMemory2
     *   and its shortestPath implementations
     *  Not working yet.
    */
    bool hlContractMmGraph(graph::Graph* graphUp, graph::Graph* graphDown,
        Word calcFunction) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlContractMmGraph" << endl);
        #endif

        graph::Queue rankQueue;

        //it doesnt matter here which graph to use, both have the same nodes
        set<graph::Vertex*,graph::Vertex::EqualVertex>::iterator vertexIt =
                                            graphUp->getGraph()->begin();
        //iterate over all nodes
        int currRank = 1;
        std::multimap<int, int> ranksNodesMultimap;
        while(vertexIt != graphUp->getGraph()->end()) {
            graph::Vertex* v = *vertexIt;

            ranksNodesMultimap.insert(pair<int, int>(currRank, v->getNr()));

            vertexIt++;
            currRank++;
        }

        //iterate over all nodes v in ranked order
        while(!ranksNodesMultimap.empty())
        {
            //get first element of multimap
            std::multimap<int,int>::iterator rankIter =
             ranksNodesMultimap.begin();

            //int rankV = (*rankIter).first;
            int vId = (*rankIter).second;

            //remove node from multimap its not used any more
            ranksNodesMultimap.erase(rankIter);

            //get incoming edges (u, v) to v
            graph::Vertex* vDown = graphDown->getVertex(vId);
            for(size_t iDown = 0; iDown < vDown->getEdges()->size(); iDown++)
            {
                //get u from ! graphUp for doing forward searches
                // but reach u via ! downGraph
                vector<graph::EdgeWrap>* edgesDown = vDown->getEdges();
                int uId = edgesDown->at(iDown).getPointer()->getDest();
                graph::Vertex* uUp = graphUp->getVertex(uId);

                //skip if u does not exist in rankMultimap
                // it then still must have been contracted
                // we use function to get the nodes rank
                // which is -1 if the node does not exist
                int rankU = hlGetRankOfNode(ranksNodesMultimap, uId);
                if(rankU > -1)
                {
                    graph::Vertex* vUp = graphUp->getVertex(vId);
                    for(size_t iUp = 0; iUp < vUp->getEdges()->size(); iUp++)
                    {
                        //get w from ! downGraph for doing reverse searches
                        // but reach w via ! UpGraph
                        vector<graph::EdgeWrap>* edgesUp = vUp->getEdges();
                        int wId = edgesUp->at(iUp).getPointer()->getDest();
                        //graph::Vertex* wDown = graphDown->getVertex(wId);

                        //skip if w does not exist in rankMultimap
                        // it then still must have been contracted
                        // we use function to get the nodes rank
                        // which is -1 if the node does not exist
                        int rankW = hlGetRankOfNode(ranksNodesMultimap, wId);
                        if(rankW > -1)
                        {
                            graph::Vertex* wUp = graphUp->getVertex(wId);

                            //do local query from u to w
                            //TODO wie dijkstra aufrufen von hier aus?
                            bool dijkstraFound = mm2algebra::dijkstra(graphUp,
                             calcFunction, uUp, wUp);

                            if(!dijkstraFound)
                            {
                                #ifdef USEDEBUG
                                LogDebug("no shortest path found" << endl);
                                #endif
                            }

                            //retrieve distance of previous local query
                            double distUW = wUp->getCost();

                            //calc shortcutLength for UV
                            //note that down is the same as up but only
                            // with invertes edges, means
                            // dist(u, v) \in up = dist(v, u) \in down
                            double distUV = hlGetCostOfEdgeUV(
                                vDown, iDown, calcFunction);

                            //calc shortcutLength for UV
                            double distVW = hlGetCostOfEdgeUV(
                                vUp, iUp, calcFunction);

                            double distUVWShortcut = distUV + distVW;

                            //if shortcut is shorter then create shortcu-edge
                            if(distUVWShortcut < distUW)
                            {
                                //add edge (u, w) to up
                                // and inverted edge (w, u) to down
                                hlCreateShortcutEdgeTuple(graphUp, uId, wId,
                                 distUVWShortcut, vId);
                                hlCreateShortcutEdgeTuple(graphDown, wId, uId,
                                 distUVWShortcut, vId);
                            }
                        }
                    }
                }
            }
        }

        #ifdef USEDEBUG
        LogDebug("finish hlContractMmGraph" << endl);
        #endif

        return true;
    }

    /*
     * retrieves the costs of an edge (u, v) of an graph:Graph
     *
     * @param vertexU is the given graph::Vertex* u
     * @param vIndex is the index position of the edge which leads to vertex v
     * @param calcFunction is the Function used to calculate the cost
     *
     * @return the calculated costs of the given edge
    */
    double hlGetCostOfEdgeUV(graph::Vertex* vertexU, int vIndex,
        Word calcFunction) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlGetCostOfEdgeUV" << endl);
        #endif
        //TODO: costFunction auf festes Feld umbauen,
        // damit Shortcutsunterstützt werden können?
        // ansonsten müsste für diese ja auch die curve definiert werden

        double cost = -1.0;

        ArgVectorPointer funArgs = qp->Argument(calcFunction.addr);
        Word funResult;
        ((*funArgs)[0]).setAddr(
            vertexU->getEdges()->at(vIndex).getPointer()->getTuple()
        );
        qp->Request(calcFunction.addr,funResult);
        cost = ((CcReal*)funResult.addr)->GetRealval();

        #ifdef USEDEBUG
        LogDebug("finish hlGetCostOfEdgeUV" << endl);
        #endif

        return cost;
    }

    /*
     * Retrieves the Rank for the given nodeId
     *
     * @param currMultimap the map containing all nodes with their ranks
     *         ordered by nodeId
     * @param nodeId
     * @return the rank if found, -1 telse
    */
    int hlGetRankOfNode(std::multimap<int, int> currMultimap,
     int nodeId) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlGetRankOfNode" << endl);
        #endif

        int rankValue = -1;
        std::multimap<int,int>::iterator multimapIter =
         currMultimap.find(nodeId);

        if(multimapIter != currMultimap.end())
        {
            rankValue = (*multimapIter).second;
        }

        #ifdef USEDEBUG
        LogDebug("finish hlGetRankOfNode" << endl);
        #endif

        return rankValue;
    }

    /*
     * Creates an Shortcut Tuple (u, v, w) and adds it to the given graph
     * Uses @see HubLabelClass::hlGetEdgesTupleTypeInfo()
     *
     * @param graph where the edge shall be added
     * @param source the index of the Attribute containing the sourceId u
     * @param target the index of the Attribute containing the targetId w
     * @param costs the costs of the shortcutEdge to be created
     * @param parentNode the nodeId of the parent v
     *
    */
    void hlCreateShortcutEdgeTuple(graph::Graph* graph, int source,
        int target, double costs, int parentNode) const
    {
        #ifdef USEDEBUG
        LogDebug("start hlCreateShortcutEdgeTuple" << endl);
        #endif

        ListExpr tupleType;
        nl->ReadFromString(HubLabelClass::hlGetEdgesTupleTypeInfo(), tupleType);
        ListExpr tupNumType =
            SecondoSystem::GetCatalog()->NumericType(tupleType);
        Tuple* insertTuple = new Tuple(tupNumType);
        insertTuple->PutAttribute(
            HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE, new CcInt(true, source));
        insertTuple->PutAttribute(
            HL_INDEX_OF_TARGET_IN_EDGE_TUPLE, new CcInt(true, target));
        insertTuple->PutAttribute(
            HL_INDEX_OF_SOURCE_POS_IN_EDGE_TUPLE,
            new Point(false, 0.0, 0.0));
        insertTuple->PutAttribute(
            HL_INDEX_OF_TARGET_POS_IN_EDGE_TUPLE,
            new Point(false, 0.0, 0.0));
        insertTuple->PutAttribute(
            HL_INDEX_OF_SOURCE_NODE_COUNTER_IN_EDGE_TUPLE,
            new CcInt(false));
        insertTuple->PutAttribute(
            HL_INDEX_OF_TARGET_NODE_COUNTER_IN_EDGE_TUPLE,
            new CcInt(false));
        insertTuple->PutAttribute(
            HL_INDEX_OF_CURVE_IN_EDGE_TUPLE,
            new SimpleLine(0));
        insertTuple->PutAttribute(
            HL_INDEX_OF_ROAD_NAME_IN_EDGE_TUPLE,
            new FText(false));
        insertTuple->PutAttribute(
            HL_INDEX_OF_ROAD_TYPE_IN_EDGE_TUPLE,
            new FText(false));
        insertTuple->PutAttribute(
            HL_INDEX_OF_WAY_ID_IN_EDGE_TUPLE,
            new LongInt(false));
        insertTuple->PutAttribute(
            HL_INDEX_OF_COSTS_IN_EDGE_TUPLE, new CcReal(true, costs));
        insertTuple->PutAttribute(
            HL_INDEX_OF_PARENT_ID_IN_EDGE_TUPLE,
            new CcInt(true, parentNode));

        //TODO sinnvoll, dass hier cost gesetzt wird?
        // oder ist das nur relevant für die dijkstrasuche?
        graph->addEdge(insertTuple,
            HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE,
            HL_INDEX_OF_TARGET_IN_EDGE_TUPLE,
            costs, 0.0);

        #ifdef USEDEBUG
        LogDebug("finish hlCreateShortcutEdgeTuple" << endl);
        #endif
    }





private:
    double x;
    double y;
    double r;

    int progressInterval;
};


/*
4 HubLabeling Secondo-Type

In the following sections show the implementation of
the HubLabeling Secondo-Type
related to the class HubLabelClass above.

4.1 The Property Function

The ~Property~ function provides a description of the Secondo type
to the user. It returns a nested list, which must be have exactly
the format given in this example. The first element of the list
is always the same and the second element of the list contains
type specific descriptions.

*/

ListExpr HubLabelClassProperty()
{
    return ( nl->TwoElemList (
                 nl->FourElemList (
                     nl->StringAtom("Signature"),
                     nl->StringAtom("Example Type List"),
                     nl->StringAtom("List Rep"),
                     nl->StringAtom("Example List")),
                 nl->FourElemList (
                     nl->StringAtom("-> SIMPLE"),
                     nl->StringAtom(HubLabelClass::BasicType()),
                     nl->StringAtom("(real real real) = (x,y,r)"),
                     nl->StringAtom("(13.5 -76.0 1.0)")
                 )));
}

/*
4.2 In Function

For the creation of a constant value within a query and for
importing objects or whole databases from a file, object
values are described by nested lists. The task of the ~IN~-function
is to convert such a list into the internal object representation, i.e.
into an instance of the class above.
The list may have an  invalid format. If the list does not
have the expected format, the output parameter ~correct~ must be set to
~false~ and the ~addr~-pointer of the result must be set to 0.
A detailed error description can be provided to the user by calling
the ~inFunError~ of the global ~cmsg~ object.
In case of success, the argument ~correct~ has to be set to ~true~ and
the ~addr~ pointer of the result points to an object instance having the
value represented by the ~instance~ argument.
The parameters of the function are:

  * ~typeInfo~: contains the complete type description and is required
   for nested types
    like tuples

  * ~instance~: the value of the object in external (nested list) representation

  * ~errorPos~: output parameter reporting the position of an error within
   the list (set types)

  * ~errorInfo~: can provide information about an error to the user

  * ~correct~: output parameter returning the success of this call



*/

Word InHubLabelClass( const ListExpr typeInfo,
                      const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo,
                      bool& correct )
{
    // create a result with addr pointing to 0
    Word res((void*)0);
    // assume an incorrect list
    correct = false;
    // check whether the list has three elements
    if(!nl->HasLength(instance,3))
    {
        cmsg.inFunError("expected three numbers");
        return res;
    }
    // check whether all elements are numeric
    if(   !listutils::isNumeric(nl->First(instance))
            || !listutils::isNumeric(nl->Second(instance))
            || !listutils::isNumeric(nl->Third(instance)))
    {
        cmsg.inFunError("expected three numbers");
        return res;
    }
    // get the numeric values of the elements
    double x = listutils::getNumValue(nl->First(
                                          instance));
    double y = listutils::getNumValue(nl->Second(
                                          instance));
    double r = listutils::getNumValue(nl->Third(
                                          instance));
    // check for a valid radius
    if(r<=0)
    {
        cmsg.inFunError("invalid radius (<=0)");
        return res;
    }
    // list was correct,  create the result
    correct = true;
    res.addr = new HubLabelClass(x,y,r);
    return res;
}

/*
4.3 Out Function

This function is used to create the external representation of an object
as nested list. Note that the ~IN~ function must be able to read in the
result of this function.  The arguments are:

  * ~typeInfo~: nested list representing the type of the object (required for
              complex types)

  * ~value~: the ~addr~ pointer of ~value~ points to the object to export.
   The Secondo
    framework ensures that the type of this object is the correct one. The
    cast in the first line will be successful.


This function must be able to convert *each* instance into a nested list.
 For this
reason, there is no function for error reporting as in the ~IN~ function.


*/
ListExpr OutHubLabelClass( ListExpr typeInfo,
                           Word value )
{
    HubLabelClass* k = (HubLabelClass*) value.addr;
    return nl->ThreeElemList(
               nl->RealAtom(k->getX()),
               nl->RealAtom(k->getY()),
               nl->RealAtom(k->getR()));
}

/*
4.4 Create Function

This function creates an object instance having an arbitrary value.
 The ~typeInfo~
argument represents the type of the object and is required for nested types like
tuples.

*/
Word CreateHubLabelClass( const ListExpr
                          typeInfo )
{
    Word w;
    w.addr = (new HubLabelClass(0,0,1.0));
    return w;
}

/*
2.4 Delete Function

Removes the complete object (inclusive disc parts if there are any, see section
\ref{largeStructures}).
The Secondo framework ensures that the type behind the ~addr~ pointer
of ~w~ is the expected one. The arguments are:

  * ~typeInfo~: the type description (for complex types)

  * ~w~: the ~addr~ pointer of this argument points to the object to delete.

*/
void DeleteHubLabelClass( const ListExpr typeInfo,
                          Word& w )
{
    HubLabelClass *k = (HubLabelClass *)w.addr;
    delete k;
    w.addr = 0;
}

/*
4.5 Open Function

Reads an object from disc  via an ~SmiRecord~.

  * ~valueRecord~: here, the disc representation of the object is stored

  * ~offset~: the object representation starts here in ~valueRecord~ After
    the call of this function, ~offset~ must be after the object's value

  * ~typeInfo~: the type description (required for complex types)

  * ~value~: output argument

The function reads data out of the SmiRecord and creates a new object from them
in case of success. The created object is stored in the ~addr~-pointer of the
~value~ argument. In the case of an error, the ~addr~ pointer has to be set to
~NULL~. The result of this functions reports the success of reading.
To implement this function, the function ~Read~ of the ~SmiRecord~ is used.
Its first argument is a  pointer to the storage where the data should be
written. The second argument determines how may data should be transferred
from the record to the buffer. The last argument indicates the position
of the data within the record. The return value of this function corresponds
to the actual read amount of data. In case of success, this number is
the same as given in the second argument.

*/
bool OpenHubLabelClass( SmiRecord& valueRecord,
                        size_t& offset, const ListExpr typeInfo,
                        Word& value )
{
    size_t size = sizeof(double);
    double x,y,r;
    bool ok = (valueRecord.Read(&x,size,
                                offset) == size);
    offset += size;
    ok = ok &&
         (valueRecord.Read(&y,size,offset) == size);
    offset += size;
    ok = ok &&
         (valueRecord.Read(&r, size, offset) == size);
    offset += size;
    if(ok)
    {
        value.addr = new HubLabelClass(x,y,r);
    }
    else
    {
        value.addr = 0;
    }
    return ok;
}

/*
4.6 Save Function

Saves an object to disc (via SmiRecord). This function has to be symmetrically
to the ~OPEN~ function. The result reports the success of the call.
The arguments are

  * ~valueRecord~: here the object will be stored

  * ~offset~: the object has to be  stored at this position in ~valueRecord~;
   after the call of this
    function, ~offset~ must be after the object's representation

  * ~typeInfo~: type description as a nested list (required for complex types)

  * ~value~: the addr pointer of this argument points to the object to save

The used ~Write~ function of the ~SmiRecord~ works similar to its ~Read~
 function but transfers the
data into the other direction.

*/

bool SaveHubLabelClass( SmiRecord& valueRecord,
                        size_t& offset,
                        const ListExpr typeInfo, Word& value )
{
    HubLabelClass* k = static_cast<HubLabelClass*>
                       ( value.addr );
    size_t size = sizeof(double);
    double v = k->getX();
    bool ok = valueRecord.Write( &v, size, offset );
    offset += size;
    v = k->getY();
    ok = ok && valueRecord.Write(&v,size,offset);
    offset += size;
    v = k->getR();
    ok = ok && valueRecord.Write(&v,size,offset);
    offset += size;
    return ok;
}

/*
4.7 Close Function

Removes the main memory part of an object. In contrast to delete, the
disc part of the object is untouched (if there is one).

  * ~typeInfo~: type description as  a nested list

  * ~w~: the ~addr~ pointer of ~w~ points to the object which is to close

*/
void CloseHubLabelClass( const ListExpr typeInfo,
                         Word& w )
{
    HubLabelClass *k = (HubLabelClass *)w.addr;
    delete k;
    w.addr = 0;
}

/*
4.8 Clone Function

Creates a depth copy (inclusive disc parts) of an object.

  * ~typeInfo~: type description as nested list

  * ~w~: holds a pointer to the object which is to clone

*/
Word CloneHubLabelClass( const ListExpr typeInfo,
                         const Word& w )
{
    HubLabelClass* k = (HubLabelClass*) w.addr;
    Word res;
    res.addr = new HubLabelClass(k->getX(), k->getY(),
                                 k->getR());
    return res;
}

/*
4.9 Cast Function

Casts a void pointer to the type using a special call of new operator.
The argument points to a memory block which is to cast to the object.
The used C++ constructor cannot initialize the object, e.g. the used
constructur must do nothing.

*/
void*  CastHubLabelClass( void* addr )
{
    return (new (addr) HubLabelClass);
}

/*
4.10 Type Check

Checks whether a given list corresponds to the type. This function
is quit similar to the ~checkType~ function within the class.
The result is ~true~ if ~type~ represents a valid type description for
the  type, ~false~ otherwise. The argument ~errorInfo~ can be used
to report an error message to the user.

*/
bool HubLabelClassTypeCheck(ListExpr type,
                            ListExpr& errorInfo)
{
    return nl->IsEqual(type,
                       HubLabelClass::BasicType());
}

/*
4.11 SizeOf Function

Returns the size required to store an instance of this object to disc
using the ~Save~ function from above. Because an ~HubLabelClass~ is represented
by three double numbers, the size is three times the size of a single double.

*/
int SizeOfHubLabelClass()
{
    return 3*sizeof(double);
}

/*
4.12 The TypeConstructor Instance

We define a Secondo type by creating an instance of  ~TypeConstructor~
 feeded with
the functions defined before.

*/

TypeConstructor HubLabelClassTC(
    HubLabelClass::BasicType(),        // name of the type
    HubLabelClassProperty,             // property function
    OutHubLabelClass,
    InHubLabelClass,        // out and in function
    0, 0,                       // deprecated, don't think about it
    CreateHubLabelClass,
    DeleteHubLabelClass, // creation and deletion
    OpenHubLabelClass,
    SaveHubLabelClass,     // open and save functions
    CloseHubLabelClass,
    CloneHubLabelClass,   // close and clone functions
    CastHubLabelClass,                 // cast function
    SizeOfHubLabelClass,               // sizeOf function
    HubLabelClassTypeCheck);           // type checking function






/*
5 Operator Implementation

The following sections show the implementation of the HubLabeling
 Secondo-Operatoprrs
used to perform the functionalities of the class HubLabelClass.
Most of the operators are used mainly for testing the functionality.

Main-Use-Cases and the equivalent operators are:
-A given Graph can be modified with auxiliary attributes
  - hlCalcRankOp
-The graph can be contracted
  - hlIterateOverAllNodesByRankAscAndDoContractionOp
-The graph can be divided into upwards- and downwards graphs
-HubLabels can be created
  - hlCreateLabels
-A HubLabeling Search can be performed
  - hlQuery
For all steps the the equivalent Secondo-Script for full execution
 of creating HubLabeling Labels


Each operator implementation in Secondo contains of a type mapping,
a set of value mappings, a selection function,
a description, and  a creation of an operator instance.

Furthermore, the syntax of the operator is described in the file
~HubLabeling.spec~ and at least one example must be given in the file
~HubLabeling.examples~. If there is no example, the operator will be
switched off by the Secondo framework.


The following sections present the implementation ~HubLabelClass~.

5.1 Type Mapping

The type mapping gets a nested list containing the argument types
for this operator. Within the implementation, the number and the
types of the arguments are checked to be a valid input for the
operator. If the argument types cannot be handled by this operator,
a type error is returned. Otherwise, the result of this function
is the result type of the operator in nested list format.

All TypeMappings are related to the equal named function of the
 class HubLabelClass.
The TypeMapping just got an additional suffix 'TM'.

5.1.1 Type Mappings of (1) Persistent Approach

*/


ListExpr hlCalcRankTM(ListExpr args)
{
    string err =
        "2x orderedRelation and one CcInt expected.";

    // check the number of arguments
    if(!nl->HasLength(args,3))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    if(!CcInt::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no CcInt)");
    }

    // return the result type
    return listutils::basicSymbol<CcReal>();
}



ListExpr hlOneHopReverseSearchTM(ListExpr args)
{
    string err =
        "2x orderedRelation and one CcInt expected.";

    // check the number of arguments
    if(!nl->HasLength(args,3))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    if(!CcInt::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no CcInt)");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetOneHopReverseOrelIdTypeInfo(),
        relType);

    return relType;
}



ListExpr hlHHopForwardSearchTM(ListExpr args)
{
    string err =
        "3x orderedRelation and 2x CcInt expected.";

    // check the number of arguments
    if(!nl->HasLength(args,5))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    if(!OrderedRelation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no OrderedRelation)");
    }

    if(!CcInt::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no CcInt)");
    }

    if(!CcInt::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcInt)");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetShortcutsCreatedSourceTypeInfo(),
        relType);

    return relType;
}



ListExpr hlForwardSearchGetDistTM(ListExpr args)
{
    string err =
        "1x orderedRelation and 2x CcInt expected.";

    // check the number of arguments
    if(!nl->HasLength(args,3))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    if(!CcInt::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no CcInt)");
    }

    if(!CcInt::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no CcInt)");
    }

    // return the result type
    return listutils::basicSymbol<CcReal>();
}



ListExpr hlRemoveTFromCurrentWitnessListTM(
    ListExpr args)
{
    string err = "1x Orel and 1x CcInt expected.";

    // check the number of arguments
    if(!nl->HasLength(args,2))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no Orel)");
    }

    if(!CcInt::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no CcInt");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetOneHopReverseOrelIdTypeInfo(),
        relType);

    return relType;
}



ListExpr hlForwardSearchCheckForWitnessPathTM(
    ListExpr args)
{
    string err =
        "1x Orel, 2x CcInt, 2x CcReal expected.";

    // check the number of arguments
    if(!nl->HasLength(args,5))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no Orel)");
    }

    if(!CcInt::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no CcInt");
    }

    if(!CcInt::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no CcInt");
    }

    if(!CcReal::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no CcReal");
    }

    if(!CcReal::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcReal");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetOneHopReverseOrelIdTypeInfo(),
        relType);
    //TODO: warum hier kein numtype, im code aber schon?

    return relType;
}



ListExpr hlInsertOrUpdateTupleInNotYetVisitedListTM(
    ListExpr args)
{
    string err = "1x rel, 3x CcInt expected.";

    // check the number of arguments
    if(!nl->HasLength(args,4))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!Relation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no rel)");
    }

    if(!CcInt::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no CcInt");
    }

    if(!CcInt::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no CcInt");
    }

    if(!CcInt::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no CcInt");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetNotYetVisitedNodesTypeInfo(),
        relType);

    return relType;
}



ListExpr hlForwardSearchIterativeStepsScanNewVerticesTM(
    ListExpr args)
{
    string err =
        "4x Orel, 1x Rel, 1 CcInt, 1x CcReal expected.";

    // check the number of arguments
    if(!nl->HasLength(args,7))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no orel)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no orel)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no orel)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no orel)");
    }

    // check type of the argument
    if(!Relation::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no rel)");
    }

    if(!CcInt::checkType(nl->Sixth(args)))
    {
        return listutils::typeError(err +
                                    " (sixth arg no CcInt");
    }

    if(!CcReal::checkType(nl->Seventh(args)))
    {
        return listutils::typeError(err +
                                    " (seventh arg no CcReal");
    }

    // return the result type
    return listutils::basicSymbol<CcInt>();
}



ListExpr hlForwardSearchProcessIncomingEdgeIterativeStepsTM(
    ListExpr args)
{
    string err =
        "4x Orel, 2x CcInt, 1x CcReal expected.";

    // check the number of arguments
    if(!nl->HasLength(args,7))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no orel)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no orel)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no orel)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no orel)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcInt)");
    }

    if(!CcInt::checkType(nl->Sixth(args)))
    {
        return listutils::typeError(err +
                                    " (sixth arg no CcInt");
    }

    if(!CcReal::checkType(nl->Seventh(args)))
    {
        return listutils::typeError(err +
                                    " (seventh arg no CcReal");
    }

    // return the result type
    return listutils::basicSymbol<CcInt>();
}



ListExpr hlForwardSearchProcessIncomingEdgeInitialStepsTM(
    ListExpr args)
{
    string err =
        "3x Orel, 2x CcInt, 1x CcReal expected.";

    // check the number of arguments
    if(!nl->HasLength(args,6))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no orel)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no orel)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no orel)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no CcInt)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcInt)");
    }

    if(!CcReal::checkType(nl->Sixth(args)))
    {
        return listutils::typeError(err +
                                    " (sixth arg no CcReal");
    }

    // return the result type
    return listutils::basicSymbol<CcInt>();
}



ListExpr hlForwardSearchCreateAndAppendShortcutsTM(
    ListExpr args)
{
    string err =
        "1x Orel, 2x CcInt, 1x CcReal expected.";

    // check the number of arguments
    if(!nl->HasLength(args,4))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no orel)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no CcInt)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no CcInt)");
    }

    // check type of the argument
    if(!CcReal::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no CcReal)");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetShortcutsCreatedSourceTypeInfo(),
        relType);

    return relType;
}



ListExpr hlForwardSearchProcessIncomingEdgeTM(
    ListExpr args)
{
    string err =
        "4x Orel, 3x CcInt, 1x CcReal expected.";

    // check the number of arguments
    if(!nl->HasLength(args,8))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no OrderedRelation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcInt)");
    }

    if(!CcInt::checkType(nl->Sixth(args)))
    {
        return listutils::typeError(err +
                                    " (sixth arg no CcInt");
    }

    if(!CcInt::checkType(nl->Seventh(args)))
    {
        return listutils::typeError(err +
                                    " (seventh arg no CcInt");
    }

    if(!CcReal::checkType(nl->Eigth(args)))
    {
        return listutils::typeError(err +
                                    " (eighth arg no CcReal");
    }

    // return the result type
    return listutils::basicSymbol<CcInt>();
}



ListExpr hlRemoveContractedEdgesFromEdgesRelationsTM(
    ListExpr args)
{
    string err = "2x Orel, 1x CcInt expected.";

    // check the number of arguments
    if(!nl->HasLength(args,3))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no CcInt)");
    }

    // return the result type
    return listutils::basicSymbol<CcInt>();
}



ListExpr hlRemoveParallelEdgesFromEdgesRelationsTM(
    ListExpr args)
{
    string err = "3x Orel expected.";

    // check the number of arguments
    if(!nl->HasLength(args,3))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no OrderedRelation)");
    }

    // return the result type
    return listutils::basicSymbol<CcInt>();
}



ListExpr hlDoContractionTM(ListExpr args)
{
    string err = "2x Orel, 2x CcInt expected.";

    // check the number of arguments
    if(!nl->HasLength(args,4))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no CcInt)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no CcInt)");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetShortcutsCreatedSourceTypeInfo(),
        relType);

    return relType;
}



ListExpr hlIterateOverAllNodesByRankAscAndDoContractionTM(
    ListExpr args)
{
    string err = "3x Orel, 1x CcInt expected.";

    // check the number of arguments
    if(!nl->HasLength(args,4))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no OrderedRelation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no CcInt)");
    }

    // return the result type
    return listutils::basicSymbol<CcInt>();
}



ListExpr hlCreateLabelCheckForWitnessScanNewVerticesTM(
    ListExpr args)
{
    string err =
        "2x Orel, 2x CcInt, 1x CcReal, 1xCcInt (bool) expected.";

    // check the number of arguments
    if(!nl->HasLength(args,6))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no CcInt)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no CcInt)");
    }

    // check type of the argument
    if(!CcReal::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcReal)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Sixth(args)))
    {
        return listutils::typeError(err +
                                    " (sixth arg no CcInt (bool))");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetNotYetVisitedNodesTypeInfo(),
        relType);

    return relType;
}



ListExpr hlCreateLabelCheckForWitnessTM(
    ListExpr args)
{
    string err =
        "2x Orel, 1x rel, 2x CcInt, 1x CcReal, 1xCcInt (bool) expected.";

    // check the number of arguments
    if(!nl->HasLength(args,7))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // check type of the argument
    if(!Relation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no Relation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no CcInt)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcInt)");
    }

    // check type of the argument
    if(!CcReal::checkType(nl->Sixth(args)))
    {
        return listutils::typeError(err +
                                    " (sixth arg no CcReal)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Seventh(args)))
    {
        return listutils::typeError(err +
                                    " (seventh arg no CcInt (CcBool))");
    }

    // return the result type
    return listutils::basicSymbol<CcInt>();
}



ListExpr hlCreateLabelScanNewVerticesTM(
    ListExpr args)
{
    string err =
        "2x Orel, 1x rel, 1x CcInt, 1x CcReal, 1x CcInt (CcBool) expected.";

    // check the number of arguments
    if(!nl->HasLength(args,6))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // check type of the argument
    if(!Relation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no Relation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no CcInt)");
    }

    // check type of the argument
    if(!CcReal::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcReal)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Sixth(args)))
    {
        return listutils::typeError(err +
                                    " (sixth arg no CcInt (CcBool))");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetNotYetVisitedNodesTypeInfo(),
        relType);

    return relType;
}



ListExpr hlGetRankByIdTM(ListExpr args)
{
    string err =
        "1x BTree, 1x Orel, 1x CcInt expected.";

    // check the number of arguments
    if(!nl->HasLength(args,3))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!BTree::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no BTree)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no CcInt)");
    }

    // return the result type
    return listutils::basicSymbol<CcInt>();
}



ListExpr hlCreateLabelByDijkstraWithStallingTM(
    ListExpr args)
{
    string err =
        "1x BTree, 3x Orel, 2x CcInt, 1x CcInt (CcBool) expected.";

    // check the number of arguments
    if(!nl->HasLength(args,7))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!BTree::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no BTree)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no OrderedRelation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcInt)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Sixth(args)))
    {
        return listutils::typeError(err +
                                    " (sixth arg no CcInt)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Seventh(args)))
    {
        return listutils::typeError(err +
                                    " (seventh arg no CcInt (CcBool))");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetForwardOrReverseLabelTypeInfo(),
        relType);

    return relType;
}



ListExpr hlFillForwardOrReverseLabelTM(
    ListExpr args)
{
    string err = "1x Rel expected.";

    // check the number of arguments
    if(!nl->HasLength(args,1))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!Relation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no Relation)");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetAllLabelsTypeInfo(), relType);

    return relType;
}



ListExpr hlGetPathViaPointsTM(ListExpr args)
{
    string err =
        "1x Nrel, 1x BTree, 1x Orel, 2x CcInt, 1x CcInt (CcBool) expected.";

    // check the number of arguments
    if(!nl->HasLength(args,6))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!NestedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no NestedRelation)");
    }

    // check type of the argument
    if(!BTree::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no BTree)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no OrderedRelation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no CcInt)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcInt)");
    }


    // check type of the argument
    if(!CcInt::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (sixth arg no CcInt (CcBool))");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetEdgesRelTypeInfo(), relType);

    return relType;
}



ListExpr hlQueryTM(ListExpr args)
{
    string err =
        "1x Nrel, 1x BTree, 2x Orel, 2x CcInt, 1x CcInt (CcBool) expected.";

    // check the number of arguments
    if(!nl->HasLength(args,7))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!NestedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no NestedRelation)");
    }

    // check type of the argument
    if(!BTree::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no BTree)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no OrderedRelation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcInt)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Sixth(args)))
    {
        return listutils::typeError(err +
                                    " (sixth arg no CcInt)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Seventh(args)))
    {
        return listutils::typeError(err +
                                    " (seventh arg no CcInt)");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetEdgesRelTypeInfo(), relType);

    return relType;
}



ListExpr hlPruneLabelByBootstrappingTM(
    ListExpr args)
{
    string err = "3x Orel, 2x CcInt expected.";

    // check the number of arguments
    if(!nl->HasLength(args,5))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no OrderedRelation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no CcInt)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcInt)");
    }

    // return the result type
    return listutils::basicSymbol<CcInt>();
}



ListExpr hlReorderLabelsTM(ListExpr args)
{
    string err = "3x Orel, 2x CcInt expected.";

    // check the number of arguments
    if(!nl->HasLength(args,5))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no OrderedRelation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no CcInt)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcInt)");
    }

    // return the result type
    return listutils::basicSymbol<CcInt>();
}



ListExpr hlCreateLabelsTM(ListExpr args)
{
    string err =
        "1x BTree, 3x Orel, 1x CcInt expected.";

    // check the number of arguments
    if(!nl->HasLength(args,5))
    {
        return listutils::typeError(err +
                                    " (wrong number of arguments)");
    }

    // check type of the argument
    if(!BTree::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no BTree)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no OrderedRelation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcInt)");
    }

    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetAllLabelsTypeInfo(), relType);

    return relType;
}



/*
5.1.2 Type Mappings of (2) In-Memory Approach

*/


ListExpr hlTransformOrelToHlGraphTM(ListExpr args)
{
    #ifdef USEDEBUG
    LogDebug("start hlTransformOrelToHlGraphTM" << endl);
    #endif



    string err = "2x orderedRelation";

    // check the number of arguments
    if(!nl->HasLength(args,2))
    {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetHlGraphOrelTypeInfo(),
        relType);

    #ifdef USEDEBUG
    LogDebug("finish hlTransformOrelToHlGraphTM" << endl);
    #endif

    return relType;
}



ListExpr hlDoContractionOfHlGraphTM(ListExpr args)
{
    #ifdef USEDEBUG
    LogDebug("start hlDoContractionOfHlGraphTM" << endl);
    #endif



    string err = "2x orderedRelation, 3x Int expected";

    // check the number of arguments
    if(!nl->HasLength(args,5))
    {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no Int)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no Int)");
    }

    // check type of the argument
    if(!CcInt::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no Int)");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetHlGraphOrelTypeInfo(),
        relType);

    #ifdef USEDEBUG
    LogDebug("finish hlDoContractionOfHlGraphTM" << endl);
    #endif

    return relType;
}



ListExpr hlDoChSearchInHlGraphTM(ListExpr args)
{
    #ifdef USEDEBUG
    LogDebug("start hlDoChSearchInHlGraphTM" << endl);
    #endif



    string err = "3x orel, 2x int expectd";

    // check the number of arguments
    if(!nl->HasLength(args,5))
    {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }
    if(!OrderedRelation::checkType(nl->Third(args)))
    {
        return listutils::typeError(err +
                                    " (third arg no OrderedRelation)");
    }
    if(!CcInt::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fourth arg no Int)");
    }
    if(!CcInt::checkType(nl->Fifth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no Int)");
    }

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetEdgesRelTypeInfo(),
        relType);

    #ifdef USEDEBUG
    LogDebug("finish hlDoChSearchInHlGraphTM" << endl);
    #endif

    return relType;
}



ListExpr hlCalcWeightsOrelTM(ListExpr args)
{
    #ifdef USEDEBUG
    LogDebug("start hlCalcWeightsOrelTM" << endl);
    #endif



    string err = "1x orel, 1x int expected";

    // check the number of arguments
    if(!nl->HasLength(args,2))
    {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }
    if(!CcInt::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no Int)");
    }

    #ifdef USEDEBUG
    LogDebug("finish hlCalcWeightsOrelTM" << endl);
    #endif

    // return the result type
    return listutils::basicSymbol<CcInt>();
}



ListExpr hlCreateLabelsFromHlGraphTM(ListExpr args)
{
    #ifdef USEDEBUG
    LogDebug("start hlCreateLabelsFromHlGraphTM" << endl);
    #endif



    string err = "2x orel expected";

    // check the number of arguments
    if(!nl->HasLength(args,2))
    {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->First(args)))
    {
        return listutils::typeError(err +
                                    " (first arg no OrderedRelation)");
    }

    // check type of the argument
    if(!OrderedRelation::checkType(nl->Second(args)))
    {
        return listutils::typeError(err +
                                    " (second arg no OrderedRelation)");
    }

    #ifdef USEDEBUG
    LogDebug("finish hlCreateLabelsFromHlGraphTM" << endl);
    #endif

    // return the result type
    ListExpr relType;
    nl->ReadFromString(
        HubLabelClass::hlGetAllLabelsTypeInfo(), relType);

    return relType;
}







/*
5.1.3 Type Mappings of (3) In-Memory Approach using MainMemory2Algebra

not yet implemented

*/

/*
  TODO: functions for doing ContractionHierarchies
   using graph from existing Algebra MainMemory2
   and its shortestPath implementations
  Not working yet.


*/
ListExpr hlContractMmGraphTM(ListExpr args)
{
    #ifdef USEDEBUG
LogDebug("start contractNewTM" << endl);
#endif

    //overall
    if(nl->ListLength(args) != 3) {
      return listutils::typeError("four arguments expected");
    }

    //first
    ListExpr first = nl->First(args);

    if(!nl->HasLength(first,2)){
      return listutils::typeError("internal error");
    }

    string errMsg;

    if(!mm2algebra::getMemType(nl->First(first),
     nl->Second(first), first, errMsg))
    return listutils::typeError("\n problem in first arg: " + errMsg);

    ListExpr graph = nl->Second(first); // remove leading mem
    if(!mm2algebra::MemoryGraphObject::checkType(graph))
    return listutils::typeError("first arg is not a mem graph");

    graph = nl->Second(graph);

    if(!listutils::isTupleDescription(graph)) {
    return listutils::typeError(
                       "second value of graph is not of type tuple");
    }

    ListExpr relAttrList(nl->Second(graph));

    if(!listutils::isAttrList(relAttrList)) {
    return listutils::typeError("Error in rel attrlist.");
    }

    if(!(nl->ListLength(relAttrList) >= 3)) {
    return listutils::typeError("rel has less than 3 attributes.");
    }




    //second
    ListExpr second = nl->First(args);

    if(!nl->HasLength(second,2)){
      return listutils::typeError("internal error");
    }

    if(!mm2algebra::getMemType(nl->First(second),
     nl->Second(second), second, errMsg))
    return listutils::typeError("\n problem in second arg: " + errMsg);

    ListExpr graph2 = nl->Second(second); // remove leading mem
    if(!mm2algebra::MemoryGraphObject::checkType(graph2))
    return listutils::typeError("second arg is not a mem graph2");


    graph2 = nl->Second(graph2);

    if(!listutils::isTupleDescription(graph2)) {
    return listutils::typeError(
                       "second value of graph2 is not of type tuple");
    }

    ListExpr relAttrList2(nl->Second(graph2));

    if(!listutils::isAttrList(relAttrList2)) {
    return listutils::typeError("Error in rel attrlist.");
    }

    if(!(nl->ListLength(relAttrList2) >= 3)) {
    return listutils::typeError("rel has less than 3 attributes.");
    }


    //TODO nur graph nicht aber graph2 im Folgenden berücksichtigt

    //Check of third argument

    ListExpr map = nl->First(nl->Third(args));
    if(!listutils::isMap<1>(map)) {
    return listutils::typeError("fourth argument should be a map");
    }

    ListExpr mapTuple = nl->Second(map);

    if(!nl->Equal(graph,mapTuple)) {
    return listutils::typeError(
                       "Tuple of map function must match graph tuple");
    }

    ListExpr mapres = nl->Third(map);

    if(!listutils::isSymbol(mapres,CcReal::BasicType())) {
    return listutils::typeError(
                 "Wrong mapping result type for mgshortestpatha");
    }



    // appends Attribute SeqNo to Attributes in orel
    ListExpr rest = nl->Second(graph);
    ListExpr listn = nl->OneElemList(nl->First(rest));
    ListExpr lastlistn = listn;
    rest = nl->Rest(rest);
    while (!(nl->IsEmpty(rest))) {
    lastlistn = nl->Append(lastlistn,nl->First(rest));
    rest = nl->Rest(rest);
    }
    lastlistn = nl->Append(lastlistn,
                        nl->TwoElemList(
                          nl->SymbolAtom("SeqNo"),
                          nl->SymbolAtom(TupleIdentifier::BasicType())));

    #ifdef USEDEBUG
LogDebug("finish contractNewTM" << endl);
#endif

    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                      nl->TwoElemList(
                                        nl->SymbolAtom(Tuple::BasicType()),
                                        listn));
}





/*
5.2 Value Mapping

The value mapping takes values as arguments and computes the result.
If the result of the operator is not a stream as here, the result storage of the
operator tree node must be used for returning the result. The arguments are
provided as an array of ~Word~ holding the arguments in the
~addr~ pointers. The type mapping ensures that only the expected types
are behind these pointers and the cast will be successful.
The parameters ~message~ and ~local~ are used for stream operators only.
The parameter ~s~ is the operator's node within the operator tree.
The result of the operator for non-stream operators is always 0.

The arguments are :

  * ~args~: array with the arguments of the operator

  * ~result~: output parameter, for non stream operators, the
          result storage must be used

  * ~message~: message used by stream operators

  * ~local~: possibility to store the state of an operator, used in
           stream operators

  * ~s~: node of this operator within the operator tree



All ValueMappings are related to the equal named function of the
 class HubLabelClass.
The ValueMappings just got an additional suffix 'VM'.

5.2.1 Value Mappings of (1) Persistent Approach

*/



int hlCalcRankVM (Word* args, Word& result,
                  int message,
                  Word& local, Supplier s)
{

    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* orelEdgesSource =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* orelEdgesTarget =
        (OrderedRelation*) args[1].addr;
    CcInt* nodeId = (CcInt*) args[2].addr;

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcReal* res = (CcReal*)
                  result.addr; // cast the result
    res->Set(true, k->hlCalcRank(orelEdgesSource,
                                 orelEdgesTarget,
                                 HubLabelClass::HL_DEFAULT_CALC_FUNCTION,
                                 nodeId));      // compute and set the result

    //free resources
    delete k;

    return 0;
}



int hlOneHopReverseSearchVM (Word* args,
                             Word& result, int message, Word& local,
                             Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start hlOneHopReverseSearchVM" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* orelEdgesSource =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* orelEdgesTarget =
        (OrderedRelation*) args[1].addr;
    CcInt* nodeId = (CcInt*) args[2].addr;

    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif
    result = qp->ResultStorage(
                 s);       // use the result storage
    OrderedRelation* oneHopReverseOrelId =
        (OrderedRelation*) result.addr; // cast the result

    if(oneHopReverseOrelId->GetNoTuples() > 0)
    {
        //Relation leeren, falls alte Daten aus vorheriger
        // Durchführung noch enthalten sind
        oneHopReverseOrelId->Clear();
    }

    #ifdef USEDEBUG
LogDebug("do hlOneHopReverseSearch" << endl);
#endif
    std::multimap<int, std::tuple<int, double>>
            oneHopReverseMultimap;
    k->hlOneHopReverseSearch(orelEdgesSource,
                             orelEdgesTarget, nodeId, oneHopReverseMultimap);

    #ifdef USEDEBUG
LogDebug("Add elements from oneHopReverseMultimap to oneHopReverseOrelId"
          << endl);
#endif
    for (std::multimap<int, std::tuple<int, double>>::iterator
            oneHopReverseMultimapIter =
                oneHopReverseMultimap.begin();
            oneHopReverseMultimapIter !=
            oneHopReverseMultimap.end();
            ++oneHopReverseMultimapIter)
    {
        std::tuple<int, double> currTuple =
            (*oneHopReverseMultimapIter).second;

        ListExpr relType;
        nl->ReadFromString(
            HubLabelClass::hlGetOneHopReverseOrelIdTypeInfo(),
            relType); //TODO: passt das? das ist ein orel type info,
            // kein tuple type info
        ListExpr tupNumType =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relType));//TODO: passt das?
        Tuple* insertTuple = new Tuple(
            tupNumType);//num type verwenden oder nicht?
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE,
            new CcInt(true,
                      (*oneHopReverseMultimapIter).first));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE,
            new CcInt(true,
                std::get<HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE
                      - 1>(currTuple)));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE,
            new CcReal(true,
                std::get<HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE
                       - 1>(currTuple)));
        oneHopReverseOrelId->AppendTuple(insertTuple);
        insertTuple->DeleteIfAllowed(); //TODO: warum muss der Tuple
        // gelöscht werden? wird er per beim Append geklont? Nicht dass
        // das Löschen hier den Tuple in der Relation zerstört.
    }

    //free resources
    delete k;

    #ifdef USEDEBUG
LogDebug("finish hlOneHopReverseSearchVM" << endl);
#endif

    return 0;
}



int hlHHopForwardSearchVM (Word* args,
                           Word& result, int message, Word& local,
                           Supplier s)
{

    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* orelEdgesSource =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* orelEdgesTarget =
        (OrderedRelation*) args[1].addr;
    OrderedRelation* reverseSearchOrelXT =
        (OrderedRelation*) args[2].addr;
    CcInt* nodeIdV = (CcInt*) args[3].addr;
    CcInt* hHopCcInt = (CcInt*) args[4].addr;

    int hHopInt = hHopCcInt->GetIntval();
    if(hHopInt < 0)
    {
        hHopInt = HubLabelClass::HL_DEFAULT_H_DEPTH;
    }


    result = qp->ResultStorage(
                 s);       // use the result storage
    OrderedRelation*
    shortcutsToBeCreatedOrelSourceToBeDeleted =
        (OrderedRelation*) result.addr; // cast the result

    std::multimap<int, std::tuple<int, double, int>>
            shortcutsMultimap;

    //Convert OrderedRelation to MultiMap
    GenericRelationIterator* reverseSearchOrelXTIter =
        reverseSearchOrelXT->MakeScan();
    Tuple* currTuple1 =
        reverseSearchOrelXTIter->GetNextTuple();

    std::multimap<int, std::tuple<int, double>>
            multimapReverseSearchXT;
    while(currTuple1)
    {
        CcInt* currTupleIdU = (CcInt*)
                              currTuple1->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE);
        CcInt* currTupleIdT = (CcInt*)
                              currTuple1->GetAttribute(
                HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE);
        CcReal* currTupleDistUT = (CcReal*)
                                  currTuple1->GetAttribute(
                        HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE);

        std::tuple<int, double> insertTuple =
            std::make_tuple(currTupleIdT->GetIntval(),
                            currTupleDistUT->GetRealval());
                            //num type verwenden oder nicht?
        multimapReverseSearchXT.insert(
            pair<int, std::tuple<int, double>>
            (currTupleIdU->GetIntval(), insertTuple));

        //Free Outgoing-Iteration
        currTuple1->DeleteIfAllowed();  //TODO: ist delete ok,
        // wenn zuvor einer anderen orel zugewiesen oder wird das
        // tuple aus der neuen orel dann auch gelöscht?
        currTuple1 = 0;
        currTuple1 =
            reverseSearchOrelXTIter->GetNextTuple();
    }
    delete reverseSearchOrelXTIter;

    k->hlHHopForwardSearch(orelEdgesSource,
                           orelEdgesTarget, multimapReverseSearchXT, nodeIdV,
                           shortcutsMultimap,
                           hHopInt); // compute and set the result

    //convert multimap back to OrderedRelation
    for (std::multimap<int, std::tuple<int, double, int>>::iterator
            shortcutsMultimapIter = shortcutsMultimap.begin();
            shortcutsMultimapIter != shortcutsMultimap.end();
            ++shortcutsMultimapIter)
    {
        std::tuple<int, double, int> currTuple2 =
            (*shortcutsMultimapIter).second;

        ListExpr relType;
        nl->ReadFromString(
            HubLabelClass::hlGetShortcutsCreatedSourceTypeInfo(),
            relType); //TODO: passt das? das ist ein orel type info,
            // kein tuple type info
        ListExpr tupNumType =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relType));//TODO: passt das?
        Tuple* insertTuple1 = new Tuple(
            tupNumType);//num type verwenden oder nicht?
        insertTuple1->PutAttribute(
            HubLabelClass::HL_INDEX_OF_SOURCE_IN_SHORTCUT_TUPLE,
            new CcInt(true, (*shortcutsMultimapIter).first));
        insertTuple1->PutAttribute(
            HubLabelClass::HL_INDEX_OF_TARGET_IN_SHORTCUT_TUPLE,
            new CcInt(true,
                std::get<HubLabelClass::HL_INDEX_OF_TARGET_IN_SHORTCUT_TUPLE
                      - 1>(currTuple2)));
        insertTuple1->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_SHORTCUT_TUPLE,
            new CcReal(true,
                std::get<HubLabelClass::HL_INDEX_OF_DIST_IN_SHORTCUT_TUPLE
                       - 1>(currTuple2)));
        insertTuple1->PutAttribute(
            HubLabelClass::HL_INDEX_OF_PARENT_ID_IN_SHORTCUT_TUPLE,
            new CcInt(true,
                std::get<HubLabelClass::HL_INDEX_OF_PARENT_ID_IN_SHORTCUT_TUPLE
                      - 1>(currTuple2)));
        shortcutsToBeCreatedOrelSourceToBeDeleted->AppendTuple(
            insertTuple1);
        insertTuple1->DeleteIfAllowed(); //TODO: warum muss der Tuple gelöscht
        // werden? wird er per beim Append geklont? Nicht dass das Löschen
        // hier den Tuple in der Relation zerstört.
    }

    //free resources
    delete k;

    return 0;
}



int hlForwardSearchGetDistVM (Word* args,
                              Word& result, int message, Word& local,
                              Supplier s)
{
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* reverseSearchEdgeSourceOrelXT =
        (OrderedRelation*) args[0].addr;
    CcInt* currentNodeIdV = (CcInt*) args[1].addr;
    CcInt* currentNodeIdT = (CcInt*) args[2].addr;

    //Convert OrderedRelation to MultiMap
    GenericRelationIterator*
    reverseSearchEdgeSourceOrelXTIter =
        reverseSearchEdgeSourceOrelXT->MakeScan();
    Tuple* currTuple1 =
        reverseSearchEdgeSourceOrelXTIter->GetNextTuple();

    std::multimap<int, std::tuple<int, double>>
            reverseSearchMultimap;
    while(currTuple1)
    {
        CcInt* currTupleIdU = (CcInt*)
                              currTuple1->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE);
        CcInt* currTupleIdT = (CcInt*)
                              currTuple1->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE);
        CcReal* currTupleDistUT = (CcReal*)
                                  currTuple1->GetAttribute(
                        HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE);

        std::tuple<int, double> insertTuple =
            std::make_tuple(currTupleIdT->GetIntval(),
                            currTupleDistUT->GetRealval());
                            //num type verwenden oder nicht?
        reverseSearchMultimap.insert(
            pair<int, std::tuple<int, double>>
            (currTupleIdU->GetIntval(), insertTuple));

        //Free Outgoing-Iteration
        currTuple1->DeleteIfAllowed();  //TODO: ist delete ok, wenn
        // zuvor einer anderen orel zugewiesen oder wird das tuple aus
        // der neuen orel dann auch gelöscht?
        currTuple1 = 0;
        currTuple1 =
            reverseSearchEdgeSourceOrelXTIter->GetNextTuple();
    }
    delete reverseSearchEdgeSourceOrelXTIter;

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcReal* res = (CcReal*)
                  result.addr; // cast the result
    res->Set(true, k->hlForwardSearchGetDistVT(
                 reverseSearchMultimap,
                 currentNodeIdV->GetIntval(),
                 currentNodeIdT->GetIntval()));
                 // compute and set the result

    //free resources
    delete k;

    return 0;
}



int hlRemoveTFromCurrentWitnessListVM (Word* args,
                                       Word& result, int message, Word& local,
                                       Supplier s)
{
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* witnessList = (OrderedRelation*)
                                   args[0].addr;
    CcInt* currNodeIdT = (CcInt*) args[1].addr;

    result = qp->ResultStorage(
                 s);       // use the result storage
    OrderedRelation* reducedWitnessLiestOrel =
        (OrderedRelation*) result.addr; // cast the result

    //insert all tuples from given orel into result orel
    GenericRelationIterator* witnessListIter =
        witnessList->MakeScan();
    Tuple* currTuple =
        witnessListIter->GetNextTuple();

    while(currTuple)
    {
        reducedWitnessLiestOrel->AppendTuple(currTuple);

        //Free Outgoing-Iteration
        currTuple->DeleteIfAllowed();  //TODO: ist delete ok, wenn zuvor
        // einer anderen orel zugewiesen oder wird das tuple aus der
        // neuen orel dann auch gelöscht?
        currTuple = 0;
        currTuple = witnessListIter->GetNextTuple();
    }

    delete witnessListIter;

    k->hlRemoveTuplesFromOrel(reducedWitnessLiestOrel,
                              currNodeIdT->GetIntval(),
                        HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE);

    //free resources
    delete k;

    return 0;
}



int hlForwardSearchCheckForWitnessPathVM (
    Word* args, Word& result, int message,
    Word& local, Supplier s)
{
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* witnessList = (OrderedRelation*)
                                   args[0].addr;
    CcInt* currNodeIdV = (CcInt*) args[1].addr;
    CcInt* currNodeIdX = (CcInt*) args[2].addr;
    CcReal* distSX = (CcReal*) args[3].addr;
    CcReal* distSV = (CcReal*) args[4].addr;

    result = qp->ResultStorage(
                 s);       // use the result storage
    OrderedRelation* reducedWitnessListOrel =
        (OrderedRelation*) result.addr; // cast the result

    //Convert OrderedRelation to MultiMap
    GenericRelationIterator* witnessListIter =
        witnessList->MakeScan();
    Tuple* currTuple =
        witnessListIter->GetNextTuple();

    std::multimap<int, std::tuple<int, double>>
            oneHopReverseMultimap;
    while(currTuple)
    {
        CcInt* currTupleIdU = (CcInt*)
                              currTuple->GetAttribute(
                        HubLabelClass::HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE);
        CcInt* currTupleIdT = (CcInt*)
                              currTuple->GetAttribute(
                        HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE);
        CcReal* currTupleDistUT = (CcReal*)
                                  currTuple->GetAttribute(
                            HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE);

        std::tuple<int, double> insertTuple =
            std::make_tuple(currTupleIdT->GetIntval(),
                            currTupleDistUT->GetRealval());
                            //num type verwenden oder nicht?
        oneHopReverseMultimap.insert(
            pair<int, std::tuple<int, double>>
            (currTupleIdU->GetIntval(), insertTuple));

        //Free Outgoing-Iteration
        currTuple->DeleteIfAllowed();  //TODO: ist delete ok, wenn
        // zuvor einer anderen orel zugewiesen oder wird das tuple aus
        // der neuen orel dann auch gelöscht?
        currTuple = 0;
        currTuple = witnessListIter->GetNextTuple();
    }

    delete witnessListIter;

    k->hlForwardSearchCheckForWitnessPath(
        oneHopReverseMultimap, currNodeIdV->GetIntval(),
        currNodeIdX->GetIntval(), distSX->GetRealval(),
        distSV->GetRealval());

    //convert multimap back to OrderedRelation
    for (std::multimap<int, std::tuple<int, double>>::iterator
            oneHopReverseMultimapIter =
                oneHopReverseMultimap.begin();
            oneHopReverseMultimapIter !=
            oneHopReverseMultimap.end();
            ++oneHopReverseMultimapIter)
    {
        std::tuple<int, double> currTuple =
            (*oneHopReverseMultimapIter).second;

        ListExpr relType;
        nl->ReadFromString(
            HubLabelClass::hlGetOneHopReverseOrelIdTypeInfo(),
            relType); //TODO: passt das? das ist ein orel type info,
            // kein tuple type info
        ListExpr tupNumType =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relType));//TODO: passt das?
        Tuple* insertTuple = new Tuple(
            tupNumType);//num type verwenden oder nicht?
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE,
            new CcInt(true,
                      (*oneHopReverseMultimapIter).first));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE,
            new CcInt(true,
            std::get<HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE
                      - 1>(currTuple)));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE,
            new CcReal(true,
                std::get<HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE
                       - 1>(currTuple)));
        reducedWitnessListOrel->AppendTuple(insertTuple);
        insertTuple->DeleteIfAllowed(); //TODO: warum muss der Tuple
        // gelöscht werden? wird er per beim Append geklont? Nicht dass das
        // Löschen hier den Tuple in der Relation zerstört.
    }

    //free resources
    delete k;

    return 0;
}



/*
 Erwartet eine relEdgesSource bei der die enthaltenen Edges in der
  Reihenfolge  der rel auch in diesem Test-Operator verarbeitet
  werden sollen.
 Der 3. Parameter enthält die ID für den Startknoten, dieser
  erkennt alle Edges der rel,
  die vom Startknoten aus weggehen.

 Folgende Testfälle sollten (jedoch in beliebiger Reihenfolge)
  enthalten sein:

 Die Erste Edge sollte S->V sein, es wird erwartet, dass diese nicht
  eingefügt wird.
 Die zweite Edge sollte S->W sein, es wird erwartet, dass diese eingefügt
  wird.
 Die dritte Edge sollte W->X sein, es wird erwartet, dass diese eingefügt
  wird.
 Die folgenden Edges sollten X1->X2 sein, es wird erwartet, dass diese
  eingefügt werden.
 Die nächste Edge sollte X3->X4 sein, wobei X4 bereits in der Orel enthalten
  sein soll (hier U genannt),
  der Weg S->X4 sollte länger sein, als der bisherige Weg S->U, es erwartet,
  dass diese edge nicht eingefügt wird.
 Die folgenden Edges sollten X1->X2 sein, es wird erwartet, dass diese
  eingefügt werden.
 Die nächste Edge sollte X5->X6 sein, wobei X6 bereits in der Orel
  enthalten sein soll (hier U2 genannt),
  der Weg S->X6 sollte kürzer sein, als der bisherige Weg S->U2,
  es erwartet, dass diese edge eingefügt wird.
 Es sollten keine weiteren Edges im Input enthalten sein.
  Die Orel wird an die Query zurückgegeben.


*/
int hlInsertOrUpdateTupleInNotYetVisitedListVM (
    Word* args, Word& result,
    int message, Word& local, Supplier s)
{
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    Relation* edgesRelSource = (Relation*)
                               args[0].addr;
    CcInt* currNodeIdS = (CcInt*) args[1].addr;
    CcInt* currNodeIdV = (CcInt*) args[2].addr;
    CcBool* isForward = (CcBool*) args[3].addr;

    result = qp->ResultStorage(
                 s);       // use the result storage
    OrderedRelation* notYetVisitedOrel =
        (OrderedRelation*) result.addr;
    // cast the result

    //get next edge (s, w)
    GenericRelationIterator* edgesRelSourceIter =
        edgesRelSource->MakeScan();
    Tuple* nextTuple =
        edgesRelSourceIter->GetNextTuple();

    std::multimap<double, std::tuple<int, int>>
            notYetVisitedNodesMultiMap;

    //iterate over all given edges in the given input order
    while(nextTuple)
    {
        CcInt* nextTupleSourceId = (CcInt*)
                                   nextTuple->GetAttribute(
                            HubLabelClass::HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);

        //create minW tuple from the source of the current edge iterated  (
        //so force the source to be the current minW also if it actually isnt)
        std::tuple<double, int, int>
        currMinNotYetVisitedNodeTuple;
        bool callFunction = false;

        //Dont insert something when no source had been inserted befire
        //Special call of the function if the actual source node s
        // shall be inserted
        //If there are entries call insert or update which may leads to
        // a insertion, an update or no change at all
        //do so for every iteration of the given relation
        if(nextTupleSourceId->GetIntval() ==
                currNodeIdS->GetIntval())
        {
            currMinNotYetVisitedNodeTuple = std::make_tuple(
                                                0.0,
                                    nextTupleSourceId->GetIntval(), 0);
            callFunction = true;
        }
        else if(notYetVisitedNodesMultiMap.size() > 0)
        {
            //get Tuple with min(dist)
            std::multimap<double, std::tuple<int, int>>::iterator
                    notYetVisitedNodesMultiMapIter =
                        notYetVisitedNodesMultiMap.begin();
            double currMinDist =
                (*notYetVisitedNodesMultiMapIter).first;

            std::tuple<int, int> currTuple =
                (*notYetVisitedNodesMultiMapIter).second;
            int currMinNodeId = std::get<
                    HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED
                                - 1>(currTuple);
            int currMinHopDepth = std::get<
                    HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED
                                  - 1>(currTuple);

            //Create auxiliary tuple
            currMinNotYetVisitedNodeTuple = std::make_tuple(
                                                currMinDist,
                                                currMinNodeId, currMinHopDepth);
            callFunction = true;
        }

        //insert edge (s, w)
        if(callFunction == true)
        {
            bool isHHop = true;
            k->hlInsertOrUpdateTupleInNotYetVisitedList(
                currNodeIdV->GetIntval(),
                currMinNotYetVisitedNodeTuple,
                nextTuple, notYetVisitedNodesMultiMap,
                isForward->GetBoolval(),
                isHHop);
        }

        nextTuple->DeleteIfAllowed();
        nextTuple = 0;
        nextTuple = edgesRelSourceIter->GetNextTuple();
    }

    //convert multimap back to OrderedRelation
    for (std::multimap<double, std::tuple<int, int>>::iterator
            notYetVisitedNodesMultiMapIter =
                notYetVisitedNodesMultiMap.begin();
            notYetVisitedNodesMultiMapIter !=
            notYetVisitedNodesMultiMap.end();
            ++notYetVisitedNodesMultiMapIter)
    {
        std::tuple<int, int> currTupleConvert =
            (*notYetVisitedNodesMultiMapIter).second;

        ListExpr relType;
        nl->ReadFromString(
            HubLabelClass::hlGetNotYetVisitedNodesTypeInfo(),
            relType);
        //TODO: passt das? das ist ein orel type info, kein tuple type info
        ListExpr tupNumType =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relType));
        //TODO: passt das?
        Tuple* insertTuple = new Tuple(tupNumType);
        //num type verwenden oder nicht?
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_NOT_YET_VISITED,
            new CcReal(true,
                       (*notYetVisitedNodesMultiMapIter).first));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED
                      - 1>(currTupleConvert)));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED
                      - 1>(currTupleConvert)));
        notYetVisitedOrel->AppendTuple(insertTuple);
        insertTuple->DeleteIfAllowed();
        //TODO: warum muss der Tuple gelöscht werden?
        // wird er per beim Append geklont? Nicht dass das Löschen
        // hier den Tuple in der Relation zerstört.
    }

    //free resources
    delete edgesRelSourceIter;
    delete k;

    return 0;
}


int hlForwardSearchIterativeStepsScanNewVerticesVM (
    Word* args, Word& result,
    int message, Word& local, Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* orelEdgesSource =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* notYetVisitedNodesOrel =
        (OrderedRelation*) args[1].addr;
    OrderedRelation* stillVisitedNodesOrel =
        (OrderedRelation*) args[2].addr;
    OrderedRelation* copyMultimapReverseSearchXT =
        (OrderedRelation*) args[3].addr;
    Relation* currMinSingleTupleRelW =
        (Relation*) args[4].addr;
    CcInt* currNodeIdV = (CcInt*) args[5].addr;
    CcReal* distSV = (CcReal*) args[6].addr;
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif

    //minW Tuple aus Container-Relation auslesen
    GenericRelationIterator*
    currMinSingleTupleRelWIter =
        currMinSingleTupleRelW->MakeScan();
    Tuple* currMinSingleTupleRelWTuple =
        currMinSingleTupleRelWIter->GetNextTuple();
    delete currMinSingleTupleRelWIter;

    if(!currMinSingleTupleRelWTuple)
    {
        #ifdef USEDEBUG
LogDebug("kein Tuple, komisch" << endl);
#endif
    }
    CcReal* currMinDist = (CcReal*)
                          currMinSingleTupleRelWTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_DIST_IN_NOT_YET_VISITED);
    CcInt* currMinNodeId = (CcInt*)
                           currMinSingleTupleRelWTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED);
    CcInt* currMinHopDepth = (CcInt*)
                             currMinSingleTupleRelWTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED);

    std::tuple<double, int, int>
    currMinNotYetVisitedNodeTuple =
        std::make_tuple(currMinDist->GetRealval(),
                        currMinNodeId->GetIntval(),
                        currMinHopDepth->GetIntval());

    currMinSingleTupleRelWTuple->DeleteIfAllowed();
    currMinSingleTupleRelWTuple = 0;



    //convert OrderedRelation to multimap notYetVisited
    std::multimap<double, std::tuple<int, int>>
            notYetVisitedNodesMultiMap;
    GenericRelationIterator*
    notYetVisitedOrelResultIter =
        notYetVisitedNodesOrel->MakeScan();
    Tuple* notYetOrelTuple =
        notYetVisitedOrelResultIter->GetNextTuple();

    while(notYetOrelTuple)
    {
        CcReal* currNotYetDist = (CcReal*)
                                 notYetOrelTuple->GetAttribute(
                        HubLabelClass::HL_INDEX_OF_DIST_IN_NOT_YET_VISITED);
        CcInt* currNotYetNodeId = (CcInt*)
                                  notYetOrelTuple->GetAttribute(
                        HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED);
        CcInt* currNotYetHopDepth = (CcInt*)
                                    notYetOrelTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED);

        std::tuple<int, double> insertMultimapTupleNotYet
            = std::make_tuple(
                  currNotYetNodeId->GetIntval(),
                  currNotYetHopDepth->GetIntval());
        //num type verwenden oder nicht?
        notYetVisitedNodesMultiMap.insert(
            pair<double, std::tuple<int, int>>(
                currNotYetDist->GetRealval(),
                insertMultimapTupleNotYet));

        notYetOrelTuple->DeleteIfAllowed();
        notYetOrelTuple = 0;
        notYetOrelTuple =
            notYetVisitedOrelResultIter->GetNextTuple();
    }

    //free resources
    delete notYetVisitedOrelResultIter;



    //Convert OrderedRelation to unordered_set stillVisitedNodes
    GenericRelationIterator* stillVisitedNodesOrelIter
        =
            stillVisitedNodesOrel->MakeScan();
    Tuple* currStillVisitedOrelTuple =
        stillVisitedNodesOrelIter->GetNextTuple();

    std::unordered_set<int> stillVisitedNodesSet;
    while(currStillVisitedOrelTuple)
    {
        CcInt* currStillVisitedTupleId = (CcInt*)
                            currStillVisitedOrelTuple->GetAttribute(
                        HubLabelClass::HL_INDEX_OF_NODE_ID_IN_STILL_VISITED);
        stillVisitedNodesSet.insert(
            currStillVisitedTupleId->GetIntval());

        //Free Outgoing-Iteration
        currStillVisitedOrelTuple->DeleteIfAllowed();
        currStillVisitedOrelTuple = 0;
        currStillVisitedOrelTuple =
            stillVisitedNodesOrelIter->GetNextTuple();
    }

    delete stillVisitedNodesOrelIter;



    //Convert OrderedRelation to MultiMap reverseSearch
    std::multimap<int, std::tuple<int, double>>
            oneHopReverseMultimap;
    GenericRelationIterator*
    copyMultimapReverseSearchXTIter =
        copyMultimapReverseSearchXT->MakeScan();
    Tuple* currReverseOrelTuple =
        copyMultimapReverseSearchXTIter->GetNextTuple();

    while(currReverseOrelTuple)
    {
        CcInt* currReverseTupleIdU = (CcInt*)
                                     currReverseOrelTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE);
        CcInt* currReverseTupleIdT = (CcInt*)
                                     currReverseOrelTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE);
        CcReal* currReverseTupleDistUT = (CcReal*)
                                         currReverseOrelTuple->GetAttribute(
                        HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE);

        std::tuple<int, double> reverseInsertMultimapTuple
            = std::make_tuple(
                  currReverseTupleIdT->GetIntval(),
                  currReverseTupleDistUT->GetRealval());
        //num type verwenden oder nicht?

        oneHopReverseMultimap.insert(
            pair<int, std::tuple<int, double>>(
                currReverseTupleIdU->GetIntval(),
                reverseInsertMultimapTuple));

        //Free Outgoing-Iteration
        currReverseOrelTuple->DeleteIfAllowed();
        //TODO: ist delete ok, wenn zuvor einer anderen orel
        // zugewiesen oder wird das tuple aus der neuen orel dann
        // auch gelöscht?
        currReverseOrelTuple = 0;
        currReverseOrelTuple =
            copyMultimapReverseSearchXTIter->GetNextTuple();
    }

    delete copyMultimapReverseSearchXTIter;



    #ifdef USEDEBUG
LogDebug("los" << endl);
#endif
    k->hlForwardSearchIterativeStepsScanNewVertices(
        orelEdgesSource,
        currNodeIdV->GetIntval(),
        currMinNotYetVisitedNodeTuple,
        notYetVisitedNodesMultiMap, stillVisitedNodesSet,
        oneHopReverseMultimap,
        distSV->GetRealval());
    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif



    //convert multimap back to OrderedRelation notYetVisitedNodes
    notYetVisitedNodesOrel->Clear();
    for (std::multimap<double, std::tuple<int, int>>::iterator
            notYetVisitedNodesMultiMapIter =
                notYetVisitedNodesMultiMap.begin();
            notYetVisitedNodesMultiMapIter !=
            notYetVisitedNodesMultiMap.end();
            ++notYetVisitedNodesMultiMapIter)
    {
        std::tuple<int, int> currTupleConvert =
            (*notYetVisitedNodesMultiMapIter).second;

        ListExpr relType;
        nl->ReadFromString(
            HubLabelClass::hlGetNotYetVisitedNodesTypeInfo(),
            relType);
        //TODO: passt das? das ist ein orel type info, kein tuple type info
        ListExpr tupNumType =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relType));
        //TODO: passt das?
        Tuple* notYetInsertOrelTuple = new Tuple(
            tupNumType);
        //num type verwenden oder nicht?
        notYetInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_NOT_YET_VISITED,
            new CcReal(true,
                       (*notYetVisitedNodesMultiMapIter).first));
        notYetInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED
                      - 1>(currTupleConvert)));
        notYetInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED
                      - 1>(currTupleConvert)));
        notYetVisitedNodesOrel->AppendTuple(
            notYetInsertOrelTuple);
        notYetInsertOrelTuple->DeleteIfAllowed();
        //TODO: warum muss der Tuple gelöscht werden?
        // wird er per beim Append geklont? Nicht dass das Löschen
        // hier den Tuple in der Relation zerstört.
    }

    //convert multimap back to OrderedRelation notYetVisitedNodes
    copyMultimapReverseSearchXT->Clear();
    for (std::multimap<int, std::tuple<int, double>>::iterator
            oneHopReverseMultimapIter =
                oneHopReverseMultimap.begin();
            oneHopReverseMultimapIter !=
            oneHopReverseMultimap.end();
            ++oneHopReverseMultimapIter)
    {
        std::tuple<int, double> currTupleConvertReverse =
            (*oneHopReverseMultimapIter).second;

        ListExpr relTypeReverse;
        nl->ReadFromString(
            HubLabelClass::hlGetOneHopReverseOrelIdTypeInfo(),
            relTypeReverse);
        //TODO: passt das? das ist ein orel type info, kein tuple type info
        ListExpr tupNumTypeReverse =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relTypeReverse));
        //TODO: passt das?
        Tuple* reverseInsertOrelTuple = new Tuple(
            tupNumTypeReverse);
        //num type verwenden oder nicht?
        reverseInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE,
            new CcInt(true,
                      (*oneHopReverseMultimapIter).first));
        reverseInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE
                      - 1>(currTupleConvertReverse)));
        reverseInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE,
            new CcReal(true, std::get<
                       HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE
                       - 1>(currTupleConvertReverse)));
        copyMultimapReverseSearchXT->AppendTuple(
            reverseInsertOrelTuple);
        reverseInsertOrelTuple->DeleteIfAllowed();
        //TODO: warum muss der Tuple gelöscht werden?
        // wird er per beim Append geklont? Nicht dass das Löschen hier den
        // Tuple in der Relation zerstört.
    }

    //free resources
    delete k;

    //Änderungen an als Parameter übergebenen Orels persistieren
    qp->SetModified(qp->GetSon(s,1));
    qp->SetModified(qp->GetSon(s,3));

    return 0;
}


int hlForwardSearchProcessIncomingEdgeIterativeStepsVM (
    Word* args,
    Word& result, int message, Word& local,
    Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* orelEdgesSource =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* notYetVisitedNodesOrel =
        (OrderedRelation*) args[1].addr;
    OrderedRelation* stillVisitedNodesOrel =
        (OrderedRelation*) args[2].addr;
    OrderedRelation* copyMultimapReverseSearchXT =
        (OrderedRelation*) args[3].addr;
    CcInt* hHop = (CcInt*) args[4].addr;
    CcInt* currNodeIdV = (CcInt*) args[5].addr;
    CcReal* distSV = (CcReal*) args[6].addr;
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif


    //convert OrderedRelation to multimap notYetVisited
    std::multimap<double, std::tuple<int, int>>
            notYetVisitedNodesMultiMap;
    GenericRelationIterator*
    notYetVisitedOrelResultIter =
        notYetVisitedNodesOrel->MakeScan();
    Tuple* notYetOrelTuple =
        notYetVisitedOrelResultIter->GetNextTuple();

    while(notYetOrelTuple)
    {
        CcReal* currNotYetDist = (CcReal*)
                                 notYetOrelTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_DIST_IN_NOT_YET_VISITED);
        CcInt* currNotYetNodeId = (CcInt*)
                                  notYetOrelTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED);
        CcInt* currNotYetHopDepth = (CcInt*)
                                    notYetOrelTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED);

        std::tuple<int, int> insertMultimapTupleNotYet =
            std::make_tuple(
                currNotYetNodeId->GetIntval(),
                currNotYetHopDepth->GetIntval());
        //num type verwenden oder nicht?
        notYetVisitedNodesMultiMap.insert(
            pair<double, std::tuple<int, int>>(
                currNotYetDist->GetRealval(),
                insertMultimapTupleNotYet));

        notYetOrelTuple->DeleteIfAllowed();
        notYetOrelTuple = 0;
        notYetOrelTuple =
            notYetVisitedOrelResultIter->GetNextTuple();
    }

    //free resources
    delete notYetVisitedOrelResultIter;



    //Convert OrderedRelation to unordered_set stillVisitedNodes
    std::unordered_set<int> stillVisitedNodesSet;
    GenericRelationIterator* stillVisitedNodesOrelIter
        =
            stillVisitedNodesOrel->MakeScan();
    Tuple* currStillVisitedOrelTuple =
        stillVisitedNodesOrelIter->GetNextTuple();

    while(currStillVisitedOrelTuple)
    {
        CcInt* currStillVisitedTupleId = (CcInt*)
                                currStillVisitedOrelTuple->GetAttribute(
                        HubLabelClass::HL_INDEX_OF_NODE_ID_IN_STILL_VISITED);
        stillVisitedNodesSet.insert(
            currStillVisitedTupleId->GetIntval());

        //Free Outgoing-Iteration
        currStillVisitedOrelTuple->DeleteIfAllowed();
        currStillVisitedOrelTuple = 0;
        currStillVisitedOrelTuple =
            stillVisitedNodesOrelIter->GetNextTuple();
    }

    delete stillVisitedNodesOrelIter;



    //Convert OrderedRelation to MultiMap reverseSearch
    std::multimap<int, std::tuple<int, double>>
            oneHopReverseMultimap;
    GenericRelationIterator*
    copyMultimapReverseSearchXTIter =
        copyMultimapReverseSearchXT->MakeScan();
    Tuple* currReverseOrelTuple =
        copyMultimapReverseSearchXTIter->GetNextTuple();

    while(currReverseOrelTuple)
    {
        CcInt* currReverseTupleIdU = (CcInt*)
                                     currReverseOrelTuple->GetAttribute(
                        HubLabelClass::HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE);
        CcInt* currReverseTupleIdT = (CcInt*)
                                     currReverseOrelTuple->GetAttribute(
                        HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE);
        CcReal* currReverseTupleDistUT = (CcReal*)
                                         currReverseOrelTuple->GetAttribute(
                            HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE);

        std::tuple<int, double> reverseInsertMultimapTuple
            = std::make_tuple(
                  currReverseTupleIdT->GetIntval(),
                  currReverseTupleDistUT->GetRealval());
        //num type verwenden oder nicht?
        oneHopReverseMultimap.insert(
            pair<int, std::tuple<int, double>>(
                currReverseTupleIdU->GetIntval(),
                reverseInsertMultimapTuple));

        //Free Outgoing-Iteration
        currReverseOrelTuple->DeleteIfAllowed();  //TODO: ist delete ok,
        // wenn zuvor einer anderen orel zugewiesen oder wird das tuple
        // aus der neuen orel dann auch gelöscht?
        currReverseOrelTuple = 0;
        currReverseOrelTuple =
            copyMultimapReverseSearchXTIter->GetNextTuple();
    }

    delete copyMultimapReverseSearchXTIter;





    #ifdef USEDEBUG
LogDebug("los" << endl);
#endif
    k->hlForwardSearchProcessIncomingEdgeIterativeSteps(
        orelEdgesSource,
        currNodeIdV->GetIntval(),
        notYetVisitedNodesMultiMap,
        stillVisitedNodesSet, oneHopReverseMultimap,
        hHop->GetIntval(),
        distSV->GetRealval());
    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif



    //convert multimap back to OrderedRelation notYetVisitedNodes
    notYetVisitedNodesOrel->Clear();
    for (std::multimap<double, std::tuple<int, int>>::iterator
            notYetVisitedNodesMultiMapIter =
                notYetVisitedNodesMultiMap.begin();
            notYetVisitedNodesMultiMapIter !=
            notYetVisitedNodesMultiMap.end();
            ++notYetVisitedNodesMultiMapIter)
    {
        std::tuple<int, int> currTupleConvertNotYet =
            (*notYetVisitedNodesMultiMapIter).second;

        ListExpr relTypeNotYet;
        nl->ReadFromString(
            HubLabelClass::hlGetNotYetVisitedNodesTypeInfo(),
            relTypeNotYet); //TODO: passt das? das ist ein orel type info,
        // kein tuple type info
        ListExpr tupNumTypeNotYet =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relTypeNotYet));
        //TODO: passt das?
        Tuple* notYetInsertOrelTuple = new Tuple(
            tupNumTypeNotYet);
        //num type verwenden oder nicht?
        notYetInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_NOT_YET_VISITED,
            new CcReal(true,
                       (*notYetVisitedNodesMultiMapIter).first));
        notYetInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED
                      - 1>(currTupleConvertNotYet)));
        notYetInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED
                      - 1>(currTupleConvertNotYet)));
        notYetVisitedNodesOrel->AppendTuple(
            notYetInsertOrelTuple);
        notYetInsertOrelTuple->DeleteIfAllowed();
        //TODO: warum muss der Tuple gelöscht werden? wird er per beim
        // Append geklont? Nicht dass das Löschen hier den Tuple in der
        // Relation zerstört.
    }



    //convert multimap back to OrderedRelation notYetVisitedNodes
    stillVisitedNodesOrel->Clear();
    for (std::unordered_set<int>::iterator
            stillVisitedNodesSetIter =
                stillVisitedNodesSet.begin();
            stillVisitedNodesSetIter !=
            stillVisitedNodesSet.end();
            ++stillVisitedNodesSetIter)
    {
        ListExpr relTypeStillVisited;
        nl->ReadFromString(
            HubLabelClass::hlGetStillVisitedNodesTypeInfo(),
            relTypeStillVisited); //TODO: passt das? das ist ein orel
        // type info, kein tuple type info
        ListExpr tupNumTypeStillVisited =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(
                    relTypeStillVisited));//TODO: passt das?
        Tuple* stillVisitedInsertOrelTuple = new Tuple(
            tupNumTypeStillVisited);
        //num type verwenden oder nicht?
        stillVisitedInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_NODE_ID_IN_STILL_VISITED,
            new CcInt(true, *stillVisitedNodesSetIter));
        stillVisitedNodesOrel->AppendTuple(
            stillVisitedInsertOrelTuple);
        stillVisitedInsertOrelTuple->DeleteIfAllowed(); //TODO: warum muss der
        // Tuple gelöscht werden? wird er per beim Append geklont?
        // Nicht dass das Löschen hier den Tuple in der Relation zerstört.
    }



    //convert multimap back to OrderedRelation notYetVisitedNodes
    copyMultimapReverseSearchXT->Clear();
    for (std::multimap<int, std::tuple<int, double>>::iterator
            oneHopReverseMultimapIter =
                oneHopReverseMultimap.begin();
            oneHopReverseMultimapIter !=
            oneHopReverseMultimap.end();
            ++oneHopReverseMultimapIter)
    {
        std::tuple<int, double> currTupleConvertReverse =
            (*oneHopReverseMultimapIter).second;

        ListExpr relTypeReverse;
        nl->ReadFromString(
            HubLabelClass::hlGetOneHopReverseOrelIdTypeInfo(),
            relTypeReverse); //TODO: passt das? das ist ein orel type info,
        // kein tuple type info
        ListExpr tupNumTypeReverse =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relTypeReverse));
        //TODO: passt das?
        Tuple* reverseInsertOrelTuple = new Tuple(
            tupNumTypeReverse);
        //num type verwenden oder nicht?
        reverseInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE,
            new CcInt(true,
                      (*oneHopReverseMultimapIter).first));
        reverseInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE
                      - 1>(currTupleConvertReverse)));
        reverseInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE,
            new CcReal(true, std::get<
                       HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE
                       - 1>(currTupleConvertReverse)));
        copyMultimapReverseSearchXT->AppendTuple(
            reverseInsertOrelTuple);
        reverseInsertOrelTuple->DeleteIfAllowed(); //TODO: warum muss der Tuple
        // gelöscht werden? wird er per beim Append geklont? Nicht dass das
        // Löschen hier den Tuple in der Relation zerstört.
    }

    //free resources
    delete k;

    //Änderungen an als Parameter übergebenen Orels persistieren
    qp->SetModified(qp->GetSon(s,1));
    qp->SetModified(qp->GetSon(s,2));
    qp->SetModified(qp->GetSon(s,3));

    return 0;
}


int hlForwardSearchProcessIncomingEdgeInitialStepsVM (
    Word* args, Word& result,
    int message, Word& local, Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* orelEdgesSource =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* notYetVisitedNodesOrel =
        (OrderedRelation*) args[1].addr;
    OrderedRelation* copyMultimapReverseSearchXT =
        (OrderedRelation*) args[2].addr;
    CcInt* currNodeIdS = (CcInt*) args[3].addr;
    CcInt* currNodeIdV = (CcInt*) args[4].addr;
    CcReal* distSV = (CcReal*) args[5].addr;
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif


    //convert OrderedRelation to multimap notYetVisited
    std::multimap<double, std::tuple<int, int>>
            notYetVisitedNodesMultiMap;
    GenericRelationIterator*
    notYetVisitedOrelResultIter =
        notYetVisitedNodesOrel->MakeScan();
    Tuple* notYetOrelTuple =
        notYetVisitedOrelResultIter->GetNextTuple();

    while(notYetOrelTuple)
    {
        CcReal* currNotYetDist = (CcReal*)
                                 notYetOrelTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_DIST_IN_NOT_YET_VISITED);
        CcInt* currNotYetNodeId = (CcInt*)
                                  notYetOrelTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED);
        CcInt* currNotYetHopDepth = (CcInt*)
                                    notYetOrelTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED);

        std::tuple<int, int> insertMultimapTupleNotYet =
            std::make_tuple(
                currNotYetNodeId->GetIntval(),
                currNotYetHopDepth->GetIntval());
        notYetVisitedNodesMultiMap.insert(
            pair<double, std::tuple<int, int>>(
                currNotYetDist->GetRealval(),
                insertMultimapTupleNotYet));

        notYetOrelTuple->DeleteIfAllowed();
        notYetOrelTuple = 0;
        notYetOrelTuple =
            notYetVisitedOrelResultIter->GetNextTuple();
    }

    //free resources
    delete notYetVisitedOrelResultIter;



    //Convert OrderedRelation to MultiMap reverseSearch
    std::multimap<int, std::tuple<int, double>>
            oneHopReverseMultimap;
    GenericRelationIterator*
    copyMultimapReverseSearchXTIter =
        copyMultimapReverseSearchXT->MakeScan();
    Tuple* currReverseOrelTuple =
        copyMultimapReverseSearchXTIter->GetNextTuple();

    while(currReverseOrelTuple)
    {
        CcInt* currReverseTupleIdU = (CcInt*)
                                     currReverseOrelTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE);
        CcInt* currReverseTupleIdT = (CcInt*)
                                     currReverseOrelTuple->GetAttribute(
                        HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE);
        CcReal* currReverseTupleDistUT = (CcReal*)
                                         currReverseOrelTuple->GetAttribute(
                        HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE);

        std::tuple<int, double> reverseInsertMultimapTuple
            = std::make_tuple(
                  currReverseTupleIdT->GetIntval(),
                  currReverseTupleDistUT->GetRealval());
        oneHopReverseMultimap.insert(
            pair<int, std::tuple<int, double>>(
                currReverseTupleIdU->GetIntval(),
                reverseInsertMultimapTuple));

        //Free Outgoing-Iteration
        currReverseOrelTuple->DeleteIfAllowed();  //TODO: ist delete ok,
        // wenn zuvor einer anderen orel zugewiesen oder wird das tuple
        // aus der neuen orel dann auch gelöscht?
        currReverseOrelTuple = 0;
        currReverseOrelTuple =
            copyMultimapReverseSearchXTIter->GetNextTuple();
    }

    delete copyMultimapReverseSearchXTIter;


    #ifdef USEDEBUG
LogDebug("los" << endl);
#endif
    k->hlForwardSearchProcessIncomingEdgeInitialSteps(
        orelEdgesSource,
        currNodeIdV->GetIntval(), currNodeIdS,
        notYetVisitedNodesMultiMap,
        oneHopReverseMultimap, distSV->GetRealval());
    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif



    //convert multimap back to OrderedRelation notYetVisitedNodes
    notYetVisitedNodesOrel->Clear();
    for (std::multimap<double, std::tuple<int, int>>::iterator
            notYetVisitedNodesMultiMapIter =
                notYetVisitedNodesMultiMap.begin();
            notYetVisitedNodesMultiMapIter !=
            notYetVisitedNodesMultiMap.end();
            ++notYetVisitedNodesMultiMapIter)
    {
        std::tuple<int, int> currTupleConvertNotYet =
            (*notYetVisitedNodesMultiMapIter).second;

        ListExpr relTypeNotYet;
        nl->ReadFromString(
            HubLabelClass::hlGetNotYetVisitedNodesTypeInfo(),
            relTypeNotYet); //TODO: passt das? das ist ein orel type info,
        // kein tuple type info
        ListExpr tupNumTypeNotYet =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relTypeNotYet));
        //TODO: passt das?
        Tuple* notYetInsertOrelTuple = new Tuple(
            tupNumTypeNotYet);
        //num type verwenden oder nicht?
        notYetInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_NOT_YET_VISITED,
            new CcReal(true,
                       (*notYetVisitedNodesMultiMapIter).first));
        notYetInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED
                      - 1>(currTupleConvertNotYet)));
        notYetInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED
                      - 1>(currTupleConvertNotYet)));
        notYetVisitedNodesOrel->AppendTuple(
            notYetInsertOrelTuple);
        notYetInsertOrelTuple->DeleteIfAllowed(); //TODO: warum muss der Tuple
        // gelöscht werden? wird er per beim Append geklont? Nicht dass das
        // Löschen hier den Tuple in der Relation zerstört.
    }



    //convert multimap back to OrderedRelation notYetVisitedNodes
    copyMultimapReverseSearchXT->Clear();
    for (std::multimap<int, std::tuple<int, double>>::iterator
            oneHopReverseMultimapIter =
                oneHopReverseMultimap.begin();
            oneHopReverseMultimapIter !=
            oneHopReverseMultimap.end();
            ++oneHopReverseMultimapIter)
    {
        std::tuple<int, double> currTupleConvertReverse =
            (*oneHopReverseMultimapIter).second;

        ListExpr relTypeReverse;
        nl->ReadFromString(
            HubLabelClass::hlGetOneHopReverseOrelIdTypeInfo(),
            relTypeReverse);
        //TODO: passt das? das ist ein orel type info, kein tuple type info
        ListExpr tupNumTypeReverse =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relTypeReverse));
        //TODO: passt das?
        Tuple* reverseInsertOrelTuple = new Tuple(
            tupNumTypeReverse);
        //num type verwenden oder nicht?
        reverseInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE,
            new CcInt(true,
                      (*oneHopReverseMultimapIter).first));
        reverseInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE
                      - 1>(currTupleConvertReverse)));
        reverseInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE,
            new CcReal(true, std::get<
                       HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE
                       - 1>(currTupleConvertReverse)));
        copyMultimapReverseSearchXT->AppendTuple(
            reverseInsertOrelTuple);
        reverseInsertOrelTuple->DeleteIfAllowed(); //TODO: warum muss der
        // Tuple gelöscht werden? wird er per beim Append geklont?
        // Nicht dass das Löschen hier den Tuple in der Relation zerstört.
    }

    //free resources
    delete k;

    //Änderungen an als Parameter übergebenen Orels persistieren
    qp->SetModified(qp->GetSon(s,1));
    qp->SetModified(qp->GetSon(s,2));

    return 0;
}


int hlForwardSearchCreateAndAppendShortcutsVM (
    Word* args, Word& result,
    int message, Word& local, Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* copyMultimapReverseSearchXT =
        (OrderedRelation*) args[0].addr;
    CcInt* currentSourceNodeFwdS = (CcInt*)
                                   args[1].addr;
    CcInt* currentVertexIdFwdV = (CcInt*)
                                 args[2].addr;
    CcReal* distSV = (CcReal*) args[3].addr;
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    OrderedRelation*
    shortcutsToBeCreatedOrelSourceToBeDeleted =
        (OrderedRelation*) result.addr; // cast the result

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif


    //Convert OrderedRelation to MultiMap
    std::multimap<int, std::tuple<int, double>>
            reverseXTMultimap;
    GenericRelationIterator* witnessListIter =
        copyMultimapReverseSearchXT->MakeScan();
    Tuple* currTuple =
        witnessListIter->GetNextTuple();
    while(currTuple)
    {
        CcInt* currTupleIdU = (CcInt*)
                              currTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE);
        CcInt* currTupleIdT = (CcInt*)
                              currTuple->GetAttribute(
                HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE);
        CcReal* currTupleDistUT = (CcReal*)
                                  currTuple->GetAttribute(
                HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE);

        std::tuple<int, double> insertTuple =
            std::make_tuple(
                currTupleIdT->GetIntval(),
                currTupleDistUT->GetRealval());
        //num type verwenden oder nicht?
        reverseXTMultimap.insert(
            pair<int, std::tuple<int, double>>(
                currTupleIdU->GetIntval(), insertTuple));

        //Free Outgoing-Iteration
        currTuple->DeleteIfAllowed();  //TODO: ist delete ok, wenn zuvor
        // einer anderen orel zugewiesen oder wird das tuple aus der
        // neuen orel dann auch gelöscht?
        currTuple = 0;
        currTuple = witnessListIter->GetNextTuple();
    }

    delete witnessListIter;

    std::multimap<int, std::tuple<int, double, int>>
            shortcutsMultimap;

    #ifdef USEDEBUG
LogDebug("los" << endl);
#endif
    k->hlForwardSearchCreateAndAppendShortcuts(
        shortcutsMultimap,
        reverseXTMultimap,
        currentSourceNodeFwdS->GetIntval(),
        currentVertexIdFwdV->GetIntval(),
        distSV->GetRealval());
    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif

    //convert multimap back to OrdeedRelation
    shortcutsToBeCreatedOrelSourceToBeDeleted->Clear();
    for (std::multimap<int, std::tuple<int, double, int>>::iterator
            shortcutsMultimapIter = shortcutsMultimap.begin();
            shortcutsMultimapIter != shortcutsMultimap.end();
            ++shortcutsMultimapIter)
    {
        std::tuple<int, double, int> currTuple =
            (*shortcutsMultimapIter).second;

        ListExpr relType;
        nl->ReadFromString(
            HubLabelClass::hlGetShortcutsCreatedSourceTypeInfo(),
            relType);
        //TODO: passt das? das ist ein orel type info, kein tuple type info
        ListExpr tupNumType =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relType));
        //TODO: passt das?
        Tuple* insertTuple = new Tuple(tupNumType);
        //num type verwenden oder nicht?
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_SOURCE_IN_SHORTCUT_TUPLE,
            new CcInt(true, (*shortcutsMultimapIter).first));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_TARGET_IN_SHORTCUT_TUPLE,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_TARGET_IN_SHORTCUT_TUPLE
                      - 1>(currTuple)));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_SHORTCUT_TUPLE,
            new CcReal(true, std::get<
                       HubLabelClass::HL_INDEX_OF_DIST_IN_SHORTCUT_TUPLE
                       - 1>(currTuple)));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_PARENT_ID_IN_SHORTCUT_TUPLE,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_PARENT_ID_IN_SHORTCUT_TUPLE
                      - 1>(currTuple)));
        shortcutsToBeCreatedOrelSourceToBeDeleted->AppendTuple(
            insertTuple);
        insertTuple->DeleteIfAllowed(); //TODO: warum muss der Tuple
        // gelöscht werden? wird er per beim Append geklont?
        // Nicht dass das Löschen hier den Tuple in der Relation zerstört.
    }

    //free resources
    delete k;

    return 0;
}


int hlForwardSearchProcessIncomingEdgeVM (
    Word* args, Word& result,
    int message, Word& local, Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* orelEdgesSource =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* stillVisitedNodesOrel =
        (OrderedRelation*) args[1].addr;
    OrderedRelation* copyMultimapReverseSearchXT =
        (OrderedRelation*) args[2].addr;
    OrderedRelation*
    shortcutsToBeCreatedOrelSourceToBeDeleted =
        (OrderedRelation*) args[3].addr;
    CcInt* currentSourceNodeFwdS = (CcInt*)
                                   args[4].addr;
    CcInt* currentVertexIdFwdV = (CcInt*)
                                 args[5].addr;
    CcInt* hDepth = (CcInt*) args[6].addr;
    CcReal* distSV = (CcReal*) args[7].addr;
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif

    //Convert OrderedRelation to MultiMap
    std::multimap<int, std::tuple<int, double>>
            oneHopReverseMultimap;
    GenericRelationIterator* witnessListIter =
        copyMultimapReverseSearchXT->MakeScan();
    Tuple* currReverseXTOrelTuple =
        witnessListIter->GetNextTuple();
    while(currReverseXTOrelTuple)
    {
        CcInt* currTupleIdU = (CcInt*)
                              currReverseXTOrelTuple->GetAttribute(
                HubLabelClass::HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE);
        CcInt* currTupleIdT = (CcInt*)
                              currReverseXTOrelTuple->GetAttribute(
                HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE);
        CcReal* currTupleDistUT = (CcReal*)
                                  currReverseXTOrelTuple->GetAttribute(
                HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE);

        std::tuple<int, double> insertTuple =
            std::make_tuple(
                currTupleIdT->GetIntval(),
                currTupleDistUT->GetRealval());
        //num type verwenden oder nicht?
        oneHopReverseMultimap.insert(
            pair<int, std::tuple<int, double>>(
                currTupleIdU->GetIntval(), insertTuple));

        //Free Outgoing-Iteration
        currReverseXTOrelTuple->DeleteIfAllowed();  //TODO: ist delete ok,
        // wenn zuvor einer anderen orel zugewiesen oder wird das tuple
        // aus der neuen orel dann auch gelöscht?
        currReverseXTOrelTuple = 0;
        currReverseXTOrelTuple =
            witnessListIter->GetNextTuple();
    }
    delete witnessListIter;


    std::unordered_set<int> stillVisitedNodesSet;

    std::multimap<int, std::tuple<int, double, int>>
            shortcutsMultimap;

    #ifdef USEDEBUG
LogDebug("los" << endl);
#endif
    k->hlForwardSearchProcessIncomingEdge(
        orelEdgesSource, shortcutsMultimap,
        currentVertexIdFwdV->GetIntval(),
        currentSourceNodeFwdS,
        stillVisitedNodesSet, oneHopReverseMultimap,
        hDepth->GetIntval(),
        distSV->GetRealval());
    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif

    //convert multimap back to OrdeedRelation
    shortcutsToBeCreatedOrelSourceToBeDeleted->Clear();
    for (std::multimap<int, std::tuple<int, double, int>>::iterator
            shortcutsMultimapIter = shortcutsMultimap.begin();
            shortcutsMultimapIter != shortcutsMultimap.end();
            ++shortcutsMultimapIter)
    {
        std::tuple<int, double, int> currTuple =
            (*shortcutsMultimapIter).second;

        ListExpr relType;
        nl->ReadFromString(
            HubLabelClass::hlGetShortcutsCreatedSourceTypeInfo(),
            relType);
        //TODO: passt das? das ist ein orel type info, kein tuple type info
        ListExpr tupNumType =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relType));
        //TODO: passt das?
        Tuple* insertTuple = new Tuple(tupNumType);
        //num type verwenden oder nicht?
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_SOURCE_IN_SHORTCUT_TUPLE,
            new CcInt(true, (*shortcutsMultimapIter).first));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_TARGET_IN_SHORTCUT_TUPLE,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_TARGET_IN_SHORTCUT_TUPLE
                      - 1>(currTuple)));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_SHORTCUT_TUPLE,
            new CcReal(true, std::get<
                       HubLabelClass::HL_INDEX_OF_DIST_IN_SHORTCUT_TUPLE
                       - 1>(currTuple)));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_PARENT_ID_IN_SHORTCUT_TUPLE,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_PARENT_ID_IN_SHORTCUT_TUPLE
                      - 1>(currTuple)));
        shortcutsToBeCreatedOrelSourceToBeDeleted->AppendTuple(
            insertTuple);
        insertTuple->DeleteIfAllowed(); //TODO: warum muss der Tuple gelöscht
        // werden? wird er per beim Append geklont? Nicht dass das Löschen
        // hier den Tuple in der Relation zerstört.
    }



    //convert multimap back to OrderedRelation notYetVisitedNodes
    stillVisitedNodesOrel->Clear();
    for (std::unordered_set<int>::iterator
            stillVisitedNodesSetIter =
                stillVisitedNodesSet.begin();
            stillVisitedNodesSetIter !=
            stillVisitedNodesSet.end();
            ++stillVisitedNodesSetIter)
    {
        ListExpr relTypeStillVisited;
        nl->ReadFromString(
            HubLabelClass::hlGetStillVisitedNodesTypeInfo(),
            relTypeStillVisited); //TODO: passt das? das ist ein orel type info,
        // kein tuple type info
        ListExpr tupNumTypeStillVisited =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(
                    relTypeStillVisited));//TODO: passt das?
        Tuple* stillVisitedInsertOrelTuple = new Tuple(
            tupNumTypeStillVisited);
        //num type verwenden oder nicht?
        stillVisitedInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_NODE_ID_IN_STILL_VISITED,
            new CcInt(true, *stillVisitedNodesSetIter));
        stillVisitedNodesOrel->AppendTuple(
            stillVisitedInsertOrelTuple);
        stillVisitedInsertOrelTuple->DeleteIfAllowed();
        //TODO: warum muss der Tuple gelöscht werden? wird er per beim Append
        // geklont? Nicht dass das Löschen hier den Tuple in der
        // Relation zerstört.
    }



    //convert multimap back to OrderedRelation notYetVisitedNodes
    copyMultimapReverseSearchXT->Clear();
    for (std::multimap<int, std::tuple<int, double>>::iterator
            oneHopReverseMultimapIter =
                oneHopReverseMultimap.begin();
            oneHopReverseMultimapIter !=
            oneHopReverseMultimap.end();
            ++oneHopReverseMultimapIter)
    {
        std::tuple<int, double> currTupleConvertReverse =
            (*oneHopReverseMultimapIter).second;

        ListExpr relTypeReverse;
        nl->ReadFromString(
            HubLabelClass::hlGetOneHopReverseOrelIdTypeInfo(),
            relTypeReverse);
        //TODO: passt das? das ist ein orel type info, kein tuple type info
        ListExpr tupNumTypeReverse =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relTypeReverse));
        //TODO: passt das?
        Tuple* reverseInsertOrelTuple = new Tuple(
            tupNumTypeReverse);
        //num type verwenden oder nicht?
        reverseInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_ID_OF_U_IN_TEMP_REV_TUPLE,
            new CcInt(true,
                      (*oneHopReverseMultimapIter).first));
        reverseInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE
                      - 1>(currTupleConvertReverse)));
        reverseInsertOrelTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE,
            new CcReal(true, std::get<
                       HubLabelClass::HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE
                       - 1>(currTupleConvertReverse)));
        copyMultimapReverseSearchXT->AppendTuple(
            reverseInsertOrelTuple);
        reverseInsertOrelTuple->DeleteIfAllowed(); //TODO: warum muss der
        // Tuple gelöscht werden? wird er per beim Append geklont? Nicht dass
        // das Löschen hier den Tuple in der Relation zerstört.
    }

    //free resources
    delete k;

    //Änderungen an als Parameter übergebenen Orels persistieren
    qp->SetModified(qp->GetSon(s,1));
    qp->SetModified(qp->GetSon(s,2));
    qp->SetModified(qp->GetSon(s,3));

    return 0;
}


int hlRemoveContractedEdgesFromEdgesRelationsVM (
    Word* args, Word& result,
    int message, Word& local, Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* edgesCopyOrelSource =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* edgesCopyOrelTarget =
        (OrderedRelation*) args[1].addr;
    CcInt* currentContractV = (CcInt*) args[2].addr;
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif

    k->hlRemoveContractedEdgesFromEdgesRelations(
        edgesCopyOrelSource,
        edgesCopyOrelTarget, currentContractV);

    //free resources
    delete k;

    //Änderungen an als Parameter übergebenen Orels persistieren
    qp->SetModified(qp->GetSon(s,0));
    qp->SetModified(qp->GetSon(s,1));

    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif
    return 0;
}


int hlRemoveParallelEdgesFromEdgesRelationsVM (
    Word* args, Word& result,
    int message, Word& local, Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* edgesCopyOrelSource =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* edgesCopyOrelTarget =
        (OrderedRelation*) args[1].addr;
    OrderedRelation*
    shortcutsToBeCreatedOrelSourceToBeDeleted =
        (OrderedRelation*) args[2].addr;
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif



    //Convert OrderedRelation to MultiMap
    std::multimap<int, std::tuple<int, double, int>>
            shortcutsMultimap;
    GenericRelationIterator*
    shortcutsToBeCreatedOrelSourceToBeDeletedIter =
        shortcutsToBeCreatedOrelSourceToBeDeleted->MakeScan();
    Tuple* currTuple =
        shortcutsToBeCreatedOrelSourceToBeDeletedIter->GetNextTuple();
    while(currTuple)
    {
        CcInt* currTupleIdS = (CcInt*)
                              currTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
        CcInt* currTupleIdT = (CcInt*)
                              currTuple->GetAttribute(
                HubLabelClass::HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
        double currTupleDistST =
            0.0; //we want the shortcut to be shorter
        int currTupleParentST = 999; //doesnt matter here

        std::tuple<int, double, int> insertTuple =
            std::make_tuple(
                currTupleIdT->GetIntval(), currTupleDistST,
                currTupleParentST);
        //num type verwenden oder nicht?
        shortcutsMultimap.insert(
            pair<int, std::tuple<int, double, int>>(
                currTupleIdS->GetIntval(), insertTuple));

        //Free Outgoing-Iteration
        currTuple->DeleteIfAllowed();
        //TODO: ist delete ok, wenn zuvor einer anderen orel zugewiesen
        // oder wird das tuple aus der neuen orel dann auch gelöscht?
        currTuple = 0;
        currTuple =
            shortcutsToBeCreatedOrelSourceToBeDeletedIter->GetNextTuple();
    }
    delete shortcutsToBeCreatedOrelSourceToBeDeletedIter;


    k->hlRemoveParallelEdgesFromEdgesRelations(
        edgesCopyOrelSource,
        edgesCopyOrelTarget, shortcutsMultimap);

    //free resources
    delete k;

    //Änderungen an als Parameter übergebenen Orels persistieren
    qp->SetModified(qp->GetSon(s,0));
    qp->SetModified(qp->GetSon(s,1));

    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif
    return 0;
}


int hlDoContractionVM (Word* args, Word& result,
                       int message, Word& local,
                       Supplier s)
{
    #ifdef USEDEBUG
    LogDebug("start" << endl);
    #endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* edgesWithViaOrelSource =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* edgesWithViaOrelTarget =
        (OrderedRelation*) args[1].addr;
    CcInt* currentVToBeContracted = (CcInt*)
                                    args[2].addr;
    CcInt* hHop = (CcInt*) args[3].addr;
    #ifdef USEDEBUG
    LogDebug("Parameter gelesen" << endl);
    #endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    OrderedRelation*
    shortcutsToBeCreatedOrelSourceToBeDeleted =
        (OrderedRelation*) result.addr; // cast the result

    #ifdef USEDEBUG
    LogDebug("result gelesen" << endl);
    #endif

    std::multimap<int, std::tuple<int, double, int>>
            shortcutsMultimap;

    #ifdef USEDEBUG
    LogDebug("los" << endl);
    #endif
    k->hlDoContraction(edgesWithViaOrelSource,
                       edgesWithViaOrelTarget,
                       shortcutsMultimap, currentVToBeContracted,
                       hHop->GetIntval());
    #ifdef USEDEBUG
    LogDebug("fertig" << endl);
    #endif

    //convert multimap back to OrderedRelation
    for (std::multimap<int, std::tuple<int, double, int>>::iterator
            shortcutsMultimapIter = shortcutsMultimap.begin();
            shortcutsMultimapIter != shortcutsMultimap.end();
            ++shortcutsMultimapIter)
    {
        std::tuple<int, double, int> currTuple =
            (*shortcutsMultimapIter).second;

        ListExpr relType;
        nl->ReadFromString(
            HubLabelClass::hlGetShortcutsCreatedSourceTypeInfo(),
            relType);
        //TODO: passt das? das ist ein orel type info, kein tuple type info
        ListExpr tupNumType =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relType));
        //TODO: passt das?
        Tuple* insertTuple = new Tuple(tupNumType);
        //num type verwenden oder nicht?
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_SOURCE_IN_SHORTCUT_TUPLE,
            new CcInt(true, (*shortcutsMultimapIter).first));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_TARGET_IN_SHORTCUT_TUPLE,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_TARGET_IN_SHORTCUT_TUPLE
                      - 1>(currTuple)));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_SHORTCUT_TUPLE,
            new CcReal(true, std::get<
                       HubLabelClass::HL_INDEX_OF_DIST_IN_SHORTCUT_TUPLE
                       - 1>(currTuple)));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_PARENT_ID_IN_SHORTCUT_TUPLE,
            new CcInt(true,
            std::get<HubLabelClass::HL_INDEX_OF_PARENT_ID_IN_SHORTCUT_TUPLE
                      - 1>(currTuple)));
        shortcutsToBeCreatedOrelSourceToBeDeleted->AppendTuple(
            insertTuple);
        insertTuple->DeleteIfAllowed(); //TODO: warum muss der Tuple gelöscht
        // werden? wird er per beim Append geklont? Nicht dass das Löschen
        // hier den Tuple in der Relation zerstört.
    }

    //free resources
    delete k;

    //Änderungen an als Parameter übergebenen Orels persistieren

    return 0;
}


int hlIterateOverAllNodesByRankAscAndDoContractionVM (
    Word* args, Word& result,
    int message, Word& local, Supplier s)
{
    #ifdef USEDEBUG
    LogDebug("start" << endl);
    #endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* nodesWithRankOrelRank =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* edgesWithViaOrelSource =
        (OrderedRelation*) args[1].addr;
    OrderedRelation* edgesWithViaOrelTarget =
        (OrderedRelation*) args[2].addr;
    CcInt* hHop = (CcInt*) args[3].addr;
    #ifdef USEDEBUG
    LogDebug("Parameter gelesen" << endl);
    #endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    #ifdef USEDEBUG
    LogDebug("result gelesen" << endl);
    #endif

    #ifdef USEDEBUG
    LogDebug("los" << endl);
    #endif
    k->hlIterateOverAllNodesByRankAscAndDoContraction(
        nodesWithRankOrelRank,
        edgesWithViaOrelSource, edgesWithViaOrelTarget,
        hHop->GetIntval());
    #ifdef USEDEBUG
    LogDebug("fertig" << endl);
    #endif

    //free resources
    delete k;

    //Änderungen an als Parameter übergebenen Orels persistieren
    qp->SetModified(qp->GetSon(s,1));
    qp->SetModified(qp->GetSon(s,2));

    return 0;
}


int hlCreateLabelCheckForWitnessScanNewVerticesVM (
    Word* args, Word& result,
    int message, Word& local, Supplier s)
{
    #ifdef USEDEBUG
    LogDebug("start" << endl);
    #endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* edgesContractedUpwardsOrelSource
        =
            (OrderedRelation*) args[0].addr;
    OrderedRelation*
    edgesContractedDownwardsOrelTarget =
        (OrderedRelation*) args[1].addr;
    CcInt* revGivenTargetIdV = (CcInt*) args[2].addr;
    CcInt* revGivenHopDepthV = (CcInt*) args[3].addr;
    CcReal* revGivenDistanceVToW = (CcReal*)
                                   args[4].addr;
    CcBool* isForward = (CcBool*) args[5].addr;
    #ifdef USEDEBUG
    LogDebug("Parameter gelesen" << endl);
    #endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    OrderedRelation* reverseNotYetVisitedNodesOrel =
        (OrderedRelation*)
        result.addr; // cast the result

    #ifdef USEDEBUG
    LogDebug("result gelesen" << endl);
    #endif

    std::vector<int> stillVisitedNodes;
    stillVisitedNodes.push_back(
        revGivenTargetIdV->GetIntval());

    std::multimap<double, std::tuple<int, int>>
            reverseNotYetVisitedNodes;

    #ifdef USEDEBUG
    LogDebug("los" << endl);
    #endif
    k->hlCreateLabelCheckForWitnessScanNewVertices(
        reverseNotYetVisitedNodes,
        stillVisitedNodes,
        edgesContractedUpwardsOrelSource,
        edgesContractedDownwardsOrelTarget,
        revGivenTargetIdV,
        revGivenDistanceVToW->GetRealval(),
        revGivenHopDepthV->GetIntval(),
        isForward->GetBoolval());
    #ifdef USEDEBUG
    LogDebug("fertig" << endl);
    #endif

    //convert multimap back to OrderedRelation
    for (std::multimap<double, std::tuple<int, int>>::iterator
            reverseNotYetVisitedNodesIter =
                reverseNotYetVisitedNodes.begin();
            reverseNotYetVisitedNodesIter !=
            reverseNotYetVisitedNodes.end();
            ++reverseNotYetVisitedNodesIter)
    {
        std::tuple<int, int> currTuple =
            (*reverseNotYetVisitedNodesIter).second;

        ListExpr relType;
        nl->ReadFromString(
            HubLabelClass::hlGetNotYetVisitedNodesTypeInfo(),
            relType);
        //TODO: passt das? das ist ein orel type info, kein tuple type info
        ListExpr tupNumType =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relType));
        //TODO: passt das?
        Tuple* insertTuple = new Tuple(tupNumType);
        //num type verwenden oder nicht?
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_NOT_YET_VISITED,
            new CcReal(true,
                       (*reverseNotYetVisitedNodesIter).first));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED
                      - 1>(currTuple)));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED
                      - 1>(currTuple)));
        reverseNotYetVisitedNodesOrel->AppendTuple(
            insertTuple);
        insertTuple->DeleteIfAllowed(); //TODO: warum muss der Tuple gelöscht
        // werden? wird er per beim Append geklont? Nicht dass das Löschen
        // hier den Tuple in der Relation zerstört.
    }

    //free resources
    delete k;

    return 0;
}


int hlCreateLabelCheckForWitnessVM (Word* args,
                                    Word& result, int message,
                                    Word& local, Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* edgesContractedUpwardsOrelSource
        = (OrderedRelation*)
          args[0].addr;
    OrderedRelation*
    edgesContractedDownwardsOrelTarget =
        (OrderedRelation*)
        args[1].addr;
    Relation* fwdOrRvsLabelRelation = (Relation*)
                                      args[2].addr;
    CcInt* givenTargetIdW = (CcInt*) args[3].addr;
    CcInt* hHopSize = (CcInt*) args[4].addr;
    CcReal* givenDistanceSW = (CcReal*) args[5].addr;
    CcBool* isForward = (CcBool*) args[6].addr;
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result

    #ifdef USEDEBUG
LogDebug("result vorbereitet" << endl);
#endif


    //Convert Relation to MultiMap
    std::multimap<int, std::tuple<int, int, double>>
            fwdOrRvsLabelMultimap;
    GenericRelationIterator* fwdOrRvsLabelIter =
        fwdOrRvsLabelRelation->MakeScan();
    Tuple* currTuple =
        fwdOrRvsLabelIter->GetNextTuple();
    while(currTuple)
    {
        CcInt* currTupleHubIdNew = (CcInt*)
                                   currTuple->GetAttribute(
                HubLabelClass::HL_INDEX_OF_HUB_NODE_ID_NEW_IN_LABEL_TUPLE);
        CcInt* currTupleHubID = (CcInt*)
                                currTuple->GetAttribute(
    HubLabelClass::HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE);
        CcInt* currTupleHubParentId = (CcInt*)
                                      currTuple->GetAttribute(
                HubLabelClass::HL_INDEX_OF_HUB_PARENT_NODE_ID_IN_LABEL_TUPLE);
        //CcInt* currTupleHubParentTupleId = (CcInt*)
        // currTuple->GetAttribute(
        // HubLabelClass::HL_INDEX_OF_HUB_PARENT_TUPLE_ID_IN_LABEL_TUPLE);
        //wird hier nicht benötigt
        CcReal* currTupleHubDistanceToSource = (CcReal*)
                                               currTuple->GetAttribute(
        HubLabelClass::HL_INDEX_OF_HUB_DISTANCE_TO_SOURCE_IN_LABEL_TUPLE);

        std::tuple<int, int, double> insertTuple =
            std::make_tuple(
                currTupleHubIdNew->GetIntval(),
                currTupleHubParentId->GetIntval(),
                currTupleHubDistanceToSource->GetRealval());
        fwdOrRvsLabelMultimap.insert(
            pair<int, std::tuple<int, int, double>>(
                currTupleHubID->GetIntval(), insertTuple));

        //Free Outgoing-Iteration
        currTuple->DeleteIfAllowed();  //TODO: ist delete ok, wenn zuvor
        // einer anderen orel zugewiesen oder wird das tuple aus der
        // neuen orel dann auch gelöscht?
        currTuple = 0;
        currTuple = fwdOrRvsLabelIter->GetNextTuple();
    }
    delete fwdOrRvsLabelIter;


    #ifdef USEDEBUG
LogDebug("los" << endl);
#endif
    bool isPruned = k->hlCreateLabelCheckForWitness(
                        fwdOrRvsLabelMultimap,
                        edgesContractedUpwardsOrelSource,
                        edgesContractedDownwardsOrelTarget,
                        givenTargetIdW, givenDistanceSW->GetRealval(),
                        hHopSize->GetIntval(),
                        isForward->GetBoolval());
    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif


    resultInt->Set(true, isPruned);

    #ifdef USEDEBUG
LogDebug("result vorbereitet" << endl);
#endif

    //free resources
    delete k;

    return 0;
}


int hlCreateLabelScanNewVerticesVM (Word* args,
                                    Word& result, int message,
                                    Word& local, Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* edgesContractedUpwardsOrelSource
        = (OrderedRelation*)
          args[0].addr;
    OrderedRelation*
    edgesContractedDownwardsOrelTarget =
        (OrderedRelation*)
        args[1].addr;
    Relation* fwdOrRvsLabelRelation = (Relation*)
                                      args[2].addr;
    CcInt* givenTargetIdV = (CcInt*) args[3].addr;
    CcReal* givenDistanceSV = (CcReal*) args[4].addr;
    CcBool* isForward = (CcBool*) args[5].addr;
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    OrderedRelation* reverseNotYetVisitedNodesOrel =
        (OrderedRelation*)
        result.addr; // cast the result

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif



    std::multimap<double, std::tuple<int, int>>
            labelNotYetVisitedNodes;

    //Convert Relation to MultiMap and notYetMulimap
    std::multimap<int, std::tuple<int, int, double>>
            fwdOrRvsLabelMultimap;
    std::vector<int> stillVisitedNodes;

    GenericRelationIterator* fwdOrRvsLabelIter =
        fwdOrRvsLabelRelation->MakeScan();
    Tuple* currTuple =
        fwdOrRvsLabelIter->GetNextTuple();
    while(currTuple)
    {
        CcInt* currTupleHubID = (CcInt*)
                                currTuple->GetAttribute(
                HubLabelClass::HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE);
        /*
        CcInt* currTupleHubIdNew = (CcInt*)
         currTuple->GetAttribute(
         HubLabelClass::HL_INDEX_OF_HUB_NODE_ID_NEW_IN_LABEL_TUPLE);
        CcInt* currTupleHubParentId = (CcInt*)
         currTuple->GetAttribute(
         HubLabelClass::HL_INDEX_OF_HUB_PARENT_NODE_ID_IN_LABEL_TUPLE);
        CcInt* currTupleHubParentTuplId = (CcInt*)
         currTuple->GetAttribute(
         HubLabelClass::HL_INDEX_OF_HUB_PARENT_TUPLE_ID_IN_LABEL_TUPLE);
        CcReal* currTupleHubDistanceToSource = (CcReal*)
         currTuple->GetAttribute(
         HubLabelClass::HL_INDEX_OF_HUB_DISTANCE_TO_SOURCE_IN_LABEL_TUPLE);

        std::tuple<int, int, double> insertTupleLabel =
         std::make_tuple(currTupleHubIdNew->GetIntval(),
         currTupleHubParentId->GetIntval(),
         currTupleHubDistanceToSource->GetRealval());
       */
        stillVisitedNodes.push_back(
            currTupleHubID->GetIntval());

        //Free Outgoing-Iteration
        currTuple->DeleteIfAllowed();
        currTuple = 0;
        currTuple = fwdOrRvsLabelIter->GetNextTuple();
    }
    delete fwdOrRvsLabelIter;


    #ifdef USEDEBUG
LogDebug("los" << endl);
#endif
    k->hlCreateLabelScanNewVertices(
        labelNotYetVisitedNodes, stillVisitedNodes,
        edgesContractedUpwardsOrelSource,
        edgesContractedDownwardsOrelTarget,
        givenTargetIdV, givenDistanceSV->GetRealval(),
        isForward->GetBoolval());
    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif


    //convert multimap back to OrderedRelation
    for (std::multimap<double, std::tuple<int, int>>::iterator
            labelNotYetVisitedNodesIter =
                labelNotYetVisitedNodes.begin();
            labelNotYetVisitedNodesIter !=
            labelNotYetVisitedNodes.end();
            ++labelNotYetVisitedNodesIter)
    {
        std::tuple<int, int> currTuple =
            (*labelNotYetVisitedNodesIter).second;

        ListExpr relType;
        nl->ReadFromString(
            HubLabelClass::hlGetNotYetVisitedNodesTypeInfo(),
            relType);
        //TODO: passt das? das ist ein orel type info, kein tuple type info
        ListExpr tupNumType =
            SecondoSystem::GetCatalog()->NumericType(
                nl->Second(relType));
        //TODO: passt das?
        Tuple* insertTuple = new Tuple(tupNumType);
        //num type verwenden oder nicht?
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_DIST_IN_NOT_YET_VISITED,
            new CcReal(true,
                       (*labelNotYetVisitedNodesIter).first));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED
                      - 1>(currTuple)));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED
                      - 1>(currTuple)));
        reverseNotYetVisitedNodesOrel->AppendTuple(
            insertTuple);
        insertTuple->DeleteIfAllowed(); //TODO: warum muss der Tuple gelöscht
        // werden? wird er per beim Append geklont? Nicht dass das Löschen
        // hier den Tuple in der Relation zerstört.
    }


    //free resources
    delete k;

    return 0;
}


int hlGetRankByIdVM (Word* args, Word& result,
                     int message, Word& local,
                     Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    BTree* bTreeNodesRanked = (BTree*) args[0].addr;
    OrderedRelation* nodesRanked = (OrderedRelation*)
                                   args[1].addr;
    CcInt* givenNodeId = (CcInt*) args[2].addr;
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif

    int retVal = -2;

    #ifdef USEDEBUG
LogDebug("los" << endl);
#endif
    retVal = k->hlGetRankById(bTreeNodesRanked,
                              nodesRanked, givenNodeId);
    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif

    resultInt->Set(true, retVal);

    //free resources
    delete k;

    return 0;
}


int hlCreateLabelByDijkstraWithStallingVM (
    Word* args, Word& result,
    int message, Word& local, Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    BTree* bTreeNodesRanked = (BTree*) args[0].addr;
    OrderedRelation* nodesRanked = (OrderedRelation*)
                                   args[1].addr;
    OrderedRelation* edgesWithViaOrelSource =
        (OrderedRelation*) args[2].addr;
    OrderedRelation* edgesWithViaOrelTarget =
        (OrderedRelation*) args[3].addr;
    CcInt* sourceNodeId = (CcInt*) args[4].addr;
    CcInt* hHop = (CcInt*) args[5].addr;
    CcBool* isForward = (CcBool*) args[6].addr;
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    Relation* fwdOrRvsLabelRel = (Relation*)
                                 result.addr; // cast the result

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif


    std::multimap<int, std::tuple<int, int, double>>
            fwdOrRvsLabelMultimap;

    #ifdef USEDEBUG
LogDebug("los" << endl);
#endif
    k->hlCreateLabelByDijkstraWithStalling(
        bTreeNodesRanked, nodesRanked,
        edgesWithViaOrelSource, edgesWithViaOrelTarget,
        fwdOrRvsLabelMultimap,
        sourceNodeId, hHop->GetIntval(),
        isForward->GetBoolval());
    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif


    //convert multimap back to OrderedRelation
    for (std::multimap<int, std::tuple<int, int, double>>::iterator
            fwdOrRvsLabelMultimapIter =
                fwdOrRvsLabelMultimap.begin();
            fwdOrRvsLabelMultimapIter !=
            fwdOrRvsLabelMultimap.end();
            ++fwdOrRvsLabelMultimapIter)
    {
        std::tuple<int, int, double> currTuple =
            (*fwdOrRvsLabelMultimapIter).second;

        TupleType* tupleType =
            fwdOrRvsLabelRel->GetTupleType();
        //TODO: nicht löschen oder doch?
        Tuple* insertTuple = new Tuple(
            tupleType);//TODO: num type
        // verwenden oder nicht?
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_HUB_NODE_ID_NEW_IN_LABEL_TUPLE,
            new CcInt(true,
                      (*fwdOrRvsLabelMultimapIter).first));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE,
            new CcInt(true, std::get<
                      HubLabelClass::HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE
                      - 1>(currTuple)));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_HUB_PARENT_NODE_ID_IN_LABEL_TUPLE,
            new CcInt(true, std::get<
            HubLabelClass::HL_INDEX_OF_HUB_PARENT_NODE_ID_IN_LABEL_TUPLE
                      - 1>(currTuple)));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_HUB_PARENT_TUPLE_ID_IN_LABEL_TUPLE,
            new CcInt(true, -1));
        insertTuple->PutAttribute(
            HubLabelClass::HL_INDEX_OF_HUB_DISTANCE_TO_SOURCE_IN_LABEL_TUPLE,
            new CcReal(true, std::get<
            HubLabelClass::HL_INDEX_OF_HUB_DISTANCE_TO_SOURCE_IN_LABEL_TUPLE
                       - 2>(currTuple)));
                       //its -2 here because the multimap does not
        // contain the hubTupleId usually on indexposition #3
        fwdOrRvsLabelRel->AppendTuple(insertTuple);
        insertTuple->DeleteIfAllowed(); //TODO: warum muss der Tuple gelöscht
        // werden? wird er per beim Append geklont? Nicht dass
        // das Löschen hier den Tuple in der Relation zerstört.
    }

    //free resources
    delete k;

    return 0;
}



int hlFillForwardOrReverseLabelVM (Word* args,
                                   Word& result, int message,
                                   Word& local, Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    Relation* fwdOrRvsLabelRelation = (Relation*)
                                      args[0].addr;
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    NestedRelation* allLabelsNrel = (NestedRelation*)
                                    result.addr;
    // cast the result

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif


    std::multimap<int, std::tuple<int, int, double>>
            labelfwdOrRvsMultimap;

    int rootNodeId = -1;

    GenericRelationIterator* fwdOrRvsLabelIter =
        fwdOrRvsLabelRelation->MakeScan();
    Tuple* currTuple =
        fwdOrRvsLabelIter->GetNextTuple();
    while(currTuple)
    {
        CcInt* currTupleHubIdNew = (CcInt*)
                                   currTuple->GetAttribute(
                HubLabelClass::HL_INDEX_OF_HUB_NODE_ID_NEW_IN_LABEL_TUPLE);
        CcInt* currTupleHubID = (CcInt*)
                                currTuple->GetAttribute(
                HubLabelClass::HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE);
        CcInt* currTupleHubParentId = (CcInt*)
                                      currTuple->GetAttribute(
            HubLabelClass::HL_INDEX_OF_HUB_PARENT_NODE_ID_IN_LABEL_TUPLE);
        CcReal* currTupleHubDistanceToSource = (CcReal*)
                                               currTuple->GetAttribute(
            HubLabelClass::HL_INDEX_OF_HUB_DISTANCE_TO_SOURCE_IN_LABEL_TUPLE);

        #ifdef USEDEBUG
LogDebug("fill Label nex tuple with rank: " <<
              currTupleHubIdNew->GetIntval() << " id: " <<
              currTupleHubID->GetIntval() << " parentId: " <<
              currTupleHubParentId->GetIntval() << endl);
#endif
        std::tuple<int, int, double> insertTupleLabel =
            std::make_tuple(
                currTupleHubID->GetIntval(),
                currTupleHubParentId->GetIntval(),
                currTupleHubDistanceToSource->GetRealval());

        labelfwdOrRvsMultimap.insert(
            pair<int, std::tuple<int, int, double>>(
                currTupleHubIdNew->GetIntval(),
                insertTupleLabel));

        //getRootNodeId
        if(currTupleHubParentId->GetIntval() == -1)
        {
            rootNodeId = currTupleHubID->GetIntval();
        }
        //Free Outgoing-Iteration
        currTuple->DeleteIfAllowed();
        currTuple = 0;
        currTuple = fwdOrRvsLabelIter->GetNextTuple();
    }
    delete fwdOrRvsLabelIter;


    SubRelation* fwdOrRvsDataSubRel =
        allLabelsNrel->getSubRel(
            HubLabelClass::hlForwardLabelColumnName());
    Relation* fwdOrRvsDataRel =
        fwdOrRvsDataSubRel->rel;

    SmiFileId fileId = fwdOrRvsDataSubRel->fileId;
    ListExpr typeInfo = fwdOrRvsDataSubRel->typeInfo;
    AttributeRelation* labelfwdOrRvsArel = new
    AttributeRelation(fileId,
                      typeInfo);
    labelfwdOrRvsArel->setPartOfNrel(true);


    #ifdef USEDEBUG
LogDebug("los" << endl);
#endif

    k->hlFillForwardOrReverseLabel(labelfwdOrRvsArel,
                                   fwdOrRvsDataRel,
                                   labelfwdOrRvsMultimap);
    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif

    //leere zweite Arel erzeugen
    SubRelation* fwdOrRvsDataSubRel2 =
        allLabelsNrel->getSubRel(
            HubLabelClass::hlReverseLabelColumnName());

    SmiFileId fileId2 = fwdOrRvsDataSubRel2->fileId;
    ListExpr typeInfo2 =
        fwdOrRvsDataSubRel2->typeInfo;
    AttributeRelation* labelfwdOrRvsArel2 = new
    AttributeRelation(fileId2,
                      typeInfo2);
    labelfwdOrRvsArel2->setPartOfNrel(true);

    TupleType* tupleNumTypeAllLabels =
        allLabelsNrel->getPrimary()->GetTupleType();
    Tuple* allLabelsTuple = new Tuple(
        tupleNumTypeAllLabels);
    allLabelsTuple->PutAttribute(
        HubLabelClass::HL_INDEX_OF_NODE_ID_IN_ALL_LABELS_TUPLE,
        new CcInt(true, rootNodeId));
    allLabelsTuple->PutAttribute(
        HubLabelClass::HL_INDEX_OF_FORWARD_LABEL_IN_ALL_LABELS_TUPLE,
        labelfwdOrRvsArel);
    allLabelsTuple->PutAttribute(
        HubLabelClass::HL_INDEX_OF_REVERSE_LABEL_IN_ALL_LABELS_TUPLE,
        labelfwdOrRvsArel2);

    allLabelsNrel->getPrimary()->AppendTuple(
        allLabelsTuple);

    allLabelsTuple->DeleteIfAllowed();
    allLabelsTuple = 0;

    //free resources
    delete k;

    return 0;
}


int hlGetPathViaPointsVM (Word* args,
                          Word& result, int message,
                          Word& local, Supplier s)
{
    #ifdef USEDEBUG
    LogDebug("start" << endl);
    #endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    NestedRelation* allLabelsNRel = (NestedRelation*)
                                    args[0].addr;
    BTree* allLabelsBTree = (BTree*) args[1].addr;
    OrderedRelation* edgesWithShortcutsOrelSource =
        (OrderedRelation*)
        args[2].addr;
    CcInt* rootNodeId = (CcInt*) args[3].addr;
    CcInt* hubId = (CcInt*) args[4].addr;
    CcBool* isForward = (CcBool*) args[5].addr;
    #ifdef USEDEBUG
    LogDebug("Parameter gelesen" << endl);
    #endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    Relation* shortesPath = (Relation*)
                            result.addr; // cast the result

    #ifdef USEDEBUG
    LogDebug("result gelesen" << endl);
    #endif

    BTreeIterator* bTreeSourceIter =
        allLabelsBTree->ExactMatch(rootNodeId);

    int tupleIdSource = -1;
    if(bTreeSourceIter->Next())
    {
        tupleIdSource = bTreeSourceIter->GetId();
        #ifdef USEDEBUG
    LogDebug("id: " << tupleIdSource << endl);
    #endif
    }
    else
    {
        return 1;
    }
    TupleId currTupleId = (TupleId) tupleIdSource;
    Relation* primary = allLabelsNRel->getPrimary();
    Tuple* currTuple = primary->GetTuple(currTupleId,
                                         false);
    int arelIndex = -1;
    string columnNameArel = "";
    if(isForward->GetBoolval())
    {
        arelIndex =
            HubLabelClass::HL_INDEX_OF_FORWARD_LABEL_IN_ALL_LABELS_TUPLE;
        columnNameArel =
            HubLabelClass::hlForwardLabelColumnName();
    }
    else
    {
        arelIndex =
            HubLabelClass::HL_INDEX_OF_REVERSE_LABEL_IN_ALL_LABELS_TUPLE;
        columnNameArel =
            HubLabelClass::hlReverseLabelColumnName();
    }
    AttributeRelation* currAttrRel =
        (AttributeRelation*)
        currTuple->GetAttribute(arelIndex);
    SubRelation* dataSubRel =
        allLabelsNRel->getSubRel(columnNameArel);
    Relation* dataRel = dataSubRel->rel;

    DbArray<TupleId>* tupleIds =
        currAttrRel->getTupleIds();
    TupleId tid;
    Tuple* currDataTuple;
    for(int i = 0; i < tupleIds->Size(); i++)
    {
        tupleIds->Get(i, tid);
        #ifdef USEDEBUG
        LogDebug("next TupleId: " << tid << endl);
        #endif
        Tuple* tmpCurrDataTuple = dataRel->GetTuple(tid,
                                  false);

        if(tmpCurrDataTuple)
        {
            CcInt* currDataTupleHubId = (CcInt*)
                                        tmpCurrDataTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE);
            #ifdef USEDEBUG
            LogDebug("currNodeId: " <<
                  currDataTupleHubId->GetIntval() <<
                  " searchedHubId: " << hubId->GetIntval()<< endl);
            #endif
            if(hubId->GetIntval() ==
                    currDataTupleHubId->GetIntval())
            {
                #ifdef USEDEBUG
                LogDebug("Tuple found for given hubId, break loop and hold"
                      " current Tuple object" << endl);
                #endif
                currDataTuple = tmpCurrDataTuple;
                break;
            }
        }
        //mpCurrDataTuple->DeleteIfAllowed(); //XXTODO darf das?
    }
    tupleIds->destroyIfNonPersistent(); //XXTODO: richig so?

    if(!currDataTuple)
    {
        #ifdef USEDEBUG
        LogDebug("Warnung: kein Tuple in Arel gefunden, breche ab"
              << endl);
        #endif
        delete k;
        currTuple->DeleteIfAllowed();
        return 0;
    }


    std::vector<Tuple*> shortestPathVector;

    #ifdef USEDEBUG
    LogDebug("los" << endl);
    #endif
    k->hlGetPathViaPoints( edgesWithShortcutsOrelSource, dataRel,
        currDataTuple, shortestPathVector, isForward->GetBoolval());
    #ifdef USEDEBUG
    LogDebug("fertig" << endl);
    #endif


    //convert multimap back to OrderedRelation
    for (std::vector<Tuple*>::iterator shortesPathIter
            =
                shortestPathVector.begin();
            shortesPathIter != shortestPathVector.end();
            ++shortesPathIter)
    {
        Tuple* currInsertTuple = *(shortesPathIter);

        shortesPath->AppendTuple(currInsertTuple);
        currInsertTuple->DeleteIfAllowed(); //TODO: warum muss der Tuple
        // gelöscht werden? wird er per beim Append geklont? Nicht dass das
        // Löschen hier den Tuple in der Relation zerstört.
    }

    //free resources
    delete k;
    currTuple->DeleteIfAllowed();
    //currHub->DeleteIfAllowed(); //wird bereits in er Methode gelöscht
    //XXTODO: was alles löschen hier?

    return 0;
}



int hlQueryVM (Word* args, Word& result,
               int message, Word& local, Supplier s)
{
    #ifdef USEDEBUG
    LogDebug("start" << endl);
    #endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    #ifdef USEPERF
    double perfTime = k->getCurrentTimeInMs();
    double perfTime2 = 0.0;
    #endif

    NestedRelation* allLabelsNRel = (NestedRelation*) args[0].addr;
    BTree* allLabelsBTree = (BTree*) args[1].addr;
    OrderedRelation* edgesWithShortcutsOrelSource =
     (OrderedRelation*) args[2].addr;
    OrderedRelation* hlGraphOrel = (OrderedRelation*) args[3].addr;
    CcInt* sourceNodeId = (CcInt*) args[4].addr;
    CcInt* targetNodeId = (CcInt*) args[5].addr;
    CcBool* isHlGraph = (CcBool*) args[6].addr;

    if(isHlGraph->GetBoolval() && !hlGraphOrel)
    {
        LogError(
         "Fehlerhafte Parameter: isHlGraph == true but hlGraphOrel is null"
         << endl);
    }

    #ifdef USEDEBUG
    LogDebug("Parameter gelesen" << endl);
    #endif

    result = qp->ResultStorage(s);       // use the result storage
    Relation* shortesPath = (Relation*) result.addr; // cast the result

    #ifdef USEDEBUG
    LogDebug("result gelesen" << endl);
    #endif

    #ifdef USEDEBUG
    LogDebug("los" << endl);
    #endif

    k->hlQuery(allLabelsNRel, allLabelsBTree, edgesWithShortcutsOrelSource,
     hlGraphOrel, shortesPath, sourceNodeId, targetNodeId,
     isHlGraph->GetBoolval());

    #ifdef USEDEBUG
    LogDebug("fertig" << endl);
    #endif

    //free resources
    delete k;

    #ifdef USEPERF
    perfTime2 = k->getCurrentTimeInMs();
    LogPerf("perf: duration of full hl search: (ms) "
     << fixed << (perfTime2 - perfTime)<< endl);
    #endif

    return 0;
}


 //XXTODO
int hlPruneLabelByBootstrappingVM (Word* args,
                                   Word& result, int message,
                                   Word& local, Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    /*
    OrderedRelation* nodesWithRankOrelRank = (OrderedRelation*) args[0].addr;
    OrderedRelation* edgesWithViaOrelSource = (OrderedRelation*) args[1].addr;
    OrderedRelation* edgesWithViaOrelTarget = (OrderedRelation*) args[2].addr;
    CcInt* hHop = (CcInt*) args[3].addr;
    CcInt* calcFunction = (CcInt*) args[4].addr;
   */
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif

    #ifdef USEDEBUG
LogDebug("los" << endl);
#endif
    //k->hlPruneLabelByBootstrapping(hHop);
    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif

    //free resources
    delete k;

    //Änderungen an als Parameter übergebenen Orels persistieren
    qp->SetModified(qp->GetSon(s,1));
    qp->SetModified(qp->GetSon(s,2));

    return 0;
}


 //XXTODO
int hlReorderLabelsVM (Word* args, Word& result,
                       int message, Word& local
                       , Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    /*
    OrderedRelation* nodesWithRankOrelRank = (OrderedRelation*) args[0].addr;
    OrderedRelation* edgesWithViaOrelSource = (OrderedRelation*) args[1].addr;
    OrderedRelation* edgesWithViaOrelTarget = (OrderedRelation*) args[2].addr;
    CcInt* hHop = (CcInt*) args[3].addr;
    CcInt* calcFunction = (CcInt*) args[4].addr;
   */
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif

    #ifdef USEDEBUG
LogDebug("los" << endl);
#endif
    //k->hlReorderLabels(hHop);
    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif

    //free resources
    delete k;

    //Änderungen an als Parameter übergebenen Orels persistieren
    qp->SetModified(qp->GetSon(s,1));
    qp->SetModified(qp->GetSon(s,2));

    return 0;
}


 //XXTODO
int hlCreateLabelsVM (Word* args, Word& result,
                      int message, Word& local,
                      Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start" << endl);
#endif
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    BTree* bTreeNodesWithRankOrelRank =
        (BTree*) args[0].addr;
    OrderedRelation* nodesWithRankOrelRank =
        (OrderedRelation*) args[1].addr;
    OrderedRelation* edgesWithViaOrelSource =
        (OrderedRelation*) args[2].addr;
    OrderedRelation* edgesWithViaOrelTarget =
        (OrderedRelation*) args[3].addr;
    CcInt* hHop = (CcInt*) args[4].addr;
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    NestedRelation* allLabelsNrel = (NestedRelation*)
                                    result.addr;
    // cast the result

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif

    #ifdef USEDEBUG
LogDebug("los" << endl);
#endif
    k->hlCreateLabels(allLabelsNrel,
                      bTreeNodesWithRankOrelRank,
                      nodesWithRankOrelRank, edgesWithViaOrelSource,
                      edgesWithViaOrelTarget,
                      hHop->GetIntval());
    #ifdef USEDEBUG
LogDebug("fertig" << endl);
#endif

    //free resources
    delete k;

    return 0;
}




/*
5.2.2 Value Mappings of (2) In-Memory Approach

*/


/*
 Takes an OSM-Graph (Orel Edges) and creates an HlGraph.
 Retrieves an Orel-Representation of that graph.
 Only used for testing purposes.


*/
int hlTransformOrelToHlGraphVM (Word* args, Word& result,
                    int message, Word& local, Supplier s)
{
    #ifdef USEDEBUG
    LogDebug("start hlTransformOrelToHlGraphVM" << endl);
    #endif

    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* edgesOrelSource = (OrderedRelation*) args[0].addr;
    OrderedRelation* nodesOrel = (OrderedRelation*) args[1].addr;
    #ifdef USEDEBUG
    LogDebug("Parameter gelesen" << endl);
    #endif

    result = qp->ResultStorage(s);       // use the result storage
    OrderedRelation* reTransformedGraphOrel =
        (OrderedRelation*) result.addr; // cast the result

    #ifdef USEDEBUG
    LogDebug("result gelesen" << endl);
    #endif

    //initialize vector with number of nodes
    //  such that we can use []-operator of vector instead using push_back
    std::vector<HlNodeEntry*> nodesEdgesVector(nodesOrel->GetNoTuples());

    #ifdef USEDEBUG
    LogDebug("los" << endl);
    #endif
    k->hlTransformOrelToHlGraph(nodesOrel, edgesOrelSource, nodesEdgesVector);
    #ifdef USEDEBUG
    LogDebug("fertig" << endl);
    #endif


    //convert multimap back to OrderedRelation
    for (uint currSourceNodeIndex = 0;
     currSourceNodeIndex < nodesEdgesVector.size();
     currSourceNodeIndex++)
    {
        HlNodeEntry* currNodeEntry = nodesEdgesVector[currSourceNodeIndex];
        int currSourceNodeId = currNodeEntry->getNodeId();
        int currNodeRank = currNodeEntry->getRankValue();
        std::vector<HlEdgeEntry*>* currNodeVector =
         currNodeEntry->getEdgesVector();

        for (std::vector<HlEdgeEntry*>::iterator
                edgesVectorIter = currNodeVector->begin();
                edgesVectorIter != currNodeVector->end();
                ++edgesVectorIter)
        {
            //get next edge
            HlEdgeEntry* currEdgeEntry = *edgesVectorIter;

            int currTargetNodeIndex = currEdgeEntry->getTargetIndex();

            int currEdgeParentIndexForward =
             currEdgeEntry->getParentIndexForward();
            int currEdgeParentIndexReverse =
             currEdgeEntry->getParentIndexReverse();

            int currTargetNodeId =  -1;
            if(currTargetNodeIndex > -1)
            {
                HlNodeEntry* currTargetNodeEntry =
                 nodesEdgesVector[currTargetNodeIndex];
                currTargetNodeId = currTargetNodeEntry->getNodeId();
            }

            int currEdgeParentIdForward = -1;
            if(currEdgeParentIndexForward > -1)
            {
                HlNodeEntry* currParentNodeEntryForward =
                 nodesEdgesVector[currEdgeParentIndexForward];
                currEdgeParentIdForward =
                 currParentNodeEntryForward->getNodeId();
            }

            int currEdgeParentIdReverse = -1;
            if(currEdgeParentIndexReverse > -1)
            {
                HlNodeEntry* currParentNodeEntryReverse =
                 nodesEdgesVector[currEdgeParentIndexReverse];
                currEdgeParentIdReverse =
                 currParentNodeEntryReverse->getNodeId();
            }

            int isForwardInt = 0;
            int isReverseInt = 0;
            if(currEdgeEntry->getIsForward())
            {
                isForwardInt = 1;
            }
            if(currEdgeEntry->getIsReverse())
            {
                isReverseInt = 1;
            }

            //create orelTuple
            ListExpr relTypeSource;
            nl->ReadFromString(HubLabelClass::hlGetHlGraphOrelTypeInfo(),
             relTypeSource);
            ListExpr tupleNumTypeSource =
             SecondoSystem::GetCatalog()->NumericType(
             nl->Second(relTypeSource));
            Tuple* insertTuple = new Tuple(tupleNumTypeSource);

            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_SOURCE_IN_HL_GRAPH,
                new CcInt(true, currSourceNodeId));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_TARGET_IN_HL_GRAPH,
                new CcInt(true, currTargetNodeId));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_SOURCE_INDEX_IN_HL_GRAPH,
                new CcInt(true, currSourceNodeIndex));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_TARGET_INDEX_IN_HL_GRAPH,
                new CcInt(true, currTargetNodeIndex));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_SOURCE_RANK_IN_HL_GRAPH,
                new CcInt(true, currNodeRank));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_IS_FORWARD_IN_HL_GRAPH,
                new CcInt(true, isForwardInt));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_IS_REVERSE_IN_HL_GRAPH,
                new CcInt(true, isReverseInt));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_PARENT_ID_FORWARD_IN_HL_GRAPH,
                new CcInt(true, currEdgeParentIdForward));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_PARENT_ID_REVERSE_IN_HL_GRAPH,
                new CcInt(true, currEdgeParentIdReverse));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_PARENT_INDEX_FORWARD_IN_HL_GRAPH,
                new CcInt(true, currEdgeParentIndexForward));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_PARENT_INDEX_REVERSE_IN_HL_GRAPH,
                new CcInt(true, currEdgeParentIndexReverse));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_WEIGHT_FORWARD_IN_HL_GRAPH,
                new CcReal(true, currEdgeEntry->getWeightForward()));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_WEIGHT_REVERSE_IN_HL_GRAPH,
                new CcReal(true, currEdgeEntry->getWeightReverse()));

            reTransformedGraphOrel->AppendTuple(insertTuple);

            insertTuple->DeleteIfAllowed();
            insertTuple = 0;

            delete currEdgeEntry;
        }
        delete currNodeVector;
        delete currNodeEntry;
    }

    #ifdef USEDEBUG
    LogDebug("finish hlTransformOrelToHlGraphVM" << endl);
    #endif


    //free resources
    delete k;

    return 1;

}


/*
 Takes an OSM-Graph (Orel Edges) and creates an HlGraph and contracts it.
 Retrieves an Orel-Representation of that contracted graph.
 This in turn can be used for performing a CH Search of to create HL Labels.


*/
int hlDoContractionOfHlGraphVM (Word* args, Word& result,
                    int message, Word& local, Supplier s)
{
    #ifdef USEINFO
    LogInfo("start hlDoContractionOfHlGraphVM" << endl);
    #endif

    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    #ifdef USEPERF
    double perfTime = k->getCurrentTimeInMs();
    double perfTime2 = 0.0;
    double perfTime3 = 0.0;
    double perfTime4 = 0.0;
    double perfTime5 = 0.0;
    double perfTime6 = 0.0;
    double perfTime7 = 0.0;
    double perfTime8 = 0.0;
    #endif


    OrderedRelation* edgesOrelSource = (OrderedRelation*) args[0].addr;
    OrderedRelation* nodesOrel = (OrderedRelation*) args[1].addr;
    CcInt* hHopSizeCcInt = (CcInt*) args[2].addr;
    CcInt* skipContractionRemainingCountCcInt = (CcInt*) args[3].addr;
    CcInt* currCalcFunctionCcInt = (CcInt*) args[4].addr;

    int skipContractionRemainingCount =
     skipContractionRemainingCountCcInt->GetIntval();
    if(skipContractionRemainingCount < 0)
    {
        skipContractionRemainingCount = 0;
    }

    int hHopSize = hHopSizeCcInt->GetIntval();

    if(hHopSize < 0)
    {
        hHopSize = 0;
    }

    #ifdef USEINFO
    LogInfo("Parameter gelesen" << endl);
    #endif

    result = qp->ResultStorage(s);       // use the result storage
    OrderedRelation* reTransformedGraphOrel =
        (OrderedRelation*) result.addr; // cast the result

    #ifdef USEINFO
    LogInfo("result gelesen" << endl);
    #endif

    int nodesCount = nodesOrel->GetNoTuples();
    k->setProgressInterval(nodesCount / 10); //ganzzahlige division

    //initialize vector with number of nodes
    //  such that we can use []-operator of vector instead using push_back
    std::vector<HlNodeEntry*> nodesEdgesVector(nodesCount);

    HubLabelClass::PriorityQueueType priorityQueue;

    #ifdef USEINFO
    LogInfo("los transform" << endl);
    #endif

    #ifdef USEPERF
    perfTime2 = k->getCurrentTimeInMs();
    #endif

    k->hlTransformOrelToHlGraph(nodesOrel, edgesOrelSource, nodesEdgesVector);

    #ifdef USEPERF
    perfTime3 = k->getCurrentTimeInMs();
    LogPerf("perf: duration of transformation: (ms) "
     << fixed << (perfTime3 - perfTime2) << endl);
    #endif


    #ifdef USEINFO
    LogInfo("init priority queue" << endl);
    #endif
    k->hlInitPriorityQueueOfHlGraph(nodesEdgesVector, priorityQueue,
        currCalcFunctionCcInt->GetIntval());

    #ifdef USEPERF
    perfTime4 = k->getCurrentTimeInMs();
    LogPerf("perf: duration of init priority queue: (ms) "
     << fixed << (perfTime4 - perfTime3) << endl);
    #endif


    #ifdef USEINFO
    LogInfo("los contraction" << endl);
    #endif
    k->hlDoContractionOfHlGraph(nodesEdgesVector, priorityQueue,
     hHopSize, skipContractionRemainingCount,
     currCalcFunctionCcInt->GetIntval());

    #ifdef USEPERF
    perfTime5 = k->getCurrentTimeInMs();
    LogPerf("perf: duration of full contraction: (ms) "
     << fixed << (perfTime5 - perfTime4)<< endl);
    #endif

    #ifdef USEINFO
    LogInfo("create upward downward graph" << endl);
    #endif
    k->hlCreateUpwardAndDownwardHlGraph(nodesEdgesVector);

    #ifdef USEPERF
    perfTime6 = k->getCurrentTimeInMs();
    LogPerf("perf: duration of up-/ downwardsgraph creation: (ms) "
     << fixed << (perfTime6 - perfTime5) << endl);
    #endif

    #ifdef USEINFO
    LogInfo("fertig" << endl);
    #endif


    //convert nodesEdgesVector back to OrderedRelation
    for (uint currSourceNodeIndex = 0;
     currSourceNodeIndex < nodesEdgesVector.size();
     currSourceNodeIndex++)
    {
        HlNodeEntry* currSourceNodeEntry =
         nodesEdgesVector[currSourceNodeIndex];
        int currSourceNodeId = currSourceNodeEntry->getNodeId();
        int currSourceNodeRank = currSourceNodeEntry->getRankValue();
        std::vector<HlEdgeEntry*>* currSourceNodeEdgesVector =
         currSourceNodeEntry->getEdgesVector();


         //check whether the currSourceNodeEdgesVector is empty
         //this is the case if we process the highest rank value
         //we need to insert an empty edge leading to target == -1
         // just for persistance purposes
         //because else we would lose information about the highest rank
         // vertext at all

         if(currSourceNodeEdgesVector->empty())
         {

            HlEdgeEntry* highestRankVertexDummyEdge = new HlEdgeEntry();
            highestRankVertexDummyEdge->setTargetIndex(-1);
            highestRankVertexDummyEdge->setIsForward(false);
            highestRankVertexDummyEdge->setIsReverse(false);
            highestRankVertexDummyEdge->setParentIndexForward(-1);
            highestRankVertexDummyEdge->setParentIndexReverse(-1);
            highestRankVertexDummyEdge->setWeightForward(0.0);
            highestRankVertexDummyEdge->setWeightReverse(0.0);
            currSourceNodeEdgesVector->push_back(highestRankVertexDummyEdge);
         }

        for (std::vector<HlEdgeEntry*>::iterator
            currSourceNodeEdgesVectorIter = currSourceNodeEdgesVector->begin();
            currSourceNodeEdgesVectorIter != currSourceNodeEdgesVector->end();
            ++currSourceNodeEdgesVectorIter)
        {

            //get next edge
            HlEdgeEntry* currEdgeEntry = *currSourceNodeEdgesVectorIter;

            int currTargetNodeIndex = currEdgeEntry->getTargetIndex();

            int currEdgeParentIndexForward =
             currEdgeEntry->getParentIndexForward();
            int currEdgeParentIndexReverse =
             currEdgeEntry->getParentIndexReverse();

            int currEdgeParentIdForward = -1;
            if(currEdgeParentIndexForward > -1)
            {
                HlNodeEntry* currParentNodeEntryForward =
                 nodesEdgesVector[currEdgeParentIndexForward];
                currEdgeParentIdForward =
                 currParentNodeEntryForward->getNodeId();
            }

            int currEdgeParentIdReverse = -1;
            if(currEdgeParentIndexReverse > -1)
            {
                HlNodeEntry* currParentNodeEntryReverse =
                 nodesEdgesVector[currEdgeParentIndexReverse];
                currEdgeParentIdReverse =
                 currParentNodeEntryReverse->getNodeId();
             }

            //in case of highest rank vertex the current targetIndex is -1
            int currTargetNodeId = -1;
            if(currTargetNodeIndex != -1)
            {
                HlNodeEntry* currTargetNodeEntry =
                 nodesEdgesVector[currTargetNodeIndex];

                currTargetNodeId = currTargetNodeEntry->getNodeId();
            }

            int isForwardInt = 0;
            int isReverseInt = 0;

            if(currEdgeEntry->getIsForward())
            {
                isForwardInt = 1;
            }

            if(currEdgeEntry->getIsReverse())
            {
                isReverseInt = 1;
            }

            //create orelTuple
            ListExpr relTypeSource;
            nl->ReadFromString(HubLabelClass::hlGetHlGraphOrelTypeInfo(),
             relTypeSource);
            ListExpr tupleNumTypeSource =
             SecondoSystem::GetCatalog()->NumericType(
             nl->Second(relTypeSource));
            Tuple* insertTuple = new Tuple(tupleNumTypeSource);


            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_SOURCE_IN_HL_GRAPH,
                new CcInt(true, currSourceNodeId));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_TARGET_IN_HL_GRAPH,
                new CcInt(true, currTargetNodeId));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_SOURCE_INDEX_IN_HL_GRAPH,
                new CcInt(true, currSourceNodeIndex));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_TARGET_INDEX_IN_HL_GRAPH,
                new CcInt(true, currTargetNodeIndex));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_SOURCE_RANK_IN_HL_GRAPH,
                new CcInt(true, currSourceNodeRank));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_IS_FORWARD_IN_HL_GRAPH,
                new CcInt(true, isForwardInt));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_IS_REVERSE_IN_HL_GRAPH,
                new CcInt(true, isReverseInt));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_PARENT_ID_FORWARD_IN_HL_GRAPH,
                new CcInt(true, currEdgeParentIdForward));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_PARENT_ID_REVERSE_IN_HL_GRAPH,
                new CcInt(true, currEdgeParentIdReverse));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_PARENT_INDEX_FORWARD_IN_HL_GRAPH,
                new CcInt(true, currEdgeParentIndexForward));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_PARENT_INDEX_REVERSE_IN_HL_GRAPH,
                new CcInt(true, currEdgeParentIndexReverse));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_WEIGHT_FORWARD_IN_HL_GRAPH,
                new CcReal(true, currEdgeEntry->getWeightForward()));
            insertTuple->PutAttribute(
                HubLabelClass::HL_INDEX_OF_WEIGHT_REVERSE_IN_HL_GRAPH,
                new CcReal(true, currEdgeEntry->getWeightReverse()));


            reTransformedGraphOrel->AppendTuple(insertTuple);

            insertTuple->DeleteIfAllowed();
            insertTuple = 0;

            delete currEdgeEntry;

        }
        delete currSourceNodeEdgesVector;
    }

    #ifdef USEPERF
    perfTime7 = k->getCurrentTimeInMs();
    LogPerf("perf: duration of retransforming into orel: (ms) "
     << fixed << (perfTime7 - perfTime6) << endl);
    #endif

    //again iterate through nodesEdgesVector to delete all nodeEntries
    // we could not do this before
    for(std::vector<HlNodeEntry*>::iterator nodesEdgesVectorIter2
     = nodesEdgesVector.begin(); nodesEdgesVectorIter2
     != nodesEdgesVector.end(); nodesEdgesVectorIter2++)
    {

        HlNodeEntry* currSourceNodeEntry2 = *nodesEdgesVectorIter2;
        delete currSourceNodeEntry2;
    }





    //free resources
    delete k;

    #ifdef USEPERF
    perfTime8 = k->getCurrentTimeInMs();
    LogPerf("perf: duration of clearing memory: (ms) "
     << fixed << (perfTime8 - perfTime7) << endl);
    LogPerf("perf: duration of full contraction operator: (ms) "
     << fixed << (perfTime8 - perfTime) << endl);
    #endif

    #ifdef USEINFO
    LogInfo("finish hlDoContractionOfHlGraphVM" << endl);
    #endif

    return 1;

}




/*
 Takes an Orel-Representation of a HlGraph.
 Performs a CH search.
 Retrieves the shortest path of the search represented by an Orel
  of edges from the given OSM-graph.


*/
int hlDoChSearchInHlGraphVM (Word* args, Word& result,
                    int message, Word& local, Supplier s)
{
    #ifdef USEINFO
    LogInfo("start hlDoChSearchInHlGraphVM" << endl);
    #endif

    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    #ifdef USEPERF
    double perfTime = k->getCurrentTimeInMs();
    double perfTime2 = 0.0;
    double perfTime3 = 0.0;
    double perfTime4 = 0.0;
    double perfTime5 = 0.0;
    double perfTime6 = 0.0;
    #endif


    OrderedRelation* nodesOrel = (OrderedRelation*) args[0].addr;
    OrderedRelation* edgesSourceOrel = (OrderedRelation*) args[1].addr;
    OrderedRelation* contractedHlGraphOrel = (OrderedRelation*) args[2].addr;
    CcInt* SourceCcInt = (CcInt*) args[3].addr;
    CcInt* targetCcInt = (CcInt*) args[4].addr;

    #ifdef USEINFO
    LogInfo("Parameter gelesen" << endl);
    #endif

    result = qp->ResultStorage(s);       // use the result storage
    Relation* shortestPathRel = (Relation*) result.addr; // cast the result

    #ifdef USEINFO
    LogInfo("result gelesen" << endl);
    #endif


    //prepare data
    bool isChElseHl = true;
    int source = SourceCcInt->GetIntval();
    int target = targetCcInt->GetIntval();
    std::vector<HlNodeEntry*> nodesEdgesVector(nodesOrel->GetNoTuples());

    #ifdef USEPERF
    perfTime2 = k->getCurrentTimeInMs();
    #endif

    #ifdef USEINFO
    LogInfo("convert hlGraphOrel to hlGraph" << endl);
    #endif
    k->hlTransformHlGraphOrelToHlGraph(contractedHlGraphOrel, nodesEdgesVector);

    #ifdef USEPERF
    perfTime3 = k->getCurrentTimeInMs();
    LogPerf("perf: duration of transformation: (ms) "
     << fixed << (perfTime3 - perfTime2) << endl);
    #endif

    #ifdef USEINFO
    LogInfo("do search" << endl);
    #endif
    std::tuple<int, double, std::map<int, ChNode*>> searchResultTuple =
    k->hlDoCHSearch(nodesEdgesVector, source, target, isChElseHl);

    #ifdef USEPERF
    perfTime4 = k->getCurrentTimeInMs();
    LogPerf("perf: duration CHSearch: (ms) "
     << fixed << (perfTime4 - perfTime3) << endl);
    #endif

    int shortestPathHubIndex = std::get<0>(searchResultTuple);
    std::map<int, ChNode*> searchTree = std::get<2>(searchResultTuple);

    #ifdef USEINFO
    double shortestPathLength = std::get<1>(searchResultTuple);
    #endif

    #ifdef USEINFO
    LogInfo("resolve shortestpath" << endl);
    #endif

    k->hlResolveShortestPathFromSearchTree(shortestPathHubIndex,
     nodesEdgesVector, searchTree, edgesSourceOrel, shortestPathRel);

    #ifdef USEPERF
    perfTime5 = k->getCurrentTimeInMs();
    LogPerf("perf: duration of resolving shortest path: (ms) "
     << fixed << (perfTime5 - perfTime4) << endl);
    #endif

    #ifdef USEINFO
    LogInfo("fertig" << endl);
    #endif

    if(shortestPathHubIndex != -1)
    {
        #ifdef USEINFO
        HlNodeEntry* hubNodeEntry = nodesEdgesVector[shortestPathHubIndex];
        int hubNodeId = hubNodeEntry->getNodeId();

        LogInfo("shortest path found start: " << source << " target: " << target
         << " hubNodeId: " << hubNodeId << " length: "
         << shortestPathLength << endl);
        #endif
    }
    else
    {
        #ifdef USEINFO
        LogInfo("no shortest path found start: "
         << source << " target: " << target
         << " hubNodeId: " << shortestPathHubIndex
         << " length: " << shortestPathLength << endl);
        #endif
    }

    //free resources
    k->hlFreeSearchTree(searchTree);
    k->hlFreeHlGraph(nodesEdgesVector);
    delete k;


    #ifdef USEPERF
    perfTime6 = k->getCurrentTimeInMs();
    LogPerf("perf: duration of clearing memory: (ms) "
     << fixed << (perfTime6 - perfTime5) << endl);
    LogPerf("perf: duration of complete CH serch operator: (ms) "
     << fixed << (perfTime6 - perfTime) << endl);
    #endif

    #ifdef USEINFO
    LogInfo("finish hlDoChSearchInHlGraphVM" << endl);
    #endif

    return 1;

}




/*
 Takes an Orel-Representation of a HlGraph.
 Performs multiple CH searches to create HubLabels.


*/
int hlCreateLabelsFromHlGraphVM (Word* args, Word& result,
                    int message, Word& local, Supplier s)
{
    #ifdef USEINFO
    LogInfo("start hlCreateLabelsFromHlGraphVM" << endl);
    #endif

    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    #ifdef USEPERF
    double perfTime = k->getCurrentTimeInMs();
    double perfTime2 = 0.0;
    double perfTime3 = 0.0;
    double perfTime4 = 0.0;
    double perfTime5 = 0.0;
    #endif


    OrderedRelation* nodesOrel = (OrderedRelation*) args[0].addr;
    OrderedRelation* contractedHlGraphOrel = (OrderedRelation*) args[1].addr;

    #ifdef USEINFO
    LogInfo("Parameter gelesen" << endl);
    #endif

    result = qp->ResultStorage(s);       // use the result storage
    NestedRelation* allLabelsNrel = (NestedRelation*) result.addr;

    #ifdef USEINFO
    LogInfo("result gelesen" << endl);
    #endif

    int nodesCount = nodesOrel->GetNoTuples();
    k->setProgressInterval(nodesCount / 10); //ganzzahlige division
    std::vector<HlNodeEntry*> nodesEdgesVector(nodesCount);

    #ifdef USEPERF
    perfTime2 = k->getCurrentTimeInMs();
    #endif

    #ifdef USEINFO
    LogInfo("convert hlGraphOrel to hlGraph" << endl);
    #endif
    k->hlTransformHlGraphOrelToHlGraph(contractedHlGraphOrel, nodesEdgesVector);

    #ifdef USEPERF
    perfTime3 = k->getCurrentTimeInMs();
    LogPerf("perf: duration of transformation: (ms) "
     << fixed << (perfTime3 - perfTime2)<< endl);
    #endif

    #ifdef USEINFO
    LogInfo("Create Labels" << endl);
    #endif
    //also does delete hlGraph objects
    k->hlCreateLabelsFromHlGraph(allLabelsNrel, nodesOrel, nodesEdgesVector);


    #ifdef USEPERF
    perfTime4 = k->getCurrentTimeInMs();
    LogPerf("perf: duration of creating all hublabels: (ms) "
     << fixed << (perfTime4 - perfTime3)<< endl);
    #endif

    //free resources
    k->hlFreeHlGraph(nodesEdgesVector);
    delete k;

    #ifdef USEPERF
    perfTime5 = k->getCurrentTimeInMs();
    LogPerf("perf: duration of complete operator for creating hublabels: (ms) "
     << fixed << (perfTime5 - perfTime)<< endl);
    #endif

    #ifdef USEINFO
    LogInfo("finish hlCreateLabelsFromHlGraphVM" << endl);
    #endif

    return 1;

}




/*
 Takes an Orel-Representation of an OSM-Graph.
 Expects the Osm-Orel to fulfil the schema of
  @see hlGetEdgesTupleTypeInfo()
 Iterates over every edge and retrieves its Curves' length.
 Gets the Edges RoadType and multiplies the length with a const value
  dependent of the RoadType representing the average speed driven
  when traversing this edge.


*/
int hlCalcWeightsOrelVM (Word* args, Word& result,
                    int message, Word& local, Supplier s)
{
    #ifdef USEDEBUG
    LogDebug("start hlCalcWeightsOrelVM" << endl);
    #endif

    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* edgesSourceOrel = (OrderedRelation*) args[0].addr;
    CcInt* calculationMode = (CcInt*) args[1].addr;

    #ifdef USEDEBUG
    LogDebug("Parameter gelesen" << endl);
    #endif

    result = qp->ResultStorage(s);       // use the result storage
    CcInt* resultInt = (CcInt*) result.addr; // cast the result
    resultInt->Set(true, 1);

    #ifdef USEDEBUG
    LogDebug("result gelesen" << endl);
    #endif


    #ifdef USEDEBUG
    LogDebug("los" << endl);
    #endif
    k->hlCalcWeightsOrel(edgesSourceOrel, calculationMode->GetIntval());


    #ifdef USEDEBUG
    LogDebug("finish" << endl);
    #endif


    //free resources
    delete k;

    //Änderungen an als Parameter übergebenen Orels persistieren
    qp->SetModified(qp->GetSon(s,0));

    #ifdef USEDEBUG
    LogDebug("finish hlCalcWeightsOrelVM" << endl);
    #endif

    return 1;

}







/*
5.2.3 Value Mappings of (3) In-Memory Approach using MainMemory2Algebra

not yet implemented right

*/

int hlContractMmGraphVM (Word* args, Word& result,
                    int message, Word& local, Supplier s)
{
    #ifdef USEDEBUG
LogDebug("start contractNewVM" << endl);
#endif

    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    CcString* oNUp = (CcString*) args[0].addr;
    CcString* oNDown = (CcString*) args[1].addr;
    Word* calcFunction = (Word*) args[2].addr;
    #ifdef USEDEBUG
LogDebug("Parameter gelesen" << endl);
#endif

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    #ifdef USEDEBUG
LogDebug("result gelesen" << endl);
#endif


    mm2algebra::MemoryGraphObject* memgraphUp =
      mm2algebra::getMemGraph(oNUp);
    mm2algebra::MemoryGraphObject* memgraphDown =
      mm2algebra::getMemGraph(oNDown);

    if(!memgraphUp || !memgraphDown) {
        return 0;
    }

    graph::Graph* graphUp = memgraphUp->getgraph();
    graph::Graph* graphDown = memgraphDown->getgraph();

    k->hlContractMmGraph(graphUp, graphDown, calcFunction);


    #ifdef USEDEBUG
LogDebug("finish contractNewVM" << endl);
#endif

    return 1;

}





/*
5.3 Specification

The specification provides an operator description for the user.
The first argument of the ~OperatorSpec~ constructor is a description
of the type mapping, the second argument describes the syntax of
the operator, than comes the operator's  meaning and the last argument
used here is an example query. If required, an additional
argument can provide some remark to this operator.

All specifications are related to the equal named function of the class
HubLabelClass.
The specifications just got an additional suffix 'Spec'

5.3.1 Specification of (1) Persistent Approach

*/


OperatorSpec hlCalcRankSpec(
    "orel x orel x ccint -> real",
    "hlCalcRank(_, _, _)",
    "For testing purposes. "
    "Computes and returns the rank of the given nodeId.",
    "query hlCalcRank(EdgesOrelSource feed head[10] oconsume,"
    " EdgesOreltarget feed head[10] oconsume, 12345678)"
);


OperatorSpec hlOneHopReverseSearchSpec(
    "orel x orel x ccint -> orel",
    "hlOneHopReverseSearch(_, _, _)",
    "For testing purposes. "
    "Processes a one hop reverse Search and returns an orel containing"
    " the results.",
    "query hlOneHopReverseSearch(EdgesTestOrelSource, EdgesTestOrelTarget,"
    " 7264168)"
);


OperatorSpec hlHHopForwardSearchSpec(
    "orel x ore x orel x ccint x ccint -> real",
    "hlHHopForwardSearch(_, _, _, _, _)",
    "For testing purposes. "
    "hlHHopForwardSearch(orelEdgesSource, orelEdgesTarget,"
    " multimapReverseSearchXT, contractNodeV, hHop)",
    "query hlHHopForwardSearch(orelEdgesSource, orelEdgesTarget,"
    " multimapReverseSearchXT, 1505, -1) feed consume"
);


OperatorSpec hlForwardSearchGetDistSpec(
    "orel x ccint x ccint -> real",
    "hlForwardSearchGetDist(_, _, _)",
    "For testing purposes. "
    "hlForwardSearchGetDist(oneHopReverseSearchOrelXT, contractNodeV,"
    " currentTargetNodeT)",
    "query hlForwardSearchGetDist(OneHopReverseSearchToBeDeleted, 7264168,"
    " 7264033)"
);


OperatorSpec hlRemoveTFromCurrentWitnessListSpec(
    "orel x ccint -> orel",
    "hlRemoveTFromCurrentWitnessList(_, _)",
    "For testing purposes. "
    "hlRemoveTFromCurrentWitnessList(oneHopReverseSearchOrelXT,"
    " currentTargetNodeT)",
    "query hlRemoveTFromCurrentWitnessList(OneHopReverseSearchToBeDeleted,"
    " 7264033)"
);


OperatorSpec
hlForwardSearchCheckForWitnessPathSpec(
    "orel x ccint x ccint x ccreal x ccreal -> orel",
    "hlForwardSearchCheckForWitnessPath(_, _, _, _, _)",
    "For testing purposes. "
    "hlForwardSearchCheckForWitnessPath(oneHopReverseSearchOrelXT,"
    " contractNodeV, currentNodeX, distSX, distSV)",
    "query hlForwardSearchCheckForWitnessPath("
    "OneHopReverseSearchToBeDeleted2, 1505, 1495, 6.16, 42,453) feed"
    " consume"
);


OperatorSpec
hlInsertOrUpdateTupleInNotYetVisitedListSpec(
    "rel x ccint x ccint x ccint -> orel",
    "hlInsertOrUpdateTupleInNotYetVisitedList(_, _, _, _)",
    "For testing purposes. "
    "hlInsertOrUpdateTupleInNotYetVisitedList(EdgesSourceTmpRel,"
    " currentNodeV, currentNodeS, isForward)",
    "query hlInsertOrUpdateTupleInNotYetVisitedList(EdgesTestRelSourceTmp,"
    " 1492, 1424, 1) feed consume"
);


OperatorSpec
hlForwardSearchIterativeStepsScanNewVerticesSpec(
    "orel x orel x orel x orel x rel x ccint x ccreal -> ccin",
    "hlForwardSearchIterativeStepsScanNewVertices(_, _, _, _, _, _, _, _)",
    "For testing purposes. "
    "hlForwardSearchIterativeStepsScanNewVertices(EdgesTestOrelSource,"
    " hlNotYetVisitedNodesTest, stillVisitedNodesOrel,"
    "  OneHopReverseSearchToBeDeleted2,"
    "  currMinNotYetVisitedNodesSingleTupleOrelFwdW, 1424, 30.00)",
    "query hlForwardSearchIterativeStepsScanNewVertices("
    "EdgesTestOrelSource, hlNotYetVisitedNodesTest, stillVisitedNodesOrel,"
     "  OneHopReverseSearchToBeDeleted2,"
     " currMinNotYetVisitedNodesSingleTupleOrelFwdW, 1424, 30.00)"
);


OperatorSpec
hlForwardSearchProcessIncomingEdgeIterativeStepsSpec(
    "orel x orel x orel x orel x ccint x ccint x ccreal -> cint",
    "hlForwardSearchProcessIncomingEdgeIterativeSteps(_, _, _, _, _, _, _,"
    " _)",
    "For testing purposes. "
    "hlForwardSearchProcessIncomingEdgeIterativeSteps(EdgesTestOrelSource,"
    " hlNotYetVisitedNodesTest, stillVisitedNodesOrel,  hHop, nodeV,"
    " distSV)",
    "query hlForwardSearchProcessIncomingEdgeIterativeSteps("
    "EdgesTestOrelSource, hlNotYetVisitedNodesTest, stillVisitedNodesOrel,"
    " OneHopReverseSearchToBeDeleted2, 999, 1424, 30.00)"
);


OperatorSpec
hlForwardSearchProcessIncomingEdgeInitialStepsSpec(
    "orel x orel x orel x ccint x ccint x ccreal -> cint",
    "hlForwardSearchProcessIncomingEdgeInitialSteps(_, _, _, _, _, _,)",
    "For testing purposes. "
    "hlForwardSearchProcessIncomingEdgeInitialSteps(EdgesTestOrelSource,"
    " hlNotYetVisitedNodesTes, hlOneHopReverseSearchXTOrel, nodeS, nodeV,"
    " distSV)",
    "query hlForwardSearchProcessIncomingEdgeInitialSteps("
    "hlEdgesOrelSource, hlNotYetVisitedNodesCopyOrelDist,"
    " hlOneHopReverseSearchXTOrelX3, 1492, 1505, 6.16);"
);


OperatorSpec
hlForwardSearchCreateAndAppendShortcutsSpec(
    "orel x ccint x ccint x ccreal -> orel",
    "hlForwardSearchCreateAndAppendShortcuts(_, _, _, _)",
    "For testing purposes. "
    "hlForwardSearchCreateAndAppendShortcuts(copyMultimapReverseSearchXT,"
    " currentSourceNodeFwdS, currentVertexIdFwdV, distSV) feed consume",
    "query hlForwardSearchCreateAndAppendShortcuts("
    "copyMultimapReverseSearchXT, 1492, 1505, 1000.0) feed consume"
);


OperatorSpec
hlForwardSearchProcessIncomingEdgeSpec(
    "orel x orel x orel x orel x ccint x ccint x ccint x ccreal -> orel",
    "hlForwardSearchProcessIncomingEdge(_, _, _, _, _, _, _, _)",
    "For testing purposes. "
    "hlForwardSearchProcessIncomingEdge(hlEdgesOrelSource,"
    " stillVisitedNodesOrel, copyMultimapReverseSearchXT,"
    " hlShortcutstoBeAdded, currentSourceNodeFwdS, currentVertexIdFwdV,"
    " hDepth, distSV) feed consume",
    "query hlForwardSearchProcessIncomingEdge(hlEdgesOrelSource,"
    " stillVisitedNodesOrel, copyMultimapReverseSearchXT,"
    " hlShortcutstoBeAdded, 1492, 1505, 2, 1000.00) feed consume"
);


OperatorSpec
hlRemoveContractedEdgesFromEdgesRelationsSpec(
    "orel x orel x ccint -> ccint",
    "hlRemoveContractedEdgesFromEdgesRelations(_, _, _)",
    "For testing purposes. "
    "hlRemoveContractedEdgesFromEdgesRelations(hlEdgesOrelSourceCopy,"
    " hlEdgesOrelTargetCopy, hlCurrentContractV)",
    "query hlRemoveContractedEdgesFromEdgesRelations(hlEdgesOrelSource,"
    " hlEdgesOrelTarget, 1505)"
);


OperatorSpec
hlRemoveParallelEdgesFromEdgesRelationsSpec(
    "orel x orel x orel -> ccint",
    "hlRemoveParallelEdgesFromEdgesRelations(_, _, _)",
    "For testing purposes. "
    "hlRemoveParallelEdgesFromEdgesRelations(hlEdgesOrelSourceCopy,"
    " hlEdgesOrelTargetCopy, hlShortcutsToBeCreatedOrelToBeDeleted)",
    "query hlRemoveParallelEdgesFromEdgesRelations(hlEdgesOrelSource,"
    " hlEdgesOrelTarget, hlShortcutsToBeCreatedOrelToBeDeleted)"
);


OperatorSpec hlDoContractionSpec(
    "orel x orel x ccint x ccint -> orel",
    "hlDoContraction(_, _, _, _)",
    "For testing purposes. "
    "hlDoContraction(edgesWithViaOrelSource, edgesWithViaOrelTarget,"
    " currentVToBeContracted, hHop) feed consume",
    "query hlDoContraction(edgesWithViaOrelSource, edgesWithViaOrelTarget,"
    " 1505, 2) feed consume"
);


OperatorSpec
hlIterateOverAllNodesByRankAscAndDoContractionSpec(
    "orel x orel x orel x ccint -> ccint",
    "hlIterateOverAllNodesByRankAscAndDoContraction(_, _, _, _)",
    "For testing purposes. "
    "hlIterateOverAllNodesByRankAscAndDoContraction(nodesWithRankOrelRank,"
    " edgesWithViaOrelSource, edgesWithViaOrelTarget, hHop)",
    "query hlIterateOverAllNodesByRankAscAndDoContraction("
    "nodesWithRankOrelRank, edgesWithViaOrelSource,"
    " edgesWithViaOrelTarget, 2)"
);


OperatorSpec
hlCreateLabelCheckForWitnessScanNewVerticesSpec(
    "orel x orel x ccint x ccint x ccreal x ccint (ccbool) -> orel",
    "hlCreateLabelCheckForWitnessScanNewVertices(_, _, _, _, _, _)",
    "For testing purposes. "
    "hlCreateLabelCheckForWitnessScanNewVertices(hlUpwardEdgesOrelSource,"
    " hlDownwardEdgesOrelTarget, nextNodeToScan, nextNodeHHop,"
    " NextNodeDistToSource, isForward) feed consume",
    "query hlCreateLabelCheckForWitnessScanNewVertices("
    "hlUpwardEdgesOrelSource, hlDownwardEdgesOrelTarget, 1496, 1, 1000.0,"
    " 1) feed consume"
);


OperatorSpec hlCreateLabelCheckForWitnessSpec(
    "orel x orel x rel x ccint x ccint x ccreal x ccint (ccbool) -> ccint"
    " (ccbool)",
    "hlCreateLabelCheckForWitness(_, _, _, _, _, _)",
    "For testing purposes. "
    "hlCreateLabelCheckForWitness(hlUpwardEdgesOrelSource,"
    " hlDownwardEdgesOrelTarget, hlFwdOrRvsLabel, currentNode, hlhHop,"
    " currentNodeDistToSource, isForward)",
    "query hlCreateLabelCheckForWitness(hlUpwardEdgesOrelSource,"
    " hlDownwardEdgesOrelTarget, hlFwdOrRvsLabel, 273, 10, 0.0, 1)"
);


OperatorSpec hlCreateLabelScanNewVerticesSpec(
    "orel x orel x rel x ccint x ccreal x ccint (ccbool) -> orel",
    "hlCreateLabelScanNewVertices(_, _, _, _, _, _)",
    "For testing purposes. "
    "hlCreateLabelScanNewVertices(hlUpwardEdgesOrelSource,"
    " hlDownwardEdgesOrelTarget, hlFwdOrRvsLabel, currentNode,"
    " currentNodeDistToSource, isForward) feed consume",
    "query hlCreateLabelScanNewVertices(hlUpwardEdgesOrelSource,"
    " hlDownwardEdgesOrelTarget, hlFwdOrRvsLabel, 273, 0.0, 1) feed"
    " consume"
);


OperatorSpec hlGetRankByIdSpec(
    "btree x orel x ccint -> orel",
    "hlGetRankById(_, _, _)",
    "For testing purposes. "
    "hlGetRankById(hlNodesWithRankOrelRank_NodeIdNew,"
    " hlNodesWithRankOrelRank, givenNodeId) feed consume",
    "query hlGetRankById(hlNodesWithRankOrelRank_NodeIdNew,"
    " hlNodesWithRankOrelRank, 700) feed consume"
);


OperatorSpec
hlCreateLabelByDijkstraWithStallingSpec(
    "btree x orel x orel x orel x Ccint x ccint x ccint (ccbool) -> orel",
    "hlCreateLabelByDijkstraWithStalling(_, _, _, _, _, _, _)",
    "For testing purposes. "
    "hlCreateLabelByDijkstraWithStalling(hlNodesWithRankOrelRank_NodeIdNew,"
    " hlNodesWithRankOrelRank, hlEdgesWithViaOrelSource,"
    " hlEdgesWithViaOrelTarget, sourceNodeId, hHop, calcFunction) feed"
    " consume",
    "query hlCreateLabelByDijkstraWithStalling("
    "hlNodesWithRankOrelRank_NodeIdNew, hlNodesWithRankOrelRank,"
    " hlEdgesWithViaOrelSource, hlEdgesWithViaOrelTarget, 700, 10, 1)"
    " feed consume"
);


OperatorSpec hlFillForwardOrReverseLabelSpec(
    "Rel -> Nrel",
    "hlFillForwardOrReverseLabel(_)",
    "For testing purposes. "
    "hlFillForwardOrReverseLabel(fwdOrRvsLabelRel) feed consume",
    "query hlFillForwardOrReverseLabel(fwdOrRvsLabelRel) feed consume"
);


OperatorSpec hlGetPathViaPointsSpec(
    "Nrel x Btree x Orel x Int x Int x Bool -> Rel",
    "hlGetPathViaPoints(_, _, _, _, _, _)",
    "For testing purposes. "
    "hlGetPathViaPoints(allLabelsNRel, allLabelsBTree,"
    " hlEdgesOrelSourceParentVia, rootNodeId, hubId, isForward) feed"
    " consume",
    "query hlGetPathViaPoints(allLabelsNRel, allLabelsBTree,"
    " hlEdgesOrelSourceParentVia, 62, 630, 1) feed consume"
);


OperatorSpec hlQuerySpec(
    "Nrel x Btree x Orel x Int x Int x Bool -> Rel",
    "hlQuery(_, _, _, _, _, _)",
    "Performs a HubLabeling query using the given Nrel containing all Hub Label"
    ", the BTree over this Nrel, an Orel containing all Edges ordered by"
    " its sourcenode, the sourceNodeId and targetNodeId of this search and"
    " and a boolean to define the mode (only use 1).",
    "query hlQuery(allLabelsNRel, allLabelsBTree,"
    " hlEdgesOrelSourceParentVia, 62, 630, 1)"
);


OperatorSpec hlPruneLabelByBootstrappingSpec(
    "Orel x Orel x Orel x Int x Int -> Int",
    "hlPruneLabelByBootstrapping(_, _, _, _, _)",
    "For testing purposes. "
    "hlPruneLabelByBootstrapping(nodesWithRankOrelRank,"
    " edgesWithViaOrelSource, edgesWithViaOrelTarget, hHop, calcFunction)",
    "query hlPruneLabelByBootstrapping(nodesWithRankOrelRank,"
    " edgesWithViaOrelSource, edgesWithViaOrelTarget, 2, 1)"
);


OperatorSpec hlReorderLabelsSpec(
    "Orel x Orel x Orel x Int x Int -> Int",
    "hlReorderLabels(_, _, _, _, _)",
    "For testing purposes. "
    "hlReorderLabels(nodesWithRankOrelRank, edgesWithViaOrelSource,"
    " edgesWithViaOrelTarget, hHop, calcFunction)",
    "query hlReorderLabels(nodesWithRankOrelRank, edgesWithViaOrelSource,"
    " edgesWithViaOrelTarget, 2, 1)"
);


OperatorSpec hlCreateLabelsSpec(
    "Btree x Orel x Orel x Orel x Int -> Nrel",
    "hlCreateLabels(_, _, _, _)",
    "For testing purposes. "
    "query hlCreateLabels(hlNodesWithRankOrelRank_NodeIdNew,"
    " nodesWithRankOrelRank, hlUpwardEdgesOrelSourceParentVia,"
    " hlDownwardEdgesOrelTargetParentVia, hHop) feed consume)",
    "query hlCreateLabels(hlNodesWithRankOrelRank_NodeIdNew,"
    " nodesWithRankOrelRank, hlUpwardEdgesOrelSourceParentVia,"
    " hlDownwardEdgesOrelTargetParentVia, 10) feed consume"
);






/*
5.3.2 Specification of (2) In-Memory Approach

*/

OperatorSpec hlTransformOrelToHlGraphSpec(
    "Orel x Orel -> Orel",
    "hlTransformOrelToHlGraph(_,_)",
    "For testing purposes. "
    "Reads the given edges-orel and nodes-orel into main memory"
    " and does nothing else",
    "query hlTransformOrelToHlGraph(hlEdgesOrelSource, hlNodesSourceOnlyOrel)"
        "feed consume"
);


OperatorSpec hlDoContractionOfHlGraphSpec(
    "Orel x Orel x Int x Int x Int -> Orel",
    "hlDoContractionOfHlGraph(_, _, _, _, _)",
    "Performs the contraction of the given edges-orel and nodes-orel."
    " Takes also a desired hop-size for h-hop searches, a skip-parameter"
    " to define to stop recalculating node-priorities after that many nodes"
    " are left to be contracted and at least it takes an integer to define"
    " which function shall be used to calculate node priorities"
    " (0 = EdgeDifference; 1 = Random; 2 = InputOrder)."
    " Returns the contracted graph (hlGraph) as OrderedRelation.",
    "query hlDoContractionOfHlGraph(hlEdgesOrelSource, hlNodesSourceOnlyOrel,"
        " 2, 50000, 0) feed consume"
);


OperatorSpec hlDoChSearchInHlGraphSpec(
    "Orel x Orel x Orel x Int x Int -> Rel",
    "hlDoChSearchInHlGraph(_, _, _, _, _)",
    "Performs a CH search using the given nodes-orel, edges-orel,"
    " the hlGraph of a previous contraction and the source- and targetNodeId"
    " Returns a relation containing all edges on the shortest path in"
    " their order of occurence on that path. Is empty if no path was found.",
    "query hlDoChSearchInHlGraph(hlNodesSourceOnlyOrel, hlEdgesOrelSource,"
        " hlGraphOrel, 1030, 2456) feed consume"
);


OperatorSpec hlCalcWeightsOrelSpec(
    "Orel x Int -> Int",
    "hlCalcWeightsOrel(_, _)",
    "Takes the given edges-orel and calculates the edge-weight for every"
    " edge using the calculation mode defined with the second parameter"
    " (which currently is not supported, so ever use 1) and"
    " stores the weight directly with the edge of the given orel.",
    "query hlCalcWeightsOrel(hlEdgesOrelSource, 1) consume"
);


OperatorSpec hlCreateLabelsFromHlGraphSpec(
    "Orel x Orel -> Nrel",
    "hlCreateLabelsFromHlGraph(_, _)",
    "Takes the node-orel and the previous contracted hlGraph and"
    " creates Hub Label for every node of the node-orel."
    " Returns a Nrel containing all Hub Label.",
    "query hlCreateLabelsFromHlGraph(hlNodesSourceOnlyOrel, hlGraphOrel)"
        " feed consume"
);







/*
5.3.3 Specification of (3) In-Memory Approach using MainMemory2Algebra

*/


OperatorSpec hlContractMmGraphSpec(
    "memgraph x memgraph x fun -> bool",
    "hlContractMmGraph(_, _, _)",
    "Not yet implemented.",
    "query hlContractMmGraph(memgraphUp, memgraphDown, distanceFunction)"
);







/*
5.4 Operator Instance

Here, we create an instance of the operator using a constructor
of the class ~Operator~ and feeding it with the defined functions.
For non-overloaded operators, always the selection function
~Operator::SimpleSelect~ is used.

All operators are related to the equal named function of the class
 HubLabelClass.
The Ooerators just got an additional suffix 'Op'

5.4.1 Operator Instance of (1) Persistent Approach

*/



Operator hlCalcRankOp(
    "hlCalcRank",             // name of the operator
    hlCalcRankSpec.getStr(),  // specification
    hlCalcRankVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlCalcRankTM              // type mapping
);



Operator hlOneHopReverseSearchOp(
    "hlOneHopReverseSearch",             // name of the operator
    hlOneHopReverseSearchSpec.getStr(),  // specification
    hlOneHopReverseSearchVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlOneHopReverseSearchTM              // type mapping
);



Operator hlHHopForwardSearchOp(
    "hlHHopForwardSearch",             // name of the operator
    hlHHopForwardSearchSpec.getStr(),  // specification
    hlHHopForwardSearchVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlHHopForwardSearchTM              // type mapping
);



Operator hlForwardSearchGetDistOp(
    "hlForwardSearchGetDist",             // name of the operator
    hlForwardSearchGetDistSpec.getStr(),  // specification
    hlForwardSearchGetDistVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlForwardSearchGetDistTM              // type mapping
);



Operator hlRemoveTFromCurrentWitnessListOp(
    "hlRemoveTFromCurrentWitnessList",             // name of the operator
    hlRemoveTFromCurrentWitnessListSpec.getStr(),  // specification
    hlRemoveTFromCurrentWitnessListVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlRemoveTFromCurrentWitnessListTM              // type mapping
);



Operator hlForwardSearchCheckForWitnessPathOp(
    "hlForwardSearchCheckForWitnessPath",             // name of the operatord
    hlForwardSearchCheckForWitnessPathSpec.getStr(),  // specification
    hlForwardSearchCheckForWitnessPathVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlForwardSearchCheckForWitnessPathTM              // type mapping
);



Operator hlInsertOrUpdateTupleInNotYetVisitedListOp(
    "hlInsertOrUpdateTupleInNotYetVisitedList",
    // name of the operator
    hlInsertOrUpdateTupleInNotYetVisitedListSpec.getStr(),  // specification
    hlInsertOrUpdateTupleInNotYetVisitedListVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlInsertOrUpdateTupleInNotYetVisitedListTM              // type mapping
);



Operator hlForwardSearchIterativeStepsScanNewVerticesOp(
    "hlForwardSearchIterativeStepsScanNewVertices",
    // name of the operator
    hlForwardSearchIterativeStepsScanNewVerticesSpec.getStr(),  // specification
    hlForwardSearchIterativeStepsScanNewVerticesVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlForwardSearchIterativeStepsScanNewVerticesTM              // type mapping
);



Operator hlForwardSearchProcessIncomingEdgeIterativeStepsOp(
    "hlForwardSearchProcessIncomingEdgeIterativeSteps",
    // name of the operator
    hlForwardSearchProcessIncomingEdgeIterativeStepsSpec.getStr(),
    // specification
    hlForwardSearchProcessIncomingEdgeIterativeStepsVM,
    // value mapping
    Operator::SimpleSelect,  // selection function
    hlForwardSearchProcessIncomingEdgeIterativeStepsTM
    // type mapping
);



Operator hlForwardSearchProcessIncomingEdgeInitialStepsOp(
    "hlForwardSearchProcessIncomingEdgeInitialSteps",
    // name of the operator
    hlForwardSearchProcessIncomingEdgeInitialStepsSpec.getStr(),
    // specification
    hlForwardSearchProcessIncomingEdgeInitialStepsVM,
    // value mapping
    Operator::SimpleSelect,  // selection function
    hlForwardSearchProcessIncomingEdgeInitialStepsTM
    // type mapping
);



Operator hlForwardSearchCreateAndAppendShortcutsOp(
    "hlForwardSearchCreateAndAppendShortcuts",
    // name of the operator
    hlForwardSearchCreateAndAppendShortcutsSpec.getStr(),  // specification
    hlForwardSearchCreateAndAppendShortcutsVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlForwardSearchCreateAndAppendShortcutsTM              // type mapping
);



Operator hlForwardSearchProcessIncomingEdgeOp(
    "hlForwardSearchProcessIncomingEdge",             // name of the operator
    hlForwardSearchProcessIncomingEdgeSpec.getStr(),  // specification
    hlForwardSearchProcessIncomingEdgeVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlForwardSearchProcessIncomingEdgeTM              // type mapping
);



Operator hlRemoveContractedEdgesFromEdgesRelationsOp(
    "hlRemoveContractedEdgesFromEdgesRelations",
    // name of the operator
    hlRemoveContractedEdgesFromEdgesRelationsSpec.getStr(),  // specification
    hlRemoveContractedEdgesFromEdgesRelationsVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlRemoveContractedEdgesFromEdgesRelationsTM              // type mapping
);



Operator hlRemoveParallelEdgesFromEdgesRelationsOp(
    "hlRemoveParallelEdgesFromEdgesRelations",
    // name of the operator
    hlRemoveParallelEdgesFromEdgesRelationsSpec.getStr(),  // specification
    hlRemoveParallelEdgesFromEdgesRelationsVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlRemoveParallelEdgesFromEdgesRelationsTM              // type mapping
);



Operator hlDoContractionOp(
    "hlDoContraction",             // name of the operator
    hlDoContractionSpec.getStr(),  // specification
    hlDoContractionVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlDoContractionTM              // type mapping
);



Operator hlIterateOverAllNodesByRankAscAndDoContractionOp(
    "hlIterateOverAllNodesByRankAscAndDoContraction",
    // name of the operator
    hlIterateOverAllNodesByRankAscAndDoContractionSpec.getStr(),
    // specification
    hlIterateOverAllNodesByRankAscAndDoContractionVM,
    // value mapping
    Operator::SimpleSelect,  // selection function
    hlIterateOverAllNodesByRankAscAndDoContractionTM
    // type mapping
);


Operator hlCreateLabelCheckForWitnessScanNewVerticesOp(
    "hlCreateLabelCheckForWitnessScanNewVertices",
    // name of the operator
    hlCreateLabelCheckForWitnessScanNewVerticesSpec.getStr(),  // specification
    hlCreateLabelCheckForWitnessScanNewVerticesVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlCreateLabelCheckForWitnessScanNewVerticesTM              // type mapping
);


Operator hlCreateLabelCheckForWitnessOp(
    "hlCreateLabelCheckForWitness",             // name of the operator
    hlCreateLabelCheckForWitnessSpec.getStr(),  // specification
    hlCreateLabelCheckForWitnessVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlCreateLabelCheckForWitnessTM              // type mapping
);


Operator hlCreateLabelScanNewVerticesOp(
    "hlCreateLabelScanNewVertices",             // name of the operator
    hlCreateLabelScanNewVerticesSpec.getStr(),  // specification
    hlCreateLabelScanNewVerticesVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlCreateLabelScanNewVerticesTM              // type mapping
);


Operator hlGetRankByIdOp(
    "hlGetRankById",             // name of the operator
    hlGetRankByIdSpec.getStr(),  // specification
    hlGetRankByIdVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlGetRankByIdTM              // type mapping
);


Operator hlCreateLabelByDijkstraWithStallingOp(
    "hlCreateLabelByDijkstraWithStalling",             // name of the operator
    hlCreateLabelByDijkstraWithStallingSpec.getStr(),  // specification
    hlCreateLabelByDijkstraWithStallingVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlCreateLabelByDijkstraWithStallingTM              // type mapping
);


Operator hlFillForwardOrReverseLabelOp(
    "hlFillForwardOrReverseLabel",             // name of the operator
    hlFillForwardOrReverseLabelSpec.getStr(),  // specification
    hlFillForwardOrReverseLabelVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlFillForwardOrReverseLabelTM              // type mapping
);


Operator hlGetPathViaPointsOp(
    "hlGetPathViaPoints",             // name of the operator
    hlGetPathViaPointsSpec.getStr(),  // specification
    hlGetPathViaPointsVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlGetPathViaPointsTM              // type mapping
);


Operator hlQueryOp(
    "hlQuery",             // name of the operator
    hlQuerySpec.getStr(),  // specification
    hlQueryVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlQueryTM              // type mapping
);


Operator hlPruneLabelByBootstrappingOp(
    "hlPruneLabelByBootstrapping",             // name of the operator
    hlPruneLabelByBootstrappingSpec.getStr(),  // specification
    hlPruneLabelByBootstrappingVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlPruneLabelByBootstrappingTM              // type mapping
);


Operator hlReorderLabelsOp(
    "hlReorderLabels",             // name of the operator
    hlReorderLabelsSpec.getStr(),  // specification
    hlReorderLabelsVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlReorderLabelsTM              // type mapping
);


Operator hlCreateLabelsOp(
    "hlCreateLabels",             // name of the operator
    hlCreateLabelsSpec.getStr(),  // specification
    hlCreateLabelsVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlCreateLabelsTM              // type mapping
);




/*
5.4.2 Operator Instance of (2) In-Memory Approach

*/


Operator hlTransformOrelToHlGraphOp(
    "hlTransformOrelToHlGraph",             // name of the operator
    hlTransformOrelToHlGraphSpec.getStr(),  // specification
    hlTransformOrelToHlGraphVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlTransformOrelToHlGraphTM              // type mapping
);



Operator hlDoContractionOfHlGraphOp(
    "hlDoContractionOfHlGraph",             // name of the operator
    hlDoContractionOfHlGraphSpec.getStr(),  // specification
    hlDoContractionOfHlGraphVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlDoContractionOfHlGraphTM              // type mapping
);



Operator hlDoChSearchInHlGraphOp(
    "hlDoChSearchInHlGraph",             // name of the operator
    hlDoChSearchInHlGraphSpec.getStr(),  // specification
    hlDoChSearchInHlGraphVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlDoChSearchInHlGraphTM              // type mapping
);



Operator hlCalcWeightsOrelOp(
    "hlCalcWeightsOrel",             // name of the operator
    hlCalcWeightsOrelSpec.getStr(),  // specification
    hlCalcWeightsOrelVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlCalcWeightsOrelTM              // type mapping
);



Operator hlCreateLabelsFromHlGraphOp(
    "hlCreateLabelsFromHlGraph",             // name of the operator
    hlCreateLabelsFromHlGraphSpec.getStr(),  // specification
    hlCreateLabelsFromHlGraphVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlCreateLabelsFromHlGraphTM              // type mapping
);







/*
5.4.2 Operator Instance of (2) In-Memory Approach

*/

Operator hlContractMmGraphOp(
    "hlContractMmGraph",             // name of the operator
    hlContractMmGraphSpec.getStr(),  // specification
    hlContractMmGraphVM,             // value mapping
    Operator::SimpleSelect,  // selection function
    hlContractMmGraphTM              // type mapping
);






/*
6 Definition of the Algebra

\label{AlgebraDefinition}

In this step, a new algebra -- a class derived from the ~Algebra~ class  --
is created. Within the constructor of the algebra, we add the type constructors
and assign the corresponding kinds to the types.
Furthermore, all operators are added to the algebra.

*/

class HubLabelingAlgebra : public Algebra
{
public:
    HubLabelingAlgebra() : Algebra()
    {

        AddTypeConstructor( &HubLabelClassTC );
        HubLabelClassTC.AssociateKind( Kind::SIMPLE() );

        AddOperator(&hlCalcRankOp);
        AddOperator(&hlOneHopReverseSearchOp);
        AddOperator(&hlHHopForwardSearchOp);
        AddOperator(&hlForwardSearchGetDistOp);
        AddOperator(&hlRemoveTFromCurrentWitnessListOp);
        AddOperator(
            &hlForwardSearchCheckForWitnessPathOp);
        AddOperator(
            &hlInsertOrUpdateTupleInNotYetVisitedListOp);
        AddOperator(
            &hlForwardSearchIterativeStepsScanNewVerticesOp);
        AddOperator(
            &hlForwardSearchProcessIncomingEdgeIterativeStepsOp);
        AddOperator(
            &hlForwardSearchProcessIncomingEdgeInitialStepsOp);
        AddOperator(
            &hlForwardSearchCreateAndAppendShortcutsOp);
        AddOperator(
            &hlForwardSearchProcessIncomingEdgeOp);
        AddOperator(
            &hlRemoveContractedEdgesFromEdgesRelationsOp);
        AddOperator(
            &hlRemoveParallelEdgesFromEdgesRelationsOp);
        AddOperator(&hlDoContractionOp);
        AddOperator(
            &hlIterateOverAllNodesByRankAscAndDoContractionOp);
        AddOperator(
            &hlCreateLabelCheckForWitnessScanNewVerticesOp);
        AddOperator(&hlCreateLabelCheckForWitnessOp);
        AddOperator(&hlCreateLabelScanNewVerticesOp);
        AddOperator(&hlGetRankByIdOp);
        AddOperator(
            &hlCreateLabelByDijkstraWithStallingOp);
        AddOperator(&hlFillForwardOrReverseLabelOp);
        AddOperator(&hlGetPathViaPointsOp);
        AddOperator(&hlQueryOp);
        AddOperator(&hlPruneLabelByBootstrappingOp);
        AddOperator(&hlReorderLabelsOp);
        AddOperator(&hlCreateLabelsOp);


        AddOperator(&hlContractMmGraphOp);


        AddOperator(&hlTransformOrelToHlGraphOp);
        AddOperator(&hlDoContractionOfHlGraphOp);
        AddOperator(&hlDoChSearchInHlGraphOp);
        AddOperator(&hlCalcWeightsOrelOp);
        AddOperator(&hlCreateLabelsFromHlGraphOp);
    }
};


/*
End of the namespace. The following code cannot be embedded into the
algebras's namespace. Thus the namespace should end here.

*/

} // end of namespace


/*
7 Initialization of the Algebra

This piece of code returns a new instance of the algebra.


*/
extern "C"
Algebra*
InitializeHubLabelingAlgebra( NestedList* nlRef,
                              QueryProcessor* qpRef )
{
    return new hublabeling::HubLabelingAlgebra;
}
