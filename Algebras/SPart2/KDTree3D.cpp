/*
----
This file is part of SECONDO.

Copyright (C) 2020,
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


#include "KDTree3D.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Collection/IntSet.h"
#include <iterator>
#include <map>

/*
  Class Cell3DTree

*/
Cell3DTree::Cell3DTree() {
  cellId = -1;
  x1 = -1;
  x2 = -1;
  y1 = -1;
  y2 = -1;
  z1 = -1;
  z2 = -1;
}

void
Cell3DTree::setCellId(int cell_id) {
  this->cellId = cell_id;
}

int
Cell3DTree::getCellId() {
  return cellId;
}

void
Cell3DTree::setValFromX(double x_1) {
  x1 = x_1;
}

double
Cell3DTree::getValFromX() {
  return x1;
}

void
Cell3DTree::setValToX(double x_2) {
  x2 = x_2;
}

double
Cell3DTree::getValToX() {
  return x2;
}

void
Cell3DTree::setValFromY(double y_1) {
  y1 = y_1;
}

double
Cell3DTree::getValFromY() {
  return y1;
}

void
Cell3DTree::setValToY(double y_2) {
  y2 = y_2;
}

double
Cell3DTree::getValToY() {
  return y2;
}

void
Cell3DTree::setValFromZ(double z_1) {
  z1 = z_1;
}

double
Cell3DTree::getValFromZ() {
  return z1;
}

void
Cell3DTree::setValToZ(double z_2) {
  z2 = z_2;
}

double
Cell3DTree::getValToZ() {
  return z2;
}

Cell3DTree::~Cell3DTree() { }

/*
  Class Tree3DStructure

*/
Tree3DStructure::Tree3DStructure() {
  x = -1;
  y = -1;
  z = -1;
  axis = -1;
  depth = -1;
  left = nullptr;
  right = nullptr;
  cellId = -1;
  leaf = false;
}

void
Tree3DStructure::setCellId(int cell_id) {
  this->cellId = cell_id;
}

int
Tree3DStructure::getCellId() {
  return cellId;
}

void Tree3DStructure::setAxis(int axis_) {
  this->axis = axis_;
}

int Tree3DStructure::getAxis() {
  return axis;
}

void Tree3DStructure::setDepth(int depth_) {
  this->depth = depth_;
}

int Tree3DStructure::getDepth() {
  return depth;
}

void Tree3DStructure::setValx(double x_) {
  this->x = x_;
}

double Tree3DStructure::getValx() {
  return x;
}

void Tree3DStructure::setValy(double y_) {
  this->y = y_;
}

double Tree3DStructure::getValy() {
  return y;
}

void Tree3DStructure::setValz(double z_) {
  this->z = z_;
}

double Tree3DStructure::getValz() {
  return z;
}

void Tree3DStructure::setLeft(Tree3DStructure* left_) {
  this->left = left_;
}

Tree3DStructure* Tree3DStructure::getLeft() {
  return left;
}

void Tree3DStructure::setRight(Tree3DStructure* right_) {
  this->right = right_;
} 

Tree3DStructure* Tree3DStructure::getRight() {
  return right;
}

void Tree3DStructure::setIsLeaf(bool leaf_) {
  this->leaf = leaf_;
}

bool Tree3DStructure::isLeaf() {
  return leaf;
}

Tree3DStructure::~Tree3DStructure() {}


/*
  Class Tree3DMedStructure
  Uses median of list of points to create 3dtree

*/
Tree3DMedStructure::Tree3DMedStructure() {
  val = -1;
  axis = -1;
  depth = -1;
  left = nullptr;
  right = nullptr;
  cellId = -1;
  leaf = false;
}

void Tree3DMedStructure::setVal(double val_) {
  this->val = val_;
}

double Tree3DMedStructure::getVal() {
  return val;
}

void Tree3DMedStructure::setAxis(int axis_) {
  this->axis = axis_;
}

int Tree3DMedStructure::getAxis() {
  return axis;
}

void Tree3DMedStructure::setCellId(int cell_id) {
  this->cellId = cell_id;
}

int Tree3DMedStructure::getCellId() {
  return cellId;
}

void Tree3DMedStructure::setDepth(int depth_) {
  this->depth = depth_;
}

int Tree3DMedStructure::getDepth() {
  return depth;
}

void Tree3DMedStructure::setIsLeaf(bool leaf_) {
  this->leaf = leaf_;
}

bool Tree3DMedStructure::isLeaf() {
  return leaf;
}

void Tree3DMedStructure::setLeft(Tree3DMedStructure* left_) {
  this->left = left_;
}

Tree3DMedStructure* Tree3DMedStructure::getLeft() {
  return left;
}

void Tree3DMedStructure::setRight(Tree3DMedStructure* right_) {
  this->right = right_;
}

Tree3DMedStructure* Tree3DMedStructure::getRight() {
  return right;
}

Tree3DMedStructure::~Tree3DMedStructure() {}

/*
  Class KDTree3D

*/
KDTree3D::KDTree3D() {
    boundingBox = nullptr;
}

KDTree3D::KDTree3D(const KDTree3D& g) {
  boundingBox = g.boundingBox;
}

KDTree3D::KDTree3D(Rectangle<3> &bounding_box) {
  boundingBox = &bounding_box;
}

void KDTree3D::Set(Stream<Rectangle<3>> rStream,
    Rectangle<3> &bounding_box, int mode_) {
  boundingBox = &bounding_box;
  mode = mode_;

  create3DTree(rStream);
}

void KDTree3D::SetVector(std::vector<Rectangle<3>>* rVector,
                     Rectangle<3> &bounding_box) {
   boundingBox = &bounding_box;

   create3DTreeVector(rVector);
}

/*void makelistofRects()
{
  double x1 = 0.0;
  double x2 = 1.0;
  double a =0.2;
  double y1 = 0.0;
  double y2 = 1.0;
  double b = 0.5;

  for(int i=0; i < 500; i++)
  {
    printf("\n ((%.2f %.2f %.2f %.2f))", x1, x2, y1, y2);
    x1+=a;
    x2+=a;

    for(int e=0; e < 100; e++)
    {
      printf("\n ((%.2f %.2f %.2f %.2f))", x1, x2, y1, y2);
    
      y1+=b;
      y2+=b;
    }
  }

  x1 = 2000.0;
  x2 = 2001.0;
  y1 = 5100.0;
  y2 = 5101.0;
  a = 1.0;
  b = 0.2;
  for(int i=0; i < 500; i++)
  {
    printf("\n ((%.2f %.2f %.2f %.2f))", x1, x2, y1, y2);
    x1+=a;
    x2+=a;
    for(int e=0; e < 100; e++)
    {
      printf("\n ((%.2f %.2f %.2f %.2f))", x1, x2, y1, y2);
      y1+=b;
      y2+=b;
    }
  }
}*/

void KDTree3D::create3DTree(Stream<Rectangle<3>> rStream) {
    processInput(rStream);

  // create 3dtree
  build3DTree();

}

void KDTree3D::create3DTreeVector(std::vector<Rectangle<3>>* rVector) {
     processInputVector(rVector);

   // create kdtree
   build3DTree();
}

Rectangle<3> *
KDTree3D::getBoundingBox() {
  return boundingBox;
}

int
KDTree3D::getMode() {
  return mode;
}

void
KDTree3D::setPointsVector(std::vector<Tree3DStructure*> points_vect) {
  this->pointsVector = points_vect;
}


std::vector<Tree3DStructure*>&
KDTree3D::getPointsVector() {
  return this->pointsVector;
}

void
KDTree3D::setPointsMedVector(std::vector<Tree3DMedStructure*> points_vect) {
  this->kd3dmedListVec = points_vect;
}


std::vector<Tree3DMedStructure*>&
KDTree3D::getPointsMedVector() {
  return this->kd3dmedListVec;
}

void
KDTree3D::setCellVector(std::vector<Cell3DTree> cell_vect) {
  this->cellVector = cell_vect;
}


std::vector<Cell3DTree>&
KDTree3D::getCellVector() {
  return this->cellVector;
}

T3DPoint
getCuboidCentre3DTree(Rectangle<3>* r) {
  double a = (r->getMaxY() - r->getMinY()) / (double)2;
  double b = (r->getMaxX() - r->getMinX()) / (double)2;
  double c = (r->getMaxZ() - r->getMinZ()) / (double)2;

  T3DPoint r_c { (r->getMinX())+b, (r->getMinY())+a, (r->getMinZ())+c};

  return r_c;
}

/*
   checks if a rectangle <3> is inside the irgrid3d bounding box

*/
bool
insideRectangle3DTree(Rectangle<3>* bbox, Rectangle<3>* r) {
  double le = bbox->getMinX();
  double ri = bbox->getMaxX();
  double bo = bbox->getMinY();
  double to = bbox->getMaxY();
  double fr = bbox->getMinZ();
  double ba = bbox->getMaxZ();

  if (r->getMinX() >= le && r->getMaxX() <= ri
      && r->getMinY() >= bo && r->getMaxY() <=to
      && r->getMinZ() >= fr && r->getMaxZ() <= ba) {                        

    return true;
  }

 return false;
}

bool
pointComparisonX(T3DPoint p1, T3DPoint p2) {
  return p1.x < p2.x;
}

bool
xValueComparison(Cell3DTree x1, Cell3DTree x2) {
  return x1.getValFromX() < x2.getValFromX();
}

bool
yValueComparison(Cell3DTree y1, Cell3DTree y2) {
  return y1.getValFromY() < y2.getValFromY();
}

bool
zValueComparison(Cell3DTree z1, Cell3DTree z2) {
  return z1.getValFromZ() < z2.getValFromZ();
}

bool
pointComparisonY(T3DPoint p1, T3DPoint p2) {
  return p1.y < p2.y;
}

