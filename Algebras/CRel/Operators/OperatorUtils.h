/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

#pragma once

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include <stdint.h>
#include <string>
#include "TBlockTC.h"
#include "TypeConstructor.h"
#include "TypeUtils.h"
#include <vector>

extern NestedList *nl;
extern QueryProcessor *qp;

namespace CRelAlgebra
{
  inline std::string GetTypeErrorString(uint64_t argumentIndex,
                                 const std::string &argumentName,
                                 const std::string &error)
  {
    static const std::string numberStrings[] =
    {
      "First", "Second", "Third", "Fourth", "Fifth", "Sixth", "Seventh",
      "Eighth", "Ninth"
    };

    return numberStrings[argumentIndex] + " argument (" + argumentName + ") is "
           "not valid: " + error;
  }

  inline std::string GetTypeErrorString(uint64_t argumentIndex,
                                        const std::string &error)
  {
    static const std::string numberStrings[] =
    {
      "First", "Second", "Third", "Fourth", "Fifth", "Sixth", "Seventh",
      "Eighth", "Ninth"
    };

    return numberStrings[argumentIndex] + " argument is not valid: " + error;
  }

  inline ListExpr GetTypeError(const std::string &error)
  {
    return listutils::typeError(error);
  }

  inline ListExpr GetTypeError(uint64_t argumentIndex,
                               const std::string &argumentName,
                               const std::string &error)
  {
    return listutils::typeError(GetTypeErrorString(argumentIndex, argumentName,
                                                   error));
  }

  inline ListExpr GetTypeError(uint64_t argumentIndex, const std::string &error)
  {
    return listutils::typeError(GetTypeErrorString(argumentIndex, error));
  }

  inline bool IsBlockStream(ListExpr typeExpr, std::string &error)
  {
    if (listutils::isStream(typeExpr))
    {
      if (TBlockTI::Check(GetStreamType(typeExpr)))
      {
        return true;
      }
      else
      {
        error = "Not a stream of tblock.";
      }
    }
    else
    {
      error = "Not a stream.";
    }

    return false;
  }

  inline bool IsBlockStream(ListExpr typeExpr)
  {
    std::string error;
    return IsBlockStream(typeExpr, error);
  }

  inline bool IsBlockStream(ListExpr typeExpr, ListExpr &blockType,
                            std::string &error)
  {
    if (listutils::isStream(typeExpr))
    {
      const ListExpr streamTypeExpr = GetStreamType(typeExpr);

      if (TBlockTI::Check(streamTypeExpr))
      {
        blockType = streamTypeExpr;
        return true;
      }
      else
      {
        error = "Not a stream of tblock.";
      }
    }
    else
    {
      error = "Not a stream.";
    }


    return false;
  }

  inline bool IsBlockStream(ListExpr typeExpr, ListExpr &blockType)
  {
    std::string error;
    return IsBlockStream(typeExpr, blockType, error);
  }

  inline bool IsBlockStream(ListExpr typeExpr, TBlockTI &blockTypeInfo,
                            std::string &error)
  {
    ListExpr blockType;

    if (IsBlockStream(typeExpr, blockType, error))
    {
      blockTypeInfo = TBlockTI(blockType, false);
      return true;
    }

    return false;
  }

  inline bool IsBlockStream(ListExpr typeExpr, TBlockTI &blockTypeInfo)
  {
    std::string error;
    return IsBlockStream(typeExpr, blockTypeInfo, error);
  }

  inline bool TryGetSymbolValue(ListExpr symbolExpr, std::string &symbolValue,
                                std::string& error)
  {
    if (nl->IsNodeType(SymbolType, symbolExpr))
    {
      symbolValue = nl->SymbolValue(symbolExpr);
      return true;
    }

    error = "Not a symbol atom.";
    return false;
  }

  inline bool TryGetSymbolValue(ListExpr symbolExpr, std::string &symbolValue)
  {
    std::string error;
    return TryGetSymbolValue(symbolExpr, symbolValue, error);
  }

  inline bool GetIndexOfColumn(const TBlockTI &typeInfo,
                               const std::string &name, uint64_t &index)
  {
    uint64_t i = 0;

    for (const TBlockTI::ColumnInfo &columnInfo : typeInfo.columnInfos)
    {
      if (name == columnInfo.name)
      {
        index = i;
        return true;
      }

      ++i;
    }

    return false;
  }

  inline std::vector<Word> GetSubArgvector(const Word &arg)
  {
    const uint64_t argCount = qp->GetNoSons(arg.addr);

    std::vector<Word> subArgs;

    for (uint64_t i = 0; i < argCount; ++i)
    {
      Word subArg;

      qp->EvalP(qp->GetSupplierSon(arg.addr, i), subArg, 0);

      subArgs.push_back(subArg);
    }

    return subArgs;
  }

  inline bool IsSpatialAttrArray(TypeConstructor &constructor, int &dimension)
  {
    if (constructor.MemberOf(Kind::SPATIALATTRARRAY1D()))
    {
      dimension = 1;
      return true;
    }

    if (constructor.MemberOf(Kind::SPATIALATTRARRAY2D()))
    {
      dimension = 2;
      return true;
    }

    if (constructor.MemberOf(Kind::SPATIALATTRARRAY3D()))
    {
      dimension = 3;
      return true;
    }

    if (constructor.MemberOf(Kind::SPATIALATTRARRAY4D()))
    {
      dimension = 4;
      return true;
    }

    if (constructor.MemberOf(Kind::SPATIALATTRARRAY8D()))
    {
      dimension = 8;
      return true;
    }

    return false;
  }

  inline bool IsSpatialAttrArray(TypeConstructor &constructor)
  {
    int dimension;

    return IsSpatialAttrArray(constructor, dimension);
  }

  inline bool IsSpatialAttrArray(ListExpr typeExpr, int &dimension)
  {
    TypeConstructor *constructor = GetTypeConstructor(typeExpr);

    return constructor != nullptr &&
           IsSpatialAttrArray(*constructor, dimension);
  }

  inline bool IsSpatialAttrArray(ListExpr typeExpr)
  {
    int dimension;

    return IsSpatialAttrArray(typeExpr, dimension);
  }

  inline bool IsSpatialAttr(TypeConstructor &constructor, int &dimension)
  {
    if (constructor.MemberOf(Kind::SPATIAL1D()))
    {
      dimension = 1;
      return true;
    }

    if (constructor.MemberOf(Kind::SPATIAL2D()))
    {
      dimension = 2;
      return true;
    }

    if (constructor.MemberOf(Kind::SPATIAL3D()))
    {
      dimension = 3;
      return true;
    }

    if (constructor.MemberOf(Kind::SPATIAL4D()))
    {
      dimension = 4;
      return true;
    }

    if (constructor.MemberOf(Kind::SPATIAL8D()))
    {
      dimension = 8;
      return true;
    }

    return false;
  }

  inline bool IsSpatialAttr(TypeConstructor &constructor)
  {
    int dimension;

    return IsSpatialAttr(constructor, dimension);
  }

  inline bool IsSpatialAttr(ListExpr typeExpr, int &dimension)
  {
    TypeConstructor *constructor = GetTypeConstructor(typeExpr);

    return constructor != nullptr &&
           IsSpatialAttr(*constructor, dimension);
  }

  inline bool IsSpatialAttr(ListExpr typeExpr)
  {
    int dimension;

    return IsSpatialAttr(typeExpr, dimension);
  }
}