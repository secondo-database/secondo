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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the Spatiotemporal Pattern Algebra

Jan, 2010 Mahmoud Sakr

[TOC]

1 Overview


2 Defines and includes

*/

#include "MSet.h"

bool 
Helpers::string2int(char* digit, int& result) 
{
  result = 0;
  while (*digit >= '0' && *digit <='9') {
    result = (result * 10) + (*digit - '0');
    digit++;
  }
  return (*digit == 0); // true if at end of string!
}

string 
Helpers::ToString( int number )
{
  ostringstream o;
  o << number ; //<< char(0)
  return o.str();
}

string 
Helpers::ToString( double number )
{
  ostringstream o;
  o << number ; //<< char(0)
  return o.str();
}

namespace SetCollection{

};