bool
pointComparisonZ(T3DPoint p1, T3DPoint p2) {
  return p1.z < p2.z;
}


std::vector<T3DPoint> slice(std::vector<T3DPoint> const &v, int m, int n)
{
  auto first = v.cbegin() + m;
  auto last = v.cbegin() + n + 1;
  
	std::vector<T3DPoint> vec(first, last);
	return vec;
}


/*
  Creates 3DTree recursive with median of points

*/
Tree3DMedStructure*
KDTree3D::KDTreeMedRec3D(std::vector<T3DPoint> point_list,
   int dim = 3, int depth = 0) 
{ 

  //printf("\n in kdtreemedrec3d");
  int axis = depth % dim;
  //printf("\n axis: %d", axis);
  double median;
  std::vector<T3DPoint> point_list_left;
  std::vector<T3DPoint> point_list_right;
  if (point_list_left.size() > 0) {
      point_list_left.clear();
  }
  if (point_list_right.size() > 0) {
      point_list_right.clear();
  }
  
  int mid = point_list.size()/2;
  //printf("\n mid: %d, size: %d", mid, (int)point_list.size());
  // x-axis
  if(axis == 0) {
    std::sort(point_list.begin(), point_list.end(), 
          pointComparisonX);
    // calculate median
    //printf("\n in axis= 0");
    if(point_list.size() % 2 == 0) 
    {
      median = 0.5*(point_list.at(mid-1).x + point_list.at(mid).x);
    } else {
      //printf("\n point list at mid: %.2f", point_list[mid].x);
      median = point_list.at(mid).x;
      //printf("\n -----median: %.2f", median);
    } 
    for(size_t i=0; i < point_list.size(); i++)
    {
      if(depth > 40000) {
        //printf("\n elemt: %.2f", point_list[i].x);
      }
      if(point_list.at(i).x < median)
      {
        point_list_left.push_back(point_list.at(i));
      } else {
        point_list_right.push_back(point_list.at(i));
      }
    }

    // repartition if size of point_list_left or point_list_right 
    // corresponds to size of (old) point_list from input
    // in case take middle position as median
    if(point_list_left.size() == point_list.size() 
    || point_list_right.size() == point_list.size())
    {
      if (point_list_left.size() > 0) {
        point_list_left.clear();
      }
      if (point_list_right.size() > 0) {
        point_list_right.clear();
      }
      //printf("\n same size");
      median = point_list[mid].x;
      for(int l=0; l < mid; l++)
      {
        point_list_left.push_back(point_list[l]);
      }
      for(size_t r=mid; r < point_list.size(); r++)
      {
        point_list_right.push_back(point_list[r]);
      }
    }

  } else if (axis == 1) { // y-axis
  //printf("\n in axis=1");
    std::sort(point_list.begin(), point_list.end(), 
        pointComparisonY);

    if(point_list.size() % 2 == 0) 
    {
      median = 0.5*(point_list.at(mid-1).y + point_list.at(mid).y);
    } else {
      median = point_list.at(mid).y;
    }
      //printf("\n -----median: %.2f", median);

    for(size_t i=0; i < point_list.size(); i++)
    {
      if(depth > 40000) {
        //printf("\n elemt: %.2f", point_list[i].y);
      }
      if(point_list.at(i).y < median)
      {
        point_list_left.push_back(point_list.at(i));
      } else {
        point_list_right.push_back(point_list.at(i));
      }
    }

    // repartition if size of point_list_left or point_list_right 
    // corresponds to size of (old) point_list from input
    // in case take middle position as median
    if(point_list_left.size() == point_list.size() 
    || point_list_right.size() == point_list.size())
    {
      if (point_list_left.size() > 0) {
        point_list_left.clear();
      }
      if (point_list_right.size() > 0) {
        point_list_right.clear();
      }
      //printf("\n same size");
      median = point_list[mid].y;
      for(int l=0; l < mid; l++)
      {
        point_list_left.push_back(point_list[l]);
      }
      for(size_t r=mid; r < point_list.size(); r++)
      {
        point_list_right.push_back(point_list[r]);
      }
    } 
  } else if( axis == 2) { // z-axis
  //printf("\n in axis = 2");
      std::sort(point_list.begin(), point_list.end(),
      pointComparisonZ);

    if(point_list.size() % 2 == 0) 
    {
      median = 0.5*(point_list.at(mid-1).z + point_list.at(mid).z);
    } else {
      median = point_list.at(mid).z;
    }
      //printf("\n -----median: %.2f", median);

    for(size_t i=0; i < point_list.size(); i++)
    {
      if(depth > 40000) {
        //printf("\n elemt: %.2f", point_list[i].z);
      }
      if(point_list.at(i).z < median)
      {
        point_list_left.push_back(point_list.at(i));
      } else {
        point_list_right.push_back(point_list.at(i));
      }
    }
    // repartition if size of point_list_left or point_list_right 
    // corresponds to size of (old) point_list from input
    // in case take middle position as median
    if(point_list_left.size() == point_list.size() 
    || point_list_right.size() == point_list.size())
    {
      if (point_list_left.size() > 0) {
        point_list_left.clear();
      }
      if (point_list_right.size() > 0) {
        point_list_right.clear();
      }
      //printf("\n same size");
      median = point_list[mid].z;
      for(int l=0; l < mid; l++)
      { 
        point_list_left.push_back(point_list[l]);
      }
      for(size_t r=mid; r < point_list.size(); r++)
      {
        point_list_right.push_back(point_list[r]);
      }
    }
  } else {
    //printf("\n in else axis");
    return 0;
  }

  
  Tree3DMedStructure* tmp = new Tree3DMedStructure();
    //printf("\n median: %.2f", median);
    tmp->setAxis(axis);
    tmp->setDepth(depth);
    tmp->setVal(median);


    if(point_list_left.size() > 1) {
      tmp->left = KDTreeMedRec3D(point_list_left, 3, depth+1); 
    }
    if(point_list_right.size() > 1) {
      tmp->right = KDTreeMedRec3D(point_list_right, 3, depth+1);
    }

    //printf("\n before push");
    // push current node in vector
    kd3dmedListVec.push_back(tmp);
    if (point_list_left.size() > 0) {
      point_list_left.clear();
    }
    if (point_list_right.size() > 0) {
      point_list_right.clear();
    }

    return tmp;


}


/*
  Creates 3DTree recursive with the middle of the list of points

*/
Tree3DStructure*
KDTree3D::KDTreeRec3D(std::vector<T3DPoint> point_list, int begin,
 int end, int dim = 3, int depth = 0) {
  
  if(begin == end) {
    // only one element in list: leaf
    Tree3DStructure* lastNode = new Tree3DStructure();
    if(!point_list.empty()) {          
      lastNode->setValx(point_list[0].x);
      lastNode->setValy(point_list[0].y);
      lastNode->setValz(point_list[0].z);
      lastNode->setAxis(depth%dim);
      lastNode->setDepth(depth);

      // left and right for cellIds
      Tree3DStructure* leftl = new Tree3DStructure();
      leftl->setCellId(-1);
      leftl->setIsLeaf(true);
      Tree3DStructure* rightl = new Tree3DStructure();
      rightl->setCellId(-1);
      rightl->setIsLeaf(true);
      lastNode->setLeft(leftl);
      lastNode->setRight(rightl);

      pointsVector.push_back(lastNode);
    }
    return lastNode;
  }

  if(end < begin) {
    return nullptr;
  } 

  int axis = depth % dim;
  if(axis == 0) {
  // points sorted by x
  std::sort(point_list.begin(), point_list.end(), 
          pointComparisonX);
  } else if (axis == 1) {
    std::sort(point_list.begin(), point_list.end(), 
        pointComparisonY);
  } else if( axis == 2) {
      std::sort(point_list.begin(), point_list.end(),
      pointComparisonZ);
  }

  int medianpos = point_list.size() / 2;
  T3DPoint middle = point_list[medianpos];
  Tree3DStructure* curr = new Tree3DStructure();

  if(!point_list[medianpos].used) {
    curr->setValx(middle.x);
    curr->setValy(middle.y);
    curr->setValz(middle.z);
    curr->setAxis(axis);
    curr->setDepth(depth);
  
    std::vector<T3DPoint> point_list_left;
    std::vector<T3DPoint> point_list_right;
    point_list[medianpos].used = true;

    // divide list of points in right and left part
    for(int l = 0; l <= medianpos-1; l++)
    {
      point_list_left.push_back(point_list[l]);
    }
  
    int pos = 0;

    for(int r = medianpos+1; r <= (int)point_list.size()-1; r++)
    {
      point_list_right.push_back(point_list[r]);
      pos++;

    }

    
    // in case left/right is empty: prepare leaf
    Tree3DStructure* leftk = new Tree3DStructure();
    leftk->setCellId(-1);
    leftk->setIsLeaf(true);
    Tree3DStructure* rightk = new Tree3DStructure();
    rightk->setCellId(-1);
    rightk->setIsLeaf(true);
  

    // recursive call with left and right lists
    if(!point_list_left.empty()) {
      curr->left = KDTreeRec3D(point_list_left, 0, medianpos-1,  3, depth+1);
    } else {
      curr->setLeft(leftk);
    }
    if(!point_list_right.empty()) {
      curr->right = KDTreeRec3D(point_list_right, 
      medianpos+1, (int)point_list.size(), 3, depth+1);
    } else {
      curr->setRight(rightk);
    }

    // push current node
    pointsVector.push_back(curr);

    // clear aux. vectors
    if (point_list_left.size() > 0) {
      point_list_left.clear();
    }
    if (point_list_right.size() > 0) {
      point_list_right.clear();
    }

    }
  
    return curr;
}

