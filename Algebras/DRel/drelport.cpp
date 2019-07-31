/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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

*/

#include <string>
#include "StringUtils.h"

namespace drel {

int drelport;
std::string drelportstring;


bool setDRelPort(const int port) {
   if(port<1 || port > 65535){
     return false;
   }
   drelport = port;
   drelportstring = stringutils::int2str(port);
   return true;
}

int getDRelPort() {
   return drelport;
}

std::string getDRelPortString() {
   return drelportstring; 
}

}

