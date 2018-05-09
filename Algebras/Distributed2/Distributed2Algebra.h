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

#include "ErrorWriter.h"
#include "DArray.h"

namespace distributed2 {

class ConnectionInfo;
class PProgressView;

/*
5 The Distributed2Algebra


Besides the usual Tasks of an algebra, namely providing types and
operators, this algebra also manages sets of connections. One set is
for user defined connections, the other one contains connections used
in DArrays.

*/

class Distributed2Algebra: public Algebra{

  public:

/*
1.1 Constructor

*/
    Distributed2Algebra();


/*
1.2 Destructor

Closes all open connections and destroys them.

*/
    virtual ~Distributed2Algebra();


/*
~addConnection~

Adds a new connection to the connection pool.

*/
    int addConnection(ConnectionInfo* ci);

/*
~noConnections~

Returns the number of connections

*/

    size_t noConnections();


/*
~getConnection~

Returns a connection

*/

    ConnectionInfo* getConnection(const int i);

/*
~showProgress~

*/
    void showProgress(bool enable);

/*
~noValidConnections~

Returns the number of non-null connections.

*/

    size_t noValidConnections();

/*
~disconnect~

Closes all connections and destroys the instances.
The return value is the number of closed connections.

*/
    int disconnect();

/*
~disconnect~

Disconnects a specified connection and removes it from the
connection pool. The result is 0 or 1 depending whether the
given argument specifies an existing connection.

*/
    int disconnect( unsigned int position);

/*
~isValidServerNo~

checks whether an given integer points to a server

*/
    bool isValidServerNumber(int no);

/*
~serverExists~

*/
    bool serverExists(int s);

/*
~serverPid~

*/

    int serverPid(int s);

/*
~sendFile~

Transfers a local file to a remove server.

*/
    int sendFile(const int con,
                 const std::string& local,
                 const std::string& remote,
                 const bool allowOverwrite);

/*
~requestFile~

This functions copies a remotely stored file into the local
file system.


*/
    int requestFile(const int con,
                    const std::string& remote,
                    const std::string& local,
                    const bool allowOverwrite);

/*
~getRequestFolder~

returns the name of the folder where get requests start.

*/
    std::string getRequestFolder(int con);


/*
~getSendFolder~

returns the name of the folder where new files are stored on remote side.

*/
    std::string getSendFolder( int con);


/*
~initProgress~

initializes the progress view.

*/
    void initProgress();

/*
~finishProgress~

mark the end of the progress view

*/
    void finishProgress();

/*
~getHost~

Returns the host name of the connection with index con.

*/

    std::string getHost(int con);


/*
~getPort~

Returns the port of the connection with index con.

*/
    int getPort(int con);

/*
~getConfig~

Returns the name of the configuration file of the
connection at index ~con~.

*/
    std::string getConfig(int con);

/*
~check~

Tests whether the connection at index ~con~ is alive.

*/
    bool check(int con);

/*
~simpleCommand~

Performs a command at connection with index ~con~.

*/

    bool simpleCommand(int con, const std::string& cmd, int& error,
                       std::string& errMsg, ListExpr& resList,
                       const bool rewrite, double& runtime);

/*
~simpleCommand~

Performs a command returning the result as as string.


*/
    bool simpleCommand(int con, const std::string& cmd, int& error,
                       std::string& errMsg, std::string& resList,
                       const bool rewrite, double& runtime);


/*
The next functions are for interacting with workers, i.e.
connections coming from darray elements.

*/
    ConnectionInfo* getWorkerConnection(const DArrayElement& info,
                                        const std::string& dbname,
                                        CommandLogger* log = 0);

/*
~getDBName~

Returns the database name currently opened at the specified connection.

*/
    std::string getDBName(const DArrayElement& info);

/*
This operator closes all non user defined existing server connections.
It returns the numer of closed workers

*/
    int closeAllWorkers();

/*
The operator ~closeWorker~ closes the connections for a
specified DArrayElement.

*/
    bool closeWorker(const DArrayElement& elem);


/*
~workerConnection~

Checks whether the specified connection exists. If so, the currently
opened database and the ConnectionInfo are returned.

*/
    bool workerConnection(const DArrayElement& elem, std::string& dbname,
                          ConnectionInfo*& ci);

/*
workersIterator

returns an iterator over the available connetions of workers.

*/
    std::map<DArrayElement, std::pair<std::string,ConnectionInfo*> >::iterator
    workersIterator();

/*
~isEnd~

checks for the end of an iterator threadsafe.

*/
  bool isEnd(std::map<DArrayElement,
             std::pair<std::string,ConnectionInfo*> >::iterator& it);


/*
~getTempName~

returns a temporary name for a specified connection.

*/
    std::string getTempName(int server);

/*
~getTempName~

returns a temporary name for a specified DArrayElement.

*/

    std::string getTempName(const DArrayElement& elem);


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


    inline bool tryReconnect() const{
      return tryReconnectFlag;
    }

    inline void setReconnect(const bool enable){
       tryReconnectFlag = enable;
    }


    bool setHeartbeat(int heartbeat);

    inline int getHeartbeat() const{
      return heartbeat;
    }

    bool setTimeout(int _timeout) {
       if(_timeout<0) return false;
       timeout = _timeout;
       return true;
    }

    inline int getTimeout() const{
      return timeout;
    }

    ErrorWriter errorWriter;

    void enableDFS(const std::string& host,
                   const int port);

    void disableDFS(); 


  private:
    // connections managed by the user
    std::vector<ConnectionInfo*> connections;
    boost::mutex mtx;

    // connections managed automatically
    // for darray type
    // the key represents the connection information,
    // the string the used database
    // the ConnctionInfo the connection
    std::map<DArrayElement,
             std::pair<std::string,ConnectionInfo*> > workerconnections;
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
    bool createWorkerConnection(const DArrayElement& worker,
            std::pair<std::string,
            ConnectionInfo*>& res);


    void enableDFS(ConnectionInfo* ci);

    void disableDFS(ConnectionInfo* ci);

};

}
