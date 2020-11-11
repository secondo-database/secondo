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


#include "IrregularGrid3D.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Collection/IntSet.h"
#include <map>
#include <iterator>

/*
  Class Cell3D

*/

Cell3D::Cell3D() {
  cellId = -1;
  valFrom = -1;
  valTo = -1;
  nbrPoints = -1;
}

void
Cell3D::setCellId(int cell_id) {
  this->cellId = cell_id;
}

int
Cell3D::getCellId() {
  return cellId;
}

void
Cell3D::setValFrom(double val_from) {
  valFrom = val_from;
}

double
Cell3D::getValFrom() {
  return valFrom;
}

void
Cell3D::setValTo(double val_to) {
  valTo = val_to;
}

double
Cell3D::getValTo() {
  return valTo;
}

int
Cell3D::getNbrOfPoints() {
  return nbrPoints;
}

void
Cell3D::setNbrOfPoints(int nbr_points) {
  nbrPoints = nbr_points;
}

Cell3D::~Cell3D() { }

/*
  Class VCell3D

*/

VCell3D::VCell3D() { }

std::vector<HCell3D>&
VCell3D::getRow() {
  return row;
}

VCell3D::~VCell3D() { }

/*
  Class SCell3D

*/
SCell::SCell() {
  upper = nullptr;
  neighbor = nullptr;

}

std::vector<SCell>&
HCell3D::getRect() {
  return rect;
}

void
SCell::setUpper(SCell* upper_) {
  upper = upper_;
}

SCell*
SCell::getUpper() {
  return upper;
}

void
SCell::setNeighbor(SCell* neighbor_) {
  neighbor = neighbor_;
}

SCell*
SCell::getNeighbor() {
  return neighbor;
}

SCell::~SCell() { }

/*
  Class HCell3D

*/
HCell3D::HCell3D() { }
HCell3D::~HCell3D() { }


/*
  Class IrregularGrid3D

*/
IrregularGrid3D::IrregularGrid3D() {
  boundingBox = nullptr;
  rowCount = 0;
  cellCount = 0;
  layerCount = 0; //
}

IrregularGrid3D::IrregularGrid3D(const IrregularGrid3D& g) {
  boundingBox = g.boundingBox;
  rowCount = g.rowCount;
  cellCount = g.cellCount;
  layerCount = g.layerCount;
}

IrregularGrid3D::IrregularGrid3D(Rectangle<3> &bounding_box,
    int row_count, int cell_count, int layer_count) {
  boundingBox = &bounding_box;
  rowCount = row_count;
  cellCount = cell_count;
  layerCount = layer_count;
}

void
IrregularGrid3D::Set(Stream<Rectangle<3>> rStream,
    Rectangle<3> &bounding_box,
    int row_count, int cell_count, int layer_count) {

  boundingBox = &bounding_box;
  rowCount = row_count;
  cellCount = cell_count;
  layerCount = layer_count;

  createIrgrid3D(rStream);
}

void
IrregularGrid3D::SetVector(std::vector<Rectangle<3>>* rVector,
                     Rectangle<3> &bounding_box,
                     int row_count, int cell_count, int layer_count) {
   boundingBox = &bounding_box;
   rowCount = row_count;
   cellCount = cell_count;
   layerCount = layer_count;

   createIrgrid3DVector(rVector);
}

std::vector<CellInfo3D*>
IrregularGrid3D::getCellInfoVector(IrregularGrid3D *in_irgrid3d) {
  std::vector<CellInfo3D*> cell_info_vect {};

  /* 
    Divide space vertical => horizontal => spatial
    SCells are final cells

  */

  std::vector<VCell3D>* col = &in_irgrid3d->getColumnVector();
  for(size_t colIdx = 0; colIdx < col->size(); colIdx++) {
    VCell3D* vcell = &col->at(colIdx);
    double vf = vcell->getValFrom();
    double vt = vcell->getValTo();

    std::vector<HCell3D>* row_vect = &vcell->getRow();

    for(size_t cellIdx = 0; cellIdx < row_vect->size(); cellIdx++) {
      HCell3D* cell = &row_vect->at(cellIdx);
      int cid = cell->getCellId();
      double hf = cell->getValFrom();
      double ht = cell->getValTo();

      std::vector<SCell>* rect_vect = &cell->getRect();

      for(size_t rectIdx = 0; rectIdx < rect_vect->size(); rectIdx++) {
        SCell* rect = &rect_vect->at(rectIdx);
        int scid = rect->getCellId();
        int nps = rect->getNbrOfPoints();
        double sf = rect->getValFrom();
        double st = rect->getValTo();

      

      CellInfo3D* ci = new CellInfo3D(scid, cid, nps, hf, ht, vf, vt, sf, st);
      cell_info_vect.push_back(ci);

      }
    }
  }

  return cell_info_vect;
}

void
IrregularGrid3D::createIrgrid3D(Stream<Rectangle<3>> rStream) {
  // sort input points by y-coordinates
  processInput(rStream);
  // create irregular grid 3d by point density
  buildGrid();

}

void
IrregularGrid3D::createIrgrid3DVector(std::vector<Rectangle<3>>* rVector) {
   // sort input points by y-coordinates
   processInputVector(rVector);
   // create irregular grid 3d by point density
   buildGrid();
}

bool
pointComparisonX(CPoint p1, CPoint p2) {
  return p1.x < p2.x;
}

bool
pointComparisonZ(CPoint p1, CPoint p2) {
  return p1.z < p2.z;
}

