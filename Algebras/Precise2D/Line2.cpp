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

#ifndef _Line2_CPP
#define _Line2_CPP

#include "Line2.h"
#include "AVL_Tree.h"

namespace p2d {

SegmentData::SegmentData(int xl, int yl, int xr, int yr, bool l,
  int edgeNo/*=-999999*/) :
  xLeft(xl), yLeft(yl), xRight(xr), yRight(yr),
  xLeftPos(0), yLeftPos(0), xRightPos(0), yRightPos(0),
  xLeftNumOfChars(0), yLeftNumOfChars(0),
  xRightNumOfChars(0), yRightNumOfChars(0), ldp(l), cycleno(-999999),
  edgeno(edgeNo), coverageno(-999999), insideAbove(false),
  partnerno(-999999) {
}

SegmentData::SegmentData(Point2* lp, Point2* rp, bool ldp, int edgeno,
  Flob& preciseCoordinates) :
  xLeft(lp->getGridX()), yLeft(lp->getGridY()),
  xRight(rp->getGridX()), yRight(rp->getGridY()),
  xLeftPos(0), yLeftPos(0), xRightPos(0), yRightPos(0),
  xLeftNumOfChars(0), yLeftNumOfChars(0), xRightNumOfChars(0),
  yRightNumOfChars(0), ldp(ldp), cycleno(-999999), edgeno(edgeno),
  coverageno(-999999), insideAbove(false), partnerno(-999999) {
 SetPreciseLeftCoordinates(lp, preciseCoordinates);
 SetPreciseRightCoordinates(rp, preciseCoordinates);
}

SegmentData::SegmentData(SegmentData& d) :
  xLeft(d.xLeft), yLeft(d.yLeft), xRight(d.xRight), yRight(d.yRight),
  xLeftPos(d.xLeftPos), yLeftPos(d.yLeftPos),
  xRightPos(d.xRightPos), yRightPos(d.yRightPos),
  xLeftNumOfChars(d.xLeftNumOfChars), yLeftNumOfChars(d.yLeftNumOfChars),
  xRightNumOfChars(d.xRightNumOfChars), yRightNumOfChars(d.yRightNumOfChars),
  ldp(d.ldp), cycleno(d.cycleno), edgeno(d.edgeno),
  coverageno(d.coverageno), insideAbove(d.insideAbove),
  partnerno(d.partnerno) {
}

SegmentData& SegmentData::operator=(const SegmentData& s) {
 xLeft = s.xLeft;
 yLeft = s.yLeft;
 xRight = s.xRight;
 yRight = s.yRight;
 xLeftPos = s.xLeftPos;
 yLeftPos = s.yLeftPos;
 xRightPos = s.xRightPos;
 yRightPos = s.yRightPos;
 xLeftNumOfChars = s.xLeftNumOfChars;
 yLeftNumOfChars = s.yLeftNumOfChars;
 xRightNumOfChars = s.xRightNumOfChars;
 yRightNumOfChars = s.yRightNumOfChars;
 ldp = s.ldp;
 cycleno = s.cycleno;
 edgeno = s.edgeno;
 coverageno = s.coverageno;
 insideAbove = s.insideAbove;
 partnerno = s.partnerno;
 return *this;
}

mpq_class SegmentData::getPreciseLeftX(const Flob& preciseCoordinates) const {
 if (xLeftNumOfChars == 0) {
  mpq_class theValue(0);
  return theValue;
 }
 SmiSize sz = preciseCoordinates.getSize();
 char* s;
 if (sz == 0) {
  s = new char[1];
  s[0] = 0;
 } else {
  s = new char[xLeftNumOfChars + 1];
  assert((xLeftPos + xLeftNumOfChars) <= sz);
  bool ok = preciseCoordinates.read(s, xLeftNumOfChars, xLeftPos);
  assert(ok);
  s[xLeftNumOfChars] = 0;

 }
 mpq_class theValue(s);
 theValue.canonicalize();
 return theValue;
}

mpq_class SegmentData::getPreciseLeftY(const Flob& preciseCoordinates) const {
 if (yLeftNumOfChars == 0) {
  mpq_class theValue(0);
  return theValue;
 }
 SmiSize sz = preciseCoordinates.getSize();
 char* s;
 if (sz == 0 || sz == yLeftPos) {
  s = new char[1];
  s[0] = 0;
 } else {
  s = new char[yLeftNumOfChars + 1];

  bool ok = preciseCoordinates.read(s, yLeftNumOfChars, yLeftPos);
  assert(ok);
  s[yLeftNumOfChars] = 0;

 }
 mpq_class theValue(s);
 theValue.canonicalize();
 return theValue;
}

mpq_class SegmentData::getPreciseRightX(const Flob& preciseCoordinates) const {
 if (xRightNumOfChars == 0) {
  mpq_class theValue(0);
  return theValue;
 }
 SmiSize sz = preciseCoordinates.getSize();
 char* s;
 if (sz == 0 || sz == xRightPos) {
  s = new char[1];
  s[0] = 0;
 } else {
  s = new char[xRightNumOfChars + 1];

  bool ok = preciseCoordinates.read(s, xRightNumOfChars, xRightPos);
  assert(ok);
  s[xRightNumOfChars] = 0;

 }
 mpq_class theValue(s);
 theValue.canonicalize();
 return theValue;
}

mpq_class SegmentData::getPreciseRightY(const Flob& preciseCoordinates) const {
 if (yRightNumOfChars == 0) {
  mpq_class theValue(0);
  return theValue;
 }
 SmiSize sz = preciseCoordinates.getSize();
 char* s;
 if (sz == 0 || sz == yRightPos) {
  s = new char[1];
  s[0] = 0;
 } else {
  s = new char[yRightNumOfChars + 1];

  bool ok = preciseCoordinates.read(s, yRightNumOfChars, yRightPos);
  assert(ok);
  s[yRightNumOfChars] = 0;

 }
 mpq_class theValue(s);
 theValue.canonicalize();
 return theValue;
}

/*
 Returns the named value of the dominating point of the
 half segment.

*/
const int SegmentData::GetDomGridXCoord() const {
 if (ldp) {
  return xLeft;
 } else {
  return xRight;
 }
}

const int SegmentData::GetDomGridYCoord() const {
 if (ldp) {
  return yLeft;
 } else {
  return yRight;
 }
}

const mpq_class SegmentData::GetDomPreciseXCoord(
  const Flob& preciseCoordinates) const {
 if (ldp) {
  return getPreciseLeftX(preciseCoordinates);
 } else {
  return getPreciseRightX(preciseCoordinates);
 }
}

const mpq_class SegmentData::GetDomPreciseYCoord(
  const Flob& preciseCoordinates) const {
 if (ldp) {
  return getPreciseLeftY(preciseCoordinates);
 } else {
  return getPreciseRightY(preciseCoordinates);
 }
}

const int SegmentData::GetSecGridXCoord() const {
 if (!ldp) {
  return xLeft;
 } else {
  return xRight;
 }
}

const int SegmentData::GetSecGridYCoord() const {
 if (!ldp) {
  return yLeft;
 } else {
  return yRight;
 }
}

const mpq_class SegmentData::GetSecPreciseXCoord(
  const Flob& preciseCoordinates) const {
 if (!ldp) {
  return getPreciseLeftX(preciseCoordinates);
 } else {
  return getPreciseRightX(preciseCoordinates);
 }
}

const mpq_class SegmentData::GetSecPreciseYCoord(
  const Flob& preciseCoordinates) const {
 if (!ldp) {
  return getPreciseLeftY(preciseCoordinates);
 } else {
  return getPreciseRightY(preciseCoordinates);
 }
}

bool SegmentData::IsLeftDomPoint() const {
 return ldp;
}

void SegmentData::SetPreciseLeftX(Flob& preciseCoordinates, mpq_class lx) {
 if (cmp(lx, 0) == 0) {
  xLeftPos = 0;
  xLeftNumOfChars = 0;
  return;
 }

 SmiSize sz = preciseCoordinates.getSize();

 string str = lx.get_str();
 char* s = new char[str.length() + 1];
 std::strcpy(s, str.c_str());

 xLeftNumOfChars = strlen(s);
 if (sz == 0) {
  xLeftPos = 0;
  // We have to allocate one byte for the ending-0 in the FLOB.
  preciseCoordinates.resize(xLeftNumOfChars + 1);
 } else {
  xLeftPos = sz - 1;
  // The FLOB already ends with 0.
  preciseCoordinates.resize(sz + xLeftNumOfChars);
 }
 assert(preciseCoordinates.write(s, xLeftNumOfChars + 1, xLeftPos));
}

void SegmentData::SetPreciseLeftY(Flob& preciseCoordinates, mpq_class ly) {
 if (cmp(ly, 0) == 0) {
  yLeftPos = 0;
  yLeftNumOfChars = 0;
  return;
 }

 SmiSize sz = preciseCoordinates.getSize();

 string str = ly.get_str();
 char* s = new char[str.length() + 1];
 std::strcpy(s, str.c_str());

 yLeftNumOfChars = strlen(s);
 if (sz == 0) {
  // We have to allocate one byte for the ending-0 in the FLOB.
  yLeftPos = 0;
  preciseCoordinates.resize(yLeftNumOfChars + 1);
 } else {
  yLeftPos = sz - 1;
  // The FLOB already ends with 0.
  preciseCoordinates.resize(sz + yLeftNumOfChars);
 }
 assert(preciseCoordinates.write(s, yLeftNumOfChars + 1, yLeftPos));
}

void SegmentData::SetPreciseRightX(Flob& preciseCoordinates, mpq_class rx) {
 if (cmp(rx, 0) == 0) {
  xRightPos = 0;
  xRightNumOfChars = 0;
  return;
 }

 SmiSize sz = preciseCoordinates.getSize();

 string str = rx.get_str();
 char* s = new char[str.length() + 1];
 std::strcpy(s, str.c_str());

 xRightNumOfChars = strlen(s);
 if (sz == 0) {
  xRightPos = 0;
  // We have to allocate one byte for the ending-0 in the FLOB.
  preciseCoordinates.resize(xRightNumOfChars + 1);
 } else {
  xRightPos = sz - 1;
  // The FLOB already ends with 0.
  preciseCoordinates.resize(sz + xRightNumOfChars);
 }
 assert(preciseCoordinates.write(s, xRightNumOfChars + 1, xRightPos));

}

void SegmentData::SetPreciseRightY(Flob& preciseCoordinates, mpq_class ry) {
 if (cmp(ry, 0) == 0) {
  yRightPos = 0;
  yRightNumOfChars = 0;
  return;
 }

 SmiSize sz = preciseCoordinates.getSize();

 string str = ry.get_str();
 char* s = new char[str.length() + 1];
 std::strcpy(s, str.c_str());

 yRightNumOfChars = strlen(s);
 if (sz == 0) {
  yRightPos = 0;
  // We have to allocate one byte for the ending-0 in the FLOB.
  preciseCoordinates.resize(yRightNumOfChars + 1);
 } else {
  yRightPos = sz - 1;
  // The FLOB already ends with 0.
  preciseCoordinates.resize(sz + yRightNumOfChars);
 }
 assert(preciseCoordinates.write(s, yRightNumOfChars + 1, yRightPos));

}

void SegmentData::SetPreciseLeftCoordinates(Point2* lp,
  Flob& preciseCoordinates) {
 SmiSize sz = preciseCoordinates.getSize();

 string slx(lp->getPreciseXAsString());
 if (slx == "0") {
  slx = "";
  xLeftPos = 0;
  xLeftNumOfChars = 0;
 } else {
  xLeftPos = sz;
  xLeftNumOfChars = slx.length();
 }

 string sly(lp->getPreciseYAsString());
 if (sly == "0") {
  sly = "";
  yLeftPos = 0;
  yLeftNumOfChars = 0;
 } else {
  yLeftPos = sz + xLeftNumOfChars;
  yLeftNumOfChars = sly.length();
 }

 size_t length = xLeftNumOfChars + yLeftNumOfChars;

 if (length > 0) {
  SmiSize newSize;
  if (sz == 0) {
   newSize = length + 1;
  } else {
   newSize = sz + length;
  }
  preciseCoordinates.resize(newSize);

  length++;
  string value = slx + sly;
  char* s = new char[length];
  std::strcpy(s, value.c_str());

  if (xLeftNumOfChars > 0) {

   assert(preciseCoordinates.write(s, length, xLeftPos));
  } else {
   assert(preciseCoordinates.write(s, length, yLeftPos));
  }
 }
}

void SegmentData::SetPreciseRightCoordinates(Point2* rp,
  Flob& preciseCoordinates) {
 SmiSize sz = preciseCoordinates.getSize();

 string srx(rp->getPreciseXAsString());
 if (srx == "0") {
  srx = "";
  xRightPos = 0;
  xRightNumOfChars = 0;
 } else {
  xRightPos = sz;
  xRightNumOfChars = srx.length();
 }

 string sry(rp->getPreciseYAsString());
 if (sry == "0") {
  sry = "";
  yRightPos = 0;
  yRightNumOfChars = 0;
 } else {
  yRightPos = sz + xRightNumOfChars;
  yRightNumOfChars = sry.length();
 }

 size_t length = xRightNumOfChars + yRightNumOfChars;

 if (length > 0) {
  SmiSize newSize;
  if (sz == 0) {
   newSize = length + 1;
  } else {
   newSize = sz + length;
  }
  preciseCoordinates.resize(newSize);

  string value = srx + sry;
  char* s = new char[value.length() + 1];
  std::strcpy(s, value.c_str());

  if (xRightNumOfChars > 0) {
   assert(
     preciseCoordinates.write(s, xRightNumOfChars + yRightNumOfChars + 1,
       xRightPos));
  } else {
   assert(
     preciseCoordinates.write(s, xRightNumOfChars + yRightNumOfChars + 1,
       yRightPos));
  }
 }
}

void SegmentData::addPreciseValues(Flob& preciseCoordinates, mpq_class pLeftX,
  mpq_class pLeftY, mpq_class pRightX, mpq_class pRightY) {
 SetPreciseLeftX(preciseCoordinates, pLeftX);
 SetPreciseLeftY(preciseCoordinates, pLeftY);
 SetPreciseRightX(preciseCoordinates, pRightX);
 SetPreciseRightY(preciseCoordinates, pRightY);
}

const Rectangle<2> SegmentData::BoundingBox(const Geoid* geoid /*= 0*/) const {
 assert(!geoid);

 double minx = getLeftX() < getRightX() ? getLeftX() : getRightX();
 double maxx = getLeftX() < getRightX() ? getRightX() + 1 : getLeftX() + 1;
 double miny = getLeftY() < getRightY() ? getLeftY() : getRightY();
 double maxy = getLeftY() < getRightY() ? getRightY() + 1 : getLeftY() + 1;
 return Rectangle<2>(true, minx, maxx, miny, maxy);
}

bool SegmentData::IsVertical(const Flob& preciseCoordinates) const {
 if (xLeft != xRight) {
  return false;
 } else {
  if (cmp(getPreciseLeftX(preciseCoordinates),
    getPreciseRightX(preciseCoordinates)) == 0) {
   return true;
  } else {
   return false;
  }
 }
}

void SegmentData::SetPartnerno(int no) {
 partnerno = no;
}

void SegmentData::SetInsideAbove(bool b) {
 insideAbove = b;
}

void SegmentData::SetCoverageno(int no) {
 coverageno = no;
}

void SegmentData::SetCycleno(int no) {
 cycleno = no;
}

void SegmentData::SetFaceno(int no) {
 faceno = no;
}

void SegmentData::SetEdgeNo(int no) {
 edgeno = no;
}

Line2::Line2() {};

Line2::Line2(const bool def, bool ldp, int xl, int yl, int xr, int yr,
  mpq_class pxl, mpq_class pyl, mpq_class pxr, mpq_class pyr) :
  StandardSpatialAttribute<2>(def), preciseCoordinates(0), segmentData(0) {
 SegmentData sd(xl, yl, xr, yr, ldp);
 sd.SetPreciseLeftX(preciseCoordinates, pxl);
 sd.SetPreciseLeftY(preciseCoordinates, pyl);
 sd.SetPreciseRightX(preciseCoordinates, pxr);
 sd.SetPreciseRightY(preciseCoordinates, pyr);
 segmentData.Append(sd);
 bbox = sd.BoundingBox();
 assert(IsDefined() && BoundingBox().IsDefined());
}

Line2::Line2(const Line2& l) :
  StandardSpatialAttribute<2>(l.IsDefined()), preciseCoordinates(
    l.preciseCoordinates.getSize()), segmentData(l.segmentData.Size()), bbox(
    l.bbox) {
 preciseCoordinates.copyFrom(l.preciseCoordinates);
 segmentData.copyFrom(l.segmentData);
 assert(IsDefined() == BoundingBox().IsDefined());
}


Line2::Line2(bool def) :
  StandardSpatialAttribute<2>(def), preciseCoordinates(0), segmentData(0),
  bbox(false) {
}

Line2& Line2::operator=(const Line2& l) {
 assert(l.ordered);
 SetDefined(l.IsDefined());
 preciseCoordinates.copyFrom(l.preciseCoordinates);
 segmentData.copyFrom(l.segmentData);
 noComponents = l.noComponents;
 ordered = true;
 bbox = l.bbox;
 return *this;
}

int Line2::getLeftGridX(int i) const {
 assert(0 <= i && i < segmentData.Size());
 SegmentData sd;
 segmentData.Get(i, &sd);
 return sd.getLeftX();
}

int Line2::getLeftGridY(int i) const {
 assert(0 <= i && i < segmentData.Size());
 SegmentData sd;
 segmentData.Get(i, &sd);
 return sd.getLeftY();
}

int Line2::getRightGridX(int i) const {
 assert(0 <= i && i < segmentData.Size());
 SegmentData sd;
 segmentData.Get(i, &sd);
 return sd.getRightX();
}

int Line2::getRightGridY(int i) const {
 assert(0 <= i && i < segmentData.Size());
 SegmentData sd;
 segmentData.Get(i, &sd);
 return sd.getRightY();
}

mpq_class Line2::getPreciseLeftX(int i) const {
 assert(0 <= i && i < segmentData.Size());
 SegmentData sd;
 segmentData.Get(i, &sd);
 return sd.getPreciseLeftX(preciseCoordinates);
}

mpq_class Line2::getPreciseLeftY(int i) const {
 assert(0 <= i && i < segmentData.Size());
 SegmentData sd;
 segmentData.Get(i, &sd);
 return sd.getPreciseLeftY(preciseCoordinates);
}

mpq_class Line2::getPreciseRightX(int i) const {
 assert(0 <= i && i < segmentData.Size());
 SegmentData sd;
 segmentData.Get(i, &sd);
 return sd.getPreciseRightX(preciseCoordinates);
}

mpq_class Line2::getPreciseRightY(int i) const {
 assert(0 <= i && i < segmentData.Size());
 SegmentData sd;
 assert(segmentData.Get(i, &sd));

 return sd.getPreciseRightY(preciseCoordinates);
}

void Line2::get(int i, SegmentData& sd) const {
 assert(0 <= i && i <= segmentData.Size());
 segmentData.Get(i, sd);
}

void Line2::addSegment(bool ldp, int leftX, int leftY, int rightX, int rightY,
  mpq_class pLeftX, mpq_class pLeftY, mpq_class pRightX, mpq_class pRightY,
  int edgeNo) {
 SegmentData sd(leftX, leftY, rightX, rightY, ldp, edgeNo);
 sd.addPreciseValues(preciseCoordinates, pLeftX, pLeftY, pRightX, pRightY);
 segmentData.Append(sd);
}

void Line2::addSegment(bool ldp, Point2* lp, Point2* rp, int edgeno) {
 SegmentData sd(lp, rp, ldp, edgeno, preciseCoordinates);
 segmentData.Append(sd);
}


bool Line2::IsLeftDomPoint(int i) const {
 assert(0 <= i && i < segmentData.Size());
 SegmentData sd;
 assert(segmentData.Get(i, &sd));

 return sd.IsLeftDomPoint();
}

int Line2::Size() const {
 return segmentData.Size();
}

const Rectangle<2> Line2::BoundingBox(const Geoid* geoid /*=0*/) const {
 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }
 return bbox;
}


