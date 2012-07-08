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

[1] Implementation of the GPXFileReader

January-April, 2012. Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of the class
~GPXFileReader~.
This class reads the data from a gpx-file.

2 Defines and includes

*/
#include "GPXFileReader.h"

#include "../TemporalNet/TemporalNetAlgebra.h"

#include <LogMsg.h>
#include <stdio.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>


namespace mapmatch {

/*
3 class GPXFileReader
  Reads the data from a gpx-file

3.1 Constructor / Destructor

*/

// Constructor
GPXFileReader::GPXFileReader()
:m_pXMLDoc(NULL)
{
}

// Destructor
GPXFileReader::~GPXFileReader()
{
    Free();
}

void GPXFileReader::Free(void)
{
    if (m_pXMLDoc != NULL)
        xmlFreeDoc(m_pXMLDoc);

    m_pXMLDoc = NULL;
}

void GPXFileReader::Init(void)
{
    Free();
}

/*
3.2 GPXFileReader::Open
    Opens the file and checks, if it is a valid GPX file

*/

bool GPXFileReader::Open(std::string strFileName)
{
    Init();

    assert(m_pXMLDoc == NULL);

    m_pXMLDoc = xmlParseFile(strFileName.c_str());
    if (m_pXMLDoc == NULL)
    {
        cmsg.error() << "Failed to read " << strFileName << endl;
        cmsg.send();
        return false;
    }

    xmlNodePtr pCurNode = xmlDocGetRootElement(m_pXMLDoc);
    if (pCurNode == NULL)
    {
        cmsg.error() << "Failed to read " << strFileName << endl;
        cmsg.send();
        xmlFreeDoc(m_pXMLDoc);
        m_pXMLDoc = NULL;
        return false;
    }

    if (xmlStrcmp(pCurNode->name, (const xmlChar *) "gpx") != 0)
    {
        cmsg.error() << "Not a gpx document " << strFileName << endl;
        cmsg.send();
        xmlFreeDoc(m_pXMLDoc);
        m_pXMLDoc = NULL;
        return false;
    }

    return true;
}

/*
3.3 GPXFileReader::DoRead
    Reading of GPX file

*/

/*bool GPXFileReader::DoRead(std::string strFileName, double dScale)
{
    if (!Open(strFileName))
        return false;

    m_dScale = dScale;

    xmlNodePtr pCurNode = GetFirstTrkNode();
    while (pCurNode != NULL)
    {
        ParseTrk(pCurNode);

        pCurNode = GetNextTrkNode(pCurNode);
    }

    Finalize();

    return true;
}*/

void GPXFileReader::ParseTrk(xmlNodePtr pNode)
{
    if (pNode == NULL)
        return;

    xmlNodePtr pCurChild = GetFirstTrkSegNode(pNode);
    while (pCurChild != NULL)
    {
        ParseTrkSeg(pCurChild);

        pCurChild = GetNextTrkSegNode(pCurChild);
    }
}

void GPXFileReader::ParseTrkSeg(xmlNodePtr pNode)
{
    if (pNode == NULL)
        return;

    xmlNodePtr pCurChild = GetFirstTrkPtNode(pNode);
    while (pCurChild != NULL)
    {
        SGPXTrkPointData Data;
        if (ParseTrkPt(pCurChild, Data))
            NewData(Data);

        pCurChild = GetNextTrkPtNode(pCurChild);
    }
}

xmlNodePtr GPXFileReader::GetFirstTrkNode(void)
{
    if (m_pXMLDoc == NULL)
        return NULL;

    xmlNodePtr pCurNode = xmlDocGetRootElement(m_pXMLDoc);

    if (pCurNode != NULL)
    {
        pCurNode = pCurNode->xmlChildrenNode;
        while (pCurNode != NULL)
        {
            if (xmlStrcmp(pCurNode->name, (const xmlChar *) "trk") == 0)
            {
                return pCurNode;
            }
            pCurNode = pCurNode->next;
        }
    }

    return NULL;
}

xmlNodePtr GPXFileReader::GetNextTrkNode(const xmlNodePtr pTrkNode)
{
    xmlNodePtr pNext = NULL;

    if (pTrkNode != NULL)
    {
        pNext = pTrkNode->next;

        while (pNext != NULL)
        {
            if (xmlStrcmp(pNext->name, (const xmlChar *) "trk") == 0)
                return pNext;

            pNext = pNext->next;
        }
    }

    return pNext;
}

xmlNodePtr GPXFileReader::GetFirstTrkSegNode(const xmlNodePtr pTrkNode)
{
    if (pTrkNode == NULL)
        return NULL;
    else
    {
        xmlNodePtr pCurChild = pTrkNode->xmlChildrenNode;
        while (pCurChild != NULL)
        {
            if (xmlStrcmp(pCurChild->name, (const xmlChar *) "trkseg") == 0)
            {
                return pCurChild;
            }
            pCurChild = pCurChild->next;
        }

        return NULL;
    }
}

xmlNodePtr GPXFileReader::GetNextTrkSegNode(const xmlNodePtr pTrkSegNode)
{
    xmlNodePtr pNext = NULL;

    if (pTrkSegNode != NULL)
    {
        pNext = pTrkSegNode->next;

        while (pNext != NULL)
        {
            if (xmlStrcmp(pNext->name, (const xmlChar *) "trkseg") == 0)
                return pNext;

            pNext = pNext->next;
        }
    }

    return pNext;
}

xmlNodePtr GPXFileReader::GetFirstTrkPtNode(const xmlNodePtr pTrkSegNode)
{
    if (pTrkSegNode == NULL)
        return NULL;
    else
    {
        xmlNodePtr pCurChild = pTrkSegNode->xmlChildrenNode;
        while (pCurChild != NULL)
        {
            if (xmlStrcmp(pCurChild->name, (const xmlChar *) "trkpt") == 0)
            {
                return pCurChild;
            }
            pCurChild = pCurChild->next;
        }

        return NULL;
    }
}

xmlNodePtr GPXFileReader::GetNextTrkPtNode(const xmlNodePtr pTrkPtNode)
{
    xmlNodePtr pNext = NULL;

    if (pTrkPtNode != NULL)
    {
        pNext = pTrkPtNode->next;

        while(pNext != NULL)
        {
            if (xmlStrcmp(pNext->name, (const xmlChar *) "trkpt") == 0)
                return pNext;

            pNext = pNext->next;
        }
    }

    return pNext;
}

static int convStrToInt (const char* pszStr)
{
    if (pszStr == NULL)
        return 0;

    return atoi(pszStr);
}

static double convStrToDouble (const char* pszStr)
{
    if (pszStr == NULL)
        return 0.0;

    return atof(pszStr);
}

bool GPXFileReader::ParseTrkPt(xmlNodePtr pNode,
                               SGPXTrkPointData& rTrkPointData)
{
    if (pNode == NULL)
        return false;

    SGPXTrkPointData TrkPointData;

    double dLat = 0.0;
    double dLon = 0.0;

    xmlChar* pLat = xmlGetProp(pNode, (const xmlChar *) "lat");
    if (pLat != NULL)
    {
        dLat = convStrToDouble((const char*)pLat);
        //dLat *= m_dScale;
        xmlFree(pLat);
    }
    else
    {
        return false;
    }

    xmlChar* pLon = xmlGetProp(pNode, (const xmlChar *) "lon");
    if (pLon != NULL)
    {
        dLon = convStrToDouble((const char*)pLon);
        //dLon *= m_dScale;
        xmlFree(pLon);
    }
    else
    {
        return false;
    }

    TrkPointData.m_Point.Set(dLon, dLat);

    xmlNodePtr pCurChild = pNode->xmlChildrenNode;
    while (pCurChild != NULL)
    {
        if (xmlStrcmp(pCurChild->name, (const xmlChar *) "time" ) == 0)
        {
            xmlChar* pTime = xmlNodeGetContent(pCurChild);
            if (pTime != NULL)
            {
                // YEAR-MONTH-DAYTHOUR:MIN:SECZ
                // replace T by -
                // remove Z

                std::string strTime((const char*)pTime);
                std::string::size_type nPos = strTime.find('T');
                if (nPos > 0)
                {
                    strTime.replace(nPos, 1, "-");
                }

                if (strTime.length() > 0 &&
                    strTime[strTime.length() - 1] == 'Z')
                {
                    strTime.erase(strTime.length() - 1, 1);
                }

                TrkPointData.m_Time.ReadFrom(strTime.c_str());
                xmlFree(pTime);
            }
        }
        else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "fix") == 0)
        {
            xmlChar* pFix = xmlNodeGetContent(pCurChild);

            if (xmlStrcmp(pFix, (const xmlChar *) "none") == 0)
            {
                TrkPointData.m_nFix.Set(0);
            }
            else if (xmlStrcmp(pFix, (const xmlChar *) "2d") == 0)
            {
                TrkPointData.m_nFix.Set(2);
            }
            else if (xmlStrcmp(pFix, (const xmlChar *) "3d") == 0)
            {
                TrkPointData.m_nFix.Set(3);
            }
            else if (xmlStrcmp(pFix, (const xmlChar *) "dgps") == 0)
            {
                TrkPointData.m_nFix.Set(4);
            }
            else if (xmlStrcmp(pFix, (const xmlChar *) "pps") == 0)
            {
                // military signal
                TrkPointData.m_nFix.Set(5);
            }

            xmlFree(pFix);
        }
        else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "sat") == 0)
        {
            xmlChar* pSat = xmlNodeGetContent(pCurChild);

            TrkPointData.m_nSat.Set(convStrToInt((const char*)pSat));

            xmlFree(pSat);
        }
        else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "hdop") == 0)
        {
            xmlChar* pHDOP = xmlNodeGetContent(pCurChild);

            TrkPointData.m_dHDOP.Set(convStrToDouble((const char*)pHDOP));

            xmlFree(pHDOP);
        }
        else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "vdop") == 0)
        {
            xmlChar* pVDOP = xmlNodeGetContent(pCurChild);

            TrkPointData.m_dVDOP.Set(convStrToDouble((const char*)pVDOP));

            xmlFree(pVDOP);
        }
        else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "pdop") == 0)
        {
            xmlChar* pPDOP = xmlNodeGetContent(pCurChild);

            TrkPointData.m_dPDOP.Set(convStrToDouble((const char*)pPDOP));

            xmlFree(pPDOP);
        }
        else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "course") == 0)
        {
            xmlChar* pCourse = xmlNodeGetContent(pCurChild);

            TrkPointData.m_dCourse.Set(convStrToDouble((const char*)pCourse));

            xmlFree(pCourse);
        }
        else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "speed") == 0)
        {
            xmlChar* pSpeed = xmlNodeGetContent(pCurChild);

            TrkPointData.m_dSpeed.Set(convStrToDouble((const char*)pSpeed));

            xmlFree(pSpeed);
        }
        else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "ele") == 0)
        {
            xmlChar* pElevation = xmlNodeGetContent(pCurChild);

            TrkPointData.m_dElevation.Set(
                                      convStrToDouble((const char*)pElevation));

            xmlFree(pElevation);
        }

        pCurChild = pCurChild->next;
    }

    rTrkPointData = TrkPointData;

    return true;
}

