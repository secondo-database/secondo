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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]

[1] Implementation of 

October, 2003. Victor Teixeira de Almeida

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

1 Overview

This implementation file contains the implementation of the
the cellnumber operators ~cellnumberG~ and ~cellnumberLine~
in addition to the ~cellnumber~ Operator implemented in die 
RectangleAlgebra


2 Defines and Includes

*/
using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "SpatialAlgebra.h"
#include "RectangleAlgebra.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "ListUtils.h"
#include "Symbols.h"
#include "CellGrid.h"

//#include <stdlib.h>
#include <math.h>
//#include <stdio.h>

extern NestedList* nl;
extern QueryProcessor* qp;

/*
4 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

4.1 Type mapping functions

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

4.1.1 Type mapping function ~cellNumberG~

This operator support 2D grid,
rect x real x real x real x real x int -> stream(int)
The parameter list contains is
rect2, x0, y0, x-width, y-width, nx
rect2 is a 2D rectangle.
Point (x0, y0) is the left-bottom point from where cellnumbering starts.
The whole grid ist not systematically bounded. Numbering is infinit along all
x-, y-axis. Only the implementation of integer numbers in the system
limits the cellnumbers.
NOTE nx ist NOT used and is only implemented for testing an changing 
~cellnumber~ operators in existing operators in a easy way.
This operator also support 3D grid, and the map becomes
rect3 x real x real x real x real x real x real x int x int -> stream(int)
The parameter list of 3D version contains
rect3, x0, y0, z0, x-width, y-width, z-width, nx, ny
rect3 is a 3D rectangle.
Point (x0, y0, z0) is the left-bottom point from where cellnumbering starts. 
The whole grid ist not systematically bounded. Numbering is infinit along all
x-, y- and z-axis. Only the implementation of integer numbers in the system 
limits the cellnumbers.
NOTE nx, ny ist NOT used and is only implemented for testing an changing 
~cellnumber~ operators in existing operators in a easy way.

*/

ListExpr
cellNumberGTM( ListExpr args )
{
  NList l(args);
  string err = "cellnumberG expects(rect, real, real, real, real, int)"
      "or (rect3, real, real, real, real, real, real, int, int) ";

  bool is3D = false;
  int len = l.length();

  if(len==2){ // rect x gridcell2d -> stream(int)
    if(listutils::isSymbol(nl->First(args), Rectangle<2>::BasicType()) &&
       listutils::isSymbol(nl->Second(args), CellGrid2D::BasicType())){
       return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                              nl->SymbolAtom(CcInt::BasicType()));
    }
  }

  if (len == 9){
    is3D = true;
  } else if (len != 6){
    return l.typeError(err);
  }

  NList tRect;
  tRect = l.first();

  if ((is3D && !tRect.isSymbol(Rectangle<3>::BasicType()))
    || (!is3D && !tRect.isSymbol(Rectangle<2>::BasicType())))
    return l.typeError(err);

  int np = is3D ? 2 : 1;
  for (int i = 0; i < np; i++)
  {
    if (!l.elem(len--).isSymbol(CcInt::BasicType()))
      return l.typeError(err);
  }

  for(int i = 2; i <= len; i++)
  {
    if (!l.elem(i).isSymbol(CcReal::BasicType()))
      return l.typeError(err);
  }

  return NList(Symbol::STREAM(), CcInt::BasicType()).listExpr();
}

/*
4.1.1 Type mapping function ~cellNumberLine~

This operator support 2D grid,
rect x real x real x real x real x int -> stream(int)

The parameter list contains is
rect2, x0, y0, x-width, y-width, nx

rect2 is a 2D rectangle.
Point (x0, y0) is middel of a 2D-coordinate system used for the grid.
The whole grid ist not systematically bounded. Numbering is infinit along all
x-, y-axis. Only the implementation of integer numbers in the system
limits the cellnumbers.
NOTE nx ist NOT used and is only implemented for testing an changing 
~cellnumber~ operators in existing operators in a easy way.

This operator also support 3D grid, and the map becomes
rect3 x real x real x real x real x real x real x int x int -> stream(int)

The parameter list of 3D version contains
rect3, x0, y0, z0, x-width, y-width, z-width, nx, ny

rect3 is a 3D rectangle.
Point (x0, y0, z0)is middel of a 2D-coordinate system used for the grid. 
The whole grid ist not systematically bounded. Numbering is infinit along all
x-, y- and z-axis. Only the implementation of integer numbers in the system 
limits the cellnumbers.
NOTE nx, ny ist NOT used and is only implemented for testing an changing 
~cellnumber~ operators in existing operators in a easy way.

*/

