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

//paragraph [1] title: [{\Large \bf ]   [}]
//characters    [2]    verbatim:   [\verb@]    [@]
//[ue] [\"{u}]
//[toc] [\tableofcontents]

""[2]

[1] Implementation file of the MapMatching Algebra

January-April 2012, Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of the class
~MapMatchingAlgebra~.

For more detailed information see MapMatchingAlgebra.h.

2 Defines and Includes

*/

#include "MapMatchingAlgebra.h"
#include <NestedList.h>
#include <ListUtils.h>
#include <NList.h>
#include <LogMsg.h>
#include <QueryProcessor.h>
#include <ConstructorTemplates.h>
#include <StandardTypes.h>

#include <NetworkAlgebra.h>
#include <TemporalNetAlgebra.h>
#include <OrderedRelationAlgebra.h>
#include <FTextAlgebra.h>
#include "ONetwork.h"
#include "ONetworkEdge.h"

extern NestedList* nl;
extern QueryProcessor *qp;

#include <TypeMapUtils.h>
#include <Symbols.h>

#include <string>
using namespace std;

#include "MapMatchingMHT.h"
#include "MapMatchingData.h"
#include "GPXImporter.h"
#include "NetworkAdapter.h"
#include "ONetworkAdapter.h"
#include "MapMatchingMHTMGPointCreator.h"
#include "MapMatchingMHTPointsCreator.h"
#include "MapMatchingMHTOEdgeTupleStreamCreator.h"
#include "MapMatchingMHTMPointCreator.h"


namespace mapmatch {

/*
3 mapmatchmht-operator

3.1 Operator-Info

*/
struct MapMatchMHTInfo : OperatorInfo
{

    MapMatchMHTInfo()
    {
        name      = "mapmatchmht";
        signature = Network::BasicType() + " x " +
                    MPoint::BasicType() + " -> " +
                    MGPoint::BasicType();

        appendSignature(Network::BasicType() + " x " +
                        FText::BasicType()  + " -> " +
                        MGPoint::BasicType());

        appendSignature(Network::BasicType() + " x " +
                        "(stream (tuple([Lat:real, Lon:real, Time:DateTime "
                                        "[,Fix:int] [,Sat:int] [,Hdop : real]"
                                        "[,Vdop:real] [,Pdop:real] "
                                        "[,Course:real] [,Speed(m/s):real]])))"
                        + " -> " +
                        MGPoint::BasicType());

        appendSignature(Network::BasicType() + " x " +
                        MPoint::BasicType() + " x " +
                        CcReal::BasicType() + " -> " +
                        MGPoint::BasicType());

        appendSignature(Network::BasicType() + " x " +
                        FText::BasicType()  + " x " +
                        CcReal::BasicType() + " -> " +
                        MGPoint::BasicType());

        appendSignature(Network::BasicType() + " x " +
                        CcReal::BasicType() + " x " +
                        "(stream (tuple([Lat:real, Lon:real, Time:DateTime "
                                        "[,Fix:int] [,Sat:int] [,Hdop : real]"
                                        "[,Vdop:real] [,Pdop:real] "
                                        "[,Course:real] [,Speed(m/s):real]])))"
                        + CcReal::BasicType() + " -> " +
                        MGPoint::BasicType());

        syntax    = "mapmatchmht ( _ , _ [, _ ] )";
        meaning   = "The operation maps the MPoint or "
                    "the data of a gpx-file or "
                    "the data of a tuple stream "
                    "to the given network."
                    "With the optional real-parameter the scaling"
                    "factor of the network can be specified."
                    "Result is a MGPoint.";
        example   = "mapmatchmht (DortmundNet, 'Trk_Dortmund.gpx')";
    }
};


/*
3.2 Type-Mapping

*/

static ListExpr GetMMDataIndexesOfTupleStream(ListExpr TupleStream);
static ListExpr AppendLists(ListExpr List1, ListExpr List2);

ListExpr OpMapMatchingMHTTypeMap(ListExpr in_xArgs)
{
    NList param(in_xArgs);

    if(param.length() != 2 && param.length() != 3)
        return listutils::typeError("two or three arguments expected");

    if (!param.first().isSymbol(Network::BasicType()))
        return listutils::typeError("1st argument must be " +
                                                          Network::BasicType());

    if (!param.second().isSymbol(MPoint::BasicType()) &&
        !param.second().isSymbol(FText::BasicType()) &&
        !listutils::isTupleStream(param.second().listExpr()))
    {
        return listutils::typeError("2nd argument must be " +
                                    MPoint::BasicType() + " or " +
                                    FText::BasicType() + " or " +
                                    "tuple stream");
    }

    if (param.length() == 3)
    {
        if (!param.third().isSymbol(CcReal::BasicType()))
            return listutils::typeError("3rd argument must be " +
                                                           CcReal::BasicType());
    }

    if (listutils::isTupleStream(param.second().listExpr()))
    {
        ListExpr Ind = GetMMDataIndexesOfTupleStream(param.second().listExpr());

        if(nl->Equal(Ind,nl->TypeError()))
            return Ind;
        else
        {
            if (param.length() == 2)
            {
                // Add scaling (default 1.0) and indexes
                return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                                         AppendLists(
                                             nl->OneElemList(nl->RealAtom(1.0)),
                                             Ind),
                                         nl->SymbolAtom(MGPoint::BasicType()));
            }
            else
            {
                // Add indexes
                return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                                         Ind,
                                         nl->SymbolAtom(MGPoint::BasicType()));
            }
        }
    }
    else
    {
        if (param.length() == 2)
        {
            // Add scaling (default 1.0)
            return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                                     nl->OneElemList(nl->RealAtom(1.0)),
                                     nl->SymbolAtom(MGPoint::BasicType()));
        }
        else
            return nl->SymbolAtom(MGPoint::BasicType());
    }
}

