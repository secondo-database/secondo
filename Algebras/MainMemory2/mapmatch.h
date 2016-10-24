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

*/

#ifndef MAPMATCHING_H
#define MAPMATCHING_H

#include <vector>
#include <set>

#include "LongInt.h"
#include "DateTime.h"
#include "NetworkAlgebra.h"
#include "TemporalNetAlgebra.h"
#include "ONetwork.h"
#include "ONetworkAdapter.h"
#include "ONetworkEdge.h"
#include "MapMatchingMHT.h"
#include "MapMatchingData.h"
#include "GPXImporter.h"
#include "NetworkAdapter.h"
#include "JNetworkAdapter.h"
#include "MapMatchingMHTMGPointCreator.h"
#include "MapMatchingMHTPointsCreator.h"
#include "MapMatchingMHTOEdgeTupleStreamCreator.h"
#include "MapMatchingMHTMPointCreator.h"
#include "MapMatchingMHTMJPointCreator.h"
#include "MapMatchingNetworkInterface.h"
#include "MapMatchingUtil.h"
#include "MHTRouteCandidate.h"
#include "MMRTree.h"

#include "MainMemoryExt.h"

namespace mapmatching {

using namespace std;

template <class T>
class MmORelNetworkEdge;

template <class T>
class MmORelNetwork;

template <class T>
class MmORelNetworkSectionAdapter;

template<class T>
class MmORelEdgeStreamCreator;

/*
class MmORelNetworkEdge
  Edge of Network based on ordered relation


The template parameter is the int-type used, e.g., CcInt or LongInt.

*/
template<class T>
class MmORelNetworkEdge {
  
  public:
    
    MmORelNetworkEdge()
      :m_pTupleEdge(NULL),
      m_pONetwork(NULL),
      m_dCurveLength(-1.0),
      m_dMaxSpeed(-1.0) {}
    
    MmORelNetworkEdge(Tuple* pTupleEdge,
                    MmORelNetwork<T>* pONetwork,
                    bool bIncReference)
      :m_pTupleEdge(pTupleEdge),
      m_pONetwork(pONetwork),
      m_dCurveLength(-1.0),
      m_dMaxSpeed(-1.0) {
    
      if (bIncReference && m_pTupleEdge != NULL)
        m_pTupleEdge->IncReference();
     }

    MmORelNetworkEdge(const MmORelNetworkEdge<T>& rEdge)
      :m_pTupleEdge(rEdge.m_pTupleEdge),
      m_pONetwork(rEdge.m_pONetwork),
      m_dCurveLength(rEdge.m_dCurveLength),
      m_dMaxSpeed(rEdge.m_dMaxSpeed) {
    
      if (m_pTupleEdge != NULL)
        m_pTupleEdge->IncReference();
    }

    ~MmORelNetworkEdge() {
      
      if (m_pTupleEdge != NULL)
        m_pTupleEdge->DeleteIfAllowed();
      m_pTupleEdge = NULL;

      m_pONetwork = NULL;
    }

    MmORelNetworkEdge<T>& operator=(const MmORelNetworkEdge<T>& rEdge) {
      if (&rEdge != this) {
        if (m_pTupleEdge != NULL) {
          m_pTupleEdge->DeleteIfAllowed();
          m_pTupleEdge = NULL;
        }

        m_pTupleEdge = rEdge.m_pTupleEdge;
        if (m_pTupleEdge != NULL)
            m_pTupleEdge->IncReference();

        m_pONetwork = rEdge.m_pONetwork;
        m_dCurveLength = rEdge.m_dCurveLength;
        m_dMaxSpeed = rEdge.m_dMaxSpeed;
      }

      return *this;
    }

    bool operator==(const MmORelNetworkEdge<T>& rEdge) const {
      if (m_pTupleEdge != NULL && rEdge.m_pTupleEdge != NULL) {
        return (m_pTupleEdge->GetTupleId() ==
                rEdge.m_pTupleEdge->GetTupleId() &&
                m_pONetwork == rEdge.m_pONetwork);
      }
      else {
        return m_pTupleEdge == rEdge.m_pTupleEdge;
      }
    }

    bool IsDefined(void) const {return m_pTupleEdge != NULL;}

    
    T* GetSource(void) const {
      if (m_pTupleEdge != NULL && m_pONetwork != NULL) {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxSource;
        if (nIdx >= 0)
          return static_cast<T*>(m_pTupleEdge->GetAttribute(nIdx));
        else
          return NULL;
      }
      else {
        return NULL;
      }
    }

    
    T* GetTarget(void) const {
      if (m_pTupleEdge != NULL) {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxTarget;
        if (nIdx >= 0)
            return static_cast<T*>(m_pTupleEdge->GetAttribute(nIdx));
        else
            return NULL;
      }
      else {
          return NULL;
      }
    }

    Point GetSourcePoint(void) const {
      if (m_pTupleEdge != NULL && m_pONetwork != NULL) {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxSourcePos;
        if (nIdx >= 0) {
            Point* pPtSource = static_cast<Point*>(
                                            m_pTupleEdge->GetAttribute(nIdx));
            return pPtSource != NULL ? *pPtSource : Point(false);
        }
        else
          return Point(false);
      }
      else {
        return Point(false);
      }
    }

    Point GetTargetPoint(void) const {
      if (m_pTupleEdge != NULL && m_pONetwork != NULL) {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxTargetPos;
        if (nIdx >= 0) {
          Point* pPtTarget = static_cast<Point*>(
                              m_pTupleEdge->GetAttribute(nIdx));
          return pPtTarget != NULL ? *pPtTarget : Point(false);
        }
        else {
          return Point(false);
        }
      }
      else {
        return Point(false);
      }
    }


    SimpleLine* GetCurve(void) const {
      if (m_pTupleEdge != NULL && m_pONetwork != NULL) {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxCurve;
        if (nIdx >= 0) {
          return static_cast<SimpleLine*>(m_pTupleEdge->GetAttribute(nIdx));
        }
        else
          return NULL;
      }
      else {
        return NULL;
      }
    }


