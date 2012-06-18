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

    void SetUTurn(double dAdditionalScore);

    void SetFailed(bool bFailed = true) {m_bFailed = bFailed;}
    bool GetFailed(void) const {return m_bFailed;}

    // A empty section is a section without assigned points
    inline unsigned short GetCountLastEmptySections(void) const
    {
        return m_nCountLastEmptySections;
    }

/*
3.1 class PointData
    Representation of matched data

*/
    class PointData
    {
    public:
        PointData();

        PointData(const MapMatchData* pMMData,
                  const Point& rPointProjection,
                  const shared_ptr<IMMNetworkSection>& pSection,
                  const double dDistance,
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
        inline double GetDistance(void) const {return m_dDistance;}
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
        double m_dDistance;
        double m_dScore;
    };

    typedef shared_ptr<const PointData> PointDataPtr;

    const PointDataPtr GetLastPoint(void) const;

    const std::vector<PointDataPtr>& GetPointsOfLastSection(void) const;

    // Return the number of (assigned) points to last section
    // (No "Offroad"-points !!)
    size_t GetCountPointsOfLastSection(void) const;

    inline size_t GetCountLastOffRoadPoints(void) const
                                             {return m_nCountLastOffRoadPoints;}

    bool CorrectUTurn(void);

/*
3.2 class RouteSegment
    Represents one route segment (Network-section, Offroad-section)

*/
    class RouteSegment
    {
    public:
        RouteSegment(void); // Off-road
        RouteSegment(const shared_ptr<IMMNetworkSection>& pSection); // On-road
        RouteSegment(const RouteSegment& rCandidate);
        ~RouteSegment();

        const shared_ptr<IMMNetworkSection>& GetSection(void) const
        {
            return m_pSection;
        }
        const std::vector<PointDataPtr>& GetPoints(void) const
        {
            return m_Points;
        }
        const PointDataPtr AddPoint(const MapMatchData* pMMData,
                const Point& rPointProjection, const double dDistance,
                const double dScore);
        const PointDataPtr AddPoint(const MapMatchData* pMMData,
                const double dScore);
        double RemoveLastPoint(void);

        bool IsOffRoad(void) const;

        void SetUTurn(bool bUTurn = true)
        {
            m_bUTurn = bUTurn;
        }
        bool HasUTurn(void) const
        {
            return m_bUTurn;
        }

        void Print(std::ostream& os) const;

    private:

        shared_ptr<IMMNetworkSection> m_pSection;
        std::vector<PointDataPtr> m_Points;
        bool m_bUTurn;

        // RefCounter for RouteCandidates
        void IncRef(void) {++m_nRefCount;}
        void DecRef(void) {assert(m_nRefCount > 0); --m_nRefCount;}
        int GetRefCount(void) {return m_nRefCount;}

        int m_nRefCount;
        friend class MHTRouteCandidate;
    };

    typedef shared_ptr<RouteSegment> RouteSegmentPtr;


/*
3.4 class RouteSegmentIterator
    Iterator for route segments

*/
    class RouteSegmentIterator
    {
    public:
        RouteSegmentIterator(const RouteSegmentIterator& rIt);
        ~RouteSegmentIterator();

        RouteSegmentIterator& operator=(const RouteSegmentIterator& rIt);

        bool operator==(const RouteSegmentIterator& rIt) const;
        bool operator!=(const RouteSegmentIterator& rIt) const;

        const RouteSegmentPtr operator*() const;

        RouteSegmentIterator& operator++();

    private:
        RouteSegmentIterator(const MHTRouteCandidate* pCandidate = NULL,
                             bool bBegin = true, bool bReverse = false);

        std::deque<RouteSegmentPtr>::const_iterator m_ItRouteSegment;
        std::deque<RouteSegmentPtr>::const_reverse_iterator m_ItRouteSegment_R;
        int m_nIdxContainer;
        bool m_bReverse;
        const MHTRouteCandidate* m_pRouteCandidate;

        friend class MHTRouteCandidate;
    };

    RouteSegmentIterator RouteSegmentBegin(void) const
    {
        return RouteSegmentIterator(this, true, false);
    }
    RouteSegmentIterator RouteSegmentEnd(void) const
    {
        return RouteSegmentIterator(this, false, false);
    }

    RouteSegmentIterator RouteSegmentRBegin(void) const
    {
        return RouteSegmentIterator(this, true, true);
    }
    RouteSegmentIterator RouteSegmentREnd(void) const
    {
        return RouteSegmentIterator(this, false, true);
    }

    size_t GetCountRouteSegments(void) const {return m_Segments.size();}


/*
3.3 class PointDataIterator
    Iterator for matched data

*/
    class PointDataIterator
    {
    public:
        PointDataIterator(const PointDataIterator& rIt);
        ~PointDataIterator();

        PointDataIterator& operator=(const PointDataIterator& rIt);

        bool operator==(const PointDataIterator& rIt) const;
        bool operator!=(const PointDataIterator& rIt) const;

        const PointDataPtr operator*() const;

        PointDataIterator& operator++();

    private:
        PointDataIterator(const MHTRouteCandidate* pCandidate,
                          bool bBegin, bool bReverse);

        RouteSegmentIterator m_ItRouteSegment;
        std::vector<PointDataPtr>::const_iterator m_ItPointData;
        RouteSegmentIterator m_ItRouteSegment_R;
        std::vector<PointDataPtr>::const_reverse_iterator m_ItPointData_R;
        bool m_bReverse;
        const MHTRouteCandidate* m_pRouteCandidate;

        friend class MHTRouteCandidate;
    };


    PointDataIterator PointDataBegin(void) const
    {
        return PointDataIterator(this, true, false);
    }
    PointDataIterator PointDataEnd(void) const
    {
        return PointDataIterator(this, false, false);
    }

    PointDataIterator PointDataRBegin(void) const
    {
        return PointDataIterator(this, true, true);
    }
    PointDataIterator PointDataREnd(void) const
    {
        return PointDataIterator(this, false, true);
    }


    // Debugging
    void Print(std::ostream& os) const;
    void PrintProjectedPoints(std::ostream& os) const;

private:

    MHTRouteCandidate():m_pMM(NULL) {}

    RouteSegmentPtr GetLastOnroadSegment(
                        std::deque<RouteSegmentPtr>::reverse_iterator* pItRet);

    void RemoveLastSection(void);

    std::deque<RouteSegmentPtr> m_Segments;

    typedef shared_ptr<std::deque<RouteSegmentPtr> > RouteSegmentContPtr;
    std::vector<RouteSegmentContPtr> m_SegmentsOutsourced;

    MapMatchingMHT* m_pMM;
    double m_dScore;
    unsigned short m_nCountLastEmptySections;
    size_t m_nCountLastOffRoadPoints;
    size_t m_nCountPoints;
    bool m_bFailed;
};


} // end of namespace mapmatch

#endif


