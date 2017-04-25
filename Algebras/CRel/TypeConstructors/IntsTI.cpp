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

*/

#include "IntsTI.h"

#include "IntsTC.h"
#include "TIUtils.h"

using namespace CRelAlgebra;

using std::string;

extern NestedList *nl;

bool IntsTI::Check(ListExpr typeExpr)
{
  return SimpleTypeCheck(IntsTC::name, typeExpr);
}

bool IntsTI::Check(ListExpr typeExpr, string &error)
{
  return SimpleTypeCheck(IntsTC::name, typeExpr, error);
}

IntsTI::IntsTI(bool numeric) :
  m_isNumeric(numeric)
{
}

ListExpr IntsTI::GetTypeExpr() const
{
  return SimpleTypeExpr(IntsTC::name, m_isNumeric);
}

bool Ints2TI::Check(ListExpr typeExpr)
{
  return SimpleTypeCheck(Ints2TC::name, typeExpr);
}

bool Ints2TI::Check(ListExpr typeExpr, string &error)
{
  return SimpleTypeCheck(Ints2TC::name, typeExpr, error);
}

Ints2TI::Ints2TI(bool numeric) :
  IntsTI(numeric)
{
}