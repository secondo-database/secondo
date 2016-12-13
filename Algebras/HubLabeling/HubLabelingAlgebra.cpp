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

Februar 2016 Sebastian Krings

[TOC]

1 Overview

The HubLabeling Algebra implements the HubLabeling Algorithm.
It offers some operators for creating the neccessary Hub-Labels over all nodes
 and for doing shortest path queries on this structure.

2 Defines, includes, and constants

*/

#include "Attribute.h"          // implementation of attribute types
#include "Algebra.h"            // definition of the algebra
#include "NestedList.h"         // required at many places
#include "QueryProcessor.h"     // needed for implementing value mappings
#include "AlgebraManager.h"     // e.g., check for a certain kind
#include "Operator.h"           // for operator creation
#include "StandardTypes.h"      // priovides int, real, string, bool type
#include "FTextAlgebra.h"
#include "Symbols.h"            // predefined strings
#include "ListUtils.h"          // useful functions for nested lists
#include "Stream.h"             // wrapper for secondo streams

#include "GenericTC.h"          // use of generic type constructors

#include "LogMsg.h"             // send error messages

#include "../../Tools/Flob/DbArray.h"  // use of DbArrays

#include "RelationAlgebra.h"           // use of tuples
#include "OrderedRelationAlgebra.h"           // use of tuples
#include "NestedRelationAlgebra.h"           // use of tuples
#include "BTreeAlgebra.h"           // use of tuples
#include "LongInt.h"           // use of longint
#include "Point.h"           // use of point
#include "SpatialAlgebra.h"           // use of sline

#include <math.h>               // required for some operators
#include <stack>
#include <limits>
#include <map>

#include <unordered_set>
#include <tuple>
/*
2.1 Global Variables

Secondo uses some variables designed as singleton pattern. For accessing these
global variables, these variables have to be declared to be extern:

*/

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;

/*
2.2 Namespace

Each algebra file defines a lot of functions. Thus, name conflicts may arise
with function names defined in other algebra modules during compiling/linking
the system. To avoid these conflicts, the algebra implementation should be
embedded into a namespace.

*/

namespace hublabeling
{
//Debug
//#define USEDEBUG

#ifdef USEDEBUG
#define Debug(x) std::cout << x
#else
#define Debug(x)
#endif


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

    /*

    3.2 HubLabeling functionality

    */
    /**
     * Definition of CostModes.
     * Used for getting shortest paths by different cost-calculations.
     */
    static const int HL_DEFAULT_COST_MODE = 1;
    static const int HL_LENGTH_COST_MODE = 2;
    static const int HL_TIME_COST_MODE = 3;

    /**
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

    /**
     * Definition of Calculation Functions used to calculate the rank
     *  of a vertex inside the underlying graph.
     */
    static const int HL_DEFAULT_CALC_FUNCTION = 1;

    /**
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
    double hlCalcRank(OrderedRelation*
                      orelEdgesSource,
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


    /**
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


    /**
     * OrderedRelation type for Edges
     * Used for creation of new OrderedRelation Objects, new Tuples of
     *  this OrderedRelation or cloning or deleting of an
     *  OrderedRelation of this type
     */
    static const string hlGetEdgesOrelSourceTypeInfo()
    {
        return "(" + OrderedRelation::BasicType() +
               hlGetEdgesTupleTypeInfo() +
               "(Source)" +
               ")";
    }


    /**
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


    /**
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


    /**
     * OrderedRelation type for Edges ordered by Target
     * Used for creation of new OrderedRelation Objects, new Tuples of
     *  this OrderedRelation or cloning or deleting of an
     *  OrderedRelation of this type
     */
    static const string hlGetEdgesOrelTargetTypeInfo()
    {
        return "(" + OrderedRelation::BasicType() +
               "(" + Tuple::BasicType() +
               "(" +
               "(Source " + CcInt::BasicType() + ")" +
               "(Target " + CcInt::BasicType() + ")" +
               "(SourcePos " + Point::BasicType() + ")" +
               "(TargetPos " + Point::BasicType() + ")" +
               "(SourceNodeCounter " + CcInt::BasicType() +
               ")" +
               "(TargetNodeCounter " + CcInt::BasicType() +
               ")" +
               "(Curve " + SimpleLine::BasicType() + ")" +
               "(RoadName " + FText::BasicType() + ")" +
               "(RoadType " + FText::BasicType() + ")" +
               "(WayId " + LongInt::BasicType() + ")" +
               "(Costs " + CcReal::BasicType() + ")" +
               "(HlShortcutViaParent " + CcInt::BasicType() +
               ")" +
               ")" +
               ")" +
               "(Target)" +
               ")";
    }

    /**
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

    /**
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

    /**
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
        int i = 0;
        while(currentEdgeTargetTupleU)
        {
            //Dont ignore current Node to be contracted because
            //we need its distances also in forward search
            CcInt* currentSourceNodeU = (CcInt*)
                                        currentEdgeTargetTupleU->GetAttribute(
                                            HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
            double currentDistXT = hlGetWeightedCost(
                            HL_DEFAULT_COST_MODE, currentEdgeTargetTupleU);
            Debug(i++ << ": (" <<
                  currentSourceNodeU->GetIntval() << ";" <<
                  currentTargetNodeRevT->GetIntval() << ";" <<
                  currentDistXT << "), ");

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
                    Debug("XT still exists" << endl);
                    if(currentDistXT < lookupEdgeDistXT)
                    {
                        Debug("still existing XT is longer, going to delete it"
                         " here to insert the current new XT afterwards");
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


    /**
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

    /**
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

    /**
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

    /**
     * Definition of Field-Indexes for notYetVisitedNodes-orel
     */
    static const int
    HL_INDEX_OF_DIST_IN_NOT_YET_VISITED = 0;
    static const int
    HL_INDEX_OF_NODE_ID_IN_NOT_YET_VISITED = 1;
    static const int
    HL_INDEX_OF_HOP_DEPTH_IN_NOT_YET_VISITED = 2;

    /**
     * Definition of default h-hop-size
     */
    static const int HL_DEFAULT_H_DEPTH = 1;

    /**
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

    /**
     * Definition of Field-Indexes for stillVisitedNodes-orel
     */
    static const int
    HL_INDEX_OF_NODE_ID_IN_STILL_VISITED = 0;


    /**
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
        Debug("Starting hlHHopForwardSearch with v = " <<
              currentVertexIdFwdV->GetIntval() <<
              " and hDepth = " << hDepth << endl);

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

            Debug("Process next source node s = " <<
                  currentSourceNodeFwdS->GetIntval() << endl);

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
                double tmpDist =
                    std::get<HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE - 1>
                    (currReverseTuple);
                // removes all even values
                if (currentEraseId ==
                        currentSourceNodeFwdS->GetIntval())
                {
                    copyMultimapReverseSearchXT.erase(
                        currReverseIter);
                    Debug("erased s (=t) itself from XT: x=" <<
                          (*currReverseIter).first << ", t=" <<
                          currentEraseId << " dist: " << tmpDist << endl);
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

                Debug("all witnesses found continue with next SV edge"
                      << endl);
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

    /**
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

        Debug("Do initial" << endl);
        hlForwardSearchProcessIncomingEdgeInitialSteps(
            orelEdgesSource, currentVertexIdFwdV,
            currentSourceNodeFwdS, notYetVisitedNodesMultiMap,
            copyMultimapReverseSearchXT, distSV);

        Debug("Do iterative" << endl);
        hlForwardSearchProcessIncomingEdgeIterativeSteps(
            orelEdgesSource, currentVertexIdFwdV,
            notYetVisitedNodesMultiMap, stillVisitedNodesSet,
            copyMultimapReverseSearchXT, hDepth, distSV);

        Debug("Do create shortcuts" << endl);
        hlForwardSearchCreateAndAppendShortcuts(
            shortcutsToBeCreatedOrelSourceToBeDeleted,
            copyMultimapReverseSearchXT,
            currentSourceNodeFwdS->GetIntval(),
            currentVertexIdFwdV, distSV);

        return true;
    }

    /**
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

                    Debug("all witnesses found, break initial steps"
                          << endl);
                    break;
                }

                std::tuple<double, int, int>
                currMinNotYetVisitedNodeTuple = std::make_tuple(
                                0.0, currentSourceNodeFwdS->GetIntval(), 0);
                Debug("forward search initial steps insertOrUpdateInNotYet"
                      << endl);
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

    /**
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
        Debug("notYetVisitedNodesMultiMap.size(): " <<
              notYetVisitedNodesMultiMap.size() <<
              " copyMultimapReverseSearchXT.size(): " <<
              copyMultimapReverseSearchXT.size() << endl);
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
            Debug("next minW: " << currMinNodeId << " - " <<
                  currMinDist << " - " << currMinHopDepth << endl);

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
        Debug("finish of hlForwardSearchProcessIncomingEdgeIterativeSteps"
              << endl);
        return true;
    }

    /**
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
        Debug("start hlForwardSearchIterativeStepsScanNewVertices"
              << endl);
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
            Debug("next iteration: currentTargetNodeX: " <<
                  currentTargetNodeX->GetIntval() << endl);

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
                    Debug("all witnesses found break scan new vertices"
                     " of forward search"
                          << endl);
                    //Free Resources
                    currentEdgeSourceTupleWX->DeleteIfAllowed();
                    currentEdgeSourceTupleWX = 0;

                    break;
                }

                Debug("forward search scan new vertices insertOrUpdateInNotYet"
                      << endl);
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

        Debug("finish hlForwardSearchIterativeStepsScanNewVertices"
              << endl);

        return true;
    }

    /**
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
        Debug("doing hlInsertOrUpdateTupleInNotYetVisitedList"
              << endl);
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
                        Debug("update: erasing old entry for reinserting"
                              << endl);
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
                Debug("going to insert new element to notYet : "
                      << currentTargetNodeX->GetIntval() << " - " <<
                      currDistSWX << endl);

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

        Debug("finish hlInsertOrUpdateTupleInNotYetVisitedList"
              << endl);

        return true;
    }

    /**
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
        Debug("check for witness start" << endl);
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

            Debug("xt found (x = " <<
                  currentSourceNodeWitnessX << ", t = " <<
                  currentEdgeReverseAttributeIdT << ")" << endl);

            double distSXT = (double) distSX +
                             currentEdgeReverseAttributeDistXT;
            double distSVT = (double) distSV +
                             hlForwardSearchGetDistVT(
                            copyMultimapReverseSearchXT, currentContractNodeV,
                                 currentEdgeReverseAttributeIdT);

            if(distSXT <= distSVT)
            {
                Debug("witness found" << endl);
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
            Debug("going to erase from XT: " << *vecIter <<
                  endl);
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
                    Debug("erased witness from XT x= " <<
                          (*currReverseIter).first << " t = " <<
                          currentEraseId << endl);
                }
            }
        }
        Debug("finish check for witness" << endl);

        return true;
    }

    /**
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

    /**
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

    /**
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
        Debug("start hlForwardSearchCreateAndAppendShortcuts"
              << endl);
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
            int currentEdgeSourceIDX =
                (*reverseXTMultimapIter).first;
            int currentEdgeSourceIDT =
                std::get<HL_INDEX_OF_ID_OF_T_IN_TEMP_REV_TUPLE -
                1>(currTuple);
            double currentEdgeSourceDistVT =
                std::get<HL_INDEX_OF_DIST_IN_TEMP_REV_TUPLE - 1>
                (currTuple);

            //create shortcut and add to
            // shortcutsToBeCreatedOrelSourceToBeDeleted
            Debug("insert new shortcut to multimap (" <<
                  currentSourceNodeS << ", " << currentEdgeSourceIDT
                  << ", " << distSV + currentEdgeSourceDistVT <<
                  ")"<< endl);
            std::tuple<int, double, int> insertTuple =
                std::make_tuple(currentEdgeSourceIDT,
                                distSV + currentEdgeSourceDistVT,
                                currentContractNodeV);
            shortcutsToBeCreatedOrelSourceToBeDeleted.insert(
                pair<int, std::tuple<int, double, int>>
                (currentSourceNodeS, insertTuple));
        }
        Debug("finish hlForwardSearchCreateAndAppendShortcuts"
              << endl);
        return true;
    }

    /**
     * Definition of Field-Indexes for ranked nodes-orel
     */
    static const int HL_INDEX_OF_ID_IN_NODES_RANKED =
        2;
    static const int HL_INDEX_OF_RANK_IN_NODES_RANKED
        = 4;

