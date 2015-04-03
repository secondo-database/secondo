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

{\Large \bf \begin{center}LengthUnit.cpp\end{center}}

\tableofcontents
\newpage

1 Defines and includes

*/
#include "LengthUnit.h"

/*
2 Class LengthUnit
 
2.1 Constructors
 
2.1.1 Default constructor
 
*/
template <typename Alpha>
LengthUnit<Alpha>::LengthUnit(bool defined): 
lengthInterval(defined) {}

/*
2.1.2 Copy constructor

*/
template <typename Alpha>
LengthUnit<Alpha>::LengthUnit( const LInterval& interval ): 
lengthInterval( interval ) {}

/*
2.2 Destructor

*/
template <typename Alpha>
LengthUnit<Alpha>::~LengthUnit() {};