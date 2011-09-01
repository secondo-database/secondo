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

//[_] [\_]
//[TOC] [\tableofcontents]
//[Title] [ \title{DateTime Algebra} \author{Thomas Behr} \maketitle]
//[times] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

[Title]
[TOC]

\noindent


0 Introduction

Time is a very important datatype. They are two different kinds
of time, namely  *duration* and *instant*. Both types
can be handled by the same data structure. Unfortunately, not
all combinations of types makes any sense in each operator, e.g.
the addition of two instants. To handle this problem, the type
information is included in the data-structure. The correct use
of the operators is checked via assert() callings. Note, that several
operations changes the type of the this-object, e.g. if  ~Minus~ is
called on an instant and an instant as argument, the this object will
be changed from instant to duration.

The Algebra provides the following operators.

\begin{tabular}{|l|l|l|}\hline
 *Operator*     & *Signature* & *Remarks* \\\hline
   +                & instant [times] duration [->] instant
                    & addition of the arguments \\\cline{2-2}
                    & duration [times] instant [->] instant
                    & possible change of the type \\\cline{2-2}
                    & duration [times] duration [->]  duration
                    & \\\hline
   -                & instant [times]  duration [->] instant
                    & difference of two time instances \\\cline{2-2}
                    & instant [times] instant [->] duration
                    & possible change of the type \\\cline{2-2}
                    & duration [times] duration [->] duration
                    & \\\hline
    *                & duration [times] real [->] duration
                    & the multiple of a duration \\\hline
   =, $<$, $>$      & instant [times] instant [->] bool
                    & the familiar comparisons \\\cline{2-2}
                    & duration [times] duration [->] bool
                    & \\\hline
   weekday          & instant [->] string
                    & the weekday in a human readable format \\\hline
   leapyear         & int [->] bool
                    & checks for leapyear \\\hline
year[_]of,month[_]of,day[_]of      & instant [->] int
                    & the date parts of an instant \\\hline
hour[_]of, minute[_]of,       & instant [->] int
                    & the time parts of an instant \\
second[_]of, millisecond[_]of &
                    &         \\\hline
now                 & [->] instant
                    & creates a new instant from the systemtime \\\hline
today               & [->] instant
                    & creates a new instant today at 0:00  \\\hline
\end{tabular}

1 Includes and Definitions

*/

#include "DateTime.h"
#include "BigInt.h"
#include <string>
#include <iostream>
#include <sstream>
#include "NestedList.h"
#include "Algebra.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "SecondoSystem.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "FTextAlgebra.h"
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include "LogMsg.h"
#include "ListUtils.h"
#include "StringUtils.h"
#include "Symbols.h"
#include <limits>

#define POS "DateTimeAlgebra.cpp:" << __LINE__

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;


static int64_t  min_VALUE = numeric_limits<int64_t>::min();
static int64_t  max_VALUE = numeric_limits<int64_t>::max();

static string begin_of_time="begin of time";
static string end_of_time="end of time";

static int64_t MAX_REPRESENTABLE = ((int64_t)2450000) * (int64_t)MILLISECONDS;
static int64_t MIN_REPRESENTABLE = ((int64_t)-2450000) * (int64_t)MILLISECONDS;


