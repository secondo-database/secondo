/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[->] [$\rightarrow$]
//[TOC] [\tableofcontents]

[1] Implementation of the Date Algebra


December 11-26, 2002 Zhiming Ding


[TOC]


1 Preliminaries

This algebra provides one type constructor ~date~, and seven operators: \verb+<+ , = , \verb+>+, ~year~, ~month~, ~day~, and ~thedate~.

Signatures of these operators are listed below:

  * \verb+<+ , = , \verb+>+

----    date x date -> bool
----
        compare two dates.


  * year, month, day

----    date -> int
----
        extract year, month, day information from a date.


  * thedate

----    int x int x int -> date
----
        generates a date according to the specified year, month, and day information


The algerbra provides basic checks on the validity of a date. For instance, Fabruary in leap years
(every 4 years except 100th year, and every 400th year) has 29 days, and Fabruary in normal years only
has 28 days.


1.1 Includes

*/

#include "StandardAttribute.h"

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include <iostream>
#include <string>
#include <stdio.h>

static NestedList* nl;
static QueryProcessor* qp;

/*

1.2 Date Validating Function

This function checks whether an input date is valid. The month and the day must be within valid
boundaries. However, the year can be any integer. For instance, -100 represents
the year of 100BC.

*/

bool isdate(int  Day, int Month, int Year)
{
    bool res=true;
    bool leapyear;
    int daysinmonth;
    if  (((Year % 4==0) && (Year % 100!=0)) || (Year % 400==0))
    	leapyear=true; else leapyear=false;
    if ((Month<1)||(Month>12)) res=false;
	  else
	  {
	    switch (Month)
	      {
	  	  case 1:
  		  case 3:
  		  case 5:
		  case 7:
  		  case 8:
  		  case 10:
  		  case 12: daysinmonth=31; break;
 		  case 4:
          	  case 6:
          	  case 9:
          	  case 11: daysinmonth=30; break;
 	      	  case 2:  if (leapyear) daysinmonth=29; else daysinmonth=28;
	      }
	    if ((Day<1)||(Day>daysinmonth)) res=false;
	  }
    return res;
}

/*

2 Type Constructor ~date~

2.1 Data Structure - Class ~Date~

In order to use ~date~ as an attribute type in tuple definitions, we must derive the class ~Date~ from ~StandardAttribute~.

*/

class Date: public StandardAttribute
{
 public:
  Date(bool Defined, int Day, int Month, int Year);
  Date();
  ~Date();
  int      GetDay();
  int      GetMonth();
  int      GetYear();
  void     SetDay( int Day);
  void     SetMonth( int Yonth);
  void     SetYear( int Year);
  void     Set(bool Defined,  int Day, int Month, int Year);
  void     successor(Date *d, Date *s);
/*************************************************************************

  The following 10 virtual functions: IsDefined(),SetDefined(), HashValue(),
  CopyFrom(), Compare(), Adjacent() Sizeof(), Clone(), Print(), need to be defined if
  we want to use ~date~ as an attribute type in tuple definitions.

*************************************************************************/

  bool     IsDefined() const;
  void     SetDefined(bool Defined);
  size_t   HashValue();
  void	   CopyFrom(StandardAttribute* right);
  int      Compare(Attribute * arg);
  int      Adjacent(Attribute * arg);
  int      Sizeof() const;
  Date*    Clone();
  ostream& Print( ostream &os );

 private:
  int day;
  int month;
  int year;
  bool defined;
};


Date::Date(bool Defined, int Day, int Month, int Year)
{
   defined = Defined;
   day = Day;
   month = Month;
   year = Year;
}

Date::Date()  {}

Date::~Date() {}

int Date::GetDay() {return day;}

int Date::GetMonth() {return month;}

int Date::GetYear() {return year;}

void Date::SetDay(int Day) {day = Day;}

void Date::SetMonth(int Month) {month = Month;}

void Date::SetYear(int Year) {year = Year;}

void Date::Set(bool Defined, int Day, int Month, int Year)
{
    defined = Defined;
    day = Day;
    month = Month;
    year = Year;
}

