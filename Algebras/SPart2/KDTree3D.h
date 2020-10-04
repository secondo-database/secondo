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
  Class Cell3DTree

*/
class Cell3DTree {
  public:
    Cell3DTree();
    ~Cell3DTree();

    int getCellId();
    void setValFromX(double x1);
    double getValFromX();
    void setValToX(double x2);
    double getValToX();
    void setValFromY(double y1);
    double getValFromY();
    void setValToY(double y2);
    double getValToY();
    void setValFromZ(double z1);
    double getValFromZ();
    void setValToZ(double z2);
    double getValToZ();
    void setCellId(int cell_id);

    friend class KDTree3D;

  private:
    int cellId;
    double x1;
    double x2;
    double y1;
    double y2;
    double z1;
    double z2;
};

/*
  Class Tree3DStructure

*/
class Tree3DStructure {
  public: 
    Tree3DStructure();
    ~Tree3DStructure();

    int getCellId();
    void setValx(double x);
    double getValx();
    void setValy(double y);
    double getValy();
    void setValz(double z);
    double getValz();
    int getAxis();
    int getDepth();
    Tree3DStructure* getLeft();
    Tree3DStructure* getRight();

    void setCellId(int cell_id);
    void setLeft(Tree3DStructure* left_);
    void setRight(Tree3DStructure* right_);
    void setAxis(int axis_);
    void setDepth(int depth_);
    void setIsLeaf(bool leaf_);
    bool isLeaf();

    friend class KDTree3D;

  private:
    double x;
    double y;
    double z;
    int axis; // 0 => x-axis; 1 => y-axis; 2 => z-axis
    int depth;
    Tree3DStructure* left;
    Tree3DStructure* right;
    int cellId; //leaf
    bool leaf; // leaf
  
};

/*
  Class Tree3DMedStructure

*/
class Tree3DMedStructure {
  public:
    Tree3DMedStructure();
    ~Tree3DMedStructure();

    int getCellId();
    void setVal(double val_);
    double getVal();
    int getAxis();
    int getDepth();
    Tree3DMedStructure* getLeft();
    Tree3DMedStructure* getRight();

    void setCellId(int cell_id);
    void setLeft(Tree3DMedStructure* left_);
    void setRight(Tree3DMedStructure* right_);
    void setAxis(int axis_);
    void setDepth(int depth_);
    void setIsLeaf(bool leaf_);
    bool isLeaf();

    friend class KDTree3D;

  private:
    double val;
    int axis;
    int depth;
    Tree3DMedStructure* left;
    Tree3DMedStructure* right;
    int cellId;
    bool leaf;


};


//Represents a 3d-point of a rectangle
struct T3DPoint {
  double x;
  double y;
  double z;
  bool used;
};

// Represents a cuboid of a 3dtree
struct KDRect3D {
  double x1;
  double x2;
  double y1;
  double y2;
  double z1;
  double z2;
};

struct KDNode3D
{
    T3DPoint point;
    int axis; // 0 = x-axis; 1 = y-axis; 2= z-axis
    int depth;
    KDNode3D* left;
    KDNode3D* right;
};

struct Cell3DT
{
  KDRect3D value;
  int id;
  Cell3DT* left;
  Cell3DT* right;
  bool final;
};

struct CellInfo3DTree {
  int cellId;
  Rectangle<3>* cell;

