
/*

----
This file is part of SECONDO.

Copyright (C) 2017, 
University in Hagen, 
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


//[_] [\_]

*/
#ifndef MGRAPH3_H
#define MGRAPH3_H


#include "MainMemoryExt.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "NestedList.h"
#include "SecondoCatalog.h"
#include "SecondoSystem.h"
#include "MEdge.h"
#include "ListUtils.h"
#include "StandardTypes.h"

#include <string>
#include <vector>
#include <list>

#include "MGraphCommon.h" 

namespace mm2algebra{

class MGraph3 : public MGraphCommon{

   public:
     MGraph3(const bool _flob, 
            const std::string& _database,
            const std::string& _type,
            int _srcPos,
            int _targetPos,
            int _costPos,
            int size): 
            MGraphCommon(_flob,_database,_type, _srcPos, _targetPos,_costPos) {
        // generate nodes
        for(int i=0;i<size;i++){
           alist l;
           graph.push_back(l);
        }
     }

     ~MGraph3(){
      }
        

      static const std::string BasicType() { return "mgraph3"; }

      static bool checkType(ListExpr t){
          return nl->HasLength(t,2) 
                 && listutils::isSymbol(nl->First(t),BasicType())
                 && Tuple::checkType(nl->Second(t));
      }

      MemoryObject* clone(){
         return new MGraph3(*this);
      }

      static ListExpr wrapType(ListExpr tupleType){
        assert(Tuple::checkType(tupleType));
        return nl->TwoElemList( nl->SymbolAtom(BasicType()),
                                tupleType);
      }


};


} // end of namespace mm2algebra

#endif