/*************************************************************************

  In the following, we give the definitions of the 9 virtual functions which are needed
  if we want to use ~date~ as an attribute type in tuple definitions.

*************************************************************************/

bool Date::IsDefined() const {return (defined); }

void Date::SetDefined(bool Defined) {defined = Defined; }

size_t Date::HashValue()
{
  if(!defined)  return (0);
  unsigned long h;
  h=5*(5*day+month)+year;
  return size_t(h);
}

void Date::CopyFrom(StandardAttribute* right)
{
  Date * d = (Date*)right;
  defined = d->defined;
  day = d->day;
  month = d->month;
  year = d->year;
}

/*

The function Compare() defines a total order on the data type ~date~.

*/

int Date::Compare(Attribute * arg)
{
 int res=0;
 Date * d = (Date* )(arg);
 if ( !d ) return (-2);

 if (!IsDefined() && !(arg->IsDefined()))  res=0;
 else if (!IsDefined())  res=-1;
      else  if (!(arg->IsDefined())) res=1;
            else
             {
              if ((this->GetYear()) > (d->GetYear())) res=1;
              else if ((this->GetYear()) < (d->GetYear())) res=-1;
                   else
                      if ((this->GetMonth())>(d->GetMonth())) res=1;
                      else if ((this->GetMonth())<(d->GetMonth())) res=-1;
                           else
                              if ((this->GetDay())>(d->GetDay())) res=1;
                              else if ((this->GetDay())<(d->GetDay())) res=1;
                                   else res=0;
            }
  return (res);
}

void Date::successor(Date *d, Date *s)
{
    assert(isdate(d->GetDay(), d->GetMonth(), d->GetYear()));

    int Year, Month, Day;
    Year=d->GetYear(); Month=d->GetMonth(); Day=d->GetDay();
//    cout<<"OldDate"<<Year<<":"<<Month<<":"<<Day<<endl;
    bool leapyear;
    int daysinmonth;
    if  (((Year % 4==0) && (Year % 100!=0)) || (Year % 400==0))
             leapyear=true;
    else  leapyear=false;

   switch (Month)
	{
	  case 1:
 	  case 3:
 	  case 5:
	  case 7:
 	  case 8:
 	  case 10:
 	  case 12: daysinmonth=31; break;
 	  case 4:
          	  case 6:
          	  case 9:
          	  case 11: daysinmonth=30; break;
       	  case 2:  if (leapyear) daysinmonth=29; else daysinmonth=28;
	}
    if ((Day<daysinmonth))
           Day++;
    else //==
    {
           Day=1;
            if (Month<12) Month++;
            else
	    {
		Month=1;
		Year++;
                    }
    }
    s->year=Year; s->month=Month; s->day=Day; s->defined=true;
//    cout<<"NewDate"<<Year<<":"<<Month<<":"<<Day<<endl;
}

int Date::Adjacent(Attribute *arg)
{
  Date *d = (Date *)arg;
  if( this->Compare( d ) == 0 ) return 1;                 //both undefined or they are equal

  if (!IsDefined() || !(arg->IsDefined()))  return 0;  //one is undefined and another defined
  else					  //both defined and they are not equal
  {
      Date * auxdate=new Date();

      successor(this, auxdate);
      if (auxdate->Compare(d)==0)
      {
	  delete auxdate;
	  return 1;
      }

      successor(d, auxdate);
      if (this->Compare(auxdate)==0)
      {
	  delete auxdate;
	  return 1;
      }
      delete auxdate;
      return 0;
  }
}

int  Date::Sizeof() const {return sizeof(Date);}

Date*  Date::Clone() {return (new Date( *this));}

ostream& Date::Print(ostream &os)
{
  return (os << year << "-"<<month <<"-"<<day);
}


/*

2.2 List Representation

The list representation of a date is

----	(isdefined dd mm yy)
----

2.3 ~In~ and ~Out~ Functions

*/
static  ListExpr
OutDate( ListExpr typeInfo, Word value )
{
  Date* date;
  string outputStr;
  char buf[100];

  date = (Date*)(value.addr);
  if (date->IsDefined())
  {
    sprintf(buf, "%d-%02d-%02d", date->GetYear(), date->GetMonth(), date->GetDay());   //eg. "1993-02-01"
  }
  else
  {
    strcpy(buf,"-");
  }
  outputStr = buf;
  return (nl->StringAtom(outputStr));
}

