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

#ifndef ALGEBRAS_NESTEDRELATION2_NR2AEXCEPTION_H_
#define ALGEBRAS_NESTEDRELATION2_NR2AEXCEPTION_H_

#include <iostream>
#include <string>

namespace nr2a {

/*
The algebra's exception class inheriting from the standard exception class.

*/
class Nr2aException : private std::exception
{
public:
    Nr2aException(const char * const msg);
    Nr2aException(const std::string msg);
    virtual ~Nr2aException() throw ();

  virtual const char * what();

protected:
  void Init(const char * const msg);
  std::string text;
};

/*
The algebra's exception class inheriting from the standard exception class.

*/
class Nr2aParserException : protected Nr2aException
{
public:
    Nr2aParserException(const char * const msg, const int line);
    Nr2aParserException(const std::string msg, const int line);
    Nr2aParserException(const Nr2aException ex, const int line);
    virtual ~Nr2aParserException() throw ();

  virtual const char * what();

private:
  void Init(const char * const msg, const int line);
};
}



#endif /* ALGEBRAS_NESTEDRELATION2_NR2AEXCEPTION_H_*/
