/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[NP] [\newpage]
//[ue] [\"u]
//[e] [\'e]

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
 
 01590 Fachpraktikum "Erweiterbare Datenbanksysteme" 
 WS 2014 / 2015

Jens Breit, Joachim Dechow, Daniel Fuchs, Simon Jacobi, G[ue]nther Milosits, 
Daijun Nagamine, Hans-Joachim Klauke.

Betreuer: Dr. Thomas Behr, Fabio Vald[e]s


[1] Implementation of a Spatial3D algebra: createCube, createCylinder,
createCone,createSphere


[TOC]

[NP]

1 Includes and Defines

*/


#include <cmath>
#include <algorithm>    // std::min
#include "Matrix4x4.h"
#include "Spatial3D.h"
#include "Spatial3DTransformations.h"
#include "RelationAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace std;

namespace spatial3DCreate {

/*
1 Some auxiliary functions for the operators:

1.1 ~addRectangle~

Adds a rectangle to a volume.
 
*/
void addRectangle(const Point3d& a,const Point3d& b,
                  const Point3d& c,const Point3d& d,Volume3d& res){
  res.add(Triangle(a,b,c));
  res.add(Triangle(a,c,d));
}

/*
1.2 ~createCube~

Creates a cube.
 
*/
void createCube(Point3d* p1,Point3d* p2,Volume3d& res){
    res.clear();
    double xMin = min(p1->getX(),p2->getX());
    double xMax = max(p1->getX(),p2->getX());
    double yMin = min(p1->getY(),p2->getY());
    double yMax = max(p1->getY(),p2->getY());
    double zMin = min(p1->getZ(),p2->getZ());
    double zMax = max(p1->getZ(),p2->getZ());

    if(xMin == xMax || yMin == yMax || zMin == zMax){
       res.SetDefined(false);
      return;
    }
      
    res.startBulkLoad();

    addRectangle(Point3d(xMin,yMax,zMax),Point3d(xMin,yMin, zMax),
                 Point3d(xMax,yMin,zMax),Point3d(xMax,yMax,zMax), res);
    
    addRectangle(Point3d(xMin,yMax,zMin),Point3d(xMax,yMax,zMin),
                 Point3d(xMax,yMin,zMin),Point3d(xMin,yMin,zMin), res);
    
    addRectangle(Point3d(xMin,yMax,zMax),Point3d(xMin,yMax,zMin),
                 Point3d(xMin,yMin,zMin),Point3d(xMin,yMin,zMax), res);
    
    addRectangle(Point3d(xMax,yMax,zMax),Point3d(xMax,yMin,zMax),
                 Point3d(xMax,yMin,zMin),Point3d(xMax,yMax,zMin), res);
    
    addRectangle(Point3d(xMin,yMax,zMax),Point3d(xMax,yMax,zMax),
                 Point3d(xMax,yMax,zMin),Point3d(xMin,yMax,zMin), res);
    
    addRectangle(Point3d(xMin,yMin,zMax),Point3d(xMin,yMin,zMin),
                 Point3d(xMax,yMin,zMin),Point3d(xMax,yMin,zMax), res);

    res.endBulkLoad(NO_REPAIR);
}


/*
1.3 ~createCylinder~

Creates a cylinder.
 
*/
void createCylinder(Point3d* p1,double r, double h, int corners,Volume3d& res){
  res.clear();
  if(corners < 3){
    res.SetDefined(false);
    return;
  }
  
  res.startBulkLoad();
  
  double cx = p1->getX();
  double cy = p1->getY();
  double cz = p1->getZ();
  
  for(int i = 0; i < corners; i++){
    double x1 = cos(i * 2 * M_PI / corners) * r + cx;
    double y1 = sin(i * 2 * M_PI / corners) * r + cy;
    double x2 = cos((i+1) * 2 * M_PI / corners) * r + cx;
    double y2 = sin((i+1) * 2 * M_PI / corners) * r + cy;
    res.add(Triangle(Point3d(cx,cy,cz),
                      Point3d(x2,y2,cz),
                      Point3d(x1,y1,cz)));
    res.add(Triangle(Point3d(cx,cy,cz + h),
                      Point3d(x1,y1,cz + h),
                      Point3d(x2,y2,cz + h)));
    addRectangle(Point3d(x1,y1,cz),
                  Point3d(x2,y2,cz),
                  Point3d(x2,y2,cz + h),
                  Point3d(x1,y1,cz + h), res);
  }
  res.endBulkLoad(NO_REPAIR);
}

/*
1.4 ~createCone~

Creates a cone.
 
*/
void createCone(Point3d* p1,double r, double h, int corners,Volume3d& res){
    res.clear();
    if(corners < 3){
      res.SetDefined(false);
      return;
    }

    res.startBulkLoad();
    
    double cx = p1->getX();
    double cy = p1->getY();
    double cz = p1->getZ();
    
    for(int i = 0; i < corners; i++){
      double x1 = cos(i * 2 * M_PI / corners) * r + cx;
      double y1 = sin(i * 2 * M_PI / corners) * r + cy;
      double x2 = cos((i+1) * 2 * M_PI / corners) * r + cx;
      double y2 = sin((i+1) * 2 * M_PI / corners) * r + cy;
      res.add(Triangle(Point3d(cx,cy,cz),
                       Point3d(x2,y2,cz),
                       Point3d(x1,y1,cz)));
      res.add(Triangle(Point3d(cx,cy,cz + h),
                       Point3d(x1,y1,cz),
                       Point3d(x2,y2,cz)));
    }
    res.endBulkLoad(NO_REPAIR);
}


/*
1.5 ~createSphere~

Creates a sphere.
 
*/
void createSphere(Point3d* p1,double r, int corners,Volume3d& res){
  res.clear();
  if(corners < 3){
    res.SetDefined(false);
    return;
  }
  if(corners % 2 != 0){
    corners++;
  }

  res.startBulkLoad();
  double cx = p1->getX();
  double cy = p1->getY();
  double cz = p1->getZ() + r;
  
  for(int i = 1; i <= corners / 2 - 1; i++){
    double y = cos(i * 2 * M_PI / corners) * r + cy;
    double r2 = sin(i * 2 * M_PI / corners) * r;
    for(int j = 1; j <= corners; j++){
      double z = sin(j * 2 * M_PI / corners) * r2 + cz;
      double x = cos(j * 2 * M_PI / corners) * r2 + cx;
      double zPrev = sin((j - 1) * 2 * M_PI / corners) * r2 + cz;
      double xPrev = cos((j - 1) * 2 * M_PI / corners) * r2 + cx;
      if(i == 1){
        res.add(Triangle(Point3d(cx,cy + r,cz),
                        Point3d(x,y,z),
                        Point3d(xPrev,y,zPrev)));
      }
      if(i > 1){
      double yPrev = cos((i - 1) * 2 * M_PI / corners) * r + cy;
      double r2Prev = sin((i - 1) * 2 * M_PI / corners) * r;
      double zOld = sin(j * 2 * M_PI / corners) * r2Prev + cz;
      double xOld = cos(j * 2 * M_PI / corners) * r2Prev + cx;
      double zOldPrev = sin((j - 1) * 2 * M_PI / corners) * r2Prev + cz;
      double xOldPrev = cos((j - 1) * 2 * M_PI / corners) * r2Prev + cx;
      addRectangle(Point3d(x,y,z),
                  Point3d(xPrev,y,zPrev),
                  Point3d(xOldPrev,yPrev,zOldPrev),
                  Point3d(xOld,yPrev,zOld), res);
      }
      if(i == corners / 2 - 1){
        res.add(Triangle(Point3d(cx,cy - r,cz),
                        Point3d(xPrev,y,zPrev),
                        Point3d(x,y,z)));
      }
    }
  }
  res.endBulkLoad(NO_REPAIR);
}

    
/*
10 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

10.1 Type mapping functions

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

10.1.1 Type mapping function Spatial3DCreateCubeMap.

This type mapping function is used for the ~creatCube~ operator.

*/
  ListExpr Spatial3DCreateCubeMap( ListExpr args )
  {
    if ( nl->ListLength( args ) != 2 )
    { ErrorReporter::ReportError("wrong number of arguments (2 expected)");
      return nl->TypeError();
    }
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    if(nl->AtomType(arg1)!=SymbolType){
      ErrorReporter::ReportError("point3D x point3d expected");
      return nl->TypeError();
    }
    string st = nl->SymbolValue(arg1);
    if( st!=Point3d::BasicType()){
      ErrorReporter::ReportError("point3D x point3d expected");
      return nl->TypeError();
    }
    if(nl->AtomType(arg2)!=SymbolType){
      ErrorReporter::ReportError("point3D x point3d expected");
      return nl->TypeError();
    }
    st = nl->SymbolValue(arg2);
    if( st!=Point3d::BasicType()){
      ErrorReporter::ReportError("point3D x point3d expected");
      return nl->TypeError();
    }

    st = Volume3d::BasicType();
    return nl->SymbolAtom(st);
  }
  
/*  
10.1.2 Type mapping function Spatial3DCreateCylinderOrConeMap

This type mapping function is used for the ~createCylinder~ 
and  ~createCone~ operators.

*/
  ListExpr Spatial3DCreateCylinderOrConeMap( ListExpr args )
  {
    if ( nl->ListLength( args ) != 4 )
    { ErrorReporter::ReportError("wrong number of arguments (4 expected)");
      return nl->TypeError();
    }
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);
    ListExpr arg4 = nl->Fourth(args);

