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

[1] Header File of the Spatial2 Algebra

May 2013, Oliver Feuer


[TOC]

1 Overview

This header file essentially contains the definition of the classes ~Point~,
~Points~, ~Line~, and ~Region~ used in the Spatial Algebra. These classes
respectively correspond to the memory representation for the type constructors
~point~, ~points~, ~line~, and ~region~.  Figure \cite{fig:spatialdatatypes.eps}
shows examples of these spatial data types.

2 Defines and includes

*/
#ifndef __SPATIAL2_ALGEBRA_H__
#define __SPATIAL2_ALGEBRA_H__

#include <limits>
#include <gmp.h>
#include <gmpxx.h>
#include "SpatialAlgebra.h"
#include "Region2Tools.h"
#include "Region2Points.h"
#include "Region2HalfSegments.h"

class Region2 : public StandardSpatialAttribute<2>
{
  public:

    explicit inline Region2( const int n );
    Region2( const Region2& r, bool onlyLeft = false );
    explicit Region2( const Region& r );
    explicit Region2( const Rectangle<2>& r );

    inline void Destroy();
    inline ~Region2() {}

    void StartBulkLoad();
    void EndBulkLoad( bool sort = true,
                      bool setCoverageNo = true,
                      bool setPartnerNo = true,
                      bool computeRegion = true,
                      bool buildDb = true );

    bool Contains( const Reg2PrecisePoint& p) const;
    bool InnerContains( const Reg2PrecisePoint& p) const;
    bool Contains( const Reg2PreciseHalfSegment& hs) const;
    bool InnerContains( const Reg2PreciseHalfSegment& hs) const;
    bool HoleEdgeContain( const Reg2PreciseHalfSegment& hs ) const;

    
    Region2& operator=( const Region2& r);
    Region2& operator+=( const Reg2PreciseHalfSegment& hs );
    Region2& AddPreciseHalfsegment( const bool ldp, 
                                    const Reg2PrecisePoint& pl, 
                                    const Reg2PrecisePoint& pr);
    Region2& AddPreciseHalfsegment( const bool ldp, 
				    const int xl, const mpq_class xlp, 
                                    const int yl, const mpq_class ylp, 
				    const int xr, const mpq_class xrp, 
                                    const int yr, const mpq_class yrp, 
				    const int scale = 0);

    bool Intersects( const Region2& r ) const;
    bool Inside( const Region2& r ) const;
    bool Adjacent( const Region2& r ) const;
    bool Overlaps( const Region2& r ) const;
    void Translate(const double& x, const double& y, Region2& result) const;

    void Translate(const double& x, const double& y)
    {
       double t[2];
       t[0] = x;
       t[1] = y;
       bbox = bbox.Translate(t);
       int size = Size();
       Reg2PreciseHalfSegment hs;
       for(int i=0;i<size;i++)
       {
           Get(i,hs);
           hs.Translate(x,y);
       }

       buildDbArrays();
    }
  
    double Area() const;

    static const string BasicType()
    {
      return "region2";
    }

    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }

    static void* Cast(void* addr) 
    {
/*      cout << "Region2::Cast..." << endl;
      Region2* res = (new (addr) Region2);
      res->buildHSvector();
      return res;
*/      return (new (addr) Region2);
    }

    inline bool IsEmpty() const
    {
      bool rb = !IsDefined() || (Size() == 0);
//      cout << "Region2::IsEmpty..." << rb << endl;
      return rb;
    }

    inline bool IsOrdered() const
    {
      return ordered;
    }

    inline int Size() const
    {
      if (gridCoordinates.Size() > 0)
        return gridCoordinates.Size();
      if (precHSvector.size() > 0)
	return precHSvector.size();
      return 0;
    }

    inline bool precHSvectorIsEmpty() const
    {
      return precHSvector.size()==0;
    }  
      
    void LogicSort();
    void Clear();
    void SetEmpty();

    void SetPartnerNo();
    bool GetCycleDirection() const;

    bool IsCriticalPoint( const Reg2PrecisePoint &adjacentPoint, 
                          const int hsPosition ) const;
    bool GetAdjacentHS( const Reg2PreciseHalfSegment &hs, 
                        const int hsPosition, int &position, 
                        const int partnerno, const int partnernoP, 
                        Reg2PreciseHalfSegment& adjacentCHS, 
			const Reg2PrecisePoint &adjacentPoint, 
                        Reg2PrecisePoint &newAdjacentPoint, bool *cycle, 
                        int step) const;
    int GetNewFaceNo(const Reg2PreciseHalfSegment& hsIn, 
                     const int startpos) const;
    void ComputeCycle( Reg2PreciseHalfSegment &hs, int faceno, int cycleno, 
                       int &edgeno, bool *cycle );
    void ComputeRegion();
    void buildDbArrays();
    void buildHSvector();
    bool validateRegion2();
    
    int Compare( const Attribute *arg ) const;

    ostream& Print( ostream &os ) const;

    inline int NumOfFLOBs() const
    {
//      cout << "Region2::NumOfFLOBS..." << endl;
      return 3;
    }

    inline Flob *GetFLOB( const int i )
    {
//      cout << "Region2::GetFLOB..." << i << endl;
      assert ( i >= 0 && i < NumOfFLOBs() );
      if ( i == 0 ) return &gridCoordinates;
      else if ( i == 1 ) return &precCoordinates;
      else if ( i == 2 ) return &preciseCoordinates;
      else return 0;
    }

    inline void SetDefined( bool defined )
    {
        if(!defined){
          Clear();
        }
        StandardSpatialAttribute<2>::SetDefined( defined );
    }

    inline size_t Sizeof() const
    {
      return sizeof( *this );
    }

    inline bool Adjacent( const Attribute* arg ) const
    {
      return false;
    }

    virtual Region2 *Clone() const;

    size_t HashValue() const;

    void CopyFrom( const Attribute* right );
    
    inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0 ) const
    {
      return bbox;
    }

    double Distance( const Rectangle<2>& r, const Geoid* geoid = 0) const;

    inline bool Intersects(const Rectangle<2>& rect, 
                           const Geoid* geoid=0 ) const 
    { return false; };

    
    inline void SetScaleFactor(int factor, bool buildDb = true)
    {
      if ( scaleFactor != factor) scaleFactor = factor;
      if (buildDb) buildDbArrays();
    }

    inline int GetScaleFactor() const
    {
      assert( IsDefined() );
      return scaleFactor;
    }

    inline int NoComponents() const
    {
      assert( IsDefined() );
      return noComponents;
    }

    inline void SetNoComponents( int nc ) 
    {
      noComponents = nc;
    }

    void Components( vector<Region2*>& components );
    void getFaces(Region2& result) const;
    void getHoles(Region2& result) const;
    
    inline bool Get(const unsigned int i, Reg2PreciseHalfSegment& hs) const
    {
      if (i >= 0 && i < precHSvector.size())
      {
	hs = precHSvector[i];
	return true;
      }
      else
	return false;
    }

    const DbArray<Reg2GridHalfSegment>* getgridCoordinates() 
    { return &gridCoordinates; }
    const DbArray<Reg2PrecHalfSegment>* getprecCoordinates() 
    { return &precCoordinates; }
    const DbArray<unsigned int>* getpreciseCoordinates() 
    { return &preciseCoordinates; }

    inline Region2() {}

    static bool SetOutType(int i);
    static int outType;

