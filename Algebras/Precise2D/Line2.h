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

#ifndef _Line2_H
#define _Line2_H

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
#include "AVL_Tree.h"
#include "SpatialAlgebra.h"

namespace p2d{

class Point2;


/*
2 Class ~SegmentData~

The class ~SegmentData~ describes the precise representation of a
segment.
The absolute coordinates of the 2 segmentpoints are stored in 4 variables of
type int.The difference between the absolute coordinates and their respective
representation will be stored in a flob. ~xLeftPos~ is the index where the
x-coordinate of the left point in this flob starts and ~xLeftNumOfChars~ is his
length (analog the y-coordinate and the right point)

*/

class SegmentData {

	/*
	2.1 Private attributes
	  * ~xLeft~, ~yLeft~, ~xRight~ and ~yRight~ is the absolute value of the
	    given coordinate

	  * ~xLeftPos~, ~yLeftPos~, ~xRightPos~, ~yRightPos~ is
		the index position of the first char representing the given
		coordinate in the flob

	  * ~xLeftNumOfChars~, ~yLeftNumOfChars~, ~xRightNumOfChars~,
	    ~yRightNumOfChars~ is the number of all chars representing this
	    respective coordinate instance in the flob.

	*/
private:
	int xLeft;
	int yLeft;
	int xRight;
	int yRight;

	size_t xLeftPos;
	size_t yLeftPos;
	size_t xRightPos;
	size_t yRightPos;
	size_t xLeftNumOfChars;
	size_t yLeftNumOfChars;
	size_t xRightNumOfChars;
	size_t yRightNumOfChars;

	/*
	 Indicates whether the left point is the dominating point or the
	 secondary point

	 */
	bool ldp;

	/*
	The face identifier

	*/
	int faceno;

	/*
	The cycle identifier

	*/
	int cycleno;

	/*
	The edge (segment) identifier

	*/
	int edgeno;

	/*
	Used for fast spatial scan of the inside[_]pr algorithm

	*/
	int coverageno;

	/*
	Indicates whether the region's area is above or left of its segment

	*/
	bool insideAbove;

	/*
	Stores the position of the partner half segment in half segment
	ordered array

	*/
	int partnerno;


public:

	/*
	 2.2 Constructors and deconstructor
	 */
/*
The default constructor does nothing

*/
	SegmentData() {}

/*
Constructor that initializes all the grid values.

*/
	SegmentData(int xLeft, int yLeft, int xRight, int yRight, bool ldp,
			int edgeNo=-999999);

	SegmentData(Point2* lp, Point2* rp, bool ldp, int edgeno,
			Flob* preciseCoordinates);
/*
 Copy-constructor

*/
	SegmentData(SegmentData& d);

/*
2.3 Attribute read access methods

For the grid values:

*/
	int getLeftX(void) const { return xLeft; }
	int getLeftY(void) const { return yLeft; }
	int getRightX(void) const { return xRight; }
	int getRightY(void) const {	return yRight; }

/*
 For the difference between the absolute coordinates and their respective
 representation:
 All these methods fetch the chars representing the given coordinate from
 the flob using the indices given in this instance's private attributes, and
 convert them to the correct instance of type mpq\_class, representing the
 value of the given coordinate.

*/
	mpq_class getPreciseLeftX(const
		Flob* preciseCoordinates) const;
	mpq_class getPreciseLeftY(const
		Flob* preciseCoordinates) const;
	mpq_class getPreciseRightX(const
		Flob* preciseCoordinates) const;
	mpq_class getPreciseRightY(const
		Flob* preciseCoordinates) const;

/*
Returns the named value of the dominating point of the half segment.

*/
	const int GetDomGridXCoord() const;
	const int GetDomGridYCoord() const;
	const mpq_class GetDomPreciseXCoord(const
	Flob* preciseCoordinates) const;
	const mpq_class GetDomPreciseYCoord(const
	Flob* preciseCoordinates) const;


/*
Returns the named value of the secondary point of the half segment.

*/
	const int GetSecGridXCoord() const;
	const int GetSecGridYCoord() const;
	const mpq_class GetSecPreciseXCoord(const
	Flob* preciseCoordinates) const;
	const mpq_class GetSecPreciseYCoord(const
	Flob* preciseCoordinates) const;

	int GetEdgeno(){ return edgeno; }
	int GetPartnerno(){ return partnerno; }
	bool GetInsideAbove(){ return insideAbove; }
	int GetCoverageno(){ return coverageno; }
	int GetCycleno(){ return cycleno; }
	int GetFaceno(){ return faceno; }
	int GetEdgeNo(){ return edgeno; }




/*
 2.4 Attribute write access methods

 For the grid values:

*/
	void SetLeftX(int lx){
		xLeft=lx;
	}

	void SetLeftY(int ly){
		yLeft=ly;
	}

	void SetRightX(int rx){
		xRight=rx;
	}

	void SetRightY(int ry){
		yRight=ry;
	}

	void SetPreciseLeftX (Flob* preciseCoordinates, mpq_class lx);
	void SetPreciseLeftY (Flob* preciseCoordinates, mpq_class ly);
	void SetPreciseRightX (Flob* preciseCoordinates, mpq_class rx);
	void SetPreciseRightY (Flob* preciseCoordinates, mpq_class ry);
	void SetPreciseLeftCoordinates(Point2* lp, Flob* preciseCoordinates);
	void SetPreciseRightCoordinates(Point2* rp, Flob* preciseCoordinates);
	void addPreciseValues(Flob* f, mpq_class pLeftX, mpq_class pLeftY,
			mpq_class pRightX, mpq_class pRightY);

/*
	Returns the boolean flag which indicates whether the dominating point is on the left side.

*/
		bool IsLeftDomPoint() const;





