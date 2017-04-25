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

#include "TransformStream.h"

#include <cstddef>
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "StreamValueMapping.h"
#include <string>
#include "Symbols.h"
#include "TBlock.h"
#include "TBlockTI.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using listutils::isStream;
using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

TransformStream::TransformStream() :
  Operator(info, StreamValueMapping<State>, TypeMapping)
{
}

const OperatorInfo TransformStream::info = OperatorInfo(
  "transformstream", "stream(tblock) -> stream(tuple)",
  "_ transformstream",
  "Transforms a stream of tuple blocks into one of tuples.",
  "");

ListExpr TransformStream::TypeMapping(ListExpr args)
{
  //Expect one parameter
  if (!nl->HasLength(args, 1))
  {
    return listutils::typeError("Expected one argument!");
  }

  //Check first parameter for stream
  ListExpr stream = nl->First(args);
  if (!isStream(stream))
  {
    return listutils::typeError("Argument isn't a stream!");
  }

  const ListExpr tblockType = GetStreamType(stream);

  //Check first parameter's stream type for 'tblock'
  string typeError;
  if (!TBlockTI::Check(tblockType, typeError))
  {
    return listutils::typeError("Argument isn't a stream of tblock: " +
                                typeError);
  }

  //Result is a stream of 'tuple'
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         TBlockTI(tblockType, false).GetTupleTypeExpr());
}

TransformStream::State::State(ArgVector args, Supplier s) :
  m_stream(args[0]),
  m_block(nullptr),
  m_tupleType(((Tuple*)qp->ResultStorage(s).addr)->GetTupleType())
{
  m_tupleType->IncReference();

  qp->DeleteResultStorage(s);

  m_stream.open();
}

Tuple *TransformStream::State::Request()
{
  if (!m_blockIterator.IsValid() || !m_blockIterator.MoveToNext())
  {
    do
    {
      if (m_block != nullptr)
      {
        m_block->DecRef();
      }

      if ((m_block = m_stream.request()) == nullptr)
      {
        return nullptr;
      }
    }
    while (!(m_blockIterator = m_block->GetIterator()).IsValid());
  }

  const TBlockEntry &blockTuple = m_blockIterator.Get();
  Tuple *tuple = new Tuple(m_tupleType);

  const size_t columnCount = m_block->GetColumnCount();
  for (size_t i = 0; i < columnCount; ++i)
  {
    Attribute *attribute = blockTuple[i].GetAttribute(true);

    tuple->PutAttribute(i, attribute);
  }

  return tuple;
}