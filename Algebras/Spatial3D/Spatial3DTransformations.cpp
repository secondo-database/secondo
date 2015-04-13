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


[1] Implementation of a Spatial3D algebra: 
rotate, mirror, translate, scaleDir, scale

[TOC]

[NP]

1 Includes and Defines

*/

#include <cmath>
#include "Matrix4x4.h"
#include "Spatial3D.h"
#include "Spatial3DTransformations.h"
#include "RelationAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace std;

namespace spatial3DTransformations {

/*
1 Some auxiliary functions for the transformations:

1.1 ~multiplyMatrix4x4WithPoint~

Multiplys the Point with the matrix.
 
*/
  void multiplyMatrix4x4WithPoint(Matrix4x4* matrix, Point3d* point)
  {
    double newX = point->getX() * matrix->values[0][0] 
      + point->getY() * matrix->values[0][1] 
      + point->getZ() * matrix->values[0][2] 
      + 1 * matrix->values[0][3];
    double newY = point->getX() * matrix->values[1][0] 
      + point->getY() * matrix->values[1][1] 
      + point->getZ() * matrix->values[1][2] 
      + 1 * matrix->values[1][3];
    double newZ = point->getX() * matrix->values[2][0] 
      + point->getY() * matrix->values[2][1] 
      + point->getZ() * matrix->values[2][2] 
      + 1 * matrix->values[2][3];

    point->set(newX, newY, newZ);
  };

  void multiplyMatrix4x4WithSimplePoint(
    Matrix4x4* matrix, SimplePoint3d& point)
  {
    double newX = point.getX() * matrix->values[0][0] 
      + point.getY() * matrix->values[0][1] 
      + point.getZ() * matrix->values[0][2] 
      + 1 * matrix->values[0][3];
    double newY = point.getX() * matrix->values[1][0] 
      + point.getY() * matrix->values[1][1] 
      + point.getZ() * matrix->values[1][2] 
      + 1 * matrix->values[1][3];
    double newZ = point.getX() * matrix->values[2][0] 
      + point.getY() * matrix->values[2][1] 
      + point.getZ() * matrix->values[2][2] 
      + 1 * matrix->values[2][3];

    point.set(newX, newY, newZ);
  };

/*
1.2 ~rotatePoint~, ~rotateTriangleContainer~

Rotates the objects.
 
*/
  void rotatePoint(
    Point3d* point,const Point3d* center, const Vector3d* axis, 
    double angle,Point3d& res)
  {
    Matrix4x4* matrix = 
      Matrix4x4::GetRotationMatrix(center->getX(),center->getY(),
        center->getZ(),axis->getX(),axis->getY(),
        axis->getZ(),angle);
    multiplyMatrix4x4WithPoint(matrix,point);
    res.set(point);
    delete matrix;
  };
  
  void rotateTriangleContainer(TriangleContainer* container, 
    const Point3d* center, const Vector3d* axis, 
    double angle,TriangleContainer& res)
  {
    Matrix4x4* matrix = 
      Matrix4x4::GetRotationMatrix(center->getX(),center->getY(),
        center->getZ(),axis->getX(),axis->getY(),
        axis->getZ(),angle);
    int size = container->size();
    res.clear();
    res.startBulkLoad();
    for(int i = 0; i < size; i++){
      Triangle triangle = container->get(i);
      SimplePoint3d pointA = triangle.getA();
      SimplePoint3d pointB = triangle.getB();
      SimplePoint3d pointC = triangle.getC();
      multiplyMatrix4x4WithSimplePoint(matrix,pointA);
      multiplyMatrix4x4WithSimplePoint(matrix,pointB);
      multiplyMatrix4x4WithSimplePoint(matrix,pointC);
      res.add(Triangle(pointA,pointB,pointC));
    }
    res.endBulkLoad(NO_REPAIR);
    delete matrix;
  };

/*
1.3 ~mirrorPoint~, ~mirrorTriangleContainer~

Mirrors the objects.
 
*/
  void mirrorPoint(Point3d* point, const Plane3d* plane,Point3d& res)
  {
    Matrix4x4* matrix = 
      Matrix4x4::GetMirrorMatrix(
        plane->getPoint().getX(),
        plane->getPoint().getY(),
        plane->getPoint().getZ(),
        plane->getNormalVector().getX(),
        plane->getNormalVector().getY(),
        plane->getNormalVector().getZ());

    multiplyMatrix4x4WithPoint(matrix,point);
    res.set(point);
    delete matrix;
  };

