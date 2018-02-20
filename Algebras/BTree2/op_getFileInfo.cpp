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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of the getFileInfo Operator

[TOC]

0 Overview


1 Defines and Includes

*/

#include "op_getFileInfo.h"

#include "ListUtils.h"
#include "QueryProcessor.h"

#include "BTree2.h"
#include "Algebras/FText/FTextAlgebra.h"


extern NestedList* nl;
extern QueryProcessor *qp;


using namespace std;

namespace BTree2Algebra {
namespace Operators {

/*
2 Operator ~getFileInfo~

This operator returns information on the file used to store the btree2.

Signature is

----
    getFileInfo: (btree2) --> (text)
----

2.1 TypeMapping for Operator ~getFileInfo~

*/

ListExpr getFileInfo::TypeMapping( ListExpr args){
  if(nl->ListLength(args) != 1){
    return listutils::typeError("Operator expects exactly one argument");
  }
  if(!listutils::isBTree2Description(nl->First(args))){
    return listutils::typeError("Operator expects a btree2 "
                               "object as argument.");
  }
  return (nl->SymbolAtom(FText::BasicType()));
}

/*
2.2 ValueMapping for Operator ~getFileInfo~

*/
int
getFileInfo::ValueMapping(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  FText* restext = (FText*)(result.addr);
  BTree2* btree = (BTree2*)(args[0].addr);
  SmiStatResultType resVector(0);
  bool atLeastOneSuccess = false;

  if (btree != 0) {
    string resString = "[[\n";
    for (int i = 0; i < 4; i++) {
      BTree2::FileEnum e = (BTree2::FileEnum) i;  // Ok, not very clean...
      if (btree->GetFileStats(e,resVector)) {
        if (atLeastOneSuccess) {
          resString += "],\n[\n";
        }
        atLeastOneSuccess = true;
        for(SmiStatResultType::iterator i = resVector.begin();
            i != resVector.end(); ){
          resString += "\t[['" + i->first + "'],['" + i->second + "']]";
          if(++i != resVector.end()){
            resString += ",\n";
          } else {
            resString += "\n";
          }
        }
      }
    }
    resString += "]]";
    if (atLeastOneSuccess) {
      restext->Set(true,resString);
    } else {
      restext->Set(false,"");
    }
  }
  return 0;
}

/*
2.3 Specification of Operator

*/
struct getFileInfoInfo : OperatorInfo {

  getFileInfoInfo() : OperatorInfo()
  {
    name =      "getFileInfo";
    signature = "(btree2 Tk Td u) -> text";
    syntax =    "getFileInfo ( _ )";
    meaning =   "Returns file statistics of the tree.";
    example =   "query getFileInfo (staedte_btree2)";
  }
};

Operator getFileInfo::def (getFileInfoInfo(), ValueMapping, TypeMapping);
}
}

