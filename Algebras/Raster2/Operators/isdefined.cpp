/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

#include "isdefined.h"
#include "../sint.h"
#include "../sreal.h"
#include "../sbool.h"
#include "../sstring.h"
#include "../msint.h"
#include "../msreal.h"
#include "../msbool.h"
#include "../msstring.h"
#include "../isint.h"
#include "../isreal.h"
#include "../isbool.h"
#include "../isstring.h"
#include "../util/parse_error.h"
#include "../util/types.h"

#include <ListUtils.h>

namespace raster2
{
  ListExpr isdefinedTM(ListExpr args) {
     if(!nl->HasLength(args,1)){
        return listutils::typeError("one argument expected");
     }
     std::string type_str = nl->ToString(nl->First(args));
     if(    util::isSType(type_str) 
        ||  util::isMSType(type_str) 
        ||  util::isISType(type_str)){
        return listutils::basicSymbol<CcBool>();
     }
     return listutils::typeError("Rastertype expected");
  }

  int isdefinedSelectFun(ListExpr args) {
    int numBasicTypes = 4;
    string type_str = nl->ToString(nl->First(args));
    int offset = 0;
    if(util::isMSType(type_str)){
       offset = 1*numBasicTypes;
    } else if(util::isISType(type_str)){
       offset = 2*numBasicTypes;
    }

    string bt = util::getValueBasicType(type_str);
    int pos = 0;
    if(bt==CcBool::BasicType()){
       pos = 0;
    } else if(bt==CcInt::BasicType()){
       pos = 1;
    } else if(bt==CcReal::BasicType()){
       pos = 2;
    } else if(bt==CcString::BasicType()){
       pos = 3;
    }
    return offset + pos;
  }

  
  
  template <typename S>
  int isdefinedFun(Word* args, Word& result, int message, 
                   Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    CcBool* res = (CcBool*) result.addr;
    S* arg = (S*) args[0].addr;
    res->Set(true,arg->isDefined());
    return 0;
  }
  

  
  
  ValueMapping isdefinedFuns[] = {
     isdefinedFun<sbool>,
     isdefinedFun<sint>,
     isdefinedFun<sreal>,
     isdefinedFun<sstring>,
     isdefinedFun<msbool>,
     isdefinedFun<msint>,
     isdefinedFun<msreal>,
     isdefinedFun<msstring>,
     isdefinedFun<isbool>,
     isdefinedFun<isint>,
     isdefinedFun<isreal>,
     isdefinedFun<isstring>,
     0
  };
}
