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

This implementation file contains the implementation of the class ~GPXFileReader~.
This class reads the data from a gpx-file.

2 Defines and includes

*/
#include "GPXFileReader.h"

#include "../TemporalNet/TemporalNetAlgebra.h"

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
:m_dScale(1.0), m_pMPoint(NULL), m_pMFix(NULL), m_pMSat(NULL),
 m_pMHDOP(NULL), m_pMVDOP(NULL), m_pMPDOP(NULL)
{
}

// Destructor
GPXFileReader::~GPXFileReader()
{
    Free();
}

void GPXFileReader::Free(void)
{
    m_pMPoint.reset(NULL);
    m_pMFix.reset(NULL);
    m_pMSat.reset(NULL);
    m_pMHDOP.reset(NULL);
    m_pMVDOP.reset(NULL);
    m_pMPDOP.reset(NULL);
}

void GPXFileReader::Init(double dScale)
{
    m_dScale = dScale;

    Free();

    m_pMPoint = AttributePtr<MPoint>(new MPoint(100));
    m_pMFix = AttributePtr<MInt>(new MInt(100));
    m_pMSat = AttributePtr<MInt>(new MInt(100));
    m_pMHDOP = AttributePtr<MReal>(new MReal(100));
    m_pMVDOP = AttributePtr<MReal>(new MReal(100));
    m_pMPDOP = AttributePtr<MReal>(new MReal(100));

    m_pMPoint->StartBulkLoad();
    m_pMFix->StartBulkLoad();
    m_pMSat->StartBulkLoad();
    m_pMHDOP->StartBulkLoad();
    m_pMVDOP->StartBulkLoad();
    m_pMPDOP->StartBulkLoad();
}

void GPXFileReader::Finalize(void)
{
    m_pMPoint->EndBulkLoad(false, false);
    m_pMFix->EndBulkLoad(false, false);
    m_pMSat->EndBulkLoad(false, false);
    m_pMHDOP->EndBulkLoad(false, false);
    m_pMVDOP->EndBulkLoad(false, false);
    m_pMPDOP->EndBulkLoad(false, false);

    std::ofstream stream("/home/secondo/Traces/GPX.txt");

    m_pMPoint->Print(stream);
    stream << endl;
    m_pMFix->Print(stream);
    stream << endl;
    m_pMSat->Print(stream);
    stream << endl;
    m_pMHDOP->Print(stream);
    stream << endl;
    m_pMVDOP->Print(stream);
    stream << endl;
    m_pMPDOP->Print(stream);
    stream << endl;
}

/*
3.2 GPXFileReader::DoRead
    Reading of GPX file

*/

bool GPXFileReader::DoRead(std::string strFileName, double dScale)
{
    Init(dScale);

    xmlDocPtr pDoc = xmlParseFile(strFileName.c_str());
    if (pDoc == NULL)
    {
        cout << "Failed to read " << strFileName << endl;
        return false;
    }

    xmlNodePtr pCurNode = xmlDocGetRootElement(pDoc);
    if (pCurNode == NULL)
    {
        cout << "Failed to read " << strFileName << endl;
        xmlFreeDoc(pDoc);
        return false;
    }

    if (xmlStrcmp(pCurNode->name, (const xmlChar *) "gpx") != 0)
    {
        cout << "Not a gpx document " << strFileName << endl;
        xmlFreeDoc(pDoc);
        return false;
    }

    pCurNode = pCurNode->xmlChildrenNode;
    while (pCurNode != NULL)
    {
        if (xmlStrcmp(pCurNode->name, (const xmlChar *) "trk") == 0)
        {
            ParseTRK(pDoc, pCurNode);
        }
        pCurNode = pCurNode->next;
    }

    xmlFreeDoc(pDoc);

    Finalize();

    return true;
}

void GPXFileReader::ParseTRK(xmlDocPtr pDoc, xmlNodePtr pNode)
{
    if (pDoc == NULL || pNode == NULL)
        return;

    xmlNodePtr pCurChild = pNode->xmlChildrenNode;
    while (pCurChild != NULL)
    {
        if (xmlStrcmp(pCurChild->name, (const xmlChar *) "trkseg") == 0)
        {
            ParseTRKSEG(pDoc, pCurChild);
        }
        pCurChild = pCurChild->next;
    }
}

