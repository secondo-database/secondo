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
#include <iostream>

#include "LogMsg.h"

const int __endian_detect = 1;
 
inline bool __little_endian() { return *(char *)&__endian_detect == 1;}
 

int
main () {

 RTFlag::initByString("hallo,test,bla,blub");

 cout << RTFlag::isActive("hallo") << endl;
 cout << RTFlag::isActive("test") << endl;
 cout << RTFlag::isActive("bla") << endl;
 cout << RTFlag::isActive("blub") << endl;
 cout << RTFlag::isActive("xfindo") << endl;

 LOGMSG( "bla", 
 
   cout << "bla is active" << endl;

 )


 LOGMSG( "sdflkj",

   cout << "sdlfkj si not active " << endl;

 )
 
 cout << "machine has ";
 if ( __little_endian() ) {
   cout << "little";
 } else {
   cout << "big";
 }
 cout << " endian byte order!" << endl;

 RTFlag::initByString("");
 RTFlag::initByString(",");
 RTFlag::initByString("xvc");
 cout << RTFlag::isActive("xvc") << endl;
 

}