void GPXFileReader::NewData(const SGPXTrkPointData& /*rData*/)
{
#if 0
    if (m_pMPoint != NULL)
    {
        // Point
        UPoint ActUPoint(Interval<Instant>(rData.m_Time, rData.m_Time,
                                           true, true),
                         rData.m_Point, rData.m_Point);
        m_pMPoint->Add(ActUPoint);
    }

    if (m_pMFix != NULL)
    {
        // Fix
        UInt ActUFix(Interval<Instant>(rData.m_Time, rData.m_Time, true, true),
                     rData.m_nFix, rData.m_nFix);
        m_pMFix->Add(ActUFix);
    }

    if (m_pMSat != NULL)
    {
        // Sat
        UInt ActUSat(Interval<Instant>(rData.m_Time, rData.m_Time, true, true),
                     rData.m_nSat, rData.m_nSat);
        m_pMSat->Add(ActUSat);
    }

    if (m_pMHDOP != NULL)
    {
        // HDOP
        UReal ActUHDop(Interval<Instant>(rData.m_Time, rData.m_Time,
                                         true, true),
                       rData.m_dHDOP, rData.m_dHDOP);
        m_pMHDOP->Add(ActUHDop);
    }

    // VDOP
    if (m_pMVDOP != NULL)
    {
        UReal ActUVDop(Interval<Instant>(rData.m_Time, rData.m_Time,
                                         true, true),
                        rData.m_dVDOP, rData.m_dVDOP);
        m_pMVDOP->Add(ActUVDop);
    }

    // PDOP
    if (m_pMPDOP != NULL)
    {
        UReal ActUPDOP(Interval<Instant>(rData.m_Time, rData.m_Time,
                                         true, true),
                       rData.m_dPDOP, rData.m_dPDOP);
        m_pMPDOP->Add(ActUPDOP);
    }
#endif
}

