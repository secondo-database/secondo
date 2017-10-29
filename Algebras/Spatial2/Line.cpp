/*
----
This file is part of SECONDO.

Copyright (C) 2016, University in Hagen,
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


*/

#include "Line.h"
#include "Region.h"
#include "Curve.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "RectangleBB.h"
#include "SpatialAlgebra.h"

#include <vector>

namespace salr {

  Line2::Line2(const Line2 &other) :
    Attribute(other.IsDefined()),
    coords(other.coords.Size()),
    pointTypes(other.pointTypes.Size()),
    hasQuads(other.hasQuads){
    for (int i = 0; i < other.pointTypes.Size(); i++) {
      appendType(other.getPointType(i));
    }
    for (int i = 0; i < other.coords.Size(); i++) {
      appendCoord(other.getCoord(i));
    }
  }

  Line2::Line2(int initialCapacity) :
    Attribute(true),
    coords(initialCapacity * 2),
    pointTypes(initialCapacity),
    hasQuads(false)
  {}

  Line2::Line2(const Line& l) :
    Attribute(l.IsDefined()),
    coords(l.Size()),
    pointTypes(l.Size()),
    hasQuads(false)
  {
    if (IsDefined()) {
      HalfSegment hs;
      l.Get(0, hs);
      double x0 = hs.GetLeftPoint().GetX();
      double y0 = hs.GetLeftPoint().GetY();
      double lastx = x0;
      double lasty = y0;
      moveTo(x0, y0);

      for(int i = 0; i < l.Size(); i=i+2) {
        l.Get(i, hs);
        double x0 = hs.GetLeftPoint().GetX();
        double y0 = hs.GetLeftPoint().GetY();
        double x1 = hs.GetRightPoint().GetX();
        double y1 = hs.GetRightPoint().GetY();
        if(x0 == lastx || y0 == lasty) {
          lineTo(x1, y1);
          lastx = x1;
          lasty = y1;
        } else if(x1 == lastx || y1 == lasty) {
          lineTo(x0, y0);
          lastx = x0;
          lasty = y0;
        } else {
          moveTo(x0, y0);
          lineTo(x1, y1);
          lastx = x1;
          lasty = y1;
        }
      }
    }
  }

  Line2::Line2(ListExpr LE, int s) :
    Attribute(true),
    coords(s),
    pointTypes(s),
    hasQuads(false)
  {
    while(!nl->IsEmpty(LE)){
      ListExpr Face = nl->First(LE);
      LE = nl->Rest(LE);
      while(!nl->IsEmpty(Face)){
        ListExpr Cycle = nl->First(Face);
        Face = nl->Rest(Face);
        bool isFirstPoint = true;
        while(!nl->IsEmpty(Cycle)){
          ListExpr P = nl->First(Cycle);
          Cycle = nl->Rest(Cycle);
          double x1 = nl->RealValue(nl->First(P));
          double y1 = nl->RealValue(nl->Second(P));
          if(isFirstPoint){
            moveTo(x1,y1);
            isFirstPoint = false;
          }else{
            lineTo(x1,y1);
          }
        }
        closeLine();
      }
    }
  }

  Line2::Line2(const Region2 &r) :
    Attribute(r.IsDefined()),
    coords(r.coords.Size()),
    pointTypes(r.pointTypes.Size()),
    hasQuads(false){
    for (int i = 0; i < r.pointTypes.Size(); i++) {
      appendType(r.getPointType(i));
    }
    for (int i = 0; i < r.coords.Size(); i++) {
      appendCoord(r.getCoord(i));
    }
  }

  Line2::~Line2() {
  }

  const std::string Line2::BasicType() {
    return "line2";
  }

  const bool Line2::checkType(const ListExpr type) {
    return listutils::isSymbol(type, BasicType());
  }

  int Line2::NumOfFLOBs() const {
    return 2;
  }

  Flob *Line2::GetFLOB(const int i) {
    assert(i >= 0 && i < NumOfFLOBs());
    if (i == 1) {
      return &coords;
    } else {
      return &pointTypes;
    }
  }