static ListExpr GetMMDataIndexesOfTupleStream(ListExpr TupleStream)
{
    ListExpr attrType;
    int nAttrLatIndex = listutils::findAttribute(
                          nl->Second(nl->Second(TupleStream)), "Lat", attrType);
    if(nAttrLatIndex==0)
    {
        return listutils::typeError("'Lat' not found in attr list");
    }
    else
    {
        if (!listutils::isSymbol(attrType, CcReal::BasicType()))
        {
            return listutils::typeError("'Lat' must be " +
                                        CcReal::BasicType());
        }
    }

    int nAttrLonIndex = listutils::findAttribute(
                          nl->Second(nl->Second(TupleStream)), "Lon", attrType);
    if (nAttrLonIndex == 0)
    {
        return listutils::typeError("'Lon' not found in attr list");
    }
    else
    {
        if (!listutils::isSymbol(attrType, CcReal::BasicType()))
        {
            return listutils::typeError("'Lon' must be " +
                                        CcReal::BasicType());
        }
    }

    int nAttrTimeIndex = listutils::findAttribute(
                         nl->Second(nl->Second(TupleStream)), "Time", attrType);
    if (nAttrTimeIndex == 0)
    {
        return listutils::typeError("'Time' not found in attr list");
    }
    else
    {
        if (!listutils::isSymbol(attrType, DateTime::BasicType()))
        {
            return listutils::typeError("'Time' must be " +
                                        DateTime::BasicType());
        }
    }

    int nAttrFixIndex = listutils::findAttribute(
                          nl->Second(nl->Second(TupleStream)), "Fix", attrType);
    if (nAttrFixIndex != 0)
    {
        if (!listutils::isSymbol(attrType, CcInt::BasicType()))
        {
            return listutils::typeError("'Fix' must be " +
                                        CcInt::BasicType());
        }
    }

    int nAttrSatIndex = listutils::findAttribute(
                          nl->Second(nl->Second(TupleStream)), "Sat", attrType);
    if (nAttrSatIndex != 0)
    {
        if (!listutils::isSymbol(attrType, CcInt::BasicType()))
        {
            return listutils::typeError("'Sat' must be " +
                                        CcInt::BasicType());
        }
    }

    int nAttrHdopIndex = listutils::findAttribute(
                         nl->Second(nl->Second(TupleStream)), "Hdop", attrType);
    if (nAttrHdopIndex != 0)
    {
        if (!listutils::isSymbol(attrType, CcReal::BasicType()))
        {
            return listutils::typeError("'Hdop' must be " +
                                        CcReal::BasicType());
        }
    }

    int nAttrVdopIndex = listutils::findAttribute(
                         nl->Second(nl->Second(TupleStream)), "Vdop", attrType);
    if (nAttrVdopIndex != 0)
    {
        if (!listutils::isSymbol(attrType, CcReal::BasicType()))
        {
            return listutils::typeError("'Vdop' must be " +
                                        CcReal::BasicType());
        }
    }

    int nAttrPdopIndex = listutils::findAttribute(
                         nl->Second(nl->Second(TupleStream)), "Pdop", attrType);
    if (nAttrPdopIndex != 0)
    {
        if (!listutils::isSymbol(attrType, CcReal::BasicType()))
        {
            return listutils::typeError("'Pdop' must be " +
                                        CcReal::BasicType());
        }
    }

    int nAttrCourseIndex = listutils::findAttribute(
                       nl->Second(nl->Second(TupleStream)), "Course", attrType);
    if (nAttrCourseIndex != 0)
    {
        if (!listutils::isSymbol(attrType, CcReal::BasicType()))
        {
            return listutils::typeError(
                    "'Course' must be " + CcReal::BasicType());
        }
    }

    int nAttrSpeedIndex = listutils::findAttribute(
                        nl->Second(nl->Second(TupleStream)), "Speed", attrType);
    if (nAttrSpeedIndex != 0)
    {
        if (!listutils::isSymbol(attrType, CcReal::BasicType()))
        {
            return listutils::typeError(
                    "'Speed' must be " + CcReal::BasicType());
        }
    }

    int nAttrEleIndex = listutils::findAttribute(
                          nl->Second(nl->Second(TupleStream)), "Ele", attrType);
    if (nAttrEleIndex != 0)
    {
        if (!listutils::isSymbol(attrType, CcReal::BasicType()))
        {
            return listutils::typeError(
                    "'Ele' must be " + CcReal::BasicType());
        }
    }

    --nAttrLatIndex;
    --nAttrLonIndex;
    --nAttrTimeIndex;
    --nAttrFixIndex;
    --nAttrSatIndex;
    --nAttrHdopIndex;
    --nAttrVdopIndex;
    --nAttrPdopIndex;
    --nAttrCourseIndex;
    --nAttrSpeedIndex;
    --nAttrEleIndex;

    ListExpr Ind = nl->OneElemList(nl->IntAtom(nAttrLatIndex));
    ListExpr Last = Ind;
    Last = nl->Append(Last, nl->IntAtom(nAttrLonIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrTimeIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrFixIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrSatIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrHdopIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrVdopIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrPdopIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrCourseIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrSpeedIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrEleIndex));

    return Ind;
}


/*
3.3 Value-Mapping

*/
int OpMapMatchingMHTMPointValueMapping(Word* args,
                                       Word& result,
                                       int message,
                                       Word& local,
                                       Supplier in_xSupplier)
{
    // cout << "OpMapMatchingMHTMPointValueMapping called" << endl;

    // Initialize Result
    result = qp->ResultStorage(in_xSupplier);
    MGPoint* pRes = static_cast<MGPoint*>(result.addr);

    // get Arguments
    Network *pNetwork = static_cast<Network*>(args[0].addr);
    MPoint *pMPoint = static_cast<MPoint*>(args[1].addr);
    CcReal *pNetworkScale = static_cast<CcReal*>(args[2].addr);

    // Do Map Matching

    NetworkAdapter Network(pNetwork, pNetworkScale->GetValue());
    MapMatchingMHT MapMatching(&Network, pMPoint);

    MGPointCreator Creator(&Network, pRes);

    if (!MapMatching.DoMatch(&Creator))
    {
        // Error
    }

    return 0;
}

int OpMapMatchingMHTGPXValueMapping(Word* args,
                                    Word& result,
                                    int message,
                                    Word& local,
                                    Supplier in_xSupplier)
{
    // cout << "OpMapMatchingMHTGPXValueMapping called" << endl;

    // Initialize Result
    result = qp->ResultStorage(in_xSupplier);
    MGPoint* pRes = static_cast<MGPoint*>(result.addr);

    // get Arguments
    Network* pNetwork = static_cast<Network*>(args[0].addr);
    FText* pFileName = static_cast<FText*>(args[1].addr);
    std::string strFileName = pFileName->GetValue();
    CcReal *pNetworkScale = static_cast<CcReal*>(args[2].addr);

    // Do Map Matching

    NetworkAdapter Network(pNetwork, pNetworkScale->GetValue());
    MapMatchingMHT MapMatching(&Network, strFileName);

    MGPointCreator Creator(&Network, pRes);

    if (!MapMatching.DoMatch(&Creator))
    {
        // Error
    }

    return 0;
}

