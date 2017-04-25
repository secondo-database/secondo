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

#include "FeedProject.h"

#include "CRelTI.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include "Project.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "StreamValueMapping.h"
#include <string>
#include "Symbols.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

//FeedProject-------------------------------------------------------------------

FeedProject::FeedProject() :
  Operator(info, StreamValueMapping<State>, TypeMapping)
{
}

const OperatorInfo FeedProject::info = OperatorInfo(
  "feedproject", "crel x symbol x symbol* -> stream tblock",
  "_ feedproject[ list ]",
  "Produces a stream of tuple blocks from a relation projected on the columns "
  "determined by the provided names.",
  "query people feedproject[Name, Age] consume");

ListExpr FeedProject::TypeMapping(ListExpr args)
{
  if (!nl->HasLength(args, 2))
  {
    return listutils::typeError("Expected two arguments!");
  }

  const ListExpr crelType = nl->First(args);
  string crelTypeError;

  if (!CRelTI::Check(crelType, crelTypeError))
  {
    return listutils::typeError(crelTypeError);
  }

  const Project::Info info(CRelTI(crelType, false), nl->Second(args));

  if (info.HasError())
  {
    return listutils::typeError("Error in second argument (column names): " +
                                info.GetError());
  }

  const ListExpr projectedBlockType = info.GetBlockTypeInfo().GetTypeExpr();

  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           info.GetIndicesExpr(),
                           nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                           projectedBlockType));
}

//FeedProject::State------------------------------------------------------------

FeedProject::State::State(ArgVector args, Supplier s) :
  m_blockIterator(((CRel*)args[0].addr)->GetBlockIterator())
{
  qp->DeleteResultStorage(s);

  const size_t indexCount = ((CcInt*)args[2].addr)->GetValue();

  for (size_t i = 0; i < indexCount; ++i)
  {
    m_indices.push_back(((CcInt*)args[i + 3].addr)->GetValue());
  }
}

TBlock *FeedProject::State::Request()
{
  if (m_blockIterator.IsValid())
  {
    TBlock *block = new TBlock(m_blockIterator.Get(), &m_indices.front(),
                               m_indices.size());

    m_blockIterator.MoveToNext();

    return block;
  }

  return nullptr;
}