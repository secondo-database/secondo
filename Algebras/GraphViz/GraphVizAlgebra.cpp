/*
---- 
This file is part of SECONDO.

Copyright (C) 2002-2009, University in Hagen, Faculty of Mathematics and Computer Science, 
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

/*

April 2009, M. Spiekermann

1 Overview

This module serves as wrapper for using an optional viewer which
dependens on graphviz and grappa. graphviz is a collection of
layout algorithms offered as command line tools or library. It
translates a graph notated in the so called DOT language into
several output formats.

Here we will use simply a textual output format which contains the
calculated node placement information and use the java package grappa
for displaying those graphs.

For further information please consult www.graphviz.org.

Currently this algebra contains only one operator called 
~lastoptree~ which returns a ~dot~ object for the last operator
tree if debug mode 1 is activated. Later it will be extended by
a special render operator which may create a graph notation extracted
from a stream of tuples which contains string or text attributes.

*/


#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "GraphViz.h"
#include "LogMsg.h"
#include "Symbols.h"
#include "../FText/FTextAlgebra.h"
#include <sstream>

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;

namespace GVZ {

/*
Operator ~lastoptree~

*/

 struct lastOpTreeInfo : OperatorInfo {

  lastOpTreeInfo()
  {
    name      = "lastoptree"; 
    signature = " -> svg";
    syntax    = name;
    meaning   = "If debug mode 1 is activated, SECONDO stores the optree "
	        "represented in SVG-language in a temporary file. "
		"This operator simply converts the stored file into a "
		"graphlayout object";
  }
};  


ListExpr
lastOpTree_tm( ListExpr args )
{	
  if ( !nl->IsEmpty(args) ) {
    return NList::typeError("Expecting an empty list");
  }  
  return NList("svg").listExpr();
}

int
lastOpTree_vm (Word* args, Word& result, int message, 
               Word& local, Supplier s)
{
  result = qp->ResultStorage(s);  
  FText* d = static_cast<FText*>( result.addr );

  const string lf = "optree_old.gv";
  string err = "no file " + lf + "!";
  FILE* f = fopen(lf.c_str(), "r");
  if(f != 0) {
    string str = FILE2string(f);
    GraphViz g(str);
    string svg = g.render(GraphViz::svg, GraphViz::DOT);
    d->Set(true, svg);
  }
  else
  {
    cmsg.error() << err << endl;
    cmsg.send();    
    d->Set(false, "");
  }	  
  return 0;
}


class GVZAlgebra : public Algebra
{
  public:
    GVZAlgebra() : Algebra()
    {
      AddOperator( lastOpTreeInfo(), lastOpTree_vm, lastOpTree_tm );
    }
    ~GVZAlgebra() {}
};

}; // end of namespace
/*

5 Initialization

*/

extern "C"
Algebra*
InitializeGraphVizAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return ( new GVZ::GVZAlgebra() );
}