static Word
InDate( const ListExpr typeInfo, const ListExpr instance, const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Date* newdate;
  string inputStr;
  char *i, *j;
  int slash=0;
  char buf[100];

  if (nl->IsAtom(instance) && nl->AtomType(instance)==StringType)
      inputStr= nl->StringValue( instance);
  else
  {      cout <<">>>invalid date - not a string type!<<<"<<endl;
         correct = false;
         return SetWord(Address(0));
  }
  const char* c_string = inputStr.c_str();
  if (strcmp(c_string,"-")==0)  //"-"
  {
    correct = true;
    newdate = new Date(false, 0, 0, 0);
    return SetWord(newdate);
  }
  else   //"1998-02-01" or "1999-2-1"
  {
    strcpy(buf, c_string);
    int bufLen=strlen(buf);
    //basic check on date format
    for ( i=buf; i<buf+bufLen; i++)
    {
	if (*i=='-') slash++;
	if ((*i!='-') && ((*i<'0') || (*i>'9')))
	{
	    cout <<">>>invalid date!<<<"<<endl;
	    correct = false;
	    return SetWord(Address(0));
	}
    }
    if (slash!=2)
    {
         cout <<">>>invalid date!<<<"<<endl;
         correct = false;
         return SetWord(Address(0));
    }
    //extract the year, month, day information from the date
    i=buf; j=i;
    while ((*j!='-') && (j<buf+bufLen))  j++;
    *j=0;
    int Year=atoi(i);

    i=j+1; j=i;
    while ((*j!='-') && (j<buf+bufLen))  j++;
    *j=0;
    int Month=atoi(i);

    i=j+1; j=i;
    while ((*j!='-') && (j<buf+bufLen))  j++;
    *j=0;
    int Day=atoi(i);

    if (isdate(Day, Month, Year))
    {
         correct = true;
         newdate = new Date(true, Day, Month, Year);
         return SetWord(newdate);
    }
    else
    {
         cout <<">>>invalid date!<<<"<<endl;
         correct = false;
         return SetWord(Address(0));
     }
  }
}

/***********************************************************************

The following 5 functions must be defined if we want to use ~date~ as an attribute type in tuple definitions.

************************************************************************/

static Word
CreateDate( const ListExpr typeInfo )
{
  return (SetWord( new Date( false, 0, 0, 0 )));
}

static void
DeleteDate( Word& w )
{
  delete (Date*) w.addr;
  w.addr = 0;
}

static void
CloseDate( Word& w )
{
  delete (Date*) w.addr;
  w.addr = 0;
}

static Word
CloneDate( const Word& w )
{
  return SetWord( ((Date *)w.addr)->Clone() );
}

static void*
CastDate( void* addr )
{
  return (new (addr) Date);
}

/*

2.4 Function Describing the Signature of the Type Constructor

This one works for type constructor ~date~ , which is an ``atomic'' type.

*/

static ListExpr
DateProperty()
{
  ListExpr listreplist = nl->TextAtom();
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(listreplist, "\"<year>-<month>-<day>\"");
  nl->AppendText(examplelist, "\"2003-09-05\"");
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
	                     nl->StringAtom("Example Type List"),
			     nl->StringAtom("List Rep"),
			     nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
	                     nl->StringAtom("date"),
			     listreplist,
			     examplelist)));
}

/*

2.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
the type constructor ~date~ does not have arguments, this is trivial.

*/

static bool
CheckDate( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual(type, "date" ));
}
/*

2.6 Creation of the Type Constructor Instance

*/
TypeConstructor date(
	"date",		            	    //name
	DateProperty, 		            //property function describing signature
	OutDate,  InDate,		    //Out and In functions
        0,        0,                        //SaveToList and RestoreFromList functions
	CreateDate, DeleteDate,		    //object creation and deletion
        0, 0, CloseDate, CloneDate,	    //object open, save, close, and clone
	CastDate,  		            //cast function
	CheckDate,			    //kind checking function
	0, 				    //predef. pers. function for model
	TypeConstructor::DummyInModel,
	TypeConstructor::DummyOutModel,
	TypeConstructor::DummyValueToModel,
	TypeConstructor::DummyValueListToModel );



