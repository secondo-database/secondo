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
#include "../util/types.h"

namespace raster2 {
  
  

  template <typename T, typename Helper, typename ResultHelper>
  int atinstantFun
    (Word* args, Word& result, int message, Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    typename ResultHelper::implementation_type* pResult =
      static_cast<typename ResultHelper::implementation_type*>(result.addr);

    typename Helper::implementation_type* mstype =
      static_cast<typename Helper::implementation_type*>(args[0].addr);
    DateTime* instant = static_cast<DateTime*>(args[1].addr);

    if(!mstype->isDefined() || !instant->IsDefined()){
        pResult->setDefined(false);
        return 0;
    }
    pResult->clear();

   size_t maxMem = qp->GetMemorySize(s)*1024*1024; // in byte
   size_t cacheItemSize = 512; 

    size_t cs = maxMem / cacheItemSize;

    if(cs<20) {
       cs = 20;
    }
    mstype->setCacheSize(cs/2);

    grid3 msgrid = mstype->getGrid();
    grid2 sgrid(msgrid.getOriginX(), msgrid.getOriginY(), msgrid.getLength());

    double i = instant->ToDouble();
    
    typename ResultHelper::spatial_type* values =
            new typename ResultHelper::spatial_type();
    values->setGrid(sgrid);

    Rectangle<3> bbox = mstype->bbox();

    RasterIndex<3> msfrom = msgrid.getIndex(bbox.MinD(0), bbox.MinD(1), i);
    RasterIndex<3> msto = msgrid.getIndex(bbox.MaxD(0), bbox.MaxD(1), i);
    
    RasterIndex<2> from = ResultHelper::spatial_type::storage_type::
                          getRegion((int[]){msfrom[0], msfrom[1]});
    RasterIndex<2> to = (int[]){msto[0], msto[1]};

    RasterIndex<2> region_size = values->begin_regions().region_size;
    
    RasterIndex<2> current = from;
    
    while (current <= to) 
    {
       for (RasterIndex<2> index = current, e = current + region_size; 
                           index < e; index.increment(current, e))
       {
         typename Helper::implementation_type::index_type i2 = 
                              (int[]){index[0], index[1], msfrom[2]};
         values->set(index, mstype->get(i2));
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

    return 0;
  }

  int atinstantFunString (Word* args, Word& result, 
                         int message, Word& local, Supplier s) {
      msstring* msin = static_cast<msstring*>(args[0].addr);

      DateTime* instant = static_cast<DateTime*>(args[1].addr);

      result = qp->ResultStorage(s);
      isstring* pResult = static_cast<isstring*>(result.addr);

      if(!msin->isDefined() || !instant->IsDefined()){
         pResult->setDefined(false);
         return 0;
      }
      pResult->clear(); 

      
      size_t maxMem = qp->GetMemorySize(s)*1024*1024; // in byte
      size_t cacheItemSize = WinUnix::getPageSize() + sizeof(size_t) * 8;

       size_t cs = maxMem / cacheItemSize;

      if(cs<20) {
         cs = 20;
      }
      msin->setCacheSize(cs/2);


      if( msin != 0 && instant != 0 )
      {

        if( pResult != 0 )
        {    
  
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



    ValueMapping atinstantFuns[] = {
        atinstantFun<int, mstype_helper<int>, istype_helper<int> >,
        atinstantFun<double, mstype_helper<double>, istype_helper<double> >,
        atinstantFun<char, msbool_helper, istype_helper<char> >,
        atinstantFunString,
        0
    };

    int atinstantSelectFun(ListExpr args) 
    {
        ListExpr arg1 = nl->First(args);

        if (msint::checkType(arg1)) {
            return 0;
        }
        else if (msreal::checkType(arg1)) {
            return 1;
        }
        else if (msbool::checkType(arg1)) {
            return 2;
        }
        else if(msstring::checkType(arg1)) {
            return 3;
        }
        
        return -1;
    }

    ListExpr atinstantTypeMap(ListExpr args)
    {
       string err = "mstype x instant expected";
       if(!nl->HasLength(args,2)){
          return listutils::typeError("2 arguments expected");
       }
       ListExpr mst = nl->First(args);
       if(!util::isMSType(mst)){
          return listutils::typeError(err + " (first arg not an mstype");
       }
       if(!DateTime::checkType(nl->Second(args))){
          return listutils::typeError(err + " (second arg not an instant");
       }
       string mstype = nl->SymbolValue(mst);
       string celltype = util::getValueBasicType(mstype);
       string itype = util::getInstantSpatialBasicType(celltype);
       return nl->SymbolAtom(itype);
    }
    
}
