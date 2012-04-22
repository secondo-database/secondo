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
#include "NetworkRoute.h"
#include "MapMatchingData.h"
#include <DateTime.h>

#ifdef SECONDO_WIN32
#include <memory>
#else
#include <tr1/memory>
#endif
using std::tr1::shared_ptr;

class Point;
class HalfSegment;


namespace mapmatch {

class MapMatchingMHT;
class IMMNetworkSection;

/*
3 class MHTRouteCandidate
  Represents one route candidate for MHT-map matching

*/

class MHTRouteCandidate
{
public:
    MHTRouteCandidate(MapMatchingMHT* pMM);
    MHTRouteCandidate(const MHTRouteCandidate& rCandidate);
    ~MHTRouteCandidate();

    MHTRouteCandidate& operator=(const MHTRouteCandidate& rCandidate);

    void AddSection(const shared_ptr<IMMNetworkSection>& pSection);
    shared_ptr<IMMNetworkSection> GetLastSection(void) const;

    void AddPoint(const Point& rPointProjection,
                  const HalfSegment& rHSProjection,
                  const MapMatchData* pMMData,
                  double dDistance);

    void AddPoint(const MapMatchData* pMMData); // No projected point (offroad)

    void RemoveLastPoint(void);

    size_t GetCountPoints(void) const {return m_nCountPoints;}

    inline double GetScore(void) const
    {
        return m_dScore;
    }

    void AddScore(double dScore);

    void SetFailed(bool bFailed = true) {m_bFailed = bFailed;}
    bool GetFailed(void) const {return m_bFailed;}

    // A empty section is a section without assigned points
    inline unsigned short GetCountLastEmptySections(void) const
    {
        return m_nCountLastEmptySections;
    }

    class PointData
    {
    public:
        PointData();

        PointData(const MapMatchData* pMMData,
                  const Point& rPointProjection,
                  const shared_ptr<IMMNetworkSection>& pSection,
                  const double dScore);

        // Constructor without projected point - Offroad-case
        PointData(const MapMatchData* pMMData,
                  const double dScore);

        PointData(const PointData& rPointData);

        bool operator==(const PointData& rPointData) const;

        ~PointData();

        inline const Point GetPointGPS(void) const
                  {return m_pData != NULL ? m_pData->GetPoint() : Point(false);}
        inline const Point* GetPointProjection(void) const
                                              {return m_pPointProjection;}
        inline double GetScore(void) const {return m_dScore;}

        inline const MapMatchData* GetMMData(void) const {return m_pData;}

        const shared_ptr<IMMNetworkSection>& GetSection(void) const
                                                            {return m_pSection;}

        inline datetime::DateTime GetTime(void) const
                                   {return m_pData != NULL ?
                                           datetime::DateTime(m_pData->m_Time) :
                                           datetime::DateTime((int64_t)0);}

        void Print(std::ostream& os) const;

    private:
        PointData& operator=(const PointData& rPointData);

        const MapMatchData* m_pData;
        Point* m_pPointProjection;
        shared_ptr<IMMNetworkSection> m_pSection;
        double m_dScore;
    };

    const PointData* GetLastPoint(void) const;

    const std::vector<PointData*>& GetPointsOfLastSection(void) const;

    // Return the number of (assigned) points to last section
    // (No "Offroad"-points !!)
    size_t GetCountPointsOfLastSection(void) const;

    inline size_t GetCountLastOffRoadPoints(void) const
                                             {return m_nCountLastOffRoadPoints;}

    bool CorrectUTurn(void);

    class RouteSegment;

    class PointDataIterator
    {
    public:
        PointDataIterator(const PointDataIterator& rIt);
        ~PointDataIterator();

        PointDataIterator& operator=(const PointDataIterator& rIt);

        bool operator==(const PointDataIterator& rIt) const;
        bool operator!=(const PointDataIterator& rIt) const;

        const PointData* operator*() const;

        PointDataIterator& operator++();

    private:
        PointDataIterator(const MHTRouteCandidate* pCandidate,
                          bool bBegin, bool bReverse);

        std::vector<RouteSegment*>::const_iterator m_ItRouteSegment;
        std::vector<PointData*>::const_iterator m_ItPointData;
        std::vector<RouteSegment*>::const_reverse_iterator m_ItRouteSegment_R;
        std::vector<PointData*>::const_reverse_iterator m_ItPointData_R;
        bool m_bReverse;
        const MHTRouteCandidate* m_pRouteCandidate;

        friend class MHTRouteCandidate;
    };



    PointDataIterator PointDataBegin(void) const
                                  {return PointDataIterator(this, true, false);}
    PointDataIterator PointDataEnd(void) const
                                 {return PointDataIterator(this, false, false);}

    PointDataIterator PointDataRBegin(void) const
                                   {return PointDataIterator(this, true, true);}
    PointDataIterator PointDataREnd(void) const
                                  {return PointDataIterator(this, false, true);}

    class RouteSegment
    {
    public:
        RouteSegment(void); // Off-road
        RouteSegment(const shared_ptr<IMMNetworkSection>& pSection); // On-road
        RouteSegment(const RouteSegment& rCandidate);
        ~RouteSegment();

        const shared_ptr<IMMNetworkSection>& GetSection(void) const
                                                           {return m_pSection;}
        const std::vector<PointData*>& GetPoints(void) const {return m_Points;}
        PointData* AddPoint(const MapMatchData* pMMData,
                            const Point& rPointProjection,
                            const double dScore);
        PointData* AddPoint(const MapMatchData* pMMData,
                            const double dScore);
        double RemoveLastPoint(void);

        bool IsOffRoad(void) const;

        void Print(std::ostream& os) const;

    private:

        shared_ptr<IMMNetworkSection> m_pSection;
        std::vector<PointData*> m_Points;
    };

    const std::vector<RouteSegment*>& GetRouteSegments(void) const
                                                            {return m_Segments;}

    // Debugging
    void Print(std::ostream& os) const;
    void PrintProjectedPoints(std::ostream& os) const;

private:

    MHTRouteCandidate():m_pMM(NULL) {}

    void RemoveLastSection(void);

    std::vector<RouteSegment*> m_Segments;
    MapMatchingMHT* m_pMM;
    double m_dScore;
    unsigned short m_nCountLastEmptySections;
    size_t m_nCountLastOffRoadPoints;
    size_t m_nCountPoints;
    bool m_bFailed;
};


} // end of namespace mapmatch

#endif


