 
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
#include "../t/tint.h"
#include "../t/treal.h"
#include "../t/tbool.h"
#include "../t/tstring.h"
#include "../mt/mtint.h"
#include "../mt/mtreal.h"
#include "../mt/mtbool.h"
#include "../mt/mtstring.h"
#include "../it/itint.h"
#include "../it/itreal.h"
#include "../it/itbool.h"
#include "../it/itstring.h"
#include "../../Raster2/sint.h"
#include "../../Raster2/sreal.h"
#include "../../Raster2/sbool.h"
#include "../../Raster2/sstring.h"
#include "../../Raster2/msint.h"
#include "../../Raster2/msreal.h"
#include "../../Raster2/msbool.h"
#include "../../Raster2/msstring.h"
#include "../../Raster2/isint.h"
#include "../../Raster2/isreal.h"
#include "../../Raster2/isbool.h"
#include "../../Raster2/isstring.h"

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
definition of GetisTypes function

*/

void GetisTypes(std::vector<std::string>& risTypes)
{
  risTypes.clear();
  risTypes.push_back(raster2::isint::BasicType());
  risTypes.push_back(raster2::isreal::BasicType());
  risTypes.push_back(raster2::isbool::BasicType());
  risTypes.push_back(raster2::isstring::BasicType());
}

/*
definition of GetType function

*/

std::string GetType(const std::string& rType,
                    const std::vector<std::string>& rTypes)
{
  std::string type;

  if(rType.empty() == false &&
     rTypes.size() > 0)
  {
    std::vector<std::string> valueWrapperTypes;
    std::vector<std::string> MTypes;
    std::vector<std::string> tTypes;
    std::vector<std::string> mtTypes;
    std::vector<std::string> itTypes;
    std::vector<std::string> sTypes;
    std::vector<std::string> msTypes;
    std::vector<std::string> isTypes;

    GetValueWrapperTypes(valueWrapperTypes);
    GetMTypes(MTypes);
    GettTypes(tTypes);
    GetmtTypes(mtTypes);
    GetitTypes(itTypes);
    GetsTypes(sTypes);
    GetmsTypes(msTypes);
    GetisTypes(isTypes);

    if(rTypes.size() == valueWrapperTypes.size() &&
       rTypes.size() == MTypes.size() &&
       rTypes.size() == tTypes.size() &&
       rTypes.size() == mtTypes.size() &&
       rTypes.size() == itTypes.size() &&
       rTypes.size() == sTypes.size() &&
       rTypes.size() == msTypes.size() &&
       rTypes.size() == isTypes.size())
    {
      for(size_t i = 0; i < rTypes.size(); i++)
      {
        if(rType == valueWrapperTypes[i] ||
           rType == MTypes[i] ||
           rType == tTypes[i] ||
           rType == mtTypes[i] ||
           rType == itTypes[i] ||
           rType == sTypes[i] ||
           rType == msTypes[i] ||
           rType == isTypes[i])
        {
          type = rTypes[i];
          break;
        }
      }
    }
  }

  return type;
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
    GetValueWrapperTypes(valueWrapperTypes);
    valueWrapperType = GetType(rType, valueWrapperTypes);
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
    std::vector<std::string> MTypes;
    GetMTypes(MTypes);
    MType = GetType(rType, MTypes);
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
    std::vector<std::string> tTypes;
    GettTypes(tTypes);
    tType = GetType(rType, tTypes);
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
    std::vector<std::string> mtTypes;
    GetmtTypes(mtTypes);
    mtType = GetType(rType, mtTypes);
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
    std::vector<std::string> itTypes;
    GetitTypes(itTypes);
    itType = GetType(rType, itTypes);
  }

  return itType;
}

/*
definition of GetsType function

*/

std::string GetsType(const std::string& rType)
{
  std::string sType;

  if(rType.empty() == false)
  {
    std::vector<std::string> sTypes;
    GetsTypes(sTypes);
    sType = GetType(rType, sTypes);
  }

  return sType;
}

/*
definition of GetmsType function

*/

std::string GetmsType(const std::string& rType)
{
  std::string msType;

  if(rType.empty() == false)
  {
    std::vector<std::string> msTypes;
    GetmsTypes(msTypes);
    msType = GetType(rType, msTypes);
  }

  return msType;
}

/*
definition of GetisType function

*/

std::string GetisType(const std::string& rType)
{
  std::string isType;

  if(rType.empty() == false)
  {
    std::vector<std::string> isTypes;
    GetisTypes(isTypes);
    isType = GetType(rType, isTypes);
  }

  return isType;
}

/*
definition of IsType function

*/

bool IsType(const std::string& rType,
            const std::vector<std::string>& rTypes)
{
  bool bIsType = false;

  if(rType.empty() == false &&
     rTypes.size() > 0)
  {
    for(size_t i = 0; i < rTypes.size(); i++)
    {
      if(rType == rTypes[i])
      {
        bIsType = true;
        break;
      }
    }
  }

  return bIsType;
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
    bIsValueWrapperType = IsType(rType, valueWrapperTypes);
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
    bIsMType = IsType(rType, MTypes);
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
    bIstType = IsType(rType, tTypes);
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
    bIsmtType = IsType(rType, mtTypes);
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
    bIsitType = IsType(rType, itTypes);
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
    bIssType = IsType(rType, sTypes);
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
    bIsmsType = IsType(rType, msTypes);
  }

  return bIsmsType;
}

/*
definition of IsisType function

*/

bool IsisType(const std::string& rType)
{
  bool bIsisType = false;

  if(rType.empty() == false)
  {
    std::vector<std::string> isTypes;
    GetisTypes(isTypes);
    bIsisType = IsType(rType, isTypes);
  }

  return bIsisType;
}

}