inline void Line2::Destroy() {
 preciseCoordinates.destroy();
 segmentData.destroy();
}

void Line2::collectFace(int faceno, int startPos, DbArray<bool>& used) {
 set<int> extensionPos;

 used.Put(startPos, true);
 SegmentData sd1;
 SegmentData sd2;

 int pos = startPos;
 segmentData.Get(startPos, sd1);
 SegmentData Sd1 = sd1;
 int edgeno = 0;
 Sd1.SetInsideAbove(false);
 Sd1.SetCoverageno(0);
 Sd1.SetCycleno(0);
 Sd1.SetFaceno(faceno);
 Sd1.SetEdgeNo(edgeno);
 segmentData.Put(pos, Sd1);
 used.Put(pos, true);

 // get and Set the Partner
 int partner = Sd1.GetPartnerno();
 segmentData.Get(partner, sd2);
 SegmentData Sd2 = sd2;
 Sd2.SetFaceno(false);
 Sd2.SetCoverageno(0);
 Sd2.SetCycleno(0);
 Sd2.SetFaceno(faceno);
 Sd2.SetEdgeNo(edgeno);
 used.Put(partner, true);
 segmentData.Put(partner, Sd2);

 if (!bbox.IsDefined()) {
  bbox = sd1.BoundingBox();
 } else {
  bbox = bbox.Union(sd1.BoundingBox());
 }

 if (getUnusedExtension(pos, used) >= 0) {
  extensionPos.insert(pos);
 }
 if (getUnusedExtension(partner, used) >= 0) {
  extensionPos.insert(partner);
 }

 edgeno++;
 while (!extensionPos.empty()) {

  int spos = *(extensionPos.begin());
  pos = getUnusedExtension(spos, used);
  if (pos < 0) {
   extensionPos.erase(spos);
  } else { // extension found at position pos
   segmentData.Get(pos, sd1);
   Sd1 = (sd1);
   Sd1.SetInsideAbove(false);
   Sd1.SetCoverageno(0);
   Sd1.SetCycleno(0);
   Sd1.SetFaceno(faceno);
   Sd1.SetEdgeNo(edgeno);
   used.Put(pos, true);
   segmentData.Put(pos, Sd1);

   partner = Sd1.GetPartnerno();
   segmentData.Get(partner, sd2);
   Sd2 = (sd2);
   Sd2.SetInsideAbove(false);
   Sd2.SetCoverageno(0);
   Sd2.SetCycleno(0);
   Sd2.SetFaceno(faceno);
   Sd2.SetEdgeNo(edgeno);
   used.Put(partner, true);
   segmentData.Put(partner, Sd2);
   if (getUnusedExtension(partner, used) >= 0) {
    extensionPos.insert(partner);
   }
   bbox = bbox.Union(sd1.BoundingBox());
   edgeno++;
  }
 }
}