void
IrregularGrid3D::buildGrid() {

  // create grid structure
  double bb_top = boundingBox->getMaxY();
  double bb_bot = boundingBox->getMinY();
  double colOffset = (bb_top - bb_bot) / rowCount;

  double bb_right = boundingBox->getMaxX();
  double bb_left = boundingBox->getMinX();
  double hOffset = (bb_right - bb_left) / cellCount;

  double bb_bck = boundingBox->getMaxZ();
  double bb_frnt = boundingBox->getMinZ();
  double sOffset = (bb_bck - bb_frnt) / layerCount;

  // check if bounding box is big enough
  /*std::sort(points.begin(), points.end(), pointComparisonX);
  double minX = points[0].x;
  double maxX = points[points.size()-1].x;
  std::sort(points.begin(), points.end(), pointComparisonY);
  double minY = points[0].y;
  double maxY = points[points.size()-1].y;
  std::sort(points.begin(), points.end(), pointComparisonZ);
  double minZ = points[0].z;
  double maxZ = points[points.size()-1].z;

  // adjust boundaries of bbox
  if(minX < bb_left ){
    bb_left = minX-1;
  } else if(maxX > bb_right) {
    bb_right = maxX+1;
  } else if(minY < bb_bot) {
    bb_bot = minY-1;
  } else if(maxY > bb_top) {
    bb_top = maxY+1;
  } else if(minZ < bb_bck) {
    bb_bck = minZ-1;
  } else if(maxZ > bb_frnt) {
    bb_frnt = maxZ+1;
  }*/

  double col_boundary_val = bb_bot;
  int hcell_id = 1;
  int scell_id = 1;


  // first divide space by rows
  for(int c = 0; c < rowCount; c++) {
    VCell3D vcell = VCell3D();
    vcell.setValFrom(col_boundary_val);

    col_boundary_val += colOffset;
    vcell.setValTo(col_boundary_val);

    double cell_boundary_val = bb_left;
    // divide rows in cells
    for(int r = 0; r < cellCount; r++) {
      HCell3D hcell = HCell3D();

      hcell.setValFrom(cell_boundary_val);

      cell_boundary_val += hOffset;
      hcell.setValTo(cell_boundary_val);

      hcell.cellId = hcell_id;
      hcell_id++;
      // hcell.upper will be determined later

      double rect_boundary_val = bb_frnt;
      // divide cells in layers => result: final cells
      for (int i = 0; i < layerCount; i++) {
        SCell scell = SCell();
        scell.setValFrom(rect_boundary_val);

        rect_boundary_val += sOffset;
        scell.setValTo(rect_boundary_val);
        scell.cellId = scell_id;
        scell_id++;

        // will be determined later
        scell.upper = nullptr;
        scell.neighbor = nullptr;

        hcell.getRect().push_back(scell);
      }
      

      cellVector.push_back(hcell);
      vcell.getRow().push_back(hcell);

    }
    columnVector.push_back(vcell);
  }

  // adjust boundaries by point distribution
  int nbrOfPoints = points.size();
  int pointsPerRow = nbrOfPoints / rowCount;
  std::vector<CPoint> tmp_row_points {};
  std::vector<CPoint> tmp_row_points_hcell {};
  std::vector<CPoint> tmp_row_points_scell {};



  int pointIdx = 0;
  int point_counter = 0;
  for(int colIdx = 0; colIdx < rowCount; colIdx++) { //proof rows
    for(size_t dp_idx = pointIdx; dp_idx < points.size(); dp_idx++) {
      CPoint rp = points[dp_idx];
      tmp_row_points.push_back(rp);
      point_counter ++;

      if ((point_counter == pointsPerRow) || (dp_idx == points.size()-1)) {
        // adjust y-boundaries
        if (colIdx > 0) {
          
          getColumnVector()[colIdx].setValFrom(
            getColumnVector()[colIdx-1].getValTo());
        }

        double new_val_to_y;
        if (dp_idx == points.size()-1) {
          new_val_to_y = bb_top;
        } else {
          new_val_to_y = rp.y;

        }

        getColumnVector()[colIdx].setValTo(new_val_to_y);
        getColumnVector()[colIdx].setNbrOfPoints(point_counter);

        point_counter = 0;

        // adjust x-boundaries
        std::sort(tmp_row_points.begin(), tmp_row_points.end(),
          pointComparisonX);

        int pointsPerCell = tmp_row_points.size() / cellCount;

        int tmpPointIdx = 0;
        for(int h = 0; h < cellCount; h++) {
          std::vector<HCell3D>* hcel_vec_ptr = &columnVector[colIdx].getRow();

          for(size_t tmp_rp_idx = tmpPointIdx;
              tmp_rp_idx < tmp_row_points.size(); tmp_rp_idx++) {

            CPoint rp_t = tmp_row_points[tmp_rp_idx];
            tmp_row_points_hcell.push_back(rp_t);
            point_counter ++;

            if((point_counter == pointsPerCell)
              || (tmp_rp_idx == tmp_row_points.size()-1)
              /* || (h == cellCount-1 ) */) {

              double new_val_to_x;
              if ((tmp_rp_idx == tmp_row_points.size())
                  || (h == cellCount-1)) {
                new_val_to_x = bb_right;
              } else {
                 new_val_to_x = rp_t.x;

              }

              hcel_vec_ptr->at(h).setValTo(new_val_to_x);
              getCellVector()[h].setValTo(new_val_to_x);
              getCellVector()[h].setNbrOfPoints(point_counter);

              if(h < cellCount-1) {
                hcel_vec_ptr->at(h+1).setValFrom(new_val_to_x);
                getCellVector()[h+1].setValFrom(new_val_to_x);
              }

              point_counter = 0;

              //adjust z-boundaries
              std::sort(tmp_row_points_hcell.begin(), 
              tmp_row_points_hcell.end(), pointComparisonZ);

              int pointsPerRect = tmp_row_points_hcell.size() / layerCount;
              int tmpPointIdz = 0;
              for(int r = 0; r < layerCount; r++) {
              std::vector<SCell>* scel_vec_ptr = &cellVector[h].getRect();

                for(size_t tmp_rp_idz = tmpPointIdz;
                  tmp_rp_idz < tmp_row_points_hcell.size(); tmp_rp_idz++) {

                  CPoint rp_tt = tmp_row_points_hcell[tmp_rp_idz];
                  tmp_row_points_scell.push_back(rp_tt);
                  point_counter++;

                  if((point_counter == pointsPerRect)
                    || (tmp_rp_idz == tmp_row_points_scell.size()-1)) {

                    if (r > 0) {
                      scel_vec_ptr->at(r).setValFrom(
                        scel_vec_ptr->at(r-1).getValTo());
                      
                    }

                  double new_val_to_z;
                  if ((tmp_rp_idz == tmp_row_points_scell.size()-1)
                    || (r == layerCount-1)) {
                    new_val_to_z = bb_bck;
                  } else {
                    new_val_to_z = rp_t.z;
                  }
                  scel_vec_ptr->at(r).setValTo(new_val_to_z);

                  tmpPointIdz = tmp_rp_idz + 1;
                  scel_vec_ptr->at(r).setNbrOfPoints(point_counter);

                  tmp_row_points_scell.clear();
                  // Smoothing in case of odd number of points per rect
                  if (r+1 < layerCount) {
                    pointsPerCell = (tmp_row_points_hcell.size()-tmpPointIdz)
                    / (layerCount-1-r);
                  }

                  point_counter = 0;
                  // next rect
                  break;

                  }
                  
                }
              }
              tmp_row_points_hcell.clear();
              tmpPointIdx = tmp_rp_idx + 1;
              hcel_vec_ptr->at(h).setNbrOfPoints(point_counter);

              // Smoothing in case of odd number of points per cell
              if (h+1 < cellCount) {
                pointsPerCell = (tmp_row_points.size()-tmpPointIdx)
                  / (cellCount-1-h);
              }
              point_counter = 0;


              // next cell
              break;
            }
          }
        }

        tmp_row_points.clear();
        pointIdx = dp_idx + 1;

        // Smoothing in case of odd number of points per row
        if (colIdx+1 < rowCount) {
          pointsPerRow = (points.size()-pointIdx)
            / (rowCount-1-colIdx);
        }

        point_counter = 0;

        // one row up
        break;
      }
    }
  }

  // clear aux. vectors
  if (points.size() > 0) {
    points.clear();
  }
  
  if (tmp_row_points.size() > 0) {
    tmp_row_points.clear();
  }
  if(tmp_row_points_hcell.size() > 0) {
    tmp_row_points_hcell.clear();
  }
  if(tmp_row_points_scell.size() > 0) {
    tmp_row_points_scell.clear();
  }

  std::vector<SCell>* row_lower = nullptr;
  std::vector<SCell>* row_upper = nullptr;
  std::vector<SCell>* row_neighbor = nullptr;

  

  // update cell pointer
  if(rowCount > 0 && cellCount > 0 && layerCount > 0)
  {
    for(int c = 0; c < rowCount; c++) {
      for(int h = 0; h < cellCount; h++) {
      
        int pointToCellIdx = 0;
        int pointToCellIdz = 0;


        // choose exact cell above current cell in z-layer
        for(int s = 0; s < layerCount; s++) {

          if(c < rowCount-1) {

          row_lower = &getColumnVector().at(c).getRow().at(h).getRect();
          row_upper = &getColumnVector().at(c+1).getRow().at(h).getRect();
          SCell* lower_cell_ptr = &row_lower->at(s);
          SCell* upper_cell_ptr = &row_upper->at(pointToCellIdx);


          if(lower_cell_ptr->getValFrom() < upper_cell_ptr->getValTo()) {
            lower_cell_ptr->setUpper(upper_cell_ptr);

          } else {
            SCell* next_upper_cell_ptr;
            do {
              pointToCellIdx ++;
              next_upper_cell_ptr = &row_upper->at(pointToCellIdx);
              lower_cell_ptr->setUpper(next_upper_cell_ptr);

            } while (lower_cell_ptr->getValFrom() 
                >= next_upper_cell_ptr->getValTo());
          }

          }

          /* Neighbor of lower_cell_ptr
            compare z-values

          */
          if(h < cellCount-1) {
            row_lower = &getColumnVector().at(c).getRow().at(h).getRect();
            row_neighbor = &getColumnVector().at(c).getRow().at(h+1).getRect();
            SCell* lower_cell_ptr = &row_lower->at(s);

          SCell* neighbor_cell_ptr = &row_neighbor->at(pointToCellIdz);
          if(lower_cell_ptr->getValFrom() < neighbor_cell_ptr->getValTo()) {
            lower_cell_ptr->setNeighbor(neighbor_cell_ptr);

          } else {
            SCell* next_neighbor_cell_ptr;
            do {
              pointToCellIdz++;
              next_neighbor_cell_ptr = &row_neighbor->at(pointToCellIdz);
              lower_cell_ptr->setNeighbor(next_neighbor_cell_ptr);

            } while(lower_cell_ptr->getValFrom() 
              >= next_neighbor_cell_ptr->getValTo());

          }
          }
        }
      }  
    }
  }
}

