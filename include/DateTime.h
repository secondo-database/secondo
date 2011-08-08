
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

----

\sloppy

1 The Header File of DateTime

1.1 Remarks

This class can be used to represent time. They are two kinds of time,
durations and instants. The class ~DateTime~ can handle each of
them. This is possible because both time types can be described
by the same data-structure. To make the time types distinguishable,
the type of time is explicitely included in this class.
Several operations are not defined for all combinations of
datatypes. E.g. the ~Minus~ operator is not defined for the
combination(duration $\times$ instant).The applicability is
checked by calling ~assert~. Note that a few of the operators
change the type of the calling instance. E.g. calling the ~Minus~
function on an {\tt instant} with an argument of {\tt instant}
will change the type of the this object to a {\tt duration}.

1.1 Includes and Constants

*/

#ifndef __DATE_TIME_H__
#define __DATE_TIME_H__


#include <string>
#include <ostream>
#include "NestedList.h"
#include "Attribute.h"
#include "IndexableAttribute.h"
#include "StandardTypes.h"
#include "BigInt.h"

namespace datetime {

  // milliseconds of a single day
#define MILLISECONDS 86400000L


/*
~The Time Reference Point ~

This number represents the time reference point of this Time class
as julian day. The value must be a multiple of 7.
The value 2451547 corresponds to 3.1.2000

*/
#define NULL_DAY  2451547

/*
~The Types of Time~

*/
enum TimeType  {instanttype, durationtype};

/*
~some constants for a compact represnetation of flags~

*/
static const unsigned char INSTANT_POS = 2;
static const unsigned char DURATION_POS = 4;



/*
1.2 The class declaration

*/

class DateTime : public IndexableAttribute  {
  public:

/*
~Constructor~

The empty constructor is the special constructor for the
cast function.

*/
     DateTime();

/*
~Constructor~

The next constructor creates a datetime instant at
the nullday and duration 0 respectively. The created type of
this DateTime is fixed by the argument.

*/
     DateTime(TimeType type);

/*
~Constructor~

This constructor creates a DateTime instance at the specified
day, milliseconds and TimeType.

*/
     DateTime(const int Day,
              const int MilliSeconds,
              const TimeType type);

     DateTime( const TimeType t, const uint64_t v);


/*
~Constructor~

This Constructor creates a DateTime taking its values from the
argument.

*/

     DateTime(const DateTime& DT);

/*
~Constructor~

Creates an instant from a given value

*/
     DateTime(const int64_t v);



/*
~Constructor~

This Constructor creates an instant from a double value.

*/
     DateTime(const double d);

/*
~Assignment operator~

*/
    DateTime& operator=(const DateTime& DT);


/*
~Destructor~

*/
     ~DateTime();



/*
~Set~

The Set-Function defined for instants sets the value to the
specified arguments. Non consistent values, e.g. the 30.2.2004
are converted into valid values.

*/
   void Set(const int year,const int month=1, const int day=1,
            const int hour=0, const int minute=0, const int second=0,
            const int millisecond=0);


/*
~GetDay~

This function returns the day part of a duration.

*/
     int64_t GetDay()const;

/*
~GetAllMilliseconds~

The function ~GetMilliseconds~ returns the time part of the day.
The result will be in interval [ 0, 86400000 ].

*/
     int32_t GetAllMilliSeconds()const;

/*
~Access Functions~

The next functions extract information of an instant.
In the comment the value range of the result is given.
All this functions can only applied to instant types.

*/
     int32_t GetGregDay()const;        // [1..31]
     int32_t GetMonth()const;          // [1..12]
     int32_t GetYear()const;           //  int / {0}
     int32_t GetHour()const;           // [0..23]
     int32_t GetMinute()const;         // [0..59]
     int32_t GetSecond()const;        // [0..59]
     int32_t GetMillisecond()const;   // [0..999]
     int32_t GetWeekday()const;       // [0..6] <=> [Monday .. Sunday]


/*
~Conversion from and into other formats~

~ToListExpr~

This function converts a DateTime into its nested list representation.
The output depends on the type of this DateTime as well as the argument.

*/
     ListExpr ToListExpr(const bool typeincluded)const;

/*
~ReadFrom~

This function reads the value of this DateTime instance from the argument {\tt LE}.
The possible formats are described in {\tt STFormat}, which can be found in
the {\tt document} directory of {\textsc{SECONDO}}.

*/
     bool ReadFrom(const ListExpr LE, const bool typeincluded);

/*
~ToString~

The function ~ToString~ returns the string representation of this
DateTime instance. The format depends on the type of time.
For an instant type, the format is \\
year-month-day-hour:minute:second.millisecond.\\
If ~sql92conform~ is ~true~, a blanc character is used instead of the minus to
separate date and hour, so the format is
year-month-day hour:minute:second.millisecond.\\

In the other case (durationtype) the format will be:\\
day$\mid\mid$millisecond.



*/
    string ToString(const bool sql92conform = false) const;

/*
~SetType~

Sets the type of this DateTime. Be careful in using of this function.

*/
     inline void SetType(const TimeType TT){
         type = TT;
     }



/*
~ReadFrom~

This function reads the value of this DateTime from the argument.
This function is only defined for instants.

*/
     bool ReadFrom(const string Time);

/*
~ReadFromString~

This function just calls ~ReadFrom~(__string__). Its added for
the CSV import.

*/
   virtual void ReadFromString(string time){
        if(!ReadFrom(time)){
          SetDefined(false);
        }
    }

/*
~ReadFrom~

This functions read the value of this DateTime from the given double
value. The return value is allways true.

*/
    bool ReadFrom(const double Time);

/*
~ToDouble~

The ~GetDouble~ function returns this DateTime instance as an
double value.

*/
    double ToDouble() const;


/*
~CompareTo~

This function compares this instance with the
value of the argument. The types of this and P2 have to
be equal.

*/
     int CompareTo(const DateTime* P2)const;

/*
~Mathematical Functions~

The following functions are the usual mathematical
operations and comparisons. These functions are only
defined for meaningful arguments.

~Add~

Add can be used for following signatures:\\
\begin{tabular}{l@{$\times$}l@{$\rightarrow$}l}
    {\bf this} & {\bf argument} & {\bf this after calling} \\
    instant  & duration & instant \\
    duration & instant & instant \\
    duration & duration & duration
\end{tabular}

*/

