/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[pow] [\verb+^+]

[1] Headerfile 

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype, 

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/
#ifndef PFACE_H_
#define PFACE_H_

#include <gmp.h>
#include <gmpxx.h>
#include <map>
#include <set>

#include "MovingRegion2Algebra.h"
#include "SourceUnit2.h"
#include "Point2D.h"
#include "Point3D.h"
#include "Vector3D.h"
#include "Segment3D.h"
#include "IntSegContainer.h"
#include "Angle.h"
#include "SetOps2.h"


namespace mregionops2 {


struct IntSegWCompare {

    bool operator()(const IntersectionSegment* const& s1,
            const IntersectionSegment* const& s2) const;
};

class PFace {

public:

    PFace(SourceUnit2* _unit, Point2D _is, Point2D _ie, 
          Point2D _fs, Point2D _fe,
          bool _insideAbove, unsigned int _cycNo, 
          unsigned int _faceNo);

    inline Point3D GetA_XYT() {

        if (InitialStartPointIsA())
            return Point3D(is.GetX(), is.GetY(),
                    unit->GetStartTime());
        else
            return Point3D(ie.GetX(), ie.GetY(),
                    unit->GetStartTime());
    }
    
    inline Point3D GetB_XYT() {

        if (InitialStartPointIsA())
            return Point3D(ie.GetX(), ie.GetY(),
                    unit->GetStartTime());
        else
            return Point3D(is.GetX(), is.GetY(),
                    unit->GetStartTime());
    }
    
    inline Point3D GetC_XYT() {

        if (InitialStartPointIsA())
            return Point3D(fs.GetX(), fs.GetY(),
                    unit->GetEndTime());
        else
            return Point3D(fe.GetX(), fe.GetY(),
                    unit->GetEndTime());
    }
    
    inline Point3D GetD_XYT() {

        if (InitialStartPointIsA())
            return Point3D(fe.GetX(), fe.GetY(),
                    unit->GetEndTime());
        else
            return Point3D(fs.GetX(), fs.GetY(),
                    unit->GetEndTime());
    }

    inline Point2D GetA_XY() {

        if (InitialStartPointIsA())
            return is;
        else
            return ie;
    }

    inline Point2D GetB_XY() {

        if (InitialStartPointIsA())
            return ie;
        else
            return is;
    }

    inline Point2D GetC_XY() {

        if (InitialStartPointIsA())
            return fs;
        else
            return fe;
    }

    inline Point2D GetD_XY() {

        if (InitialStartPointIsA())
            return fe;
        else
            return fs;
    }

    inline Vector3D GetNormalVector() const {
         return normalVector;
    }
 
    inline Vector3D GetVerticalVector() const {
         return verticalVector;
    }
 
    inline Vector3D GetWVector() const {
        return wVector;
    }

    inline bool GetPointInitial () {
         return (is == ie);
    }
 
    inline bool GetPointFinal () {
         return (fs == fe);
    }

    inline mpq_class GetStartTime() {
         return unit->GetStartTime();
    }

    inline mpq_class GetEndTime() {
         return unit->GetEndTime();
    }

    inline mpq_class GetTimeIntervalStart(unsigned int i) const {
      return unit->GetTimeIntervalStart(i);
    }

    inline mpq_class GetTimeIntervalEnd(unsigned int i) const {
      return unit->GetTimeIntervalEnd(i);
    }

    inline mpq_class TransformToW(const Point3D& xyt) const {
        return xyt.GetX() * GetWVector().GetX() +
               xyt.GetY() * GetWVector().GetY();
    }

    inline mpq_class TransformToW(const Vector3D& xyt) const {
        return xyt.GetX() * GetWVector().GetX() +
               xyt.GetY() * GetWVector().GetY();
    }

    inline Point2D TransformToWT(const Point3D& xyt) const {
        return Point2D(TransformToW(xyt), xyt.GetT());
    }

    inline const SetOp GetOperation() const {
        return unit->GetOperation();
    }

