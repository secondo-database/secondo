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
3.2 Reads the file

*/
    bool DoRead(std::string strFileName,
                double dScale = 1.0 /* Scale for Lat/Lon */);

/*
3.3 Returns data

*/
    AttributePtr<MPoint> GetMPoint(void) const {return m_pMPoint;}
    AttributePtr<MInt> GetFix(void) const {return m_pMFix;}
    AttributePtr<MInt> GetSat(void) const {return m_pMSat;}
    AttributePtr<MReal> GetHDOP(void) const {return m_pMHDOP;}
    AttributePtr<MReal> GetVDOP(void) const {return m_pMVDOP;}
    AttributePtr<MReal> GetPDOP(void) const {return m_pMPDOP;}

private:

    struct SGPXData
    {
        SGPXData();
        SGPXData(const SGPXData& rData);
        SGPXData& operator=(const SGPXData& rData);

        datetime::DateTime m_Time;
        Point m_Point;
        int m_nFix;
        int m_nSat;
        double m_dHDOP;
        double m_dVDOP;
        double m_dPDOP;
    };

    void ParseTRK(xmlDocPtr pDoc, xmlNodePtr pNode);
    void ParseTRKSEG(xmlDocPtr pDoc, xmlNodePtr pNode);
    void ParseTRKPT(xmlDocPtr pDoc, xmlNodePtr pNode);
    void NewData(const SGPXData& rData);

    void Free(void);
    void Init(double dScale);
    void Finalize(void);


    double m_dScale; // Scale for Lat/Lon
    AttributePtr<MPoint> m_pMPoint;
    AttributePtr<MInt> m_pMFix;
    AttributePtr<MInt> m_pMSat;
    AttributePtr<MReal> m_pMHDOP;
    AttributePtr<MReal> m_pMVDOP;
    AttributePtr<MReal> m_pMPDOP;
};

} // end of namespace mapmatch

#endif /* __GPX_FILE_READER_H__ */
