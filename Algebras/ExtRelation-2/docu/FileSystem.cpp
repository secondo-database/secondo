/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics and Computer Science,
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

1 File System Management

May 2002 Ulrich Telle

Sept 2004 M. Spiekermann. Bugs in ~GetparentFolder~ and ~AppendSlash~ corrected.

Sept 2006 M. Spiekermann. When windows.h is included many WIN-API functions
like ~CopyFile~ are defined as a macro and mapped to ~CopyFileA~ or
~CopyFileB~. This is very awful since code parts using the
same name like class member functions are also renamed which causes strange
linker errors!

June 2009 Sven Jungnickel new function MakeTemp() added.

...

1 Implementation of extension of class ~FileSystem~

*/

string
FileSystem::MakeTemp(const string& templ)
{
  static int ctr = 0;	
  // append CPU clock and placeholder for mktemp function
  
  stringstream ss;
  ss << templ << clock() << "-" << ctr++;

  return ss.str();
}

/*
...

*/