    /**
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
        Debug("start hlIterateOverAllNodesByRankAscAndDoContraction"
              << endl);
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
        Debug("edgesWithViaOrelSource cloned" << endl);

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
        Debug("edgesWithViaOrelTarget cloned" << endl);

        std::multimap<int, std::tuple<int, double, int>>
                shortcutsToBeCreatedOrelSourceToBeDeleted;

        //Iterate Over all Ranked Nodes in ascending order of their rank
        GenericRelationIterator* nodesWithRankOrelRankIter
            = nodesWithRankOrelRank->MakeScan();
        Tuple* currentNodeRanked =
            nodesWithRankOrelRankIter->GetNextTuple();
        Debug("iterate over all nodes by rank" << endl);

        int countProgress = 0;
        while(currentNodeRanked)
        {
            CcInt* currentNodeRankedId = (CcInt*)
                                         currentNodeRanked->GetAttribute(
                                             HL_INDEX_OF_ID_IN_NODES_RANKED);

            Debug("do Contraction for: " <<
                  currentNodeRankedId->GetIntval() << " (" <<
                  ++countProgress << "/" <<
                  nodesWithRankOrelRank->GetNoTuples() << ")" <<
                  endl);
            hlDoContraction(copyEdgesWithViaOrelSource,
                            copyEdgesWithViaOrelTarget,
                            shortcutsToBeCreatedOrelSourceToBeDeleted,
                            currentNodeRankedId, hHop);

            Debug("remove edges to and from current v in copy edges relation"
                  << endl);
            hlRemoveContractedEdgesFromEdgesRelations(
                copyEdgesWithViaOrelSource,
                copyEdgesWithViaOrelTarget, currentNodeRankedId);

            Debug("remove existing edges parallel to new"
             " shortcuts to be created"
                  << endl);
            hlRemoveParallelEdgesFromEdgesRelations(
                copyEdgesWithViaOrelSource,
                copyEdgesWithViaOrelTarget,
                shortcutsToBeCreatedOrelSourceToBeDeleted);

            Debug("remove existing original edges parallel to"
             " new shortcuts to be created"
                  << endl);
            hlRemoveParallelEdgesFromEdgesRelations(
                edgesWithViaOrelSource, edgesWithViaOrelTarget,
                shortcutsToBeCreatedOrelSourceToBeDeleted);

            Debug("add shortcuts to copy edges relations" <<
                  endl);
            //Add Shortcuts to Edges Relations
            hlAddShortcutsToEdgesRelations(
                copyEdgesWithViaOrelSource,
                copyEdgesWithViaOrelTarget,
                shortcutsToBeCreatedOrelSourceToBeDeleted);

            Debug("add shortcuts to original edges relations"
                  << endl);
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
        Debug("finished iteration" << endl);

        //Free Resources
        delete nodesWithRankOrelRankIter; //TODO: richtig, dass der
        // iter nur deleted wird ohne besondere funktion?
        OrderedRelation::Delete(relNumTypeSource,
                                wrelSource2);
        OrderedRelation::Delete(relNumTypeTarget,
                                wrelTarget2);

        Debug("finish hlIterateOverAllNodesByRankAscAndDoContraction"
              << endl);
        return true;
    }

    /**
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
        Debug("start hlDoContraction" << endl);

        //Reverse search
        std::multimap<int, std::tuple<int, double>>
                oneHopReverseMultiMap; //XXTODO: ist diese zuweisung ein
                // Problem in Bezug auf Daten aus vorherigen Iterationen?

        Debug("do hlOneHopReverseSearch" << endl);
        hlOneHopReverseSearch(edgesWithViaOrelSource,
                              edgesWithViaOrelTarget, currentVToBeContracted,
                              oneHopReverseMultiMap);

        Debug("do hlHHopForwardSearch" << endl);
        hlHHopForwardSearch(edgesWithViaOrelSource,
                            edgesWithViaOrelTarget, oneHopReverseMultiMap,
                            currentVToBeContracted,
                            shortcutsToBeCreatedOrelSourceToBeDeleted, hHop);

        oneHopReverseMultiMap.clear();

        Debug("finish hlDoContraction" << endl);
        return true;
    }



    /**
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
        Debug("start hlAddShortcutsToEdgesRelations" <<
              endl);

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

            Debug("build next inputtuple (" << currentSource
                  << ", " << currentTarget << ")" << endl);

            ListExpr relTypeSource;
            nl->ReadFromString(hlGetEdgesOrelSourceTypeInfo(),
                               relTypeSource);
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

            Debug("append to edgesWithViaOrelSource" << endl);
            edgesWithViaOrelSource->AppendTuple(insertTuple);

            Debug("append to edgesWithViaOrelTarget" << endl);
            edgesWithViaOrelTarget->AppendTuple(insertTuple);

            insertTuple->DeleteIfAllowed(); //TODO zu früh gelöscht weil orel
            // wo es appended wurde noch verwendet wird?
            insertTuple = 0;

            Debug("get next tuple" << endl);
            //Free Outgoing-Iteration
        }
        Debug("finish iteration" << endl);

        Debug("end hlAddShortcutsToEdgesRelations" <<
              endl);
        return true;
    }


    /**
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
        Debug("start hlRemoveContractedEdgesFromEdgesRelations"
              << endl);

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

        CcInt currentContractedVPlusOneFwdV(true,
                                    currentContractedV->GetIntval() + 1);
        CcInt* currentVertexIdPlusOnePtrFwdV =
            &currentContractedVPlusOneFwdV;
        std::vector<void*> vecAttributesToFwdV(1);
        vecAttributesToFwdV[0] =
            currentVertexIdPlusOnePtrFwdV;
        std::vector<SmiKey::KeyDataType>
        vecAttrTypesToFwdV(1);
        vecAttrTypesToFwdV[0] =
            currentVertexIdPlusOnePtrFwdV->getSMIKeyType();
        CompositeKey toFwdV(vecAttributesToFwdV,
                            vecAttrTypesToFwdV, false);


        vector<Tuple*> tupleVectorToDeleteSource;
        vector<Tuple*> tupleVectorToDeleteTarget;


        Debug("remove incoming edges SV" <<
              endl); //XXTODO: in orelTarget müssten aber auch die
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
            CcInt* currentEdgeTargetTupleSVTargetId =
                (CcInt*) currentEdgeTargetTupleSV->GetAttribute(
                    HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);

            Debug("going to delete: (" <<
                  currentEdgeTargetTupleSVSourceId->GetIntval() <<
                  "," << currentEdgeTargetTupleSVTargetId->GetIntval()
                  << ")" << endl);
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


        Debug("remove outgoing edges VT" <<
              endl); //XXTOTO: auch eingehende Kanten sollten gelöscht werden?!
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
            CcInt* currentEdgeSourceTupleVTSourceId =
                (CcInt*) currentEdgeSourceTupleVT->GetAttribute(
                    HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);

            Debug("going to delete: (" <<
                  currentEdgeSourceTupleVTSourceId->GetIntval() <<
                  "," << currentEdgeSourceTupleVTTargetId->GetIntval()
                  << ")" << endl);
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


        Debug("iterate tupleVectorToDeleteTarget" <<
              endl);
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


        Debug("iterate tupleVectorToDeleteSource" <<
              endl);
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

        Debug("end hlRemoveContractedEdgesFromEdgesRelations"
              << endl);
        return true;
    }


    /**
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
        Debug("start hlRemoveParallelEdgesFromEdgesRelations"
              << endl);

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
            //int currentParent =
            // std::get<HL_INDEX_OF_PARENT_ID_IN_SHORTCUT_TUPLE - 1>(currTuple);
            double currentCosts =
                std::get<HL_INDEX_OF_DIST_IN_SHORTCUT_TUPLE - 1>
                (currTuple);

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

            GenericRelationIterator*
            currentEdgesOrelSourceIter =
                currentEdgesOrelSource->MakeRangeScan(fromSource,
                        toSource);
            Tuple* currentEdgesOrelSourceTuple =
                currentEdgesOrelSourceIter->GetNextTuple();
            while(currentEdgesOrelSourceTuple)
            {
                CcInt* currentEdgesOrelSourceTupleSourceId =
                    (CcInt*) currentEdgesOrelSourceTuple->GetAttribute(
                        HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
                CcInt* currentEdgesOrelSourceTupleTargetId =
                    (CcInt*) currentEdgesOrelSourceTuple->GetAttribute(
                        HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
                CcReal* currentEdgesOrelSourceTupleDist =
                    (CcReal*) currentEdgesOrelSourceTuple->GetAttribute(
                        HL_INDEX_OF_COSTS_IN_EDGE_TUPLE);


                //If there is a parallel edge it should be
                // removed from the working-graph
                if(currentEdgesOrelSourceTupleTargetId->GetIntval()
                        == currentTarget)
                {
                    Debug("remove incoming edges ST: (" <<
                          currentEdgesOrelSourceTupleSourceId->GetIntval()
                          << ", " <<
                          currentEdgesOrelSourceTupleTargetId->GetIntval()
                          << ", " <<
                          currentEdgesOrelSourceTupleDist->GetRealval() <<
                          ")"<< endl);
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
                CcInt* currentEdgesOrelTargetTupleTargetId =
                    (CcInt*) currentEdgesOrelTargetTuple->GetAttribute(
                        HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
                CcReal* currentEdgesOrelTargetTupleDist =
                    (CcReal*) currentEdgesOrelTargetTuple->GetAttribute(
                        HL_INDEX_OF_COSTS_IN_EDGE_TUPLE);

                //If there is a parallel edge it should be
                // removed from the working-graph
                if(currentEdgesOrelTargetTupleSourceId->GetIntval()
                        == currentSource)
                {
                    Debug("remove incoming edges ST: (" <<
                          currentEdgesOrelTargetTupleSourceId->GetIntval()
                          << ", " <<
                          currentEdgesOrelTargetTupleTargetId->GetIntval()
                          << ", " <<
                          currentEdgesOrelTargetTupleDist->GetRealval() <<
                          ")"<< endl);
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


        Debug("iterate tupleVectorToDeleteSource" <<
              endl);
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

        Debug("iterate tupleVectorToDeleteTarget" <<
              endl);
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

        Debug("end hlRemoveParallelEdgesFromEdgesRelations"
              << endl);
        return true;
    }



    /**
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
        Debug("start hlRemoveContractedEdgesFromEdgesRelationsScanOppositeOrel"
              << endl);

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
                Debug("pushback: " <<
                      currentSourceOrTargetNodeId->GetIntval() << endl);
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

        Debug("end hlRemoveContractedEdgesFromEdgesRelationsScanOppositeOrel"
              << endl);
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

    /**
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

    /**
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

    /**
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

    /**
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

    /**
     * Definition of Field-Indexes for NestedRelation allLabels
     */
    static const int
    HL_INDEX_OF_NODE_ID_IN_ALL_LABELS_TUPLE = 0;
    static const int
    HL_INDEX_OF_FORWARD_LABEL_IN_ALL_LABELS_TUPLE = 1;
    static const int
    HL_INDEX_OF_REVERSE_LABEL_IN_ALL_LABELS_TUPLE = 2;

