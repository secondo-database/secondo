/*
----
This file is part of SECONDO.

Copyright (C) 2021,
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

//[$][\$]

*/
#ifndef _RESULT_ITERATOR_POSTGRES_H_
#define _RESULT_ITERATOR_POSTGRES_H_

#include "ResultIteratorGeneric.h"

#include "Attribute.h"
#include "NestedList.h"
#include "StandardTypes.h"
#include "Algebra.h"

#include <vector>
#include <string>
#include <postgres.h>
#include <libpq-fe.h>
#include <catalog/pg_type.h>


namespace BasicEngine {
   class ResultIteratorPostgres : public ResultIteratorGeneric {
       public:

        ResultIteratorPostgres(PGresult* res, ListExpr type) 
           : res(res), type(type) {
            
            ListExpr numType = nl->Second(
                SecondoSystem::GetCatalog()->NumericType(type));

            tt = new TupleType(numType);
            basicTuple = new Tuple(tt);

            // build instances for each type
            ListExpr attrList = nl->Second(nl->Second(type));
            while(!nl->IsEmpty(attrList)){

                ListExpr attrType = nl->Second(nl->First(attrList));
                attrList = nl->Rest(attrList);
                int algId;
                int typeId;
                std::string tname;

                if(! ((SecondoSystem::GetCatalog())->LookUpTypeExpr(attrType,
                                                    tname, algId, typeId))) {
                    
                    std::cerr << "Type: " << nl->ToString(attrType) << endl;

                    std::cerr << "Error: Unable to find attribute in catalog" 
                         << endl;
                
                    totalTuples = 0;
                }

                Word w = am->CreateObj(algId,typeId)(attrType);
                instances.push_back(static_cast<Attribute*>(w.addr));
            }

            totalTuples = PQntuples(res);
            // cout << "Total Tuples are " << totalTuples << endl;
        }

        virtual ~ResultIteratorPostgres() {
            if(tt != NULL) {
                tt -> DeleteIfAllowed();
                tt = NULL;
            }

            if(basicTuple != NULL) {
                basicTuple -> DeleteIfAllowed();
                basicTuple = NULL;
            }

            if(res != NULL) {
                PQclear(res);
                res = NULL;
            }

            for(unsigned int i=0; i<instances.size();i++){
                delete instances[i];
            }

            instances.clear();
        }

        virtual bool hasNextTuple();
        virtual Tuple* getNextTuple();

       private:
        PGresult* res = NULL;
        ListExpr type;
        TupleType* tt = NULL;
        Tuple* basicTuple = NULL;
        std::vector<Attribute*> instances;
        int currentTuple = 0;
        int totalTuples = 0;
   };
}

#endif