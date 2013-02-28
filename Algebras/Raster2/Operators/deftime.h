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

#ifndef RASTER2_DEFTIME_H
#define RASTER2_DEFTIME_H

#include <NList.h>

#include "../msbool.h"
#include "../msreal.h"
#include "../msint.h"
#include "../msstring.h"
#include "TemporalAlgebra.h"

namespace raster2 {
    extern ValueMapping deftimeFuns[];
    ListExpr deftimeTypeMap(ListExpr args);
    int deftimeSelectFun(ListExpr args);

    template <typename MSType>
    int deftimeFun
        (Word* args, Word& result, int message, Word& local, Supplier s)
    {
        result = qp->ResultStorage(s);
        Periods* pResult = static_cast<Periods*>(result.addr);
        MSType* ms = static_cast<MSType*>(args[0].addr);
        if(ms->isDefined()){
             pResult->Clear();
             ms->getDefinedPeriods(*pResult);
        } else {
           pResult->SetDefined(false);
        }

        return 0;
    }

    struct deftimeInfo : OperatorInfo
    {
        deftimeInfo()
        {
            name      = "deftime";
            signature = msbool::BasicType() + " deftime -> "
                + Periods::BasicType();
            appendSignature(msreal::BasicType() + " deftime -> "
                + Periods::BasicType());
            appendSignature(msint::BasicType() + " deftime -> "
                + Periods::BasicType());
            appendSignature(msstring::BasicType() + " deftime -> "
                + Periods::BasicType());
            syntax    = "deftime(_)";
            meaning   = "returns defined periods";
        }
    };

}

#endif /* #define RASTER2_DEFTIME_H */

