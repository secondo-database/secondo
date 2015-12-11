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

[1] LengthUnit.h

April 2015 - Rene Steinbrueck

1 Overview

[TOC]

2 Defines, includes, and constants

*/
#ifndef LENGTH_UNIT_H
#define LENGTH_UNIT_H

#include "LInterval.h"

/*
3 C++ Classes (Defintion)

3.1 LengthUnit

This class will generically implements a length unit. It is an abstract class
that enforces each kind of length unit to have a function that computes a value
inside the length unit given a length instant (also inside the length unit).

The LengthUnit does not yet contain a ~defined~ flag!

*/
template <typename Alpha>
class LengthUnit
{

public:

/*
3.1.1 Constructors and Destructor

3.1.1.1 Standard constructor

*/

    LengthUnit() {}
/*
The simple constructor. This constructor should not be used.

3.1.1.2 Default constructor

*/

    LengthUnit(bool defined): lengthInterval(defined) {}
/*
This constructor initializes the LInterval with zero. If the passed
bool flag is true start and end are defined, otherwise not. The produced
linterval is left and right closed.

3.1.1.3 Setting constructor

*/

    LengthUnit( const LInterval& linterval ): lengthInterval( linterval ) { }
/*
This constructor intializes the LengthUnit with the passed LInterval.

3.1.1.4 Destructor

*/

    virtual ~LengthUnit() {}

/*
3.1.2 Member Functions

3.1.2.1 IsValid

*/

    bool IsValid() const;
/*
Checks if the Length Unit is valid or not. This function should be used for debugging purposes
only.  A LengthUnit is valid if its lengthInterval is valid.

3.1.2.2 Disjoint

*/

    bool R_Disjoint( const LengthUnit& i ) const;
/*
Returns ~true~ if this length unit is r-disjoint with the length unit ~i~ and ~false~ otherwise.

*/

    bool Disjoint( const LengthUnit& i ) const;
/*
Returns ~true~ if this length unit is disjoint with the length unit ~i~ and ~false~ otherwise.

3.1.2.3 Before

*/

    bool Before( const LengthUnit& i ) const;
/*
Returns ~true~ if this length unit is before the length unit ~i~ and ~false~ otherwise.

*/

    bool Before( const CcReal& m ) const;
/*
Returns ~true~ if this length unit is before the value ~a~ and ~false~ otherwise.

3.1.2.4 After

*/

    bool After( const CcReal& m ) const;
/*
Returns ~true~ if this length unit is after the value ~a~ and ~false~ otherwise.

3.1.2.5 LengthFunction

*/

virtual void LengthFunction( const CcReal& l,
                             Alpha& result,
                             bool ignoreLimits = false ) const = 0;
/*
The length function that receives a length instant ~l~ and returns the value
associated with length ~l~ in the output argument ~result~.
If ~ignoreLimits = true~, the limits given by the ~lengthinterval~ will be ignored.
You can use this feature e.g. to calculate values for the ~initial~ and ~final~
instants of left/right open intervals.

Otherwise (~ignoreLimits = false~, resp. unspecified) there will be following

*Precondition:* l must be inside the length unit length interval.

3.1.3 Operator Redefinitions

3.1.3.1 Operation ~copy~

*/

    virtual LengthUnit& operator=( const LengthUnit& i );
/*
Redefinition of the copy operator ~=~.

3.1.3.2 Operation $=$ (~equal~)

*/

    virtual bool operator==( const LengthUnit& i ) const;
/*
Returns ~true~ if this length unit is equal to the length unit ~i~ and ~false~ if they are different.

3.1.3.3 Operation $\neq$ (~not equal~)

*/

    virtual bool operator!=( const LengthUnit& i ) const;
/*
Returns ~true~ if this length unit is different to the length unit ~i~ and ~false~ if they are equal.

3.1.4 Functions to be part of relations

3.1.4.1 Adjacent

*/

