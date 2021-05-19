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

#pragma once


#include "Algebras/Distributed2/ErrorWriter.h"
#include "Algebras/Distributed2/DArray.h"

#define DPROGRESS

#ifdef DPROGRESS
#include "Algebras/Distributed2/ProgressObserver.h"
#endif


namespace distributed3 {

class ConnectionInfo;
class PProgressView;



/*
5 The Distributed3Algebra


Besides the usual Tasks of an algebra, namely providing types and
operators, this algebra also manages sets of connections. One set is
for user defined connections, the other one contains connections used
in DArrays.

*/

class Distributed3Algebra: public Algebra{

  public:

/*
1.1 Constructor

*/
    Distributed3Algebra();


/*
1.2 Destructor

Closes all open connections and destroys them.

*/
    virtual ~Distributed3Algebra();




    distributed2::ConnectionInfo* getWorkerConnection(
                                        const distributed2::DArrayElement& info,
                                        const std::string& dbname,
                                        distributed2::CommandLogger* log = 0);

  template<class A>
  distributed2::ConnectionInfo* getWorkerConnection(
       A* array,
       int slot,
       const std::string& dbname,
       distributed2::CommandLogger* log,
       bool allowArrayChange);

/*
This operator closes all non user defined existing server connections.
It returns the numer of closed workers

*/
    int closeAllWorkers();






    std::string getTempName(int server);

/*
~getTempName~

returns a temporary name for a specified DArrayElement.

*/

    std::string getTempName(const distributed2::DArrayElement& elem);


/*
~getTempName~

returns some temporary name.

*/
    std::string getTempName();

/*
~cleanUp~

removes temporary files and objects of all opened
connections.

*/
    void cleanUp();

    inline int getTimeout() const{
      return timeout;
    }

    distributed2::ErrorWriter errorWriter;

    void enableDFS(const std::string& host,
                   const int port);

    void disableDFS();
    
    static Distributed3Algebra* getAlgebra();
    static void setAlgebra(Distributed3Algebra* algebra);

#ifdef DPROGRESS
    ProgressObserver* progressObserver;
#endif


  private:
    // connections managed by the user
    std::vector<distributed2::ConnectionInfo*> connections;
    boost::mutex mtx;

    // connections managed automatically
    // for darray type
    // the key represents the connection information,
    // the string the used database
    // the ConnctionInfo the connection
    std::map<distributed2::DArrayElement,
       std::pair<std::string,distributed2::ConnectionInfo*> > workerconnections;
       boost::mutex workerMtx;

    size_t namecounter;
    boost::mutex namecounteraccess;

    PProgressView* pprogView;

    bool tryReconnectFlag;

    int heartbeat;
    int timeout;

    std::string dfshost;
    int dfsport;


    // returns a unique number
    size_t nextNameNumber();

    // tries to create a new connection to a worker
    bool createWorkerConnection(const distributed2::DArrayElement& worker,
            std::pair<std::string,
            distributed2::ConnectionInfo*>& res);


    void enableDFS(distributed2::ConnectionInfo* ci);

    //void disableDFS(distributed2::ConnectionInfo* ci);
    
    static Distributed3Algebra* algebra;

};

}