  CellInfo3DTree (int c_id,
    double left, double right, double bottom,
     double top, double front, double back) {
    cellId = c_id;

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
  Class KDTree3D

*/
class KDTree3D {
    public:
    KDTree3D();

    // The first constructor.
    KDTree3D(Rectangle<3> &bounding_box);

    // The copy constructor.
    KDTree3D( const KDTree3D& g );


    Rectangle<3> * getBoundingBox();
    int getMode();

    /* Auxiliary functions for In or/and 
    Value mapping functions */
    void Set(Stream<Rectangle<3>> rStream, 
      Rectangle<3> &bounding_box, int mode);
    // Auxiliary functions for use with vector input
    void SetVector(std::vector<Rectangle<3>>* rVector,
                  Rectangle<3> &bounding_box);
    static std::vector<CellInfo3DTree*> getCellInfoVector(
      KDTree3D *in_ktree3d);
    void create3DTreeVector(std::vector<Rectangle<3>>* rVector);
    void processInputVector(std::vector<Rectangle<3>>* rVector);
    /*std::vector<KDNode3D*> &getPointsVector();
    void setPointsVector(std::vector<KDNode3D*> points_vect);*/
    std::vector<Tree3DStructure*> &getPointsVector();
    void setPointsVector(std::vector<Tree3DStructure*> points_vect);

    std::vector<Tree3DMedStructure*> &getPointsMedVector();
    void setPointsMedVector(std::vector<Tree3DMedStructure*> points_vect);

    std::vector<Cell3DTree> &getCellVector();
    void setCellVector(std::vector<Cell3DTree> cell_vect);

    static ListExpr KdTree3dFeedTypeMap( ListExpr args );
    static int KdTree3dValueMapFeed( Word* args, Word& result, int message,
      Word& local, Supplier s );

    static ListExpr Kdtree3dCellnosTypeMap( ListExpr args );
    static int Kdtree3dValueMapCellnos( Word* args, Word& result, int message,
      Word& local, Supplier s );
    static ListExpr Kdtree3dTRCTypeMap( ListExpr args );
    static int Kdtree3dValueMapTRC( Word* args, Word& result, int message,
      Word& local, Supplier s );
      static ListExpr Kdtree3dTRCCellIdTypeMap( ListExpr args );
    static int Kdtree3dValueMapTRCCellId( Word* args, Word& result, int message,
      Word& local, Supplier s );
    static ListExpr Kdtree3dSCCTypeMap( ListExpr args );
    static int Kdtree3dValueMapSCC( Word* args, Word& result, int message,
      Word& local, Supplier s );


    // Algebra supporting functions

    static ListExpr Property3DTree();

    static ListExpr Out3DTree(ListExpr typeInfo, Word value);

    static Word In3DTree( const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct);

    static Word Create3DTree( const ListExpr typeInfo);

    static void Delete3DTree( const ListExpr typeInfo, Word& w);

    static void Close3DTree( const ListExpr typeInfo, Word& w);

    static Word Clone3DTree( const ListExpr typeInfo, const Word& w);

    static int SizeOf3DTree();

    static bool KindCheck3DTree (ListExpr type, ListExpr& errorInfo);

    inline static const std::string BasicType() {
      return "kdtree3d";
    }

    static const bool checkType(const ListExpr type) {
      return listutils::isSymbol(type, BasicType());
    }

    ~KDTree3D();

    private:
    // Vector with points from input
    std::vector<T3DPoint> points{};
    // Vector with points in preoder
    std::vector<Tree3DStructure*> pointsPreorder{};
    // Vector with points in preoder with median as middle
    std::vector<Tree3DMedStructure*> pointsPreorderMed{};
    std::vector<Cell3DT*> cellsPreorder{};
    std::vector<Cell3DT*> finaleCells;

    // vector of points after kd-tree creation
    std::vector<Tree3DStructure*> pointsVector {};
    std::vector<Tree3DMedStructure*> kd3dmedListVec {};

    Rectangle<3> *boundingBox;
    // mode = 1 => take middle of list for creation, else: median
    int mode; 
    std::vector<Cell3DTree> cellVector {};


    void create3DTree(Stream<Rectangle<3>> rStream);
    void processInput(Stream<Rectangle<3>> rStream);
    void build3DTree();
    // calculates 3DTree recursive
    Tree3DStructure* KDTreeRec3D(std::vector<T3DPoint> point_list,
       int begin, int end, int dim, int depth);
    Tree3DMedStructure* KDTreeMedRec3D(std::vector<T3DPoint> point_list,
       int dim, int depth);

    // returns preorder of points
    void preorder3D (Tree3DStructure* root);
    void preorder3DMed (Tree3DMedStructure* root);

    // returns grid
    void preorderGrid3D (Cell3DT* boundBox,
       std::vector<Tree3DStructure*> pointsPreOrdered);
    void preorderMedGrid3D (Cell3DT* boundBox,
       std::vector<Tree3DMedStructure*> pointsPreOrdered);

    // set cell id
    void setCellId3D(Cell3DTree cell, Tree3DStructure* kdnode);
    void setCellId3D(Cell3DTree cell, Tree3DMedStructure* kdnode);

};