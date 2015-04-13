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

#include "Spatial3D.h"
#include "RelationAlgebra.h"
#include "MMRTree.h"
#include "geometric_algorithm.h"
#include "Spatial3DTransformations.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

using namespace std;

TriangleContainer::TriangleContainer()
    : TransformObject() { }

TriangleContainer::TriangleContainer(bool def)
    : TransformObject(def),
      pointFlob(0), triangleFlob(0), bulkloadData(0) { }

TriangleContainer::TriangleContainer(const TriangleContainer& src)
    : TransformObject(src.IsDefined()),
      pointFlob(src.pointFlob.Size()),
      triangleFlob(src.pointFlob.Size()),
      bulkloadData(0)
{
  pointFlob.copyFrom(src.pointFlob);
  triangleFlob.copyFrom(src.triangleFlob);
  boundingBox = src.boundingBox;
}

TriangleContainer::TriangleContainer(int n)
    : TransformObject(true), pointFlob(n), triangleFlob(n), bulkloadData(0) { }

TriangleContainer::~TriangleContainer()
{
  if (bulkloadData != NULL) {
    delete bulkloadData;
  }
}

void TriangleContainer::clear()
{
  pointFlob.clean();
  triangleFlob.clean();
  boundingBox.SetDefined(false);
}

void TriangleContainer::startBulkLoad()
{
  // startBulkLoad() must not be called again while bulkload is active
  assert(bulkloadData == NULL);
  bulkloadData = new BulkloadData();
}

bool TriangleContainer::endBulkLoad(BulkLoadOptions options)
{
  assert(bulkloadData != 0);

  if (bulkloadData->getStatus() == ERROR
      || (bulkloadData->hasCorrections() && ((options & REPAIR) == 0))
      || !checkBulkloadData(options))
  {
    triangleFlob.clean();
    pointFlob.clean();
    boundingBox.SetDefined(false);
    SetDefined(false);
  }
  else
  {
    SetDefined(true);
    
    vector<Triangle> triangles;
    bulkloadData->getContainer().exportObject(1, triangles);
    
    // Clear the old data structures before we construct our own tree
    bulkloadData->getContainer().removeObject(1);
    
    mmrtree::RtreeT<3, size_t> points(4,8);
    
    for (vector<Triangle>::iterator it = triangles.begin();
         it != triangles.end(); ++it)
    {
      TrianglePointIndices pointIndices;
      
      pointIndices.pointIndexA = addPoint(it->getA());
      pointIndices.pointIndexB = addPoint(it->getB());
      pointIndices.pointIndexC = addPoint(it->getC());
      
      triangleFlob.Append(pointIndices);
    }
  }

  delete bulkloadData;
  bulkloadData = 0;
  return true;
}

bool TriangleContainer::checkBulkloadData(BulkLoadOptions options) const
{
  return true;
}

int TriangleContainer::addPoint(const Point3d& point)
{
  /* First, compare coordinates with existing points. If there is a
   * matching point, just return its index. Otherwise, we create a new
   * point.
   */

  SimplePoint3d simplePoint(point);
  mmrtree::RtreeT<3, size_t>::iterator* it;
  Rectangle<3> bbox = point.BoundingBox();
  bbox.Extend(0.0001);
  it = bulkloadData->getPoints().find(bbox);
  size_t const * index;
  while( (index = it->next()) != 0)
  {
    SimplePoint3d p;
    pointFlob.Get(*index, p);
    if (AlmostEqual(simplePoint.getX(), p.getX()) &&
        AlmostEqual(simplePoint.getY(), p.getY()) &&
        AlmostEqual(simplePoint.getZ(), p.getZ()))
    {
      delete it;
      return *index;
    }
  }
  delete it;
  int res = pointFlob.Size();
  pointFlob.Append(simplePoint);
  bulkloadData->getPoints().insert(bbox, res);
  return res;
}

