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
using namespace std;

#include <iostream>
#include <string>
#include "Messenger.h"
#include "SocketIO.h"

bool
Messenger::Send( const string& message, string& answer )
{
  bool ok = false;
  answer = "";
  Socket* msgServer = Socket::Connect( msgQueue, "", Socket::SockLocalDomain, 3, 1 );
  if ( msgServer && msgServer->IsOk() )
  {
    iostream& ss = msgServer->GetSocketStream();
    ss << message << endl;
    getline( ss, answer );
    ok = true;
  }
  else
  {
    answer = "Connect to registrar failed.";
  }
  delete msgServer;
  return (ok);
}