TrkPointIteratorPtr GPXFileReader::GetTrkPointIterator(void)
{
    if (m_pXMLDoc == NULL)
        return TrkPointIteratorPtr();

    return TrkPointIteratorPtr(new CTrkPointIterator(this));
}


/*
3.4 struct GPXFileReader::SGPXTrkPointData

*/

GPXFileReader::SGPXTrkPointData::SGPXTrkPointData()
:m_Time(0.0), m_Point(false), m_nFix(false), m_nSat(false),
 m_dHDOP(false), m_dVDOP(false), m_dPDOP(false),
 m_dCourse(false), m_dSpeed(false), m_dElevation(false)
{
    m_Time.SetDefined(false);
}

GPXFileReader::SGPXTrkPointData::SGPXTrkPointData(const SGPXTrkPointData& rData)
:m_Time(rData.m_Time), m_Point(rData.m_Point), m_nFix(rData.m_nFix),
 m_nSat(rData.m_nSat), m_dHDOP(rData.m_dHDOP), m_dVDOP(rData.m_dVDOP),
 m_dPDOP(rData.m_dPDOP), m_dCourse(rData.m_dCourse), m_dSpeed(rData.m_dSpeed),
 m_dElevation(rData.m_dElevation)
{
}

GPXFileReader::SGPXTrkPointData& GPXFileReader::SGPXTrkPointData::operator=(
                                                  const SGPXTrkPointData& rData)
{
    if (&rData != this)
    {
        m_Time = rData.m_Time;
        m_Point = rData.m_Point;
        m_nFix = rData.m_nFix;
        m_nSat = rData.m_nSat;
        m_dHDOP = rData.m_dHDOP;
        m_dVDOP = rData.m_dVDOP;
        m_dPDOP = rData.m_dPDOP;
        m_dCourse = rData.m_dCourse;
        m_dSpeed = rData.m_dSpeed;
        m_dElevation = rData.m_dElevation;
    }

    return *this;
}