    std::string GetRoadName(void) const {
      if (m_pTupleEdge != NULL && m_pONetwork != NULL) {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxRoadName;
        if (nIdx >= 0) {
          FText* pRoadName = static_cast<FText*>(
                                            m_pTupleEdge->GetAttribute(nIdx));
          if (pRoadName != NULL && pRoadName->IsDefined())
            return pRoadName->GetValue();
          else
            return "";
        }
        else {
          return "";
        }
      }
      else {
        return "";
      }
    }

    std::string GetRoadType(void) const {
      if (m_pTupleEdge != NULL && m_pONetwork != NULL) {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxRoadType;
        if (nIdx >= 0) {
          FText* pRoadType = static_cast<FText*>(
                              m_pTupleEdge->GetAttribute(nIdx));
          if (pRoadType != NULL && pRoadType->IsDefined())
            return pRoadType->GetValue();
          else
            return "";
        }
        else
          return "";
      }
      else {
        return "";
      }
    }


    static double convStrToDouble (const char* pszStr) {
      if (pszStr == NULL)
        return 0.0;

      return atof(pszStr);
    }

    double GetMaxSpeed(void) const {
      /*
      * http://wiki.openstreetmap.org/wiki/Key:maxspeed
      *
      *  maxspeed=60       -> km/h
      *  maxspeed=50 mph   -> mph
      *  maxspeed=10 knots -> knots
      *
      */

      if (m_pTupleEdge != NULL && m_pONetwork != NULL && m_dMaxSpeed < 0.0) {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxMaxSpeed;
        if (nIdx >= 0) {
          FText* pMaxSpeed = static_cast<FText*>(
                                            m_pTupleEdge->GetAttribute(nIdx));
          if (pMaxSpeed != NULL && pMaxSpeed->IsDefined()) {
            m_dMaxSpeed = convStrToDouble(pMaxSpeed->GetValue().c_str());

            // convert to km/h
            if (pMaxSpeed->GetValue().find("mph") != std::string::npos) {
              m_dMaxSpeed *= 1.609344;
            }
            else if (pMaxSpeed->GetValue().find("knots") 
                      != std::string::npos) {
                m_dMaxSpeed *= 1.852;
            }
          }
          else
              m_dMaxSpeed = 0.0;
        }
        else
          m_dMaxSpeed = 0.0;
      }

      return m_dMaxSpeed;
    }

    T* GetWayId(void) const {
      if (m_pTupleEdge != NULL && m_pONetwork != NULL) {
        const int nIdx = m_pONetwork->m_EdgeAttrIndexes.m_IdxWayId;
        if (nIdx >= 0)
          return static_cast<T*>(m_pTupleEdge->GetAttribute(nIdx));
        else
          return NULL;
      }
      else {
        return NULL;
      }
    }
    

    double GetCurveLength(const double dScale) const {
      if (m_dCurveLength < 0.) 
          m_dCurveLength = mapmatch::MMUtil::CalcLengthCurve(GetCurve(), 
                                                             dScale);
      
      return m_dCurveLength;
    }


    void Print(std::ostream& os) const {
      os << "Source: ";
      GetSource()->Print(os);
      os << endl;
      os << "Target: ";
      GetTarget()->Print(os);
      os << endl;
      os << "Source-Point: ";
      GetSourcePoint().Print(os);
      os << endl;
      os << "Target-Point: ";
      GetTargetPoint().Print(os);
      os << endl;
      os << "Curve: ";
      GetCurve()->Print(os);
      os << endl;
      os << "RoadName: " << GetRoadName() << endl;
      os << "RoadType: " << GetRoadType() << endl;
      os << "MaxSpeed: " << GetMaxSpeed() << endl;
      os << "WayId: " << GetWayId() << endl;
    }

    const Tuple* GetTuple(void) const {return m_pTupleEdge;}

  private:

    Tuple* m_pTupleEdge;
    MmORelNetwork<T>* m_pONetwork;
    mutable double m_dCurveLength;
    mutable double m_dMaxSpeed;
};





/*
class MmORelNetwork
  Network based on main memory ordered relation

The template parameter is the int-type used.

*/
template<class T>
class MmORelNetwork {
  
  friend class MmORelNetworkEdge<T>;
  
public:

    struct OEdgeAttrIndexes {
        OEdgeAttrIndexes(int IdxSource = -1, int IdxTarget = -1,
                         int IdxSourcePos = -1, int IdxTargetPos = -1,
                         int IdxCurve = -1, int IdxRoadName = -1,
                         int IdxRoadType = -1, int IdxMaxSpeed = -1,
                         int IdxWayId = -1)
        :m_IdxSource(IdxSource), m_IdxTarget(IdxTarget),
         m_IdxSourcePos(IdxSourcePos), m_IdxTargetPos(IdxTargetPos),
         m_IdxCurve(IdxCurve), m_IdxRoadName(IdxRoadName),
         m_IdxRoadType(IdxRoadType), m_IdxMaxSpeed(IdxMaxSpeed),
         m_IdxWayId(IdxWayId)
        {}

        int m_IdxSource;
        int m_IdxTarget;
        int m_IdxSourcePos;
        int m_IdxTargetPos;
        int m_IdxCurve;
        int m_IdxRoadName;
        int m_IdxRoadType;
        int m_IdxMaxSpeed;
        int m_IdxWayId;
    };
    
    
    MmORelNetwork(mm2algebra::MemoryORelObject* pOrderedRelation,
                mm2algebra::MemoryRtreeObject<2>* pRTreeEdges,
                mm2algebra::MemoryRelObject* pIndexEdges,
                const OEdgeAttrIndexes& rEdgeAttrIndexes)
      :m_pOrderedRelation(pOrderedRelation),
       m_pRTreeEdges(pRTreeEdges),
       m_pIndexEdges(pIndexEdges),
       m_bOwnData(false),
       m_EdgeAttrIndexes(rEdgeAttrIndexes)
    {}
    
 
    ~MmORelNetwork() {
        if (m_bOwnData) {
            delete m_pOrderedRelation;
            delete m_pRTreeEdges;
            delete m_pIndexEdges;
        }

        m_pOrderedRelation = NULL;
        m_pRTreeEdges = NULL;
        m_pIndexEdges = NULL;
    }
    
    