ListExpr
cellNumberLineTM( ListExpr args )
{
  NList l(args);
  string err = "cellnumberLine expects(rect, real, real, real, real, int)"
      "or (rect3, real, real, real, real, real, real, int, int) ";

  bool is3D = false;
  int len = l.length();

  if(len==2){ // rect x gridcell2d -> stream(int)
    if(listutils::isSymbol(nl->First(args), Rectangle<2>::BasicType()) &&
       listutils::isSymbol(nl->Second(args), CellGrid2D::BasicType())){
       return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                              nl->SymbolAtom(CcInt::BasicType()));
    }
  }

  if (len == 9){
    is3D = true;
  } else if (len != 6){
    return l.typeError(err);
  }

  NList tRect;
  tRect = l.first();

  if ((is3D && !tRect.isSymbol(Rectangle<3>::BasicType()))
    || (!is3D && !tRect.isSymbol(Rectangle<2>::BasicType())))
    return l.typeError(err);

  int np = is3D ? 2 : 1;
  for (int i = 0; i < np; i++)
  {
    if (!l.elem(len--).isSymbol(CcInt::BasicType()))
      return l.typeError(err);
  }

  for(int i = 2; i <= len; i++)
  {
    if (!l.elem(i).isSymbol(CcReal::BasicType()))
      return l.typeError(err);
  }

  return NList(Symbol::STREAM(), CcInt::BasicType()).listExpr();
}



/*
4.2 Selection functions

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types. It has already been checked by the type mapping 
function that it is applied to correct arguments.

4.2.1 Selection function ~cellnumberG~

Not used, because the ValueMapFunktion covers both cases, 2D and 3D.

4.2.2 Selection function ~cellnumberLine~

Not used, because the ValueMapFunktion covers both cases, 2D and 3D.

4.4.1 Value mapping functions of operator ~cellnumberG~ and ~cellnumberLine~

Build a cell grid around a defined starting point cells are 
numbered infinit until there are not enough integer values 
to enlarge the grid definition again,
and return the number of cells that the rectangle object covers. 
Especially all sectors in a 2D or 3D are used.

4.4.1.1 Auxiliary functions for ~cellnumberG~ 

maxebeneG3D, maxebeneG2D compute how many steps 
the grid has to be enlarged for covering current rectangle 
an determines how far the grid starting point hass to be moved.

*/

int maxebeneG3D (double x, double y, double z){
int maxeb=0;
int i=0;
int power;


if(x<0 || y<0 || z<0)
    {maxeb=maxeb+2;
    x=-x-3;
    y=-y-3;
    z=-z-3;
    }
    while(i==0){
    if(x>0 || y>0 || z>0)
    {maxeb++;
     power=pow(3,maxeb);
     x=x-power;
     y=y-power;
     z=z-power;
    }
    else {i=1;}
    }
return (int) maxeb;
}

int maxebeneG2D (double x, double y){
int maxeb=0;
int i=0;
int power;

if(x<0 || y<0)
    {maxeb=maxeb+2;
     x=-x-3;
     y=-y-3;
    }
while(i==0){
    if(x>0 || y>0)
    {maxeb++;
     power=pow(3,maxeb);
     x=x-power;
     y=y-power;
    }
    else {i=1;}
    }
return (int) maxeb;
}

/*
4.4.1.1 Auxiliary functions for ~cellnumberLine~ 

lb supports the logarithmic computation

*/
double logzwei = log10( 2.0 );

double lb( double x ){
return log10( x ) / logzwei;
}

int maxebeneLine2D (int x, int y){
int maxeb=-1;
int ebenex=0;
int ebeney=0;

if (x == 0){ebenex=0;}
else if (x<0) {ebenex = (int) lb(abs(x));}
else {ebenex = (int) lb(x)+1;}

if (y == 0){ebeney=0;}
else if (y<0) {ebeney = (int) lb(abs(y));}
else {ebeney = (int) lb(y)+1;}

maxeb = max(ebenex,ebeney);

return maxeb;
}