bool
pointComparisonY(CPoint p1, CPoint p2) {
  return p1.y < p2.y;
}

/*
  Returns centre of cuboid

*/
CPoint
getCuboidCentre(Rectangle<3>* r) {
  double a = (r->getMaxY() - r->getMinY()) / (double)2;
  double b = (r->getMaxX() - r->getMinX()) / (double)2;
  double c = (r->getMaxZ() - r->getMinZ()) / (double)2;

  CPoint r_c { (r->getMinX())+b, (r->getMinY())+a, (r->getMinZ()+c) };

  return r_c;
}

/*
  Check if a rectangle <3> is inside the irgrid3d bounding box

*/
bool
insideBoundingBox(Rectangle<3>* bbox, Rectangle<3>* r) {
  double le = bbox->getMinX();
  double ri = bbox->getMaxX();
  double bo = bbox->getMinY();
  double to = bbox->getMaxY();
  double fr = bbox->getMinZ();
  double bk = bbox->getMaxZ();

  if (r->getMinX() >= le && r->getMaxX() <= ri
      && r->getMinY() >= bo && r->getMaxY() <=to
      && r->getMinZ() >= fr && r->getMaxZ() <= bk) {                        

    return true;
  }

  return false;
}

void
IrregularGrid3D::processInput(Stream<Rectangle<3>> rStream) {
  rStream.open();
  Rectangle<3>* next = rStream.request();

  while(next != 0){
    if (!insideBoundingBox(boundingBox, next)) {
      // rectangle (partially) outside the bounding box is discarded
      next = rStream.request();
      continue;
    }
    points.push_back(getCuboidCentre(next));
    next = rStream.request();
  }
  rStream.close();

  // sort point vector by y-coordinates
  std::sort(points.begin(), points.end(), pointComparisonY);
}

void
IrregularGrid3D::processInputVector(std::vector<Rectangle<3>>* rVector) {
   for (Rectangle<3> bbox : *rVector) {
      if (!insideBoundingBox(boundingBox, &bbox)) {
         // rectangle (partially) outside the bounding box is discarded
         continue;
      }
      points.push_back(getCuboidCentre(&bbox));
   }

   // sort point vector by y-coordinates
   std::sort(points.begin(), points.end(), pointComparisonY);
}

Rectangle<3> *
IrregularGrid3D::getBoundingBox() {
  return boundingBox;
}

int
IrregularGrid3D::getRowCount() {
  return rowCount;
}

int
IrregularGrid3D::getCellCount() {
  return cellCount;
}

int
IrregularGrid3D::getLayerCount() {
  return layerCount;
}

void
IrregularGrid3D::setColumnVector(std::vector<VCell3D> column_vect) {
  this->columnVector = column_vect;
}

void
IrregularGrid3D::setCellVector(std::vector<HCell3D> cell_vect) {
  this->cellVector = cell_vect;
}

