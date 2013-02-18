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

#include "map.h"
#include "../sint.h"
#include "../sreal.h"
#include "../sbool.h"
#include "../sstring.h"
#include "../msint.h"
#include "../msreal.h"
#include "../msbool.h"
#include "../msstring.h"
#include "../util/parse_error.h"
#include "../util/types.h"

#include <ListUtils.h>

namespace raster2
{
  ListExpr mapTypeMap(ListExpr args) {


    if(!nl->HasLength(args,2)){
      return listutils::typeError("two arguments expected");
    }
    ListExpr rastertype = nl->First(args);
    ListExpr fun = nl->Second(args);

    std::string rastertype_str = nl->ToString(rastertype);

    if(!util::isMSType(rastertype_str) 
        && !util::isSType(rastertype_str)){
      return listutils::typeError("First arg must be a raster type");
    }
    if(!listutils::isMap<1>(fun)){
       return listutils::typeError("Second arg must be a function "
                                   "with 1 argument");
    }

    std::string rasterval = util::getValueBasicType(rastertype_str);
    std::string funarg = nl->SymbolValue(nl->Second(fun));
    if(rasterval!=funarg){
       return listutils::typeError("function arg does not fit the "
                                   "raster type");
    }
    std::string funres = nl->ToString(nl->Third(fun));
    std::string res_str;
    if(util::isSType(rastertype_str)){
      res_str = util::getSpatialBasicType(funres); 
    } else {
      res_str = util::getMovingSpatialBasicType(funres);
    }
    if(res_str.empty()){
       return listutils::typeError("function result is not a raster "
                                   "value type");
    }
    return nl->SymbolAtom(res_str);

  }

  int mapSelectFun(ListExpr args) {
    int offset = 0;
    int i;

    NList types(args);
    std::string first = types.first().str();
    std::string result = types.second().third().str();

    std::string idf = first.substr(first.length()-2, 2);
    std::string idr = result.substr(result.length()-2, 2);

    if (idf == "nt")      i = 0;
    else if (idf == "ol") i = 1;
    else if (idf == "al") i = 2;
    else                  i = 3;
    offset += util::isSType(first) ?  4 * i : 4 * i + 16;

    if (idr == "nt")      i = 0;
    else if (idr == "ol") i = 1;
    else if (idr == "al") i = 2;
    else                  i = 3;
    offset += i;

    return offset;
  }

  
  
  template <typename S, typename Result>
  int mapFunSS(Word* args, Word& result, int message, Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);

    S* sArg = static_cast<S*>(args[0].addr);
    Address function = args[1].addr;
    Result* r = static_cast<Result*>(result.addr);

    grid2 grid = sArg->getGrid();
    
    // If one value is undefined, the mapped value is undefined. This means we
    // only need to consider the intersection of the two bounding boxes.
    BBox<2> bb = sArg->bbox();

    // If the intersection of the bboxes is empty, we are done.
    if (bb.IsDefined()) {
      const typename S::index_type& region_size = S::riter_type::region_size;

      r->setGrid(grid);
      typename S::index_type from = grid.getIndex(bb.MinD(0), bb.MinD(1));
      typename S::index_type to   = grid.getIndex(bb.MaxD(0), bb.MaxD(1));

      // Some magic that I do not understand.
      ArgVector& arguments = *qp->Argument(function);
      Word result;

      // Iterate over regions in the first argument.
      for (typename S::riter_type rit = sArg->iterate_regions(from, to),
                                    re = sArg->end_regions();
           rit != re; ++rit)
      {
        typename S::index_type rfrom = *rit;
        typename S::index_type rto   = *rit + region_size;

        // Iterate over the cells in the current region.
        for (typename S::index_type i1 = rfrom, e = rto;
             i1 != e; i1.increment(rfrom, rto))
        { 
          if (!S::isUndefined(sArg->get(i1)))
          {
            typename S::wrapper_type w1 = S::wrap(sArg->get(i1));

            // Some more magic
            arguments[0].setAddr(&w1);
            qp->Request(function, result);

            typename Result::wrapper_type& mapped =
                *static_cast<typename Result::wrapper_type*>(result.addr);
            r->set(i1, Result::unwrap(mapped));
          }
        }
      }
    }

    return 0;
  }
  
  template <typename M, typename Result>
  int mapFunMM(Word* args, Word& result, int message, Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    
    M* mArg = static_cast<M*>(args[0].addr);
    Address function = args[1].addr;
    Result* r = static_cast<Result*>(result.addr);

    grid3 grid = mArg->getGrid();
    
    // If one value is undefined, the mapped value is undefined. This means we
    // only need to consider the intersection of the two bounding boxes.
    BBox<3> bb = mArg->bbox();

    // If the intersection of the bboxes is empty, we are done.
    if (bb.IsDefined()) {
      const typename M::index_type& region_size = M::riter_type::region_size;

      r->setGrid(grid);
      typename M::index_type from = grid.getIndex(bb.MinD(0), bb.MinD(1), 
              bb.MinD(2));
      typename M::index_type to   = grid.getIndex(bb.MaxD(0), bb.MaxD(1), 
              bb.MinD(2));

      // Some magic that I do not understand.
      ArgVector& arguments = *qp->Argument(function);
      Word result;

      // Iterate over regions in the first argument.
      for (typename M::riter_type rit = mArg->iterate_regions(from, to),
                                    re = mArg->end_regions();
           rit != re; ++rit)
      {
        typename M::index_type rfrom = *rit;
        typename M::index_type rto   = *rit + region_size;

        // Iterate over the cells in the current region.
        for (typename M::index_type i1 = rfrom, e = rto;
             i1 != e; i1.increment(rfrom, rto))
        { 
          if (!M::isUndefined(mArg->get(i1)))
          {
            typename M::wrapper_type w = M::wrap(mArg->get(i1));

            // Some more magic
            arguments[0].setAddr(&w);
            qp->Request(function, result);

            typename Result::wrapper_type & mapped =
               *static_cast<typename Result::wrapper_type*>(result.addr);

            r->set(i1, Result::unwrap(mapped));
          }
        }
      }
    }
    
    return 0;
  }


  
  
  ValueMapping mapFuns[] = {
      mapFunSS<sint, sint>,
      mapFunSS<sint, sbool>,
      mapFunSS<sint, sreal>,
      mapFunSS<sint, sstring>,

      mapFunSS<sbool, sint>,
      mapFunSS<sbool, sbool>,
      mapFunSS<sbool, sreal>,
      mapFunSS<sbool, sstring>,
      
      mapFunSS<sreal, sint>,
      mapFunSS<sreal, sbool>,
      mapFunSS<sreal, sreal>,
      mapFunSS<sreal, sstring>,

      mapFunSS<sstring, sint>,
      mapFunSS<sstring, sbool>,
      mapFunSS<sstring, sreal>,
      mapFunSS<sstring, sstring>,
      
      mapFunMM<msint, msint>,
      mapFunMM<msint, msbool>,
      mapFunMM<msint, msreal>,
      mapFunMM<msint, msstring >,
     
      mapFunMM<msbool, msint>,
      mapFunMM<msbool, msbool>,
      mapFunMM<msbool, msreal>,
      mapFunMM<msbool, msstring>,

      mapFunMM<msreal, msint>,
      mapFunMM<msreal, msbool>,
      mapFunMM<msreal, msreal>,
      mapFunMM<msreal, msstring>,

      mapFunMM<msstring, msint>,
      mapFunMM<msstring, msbool>,
      mapFunMM<msstring, msreal>,
      mapFunMM<msstring, msstring>,
      0
  };
}
