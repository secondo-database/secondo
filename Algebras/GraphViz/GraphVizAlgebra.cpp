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
#include "FLOB.h"
#include "Symbols.h"
#include <sstream>

extern NestedList* nl;
extern QueryProcessor *qp;

namespace GVZ {

  const string DOT = "svg";

  class Dot : public Attribute {

    public:	  
    Dot() {}

    Dot(const string& s) : text(0) {

      SetDefined(true);
      assign(s);	    
    }

    Dot(const Dot& rhs) : text(0) {

      SetDefined( rhs.IsDefined() );
      assign( rhs.GetText() );	    
    }

    ~Dot() {}

    void Set(bool b, const string& s) {
       SetDefined(b);
       assign(s);       
    }	    

    const char* GetText() const {
      const char* s = 0;
      text.Get(0, &s);
      return s;
    }

    inline virtual int NumOfFLOBs() const { return 1; }

    inline virtual FLOB* GetFLOB(const int n) { return &text; }

    virtual size_t  Sizeof() const { return sizeof(Dot); }

    virtual int Compare(const Attribute*)  const { return 0; }

    virtual bool Adjacent(const Attribute*) const { return true; }

    virtual Attribute*  Clone() const { return new Dot(*this); }


    static Word In( const ListExpr typeInfo, const ListExpr instance,
		    const int errorPos, ListExpr& errorInfo, bool& correct );

    static ListExpr Out( ListExpr typeInfo, Word value );

    static Word Create( const ListExpr typeInfo );

    private:

    inline void assign(const string& s) {      
      text.Clean();	    
      text.Put( 0, s.length() + 1, s.c_str());
    }	    

    FLOB text;
  };	  

  struct DotFunctions : ConstructorFunctions<Dot> {

  DotFunctions()
  {
    //re-assign some function pointers
    create = Dot::Create;
    in = Dot::In;
    out = Dot::Out;
  }  
};    

Word
Dot::In( const ListExpr typeInfo, const ListExpr instance,
            const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = false;
  Word result = SetWord(Address(0));

  NList list(instance);
  stringstream ss;
  ss << "Expecting a list of type: text-atom!";
  
  if ( !list.isText() ) {
    ss << "But got " << list; 	  
    cmsg.inFunError(ss.str());
    return result;
  }	  

  correct = true;
  Dot* d = new Dot(list.str());
  result.addr = d;
  return result;
}


ListExpr
Dot::Out( ListExpr typeInfo, Word value )
{
  Dot* d = static_cast<Dot*>( value.addr );
  return NList( NList().textAtom( d->GetText() )).listExpr();
}

Word
Dot::Create( const ListExpr typeInfo )
{
  return (SetWord( new Dot("") ));
}

struct DotInfo : ConstructorInfo {

  DotInfo() {

    name         = DOT;
    signature    = "-> svg";
    typeExample  = name;
    listRep      =  "( svg text-atom)";
    valueExample = "( svg 'xml description ...')";
    remarks      = "This list simply stores a graph represented by a "
	           "svg graphic computed by the graphviz library. "
		   "Note: this is a result of processing "
		   "a node and edge description and may not be appropiate for "
		   "further processing. It is simply used as wrapper to have " 
		   "a special type used by JavaGUI";
  }
};


DotInfo di;
DotFunctions df;
TypeConstructor dotTC( di, df );


/*
Operator ~lastoptree~

*/

 struct lastOpTreeInfo : OperatorInfo {

  lastOpTreeInfo()
  {
    name      = "lastoptree"; 
    signature = " -> " + DOT;
    syntax    = name;
    meaning   = "If debug 1 is activated, SECONDO stores the optree "
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
  return NList(DOT).listExpr();
}

int
lastOpTree_vm (Word* args, Word& result, int message, 
               Word& local, Supplier s)
{
  result = qp->ResultStorage(s);  
  Dot* d = static_cast<Dot*>( result.addr );

  string str = "Error: no file lastoptree.gv!";
  FILE* f = fopen("optree_old.gv", "r");
  if(f != 0) {
    str = FILE2string(f);
  }
  GraphViz g(str);
  string svg = g.render(GraphViz::svg, GraphViz::DOT);

  d->Set(true, svg);
  return 0;
}


class GVZAlgebra : public Algebra
{
  public:
    GVZAlgebra() : Algebra()
    {
      AddTypeConstructor(&dotTC);
      dotTC.AssociateKind("DATA");

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


