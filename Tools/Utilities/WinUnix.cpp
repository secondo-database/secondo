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

September 2003, M. Spiekermann: Implementation of getpagesize()

*/

#include <unistd.h>
#include "SecondoConfig.h"

#ifdef SECONDO_WIN32
#include <windows.h>
#endif

#include "WinUnix.h"

const int
WinUnix::endian_detect = 1;

int
WinUnix::getPageSize( void ) { 

#ifndef SECONDO_WIN32
   return ( getpagesize() );
#else
   LPSYSTEM_INFO lpSysInf;
   GetSystemInfo( lpSysInf );
   return ( lpSysInf->dwPageSize );
#endif

}