static shared_ptr<MapMatchDataContainer> GetMMDataFromTupleStream(
                                                                Supplier Stream,
                                                                Word* args,
                                                                int nOffset)
{

    // see also GetMMDataIndexesOfTupleStream

    shared_ptr<MapMatchDataContainer> pContData(new MapMatchDataContainer);

    const CcInt* pIdxLat    = static_cast<CcInt*>(args[nOffset + 0].addr);
    const CcInt* pIdxLon    = static_cast<CcInt*>(args[nOffset + 1].addr);
    const CcInt* pIdxTime   = static_cast<CcInt*>(args[nOffset + 2].addr);
    const CcInt* pIdxFix    = static_cast<CcInt*>(args[nOffset + 3].addr);
    const CcInt* pIdxSat    = static_cast<CcInt*>(args[nOffset + 4].addr);
    const CcInt* pIdxHdop   = static_cast<CcInt*>(args[nOffset + 5].addr);
    const CcInt* pIdxVdop   = static_cast<CcInt*>(args[nOffset + 6].addr);
    const CcInt* pIdxPdop   = static_cast<CcInt*>(args[nOffset + 7].addr);
    const CcInt* pIdxCourse = static_cast<CcInt*>(args[nOffset + 8].addr);
    const CcInt* pIdxSpeed  = static_cast<CcInt*>(args[nOffset + 9].addr);
    //const CcInt* pIdxEle    = static_cast<CcInt*>(args[nOffset + 10].addr);

    const int nIdxLat    = pIdxLat->GetValue();
    const int nIdxLon    = pIdxLon->GetValue();
    const int nIdxTime   = pIdxTime->GetValue();
    const int nIdxFix    = pIdxFix->GetValue();
    const int nIdxSat    = pIdxSat->GetValue();
    const int nIdxHdop   = pIdxHdop->GetValue();
    const int nIdxVdop   = pIdxVdop->GetValue();
    const int nIdxPdop   = pIdxPdop->GetValue();
    const int nIdxCourse = pIdxCourse->GetValue();
    const int nIdxSpeed  = pIdxSpeed->GetValue();
    //const int nIdxEle    = pIdxEle->GetValue();

    if (nIdxLat < 0 || nIdxLon < 0 || nIdxTime < 0)
    {
        assert(false);
        return pContData;
    }

    Word wTuple;
    qp->Open(Stream);
    qp->Request(Stream, wTuple);
    while (qp->Received(Stream))
    {
        Tuple* pTpl = (Tuple*)wTuple.addr;

        CcReal* pLat    = static_cast<CcReal*>(pTpl->GetAttribute(nIdxLat));
        CcReal* pLon    = static_cast<CcReal*>(pTpl->GetAttribute(nIdxLon));
        DateTime* pTime = static_cast<DateTime*>(pTpl->GetAttribute(nIdxTime));

        CcInt* pFix     = NULL;
        if (nIdxFix >= 0)
            pFix = static_cast<CcInt*>(pTpl->GetAttribute(nIdxFix));

        CcInt* pSat     = NULL;
        if (nIdxSat >= 0)
            pSat = static_cast<CcInt*>(pTpl->GetAttribute(nIdxSat));

        CcReal* pHdop   = NULL;
        if (nIdxHdop >= 0)
            pHdop = static_cast<CcReal*>(pTpl->GetAttribute(nIdxHdop));

        CcReal* pVdop   = NULL;
        if (nIdxVdop >= 0)
            pVdop = static_cast<CcReal*>(pTpl->GetAttribute(nIdxVdop));

        CcReal* pPdop   = NULL;
        if (nIdxPdop >= 0)
            pPdop = static_cast<CcReal*>(pTpl->GetAttribute(nIdxPdop));

        CcReal* pCourse = NULL;
        if (nIdxCourse >= 0)
            pCourse = static_cast<CcReal*>(pTpl->GetAttribute(nIdxCourse));

        CcReal* pSpeed  = NULL;
        if (nIdxSpeed >= 0)
            pSpeed = static_cast<CcReal*>(pTpl->GetAttribute(nIdxSpeed));

        /*CcReal* pEle    = NULL;
        if (nIdxEle >= 0)
            pEle = static_cast<CcReal*>(pTpl->GetAttribute(nIdxEle));
        */

        if (pLat != NULL && pLon != NULL && pTime != NULL &&
            pLat->IsDefined() && pLon->IsDefined() && pTime->IsDefined())
        {
            MapMatchData Data(pLat->GetValue(),
                              pLon->GetValue(),
                              pTime->millisecondsToNull());

            if (pFix != NULL && pFix->IsDefined())
                Data.m_nFix = pFix->GetValue();

            if (pSat != NULL && pSat->IsDefined())
                Data.m_nSat = pSat->GetValue();

            if (pHdop != NULL && pHdop->IsDefined())
                Data.m_dHdop = pHdop->GetValue();

            if (pVdop != NULL && pVdop->IsDefined())
                Data.m_dVdop = pVdop->GetValue();

            if (pPdop != NULL && pPdop->IsDefined())
                Data.m_dPdop = pPdop->GetValue();

            if (pCourse != NULL && pCourse->IsDefined())
                Data.m_dCourse = pCourse->GetValue();

            if (pSpeed != NULL && pSpeed->IsDefined())
                Data.m_dSpeed = pSpeed->GetValue();

            /*
            if (pEle != NULL && pEle->IsDefined())
                Data.m_dEle = pEle->GetValue();
            */

            pContData->Append(Data);
        }

        pTpl->DeleteIfAllowed();
        pTpl = NULL;

        qp->Request(Stream, wTuple);
    }
    qp->Close(Stream);

    return pContData;
}

int OpMapMatchingMHTStreamValueMapping(Word* args,
                                       Word& result,
                                       int message,
                                       Word& local,
                                       Supplier in_xSupplier)
{
    // cout << "OpMapMatchingMHTStreamValueMapping called" << endl;

    // Initialize Result
    result = qp->ResultStorage(in_xSupplier);
    MGPoint* pRes = static_cast<MGPoint*>(result.addr);

    // get Arguments
    Network* pNetwork = static_cast<Network*>(args[0].addr);

    shared_ptr<MapMatchDataContainer> pContData =
                                GetMMDataFromTupleStream(args[1].addr, args, 3);

    CcReal *pNetworkScale = static_cast<CcReal*>(args[2].addr);

    // Matching

    NetworkAdapter Network(pNetwork, pNetworkScale->GetValue());
    MapMatchingMHT MapMatching(&Network, pContData);

    MGPointCreator Creator(&Network, pRes);

    if (!MapMatching.DoMatch(&Creator))
    {
        // Error
    }

    return 0;
}

