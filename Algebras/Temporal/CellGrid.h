
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

*/

/*
1 Class CellGrid2D

This class defines a grid in 2d Euclidean space which
is open in y direction.


*/


#include "Attribute.h"
#include "NestedList.h"
#include "RectangleAlgebra.h"
#include "ListUtils.h"

/*
3.8 Grid structures

The following 2 classes are used to represent 2D cell grids and events created
by moving objects on such a grid.

The following class ~CellGrid2D~ describes a regular grid.

*/

class CellGrid2D: public Attribute{
  public:
    CellGrid2D(); // does nothing
    CellGrid2D(const int dummy); // initializes the CellGrid
    CellGrid2D(const double &x0_, const double &y0_,
               const double &wx_, const double &wy_, const int32_t &nox_);
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

     CellGrid2D(const CellGrid2D& other);

     CellGrid2D& operator=(const CellGrid2D& other);


    ~CellGrid2D(); // standard destructor


    bool set(const double x0,
             const double y0,
             const double wx,
             const double wy,
             const int no_cells_x);

    double getX0() const;
    double getY0() const;
    double getXw() const;
    double getYw() const;
    int getNx() const;


    double getMaxX() const;
      // returns the maximum X-coordinate lying on the grid

    double getMaxY() const;
      // returns the maximum Y-coordinate lying on the grid

    double getMinX() const;
      // returns the minimum X-coordinate lying on the grid

    double getMinY() const;
      // returns the minimum Y-coordinate lying on the grid

    bool onGrid(const double &x, const double &y) const;
      // returns true iff (x,y) is located on the grid

    int32_t getCellNo(const double &x, const double &y) const;
      // returns the cell number for a given point (x,y)
      // Only positive cell numbers are valid. Negative result indicates
      // that (x,y) is not located on the grid.

    int32_t getCellNo(const Point &p) const;
    int32_t getXIndex(const double &x) const;
    int32_t getYIndex(const double &y) const;
    Rectangle<2> getMBR() const;
      // returns the grid's MBR as a 2D-rectangle

    Rectangle<2> getRowMBR(const int32_t &n) const;
      // returns the grid's nth row as a 2D-rectangle
      // row numbering starts with 0

    Rectangle<2> getColMBR(const int32_t &n) const;
      // returns the grid's nth column as a 2D-rectangle
      // column numbering starts with 0 and ends with no_cells_x - 1

    bool isValidCellNo(const int32_t &n) const;
      // returns true iff n is a valid grid cell number

    int32_t getInvalidCellNo() const;
      // returns an invalid cell number

    ostream& Print( ostream &os ) const;


/*
1.1 Functions to works as an attribute type


*/
    size_t Sizeof() const;

    int Compare(const Attribute* other) const;

    bool Adjacent(const Attribute* other) const;

    Attribute* Clone() const;

    size_t HashValue() const;

    void CopyFrom(const Attribute* other);

    static const string BasicType();

    static const bool checkType(const ListExpr type){
      ListExpr errorInfo = listutils::emptyErrorInfo();
      return CheckKind(type, errorInfo);
    }

    static ListExpr Property();

    static bool CheckKind(ListExpr type, ListExpr& errorInfo);

    ListExpr ToListExpr(const ListExpr typeInfo)const;

    bool ReadFrom(const ListExpr value,const ListExpr typeInfo);

  private:
    double x0, y0;       // origin of the grid
    double wx, wy;       // cell-widths for both dimensions
    int32_t no_cells_x;  // number of cells along X-axis
};