     void Add(const DateTime* P2);

/*
~Minus~

The allowed signatures for Minus are:\\
\begin{tabular}{l@{$\times$}l@{$\rightarrow$}l}
    {\bf this} & {\bf argument} & {\bf this after calling} \\
    instant  & duration & instant \\
    instant & instant & duration \\
    duration & duration & duration
\end{tabular}

*/
     void Minus(const DateTime* P2);

/*
~Mul~

For the ~Mul~ function  the this object must be of type {\tt duration}.

*/
     void Mul(const int32_t factor);

/*
~Mul~

For the ~Mul~ function  the this object must be of type {\tt duration}.
Note that the product of a duration with a double value is not robust
against numeric inaccuracies. This means that $a*d + (1-a)*d == d$
may be false. (a is a double-constant and d a duration value).

*/
     void Mul(const double factor);

/*
~Div~

By calling this function, one can compute how often
a duration is contained within another duration.
The result will be of type integer. The remainder is returned
in the corresponding argument - also as duration type -.

*/
    int64_t Div(const DateTime& dividend, DateTime& remainder);


/*
~Operator +~

The Operator ~+~ provides the same functionality like the ~Add~ function.
The difference is that the ~Add~ function changes the calling instance
while the ~+~ operator creates a new one. The allowed timetypes are the
same like in the ~Add~ function.

*/
    DateTime operator+(const DateTime& T2)const;

/*
~Operator +=~

This operator adds a value to this DateTime.

*/
 DateTime operator+=(const DateTime& T2);


/*
~Operator -=~

This operator subtracts a value from this DateTime.

*/
 DateTime operator-=(const DateTime& T2);



/*
~Operator -~

This operator computes the difference between two DateTime instances.
The allowed arguments are the same like in the ~Minus~ function.

*/
    DateTime operator-(const DateTime& T2)const;

/*
~Operator /~

This Operator divides a DateTime by another dateTime

*/
    double operator/(const DateTime& T2)const;

/*
~Operator /~

This Operator divides a DateTime (durationtype) by a LONGTYPE

*/
    DateTime operator/(const int32_t divisor)const;

/*
~Operator multiply~

This Operator multiplies a DateTime by a int and double number

*/
    DateTime operator*(const int32_t factor)const;
    DateTime operator*(const double factor)const;

/*
~Operators for Comparisions~

*/
    bool operator==(const DateTime& T2)const;
    bool operator!=(const DateTime& T2)const;
    bool operator<(const DateTime& T2)const;
    bool operator>(const DateTime& T2)const;
    bool operator<=(const DateTime& T2)const;
    bool operator>=(const DateTime& T2)const;


/*
~Abs~

This Funtion computes the absolute value of a Duration. ~Abs~ is not
allowed for an instant type.

*/
   void Abs();

/*
The next two function are defined for any DateTime type.

*/
     bool IsZero()const;
     bool LessThanZero()const;


/*
~SetToZero~

This function changes the value of this DateTime to have
length zero and be the NULLDATE respectively.

*/

