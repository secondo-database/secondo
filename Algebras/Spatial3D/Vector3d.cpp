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

#include "AuxiliaryTypes.h"
#include "geometric_algorithm.h"

Vector3d::Vector3d() : Attribute() { }

Vector3d::Vector3d(bool def) : Attribute(def) { }

Vector3d::Vector3d(double x, double y, double z)
     : Attribute(true)
{
  set(x, y, z);
}

Vector3d::Vector3d(const SimplePoint3d& from, const SimplePoint3d& to)
     : Attribute(true)
{
  set(from, to);
}

Vector3d::~Vector3d() { }

void Vector3d::set(double x, double y, double z)
{
  this->x = x;
  this->y = y;
  this->z = z;
}

void Vector3d::set(const SimplePoint3d& from, const SimplePoint3d& to)
{
  this->x = to.getX() - from.getX();
  this->y = to.getY() - from.getY();
  this->z = to.getZ() - from.getZ();
}

double Vector3d::getX() const
{
  return x;
}

double Vector3d::getY() const
{
  return y;
}

double Vector3d::getZ() const
{
  return z;
}

double Vector3d::getLength() const
{
  return spatial3d_geometric::length(*this);
}


const string Vector3d::BasicType()
{
  return "vector3d";
}

const bool Vector3d::checkType(const ListExpr list)
{
  return listutils::isSymbol(list, BasicType());
}

ListExpr Vector3d::Property()
{
  return gentc::GenProperty(
    "-> DATA",
    BasicType(),
    "(x y z)",
    "(4.0 -3.7 2.5)");
}

bool Vector3d::CheckKind(ListExpr type, ListExpr& errorInfo)
{
  return nl->IsEqual(type, BasicType());
}

int Vector3d::NumOfFLOBs() const
{
  return 0;
}

Flob* Vector3d::GetFLOB(const int i)
{
  return NULL;
}

size_t Vector3d::Sizeof() const
{
  return sizeof(*this);
}

int Vector3d::Compare(const Attribute *arg) const
{
  Vector3d* other = (Vector3d*) arg;
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

Attribute* Vector3d::Clone() const
{
  return new Vector3d(*this);
}

bool Vector3d::Adjacent(const Attribute *arg) const
{
  return false;
}

size_t Vector3d::HashValue() const
{
  return 0; // TODO: vernünftiger Wert (Jens Breit)
}

void Vector3d::CopyFrom(const Attribute *arg)
{
  *this = *((Vector3d*)arg);
}

Vector3d& Vector3d::operator=(const Vector3d& src)
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

ListExpr Vector3d::ToListExpr(ListExpr typeInfo)
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

bool Vector3d::ReadFrom(ListExpr value, ListExpr typeInfo)
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

bool Vector3d::operator==(Vector3d& other) const
{
  if (!this->IsDefined() && !other.IsDefined())
    return true;
  if (!this->IsDefined() || !other.IsDefined())
    return false;
  
  return this->x == other.x && this->y == other.y && this->z == other.z;
}

ostream& operator<< (ostream& os, const Vector3d& vector) {
  return os << "(" << vector.getX()
  << "/" << vector.getY()
  << "/" << vector.getZ() << ")";
}