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


#include "IrregularGrid2D.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Collection/IntSet.h"
#include <map>
#include <iterator>

Cell::Cell() {
  cellId = -1;
  valFrom = -1;
  valTo = -1;
  nbrPoints = -1;
}

void
Cell::setCellId(int cell_id) {
  this->cellId = cell_id;
}

int
Cell::getCellId() {
  return cellId;
}

void
Cell::setValFrom(double val_from) {
  valFrom = val_from;
}

double
Cell::getValFrom() {
  return valFrom;
}

void
Cell::setValTo(double val_to) {
  valTo = val_to;
}

double
Cell::getValTo() {
  return valTo;
}

int
Cell::getNbrOfPoints() {
  return nbrPoints;
}

void
Cell::setNbrOfPoints(int nbr_points) {
  nbrPoints = nbr_points;
}

Cell::~Cell() { }

VCell::VCell() {
}

std::vector<HCell>&
VCell::getRow() {
  return row;
}

VCell::~VCell() { }

HCell::HCell() {
  upper = nullptr;
}

void
HCell::setUpper(HCell* upper_) {
  upper = upper_;
}

HCell*
HCell::getUpper() {
  return upper;
}

HCell::~HCell() { }

IrregularGrid2D::IrregularGrid2D() {
  boundingBox = nullptr;
  rowCount = 0;
  cellCount = 0;
}

IrregularGrid2D::IrregularGrid2D(const IrregularGrid2D& g) {
  boundingBox = g.boundingBox;
  rowCount = g.rowCount;
  cellCount = g.cellCount;
}

IrregularGrid2D::IrregularGrid2D(Rectangle<2> &bounding_box,
    int row_count, int cell_count) {
  boundingBox = &bounding_box;
  rowCount = row_count;
  cellCount = cell_count;
}

void
IrregularGrid2D::Set(Stream<Rectangle<2>> rStream,
    Rectangle<2> &bounding_box,
    int row_count, int cell_count) {

  boundingBox = &bounding_box;
  rowCount = row_count;
  cellCount = cell_count;

  createIrgrid2D(rStream);
}

void
IrregularGrid2D::SetVector(std::vector<Rectangle<2>>* rVector,
                     Rectangle<2> &bounding_box,
                     int row_count, int cell_count) {
   boundingBox = &bounding_box;
   rowCount = row_count;
   cellCount = cell_count;

   createIrgrid2DVector(rVector);
}

std::vector<CellInfo*>
IrregularGrid2D::getCellInfoVector(IrregularGrid2D *in_irgrid2d) {
  std::vector<CellInfo*> cell_info_vect {};

  std::vector<VCell>* col = &in_irgrid2d->getColumnVector();
  for(size_t colIdx = 0; colIdx < col->size(); colIdx++) {
    VCell* vcell = &col->at(colIdx);
    double vf = vcell->getValFrom();
    double vt = vcell->getValTo();

    std::vector<HCell>* row_vect = &vcell->getRow();

    for(size_t cellIdx = 0; cellIdx < row_vect->size(); cellIdx++) {
      HCell* cell = &row_vect->at(cellIdx);
      int cid = cell->getCellId();
      int np = cell->getNbrOfPoints();
      double hf = cell->getValFrom();
      double ht = cell->getValTo();

      CellInfo* ci = new CellInfo(cid, np, hf, ht, vf, vt);
      cell_info_vect.push_back(ci);
    }
  }

  return cell_info_vect;
}

void
IrregularGrid2D::createIrgrid2D(Stream<Rectangle<2>> rStream) {
  // sort input points by y-coordinates
  processInput(rStream);
  // create irregular grid 2d by point density
  buildGrid();
}

void
IrregularGrid2D::createIrgrid2DVector(std::vector<Rectangle<2>>* rVector) {
   // sort input points by y-coordinates
   processInputVector(rVector);
   // create irregular grid 2d by point density
   buildGrid();
}

bool
pointComparisonX(RPoint p1, RPoint p2) {
  return p1.x < p2.x;
}

