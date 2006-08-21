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
#include "StandardAttribute.h"
#include "StandardTypes.h"
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include "LogMsg.h"
#include <limits>


#define POS "DateTimeAlgebra.cpp:" << __LINE__

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;


static long min_long = numeric_limits<long>::min(); 
static long max_long = numeric_limits<long>::max();
static int min_int = numeric_limits<int>::min(); 
static int max_int = numeric_limits<int>::max();

static string begin_of_time="begin of time";
static string end_of_time="end of time";

static long MAX_REPRESENTABLE = 2450000;
static long MIN_REPRESENTABLE = -2450000;


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
DateTime::DateTime(){}

/*
~Constructor~

This constructor creates a DateTime at the NULL[_]DAY and
duration zero, respectively.

*/
DateTime::DateTime(const TimeType type){
  day=0;
  milliseconds=0;
  defined = true;
  canDelete=false;
  this->type = type;
}


/*
~A Constructor ~

The Value of MilliSeconds has to be greater than or equals to zero.

*/
DateTime::DateTime(const long Day,const long MilliSeconds,const TimeType type){
   assert(MilliSeconds>=0);
   day = Day;
   milliseconds = MilliSeconds;
   if(milliseconds>=MILLISECONDS){
      long dif = milliseconds / MILLISECONDS;
      day += dif;
      milliseconds -= dif*MILLISECONDS;
   }
   defined = true;
   canDelete=false;
   this->type = type;
}

/*
~Constructor~

This conmstructor creates a DateTime with the same value like the
argument.

*/
DateTime::DateTime(const DateTime& DT){
   Equalize(&DT);
}

/*
~Destructor~

*/
DateTime::~DateTime(){}

void DateTime::Destroy(){
   canDelete=true;
}

/*
~Set~

This function sets an instant to the given values.

*/
void DateTime::Set(const int year,const int month, const int day,
            const int hour, const int minute, const int second,
            const int millisecond){

   assert(type==instanttype);
   long ms = ((hour*60+minute)*60+second)*1000+millisecond;
   long d = ToJulian(year,month,day);
   long dif = ms / MILLISECONDS;
   ms = ms - dif;
   d = d + dif;
   if(ms < 0){
      ms = ms + MILLISECONDS;
      d--;
   }
   this->day = d;
   this->milliseconds = ms;
   this->defined = true;
}


/*
~SetType~

*/
void DateTime::SetType(const TimeType TT){
    type = TT;
}

/*
~GetType~

*/
TimeType DateTime::GetType()const{
    return type;
}



/*
~Now~

Gets the value of this DateTime from the System.
Cannot be applied to a duration.

*/
void DateTime::Now(){
  assert(type != durationtype);
  timeb tb;
  time_t now;
  int ms;
  ftime(&tb);
  now = tb.time;
  ms = tb.millitm;
  tm* time = localtime(&now);
  day = ToJulian(time->tm_year+1900,time->tm_mon+1,time->tm_mday);
  milliseconds = ((((time->tm_hour)*60)+time->tm_min)*
                     60+time->tm_sec)*1000+ms;
}

/*
~Today~

This function reads this DateTime value from the Systemtime. The time part is
ignored here. This means, that hour ... millisecond are assumed to be zero.
Cannot applied to a duration.

*/
void DateTime::Today(){
   assert(type != durationtype);
   time_t today;
   time(&today);
   tm* lt = localtime(&today);
   day = ToJulian(lt->tm_year+1900,lt->tm_mon+1,lt->tm_mday);
   milliseconds = 0;
}

/*
~ToMinimum~

Sets this instant to the mimimum possible value.

*/
void DateTime::ToMinimum(){
   defined = true;
   day = min_long;
   milliseconds = 0; 
}


/*
~ToMaximum~

Sets this instant to the maximum possible value.

*/
void DateTime::ToMaximum(){
   defined = true;
   day = max_long;
   milliseconds = MILLISECONDS-1; 
}


/*
~IsMinimum~

Checks if this value is the most minimum representable one.

*/
bool DateTime::IsMinimum()const {
  if(!defined){
     return false;
  }
  return day==min_long && milliseconds==0; 
}

/*
~IsMaximum~

Checks if this value is the most maximum representable one.

*/
bool DateTime::IsMaximum()const{
  if(!defined){
     return false;
  }
  return day==max_long && milliseconds==MILLISECONDS-1; 
}


