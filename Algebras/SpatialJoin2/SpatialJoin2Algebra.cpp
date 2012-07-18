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
#include "MMRTreeAlgebra.h"
#include "MMRTree.h"

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <map>
#include <utility>

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
4.1.1 Type mapping function ~cellNumberZ~

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
cellNumberZTM( ListExpr args )
{
  NList l(args);
  string err = "cellnumberZ expects(rect, real, real, real, real, int)"
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

4.2.3 Selection function ~cellnumberZ~

Not used, because the ValueMapFunktion covers both cases, 2D and 3D.


4.4.1 Value mapping functions of operator ~cellnumberG~, ~cellnumberLine~ 
and ~cellnumberZ~

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

//cout << "hier?" << endl;
if(x<0 || y<0)
    {maxeb=maxeb+2;
     x=-x-3;
     y=-y-3;
    }
while(i==0){//cout << "hier??->i=" << i << "|" << x << "/" << y << endl;
    if(x>0 || y>0)
    {maxeb++;
     power=pow(3,maxeb);
     x=x-power;
     y=y-power;
    }
    else {i=1;}
    }
//cout << "hier???" << endl;
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

if (abs(x)<1) {ebenex = 0;}
else {ebenex = (int) lb(abs(x))+1;}

if (abs(y)<1) {ebeney = 0;}
else {ebeney = (int) lb(abs(y))+1;}

maxeb = max(ebenex,ebeney);

return maxeb;
}

int maxebeneLine3D (int x, int y, int z){
int maxeb=-1;
int ebenex=0;
int ebeney=0;
int ebenez=0;

if (abs(x)<1){ebenex = 0;}
else {ebenex = (int) lb(abs(x))+1;}

if (abs(y)<1){ebeney = 0;}    
else {ebeney = (int) lb(abs(y))+1;}

if (abs(z)<1){ebenez = 0;}
else {ebenez = (int) lb(abs(z))+1;}

maxeb = max(ebenex,max(ebeney,ebenez));

return maxeb;
}

/*
4.4.1.1 Auxiliary functions for ~cellnumberZ~ 

maxebeneZ3D, maxebeneZ2D compute how many steps 
the grid has to be enlarged for covering current rectangle 
an determines how far the grid starting point hass to be moved.

*/

int maxebeneZ2D (double x, double y){
int maxeb=-1;
int ebenex=0;
int ebeney=0;

if (abs(x)<1){ebenex=1;}
else {ebenex = (int) lb(abs(x))+2;}
if (abs(y)<1){ebeney=1;}
else {ebeney = (int) lb(abs(y))+2;}

maxeb = max(ebenex,ebeney);

return maxeb;
}

int maxebeneZ3D (double  x, double y, double z){
int maxeb=-1;
int ebenex=0;
int ebeney=0;
int ebenez=0;

if (abs(x)<1){ebenex=1;}
else {ebenex = (int) lb(abs(x))+2;}
if (abs(y)<1){ebeney=1;}
else {ebeney = (int) lb(abs(y))+2;}
if (abs(z)<1){ebenez=1;}
else {ebenez = (int) lb(abs(z))+2;}

maxeb = max(ebenex,max(ebeney,ebenez));

return maxeb;
}

//ValueMappingFunction-CellnumberG
int cellNumberGVM(Word* args,Word& result,int message,Word& local,Supplier s){

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
//spaces for hirachical numbering are defined from input via parameter nx
hierachicalBorder=0;
hierachicalSteps=0;
if (nx>=1000 && nx<10000){
  cerr << "WARNING!! To use hiracical numbering the " 
 "parameter has to be > 9999 "
 "as otherwise not all values can be defined. \n";
}
else {hierachicalSteps=nx/1000;
      hierachicalBorder=hierachicalSteps/10;
      hierachicalSteps=hierachicalSteps%10;
      nx=nx%1000;
}                

   
if (nx==0){layerSpacing=0;
           initialSpacing=0;
           cubeSpacing=0;}
else if (nx<10 && nx>0){initialSpacing=nx;
                        layerSpacing=0;
                        cubeSpacing=0;}
else if (nx>=10 && nx<100){initialSpacing=nx%10;
                           layerSpacing=(nx%100-nx%10)/10;
                           cubeSpacing=(int)nx/100;}
else if (nx>99 && nx<1000){initialSpacing=nx%10;
                           layerSpacing=(nx%100-nx%10)/10;
                           cubeSpacing=(int)nx/100;}
else {layerSpacing=0;
      initialSpacing=0;
      cubeSpacing=0;} 


if (hierachicalBorder>0 && initialSpacing==0){
  cerr << "WARNING!! To use hiracical numbering initilSpacing, "
 "has to be at least 1.\n";
}
if (is3D == false && hierachicalSteps>layerSpacing){
      cerr << "WARNING!! hirachicalSteps need to have  "
 "layerSpacing of at least same number! \n";
}
if (is3D == true && hierachicalSteps>cubeSpacing){
   cerr << "WARNING!! hirachicalSteps need to have  "
 "cubeSayerspacing of at least same number! \n";
}
       
    
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
    
//should not happen!!
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

//grid coordinates of left-bottom point and right-top point of the rectangle
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
int numberG3D (int bx, int by, int bz, int ebenen, int dim2,  int cube){
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
    if (bz==2){cellziff=cellziff + 2*dim2;}
    else if (bz==0){cellziff=cellziff+1*dim2;}
    }
else if (ebenen==2){
    cellziff = cellziff*(3*dim2+cube);
    if (bz==2){cellziff=cellziff+(2*dim2)*27+(2*9*cube);}
    else if(bz==0){cellziff=cellziff+(1*dim2)*27+(1*9*cube);}
    }
else if (ebenen>2){
    cellziff = cellziff*((3*dim2)*
            (int)(pow(27,(max(0,ebenen-2))))+
            (cube*(int)(pow(27,(max(0,ebenen-2))))));
    if (bz==2){cellziff=cellziff+2*dim2*
            (int)(pow(27,(max(0,ebenen-1))))+
            (2*9*(int)(pow(27,(max(0,ebenen-2))))*cube);}
    else if(bz==0){cellziff=cellziff+1*dim2*
            (int)(pow(27,(max(0,ebenen-1))))+
            (1*9*(int)(pow(27,(max(0,ebenen-2))))*cube);}
    }

return cellziff;
}

//numberG2D compute the cellnumber in a area of cells
int numberG2D (int bx, int by, int ebenen, int twodim){
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
cellziff= cellziff*twodim;
}
else if (ebenen>2){
cellziff = cellziff*twodim*(int)(pow(9,(max(0,ebenen-2))));
}
return cellziff;

}

//cellnumbering is done stepwise or each step in numbering 
//in the same manner.
//cellnumberG3D controls the different steps.
int cellnumberG3D (int ax, int ay, int az, int ebenen){
double cellnum=0;
int dim2effLayerSpacing=9+layerSpacing;

for (int r=1;r<ebenen+1;r++){
    cellnum = cellnum+numberG3D((abs(ax))%3,(abs(ay))%3,
            (abs(az))%3,r,dim2effLayerSpacing,cubeSpacing);
    ax=ax/3;
    ay=ay/3;
    az=az/3;
}
return (int) cellnum+initialSpacing;
}

//cellnumbering is done stepwise or each step in numbering in the same manner.
//cellnumberG2D controls the different steps.
int cellnumberG2D (int ax, int ay, int ebenen){
double cellnum=0;
int dim2effLayerSpacing=9+layerSpacing;

for (int r=1;r<ebenen+1;r++){
    cellnum = cellnum + numberG2D(((int)abs(ax))%3,
            ((int)abs(ay))%3,r,dim2effLayerSpacing);
    ax=ax/3;
    ay=ay/3;
}
return (int) cellnum+initialSpacing;
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
    //test+='/';
    //test+=convertInt(cellNum);
    //cout << test << endl;
    
    if ((is3D==false &&  hierachicalBorder>0  && cellNum>=0 &&
            (RTX-LBX>=hierachicalBorder || RTY-LBY>=hierachicalBorder))
    || (is3D==true &&  hierachicalBorder>0  && cellNum>=0 && 
            (RTX-LBX>=hierachicalBorder || 
            RTY-LBY>=hierachicalBorder || 
            RTZ-LBZ>=hierachicalBorder))){
    step=1; 
    while ((RTX-LBX>hierachicalBorder*step || 
            RTY-LBY>hierachicalBorder*step || 
            RTZ-LBZ>hierachicalBorder*step))
    {step=step*3;}
    step=min(step/3,hierachicalSteps);
    lsp=1;
    if (is3D){lsp=3;}
     
    cellNum=(cellNum-initialSpacing)/
            (pow(9*lsp,step)+pow(9*lsp,step-1)*
            layerSpacing*lsp+cubeSpacing);
    cellNum=(cellNum+1)*(pow(9*lsp,step)+pow(9*lsp,step-1)*
            layerSpacing*lsp+cubeSpacing);
    cellNum=cellNum+initialSpacing;
             
    if (cellMap.size()==0){
        cellMap.insert(MapType::value_type(cellNum, cellNum));}
    else {cellMapIter=cellMap.find(cellNum);
        if (cellMapIter == cellMap.end()){
            cellMap.insert(MapType::value_type(cellNum, cellNum));
        } 
        else {cellNum=0;}      
    }
    }
    cellMapIter = cellMap.begin();
    while (cellMapIter != cellMap.end()){
    cout << "Value is: " << (*cellMapIter).first << endl;
        cellMapIter++;
    
    }
    
    return cellNum;
}

string convertInt(int number)
{
    if (number == 0)
        return "0";
    string temp="";
    string returnvalue="";
    while (number>0)
    {
        temp+=number%10+48;
        number/=10;
    }
    for (int i=0;i<temp.length();i++)
        returnvalue+=temp[temp.length()-i-1];
    return returnvalue;
}

string test;
double x0, y0, z0, xWidth, yWidth, zWidth;
int nx, ny; 
bool is3D;
int maxebene, power;
int LBX, LBY, LBZ, RTX, RTY, RTZ; //LB: left-buttom; RT: right-top
int cx, cy, cz; //Current cell coordinate number
bool outGrid;   //Whether the rectangle is outside the given grid
bool finished;
int cubeSpacing;  
int initialSpacing;
int layerSpacing;
int hierachicalBorder;
int hierachicalSteps;
int step;
int lsp;
typedef std::map<int, int> MapType;
MapType cellMap;
MapType::iterator cellMapIter;
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

    
    
if (is3D) {
maxebene=max(maxebeneZ3D(simLBX,simLBY,simLBZ),
              maxebeneZ3D(simRTX,simRTY,simRTZ));
}
else {
maxebene=max(maxebeneZ2D(simLBX,simLBY),
              maxebeneZ2D(simRTX,simRTY));
}
maxxyz=pow(2,maxebene);


//grid coortinates of left-bottom point and right-top point 
//of the rectangle are computed
if (simLBX<0){LBX = (int)(simLBX-1)+(int)(maxxyz/2);}
else {LBX = (int)(simLBX)+(int)(maxxyz/2);}
if (simLBY<0){LBY = (int)(simLBY-1)+(int)(maxxyz/2);}
else {LBY = (int)(simLBY)+(int)(maxxyz/2);}
if (simRTX<0){RTX = (int)(simRTX-1)+(int)(maxxyz/2);}
else {RTX = (int)(simRTX)+(int) (maxxyz/2);}
if (simRTY<0){RTY = (int)(simRTY-1)+(int)(maxxyz/2);}
else {RTY = (int)(simRTY)+(int)(maxxyz/2);}
cx=LBX;
cy=LBY;

if (is3D){
    if (simLBZ<0){LBZ = (int)(simLBZ-1)+(int)(maxxyz/2);}
    else {LBZ = (int)(simLBZ)+(int)(maxxyz/2);}
    if (simRTZ<0){RTZ = (int)simRTZ+(int)(maxxyz/2);}
    else {RTZ = (int)(simRTZ)+(int)(maxxyz/2);} 
    cz=LBZ;
}

}
/*
For 2D space,
the cell grid grows infinitely....for a detailed explanaintion of the 
numbering System see the dokumentation
 
*/

int cellnumberLine2D (int ax, int ay, int maxEbene){
int cellnumb = 0;

if (maxEbene==0){
        cellnumb = (ax) + 2 * (ay);
    }
    else {
        int power = pow(2,maxEbene+1);
        cellnumb = ax + power*ay;
        for (int r=1;r<=maxEbene;r++){
            cellnumb = cellnumb + pow(4,r);
        }
    }
    return cellnumb;
}

