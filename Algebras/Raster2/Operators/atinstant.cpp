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

#include "atinstant.h"

#include "../msint.h"
#include "../msreal.h"
#include "../msbool.h"
#include "../msstring.h"
#include "../isint.h"
#include "../isreal.h"
#include "../isbool.h"
#include "../isstring.h"

namespace raster2 {
  
    int atinstantFunString(Word*, Word&, int, Word&, Supplier);

    ValueMapping atinstantFuns[] = {
        atinstantFun<int, mstype_helper<int>, istype_helper<int> >,
        atinstantFun<double, mstype_helper<double>, istype_helper<double> >,
        atinstantFun<char, msbool_helper, istype_helper<char> >,
        atinstantFunString,
        0
    };

    int atinstantSelectFun(ListExpr args) 
    {
        NList type(args);

        // The selection function should not have been called if the second
        // argument does not meet the criteria in the type mapping
        assert(type.second().isSymbol(DateTime::BasicType()));

        if (type.first().isSymbol(msint::BasicType())) {
            return 0;
        }
        else if (type.first().isSymbol(msreal::BasicType())) {
            return 1;
        }
        else if (type.first().isSymbol(msbool::BasicType())) {
            return 2;
        }
        else if(type.first().isSymbol(msstring::BasicType())) {
            return 3;
        }
        
        return -1;
    }

    ListExpr atinstantTypeMap(ListExpr args)
    {
        NList types(args);

        std::ostringstream error;

        try {
            if (types.length() != 2) {
                error << "Expected 2 arguments, got " << types.length() << ".";
                throw util::parse_error(error.str());
            }
            
            if (types.second().first() != NList(Instant::BasicType())) {
                error << "Expected " << Instant::BasicType()
                      << " as argument 2, got " << types.second().first().str()
                      << ".";
                throw util::parse_error(error.str());
            }

            bool ok;
            Word result;
            DateTime* value;
            
            ok = QueryProcessor::ExecuteQuery
                        (types.elem(2).second().convertToString(), result);

            if (ok) 
            {
                value = static_cast<DateTime*>(result.addr);
                if (!value->IsDefined()) 
                {
                    error << "Argument 2 cannot be undefined.";
                    throw util::parse_error(error.str());
                }
            } 
            else 
            {
                error << "Argument 2 cannot be evaluated.";
                throw util::parse_error(error.str());
            }

            if (types.second().first() == NList(DateTime::BasicType())) 
            {
                if(types.first().first() == NList(msint::BasicType())) 
                {
                    return NList(isint::BasicType()).listExpr();
                }
                else if(types.first().first() == NList(msreal::BasicType())) 
                {
                    return NList(isreal::BasicType()).listExpr();
                }
                else if(types.first().first() == NList(msbool::BasicType())) 
                {
                    return NList(isbool::BasicType()).listExpr();
                }
                else if(types.first().first() == NList(msstring::BasicType())) 
                {
                   return NList(isstring::BasicType()).listExpr();
                }
                else
                {
                    error << "Expected msType as argument 1, "
                        "got " << types.first().first().str() << ".";
                    throw util::parse_error(error.str());
                }
            }
        } 
        catch (util::parse_error& e) 
        {
            return NList::typeError(e.what());
        }
        
        return 0;       
    }
    
    int atinstantFunString
    (Word* args, Word& result, int message, Word& local, Supplier s)
    {
      msstring* msin = static_cast<msstring*>(args[0].addr);

      DateTime* instant = static_cast<DateTime*>(args[1].addr);

      if( msin != 0 && instant != 0 )
      {
        result = qp->ResultStorage(s);

        isstring* pResult = static_cast<isstring*>(result.addr);

        if( pResult != 0 )
        {    
          msin->setCacheSize(16);
  
          grid3 grid = msin->getGrid();
    
          grid2 copy(grid.getOriginX(),
                   grid.getOriginY(),
                   grid.getLength());
    
          double i = instant->ToDouble();
    
          sstring* values = new sstring();
          values->setGrid(copy);
     
          Rectangle<3> bbox = msin->bbox();

          RasterIndex<3> msfrom = grid.getIndex(bbox.MinD(0), bbox.MinD(1), i);
          RasterIndex<3> msto = grid.getIndex(bbox.MaxD(0), bbox.MaxD(1), i);

          RasterIndex<2> from = 
             sstring::storage_type::getRegion((int[]){msfrom[0], msfrom[1]});
          RasterIndex<2> to = ((int[]){msto[0], msto[1]});
          RasterIndex<2> region_size = values->begin_regions().region_size;
    
          RasterIndex<2> current = from;
    
          while (current <= to) 
          {
            for (RasterIndex<2> index = current, e = current + region_size; 
                                index < e; index.increment(current, e))
            {
              msstring::index_type i2 = 
                                 (int[]){index[0], index[1], msfrom[2]};
              values->set(index, msin->get(i2));
            }

            current[0] += region_size[0];
       
            if (current[0] > to[0]) 
            {
              current[1] += region_size[1];

              if (current[1] < to[1]) 
              {
                current[0] = from[0];
              }
            }
          }
 
          pResult->setInstant(new DateTime(*instant));
          pResult->setValues(values);
      }
    }

    return 0;
  }
}
