/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

//paragraph [1] title: [{\Large \bf ]	[}]
//[->] [$\rightarrow$]



[1] Stream Example Algebra

July 2002 RHG

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

August 2006, Christian Duentgen changed explicit allowed types for 
streams in operators ~count~ and ~filter~ to all datatypes in kind DATA.
Added operator ~printstream~.

This little algebra demonstrates the use of streams and parameter functions
in algebra operators. It does not introduce any type constructors, but has
several operators to manipulate streams.


1 Preliminaries

1.1 Includes

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"	//We need integers, for example
#include <string>
#include <iostream>		//for testing

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager* am;


/*

2 Creating Operators

2.1 Overview

This algebra provides the following operators:

  * int x int [->] stream(int) 	intstream

    Creates a stream of integers containing all integers between the first and
the second argument. If the second argument is smaller than the first, the
stream will be empty.

  * stream(T) [->] int	count

    Returns the number of elements in an arbitrary stream.

  * stream(int) [->] stream(int)		printintstream

    Prints out all elements of the stream

  * stream(T) [->] stream(T)		printstream

    Prints out all elements of the stream

  * stream(T) x (T [->] bool) [->] stream(T)	filter

    Filters the elements of an arbitrary stream by a predicate.


2.2 Type Mapping Function

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol
~typeerror~.


Type mapping for ~intstream~ is

----	(int int) -> (stream int)
----

*/
ListExpr
intstreamType( ListExpr args ){
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, "int") && nl->IsEqual(arg2, "int") )
      return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("int"));
  }
  return nl->SymbolAtom("typeerror");
}

/*
Type mapping for ~count~ is

----	((stream int)) -> int
----

*/
ListExpr
countType( ListExpr args )
{
  ListExpr arg1;
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  string outstr;

  if ( nl->ListLength(args) == 1 )
  {
    arg1 = nl->First(args);

    if ( nl->ListLength(arg1) == 2 )
      {
	if (    nl->IsEqual(nl->First(arg1), "stream")
	     && ( nl->IsAtom(nl->Second(arg1) ) )
	     && am->CheckKind("DATA", nl->Second(arg1), errorInfo) )
	  return nl->SymbolAtom("int");
	else
	  {
	    nl->WriteToString(outstr, arg1);
	    ErrorReporter::ReportError("Operator count expects a (stream T), "
				       "in kind DATA. The argument profided "
				       "has type '" + outstr + "' instead.");
	  }
      }
  }
  else
    {
      nl->WriteToString(outstr, nl->First(args));
      ErrorReporter::ReportError("Operator count expects only a single "
				 " argument of type (stream T), T "
				 "in kind DATA. The argument provided "
				 "has type '" + outstr + "' instead.");	
    }
  return nl->SymbolAtom("typeerror");
}

/*
Type mapping for ~printintstream~ is

----	((stream int)) -> (stream int)
----

*/
ListExpr
printintstreamType( ListExpr args )
{
  ListExpr arg11, arg12;
  string out;

  if ( nl->ListLength(args) == 1 )
  {
    arg11 = nl->First(nl->First(args));
    arg12 = nl->Second(nl->First(args));

    if ( nl->IsEqual(arg11, "stream") && nl->IsEqual(arg12, "int") )
      return nl->First(args);
  }
  nl->WriteToString(out, nl->First(args));
  ErrorReporter::ReportError("Operator printintstream expects a "
			     "(stream int) as its first argument. "
			     "The argument provided "
			     "has type '" + out + "' instead.");  
  return nl->SymbolAtom("typeerror");
}

/*
Type mapping for ~printstream~ is

----	((stream T)) -> (stream T)
        T in kind DATA
----

*/
ListExpr
printstreamType( ListExpr args )
{
  ListExpr stream, errorInfo;
  string out;

  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  stream = nl->First(args);

  if ( nl->ListLength(args) != 1 )
    {
      ErrorReporter::ReportError("Operator printstream expects only a single "
				 "argument.");
      return nl->SymbolAtom("typeerror");
    }

  // test first argument for stream(T), T in kind DATA
  if (     nl->IsAtom(stream)
	   || !(nl->ListLength(stream) == 2)
	   || !nl->IsEqual(nl->First(stream), "stream")
	   || !am->CheckKind("DATA", nl->Second(stream), errorInfo) )
    {
      nl->WriteToString(out, stream);
      ErrorReporter::ReportError("Operator printstream expects a (stream T), "
				 "T in kind DATA, as its first argument. "
				 "The argument provided "
				 "has type '" + out + "' instead.");
      return nl->SymbolAtom("typeerror");
    }

  // return the input type as result
  return stream; 
}