namespace datetime{


/*
~GetValue~

The function ~GetValue~ returns the integer value of an given
character. If this character does not represent any digit, -1
is returned.

*/
static int GetValue(const char c){
  switch(c){
    case '0' : return 0;
    case '1' : return 1;
    case '2' : return 2;
    case '3' : return 3;
    case '4' : return 4;
    case '5' : return 5;
    case '6' : return 6;
    case '7' : return 7;
    case '8' : return 8;
    case '9' : return 9;
    default  : return -1;
  }
}



/*
2 The Implementation of the Class DateTime

~The Standard Constructor~

This constructor makes nothing and should never be called.
It is used in a special way in a cast function.

*/
DateTime::DateTime():IndexableAttribute(){}



/*
~Constructor~

This constructor creates a DateTime at the NULL[_]DAY and
duration zero, respectively.

*/
DateTime::DateTime(const TimeType type1):
 IndexableAttribute(true), type(type1), value(0){ }

/*
~A Constructor ~

The Value of MilliSeconds has to be greater than or equals to zero.

*/
DateTime::DateTime(const int32_t Day,
                   const int32_t MilliSeconds,
                   const TimeType Type):
  IndexableAttribute(true),
  type(Type), value ( (((int64_t)Day)*MILLISECONDS) + MilliSeconds) { }

DateTime::DateTime(const int64_t v):
  IndexableAttribute(true),
  type(instanttype), value(v) {}

/*
~Constructor~

This conmstructor creates a DateTime with the same value like the
argument.

*/
DateTime::DateTime(const DateTime& DT):
   IndexableAttribute(DT), type(DT.type), value(DT.value) { }


DateTime::DateTime(const double d):
  IndexableAttribute(true), type(instanttype), value(0)
 {
   ReadFrom(d);
 }

DateTime::DateTime(const TimeType t, const uint64_t v):
  IndexableAttribute(true), type(t), value(v){}




/*
~Assignment operator~

*/
DateTime& DateTime::operator=(const DateTime& DT){
    Equalize(&DT);
    return (*this);
}


/*
~Destructor~

*/
DateTime::~DateTime(){}


/*
~Set~

This function sets an instant to the given values.

*/
void DateTime::Set(const int32_t year,const int32_t month, const int32_t day,
            const int32_t hour, const int32_t minute, const int32_t second,
            const int32_t millisecond){

   assert(type == instanttype);

   int64_t ms = ((hour*60+minute)*60+second)*1000+millisecond;
   int64_t d = ToJulian(year,month,day);
   value = d*MILLISECONDS + ms;
   SetDefined(true);
}




/*
~Now~

Gets the value of this DateTime from the System.
Cannot be applied to a duration.

*/
void DateTime::Now(){
  assert(type!=(durationtype));
  timeb tb;
  time_t now;
  int ms;
  ftime(&tb);
  now = tb.time;
  ms = tb.millitm;
  tm* time = localtime(&now);
  int64_t day = ToJulian(time->tm_year+1900,time->tm_mon+1,time->tm_mday);
  int64_t milliseconds = ((((time->tm_hour)*60)+time->tm_min)*
                     60+time->tm_sec)*1000+ms;
  value = day*MILLISECONDS + milliseconds;
  SetDefined(true);
}

/*
~Today~

This function reads this DateTime value from the Systemtime. The time part is
ignored here. This means, that hour ... millisecond are assumed to be zero.
Cannot applied to a duration.

*/
void DateTime::Today(){
   assert(type != (durationtype));
   time_t today;
   time(&today);
   tm* lt = localtime(&today);
   int64_t day = ToJulian(lt->tm_year+1900,lt->tm_mon+1,lt->tm_mday);
   value= day*MILLISECONDS;
   SetDefined(true);
}

/*
~ToMinimum~

Sets this instant to the mimimum possible value.

*/
void DateTime::ToMinimum(){
   SetDefined(true);
   value = min_VALUE;
}


/*
~ToMaximum~

Sets this instant to the maximum possible value.

*/
void DateTime::ToMaximum(){
   SetDefined(true);
   value = max_VALUE;
}


/*
~IsMinimum~

Checks if this value is the most minimum representable one.

*/
bool DateTime::IsMinimum()const {
  if(!IsDefined()){
     return false;
  }
  return value == min_VALUE;
}

/*
~IsMaximum~

Checks if this value is the most maximum representable one.

*/
bool DateTime::IsMaximum()const{
  if(!IsDefined()){
     return false;
  }
  return value==max_VALUE;
}


/*
~GetDay~

This functions yields the day-part of a duration.
The function can't applied to an instant.

*/
int64_t DateTime::GetDay()const{
   int64_t d =  (value / (int64_t) MILLISECONDS);
   if((value<0) && (value%MILLISECONDS!=0)){
       d--;
   }
   return d;
}

/*
~GetAllMilliSeconds~

This function returns the milliseconds of the day
of this Time instance.

*/
int32_t DateTime::GetAllMilliSeconds()const{
   int32_t ms = (int32_t) (value % MILLISECONDS);
   if(value<0 && ms!=0){
      ms += MILLISECONDS;
   }
   return ms;
}

/*
~Gregorian Calender~

The following functions return single values of this DateTime instance.
This functions cannot applied to durations.

*/

int32_t DateTime::GetGregDay()const{
    assert(type != (durationtype));
    int32_t y,m,d;
    ToGregorian(y,m,d);
    return d;
}

int DateTime::GetMonth()const{
   assert(type !=(durationtype));
   int32_t y,m,d;
   ToGregorian(y,m,d);
   return m;
}

int32_t DateTime::GetYear()const{
   assert(type != (durationtype));
   int32_t y,m,d;
   ToGregorian(y,m,d);
   return y;
}

int32_t DateTime::GetHour()const{
   assert(type != (durationtype));
   int32_t milliseconds = GetAllMilliSeconds();
   return (int32_t)(milliseconds / 3600000);
}

int32_t DateTime::GetMinute()const{
   assert(type != (durationtype));
   int32_t milliseconds = GetAllMilliSeconds();
   return (int32_t) ( (milliseconds / 60000) % 60);
}

int32_t DateTime::GetSecond()const{
  assert(type != (durationtype));
  int32_t milliseconds = GetAllMilliSeconds();
  return (int32_t) ( (milliseconds / 1000) % 60);
}

int32_t DateTime::GetMillisecond()const{
  assert(type != (durationtype));
  int32_t milliseconds = GetAllMilliSeconds();
  return (int32_t) (milliseconds % 1000);
}

int32_t DateTime::GetWeekday()const{
    int32_t day = GetDay();
    if(day>=0){
        return day % 7;
    } else {
        return (day % 7 )  + 7;
    }
}

/*
~ToJulian~

This function computes the Julian day number of the given
gregorian date + the reference time.
Positive year signifies A.D., negative year B.C.
Remember that the year after 1 B.C. was 1 A.D.

Julian day 0 is a Monday.

This algorithm is from Press et al., Numerical Recipes
in C, 2nd ed., Cambridge University Press 1992

*/
int32_t DateTime::ToJulian(const int32_t year,
                           const int32_t month,
                           const int32_t day) const{
  int32_t jy = year;
  if (year < 0)
     jy++;
  int32_t jm = month;
  if (month > 2)
     jm++;
  else{
     jy--;
     jm += 13;
  }

  int32_t jul = (int32_t)(floor(365.25 * jy) + floor(30.6001*jm)
                  + day + 1720995.0);
  int32_t IGREG = 15 + 31*(10+12*1582);
  // Gregorian Calendar adopted Oct. 15, 1582
  if (day + 31 * (month + 12 * year) >= IGREG){
     // change over to Gregorian calendar
     int32_t ja = (int32_t)(0.01 * jy);
     jul += 2 - ja + (int32_t)(0.25 * ja);
  }
  return jul-NULL_DAY;
}

/*
~ToGregorian~

This function converts a Julian day to a date in the gregorian calender.

This algorithm is from Press et al., Numerical Recipes
in C, 2nd ed., Cambridge University Press 1992

*/
void DateTime::ToGregorian(const int32_t Julian, int32_t &year,
                           int32_t &month, int32_t &day) const{
  int32_t j=(int32_t)(Julian+NULL_DAY);
   int32_t ja = j;
   int32_t JGREG = 2299161;
   /* the Julian date of the adoption of the Gregorian
      calendar
   */
    if (j >= JGREG){
    /* cross-over to Gregorian Calendar produces this
       correction
    */
       int32_t jalpha = (int32_t)(((float)(j - 1867216) - 0.25)/36524.25);
       ja += 1 + jalpha - (int32_t)(0.25 * jalpha);
    }
    int32_t jb = ja + 1524;
    int32_t jc = (int32_t)(6680.0 + ((float)(jb-2439870) - 122.1)/365.25);
    int32_t jd = (int32_t)(365 * jc + (0.25 * jc));
    int32_t je = (int32_t)((jb - jd)/30.6001);
    day = jb - jd - (int32_t)(30.6001 * je);
    month = je - 1;
    if (month > 12) month -= 12;
    year = jc - 4715;
    if (month > 2) --year;
    if (year <= 0) --year;


// the followingcode is converted from the fre pascal compiler unixutils.pp
   /*    long long D0   =   1461;
     long long D1   = 146097;
     long long D2   =1721119;
     long long JulianDN = (long long)Julian+(long long)NULL_DAY;
     long long  Temp =((JulianDN-D2) << 2ll)-1ll;
     JulianDN = Temp / D1;
     long long  XYear=(Temp % D1) |  3ll;
     long long  YYear=(XYear / D0);
     Temp=((((XYear % D0)+4ll) >> 2ll)*5ll)-3ll;
     long long  Day=((Temp % 153ll)+5ll) / 5ll;
     long long  TempMonth=Temp / 153ll;
     if(TempMonth>=10ll){
           YYear++;
           TempMonth-=12ll;
     }
     TempMonth+=3ll;
     month = TempMonth;
     year=YYear+(JulianDN*100ll);
     day = Day;
   */
}

void DateTime::ToGregorian(int32_t &year,
                           int32_t &month, int32_t &day) const{
   ToGregorian(GetDay(),year,month,day);
}

/*

~ToDouble~

This function converts this Time instance to the corresponding
double value;

*/
   double DateTime::ToDouble() const{
   return  ((double)value) / MILLISECONDS;
}


/*
~ToString~

This function returns the string representation of this DateTime instance.

*/
string DateTime::ToString(const bool sql92conform /*=false*/ ) const{
  ostringstream tmp;
  if(!IsDefined()){
    return Symbol::UNDEFINED();
  }
  if(type == (durationtype)){ //a duration
    tmp << GetDay() << ";";
    tmp << GetAllMilliSeconds();
  }else if(type ==(instanttype)){ // an instant
     if(IsMinimum()){
       return begin_of_time;
     }
     if(IsMaximum()){
       return end_of_time;
     }
     // SOME DATES CAN'T BE CONVERTED CORRECTLY INTO THE GREGORIAN
     // calendar
    if(value < MIN_REPRESENTABLE || value >MAX_REPRESENTABLE){
        tmp << ToDouble();
        return tmp.str();
    }

    int32_t day,month;
    int32_t year;
    ToGregorian(year,month,day);
    if(!(day>0 && month>0 && month<13 && day<32)){
       cmsg.error() << "error in ToString function of instant detected \n"
                    << "day ("<<day<<") or month ("<<month<<") outside"
                    << " of the valid range\n";
       cmsg.send();
    }
    // ensure to write at least 4 digits for a year
    if(year < 1000 && year>0)
      tmp << "0";
    if(year < 100 && year >0)
      tmp << "0";
    if(year < 10 && year > 0)
      tmp << "0";
    tmp << year << "-";
    if(month<10)
       tmp << "0";
    tmp << month << "-";
    if(day<10)
       tmp << "0";
    tmp << day;
    int32_t milliseconds = GetAllMilliSeconds();
    if(milliseconds==0) // without time
       return tmp.str();

    int32_t v = GetAllMilliSeconds();
    int32_t ms = v % 1000;
    v  = v / 1000;
    int32_t sec = v % 60;
    v = v / 60;
    int32_t min = v % 60;
    int32_t hour = v / 60;


    tmp << (sql92conform?" ":"-");
    if(hour<10)
        tmp << "0";
    tmp << hour << ":";
    if(min<10)
      tmp << "0";
    tmp << min;

    if((sec==0) && (ms == 0))
       return tmp.str();

    tmp << ":";
    if(sec<10)
       tmp << "0";
    tmp << sec;
    if(ms==0)
       return tmp.str();

    tmp << ".";
    if(ms <100)
       tmp << "0";
    if(ms <10)
       tmp << "0";
    tmp << ms;
  } else{
    tmp << "unknown type, def=" << IsDefined()
        << " type=" << GetType() << " day = " << GetDay() << " ms = "
        << GetAllMilliSeconds();
  }
  return tmp.str();
}

ostream& DateTime::Print(ostream &os) const
{
  os << ToString();
  return os;
}


/*
~ReadFrom~

This function reads the Time from a given string. If the string
don't represent a valid Time value, this instance remains unchanged
and false is returned. In the other case, this instance will hold the
the time represented by the string and the result is true.
The format of the string must be:

  *  YEAR-MONTH-DAY-HOUR:MIN[:SECOND[.MILLISECOND]]

Spaces are not allowed in this string. The squared brackets
indicates optional parts. This function is not defined for durations.

*/
bool DateTime::ReadFrom(const string Time1){
   
  string Time = Time1;
  stringutils::trim(Time);
  SetDefined(true);
  if(type == (instanttype)){
    // read instant type from string
    if(listutils::isSymbolUndefined(Time)){
        SetDefined(false);
        return true;
    }
    if(Time==begin_of_time){
          ToMinimum();
          return true;
    }
    if(Time==end_of_time){
        ToMaximum();
        return true;
    }
    int32_t year = 0;
    int32_t digit;
    int32_t len = Time.length();
    if(len==0) return false;
    int32_t pos = 0;
    // read the year
    int32_t signum = 1;
    if(Time[0]=='-'){
        signum=-1;
        pos++;
    }

    if(pos==len) return false;
    while(Time[pos]!='-'){
        digit = datetime::GetValue(Time[pos]);
        if(digit<0) return false;
        pos++;
        if(pos==len) return false;
        year = year*10+digit;
      }
    year = year*signum;
    pos++; // read over  '-'
    if(pos==len) return false;
    // read the month
    int32_t month = 0;
    while(Time[pos]!='-'){
        digit = datetime::GetValue(Time[pos]);
        if(digit<0) return false;
        pos++;
        if(pos==len) return false;
        month = month*10+digit;
    }
    pos++; // read over '-'
    if(pos==len) return false;
    // read the day
    int32_t day = 0;
    while( (Time[pos]!='-') && (Time[pos]!=' ') ){
        digit = datetime::GetValue(Time[pos]);
        if(digit<0) return false;
        pos++;
        day = day*10+digit;
        if(pos==len){ // we allow pure date string without any hour
          if(!IsValid(year,month,day))
              return false;
          value = ((int64_t)ToJulian(year,month,day))*MILLISECONDS;
          SetDefined(true);
          return true;
        }
    }
    pos++; // read over '-' or ' '
    if(pos==len) return false;
    if(!IsValid(year,month,day))
        return false;
    // read the hour
    int32_t hour = 0;
    while(Time[pos]!=':'){
        digit = datetime::GetValue(Time[pos]);
        if(digit<0) return false;
        pos++;
        if(pos==len) return false;
        hour = hour*10+digit;
    }
    pos++; // read over ':'
    if(pos==len) return false;
    // read the minute
    int32_t minute = 0;
    bool done = false;
    bool next = false;
    while(!done && !next){
        digit = datetime::GetValue(Time[pos]);
        if(digit<0) return false;
        pos++;
        minute = minute*10+digit;
        done = pos==len;
        if(!done)
            next = Time[pos]==':';
    }
    // initialize seconds and milliseconds with zero
    int32_t seconds = 0;
    int32_t mseconds = 0;
    if(!done){ // we have to read seconds
      pos++; // read over the ':'
      if(pos==len) return false;
      next = false;
      while(!done && !next){
          digit = datetime::GetValue(Time[pos]);
          if(digit<0) return false;
          pos++;
          seconds = seconds*10+digit;
          done = pos==len;
          if(!done)
            next = Time[pos]=='.';
      }
      if(!done ){ // milliseconds are available
          pos++; // read over the '.'
          if(pos==len) return false;
          next = false;
          while(!done){
              digit = datetime::GetValue(Time[pos]);
              if(digit<0) return false;
              pos++;
              mseconds = mseconds*10+digit;
              done = pos==len;
          }
      }
    }
    // At this place we have all needed information to create a date
    day = ToJulian(year,month,day);
    int64_t milliseconds = ((hour*60+minute)*60+seconds)*1000+mseconds;

    value = ((int64_t)day)*MILLISECONDS + milliseconds;

    SetDefined(true);
    return true;
  } else { // read durationtype from string
    assert(type ==(durationtype));
    if(listutils::isSymbolUndefined(Time)){
        SetDefined(false);
        return true;
    }
    int32_t day = 0;
    int32_t digit;
    int32_t len = Time.length();
    if(len==0) return false;
    int32_t pos = 0;
    // read the day
    int32_t signum = 1;
    if(Time[0]=='-'){
        signum=-1;
        pos++;
    }
    if(pos==len) return false;
    while(Time[pos]!=';'){
        digit = datetime::GetValue(Time[pos]);
        if(digit<0) return false;
        pos++;
        if(pos==len) return false;
        day = day*10+digit;
      }
    day = day*signum;
    pos++; // read over  ';'
    bool done = (pos==len);
    if(done) return false;

    // read the millisecond
    int32_t mseconds = 0;
    signum = 1;
    if(Time[pos]=='-'){
      signum=-1;
      pos++;
      done = (pos==len);
    }
    while(!done){
      digit = datetime::GetValue(Time[pos]);
      if(digit<0) return false;
      pos++;
      mseconds = mseconds*10+digit;
      done = pos==len;
    }
    mseconds = mseconds * signum;

    // store data to attributesa
    value = ((int64_t)day) + mseconds;
    SetDefined(true);
    return true;
  }
}


/*
~ReadFrom~

This functions reads the value of this instance from the given double.

*/
bool DateTime::ReadFrom(const double Time){
   SetDefined(true);
   value = (int64_t) (Time*MILLISECONDS + 0.5);
   return true;
}

/*
~ReadFrom~

This functions reads the value of this instance from the given int64\_t.

*/
bool DateTime::ReadFrom(const int64_t Time){
   SetDefined(true);
   value = Time;
   return true;
}

/*
~IsValid~

This functions checks if the given arguments represent a valid gregorian
date. E.g. this function will return false if month is greater than twelve,
or the day is not included in the given month/year.

*/
bool DateTime::IsValid(const int32_t year,
                       const int32_t month,
                       const int32_t day)const {
   int32_t jday = ToJulian(year,month,day);
   int m=0,d=0;
   int32_t y = 0;
   ToGregorian(jday,y,m,d);
   return year==y && month==m && day==d;
}

/*
~ReadFrom~

This function reads the time value from the given nested list.
If the format is not correct, this function will return false and this
instance remains unchanged. Otherwise this instance will take the value
represented by this list and the result will be true.

*/
bool DateTime::ReadFrom(const ListExpr LE, const bool typeincluded){
  ListExpr ValueList;
  if(typeincluded){
     if(nl->ListLength(LE)!=2){
        return false;
     }
     ListExpr TypeList = nl->First(LE);
     if(!listutils::isSymbol(TypeList,"datetime")){
       if( (type == (instanttype)) &&
           !listutils::isSymbol(TypeList,DateTime::BasicType())){
            return false;
       }
       if( (type == (durationtype)) &&
           !listutils::isSymbol(TypeList,Duration::BasicType())){
          return false;
       }
     }
     ValueList = nl->Second(LE);
  }else{
     ValueList=LE;
  }

  // Special Representation in this Algebra
  if(listutils::isSymbolUndefined(ValueList)){
    SetDefined(false);
    return true;
  }
  if(listutils::isSymbol(ValueList,"currenttime") ||
    listutils::isSymbol(ValueList,"CURRENTTIME")){
    if(type == (instanttype)){
      Now();
      return true;
    } else
      return false;
  }
  if(listutils::isSymbol(ValueList,"today") ||
     listutils::isSymbol(ValueList,"TODAY")){
    if(type == (instanttype)){
      Today();
      return  true;
    } else
      return false;
  }

  // string representation
  if(nl->AtomType(ValueList)==StringType){
     if(type ==(instanttype))
        return ReadFrom(nl->StringValue(ValueList));
     else
        return false;
  }
  // real representation
  if(nl->AtomType(ValueList)==RealType ){
        bool res = ReadFrom(nl->RealValue(ValueList));
        return res;
  }
  // accept also integer values
  if(nl->AtomType(ValueList)==IntType ){
     if(type ==(instanttype)){
        bool res = 
         ReadFrom(static_cast<double>(nl->IntValue(ValueList)));
        return res;
     }
     else
        return false;
  }

  // (day month year [hour minute [second [millisecond]]])
  if( (nl->ListLength(ValueList)>=3) && nl->ListLength(ValueList)<=7){
     if(type == (durationtype))
        return  false;
     int32_t len = nl->ListLength(ValueList);
     if(len==4) return false; // only hours is not allowed
     ListExpr tmp = ValueList;
     while(!nl->IsEmpty(tmp)){
        if(nl->AtomType(nl->First(tmp))!=IntType)
           return false;
        tmp = nl->Rest(tmp);
     }
     int32_t d,m,y,h,min,sec,ms;

     d = nl->IntValue(nl->First(ValueList));
     m = nl->IntValue(nl->Second(ValueList));
     y = nl->IntValue(nl->Third(ValueList));
     h = len>3? nl->IntValue(nl->Fourth(ValueList)):0;
     min = len>4? nl->IntValue(nl->Fifth(ValueList)):0;
     sec = len>5? nl->IntValue(nl->Sixth(ValueList)):0;
     ms = 0;
     if(len==7){
          ValueList = nl->Rest(ValueList);
          ms = nl->IntValue(nl->Sixth(ValueList));
     }
     // check the ranges
     if(!IsValid(y,m,d)) return false;
     if(h<0 || h>23) return false;
     if(min<0 || min > 59) return false;
     if(sec<0 || sec > 59) return false;
     if(ms<0 || ms > 999) return false;
     // set the values
     int64_t day = ToJulian(y,m,d);
     int64_t milliseconds = (((h*60)+min)*60 + sec)*1000 +ms;
     this->value = day*MILLISECONDS + milliseconds;
     SetDefined(true);
     return true;
  }

  // (julianday milliseconds)  // for durations
  if(nl->ListLength(ValueList)!=2){
    return false;
  }
  if(type == (instanttype))
     return false;
  ListExpr DayList = nl->First(ValueList);
  ListExpr MSecList = nl->Second(ValueList);
  if(nl->AtomType(DayList)!=IntType || nl->AtomType(MSecList)!=IntType){
     return false;
  }
  int64_t day = nl->IntValue(DayList);
  int64_t milliseconds = nl->IntValue(MSecList);
  this->value = day*MILLISECONDS + milliseconds;
  return  true;
}

/*
~CompareTo~

This function compares this DateTime instance with another one.
The result will be:

  * -1 if this instance is before P2

  * 0 if this instance is equals to P2

  * 1 if this instance is after P2

The types of the arguments has to be equals.

*/
int DateTime::CompareTo(const DateTime* P2)const{
   if(GetType()!=P2->GetType()){
       cerr << "try to compare " << this->ToString() << " with "
            << P2->ToString() << endl;
       assert(GetType()==P2->GetType());
   }
   if(!IsDefined() && !P2->IsDefined())
      return 0;
   if(!IsDefined() && P2->IsDefined())
      return -1;
   if(IsDefined() && !P2->IsDefined())
      return 1;
   // at this point this and P2 are defined
   if(value < P2->value) return -1;
   if(value > P2->value) return 1;
   return 0;
}

/*
~Operators for Comparisons~

*/
bool DateTime::operator==(const DateTime& T2)const{
  return CompareTo(&T2)==0;
}

bool DateTime::operator!=(const DateTime& T2)const{
  return CompareTo(&T2)!=0;
}

bool DateTime::operator<(const DateTime& T2)const{
  return CompareTo(&T2)<0;
}

bool DateTime::operator>(const DateTime& T2)const{
  return CompareTo(&T2)>0;
}

bool DateTime::operator<=(const DateTime& T2)const{
  return CompareTo(&T2)<=0;
}

bool DateTime::operator>=(const DateTime& T2)const{
  return CompareTo(&T2)>=0;
}

/*
~Clone~

This funtion returns a copy of this instance.

*/
DateTime* DateTime::Clone() const {
   return new DateTime(*this);
}


/*
~Split~

The function ~Split~ splits a duration into two ones.

*/
bool DateTime::Split(const double delta, DateTime& Rest){
  assert(type == (durationtype));
  assert((delta>=0) && (delta<=1));
  Rest.Equalize(this);
  Mul(delta);
  Rest.Minus(this);
  return true;
}



/*
~Compare~

This function compare this DateTime with the given Attribute.

*/
int DateTime::Compare(const Attribute* arg) const{
  return CompareTo( (const DateTime*) arg);
}

/*
~Adjacent~

We treat time as continuous. For this reason, in our discrete
representation we never find adjacent instants and the result
will be __false__.

*/
bool DateTime::Adjacent(const Attribute* arg) const{
  return false;
  /*
  // adjacent in discrete case:
  const DateTime* T2 = (const DateTime*) arg;
  if(day==T2->day && abs(milliseconds-T2->milliseconds)==1)
    return true;
  if((day-1==T2->day) && (milliseconds==MILLISECONDS-1)
      && (T2->milliseconds==0))
     return true;
  if( (day==T2->day-1) && (milliseconds==0)
       && (T2->milliseconds==MILLISECONDS-1))
     return true;
  return false;
 */
}


/*
~Sizeof~

The function ~Sizeof~ returns the ~sizeof~ of and instance
of the ~DateTime~ class.

*/
size_t DateTime::Sizeof() const{
   return sizeof(*this);
}

/*
~HashValue~

This function return the HashValue for this DateTime instance.

*/
size_t DateTime::HashValue() const{
  if(!IsDefined()) return 0;
  return (size_t) value;
}

/*
~CopyFrom~

This Time instance take its value from arg if this function is
called.

*/
void DateTime::CopyFrom(const Attribute* arg){
   Equalize(((const DateTime*) arg));
}

/*
~add~

Adds P2 to this instance. P2 remains unchanged.
The ~Add~ function requires least one argument to be
a duration type.

*/
void DateTime::Add(const DateTime* P2){

   // do not allow to add two instants
   assert( (type == (durationtype)) || (P2->type == durationtype));
   if(P2->type==instanttype){
     this->type = instanttype;
   }
   if(!IsDefined()) return;
   if(!P2->IsDefined()){
      SetDefined(false);
      return;
   }
   value += P2->value;
}


/*
~Operator +~

This operator has the same functionality like the ~Add~ function
returning the result in a new instance.

*/
DateTime DateTime::operator+(const DateTime& T2)const{
   DateTime Result(*this);
   Result.Add(&T2);
   return Result;
}


/*
~Operator +=~

This operator has the same functionality like the ~Add~ function.

*/
DateTime DateTime::operator+=(const DateTime& T2){
   Add(&T2);
   return *this;
}

/*
~Operator -=~

This operator has the same functionality like the ~Minus~ function.

*/
DateTime DateTime::operator-=(const DateTime& T2){
   Minus(&T2);
   return *this;
}
/*
~minus~

Subtracts P2 from this instance.
If this instance is of type duration the, type of the argument
has also to be a duration. Its possible that this function changes
the type of this instance.

*/
void DateTime::Minus(const DateTime* P2) {
   assert(type ==(instanttype) || P2->type==(durationtype));

   if(P2->type==instanttype){
       type=durationtype;
   }

   if(!IsDefined()){
    return;
   }
   if(!P2->IsDefined()){
      SetDefined(false);
      return;
   }
   value -= P2->value;


}

/*
~Operator -~

This operator has the same functionality like the ~Minus~ function
returning the result in a new instance.

*/
DateTime DateTime::operator-(const DateTime& T2)const{
   DateTime Result(*this);
   Result.Minus(&T2);
   return Result;
}

/*
~Operator /~

This Operator divides a DateTime by another dateTime

*/
double DateTime::operator/(const DateTime& T2)const{
  double myms = (double)(value);
  double t2ms = (double) T2.value;
  return myms / t2ms;
}


/*
~Operator /~

This Operator divides a DateTime by an integer

*/
DateTime DateTime::operator/(const int32_t divisor)const{
  assert(type==(durationtype));
  assert(divisor != 0);
  int64_t v = value / divisor;
  DateTime res(durationtype,v);
  return res;
}

/*
~mul~

Computes a multiple of a duration.

*/
void DateTime::Mul(const int32_t factor){
   assert(type==(durationtype));
   value *= factor;
}

/*
~mul~

Computes a multiple of a duration. This function is not
numerical robust. Use this function only when you know what
you do.

*/
void DateTime::Mul(const double factor){
   value = (int64_t)(value*factor + 0.5);
}

/*
~Div~

This operator computes how often ~dividend~ is contained whithin this
DateTime. This DateTime, the dividend, and ~remainder~ must be of
type ~duration~.

*/
 int64_t DateTime::Div(const DateTime& dividend, DateTime& remainder){
   int64_t res = value / dividend.value;
   int64_t rem = value % dividend.value;
   remainder.SetDefined(true);
   remainder.value = rem;
   return res;
 }




/*
~Operator mul~

This operator has the same functionality like the ~Mul~ function
returning the result in a new instance.

*/
DateTime DateTime::operator*(const int32_t factor)const{
   DateTime Result(*this);
   Result.Mul(factor);
   return Result;
}

DateTime DateTime::operator*(const double factor)const{
   DateTime Result(*this);
   Result.Mul(factor);
   return Result;
}


/*
~Abs~

~Abs~ compute the absolute value of a duration.

*/
  void DateTime::Abs(){
    if(value<0){
      value = -value;
    }
  }


/*
~ToListExpr~

This functions returns this time value in nested list format.
The argument controls the format of the output.
If absolute is false, the value-representation will be a
list containing two int atoms. The first integer is the
day of this time and the second one the part of this day
in milliseconds. If the parameter is true, the value will be a
string in format year-month-day-hour:minute:second.millisecond

*/
ListExpr DateTime::ToListExpr(const bool typeincluded)const {
  assert(IsDefined() );
  ListExpr value;
  if(type==(instanttype)){
      if( (this->value<MIN_REPRESENTABLE || this->value>MAX_REPRESENTABLE )
         && !IsMinimum() && !IsMaximum()){

          value = nl->RealAtom(ToDouble());
      }else{
          value = nl->StringAtom(this->ToString());
      }
  } else {  // a duration
    int64_t day = GetDay();
    int32_t day2=0;
    int32_t ms = GetAllMilliSeconds();
    if(day != ((int32_t) day)){ // out of the representable range
       ms = 0;
       if(day<0){
         day2  = numeric_limits<int32_t>::min();
       } else {
         day2  = numeric_limits<int32_t>::max();
       }
    } else {
       day2 = (int32_t) day;
    }

    value = nl->TwoElemList( nl->IntAtom(day2),
                             nl->IntAtom(ms)
                             );
  }
  if(typeincluded)
     if(type==(instanttype))
        return nl->TwoElemList(nl->SymbolAtom(DateTime::BasicType()),value);
     else if(type==(durationtype))
        return nl->TwoElemList(nl->SymbolAtom(Duration::BasicType()),value);
     else
        return nl->TwoElemList(nl->SymbolAtom("datetime"),value);
  else
    return value;
}

/*
~Equalize~

This function changes the value of this Time instance to be equal to
P2.

*/
void DateTime::Equalize(const DateTime* P2){
   this->type = P2->type;
   this->SetDefined(P2->IsDefined());
   this->value = P2->value;
}

/*
~SetToZero~

A call of this function will set the value of this datetime to be zero,
this means to have length zero or be the NULLDATE respectively.

*/
    void DateTime::SetToZero(){
       value =0;
       SetDefined(true);
    }



/*
~IsZero~

~IsZero~ returns true iff this

*/
bool DateTime::IsZero()const {
  return  IsDefined() && value==0;
}

/*
~LessThanZero~

This function returns true if this instnace is before the Null-Day

*/
bool DateTime::LessThanZero()const{
   return IsDefined() && value < 0;
}

SmiSize DateTime::SizeOfChars() const
{
  return ToString().length()+1;
}

void DateTime::WriteTo( char *dest ) const
{
  strcpy( dest, ToString().c_str() );
}

void DateTime::ReadFrom( const char *src )
{
  ReadFrom( string(src) );
}

/*
~Stream Operator~

*/

ostream& operator<<(ostream& o, const DateTime& DT){
   o << DT.ToString();
   return o;
}



/*
2 Algebra Functions

2.1 In and Out functions

*/

ListExpr OutDateTime( ListExpr typeInfo, Word value ){
   DateTime* T = (DateTime*) value.addr;
   if( T->IsDefined() )
     return T->ToListExpr(false);
   else
     return nl->SymbolAtom(Symbol::UNDEFINED());
}


Word InInstant( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct ){

  DateTime* T = new DateTime(instanttype);


  ListExpr value = instance;
  if( !nl->IsAtom(instance) &&
      (nl->ListLength(instance)==2) ){
    if(nl->IsEqual(nl->First(instance),DateTime::BasicType()))
      value = nl->Second(instance);
  }
  if(T->ReadFrom(value,false)){
    correct=true;
    return SetWord(T);
  }
  correct = false;
  delete(T);
  return SetWord(Address(0));
}

Word InInstantValueOnly( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct ){

  DateTime* T = new DateTime(instanttype);
  if(T->ReadFrom(instance,false)){
    correct=true;
    return SetWord(T);
  }
  correct = false;
  delete(T);
  return SetWord(Address(0));
}



Word InDuration( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct ){

  DateTime* T = new DateTime(durationtype);
  if(T->ReadFrom(instance,false)){
    correct=true;
    return SetWord(T);
  }
  correct = false;
  delete(T);
  return SetWord(Address(0));
}


/*
2.2 Property Functions

*/
ListExpr InstantProperty(){
  return (nl->TwoElemList(
            nl->FiveElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"),
                nl->StringAtom("Remarks")),
            nl->FiveElemList(
                nl->StringAtom("-> Data"),
                nl->StringAtom(DateTime::BasicType()),
                nl->StringAtom(CcString::BasicType()),
                nl->StringAtom("2004-4-12-8:03:32.645"),
                nl->StringAtom("This type represents a point of time")
         )));
}

