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
#include "Crossings.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "AreaOp.h"
#include "RectangleBB.h"


using namespace std;

namespace salr {

  Region2::Region2(const Region2 &other) :
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

  Region2::Region2(const Line2 &line) :
    Attribute(line.IsDefined()),
    curves(0),
    coords(line.coords.Size()),
    pointTypes(line.pointTypes.Size())
  {
    if(line.hasQuads) {
      cerr << "Can't transform line2 with quad-segment to region2" << endl;
      SetDefined(false);
    } else {
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
  }

  Region2::~Region2() {
  }

  const string Region2::BasicType() {
    return "region2";
  }

  const bool Region2::checkType(const ListExpr type) {
    return listutils::isSymbol(type, BasicType());
  }

  int Region2::NumOfFLOBs() const {
    return 2;
  }

  Flob *Region2::GetFLOB(const int i) {
    assert(i >= 0 && i < NumOfFLOBs());
    if (i == 1) {
      return &coords;
    } else {
      return &pointTypes;
    }
  }

  int Region2::Compare(const Attribute *arg) const {
    const Region2 &l = *((const Region2 *) arg);
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

  bool Region2::Adjacent(const Attribute *arg) const {
    return false;
  }

  size_t Region2::Sizeof() const {
    return sizeof(*this);
  }

  size_t Region2::HashValue() const {
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

  void Region2::CopyFrom(const Attribute *arg) {
    *this = *((Region2 *) arg);
  }

  Attribute *Region2::Clone() const {
    return new Region2(*this);
  }

  bool Region2::CheckKind(ListExpr type, ListExpr &errorInfo) {
    return checkType(type);
  }

  bool Region2::ReadFrom(ListExpr LE, const ListExpr typeInfo) {
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
    if (numTypes == 0 || numCoords == 0) {
      cmsg.inFunError("None of the num variables can be 0");
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
         && pointType != Curve::SEG_CLOSE) {
        cmsg.inFunError("Allowed pointType values: 0, 1 or 4");
        return false;
      }
      this->appendType(pointType);
    }
    if (this->pointTypes.Size() > 0
        && this->getPointType(0) != Curve::SEG_MOVETO) {
      cmsg.inFunError("First pointType must be a moveTo");
      return false;
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

    this->createCurves();
    this->updateFLOBs();
    SetDefined(true);
    return true;
  }

  ListExpr Region2::ToListExpr(ListExpr typeInfo) const {
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

  RectangleBB* Region2::getBounds() {
    RectangleBB* r = new RectangleBB(0, 0 ,0 ,0);
    if (curves.size() > 0) {
      Curve* c = curves.at(0);
      // First point is always an order 0 curve (moveto)
      r->x = c->getX0();
      r->y = c->getY0();
      for (unsigned int i = 1; i < curves.size(); i++) {
        curves.at(i)->enlarge(r);
//        Curve* tmp = curves.at(i);
//        if(tmp->getOrder() == Curve::SEG_MOVETO) {
//          (dynamic_cast<MoveCurve *>(tmp))->enlarge(r);
//        } else if(tmp->getOrder() == Curve::SEG_LINETO) {
//          (dynamic_cast<LineCurve *>(tmp))->enlarge(r);
//        } else if(tmp->getOrder() == Curve::SEG_QUADTO) {
//          (dynamic_cast<QuadCurve *>(tmp))->enlarge(r);
//        }
      }
    }
    return r;
  }

  bool Region2::intersects(RectangleBB *bbox) {
    double x, y, w, h;
    x = bbox->x;
    y = bbox->y;
    w = bbox->width;
    h = bbox->height;
    if (w < 0 || h < 0) {
      return false;
    }
    if (!getBounds()->intersects(x, y, w, h)) {
      return false;
    }
    return Crossings::findCrossings(&curves, x, y, x+w, y+h);
  }

  Region2* Region2::union1(Region2 *rhs) {
    UnionOp op;
    op.calculate(&curves, &rhs->curves);
    this->updateFLOBs();
    return this;
  }

  Region2* Region2::minus1(Region2 *rhs) {
    MinusOp op;
    op.calculate(&curves, &rhs->curves);
    this->updateFLOBs();
    return this;
  }

  Region2* Region2::intersects1(Region2 *rhs) {
    IntersectsOp op;
    op.calculate(&curves, &rhs->curves);
    this->updateFLOBs();
    return this;
  }

  Region2* Region2::xor1(Region2 *rhs) {
    XorOp op;
    op.calculate(&curves, &rhs->curves);
    this->updateFLOBs();
    return this;
  }

  void Region2::createCurves() {
    if(pointTypes.Size() <= 0) {
      return;
    }
    vector<Curve*> c(this->curves.size());
    double coords[10];
    double movx = 0, movy = 0, curx = 0, cury = 0;
    double newx, newy;
    int offset = 0;
    for (int i = 0; i < pointTypes.Size(); i++) {
      switch (getPointType(i)) {
        case Curve::SEG_MOVETO:
          nextSegment(offset, Curve::SEG_MOVETO, coords);
          offset += 2;
          Curve::insertLine(&curves, curx, cury, movx, movy);
          curx = movx = coords[0];
          cury = movy = coords[1];
          Curve::insertMove(&curves, movx, movy);
          break;
        case Curve::SEG_LINETO:
          nextSegment(offset, Curve::SEG_LINETO, coords);
          offset += 2;
          newx = coords[0];
          newy = coords[1];
          Curve::insertLine(&curves, curx, cury, newx, newy);
          curx = newx;
          cury = newy;
          break;
        case Curve::SEG_CLOSE:
          Curve::insertLine(&curves, curx, cury, movx, movy);
          curx = movx;
          cury = movy;
          break;
      }
    }
    Curve::insertLine(&curves, curx, cury, movx, movy);
    EOWindOp op;
    op.calculate(&curves, NULL);
  }

  void Region2::updateFLOBs() {
    coords.clean();
    pointTypes.clean();
    Curve *lastcurve;
    for (unsigned int i = 0; i < curves.size(); i++) {
      Curve *curve = curves.at(i);
      if (curve->getOrder() == Curve::SEG_MOVETO) {
        if(i > 0 && lastcurve->getOrder() != Curve::SEG_MOVETO) {
          pointTypes.Append((int) Curve::SEG_CLOSE);
        }
        coords.Append(curve->getX0());
        coords.Append(curve->getY0());
      } else if (curve->getOrder() == Curve::SEG_LINETO) {
        if(getPointType(pointTypes.Size() - 1) == Curve::SEG_LINETO
          && (getCoord(coords.Size()-2) != curve->getX0()
          || getCoord(coords.Size()-1) != curve->getY0())) {
          // horizontal lines are added again
          pointTypes.Append((int) Curve::SEG_LINETO);
          coords.Append(curve->getX0());
          coords.Append(curve->getY0());
        }
        coords.Append(curve->getX1());
        coords.Append(curve->getY1());
      }
      pointTypes.Append(curves.at(i)->getOrder());
      lastcurve = curve;
    }
    if(lastcurve->getOrder() != Curve::SEG_CLOSE) {
      pointTypes.Append((int) Curve::SEG_CLOSE);
    }
  }

  void Region2::nextSegment(int offset, int pointType, double *result) const {
    if (pointType == Curve::SEG_MOVETO
        || pointType == Curve::SEG_LINETO){
      result[0] = getCoord(offset);
      result[1] = getCoord(offset + 1);
    }
  }

}