/*
  Order points in preoder

*/
void 
KDTree3D::preorder3D (Tree3DStructure* root)
{
  if (root == nullptr ) {
    return;
  }
  pointsPreorder.push_back(root);

  if(root->left != nullptr) {
    preorder3D(root->left);
  }

  if(root->right != nullptr) {
    preorder3D(root->right);
  }

  return;
}

/*
  order points of median version in preoder

*/
void 
KDTree3D::preorder3DMed(Tree3DMedStructure* root)
{
  if (root == nullptr) {
    return;
  } 
  pointsPreorderMed.push_back(root);
  if(root->left != nullptr) {
    preorder3DMed(root->left);
  }

  if(root->right != nullptr) {
    preorder3DMed(root->right);
  }

  return;
}


/*
  create grid with points of median version

*/
void
KDTree3D::preorderMedGrid3D(Cell3DT* boundBox,
Tree3DMedStructure* node)
   //std::vector<Tree3DMedStructure*> pointsPreOrdered)
{
  boundBox->left = new Cell3DT();
  Cell3DT* cell_left = boundBox->left;
  boundBox->right = new Cell3DT();
  Cell3DT* cell_right = boundBox->right;

  if(node->getAxis() == 0)
  {
    cell_left->value.x1 = boundBox->value.x1;
  cell_left->value.x2 = node->getVal();
  cell_left->value.y1 = boundBox->value.y1;
  cell_left->value.y2 = boundBox->value.y2;
  cell_left->value.z1 = boundBox->value.z1;
  cell_left->value.z2 = boundBox->value.z2;
  cell_left->final = true;

  cell_right->value.x1 = node->getVal();
  cell_right->value.x2 = boundBox->value.x2;
  cell_right->value.y1 = boundBox->value.y1;
  cell_right->value.y2 = boundBox->value.y2;
  cell_right->value.z1 = boundBox->value.z1;
  cell_right->value.z2 = boundBox->value.z2;
  cell_right->final = true;

  boundBox->final = false;
  //cellsPreorder.push_back(boundBox);
  cellsPreorder.push_back(cell_right);
  cellsPreorder.push_back(cell_left);
  } else if(node->getAxis() == 1)
  {
    cell_left->value.y1 = boundBox->value.y1;
    cell_left->value.y2 = node->getVal();
    cell_left->value.x1 = boundBox->value.x1;
    cell_left->value.x2 = boundBox->value.x2;
    cell_left->value.z1 = boundBox->value.z1;
    cell_left->value.z2 = boundBox->value.z2;
    cell_left->final = true;


    cell_right->value.y1 = node->getVal();
    cell_right->value.y2 = boundBox->value.y2;
    cell_right->value.x1 = boundBox->value.x1;
    cell_right->value.x2 = boundBox->value.x2;
    cell_right->value.z1 = boundBox->value.z1;
    cell_right->value.z2 = boundBox->value.z2;

    cell_right->final = true;

    boundBox->final = false;

    //cellsPreorder.push_back(boundBox);
    cellsPreorder.push_back(cell_right);
    cellsPreorder.push_back(cell_left);
  } else {
    cell_left->value.y1 = boundBox->value.y1;
    cell_left->value.y2 = boundBox->value.y2;
    cell_left->value.x1 = boundBox->value.x1;
    cell_left->value.x2 = boundBox->value.x2;
    cell_left->value.z2 = node->getVal();
    cell_left->value.z1 = boundBox->value.z1;
    cell_left->final = true;


    cell_right->value.y1 = boundBox->value.y1;
    cell_right->value.y2 = boundBox->value.y2;
    cell_right->value.x1 = boundBox->value.x1;
    cell_right->value.x2 = boundBox->value.x2;
    cell_right->value.z1 = node->getVal();
    cell_right->value.z2 = boundBox->value.z2;
    cell_right->final = true;

    boundBox->final = false;

    //cellsPreorder.push_back(boundBox);
    cellsPreorder.push_back(cell_right);
    cellsPreorder.push_back(cell_left);
  }

  if(node->getLeft() != nullptr) {
    preorderMedGrid3D(cell_left, node->getLeft());
  }
  if(node->getRight() != nullptr) {
    preorderMedGrid3D(cell_right, node->getRight());
  }

 /* if (pointsPreOrdered.empty()) {
    return;
  }

  boundBox->left = new Cell3DT();
  Cell3DT* cell_left = boundBox->left;
  boundBox->right = new Cell3DT();
  Cell3DT* cell_right = boundBox->right;

  std::vector<Tree3DMedStructure*> pointsPreOrdered_left; // < current
  std::vector<Tree3DMedStructure*> pointsPreOrdered_right; // > current

  if(pointsPreOrdered.front()->axis == 0) {

  // divide list of points bei elements smaller and bigger than first element
  for(int i = 1; i < (int)pointsPreOrdered.size(); i++ )
  {
    if(pointsPreOrdered.at(i)->getVal() < pointsPreOrdered.front()->getVal()) {
      pointsPreOrdered_left.push_back(pointsPreOrdered.at(i));
    } else if (pointsPreOrdered.at(i)->getVal() 
        >= pointsPreOrdered.front()->getVal()) {
      pointsPreOrdered_right.push_back(pointsPreOrdered.at(i));
    }
  }
  
  cell_left->value.x1 = boundBox->value.x1;
  cell_left->value.x2 = pointsPreOrdered.front()->getVal();
  cell_left->value.y1 = boundBox->value.y1;
  cell_left->value.y2 = boundBox->value.y2;
  cell_left->value.z1 = boundBox->value.z1;
  cell_left->value.z2 = boundBox->value.z2;
  cell_left->final = true;

  cell_right->value.x1 = pointsPreOrdered.front()->getVal();
  cell_right->value.x2 = boundBox->value.x2;
  cell_right->value.y1 = boundBox->value.y1;
  cell_right->value.y2 = boundBox->value.y2;
  cell_right->value.z1 = boundBox->value.z1;
  cell_right->value.z2 = boundBox->value.z2;
  cell_right->final = true;

  boundBox->final = false;
  cellsPreorder.push_back(boundBox);
  cellsPreorder.push_back(cell_right);
  cellsPreorder.push_back(cell_left);

  } else if (pointsPreOrdered.front()->axis == 1) {

     // divide list of points in elements smaller and bigger than first element
    for(int i = 1; i < (int)pointsPreOrdered.size(); i++ )
    {
      if(pointsPreOrdered.at(i)->getVal() 
        < pointsPreOrdered.front()->getVal()) {
        pointsPreOrdered_left.push_back(pointsPreOrdered.at(i));
      } else if (pointsPreOrdered.at(i)->getVal() 
        >= pointsPreOrdered.front()->getVal()) {
        pointsPreOrdered_right.push_back(pointsPreOrdered.at(i));
      }
    }

    cell_left->value.y1 = boundBox->value.y1;
    cell_left->value.y2 = pointsPreOrdered.front()->getVal();
    cell_left->value.x1 = boundBox->value.x1;
    cell_left->value.x2 = boundBox->value.x2;
    cell_left->value.z1 = boundBox->value.z1;
    cell_left->value.z2 = boundBox->value.z2;
    cell_left->final = true;


    cell_right->value.y1 = pointsPreOrdered.front()->getVal();
    cell_right->value.y2 = boundBox->value.y2;
    cell_right->value.x1 = boundBox->value.x1;
    cell_right->value.x2 = boundBox->value.x2;
    cell_right->value.z1 = boundBox->value.z1;
    cell_right->value.z2 = boundBox->value.z2;

    cell_right->final = true;

    boundBox->final = false;

    cellsPreorder.push_back(boundBox);
    cellsPreorder.push_back(cell_right);
    cellsPreorder.push_back(cell_left);

  } else if (pointsPreOrdered.front()->axis == 2) {

     // divide list of points in elements smaller and bigger than first element
    for(int i = 1; i < (int)pointsPreOrdered.size(); i++ )
    {
      if(pointsPreOrdered.at(i)->getVal() 
        < pointsPreOrdered.front()->getVal()) {
        pointsPreOrdered_left.push_back(pointsPreOrdered.at(i));
      } else if (pointsPreOrdered.at(i)->getVal() 
        >= pointsPreOrdered.front()->getVal()) {
        pointsPreOrdered_right.push_back(pointsPreOrdered.at(i));
      }
    }

    cell_left->value.y1 = boundBox->value.y1;
    cell_left->value.y2 = boundBox->value.y2;
    cell_left->value.x1 = boundBox->value.x1;
    cell_left->value.x2 = boundBox->value.x2;
    cell_left->value.z2 = pointsPreOrdered.front()->getVal();
    cell_left->value.z1 = boundBox->value.z1;
    cell_left->final = true;


    cell_right->value.y1 = boundBox->value.y1;
    cell_right->value.y2 = boundBox->value.y2;
    cell_right->value.x1 = boundBox->value.x1;
    cell_right->value.x2 = boundBox->value.x2;
    cell_right->value.z1 = pointsPreOrdered.front()->getVal();
    cell_right->value.z2 = boundBox->value.z2;
    cell_right->final = true;

    boundBox->final = false;

    cellsPreorder.push_back(boundBox);
    cellsPreorder.push_back(cell_right);
    cellsPreorder.push_back(cell_left);

  }

  pointsPreOrdered.erase(pointsPreOrdered.begin());

  
  preorderMedGrid3D(cell_left, pointsPreOrdered_left);
  preorderMedGrid3D(cell_right, pointsPreOrdered_right);
  */
}


