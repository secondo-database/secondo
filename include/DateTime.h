/*
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
#endif

#include <string>
#include "NestedList.h"
#include "StandardAttribute.h"

  // milliseconds of a single day
#define MILLISECONDS 86400000


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
1.2 The class declaration

*/

class DateTime : public StandardAttribute  {
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
     DateTime(const long Day,const long MilliSeconds,TimeType type);

/*
~Destructor~

*/
     ~DateTime();

/*
~GetDay~

This function returns the day part of a duration.
~GetDay~ is only defined for duration types.

*/
     long GetDay();

/*
~GetAllMilliseconds~

The function ~GetMilliseconds~ returns the time part of the day.
The result will be in interval [ 0, 86400000 ].

*/
     long GetAllMilliSeconds();

/*
~Access Functions~

The next functions extract information of an instant.
In the comment the value range of the result is given.
All this functions can only applied to instant types.

*/
     int GetGregDay();        // [1..31]
     int GetMonth();          // [1..12]
     int GetYear();           //  int / {0}
     int GetHour();           // [0..23]
     int GetMinute();         // [0..59]
     int GetSecond();        // [0..59]
     int GetMillisecond();   // [0..999]
     int GetWeekday();       // [0..6] <=> [Monday .. Sunday]

/*
~Destroy~

The familiar ~Destroy~ function included in the most algebras.

*/
     void Destroy();

/*
~Open~

This function overwrites the inherited ~Open~ function for the reason
of resistency against recompiling.

*/
     virtual void Open(SmiRecord& valueRecord, const ListExpr typeInfo);

/*
~Save~

The ~Save~ function is the counterpart of the ~Open~ function.

*/
     virtual void Save(SmiRecord& valueRecord, const ListExpr typeInfo);

/*
~Conversion from and into other formats~

~ToListExpr~

This function converts a DateTime into its nested list representation.
The output depends on the type of this DateTime as well as the argument.

*/
     ListExpr ToListExpr(bool typeincluded);

/*
~ReadFrom~

This function reads the value of this DateTime instance from the argument {\tt LE}.
The possible formats are described in {\tt STFormat}, which can be found in
the {\tt document} directory of {\textsc{SECONDO}}.

*/
     bool ReadFrom(ListExpr LE, bool typeincluded);

/*
~ToString~

The function ~ToString~ returns the string representation of this
DateTime instance. The format depends on the type of time.
For an instant type, the format is \\
year-month-day-hour:minute:second.millisecond.\\
In the other case the format will be:\\
day$\mid\mid$millisecond.

*/
     string ToString();

/*
~SetType~

Sets the type of this DateTime. Be careful in using of this function.

*/
      void SetType(TimeType TT);

/*
~GetType~

This function returns the type of this DateTime;

*/
      TimeType GetType();


/*
~ReadFrom~

This function reads the value of this DateTime from the argument.
This function is only defined for instants.

*/
     bool ReadFrom(const string Time);

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
     double ToDouble();

/*
~CompareTo~

This function compares this instance with the
value of the argument. The types of this and P2 have to
be equal.

*/
     int CompareTo(DateTime* P2);

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

     void Add(DateTime* P2);

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
     void Minus(DateTime* P2);

/*
~Mul~

For the ~Mul~ function  the this object must be of type {\tt duration}.

*/
     void Mul(int factor);

/*
The next two function are defined for any DateTime type.

*/
     bool IsZero();
     bool LessThanZero();

/*
~Equalize~

When this function is called this DateTime instance takes its value
from the argument.

*/
     void Equalize(DateTime* P2);
/*
~IsValid~

This functions checks if the combination of the argument values
build a valid date.

*/
     bool IsValid(int year, int month, int day);

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
~Algebra Functions~

The next functions are needed for the DateTime class to act as
an attribute of a relation.

*/
     int Compare(Attribute* arg);
     bool Adjacent(Attribute*);
     int Sizeof();
     bool IsDefined() const;
     void SetDefined( bool defined );
     size_t HashValue();
     void CopyFrom(StandardAttribute* arg);
     DateTime* Clone();

/*
~WriteToSmiRecord~

The following function writes the content of this DateTime into
the SmiRecord {\tt valueRecord} beginning at position {\tt offset}.
After calling this function, {\tt offset} will holds the position
behind the written data.

*/
     void WriteToSmiRecord(SmiRecord& valueRecord, int& offset);
/*
~ReadFromSmiRecord~

This function reads the value of this DateTime instance. The offset
is adjusted.

*/
     void ReadFromSmiRecord(SmiRecord& valueRecord, int& offset);

  private:
    // the data-part of datetime
    long day;
    long milliseconds;
    bool defined;
    bool canDelete;
    TimeType type; // can be an instant or a duration
    // a few functions for internal use
    long ToJulian(int year, int month, int day);
    void ToGregorian(long Julian, int &year, int &month, int &day);
};