/*
For 3D space, 
the cell grid grows infinitely...for a detailed explanaintion of the 
numbering System see the dokumentation

*/
int cellnumberLine3D (int ax, int ay, int az, int maxEbene){
int cellnumb = 0;

    if (maxEbene==0){
        cellnumb = ax + 2 *ay + 4*az;
    }

    else if (maxEbene>=1) {
        int power1 = pow(2,maxEbene+1); //4;
        int power2 = pow(power1,2); //16;

        cellnumb = ax + power1*ay + power2*az;
        for (int r=1;r<=maxEbene;r++){
            cellnumb = cellnumb + pow(8,r);
        }
    }

    
    return cellnumb;
}


//same use as in normal ~cellnumber~ operator. All grid cells 
//covered by a rectangel are located and for each gridcell, 
//the cellnumber ist computed by the above functions

int getNextCellNum()
  { double epsilon=0.0000003;
  int maxEbene=-1;
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
        maxEbene=maxebeneLine3D(cx-((maxxyz-epsilon)/2),
                cy-((maxxyz-epsilon)/2),
                cz-((maxxyz-epsilon)/2));
        cellNum = cellnumberLine3D(
                cx-(maxxyz/2)+(int)pow(2,maxEbene),
                cy-(maxxyz/2)+(int)pow(2,maxEbene),
                cz-(maxxyz/2)+(int)pow(2,maxEbene),maxEbene);
        }
       else {
        maxEbene=maxebeneLine2D(cx-((maxxyz-epsilon)/2),
                cy-((maxxyz-epsilon)/2));
        cellNum = cellnumberLine2D(
                cx-(maxxyz/2)+(int)pow(2,maxEbene),
                cy-(maxxyz/2)+(int)pow(2,maxEbene),maxEbene);
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
int maxebene, power, maxxyz;
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
    
     
    //der 2D Fall
   if (6 == len) {
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
  

//ValueMappingFunction-CellnumberZ
int cellNumberZVM(Word* args, Word& result,
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
//spaces for hirachical numbering are defined from input via parameter nx
hierachicalBorder=0;
hierachicalSteps=0;
if (nx>=1000 && nx<10000){
  cerr << "WARNING!! To use hiracical numbering "
          "the parameter has to be > 9999 "
 "as otherwise not all values can be defined. \n";
}
else {hierachicalSteps=nx/1000;
      hierachicalBorder=hierachicalSteps/10;
      hierachicalSteps=hierachicalSteps%10; 
      nx=nx%1000;
}                

   
if (nx==0){layerSpacing=0;
           initialSpacing=0;
           cubeSpacing=0;}
else if (nx<10 && nx>0){initialSpacing=nx;
                        layerSpacing=0;
                        cubeSpacing=0;}
else if (nx>=10 && nx<100){initialSpacing=nx%10;
                           layerSpacing=(nx%100-nx%10)/10;
                           cubeSpacing=(int)nx/100;}
else if (nx>99 && nx<1000){initialSpacing=nx%10;
                           layerSpacing=(nx%100-nx%10)/10;
                           cubeSpacing=(int)nx/100;}
else {layerSpacing=0;
      initialSpacing=0;
      cubeSpacing=0;} 


if (hierachicalBorder>0 && initialSpacing==0){
  cerr << "WARNING!! To use hiracical numbering initialSpacing, "
 "has to be at least 1.\n";
}
if (hierachicalSteps>1){
      cerr << "WARNING!! hierachicalSteps > 1 is not meaningful defined in "
 "Cellnumber-Z! Only one step possible. hirachicalSteps forced to 1!!  \n";
 hierachicalSteps=1;     
}
if (is3D == false && hierachicalSteps>layerSpacing){
      cerr << "WARNING!! hirachicalSteps need to have  "
 "layerSpacing of at least same number! \n";
}
if (is3D == true && hierachicalSteps>cubeSpacing){
   cerr << "WARNING!! hirachicalSteps need to have  "
 "cubeSpacing of at least same number! \n";
}    
    
//computation of the grid coordinates
power=0;
   
double simLBX=((lbx-x0)/xWidth);
double simLBY=((lby-y0)/yWidth);
double simRTX=((rtx-x0)/xWidth);
double simRTY=((rty-y0)/yWidth);
double simLBZ=((lbz-z0)/zWidth);
double simRTZ=((rtz-z0)/zWidth);

cout << simLBX << endl;
cout << simLBY << endl; 
cout << simRTX << endl;
cout << simRTY << endl;
cout << simLBZ << endl;
cout << simRTZ << endl;

if (is3D) {
maxebene=max(maxebeneZ3D(simLBX,simLBY,simLBZ),
              maxebeneZ3D(simRTX,simRTY,simRTZ));
}
else {
maxebene=max(maxebeneZ2D(simLBX,simLBY),
              maxebeneZ2D(simRTX,simRTY));
}
maxxyz=pow(2,maxebene);

//should not happen!!
if (maxebene==0){cerr <<"WARNING!! Calculation Error!\n";} 

//Make sure all possible cell numbers don't exceed the range
//of the integer type.
if (is3D){
long maxN = pow(8,maxebene);
if (maxN>INT_MAX){cerr << "WARNING!! The grid is too dense, "
 "part cell number may exceed the range of Integer type.\n";} 
}
else {
long maxN = pow(4,maxebene);
if (maxN>INT_MAX){cerr << "WARNING!! The grid is too dense, "
 "part cell number may exceed the range of Integer type.\n";} 
}


//grid coortinates of left-bottom point and right-top point og the rectangle
//are computed
if (simLBX<0){LBX = (int)(simLBX-1)+(int)(maxxyz/2);}
else {LBX = (int)(simLBX)+(int)(maxxyz/2);}
if (simLBY<0){LBY = (int)(simLBY-1)+(int)(maxxyz/2);}
else {LBY = (int)(simLBY)+(int)(maxxyz/2);}
if (simRTX<0){RTX = (int)(simRTX-1)+(int)(maxxyz/2);}
else {RTX = (int)(simRTX)+(int) (maxxyz/2);}
if (simRTY<0){RTY = (int)(simRTY-1)+(int)(maxxyz/2);}
else {RTY = (int)(simRTY)+(int)(maxxyz/2);}
cx=LBX;
cy=LBY;

if (is3D){
    if (simLBZ<0){LBZ = (int)(simLBZ-1)+(int)(maxxyz/2);}
    else {LBZ = (int)(simLBZ)+(int)(maxxyz/2);}
    if (simRTZ<0){RTZ = (int)simRTZ+(int)(maxxyz/2);}
    else {RTZ = (int)(simRTZ)+(int)(maxxyz/2);} 
    cz=LBZ;
}
 

}

//numberZ3D compute the cellnumber in a cube of cells
int numberZ3D (int bx, int by, int bz, int ebenen, int dim3){
int cellziff = 0;

cellziff = bx+by;

if (ebenen==1){
    if (by==1){cellziff=cellziff+1;}
    if (bz==1){cellziff=cellziff+4;}
    }
else if (ebenen==2){
        if (by==1){cellziff=cellziff+1;}
        if (bz==1){cellziff=cellziff+4;}
        cellziff=cellziff*(dim3); 
    }
else if (ebenen>2){
        if (by==1){cellziff=cellziff+1;}
        if (bz==1){cellziff=cellziff+4;}
        cellziff=cellziff*(dim3)*
                (int)(pow(8,(max(0,ebenen-2)))); 

    }

return cellziff;
}

//numberZ2D compute the cellnumber in a area of cells
int numberZ2D (int bx, int by, int ebenen, int dim2){
int cellziff = 0;
cellziff= bx+by;

if (by==1){cellziff=cellziff+1;}

if (ebenen==2){
        cellziff=cellziff*dim2; 
    }
else if (ebenen>2){
        cellziff = cellziff*dim2*(int)(pow(4,(max(0,ebenen-2)))); 
    }

return cellziff;

}

//cellnumbering is done stepwise or each step in numbering 
//in the same manner.
//cellnumberZ3D controls the different steps.
int cellnumberZ3D (int ax, int ay, int az, int ebenen){
double cellnum=0;
double offset=0;
int dim3effLayerSpacing=2*(4+layerSpacing)+cubeSpacing;

for (int r=1;r<ebenen+1;r++){
    cellnum = cellnum+numberZ3D((abs(ax))%2,
            (abs(ay))%2,(abs(az))%2,r,
            dim3effLayerSpacing);
    if ((int)(abs(az)%2)==1 && r==1){offset=layerSpacing;}
    ax=ax/2;
    ay=ay/2;
    az=az/2;
}
if (ebenen==1){
        offset=0;
    }
else {for (int r=1;r<ebenen;r++){
        offset=offset+pow(8,r);
        offset=offset+(pow(8,r-1)*cubeSpacing);
        offset=offset+(pow(8,r)*layerSpacing/4);
        }
    }
cellnum=cellnum+offset;
cellnum=cellnum+initialSpacing;
return (int) cellnum;

}

//cellnumbering is done stepwise or each step in numbering in the same manner.
//cellnumberZ2D controls the different steps.
int cellnumberZ2D (int ax, int ay, int ebenen){
double cellnum=0;
double offset=0;
double dim2effLayerSpacing=4+layerSpacing;

for (int r=1;r<ebenen+1;r++){
    cellnum = cellnum + numberZ2D(((int)abs(ax))%2,
            ((int)abs(ay))%2,r,
            dim2effLayerSpacing);
    ax=ax/2;
    ay=ay/2;
}
if (ebenen==1){
        offset=0;
    }
else {for (int r=1;r<=ebenen-1;r++){
        offset=offset+pow(4,r);
        offset=offset+layerSpacing*pow(4,r-1);
        }
    }
cellnum=cellnum+offset;
cellnum=cellnum+initialSpacing;

return (int) cellnum;
}
   

//same use as in normal ~cellnumber~ operator. All grid cells 
//covered by a rectangel are located and for each gridcell, 
//the cellnumber ist computed by the above functions

int getNextCellNum()
  { int maxEbene= -1;
    int cellNum = -1;
    double epsilon=0.0000003;
    
    if (outGrid)
    {
      cellNum = 0;
      outGrid = false;
    }  
    else if (!finished)
    {
      if ((cx <= RTX) && (cy <= RTY)){
        maxEbene=-1;
        if (is3D){  
        maxEbene=maxebeneZ3D(cx-((maxxyz-epsilon)/2),
                cy-((maxxyz-epsilon)/2),
                cz-((maxxyz-epsilon)/2)); 
        cellNum = cellnumberZ3D(
                cx-((maxxyz-epsilon)/2)+(int)pow(2,maxEbene-1),
                cy-((maxxyz-epsilon)/2)+(int)pow(2,maxEbene-1),
                cz-((maxxyz-epsilon)/2)+(int)pow(2,maxEbene-1),maxEbene);
        }
        else {
            
        maxEbene=maxebeneZ2D(cx-((maxxyz-epsilon)/2),
                cy-((maxxyz-epsilon)/2));
        cellNum = cellnumberZ2D(
                cx-((maxxyz-epsilon)/2)+(int)pow(2,maxEbene-1),
                cy-((maxxyz-epsilon)/2)+(int)pow(2,maxEbene-1),maxEbene);
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
    //test+='/';
    //test+=convertInt(cellNum);
    //cout << test << endl;
    
    
    if ((is3D==false &&  hierachicalBorder>0  && cellNum>=0 &&
            (RTX-LBX>=hierachicalBorder || 
            RTY-LBY>=hierachicalBorder))
    || (is3D==true &&  hierachicalBorder>0  && cellNum>=0 && 
            (RTX-LBX>=hierachicalBorder || 
            RTY-LBY>=hierachicalBorder || 
            RTZ-LBZ>=hierachicalBorder))){
    step=1; 
    while ((RTX-LBX>hierachicalBorder*step || 
            RTY-LBY>hierachicalBorder*step || 
            RTZ-LBZ>hierachicalBorder*step))
    {step=step*2;}
    step=min(step/2,hierachicalSteps);
    lsp=1;
    if (is3D){lsp=2;}
    
    cellNum=(cellNum-initialSpacing)/
            (pow(4*lsp,step)+pow(4*lsp,step-1)*
            layerSpacing*lsp+cubeSpacing);
    cellNum=(cellNum+1)*(pow(4*lsp,step)+pow(4*lsp,step-1)*
            layerSpacing*lsp+cubeSpacing);
    cellNum=cellNum+initialSpacing;
            
    if (cellMap.size()==0){
        cellMap.insert(MapType::value_type(cellNum, cellNum));}
    else {cellMapIter=cellMap.find(cellNum);
        if (cellMapIter == cellMap.end()){
            cellMap.insert(MapType::value_type(cellNum, cellNum));
        } 
        else {cellNum=0;}      
    }
    }
    cellMapIter = cellMap.begin();
    while (cellMapIter != cellMap.end()){
    cout << "Value is: " << (*cellMapIter).first << endl;
        cellMapIter++;
    
    }
    
    return cellNum;
}

string convertInt(int number)
{
    if (number == 0)
        return "0";
    string temp="";
    string returnvalue="";
    while (number>0)
    {
        temp+=number%10+48;
        number/=10;
    }
    for (int i=0;i<temp.length();i++)
        returnvalue+=temp[temp.length()-i-1];
    return returnvalue;
}

string test;
double x0, y0, z0, xWidth, yWidth, zWidth;
int nx, ny; 
bool is3D;
int maxebene, power, maxxyz;
int LBX, LBY, LBZ, RTX, RTY, RTZ; //LB: left-buttom; RT: right-top
int cx, cy, cz; //Current cell coordinate number
bool outGrid;   //Whether the rectangle is outside the given grid
bool finished;
int cubeSpacing;  
int initialSpacing;
int layerSpacing;
int hierachicalBorder;
int hierachicalSteps;
int step;
int lsp;
typedef std::map<int, int> MapType;
MapType cellMap;
MapType::iterator cellMapIter;
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
     
    //der 2D Fall
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
//END ValueMapping ~cellnumberZ~ CLOSE


/*
1.7 InfoGrepSpatialJoin

This version of the SpatialJoin operator uses only the memory 
available for that operator. From the first stream, it collects
all Tuples into a vector and creates an r-tree from the geometries
until the stream or the memory  is exhausted. 
Then, it starts to search on the r-tree using the tuples from the
second stream. If the first stream was not exhausted, all tuples 
of the second stream are collected within a tuple buffer.
While there are elements within the first stream, a partition of
is is used to create a new r-tree and the tuple buffer is scanned
for searching on the r-tree.

1.7.1 Type mapping ~realJoinMMRTReeInfoGrep~

Signature is: stream(tuple(A)) x stream(tuple(B)) x a[ ]i x b[ ]j x 
              int x int [ x int ] -> stream(tuple(A o B))

Meaning is : stream1 x stream2 x attrname1 x attrname2 x min x max x maxMem
where


----
  stream1   : first input stream
  stream2   : second input stream
  attrname1 : attribute name for an attribute of spatial type  in stream1
  attrname2 : attribute name for an attribute of spatial type  in stream2
  min       : minimum number of entries within a node of the used rtree
  max       : maximum number of entries within a node of the used rtree
 [maxMem]   : maximum memory available for storing tuples

----

*/

ListExpr realJoinMMRTreeTMInfoGrep(ListExpr args){

  string err = "stream(tuple(A)) x stream(tuple(B)) x"
               " ai x bi x int x int [x int] expected";
   int len = nl->ListLength(args);
   if((len!=6) && (len!=7)){
     return listutils::typeError(err);
   }

   if(!Stream<Tuple>::checkType(nl->First(args)) ||
      !Stream<Tuple>::checkType(nl->Second(args)) ||
      !listutils::isSymbol(nl->Third(args)) ||
      !listutils::isSymbol(nl->Fourth(args))  ||
      !CcInt::checkType(nl->Fifth(args)) ||
      !CcInt::checkType(nl->Sixth(args))){
     return listutils::typeError(err);
   }

   if( (len==7) && ! CcInt::checkType(nl->Sixth(nl->Rest((args))))){
     return listutils::typeError(err);
   }

   ListExpr attrList1 = nl->Second(nl->Second(nl->First(args)));
   ListExpr attrList2 = nl->Second(nl->Second(nl->Second(args)));
   string name1 = nl->SymbolValue(nl->Third(args));
   string name2 = nl->SymbolValue(nl->Fourth(args));

   ListExpr type1;
   ListExpr type2;

   int index1 = listutils::findAttribute(attrList1, name1, type1);
   if(index1 == 0){
     return listutils::typeError("Attribute " + name1 + 
                                 " not present in first stream");
   }

   
   int index2 = listutils::findAttribute(attrList2, name2, type2);
   if(index2 == 0){
     return listutils::typeError("Attribute " + name2 + 
                                 " not present in second stream");
   }

   // check for spatial attribute
   if(!listutils::isKind(type1,Kind::SPATIAL2D()) &&
      !listutils::isKind(type1,Kind::SPATIAL3D())  ){
     string t = " (type is " + nl->ToString(type1)+ ")";  
     return listutils::typeError("Attribute " + name1 + 
                                 " is not in Kind Spatial2D or Spatial3D"
                                 + t);
   }
   if(!listutils::isKind(type2,Kind::SPATIAL2D()) &&
      !listutils::isKind(type2,Kind::SPATIAL3D())  ){
     string t = " (type is " + nl->ToString(type2)+ ")";  
     return listutils::typeError("Attribute " + name2 + 
                                 " is not in Kind Spatial2D or Spatial3D"
                                 + t);
   }
 
  // check for same dimension
  if(listutils::isKind(type1,Kind::SPATIAL2D()) &&
     !listutils::isKind(type2,Kind::SPATIAL2D())){
    return listutils::typeError("Selected attributes have "
                                "different dimensions"); 
  }
  if(listutils::isKind(type1,Kind::SPATIAL3D()) &&
     !listutils::isKind(type2,Kind::SPATIAL3D())){
    return listutils::typeError("Selected attributes have"
                                " different dimensions"); 
  }

 bool rect1 = Rectangle<2>::checkType(type1) || Rectangle<3>::checkType(type1);
 bool rect2 = Rectangle<2>::checkType(type2) || Rectangle<3>::checkType(type2);

 
   ListExpr attrList = listutils::concat(attrList1, attrList2);

   if(!listutils::isAttrList(attrList)){
     return listutils::typeError("name conflicts in tuple streams");
   }

   ListExpr resList = nl->TwoElemList( 
                           nl->SymbolAtom(Stream<Tuple>::BasicType()),
                           nl->TwoElemList(
                               nl->SymbolAtom(Tuple::BasicType()),
                               attrList));
   ListExpr appendList;
   if(len==7){
      appendList =   nl->FourElemList(nl->IntAtom(index1-1), 
                                     nl->IntAtom(index2-1),
                                     nl->BoolAtom(rect1),
                                     nl->BoolAtom(rect2));
   } else {
      appendList =   nl->FiveElemList(
                                     nl->IntAtom(-1),
                                     nl->IntAtom(index1-1), 
                                     nl->IntAtom(index2-1),
                                     nl->BoolAtom(type1),
                                     nl->BoolAtom(type2));
   }

   return nl->ThreeElemList(
                   nl->SymbolAtom(Symbols::APPEND()),
                   appendList,
                   resList);

}

/*
1.7.2 LocalInfo

*/
template<int dim>
class InfoGrepSpatialJoinInfo{
  public:
      
  double dimMaxX;
  double dimMinX;
  double dimMaxY;
  double dimMinY;
  double dimMaxZ;
  double dimMinZ;
  double maxXup;
  double minXdown;
  double maxYup;
  double minYdown;
  double maxZup;
  double minZdown;
  
  vector<int> cellnumbers;
  
  vector<int> HistogramXdim;
  vector<int> HistogramYdim;
  vector<int> HistogramZdim;
  
  vector<int> RepHistX;
  vector<int> RepHistY;
  vector<int> RepHistZ;
  double maxrepfaktX;
  double maxrepfaktY;
  double maxrepfaktZ;
  
  vector<int> HistogrammXdimG;
  double balkenbreiteHistXG;
  double balkenbreiteHistXGalt;
  double HistGIntvStartX;
  double HistGIntvEndX;
  double HistGIntvStartXalt;
  double HistGIntvEndXalt;
  
  vector<int> HistogrammYdimG;
  double balkenbreiteHistYG;
  double balkenbreiteHistYGalt;
  double HistGIntvStartY;
  double HistGIntvEndY;
  double HistGIntvStartYalt;
  double HistGIntvEndYalt;
  
  vector<int> HistogrammZdimG;
  double balkenbreiteHistZG;
  double balkenbreiteHistZGalt;
  double HistGIntvStartZ;
  double HistGIntvEndZ;
  double HistGIntvStartZalt;
  double HistGIntvEndZalt;
  
  double count;
  
  double GridXwidth;
  double GridYwidth;
  double GridZwidth;
  double GridXroot;
  double GridYroot;
  double GridZroot;
  double perzentilx;
  double perzentily;
  double perzentilz;
  
  int param;
  bool paramMinMax;
  bool paramHistLog;
  bool paramHistG;
  
  
  InfoGrepSpatialJoinInfo(Word& _s1, Word& _s2, 
                    const int _min, const int _max,
                    size_t _maxMem, const int _a1,
                    const int _a2, ListExpr ttl):
                    s1(_s1), s2(_s2), min(_min), max(_max),
                     a1(_a1), a2(_a2)
  {   param=111;
      
      if (param==0){paramMinMax=false;
                    paramHistLog=false;
                    paramHistG=false;}
      else if (param<10 && param>0){    
                    paramMinMax=true;
                    paramHistLog=false;
                    paramHistG=false;}
      else if (param>=10 && param<100){
                    if (param%10>0){paramMinMax=true;}
                    else {paramMinMax=false;}
                    paramHistLog=true;
                    paramHistG=false;}         
      else if (param>99 && param<1000){
                    if ((param%100-param%10)/10>0){paramMinMax=true;}
                    else {paramMinMax=false;}
                    if (param%100>0){paramHistLog=true;}
                    else {paramHistLog=false;}
                    paramHistG=true;} 
       
      s1.open();
      s2.open();
      Tuple* t1 = s1.request();
      tt = new TupleType(ttl);
      tuples2 = 0;
      treeIt = 0;
      bufferIt = 0;
      currentTuple2 =0; 
      
      // process the first Tuple
      if(t1){
         count=1; 
         index = new mmrtree::RtreeT<dim, TupleId>(min,max);
         int rootSize = t1->GetRootSize();
         int sizePerTuple = rootSize + sizeof(void*) + 100;  
         maxTuples = (_maxMem*1024) / sizePerTuple; 
         if(maxTuples <= 10){
            maxTuples = 10; // force a minimum of ten tuples
         }
        TupleId id = (TupleId) tuples1.size();
        tuples1.push_back(t1);
        Rectangle<dim> box = ((StandardSpatialAttribute<dim>*)
                         t1->GetAttribute(a1))->BoundingBox();
        maxrepfaktX=1.15;
        maxrepfaktY=1.15;
        maxrepfaktZ=1.15;
        
        if (paramMinMax){
        dimMaxX=box.MaxD(0);
        dimMinX=box.MinD(0);
        dimMaxY=box.MaxD(1);
        dimMinY=box.MinD(1);   
        if (dim==3){
           dimMaxZ=box.MaxD(2);
           dimMinZ=box.MinD(2); 
        }
        }
        if (paramHistLog){
        if (dim==2){
           putDataToHistX(box.MaxD(0)*1000-box.MinD(0)*1000);
           putDataToHistY(box.MaxD(1)*1000-box.MinD(1)*1000);         
        }
        else if (dim==3){
           putDataToHistZ(box.MaxD(2)*1000-box.MinD(2)*1000);
        }
        }
        if(paramHistG){
        HistGIntvStartX=box.MinD(0)*1000-0.0005;
        HistGIntvEndX=box.MaxD(0)*1000+0.0005;
        HistGIntvStartXalt=HistGIntvStartX;
        HistGIntvEndXalt=HistGIntvEndX;
        HistogrammXdimG.resize(11);
        balkenbreiteHistXG=(HistGIntvEndX-HistGIntvStartX)/10;
        balkenbreiteHistXGalt=balkenbreiteHistXG;
        setintvALLX(((box.MaxD(0)*1000-box.MinD(0)*1000)/2)+
                box.MinD(0)*1000);
        
        HistGIntvStartY=box.MinD(1)*1000-0.0005;
        HistGIntvEndY=box.MaxD(1)*1000+0.0005;
        HistGIntvStartYalt=HistGIntvStartY;
        HistGIntvEndYalt=HistGIntvEndY;
        HistogrammYdimG.resize(11);
        balkenbreiteHistYG=(HistGIntvEndY-HistGIntvStartY)/10;
        balkenbreiteHistYGalt=balkenbreiteHistYG;
        setintvALLY(((box.MaxD(1)*1000-box.MinD(1)*1000)/2)+
                box.MinD(1)*1000);
 
        if (dim==3){    
        HistGIntvStartZ=box.MinD(2)*1000-0.0005;
        HistGIntvEndZ=box.MaxD(2)*1000+0.0005;
        HistGIntvStartZalt=HistGIntvStartZ;
        HistGIntvEndZalt=HistGIntvEndZ;
        HistogrammZdimG.resize(11);
        balkenbreiteHistZG=(HistGIntvEndZ-HistGIntvStartZ)/10;
        balkenbreiteHistZGalt=balkenbreiteHistZG;
        setintvALLZ(((box.MaxD(2)*1000-box.MinD(2)*1000)/2)+
                box.MinD(2)*1000);
        }
        
        }
        
      index->insert(box,id);
      } else {
        index = 0;
        return; 
      }      
      t1 = s1.request();
      int noTuples = 1;
      while( (t1!=0) && (noTuples < maxTuples -1)){
        count++;  
        TupleId id = (TupleId) tuples1.size();
        tuples1.push_back(t1);
        Rectangle<dim> box = ((StandardSpatialAttribute<dim>*)
                         t1->GetAttribute(a1))->BoundingBox();
        
        if (paramMinMax){
            if (box.MaxD(0)>dimMaxX){dimMaxX=box.MaxD(0);}
            if (box.MinD(0)<dimMinX){dimMinX=box.MinD(0);}
            if (box.MaxD(1)>dimMaxY){dimMaxY=box.MaxD(1);}
            if (box.MinD(1)<dimMinY){dimMinY=box.MinD(1);}
            if (dim==3){
                if (box.MaxD(2)>dimMaxZ){dimMaxZ=box.MaxD(2);}
                if (box.MinD(2)<dimMinZ){dimMinZ=box.MinD(2);} 
        }
        }
        
        if(paramHistLog){
        if (dim==2){
            putDataToHistX(box.MaxD(0)*1000-box.MinD(0)*1000);
            putDataToHistY(box.MaxD(1)*1000-box.MinD(1)*1000);         
        }
        else if (dim==3){
            putDataToHistZ(box.MaxD(2)*1000-box.MinD(2)*1000);
        } 
        }
        
        if (paramHistG){
        double temp;    
        balkenbreiteHistXGalt = balkenbreiteHistXG;
        temp=(((box.MaxD(0)*1000-box.MinD(0)*1000)/2)+box.MinD(0)*1000);
        if (temp<HistGIntvStartX){    
           minXdown++; 
           HistGIntvStartXalt = HistGIntvStartX;
           HistGIntvStartX=temp-0.00005;
        }
        if (temp>HistGIntvEndX){
           maxXup++;
           HistGIntvEndXalt = HistGIntvEndX;
           HistGIntvEndX=temp+0.00005;
        }
        balkenbreiteHistXG=(double)((HistGIntvEndX-HistGIntvStartX)/10);
        setintvALLX(temp);
        
        temp=(((box.MaxD(1)*1000-box.MinD(1)*1000)/2)+box.MinD(1)*1000);
        balkenbreiteHistYGalt = balkenbreiteHistYG;    
        if (temp<HistGIntvStartY){
           minYdown++;
           HistGIntvStartYalt = HistGIntvStartY;
           HistGIntvStartY=temp-0.00005;
        }
        if (temp>HistGIntvEndY){
           maxYup++;
           HistGIntvEndYalt = HistGIntvEndY;
           HistGIntvEndY=temp+0.00005;
        }
        balkenbreiteHistYG=(double)((HistGIntvEndY-HistGIntvStartY)/10);
        setintvALLY(temp);
         
        if(dim==3){
        balkenbreiteHistZGalt = balkenbreiteHistZG;
        temp=(((box.MaxD(2)*1000-box.MinD(2)*1000)/2)+box.MinD(2)*1000);
        if (temp<HistGIntvStartZ){
           minZdown++;
           HistGIntvStartZalt = HistGIntvStartZ;
           HistGIntvStartZ=temp-0.0005;
        }
        if (temp>HistGIntvEndZ){
           maxZup++;
           HistGIntvEndZalt = HistGIntvEndZ;
           HistGIntvEndZ=temp+0.0005;
        }
        balkenbreiteHistZG=
                (double)((HistGIntvEndZ-HistGIntvStartZ)/10);
        setintvALLZ(temp);
        }
        }
        
        index->insert(box,id);
        t1 = s1.request();
        noTuples++;
      }
      
      if(paramMinMax){
      cout << "MaxXDim: " << dimMaxX << endl;
      cout << "MinXDim: " << dimMinX << endl;
      cout << "MaxYDim: " << dimMaxY << endl;
      cout << "MinYDim: " << dimMinY << endl;
      cout << "MaZDim: " << dimMaxZ << endl;
      cout << "MaxZDim: " << dimMinZ << endl;
      }
      
      if(paramHistLog){
      for (size_t i=0; i<HistogramXdim.size(); i++){
      cout << "LogHistx(" <<  i << ") " << HistogramXdim[i] << endl;}
      for (size_t i=0; i<HistogramYdim.size(); i++){
      cout << "LogHisty(" <<  i << ") " << HistogramYdim[i] << endl;}
      for (size_t i=0; i<HistogramZdim.size(); i++){
      cout << "LogHistz(" <<  i << ") " << HistogramZdim[i] << endl;}
      cout << "Tupelgesamtzahl R1: " << count << endl;
      RepHistX.resize(HistogramXdim.size());
      analyseHistXLog();
      RepHistY.resize(HistogramYdim.size());
      analyseHistYLog();
      RepHistZ.resize(HistogramZdim.size());
      analyseHistZLog();
      for (size_t i=0; i<RepHistX.size(); i++){
          cout << "RepHistX(" <<  i << ") " << RepHistX[i] << endl;}
      for (size_t i=0; i<RepHistY.size(); i++){
          cout << "RepHistY(" <<  i << ") " << RepHistY[i] << endl;}
      for (size_t i=0; i<RepHistZ.size(); i++){
          cout << "RepHistZ(" <<  i << ") " << RepHistZ[i] << endl;} 
      analyseDataLogHist(maxrepfaktX,maxrepfaktY,maxrepfaktZ);
      }
      
      if(paramHistG){
      for (size_t i=0; i<HistogrammXdimG.size(); i++){
      cout<<"HistogrammXdimG("<<i<<") = "<<HistogrammXdimG[i]<<endl;}
      for (size_t i=0; i<HistogrammYdimG.size(); i++){
      cout<<"HistogrammYdimG("<<i<<") = "<<HistogrammYdimG[i]<<endl;}
      for (size_t i=0; i<HistogrammZdimG.size(); i++){
      cout<<"HistogrammZdimG("<<i<<") = "<<HistogrammZdimG[i]<<endl;}
      cout << "Tupelgesamtzahl R1: " << count << endl;
      analyseDataHistG();
      }
      // process the last tuple if present
      if(t1){
        finished = false;
        TupleId id = (TupleId) tuples1.size();
        tuples1.push_back(t1);
        Rectangle<dim> box = ((StandardSpatialAttribute<dim>*)
                         t1->GetAttribute(a1))->BoundingBox(); 
        index->insert(box,id);
        
      } else {
        finished = true; // stream 1 exhausted
      }
      scans = 1;

   }
    
   ~InfoGrepSpatialJoinInfo(){
      s1.close();
      s2.close();
      tt->DeleteIfAllowed();
      for(unsigned int i=0;i< tuples1.size();i++){
         if(tuples1[i]){
           tuples1[i]->DeleteIfAllowed();
           tuples1[i] = 0;
         }
      }
      tuples1.clear();
      if(bufferIt){
         delete bufferIt;
      }
      if(tuples2){
         tuples2->DeleteAndTruncate();
         //delete tuples2;
      } 
      if(treeIt){
        delete treeIt;
      }
      if(index){
        delete index;
      }
    }



    Tuple* next(){
       if(!index){
         return 0;
       }
       while(true){ // use return for finishing this loop 
           
          if(!treeIt){ // try to create a new tree iterator
             createNewTreeIt();
             if(treeIt==0){
               cout << "Join finished with " << scans 
                    << " scans for stream 2" << endl;
               cout << "there are " << maxTuples 
                    << " maximum stored tuples within the index" << endl;
               return 0;
             }
          }
          
          TupleId const* id = treeIt->next();    
          if(!id){ //treeIt exhausted
             delete treeIt;
             treeIt = 0;
             currentTuple2->DeleteIfAllowed();
             currentTuple2=0;
          } else { // new result found
            Tuple* res = new Tuple(tt);
            Tuple* t1 = tuples1[*id];
            Concat(t1,currentTuple2,res);
            return res;
          }        
       } 
          
    }
    
  private:
      Stream<Tuple> s1;
      Stream<Tuple> s2;
      int min;
      int max;
      int a1;
      int a2;
      TupleType* tt;
      int maxTuples;
      vector<Tuple*> tuples1;
      vector<Relation*> CellRelations;
      Relation* tuplesx;
      Relation* tuples2;
      bool finished;  // all tuples from stream1 read 
      typename mmrtree::RtreeT<dim, TupleId>::iterator* treeIt; 
      Tuple* currentTuple2;  // tuple from stream 2
      GenericRelationIterator* bufferIt;
      mmrtree::RtreeT<dim, TupleId>* index;
      int scans;

      
            void createNewTreeIt(){
         assert(!treeIt);
         if(!currentTuple2){
            getCurrentTuple(); 
         } 
         if(!currentTuple2){
             return;
         }
         Rectangle<dim> r = ((StandardSpatialAttribute<dim>*)
                    currentTuple2->GetAttribute(a2))->BoundingBox();
         treeIt = index->find(r);
      }

      void getCurrentTuple(){
         if(currentTuple2){
           currentTuple2->DeleteIfAllowed();
           currentTuple2=0;
         }
         if(bufferIt){ // read from Buffer
            currentTuple2 = bufferIt->GetNextTuple();
            if(!currentTuple2){
               if(finished){
                 return;
               } else { 
                 rebuildIndex();
                 resetBuffer();
               }
            } else {
              return; // next current tuple found
            }
         } else {
           currentTuple2 = s2.request();
           if(!currentTuple2){
              if(finished){
                return; 
              } else {
                rebuildIndex();
                resetBuffer();
              }
           } else {
             if(!finished){ //insert Tuple into buffer
               if(!tuples2){
                  tuples2 = new Relation(currentTuple2->GetTupleType(),true);
               }
               currentTuple2->PinAttributes();
               tuples2->AppendTupleNoLOBs(currentTuple2);
             }
             return;
           }
         }
      }

      void rebuildIndex(){
         if(treeIt){
           delete treeIt;
           treeIt=0;
         }
         if(index){
           delete index;
         }
         for(unsigned int i=0;i<tuples1.size();i++){
            tuples1[i]->DeleteIfAllowed();
            tuples1[i]=0;
         }
         tuples1.clear();
         index = new mmrtree::RtreeT<dim, TupleId>(min,max);
         Tuple* t1 = s1.request();
         int noTuples = 0;
         while( (t1!=0) && (noTuples < maxTuples -1)){
             TupleId id = (TupleId) tuples1.size();
             tuples1.push_back(t1);
             Rectangle<dim> box = ((StandardSpatialAttribute<dim>*)
                           t1->GetAttribute(a1))->BoundingBox(); 
             index->insert(box,id);
             t1 = s1.request();
             noTuples++;
         }
         // process the last tuple if present
        if(t1){
           finished = false;
           TupleId id = (TupleId) tuples1.size();
           tuples1.push_back(t1);
           Rectangle<dim> box = ((StandardSpatialAttribute<dim>*)
                          t1->GetAttribute(a1))->BoundingBox(); 
           index->insert(box,id);
        } else {
          finished = true; // stream 1 exhausted
        }
      }

      void resetBuffer(){
         assert(tuples2);
         if(bufferIt){
           delete bufferIt;
         }
         bufferIt = tuples2->MakeScan();
         assert(currentTuple2==0);
         currentTuple2 = bufferIt->GetNextTuple();
         scans++;
      }
      
      
      
//Hilfsfunktionen PRIVAT      
     double lb( double x ){
        return log10( x ) / log10( 2.0 );
     }
 
     void analyseDataLogHist(double maxrepfaktX, 
             double maxrepfaktY, 
             double maxrepfaktZ){
        unsigned int i=0;
        //int i=0;
        while (RepHistX[i]/count>maxrepfaktX && i<RepHistX.size()){
        cout<<RepHistX[i]/count<<"<"<<maxrepfaktX<<":"<<i<<endl;
        i++;} 
        cout<<RepHistX[i]/count<<"<"<<maxrepfaktX<<":"<<i<<endl;
        GridXwidth=pow(2,i)/1000;
        cout<<"GridXwidth: "<<GridXwidth<<endl;
        i=0;
        while (RepHistY[i]/count>maxrepfaktY && i<RepHistY.size()){
        cout<<RepHistY[i]/count<<"<"<<maxrepfaktY<<":"<<i<<endl; 
        i++;}
        cout<<RepHistY[i]/count<<"<"<<maxrepfaktY<<":"<<i<<endl;
        GridYwidth=pow(2,i)/1000;
        cout<<"GridYwidth: "<<GridYwidth<<endl;
        if (dim==3){
        i=0;
        while (RepHistZ[i]/count>maxrepfaktZ && i<RepHistZ.size()){i++;
        cout<<RepHistZ[i]/count<<"<"<<maxrepfaktZ<<":"<<i<<endl;
        i++;}
        cout<<RepHistZ[i]/count<<"<"<<maxrepfaktZ<<":"<<i<<endl;
        GridZwidth=pow(2,i)/1000;
        cout<<"GridZwidth: "<<GridZwidth<<endl;
        }  
     }
     
     void analyseDataHistG(){  
        if(paramHistG){
        if (minXdown/maxXup>4){
        cout<<"Bias <- "<<minXdown/maxXup<<endl;perzentilx=25;}
        else if (maxXup/minXdown>4){
        cout<<"Bias -> "<<maxXup/minXdown<<endl;perzentilx=75;}
        else {
        cout<<"NO Bias "<< minXdown/maxXup<<"/"<<maxXup/minXdown<<endl;
        perzentilx=50;}
        if (minYdown/maxYup>4){
        cout<<"Bias <- "<<minYdown/maxYup<<endl;perzentily=25;}
        else if (maxYup/minYdown>4){
        cout<<"Bias -> "<<maxYup/minYdown<<endl;perzentily=75;}
        else {
        cout <<"NO Bias "<< minYdown/maxYup<<"/"<<maxYup/minYdown<<endl;
        perzentily=50;}
        if (dim==3){
        if (minZdown/maxZup>4){
        cout<<"Bias <-"<<minZdown/maxZup<<endl;perzentilz=25;}
        else if (maxZup/minZdown>4){
        cout<<"Bias ->"<<maxZup/minZdown<<endl;perzentilz=75;}
        else {
        cout<<"NO Bias "<<minZdown/maxZup<<"/"<<maxZup/minZdown<<endl;
        perzentilz=50;}
        }
        int perzentilCounter=0;
        //unsigned
        for (unsigned int i=1;i<HistogrammXdimG.size();i++){
        if ((perzentilCounter/count)<(perzentilx/100)){
        perzentilCounter=perzentilCounter+HistogrammXdimG[i];   
        GridXroot=((dimMinX+i*balkenbreiteHistXG/1000)); 
            } 
        }
        cout << "GridXroot: " << GridXroot << endl;
        perzentilCounter=0;
        //unsigned
        for (unsigned int i=1;i<HistogrammYdimG.size();i++){
        if ((perzentilCounter/count<perzentily/100)){
        perzentilCounter=perzentilCounter+HistogrammYdimG[i];
        GridYroot=((dimMinY+i*balkenbreiteHistYG/1000));    
            }    
        }
        
        cout << "GridYroot: " << GridYroot << endl;
        if (dim==3){
        perzentilCounter=0;
        //unsigned
        for (unsigned int i=1;i<HistogrammZdimG.size();i++){
        if ((perzentilCounter/count)<(perzentilz/100)){
        perzentilCounter=perzentilCounter+HistogrammZdimG[i];
        GridZroot=((dimMinZ+i*balkenbreiteHistZG/1000));    
            }    
        }
        cout << "GridZroot: " << GridZroot << endl;
        }
     }
     }
        
     void setDataHistogramXdimG (double wert){
         int hx = (int)(abs(wert-HistGIntvStartX)/balkenbreiteHistXG)+1; 
         HistogrammXdimG[hx]=HistogrammXdimG[hx]+1;
     }
     
     void setintvALLX (double wert){

        int altST=HistGIntvStartXalt;
        int altND=HistGIntvEndXalt;
        if (wert>HistGIntvEndXalt){
                umgewichtenHistXG(altST,altND,true);
        }
        if (wert<HistGIntvStartXalt){
            umgewichtenHistXG(altST,altND,false);
        }
        setDataHistogramXdimG(wert);
    }
      
    void umgewichtenHistXG(int altst, int altnd, bool flag){
        
        vector<double> tmpH;
        tmpH.resize(11);
        double balkenSTalt, balkenSTneu, balkenNDalt, balkenNDneu;
        double gewichtneu=0;
        int i=1;
        tmpH[0]=0;
        bool off=false;

        for(int j=1; j < 11; j++){
            off=false;
            while (off==false) {
                balkenSTneu = HistGIntvStartX+(j-1)*balkenbreiteHistXG;
                balkenNDneu = HistGIntvStartX+j*balkenbreiteHistXG;
           if (i<12){
               balkenSTalt = altst+(i-1)*balkenbreiteHistXGalt;
               balkenNDalt = altst+i*balkenbreiteHistXGalt;
           }
           else {
               balkenSTalt = 0;
               balkenNDalt = 0;
           }
        if (flag==false) { 
        //Intervall wird nach unten hin erweitert
         if (balkenNDneu<balkenSTalt) { 
             //d.h. der Balken j im neuen Interval erhält Gewicht o,
             //denn er schneidet i im alten Interval nicht
             gewichtneu = 0;
             tmpH[j] = gewichtneu;
             off=true; //neuer j-Schleifendurchlauf soll ja starten!
             // j muss weitergezählt werden, also weiterer Durchlauf
             //der for-Schleife, keine weitere Aktio n nötig
         }
         else if (balkenNDneu<=balkenNDalt && i<11) {
             //d.h. Balken j des neuen Intervalls ragt in i des Alten
             //Intervalls hinein, j bekommt Anteil von i
             double bchoose;
             if (balkenNDneu<altnd){bchoose=balkenNDneu;}
             else {bchoose=altnd;}
             gewichtneu=gewichtneu+((bchoose-
                     balkenSTalt)/balkenbreiteHistXGalt )*
                     HistogrammXdimG[i];
             tmpH[j] = gewichtneu;
             gewichtneu = 0; 
             //kann gesetzt werden, da ja da Ende des Balken j im 
             //neuen Intervall erreicht wird Weiteres Gewicht aus 
             //i muss in nächsten Balken j
             off=true; 
             //neuer j-Schleifendurchlauf soll ja starten!;
         }
         else if (balkenNDneu>=balkenNDalt && i<11){
             double bchoose;
             if (balkenSTalt>balkenSTneu){bchoose=balkenSTalt;}
             else {bchoose=balkenSTneu;}
             gewichtneu=gewichtneu+((balkenNDalt-bchoose)/
                     balkenbreiteHistXGalt )*HistogrammXdimG[i];
             i++; 
             //j überspannt den Balken aus i vollständig. Aus dem 
             //nächsten Balken aus i muss weiteres Gewicht addiert 
             //werden, also muss i um 1 erhöht wrden.
         }
         else if (i>=11){
             tmpH[j] = gewichtneu;
             gewichtneu = 0;
             off=true; //neuer j-Schleifendurchlauf soll ja starten!
         }
        }
        else if (flag) { 
            //Intervall wird nach oben verlängert
            if (balkenNDneu<=balkenNDalt && i<11) {
                //d.h. Balken j des neuen Intervalls ragt in i des 
                //Alten Intervalls hinein, j bekommt Anteil von i
                double bchoose;
                if (balkenNDneu<altnd){bchoose=balkenNDneu;}
                else {bchoose=altnd;} 
                gewichtneu=gewichtneu+((bchoose
                        -balkenSTalt)/balkenbreiteHistXGalt )
                        *HistogrammXdimG[i];
                tmpH[j] = gewichtneu;
                gewichtneu = 0; //kann gesetzt werden, da ja da Ende des
                //Balken j im neuen Intervall erreicht wird Weiteres 
                //Gewicht aus i muss in nächsten Balken j
                off=true; 
                //neuer j-Schleifendurchlauf soll ja starten!
            }
            else if (balkenNDneu>=balkenNDalt && i<11){
                double bchoose;
                if (balkenSTalt>balkenSTneu){bchoose=balkenSTalt;}
                else {bchoose=balkenSTneu;}
                gewichtneu=gewichtneu +((balkenNDalt
                        -bchoose)/balkenbreiteHistXGalt)
                        *HistogrammXdimG[i];
                i++; 
                //j überspannt den Balken aus i vollständig. Aus dem 
                //nächsten Balken aus i muss weiteres Gewicht addiert 
                //werden, also muss i um 1 erhöht wrden.
            }
            else if (i>=11){
                tmpH[j] = gewichtneu;
                gewichtneu = 0;
                off=true; //neuer j-Schleifendurchlauf soll ja starten!
            }
        }
        }
        }        
        for (int j=1; j < 11; j++){
            HistogrammXdimG[j]=tmpH[j];
        }
    }
    
    void setDataHistogramYdimG (double wert){
        int hx = (int)(abs(wert-HistGIntvStartY)/balkenbreiteHistYG)+1;
        HistogrammYdimG[hx]=HistogrammYdimG[hx]+1;
    }
     
    void setintvALLY (double wert){
         int altST=HistGIntvStartYalt;
        int altND=HistGIntvEndYalt;
        if (wert>HistGIntvEndYalt){
            umgewichtenHistYG(altST,altND,true);
        }
	if (wert<HistGIntvStartYalt){
            umgewichtenHistYG(altST,altND,false);
	}
        setDataHistogramYdimG(wert);
    }
      
    void umgewichtenHistYG(int altst, int altnd, bool flag){
        vector<double> tmpH;
        tmpH.resize(11);
        double balkenSTalt, balkenSTneu, balkenNDalt, balkenNDneu;
        double gewichtneu=0;
        int i=1;
        tmpH[0]=0;
        bool off=false;
        for(int j=1; j < 11; j++){
            off=false;
            while (off==false) {
                balkenSTneu = HistGIntvStartY+(j-1)*balkenbreiteHistYG;
                balkenNDneu = HistGIntvStartY+j*balkenbreiteHistYG;
                if (i<12){
		balkenSTalt = altst+(i-1)*balkenbreiteHistYGalt;
		balkenNDalt = altst+i*balkenbreiteHistYGalt;
                }
                else {
                balkenSTalt = 0;
		balkenNDalt = 0;
                }
                if (flag==false) { 
                    //Intervall wird nach unten hin erweitert
                    if (balkenNDneu<balkenSTalt) { 
                        //d.h. der Balken j im neuen Interval erhält Gewicht o,
                        //denn er schneidet i im alten Interval nicht
                        gewichtneu=0;
                        tmpH[j] = gewichtneu;
                        off=true; //neuer j-Schleifendurchlauf soll ja starten!
                    // j muss weitergezählt werden, also weiterer Durchlauf 
                    //der for-Schleife, keine weitere Aktio n nötig
                    }
                    else if (balkenNDneu<=balkenNDalt && i<11) {
                    //d.h. Balken j des neuen Intervalls ragt in i des Alten
                    //Intervalls hinein, j bekommt Anteil von i
                       double bchoose;
                       if (balkenNDneu<altnd){bchoose=balkenNDneu;}
                       else {bchoose=altnd;}
                    
                       gewichtneu=gewichtneu+((bchoose-balkenSTalt)/
                               balkenbreiteHistYGalt)*HistogrammYdimG[i];
                       tmpH[j] = gewichtneu;
                       gewichtneu=0; 
                       //kann gesetzt werden, da ja da Ende des Balken j im 
                       //neuen Intervall erreicht wird Weiteres Gewicht aus 
                       //i muss in nächsten Balken j
                       off=true; //neuer j-Schleifendurchlauf soll ja starten!;
                    }
                    else if (balkenNDneu>=balkenNDalt && i<11){
                       double bchoose;
                       if (balkenSTalt>balkenSTneu){bchoose=balkenSTalt;}
                       else {bchoose=balkenSTneu;}
                        
                        gewichtneu=gewichtneu+((balkenNDalt-bchoose)/
                               balkenbreiteHistYGalt)*HistogrammYdimG[i];
                       i++; 
                       //j überspannt den Balken aus i vollständig. Aus dem 
                       //nächsten Balken aus i muss weiteres Gewicht addiert 
                       //werden, also muss i um 1 erhöht wrden.
                    }
                    else if (i>=11){
                        tmpH[j] = gewichtneu;
                        gewichtneu=0;
                        off=true; //neuer j-Schleifendurchlauf soll ja starten!
                    }

                }
                else if (flag) { 
                    //Intervall wird nach oben verlängert
                    if (balkenNDneu<=balkenNDalt && i<11) {
                        //d.h. Balken j des neuen Intervalls ragt in i des 
                        //Alten Intervalls hinein, j bekommt Anteil von i
                       double bchoose;
                       if (balkenNDneu<altnd){bchoose=balkenNDneu;}
                       else {bchoose=altnd;} 
                       gewichtneu=gewichtneu+((bchoose
                               -balkenSTalt)/balkenbreiteHistYGalt )
                               *HistogrammYdimG[i];
                       tmpH[j] = gewichtneu;
                       gewichtneu=0;                    
                       //kann gesetzt werden, da ja da Ende des
                       //Balken j im neuen Intervall erreicht wird Weiteres 
                       //Gewicht aus i muss in nächsten Balken j
                       off=true; 
                       //neuer j-Schleifendurchlauf soll ja starten!
                    }
                    else if (balkenNDneu>=balkenNDalt && i<11){
                       double bchoose;
                       if (balkenSTalt>balkenSTneu){bchoose=balkenSTalt;}
                       else {bchoose=balkenSTneu;}
                        gewichtneu=gewichtneu+((balkenNDalt
                               -bchoose)/balkenbreiteHistYGalt)
                                *HistogrammYdimG[i];
                       i++; 
                       //j überspannt den Balken aus i vollständig. Aus dem 
                       //nächsten Balken aus i muss weiteres Gewicht addiert 
                       //werden, also muss i um 1 erhöht wrden.
                    }
                    else if (i>=11){
                        tmpH[j] = gewichtneu;
                        gewichtneu=0;
                        off=true; //neuer j-Schleifendurchlauf soll ja starten!
                    }
                  }
                }
              }        
             for (int j=1; j < 11; j++){
		HistogrammYdimG[j]=tmpH[j];
	     }
	   
    }
    
    
    void setDataHistogramZdimG (double wert){
	int hx = (int)(abs(wert-HistGIntvStartZ)/balkenbreiteHistZG)+1;
	HistogrammZdimG[hx]=HistogrammZdimG[hx]+1;
     }
     
     void setintvALLZ (double wert){
        int altST=HistGIntvStartZalt;
	int altND=HistGIntvEndZalt;
	if (wert>HistGIntvEndZalt){
                umgewichtenHistZG(altST, altND,true);
	}
	if (wert<HistGIntvStartZalt){
                umgewichtenHistZG(altST, altND,false);
	}
	setDataHistogramZdimG(wert);
    }
      
    void umgewichtenHistZG(int altst, int altnd, bool flag){
	vector<double> tmpH;
        tmpH.resize(11);
	double balkenSTalt, balkenSTneu, balkenNDalt, balkenNDneu;
	double gewichtneu;
	int i=1;
	tmpH[0]=0;
	bool off=false;
	for(int j=1; j < 11; j++){
		off=false;
                while (off==false) {   
                balkenSTneu = HistGIntvStartZ+(j-1)*balkenbreiteHistZG;
		balkenNDneu = HistGIntvStartZ+j*balkenbreiteHistZG;
                if (i<12){
		balkenSTalt = altst+(i-1)*balkenbreiteHistZGalt;
		balkenNDalt = altst+i*balkenbreiteHistZGalt;
                }
                else {
                balkenSTalt = 0;
		balkenNDalt = 0;
                }
                if (flag==false) {  
                    //Intervall wird nach unten hin erweitert

                    if (balkenNDneu<balkenSTalt) { 
                    //d.h. der Balken j im neuen Interval erhält Gewicht o,
                    //denn er schneidet i im alten Interval nicht
                    gewichtneu=0;
                    tmpH[j] = gewichtneu;
                    off=true; //neuer j-Schleifendurchlauf soll ja starten!
                    // j muss weitergezählt werden, also weiterer Durchlauf 
                    //der for-Schleife, keine weitere Aktio n nötig
                    }
                    else if (balkenNDneu<=balkenNDalt && i<11) {
                        //d.h. Balken j des neuen Intervalls ragt in i des Alten
                        //Intervalls hinein, j bekommt Anteil von i
                       double bchoose;
                       if (balkenNDneu<altnd){bchoose=balkenNDneu;}
                       else {bchoose=altnd;}
                      
                       gewichtneu=gewichtneu+((bchoose- 
                               balkenSTalt)/balkenbreiteHistZGalt )*
                               HistogrammZdimG[i];
                       tmpH[j] = gewichtneu;
                       gewichtneu=0; 
                       //kann gesetzt werden, da ja da Ende des Balken j im 
                       //neuen Intervall erreicht wird Weiteres Gewicht aus 
                       //i muss in nächsten Balken j
                       off=true; //neuer j-Schleifendurchlauf soll ja starten!;
                    }
                    else if (balkenNDneu>=balkenNDalt && i<11){
                       double bchoose;
                       if (balkenSTalt>balkenSTneu){bchoose=balkenSTalt;}
                       else {bchoose=balkenSTneu;}
                       
                        gewichtneu=gewichtneu+((balkenNDalt-bchoose)/
                               balkenbreiteHistZGalt)*HistogrammZdimG[i];
                       i++; 
                       //j überspannt den Balken aus i vollständig. Aus dem 
                       //nächsten Balken aus i muss weiteres Gewicht addiert 
                       //werden, also muss i um 1 erhöht wrden.
                    }
                    else if (i>=11){
                        tmpH[j] = gewichtneu;
                        gewichtneu=0;
                        off=true; //neuer j-Schleifendurchlauf soll ja starten!
                    }

                }
                else if (flag) {  
                    //Intervall wird nach oben verlängert
                    if (balkenNDneu<=balkenNDalt && i<11) {
                        //d.h. Balken j des neuen Intervalls ragt in i des 
                        //Alten Intervalls hinein, j bekommt Anteil von i
                       double bchoose;
                       if (balkenNDneu<altnd){bchoose=balkenNDneu;}
                       else {bchoose=altnd;} 
                       gewichtneu=gewichtneu+((bchoose
                               -balkenSTalt)/balkenbreiteHistZGalt )
                               *HistogrammZdimG[i];
                       tmpH[j] = gewichtneu;
                       gewichtneu=0; //kann gesetzt werden, da ja da Ende des
                       //Balken j im neuen Intervall erreicht wird Weiteres 
                       //Gewicht aus i muss in nächsten Balken j
                       off=true; 
                       //neuer j-Schleifendurchlauf soll ja starten!
                    }
                    else if (balkenNDneu>=balkenNDalt && i<11){
                       double bchoose;
                       if (balkenSTalt>balkenSTneu){bchoose=balkenSTalt;}
                       else {bchoose=balkenSTneu;}
                        gewichtneu=gewichtneu+((balkenNDalt
                               -bchoose)/balkenbreiteHistZGalt)
                                *HistogrammZdimG[i];
                       i++; 
                       //j überspannt den Balken aus i vollständig. Aus dem 
                       //nächsten Balken aus i muss weiteres Gewicht addiert 
                       //werden, also muss i um 1 erhöht wrden.
                    }
                    else if (i>=11){
                        tmpH[j] = gewichtneu;
                        gewichtneu=0;
                        off=true; //neuer j-Schleifendurchlauf soll ja starten!
                    }
                  }
                }
              }        
             for (int j=1; j < 11; j++){
		HistogrammZdimG[j]=tmpH[j];
	     }
	   
    }
      
    void analyseHistXLog(){
    //unsigned 
    unsigned int bzg=0;
    //unsigned 
    for (unsigned int BZG=0;BZG<HistogramXdim.size();BZG++){ 
    //für jeden Balken d.h. BasisZellGröße für die Replikationsberechnung
    double Replikationsfaktor=0;
    double weight=1;
    double cases=1;
    int maxZellsCovered=2;
    bzg=bzg+1;
    //unsigned 
        for (unsigned int j=0;j<HistogramXdim.size();j++){ 
            //durchlauf durch jeden Balken zur BasisZellGröße
            if (bzg-1>j){
            Replikationsfaktor=Replikationsfaktor+(HistogramXdim[j]*1);
            }
            else if (bzg-1==j){
            for(double c=1; c<=cases; c++){
             weight=1;
               Replikationsfaktor=Replikationsfaktor+
                       (HistogramXdim[j]*weight*maxZellsCovered);
               maxZellsCovered++;
                    }
                }
            else if (bzg-1<j){
            cases = cases*2;
            for(double c=1; c<=cases; c++){
             weight=1/(cases);
               Replikationsfaktor=Replikationsfaktor+
                       (HistogramXdim[j]*weight*maxZellsCovered);
               maxZellsCovered++;
                    }
                }
            }
        RepHistX[BZG]=(int)Replikationsfaktor;
        }
      }
    
    void analyseHistYLog(){        
    //unsigned 
    unsigned int bzg=0;
    //unsigned 
    for (unsigned int BZG=0;BZG<HistogramYdim.size();BZG++){ 
    //für jeden Balken d.h. BasisZellGröße für die Replikationsberechnung
    double Replikationsfaktor=0;
    double weight=1;
    double cases=1;
    int maxZellsCovered=2;
    bzg=bzg+1;
    //unsigned 
        for (unsigned int j=0;j<HistogramYdim.size();j++){ 
            //durchlauf durch jeden Balken zur BasisZellGröße
            if (bzg-1>j){
            Replikationsfaktor=Replikationsfaktor+(HistogramYdim[j]*1);
            }
            else if (bzg-1==j){
         for(double c=1; c<=cases; c++){
            weight=1;
              Replikationsfaktor=Replikationsfaktor+
                      (HistogramYdim[j]*weight*maxZellsCovered);
              maxZellsCovered++;
                    }
                }
            else if (bzg-1<j){
                cases = cases*2;
                for(double c=1; c<=cases; c++){
                 weight=1/(cases);
                  Replikationsfaktor=Replikationsfaktor+
                          (HistogramYdim[j]*weight*maxZellsCovered);
                  maxZellsCovered++;
                    }
                }
            }
        RepHistY[BZG]=(int)Replikationsfaktor;
        }
      }
  
    void analyseHistZLog(){
    //unsigned 
    unsigned int bzg=0;
    //unsigned 
    for (unsigned int BZG=0;BZG<HistogramZdim.size();BZG++){ 
    //für jeden Balken d.h. BasisZellGröße für die Replikationsberechnung
    double Replikationsfaktor=0;
    double weight=1;
    double cases=1;
    int maxZellsCovered=2;
    bzg=bzg+1;
    //unsigned 
        for (unsigned int j=0;j<HistogramZdim.size();j++){ 
        //durchlauf durch jeden Balken zur BasisZellGröße
            if (bzg-1>j){
             Replikationsfaktor=Replikationsfaktor+(HistogramZdim[j]*1);
            }
            else if (bzg-1==j){
             for(double c=1; c<=cases; c++){
              weight=1;
               Replikationsfaktor=Replikationsfaktor+
                       (HistogramZdim[j]*weight*maxZellsCovered);
               maxZellsCovered++;
                    }
                }
            else if (bzg-1<j){
                cases = cases*2;
                for(double c=1; c<=cases; c++){
                  weight=1/(cases);
                    Replikationsfaktor=Replikationsfaktor+
                            (HistogramZdim[j]*weight*maxZellsCovered);
                    maxZellsCovered++;
                    }
                }
            }
        RepHistZ[BZG]=(int)Replikationsfaktor;
        }
      }

      void putDataToHistX (double a) {
      int hi = (int) lb(a);
      if (HistogramXdim.size()==0){
          HistogramXdim.resize(3);
      }

      if (hi<=0){
           HistogramXdim[0]=HistogramXdim[0]+1; 
        }
      else if (hi>HistogramXdim.size()-2){
           while(hi>HistogramXdim.size()-2){
              HistogramXdim.resize(HistogramXdim.size()+1);
              HistogramXdim[HistogramXdim.size()-2]=
                      HistogramXdim[HistogramXdim.size()-2]+1;
           }
           HistogramXdim[hi]=HistogramXdim[hi]+1;
        }
        else {
           HistogramXdim[hi]=HistogramXdim[hi]+1;
        }
      }
 
      void putDataToHistY (double a) {
      int hi = (int) lb(a);
      if (HistogramYdim.size()==0){
          HistogramYdim.resize(3);
      }
      if (hi<=0){
           HistogramYdim[0]=HistogramYdim[0]+1;
        }
        else if (hi>HistogramYdim.size()-2){
           while((hi>HistogramYdim.size()-2)){
              HistogramYdim.resize(HistogramYdim.size()+1);
              HistogramYdim[HistogramYdim.size()-2]=
                      HistogramYdim[HistogramYdim.size()-2]+1;
           }
           HistogramYdim[hi]=HistogramYdim[hi]+1;
        }
        else {
           HistogramYdim[hi]=HistogramYdim[hi]+1;
        }
      }
      
      void putDataToHistZ (double a) {
      int hi = (int) lb(a);
      if (HistogramZdim.size()==0){
          HistogramZdim.resize(3);
      }
      if (hi<=0){
           HistogramZdim[0]=HistogramZdim[0]+1; 
        }
        else if (hi>HistogramZdim.size()-2){
           while((hi>HistogramZdim.size()-2)){
              HistogramZdim.resize(HistogramZdim.size()+1);
              HistogramZdim[HistogramZdim.size()-2]=
                      HistogramZdim[HistogramZdim.size()-2]+1;
           }
           HistogramZdim[hi]=HistogramZdim[hi]+1;
        }
        else {
           HistogramZdim[hi]=HistogramZdim[hi]+1;
        }
      }
      
//----------------------------------------------------
      
void InfoGrepCellnumberG(double lbx, double rtx, double lby, 
        double rty, double lbz, double rtz,
        double gridXroot,double gridYroot, double gridZroot,
        double gridXwidth,double gridwidthy,double gridZwidth,
        int nx, int ny){

bool is3D=false;
int maxebene, power;
int LBX, LBY, LBZ, RTX, RTY, RTZ;
int cubeSpacing;  
int initialSpacing;
int layerSpacing;
int hierachicalBorder;
int hierachicalSteps;
int step;
int lsp;
typedef std::map<int, int> MapType;
MapType cellMap;
MapType::iterator cellMapIter;

hierachicalBorder=0;
hierachicalSteps=0;
if (nx>=1000 && nx<10000){
  cerr << "WARNING!! To use hiracical numbering the "
          "parameter has to be > 9999 "
 "as otherwise not all values can be defined. \n";
}
else {hierachicalSteps=nx/1000;
      hierachicalBorder=hierachicalSteps/10;
      hierachicalSteps=hierachicalSteps%10;
      nx=nx%1000;
}                

   
if (nx==0){layerSpacing=0;
           initialSpacing=0;
           cubeSpacing=0;}
else if (nx<10 && nx>0){initialSpacing=nx;
                        layerSpacing=0;
                        cubeSpacing=0;}
else if (nx>=10 && nx<100){initialSpacing=nx%10;
                           layerSpacing=(nx%100-nx%10)/10;
                           cubeSpacing=(int)nx/100;}
else if (nx>99 && nx<1000){initialSpacing=nx%10;
                           layerSpacing=(nx%100-nx%10)/10;
                           cubeSpacing=(int)nx/100;}
else {layerSpacing=0;
      initialSpacing=0;
      cubeSpacing=0;} 


if (hierachicalBorder>0 && initialSpacing==0){
  cerr << "WARNING!! To use hiracical numbering initialSpacing, "
 "has to be at least 1.\n";
}
if (is3D == false && hierachicalSteps>layerSpacing){
      cerr << "WARNING!! hirachicalSteps need to have  "
 "layerSpacing of at least same number! \n";
}
if (is3D == true && hierachicalSteps>cubeSpacing){
   cerr << "WARNING!! hirachicalSteps need to have  "
 "cubeSpacing of at least same number! \n";
}

//computation of the grid coordinates
power=0;
if (dim==3){is3D=true;}  

double simLBX=((lbx-gridXroot)/gridXwidth);
double simLBY=((lby-gridYroot)/gridwidthy);
double simRTX=((rtx-gridXroot)/gridXwidth);
double simRTY=((rty-gridYroot)/gridwidthy);
double simLBZ=((lbz-gridZroot)/gridZwidth);
double simRTZ=((rtz-gridZroot)/gridZwidth);

if (is3D) {
    if(maxebeneG3D(simLBX,simLBY,simLBZ)>=maxebeneG3D(simRTX,simRTY,simRTZ)){
    maxebene=maxebeneG3D(simLBX,simLBY,simLBZ);}
    else {maxebene=maxebeneG3D(simRTX,simRTY,simRTZ);}
}
else {
    if(maxebeneG2D(simLBX,simLBY)>=maxebeneG2D(simRTX,simRTY)){
    maxebene=maxebeneG2D(simLBX,simLBY);}
    else {maxebene=maxebeneG2D(simRTX,simRTY);}
}

//should not happen!!
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
//grid coordinates of left-bottom point and right-top point of the rectangle
//are computed
LBX= int (simLBX+power);
LBY= int (simLBY+power);
RTX= int (simRTX+power);
RTY= int (simRTY+power);

if (is3D){
LBZ= int (simLBZ+power);
RTZ= int (simRTZ+power);

}
int cellNum;
for (int x=LBX;x<=RTX;x++){
    for (int y=LBY;y<=RTY;y++){
    cellNum=-1;    
    if (dim==2){
        cout << (RTX-LBX+1)*(RTY-LBY+1) << endl;
        cellNum=cellnumberG2D(x,y,maxebene,layerSpacing,initialSpacing);

    }
    else if (dim==3){
    for (int z=LBZ;z<=RTZ;z++){        
        cellNum=cellnumberG3D(x,y,z,maxebene,
                cubeSpacing,layerSpacing,initialSpacing);
    }
    }
    if ((is3D==false &&  hierachicalBorder>0  && cellNum>=0 &&
            (RTX-LBX>=hierachicalBorder || 
            RTY-LBY>=hierachicalBorder))
    || (is3D==true &&  hierachicalBorder>0  && cellNum>=0 && 
            (RTX-LBX>=hierachicalBorder || 
            RTY-LBY>=hierachicalBorder || 
            RTZ-LBZ>=hierachicalBorder))){
    step=1; 
    while ((RTX-LBX>hierachicalBorder*step || 
            RTY-LBY>hierachicalBorder*step || 
            RTZ-LBZ>hierachicalBorder*step))
    {step=step*3;}
    step=std::min(step/3,hierachicalSteps);
    lsp=1;
    if (is3D){lsp=3;}
     
    cellNum=(cellNum-initialSpacing)/
            (pow(9*lsp,step)+pow(9*lsp,step-1)*layerSpacing*lsp+cubeSpacing);
    cellNum=(cellNum+1)*
            (pow(9*lsp,step)+pow(9*lsp,step-1)*layerSpacing*lsp+cubeSpacing);
    cellNum=cellNum+initialSpacing;
             
    if (cellMap.size()==0){
        cellMap.insert(MapType::value_type(cellNum, cellNum));}
    else {cellMapIter=cellMap.find(cellNum);
        if (cellMapIter == cellMap.end()){
            cellMap.insert(MapType::value_type(cellNum, cellNum));
        } 
        else {cellNum=0;}      
    }
    }
    //Cellnummer irgendwo eintragen!!!
    if (cellnumbers.size()==0){cellnumbers.resize((RTX-LBX+1)*(RTY-LBY+1));}
     if ((hierachicalSteps>0 && cellNum>0)||
             (hierachicalSteps==0 && cellNum>=0))
        cellnumbers[(RTY-LBY+1)*LBX-x+LBY-y]= cellNum;
    
}
}


}


//numberG3D compute the cellnumber in a cube of cells
int numberG3D (int bx, int by, int bz, int ebenen, int dim2, int cube){
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
    if (bz==2){cellziff=cellziff + 2*dim2;}
    else if (bz==0){cellziff=cellziff+1*dim2;}
    }
else if (ebenen==2){
    cellziff = cellziff*(3*dim2+cube);
    if (bz==2){cellziff=cellziff+(2*dim2)*27+(2*9*cube);}
    else if(bz==0){cellziff=cellziff+(1*dim2)*27+(1*9*cube);}
    }
else if (ebenen>2){
    int ebpowone=0;
    int ebpowtwo=0;
    if (ebenen-2>0){ebpowtwo=ebenen-2;}    
        cellziff = cellziff*((3*dim2)*(int)(pow(27,ebpowtwo))+
            (cube*(int)(pow(27,ebpowtwo))));
    if (bz==2){
        if (ebenen-1>0){ebpowone=ebenen-1;}
        cellziff=cellziff+2*dim2*(int)(pow(27,ebpowone))+
            (2*9*(int)(pow(27,ebpowone))*cube);}
    else if(bz==0){
        if (ebenen-1>0){ebpowone=ebenen-1;}
        if (ebenen-2>0){ebpowtwo=ebenen-2;}
        cellziff=cellziff+1*dim2*(int)(pow(27,ebpowone))+
            (1*9*(int)(pow(27,ebpowtwo))*cube);}
    }

return cellziff;
}

//numberG2D compute the cellnumber in a area of cells
int numberG2D (int bx, int by, int ebenen, int twodim){
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
cellziff= cellziff*twodim;
}
else if (ebenen>2){
int ebpowtwo=0;
if (ebenen-2>0){ebpowtwo=ebenen-2;}     
cellziff = cellziff*twodim*(int)(pow(9,ebpowtwo));
}
return cellziff;

}

//cellnumbering is done stepwise or each step in numbering 
//in the same manner.
//cellnumberG3D controls the different steps.
int cellnumberG3D (int ax, int ay, int az, int ebenen
                ,int cubesp,int layersp, int initialsp){
double cellnum=0;
int dim2effLayerSpacing=9+layersp;

for (int r=1;r<ebenen+1;r++){
    cellnum = cellnum+numberG3D((abs(ax))%3,(abs(ay))%3,(abs(az))%3,
            r,dim2effLayerSpacing,cubesp);
    ax=ax/3;
    ay=ay/3;
    az=az/3;
}
return (int) cellnum+initialsp;
}

//cellnumbering is done stepwise or each step in numbering in the same manner.
//cellnumberG2D controls the different steps.
int cellnumberG2D (int ax, int ay, int ebenen
                ,int layersp, int initialsp){
double cellnum=0; 
//add here the recommanded spacing between EVERY 
//G-shaped Layer (3 within a Würfel?!?
int dim2effLayerSpacing=9+layersp;

for (int r=1;r<ebenen+1;r++){
    cellnum = cellnum + numberG2D(((int)abs(ax))%3,((int)abs(ay))%3,
            r,dim2effLayerSpacing);
    ax=ax/3;
    ay=ay/3;
}
return (int) cellnum+initialsp;
}

//------------------------------------------------------------
//------------------------------------------------------------
      
void InfoGrepCellnumberZ(double lbx, double rtx, double lby, 
        double rty, double lbz, double rtz,
        double gridXroot, double gridYroot, double gridZroot,
        double gridXwidth,double gridYwidth, double gridZwidth, 
        int nx, int ny){

bool is3D=false;
int maxebene, power;
int LBX, LBY, LBZ, RTX, RTY, RTZ;
int cubeSpacing;  
int initialSpacing;
int layerSpacing;
int hierachicalBorder;
int hierachicalSteps;
int maxxyz;
int step;
int lsp;
typedef std::map<int, int> MapType;
MapType cellMap;
MapType::iterator cellMapIter;

hierachicalBorder=0;
hierachicalSteps=0;
if (nx>=1000 && nx<10000){
  cerr << "WARNING!! To use hiracical numbering the "
 "parameter has to be > 9999 "
 "as otherwise not all values can be defined. \n";
}
else {hierachicalSteps=nx/1000;
      hierachicalBorder=hierachicalSteps/10;
      hierachicalSteps=hierachicalSteps%10;
      nx=nx%1000;
}                

if (nx==0){layerSpacing=0;
           initialSpacing=0;
           cubeSpacing=0;}
else if (nx<10 && nx>0){initialSpacing=nx;
                        layerSpacing=0;
                        cubeSpacing=0;}
else if (nx>=10 && nx<100){initialSpacing=nx%10;
                           layerSpacing=(nx%100-nx%10)/10;
                           cubeSpacing=(int)nx/100;}
else if (nx>99 && nx<1000){initialSpacing=nx%10;
                           layerSpacing=(nx%100-nx%10)/10;
                           cubeSpacing=(int)nx/100;}
else {layerSpacing=0;
      initialSpacing=0;
      cubeSpacing=0;} 

if (hierachicalBorder>0 && initialSpacing==0){
  cerr << "WARNING!! To use hiracical numbering initialSpacing, "
 "has to be at least 1.\n";
}
if (is3D == false && hierachicalSteps>layerSpacing){
      cerr << "WARNING!! hirachicalSteps need to have  "
 "layerSpacing of at least same number! \n";
}
if (is3D == true && hierachicalSteps>cubeSpacing){
   cerr << "WARNING!! hirachicalSteps need to have  "
 "cubeSpacing of at least same number! \n";
}

//computation of the grid coordinates
if (dim==3){is3D=true;}
power=0;
   
double simLBX=((lbx-gridXroot)/gridXwidth);
double simLBY=((lby-gridYroot)/gridYwidth);
double simRTX=((rtx-gridXroot)/gridXwidth);
double simRTY=((rty-gridYroot)/gridYwidth);
double simLBZ=((lbz-gridZroot)/gridZwidth);
double simRTZ=((rtz-gridZroot)/gridZwidth);

if (is3D) {
maxebene=max(maxebeneZ3D(simLBX,simLBY,simLBZ),
              maxebeneZ3D(simRTX,simRTY,simRTZ));
}
else {
maxebene=max(maxebeneZ2D(simLBX,simLBY),
              maxebeneZ2D(simRTX,simRTY));
}

maxxyz=pow(2,maxebene);

//should not happen!!
if (maxebene==0){cerr <<"WARNING!! Calculation Error!\n";} 

//Make sure all possible cell numbers don't exceed the range
//of the integer type.
if (is3D){
long maxN = pow(8,maxebene);
if (maxN>INT_MAX){cerr << "WARNING!! The grid is too dense, "
 "part cell number may exceed the range of Integer type.\n";} 
}
else {
long maxN = pow(4,maxebene);
if (maxN>INT_MAX){cerr << "WARNING!! The grid is too dense, "
 "part cell number may exceed the range of Integer type.\n";} 
}


//grid coortinates of left-bottom point and right-top point og the rectangle
//are computed
if (simLBX<0){LBX = (int)(simLBX-1)+(int)(maxxyz/2);}
else {LBX = (int)(simLBX)+(int)(maxxyz/2);}
if (simLBY<0){LBY = (int)(simLBY-1)+(int)(maxxyz/2);}
else {LBY = (int)(simLBY)+(int)(maxxyz/2);}
if (simRTX<0){RTX = (int)(simRTX-1)+(int)(maxxyz/2);}
else {RTX = (int)(simRTX)+(int) (maxxyz/2);}
if (simRTY<0){RTY = (int)(simRTY-1)+(int)(maxxyz/2);}
else {RTY = (int)(simRTY)+(int)(maxxyz/2);}

if (is3D){
    if (simLBZ<0){LBZ = (int)(simLBZ-1)+(int)(maxxyz/2);}
    else {LBZ = (int)(simLBZ)+(int)(maxxyz/2);}
    if (simRTZ<0){RTZ = (int)simRTZ+(int)(maxxyz/2);}
    else {RTZ = (int)(simRTZ)+(int)(maxxyz/2);} 
}
  
int cellNum;
int maxEbene=-1;
double epsilon=0.0000003;

for (int x=LBX;x<=RTX;x++){
    for (int y=LBY;y<=RTY;y++){
    maxEbene=-1;
    cellNum=-1;    
    if (dim==2){
        cout << (RTX-LBX+1)*(RTY-LBY+1) << endl;
        maxEbene=maxebeneZ2D(
                x-((maxxyz-epsilon)/2),
                y-((maxxyz-epsilon)/2));
        cellNum=cellnumberZ2D(
                x-((maxxyz-epsilon)/2)+(int)pow(2,maxEbene-1),
                y-((maxxyz-epsilon)/2)+(int)pow(2,maxEbene-1),maxEbene,
                    layerSpacing,initialSpacing);

    }
    else if (dim==3){
        for (int z=LBZ;z<=RTZ;z++){
            maxEbene=maxebeneZ3D(
                x-((maxxyz-epsilon)/2),
                y-((maxxyz-epsilon)/2),
                z-((maxxyz-epsilon)/2));
            cellNum=cellnumberZ3D(
                x-((maxxyz-epsilon)/2)+(int)pow(2,maxEbene-1),
                y-((maxxyz-epsilon)/2)+(int)pow(2,maxEbene-1),
                z-((maxxyz-epsilon)/2)+(int)pow(2,maxEbene-1),maxEbene,
                    cubeSpacing,layerSpacing,initialSpacing);
            
        }
    }
    if ((is3D==false &&  hierachicalBorder>0  && cellNum>=0 &&
            (RTX-LBX>=hierachicalBorder || RTY-LBY>=hierachicalBorder))
    || (is3D==true &&  hierachicalBorder>0  && cellNum>=0 && 
            (RTX-LBX>=hierachicalBorder || RTY-LBY>=hierachicalBorder || 
            RTZ-LBZ>=hierachicalBorder))){
    step=1; 
    while ((RTX-LBX>hierachicalBorder*step || 
            RTY-LBY>hierachicalBorder*step || 
            RTZ-LBZ>hierachicalBorder*step))
    {step=step*3;}
    step=std::min(step/3,hierachicalSteps);
    lsp=1;
    if (is3D){lsp=3;} 
    cellNum=(cellNum-initialSpacing)/
            (pow(9*lsp,step)+pow(9*lsp,step-1)*layerSpacing*lsp+cubeSpacing);
    cellNum=(cellNum+1)*
            (pow(9*lsp,step)+pow(9*lsp,step-1)*layerSpacing*lsp+cubeSpacing);
    cellNum=cellNum+initialSpacing;        
    if (cellMap.size()==0){
        cellMap.insert(MapType::value_type(cellNum, cellNum));}
    else {cellMapIter=cellMap.find(cellNum);
        if (cellMapIter == cellMap.end()){
            cellMap.insert(MapType::value_type(cellNum, cellNum));
        } 
        else {cellNum=0;}      
    }
    }
    //Cellnummer irgendwo eintragen!!!
    if (cellnumbers.size()==0){cellnumbers.resize((RTX-LBX+1)*(RTY-LBY+1));}
        if ((hierachicalSteps>0 && cellNum>0)||
                (hierachicalSteps==0 && cellNum>=0))
        cellnumbers[(RTY-LBY+1)*LBX-x+LBY-y]= cellNum;
    
}
}


}

//numberZ3D compute the cellnumber in a cube of cells
int numberZ3D (int bx, int by, int bz, int ebenen, int dim3){
int cellziff = 0;

cellziff = bx+by;

if (ebenen==1){
    if (by==1){cellziff=cellziff+1;}
    if (bz==1){cellziff=cellziff+4;}
    }
else if (ebenen==2){
        if (by==1){cellziff=cellziff+1;}
        if (bz==1){cellziff=cellziff+4;}
        cellziff=cellziff*(dim3); 
    }
else if (ebenen>2){
        if (by==1){cellziff=cellziff+1;}
        if (bz==1){cellziff=cellziff+4;}
        cellziff=cellziff*(dim3)*(int)(pow(8,(max(0,ebenen-2)))); 
    }

return cellziff;
}

//numberZ2D compute the cellnumber in a area of cells
int numberZ2D (int bx, int by, int ebenen, int dim2){
int cellziff = 0;
cellziff= bx+by;

if (by==1){cellziff=cellziff+1;}

if (ebenen==2){
        cellziff=cellziff*dim2; 
    }
else if (ebenen>2){
        cellziff = cellziff*dim2*(int)(pow(4,(max(0,ebenen-2)))); 
    }

return cellziff;

}

//cellnumbering is done stepwise or each step in numbering 
//in the same manner.
//cellnumberZ3D controls the different steps.
int cellnumberZ3D (int ax, int ay, int az, int ebenen,
        int cubeSpacing,int layerSpacing, int initialSpacing){
double cellnum=0;
double offset=0;
int dim3effLayerSpacing=2*(4+layerSpacing)+cubeSpacing;

for (int r=1;r<ebenen+1;r++){
    cellnum = cellnum+numberZ3D((abs(ax))%2,(abs(ay))%2,(abs(az))%2,
            r,dim3effLayerSpacing);
    if ((int)(abs(az)%2)==1 && r==1){offset=layerSpacing;}
    ax=ax/2;
    ay=ay/2;
    az=az/2;
}
if (ebenen==1){
        offset=0;
    }
else {for (int r=1;r<ebenen;r++){
        offset=offset+pow(8,r);
        offset=offset+(pow(8,r-1)*cubeSpacing);
        offset=offset+(pow(8,r)*layerSpacing/4);
        }
    }
cellnum=cellnum+offset;
cellnum=cellnum+initialSpacing;
return (int) cellnum;

}

//cellnumbering is done stepwise or each step in numbering in the same manner.
//cellnumberZ2D controls the different steps.
int cellnumberZ2D (int ax, int ay, int ebenen, 
        int layerSpacing, int initialSpacing){
double cellnum=0;
double offset=0;
double dim2effLayerSpacing=4+layerSpacing;

for (int r=1;r<ebenen+1;r++){
    cellnum = cellnum + 
      numberZ2D(((int)abs(ax))%2,((int)abs(ay))%2,r,dim2effLayerSpacing);
    ax=ax/2;
    ay=ay/2;
}
if (ebenen==1){
        offset=0;
    }
else {for (int r=1;r<=ebenen-1;r++){
        offset=offset+pow(4,r);
        offset=offset+layerSpacing*pow(4,r-1);
        }
    }
cellnum=cellnum+offset;
cellnum=cellnum+initialSpacing;

return (int) cellnum;
}  
//--------------------------------------------------------------------------

};



/*
1.7.3 Value Mapping
 
*/

template <int dim, class LocalInfo>
int joinMMRTreeInfoGrepVM( Word* args, Word& result, int message,
                      Word& local, Supplier s ) {


   LocalInfo* li = (LocalInfo*) local.addr;
   switch (message){
     case OPEN : {
                   if(li){
                     delete li;
                     li = 0;
                     local.setAddr(0);
                   }
                   CcInt* Min = (CcInt*) args[4].addr;
                   CcInt* Max = (CcInt*) args[5].addr;
                   CcInt* MaxMem = (CcInt*) args[6].addr;
                   if(!Min->IsDefined() || !Max->IsDefined()){
                      return 0; // invalid option
                   }
                   int min = Min->GetIntval();
                   int max = Max->GetIntval();
                   if(min<2 || max < 2*min){
                       return 0;
                   } 
                   size_t maxMem = (qp->GetMemorySize(s) * 1024); // in kB
                   if(MaxMem->IsDefined() && MaxMem->GetValue() > 0){
                      maxMem = MaxMem->GetValue();
                   }
                   local.setAddr(new LocalInfo(args[0], args[1], min, 
                                               max, maxMem, 
                                      ((CcInt*)args[7].addr)->GetIntval(),
                                      ((CcInt*)args[8].addr)->GetIntval(),
                                       nl->Second(GetTupleResultType(s)) ));
                    return 0;
                 }

      case REQUEST : {
                    if(!li){
                      return CANCEL;
                    }
                    result.addr = li->next();
                    return result.addr?YIELD:CANCEL;

                 }
      case CLOSE :{
                   if(li){
                     delete li;
                     local.setAddr(0);
                   }
                   return 0;
                 }
   }
  return -1;
}

/*
1.7.4 Value Mapping Array

*/

ValueMapping InfoGrepSpatialJoinVM[] = {
         joinMMRTreeInfoGrepVM<2,InfoGrepSpatialJoinInfo<2> >,
         joinMMRTreeInfoGrepVM<3,InfoGrepSpatialJoinInfo<3> > 
      };



/*
1.7.5 Selection function

*/

int realJoinSelectInfoGrep(ListExpr args){
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  string name = nl->SymbolValue(nl->Third(args));

  ListExpr type;

  int index = listutils::findAttribute(attrList,name,type);
  assert(index>0);
  if(listutils::isKind(type,Kind::SPATIAL2D())){
     return 0;
  }
  if(listutils::isKind(type, Kind::SPATIAL3D())){
     return 1;
   }
   assert(false);
   return -1;
}


/*
1.7.6 Specification
*/

const string realJoinMMRTreeSpecInfoGrep  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> stream(tuple(A)) x stream(tuple(B)) x a_i x b_j x int"
    " x int [ x int]-> stream(tuple(X o Y)) </text---> "
    "<text>streamA streamB realJoinMMRTree[attrnameA, attrnameB, min,"
    " max, maxMem]"
    "  </text--->"
    "<text>Performes a spatial join on two streams. "
    "The attributes a_i and b_j must"
    " be of type rect or rect3 in A and B, respectively. "
    "The result is a stream of "
    "tuples with intersecting rectangles build as concatenation"
    " of the source tuples. min and max define the range for the number of"
    " entries within the nodes of the used rtree. maxMem is the maximum cache"
    " size of the tuple store for tuples coming from streamA."
    " If maxMem is omitted, "
    " The default value MaxMemPerOperator (see SecondoConfig.ini) is used."
    " </text--->"
    "<text>query strassen feed extend[B : bbox(.geoData] strassen "
    "feed extend[B : bbox(.geoData)] {a} realJoinMMRTree[B,B_a,8,16, 1024]"
    " count "
    " </text--->"
         ") )";

/*
1.7.7 Operator instance

*/
Operator InfoGrepSpatialJoin(
      "InfoGrepSpatialJoin",
       realJoinMMRTreeSpecInfoGrep,
       2, 
       InfoGrepSpatialJoinVM,
       realJoinSelectInfoGrep,
       realJoinMMRTreeTMInfoGrep
     );

/*
4.5.3 Definition of the operators

*/

struct cellnumberGInfo : OperatorInfo {

  cellnumberGInfo() : OperatorInfo()
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

struct cellnumberLineInfo : OperatorInfo {

  cellnumberLineInfo() : OperatorInfo()
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

struct cellnumberZInfo : OperatorInfo {

  cellnumberZInfo() : OperatorInfo()
  {
    name = "cellnumberZ";
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
    AddOperator(cellnumberLineInfo(), cellNumberLineVM, cellNumberLineTM);
    AddOperator(cellnumberGInfo(), cellNumberGVM, cellNumberGTM);
    AddOperator(cellnumberZInfo(), cellNumberZVM, cellNumberZTM);
    AddOperator(&InfoGrepSpatialJoin);
        InfoGrepSpatialJoin.SetUsesMemory();
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

