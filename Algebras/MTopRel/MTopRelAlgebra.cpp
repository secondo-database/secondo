/*
----
This file is part of SECONDO.

Copyright (C) 2007, 
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



#include "dfa.h"
#include "GenericTC.h"
#include "GenOps.h"
#include "Algebra.h"
#include "FTextAlgebra.h"


/*
1 Type Constructor

*/

GenTC<TopRelDfa> topreldfa;


/*
2 Operators


1.1 createdfa


Operator for creating a topreldfa from a pridicategroup and a 
text.

*/

int
  createDfaVM( Word* args, Word& result, int message,
               Word& local, Supplier s ){

    result = qp->ResultStorage(s);
    toprel::PredicateGroup* pg = 
         static_cast<toprel::PredicateGroup*>(args[0].addr);
    FText* regex = static_cast<FText*>(args[1].addr);
    TopRelDfa* res = static_cast<TopRelDfa*>(result.addr);
    if(!pg->IsDefined() || !regex->IsDefined()){
      res->SetDefined(false); 
      return 0;
    }
    string r = regex->GetValue();

    res->setTo( r, *pg);
    return 0;
}

const string createDfaSpec  = 
      "( ( \"Signature\" \"Syntax\" \"Meaning\" "
           "\"Example\" )"
        "( <text>predicategroup x text -> mtoprel  </text--->"
       "<text>createDfa(_,_)</text--->"
       "<text>Creates an mtoprel object</text--->"
       "<text>query createDfa(stdpgroup(), 'inside meet disjoint'</text--->"
       ") )";


Operator createDfa(
           "createDfa",
           createDfaSpec,
           createDfaVM,
           Operator::SimpleSelect,
           TypeMap2<toprel::PredicateGroup, FText, TopRelDfa>);










class MTopRelAlgebra: public Algebra{

  public:

    MTopRelAlgebra(){
       AddTypeConstructor( &topreldfa );
       topreldfa.AssociateKind("DATA");

       // add operators
       AddOperator(&createDfa);

    }
    
    ~MTopRelAlgebra() {};

};

/*
3 Initialization of the Algebra


*/
extern "C"
Algebra*
InitializeMTopRelAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new MTopRelAlgebra());
}




