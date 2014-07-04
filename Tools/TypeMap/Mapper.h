/*
----  /Tools/TypeMap/Mapper.h
---- 

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
1.5 Function ~typemap~

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
1.5.1 Functions for predicates

*/


/*
Function ~tmInput~:

This function returns true, if a predicate is found in the input, followed by a nested list. It writes out the resulted nested list or in case of the ~typemap~ predicate the different lists of the parts of typemapping. In this case also the ~SignatureArgs~ of an operator are included.

*/
    bool tmInput(string sigInput, ListExpr sigArgs);



/*
Function ~matches~:

The list of argument types ~CurrentArgTypes~ matches the list of argument type specifications ~SigArgTypes~ with the bindings ~Bindings~. The function returns Bindings.
  
*/
    ListExpr matches(ListExpr mList);
    ListExpr matches2(ListExpr mList2);



/*
Function ~element~:

Within nested list ~Type~ rewrite every occurrence of (lvar, Tc, N) into (var, Tc, (N, I)) and return this as ~Type2~.

*/
    ListExpr element(ListExpr elemList);



/*
Function ~consistent~:

Two lists of bindings ~B1~ and ~B2~ are consistent, if their sets of variables are disjoint or for equal variables they have the same values. The joint bindings are returned in ~Bindings~.
  
*/
    ListExpr consistent(ListExpr B1, ListExpr B2);



/*
Function ~conflict~:

Two bindings ~B1~ and ~B2~ are in conflict if they have the same variable but different values. The function returns true if a conflict is given, false otherwise.
  
*/
    bool conflict(ListExpr B1, ListExpr B2);



/*
Function ~evalPreds~:
  
Evaluate predicates ~Preds~ based on ~Bindings~, resulting in new ~Bindings2~.
  
*/
    ListExpr evalPreds(ListExpr ePsList);
    ListExpr evalPred(ListExpr ePList);



/*
Function ~isAttr~:

This function returns the ~Type~ (e.g. string) and the ~Number~ of a given attribute ~Attr~. If the given attribute is not found in the ~List~, an error is printed out. 

*/  
    ListExpr isAttr(ListExpr attrList);
    ListExpr isAttr2(ListExpr attrList2);



/*
Function ~attrs~:

This function returns a nested list of ~Types~ (e.g. string) and a nested list of ~Numbers~ of the position in the attributlist from a list of given attributes with ~Ident~ and ~Attrs~. It consults the function ~isAttr~.

*/  
    ListExpr attrs(ListExpr attrsList);



/*
Function ~combine~:

This function returns a nested list of attributes ~Attrs~ (e.g. (kennzeichen string)) from two nested lists ~Idents~ and ~Types~.

*/  
    ListExpr combine(ListExpr combList);



/*
Function ~attrNames~:

This function returns a nested list ~Names~ of the attribute names from a nested list ~Attrs~.

*/
    ListExpr attrNames(ListExpr attrNList);



/*
Function ~checkMember~:

This function returns false if a given ~Name~ of an attribute equals not a member name in the rest of list ~Names~. It returns true otherwise and an error is yield.

*/
    bool checkMember(ListExpr cMList);



/*
Function ~distinctList~:

This function returns true if a list of ~Names~ has distincted names as elements and false otherwise.

*/
    bool distinctList(ListExpr distLList);



/*
Function ~distinctAttrs~:

This function returns true if the attributes ~Attrs~ are distincted and false otherwise.

*/
    bool distinctAttrs(ListExpr distAList);



/*
Function ~bound~:

Handling Bindings: The first version ~(Bindings, (var, Tc, No))~ finds a binding for a given variable if it exists. The second version ~(Bindings, (lvar, Tc, No))~ is used for list variables. For them, all bindings of the form (Tc, (No, i), X\_i) will be collected into a list (X\_1, ..., X\_n) and be returned in ~Bound~. 
  
*/
    ListExpr bound(ListExpr boList);
    ListExpr bound2(ListExpr boList2);



/*
Function ~addBinding~:

This function adds a variable to the ~Bindings~, resulting in new ~Bindings2~.

*/
    ListExpr addBinding(ListExpr aBList);



/*
Function ~apply~:

Computing the Result Type: Applying the ~Bindings~ to the result type specification ~Res~ yields the result type ~ResType~.

*/
    ListExpr apply(ListExpr aList);





} // end of namespace typemap

#endif 