/*
 3.4 Selection Function

*/
int MapMatchMHTSelect(ListExpr args)
{
    NList type(args);
    if ((type.length() == 2 || type.length() == 3) &&
        type.first().isSymbol(Network::BasicType()))
    {
        if (type.second().isSymbol(MPoint::BasicType()))
        {
            return 0;
        }
        else if (type.second().isSymbol(FText::BasicType()))
        {
            return 1;
        }
        else if (listutils::isTupleStream(type.second().listExpr()))
        {
            return 2;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}



/*
4 omapmatchmht-operator

4.1 Operator-Info
    map matching with an ordered relation
    Result: Tuples of matched edges with timestamps

*/
struct OMapMatchMHTInfo : OperatorInfo
{

    OMapMatchMHTInfo()
    {
        name      = "omapmatchmht";
        signature = OrderedRelation::BasicType() + " x " +
                    RTree2TID::BasicType() + " x " +
                    Relation::BasicType() + " x " +
                    MPoint::BasicType() + " -> " +
                    "stream(tuple([<attributes of tuple of edge>,"
                                 "StartTime:DateTime, EndTime:DateTime]))";

        appendSignature(OrderedRelation::BasicType() + " x " +
                        RTree2TID::BasicType() + " x " +
                        Relation::BasicType() + " x " +
                        FText::BasicType()  + " -> " +
                        "stream(tuple([<attributes of tuple of edge>,"
                                     "StartTime:DateTime, EndTime:DateTime]))");

        appendSignature(OrderedRelation::BasicType() + " x " +
                        RTree2TID::BasicType() + " x " +
                        Relation::BasicType() + " x " +
                        "(stream (tuple([Lat:real, Lon:real, Time:DateTime "
                                       "[,Fix:int] [,Sat:int] [,Hdop : real]"
                                       "[,Vdop:real] [,Pdop:real] "
                                       "[,Course:real] [,Speed(m/s):real]])))"
                        + " -> " +
                        "stream(tuple([<attributes of tuple of edge>,"
                                     "StartTime:DateTime, EndTime:DateTime]))");


        syntax    = "omapmatchmht ( _ , _ , _ , _ )";
        meaning   = "The operation maps the MPoint or "
                    "the data of a gpx-file or "
                    "the data of a tuple stream "
                    "to the given network, which is based on an "
                    "ordered relation."
                    "Result is a stream of tuples with the matched edges "
                    "of the network with timestamps.";
        example   = "omapmatchmht(Edges, EdgeIndex_Box_rtree, "
                                  "EdgeIndex, 'Trk_Dortmund.gpx')";
    }
};

static ListExpr GetORelNetworkAttrIndexes(ListExpr ORelAttrList);

/*
4.2 Type-Mapping

*/
enum EOMapMatchingMHTResultType
{
    OMM_RESULT_EDGES,
    OMM_RESULT_POSITIONS_ON_EDGES,
    OMM_RESULT_MPOINT
};

ListExpr OpOMapMatchingMHTTypeMap_Common(ListExpr in_xArgs,
                                         EOMapMatchingMHTResultType eResultType)
{
    NList param(in_xArgs);

    if( param.length() != 4)
        return listutils::typeError("four arguments expected");

    // Check Network - OrderedRelation, RTree, Relation

    NList ParamORel = param.first();
    NList ParamRTree = param.second();
    NList ParamRel = param.third();

    // Orel

    if (!listutils::isOrelDescription(ParamORel.listExpr()))
    {
        return listutils::typeError("1st argument must be orel");
    }

    NList orelTuple = ParamORel.second();
    if (!listutils::isTupleDescription(orelTuple.listExpr()))
    {
        return listutils::typeError("2nd value of orel is not of type tuple");
    }

    NList orelAttrList(orelTuple.second());
    if (!listutils::isAttrList(orelAttrList.listExpr()))
    {
        return listutils::typeError("Error in orel attrlist");
    }

    // Check attributes of ORel

    ListExpr IndNetwAttr = GetORelNetworkAttrIndexes(orelAttrList.listExpr());

    if (nl->Equal(IndNetwAttr, nl->TypeError()))
        return IndNetwAttr;

    // Rtree

    if (!listutils::isRTreeDescription(ParamRTree.listExpr()))
    {
        return listutils::typeError("2nd argument must be RTree2TID");
    }

    const int nRTreeDim = listutils::getRTreeDim(ParamRTree.listExpr());
    if (nRTreeDim != 2)
    {
        return listutils::typeError("rtree with dim==2 expected");
    }

    if (!ParamRTree.first().isSymbol(RTree2TID::BasicType()))
    {
        return listutils::typeError("2nd argument must be RTree2TID");
    }

    ListExpr rtreeKeyType = listutils::getRTreeType(ParamRTree.listExpr());

    if(!listutils::isSpatialType(rtreeKeyType) &&
       !listutils::isRectangle(rtreeKeyType))
    {
        return listutils::typeError("tree not over a spatial attribute");
    }

    // Relation

    if(!listutils::isRelDescription(ParamRel.listExpr()))
    {
        return listutils::typeError("3rd argument must be relation");
    }

    if(ParamRTree.second() != ParamRel.second())
    {
        return listutils::typeError("type of rtree and relation are different");
    }

    // GPS-Data (MPoint, FileName, TupleStream)

    if (!param.fourth().isSymbol(MPoint::BasicType()) &&
        !param.fourth().isSymbol(FText::BasicType()) &&
        !listutils::isTupleStream(param.fourth().listExpr()))
    {
        return listutils::typeError("4th argument must be " +
                                    MPoint::BasicType() + " or " +
                                    FText::BasicType() + " or " +
                                    "tuple stream");
    }

    // Result

    ListExpr ResultType = nl->TheEmptyList();

    switch (eResultType)
    {
        case OMM_RESULT_EDGES:
        {
            ListExpr addAttrs = nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("StartTime"),
                                        nl->SymbolAtom(DateTime::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("EndTime"),
                                        nl->SymbolAtom(DateTime::BasicType())));

            ListExpr ResAttr = AppendLists(orelAttrList.listExpr(), addAttrs);

            ResultType = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                         nl->TwoElemList(
                                             nl->SymbolAtom(Tuple::BasicType()),
                                             ResAttr));
        }
        break;

        case OMM_RESULT_POSITIONS_ON_EDGES:
        {
            ListExpr addAttrs = nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("StartTime"),
                                        nl->SymbolAtom(DateTime::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("EndTime"),
                                        nl->SymbolAtom(DateTime::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("StartPos"),
                                        nl->SymbolAtom(Point::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("EndPos"),
                                        nl->SymbolAtom(Point::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("StartLength"),
                                        nl->SymbolAtom(CcReal::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("EndLength"),
                                        nl->SymbolAtom(CcReal::BasicType())));

            ListExpr ResAttr = AppendLists(orelAttrList.listExpr(), addAttrs);

            ResultType = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                         nl->TwoElemList(
                                             nl->SymbolAtom(Tuple::BasicType()),
                                             ResAttr));
        }
        break;

        case OMM_RESULT_MPOINT:
        {
            ResultType = nl->SymbolAtom(MPoint::BasicType());
        }
        break;
    }


    if (listutils::isTupleStream(param.fourth().listExpr()))
    {
        ListExpr IndMMData =
                       GetMMDataIndexesOfTupleStream(param.fourth().listExpr());

        if (nl->Equal(IndMMData, nl->TypeError()))
            return IndMMData;
        else
        {
            return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                                     AppendLists(IndNetwAttr, IndMMData),
                                     ResultType);
        }
    }
    else
    {
        return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                                 IndNetwAttr,
                                 ResultType);
    }
}

ListExpr OpOMapMatchingMHTTypeMap(ListExpr in_xArgs)
{
    return OpOMapMatchingMHTTypeMap_Common(in_xArgs, OMM_RESULT_EDGES);
}

static ListExpr GetORelNetworkAttrIndexes(ListExpr ORelAttrList)
{
    ListExpr attrType;
    int nAttrSourceIndex = listutils::findAttribute(
                                              ORelAttrList, "Source", attrType);
    if (nAttrSourceIndex == 0)
    {
        return listutils::typeError("'Source' not found in attr list");
    }
    else
    {
        if (!listutils::isSymbol(attrType, CcInt::BasicType()))
        {
            return listutils::typeError(
                                      "'Source' must be " + CcInt::BasicType());
        }
    }

    int nAttrTargetIndex = listutils::findAttribute(
                                              ORelAttrList, "Target", attrType);
    if (nAttrTargetIndex == 0)
    {
        return listutils::typeError("'Target' not found in attr list");
    }
    else
    {
        if (!listutils::isSymbol(attrType, CcInt::BasicType()))
        {
            return listutils::typeError(
                                      "'Target' must be " + CcInt::BasicType());
        }
    }

    int nAttrSourcePosIndex = listutils::findAttribute(
                                           ORelAttrList, "SourcePos", attrType);
    if (nAttrSourcePosIndex == 0)
    {
        return listutils::typeError("'SourcePos' not found in attr list");
    }
    else
    {
        if (!listutils::isSymbol(attrType, Point::BasicType()))
        {
            return listutils::typeError(
                                   "'SourcePos' must be " + Point::BasicType());
        }
    }

    int nAttrTargetPosIndex = listutils::findAttribute(
                                           ORelAttrList, "TargetPos", attrType);
    if (nAttrTargetPosIndex == 0)
    {
        return listutils::typeError("'TargetPos' not found in attr list");
    }
    else
    {
        if (!listutils::isSymbol(attrType, Point::BasicType()))
        {
            return listutils::typeError(
                                   "'TargetPos' must be " + Point::BasicType());
        }
    }

    int nAttrCurveIndex = listutils::findAttribute(
                                               ORelAttrList, "Curve", attrType);
    if (nAttrCurveIndex == 0)
    {
        return listutils::typeError("'Curve' not found in attr list");
    }
    else
    {
        if (!listutils::isSymbol(attrType, SimpleLine::BasicType()))
        {
            return listutils::typeError(
                                  "'Curve' must be " + SimpleLine::BasicType());
        }
    }

    int nAttrRoadNameIndex = listutils::findAttribute(
                                            ORelAttrList, "RoadName", attrType);
    if (nAttrRoadNameIndex != 0)
    {
        if (!listutils::isSymbol(attrType, FText::BasicType()))
        {
            return listutils::typeError(
                                    "'RoadName' must be " + FText::BasicType());
        }
    }

    int nAttrRoadTypeIndex = listutils::findAttribute(
                                            ORelAttrList, "RoadType", attrType);
    if (nAttrRoadTypeIndex != 0)
    {
        if (!listutils::isSymbol(attrType, FText::BasicType()))
        {
            return listutils::typeError(
                                    "'RoadType' must be " + FText::BasicType());
        }
    }

    int nAttrMaxSpeedTypeIndex = listutils::findAttribute(
                                            ORelAttrList, "MaxSpeed", attrType);
    if (nAttrMaxSpeedTypeIndex != 0)
    {
        if (!listutils::isSymbol(attrType, FText::BasicType()))
        {
            return listutils::typeError(
                                    "'MaxSpeed' must be " + FText::BasicType());
        }
    }

    int nAttrWayIdTypeIndex = listutils::findAttribute(
                                               ORelAttrList, "WayId", attrType);
    if (nAttrWayIdTypeIndex == 0)
    {
        return listutils::typeError("'WayId' not found in attr list");
    }
    else
    {
        if (!listutils::isSymbol(attrType, CcInt::BasicType()))
        {
            return listutils::typeError(
                                       "'WayId' must be " + CcInt::BasicType());
        }
    }

    --nAttrSourceIndex;
    --nAttrTargetIndex;
    --nAttrSourcePosIndex;
    --nAttrTargetPosIndex;
    --nAttrCurveIndex;
    --nAttrRoadNameIndex;
    --nAttrRoadTypeIndex;
    --nAttrMaxSpeedTypeIndex;
    --nAttrWayIdTypeIndex;

    ListExpr Ind = nl->OneElemList(nl->IntAtom(nAttrSourceIndex));
    ListExpr Last = Ind;
    Last = nl->Append(Last, nl->IntAtom(nAttrTargetIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrSourcePosIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrTargetPosIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrCurveIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrRoadNameIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrRoadTypeIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrMaxSpeedTypeIndex));
    Last = nl->Append(Last, nl->IntAtom(nAttrWayIdTypeIndex));

    return Ind;
}

