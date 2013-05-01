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

#ifndef _SimpleLine2_CPP
#define _SimpleLine2_CPP

#include "SimpleLine2.h"

namespace p2d {

SimpleLine2::SimpleLine2(const bool def, bool ldp, int xl, int yl,
		int xr, int yr,
		mpq_class pxl, mpq_class pyl,
		mpq_class pxr, mpq_class pyr) :
		StandardSpatialAttribute<2>(def),
		preciseCoordinates(0),
		segmentData(0),
		lrsArray(0),
		ordered(true),startSmaller(true),
		isCycle(false), bbox(false), currentSD(-1){
	SegmentData sd(xl, yl, xr, yr, ldp);
	sd.SetPreciseLeftX(&preciseCoordinates, pxl);
	sd.SetPreciseLeftY(&preciseCoordinates, pyl);
	sd.SetPreciseRightX(&preciseCoordinates, pxr);
	sd.SetPreciseRightY(&preciseCoordinates, pyr);
	segmentData.Append(sd);
	bbox = sd.BoundingBox();
}

SimpleLine2::SimpleLine2(const SimpleLine2& l) :
		StandardSpatialAttribute<2>(l.IsDefined()),
		preciseCoordinates(l.preciseCoordinates.getSize()),
		segmentData(l.segmentData.Size()),
    lrsArray(l.Size()/2),
    ordered(true),
		startSmaller(true),
		isCycle(false), bbox(false), currentSD(-1)
{
  Equalize(l);

}

SimpleLine2::SimpleLine2(bool def) :
		StandardSpatialAttribute<2>(def),
		preciseCoordinates(0),
		segmentData(0),
		lrsArray(0),
		ordered(true),
		startSmaller(true),
		isCycle(false),
		bbox(false),
		currentSD(-1){
}

