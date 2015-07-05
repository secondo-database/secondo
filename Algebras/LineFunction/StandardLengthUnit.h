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

[1] StandardLengthUnit.h

April 2015 - Rene Steinbrueck

1 Overview

[TOC]

2 Defines, includes, and constants

*/
#ifndef STANDARD_LENGTH_UNIT_H
#define STANDARD_LENGTH_UNIT_H

#include "LengthUnit.h"

/*
3 C++ Classes (Defintion)

3.1 StandardLengthUnit

This class inherits from ~Attribute~ and allows length units
of standard types to be part of relations. One should note that it is
still an abstract class, because the functions ~CopyFrom~ and ~Clone~
are not implemented.

This class contains a defined flag.

*/
template<typename Alpha>
class StandardLengthUnit :
    public Attribute,
    public LengthUnit<Alpha>
{
public:

/*
3.1.1 Constructors and Destructor

3.1.1.1 Standard constructor

*/

    StandardLengthUnit() {}
/*
The simple constructor. This constructor should not be used.

3.1.1.2 Default constructor

*/

    StandardLengthUnit( bool is_defined):
        Attribute(is_defined),
        LengthUnit<Alpha>(is_defined)
     {}

/*
This constructor initializes the LInterval with zero. If the passed
bool flag is true start and end are defined, otherwise not. The produced
linterval is left and right closed.

3.1.1.3 Setting constructor

*/

    StandardLengthUnit( const LInterval& linterval ):
        Attribute(true),
        LengthUnit<Alpha>( linterval )
        {
            del.refs=1;
            del.SetDelete();
            del.isDefined=true;
        }
/*
This constructor intializes the StandardLengthUnit with the passed LInterval.

3.1.1.4 Destructor

*/

    virtual ~StandardLengthUnit() {}

/*
3.1.2 Functions to be part of relations

3.1.2.1 Compare

*/

    virtual int Compare( const Attribute* arg ) const
    {
        return 0;
    }
/*
3.1.2.2 Adjacent

*/
    virtual bool Adjacent( const Attribute* arg ) const
    {
        return false;
    }

/*
3.1.2.3 HashValue

*/
    virtual size_t HashValue() const
    {
        if(!IsDefined())
        {
            return 0;
        }
        return static_cast<size_t>
        (
            this->lengthInterval.start.HashValue()^
            this->lengthInterval.end.HashValue()
        ) ;
    }

/*
3.1.2.4 CopyFrom

*/

    virtual void CopyFrom( const Attribute* right ) = 0;

/*
3.1.2.5 Print

*/

    virtual ostream& Print( ostream &os ) const
    {
      if( IsDefined() )
        {
          os << StandardLengthUnit<Alpha>::BasicType() << ": ( ";
          LengthUnit<Alpha>::lengthInterval.Print(os);
          os << ", NO SPECIFIC Print()-Method for this StandardTemporalUnit! )";
          return os;
        }
      else
        return os << StandardLengthUnit<Alpha>::BasicType() <<": (undef) ";
    }

/*
3.1.2.6 Basic Type

*/

    inline static const string BasicType()
    {
        return "lu"+Alpha::BasicType();
    }

/*
3.1.2.7 Check Type

*/

    static const bool checkType(const ListExpr type)
    {
        return listutils::isSymbol(type, BasicType());
    }

/*
3.1.2.8 Clone Function

*/

    virtual StandardLengthUnit<Alpha>* Clone() const = 0;

/*
3.1.2.9 SizeOf Function

*/
    virtual size_t Sizeof() const = 0;

};

/*
3.1.4 Output operator

*/

template<class Alpha>
ostream& operator<<(ostream& o, const StandardLengthUnit<Alpha>& u)
{
    return  u.Print(o);
}


#endif //STANDARD_LENGTH_UNIT_H