static ListExpr AppendLists(ListExpr List1, ListExpr List2)
{
    ListExpr ResultList = nl->TheEmptyList();
    ListExpr last = nl->TheEmptyList();

    ListExpr Lauf = List1;

    while (!nl->IsEmpty(Lauf))
    {
        ListExpr attr = nl->First(Lauf);

        if (nl->IsEmpty(ResultList))
        {
            ResultList = nl->OneElemList(attr);
            last = ResultList;
        }
        else
        {
            last = nl->Append(last, attr);
        }

        Lauf = nl->Rest(Lauf);
    }

    Lauf = List2;

    while (!nl->IsEmpty(Lauf))
    {
        ListExpr attr = nl->First(Lauf);

        if (nl->IsEmpty(ResultList))
        {
            ResultList = nl->OneElemList(attr);
            last = ResultList;
        }
        else
        {
            last = nl->Append(last, attr);
        }

        Lauf = nl->Rest(Lauf);
    }

    return ResultList;
}


/*
4.3 Value-Mapping

*/

static ONetwork::OEdgeAttrIndexes GetOEdgeAttrIndexes(Word* args, int nOffset)
{
    // s. GetORelNetworkAttrIndexes

    ONetwork::OEdgeAttrIndexes Indexes;

    const CcInt* pIdxSource    = static_cast<CcInt*>(args[nOffset + 0].addr);
    const CcInt* pIdxTarget    = static_cast<CcInt*>(args[nOffset + 1].addr);
    const CcInt* pIdxSourcePos = static_cast<CcInt*>(args[nOffset + 2].addr);
    const CcInt* pIdxTargetPos = static_cast<CcInt*>(args[nOffset + 3].addr);
    const CcInt* pIdxCurve     = static_cast<CcInt*>(args[nOffset + 4].addr);
    const CcInt* pIdxRoadName  = static_cast<CcInt*>(args[nOffset + 5].addr);
    const CcInt* pIdxRoadType  = static_cast<CcInt*>(args[nOffset + 6].addr);
    const CcInt* pIdxMaxSpeed  = static_cast<CcInt*>(args[nOffset + 7].addr);
    const CcInt* pIdxWayId     = static_cast<CcInt*>(args[nOffset + 8].addr);


    Indexes.m_IdxSource    = pIdxSource->GetValue();
    Indexes.m_IdxTarget    = pIdxTarget->GetValue();
    Indexes.m_IdxSourcePos = pIdxSourcePos->GetValue();
    Indexes.m_IdxTargetPos = pIdxTargetPos->GetValue();
    Indexes.m_IdxCurve     = pIdxCurve->GetValue();
    Indexes.m_IdxRoadName  = pIdxRoadName->GetValue();
    Indexes.m_IdxRoadType  = pIdxRoadType->GetValue();
    Indexes.m_IdxMaxSpeed  = pIdxMaxSpeed->GetValue();
    Indexes.m_IdxWayId     = pIdxWayId->GetValue();

    if (Indexes.m_IdxSource < 0 || Indexes.m_IdxTarget < 0 ||
        Indexes.m_IdxSourcePos < 0 || Indexes.m_IdxTargetPos < 0 ||
        Indexes.m_IdxCurve < 0 || Indexes.m_IdxWayId < 0)
    {
        assert(false);
    }

    return Indexes;
}

int OpOMapMatchingMHTMPoint2OStreamValueMapping(
                                           Word* args,
                                           Word& result,
                                           int message,
                                           Word& local,
                                           Supplier in_xSupplier,
                                           OEdgeTupleStreamCreator::EMode eMode)
{
    // cout << "OpOMapMatchingMHTMPoint2OStreamValueMapping called" << endl;

    OEdgeTupleStreamCreator* pCreator =
                              static_cast<OEdgeTupleStreamCreator*>(local.addr);
    switch (message)
    {
      case OPEN:
      {
          if (pCreator != NULL)
          {
              delete pCreator;
              pCreator = NULL;
          }

          // get Arguments
          OrderedRelation* pORel = static_cast<OrderedRelation*>(args[0].addr);
          RTree2TID* pRTree = static_cast<RTree2TID*>(args[1].addr);
          Relation* pRelation = static_cast<Relation*>(args[2].addr);

          ONetwork::OEdgeAttrIndexes Indexes = GetOEdgeAttrIndexes(args, 4);

          ONetwork Network(pORel, pRTree, pRelation, Indexes);

          MPoint* pMPoint = static_cast<MPoint*>(args[3].addr);

          // Do Map Matching

          ONetworkAdapter NetworkAdapter(&Network);
          MapMatchingMHT MapMatching(&NetworkAdapter, pMPoint);

          OEdgeTupleStreamCreator* pCreator =
                                     new OEdgeTupleStreamCreator(in_xSupplier,
                                                                 NetworkAdapter,
                                                                 eMode);

          if (!MapMatching.DoMatch(pCreator))
          {
              // Error
              delete pCreator;
              pCreator = NULL;
          }

          local.setAddr(pCreator);
          return 0;
      }
      case REQUEST:
      {
          if (pCreator == NULL)
          {
              return CANCEL;
          }
          else
          {
              result.addr = pCreator->GetNextTuple();
              return result.addr ? YIELD : CANCEL;
          }
      }
      case CLOSE:
      {
          if (pCreator)
          {
              delete pCreator;
              local.addr = NULL;
          }
          return 0;
      }
      default:
      {
          return 0;
      }
    }
}

