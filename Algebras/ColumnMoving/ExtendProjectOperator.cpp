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

1 ExtendProjectOperator.cpp

*/

#include "ExtendProjectOperator.h"

#include "Algebras/CRel/AttrArray.h"
#include "Algebras/CRel/TypeConstructors/LongIntsTC.h"
#include "Algebras/CRel/ListExprUtils.h"
#include "LogMsg.h"
#include "Algebras/CRel/Operators/OperatorUtils.h"
#include "Algebras/CRel/Operators/Project.h"
#include "QueryProcessor.h"
#include "Algebras/CRel/Shared.h"
#include "Algebras/CRel/Operators/StreamValueMapping.h"
#include <string>
#include "Symbols.h"
#include "Algebras/CRel/TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::string;
using std::vector;

extern NestedList *nl;
extern QueryProcessor *qp;

/*
1.1 Implementation of the class ExtendProjectOperator

the constructor sets the parameters in the base class operator. we will
use the generic ~StreamValueMapping~ implemented in the CRel algebra.
It will create a object of our nested class ~State~ when a stream opens and
it will call the method ~Request~ on this instance of ~State~ 
when it receives a request on an open stream.

*/

ExtendProjectOperator::ExtendProjectOperator() :
  Operator(info, StreamValueMapping<State, CreateState>,
           TypeMapping)
{
  SetUsesArgsInTypeMapping();
}

/*
description of the operator for the user interface

*/

const OperatorInfo ExtendProjectOperator::info = 
  OperatorInfo("cprojectextend", 
               "stream(tblock) x (a*) x (e*) -> stream(tblock) \n"
               "where each a is a name of one of the columns of tblock \n"
               "and each e is of form (name (map tblock attrarray)) \n"
               "where name is a valid attribute name",
               "_ extend [ a0, ..., an ; e0: fun , ... , en: fun ]",
               "Projects a stream of tuple blocks to the given attributs in "
               "the first list and extends it by the given attributes in "
               "the second list",
               "query cities feed "
               "cprojectextend[;Dist: distance(.Location, here)] consume");

/*
for type mapping  we expect a ListExpr with three arguments: a stream, 
a projection list and an extension list. to get the column information
of for the stream we can use ~IsBlockStream~ implemented in the CRel algebra.
Then we can determine the indices for the projected columns. Then we evaluate
the extension list and retrieve the names for new columns and their types.
The return type is a new stream with the projected and extended columns.
We append the list for column indices as we are going to need them during
value mapping.

*/


ListExpr ExtendProjectOperator::TypeMapping(ListExpr args)
{
  auto predicateError = [](int i, const char *s0, const char *s1) -> ListExpr 
  {
    return GetTypeError(2, (std::string(s0) + std::to_string(i)) + s1);
  };
  
  if (!nl->HasLength(args, 3))
    return GetTypeError("Three arguments expected.");
    
  ListExpr tupleContainerArg = nl->First(args);
  ListExpr projectionListContainerArg = nl->Second(args);
  ListExpr extensionListContainerArg = nl->Third(args);

  if (!nl->HasLength(tupleContainerArg, 2))
    return GetTypeError(0, "Stream Operand does not have the expected form.");

  if (!nl->HasLength(projectionListContainerArg, 2))
    return GetTypeError(1, "Projection List does not have the expected form.");

  if (!nl->HasLength(extensionListContainerArg, 2))
    return GetTypeError(2, "Extension List does not have the expected form.");
    
  ListExpr tupleArg = nl->First(tupleContainerArg);
  ListExpr extensionListArg = nl->First(extensionListContainerArg);
  ListExpr projectionListArg = nl->First(projectionListContainerArg);
  
  if (nl->IsAtom(projectionListArg))
    return GetTypeError(1, "Projection List isn't a list!");

  if (nl->IsAtom(extensionListArg))
    return GetTypeError(2, "Extension List isn't a list!");

  string error;
  ListExpr blockType;
  if (!IsBlockStream(tupleArg, blockType, error))
    return GetTypeError(0, "block stream", error);

  TBlockTI oldBlockInfo = TBlockTI(blockType, false);
  
  std::set<std::string> columnNames;
  std::list<uint64_t> indices;

  while (!nl->IsEmpty(projectionListArg)) {
    const ListExpr projectionArg = nl->First(projectionListArg);
    projectionListArg = nl->Rest(projectionListArg);

    if (!nl->IsNodeType(SymbolType, projectionArg))
      return GetTypeError(1, "Column name isn't a symbol!");

    const string columnNameString = nl->SymbolValue(projectionArg);

    if (!columnNames.insert(columnNameString).second)
      return GetTypeError(1, "List of column names contains duplicate value!");

    uint64_t index = 0;

    for (const auto &columnInfo : oldBlockInfo.columnInfos) {
      if (columnInfo.name == columnNameString)
        break;
        
      index++;
    }

    if (index == oldBlockInfo.columnInfos.size())
      return GetTypeError(1, "projection column name unknown!");

    indices.push_back(index);
  }

  ListExpr newColumns = nl->Empty(), newColumnsEnd;
  
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
        
    const std::string columnNameString = nl->SymbolValue(columnName);
    if (!columnNames.insert(columnNameString).second)
      return GetTypeError(1, "List of column names contains duplicate value!");
    
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
  
  TBlockTI blockInfo = TBlockTI(blockType, false);
  blockInfo.columnInfos.clear();
  
  const ListExpr indicesList = nl->OneElemList(nl->IntAtom(indices.size()));
  ListExpr indicesListEnd = indicesList;

  for (const uint64_t index : indices) {
    blockInfo.columnInfos.push_back(oldBlockInfo.columnInfos[index]);
    indicesListEnd = nl->Append(indicesListEnd, nl->IntAtom(index));
  }
  
  blockInfo.AppendColumnInfos(newColumns);

  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           indicesList, blockInfo.GetTypeExpr(true));
}

