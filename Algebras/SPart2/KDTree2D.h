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


// represents grid structure
class Cell2DTree {
  public:
    Cell2DTree();
    ~Cell2DTree();

    int getCellId();
    void setValFromX(double x1);
    double getValFromX();
    void setValToX(double x2);
    double getValToX();
    void setValFromY(double y1);
    double getValFromY();
    void setValToY(double y2);
    double getValToY();
    void setCellId(int cell_id);

    friend class KDTree2D;

  private:
    int cellId;
    double x1;
    double x2;
    double y1;
    double y2;
};

// represents tree structure
class KDNodeList {
  public: 
    KDNodeList();
    ~KDNodeList();

    int getCellId();
    void setValx(double x);
    double getValx();
    void setValy(double y);
    double getValy();
    int getAxis();
    int getDepth();
    KDNodeList* getLeft();
    KDNodeList* getRight();

    void setCellId(int cell_id);
    void setLeft(KDNodeList* left_);
    void setRight(KDNodeList* right_);
    void setAxis(int axis_);
    void setDepth(int depth_);
    void setIsLeaf(bool leaf_);
    bool isLeaf();

    friend class KDTree2D;

  private:
    double x;
    double y;
    int axis; // 0 => x-axis; 1 => y-axis
    int depth;
    KDNodeList* left;
    KDNodeList* right;
    int cellId; //leaf
    bool leaf; // leaf
  
};

class KDMedList {
  public:
  KDMedList();
  ~KDMedList();

  void setVal(double part);
  double getVal();

  KDMedList* getLeft();
  KDMedList* getRight();
  
  int getCellId();
  void setCellId(int cell_id);
  void setLeft(KDMedList* left_);
  void setRight(KDMedList* right_);

  int getAxis();
  int getDepth();
  void setAxis(int axis_);
  void setDepth(int depth_);
  bool isLeaf();

  friend class KDTree2D;


  private:
  double part; // x or y value
  int axis;
  int depth;
  KDMedList* left;
  KDMedList* right;
  int cellId;
  bool isleaf;
};

//Represents a point of a rectangle
struct TPoint {
  double x;
  double y;
  bool used;
};

struct KDRect {
  double x1;
  double x2;
  double y1;
  double y2;
};

struct CellKD
{
  KDRect value;
  int id;
  CellKD* left;
  CellKD* right;
  bool final;
};

struct CellInfo2DTree {
  int cellId;
  Rectangle<2>* cell;

  CellInfo2DTree (int c_id,
    double left, double right, double bottom, double top) {
    cellId = c_id;

    double min[2], max[2];
    min[0] = left;
    min[1] = bottom;
    max[0] = right;
    max[1] = top;

    cell = new Rectangle<2>(true, min, max);
  }
};


class KDTree2D {
    public:
    KDTree2D();

    // The first constructor.
    KDTree2D(Rectangle<2> &bounding_box);

    // The copy constructor.
    KDTree2D( const KDTree2D& g );


    Rectangle<2> * getBoundingBox();
    int getMode();

    // Auxiliary functions for In or/and Value mapping functions
    void Set(Stream<Rectangle<2>> rStream, 
      Rectangle<2> &bounding_box, int mode);
    // Auxiliary functions for use with vector input
    void SetVector(std::vector<Rectangle<2>>* rVector,
                  Rectangle<2> &bounding_box);
    static std::vector<CellInfo2DTree*> getCellInfoVector(
      KDTree2D *in_ktree2d);
    void create2DTreeVector(std::vector<Rectangle<2>>* rVector);
    void processInputVector(std::vector<Rectangle<2>>* rVector);

    std::vector<KDNodeList*> &getPointsVector();
    void setPointsVector(std::vector<KDNodeList*> points_vect);

    std::vector<KDMedList*> &getPointsMedVector();
    void setPointsMedVector(std::vector<KDMedList*> points_vect);

    std::vector<Cell2DTree> &getCellVector();
    void setCellVector(std::vector<Cell2DTree> cell_vect);

    static ListExpr KdTree2dFeedTypeMap( ListExpr args );
    static int KdTree2dValueMapFeed( Word* args, Word& result, int message,
      Word& local, Supplier s );

    static ListExpr Kdtree2dCellnosTypeMap( ListExpr args );
    static ListExpr Kdtree2dTRCTypeMap( ListExpr args );
    static ListExpr Kdtree2dTRCCellIdTypeMap( ListExpr args );

    static ListExpr Kdtree2dSCCTypeMap( ListExpr args );

    static int Kdtree2dValueMapCellnos( Word* args, Word& result, int message,
      Word& local, Supplier s );
    static int Kdtree2dValueMapTRC( Word* args, Word& result, int message,
      Word& local, Supplier s ); 
      static int Kdtree2dValueMapTRCCellId( Word* args, Word& result,
       int message, Word& local, Supplier s ); 
    static int Kdtree2dValueMapSCC( Word* args, Word& result, int message,
      Word& local, Supplier s );  


    // Algebra supporting functions

    static ListExpr Property2DTree();

    static ListExpr Out2DTree(ListExpr typeInfo, Word value);

    static Word In2DTree( const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct);

    static Word Create2DTree( const ListExpr typeInfo);

    static void Delete2DTree( const ListExpr typeInfo, Word& w);

    static void Close2DTree( const ListExpr typeInfo, Word& w);

    static Word Clone2DTree( const ListExpr typeInfo, const Word& w);

    static int SizeOf2DTree();

    static bool KindCheck2DTree (ListExpr type, ListExpr& errorInfo);

    inline static const std::string BasicType() {
      return "kdtree2d";
    }

    static const bool checkType(const ListExpr type) {
      return listutils::isSymbol(type, BasicType());
    }

    ~KDTree2D();

    private:
    // Points from input
    std::vector<TPoint> points{};
    // Points ordered after 2dtree creation
    std::vector<KDNodeList*> pointsPreorder{};
    // Points ordered after 2dtree creation with median as middle
    std::vector<KDMedList*> pointsPreorderMed{};

    // cells in preorder
    std::vector<CellKD*> cellsPreorder{};
    std::vector<CellKD*> finaleCells;

    // vector of points after kd-tree creation
    std::vector<KDNodeList*> pointsVector {};
    // vector of points after kdtree creation with median as middle
    std::vector<KDMedList*> kdmedListVec {};

    Rectangle<2> *boundingBox;
    int mode; // 1=>middle of list, 2=>median of points
    std::vector<Cell2DTree> cellVector {};


    void create2DTree(Stream<Rectangle<2>> rStream);
    void processInput(Stream<Rectangle<2>> rStream);
    void build2DTree();
    // create KDTree recursive
    KDNodeList* KDTreeRec(std::vector<TPoint> point_list, int begin,
       int end, int dim, int depth);
    KDMedList* KDTreeMedRec(std::vector<TPoint> point_list, int dim, int depth);

    // returns points in preoder
    void preorder (KDNodeList* root);
    void preorderMed (KDMedList* root);

    // returns grid
    void preorderGrid (CellKD* boundBox, KDNodeList* node);
      //std::vector<KDNodeList*> pointsPreOrdered);
    void preorderMedGrid (CellKD* boundBox, KDMedList* node); 
      //std::vector<KDMedList*> pointsPreOrdered);
    // sets cell id
    void setCellId(Cell2DTree cell, KDNodeList* kdnode);
    void setCellId(Cell2DTree cell, KDMedList* kdnode);
    bool duplicateP(TPoint p);

};