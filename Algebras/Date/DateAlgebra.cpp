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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[->] [$\rightarrow$]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Implementation of the Date Algebra


December 11-26, 2002 Zhiming Ding

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

[TOC]


1 Preliminaries

This algebra provides one type constructor ~date~, and seven operators:
\verb+<+ , = , \verb+>+, ~year[_]of~, ~month[_]of~, ~day[_]of~, and ~thedate~.

Signatures of these operators are listed below:

  * \verb+<+ , = , \verb+>+

----    date x date -> bool
----
        compare two dates.


  * year[_]of, month[_]of, day[_]of

----    date -> int
----
        extract year, month, day information from a date.


  * thedate

----    int x int x int -> date
----
        generates a date according to the specified year, month,
        and day information


The algebra provides basic checks on the validity of a date.
For instance, Fabruary in leap years
(every 4 years except 100th year, and every 400th year) has
29 days, and Fabruary in normal years only
has 28 days.


1.1 Includes

*/

#include "Attribute.h"


#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "StringUtils.h"
#include "ListUtils.h"
#include <iostream>
#include <string>
#include <stdio.h>

using namespace std;
extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

/*

1.2 Date Validating Function

This function checks whether an input date is valid. The month
and the day must be within valid
boundaries. However, the year can be any integer. For instance, -100 represents
the year of 100BC.

*/