/*

3 Creating Operators

3.1 Type Mapping Function

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol
~typeerror~.

*/

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


static ListExpr
IntIntIntDate( ListExpr args )
{
  ListExpr arg1, arg2, arg3;
  if ( nl->ListLength(args) == 3 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    arg3 = nl->Third(args);
    if ( nl->IsEqual(arg1, "int") &&
         nl->IsEqual(arg2, "int") &&
	 nl->IsEqual(arg3, "int"))
      return nl->SymbolAtom("date");
  }
  return nl->SymbolAtom("typeerror");
}


/*

3.2 Selection Function

Selection Function is used to select one of several evaluation functions for an overloaded
operator, based on the types of the arguments. In case of a non-overloaded
operator, we just have to return 0.

operators ~day~, ~month~, ~year~, ~\verb+<+~, ~=~, ~\verb+>+~, ~thedate~ are all non-overloaded
operators, therefore we only need to return 0.

*/


static int
simpleSelect (ListExpr args ) {return 0;}


/*

3.3 Value Mapping Functions

*/

static int
dayFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  Date* d;
  d = ((Date*)args[0].addr);

  result = qp->ResultStorage(s);
  int res=d->GetDay();

  if (d->IsDefined())
       ((CcInt*)result.addr)->Set(true, res);
  else ((CcInt*)result.addr)->Set(false, res);
  return 0;
}

static int
monthFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  Date* d;
  d = ((Date*)args[0].addr);

  result = qp->ResultStorage(s);
  int res=d->GetMonth();

  if (d->IsDefined())
       ((CcInt*)result.addr)->Set(true, res);
  else ((CcInt*)result.addr)->Set(false, res);
  return 0;
}

static int
yearFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  Date* d;
  d = ((Date*)args[0].addr);

  result = qp->ResultStorage(s);
  int res=d->GetYear();

  if (d->IsDefined())
       ((CcInt*)result.addr)->Set(true, res);
  else ((CcInt*)result.addr)->Set(false, res);
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

  if (!(d1->IsDefined()) && !(d2->IsDefined())) res=false;
  else if (!(d1->IsDefined()) && (d2->IsDefined())) res=true;
       else if ((d1->IsDefined()) && !(d2->IsDefined())) res=false;
 	    else
	       { if ((d1->GetYear()) > (d2->GetYear())) res=false;
 		 else if ((d1->GetYear()) < (d2->GetYear())) res=true;
		      else
			if ((d1->GetMonth())>(d2->GetMonth())) res=false;
			else if ((d1->GetMonth())<(d2->GetMonth())) res=true;
			     else
				if ((d1->GetDay())>=(d2->GetDay()))
				     res=false;
				else res=true;
		}

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

  if (!(d1->IsDefined()) && !(d2->IsDefined())) res=true;
  else  if ((d1->IsDefined()) && (d2->IsDefined()) &&
	    (d1->GetYear() == d2->GetYear()) &&
	    (d1->GetMonth()== d2->GetMonth()) &&
	    (d1->GetDay()  == d2->GetDay()))
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

  if (!(d1->IsDefined()) && !(d2->IsDefined())) res=false;
  else if (!(d1->IsDefined()) && (d2->IsDefined())) res=false;
       else if ((d1->IsDefined()) && !(d2->IsDefined())) res=true;
	    else
	       {
	        if ((d1->GetYear()) > (d2->GetYear())) res=true;
		else if ((d1->GetYear()) < (d2->GetYear())) res=false;
		     else
			if ((d1->GetMonth())>(d2->GetMonth())) res=true;
			else if ((d1->GetMonth())<(d2->GetMonth())) res=false;
			     else
				if ((d1->GetDay())<=(d2->GetDay()))
				     res=false;
				else res=true;
		}

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
  //bool leapyear;
  //int daysinmonth;

  if (isdate(Day, Month, Year))
   {
      delem=new Date (true, Day, Month, Year);
      result.addr=delem;
   }
  else
   {
       cout <<"   >>>invalid date, replaced by UNDEFINED!<<<" << endl;
       delem=new Date (false, 0, 0, 0);
       result.addr=delem;
    }
  return 0;
}

