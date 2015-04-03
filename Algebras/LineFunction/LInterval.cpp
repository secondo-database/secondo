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

{\Large \bf \begin{center}LInterval.cpp\end{center}}

\tableofcontents
\newpage

1 Defines and includes

*/
#include "LInterval.h"

/*
2 LInterval

2.1 Constructors

2.1.1 Default constructor

*/

LInterval::LInterval ( const bool boolvalue ) :
start ( CcReal ( boolvalue ) ) ,
end ( CcReal ( boolvalue ) ) ,
lc ( true ) , rc ( true ) { }

/*
2.1.2 Copy Constructor

*/

LInterval::LInterval ( const LInterval& linterval ) :
start ( linterval.start ) , 
end ( linterval.end ) ,
lc ( linterval.lc ) , 
rc ( linterval.rc ) { }

/*
2.1.3 Setting Constructor

*/

LInterval::LInterval ( const CcReal& start , const CcReal& end ,
            const bool lc , const bool rc ) :
start ( start ) , end ( end ) , lc ( lc ) , rc ( rc ) { }

/*
2.2 Member functions

2.2.1 IsValid

*/
bool LInterval::IsValid() const
{
    if( !start.IsDefined() || !end.IsDefined() )
    {
        return false;
    }
    int cmp = start.Compare( &end );
    if( cmp < 0 ) // start < end
    {
        return true;
    }
    else if( cmp == 0 ) // start == end
    {
        return rc && lc;
    }
    // start > end
    return false;
}

/*
2.2.2 Disjoint

*/

bool LInterval::R_Disjoint( const LInterval& i ) const
{
    bool res= ((end.Compare( &i.start ) < 0) ||
    ( (end.Compare( &i.start ) == 0) && !( rc && i.lc ) ));
    return( res );
}

bool LInterval::Disjoint( const LInterval& i ) const
{
    assert( IsValid() && i.IsValid() );
    bool res=( R_Disjoint( i ) || i.R_Disjoint( *this ) );
    return( res );
}

/*
2.3 Functions to be part of relations

2.3.1 Adjacent

*/

bool LInterval::R_Adjacent( const LInterval& i ) const
{
    bool res=( (Disjoint( i ) &&
    ( end.Compare( &i.start ) == 0 && (rc || i.lc) )) ||
    ( ( end.Compare( &i.start ) < 0 && rc && i.lc ) &&
    end.Adjacent( &i.start ) ) );
    return( res );
}

bool LInterval::Adjacent( const LInterval& i ) const
{
    assert( IsValid() && i.IsValid() );
    bool res= ( R_Adjacent( i ) || i.R_Adjacent( *this ) );
    return( res );
}