    if(nl->AtomType(arg1)!=SymbolType){
      ErrorReporter::ReportError("point3D x real  x real x int expected");
      return nl->TypeError();
    }
    string st = nl->SymbolValue(arg1);
    if( st!=Point3d::BasicType()){
      ErrorReporter::ReportError("point3D x real  x real x int expected");
      return nl->TypeError();
    }

    if(!nl->IsEqual(arg2,CcReal::BasicType())){
      ErrorReporter::ReportError("point3D x real  x real x int expected");
      return nl->TypeError();
    }    
    
    if(!nl->IsEqual(arg3,CcReal::BasicType())){
      ErrorReporter::ReportError("point3D x real  x real x int expected");
      return nl->TypeError();
    }    
    
    if(!nl->IsEqual(arg4,CcInt::BasicType())){
      ErrorReporter::ReportError("point3D x real  x real x int expected");
      return nl->TypeError();
    }    

    st = Volume3d::BasicType();
    return nl->SymbolAtom(st);
  }
  
  
/*  
10.1.3 Type mapping function Spatial3DCreateSphereMap

This type mapping function is used for the ~createSphere~ operator.

*/
  ListExpr Spatial3DCreateSphereMap( ListExpr args )
  {
    if ( nl->ListLength( args ) != 3 )
    { ErrorReporter::ReportError("wrong number of arguments (3 expected)");
      return nl->TypeError();
    }
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);

    if(nl->AtomType(arg1)!=SymbolType){
      ErrorReporter::ReportError("point3D x real x int expected");
      return nl->TypeError();
    }
    string st = nl->SymbolValue(arg1);
    if( st!=Point3d::BasicType()){
      ErrorReporter::ReportError("point3D x real x int expected");
      return nl->TypeError();
    }

    if(!nl->IsEqual(arg2,CcReal::BasicType())){
      ErrorReporter::ReportError("point3D x real x int expected");
      return nl->TypeError();
    }    
    
    if(!nl->IsEqual(arg3,CcInt::BasicType())){
      ErrorReporter::ReportError("point3D x real x int expected");
      return nl->TypeError();
    }    

    st = Volume3d::BasicType();
    return nl->SymbolAtom(st);
  }
  