ListExpr DurationProperty(){
  return (nl->TwoElemList(
            nl->FiveElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"),
                nl->StringAtom("Remarks")),
            nl->FiveElemList(
                nl->StringAtom("-> Data"),
                nl->StringAtom(DateTime::BasicType()),
                nl->StringAtom("(int int)"),
                nl->StringAtom("(12 273673)"),
                nl->StringAtom("the arguments are days and milliseconds")
         )));
}


/*
2.3 ~Create~ function

*/
Word CreateInstant(const ListExpr typeInfo){
  return SetWord(new DateTime(instanttype));
}

Word CreateDuration(const ListExpr typeInfo){
  return SetWord(new DateTime(durationtype));
}


/*
2.4 ~Delete~ function

*/
void DeleteDateTime(const ListExpr typeInfo, Word &w){
  DateTime* T = (DateTime*) w.addr;
  delete T;
  w.addr=0;
}


/*
2.5 ~Open~ function

*/
bool OpenDateTime( SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value ){
  // This Open function is implemented in the Attribute class
  // and uses the same method of the Tuple manager to open objects
 DateTime *dt =
      (DateTime*)Attribute::Open( valueRecord, offset, typeInfo );
 value = SetWord( dt );
 return true;
}
/*
2.6 ~Save~ function

*/
bool SaveDateTime( SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value ){
 DateTime *dt = (DateTime *)value.addr;
 Attribute::Save( valueRecord, offset, typeInfo, dt );
 return true;
}

