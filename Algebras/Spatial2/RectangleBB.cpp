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

#include "RectangleBB.h"
#include "NestedList.h"
#include "ListUtils.h"

#include <algorithm>

using namespace std;

namespace salr {

  RectangleBB::RectangleBB(double x, double y, double width, double height) :
    Attribute(true), x(x), y(y), width(width), height(height) {
  }

  RectangleBB::RectangleBB(int init) :
    Attribute(true), x(0), y(0), width(0), height(0) {
  }

  RectangleBB::~RectangleBB() {
  }

  const std::string RectangleBB::BasicType() {
    return "rectangleBB";
  }

  const bool RectangleBB::checkType(const ListExpr type) {
    return listutils::isSymbol(type, BasicType());
  }

  int RectangleBB::NumOfFLOBs() const {
    return 0;
  }

  Flob* RectangleBB::GetFLOB(const int i) {
    return NULL;
  }

  int RectangleBB::Compare(const Attribute *arg) const {
    const RectangleBB &l = *((const RectangleBB *) arg);
    if (!IsDefined() && !l.IsDefined()) {
      return 0;
    }
    if (!IsDefined()) {
      return -1;
    }
    if (!l.IsDefined()) {
      return 1;
    }

    if (x > l.x)
      return 1;
    if (x < l.x)
      return -1;
    if (y > l.y)
      return 1;
    if (y < l.y)
      return -1;
    return 0;
  }

  bool RectangleBB::Adjacent(const Attribute *arg) const {
    return false;
  }

  size_t RectangleBB::Sizeof() const {
    return sizeof(*this);
  }

  size_t RectangleBB::HashValue() const {
    size_t h = 17 * x;
    h += 13 * y;
    h += 7 + width;
    h += 5 * height;
    return h;
  }

  void RectangleBB::CopyFrom(const Attribute *arg) {
    *this = *((RectangleBB *) arg);
  }

  Attribute *RectangleBB::Clone() const {
    return new RectangleBB(this->x, this->y, this->width, this->height);
  }

  bool RectangleBB::CheckKind(ListExpr type, ListExpr &errorInfo) {
    return checkType(type);
  }

  bool RectangleBB::ReadFrom(ListExpr LE, const ListExpr typeInfo) {
    if (listutils::isSymbolUndefined(LE)) {
      SetDefined(false);
      return true;
    }

    if (!nl->HasLength(LE, 4)) {
      cmsg.inFunError("List in ReadFrom-Function wrong size");
      return false;
    }
    if (!listutils::isNumeric(nl->First(LE))
        || !listutils::isNumeric(nl->Second(LE))
        || !listutils::isNumeric(nl->Third(LE))
        || !listutils::isNumeric(nl->Fourth(LE))) {
      cmsg.inFunError("All elements must be numeric");
      return false;
    }

    if(nl->AtomType(nl->First(LE)) == IntType) {
      x = nl->IntValue(nl->First(LE));
    } else {
      x = nl->RealValue(nl->First(LE));
    }
    if(nl->AtomType(nl->Second(LE)) == IntType) {
      y = nl->IntValue(nl->Second(LE));
    } else {
      y = nl->RealValue(nl->Second(LE));
    }
    if(nl->AtomType(nl->Third(LE)) == IntType) {
      width = nl->IntValue(nl->Third(LE));
    } else {
      width = nl->RealValue(nl->Third(LE));
    }
    if(nl->AtomType(nl->Fourth(LE)) == IntType) {
      height = nl->IntValue(nl->Fourth(LE));
    } else {
      height = nl->RealValue(nl->Fourth(LE));
    }

    this->setRect(x, y, width, height);

    SetDefined(true);
    return true;
  }

  ListExpr RectangleBB::ToListExpr(ListExpr typeInfo) const {
    if (!IsDefined()) {
      return listutils::getUndefined();
    }

    ListExpr result = nl->FourElemList(nl->RealAtom(x), nl->RealAtom(y),
                                nl->RealAtom(width), nl->RealAtom(height));

    return result;
  }

  bool RectangleBB::contains(double x1, double y1) {
    return (x1 >= x && y1 >= y &&
            x1 < x + width && y1 < y + height);
  }

  bool RectangleBB::isEmpty() {
    return (width <= 0.0) || (height <= 0.0);
  }

  void RectangleBB::setRect(double x1, double y1, double w, double h) {
    x = x1;
    y = y1;
    width = w;
    height = h;
  }

  void RectangleBB::add(double newx, double newy) {
    double x1 = min(x, newx);
    double x2 = max(x + width, newx);
    double y1 = min(y, newy);
    double y2 = max(y + height, newy);
    setRect(x1, y1, x2 - x1, y2 - y1);
  }

  bool RectangleBB::intersects(double x1, double y1, double w, double h) {
    if (isEmpty() || w <= 0 || h <= 0) {
      return false;
    }
    return (x1 + w > x && y1 + h > y &&
            x1 < x + width && y1 < y + height);
  }

}
