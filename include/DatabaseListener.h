/*
---- 
This file is part of SECONDO.

Copyright (C) 2018, University in Hagen, 
Faculty of Mathematics and  Computer Science, 
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

#pragma once

#include <string>
#include "SecondoSystem.h"

/*
1 Database Listener

This class provides an interface to get information about changes in 
the database. This listener can be registered at the SecondoSystem. 

*/
class DatabaseListener{
  public:

/*
1.1 ~openDatabase~

This function will be called if this DatabaseListener is registered
at the SecondoSystem (function addDBListener) after a database 
has been successful opened.

*/     
     virtual void openDatabase(const std::string& name) = 0;

/*
1.1 ~closeDatabase~

This function will be called if this DatabaseListener is registered
at the SecondoSystem (function addDBListener) before a database 
will be closed.

*/
     virtual void closeDatabase() = 0;


     virtual ~DatabaseListener(){
        SecondoSystem* sys =  SecondoSystem::GetInstance();
        if(sys){
           sys->removeDBListener(this);
        }
     }


};


