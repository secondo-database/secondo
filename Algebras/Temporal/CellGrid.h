
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

Update on 2th Apr. Jimain Lu
Increase a CellGrid template class, which sets cell-grids for arbitrary D dimension,
and opens at its Dth axis. The CellGrid<2> can be used to replace CellGrid2D.

*/

/*
1 Class CellGrid2D

This class defines a grid in 2d Euclidean space which
is open in y direction.


*/

#include <limits>
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

/*
2 Class CellGrid

This template class defines a grid in D dimension Euclidean space,
and it opens at its Dth axis.
It can be used to replace the CellGrid2D,
which I still keep for the sake of safety, though.

Basically, this function is implemented after the ~Rectangle~ class.

*/
template <unsigned dim>
class CellGrid : public Attribute{
public:

/*
Do not use the standard constructor:

*/
inline CellGrid(){}

/*
Check the validity of parameters in the constructor

*/
inline CellGrid(const bool defined, ...);

/*
Called after the above function, while the parameters have been checked.

*/
inline CellGrid( const bool defined, const double* originPoint,
    const double *cellWidthes, const int32_t* cellNumbers);
/*
The copy constructor.

*/
inline CellGrid( const CellGrid<dim>& cg);
/*
Redefinition of operator ~=~

*/
inline CellGrid<dim>& operator = (const CellGrid<dim>& r);
/*
Destructor

*/
~CellGrid(){}
/*
Checks if the cellgrid is defined.

*/
inline void set(/*const bool defined,*/ const double* originPoint,
    const double *cellWidthes, const int32_t* cellNumbers);
/*
Get denoted parameters

*/
double getOrigin( int d ) const;
double getCellWidth( int d ) const;
int32_t getCellNum( int d ) const;
/*
Get the maximum d-coordinate lying on the grid

*/
double getMaxCoord( int d ) const;
/*
Get the minimum d-coordinate lying on the grid

*/
double getMinCoord( int d ) const;
/*
Returns a coordinate's index on one axis

*/
int32_t getIndex(const double &c, const int &d) const;
/*
Returns true iff p(dim) is located inside this grid.
p is a dim-dimension point

*/
bool onGrid(const double p[dim]) const;
/*
Returns the cell number of a given point p(dim)

*/
int32_t getCellNo(const double p[dim]) const;
/*
Get the cell-grid's MBR

*/
Rectangle<dim> getMBR() const;

//TODO Define the row and the column of a cell-grid
//Hard for grids with dims larger than 2.
/*
Get the MBR of the cell-grid on n-th row

*/
/*
Get the MBR of the cell-grid on n-th column

*/

/*
Check whether a cell number is valid.

*/
inline bool isValidCellNo(const int32_t &n) const;
/*
Returns a constant invalid cell number

*/
int32_t getInvalidCellNo() const;
/*
Print the cell-grid

*/
ostream& Print( ostream &os) const;



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
  double p0[dim];                // origin of the grid
  double cw[dim];                // cell-widths for every dimension
  int32_t no_cells[dim];         // numbers of cells along (D-1)axes

/*
Check whether the cell-grid is defined properly, based on following rules:

  * Denoted cell numbers must be positive integer numbers

  * Cell widths are also positive,
hence the cell-grid only expands in the first quadrant,
start from the given origin



*/
inline bool Proper(bool isSet = false) const;
/*
Check whether a given index is valid of a grid.

*/
inline bool isValidCellIndex( const int32_t& index, const int& d) const;

};


/*
2.1 Implementation of the class ~CellGrid~

*/

/*
Check the validity of parameters in the constructor.
Simulate the function for the rectangle algebra.

*/
template<unsigned dim>
inline CellGrid<dim>::CellGrid( const bool defined, ... )
  //: StandardSpatialAttribute<dim>(defined)
{
  va_list ap;
  va_start(ap, defined);
  for ( int i = 0; i < 3 ; i++ )
  {
    // first round for the origin of the grid,
    // second round for the cell width,
    // and the third round for the cell numbers of (dim-1) axes
    if (i < 2 )
    {
      for ( unsigned i = 0; i < dim; i++ )
      {
        double ag = va_arg( ap, double );
        if ( 0 == i ){
          p0[i] = ag;
        }
        else{
          cw[i] = ag;
        }
      }
    }
    else
    {
      //The grid opens at the Dth axis
      for ( unsigned i = 0; i < (dim - 1); i++){
        int32_t num = va_arg( ap, int32_t );
        no_cells[i] = num;
      }
    }
  }
  va_end(ap);

  if (!Proper())
  {
    this->del.isDefined = false;
  }
}