/*
~GetDay~

This functions yields the day-part of a duration.
The function can't applied to an instant.

*/
long DateTime::GetDay()const{
   assert(type!=instanttype);
   return day;
}

/*
~GetAllMilliSeconds~

This function returns the milliseconds of the day
of this Time instance.

*/
long DateTime::GetAllMilliSeconds()const{
   return milliseconds;
}

/*
~Gregorian Calender~

The following functions return single values of this DateTime instance.
This functions cannot applied to durations.

*/

int DateTime::GetGregDay()const{
    assert(type!=durationtype);
    long y;
    int m,d;
    ToGregorian(day,y,m,d);
    return d;
}

int DateTime::GetMonth()const{
   assert(type!=durationtype);
   long y;
   int m,d;
   ToGregorian(day,y,m,d);
   return m;
}
long DateTime::GetYear()const{
   assert(type!=durationtype);
   long y;
   int m,d;
   ToGregorian(day,y,m,d);
   return y;
}

int DateTime::GetHour()const{
   assert(type!=durationtype);
   return (int)(milliseconds / 3600000);
}

int DateTime::GetMinute()const{
    assert(type!=durationtype);
   return (int) ( (milliseconds / 60000) % 60);
}

int DateTime::GetSecond()const{
  assert(type!=durationtype);
  return (int) ( (milliseconds / 1000) % 60);
}

int DateTime::GetMillisecond()const{
  assert(type!=durationtype);
  return (int) (milliseconds % 1000);
}

