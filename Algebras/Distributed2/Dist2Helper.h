/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
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


//[$][\$]

*/

#ifndef DIST2HELPER
#define DIST2HELPER


#include "SecondoSMI.h"

namespace distributed2{

/*

1.1 ~readVar~

Using this function, objects stored in a single block of memory
can be read from a SmiRecord.

*/

template<class T>
bool readVar(T& value, SmiRecord& record, size_t& offset){
  bool res = record.Read(&value, sizeof(T), offset) == sizeof(T);
  offset += sizeof(T);
  return res;
}


/*
1.2 ~readVar~

Using this variant of readVar, a string can be read from an
SmiRecord.

*/
template<>
bool readVar<std::string>(std::string& value, 
                          SmiRecord& record, size_t& offset);

/*
1.3 writeVar

This function writes an object cosisting of a single block of
memory into an SmiRecord.

*/

template<class T>
bool writeVar(const T& value, SmiRecord& record, size_t& offset){
  bool res = record.Write(&value, sizeof(T), offset) == sizeof(T);
  offset += sizeof(T);
  return res;
}

/*
1.4 writeVar

This variant of ~writeVar~ write a string into an SmiRecord.

*/

template<>
bool writeVar<std::string>(
  const std::string& value, SmiRecord& record, size_t& offset);


}


#endif

