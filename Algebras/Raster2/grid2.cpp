/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

*/ 

#include <cmath>
#include <utility>
#include <vector>

#include <Algebra.h>
#include <Point.h>
#include <TypeConstructor.h>

#include "RasterStorage.h"
#include "grid2.h"
#include "util/util.h"

using namespace raster2;

const grid2::index_type grid2::None = (int[]){0,0};

grid2::grid2() : x(0), y(0), length(1)
{
  
}

grid2::grid2(double ax, double ay, double alength)
        : x(ax), y(ay), length(alength)
{
    assert(length > 0);
}

grid2::grid2(const grid2& rgrid2)
{
  x = rgrid2.x;
  y = rgrid2.y;
  length = rgrid2.length;
}

grid2::~grid2()
{
  
}

double grid2::getOriginX() const { return x; }
double grid2::getOriginY() const { return y; }
double grid2::getLength() const { return length; }

grid2::index_type grid2::getIndex(double xcoord, double ycoord) const {
    return index_type((int[]){std::floor((xcoord - x)/length),
                              std::floor((ycoord - y)/length)});
}

grid2::region_type grid2::getRegion(const Rectangle<2>& bbox) const {
    return region_type(
        index_type((int[]){std::floor((bbox.MinD(0) - x)/length),
                           std::floor((bbox.MinD(1) - y)/length)}),
        index_type((int[]){std::floor((bbox.MaxD(0) - x)/length),
                           std::floor((bbox.MaxD(1) - y)/length)})
    );
}

Rectangle<2> grid2::getCell(const index_type& i) const {
  return Rectangle<2>(true,
      i[0] * length + x,            (1 + i[0]) * length + x,
      i[1] * length + y,            (1 + i[1]) * length + y);
}

Rectangle<2> grid2::getBBox(const region_type& r) const {
  return Rectangle<2>(true,
      r.Min[0] * length + x,            (1 + r.Max[0]) * length + x,
      r.Min[1] * length + y,            (1 + r.Max[1]) * length + y);
}