/*
  create grid of 3dtree values

*/
void
KDTree3D::preorderGrid3D (Cell3DT* boundBox, 
  Tree3DStructure* node)
  //std::vector<Tree3DStructure*> pointsPreOrdered)
  {
    boundBox->left = new Cell3DT();
    Cell3DT* cell_left = boundBox->left;
    boundBox->right = new Cell3DT();
    Cell3DT* cell_right = boundBox->right;

    if(node->getAxis() == 0)
    {
      cell_left->value.x1 = boundBox->value.x1;
      cell_left->value.x2 = node->x;
      cell_left->value.y1 = boundBox->value.y1;
      cell_left->value.y2 = boundBox->value.y2;
      cell_left->value.z1 = boundBox->value.z1;
      cell_left->value.z2 = boundBox->value.z2;
      cell_left->final = true;

      cell_right->value.x1 = node->x;
      cell_right->value.x2 = boundBox->value.x2;
      cell_right->value.y1 = boundBox->value.y1;
      cell_right->value.y2 = boundBox->value.y2;
      cell_right->value.z1 = boundBox->value.z1;
      cell_right->value.z2 = boundBox->value.z2;
      cell_right->final = true;

      boundBox->final = false;
      //cellsPreorder.push_back(boundBox);
      cellsPreorder.push_back(cell_right);
      cellsPreorder.push_back(cell_left);

    } else if(node->getAxis() == 1) 
    {
      cell_left->value.y1 = boundBox->value.y1;
      cell_left->value.y2 = node->y;
      cell_left->value.x1 = boundBox->value.x1;
      cell_left->value.x2 = boundBox->value.x2;
      cell_left->value.z1 = boundBox->value.z1;
      cell_left->value.z2 = boundBox->value.z2;
      cell_left->final = true;


      cell_right->value.y1 = node->y;
      cell_right->value.y2 = boundBox->value.y2;
      cell_right->value.x1 = boundBox->value.x1;
      cell_right->value.x2 = boundBox->value.x2;
      cell_right->value.z1 = boundBox->value.z1;
      cell_right->value.z2 = boundBox->value.z2;
      cell_right->final = true;

      boundBox->final = false;

      //cellsPreorder.push_back(boundBox);
      cellsPreorder.push_back(cell_right);
      cellsPreorder.push_back(cell_left);

    } else if(node->getAxis() == 2)
    {
      cell_left->value.y1 = boundBox->value.y1;
      cell_left->value.y2 = boundBox->value.y2;
      cell_left->value.x1 = boundBox->value.x1;
      cell_left->value.x2 = boundBox->value.x2;
      cell_left->value.z2 = node->z;
      cell_left->value.z1 = boundBox->value.z1;
      cell_left->final = true;


      cell_right->value.y1 = boundBox->value.y1;
      cell_right->value.y2 = boundBox->value.y2;
      cell_right->value.x1 = boundBox->value.x1;
      cell_right->value.x2 = boundBox->value.x2;
      cell_right->value.z1 = node->z;
      cell_right->value.z2 = boundBox->value.z2;
      cell_right->final = true;

      boundBox->final = false;

      //cellsPreorder.push_back(boundBox);
      cellsPreorder.push_back(cell_right);
      cellsPreorder.push_back(cell_left);

    } else {
      return;
    }

    if(node->getLeft() != nullptr && !node->getLeft()->isLeaf()) {
      preorderGrid3D(cell_left, node->getLeft());
    }
    if(node->getRight() != nullptr && !node->getRight()->isLeaf()) {
      preorderGrid3D(cell_right, node->getRight());
    }

  /*if (pointsPreOrdered.empty()) {
    return;
  }

  boundBox->left = new Cell3DT();
  Cell3DT* cell_left = boundBox->left;
  boundBox->right = new Cell3DT();
  Cell3DT* cell_right = boundBox->right;

  std::vector<Tree3DStructure*> pointsPreOrdered_left; // < current
  std::vector<Tree3DStructure*> pointsPreOrdered_right; // > current

  if(pointsPreOrdered.front()->axis == 0) {

  // divide list of points bei elements smaller and bigger than first element
  for(int i = 1; i < (int)pointsPreOrdered.size(); i++ )
  {
    if(pointsPreOrdered.at(i)->x < pointsPreOrdered.front()->x) {
      pointsPreOrdered_left.push_back(pointsPreOrdered.at(i));
    } else if (pointsPreOrdered.at(i)->x > pointsPreOrdered.front()->x) {
      pointsPreOrdered_right.push_back(pointsPreOrdered.at(i));
    }
  }
  
  cell_left->value.x1 = boundBox->value.x1;
  cell_left->value.x2 = pointsPreOrdered.front()->x;
  cell_left->value.y1 = boundBox->value.y1;
  cell_left->value.y2 = boundBox->value.y2;
  cell_left->value.z1 = boundBox->value.z1;
  cell_left->value.z2 = boundBox->value.z2;
  cell_left->final = true;

  cell_right->value.x1 = pointsPreOrdered.front()->x;
  cell_right->value.x2 = boundBox->value.x2;
  cell_right->value.y1 = boundBox->value.y1;
  cell_right->value.y2 = boundBox->value.y2;
  cell_right->value.z1 = boundBox->value.z1;
  cell_right->value.z2 = boundBox->value.z2;
  cell_right->final = true;

  boundBox->final = false;
  cellsPreorder.push_back(boundBox);
  cellsPreorder.push_back(cell_right);
  cellsPreorder.push_back(cell_left);

  } else if (pointsPreOrdered.front()->axis == 1) {

     // divide list of points in elements smaller and bigger than first element
    for(int i = 1; i < (int)pointsPreOrdered.size(); i++ )
    {
      if(pointsPreOrdered.at(i)->y < pointsPreOrdered.front()->y) {
        pointsPreOrdered_left.push_back(pointsPreOrdered.at(i));
      } else if (pointsPreOrdered.at(i)->y > pointsPreOrdered.front()->y) {
        pointsPreOrdered_right.push_back(pointsPreOrdered.at(i));
      }
    }

    cell_left->value.y1 = boundBox->value.y1;
    cell_left->value.y2 = pointsPreOrdered.front()->y;
    cell_left->value.x1 = boundBox->value.x1;
    cell_left->value.x2 = boundBox->value.x2;
    cell_left->value.z1 = boundBox->value.z1;
    cell_left->value.z2 = boundBox->value.z2;
    cell_left->final = true;


    cell_right->value.y1 = pointsPreOrdered.front()->y;
    cell_right->value.y2 = boundBox->value.y2;
    cell_right->value.x1 = boundBox->value.x1;
    cell_right->value.x2 = boundBox->value.x2;
    cell_right->value.z1 = boundBox->value.z1;
    cell_right->value.z2 = boundBox->value.z2;

    cell_right->final = true;

    boundBox->final = false;

    cellsPreorder.push_back(boundBox);
    cellsPreorder.push_back(cell_right);
    cellsPreorder.push_back(cell_left);

  } else if (pointsPreOrdered.front()->axis == 2) {

     // divide list of points in elements smaller and bigger than first element
    for(int i = 1; i < (int)pointsPreOrdered.size(); i++ )
    {
      if(pointsPreOrdered.at(i)->z < pointsPreOrdered.front()->z) {
        pointsPreOrdered_left.push_back(pointsPreOrdered.at(i));
      } else if (pointsPreOrdered.at(i)->z > pointsPreOrdered.front()->z) {
        pointsPreOrdered_right.push_back(pointsPreOrdered.at(i));
      }
    }

    cell_left->value.y1 = boundBox->value.y1;
    cell_left->value.y2 = boundBox->value.y2;
    cell_left->value.x1 = boundBox->value.x1;
    cell_left->value.x2 = boundBox->value.x2;
    cell_left->value.z2 = pointsPreOrdered.front()->z;
    cell_left->value.z1 = boundBox->value.z1;
    cell_left->final = true;


    cell_right->value.y1 = boundBox->value.y1;
    cell_right->value.y2 = boundBox->value.y2;
    cell_right->value.x1 = boundBox->value.x1;
    cell_right->value.x2 = boundBox->value.x2;
    cell_right->value.z1 = pointsPreOrdered.front()->z;
    cell_right->value.z2 = boundBox->value.z2;
    cell_right->final = true;

    boundBox->final = false;

    cellsPreorder.push_back(boundBox);
    cellsPreorder.push_back(cell_right);
    cellsPreorder.push_back(cell_left);

  }

  pointsPreOrdered.erase(pointsPreOrdered.begin());


  preorderGrid3D(cell_left, pointsPreOrdered_left);
  preorderGrid3D(cell_right, pointsPreOrdered_right);
  */
}

/*
  get cell id of a given cell and set it in the corresponding
  element in vector of nodes

*/
void
KDTree3D::setCellId3D(Cell3DTree cell, Tree3DMedStructure* kdNode)
{
  Tree3DMedStructure* node = new Tree3DMedStructure();

  while(kdNode != nullptr) {

    if(kdNode->axis == 0) {
    // check x1 value of cell
    if(cell.getValFromX() < kdNode->getVal()) {
      if(kdNode->left != nullptr) {
        kdNode = kdNode->left;
      } else {
        node->leaf = true;
        kdNode->left = node;
        kdNode->left->setCellId(cell.getCellId());
        return;
      }
    } else {
      if(kdNode->right != nullptr) {
        kdNode = kdNode->right;
      } else { 
        node->leaf = true;
        kdNode->right = node;
        kdNode->right->setCellId(cell.getCellId());
        return; }
    }
    } else if(kdNode->axis == 1) {
      // check y1 value of cell
      if(cell.getValFromY() < kdNode->getVal()) {
        if(kdNode->left != nullptr) {
          kdNode = kdNode->left;
        } else {
          node->leaf = true;
          kdNode->left = node;
          kdNode->left->setCellId(cell.getCellId());
          return;
        }
      } else {
        if(kdNode->right != nullptr) {
          kdNode = kdNode->right;
        } else { 
          node->leaf = true;
          kdNode->right = node;
          kdNode->right->setCellId(cell.getCellId());
          return; }
      }
    } else if(kdNode->axis == 2) {
      // check y1 value of cell
      if(cell.getValFromZ() < kdNode->getVal()) {
        if(kdNode->left != nullptr) {
          kdNode = kdNode->left;
        } else {
          node->leaf = true;
          kdNode->left = node;
          kdNode->left->setCellId(cell.getCellId());
          return;
        }
      } else {
        if(kdNode->right != nullptr) {
          kdNode = kdNode->right;
        } else { 
          node->leaf = true;
          kdNode->right = node;
          kdNode->right->setCellId(cell.getCellId());
          return; }
      }
    } else {
      return;
    }
    
  }

  kdNode->setCellId(cell.getCellId());
  return;
}


