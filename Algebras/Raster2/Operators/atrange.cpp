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

#include "atrange.h"
#include "../sint.h"
#include "../msint.h"
#include "../sreal.h"
#include "../msreal.h"
#include "../sbool.h"
#include "../msbool.h"
#include "../sstring.h"
#include "../msstring.h"
#include "../util/parse_error.h"

namespace raster2
{
  ValueMapping atrangeFuns[] =
  {
    atrangeFun<int, stype_helper<int> >,
    atrangeFun<double, stype_helper<double> >,
    atrangeFun<char, sbool_helper>,
    atrangeFun<string, stype_helper<string> >,
    atrangeMFun<msint, int>,
    atrangeMFun<msreal, double>,
    atrangeMFun<msbool, bool>,
    atrangeMFun<msstring, string>,
    0
  };

  int atrangeSelectFun(ListExpr args)
  {
    NList type(args);

    // The selection function should not have been called if the second
    // argument does not meet the criteria in the type mapping
    assert(type.second().isSymbol(Rect::BasicType()));

    if(type.first().isSymbol(sint::BasicType()))
    {
      return 0;
    }
    
    else if(type.first().isSymbol(sreal::BasicType()))
    {
      return 1;
    }
    
    else if(type.first().isSymbol(sbool::BasicType()))
    {
      return 2;
    }
    
    else if(type.first().isSymbol(sstring::BasicType()))
    {
      return 3;
    }
    
    else if(type.first().isSymbol(msint::BasicType()))
    {
      return 4;
    }
    
    else if (type.first().isSymbol(msreal::BasicType()))
    {
      return 5;
    }
    
    else if (type.first().isSymbol(msbool::BasicType()))
    {
      return 6;
    }

    else if (type.first().isSymbol(msstring::BasicType()))
    {
      return 7;
    }

    return 8;
  }

  ListExpr atrangeTypeMap(ListExpr args)
  {
    NList nlist(args);

    std::ostringstream error;

    try {
        if ((nlist.length() != 4) && (nlist.length() != 2)) {
            error << "Expected 2 or 4 arguments, got " << nlist.length() << ".";
            throw util::parse_error(error.str());
        }
        if (nlist.second() != NList(Rect::BasicType())) {
            error << "Expected " << Rect::BasicType()
                  << " as argument 2, got " << nlist.second().str()
                  << ".";
            throw util::parse_error(error.str());
        }
        if (nlist.length() == 4) {
            if (nlist.third() != NList(Instant::BasicType())) {
                error << "Expected " << Instant::BasicType()
                      << " as argument 3, got " << nlist.third().str()
                      << ".";
                throw util::parse_error(error.str());
            }
            if (nlist.fourth() != NList(Instant::BasicType())) {
                error << "Expected " << Instant::BasicType()
                      << " as argument 4, got " << nlist.fourth().str()
                      << ".";
                throw util::parse_error(error.str());
            }
        }

        if( nlist.first() != NList(sint::BasicType()) &&
            nlist.first() != NList(sreal::BasicType()) &&
            nlist.first() != NList(sbool::BasicType()) &&
            nlist.first() != NList(sstring::BasicType()) &&
            nlist.first() != NList(msint::BasicType()) &&
            nlist.first() != NList(msreal::BasicType()) &&
            nlist.first() != NList(msbool::BasicType()) &&
            nlist.first() != NList(msstring::BasicType()) )
        {
          error << "Expected sType or msType as argument 1, "
                  "got " << nlist.first().str() << ".";
          throw util::parse_error(error.str());
        }
    }
    catch (util::parse_error& e) {
        return NList::typeError(e.what());
    }

    return nlist.first().listExpr();
  }
}
