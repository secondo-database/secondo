/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

Implementation of methods for class ~CellGrid2D~

*/

#include "CellGrid.h"


CellGrid2D::CellGrid2D() {}

CellGrid2D::CellGrid2D(const int dummy):
   Attribute(false),
   x0(0),y0(0),wx(0),wy(0),no_cells_x(0) {}

CellGrid2D::CellGrid2D(const double &x0_, const double &y0_,
               const double &wx_, const double &wy_, const int32_t &nox_)
      : x0(x0_), y0(y0_), wx(wx_), wy(wy_), no_cells_x(nox_) {
      // Defines a structure for a three-side bounded grid, open to one
      // Y-direction. The grid is specified by an origin (x0_,x0_), a cell-width
      // wx_, a cell-height wy_ and a number of cells in X-direction nox_.
      // If wy_ is positive, the grid is open to +X, if it is negative, to -X.
      // If wx_ is positive, the grid extends to the "left", otherwise to the
      // "right" side of the origin.
      // The cells are numbered sequentially starting with 1 for the cell
      // nearest to the origin.
      // Cell Bounderies. Point located on the bounderies between cells are
      // contained by the cell with the lower cell number. Thus, the origin
      // itself is never located on the grid!
        SetDefined( (no_cells_x > 0)
                     && !AlmostEqual(wx,0.0)
                     && !AlmostEqual(wy,0.0));
    }

CellGrid2D::CellGrid2D(
  const double &wx_, const double &wy_, const int32_t &nox_)
  : x0(0.0), y0(0.0), wx(wx_), wy(wy_), no_cells_x(nox_) {
  //Defines the grid with the constant origin
  SetDefined( (no_cells_x > 0)
               && !AlmostEqual(wx,0.0)
               && !AlmostEqual(wy,0.0));
}

CellGrid2D::CellGrid2D(const CellGrid2D& other):
   Attribute(other.IsDefined()),
   x0(other.x0), y0(other.y0), wx(other.wx),wy(other.wy),
   no_cells_x(other.no_cells_x) {}

CellGrid2D& CellGrid2D::operator=(const CellGrid2D& other){
  Attribute::operator=(other);
  x0 = other.x0;
  y0 = other.y0;
  wx = other.wx;
  wy = other.wy;
  no_cells_x = other.no_cells_x;
  return *this;
}


CellGrid2D::~CellGrid2D(){}

bool CellGrid2D::set(const double x0, const double y0,
                     const double wx, const double wy,
                     const int no_cells_x){

   this->x0 = x0;
   this->y0 = y0;
   this->wx = wx;
   this->wy = wy;
   this->no_cells_x = no_cells_x;
   SetDefined( (no_cells_x > 0)
              && !AlmostEqual(wx,0.0)
              && !AlmostEqual(wy,0.0));
   return IsDefined();
}

bool CellGrid2D::set(const double wx, const double wy,
                     const int no_cells_x){

   this->x0 = 0.0;
   this->y0 = 0.0;
   this->wx = wx;
   this->wy = wy;
   this->no_cells_x = no_cells_x;
   SetDefined( (no_cells_x > 0)
              && !AlmostEqual(wx,0.0)
              && !AlmostEqual(wy,0.0));
   return IsDefined();
}

double CellGrid2D::getX0() const{
  return x0;
}

double CellGrid2D::getY0() const{
  return y0;
}
double CellGrid2D::getXw() const{
  return wx;
}
double CellGrid2D::getYw() const{
  return wy;
}
int CellGrid2D::getNx() const{
  return no_cells_x;
}


double CellGrid2D::getMaxX() const {
      // returns the maximum X-coordinate lying on the grid
      return wx < 0.0 ? x0 : x0 + wx * no_cells_x;
    }

double CellGrid2D::getMaxY() const {
      // returns the maximum Y-coordinate lying on the grid
      return wy < 0.0 ? y0 : std::numeric_limits<double>::max();
    }

