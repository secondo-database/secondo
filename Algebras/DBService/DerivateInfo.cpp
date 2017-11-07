

/*

1 Class Implementation 


This class stores information about a derived object.
We store only such information that cannot be derived 
from the relation from that this objects depends on. 
For this reason, we ommit database name and original 
location. 

----
This file is part of SECONDO.

Copyright (C) 2017,
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
#include "Algebras/DBService/DerivateInfo.hpp"

namespace DBService
{

const std::string& DerivateInfo::getName() const
{
   return objectName;
}

const std::string& DerivateInfo::getSource() const
{
  return dependsOn;
}

const std::string& DerivateInfo::getFun() const
{
  return fundef;
}

} // end of namespace