void TriangleContainer::add(const Triangle& triangle)
{
    /* add may only be called while bulkload is active */
  assert(bulkloadData != NULL); 

  if(bulkloadData->getStatus() == ERROR)
  {
    return;
  }
  
  if (! spatial3d_geometric::isValidTriangle(triangle))
  {
    bulkloadData->correction();
    return;
  }

  MultiObjectTriangleContainer& container = bulkloadData->getContainer();
  
  bool res = container.addTriangle(triangle, 1, bulkloadData->hasCorrections());
  if (!res && !bulkloadData->hasCorrections())
  {
    bulkloadData->correction();
    res = container.addTriangle(triangle, 1, true);
  }
  if (!res)
  {
    bulkloadData->error();
  }
  else
  {  
    Rectangle<3> boundingBoxA = ((Point3d)(triangle.getA())).BoundingBox();
    if (boundingBox.IsDefined())
    {
      boundingBox.Extend(boundingBoxA);
    }
    else
    {
      boundingBox = boundingBoxA;
    }
    boundingBox.Extend(((Point3d)(triangle.getB())).BoundingBox());
    boundingBox.Extend(((Point3d)(triangle.getC())).BoundingBox());
  }
}

int TriangleContainer::size() const
{
  return triangleFlob.Size();
}

Triangle TriangleContainer::get(int index) const
{
  assert(index >= 0 && index < size());
  TrianglePointIndices indices;
  triangleFlob.Get(index, indices);
  SimplePoint3d spA, spB, spC;
  pointFlob.Get(indices.pointIndexA, spA);
  pointFlob.Get(indices.pointIndexB, spB);
  pointFlob.Get(indices.pointIndexC, spC);
  Point3d pA(spA.getX(), spA.getY(), spA.getZ());
  Point3d pB(spB.getX(), spB.getY(), spB.getZ());
  Point3d pC(spC.getX(), spC.getY(), spC.getZ());
  return Triangle(pA, pB, pC);
}

Rectangle<3> TriangleContainer::getBoundingBox() const
{
  //We also want to have empty volumes...
  //assert(size() > 0);
  return boundingBox;
}

int TriangleContainer::NumOfFLOBs() const
{
  return 2;
}

Flob* TriangleContainer::GetFLOB(int i)
{
  switch (i)
  {
    case 0:
      return &pointFlob;
    case 1:
      return &triangleFlob;
    default:
      assert(false);
      return NULL;
  }
}

bool TriangleContainer::Adjacent(const Attribute *arg) const
{
  return false;
}

size_t TriangleContainer::HashValue() const
{
  return 0; // TODO: sinnvoller Wert (Jens Breit)
}

NList TriangleContainer::getPointList(int pointIndex)
{
  SimplePoint3d p;
  pointFlob.Get(pointIndex, p);
  return NList(p.getX(), p.getY(), p.getZ());
}

NList TriangleContainer::getTriangleList(int triangleIndex)
{
  TrianglePointIndices pointIndices;
  triangleFlob.Get(triangleIndex, pointIndices);
  return NList(getPointList(pointIndices.pointIndexA),
               getPointList(pointIndices.pointIndexB),
               getPointList(pointIndices.pointIndexC));
}

ListExpr TriangleContainer::ToListExpr(ListExpr typeInfo)
{
  if (IsDefined())
  {
    NList result(nl);
    for (int ti = 0; ti < triangleFlob.Size(); ++ti)
    {
      result.append(getTriangleList(ti));
    }
    return result.listExpr();
  }
  else
  {
    return listutils::getUndefined();
  }
}

bool TriangleContainer::checkPoint(NList value)
{
  if (value.length() != 3)
  {
    return false;
  }
  return listutils::isNumeric(value.first ().listExpr())
      && listutils::isNumeric(value.second().listExpr())
      && listutils::isNumeric(value.third ().listExpr());
}

bool TriangleContainer::checkTriangle(NList value)
{
  if (value.length() != 3)
  {
    return false;
  }
  return checkPoint(value.first())
      && checkPoint(value.second())
      && checkPoint(value.third());
}

bool TriangleContainer::checkTriangleList(NList value)
{
  int length = value.length();
 /* SJ: We want an empty list, too
  if (length < 1)
  {
    return false;
  }
  */
  while (!value.isEmpty())
  {
    if (!checkTriangle(value.first()))
    {
      return false;
    }
    value.rest();
  }
  return true;
}