	void SetPartnerno(int no);
	void SetInsideAbove(bool b);
	void SetCoverageno(int no);
	void SetCycleno(int no);
	void SetFaceno(int no);
	void SetEdgeNo(int no);

/*
	Returns the length of the half segmtent, i.e., the distance between the left point to the
	right point.

*/
	double Length(Flob* f, const Geoid* geoid=0) const;

    inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;

	bool IsVertical(const Flob* preciseCoordinates) const;

};

/*
 3 Class Line2

*/
class Line2 : public StandardSpatialAttribute<2>{

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

	bool ordered;

/*
	The number of components for the line.

*/
    int noComponents;


/*
	The bounding box that fully encloses all half segments of the line.

*/
    Rectangle<2> bbox;

/*
~Sort~

Sorts the segmens using mergesort.

*/
	void Sort();

	void MergeSort(int startIndex, int endIndex);

	void Merge(int startIndex, int divide, int endIndex);

/*
	 ~SegmentIsVertical~
	 Returns true if a segment with the given x-coordinates is vertical, false otherwise.

*/
	bool SegmentIsVertical(int lx, mpq_class plx, int rx, mpq_class prx);

/*
	 ~CompareSegment~
	 Compares the two given segments. Returns
	 1	if ~seg1~ > ~seg2~
	 0	if ~seg1~ = ~seg2~
	 -1	if ~seg1~ < ~seg2~

*/
	int CompareSegment(const SegmentData& seg1,
			const SegmentData& seg2)const;

	int CompareSegment2(const SegmentData& seg1,
			const SegmentData& seg2,
			const Flob* preciseCoordOfSeg2)const;

/*
	 ~SetPartnerNo~

*/
	void SetPartnerNo();


	public:

/*
	  3.1 Constructors and deconstructor

*/
	inline Line2(){};

	Line2( const bool def, bool ldp, int xl, int yl, int xr, int yr,
				mpq_class pxl, mpq_class pyl,
				mpq_class pxr, mpq_class pyr);

	Line2(const Line2& cp);

	Line2(bool def);

	~Line2(){};


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
	 Returns the number of segments in this line2-object.

*/
	int Size()const;

/*
	 3.2.7 ~Destroy~
	 Deletes the segments

*/
	inline void Destroy();

	void collectFace(int faceno, int startPos, DbArray<bool>& used);

	int getUnusedExtension(int startPos,const DbArray<bool>& used)const;

	void computeComponents();

/*
	 3.2.8 ~StartBulkLoad~
	 Marks this line2-object as unsorted.

*/
	void StartBulkLoad();
 
/*
	 3.2.9 ~EndBulkLoad~
	 Sorts the segments, realminize them and updates the attributes.

*/
    void EndBulkLoad ( bool sort = true, bool realminize = true,
    		bool trim = true );

/*
	 3.2.10 ~unionOp~
	 Computes the union of two line2-objects

*/
    void unionOP(Line2& l2, Line2& res, const Geoid* geoid=0);

/*
	 3.2.11 ~intersection~
	 Computes the intersection of two line2-objects

*/
    void intersection(Line2& l2, Line2& res, const Geoid* geoid=0);

/*
	 3.2.12 ~minus~
	 Returns all segments and parts of segments of ~this~ which are not
	 in the ~l2~. The result will be stored in ~res~

*/
    void minus(Line2& l2, Line2& res, const Geoid* geoid=0);

/*
3.2.13 ~intersect~
Returns true, if ~this~ intersect ~l2~, false otherwise.

*/
    bool intersect(Line2& l2, const Geoid* geoid=0);

/*
3.2.14 ~crossings~
~result~ will contain all crossing-points of ~this~ and ~l2~.

*/
    void crossings(Line2& l2, Points2& result, const Geoid* geoid=0);

/*
3.3 The following functions are required by Secondo

*/

/*
3.3.1 ~BasicType~

*/
	static const string BasicType(){
		return "line2";
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
	inline Line2* Clone() const
      	{ 
		return new Line2( *this ); 
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
3.3.12 ~CloneLine2~

*/
	static Word CloneLine2( const ListExpr typeInfo,
	            const Word& w );

/*
3.3.13 ~CastLine2~

*/
	static void* CastLine2(void* addr);

/*
3.3.14 ~SizeOfLine2~

*/
	static int SizeOfLine2();

/*
3.3.15 ~OutLine2~

*/
	static ListExpr OutLine2( ListExpr typeInfo, Word value );

/*
3.3.16 ~InLine2~

*/
	static Word InLine2( const ListExpr typeInfo, const ListExpr instance,
	       const int errorPos, ListExpr& errorInfo, bool& correct );

/*
3.3.17 ~CreateLine2~

*/
	static Word CreateLine2( const ListExpr typeInfo );

/*
3.3.18 ~DeleteLine2~

*/
	static void DeleteLine2( const ListExpr typeInfo, Word& w );

/*
3.3.19 ~CloseLine2~

*/
	static void CloseLine2( const ListExpr typeInfo, Word& w );

/*
3.3.20 ~Line2Property~

*/
	static ListExpr Line2Property();

/*
3.3.21 ~CheckLine2~

*/
	static bool CheckLine2( ListExpr type, ListExpr& errorInfo );

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
void convertLineIntoLine2(Line& l, Line2& result);




} // end of namespace p2d


#endif /* _Line2_H */