bool isdate(int  Day, int Month, int Year)
{
    bool res=true;
    bool leapyear=false;
    int daysinmonth=0;
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

In order to use ~date~ as an attribute type in tuple definitions,
we must derive the class ~Date~ from ~Attribute~.

*/

class Date: public Attribute
{
 public:
  Date(bool Defined, int Day, int Month, int Year);
  Date();
  ~Date();
  int      GetDay() const;
  int      GetMonth() const;
  int      GetYear() const;
  void     SetDay( int Day);
  void     SetMonth( int Yonth);
  void     SetYear( int Year);
  void     Set(bool Defined,  int Day, int Month, int Year);
/*
~readFrom~

Sets this date value to the given string. accepted formats are
yyy-mm-dd and dd.mm.yyyy where leading zeros can be ommitted.
If the string  does not represent a valid day, this Date value
remains unchanged and false is returned.

*/
  bool     readFrom(string& d);

/*
~ReadFromString~

works similar to the readFrom function. In contrast to that function,
this Date is set to be false in the string does not represent a
valid date.

*/
 virtual  void ReadFromString(string d);
/*
getCsvStr

Returns a string represnetation of this date value.

*/

  virtual string getCsvStr() const;

  void     successor(const Date *d, Date *s) const;
/*************************************************************************

  The following 8 virtual functions: IsDefined(), SetDefined(), HashValue(),
  CopyFrom(), Compare(), Adjacent(), Clone(), Print(), need to be defined if
  we want to use ~date~ as an attribute type in tuple definitions.

*************************************************************************/

  bool     IsDefined() const;
  void     SetDefined(bool Defined);
  size_t   Sizeof() const;
  size_t   HashValue() const;
  void     CopyFrom(const Attribute* right);
  int      Compare(const Attribute * arg) const;
  bool     Adjacent(const Attribute * arg) const;
  Date*    Clone() const;
  ostream& Print( ostream &os ) const;
  static const string BasicType(){
      return "date";
  }
  static const bool checkType(const ListExpr list){
     return listutils::isSymbol(list, BasicType());
  }


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

int Date::GetDay() const {return day;}

int Date::GetMonth() const {return month;}

int Date::GetYear() const {return year;}

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

bool Date::readFrom(string& s){

  stringutils::trim(s);

  if(s.length()>99){
    return false;
  }

  char *i, *j;
  int dot = 0;
  int hyphen = 0;
  int Year;
  int Month;
  int Day;
  char buf[100];
  const char* c_string = s.c_str();

  if ( (strcmp(c_string,"-")==0) ) // undefined, special notation
  {
    Set(false, 0,0,0);
    return true;
  }

   //"1998-02-01" or "1999-2-1" or "1.2.1998" or "02.01.1999"
  strcpy(buf, c_string);
  int bufLen=strlen(buf);


  //basic check on date format
  for ( i=buf; i<buf+bufLen; i++) {
     if (*i=='.') dot++;
     if (*i=='-') hyphen++;
     if ((*i!='.') && (*i!='-') && ((*i<'0') || (*i>'9'))) {
        // invalid character
          return false;
      }
   }


    if ((hyphen == 2) && (dot == 0)) //format is "1998-02-01"
    {
      //extract the year, month, day information from the date
      i=buf; j=i;
      while ((*j!='-') && (j<buf+bufLen))  j++;
      *j=0;
      Year=atoi(i);

      i=j+1; j=i;
      while ((*j!='-') && (j<buf+bufLen))  j++;
      *j=0;
      Month=atoi(i);

      i=j+1; j=i;
      while ((*j!='-') && (j<buf+bufLen))  j++;
      *j=0;
      Day=atoi(i);
    } else if ((hyphen == 0) && (dot == 2)) { //format is "1.2.1998"

        //extract the year, month, day information from the date
        i=buf; j=i;
        while ((*j!='.') && (j<buf+bufLen))  j++;
        *j=0;
        Day=atoi(i);

        i=j+1; j=i;
        while ((*j!='.') && (j<buf+bufLen))  j++;
        *j=0;
        Month=atoi(i);

        i=j+1; j=i;
        while ((*j!='.') && (j<buf+bufLen))  j++;
        *j=0;
        Year=atoi(i);
   } else {
        return false;
   }

   if (isdate(Day, Month, Year)) {
       Set(true, Day, Month, Year);
       return true;
   } else {
       return false;
   }
}

void Date::ReadFromString(string s){
   if(!readFrom(s)){
      SetDefined(false);
   }
}

string Date::getCsvStr() const{
  stringstream ss;
  Print(ss);
  return ss.str();
}



/*************************************************************************

  In the following, we give the definitions of the 9 virtual functions
  which are needed
  if we want to use ~date~ as an attribute type in tuple definitions.

*************************************************************************/

bool Date::IsDefined() const {return (defined); }

void Date::SetDefined(bool Defined) {defined = Defined; }

size_t Date::Sizeof() const
{
  return sizeof( *this );
}

size_t Date::HashValue() const
{
  if(!defined)  return (0);
  unsigned long h;
  h=5*(5*day+month)+year;
  return size_t(h);
}

void Date::CopyFrom(const Attribute* right)
{
  const Date * d = (const Date*)right;
  defined = d->defined;
  day = d->day;
  month = d->month;
  year = d->year;
}

/*

The function Compare() defines a total order on the data type ~date~.

*/

int Date::Compare(const Attribute * arg) const
{
 int res=0;
 const Date * d = (const Date* )(arg);
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
                         else if ((this->GetDay())<(d->GetDay())) res=-1;
                              else res=0;
       }
  return (res);
}

void Date::successor(const Date *d, Date *s) const
{
    assert(isdate(d->GetDay(), d->GetMonth(), d->GetYear()));

    int Year, Month, Day;
    Year=d->GetYear(); Month=d->GetMonth(); Day=d->GetDay();
//    cout<<"OldDate"<<Year<<":"<<Month<<":"<<Day<<endl;
    bool leapyear=false;
    int daysinmonth=0;
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

bool Date::Adjacent(const Attribute *arg) const
{
  const Date *d = (const Date *)arg;
  if( this->Compare( d ) == 0 ) return 1;  //both undefined or they are equal

  if (!IsDefined() || !(arg->IsDefined()))
     //one is undefined and another defined
     return 0;
  else      //both defined and they are not equal
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

Date* Date::Clone() const
{
  return (new Date( *this));
}

ostream& Date::Print(ostream &os) const
{ if(!IsDefined()) return os << "undef";
  return (os << day << "." << month << "." << year);
}


/*

2.2 List Representation

The list representation of a date is

----        (isdefined dd mm yy)
----

2.3 ~In~ and ~Out~ Functions

*/
ListExpr
OutDate( ListExpr typeInfo, Word value )
{
  Date* date;
  string outputStr;
  size_t buf_len = 100;
  char buf[buf_len];

  date = (Date*)(value.addr);
  if (date->IsDefined())
  {
    snprintf(buf, buf_len, "%02d.%02d.%d", date->GetDay(),
            date->GetMonth(), date->GetYear());   //eg. "01.02.1993"
  }
  else
  {
    strcpy(buf,"-");
  }
  outputStr = buf;
  return (nl->StringAtom(outputStr));
}

Word
InDate( const ListExpr typeInfo, const ListExpr instance,
        const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if (listutils::isSymbolUndefined(instance)) {
    Date* date = new Date(false,0,0,0);
    correct = true;
    return SetWord(date);
  }
  
  if (nl->AtomType(instance)!=StringType){
     correct = false;
     return SetWord(Address(0));
  }
  Date* date = new Date(false,0,0,0);
  string s = nl->StringValue(instance);
  if(!date->readFrom(s)){
    delete date;
    correct = false;
    return SetWord(Address(0));
  }
  correct = true;
  return SetWord(date);

}

/***********************************************************************

The following 5 functions must be defined if we want to use ~date~ as
an attribute type in tuple definitions.

************************************************************************/

Word
CreateDate( const ListExpr typeInfo )
{
  return (SetWord( new Date( false, 0, 0, 0 )));
}

void
DeleteDate( const ListExpr typeInfo, Word& w )
{
  delete (Date*) w.addr;
  w.addr = 0;
}

void
CloseDate( const ListExpr typeInfo, Word& w )
{
  delete (Date*) w.addr;
  w.addr = 0;
}

Word
CloneDate( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((Date *)w.addr)->Clone() );
}

int
SizeOfDate()
{
  return sizeof(Date);
}

void*
CastDate( void* addr )
{
  return (new (addr) Date);
}

/*

2.4 Function Describing the Signature of the Type Constructor

This one works for type constructor ~date~ , which is an ``atomic'' type.

*/

ListExpr
DateProperty()
{
  ListExpr listreplist = nl->TextAtom();
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(listreplist,
    "Either \"<day>.<month>.<year>\" or \"<year>-<month>-<day>\"");
  nl->AppendText(examplelist,
    "\"9.5.1955\" or \"09.05.1955\" or \"1955-5-9\" or \"1955-05-09\"");

  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom(Date::BasicType()),
                             listreplist,
                             examplelist)));
}

