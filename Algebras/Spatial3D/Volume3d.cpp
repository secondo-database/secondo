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

Volume3d::Volume3d() : TriangleContainer() { }

Volume3d::Volume3d(bool def) : TriangleContainer(def) { }

Volume3d::Volume3d(const Volume3d& src) : TriangleContainer(src) { }

Volume3d::Volume3d(int n) : TriangleContainer(n) { }

Volume3d::~Volume3d() { }

const string Volume3d::BasicType()
{
  return "volume3d";
}

const bool Volume3d::checkType(const ListExpr list)
{
  return listutils::isSymbol(list, BasicType());
}

ListExpr Volume3d::Property()
{
  return gentc::GenProperty(
    "-> DATA",
    BasicType(),
    "(((a1x a1y a1z) (b1x b1y b1z) (c1x c1y c1z)) "
    "((a2x a2y a2z) (b2x b2y b2z) (c2x c2y c2z)) "
    "((a3x a3y a3z) (b3x b3y b3z) (c3x c3y c3z)))",
    "(((0 0 0) (1 0 0) (0 1 0)) ((0 0 0) (1 0 0) (0 0 1)) "
    "((1 0 0) (0 1 0) (0 0 1)) ((0 1 0) (0 0 0) (0 0 1)))");
}

bool Volume3d::CheckKind(ListExpr type, ListExpr& errorInfo)
{
  return nl->IsEqual(type, BasicType());
}

size_t Volume3d::Sizeof() const
{
  return sizeof(*this);
}

int Volume3d::Compare(const Attribute *arg) const
{
  Volume3d* other = (Volume3d*) arg;
  return TriangleContainer::CompareTriangleContainer(other);
}

Attribute* Volume3d::Clone() const
{
  return new Volume3d(*this);
}

void Volume3d::CopyFrom(const Attribute *arg)
{
  *this = *((Volume3d*)arg);
}

Volume3d& Volume3d::operator=(const Volume3d& src)
{
  SetDefined(src.IsDefined());
  if (src.IsDefined())
  {
    copyTrianglesFrom(src);
  }
  return *this;
}

bool Volume3d::operator==(Volume3d& other) const
{
  return Compare(&other) == 0;
}

bool Volume3d::checkBulkloadData(BulkLoadOptions options) const
{
  bool success = bulkloadData->getContainer()
                               .checkVolume(1, (options & REPAIR) != 0);
  if (!success)
  {
    cerr << "Volume check not successful!" << endl;
  }
  return success;
}