    bool GetEdgesTuple(const Tuple* pTuple,
             std::vector<MmORelNetworkEdge<T> >& vecEdges) {
      
      cout << "OrelNetwork GetEdgesTuple" << endl;
      if (pTuple == NULL ||
          m_pOrderedRelation == NULL ||
          m_EdgeAttrIndexes.m_IdxSource < 0 ||
          m_EdgeAttrIndexes.m_IdxTarget < 0)
          return false;

      
      ttree::Iterator<TupleWrap,mm2algebra::TupleComp> it 
                                = m_pOrderedRelation->getmmorel()->begin();
      // TODO check
      while(it.hasNext()) {
        Tuple* pTupleEdge = *it;
    
        // find all tuple with startnode as first argument
        if((pTupleEdge->GetAttribute(m_EdgeAttrIndexes.m_IdxSource)
          ->Compare(pTuple->GetAttribute(m_EdgeAttrIndexes.m_IdxSource)) == 0) 
          &&
         (pTupleEdge->GetAttribute(m_EdgeAttrIndexes.m_IdxTarget)
         ->Compare(pTuple->GetAttribute(m_EdgeAttrIndexes.m_IdxTarget)) == 0)){
          
          cout << "GetEdges private push_back Tuple: " << endl;
        pTupleEdge->Print(cout);
          vecEdges.push_back(MmORelNetworkEdge<T>(pTupleEdge, this, false));   
          break;
          
        }
        else 
          it++;
      }
      
      return true;
    }
    
    // TODO
    bool GetEdges(const Rectangle<2>& rBBox,
             std::vector<MmORelNetworkEdge<T> >& vecEdges) {
    
//       cout << "MmORelNetworkEdge GetEdges()" << endl;
      rBBox.Print(cout);
      if (!rBBox.IsDefined() || 
          m_pRTreeEdges == NULL || 
          m_pIndexEdges == NULL) {
        assert(false);
        return false;
      }
      
      std::set<size_t> res;
      // get first entry in rtree
      mmrtree::RtreeT<2, size_t>* tree = m_pRTreeEdges->getrtree();
      tree->findAll(rBBox,res);
      
      for (std::set<size_t>::iterator it = res.begin(); it!=res.end(); it++) {
        TupleId id = *it;
        Tuple* pTuple = m_pIndexEdges->getTuple(id);
        if (pTuple != NULL) {
          GetEdgesTuple(pTuple, vecEdges);
          pTuple->DeleteIfAllowed(); pTuple = NULL;
        }
      }
      return true;
    }


    // TODO
    bool GetAdjacentEdges(const MmORelNetworkEdge<T>& rEdge,
                          const bool bUpDown,
                          std::vector<MmORelNetworkEdge<T> >& vecEdges) const {
    
      if (m_pOrderedRelation == NULL)
        return false;

      return true;
    }


    Rectangle<2> GetBoundingBox(void) const {
      if (m_pRTreeEdges != NULL) {
        mmrtree::RtreeT<2, size_t>* tree = m_pRTreeEdges->getrtree();
        return tree->getBBox();
      }
      else
        return Rectangle<2>(false);
    }

    Tuple* GetUndefEdgeTuple(void) {
     
      Rectangle<2> BBox = GetBoundingBox();

      if (!BBox.IsDefined() || 
          m_pRTreeEdges == NULL || 
          m_pIndexEdges == NULL) {
        return NULL;
      }

      std::set<size_t> res;
      // get first entry in rtree
      mmrtree::RtreeT<2, size_t>* tree = m_pRTreeEdges->getrtree();
      tree->findAll(BBox,res);
      
      if(!res.empty()) {
        for(std::set<size_t>::iterator it=res.begin(); 
            it!=res.end(); it=res.end()) {
          TupleId id = *it;
          Tuple* pTuple = m_pIndexEdges->getTuple(id);
          if (pTuple != NULL) {
            std::vector<MmORelNetworkEdge<T> > vecEdges;
            GetEdgesTuple(pTuple, vecEdges);

            pTuple->DeleteIfAllowed(); pTuple = NULL;
            
            
            if(vecEdges.size() > 0) {
              const Tuple* pEdgeTuple = vecEdges.front().GetTuple();
              if (pEdgeTuple != NULL) {
                Tuple* pTupleRes = pEdgeTuple->Clone();
                if (pTupleRes != NULL) {
                  const int nAttributes = pTupleRes->GetNoAttributes();
                  for (int i = 0; i < nAttributes; ++i) {
                    Attribute* pAttr = pTupleRes->GetAttribute(i);
                    if(pAttr != NULL)
                      pAttr->SetDefined(false);
                  }
                }
                return pTupleRes;
              }
            }
          }
          assert(false);
          return NULL;
        }
        return 0;
      }
      else {
        return NULL;
      }
    }

  private:



    mm2algebra::MemoryORelObject* m_pOrderedRelation;
    mm2algebra::MemoryRtreeObject<2>* m_pRTreeEdges;
    mm2algebra::MemoryRelObject* m_pIndexEdges;
    bool m_bOwnData;

    OEdgeAttrIndexes m_EdgeAttrIndexes;
    
};




template<class T>
class MmORelNetworkAdapter : public mapmatch::IMMNetwork {
  
  public:
    MmORelNetworkAdapter(MmORelNetwork<T>* pNetwork)
      :m_pNetwork(pNetwork) {}
    
    
    MmORelNetworkAdapter(const MmORelNetworkAdapter<T>& rNetworkAdapter)
      :m_pNetwork(rNetworkAdapter.m_pNetwork) {}

    ~MmORelNetworkAdapter() {
      m_pNetwork = NULL;
    }

    
    bool GetSections(const Rectangle<2>& rBBox,
      std::vector<shared_ptr<mapmatch::IMMNetworkSection> >& rvecSections) 
      const {
        
      if (m_pNetwork == NULL)
          return false;

      std::vector<MmORelNetworkEdge<T> > vecEdges;
      if (!m_pNetwork->GetEdges(rBBox, vecEdges))
        return false;

      const size_t nEdges = vecEdges.size();
      for (size_t i = 0; i < nEdges; ++i) {
        const MmORelNetworkEdge<T>& rEdge = vecEdges[i];
        shared_ptr<mapmatch::IMMNetworkSection> pSection(
                      new MmORelNetworkSectionAdapter<T>(rEdge, m_pNetwork));
        rvecSections.push_back(pSection);
      }

      return true;
    }

    
    Rectangle<2> GetBoundingBox(void) const {
      if (m_pNetwork != NULL)
        return m_pNetwork->GetBoundingBox();
      else
        return Rectangle<2>(false);
    }
    

