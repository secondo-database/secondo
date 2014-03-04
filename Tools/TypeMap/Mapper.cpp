
/*
----
This file is part of SECONDO.

Copyright (C) 2014,
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


1 Implementation of the mapper class

*/


#include "Mapper.h"
#include "ListUtils.h"

using namespace std;

namespace typemap{

/*
1 Constructor

*/
 Mapper::Mapper(NestedList* _pnl, NestedList* _nl) :
     pnl(_pnl), nl(_nl) {
  }


 Mapper::Mapper(){
 }


/*
1.2 Destructor

*/ 
 Mapper::~Mapper(){}


/*
1.3 init

*/
   bool Mapper::init( const string& path){
     return false; // dummy implementation
   }

/*
1.4 getOpSig

*/

  ListExpr Mapper::getOpSig(const string& algebraName,
                            const string& operatorName){

    return pnl->TheEmptyList(); // dummy implementation
  }



/*
1.5 typemap

*/

  ListExpr Mapper::typemap(const ListExpr SigArgType,
                   const ListExpr CurrentArgTypes){
    return listutils::typeError();
  }



} // end of namespace typemape