int Line2::getUnusedExtension(int startPos, const DbArray<bool>& used) const {
 SegmentData hs, hs2;
 segmentData.Get(startPos, hs);
 int pos = startPos - 1;
 bool done = false;
 bool u;
 // search on the left side
 while (pos >= 0 && !done) {
  segmentData.Get(pos, hs2);
  if (hs.GetDomGridXCoord() != hs2.GetDomGridXCoord()
    || hs.GetDomGridYCoord() != hs2.GetDomGridYCoord()
    || cmp(hs.GetDomPreciseXCoord(preciseCoordinates),
      hs2.GetDomPreciseXCoord(preciseCoordinates)) != 0
    || cmp(hs.GetDomPreciseYCoord(preciseCoordinates),
      hs2.GetDomPreciseYCoord(preciseCoordinates)) != 0) {
   done = true;
  } else {
   used.Get(pos, u);
   if (!u) {
    return pos;
   } else {
    pos--;
   }
  }
 }
 // search on the right side
 done = false;
 pos = startPos + 1;
 int size = segmentData.Size();
 while (!done && pos < size) {
  segmentData.Get(pos, hs);
  if (hs.GetDomGridXCoord() != hs2.GetDomGridXCoord()
    || hs.GetDomGridYCoord() != hs2.GetDomGridYCoord()
    || cmp(hs.GetDomPreciseXCoord(preciseCoordinates),
      hs2.GetDomPreciseXCoord(preciseCoordinates)) != 0
    || cmp(hs.GetDomPreciseYCoord(preciseCoordinates),
      hs2.GetDomPreciseYCoord(preciseCoordinates)) != 0) {
   done = true;
  } else {
   used.Get(pos, u);
   if (!u) {
    return pos;
   } else {
    pos++;
   }
  }
 }
 return -1;
}

