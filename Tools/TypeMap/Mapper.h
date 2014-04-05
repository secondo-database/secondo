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


*/


#ifndef MAPPER_H
#define MAPPER_H

#include <string>
#include "NestedList.h"


namespace typemap{


/*
1 Class Mapper

this class represents the interface used by the Secondo system.
It should be keept as small as possible.


*/
class Mapper{

  public:

/*
1.1 Constructor

*/
     Mapper(NestedList* pnl, NestedList* nl);

/*
1.2 Destructor

*/
     ~Mapper();

/*
1.3 Function ~init~

This function parses a file containing TypeMap specifications and stores
the nested lists created from the content of the file into internal 
data structures. It returns true in the case of success, false otherwise.

The argument path points to the file containing the type map specifications.
The lists should be created using the pnl list storage.

*/
   bool init( const std::string& path);



/*
1.4 Function ~getOpSig~

This function returns the nested list representation for an operator
specified by it's name and the name of the algebra containing it.
If no operator is found, an empty list is returned.

*/
   ListExpr getOpSig( const std::string& algebraName,
                      const std::string& operatorName);



/*
1.5 Function ~typemape~

This function performs the typemap for a given operator signature 
and current values for the types. Note that the operator signature 
used the pnl list storage and the current arguments use the standard 
nl list storage. The result is created using the standard nl list storage.

*/
  ListExpr typemap( const ListExpr SigArgTypes,
                    const ListExpr CurrentArgTypes);



  private:
      NestedList* pnl;
      NestedList* nl;


      Mapper();

}; // end of class Mapper


/*
2 Functions for predicates

*/

ListExpr sig(ListExpr sList);
ListExpr csig(ListExpr cSList);
ListExpr matches(ListExpr mList);
ListExpr consistent(ListExpr B1, ListExpr B2);
bool conflict(ListExpr B1, ListExpr B2);
ListExpr evalPreds(ListExpr ePsList);
ListExpr evalPred(ListExpr ePList);
ListExpr isAttr(ListExpr attrList);
ListExpr isAttr2(ListExpr attrList2);
void checkMember(ListExpr cMList);
void distinctList(ListExpr distLList);
void distinctAttrs(ListExpr distAList);
ListExpr attrNames(ListExpr attrNList);
ListExpr bound(ListExpr boList);
ListExpr addBinding(ListExpr aBList);
ListExpr apply(ListExpr aList);



} // end of namespace typemap

#endif 