Rectangle<2> grid2::getBBox(const index_type& from, const index_type& to) const
{
  return Rectangle<2>(true,
      from[0] * length + x,            (1 + to[0]) * length + x,
      from[1] * length + y,            (1 + to[1]) * length + y);
}
/*
The function ~intersect~ takes a line segment and a raster cell (given by its
index) and determines the offsets of the raster cells that the line segment
comes from and goes to. For the purpose of this function, the segment comes from
the dominating point and goes to the secondary point.

*/
std::pair<grid2::index_type, grid2::index_type > grid2::intersect
    (const HalfSegment& hs, const index_type& cell) const
{
  static const index_type Top = (int[]){0,1};
  static const index_type Bottom = (int[]){0,-1};
  static const index_type Left = (int[]){-1,0};
  static const index_type Right = (int[]){1,0};
  static const index_type TopRight = Top + Right;
  static const index_type BottomLeft = Bottom + Left;
  static const index_type Edges[4] = {Left, Top, Right, Bottom};

  std::pair<index_type, index_type > result(None, None);

  Point from = hs.GetDomPoint();
  Point to = hs.GetSecPoint();

  double minx = from.GetX() < to.GetX() ? from.GetX() : to.GetX();
  double maxx = from.GetX() > to.GetX() ? from.GetX() : to.GetX();
  double miny = from.GetY() < to.GetY() ? from.GetY() : to.GetY();
  double maxy = from.GetY() > to.GetY() ? from.GetY() : to.GetY();

  std::set<index_type > intersections; // The found edges

  // The coordinates of the cell boundaries.
  double xp[] = {cell[0] * length + x, (1+cell[0]) * length + x};
  double yp[] = {cell[1] * length + y, (1+cell[1]) * length + y};

  if (miny == maxy) {
    // The line is horizontal
    if (yp[0] <= miny && miny < yp[1]) {
      if (minx < xp[0] && xp[0] <= maxx) {
        intersections.insert(Left);
      }
      if (minx < xp[1] && xp[1] <= maxx) {
        intersections.insert(Right);
      }
    }
  }
  else if (minx == maxx) {
    // The line is vertical
    if (xp[0] <= minx && minx < xp[1]) {
      if (miny < yp[0] && yp[0] <= maxy) {
        intersections.insert(Bottom);
      }
      if (miny < yp[1] && yp[1] <= maxy) {
        intersections.insert(Top);
      }
    }
  } else {
    // Lines are transformed into the representation
    //   a_i * x + b_i y = c_i.
    //
    // Then, the intersection (x,y) of two lines is given by
    //   x = (b_2 * c_1 - b_1 * c_2) / det
    //   y = (a_1 * c_2 - a_2 * c_1) / det
    // where
    //   det = a_1 * b_2 - a_2 * b_1.
    //
    double a = to.GetY() - from.GetY();
    double b = from.GetX() - to.GetX();
    double c = a * from.GetX() + b * from.GetY();

    double cb_x[][4] = {{ xp[0], xp[0], xp[1], xp[1] },
                        { xp[0], xp[1], xp[1], xp[0] }};
    double cb_y[][4] = {{ yp[0], yp[1], yp[1], yp[0] },
                        { yp[1], yp[1], yp[0], yp[0] }};

    double cb_a[4] = { length, 0, length, 0 };
    double cb_b[4] = { 0, -length, 0, -length };
    double cb_c[4] = {
        cb_a[0] * xp[0] + cb_b[0] * yp[0], // Left
        cb_a[1] * xp[0] + cb_b[1] * yp[1], // Top
        cb_a[2] * xp[1] + cb_b[2] * yp[0], // Right
        cb_a[3] * xp[0] + cb_b[3] * yp[0]  // Bottom
    };

    double m = -a/b;

    bool finished = false; // Exit early

    // Test all four cell edges and remember any intersections.
    for (int i = 0; i < 4; ++i) {
      double det = a * cb_b[i] - cb_a[i] * b;
      double ix = (cb_b[i] * c - b * cb_c[i])/det;
      double iy = (a * cb_c[i] - cb_a[i] * c)/det;

      if ((minx <= cb_x[0][i] && cb_x[0][i] <= maxx) ||
          (miny <= cb_y[0][i] && cb_y[0][i] <= maxy))
      {
        bool crossesEdgeStart =
            util::equals(cb_x[0][i], ix) && util::equals(cb_y[0][i], iy);
        bool crossesEdgeEnd =
            util::equals(cb_x[1][i], ix) && util::equals(cb_y[1][i], iy);
        bool crossesEdgeHorizontally =
            (minx <= cb_x[0][i] && cb_x[0][i] <= maxx) &&
            ((cb_y[0][i] <= iy && iy <= cb_y[1][i]) ||
             (cb_y[1][i] <= iy && iy <= cb_y[0][i]));
        bool crossesEdgeVertically =
            (miny <= cb_y[0][i] && cb_y[0][i] <= maxy) &&
            ((cb_x[0][i] <= ix && ix <= cb_x[1][i]) ||
             (cb_x[1][i] <= ix && ix <= cb_x[0][i]));

        switch (i) {
          case 0:
            if (crossesEdgeStart) {
              if (m > 0 && minx < cb_x[0][i]) {
                intersections.insert(BottomLeft);
              } else if (m < 0) {
                if (minx < cb_x[0][i]) {
                  intersections.insert(Left);
                }
                if (maxx > cb_x[0][i]) {
                  intersections.insert(Bottom);
                }
                // finished = true;
              }
            } else if (crossesEdgeEnd) {
              if (m < 0 && minx <= cb_x[1][i] && cb_x[1][i] < maxx) {
                intersections.insert(Top);
              } else {
                // finished = true;
              }
            } else if (crossesEdgeHorizontally && minx < cb_x[0][i])
            {
              intersections.insert(Edges[i]);
            }
            break;
          case 1:
            if (crossesEdgeStart) {
              if (m < 0 && minx <= cb_x[0][i] && cb_x[0][i] < maxx) {
                intersections.insert(Top);
              } else {
                // finished = true;
              }
            } else if (crossesEdgeEnd) {
              if (m > 0 && minx < cb_x[1][i] && cb_x[1][i] <= maxx) {
                intersections.insert(TopRight);
              } else {
                // finished = true;
              }
            } else if (crossesEdgeVertically && miny < cb_y[0][i])
            {
              intersections.insert(Edges[i]);
            }
            break;
          case 2:
            if (crossesEdgeStart) {
              if (m > 0 && minx < cb_x[0][i] && cb_x[0][i] <= maxx) {
                intersections.insert(TopRight);
              } else {
                // finished = true;
              }
            } else if (crossesEdgeEnd) {
              if (m < 0 && minx < cb_x[1][i] && cb_x[1][i] <= maxx) {
                intersections.insert(Right);
              } else {
                // finished = true;
              }
            } else if (crossesEdgeHorizontally && minx < cb_x[0][i])
            {
              intersections.insert(Edges[i]);
            }
            break;
          case 3:
            if (crossesEdgeStart) {
              if (m < 0 && minx < cb_x[0][i] && cb_x[0][i] <= maxx) {
                intersections.insert(Right);
              } else {
                // finished = true;
              }
            } else if (crossesEdgeEnd) {
              if (m > 0 && minx < cb_x[1][i]) {
                intersections.insert(BottomLeft);
              } else if (m < 0) {
                if (minx < cb_x[1][i]) {
                  intersections.insert(Left);
                }
                if (maxx > cb_x[1][i]) {
                  intersections.insert(Bottom);
                }
                // finished = true;
              }
            } else if (crossesEdgeVertically && miny < cb_y[0][i])
            {
              intersections.insert(Edges[i]);
            }
            break;
        }

        if (finished || intersections.size() == 2) {
          // Break early if we found two edges or we know from other
          // circumstances, that no further edges are of interest.
          break;
        }
      }
    }
  }

  // Now that we have found the edges that the segment intersects, we need to
  // figure out which one is the one that the segment comes from and which one
  // is the one that the segment goes to.
  if (!intersections.empty()) {
    index_type* other=0;
    if (intersections.find(Top) != intersections.end()) {
      if (from.GetY() > to.GetY()) {
        result.first = Top;
        other = &result.second;
      } else {
        result.second = Top;
        other = &result.first;
      }
      intersections.erase(Top);
    } else if (intersections.find(TopRight) != intersections.end()) {
      if (from.GetY() > to.GetY()) {
        result.first = TopRight;
        other = &result.second;
      } else {
        result.second = TopRight;
        other = &result.first;
      }
      intersections.erase(TopRight);
    } else if (intersections.find(Left) != intersections.end()) {
      if (from.GetX() < to.GetX()) {
        result.first = Left;
        other = &result.second;
      } else {
        result.second = Left;
        other = &result.first;
      }
      intersections.erase(Left);
    } else if (intersections.find(Right) != intersections.end()) {
      if (from.GetX() > to.GetX()) {
        result.first = Right;
        other = &result.second;
      } else {
        result.second = Right;
        other = &result.first;
      }
      intersections.erase(Right);
    } else if (intersections.find(BottomLeft) != intersections.end()) {
      if (from.GetX() < to.GetX()) {
        result.first = BottomLeft;
        other = &result.second;
      } else {
        result.second = BottomLeft;
        other = &result.first;
      }
      intersections.erase(BottomLeft);
    } else if (intersections.find(Bottom) != intersections.end()) {
      if (from.GetY() < to.GetY()) {
        result.first = Bottom;
        other = &result.second;
      } else {
        result.second = Bottom;
        other = &result.first;
      }
      intersections.erase(Bottom);
    }

    if (!intersections.empty()) {
      *other = *intersections.begin();
    }
  }

  return result;
}

