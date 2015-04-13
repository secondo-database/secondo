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


extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

Surface3d::Surface3d() : TriangleContainer() { }

Surface3d::Surface3d(bool def) : TriangleContainer(def) { }

Surface3d::Surface3d(const Surface3d& src) : TriangleContainer(src) { }

Surface3d::Surface3d(int n) : TriangleContainer(n) { }

Surface3d::~Surface3d() { }

const string Surface3d::BasicType()
{
  return "surface3d";
}

const bool Surface3d::checkType(const ListExpr list)
{
  return listutils::isSymbol(list, BasicType());
}

ListExpr Surface3d::Property()
{
  return gentc::GenProperty(
    "-> DATA",
    BasicType(),
    "(((a1x a1y a1z) (b1x b1y b1z) (c1x c1y c1z)) "
    "((a2x a2y a2z) (b2x b2y b2z) (c2x c2y c2z)))",
    "(((0 0 0) (0 1 0) (1 1 0)) ((0 0 0) (1 0 0) (1 1 0)))");
}

bool Surface3d::CheckKind(ListExpr type, ListExpr& errorInfo)
{
  return nl->IsEqual(type, BasicType());
}

size_t Surface3d::Sizeof() const
{
  return sizeof(*this);
}

int Surface3d::Compare(const Attribute *arg) const
{
  Surface3d* other = (Surface3d*) arg;
  return TriangleContainer::CompareTriangleContainer(other);
}

Attribute* Surface3d::Clone() const
{
  return new Surface3d(*this);
}

void Surface3d::CopyFrom(const Attribute *arg)
{
  *this = *((Surface3d*)arg);
}

Surface3d& Surface3d::operator=(const Surface3d& src)
{
  SetDefined(src.IsDefined());
  if (src.IsDefined())
  {
    copyTrianglesFrom(src);
  }
  return *this;
}

bool Surface3d::operator==(Surface3d& other) const
{
  return Compare(&other) == 0;
}
