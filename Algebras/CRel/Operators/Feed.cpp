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

#include "Feed.h"

#include "CRelTC.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "Project.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "StreamValueMapping.h"
#include <string>
#include "Symbols.h"
#include "TBlockTC.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

//Feed--------------------------------------------------------------------------

Feed::Feed() :
  Operator(info, StreamValueMapping<State>, TypeMapping)
{
}

const OperatorInfo Feed::info = OperatorInfo(
  "feed", "crel -> stream(tblock)",
  "_ feed",
  "Creates a stream of tuple blocks from a column-oriented relation.",
  "query people feed consume");

ListExpr Feed::TypeMapping(ListExpr args)
{
  //Expect one parameter
  if (!nl->HasLength(args, 1))
  {
    return GetTypeError("Expected one argument.");
  }

  ListExpr crelType = nl->First(args);

  //Check parameter for 'crel' type
  if (!CRelTI::Check(crelType))
  {
    return GetTypeError(0, "relation", "Not a crel.");
  }

  //Return type is stream of 'tblock'
  return TBlockTI(CRelTI(crelType, false)).GetTypeExpr(true);
}

//Feed::State-------------------------------------------------------------------

Feed::State::State(ArgVector args, Supplier s) :
  m_relation(*(CRel*)args[0].addr),
  m_blockIndex(0)
{
  qp->DeleteResultStorage(s);
}

TBlock *Feed::State::Request()
{
  if (m_blockIndex < m_relation.GetBlockCount())
  {
    TBlock *block = &m_relation.GetBlock(m_blockIndex++);
    block->IncRef();

    return block;
  }

  return nullptr;
}