void
IrregularGrid2D::buildGrid() {

  // create grid structure
  double bb_top = boundingBox->getMaxY();
  double bb_bot = boundingBox->getMinY();
  double colOffset = (bb_top - bb_bot) / rowCount;

  double bb_right = boundingBox->getMaxX();
  double bb_left = boundingBox->getMinX();
  double hOffset = (bb_right - bb_left) / cellCount;

  double col_boundary_val = bb_bot;
  int hcell_id = 1;

  for(int c = 0; c < rowCount; c++) {
    VCell vcell = VCell();
    vcell.setValFrom(col_boundary_val);

    col_boundary_val += colOffset;
    vcell.setValTo(col_boundary_val);

    double cell_boundary_val = bb_left;
    for(int r = 0; r < cellCount; r++) {
      HCell hcell = HCell();

      hcell.setValFrom(cell_boundary_val);

      cell_boundary_val += hOffset;
      hcell.setValTo(cell_boundary_val);

      hcell.cellId = hcell_id;
      hcell_id++;

      // will be determined later
      hcell.upper = nullptr;

      vcell.getRow().push_back(hcell);

    }
    columnVector.push_back(vcell);
  }

  // adjust boundaries by point distribution
  int nbrOfPoints = points.size();
  int pointsPerRow = nbrOfPoints / rowCount;
  std::vector<RPoint> tmp_row_points {};

  int pointIdx = 0;
  int point_counter = 0;
  for(int colIdx = 0; colIdx < rowCount; colIdx++) {
    for(size_t dp_idx = pointIdx; dp_idx < points.size(); dp_idx++) {
      RPoint rp = points[dp_idx];
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

        std::vector<HCell> c_row =  columnVector[colIdx].getRow();
        int pointsPerCell = tmp_row_points.size() / cellCount;

        int tmpPointIdx = 0;
        for(int h = 0; h < cellCount; h++) {
          std::vector<HCell>* hcel_vec_ptr = &columnVector[colIdx].getRow();

          for(size_t tmp_rp_idx = tmpPointIdx;
              tmp_rp_idx < tmp_row_points.size(); tmp_rp_idx++) {

            RPoint rp_t = tmp_row_points[tmp_rp_idx];
            point_counter ++;

            if((point_counter == pointsPerCell)
              || (tmp_rp_idx == tmp_row_points.size()-1)
              /* || (h == cellCount-1 ) */) {

              if (h > 0) {
                hcel_vec_ptr->at(h).setValFrom(
                  hcel_vec_ptr->at(h-1).getValTo());
              }

              double new_val_to_x;
              if ((tmp_rp_idx == tmp_row_points.size()-1)
                  || (h == cellCount-1)) {
                new_val_to_x = bb_right;
              } else {
                 new_val_to_x = rp_t.x;
              }
              hcel_vec_ptr->at(h).setValTo(new_val_to_x);

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

  // update cell pointer
  if (rowCount > 1 && cellCount > 0) {
    for(int c = 0; c < rowCount-1; c++) {
      std::vector<HCell>* row_lower = &getColumnVector().at(c).getRow();
      std::vector<HCell>* row_upper = &getColumnVector().at(c+1).getRow();

      int pointToCellIdx = 0;
      for(int h = 0; h < cellCount; h++) {
        HCell* lower_cell_ptr = &row_lower->at(h);
        HCell* upper_cell_ptr = &row_upper->at(pointToCellIdx);

        if (lower_cell_ptr->getValFrom() <= upper_cell_ptr->getValTo()) {
          lower_cell_ptr->setUpper(upper_cell_ptr);
        } else {
          HCell* next_upper_cell_ptr;
          do {
            pointToCellIdx ++;
            next_upper_cell_ptr = &row_upper->at(pointToCellIdx);
            lower_cell_ptr->setUpper(next_upper_cell_ptr);
          } while (lower_cell_ptr->getValFrom()
              >= next_upper_cell_ptr->getValTo());
        }
      }
    }
  }

  const double min[] { boundingBox->getMinX(), boundingBox->getMinY() };
   const double max[] { boundingBox->getMaxX(), boundingBox->getMaxY() };
  box.Set(true, min, max);
}

bool
pointComparisonY(RPoint p1, RPoint p2) {
  return p1.y < p2.y;
}

RPoint
getRectangleCentre(Rectangle<2>* r) {
  double a = (r->getMaxY() - r->getMinY()) / (double)2;
  double b = (r->getMaxX() - r->getMinX()) / (double)2;

  RPoint r_c { (r->getMinX())+b, (r->getMinY())+a };

  return r_c;
}

// check if a rectangle is inside the irgrid2d bounding box
bool
insideBoundingBox(Rectangle<2>* bbox, Rectangle<2>* r) {
  double le = bbox->getMinX();
  double ri = bbox->getMaxX();
  double bo = bbox->getMinY();
  double to = bbox->getMaxY();

  if (r->getMinX() >= le && r->getMaxX() <= ri
      && r->getMinY() >= bo && r->getMaxY() <=to) {
    return true;
  }

  return false;
}

void
IrregularGrid2D::processInput(Stream<Rectangle<2>> rStream) {
  rStream.open();
  Rectangle<2>* next = rStream.request();

  while(next != 0){
    if (!insideBoundingBox(boundingBox, next)) {
      // rectangle (partially) outside the bounding box is discarded
      next = rStream.request();
      continue;
    }

    points.push_back(getRectangleCentre(next));
    next = rStream.request();
  }
  rStream.close();

  // sort point vector by y-coordinates
  std::sort(points.begin(), points.end(), pointComparisonY);
}

void
IrregularGrid2D::processInputVector(std::vector<Rectangle<2>>* rVector) {
   for (Rectangle<2> bbox : *rVector) {
      if (!insideBoundingBox(boundingBox, &bbox)) {
         // rectangle (partially) outside the bounding box is discarded
         continue;
      }
      points.push_back(getRectangleCentre(&bbox));
   }

   // sort point vector by y-coordinates
   std::sort(points.begin(), points.end(), pointComparisonY);
}

Rectangle<2> *
IrregularGrid2D::getBoundingBox() {
  return boundingBox;
}

int
IrregularGrid2D::getRowCount() {
  return rowCount;
}

int
IrregularGrid2D::getCellCount() {
  return cellCount;
}

void
IrregularGrid2D::setColumnVector(std::vector<VCell> column_vect) {
  this->columnVector = column_vect;
}

std::vector<VCell>&
IrregularGrid2D::getColumnVector() {
  return this->columnVector;
}

IrregularGrid2D::~IrregularGrid2D() {}

// Type Constructor irgrid2d
ListExpr
IrregularGrid2D::PropertyIrGrid2D()
{
    ListExpr desclst = nl->TextAtom();
    nl->AppendText(desclst,
    "A <rectangle> bounding box followed by list of rows.\n"
    "A row consists of two-element list (<from> <to>) "
    "followed by list of cells.\n"
    "A cell consists of four-element list\n(<from> <to> <id> <ref_id>).");

  ListExpr formatlst = nl->TextAtom();
    nl->AppendText(formatlst,
    "((0.0 2.0 0.0 2.0)(\n(0.0 0.45)((0.0 0.4 1 3)(0.4 2.0 2 4))\n"
    "(0.45 2.0)((0.0 0.3 3 -1)(0.3 2.0 4 -1))))");

  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom(IrregularGrid2D::BasicType()),
               desclst,
               formatlst)));
}


// Out function
ListExpr
IrregularGrid2D::OutIrGrid2D( ListExpr typeInfo, Word value ) {
  IrregularGrid2D* irgrid2d = static_cast<IrregularGrid2D*>( value.addr );

  if (irgrid2d != nullptr) {
    Rectangle<2> * b_box = irgrid2d->getBoundingBox();
    ListExpr bboxLstExpr = nl->FourElemList(
      nl->RealAtom(b_box->getMinX()),
      nl->RealAtom(b_box->getMaxX()),
      nl->RealAtom(b_box->getMinY()),
      nl->RealAtom(b_box->getMaxY()));

    std::vector<VCell>* col = &irgrid2d->getColumnVector();

    ListExpr rowLstExpr = nl->Empty();
    ListExpr lastRowLstExpr;
    if (col->size() > 0) {
      for(size_t colIdx = 0; colIdx < col->size(); colIdx++) {
        VCell* vcell = &col->at(colIdx);
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

        std::vector<HCell>* row_vect = &col->at(colIdx).getRow();
        if (row_vect->size() > 0) {
          ListExpr cellLstExpr;
          ListExpr lastCellLstExpr;
          for(size_t rowIdx = 0; rowIdx < row_vect->size(); rowIdx++) {
            HCell* row_cell = &row_vect->at(rowIdx);

            if (rowIdx > 0) {
              lastCellLstExpr = nl->Append(lastCellLstExpr,
                  nl->FourElemList(nl->RealAtom(row_cell->getValFrom()),
                  nl->RealAtom(row_cell->getValTo()),
                  nl->IntAtom(row_cell->getCellId()),
                  nl->IntAtom((row_cell->getUpper() != nullptr
                    ? row_cell->getUpper()->getCellId() : -1))));
            } else {
              cellLstExpr = nl->OneElemList(nl->FourElemList
                  (nl->RealAtom(row_cell->getValFrom()),
                  nl->RealAtom(row_cell->getValTo()),
                  nl->IntAtom(row_cell->getCellId()),
                  nl->IntAtom((row_cell->getUpper() != nullptr
                    ? row_cell->getUpper()->getCellId() : -1))));
              lastCellLstExpr = cellLstExpr;
            }
          }
          lastRowLstExpr = nl->Append(lastRowLstExpr, cellLstExpr);
        }
      }
    }
    ListExpr irgrid2dLstExpr = nl->TwoElemList(bboxLstExpr, rowLstExpr);
    return irgrid2dLstExpr;
  } else {
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
  }
}


// In function
Word
IrregularGrid2D::InIrGrid2D( const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct ) {

  Word w = SetWord(Address(0));

  try {
    Rectangle<2>* bbox;

    ListExpr bboxLstExpr;
    ListExpr rowLstExpr;

    if ( nl->ListLength( instance ) == 2 ) {
      bboxLstExpr = nl->First(instance);
      rowLstExpr = nl->Second(instance);
    } else {
      throw 1;
    }

    // fetch bounding box information from input
    if (nl->ListLength( bboxLstExpr ) == 4 ) {
      ListExpr left = nl->First(bboxLstExpr);
      ListExpr right = nl->Second(bboxLstExpr);
      ListExpr bottom = nl->Third(bboxLstExpr);
      ListExpr top = nl->Fourth(bboxLstExpr);

      if ( nl->IsAtom(left) && nl->AtomType(left) == RealType
          && nl->IsAtom(right) && nl->AtomType(right) == RealType
          && nl->IsAtom(bottom) && nl->AtomType(bottom) == RealType
          && nl->IsAtom(top) && nl->AtomType(top) == RealType) {

        double min[2], max[2];
        min[0] = nl->RealValue(left);
        min[1] = nl->RealValue(bottom);
        max[0] = nl->RealValue(right);
        max[1] = nl->RealValue(top);

        bbox = new Rectangle<2>(true, min, max);
      } else {
        throw 3;
      }

    } else {
      throw 2;
    }

    // fetch row information from input

    // temporary support structures
    std::map<int, int> cellRef;
    std::map<int, HCell*> cellIds;

    std::vector<VCell> column_vec {};
    int row_cnt = 0;
    int cell_cnt = 0;
    if (nl->ListLength( rowLstExpr ) > 1 ) {
      VCell vc;
      VCell* vc_ptr;

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
            vc = VCell();

            vc.setValFrom(nl->RealValue(fLst));
            vc.setValTo(nl->RealValue(tLst));
            column_vec.push_back(vc);
            vc_ptr = &(column_vec.back());
          }
        } else {
          ListExpr rowLstExpr = lstElem; // row

          while(!nl->IsEmpty(rowLstExpr)) {
            // cell information
            ListExpr cellLstExpr = nl->First(rowLstExpr);
            if (nl->ListLength( cellLstExpr ) == 4) {
              // a four-element list initiates a new cell
              ListExpr cv1Lst = nl->First(cellLstExpr);
              ListExpr cv2Lst = nl->Second(cellLstExpr);
              ListExpr cv3Lst = nl->Third(cellLstExpr);
              ListExpr cv4Lst = nl->Fourth(cellLstExpr);

              if ( nl->IsAtom(cv1Lst) && nl->AtomType(cv1Lst) == RealType
                  && nl->IsAtom(cv2Lst) && nl->AtomType(cv2Lst) == RealType
                  && nl->IsAtom(cv3Lst) && nl->AtomType(cv3Lst) == IntType
                  && nl->IsAtom(cv4Lst) && nl->AtomType(cv4Lst) == IntType) {
                HCell* hc = new HCell();
                hc->setValFrom(nl->RealValue(cv1Lst));
                hc->setValTo(nl->RealValue(cv2Lst));
                hc->setCellId(nl->IntValue(cv3Lst));
                // will be determined later
                hc->setUpper(nullptr);

                vc_ptr->getRow().push_back(*hc);
                int cellRefId = nl->IntValue(cv4Lst);
                if (cellRefId != -1) {
                  cellRef.insert(std::make_pair(
                    ((int)nl->IntValue(cv3Lst)), cellRefId));
                }
                cellIds.insert(std::make_pair(
                  ((int)nl->IntValue(cv3Lst)), hc));
              } else {
                throw 5;
              }
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
    if (row_cnt > 0 && cell_cnt > 0) {
      for(int colIdx = 0; colIdx < row_cnt-1; colIdx++) {
        VCell* vcell = &column_vec.at(colIdx);
        std::vector<HCell>* row_vect = &vcell->getRow();
        for(int cIdx = 0; cIdx < cell_cnt; cIdx++) {
          HCell* hcell = &(*row_vect).at(cIdx);
          if(cellRef.find(hcell->getCellId()) != cellRef.end()) {
            int cell_ref = cellRef.at(hcell->getCellId());
            if(cellIds.find(cell_ref) != cellIds.end()) {
              hcell->setUpper(cellIds.at(cell_ref));
            }
          }
        }
      }
    }

    correct = true;
    IrregularGrid2D* irgrid = new IrregularGrid2D(*bbox, row_cnt, cell_cnt);
    irgrid->setColumnVector(column_vec);
    irgrid->box = *bbox;
    w.addr = irgrid;
    return w;
  } catch (int e) {
    correct = false;
    cmsg.inFunError("Expecting a irgrid2d list representation. Exit code "
        + std::to_string(e));

    return w;
  }
}

bool
IrregularGrid2D::OpenIrGrid2D(SmiRecord& valueRecord,
 size_t& offset, const ListExpr typeInfo, Word& value) 
{
size_t size = sizeof(int);
size_t sizeD = sizeof(double);
int xr = 0, yb = 0;
size_t cols = 0, rows = 0;
double vf = 0.0, vt = 0.0; //column values
double rvf = 0.0, rvt = 0.0; // row values
int rid = 0, cellu = 0;
double xmin=0.0, xmax=0.0, ymin=0.0, ymax=0.0;
std::map<int, int> cellRef;
std::map<int, HCell*> cellIds;

std::vector<VCell> column_vec {};
VCell vc;
VCell* vc_ptr;
bool ok = true;

ok = ok && valueRecord.Read( &xmin, sizeD, offset );
offset += sizeD;
ok = ok && valueRecord.Read( &xmax, sizeD, offset );
offset += sizeD;
ok = ok && valueRecord.Read( &ymin, sizeD, offset );
offset += sizeD;
ok = ok && valueRecord.Read( &ymax, sizeD, offset );
offset += sizeD;

ok = ok && valueRecord.Read( &xr, size, offset );
offset += size;
ok = ok && valueRecord.Read( &yb, size, offset );
offset += size;

// colsize
ok = ok && valueRecord.Read( &cols, sizeof(size_t), offset );
offset += sizeof(size_t);
for(size_t i = 0; i < cols; i++)
{
  vc = VCell();
  ok = ok && valueRecord.Read( &vf, sizeD, offset );
  offset += sizeD;
  ok = ok && valueRecord.Read( &vt, sizeD, offset );
  offset += sizeD;
  vc.setValFrom(vf);
  vc.setValTo(vt);
  column_vec.push_back(vc);
  vc_ptr = &(column_vec.back());

  //rowsize
  ok = ok && valueRecord.Read( &rows, sizeof(size_t), offset );
  offset += sizeof(size_t);
  for(size_t j = 0; j < rows; j++)
  {
    HCell* hc = new HCell();
    ok = ok && valueRecord.Read( &rvf, sizeD, offset );
    offset += sizeD;
    ok = ok && valueRecord.Read( &rvt, sizeD, offset );
    offset += sizeD;
    ok = ok && valueRecord.Read( &rid, size, offset );
    offset += size;
    ok = ok && valueRecord.Read( &cellu, size, offset );
    offset += size;
    hc->setValFrom(rvf);
    hc->setValTo(rvt);
    hc->setCellId(rid);
    hc->setUpper(nullptr);
    vc_ptr->getRow().push_back(*hc);
    int cellRefId = cellu;
    if (cellRefId != -1) {
      cellRef.insert(std::make_pair(
      rid, cellRefId));
    }
    cellIds.insert(std::make_pair(
    rid, hc));
  }
}
// update pointer
  if (xr > 0 && yb > 0) {
    for(int colIdx = 0; colIdx < xr; colIdx++) {
      VCell* vcell = &column_vec.at(colIdx);
      std::vector<HCell>* row_vect = &vcell->getRow();
      for(int cIdx = 0; cIdx < yb; cIdx++) {
        HCell* hcell = &(*row_vect).at(cIdx);
        if(cellRef.find(hcell->getCellId()) != cellRef.end()) {
          int cell_ref = cellRef.at(hcell->getCellId());
          if(cellIds.find(cell_ref) != cellIds.end()) {
            hcell->setUpper(cellIds.at(cell_ref));
          }
        }
      }
    }
  }

  const double min[] { xmin, ymin };
  const double max[] { xmax, ymax };
  Rectangle<2> *bbox = new Rectangle<2>(true, min, max);
  IrregularGrid2D* irgrid = new IrregularGrid2D(*bbox, xr, yb);
  irgrid->setColumnVector(column_vec);
  irgrid->box = *bbox;
  value.addr = irgrid;

  return ok;
}

bool
IrregularGrid2D::SaveIrGrid2D(SmiRecord& valueRecord,
 size_t& offset, const ListExpr typeInfo, Word& value) 
{
   IrregularGrid2D* r = static_cast<IrregularGrid2D*>( value.addr );

   size_t size = sizeof(int);
   size_t sizeD = sizeof(double);
   bool ok = true;

   double minx = r->box.getMinX();//->getMaxX();
   double maxx = r->box.getMaxX();//->getMaxX();
   double miny = r->box.getMinY();// b_box->getMinY();
   double maxy = r->box.getMaxY(); // b_box->getMaxY();

   ok = ok && valueRecord.Write(&minx, sizeD, offset );
   offset += sizeD;
   ok = ok && valueRecord.Write(&maxx, sizeD, offset );
   offset += sizeD;
   ok = ok && valueRecord.Write(&miny, sizeD, offset );
   offset += sizeD;
   ok = ok && valueRecord.Write(&maxy, sizeD, offset );
   offset += sizeD;

   ok = ok && valueRecord.Write(&r->rowCount, size, offset );
   offset += size;
   ok = ok && valueRecord.Write(&r->cellCount, size, offset );
   offset += size;

  std::vector<VCell>* col = &r->getColumnVector();
  size_t cols = col->size();
  if (col->size() > 0) {
    ok = ok && valueRecord.Write(&cols, sizeof(size_t), offset );
    offset += sizeof(size_t);
    for(size_t colIdx = 0; colIdx < col->size(); colIdx++) {
      VCell* vcell = &col->at(colIdx);
      double vf = vcell->getValFrom();
      double vt = vcell->getValTo();
      ok = ok && valueRecord.Write(&vf, sizeD, offset );
      offset += sizeD;
      ok = ok && valueRecord.Write(&vt, sizeD, offset );
      offset += sizeD;
      

      std::vector<HCell>* row_vect = &col->at(colIdx).getRow();
      size_t rows = row_vect->size();
      if (row_vect->size() > 0) {
        ok = ok && valueRecord.Write(&rows, sizeof(size_t), offset );
        offset += sizeof(size_t);          
        for(size_t rowIdx = 0; rowIdx < row_vect->size(); rowIdx++) {
          HCell* row_cell = &row_vect->at(rowIdx);
          double rvf = row_cell->getValFrom();
          double rvt = row_cell->getValTo();
          int ri = row_cell->getCellId();
          int ru = row_cell->getUpper() != nullptr
            ? row_cell->getUpper()->getCellId() : -1;
          ok = ok && valueRecord.Write(&rvf, sizeD, offset );
          offset += sizeD;
          ok = ok && valueRecord.Write(&rvt, sizeD, offset );
          offset += sizeD;
          ok = ok && valueRecord.Write(&ri, size, offset );
          offset += size;
          ok = ok && valueRecord.Write(&ru, size, offset );
          offset += size;
          
          }
        }
      }
    }

  return ok;

}


// This function checks whether the type constructor is applied correctly.
bool
IrregularGrid2D::KindCheckIrGrid2D( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, IrregularGrid2D::BasicType() ));
}

// Close -function
void
IrregularGrid2D::CloseIrGrid2D( const ListExpr typeInfo, Word& w )
{
  delete (IrregularGrid2D *)w.addr;
  w.addr = 0;
}

// Clone function
Word
IrregularGrid2D::CloneIrGrid2D( const ListExpr typeInfo, const Word& w )
{
  IrregularGrid2D *g = new IrregularGrid2D( *((IrregularGrid2D *)w.addr) );
  return SetWord( g );
}

// Create function
Word
IrregularGrid2D::CreateIrGrid2D( const ListExpr typeInfo )
{
  return SetWord( new IrregularGrid2D() );
}

// Delete function
void
IrregularGrid2D::DeleteIrGrid2D( const ListExpr typeInfo, Word& w )
{
  delete (IrregularGrid2D *)w.addr;
  w.addr = 0;
}

// SizeOf function
int
IrregularGrid2D::SizeOfIrGrid2D()
{
  return sizeof(IrregularGrid2D);
}

/*
Type mapping function ~IrGrid2dFeedTypeMap~

It is used for the ~feed~ operator.

*/
ListExpr
IrregularGrid2D::IrGrid2dFeedTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 1)) {

    ListExpr first = nl->First(args);
    if (IrregularGrid2D::checkType(first)) {
      ListExpr resAttrList = nl->ThreeElemList(
          nl->TwoElemList(
            nl->SymbolAtom("Id"),
            nl->SymbolAtom(CcInt::BasicType())),
          nl->TwoElemList(
            nl->SymbolAtom("Count"),
            nl->SymbolAtom(CcInt::BasicType())),
          nl->TwoElemList(
            nl->SymbolAtom("Cell"),
            nl->SymbolAtom(Rectangle<2>::BasicType())));

      return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple>>(),
        nl->TwoElemList(
          listutils::basicSymbol<Tuple>(),
          resAttrList));

      }
  }

  const std::string errMsg = "The following argument is expected:"
      " irgrid2d";

  return  listutils::typeError(errMsg);
}

