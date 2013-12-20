/*
----
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, 
Faculty of Mathematics and Computer Science,
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

[1] Header File of the Region2Algebra

September 2013, first implementation by Oliver Feuer for bachelor thesis

[TOC]

1 Overview

This header file contains the definition of the class ~Region2~ which correspond
to the memory representation for the type constructor ~region2~.

1 Defines and includes

*/
#ifndef __REGION2_ALGEBRA_H__
#define __REGION2_ALGEBRA_H__

#include <limits>
#include <gmp.h>
#include <gmpxx.h>
#include "SpatialAlgebra.h"
#include "Region2Tools.h"
#include "Region2Points.h"
#include "Region2HalfSegments.h"

/*
1 Class ~Region2~

A region is composed of a set of faces. Each face consists of a set of cycles which correspond to an outer
cycle and a groups of holes (inner cycles).

A ~Region2~ value is a set of halfsegments. In the external (nested list) representation, a ~region2~
value is expressed as a set of faces, and each face is composed of a set of cycles.  However, in the
internal (class) representation, it is expressed as a set of sorted halfsegments, which are stored
in persistent DbArrays.

*/
class Region2 : public StandardSpatialAttribute<2>
{
  public:
/*
1.1 Constructors and Destructor

This constructor should not be used.

*/
    inline Region2() {}
/*
Constructs an empty region allocating space for ~n~ halfsegments.

*/
    explicit inline Region2( const int n );
/*
The copy constructor. If the flag ~onlyLeft~ is set, then only the halfsegments with left
dominating point are copied.

*/
    Region2( const Region2& r, bool onlyLeft = false );
/*
Creates a region2 object from a region object.

*/
    explicit Region2( const Region& r, const int sFactor = 0 );
/*
Creates a rectangular region from a rect2 object.

*/
    explicit Region2( const Rectangle<2>& r, const int sFactor = 0 );

/*
This function should be called before the destructor if one wants to destroy the
persistent array of halfsegments. It marks the persistent array for destroying. 
The destructor will perform the real destroying.

*/
    inline void Destroy();
/*
The destructor.

*/
    inline ~Region2() {}

/*
1.1 Functions for Bulk Load of halfsegments

As said before, the region is implemented as an ordered array of halfsegments.
The time complexity of an insertion operation in an ordered array is $O(n)$, where ~n~
is the number of halfsegments. In some cases, bulk load of segments for example, it is good
to relax the ordered condition to improve the performance. We have relaxed this ordered
condition only for bulk load of halfsegments. All other operations assume that the set of
halfsegments is ordered.

Returns whether the set of halfsegments is ordered. There is a flag ~ordered~ 
in order to avoid a scan in the halfsegments set to answer this question.

*/
    inline bool IsOrdered() const
    {
      return ordered;
    }

/*
Marks the begin of a bulk load of halfsegments relaxing the condition that the halfsegments
must be ordered.

*/
    void StartBulkLoad();
/*
Marks the end of a bulk load and sorts the halfsegments set if the argument ~sort~ is set to true.
The additional parameters can switch off the steps of this method, if not necessarily to do.

*/
    void EndBulkLoad( bool sort = true,
                      bool setCoverageNo = true,
                      bool setPartnerNo = true,
                      bool computeRegion = true,
                      bool buildDb = true );
/*
Operator and methods for adding halfsegments to the region.
These methods are used in the Bulk Load of halfsegments.
Different versions exists according the data that defines the halfsegemnt.

*/
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
    
/*
1.1 Member functions

Returns the bounding box of the region.

*/
    inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0 ) const
    {
      return bbox;
    }
/*
Sets the scalefactor of the ~region2~ value.

*/
    inline bool SetScaleFactor(int factor, bool buildDb = true)
    {
      if ( scaleFactor != factor) 
      {
        int maxInt = max(maxIntx, maxInty);
        int minInt = min(minIntx, minInty);
        if (factor > scaleFactor 
            && checkFactorOverflow(maxInt, minInt+1, factor-scaleFactor))
            return false;

        if (precHSvectorIsEmpty()) buildHSvector(); 

        if (factor > scaleFactor 
            && checkFactorOverflow(maxInt+1, minInt, factor-scaleFactor)
            && (overflowAsInt(maxPrecx, minPrecx, factor)
             || overflowAsInt(maxPrecy, minPrecy, factor)))
          return false;
        
        scaleFactor = factor;
        if (buildDb) buildDbArrays();
      }
      return true;
    }
