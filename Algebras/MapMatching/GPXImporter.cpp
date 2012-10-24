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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Implemantation of the GPXImporter

January-April, 2012. Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of the class ~GPXImporter~.

2 Defines and includes

*/

#include "GPXImporter.h"
#include "GPXFileReader.h"

#include <RelationAlgebra.h>


namespace mapmatch {

/*
3 class GPXImporter
creates tuple streams of GPX-data

3.1 Constructor / Destructor

*/

GPXImporter::GPXImporter(const std::string strFileName,
                         double dScale)
:m_pReader(new GPXFileReader), m_pTupleTypeTrkPt(NULL),
 m_bOk(false), m_pTrkPointIterator(), m_dScale(dScale)
{
    m_bOk = m_pReader->Open(strFileName);

    if (m_bOk)
        m_pTrkPointIterator = m_pReader->GetTrkPointIterator();

    ListExpr typeInfo = SecondoSystem::GetCatalog()->
                                       NumericType(GetTupleTypeTrkPtListExpr());
    m_pTupleTypeTrkPt = new TupleType(typeInfo);
}

GPXImporter::~GPXImporter()
{
    delete m_pReader;
    m_pReader = NULL;
    m_pTupleTypeTrkPt->DeleteIfAllowed();
}

void GPXImporter::SetScaleFactor(double dScale)
{
    m_dScale = dScale;
}

ListExpr GPXImporter::GetTupleTypeTrkPtListExpr(void)
{
    using datetime::DateTime;
    ListExpr attrList = nl->OneElemList(nl->TwoElemList(
                                        nl->SymbolAtom("Time"),
                                        nl->SymbolAtom(DateTime::BasicType())));

    ListExpr ActNode = attrList;
    ActNode = nl->Append(ActNode, nl->TwoElemList(
                                        nl->SymbolAtom("Lat"),
                                        nl->SymbolAtom(CcReal::BasicType())));

    ActNode = nl->Append(ActNode, nl->TwoElemList(
                                        nl->SymbolAtom("Lon"),
                                        nl->SymbolAtom(CcReal::BasicType())));

    ActNode = nl->Append(ActNode, nl->TwoElemList(
                                        nl->SymbolAtom("Fix"),
                                        nl->SymbolAtom(CcInt::BasicType())));

    ActNode = nl->Append(ActNode, nl->TwoElemList(
                                        nl->SymbolAtom("Sat"),
                                        nl->SymbolAtom(CcInt::BasicType())));

    ActNode = nl->Append(ActNode, nl->TwoElemList(
                                        nl->SymbolAtom("Hdop"),
                                        nl->SymbolAtom(CcReal::BasicType())));

    ActNode = nl->Append(ActNode, nl->TwoElemList(
                                        nl->SymbolAtom("Vdop"),
                                        nl->SymbolAtom(CcReal::BasicType())));

    ActNode = nl->Append(ActNode, nl->TwoElemList(
                                        nl->SymbolAtom("Pdop"),
                                        nl->SymbolAtom(CcReal::BasicType())));

    ActNode = nl->Append(ActNode, nl->TwoElemList(
                                        nl->SymbolAtom("Course"),
                                        nl->SymbolAtom(CcReal::BasicType())));

    ActNode = nl->Append(ActNode, nl->TwoElemList(
                                        nl->SymbolAtom("Speed"),
                                        nl->SymbolAtom(CcReal::BasicType())));

    ActNode = nl->Append(ActNode, nl->TwoElemList(
                                        nl->SymbolAtom("Ele"),
                                        nl->SymbolAtom(CcReal::BasicType())));

    return nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), attrList);
}

Tuple* GPXImporter::GetNextTrkPt(void)
{
    if (!m_bOk || m_pTrkPointIterator == NULL)
        return NULL;

    GPXFileReader::SGPXTrkPointData Data;

    if (m_pTrkPointIterator->GetCurrent(Data))
    {
        Tuple* pTuple = new Tuple(m_pTupleTypeTrkPt);

        const double dLat = Data.m_Point.GetY() * m_dScale;
        const double dLon = Data.m_Point.GetX() * m_dScale;

        using datetime::DateTime;
        pTuple->PutAttribute(0, new DateTime(Data.m_Time));       // Time
        pTuple->PutAttribute(1, new CcReal(dLat));                // Lat
        pTuple->PutAttribute(2, new CcReal(dLon));                // Lon
        pTuple->PutAttribute(3, new CcInt(Data.m_nFix));          // Fix
        pTuple->PutAttribute(4, new CcInt(Data.m_nSat));          // Sat
        pTuple->PutAttribute(5, new CcReal(Data.m_dHDOP));        // Hdop
        pTuple->PutAttribute(6, new CcReal(Data.m_dVDOP));        // Vdop
        pTuple->PutAttribute(7, new CcReal(Data.m_dPDOP));        // Pdop
        pTuple->PutAttribute(8, new CcReal(Data.m_dCourse));      // Course
        pTuple->PutAttribute(9, new CcReal(Data.m_dSpeed));       // Speed
        pTuple->PutAttribute(10, new CcReal(Data.m_dElevation));  // Ele

        m_pTrkPointIterator->Next();

        return pTuple;
    }
    else
    {
        return NULL;
    }
}

} // end of namespace mapmatch