/*
  set cell id in corresponding element to given cell
  in vector of tree nodes

*/
void
KDTree3D::setCellId3D(Cell3DTree cell, Tree3DStructure* kdNode)
{

  while(kdNode != nullptr) {
    if(kdNode->isLeaf()) {
      break; 
    }

    if(kdNode->axis == 0) {
    // check x1 value of cell
    if(cell.getValFromX() < kdNode->x) {
      kdNode = kdNode->left;
    } else {
      kdNode = kdNode->right;
    }
    } else if(kdNode->axis == 1) { // check y-values
      if(cell.getValFromY() < kdNode->y) {
        kdNode = kdNode->left;
      } else {
        kdNode = kdNode->right;
      }
    } else if(kdNode->axis == 2) { // check z-values
      if(cell.getValFromZ() < kdNode->z) {
        kdNode = kdNode->left;
      } else {
        kdNode = kdNode->right;
      }
    } else {
      return;
    }

    
  }

  kdNode->setCellId(cell.getCellId());
  return;
}

void
KDTree3D::build3DTree() {

  // create grid structure
  Cell3DT* boundBox = new Cell3DT();
  boundBox->value.x1 = boundingBox->getMinX();
  boundBox->value.x2 = boundingBox->getMaxX();
  boundBox->value.y1 = boundingBox->getMinY();
  boundBox->value.y2 = boundingBox->getMaxY();
  boundBox->value.z1 = boundingBox->getMinZ();
  boundBox->value.z2 = boundingBox->getMaxZ();

  Tree3DStructure* root;
  Tree3DMedStructure* rootMed; 

  int mode_ = mode; // 1 => middle of list for partition, 2 => median 
  if(mode_ == 1) {

    // build 3dtree recursive, push elements in pointsVector
    KDTreeRec3D(points, 0, (int)points.size()-1);
    root = pointsVector.back();

    //order elements and prepare cells
    preorder3D(root);
    preorderGrid3D(boundBox, pointsPreorder[0]);
  } else {

    //printf("\n in else zu med");
    KDTreeMedRec3D(points);
    //printf("\n after kdtreemedrec");
    rootMed = kd3dmedListVec.back();
    //printf("\n after rootmed");
    // order elements and prepare cells
    preorder3DMed(rootMed);
    //printf("\n afer preorder3dmed");
    preorderMedGrid3D(boundBox, pointsPreorderMed[0]);
    //printf("\n after preordermedgrid3d");
  }

  Cell3DTree cell = Cell3DTree();

  // filter finale cells
  for(int r = 0; r < (int)cellsPreorder.size(); r++) {
    if(cellsPreorder.at(r)->final == 1) {
      cell.setValFromX(cellsPreorder.at(r)->value.x1);
      cell.setValToX(cellsPreorder.at(r)->value.x2);
      cell.setValFromY(cellsPreorder.at(r)->value.y1);
      cell.setValToY(cellsPreorder.at(r)->value.y2);
      cell.setValFromZ(cellsPreorder.at(r)->value.z1);
      cell.setValToZ(cellsPreorder.at(r)->value.z2);      

      cellVector.push_back(cell);
    }
  }
  
  // first sort by x-value then by y-value and z-value
  sort(cellVector.begin(), cellVector.end(), zValueComparison);
  sort(cellVector.begin(), cellVector.end(), xValueComparison);
  sort(cellVector.begin(), cellVector.end(), yValueComparison);

  int cellId = 0;
  
  // set cellIds of elements in cellVector
  for(int e = 0; e < (int)cellVector.size(); e++)
  {   
    cellId++;
    cellVector.at(e).setCellId(cellId);
    if(mode_ == 1) {
      setCellId3D(cellVector.at(e), root);
    } else {
      setCellId3D(cellVector.at(e), rootMed);
      //printf("\n after setcellid3d");
    }
  }

  /*for(int pri=0; pri < (int)cellVector.size(); pri++)
  {
    printf("\n %.2f %.2f %.2f %.2f %.2f %.2f %d", cellVector[pri].getValFromX(),
     cellVector[pri].getValToX(), cellVector[pri].getValFromY(),
    cellVector[pri].getValToY(), cellVector[pri].getValFromZ(),
     cellVector[pri].getValToZ(), cellVector[pri].getCellId());
  }
  printf("\n size Points %d size cells %d", (int)pointsPreorderMed.size(),
  (int)cellVector.size());*/
}

bool
KDTree3D::duplicateP(T3DPoint p)
{
  for(size_t i = 0; i < points.size(); i++)
  {
    if(points[i].x == p.x && points[i].y == p.y
      && points[i].z == p.z)
    {
      return true;
    }
  }
  return false;
}

void
KDTree3D::processInput(Stream<Rectangle<3>> rStream) {
  rStream.open();
  Rectangle<3>* next = rStream.request();

  while(next != 0){
    if (!insideRectangle3DTree(boundingBox, next)) {
      // rectangle (partially) outside the bounding box is discarded
      next = rStream.request();
      continue;
    }

    T3DPoint p = getCuboidCentre3DTree(next);
    if(!duplicateP(p)) {
      points.push_back(getCuboidCentre3DTree(next));
    }
    next = rStream.request();
  }

  rStream.close();
}

void
KDTree3D::processInputVector(std::vector<Rectangle<3>>* rVector) {
   for (Rectangle<3> bbox : *rVector) {
      if (!insideRectangle3DTree(boundingBox, &bbox)) {
         // rectangle (partially) outside the bounding box is discarded
         continue;
      }
      points.push_back(getCuboidCentre3DTree(&bbox));
   }
}


KDTree3D::~KDTree3D() {}

ListExpr
KDTree3D::Property3DTree()
{
    ListExpr desclst = nl->TextAtom();
    nl->AppendText(desclst,
    "A <rectangle> bounding box "
    "followed by list of lists of cell and cell id.\n"
    "A cell consists of a six-element list\n(<from x> "
    "<to x> <from y> <to y> <from z> <to z>). ");

  ListExpr formatlst = nl->TextAtom();
    nl->AppendText(formatlst,
    "(0.0 2.0 0.0 2.0 0.0 2.0) (((0.0 0.9 0.0 0.7 0.0 0.25) 1) "
    "((0.0 0.9 0.0 0.7 0.25 2.0) 2) ((0.9 2.0 0.0 1.5 0.0 0.75) 3) "
    " ((0.9 2.0 0.0 1.5 0.75 2.0) 4) ((0.0 0.9 0.7 2.0 0.0 2.0) 5)"
    " ((0.9 2.0 1.5 2.0 0.0 2.0) 6)))");

  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom(KDTree3D::BasicType()),
               desclst,
               formatlst)));
}

// Out function
ListExpr
KDTree3D::Out3DTree( ListExpr typeInfo, Word value ) {
  KDTree3D* kdtree3d = static_cast<KDTree3D*>( value.addr );
  if (kdtree3d != nullptr) {
    Rectangle<3> * b_box = kdtree3d->getBoundingBox();
    ListExpr bboxLstExpr = nl->SixElemList(
      nl->RealAtom(b_box->getMinX()),
      nl->RealAtom(b_box->getMaxX()),
      nl->RealAtom(b_box->getMinY()),
      nl->RealAtom(b_box->getMaxY()),
      nl->RealAtom(b_box->getMinZ()),
      nl->RealAtom(b_box->getMaxZ()));

    //ListExpr rowLstExpr = nl->Empty();
    ListExpr cellLstExpr = nl->Empty();

    
    std::vector<Cell3DTree>* cells = &kdtree3d->getCellVector();

        if (cells->size() > 0) {
          ListExpr lastCellLstExpr;
          for(size_t cellIdx = 0; cellIdx < cells->size(); cellIdx++) {
            Cell3DTree* curr_cell = &cells->at(cellIdx);

            if (cellIdx > 0) {
              lastCellLstExpr = nl->Append(lastCellLstExpr,
                  nl->TwoElemList(
                  nl->SixElemList(nl->RealAtom(curr_cell->getValFromX()),
                  nl->RealAtom(curr_cell->getValToX()),
                  nl->RealAtom(curr_cell->getValFromY()),
                  nl->RealAtom(curr_cell->getValToY()),
                  nl->RealAtom(curr_cell->getValFromZ()),
                  nl->RealAtom(curr_cell->getValToZ())),
                  nl->OneElemList(
                  nl->IntAtom(curr_cell->getCellId()))));
            } else {
              cellLstExpr = nl->OneElemList(nl->TwoElemList(
                  nl->SixElemList(nl->RealAtom(curr_cell->getValFromX()), 
                  nl->RealAtom(curr_cell->getValToX()),
                  nl->RealAtom(curr_cell->getValFromY()),
                  nl->RealAtom(curr_cell->getValToY()),
                  nl->RealAtom(curr_cell->getValFromZ()),
                  nl->RealAtom(curr_cell->getValToZ())),
                  nl->OneElemList(
                  nl->IntAtom(curr_cell->getCellId()))));
              lastCellLstExpr = cellLstExpr;
            }
          }
        }   
        
    
    ListExpr kdtree3dLstExpr = nl->TwoElemList(bboxLstExpr, cellLstExpr);
    return kdtree3dLstExpr;
  } else {
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
  }
}