/*
Called after the above function, while the parameters have been checked.

*/
template<unsigned dim>
inline CellGrid<dim>::CellGrid( const bool defined, const double* originPoint,
    const double *cellWidthes, const int32_t* cellNumbers):
    StandardSpatialAttribute<dim>(defined)
{
  set(defined, originPoint, cellWidthes, cellNumbers);
}

/*
The copy constructor.

*/
template<unsigned dim>
inline CellGrid<dim>::CellGrid(const CellGrid<dim>& g)
  //:  StandardSpatialAttribute<dim>(g.IsDefined())
{
  for ( unsigned i = 0; i < dim; i++)
  {
    p0[i] = g.p0[i];
    cw[i] = g.cw[i];
    if ( i < (dim - 1))
      no_cells[i] = g.no_cells[i];
  }
  assert( Proper() );
}

/*
Redefinition of operator ~=~

*/
template<unsigned dim>
inline CellGrid<dim>& CellGrid<dim>::operator= (const CellGrid<dim>& g)
{
  this->del.isDefined = g.IsDefined();
  if ( (this->IsDefined()))
  {
    for ( unsigned i = 0; i < dim; i++)
    {
      p0[i] = g.p0[i];
      cw[i] = g.cw[i];
      if ( i < (dim - 1))
        no_cells[i] = g.no_cells[i];
    }
  }
  assert( Proper() );
  return *this;
}

/*
Check whether a given index is valid of a grid.

*/
template<unsigned dim>
inline bool CellGrid<dim>::isValidCellIndex(
    const int32_t& index, const int& d) const
{
  assert ( d >= 0 && (unsigned)d < dim );

  if ( index < 0 )
    return false;
  if ( (unsigned)d < (dim - 1) )
    return (index <= no_cells[d]);

  return true;
}


/*
Check whether a cell number is valid.

*/
template<unsigned dim>
inline bool CellGrid<dim>::isValidCellNo(const int32_t &n) const{
  return this->IsDefined() && (n>0);
}


/*
Checks if the cellgrid is defined.

*/
template<unsigned dim>
void CellGrid<dim>::set(const double* originPoint,
   const double *cellWidthes, const int32_t* cellNumbers)
{
  for ( unsigned i = 0; i < dim; i++)
  {
    p0[i] = originPoint[i];
    cw[i] = cellWidthes[i];
    if ( i < (dim - 1))
      no_cells[i] = cellNumbers[i];
  }
  if ( !Proper() )
  {
    static MessageCenter* msg = MessageCenter::GetInstance();
    NList msgList( NList("simple"),
                   NList("CellGrid built with invalid dimensions!") );
    msg->Send(msgList);
    this->SetDefined(false);
  }
  this->SetDefined(true);
}

/*
Check whether the cell-grid is defined properly, based on following rules:


*/
template<unsigned dim>
inline bool CellGrid<dim>::Proper(bool isSet) const
{
  if ( (this->IsDefined()) )
  {
    for ( unsigned i = 0; i < dim; i++)
    {
      if ((cw[i] < 0) || AlmostEqual(cw[i], 0.0))
        return false;
      if ( i < (dim - 1)){
        if ( no_cells[i] <= 0 )
          return false;
      }
    }
  }
  return true;
}


/*
Implementation of methods for class ~CellGrid~

*/
/*
Get denoted parameters

*/
template<unsigned dim>
double CellGrid<dim>::getOrigin( int d ) const{
  assert ( d >= 0 && (unsigned)d < dim );
  return p0[d];
}

template<unsigned dim>
double CellGrid<dim>::getCellWidth( int d ) const{
  assert ( d >= 0 && (unsigned)d < dim );
  return cw[d];
}

template<unsigned dim>
int32_t CellGrid<dim>::getCellNum( int d ) const{
  assert ( d >= 0 && (unsigned)d < (dim-1) );
  return no_cells[d];
}

