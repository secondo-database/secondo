/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module

January 2004 Victor Almeida

March - April 2004 Zhiming Ding

[TOC]

1 Overview

This file contains the implementation of the type constructors ~instant~, 
~range~, ~intime~, ~const~, and ~mapping~. The memory data structures 
used for these type constructors are implemented in the TemporalAlgebra.h
file.

2 Defines, includes, and constants

*/
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;


#include "DateTime.h"
using namespace datetime;

#include "TemporalAlgebra.h"

ListExpr
RangeIntProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,"lci means left closed interval, rci respectively right closed interval,"
                             " e.g. (0 1 TRUE FALSE) means the range [0, 1[");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> RANGE"),
                             nl->StringAtom("(rangeint) "),
                             nl->StringAtom("( (b1 e1 lci rci) ... "
                             "(bn en lci rci) )"),
                             nl->StringAtom("( (0 1 TRUE FALSE)"
                             "(2 5 TRUE TRUE) )"),
                             remarkslist)));
}

/*
4.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. It
checks if the argument $\alpha$ of the range belongs to the ~BASE~ kind.

*/
bool
CheckRangeInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "rangeint" ));
}

/*
4.4 Creation of the type constructor ~rangeint~

*/
TypeConstructor rangeint(
        "rangeint",                 	                    //name
        RangeIntProperty,            	    //property function describing signature
        OutRange<CcInt, OutCcInt>,
        InRange<CcInt, InCcInt>,               //Out and In functions
        0,                      0,       	                   //SaveToList and RestoreFromList functions
        CreateRange<CcInt>,DeleteRange<CcInt>,     //object creation and deletion
        OpenRange<CcInt>,  SaveRange<CcInt>,       // object open and save
        CloseRange<CcInt>, CloneRange<CcInt>,      //object close and clone
        CastRange<CcInt>,                        //cast function
        SizeOfRange<CcInt>,                    //sizeof function
        CheckRangeInt,                              //kind checking function
        0,                                    		   //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );


/*
5 Type Constructor ~rangereal~

This type constructor implements the carrier set for ~range(real)~.

5.1 List Representation

The list representation of a ~rangereal~ is

----    ( (i1b i1e lc1 rc1) (i2b i2e lc2 rc2) ... (inb ine lcn rcn) )
----

For example:

----    ( (1.01 5 TRUE FALSE) (6.37 9.9 FALSE FALSE) (11.93 11.99 TRUE TRUE) )
----

5.2 function Describing the Signature of the Type Constructor

*/
ListExpr
RangeRealProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,"lci means left closed interval, rci respectively right closed interval,"
                             " e.g. (0.5 1.1 TRUE FALSE) means the range [0.5, 1.1[");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> RANGE"),
                             nl->StringAtom("(rangereal) "),
                             nl->StringAtom("( (b1 e1 lci rci) ... "
                             "(bn en lci rci) )"),
                             nl->StringAtom("( (0.5 1.1 TRUE FALSE)"
                             "(2 5.04 TRUE TRUE) )"),
                             remarkslist)));
}

/*
5.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. It
checks if the argument $\alpha$ of the range belongs to the ~BASE~ kind.

*/
bool
CheckRangeReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "rangereal" ));
}

/*
5.4 Creation of the type constructor ~rangereal~

*/
TypeConstructor rangereal(
        "rangereal",                  				//name
        RangeRealProperty,   				//property function describing signature
        OutRange<CcReal, OutCcReal>,
        InRange<CcReal, InCcReal>, 		     	//Out and In functions
        0,              	0,            			 	//SaveToList and RestoreFromList functions
        CreateRange<CcReal>,DeleteRange<CcReal>,  	//object creation and deletion
        OpenRange<CcReal>,SaveRange<CcReal>,	// object open and save
        CloseRange<CcReal>,CloneRange<CcReal>,	//object close and clone
        CastRange<CcReal>,				//cast function
        SizeOfRange<CcReal>,                  		//sizeof function
        CheckRangeReal,      		              		//kind checking function
        0,                    			        		//predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
6 Type Constructor ~periods~

This type constructor implements the carrier set for ~range(instant)~, which is
called ~periods~.

6.1 List Representation

The list representation of a ~periods~ is

----    ( (i1b i1e lc1 rc1) (i2b i2e lc2 rc2) ... (inb ine lcn rcn) )
----

For example:

----    ( ( (instant 1.01)  (instant 5)     TRUE  FALSE) 
          ( (instant 6.37)  (instant 9.9)   FALSE FALSE) 
          ( (instant 11.93) (instant 11.99) TRUE  TRUE) )
----

6.2 function Describing the Signature of the Type Constructor

*/
ListExpr
PeriodsProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,"lci means left closed interval, rci respectively right closed interval,"
                             " e.g. ((instant 0.5) (instant 1.1) TRUE FALSE) means the interval [0.5, 1.1[");

  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> RANGE"),
                             nl->StringAtom("(periods) "),
                             nl->StringAtom("( (b1 e1 lci rci) ... (bn en lci rci) )"),
                             nl->StringAtom("( ((instant 0.5) (instant 1.1) TRUE FALSE) ...)"))));
}

/*
6.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. It
checks if the argument $\alpha$ of the range belongs to the ~BASE~ kind.

*/
bool
CheckPeriods( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "periods" ));
}

/*
6.4 Creation of the type constructor ~periods~

*/
TypeConstructor periods(
        "periods",            		                        //name
        PeriodsProperty,        		                        //property function describing signature
        OutRange<Instant, OutDateTime>,
        InRange<Instant, InInstant>, 		        //Out and In functions
        0,                      0,     			        //SaveToList and RestoreFromList functions
        CreateRange<Instant>, DeleteRange<Instant>,  //object creation and deletion
        OpenRange<Instant>,   SaveRange<Instant>,     // object open and save
        CloseRange<Instant>,  CloneRange<Instant>,    //object close and clone
        CastRange<Instant>,                          	   //cast function
        SizeOfRange<Instant>,                         	   //sizeof function
        CheckPeriods,                                  	   //kind checking function
        0,                                             		   //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
7 Type Constructor ~intimeint~

Type ~intimeint~ represents an (instant, value)-pair of integers.

7.1 List Representation

The list representation of an ~intimeint~ is

----    ( t int-value )
----

For example:

----    ( 1.0 5 )
----

7.2 function Describing the Signature of the Type Constructor

*/
ListExpr
IntimeIntProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(intimeint) "),
                             nl->StringAtom("( (inst val) "),
                             nl->StringAtom("( ((instant 0.5) 1 )"))));
}

/*
7.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. 

*/
bool
CheckIntimeInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "intimeint" ));
}

/*
7.4 Creation of the type constructor ~intimeint~

*/
TypeConstructor intimeint(
        "intimeint",                             	    //name
        IntimeIntProperty,                        	    //property function describing signature
        OutIntime<CcInt, OutCcInt>,
        InIntime<CcInt, InCcInt>,                 //Out and In functions
        0,                      0,                	    //SaveToList and RestoreFromList functions
        CreateIntime<CcInt>,    DeleteIntime<CcInt>,  //object creation and deletion
        0,                      0,                 	   // object open and save
        CloseIntime<CcInt>,     CloneIntime<CcInt>,   //object close and clone
        CastIntime<CcInt>,                          //cast function
        SizeOfIntime<CcInt>,                      //sizeof function
        CheckIntimeInt,                                //kind checking function
        0,                                           	   //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
8 Type Constructor ~intimereal~

Type ~intimereal~ represents an (instant, value)-pair of reals.

8.1 List Representation

The list representation of an ~intimereal~ is

----    ( t real-value )
----

For example:

----    ( 1.0 5.0 )
----

8.2 function Describing the Signature of the Type Constructor

*/
ListExpr
IntimeRealProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(intimereal) "),
                             nl->StringAtom("( (inst val) "),
                             nl->StringAtom("( ((instant 0.5) 1.0 )"))));
}

/*
8.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckIntimeReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "intimereal" ));
}

/*
8.4 Creation of the type constructor ~intimeint~

*/
TypeConstructor intimereal(
        "intimereal",                        	          //name
        IntimeRealProperty,                   	         //property function describing signature
        OutIntime<CcReal, OutCcReal>,
        InIntime<CcReal, InCcReal>,             //Out and In functions
        0,                      0,            	        //SaveToList and RestoreFromList functions
        CreateIntime<CcReal>,    DeleteIntime<CcReal>,  //object creation and deletion
        0,                      0,             	        // object open and save
        CloseIntime<CcReal>,     CloneIntime<CcReal>,   //object close and clone
        CastIntime<CcReal>,                          //cast function
        SizeOfIntime<CcReal>,                      //sizeof function
        CheckIntimeReal,                                //kind checking function
        0,                                        	        //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
9 Type Constructor ~intimepoint~

Type ~intimereal~ represents an (instant, value)-pair of reals.

9.1 List Representation

The list representation of an ~intimereal~ is

----    ( t real-value )
----

For example:

----    ( 1.0 5.0 )
----

9.2 function Describing the Signature of the Type Constructor

*/
ListExpr
IntimePointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(intimepoint) "),
                             nl->StringAtom("(instant point) "),
                             nl->StringAtom("( 0.5 (1.0 2.0) )"))));
}

/*
9.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckIntimePoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "intimepoint" ));
}

/*
9.4 Creation of the type constructor ~intimepoint~

*/
TypeConstructor intimepoint(
        "intimepoint",                                	     //name
        IntimePointProperty,                         //property function describing signature
        OutIntime<Point, OutPoint>,
        InIntime<Point, InPoint>,                 //Out and In functions
        0,                      0,                	    //SaveToList and RestoreFromList functions
        CreateIntime<Point>,    DeleteIntime<Point>,  //object creation and deletion
        0,                      0,                 	   // object open and save
        CloseIntime<Point>,     CloneIntime<Point>,   //object close and clone
        CastIntime<Point>,                            //cast function
        SizeOfIntime<Point>,                        //sizeof function
        CheckIntimePoint,                             //kind checking function
        0,                                          	     //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
10 Type Constructor ~constint~

Type ~constint~ represents an (tinterval, intvalue)-pair.

10.1 List Representation

The list representation of an ~constint~ is

----    ( timeinterval int-value )
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   5 )
----

10.2 function Describing the Signature of the Type Constructor

*/
ListExpr
ConstIntProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("(constint) "),
                             nl->StringAtom("(timeInterval int) "),
                             nl->StringAtom("( (6.37 9.9 FALSE FALSE) 1 )"))));
}

/*
10.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. 

*/
bool
CheckConstInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "constint" ));
}

/*
10.4 Creation of the type constructor ~constint~

*/
TypeConstructor constint(
        "constint",                   	  	      	       //name
        ConstIntProperty,                 	     	      //property function describing signature
        OutConstTemporalUnit<CcInt, OutCcInt>,
        InConstTemporalUnit<CcInt, InCcInt>,	     //Out and In functions
        0,                      0,           	     	    	     //SaveToList and RestoreFromList functions
        CreateConstTemporalUnit<CcInt>,
        DeleteConstTemporalUnit<CcInt>,  	    //object creation and deletion
        0,                      0,            	        	    // object open and save
        CloseConstTemporalUnit<CcInt>,     
        CloneConstTemporalUnit<CcInt>,   	    //object close and clone
        CastConstTemporalUnit<CcInt>,       	    //cast function
        SizeOfConstTemporalUnit<CcInt>, 	    //sizeof function
        CheckConstInt,                        	  	    //kind checking function
        0,                                           	      	   //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
11 Type Constructor ~ureal~

Type ~ureal~ represents an (tinterval, (a, b, c, r))-pair. a, b, c are real numbers, r is a boolean flag

11.1 List Representation

The list representation of an ~ureal~ is

----    ( timeinterval (a b c r)) where a, b, c are real numbers, and r is a boolean flag
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   (1.0 2.3 4.1 TRUE) )
----

11.2 function Describing the Signature of the Type Constructor

*/
ListExpr
UrealProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("(ureal) "),
                             nl->StringAtom("( timeInterval (real1 real2 real3 bool)) "),
                             nl->StringAtom("( (6.37 9.9 T F) (1.0 2.2 2.5 T) )"))));
}

/*
11.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. 

*/
bool
CheckUreal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "ureal" ));
}

/*
11.4 ~Out~-function

*/
ListExpr OutUreal( ListExpr typeInfo, Word value )
{
  //1.get the address of the object and have a class object
  UReal* ureal = (UReal*)(value.addr);

  //2.output the time interval -> NL
  ListExpr timeintervalList = nl->FourElemList(
          OutDateTime( nl->TheEmptyList(), SetWord(&ureal->timeInterval.start) ),
          //nl->RealAtom( ureal->timeInterval.start.GetRealval()),
          OutDateTime( nl->TheEmptyList(), SetWord(&ureal->timeInterval.end) ),
          //nl->RealAtom( ureal->timeInterval.end.GetRealval()),
          nl->BoolAtom( ureal->timeInterval.lc ),
          nl->BoolAtom( ureal->timeInterval.rc));

  //3. get the real function NL (a b c r)
    ListExpr realfunList = nl->FourElemList(
            nl->RealAtom( ureal->a),
            nl->RealAtom( ureal->b),
            nl->RealAtom( ureal->c ),
            nl->BoolAtom( ureal->r));

  //4. return the final result
  return nl->TwoElemList(timeintervalList, realfunList );
}

/*
5.4.2 ~In~-function

the Nested list form is like this:  ( ( 6.37 9.9 TRUE FALSE)   (1.0 2.3 4.1 TRUE) )

*/
Word InUreal( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
    if ( nl->ListLength( instance ) == 2 )
    {
        //1. deal with the time interval  ( 6.37  9.9  T F) or ((instant 1.0) (instant 2.3) T F)
        ListExpr first = nl->First( instance );
        if( nl->ListLength( first ) == 4 &&
            //nl->IsAtom( nl->First( first ) ) &&
            //nl->IsAtom( nl->Second( first ) ) &&
            nl->IsAtom( nl->Third( first ) ) &&
            nl->AtomType( nl->Third( first ) ) == BoolType &&
            nl->IsAtom( nl->Fourth( first ) ) &&
            nl->AtomType( nl->Fourth( first ) ) == BoolType )
        {
            Instant *start = (Instant *)InInstant(
                    nl->TheEmptyList(),
                    nl->First( first ),
                    errorPos, errorInfo, correct ).addr;
            //Instant *start = new Instant ( true, nl->RealValue( nl->First( first ) ) );
            if( correct == false )
            {
                return SetWord( Address(0) );
            }

            Instant *end = (Instant *)InInstant(
                    nl->TheEmptyList(),
                    nl->Second( first ),
                    errorPos, errorInfo, correct ).addr;
            //Instant *end = new Instant ( true, nl->RealValue( nl->Second( first ) ) );
            if( correct == false )
            {
                delete start;
                return SetWord( Address(0) );
            }

            Interval<Instant> tinterval( *start, *end,
                                      nl->BoolValue( nl->Third( first ) ),
                                      nl->BoolValue( nl->Fourth( first ) ) );

            delete start;
            delete end;

            //2. deal with the unit-function: (1.0 2.3 4.1 TRUE)
            ListExpr second = nl->Second( instance );
            if( nl->ListLength( second ) == 4 &&
                nl->IsAtom( nl->First( second ) ) &&
                nl->AtomType( nl->First( second ) ) == RealType &&
                nl->IsAtom( nl->Second( second ) ) &&
                nl->AtomType( nl->Second( second ) ) == RealType &&
                nl->IsAtom( nl->Third( second ) ) &&
                nl->AtomType( nl->Third( second ) ) == RealType &&
                nl->IsAtom( nl->Fourth( second ) ) &&
                nl->AtomType( nl->Fourth( second ) ) == BoolType )
            {
                //3. create the class object
                correct = true;
                UReal *ureal = new UReal( tinterval,
                                          nl->RealValue( nl->First( second ) ),
                                          nl->RealValue( nl->Second( second ) ),
                                          nl->RealValue( nl->Third( second ) ),
                                          nl->BoolValue( nl->Fourth( second ) ) );
                return SetWord( ureal );
            }
            else
            {
                correct = false;
                return SetWord( Address(0) );
            }

        }
        else
        {
            correct = false;
            return SetWord( Address(0) );
        }
    }
    correct = false;
    return SetWord( Address(0) );
}