// for value mapping function of ~feed~ operator
struct IrGridTupleInfo
{
  std::vector<CellInfo*> cell_info_vect;
  unsigned int currentTupleIdx;
  ListExpr numTupleTypeList;

  void init(IrregularGrid2D *irgrid2d_in) {
    currentTupleIdx = 0;
    cell_info_vect = IrregularGrid2D::getCellInfoVector(irgrid2d_in);

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
          nl->SymbolAtom("Cell"),
          nl->SymbolAtom(Rectangle<2>::BasicType()))));

    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    numTupleTypeList = sc->NumericType(tupleTypeLst);
  }

  TupleType* getTupleType() {
    TupleType *tupleType = new TupleType(numTupleTypeList);

    return tupleType;
  }

  Tuple* getNext(TupleType *ttype) {
    if (currentTupleIdx < cell_info_vect.size()) {
      CellInfo * cell_info = cell_info_vect.at(currentTupleIdx);
      int tp_p1 = cell_info->cellId;
      int tp_p2 = cell_info->statNbrOfPoints;
      Rectangle<2> tp_p3 = *cell_info->cell;

      Tuple *tuple = new Tuple(ttype);
      tuple->PutAttribute(0, new CcInt(true, tp_p1));
      tuple->PutAttribute(1, new CcInt(true, tp_p2));
      tuple->PutAttribute(2, new Rectangle<2> (tp_p3));

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
IrregularGrid2D::IrGrid2dValueMapFeed( Word* args, Word& result, int message,
                        Word& local, Supplier s ) {
  IrregularGrid2D *input_irgrid2d_ptr =
    static_cast<IrregularGrid2D*>( args[0].addr );

  IrGridTupleInfo* tp_info = static_cast<IrGridTupleInfo*>(local.addr);
  TupleType* tupleType = nullptr;
  Tuple* tuple = nullptr;

  switch (message) {
    case OPEN: {
      tp_info = new IrGridTupleInfo();
      tp_info->init(input_irgrid2d_ptr);
      local.addr = tp_info;

      return 0;
    }
    case REQUEST: {
      if (local.addr) {
        tp_info = ((IrGridTupleInfo*)local.addr);
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
        tp_info = ((IrGridTupleInfo*)local.addr);
        delete tp_info;
        local.addr = 0;
      }
      return 0;
    }
  }

  return -1;
}

/*
Type mapping function ~IrGrid2dCellnosTypeMap~

It is used for the ~cellnos\_ir~ operator.

*/
ListExpr
IrregularGrid2D::IrGrid2dCellnosTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 2)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if (IrregularGrid2D::checkType(first) && Rectangle<2>::checkType(second)) {
      return nl->SymbolAtom(collection::IntSet::BasicType());
    }
  }

  const std::string errMsg = "The following two arguments are expected:"
      " irgrid2d x rect";

  return  listutils::typeError(errMsg);
}