/*
Get the maximum d-coordinate lying on the grid

*/
template<unsigned dim>
double CellGrid<dim>::getMaxCoord( int d ) const{
  assert ( d >= 0 && (unsigned)d < dim );
  if ( d == (dim - 1) ){
    return (p0[d] + cw[d] * no_cells[d]);
  }else{
    return numeric_limits<double>::max();
  }
}

/*
Get the minimum d-coordinate lying on the grid

*/
template<unsigned dim>
double CellGrid<dim>::getMinCoord( int d ) const{
  assert ( d >= 0 && (unsigned)d < dim );
  return getOrigin(d);
}

/*
Returns a coordinate's index on one axis

*/
template<unsigned dim>
int32_t CellGrid<dim>::getIndex(const double& c, const int& d) const
{
  assert ( d >= 0 && (unsigned)d < dim );
  if (this->IsDefined()){
    return static_cast<int32_t>(floor((c - p0[d]) / cw[d]));
  }
  else{
    return getInvalidCellNo();
  }
}


/*
Returns true iff p(dim) is located inside this grid.
p is a dim-dimension point

*/
template<unsigned dim>
bool CellGrid<dim>::onGrid(const double p[dim]) const
{
  if (!this->IsDefined()){
    return false;
  }

  for ( int d = 0; (unsigned)d < dim ; d++)
  {
    int32_t dIndex = getIndex(p[d], d);
    if (!isValidCellIndex(dIndex, d)){
      return false;
    }
  }
  return true;
}

/*
Returns the cell number of a given point p(dim)
For points locate outside the defined grid,
a constant invalid cell number is given,
which better to be set more precisely in the future.

*/
template<unsigned dim>
int32_t CellGrid<dim>::getCellNo(const double p[dim]) const
{
  if (!this->IsDefined()){
    return getInvalidCellNo();
  }
  bool succ = true;
  int32_t cellno = 1;
  for (int d = 0; (unsigned)d < dim; d++)
  {
    int32_t dIndex = getIndex(p[d], d);
    if (!isValidCellIndex(dIndex, d))
      return getInvalidCellNo();
    else
      cellno += ( (d == 0) ? dIndex : dIndex* no_cells[d-1]);
  }
  return cellno;
}

/*
Returns a constant invalid cell number

*/
template<unsigned dim>
int32_t CellGrid<dim>::getInvalidCellNo() const{
  return -555;
}

/*
Get the cell-grid's MBR

*/
template<unsigned dim>
Rectangle<dim> CellGrid<dim>::getMBR() const
{
  if (this->IsDefined())
  {
    double min[dim], max[dim];
    for ( int d = 0; d < dim; d++)
    {
      min[d] = getMinCoord(d);
      max[d] = getMaxCoord(d);
    }
    return Rectangle<dim>(true, min, max);
  } else {
    return Rectangle<dim>(false);
  }
}

/*
Print the cell-grid

*/
template<unsigned dim>
ostream& CellGrid<dim>::Print( ostream &os) const
{
  if ( !this->IsDefined() ){
    return os<< "(CellGrid" << dim << "D: undefined)";
  }

  os << "(CellGrid" << dim << "D: defined, " << endl;
  os << " origin: ( ";
  for (int i=0; (unsigned)i<dim; i++)
    os << p0[i] << " , ";
  os << ") " << endl;

  os << " cell-widths: ( ";
  for (int i=0; (unsigned)i<dim; i++)
    os << cw[i] << " , ";
  os << ") " << endl;

  os << " cell-numbers: ( ";
  for (int i=0; (unsigned)i<dim - 1 ; i++)
    os << no_cells[i] << " , ";
  os << ") )" << endl;

  return os;
}

template<unsigned dim>
size_t CellGrid<dim>::Sizeof() const{
  return sizeof(*this);
}

template<unsigned dim>
int CellGrid<dim>::Compare(const Attribute* other) const{
  const CellGrid<dim>* g = static_cast<const CellGrid<dim>* > (other);
  if (!this->IsDefined()){
    return g->IsDefined()?-1:0;
  }
  if (!g->IsDefined()){
    return 1;
  }

  for (int i = 0; (unsigned)i<dim; i++){
    if (!AlmostEqual(p0[i], g->p0[i])){
      return p0[i]<g->p0[i]?-1:1;
    }
  }

  for (int i = 0; (unsigned)i<(dim - 1); i++){
    if (no_cells[i] != g->no_cells[i]){
      return no_cells[i]<g->no_cells[i]?-1:1;
    }
  }

  for (int i = 0; (unsigned)i<dim; i++){
    if (!AlmostEqual(cw[i], g->cw[i])){
      return cw[i]<g->cw[i]?-1:1;
    }
  }
  return 0;
}

