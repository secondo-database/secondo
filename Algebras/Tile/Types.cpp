 
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
#include "../Raster2/sint.h"
#include "../Raster2/sreal.h"
#include "../Raster2/sbool.h"
#include "../Raster2/sstring.h"
#include "../Raster2/msint.h"
#include "../Raster2/msreal.h"
#include "../Raster2/msbool.h"
#include "../Raster2/msstring.h"

namespace TileAlgebra
{

/*
definition of GetValueWrapperTypes function

*/

void GetValueWrapperTypes(std::vector<std::string>& rValueWrapperTypes)
{
  rValueWrapperTypes.clear();
  rValueWrapperTypes.push_back(CcInt::BasicType());
  rValueWrapperTypes.push_back(CcReal::BasicType());
  rValueWrapperTypes.push_back(CcBool::BasicType());
  rValueWrapperTypes.push_back(CcString::BasicType());
}

/*
definition of GetMTypes function

*/

void GetMTypes(std::vector<std::string>& rMTypes)
{
  rMTypes.clear();
  rMTypes.push_back(MInt::BasicType());
  rMTypes.push_back(MReal::BasicType());
  rMTypes.push_back(MBool::BasicType());
  rMTypes.push_back(MString::BasicType());
}

/*
definition of GettTypes function

*/

void GettTypes(std::vector<std::string>& rtTypes)
{
  rtTypes.clear();
  rtTypes.push_back(tint::BasicType());
  rtTypes.push_back(treal::BasicType());
  rtTypes.push_back(tbool::BasicType());
  rtTypes.push_back(tstring::BasicType());
}

/*
definition of GetmtTypes function

*/

void GetmtTypes(std::vector<std::string>& rmtTypes)
{
  rmtTypes.clear();
  rmtTypes.push_back(mtint::BasicType());
  rmtTypes.push_back(mtreal::BasicType());
  rmtTypes.push_back(mtbool::BasicType());
  rmtTypes.push_back(mtstring::BasicType());
}

/*
definition of GetitTypes function

*/

void GetitTypes(std::vector<std::string>& ritTypes)
{
  ritTypes.clear();
  ritTypes.push_back(itint::BasicType());
  ritTypes.push_back(itreal::BasicType());
  ritTypes.push_back(itbool::BasicType());
  ritTypes.push_back(itstring::BasicType());
}

/*
definition of GetsTypes function

*/

void GetsTypes(std::vector<std::string>& rsTypes)
{
  rsTypes.clear();
  rsTypes.push_back(raster2::sint::BasicType());
  rsTypes.push_back(raster2::sreal::BasicType());
  rsTypes.push_back(raster2::sbool::BasicType());
  rsTypes.push_back(raster2::sstring::BasicType());
}

/*
definition of GetmsTypes function

*/

void GetmsTypes(std::vector<std::string>& rmsTypes)
{
  rmsTypes.clear();
  rmsTypes.push_back(raster2::msint::BasicType());
  rmsTypes.push_back(raster2::msreal::BasicType());
  rmsTypes.push_back(raster2::msbool::BasicType());
  rmsTypes.push_back(raster2::msstring::BasicType());
}

/*
definition of GetValueWrapperType function

*/

std::string GetValueWrapperType(const std::string& rType)
{
  std::string valueWrapperType;

  if(rType.empty() == false)
  {
    std::vector<std::string> valueWrapperTypes;
    std::vector<std::string> MTypes;
    std::vector<std::string> tTypes;
    std::vector<std::string> mtTypes;
    std::vector<std::string> itTypes;

    GetValueWrapperTypes(valueWrapperTypes);
    GetMTypes(MTypes);
    GettTypes(tTypes);
    GetmtTypes(mtTypes);
    GetitTypes(itTypes);

    if(valueWrapperTypes.size() == MTypes.size() &&
       valueWrapperTypes.size() == tTypes.size() &&
       valueWrapperTypes.size() == mtTypes.size() &&
       valueWrapperTypes.size() == itTypes.size())
    {
      for(size_t i = 0; i < valueWrapperTypes.size(); i++)
      {
        if(rType == MTypes[i] ||
           rType == tTypes[i] ||
           rType == mtTypes[i] ||
           rType == itTypes[i])
        {
          valueWrapperType = valueWrapperTypes[i];
          break;
        }
      }
    }

    else
    {
      assert(false);
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
    std::vector<std::string> valueWrapperTypes;
    std::vector<std::string> MTypes;
    std::vector<std::string> tTypes;
    std::vector<std::string> mtTypes;
    std::vector<std::string> itTypes;

    GetValueWrapperTypes(valueWrapperTypes);
    GetMTypes(MTypes);
    GettTypes(tTypes);
    GetmtTypes(mtTypes);
    GetitTypes(itTypes);

    if(MTypes.size() == valueWrapperTypes.size() &&
       MTypes.size() == tTypes.size() &&
       MTypes.size() == mtTypes.size() &&
       MTypes.size() == itTypes.size())
    {
      for(size_t i = 0; i < MTypes.size(); i++)
      {
        if(rType == valueWrapperTypes[i] ||
           rType == tTypes[i] ||
           rType == mtTypes[i] ||
           rType == itTypes[i])
        {
          MType = MTypes[i];
          break;
        }
      }
    }

    else
    {
      assert(false);
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
    std::vector<std::string> valueWrapperTypes;
    std::vector<std::string> MTypes;
    std::vector<std::string> tTypes;
    std::vector<std::string> mtTypes;
    std::vector<std::string> itTypes;

    GetValueWrapperTypes(valueWrapperTypes);
    GetMTypes(MTypes);
    GettTypes(tTypes);
    GetmtTypes(mtTypes);
    GetitTypes(itTypes);

    if(tTypes.size() == valueWrapperTypes.size() &&
       tTypes.size() == MTypes.size() &&
       tTypes.size() == mtTypes.size() &&
       tTypes.size() == itTypes.size())
    {
      for(size_t i = 0; i < tTypes.size(); i++)
      {
        if(rType == valueWrapperTypes[i] ||
           rType == MTypes[i] ||
           rType == mtTypes[i] ||
           rType == itTypes[i])
        {
          tType = tTypes[i];
          break;
        }
      }
    }

    else
    {
      assert(false);
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
    std::vector<std::string> valueWrapperTypes;
    std::vector<std::string> MTypes;
    std::vector<std::string> tTypes;
    std::vector<std::string> mtTypes;
    std::vector<std::string> itTypes;

    GetValueWrapperTypes(valueWrapperTypes);
    GetMTypes(MTypes);
    GettTypes(tTypes);
    GetmtTypes(mtTypes);
    GetitTypes(itTypes);

    if(mtTypes.size() == valueWrapperTypes.size() &&
       mtTypes.size() == MTypes.size() &&
       mtTypes.size() == tTypes.size() &&
       mtTypes.size() == itTypes.size())
    {
      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        if(rType == valueWrapperTypes[i] ||
           rType == MTypes[i] ||
           rType == tTypes[i] ||
           rType == itTypes[i])
        {
          mtType = mtTypes[i];
          break;
        }
      }
    }

    else
    {
      assert(false);
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
    std::vector<std::string> valueWrapperTypes;
    std::vector<std::string> MTypes;
    std::vector<std::string> tTypes;
    std::vector<std::string> mtTypes;
    std::vector<std::string> itTypes;

    GetValueWrapperTypes(valueWrapperTypes);
    GetMTypes(MTypes);
    GettTypes(tTypes);
    GetmtTypes(mtTypes);
    GetitTypes(itTypes);

    if(itTypes.size() == valueWrapperTypes.size() &&
       itTypes.size() == MTypes.size() &&
       itTypes.size() == tTypes.size() &&
       itTypes.size() == mtTypes.size())
    {
      for(size_t i = 0; i < itTypes.size(); i++)
      {
        if(rType == valueWrapperTypes[i] ||
           rType == MTypes[i] ||
           rType == tTypes[i] ||
           rType == mtTypes[i])
        {
          itType = itTypes[i];
          break;
        }
      }
    }

    else
    {
      assert(false);
    }
  }

  return itType;
}

/*
definition of IsValueWrapperType function

*/

bool IsValueWrapperType(const std::string& rType)
{
  bool bIsValueWrapperType = false;

  if(rType.empty() == false)
  {
    std::vector<std::string> valueWrapperTypes;
    GetValueWrapperTypes(valueWrapperTypes);

    for(size_t i = 0; i < valueWrapperTypes.size(); i++)
    {
      if(rType == valueWrapperTypes[i])
      {
        bIsValueWrapperType = true;
        break;
      }
    }
  }

  return bIsValueWrapperType;
}

/*
definition of IsMType function

*/

bool IsMType(const std::string& rType)
{
  bool bIsMType = false;

  if(rType.empty() == false)
  {
    std::vector<std::string> MTypes;
    GetMTypes(MTypes);

    for(size_t i = 0; i < MTypes.size(); i++)
    {
      if(rType == MTypes[i])
      {
        bIsMType = true;
        break;
      }
    }
  }

  return bIsMType;
}

/*
definition of IstType function

*/

bool IstType(const std::string& rType)
{
  bool bIstType = false;

  if(rType.empty() == false)
  {
    std::vector<std::string> tTypes;
    GettTypes(tTypes);

    for(size_t i = 0; i < tTypes.size(); i++)
    {
      if(rType == tTypes[i])
      {
        bIstType = true;
        break;
      }
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
    std::vector<std::string> mtTypes;
    GetmtTypes(mtTypes);

    for(size_t i = 0; i < mtTypes.size(); i++)
    {
      if(rType == mtTypes[i])
      {
        bIsmtType = true;
        break;
      }
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
    std::vector<std::string> itTypes;
    GetitTypes(itTypes);

    for(size_t i = 0; i < itTypes.size(); i++)
    {
      if(rType == itTypes[i])
      {
        bIsitType = true;
        break;
      }
    }
  }

  return bIsitType;
}

/*
definition of IssType function

*/

bool IssType(const std::string& rType)
{
  bool bIssType = false;

  if(rType.empty() == false)
  {
    std::vector<std::string> sTypes;
    GetsTypes(sTypes);

    for(size_t i = 0; i < sTypes.size(); i++)
    {
      if(rType == sTypes[i])
      {
        bIssType = true;
        break;
      }
    }
  }

  return bIssType;
}

/*
definition of IsmsType function

*/

bool IsmsType(const std::string& rType)
{
  bool bIsmsType = false;

  if(rType.empty() == false)
  {
    std::vector<std::string> msTypes;
    GetmsTypes(msTypes);

    for(size_t i = 0; i < msTypes.size(); i++)
    {
      if(rType == msTypes[i])
      {
        bIsmsType = true;
        break;
      }
    }
  }

  return bIsmsType;
}

}