/*
Returns the scalefactor of the ~region2~ value.

*/
    inline int GetScaleFactor() const
    {
      assert( IsDefined() );
      return scaleFactor;
    }
/*
Sets the number of components with the given argument value ~nc~.

*/
    inline void SetNoComponents( int nc ) 
    {
      noComponents = nc;
    }
/*
Sorts the dynamic array of halfsegments to the logical order of the halfsegements
using ~Reg2PreciseHalfSegment.LogicCompare~.

*/
    void LogicSort();
/*
Clears all DbArrays and the dynamic array of the halfsegments.

*/
    void Clear();
/*
Sets the region value to an empty, but defined object.

*/
    void SetEmpty();
/*
Returns whether the ~region2~ value is empty.

*/
    inline bool IsEmpty() const
    {
      bool rb = !IsDefined() || (Size() == 0);
      return rb;
    }
/*
Returns whether the dynamic array is empty.

*/
    inline bool precHSvectorIsEmpty() const
    {
      return precHSvector.size()==0;
    }  
/*
Sets the partner number of the halfsegements, this function requires 
the edgenumbers set correctly.

*/
    void SetPartnerNo();
/*
Get cycle direction function

This function determine the direction of a region's cycle that was typed by
the user. It is returned true if the cycle is clockwise (the enclosed part is on
the right) or false if the cycle is counterclockwise (the cycle has the enclosed
part on the left). 

*/
    bool GetCycleDirection() const;
/*
Compute region function

This function computes a region from a list of halfsegments, in other orders
it sets the face number, cycle number and edge number of the half segments.
It calls the function ~GetNewFaceNo~ in order to get a new face number, and
it calls the function compute cycle to compute the cycle and edge numbers.
There are two pre-requisite: the partner number of the half segments must be already set,
and they need to be ordered in the half segment order.

*/
    void ComputeRegion();
/*
Compute cycle function

This function sets the cycle and edge number of a face's cycle.

*/
    void ComputeCycle( Reg2PreciseHalfSegment &hs, int faceno, int cycleno, 
                       int &edgeno, bool *cycle );
/*
Is CriticalPoint function returns if ~adjacentPoint~ is a critical point.

*/
    bool IsCriticalPoint( const Reg2PrecisePoint &adjacentPoint, 
                          const int hsPosition ) const;
/*
Get adjacent half segment function

This function is used by ~ComputeCycle~ and returns the adjacent halfsegment of hs 
that hasn't set the cycle and edge numbers yet. It also returns the point that belongs 
to both halfsegments, and also if this point is a critical one.

*/
    bool GetAdjacentHS( const Reg2PreciseHalfSegment &hs, 
                        const int hsPosition, int &position, 
                        const int partnerno, const int partnernoP, 
                        Reg2PreciseHalfSegment& adjacentCHS, 
                        const Reg2PrecisePoint &adjacentPoint, 
                        Reg2PrecisePoint &newAdjacentPoint, bool *cycle, 
                        int step) const;
/*
Get new face number function

This function finds the face number for a cycle. It finds it the cycle
is a hole of an existing face, or if it is a cycle of a new face.

*/
    int GetNewFaceNo(const Reg2PreciseHalfSegment& hsIn, 
                     const int startpos) const;
/*
Builds the DbArrays from the precHSvector.

*/
    void buildDbArrays();
/*
Builds the precHSvector from the DbArrays.

*/
    void buildHSvector();
/*
Reads the ith halfsegment from the ~region2~ value.

*/
    bool Get(const unsigned int i, Reg2PreciseHalfSegment& hs) const;
/*
Return the internal arrays containing the halfsegments for read-only access.

*/


    const DbArray<Reg2GridHalfSegment>* getgridCoordinates() 
    { return &gridCoordinates; }
    
    const DbArray<Reg2PrecHalfSegment>* getprecCoordinates() 
    { return &precCoordinates; }
    
    const DbArray<unsigned int>* getpreciseCoordinates() 
    { return &preciseCoordinates; }