// In function
Word
KDTree3D::In3DTree( const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct ) {

  Word w = SetWord(Address(0));
  try {
    Rectangle<3>* bbox;

    ListExpr bboxLstExpr;
    ListExpr cellLstExpr;

    if ( nl->ListLength( instance ) == 2 ) {
      bboxLstExpr = nl->First(instance);
      cellLstExpr = nl->Second(instance);
    } else {
      throw 1;
    }

    // fetch bounding box information from input
    if (nl->ListLength( bboxLstExpr ) == 6 ) {
      ListExpr left = nl->First(bboxLstExpr);
      ListExpr right = nl->Second(bboxLstExpr);
      ListExpr bottom = nl->Third(bboxLstExpr);
      ListExpr top = nl->Fourth(bboxLstExpr);
      ListExpr front = nl->Fifth(bboxLstExpr);
      ListExpr back = nl->Sixth(bboxLstExpr);

      if ( nl->IsAtom(left) && nl->AtomType(left) == RealType
          && nl->IsAtom(right) && nl->AtomType(right) == RealType
          && nl->IsAtom(bottom) && nl->AtomType(bottom) == RealType
          && nl->IsAtom(top) && nl->AtomType(top) == RealType
          && nl->IsAtom(front) && nl->AtomType(front) == RealType
          && nl->IsAtom(back) && nl->AtomType(back) == RealType) {

        double min[3], max[3];
        min[0] = nl->RealValue(left);
        min[1] = nl->RealValue(bottom);
        max[0] = nl->RealValue(right);
        max[1] = nl->RealValue(top);
        min[2] = nl->RealValue(front);
        max[2] = nl->RealValue(back);

        bbox = new Rectangle<3>(true, min, max);
      } else {
        throw 3;
      }

    } else {
      throw 2;
    }

    // temporary support structures
    std::map<int, int> cellRef;
    std::map<int, Cell3DTree*> cellIds;

    std::vector<Cell3DTree> cell_vec {};
        int cell_cnt = 0;
                        Cell3DTree c;

        //Cell3DTree* c_ptr;

    if (nl->ListLength( cellLstExpr ) > 1 ) {
      while(!nl->IsEmpty(cellLstExpr)) {
            ListExpr cellElem = nl->First(cellLstExpr);
    
            if(nl->ListLength(cellElem) == 2) {
              while(!nl->IsEmpty(cellElem)) {
              ListExpr lstElem = nl->First(cellElem);
                if (nl->ListLength( lstElem ) == 6) {
              // a six-element list initiates a new cell
              ListExpr cv1Lst = nl->First(lstElem);
              ListExpr cv2Lst = nl->Second(lstElem);
              ListExpr cv3Lst = nl->Third(lstElem);
              ListExpr cv4Lst = nl->Fourth(lstElem);
              ListExpr cv5Lst = nl->Fifth(lstElem);
              ListExpr cv6Lst = nl->Sixth(lstElem);
              //ListExpr cv7Lst = nl->Seventh(lstElem);

              cell_cnt++;

              if ( nl->IsAtom(cv1Lst) && nl->AtomType(cv1Lst) == RealType
                  && nl->IsAtom(cv2Lst) && nl->AtomType(cv2Lst) == RealType
                  && nl->IsAtom(cv3Lst) && nl->AtomType(cv3Lst) == RealType
                  && nl->IsAtom(cv4Lst) && nl->AtomType(cv4Lst) == RealType
                  && nl->IsAtom(cv5Lst) && nl->AtomType(cv5Lst) == RealType
                  && nl->IsAtom(cv6Lst) && nl->AtomType(cv6Lst) == RealType
                 ){
                c = Cell3DTree();
                c.setValFromX(nl->RealValue(cv1Lst));
                c.setValToX(nl->RealValue(cv2Lst));
                c.setValFromY(nl->RealValue(cv3Lst));
                c.setValToY(nl->RealValue(cv4Lst));
                c.setValFromZ(nl->RealValue(cv5Lst));
                c.setValToZ(nl->RealValue(cv6Lst));
                
                cell_vec.push_back(c);

              } else {
                throw 4;
              }
            } else { // only cellid
              if(nl->ListLength(lstElem) == 1) {
                ListExpr clLst1 = nl->First(lstElem);
                if(nl->IsAtom(clLst1) && nl->AtomType(clLst1) == IntType) {
                  Cell3DTree ce = cell_vec.back();
                  cell_vec.pop_back();
                  ce.setCellId(nl->IntValue(clLst1));
                  //c_ptr->setCellId(nl->IntValue(clLst1));
                  cell_vec.push_back(ce);
                  cellIds.insert(std::make_pair(
                  ((int)nl->IntValue(clLst1)), &ce));
                }  
              }
            }
            cellElem = nl->Rest(cellElem);

            }
            }
            cellLstExpr = nl->Rest(cellLstExpr);

          }
    }
    
    std::vector<Tree3DStructure*> points_vec {};

    correct = true;
    KDTree3D* kdtree = new KDTree3D(*bbox);
    kdtree->setPointsVector(points_vec);
    kdtree->setCellVector(cell_vec);

    w.addr = kdtree;
    return w;
  } catch (int e) {
    correct = false;
    cmsg.inFunError("Expecting a 3dtree list representation. Exit code "
        + std::to_string(e));

    return w;
  }
}

// This function checks whether the type constructor is applied correctly.
bool
KDTree3D::KindCheck3DTree( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, KDTree3D::BasicType() ));
}

// Close -function
void
KDTree3D::Close3DTree( const ListExpr typeInfo, Word& w )
{
  delete (KDTree3D *)w.addr;
  w.addr = 0;
}

// Clone function
Word
KDTree3D::Clone3DTree( const ListExpr typeInfo, const Word& w )
{
  KDTree3D *g = new KDTree3D( *((KDTree3D *)w.addr) );
  return SetWord( g );
}

// Create function
Word
KDTree3D::Create3DTree( const ListExpr typeInfo )
{
  return SetWord( new KDTree3D() );
}

// Delete function
void
KDTree3D::Delete3DTree( const ListExpr typeInfo, Word& w )
{
  delete (KDTree3D *)w.addr;
  w.addr = 0;
}

// SizeOf function
int
KDTree3D::SizeOf3DTree()
{
  return sizeof(KDTree3D);
}

ListExpr
KDTree3D::KdTree3dFeedTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 1)) {

    ListExpr first = nl->First(args);
    if (KDTree3D::checkType(first)) {
      ListExpr resAttrList = nl->TwoElemList(
          nl->TwoElemList(
            nl->SymbolAtom("Id"),
            nl->SymbolAtom(CcInt::BasicType())),
          nl->TwoElemList(
            nl->SymbolAtom("Cell3DTree"),
            nl->SymbolAtom(Rectangle<3>::BasicType())));

      return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple>>(),
        nl->TwoElemList(
          listutils::basicSymbol<Tuple>(),
          resAttrList));

      }
  }
  const std::string errMsg = "The following argument is expected:"
      " 3dtree";

  return  listutils::typeError(errMsg);
}

std::vector<CellInfo3DTree*>
KDTree3D::getCellInfoVector(KDTree3D *in_kdtree3d) {
  std::vector<CellInfo3DTree*> cell_info_vect {};

  std::vector<Cell3DTree>* cells = &in_kdtree3d->getCellVector();
  for(size_t cellIdx = 0; cellIdx < cells->size(); cellIdx++) {
    Cell3DTree* cell = &cells->at(cellIdx);
    double cfx = cell->getValFromX();
    double ctx = cell->getValToX();
    double cfy = cell->getValFromY();
    double cty = cell->getValToY();
    double cfz = cell->getValFromZ();
    double ctz = cell->getValToZ();
    int cid = cell->getCellId();

    CellInfo3DTree* ci = new CellInfo3DTree(cid, cfx, ctx, cfy, cty, cfz, ctz);
    cell_info_vect.push_back(ci);
  
  }

  return cell_info_vect;
}


// for value mapping function of ~feed~ operator
struct KDTRee3DTupleInfo
{
  std::vector<CellInfo3DTree*> cell_info_vect;
  unsigned int currentTupleIdx;
  ListExpr numTupleTypeList;

  void init(KDTree3D *kdtree3d_in) {
    currentTupleIdx = 0;
    cell_info_vect = KDTree3D::getCellInfoVector(kdtree3d_in);

    ListExpr tupleTypeLst = nl->TwoElemList(
      nl->SymbolAtom(Tuple::BasicType()),
      nl->TwoElemList(
        nl->TwoElemList(
          nl->SymbolAtom("Id"),
          nl->SymbolAtom(CcInt::BasicType())),
        nl->TwoElemList(
          nl->SymbolAtom("Cell3DTree"),
          nl->SymbolAtom(Rectangle<3>::BasicType()))));

    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    numTupleTypeList = sc->NumericType(tupleTypeLst);
  }

  TupleType* getTupleType() {
    TupleType *tupleType = new TupleType(numTupleTypeList);

    return tupleType;
  }

  Tuple* getNext(TupleType *ttype) {
    if (currentTupleIdx < cell_info_vect.size()) {

      CellInfo3DTree * cell_info = cell_info_vect.at(currentTupleIdx);
      int tp_p1 = cell_info->cellId;
      Rectangle<3> tp_p2 = *cell_info->cell;

      Tuple *tuple = new Tuple(ttype);
      tuple->PutAttribute(0, new CcInt(true, tp_p1));
      tuple->PutAttribute(1, new Rectangle<3> (tp_p2));

      currentTupleIdx++;
      return tuple;
    } else {
      return nullptr;
    }
  }
};

