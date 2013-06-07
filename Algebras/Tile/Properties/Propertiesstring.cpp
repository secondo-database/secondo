/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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

*/

#include "Propertiesstring.h"

namespace TileAlgebra
{

/*
implementation of template class Properties<std::string>

*/

std::string Properties<std::string>::GetUndefinedValue()
{
  return UNDEFINED_STRING;
}

std::string Properties<std::string>::GetValue(const NList& rNList)
{
  return rNList.str();
}

CcString Properties<std::string>::GetWrappedValue(const std::string& rstring)
{
  return CcString(!IsUndefinedValue(rstring), rstring);
}

bool Properties<std::string>::IsUndefinedValue(const std::string& rstring)
{
  bool bUndefinedValue = false;
  
  if(rstring == GetUndefinedValue())
  {
    bUndefinedValue = true;
  }

  return bUndefinedValue;
}

bool Properties<std::string>::IsValidValueType(const NList& rNList)
{
  bool bValidValueType = rNList.isString();

  return bValidValueType;
}

NList Properties<string>::ToNList(const std::string& rstring)
{
  NList nList = NList(Symbol::UNDEFINED());

  if(IsUndefinedValue(rstring) == false)
  {
    nList = NList(rstring, true);
  }

  return nList;
}

}
