
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

#ifndef FILEATTRIBUTE_H
#define FILEATTRIBUTE_H


#include <string>

#include "NestedList.h"
#include "Attribute.h"

/*
1 Class ~FileAttribute~

This class provides implementation for saving an attribute to a file
and reading it back from the file. 

*/
class FileAttribute{
  public:

   static bool saveAttribute(ListExpr type, Attribute* value, 
                             const std::string& fileName);

   static bool saveAttribute(const std::string&  type, Attribute* value, 
                             const std::string& fileName);

   static ListExpr getType(const std::string& filename);

   static Attribute* restoreAttribute(ListExpr& type, 
                                      const std::string& fileName);
};

#endif