  void mirrorTriangleContainer(TriangleContainer* container, 
    const Plane3d* plane, TriangleContainer& res)
  {
    Matrix4x4* matrix = 
      Matrix4x4::GetMirrorMatrix(
        plane->getPoint().getX(),
        plane->getPoint().getY(),
        plane->getPoint().getZ(),
        plane->getNormalVector().getX(),
        plane->getNormalVector().getY(),
        plane->getNormalVector().getZ());
      
    int size = container->size();
    res.clear();
    res.startBulkLoad();
    for(int i = 0; i < size; i++){
      Triangle triangle = container->get(i);
      SimplePoint3d pointA = triangle.getA();
      SimplePoint3d pointB = triangle.getB();
      SimplePoint3d pointC = triangle.getC();
      multiplyMatrix4x4WithSimplePoint(matrix,pointA);
      multiplyMatrix4x4WithSimplePoint(matrix,pointB);
      multiplyMatrix4x4WithSimplePoint(matrix,pointC);
      res.add(Triangle(pointA,pointC,pointB));
    }
    res.endBulkLoad(NO_REPAIR);
    delete matrix;
  };

/*
1.4 ~translatePoint~,  ~translateTriangleContainer~

Translates the objects.
 
*/
  void translatePoint(
    Point3d* point, const Vector3d* translation,Point3d& res)
  {
    Matrix4x4* matrix = 
      Matrix4x4::GetTranslateMatrix(
        translation->getX(),translation->getY(),translation->getZ());
      
    multiplyMatrix4x4WithPoint(matrix,point);
    res.set(point);
    delete matrix;
  };

  void translateTriangleContainer(
    TriangleContainer* container, 
    const Vector3d* translation,TriangleContainer& res)
  {
    Matrix4x4* matrix = 
      Matrix4x4::GetTranslateMatrix(
        translation->getX(),translation->getY(),translation->getZ());
      
    int size = container->size();
  
    res.clear();
    res.startBulkLoad();
    for(int i = 0; i < size; i++){
      Triangle triangle = container->get(i);
      SimplePoint3d pointA = triangle.getA();
      SimplePoint3d pointB = triangle.getB();
      SimplePoint3d pointC = triangle.getC();
      multiplyMatrix4x4WithSimplePoint(matrix,pointA);
      multiplyMatrix4x4WithSimplePoint(matrix,pointB);
      multiplyMatrix4x4WithSimplePoint(matrix,pointC);
      res.add(Triangle(pointA,pointB,pointC));
    }
    res.endBulkLoad(NO_REPAIR);
    delete matrix;
  };


/*
1.5 ~scaleDirPoint~, ~scaleDirTriangleContainer~, ~scaleDirPoint~, 
~scaleDirTriangleContainer~

Scales the objects.
 
*/ 
  void scaleDirPoint(
    Point3d* point, const Point3d* center, 
    const Vector3d* direction,Point3d& res)
  {
    Matrix4x4* matrix = 
      Matrix4x4::GetScaleDirMatrix(
        center->getX(),center->getY(), center->getZ(),
        direction->getX(),direction->getY(), direction->getZ());
      
    multiplyMatrix4x4WithPoint(matrix,point);
    res.set(point);
    delete matrix;
  };

  void scaleDirTriangleContainer(TriangleContainer* container, 
    const Point3d* center,  
    const Vector3d* direction,TriangleContainer& res)
  {
    Matrix4x4* matrix = 
      Matrix4x4::GetScaleDirMatrix(
        center->getX(),center->getY(), center->getZ(),
        direction->getX(),direction->getY(), direction->getZ());
      
    int size = container->size();
    res.clear();
    res.startBulkLoad();
    for(int i = 0; i < size; i++){
      Triangle triangle = container->get(i);
      SimplePoint3d pointA = triangle.getA();
      SimplePoint3d pointB = triangle.getB();
      SimplePoint3d pointC = triangle.getC();
      multiplyMatrix4x4WithSimplePoint(matrix,pointA);
      multiplyMatrix4x4WithSimplePoint(matrix,pointB);
      multiplyMatrix4x4WithSimplePoint(matrix,pointC);
      res.add(Triangle(pointA,pointB,pointC));
    }
    res.endBulkLoad(NO_REPAIR);
    delete matrix;
  };

  void scalePoint(Point3d* point, const Point3d* center,  
    double factor,Point3d& res)
  {
    Matrix4x4* matrix = 
      Matrix4x4::GetScaleMatrix(
        center->getX(),center->getY(), center->getZ(),factor);
      
    multiplyMatrix4x4WithPoint(matrix,point);
    res.set(point);
    delete matrix;
  };
  
