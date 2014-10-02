/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2009, University in Hagen, Faculty of Mathematics and
 Computer Science, Database Systems for New Applications.

 SECONDO is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published
 by
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

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[TOC] [\tableofcontents]

 [TOC]

 0 Overview

 1 Includes and defines

*/

#ifndef _SimpleLine2_H
#define _SimpleLine2_H

#include <stdio.h>
#include <gmp.h>
#include <gmpxx.h>
#include "Algebra.h"
#include "RectangleAlgebra.h"
#include "../../Tools/Flob/DbArray.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "AttrType.h"
#include "Precise2DAlgebra.h"
#include "Point2.h"
#include "Line2.h"
#include "AVL_Tree.h"
#include "SpatialAlgebra.h"

namespace p2d{

class Point2;
class SegmentData;


/*
 3 Class SimpleLine2

*/
class SimpleLine2 : public StandardSpatialAttribute<2>{

  private:

/*
   One coordinate consosts of an integer as a grid coordinate and a
   rational number as the difference between the real coordinate and
   the grid value. These rational numbers will be stored in the
   Flob ~preciseCoodinates~

*/
  Flob preciseCoordinates;

/*
   segmentData stores the gridvalues, the attributes of the segments and the
   indices for the precise Coordinates.

*/
  DbArray<SegmentData> segmentData;

  DbArray<LRS> lrsArray;

  bool ordered;

/*
  The number of components for the line.

*/
    bool startSmaller;

    bool isCycle;

/*
  The bounding box that fully encloses all half segments of the line.

*/
    Rectangle<2> bbox;

    int currentSD;

/*

~Sort~

Sorts the segmens using mergesort.

*/
  void Sort();

  void MergeSort(int startIndex, int endIndex);

  void Merge(int startIndex, int divide, int endIndex);

/*
   ~SegmentIsVertical~
   Returns true if a segment with the given x-coordinates is vertical, false
   otherwise.

*/
  bool SegmentIsVertical(int lx, mpq_class plx, int rx, mpq_class prx);

/*
   ~CompareSegment~
   Compares the two given segments. Returns
   1  if ~seg1~ > ~seg2~
   0  if ~seg1~ = ~seg2~
   -1 if ~seg1~ < ~seg2~

*/
  int CompareSegment(const SegmentData& seg1,
      const SegmentData& seg2)const;

  int CompareSegment2(const SegmentData& seg1,
      const SegmentData& seg2,
      const Flob* preciseCoordOfSeg2)const;

  /*
   ~Equalize~


  */
  void Equalize(const SimpleLine2& src);

  /*
  ~SetPartnerNo~

  Changes the partnerno of each HalfSegment to the index of the reverse
  segment within the halfsegmenst array.

  */
   void SetPartnerNo();

  /*
  ~computePolyline~

  This function searches for a polyline within the halfsegment array and
  creates the lrsarray. Additionally, the edge number of each segment is set
  to the corresponding entry within the lrs array. If the segments does not
  represent a simple polyLine, i.e. several components or branches, the result
  of this function will be __false__.

  */
  bool computePolyline();

  void TrimToSize();

  public:

/*
 3.1 Constructors and deconstructor

*/
  inline SimpleLine2(){};

  SimpleLine2( const bool def, bool ldp, int xl, int yl, int xr, int yr,
        mpq_class pxl, mpq_class pyl,
        mpq_class pxr, mpq_class pyr);

  SimpleLine2(const SimpleLine2& cp);

  SimpleLine2(bool def);

  ~SimpleLine2(){};

  SimpleLine2& operator=(const SimpleLine2& src);

/*
 3.2 Member-functions

*/
  void Clear();

  bool IsOrdered(){return ordered;}

/*
   3.2.1 ~getter~
   The following functions returns the asked coordinate of the
   ~i~-th segment of the line2-object.

*/
  int getLeftGridX(int i) const;

  int getLeftGridY(int i) const;

  int getRightGridX(int i) const;

  int getRightGridY(int i) const;

  mpq_class getPreciseLeftX(int i) const;

  mpq_class getPreciseLeftY(int i) const;

  mpq_class getPreciseRightX(int i) const;

  mpq_class getPreciseRightY(int i) const;

/*
   3.2.2 ~get~
   Returns the ~i~-th SegmentData-object of the line2-object.

*/
  void get(int i, SegmentData&) const;

/*
   3.2.3 ~getPreciseCoordinates~
   Returns a pointer to he flob, which stores the precise coordinates.

*/
  const Flob* getPreciseCoordinates() const{
    return &preciseCoordinates;
  }

/*
   3.2.4 ~addSegment~
   Adds a segment to the line2-object.

*/
  void addSegment(bool ldp, int leftX, int leftY, int rightX, int rightY,
    mpq_class pLeftX, mpq_class pLeftY, mpq_class pRightX,
    mpq_class pRightY, int edgeNo);

  void addSegment(bool ldp, Point2* lp, Point2* rp, int edgeno);

/*
   3.2.5 ~IsLeftDomPoint~
   Returns true if the left point of segment no ~i~ is the dominating point, false otherwise.

*/
  bool IsLeftDomPoint(int i) const;

/*
   3.2.6 ~Size~
   Returns the number of segments in this simpleLine2-object.

*/
  int Size()const;

  /*
  ~StartsSmaller~

  Returns true if SimpleLine start at smaller end point.

  */
  inline bool startsSmaller() const{
   return startSmaller;
  }