std::vector<VCell3D>&
IrregularGrid3D::getColumnVector() {
  return this->columnVector;
}

std::vector<HCell3D>&
IrregularGrid3D::getCellVector() {
  return this->cellVector;
}

IrregularGrid3D::~IrregularGrid3D() {}

ListExpr
IrregularGrid3D::PropertyIrGrid3D()
{
    ListExpr desclst = nl->TextAtom();
    nl->AppendText(desclst,
    "A <rectangle<3>> bounding box followed by list of rows.\n"
    "A row consists of two-element list (<from> <to>) "
    "followed by list of cells.\n"
    "A cell consists of a" 
    "three-element list\n(<from> <to> <id>) "
    "followed by a list of layers.\n"
    "A layer consists of a four-element list\n(<from> "
    "<to> <id> <ref_id> <ref2_id>.");

  ListExpr formatlst = nl->TextAtom();
    nl->AppendText(formatlst,
    "(0.0 2.0 0.0 2.0 0.0 2.0) ((0.0 0.3) ((0.0 0.25 1) "
    "((0.0 1.0 1 5 3) (1.0 2.0 2 6 4)) "
    " (0.25 2.0 2) ((0.0 1.0 3 7 -1) (1.0 2.0 4 8 -1)))"
    "(0.3 2.0) ((0.0 0.375 3) ((0.0 1.0 5 -1 7) "
    "(1.0 2.0 6 -1 8)) (0.375 2.0 4) "
    " ((0.0 1.0 7 -1 -1) (1.0 2.0 8 -1 -1))))");

  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom(IrregularGrid3D::BasicType()),
               desclst,
               formatlst)));
}


// Out function
ListExpr
IrregularGrid3D::OutIrGrid3D( ListExpr typeInfo, Word value ) {
  IrregularGrid3D* irgrid3d = static_cast<IrregularGrid3D*>( value.addr );
  if (irgrid3d != nullptr) {
    Rectangle<3> * b_box = irgrid3d->getBoundingBox();
    ListExpr bboxLstExpr = nl->SixElemList(
      nl->RealAtom(b_box->getMinX()),
      nl->RealAtom(b_box->getMaxX()),
      nl->RealAtom(b_box->getMinY()),
      nl->RealAtom(b_box->getMaxY()),
      nl->RealAtom(b_box->getMinZ()),
      nl->RealAtom(b_box->getMaxZ()));

    std::vector<VCell3D>* col = &irgrid3d->getColumnVector();

    ListExpr rowLstExpr = nl->Empty();
    ListExpr lastRowLstExpr;
    if (col->size() > 0) {
      for(size_t colIdx = 0; colIdx < col->size(); colIdx++) {
        VCell3D* vcell = &col->at(colIdx);
        if (colIdx > 0) {
            lastRowLstExpr = nl->Append(lastRowLstExpr,
              nl->TwoElemList(nl->RealAtom(vcell->getValFrom()),
              nl->RealAtom(vcell->getValTo())));
        } else {
          rowLstExpr =  nl->OneElemList(
            nl->TwoElemList(nl->RealAtom(vcell->getValFrom()),
            nl->RealAtom(vcell->getValTo())));
          lastRowLstExpr = rowLstExpr;
        }

        std::vector<HCell3D>* row_vect = &col->at(colIdx).getRow();
        if (row_vect->size() > 0) {
          ListExpr cellLstExpr;
          ListExpr lastCellLstExpr;
          for(size_t rowIdx = 0; rowIdx < row_vect->size(); rowIdx++) {
            HCell3D* row_cell = &row_vect->at(rowIdx);

            if (rowIdx > 0) {
              lastCellLstExpr = nl->Append(lastCellLstExpr,
                  nl->ThreeElemList(nl->RealAtom(row_cell->getValFrom()),
                  nl->RealAtom(row_cell->getValTo()),
                  nl->IntAtom(row_cell->getCellId())));
            } else {
              cellLstExpr = nl->OneElemList(nl->ThreeElemList
                  (nl->RealAtom(row_cell->getValFrom()),
                  nl->RealAtom(row_cell->getValTo()),
                  nl->IntAtom(row_cell->getCellId())));
              lastCellLstExpr = cellLstExpr;
            }

            std::vector<SCell>* cell_vect = &row_vect->at(rowIdx).getRect();
            if(cell_vect->size() > 0) {
              ListExpr layerLstExpr;
              ListExpr lastLayerLstExpr;
              for(size_t cellIdx = 0; cellIdx < cell_vect->size(); cellIdx++) {
                SCell* cell_layer = &cell_vect->at(cellIdx);

                if(cellIdx > 0) {
                  lastLayerLstExpr = nl->Append(lastLayerLstExpr,
                  nl->FiveElemList(nl->RealAtom(cell_layer->getValFrom()),
                  nl->RealAtom(cell_layer->getValTo()),
                  nl->IntAtom(cell_layer->getCellId()),
                  nl->IntAtom(cell_layer->getUpper() != nullptr
                    ? cell_layer->getUpper()->getCellId() : -1),
                  nl->IntAtom(cell_layer->getNeighbor() != nullptr
                    ? cell_layer->getNeighbor()->getCellId() : -1)
                  ));
                } else {
                  layerLstExpr = nl->OneElemList(nl->FiveElemList
                  (nl->RealAtom(cell_layer->getValFrom()),
                  nl->RealAtom(cell_layer->getValTo()),
                  nl->IntAtom(cell_layer->getCellId()),
                  nl->IntAtom(cell_layer->getUpper() != nullptr
                    ? cell_layer->getUpper()->getCellId() : -1),
                  nl->IntAtom(cell_layer->getNeighbor() != nullptr
                    ? cell_layer->getNeighbor()->getCellId() : -1)
                  ));
                  lastLayerLstExpr = layerLstExpr;
                }
              }
              lastCellLstExpr = nl->Append(lastCellLstExpr,layerLstExpr);
            }
          }
          lastRowLstExpr = nl->Append(lastRowLstExpr, cellLstExpr);
        }
      }
    }
    ListExpr irgrid3dLstExpr = nl->TwoElemList(bboxLstExpr, rowLstExpr);
    return irgrid3dLstExpr;
  } else {
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
  }
}