  void scaleTriangleContainer(TriangleContainer* container, 
    const Point3d* center,double factor,TriangleContainer& res)
  {
    Matrix4x4* matrix = 
      Matrix4x4::GetScaleMatrix(
        center->getX(),center->getY(), center->getZ(),factor);
      
    int size = container->size();
    res.clear();
    res.startBulkLoad();
    for(int i = 0; i < size; i++){
      Triangle triangle = container->get(i);
      SimplePoint3d pointA = triangle.getA();
      SimplePoint3d pointB = triangle.getB();
      SimplePoint3d pointC = triangle.getC();
      multiplyMatrix4x4WithSimplePoint(matrix,pointA);
      multiplyMatrix4x4WithSimplePoint(matrix,pointB);
      multiplyMatrix4x4WithSimplePoint(matrix,pointC);
      res.add(Triangle(pointA,pointB,pointC));
    }
    res.endBulkLoad(NO_REPAIR);
    delete matrix;
  };

    
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

10.1.1 Type mapping function Spatial3DRotateMap

This type mapping function is used for the ~rotate~ operator.
The mamp is spatial3Dtype x point3D x vector3D x real -> spatial3Dtype

*/
  ListExpr Spatial3DRotateMap( ListExpr args )
  {
    if ( nl->ListLength( args ) != 4 )
    { ErrorReporter::ReportError("wrong number of arguments (4 expected)");
      return nl->TypeError();
    }
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);
    ListExpr arg4 = nl->Fourth(args);

    if(nl->AtomType(arg2)!=SymbolType){
      ErrorReporter::ReportError(
        "spatial3Dtype x point3D x vector3D x real expected");
      return nl->TypeError();
    }
    string st = nl->SymbolValue(arg2);
    if( st!=Point3d::BasicType()){
      ErrorReporter::ReportError(
        "spatial3Dtype x point3D x vector3D x real expected");
      return nl->TypeError();
    }

    if(nl->AtomType(arg3)!=SymbolType){
      ErrorReporter::ReportError(
        "spatial3Dtype x point3D x vector3D x real expected");
      return nl->TypeError();
    }
    st = nl->SymbolValue(arg3);
    if( st!=Vector3d::BasicType()){
      ErrorReporter::ReportError(
        "spatial3Dtype x point3D x vector3D x real expected");
      return nl->TypeError();
    }    

    if(!nl->IsEqual(arg4,CcReal::BasicType())){
      ErrorReporter::ReportError(
        "spatial3Dtype x point3D x vector3D x real expected");
      return nl->TypeError();
    }    

    if(nl->AtomType(arg1)!=SymbolType){
      ErrorReporter::ReportError(
        "spatial3Dtype x point3D x vector3D x real expected");
      return nl->TypeError();
    }
    st = nl->SymbolValue(arg1);
    if( st!=Point3d::BasicType() && st!=Surface3d::BasicType() &&
        st!=Volume3d::BasicType()){
      ErrorReporter::ReportError(
        "spatial3Dtype x point3D x vector3D x real expected");
      return nl->TypeError();
    }

    return nl->SymbolAtom(st);
  }
  
/*  
10.1.2 Type mapping function Spatial3DMirrorMap

This type mapping function is used for the ~mirror~ operator.
The mamp is spatial3Dtype x plane3D -> spatial3Dtype

*/
  ListExpr Spatial3DMirrorMap( ListExpr args )
  {
    if ( nl->ListLength( args ) != 2 )
    { ErrorReporter::ReportError("wrong number of arguments (2 expected)");
      return nl->TypeError();
    }
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    if(nl->AtomType(arg2)!=SymbolType){
      ErrorReporter::ReportError("spatial3Dtype x plane3D expected");
      return nl->TypeError();
    }
    string st = nl->SymbolValue(arg2);
    if( st!=Plane3d::BasicType()){
      ErrorReporter::ReportError("spatial3Dtype x plane3D expected");
      return nl->TypeError();
    }

    if(nl->AtomType(arg1)!=SymbolType){
      ErrorReporter::ReportError("spatial3Dtype x plane3D expected");
      return nl->TypeError();
    }
    st = nl->SymbolValue(arg1);
    if( st!=Point3d::BasicType() && st!=Surface3d::BasicType() &&
        st!=Volume3d::BasicType()){
      ErrorReporter::ReportError("spatial3Dtype x plane3D expected");
      return nl->TypeError();
    }

    return nl->SymbolAtom(st);
  }
  