/*
2.7 ~Close~ function

*/
void CloseDateTime( const ListExpr typeInfo, Word& w ){
  delete (DateTime *)w.addr;
  w.addr = 0;
}

/*
2.8 ~Clone~ function

*/
Word CloneDateTime( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((DateTime *)w.addr)->Clone() );
}

/*
2.9 ~SizeOf~-Function

*/
int SizeOfDateTime(){
  return sizeof(DateTime);
}

/*
2.10 ~Cast~-Function

*/
void* CastDateTime( void* addr )
{
  return new (addr) DateTime;
}

/*
2.11 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
the type constructor don't have arguments, this is trivial.

*/

bool CheckInstant(ListExpr type, ListExpr& errorInfo){
  return (nl->IsEqual(type,DateTime::BasicType()));
}

bool CheckDuration(ListExpr type, ListExpr& errorInfo){
  return (nl->IsEqual(type,Duration::BasicType()));
}

/*
3 Type Constructor

*/
TypeConstructor instant(
        DateTime::BasicType(),                     //name
        InstantProperty,              //property function describing signature
        OutDateTime, InInstantValueOnly,       //Out and In functions
        0,                            //SaveToList and
        0,                            // RestoreFromList functions
        CreateInstant, DeleteDateTime,//object creation and deletion
        OpenDateTime,    SaveDateTime,//object open and save
        CloseDateTime,  CloneDateTime,//object close and clone
        CastDateTime,                 //cast function
        SizeOfDateTime,               //sizeof function
        CheckInstant );               //kind checking function

