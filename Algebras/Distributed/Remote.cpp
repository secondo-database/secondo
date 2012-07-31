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

November 2010 Tobias Timmerscheidt

This file contains the implementation of DServer, DServerManager,
DServerCreator

*/

#include "Remote.h"
#include "DServer.h"
#include "DBAccessGuard.h"
#include "SocketIO.h"
#include "Processes.h"
#include "zthread/Runnable.h"
#include "zthread/Thread.h"
#ifndef SINGLE_THREAD1
#include "zthread/ThreadedExecutor.h"
#endif
#include "zthread/Mutex.h"
#include "StringUtils.h"
#include <iostream>
#include "ThreadedMemoryCntr.h"

using namespace std; 


/*

7 Class DServerCreator

*/

DServer*
DServerCreator::createServer()
{
  m_server = new DServer(m_host, m_port,m_name,m_type);
  //cout << "Server: "<< port << "@" << host << " created" << endl;
  return m_server;
}


void DServerCreator::run()
{ 
  //cout << "Connecting Server: "<< m_host << ":" << m_port << endl;
  m_server -> connectToWorker();
}

void 
DServerMultiplyer::run()
{
  if (!server->Multiply(count))
    {
      cerr << "Error multiplying Servers:" 
           << server -> getErrorText() << endl;
      assert(0);
    }
}