    inline bool IsPartOfUnitA() const {
        return unit->IsUnitA();
    }

    inline bool IsPartOfUnitB() const {
        return !unit->IsUnitA();
    }

    void Intersection(PFace& pf);    
    bool IsParallel(const PFace& pf);    
    Angle GetAngle();    
    void AddToIntSegsTable(unsigned int ind, IntersectionSegment* iseg);
    void FinalizeIntSegs(); 
    void Print();

    inline unsigned int Get_cycleNo()
	{ return cycleNo;}
    inline unsigned int Get_faceNo()
	{ return faceNo;}

    inline bool HasIntersegs()
    {
	if(intSegsToInterval.size() > 0)
		return true;
	else
		return false;
    }
    inline void SetDebugId(unsigned int ID)
    {
       debugId = ID;
    }

    inline void PrintIdentifier()
    {
	Point3D p_a = GetA_XYT();
	Point3D p_b = GetB_XYT();
	if(unit->IsUnitA() ==true)
		cout << "Unit A PFace " << debugId << 
		"; A=" << p_a << "; B=" << p_b << endl;
	else
		cout << "Unit B PFace " << debugId << 
		"; A=" << p_a << "; B=" << p_b << endl;
    }

	inline IntSegContainer* getIntSegs()
	{
		return &intSegs;
	}
   inline vector<IntersectionSegment*> GetIntSegs()
   {
	return myIntSegs;
   }
   inline vector<IntersectionSegment*> 
getIntersectionSegmentByInterval(unsigned int interval)
   {
	vector<IntersectionSegment*> result;
	
	multimap<unsigned int, IntersectionSegment*>::iterator iter;
for (iter = intSegsToInterval.begin(); iter != intSegsToInterval.end(); iter++)
	{
		cout << "key=" << (*iter).first << endl;
		if((*iter).first == interval)
		{
			cout << "adding IntersectionSegment\n";
			result.push_back((*iter).second);
		}
		else
			cout << "skipping IntersectionSegment\n";
	}
	return result;
   }

private:

    enum TouchMode {LEFT, RIGHT, BOTH, NONE};
    bool IntersectionPlaneSegment(const Segment3D& seg, Point3D& result);
    bool Isintersectedby(const Segment3D seg, Segment3D& resultseg);
    TouchMode LiesOnBorder(const Segment3D& seg);
    TouchMode BorderLiesOn(PFace& pf);
    void AddIntSeg(const Segment3D& seg, Angle ang, Direction dir);
    void AddHorizontalIntSeg(const Segment3D& seg, Angle ang, Direction dir);
    void SetInitialStartPointIsA();
    void ComputeNormalVector();
    void ComputeVerticalVector();
    void ComputeWVector();
    void ComputeAngle();
    void FillIntSegIntervallList(unsigned int intervalNo);
        
    inline bool AEqualsB() const {
        return (is == ie);
    }

    inline bool InitialStartPointIsA() const {
        return initialStartPointIsA;
    }

    SourceUnit2* const unit;
    Point2D is, ie, fs, fe;
    bool insideAbove;

    unsigned int cycleNo;
    unsigned int faceNo;
    Vector3D normalVector;
    Vector3D verticalVector;
    Vector3D wVector;
    bool initialStartPointIsA;
    IntSegContainer intSegs;
    vector<IntersectionSegment*> myIntSegs;
    IntSegContainer horizontalIntSegs;
    Angle angle;

    unsigned int debugId;

//  alle Intersektionsegmente zurück gegeben die 
//  für die Zeitscheibe Key relevant sind
//  die Liste gilt pro pface dauerhaft
    multimap<unsigned int, IntersectionSegment*> intSegsToInterval;
    
//  Menge nach w-Koordinate geordnet
//  wird pro Zeitscheibe neu berechnet, sortiert nach IntSegWCompare
    set<IntersectionSegment*, IntSegWCompare> intSegIntervalList;
};

}
#endif