/*
Checks if the factor ~f~ causes an interger overflow.

*/
    inline bool factorOverflow(const double f)
    {
      if (checkFactorOverflow(max(maxIntx, maxInty), 
                              min(minIntx, minInty)+1, f))
      {
        cerr << "Overflow of Int values: Factor " << f 
             << " is too big!" << endl;
        return true;
      }

      return false;
    }
/*
Checks if the factors ~xf~ and ~yf~ causes an interger overflow.

*/
    inline bool factorOverflow(const double xf, const double yf)
    {
      if (checkFactorOverflow(maxIntx, minIntx+1, xf))
      {
        cerr << "Overflow of Int values: x-Factor " << xf 
             << " is too big!" << endl;
        return true;
      }
      if (checkFactorOverflow(maxInty, minInty+1, yf))
      {
        cerr << "Overflow of Int values: y-Factor " << yf 
             << " is too big!" << endl;
        return true;
      }

      return false;
    }
    
/*
1.1 Functions for use as StandardSpatialAttribute.

*/
    static const string BasicType()
    {
      return "region2";
    }

    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }

    static void* Cast(void* addr) 
    {
      return (new (addr) Region2);
    }

    inline int Size() const
    {
      if (gridCoordinates.Size() > 0)
        return gridCoordinates.Size();
      if (precHSvector.size() > 0)
        return precHSvector.size();
      return 0;
    }

    int Compare( const Attribute *arg ) const;

    ostream& Print( ostream &os ) const;

    inline int NumOfFLOBs() const
    {
      return 3;
    }

    inline Flob *GetFLOB( const int i )
    {
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
    
    inline bool Intersects(const Rectangle<2>& r, 
                           const Geoid* geoid=0 ) const
    {
    //not yet implemented
            return false;
    }
    
/*
1.1 Operator functions

The assignement operator redefinition.

*/
    Region2& operator=( const Region2& r);
/*
Decides whether two regions intersect with each other with any kind of
intersection.

*/
    bool Intersects( const Region2& r ) const;
/*
Computes whether the region is inside the region ~r~.

*/
    bool Inside( const Region2& r ) const;
/*
Computes whether the region is adjacent to the region ~r~.

*/
    bool Adjacent( const Region2& r ) const;
/*
Computes whether the region overlaps with the region ~r~.

*/
    bool Overlaps( const Region2& r ) const;
/*
Funtion ~Distance~ (with ~rect2~) is only implemented for compatibility

*/
    inline double Distance( const Rectangle<2>& r, 
                            const Geoid* geoid = 0) const
    {
    //not yet implemented
            return 0.0;
    }
/*
~Components~ returns the faces of the region in the regions vector. 
The pointers inside the array ~components~ are here initialized and 
must be deleted outside.

*/
    void Components( vector<Region2*>& components );
/*
Operation ~no\_components~

*/
    inline int NoComponents() const
    {
      assert( IsDefined() );
      return noComponents;
    }
/*
~getHoles~ returns all holes of the region as another region.

*/
    void getHoles(Region2& result) const;
/*
This operation returns all faces of the region without the holes as another region.

*/
    void getFaces(Region2& result) const;
/*
Computes whether the region contains the point ~p~.

*/
    bool Contains( const Reg2PrecisePoint& p) const;
/*
Computes whether the region contains the point ~p~ and ~p~ is not part of any border.

*/
    bool InnerContains( const Reg2PrecisePoint& p) const;
/*
Computes whether the region contains the halfsegment ~hs~.

*/
    bool Contains( const Reg2PreciseHalfSegment& hs) const;
/*
Computes whether the region contains the halfsegment ~hs~ and ~hs~ is not part of any border.

*/
    bool InnerContains( const Reg2PreciseHalfSegment& hs) const;
/*
Computes whether any hole edge of the region contains the halfsegment ~hs~.

*/
    bool HoleEdgeContain( const Reg2PreciseHalfSegment& hs ) const;
/*
Moves the region according ~x~ and ~y~ and stores the result in ~result~.

*/
    void Translate(const double& x, const double& y, Region2& result) const;
/*
Moves this region according ~x~ and ~y~.

*/
    bool Translate(const double& x, const double& y)
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
           if ( overflowAsInt(hs.GetLeftPoint().x, 
                              hs.GetRightPoint().x, scaleFactor) )
           {
             cerr << "Overflow of Int values: x-translation with value " 
                  << x << " is too big!" << endl;
             return false;
           }
           if ( overflowAsInt(hs.GetLeftPoint().y, 
                              hs.GetRightPoint().y, scaleFactor) )
           {
             cerr << "Overflow of Int values: y-translation with value " 
                  << y << " is too big!" << endl;
             return false;
           }
       }

       buildDbArrays();
       return true;
    }