ListExpr
IrregularGrid2D::IrGrid2dSCCTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 4)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr third = nl->Third(args);
    ListExpr fourth = nl->Fourth(args);

    if (IrregularGrid2D::checkType(first) && Rectangle<2>::checkType(second)
        && Rectangle<2>::checkType(third) && CcInt::checkType(fourth)) {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }

  const std::string errMsg = "The following four arguments are expected:"
      " irgrid2d x rect x rect x int";

  return  listutils::typeError(errMsg);
}

ListExpr
IrregularGrid2D::IrGrid2dGetCellTypeMap( ListExpr args )
{
  if(nl->HasLength(args, 2)) {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);

    if (IrregularGrid2D::checkType(first) && CcInt::checkType(second)) {
      return nl->SymbolAtom(Rectangle<2>::BasicType());
    }
  }

  const std::string errMsg = "The following two arguments are expected:"
      " irgrid2d x int";

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

void cellNum( IrregularGrid2D *input_irgrid2d_ptr, 
  Rectangle<2> *search_window_ptr, std::set<int> *cell_ids)
{
      std::vector<VCell>* col = &input_irgrid2d_ptr->getColumnVector();

    double le = search_window_ptr->getMinX();
    double ri = search_window_ptr->getMaxX();
    double bo = search_window_ptr->getMinY();
    double to = search_window_ptr->getMaxY();

    int pos_bo = CellBS(col, 0, col->size(), bo);
    if (pos_bo != -1) {
      VCell vCell = col->at(pos_bo);
      std::vector<HCell>* row = &vCell.getRow();

      int pos_le = CellBS(row, 0, row->size(), le);
      if (pos_le != -1) {

        // collect ids
        unsigned int cellIdx = pos_le;
        while (cellIdx < row->size()) {
          HCell i = row->at(cellIdx);
          cell_ids->insert(i.getCellId());

          if ((ri >= i.getValFrom() && ri < i.getValTo())
            || (cellIdx == row->size()-1  && ri >= i.getValFrom()))  {
              HCell fi = row->at(pos_le);
              if (to >= vCell.getValTo() && fi.getUpper() != nullptr) {
                vCell = col->at(++pos_bo);
                row = &vCell.getRow();

                HCell * u = fi.getUpper();
                int nbr_cpr = input_irgrid2d_ptr->getCellCount();
                int cid_pos = (u->getCellId()) % nbr_cpr;
                pos_le = cid_pos == 0 ? nbr_cpr-1 : cid_pos-1;

                cellIdx = pos_le-1;
            } else if (to < vCell.getValTo() || fi.getUpper() == nullptr) {
              break;
            }
          }
          cellIdx++;
        }

      }
    }
}