/*
Value mapping function of operator ~feed~

*/
int
KDTree3D::KdTree3dValueMapFeed( Word* args, Word& result, int message,
                        Word& local, Supplier s ) {
  KDTree3D *input_kdtree3d_ptr =
    static_cast<KDTree3D*>( args[0].addr );
  KDTRee3DTupleInfo* tp_info = static_cast<KDTRee3DTupleInfo*>(local.addr);
  TupleType* tupleType = nullptr;
  Tuple* tuple = nullptr;

  switch (message) {
    case OPEN: {
      tp_info = new KDTRee3DTupleInfo();
      tp_info->init(input_kdtree3d_ptr);
      local.addr = tp_info;
      return 0;
    }
    case REQUEST: {
      if (local.addr) {
        tp_info = ((KDTRee3DTupleInfo*)local.addr);
        tupleType = tp_info->getTupleType();
      } else {
        return CANCEL;
      }
      // get next tuple
      tuple = tp_info->getNext(tupleType);

      if (tuple != nullptr) {
          result.addr = tuple;
          return YIELD;
      } else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (local.addr) {

        tp_info = ((KDTRee3DTupleInfo*)local.addr);
        delete tp_info;
        local.addr = 0;
      }
      return 0;
    }
  }

  return -1;
}

/*
Type mapping function ~KDTree3dCellnosTypeMap~

It is used for the ~cellnos\_kd~ operator.

*/
ListExpr
KDTree3D::Kdtree3dCellnosTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 2)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if (KDTree3D::checkType(first) && Rectangle<3>::checkType(second)) {
      return nl->SymbolAtom(collection::IntSet::BasicType());
    }
  }

  const std::string errMsg = "The following two arguments are expected:"
      " 3dtree x rect<3>";

  return  listutils::typeError(errMsg);
}

ListExpr
KDTree3D::Kdtree3dTRCTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 3)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (KDTree3D::checkType(first) && Rectangle<3>::checkType(second)
      && Rectangle<3>::checkType(third)) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }

  const std::string errMsg = "The following three arguments are expected:"
      " 3dtree x rect<3> x rect<3>";

  return  listutils::typeError(errMsg);
}

ListExpr
KDTree3D::Kdtree3dTRCCellIdTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 3)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (KDTree3D::checkType(first) && Rectangle<3>::checkType(second)
      && Rectangle<3>::checkType(third)) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }

  const std::string errMsg = "The following three arguments are expected:"
      " 3dtree x rect<3> x rect<3>";

  return  listutils::typeError(errMsg);
}

ListExpr
KDTree3D::Kdtree3dSCCTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 3)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (KDTree3D::checkType(first) && Rectangle<3>::checkType(second)
      && Rectangle<3>::checkType(third)) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }

  const std::string errMsg = "The following three arguments are expected:"
      " 3dtree x rect<3> x rect<3>";

  return  listutils::typeError(errMsg);
}

bool
InCell(Cell3DTree cell, double valy, double valx, double valz) {
  return (valx >= cell.getValFromX()
    && valx < cell.getValToX()
    && valy >= cell.getValFromY()
    && valy < cell.getValToY()
    && valz >= cell.getValFromZ()
    && valz < cell.getValToZ());
}

//template <class C>
int
CellBS(std::vector<Cell3DTree>* c_vec, int start, int end,
 const double valy, const double valx, const double valz) {
  if (start >= end) {
    return -1;
  }
  const int mid = start;

  if (InCell(c_vec->at(mid), valy, valx, valz)) {
    return mid;
  }
    return CellBS(c_vec, mid+1, end, valy, valx, valz);

}

void
GetLeaf(Tree3DMedStructure* node, double le, double ri,
 double bo, double to, double fr, double ba, std::set<int> *cell_ids) 
{
  // reached the end of the branch: cell id is in leaf
  if(node->isLeaf()) {
    cell_ids->insert(node->getCellId());
    return;
    }

  if(node->getAxis() == 0) {
    // check x1 value
    if(le < node->getVal() && node->getLeft() != nullptr) {
      GetLeaf(node->getLeft(), le, ri, bo, to, fr, ba, cell_ids);
    } else {
      if(node->getRight() != nullptr) {
      GetLeaf(node->getRight(), le, ri, bo, to, fr, ba, cell_ids);
      }
    }
    // check x2 value
    if(ri < node->getVal() && node->getLeft() != nullptr) {
      GetLeaf(node->getLeft(), le, ri, bo, to, fr, ba, cell_ids);
    } else {
      if(node->getRight() != nullptr) {
      GetLeaf(node->getRight(), le, ri, bo, to, fr, ba, cell_ids);
      }
    }
    
  }
  if (node->getAxis() == 1) {
    // check y1 value
    if(bo < node->getVal() && node->getLeft() != nullptr) {
      GetLeaf(node->getLeft(), le, ri, bo, to, fr, ba, cell_ids);
    } else {
      if(node->getRight() != nullptr) {
      GetLeaf(node->getRight(), le, ri, bo, to, fr, ba, cell_ids);
      }
    }
    // check y2 value
    if(to < node->getVal() && node->getLeft() != nullptr) {
      GetLeaf(node->getLeft(), le, ri,  bo, to, fr, ba, cell_ids);
    } else {
      if(node->getRight() != nullptr) {
      GetLeaf(node->getRight(), le, ri, bo, to, fr, ba, cell_ids);
      }
    }
  }
  if (node->getAxis() == 2) {
    // check z1 value
    if(fr < node->getVal() && node->getLeft() != nullptr) {
      GetLeaf(node->getLeft(), le, ri, bo, to, fr, ba, cell_ids);
    } else {
      if(node->getRight() != nullptr) {
      GetLeaf(node->getRight(), le, ri, bo, to, fr, ba, cell_ids);
      }
    }
    // check z2 value
    if(ba < node->getVal() && node->getLeft() != nullptr) {
      GetLeaf(node->getLeft(), le, ri,  bo, to, fr, ba, cell_ids);
    } else {
      if(node->getRight() != nullptr) {
      GetLeaf(node->getRight(), le, ri, bo, to, fr, ba, cell_ids);
      }
    }
  } 
  return;
}

void
GetLeaf(Tree3DStructure* node, double le, double ri, 
double bo, double to, double fr, double ba,
 std::set<int> *cell_ids) 
{
  // reached the end of the branch: cell id is in leaf
  if(node->isLeaf()) {
    cell_ids->insert(node->getCellId());
    return;
    }

  if(node->getAxis() == 0) {
    // check x1 value
    if(le < node->getValx() && node->getLeft() != nullptr) {
      GetLeaf(node->getLeft(), le, ri, bo, to, fr, ba, cell_ids);
    } else {
      if(node->getRight() != nullptr) {
      GetLeaf(node->getRight(), le, ri, bo, to, fr, ba, cell_ids);
      }
    }
    // check x2 value
    if(ri < node->getValx() && node->getLeft() != nullptr) {
      GetLeaf(node->getLeft(), le, ri, bo, to, fr, ba, cell_ids);
    } else {
      if(node->getRight() != nullptr) {
      GetLeaf(node->getRight(), le, ri, bo, to, fr, ba, cell_ids);
      }
    }
    
  }
  if (node->getAxis() == 1) {
    // check y1 value
    if(bo < node->getValy() && node->getLeft() != nullptr) {
      GetLeaf(node->getLeft(), le, ri, bo, to, fr, ba, cell_ids);
    } else {
      if(node->getRight() != nullptr) {
      GetLeaf(node->getRight(), le, ri, bo, to, fr, ba, cell_ids);
      }
    }
    // check y2 value
    if(to < node->getValy() && node->getLeft() != nullptr) {
      GetLeaf(node->getLeft(), le, ri,  bo, to, fr, ba, cell_ids);
    } else {
      if(node->getRight() != nullptr) {
      GetLeaf(node->getRight(), le, ri, bo, to, fr, ba, cell_ids);
      }
    }
  }
  if (node->getAxis() == 2) {
    // check z1 value
    if(fr < node->getValz() && node->getLeft() != nullptr) {
      GetLeaf(node->getLeft(), le, ri, bo, to, fr, ba, cell_ids);
    } else {
      if(node->getRight() != nullptr) {
      GetLeaf(node->getRight(), le, ri, bo, to, fr, ba, cell_ids);
      }
    }
    // check z2 value
    if(ba < node->getValz() && node->getLeft() != nullptr) {
      GetLeaf(node->getLeft(), le, ri,  bo, to, fr, ba, cell_ids);
    } else {
      if(node->getRight() != nullptr) {
      GetLeaf(node->getRight(), le, ri, bo, to, fr, ba, cell_ids);
      }
    }
  } 
  return;
}

void
cellnumber(double le, double ri, 
  double bo, double to, double fr, double ba,
  std::set<int> *cell_ids,
  std::vector<Cell3DTree>* cells)
{ 
  for(size_t ce=0; ce < cells->size(); ce++)
  {
    Cell3DTree* curr_cell = &cells->at(ce);
    if(le >= curr_cell->getValFromX() && le <= curr_cell->getValToX())
    {
      if((bo >= curr_cell->getValFromY() && bo <= curr_cell->getValToY())
      || (to >= curr_cell->getValFromY() && to <=curr_cell->getValToY()))
      {
        if((fr >= curr_cell->getValFromZ() && fr <= curr_cell->getValToZ())
        || (ba >= curr_cell->getValFromZ() && ba <= curr_cell->getValToZ()))
        {
          cell_ids->insert(curr_cell->getCellId());
        }
      }
    } else if(ri >= curr_cell->getValFromX() && ri <= curr_cell->getValToX())
    {
      if((bo >= curr_cell->getValFromY() && bo <= curr_cell->getValToY())
      || (to >= curr_cell->getValFromY() && to <=curr_cell->getValToY()))
      {
        if((fr >= curr_cell->getValFromZ() && fr <= curr_cell->getValToZ())
        || (ba >= curr_cell->getValFromZ() && ba <= curr_cell->getValToZ()))
        {
          cell_ids->insert(curr_cell->getCellId());
        }
      }
    }
  }

  return;

}

