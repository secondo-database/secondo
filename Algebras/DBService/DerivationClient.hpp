/*

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

#ifndef DERIVATIONCLIENT_HPP
#define DERIVATIONCLIENT_HPP

#include <string>
#include <string.h>

namespace DBService{

/*
1 Class DerivationClient

This class derives a new object from a relation using a 
function and inserts it into the database.

*/


class DerivationClient{

 public:

/*
1.1 Constructor

*/    
  DerivationClient(
         const std::string& _DBName, 
         const std::string& _targetName,
         const std::string& _relName,
         const std::string& _fundef);

/*
1.2 Function start()

This function creates a new object using arguments given in constructor.

*/
  void start();

 private:
     std::string DBName;
     std::string targetName;
     std::string relName;
     std::string fundef;
     std::string relId;
     std::string targetId;

     void derivationFailed(const std::string& error);
     void derivationSuccessful();

};

}

#endif
