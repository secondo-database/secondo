/*
//paragraph [1] title: [{\Large \bf ]	[}]

[1] Date Algebra

December 11-16, 2002 DZM
***************************************************************************************************
This little example algebra provides one type constructor ~date~  and 7 operators:
date -> int                      year, month, day
date x date -> bool       <,  =, >
ini x int x int->date       opdate

The algerbra provides basic checking on the validity of data. For instance, Fabruary in
leap years (every 4 years except 100th year, and every 400th year) has 29 days, and 
Fabruary in normal years only has 28 days.

If an invalid date is input with list representation, then the system will refuse to take it.
If an invalid date is input with operator ~opdate~, the system will change it into the
nearest valid date. For instance, opdate(29,2,1900) and opdate(31,4,2000) will return 
(28 2 1900) and (30 4 2000) respectively.
*****************************************************************************************************
1 Preliminaries

1.1 Includes

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include <string>

static NestedList* nl;
static QueryProcessor* qp;

/*
1.2 Dummy Functions

Not interesting, but needed in the definition of a type constructor.

*/
static Word
NoSpace( int size ) {return (SetWord( Address( 0 ) ));}

static void
DoNothing( Word& w ) {w.addr = 0;}
 
static void* DummyCast( void* addr ) {return (0);}

/*
2 Type Constructor ~date~

2.1 Data Structure - Class ~Date~

*/
class Date
{
 public:
  Date( int Day, int Month, int Year );
  ~Date();  
  int      GetDay();
  int      GetMonth();
  int      GetYear();
  void     SetDay( int Day );
  void     SetMonth( int Yonth);
  void     SetYear( int Year );
  void     Set( int Day, int Month, int Year );

 private:
  int day;
  int month;
  int year;  
};

Date::Date(int Day, int Month, int Year) {day = Day; month = Month; year = Year;}

Date::~Date() {}

int Date::GetDay() {return day;}

int Date::GetMonth() {return month;}

int Date::GetYear() {return year;}

void Date::SetDay(int Date) {day = Date;}

void Date::SetMonth(int Month) {month = Month;}

void Date::SetYear(int Year) {year = Year;}

void Date::Set(int Day, int Month, int Year) {day = Day; month = Month; year = Year;}

bool isdate(int  Day, int Month, int Year) 
      {  //in this function, we don't check year information. If the year is a negtive integer, for instance -100, then it represents
         //the year of 100 BC.
          bool res=true; 
          bool leapyear;
          int daysinmonth;
          if  (((Year % 4==0) && (Year % 100!=0)) || (Year % 400==0))  leapyear=true; else leapyear=false;
          if ((Month<1)||(Month>12))
	  res=false; 	 //checking month
	  else 		 //checking day
	  {
	      switch (Month)
	      {
		  case 1:
  		  case 3:
  		  case 5:
		  case 7:
  		  case 8:
  		  case 10:
  		  case 12: daysinmonth=31;
			  break;
 	                  case 4: 
 	                  case 6: 
 	                  case 9: 
 	                  case 11: daysinmonth=30; 
			  break;
 	                  case 2: 	 if (leapyear) daysinmonth=29; else daysinmonth=28;
	      }
	      if ((Day<1)||(Day>daysinmonth)) res=false; 
	  }
            return res;	  
      }


/*
2.2 List Representation

The list representation of a date is

----	(dd mm yy)
----

2.3 ~In~ and ~Out~ Functions
*/

static ListExpr
OutDate( ListExpr typeInfo, Word value )
{
  Date* date;
  date = (Date*)(value.addr);
  return nl->ThreeElemList(nl->IntAtom(date->GetDay()), nl->IntAtom(date->GetMonth()), nl->IntAtom(date->GetYear()));
}

static Word
InDate( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Date* newdate;

  if ( nl->ListLength( instance ) == 3 )
  { 
    ListExpr First = nl->First(instance); 
    ListExpr Second = nl->Second(instance);
    ListExpr Third = nl->Third(instance);     

    if    ( nl->IsAtom(First) && nl->AtomType(First) == IntType 
      && nl->IsAtom(Second) && nl->AtomType(Second) == IntType
      && nl->IsAtom(Third) && nl->AtomType(Third) == IntType)	
    { 

          int Day=nl->IntValue(First);
          int Month=nl->IntValue(Second);
          int Year=nl->IntValue(Third);
	  
          if (isdate(Day, Month, Year))   //to validate the date. if the date is invalid, the system will not take it.
	{
	    correct = true;
	    newdate = new Date(nl->IntValue(First), nl->IntValue(Second), nl->IntValue(Third));
	    return SetWord(newdate);
	}
	  else
               {
	    cout <<"---------------------------------------------------------------" << endl;	    
	    cout <<"   >>>invalid date, ignored by the system!<<<" << endl;
	    cout <<"---------------------------------------------------------------" << endl;	    	   
 
               }
    }
  }
  correct = false;
  return SetWord(Address(0));
}

