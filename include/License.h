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

Jan. 2005, M. Spiekermann

*/

#ifndef SECONDO_H_CLASS_LICENSE

#include <string>

class License {

private:
  License(){}

public:
  ~License(){}

  static const std::string& getStr() {

    static const std::string licenseStr = std::string(
      "\nCopyright (c) 2004-2021 University in Hagen, \n"
      "Department of Computer Science,  \n"
      "Database Systems for New Applications. \n\n"
      "This is free software; see the source for copying conditions.\n"
      "There is NO warranty; not even for MERCHANTABILITY or FITNESS \n"
      "FOR A PARTICULAR PURPOSE.\n");

    return licenseStr;
  }

};

#endif
