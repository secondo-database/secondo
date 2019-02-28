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

*/

#include "Nr2aException.h"
#include "Nr2aHelper.h"

using namespace std;

namespace nr2a {

/*
It features constructors taking a char pointer or a string describing the
exception thrown.

*/
Nr2aException::Nr2aException(const char * const msg)
{
  text = string(msg);
}

Nr2aException::Nr2aException(const string msg)
{
  text = msg;
}


/*virtual*/ Nr2aException::~Nr2aException() throw ()
{

}

/*
The exception text is prefixed with ~NR2A:~ to clarify the exception's source.

*/
void
Nr2aException::Init(const char * const msg)
{
  text = msg;
}

/*virtual*/const char *Nr2aException::what() const noexcept
{
  return text.c_str();
}

/*
It features constructors taking a char pointer or a string describing the
exception thrown.

*/
Nr2aParserException::Nr2aParserException(const char * const msg,
    const int line) : Nr2aException(msg)
{
  text.append(" before or in line ");
  text.append(Nr2aHelper::IntToString(line));
}

Nr2aParserException::Nr2aParserException(const string msg,
    const int line) : Nr2aException(msg)
{
  text.append(" before or in line ");
  text.append(Nr2aHelper::IntToString(line));
}

/*virtual*/ Nr2aParserException::~Nr2aParserException() throw ()
{

}

/*virtual*/const char *Nr2aParserException::what() const noexcept
{
  return text.c_str();
}
}