// In function
Word
IrregularGrid3D::InIrGrid3D( const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct ) {

  Word w = SetWord(Address(0));
  try {
    Rectangle<3>* bbox;

    ListExpr bboxLstExpr;
    ListExpr rowLstExpr;

    if ( nl->ListLength( instance ) == 2 ) {
      bboxLstExpr = nl->First(instance);
      rowLstExpr = nl->Second(instance);
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

    // fetch row information from input

    // temporary support structures
    std::map<int, int> cellRef;
    std::map<int, int> NeighbRef;
    std::map<int, SCell*> cellIds;

    std::vector<VCell3D> column_vec {};
    std::vector<HCell3D> cell_vec {};
    int row_cnt = 0;
    int cell_cnt = 0;
    int layer_cnt = 0;
    if (nl->ListLength( rowLstExpr ) > 1 ) {
      VCell3D vc;
      HCell3D hc;
      VCell3D* vc_ptr;
      HCell3D* hc_ptr;

      while(!nl->IsEmpty(rowLstExpr)) {
        ListExpr lstElem = nl->First(rowLstExpr);

        if ((nl->ListLength(lstElem)) == 2
            && (nl->IsAtom(nl->First(lstElem))
            && nl->IsAtom(nl->Second(lstElem))) ) {
          // a two-element double list initiates a new row
          row_cnt ++;
          ListExpr fLst = nl->First(lstElem);
          ListExpr tLst = nl->Second(lstElem);

          if ( nl->AtomType(fLst) == RealType
                && nl->AtomType(tLst) == RealType) {
            vc = VCell3D();

            vc.setValFrom(nl->RealValue(fLst));
            vc.setValTo(nl->RealValue(tLst));
            column_vec.push_back(vc);
            vc_ptr = &(column_vec.back());
          }
        } else {
          ListExpr rowLstExpr = lstElem;

          while(!nl->IsEmpty(rowLstExpr)) {
            // cell information
            ListExpr cellLstExpr = nl->First(rowLstExpr);
            if (nl->ListLength( cellLstExpr ) == 3) {
              // a three-element list initiates a new rectangle
              ListExpr cv1Lst = nl->First(cellLstExpr);
              ListExpr cv2Lst = nl->Second(cellLstExpr);
              ListExpr cv3Lst = nl->Third(cellLstExpr);

              if ( nl->IsAtom(cv1Lst) && nl->AtomType(cv1Lst) == RealType
                  && nl->IsAtom(cv2Lst) && nl->AtomType(cv2Lst) == RealType
                  && nl->IsAtom(cv3Lst) && nl->AtomType(cv3Lst) == IntType
                  ) {
                hc = HCell3D();
                hc.setValFrom(nl->RealValue(cv1Lst));
                hc.setValTo(nl->RealValue(cv2Lst));
                hc.setCellId(nl->IntValue(cv3Lst));
                // will be determined later
                //hc->setUpper(nullptr);

                vc_ptr->getRow().push_back(hc);
                cell_vec.push_back(hc);
                hc_ptr = &(vc_ptr->getRow().back());
                //hc_ptr = &(cell_vec.back());
              } else {
                throw 5;
              }
            } else { 
              //ListExpr rowLstExpr = cellLstExpr;
              while(!nl->IsEmpty(cellLstExpr)) {
                ListExpr layerLstExpr = nl->First(cellLstExpr);
                if(nl->ListLength( layerLstExpr ) == 5) {
                  // a five-element list initiates a new layer/ the whole cell
                  ListExpr lv1Lst = nl->First(layerLstExpr);
                  ListExpr lv2Lst = nl->Second(layerLstExpr);
                  ListExpr lv3Lst = nl->Third(layerLstExpr);
                  ListExpr lv4Lst = nl->Fourth(layerLstExpr);
                  ListExpr lv5Lst = nl->Fifth(layerLstExpr);

                  if(nl->IsAtom(lv1Lst) && nl->AtomType(lv1Lst) == RealType
                  && nl->IsAtom(lv2Lst) && nl->AtomType(lv2Lst) == RealType
                  && nl->IsAtom(lv3Lst) && nl->AtomType(lv3Lst) == IntType
                  && nl->IsAtom(lv4Lst) && nl->AtomType(lv4Lst) == IntType
                  && nl->IsAtom(lv5Lst) && nl->AtomType(lv5Lst) == IntType) {
                    SCell* sc = new SCell();
                    sc->setValFrom(nl->RealValue(lv1Lst));
                    sc->setValTo(nl->RealValue(lv2Lst));
                    sc->setCellId(nl->IntValue(lv3Lst));
                    sc->setUpper(nullptr);
                    sc->setNeighbor(nullptr);
                    hc_ptr->getRect().push_back(*sc);

                    int cellRefId = nl->IntValue(lv4Lst);
                    if (cellRefId != -1) {
                      cellRef.insert(std::make_pair(
                      ((int)nl->IntValue(lv3Lst)), cellRefId));
                    }
                    cellIds.insert(std::make_pair(
                      ((int)nl->IntValue(lv3Lst)), sc));

                    int NeighbRefId = nl->IntValue(lv5Lst);
                    if(NeighbRefId != -1) {
                      NeighbRef.insert(std::make_pair(
                        ((int)nl->IntValue(lv3Lst)), NeighbRefId));
                    }
                    cellIds.insert(std::make_pair(
                      ((int)nl->IntValue(lv3Lst)), sc));
                  }
                }
                cellLstExpr = nl->Rest(cellLstExpr);
              }
            }
            if(cell_cnt > 0 && layer_cnt == 0) {
                layer_cnt = hc_ptr->getRect().size();
            }
            rowLstExpr = nl->Rest(rowLstExpr);
          }
        } 
        // determine number of cells (one-time)
        if (row_cnt > 0 && cell_cnt == 0) {
          cell_cnt = vc_ptr->getRow().size();
          
        }
        rowLstExpr = nl->Rest(rowLstExpr);
      }

    } else {
      throw 4;
    }

    // update pointer
    if (row_cnt > 0 && cell_cnt > 0 && layer_cnt > 0) {
      for(int colIdx = 0; colIdx < row_cnt-1; colIdx++) {
        VCell3D* vcell = &column_vec.at(colIdx);
        std::vector<HCell3D>* row_vect = &vcell->getRow();
        for(int cIdx = 0; cIdx < cell_cnt; cIdx++) {
          HCell3D* hcell = &(*row_vect).at(cIdx);
          std::vector<SCell>* cell_vect = &hcell->getRect();

          for(int lIdx = 0; lIdx < layer_cnt; lIdx++) {
            SCell* scell = &(*cell_vect).at(lIdx);
            if(cellRef.find(scell->getCellId()) != cellRef.end()) {
              int cell_ref = cellRef.at(scell->getCellId());
              if(cellIds.find(cell_ref) != cellIds.end()) {
                scell->setUpper(cellIds.at(cell_ref));
              }
            }
            if(NeighbRef.find(scell->getCellId()) != NeighbRef.end()) {
              int neighb_ref = NeighbRef.at(scell->getCellId());
              if(cellIds.find(neighb_ref) != cellIds.end()) {
                scell->setNeighbor(cellIds.at(neighb_ref));
              }
            }
          }
        }
      }
    }

    correct = true;
    IrregularGrid3D* irgrid = new IrregularGrid3D(*bbox,
       row_cnt, cell_cnt, layer_cnt);
    irgrid->setColumnVector(column_vec);
    irgrid->setCellVector(cell_vec);

    w.addr = irgrid;
    return w;
  } catch (int e) {
    correct = false;
    cmsg.inFunError("Expecting a irgrid3d list representation. Exit code "
        + std::to_string(e));

    return w;
  }
}

// This function checks whether the type constructor is applied correctly.
bool
IrregularGrid3D::KindCheckIrGrid3D( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, IrregularGrid3D::BasicType() ));
}

