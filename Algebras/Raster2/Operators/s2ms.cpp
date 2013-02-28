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

#include <sstream>

#include "../stype.h"
#include "../sint.h"
#include "../sbool.h"
#include "../sreal.h"
#include "../sstring.h"
#include "../mstype.h"
#include "../msint.h"
#include "../msbool.h"
#include "../msreal.h"
#include "../msstring.h"
#include "../util/types.h"


#include "s2ms.h"

namespace raster2 {
    ValueMapping s2msFuns[] = {
        s2msFun<sint, msint>,
        s2msFun<sreal, msreal>,
        s2msFun<sbool, msbool>,
        s2msFun<sstring, msstring>,
        0
    };

    int s2msSelectFun(ListExpr args) {
        ListExpr a1 = nl->First(args);;

        if (sint::checkType(a1)) {
            return 0;
        } else if (sreal::checkType(a1)) {
            return 1;
        } else if (sbool::checkType(a1)) {
            return 2;
        } else if (sstring::checkType(a1)) {
            return 3;
        } else { // invalid type
            return -1;
        }
    }

    ListExpr s2msTypeMap(ListExpr args) {
        if(!nl->HasLength(args,4)){
          return listutils::typeError("4 arguments expeted");
        }
        string err = "stype x duration x instant x instant expected";
        ListExpr stype = nl->First(args);
        ListExpr dur = nl->Second(args);
        ListExpr start = nl->Third(args);
        ListExpr end = nl->Fourth(args);

        if(   !util::isSType(stype) 
           || !Duration::checkType(dur) 
           || !DateTime::checkType(start)
           || !DateTime::checkType(end)){
         return listutils::typeError(err);
       }
       std::string st = nl->SymbolValue(stype);
       std::string ct = util::getValueBasicType(st);
       std::string mt = util::getMovingSpatialBasicType(ct);
       return  nl->SymbolAtom(mt); 
    }
}

