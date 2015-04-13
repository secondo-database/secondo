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

#include <cmath>
#include "AuxiliaryTypes.h"
#include "geometric_algorithm.h"


Plane3d::Plane3d() : Attribute() { }

Plane3d::Plane3d(bool def) : Attribute(def) { }

Plane3d::Plane3d(const SimplePoint3d& point, const Vector3d& normalVector)
   : Attribute(true)
{
  set(point, normalVector);
}

Plane3d::Plane3d(double distanceToOrigin, const Vector3d& normalVector)
   : Attribute(true)
{
  set(distanceToOrigin, normalVector);
}

Plane3d::Plane3d(const SimplePoint3d& p1,
                 const SimplePoint3d& p2,
                 const SimplePoint3d& p3)
   : Attribute(true)
{
  set(p1, p2, p3);
}

Plane3d::Plane3d(const Triangle& triangle)
   : Attribute(true)
{
  set(triangle);
}

void Plane3d::set(const SimplePoint3d& point, const Vector3d& normalVector)
{
  this->normalVector = normalVector;
  distanceToOrigin = spatial3d_geometric::planeDistanceToOrigin(point,
                                                                normalVector);
}

void Plane3d::set(const double distanceToOrigin, const Vector3d& normalVector)
{
  this->distanceToOrigin = distanceToOrigin;
  this->normalVector = normalVector;
}

void Plane3d::set(const SimplePoint3d& p1,
                  const SimplePoint3d& p2,
                  const SimplePoint3d& p3)
{
  spatial3d_geometric::planeHessianNormalForm(p1, p2, p3,
                                              distanceToOrigin, normalVector);
}

void Plane3d::set(const Triangle& triangle)
{
  set(triangle.getA(), triangle.getB(), triangle.getC());
}

SimplePoint3d Plane3d::getPoint() const
{
   double nx = normalVector.getX();
   double ny = normalVector.getY();
   double nz = normalVector.getZ();
   double nlength = sqrt(nx*nx + ny*ny + nz*nz);
   return SimplePoint3d(distanceToOrigin * (nx / nlength),
                        distanceToOrigin * (ny / nlength),
                        distanceToOrigin * (nz / nlength));
}

Vector3d Plane3d::getNormalVector() const
{
  return normalVector;
}

double Plane3d::getDistanceToOrigin() const
{
  return distanceToOrigin;
}


const string Plane3d::BasicType()
{
  return "plane3d";
}

const bool Plane3d::checkType(const ListExpr list)
{
  return listutils::isSymbol(list, BasicType());
}

ListExpr Plane3d::Property()
{
  return gentc::GenProperty(
    "-> DATA",
    BasicType(),
    "(d x y z)",
    "(0.0 0.0 0.0 1.0)");
}

bool Plane3d::CheckKind(ListExpr type, ListExpr& errorInfo)
{
  return nl->IsEqual(type, BasicType());
}

int Plane3d::NumOfFLOBs() const
{
  return 0;
}

Flob* Plane3d::GetFLOB(const int i)
{
  return NULL;
}

size_t Plane3d::Sizeof() const
{
  return sizeof(*this);
}

int Plane3d::Compare(const Attribute *arg) const
{
  Plane3d* other = (Plane3d*) arg;
  if (!this->IsDefined())
  {
    return other->IsDefined() ? -1 : 0;
  }
  if (!other->IsDefined())
  {
    return 1;
  }
  if (this->distanceToOrigin != other->distanceToOrigin)
  {
    return this->distanceToOrigin < other->distanceToOrigin ? -1 : 1;
  }
  return 0;
}

Attribute* Plane3d::Clone() const
{
  return new Plane3d(*this);
}

bool Plane3d::Adjacent(const Attribute *arg) const
{
  return false;
}

size_t Plane3d::HashValue() const
{
  return (size_t)distanceToOrigin;
}

void Plane3d::CopyFrom(const Attribute *arg)
{
  *this = *((Plane3d*)arg);
}

Plane3d& Plane3d::operator=(const Plane3d& src)
{
  if (src.IsDefined())
  {
    SetDefined(true);
    this->distanceToOrigin = src.distanceToOrigin;
    this->normalVector = src.normalVector;
  }
  else
  {
    SetDefined(false);
  }
  return *this;
}

ListExpr Plane3d::ToListExpr(ListExpr typeInfo)
{
  if (IsDefined())
  {
   double d = distanceToOrigin;
   double x = normalVector.getX();
   double y = normalVector.getY();
   double z = normalVector.getZ();
   
  return nl->FourElemList(nl->RealAtom(d),
                            nl->RealAtom(x),
                            nl->RealAtom(y),
                            nl->RealAtom(z));
  }
  else
  {
    return listutils::getUndefined();
  }
}

bool Plane3d::ReadFrom(ListExpr value, ListExpr typeInfo)
{
  if (listutils::isSymbolUndefined(value))
  {
    SetDefined(false);
    return true;
  }
  if (!nl->HasLength(value, 4))
  {
    return false;
  }
  
  if (!listutils::isNumeric(nl->First(value)) ||
      !listutils::isNumeric(nl->Second(value)) ||
      !listutils::isNumeric(nl->Third(value)) ||
      !listutils::isNumeric(nl->Fourth(value)))
  {
    return false;
  }
  double d = listutils::getNumValue(nl->First(value));
  double x = listutils::getNumValue(nl->Second(value));
  double y = listutils::getNumValue(nl->Third(value));
  double z = listutils::getNumValue(nl->Fourth(value));

  Vector3d normal(x,y,z);
  set( d, normal);
  SetDefined(true);
  return true;
}

bool Plane3d::operator==(Plane3d& other) const
{
  if (!this->IsDefined() && !other.IsDefined())
    return true;
  if (!this->IsDefined() || !other.IsDefined())
    return false;
  
  return this->distanceToOrigin == other.distanceToOrigin && 
    this->normalVector == other.normalVector;
}

