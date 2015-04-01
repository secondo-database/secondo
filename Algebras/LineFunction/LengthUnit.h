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

{\Large \bf \begin{center}LengthUnit.h\end{center}}

\tableofcontents
\newpage

1 Defines and includes

*/

#ifndef LENGTH_UNIT_H
#define LENGTH_UNIT_H

#include "LInterval.h"


/*
2 Class LengthUnit

This class will generically implements a length unit.
It is an abstract class that enforces each kind of length unit to have a 
function that computes a value inside the length unit given a length instant 
(also inside the length unit).

The LengthUnit does not yet contain a ~defined~ flag!

*/
template <class Alpha>
class LengthUnit
{
public:
/*
2.1 Constructors

2.1.1 Default Constructor

*/
    LengthUnit(bool defined = true);
/*
Use this constructor when declaring length object variables etc.
The linterval is initialized by calling the default constructor of 
the linterval.
 
2.1.2 Copy constructor

*/
    LengthUnit( const LInterval& interval );

/*
This constructor sets the length interval of the length unit by calling the
copy constructor of the linterval.

2.2 Destructor

*/
    virtual ~LengthUnit();

/*
2.3 Member functions

2.3.1 Length function

*/
    virtual void LengthFunction 
    ( const CcReal& l ) const = 0;
/*
The length function that receives a length instant ~l~ and returns 
the value associated with length ~l~ in the output argument ~result~.

2.4 Attributes

*/

    LInterval lengthInterval;
/*
The length interval of the length unit.

*/

};
#endif //LENGTH_UNIT_H