Point3d TriangleContainer::readPoint(NList value)
{
  double x, y, z;
  x = listutils::getNumValue(value.first ().listExpr());
  y = listutils::getNumValue(value.second().listExpr());
  z = listutils::getNumValue(value.third ().listExpr());
  
  Point3d newPoint(x, y, z);
  
  return newPoint;
}

void TriangleContainer::readTriangle(NList value)
{
  add(Triangle(readPoint(value.first()),
               readPoint(value.second()),
               readPoint(value.third())));
  
  // TODO: check (Jens Breit)
}

void TriangleContainer::readTriangleList(NList value)
{
  while (!value.isEmpty())
  {
    readTriangle(value.first());
    value.rest();
  }
}

bool TriangleContainer::ReadFrom(ListExpr value, ListExpr typeInfo)
{
  if (listutils::isSymbolUndefined(value))
  {
    SetDefined(false);
    return true;
  }
  
  if (checkTriangleList(NList(value)))
  {
    SetDefined(true);
    //SJ: init bbox here (just in case we have an empty list
    boundingBox= Rectangle<3>(false);
    pointFlob.resize(nl->ListLength(value) * 3);
    triangleFlob.resize(nl->ListLength(value));
    startBulkLoad();
    readTriangleList(NList(value));
    endBulkLoad(NO_REPAIR);
    return true;
  }
  else
  {
    return false;
  }
}

int TriangleContainer::CompareTriangleContainer(
                              const TriangleContainer *other) const
{
  if (!this->IsDefined())
  {
    return other->IsDefined() ? -1 : 0;
  }
  if (!other->IsDefined())
  {
    return 1;
  }
  if (this->triangleFlob.Size() < other->triangleFlob.Size())
  {
    return -1;
  }
  if (this->triangleFlob.Size() > other->triangleFlob.Size())
  {
    return 1;
  }
  if (this->pointFlob.Size() < other->pointFlob.Size())
  {
    return -1;
  }
  if (this->pointFlob.Size() > other->pointFlob.Size())
  {
    return 1;
  }
  
  // TODO (Jens Breit)
  
  return 0;
}

void TriangleContainer::copyTrianglesFrom(const TriangleContainer& src)
{
  pointFlob.copyFrom(src.pointFlob);
  triangleFlob.copyFrom(src.triangleFlob);
  boundingBox = src.boundingBox;
}

const Rectangle<3> TriangleContainer::BoundingBox(const Geoid* geoid) const
{
  return boundingBox;
}

double TriangleContainer::Distance(const Rectangle<3>& rect,
                                   const Geoid* geoid = 0) const
{
  return rect.Distance(BoundingBox()); // TODO: sinnvollerer Wert (Jens Breit)
}

bool TriangleContainer::Intersects(const Rectangle<3>& rect,
                                   const Geoid* geoid = 0) const
{
  return rect.Intersects(BoundingBox()); // TODO: sinnvollerer Wert (Jens Breit)
}

void TriangleContainer::rotate(const Point3d* center,const Vector3d* axis, 
                               double angle, TriangleContainer& res)
{
  spatial3DTransformations::rotateTriangleContainer(
    this,center,axis,angle,res);  
}

void TriangleContainer::mirror(const Plane3d* plane, TriangleContainer& res)
{
  spatial3DTransformations::mirrorTriangleContainer(this,plane,res);
}

void TriangleContainer::translate(const Vector3d* translation, 
                                  TriangleContainer& res)
{
  spatial3DTransformations::translateTriangleContainer(this,translation,res);
}

void TriangleContainer::scaleDir(const Point3d* center,
                        const Vector3d* direction, TriangleContainer& res)
{
  spatial3DTransformations::scaleDirTriangleContainer(
    this,center,direction,res);
}

void TriangleContainer::scale(const Point3d* center,
                              double factor, TriangleContainer& res)
{
  spatial3DTransformations::scaleTriangleContainer(this,center,factor,res);
}
