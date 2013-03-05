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

#include "addLayer.h"
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

/*
1 Type Mapping

The sigmature is:

msT x sT x instant -> bool

*/
  ListExpr addLayerTM(ListExpr args) {
    if(!nl->HasLength(args,3)){
      return listutils::typeError("3 arguments expected");
    }
    ListExpr a1 = nl->First(args);
    ListExpr a2 = nl->Second(args);
    ListExpr a3 = nl->Third(args);

    string err = "msT x sT x instant expected";
    if(!util::isMSType(a1)  ||
       !util::isSType(a2) ||
       !DateTime::checkType(a3)){
       return listutils::typeError(err);
    }
    // check whether msT and sT are compatible
    string bt1 = util::getValueBasicType(nl->ToString(a1));
    string bt2 = util::getValueBasicType(nl->ToString(a2));
    if(bt1!=bt2){
      return listutils::typeError("incompatible raster types");
    }
    if(bt1=="string"){
      return listutils::typeError("addLayer not implemented for msstring");
    }
    return listutils::basicSymbol<CcBool>();
  }

  int addLayerSelectFun(ListExpr args){
    ListExpr a1 = nl->First(args);
    if(msint::checkType(a1)){
       return 0;
    }
    if(msreal::checkType(a1)){
      return 1;
    }
    if(msbool::checkType(a1)){
      return 2;
    }
    //if(msstring::checkType(a1)){
    //  return 3;
    //}
    return -1; // should never occur
  }

  
  
  template <typename M>
  int addLayerFun(Word* args, Word& result, int message, 
                   Word& local, Supplier s)
  {
    typedef typename M::spatial_type S;
    
    M* mraster = static_cast<M*>(args[0].addr);
    S* raster = static_cast<S*>(args[1].addr);
    DateTime* instant = static_cast<DateTime*>(args[2].addr);
    result = qp->ResultStorage(s);
    CcBool* res = static_cast<CcBool*>(result.addr);

    if(!raster->isDefined() || !instant->IsDefined()){
       cout << "raster or instant undefined" << endl;
       res->Set(true,false);
       return 0;
    }

    if(!mraster->isDefined()){
      mraster->clear();
    }

    grid3 mg = mraster->getGrid();
    grid2 g = raster->getGrid();
    
    if(!mg.project().matches(g)){
      // incompatible grids
      cout << "grids incompatible" << endl;
      res->Set(true,false);
      return 0;
    }
    
    qp->SetModified(qp->GetSon(s,0));

    typename M::index_type cell = mg.getIndex(g.getOriginX(), 
                                              g.getOriginY(), 
                                              instant->ToDouble());

    int timecell = cell[2];

    size_t maxMem = qp->GetMemorySize(s)*1024*1024;

    size_t numCacheItems = maxMem / (WinUnix::getPageSize() + 8*sizeof(size_t));

    size_t nc1 = numCacheItems / 4;
    size_t nc2 = numCacheItems - nc1;

    if(nc1 < 10){
       nc1 = 10;
    }
    if(nc2 < 10){
       nc2 = 10;
    }
    raster->setCacheSize(nc1);
    mraster->setCacheSize(nc2);
    
    // iterate over contained cells
    typedef typename S::storage_type storage_type;

    typedef typename S::cell_type cell_type;

    storage_type& storage = raster->getStorage();
    for(typename S::iter_type it = storage.begin(), e = storage.end();
                     it!=e; ++it){
       cell_type element = *it;
       typename S::index_type index2 = it.getIndex();
       // compute corresponding index in mstype
       typename M::index_type index3((int[]) {index2[0]+cell[0],
                                     index2[1]+cell[1],
                                     timecell});

       mraster->set(index3,element);
    }
    res->Set(true,true);

    return 0;
  }
  

  
  
  ValueMapping addLayerFuns[] = {
     addLayerFun<msint>,
     addLayerFun<msreal>,
     addLayerFun<msbool>,
     0
  };
}
