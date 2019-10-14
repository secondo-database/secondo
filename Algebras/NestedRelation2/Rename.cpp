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

#include "Include.h"

using namespace std;
using namespace nr2a;

/*
For processing the stream "Rename"[2] uses a structure to store its context.

*/
struct RenameLocalInfo :
    public Nr2aLocalInfo<LinearProgressEstimator<RenameLocalInfo> >
{
  public:
    Stream<Tuple>* inputStream;

    RenameLocalInfo();
    virtual ~RenameLocalInfo();
};

RenameLocalInfo::RenameLocalInfo()
    : inputStream(NULL)
{
}

RenameLocalInfo::~RenameLocalInfo()
{
}

Rename::~Rename()
{
}

/*
Renaming the attributes affects the type of the tuples only, but not their
value. For the renaming is done recursively a helper function is implemented
renaming the attributes of one relation, calling itself for each subrelation.

*/
/*static*/ListExpr Rename::MapType(ListExpr args)
{
  AutoWrite(args);
  if (nl->ListLength(args) != 2)
  {
    return listutils::typeError("two arguments expected");
  }
  ListExpr inStreamType = nl->First(nl->First(args));
  if (!listutils::isTupleStream(inStreamType))
  {
    return nl->TypeError();
  }
  string postfix = string("_");
  ListExpr postfixList = nl->Second(nl->Second(args));
  if (listutils::isSymbol(postfixList))
  {
    postfix += nl->SymbolValue(postfixList);
  }
  else
  {
    return listutils::typeError
        ("Second argument is expected to be a symbol");
  }

  ListExpr outStreamType = nl->TheEmptyList();
  string errorMsg = "";
  if (!RenameTypes(inStreamType, postfix, outStreamType, errorMsg))
  {
    return listutils::typeError(errorMsg);
  }
  AutoWrite(outStreamType);

  return outStreamType;
}

/*static*/ValueMapping Rename::functions[] = { RenameValue, NULL };

/*static*/int Rename::SelectFunction(ListExpr args)
{
  return 0;
}

/*static*/int Rename::RenameValue(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  int resultInt = 0;

  RenameLocalInfo *localInfo = NULL;
  switch (message)
  {
    case OPEN:
    {
      localInfo = new RenameLocalInfo();
      localInfo->inputStream = new Stream<Tuple>(args[0]);
      localInfo->inputStream->open();
      local.addr = localInfo;
    }
      break;

    case REQUEST:
    {
      Tuple *tuple = NULL;
      localInfo = static_cast<RenameLocalInfo*>(local.addr);
      localInfo->UnitReceived();
      tuple = localInfo->inputStream->request();
      if (tuple != NULL)
      {
        result.setAddr(tuple);
        resultInt = YIELD;
        localInfo->UnitProcessed();
      }
      else
      {
        resultInt = CANCEL;
      }
    }
      break;

    case CLOSE:
      localInfo = (RenameLocalInfo*) local.addr;
      delete localInfo->inputStream;
      delete localInfo;
      local.addr = NULL;
      break;
  }
  return resultInt;
}

/*static*/bool Rename::RenameTypes(const ListExpr type, const string & postfix,
    ListExpr & outType, string & errorMsg, const bool sub /*= false*/)
{
  bool result = true;
  ListBuilder outTypeBuilder;
  ListExpr attributesTypes = nl->Second(nl->Second(type));
  AutoWrite(attributesTypes);
  listForeach(attributesTypes, currentAttributeDescription)
  {
    AutoWrite(currentAttributeDescription);
    string currentName = nl->SymbolValue(
        nl->First(currentAttributeDescription));
    ListExpr currentType = nl->Second(currentAttributeDescription);
    ListExpr newType = nl->TheEmptyList();
    ListExpr currentNameList = nl->SymbolAtom(currentName + postfix);
    if (listutils::isValidAttributeName(currentNameList, errorMsg))
    {
      if ((nl->ListLength(currentType) == 2)
          && (listutils::isSymbol(nl->First(currentType), ARel::BasicType())))
      {
        ListExpr newNrelType = nl->TheEmptyList();
        if (RenameTypes(currentType, postfix, newNrelType, errorMsg, true))
        {
          currentType = newNrelType;
        }
        else
        {
          result = false;
        }
      }
      newType = nl->TwoElemList(currentNameList, currentType);
    }
    else
    {
      result = false;
    }
    outTypeBuilder.Append(newType);
  }
  if (result)
  {
    outType = sub ? outTypeBuilder.GetARel() : outTypeBuilder.GetTupleStream();
  }
  return result;
}

/*
List of functions for cost estimation.

*/
CreateCostEstimation Rename::costEstimators[] =
  { LinearProgressEstimator<RenameLocalInfo>::Build };

