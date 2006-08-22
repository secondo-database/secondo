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

September 03, M. Spiekermann: Initial Version


This class consists of static functions which are used to hide
calls to different Win32 or Unix/Linux libraries, thus all system
dependent code should be isolated in this class.

*/

#ifndef CLASS_WINUNIX_H
#define CLASS_WINUNIX_H


class WinUnix {

public:
   WinUnix(){};
   ~WinUnix(){};

   static int getPageSize( void );
 
   static inline bool isLittleEndian() { return *(char *)&endian_detect == 1;}

   static int rand(void) { return rand(); }

   static void srand(unsigned int seed) { return srand(seed); }
   
private:
   static const int endian_detect;

};

#endif
