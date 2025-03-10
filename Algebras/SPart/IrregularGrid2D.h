/*
----
This file is part of SECONDO.

Copyright (C) 2019,
University in Hagen,
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

#pragma once

#include "Stream.h"
#include <vector>
#include "Algebras/Rectangle/RectangleAlgebra.h"

class Cell {
  public:
    Cell();
    ~Cell();

    int getCellId();
    void setValFrom(double valFrom);
    double getValFrom();
    void setValTo(double valTo);
    double getValTo();
    int getNbrOfPoints();
    void setNbrOfPoints(int nbrPoints);

    // Auxiliary functions for In or/and Value mapping functions
    void setCellId(int cell_id);

    friend class IrregularGrid2D;

  private:
    int cellId;
    double valFrom;
    double valTo;
    int nbrPoints;
};

class HCell : public Cell {
  public:
    HCell();
    ~HCell();

    HCell* getUpper();

    // Auxiliary functions for In or/and Value mapping functions
    void setUpper(HCell* upper_);

    friend class IrregularGrid2D;

  private:
    HCell* upper;
};

class VCell : public Cell {
  public:
    VCell();
    ~VCell();

    std::vector<HCell> &getRow();

  private:
    // row at a specific level
    std::vector<HCell> row {};
};

//Represents a point of a rectangle corner
struct RPoint {
  double x;
  double y;
};

// Represents a row cell as rectangle
struct CellInfo {
  int cellId;
  Rectangle<2>* cell;
  int statNbrOfPoints;

  CellInfo (int c_id, int nbr_points,
    double left, double right, double bottom, double top) {
    cellId = c_id;
    statNbrOfPoints = nbr_points;

    double min[2], max[2];
    min[0] = left;
    min[1] = bottom;
    max[0] = right;
    max[1] = top;

    cell = new Rectangle<2>(true, min, max);
  }
};

class IrregularGrid2D {
  public:
    // Do not use the standard constructor
    IrregularGrid2D();
    // The first constructor.
    IrregularGrid2D(Rectangle<2> &bounding_box,
      int rowCount, int cellCount);

    // The copy constructor.
    IrregularGrid2D( const IrregularGrid2D& g );

    Rectangle<2> getBoundingBox();

    int getRowCount();
    int getCellCount();
    std::vector<VCell> &getColumnVector();
    std::vector<Rectangle<2>> &getcCellRectangleVector();

    // Auxiliary functions for In or/and Value mapping functions
    void Set(Stream<Rectangle<2>> rStream, Rectangle<2>& bounding_box,
        int rowCount, int cellCount);
    static std::vector<CellInfo*> getCellInfoVector(
      IrregularGrid2D *in_irgrid2d);
    void setColumnVector(std::vector<VCell> column_vect);

   // Auxiliary functions for use with vector input
   void SetVector(std::vector<Rectangle<2>>* rVector,
                  Rectangle<2> &bounding_box,
                  int row_count, int cell_count);
   void createIrgrid2DVector(std::vector<Rectangle<2>>* rVector);
   void processInputVector(std::vector<Rectangle<2>>* rVector);

    // operator relevant functions
    static ListExpr IrGrid2dFeedTypeMap( ListExpr args );
    static int IrGrid2dValueMapFeed( Word* args, Word& result, int message,
      Word& local, Supplier s );

    static ListExpr IrGrid2dCellnosTypeMap( ListExpr args );
    static int IrGrid2dValueMapCellnos( Word* args, Word& result, int message,
      Word& local, Supplier s );

    static ListExpr IrGrid2dcellToRectTypeMap( ListExpr args );
    static int IrGrid2dValueMapcellToRect( Word* args, Word& result,
      int message, Word& local, Supplier s );

    // Algebra supporting functions

    static ListExpr PropertyIrGrid2D();

    static ListExpr OutIrGrid2D(ListExpr typeInfo, Word value);

    static Word InIrGrid2D( const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct);

    static Word CreateIrGrid2D( const ListExpr typeInfo);

    static void DeleteIrGrid2D( const ListExpr typeInfo, Word& w);

    static void CloseIrGrid2D( const ListExpr typeInfo, Word& w);

    static Word CloneIrGrid2D( const ListExpr typeInfo, const Word& w);

    static int SizeOfIrGrid2D();

    static bool KindCheckIrGrid2D( ListExpr type, ListExpr& errorInfo);

    static bool OpenIrGrid2D(SmiRecord& valueRecord,
      size_t& offset, const ListExpr typeInfo, Word& value);

    static bool SaveIrGrid2D(SmiRecord& valueRecord, size_t& offset,
      const ListExpr typeInfo, Word& value);

    inline static const std::string BasicType() {
      return "irgrid2d";
    }

    static const bool checkType(const ListExpr type) {
      return listutils::isSymbol(type, BasicType());
    }

    ~IrregularGrid2D();

  private:
    // points sorted by y-coordinates
  std::vector<RPoint> points{};
  // irregular grid bounding box
  Rectangle<2> boundingBox;
  // number of rows and cells per row
  int rowCount, cellCount;
  // column (y-axis) vector with access to the section vectors (x-axes)
  std::vector<VCell> columnVector {};
  //Cell representation as rectangles
  std::vector<Rectangle<2>> cell_rectangle_vect{};

  void createIrgrid2D(Stream<Rectangle<2>> rStream);
  void processInput(Stream<Rectangle<2>> rStream);
  void buildGrid();
  void buildCellRectangleVector(IrregularGrid2D *b_irgrid2d);
  Rectangle<2> cellIdToRectangle(IrregularGrid2D *b_irgrid2d, int cellNbr);
};