/*
10.4 Value mapping functions
  
10.4.1 Value mapping functions of operator ~createCube~

*/
  int Spatial3DCreateCube( Word* args, Word& result, int message,
                      Word& local, Supplier s ){
    result = qp->ResultStorage(s);
    Volume3d* res = static_cast<Volume3d*>(result.addr);
    Point3d* p1 = static_cast<Point3d*>(args[0].addr);
    Point3d* p2 = static_cast<Point3d*>(args[1].addr);
    if(!p1->IsDefined() || !p2->IsDefined()){
        res->SetDefined(false);
        return 0;
    }
    createCube(p1,p2,*res);
    return 0;
  }
  
  int Spatial3DCreateCylinder( Word* args, Word& result, int message,
                      Word& local, Supplier s ){
    result = qp->ResultStorage(s);
    Volume3d* res = static_cast<Volume3d*>(result.addr);
    Point3d* p = static_cast<Point3d*>(args[0].addr);
    CcReal* cr = static_cast<CcReal*>(args[1].addr);
    CcReal* ch = static_cast<CcReal*>(args[2].addr);
    CcInt* cc = static_cast<CcInt*>(args[3].addr);
    if(!p->IsDefined() || !cr->IsDefined()
       || !ch->IsDefined() || !cc->IsDefined()
    ){
        res->SetDefined(false);
        return 0;
    }
    double r = cr->GetRealval();
    double h = ch->GetRealval();
    int c = cc->GetIntval();
    
    createCylinder(p,r,h,c,*res);
    return 0;
  }
      
  int Spatial3DCreateCone( Word* args, Word& result, int message,
                      Word& local, Supplier s ){
    result = qp->ResultStorage(s);
    Volume3d* res = static_cast<Volume3d*>(result.addr);
    Point3d* p = static_cast<Point3d*>(args[0].addr);
    CcReal* cr = static_cast<CcReal*>(args[1].addr);
    CcReal* ch = static_cast<CcReal*>(args[2].addr);
    CcInt* cc = static_cast<CcInt*>(args[3].addr);
    if(!p->IsDefined() || !cr->IsDefined()
       || !ch->IsDefined() || !cc->IsDefined()
    ){
        res->SetDefined(false);
        return 0;
    }
    double r = cr->GetRealval();
    double h = ch->GetRealval();
    int c = cc->GetIntval();
    
    createCone(p,r,h,c,*res);
    return 0;
  }

  int Spatial3DCreateSphere( Word* args, Word& result, int message,
                      Word& local, Supplier s ){
    result = qp->ResultStorage(s);
    Volume3d* res = static_cast<Volume3d*>(result.addr);
    Point3d* p = static_cast<Point3d*>(args[0].addr);
    CcReal* cr = static_cast<CcReal*>(args[1].addr);
    CcInt* cc = static_cast<CcInt*>(args[2].addr);
    if(!p->IsDefined() || !cr->IsDefined()|| !cc->IsDefined()){
        res->SetDefined(false);
        return 0;
    }
    double r = cr->GetRealval();
    int c = cc->GetIntval();
    
    createSphere(p,r,c,*res);
    return 0;
  }
  