// Close -function
void
IrregularGrid3D::CloseIrGrid3D( const ListExpr typeInfo, Word& w )
{
  delete (IrregularGrid3D *)w.addr;
  w.addr = 0;
}

// Clone function
Word
IrregularGrid3D::CloneIrGrid3D( const ListExpr typeInfo, const Word& w )
{
  IrregularGrid3D *g = new IrregularGrid3D( *((IrregularGrid3D *)w.addr) );
  return SetWord( g );
}

// Create function
Word
IrregularGrid3D::CreateIrGrid3D( const ListExpr typeInfo )
{
  return SetWord( new IrregularGrid3D() );
}

// Delete function
void
IrregularGrid3D::DeleteIrGrid3D( const ListExpr typeInfo, Word& w )
{
  delete (IrregularGrid3D *)w.addr;
  w.addr = 0;
}

// SizeOf function
int
IrregularGrid3D::SizeOfIrGrid3D()
{
  return sizeof(IrregularGrid3D);
}

/*
Type mapping function ~IrGrid3dFeedTypeMap~

It is used for the ~feed~ operator.

*/
ListExpr
IrregularGrid3D::IrGrid3dFeedTypeMap( ListExpr args )
{

  if(nl->HasLength(args, 1)) {

    ListExpr first = nl->First(args);
    if (IrregularGrid3D::checkType(first)) {
      ListExpr resAttrList = nl->ThreeElemList(
          nl->TwoElemList(
            nl->SymbolAtom("Id"),
            nl->SymbolAtom(CcInt::BasicType())),
          nl->TwoElemList(
            nl->SymbolAtom("Count"),
            nl->SymbolAtom(CcInt::BasicType())),
          nl->TwoElemList(
            nl->SymbolAtom("Cell3D"),
            nl->SymbolAtom(Rectangle<3>::BasicType())));

      return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple>>(),
        nl->TwoElemList(
          listutils::basicSymbol<Tuple>(),
          resAttrList));

      }
  }
  const std::string errMsg = "The following argument is expected:"
      " irgrid3d";

  return  listutils::typeError(errMsg);
}

// for value mapping function of ~feed~ operator
struct IrGrid3DTupleInfo
{
  std::vector<CellInfo3D*> cell_info_vect;
  unsigned int currentTupleIdx;
  ListExpr numTupleTypeList;

  void init(IrregularGrid3D *irgrid3d_in) {
    currentTupleIdx = 0;
    cell_info_vect = IrregularGrid3D::getCellInfoVector(irgrid3d_in);

    ListExpr tupleTypeLst = nl->TwoElemList(
      nl->SymbolAtom(Tuple::BasicType()),
      nl->ThreeElemList(
        nl->TwoElemList(
          nl->SymbolAtom("Id"),
          nl->SymbolAtom(CcInt::BasicType())),
        nl->TwoElemList(
          nl->SymbolAtom("Count"),
          nl->SymbolAtom(CcInt::BasicType())),
        nl->TwoElemList(
          nl->SymbolAtom("Cell3D"),
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

      CellInfo3D * cell_info = cell_info_vect.at(currentTupleIdx);
      int tp_p1 = cell_info->cellId;
      int tp_p2 = cell_info->statNbrOfPoints;
      Rectangle<3> tp_p3 = *cell_info->cell;

      Tuple *tuple = new Tuple(ttype);
      tuple->PutAttribute(0, new CcInt(true, tp_p1));
      tuple->PutAttribute(1, new CcInt(true, tp_p2));
      tuple->PutAttribute(2, new Rectangle<3> (tp_p3));

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
IrregularGrid3D::IrGrid3dValueMapFeed( Word* args, Word& result, int message,
                        Word& local, Supplier s ) {
  IrregularGrid3D *input_irgrid3d_ptr =
    static_cast<IrregularGrid3D*>( args[0].addr );
  IrGrid3DTupleInfo* tp_info = static_cast<IrGrid3DTupleInfo*>(local.addr);
  TupleType* tupleType = nullptr;
  Tuple* tuple = nullptr;

  switch (message) {
    case OPEN: {
      tp_info = new IrGrid3DTupleInfo();
      tp_info->init(input_irgrid3d_ptr);
      local.addr = tp_info;
      return 0;
    }
    case REQUEST: {
      if (local.addr) {
        tp_info = ((IrGrid3DTupleInfo*)local.addr);
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

        tp_info = ((IrGrid3DTupleInfo*)local.addr);
        delete tp_info;
        local.addr = 0;
      }
      return 0;
    }
  }

  return -1;
}

/*
Type mapping function ~IrGrid3dCellnosTypeMap~

It is used for the ~cellnos\_ir~ operator.

*/
ListExpr
IrregularGrid3D::IrGrid3dCellnosTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 2)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if (IrregularGrid3D::checkType(first) && Rectangle<3>::checkType(second)) {
      return nl->SymbolAtom(collection::IntSet::BasicType());
    }
  }

  const std::string errMsg = "The following two arguments are expected:"
      " irgrid3d x rect<3>";

  return  listutils::typeError(errMsg);
}

ListExpr
IrregularGrid3D::IrGrid3dTRCTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 3)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (IrregularGrid3D::checkType(first) && Rectangle<3>::checkType(second)
        && Rectangle<3>::checkType(third)) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }

  const std::string errMsg = "The following three arguments are expected:"
      " irgrid3d x rect3d x rect3";

  return  listutils::typeError(errMsg);
}