  SimpleLine2& SimpleLine2::operator=(const SimpleLine2& src){
     Equalize(src);
     return *this;
  }

void SimpleLine2::Equalize(const SimpleLine2& src){
    preciseCoordinates.copyFrom(src.preciseCoordinates);
    segmentData.copyFrom(src.segmentData);
    lrsArray.copyFrom(src.lrsArray);
    this->SetDefined( src.IsDefined() );
    this->startSmaller = src.startSmaller;
    this->isCycle = src.isCycle;
    this->ordered = src.ordered;
    this->bbox = src.bbox;
    this->currentSD = src.currentSD;
}

int SimpleLine2::getLeftGridX(int i) const {
	assert(0 <= i && i < segmentData.Size());
	SegmentData sd;
	segmentData.Get(i, &sd);
	return sd.getLeftX();
}

int SimpleLine2::getLeftGridY(int i) const {
	assert(0 <= i && i < segmentData.Size());
	SegmentData sd;
	segmentData.Get(i, &sd);
	return sd.getLeftY();
}

int SimpleLine2::getRightGridX(int i) const {
	assert(0 <= i && i < segmentData.Size());
	SegmentData sd;
	segmentData.Get(i, &sd);
	return sd.getRightX();
}

int SimpleLine2::getRightGridY(int i) const {
	assert(0 <= i && i < segmentData.Size());
	SegmentData sd;
	segmentData.Get(i, &sd);
	return sd.getRightY();
}

mpq_class SimpleLine2::getPreciseLeftX(int i) const {
	assert(0 <= i && i < segmentData.Size());
	SegmentData sd;
	segmentData.Get(i, &sd);
	return sd.getPreciseLeftX(&preciseCoordinates);
}

mpq_class SimpleLine2::getPreciseLeftY(int i) const {
	assert(0 <= i && i < segmentData.Size());
	SegmentData sd;
	segmentData.Get(i, &sd);
	return sd.getPreciseLeftY(&preciseCoordinates);
}

mpq_class SimpleLine2::getPreciseRightX(int i) const {
	assert(0 <= i && i < segmentData.Size());
	SegmentData sd;
	segmentData.Get(i, &sd);
	return sd.getPreciseRightX(&preciseCoordinates);
}

mpq_class SimpleLine2::getPreciseRightY(int i) const {
	assert(0 <= i && i < segmentData.Size());
	SegmentData sd;
	assert(segmentData.Get(i, &sd));

	return sd.getPreciseRightY(&preciseCoordinates);
}

void SimpleLine2::get(int i, SegmentData& sd) const {
	assert(0 <= i && i <= segmentData.Size());
	segmentData.Get(i, sd);
}

void SimpleLine2::addSegment(bool ldp, int leftX, int leftY,
		int rightX, int rightY,
		mpq_class pLeftX, mpq_class pLeftY,
		mpq_class pRightX,
		mpq_class pRightY, int edgeNo) {
	SegmentData sd(leftX, leftY, rightX, rightY,
			ldp, edgeNo);
	sd.addPreciseValues(&preciseCoordinates, pLeftX, pLeftY,
			pRightX, pRightY);
	segmentData.Append(sd);
}

void SimpleLine2::addSegment(bool ldp, Point2* lp, Point2* rp,
		int edgeno) {
	SegmentData sd(lp, rp, ldp,
			edgeno, &preciseCoordinates);
	segmentData.Append(sd);
}

bool SimpleLine2::IsLeftDomPoint(int i) const {
	assert(0 <= i && i < segmentData.Size());
	SegmentData sd;
	assert(segmentData.Get(i, &sd));

	return sd.IsLeftDomPoint();
}

int SimpleLine2::Size() const {
	return segmentData.Size();
}


const Rectangle<2> SimpleLine2::BoundingBox(const Geoid* geoid /*=0*/) const {
 if(geoid){
 cerr << ": Spherical geometry not implemented."
 << endl;
 assert( false );
 }
	return bbox;
}

inline void SimpleLine2::Destroy() {
	preciseCoordinates.destroy();
	segmentData.destroy();
	lrsArray.destroy();
}


void SimpleLine2::StartBulkLoad() {
	ordered = false;
	SetDefined(true);
}

/*
 ~EndBulkLoad~

 Finishes the bulkload for a simpleLine2-object. If this function is called,
 both halfSegments assigned to a segment of the simpleline must be part
 of this simpleline.

 The parameter ~sort~ can be set to __false__ if the halfsegments are
 already ordered using the falfSegment order.

 The parameter ~realminize~ can be set to __false__ if the simpleline is
 already realminized, meaning each pair of different Segments has
 at most a common endpoint. Furthermore, the two halfsegments belonging
 to a segment must have the same edge number. The edge numbers must be
 in Range [0..Size()-1]. HalfSegments belonging to different segments
 must have different edge numbers.

 Only change one of the parameters if you exactly know what you do.
 Changing such parameters without fullfilling the conditions stated
 above may construct invalid line representations which again may
 produce a system crash within some operators.

*/
bool SimpleLine2::EndBulkLoad(const bool sort /* = true */,
		const bool realminize /* = true */,
		const bool robust /* = false */) {
	if (!IsDefined()) {
		Clear();
		SetDefined(false);
	}
	Sort();
	if (Size() > 0) {
		if (realminize) {
			SimpleLine2* resultLine = new SimpleLine2(true);
			Realminize(*this, *resultLine, false);
			this->CopyFrom(resultLine);
			resultLine->Destroy();
			delete resultLine;
		}
		SetPartnerNo();
	}

  if(!computePolyline()){
     Clear();
     SetDefined(false);
     return false;
  } else {
     TrimToSize();
     return true;
  }

}

/*
 ~computePolyline~

*/
bool SimpleLine2::computePolyline(){

 if( !IsDefined() ) {
  cout <<"is not defined"<<endl;
   return false;
 }
 lrsArray.clean();
 isCycle = false;
 if( segmentData.Size()==0){ // an empty line
    return true;
 }

 // the segmentData array has to be sorted, realminized and
 // the partnernumber must be set correctly

 // step 1: try to find the start of the polyline and check for branches
 int size = segmentData.Size();
 int start = -1;
 int end = -1;
 int count = 0;
 int pos = 0;
 SegmentData sd;

 int p1DX, p1DY;
 mpq_class p1PX, p1PY;

 int p2DX, p2DY;
 mpq_class p2PX, p2PY;

cout <<"size: "<<size<<endl;
 while(pos<size){
  cout <<"pos"<<pos<<endl;
   count = 1;
   segmentData.Get(pos,sd);

   p1DX = sd.GetDomGridXCoord();
   p1DY = sd.GetDomGridYCoord();
   p1PX = sd.GetDomPreciseXCoord(&preciseCoordinates);
   p1PY = sd.GetDomPreciseYCoord(&preciseCoordinates);

   pos++;
   if(pos<size){
     segmentData.Get(pos,sd);

     p2DX = sd.GetDomGridXCoord();
     p2DY = sd.GetDomGridYCoord();
     p2PX = sd.GetDomPreciseXCoord(&preciseCoordinates);
     p2PY = sd.GetDomPreciseYCoord(&preciseCoordinates);

   }
   while(pos<size && ((p1DX==p2DX) && p1DY==p2DY
       && (cmp(p1PX, p2PX)==0) && (cmp(p1PY, p2PY)==0))){
     count++;
     if(count>2){  // branch detected
      cout <<"branch detected"<<endl;
       return false;
     } else {
       pos++;

       p1DX = p2DX;
       p1DY = p2DY;
       p1PX = p2PX;
       p1PY = p2PY;

       if(pos<size){
          segmentData.Get(pos,sd);

          p2DX = sd.GetDomGridXCoord();
          p2DY = sd.GetDomGridYCoord();
          p2PX = sd.GetDomPreciseXCoord(&preciseCoordinates);
          p2PY = sd.GetDomPreciseYCoord(&preciseCoordinates);
       }
     }
   }
   if(count==1){
      if(start<0){
        start = pos - 1;
        cout <<"startPos:"<<start<<endl;
        cout << "p:" <<p1DX<<" " <<p1PX<<", "<<p1DY<<" "<<p1PY<<endl;
      } else if(end < 0){
        end = pos - 1;
        cout <<"endPos:"<<end<<endl;
        cout << "p:" <<p1DX<<" " <<p1PX<<", "<<p1DY<<" "<<p1PY<<endl;
      } else { // third end detected
       cout <<"third end detected"<<endl;
       cout <<"os:"<<pos<<endl;
       cout << "p:" <<p1DX<<" " <<p1PX<<", "<<p1DY<<" "<<p1PY<<endl;
        return false;
      }
   }
 }

 if(start<0 && end>=0){ // loop detected
  cout <<"loop "<<endl;
     return false;
 }

 pos = 0;
 if(start<0){ // line is a cycle
   isCycle=true;
 } else {
   isCycle = false;
   pos = start;
 }

 // the line has two or zero endpoints, may be several components
 vector<bool> used(size,false);
 int noUnused = size;
 SegmentData sd1;
 SegmentData sd2;
 lrsArray.resize(segmentData.Size()/2 + 1);
 double lrsPos = 0.0;
 int sdPos = pos;
 int edge = 0;
 while(noUnused > 0){
   segmentData.Get(sdPos,sd1);
   used[sdPos]=true; // segment is used
   noUnused--;
   int partnerpos = sd1.GetPartnerno();
   segmentData.Get(partnerpos,sd2);
   used[partnerpos] = true; // partner is used
   noUnused--;
   // store edgenumber
   SegmentData SD1 = sd1;
   SegmentData SD2 = sd2;
   SD1.SetEdgeNo(edge);
   SD2.SetEdgeNo(edge);
   edge++;
   segmentData.Put(sdPos, SD1);
   segmentData.Put(partnerpos, SD2);

   lrsArray.Append(LRS(lrsPos,sdPos));
   //lrsPos += sd1.Length();


   p1DX = sd2.GetDomGridXCoord();
   p1DY = sd2.GetDomGridYCoord();
   p1PX = sd2.GetDomPreciseXCoord(&preciseCoordinates);
   p1PY = sd2.GetDomPreciseYCoord(&preciseCoordinates);

   if(noUnused > 0){
      bool found = false;
      if(partnerpos > 0 && !used[partnerpos-1]){ // check left side
        segmentData.Get(partnerpos-1,sd2);

        p2DX = sd2.GetDomGridXCoord();
        p2DY = sd2.GetDomGridYCoord();
        p2PX = sd2.GetDomPreciseXCoord(&preciseCoordinates);
        p2PY = sd2.GetDomPreciseYCoord(&preciseCoordinates);

        if(p1DX==p2DX && p1DY==p2DY
          && (cmp(p1PX, p2PX)==0) && (cmp(p1PY, p2PY)==0)){ // extension found
          found = true;
          sdPos = partnerpos-1;
        }
      }
      if(!found  && (partnerpos < (size-1) && !used[partnerpos+1])){
          segmentData.Get(partnerpos+1,sd2);

          p2DX = sd2.GetDomGridXCoord();
          p2DY = sd2.GetDomGridYCoord();
          p2PX = sd2.GetDomPreciseXCoord(&preciseCoordinates);
          p2PY = sd2.GetDomPreciseYCoord(&preciseCoordinates);

          if(p1DX==p2DX && p1DY==p2DY
            && (cmp(p1PX, p2PX)==0) && (cmp(p1PY, p2PY)==0)){
             found = true;
             sdPos = partnerpos+1;
          }
      }
      if(!found){  // no extension found
       cout <<"no extension found"<<endl;
        return false;
      }
   }
 }
 lrsArray.Append(LRS(lrsPos,sdPos));
 //length = lrsPos;
 return true;
}

Point2 SimpleLine2::StartPoint() const {
  if( !IsDefined() || IsEmpty() ) return Point2( false );
  LRS lrs;
  SegmentData sd;
  int pos = 0;
  if (startSmaller){
    lrsArray.Get( pos, lrs );
    // Get half-segment
    segmentData.Get( lrs.hsPos, sd );
    // Return one end of the half-segment depending
    // on the start.
    return Point2(true, sd.GetSecGridXCoord(), sd.GetSecGridYCoord(),
         sd.GetSecPreciseXCoord(&preciseCoordinates),
         sd.GetSecPreciseYCoord(&preciseCoordinates));
  } else {
    pos = lrsArray.Size()-1;
    lrsArray.Get( pos, lrs );
    // Get half-segment
    segmentData.Get( lrs.hsPos, sd );
    // Return one end of the half-segment depending
    // on the start.
    return Point2(true, sd.GetDomGridXCoord(), sd.GetDomGridYCoord(),
         sd.GetDomPreciseXCoord(&preciseCoordinates),
         sd.GetDomPreciseYCoord(&preciseCoordinates));
  }

}

Point2 SimpleLine2::StartPoint( bool startsSmaller ) const {
  if( IsEmpty() || !IsDefined() ) return Point2( false );
  if (startsSmaller == startSmaller) return StartPoint();
  else return EndPoint();
}

/*
~EndPoint~

Returns the endpoint of this simple Line.

*/

Point2 SimpleLine2::EndPoint() const {
  if( !IsDefined() || IsEmpty()) return Point2( false );
  LRS lrs;
  SegmentData sd;
  int pos = lrsArray.Size()-1;
  if (startSmaller){
    lrsArray.Get( pos, lrs );
    // Get half-segment
    segmentData.Get( lrs.hsPos, sd );
    // Return one end of the half-segment depending
    // on the start.
    return Point2(true, sd.GetSecGridXCoord(), sd.GetSecGridYCoord(),
         sd.GetSecPreciseXCoord(&preciseCoordinates),
         sd.GetSecPreciseYCoord(&preciseCoordinates));
  } else {
    pos = 0;
    lrsArray.Get( pos, lrs );
    // Get half-segment
    segmentData.Get( lrs.hsPos, sd );
    // Return one end of the half-segment depending
    // on the start.
    return Point2(true, sd.GetDomGridXCoord(), sd.GetDomGridYCoord(),
         sd.GetDomPreciseXCoord(&preciseCoordinates),
         sd.GetDomPreciseYCoord(&preciseCoordinates));
  }
}


Point2 SimpleLine2::EndPoint( bool startsSmaller ) const {
  if( IsEmpty() || !IsDefined() ) return Point2( false );
  if (startsSmaller == startSmaller) return EndPoint();
  else return StartPoint();
}


/*
~TrimToSize~

Changes the capacities of the contained arrays to the required size.

*/
  inline void SimpleLine2::TrimToSize(){
     segmentData.TrimToSize();
     lrsArray.TrimToSize();
  }



void SimpleLine2::Clear() {
	segmentData.clean();
	preciseCoordinates.clean();
	ordered = true;
	bbox.SetDefined(false);
	SetDefined(true);
}

bool SimpleLine2::SegmentIsVertical(int lx, mpq_class plx, int rx,
		mpq_class prx) {
	if (lx != rx) {
		return false;
	}
	if (cmp(plx, prx) != 0) {
		return false;
	}
	return true;
}

/*
 ~CompareSegment~

 Compares the given halfsegments. Returns
 1	if ~seg1~ > ~seg2~,
 0 	if ~seg1~ = ~seg2~ and
 -1	if ~seg1~ < ~seg2~

 First the dominating points of both halfsegments will be
 compared. If both halfsegments have the same left
 dominating point, we compare the slopes of the
 halfsegments. If they are equal too, the second point of
  both halfsegments will be compared.

*/
int SimpleLine2::CompareSegment(const SegmentData& seg1,
		const SegmentData& seg2) const {

	int seg1DomGridX = seg1.GetDomGridXCoord();
	int seg1DomGridY = seg1.GetDomGridYCoord();
	int seg1SecGridX = seg1.GetSecGridXCoord();
	int seg1SecGridY = seg1.GetSecGridYCoord();

	mpq_class seg1DomPreciseX =
			seg1.GetDomPreciseXCoord(&preciseCoordinates);
	mpq_class seg1DomPreciseY =
			seg1.GetDomPreciseYCoord(&preciseCoordinates);

	int seg2DomGridX = seg2.GetDomGridXCoord();
	int seg2DomGridY = seg2.GetDomGridYCoord();
	int seg2SecGridX = seg2.GetSecGridXCoord();
	int seg2SecGridY = seg2.GetSecGridYCoord();

	mpq_class seg2DomPreciseX =
			seg2.GetDomPreciseXCoord(&preciseCoordinates);
	mpq_class seg2DomPreciseY =
			seg2.GetDomPreciseYCoord(&preciseCoordinates);

	int cmpDomPreciseX =
			cmp(seg1DomPreciseX, seg2DomPreciseX);

	//comparing the dominating points
	if ((seg1DomGridX < seg2DomGridX)
			|| ((seg1DomGridX == seg2DomGridX)
					&& (cmpDomPreciseX < 0))
			|| ((seg1DomGridX == seg2DomGridX)
					&& (cmpDomPreciseX == 0)
					&& (seg1DomGridY < seg2DomGridY))
			|| ((seg1DomGridX == seg2DomGridX)
					&& (cmpDomPreciseX == 0)
					&& (seg1DomGridY == seg2DomGridY)
					&& (cmp(seg1DomPreciseY,
						seg2DomPreciseY) < 0))) {
		//The dominating point of ~this~ is less than
		//the dominating point of ~s~.
		return -1;
	} else {
		if ((seg1DomGridX > seg2DomGridX)
			|| ((seg1DomGridX == seg2DomGridX)
					&& (cmpDomPreciseX > 0))
			|| ((seg1DomGridX == seg2DomGridX)
					&& (cmpDomPreciseX == 0)
			&& (seg1DomGridY > seg2DomGridY))
			|| ((seg1DomGridX == seg2DomGridX)
					&& (cmpDomPreciseX == 0)
					&& (seg1DomGridY == seg2DomGridY)
					&& (cmp(seg1DomPreciseY,
						seg2DomPreciseY) > 0))) {
			//The dominating point of ~this~ is greater
			//than the dominating point of ~s~.
			return 1;

		}
	}

	//both halfsegments have the same dominating point
	//they might be the both halfsegments of one segment.
	//If so, this function returns 1 if the left point of
	//~seg1~ is the dominating point (in this case the
	//second point of ~seg2~ is a dominating point too.)
	if (seg1.IsLeftDomPoint() != seg2.IsLeftDomPoint()) {
		if (!seg1.IsLeftDomPoint()) {
			return -1;
		}
		return 1;
	} else {
		//both halfsegments have the same dominating point,
		//which are in both halfsegments the left points.
		//Now we compare the slopes of both halfsegments
		mpq_class seg1SecPreciseX = seg1.GetSecPreciseXCoord(
				&preciseCoordinates);
		mpq_class seg1SecPreciseY = seg1.GetSecPreciseYCoord(
				&preciseCoordinates);

		mpq_class seg2SecPreciseX = seg2.GetSecPreciseXCoord(
				&preciseCoordinates);
		mpq_class seg2SecPreciseY = seg2.GetSecPreciseYCoord(
				&preciseCoordinates);

		bool v1 = ((seg1DomGridX == seg1SecGridX)
				&& (seg1DomPreciseX == seg1SecPreciseX));
		bool v2 = ((seg2DomGridX == seg2SecGridX)
				&& (seg2DomPreciseX == seg2SecPreciseX));

		if (v1 && v2) {
			//both halfsegments are vertical
			int cmpThisY =
					cmp(seg1SecPreciseY, seg1DomPreciseY);
			int cmpSY =
					cmp(seg2SecPreciseY, seg2DomPreciseY);
			if ((((seg1SecGridY > seg1DomGridY)
				|| ((seg1SecGridY == seg1DomGridY)
						&& cmpThisY > 0))
				&& ((seg2SecGridY > seg2DomGridY)
				|| ((seg2SecGridY == seg2DomGridY)
						&& cmpSY > 0)))
				|| (((seg1SecGridY < seg1DomGridY)
					|| ((seg1SecGridY == seg1DomGridY)
							&& cmpThisY < 0))
					&& ((seg2SecGridY < seg2DomGridY)
				|| ((seg2SecGridY == seg2DomGridY)
						&& cmpSY < 0)))) {
				//The y-value of the second points of both
				//halfsegments are greater than their
				//dominating points or the y-value of the
				//second points of the halfsegments are less
				//than their dominating points.
				int cmpSecPreciseX = cmp(seg1SecPreciseX,
						seg2SecPreciseX);
				if ((seg1SecGridX < seg2SecGridX)
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX < 0))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY < seg2SecGridY))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY == seg2SecGridY)
						&& (cmp(seg1SecPreciseX,
						seg2SecPreciseX) < 0))) {
					//The second point of ~this~ is less
					//than the
					//second point of ~s~.
					return -1;
				}
				if ((seg1SecGridX > seg2SecGridX)
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX > 0))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY > seg2SecGridY))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY == seg2SecGridY)
						&& (cmp(seg1SecPreciseX,
						seg2SecPreciseX) > 0))) {
					//The second point of ~this~ is greater
					//than the second point of ~s~.
					return 1;
				}
				return 0;
			} else {
				if ((seg1SecGridY > seg1DomGridY)
					|| ((seg1SecGridY == seg1DomGridY)
						&& (cmp(seg1SecPreciseY,
						seg1DomPreciseY) > 0))) {
					//The y-value of the second point of
					//~this~ is greater than the y-value of
					//the dominating point of ~this~.
					if (seg1.IsLeftDomPoint()) {
						return 1;
					} else {
						return -1;
					}
				} else {
					if (seg1.IsLeftDomPoint()) {
						return -1;
					} else {
						return 1;
					}
				}
			}
		} else {
			if (v1) {
				//~this~ is vertical
				if ((seg1SecGridY > seg1DomGridY)
					|| ((seg1SecGridY == seg1DomGridY)
							&& cmp(seg1SecPreciseY,
							seg1DomPreciseY) > 0)){
					//the y-value of the second point of
					//~this~ is greater than
					//the dominating point of ~this~
					if (seg1.IsLeftDomPoint())
						return 1;
					return -1;
				} else {
					if ((seg1SecGridY < seg1DomGridY)
					|| ((seg1SecGridY == seg1DomGridY)
							&& cmp(seg1SecPreciseY,
							seg1DomPreciseY) < 0))

							{
						if (seg1.IsLeftDomPoint())
							return -1;
						return 1;
					}
				}
			} else {
				if (v2) {
					//~s~ is vertical

					if ((seg2SecGridY > seg2DomGridY)
					|| ((seg2SecGridY == seg2DomGridY)
							&& cmp(seg2SecPreciseY,
							seg2DomPreciseY) > 0))
						//the y-value of the second
						//point of ~s~ isgreater
						//than the dominating point
						//of ~s~
							{
						if (seg1.IsLeftDomPoint())
							return -1;
						return 1;
				} else if ((seg2SecGridY < seg2DomGridY)
					|| ((seg2SecGridY == seg2DomGridY)
							&& cmp(seg2SecPreciseY,
							seg2DomPreciseY) < 0)) {
						//the y-value of the second
						//point of ~s~ is greater
						//than the dominating point
						//of ~s~
						if (seg1.IsLeftDomPoint())
							return 1;
						return -1;
					}
				} else {
					mpq_class xd = seg1DomPreciseX
							+ seg1DomGridX;
					mpq_class yd = seg1DomPreciseY
							+ seg1DomGridY;
					mpq_class xs = seg1SecPreciseX
							+ seg1SecGridX;
					mpq_class ys = seg1SecPreciseY
							+ seg1SecGridY;
					mpq_class Xd = seg2DomPreciseX
							+ seg2DomGridX;
					mpq_class Yd = seg2DomPreciseY
							+ seg2DomGridY;
					mpq_class Xs = seg2SecPreciseX
							+ seg2SecGridX;
					mpq_class Ys = seg2SecPreciseY
							+ seg2SecGridY;

					mpq_class seg1Slope =
							(yd - ys) / (xd - xs);
					mpq_class seg2Slope =
							(Yd - Ys) / (Xd - Xs);
					int cmpSlope = cmp(seg1Slope,
							seg2Slope);
					if (cmpSlope < 0) {
						return -1;
					}
					if (cmpSlope > 0) {
						return 1;
					}
					int cmpSecPreciseX =
							cmp(seg1SecPreciseX,
							seg2SecPreciseX);
				if ((seg1SecGridX < seg2SecGridX)
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX < 0))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY < seg2SecGridY))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY == seg2SecGridY)
						&& (cmp(seg1SecPreciseX,
							seg2SecPreciseX)< 0))){
					//The second point of ~this~ is less
					//than thesecond point of ~s~
					return -1;
					}
					if ((seg1SecGridX > seg2SecGridX)
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX > 0))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY > seg2SecGridY))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY == seg2SecGridY)
						&& (cmp(seg1SecPreciseX,
						seg2SecPreciseX) > 0))) {
						//The second point of ~this~ is
						//greater than
						//the second point of ~s~
						return 1;
					}
					return 0;
				}
			}
		}

	}

	assert(false); // This code should never be reached
	return 0;

}

