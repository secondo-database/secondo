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

#include "Region.h"
#include "Curve.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "AreaOp.h"


using namespace std;

namespace salr {

  Region::Region(const Region &other) :
    Attribute(other.IsDefined()),
    curves(other.curves.size()),
    coords(other.coords.Size()),
    pointTypes(other.pointTypes.Size())
  {
    if (other.IsDefined()) {
      for (int i = 0; i < other.pointTypes.Size(); i++) {
        appendType(other.getPointType(i));
      }
      for (int i = 0; i < other.coords.Size(); i++) {
        appendCoord(other.getCoord(i));
      }
      Curve::copyCurves(&other.curves, &curves);
    }
  }

  Region::Region(const Line &line) :
    Attribute(line.IsDefined()),
    curves(line.pointTypes.Size()),
    coords(line.coords.Size()),
    pointTypes(line.pointTypes.Size())
  {
    if (line.IsDefined() && line.pointTypes.Size() > 0) {
      for (int i = 0; i < line.pointTypes.Size(); i++) {
        appendType(line.getPointType(i));
      }
      for (int i = 0; i < line.coords.Size(); i++) {
        appendCoord(line.getCoord(i));
      }
      createCurves();
      updateFLOBs();
    }
  }

  Region::~Region() {
  }

  const std::string Region::BasicType() {
    return "region2";
  }

  const bool Region::checkType(const ListExpr type) {
    return listutils::isSymbol(type, BasicType());
  }

  int Region::NumOfFLOBs() const {
    return 2;
  }

  Flob *Region::GetFLOB(const int i) {
    assert(i >= 0 && i < NumOfFLOBs());
    if (i == 1) {
      return &coords;
    } else {
      return &pointTypes;
    }
  }

  int Region::Compare(const Attribute *arg) const {
    const Region &l = *((const Region *) arg);
    if (!IsDefined() && !l.IsDefined()) {
      return 0;
    }
    if (!IsDefined()) {
      return -1;
    }
    if (!l.IsDefined()) {
      return 1;
    }

    if (curves.size() > l.curves.size())
      return 1;
    if (curves.size() < l.curves.size())
      return -1;
    return 0;
  }

  bool Region::Adjacent(const Attribute *arg) const {
    return false;
  }

  size_t Region::Sizeof() const {
    return sizeof(*this);
  }

  size_t Region::HashValue() const {
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

  void Region::CopyFrom(const Attribute *arg) {
    *this = *((Region *) arg);
  }

  Attribute *Region::Clone() const {
    return new Region(*this);
  }

  bool Region::CheckKind(ListExpr type, ListExpr &errorInfo) {
    return checkType(type);
  }

  bool Region::ReadFrom(ListExpr LE, const ListExpr typeInfo) {
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
    if (numTypes == 0 || numCoords == 0) {
      cmsg.inFunError("None of the num variables can be 0");
      return false;
    }
    if (!nl->HasLength(nl->Third(LE), numTypes)) {
      cmsg.inFunError("PointTypes list length does not match numTypes element");
      return false;
    }
    if (!nl->HasLength(nl->Fourth(LE), numCoords)) {
      cmsg.inFunError("Coords list length does not match numCoords element");
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

    this->createCurves();
    this->updateFLOBs();
    SetDefined(true);
    return true;
  }

  ListExpr Region::ToListExpr(ListExpr typeInfo) const {
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

  void Region::createCurves() {
    if(pointTypes.Size() <= 0) {
      return;
    }
    vector<Curve*> c(this->curves.size());
    copy(this->curves.begin(), this->curves.end(), c.begin());
    double coords[10];
    double movx = 0, movy = 0, curx = 0, cury = 0;
    double newx, newy;
    int offset = 0;
    for (int i = 0; i < pointTypes.Size(); i++) {
      switch (getPointType(i)) {
        case Curve::SEG_MOVETO:
          nextSegment(offset, Curve::SEG_MOVETO, coords);
          offset += 2;
          Curve::insertLine(&c, curx, cury, movx, movy);
          curx = movx = coords[0];
          cury = movy = coords[1];
          Curve::insertMove(&c, movx, movy);
          break;
        case Curve::SEG_LINETO:
          nextSegment(offset, Curve::SEG_LINETO, coords);
          offset += 2;
          newx = coords[0];
          newy = coords[1];
          Curve::insertLine(&c, curx, cury, newx, newy);
          curx = newx;
          cury = newy;
          break;
        case Curve::SEG_QUADTO:
          nextSegment(offset, Curve::SEG_QUADTO, coords);
          offset += 4;
          newx = coords[2];
          newy = coords[3];
          Curve::insertQuad(&c, curx, cury, coords);
          curx = newx;
          cury = newy;
          ((QuadCurve *) c.back())->setSeedCoords(coords[0], coords[1],
                                                  coords[2], coords[3]);
          break;
        case Curve::SEG_CLOSE:
          Curve::insertLine(&c, curx, cury, movx, movy);
          curx = movx;
          cury = movy;
          break;
      }
    }
    Curve::insertLine(&c, curx, cury, movx, movy);
    AreaOp op = AreaOp();
    op.calculate(&c, NULL, &(this->curves));
  }

  void Region::updateFLOBs() {
    coords.clean();
    pointTypes.clean();
    for (unsigned int i = 0; i < curves.size(); i++) {
      pointTypes.Append(curves.at(i)->getOrder());
      Curve *curve = curves.at(i);
      if (curve->getOrder() == Curve::SEG_MOVETO) {
        coords.Append(curve->getX0());
        coords.Append(curve->getY0());
      } else if (curve->getOrder() == Curve::SEG_LINETO) {
        if (curve->getDirection() == Curve::INCREASING) {
          coords.Append(curve->getX1());
          coords.Append(curve->getY1());
        } else {
          coords.Append(curve->getX0());
          coords.Append(curve->getY0());
        }
      } else if (curve->getOrder() == Curve::SEG_QUADTO) {
        QuadCurve *quadCurve = (QuadCurve *) curve;
        coords.Append(quadCurve->getSeedX0());
        coords.Append(quadCurve->getSeedY0());
        coords.Append(quadCurve->getSeedX1());
        coords.Append(quadCurve->getSeedY1());
      }
    }
  }

  void Region::nextSegment(int offset, int pointType, double *result) const {
    if (pointType == Curve::SEG_MOVETO || pointType == Curve::SEG_LINETO) {
      result[0] = getCoord(offset);
      result[1] = getCoord(offset + 1);
    }
    if (pointType == Curve::SEG_QUADTO) {
      result[2] = getCoord(offset + 2);
      result[3] = getCoord(offset + 3);
    }
  }

}

