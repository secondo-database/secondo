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

/*
[1] DistributedAlgebra

March 2010 Tobias Timmerscheidt

*/

// Enthält die Klassen ObjectReader/Writer, RemoteObject

#ifndef H_REMOTE_H
#define H_REMOTE_H

#include "StandardTypes.h"
#include "SocketIO.h"
#include "Profiles.h"
#include "CSProtocol.h"

using namespace std;

class DServer
{
         public:
                  DServer(string,int,string,ListExpr);
                  void Terminate();
                  //Word getData();
                  //void setData(Word);
                  void setCmd(string,ListExpr,Word*);
                  void run();
                  
                  int getState();
         
         private:
                  string host,name,cmd;
                  int port,state;
                  ListExpr type,arg;
                  int arg2;
                  Word* elements;
         
                  CSProtocol* csp;
                  Socket* server;
};
                  
                  
                  


#endif