    /**
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




    /**
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
        Debug("start hlCreateLabels" << endl);

        Debug("create forward and reverse relation containing label-data"
         " used by all single forward and reverse labels"
              << endl);
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

        Debug("do while" << endl);
        GenericRelationIterator* nodesRankIter =
            nodesRankedDescOrelRank->MakeScan();
            //XXTODO: iteration in der iteration???
        Tuple* currentNodeV =
            nodesRankIter->GetNextTuple();

        while(currentNodeV)
        {
            Debug("next iteration" << endl);

            CcInt* nodeIdV = (CcInt*)
                             currentNodeV->GetAttribute(
                                 HL_INDEX_OF_ID_IN_NODES_RANKED);

            Debug("prepare NestedRelationTuple" << endl);

            TupleType* tupleTypeAllLabels =
                allLabelsNrel->getPrimary()->GetTupleType();
            Tuple* allLabelsTuple = new Tuple(
                tupleTypeAllLabels);
            allLabelsTuple->PutAttribute(
                HL_INDEX_OF_NODE_ID_IN_ALL_LABELS_TUPLE,
                new CcInt(true, nodeIdV->GetIntval()));

            Debug("forward  and reverse label multimaps" <<
                  endl);
            std::multimap<int, std::tuple<int, int, double>>
                    labelForwardMultimap;
            std::multimap<int, std::tuple<int, int, double>>
                    labelReverseMultimap;

            bool isForward = true;
            //Create forward label
            Debug("create forward label" << endl);
            hlCreateLabelByDijkstraWithStalling(
                bTreeNodesRankedDescOrelRank,
                nodesRankedDescOrelRank,
                edgesContractedUpwardsOrelSource,
                edgesContractedDownwardsOrelTarget,
                labelForwardMultimap, nodeIdV, hHop, isForward);
            Debug("fully prune forward label" << endl);
            //hlPruneLabelByBootstrapping(allLabelsNrel,
            // labelForwardMultimap, nodeIdV, isForward);
            Debug("create and fill forwardLabel");
            AttributeRelation* labelForwardArel = new
            AttributeRelation(forwardDataSubRelFileId,
                              forwardDataSubRelTypeInfo,
                              labelForwardMultimap.size());
            labelForwardArel->setPartOfNrel(true);
            hlFillForwardOrReverseLabel(labelForwardArel,
                                        forwardDataRel, labelForwardMultimap);
            Debug("add forward label to all labels" << endl);
            allLabelsTuple->PutAttribute(
                HL_INDEX_OF_FORWARD_LABEL_IN_ALL_LABELS_TUPLE,
                labelForwardArel);

            //Create reverse label
            isForward = false;
            Debug("create reverse label" << endl);
            hlCreateLabelByDijkstraWithStalling(
                bTreeNodesRankedDescOrelRank,
                nodesRankedDescOrelRank,
                edgesContractedUpwardsOrelSource,
                edgesContractedDownwardsOrelTarget,
                labelReverseMultimap, nodeIdV, hHop, isForward);
            Debug("fully prune reverse label" << endl);
            //hlPruneLabelByBootstrapping(allLabelsNrel,
            // labelReverseMultimap, nodeIdV, isForward);
            Debug("create and fill reverseLabel");
            AttributeRelation* labelReverseArel = new
            AttributeRelation(reverseDataSubRelFileId,
                              reverseDataSubRelTypeInfo,
                              labelReverseMultimap.size());
            labelReverseArel->setPartOfNrel(true);
            hlFillForwardOrReverseLabel(labelReverseArel,
                                        reverseDataRel, labelReverseMultimap);

            Debug("add forward label to all labels" << endl);
            allLabelsTuple->PutAttribute(
                HL_INDEX_OF_REVERSE_LABEL_IN_ALL_LABELS_TUPLE,
                labelReverseArel);

            Debug("finish current all labels Tuple" << endl);
            allLabelsNrel->getPrimary()->AppendTuple(
                allLabelsTuple);

            allLabelsTuple->DeleteIfAllowed();

            allLabelsTuple = 0;


            Debug("get next tuple" << endl);
            //Free Outgoing-Iteration
            currentNodeV->DeleteIfAllowed();
            currentNodeV = 0;
            currentNodeV = nodesRankIter->GetNextTuple();
        }

        delete nodesRankIter;

        Debug("reorder labels" << endl);
        //hlReorderLabels(); //XXTODO

        Debug("finish hlCreateLabels" << endl);

        return true;
    }

    /**
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
        Debug("start hlFillForwardOrReverseLabel" <<
              endl);

        std::multimap<int, int> tmpIndexTupleIds;
        for (std::multimap<int, std::tuple<int, int, double>>::iterator
                fwdOrRvsLabelMultimapIter =
                    fwdOrRvsLabelMultimap.begin();
                fwdOrRvsLabelMultimapIter !=
                fwdOrRvsLabelMultimap.end();
                ++fwdOrRvsLabelMultimapIter)
        {
            Debug("next iteration" << endl);

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

            Debug("currNode rank: " << currNodeIdNew <<
                  " id: " << currNodeId << " parent: " <<
                  currParentNodeId << endl);

            TupleType* tt1 = fwdOrRvsDataRel->GetTupleType();
            Tuple* relInsertTuple = new Tuple(tt1);


            //get tupleId of currParentNodeId from temporal structure
            Debug("suche nach parent von " << currParentNodeId
                  << " anzahl gesamt vorhanden: " <<
                  tmpIndexTupleIds.size() << endl);
            std::multimap<int, int>::iterator
            parentTupleIdIter = tmpIndexTupleIds.find(
                                    currParentNodeId);

            //only check distances when current min tuple from h-hop
            // reverse search does exist in current forward label
            int currParentTupleId = -1;
            if(parentTupleIdIter != tmpIndexTupleIds.end())
            {
                currParentTupleId = (*parentTupleIdIter).second;
                Debug("parent tupleId found (" <<
                      currParentTupleId << ") of parent nodeId: " <<
                      currParentNodeId << " for currTuple: " <<
                      currNodeId << endl);
            }

            Debug("Add data to Data-Relation" << endl);
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

            Debug("Free Resources" << endl);
            relInsertTuple->DeleteIfAllowed();
        }

        Debug("finish hlFillForwardOrReverseLabel" <<
              endl);

        return true;
    }


    /**
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
        Debug("start hlGetRankById" << endl);

        int resultingRank = -1;
        int tupleIdRankedNodeInt = -1;

        BTreeIterator* btreeNodesRankedIter =
            btreeNodesRanked->ExactMatch(givenNodeId);

        if(btreeNodesRankedIter)
        {
            Debug("using btree iterator" << endl);
            if(btreeNodesRankedIter->Next())
            {
                Debug("next iterated element" << endl);
                tupleIdRankedNodeInt =
                    btreeNodesRankedIter->GetId();
                Debug("tupleId: " << tupleIdRankedNodeInt <<
                      endl);
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
            Debug("Warning, no tuple found" << endl);
            GenericRelationIterator* nodesRankedIter =
                nodesRanked->MakeScan();
            Tuple* tmpTuple = nodesRankedIter->GetNextTuple();
            while(tmpTuple)
            {

                CcInt* tmpId = (CcInt*) tmpTuple->GetAttribute(
                                   HL_INDEX_OF_ID_IN_NODES_RANKED);
                Debug("next iteration nodeId: " <<
                      tmpId->GetIntval() << endl);

                if(tmpId->GetIntval() == givenNodeId->GetIntval())
                {
                    Debug("right tuple found, going to break" <<
                          endl);
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
            Debug("finish iteration" << endl);
            delete nodesRankedIter;
        }

        if(rankedNodeTuple)
        {
            Debug("tuple found" << endl);
            CcInt* currentRank = (CcInt*)
                                 rankedNodeTuple->GetAttribute(
                                     HL_INDEX_OF_RANK_IN_NODES_RANKED);
            resultingRank = currentRank->GetIntval();
            rankedNodeTuple->DeleteIfAllowed();
            Debug("node: " << givenNodeId->GetIntval() <<
                  " rank: "<< currentRank->GetIntval() << endl);
        }

        Debug("end hlGetRankById" << endl);

        return resultingRank;
    }


    /**
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
        Debug("start hlCreateLabelByDijkstraWithStalling"
              << endl);
        int currRank = -1;

        Debug("insert source node itself into label" <<
              endl);
        currRank = hlGetRankById(btreeNodesRanked,
                                 nodesRanked, givenSourceIdS);
        std::tuple<int, int, double> insertTupleSource =
            std::make_tuple(givenSourceIdS->GetIntval(), -1,
                            0.0);
        fwdOrRvsLabelMultimap.insert(
            pair<int, std::tuple<int, int, double>>(currRank,
                    insertTupleSource));

        Debug("create labelNotYetVisitedNodes" << endl);
        std::multimap<double, std::tuple<int, int>>
                labelNotYetVisitedNodes;

        Debug("create stillVisitedNodes" << endl);
        std::vector<int> labelStillVisitedNodes;
        labelStillVisitedNodes.push_back(
            givenSourceIdS->GetIntval());

        Debug("process initial edges" << endl);
        hlCreateLabelScanNewVertices(
            labelNotYetVisitedNodes, labelStillVisitedNodes,
            edgesContractedUpwardsOrelSource,
            edgesContractedDownwardsOrelTarget,
            givenSourceIdS, 0.0, isForward);

        Debug("process iterative edges" << endl);
        while(labelNotYetVisitedNodes.size() > 0)
        {
            Debug("get next minV" << endl);
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

            Debug("remove tuple from orel" << endl);
            labelNotYetVisitedNodes.erase(
                labelNotYetVisitedNodesIter);

            Debug("Add to still visited Nodes" << endl);
            labelStillVisitedNodes.push_back(currMinVNodeId);

            Debug("check for witness before inserting by doing"
             " h-Hop-Reverse-Search"
                  << endl);
            bool isPruned = hlCreateLabelCheckForWitness(
                                fwdOrRvsLabelMultimap,
                                edgesContractedUpwardsOrelSource,
                                edgesContractedDownwardsOrelTarget,
                                currMinVNodeIdCcInt, currMinVDist, hHop,
                                isForward);

            if(!isPruned)
            {
                Debug("append tuple to label" << endl);
                currRank = -1;
                currRank = hlGetRankById(btreeNodesRanked,
                                         nodesRanked, currMinVNodeIdCcInt);
                std::tuple<int, int, double> insertTupleLabel =
                    std::make_tuple(currMinVNodeId,
                                    currMinVParentNodeId, currMinVDist);
                fwdOrRvsLabelMultimap.insert(
                    pair<int, std::tuple<int, int, double>>(currRank,
                            insertTupleLabel));

                Debug("scan new vertices" << endl);
                hlCreateLabelScanNewVertices(
                    labelNotYetVisitedNodes, labelStillVisitedNodes,
                    edgesContractedUpwardsOrelSource,
                    edgesContractedDownwardsOrelTarget,
                    currMinVNodeIdCcInt, currMinVDist, isForward);
            }

            currMinVNodeIdCcInt->DeleteIfAllowed(
                true); //TODO: notwendig? bei getAttribute auch nicht notwendig
        }

        Debug("finish hlCreateLabelByDijkstraWithStalling"
              << endl);

        return true;
    }



    /**
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
        Debug("Start hlCreateLabelScanNewVertices with isForward = "
              << isForward << endl);

        Debug("prepare iteration" << endl);
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
            Debug("get outgoing edges (v, w) within upwardsgraph"
                  << endl);
            edgesContractedUpOrDownOrelSourceOrTargetIter =
                edgesContractedUpwardsOrelSource->MakeRangeScan(
                    fromSourceV, toSourceV);
        }
        else
        {
            Debug("get incoming edges (w, v) within downwardsgraph"
                  << endl);
            edgesContractedUpOrDownOrelSourceOrTargetIter =
                edgesContractedDownwardsOrelTarget->MakeRangeScan(
                    fromSourceV, toSourceV);
        }
        currentContractedEdgeVW =
            edgesContractedUpOrDownOrelSourceOrTargetIter->GetNextTuple();

        Debug("iterate and store outgoing/ incoming egdes (v, w)/ (w, v)"
         " into labelNotYetVisited"
              << endl);
        while(currentContractedEdgeVW)
        {
            Debug("next iteration" << endl);
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
                Debug("append tuple to labelNotYet" << endl);

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
                Debug("insert or update" << endl);
                //Either insert the new scanned W/V into not visited
                // list or update it if it sill exists and has a shorter
                // path via current v
                hlInsertOrUpdateTupleInNotYetVisitedList(-1,
                        currParentNotYetMultimapTuple,
                        currentContractedEdgeVW, labelNotYetVisitedNodes,
                        isForward, isHHop);
                Debug("inserted or updated" << endl);
            }

            Debug("get next tuple" << endl);
            //Free Outgoing-Iteration
            currentContractedEdgeVW->DeleteIfAllowed();
            currentContractedEdgeVW = 0;
            currentContractedEdgeVW =
                edgesContractedUpOrDownOrelSourceOrTargetIter->GetNextTuple();
        }
        Debug("finish iteration" << endl);

        //Free resources
        delete edgesContractedUpOrDownOrelSourceOrTargetIter;

        Debug("end hlCreateLabelScanNewVertices" << endl);
        return true;
    }

    /**
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
        Debug("Start hlCreateLabelCheckForWitness" <<
              endl);
        bool isPruned = false;

        Debug("create reverseNotYetVisitedNodes" << endl);
        std::multimap<double, std::tuple<int, int>>
                reverseNotYetVisitedNodes;

        Debug("create stillVisitedNodes" << endl);
        std::vector<int> stillVisitedNodes;
        stillVisitedNodes.push_back(
            fwdCurrSourceOrTargetW->GetIntval());

        //scan new vertices in h-hop reverse search
        hlCreateLabelCheckForWitnessScanNewVertices(
            reverseNotYetVisitedNodes, stillVisitedNodes,
            edgesContractedUpwardsOrelSource,
            edgesContractedDownwardsOrelTarget,
            fwdCurrSourceOrTargetW, 0.0, 0, isForward);

        Debug("process iterative edges in hHop-reverse" <<
              endl);
        while(reverseNotYetVisitedNodes.size() > 0)
        {
            Debug("next iteration witness" << endl);

            Debug("get next revMinV" << endl);
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

            Debug("remove tuple from orel" << endl);
            reverseNotYetVisitedNodes.erase(
                reverseNotYetVisitedNodesIter);

            Debug("check for witness for: " << revMinVNodeID
                  << endl);
            std::multimap<int, std::tuple<int, int, double>>::iterator
                    fwdOrRvsLabelMultimapIter =
                        fwdOrRvsLabelMultimap.find(revMinVNodeID);

            //only check distances when current min tuple from h-hop
            // reverse search does exist in current forward label
            if(fwdOrRvsLabelMultimapIter !=
                    fwdOrRvsLabelMultimap.end())
            {
                Debug("potential witness access found" << endl);
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
                    Debug("witness found" << endl);
                    isPruned = true;
                    break;
                }
            }

            Debug("scan new vertices" << endl);
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

        Debug("finish hlCreateLabelCheckForWitness" <<
              endl);
        return isPruned;
    }


    /**
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
        Debug("Start hlCreateLabelCheckForWitnessScanNewVertices"
              << endl);

        Debug("prepare iteration" << endl);
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
            Debug("get incoming edges (u, v) within downwardsgraph"
                  << endl);
            reverseSearchEdgeOrelIter =
                edgesContractedDownwardsOrelTarget->MakeRangeScan(
                    fromSourceV, toSourceV);
        }
        else
        {
            Debug("get outgoing edges (v, u) within upwardsgraph"
                  << endl);
            reverseSearchEdgeOrelIter =
                edgesContractedUpwardsOrelSource->MakeRangeScan(
                    fromSourceV, toSourceV);
        }
        currentReverseEdgeUV =
            reverseSearchEdgeOrelIter->GetNextTuple();

        Debug("iterate and store incoming/outgoing egdes (u, v)/(v, u)"
         " into reverseNotYetVisited"
              << endl);
        while(currentReverseEdgeUV)
        {
            Debug("next iteration witness scan new vertices"
                  << endl);

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
                Debug("append tuple to reverseNotYet" << endl);

                //create auxiliary data
                std::tuple<double, int, int>
                currParentNotYetMultimapTuple = std::make_tuple(
                                                    revGivenDistanceVToW,
                                    revGivenTargetIdV->GetIntval(),
                                                    revGivenHopDepthV);
                Debug("insert or update" << endl);
                //Either insert the new scanned W/V into not visited list
                // or update it if it sill exists and has a shorter
                // path via current v
                bool isHHop = true;
                hlInsertOrUpdateTupleInNotYetVisitedList(-1,
                        currParentNotYetMultimapTuple,
                        currentReverseEdgeUV, reverseNotYetVisitedNodes,
                        not isForward, isHHop);
                Debug("inserted or updated" << endl);
            }

            Debug("get next tuple" << endl);
            //Free Outgoing-Iteration
            currentReverseEdgeUV->DeleteIfAllowed();
            currentReverseEdgeUV = 0;
            currentReverseEdgeUV =
                reverseSearchEdgeOrelIter->GetNextTuple();
        }
        Debug("finish iteration" << endl);

        //Free resources
        delete reverseSearchEdgeOrelIter;

        Debug("end hlCreateLabelCheckForWitnessScanNewVertices"
              << endl);
        return true;
    }


    /**
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
                Debug("Witness found by bootstrapping" << endl);

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
        Debug("Free resources" << endl);

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
        Debug("finish" << endl);

        return true;
    }


    /**
     * XXTODO
     */
    bool hlReorderLabels() const
    {
        //XXTODO
        return true;
    }


