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

[1] Header File of the GPXFileReader

January-April, 2012. Matthias Roth

[TOC]

1 Overview

This header file essentially contains the definition of the class ~GPXFileReader~.

2 Defines and includes

*/
#ifndef __GPX_FILE_READER_H__
#define __GPX_FILE_READER_H__

#include "MapMatchingUtil.h"

class MPoint;
class MInt;
class MReal;
#include <Point.h>
#include <DateTime.h>
#include <StandardTypes.h>
#include <TemporalAlgebra.h>

typedef struct _xmlDoc xmlDoc;
typedef xmlDoc *xmlDocPtr;
typedef struct _xmlNode xmlNode;
typedef xmlNode *xmlNodePtr;


namespace mapmatch {

/*
3 class GPXFileReader
Reads the data from a gpx-file

*/

class GPXFileReader
{
public:

/*
3.1 Constructors and Destructor

*/
    GPXFileReader();
    ~GPXFileReader();

/*
3.2 Opens the file and checks the format

*/
    bool Open(std::string strFileName);

/*
3.3 SGPXTrkPointData

*/
    struct SGPXTrkPointData
    {
        SGPXTrkPointData();
        SGPXTrkPointData(const SGPXTrkPointData& rData);
        SGPXTrkPointData& operator=(const SGPXTrkPointData& rData);

        datetime::DateTime m_Time;
        Point m_Point;
        CcInt m_nFix;
        CcInt m_nSat;
        CcReal m_dHDOP;
        CcReal m_dVDOP;
        CcReal m_dPDOP;
        CcReal m_dCourse;
        CcReal m_dSpeed;
        CcReal m_dElevation;
    };


/*
3.4 Iterator for Trackpoints
    The file must be opened before - GPXFileReader::Open(std::string)

*/
    class CTrkPointIterator* GetTrkPointIterator(void);

    void FreeTrkPointIterator(class CTrkPointIterator*& rpIterator);

private:

    void ParseTrk(xmlNodePtr pNode);
    void ParseTrkSeg(xmlNodePtr pNode);
    bool ParseTrkPt(xmlNodePtr pNode, SGPXTrkPointData& rData);
    void NewData(const SGPXTrkPointData& rData);

    xmlNodePtr GetFirstTrkNode(void);
    xmlNodePtr GetNextTrkNode(const xmlNodePtr pTrkNode);

    xmlNodePtr GetFirstTrkSegNode(const xmlNodePtr pTrkNode);
    xmlNodePtr GetNextTrkSegNode(const xmlNodePtr pTrkSegNode);

    xmlNodePtr GetFirstTrkPtNode(const xmlNodePtr pTrkSegNode);
    xmlNodePtr GetNextTrkPtNode(const xmlNodePtr pTrkPtNode);

    void Free(void);
    void Init(void);

    xmlDocPtr m_pXMLDoc;

    friend class CTrkPointIterator;
};



/*
4 class CTrkPointIterator
Iterator for Trk-points

*/

class CTrkPointIterator
{
public:
    bool GetCurrent(GPXFileReader::SGPXTrkPointData& rData);
    void Next(void);

private:
    CTrkPointIterator(GPXFileReader* pReader);

    ~CTrkPointIterator();

    GPXFileReader* m_pReader;
    xmlNodePtr m_pCurrentTrk;
    xmlNodePtr m_pCurrentTrkSeg;
    xmlNodePtr m_pCurrentTrkPt;

    friend class GPXFileReader;
};



} // end of namespace mapmatch

#endif /* __GPX_FILE_READER_H__ */
