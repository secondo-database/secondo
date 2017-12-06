/*

1.1.1 Class Implementation

----
This file is part of SECONDO.

Copyright (C) 2017,
Faculty of Mathematics and Computer Science,
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
----

*/
#include "FileSystem.h"
#include "NestedList.h"
#include "StandardTypes.h"

#include "Algebras/DBService/DBServiceClient.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/OperatorDDeleteDB.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"

using namespace std;

namespace DBService
{

ListExpr OperatorDDeleteDB::mapType(ListExpr nestedList)
{
   if(nl->IsEmpty(nestedList)){
      return listutils::basicSymbol<CcBool>();
   }
   if(!nl->HasLength(nestedList,1)){
      return listutils::typeError("no argument or string expected");
   }
   if(!CcString::checkType(nl->First(nestedList))){
      return listutils::typeError("no argument or string expected");
   }
   return listutils::basicSymbol<CcBool>();
}

int OperatorDDeleteDB::mapValue(Word* args,
                              Word& result,
                              int message,
                              Word& local,
                              Supplier s)
{
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   if(qp->GetNoSons(s)==1){
     CcString* db = (CcString*) args[0].addr;
     if(!db->IsDefined()){
        res->SetDefined(false);
        return 0;
     }
     dbname = db->GetValue();
     stringutils::toUpper(dbname);
   }


    bool success =
            DBServiceClient::getInstance()->deleteReplicas(
                    dbname,
                    "",
                    "");
    res->Set(true,success);
    return 0;
}

} /* namespace DBService */