/*

2.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
the type constructor ~date~ does not have arguments, this is trivial.

*/

bool
CheckDate( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual(type, Date::BasicType() ));
}
/*

2.6 Creation of the Type Constructor Instance

*/
TypeConstructor date(
        Date::BasicType(),                      //name
        DateProperty,                //property function describing signature
        OutDate,  InDate,            //Out and In functions
        0,        0,                 //SaveToList and RestoreFromList functions
        CreateDate, DeleteDate,      //object creation and deletion
        0, 0, CloseDate, CloneDate,  //object open, save, close, and clone
        CastDate,                    //cast function
        SizeOfDate,                  //sizeof function
        CheckDate );                 //kind checking function

/*

3 Creating Operators

3.1 Type Mapping Function

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol
~typeerror~.

*/

ListExpr
DateInt( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength(args) == 1 )
  {
    arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, Date::BasicType()))
    return nl->SymbolAtom(CcInt::BasicType());
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}


ListExpr
DateDateBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, Date::BasicType()) &&
      nl->IsEqual(arg2, Date::BasicType()) )
    return nl->SymbolAtom(CcBool::BasicType());
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}


ListExpr
IntIntIntDate( ListExpr args )
{
  ListExpr arg1, arg2, arg3;
  if ( nl->ListLength(args) == 3 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    arg3 = nl->Third(args);
    if ( nl->IsEqual(arg1, CcInt::BasicType()) &&
         nl->IsEqual(arg2, CcInt::BasicType()) &&
         nl->IsEqual(arg3, CcInt::BasicType()))
      return nl->SymbolAtom(Date::BasicType());
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}


ListExpr str2dateTM(ListExpr args){
  string err = " string expected";
  if(nl->ListLength(args)!=1){
     return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(!listutils::isSymbol(arg1,CcString::BasicType())){
     return listutils::typeError(err);
  }
  return nl->SymbolAtom(Date::BasicType());

}


/*
3.3 Value Mapping Functions

*/

int
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

int
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

int
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

int
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

int
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

int
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

int
dateFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  CcInt *dd;
  CcInt *mm;
  CcInt *yy;
  dd = (CcInt *)args[0].addr;
  mm = (CcInt *)args[1].addr;
  yy = (CcInt *)args[2].addr;

  result = qp->ResultStorage(s);

  int Day=dd->GetIntval();
  int Month=mm->GetIntval();
  int Year=yy->GetIntval();
  //bool leapyear;
  //int daysinmonth;
  Date* res = (Date*) result.addr;
  if (isdate(Day, Month, Year)) {
      res->Set(true, Day, Month, Year);
  } else {
       res->Set(false, 0, 0, 0);
  }
  return 0;
}


int
str2dateFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  Date* res = static_cast<Date*>(result.addr);
  CcString* str = static_cast<CcString*>(args[0].addr);
  if(!str->IsDefined()){
    res->SetDefined(false);
  } else {
    string st = str->GetValue();
    res->ReadFromString(st);
  }
  return 0;
}

