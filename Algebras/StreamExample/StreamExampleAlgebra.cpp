/*
//paragraph [1] title: [{\Large \bf ]	[}]
//[->] [$\rightarrow$]



[1] Stream Example Algebra

July 2002 RHG

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
#include "StandardTypes.h"	//We need integers, for example
#include <string>
#include <iostream>		//for testing

static NestedList* nl;
static QueryProcessor* qp;


/*

2 Creating Operators

2.1 Overview

This algebra provides the following operators:

  * int x int [->] stream(int) 	intstream

    Creates a stream of integers containing all integers between the first and
the second argument. If the second argument is smaller than the first, the
stream will be empty.

  * stream(int) [->] int	count

    Returns the number of elements in an integer stream.  


  * stream(int) [->] stream(int)		printintstream

    Prints out all elements of the stream

  * stream(int) x (int [->] bool) [->] stream(int)	filter

    Filters the elements of an integer stream by a predicate.


2.2 Type Mapping Function

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol
~typeerror~.


Type mapping for ~intstream~ is

----	(int int) -> (stream int)
----

*/
static ListExpr
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
static ListExpr
countType( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength(args) == 1 )
  {
    arg1 = nl->First(args);

    if ( nl->ListLength(arg1) == 2 ) 
      if ( nl->IsEqual(nl->First(arg1), "stream") 
	   && nl->IsEqual(nl->Second(arg1), "int") ) 
      return nl->SymbolAtom("int");
  }
  return nl->SymbolAtom("typeerror");
}

/*
Type mapping for ~printintstream~ is

----	((stream int)) -> (stream int)
----

*/
static ListExpr
printintstreamType( ListExpr args )
{
  ListExpr arg11, arg12;
  if ( nl->ListLength(args) == 1 )
  {
    arg11 = nl->First(nl->First(args));
    arg12 = nl->Second(nl->First(args));

    if ( nl->IsEqual(arg11, "stream") && nl->IsEqual(arg12, "int") )
      return nl->First(args);
  }
  return nl->SymbolAtom("typeerror");
}

/*
Type mapping for ~filter~ is

----	((stream int) (map int bool)) -> (stream x)
----

*/
static ListExpr
filterType( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);

    if ( nl->ListLength(arg1) == 2 && nl->ListLength(arg2) == 3 
      && nl->IsEqual(nl->First(arg1), "stream")
      && nl->IsEqual(nl->Second(arg1), "int")
      && nl->IsEqual(nl->First(arg2), "map")
      && nl->IsEqual(nl->Second(arg2), "int")
      && nl->IsEqual(nl->Third(arg2), "bool") )
    return arg1;
  } 
  return nl->SymbolAtom("typeerror");
}

/*
4.2 Selection Function

Is used to select one of several evaluation functions for an overloaded
operator, based on the types of the arguments. In case of a non-overloaded
operator, we just have to return 0.

*/

static int
simpleSelect (ListExpr args ) { return 0; }

/*
4.3 Value Mapping Function

*/
static int
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

static int
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
    delete((CcInt*) elem.addr);			//consume the stream objects
    qp->Request(args[0].addr, elem);
  }
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, count);

  qp->Close(args[0].addr);

  return 0;
}

static int
printintstreamFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Print the elements of an integer stream. An example for a pure stream operator
(input and output are streams).

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

static int
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

      funargs = qp->Argument(args[1].addr);	//Get the argument vector for
						//the parameter function.
      qp->Request(args[0].addr, elem);
      while ( qp->Received(args[0].addr) )
      {
	(*funargs)[0] = elem;			//Supply the argument for the
						//parameter function.
        qp->Request(args[1].addr, funresult);	//Ask the parameter function
						//to be evaluated.
	if ( ((CcBool*) funresult.addr)->GetBoolval() )      
	{	 	 	
	  result = elem;
	  return YIELD;
        }
      qp->Request(args[0].addr, elem);
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

const string intstreamSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" ) "
                             "( <text>(int int) -> (stream int)</text--->"
			    "<text>intstream ( _ , _ ) [ fun ]</text--->"
			    "<text>Creates a stream of integers containing "
			    "the numbers between the first and the second "
			    "argument.</text--->"
			    "<text>query intstream (1,10) printintstream "
			    "count</text--->"
			      ") )";

const string countSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" " 
                          "\"Example\" ) "
                             "( <text>((stream x)) -> int</text--->"
			     "<text>_ count</text--->"
			     "<text>Counts the number of elements of a "
			     "stream.</text--->"
			     "<text>query intstream (1,10) count</text--->"
			     ") )";

const string printintstreamSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                   "\"Example\" ) "
                            "( <text>((stream x)) -> (stream x)</text--->"
			    "<text>_ printintstream</text--->"
			    "<text>Prints the elements of an integer "
			    "stream.</text--->"
			    "<text>query intstream (1,10) printintstream "
			    "count</text--->"
			    ") )";

const string filterSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>((stream x) (map x bool)) -> (stream x)"
			   "</text--->"
			   "<text>_ filter [ fun ]</text--->"
			   "<text>Filters the elements of a stream by a "
			   "predicate.</text--->"
			   "<text>query intstream (1,10) filter[. > 7] "
			   "printintstream count</text--->"
			      ") )";
/*
Used to explain the signature and the meaning of operators.

*/

Operator intstream (
	"intstream", 		//name
	intstreamSpec,         //specification
	intstreamFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function 
	intstreamType		//type mapping
);

Operator cppcount (
	"count", 		//name
	countSpec,         	//specification
	countFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function 
	countType		//type mapping
);

Operator printintstream (
	"printintstream", 	//name
	printintstreamSpec,	//specification
	printintstreamFun,	//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function 
	printintstreamType	//type mapping
);

Operator filter (
	"filter", 		//name
	filterSpec,		//specification
	filterFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
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
    AddOperator( &filter );
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

