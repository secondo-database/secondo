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
//[TOC] [\tableofcontents]

[1] LInterval.h

April 2015 - Rene Steinbrueck

1 Overview

[TOC]

2 Defines, includes, and constants

*/
#ifndef LINTERVAL_H
#define LINTERVAL_H

#include "./../../include/StandardTypes.h"

/*
3 C++ Classes (Defintion)

3.1 LInterval

The class ~LInterval~ implements the closure of an length interval.An interval
contains a ~start~, an ~end~ and two flags ~lc~ and ~rc~ indicating if the interval is
left-closed and right-closed (or left-right-closed), respectively.

*/
class LInterval
{

public:
/*
3.1.1 Constructors

3.1.1.1 Standard constructor

*/
    LInterval() {}
/*
The simple constructor. This constructor should not be used.

3.1.1.2 Default constructor

*/
    explicit LInterval(bool defined):
    start(defined), end(defined), lc(true), rc(true){}
/*
This constructor initializes the LInterval with zero. If the passed
bool flag is true start and end are defined, otherwise not. The produced
linterval is left and right closed.

3.1.1.3 Copy constructor

*/
    LInterval( const LInterval& linterval );
/*
This constructor intializes the LInterval with the passed LInterval.

3.1.1.4 Setting constructor

*/

    LInterval( const CcReal& start,
              const CcReal& end,
              const bool lc,
              const bool rc );
/*
The creation of the interval setting all attributes.

3.1.2 Member functions

3.1.2.1 getLengthInterval

*/
    const LInterval& getLengthInterval() const
    {
        return  *this;
    }
/*
3.1.2.2 IsValid

*/
    bool IsValid() const;
/*
Checks if the linterval is valid or not. This function should be used for debugging purposes
only.  An linterval is valid if the following conditions are true:

  1 ~start~ and ~end~ are defined

  2 ~start~ $<=$ ~end~

  3 if ~start~ $==$ ~end~, then must ~lc~ $=$ ~rc~ $=$ ~true~

3.1.2.3 Disjoint

*/

    bool R_Disjoint( const LInterval& i ) const;
/*
Returns ~true~ if this linterval is r-disjoint with the linterval ~i~ and ~false~ otherwise.

*/

    bool Disjoint( const LInterval& i ) const;
/*
Returns ~true~ if this linterval is disjoint with the linterval ~i~ and ~false~ otherwise.

3.1.2.4 Before

*/

    bool Before( const LInterval& i ) const;
/*
Returns ~true~ if this linterval is before the linterval ~i~ and ~false~ otherwise.

*/

    bool Before( const CcReal& m ) const;
/*
Returns ~true~ if this linterval is before the value ~a~ and ~false~ otherwise.

3.1.2.5 After

*/

    bool After( const LInterval& i ) const;
/*
Returns ~true~ if this linterval is after the linterval ~i~ and ~false~ otherwise.

*/

    bool After( const CcReal& m ) const;
/*
Returns ~true~ if this linterval is after the linterval ~i~ and ~false~ otherwise.

3.1.2.6 CompareTo

*/
    int CompareTo(const LInterval& i) const;
/*
Compares this and the argument

3.1.2.7 Contains

*/

    bool Contains( const CcReal& a,
                   const bool ignoreCloseness = false ) const;
/*
Returns ~true~ if this linterval contains the value ~a~ and ~false~ otherwise.
If ignoreCloseness is set to be true, the return value will also be true, if
a is one of the borders of this interval regardless of the closeness at this
point.

*Precondition:* ~a.IsDefined()~

*/
    bool Contains(const LInterval& i,
                  const bool ignoreCloseness = false) const;
/*
Returns ~true~ if this linterval contains the linterval ~i~ and ~false~ otherwise.
If ignoreCloseness is set to be true, the return value will also be true, if
start or end point of i are one of the borders of this interval regardless
of the closeness at this point.

3.1.3 Operator Redifinitions

3.1.3.1 Operation ~copy~

*/

    LInterval& operator=( const LInterval& i );
/*
Redefinition of the copy operator ~=~.

3.1.3.2 Operation $=$ (~equal~)

*/

    bool operator==( const LInterval& i ) const;
/*
Returns ~true~ if this linterval is equal to the linterval ~i~ and ~false~ if they are different.

3.3.3.3 Operation $\neq$ (~not equal~)


*/

    bool operator!=( const LInterval& i ) const;
/*
Returns ~true~ if this linterval is different to the linterval ~i~ and ~false~ if they are equal.

3.3.3.4 Operation $<$ (~smaller~)

*/

    bool operator<(const LInterval& i) const;
/*
Returns ~true~ if this linterval is smaller than the linterval ~i~ and ~false~ otherwise.

3.3.3.5 Operation $>$ (~greater~)

*/

    bool operator>(const LInterval& i) const;
/*
Returns ~true~ if this linterval is greater than the linterval ~i~ and ~false~ otherwise.

3.1.4 Functions to be part of relations

3.1.4.1 Adjacent

*/

    bool R_Adjacent( const LInterval& i ) const;
/*
Returns ~true~ if this linterval is r-adjacent with the linterval ~i~ and ~false~ otherwise.

*/

    bool Adjacent( const LInterval& i ) const;
/*
Returns ~true~ if this linterval is adjacent with the linterval ~i~ and ~false~ otherwise.

3.1.4.2 CopyFrom

*/

    void CopyFrom( const LInterval& interval );

/*
3.1.4.3 Print

*/

    std::ostream& Print(std::ostream& os) const
    {
        os << (lc?"[":"(");
        start.Print(os) << ", ";
        end.Print(os) << (rc?"]":")");
        return os;
    }

/*

3.1.5 Attributes

*/
  CcReal start;
/*
The starting value of the interval.

*/

  CcReal end;
/*
The ending value of the interval.

*/

  bool lc;
/*
A flag indicating that the interval is left-closed.

*/

  bool rc;
/*
A flag indicating that the interval is right-closed.

*/

};

#endif //LINTERVAL_H