int OpOMapMatchingMHTGPX2OStreamValueMapping(
                                           Word* args,
                                           Word& result,
                                           int message,
                                           Word& local,
                                           Supplier in_xSupplier,
                                           OEdgeTupleStreamCreator::EMode eMode)
{
    // cout << "OpOMapMatchingMHTGPX2OStreamValueMapping called" << endl;

    OEdgeTupleStreamCreator* pCreator =
                              static_cast<OEdgeTupleStreamCreator*>(local.addr);
    switch (message)
    {
      case OPEN:
      {
          if (pCreator != NULL)
          {
              delete pCreator;
              pCreator = NULL;
          }

          // get Arguments
          OrderedRelation* pORel = static_cast<OrderedRelation*>(args[0].addr);
          RTree2TID* pRTree = static_cast<RTree2TID*>(args[1].addr);
          Relation* pRelation = static_cast<Relation*>(args[2].addr);

          ONetwork::OEdgeAttrIndexes Indexes = GetOEdgeAttrIndexes(args, 4);

          ONetwork Network(pORel, pRTree, pRelation, Indexes);

          FText* pFileName = static_cast<FText*>(args[3].addr);
          std::string strFileName = pFileName->GetValue();

          // Do Map Matching

          ONetworkAdapter NetworkAdapter(&Network);
          MapMatchingMHT MapMatching(&NetworkAdapter, strFileName);

          OEdgeTupleStreamCreator* pCreator =
                                     new OEdgeTupleStreamCreator(in_xSupplier,
                                                                 NetworkAdapter,
                                                                 eMode);

          if (!MapMatching.DoMatch(pCreator))
          {
              // Error
              delete pCreator;
              pCreator = NULL;
          }

          local.setAddr(pCreator);
          return 0;
      }
      case REQUEST:
      {
          if (pCreator == NULL)
          {
              return CANCEL;
          }
          else
          {
              result.addr = pCreator->GetNextTuple();
              return result.addr ? YIELD : CANCEL;
          }
      }
      case CLOSE:
      {
          if (pCreator)
          {
              delete pCreator;
              local.addr = NULL;
          }
          return 0;
      }
      default:
      {
          return 0;
      }
    }
}

int OpOMapMatchingMHTStream2OStreamValueMapping(
                                           Word* args,
                                           Word& result,
                                           int message,
                                           Word& local,
                                           Supplier in_xSupplier,
                                           OEdgeTupleStreamCreator::EMode eMode)
{
    // cout << "OpOMapMatchingMHTStream2OStreamValueMapping called" << endl;

    OEdgeTupleStreamCreator* pCreator =
                              static_cast<OEdgeTupleStreamCreator*>(local.addr);
    switch (message)
    {
      case OPEN:
      {
          if (pCreator != NULL)
          {
              delete pCreator;
              pCreator = NULL;
          }

          // get Arguments
          OrderedRelation* pORel = static_cast<OrderedRelation*>(args[0].addr);
          RTree2TID* pRTree = static_cast<RTree2TID*>(args[1].addr);
          Relation* pRelation = static_cast<Relation*>(args[2].addr);

          ONetwork::OEdgeAttrIndexes Indexes = GetOEdgeAttrIndexes(args, 4);

          ONetwork Network(pORel, pRTree, pRelation, Indexes);

          shared_ptr<MapMatchDataContainer> pContData = // 9 OEdge-Attr-Indexes
                            GetMMDataFromTupleStream(args[3].addr, args, 4 + 9);

          // Do Map Matching

          ONetworkAdapter NetworkAdapter(&Network);
          MapMatchingMHT MapMatching(&NetworkAdapter, pContData);

          OEdgeTupleStreamCreator* pCreator =
                                     new OEdgeTupleStreamCreator(in_xSupplier,
                                                                 NetworkAdapter,
                                                                 eMode);

          if (!MapMatching.DoMatch(pCreator))
          {
              // Error
              delete pCreator;
              pCreator = NULL;
          }

          local.setAddr(pCreator);
          return 0;
      }
      case REQUEST:
      {
          if (pCreator == NULL)
          {
              return CANCEL;
          }
          else
          {
              result.addr = pCreator->GetNextTuple();
              return result.addr ? YIELD : CANCEL;
          }
      }
      case CLOSE:
      {
          if (pCreator)
          {
              delete pCreator;
              local.addr = NULL;
          }
          return 0;
      }
      default:
      {
          return 0;
      }
    }
}


int OpOMapMatchingMHTMPoint2EdgesValueMapping(Word* args,
                                              Word& result,
                                              int message,
                                              Word& local,
                                              Supplier in_xSupplier)
{
    return OpOMapMatchingMHTMPoint2OStreamValueMapping(
                                           args,
                                           result,
                                           message,
                                           local,
                                           in_xSupplier,
                                           OEdgeTupleStreamCreator::MODE_EDGES);
}

int OpOMapMatchingMHTGPX2EdgesValueMapping(Word* args,
                                           Word& result,
                                           int message,
                                           Word& local,
                                           Supplier in_xSupplier)
{
    return OpOMapMatchingMHTGPX2OStreamValueMapping(
                                           args,
                                           result,
                                           message,
                                           local,
                                           in_xSupplier,
                                           OEdgeTupleStreamCreator::MODE_EDGES);
}

int OpOMapMatchingMHTStream2EdgesValueMapping(Word* args,
                                              Word& result,
                                              int message,
                                              Word& local,
                                              Supplier in_xSupplier)
{
    return OpOMapMatchingMHTStream2OStreamValueMapping(
                                           args,
                                           result,
                                           message,
                                           local,
                                           in_xSupplier,
                                           OEdgeTupleStreamCreator::MODE_EDGES);
}