/*
4 class CTrkPointIterator
Iterator for Trk-points

*/

CTrkPointIterator::CTrkPointIterator()
:m_pReader(NULL),
 m_pCurrentTrk(NULL),
 m_pCurrentTrkSeg(NULL),
 m_pCurrentTrkPt(NULL)
{
}

CTrkPointIterator::CTrkPointIterator(GPXFileReader* pReader)
:m_pReader(pReader),
 m_pCurrentTrk(NULL),
 m_pCurrentTrkSeg(NULL),
 m_pCurrentTrkPt(NULL)
{
    if (m_pReader != NULL)
    {
        m_pCurrentTrk = m_pReader->GetFirstTrkNode();
        while (m_pCurrentTrk != NULL)
        {
            m_pCurrentTrkSeg = m_pReader->GetFirstTrkSegNode(m_pCurrentTrk);
            while (m_pCurrentTrkSeg != NULL)
            {
                m_pCurrentTrkPt =
                                 m_pReader->GetFirstTrkPtNode(m_pCurrentTrkSeg);
                if (m_pCurrentTrkPt != NULL)
                    return;

                m_pCurrentTrkSeg =
                                 m_pReader->GetNextTrkSegNode(m_pCurrentTrkSeg);
            }

            m_pCurrentTrk = m_pReader->GetNextTrkNode(m_pCurrentTrk);
        }
    }
}

CTrkPointIterator::~CTrkPointIterator()
{
    m_pReader = NULL;
    m_pCurrentTrk = NULL;
    m_pCurrentTrkSeg = NULL;
    m_pCurrentTrkPt = NULL;
}

bool CTrkPointIterator::GetCurrent(GPXFileReader::SGPXTrkPointData& rData)
{
    if (m_pReader != NULL && m_pCurrentTrkPt != NULL)
    {
        return m_pReader->ParseTrkPt(m_pCurrentTrkPt, rData);
    }
    else
    {
        return false;
    }
}

void CTrkPointIterator::Next(void)
{
    if (m_pReader != NULL)
    {
        m_pCurrentTrkPt = m_pReader->GetNextTrkPtNode(m_pCurrentTrkPt);

        while (m_pCurrentTrkPt == NULL && m_pCurrentTrk != NULL)
        {
            m_pCurrentTrkSeg = m_pReader->GetNextTrkSegNode(m_pCurrentTrkSeg);

            if (m_pCurrentTrkSeg != NULL)
            {
                m_pCurrentTrkPt =
                                 m_pReader->GetFirstTrkPtNode(m_pCurrentTrkSeg);
            }
            else
            {
                m_pCurrentTrk = m_pReader->GetNextTrkNode(m_pCurrentTrk);
                m_pCurrentTrkSeg = m_pReader->GetFirstTrkSegNode(m_pCurrentTrk);
                m_pCurrentTrkPt =
                                 m_pReader->GetFirstTrkPtNode(m_pCurrentTrkSeg);
            }
        }
    }
}


} // end of namespace mapmatch