    double GetNetworkScale(void) const {
      return 1.0;
    }

    
    bool IsDefined(void) const {
      return m_pNetwork != NULL;
    }

    
    bool CanGetRoadType(void) const {
      return true;
    }

    Tuple* GetUndefEdgeTuple(void) const {
      if (m_pNetwork != NULL)
        return m_pNetwork->GetUndefEdgeTuple();
      else
        return NULL;
    }

  private:
    MmORelNetwork<T>* m_pNetwork;
};




/*
class MmORelNetworkSectionAdapter

*/
template<class T>
class MmORelNetworkSectionAdapter : public mapmatch::IMMNetworkSection {
  
  friend class MmORelNetworkEdge<T>;
  
  
  public:
    MmORelNetworkSectionAdapter()
      :m_pEdge(NULL),
       m_pONetwork(NULL),
       m_eRoadType((ERoadType)-1) {}

    MmORelNetworkSectionAdapter(
                            const MmORelNetworkEdge<T>& rEdge,
                            const MmORelNetwork<T>* pONetwork)
      :m_pEdge(new MmORelNetworkEdge<T>(rEdge)),
       m_pONetwork(pONetwork),
       m_eRoadType((ERoadType)-1) {}
    

    MmORelNetworkSectionAdapter(
         const MmORelNetworkSectionAdapter<T>& rONetworkSectionAdapter)
      :m_pEdge(rONetworkSectionAdapter.m_pEdge != NULL ?
         new MmORelNetworkEdge<T>(*rONetworkSectionAdapter.m_pEdge) : NULL),
       m_pONetwork(rONetworkSectionAdapter.m_pONetwork),
       m_eRoadType(rONetworkSectionAdapter.m_eRoadType) {}
       
    ~MmORelNetworkSectionAdapter() {
      delete m_pEdge;
      m_pEdge = NULL;
      m_pONetwork = NULL;
    }

    const SimpleLine* GetCurve(void) const {
      if (m_pEdge != NULL)
        return m_pEdge->GetCurve();
      else
        return NULL;
    }

    double GetCurveLength(const double dScale) const {
      if (m_pEdge != NULL)
        return m_pEdge->GetCurveLength(dScale);
      else
        return 0.0;
    }

    bool GetCurveStartsSmaller(void) const {
      if (m_pEdge != NULL)
        return m_pEdge->GetCurve()->GetStartSmaller();
      else
        return true;
    }

    Point GetStartPoint(void) const {
      if (m_pEdge != NULL)
        return m_pEdge->GetSourcePoint();
      else
        return Point(false);
    }

    Point GetEndPoint(void) const {
      if (m_pEdge != NULL)
        return m_pEdge->GetTargetPoint();
      else
        return Point(false);
    }

    mapmatch::IMMNetworkSection::EDirection GetDirection(void) const {
      if (m_pEdge != NULL)
        return DIR_UP;
      else
        return DIR_NONE;
    }

    bool IsDefined(void) const {
      return m_pEdge != NULL && m_pEdge->IsDefined();
    }

    bool GetAdjacentSections(const bool _bUpDown,
             std::vector<shared_ptr<mapmatch::IMMNetworkSection> >& 
             rvecSections) const {
      if (m_pEdge == NULL || m_pONetwork == NULL)
          return false;

      std::vector<MmORelNetworkEdge<T> > vecEdges;
      m_pONetwork->GetAdjacentEdges(*m_pEdge, _bUpDown, vecEdges);

      const typename T::inttype nSource = m_pEdge->GetSource()->GetValue();
      const typename T::inttype nTarget = m_pEdge->GetTarget()->GetValue();

      size_t nEdges = vecEdges.size();
      for (size_t i = 0; i < nEdges; ++i) {
          const MmORelNetworkEdge<T>& rEdge = vecEdges[i];

          if ((rEdge.GetSource()->GetValue() == nSource &&
              rEdge.GetTarget()->GetValue() == nTarget)   ||
              (rEdge.GetSource()->GetValue() == nTarget &&
              rEdge.GetTarget()->GetValue() == nSource))
          {
              continue;
          }

          shared_ptr<mapmatch::IMMNetworkSection> pAdjSection(
                    new MmORelNetworkSectionAdapter<T>(rEdge, m_pONetwork));
          rvecSections.push_back(pAdjSection);
      }

      return true;
    }

    std::string GetRoadName(void) const {
      if (m_pEdge != NULL)
        return m_pEdge->GetRoadName();
     else
       return "";
    }

    mapmatch::IMMNetworkSection::ERoadType GetRoadType(void) const {
      if (((int)m_eRoadType) >= 0)
        return m_eRoadType;
      
      else if (m_pEdge != NULL) {
        const std::string& strRoadType = m_pEdge->GetRoadType();

        if (strRoadType.compare("motorway") == 0)
          m_eRoadType = RT_MOTORWAY;
        
        else if (strRoadType.compare("trunk") == 0)
          m_eRoadType = RT_TRUNK;
        
        else if (strRoadType.compare("primary") == 0)
          m_eRoadType = RT_PRIMARY;
        
        else if (strRoadType.compare("secondary") == 0)
          m_eRoadType = RT_SECONDARY;
        
        else if (strRoadType.compare("tertiary") == 0) 
          m_eRoadType = RT_TERTIARY;
        
        else if (strRoadType.compare("living_street") == 0)
          m_eRoadType = RT_LIVING_STREET;
        
        else if (strRoadType.compare("pedestrian") == 0)
          m_eRoadType = RT_PEDESTRIAN;
        
        else if (strRoadType.compare("residential") == 0)
          m_eRoadType = RT_RESIDENTIAL;
        
        else if (strRoadType.compare("service") == 0)
          m_eRoadType = RT_SERVICE;
        
        else if (strRoadType.compare("track") == 0)
          m_eRoadType = RT_TRACK;
        
        else if (strRoadType.compare("road") == 0)
          m_eRoadType = RT_ROAD;
        
        else if (strRoadType.compare("path") == 0)
          m_eRoadType = RT_PATH;
        
        else if (strRoadType.compare("footway") == 0)
          m_eRoadType = RT_FOOTWAY;
        
        else if (strRoadType.compare("cycleway") == 0)
          m_eRoadType = RT_CYCLEWAY;
        
        else if (strRoadType.compare("bridleway") == 0)
          m_eRoadType = RT_BRIDLEWAY;
        
        else if (strRoadType.compare("steps") == 0)
          m_eRoadType = RT_STEPS;
        
        else if (strRoadType.compare("proposed") == 0)
          m_eRoadType = RT_PROPOSED;
        
        else if (strRoadType.compare("construction") == 0)
          m_eRoadType = RT_CONSTRUCTION;
        
        else
          m_eRoadType = RT_OTHER;
        
        return m_eRoadType;
      }
      else
        return IMMNetworkSection::RT_UNKNOWN;
      
    }