  int Line2::Compare(const Attribute *arg) const {
    const Line2 &l = *((const Line2 *) arg);
    if (!IsDefined() && !l.IsDefined()) {
      return 0;
    }
    if (!IsDefined()) {
      return -1;
    }
    if (!l.IsDefined()) {
      return 1;
    }

    if (coords.Size() > l.coords.Size())
      return 1;
    if (coords.Size() < l.coords.Size())
      return -1;
    return 0;
  }

  bool Line2::Adjacent(const Attribute *arg) const {
    return false;
  }

  size_t Line2::Sizeof() const {
    return sizeof(*this);
  }

  size_t Line2::HashValue() const {
    if (pointTypes.Size() == 0) return 0;

    size_t h = 17 * (coords.Size() + pointTypes.Size());
    for (int i = 0; i < coords.Size(); i++) {
      h += (size_t)((5 * getCoord(i)));
    }
    for (int i = 0; i < pointTypes.Size(); i++) {
      h += (size_t)((7 * getPointType(i)));
    }
    return h;
  }

  void Line2::CopyFrom(const Attribute *arg) {
    *this = *((Line2 *) arg);
  }

  Attribute *Line2::Clone() const {
    return new Line2(*this);
  }

  bool Line2::CheckKind(ListExpr type, ListExpr &errorInfo) {
    return checkType(type);
  }

  bool Line2::ReadFrom(ListExpr LE, const ListExpr typeInfo) {
    if (listutils::isSymbolUndefined(LE)) {
      SetDefined(false);
      return true;
    }

    if (!nl->HasLength(LE, 2)) {
      cmsg.inFunError("List in ReadFrom-Function wrong size");
      return false;
    }

    int numTypes = nl->ListLength(nl->First(LE));
    int numCoords = nl->ListLength(nl->Second(LE));
    if ((numTypes == 0 && numCoords != 0) ||
        (numTypes != 0 && numCoords == 0)) {
      cmsg.inFunError("Either both list length must be 0 or none");
      return false;
    }
    if (numTypes == 0) {
      this->hasInitialMove();
    }

    if (this->pointTypes.Size() > 0
        && this->getPointType(0) != Curve::SEG_MOVETO) {
      cmsg.inFunError("First pointType must be a moveTo");
      return false;
    }
    ListExpr pointTypesList = nl->First(LE);
    for (int i = 1; i <= numTypes; i++) {
      ListExpr current = nl->Nth(i, pointTypesList);
      if (!(nl->IsAtom(current) && nl->AtomType(current) == IntType)) {
        cmsg.inFunError("All pointType values must be of IntType");
        return false;
      }
      int pointType = nl->IntValue(current);
      if(pointType != Curve::SEG_MOVETO && pointType != Curve::SEG_LINETO
         && pointType != Curve::SEG_QUADTO && pointType != Curve::SEG_CLOSE) {
        cmsg.inFunError("Allowed pointType values: 0, 1, 2 or 4");
        return false;
      }
      this->appendType(pointType);
      if(pointType == Curve::SEG_QUADTO) {
        hasQuads = true;
      }
    }
    ListExpr coordsList = nl->Second(LE);
    for (int i = 1; i <= numCoords; i++) {
      ListExpr current = nl->Nth(i, coordsList);
      if (!listutils::isNumeric(current)) {
        cmsg.inFunError("All coord values must be of numeric");
        return false;
      } else if (nl->AtomType(current) == IntType) {
        this->appendCoord((double)nl->IntValue(current));
      } else if (nl->AtomType(current) == RealType) {
        this->appendCoord(nl->RealValue(current));
      }
    }
    SetDefined(true);
    return true;
  }

