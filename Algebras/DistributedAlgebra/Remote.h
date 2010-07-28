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

// Enthält die Klassen DServer, DServerManager

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
                  Word* elements;
         
                  CSProtocol* csp;
                  Socket* server;
};

class DServerManager
{
	public:
		DServerManager(ListExpr serverlist_n, 
			string name_n, ListExpr type, int sizeofarray);
		~DServerManager();
		DServer* getServerByIndex(int index);
		DServer* getServerbyID(int id);
		
		//int getID(int index) = 0;
		ListExpr getIndexList(int id);
		ListExpr getNamedIndexList(int id);
	
		int getNoOfServers();
	
	private:
		
		DServer** serverlist;
		int size;
		int array_size;
		string name;
};
                  
                  
                  


#endif