/*
4.4 Selection Function

*/
int OMapMatchMHTSelect(ListExpr args)
{
    NList type(args);
    if (type.length() == 4 &&
        listutils::isOrelDescription(type.first().listExpr()) &&
        listutils::isRTreeDescription(type.second().listExpr()) &&
        listutils::isRelDescription(type.third().listExpr()))
    {
        if (type.fourth().isSymbol(MPoint::BasicType()))
        {
            return 0;
        }
        else if (type.fourth().isSymbol(FText::BasicType()))
        {
            return 1;
        }
        else if (listutils::isTupleStream(type.fourth().listExpr()))
        {
            return 2;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}



/*
5 omapmatchmht\_P-operator

5.1 Operator-Info
    map matching with an ordered relation
    Result: Tuples of matched points on edges

*/
struct OMapMatchMHT_PInfo : OperatorInfo
{

    OMapMatchMHT_PInfo()
    {
        name      = "omapmatchmht_p";
        signature = OrderedRelation::BasicType() + " x " +
                    RTree2TID::BasicType() + " x " +
                    Relation::BasicType() + " x " +
                    MPoint::BasicType() + " -> " +
                    "stream(tuple([<attributes of tuple of edge>,"
                                 "StartTime:DateTime, EndTime:DateTime,"
                                 "StartPos:Point, EndPos:Point,"
                                 "StartLength:real, EndPos:real]))";

        appendSignature(OrderedRelation::BasicType() + " x " +
                        RTree2TID::BasicType() + " x " +
                        Relation::BasicType() + " x " +
                        FText::BasicType()  + " -> " +
                        "stream(tuple([<attributes of tuple of edge>,"
                                     "StartTime:DateTime, EndTime:DateTime,"
                                     "StartPos:Point, EndPos:Point,"
                                     "StartLength:real, EndPos:real]))");

        appendSignature(OrderedRelation::BasicType() + " x " +
                        RTree2TID::BasicType() + " x " +
                        Relation::BasicType() + " x " +
                        "(stream (tuple([Lat:real, Lon:real, Time:DateTime "
                                       "[,Fix:int] [,Sat:int] [,Hdop : real]"
                                       "[,Vdop:real] [,Pdop:real] "
                                       "[,Course:real] [,Speed(m/s):real]])))"
                        + " -> " +
                        "stream(tuple([<attributes of tuple of edge>,"
                                     "StartTime:DateTime, EndTime:DateTime,"
                                     "StartPos:Point, EndPos:Point,"
                                     "StartLength:real, EndPos:real]))");


        syntax    = "omapmatchmht_p ( _ , _ , _ , _ )";
        meaning   = "The operation maps the MPoint or "
                    "the data of a gpx-file or "
                    "the data of a tuple stream "
                    "to the given network, which is based on an "
                    "ordered relation."
                    "Result is a stream of tuples with the matched edges "
                    "of the network with timestamps, and matched positions.";
        example   = "omapmatchmht_p(Edges, EdgeIndex_Box_rtree, "
                                    "EdgeIndex, 'Trk_Dortmund.gpx')";
    }
};


/*
5.2 Type-Mapping

*/

ListExpr OpOMapMatchingMHT_PTypeMap(ListExpr in_xArgs)
{
    return OpOMapMatchingMHTTypeMap_Common(in_xArgs,
                                           OMM_RESULT_POSITIONS_ON_EDGES);
}


/*
5.3 Value-Mapping

*/

int OpOMapMatchingMHTMPoint2PositionsValueMapping(Word* args,
                                                  Word& result,
                                                  int message,
                                                  Word& local,
                                                  Supplier in_xSupplier)
{
    // cout << "OpOMapMatchingMHTMPoint2PositionsValueMapping called" << endl;

    return OpOMapMatchingMHTMPoint2OStreamValueMapping(
                             args,
                             result,
                             message,
                             local,
                             in_xSupplier,
                             OEdgeTupleStreamCreator::MODE_EDGES_AND_POSITIONS);
}

int OpOMapMatchingMHTGPX2PositionsValueMapping(Word* args,
                                               Word& result,
                                               int message,
                                               Word& local,
                                               Supplier in_xSupplier)
{
    // cout << "OpOMapMatchingMHTGPX2PositionsValueMapping called" << endl;

    return OpOMapMatchingMHTGPX2OStreamValueMapping(
                             args,
                             result,
                             message,
                             local,
                             in_xSupplier,
                             OEdgeTupleStreamCreator::MODE_EDGES_AND_POSITIONS);
}

int OpOMapMatchingMHTStream2PositionsValueMapping(Word* args,
                                                  Word& result,
                                                  int message,
                                                  Word& local,
                                                  Supplier in_xSupplier)
{
    // cout << "OpOMapMatchingMHTStream2PositionsValueMapping called" << endl;

    return OpOMapMatchingMHTStream2OStreamValueMapping(
                             args,
                             result,
                             message,
                             local,
                             in_xSupplier,
                             OEdgeTupleStreamCreator::MODE_EDGES_AND_POSITIONS);
}


/*
5.4 Selection Function

*/
int OMapMatchMHT_PSelect(ListExpr args)
{
    return OMapMatchMHTSelect(args);
}



/*
6 omapmatchmht\_mpoint-operator

6.1 Operator-Info
    map matching with an ordered relation
    result is a mpoint

*/
struct OMapMatchMHT_MPointInfo : OperatorInfo
{

    OMapMatchMHT_MPointInfo()
    {
        name      = "omapmatchmht_mpoint";
        signature = OrderedRelation::BasicType() + " x " +
                    RTree2TID::BasicType() + " x " +
                    Relation::BasicType() + " x " +
                    MPoint::BasicType() + " -> " +
                    MPoint::BasicType();

        appendSignature(OrderedRelation::BasicType() + " x " +
                        RTree2TID::BasicType() + " x " +
                        Relation::BasicType() + " x " +
                        FText::BasicType()  + " -> " +
                        MPoint::BasicType());

        appendSignature(OrderedRelation::BasicType() + " x " +
                        RTree2TID::BasicType() + " x " +
                        Relation::BasicType() + " x " +
                        "(stream (tuple([Lat:real, Lon:real, Time:DateTime "
                                       "[,Fix:int] [,Sat:int] [,Hdop : real]"
                                       "[,Vdop:real] [,Pdop:real] "
                                       "[,Course:real] [,Speed(m/s):real]])))"
                        + " -> " +
                        MPoint::BasicType());


        syntax    = "omapmatchmht_mpoint ( _ , _ , _ , _ )";
        meaning   = "The operation maps the MPoint or "
                    "the data of a gpx-file or "
                    "the data of a tuple stream "
                    "to the given network, which is based on an "
                    "ordered relation."
                    "Result is a MPoint.";
        example   = "omapmatchmht_mpoint(Edges, EdgeIndex_Box_rtree, "
                                         "EdgeIndex, 'Trk_Dortmund.gpx')";
    }
};


/*
6.2 Type-Mapping

*/

ListExpr OpOMapMatchingMHT_MPointTypeMap(ListExpr in_xArgs)
{
    return OpOMapMatchingMHTTypeMap_Common(in_xArgs, OMM_RESULT_MPOINT);
}


/*
6.3 Value-Mapping

*/

int OpOMapMatchingMHTMPoint2MPointValueMapping(Word* args,
                                               Word& result,
                                               int message,
                                               Word& local,
                                               Supplier in_xSupplier)
{
    // cout << "OpOMapMatchingMHTMPoint2MPointValueMapping called" << endl;

    // Initialize Result
    result = qp->ResultStorage(in_xSupplier);
    MPoint* pRes = static_cast<MPoint*>(result.addr);

    // get Arguments
    OrderedRelation* pORel = static_cast<OrderedRelation*>(args[0].addr);
    RTree2TID* pRTree = static_cast<RTree2TID*>(args[1].addr);
    Relation* pRelation = static_cast<Relation*>(args[2].addr);

    ONetwork::OEdgeAttrIndexes Indexes = GetOEdgeAttrIndexes(args, 4);

    ONetwork Network(pORel, pRTree, pRelation, Indexes);

    MPoint *pMPoint = static_cast<MPoint*>(args[3].addr);

    // Matching

    ONetworkAdapter NetworkAdapter(&Network);
    MapMatchingMHT MapMatching(&NetworkAdapter, pMPoint);

    MPointCreator Creator(pRes, NetworkAdapter.GetNetworkScale());

    if (!MapMatching.DoMatch(&Creator))
    {
        // Error
    }

    return 0;
}

int OpOMapMatchingMHTGPX2MPointValueMapping(Word* args,
                                            Word& result,
                                            int message,
                                            Word& local,
                                            Supplier in_xSupplier)
{
    // cout << "OpOMapMatchingMHTStream2MPointValueMapping called" << endl;

    // Initialize Result
    result = qp->ResultStorage(in_xSupplier);
    MPoint* pRes = static_cast<MPoint*>(result.addr);

    // get Arguments
    OrderedRelation* pORel = static_cast<OrderedRelation*>(args[0].addr);
    RTree2TID* pRTree = static_cast<RTree2TID*>(args[1].addr);
    Relation* pRelation = static_cast<Relation*>(args[2].addr);

    ONetwork::OEdgeAttrIndexes Indexes = GetOEdgeAttrIndexes(args, 4);

    ONetwork Network(pORel, pRTree, pRelation, Indexes);

    FText* pFileName = static_cast<FText*>(args[3].addr);
    std::string strFileName = pFileName->GetValue();

    // Matching

    ONetworkAdapter NetworkAdapter(&Network);
    MapMatchingMHT MapMatching(&NetworkAdapter, strFileName);

    MPointCreator Creator(pRes, NetworkAdapter.GetNetworkScale());

    if (!MapMatching.DoMatch(&Creator))
    {
        // Error
    }

    return 0;
}

int OpOMapMatchingMHTStream2MPointValueMapping(Word* args,
                                               Word& result,
                                               int message,
                                               Word& local,
                                               Supplier in_xSupplier)
{
    // cout << "OpOMapMatchingMHTStream2MPointValueMapping called" << endl;

    // Initialize Result
    result = qp->ResultStorage(in_xSupplier);
    MPoint* pRes = static_cast<MPoint*>(result.addr);

    // get Arguments
    OrderedRelation* pORel = static_cast<OrderedRelation*>(args[0].addr);
    RTree2TID* pRTree = static_cast<RTree2TID*>(args[1].addr);
    Relation* pRelation = static_cast<Relation*>(args[2].addr);

    ONetwork::OEdgeAttrIndexes Indexes = GetOEdgeAttrIndexes(args, 4);

    ONetwork Network(pORel, pRTree, pRelation, Indexes);

    shared_ptr<MapMatchDataContainer> pContData = // 9 OEdge-Attr-Indexes
                            GetMMDataFromTupleStream(args[3].addr, args, 4 + 9);

    // Matching

    ONetworkAdapter NetworkAdapter(&Network);
    MapMatchingMHT MapMatching(&NetworkAdapter, pContData);

    MPointCreator Creator(pRes, NetworkAdapter.GetNetworkScale());

    if (!MapMatching.DoMatch(&Creator))
    {
        // Error
    }

    return 0;
}


/*
6.4 Selection Function

*/
int OMapMatchMHT_MPointSelect(ListExpr args)
{
    return OMapMatchMHTSelect(args);
}



/*
7 gpximport-operator

7.1 Operator-Info

*/
struct GPXImportInfo : OperatorInfo
{
    GPXImportInfo()
    {
        name      = "gpximport";
        signature = FText::BasicType() + " -> " +
                    "stream(tuple([Time: DateTime, Lat: CcReal, Lon: CcReal, "
                                  "Fix: CcInt, Sat: CcInt, Hdop: CcReal, "
                                  "Vdop: CcReal, Pdop: CcReal, Course: CcReal, "
                                  "Speed: CcReal]))";

        appendSignature(FText::BasicType() +
                        CcReal::BasicType() + " -> " +
                        "stream(tuple([Time: DateTime, Lat: CcReal, "
                        "Lon: CcReal, Fix: CcInt, Sat: CcInt, Hdop: CcReal, "
                        "Vdop: CcReal, Pdop: CcReal, Course: CcReal, "
                        "Speed: CcReal]))");

        syntax    = "gpximport ( _ [, _ ] )";
        meaning   = "Imports the trackpoint data of a gpx file."
                    "With the optional parameter (CcReal) a scaling factor "
                    "for the coordinates (Lat, Lon) can be specified.";
        example   = "gpximport ('Trk_Dortmund.gpx')";
    }
};

/*
7.2 Type-Mapping

*/

ListExpr OpGPXImportTypeMap(ListExpr in_xArgs)
{
    NList param(in_xArgs);

    if( param.length() > 2)
        return listutils::typeError("one or two arguments expected");

    if (!param.first().isSymbol(FText::BasicType()))
        return listutils::typeError("1st argument must be " +
                                                            FText::BasicType());

    if (param.length() == 2 && !param.second().isSymbol(CcReal::BasicType()))
        return listutils::typeError("2nd argument must be " +
                                                           CcReal::BasicType());

    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                           GPXImporter::GetTupleTypeTrkPtListExpr());
}

/*
7.3 Value-Mapping

*/

int OpGPXImportValueMapping(Word* args,
                            Word& result,
                            int message,
                            Word& local,
                            Supplier in_xSupplier)
{
    GPXImporter* pImporter = static_cast<GPXImporter*>(local.addr);
    switch (message)
    {
      case OPEN:
      {
          FText* pFileName = static_cast<FText*>(args[0].addr);
          if (pImporter != NULL)
          {
              delete pImporter;
              pImporter = NULL;
          }

          std::string strFileName = pFileName->GetValue();

          pImporter = new GPXImporter(strFileName);

          local.setAddr(pImporter);
          return 0;
      }
      case REQUEST:
      {
          if (pImporter == NULL)
          {
              return CANCEL;
          }
          else
          {
              result.addr = pImporter->GetNextTrkPt();
              return result.addr ? YIELD : CANCEL;
          }
      }
      case CLOSE:
      {
          if (pImporter)
          {
              delete pImporter;
              local.addr = NULL;
          }
          return 0;
      }
      default:
      {
          return 0;
      }
    }
}

int OpGPXImportValueMappingWithScale(Word* args,
                                     Word& result,
                                     int message,
                                     Word& local,
                                     Supplier in_xSupplier)
{

    int nRet = OpGPXImportValueMapping(args, result, message,
                                       local, in_xSupplier);

    if (message == OPEN)
    {
        GPXImporter* pImporter = static_cast<GPXImporter*>(local.addr);
        if (pImporter != NULL)
        {
            CcReal* pScale = static_cast<CcReal*>(args[1].addr);
            pImporter->SetScaleFactor((pScale != NULL && pScale->IsDefined()) ?
                                                      pScale->GetValue() : 1.0);
        }
        else
        {
            assert(false);
        }
    }

    return nRet;
}

/*
7.4 Selection Function

*/

int GPXImportSelect(ListExpr args)
{
    NList type(args);
    if (type.length() == 1)
    {
        return 0;
    }
    else if (type.length() == 2)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}



/*
8 Implementation of the MapMatchingAlgebra Class

*/

MapMatchingAlgebra::MapMatchingAlgebra()
:Algebra()
{

/*
8.1 Registration of Types

*/


/*
8.2 Registration of Operators

*/

    // MapMatchMHT
    ValueMapping MapMatchMHTFuns[] = { OpMapMatchingMHTMPointValueMapping,
                                       OpMapMatchingMHTGPXValueMapping,
                                       OpMapMatchingMHTStreamValueMapping,
                                       0 };

    AddOperator(MapMatchMHTInfo(),
                MapMatchMHTFuns,
                MapMatchMHTSelect,
                OpMapMatchingMHTTypeMap);

    // OMapMatchMHT
    ValueMapping OMapMatchMHTFuns[] =
                                    { OpOMapMatchingMHTMPoint2EdgesValueMapping,
                                      OpOMapMatchingMHTGPX2EdgesValueMapping,
                                      OpOMapMatchingMHTStream2EdgesValueMapping,
                                      0 };

    AddOperator(OMapMatchMHTInfo(),
                OMapMatchMHTFuns,
                OMapMatchMHTSelect,
                OpOMapMatchingMHTTypeMap);

    // OMapMatchMHT_P
    ValueMapping OMapMatchMHT_PFuns[] =
                                { OpOMapMatchingMHTMPoint2PositionsValueMapping,
                                  OpOMapMatchingMHTGPX2PositionsValueMapping,
                                  OpOMapMatchingMHTStream2PositionsValueMapping,
                                  0 };

    AddOperator(OMapMatchMHT_PInfo(),
                OMapMatchMHT_PFuns,
                OMapMatchMHT_PSelect,
                OpOMapMatchingMHT_PTypeMap);

    // OMapMatchMHT_MPoint
    ValueMapping OMapMatchMHT_MPointFuns[] =
                                   { OpOMapMatchingMHTMPoint2MPointValueMapping,
                                     OpOMapMatchingMHTGPX2MPointValueMapping,
                                     OpOMapMatchingMHTStream2MPointValueMapping,
                                     0 };

    AddOperator(OMapMatchMHT_MPointInfo(),
                OMapMatchMHT_MPointFuns,
                OMapMatchMHT_MPointSelect,
                OpOMapMatchingMHT_MPointTypeMap);

    // GPXImport
    ValueMapping GPXImportFuns[] = { OpGPXImportValueMapping,
                                     OpGPXImportValueMappingWithScale,
                                     0 };

    AddOperator(GPXImportInfo(),
                GPXImportFuns,
                GPXImportSelect,
                OpGPXImportTypeMap);
}

MapMatchingAlgebra::~MapMatchingAlgebra()
{
}

} // end of namespace ~mapmatch~


/*
8 Initialization

*/

extern "C" Algebra* InitializeMapMatchingAlgebra(NestedList* /*nlRef*/,
                                                 QueryProcessor* /*qpRef*/)
{
  return new mapmatch::MapMatchingAlgebra;
}