double CellGrid2D::getMinX() const {
      // returns the minimum X-coordinate lying on the grid
      return wx > 0.0 ? x0 : x0 + wx * no_cells_x;
    }

double CellGrid2D::getMinY() const {
      // returns the minimum Y-coordinate lying on the grid
      return wy > 0.0 ? y0 : std::numeric_limits<double>::min();
    }


bool CellGrid2D::onGrid(const double &x, const double &y) const {
      // returns true iff (x,y) is located on the grid
      if(!IsDefined()) {
        return false;
      }
      int32_t xIndex = static_cast<int32_t>(floor((x - x0) / wx));
      int32_t yIndex = static_cast<int32_t>(floor((y - y0) / wy));
      return ( (xIndex>=0) && (xIndex<=no_cells_x) && (yIndex>=0) );
    }

int32_t CellGrid2D::getCellNo(const double &x, const double &y) const {
      // returns the cell number for a given point (x,y)
      // Only positive cell numbers are valid. Negative result indicates
      // that (x,y) is not located on the grid.
      if(!IsDefined()) {
        return getInvalidCellNo();
      }
      int32_t xIndex = static_cast<int32_t>(floor((x - x0) / wx));
      int32_t yIndex = static_cast<int32_t>(floor((y - y0) / wy));
      if( (xIndex>=0) && (xIndex<=no_cells_x) && (yIndex>=0) ) {
        return xIndex + yIndex * no_cells_x + 1;
      } else {
        return getInvalidCellNo();
      }
    }

int32_t CellGrid2D::getCellNo(const Point &p) const {
      if(IsDefined() && p.IsDefined()){
        return getCellNo(p.GetX(),p.GetY());
      } else {
        return getInvalidCellNo();
      }
    }

int32_t CellGrid2D::getXIndex(const double &x) const {
      if(IsDefined()) {
        return static_cast<int32_t>(floor((x - x0) / wx));
      } else {
        return getInvalidCellNo();
      }
    }

int32_t CellGrid2D::getYIndex(const double &y) const {
      if(IsDefined()) {
        return static_cast<int32_t>(floor((y - y0) / wy));
      } else {
        return getInvalidCellNo();
      }
    }

Rectangle<2> CellGrid2D::getMBR() const {
      // returns the grid's MBR as a 2D-rectangle
      if(IsDefined()){
        double min[2], max[2];
        min[0] = getMinX(); min[1] = getMinY();
        max[0] = getMaxX(); max[1] = getMaxY();
        return Rectangle<2>( true, min, max );
      } else {
        return Rectangle<2>( false );
      }
    }

Rectangle<2> CellGrid2D::getRowMBR(const int32_t &n) const {
      // returns the grid's nth row as a 2D-rectangle
      // row numbering starts with 0
      if( IsDefined() ){
        double min_val[2], max_val[2];
        double y1 = y0 + n     * wy;
        double y2 = y0 + (n+1) * wy;
        min_val[0] = getMinX(); min_val[1] = std::min(y1,y2);
        max_val[0] = getMaxX(); max_val[1] = std::max(y1,y2);
        return Rectangle<2>( true, min_val, max_val );
      } else {
        return Rectangle<2>( false );
      }
    }

Rectangle<2> CellGrid2D::getColMBR(const int32_t &n) const {
      // returns the grid's nth column as a 2D-rectangle
      // column numbering starts with 0 and ends with no_cells_x - 1
      if( IsDefined() ){
        double min_val[2], max_val[2];
        double x1 = x0 + n * wx;
        double x2 = x0 + (n+1) * wx;
        min_val[0] = std::min(x1,x2); min_val[1] = getMinY();
        max_val[0] = std::max(x1,x2); max_val[1] = getMaxY();
        return Rectangle<2>( true, min_val, max_val );
      } else {
        return Rectangle<2>( false );
      }
    }

bool CellGrid2D::isValidCellNo(const int32_t &n) const {
      // returns true iff n is a valid grid cell number
      return IsDefined() && (n>0);
    }

int32_t CellGrid2D::getInvalidCellNo() const {
      // returns an invalid cell number
      return -666;
    }