    double GetMaxSpeed(void) const {
      if (m_pEdge != NULL) {
        return m_pEdge->GetMaxSpeed();
      }
      else {
        return -1.0;
      }
    }

    bool operator==(const mapmatch::IMMNetworkSection& rSection) const {
      const MmORelNetworkSectionAdapter<T>* pSectionAdapter =
                        (MmORelNetworkSectionAdapter<T>*)  &rSection;
      if (pSectionAdapter != NULL) {
        MmORelNetworkEdge<T>* pEdgeOther = pSectionAdapter->m_pEdge;
        if (pEdgeOther != NULL && m_pEdge != NULL) {
          return *m_pEdge == *pEdgeOther;
        }
        else {
          return m_pEdge == pEdgeOther;
        }
      }
      else {
        assert(false);
        return false;
      }
    }

    const MmORelNetworkSectionAdapter<T>* CastToONetworkSection( void) const {
      return this;
    }


    MmORelNetworkSectionAdapter<T>* CastToONetworkSection(void) {
      return this;
    }



    void PrintIdentifier(std::ostream& os) const {
      if (m_pEdge != NULL) {
        os << m_pEdge->GetSource()->GetValue()
          << " : "
          << m_pEdge->GetTarget()->GetValue();
      }
      else
        os << "invalid";
    }

    const MmORelNetworkEdge<T>* GetNetworkEdge(void) const {return m_pEdge;}

  private:
    MmORelNetworkEdge<T>* m_pEdge;
    const MmORelNetwork<T>* m_pONetwork;
    mutable mapmatch::IMMNetworkSection::ERoadType m_eRoadType;
};




/*

class MmORelEdgeStreamCreator

*/
template<class T>
class MmORelEdgeStreamCreator : public mapmatch::IMapMatchingMHTResultCreator {
 
public:
    enum EMode {
        MODE_EDGES,
        MODE_EDGES_AND_POSITIONS
    };

    // TODO
    MmORelEdgeStreamCreator(Supplier s,
                             const MmORelNetworkAdapter<T>& rNetw,
                             EMode eMode)
      :m_eMode(eMode),
       m_pTupleType(NULL),

       m_pTupleUndefEdge(NULL),
       m_dNetworkScale(rNetw.GetNetworkScale()) {
        
      ListExpr tupleType = GetTupleResultType(s);
      m_pTupleType = new TupleType(nl->Second(tupleType));

      m_pTupleUndefEdge = rNetw.GetUndefEdgeTuple();
    }

    // TODO
    ~MmORelEdgeStreamCreator() {
      if (m_pTupleType != NULL)
        m_pTupleType->DeleteIfAllowed();
      m_pTupleType = NULL;

      if (m_pTupleUndefEdge != NULL)
          m_pTupleUndefEdge->DeleteIfAllowed();
      m_pTupleUndefEdge = NULL;

    }



    bool CreateResult(const std::vector<mapmatch::MHTRouteCandidate*>& 
      rvecRouteCandidates) {
      Init();

      const size_t nCandidates = rvecRouteCandidates.size();

      for (size_t i = 0; i < nCandidates; ++i) {
        mapmatch::MHTRouteCandidate* pCandidate = rvecRouteCandidates[i];
        if (pCandidate == NULL)
            continue;

        DateTime EndTimePrev((int64_t)0);
        mapmatch::MHTRouteCandidate::PointDataPtr pDataEnd =
                                  mapmatch::MHTRouteCandidate::PointDataPtr();

        mapmatch::MHTRouteCandidate::RouteSegmentIterator it =
                                              pCandidate->RouteSegmentBegin();
        mapmatch::MHTRouteCandidate::RouteSegmentIterator itEnd =
                                              pCandidate->RouteSegmentEnd();


        for (/*empty*/; it != itEnd; ++it) {
          const mapmatch::MHTRouteCandidate::RouteSegmentPtr& pSegment = *it;
          if (pSegment == NULL)
              continue;

          double dDistance = 0.0;
          const mapmatch::MHTRouteCandidate::PointDataPtr& pData =
                                  GetFirstPointOfNextSegment(it,
                                                              itEnd,
                                                              dDistance);
          EndTimePrev = ProcessSegment(*pSegment,
                                        EndTimePrev,
                                        pDataEnd,
                                        pData,
                                        dDistance);

          if (pSegment->GetPoints().size() > 0)
              pDataEnd = pSegment->GetPoints().back();
          else
              pDataEnd = mapmatch::MHTRouteCandidate::PointDataPtr();
        }
      }

      return true;
    }

    // TODO
    Tuple* GetNextTuple(void) const {

          return NULL;

    }



  private:
    // TODO  
    void Init(void) {

    }

