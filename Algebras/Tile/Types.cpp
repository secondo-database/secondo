 
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

#include "Types.h"
#include "StandardTypes.h"
#include "t/tint.h"
#include "t/treal.h"
#include "t/tbool.h"
#include "t/tstring.h"
#include "mt/mtint.h"
#include "mt/mtreal.h"
#include "mt/mtbool.h"
#include "mt/mtstring.h"
#include "it/itint.h"
#include "it/itreal.h"
#include "it/itbool.h"
#include "it/itstring.h"

namespace TileAlgebra
{

/*
definition of IstType function

*/

bool IstType(const std::string& rType)
{
  bool bIstType = false;

  if(rType.empty() == false)
  {
    if(rType == tint::BasicType() ||
       rType == treal::BasicType() ||
       rType == tbool::BasicType() ||
       rType == tstring::BasicType())
    {
      bIstType = true;
    }
  }

  return bIstType;
}

/*
definition of IsmtType function

*/

bool IsmtType(const std::string& rType)
{
  bool bIsmtType = false;

  if(rType.empty() == false)
  {
    if(rType == mtint::BasicType() ||
       rType == mtreal::BasicType() ||
       rType == mtbool::BasicType() ||
       rType == mtstring::BasicType())
    {
      bIsmtType = true;
    }
  }

  return bIsmtType;
}

/*
definition of IsitType function

*/

bool IsitType(const std::string& rType)
{
  bool bIsitType = false;

  if(rType.empty() == false)
  {
    if(rType == itint::BasicType() ||
       rType == itreal::BasicType() ||
       rType == itbool::BasicType() ||
       rType == itstring::BasicType())
    {
      bIsitType = true;
    }
  }

  return bIsitType;
}

/*
definition of GetValueWrapperType function

*/

std::string GetValueWrapperType(const std::string& rType)
{
  std::string valueWrapperType;

  if(rType.empty() == false)
  {
    if(rType == tint::BasicType() ||
       rType == mtint::BasicType() ||
       rType == itint::BasicType())
    {
      valueWrapperType = CcInt::BasicType();
    }

    if(rType == treal::BasicType() ||
       rType == mtreal::BasicType() ||
       rType == itreal::BasicType())
    {
      valueWrapperType = CcReal::BasicType();
    }

    if(rType == tbool::BasicType() ||
       rType == mtbool::BasicType() ||
       rType == itbool::BasicType())
    {
      valueWrapperType = CcBool::BasicType();
    }

    if(rType == tstring::BasicType() ||
       rType == mtstring::BasicType() ||
       rType == itstring::BasicType())
    {
      valueWrapperType = CcString::BasicType();
    }
  }

  return valueWrapperType;
}

/*
declaration of GetMType function

*/

std::string GetMType(const std::string& rType)
{
  std::string MType;

  if(rType.empty() == false)
  {
    if(rType == CcInt::BasicType() ||
       rType == tint::BasicType() ||
       rType == mtint::BasicType() ||
       rType == itint::BasicType())
    {
      MType = MInt::BasicType();
    }

    if(rType == CcReal::BasicType() ||
       rType == treal::BasicType() ||
       rType == mtreal::BasicType() ||
       rType == itreal::BasicType())
    {
      MType = MReal::BasicType();
    }

    if(rType == CcBool::BasicType() ||
       rType == tbool::BasicType() ||
       rType == mtbool::BasicType() ||
       rType == itbool::BasicType())
    {
      MType = MBool::BasicType();
    }

    if(rType == CcString::BasicType() ||
       rType == tstring::BasicType() ||
       rType == mtstring::BasicType() ||
       rType == itstring::BasicType())
    {
      MType = MString::BasicType();
    }
  }

  return MType;
}

/*
definition of GettType function

*/

std::string GettType(const std::string& rType)
{
  std::string tType;

  if(rType.empty() == false)
  {
    if(rType == CcInt::BasicType() ||
       rType == mtint::BasicType() ||
       rType == itint::BasicType())
    {
      tType = tint::BasicType();
    }

    if(rType == CcReal::BasicType() ||
       rType == mtreal::BasicType() ||
       rType == itreal::BasicType())
    {
      tType = treal::BasicType();
    }

    if(rType == CcBool::BasicType() ||
       rType == mtbool::BasicType() ||
       rType == itbool::BasicType())
    {
      tType = tbool::BasicType();
    }

    if(rType == CcString::BasicType() ||
       rType == mtstring::BasicType() ||
       rType == itstring::BasicType())
    {
      tType = tstring::BasicType();
    }
  }

  return tType;
}

/*
definition of GetmtType function

*/

std::string GetmtType(const std::string& rType)
{
  std::string mtType;

  if(rType.empty() == false)
  {
    if(rType == CcInt::BasicType() ||
       rType == tint::BasicType() ||
       rType == itint::BasicType())
    {
      mtType = mtint::BasicType();
    }

    if(rType == CcReal::BasicType() ||
       rType == treal::BasicType() ||
       rType == itreal::BasicType())
    {
      mtType = mtreal::BasicType();
    }

    if(rType == CcBool::BasicType() ||
       rType == tbool::BasicType() ||
       rType == itbool::BasicType())
    {
      mtType = mtbool::BasicType();
    }

    if(rType == CcString::BasicType() ||
       rType == tstring::BasicType() ||
       rType == itstring::BasicType())
    {
      mtType = mtstring::BasicType();
    }
  }

  return mtType;
}

/*
definition of GetitType function

*/

std::string GetitType(const std::string& rType)
{
  std::string itType;

  if(rType.empty() == false)
  {
    if(rType == CcInt::BasicType() ||
       rType == tint::BasicType() ||
       rType == mtint::BasicType())
    {
      itType = itint::BasicType();
    }

    if(rType == CcReal::BasicType() ||
       rType == treal::BasicType() ||
       rType == mtreal::BasicType())
    {
      itType = itreal::BasicType();
    }

    if(rType == CcBool::BasicType() ||
       rType == tbool::BasicType() ||
       rType == mtbool::BasicType())
    {
      itType = itbool::BasicType();
    }

    if(rType == CcString::BasicType() ||
       rType == tstring::BasicType() ||
       rType == mtstring::BasicType())
    {
      itType = itstring::BasicType();
    }
  }

  return itType;
}

}