ListExpr
IrregularGrid3D::IrGrid3dTRCCellIdTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 3)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (IrregularGrid3D::checkType(first) && Rectangle<3>::checkType(second)
        && Rectangle<3>::checkType(third)) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }

  const std::string errMsg = "The following three arguments are expected:"
      " irgrid3d x rect3d x rect3";

  return  listutils::typeError(errMsg);
}

ListExpr
IrregularGrid3D::IrGrid3dSCCTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 3)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);

    if (IrregularGrid3D::checkType(first) && Rectangle<3>::checkType(second)
        && Rectangle<3>::checkType(third)) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }

  const std::string errMsg = "The following three arguments are expected:"
      " irgrid3d x rect3d x rect3";

  return  listutils::typeError(errMsg);
}

ListExpr
IrregularGrid3D::IrGrid3dBBoxTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 1)) {
    ListExpr first = nl->First(args);

    if (Stream<Rectangle<3>>::checkType(first)) {
      return nl->SymbolAtom(Rectangle<3>::BasicType());
    }
  }

  const std::string errMsg = "The following argument is expected:"
      " stream<rect<3>>";

  return  listutils::typeError(errMsg);
}

ListExpr
IrregularGrid3D::IrGrid2dBBoxTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 1)) {
    ListExpr first = nl->First(args);
    
    if (Stream<Rectangle<2>>::checkType(first)) {
      return nl->SymbolAtom(Rectangle<2>::BasicType());
    }
  }

  const std::string errMsg = "The following argument is expected:"
      " stream<rect>";

  return  listutils::typeError(errMsg);
}

template <class C>
bool
InCell(C cell, double val) {
  return (val >= cell.getValFrom()
    && val < cell.getValTo());
}

template <class C>
bool
GtCell(C cell, double val) {
  return (cell.getValFrom() >= val);
}

template <class C>
int
CellBS(const std::vector<C>* c_vec, int start, int end, const double val) {
  if (start > end) {
    return -1;
  }
  const int mid = start + ((end - start) / 2);
  if (InCell(c_vec->at(mid), val)) {
    return mid;
  } else if (GtCell(c_vec->at(mid), val)) {
    return CellBS(c_vec, start, mid-1, val);
  }

  return CellBS(c_vec, mid+1, end, val);
}

/*
  returns cellnumbers of cells which a given
  cuboid intersects

*/
void 
cellNum(IrregularGrid3D *input_irgrid3d_ptr,
  Rectangle<3> *search_window_ptr, std::set<int> *cell_ids){
  std::vector<VCell3D>* col = &input_irgrid3d_ptr->getColumnVector();

    double le = search_window_ptr->getMinX();
    double ri = search_window_ptr->getMaxX();
    double bo = search_window_ptr->getMinY();
    double to = search_window_ptr->getMaxY();
    double fr = search_window_ptr->getMinZ();
    double ba = search_window_ptr->getMaxZ();

    // check for 3d
    if(le == ri || bo == to || fr == ba)
    {
      printf("\n le: %.2f, ri: %.2f, bo: %.2f, to: %.2f, fr: %2.f, ba: %2.f", 
      le, ri, bo, to, fr, ba);

      cell_ids->insert(0);
      return; 
    }

    int pos_bo = CellBS(col, 0, col->size(), bo);
    if (pos_bo != -1) {
      VCell3D vCell = col->at(pos_bo);
      std::vector<HCell3D>* row = &vCell.getRow();

      int pos_le = CellBS(row, 0, row->size(), le);
      if (pos_le != -1) {
        HCell3D hcell = row->at(pos_le);
        std::vector<SCell>* cells = &hcell.getRect();

        int pos_fr = CellBS(cells, 0, cells->size(), fr);
        if(pos_fr != -1) {

          // collect ids
          unsigned int cellIdx = pos_fr;
          int pos_left = pos_le;
          SCell begin_i = cells->at(cellIdx); // first cell
          while (cellIdx < cells->size()) {
            SCell i = cells->at(cellIdx);
            cell_ids->insert(i.getCellId());


            if((ba >= i.getValFrom() && ba <= i.getValTo())
              || (cellIdx == cells->size()-1 && ba >= i.getValFrom())) {
                SCell fi = cells->at(pos_fr);
                if(ri >= hcell.getValTo() && fi.getNeighbor() != nullptr) {
                  hcell = row->at(++pos_le);
                  cells = &hcell.getRect();

                  SCell* u = fi.getNeighbor();
                  int nbr_cpr = input_irgrid3d_ptr->getLayerCount();
                  int cid_pos = (u->getCellId()) % nbr_cpr;
                  pos_fr = cid_pos == 0 ? nbr_cpr-1 : cid_pos-1;

                  cellIdx = pos_fr-1;


                } else if (ri <= hcell.getValTo() || 
                  fi.getNeighbor() == nullptr) {
                  if (to >= vCell.getValTo() && begin_i.getUpper() != nullptr) {
                    SCell* up = begin_i.getUpper();
                    int nbr_cpr = input_irgrid3d_ptr->getLayerCount();
                    int cid_pos = (up->getCellId()) % nbr_cpr;
                    pos_fr = cid_pos == 0 ? nbr_cpr-1 : cid_pos-1;

                    cellIdx = pos_fr-1;
                    row = &col->at(++pos_bo).getRow();  // one row up
                    hcell = row->at(pos_fr);
                    cells = &hcell.getRect();
                    begin_i = cells->at(pos_fr);
                    // in case there is another neighbor in the new row
                    pos_le = pos_left; 

                } else if (to <= vCell.getValTo() && 
                      begin_i.getUpper() == nullptr) {
                  break;
                } else { break; }
                } 
              }
              cellIdx++;
          }

        }
      }
    }
}

/*
Value mapping function of operator ~cellnos\_ir~

*/
int
IrregularGrid3D::IrGrid3dValueMapCellnos( Word* args, Word& result, int message,
    Word& local, Supplier s ) {
  IrregularGrid3D *input_irgrid3d_ptr
    = static_cast<IrregularGrid3D*>( args[0].addr );

  Rectangle<3> *search_window_ptr
    = static_cast<Rectangle<3>*>( args[1].addr );

  if (input_irgrid3d_ptr != nullptr && search_window_ptr != nullptr) {
    std::set<int> cell_ids;

    result = qp->ResultStorage(s);
    collection::IntSet* res = (collection::IntSet*) result.addr;

    Rectangle<3> * b_box = input_irgrid3d_ptr->getBoundingBox();
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

    cellNum(input_irgrid3d_ptr, search_window_ptr, &cell_ids);

    res->setTo(cell_ids);

    return 0;
    
  }
  return -1;
}

