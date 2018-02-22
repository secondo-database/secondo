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

#include "Rename.h"

#include <cstdint>
#include "ListUtils.h"
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include <string>
#include "StreamValueMapping.h"
#include "Symbols.h"
#include "../TypeConstructors/TBlockTC.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using listutils::isStream;
using listutils::isValidAttributeName;
using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

Rename::Rename() :
  Operator(info, StreamValueMapping<State>, TypeMapping)
{
}

const OperatorInfo Rename::info = OperatorInfo(
  "rename", "stream(tblock) x symbol -> stream(tblock)",
  "_ rename[ _ ]",
  "Transforms a stream of tuple-blocks into one to which's column names the "
  "specified suffix got appended.",
  "");

ListExpr Rename::TypeMapping(ListExpr args)
{
  if (!nl->HasLength(args, 2))
  {
    return GetTypeError("Expected two arguments.");
  }

  ListExpr stream = nl->First(args);

  if (!isStream(stream))
  {
    return GetTypeError(0, "Isn't a stream.");
  }

  const ListExpr tblock = GetStreamType(stream);

  if (!TBlockTI::Check(tblock))
  {
    return GetTypeError(0, "Isn't a stream of tblock.");
  }

  const ListExpr suffixExpr = nl->Second(args);

  if (!nl->IsNodeType(SymbolType, suffixExpr))
  {
    return GetTypeError(1, "Isn't a symbol.");
  }

  const string suffix = nl->SymbolValue(suffixExpr);

  string error;

  TBlockTI blockInfo(tblock, false);

  const uint64_t blockAttributeCount = blockInfo.columnInfos.size();
  for (uint64_t i = 0; i < blockAttributeCount; ++i)
  {
    const string name = blockInfo.columnInfos[i].name + "_" + suffix;

    if (!isValidAttributeName(nl->SymbolAtom(name), error))
    {
      return GetTypeError("Resulting attribute name '" + name + "' is not "
                          "valid");
    }

    blockInfo.columnInfos[i].name = name;
  }

  return blockInfo.GetTypeExpr(true);
}

Rename::State::State(Word* args, Supplier s) :
  m_stream(args[0])
{
  qp->DeleteResultStorage(s);

  m_stream.open();
}

Rename::State::~State()
{
  m_stream.close();
}

TBlock *Rename::State::Request()
{
  return m_stream.request();
}
