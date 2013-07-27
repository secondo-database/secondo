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

/*
SECONDO includes

*/

#include "StandardTypes.h"

/*
Raster2Algebra includes

*/

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

/*
TileAlgebra includes

*/

#include "Types.h"
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

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Method GetValueWrapperTypes returns all value wrapper types.

author: Dirk Zacher
parameters: rValueWrapperTypes - reference to a vector of strings containing
                                 all value wrapper types
return value: -
exceptions: -

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
Method GetMTypes returns all moving types.

author: Dirk Zacher
parameters: rMTypes - reference to a vector of strings containing
                      all moving types
return value: -
exceptions: -

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
Method GettTypes returns all t types of Tile Algebra.

author: Dirk Zacher
parameters: rtTypes - reference to a vector of strings containing
                      all t types of Tile Algebra
return value: -
exceptions: -

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
Method GetmtTypes returns all mt types of Tile Algebra.

author: Dirk Zacher
parameters: rmtTypes - reference to a vector of strings containing
                       all mt types of Tile Algebra
return value: -
exceptions: -

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
Method GetitTypes returns all it types of Tile Algebra.

author: Dirk Zacher
parameters: ritTypes - reference to a vector of strings containing
                       all it types of Tile Algebra
return value: -
exceptions: -

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
Method GetsTypes returns all s types of Raster2 Algebra.

author: Dirk Zacher
parameters: rsTypes - reference to a vector of strings containing
                      all s types of Raster2 Algebra
return value: -
exceptions: -

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
Method GetmsTypes returns all ms types of Raster2 Algebra.

author: Dirk Zacher
parameters: rmsTypes - reference to a vector of strings containing
                       all ms types of Raster2 Algebra
return value: -
exceptions: -

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
Method GetisTypes returns all is types of Raster2 Algebra.

author: Dirk Zacher
parameters: risTypes - reference to a vector of strings containing
                       all is types of Raster2 Algebra
return value: -
exceptions: -

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
Method GetType returns corresponding type of given type
included in given types vector.

author: Dirk Zacher
parameters: rType - reference to a type
            rTypes - reference to a vector of strings of all types
                     of the type category of the returned type
return value: corresponding type of given type included in given types vector
exceptions: -

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
Method GetValueWrapperType returns corresponding value wrapper type
of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding value wrapper type of given type
exceptions: -

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
Method GetMType returns corresponding moving type of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding moving type of given type
exceptions: -

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
Method GettType returns corresponding Tile Algebra t type of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding Tile Algebra t type of given type
exceptions: -

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
Method GetmtType returns corresponding Tile Algebra mt type of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding Tile Algebra mt type of given type
exceptions: -

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
Method GetitType returns corresponding Tile Algebra it type of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding Tile Algebra it type of given type
exceptions: -

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
Method GetsType returns corresponding Raster2 Algebra s type of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding Raster2 Algebra s type of given type
exceptions: -

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
Method GetmsType returns corresponding Raster2 Algebra ms type of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding Raster2 Algebra ms type of given type
exceptions: -

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
Method GetisType returns corresponding Raster2 Algebra is type of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding Raster2 Algebra is type of given type
exceptions: -

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
Method IsType checks if given type is included in given types vector.

author: Dirk Zacher
parameters: rType - reference to a type
            rTypes - reference to a vector of strings of all types to check
return value: true, if given type is included in given types vector,
              otherwise false
exceptions: -

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
Method IsValueWrapperType checks if given type is a value wrapper type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a value wrapper type, otherwise false
exceptions: -

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
Method IsMType checks if given type is a moving type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a moving type, otherwise false
exceptions: -

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
Method IstType checks if given type is a Tile Algebra t type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a Tile Algebra t type, otherwise false
exceptions: -

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
Method IsmtType checks if given type is a Tile Algebra mt type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a Tile Algebra mt type, otherwise false
exceptions: -

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
Method IsitType checks if given type is a Tile Algebra it type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a Tile Algebra it type, otherwise false
exceptions: -

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
Method IssType checks if given type is a Raster2 Algebra s type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a Raster2 Algebra s type, otherwise false
exceptions: -

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
Method IsmsType checks if given type is a Raster2 Algebra ms type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a Raster2 Algebra ms type, otherwise false
exceptions: -

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
Method IsisType checks if given type is a Raster2 Algebra is type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a Raster2 Algebra is type, otherwise false
exceptions: -

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