    /**
     * Gets all edges by the given source and iterates over them searching
     * for the given target.
     * Returns the first edge-Tuple if found.
     *
     * @param currOrel
     * @param sourceId
     * @param targetId
     * @return a new Tuple-Pointer
     */
    Tuple* hlGetEdgeFromOrel(OrderedRelation*
                             currOrel, CcInt* sourceId, CcInt* targetId) const
    {
        Debug("start hlGetEdgeFromOrel" << endl);

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
            CcInt* currTargetId = (CcInt*)
                                  currTuple->GetAttribute(
                                      HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);

            if(currTargetId->GetIntval() ==
                    targetId->GetIntval())
            {
                Debug("tupleFound, break" << endl);
                break;
            }
            else
            {
                currTuple->DeleteIfAllowed();
                currTuple = 0;
                currTuple = currOrelIter->GetNextTuple();
            }
        }

        Debug("finish hlGetEdgeFromOrel" << endl);
        return currTuple;
    }

    /**
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
    bool hlResolveShortcuts(OrderedRelation*
                            edgesOrelSource, Tuple* currEdge,
                            std::vector<Tuple*> &currPath,
                            bool isForward) const
    {
        Debug("start hlResolveShortcuts" << endl);

        //Get parameters of currEdge
        CcInt* sourceId = (CcInt*) currEdge->GetAttribute(
                              HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
        CcInt* targetId = (CcInt*) currEdge->GetAttribute(
                              HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
        CcInt* parentId = (CcInt*) currEdge->GetAttribute(
                              HL_INDEX_OF_PARENT_ID_IN_EDGE_TUPLE);

        Tuple* parentTuple = 0;

        //check whether curr Edge is shortcut
        if(parentId->GetIntval() > -1)
        {
            Debug("source: " << sourceId->GetIntval() <<
                  " parent: " << parentId->GetIntval() <<
                  " target: " << targetId->GetIntval() <<
                  " is shortcut, doing recursive call" << endl);
            if(isForward)
            {
                //reverse insert because we will reverse output them because
                // we handle forward label here
                // (which means that we are inserting the hub first, then its
                // parents and the source last
                Debug("edge from parent: " <<
                      parentId->GetIntval() << " to target: " <<
                      targetId->GetIntval() << endl);
                parentTuple = hlGetEdgeFromOrel(edgesOrelSource,
                                                parentId, targetId);
                hlResolveShortcuts(edgesOrelSource, parentTuple,
                                   currPath, isForward);

                Debug("edge from source: " <<
                      sourceId->GetIntval() << " to parent: " <<
                      parentId->GetIntval() << endl);
                parentTuple = hlGetEdgeFromOrel(edgesOrelSource,
                                                sourceId, parentId);
                hlResolveShortcuts(edgesOrelSource, parentTuple,
                                   currPath, isForward);

            }
            else
            {
                //vice versa as forward
                Debug("edge from source: " <<
                      sourceId->GetIntval() << " to parent: " <<
                      parentId->GetIntval() << endl);
                parentTuple = hlGetEdgeFromOrel(edgesOrelSource,
                                                sourceId, parentId);
                hlResolveShortcuts(edgesOrelSource, parentTuple,
                                   currPath, isForward);

                Debug("edge from parent: " <<
                      parentId->GetIntval() << " to target: " <<
                      targetId->GetIntval() << endl);
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
            Debug("is original Edge, pushback into path-vector");
            currPath.push_back(currEdge);
        }

        Debug("finish hlResolveShortcuts" << endl);
        return true;
    }

    /**
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
    bool hlGetPathViaPoints(OrderedRelation*
                            edgesWithShortcutsOrelSource, Relation* dataRel,
                            Tuple* givenHubTuple, std::vector<Tuple*> &path,
                            bool isForward) const
    {
        Debug("start hlGetPathViaPoints" << endl);

        bool isFinished = false;
        Tuple* currTuple = givenHubTuple;

        while(!isFinished)
        {
            CcInt* currTupleNodeIdNew = (CcInt*)
                                        currTuple->GetAttribute(
                            HL_INDEX_OF_HUB_NODE_ID_NEW_IN_LABEL_TUPLE);
            CcInt* currTupleNodeId = (CcInt*)
                                     currTuple->GetAttribute(
                                    HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE);
            CcInt* currTupleParentTupleId = (CcInt*)
                                            currTuple->GetAttribute(
                        HL_INDEX_OF_HUB_PARENT_TUPLE_ID_IN_LABEL_TUPLE);

            Debug("current tuple rank: " <<
                  currTupleNodeIdNew->GetIntval() << " nodeId: " <<
                  currTupleNodeId->GetIntval() << endl);

            if(currTupleParentTupleId->GetIntval() > -1)
            {
                TupleId currParentTupleId = (TupleId)
                                            currTupleParentTupleId->GetIntval();
                Tuple* currParentTuple = dataRel->GetTuple(
                                             currParentTupleId, false);

                if(!currParentTuple)
                {
                    Debug("Fehler, Tuple aus dataRel ist null mit TupleId: "
                          << currParentTupleId << endl);
                    return false;
                }

                CcInt* currParentNodeId = (CcInt*)
                                          currParentTuple->GetAttribute(
                                    HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE);
                Debug("curr parentNodeId: " <<
                      currParentNodeId->GetIntval() <<
                      " parentTupleid: " << currParentTupleId << endl);

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

                currEdge = hlGetEdgeFromOrel(
                               edgesWithShortcutsOrelSource, currSourceId,
                               currTargetId);


                CcInt* currEdgeParentVia = (CcInt*)
                                           currEdge->GetAttribute(
                                    HL_INDEX_OF_PARENT_ID_IN_EDGE_TUPLE);

                Debug("resolve shortcuts for source: " <<
                      currSourceId->GetIntval() << " target: " <<
                      currTargetId->GetIntval() << endl);
                hlResolveShortcuts(edgesWithShortcutsOrelSource,
                                   currEdge, path, isForward);

                currTuple->DeleteIfAllowed();
                currTuple = 0;
                currTuple = currParentTuple;

            }
            else
            {
                Debug("parent found exit" << endl);

                currTuple->DeleteIfAllowed();
                currTuple = 0;
                isFinished = true;
            }
        }

        Debug("finish hlGetPathViaPoints" << endl);

        return true;
    }


    /**
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
     *  all edges including shortcuts
     * @param shortestPath is an Relation representing the result of this
     *  function
     * @param sourceNodeId is the actual source node of the given forward or
     *  reverse label
     * @param targetNodeId is a boolean that indicates whether a forward or
     *  reverselabel shall be created
     * @return true, since the result ist stored in shortestPath
     */
    bool hlQuery(NestedRelation* allLabelsNRel,
                 BTree* allLabelsBTree,
                 OrderedRelation* edgesWithShortcutsOrelSource,
                 Relation* shortestPath, CcInt* sourceNodeId,
                 CcInt* targetNodeId) const
    {
        Debug("start hlQuery" << endl);


        Debug("get btree iterator by source: " <<
              sourceNodeId->GetIntval() << " target: " <<
              targetNodeId->GetIntval() << endl);

        BTreeIterator* bTreeSourceIter =
            allLabelsBTree->ExactMatch(sourceNodeId);
        BTreeIterator* bTreeTargetIter =
            allLabelsBTree->ExactMatch(targetNodeId);


        Debug("get next tuple from btree forward" <<
              endl);

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
                Debug("Fehler: kein Tuple in BTree für Source enthalten,"
                 " breche Anfrage ab"
                      << endl);
                return false;
            }
            delete bTreeSourceIter;
        }
        else
        {
            Debug("Fehler: sourceNode wurde nicht in Labels gefunden,"
             " breche Anfrage ab"
                  << endl);
            return false;
        }

        Debug("get next tuple from btree reverse" <<
              endl);

        if(bTreeTargetIter)
        {
            if(bTreeTargetIter->Next())
            {
                tupleIdTargetInt = bTreeTargetIter->GetId();
            }
            else
            {
                Debug("Fehler: kein Tuple in BTree für Target enthalten,"
                 " breche Anfrage ab"
                      << endl);
                return false;
            }
            delete bTreeTargetIter;
        }
        else
        {
            Debug("Fehler: targetNode wurde nicht in Labels gefunden,"
             " breche Anfrage ab"
                  << endl);
            return false;
        }

        Debug("create TupleIds and get primary" << endl);

        TupleId tupleIdSource = (TupleId)
                                tupleIdSourceInt;
        TupleId tupleIdTarget = (TupleId)
                                tupleIdTargetInt;

        Relation* primary = allLabelsNRel->getPrimary();

        Debug("get tuple from primary" << endl);

        Tuple* tupleSource = primary->GetTuple(
                                 tupleIdSource, false);
        Tuple* tupleTarget = primary->GetTuple(
                                 tupleIdTarget, false);

        Debug("get arel from tupled" << endl);

        AttributeRelation* attrRelSource =
            (AttributeRelation*) tupleSource->GetAttribute(
                HL_INDEX_OF_FORWARD_LABEL_IN_ALL_LABELS_TUPLE);
        AttributeRelation* attrRelTarget =
            (AttributeRelation*) tupleTarget->GetAttribute(
                HL_INDEX_OF_REVERSE_LABEL_IN_ALL_LABELS_TUPLE);

        Debug("get subrel from nrel" << endl);

        SubRelation* dataSubRelForward =
            allLabelsNRel->getSubRel(
                hlForwardLabelColumnName());
        SubRelation* dataSubRelReverse =
            allLabelsNRel->getSubRel(
                hlReverseLabelColumnName());

        Debug("get datarel from subrel" << endl);

        Relation* dataRelForward = dataSubRelForward->rel;
        Relation* dataRelReverse = dataSubRelReverse->rel;

        Debug("get tupleids from arels" << endl);

        DbArray<TupleId>* tupleIdsForward =
            attrRelSource->getTupleIds();
        DbArray<TupleId>* tupleIdsReverse =
            attrRelTarget->getTupleIds();

        TupleId tidForward;
        TupleId tidReverse;

        int indexForward = 0;
        int indexReverse = 0;


        Debug("get tuple is for data rel from dbarray" <<
              endl);

        tupleIdsForward->Get(indexForward,
                             tidForward); //XXTODO: TupleId hier kein Zeiger?
        tupleIdsReverse->Get(indexReverse,
                             tidReverse); //XXTODO: TupleId hier kein Zeiger?

        Debug("get tuples from datarel" << endl);

        Tuple* currTupleDataRelSource =
            dataRelForward->GetTuple(tidForward,
                                     false); //XXTODO: TupleId hier kein Zeiger?
        Tuple* currTupleDataRelTarget =
            dataRelReverse->GetTuple(tidReverse,
                                     false); //XXTODO: TupleId hier kein Zeiger?

        Tuple* currMinHubSource = 0;
        Tuple* currMinHubTarget = 0;

        double currMinDist = -1.0;

        Debug("parallel sweep" << endl);

        //usually breaks when tupleId-arrays reach end of index
        while(currTupleDataRelSource &&
                currTupleDataRelTarget)
        {
            //get HubNodeIds of both Iterators
            CcInt* currHubNodeIdNewSource = (CcInt*)
                                        currTupleDataRelSource->GetAttribute(
                                HL_INDEX_OF_HUB_NODE_ID_NEW_IN_LABEL_TUPLE);
            CcInt* currHubNodeIdNewTarget = (CcInt*)
                            currTupleDataRelTarget->GetAttribute(
                            HL_INDEX_OF_HUB_NODE_ID_NEW_IN_LABEL_TUPLE);

            if(currHubNodeIdNewSource->GetIntval() ==
                    currHubNodeIdNewTarget->GetIntval())
            {

                Debug("source and target are equal: " <<
                      currHubNodeIdNewSource->GetIntval() << endl);

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
                    Debug("new min found: " << currMinDistTmp <<
                          endl);
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

                Debug("increase both" << endl);

                //increase both iterators
                indexForward++;
                indexReverse++;

                if(tupleIdsForward->Size() > indexForward)
                {
                    tupleIdsForward->Get(indexForward, tidForward);
                }
                else
                {
                    Debug("end of Array tupleIdsForward reached (size: "
                          << tupleIdsForward->Size() << " currIndex: " <<
                          indexForward << "), breaking loop" << endl);
                    break;
                }

                if(tupleIdsReverse->Size() > indexReverse)
                {
                    tupleIdsReverse->Get(indexReverse, tidReverse);
                }
                else
                {
                    Debug("end of Array tupleIdsReverse reached (size: "
                          << tupleIdsReverse->Size() << " currIndex: " <<
                          indexReverse << "), breaking loop" << endl);
                    break;
                }

                Debug("increase both - set next tuples. forward: "
                      << tidForward << " reverse: " << tidReverse <<
                      endl);

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

                Debug("increase forward" << endl);

                indexForward++;

                if(tupleIdsForward->Size() > indexForward)
                {
                    tupleIdsForward->Get(indexForward, tidForward);
                }
                else
                {
                    Debug("end of Array tupleIdsForward reached (size: "
                          << tupleIdsForward->Size() << " currIndex: " <<
                          indexForward << "), breaking loop" << endl);
                    break;
                }

                Debug("increase forward - set next tuple. forward: "
                      << tidForward << endl);
                currTupleDataRelSource = dataRelForward->GetTuple(
                                             tidForward,
                                false); //XXTODO: TupleId hier kein Zeiger?
            }
            else
            {
                //increase reverse iterator
                currTupleDataRelTarget->DeleteIfAllowed();
                currTupleDataRelTarget = 0;

                Debug("increase reverse" << endl);

                indexReverse++;

                if(tupleIdsReverse->Size() > indexReverse)
                {
                    tupleIdsReverse->Get(indexReverse, tidReverse);
                }
                else
                {
                    Debug("end of Array tupleIdsReverse reached (size: "
                          << tupleIdsReverse->Size() << " currIndex: " <<
                          indexReverse << "), breaking loop" << endl);
                    break;
                }

                Debug("increase reverse - set next tuple. reverse: "
                      << tidReverse << endl);
                currTupleDataRelTarget = dataRelReverse->GetTuple(
                                             tidReverse,
                                             false);
                                             //XXTODO: TupleId hier kein Zeiger?
            }
        }

        std::vector<Tuple*> pathForward;
        std::vector<Tuple*> pathReverse;

        Debug("lookup paths" << endl);

        if(currMinHubSource)
        {
            Debug("get PathVia for currMinHubSource" << endl);
            hlGetPathViaPoints(edgesWithShortcutsOrelSource,
                               dataRelForward, currMinHubSource, pathForward,
                               true);
        }
        else
        {
            Debug("Warnung, Tuple currMinHubSource ist null, fahre fort"
                  << endl);
        }
        if(currMinHubTarget)
        {
            Debug("get PathVia for currMinHubTarget" << endl);
            hlGetPathViaPoints(edgesWithShortcutsOrelSource,
                               dataRelReverse, currMinHubTarget, pathReverse,
                               false);
        }
        else
        {
            Debug("Warnung, Tuple currMinHubTarget ist null, fahre fort"
                  << endl);
        }

        Debug("iterate forward path" << endl);

        //Reverse-Iterate through pathForward and add edges
        // (source and target) to shortestPath
        for (std::vector<Tuple*>::reverse_iterator
                pathForwardIterReverse = pathForward.rbegin();
                pathForwardIterReverse != pathForward.rend();
                ++pathForwardIterReverse )
        {
            Debug("insert next edge" << endl);
            Tuple* currEdge = *(pathForwardIterReverse);

            //XXTODO
            CcInt* sourceTmp = (CcInt*)
                               currEdge->GetAttribute(
                                   HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
            CcInt* targetTmp = (CcInt*)
                               currEdge->GetAttribute(
                                   HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
            Debug("appendtuple forward: source = " <<
                  sourceTmp->GetIntval() << " target:  " <<
                  targetTmp->GetIntval() << endl);

            shortestPath->AppendTuple(currEdge);
            currEdge->DeleteIfAllowed();
        }

        Debug("iterate reverse path" << endl);

        //forward iterate through pathReverse and add it to shortestPath
        for (std::vector<Tuple*>::iterator pathReverseIter
                = pathReverse.begin();
                pathReverseIter != pathReverse.end();
                ++pathReverseIter)
        {
            Debug("insert next edge" << endl);
            Tuple* currEdge = *(pathReverseIter);

            //XXTODO
            CcInt* sourceTmp = (CcInt*)
                               currEdge->GetAttribute(
                                   HL_INDEX_OF_SOURCE_IN_EDGE_TUPLE);
            CcInt* targetTmp = (CcInt*)
                               currEdge->GetAttribute(
                                   HL_INDEX_OF_TARGET_IN_EDGE_TUPLE);
            Debug("appendtuple reverse: source = " <<
                  sourceTmp->GetIntval() << " target:  " <<
                  targetTmp->GetIntval() << endl);

            shortestPath->AppendTuple(currEdge);
            currEdge->DeleteIfAllowed();
        }

        Debug("finish hlQuery" << endl);

        return true;
    }


private:
    double x;
    double y;
    double r;
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
        "1x Nrel, 1x BTree, 1x Orel, 2x CcInt expected.";

    // check the number of arguments
    if(!nl->HasLength(args,5))
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
    if(!CcInt::checkType(nl->Fourth(args)))
    {
        return listutils::typeError(err +
                                    " (fifth arg no CcInt)");
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
    Debug("start hlOneHopReverseSearchVM" << endl);
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* orelEdgesSource =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* orelEdgesTarget =
        (OrderedRelation*) args[1].addr;
    CcInt* nodeId = (CcInt*) args[2].addr;

    Debug("Parameter gelesen" << endl);
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

    Debug("do hlOneHopReverseSearch" << endl);
    std::multimap<int, std::tuple<int, double>>
            oneHopReverseMultimap;
    k->hlOneHopReverseSearch(orelEdgesSource,
                             orelEdgesTarget, nodeId, oneHopReverseMultimap);

    Debug("Add elements from oneHopReverseMultimap to oneHopReverseOrelId"
          << endl);
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

    Debug("finish hlOneHopReverseSearchVM" << endl);

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



/**
 * Erwartet eine relEdgesSource bei der die enthaltenen Edges in der
 *  Reihenfolge  der rel auch in diesem Test-Operator verarbeitet
 *  werden sollen.
 * Der 3. Parameter enthält die ID für den Startknoten, dieser
 *  erkennt alle Edges der rel,
 *  die vom Startknoten aus weggehen.
 *
 * Folgende Testfälle sollten (jedoch in beliebiger Reihenfolge)
 *  enthalten sein:
 *
 * Die Erste Edge sollte S->V sein, es wird erwartet, dass diese nicht
 *  eingefügt wird.
 * Die zweite Edge sollte S->W sein, es wird erwartet, dass diese eingefügt
 *  wird.
 * Die dritte Edge sollte W->X sein, es wird erwartet, dass diese eingefügt
 *  wird.
 * Die folgenden Edges sollten X1->X2 sein, es wird erwartet, dass diese
 *  eingefügt werden.
 * Die nächste Edge sollte X3->X4 sein, wobei X4 bereits in der Orel enthalten
 *  sein soll (hier U genannt),
 *  der Weg S->X4 sollte länger sein, als der bisherige Weg S->U, es erwartet,
 *  dass diese edge nicht eingefügt wird.
 * Die folgenden Edges sollten X1->X2 sein, es wird erwartet, dass diese
 *  eingefügt werden.
 * Die nächste Edge sollte X5->X6 sein, wobei X6 bereits in der Orel
 *  enthalten sein soll (hier U2 genannt),
 *  der Weg S->X6 sollte kürzer sein, als der bisherige Weg S->U2,
 *  es erwartet, dass diese edge eingefügt wird.
 * Es sollten keine weiteren Edges im Input enthalten sein.
 *  Die Orel wird an die Query zurückgegeben.
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
    Debug("start" << endl);
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
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    Debug("result gelesen" << endl);

    //minW Tuple aus Container-Relation auslesen
    GenericRelationIterator*
    currMinSingleTupleRelWIter =
        currMinSingleTupleRelW->MakeScan();
    Tuple* currMinSingleTupleRelWTuple =
        currMinSingleTupleRelWIter->GetNextTuple();
    delete currMinSingleTupleRelWIter;

    if(!currMinSingleTupleRelWTuple)
    {
        Debug("kein Tuple, komisch" << endl);
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



    Debug("los" << endl);
    k->hlForwardSearchIterativeStepsScanNewVertices(
        orelEdgesSource,
        currNodeIdV->GetIntval(),
        currMinNotYetVisitedNodeTuple,
        notYetVisitedNodesMultiMap, stillVisitedNodesSet,
        oneHopReverseMultimap,
        distSV->GetRealval());
    Debug("fertig" << endl);



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
    Debug("start" << endl);
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
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    Debug("result gelesen" << endl);


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





    Debug("los" << endl);
    k->hlForwardSearchProcessIncomingEdgeIterativeSteps(
        orelEdgesSource,
        currNodeIdV->GetIntval(),
        notYetVisitedNodesMultiMap,
        stillVisitedNodesSet, oneHopReverseMultimap,
        hHop->GetIntval(),
        distSV->GetRealval());
    Debug("fertig" << endl);



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
    Debug("start" << endl);
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
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    Debug("result gelesen" << endl);


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


    Debug("los" << endl);
    k->hlForwardSearchProcessIncomingEdgeInitialSteps(
        orelEdgesSource,
        currNodeIdV->GetIntval(), currNodeIdS,
        notYetVisitedNodesMultiMap,
        oneHopReverseMultimap, distSV->GetRealval());
    Debug("fertig" << endl);



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
    Debug("start" << endl);
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* copyMultimapReverseSearchXT =
        (OrderedRelation*) args[0].addr;
    CcInt* currentSourceNodeFwdS = (CcInt*)
                                   args[1].addr;
    CcInt* currentVertexIdFwdV = (CcInt*)
                                 args[2].addr;
    CcReal* distSV = (CcReal*) args[3].addr;
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    OrderedRelation*
    shortcutsToBeCreatedOrelSourceToBeDeleted =
        (OrderedRelation*) result.addr; // cast the result

    Debug("result gelesen" << endl);


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

    Debug("los" << endl);
    k->hlForwardSearchCreateAndAppendShortcuts(
        shortcutsMultimap,
        reverseXTMultimap,
        currentSourceNodeFwdS->GetIntval(),
        currentVertexIdFwdV->GetIntval(),
        distSV->GetRealval());
    Debug("fertig" << endl);

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
    Debug("start" << endl);
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
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    Debug("result gelesen" << endl);

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

    Debug("los" << endl);
    k->hlForwardSearchProcessIncomingEdge(
        orelEdgesSource, shortcutsMultimap,
        currentVertexIdFwdV->GetIntval(),
        currentSourceNodeFwdS,
        stillVisitedNodesSet, oneHopReverseMultimap,
        hDepth->GetIntval(),
        distSV->GetRealval());
    Debug("fertig" << endl);

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
    Debug("start" << endl);
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* edgesCopyOrelSource =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* edgesCopyOrelTarget =
        (OrderedRelation*) args[1].addr;
    CcInt* currentContractV = (CcInt*) args[2].addr;
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    Debug("result gelesen" << endl);

    k->hlRemoveContractedEdgesFromEdgesRelations(
        edgesCopyOrelSource,
        edgesCopyOrelTarget, currentContractV);

    //free resources
    delete k;

    //Änderungen an als Parameter übergebenen Orels persistieren
    qp->SetModified(qp->GetSon(s,0));
    qp->SetModified(qp->GetSon(s,1));

    Debug("fertig" << endl);
    return 0;
}


int hlRemoveParallelEdgesFromEdgesRelationsVM (
    Word* args, Word& result,
    int message, Word& local, Supplier s)
{
    Debug("start" << endl);
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* edgesCopyOrelSource =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* edgesCopyOrelTarget =
        (OrderedRelation*) args[1].addr;
    OrderedRelation*
    shortcutsToBeCreatedOrelSourceToBeDeleted =
        (OrderedRelation*) args[2].addr;
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    Debug("result gelesen" << endl);



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

    Debug("fertig" << endl);
    return 0;
}


int hlDoContractionVM (Word* args, Word& result,
                       int message, Word& local,
                       Supplier s)
{
    Debug("start" << endl);
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* edgesWithViaOrelSource =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* edgesWithViaOrelTarget =
        (OrderedRelation*) args[1].addr;
    CcInt* currentVToBeContracted = (CcInt*)
                                    args[2].addr;
    CcInt* hHop = (CcInt*) args[3].addr;
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    OrderedRelation*
    shortcutsToBeCreatedOrelSourceToBeDeleted =
        (OrderedRelation*) result.addr; // cast the result

    Debug("result gelesen" << endl);

    std::multimap<int, std::tuple<int, double, int>>
            shortcutsMultimap;

    Debug("los" << endl);
    k->hlDoContraction(edgesWithViaOrelSource,
                       edgesWithViaOrelTarget,
                       shortcutsMultimap, currentVToBeContracted,
                       hHop->GetIntval());
    Debug("fertig" << endl);

    //convert multimap back to OrdeedRelation
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
    Debug("start" << endl);
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    OrderedRelation* nodesWithRankOrelRank =
        (OrderedRelation*) args[0].addr;
    OrderedRelation* edgesWithViaOrelSource =
        (OrderedRelation*) args[1].addr;
    OrderedRelation* edgesWithViaOrelTarget =
        (OrderedRelation*) args[2].addr;
    CcInt* hHop = (CcInt*) args[3].addr;
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    Debug("result gelesen" << endl);

    Debug("los" << endl);
    k->hlIterateOverAllNodesByRankAscAndDoContraction(
        nodesWithRankOrelRank,
        edgesWithViaOrelSource, edgesWithViaOrelTarget,
        hHop->GetIntval());
    Debug("fertig" << endl);

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
    Debug("start" << endl);
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
    CcInt* isForwardInt = (CcInt*)
                          args[5].addr; //Debug
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    OrderedRelation* reverseNotYetVisitedNodesOrel =
        (OrderedRelation*)
        result.addr; // cast the result

    Debug("result gelesen" << endl);

    std::vector<int> stillVisitedNodes;
    stillVisitedNodes.push_back(
        revGivenTargetIdV->GetIntval());

    std::multimap<double, std::tuple<int, int>>
            reverseNotYetVisitedNodes;

    Debug("los" << endl);
    k->hlCreateLabelCheckForWitnessScanNewVertices(
        reverseNotYetVisitedNodes,
        stillVisitedNodes,
        edgesContractedUpwardsOrelSource,
        edgesContractedDownwardsOrelTarget,
        revGivenTargetIdV,
        revGivenDistanceVToW->GetRealval(),
        revGivenHopDepthV->GetIntval(),
        isForward->GetBoolval());
    Debug("fertig" << endl);

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
    Debug("start" << endl);
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
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result

    Debug("result vorbereitet" << endl);


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


    Debug("los" << endl);
    bool isPruned = k->hlCreateLabelCheckForWitness(
                        fwdOrRvsLabelMultimap,
                        edgesContractedUpwardsOrelSource,
                        edgesContractedDownwardsOrelTarget,
                        givenTargetIdW, givenDistanceSW->GetRealval(),
                        hHopSize->GetIntval(),
                        isForward->GetBoolval());
    Debug("fertig" << endl);


    resultInt->Set(true, isPruned);

    Debug("result vorbereitet" << endl);

    //free resources
    delete k;

    return 0;
}


int hlCreateLabelScanNewVerticesVM (Word* args,
                                    Word& result, int message,
                                    Word& local, Supplier s)
{
    Debug("start" << endl);
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
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    OrderedRelation* reverseNotYetVisitedNodesOrel =
        (OrderedRelation*)
        result.addr; // cast the result

    Debug("result gelesen" << endl);



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


    Debug("los" << endl);
    k->hlCreateLabelScanNewVertices(
        labelNotYetVisitedNodes, stillVisitedNodes,
        edgesContractedUpwardsOrelSource,
        edgesContractedDownwardsOrelTarget,
        givenTargetIdV, givenDistanceSV->GetRealval(),
        isForward->GetBoolval());
    Debug("fertig" << endl);


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
    Debug("start" << endl);
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    BTree* bTreeNodesRanked = (BTree*) args[0].addr;
    OrderedRelation* nodesRanked = (OrderedRelation*)
                                   args[1].addr;
    CcInt* givenNodeId = (CcInt*) args[2].addr;
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result

    Debug("result gelesen" << endl);

    int retVal = -2;

    Debug("los" << endl);
    retVal = k->hlGetRankById(bTreeNodesRanked,
                              nodesRanked, givenNodeId);
    Debug("fertig" << endl);

    resultInt->Set(true, retVal);

    //free resources
    delete k;

    return 0;
}


int hlCreateLabelByDijkstraWithStallingVM (
    Word* args, Word& result,
    int message, Word& local, Supplier s)
{
    Debug("start" << endl);
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
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    Relation* fwdOrRvsLabelRel = (Relation*)
                                 result.addr; // cast the result

    Debug("result gelesen" << endl);


    std::multimap<int, std::tuple<int, int, double>>
            fwdOrRvsLabelMultimap;

    Debug("los" << endl);
    k->hlCreateLabelByDijkstraWithStalling(
        bTreeNodesRanked, nodesRanked,
        edgesWithViaOrelSource, edgesWithViaOrelTarget,
        fwdOrRvsLabelMultimap,
        sourceNodeId, hHop->GetIntval(),
        isForward->GetBoolval());
    Debug("fertig" << endl);


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
    Debug("start" << endl);
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    Relation* fwdOrRvsLabelRelation = (Relation*)
                                      args[0].addr;
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    NestedRelation* allLabelsNrel = (NestedRelation*)
                                    result.addr;
    // cast the result

    Debug("result gelesen" << endl);


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

        Debug("fill Label nex tuple with rank: " <<
              currTupleHubIdNew->GetIntval() << " id: " <<
              currTupleHubID->GetIntval() << " parentId: " <<
              currTupleHubParentId->GetIntval() << endl);
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


    Debug("los" << endl);

    k->hlFillForwardOrReverseLabel(labelfwdOrRvsArel,
                                   fwdOrRvsDataRel,
                                   labelfwdOrRvsMultimap);
    Debug("fertig" << endl);

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
    Debug("start" << endl);
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
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    Relation* shortesPath = (Relation*)
                            result.addr; // cast the result

    Debug("result gelesen" << endl);

    BTreeIterator* bTreeSourceIter =
        allLabelsBTree->ExactMatch(rootNodeId);

    int tupleIdSource = -1;
    if(bTreeSourceIter->Next())
    {
        tupleIdSource = bTreeSourceIter->GetId();
        Debug("id: " << tupleIdSource << endl);
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
        Debug("next TupleId: " << tid << endl);
        Tuple* tmpCurrDataTuple = dataRel->GetTuple(tid,
                                  false);

        if(tmpCurrDataTuple)
        {
            CcInt* currDataTupleHubId = (CcInt*)
                                        tmpCurrDataTuple->GetAttribute(
                    HubLabelClass::HL_INDEX_OF_HUB_NODE_ID_IN_LABEL_TUPLE);
            Debug("currNodeId: " <<
                  currDataTupleHubId->GetIntval() <<
                  " searchedHubId: " << hubId->GetIntval()<< endl);
            if(hubId->GetIntval() ==
                    currDataTupleHubId->GetIntval())
            {
                Debug("Tuple found for given hubId, break loop and hold"
                      " current Tuple object" << endl);
                currDataTuple = tmpCurrDataTuple;
                break;
            }
        }
        //mpCurrDataTuple->DeleteIfAllowed(); //XXTODO darf das?
    }
    tupleIds->destroyIfNonPersistent(); //XXTODO: richig so?

    if(!currDataTuple)
    {
        Debug("Warnung: kein Tuple in Arel gefunden, breche ab"
              << endl);
        delete k;
        currTuple->DeleteIfAllowed();
        return 0;
    }


    std::vector<Tuple*> shortestPathVector;


    Debug("los" << endl);
    k->hlGetPathViaPoints(
        edgesWithShortcutsOrelSource, dataRel,
        currDataTuple,
        shortestPathVector, isForward->GetBoolval());
    Debug("fertig" << endl);


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


//XXTODO
int hlQueryVM (Word* args, Word& result,
               int message, Word& local, Supplier s)
{
    Debug("start" << endl);
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    NestedRelation* allLabelsNRel = (NestedRelation*)
                                    args[0].addr;
    BTree* allLabelsBTree = (BTree*) args[1].addr;
    OrderedRelation* edgesWithShortcutsOrelSource =
        (OrderedRelation*)
        args[2].addr;
    CcInt* sourceNodeId = (CcInt*) args[3].addr;
    CcInt* targetNodeId = (CcInt*) args[4].addr;
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    Relation* shortesPath = (Relation*)
                            result.addr; // cast the result

    Debug("result gelesen" << endl);

    Debug("los" << endl);
    k->hlQuery(allLabelsNRel, allLabelsBTree,
               edgesWithShortcutsOrelSource,
               shortesPath, sourceNodeId, targetNodeId);
    Debug("fertig" << endl);

    //free resources
    delete k;

    return 0;
}


//XXTODO
int hlPruneLabelByBootstrappingVM (Word* args,
                                   Word& result, int message,
                                   Word& local, Supplier s)
{
    Debug("start" << endl);
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    /*
    OrderedRelation* nodesWithRankOrelRank = (OrderedRelation*) args[0].addr;
    OrderedRelation* edgesWithViaOrelSource = (OrderedRelation*) args[1].addr;
    OrderedRelation* edgesWithViaOrelTarget = (OrderedRelation*) args[2].addr;
    CcInt* hHop = (CcInt*) args[3].addr;
    CcInt* calcFunction = (CcInt*) args[4].addr;
    */
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    Debug("result gelesen" << endl);

    Debug("los" << endl);
    //k->hlPruneLabelByBootstrapping(hHop);
    Debug("fertig" << endl);

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
    Debug("start" << endl);
    HubLabelClass* k = new HubLabelClass(1, 2, 3);

    /*
    OrderedRelation* nodesWithRankOrelRank = (OrderedRelation*) args[0].addr;
    OrderedRelation* edgesWithViaOrelSource = (OrderedRelation*) args[1].addr;
    OrderedRelation* edgesWithViaOrelTarget = (OrderedRelation*) args[2].addr;
    CcInt* hHop = (CcInt*) args[3].addr;
    CcInt* calcFunction = (CcInt*) args[4].addr;
    */
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    CcInt* resultInt = (CcInt*)
                       result.addr; // cast the result
    resultInt->Set(true, 1);

    Debug("result gelesen" << endl);

    Debug("los" << endl);
    //k->hlReorderLabels(hHop);
    Debug("fertig" << endl);

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
    Debug("start" << endl);
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
    Debug("Parameter gelesen" << endl);

    result = qp->ResultStorage(
                 s);       // use the result storage
    NestedRelation* allLabelsNrel = (NestedRelation*)
                                    result.addr;
    // cast the result

    Debug("result gelesen" << endl);

    Debug("los" << endl);
    k->hlCreateLabels(allLabelsNrel,
                      bTreeNodesWithRankOrelRank,
                      nodesWithRankOrelRank, edgesWithViaOrelSource,
                      edgesWithViaOrelTarget,
                      hHop->GetIntval());
    Debug("fertig" << endl);

    //free resources
    delete k;

    return 0;
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

*/


OperatorSpec hlCalcRankSpec(
    "orel x orel x ccint -> real",
    "hlCalcRank(_, _, _)",
    "hlCalcRank(orelEdgesSource, orelEdgesTarget, nodeId)"
    " -- Computes and returns the rank of the given nodeId.",
    "query hlCalcRank(EdgesOrelSource feed head[10] oconsume,"
    " EdgesOreltarget feed head[10] oconsume, 12345678)"
);


OperatorSpec hlOneHopReverseSearchSpec(
    "orel x orel x ccint -> orel",
    "hlOneHopReverseSearch(_, _, _)",
    "hlOneHopReverseSearch(orelEdgesSource, orelEdgesTarget, nodeIdV)"
    " -- Processes a one hop reverse Search and returns an orel containing"
    " the results.",
    "query hlOneHopReverseSearch(EdgesTestOrelSource, EdgesTestOrelTarget,"
    " 7264168)"
);


OperatorSpec hlHHopForwardSearchSpec(
    "orel x ore x orel x ccint x ccint -> real",
    "hlHHopForwardSearch(_, _, _, _, _)",
    "hlHHopForwardSearch(orelEdgesSource, orelEdgesTarget,"
    " multimapReverseSearchXT, contractNodeV, hHop)",
    "query hlHHopForwardSearch(orelEdgesSource, orelEdgesTarget,"
    " multimapReverseSearchXT, 1505, -1) feed consume"
);


OperatorSpec hlForwardSearchGetDistSpec(
    "orel x ccint x ccint -> real",
    "hlForwardSearchGetDist(_, _, _)",
    "hlForwardSearchGetDist(oneHopReverseSearchOrelXT, contractNodeV,"
    " currentTargetNodeT)",
    "query hlForwardSearchGetDist(OneHopReverseSearchToBeDeleted, 7264168,"
    " 7264033)"
);


OperatorSpec hlRemoveTFromCurrentWitnessListSpec(
    "orel x ccint -> orel",
    "hlRemoveTFromCurrentWitnessList(_, _)",
    "hlRemoveTFromCurrentWitnessList(oneHopReverseSearchOrelXT,"
    " currentTargetNodeT)",
    "query hlRemoveTFromCurrentWitnessList(OneHopReverseSearchToBeDeleted,"
    " 7264033)"
);


OperatorSpec
hlForwardSearchCheckForWitnessPathSpec(
    "orel x ccint x ccint x ccreal x ccreal -> orel",
    "hlForwardSearchCheckForWitnessPath(_, _, _, _, _)",
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
    "hlInsertOrUpdateTupleInNotYetVisitedList(EdgesSourceTmpRel,"
    " currentNodeV, currentNodeS, isForward)",
    "query hlInsertOrUpdateTupleInNotYetVisitedList(EdgesTestRelSourceTmp,"
    " 1492, 1424, 1) feed consume"
);


OperatorSpec
hlForwardSearchIterativeStepsScanNewVerticesSpec(
    "orel x orel x orel x orel x rel x ccint x ccreal -> ccin",
    "hlForwardSearchIterativeStepsScanNewVertices(_, _, _, _, _, _, _, _)",
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
    "hlForwardSearchCreateAndAppendShortcuts(copyMultimapReverseSearchXT,"
    " currentSourceNodeFwdS, currentVertexIdFwdV, distSV) feed consume",
    "query hlForwardSearchCreateAndAppendShortcuts("
    "copyMultimapReverseSearchXT, 1492, 1505, 1000.0) feed consume"
);


OperatorSpec
hlForwardSearchProcessIncomingEdgeSpec(
    "orel x orel x orel x orel x ccint x ccint x ccint x ccreal -> orel",
    "hlForwardSearchProcessIncomingEdge(_, _, _, _, _, _, _, _)",
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
    "hlRemoveContractedEdgesFromEdgesRelations(hlEdgesOrelSourceCopy,"
    " hlEdgesOrelTargetCopy, hlCurrentContractV)",
    "query hlRemoveContractedEdgesFromEdgesRelations(hlEdgesOrelSource,"
    " hlEdgesOrelTarget, 1505)"
);


OperatorSpec
hlRemoveParallelEdgesFromEdgesRelationsSpec(
    "orel x orel x orel -> ccint",
    "hlRemoveParallelEdgesFromEdgesRelations(_, _, _)",
    "hlRemoveParallelEdgesFromEdgesRelations(hlEdgesOrelSourceCopy,"
    " hlEdgesOrelTargetCopy, hlShortcutsToBeCreatedOrelToBeDeleted)",
    "query hlRemoveParallelEdgesFromEdgesRelations(hlEdgesOrelSource,"
    " hlEdgesOrelTarget, hlShortcutsToBeCreatedOrelToBeDeleted)"
);


OperatorSpec hlDoContractionSpec(
    "orel x orel x ccint x ccint -> orel",
    "hlDoContraction(_, _, _, _)",
    "hlDoContraction(edgesWithViaOrelSource, edgesWithViaOrelTarget,"
    " currentVToBeContracted, hHop) feed consume",
    "query hlDoContraction(edgesWithViaOrelSource, edgesWithViaOrelTarget,"
    " 1505, 2) feed consume"
);


OperatorSpec
hlIterateOverAllNodesByRankAscAndDoContractionSpec(
    "orel x orel x orel x ccint -> ccint",
    "hlIterateOverAllNodesByRankAscAndDoContraction(_, _, _, _)",
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
    "hlCreateLabelCheckForWitness(hlUpwardEdgesOrelSource,"
    " hlDownwardEdgesOrelTarget, hlFwdOrRvsLabel, currentNode, hlhHop,"
    " currentNodeDistToSource, isForward)",
    "query hlCreateLabelCheckForWitness(hlUpwardEdgesOrelSource,"
    " hlDownwardEdgesOrelTarget, hlFwdOrRvsLabel, 273, 10, 0.0, 1)"
);


OperatorSpec hlCreateLabelScanNewVerticesSpec(
    "orel x orel x rel x ccint x ccreal x ccint (ccbool) -> orel",
    "hlCreateLabelScanNewVertices(_, _, _, _, _, _)",
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
    "hlGetRankById(hlNodesWithRankOrelRank_NodeIdNew,"
    " hlNodesWithRankOrelRank, givenNodeId) feed consume",
    "query hlGetRankById(hlNodesWithRankOrelRank_NodeIdNew,"
    " hlNodesWithRankOrelRank, 700) feed consume"
);


OperatorSpec
hlCreateLabelByDijkstraWithStallingSpec(
    "btree x orel x orel x orel x Ccint x ccint x ccint (ccbool) -> orel",
    "hlCreateLabelByDijkstraWithStalling(_, _, _, _, _, _, _)",
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
    "rel -> nrel",
    "hlFillForwardOrReverseLabel(_)",
    "hlFillForwardOrReverseLabel(fwdOrRvsLabelRel) feed consume",
    "query hlFillForwardOrReverseLabel(fwdOrRvsLabelRel) feed consume"
);


OperatorSpec hlGetPathViaPointsSpec(
    "nrel x btree x orel x ccint x ccint x ccint (ccbool) -> rel",
    "hlGetPathViaPoints(_, _, _, _, _, _)",
    "hlGetPathViaPoints(allLabelsNRel, allLabelsBTree,"
    " hlEdgesOrelSourceParentVia, rootNodeId, hubId, isForward) feed"
    " consume",
    "query hlGetPathViaPoints(allLabelsNRel, allLabelsBTree,"
    " hlEdgesOrelSourceParentVia, 62, 630, 1) feed consume"
);


OperatorSpec hlQuerySpec(
    "nrel x btree x orel x ccint x ccint -> rel",
    "hlQuery(_, _, _, _, _)",
    "hlQuery(allLabelsNRel, allLabelsBTree, hlEdgesOrelSourceParentVia,"
    " sourceNodeId, targetNodeId)",
    "query hlQuery(allLabelsNRel, allLabelsBTree,"
    " hlEdgesOrelSourceParentVia, 62, 630)"
);


OperatorSpec hlPruneLabelByBootstrappingSpec(
    "orel x orel x orel x ccint x ccint -> ccint",
    "hlPruneLabelByBootstrapping(_, _, _, _, _)",
    "hlPruneLabelByBootstrapping(nodesWithRankOrelRank,"
    " edgesWithViaOrelSource, edgesWithViaOrelTarget, hHop, calcFunction)",
    "query hlPruneLabelByBootstrapping(nodesWithRankOrelRank,"
    " edgesWithViaOrelSource, edgesWithViaOrelTarget, 2, 1)"
);


OperatorSpec hlReorderLabelsSpec(
    "orel x orel x orel x ccint x ccint -> ccint",
    "hlReorderLabels(_, _, _, _, _)",
    "hlReorderLabels(nodesWithRankOrelRank, edgesWithViaOrelSource,"
    " edgesWithViaOrelTarget, hHop, calcFunction)",
    "query hlReorderLabels(nodesWithRankOrelRank, edgesWithViaOrelSource,"
    " edgesWithViaOrelTarget, 2, 1)"
);


OperatorSpec hlCreateLabelsSpec(
    "btree x orel x orel x orel x ccint -> nrel",
    "hlCreateLabels(_, _, _, _)",
    "query hlCreateLabels(hlNodesWithRankOrelRank_NodeIdNew,"
    " nodesWithRankOrelRank, hlUpwardEdgesOrelSourceParentVia,"
    " hlDownwardEdgesOrelTargetParentVia, hHop) feed consume)",
    "query hlCreateLabels(hlNodesWithRankOrelRank_NodeIdNew,"
    " nodesWithRankOrelRank, hlUpwardEdgesOrelSourceParentVia,"
    " hlDownwardEdgesOrelTargetParentVia, 10) feed consume"
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
