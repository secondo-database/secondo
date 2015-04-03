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

{\Large \bf \begin{center}LInterval.h\end{center}}

\tableofcontents
\newpage

1 Defines and includes

*/

#ifndef LINTERVAL_H
#define LINTERVAL_H

#include "../../include/StandardTypes.h"

/*
2 LInterval

The class ~LInterval~ implements the closure of an length-interval.
An linterval contains a ~start~, an ~end~ and two flags ~lc~ and ~rc~ indicating 
if the interval is left-closed and right-closed (or left-right-closed), 
respectively.

*/
class LInterval {
public:

/*
2.1 Constructors

2.1.1 Default constructor

*/

    explicit LInterval(const bool boolvalue = true);
/*
This constructor initializes the linterval with zero. 
If the passed boolvalue is false, start and end are undefined. 

2.1.2 Copy Constructor

*/
    LInterval(const LInterval& linterval);

/*
This constructor initializes the values with the values
of the passed linterval

2.1.3 Setting Constructor

*/

    LInterval(const CcReal& start, const CcReal& end,
            const bool lc, const bool rc);

/*
2.2 Member functions

2.2.1 IsValid

*/
    bool IsValid() const;
/*
Checks if the interval is valid or not. 
An interval is valid if the following conditions are true:

  1 ~start~ and ~end~ are defined

  2 ~start~ $<=$ ~end~

  3 if ~start~ $==$ ~end~, then must ~lc~ $=$ ~rc~ $=$ ~true~

2.2.2 Disjoint

*/

    bool R_Disjoint( const LInterval& i ) const;
/*
Returns ~true~ if this interval is r-disjoint with the interval ~i~ 
and ~false~ otherwise. This Function is called recursivly from function Disjoint 
to check disjoining on both sides of the interval.

*/

  bool Disjoint( const LInterval& i ) const;
/*
Returns ~true~ if this interval is disjoint with the interval ~i~ and 
~false~ otherwise.

*/

/*
2.3 Functions to be part of relations

2.3.1 Adjacent

*/
bool R_Adjacent( const LInterval& i ) const;
/*
Returns ~true~ if this interval is r-adjacent with the interval ~i~ and ~
false~ otherwise. This Function is called recursivly from function Adjacent to 
check the adjacence on both sides of the interval.

*/

  bool Adjacent( const LInterval& i ) const;
/*
Returns ~true~ if this interval is adjacent with the interval ~i~ and ~
 false~ otherwise.

*/

/*
2.4 Attributes

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