  ListExpr Line2::ToListExpr(ListExpr typeInfo) const {
    if (!IsDefined()) {
      return listutils::getUndefined();
    }

    ListExpr last;
    ListExpr typesList;
    for (int i = 0; i < pointTypes.Size(); i++) {
      if (i == 0) {
        typesList = last = nl->OneElemList(nl->IntAtom(this->getPointType(i)));
      } else {
        last = nl->Append(last, nl->IntAtom(this->getPointType(i)));
      }
    }

    ListExpr coordsList;
    for (int i = 0; i < coords.Size(); i++) {
      if (i == 0) {
        coordsList = last = nl->OneElemList(nl->RealAtom(this->getCoord(i)));
      } else {
        last = nl->Append(last, nl->RealAtom(this->getCoord(i)));
      }
    }

    ListExpr result = nl->TwoElemList(typesList, coordsList);
    return result;
  }

  void Line2::moveTo(double x, double y) {
    if (pointTypes.Size() > 0 &&
        getPointType(pointTypes.Size() - 1) == Curve::SEG_MOVETO) {
      coords.Put(coords.Size() - 2, x);
      coords.Put(coords.Size() - 1, y);
    } else {
      pointTypes.Append((int) Curve::SEG_MOVETO);
      coords.Append(x);
      coords.Append(y);
    }
  }

  void Line2::lineTo(double x, double y) {
    hasInitialMove();
    pointTypes.Append((int) Curve::SEG_LINETO);
    coords.Append(x);
    coords.Append(y);
  }

  void Line2::quadTo(double x1, double y1, double x2, double y2) {
    hasInitialMove();
    pointTypes.Append((int) Curve::SEG_QUADTO);
    coords.Append(x1);
    coords.Append(y1);
    coords.Append(x2);
    coords.Append(y2);
    hasQuads = true;
  }

  void Line2::closeLine() {
    if (pointTypes.Size() == 0 ||
        getPointType(pointTypes.Size() - 1) != Curve::SEG_CLOSE) {
      hasInitialMove();
      pointTypes.Append((int) Curve::SEG_CLOSE);
    }
  }

  RectangleBB* Line2::getBounds() {
    double x1, y1, x2, y2;
    int i = coords.Size();
    if (i > 0) {
      y1 = y2 = getCoord(--i);
      x1 = x2 = getCoord(--i);
      while (i > 0) {
        double y = getCoord(--i);
        double x = getCoord(--i);
        if (x < x1) x1 = x;
        if (y < y1) y1 = y;
        if (x > x2) x2 = x;
        if (y > y2) y2 = y;
      }
    } else {
      x1 = y1 = x2 = y2 = 0.0;
    }
    return new RectangleBB(x1, y1, x2 - x1, y2 - y1);
  }

  bool Line2::intersects(RectangleBB *bbox) {
    if (bbox->width <= 0 || bbox->height <= 0) {
      return false;
    }
    int crossings = rectCrossings(bbox->x, bbox->y,
                                  bbox->x+bbox->width, bbox->y+bbox->height);
    return (crossings == Curve::RECT_INTERSECTS ||
            (crossings & -1) != 0);
  }

  void Line2::nextSegment(int offset, int pointType, double *result) const {
    if(pointType == Curve::SEG_MOVETO
       || pointType == Curve::SEG_LINETO
       || pointType == Curve::SEG_QUADTO) {
      result[0] = getCoord(offset);
      result[1] = getCoord(offset + 1);
    }
    if(pointType == Curve::SEG_QUADTO) {
      result[2] = getCoord(offset + 2);
      result[3] = getCoord(offset + 3);
    }
  }