/*
5.4.3 ~Create~-function

*/
Word CreateUreal( const ListExpr typeInfo )
{
  return (SetWord( new UReal() ));
}

/*
5.4.4 ~Delete~-function

*/
void DeleteUreal( Word& w )
{
  delete (UReal *)w.addr;
  w.addr = 0;
}

/*
5.4.5 ~Close~-function

*/
void CloseUreal( Word& w )
{
  delete (UReal *)w.addr;
  w.addr = 0;
}

/*
5.4.6 ~Clone~-function

*/
Word CloneUreal( const Word& w )
{
  UReal *ureal = (UReal *)w.addr;
  return SetWord( new UReal( *ureal ) );
}

/*
5.4.7 ~Sizeof~-function

*/
int SizeOfUreal()
{
  return sizeof(UReal);
}

/*
5.4.8 ~Cast~-function

*/
void* CastUreal(void* addr)
{
  return new (addr) UReal;
}

/*
11.4 Creation of the type constructor ~ureal~

*/
TypeConstructor ureal(
        "ureal",                   	  	    	       //name
        UrealProperty,                 	     	      //property function describing signature
        OutUreal, InUreal,			     //Out and In functions
        0,                      0,           	     	    	     //SaveToList and RestoreFromList functions
        CreateUreal,
        DeleteUreal,			  	    //object creation and deletion
        0,                      0,            	        	    // object open and save
        CloseUreal,   CloneUreal,	   	    //object close and clone
        CastUreal,			       	    //cast function
        SizeOfUreal,			 	    //sizeof function
        CheckUreal,                        	  	    //kind checking function
        0,                                           	      	   //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
12 Type Constructor ~upoint~

Type ~upoint~ represents an (tinterval, (x0, y0, x1, y1))-pair. 

12.1 List Representation

The list representation of an ~upoint~ is

----    ( timeinterval (x0 yo x1 y1)) where x0, x1, y0, y1 are real numbers
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   (1.0 2.3 4.1 2.1) )
----

12.2 function Describing the Signature of the Type Constructor

*/
ListExpr
UPointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("(upoint) "),
                             nl->StringAtom("( timeInterval (real1 real2 real3 real4) ) "),
                             nl->StringAtom("( (6.37 9.9 T F) (1.0 2.2 2.5 2.1) )"))));
}

/*
12.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. 

*/
bool
CheckUPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "upoint" ));
}

/*
5.5.1 ~Out~-function

*/
ListExpr OutUPoint( ListExpr typeInfo, Word value )
{
  //1.get the address of the object and have a class object
  UPoint* upoint = (UPoint*)(value.addr);

  //2.output the time interval -> NL
  ListExpr timeintervalList = nl->FourElemList(
          OutDateTime( nl->TheEmptyList(), SetWord(&upoint->timeInterval.start) ),
          //nl->RealAtom( upoint->timeInterval.start.GetRealval()),
          OutDateTime( nl->TheEmptyList(), SetWord(&upoint->timeInterval.end) ),
          //nl->RealAtom( upoint->timeInterval.end.GetRealval()),
          nl->BoolAtom( upoint->timeInterval.lc ),
          nl->BoolAtom( upoint->timeInterval.rc));

  //3. get the real function NL (x0 x1 y0 y1)
  ListExpr pointfunList = nl->FourElemList(
          nl->RealAtom( upoint->x0),
          nl->RealAtom( upoint->y0),
          nl->RealAtom( upoint->x1),
          nl->RealAtom( upoint->y1));

  //4. return the final result
  return nl->TwoElemList(timeintervalList, pointfunList );
}

/*
5.5.2 ~In~-function

the Nested list form is like this:  ( ( 6.37  9.9  TRUE FALSE)   (1.0 2.3 4.1 2.1) )

*/
Word InUPoint( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
    if ( nl->ListLength( instance ) == 2 )
    {
        //1. deal with the time interval  ( 6.37  9.9  T F) or ((instant 2.0) (instant 3.0) T F)
        ListExpr first = nl->First( instance );

        if( nl->ListLength( first ) == 4 &&
            //nl->IsAtom( nl->First( first ) ) &&
            //nl->IsAtom( nl->Second( first ) ) &&
            nl->IsAtom( nl->Third( first ) ) &&
            nl->AtomType( nl->Third( first ) ) == BoolType &&
            nl->IsAtom( nl->Fourth( first ) ) &&
            nl->AtomType( nl->Fourth( first ) ) == BoolType )
        {
            correct = true;
            Instant *start = (Instant *)InInstant(
                    nl->TheEmptyList(),
                    nl->First( first ),
                    errorPos, errorInfo, correct ).addr;
            //Instant *start = new Instant ( true, nl->RealValue( nl->First( first ) ) );

            if( correct == false )
            {
                return SetWord( Address(0) );
            }

            Instant *end = (Instant *)InInstant(
                    nl->TheEmptyList(),
                    nl->Second( first ),
                    errorPos, errorInfo, correct ).addr;
            //Instant *end = new Instant ( true, nl->RealValue( nl->Second( first ) ) );

            if( correct == false )
            {
                delete start;
                return SetWord( Address(0) );
            }

            Interval<Instant> tinterval( *start, *end,
                         nl->BoolValue( nl->Third( first ) ),
                         nl->BoolValue( nl->Fourth( first ) ) );

            delete start;
            delete end;

            //2. deal with the unit-function: (1.0 2.3 4.1 2.1)
            ListExpr second = nl->Second( instance );
            if( nl->ListLength( second ) == 4 &&
                nl->IsAtom( nl->First( second ) ) &&
                nl->AtomType( nl->First( second ) ) == RealType &&
                nl->IsAtom( nl->Second( second ) ) &&
                nl->AtomType( nl->Second( second ) ) == RealType &&
                nl->IsAtom( nl->Third( second ) ) &&
                nl->AtomType( nl->Third( second ) ) == RealType &&
                nl->IsAtom( nl->Fourth( second ) ) &&
                nl->AtomType( nl->Fourth( second ) ) == RealType )
            {
                //3. create the class object
                correct = true;
                UPoint *upoint = new UPoint( tinterval,
                                          nl->RealValue( nl->First( second ) ),
                                          nl->RealValue( nl->Second( second ) ),
                                          nl->RealValue( nl->Third( second ) ),
                                          nl->RealValue( nl->Fourth( second ) ) );
                return SetWord( upoint );
            }
            else
            {
                correct = false;
                return SetWord( Address(0) );
            }
        }
        else
        {
            correct = false;
            return SetWord( Address(0) );
        }
    }
    correct = false;
    return SetWord( Address(0) );
}

/*
5.5.3 ~Create~-function

*/
Word CreateUPoint( const ListExpr typeInfo )
{
  return (SetWord( new UPoint() ));
}

/*
5.5.4 ~Delete~-function

*/
void DeleteUPoint( Word& w )
{
  delete (UPoint *)w.addr;
  w.addr = 0;
}

/*
5.5.5 ~Close~-function

*/
void CloseUPoint( Word& w )
{
  delete (UPoint *)w.addr;
  w.addr = 0;
}

/*
5.5.6 ~Clone~-function

*/
Word CloneUPoint( const Word& w )
{
  UPoint *upoint = (UPoint *)w.addr;
  return SetWord( new UPoint( *upoint ) );
}

/*
5.5.7 ~Sizeof~-function

*/
int SizeOfUPoint()
{
  return sizeof(UPoint);
}

/*
5.5.8 ~Cast~-function

*/
void* CastUPoint(void* addr)
{
  return new (addr) UPoint;
}

/*
12.4 Creation of the type constructor ~upoint~

*/
TypeConstructor upoint(
        "upoint",                   	  	      	       //name
        UPointProperty,                 	     	      //property function describing signature
        OutUPoint, InUPoint,			     //Out and In functions
        0,                      0,           	     	    	     //SaveToList and RestoreFromList functions
        CreateUPoint,
        DeleteUPoint,		  	    //object creation and deletion
        0,                      0,            	        	    // object open and save
        CloseUPoint,   CloneUPoint,	   	    //object close and clone
        CastUPoint,			       	    //cast function
        SizeOfUPoint,			    //sizeof function
        CheckUPoint,                        	  	    //kind checking function
        0,                                           	      	   //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
13 Type Constructor ~mpoint~

Type ~upoint~ represents an (tinterval, (x0, x1, y0, y1))-pair. 

13.1 List Representation

The list representation of an ~upoint~ is

----    ( timeinterval (x0 x1 y0 y1)) where x0, x1, y0, y1 are real numbers
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   (1.0 2.3 4.1 2.1) )
----

13.2 function Describing the Signature of the Type Constructor

*/
ListExpr
MPointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mpoint) "),
                             nl->StringAtom("( upoint1 upoint2 ... upointn) "),
                             nl->StringAtom("( ((6.37 9.9 T F) (1.0 2.2 2.5 2.1)) ...)"))));
}

/*
13.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. 

*/
bool
CheckMPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "mpoint" ));
}

/*
5.6.1 ~Out~-function

*/
ListExpr OutMPoint( ListExpr typeInfo, Word value )
{
    MPoint* mpoint = (MPoint*)(value.addr);

    if( mpoint->IsEmpty() )
    {
        return (nl->TheEmptyList());
    }
    else
    {
        assert( mpoint->IsOrdered() );
        ListExpr l = nl->TheEmptyList(), lastElem, upointList;

        for( int i = 0; i < mpoint->GetNoComponents(); i++ )
        {
            UPoint unit;
            mpoint->Get( i, unit );
            upointList = OutUPoint( nl->TheEmptyList(), SetWord(&unit) );
            if (l == nl->TheEmptyList())
            {
                l = nl->Cons( upointList, nl->TheEmptyList());
                lastElem = l;
            }
            else
                lastElem = nl->Append(lastElem, upointList);
        }
        return l;
    }
}

/*
5.6.2 ~In~-function

*/

Word InMPoint( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
    MPoint* mpoint = new MPoint( 0 );
    mpoint->StartBulkLoad();

    ListExpr rest = instance;
    while( !nl->IsEmpty( rest ) )
    {
        ListExpr first = nl->First( rest );
        rest = nl->Rest( rest );

        UPoint *upoint = (UPoint*)InUPoint(
                nl->TheEmptyList(),
                first,
                errorPos, errorInfo, correct ).addr;
        if( correct == false ) return SetWord( Address(0) );
        mpoint->Add( *upoint );
    }
    mpoint->EndBulkLoad( true );
    if (mpoint->IsValid())
    {
        correct = true;
        return SetWord( mpoint );
    }
    else
    {
        correct = false;
        return SetWord( 0 );
    }
}

/*
5.6.3 ~Open~-function

*/
bool OpenMPoint( SmiRecord& valueRecord, const ListExpr typeInfo, Word& value )
{
  MPoint *mpoint = new MPoint( 0 );

  mpoint->Open( valueRecord, typeInfo );

  value = SetWord( mpoint );
  return true;
}

/*
5.6.4 ~Save~-function

*/
bool SaveMPoint( SmiRecord& valueRecord, const ListExpr typeInfo, Word& value )
{
  MPoint *mpoint = (MPoint *)value.addr;

  mpoint->Save( valueRecord, typeInfo );

  return true;
}

/*
5.6.5 ~Create~-function

*/
Word CreateMPoint( const ListExpr typeInfo )
{
  return (SetWord( new MPoint( 0 ) ));
}

/*
5.6.6 ~Delete~-function

*/
void DeleteMPoint( Word& w )
{
  ((MPoint *)w.addr)->Destroy();
  delete (MPoint *)w.addr;
  w.addr = 0;
}

/*
5.6.7 ~Close~-function

*/
void CloseMPoint( Word& w )
{
  delete (MPoint *)w.addr;
  w.addr = 0;
}

/*
5.6.8 ~Clone~-function

*/
Word CloneMPoint( const Word& w )
{
  MPoint *r = (MPoint *)w.addr;
  return SetWord( r->Clone() );
}

/*
5.6.9 ~Sizeof~-function

*/
int SizeOfMPoint()
{
  return sizeof(MPoint);
}

/*
5.6.10 ~Cast~-function

*/
void* CastMPoint(void* addr)
{
  return new (addr) MPoint;
}

/*
13.4 Creation of the type constructor ~mpoint~

*/
TypeConstructor mpoint(
        "mpoint",                   	  	      	       //name
        MPointProperty,                 	     	      //property function describing signature
        OutMPoint, InMPoint,			     //Out and In functions
        0,                      0,           	     	    	     //SaveToList and RestoreFromList functions
        CreateMPoint,
        DeleteMPoint,		  	    //object creation and deletion
        0,                      0,            	        	    // object open and save
        CloseMPoint,   CloneMPoint,	   	    //object close and clone
        CastMPoint,			       	    //cast function
        SizeOfMPoint,			 	    //sizeof function
        CheckMPoint,                        	  	    //kind checking function
        0,                                           	      	   //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
14 Type Constructor ~mint~

14.1 List Representation

The list representation of an ~upoint~ is

----    ( timeinterval (x0 x1 y0 y1)) where x0, x1, y0, y1 are real numbers
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   (1.0 2.3 4.1 2.1) )
----

14.2 function Describing the Signature of the Type Constructor

*/
ListExpr
MIntProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mint) "),
                             nl->StringAtom("( constTempUnitInt1 ... constTempUnitIntn)"),
                             nl->StringAtom("(((6.3 8.9 T T) 1) ((8.9 9.9 T F) 2)...)"))));
}

/*
14.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. 

*/
bool
CheckMInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "mint" ));
}

/*
5.7.1 ~Out~-function

*/
ListExpr OutMInt( ListExpr typeInfo, Word value )
{
    MInt* mint = (MInt*)(value.addr);

    if( mint->IsEmpty() )
    {
        return (nl->TheEmptyList());
    }
    else
    {
        assert( mint->IsOrdered() );
        ListExpr l = nl->TheEmptyList(), lastElem, uintList;

        for( int i = 0; i < mint->GetNoComponents(); i++ )
        {
            ConstTemporalUnit<CcInt> unit;
            mint->Get( i, unit );
            uintList = OutConstTemporalUnit<CcInt, OutCcInt>(
                    nl->TheEmptyList(),
                    SetWord(&unit) );

            if (l == nl->TheEmptyList())
            {
                l = nl->Cons( uintList, nl->TheEmptyList());
                lastElem = l;
            }
            else
                lastElem = nl->Append(lastElem, uintList);
        }
        return l;
    }
}