    const mapmatch::MHTRouteCandidate::PointDataPtr GetFirstPointOfNextSegment(
              mapmatch::MHTRouteCandidate::RouteSegmentIterator itSegments,
              mapmatch::MHTRouteCandidate::RouteSegmentIterator itSegmentsEnd,
              double& dDistance) {
      ++itSegments;
      dDistance = 0.0;

      while (itSegments != itSegmentsEnd) {
        const mapmatch::MHTRouteCandidate::RouteSegmentPtr& pSegment = 
                                                                  *itSegments;
        if (pSegment == NULL) {
            ++itSegments;
            continue;
        }

        const shared_ptr<mapmatch::IMMNetworkSection> pSect =
                                                    pSegment->GetSection();
        if (pSect != NULL && pSect->GetCurve() != NULL) {
          if (pSegment->GetPoints().size() == 0) {
              dDistance += pSect->GetCurveLength(m_dNetworkScale);
          }
          else {
            const mapmatch::MHTRouteCandidate::PointDataPtr pData =
                                          pSegment->GetPoints().front();
            if (pData != NULL && pData->GetPointProjection() != NULL) {
              const SimpleLine* pCurve = pSect->GetCurve();
              const Point& rPtStart = pSect->GetStartPoint();
              const Point* pPt = pData->GetPointProjection();

              const double dDistStart2FirstPt =
                          mapmatch::MMUtil::CalcDistance(rPtStart,
                                                        *pPt,
                                                        *pCurve,
                                                        m_dNetworkScale);

              dDistance += dDistStart2FirstPt;
              return pData;
            }
            else {
              assert(false);
            }
          }
        }

        ++itSegments;
      }

      return mapmatch::MHTRouteCandidate::PointDataPtr();
    }

    
  
   
    DateTime ProcessSegment(
      const mapmatch::MHTRouteCandidate::RouteSegment& rSegment,
      const DateTime& rEndTimePrevSegment,
      const mapmatch::MHTRouteCandidate::PointDataPtr& pPrevPointData,
      const mapmatch::MHTRouteCandidate::PointDataPtr& pFirstPointofNextSeg,
      double dDistance) /* Distance to first point of next segment */ {
        if (m_eMode == MODE_EDGES) {
            return ProcessSegment_Edges(rSegment,
                                        rEndTimePrevSegment,
                                        pPrevPointData,
                                        pFirstPointofNextSeg,
                                        dDistance);
        }
        else if (m_eMode == MODE_EDGES_AND_POSITIONS) {
            return ProcessSegment_EdgesAndPositions(rSegment,
                                                    rEndTimePrevSegment,
                                                    pPrevPointData,
                                                    pFirstPointofNextSeg,
                                                    dDistance);
        }
        else {
          assert(false);
          DateTime TimeUndef((int64_t)0);
          TimeUndef.SetDefined(false);
          return TimeUndef;
        }
    }

  
  

