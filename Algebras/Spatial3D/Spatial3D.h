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
 
 01590 Fachpraktikum "Erweiterbare Datenbanksysteme" 
 WS 2014 / 2015

<our names here>

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
 
[1] Implementation of a Spatial3D algebra

[TOC]

1 Includes and Defines

*/

//the above header was modified and copied from SuffixTree.h


#ifndef _SPATIAL3D_H
#define _SPATIAL3D_H

#include "AuxiliaryTypes.h"
#include "../../Tools/Flob/DbArray.h"
#include "RectangleAlgebra.h"
#include "MultiObjectTriangleContainer.h"
#include "Spatial3DTransformations.h"
#include "Spatial3DCreate.h"
#include "MMRTree.h"
#include "Spatial3DOperatorSize.h"
#include "Spatial3DOperatorBBox.h"
#include "Spatial3DConvert.h"
#include "OperatorTest.h"

class Point3d;
class MultiObjectTriangleContainer;

class TransformObject : public StandardSpatialAttribute<3>
{
public:
  TransformObject();
  TransformObject(bool def);
  TransformObject(const TransformObject& src);

  ~TransformObject();

  // StandardSpatialAttribute
  virtual bool IsEmpty() const;
};

class Point3d : public TransformObject
{
public:
  Point3d();
  Point3d(bool def);
  Point3d(const Point3d& src);
  Point3d(const SimplePoint3d& src);
  Point3d(double x, double y, double z);
  
  ~Point3d();

  operator SimplePoint3d() const;
  
  static const string BasicType();
  static const bool checkType(const ListExpr list);
  static ListExpr Property();
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  virtual int NumOfFLOBs() const;
  virtual Flob* GetFLOB(const int i);
  size_t Sizeof() const;
  int Compare(const Attribute *arg) const;
  Attribute* Clone() const;
  bool Adjacent(const Attribute *arg) const;
  size_t HashValue() const;
  void CopyFrom(const Attribute *arg);
  Point3d& operator=(const Point3d& src);
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr value, ListExpr typeInfo);
  bool operator==(Point3d& other) const;  
  
  virtual const Rectangle<3> BoundingBox(const Geoid* = 0) const;
  virtual double Distance(const Rectangle<3>&, const Geoid*) const;
  virtual bool Intersects(const Rectangle<3>&, const Geoid*) const;

  void set(double x, double y, double z);
  void set(Point3d* point);
  void write(Point3d* point);
  
  double getX() const;
  double getY() const;
  double getZ() const;

  virtual void rotate(const Point3d* center,
                      const Vector3d* axis, double angle, Point3d& res);
  virtual void mirror(const Plane3d* plane, Point3d& res);
  virtual void translate(const Vector3d* translation, Point3d& res);
  virtual void scaleDir(const Point3d* center,
                        const Vector3d* direction, Point3d& res);
  virtual void scale(const Point3d* center,double factor, Point3d& res);

private:

  double x, y, z;
  
};

enum BulkLoadOptions { NO_REPAIR = 0, REPAIR = 1 };

class TriangleContainer : public TransformObject
{
public:

  /* Constructors */
  
  TriangleContainer();
  TriangleContainer(bool def);
  TriangleContainer(const TriangleContainer& src);
  TriangleContainer(int n);

  /* Destructor */
  ~TriangleContainer();

  /* Creation */
  void clear();
  void startBulkLoad();
  void add(const Triangle& triangle);
  bool endBulkLoad(BulkLoadOptions options);

  /* Reading */
  int size() const;
  Triangle get(int index) const;
  Rectangle<3> getBoundingBox() const;

  /* Attribute */
  virtual int NumOfFLOBs() const;
  virtual Flob* GetFLOB(int i);
  bool Adjacent(const Attribute *arg) const;
  size_t HashValue() const;
  virtual ListExpr ToListExpr(ListExpr typeInfo);
  virtual bool ReadFrom(ListExpr value, ListExpr typeInfo);

  /* StandardSpatialAttribute */
  virtual const Rectangle<3> BoundingBox(const Geoid* = 0) const;
  virtual double Distance(const Rectangle<3>&, const Geoid*) const;
  virtual bool Intersects(const Rectangle<3>&, const Geoid*) const;

  /* Manipulation: TransformObject */
  virtual void rotate(const Point3d* center,const Vector3d* axis, 
                      double angle, TriangleContainer& res);
  virtual void mirror(const Plane3d* plane, TriangleContainer& res);
  virtual void translate(const Vector3d* translation, TriangleContainer& res);
  virtual void scaleDir(const Point3d* center,
                        const Vector3d* direction, TriangleContainer& res);
  virtual void scale(const Point3d* center,double factor, 
                     TriangleContainer& res);

  /* Manipulation: other */
  void matrixMultiplication(int[][4]);
  void changeTriangleOrientation();

  void set(const TriangleContainer* container);

protected:
  
  int CompareTriangleContainer(const TriangleContainer *arg) const;
  void copyTrianglesFrom(const TriangleContainer& src);
  
  struct TrianglePointIndices
  {
    int pointIndexA;
    int pointIndexB;
    int pointIndexC;
  };
  
  DbArray<SimplePoint3d> pointFlob;
  DbArray<TrianglePointIndices> triangleFlob;
  Rectangle<3> boundingBox;

  enum BulkloadStatus { ACTIVE, ERROR };
  
  class BulkloadData
  {

  public:

    BulkloadData();
  
    ~BulkloadData();
  
    BulkloadStatus getStatus();
    bool hasCorrections();
  
    mmrtree::RtreeT<3, size_t>& getPoints();
    MultiObjectTriangleContainer& getContainer();
    
    void error();
    void correction();
  
  private:
  
    BulkloadStatus status;
    mmrtree::RtreeT<3, size_t>* points;
    MultiObjectTriangleContainer container;
    bool hadToCorrect;
  };
  
  BulkloadData* bulkloadData;

private:

  int addPoint(const Point3d& point3d);

  NList getPointList(int pointIndex);
  NList getTriangleList(int triangleIndex);
  
  bool checkPoint(NList value);
  bool checkTriangle(NList value);
  bool checkTriangleList(NList value);

  Point3d readPoint(NList value);
  void readTriangle(NList value);
  void readTriangleList(NList value);
  
  virtual bool checkBulkloadData(BulkLoadOptions options) const;
};

class Surface3d : public TriangleContainer
{
public:
  
  Surface3d();
  Surface3d(bool def);
  Surface3d(const Surface3d& src);
  Surface3d(int n);
  ~Surface3d();

  static const string BasicType();
  static const bool checkType(const ListExpr list);
  static ListExpr Property();
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  size_t Sizeof() const;
  int Compare(const Attribute *arg) const;
  Attribute* Clone() const;
  void CopyFrom(const Attribute *arg);
  Surface3d& operator=(const Surface3d& src);
  bool operator==(Surface3d& other) const;  
};

class Volume3d : public TriangleContainer
{
public:

  Volume3d();
  Volume3d(bool def);
  Volume3d(const Volume3d& src);
  Volume3d(int n);
  ~Volume3d();

  static const string BasicType();
  static const bool checkType(const ListExpr list);
  static ListExpr Property();
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  size_t Sizeof() const;
  int Compare(const Attribute *arg) const;
  Attribute* Clone() const;
  void CopyFrom(const Attribute *arg);
  Volume3d& operator=(const Volume3d& src);
  bool operator==(Volume3d& other) const;
private:
  virtual bool checkBulkloadData(BulkLoadOptions options) const;
};


#endif