/*
Type mapping for ~sfilter~ is

----	((stream int) (map int bool)) -> (stream x)
----

*/
ListExpr
filterType( ListExpr args )
{
  ListExpr stream, map, errorInfo;
  string out, out2;

  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  if ( nl->ListLength(args) == 2 )
  {
    stream = nl->First(args);
    map = nl->Second(args);

    // test first argument for stream(T), T in kind DATA
    if (     nl->IsAtom(stream)
	 || !(nl->ListLength(stream) == 2)
	 || !nl->IsEqual(nl->First(stream), "stream")
	 || !am->CheckKind("DATA", nl->Second(stream), errorInfo) )
      {
	nl->WriteToString(out, stream);
	ErrorReporter::ReportError("Operator filter expects a (stream T), "
				   "T in kind DATA as its first argument. "
				   "The argument provided "
				   "has type '" + out + "' instead.");
	return nl->SymbolAtom("typeerror");
      }

    // test second argument for map T' bool. T = T'
    if (     nl->IsAtom(map)
	 || !nl->ListLength(map) == 3
         || !nl->IsEqual(nl->First(map), "map")
         || !nl->IsEqual(nl->Third(map), "bool") )
      {
	nl->WriteToString(out, map);
	ErrorReporter::ReportError("Operator filter expects a "
				   "(map T bool), T in kind DATA, "
				   "as its second argument. "
				   "The second argument provided "
				   "has type '" + out + "' instead.");
	return nl->SymbolAtom("typeerror");
      }
    
    if ( !( nl->Equal( nl->Second(stream), nl->Second(map) ) ) )
      {
	nl->WriteToString(out, nl->Second(stream));
	nl->WriteToString(out2, nl->Second(map));
	ErrorReporter::ReportError("Operator filter: the stream base type "
				   "T must match the map's argument type, "
				   "e.g. 1st: (stream T), 2nd: (map T bool). "
				   "The actual types are 1st: '" + out +
				   "', 2nd: '" + out2 + "'.");
	return nl->SymbolAtom("typeerror");
      }
  }
  else 
    { // wrong number of arguments
      ErrorReporter::ReportError("Operator filter expects two arguments.");
      return nl->SymbolAtom("typeerror");      
    }
  return stream; // return type of first argument
}

/*
4.2 Selection Function

Is used to select one of several evaluation functions for an overloaded
operator, based on the types of the arguments. In case of a non-overloaded
operator, we just have to return 0.

*/

int
simpleSelect (ListExpr args ) { return 0; }

/*
4.3 Value Mapping Function

*/
int
intstreamFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Create integer stream. An example for creating a stream.

Note that for any operator that produces a stream its arguments are NOT
evaluated automatically. To get the argument value, the value mapping function
needs to use ~qp->Request~ to ask the query processor for evaluation explicitly.
This is illustrated in the value mapping functions below.

*/
{
  struct Range {int current, last;}* range;

  CcInt* i1;
  CcInt* i2;
  CcInt* elem;

  Word arg0, arg1;

  switch( message )
  {
    case OPEN:

      qp->Request(args[0].addr, arg0);
      qp->Request(args[1].addr, arg1);

      i1 = ((CcInt*)arg0.addr);
      i2 = ((CcInt*)arg1.addr);

      range = new Range;
      range->current = i1->GetIntval();
      range->last =  i2->GetIntval();

      local.addr = range;

      return 0;

    case REQUEST:

      range = ((Range*) local.addr);

      if ( range->current <= range->last )
      {
        elem = new CcInt(true, range->current++);
	result.addr = elem;
        return YIELD;
      }
      else return CANCEL;

    case CLOSE:

      range = ((Range*) local.addr);
      delete range;
      return 0;
  }
  /* should not happen */
  return -1;
}

int
countFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Count the number of elements in a stream. An example for consuming a stream.

*/
{
  Word elem;
  int count = 0;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);

  while ( qp->Received(args[0].addr) )
  {
    count++;
    ((Attribute*) elem.addr)->DeleteIfAllowed();// consume the stream objects
    qp->Request(args[0].addr, elem);
  }
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, count);

  qp->Close(args[0].addr);

  return 0;
}

int
printintstreamFun (Word* args, Word& result, 
		   int message, Word& local, Supplier s)
/*
Print the elements of an Attribute-type stream. 
An example for a pure stream operator (input and output are streams).

*/
{
  Word elem;

  switch( message )
  {
    case OPEN:

      qp->Open(args[0].addr);
      return 0;

    case REQUEST:

      qp->Request(args[0].addr, elem);
      if ( qp->Received(args[0].addr) )
      {
        cout << ((CcInt*) elem.addr)->GetIntval() << endl;
	result = elem;
	return YIELD;
      }
      else return CANCEL;

    case CLOSE:

      qp->Close(args[0].addr);
      return 0;
  }
  /* should not happen */
  return -1;
}

int
printstreamFun (Word* args, Word& result, 
		   int message, Word& local, Supplier s)