/*
2.4 Function Describing the Signature of the Type Constructor

This one works for type constructors ~date~ , which is an ``atomic'' type.

*/

static ListExpr
DateProperty()
{
  return (nl->TwoElemList(
		nl->TheEmptyList(),
		nl->SymbolAtom("DATA") ));
}

/*
2.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~date~ does not have arguments, this is trivial.

*/
static bool
CheckDate( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "date" ));
}
/*
2.6 Creation of the Type Constructor Instance

*/
TypeConstructor date(
	"date",				//name		
	DateProperty, 			//property function describing signature
	OutDate,   	InDate,		//Out and In functions
	NoSpace,	DoNothing,	//object creation and deletion
	DummyCast,			//cast function
	CheckDate,	          		//kind checking function
	0,				//predefined persistence function        	
	0, 				//predef. pers. function for model
        TypeConstructor::DummyInModel, 	
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );



/*
4 Creating Operators

4.1 Type Mapping Function

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol
~typeerror~.

*/

//  date -> int    operator: year, month, day
static ListExpr
DateInt( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength(args) == 1 )
  {
    arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "date"))  
    return nl->SymbolAtom("int");
  }
  return nl->SymbolAtom("typeerror");
}

//  date x date -> bool    operators: <  = >
static ListExpr
DateDateBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, "date") && nl->IsEqual(arg2, "date") )  
    return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

//   int x int x int -> date    operator: opdate
static ListExpr
IntIntIntDate( ListExpr args )
{
  ListExpr arg1, arg2, arg3;
  if ( nl->ListLength(args) == 3 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    arg3 = nl->Third(args);
    if ( nl->IsEqual(arg1, "int") && nl->IsEqual(arg2, "int") && nl->IsEqual(arg3, "int"))  
    return nl->SymbolAtom("date");
  }
  return nl->SymbolAtom("typeerror");
}


/*
4.2 Selection Function

Is used to select one of several evaluation functions for an overloaded
operator, based on the types of the arguments. In case of a non-overloaded
operator, we just have to return 0.

*/
//operators ~day~, ~month~, ~year~, <, =, >, ~opdate~ are all non-overloaded 
//operators, therefore we only need to return 0.

static int
simpleSelect (ListExpr args ) { return 0; }


/*
4.3 Value Mapping Functions

*/

static int
dayFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  Date* d;
  d = ((Date*)args[0].addr);

  result = qp->ResultStorage(s);
  int res=d->GetDay();
  
  ((CcInt*)result.addr)->Set(true, res);
  return 0;
}

static int
monthFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  Date* d;
  d = ((Date*)args[0].addr);

  result = qp->ResultStorage(s);
  int res=d->GetMonth();
  
  ((CcInt*)result.addr)->Set(true, res);
  return 0;
}

static int
yearFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  Date* d;
  d = ((Date*)args[0].addr);

  result = qp->ResultStorage(s);
  int res=d->GetYear();
  
  ((CcInt*)result.addr)->Set(true, res);
  return 0;
}

static int
earlierFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  Date* d1;
  Date* d2;
  d1 = ((Date*)args[0].addr);
  d2 = ((Date*)args[1].addr);
  
  result = qp->ResultStorage(s);
  bool res;
  if ((d1->GetYear()) > (d2->GetYear())) res=false; 
       else if ((d1->GetYear()) < (d2->GetYear())) res=true;
             else    //year1=year2
		 if ((d1->GetMonth())>(d2->GetMonth())) res=false;
     		      else if ((d1->GetMonth())<(d2->GetMonth())) res=true;
		      	else	//year1=year2 AND month1=month2
			    if ((d1->GetDay())>=(d2->GetDay())) res=false; else res=true; //to check whether day1<day2
	 
  ((CcBool*)result.addr)->Set(true, res);
   return 0;
}

static int
equalFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  Date* d1;
  Date* d2;
  d1 = ((Date*)args[0].addr);
  d2 = ((Date*)args[1].addr);
  
  result = qp->ResultStorage(s);
  bool res;
  if (((d1->GetYear()) == (d2->GetYear())) && 
     ((d1->GetMonth())==(d2->GetMonth())) &&
     ((d1->GetDay())==(d2->GetDay()))) 
      res=true; 
  else res=false; 
	 
  ((CcBool*)result.addr)->Set(true, res);
  return 0;
}