    bool R_Adjacent( const LengthUnit& i ) const;
/*
Returns ~true~ if this length unit is r-adjacent with the length unit ~i~ and ~false~ otherwise.

*/

    bool Adjacent( const LengthUnit& i ) const;
/*
Returns ~true~ if this length unit is adjacent with the length unit ~i~ and ~false~ otherwise.

3.1.4.2 Basic Type

*/

    inline static const std::string BasicType()
    {
        return "lu"+Alpha::BasicType();
    }

/*
3.1.4.3 Check Type

*/

    static const bool checkType(const ListExpr type)
    {
        return listutils::isSymbol(type, BasicType());
    }

/*
3.1.5 Attributes

*/

    inline const LInterval& getLengthInterval() const
    {
        return lengthInterval;
    }

    LInterval lengthInterval;
/*
The length interval of the length unit.

*/

};
/*
1 C++ Classes (Implementation)

1.1 LengthUnit

1.1.1 Member Functions

1.1.1.1 IsValid

*/

template <typename Alpha>
bool LengthUnit<Alpha>::IsValid() const
{
    return lengthInterval.IsValid();
}

/*
1.1.1.2 Disjoint

*/

template <typename Alpha>
bool LengthUnit<Alpha>::R_Disjoint( const LengthUnit<Alpha>& i ) const
{
    return( lengthInterval.R_Disjoint( i.lengthInterval ) );
}

template <typename Alpha>
bool LengthUnit<Alpha>::Disjoint( const LengthUnit<Alpha>& i ) const
{
    assert( IsValid() && i.IsValid() );

    return( R_Disjoint( i ) || i.R_Disjoint( *this ) );
}

/*
1.1.1.3 Before

*/

template <typename Alpha>
bool LengthUnit<Alpha>::Before( const LengthUnit<Alpha>& i ) const
{
    assert( IsValid() && i.IsValid() );

    return ( lengthInterval.Before(i.lengthInterval) );
}

template <typename Alpha>
bool LengthUnit<Alpha>::Before( const CcReal& i ) const
{
    assert( IsValid() && i.IsDefined() );

    return ( lengthInterval.Before( i ) );
}

/*
1.1.1.4 After

*/

template <typename Alpha>
bool LengthUnit<Alpha>::After( const CcReal& i ) const
{
    assert( IsValid() && i.IsDefined() );

    return ( lengthInterval.After( i ) );
}

/*
1.1.2 Operator Redefinitions

1.1.2.1 Operation ~copy~

*/

template <typename Alpha>
LengthUnit<Alpha>& LengthUnit<Alpha>::operator=( const LengthUnit<Alpha>& i )
{
    assert( i.lengthInterval.IsValid() );
    lengthInterval = i.lengthInterval;
    return *this;
}

/*
1.1.2.2 Operation $=$ (~equal~)

*/

template <typename Alpha>
bool LengthUnit<Alpha>::operator==( const LengthUnit<Alpha>& i ) const
{
    assert( lengthInterval.IsValid() && i.lengthInterval.IsValid() );
    return( lengthInterval == i.lengthInterval);
}

/*
1.1.2.3 Operation $neq$ (~not equal~)

*/

template <typename Alpha>
bool LengthUnit<Alpha>::operator!=( const LengthUnit<Alpha>& i ) const
{
    return !( *this == i );
}

/*
1.1.3 Functions to be part of relations

1.1.3.1 Adjacent

*/

template <typename Alpha>
bool LengthUnit<Alpha>::R_Adjacent( const LengthUnit<Alpha>& i ) const
{
    return( lengthInterval.R_Adjacent(i.lengthInterval));
}

template <typename Alpha>
bool LengthUnit<Alpha>::Adjacent( const LengthUnit<Alpha>& i ) const
{
    assert( IsValid() && i.IsValid() );

    return( R_Adjacent( i ) || i.R_Adjacent( *this ) );
}

#endif //LENGTH_UNIT_H
