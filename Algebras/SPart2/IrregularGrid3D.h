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


/*
   describes a created 3D cell

*/
class Cell3D {
  public:
    Cell3D();
    ~Cell3D();

    void setValFrom(double valFrom);
    double getValFrom();
    void setValTo(double valTo);
    double getValTo();
    int getNbrOfPoints();
    void setNbrOfPoints(int nbrPoints);
    void setCellId(int cell_id);
    int getCellId();

    friend class IrregularGrid3D;

  private:
    int cellId;
    double valFrom;
    double valTo;
    int nbrPoints;
};


/*
 describes cells divided on the temporal axis

*/
class SCell : public Cell3D {
  public:
    SCell();
    ~SCell();

    SCell* getUpper(); //cell lying on top of current cell
    SCell* getNeighbor(); // cell lying next to current cell

    // Auxiliary functions for In or/and Value mapping functions
    void setUpper(SCell* upper_);
    void setNeighbor(SCell* neighbor);

    friend class IrregularGrid3D;

    private:
      SCell* upper;
      SCell* neighbor;

};

/*
  Class HCell3D

*/
class HCell3D : public Cell3D {
  public:
    HCell3D();
    ~HCell3D();

    std::vector<SCell> &getRect();

    friend class IrregularGrid3D;

  private:
    std::vector<SCell> rect{};
};

/*
  Class VCell3D

*/
class VCell3D : public Cell3D {
  public:
    VCell3D();
    ~VCell3D();

    std::vector<HCell3D> &getRow();

  private:
    // row at a specific level
    std::vector<HCell3D> row {};
};



/*
  Represents a 3D point of a 3D rectangle corner

*/
struct CPoint {
  double x;
  double y;
  double z;
};

struct CellInfo3D {
  int cellId;
  int rectcellId;
  Rectangle<3>* cell;
  int statNbrOfPoints;

  /* Cell as a cuboid
  Rectangle: (x1,x2,y1,y2) => x1=left, x2=right, y1=bottom, y2=top
  
  y2-------------|   z2
  |              |  /
  |              | /
  |--------------|/
  x1/y1          x2/z2

  Cuboid: z1 = front, z2= back

  */
  CellInfo3D (int sc_id, int c_id, int nbr_points,
    double left, double right, double bottom, double top,
     double front, double back) {
    cellId = sc_id; // id of the final cell
    rectcellId = c_id; // id of a 2d cell used to create 3d cells
    statNbrOfPoints = nbr_points;

    double min[3], max[3];
    min[0] = left;
    min[1] = bottom;
    min[2] = front;
    max[0] = right;
    max[1] = top;
    max[2] = back;


    cell = new Rectangle<3>(true, min, max);
  }
};


/*
  Class IrregularGrid3D

*/
class IrregularGrid3D {
  public:
    // Do not use the standard constructor
    IrregularGrid3D();
    // The first constructor.
    IrregularGrid3D(Rectangle<3> &bounding_box,
      int rowCount, int cellCount, int layerCount);

    // The copy constructor.
    IrregularGrid3D( const IrregularGrid3D& g );

    Rectangle<3> * getBoundingBox();
    int getRowCount();
    int getCellCount();
    int getLayerCount();
    

    // Auxiliary functions for In or/and Value mapping functions
    void Set(Stream<Rectangle<3>> rStream, Rectangle<3> &bounding_box,
        int rowCount, int cellCount, int layerCount);
    static std::vector<CellInfo3D*> getCellInfoVector(
      IrregularGrid3D *in_irgrid3d);
      
    void setColumnVector(std::vector<VCell3D> column_vect);
    void setCellVector(std::vector<HCell3D> cell_vect);
    std::vector<VCell3D> &getColumnVector();
    std::vector<HCell3D> &getCellVector();

   // Auxiliary functions for use with vector input
   void SetVector(std::vector<Rectangle<3>>* rVector,
                  Rectangle<3> &bounding_box,
                  int row_count, int cell_count, int layer_count);
   void createIrgrid3DVector(std::vector<Rectangle<3>>* rVector);
   void processInputVector(std::vector<Rectangle<3>>* rVector);

    // operator relevant functions
    static ListExpr IrGrid3dFeedTypeMap( ListExpr args );
    static int IrGrid3dValueMapFeed( Word* args, Word& result, int message,
      Word& local, Supplier s );

    static ListExpr IrGrid3dCellnosTypeMap( ListExpr args );
    static int IrGrid3dValueMapCellnos( Word* args, Word& result, int message,
      Word& local, Supplier s );

    static ListExpr IrGrid3dTRCTypeMap( ListExpr args );
    static int IrGrid3dValueMapTRC( Word* args, Word& result, int message,
      Word& local, Supplier s );

       static ListExpr IrGrid3dTRCCellIdTypeMap( ListExpr args );
    static int IrGrid3dValueMapTRCCellId( Word* args, Word& result, int message,
      Word& local, Supplier s );

    static ListExpr IrGrid3dSCCTypeMap( ListExpr args );
    static int IrGrid3dValueMapSCC( Word* args, Word& result, int message,
      Word& local, Supplier s );

    static ListExpr IrGrid3dBBoxTypeMap( ListExpr args );
    static int IrGrid3dValueMapBBox( Word* args, Word& result, int message,
      Word& local, Supplier s );

    static ListExpr IrGrid2dBBoxTypeMap( ListExpr args );
    static int IrGrid2dValueMapBBox( Word* args, Word& result, int message,
      Word& local, Supplier s );

    // Algebra supporting functions

    static ListExpr PropertyIrGrid3D();

    static ListExpr OutIrGrid3D(ListExpr typeInfo, Word value);

    static Word InIrGrid3D( const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct);

    static Word CreateIrGrid3D( const ListExpr typeInfo);

    static void DeleteIrGrid3D( const ListExpr typeInfo, Word& w);

    static void CloseIrGrid3D( const ListExpr typeInfo, Word& w);

    static Word CloneIrGrid3D( const ListExpr typeInfo, const Word& w);

    static int SizeOfIrGrid3D();

    static bool KindCheckIrGrid3D( ListExpr type, ListExpr& errorInfo);

    inline static const std::string BasicType() {
      return "irgrid3d";
    }

    static const bool checkType(const ListExpr type) {
      return listutils::isSymbol(type, BasicType());
    }

    ~IrregularGrid3D();

  private:
    // points sorted by y-coordinates
  std::vector<CPoint> points{};
  // irregular grid bounding box
  Rectangle<3> * boundingBox;
  // number of rows and cells per row and layers (z-coordinate) per cell
  int rowCount, cellCount, layerCount;
  // column (y-axis) vector with access to the section vectors (x-axes)
  std::vector<VCell3D> columnVector {};
  // cell vector (x-axis) with access to the spacial vectors (z-axes)
  std::vector<HCell3D> cellVector {};

  void createIrgrid3D(Stream<Rectangle<3>> rStream);
  void processInput(Stream<Rectangle<3>> rStream);
  void buildGrid();
};