/*
  Value mapping function of ~trccell\_3d~

*/
int
IrregularGrid3D::IrGrid3dValueMapTRCCellId( Word* args, Word& result,
   int message, Word& local, Supplier s ) {
  IrregularGrid3D *input_irgrid3d_ptr
    = static_cast<IrregularGrid3D*>( args[0].addr );

  Rectangle<3> *search_window_ptr
    = static_cast<Rectangle<3>*>( args[1].addr );

  Rectangle<3> *search_window_ptr_2
    = static_cast<Rectangle<3>*>( args[2].addr );

  if (input_irgrid3d_ptr != nullptr && search_window_ptr != nullptr
      && search_window_ptr_2 != nullptr) {
    std::set<int> cell_ids;
    std::set<int> cell_ids_2;

    

    result = qp->ResultStorage(s);
    CcInt* res = (CcInt*) result.addr;

    cellNum(input_irgrid3d_ptr, search_window_ptr, &cell_ids);
    cellNum(input_irgrid3d_ptr, search_window_ptr_2, &cell_ids_2);

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

    std::vector<VCell3D>* col = &input_irgrid3d_ptr->getColumnVector();

    for (it=v.begin(); it!=v.end(); ++it) {
      for(size_t c = 0; c < col->size(); c++) {
        std::vector<HCell3D>* row_vect = &col->at(c).getRow();
          for(size_t rowIdx = 0; rowIdx < row_vect->size(); rowIdx++) {
            std::vector<SCell>* cell_vect = &row_vect->at(rowIdx).getRect();
              for(size_t cellIdx = 0; cellIdx < cell_vect->size(); cellIdx++) {
                if(cell_vect->at(cellIdx).getCellId() == *it) {

                // get TRC Values
                int value_rect1 = 0;
                int value_rect2 = 0;

                if ( search_window_ptr->getMaxX() 
                  >= row_vect->at(rowIdx).getValTo() )
                  {value_rect1++;}
                if ( search_window_ptr->getMaxY() >= col->at(c).getValTo() ) {
                  value_rect1 += 2;}
        

                if ( search_window_ptr_2->getMaxX() 
                  >= row_vect->at(rowIdx).getValTo() )
                  {value_rect2++;}
                if ( search_window_ptr_2->getMaxY() >= col->at(c).getValTo() ) {
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
    }
  }
  return -1;
}


/*
  Value mapping function of operator ~trc\_3d~

*/
int
IrregularGrid3D::IrGrid3dValueMapTRC( Word* args, Word& result, int message,
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
      { value_rect1++;}
    if ( search_window_ptr->getMaxY() >= search_window_ptr_2->getMaxY() ) {
      value_rect1 += 2;}

    res->Set(value_rect1);
    return 0;
      
  }
  return -1;
}

/*
  Value mapping function of operator ~scc\_3d~

*/
int
IrregularGrid3D::IrGrid3dValueMapSCC( Word* args, Word& result, int message,
    Word& local, Supplier s ) {
  IrregularGrid3D *input_irgrid3d_ptr
    = static_cast<IrregularGrid3D*>( args[0].addr );

  Rectangle<3> *search_window_ptr
    = static_cast<Rectangle<3>*>( args[1].addr );

  Rectangle<3> *search_window_ptr_2
    = static_cast<Rectangle<3>*>( args[2].addr );

  if (input_irgrid3d_ptr != nullptr && search_window_ptr != nullptr
      && search_window_ptr_2 != nullptr) {
    std::set<int> cell_ids;
    std::set<int> cell_ids_2;

    result = qp->ResultStorage(s);
    CcInt* res = (CcInt*) result.addr;

    cellNum(input_irgrid3d_ptr, search_window_ptr, &cell_ids);
    cellNum(input_irgrid3d_ptr, search_window_ptr_2, &cell_ids_2);

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

/*
  Value mapping function of operator ~bbox\_grid~

*/
int
IrregularGrid3D::IrGrid2dValueMapBBox( Word* args, Word& result, int message,
    Word& local, Supplier s ) {

    Stream<Rectangle<2>> rStream(args[0]);
    result = qp->ResultStorage(s);
    Rectangle<2>* res = (Rectangle<2>*) result.addr;


    rStream.open();
    Rectangle<2>* next = rStream.request();

    if(next != 0) {
    double min[2], max[2];
    min[0] = next->getMinX();
    min[1] = next->getMinY();
    max[0] = next->getMaxX();
    max[1] = next->getMaxY();
    next = rStream.request();

    while(next != 0){
      if(next->getMinX() < min[0]) 
      { min[0] = next->getMinX(); }
      if(next->getMinY() < min[1])
      { min[1] = next->getMinY(); }
      if(next->getMaxX() > max[0])
      { max[0] = next->getMaxX(); }
      if(next->getMaxY() > max[1])
      { max[1] = next->getMaxY(); }

      next = rStream.request();
    }
    
    rStream.close();

    res->Set(true, min, max);
  
  return 0;
  }
  return -1;
  
}

/*
  Value mapping function of operator ~bbox\_grid3d~

*/
int
IrregularGrid3D::IrGrid3dValueMapBBox( Word* args, Word& result, int message,
    Word& local, Supplier s ) {

    Stream<Rectangle<3>> rStream(args[0]);
    result = qp->ResultStorage(s);
    Rectangle<3>* res = (Rectangle<3>*) result.addr;

    rStream.open();
    Rectangle<3>* next = rStream.request();
    if(next != 0) {
    double min[3], max[3];
    min[0] = next->getMinX();
    min[1] = next->getMinY();
    min[2] = next->getMinZ();
    max[0] = next->getMaxX();
    max[1] = next->getMaxY();
    max[2] = next->getMaxZ();

    next = rStream.request();

    while(next != 0){
      if(next->getMinX() < min[0]) 
      { min[0] = next->getMinX(); }
      if(next->getMinY() < min[1])
      { min[1] = next->getMinY(); }
      if(next->getMinZ() < min[2])
      { min[2] = next->getMinZ(); };
      if(next->getMaxX() > max[0])
      { max[0] = next->getMaxX(); }
      if(next->getMaxY() > max[1])
      { max[1] = next->getMaxY(); }
      if(next->getMaxZ() > max[2])
      { max[2] = next->getMaxZ(); }

      next = rStream.request();
    }
    rStream.close();

    res->Set(true, min, max);
  
  return 0;
  }
  return -1;
}