/*
Value mapping function of operator ~cellnos\_ir~

*/
int
IrregularGrid2D::IrGrid2dValueMapCellnos( Word* args, Word& result, int message,
    Word& local, Supplier s ) {
  IrregularGrid2D *input_irgrid2d_ptr
    = static_cast<IrregularGrid2D*>( args[0].addr );

  Rectangle<2> *search_window_ptr
    = static_cast<Rectangle<2>*>( args[1].addr );

  if (input_irgrid2d_ptr != nullptr && search_window_ptr != nullptr) {
    std::set<int> cell_ids;

    result = qp->ResultStorage(s);
    collection::IntSet* res = (collection::IntSet*) result.addr;

    Rectangle<2> * b_box = input_irgrid2d_ptr->getBoundingBox();
    if (!search_window_ptr->Intersects(*b_box)) {
      cell_ids.insert(0);
      res->setTo(cell_ids);
      return 0;
    }

    // 'truncate' search window in case of partial cutting
    if (!b_box->Contains(*search_window_ptr)) {
      search_window_ptr = new Rectangle<2>(
        search_window_ptr->Intersection(*b_box));

      cell_ids.insert(0);
    }

    cellNum(input_irgrid2d_ptr, search_window_ptr, &cell_ids);

    res->setTo(cell_ids);

    return 0;

  }
  return -1;
}


