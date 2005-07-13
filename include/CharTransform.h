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
#ifndef CHAR_TRANSFORM_H
#define CHAR_TRANSFORM_H

#include <cstdlib>

/*

The functions in this file are necessary because in GCC 3.2
~toupper~ and ~tolower~ are templates. This makes it impossible
to use ~toupper~ or ~tolower~ in calls to the STL function
~transform~. Instead, the functions defined below can be used.

*/

inline char
ToUpperProperFunction(char c)
{
  return toupper(c);
};

inline char
ToLowerProperFunction(char c)
{
  return tolower(c);
}

/*
The class below can be used as a manipulator in streams:
os << tab(5, 'x') will create "xxxxx".

*/

class tab {
  char c;
  int n;
  public:
  tab(int no=4, char ch=' ') : c(ch), n(no) {}
  ostream& operator() (ostream& os) const {
    for (int i=n; i!=0; i--) os.put(c);
    return os; 
  }
};

// Next operator implemented in UtilFunctions.cpp
ostream& operator << (ostream& os, const tab& f);

#endif /* CHAR_TRANSFORM_H */