/*
Returns the region's area. The region must be defined!

*/
    double Area() const;
/*
The function ~validateRegion2~ checks whether a ~region2~ value is valid.
A valid region must satisfy the following conditions:

1) Any two cycles of the same region must be disconnected, which means that no edges
of different cycles can intersect each other;

2) Edges of the same cycle can only intersect in their endpoints, but not in their middle points;

3) For a certain face, the holes must be inside the outer cycle;

4) For a certain face, any two holes can not contain each other;

5) Faces must have the outer cycle, but they can have no holes;

6) For a certain cycle, any two vertex can not be the same;

7) Any cycle must be made up of at least 3 edges;

8) It is allowed that one face is inside another provided that their edges do not intersect.

9) Outer cycles of different faces may have euqal edges.

*/
    bool validateRegion2();
/*
The static attribute ~outType~ defines the type of the nested list format for the output function.
The method ~SetOutType~ is used by the operator to switch betweeen the two types.

*/
    static bool SetOutType(int i);
    static int outType;

private:
/*
1.1 Private atrtibutes

The scalefactor of the ~region2~ value.

*/
    int scaleFactor;
/*
The database arrays of the halfsegments representation.

*/
    DbArray<Reg2GridHalfSegment> gridCoordinates;
    DbArray<Reg2PrecHalfSegment> precCoordinates;
    DbArray<unsigned int> preciseCoordinates;
/*
The bounding box that encloses the region.

*/
    Rectangle<2> bbox;
/*
The number of components resp. faces of the region.

*/
    int noComponents;
/*
Whether the half segments in the region value are sorted.

*/
    bool ordered;
/*  
Memory representation of the ordered set of precise halfsegments in a dynamic array.
 
*/  
    vector<Reg2PreciseHalfSegment> precHSvector;
/*
The values ~minIntx~, ~maxIntx~, ~minInty~ and ~maxInty~ are boundaries of the 
representation and used to decide an integer overflow. 

*/
    int maxIntx, maxInty, minIntx, minInty;
/*
The same with the values ~minPrecx~, ~maxPrecx~, ~minPrecy~ and ~maxPrecy~.

*/
    mpq_class maxPrecx, maxPrecy, minPrecx, minPrecy;
    
};


/*
1 Class ~SCycle2~

This class is used to store the information needed for cycle computation which sets the face number,
cycle number and edge number of the halfsegments.

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
      if (criticalPoint!=NULL){
        delete criticalPoint;
        criticalPoint=NULL;
      }
    }
};


/*
1 Implementation of inline functions of class ~Region2~.

*/
inline Region2::Region2( const int initsize ) :
StandardSpatialAttribute<2>(true),
scaleFactor(0),
gridCoordinates( initsize ),
precCoordinates( initsize ),
preciseCoordinates( 0 ),
bbox( false ),
noComponents( 0 ),
ordered( true ),
maxIntx(0),
maxInty(0),
minIntx(0),
minInty(0),
maxPrecx(0),
maxPrecy(0),
minPrecx(0),
minPrecy(0)
{
}

inline void Region2::Destroy()
{
  gridCoordinates.destroy();
  precCoordinates.destroy();
  preciseCoordinates.destroy();
}

ListExpr OutRegion2( ListExpr typeInfo, Word value );
Word InRegion2( const ListExpr typeInfo, const ListExpr instance, 
                const int errorPos, ListExpr& errorInfo, bool& correct );

#endif // __REGION2_ALGEBRA_H__

