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
#include "Spatial3DTransformations.h"
#include "RelationAlgebra.h"


extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

Point3d::Point3d() : TransformObject() { }

Point3d::Point3d(bool def) : TransformObject(def), x(0), y(0), z(0) { }

Point3d::Point3d(const Point3d& src)
    : TransformObject(src.IsDefined()), x(src.x), y(src.y), z(src.z) { }

Point3d::Point3d(const SimplePoint3d& src)
    : TransformObject(true), x(src.getX()), y(src.getY()), z(src.getZ()) { }
    
Point3d::Point3d(double x, double y, double z) : TransformObject(true)
{
  set(x, y, z);
}

Point3d::~Point3d() { }

Point3d::operator SimplePoint3d() const
{
  assert(IsDefined());
  return SimplePoint3d(x, y, z);
}

const string Point3d::BasicType()
{
  return "point3d";
}

const bool Point3d::checkType(const ListExpr list)
{
  return listutils::isSymbol(list, BasicType());
}

ListExpr Point3d::Property()
{
  return gentc::GenProperty(
    "-> DATA",
    BasicType(),
    "(x y z)",
    "(4.0 -3.7 2.5)");
}

bool Point3d::CheckKind(ListExpr type, ListExpr& errorInfo)
{
  return nl->IsEqual(type, BasicType());
}

int Point3d::NumOfFLOBs() const
{
  return 0;
}

Flob* Point3d::GetFLOB(const int i)
{
  return NULL;
}

size_t Point3d::Sizeof() const
{
  return sizeof(*this);
}

int Point3d::Compare(const Attribute *arg) const
{
  Point3d* other = (Point3d*) arg;
  if (!this->IsDefined())
  {
    return other->IsDefined() ? -1 : 0;
  }
  if (!other->IsDefined())
  {
    return 1;
  }
  if (this->x != other->x)
  {
    return this->x < other->x ? -1 : 1;
  }
  if (this->y != other->y)
  {
    return this->y < other->y ? -1 : 1;
  }
  if (this->z != other->z)
  {
    return this->z < other->z ? -1 : 1;
  }
  return 0;
}

Attribute* Point3d::Clone() const
{
  return new Point3d(*this);
}

bool Point3d::Adjacent(const Attribute *arg) const
{
  return false;
}

size_t Point3d::HashValue() const
{
  return 0; // TODO: vernünftiger Wert (Jens Breit)
}

void Point3d::CopyFrom(const Attribute *arg)
{
  *this = *((Point3d*)arg);
}

Point3d& Point3d::operator=(const Point3d& src)
{
  if (src.IsDefined())
  {
    SetDefined(true);
    this->x = src.x;
    this->y = src.y;
    this->z = src.z;
  }
  else
  {
    SetDefined(false);
  }
  return *this;
}

ListExpr Point3d::ToListExpr(ListExpr typeInfo)
{
  if (IsDefined())
  {
    return nl->ThreeElemList(nl->RealAtom(x),
                             nl->RealAtom(y),
                             nl->RealAtom(z));
  }
  else
  {
    return listutils::getUndefined();
  }
}

bool Point3d::ReadFrom(ListExpr value, ListExpr typeInfo)
{
  if (listutils::isSymbolUndefined(value))
  {
    SetDefined(false);
    return true;
  }
  if (!nl->HasLength(value, 3))
  {
    return false;
  }
  
  if (!listutils::isNumeric(nl->First(value)) ||
      !listutils::isNumeric(nl->Second(value)) ||
      !listutils::isNumeric(nl->Third(value)))
  {
    return false;
  }
  x = listutils::getNumValue(nl->First(value));
  y = listutils::getNumValue(nl->Second(value));
  z = listutils::getNumValue(nl->Third(value));
  
  SetDefined(true);
  return true;
}

bool Point3d::operator==(Point3d& other) const
{
  if (!this->IsDefined() && !other.IsDefined())
    return true;
  if (!this->IsDefined() || !other.IsDefined())
    return false;
  
  return this->x == other.x && this->y == other.y && this->z == other.z;
}

const Rectangle<3> Point3d::BoundingBox(const Geoid* geoid) const 
{
  return Rectangle<3>(true, x, x, y, y, z, z);
}

double Point3d::Distance(const Rectangle<3>& rect, const Geoid* geoid = 0) const
{
  return rect.Distance(BoundingBox());
}

bool Point3d::Intersects(const Rectangle<3>& rect, const Geoid* geoid = 0) const
{
  return rect.Intersects(BoundingBox());
}

void Point3d::set(double x, double y, double z)
{
  this->x = x;
  this->y = y;
  this->z = z;
}

void Point3d::set(Point3d* point)
{
  this->x = point->x;
  this->y = point->y;
  this->z = point->z;
}

void Point3d::write(Point3d* point)
{
  cout << "point(";
  cout << point->x;
  cout << " , ";
  cout << point->y;
  cout << " , ";
  cout << point->z;
  cout << " )";
  cout << endl;
}

double Point3d::getX() const
{
  return x;
}

double Point3d::getY() const
{
  return y;
}

double Point3d::getZ() const
{
  return z;
};

void Point3d::rotate(const Point3d* center, const Vector3d* axis, 
  double angle, Point3d& res)
{
  spatial3DTransformations::rotatePoint(this,center,axis,angle,res);
};

void Point3d::mirror(const Plane3d* plane, Point3d& res)
{
  spatial3DTransformations::mirrorPoint(this,plane,res);
};

void Point3d::translate(const Vector3d* translation, Point3d& res)
{
  spatial3DTransformations::translatePoint(this,translation,res);
};

void Point3d::scaleDir(const Point3d* center,
                       const Vector3d* direction, Point3d& res)
{
  spatial3DTransformations::scaleDirPoint(this,center,direction, res);
};

void Point3d::scale(const Point3d* center,double factor, Point3d& res)
{
  spatial3DTransformations::scalePoint(this, center, factor, res);
};