int DateTime::GetWeekday()const{
    return day % 7;
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
long DateTime::ToJulian(const int year, const int month, const int day) const{
  int jy = year;
  if (year < 0)
     jy++;
  int jm = month;
  if (month > 2)
     jm++;
  else{
     jy--;
     jm += 13;
  }

  int jul = (int)(floor(365.25 * jy) + floor(30.6001*jm)
                  + day + 1720995.0);
  int IGREG = 15 + 31*(10+12*1582);
  // Gregorian Calendar adopted Oct. 15, 1582
  if (day + 31 * (month + 12 * year) >= IGREG){
     // change over to Gregorian calendar
     int ja = (int)(0.01 * jy);
     jul += 2 - ja + (int)(0.25 * ja);
  }
  return jul-NULL_DAY;
}

/*
~ToGregorian~

This function converts a Julian day to a date in the gregorian calender.

This algorithm is from Press et al., Numerical Recipes
in C, 2nd ed., Cambridge University Press 1992

*/
void DateTime::ToGregorian(const long Julian, long &year,
                           int &month, int &day) const{
  int j=(int)(Julian+NULL_DAY);
   int ja = j;
   int JGREG = 2299161;
   /* the Julian date of the adoption of the Gregorian
      calendar
   */
    if (j >= JGREG){
    /* cross-over to Gregorian Calendar produces this
       correction
    */
       int jalpha = (int)(((float)(j - 1867216) - 0.25)/36524.25);
       ja += 1 + jalpha - (int)(0.25 * jalpha);
    }
    int jb = ja + 1524;
    int jc = (int)(6680.0 + ((float)(jb-2439870) - 122.1)/365.25);
    int jd = (int)(365 * jc + (0.25 * jc));
    int je = (int)((jb - jd)/30.6001);
    day = jb - jd - (int)(30.6001 * je);
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

/*

~ToDouble~

This function converts this Time instance to the corresponding
double value;

*/
   double DateTime::ToDouble() const{
   return (double)day + (double)milliseconds/MILLISECONDS;
}

/*
~ToString~

This function returns the string representation of this DateTime instance.

*/
string DateTime::ToString() const{
  ostringstream tmp;
  if(!defined){
    return "undefined";
  }
  if(type==durationtype){ //a duration
    tmp << day << ";";
    tmp << milliseconds;
  }else if(type==instanttype){ // an instant
     if(IsMinimum()){
       return begin_of_time;
     }
     if(IsMaximum()){
       return end_of_time;
     }
     // SOME DATES CAN'T BE CONVERTED CORRECTLY INTO THE GREGORIAN
     // calendar
    if(day < MIN_REPRESENTABLE || day >MAX_REPRESENTABLE){
        tmp << ToDouble();
        return tmp.str(); 
    } 

    int day,month;
    long year;
    ToGregorian(this->day,year,month,day);
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
    long value = milliseconds;
    long ms = value % 1000;
    value = value / 1000;
    long sec = value % 60;
    value = value / 60;
    long min = value % 60;
    long hour = value / 60;

    if(milliseconds==0) // without time
       return tmp.str();

    tmp << "-";
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
    tmp << "unknown type, def=" << defined << " candel=" << canDelete 
        << " type=" << type << " day = " << day << " ms = " 
        << milliseconds;
  }
  return tmp.str();
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
bool DateTime::ReadFrom(const string Time){
   assert(type!=durationtype);
   canDelete = false;
   if(Time=="undefined"){
      defined=false;
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
   int year = 0;
   int digit;
   int len = Time.length();
   if(len==0) return false;
   int pos = 0;
   // read the year
   int signum = 1;
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
   int month = 0;
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
   int day = 0;
   while(Time[pos]!='-'){
      digit = datetime::GetValue(Time[pos]);
      if(digit<0) return false;
      pos++;
      day = day*10+digit;
      if(pos==len){ // we allow pure date string without any hour
         if(!IsValid(year,month,day))
            return false;
         this->day = ToJulian(year,month,day);;
         milliseconds=0;
         defined=true;
         return true;
      }
   }
   pos++; // read over '-'
   if(pos==len) return false;
   if(!IsValid(year,month,day))
      return false;
   // read the hour
   int hour = 0;
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
   int minute = 0;
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
   long seconds = 0;
   long mseconds = 0;
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
   this->day = ToJulian(year,month,day);
   milliseconds = ((hour*60+minute)*60+seconds)*1000+mseconds;
   if(milliseconds>MILLISECONDS){
      long dif = milliseconds/MILLISECONDS;
      this->day += dif;
      milliseconds -= dif*MILLISECONDS;
   }
   defined=true;
   return true;
}


/*
~ReadFrom~

This functions reads the value of this instance from the given double.

*/
bool DateTime::ReadFrom(const double Time){
   day = (long) Time;
   long dms =  (long) ((Time - (double) day)*MILLISECONDS+0.5);
   if( dms<0 ){
      day--;
      dms += MILLISECONDS;
   }
   milliseconds = dms;
   defined = true;
   return true;
}

/*
~IsValid~

This functions checks if the given arguments represent a valid gregorian
date. E.g. this function will return false if month is greater than twelve,
or the day is not included in the given month/year.

*/
bool DateTime::IsValid(const int year,const int month,const int day)const {
   long jday = ToJulian(year,month,day);
   int m=0,d=0;
   long y = 0;
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
bool DateTime::ReadFrom(const ListExpr LE,const bool typeincluded){
  canDelete=false;
  ListExpr ValueList;
  if(typeincluded){
     if(nl->ListLength(LE)!=2){
        return false;
     }
     ListExpr TypeList = nl->First(LE);
     if(!nl->IsEqual(TypeList,"datetime")){
       if( (type==instanttype) && !nl->IsEqual(TypeList,"instant")){
            return false;
       }
       if( (type==durationtype) && !nl->IsEqual(TypeList,"duration")){
          return false;
       }
     }
     ValueList = nl->Second(LE);
  }else{
     ValueList=LE;
  }

  // Special Representation in this Algebra
  if(nl->AtomType(ValueList)==SymbolType){
    if(nl->SymbolValue(ValueList)=="now"){
        if(type==instanttype){
           Now();
           return true;
        } else
           return false;
    }
    if(nl->SymbolValue(ValueList)=="today"){
        if(type==instanttype){
           Today();
           return  true;
        } else
           return false;
    }
  }

  // string representation
  if(nl->AtomType(ValueList)==StringType){
     if(type==instanttype)
        return ReadFrom(nl->StringValue(ValueList));
     else
        return false;
  }
  // real representation
  if(nl->AtomType(ValueList)==RealType ){
        bool res = ReadFrom(nl->RealValue(ValueList));
        return res;
  }
  // accect also integer values
  if(nl->AtomType(ValueList)==IntType ){
     if(type==instanttype){
        bool res = ReadFrom(nl->IntValue(ValueList));
        return res;
     }
     else
        return false;
  }


  // (day month year [hour minute [second [millisecond]]])
  if( (nl->ListLength(ValueList)>=3) && nl->ListLength(ValueList)<=7){
     if(type==durationtype)
        return  false;
     int len = nl->ListLength(ValueList);
     if(len==4) return false; // only hours is not allowed
     ListExpr tmp = ValueList;
     while(nl->IsEmpty(tmp)){
        if(nl->AtomType(nl->First(tmp))!=IntType)
           return false;
        tmp = nl->Rest(tmp);
     }
     int d,m,y,h,min,sec,ms;

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
     this->day = ToJulian(y,m,d);
     this->milliseconds = (((h*60)+min)*60 + sec)*1000 +ms;
     defined = true;
     return true;
  }

  // (julianday milliseconds)  // for durations
  if(nl->ListLength(ValueList)!=2){
    return false;
  }
  if(type == instanttype)
     return false;
  ListExpr DayList = nl->First(ValueList);
  ListExpr MSecList = nl->Second(ValueList);
  if(nl->AtomType(DayList)!=IntType || nl->AtomType(MSecList)!=IntType){
     return false;
  }
  day = nl->IntValue(DayList);
  milliseconds = nl->IntValue(MSecList);
  if(milliseconds>MILLISECONDS){
      long dif = milliseconds/MILLISECONDS;
      day += dif;
      milliseconds -= dif*MILLISECONDS;
  }
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
   if(type!=P2->type){
       cerr << "try to compare " << this->ToString() << " with "
            << P2->ToString() << endl;
       assert(type==P2->type);
   }
   if(!defined && !P2->defined)
      return 0;
   if(!defined && P2->defined)
      return -1;
   if(defined && !P2->defined)
      return 1;
   // at this point this and P2 are defined
   if(day<P2->day) return -1;
   if(day>P2->day) return 1;
   if(milliseconds<P2->milliseconds) return -1;
   if(milliseconds>P2->milliseconds) return 1;
   return 0;
}

/*
~Operators for Comparisons~

*/
bool DateTime::operator==(const DateTime T2)const{
  return CompareTo(&T2)==0;
}

bool DateTime::operator<(const DateTime T2)const{
  return CompareTo(&T2)<0;
}

bool DateTime::operator>(const DateTime T2)const{
  return CompareTo(&T2)>0;
}


/*
~Clone~

This funtion returns a copy of this instance.

*/
DateTime* DateTime::Clone() const {
   DateTime* res = new DateTime(type);
   res->Equalize(this);
   return res;
}


/*
~Split~

The function ~Split~ splits a duration into two ones.

*/
bool DateTime::Split(const double delta, DateTime& Rest){
  assert(type==durationtype);
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

This function returns true if this is directly neighbooring with arg.
Because we use a fixed time resolution, we can implement this function.

*/
bool DateTime::Adjacent(const Attribute* arg) const{
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
}

/*
~IsDefined~

~IsDefined~ returns true if this instance contains a
defined value.

*/
bool DateTime::IsDefined() const{
   return defined;
}

/*
~SetDefined~

The function ~SetDefined~ sets the defined flasg of this time
instance to the value of the argument.

*/
void DateTime::SetDefined( bool defined ){
   this->defined = defined;
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
  return (size_t) (int)(day*MILLISECONDS+milliseconds);
}

/*
~CopyFrom~

This Time instance take its value from arg if this function is
called.

*/
void DateTime::CopyFrom(const StandardAttribute* arg){
   Equalize(((const DateTime*) arg));
}

/*
~add~

Adds P2 to this instance. P2 remains unchanged.
The ~Add~ function requires least one argument to be
a duration type.

*/
void DateTime::Add(const DateTime* P2){
   assert(type==durationtype || P2->type==durationtype);
   long d1 = day;
   long d2 = P2->day;
   long ms1 = milliseconds;
   long ms2 = P2->milliseconds;
   // transform negative values
   if(d1<0){
      ms1 -= MILLISECONDS;
      d1++;
   }
   if(d2<0){
      ms2 -= MILLISECONDS;
      d2++;
   }
   long d = d1+d2;
   long ms = ms1+ms2;
   while(ms<0){ // this loop is excuted maximum two times
     d--;
     ms += MILLISECONDS;
   }
   while(ms>=MILLISECONDS){  // this is executed maximum one times
     d++;
     ms -= MILLISECONDS;
   }
   day = d;
   milliseconds = ms;
   if(P2->type==instanttype)
      type = instanttype;
}


/*
~Operator +~

This operator has the same functionality like the ~Add~ function
returning the result in a new instance.

*/
DateTime DateTime::operator+(const DateTime T2)const{
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
   assert(type==instanttype || P2->type==durationtype);
   long d1 = day;
   long d2 = P2->day;
   long ms1 = milliseconds;
   long ms2 = P2->milliseconds;
   // transform negative values
   if(d1<0){
      ms1 -= MILLISECONDS;
      d1++;
   }
   if(d2<0){
      ms2 -= MILLISECONDS;
      d2++;
   }
   long d = d1-d2;
   long ms = ms1-ms2;

   while(ms<0){ // this loop is excuted maximum two times
     d--;
     ms += MILLISECONDS;
   }
   while(ms>=MILLISECONDS){  // this is executed maximum one times
     d++;
     ms -= MILLISECONDS;
   }
   day = d;
   milliseconds = ms;

   if(type==instanttype && P2->type==instanttype)
      type = durationtype;
}

/*
~Operator -~

This operator has the same functionality like the ~Minus~ function
returning the result in a new instance.

*/
DateTime DateTime::operator-(const DateTime T2)const{
   DateTime Result(*this);
   Result.Minus(&T2);
   return Result;
}

/*
~Operator /~

This Operator divides a DateTime by another dateTime

*/
double DateTime::operator/(const DateTime T2)const{
  double myms = (double)(day*MILLISECONDS+milliseconds);
  double T2ms =  (double)(T2.day*MILLISECONDS+T2.milliseconds);
  return myms / T2ms;
}



/*
~mul~

Computes a multiple of a duration.

*/
void DateTime::Mul(const long factor){
   assert(type==durationtype);
   bool of;
   BigInt<2> d(day);
   BigInt<2> ms(milliseconds);
   BigInt<2> MS(MILLISECONDS);
   d.MulInternal(MS,of);
   d.AddInternal(ms,of);
   BigInt<2> f(factor);
   d.MulInternal(f,of);
   BigInt<2> nd(0);
   nd = d.Div(MS,ms);
   day = nd.ToLong(of);
   milliseconds = ms.ToLong(of); 
}

/*
~mul~

Computes a multiple of a duration. This function is not
numerical robust. Use this function only when you know what
you do.

*/
void DateTime::Mul(const double factor){
   assert(type==durationtype);
   double d = day;
   double ms = milliseconds;
   if(day<0.0){
       ms -= MILLISECONDS;
       d +=1.0;
   }
   d = d*factor;
   ms = ms*factor;

   double dms = d - (((double)( (long)(day))));
   d = (double) ((long) day);
   ms = ms + dms*MILLISECONDS;

   while(ms<0){
     ms += MILLISECONDS;
     d = d -1.0;
   }
   while(ms>=MILLISECONDS){
     ms -= MILLISECONDS;
     d = d + 1.0;
   }

   day = (long) d;
   milliseconds=(long) ms;
}

/*
~Div~

This operator computes how often ~dividend~ is contained whithin this
DateTime. This DateTime, the dividend, and ~remainder~ must be of 
type ~duration~.  

*/
 long DateTime::Div(DateTime dividend, DateTime& remainder,bool& overflow){
   // first, create a bigint from this and the divident
   BigInt<2> MyValue(day);
   BigInt<2> MyMillis(milliseconds);
   BigInt<2> DivValue(dividend.day);
   BigInt<2> DivMillis(dividend.milliseconds);
   // create the Bigint Milliseconds
   bool of;
   BigInt<2> BIMILLISECONDS(MILLISECONDS);
   MyValue.MulInternal(BIMILLISECONDS,of);
   DivValue.MulInternal(BIMILLISECONDS,of);
   MyValue.AddInternal(MyMillis,of);
   DivValue.AddInternal(DivMillis,of);
   // create values for result and remainder
   BigInt<2> Result(0), biremainder(0);
   Result = MyValue.Div(DivValue,biremainder);
   long result = Result.ToLong(overflow); 
   // the biremainder must be devided into day and milliseconds 
   // We store the results into DivValue and DivMillis
   DivValue = biremainder.Div(BIMILLISECONDS,DivMillis);
   long l1 = DivMillis.ToLong(of);
   remainder.day = DivValue.ToLong( of);
   remainder.milliseconds = l1;
   if(remainder.milliseconds<0){
      remainder.milliseconds += MILLISECONDS;
      remainder.day--;
   }
   remainder.defined=true;
   return result;
 }




/*
~Operator mul~

This operator has the same functionality like the ~Mul~ function
returning the result in a new instance.

*/
DateTime DateTime::operator*(const long factor)const{
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
     assert(type==instanttype);
     if(day<0){
        day = -1 * day;
        if(milliseconds!=0){
           day--;
           milliseconds = MILLISECONDS-milliseconds;
        }
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
  assert( defined );
  ListExpr value;
  if(type==instanttype){
      if( (day<MIN_REPRESENTABLE || day>MAX_REPRESENTABLE )
         && !IsMinimum() && !IsMaximum()){
          value = nl->RealAtom(ToDouble());
      }else{
          value = nl->StringAtom(this->ToString());
      }
  }

  else // a duration
    value = nl->TwoElemList( nl->IntAtom((int)day),
                             nl->IntAtom((int)milliseconds)
                             );
  if(typeincluded)
     if(type==instanttype)
        return nl->TwoElemList(nl->SymbolAtom("instant"),value);
     else if(type==durationtype)
        return nl->TwoElemList(nl->SymbolAtom("duration"),value);
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
   day = P2->day;
   milliseconds = P2->milliseconds;
   defined = P2->defined;
   type = P2->type;
}

/*
~SetToZero~

A call of this function will set the value of this datetime to be zero,
this means to have length zero or be the NULLDATE respectively.

*/
    void DateTime::SetToZero(){
        day=0; 
        milliseconds = 0;
    }



/*
~IsZero~

~IsZero~ returns true iff this

*/
bool DateTime::IsZero()const {
  return day==0 && milliseconds==0;
}

/*
~LessThanZero~

This function returns true if this instnace is before the Null-Day

*/
bool DateTime::LessThanZero()const{
   return day<0;
}

SmiSize DateTime::SizeOfChars() const
{
  return ToString().length();  
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
2 Algebra Functions

2.1 In and Out functions

*/

ListExpr OutDateTime( ListExpr typeInfo, Word value ){
   DateTime* T = (DateTime*) value.addr;
   if( T->IsDefined() )
     return T->ToListExpr(false);
   else 
     return nl->SymbolAtom("undef");
}


Word InInstant( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct ){

  DateTime* T = new DateTime(instanttype);

  if( nl->IsEqual(instance, "undef") ){
     T->SetDefined( false );
     return SetWord(T);
  } else {
    ListExpr value = instance;
    if(nl->ListLength(instance)==2){
      if(nl->IsEqual(nl->First(instance),"instant"))
        value = nl->Second(instance);
    }
    if(T->ReadFrom(value,false)){
      correct=true;
      return SetWord(T);
    }
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
                nl->StringAtom("instant"),
                nl->StringAtom("string"),
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
                nl->StringAtom("instant"),
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
  T->Destroy();
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
  return (nl->IsEqual(type,"instant"));
}

bool CheckDuration(ListExpr type, ListExpr& errorInfo){
  return (nl->IsEqual(type,"duration"));
}

/*
3 Type Constructor

*/
TypeConstructor instant(
        "instant",                     //name
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
        "duration",                //name
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
     return nl->SymbolAtom("instant");
  ErrorReporter::ReportError("no argument allowed\n");
  return nl->SymbolAtom("typeerror");
}

ListExpr IntBool(ListExpr args){
  if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("one argument expected\n");
     return nl->SymbolAtom("typeerror");
  }
  if(nl->IsEqual(nl->First(args),"int"))
     return nl->SymbolAtom("bool");
  ErrorReporter::ReportError("argument must be of type int\n");
  return nl->SymbolAtom("typeerror");
}

ListExpr InstantInt(ListExpr args){
  if(nl->ListLength(args)==1){
     if(nl->IsEqual(nl->First(args),"instant")){
         return nl->SymbolAtom("int");
     } else {
         ErrorReporter::ReportError("int parapater expected \n");
     }
  }
  ErrorReporter:: ReportError("exactly one argument required");
  return nl->SymbolAtom("typeerror");
}

ListExpr PlusCheck(ListExpr args){
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("plus expects two arguments\n");
     return nl->SymbolAtom("typeerror");
  }
  if(nl->IsEqual(nl->First(args),"instant") &&
     nl->IsEqual(nl->Second(args),"duration"))
     return nl->SymbolAtom("instant");
  if(nl->IsEqual(nl->First(args),"duration") &&
     nl->IsEqual(nl->Second(args),"instant"))
     return nl->SymbolAtom("instant");
  if(nl->IsEqual(nl->First(args),"duration") &&
     nl->IsEqual(nl->Second(args),"duration"))
     return nl->SymbolAtom("duration");
  ErrorReporter::ReportError("duration/instant or"
                             " duration/duration expected\n"); 
  return nl->SymbolAtom("typeerror");
}

ListExpr MinusCheck(ListExpr args){
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("operator - requires two arguments\n");
     return nl->SymbolAtom("typeerror");
  }
  if(nl->IsEqual(nl->First(args),"instant") &&
     nl->IsEqual(nl->Second(args),"duration"))
     return nl->SymbolAtom("instant");
  if(nl->IsEqual(nl->First(args),"instant") &&
     nl->IsEqual(nl->Second(args),"instant"))
     return nl->SymbolAtom("duration");
  if(nl->IsEqual(nl->First(args),"duration") &&
     nl->IsEqual(nl->Second(args),"duration"))
     return nl->SymbolAtom("duration");
  ErrorReporter::ReportError("duration/instant or"
                             " duration/duration expected\n"); 
  return nl->SymbolAtom("typeerror");
}


ListExpr DurationIntDuration(ListExpr args){
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("Two arguments required\n");
     return nl->SymbolAtom("typeerror");
  }
  if(nl->IsEqual(nl->First(args),"duration") &&
     nl->IsEqual(nl->Second(args),"int"))
     return nl->SymbolAtom("duration");
  ErrorReporter::ReportError("duration x int expected\n" );
  return nl->SymbolAtom("typeerror");
}

ListExpr DurationRealDuration(ListExpr args){
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("Two arguments required\n");
     return nl->SymbolAtom("typeerror");
  }
  if(nl->IsEqual(nl->First(args),"duration") &&
     nl->IsEqual(nl->Second(args),"real"))
     return nl->SymbolAtom("duration");
  ErrorReporter::ReportError("duration x real expected\n" );
  return nl->SymbolAtom("typeerror");
}

ListExpr InstantString(ListExpr args){
  if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("one argument expected\n");
     return nl->SymbolAtom("typeerror");
  }
  if(nl->IsEqual(nl->First(args),"instant"))
     return nl->SymbolAtom("string");
  ErrorReporter::ReportError("string expected\n");
  return nl->SymbolAtom("typeerror");
}


ListExpr CheckComparisons(ListExpr args){
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("two arguments required\n");
     return nl->SymbolAtom("typeerror");
  }

  if(nl->IsEqual(nl->First(args),"instant") &&
     nl->IsEqual(nl->Second(args),"instant"))
       return nl->SymbolAtom("bool");

  if(nl->IsEqual(nl->First(args),"duration") &&
     nl->IsEqual(nl->Second(args),"duration"))
       return nl->SymbolAtom("bool");

  ErrorReporter::ReportError("(instant x instant) or"
                             "(duratin x duration) required");
  return nl->SymbolAtom("typeerror");
}

ListExpr TheInstantTM(ListExpr args){
   int l = nl->ListLength(args);
   if(l<1 || l>7){
      ErrorReporter::ReportError(" 1..7 arguements required\n");
      return nl->SymbolAtom("typeerror");
   }
   ListExpr rest = args;
   while(!nl->IsEmpty(rest)){
       if(!nl->IsEqual(nl->First(rest),"int")){
           ErrorReporter::ReportError("All arguments must be of type int\n");
           return nl->SymbolAtom("typeerror");
       }
       rest = nl->Rest(rest);
   }
   return nl->SymbolAtom("instant");
}

ListExpr DivTM(ListExpr args){
   if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("Two arguments required\n");
      return nl->SymbolAtom("typeerror");
   }
   if(!nl->IsEqual(nl->First(args),"duration") ||
      !nl->IsEqual(nl->Second(args),"duration")){
      ErrorReporter::ReportError("two duration values expected\n");
      return nl->SymbolAtom("typeerror");
   }
   return nl->SymbolAtom("int");
}

ListExpr MinMaxInstantTM(ListExpr args){
   if(nl->IsEmpty(args)){
       return nl->SymbolAtom("instant");
   }
   ErrorReporter::ReportError("no arguments allowed");
   return nl->SymbolAtom("typeerror");
}

ListExpr MinMaxDurationTM(ListExpr args){
   if(nl->IsEmpty(args)){
       return nl->SymbolAtom("duration");
   }
   ErrorReporter::ReportError("no arguments allowed");
   return nl->SymbolAtom("typeerror");
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
  bool overflow=false;
  long res = T1->Div( (*T2),Remainder,overflow);
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
    STRING* WD;
    switch(day){
       case 0 :  WD = (STRING*) "Monday";
                 break;
       case 1 : WD =  (STRING*) "Tuesday";
                 break;
       case 2 : WD =  (STRING*) "Wednesday";
                break;
       case 3 : WD = (STRING*) "Thursday";
                break;
       case 4 : WD = (STRING*) "Friday";
                break;
       case 5 : WD = (STRING*) "Saturday";
                break;
       case 6 :  WD = (STRING*) "Sunday";
                 break;

       default : WD = (STRING*)"Errorsday";
                 break;
    }
    ((CcString*)result.addr)->Set(true,WD);
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
   " ( \"instant -> int\""
   " \" day_of ( _ ) \" "
   "   \"return the day of this instant\" "
   "   \" query day_of(T1) \" ))";

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
   " ( \"instant -> int\""
   " \" millisecond_of(_) \" "
   "   \"return the millisecond of this instant\" "
   "   \" query millisecond_of(T1) \" ))";

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
   "   \" query weekday_of(today())\" ))";

const string DivSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"duration x duration -> int\""
   " \"  _ / _  \" "
   "   \"Computes how often the second argument is part of the first one\" "
   "   \" query a / b \" ))";


const string MinInstantSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \" -> instant\""
   " \"  minInstant()  \" "
   "   \"returns the minimum possible instant \" "
   "   \" query minInstant() \" ))";


const string MaxInstantSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \" -> instant\""
   " \"  maxInstant()  \" "
   "   \"returns the maximum possible instant \" "
   "   \" query maxInstant() \" ))";

const string MinDurationSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \" -> duration \""
   " \"  minDuration()  \" "
   "   \"returns the minimum representable duration value \" "
   "   \" query minDuration() \" ))";

const string MaxDurationSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \" -> duration \""
   " \"  maxDuration()  \" "
   "   \"returns the maximum representable duration value \" "
   "   \" query maxDuration() \" ))";

/*
4.3 ValueMappings of overloaded Operators

*/

ValueMapping TheInstantValueMap[] = {
        TheInstantFun_Int1,TheInstantFun_Int2,TheInstantFun_Int3,
        TheInstantFun_Int4,TheInstantFun_Int5,TheInstantFun_Int6,
        TheInstantFun_Int7 };

/*
4.4 SelectionFunctions

*/

static int TheInstantSelect(ListExpr args){
  return nl->ListLength(args)-1;
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
       "day_of", // name
       DaySpec, // specification
       DayFun,
       Operator::SimpleSelect,
       InstantInt);

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
       "millisecond_of", // name
       MillisecondSpec, // specification
       MillisecondFun,
       Operator::SimpleSelect,
       InstantInt);

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
       "theInstant",                        // name
       TheInstantSpec,                 // specification
       7,                          // number of functions
       TheInstantValueMap,
       TheInstantSelect,
       TheInstantTM);


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
    instant.AssociateKind("DATA");
    instant.AssociateKind("INDEXABLE");
    AddTypeConstructor( &duration );
    duration.AssociateKind("DATA");

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

  }
  ~DateTimeAlgebra() {};
};

DateTimeAlgebra dateTimeAlgebra;

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
  return (&dateTimeAlgebra);
}

} // end of namespace

