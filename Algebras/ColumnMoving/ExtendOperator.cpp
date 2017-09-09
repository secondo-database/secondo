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

#include "ExtendOperator.h"

#include "AttrArray.h"
#include "LongIntsTC.h"
#include "ListExprUtils.h"
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "Project.h"
#include "QueryProcessor.h"
#include "Shared.h"
#include "StreamValueMapping.h"
#include <string>
#include "Symbols.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::string;
using std::vector;

extern NestedList *nl;
extern QueryProcessor *qp;

//ExtendOperator---------------------------------------------------------

ExtendOperator::ExtendOperator() :
  Operator(info, StreamValueMapping<State, CreateState>,
           TypeMapping)
{
  SetUsesArgsInTypeMapping();
}

const OperatorInfo ExtendOperator::info = 
  OperatorInfo("cextend", 
               "stream(tblock) x (e*) -> stream(tblock) \n"
               "where each e is of form (name (map tblock attrarray)) \n"
               "where name is a valid attribute name",
               "_ extend [ x0: fun , ... , xn: fun ]",
               "Extends a stream of tuple blocks by the given attributes",
               "query cities feed cextend[Dist: distance(.Location, here)] "
               "consume");

ListExpr ExtendOperator::TypeMapping(ListExpr args)
{
  auto predicateError = [](int i, const char *s0, const char *s1) -> ListExpr 
  {
    return GetTypeError(i, (std::string(s0) + std::to_string(i)) + s1);
  };
  
  if (!nl->HasLength(args, 2))
    return GetTypeError("Two arguments expected.");
    
  const ListExpr tupleArg = nl->First(args);
  const ListExpr extensionListContainerArg = nl->Second(args);
  
  string error;
  ListExpr blockType;
  if (!IsBlockStream(nl->First(tupleArg), blockType, error))
    return GetTypeError(0, "block stream", error);

  TBlockTI blockInfo = TBlockTI(blockType, false);
  ListExpr newColumns = nl->Empty(), newColumnsEnd;
  
  std::set<std::string> columnNames;
  for (auto & n : blockInfo.columnInfos)
    columnNames.insert(n.name);

  if (!nl->HasLength(extensionListContainerArg, 2))
    return GetTypeError("Extension List does not have the expected form.");
    
  const ListExpr extensionListArg = nl->First(extensionListContainerArg);

  for (int i = 1; i <= nl->ListLength(extensionListArg); i++) {
    ListExpr extension = nl->Nth(i, extensionListArg);
    if (!nl->HasLength(extension, 2))
      return predicateError(i, "the extend parameter ", 
        " is not composed of a new attribute name and a mapping function.");

    const ListExpr columnName    = nl->First (extension);
    const ListExpr columnMapType = nl->Second(extension);
    
    if (!nl->IsNodeType(SymbolType, columnName))
      return predicateError(i, "the attribute name of extend parameter ",
        " is not a valid column name.");
        
    std::string columnNameString = nl->SymbolValue(columnName);
    if (columnNames.find(columnNameString) != columnNames.end())
      return predicateError(i, "the column name of extend parameter ",
        " is not unique.");
        
    columnNames.insert(columnNameString);
    
    if (!nl->HasLength(columnMapType, 3) ||
        !nl->IsEqual(nl->First(columnMapType), Symbols::MAP()) ||
        !nl->Equal(nl->Second(columnMapType), blockType))
    {
      return predicateError(i, "predicate ", " is not a map that accepts "
        "the given tuple block type as argument.");
    }
    
    const ListExpr columnMapResultType = nl->Third(columnMapType);
    
    CRelAlgebra::AttrArrayTypeConstructor *mapResultTC =
      CRelAlgebra::AttrArray::GetTypeConstructor(columnMapResultType);

    if (mapResultTC == nullptr) 
      return predicateError(i, "the result of predicate ", " is not of kind "
        "ATTRARRAY.");

    ListExpr newColumn = nl->TwoElemList(columnName, columnMapResultType);
    
    if (nl->IsEmpty(newColumns))
      newColumnsEnd = newColumns = nl->OneElemList(newColumn);
    else
      newColumnsEnd = nl->Append(newColumnsEnd, newColumn);
  }
  
  blockInfo.AppendColumnInfos(newColumns);

  return blockInfo.GetTypeExpr(true);
}

typename ExtendOperator::State *ExtendOperator::CreateState(ArgVector args,
  Supplier s)
{
  qp->DeleteResultStorage(s);
  const TBlockTI blockType = TBlockTI(qp->GetType(s), false);

  return new State(args, blockType);
}

//ExtendOperator::State------------------------------------------------

ExtendOperator::State::State(ArgVector args, const TBlockTI &blockType) :
  m_stream(args[0].addr),
  m_ExtendParameter(args[1].addr),
  m_blockInfo(blockType.GetBlockInfo())
{
  m_stream.open();
}

ExtendOperator::State::~State()
{
  m_stream.close();
}

TBlock *ExtendOperator::State::Request()
{
/*  TBlock *sourceBlock = m_stream.request();
  
  if (sourceBlock == nullptr)
    return nullptr;
    
  int extendParameterCount = qp->GetNoSons(m_ExtendParameter);
  std::vector<AttrArray*> extensionColumns;

  for(int i = 0; i < extendParameterCount; i++) {
    Supplier extension = qp->GetSupplier(m_ExtendParameter, i);
    Supplier function = qp->GetSupplier(extension, 1);
    (*qp->Argument(function))[0].setAddr(sourceBlock);

    Word newAttrArray;
    qp->Request(function, newAttrArray);
    extensionColumns.push_back(static_cast<AttrArray*>(newAttrArray.addr));
  }
  
  TBlock *result = new TBlock(*sourceBlock, m_blockInfo, 
    extensionColumns.data());

  sourceBlock->DecRef();
  sourceBlock = nullptr;

  return result;*/
  
  TBlock *sourceBlock = m_stream.request();
  
  if (sourceBlock == nullptr)
    return nullptr;
    
  size_t sourceColumnCount = sourceBlock->GetInfo()->columnCount;
  size_t extendColumnCount = qp->GetNoSons(m_ExtendParameter);
  size_t columnCount = sourceColumnCount + extendColumnCount;

  AttrArray *columns[columnCount];

  size_t c = 0;
  
  while (c < sourceColumnCount) {
    columns[c] = &sourceBlock->GetAt(c);
    c++;
  }
  
  size_t e = 0;

  while (c < columnCount) {
    Supplier extension = qp->GetSupplier(m_ExtendParameter, e);
    Supplier function = qp->GetSupplier(extension, 1);
    (*qp->Argument(function))[0].setAddr(sourceBlock);

    Word newAttrArray;
    qp->Request(function, newAttrArray);
    columns[c] = static_cast<AttrArray*>(newAttrArray.addr);
    
    e++;
    c++;
  }

  const TBlockFilter &filter = sourceBlock->GetFilter();
  const size_t rowCount = filter.GetRowCount();
  TBlock *target = new TBlock(m_blockInfo, 0, 0);

  for (size_t r = 0; r < rowCount; r++) {
    const size_t sourceRow = filter.GetAt(r);
    SharedArray<AttrArrayEntry> tuple(columnCount);

    size_t c = 0;

    while (c < sourceColumnCount) {
      tuple[c] = columns[c]->GetAt(sourceRow);
      c++;
    }

    while (c < columnCount) {
      tuple[c] = columns[c]->GetAt(r);
      c++;
    }

    target->Append(tuple.GetPointer());
  }

  sourceBlock->DecRef();
  sourceBlock = nullptr;

  return target;
}