/*
10.5 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

10.5.2 Definition of specification strings

*/
  const string Spatial3DSpecCreateCube  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>(point3D x point3D) -> volume3d</text--->"
    "<text> createCube(p1,p2)</text--->"
    "<text> creates a cube with he corner p1 and p2</text--->"
    "<text> query createCube(point1,point2]</text--->"
    ") )";

  const string Spatial3DSpecCreateCylinder  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>point3D x real x real x int) -> volume3d</text--->"
    "<text> createCylinder(p,r,h,c)</text--->"
    "<text> creates a cylinder with radius r and hight h</text--->"
    "<text> query createCylinder(point, radius, height, corner)</text--->"
    ") )";

  const string Spatial3DSpecCreateCone  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>point3D x real x real x int) -> volume3d</text--->"
    "<text> createCone(p,r,h,c)</text--->"
    "<text> creates a cone with radius r and hight h</text--->"
    "<text> query createCone(point, radius, height, corner)</text--->"
    ") )";

  const string Spatial3DSpecCreateSphere  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>point3D x real x int) -> volume3d</text--->"
    "<text> createSphere(p,r,h,c)</text--->"
    "<text> creates a sphere with radius r</text--->"
    "<text> query createCone(point, radius, corner)</text--->"
    ") )";

/*
10.5.3 Definition of the operators

*/
 
  Operator* getCreateCubePtr(){
    return new Operator(
    "createCube",
    Spatial3DSpecCreateCube,
    Spatial3DCreateCube,
    Operator::SimpleSelect,
    Spatial3DCreateCubeMap
   );
  }
 
  Operator* getCreateCylinderPtr(){
    return new Operator(
    "createCylinder",
    Spatial3DSpecCreateCylinder,
    Spatial3DCreateCylinder,
    Operator::SimpleSelect,
    Spatial3DCreateCylinderOrConeMap
   );
  }
 
  Operator* getCreateConePtr(){
    return new Operator(
    "createCone",
    Spatial3DSpecCreateCone,
    Spatial3DCreateCone,
    Operator::SimpleSelect,
    Spatial3DCreateCylinderOrConeMap
   );
  }
 
  Operator* getCreateSpherePtr(){
    return new Operator(
    "createSphere",
    Spatial3DSpecCreateSphere,
    Spatial3DCreateSphere,
    Operator::SimpleSelect,
    Spatial3DCreateSphereMap
   );
  }

}
  