TypeConstructor duration(
        Duration::BasicType(),                //name
        DurationProperty,          //property function describing signature
        OutDateTime, InDuration,   //Out and In functions
        0,                         //SaveToList and
        0,                         // RestoreFromList functions
        CreateDuration, DeleteDateTime,   //object creation and deletion
        OpenDateTime,    SaveDateTime,    //object open and save
        CloseDateTime,  CloneDateTime,    //object close and clone
        CastDateTime,                     //cast function
        SizeOfDateTime,                   //sizeof function
        CheckDuration );                  //kind checking function


/*
4 Operators

4.1 Type Mappings

*/
ListExpr VoidInstant(ListExpr args){
  if(nl->IsEmpty(args))
     return nl->SymbolAtom(DateTime::BasicType());
  ErrorReporter::ReportError("no argument allowed\n");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr IntBool(ListExpr args){
  if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("one argument expected\n");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  if(nl->IsEqual(nl->First(args),CcInt::BasicType()))
     return nl->SymbolAtom(CcBool::BasicType());
  ErrorReporter::ReportError("argument must be of type int\n");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr InstantInt(ListExpr args){
  if(nl->ListLength(args)==1){
     if(nl->IsEqual(nl->First(args),DateTime::BasicType())){
         return nl->SymbolAtom(CcInt::BasicType());
     } else {
         ErrorReporter::ReportError("instant parameter expected \n");
     }
  }
  ErrorReporter::ReportError("exactly one argument required");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr PlusCheck(ListExpr args){
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("plus expects two arguments\n");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  if(nl->IsEqual(nl->First(args),DateTime::BasicType()) &&
     nl->IsEqual(nl->Second(args),Duration::BasicType()))
     return nl->SymbolAtom(DateTime::BasicType());
  if(nl->IsEqual(nl->First(args),Duration::BasicType()) &&
     nl->IsEqual(nl->Second(args),DateTime::BasicType()))
     return nl->SymbolAtom(DateTime::BasicType());
  if(nl->IsEqual(nl->First(args),Duration::BasicType()) &&
     nl->IsEqual(nl->Second(args),Duration::BasicType()))
     return nl->SymbolAtom(Duration::BasicType());
  ErrorReporter::ReportError("duration/instant or"
                             " duration/duration expected\n");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr MinusCheck(ListExpr args){
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("operator - requires two arguments\n");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  if(nl->IsEqual(nl->First(args),DateTime::BasicType()) &&
     nl->IsEqual(nl->Second(args),Duration::BasicType()))
     return nl->SymbolAtom(DateTime::BasicType());
  if(nl->IsEqual(nl->First(args),DateTime::BasicType()) &&
     nl->IsEqual(nl->Second(args),DateTime::BasicType()))
     return nl->SymbolAtom(Duration::BasicType());
  if(nl->IsEqual(nl->First(args),Duration::BasicType()) &&
     nl->IsEqual(nl->Second(args),Duration::BasicType()))
     return nl->SymbolAtom(Duration::BasicType());
  ErrorReporter::ReportError("duration/instant or"
                             " duration/duration expected\n");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}


ListExpr DurationIntDuration(ListExpr args){
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("Two arguments required\n");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  if(nl->IsEqual(nl->First(args),Duration::BasicType()) &&
     nl->IsEqual(nl->Second(args),CcInt::BasicType()))
     return nl->SymbolAtom(Duration::BasicType());
  ErrorReporter::ReportError("duration x int expected\n" );
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr DurationRealDuration(ListExpr args){
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("Two arguments required\n");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  if(nl->IsEqual(nl->First(args),Duration::BasicType()) &&
     nl->IsEqual(nl->Second(args),CcReal::BasicType()))
     return nl->SymbolAtom(Duration::BasicType());
  ErrorReporter::ReportError("duration x real expected\n" );
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr InstantString(ListExpr args){
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("one argument expected\n");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  if(nl->IsEqual(nl->First(args),DateTime::BasicType()))
    return nl->SymbolAtom(CcString::BasicType());
  ErrorReporter::ReportError("instant expected\n");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr DateTimeString(ListExpr args){
  if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("one argument expected\n");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  if(nl->IsEqual(nl->First(args),DateTime::BasicType()) ||
     nl->IsEqual(nl->First(args),Duration::BasicType()))
     return nl->SymbolAtom(CcString::BasicType());
  ErrorReporter::ReportError("instant or duration expected\n");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}


ListExpr CheckComparisons(ListExpr args){
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("two arguments required\n");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  if(nl->IsEqual(nl->First(args),DateTime::BasicType()) &&
     nl->IsEqual(nl->Second(args),DateTime::BasicType()))
       return nl->SymbolAtom(CcBool::BasicType());

  if(nl->IsEqual(nl->First(args),Duration::BasicType()) &&
     nl->IsEqual(nl->Second(args),Duration::BasicType()))
       return nl->SymbolAtom(CcBool::BasicType());

  ErrorReporter::ReportError("(instant x instant) or"
                             "(duratin x duration) required");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr TheInstantTM(ListExpr args){
   int l = nl->ListLength(args);
   if(l<1 || l>7){
      ErrorReporter::ReportError(" 1..7 arguements required\n");
      return nl->SymbolAtom(Symbol::TYPEERROR());
   }
   ListExpr rest = args;
   while(!nl->IsEmpty(rest)){
       if(!nl->IsEqual(nl->First(rest),CcInt::BasicType())){
           ErrorReporter::ReportError("All arguments must be of type int\n");
           return nl->SymbolAtom(Symbol::TYPEERROR());
       }
       rest = nl->Rest(rest);
   }
   return nl->SymbolAtom(DateTime::BasicType());
}

ListExpr DivTM(ListExpr args){
   if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("Two arguments required\n");
      return nl->SymbolAtom(Symbol::TYPEERROR());
   }
   if(!nl->IsEqual(nl->First(args),Duration::BasicType()) ||
      !nl->IsEqual(nl->Second(args),Duration::BasicType())){
      ErrorReporter::ReportError("two duration values expected\n");
      return nl->SymbolAtom(Symbol::TYPEERROR());
   }
   return nl->SymbolAtom(CcInt::BasicType());
}

ListExpr MinMaxInstantTM(ListExpr args){
   if(nl->IsEmpty(args)){
       return nl->SymbolAtom(DateTime::BasicType());
   }
   ErrorReporter::ReportError("no arguments allowed");
   return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr MinMaxDurationTM(ListExpr args){
   if(nl->IsEmpty(args)){
       return nl->SymbolAtom(Duration::BasicType());
   }
   ErrorReporter::ReportError("no arguments allowed");
   return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr CreateDurationTM(ListExpr args){
  if(nl->ListLength(args)==1){
    if ( nl->IsEqual(nl->First(args),CcReal::BasicType()) ) {
      return nl->SymbolAtom(Duration::BasicType());
    } else {
      ErrorReporter::ReportError("one real value expected\n");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  if(nl->ListLength(args)==2){
    if(nl->IsEqual(nl->First(args),CcInt::BasicType()) &&
       nl->IsEqual(nl->Second(args),CcInt::BasicType())) {
      return nl->SymbolAtom(Duration::BasicType());
    } else {
          ErrorReporter::ReportError("two int values expected\n");
          return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("One or two arguments required\n");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr CreateInstantTM(ListExpr args){
  if(nl->ListLength(args)==1){
    if ( nl->IsEqual(nl->First(args),CcReal::BasicType()) ) {
      return nl->SymbolAtom(DateTime::BasicType());
    } else {
      ErrorReporter::ReportError("one real value expected\n");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  if(nl->ListLength(args)==2){
    if(nl->IsEqual(nl->First(args),CcInt::BasicType()) &&
       nl->IsEqual(nl->Second(args),CcInt::BasicType())) {
      return nl->SymbolAtom(DateTime::BasicType());
    } else {
      ErrorReporter::ReportError("two int values expected\n");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("One or two arguments required\n");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}


ListExpr Duration2RealTM(ListExpr args){
  if(nl->ListLength(args)==1){
    if ( nl->IsEqual(nl->First(args),Duration::BasicType()) ) {
      return nl->SymbolAtom(CcReal::BasicType());
    } else {
      ErrorReporter::ReportError("one duration value expected\n");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("One argument required\n");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr Instant2RealTM(ListExpr args){
  if(nl->ListLength(args)==1){
    if ( nl->IsEqual(nl->First(args),DateTime::BasicType()) ) {
      return nl->SymbolAtom(CcReal::BasicType());
    } else {
      ErrorReporter::ReportError("one instant value expected\n");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("One argument required\n");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr InstantOrDurationIntTM(ListExpr args){
  if(nl->ListLength(args)==1){
     if(nl->IsEqual(nl->First(args),DateTime::BasicType())){
         return nl->SymbolAtom(CcInt::BasicType());
     }
     if(nl->IsEqual(nl->First(args),Duration::BasicType())){
         return nl->SymbolAtom(CcInt::BasicType());
     }
     else {
         ErrorReporter::ReportError("instant/duration parameter expected \n");
     }
  }
  ErrorReporter::ReportError("exactly one argument required");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}


ListExpr str2instantTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("string expected");
  }
  if(!listutils::isSymbol(nl->First(args),CcString::BasicType()) &&
     !listutils::isSymbol(nl->First(args),FText::BasicType())){
    return listutils::typeError("string expected");
  }
  return nl->SymbolAtom(DateTime::BasicType());

}



/*
4.2 Value Mappings

*/
int LeapYearFun(Word* args, Word& result, int message,
                Word& local, Supplier s){
    result = qp->ResultStorage(s);
    CcInt* Y = (CcInt*) args[0].addr;
    DateTime T;
    bool res = T.IsValid(Y->GetIntval(),2,29);
    ((CcBool*) result.addr)->Set(true,res);
    return 0;
}

int TheInstantFun_Int1(Word* args, Word& result,
                      int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    ((DateTime*) result.addr)->Set(((CcInt*)args[0].addr)->GetIntval());
    return 0;
}

int TheInstantFun_Int2(Word* args, Word& result, int message,
                       Word& local, Supplier s){
    result = qp->ResultStorage(s);
    ((DateTime*) result.addr)->Set(((CcInt*)args[0].addr)->GetIntval(),
                                   ((CcInt*)args[1].addr)->GetIntval()
                                  );
    return 0;
}

int TheInstantFun_Int3(Word* args, Word& result, int message,
                    Word& local, Supplier s){
    result = qp->ResultStorage(s);
    ((DateTime*) result.addr)->Set(((CcInt*)args[0].addr)->GetIntval(),
                                   ((CcInt*)args[1].addr)->GetIntval(),
                                   ((CcInt*)args[2].addr)->GetIntval()
                                  );
    return 0;
}

int TheInstantFun_Int4(Word* args, Word& result, int message,
                       Word& local, Supplier s){
    result = qp->ResultStorage(s);
    ((DateTime*) result.addr)->Set(((CcInt*)args[0].addr)->GetIntval(),
                                   ((CcInt*)args[1].addr)->GetIntval(),
                                   ((CcInt*)args[2].addr)->GetIntval(),
                                   ((CcInt*)args[3].addr)->GetIntval()
                                  );
    return 0;
}

int TheInstantFun_Int5(Word* args, Word& result, int message,
                       Word& local, Supplier s){
    result = qp->ResultStorage(s);
    ((DateTime*) result.addr)->Set(((CcInt*)args[0].addr)->GetIntval(),
                                   ((CcInt*)args[1].addr)->GetIntval(),
                                   ((CcInt*)args[2].addr)->GetIntval(),
                                   ((CcInt*)args[3].addr)->GetIntval(),
                                   ((CcInt*)args[4].addr)->GetIntval()
                                  );
    return 0;
}

int TheInstantFun_Int6(Word* args, Word& result, int message,
                       Word& local, Supplier s){
    result = qp->ResultStorage(s);
    ((DateTime*) result.addr)->Set(((CcInt*)args[0].addr)->GetIntval(),
                                   ((CcInt*)args[1].addr)->GetIntval(),
                                   ((CcInt*)args[2].addr)->GetIntval(),
                                   ((CcInt*)args[3].addr)->GetIntval(),
                                   ((CcInt*)args[4].addr)->GetIntval(),
                                   ((CcInt*)args[5].addr)->GetIntval()
                                  );
    return 0;
}

int TheInstantFun_Int7(Word* args, Word& result, int message,
                       Word& local, Supplier s){
    result = qp->ResultStorage(s);
    ((DateTime*) result.addr)->Set(((CcInt*)args[0].addr)->GetIntval(),
                                   ((CcInt*)args[1].addr)->GetIntval(),
                                   ((CcInt*)args[2].addr)->GetIntval(),
                                   ((CcInt*)args[3].addr)->GetIntval(),
                                   ((CcInt*)args[4].addr)->GetIntval(),
                                   ((CcInt*)args[5].addr)->GetIntval(),
                                   ((CcInt*)args[6].addr)->GetIntval()
                                  );
    return 0;
}

int NowFun(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    ((DateTime*) result.addr)->Now();
    return 0;
}

int TodayFun(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    ((DateTime*) result.addr)->Today();
    return 0;
}


int DayFun(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T = (DateTime*) args[0].addr;
    ((CcInt*) result.addr)->Set(true,T->GetGregDay());
    return 0;
}

int DurationDayFun(Word* args, Word& result, int message, Word& local,
                   Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T = (DateTime*) args[0].addr;
    ((CcInt*) result.addr)->Set(true,T->GetDay());
    return 0;
}

int MonthFun(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T = (DateTime*) args[0].addr;
    ((CcInt*) result.addr)->Set(true,T->GetMonth());
    return 0;
}

int YearFun(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T = (DateTime*) args[0].addr;
    ((CcInt*) result.addr)->Set(true,T->GetYear());
    return 0;
}

int HourFun(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T = (DateTime*) args[0].addr;
    ((CcInt*) result.addr)->Set(true,T->GetHour());
    return 0;
}

int MinuteFun(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T = (DateTime*) args[0].addr;
    ((CcInt*) result.addr)->Set(true,T->GetMinute());
    return 0;
}

int SecondFun(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T = (DateTime*) args[0].addr;
    ((CcInt*) result.addr)->Set(true,T->GetSecond());
    return 0;
}

int MillisecondFun(Word* args, Word& result, int message,
                   Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T = (DateTime*) args[0].addr;
    ((CcInt*) result.addr)->Set(true,T->GetMillisecond());
    return 0;
}

int DurationMillisecondFun(Word* args, Word& result, int message,
                           Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T = (DateTime*) args[0].addr;
    ((CcInt*) result.addr)->Set(true,T->GetAllMilliSeconds());
    return 0;
}

int AddFun(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T1 = (DateTime*) args[0].addr;
    DateTime* T2 = (DateTime*) args[1].addr;
    DateTime TRes = (*T1) + (*T2);
    ((DateTime*) result.addr)->Equalize(&TRes);
    return 0;
}

int MinusFun(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T1 = (DateTime*) args[0].addr;
    DateTime* T2 = (DateTime*) args[1].addr;
    DateTime TRes = (*T1)-(*T2);
    ((DateTime*) result.addr)->Equalize(&TRes);
    return 0;
}

int EqualsFun(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T1 = (DateTime*) args[0].addr;
    DateTime* T2 = (DateTime*) args[1].addr;
    bool res = (*T1) == (*T2);
    ((CcBool*) result.addr)->Set(true,res);
    return 0;
}

int BeforeFun(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T1 = (DateTime*) args[0].addr;
    DateTime* T2 = (DateTime*) args[1].addr;
    bool res = (*T1) < (*T2);
    ((CcBool*) result.addr)->Set(true,res);
    return 0;
}

int AfterFun(Word* args, Word& result, int message, Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T1 = (DateTime*) args[0].addr;
    DateTime* T2 = (DateTime*) args[1].addr;
    bool res = (*T1) > (*T2);
    ((CcBool*) result.addr)->Set(true,res);
    return 0;
}

int MulFun(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  DateTime* T1 = (DateTime*) args[0].addr;
  CcReal* Fact = (CcReal*) args[1].addr;
  DateTime* TRes = T1->Clone();
  TRes->Mul(Fact->GetRealval());
  ((DateTime*) result.addr)->Equalize(TRes);
  delete TRes;
  return 0;
}

int DivFun(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  DateTime* T1 = (DateTime*) args[0].addr;
  DateTime* T2 = (DateTime*) args[1].addr;
  DateTime Remainder(durationtype);
  int32_t res = (int32_t) (T1->Div( (*T2),Remainder));
  ((CcInt*) result.addr)->Set(true,res);
  return 0;
}

int MinFun(Word* args, Word& result, int message,
                  Word& local, Supplier s){
  result = qp->ResultStorage(s);
  ((DateTime*) result.addr)->ToMinimum();
  return 0;
}

int MaxFun(Word* args, Word& result, int message,
                  Word& local, Supplier s){
  result = qp->ResultStorage(s);
  ((DateTime*) result.addr)->ToMaximum();
  return 0;
}

int WeekdayFun(Word* args, Word& result, int message,
               Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T = (DateTime*) args[0].addr;
    int day = T->GetWeekday();
    STRING_T* WD;
    switch(day){
       case 0 :  WD = (STRING_T*) "Monday";
                 break;
       case 1 : WD =  (STRING_T*) "Tuesday";
                 break;
       case 2 : WD =  (STRING_T*) "Wednesday";
                break;
       case 3 : WD = (STRING_T*) "Thursday";
                break;
       case 4 : WD = (STRING_T*) "Friday";
                break;
       case 5 : WD = (STRING_T*) "Saturday";
                break;
       case 6 :  WD = (STRING_T*) "Sunday";
                 break;

       default : WD = (STRING_T*)"Errorsday";
                 break;
    }
    ((CcString*)result.addr)->Set(true,WD);
    return 0;
}

int DateTime2RealFun(Word* args, Word& result, int message,
               Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime* T = (DateTime*) args[0].addr;
    if(T->IsDefined())
      {
        double days = T->ToDouble();
        ((CcReal*)result.addr)->Set(true,days);
      }
    else
      {
        ((CcReal*)result.addr)->Set(false,0.0);
      }
    return 0;
}


int CreateDurationFromRealFun(Word* args, Word& result, int message,
                              Word& local, Supplier s){
    result = qp->ResultStorage(s);
    CcReal* R = (CcReal*) args[0].addr;
    DateTime* T = (DateTime*)result.addr;

    T->SetType(durationtype);
    if(R->IsDefined())
      {
        double days = R->GetRealval();
        T->ReadFrom(days);
        T->SetDefined(true);
      }
    else
      {
        T->ReadFrom(0.0);
        T->SetDefined(false);
      }
    return 0;
}

int CreateDurationFromIntIntFun(Word* args, Word& result, int message,
                                Word& local, Supplier s){
    result = qp->ResultStorage(s);
    CcInt* Idays = (CcInt*) args[0].addr;
    CcInt* Imsec = (CcInt*) args[1].addr;
    DateTime* T = (DateTime*)result.addr;

    T->SetType(durationtype);
    if(Idays->IsDefined() && Imsec->IsDefined() && Imsec->GetIntval() >= 0.0)
      {
        *T = DateTime(Idays->GetIntval(),Imsec->GetIntval(),durationtype);
        T->SetDefined(true);
      }
    else
      {
        *T = DateTime(0,0,durationtype);
        T->SetDefined(false);
      }
    return 0;
}

int CreateInstantFromRealFun(Word* args, Word& result, int message,
                              Word& local, Supplier s){
    result = qp->ResultStorage(s);
    CcReal* R = (CcReal*) args[0].addr;
    DateTime* T = (DateTime*)result.addr;

    T->SetType(instanttype);
    if(R->IsDefined())
      {
        double days = R->GetRealval();
        T->ReadFrom(days);
        T->SetDefined(true);
      }
    else
      {
        T->ReadFrom(0.0);
        T->SetDefined(false);
      }
    return 0;
}

int CreateInstantFromIntIntFun(Word* args, Word& result, int message,
                                Word& local, Supplier s){
    result = qp->ResultStorage(s);
    CcInt* Idays = (CcInt*) args[0].addr;
    CcInt* Imsec = (CcInt*) args[1].addr;
    DateTime* T = (DateTime*)result.addr;

    T->SetType(instanttype);
    if(Idays->IsDefined() && Imsec->IsDefined() && Imsec->GetIntval() >= 0.0)
      {
        *T = DateTime(Idays->GetIntval(),Imsec->GetIntval(),instanttype);
        T->SetDefined(true);
      }
    else
      {
        *T = DateTime(0,0,instanttype);
        T->SetDefined(false);
      }
    return 0;
}


int DateTimeToStringFun(Word* args, Word& result, int message,
                                Word& local, Supplier s){
    result = qp->ResultStorage(s);
    DateTime *T = static_cast<DateTime*>(args[0].addr);
    string rs = T->ToString();
    ( static_cast<CcString*>(result.addr))->Set(true,rs);
   return 0;
}


template<class T>
int str2instantVM(Word* args, Word& result, int message,
                                Word& local, Supplier s){
  result = qp->ResultStorage(s);
  DateTime* res = static_cast<DateTime*>(result.addr);
  T* arg = static_cast<T*>(args[0].addr);
  if(!arg->IsDefined()){
    res->SetDefined(false);
  } else {
    if(!res->ReadFrom(arg->GetValue())){
       res->SetDefined(false);
    }
  }
  return 0;
}




/*
4.3 Specifications

*/
const string LeapYearSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"int -> bool\""
   " \" _ leapyear \" "
   "   \"checks whether the given int is a leap year\" "
   "   \" query 2000 leapyear\" ))";

const string TheInstantSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" \"Example\" )"
   " ( \" int (x int){0-6} -> instant\""
   " \" theInstant(_,_,_,_,_,_,_) \" "
   "   \"creates an instant from the arguments\""
   " \"arguments are from years down to milliseconds\""
   "   \" query theInstant(2004,7,17)\" ))";

const string NowSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \" -> instant\""
   " \" now \" "
   "   \"creates an instant from the current systemtime\" "
   "   \" query now()\" ))";

const string TodaySpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \" -> instant\""
   " \" today \" "
   "   \"creates an instant from the current systemtime\" "
   "   \" query today()\" ))";

const string DaySpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"instant -> int\nduration -> int\""
   "\" day_of ( _ ) \" "
   "\"return the day of this instant/duration\" "
   "\"query day_of(T1) \" ))";

const string MonthSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"instant -> int\""
   " \" month_of ( _ ) \" "
   "   \"return the month of this instant\" "
   "   \" query month_of(T1)  \" ))";

const string YearSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"instant -> int\""
   " \" year_of ( _ ) \" "
   "   \"return the year of this instant\" "
   "   \" query year_of(T1) \" ))";

const string HourSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"instant -> int\""
   " \" hour_of(_)\" "
   "   \"return the hour of this instant\" "
   "   \" query hour_of(T1) \" ))";

const string MinuteSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"instant -> int\""
   " \"minute_of(_) \" "
   "   \"return the minute of this instant\" "
   "   \" query minute_of(T1) \" ))";

const string SecondSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"instant -> int\""
   " \" second_of ( _ )\" "
   "   \"return the second of this instant\" "
   "   \" query second_of(T1) \" ))";

const string MillisecondSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"instant -> int\nduration -> int\""
   "\" millisecond_of(_) \" "
   "\"return the millisecond of this instant/duration\" "
   "\"query millisecond_of(T1) \" ))";

const string AddSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"DateTime x DateTime -> DateTime\""
   " \" _ + _ \" "
   "   \" DateTime stands for instant or duration\" "
   "   \" query T1 + D2\" ))";

const string MinusSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"DateTime x DateTime -> DateTime\""
   " \" _ - _ \" "
   "   \"Computes the difference\" "
   "   \" query T1 - T2\" ))";


const string EqualsSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"DateTime_i x DateTime_i -> bool\""
   "\" _ = _ \" "
   "   \"checks for equality\" "
   "   \" query T1 = T2\" ))";

const string LessSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"DateTime_i x DateTime_i -> bool\""
   " \" _ < _ \" "
   "   \"returns true if T1 is before (shorter than) t2\" "
   "   \" query T1 < T2\" ))";

const string GreaterSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"DateTime_i x DateTime_i -> bool\""
   " \" _ > _ \" "
   "   \"returns true if T1 is after (longer than) T2\" "
   "   \" query T1 > T2\" ))";

const string MulSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"duration x real -> duration\""
   " \" _ * _ \" "
   "   \"computes the multiple of a duration\" "
   "   \" query D * 7.0\" ))";

const string WeekdaySpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"instant -> string\""
   " \"  weekday_of ( _ ) \" "
   "   \"returns the weekday in human readable format\" "
   "   \"query weekday_of(today())\" ))";

const string DivSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"duration x duration -> int\""
   " \"  _ / _  \" "
   " 'Computes how often the second argument is part of the first one' "
   "   \"query a / b \" ))";

const string MinInstantSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \" -> instant\""
   " \"  minInstant()  \" "
   "   \"returns the minimum possible instant \" "
   "   \"query minInstant() \" ))";

const string MaxInstantSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \" -> instant\""
   " \"  maxInstant()  \" "
   "   \"returns the maximum possible instant \" "
   "   \"query maxInstant() \" ))";

const string MinDurationSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \" -> duration \""
   " \"  minDuration()  \" "
   "   'returns the minimum representable duration value ' "
   "   \"query minDuration() \" ))";

const string MaxDurationSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \" -> duration \""
   " \"  maxDuration()  \" "
   "   'returns the maximum representable duration value'  "
   "   \"query maxDuration() \" ))";

const string Duration2RealSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"duration -> real\""
   "\"duration2real( _ )  \" "
   "'Converts the duration to a real giving the duration in days.'  "
   "'query duration2real([const duration value (-5 1000)])' ))";

const string Instant2RealSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"instant -> real\""
   "\"instant2real( _ )  \" "
   "'Converts the instant to a real giving the distance to "
   "the NULL_DAY in days.'  "
   "'query duration2real([const instant value (-5 1000)])' ))";

const string CreateDurationSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"(real) -> duration\n(int int) -> duration \""
   "\"create_duration( _ )\ncreate_duration( _ , _ )\" "
   "'Create a duration value from a real (days) or a pair of int "
   "(days, milliseconds). Parameter milliseconds must be >=0'  "
   "\"query create_duration(10, 5) \" ))";

const string CreateInstantSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"(real) -> instant\n(int int) -> instant \""
   "\"create_instant( _ )\ncreate_instant( _ , _ )\" "
   "'Create an instant value from a real (days) or a pair of int "
   "(days, milliseconds). Parameter milliseconds must be >=0'  "
   "\"query create_instant(10, 5) \" ))";

const string ToStringSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  " ( \"{instant, duration} -> string \""
   "\" tostring(_)\" "
   "'returns a string representation of an instant'  "
   "\"query tostring(now()) \" ))";


const string str2instantSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  " ( \"{string,text} -> instant \""
   "\" str2instant(_)\" "
   "'reads a instant from a string'  "
   "\"query str2instant(tostring(now))  \" ))";



/*
4.3 ValueMappings of overloaded Operators

*/

ValueMapping TheInstantValueMap[] = {
        TheInstantFun_Int1,TheInstantFun_Int2,TheInstantFun_Int3,
        TheInstantFun_Int4,TheInstantFun_Int5,TheInstantFun_Int6,
        TheInstantFun_Int7 };

ValueMapping CreateDurationValueMap[] = {
        CreateDurationFromRealFun,
        CreateDurationFromIntIntFun };

ValueMapping CreateInstantValueMap[] = {
        CreateInstantFromRealFun,
        CreateInstantFromIntIntFun };

ValueMapping DayValueMap[] = {
        DayFun,
        DurationDayFun};

ValueMapping MillisecondValueMap[] = {
        MillisecondFun,
        DurationMillisecondFun};

ValueMapping str2instantvm[] = {
      str2instantVM<CcString>,
      str2instantVM<FText>
   };



/*
4.4 SelectionFunctions

*/

static int TheInstantSelect(ListExpr args){
  return nl->ListLength(args)-1;
}

static int InstantOrDurationIntSelect(ListExpr args){
  if(nl->IsEqual(nl->First(args),DateTime::BasicType()))
    return 0;
  if(nl->IsEqual(nl->First(args),Duration::BasicType()))
    return 1;
  return -1; // should not happen
}

static int str2instantSelect(ListExpr args){
  if(listutils::isSymbol(nl->First(args),CcString::BasicType())){
     return 0;
  } else { // text
     return 1;
  }

}



/*
4.4 Definition of Operators

*/
Operator dt_leapyear(
       "leapyear", // name
       LeapYearSpec, // specification
       LeapYearFun,
       Operator::SimpleSelect,
       IntBool);

Operator dt_now(
       "now", // name
       NowSpec, // specification
       NowFun,
       Operator::SimpleSelect,
       VoidInstant);

Operator dt_today(
       "today", // name
       TodaySpec, // specification
       TodayFun,
       Operator::SimpleSelect,
       VoidInstant);

Operator dt_day(
       "day_of",            // name
       DaySpec,             // specification
       2,                   // number of functions
       DayValueMap,
       InstantOrDurationIntSelect,
       InstantOrDurationIntTM);

Operator dt_month(
       "month_of", // name
       MonthSpec, // specification
       MonthFun,
       Operator::SimpleSelect,
       InstantInt);

Operator dt_year(
       "year_of", // name
       YearSpec, // specification
       YearFun,
       Operator::SimpleSelect,
       InstantInt);

Operator dt_hour(
       "hour_of", // name
       HourSpec, // specification
       HourFun,
       Operator::SimpleSelect,
       InstantInt);

Operator dt_minute(
       "minute_of", // name
       MinuteSpec, // specification
       MinuteFun,
       Operator::SimpleSelect,
       InstantInt);

Operator dt_second(
       "second_of", // name
       SecondSpec, // specification
       SecondFun,
       Operator::SimpleSelect,
       InstantInt);

Operator dt_millisecond(
       "millisecond_of",           // name
       MillisecondSpec,            // specification
       2,                          // number of functions
       MillisecondValueMap,
       InstantOrDurationIntSelect,
       InstantOrDurationIntTM);

Operator dt_add(
       "+", // name
       AddSpec, // specification
       AddFun,
       Operator::SimpleSelect,
       PlusCheck);

Operator dt_minus(
       "-", // name
       MinusSpec, // specification
       MinusFun,
       Operator::SimpleSelect,
       MinusCheck);

Operator dt_div(
       "/", // name
       DivSpec, // specification
       DivFun,
       Operator::SimpleSelect,
       DivTM);

Operator dt_minInstant(
       "minInstant", // name
       MinInstantSpec, // specification
       MinFun,
       Operator::SimpleSelect,
       MinMaxInstantTM);

Operator dt_maxInstant(
       "maxInstant", // name
       MaxInstantSpec, // specification
       MaxFun,
       Operator::SimpleSelect,
       MinMaxInstantTM);

Operator dt_maxDuration(
       "maxDuration", // name
       MaxDurationSpec, // specification
       MaxFun,
       Operator::SimpleSelect,
       MinMaxDurationTM);

Operator dt_minDuration(
       "minDuration", // name
       MinDurationSpec, // specification
       MinFun,
       Operator::SimpleSelect,
       MinMaxDurationTM);

Operator dt_less(
       "<", // name
       LessSpec, // specification
       BeforeFun,
       Operator::SimpleSelect,
       CheckComparisons);

Operator dt_greater(
       ">", // name
       GreaterSpec, // specification
       AfterFun,
       Operator::SimpleSelect,
       CheckComparisons);

Operator dt_equals(
       "=", // name
       EqualsSpec, // specification
       EqualsFun,
       Operator::SimpleSelect,
       CheckComparisons);


Operator dt_mul(
       "*", // name
       MulSpec, // specification
       MulFun,
       Operator::SimpleSelect,
       DurationRealDuration);

Operator dt_weekday(
       "weekday_of", // name
       WeekdaySpec, // specification
       WeekdayFun,
       Operator::SimpleSelect,
       InstantString);

Operator dt_theInstant(
       "theInstant",               // name
       TheInstantSpec,             // specification
       7,                          // number of functions
       TheInstantValueMap,
       TheInstantSelect,
       TheInstantTM);

Operator dt_duration2real(
       "duration2real",    // name
       Duration2RealSpec,  // specification
       DateTime2RealFun,
       Operator::SimpleSelect,
       Duration2RealTM );

Operator dt_instant2real(
       "instant2real",    // name
       Instant2RealSpec,  // specification
       DateTime2RealFun,
       Operator::SimpleSelect,
       Instant2RealTM );

Operator dt_create_duration(
       "create_duration",          // name
       CreateDurationSpec,         // specification
       2,                          // number of functions
       CreateDurationValueMap,
       TheInstantSelect,
       CreateDurationTM);

Operator dt_create_instant(
       "create_instant",          // name
       CreateInstantSpec,         // specification
       2,                          // number of functions
       CreateInstantValueMap,
       TheInstantSelect,
       CreateInstantTM);

Operator dt_tostring(
       "tostring",    // name
       ToStringSpec,  // specification
       DateTimeToStringFun,
       Operator::SimpleSelect,
       DateTimeString );


Operator str2instant(
       "str2instant",
        str2instantSpec,
        2,
        str2instantvm,
        str2instantSelect,
        str2instantTM
    );

/*
5 Creating the Algebra

*/
class DateTimeAlgebra : public Algebra
{
 public:
  DateTimeAlgebra() : Algebra()
  {
    // type constructors
    AddTypeConstructor( &instant );
    instant.AssociateKind( Kind::DATA() );
    instant.AssociateKind( Kind::INDEXABLE() );
    instant.AssociateKind( Kind::CSVEXPORTABLE() );
    instant.AssociateKind( Kind::CSVIMPORTABLE() );
    AddTypeConstructor( &duration );
    duration.AssociateKind( Kind::DATA() );
    duration.AssociateKind( Kind::CSVEXPORTABLE() );
    duration.AssociateKind( Kind::CSVIMPORTABLE() );

    // operators
    AddOperator(&dt_add);
    AddOperator(&dt_minus);
    AddOperator(&dt_equals);
    AddOperator(&dt_less);
    AddOperator(&dt_greater);
    AddOperator(&dt_mul);
    AddOperator(&dt_weekday);
    AddOperator(&dt_leapyear);
    AddOperator(&dt_year);
    AddOperator(&dt_month);
    AddOperator(&dt_day);
    AddOperator(&dt_hour);
    AddOperator(&dt_minute);
    AddOperator(&dt_second);
    AddOperator(&dt_millisecond);
    AddOperator(&dt_now);
    AddOperator(&dt_today);
    AddOperator(&dt_theInstant);
    AddOperator(&dt_div);
    AddOperator(&dt_minInstant);
    AddOperator(&dt_maxInstant);
    AddOperator(&dt_minDuration);
    AddOperator(&dt_maxDuration);
    AddOperator(&dt_create_duration);
    AddOperator(&dt_create_instant);
    AddOperator(&dt_duration2real);
    AddOperator(&dt_instant2real);
    AddOperator(&dt_tostring);
    AddOperator(&str2instant);
  }
  ~DateTimeAlgebra() {};
};


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
InitializeDateTimeAlgebra( NestedList* nlRef,
                           QueryProcessor* qpRef,
                           AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (new DateTimeAlgebra());
}

} // end of namespace


ostream& operator<<(ostream& o, const datetime::DateTime& DT) {
   return DT.Print(o);
}