private:
    int scaleFactor;
    DbArray<Reg2GridHalfSegment> gridCoordinates;
    DbArray<Reg2PrecHalfSegment> precCoordinates;
    DbArray<unsigned int> preciseCoordinates;
    Rectangle<2> bbox;
    int noComponents;
    bool ordered;
    vector<Reg2PreciseHalfSegment> precHSvector;
    
};


/*
10.2 SCycle2

This class is used to store the information need for cycle computation which sets the face number,
cycle number and edge number of the half segments.

*/

class SCycle2
{
  public:
    Reg2PreciseHalfSegment hs1,hs2;
    int hs1PosRight,hs1PosLeft,
        hs2PosRight,hs2PosLeft;
    bool goToCHS1Right,goToCHS1Left,
         goToCHS2Right,goToCHS2Left;
    int hs1Partnerno,hs2Partnerno;
    Reg2PrecisePoint *criticalPoint;
    Reg2PrecisePoint nextPoint;

    SCycle2(){}

    SCycle2( const Reg2PreciseHalfSegment &hs, const int partnerno,
            const Reg2PreciseHalfSegment &hsP,const int partnernoP,
            Reg2PrecisePoint *criticalP, const Reg2PrecisePoint &nextPOINT)
    {
      hs1 = hs;
      hs2 = hsP;
      hs1PosRight = hs1PosLeft = partnernoP;
      hs2PosRight = hs2PosLeft = partnerno;
      hs1Partnerno = partnerno;
      hs2Partnerno = partnernoP;

      goToCHS1Right=goToCHS1Left=goToCHS2Right=goToCHS2Left=true;
      criticalPoint = criticalP;
      nextPoint = nextPOINT;
    }

    SCycle2(const SCycle2 &sAux)
    {
      hs1 = sAux.hs1;
      hs2 = sAux.hs2;
      hs1PosRight  = sAux.hs1PosRight;
      hs1PosLeft   = sAux.hs1PosLeft;
      hs1Partnerno = sAux.hs1Partnerno ;
      goToCHS1Right = sAux.goToCHS1Right;
      goToCHS1Left  = sAux.goToCHS1Left;

      hs2PosRight  = sAux.hs2PosRight;
      hs2PosLeft   = sAux.hs2PosLeft;
      hs2Partnerno = sAux.hs2Partnerno ;
      goToCHS2Right = sAux.goToCHS2Right;
      goToCHS2Left  = sAux.goToCHS2Left;

      criticalPoint = sAux.criticalPoint;
      nextPoint     = sAux.nextPoint;

    }

    ~SCycle2()
    {
      if (criticalPoint==NULL){
        delete criticalPoint;
        criticalPoint=NULL;
      }
    }
};



inline Region2::Region2( const int initsize ) :
StandardSpatialAttribute<2>(true),
scaleFactor(0),
gridCoordinates( initsize ),
precCoordinates( initsize ),
preciseCoordinates( 0 ),
bbox( false ),
noComponents( 0 ),
ordered( true )
{
//  cout << "Region2(int)... " << initsize << endl;
}

inline void Region2::Destroy()
{
//  cout << "Region2::Destroy... " << endl;
  gridCoordinates.destroy();
  precCoordinates.destroy();
  preciseCoordinates.destroy();
}

ListExpr OutRegion2( ListExpr typeInfo, Word value );
Word InRegion2( const ListExpr typeInfo, const ListExpr instance, 
                const int errorPos, ListExpr& errorInfo, bool& correct );

#endif // __SPATIAL2_ALGEBRA_H__

