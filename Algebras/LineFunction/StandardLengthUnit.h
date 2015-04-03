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

{\Large \bf \begin{center}StandardLengthUnit.h\end{center}}

\tableofcontents
\newpage

1 Defines and includes

*/
#ifndef STANDARD_LENGTH_UNIT_H
#define STANDARD_LENGTH_UNIT_H

#include "../../include/Attribute.h"
#include "LengthUnit.h"

/*
2 StandardLengthUnit

This class inherits from ~Attribute~ and allows length units of standard types 
to be part of relations. One should note that it is still an abstract class, 
because not all functions are implemented.

This class contains a defined flag (derived from class attribute).

*/
template<typename Alpha>
class StandardLengthUnit : public Attribute , public LengthUnit<Alpha>
{
public:
/*
2.1 Constructors

2.1.2 Standard constructor

*/
    StandardLengthUnit () {}
/*
This constructor should not be used. The definition is necessary because the 
class StandardLengthUnit is derived from class Attribute.

2.1.3 Default constructor

*/
    StandardLengthUnit ( bool is_defined = true ):
    Attribute ( is_defined ) , LengthUnit<Alpha> ( is_defined ) {}
/*
Use this constructor when declaring length object variables etc.

2.1.4 Copy constructor

*/
    StandardLengthUnit ( const LInterval& interval ):
    Attribute ( true ) , LengthUnit<Alpha> ( interval ) {}
/*
This constructor sets the length interval of the length unit with the value
of the passed length interval.

2.2 Destructor

*/
    virtual ~StandardLengthUnit () {}

/*
2.3 Functions to be part of relations

2.3.1 Adjacent

*/

    bool R_Adjacent( const StandardLengthUnit<Alpha>& i ) const
    {
        return( lengthInterval.R_Adjacent(i.lengthInterval));
    }
/*
Returns ~true~ if this length unit is r-adjacent with the length unit ~i~ 
and ~false~ otherwise. This Function is called recursivly from function Adjacent 
to check the adjacence on both sides of the StandardLengthUnit.

*/

    bool Adjacent( const StandardLengthUnit& i ) const
    {
        assert( IsValid() && i.IsValid() );
        return( R_Adjacent( i ) || i.R_Adjacent( *this ) );
    }
/*
Returns ~true~ if this length unit is adjacent with the length unit ~i~ 
and ~false~ otherwise.

2.3.2 CheckType

*/

    static const bool checkType ( const ListExpr type )
    {
        return (listutils::isSymbol ( type , BasicType ( ) ));
    }
/*
This function checks wheter a nested list is a correct type description for the
 appropriate length type.

2.3.4 Functions that are implemented not yet.

*/
    virtual int Compare ( const Attribute* arg ) const = 0

    virtual StandardLengthUnit<Alpha>* Clone ( ) const = 0;

    virtual void CopyFrom ( const Attribute* right ) = 0;

    virtual size_t Sizeof ( ) const = 0;

    virtual ostream& Print ( ostream &os ) const = 0

    virtual size_t HashValue ( ) const = 0

    virtual string BasicType ( )
}
/*
3 The output operator:

*/
template<class Alpha>
ostream& operator<<(ostream& o, const StandardLengthUnit<Alpha>& u)
{
    return  u.Print(o);
}

#endif //STANDARD_LENGTH_UNIT_H