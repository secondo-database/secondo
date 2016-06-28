/*
----
This file is part of SECONDO.

Copyright (C) since 2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

#include "Trace.h"
#include <string.h>

using std::string;
using std::stringstream;
using std::endl;

string Array2HexStr(const char* data, size_t size, size_t offset /*= 0*/)
{
  stringstream res;

  size_t i = 0;
  const size_t k = 16;
  res << endl << "Memory plot of address " << (void*)data << ":" << endl;
  res << std::hex; 
  while ( i < size )
  {
    unsigned int val = static_cast<unsigned char>( data[i+offset] ) & 255;  
    res.width(2);
    res.fill('0');
    res << val << " ";
    i++;
    if ( (i % k) == 0) {
      res << endl;
    }  
  }  
  return res.str();  
} 

string Array2Str(const char* data, size_t size)
{
  char* buf = new char[size+1];
  memcpy(buf,data,size);
  buf[size] = '\0';
  string res(buf);
  delete [] buf;
  return res;
}