std::ostream& CellGrid2D::Print( std::ostream &os ) const {
  if( !IsDefined() )
  {
    return os << "(CellGrid2D: undefined)";
  }
  os << "(CellGrid2D: defined, "
     << " origin: (" << x0 << "," << y0 << "), "
     << " cellwidths: " << wx << " x " << wy << " (X x Y), "
     << " cells along X-axis: " << no_cells_x
     << ")" << endl;
  return os;
}


size_t CellGrid2D::Sizeof() const{
   return sizeof(*this);
}

int CellGrid2D::Compare(const Attribute* other) const{
  const CellGrid2D* g = static_cast<const CellGrid2D*> (other);
  if(!IsDefined()){
     return g->IsDefined()?-1:0;
  }
  if(!g->IsDefined()){
    return 1;
  }
  // both are defined
  if(!AlmostEqual(x0,g->x0)){
    return x0<g->x0?-1:1;
  }
  if(!AlmostEqual(y0,g->y0)){
    return y0<g->y0?-1:1;
  }
  if(no_cells_x!=g->no_cells_x){
    return no_cells_x<g->no_cells_x?-1:1;
  }
  if(!AlmostEqual(wx,g->wx)){
    return wx<g->wx?-1:1;
  }

  if(!AlmostEqual(wy,g->wy)){
    return wy<g->wy?-1:1;
  }
  return 0;
}


bool CellGrid2D::Adjacent(const Attribute* other) const{
  return false;
}

Attribute* CellGrid2D::Clone() const{
   return new CellGrid2D(*this);
}

size_t CellGrid2D::HashValue() const{
   return static_cast<size_t>(x0+y0+no_cells_x*wx);
}

void CellGrid2D::CopyFrom(const Attribute* other) {
    operator=(*(static_cast<const CellGrid2D*>(other)));
}


const std::string CellGrid2D::BasicType(){
   return "cellgrid2d";
}

ListExpr CellGrid2D::Property(){
  return gentc::GenProperty("-> DATA",
                            BasicType(),
                           "(x0 y0 xw yw n_x)",
                           "(14.0 15.0 2.0 2.0 37)");
}

bool CellGrid2D::CheckKind(ListExpr type, ListExpr& errorInfo){
 return nl->IsEqual(type,BasicType());
}

ListExpr CellGrid2D::ToListExpr(const ListExpr typeInfo)const{
   if(!IsDefined()){
     return nl->SymbolAtom(Symbol::UNDEFINED());
   } else {
     return nl->FiveElemList( nl->RealAtom(x0),
                              nl->RealAtom(y0),
                              nl->RealAtom(wx),
                              nl->RealAtom(wy),
                              nl->IntAtom(no_cells_x));
   }
}

bool CellGrid2D::ReadFrom(const ListExpr value,const ListExpr typeInfo){
   if(listutils::isSymbolUndefined(value)){
      SetDefined(false);
      return  true;
   }
   if(!nl->HasLength(value,5)){
      return false;
   }
   ListExpr l1 = nl->First(value);
   ListExpr l2 = nl->Second(value);
   ListExpr l3 = nl->Third(value);
   ListExpr l4 = nl->Fourth(value);
   ListExpr l5 = nl->Fifth(value);
   if(!listutils::isNumeric(l1) ||
      !listutils::isNumeric(l2) ||
      !listutils::isNumeric(l3) ||
      !listutils::isNumeric(l4) ||
      (nl->AtomType(l5) != IntType)){
     return  false;
   }
   x0 = listutils::getNumValue(l1);
   y0 = listutils::getNumValue(l2);
   wx = listutils::getNumValue(l3);
   wy = listutils::getNumValue(l4);
   no_cells_x = nl->IntValue(l5);
   SetDefined( (no_cells_x > 0)
                 && !AlmostEqual(wx,0.0)
                 && !AlmostEqual(wy,0.0));

    return true;
}

std::ostream& operator<<(std::ostream& o, const CellGrid2D& u){
  return u.Print(o);
}