/*
CreateState is called when a new stream is created.

*/

typename ExtendProjectOperator::State *ExtendProjectOperator::CreateState(
  ArgVector args, Supplier s)
{
  qp->DeleteResultStorage(s);
  const TBlockTI blockType = TBlockTI(qp->GetType(s), false);

  return new State(args, blockType);
}

/*
1.2 Implementation of the class ExtendProjectOperator::State

the constructor collects the indices for the projected columns and opens
the stream.

*/

ExtendProjectOperator::State::State(ArgVector args, const TBlockTI &blockType) :
  m_stream(args[0].addr),
  m_projectionIndices(((CcInt*)args[3].addr)->GetValue()),
  m_extendParameter(args[2].addr)
{
  // initialize projection indices
  const uint64_t projectionIndexCount = m_projectionIndices.GetCapacity();

  for (uint64_t i = 0; i < projectionIndexCount; ++i) {
    m_projectionIndices[i] = ((CcInt*)args[i + 4].addr)->GetValue();
  }

  // initialize extension column types
  ListExpr extensionTypes = nl->Empty(),
    extensionTypesEnd = extensionTypes;

  const PTBlockInfo &blockInfo = blockType.GetBlockInfo();

  uint64_t columnCount = 
    projectionIndexCount + qp->GetNoSons(m_extendParameter);

  for (uint64_t i = projectionIndexCount; i < columnCount; ++i)
  {
    ListExpr columnType = blockInfo->columnTypes[i];

    if (nl->IsEmpty(extensionTypes))
    {
      extensionTypes = nl->OneElemList(columnType);
      extensionTypesEnd = extensionTypes;
    }
    else
    {
      extensionTypesEnd = nl->Append(extensionTypesEnd, columnType);
    }
  }

  m_extensionTypes = extensionTypes;

  // open source stream
  m_stream.open();
}

/*
destruktor closes stream.

*/

ExtendProjectOperator::State::~State()
{
  m_stream.close();
}

/*
~Request~ is called by ~StreamValueMapping~ when it receives a request on
an open stream. 

*/

TBlock *ExtendProjectOperator::State::Request()
{
  TBlock *sourceBlock = m_stream.request();
  
  if (sourceBlock == nullptr)
    return nullptr;
    
  size_t extendColumnCount = qp->GetNoSons(m_extendParameter);

  AttrArray *extendColumns[extendColumnCount];

/*
call the mapping functions of the extension list to create the
attribut arrays for the new columns.

*/
  for (size_t c = 0; c < extendColumnCount; ++c) {
    Supplier extension = qp->GetSupplier(m_extendParameter, c);
    Supplier function = qp->GetSupplier(extension, 1);
    (*qp->Argument(function))[0].setAddr(sourceBlock);

    Word newAttrArray;
    qp->Request(function, newAttrArray);
    extendColumns[c] = static_cast<AttrArray*>(newAttrArray.addr);
  }

/*
create and return the projected and extended block

*/
  TBlock *block = new TBlock(*sourceBlock, m_projectionIndices.GetPointer(), 
    m_projectionIndices.GetCapacity(), extendColumns, extendColumnCount,
    m_extensionTypes);

  return block;
}