/*
5.7.2 ~In~-function

*/

Word InMInt( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
    MInt* mint = new MInt( 0 );
    mint->StartBulkLoad();

    ListExpr rest = instance;
    while( !nl->IsEmpty( rest ) )
    {
        ListExpr first = nl->First( rest );
        rest = nl->Rest( rest );

        ConstTemporalUnit<CcInt> *constuint =
                (ConstTemporalUnit<CcInt>*)
                InConstTemporalUnit<CcInt, InCcInt>(
                        nl->TheEmptyList(),
                        first,
                        errorPos, errorInfo, correct ).addr;
        if( correct == false ) return SetWord( Address(0) );

        mint->Add( *constuint );
    }
    mint->EndBulkLoad( true );
    if (mint->IsValid())
    {
        correct = true;
        return SetWord( mint );
    }
    else
    {
        correct = false;
        return SetWord( 0 );
    }
}

/*
5.7.3 ~Open~-function

*/
bool OpenMInt( SmiRecord& valueRecord, const ListExpr typeInfo, Word& value )
{
  MInt *mint = new MInt( 0 );

  mint->Open( valueRecord, typeInfo );

  value = SetWord( mint );
  return true;
}

/*
5.7.4 ~Save~-function

*/
bool SaveMInt( SmiRecord& valueRecord, const ListExpr typeInfo, Word& value )
{
  MInt *mint = (MInt *)value.addr;

  mint->Save( valueRecord, typeInfo );

  return true;
}

/*
5.7.5 ~Create~-function

*/
Word CreateMInt( const ListExpr typeInfo )
{
  return (SetWord( new MInt( 0 ) ));
}

/*
5.7.6 ~Delete~-function

*/
void DeleteMInt( Word& w )
{
  ((MInt *)w.addr)->Destroy();
  delete (MInt *)w.addr;
  w.addr = 0;
}

/*
5.7.7 ~Close~-function

*/
void CloseMInt( Word& w )
{
  delete (MInt *)w.addr;
  w.addr = 0;
}

/*
5.7.8 ~Clone~-function

*/
Word CloneMInt( const Word& w )
{
  MInt *r = (MInt *)w.addr;
  return SetWord( r->Clone() );
}

/*
5.7.9 ~Sizeof~-function

*/
int SizeOfMInt()
{
  return sizeof(MInt);
}

/*
5.7.10 ~Cast~-function

*/
void* CastMInt(void* addr)
{
  return new (addr) MInt;
}

/*
14.4 Creation of the type constructor ~mint~

*/
TypeConstructor mint(
        "mint",                   	  	      	       //name
        MIntProperty,                 	     	      //property function describing signature
        OutMInt, InMInt,			     //Out and In functions
        0,                      0,           	     	    	     //SaveToList and RestoreFromList functions
        CreateMInt,
        DeleteMInt,		  	    	    //object creation and deletion
        0,                      0,            	        	    // object open and save
        CloseMInt,   CloneMInt,	   	    //object close and clone
        CastMInt,			       	    //cast function
        SizeOfMInt,			 	    //sizeof function
        CheckMInt,                        	  	    //kind checking function
        0,                                           	      	   //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
15 Type Constructor ~mreal~

Type ~upoint~ represents an (tinterval, (x0, x1, y0, y1))-pair. 

15.1 List Representation

The list representation of an ~upoint~ is

----    ( timeinterval (x0 x1 y0 y1)) where x0, x1, y0, y1 are real numbers
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   (1.0 2.3 4.1 2.1) )
----

15.2 function Describing the Signature of the Type Constructor

*/
ListExpr
MRealProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mreal) "),
                             nl->StringAtom("( ureal1 ureal2 ... urealn) "),
                             nl->StringAtom("( ((6.37 9.9 T F) (1.0 2.2 2.5 F)) ...)"))));
}

/*
15.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. 

*/
bool
CheckMReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "mreal" ));
}

/*
5.8.1 ~Out~-function

*/
ListExpr OutMReal( ListExpr typeInfo, Word value )
{
    MReal* mreal = (MReal*)(value.addr);

    if( mreal->IsEmpty() )
    {
        return (nl->TheEmptyList());
    }
    else
    {
        assert( mreal->IsOrdered() );
        ListExpr l = nl->TheEmptyList(), lastElem, urealList;

        for( int i = 0; i < mreal->GetNoComponents(); i++ )
        {
            UReal unit;
            mreal->Get( i, unit );
            urealList = OutUreal( nl->TheEmptyList(), SetWord(&unit) );
            if (l == nl->TheEmptyList())
            {
                l = nl->Cons( urealList, nl->TheEmptyList());
                lastElem = l;
            }
            else
                lastElem = nl->Append(lastElem, urealList);
        }
        return l;
    }
}

/*
5.8.2 ~In~-function

*/

Word InMReal( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
    MReal* mreal = new MReal( 0 );
    mreal->StartBulkLoad();

    ListExpr rest = instance;
    while( !nl->IsEmpty( rest ) )
    {
        ListExpr first = nl->First( rest );
        rest = nl->Rest( rest );

        UReal *ureal = (UReal*)InUreal(
                nl->TheEmptyList(),
                first,
                errorPos, errorInfo, correct ).addr;
        if( correct == false ) return SetWord( Address(0) );

        mreal->Add( *ureal );
    }

    mreal->EndBulkLoad( true );
    if (mreal->IsValid())
    {
        correct = true;
        return SetWord( mreal );
    }
    else
    {
        correct = false;
        return SetWord( 0 );
     }
}

/*
5.8.3 ~Open~-function

*/
bool OpenMReal( SmiRecord& valueRecord, const ListExpr typeInfo, Word& value )
{
  MReal *mreal = new MReal( 0 );

  mreal->Open( valueRecord, typeInfo );

  value = SetWord( mreal );
  return true;
}

/*
5.8.4 ~Save~-function

*/
bool SaveMReal( SmiRecord& valueRecord, const ListExpr typeInfo, Word& value )
{
  MReal *mreal = (MReal *)value.addr;

  mreal->Save( valueRecord, typeInfo );

  return true;
}

/*
5.8.5 ~Create~-function

*/
Word CreateMReal( const ListExpr typeInfo )
{
  return (SetWord( new MReal( 0 ) ));
}

/*
5.8.6 ~Delete~-function

*/
void DeleteMReal( Word& w )
{
  ((MReal *)w.addr)->Destroy();
  delete (MReal *)w.addr;
  w.addr = 0;
}

/*
5.8.7 ~Close~-function

*/
void CloseMReal( Word& w )
{
  delete (MReal *)w.addr;
  w.addr = 0;
}

/*
5.8.8 ~Clone~-function

*/
Word CloneMReal( const Word& w )
{
  MReal *r = (MReal *)w.addr;
  return SetWord( r->Clone() );
}

/*
5.8.9 ~Sizeof~-function

*/
int SizeOfMReal()
{
  return sizeof(MReal);
}

/*
5.8.10 ~Cast~-function

*/
void* CastMReal(void* addr)
{
  return new (addr) MReal;
}