/*

3.4 Definition of Operators

*/

const string DaySpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                        "\"Example\" )"
                             "( <text>(date) -> int</text--->"
			       "<text>day ( _ )</text--->"
			       "<text>extract the day info. from a date."
			       "</text--->"
			       "<text>query day ( date1 )</text--->"
			       ") )";

const string MonthSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                             "( <text>(date) -> int</text--->"
			       "<text>month ( _ )</text--->"
			       "<text>extract the month info. from a date."
			       "</text--->"
			       "<text>query month ( date1 )</text--->"
			       ") )";

const string YearSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(date) -> int</text--->"
			       "<text>year ( _ )</text--->"
			       "<text>extract the year info. from a date."
			       "</text--->"
			       "<text>query year ( date1 )</text--->"
			       ") )";

const string EarlierSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" )"
                             "( <text>(date date) -> bool</text--->"
			      " <text>_ < _</text--->"
			       "<text>Earlier predicate.</text--->"
			       "<text>query date1 < date2</text--->"
			       ") )";
const string EqualSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                             "( <text>(date date) -> bool</text--->"
			       "<text>_ = _</text--->"
			       "<text>Equal predicate.</text--->"
			       "<text>query date1 = date2</text--->"
			       ") )";

const string LaterSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                             "( <text>(date date) -> bool</text--->"
			       "<text>_ > _</text--->"
			       "<text>Later predicate.</text--->"
			       "<text>query date1 > date2</text--->"
			       ") )";

const string DateSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(int int int) -> date</text--->"
			     "<text>thedate ( <day>, <month>, <year> ) where"
			     " <day>, <month> and <year> are of type int"
			     "</text--->"
			     "<text>To generate a date.</text--->"
			     "<text>let date1 = thedate(5,4,2003)</text--->"
			      ") )";

/*
The above strings are used to explain the signature and the meaning of operators.

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
	MonthSpec,	        //specification
	monthFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function
	DateInt			//type mapping
);

Operator year (
	"year", 		//name
	YearSpec, 		//specification
	yearFun,		//value mapping
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
	laterFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function
	DateDateBool		//type mapping
);

Operator thedate (
	"thedate", 		//name
	DateSpec,		//specification
	dateFun,		//value mapping
	Operator::DummyModel,	//dummy model mapping, defined in Algebra.h
	simpleSelect,		//trivial selection function
	IntIntIntDate		//type mapping
);
/*

4 Creating the Algebra

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
    AddOperator( &thedate );
  }
  ~DateAlgebra() {};
};

DateAlgebra dateAlgebra;

/*

5 Initialization

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
InitializeDateAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&dateAlgebra);
}

/*

6 Other work

6.1 Syntax Specification

We need to write the following DateAlgebra.spec file to indicate the syntax of the operations in the date algebra:

----    operator year alias YEAR pattern op ( _ )
	operator month alias MONTH pattern op ( _ )
	operator day alias DAY pattern op ( _ )
	operator thedate alias THEDATE pattern op ( _, _, _ )
----

//[->] [$\rightarrow$]

6.2 Display Function

We need to add the following display function into DisplayTTY.h and DisplayTTY.cpp
(under the secondo/UserInterface/ subdirectary) to display date correctly:

----    void
	DisplayTTY::DisplayDate(ListExpr type,ListExpr numType,ListExpr value)
	{
   	  ListExpr d, m, y;
   	  if( nl->IsAtom( value ) &&
	      nl->AtomType( value ) == SymbolType &&
	      nl->SymbolValue( value ) == "undef" )
    	  {
      	      cout << "UNDEFINED";
    	  }
   	  else
   	  {
      	      d =  nl->Second( value ) ;
      	      m =  nl->Third( value ) ;
      	      y = nl->Fourth( value );
      	      nl->WriteListExpr( d, cout );
      	      cout << ",";
      	      nl->WriteListExpr( m, cout );
      	      cout << ",";
      	      nl->WriteListExpr( y, cout );
   	  }
	}
----

*/