    DateTime ProcessSegment_Edges(
      const mapmatch::MHTRouteCandidate::RouteSegment& rSegment,
      const DateTime& rEndTimePrevSegment,
      const mapmatch::MHTRouteCandidate::PointDataPtr& pPrevPointData,
      const mapmatch::MHTRouteCandidate::PointDataPtr& pFirstPointofNextSeg,
      double dDistance) /* Distance to first point of next segment */ {
      
      const shared_ptr<mapmatch::IMMNetworkSection> pSection = 
                                                        rSegment.GetSection();
      if (pSection == NULL)
          return DateTime((int64_t)0);

      const std::vector<mapmatch::MHTRouteCandidate::PointDataPtr>& vecPoints =
                                                          rSegment.GetPoints();
      const size_t nPoints = vecPoints.size();

      const SimpleLine* pCurve = pSection->GetCurve();

      if (rSegment.IsOffRoad() || pCurve == NULL) { // Offroad
          return DateTime((int64_t)0);
      }

      // Get first point
      Point PtStart = pSection->GetStartPoint();
      DateTime TimeStart = rEndTimePrevSegment;

      if (!TimeStart.IsDefined() || TimeStart.IsZero()) {
        // first segment -> Get first  projected point
        for (size_t i = 0; i < nPoints; ++i) {
          const mapmatch::MHTRouteCandidate::PointDataPtr& pData = 
                                                                 vecPoints[i];
          if (pData != NULL) {
            const Point* pPtProj = pData->GetPointProjection();
            if (pPtProj != NULL && pPtProj->IsDefined()) {
                PtStart = *pPtProj;
                TimeStart = pData->GetTime();
                break;
            }
          }
        }

        if (!TimeStart.IsDefined() || TimeStart.IsZero())
            return DateTime((int64_t)0);; // first segment and no point
      }

      // get last point
      DateTime TimeLastPt((int64_t) 0);
      TimeLastPt.SetDefined(false);

      Point PtLast(GetLastProjectedPoint(vecPoints, TimeLastPt));

      // Process endpoint of segment

      if (pFirstPointofNextSeg == NULL ||
          !pFirstPointofNextSeg->GetTime().IsDefined() ||
          pFirstPointofNextSeg->GetPointProjection() == NULL ||
          !pFirstPointofNextSeg->GetPointProjection()->IsDefined()) {
          // this is the last point or the next segment is offroad

          if (!PtLast.IsDefined()) {
              PtLast = PtStart;
              TimeLastPt = TimeStart;
          }

          ProcessPoints(rSegment, TimeStart, TimeLastPt, PtStart, PtLast);
          return TimeLastPt;
      }
      else {
          Point PtEnd = rSegment.HasUTurn() ? pSection->GetStartPoint():
                                              pSection->GetEndPoint();

          // Calculate distance to endpoint of section
          double dDist2End = 0.0;
          DateTime Duration((int64_t)0); // Time to first point of next segment

          DateTime TimeEnd((int64_t)0);

          if (!PtLast.IsDefined()) {
            dDist2End = pSection->GetCurveLength(m_dNetworkScale);
            Duration = pFirstPointofNextSeg->GetTime() - TimeStart;
            TimeEnd = TimeStart;
          }
          else {
              dDist2End = mapmatch::MMUtil::CalcDistance(PtLast,
                                                         PtEnd,
                                                         *pCurve,
                                                         m_dNetworkScale);
              Duration = pFirstPointofNextSeg->GetTime() - TimeLastPt;
              TimeEnd = TimeLastPt;
          }

          double dDist2FirstPtOfNextSeg = dDist2End + dDistance;

          if (!AlmostEqual(dDist2FirstPtOfNextSeg, 0.0)) {
              TimeEnd += DateTime(datetime::durationtype,
                              (uint64_t)((Duration.millisecondsToNull()
                                      / dDist2FirstPtOfNextSeg) * dDist2End));

              if(!mapmatch::MMUtil::CheckSpeed(dDist2FirstPtOfNextSeg,
                                      TimeStart, TimeEnd, PtStart, PtEnd)) {
                  return TimeEnd;
              }
          }

          ProcessPoints(rSegment, TimeStart, TimeEnd, PtStart, PtEnd);

          return TimeEnd;
      }
    }

  
  
    
    DateTime ProcessSegment_EdgesAndPositions(
        const mapmatch::MHTRouteCandidate::RouteSegment& rSegment,
        const DateTime& rEndTimePrevSegment,
        const mapmatch::MHTRouteCandidate::PointDataPtr& pPrevPointData,
        const mapmatch::MHTRouteCandidate::PointDataPtr& pFirstPointofNextSeg,
        double dDistance) { // Distance to first point of next segment
    
      const shared_ptr<mapmatch::IMMNetworkSection> pSection = 
                                                      rSegment.GetSection();
      const std::vector<mapmatch::MHTRouteCandidate::PointDataPtr>& vecPoints =
                                                      rSegment.GetPoints();
      const size_t nPoints = vecPoints.size();

      DateTime TimeStart = rEndTimePrevSegment;
      Point PtStart(false);

      const SimpleLine* pCurve = pSection != NULL?pSection->GetCurve():NULL;
      bool bStartsSmaller = pSection != NULL?pSection->GetCurveStartsSmaller()
                                            :false;

      if (!TimeStart.IsDefined() || TimeStart.IsZero()) {
          // first segment
        mapmatch::MHTRouteCandidate::PointDataPtr pData = (nPoints > 0) ?
                                                            vecPoints.front():
                                  mapmatch::MHTRouteCandidate::PointDataPtr();

        if (pData != NULL) {
          TimeStart = pData->GetTime();

          const Point* pPtProj = pData->GetPointProjection();
          if (pPtProj != NULL && pPtProj->IsDefined())
            PtStart = *pPtProj;
          else
            PtStart = pData->GetPointGPS();
        }
        else {
            return DateTime((int64_t)0); // first segment and no point
        }
      }
      else {
        if (pCurve == NULL) {
          // this segment is offroad
          if (pPrevPointData == NULL) {
            mapmatch::MHTRouteCandidate::PointDataPtr pData = (nPoints > 0) ?
                                                      vecPoints.front() :
                                   mapmatch::MHTRouteCandidate::PointDataPtr();

            if (pData != NULL) {
              const Point* pPtProj = pData->GetPointProjection();
              if (pPtProj != NULL && pPtProj->IsDefined())
                  PtStart = *pPtProj;
              else
                  PtStart = pData->GetPointGPS();
            }
            else {
                return DateTime((int64_t)0);
            }
          }
          else {
              PtStart = *(pPrevPointData->GetPointProjection());
              if (!PtStart.IsDefined())
                PtStart = pPrevPointData->GetPointGPS();
          }
          const Point* pPtProj = pPrevPointData->GetPointProjection();
          if (pPtProj != NULL)
            PtStart = *pPtProj;
        }
        else {
          if (pPrevPointData == NULL ||
              pPrevPointData->GetPointProjection() != NULL) { // onroad
            PtStart = pCurve->StartPoint(bStartsSmaller);
          }
          else
              PtStart = pPrevPointData->GetPointGPS();
        }
      }

      // Process all points

      Point PtEnd(false);
      DateTime TimeEnd((int64_t)0);

      for (size_t i = 0; i < nPoints; ++i) {
        const mapmatch::MHTRouteCandidate::PointDataPtr& pData = vecPoints[i];
        if (pData == NULL)
            continue;

        TimeEnd = pData->GetTime();

        const Point* pPtProj = pData->GetPointProjection();
        if (pPtProj != NULL && pPtProj->IsDefined()) {
          PtEnd = *pPtProj;
        }
        else {
          PtEnd = pData->GetPointGPS();
        }

        if (TimeStart == TimeEnd && AlmostEqual(PtStart, PtEnd))
          continue;

        ProcessPoints(rSegment, TimeStart, TimeEnd, PtStart, PtEnd);

        PtStart = PtEnd;
        TimeStart = TimeEnd;
      }

      // Process endpoint of segment

      if (pFirstPointofNextSeg == NULL ||
          !pFirstPointofNextSeg->GetTime().IsDefined() ||
          pFirstPointofNextSeg->GetPointProjection() == NULL ||
          !pFirstPointofNextSeg->GetPointProjection()->IsDefined()) {
          // this is the last point or the next segment is offroad
          return TimeStart;
      }
      else if (pSection != NULL && pCurve != NULL) {
          // Calculate distance to endpoint of section

        Point PtEndSegment = rSegment.HasUTurn() ? pSection->GetStartPoint():
                                                  pSection->GetEndPoint();

        const double dDistLastPt2End = mapmatch::MMUtil::CalcDistance(PtStart,
                                                            PtEndSegment,
                                                            *pCurve,
                                                            m_dNetworkScale);

        double dDistLastPt2FirstPtOfNextSeg = dDistLastPt2End + dDistance;
        if (AlmostEqual(dDistLastPt2FirstPtOfNextSeg, 0.0)) {
          return TimeStart;
        }

        DateTime Duration = pFirstPointofNextSeg->GetTime() - TimeStart;
        DateTime TimeEnd = TimeStart +
                    DateTime(datetime::durationtype,
                            (uint64_t)((Duration.millisecondsToNull()
                              / dDistLastPt2FirstPtOfNextSeg) *
                                                           dDistLastPt2End));

        if (TimeStart != TimeEnd && !AlmostEqual(PtStart, PtEndSegment)) {
          if(mapmatch::MMUtil::CheckSpeed(dDistLastPt2End, TimeStart, TimeEnd,
                                PtStart, PtEnd,
                                rSegment.GetSection()->GetRoadType(),
                                rSegment.GetSection()->GetMaxSpeed())) {
            ProcessPoints(rSegment,
                          TimeStart, TimeEnd,
                          PtStart, PtEndSegment);
          }
        }

        return TimeEnd;
      }
      else {
        mapmatch::MHTRouteCandidate::PointDataPtr pData = (nPoints > 0) ?
                                                  vecPoints.back() :
                                mapmatch::MHTRouteCandidate::PointDataPtr();
        if (pData != NULL)
          return pData->GetTime();
        else
          return DateTime((int64_t)0);

      }
    }

     
    void ProcessPoints(const mapmatch::MHTRouteCandidate::RouteSegment& 
                                                                 rSegment,
                       const DateTime& rTimeStart,
                       const DateTime& rTimeEnd,
                       const Point& rPtStart,
                       const Point& rPtEnd) {
        
      if (m_eMode == MODE_EDGES) {
        CreateTuple(rSegment, rTimeStart, rTimeEnd);
      }
      else if (m_eMode == MODE_EDGES_AND_POSITIONS) {
        double dPosStart = -1.0;
        double dPosEnd = -1.0;

        const shared_ptr<mapmatch::IMMNetworkSection>& pSection = 
                                                      rSegment.GetSection();
        if (pSection != NULL && pSection->IsDefined()) {
          const SimpleLine* pCurve = pSection->GetCurve();
          if (pCurve != NULL && !pCurve->IsEmpty()) {
            const bool bStartsSmaller = pSection->GetCurveStartsSmaller();

            const Point& rPtStartCurve = pCurve->StartPoint(bStartsSmaller);
            const Point& rPtEndCurve = pCurve->EndPoint(bStartsSmaller);

            // calculate PosStart
            if (AlmostEqual(rPtStart, rPtStartCurve) ||
                AlmostEqual(rPtStart, rPtEndCurve)) {
              pCurve->AtPoint(rPtStart, bStartsSmaller, 0.0, dPosStart);
            }

            if (dPosStart < 0.0) {
              mapmatch::MMUtil::GetPosOnSimpleLine(*pCurve,
                                          rPtStart,
                                          bStartsSmaller,
                                          m_dNetworkScale,
                                          dPosStart);
            }

            // calculate PosEnd
            if (AlmostEqual(rPtEnd, rPtStartCurve) ||
                AlmostEqual(rPtEnd, rPtEndCurve)) {
              pCurve->AtPoint(rPtEnd, bStartsSmaller, 0.0, dPosEnd);
            }

            if (dPosEnd < 0.0) {
              mapmatch::MMUtil::GetPosOnSimpleLine(*pCurve,
                                          rPtEnd,
                                          bStartsSmaller,
                                          m_dNetworkScale,
                                          dPosEnd);
            }
          }
        }

        CreateTuple(rSegment,
                    rTimeStart, rTimeEnd,
                    rPtStart, rPtEnd,
                    dPosStart, dPosEnd);
      }
      else {
        assert(false);
      }
    }

  
  
    
    const Tuple* GetEdgeTuple(
                 const mapmatch::MHTRouteCandidate::RouteSegment& rSegment) {
        
      const shared_ptr<mapmatch::IMMNetworkSection> pSection = 
                                                        rSegment.GetSection();
        if (pSection == NULL) {
            return m_pTupleUndefEdge;
        }
    
        const MmORelNetworkSectionAdapter<T>* pAdapter =
               (const MmORelNetworkSectionAdapter<T>*)  pSection.get();
        if (pAdapter == NULL) {
            assert(false); // only for OrelNetwork
            return NULL;
        }
    
        const MmORelNetworkEdge<T>* pEdge = pAdapter->GetNetworkEdge();
        if (pEdge == NULL) {
          assert(false);
          return NULL;
        }
    
        return pEdge->GetTuple();
    }


    
    void CreateTuple(const mapmatch::MHTRouteCandidate::RouteSegment& rSegment,
                     const DateTime& rTimeStart,
                     const DateTime& rTimeEnd) {
      
      const Tuple* pTupleEdge = GetEdgeTuple(rSegment);
      if (pTupleEdge == NULL) {
          return;
      }

      // Create new Tuple and copy attributes
      Tuple* pResultTuple = new Tuple(m_pTupleType);
      int i = 0;
      while (i < pTupleEdge->GetNoAttributes()) {
        pResultTuple->CopyAttribute(i, pTupleEdge, i);
        ++i;
      }

      // Create new attributes
      DateTime* pTimeStart = new DateTime(rTimeStart);
      pResultTuple->PutAttribute(i, pTimeStart);
      ++i;

      DateTime* pTimeEnd = new DateTime(rTimeEnd);
      pResultTuple->PutAttribute(i, pTimeEnd);

      // Add new tuple TODO

      pResultTuple->DeleteIfAllowed();
    }

      
      
    
    void CreateTuple(const mapmatch::MHTRouteCandidate::RouteSegment& rSegment,
                     const DateTime& rTimeStart,
                     const DateTime& rTimeEnd,
                     const Point& rPtStart,
                     const Point& rPtEnd,
                     const double dPosStart,
                     const double dPosEnd) {
      
      const Tuple* pTupleEdge = GetEdgeTuple(rSegment);
      if (pTupleEdge == NULL) {
        //assert(false);
        return;
      }

      // Create new Tuple and copy attributes
      Tuple* pResultTuple = new Tuple(m_pTupleType);
      int i = 0;
      while (i < pTupleEdge->GetNoAttributes()) {
        pResultTuple->CopyAttribute(i, pTupleEdge, i);
        ++i;
      }

      // Create new attributes
      DateTime* pTimeStart = new DateTime(rTimeStart);
      pResultTuple->PutAttribute(i, pTimeStart);
      ++i;

      DateTime* pTimeEnd = new DateTime(rTimeEnd);
      pResultTuple->PutAttribute(i, pTimeEnd);
      ++i;

      Point* pPtStart = new Point(rPtStart);
      pResultTuple->PutAttribute(i, pPtStart);
      ++i;

      Point* pPtEnd = new Point(rPtEnd);
      pResultTuple->PutAttribute(i, pPtEnd);
      ++i;

      CcReal* pPosStart = NULL;
      if (dPosStart < 0.0)
          pPosStart = new CcReal(false);
      else
          pPosStart = new CcReal(true, dPosStart);

      pResultTuple->PutAttribute(i, pPosStart);
      ++i;

      CcReal* pPosEnd = NULL;
      if (dPosEnd < 0.0)
          pPosEnd = new CcReal(false);
      else
          pPosEnd = new CcReal(true, dPosEnd);

      pResultTuple->PutAttribute(i, pPosEnd);

      // Add new tuple  TODO

      pResultTuple->DeleteIfAllowed();
    }



    EMode m_eMode;
    TupleType* m_pTupleType;
    Tuple* m_pTupleUndefEdge;
    double m_dNetworkScale;
};



}

#endif
