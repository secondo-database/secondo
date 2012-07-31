/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[&] [\&]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]
//[ast] [\ensuremath{\ast}]

*/

/*
[1] Class DServerManager Implementation

\begin{center}
March 2012 Thomas Achmann


November 2010 Tobias Timmerscheidt
\end{center}

[TOC]

0 Description

Implementation of the class ~DServerManager~



1 Preliminaries

1.1 Includes

*/
#include "DServerManager.h"
#include "DServer.h"
#include "Remote.h" // for DServerCreator
#include "zthread/ThreadedExecutor.h"
/*

2 Implementation of the class DServerManager

2.1 Constructor

Creates a new DServerManager specified by a serverlist
All DServer-Obects are created

*/

DServerManager::DServerManager(ListExpr serverlist_n, 
                               string name_n, 
                               ListExpr inType,
                               int sizeofarray)
  : m_error(false)
{
  cout << "Connecting to Workers... " << endl;
  //m_watch.start();
   array_size = sizeofarray;
   name = name_n;
   size = nl->ListLength(serverlist_n);
   if(size==-1) 
     size=1;
                
   ListExpr elem = nl->First(serverlist_n);
   serverlist_n = nl->Rest(serverlist_n);

   // starting two workers on the same
   // host is terribly slow.
   // This routine starts only
   // one woker on the host and
   // waits until started. Then 
   // it continues startning the 
   // next worker.

   // creating a list of all hosts and ports
   vector<string> nextRoundHosts (size);
   vector<int> nextRoundPorts (size);
   vector<int> nextRoundHostID (size); // needed for backward compatibility.

   for(int i = 0; i<size; i++)
     {
       nextRoundHosts[i] =  nl->StringValue(nl->First(elem));
       nextRoundPorts[i] = nl->IntValue(nl->Second(elem));
       nextRoundHostID[i] = i;
       m_serverlist.push_back(NULL);

       if(i < size-1)
         {
           elem = nl->First(serverlist_n);
           serverlist_n = nl->Rest(serverlist_n);
         }
     }

   while(!nextRoundHosts.empty())
     {
       vector<string> thisRoundHosts;
       vector<int> thisRoundPorts;
       vector<int> thisRoundHostID;
       //swap
       thisRoundHosts.swap(nextRoundHosts);
       thisRoundPorts.swap(nextRoundPorts);
       thisRoundHostID.swap(nextRoundHostID);
       try
         {
           ZThread::ThreadedExecutor exec;      
                    
           set<string> connectedHosts;

           while (!thisRoundHosts.empty())
             {
               string host = thisRoundHosts.back();
               thisRoundHosts.pop_back();
               int port = thisRoundPorts.back();
               thisRoundPorts.pop_back();
               int hId = thisRoundHostID.back();
               thisRoundHostID.pop_back();
               /*
                * enable this line, if sequential start
                * of secondo on the worker is better!
               if (connectedHosts.find(host) != connectedHosts.end())
                 {
                   nextRoundHosts.push_back(host);
                   nextRoundPorts.push_back(port);
                   nextRoundHostID.push_back(hId);
                 }
               else
               */
                 {
                   connectedHosts.insert(host);
                
                   DServerCreator* c = 
                     new DServerCreator(host,
                                        port, 
                                        name,
                                        inType);
                   DServer* s = c -> createServer();
                   
                   m_serverlist[hId] = s;
                                       
                   exec.execute(c);
                   
                 }

             } //while (!thisRoundPorts.empty)

           exec.wait();
         } // try
 
       catch(ZThread::Synchronization_Exception& e) 
         {
           setErrorText(string("Could not create DServers!\nError:") + 
                        e.what());
           cerr << e.what() << endl;
           m_status = false;
           return;
         }

     } //while(!nextRoundHosts.empty())
   
   m_status = true; //seems to be ok

   for(int i = 0; i< size; i++)
     {
       if (m_serverlist[i] != NULL)
         {
           if (m_serverlist[i]->hasError())
             {
               setErrorText(m_serverlist[i] -> getErrorText());
               m_status = false;
             }
         }
       else
         {
           setErrorText( "Worker not created!");
           m_status = false;
         }
     }

   for (int id = 0; id < size; ++id)
     {
       vector<int> insertList;
       for(int i = id; i<array_size; i+=size)
         insertList.push_back(i);
       m_idIndexMap[id] = insertList;
     }

   //cout << "Done:" << m_watch.diffTimes() << endl;
}

/*

2.2 Destructor

All DServer-Objects controlled by the DServerManager are destoyed.

*/

DServerManager::~DServerManager()
{
   for(int i = 0; i<size; i++)
   {
      if(m_serverlist[i] != 0) 
         m_serverlist[i]->Terminate();
      delete m_serverlist[i];
   }
        
   m_serverlist.clear();
   m_idIndexMap.clear();
}

/*
2.6 checkServers

returns false, if server were not created correctly

*/ 
bool 
DServerManager::checkServers(bool writeError) const
{ 
  if (m_serverlist.size() != (unsigned int)size)
    return false;

  for(int i = 0; i<size; i++)
    {
      if (!m_serverlist[i]->checkServer(writeError))
        {
          return false;
        }
    }
  return true;
}