int maxebeneLine3D (int x, int y, int z){
int maxeb=-1;
int ebenex=0;
int ebeney=0;
int ebenez=0;

if (x == 0){ebenex=0;}
else if (x<0){ebenex = (int) lb(abs(x));}
else {ebenex = (int) lb(x)+1;}

if (y == 0){ebeney=0;}
else if (y<0){ebeney = (int) lb(abs(y));}    
else {ebeney = (int) lb(y)+1;}

if (z == 0){ebenez=0;}
else if (z<0){ebenez = (int) lb(abs(z));}
else {ebenez = (int) lb(z)+1;}

maxeb = max(ebenex,max(ebeney,ebenez));

return maxeb;
}


//ValueMappingFunction-CellnumberG
int cellNumberGVM(Word* args, Word& result,
                    int message, Word& local, Supplier s){

struct CellGrid{
  CellGrid(double _x0, double _y0, double _z0,
           double _xw, double _yw, double _zw,
           int _nx, int _ny, bool _3D):
    
    x0(_x0), y0(_y0), z0(_z0),
    xWidth(_xw), yWidth(_yw), zWidth(_zw),
    nx(_nx), ny(_ny), is3D(_3D),
    maxebene(0),
    cx(0), cy(0), cz(0), 
    outGrid(false), finished(false)
  {}


  //Set the MBR of the rectangle in the grid
  void setBoundBox(double lbx, double rtx,
                   double lby, double rty,
                   double lbz = 0.0, double rtz = 0.0)
  {
    //Make sure the widths of the grid all are not 0
    if (   fabs(xWidth - 0.0) <= 1e-10
        || fabs(yWidth - 0.0) <= 1e-10
        || (is3D && fabs(zWidth - 0.0) <= 1e-10))
    {
      cerr << "Unacceptable grid width: " <<
          xWidth << "," << yWidth << "," << zWidth << endl;
      return;
    }

/*
For 2D space,
the cell grid grows infinitely....for a detailed explanaintion of the 
numbering System see the dokumentation 

For 3D space, 
the cell grid grows infinitely...for a detailed explanaintion of the 
numbering System see the dokumentation

*/
//computation of the grid coordinates	
   
power=0;
    
double simLBX=((lbx-x0)/xWidth);
double simLBY=((lby-y0)/yWidth);
double simRTX=((rtx-x0)/xWidth);
double simRTY=((rty-y0)/yWidth);
double simLBZ=((lbz-z0)/zWidth);
double simRTZ=((rtz-z0)/zWidth);	

 
if (is3D) {
maxebene=max(maxebeneG3D(simLBX,simLBY,simLBZ),
              maxebeneG3D(simRTX,simRTY,simRTZ));
}
else {
maxebene=max(maxebeneG2D(simLBX,simLBY),
              maxebeneG2D(simRTX,simRTY));
}
    
//should noch happen!!
if (maxebene==0){cerr <<"WARNING!! Calculation Error!\n";} 

//Make sure all possible cell numbers don't exceed the range
//of the integer type.
if (is3D){
long maxN = 30*pow(27,maxebene-1);
if (maxN>INT_MAX){cerr << "WARNING!! The grid is too dense, "
 "part cell number may exceed the range of Integer type.\n";} 
}
else {
long maxN = 10*pow(9,maxebene-1);
if (maxN>INT_MAX){cerr << "WARNING!! The grid is too dense, "
 "part cell number may exceed the range of Integer type.\n";} 
}

//grid is moved to cover current rectangle
for (int i=1;i<maxebene;i++){
    power=power+pow(3,i);
}

//grid coortinates of left-bottom point and right-top point og the rectangle
//are computed
LBX= int (simLBX+power);
LBY= int (simLBY+power);
RTX= int (simRTX+power);
RTY= int (simRTY+power);
cx=LBX;
cy=LBY;

if (is3D){
LBZ= int (simLBZ+power);
RTZ= int (simRTZ+power);
cz=LBZ;
}

}



//numberG3D compute the cellnumber in a cube of cells
int numberG3D (int bx, int by, int bz, int ebenen){
int cellziff = 0;
cellziff = bx+by;

if (by==2){cellziff=cellziff+4;}
else {
    if (bx==0){cellziff=cellziff+4;}
    else {
        if (by==1) {cellziff=cellziff-2;}
        else {
            if (bx!=2) {cellziff=cellziff+2;}
            }
         }
    }

if (ebenen==1){
    if (bz==2){cellziff=cellziff + 20;}
    else if (bz==0){cellziff=cellziff+10;}
    }
else if (ebenen==2){
    cellziff = cellziff * 30;
    if (bz==2){cellziff=cellziff + 20 * (int)(pow(27,(max(0,ebenen-1))));}
    else if(bz==0){cellziff=cellziff+10*(int)(pow(27,(max(0,ebenen-1))));}
    }
else if (ebenen>2){
    cellziff = cellziff * 30 * (int)(pow(27,(max(0,ebenen-2))));
    if (bz==2){cellziff=cellziff + 20 * (int)(pow(27,(max(0,ebenen-1))));}
    else if(bz==0){cellziff=cellziff+10*(int)(pow(27,(max(0,ebenen-1))));}
    }

return cellziff;
}

//numberG2D compute the cellnumber in a area of cells
int numberG2D (int bx, int by, int ebenen){
int cellziff = 0;
cellziff= bx+by;

if (by==2){cellziff=cellziff+4;}
else {
    if (bx==0){cellziff=cellziff+4;}
    else {
        if (by==1) {cellziff=cellziff-2;}
        else {
            if (bx!=2) {cellziff=cellziff+2;}
        }
    }
}

if (ebenen==2){
cellziff= cellziff*10;
}
else if (ebenen>2){
cellziff = cellziff*10*(int)(pow(9,(max(0,ebenen-2))));
}
return cellziff;

}

//cellnumbering is done stepwise or each step in numbering 
//in the same manner.
//cellnumberG3D controls the different steps.
int cellnumberG3D (int ax, int ay, int az, int ebenen){
double cellnum=0;
for (int r=1;r<ebenen+1;r++){
    cellnum = cellnum+numberG3D((abs(ax))%3,(abs(ay))%3,(abs(az))%3,r);
    ax=ax/3;
    ay=ay/3;
    az=az/3;
}
return (int) cellnum;
}

//cellnumbering is done stepwise or each step in numbering in the same manner.
//cellnumberG2D controls the different steps.
int cellnumberG2D (int ax, int ay, int ebenen){
double cellnum=0;
for (int r=1;r<ebenen+1;r++){
    cellnum = cellnum + numberG2D(((int)abs(ax))%3,((int)abs(ay))%3,r);
    ax=ax/3;
    ay=ay/3;
}
return (int) cellnum;
}
   

//same use as in normal ~cellnumber~ operator. All grid cells 
//covered by a rectangel are located and for each gridcell, 
//the cellnumber ist computed by the above functions

int getNextCellNum()
  { 
    int cellNum = -1;
    if (outGrid)
    {
      cellNum = 0;
      outGrid = false;
    }  
    else if (!finished)
    {
      if ((cx <= RTX) && (cy <= RTY)){
	if (is3D){        
	cellNum = cellnumberG3D(cx,cy,cz,maxebene);
	}
	else {
	cellNum = cellnumberG2D(cx,cy,maxebene);
	}
      }
 
      if (cx < RTX)
        cx++;
      else if (cy < RTY)
      {
        cx = LBX;
        cy++;
      }
      else if (is3D && cz < RTZ )
      {
        cx = LBX;
        cy = LBY;
        cz++;
      }
      else
      {
        finished = true;
      }

    }
    return cellNum;
}


double x0, y0, z0, xWidth, yWidth, zWidth;
int nx, ny; 
bool is3D;
int maxebene, power;
int LBX, LBY, LBZ, RTX, RTY, RTZ; //LB: left-buttom; RT: right-top
int cx, cy, cz; //Current cell coordinate number
bool outGrid;   //Whether the rectangle is outside the given grid
bool finished;

};
	
 
  CellGrid* grid = static_cast<CellGrid*>(local.addr);

  
  switch (message) {
  //START OPEN
  case OPEN: {
    if(grid){
      delete grid;
      grid = 0;
    }

    double x0 = 0.0, y0 = 0.0, z0 = 0.0;
    double xw = 0.0, yw = 0.0, zw = 0.0;
    int nx  = 0, ny = INT_MAX;//????

    int len = qp->GetNoSons(s);
    for(int arg=0; arg<len; arg++){
      if(!(static_cast<Attribute*>(args[arg].addr))->IsDefined()) {
        cerr << "Undefined argument used in cellnumberG." << endl;
        return CANCEL;
      }
    }

    if(len==2){
      Rectangle<2> *rect = (Rectangle<2> *)args[0].addr;
      const CellGrid2D* g = static_cast<CellGrid2D*>(args[1].addr);
      grid = new CellGrid(g->getX0(), g->getY0(), 0.0,
                          g->getXw(), g->getYw(), 0.0,
                          g->getNx(), 0, false);
      grid->setBoundBox(rect->MinD(0), rect->MaxD(0),
                        rect->MinD(1),rect->MaxD(1));

    } 
     
    //der 2D Fall
    else if (6 == len) {
      Rectangle<2> *rect = (Rectangle<2> *)args[0].addr;
      x0 = ((CcReal *)args[1].addr)->GetValue();
      y0 = ((CcReal *)args[2].addr)->GetValue();
      xw = ((CcReal *)args[3].addr)->GetValue();
      yw = ((CcReal *)args[4].addr)->GetValue();
      nx = ((CcInt *)args[5].addr)->GetValue();
      grid = new CellGrid(x0, y0, z0, xw, yw, zw, nx, ny, false);
      grid->setBoundBox(rect->MinD(0), rect->MaxD(0),
                        rect->MinD(1),rect->MaxD(1));
    } 
   
   // der 3D Fall
   else { // len = 9
      Rectangle<3> *rect = (Rectangle<3> *)args[0].addr;
      x0 = ((CcReal *)args[1].addr)->GetValue();
      y0 = ((CcReal *)args[2].addr)->GetValue();
      z0 = ((CcReal *)args[3].addr)->GetValue();
      xw = ((CcReal *)args[4].addr)->GetValue();
      yw = ((CcReal *)args[5].addr)->GetValue();
      zw = ((CcReal *)args[6].addr)->GetValue();
      nx = ((CcInt *)args[7].addr)->GetValue();
      ny = ((CcInt *)args[8].addr)->GetValue();
      grid = new CellGrid(x0, y0, z0, xw, yw, zw, nx, ny, true);
      grid->setBoundBox(rect->MinD(0), rect->MaxD(0),
                        rect->MinD(1), rect->MaxD(1),
                        rect->MinD(2), rect->MaxD(2));
    }


    local.addr = grid;
    return 0;
  }

//END OPEN
//START REQUEST
  
  case REQUEST: {
    if (!grid){
      return CANCEL;
    }
    else {
      int nextCellNum = grid->getNextCellNum();
      if (nextCellNum >= 0)
      {
        CcInt* res = new CcInt(true, nextCellNum);
        result.addr = res;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
  }

  //END REQUEST
  //START CLOSE

  case CLOSE: {
    if (grid != 0){
      delete grid;
      local.addr = 0;
    }
    return 0;
  }
  //END CLOSE

  default: {
    /* should never happen */
    assert(false);
    return -1;
  }

  }//END switch
  

}
//END ValueMapping ~cellnumberG~ CLOSE


//ValueMappingFunction-CellnumberLine

int cellNumberLineVM(Word* args, Word& result,
                    int message, Word& local, Supplier s){

struct CellGrid{
  CellGrid(double _x0, double _y0, double _z0,
           double _xw, double _yw, double _zw,
           int _nx, int _ny, bool _3D):
    
    x0(_x0), y0(_y0), z0(_z0),
    xWidth(_xw), yWidth(_yw), zWidth(_zw),
    nx(_nx), ny(_ny), is3D(_3D),
    maxebene(0),
    cx(0), cy(0), cz(0), 
    outGrid(false), finished(false)
  {}


  //Set the MBR of the rectangle in the grid
  void setBoundBox(double lbx, double rtx,
                   double lby, double rty,
                   double lbz = 0.0, double rtz = 0.0)
  {
    //Make sure the widths of the grid all are not 0
    if (   fabs(xWidth - 0.0) <= 1e-10
        || fabs(yWidth - 0.0) <= 1e-10
        || (is3D && fabs(zWidth - 0.0) <= 1e-10))
    {
      cerr << "Unacceptable grid width: " <<
          xWidth << "," << yWidth << "," << zWidth << endl;
      return;
    }

    double simLBX=((lbx-x0)/xWidth);
    double simLBY=((lby-y0)/yWidth);
    double simRTX=((rtx-x0)/xWidth);
    double simRTY=((rty-y0)/yWidth);
    double simLBZ=((lbz-z0)/zWidth);
    double simRTZ=((rtz-z0)/zWidth);

    //grid coortinates of left-bottom point and right-top point 
    //of the rectangle are computed
    if (simLBX<0){LBX = int (simLBX)-1;}
    else {LBX = int (simLBX);}
    if (simLBY<0){LBY = int (simLBY)-1;}
    else {LBY = int (simLBY);}
    if (simRTX<0){RTX = int (simRTX)-1;}
    else {RTX = int (simRTX);}
    if (simRTY<0){RTY = int (simRTY)-1;}
    else {RTY = int (simRTY);}
    cx=LBX;
    cy=LBY;

    if (is3D){
        if (simLBZ<0){LBZ = int (simLBZ)-1;}
        else {LBZ = int (simLBZ);}
        if (simRTZ<0){RTZ = int (simRTZ)-1;}
        else {RTZ = int (simRTZ);} 
        cz=LBZ;
    }

}
/*
For 2D space,
the cell grid grows infinitely....for a detailed explanaintion of the 
numbering System see the dokumentation
 
*/

int cellnumberLine2D (int ax, int ay){
int cellnumb = 0;

maxebene = maxebeneLine2D(ax,ay);

//Make sure all possible cell numbers don't exceed the range
//of the integer type.
    if (is3D){
    	long maxN = pow(8,maxebene-1);
    	if (maxN>INT_MAX){cerr << "WARNING!! The grid is too dense, "
        	"part cell number may exceed the range of Integer type.\n";} 
	}
    	else {
    	long maxN = pow(4,maxebene-1);
    	if (maxN>INT_MAX){cerr << "WARNING!! The grid is too dense, "
        	"part cell number may exceed the range of Integer type.\n";} 
	}


if (maxebene==0){
        cellnumb = (ax+1) + 2 * (ay+1);
    }
    else {
        int power = pow(2,maxebene+1);
        cellnumb = (ax+power/2) + power*(ay+power/2);
        for (int r=1;r<=maxebene;r++){
            cellnumb = cellnumb + pow(4,r);
        }
    }
    //cout<<cellnumb <<endl;
    return cellnumb;
}

/*
For 3D space, 
the cell grid grows infinitely...for a detailed explanaintion of the 
numbering System see the dokumentation

*/
int cellnumberLine3D (int ax, int ay, int az){
    int cellnumb = 0;

    maxebene = maxebeneLine3D(ax,ay,az);

    //Make sure all possible cell numbers don't exceed the range
    //of the integer type.
    if (is3D){
    	long maxN = pow(8,maxebene-1);
    	if (maxN>INT_MAX){cerr << "WARNING!! The grid is too dense, "
        	"part cell number may exceed the range of Integer type.\n";} 
	}
    	else {
    	long maxN = pow(4,maxebene-1);
    	if (maxN>INT_MAX){cerr << "WARNING!! The grid is too dense, "
        	"part cell number may exceed the range of Integer type.\n";} 
	}

    if (maxebene==0){
        cellnumb = (ax+1) + 2 *(ay+1) + (az+1)*4;
        }

    else if (maxebene==1) {
        int power1 = 4; //pow(2,max_ebene+1);
        int power2 = 16; //pow(power,2);
        int power0 = 2; //pow(2,max_ebene);

        cellnumb = (ax+power1/2) + power1*(ay+power1/2) + (az+power0)*power2;
        for (int r=1;r<=maxebene;r++){
            cellnumb = cellnumb + pow(8,r);
        }
    }

    else if (maxebene==2)  {
        int power1 = 8; //pow(2,max_ebene+1);
        int power2 = 64; //pow(power,2);
        int power0 = 4; //pow(2,max_ebene);

        cellnumb=(ax+power1/2)+power1*(ay+power1/2)+((az)+power0)*power2;
        for (int r=1;r<=maxebene;r++){
            cellnumb = cellnumb + pow(8,r);
        }
    }

    else if (maxebene>2)  {
        int power1 = pow(2,maxebene+1);
        int power2 = pow(power1,2);
        int power0 = pow(2,maxebene);

        cellnumb=(ax+power1/2)+power1*(ay+power1/2)+((az)+power0)*power2;
        for (int r=1;r<=maxebene;r++){
            cellnumb = cellnumb + pow(8,r);
        }
    }
    //cout<<cellnumb <<endl;
    return cellnumb;
}


//same use as in normal ~cellnumber~ operator. All grid cells 
//covered by a rectangel are located and for each gridcell, 
//the cellnumber ist computed by the above functions

int getNextCellNum()
  { 
    int cellNum = -1;
    if (outGrid)
    {
      cellNum = 0;
      outGrid = false;
    }  
    else if (!finished)
    {
      if ((cx <= RTX) && (cy <= RTY)){
	if (is3D){       
	cellNum = cellnumberLine3D(cx,cy,cz);
	}
	else {
	cellNum = cellnumberLine2D(cx,cy);
	}
      }
 
      if (cx < RTX)
        cx++;
      else if (cy < RTY)
      {
        cx = LBX;
        cy++;
      }
      else if (is3D && cz < RTZ )
      {
        cx = LBX;
        cy = LBY;
        cz++;
      }
      else
      {
        finished = true;
      }

    }
    return cellNum;
}


double x0, y0, z0, xWidth, yWidth, zWidth;
int nx, ny; 
bool is3D;
int maxebene, power;
int LBX, LBY, LBZ, RTX, RTY, RTZ; //LB: left-buttom; RT: right-top
int cx, cy, cz; //Current cell coordinate number
bool outGrid;   //Whether the rectangle is outside the given grid
bool finished;

};
	
 
  CellGrid* grid = static_cast<CellGrid*>(local.addr);

  
  switch (message) {
  //START OPEN
  case OPEN: {
    if(grid){
      delete grid;
      grid = 0;
    }

    double x0 = 0.0, y0 = 0.0, z0 = 0.0;
    double xw = 0.0, yw = 0.0, zw = 0.0;
    int nx  = 0, ny = INT_MAX;//????

    int len = qp->GetNoSons(s);
    for(int arg=0; arg<len; arg++){
      if(!(static_cast<Attribute*>(args[arg].addr))->IsDefined()) {
        cerr << "Undefined argument used in cellnumberLine." << endl;
        return CANCEL;
      }
    }

    if(len==2){
      Rectangle<2> *rect = (Rectangle<2> *)args[0].addr;
      const CellGrid2D* g = static_cast<CellGrid2D*>(args[1].addr);
      grid = new CellGrid(g->getX0(), g->getY0(), 0.0,
                          g->getXw(), g->getYw(), 0.0,
                          g->getNx(), 0, false);
      grid->setBoundBox(rect->MinD(0), rect->MaxD(0),
                        rect->MinD(1),rect->MaxD(1));

    } 
     
    //der 2D Fall
    else if (6 == len) {
      Rectangle<2> *rect = (Rectangle<2> *)args[0].addr;
      x0 = ((CcReal *)args[1].addr)->GetValue();
      y0 = ((CcReal *)args[2].addr)->GetValue();
      xw = ((CcReal *)args[3].addr)->GetValue();
      yw = ((CcReal *)args[4].addr)->GetValue();
      nx = ((CcInt *)args[5].addr)->GetValue();
      grid = new CellGrid(x0, y0, z0, xw, yw, zw, nx, ny, false);
      grid->setBoundBox(rect->MinD(0), rect->MaxD(0),
                        rect->MinD(1),rect->MaxD(1));
    } 
   
   // der 3D Fall
   else { // len = 9
      Rectangle<3> *rect = (Rectangle<3> *)args[0].addr;
      x0 = ((CcReal *)args[1].addr)->GetValue();
      y0 = ((CcReal *)args[2].addr)->GetValue();
      z0 = ((CcReal *)args[3].addr)->GetValue();
      xw = ((CcReal *)args[4].addr)->GetValue();
      yw = ((CcReal *)args[5].addr)->GetValue();
      zw = ((CcReal *)args[6].addr)->GetValue();
      nx = ((CcInt *)args[7].addr)->GetValue();
      ny = ((CcInt *)args[8].addr)->GetValue();
      grid = new CellGrid(x0, y0, z0, xw, yw, zw, nx, ny, true);
      grid->setBoundBox(rect->MinD(0), rect->MaxD(0),
                        rect->MinD(1), rect->MaxD(1),
                        rect->MinD(2), rect->MaxD(2));
    }


    local.addr = grid;
    return 0;
  }

//END OPEN
//START REQUEST
  
  case REQUEST: {
    if (!grid){
      return CANCEL;
    }
    else {
      int nextCellNum = grid->getNextCellNum();
      if (nextCellNum >= 0)
      {
        CcInt* res = new CcInt(true, nextCellNum);
        result.addr = res;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
  }

  //END REQUEST
  //START CLOSE

  case CLOSE: {
    if (grid != 0){
      delete grid;
      local.addr = 0;
    }
    return 0;
  }//END CLOSE

  default: {
    /* should never happen */
    assert(false);
    return -1;
  }

  }//END switch

}//END CLOSE
  


/*
4.5.3 Definition of the operators

*/

struct cellnumberG_Info : OperatorInfo {

  cellnumberG_Info() : OperatorInfo()
  {
    name = "cellnumberG";
    signature =
        "rect x real x real x real x real x int -> stream(int)\n"
        "rect3 x real x real x real x real x real x real x int x int"
        "-> stream(int)";
    syntax = "cellnumber( box, x0, y0, [z0,] wx, wy, [wz,] nx, [ny] )";
    meaning = "Returns a stream of numbers of all cells intersected by box "
        "with respect to a special 2D-[3D-] G-grid" 
	"starting at (x0,y0 [,z0]) and--- for positive widths ---"
	"extending to to all quadrants (octants). "
        "Each cell has widths wx, wy [and wz]."
	"nx and ny are not used, as to the fact the Grid ist unbounded in"
	"any direction." 
	"The grid is unbounded and cell numbering follows a special"
	"G-curve form"
        "starting in a seed cell in the positiv sector relative" 
	"to the starting point at (x0,y0 [,z0])."
	;
  }

};

struct cellnumberLine_Info : OperatorInfo {

  cellnumberLine_Info() : OperatorInfo()
  {
    name = "cellnumberLine";
    signature =
        "rect x real x real x real x real x int -> stream(int)\n"
        "rect3 x real x real x real x real x real x real x int x int"
        "-> stream(int)";
    syntax = "cellnumberLine( box, x0, y0, [z0,] wx, wy, [wz,] [nx], [ny] )";
    meaning = "Returns a stream of numbers of all cells intersected by box "
        "with respect to a special 2D-[3D-] G-grid" 
	"starting at (x0,y0 [,z0]) and --- for positive widths ---"
	"extending to to all quadrants (octants). "
        "Each cell has widths wx, wy [and wz]." 
	"nx and ny are not used, as to the fact the Grid ist unbounded in"
	"any direction."
	"The grid is unbounded and cell numbering follows a special"
	"cube extending manner starting with a minimum of 4 (8) cells, "
        "one for each relevant Sector of the coordinate system." 
	"Starting point is (x0,y0 [,z0]) as the center point of the"
	"coordinate system."
	;
  }

};

/*
5 Creating the Algebra

*/

class SpatialJoin2Algebra : public Algebra
{
 public:
  SpatialJoin2Algebra() : Algebra()
  {
    AddOperator(cellnumberLine_Info(), cellNumberLineVM, cellNumberLineTM);
    AddOperator(cellnumberG_Info(), cellNumberGVM, cellNumberGTM);
  }
  ~SpatialJoin2Algebra() {};
};

/*
6 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeSpatialJoin2Algebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new SpatialJoin2Algebra());
}