/*
15.4 Creation of the type constructor ~constint~

*/
TypeConstructor mreal(
        "mreal",                   	  	      	       //name
        MRealProperty,                 	     	      //property function describing signature
        OutMReal, InMReal,			     //Out and In functions
        0,                      0,           	     	    	     //SaveToList and RestoreFromList functions
        CreateMReal,
        DeleteMReal,		  	    //object creation and deletion
        0,                      0,            	        	    // object open and save
        CloseMReal,   CloneMReal,	   	    //object close and clone
        CastMReal,			       	    //cast function
        SizeOfMReal,			 	    //sizeof function
        CheckMReal,                        	  	    //kind checking function
        0,                                           	      	   //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
16 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

16.1 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

16.1.1 Typa mapping function InstantTypeMapBool

*/
ListExpr
InstantTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if( nl->IsAtom( arg1 ) &&
        nl->AtomType( arg1 ) == SymbolType &&
        nl->SymbolValue( arg1 ) == "instant" )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.1 Typa mapping function InstantInstantTypeMapBool

*/
ListExpr
InstantInstantTypeMapBool( ListExpr args )
{
    ListExpr arg1, arg2;
    if ( nl->ListLength( args ) == 2 )
    {
	arg1 = nl->First( args );
	arg2 = nl->Second( args );
	if( nl->IsAtom( arg1 ) &&
	    nl->AtomType( arg1 ) == SymbolType &&
	    nl->SymbolValue( arg1 ) == "instant" &&
	    nl->IsAtom( arg2 ) &&
	    nl->AtomType( arg2 ) == SymbolType &&
	    nl->SymbolValue( arg2 ) == "instant" )
	    return (nl->SymbolAtom( "bool" ));
    }
    return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.1 Type mapping function RangeTypeMapBool1

It is for the operator ~isempty~ which have a ~range~ as input and ~bool~ result type.

*/
bool IsRangeAtom( const ListExpr atom )
{
  if( nl->IsAtom( atom ) &&
      nl->AtomType( atom ) == SymbolType &&
      ( nl->SymbolValue( atom ) == "rangeint" ||
        nl->SymbolValue( atom ) == "rangereal" ||
        nl->SymbolValue( atom ) == "periods" ) )
    return true;
  return false;
}

bool IsOfRangeType( const ListExpr type, const ListExpr range )
{ 
  assert( IsRangeAtom( range ) );
  if( nl->IsAtom( type ) && nl->AtomType( type ) == SymbolType &&
      ( nl->SymbolValue( range ) == string("range") + nl->SymbolValue( type ) ||
        nl->SymbolValue( range ) == string("periods") && nl->SymbolValue( type ) == string("instant") ) )
    return true;
  return false;
}

ListExpr RangeBaseType( const ListExpr range )
{
  assert( IsRangeAtom( range ) );

  if( nl->SymbolValue( range ) == "rangeint" )
    return nl->SymbolAtom( "int" );
  else if( nl->SymbolValue( range ) == "rangereal" )
    return nl->SymbolAtom( "real" );
  else if( nl->SymbolValue( range ) == "periods" )
    return nl->SymbolAtom( "instant" );
  return nl->SymbolAtom( "typeerror" );
}

ListExpr
RangeTypeMapBool1( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( IsRangeAtom( arg1 ) )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.2 Type mapping function RangeRangeTypeMapBool

It is for the operators $=$, $\neq$, and ~intersects~ which have two
~ranges~ as input and ~bool~ result type.

*/
ListExpr
RangeRangeTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if( IsRangeAtom( arg1 ) &&
        IsRangeAtom( arg2 ) &&
        nl->IsEqual( arg1, nl->SymbolValue( arg2 ) ) )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.3 Type mapping function RangeBaseTypeMapBool1

It is for the operator ~inside~ which have two ~ranges~ as input or a
~BASE~ and a ~range~ in this order as arguments and ~bool~ as the result type.

*/
ListExpr
RangeBaseTypeMapBool1( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );
    if( IsRangeAtom( arg1 ) &&
        IsRangeAtom( arg2 ) &&
        nl->IsEqual( arg1, nl->SymbolValue( arg2 ) ) )
      return (nl->SymbolAtom( "bool" ));
    else if( IsRangeAtom( arg2 ) &&
             IsOfRangeType( arg1, arg2 ) )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.4 Type mapping function RangeBaseTypeMapBool2

It is for the operator ~before~ which have two ~ranges~ as input or a
~BASE~ and a ~range~ in any order as arguments and ~bool~ as the result type.

*/
ListExpr
RangeBaseTypeMapBool2( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );
    if( IsRangeAtom( arg1 ) &&
        IsRangeAtom( arg2 ) &&
        nl->IsEqual( arg1, nl->SymbolValue( arg2 ) ) )
      return (nl->SymbolAtom( "bool" ));
    else if( IsRangeAtom( arg2 ) &&
             IsOfRangeType( arg1, arg2 ) )
      return (nl->SymbolAtom( "bool" ));
    else if( IsRangeAtom( arg1 ) &&
             IsOfRangeType( arg2, arg1 ) )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.5 Type mapping function RangeRangeTypeMapRange

It is for the operators ~intersection~, ~union~, and ~minus~ which have two
~ranges~ as input and a ~range~ as result type.

*/
ListExpr
RangeRangeTypeMapRange( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if( IsRangeAtom( arg1 ) &&
        IsRangeAtom( arg2 ) &&
        nl->SymbolValue( arg1 ) == nl->SymbolValue( arg2 ) )
      return (nl->SymbolAtom( nl->SymbolValue( arg1 ) ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.6 Type mapping function RangeTypeMapBase

It is for the aggregate operators ~min~, ~max~, and ~avg~ which have one
~range~ as input and a ~BASE~ as result type.

*/
ListExpr
RangeTypeMapBase( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( IsRangeAtom( arg1 ) )
      return (RangeBaseType( arg1 ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.7 Type mapping function RangeTypeMapInt

It is for the ~no\_components~ operator which have one
~range~ as input and a ~int~ as result type.

*/
ListExpr
RangeTypeMapInt( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( IsRangeAtom( arg1 ) )
      return (nl->SymbolAtom( "int" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.6 Type mapping function IntimeTypeMapBase

It is for the operator ~val~.

*/
bool IsIntimeAtom( const ListExpr atom )
{
  if( nl->IsAtom( atom ) &&
      nl->AtomType( atom ) == SymbolType &&
      ( nl->SymbolValue( atom ) == "intimeint" ||
        nl->SymbolValue( atom ) == "intimereal"||
        nl->SymbolValue( atom ) == "intimepoint" ) )
    return true;
  return false;
}

ListExpr IntimeBaseType( const ListExpr range )
{
  assert( IsIntimeAtom( range ) );

  if( nl->SymbolValue( range ) == "intimeint" )
      return nl->SymbolAtom( "int" );
  else if( nl->SymbolValue( range ) == "intimereal" )
      return nl->SymbolAtom( "real" );
  else if( nl->SymbolValue( range ) == "intimepoint" )
      return nl->SymbolAtom( "point" );
  return nl->SymbolAtom( "typeerror" );
}

ListExpr
IntimeTypeMapBase( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( IsIntimeAtom( arg1 ) )
      return (IntimeBaseType( arg1 ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.6 Type mapping function IntimeTypeMapInstant

It is for the operator ~inst~.

*/
ListExpr
IntimeTypeMapInstant( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( IsIntimeAtom( arg1 ) )
      return (nl->SymbolAtom( "instant" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.7 Type mapping function Mapping x Instant -- Intime

It is for the operator ~atinstant~.

*/
ListExpr
MappingInstantTypeMapIntime( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mint" &&
        nl->IsAtom( arg2 ) && nl->AtomType( arg2 ) == SymbolType && nl->SymbolValue( arg2 ) == "instant" )
	return (nl->SymbolAtom( "intimeint" ));
    
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mreal" &&
        nl->IsAtom( arg2 ) && nl->AtomType( arg2 ) == SymbolType && nl->SymbolValue( arg2 ) == "instant" )
	return (nl->SymbolAtom( "intimereal" ));
    
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mpoint" &&
        nl->IsAtom( arg2 ) && nl->AtomType( arg2 ) == SymbolType && nl->SymbolValue( arg2 ) == "instant" )
	return (nl->SymbolAtom( "intimepoint" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.8 Type mapping function Mapping -- RangeReal (RangeInstant)

It is for the operator ~deftime~.

*/
ListExpr
MappingTypeMapRangeInstant( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    
    if    (( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mint" )||
           ( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mreal")||
           ( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mpoint"))
	return (nl->SymbolAtom( "periods" ));
  }
  
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.8 Type mapping function Mapping(point) -- line

It is for the operator ~trajectory~.

*/
ListExpr
MPointTypeMapLine( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    
    if    ( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mpoint" )
	return (nl->SymbolAtom( "line")); 
  }
  
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.9 Type mapping function Mapping x Instant -- Bool

It is for the operator ~present~.

*/
ListExpr
MappingInstantTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mint" &&
        nl->IsAtom( arg2 ) && nl->AtomType( arg2 ) == SymbolType && nl->SymbolValue( arg2 ) == "instant" )
	return (nl->SymbolAtom( "bool" ));
    
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mreal" &&
        nl->IsAtom( arg2 ) && nl->AtomType( arg2 ) == SymbolType && nl->SymbolValue( arg2 ) == "instant" )
	return (nl->SymbolAtom( "bool" ));
    
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mpoint" &&
        nl->IsAtom( arg2 ) && nl->AtomType( arg2 ) == SymbolType && nl->SymbolValue( arg2 ) == "instant" )
	return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.10 Type mapping function Mapping(a) x a -- Bool

It is for the operator ~passes~.

*/
ListExpr
MappingATypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mint" &&
        nl->IsAtom( arg2 ) && nl->AtomType( arg2 ) == SymbolType && nl->SymbolValue( arg2 ) == "int" )
	return (nl->SymbolAtom( "bool" ));

//    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mreal" &&
//        nl->IsAtom( arg2 ) && nl->AtomType( arg2 ) == SymbolType && nl->SymbolValue( arg2 ) == "real" )
//	return (nl->SymbolAtom( "bool" ));
    
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mpoint" &&
        nl->IsAtom( arg2 ) && nl->AtomType( arg2 ) == SymbolType && nl->SymbolValue( arg2 ) == "point" )
	return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.11 Type mapping function Mapping -- Intime

It is for the operators ~initial~ and ~final~.

*/
ListExpr
MappingTypeMapIntime( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mint")
	return (nl->SymbolAtom( "intimeint" ));
    
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mreal")
	return (nl->SymbolAtom( "intimereal" ));
    
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mpoint")
	return (nl->SymbolAtom( "intimepoint" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
16.1.12 Type Mapping Function for the Operstor ~units~ 

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol
~typeerror~.


Type mapping for ~units~ is

----	(mpoint) -> (stream upoint)
                (mint) -> (stream constint)
	(mreal) -> (stream ureal)
----

*/
ListExpr
MappingTypeMapUnits( ListExpr args ){
  ListExpr arg1;
  if ( nl->ListLength(args) == 1 )
  {
    arg1 = nl->First(args);
    //arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, "mpoint"))
	return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("upoint"));
    if ( nl->IsEqual(arg1, "mint"))
	return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("constint"));
    if ( nl->IsEqual(arg1, "mreal"))
	return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("ureal"));
  }
  return nl->SymbolAtom("typeerror");
}

/*
16.1.13 Type Mapping Function for the Operstor ~theyear~, ~themonth~,
~theday~,~thehour~,~theminute~,~thesecond~,~theperiod~

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol
~typeerror~.

*/
ListExpr
MappingTypeMapIntPeriods( ListExpr args ){
  ListExpr argi;
  bool correct=true;
  
  if (( nl->ListLength(args) < 1 )|| ( nl->ListLength(args) > 6))  
      return nl->SymbolAtom("typeerror");
  
  if ( nl->ListLength(args) >= 1 )
  {
      argi = nl->First(args);
      if (!( nl->IsEqual(argi, "int")) ) correct=false;
  }
    
  if ( nl->ListLength(args) >= 2 )
  {
      argi = nl->Second(args);
      if (!( nl->IsEqual(argi, "int")) ) correct=false;
  }
  
  if ( nl->ListLength(args) >= 3 )
  {
      argi = nl->Third(args);
      if (!( nl->IsEqual(argi, "int")) ) correct=false;
  }
    
  if ( nl->ListLength(args) >= 4 )
  {
      argi = nl->Fourth(args);
      if (!( nl->IsEqual(argi, "int")) ) correct=false;
  }
  
  if ( nl->ListLength(args) >= 5 )
  {
      argi = nl->Fifth(args);
      if (!( nl->IsEqual(argi, "int")) ) correct=false;
  }
  
  if ( nl->ListLength(args) >= 6 )
  {
      argi = nl->Sixth(args);
      if (!( nl->IsEqual(argi, "int")) ) correct=false;
  }
  
  if (correct) return (nl->SymbolAtom( "periods" ));
    
  return nl->SymbolAtom("typeerror");
}

ListExpr
MappingTypeMapPeriodsPeriods( ListExpr args ){
  ListExpr arg1, arg2;
  
  if ( nl->ListLength(args) ==2 )
  {
      arg1 = nl->First(args);
      arg2 = nl->Second(args);
      if (( nl->IsEqual(arg1, "periods")) &&( nl->IsEqual(arg2, "periods"))) 
	  return (nl->SymbolAtom( "periods" ));
  }

  return nl->SymbolAtom("typeerror");
}

/*
16.2 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

16.2.1 Selection function ~RangeSelectPredicates~

Is used for the ~inside~ and ~before~ operations.

*/
int
RangeSelectPredicates( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( IsRangeAtom( arg1 ) &&
      IsRangeAtom( arg2 ) &&
      nl->SymbolValue( arg1 ) == nl->SymbolValue( arg2 ) )
    return (0);

  if( IsRangeAtom( arg2 ) &&
      IsOfRangeType( arg1, arg2 ) )
    return (1);

  if( IsRangeAtom( arg1 ) &&
      IsOfRangeType( arg2, arg1 ) )
    return (2);

  assert( false );
  return (-1); // This point should never be reached
}

/*
16.2.2 Selection function ~TemporalSelectAtinstant~

Is used for the ~atinstant~ operations.

*/
int
TemporalSelectAtInstant( ListExpr args )
{ //also for present operator
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );
  
  if (nl->IsAtom( arg2 ) && nl->AtomType( arg2 ) == SymbolType && nl->SymbolValue( arg2 ) == "instant" )
  {
      if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mint" )
	  return (0);
  
      if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mreal" )
	  return (1);
  
      if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mpoint" )
	  return (2);
  }
  
  cout<<endl<<">>>currently the instant value can only be input with nested list format queries...eg. "<<endl;
  cout<<"(query (atinstant mb (instant 1.5)))<<<"<<endl<<endl;

  //assert( false );
  return (-1); // This point should never be reached
}

/*
16.2.2 Selection function ~TemporalSelectPasses~

Is used for the ~passes~ operations.

*/
int
TemporalSelectPasses( ListExpr args )
{ 
  ListExpr arg1 = nl->First( args );
  //ListExpr arg2 = nl->Second( args );
  
  if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mint" )
      return (0);
  
  if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mreal" )
      return (1);
  
  if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mpoint" )
      return (2);

  //assert( false );
  return (-1); // This point should never be reached
}

/*
16.2.3 Selection function ~TemporalSelectDeftime~

Is used for the ~deftime~ operations.

*/
int
TemporalSelectDeftime( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  
  if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mint" )
      return (0);
  
  if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mreal" )
      return (1);
  
  if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mpoint" )
      return (2);	
  
  return (-1); // This point should never be reached
}

/*
16.2.4 Selection function ~TemporalSelectInitial Final~

Is used for the ~passes~ operations.

*/
int
TemporalSelectInitialFinal( ListExpr args )
{ 
  ListExpr arg1 = nl->First( args );
  
  if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mint" )
      return (0);
  
  if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mreal" )
      return (1);
  
  if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mpoint" )
      return (2);

  //assert( false );
  return (-1); // This point should never be reached
}

/*
16.2.5 Selection function ~units~

Is used for the ~units~ operations.

*/

int
TemporalSelectUnits( ListExpr args )
{
    ListExpr arg1 = nl->First( args );
  
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mpoint" )
	return (0);
  
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mint" )
	return (1);
  
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "mreal" )
	return (2);	
  
    return (-1); // This point should never be reached
}

/*
16.2.6 Selection function for int-to-periods operations

Is used for the ~theyear~, ~themonth~, ... operations.

*/

int
TemporalSelectIntPeriods( ListExpr args )
{
    ListExpr arg1 = nl->First( args );
    
    int noArg = nl->ListLength(args);
	    
    if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "int" )
	return (noArg-1);
    else return (6);  //for the theperiod operation
  
    return (-1); // This point should never be reached
}

/*
16.3 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are
several value mapping functions, one for each possible combination of input
parameter types.

16.3.1 Value mapping functions of operator ~isempty~

*/
int InstantIsEmpty( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Instant*)args[0].addr)->IsDefined() )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

template <class Alpha>
int RangeIsEmpty_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range<Alpha>*)args[0].addr)->IsEmpty() )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
16.3.2 Value mapping functions of operator $=$ (~equal~)

*/
int
InstantEqual( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Instant*)args[0].addr)->IsDefined() &&
       ((Instant*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((Instant*)args[0].addr)->ToDouble() ==
                 ((Instant*)args[1].addr)->ToDouble() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

template <class Alpha>
int RangeEqual_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( *((Range<Alpha>*)args[0].addr) == *((Range<Alpha>*)args[1].addr) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }

  return (0);
}

/*
16.3.3 Value mapping functions of operator $\#$ (~not equal~)

*/
int
InstantNotEqual( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Instant*)args[0].addr)->IsDefined() &&
       ((Instant*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((Instant*)args[0].addr)->ToDouble() !=
                 ((Instant*)args[1].addr)->ToDouble() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

template <class Alpha>
int RangeNotEqual_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( *((Range<Alpha>*)args[0].addr) != *((Range<Alpha>*)args[1].addr) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
16.3.4 Value mapping functions of operator $<$

*/
int
InstantLess( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Instant*)args[0].addr)->IsDefined() &&
       ((Instant*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((Instant*)args[0].addr)->ToDouble() <
                 ((Instant*)args[1].addr)->ToDouble() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
16.3.5 Value mapping functions of operator $<=$

*/
int
InstantLessEqual( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Instant*)args[0].addr)->IsDefined() &&
       ((Instant*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((Instant*)args[0].addr)->ToDouble() <=
                 ((Instant*)args[1].addr)->ToDouble() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
16.3.6 Value mapping functions of operator $>$

*/
int
InstantGreater( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Instant*)args[0].addr)->IsDefined() &&
       ((Instant*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((Instant*)args[0].addr)->ToDouble() >
                 ((Instant*)args[1].addr)->ToDouble() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
16.3.7 Value mapping functions of operator $>=$

*/
int
InstantGreaterEqual( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Instant*)args[0].addr)->IsDefined() &&
       ((Instant*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((Instant*)args[0].addr)->ToDouble() >=
                 ((Instant*)args[1].addr)->ToDouble() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
16.3.8 Value mapping functions of operator ~intersects~

*/
template <class Alpha>
int RangeIntersects_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range<Alpha>*)args[0].addr)->Intersects( *((Range<Alpha>*)args[1].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
16.3.9 Value mapping functions of operator ~inside~

*/
template <class Alpha>
int RangeInside_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range<Alpha>*)args[0].addr)->Inside( *((Range<Alpha>*)args[1].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

template <class Alpha>
int RangeInside_ar( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range<Alpha>*)args[1].addr)->Contains( *((Alpha*)args[0].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
16.3.10 Value mapping functions of operator ~before~

*/
template <class Alpha>
int RangeBefore_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range<Alpha>*)args[0].addr)->Before( *((Range<Alpha>*)args[1].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

template <class Alpha>
int RangeBefore_ar( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range<Alpha>*)args[1].addr)->After( *((Alpha*)args[0].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

template <class Alpha>
int RangeBefore_ra( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range<Alpha>*)args[0].addr)->Before( *((Alpha*)args[1].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
16.3.11 Value mapping functions of operator ~intersection~

*/
template <class Alpha>
int RangeIntersection_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range<Alpha>*)args[0].addr)->Intersection( *((Range<Alpha>*)args[1].addr), (*(Range<Alpha>*)result.addr) );
  return (0);
}

/*
16.3.12 Value mapping functions of operator ~union~

*/
template <class Alpha>
int RangeUnion_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range<Alpha>*)args[0].addr)->Union( *((Range<Alpha>*)args[1].addr), (*(Range<Alpha>*)result.addr) );
  return (0);
}

/*
16.3.13 Value mapping functions of operator ~minus~

*/
template <class Alpha>
int RangeMinus_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range<Alpha>*)args[0].addr)->Minus( *((Range<Alpha>*)args[1].addr), (*(Range<Alpha>*)result.addr) );
  return (0);
}

/*
16.3.14 Value mapping functions of operator ~min~

*/
template <class Alpha>
int RangeMinimum_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if( ((Range<Alpha>*)args[0].addr)->IsEmpty() )
  {
    ((Alpha *)result.addr)->SetDefined( false );
  }
  else
  {
    ((Range<Alpha>*)args[0].addr)->Minimum( *(Alpha *)result.addr);
  }
  return (0);
}

/*
16.3.15 Value mapping functions of operator ~max~

*/
template <class Alpha>
int RangeMaximum_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if( ((Range<Alpha>*)args[0].addr)->IsEmpty() )
  {
    ((Alpha*)result.addr)->SetDefined( false );
  }
  else
  {
    ((Range<Alpha>*)args[0].addr)->Maximum( *(Alpha*)result.addr);
  }
  return (0);
}

/*
16.3.16 Value mapping functions of operator ~no\_components~

*/
template <class Alpha>
int RangeNoComponents_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcInt *)result.addr)->Set( true, ((Range<Alpha>*)args[0].addr)->GetNoComponents() );
  return (0);
}

/*
16.3.17 Value mapping functions of operator ~inst~

*/
template <class Alpha>
int IntimeInst( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Instant*)result.addr)->ReadFrom( ((Intime<Alpha>*)args[0].addr)->instant.ToDouble() );
  return (0);
}

/*
16.3.18 Value mapping functions of operator ~val~

*/
template <class Alpha>
int IntimeVal( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Alpha*)result.addr)->CopyFrom( &((Intime<Alpha>*)args[0].addr)->value );
  return (0);
}

/*
16.3.19 Value mapping functions of operator ~atinstant~

*/
int atinstant_mint( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint x instant  --> intime(int) note: Operators run based on class objects
  result = qp->ResultStorage( s );
  
  MInt *mint;
  Instant* inst;
  
  mint=((MInt*)args[0].addr);
  inst=((Instant*)args[1].addr);
  
  CcInt resInt;
  if (mint->TemporalFunction( *inst, resInt ))
  {
      ((Intime<CcInt>*)result.addr)->instant = *inst;
      ((Intime<CcInt>*)result.addr)->value = resInt;
      return (0);
  }
  else //not included in any units
  {
      ((Intime<CcInt>*)result.addr)->instant = *inst;
      ((Intime<CcInt>*)result.addr)->value.SetDefined(false);
      return (0);
  }
}

int atinstant_mreal( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint x instant  --> intime(int) note: Operators run based on class objects
  result = qp->ResultStorage( s );
  
  MReal *mreal;
  Instant* inst;
  
  mreal=((MReal*)args[0].addr);
  inst=((Instant*)args[1].addr);
  
  CcReal resReal;
  if (mreal->TemporalFunction( *inst, resReal ))
  {
      ((Intime<CcReal>*)result.addr)->instant = *inst;
      ((Intime<CcReal>*)result.addr)->value = resReal;
      return (0);
  }
  else  //not included in any unit
  {
      ((Intime<CcReal>*)result.addr)->instant = *inst;
      ((Intime<CcReal>*)result.addr)->value.SetDefined(false);
      return (0);
  }
}

int atinstant_mpoint( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint x instant  --> intime(int) note: Operators run based on class objects
  result = qp->ResultStorage( s );
  
  MPoint *mpoint;
  Instant* inst;
  
  mpoint=((MPoint*)args[0].addr);
  inst=((Instant*)args[1].addr);
  
  Point resPoint;
  if (mpoint->TemporalFunction( *inst, resPoint ))
  {
      ((Intime<Point>*)result.addr)->instant = *inst;
      ((Intime<Point>*)result.addr)->value = resPoint;
      return (0);
  }
  else //not included in any unit
  { 
      ((Intime<Point>*)result.addr)->instant = *inst;
      ((Intime<Point>*)result.addr)->value.SetDefined(false);
      return (0); 
  }
}

/*
16.3.20 Value mapping functions of operator ~deftime~

*/
int deftime_mint( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint --> periods (=range(instant))
  result = qp->ResultStorage( s );
  
  ((Range<Instant>*)result.addr)->Clear();
  
  //1.get the input and out put objects
  MInt *mint;
  
  Range<Instant> *defrange = new Range<Instant>( 0 );
  //Range<Instant>* defrange=((Range<Instant>*)result.addr);
  mint=((MInt*)args[0].addr);
  
  //2.get the timeintervals and add them to the result
  ConstTemporalUnit<CcInt> unit;
  
  defrange->Clear();
  defrange->StartBulkLoad();
  for( int i = 0; i < mint->GetNoComponents(); i++ )
  {
      mint->Get(i, unit );
      defrange->Add( unit.timeInterval ); 
  }
  defrange->EndBulkLoad( true );
  //cout<<"&&&&&&&&&&&&"<<endl;
  defrange->Merge(((Range<Instant>*)result.addr));
  
  return (0);
}

int deftime_mreal( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mreal --> periods (=range(instant))
  result = qp->ResultStorage( s );
  ((Range<Instant>*)result.addr)->Clear();
  
  //1.get the input and out put objects
  MReal *mreal;
  //Range<Instant>* defrange; 
  
  mreal=((MReal*)args[0].addr);
  //defrange=((Range<Instant>*)result.addr);
  Range<Instant> *defrange = new Range<Instant>( 0 );
  
  //2.get the timeintervals and add them to the result
  UReal unit;
  
  defrange->StartBulkLoad();
  for( int i = 0; i < mreal->GetNoComponents(); i++ )
  {
      mreal->Get(i, unit );
      defrange->Add( unit.timeInterval ); 
  }
  defrange->EndBulkLoad( true );
  
  defrange->Merge(((Range<Instant>*)result.addr));
  return (0);
}

int deftime_mpoint( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mpoint --> periods (=range(instant))
  result = qp->ResultStorage( s );
  ((Range<Instant>*)result.addr)->Clear();
  
  //1.get the input and out put objects
  MPoint *mpoint;
  //Range<Instant>* defrange; 
  
  mpoint=((MPoint*)args[0].addr);
  //defrange=((Range<Instant>*)result.addr);
  Range<Instant> *defrange = new Range<Instant>( 0 );
  
  //2.get the timeintervals and add them to the result
  UPoint unit;
  
  defrange->StartBulkLoad();
  for( int i = 0; i < mpoint->GetNoComponents(); i++ )
  {
      mpoint->Get(i, unit );
      defrange->Add( unit.timeInterval ); 
  }
  defrange->EndBulkLoad( true );
  
  defrange->Merge(((Range<Instant>*)result.addr));
  return (0);
}

/*
16.3.21 Value mapping functions of operator ~trajectory~

*/
int trajectory_mp( Word* args, Word& result, int message, Word& local, Supplier s )
{ // moving(point) --> line
  result = qp->ResultStorage( s );
  ((CLine *)result.addr)->Clear();
  
  //1.get the input and out put objects
  MPoint *mpoint;
  CLine *line; 
  CHalfSegment reschs;
  Point p1, p2;
  
  mpoint=((MPoint*)args[0].addr);
  line=((CLine*)result.addr);
  
  //2.get the halfsegment and add it to the result
  UPoint unit;

  line->Clear();  //this line should not be forgotten...
  line->StartBulkLoad();
  for( int i = 0; i < mpoint->GetNoComponents(); i++ )
  {
      mpoint->Get(i, unit );
      
      //3. add the segment to the line value.
      p1.Set(unit.x0, unit.y0); 
      p2.Set(unit.x1, unit.y1);
      reschs.Set(true, p1, p2);
      
      *((CLine *)result.addr) += reschs;
      reschs.SetLDP(false);
      *((CLine *)result.addr) += reschs;
  }
  line->EndBulkLoad( );
  return (0);
}

/*
16.3.22 Value mapping functions of operator ~present~

*/
int present_mint( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint x instant  --> bool
  result = qp->ResultStorage( s );
  
  MInt *mint;
  Instant* inst;
  
  ConstTemporalUnit<CcInt> unit;
  
  mint=((MInt*)args[0].addr);
  inst=((Instant*)args[1].addr);

  for( int i = 0; i < mint->GetNoComponents(); i++ )
  {
      mint->Get(i, unit );
      
      if (unit.timeInterval.Contains(*inst)) 
      {
	  ((CcBool *)result.addr)->Set( true, true);
	  return (0);
      }
  }
  ((CcBool *)result.addr)->Set( true, false );
  return (0);
}

int present_mreal( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint x instant  --> bool
  result = qp->ResultStorage( s );
  
  MReal *mreal;
  Instant* inst;
  
  UReal unit;
  
  mreal=((MReal*)args[0].addr);
  inst=((Instant*)args[1].addr);

  for( int i = 0; i < mreal->GetNoComponents(); i++ )
  {
      mreal->Get(i, unit );
      
      if (unit.timeInterval.Contains(*inst)) 
      {
	  ((CcBool *)result.addr)->Set( true, true);
	  return (0);
      }
  }
  ((CcBool *)result.addr)->Set( true, false );
  return (0);
}

int present_mpoint( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint x instant  --> bool
  result = qp->ResultStorage( s );
  
  MPoint *mpoint;
  Instant* inst;
  
  UPoint unit;
  
  mpoint=((MPoint*)args[0].addr);
  inst=((Instant*)args[1].addr);

  for( int i = 0; i < mpoint->GetNoComponents(); i++ )
  {
      mpoint->Get(i, unit );
      
      if (unit.timeInterval.Contains(*inst)) 
      {
	  ((CcBool *)result.addr)->Set( true, true);
	  return (0);
      }
  }
  ((CcBool *)result.addr)->Set( true, false );
  return (0);
}

/*
16.3.22 Value mapping functions of operator ~passes~

*/
int passes_mint( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint x int  --> bool
  result = qp->ResultStorage( s );
  
  MInt *mint;
  CcInt* val;
  
  ConstTemporalUnit<CcInt> unit;
  
  mint=((MInt*)args[0].addr);
  val=((CcInt*)args[1].addr);

  for( int i = 0; i < mint->GetNoComponents(); i++ )
  {
      mint->Get(i, unit );
      
      if (unit.constValue.GetIntval()==val->GetIntval()) 
      {
	  ((CcBool *)result.addr)->Set( true, true);
	  return (0);
      }
  }
  ((CcBool *)result.addr)->Set( true, false );
  return (0);
}

int passes_mreal( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint x int  --> bool
  result = qp->ResultStorage( s );
  
  MReal *mreal;
  CcReal* val;
  
  UReal unit;
  
  mreal=((MReal*)args[0].addr);
  val=((CcReal*)args[1].addr);

  for( int i = 0; i < mreal->GetNoComponents(); i++ )
  {
      mreal->Get(i, unit );
      
      //if (unit.constValue.GetIntval()==val->GetIntval()) 
      {  
	  ((CcBool *)result.addr)->Set( true, true);
	  return (0);
      }
  }
  ((CcBool *)result.addr)->Set( true, false );
  return (0);
}

int passes_mpoint( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mpoint x point  --> bool
  result = qp->ResultStorage( s );
  
  MPoint *mpoint;
  Point* val;
    
  UPoint unit;
  CHalfSegment reschs;
  Point p1, p2;
  
  mpoint=((MPoint*)args[0].addr);
  val=((Point*)args[1].addr);

  for( int i = 0; i < mpoint->GetNoComponents(); i++ )
  {
      mpoint->Get(i, unit );
      p1.Set(unit.x0, unit.y0); 
      p2.Set(unit.x1, unit.y1);
      reschs.Set(true, p1, p2);
      
      if (reschs.Contains(*val)) 
      {  
	  ((CcBool *)result.addr)->Set( true, true);
	  return (0);
      }
  }
  ((CcBool *)result.addr)->Set( true, false );
  return (0);
}

/*
16.3.23 Value mapping functions of operator ~initial~

*/
int initial_mint( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint  --> intime(int) note: Operators run based on class objects
  result = qp->ResultStorage( s );
  
  MInt *mint;
  Instant inst;
  
  ConstTemporalUnit<CcInt> unit;
  mint=((MInt*)args[0].addr);
  
  //  inst=((Instant*)args[1].addr);
  if ((mint->GetNoComponents() <=0)||(!(mint->IsOrdered()))) return (0);
  mint->Get(0, unit );
  inst.CopyFrom(&unit.timeInterval.start);
  
  CcInt resInt;
  if (mint->TemporalFunction( inst, resInt ))
  {
      ((Intime<CcInt>*)result.addr)->instant = inst;
      ((Intime<CcInt>*)result.addr)->value = resInt;
      return (0);
  }
  else //not included in any units
  {
      ((Intime<CcInt>*)result.addr)->instant = inst;
      ((Intime<CcInt>*)result.addr)->value.SetDefined(false);
      return (0);
  }
}

int initial_mreal( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint  --> intime(int) note: Operators run based on class objects
  result = qp->ResultStorage( s );
  
  MReal *mreal;
  Instant inst;
  
  UReal unit;
  
  mreal=((MReal*)args[0].addr);
  
  //  inst=((Instant*)args[1].addr);
  if ((mreal->GetNoComponents() <=0)||(!mreal->IsOrdered())) return (0);
  mreal->Get(0, unit );
  inst.CopyFrom(&unit.timeInterval.start);
  
  CcReal resReal;
  if (mreal->TemporalFunction( inst, resReal ))
  {
      ((Intime<CcReal>*)result.addr)->instant = inst;
      ((Intime<CcReal>*)result.addr)->value = resReal;
      return (0);
  }
  else  //not included in any unit
  {
      ((Intime<CcReal>*)result.addr)->instant = inst;
      ((Intime<CcReal>*)result.addr)->value.SetDefined(false);
      return (0);
  }
}

int initial_mpoint( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint  --> intime(int) note: Operators run based on class objects
  result = qp->ResultStorage( s );
  
  MPoint *mpoint;
  Instant inst;
  UPoint unit;
  
  mpoint=((MPoint*)args[0].addr);
  
  //  inst=((Instant*)args[1].addr);
  if ((mpoint->GetNoComponents() <=0)||(!mpoint->IsOrdered())) return (0);
  mpoint->Get(0, unit );
  inst.CopyFrom(&unit.timeInterval.start);
  
  Point resPoint;
  if (mpoint->TemporalFunction( inst, resPoint ))
  {
      ((Intime<Point>*)result.addr)->instant = inst;
      ((Intime<Point>*)result.addr)->value = resPoint;
      return (0);
  }
  else //not included in any unit
  { 
      ((Intime<Point>*)result.addr)->instant = inst;
      ((Intime<Point>*)result.addr)->value.SetDefined(false);
      return (0); 
  }
}

/*
16.3.24 Value mapping functions of operator ~final~

*/
int final_mint( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint  --> intime(int) note: Operators run based on class objects
  result = qp->ResultStorage( s );
  
  MInt *mint;
  Instant inst;
  
  ConstTemporalUnit<CcInt> unit;
  mint=((MInt*)args[0].addr);
  
  if ((mint->GetNoComponents() <=0)||(!(mint->IsOrdered()))) return (0);
  mint->Get(mint->GetNoComponents()-1, unit );
  inst.CopyFrom(&unit.timeInterval.end);
  
  CcInt resInt;
  if (mint->TemporalFunction( inst, resInt ))
  {
      ((Intime<CcInt>*)result.addr)->instant = inst;
      ((Intime<CcInt>*)result.addr)->value = resInt;
      return (0);
  }
  else //not included in any units
  {
      ((Intime<CcInt>*)result.addr)->instant = inst;
      ((Intime<CcInt>*)result.addr)->value.SetDefined(false);
      return (0);
  }
}

int final_mreal( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint  --> intime(int) note: Operators run based on class objects
  result = qp->ResultStorage( s );
  
  MReal *mreal;
  Instant inst;
  
  UReal unit;
  
  mreal=((MReal*)args[0].addr);
  
  if ((mreal->GetNoComponents() <=0)||(!mreal->IsOrdered())) return (0);
  mreal->Get(mreal->GetNoComponents()-1, unit );
  inst.CopyFrom(&unit.timeInterval.end);
  
  CcReal resReal;
  if (mreal->TemporalFunction( inst, resReal ))
  {
      ((Intime<CcReal>*)result.addr)->instant = inst;
      ((Intime<CcReal>*)result.addr)->value = resReal;
      return (0);
  }
  else  //not included in any unit
  {
      ((Intime<CcReal>*)result.addr)->instant = inst;
      ((Intime<CcReal>*)result.addr)->value.SetDefined(false);
      return (0);
  }
}

int final_mpoint( Word* args, Word& result, int message, Word& local, Supplier s )
{ // mint  --> intime(int) note: Operators run based on class objects
  result = qp->ResultStorage( s );
  
  MPoint *mpoint;
  Instant inst;
  UPoint unit;
  
  mpoint=((MPoint*)args[0].addr);
  
  if ((mpoint->GetNoComponents() <=0)||(!mpoint->IsOrdered())) return (0);
  mpoint->Get(mpoint->GetNoComponents()-1, unit );
  inst.CopyFrom(&unit.timeInterval.end);
  
  Point resPoint;
  if (mpoint->TemporalFunction( inst, resPoint ))
  {
      ((Intime<Point>*)result.addr)->instant = inst;
      ((Intime<Point>*)result.addr)->value = resPoint;
      return (0);
  }
  else //not included in any unit
  { 
      ((Intime<Point>*)result.addr)->instant = inst;
      ((Intime<Point>*)result.addr)->value.SetDefined(false);
      return (0); 
  }
}

/*
16.3.25 Value mapping functions of operator ~units~

(mpoint) ---- (stream upoint)

(mint) ---- (stream constint)

(mreal) ---- (stream ureal)

*/

struct UnitsLocalInfo
{
  Word mpir;     //the address of the moving point/int/real value
  int unitIndex;  //current item index
};

int
units_mp (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Create upoint stream. Note that for any operator that produces a stream its arguments are NOT
evaluated automatically. To get the argument value, the value mapping function
needs to use ~qp->Request~ to ask the query processor for evaluation explicitly.
This is illustrated in the value mapping functions below.

*/
{
  Word arg0;  //address of the input value  (mpoint / mint / mreal)
  //Word tuplex, tupley, tuplexy, streamy;
  
  MPoint* mp;  //the corresponding class objects (input and output)
  UPoint* unit; 
    
  UnitsLocalInfo *localinfo;
  
  switch( message )
  {
    case OPEN:

      qp->Request(args[0].addr, arg0);

      mp = ((MPoint*)arg0.addr);  //receive the MPoint value

      localinfo = new UnitsLocalInfo;
      localinfo->mpir = arg0;
      localinfo->unitIndex = 0;
      local = SetWord(localinfo);
      
      return 0;

    case REQUEST:
      
      if (local.addr ==0) return CANCEL;
      localinfo=(UnitsLocalInfo *) local.addr;
      
      arg0 = localinfo->mpir;
      mp = (MPoint*)arg0.addr;   //recover from local info.
      
      if (( 0 <= localinfo->unitIndex )&&( localinfo->unitIndex < mp->GetNoComponents() ))
      {
	  unit = new UPoint;
	  mp->Get(localinfo->unitIndex++, *unit);
	  
	  //cout<<*unit<<endl;
	  result.addr = unit;
	  return YIELD;
      }
      else return CANCEL;

    case CLOSE:
      
      if( local.addr != 0 )
      {
	  localinfo=(UnitsLocalInfo *) local.addr;
	  delete localinfo;
      }
      
      return 0;
  }
  /* should not happen */
  return -1;
}

int
units_mi (Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word arg0;  //address of the input value  (mpoint / mint / mreal)
  //Word tuplex, tupley, tuplexy, streamy;
  
  MInt* mi;  //the corresponding class objects (input and output)
  ConstTemporalUnit<CcInt> *unit;
  
  UnitsLocalInfo *localinfo;
  
  switch( message )
  {
    case OPEN:

      qp->Request(args[0].addr, arg0);

      mi = ((MInt*)arg0.addr);  //receive the MPoint value

      localinfo = new UnitsLocalInfo;
      localinfo->mpir = arg0;
      localinfo->unitIndex = 0;
      local = SetWord(localinfo);
      
      return 0;

    case REQUEST:
      
      if (local.addr ==0) return CANCEL;
      localinfo=(UnitsLocalInfo *) local.addr;
      
      arg0 = localinfo->mpir;
      mi = (MInt*)arg0.addr;   //recover from local info.
      
      if (( 0 <= localinfo->unitIndex )&&( localinfo->unitIndex < mi->GetNoComponents() ))
      {
	  unit = new ConstTemporalUnit<CcInt>;
	  mi->Get(localinfo->unitIndex++, *unit);
	  
	  result.addr = unit;
	  return YIELD;
      }
      else return CANCEL;

    case CLOSE:
      
      if( local.addr != 0 )
      {
	  localinfo=(UnitsLocalInfo *) local.addr;
	  delete localinfo;
      }
      
      return 0;
  }
  /* should not happen */
  return -1;
}

int
units_mr (Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word arg0;  //address of the input value  (mpoint / mint / mreal)
  
  MReal* mr;  //the corresponding class objects (input and output)
  UReal* unit; 
    
  UnitsLocalInfo *localinfo;
  
  switch( message )
  {
    case OPEN:

      qp->Request(args[0].addr, arg0);

      mr = ((MReal*)arg0.addr);  //receive the MPoint value

      localinfo = new UnitsLocalInfo;
      localinfo->mpir = arg0;
      localinfo->unitIndex = 0;
      local = SetWord(localinfo);
      
      return 0;

    case REQUEST:
      
      if (local.addr ==0) return CANCEL;
      localinfo=(UnitsLocalInfo *) local.addr;
      
      arg0 = localinfo->mpir;
      mr = (MReal*)arg0.addr;   //recover from local info.
      
      if (( 0 <= localinfo->unitIndex )&&( localinfo->unitIndex < mr->GetNoComponents() ))
      {
	  unit = new UReal;
	  mr->Get(localinfo->unitIndex++, *unit);
	  
	  result.addr = unit;
	  return YIELD;
      }
      else return CANCEL;

    case CLOSE:
      
      if( local.addr != 0 )
      {
	  localinfo=(UnitsLocalInfo *) local.addr;
	  delete localinfo;
      }
      
      return 0;
  }
  /* should not happen */
  return -1;
}

/*
16.3.26 Value mapping functions of operator ~theyear~

(int) ---- (periods)

*/

int theyearfun( Word* args, Word& result, int message, Word& local, Supplier s )
{ // int --> periods (=range(instant))
  result = qp->ResultStorage( s );
  ((Range<Instant>*)result.addr)->Clear();
  
  //1.get the input and out put objects
  CcInt *CcIntyear;
  int intyear;
  
  CcIntyear=((CcInt*)args[0].addr);
  intyear=CcIntyear->GetIntval();
  
  Range<Instant> *defrange = new Range<Instant>( 0 );
  
  //2.get the timeintervals and add them to the result
  defrange->StartBulkLoad();
  Instant inst1, inst2;
  inst1.SetType(instanttype);
  inst1.Set(intyear, 1, 1, 0, 0, 0, 0);
  inst2.SetType(instanttype);
  inst2.Set(intyear+1, 1, 1, 0, 0, 0, 0);
  Interval<Instant> timeInterval(inst1, inst2, true, false);
  	  
  defrange->Add( timeInterval ); 
      
  defrange->EndBulkLoad( true );
  
  defrange->Merge(((Range<Instant>*)result.addr));
  
  return (0);
}

/*
16.3.26 Value mapping functions of operator ~themonth~

(int x int) ---- (periods)

*/

int themonthfun( Word* args, Word& result, int message, Word& local, Supplier s )
{ 
  result = qp->ResultStorage( s );
  ((Range<Instant>*)result.addr)->Clear();
  
  //1.get the input and out put objects
  CcInt *CcIntyear;
  CcInt *CcIntmonth;
  int intyear, intmonth;
  
  CcIntyear=((CcInt*)args[0].addr);
  intyear=CcIntyear->GetIntval();
  
  CcIntmonth=((CcInt*)args[1].addr);
  intmonth=CcIntmonth->GetIntval();
  //cout<<"year: "<<intyear<<"month:"<<intmonth<<endl;
  Range<Instant> *defrange = new Range<Instant>( 0 );
  
  //2.get the timeintervals and add them to the result
  defrange->StartBulkLoad();
  Instant inst1, inst2;
  
  inst1.SetType(instanttype);
  inst1.Set(intyear, intmonth, 1, 0, 0, 0, 0);
  
  inst2.SetType(instanttype);
  if (intmonth<12)
      inst2.Set(intyear, intmonth+1, 1, 0, 0, 0, 0);
  else inst2.Set(intyear+1, 1, 1, 0, 0, 0, 0);
  
  Interval<Instant> timeInterval(inst1, inst2, true, false);
  	  
  defrange->Add( timeInterval ); 
      
  defrange->EndBulkLoad( true );
  
  defrange->Merge(((Range<Instant>*)result.addr));
  
  return (0);
}

/*
16.3.27 Value mapping functions of operator ~theday~

(int x int x int) ---- (periods)

*/

int thedayfun( Word* args, Word& result, int message, Word& local, Supplier s )
{ 
  result = qp->ResultStorage( s );
  ((Range<Instant>*)result.addr)->Clear();
  
  //1.get the input and out put objects
  CcInt *CcIntyear;
  CcInt *CcIntmonth;
  CcInt *CcIntday;
  int intyear, intmonth, intday;
  
  CcIntyear=((CcInt*)args[0].addr);
  intyear=CcIntyear->GetIntval();
  
  CcIntmonth=((CcInt*)args[1].addr);
  intmonth=CcIntmonth->GetIntval();
  
  CcIntday=((CcInt*)args[2].addr);
  intday=CcIntday->GetIntval();
  
  //cout<<"year: "<<intyear<<"month:"<<intmonth<<endl;
  Range<Instant> *defrange = new Range<Instant>( 0 );
  
  //2.get the timeintervals and add them to the result
  defrange->StartBulkLoad();
  Instant inst1, inst2, oneday(1,0,durationtype);
  
  inst1.SetType(instanttype);
  inst1.Set(intyear, intmonth, intday, 0, 0, 0, 0);
  
  inst2 .SetType(instanttype);
  inst2 = inst1 + oneday;
  
  Interval<Instant> timeInterval(inst1, inst2, true, false);
  	  
  defrange->Add( timeInterval ); 
      
  defrange->EndBulkLoad( true );
  
  defrange->Merge(((Range<Instant>*)result.addr));
  
  return (0);
}

/*
16.3.28 Value mapping functions of operator ~thehour~

(int x int x int x int) ---- (periods)

*/

int thehourfun( Word* args, Word& result, int message, Word& local, Supplier s )
{ 
  result = qp->ResultStorage( s );
  ((Range<Instant>*)result.addr)->Clear();
  
  //1.get the input and out put objects
  CcInt *CcIntyear;
  CcInt *CcIntmonth;
  CcInt *CcIntday;
  CcInt *CcInthour;
  int intyear, intmonth, intday, inthour;
  
  CcIntyear=((CcInt*)args[0].addr);
  intyear=CcIntyear->GetIntval();
  
  CcIntmonth=((CcInt*)args[1].addr);
  intmonth=CcIntmonth->GetIntval();
  
  CcIntday=((CcInt*)args[2].addr);
  intday=CcIntday->GetIntval();
  
  CcInthour=((CcInt*)args[3].addr);
  inthour=CcInthour->GetIntval();
  
  //cout<<"year: "<<intyear<<"month:"<<intmonth<<endl;
  Range<Instant> *defrange = new Range<Instant>( 0 );
  
  //2.get the timeintervals and add them to the result
  defrange->StartBulkLoad();
  Instant inst1, inst2, onehour(0, 1*60*60*1000, durationtype);
  
  inst1.SetType(instanttype);
  inst1.Set(intyear, intmonth, intday, inthour, 0, 0, 0);
  
  inst2 .SetType(instanttype);
  inst2 = inst1 + onehour;
  
  Interval<Instant> timeInterval(inst1, inst2, true, false);
  	  
  defrange->Add( timeInterval ); 
      
  defrange->EndBulkLoad( true );
  
  defrange->Merge(((Range<Instant>*)result.addr));
  
  return (0);
}

/*
16.3.28 Value mapping functions of operator ~theminute~

(int x int x int x int x int) ---- (periods)

*/

int theminutefun( Word* args, Word& result, int message, Word& local, Supplier s )
{ 
  result = qp->ResultStorage( s );
  ((Range<Instant>*)result.addr)->Clear();
  
  //1.get the input and out put objects
  CcInt *CcIntyear;
  CcInt *CcIntmonth;
  CcInt *CcIntday;
  CcInt *CcInthour;
  CcInt *CcIntminute;
  int intyear, intmonth, intday, inthour, intminute;
  
  CcIntyear=((CcInt*)args[0].addr);
  intyear=CcIntyear->GetIntval();
  
  CcIntmonth=((CcInt*)args[1].addr);
  intmonth=CcIntmonth->GetIntval();
  
  CcIntday=((CcInt*)args[2].addr);
  intday=CcIntday->GetIntval();
  
  CcInthour=((CcInt*)args[3].addr);
  inthour=CcInthour->GetIntval();
  
  CcIntminute=((CcInt*)args[4].addr);
  intminute=CcIntminute->GetIntval();
  
  //cout<<"year: "<<intyear<<"month:"<<intmonth<<endl;
  Range<Instant> *defrange = new Range<Instant>( 0 );
  
  //2.get the timeintervals and add them to the result
  defrange->StartBulkLoad();
  Instant inst1, inst2, oneminute(0, 1*60*1000, durationtype);
  
  inst1.SetType(instanttype);
  inst1.Set(intyear, intmonth, intday, inthour, intminute, 0, 0);
  
  inst2 .SetType(instanttype);
  inst2 = inst1 + oneminute;
  
  Interval<Instant> timeInterval(inst1, inst2, true, false);
  	  
  defrange->Add( timeInterval ); 
      
  defrange->EndBulkLoad( true );
  
  defrange->Merge(((Range<Instant>*)result.addr));
  
  return (0);
}

/*
16.3.28 Value mapping functions of operator ~thesecond~

(int x int x int x int x int x int) ---- (periods)

*/

int thesecondfun( Word* args, Word& result, int message, Word& local, Supplier s )
{ 
  result = qp->ResultStorage( s );
  ((Range<Instant>*)result.addr)->Clear();
  
  //1.get the input and out put objects
  CcInt *CcIntyear;
  CcInt *CcIntmonth;
  CcInt *CcIntday;
  CcInt *CcInthour;
  CcInt *CcIntminute;
  CcInt *CcIntsecond;
  int intyear, intmonth, intday, inthour, intminute, intsecond;
  
  CcIntyear=((CcInt*)args[0].addr);
  intyear=CcIntyear->GetIntval();
  
  CcIntmonth=((CcInt*)args[1].addr);
  intmonth=CcIntmonth->GetIntval();
  
  CcIntday=((CcInt*)args[2].addr);
  intday=CcIntday->GetIntval();
  
  CcInthour=((CcInt*)args[3].addr);
  inthour=CcInthour->GetIntval();
  
  CcIntminute=((CcInt*)args[4].addr);
  intminute=CcIntminute->GetIntval();
  
  CcIntsecond=((CcInt*)args[5].addr);
  intsecond=CcIntsecond->GetIntval();
  
  //cout<<"year: "<<intyear<<"month:"<<intmonth<<endl;
  Range<Instant> *defrange = new Range<Instant>( 0 );
  
  //2.get the timeintervals and add them to the result
  defrange->StartBulkLoad();
  Instant inst1, inst2, onesecond(0, 1000, durationtype);
  
  inst1.SetType(instanttype);
  inst1.Set(intyear, intmonth, intday, inthour, intminute, intsecond, 0);
  
  inst2 .SetType(instanttype);
  inst2 = inst1 + onesecond;
  
  Interval<Instant> timeInterval(inst1, inst2, true, false);
  	  
  defrange->Add( timeInterval ); 
      
  defrange->EndBulkLoad( true );
  
  defrange->Merge(((Range<Instant>*)result.addr));
  
  return (0);
}

/*
16.3.29 Value mapping functions of operator ~theperiod~

(periods x periods) ---- (periods)

*/

int theperiodfun( Word* args, Word& result, int message, Word& local, Supplier s )
{ cout<<"the period called"<<endl;
  result = qp->ResultStorage( s );
  ((Range<Instant>*)result.addr)->Clear();
  
  //1.get the input and out put objects
  Range<Instant>  *range1, *range2;
  
  range1=((Range<Instant>*)args[0].addr);
  range2=((Range<Instant>*)args[1].addr);
  
  Range<Instant> *defrange = new Range<Instant>( 0 );
  
  //2.get the timeintervals and add them to the result
  defrange->StartBulkLoad();
  
  Interval<Instant> intv1, intv2;
  if (!(range1->IsEmpty()))
      range1->Get(0, intv1);
  
  if (!(range2->IsEmpty()))
      range2->Get(range2->GetNoComponents()-1, intv2);
  
  if ((!(range1->IsEmpty()))&&(!(range2->IsEmpty())))
  {
	  Interval<Instant> timeInterval(intv1.start, intv2.end, intv1.lc, intv2.rc);
  	  
	  defrange->Add( timeInterval ); 
	  defrange->EndBulkLoad( true );
  
	  defrange->Merge(((Range<Instant>*)result.addr));
  }
  return (0);
}

/*
16.4 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of value
mapping functions for each operator. For nonoverloaded operators there is also such and array
defined, so it easier to make them overloaded.

16.4.1 ValueMapping arrays

*/
ValueMapping instantisemptymap[] = { InstantIsEmpty };
ValueMapping instantequalmap[] = { InstantEqual };
ValueMapping instantnotequalmap[] = { InstantNotEqual };
ValueMapping instantlessmap[] = { InstantLess };
ValueMapping instantlessequalmap[] = { InstantLessEqual };
ValueMapping instantgreatermap[] = { InstantGreater };
ValueMapping instantgreaterequalmap[] = { InstantGreaterEqual };

ValueMapping rangeintisemptymap[] = { RangeIsEmpty_r<CcInt> };
ValueMapping rangeintequalmap[] = { RangeEqual_rr<CcInt> };
ValueMapping rangeintnotequalmap[] = { RangeNotEqual_rr<CcInt> };
ValueMapping rangeintintersectsmap[] = { RangeIntersects_rr<CcInt> };
ValueMapping rangeintinsidemap[] = { RangeInside_rr<CcInt>, RangeInside_ar<CcInt> };
ValueMapping rangeintbeforemap[] = { RangeBefore_rr<CcInt>, RangeBefore_ar<CcInt>, RangeBefore_ra<CcInt> };
ValueMapping rangeintintersectionmap[] = { RangeIntersection_rr<CcInt> };
ValueMapping rangeintunionmap[] = { RangeUnion_rr<CcInt> };
ValueMapping rangeintminusmap[] = { RangeMinus_rr<CcInt> };
ValueMapping rangeintminmap[] = { RangeMinimum_r<CcInt> };
ValueMapping rangeintmaxmap[] = { RangeMaximum_r<CcInt> };
ValueMapping rangeintnocomponentsmap[] = { RangeNoComponents_r<CcInt> };

ValueMapping rangerealisemptymap[] = { RangeIsEmpty_r<CcReal> };
ValueMapping rangerealequalmap[] = { RangeEqual_rr<CcReal> };
ValueMapping rangerealnotequalmap[] = { RangeNotEqual_rr<CcReal> };
ValueMapping rangerealintersectsmap[] = { RangeIntersects_rr<CcReal> };
ValueMapping rangerealinsidemap[] = { RangeInside_rr<CcReal>, RangeInside_ar<CcReal> };
ValueMapping rangerealbeforemap[] = { RangeBefore_rr<CcReal>, RangeBefore_ar<CcReal>, RangeBefore_ra<CcReal> };
ValueMapping rangerealintersectionmap[] = { RangeIntersection_rr<CcReal> };
ValueMapping rangerealunionmap[] = { RangeUnion_rr<CcReal> };
ValueMapping rangerealminusmap[] = { RangeMinus_rr<CcReal> };
ValueMapping rangerealminmap[] = { RangeMinimum_r<CcReal> };
ValueMapping rangerealmaxmap[] = { RangeMaximum_r<CcReal> };
ValueMapping rangerealnocomponentsmap[] = { RangeNoComponents_r<CcReal> };

ValueMapping intimeintinstmap[] = { IntimeInst<CcInt> };
ValueMapping intimeintvalmap[] = { IntimeVal<CcInt> };

ValueMapping intimerealinstmap[] = { IntimeInst<CcReal> };
ValueMapping intimerealvalmap[] = { IntimeVal<CcReal> };

ValueMapping intimepointinstmap[] = { IntimeInst<Point> };
ValueMapping intimepointvalmap[] = { IntimeVal<Point> };

ValueMapping atinstantmap[] =   {  atinstant_mint,
			     atinstant_mreal,
			     atinstant_mpoint
			   };

ValueMapping deftimemap[] =   {  deftime_mint,
			     deftime_mreal,
			     deftime_mpoint
			   };

ValueMapping trajectorymap[] =   {  trajectory_mp };

ValueMapping presentmap[] =   {  present_mint,
			     present_mreal,
			     present_mpoint
			   };

ValueMapping passesmap[] =   {  passes_mint,
			     passes_mreal,
			     passes_mpoint
			   };

ValueMapping initialmap[] =   {  initial_mint,
			     initial_mreal,
			     initial_mpoint
			   };

ValueMapping finalmap[] =   {  final_mint,
			     final_mreal,
			     final_mpoint
			   };

ValueMapping unitsmap[] =   {  units_mp,
			     units_mi,
			     units_mr
			   };

ValueMapping intperiodsmap[] =   {      theyearfun,
				    themonthfun,
				    thedayfun,
				    thehourfun,
				    theminutefun,
				    thesecondfun,
				    theperiodfun
				};

Word TemporalNoModelMapping( ArgVector arg, Supplier opTreeNode )
{
  return (SetWord( Address( 0 ) ));
}

ModelMapping temporalnomodelmap[] = { TemporalNoModelMapping,
				      TemporalNoModelMapping,
				      TemporalNoModelMapping,
				      TemporalNoModelMapping,
				      TemporalNoModelMapping,
				      TemporalNoModelMapping };

ModelMapping rangenomodelmap[] = {TemporalNoModelMapping, 
				  TemporalNoModelMapping,
				  TemporalNoModelMapping,
				  TemporalNoModelMapping,
				  TemporalNoModelMapping,
				  TemporalNoModelMapping };

/*
16.4.2 Specification strings

*/
const string TemporalSpecIsEmpty  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                    "\"Example\" ) "
                                    "( <text>instant -> bool, range -> bool</text--->"
                                    "<text>isempty ( _ )</text--->"
                                    "<text>Returns whether the instant is empty or "
                                    "not.</text--->"
                                    "<text>query isempty ( instant )</text--->"
                                    ") )";

const string TemporalSpecEQ  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" )"
                               "( <text>(instant instant) -> bool, (range range) -> bool</text--->"
                               "<text>_ = _</text--->"
                               "<text>Equal.</text--->"
                               "<text>query i1 = i2</text--->"
                               ") )";

const string TemporalSpecNE  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" )"
                               "( <text>(instant instant) -> bool, (range range) -> bool</text--->"
                               "<text>_ # _</text--->"
                               "<text>Not equal.</text--->"
                               "<text>query i1 # i2</text--->"
                               ") )";

const string TemporalSpecLT  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" )"
                               "( <text>(instant instant) -> bool</text--->"
                               "<text>_ < _</text--->"
                               "<text>Less than.</text--->"
                               "<text>query i1 < i2</text--->"
                               ") )";

const string TemporalSpecLE  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" )"
                               "( <text>(instant instant) -> bool</text--->"
                               "<text>_ <= _</text--->"
                               "<text>Less or equal than.</text--->"
                               "<text>query i1 <= i2</text--->"
                               ") )";

const string TemporalSpecGT  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" )"
                               "( <text>(instant instant) -> bool</text--->"
                               "<text>_ > _</text--->"
                               "<text>Greater than.</text--->"
                               "<text>query i1 > i2</text--->"
                               ") )";

const string TemporalSpecGE  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" )"
                               "( <text>(instant instant) -> bool</text--->"
                               "<text>_ >= _</text--->"
                               "<text>Greater or equal than.</text--->"
                               "<text>query i1 >= i2</text--->"
                               ") )";

const string TemporalSpecIntersects  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                       "\"Example\" ) "
                                       "( <text>( (range x) (range x) ) -> bool</text--->"
                                       "<text>_ intersects _</text--->"
                                       "<text>Intersects.</text--->"
                                       "<text>query range1 intersects range2</text--->"
                                       ") )";

const string TemporalSpecInside  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                   "\"Example\" ) "
                                   "( <text>( (range x) (range x) ) -> bool,"
                                   "( x (range x) ) -> bool</text--->"
                                   "<text>_ inside _</text--->"
                                   "<text>Inside.</text--->"
                                   "<text>query 5 inside rangeint1</text--->"
                                   ") )";

const string TemporalSpecBefore  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                   "\"Example\" ) "
                                   "( <text>( (range x) (range x) ) -> bool, "
                                   "( x (range x) ) -> bool, ( (range x) x ) -> "
                                   "bool</text--->"
                                   "<text>_ before _</text--->"
                                   "<text>Before.</text--->"
                                   "<text>query 5 before rangeint1</text--->"
                                   ") )";

const string TemporalSpecIntersection  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                         "\"Example\" ) "
                                         "( <text>( (range x) (range x) ) -> (range x)</text--->"
                                         "<text>_ intersection _</text--->"
                                         "<text>Intersection.</text--->"
                                         "<text>query range1 intersection range2</text--->"
                                         ") )";

const string TemporalSpecUnion  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                  "\"Example\" ) "
                                  "( <text>( (range x) (range x) ) -> (range x)</text--->"
                                  "<text>_ union _</text--->"
                                  "<text>Union.</text--->"
                                  "<text>query range1 union range2</text--->"
                                  ") )";

const string TemporalSpecMinus  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                  "\"Example\" ) "
                                  "( <text>( (range x) (range x) ) -> (range x)</text--->"
                                  "<text>_ minus _</text--->"
                                  "<text>Minus.</text--->"
                                  "<text>query range1 minus range2</text--->"
                                  ") )";

const string TemporalSpecMinimum  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                    "\"Example\" ) "
                                    "( <text>(range x) -> x</text--->"
                                    "<text>minimum ( _ )</text--->"
                                    "<text>Minimum.</text--->"
                                    "<text>minimum ( range1 )</text--->"
                                    ") )";

const string TemporalSpecMaximum  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                    "\"Example\" ) "
                                    "( <text>(range x) -> x</text--->"
                                    "<text>maximum ( _ )</text--->"
                                    "<text>Maximum.</text--->"
                                    "<text>maximum ( range1 )</text--->"
                                    ") )";

const string TemporalSpecNoComponents  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                         "\"Example\" ) "
                                         "( <text>(range x) -> int</text--->"
                                         "<text>no_components ( _ )</text--->"
                                         "<text>Number of components.</text--->"
                                         "<text>no_components ( range1 )</text--->"
                                         ") )";

const string TemporalSpecInst  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                 "\"Example\" ) "
                                 "( <text>(intime x) -> instant</text--->"
                                 "<text>inst ( _ )</text--->"
                                 "<text>Intime time instant.</text--->"
                                 "<text>inst ( i1 )</text--->"
                                 ") )";

const string TemporalSpecVal  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
                                "( <text>(intime x) -> x</text--->"
                                "<text>val ( _ )</text--->"
                                "<text>Intime value.</text--->"
                                "<text>val ( i1 )</text--->"
                                ") )";

const string TemporalSpecAtInstant  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
                                "( <text>(mint||mreal||mpoint) x (instant) -> intimeint||intimereal||intimepoint</text--->"
                                "<text>_ atinstant _ </text--->"
                                "<text>get the Intime value corresponding to the instant.</text--->"
                                "<text>mpoint1 at instant 21.2</text--->"
                                ") )";

const string TemporalSpecDeftime  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
                                "( <text>(mint||mreal||mpoint) -> periods</text--->"
                                "<text> deftime( _ )</text--->"
                                "<text>get the definetime of the corresponding moving data objects.</text--->"
                                "<text>deftime(mp1)</text--->"
                                ") )";

const string TemporalSpecTrajectory  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
                                "( <text>(mpoint -> line</text--->"
                                "<text> trajectory( _ )</text--->"
                                "<text>get the trajectory of the corresponding moving data objects.</text--->"
                                "<text>trajectory(mp1)</text--->"
                                ") )";

const string TemporalSpecPresent  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
                                "( <text>(mint||mreal||mpoint) x (instant) -> bool</text--->"
                                "<text>_ present _ </text--->"
                                "<text>whether the object is present at the given instant.</text--->"
                                "<text>mpoint1 present (instant 21.2)</text--->"
                                ") )";

const string TemporalSpecPasses = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
                                "( <text>(mint||mreal||mpoint) x (int||real||point) -> bool</text--->"
                                "<text>_ passes _ </text--->"
                                "<text>whether the object passes the given value.</text--->"
                                "<text>mpoint1 passes point1</text--->"
                                ") )";

const string TemporalSpecInitial  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
                                "( <text>mint||mreal||mpoint -> intimeint||intimereal||intimepoint</text--->"
                                "<text> initial( _ )</text--->"
                                "<text>get the Intime value corresponding to the initial instant.</text--->"
                                "<text>initial(mpoint1)</text--->"
                                ") )";

const string TemporalSpecFinal  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
                                "( <text>mint||mreal||mpoint -> intimeint||intimereal||intimepoint</text--->"
                                "<text> final( _ )</text--->"
                                "<text>get the Intime value corresponding to the final instant.</text--->"
                                "<text>final(mpoint1)</text--->"
                                ") )";

const string TemporalSpecUnits  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
                                "( <text>mpoint||mint||mreal -> stream(upoint||constint||ureal)</text--->"
                                "<text> units( _ )</text--->"
                                "<text>get the stream of units of the moving value.</text--->"
                                "<text>units(mpoint1)</text--->"
                                ") )";

const string TemporalSpecTheyear  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
		"( <text>int -> periods</text--->"
                                "<text> theyear( _ )</text--->"
                                "<text>get the periods value of the year.</text--->"
                                "<text>theyear(2002)</text--->"
                                ") )";

const string TemporalSpecThemonth  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
		"( <text>int x int -> periods</text--->"
                                "<text> themonth( _, _ )</text--->"
                                "<text>get the periods value of the month.</text--->"
                                "<text>themonth(2002, 3)</text--->"
                                ") )";

const string TemporalSpecTheday  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
		"( <text>int x int x int -> periods</text--->"
                                "<text> theday( _, _, _ )</text--->"
                                "<text>get the periods value of the day.</text--->"
                                "<text>theday(2002, 6,3)</text--->"
                                ") )";

const string TemporalSpecThehour  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
		"( <text>int x int x int x int -> periods</text--->"
                                "<text> thehour( _, _, _ , _)</text--->"
                                "<text>get the periods value of the hour.</text--->"
                                "<text>theyear(2002, 2, 28, 8)</text--->"
                                ") )";

const string TemporalSpecTheminute  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
		"( <text>int x int x int x int x int -> periods</text--->"
                                "<text> theminute( _ )</text--->"
                                "<text>get the periods value of the minute.</text--->"
                                "<text>theminute(2002, 3, 28, 8, 59)</text--->"
                                ") )";

const string TemporalSpecThesecond  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
		"( <text>int x int x int x int x int x int  -> periods</text--->"
                                "<text> thesecond( _ )</text--->"
                                "<text>get the periods value of the second.</text--->"
                                "<text>thesecond(2002, 12, 31, 23, 59, 59)</text--->"
                                ") )";

const string TemporalSpecTheperiod  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
		"( <text>period x periods -> periods</text--->"
                                "<text> theperiod( _, _ )</text--->"
                                "<text>get the periods value of the 2 periods.</text--->"
                                "<text>theperiod(theyear(2002), theyear(2004))</text--->"
                                ") )";

/*
16.4.3 Operators

*/
Operator instantisempty( "isempty",
                          TemporalSpecIsEmpty,
                          1,
                          instantisemptymap,
                          temporalnomodelmap,
                          Operator::SimpleSelect,
                          InstantTypeMapBool );

Operator instantequal( "=",
                       TemporalSpecEQ,
                       1,
                       instantequalmap,
                       temporalnomodelmap,
                       Operator::SimpleSelect,
                       InstantInstantTypeMapBool );

Operator instantnotequal( "#",
                          TemporalSpecNE,
                          1,
                          instantnotequalmap,
                          temporalnomodelmap,
                          Operator::SimpleSelect,
                          InstantInstantTypeMapBool );

Operator instantless( "<",
                      TemporalSpecLT,
                      1,
                      instantlessmap,
                      temporalnomodelmap,
                      Operator::SimpleSelect,
                      InstantInstantTypeMapBool );

Operator instantlessequal( "<=",
                           TemporalSpecLE,
                           1,
                           instantlessequalmap,
                           temporalnomodelmap,
                           Operator::SimpleSelect,
                           InstantInstantTypeMapBool );

Operator instantgreater( ">",
                         TemporalSpecLT,
                         1,
                         instantgreatermap,
                         temporalnomodelmap,
                         Operator::SimpleSelect,
                         InstantInstantTypeMapBool );

Operator instantgreaterqual( ">=",
                             TemporalSpecLE,
                             1,
                             instantgreaterequalmap,
                             temporalnomodelmap,
                             Operator::SimpleSelect,
                             InstantInstantTypeMapBool );

Operator rangeintisempty( "isempty",
                          TemporalSpecIsEmpty,
                          1,
                          rangeintisemptymap,
                          rangenomodelmap,
                          Operator::SimpleSelect,
                          RangeTypeMapBool1 );

Operator rangeintequal( "=",
                        TemporalSpecEQ,
                        1,
                        rangeintequalmap,
                        rangenomodelmap,
                        Operator::SimpleSelect,
                        RangeRangeTypeMapBool );

Operator rangeintnotequal( "#",
                           TemporalSpecNE,
                           1,
                           rangeintnotequalmap,
                           rangenomodelmap,
                           Operator::SimpleSelect,
                           RangeRangeTypeMapBool );

Operator rangeintintersects( "intersects",
                             TemporalSpecIntersects,
                             1,
                             rangeintintersectsmap,
                             rangenomodelmap,
                             Operator::SimpleSelect,
                             RangeRangeTypeMapBool );

Operator rangeintinside( "inside",
                         TemporalSpecInside,
                         2,
                         rangeintinsidemap,
                         rangenomodelmap,
                         RangeSelectPredicates,
                         RangeBaseTypeMapBool1 );

Operator rangeintbefore( "before",
                         TemporalSpecBefore,
                         3,
                         rangeintbeforemap,
                         rangenomodelmap,
                         RangeSelectPredicates,
                         RangeBaseTypeMapBool2 );

Operator rangeintintersection( "intersection",
                               TemporalSpecIntersection,
                               1,
                               rangeintintersectionmap,
                               rangenomodelmap,
                               Operator::SimpleSelect,
                               RangeRangeTypeMapRange );

Operator rangeintunion( "union",
                        TemporalSpecUnion,
                        1,
                        rangeintunionmap,
                        rangenomodelmap,
                        Operator::SimpleSelect,
                        RangeRangeTypeMapRange );

Operator rangeintminus( "minus",
                        TemporalSpecMinus,
                        1,
                        rangeintminusmap,
                        rangenomodelmap,
                        Operator::SimpleSelect,
                        RangeRangeTypeMapRange );

Operator rangeintmin( "minimum",
                      TemporalSpecMinimum,
                      1,
                      rangeintminmap,
                      rangenomodelmap,
                      Operator::SimpleSelect,
                      RangeTypeMapBase );

Operator rangeintmax( "maximum",
                      TemporalSpecMaximum,
                      1,
                      rangeintmaxmap,
                      rangenomodelmap,
                      Operator::SimpleSelect,
                      RangeTypeMapBase );
 
Operator rangeintnocomponents( "no_components",
                               TemporalSpecNoComponents,
                               1,
                               rangeintnocomponentsmap,
                               rangenomodelmap,
                               Operator::SimpleSelect,
                               RangeTypeMapInt );

Operator rangerealisempty( "isempty",
                           TemporalSpecIsEmpty,
                           1,
                           rangerealisemptymap,
                           rangenomodelmap,
                           Operator::SimpleSelect,
                           RangeTypeMapBool1 );

Operator rangerealequal( "=",
                         TemporalSpecEQ,
                         1,
                         rangerealequalmap,
                         rangenomodelmap,
                         Operator::SimpleSelect,
                         RangeRangeTypeMapBool );

Operator rangerealnotequal( "#",
                            TemporalSpecNE,
                            1,
                            rangerealnotequalmap,
                            rangenomodelmap,
                            Operator::SimpleSelect,
                            RangeRangeTypeMapBool );

Operator rangerealintersects( "intersects",
                              TemporalSpecIntersects,
                              1,
                              rangerealintersectsmap,
                              rangenomodelmap,
                              Operator::SimpleSelect,
                              RangeRangeTypeMapBool );

Operator rangerealinside( "inside",
                          TemporalSpecInside,
                          2,
                          rangerealinsidemap,
                          rangenomodelmap,
                          RangeSelectPredicates,
                          RangeBaseTypeMapBool1 );

Operator rangerealbefore( "before",
                          TemporalSpecBefore,
                          3,
                          rangerealbeforemap,
                          rangenomodelmap,
                          RangeSelectPredicates,
                          RangeBaseTypeMapBool2 );
 
Operator rangerealintersection( "intersection",
                                TemporalSpecIntersection,
                                1,
                                rangerealintersectionmap,
                                rangenomodelmap,
                                Operator::SimpleSelect,
                                RangeRangeTypeMapRange );

Operator rangerealunion( "union",
                         TemporalSpecUnion,
                         1,
                         rangerealunionmap,
                         rangenomodelmap,
                         Operator::SimpleSelect,
                         RangeRangeTypeMapRange );

Operator rangerealminus( "minus",
                         TemporalSpecMinus,
                         1,
                         rangerealminusmap,
                         rangenomodelmap,
                         Operator::SimpleSelect,
                         RangeRangeTypeMapRange );

Operator rangerealmin( "minimum",
                       TemporalSpecMinimum,
                       1,
                       rangerealminmap,
                       rangenomodelmap,
                       Operator::SimpleSelect,
                       RangeTypeMapBase );

Operator rangerealmax( "maximum",
                       TemporalSpecMaximum,
                       1,
                       rangerealmaxmap,
                       rangenomodelmap,
                       Operator::SimpleSelect,
                       RangeTypeMapBase );
  
Operator rangerealnocomponents( "no_components",
                                TemporalSpecNoComponents,
                                1,
                                rangerealnocomponentsmap,
                                rangenomodelmap,
                                Operator::SimpleSelect,
                                RangeTypeMapInt );

Operator intimeintinst( "inst",
                        TemporalSpecInst,
                        1,
                        intimeintinstmap,
                        temporalnomodelmap,
                        Operator::SimpleSelect,
                        IntimeTypeMapInstant );

Operator intimeintval( "val",
                       TemporalSpecVal,
                       1,
                       intimeintvalmap,
                       temporalnomodelmap,
                       Operator::SimpleSelect,
                       IntimeTypeMapBase );

Operator intimerealinst( "inst",
                         TemporalSpecInst,
                         1,
                         intimerealinstmap,
                         temporalnomodelmap,
                         Operator::SimpleSelect,
                         IntimeTypeMapInstant );

Operator intimerealval( "val",
                        TemporalSpecVal,
                        1,
                        intimerealvalmap,
                        temporalnomodelmap,
                        Operator::SimpleSelect,
                        IntimeTypeMapBase );

Operator atinstant( "atinstant",
                        TemporalSpecAtInstant,
                        3,
                        atinstantmap,
                        temporalnomodelmap,
                        TemporalSelectAtInstant,
                        MappingInstantTypeMapIntime );

Operator deftime( "deftime",
                        TemporalSpecDeftime,
                        3,
                        deftimemap,
                        temporalnomodelmap,
                        TemporalSelectDeftime,
                        MappingTypeMapRangeInstant );

 
Operator trajectory( "trajectory",
                                TemporalSpecTrajectory,
                                1,
                                trajectorymap,
                                rangenomodelmap,
                                Operator::SimpleSelect,
                                MPointTypeMapLine);

Operator present( "present",
                        TemporalSpecPresent,
                        3,
                        presentmap,
                        temporalnomodelmap,
                        TemporalSelectAtInstant,
                        MappingInstantTypeMapBool);

Operator passes( "passes",
                        TemporalSpecPasses,
                        3,
                        passesmap,
                        temporalnomodelmap,
                        TemporalSelectPasses,
                        MappingATypeMapBool);

Operator initial( "initial",
                        TemporalSpecInitial,
                        3,
                        initialmap,
                        temporalnomodelmap,
                        TemporalSelectInitialFinal,
                        MappingTypeMapIntime );

Operator final( "final",
                        TemporalSpecFinal,
                        3,
                        finalmap,
                        temporalnomodelmap,
                        TemporalSelectInitialFinal,
                        MappingTypeMapIntime );

Operator units( "units",
                        TemporalSpecUnits,
                        3,
                        unitsmap,
                        temporalnomodelmap,
                        TemporalSelectUnits,
                        MappingTypeMapUnits);

Operator theyear( "theyear",
                        TemporalSpecTheyear,
                        7,
                        intperiodsmap,
                        temporalnomodelmap,
                        TemporalSelectIntPeriods,
                        MappingTypeMapIntPeriods);

Operator themonth( "themonth",
                        TemporalSpecThemonth,
                        7,
                        intperiodsmap,
                        temporalnomodelmap,
                        TemporalSelectIntPeriods,
                        MappingTypeMapIntPeriods);

Operator theday( "theday",
                        TemporalSpecTheday,
                        7,
                        intperiodsmap,
                        temporalnomodelmap,
                        TemporalSelectIntPeriods,
                        MappingTypeMapIntPeriods);

Operator thehour( "thehour",
                        TemporalSpecThehour,
                        7,
                        intperiodsmap,
                        temporalnomodelmap,
                        TemporalSelectIntPeriods,
                        MappingTypeMapIntPeriods);

Operator theminute( "theminute",
                        TemporalSpecTheminute,
                        7,
                        intperiodsmap,
                        temporalnomodelmap,
                        TemporalSelectIntPeriods,
                        MappingTypeMapIntPeriods);

Operator thesecond( "thesecond",
                        TemporalSpecThesecond,
                        7,
                        intperiodsmap,
                        temporalnomodelmap,
                        TemporalSelectIntPeriods,
                        MappingTypeMapIntPeriods);

Operator theperiod( "theperiod",
                        TemporalSpecTheperiod,
                        7,
                        intperiodsmap,
                        temporalnomodelmap,
                        TemporalSelectIntPeriods,
                        MappingTypeMapPeriodsPeriods);

/*
6 Creating the Algebra

*/

class TemporalAlgebra : public Algebra
{
 public:
  TemporalAlgebra() : Algebra()
  {
    AddTypeConstructor( &rangeint );
    AddTypeConstructor( &rangereal );
    AddTypeConstructor( &periods );
    AddTypeConstructor( &intimeint );
    AddTypeConstructor( &intimereal );
    AddTypeConstructor( &intimepoint );
    
    AddTypeConstructor( &constint );
    AddTypeConstructor( &ureal );
    AddTypeConstructor( &upoint );
    
    AddTypeConstructor( &mpoint );
    AddTypeConstructor( &mint );
    AddTypeConstructor( &mreal );
	    
    rangeint.AssociateKind( "RANGE" );
    rangereal.AssociateKind( "RANGE" );
    periods.AssociateKind( "RANGE" );
    intimeint.AssociateKind( "TEMPORAL" );
    intimereal.AssociateKind( "TEMPORAL" );
    intimepoint.AssociateKind( "TEMPORAL" );
    constint.AssociateKind( "TEMPORAL" );
    ureal.AssociateKind( "TEMPORAL" );
    upoint.AssociateKind( "TEMPORAL" );
    mint.AssociateKind( "TEMPORAL" );
    mreal.AssociateKind( "TEMPORAL" );
    mpoint.AssociateKind( "TEMPORAL" );
    
    rangeint.AssociateKind( "DATA" );
    rangereal.AssociateKind( "DATA" );
    periods.AssociateKind( "DATA" );
    constint.AssociateKind( "DATA" );
    ureal.AssociateKind( "DATA" );
    upoint.AssociateKind( "DATA" );
    mint.AssociateKind( "DATA" );
    mreal.AssociateKind( "DATA" );
    mpoint.AssociateKind( "DATA" );
    intimeint.AssociateKind( "DATA" );
    intimereal.AssociateKind( "DATA" );
    intimepoint.AssociateKind( "DATA" );
    

    AddOperator( &rangeintisempty );
    AddOperator( &rangeintequal );
    AddOperator( &rangeintnotequal );
    AddOperator( &rangeintintersects );
    AddOperator( &rangeintinside );
    AddOperator( &rangeintbefore );
    AddOperator( &rangeintintersection );
    AddOperator( &rangeintunion );
    AddOperator( &rangeintminus );
    AddOperator( &rangeintmin );
    AddOperator( &rangeintmax );
    AddOperator( &rangeintnocomponents );

    AddOperator( &rangerealisempty );
    AddOperator( &rangerealequal );
    AddOperator( &rangerealnotequal );
    AddOperator( &rangerealintersects );
    AddOperator( &rangerealinside );
    AddOperator( &rangerealbefore );
    AddOperator( &rangerealintersection );
    AddOperator( &rangerealunion );
    AddOperator( &rangerealminus );
    AddOperator( &rangerealmin );
    AddOperator( &rangerealmax );
    AddOperator( &rangerealnocomponents );

    AddOperator( &intimeintinst );
    AddOperator( &intimeintval );
    AddOperator( &intimerealinst );
    AddOperator( &intimerealval );
    AddOperator( &atinstant);
    AddOperator( &deftime);
    AddOperator( &trajectory);
    AddOperator( &present);
    AddOperator( &passes);
    AddOperator( &initial);
    AddOperator( &final);
    AddOperator( &units);
    AddOperator( &theyear);
    AddOperator( &themonth);
    AddOperator( &theday);
    AddOperator( &thehour);
    AddOperator( &theminute);
    AddOperator( &thesecond);
    AddOperator( &theperiod);
  }
  ~TemporalAlgebra() {};
};

TemporalAlgebra temporalAlgebra;

/*
7 Initialization

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
InitializeTemporalAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&temporalAlgebra);
}