     void SetToZero();


/*
~Equalize~

When this function is called this DateTime instance takes its value
from the argument.

*/
     void Equalize(const DateTime* P2);
/*
~IsValid~

This functions checks if the combination of the argument values
build a valid date.

*/
     bool IsValid(const int32_t year,
                  const int32_t month,
                  const int32_t day)const;

/*
~Now~

This functions reads out the current system time an adopt the
value of the DateTime to it. This function is only defined for
an {\tt instant} type.

*/
     void Now();

/*
~Today~

This function read this dateTime value from the System
ignoring the time part. (for {\tt instant}s only)

*/
     void Today();

/*
~ToMinimum~

This function sets this instant to the mimimum possible instant.

*/
    void ToMinimum();


/*
~ToMaximum~

If this function is called, the value of this datetime instance will
set to the maximum possible value.

*/
    void ToMaximum();


/*
~IsMinimum~

This function checks whether this dateTime value is the mimimum
representable one. If it is not defined, the result is __false__.

*/
    bool IsMinimum() const;


/*
~IsMaximum~

This function checks whether this dateTime value is the maximum
representable one. If it is not defined, the result is __false__.

*/
    bool IsMaximum() const;

/*
~Algebra Functions~

The next functions are needed for the DateTime class to act as
an attribute of a relation.

*/
     int Compare(const Attribute* arg) const;
     bool Adjacent(const Attribute *arg) const;
     size_t Sizeof() const;
     size_t HashValue() const;
     void CopyFrom(const Attribute* arg);
     DateTime* Clone() const;
     void WriteTo( char *dest ) const;
     void ReadFrom( const char *src );
     SmiSize SizeOfChars() const;

     ostream& Print(ostream &os) const;

/*
~WriteToSmiRecord~

The following function writes the content of this DateTime into
the SmiRecord {\tt valueRecord} beginning at position {\tt offset}.
After calling this function, {\tt offset} will holds the position
behind the written data.

*/
     void WriteToSmiRecord(SmiRecord& valueRecord, size_t& offset)const;
/*
~ReadFromSmiRecord~

This function reads the value of this DateTime instance. The offset
is adjusted.

*/
     void ReadFromSmiRecord(SmiRecord& valueRecord, size_t& offset);


/*
~Split~

This functions splits a duration into two ones at the specified position
delta in [0,1].

*/
  bool Split(const double delta, DateTime& Rest);


  virtual string getCsvStr() const {
     return ToString(true); // create sql92 compatible output for instant
  }

  virtual bool hasDB3Representation() const {return true;}
  virtual unsigned char getDB3Type() const { return 'C'; }
  virtual unsigned char getDB3Length() const { return 30; }
  virtual unsigned char getDB3DecimalCount(){ return 0; }
  virtual string getDB3String() const { return ToString(); }

  static const string BasicType(){
    return "instant";
  }

  static const bool checkType(const ListExpr list){
    return listutils::isSymbol(list, BasicType());
  } 

  inline virtual StorageType GetStorageType() const { return Core; }

  inline virtual size_t SerializedSize() const
  {
    return sizeof(int64_t) + sizeof(TimeType) + 1;
  }

  inline virtual void Serialize(char* storage, size_t sz, size_t offset) const
  {
    WriteVar<int64_t>(value, storage, offset);
    WriteVar<bool>(del.isDefined,storage,offset);
    WriteVar<TimeType>(type, storage, offset);
  }

  inline virtual void Rebuild(char* state,  size_t sz )
  {
    size_t offset = 0;
    ReadVar<int64_t>(value, state, offset);
    ReadVar<bool>(del.isDefined, state, offset);
    ReadVar<TimeType>(type, state, offset);
  }

/*
~GetType~

This function returns the type of this DateTime;

*/
  inline TimeType GetType() const{
    return type;
  }


/*
~millisecsToNull~

For an duration value, this function returns the duration in milliseconds. 
For in instant, this function returns the distance to the NULL-Day in 
milliseconds.

*/

 int64_t millisecondsToNull() const{
    return value;
 }




  private:
    // the data-part of datetime
    TimeType type;
    int64_t value;
    // a few functions for internal use
    int32_t ToJulian(const int year, const int month,const int day) const;
    void ToGregorian(const int32_t Julian, int32_t  &year,
                     int32_t &month, int32_t &day) const;

    void ToGregorian(int32_t  &year, int32_t &month, int32_t &day) const;
};

Word InInstant( const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr OutDateTime( ListExpr typeInfo, Word value );

}

/*
~ Stream operator~

*/

ostream& operator<<(ostream& o, const datetime::DateTime& DT);


/*
Type name for Duration type in Secondo

*/
class Duration{
  public:
    static const string BasicType() {return "duration"; }
    static const bool checkType(const ListExpr list){
       return listutils::isSymbol(list, BasicType());
    }

  private:
    Duration(){} // do not allow to create an instance of this class
};

/*
A frequently used alias name for "DateTime" is "Instant".

*/
typedef datetime::DateTime Instant;

#endif
