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

[1] Header File of the GPXImporter

January-April, 2012. Matthias Roth

[TOC]

1 Overview

This header file essentially contains the definition of the class ~GPXImporter~.

2 Defines and includes

*/
#ifndef __GPX_IMPORTER_H__
#define __GPX_IMPORTER_H__

#include <Point.h>
#include <DateTime.h>
#include <StandardTypes.h>

class TupleType;


namespace mapmatch {

/*
3 class GPXImporter
creates tuples of GPX-data

*/

class GPXImporter
{
public:

/*
3.1 Constructor and Destructor
    Reads a GPX file

*/
    GPXImporter(const std::string strFileName, double dScale = 1.0);
    ~GPXImporter();


/*
 3.1 Set scale-factor for coordinates (lat, lon)

*/
    void SetScaleFactor(double dScale);

/*
3.2 GetNextTrkPt
    creates a tuple of the next track point

*/
    Tuple* GetNextTrkPt(void);

/*
3.3 GetTupleTypeTrkPtListExpr
    List-expression of tuple type of track point-tuple

*/
    static ListExpr GetTupleTypeTrkPtListExpr(void);

private:
    class GPXFileReader* m_pReader;
    TupleType* m_pTupleTypeTrkPt;
    bool m_bOk;
    class CTrkPointIterator* m_pTrkPointIterator;
    double m_dScale; // Scale for coordinates (lat, lon)
};


} // end of namespace mapmatch

#endif /* __GPX_IMPORTER_H__ */