/*
Value mapping function of operator ~cellnos\_ir~

*/
int
KDTree3D::Kdtree3dValueMapCellnos( Word* args, Word& result,
   int message, Word& local, Supplier s ) {
  KDTree3D *input_kdtree3d_ptr
    = static_cast<KDTree3D*>( args[0].addr );

  Rectangle<3> *search_window_ptr
    = static_cast<Rectangle<3>*>( args[1].addr );

  if (input_kdtree3d_ptr != nullptr && search_window_ptr != nullptr) {
    std::set<int> cell_ids;

    result = qp->ResultStorage(s);
    collection::IntSet* res = (collection::IntSet*) result.addr;

    Rectangle<3> * b_box = input_kdtree3d_ptr->getBoundingBox();
    int mode_ = input_kdtree3d_ptr->getMode();
    
    if (!search_window_ptr->Intersects(*b_box)) {
      cell_ids.insert(0);
      res->setTo(cell_ids);
      return 0;
    }

    // 'truncate' search window in case of partial cutting
    if (!b_box->Contains(*search_window_ptr)) {
      search_window_ptr = new Rectangle<3>(
        search_window_ptr->Intersection(*b_box));

      cell_ids.insert(0);
    }

    double le = search_window_ptr->getMinX();
    double ri = search_window_ptr->getMaxX();
    double bo = search_window_ptr->getMinY();
    double to = search_window_ptr->getMaxY();
    double fr = search_window_ptr->getMinZ();
    double ba = search_window_ptr->getMaxZ();

    // mode not set
    if(mode_ != 1 && mode_ != 2) {
      // use different cellnum method!
      cell_ids.clear();
      std::vector<Cell3DTree>* cells = &input_kdtree3d_ptr->getCellVector();    
      cellnumber(le, ri, bo, to, fr, ba, &cell_ids, cells);
      //cell_ids.insert(0);
      res->setTo(cell_ids);
      return 0;
    }

    // find leaf with cellIds recursive and insert cell ids in cell_ids
    if(mode_ == 1) {
      std::vector<Tree3DStructure*> nodes = 
        input_kdtree3d_ptr->getPointsVector();
      Tree3DStructure* root = nodes.back();
      GetLeaf(root, le, ri, bo, to, fr, ba, &cell_ids);

    } else {
      std::vector<Tree3DMedStructure*> nodes = 
        input_kdtree3d_ptr->getPointsMedVector();
      Tree3DMedStructure* root = nodes.back();
      GetLeaf(root, le, ri, bo, to, fr, ba, &cell_ids);

    }


    res->setTo(cell_ids);

    return 0;
    
    }
  
  return -1;
}

/*
  Value mapping function for operator ~trccell\_3d~

*/
int
KDTree3D::Kdtree3dValueMapTRCCellId( Word* args, Word& result, int message,
    Word& local, Supplier s ) {
  KDTree3D *input_kdtree3d_ptr
    = static_cast<KDTree3D*>( args[0].addr );

  Rectangle<3> *search_window_ptr
    = static_cast<Rectangle<3>*>( args[1].addr );

  Rectangle<3> *search_window_ptr_2
    = static_cast<Rectangle<3>*>( args[2].addr );

  if (input_kdtree3d_ptr != nullptr && search_window_ptr != nullptr
      && search_window_ptr_2 != nullptr) {
    std::set<int> cell_ids;
    std::set<int> cell_ids_2;


    result = qp->ResultStorage(s);
    CcInt* res = (CcInt*) result.addr;

    std::vector<Tree3DStructure*> nodes = input_kdtree3d_ptr->getPointsVector();

    double le = search_window_ptr->getMinX();
    double ri = search_window_ptr->getMaxX();
    double bo = search_window_ptr->getMinY();
    double to = search_window_ptr->getMaxY();
    double fr = search_window_ptr->getMinZ();
    double ba = search_window_ptr->getMaxZ();

    double le_2 = search_window_ptr_2->getMinX();
    double ri_2 = search_window_ptr_2->getMaxX();
    double bo_2 = search_window_ptr_2->getMinY();
    double to_2 = search_window_ptr_2->getMaxY();
    double fr_2 = search_window_ptr_2->getMinZ();
    double ba_2 = search_window_ptr_2->getMaxZ();

    Tree3DStructure* root = nodes.back();
    GetLeaf(root, le, ri, bo, to, fr, ba, &cell_ids);
    GetLeaf(root, le_2, ri_2, bo_2, to_2, fr_2, ba_2, &cell_ids_2);


    std::vector<int> v(sizeof(cell_ids)+ sizeof(cell_ids_2));
      std::vector<int>::iterator it;

      it=std::set_intersection (cell_ids.begin(), cell_ids.end(),
         cell_ids_2.begin(), cell_ids_2.end(), v.begin());
      v.resize(it-v.begin());                      
  
      if(v.empty()) { 
      //no intersection between rectangles
        res->Set(0);
        return -1;

      }

      std::vector<Cell3DTree>* cells = &input_kdtree3d_ptr->getCellVector();    


      for (it=v.begin(); it!=v.end(); ++it) {
      for(size_t c = 0; c < cells->size(); c++) {
      if(cells->at(c).getCellId() == *it) {
        // get TRC Values
        int value_rect1 = 0;
        int value_rect2 = 0;

        // create bbox of kdtree cell
        Cell3DTree* tmp = &cells->at(c);

        //Rectangle<2> bbox =  bbox(tmp);
        if ( search_window_ptr->getMaxX() >= tmp->getValToX() )
          {value_rect1++;}
        if ( search_window_ptr->getMaxY() >= tmp->getValToY() ) {
          value_rect1 += 2;}
        

        if ( search_window_ptr_2->getMaxX() >= tmp->getValToX() )
          {value_rect2++;}
        if ( search_window_ptr_2->getMaxY() >= tmp->getValToY() ) {
          value_rect2 += 2;}
        
        int value = value_rect1 & value_rect2;
        if (value == 0)
        {
          res->Set( *it );
          return 0;
        }
      }
      }
      }
      }

      return 0;
  }


/*
  value mapping function for operator ~trc\_3d~

*/
int
KDTree3D::Kdtree3dValueMapTRC( Word* args, Word& result, int message,
    Word& local, Supplier s ) {

  Rectangle<3> *search_window_ptr
    = static_cast<Rectangle<3>*>( args[1].addr );

  Rectangle<3> *search_window_ptr_2
    = static_cast<Rectangle<3>*>( args[2].addr );

  if (search_window_ptr != nullptr
      && search_window_ptr_2 != nullptr) {


    result = qp->ResultStorage(s);
    CcInt* res = (CcInt*) result.addr;

   
        // get TRC Values
        int value_rect1 = 0;
  
        if ( search_window_ptr->getMaxX() >= search_window_ptr_2->getMaxX() )
          {value_rect1++;}
        if ( search_window_ptr->getMaxY() >= search_window_ptr_2->getMaxY() ) {
          value_rect1 += 2;}

        res->Set(value_rect1);
      }


      return 0;
  }


  /*
    Value mapping function for operator ~scc\_3d~

  */
  int
  KDTree3D::Kdtree3dValueMapSCC( Word* args, Word& result, int message,
    Word& local, Supplier s ) {
  KDTree3D *input_kdtree3d_ptr
    = static_cast<KDTree3D*>( args[0].addr );

  Rectangle<3> *search_window_ptr
    = static_cast<Rectangle<3>*>( args[1].addr );

  Rectangle<3> *search_window_ptr_2
    = static_cast<Rectangle<3>*>( args[2].addr );

  if (input_kdtree3d_ptr != nullptr && search_window_ptr != nullptr
      && search_window_ptr_2 != nullptr) {
    std::set<int> cell_ids;
    std::set<int> cell_ids_2;


    result = qp->ResultStorage(s);
    CcInt* res = (CcInt*) result.addr;

    std::vector<Tree3DStructure*> nodes = input_kdtree3d_ptr->getPointsVector();

    double le = search_window_ptr->getMinX();
    double ri = search_window_ptr->getMaxX();
    double bo = search_window_ptr->getMinY();
    double to = search_window_ptr->getMaxY();
    double fr = search_window_ptr->getMinZ();
    double ba = search_window_ptr->getMaxZ();

    double le_2 = search_window_ptr_2->getMinX();
    double ri_2 = search_window_ptr_2->getMaxX();
    double bo_2 = search_window_ptr_2->getMinY();
    double to_2 = search_window_ptr_2->getMaxY();
    double fr_2 = search_window_ptr_2->getMinZ();
    double ba_2 = search_window_ptr_2->getMaxZ();

    Tree3DStructure* root = nodes.back();
    GetLeaf(root, le, ri, bo, to, fr, ba, &cell_ids);
    GetLeaf(root, le_2, ri_2, bo_2, to_2, fr_2, ba_2, &cell_ids_2);


    std::vector<int> v(sizeof(cell_ids)+ sizeof(cell_ids_2));
      std::vector<int>::iterator it;

      it=std::set_intersection (cell_ids.begin(), cell_ids.end(),
         cell_ids_2.begin(), cell_ids_2.end(), v.begin());
      v.resize(it-v.begin());                      
  
      if(v.empty()) { 
      //no intersection between rectangles
        res->Set(0);
        return -1;

      }

      res->Set(v.at(0));

      }
      
      return 0;
  }