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

#include "createGrid3.h"
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

  ListExpr createGrid3TM(ListExpr args) {
     if(!nl->HasLength(args,2)){
        return listutils::typeError("2 args expected");
     }
     string err = "grid2 x {duration, real} expected";
     if(!grid2::checkType(nl->First(args) )){
        return listutils::typeError(err + " (first arg is not of type grid2");
     }
     ListExpr a2 = nl->Second(args);
     if(    !CcReal::checkType(a2) 
         && !Duration::checkType(a2)){
        return listutils::typeError(err + " (second arg not of type "
                                          "real or duration)");
     } 
     return listutils::basicSymbol<grid3>();
  }


  int createGrid3SelectFun(ListExpr args) {
     return CcReal::checkType(nl->Second(args))?0:1;
  }

 
  DateTime getDateTime(CcReal* r){
     DateTime dt(datetime::durationtype);
     dt.ReadFrom(1.0);
     if(r->IsDefined() && (r->GetValue()>0) ){
       dt.ReadFrom(r->GetValue());
     }
     return dt;
  }

  DateTime getDateTime(DateTime* dt){
     if( dt->IsDefined() && (dt->ToDouble()>0)){
        return *dt;
     }
     DateTime res(datetime::durationtype);
     res.ReadFrom(1.0);
     return res;
  }
  
  template <typename T>
  int createGrid3Fun(Word* args, Word& result, int message, 
                   Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    grid3* res = (grid3*) result.addr;
    grid2* a1 = static_cast<grid2*>(args[0].addr);
    T*     s2 = static_cast<T*>(args[1].addr);
    DateTime dt = getDateTime(s2);
    res->set(a1->getOriginX(), a1->getOriginY(),
             a1->getLength() , dt);
    return 0;
  }
  

  
  
  ValueMapping createGrid3Funs[] = {
     createGrid3Fun<CcReal>,
     createGrid3Fun<DateTime>,
     0
  };
}