/*
 ~CompareSegment2~

 Compares the given halfsegments. Returns
 1	if ~seg1~ > ~seg2~,
 0 	if ~seg1~ = ~seg2~ and
 -1	if ~seg1~ < ~seg2~

 First the dominating points of both halfsegments will be
 compared. If both halfsegments have the same left
 dominating point, we compare the slopes of the
 halfsegments. If they are equal too, the second
 point of both halfsegments will be compared.

*/
int SimpleLine2::CompareSegment2(const SegmentData& seg1,
		const SegmentData& seg2,
		const Flob* preciseCoordOfSeg2) const {

	int seg1DomGridX = seg1.GetDomGridXCoord();
	int seg1DomGridY = seg1.GetDomGridYCoord();
	int seg1SecGridX = seg1.GetSecGridXCoord();
	int seg1SecGridY = seg1.GetSecGridYCoord();

	mpq_class seg1DomPreciseX =
			seg1.GetDomPreciseXCoord(&preciseCoordinates);
	mpq_class seg1DomPreciseY =
			seg1.GetDomPreciseYCoord(&preciseCoordinates);

	int seg2DomGridX = seg2.GetDomGridXCoord();
	int seg2DomGridY = seg2.GetDomGridYCoord();
	int seg2SecGridX = seg2.GetSecGridXCoord();
	int seg2SecGridY = seg2.GetSecGridYCoord();

	mpq_class seg2DomPreciseX =
			seg2.GetDomPreciseXCoord(preciseCoordOfSeg2);
	mpq_class seg2DomPreciseY =
			seg2.GetDomPreciseYCoord(preciseCoordOfSeg2);

	int cmpDomPreciseX =
			cmp(seg1DomPreciseX, seg2DomPreciseX);

	//comparing the dominating points
	if ((seg1DomGridX < seg2DomGridX)
		|| ((seg1DomGridX == seg2DomGridX)
				&& (cmpDomPreciseX < 0))
		|| ((seg1DomGridX == seg2DomGridX)
				&& (cmpDomPreciseX == 0)
			&& (seg1DomGridY < seg2DomGridY))
		|| ((seg1DomGridX == seg2DomGridX)
				&& (cmpDomPreciseX == 0)
			&& (seg1DomGridY == seg2DomGridY)
			&& (cmp(seg1DomPreciseY,
					seg2DomPreciseY) < 0))) {
		//The dominating point of ~this~ is less than the
		//dominating point of ~s~.
		return -1;
	} else {
		if ((seg1DomGridX > seg2DomGridX)
		|| ((seg1DomGridX == seg2DomGridX) &&
				(cmpDomPreciseX > 0))
		|| ((seg1DomGridX == seg2DomGridX) &&
				(cmpDomPreciseX == 0)
			&& (seg1DomGridY > seg2DomGridY))
		|| ((seg1DomGridX == seg2DomGridX) &&
				(cmpDomPreciseX == 0)
			&& (seg1DomGridY == seg2DomGridY)
			&& (cmp(seg1DomPreciseX,
					seg2DomPreciseX) > 0))) {
		//The dominating point of ~this~ is greater than the
		//dominating point of ~s~.
		return 1;

		}
	}

	//both halfsegments have the same dominating point
	//they might be the both halfsegments of one segment.
	//If so, this function returns 1 if the left point of
	//~seg1~ is the dominating point (in this case the
	//second point of ~seg2~ is a dominating point too.)
	if (seg1.IsLeftDomPoint() != seg2.IsLeftDomPoint()) {
		if (!seg1.IsLeftDomPoint()) {
			return -1;
		}
		return 1;
	} else {
		//both halfsegments have the same dominating point,
		//which are in both
		//halfsegments the left points.
		//Now we compare the slopes of both halfsegments
		mpq_class seg1SecPreciseX =
				seg1.GetSecPreciseXCoord(
				&preciseCoordinates);
		mpq_class seg1SecPreciseY =
				seg1.GetSecPreciseYCoord(
				&preciseCoordinates);

		mpq_class seg2SecPreciseX =
				seg2.GetSecPreciseXCoord(
				preciseCoordOfSeg2);
		mpq_class seg2SecPreciseY =
				seg2.GetSecPreciseYCoord(
				preciseCoordOfSeg2);

		bool v1 = ((seg1DomGridX == seg1SecGridX)
				&& (seg1DomPreciseX == seg1SecPreciseX));
		bool v2 = ((seg2DomGridX == seg2SecGridX)
				&& (seg2DomPreciseX == seg2SecPreciseX));

		if (v1 && v2) {
			//both halfsegments are vertical
			int cmpThisY =
					cmp(seg1SecPreciseY, seg1DomPreciseY);
			int cmpSY =
					cmp(seg2SecPreciseY, seg2DomPreciseY);
			if ((((seg1SecGridY > seg1DomGridY)
				|| ((seg1SecGridY == seg1DomGridY)
						&& cmpThisY > 0))
				&& ((seg2SecGridY > seg2DomGridY)
				|| ((seg2SecGridY == seg2DomGridY)
						&& cmpSY > 0)))
				|| (((seg1SecGridY < seg1DomGridY)
				|| ((seg1SecGridY == seg1DomGridY)
						&& cmpThisY < 0))
				&& ((seg2SecGridY < seg2DomGridY)
				|| ((seg2SecGridY == seg2DomGridY)
						&& cmpSY < 0)))) {
				//The y-value of the second points of both
				//halfsegments are greater than their
				//dominating points or the y-value of the
				//second points of the halfsegments are
				//lessthan their dominating points.
				int cmpSecPreciseX = cmp(seg1SecPreciseX,
						seg2SecPreciseX);
				if ((seg1SecGridX < seg2SecGridX)
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX < 0))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY < seg2SecGridY))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY == seg2SecGridY)
						&& (cmp(seg1SecPreciseX,
						seg2SecPreciseX) < 0))) {
					//The second point of ~this~ is less
					//than the second point of ~s~.
					return -1;
				}
				if ((seg1SecGridX > seg2SecGridX)
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX > 0))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY > seg2SecGridY))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY == seg2SecGridY)
						&& (cmp(seg1SecPreciseX,
						seg2SecPreciseX) > 0))) {
					//The second point of ~this~ is greater
					//than the second point of ~s~.
					return 1;
				}
				return 0;
			} else {
				if ((seg1SecGridY > seg1DomGridY)
					|| ((seg1SecGridY == seg1DomGridY)
						&& (cmp(seg1SecPreciseY,
						seg1DomPreciseY) > 0))) {
					//The y-value of the second point of
					//~this~ is greater than the y-value
					//of the dominating point of ~this~.
					if (seg1.IsLeftDomPoint()) {
						return 1;
					} else {
						return -1;
					}
				} else {
					if (seg1.IsLeftDomPoint()) {
						return -1;
					} else {
						return 1;
					}
				}
			}
		} else {
			if (v1) {
				//~this~ is vertical
				if ((seg1SecGridY > seg1DomGridY)
				|| ((seg1SecGridY == seg1DomGridY)
						&& cmp(seg1SecPreciseY,
							seg1DomPreciseY) > 0)){
					//the y-value of the second point
					//of ~this~ is greater than
					//the dominating point of ~this~
					if (seg1.IsLeftDomPoint())
						return 1;
					return -1;
				} else {
					if ((seg1SecGridY < seg1DomGridY)
					|| ((seg1SecGridY == seg1DomGridY)
							&& cmp(seg1SecPreciseY,
							seg1DomPreciseY) < 0))

							{
						if (seg1.IsLeftDomPoint())
							return -1;
						return 1;
					}
				}
			} else {
				if (v2) {
					//~s~ is vertical

					if ((seg2SecGridY > seg2DomGridY)
					|| ((seg2SecGridY == seg2DomGridY)
							&& cmp(seg2SecPreciseY,
							seg2DomPreciseY) > 0))
						//the y-value of the second
						//point of ~s~ is greater than
						//the dominating point of ~s~
					{
						if (seg1.IsLeftDomPoint())
							return -1;
						return 1;
					} else
						if ((
						seg2SecGridY < seg2DomGridY)
					|| ((seg2SecGridY == seg2DomGridY)
						&& cmp(seg2SecPreciseY,
							seg2DomPreciseY) < 0)) {
						//the y-value of the second
						//point of ~s~ is greater than
						//the dominating point of ~s~
						if (seg1.IsLeftDomPoint())
							return 1;
						return -1;
					}
				} else {
					mpq_class xd = seg1DomPreciseX
							+ seg1DomGridX;
					mpq_class yd = seg1DomPreciseY
							+ seg1DomGridX;
					mpq_class xs = seg1SecPreciseX
							+ seg1SecGridX;
					mpq_class ys = seg1SecPreciseY
							+ seg1SecGridY;
					mpq_class Xd = seg2DomPreciseX
							+ seg2DomGridX;
					mpq_class Yd = seg2DomPreciseY
							+ seg2DomGridY;
					mpq_class Xs = seg2SecPreciseX
							+ seg2SecGridX;
					mpq_class Ys = seg2SecPreciseY
							+ seg2SecGridY;

					mpq_class seg1Slope =
							(yd - ys) / (xd - xs);
					mpq_class seg2Slope =
							(Yd - Ys) / (Xd - Xs);

					int cmpSlope =
						cmp(seg1Slope, seg2Slope);
					if (cmpSlope < 0) {
						return -1;
					}
					if (cmpSlope > 0) {
						return 1;
					}

					int cmpSecPreciseX =
							cmp(seg1SecPreciseX,
							seg2SecPreciseX);

					if ((seg1SecGridX< seg2SecGridX)
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX < 0))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY < seg2SecGridY))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY == seg2SecGridY)
						&& (cmp(seg1SecPreciseX,
						seg2SecPreciseX)< 0))) {
					//The second point of ~this~ is
					//less than
					//the second point of ~s~
					return -1;
					}
					if ((seg1SecGridX > seg2SecGridX)
					|| ((seg1SecGridX == seg2SecGridX)
							&& (cmpSecPreciseX > 0))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY > seg2SecGridY))
					|| ((seg1SecGridX == seg2SecGridX)
						&& (cmpSecPreciseX == 0)
					&& (seg1SecGridY == seg2SecGridY)
						&& (cmp(seg1SecPreciseX,
						seg2SecPreciseX)> 0))) {
						//The second point of ~this~ is
						//greater than
						//the second point of ~s~
						return 1;
					}
					return 0;
				}
			}
		}

	}

	assert(false); // This code should never be reached
	return 0;

}

