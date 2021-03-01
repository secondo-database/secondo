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

#ifndef _RESULT_ITERATOR_GENERIC_H_
#define _RESULT_ITERATOR_GENERIC_H_

#include "Attribute.h"
#include "NestedList.h"
#include "StandardTypes.h"
#include "Algebra.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

#include <vector>
#include <string>
#include <boost/log/trivial.hpp>

namespace BasicEngine {


/*
1 Class ~ResultIteratorGeneric~

A database independend iterator for query results

*/
   class ResultIteratorGeneric {

/*
  1.1 Public Methods

*/
    public:
    
    ResultIteratorGeneric(ListExpr &type) {

      ListExpr numType = nl->Second(
          SecondoSystem::GetCatalog()->NumericType(type));

      tt = new TupleType(numType);
      basicTuple = new Tuple(tt);

      // build attributeInstances for each type
      ListExpr attrList = nl->Second(nl->Second(type));
      while(!nl->IsEmpty(attrList)){

          ListExpr attrType = nl->Second(nl->First(attrList));
          attrList = nl->Rest(attrList);
          int algId;
          int typeId;
          std::string tname;

          if(! ((SecondoSystem::GetCatalog())->LookUpTypeExpr(attrType,
                                              tname, algId, typeId))) {
              
            BOOST_LOG_TRIVIAL(error) << "Type: " 
                << nl->ToString(attrType)
                << "Unable to find attribute in catalog";
              
              ready = false;
          }

          Word w = am->CreateObj(algId,typeId)(attrType);
          attributeInstances.push_back(static_cast<Attribute*>(w.addr));
        }
    }

    virtual ~ResultIteratorGeneric() {

        if(tt != nullptr) {
            tt -> DeleteIfAllowed();
            tt = nullptr;
        }

        if(basicTuple != nullptr) {
            basicTuple -> DeleteIfAllowed();
            basicTuple = nullptr;
        }

        for(size_t i=0; i<attributeInstances.size(); i++){
            delete attributeInstances[i];
        }

        attributeInstances.clear();
    }

/*
  1.1.1 Returns whether or not a tuple is available

*/
    virtual bool hasNextTuple() = 0;

/*
  1.1.2 Get the next tuple

*/
    virtual Tuple* getNextTuple() = 0;


  protected:
    bool ready = true;
    ListExpr type;
    TupleType* tt = nullptr;
    Tuple* basicTuple = nullptr;
    std::vector<Attribute*> attributeInstances;

  private:
        
   }; // Class

}; // Namespace

#endif