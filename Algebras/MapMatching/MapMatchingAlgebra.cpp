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

This implementation file contains the implementation of the class ~MapMatchingAlgebra~.

For more detailed information see MapMatchingAlgebra.h.

2 Defines and Includes

*/

#include "MapMatchingAlgebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"

#include "NetworkAlgebra.h"
#include "TemporalNetAlgebra.h"
#include "FTextAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;

#include "TypeMapUtils.h"
#include "Symbols.h"

#include <string>
using namespace std;

#include "MapMatchingSimple.h"
#include "MapMatchingMHT.h"
#include "GPXImporter.h"


namespace mapmatch {

/*
3 mapmatchsimple-operator

3.1 Operator-Info

*/
struct MapMatchSimpleInfo : OperatorInfo
{
    MapMatchSimpleInfo()
    {
        name      = "mapmatchsimple";
        signature = Network::BasicType() + " x " +
                    MPoint::BasicType() + " -> " +
                    MGPoint::BasicType();
        syntax    = "mapmatchsimple ( _ , _ )";
        meaning   = "The operation tries to map the mpoint to "
                    "the given network as well as possible.";
        example   = "mapmatchsimple (TODO, TODO)";
    }
};

/*
3.2 Type-Mapping

*/
ListExpr OpMapMatchingTypeMap(ListExpr in_xArgs)
{
    NList param(in_xArgs);

    if( param.length() != 2)
        return listutils::typeError("two arguments expected");

    if (!param.first().isSymbol(Network::BasicType()))
        return listutils::typeError("1st argument must be " +
                                                          Network::BasicType());

     if (!param.second().isSymbol(MPoint::BasicType()))
         return listutils::typeError("2nd argument must be " +
                                                           MPoint::BasicType());

    return nl->SymbolAtom(MGPoint::BasicType());
}

/*
3.3 Value-Mapping

*/
int OpMapMatchingSimpleValueMapping(Word* args,
                                    Word& result,
                                    int message,
                                    Word& local,
                                    Supplier in_xSupplier)
{
    // cout << "OpMapMatching called" << endl;

    // Initialize Result
    result = qp->ResultStorage(in_xSupplier);
    MGPoint* res = static_cast<MGPoint*>(result.addr);

    // get Arguments
    Network *pNetwork = static_cast<Network*>(args[0].addr);
    MPoint *pMPoint = static_cast<MPoint*>(args[1].addr);

    // Do Map Matching

    MapMatchingSimple MapMatching(pNetwork, pMPoint);

    if (!MapMatching.DoMatch(res))
    {
        // Error
    }

    return 0;
}


/*
4 mapmatchmht-operator

4.1 Operator-Info

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

        syntax    = "mapmatchmht ( _ , _ )";
        meaning   = "The operation tries to map the MPoint or "
                    "the data from a gpx-file or "
                    "the data of a tuple stream "
                    "to the given network as well as possible.";
        example   = "mapmatchmht (DortmundNet, 'Trk_Dortmund.gpx')";
    }
};

/*
4.2 Type-Mapping

*/

ListExpr OpMapMatchingMHTTypeMap(ListExpr in_xArgs)
{
    NList param(in_xArgs);

    if( param.length() != 2)
        return listutils::typeError("two arguments expected");

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

    if (listutils::isTupleStream(param.second().listExpr()))
    {
        ListExpr TupleStream = param.second().listExpr();
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
            if (!listutils::isSymbol(attrType, datetime::DateTime::BasicType()))
            {
                return listutils::typeError("'Time' must be " +
                                            datetime::DateTime::BasicType());
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

        return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                                 Ind,
                                 nl->SymbolAtom(MGPoint::BasicType()));
    }
    else
    {
        return nl->SymbolAtom(MGPoint::BasicType());
    }
}

/*
4.3 Value-Mapping

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
    MGPoint* res = static_cast<MGPoint*>(result.addr);

    // get Arguments
    Network *pNetwork = static_cast<Network*>(args[0].addr);
    MPoint *pMPoint = static_cast<MPoint*>(args[1].addr);

    // Do Map Matching

    MapMatchingMHT MapMatching(pNetwork, pMPoint);

    if (!MapMatching.DoMatch(res))
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
    MGPoint* res = static_cast<MGPoint*>(result.addr);

    // get Arguments
    Network* pNetwork = static_cast<Network*>(args[0].addr);
    FText* pFileName = static_cast<FText*>(args[1].addr);

    std::string strFileName = pFileName->Get();

    // Do Map Matching

    MapMatchingMHT MapMatching(pNetwork, strFileName);

    if (!MapMatching.DoMatch(res))
    {
        // Error
    }

    return 0;
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
    MGPoint* res = static_cast<MGPoint*>(result.addr);

    // get Arguments
    Network* pNetwork = static_cast<Network*>(args[0].addr);

    const CcInt* pIdxLat    = static_cast<CcInt*>(args[2].addr);
    const CcInt* pIdxLon    = static_cast<CcInt*>(args[3].addr);
    const CcInt* pIdxTime   = static_cast<CcInt*>(args[4].addr);
    const CcInt* pIdxFix    = static_cast<CcInt*>(args[5].addr);
    const CcInt* pIdxSat    = static_cast<CcInt*>(args[6].addr);
    const CcInt* pIdxHdop   = static_cast<CcInt*>(args[7].addr);
    const CcInt* pIdxVdop   = static_cast<CcInt*>(args[8].addr);
    const CcInt* pIdxPdop   = static_cast<CcInt*>(args[9].addr);
    const CcInt* pIdxCourse = static_cast<CcInt*>(args[10].addr);
    const CcInt* pIdxSpeed  = static_cast<CcInt*>(args[11].addr);
    //const CcInt* pIdxEle    = static_cast<CcInt*>(args[12].addr);

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
        return 0;
    }

    DbArrayPtr<DbArray<MapMatchingMHT::MapMatchData> >
                         pArrData(new DbArray<MapMatchingMHT::MapMatchData>(0));

    Word wTuple;
    qp->Open(args[1].addr);
    qp->Request(args[1].addr, wTuple);
    while (qp->Received(args[1].addr))
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
            MapMatchingMHT::MapMatchData Data(pLat->GetValue(),
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

            pArrData->Append(Data);
        }

        pTpl->DeleteIfAllowed();
        pTpl = NULL;

        qp->Request(args[1].addr, wTuple);
    }
    qp->Close(args[1].addr);


    // Matching

    MapMatchingMHT MapMatching(pNetwork, pArrData);

    if (!MapMatching.DoMatch(res))
    {
        // Error
    }

    return 0;
}

/*
 4.4 Selection Function

*/

int MapMatchMHTSelect(ListExpr args)
{
    NList type(args);
    if (type.length() == 2 &&
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
5 gpximport-operator

5.1 Operator-Info

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
        example   = "gpximport ('gpx-filename')";
    }
};

/*
5.2 Type-Mapping

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
5.3 Value-Mapping

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

          std::string strFileName = pFileName->Get();

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
 5.4 Selection Function

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
6 Implementation of the MapMatchingAlgebra Class

*/

MapMatchingAlgebra::MapMatchingAlgebra()
:Algebra()
{

/*
6.1 Registration of Types

*/


/*
6.2 Registration of Operators

*/

    // MapMatchSimple
    /* only for testing purposes
     * AddOperator(MapMatchSimpleInfo(),
                OpMapMatchingSimpleValueMapping,
                OpMapMatchingTypeMap);*/

    // MapMatchMHT - overloaded
    ValueMapping MapMatchMHTFuns[] = { OpMapMatchingMHTMPointValueMapping,
                                       OpMapMatchingMHTGPXValueMapping,
                                       OpMapMatchingMHTStreamValueMapping,
                                       0 };

    AddOperator(MapMatchMHTInfo(),
                MapMatchMHTFuns,
                MapMatchMHTSelect,
                OpMapMatchingMHTTypeMap);


    // GPXImport - overloaded
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
7 Initialization

*/

extern "C" Algebra* InitializeMapMatchingAlgebra(NestedList* /*nlRef*/,
                                                 QueryProcessor* /*qpRef*/)
{
  return new mapmatch::MapMatchingAlgebra;
}

