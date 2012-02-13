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

[1] Header File of the MapMatching Algebra

January-April, 2012. Matthias Roth

[TOC]

1 Overview

This header file contains the definition of the class ~MHTRouteCandidate~.

2 Defines and includes

*/
#ifndef __MHT_ROUTE_CANDIDATE__
#define __MHT_ROUTE_CANDIDATE__

#include <vector>
#include <deque>
#include "NetworkSection.h"
#include "DateTime.h"

class Point;
class GPoint;


namespace mapmatch {

/*
3 class MHTRouteCandidate
  Represents one route candidate for MHT-map matching

*/

class MHTRouteCandidate
{
public:
    MHTRouteCandidate();
    MHTRouteCandidate(const MHTRouteCandidate& rCandidate);
    ~MHTRouteCandidate();

    MHTRouteCandidate& operator=(const MHTRouteCandidate& rCandidate);

    bool operator==(const MHTRouteCandidate& rCandidate);

    void AddSection(const DirectedNetworkSection& rSection);
    size_t GetSectionCount(void) const;
    const DirectedNetworkSection& GetLastSection(void) const;

    void AddPoint(const GPoint& rGPoint, const Point& rPoint,
                  const Point& rPointProjection,
                  const double dDistance, const datetime::DateTime& rDateTime,
                  bool bClosed);

    void AddPoint(const Point& rPoint,
                  const double dDistance, const datetime::DateTime& rDateTime,
                  bool bClosed);

    void RemoveLastPoint(void);

    inline double GetScore(void) const
    {
        return m_dScore;
    }

    // A empty section is a section without assigned points
    inline unsigned short GetCountLastEmptySections(void) const
    {
        return m_nCountLastEmptySections;
    }

    void MarkAsInvalid(void);

    class PointData
    {
    public:
        PointData(const GPoint& rGPoint, const Point& rPointGPS,
                  const Point& rPointProjection,
                  const double dDistance,
                  const datetime::DateTime& rDateTime, bool bClosed);

        // Constructor without GPoint - Offroad-case
        PointData(const Point& rPoint, const double dDistance,
                  const datetime::DateTime& rDateTime, bool bClosed);

        PointData(const PointData& rPointData);

        bool operator==(const PointData& rPointData);

        ~PointData();

        inline GPoint* GetGPoint(void) const {return m_pGPoint;}
        inline Point* GetPointGPS(void) const {return m_pPointGPS;}
        inline double GetScore(void) const {return m_dScore;}
        inline datetime::DateTime GetTime(void) const {return m_Time;}
        inline bool GetClosed(void) const {return m_bClosed;}

    private:
        PointData& operator=(const PointData& rPointData);

        GPoint* m_pGPoint;
        Point* m_pPointGPS;
        //Point* m_pPointProjection;
        double m_dScore;
        datetime::DateTime m_Time;
        bool m_bClosed;
    };

    inline const std::vector<PointData*>& GetPoints(void) const
    {
        return m_Points;
    }

    void GetPointsOfLastSection(std::vector<PointData*>& rvecPoints) const;

    inline size_t GetCountPointsOfLastSection(void) const
                                                {return m_nPointsOfLastSection;}

    // Debugging
    void Print(std::ostream& os) const;
    void PrintGPoints(std::ostream& os) const;
    void PrintGPointsAsPoints(std::ostream& os) const;

private:

    std::vector<PointData*> m_Points; // TODO ggf. DBArray
    std::deque<DirectedNetworkSection> m_Sections;
    double m_dScore;
    size_t m_nPointsOfLastSection;
    size_t m_nCountSections;
    unsigned short m_nCountLastEmptySections;
    unsigned long m_nCountLastOffRoadPoints;
};


} // end of namespace mapmatch

#endif