  /*
   ~setStartSmaller~
   */
  inline void setStartSmaller(bool s){
   startSmaller = s;
  }

/*
   3.2.7 ~Destroy~
   Deletes the segments

*/
  inline void Destroy();

//TODO  void collectFace(int faceno, int startPos, DbArray<bool>& used);

//TODO int getUnusedExtension(int startPos,const DbArray<bool>& used)const;

  //void computeComponents();

/*
   3.2.8 ~StartBulkLoad~
   Marks this simpleLine2-object as unsorted.

*/
  void StartBulkLoad();

/*
   3.2.9 ~EndBulkLoad~
   Sorts the segments, realminize them and updates the attributes.

*/
    bool EndBulkLoad ( bool sort = true, bool realminize = true,
        bool trim = true );


    /*
    ~StartPoint~

    Returns the start point of this simple line.

    */
      Point2 StartPoint(const bool startSmaller) const;
      Point2 StartPoint() const;

    /*
    ~EndPoint~

    Returns the end point of this simple line.

    */
      Point2 EndPoint(const bool startSmaller) const;
      Point2 EndPoint() const;
/*
   3.2.10 ~unionOp~
   Computes the union of two line2-objects

*/
 //TODO   void unionOP(Line2& l2, Line2& res, const Geoid* geoid=0);

/*
   3.2.11 ~intersection~
   Computes the intersection of two line2-objects

*/
//TODO    void intersection(Line2& l2, Line2& res, const Geoid* geoid=0);

/*
   3.2.12 ~minus~
   Returns all segments and parts of segments of ~this~ which are not
   in the ~l2~. The result will be stored in ~res~

*/
 //TODO   void minus(Line2& l2, Line2& res, const Geoid* geoid=0);

/*
3.2.13 ~intersect~
Returns true, if ~this~ intersect ~l2~, false otherwise.

*/
//TODO    bool intersect(Line2& l2, const Geoid* geoid=0);

/*
3.2.14 ~crossings~
~result~ will contain all crossing-points of ~this~ and ~l2~.

*/
  //TODO  void crossings(Line2& l2, Points2& result, const Geoid* geoid=0);

/*
3.3 The following functions are required by Secondo

*/

/*
3.3.1 ~BasicType~

*/
  static const string BasicType(){
    return "sline2";
  }

/*
3.3.2 ~checkType~

*/
  static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }

/*
3.3.3 ~Sizeof~

*/
  inline size_t Sizeof() const;

/*
3.3.4 ~HashValue~

*/
  inline size_t HashValue() const;

/*
3.3.5 ~CopyFrom~

*/
  inline void CopyFrom( const Attribute* right );

/*
3.3.6 ~Compare~

*/
  inline int Compare( const Attribute *arg ) const;

/*
3.3.7 ~Adjacent~

*/
  inline bool Adjacent( const Attribute *arg ) const
  {
    return false;
  }

/*
3.3.8 ~Clone~

*/
  inline SimpleLine2* Clone() const
        {
    return new SimpleLine2( *this );
  }

/*
3.3.9 ~Print~

*/
  ostream& Print( ostream &os ) const{
    os << *this;
    return os;
  }

/*
3.3.10 ~NumOfFLOBs~

*/
  int NumOfFLOBs(void) const{
    return 2;
  }

/*
3.3.11 ~GetFLOB~

*/
  Flob *GetFLOB(const int i){
    assert(0<=i && i<=1);
    if (i==0){
      return &preciseCoordinates;
    } else {
      return &segmentData;
    }
  }

/*
3.3.12 ~CloneSimpleLine2~

*/
  static Word CloneSimpleLine2( const ListExpr typeInfo,
              const Word& w );

/*
3.3.13 ~CastSimpleLine2~

*/
  static void* CastSimpleLine2(void* addr);

/*
3.3.14 ~SizeOfSimpleLine2~

*/
  static int SizeOfSimpleLine2();

/*
3.3.15 ~OutSimpleLine2~

*/
  static ListExpr OutSimpleLine2( ListExpr typeInfo, Word value );

/*
3.3.16 ~InSimpleLine2~

*/
  static Word InSimpleLine2( const ListExpr typeInfo, const ListExpr instance,
         const int errorPos, ListExpr& errorInfo, bool& correct );

/*
3.3.17 ~CreateSimpleLine2~

*/
  static Word CreateSimpleLine2( const ListExpr typeInfo );

/*
3.3.18 ~DeleteSimpleLine2~

*/
  static void DeleteSimpleLine2( const ListExpr typeInfo, Word& w );

/*
3.3.19 ~CloseSimpleLine2~

*/
  static void CloseSimpleLine2( const ListExpr typeInfo, Word& w );

/*
3.3.20 ~SimpleLine2Property~

*/
  static ListExpr SimpleLine2Property();

/*
3.3.21 ~CheckSimpleLine2~

*/
  static bool CheckSimpleLine2( ListExpr type, ListExpr& errorInfo );

/*
3.3.22 ~BoundingBox~

*/
  virtual const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;

/*
3.3.23 ~Distance~

*/
  virtual double Distance(const Rectangle<2>& rect,
      const Geoid* geoid=0) const;

/*
3.3.24 ~IsEmpty~

*/
  virtual bool IsEmpty() const;
};

/*
4 ~convertLineIntoLine2~

This function converts a line-object in a line2-object.

*/
//void convertLineIntoLine2(Line& l, Line2& result);




} // end of namespace p2d

#endif /* SimpleLine2_H_ */