void grid2::Reset()
{
  x = 0.0;
  y = 0.0;
  length = 1.0;
}

TypeConstructor grid2::getTypeConstructor() {
  TypeConstructor tc_grid2(
    grid2::BasicType(),
    grid2::Property,
    grid2::Out,
    grid2::In,
    0,
    0,
    grid2::Create,
    grid2::Delete,
    0,
    0,
    grid2::Close,
    grid2::Clone,
    grid2::Cast,
    grid2::SizeOfObj,
    grid2::KindCheck);

    tc_grid2.AssociateKind(Kind::SIMPLE());
  return tc_grid2;
}

std::string grid2::BasicType() {
    return "grid2";
}

/*
List expression of ~grid2~ is (x y l) where ~x~ is the first coordinate
of the origin, ~y~ is the second coordinate of the origin and ~l~ is the size
of a cell.

~l~ must be >= 0.

*/
Word grid2::In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo,
               bool& correct )
{
    NList nlist(instance);

    grid2* result = new grid2();

    correct = false;
    if (nlist.length() == 3) {
        if (nlist.isReal(1) && nlist.isReal(2) && nlist.isReal(3)) {
            result->x = nlist.elem(1).realval();
            result->y = nlist.elem(2).realval();
            result->length = nlist.elem(3).realval();
            correct = true;
        }
        else {
            cmsg.inFunError("Type mismatch.");
        }
    }

    if (result->length <= 0) {
        correct = false;
        cmsg.inFunError("Length must be larger than zero.");
    }

    if (!correct) {
        delete result;
        result = 0;
    }

    Word w = SetWord(result);
    w.addr = result;
    return w;
}

ListExpr grid2::Out(ListExpr typeInfo, Word value)
{
    NList result;
    grid2* g = static_cast<grid2*>(value.addr);

    result.append(NList(g->x));
    result.append(NList(g->y));
    result.append(NList(g->length));

    return result.listExpr();
}

Word grid2::Create(const ListExpr typeInfo) {
    return (SetWord(new grid2));
}

void grid2::Delete(const ListExpr typeInfo, Word& w) {
    delete static_cast<grid2*>(w.addr);
    w.addr = 0;
}

void grid2::Close(const ListExpr typeInfo, Word& w) {
    w.addr = 0;
}

Word grid2::Clone(const ListExpr typeInfo, const Word& w) {
    grid2* g = static_cast<grid2*>(w.addr);
    return SetWord(new grid2(*g));
}

bool grid2::KindCheck(ListExpr type, ListExpr& errorInfo) {
    return NList(type).isSymbol(grid2::BasicType());
}

void* grid2::Cast(void* placement) {
    return new(placement) grid2;
}

int grid2::SizeOfObj() {
    return sizeof(grid2);
}

ListExpr grid2::Property() {
    NList property;

    NList names;
    names.append(NList(std::string("Signature"), true));
    names.append(NList(std::string("Example Type List"), true));
    names.append(NList(std::string("ListRep"), true));
    names.append(NList(std::string("Example List"), true));
    names.append(NList(std::string("Remarks"), true));

    NList values;
    values.append(NList(std::string("-> DATA"), true));
    values.append(NList(BasicType(), true));
    values.append(NList(std::string("(<x> <y> <length>)"), true));
    values.append(NList(std::string("(3.1415 2.718 12.0)"), true));
    values.append(NList(std::string("length must be positive"), true));

    property = NList(names, values);

    return property.listExpr();
}
