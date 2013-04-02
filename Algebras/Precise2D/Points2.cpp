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

#include "Points2.h"
#include "../Rectangle/RectangleAlgebra.h"

namespace p2d {


Points2::Points2(const bool def, Point2* p) :
		StandardSpatialAttribute<2>(def),
		preciseCoordinates(0), pointsData(0),
		ordered(true), 	bbox(false){
	PointData pd(p->getGridX(), p->getGridY());
	pd.CopyPreciseCoordinates(p, &preciseCoordinates);
	pointsData.Append(pd);
}

Points2::Points2(const Points2& l) :
		StandardSpatialAttribute<2>(l.IsDefined()),
		preciseCoordinates(l.preciseCoordinates.getSize()),
		pointsData(l.pointsData.Size()),
		ordered(l.ordered),
		bbox(l.bbox){
	preciseCoordinates.copyFrom(l.preciseCoordinates);
	pointsData.copyFrom(l.pointsData);
}

Points2::Points2(bool def) :
		StandardSpatialAttribute<2>(def), preciseCoordinates(0),
		pointsData(0), ordered(false), bbox(false) {
}

int Points2::getGridX(int i) const {
	assert(0 <= i && i < pointsData.Size());
	PointData pd;
	pointsData.Get(i, &pd);
	return pd.getGridX();
}

int Points2::getGridY(int i) const {
	assert(0 <= i && i < pointsData.Size());
	PointData pd;
	pointsData.Get(i, &pd);
	return pd.getGridY();
}

mpq_class& Points2::getPreciseX(int i) const {
	assert(0 <= i && i < pointsData.Size());
	PointData pd;
	pointsData.Get(i, &pd);
	mpq_class* value = new mpq_class(0);
	pd.getPreciseX(&preciseCoordinates, *value);
	return *value;
}

mpq_class& Points2::getPreciseY(int i) const {
	assert(0 <= i && i < pointsData.Size());
	PointData pd;
	pointsData.Get(i, &pd);
	mpq_class* value = new mpq_class(0);
	pd.getPreciseY(&preciseCoordinates, *value);
	return *value;
}

char* Points2::getPreciseXAsString(int i) const {
	assert(0 <= i && i < pointsData.Size());
	PointData pd;
	pointsData.Get(i, &pd);
	return pd.getPreciseXAsString(&preciseCoordinates);
}

char* Points2::getPreciseYAsString(int i) const {
	assert(0 <= i && i < pointsData.Size());
	PointData pd;
	pointsData.Get(i, &pd);
	return pd.getPreciseYAsString(&preciseCoordinates);
}

PointData Points2::getPoint(int i)const{
	assert(0 <= i && i < pointsData.Size());
	PointData pd;
	pointsData.Get(i, &pd);
	return pd;
}

void Points2::addPoint(Point2* p){
	PointData pd(p->getGridX(), p->getGridY());
	pd.CopyPreciseCoordinates(p, &preciseCoordinates);
	pointsData.Append(pd);
}

int Points2::Size() const {
	return pointsData.Size();
}

void Points2::StartBulkLoad() {
	ordered = false;
}

/*
 ~EndBulkLoad~

 Finishs the bulkload for a points-object.

 The parameter ~sort~ can be set to __false__ if the Points are
 already ordered using the Point order.

 The parameter ~remDup~ can be set to __false__ if there are no duplicates.

*/

void Points2::EndBulkLoad(const bool sort /* = true */,
		const bool remDup /* = true */, const bool trim /* = false */
		) {
	  if( !IsDefined() ) {
	    Clear();
	    SetDefined( false );
	  }

	  if( sort ){
	    Sort();
	  }
	  else{
	    ordered = true;
	  }
	  if( remDup ){
	    RemoveDuplicates();
	  }
	  if(trim){
	    pointsData.TrimToSize();
	  }
}

void Points2::Clear()
{
  pointsData.clean();
  preciseCoordinates.clean();
  ordered = true;
  bbox.SetDefined( false );
  SetDefined(true);
}

/*
 ~ComparePoints~

 Compares both points

*/
int Points2::ComparePoints(const PointData* p1,
		const PointData* p2) const {
	//compare the  x-coordinates
	int gridX1 = p1->getGridX();
	int gridX2 = p2->getGridX();
	if (gridX1>gridX2){
		return 1;
	}
	if (gridX1<gridX2){
		return -1;
	}
	mpq_class pX1(0);
	//mpq_class pX1 = p1->getPreciseX(&preciseCoordinates);
	p1->getPreciseX(&preciseCoordinates, pX1);
	mpq_class pX2(0);
	p2->getPreciseX(&preciseCoordinates, pX2);
	//mpq_class pX2 = p2->getPreciseX(&preciseCoordinates);
	int c=cmp(pX1, pX2);
	if (c!=0){
		return c;
	}

	//compare the y-coordinates
	int gridY1 = p1->getGridY();
	int gridY2 = p2->getGridY();
	if (gridY1>gridY2){
		return 1;
	}
	if (gridY1<gridY2){
		return -1;
	}
	mpq_class pY1(0);
	p1->getPreciseY(&preciseCoordinates, pY1);
	mpq_class pY2(0);
	p2->getPreciseY(&preciseCoordinates, pY2);
	return cmp(pY1, pY2);
}

/*
 ~ComparePoints2~

 Compares both points. The second argument is a point of another points object
 than ~this~. For this, the third argument is needed.

*/
int Points2::ComparePoints2(const PointData& p1,
		const PointData& p2, const Flob* preciseCoordOfP2) const {
	//compare the  x-coordinates
	int gridX1 = p1.getGridX();
	int gridX2 = p2.getGridX();
	if (gridX1>gridX2){
		return 1;
	}
	if (gridX1<gridX2){
		return -1;
	}
	mpq_class pX1(0);
	p1.getPreciseX(&preciseCoordinates, pX1);
	mpq_class pX2(0);
	p2.getPreciseX(preciseCoordOfP2, pX2);
	int c=cmp(pX1, pX2);
	if (c!=0){
		return c;
	}

	//compare the y-coordinates
	int gridY1 = p1.getGridY();
	int gridY2 = p2.getGridY();
	if (gridY1>gridY2){
		return 1;
	}
	if (gridY1<gridY2){
		return -1;
	}
	mpq_class pY1(0);
	p1.getPreciseY(&preciseCoordinates, pY1);
	mpq_class pY2(0);
	p2.getPreciseY(preciseCoordOfP2, pY2);
	return cmp(pY1, pY2);
}

/*
 ~Sort~

 Sorts the points with Mergesort

*/
void Points2::Sort() {
	assert(!IsOrdered());
	int sz = pointsData.Size();
	if (sz > 1) {
		MergeSort(0, sz - 1);
	}
	ordered = true;
}

/*
 ~MergeSort~

*/
void Points2::MergeSort(int startindex, int endindex) {
	if (startindex < endindex) {
		int divide = floor(((double) (startindex + endindex)) / 2.0);
		MergeSort(startindex, divide);
		MergeSort(divide + 1, endindex);
		Merge(startindex, divide, endindex);
	}
}

/*
 ~Merge~

 This function merges 2 sorted sequences stored in the DbArray ~segentData~.
 The first sequence starts with ~startindex~ and ends with ~divide~ and the
 second sequence starts with ~divide~+1 and ends with ~endindex~. Both
 sequences will be edited in parallel and the sorted result will be stored
 in the DbArray starting at index ~startindex~.

*/
void Points2::Merge(int startindex, int divide, int endindex) {
	int startL = startindex;
	int startR = divide + 1;
	PointData p1, p2;
	 // The number of elements in both sequences
	int elemNo = (endindex - startindex) + 1;
	PointData result[elemNo];
	int index = 0;
	pointsData.Get(startL, &p1);
	pointsData.Get(startR, &p2);
	while (index <= elemNo) {
		if (ComparePoints(&p1, &p2) <= 0) {
			// The current element in the first sequence is less
			// than or equal to the current element in the second
			// sequence and will be stored in result.
			result[index] = p1;
			index++;
			startL++;
			if (startL <= divide) {
				pointsData.Get(startL, &p1);
			} else {
			// the first sequence is completely stored in result.
				break;
			}
		} else {
		// The current element in the first sequence is greater
		// than the current element in the second sequence. The
		// element of the second sequence will be stored in result.
			result[index] = p2;
			index++;
			startR++;
			if (startR <= endindex) {
				pointsData.Get(startR, &p2);
			} else {
				// the second sequence is completely stored in
				// result.
				break;
			}
		}
	}
	while (startL<=divide){
		// the second sequence is completely stored in result, but there
		// are still some elements in the first sequence.
		pointsData.Get(startL, &p1);
		result[index]=p1;
		index++;
		startL++;
	}
	while (startR<=endindex){
		// the first sequence is completely stored in result, but
		// there are still some elements in the second sequence.
		pointsData.Get(startR, &p2);
		result[index]=p2;
		index++;
		startR++;
	}

	// Storing the merged sequences in the DbArray.
	for ( int i=0; i < elemNo; i++) {
		pointsData.Put(startindex + i, result[i]);
	}
}

void Points2::RemoveDuplicates(){
	assert(IsOrdered());
	 DbArray<PointData> allPoints(pointsData.Size());
	 PointData p, lastPoint;
	 for(int i=0;i<pointsData.Size();i++){
	    pointsData.Get(i,p);
	    if (i>0){
	    	PointData pt;
	    	if (ComparePoints(&p, &lastPoint)!=0){
	  	      allPoints.Append(p);
	    		lastPoint=p;
	    	}
	    } else {
	    	lastPoint=p;
		    allPoints.Append(p);
	    }
	 }
	 if(allPoints.Size()!=Size()){
	     pointsData.clean();
	     for(int i=0;i < allPoints.Size(); i++){
	        allPoints.Get(i,p);
	        pointsData.Append(p);
	     }
	 }
	 allPoints.destroy();
}

/*
 4.4 ~Sizeof~-function

*/
inline size_t Points2::Sizeof() const {
	return (sizeof(*this));

}

/*
4.4 ~HashValue~-function

*/
inline size_t Points2::HashValue() const {
	  if( IsEmpty() ){
	    return 0;
	  }
	  size_t h = 0;

	  PointData p;
	  int x, y;

	  for( int i = 0; i < Size() && i < 5; i++ )
	  {
	    pointsData.Get( i, p );
	    x = p.getGridX();
	    y = p.getGridY();

	    h += (size_t)( (5 * x + y) );
	  }
	  return h;
}

/*
4.4 ~CopyFrom~-function

*/
inline void Points2::CopyFrom(const Attribute* right) {
	const Points2* p = (const Points2*) right;
	SetDefined(p->IsDefined());
	if (IsDefined()) {
		preciseCoordinates.copyFrom(p->preciseCoordinates);
		pointsData.copyFrom(p->pointsData);
	}
}

/*
4.4 ~Compare~-function

*/
inline int Points2::Compare(const Attribute *arg) const {
	const Points2 &p = *((const Points2*) arg);

	if (!IsDefined() && !p.IsDefined()) {
		return 0;
	}
	if (!IsDefined()) {
		return -1;
	}
	if (!p.IsDefined()) {
		return 1;
	}

	if (Size() > p.Size())
		return 1;
	if (Size() < p.Size())
		return -1;

	if (Size() == 0) {
		return 0;
	}

	int index = 0;
	int result = 0;

	while (index < Size()) {
		PointData p1;
		PointData p2;
		pointsData.Get(index, p1);
		p2 = p.getPoint(index);
		result = ComparePoints2(p1, p2, p.getPreciseCoordinates());
		if (result != 0) {
			return result;
		}
		index++;
	}
	return 0;
}

/*
 4.4 ~ClonePoints2~-function

*/
Word Points2::ClonePoints2(const ListExpr typeInfo, const Word& w) {
	return SetWord(new Points2(*((Points2 *) w.addr)));
}

/*
 4.4 ~CastPoints2~-function

*/
void* Points2::CastPoints2(void* addr) {
	return (new (addr) Points2());
}

/*
 4.4 ~SizeOfPoints2~-function

*/
int Points2::SizeOfPoints2() {
	return sizeof(Points2);
}

/*
 4.4 ~CreatePoints2~-function

*/
Word Points2::CreatePoints2(const ListExpr typeInfo) {
	return SetWord(new Points2(false));
}

/*
 4.4 ~DeletePoints2~-function

*/
void Points2::DeletePoints2(const ListExpr typeInfo, Word& w) {
	Points2 *p = (Points2 *) w.addr;
	(p->GetFLOB(0))->destroy();
	(p->GetFLOB(1))->destroy();
	p->SetDefined(false);
	delete p;
	w.addr = 0;
}

/*
 4.4 ~ClosePoints2~-function

*/
void Points2::ClosePoints2(const ListExpr typeInfo, Word& w) {
	delete (Points2*) w.addr;
	w.addr = 0;
}

/*
 4.4 ~Points2Property~-function

*/
ListExpr Points2::Points2Property() {
	ListExpr pointsRepr = nl->TextAtom();
	nl->AppendText(pointsRepr,
			"(<point>*), where <point> is (<x> <x> ( <px> <py>)))");

	return nl->TwoElemList(
		nl->FourElemList(nl->StringAtom("Signature"),
			nl->StringAtom("Example Type List"),
			nl->StringAtom("List Rep"),
			nl->StringAtom("Example List")),
			nl->FourElemList(nl->StringAtom("-> DATA"),
			nl->StringAtom(Line2::BasicType()),
				pointsRepr,
				nl->StringAtom
				("((1 1 ('1/4' '3/4'))(3 4 ('1/8' '1/10')))")));
}

/*
 4.4 ~CheckPoints2~-function

*/
bool Points2::CheckPoints2(ListExpr type, ListExpr& errorInfo) {
	return (nl->IsEqual(type, Points2::BasicType()));
}

/*
 4.4 ~BoundingBox~-function

*/
const Rectangle<2> Points2::BoundingBox(const Geoid* geoid /*= 0*/) const {
	return bbox;
}

/*
 4.4 ~Distance~-function

*/
double Points2::Distance(const Rectangle<2>& rect,
		const Geoid* geoid/*=0*/) const {
	  return 0.0;
}

/*
 4.4 ~Empty~-function

*/
bool Points2::IsEmpty() const {
	return !IsDefined();
}

} // end of namespace p2d


