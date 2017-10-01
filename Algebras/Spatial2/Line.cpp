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
#include "Curve.h"
#include "NestedList.h"
#include "ListUtils.h"

namespace salr {

  Line::Line(const Line &other) :
    Attribute(other.IsDefined()),
    coords(other.coords.Size()),
    pointTypes(other.pointTypes.Size()) {
    for (int i = 0; i < other.pointTypes.Size(); i++) {
      appendType(other.getPointType(i));
    }
    for (int i = 0; i < other.coords.Size(); i++) {
      appendCoord(other.getCoord(i));
    }
  }

  Line::Line(int initialCapacity) :
    Attribute(true),
    coords(initialCapacity * 2),
    pointTypes(initialCapacity) {
  }

  Line::~Line() {
  }

  const std::string Line::BasicType() {
    return "line2";
  }

  const bool Line::checkType(const ListExpr type) {
    return listutils::isSymbol(type, BasicType());
  }

  int Line::NumOfFLOBs() const {
    return 2;
  }

  Flob *Line::GetFLOB(const int i) {
    assert(i >= 0 && i < NumOfFLOBs());
    if (i == 1) {
      return &coords;
    } else {
      return &pointTypes;
    }
  }

  int Line::Compare(const Attribute *arg) const {
    const Line &l = *((const Line *) arg);
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

  bool Line::Adjacent(const Attribute *arg) const {
    return false;
  }

  size_t Line::Sizeof() const {
    return sizeof(*this);
  }

  size_t Line::HashValue() const {
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

  void Line::CopyFrom(const Attribute *arg) {
    *this = *((Line *) arg);
  }

  Attribute *Line::Clone() const {
    return new Line(*this);
  }

  bool Line::CheckKind(ListExpr type, ListExpr &errorInfo) {
    return checkType(type);
  }

  bool Line::ReadFrom(ListExpr LE, const ListExpr typeInfo) {
    if (listutils::isSymbolUndefined(LE)) {
      SetDefined(false);
      return true;
    }

    if (!nl->HasLength(LE, 4)) {
      cmsg.inFunError("List in ReadFrom-Function wrong size");
      return false;
    }
    if (!listutils::isNumeric(nl->First(LE))
        || !listutils::isNumeric(nl->Second(LE))) {
      cmsg.inFunError("First two elements must be numeric");
      return false;
    }

    int numTypes = nl->IntValue(nl->First(LE));
    int numCoords = nl->IntValue(nl->Second(LE));
    if ((numTypes == 0 && numCoords != 0) ||
        (numTypes != 0 && numCoords == 0)) {
      cmsg.inFunError("Either both num variables must be 0 or none");
      return false;
    }
    if (numTypes == 0) {
      this->hasInitialMove();
    }
    if (!nl->HasLength(nl->Third(LE), numTypes)) {
      cmsg.inFunError("PointTypes list length does not match numTypes element");
      return false;
    }
    if (!nl->HasLength(nl->Fourth(LE), numCoords)) {
      cmsg.inFunError("Coords list length does not match numCoords element");
      return false;
    }

    if (this->pointTypes.Size() > 0
        && this->getPointType(0) != Curve::SEG_MOVETO) {
      cmsg.inFunError("First pointType must be a moveTo");
      return false;
    }
    ListExpr pointTypesList = nl->Third(LE);
    for (int i = 1; i <= numTypes; i++) {
      ListExpr current = nl->Nth(i, pointTypesList);
      if (!(nl->IsAtom(current) && nl->AtomType(current) == IntType)) {
        cmsg.inFunError("All pointType values must be of IntType");
        return false;
      }
      this->appendType(nl->IntValue(current));
    }
    ListExpr coordsList = nl->Fourth(LE);
    for (int i = 1; i <= numCoords; i++) {
      ListExpr current = nl->Nth(i, coordsList);
      if (!(nl->IsAtom(current) && nl->AtomType(current) == RealType)) {
        cmsg.inFunError("All coord values must be of RealType");
        return false;
      }
      this->appendCoord(nl->RealValue(current));
    }
    SetDefined(true);
    return true;
  }

  ListExpr Line::ToListExpr(ListExpr typeInfo) const {
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

    ListExpr result = nl->FourElemList(nl->IntAtom(pointTypes.Size()),
                                       nl->IntAtom(coords.Size()),
                                       typesList, coordsList);
    return result;
  }

  void Line::moveTo(double x, double y) {
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

  void Line::lineTo(double x, double y) {
    hasInitialMove();
    pointTypes.Append((int) Curve::SEG_LINETO);
    coords.Append(x);
    coords.Append(y);
  }

  void Line::quadTo(double x1, double y1, double x2, double y2) {
    hasInitialMove();
    pointTypes.Append((int) Curve::SEG_QUADTO);
    coords.Append(x1);
    coords.Append(y1);
    coords.Append(x2);
    coords.Append(y2);
  }

  void Line::closeLine() {
    if (pointTypes.Size() == 0 ||
        getPointType(pointTypes.Size() - 1) != Curve::SEG_CLOSE) {
      hasInitialMove();
      pointTypes.Append((int) Curve::SEG_CLOSE);
    }
  }

  void Line::nextSegment(int offset, int pointType, double *result) const {
    if(pointType == Curve::SEG_MOVETO || pointType == Curve::SEG_LINETO) {
      result[0] = getCoord(offset);
      result[1] = getCoord(offset + 1);
    }
    if(pointType == Curve::SEG_QUADTO) {
      result[2] = getCoord(offset + 2);
      result[3] = getCoord(offset + 3);
    }
  }

  void Line::hasInitialMove() {
    if (pointTypes.Size() == 0) {
      moveTo(0, 0);
    }
  }

  int Line::pointCrossings(double px, double py) {
    if (pointTypes.Size() == 0) {
      return 0;
    }
    double movx, movy, curx, cury, endx, endy;
    DbArray<double> loc_coords = coords;
    loc_coords.Get(0, &movx);
    loc_coords.Get(1, &movy);
    curx = movx;
    cury = movy;
    int crossings = 0;
    int ci = 2;
    for (int i = 1; i < pointTypes.Size(); i++) {
      int pointTyp;
      pointTypes.Get(i, &pointTyp);
      switch (pointTyp) {
        case Curve::SEG_MOVETO:
          if (cury != movy) {
            crossings +=
              Curve::pointCrossingsForLine(px, py,
                                           curx, cury,
                                           movx, movy);
          }
          loc_coords.Get(ci++, &curx);
          loc_coords.Get(ci++, &cury);
          movx = curx;
          movy = cury;
          break;
        case Curve::SEG_LINETO:
          loc_coords.Get(ci++, &endx);
          loc_coords.Get(ci++, &endy);
          crossings +=
            Curve::pointCrossingsForLine(px, py,
                                         curx, cury,
                                         endx, endy);
          curx = endx;
          cury = endy;
          break;
        case Curve::SEG_QUADTO:
          double tmpx, tmpy;
          loc_coords.Get(ci++, &tmpx);
          loc_coords.Get(ci++, &tmpy);
          loc_coords.Get(ci++, &endx);
          loc_coords.Get(ci++, &endy);
          crossings +=
            Curve::pointCrossingsForQuad(px, py,
                                         curx, cury,
                                         tmpx, tmpy,
                                         endx, endy, 0);
          curx = endx;
          cury = endy;
          break;
        case Curve::SEG_CLOSE:
          if (cury != movy) {
            crossings +=
              Curve::pointCrossingsForLine(px, py,
                                           curx, cury,
                                           movx, movy);
          }
          curx = movx;
          cury = movy;
          break;
      }
    }
    if (cury != movy) {
      crossings +=
        Curve::pointCrossingsForLine(px, py,
                                     curx, cury,
                                     movx, movy);
    }
    return crossings;
  }

  int
  Line::rectCrossings(double rxmin, double rymin, double rxmax, double rymax) {
    if (pointTypes.Size() == 0) {
      return 0;
    }
    DbArray<double> loc_coords = coords;
    double curx, cury, movx, movy, endx, endy;
    loc_coords.Get(0, &movx);
    loc_coords.Get(1, &movy);
    curx = movx;
    cury = movy;
    int crossings = 0;
    int ci = 2;
    for (int i = 1;
         crossings != Curve::RECT_INTERSECTS && i < pointTypes.Size(); i++) {
      int pointTyp;
      pointTypes.Get(i, &pointTyp);
      switch (pointTyp) {
        case Curve::SEG_MOVETO:
          if (curx != movx || cury != movy) {
            crossings =
              Curve::rectCrossingsForLine(crossings,
                                          rxmin, rymin,
                                          rxmax, rymax,
                                          curx, cury,
                                          movx, movy);
          }
          loc_coords.Get(ci++, &curx);
          loc_coords.Get(ci++, &cury);
          movx = curx;
          movy = cury;
          break;
        case Curve::SEG_LINETO:
          loc_coords.Get(ci++, &endx);
          loc_coords.Get(ci++, &endy);
          crossings =
            Curve::rectCrossingsForLine(crossings,
                                        rxmin, rymin,
                                        rxmax, rymax,
                                        curx, cury,
                                        endx, endy);
          curx = endx;
          cury = endy;
          break;
        case Curve::SEG_QUADTO:
          double tmpx, tmpy;
          loc_coords.Get(ci++, &tmpx);
          loc_coords.Get(ci++, &tmpy);
          loc_coords.Get(ci++, &endx);
          loc_coords.Get(ci++, &endy);
          crossings =
            Curve::rectCrossingsForQuad(crossings,
                                        rxmin, rymin,
                                        rxmax, rymax,
                                        curx, cury,
                                        tmpx, tmpy,
                                        endx, endy, 0);
          curx = endx;
          cury = endy;
          break;
        case Curve::SEG_CLOSE:
          if (curx != movx || cury != movy) {
            crossings =
              Curve::rectCrossingsForLine(crossings,
                                          rxmin, rymin,
                                          rxmax, rymax,
                                          curx, cury,
                                          movx, movy);
          }
          curx = movx;
          cury = movy;
          // Count should always be a multiple of 2 here.
          break;
      }
    }
    if (crossings != Curve::RECT_INTERSECTS &&
        (curx != movx || cury != movy)) {
      crossings =
        Curve::rectCrossingsForLine(crossings,
                                    rxmin, rymin,
                                    rxmax, rymax,
                                    curx, cury,
                                    movx, movy);
    }
    // Count should always be a multiple of 2 here.
    return crossings;
  }

}