/*
Print the elements of an Attribute-type stream. 
An example for a pure stream operator (input and output are streams).

*/
{
  Word elem;

  switch( message )
  {
    case OPEN:

      qp->Open(args[0].addr);
      return 0;

    case REQUEST:

      qp->Request(args[0].addr, elem);
      if ( qp->Received(args[0].addr) )
      {
        ((Attribute*) elem.addr)->Print(cout); cout << endl;
	result = elem;
	return YIELD;
      }
      else return CANCEL;

    case CLOSE:

      qp->Close(args[0].addr);
      return 0;
  }
  /* should not happen */
  return -1;
}

int
filterFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Filter the elements of a stream by a predicate. An example for a stream
operator and also for one calling a parameter function.

*/
{
  Word elem, funresult;
  ArgVectorPointer funargs;

  switch( message )
  {
    case OPEN:

      qp->Open(args[0].addr);
      return 0;

    case REQUEST:

      funargs = qp->Argument(args[1].addr);  //Get the argument vector for
					     //the parameter function.
      qp->Request(args[0].addr, elem);
      while ( qp->Received(args[0].addr) )
      {
	(*funargs)[0] = elem;		     //Supply the argument for the
					     //parameter function.
        qp->Request(args[1].addr, funresult);//Ask the parameter function
					     //to be evaluated.
	if ( ((CcBool*) funresult.addr)->GetBoolval() )
	{
	  result = elem;
	  return YIELD;
        }
	//consume the stream object:
	((Attribute*) elem.addr)->DeleteIfAllowed(); 
	qp->Request(args[0].addr, elem); // get next element
      }
      return CANCEL;

    case CLOSE:

      qp->Close(args[0].addr);
      return 0;
  }
  /* should not happen */
  return -1;
}

/*
4.4 Definition of Operators

*/

const string intstreamSpec  = 
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>StreamExampleAlgebra</text--->"
  "<text>(int int) -> (stream int)</text--->"
  "<text>intstream ( _ , _ )</text--->"
  "<text>Creates a stream of integers containing the numbers "
  "between the first and the second argument.</text--->"
  "<text>query intstream (1,10) printintstream count</text--->"
  ") )";

const string countSpec  = 
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>StreamExampleAlgebra</text--->"
  "<text>For T in kind DATA:\n"
  "((stream T)) -> int</text--->"
  "<text>_ count</text--->"
  "<text>Counts the number of elements of a stream.</text--->"
  "<text>query intstream (1,10) count</text--->"
  ") )";

const string printintstreamSpec  = 
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>StreamExampleAlgebra</text--->"
  "<text>((stream int)) -> (stream int)</text--->"
  "<text>_ printintstream</text--->"
  "<text>Prints the elements of an integer stream.</text--->"
  "<text>query intstream (1,10) printintstream count</text--->"
  ") )";

const string printstreamSpec  = 
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>StreamExampleAlgebra</text--->"
  "<text>For T in kind DATA:\n"
  "((stream T)) -> (stream T)</text--->"
  "<text>_ printstream</text--->"
  "<text>Prints the elements of an arbitrary stream.</text--->"
  "<text>query intstream (1,10) printstream count</text--->"
  ") )";

const string filterSpec  = 
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>StreamExampleAlgebra</text--->"
  "<text>For T in kind DATA:\n"
  "((stream T) (map T bool)) -> (stream T)</text--->"
  "<text>_ filter [ fun ]</text--->"
  "<text>Filters the elements of a stream by a predicate.</text--->"
  "<text>query intstream (1,10) filter[. > 7] printintstream count</text--->"
  ") )";
/*
Used to explain the signature and the meaning of operators.

*/

Operator intstream (
	"intstream", 		//name
	intstreamSpec,         //specification
	intstreamFun,		//value mapping
	simpleSelect,		//trivial selection function
	intstreamType		//type mapping
);

Operator cppcount (
	"count", 		//name
	countSpec,         	//specification
	countFun,		//value mapping
	simpleSelect,		//trivial selection function
	countType		//type mapping
);

Operator printintstream (
	"printintstream", 	//name
	printintstreamSpec,	//specification
	printintstreamFun,	//value mapping
	simpleSelect,		//trivial selection function
	printintstreamType	//type mapping
);

Operator printstream (
	"printstream", 	        //name
	printstreamSpec,	//specification
	printstreamFun,	        //value mapping
	simpleSelect,		//trivial selection function
	printstreamType	        //type mapping
);

Operator sfilter (
	"filter", 		//name
	filterSpec,		//specification
	filterFun,		//value mapping
	simpleSelect,		//trivial selection function
	filterType		//type mapping
);



/*
5 Creating the Algebra

*/

class StreamExampleAlgebra : public Algebra
{
 public:
  StreamExampleAlgebra() : Algebra()
  {
    AddOperator( &intstream );
    AddOperator( &cppcount );
    AddOperator( &printintstream );
    AddOperator( &printstream );
    AddOperator( &sfilter );
  }
  ~StreamExampleAlgebra() {};
};

StreamExampleAlgebra streamExampleAlgebra;

/*
6 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeStreamExampleAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&streamExampleAlgebra);
}