/*  
10.1.3 Type mapping function Spatial3DTranslateMap

This type mapping function is used for the ~translate~ operator.
The mamp is spatial3Dtype x vector3D -> spatial3Dtype

*/
  ListExpr Spatial3DTranslateMap( ListExpr args )
  {
    if ( nl->ListLength( args ) != 2 )
    { ErrorReporter::ReportError("wrong number of arguments (2 expected)");
      return nl->TypeError();
    }
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    if(nl->ListLength( arg2 ) != 1){
      ErrorReporter::ReportError("spatial3Dtype x vector3D expected1");
      return nl->TypeError();
    }
    if(nl->AtomType(nl->First( arg2 ))!=SymbolType){
      ErrorReporter::ReportError("spatial3Dtype x vector3D expected5");
      return nl->TypeError();
    }
    string st = nl->SymbolValue(nl->First( arg2 ));
    if( st != Vector3d::BasicType()){
      ErrorReporter::ReportError("spatial3Dtype x vector3D expected2");
      return nl->TypeError();
    }

    if(nl->AtomType(arg1)!=SymbolType){
      ErrorReporter::ReportError("spatial3Dtype x vector3D expected3");
      return nl->TypeError();
    }
    st = nl->SymbolValue(arg1);
    if( st!=Point3d::BasicType() && st!=Surface3d::BasicType() &&
        st!=Volume3d::BasicType()){
      ErrorReporter::ReportError("spatial3Dtype x vector3D expected4");
      return nl->TypeError();
    }

    return nl->SymbolAtom(st);
  }
    
/*  
10.1.4 Type mapping function Spatial3DScaleDirMap

This type mapping function is used for the ~translate~ operator.
The mamp is spatial3Dtype x vector3D -> spatial3Dtype

*/
  ListExpr Spatial3DScaleDirMap( ListExpr args )
  {
    if ( nl->ListLength( args ) != 3 )
    { ErrorReporter::ReportError("wrong number of arguments (2 expected)");
      return nl->TypeError();
    }
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);

    if(nl->AtomType(arg2)!=SymbolType){
      ErrorReporter::ReportError(
        "spatial3Dtype x point3D x vector3D expected");
      return nl->TypeError();
    }
    string st = nl->SymbolValue(arg2);
    if( st!=Point3d::BasicType()){
      ErrorReporter::ReportError(
        "spatial3Dtype x point3D x vector3D expected");
      return nl->TypeError();
    }

    if(nl->AtomType(arg3)!=SymbolType){
      ErrorReporter::ReportError(
        "spatial3Dtype x point3D x vector3D expected");
      return nl->TypeError();
    }
    st = nl->SymbolValue(arg3);
    if( st!=Vector3d::BasicType()){
      ErrorReporter::ReportError(
        "spatial3Dtype x point3D x vector3D expected");
      return nl->TypeError();
    }

    if(nl->AtomType(arg1)!=SymbolType){
      ErrorReporter::ReportError(
        "spatial3Dtype x point3D x vector3D expected");
      return nl->TypeError();
    }
    st = nl->SymbolValue(arg1);
    if( st!=Point3d::BasicType() && st!=Surface3d::BasicType() &&
        st!=Volume3d::BasicType()){
      ErrorReporter::ReportError(
        "spatial3Dtype x point3D x vector3D expected");
      return nl->TypeError();
    }

    return nl->SymbolAtom(st);
  }
  
    
/*  
10.1.4 Type mapping function Spatial3DScaleDirMap

This type mapping function is used for the ~translate~ operator.
The mamp is spatial3Dtype x vector3D -> spatial3Dtype

*/
  ListExpr Spatial3DScaleMap( ListExpr args )
  {
    if ( nl->ListLength( args ) != 3 )
    { ErrorReporter::ReportError("wrong number of arguments (2 expected)");
      return nl->TypeError();
    }
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);

    if(nl->AtomType(arg2)!=SymbolType){
      ErrorReporter::ReportError("spatial3Dtype x point3D x real expected");
      return nl->TypeError();
    }
    string st = nl->SymbolValue(arg2);
    if( st!=Point3d::BasicType()){
      ErrorReporter::ReportError("spatial3Dtype x point3D x real expected");
      return nl->TypeError();
    }

    if(!nl->IsEqual(arg3,CcReal::BasicType())){
      ErrorReporter::ReportError("spatial3Dtype x point3D x real expected");
      return nl->TypeError();
    }    

    if(nl->AtomType(arg1)!=SymbolType){
      ErrorReporter::ReportError("spatial3Dtype x point3D x real expected");
      return nl->TypeError();
    }
    st = nl->SymbolValue(arg1);
    if( st!=Point3d::BasicType() && st!=Surface3d::BasicType() &&
        st!=Volume3d::BasicType()){
      ErrorReporter::ReportError("spatial3Dtype x point3D x real expected");
      return nl->TypeError();
    }

    return nl->SymbolAtom(st);
  }