/*
 ~ComputeComponents~

 Computes the length of this lines as well as its bounding box and the number
 of components of this line. Each Halfsegment is assigned to a face number
 (the component) and an egde number within this face.

*/
void Line2::computeComponents() {
 noComponents = 0;
 bbox.SetDefined(false);

 if (!IsDefined() || Size() == 0) {
  return;
 }

 DbArray<bool> used(segmentData.Size());

 for (int i = 0; i < segmentData.Size(); i++) {
  used.Append(false);
 }

 int faceno = 0;

 bool u;
 for (int i = 0; i < segmentData.Size(); i++) {
  used.Get(i, u);
  if (!(u)) { // an unused halfsegment
   collectFace(faceno, i, used);
   faceno++;
  }
 }
 noComponents = faceno;
 used.Destroy();
}

void Line2::StartBulkLoad() {
 ordered = false;
}

/*
 ~EndBulkLoad~

 Finishs the bulkload for a line. If this function is called,
 both halfSegments assigned to a segment of the line must be part
 of this line.

 The parameter ~sort~ can be set to __false__ if the halfsegments are
 already ordered using the falfSegment order.

 The parameter ~realminize~ can be set to __false__ if the line is
 already realminized, meaning each pair of different Segments has
 at most a common endpoint. Furthermore, the two halfsegments belonging
 to a segment must have the same edge number. The edge numbers must be
 in Range [0..Size()-1]. HalfSegments belonging to different segments
 must have different edge numbers.

 Only change one of the parameters if you exacly know what you do.
 Changing such parameters without fulifilling the conditions stated
 above may construct invalid line representations which again may
 produce a system crash within some operators.

*/
void Line2::EndBulkLoad(const bool sort /* = true*/,
  const bool realminize /* = true*/, const bool robust /* = false*/) {
 if (!IsDefined()) {
  Clear();
  SetDefined(false);
 }

 if (sort) {
  Sort();
 }
 if (Size() > 0) {
  if (realminize) {
   Line2* resultLine = new Line2(true);
   Realminize(*this, *resultLine, false);
   resultLine->Sort();
   this->CopyFrom(resultLine);
   resultLine->Destroy();
   delete resultLine;
  }
  SetPartnerNo();
 }
 computeComponents();

 segmentData.TrimToSize();
}