int
IrregularGrid2D::IrGrid2dValueMapSCC( Word* args, Word& result, int message,
  Word& local, Supplier s) {
  IrregularGrid2D *input_irgrid2d_ptr
    = static_cast<IrregularGrid2D*>( args[0].addr );

  Rectangle<2> *search_window_ptr
    = static_cast<Rectangle<2>*>( args[1].addr );
  Rectangle<2> *search_window_ptr2
    = static_cast<Rectangle<2>*>( args[2].addr );

  CcInt* cellno_ptr = static_cast<CcInt*>(args[3].addr);
  int cellno = cellno_ptr->GetIntval();

  std::set<int> cell_ids;
  std::set<int> cell_ids2;


  if (input_irgrid2d_ptr != nullptr && search_window_ptr != nullptr
    && search_window_ptr2 != nullptr) 
  {
    result = qp->ResultStorage( s );
    CcBool *res = (CcBool*) result.addr;
    bool boolval = false;

    Rectangle<2> * b_box = input_irgrid2d_ptr->getBoundingBox();
    if (!search_window_ptr->Intersects(*b_box) 
    || !search_window_ptr2->Intersects(*b_box)) {
      return 0;
    }

    // 'truncate' search window in case of partial cutting
    if (!b_box->Contains(*search_window_ptr)) {
      search_window_ptr = new Rectangle<2>(
        search_window_ptr->Intersection(*b_box));

      cell_ids.insert(0);
    }

    if (!b_box->Contains(*search_window_ptr2)) {
      search_window_ptr2 = new Rectangle<2>(
        search_window_ptr2->Intersection(*b_box));

      cell_ids2.insert(0);
    }

    cellNum(input_irgrid2d_ptr, search_window_ptr, &cell_ids);
    cellNum(input_irgrid2d_ptr, search_window_ptr2, &cell_ids2);

    std::vector<int> v(sizeof(cell_ids)+ sizeof(cell_ids2));
    std::vector<int>::iterator it;

    it=std::set_intersection (cell_ids.begin(), cell_ids.end(),
         cell_ids2.begin(), cell_ids2.end(), v.begin());
    v.resize(it-v.begin());                      
  
    if(v.empty()) { 
    //no intersection between rectangles
      res->Set( true, boolval);
      return 0;
    }

    if(v[0] == cellno)
    {
      boolval = true;
      res->Set( true, boolval);
      return 0;
    }

    res->Set( true, boolval);
    return 0;
  }
  return -1;
}

int
IrregularGrid2D::IrGrid2dValueMapGetCell(Word* args, Word& result, int message,
  Word& local, Supplier s)
{
  IrregularGrid2D *input_irgrid2d_ptr
    = static_cast<IrregularGrid2D*>( args[0].addr );

  CcInt* cellno_ptr = static_cast<CcInt*>(args[1].addr);
  int cellno = cellno_ptr->GetIntval();

  if (input_irgrid2d_ptr != nullptr)
  {
    result = qp->ResultStorage( s );
    Rectangle<2> *res = (Rectangle<2>*) result.addr;

    std::vector<VCell>* column = &input_irgrid2d_ptr->getColumnVector();
    for(size_t i = 0; i < column->size(); i++)
    {
      VCell vCell = column->at(i);
      std::vector<HCell>* row = &vCell.getRow();
      for(size_t ii=0; ii < row->size(); ii++)
      {
        HCell hCell = row->at(ii);
        if(cellno == hCell.getCellId())
        {
          double min[2], max[2];
          min[0] = hCell.getValFrom();
          min[1] = vCell.getValFrom();
          max[0] = hCell.getValTo();
          max[1] = vCell.getValTo();
          res->Set(true, min, max);
          return 0;
        }
      }
    }
  }


  return -1;

}