/*
10.3 Selection functions

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

10.3.1 Selection function ~Spatial3DSelectTransformation~

This select function is used for the ~translate~, ~rotate~,
~mirror~ , ~scale~ and ~scaleDir~ operators.

*/  
  int Spatial3DSelect( ListExpr args )
  {
    ListExpr arg1 = nl->First( args );
    string st = nl->SymbolValue(arg1);
    
    if( st==Point3d::BasicType()){
      return 0;
    }
    if( st==Surface3d::BasicType()){
      return 1;
    }
    if( st==Volume3d::BasicType()){
      return 2;
    }

    return -1; // This point should never be reached
  }

/*
10.4 Value mapping functions
  
10.4.1 Value mapping functions of operator ~rotate~

*/
  template<class T>
  int Spatial3DRotate( Word* args, Word& result, int message,
                      Word& local, Supplier s ){
    result = qp->ResultStorage(s);
    T* res = static_cast<T*>(result.addr);
    T* st = static_cast<T*>(args[0].addr);
    Point3d* p = static_cast<Point3d*>(args[1].addr);
    Vector3d* v = static_cast<Vector3d*>(args[2].addr);
    CcReal* a = static_cast<CcReal*>(args[3].addr);
    if(!st->IsDefined() || !p->IsDefined() || !v->IsDefined()
      || !a->IsDefined()){
        res->SetDefined(false);
        return 0;
    }
    double angle = a->GetRealval() * M_PI / 180;
    st->rotate(p,v,angle,*res);
    return 0;
  }
  
/*  
10.4.2 Value mapping functions of operator ~mirror~

*/
  template<class T>
  int Spatial3DMirror( Word* args, Word& result, int message,
                      Word& local, Supplier s ){
    result = qp->ResultStorage(s);
    T* res = static_cast<T*>(result.addr);
    T* st = static_cast<T*>(args[0].addr);
    Plane3d* p = static_cast<Plane3d*>(args[1].addr);
    if(!st->IsDefined() || !p->IsDefined()){
        res->SetDefined(false);
        return 0;
    }
    st->mirror(p,*res);
    return 0;
  }
  
/*  
10.4.3 Value mapping functions of operator ~translate~

*/
  template<class T>
  int Spatial3DTranslate( Word* args, Word& result, int message,
                      Word& local, Supplier s ){
    result = qp->ResultStorage(s);
    T* res = static_cast<T*>(result.addr);
    T* st = static_cast<T*>(args[0].addr);
    Supplier son;
    Word arg;

    son = qp->GetSon( args[1].addr, 0 );
    qp->Request( son, arg );
    Vector3d* v = static_cast<Vector3d*>(arg.addr);
    if(!st->IsDefined() || !v->IsDefined()){
        res->SetDefined(false);
        return 0;
    }
    st->translate(v,*res);
    return 0;
  }

/*  
10.4.4 Value mapping functions of operator ~scaleDir~

*/
  template<class T>
  int Spatial3DScaleDir( Word* args, Word& result, int message,
                      Word& local, Supplier s ){
    result = qp->ResultStorage(s);
    T* res = static_cast<T*>(result.addr);
    T* st = static_cast<T*>(args[0].addr);
    Point3d* p = static_cast<Point3d*>(args[1].addr);
    Vector3d* v = static_cast<Vector3d*>(args[2].addr);
    if(!st->IsDefined() || !p->IsDefined() || !v->IsDefined()){
        res->SetDefined(false);
        return 0;
    }
    st->scaleDir(p,v,*res);
    return 0;
  }

/*  
10.4.5 Value mapping functions of operator ~scale~

*/
  template<class T>
  int Spatial3DScale( Word* args, Word& result, int message,
                      Word& local, Supplier s ){
    result = qp->ResultStorage(s);
    T* res = static_cast<T*>(result.addr);
    T* st = static_cast<T*>(args[0].addr);
    Point3d* p = static_cast<Point3d*>(args[1].addr);
    CcReal* a = static_cast<CcReal*>(args[2].addr);
    if(!st->IsDefined() || !p->IsDefined() || !a->IsDefined()){
        res->SetDefined(false);
        return 0;
    }
    double factor = a->GetRealval();
    st->scale(p,factor,*res);
    return 0;
  }
  