void GPXFileReader::ParseTRKSEG(xmlDocPtr pDoc, xmlNodePtr pNode)
{
    if (pDoc == NULL || pNode == NULL)
        return;

    xmlNodePtr pCurChild = pNode->xmlChildrenNode;
    while (pCurChild != NULL)
    {
        if (xmlStrcmp(pCurChild->name, (const xmlChar *) "trkpt") == 0)
        {
            ParseTRKPT(pDoc, pCurChild);
        }
        pCurChild = pCurChild->next;
    }
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

void GPXFileReader::ParseTRKPT(xmlDocPtr pDoc, xmlNodePtr pNode)
{
    if (pDoc == NULL || pNode == NULL)
        return;

    SGPXData Data;

    double dLat = 0.0;
    double dLon = 0.0;

    xmlChar* pLat = xmlGetProp(pNode, (const xmlChar *) "lat");
    if (pLat != NULL)
    {
        dLat = convStrToDouble((const char*)pLat);
        xmlFree(pLat);
    }

    xmlChar* pLon = xmlGetProp(pNode, (const xmlChar *) "lon");
    if (pLon != NULL)
    {
        dLon = convStrToDouble((const char*)pLon);
        xmlFree(pLat);
    }

    Data.m_Point.Set(dLon, dLat);

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

                Data.m_Time.ReadFrom(strTime.c_str());
                xmlFree(pTime);
            }
        }
        else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "fix") == 0)
        {
            xmlChar* pFix = xmlNodeGetContent(pCurChild);

            if (xmlStrcmp(pFix, (const xmlChar *) "none") == 0)
            {
                Data.m_nFix = 0;
            }
            else if (xmlStrcmp(pFix, (const xmlChar *) "2d") == 0)
            {
                Data.m_nFix = 2;
            }
            else if (xmlStrcmp(pFix, (const xmlChar *) "3d") == 0)
            {
                Data.m_nFix = 3;
            }
            else if (xmlStrcmp(pFix, (const xmlChar *) "dgps") == 0)
            {
                Data.m_nFix = 4;
            }
            else if (xmlStrcmp(pFix, (const xmlChar *) "pps") == 0)
            {
                // military signal
                Data.m_nFix = 5;
            }

            xmlFree(pFix);
        }
        else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "sat") == 0)
        {
            xmlChar* pSat = xmlNodeGetContent(pCurChild);

            Data.m_nSat = convStrToInt((const char*)pSat);

            xmlFree(pSat);
        }
        else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "hdop") == 0)
        {
            xmlChar* pHDOP = xmlNodeGetContent(pCurChild);

            Data.m_dHDOP = convStrToDouble((const char*)pHDOP);

            xmlFree(pHDOP);
        }
        else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "vdop") == 0)
        {
            xmlChar* pVDOP = xmlNodeGetContent(pCurChild);

            Data.m_dVDOP = convStrToDouble((const char*)pVDOP);

            xmlFree(pVDOP);
        }
        else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "pdop") == 0)
        {
            xmlChar* pPDOP = xmlNodeGetContent(pCurChild);

            Data.m_dPDOP = convStrToDouble((const char*)pPDOP);

            xmlFree(pPDOP);
        }
        /*else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "course") == 0)
        {
            xmlChar* pCourse = xmlNodeGetContent(pCurChild);

            Data.m_dCourse = convStrToDouble((const char*)pCourse);

            xmlFree(pCourse);
        }
        else if (xmlStrcmp(pCurChild->name, (const xmlChar *) "speed") == 0)
        {
            xmlChar* pSpeed = xmlNodeGetContent(pCurChild);

            Data.m_dSpeed = convStrToDouble((const char*)pSpeed);

            xmlFree(pSpeed);
        }*/

        pCurChild = pCurChild->next;
    }

    NewData(Data);
}

void GPXFileReader::NewData(const SGPXData& rData)
{
    if (m_pMPoint != NULL)
    {
        // Point
        Point pt(rData.m_Point);
        pt.Scale(m_dScale);

        UPoint ActUPoint(Interval<Instant>(rData.m_Time, rData.m_Time,
                                           true, true),
                         pt, pt);
        m_pMPoint->Add(ActUPoint);
    }

    if (m_pMFix != NULL)
    {
        // Fix
        UInt ActUFix(Interval<Instant>(rData.m_Time, rData.m_Time, true, true),
                     CcInt(true, rData.m_nFix), CcInt(true, rData.m_nFix));
        m_pMFix->Add(ActUFix);
    }

    if (m_pMSat != NULL)
    {
        // Sat
        UInt ActUSat(Interval<Instant>(rData.m_Time, rData.m_Time, true, true),
                     CcInt(true, rData.m_nSat), CcInt(true, rData.m_nSat));
        m_pMSat->Add(ActUSat);
    }

    if (m_pMHDOP != NULL)
    {
        // HDOP
        UReal ActUHDop(Interval<Instant>(rData.m_Time, rData.m_Time,
                                         true, true),
                       CcReal(true, rData.m_dHDOP),
                       CcReal(true, rData.m_dHDOP));
        m_pMHDOP->Add(ActUHDop);
    }

    // VDOP
    if (m_pMVDOP != NULL)
    {
        UReal ActUVDop(Interval<Instant>(rData.m_Time, rData.m_Time,
                                         true, true),
                        CcReal(true, rData.m_dVDOP),
                        CcReal(true, rData.m_dVDOP));
        m_pMVDOP->Add(ActUVDop);
    }

    // PDOP
    if (m_pMPDOP != NULL)
    {
        UReal ActUPDOP(Interval<Instant>(rData.m_Time, rData.m_Time,
                                         true, true),
                       CcReal(true, rData.m_dPDOP),
                       CcReal(true, rData.m_dPDOP));
        m_pMPDOP->Add(ActUPDOP);
    }
}


/*
4 struct GPXFileReader::SGPXData

*/

GPXFileReader::SGPXData::SGPXData()
:m_Time(0.0), m_Point(false), m_nFix(-1), m_nSat(-1),
 m_dHDOP(-1.0), m_dVDOP(-1.0), m_dPDOP(-1.0)
{
    m_Time.SetDefined(false);
}

GPXFileReader::SGPXData::SGPXData(const SGPXData& rData)
:m_Time(rData.m_Time), m_Point(rData.m_Point), m_nFix(rData.m_nFix),
 m_nSat(rData.m_nSat), m_dHDOP(rData.m_dHDOP), m_dVDOP(rData.m_dVDOP),
 m_dPDOP(rData.m_dPDOP)
{
}

GPXFileReader::SGPXData& GPXFileReader::SGPXData::operator=(
                                                          const SGPXData& rData)
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
    }

    return *this;
}


} // end of namespace mapmatch



