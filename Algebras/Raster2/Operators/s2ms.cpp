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
#include "../util/parse_error.h"

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
        NList type(args);

        if (type.first().isSymbol(sint::BasicType())) {
            return 0;
        } else if (type.first().isSymbol(sreal::BasicType())) {
            return 1;
        } else if (type.first().isSymbol(sbool::BasicType())) {
            return 2;
        } else if (type.first().isSymbol(sstring::BasicType())) {
            return 3;
        } else {
            return 4;
        }
    }

    ListExpr s2msTypeMap(ListExpr args) {
        NList nlist(args);

        std::ostringstream error;

        try {
            if (nlist.length() != 4) {
                error << "Expected 4 arguments, got " << nlist.length() << ".";
                throw util::parse_error(error.str());
            }
            if (nlist.second().first() != NList(Duration::BasicType())) {
                error << "Expected " << Duration::BasicType()
                      << " as argument 2, got " << nlist.second().first().str()
                      << ".";
                throw util::parse_error(error.str());
            }
            if (nlist.third().first() != NList(Instant::BasicType())) {
                error << "Expected " << Instant::BasicType()
                      << " as argument 3, got " << nlist.third().first().str()
                      << ".";
                throw util::parse_error(error.str());
            }
            if (nlist.fourth().first() != NList(Instant::BasicType())) {
                error << "Expected " << Instant::BasicType()
                      << " as argument 4, got " << nlist.fourth().first().str()
                      << ".";
                throw util::parse_error(error.str());
            }

            bool ok;
            Word result;
            DateTime* value;
            for (Cardinal i = 2; i <= 4; ++i) {
                ok = QueryProcessor::ExecuteQuery
                        (nlist.elem(i).second().convertToString(), result);
                if (ok) {
                    value = static_cast<DateTime*>(result.addr);
                    if (!value->IsDefined()) {
                        delete value;
                        error << "Argument " << i << " cannot be undefined.";
                        throw util::parse_error(error.str());
                    }
                    delete value;
                } else {
                    if(value){
                      delete value;
                    }   
                    error << "Argument " << i << " cannot be evaluated.";
                    throw util::parse_error(error.str());
                }
                value = 0;
            }

            if(nlist.first().first() == NList(sint::BasicType())) {
              return NList(msint::BasicType()).listExpr();
            } else if(nlist.first().first() == NList(sreal::BasicType())) {
              return NList(msreal::BasicType()).listExpr();
            } else if(nlist.first().first() == NList(sbool::BasicType())) {
              return NList(msbool::BasicType()).listExpr();
            } else if(nlist.first().first() == NList(sstring::BasicType())) {
              return NList(msstring::BasicType()).listExpr();
            } else {
                error << "Expected sType as argument 1, "
                        "got " << nlist.first().first().str() << ".";
                throw util::parse_error(error.str());
            }
        } catch (util::parse_error& e) {
            return NList::typeError(e.what());
        }

        return 0;
    }
}