static int
laterFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  Date* d1;
  Date* d2;
  d1 = ((Date*)args[0].addr);
  d2 = ((Date*)args[1].addr);
  
  result = qp->ResultStorage(s);
  bool res;
  if ((d1->GetYear()) < (d2->GetYear())) res=false; 
       else if ((d1->GetYear()) > (d2->GetYear())) res=true;
             else    //year1=year2
		 if ((d1->GetMonth())<(d2->GetMonth())) res=false;
     		      else if ((d1->GetMonth())>(d2->GetMonth())) res=true;
		      	else	//year1=year2 AND month1=month2
			    if ((d1->GetDay())<=(d2->GetDay())) res=false; else res=true; //to check whether day1<day2
	 
  ((CcBool*)result.addr)->Set(true, res);
   return 0;
}

static int
dateFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  CcInt *dd;
  CcInt *mm;
  CcInt *yy;
  dd = (CcInt *)args[0].addr;
  mm = (CcInt *)args[1].addr;
  yy = (CcInt *)args[2].addr;
  Date *delem;
 
  result = qp->ResultStorage(s);
  
  int Day=dd->GetIntval();
  int Month=mm->GetIntval();
  int Year=yy->GetIntval();
  bool leapyear;
  int daysinmonth;
	  
  if (isdate(Day, Month, Year))
   {
      delem=new Date (Day, Month, Year);
      result.addr=delem;
   } 
  else
   {
          if  (((Year % 4==0) && (Year % 100!=0)) || (Year % 400==0))  leapyear=true; else leapyear=false;
	  
          if (Month<1) Month=1; 
          if (Month>12) Month=12;
          
          switch (Month)
	      {
		  case 1:
  		  case 3:
  		  case 5:
		  case 7:
  		  case 8:
  		  case 10:
  		  case 12: daysinmonth=31;
			  break;
 	                  case 4: 
 	                  case 6: 
 	                  case 9: 
 	                  case 11: daysinmonth=30; 
			  break;
 	                  case 2: 	 if (leapyear) daysinmonth=29; else daysinmonth=28;
	      }
	  
            if (Day<1) Day=1;
            if (Day>daysinmonth) Day=daysinmonth; 
           cout <<"------------------------------------------------------------------------------------" << endl;	    
           cout <<"   >>>invalid date! It will be replaced by the closest valid date.<<<" << endl;
           cout <<"------------------------------------------------------------------------------------" << endl;	    	   
           delem=new Date (Day, Month, Year); 
           result.addr=delem;
    } 
  return 0;
}

/*
4.4 Definition of Operators
*/

const string DaySpec =
  "(<text>(date) -> int</text---><text>extract the day info. from a date.</text--->)";

const string MonthSpec =
  "(<text>(date) -> int</text---><text>extract the month info. from a date.</text--->)";

const string YearSpec =
  "(<text>(date) -> int</text---><text>extract the year info. from a date.</text--->)";

const string EarlierSpec =
  "(<text>(date date) -> bool</text---><text>earlier predicate.</text--->)";

const string EqualSpec =
  "(<text>(date date) -> bool</text---><text>equal predicate.</text--->)";

const string LaterSpec =
  "(<text>(date date) -> bool</text---><text>later predicate.</text--->)";

const string DateSpec =
  "(<text>(int int int) -> date</text---><text>to generate a date. </text--->)";

/*
Used to explain the signature and the meaning of the day, month, year, <,=, >, and opdate operators.

*/

Operator day (
	"day", 			//name
	DaySpec, 	     	//specification
	dayFun,			//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function 
	DateInt			//type mapping 
);	

Operator month (
	"month", 		//name
	MonthSpec,	         	//specification
	monthFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function 
	DateInt			//type mapping 
);	

Operator year (
	"year", 			//name
	YearSpec, 	       	//specification
	yearFun,			//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function 
	DateInt			//type mapping 
);	

Operator earlier (
	"<", 			//name
	EarlierSpec,		//specification
	earlierFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function 
	DateDateBool		//type mapping 
);	

Operator opequal (
	"=", 			//name
	EqualSpec,		//specification
	equalFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function 
	DateDateBool		//type mapping 
);	

Operator later (
	">", 			//name
	LaterSpec,		//specification
	laterFun,			//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function 
	DateDateBool		//type mapping 
);	

Operator opdate (
	"opdate", 		//name
	DateSpec,		//specification
	dateFun,			//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function 
	IntIntIntDate		//type mapping 
);	
/*
5 Creating the Algebra

*/

class DateAlgebra : public Algebra
{
 public:
  DateAlgebra() : Algebra()
  {
    AddTypeConstructor( &date );

    date.AssociateKind("DATA");  
 
    AddOperator( &day );
    AddOperator( &month ); 
    AddOperator( &year );    
    AddOperator( &earlier );
    AddOperator( &opequal);
    AddOperator( &later );
    AddOperator( &opdate );
  }
  ~DateAlgebra() {};
};

DateAlgebra dateAlgebra; 

/*
6 Initialization
*/

extern "C"
Algebra*
InitializeDateAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&dateAlgebra);
}