template<unsigned dim>
bool CellGrid<dim>::Adjacent(const Attribute* other) const{
  return false;
}

template<unsigned dim>
Attribute* CellGrid<dim>::Clone() const{
  return new CellGrid<dim>(*this);
}

template<unsigned dim>
size_t CellGrid<dim>::HashValue() const{
  double value = 0.0;
  for (int i = 0; (unsigned)i<dim; i++){
    value += p0[i];
    if ( (unsigned)i < (dim - 1)){
      value += no_cells[i] * cw[i];
    }
  }
  return static_cast<size_t>(value);
}

template<unsigned dim>
void CellGrid<dim>::CopyFrom(const Attribute* other){
  operator=(*(static_cast<const CellGrid<dim>*>(other)));
}

template<unsigned dim>
const string CellGrid<dim>::BasicType(){
  stringstream ss;
  ss << "cellgrid" << dim << "d";
  return ss.str();
}


template<unsigned dim>
ListExpr CellGrid<dim>::Property()
{
  if ( dim == 2 )
  {
    return gentc::GenProperty("-> DATA",
                              BasicType(),
                             "(x0 y0 xw yw n_x)",
                             "(14.0 15.0 2.0 2.0 37)");
  }
  else if ( dim == 3 )
  {
    return gentc::GenProperty("-> DATA",
                              BasicType(),
                             "(x0 y0 t0 xw yw tw n_x n_y)",
                             "(14.0 15.0 16.0 2.0 2.0 2.0 37 38)");
  }
  else
  {
    return gentc::GenProperty("-> DATA",
                              BasicType(),
                             "(unindicated type)",
                             "(unindicated value)");
  }
}

template<unsigned dim>
bool CellGrid<dim>::CheckKind(ListExpr type, ListExpr& errorInfo){
  return nl->IsEqual(type, BasicType());
}

template<unsigned dim>
ListExpr CellGrid<dim>::ToListExpr(const ListExpr typeInfo)const{
  if (!this->IsDefined()){
    return nl->SymbolAtom(Symbol::UNDEFINED());
  } else {
    NList resultList;

    resultList.makeHead(NList(p0[0]));
    for (int i=1; (unsigned)i<dim; i++){
      resultList.append(NList(p0[i]));
    }
    for (int i=0; (unsigned)i<dim; i++){
      resultList.append(NList(cw[i]));
    }
    for (int i=0; (unsigned)i<(dim-1); i++){
      resultList.append(NList(no_cells[i]));
    }

    return resultList.listExpr();
  }
}

template<unsigned dim>
bool CellGrid<dim>::ReadFrom(const ListExpr value,const ListExpr typeInfo){
  if (listutils::isSymbolUndefined(value)){
    this->SetDefined(false);
    return true;
  }

  if (!nl->HasLength(value, (3*dim - 1))){
    return false;
  }

  int pCnt = 0;
  ListExpr rest = value;
  while (!nl->IsEmpty(rest)){
    pCnt++;

    ListExpr lp = nl->First(rest);
    if (!listutils::isNumeric(lp))
      return false;
    if ( (unsigned)pCnt > 2*dim ){
      if (nl->AtomType(lp) != IntType)
        return false;
    }

    if ( (unsigned)pCnt <= dim )
      p0[(pCnt - 1)] = listutils::getNumValue(lp);
    else if ( (unsigned)pCnt > dim && (unsigned)pCnt <= (2*dim))
      cw[(unsigned)(pCnt - dim - 1)] = listutils::getNumValue(lp);
    else
      no_cells[(pCnt - (2*dim) - 1)] = listutils::getNumValue(lp);

    rest = nl->Rest(rest);
  }
  this->SetDefined(true);

  if (!Proper())
  {
    static MessageCenter* msg = MessageCenter::GetInstance();
    NList msgList( NList("simple"),
                   NList("CellGrid built with invalid dimensions!") );
    msg->Send(msgList);
    this->SetDefined(false);
  }

  return true;
}