  Line* Line2::toLine() {
    if(hasQuads) {
      cerr << "Line2 contains quad-segment. Invalid transformation." << endl;
      Line *line = new Line(0);
      line->SetDefined(false);
      return line;
    }
    Line* l = new Line(pointTypes.Size()*2);
    l->StartBulkLoad();
    int edgeno = 0;
    double coords[2];
    double x0, x1, y0, y1, movx, movy;
    int offset = 0;
    HalfSegment * hs;
    Point *lp, *rp;
    for(int i = 0; i < pointTypes.Size(); i++) {
      switch (getPointType(i)) {
        case Curve::SEG_MOVETO:
          nextSegment(offset, Curve::SEG_MOVETO, coords);
          offset += 2;
          movx = x0 = coords[0];
          movy = y0 = coords[1];
          break;
        case Curve::SEG_LINETO:
          nextSegment(offset, Curve::SEG_LINETO, coords);
          offset += 2;
          x1 = coords[0];
          y1 = coords[1];
          lp = new Point(true, x0, y0);
          rp = new Point(true, x1, y1);
          hs = new HalfSegment(true, *lp, *rp);
          hs->attr.edgeno = edgeno++;
          *l += *hs;
          hs->SetLeftDomPoint( !hs->IsLeftDomPoint() );
          *l += *hs;
          x0 = x1;
          y0 = y1;
          break;
        case Curve::SEG_CLOSE:
          x1 = movx;
          y1 = movy;
          lp = new Point(true, x0, y0);
          rp = new Point(true, x1, y1);
          hs = new HalfSegment(true, *lp, *rp);
          hs->attr.edgeno = edgeno++;
          *l += *hs;
          hs->SetLeftDomPoint( !hs->IsLeftDomPoint() );
          *l += *hs;
          x0 = x1;
          y0 = y1;
          break;
      }
    }
    l->EndBulkLoad();
    delete hs;
    delete lp;
    delete rp;
    return l;
  }

  void Line2::hasInitialMove() {
    if (pointTypes.Size() == 0) {
      moveTo(0, 0);
    }
  }

  int
  Line2::rectCrossings(double rxmin, double rymin, double rxmax, double rymax) {
    if (pointTypes.Size() == 0) {
      return 0;
    }
    double curx, cury, movx, movy, endx, endy;
    std::vector<double> loc_coords;
    for(int i = 0; i < coords.Size(); i++) {
      loc_coords.push_back(getCoord(i));
    }
    curx = movx = loc_coords.at(0);
    cury = movy = loc_coords.at(1);
    int crossings = 0;
    int ci = 2;
    for (int i = 1;
         crossings != Curve::RECT_INTERSECTS && i < pointTypes.Size(); i++) {
      int pointTyp = getPointType(i);
      switch (pointTyp) {
        case Curve::SEG_MOVETO:
          if (curx != movx || cury != movy) {
            crossings = Curve::rectCrossingsForLine(crossings,
                                          rxmin, rymin, rxmax, rymax,
                                          curx, cury, movx, movy);
          }
          movx = curx = loc_coords.at(ci++);
          movy = cury = loc_coords.at(ci++);
          break;
        case Curve::SEG_LINETO:
          endx = loc_coords.at(ci++);
          endy = loc_coords.at(ci++);
          crossings = Curve::rectCrossingsForLine(crossings,
                                        rxmin, rymin, rxmax, rymax,
                                        curx, cury, endx, endy);
          curx = endx;
          cury = endy;
          break;
        case Curve::SEG_QUADTO:
          double tmpx, tmpy;
          tmpx = loc_coords.at(ci++);
          tmpy = loc_coords.at(ci++);
          endx = loc_coords.at(ci++);
          endy = loc_coords.at(ci++);
          crossings = Curve::rectCrossingsForQuad(crossings,
                                        rxmin, rymin, rxmax, rymax,
                                        curx, cury, tmpx, tmpy,
                                        endx, endy, 0);
          curx = endx;
          cury = endy;
          break;
        case Curve::SEG_CLOSE:
          if (curx != movx || cury != movy) {
            crossings = Curve::rectCrossingsForLine(crossings,
                                          rxmin, rymin, rxmax, rymax,
                                          curx, cury, movx, movy);
          }
          curx = movx;
          cury = movy;
          // Count should always be a multiple of 2 here.
          break;
      }
    }
    if (crossings != Curve::RECT_INTERSECTS &&
        (curx != movx || cury != movy)) {
      crossings = Curve::rectCrossingsForLine(crossings,
                                    rxmin, rymin, rxmax, rymax,
                                    curx, cury, movx, movy);
    }
    // Count should always be a multiple of 2 here.
    return crossings;
  }

}