void Line2::Clear() {
 segmentData.clean();
 preciseCoordinates.clean();
 ordered = true;
 bbox.SetDefined(false);
 SetDefined(true);
}

bool Line2::SegmentIsVertical(int lx, mpq_class plx, int rx, mpq_class prx) {
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
int Line2::CompareSegment(const SegmentData& seg1,
  const SegmentData& seg2) const {

 int seg1DomGridX = seg1.GetDomGridXCoord();
 int seg1DomGridY = seg1.GetDomGridYCoord();
 int seg1SecGridX = seg1.GetSecGridXCoord();
 int seg1SecGridY = seg1.GetSecGridYCoord();

 mpq_class seg1DomPreciseX = seg1.GetDomPreciseXCoord(preciseCoordinates);
 mpq_class seg1DomPreciseY = seg1.GetDomPreciseYCoord(preciseCoordinates);

 int seg2DomGridX = seg2.GetDomGridXCoord();
 int seg2DomGridY = seg2.GetDomGridYCoord();
 int seg2SecGridX = seg2.GetSecGridXCoord();
 int seg2SecGridY = seg2.GetSecGridYCoord();

 mpq_class seg2DomPreciseX = seg2.GetDomPreciseXCoord(preciseCoordinates);
 mpq_class seg2DomPreciseY = seg2.GetDomPreciseYCoord(preciseCoordinates);

 int cmpDomPreciseX = cmp(seg1DomPreciseX, seg2DomPreciseX);

 //comparing the dominating points
 if ((seg1DomGridX < seg2DomGridX)
   || ((seg1DomGridX == seg2DomGridX) && (cmpDomPreciseX < 0))
   || ((seg1DomGridX == seg2DomGridX) && (cmpDomPreciseX == 0)
     && (seg1DomGridY < seg2DomGridY))
   || ((seg1DomGridX == seg2DomGridX) && (cmpDomPreciseX == 0)
     && (seg1DomGridY == seg2DomGridY)
     && (cmp(seg1DomPreciseY, seg2DomPreciseY) < 0))) {
  //The dominating point of ~this~ is less than
  //the dominating point of ~s~.
  return -1;
 } else {
  if ((seg1DomGridX > seg2DomGridX)
    || ((seg1DomGridX == seg2DomGridX) && (cmpDomPreciseX > 0))
    || ((seg1DomGridX == seg2DomGridX) && (cmpDomPreciseX == 0)
      && (seg1DomGridY > seg2DomGridY))
    || ((seg1DomGridX == seg2DomGridX) && (cmpDomPreciseX == 0)
      && (seg1DomGridY == seg2DomGridY)
      && (cmp(seg1DomPreciseY, seg2DomPreciseY) > 0))) {
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
  mpq_class seg1SecPreciseX = seg1.GetSecPreciseXCoord(preciseCoordinates);
  mpq_class seg1SecPreciseY = seg1.GetSecPreciseYCoord(preciseCoordinates);

  mpq_class seg2SecPreciseX = seg2.GetSecPreciseXCoord(preciseCoordinates);
  mpq_class seg2SecPreciseY = seg2.GetSecPreciseYCoord(preciseCoordinates);

  bool v1 = ((seg1DomGridX == seg1SecGridX)
    && (seg1DomPreciseX == seg1SecPreciseX));
  bool v2 = ((seg2DomGridX == seg2SecGridX)
    && (seg2DomPreciseX == seg2SecPreciseX));

  if (v1 && v2) {
   //both halfsegments are vertical
   int cmpThisY = cmp(seg1SecPreciseY, seg1DomPreciseY);
   int cmpSY = cmp(seg2SecPreciseY, seg2DomPreciseY);
   if ((((seg1SecGridY > seg1DomGridY)
     || ((seg1SecGridY == seg1DomGridY) && cmpThisY > 0))
     && ((seg2SecGridY > seg2DomGridY)
       || ((seg2SecGridY == seg2DomGridY) && cmpSY > 0)))
     || (((seg1SecGridY < seg1DomGridY)
       || ((seg1SecGridY == seg1DomGridY) && cmpThisY < 0))
       && ((seg2SecGridY < seg2DomGridY)
         || ((seg2SecGridY == seg2DomGridY) && cmpSY < 0)))) {
    //The y-value of the second points of both
    //halfsegments are greater than their
    //dominating points or the y-value of the
    //second points of the halfsegments are less
    //than their dominating points.
    int cmpSecPreciseX = cmp(seg1SecPreciseX, seg2SecPreciseX);
    if ((seg1SecGridX < seg2SecGridX)
      || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX < 0))
      || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
        && (seg1SecGridY < seg2SecGridY))
      || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
        && (seg1SecGridY == seg2SecGridY)
        && (cmp(seg1SecPreciseX, seg2SecPreciseX) < 0))) {
     //The second point of ~this~ is less
     //than the
     //second point of ~s~.
     return -1;
    }
    if ((seg1SecGridX > seg2SecGridX)
      || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX > 0))
      || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
        && (seg1SecGridY > seg2SecGridY))
      || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
        && (seg1SecGridY == seg2SecGridY)
        && (cmp(seg1SecPreciseX, seg2SecPreciseX) > 0))) {
     //The second point of ~this~ is greater
     //than the second point of ~s~.
     return 1;
    }
    return 0;
   } else {
    if ((seg1SecGridY > seg1DomGridY)
      || ((seg1SecGridY == seg1DomGridY)
        && (cmp(seg1SecPreciseY, seg1DomPreciseY) > 0))) {
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
        && cmp(seg1SecPreciseY, seg1DomPreciseY) > 0)) {
     //the y-value of the second point of
     //~this~ is greater than
     //the dominating point of ~this~
     if (seg1.IsLeftDomPoint())
      return 1;
     return -1;
    } else {
     if ((seg1SecGridY < seg1DomGridY)
       || ((seg1SecGridY == seg1DomGridY)
         && cmp(seg1SecPreciseY, seg1DomPreciseY) < 0))

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
         && cmp(seg2SecPreciseY, seg2DomPreciseY) > 0))
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
         && cmp(seg2SecPreciseY, seg2DomPreciseY) < 0)) {
      //the y-value of the second
      //point of ~s~ is greater
      //than the dominating point
      //of ~s~
      if (seg1.IsLeftDomPoint())
       return 1;
      return -1;
     }
    } else {
     mpq_class xd = seg1DomPreciseX + seg1DomGridX;
     mpq_class yd = seg1DomPreciseY + seg1DomGridY;
     mpq_class xs = seg1SecPreciseX + seg1SecGridX;
     mpq_class ys = seg1SecPreciseY + seg1SecGridY;
     mpq_class Xd = seg2DomPreciseX + seg2DomGridX;
     mpq_class Yd = seg2DomPreciseY + seg2DomGridY;
     mpq_class Xs = seg2SecPreciseX + seg2SecGridX;
     mpq_class Ys = seg2SecPreciseY + seg2SecGridY;

     mpq_class seg1Slope = (yd - ys) / (xd - xs);
     mpq_class seg2Slope = (Yd - Ys) / (Xd - Xs);
     int cmpSlope = cmp(seg1Slope, seg2Slope);
     if (cmpSlope < 0) {
      return -1;
     }
     if (cmpSlope > 0) {
      return 1;
     }
     int cmpSecPreciseX = cmp(seg1SecPreciseX, seg2SecPreciseX);
     if ((seg1SecGridX < seg2SecGridX)
       || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX < 0))
       || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
         && (seg1SecGridY < seg2SecGridY))
       || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
         && (seg1SecGridY == seg2SecGridY)
         && (cmp(seg1SecPreciseX, seg2SecPreciseX) < 0))) {
      //The second point of ~this~ is less
      //than thesecond point of ~s~
      return -1;
     }
     if ((seg1SecGridX > seg2SecGridX)
       || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX > 0))
       || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
         && (seg1SecGridY > seg2SecGridY))
       || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
         && (seg1SecGridY == seg2SecGridY)
         && (cmp(seg1SecPreciseX, seg2SecPreciseX) > 0))) {
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
int Line2::CompareSegment2(const SegmentData& seg1, const SegmentData& seg2,
  const Flob& preciseCoordOfSeg2) const {

 int seg1DomGridX = seg1.GetDomGridXCoord();
 int seg1DomGridY = seg1.GetDomGridYCoord();
 int seg1SecGridX = seg1.GetSecGridXCoord();
 int seg1SecGridY = seg1.GetSecGridYCoord();

 mpq_class seg1DomPreciseX = seg1.GetDomPreciseXCoord(preciseCoordinates);
 mpq_class seg1DomPreciseY = seg1.GetDomPreciseYCoord(preciseCoordinates);

 int seg2DomGridX = seg2.GetDomGridXCoord();
 int seg2DomGridY = seg2.GetDomGridYCoord();
 int seg2SecGridX = seg2.GetSecGridXCoord();
 int seg2SecGridY = seg2.GetSecGridYCoord();

 mpq_class seg2DomPreciseX = seg2.GetDomPreciseXCoord(preciseCoordOfSeg2);
 mpq_class seg2DomPreciseY = seg2.GetDomPreciseYCoord(preciseCoordOfSeg2);

 int cmpDomPreciseX = cmp(seg1DomPreciseX, seg2DomPreciseX);

 //comparing the dominating points
 if ((seg1DomGridX < seg2DomGridX)
   || ((seg1DomGridX == seg2DomGridX) && (cmpDomPreciseX < 0))
   || ((seg1DomGridX == seg2DomGridX) && (cmpDomPreciseX == 0)
     && (seg1DomGridY < seg2DomGridY))
   || ((seg1DomGridX == seg2DomGridX) && (cmpDomPreciseX == 0)
     && (seg1DomGridY == seg2DomGridY)
     && (cmp(seg1DomPreciseY, seg2DomPreciseY) < 0))) {
  //The dominating point of ~this~ is less than the
  //dominating point of ~s~.
  return -1;
 } else {
  if ((seg1DomGridX > seg2DomGridX)
    || ((seg1DomGridX == seg2DomGridX) && (cmpDomPreciseX > 0))
    || ((seg1DomGridX == seg2DomGridX) && (cmpDomPreciseX == 0)
      && (seg1DomGridY > seg2DomGridY))
    || ((seg1DomGridX == seg2DomGridX) && (cmpDomPreciseX == 0)
      && (seg1DomGridY == seg2DomGridY)
      && (cmp(seg1DomPreciseX, seg2DomPreciseX) > 0))) {
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
  mpq_class seg1SecPreciseX = seg1.GetSecPreciseXCoord(preciseCoordinates);
  mpq_class seg1SecPreciseY = seg1.GetSecPreciseYCoord(preciseCoordinates);

  mpq_class seg2SecPreciseX = seg2.GetSecPreciseXCoord(preciseCoordOfSeg2);
  mpq_class seg2SecPreciseY = seg2.GetSecPreciseYCoord(preciseCoordOfSeg2);

  bool v1 = ((seg1DomGridX == seg1SecGridX)
    && (seg1DomPreciseX == seg1SecPreciseX));
  bool v2 = ((seg2DomGridX == seg2SecGridX)
    && (seg2DomPreciseX == seg2SecPreciseX));

  if (v1 && v2) {
   //both halfsegments are vertical
   int cmpThisY = cmp(seg1SecPreciseY, seg1DomPreciseY);
   int cmpSY = cmp(seg2SecPreciseY, seg2DomPreciseY);
   if ((((seg1SecGridY > seg1DomGridY)
     || ((seg1SecGridY == seg1DomGridY) && cmpThisY > 0))
     && ((seg2SecGridY > seg2DomGridY)
       || ((seg2SecGridY == seg2DomGridY) && cmpSY > 0)))
     || (((seg1SecGridY < seg1DomGridY)
       || ((seg1SecGridY == seg1DomGridY) && cmpThisY < 0))
       && ((seg2SecGridY < seg2DomGridY)
         || ((seg2SecGridY == seg2DomGridY) && cmpSY < 0)))) {
    //The y-value of the second points of both
    //halfsegments are greater than their
    //dominating points or the y-value of the
    //second points of the halfsegments are
    //lessthan their dominating points.
    int cmpSecPreciseX = cmp(seg1SecPreciseX, seg2SecPreciseX);
    if ((seg1SecGridX < seg2SecGridX)
      || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX < 0))
      || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
        && (seg1SecGridY < seg2SecGridY))
      || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
        && (seg1SecGridY == seg2SecGridY)
        && (cmp(seg1SecPreciseX, seg2SecPreciseX) < 0))) {
     //The second point of ~this~ is less
     //than the second point of ~s~.
     return -1;
    }
    if ((seg1SecGridX > seg2SecGridX)
      || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX > 0))
      || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
        && (seg1SecGridY > seg2SecGridY))
      || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
        && (seg1SecGridY == seg2SecGridY)
        && (cmp(seg1SecPreciseX, seg2SecPreciseX) > 0))) {
     //The second point of ~this~ is greater
     //than the second point of ~s~.
     return 1;
    }
    return 0;
   } else {
    if ((seg1SecGridY > seg1DomGridY)
      || ((seg1SecGridY == seg1DomGridY)
        && (cmp(seg1SecPreciseY, seg1DomPreciseY) > 0))) {
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
        && cmp(seg1SecPreciseY, seg1DomPreciseY) > 0)) {
     //the y-value of the second point
     //of ~this~ is greater than
     //the dominating point of ~this~
     if (seg1.IsLeftDomPoint())
      return 1;
     return -1;
    } else {
     if ((seg1SecGridY < seg1DomGridY)
       || ((seg1SecGridY == seg1DomGridY)
         && cmp(seg1SecPreciseY, seg1DomPreciseY) < 0))

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
         && cmp(seg2SecPreciseY, seg2DomPreciseY) > 0))
       //the y-value of the second
       //point of ~s~ is greater than
       //the dominating point of ~s~
       {
      if (seg1.IsLeftDomPoint())
       return -1;
      return 1;
     } else if ((seg2SecGridY < seg2DomGridY)
       || ((seg2SecGridY == seg2DomGridY)
         && cmp(seg2SecPreciseY, seg2DomPreciseY) < 0)) {
      //the y-value of the second
      //point of ~s~ is greater than
      //the dominating point of ~s~
      if (seg1.IsLeftDomPoint())
       return 1;
      return -1;
     }
    } else {
     mpq_class xd = seg1DomPreciseX + seg1DomGridX;
     mpq_class yd = seg1DomPreciseY + seg1DomGridX;
     mpq_class xs = seg1SecPreciseX + seg1SecGridX;
     mpq_class ys = seg1SecPreciseY + seg1SecGridY;
     mpq_class Xd = seg2DomPreciseX + seg2DomGridX;
     mpq_class Yd = seg2DomPreciseY + seg2DomGridY;
     mpq_class Xs = seg2SecPreciseX + seg2SecGridX;
     mpq_class Ys = seg2SecPreciseY + seg2SecGridY;

     mpq_class seg1Slope = (yd - ys) / (xd - xs);
     mpq_class seg2Slope = (Yd - Ys) / (Xd - Xs);

     int cmpSlope = cmp(seg1Slope, seg2Slope);
     if (cmpSlope < 0) {
      return -1;
     }
     if (cmpSlope > 0) {
      return 1;
     }

     int cmpSecPreciseX = cmp(seg1SecPreciseX, seg2SecPreciseX);

     if ((seg1SecGridX < seg2SecGridX)
       || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX < 0))
       || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
         && (seg1SecGridY < seg2SecGridY))
       || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
         && (seg1SecGridY == seg2SecGridY)
         && (cmp(seg1SecPreciseX, seg2SecPreciseX) < 0))) {
      //The second point of ~this~ is
      //less than
      //the second point of ~s~
      return -1;
     }
     if ((seg1SecGridX > seg2SecGridX)
       || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX > 0))
       || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
         && (seg1SecGridY > seg2SecGridY))
       || ((seg1SecGridX == seg2SecGridX) && (cmpSecPreciseX == 0)
         && (seg1SecGridY == seg2SecGridY)
         && (cmp(seg1SecPreciseX, seg2SecPreciseX) > 0))) {
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
void Line2::Sort() {
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
void Line2::MergeSort(int startindex, int endindex) {
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
void Line2::Merge(int startindex, int divide, int endindex) {
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

void Line2::SetPartnerNo() {
 if (!IsDefined() || segmentData.Size() == 0) {
  return;
 }

 // reserve a slot for each edgeno
 int tmpsize = (segmentData.Size() + 1) / 2;

 DbArray<int> TMP(tmpsize);
 // intialize the array (it's only needed for
 // wrong sorted halfsegments)
 for (int i = 0; i < tmpsize + 1; i++) {
  TMP.Put(i, -1);
 }

 SegmentData seg1;
 SegmentData seg2;
 int lpp;
 for (int i = 0; i < segmentData.Size(); i++) {
  segmentData.Get(i, seg1);
  TMP.Get(seg1.GetEdgeno(), lpp);
  if (seg1.IsLeftDomPoint()) {
   if (lpp >= 0) {
    int leftpos = lpp;
    SegmentData right = seg1;
    right.SetPartnerno(leftpos);
    right.SetInsideAbove(false);
    right.SetCoverageno(0);
    right.SetCycleno(0);
    right.SetFaceno(0);
    segmentData.Get(leftpos, seg2);
    SegmentData left = seg2;
    left.SetPartnerno(i);
    left.SetInsideAbove(false);
    left.SetCoverageno(0);
    left.SetCycleno(0);
    left.SetFaceno(0);
    segmentData.Put(i, right);
    segmentData.Put(leftpos, left);
   } else {
    // normal case, put number to tmp
    TMP.Put(seg1.GetEdgeno(), i);
   }
  } else { // RightDomPoint
   if (lpp < 0) {
    TMP.Put(seg1.GetEdgeno(), i);
   } else {
    int leftpos = lpp;
    SegmentData right = seg1;
    right.SetPartnerno(leftpos);
    right.SetInsideAbove(false);
    right.SetCoverageno(0);
    right.SetCycleno(0);
    right.SetFaceno(0);
    segmentData.Get(leftpos, seg2);
    SegmentData left = seg2;
    left.SetPartnerno(i);
    left.SetInsideAbove(false);
    left.SetCoverageno(0);
    left.SetCycleno(0);
    left.SetFaceno(0);
    segmentData.Put(i, right);
    segmentData.Put(leftpos, left);
   }
  }
 }
 TMP.Destroy();
}

void Line2::unionOP(Line2& l2, Line2& result, const Geoid* geoid/*=0*/) {
 p2d::SetOp(*this, l2, result, union_op, geoid);
}

void Line2::intersection(Line2& l2, Line2& result, const Geoid* geoid/*=0*/) {
 p2d::SetOp(*this, l2, result, intersection_op, geoid);
}

void Line2::minus(Line2& l2, Line2& result, const Geoid* geoid/*=0*/) {
 p2d::SetOp(*this, l2, result, difference_op, geoid);
}

bool Line2::intersect(Line2& l2, const Geoid* geoid/*=0*/) {
 return p2d::intersects(*this, l2, geoid);
}

void Line2::crossings(Line2& l2, Points2& result, const Geoid* geoid/*=0*/) {
 p2d::crossings(*this, l2, result, geoid);
}

void convertLineToLine2(Line& l, Line2& result) {
 result.Clear();

 if (!l.IsDefined()) {
  result.SetDefined(false);
  return;
 }

 HalfSegment hs;
 int edgeNo = 0;

 result.StartBulkLoad();

 for (int i = 0; i < l.Size(); i++) {
  l.Get(i, hs);
  if (hs.IsLeftDomPoint()) {

   int glx;
   int gly;
   mpq_class plx(0);
   mpq_class ply(0);
   createValue(hs.GetDomPoint().GetX(), glx, plx);
   createValue(hs.GetDomPoint().GetY(), gly, ply);

   int grx;
   int gry;
   mpq_class prx(0);
   mpq_class pry(0);
   createValue(hs.GetSecPoint().GetX(), grx, prx);
   createValue(hs.GetSecPoint().GetY(), gry, pry);

   result.addSegment(true, glx, gly, grx, gry, plx, ply, prx, pry, edgeNo);
   result.addSegment(false, grx, gry, glx, gly, prx, pry, plx, ply, edgeNo);
   edgeNo++;
  }
 }

 result.EndBulkLoad(true, false);
 assert(result.IsDefined() && result.BoundingBox().IsDefined());
}

/*
 4.4 ~Sizeof~-function

*/
inline size_t Line2::Sizeof() const {
 return (sizeof(*this));

}

/*
 4.4 ~HashValue~-function

*/
inline size_t Line2::HashValue() const {
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
inline void Line2::CopyFrom(const Attribute* right) {
 *this = *((const Line2 *) right);
}

/*
 4.4 ~Compare~-function

*/
inline int Line2::Compare(const Attribute *arg) const {
 const Line2 &l = *((const Line2*) arg);

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

 int index = 0;
 int result = 0;

 while (index < Size()) {
  SegmentData sd1;
  SegmentData sd2;
  segmentData.Get(index, sd1);
  l.get(index, sd2);
  result = CompareSegment2(sd1, sd2, *(l.getPreciseCoordinates()));
  if (result != 0) {
   return result;
  }
  index++;
 }
 return 0;
}

bool Line2::Adjacent(const Attribute *arg) const {
 return false;
}

Line2* Line2::Clone() const {
  return new Line2(*this);
 }

/*
 4.4 ~CloneLine2~-function

*/
Word Line2::CloneLine2(const ListExpr typeInfo, const Word& w) {
 return SetWord(new Line2(*((Line2 *) w.addr)));
}

/*
 4.4 ~CastLine2~-function

*/
void* Line2::CastLine2(void* addr) {
 return (new (addr) Line2());
}

/*
 4.4 ~SizeOfLine2~-function

*/
int Line2::SizeOfLine2() {
 return sizeof(Line2);
}

/*
 4.4 ~CreateLine2~-function

*/
Word Line2::CreateLine2(const ListExpr typeInfo) {
 return SetWord(new Line2(false));
}

/*
 4.4 ~DeleteLine2~-function

*/
void Line2::DeleteLine2(const ListExpr typeInfo, Word& w) {
 Line2 *l = (Line2 *) w.addr;
 (l->GetFLOB(0))->destroy();
 (l->GetFLOB(1))->destroy();
 l->SetDefined(false);
 delete l;
 w.addr = 0;
}

/*
 4.4 ~CloseLine2~-function

*/
void Line2::CloseLine2(const ListExpr typeInfo, Word& w) {
 delete (Line2*) w.addr;
 w.addr = 0;
}

/*
 4.4 ~Line2Property~-function

*/
ListExpr Line2::Line2Property() {

 ListExpr ListRepr = nl->TextAtom();
 nl->AppendText(ListRepr, "(<segment>*), where <segment> is "
   "((<xl> <xr> ( <pxl> <pyl>))(<xr> <yr> "
   "( <pxr> <pyr>))))");

 return nl->TwoElemList(
   nl->FourElemList(nl->StringAtom("Signature"),
     nl->StringAtom("Example Type List"), nl->StringAtom("List Rep"),
     nl->StringAtom("Example List")),
   nl->FourElemList(nl->StringAtom("-> DATA"),
     nl->StringAtom(Line2::BasicType()), ListRepr,
     nl->StringAtom("(((1 1 ('1/4' '3/4'))(3 4 "
       "('1/8' '1/10'))))")));
}

/*
 4.4 ~CheckLine2~-function

*/
bool Line2::CheckLine2(ListExpr type, ListExpr& errorInfo) {
 return (nl->IsEqual(type, Line2::BasicType()));
}

/*
 4.4 ~Distance~-function

*/
double Line2::Distance(const Rectangle<2>& rect,
  const Geoid* geoid/*=0*/) const {
//TODO Distance
 return 1.0;
}

/*
 4.4 ~Empty~-function

*/
bool Line2::IsEmpty() const {
 return ((!IsDefined()) || segmentData.Size() == 0);
}

bool Line2::Intersects(const Rectangle<2>& rect,
  const Geoid* geoid/*=0*/) const {
 assert( !IsEmpty() ); // includes !undef
 assert( this->ordered );
 assert( rect.IsDefined() );
 assert( !geoid || geoid->IsDefined() );

 if(!BoundingBox().Intersects(rect,geoid)){
    return false;
 }

 Line2* rectangle = new Line2(0);
 Point2* p1;
 Point2* p2;
 Point2* p3;
 Point2* p4;
 createPoint2(rect.MinD(0), rect.MinD(1), &p1);
 createPoint2(rect.MaxD(0), rect.MinD(1), &p2);
 createPoint2(rect.MaxD(0), rect.MaxD(1), &p3);
 createPoint2(rect.MinD(0), rect.MaxD(1), &p4);

 rectangle->addSegment(true, p1, p2, 0);
 rectangle->addSegment(false, p1, p2, 0);

 rectangle->addSegment(true, p2, p3, 1);
 rectangle->addSegment(false, p2, p3, 1);

 rectangle->addSegment(true, p1, p4, 2);
 rectangle->addSegment(false, p1, p4, 2);

 rectangle->addSegment(true, p4, p3, 3);
 rectangle->addSegment(false, p4, p3, 3);

bool intersects = p2d::intersects(*this, *rectangle, geoid);

delete p1;
delete p2;
delete p3;
delete p4;
delete rectangle;

return intersects;
}


} // end of namespace p2d

#endif /* _Line2_CPP*/