/*
10.5 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an
array of value mapping functions for each operator.

10.5.1 Definition of value mapping vectors

*/
  
  ValueMapping spatial3drotatemap[] = {
    Spatial3DRotate<Point3d>,
    Spatial3DRotate<Surface3d>,
    Spatial3DRotate<Volume3d>};

  ValueMapping spatial3dmirrormap[] = {
    Spatial3DMirror<Point3d>,
    Spatial3DMirror<Surface3d>,
    Spatial3DMirror<Volume3d>};


  ValueMapping spatial3dtranslatemap[] = {
    Spatial3DTranslate<Point3d>,
    Spatial3DTranslate<Surface3d>,
    Spatial3DTranslate<Volume3d>};


  ValueMapping spatial3dscaledirmap[] = {
    Spatial3DScaleDir<Point3d>,
    Spatial3DScaleDir<Surface3d>,
    Spatial3DScaleDir<Volume3d>};


  ValueMapping spatial3dscalemap[] = {
    Spatial3DScale<Point3d>,
    Spatial3DScale<Surface3d>,
    Spatial3DScale<Volume3d>};

/*
10.5.2 Definition of specification strings

*/
  const string Spatial3DSpecRotate  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>(point3D||surface3d||volume3d x point3D x vector3D x real) -> "
    "point3D||surface3d||volume3d</text--->"
    "<text> _ rotate[ p, v, theta ]</text--->"
    "<text> rotates the object by 'theta' degrees around the line,"
    "through the point p with vector v</text--->"
    "<text> query object rotate[point,vector,10.0]</text--->"
    ") )";

  const string Spatial3DSpecMirror  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>(point3D||surface3d||volume3d x plane) -> "
    "point3D||surface3d||volume3d</text--->"
    "<text> _ mirror[ p]</text--->"
    "<text> mirrors the object on the plane</text--->"
    "<text> query object mirror[plane]</text--->"
    ") )";

  const string Spatial3DSpecTranslate  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>(point3D||surface3d||volume3d x vector3D) -> "
    "point3D||surface3d||volume3d</text--->"
    "<text> _ translate[ v]</text--->"
    "<text> translate the object with the vector</text--->"
    "<text> query object translate[vector]</text--->"
    ") )";

  const string Spatial3DSpecScaleDir  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>(point3D||surface3d||volume3d x point3D x vector3D) -> "
    "point3D||surface3d||volume3d</text--->"
    "<text> _ scaleDir[ p, v]</text--->"
    "<text> scales the object from a point with a vector--->"
    "<text> query object scaleDir[point,vector]</text--->"
    ") )";

  const string Spatial3DSpecScale  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>(point3D||surface3d||volume3d  x point3D x real) -> "
    "point3D||surface3d||volume3d</text--->"
    "<text> _ scale[ p, a]</text--->"
    "<text> scales the object from a point by a factor--->"
    "<text> query object scale[point, factor]</text--->"
    ") )";


/*
10.5.3 Definition of the operators

*/
 
  Operator* getRotatePtr(){
    return new Operator(
    "rotate",
    Spatial3DSpecRotate,
    3,
    spatial3drotatemap,
    Spatial3DSelect,
    Spatial3DRotateMap
   );
  }
 
  Operator* getMirrorPtr(){
    return new Operator(
    "mirror",
    Spatial3DSpecMirror,
    3,
    spatial3dmirrormap,
    Spatial3DSelect,
    Spatial3DMirrorMap
   );
  }
 
  Operator* getTranslatePtr(){
    return new Operator(
    "translate",
    Spatial3DSpecTranslate,
    3,
    spatial3dtranslatemap,
    Spatial3DSelect,
    Spatial3DTranslateMap
   );
  }
 
  Operator* getScaleDirPtr(){
    return new Operator(
    "scaleDir",
    Spatial3DSpecScaleDir,
    3,
    spatial3dscaledirmap,
    Spatial3DSelect,
    Spatial3DScaleDirMap
   );
  }
 
  Operator* getScalePtr(){
    return new Operator(
    "scale",
    Spatial3DSpecScale,
    3,
    spatial3dscalemap,
    Spatial3DSelect,
    Spatial3DScaleMap
   );
  }
}
  
