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

[1] LUReal.h

April 2015 - Rene Steinbrueck

1 Overview

[TOC]

2 Defines, includes, and constants

*/
#ifndef LUREAL_H
#define LUREAL_H

#define PI 3.14159265

#include "StandardLengthUnit.h"

/*
3 C++ Classes (Defintion)

3.1 LUReal

This class will be used in the ~lureal~ type constructor, i.e., the type constructor
for the length unit of real numbers.
Inherits a denined flag.

*/

class LUReal : public StandardLengthUnit<CcReal>
{
public:
/*
3.1.1 Constructors and Destructor

3.1.1.1 Standard constructor

*/

    LUReal() {};
/*
The simple constructor. This constructor should not be used.

3.1.1.2 Default constructor

*/

    LUReal(bool is_defined):
        StandardLengthUnit<CcReal>(is_defined),
        m(0.0), n(0.0)
    { };
/*
This constructor initializes the LInterval and the length function parameters with
zero. If the passed bool flag is true start and end are defined, otherwise not.
The produced linterval is left and right closed.

3.1.1.3 Setting constructor

*/

    LUReal( const LInterval& linterval,
            const double a,
            const double b,
            const bool functionparameters = true):
            StandardLengthUnit<CcReal>( linterval )
    {
        del.refs=1;
        del.SetDelete();
        del.isDefined=true;

        if (functionparameters)
        {
            m = a;
            n = b;
        }
        else
        {
            m = (b-a) / (linterval.end.GetRealval()
                - linterval.start.GetRealval());
            n = a - (m * linterval.start.GetRealval());
        }
    }
/*
This constructor intializes the length interval of the LUReal with the
passes linterval. If the boolean flag ~functionparameters~ is true m and n
are initialized with the passed values ~a~ and ~b~. Otherwise the values of ~m~ and
~n~ are calculated from the passed values ~a~ and ~b~ and the start and end value
of the passed linterval.

3.1.2 Member functions

3.1.2.1 LengthFunction

*/

    virtual void LengthFunction( const CcReal& l,
                                 CcReal& result,
                                 bool ignoreLimits = false ) const
    {
        if ( !this->IsDefined() ||
        !l.IsDefined() ||
        (!this->lengthInterval.Contains( l ) && !ignoreLimits) )
    {
        result.Set(false, 0.0);
    }
    else
    {
        double res = m * l.GetRealval() + n;
        result.Set( true, res );
    }
}

/*
3.1.2.2 EqualValue

*/
    virtual bool EqualValue( const LUReal& i1 ) const
    {
        if( !this->IsDefined() && !i1.IsDefined() )
            return true;
        if( !this->IsDefined() || !i1.IsDefined() )
            return false;

        double val1 = atan (this->m) * 180 / PI;
        double val2 = atan (i1.m) * 180 / PI;
        return (((val2-val1)<1 && (val2-val1)>-1) ? true : false) ;
  }

/*
Returns ~true~ if the value of this length unit is equal to the
value of the length unit ~i~ and ~false~ if they are different.
Two values are equal if the absolut difference between the gradient ~m~
is smaller than 1 degree.


3.1.3 Operator Redefinitions

3.1.3.1 Operation ~copy~

*/

    virtual LUReal& operator=( const LUReal& i )
    {
        del.isDefined = i.del.isDefined;
        if( !i.IsDefined() )
        {
            return *this;
        }
        *((LengthUnit<CcReal>*)this) = *((LengthUnit<CcReal>*)&i);
        m = i.m;
        n = i.n;
        return *this;
    }
/*
Redefinition of the copy operator ~=~.

3.1.3.2 Operation $=$ (~equal~)

*/

    virtual bool operator==( const LUReal& i ) const
    {
        if( !this->IsDefined() && !i.IsDefined() )
        {
            return true;
        }
        return
        (
            this->IsDefined() && i.IsDefined()
            && *((LengthUnit<CcReal>*)this) == *((LengthUnit<CcReal>*)&i)
            && AlmostEqual( m, i.m )
            && AlmostEqual( n, i.n )
        );
    }
/*
Returns ~true~ if this length unit is equal to the length unit ~i~ and ~false~
if they are different. Two undefined units are always equal.

3.1.3.3 Operation $\neq$ (~not equal~)

*/

    virtual bool operator!=( const LUReal& i ) const
    {
        return !( *this == i );
    }
/*
Returns ~true~ if this length unit is different to the length unit ~i~ and ~false~
if they are equal.

3.1.4 Functions to be part of relations

3.1.4.1 Compare

*/

    virtual int Compare( const Attribute* arg ) const
    {
        LUReal* lur2 = (LUReal*) arg;

        if (!IsDefined() && !lur2->IsDefined())
            return 0;
        if (!IsDefined())
            return -1;
        if (!lur2->IsDefined())
            return 1;

        // both ureals are defined...
        int cmp = lengthInterval.CompareTo(lur2->lengthInterval);
        if(cmp)
            return cmp;
        // because we can't compare the functions themself, we
        // use the lexicographical order of the parameters
        if(m<lur2->m) return -1;
        if(m>lur2->m) return 1;
        if(n<lur2->n) return -1;
        if(n>lur2->n) return 1;
        return 0;
    }

/*
3.1.4.2 Adjacent

*/

    virtual bool Adjacent( const Attribute* arg ) const
    {
        return false;
    }

/*
3.1.4.3 HashValue

*/

    virtual size_t HashValue() const
    {
        if(!IsDefined())
            return 0;
        return static_cast<size_t>
        (
            lengthInterval.start.HashValue()^
            lengthInterval.end.HashValue()
        );
    }

/*
3.1.4.4 CopyFrom

*/

    virtual void CopyFrom( const Attribute* right )
    {
        const LUReal* i = (const LUReal*)right;

        del.isDefined = i->del.isDefined;

        if(i->IsDefined())
        {
            lengthInterval.CopyFrom(i->lengthInterval);
            m = i->m;
            n = i->n;
        }
        else
        {
            lengthInterval = LInterval();
            m = 0;
            n = 0;
        }
    }

/*
3.1.4.5 Print

*/

    virtual std::ostream& Print( std::ostream &os ) const
    {
        if( IsDefined() )
        {
            os << "LUReal: " << "( ";
            lengthInterval.Print(os);
            os << ", ";
            // print specific stuff:
            os << " ( " << m << ", " << n << " )";
            os << " ) ";
            return os;
        }
        else
            return os << "LUReal: (undef) ";
    }

/*
3.1.4.6 Basic Type

*/

    static const std::string BasicType(){ return "lureal"; }

/*
3.1.4.7 Check Type

*/

    static const bool checkType(const ListExpr type)
    {
        return listutils::isSymbol(type, BasicType());
    }

/*
3.1.4.8 Clone Function

*/

    virtual LUReal* Clone() const
    {
        LUReal *res;
        if( !this->IsDefined() )
            res = new LUReal( false );
        else
            res = new LUReal( lengthInterval, m, n );
        res->del.isDefined = del.isDefined;
        return res;
    }

/*
3.1.4.9 SizeOf Function

*/

    virtual size_t Sizeof() const
    {
        return sizeof( *this );
    }


/*
3.1.6 Attributes

*/
    double m, n;

};


#endif //LUREAL_H