/*

3.4 Definition of Operators

*/

const string DaySpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                        "\"Example\" )"
                             "( <text>(date) -> int</text--->"
                               "<text>day_of ( _ )</text--->"
                               "<text>extract the day info. from a date."
                               "</text--->"
                               "<text>query day_of ( date1 )</text--->"
                               ") )";

const string MonthSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                             "( <text>(date) -> int</text--->"
                               "<text>month_of ( _ )</text--->"
                               "<text>extract the month info. from a date."
                               "</text--->"
                               "<text>query month_of ( date1 )</text--->"
                               ") )";

const string YearSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(date) -> int</text--->"
                               "<text>year_of ( _ )</text--->"
                               "<text>extract the year info. from a date."
                               "</text--->"
                               "<text>query year_of ( date1 )</text--->"
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

const string str2dateSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" )"
    "( <text>string -> date</text--->"
    "<text> str2date(_)"
    "</text--->"
    "<text>Converts a string to a date. If the string is undefined "
    "or is not a valid date, the result will be undefined. "
    "Allowed formats are dd.mm.yyyy and yyyy-mm-dd where leading zeros "
    " can be omitted."
    ".</text--->"
    "<text>query str2date(\"2011-05-23\") = str2date(\"23.5.2011\")</text--->"
     ") )";
/*
The above strings are used to explain the signature and the
meaning of operators.

*/

Operator day (
        "day_of",                     //name
        DaySpec,                      //specification
        dayFun,                      //value mapping
        Operator::SimpleSelect,      //trivial selection function
        DateInt                      //type mapping
);

Operator month (
        "month_of",                 //name
        MonthSpec,                  //specification
        monthFun,                   //value mapping
        Operator::SimpleSelect,     //trivial selection function
        DateInt                     //type mapping
);

Operator year (
        "year_of",                  //name
        YearSpec,                   //specification
        yearFun,                    //value mapping
        Operator::SimpleSelect,     //trivial selection function
        DateInt                     //type mapping
);

Operator earlier (
        "<",                        //name
        EarlierSpec,                //specification
        earlierFun,                 //value mapping
        Operator::SimpleSelect,     //trivial selection function
        DateDateBool                //type mapping
);

Operator opequal (
        "=",                        //name
        EqualSpec,                  //specification
        equalFun,                   //value mapping
        Operator::SimpleSelect,     //trivial selection function
        DateDateBool                //type mapping
);

Operator later (
        ">",                        //name
        LaterSpec,                  //specification
        laterFun,                   //value mapping
        Operator::SimpleSelect,     //trivial selection function
        DateDateBool                //type mapping
);

Operator thedate (
        "thedate",                 //name
        DateSpec,                  //specification
        dateFun,                   //value mapping
        Operator::SimpleSelect,    //trivial selection function
        IntIntIntDate              //type mapping
);


Operator str2date (
        "str2date",                 //name
        str2dateSpec,                  //specification
        str2dateFun,                   //value mapping
        Operator::SimpleSelect,    //trivial selection function
        str2dateTM              //type mapping
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

    date.AssociateKind(Kind::DATA());
    date.AssociateKind(Kind::CSVIMPORTABLE());
    date.AssociateKind(Kind::CSVEXPORTABLE());

    AddOperator( &day );
    AddOperator( &month );
    AddOperator( &year );
    AddOperator( &earlier );
    AddOperator( &opequal);
    AddOperator( &later );
    AddOperator( &thedate );
    AddOperator( &str2date );
  }
  ~DateAlgebra() {};
};


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
InitializeDateAlgebra( NestedList* nlRef,
                       QueryProcessor* qpRef,
                       AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (new DateAlgebra());
}

/*

6 Other work

6.1 Syntax Specification

We need to write the following DateAlgebra.spec file to indicate the syntax
of the operations in the date algebra:

----    operator year[_]of alias YEAR[_]OF pattern op ( _ )
        operator month[_]of alias MONTH[_]OF pattern op ( _ )
        operator day[_]of alias DAY[_]OF pattern op ( _ )
        operator thedate alias THEDATE pattern op ( _, _, _ )
----

//[->] [$\rightarrow$]

6.2 Display Function

We need to add the following display function into DisplayTTY.h and
DisplayTTY.cpp
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