/*
 ~Sort~

 Sorts the segments with Mergesort using the function CompareSegment

*/
void SimpleLine2::Sort() {
	assert(!IsOrdered());
	int sz = segmentData.Size();
	if (sz > 1) {
		MergeSort(0, sz - 1);
	}
	ordered = true;
}

/*
 ~MergeSort~

*/
void SimpleLine2::MergeSort(int startindex, int endindex) {
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
 sequences will be edited in parallel and the sorted result will be
 stored in the DbArray starting at index ~startindex~.

*/
void SimpleLine2::Merge(int startindex, int divide, int endindex) {
	int startL = startindex;
	int startR = divide + 1;
	SegmentData seg1, seg2;
	// The number of elements in both sequences:
	int elemNo = (endindex - startindex) + 1;
	SegmentData result[elemNo];
	int index = 0;
	segmentData.Get(startL, &seg1);
	segmentData.Get(startR, &seg2);
	while (index <= elemNo) {
		if (CompareSegment(seg1, seg2) <= 0) {
			// The current element in the first sequence is less
			// than or equal to the current element in the second
			// sequence and will be stored in result.
			result[index] = seg1;
			index++;
			startL++;
			if (startL <= divide) {
				segmentData.Get(startL, &seg1);
			} else {
				// the first sequence is completely stored
				// in result.
				break;
			}
		} else {
			// The current element in the first sequence is greater
			// than the current element in the second sequence.
			// The element of the second sequence will be stored
			// in result.
			result[index] = seg2;
			index++;
			startR++;
			if (startR <= endindex) {
				segmentData.Get(startR, &seg2);
			} else {
				// the second sequence is completely stored
				// in result.
				break;
			}
		}
	}
	while (startL <= divide) {
		// the second sequence is completely stored in result,
		// but there are
		// still some elements in the first sequence.
		segmentData.Get(startL, &seg1);
		result[index] = seg1;
		index++;
		startL++;
	}
	while (startR <= endindex) {
		// the first sequence is completely stored in
		// result, but there are
		// still some elements in the second sequence.
		segmentData.Get(startR, &seg2);
		result[index] = seg2;
		index++;
		startR++;
	}

	// Storing the merged sequences in the DbArray.
	for (int i = 0; i < elemNo; i++) {
		segmentData.Put(startindex + i, result[i]);
	}
}

void SimpleLine2::SetPartnerNo(){
  if( !IsDefined() || segmentData.Size()==0){
    return;
  }
  DbArray<int> TMP((segmentData.Size()+1)/2);

  SegmentData sd1;
  SegmentData sd2;
  for(int i=0; i<segmentData.Size(); i++){
     segmentData.Get(i,sd1);
     if(sd1.IsLeftDomPoint()){
        TMP.Put(sd1.GetEdgeNo(), i);
        if (!bbox.IsDefined()) {
          bbox = sd1.BoundingBox();
        } else {
          bbox = bbox.Union(sd1.BoundingBox());
        }
      } else {
        int lpp;
        TMP.Get(sd1.GetEdgeNo(),lpp);
        int leftpos = lpp;
        SegmentData right = sd1;
        right.SetPartnerno(leftpos);
        right.SetInsideAbove(false);
        right.SetCoverageno(0);
        right.SetCycleno(0);
        right.SetFaceno(0);
        segmentData.Get(leftpos,sd2);
        SegmentData left = sd2;
        left.SetPartnerno(i);
        left.SetInsideAbove(false);
        left.SetCoverageno(0);
        left.SetCycleno(0);
        left.SetFaceno(0);
        segmentData.Put(i,right);
        segmentData.Put(leftpos,left);
      }
   }
   TMP.Destroy();
 }

/*
 4.4 ~Sizeof~-function

*/
inline size_t SimpleLine2::Sizeof() const {
	return (sizeof(*this));

}

/*
 4.4 ~HashValue~-function

*/
inline size_t SimpleLine2::HashValue() const {
	if (IsEmpty()) {
		return 0;
	}
	size_t h = 0;

	SegmentData seg;
	int x1, y1, x2, y2;

	for (int i = 0; i < Size() && i < 5; i++) {
		segmentData.Get(i, seg);
		x1 = seg.GetDomGridXCoord();
		y1 = seg.GetDomGridYCoord();
		x2 = seg.GetSecGridXCoord();
		y2 = seg.GetSecGridYCoord();

		h += (size_t)((5 * x1 + y1) + (5 * x2 + y2));
	}
	return h;
}

/*
 4.4 ~CopyFrom~-function

*/
inline void SimpleLine2::CopyFrom(const Attribute* right) {
	const SimpleLine2* l = (const SimpleLine2*) right;
	SetDefined(l->IsDefined());
	if (IsDefined()) {
		preciseCoordinates.copyFrom(l->preciseCoordinates);
		segmentData.copyFrom(l->segmentData);
	}
}

/*
 4.4 ~Compare~-function

*/
inline int SimpleLine2::Compare(const Attribute *arg) const {
	const SimpleLine2 &l = *((const SimpleLine2*) arg);

	if (!IsDefined() && !l.IsDefined()) {
		return 0;
	}
	if (!IsDefined()) {
		return -1;
	}
	if (!l.IsDefined()) {
		return 1;
	}

	if (Size() > l.Size())
		return 1;
	if (Size() < l.Size())
		return -1;

  if (startSmaller != l.startsSmaller()){
   if (startsSmaller()){
    return 1;
   } else {
    return -1;
   }
  }

	int index = 0;
	int result = 0;

	while (index < Size()) {
		SegmentData sd1;
		SegmentData sd2;
		segmentData.Get(index, sd1);
		l.get(index, sd2);
		result = CompareSegment2(sd1, sd2, l.getPreciseCoordinates());
		if (result != 0) {
			return result;
		}
		index++;
	}
	return 0;
}

/*
 4.4 ~CloneSimpleLine2~-function

*/
Word SimpleLine2::CloneSimpleLine2(const ListExpr typeInfo, const Word& w) {
	return SetWord(new SimpleLine2(*((SimpleLine2 *) w.addr)));
}

/*
 4.4 ~CastSimpleLine2~-function

*/
void* SimpleLine2::CastSimpleLine2(void* addr) {
	return (new (addr) SimpleLine2());
}

/*
 4.4 ~SizeOfSimpleLine2~-function

*/
int SimpleLine2::SizeOfSimpleLine2() {
	return sizeof(SimpleLine2);
}

/*
 4.4 ~CreateSimpleLine2~-function

*/
Word SimpleLine2::CreateSimpleLine2(const ListExpr typeInfo) {
	return SetWord(new SimpleLine2(false));
}

/*
 4.4 ~DeleteSimpleLine2~-function

*/
void SimpleLine2::DeleteSimpleLine2(const ListExpr typeInfo, Word& w) {
	SimpleLine2 *l = (SimpleLine2 *) w.addr;
  l->Destroy();
	l->SetDefined(false);
	delete l;
	w.addr = 0;
}

/*
 4.4 ~CloseSimpleLine2~-function

*/
void SimpleLine2::CloseSimpleLine2(const ListExpr typeInfo, Word& w) {
	delete (SimpleLine2*) w.addr;
	w.addr = 0;
}

/*
 4.4 ~SimpleLine2Property~-function

*/
ListExpr SimpleLine2::SimpleLine2Property() {

	ListExpr ListRepr = nl->TextAtom();
	nl->AppendText(ListRepr,
			"((<segment>*) <bool> ), where <segment> is "
			"((<xl> <xr> ( <pxl> <pyl>))(<xr> <yr> "
			"( <pxr> <pyr>)))) and bool is true if the simpleline "
      "starts with the smaller endpoint and false otherwise. ");

  ListExpr ExampleList = nl->TextAtom();
  nl->AppendText(ExampleList,
    "((((1 1 ('1/4' '3/4')) (3 4 ('1/8' '1/10')))) TRUE )");

	return nl->TwoElemList(
			nl->FourElemList(nl->StringAtom("Signature"),
					nl->StringAtom("Example Type List"),
					nl->StringAtom("List Rep"),
					nl->StringAtom("Example List")),
			nl->FourElemList(nl->StringAtom("-> DATA"),
          nl->StringAtom(SimpleLine2::BasicType()),
					ListRepr, ExampleList));
}

/*
 4.4 ~CheckSimpleLine2~-function

*/
bool SimpleLine2::CheckSimpleLine2(ListExpr type, ListExpr& errorInfo) {
	return (nl->IsEqual(type, SimpleLine2::BasicType()));
}

/*
 4.4 ~Distance~-function

*/
double SimpleLine2::Distance(const Rectangle<2>& rect,
		const Geoid* geoid/*=0*/) const {
//TODO Distance
	return 1.0;
}

/*
 4.4 ~Empty~-function

*/
bool SimpleLine2::IsEmpty() const {
	return ((!IsDefined()) || (segmentData.Size()==0));
}

} // end of namespace p2d

#endif /* _SimpleLine2_CPP */


