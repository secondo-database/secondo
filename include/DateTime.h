/*

1 The Class DateTime

A DateTime value is represented by the tuple {\tt (day,milliseconds)}.
In this tuple {\tt day} represents the difference of the day to
the time reference point (see \ref{TRP}). The second part {\tt milliseconds}
represents the time point in this day. For this reason, this part
can be in the interval $[0,86399999]$.

The DateTime class can be used to describe both, instants and
differences between them.

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
The value 2451547 corresponds to 3.1.2004

*/
#define NULL_DAY  2451547


/*
1.2 The class declaration

*/

class DateTime : public StandardAttribute  {
  public:

/*
1.2.1 The empty Constructor

The empty constructor is the special constructor for the
cast function.

*/
     DateTime();

/*
1.2.2 Constructor


The next constructor creates a datetime instant at
the nullday. The argument is ignored. The purpose of
the parameter is only to make this constructor distinct
from the empty one.

*/
     DateTime(int dummy);

/*
1.2.3 Constructor

This constructor creates a DateTime instance at the specified
day and milliseconds

*/
     DateTime(const long Day,const long MilliSeconds);

/*
1.2.4 Destructor

*/
     ~DateTime();

/*
1.2.5 GetDay

This function returns the julian day of this DateTime instance.

*/
     long GetDay();

/*
1.2.6 GetAllMilliseconds

The function ~GetMilliseconds~ returns the time part of the day.
The result will be in interval [0,86400000[.

*/
     long GetAllMilliSeconds();


/*
1.2.7 Access Functions

The next functions extract information of this DateTime.
In the comment the value range of the result is given.

*/
     int GetGregDay();        // [1..31]
     int GetMonth();          // [1..12]
     int GetYear();           //  int / {0}
     int GetHour();           // [0..23]
     int GetMinute();         // [0..59]
     int GetSecond();        // [0..59]
     int GetMillisecond();   // [0..999]


/*
1.2.9 Destroy

The familiar ~Destroy~ function included in the most algebras.

*/
     void Destroy();

/*
1.2.10 Open

This function overwrites the inherited ~Open~ function for the reason
of resistency against recompiling.

*/
     virtual void Open(SmiRecord& valueRecord, const ListExpr typeInfo);

/*
1.2.11

The ~Save~ function is the conterpart of the ~Open~ function.

*/
     virtual void Save(SmiRecord& valueRecord, const ListExpr typeInfo);

/*
1.2.12 Conversion from and into other formats

1.12.1 ToListExpr

This function converts this DateTime instance into its
nested list representation. The format depends on the values
of the parameters.

\begin{tabular}{|l|l|l|}\hline
                 & \multicolumn{2}{c}{absolute} \\
    typeincluded &    true                                & false \\\hline
         true    & (datetime "yyyy-mm-dd-hh:min:sec.ms")  & (datetime (day millisecs)) \\\hline
	 false   & "yyyy-mm-dd-hh:min:sec.ms"  & (day millisecs) \\\hline
\end{tabular}

*/
     ListExpr ToListExpr(bool absolute,bool typeincluded);

/*
1.12.2 ReadFrom

This function reads the value of this DateTime instance from the argument {\tt LE}.
The format can be absolute or relative. If {\tt LE} does not represent a valid
DateTime representation the return value will be {\tt false} and the value of this
istance remains unchanged.

*/
     bool ReadFrom(ListExpr LE,bool typeincluded);


/*
1.2.8 ToString

The function ~ToString~ returns the string representation of this
DateTime instance. The format depends on the value of the argument
{\tt absolute}. If {\tt absolute} is {\tt true}, the format is \\
year-month-day-hour:minute:second.millisecond.\\
In the other case the format will be:\\
day||millisecond.\\
The first format describes a time instant, the second one describes
a time difference.

*/
     string ToString(bool absolute);




/*
1.12.3 ReadFrom

This function reads the value of this DateTime from the argument.
The format is:\\
year-month-day[-hour:minute[:second[.millisecond.]]\\
where the squared brackets indicate optionally parts.

*/
     bool ReadFrom(const string Time);


/*
1.12.3 ReadFrom

This functions read the value of this DateTime from the given double
value. The return value is allways true.

*/
    bool ReadFrom(const double Time);


/*
1.2.7 ToDouble

The ~GetDouble~ function returns this DateTime Instance as an
double value.

*/
     double ToDouble();


/*
1.2.13 CompareTo

This function compares this instance with the
value of the argument.

*/
     int CompareTo(DateTime* P2);

/*
1.2.14 Mathematical functions

The following functions are the usual mathematical
operations and comparisons.

*/

     void Add(DateTime* P2);
     void Minus(DateTime* P2);
     void Mul(int factor);
     bool IsZero();
     bool LessThanZero();

/*
1.2.15 Equalize

When this fucntion is called this DateTime instance takes its value
from the argument.

*/

     void Equalize(DateTime* P2);


/*
1.2.16 IsValid

This fucntions checks if the combination of the argument values
build a valid date.

*/
     bool IsValid(int year, int month, int day);

/*
1.2.17 Now

This functions reads out the currenty system time an adopt the
value of tho DateTime to it.

*/
     void Now();


/*
1.2.17

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
     void WriteToSmiRecord(SmiRecord& valueRecord, int& offset);
     void ReadFromSmiRecord(SmiRecord& valueRecord, int& offset);

  private:
    // the data-part of datetime
    long day;
    long milliseconds;
    bool defined;
    bool canDelete;
    // a few functions for internal use
    long ToJulian(int year, int month, int day);
    void ToGregorian(long Julian, int &year, int &month, int &day);
};
