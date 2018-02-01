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
//[_][\_]

*/
#ifndef ALGEBRAS_DISTRIBUTED2_CONNECTIONINFO_H_
#define ALGEBRAS_DISTRIBUTED2_CONNECTIONINFO_H_

#include "SecondoInterfaceCS.h"
#include "NestedList.h"
#include "CommandLog.h"
#include "CommandLogger.h"



namespace distributed2
{

template<class T>class TimeoutNotifier;
template<class T>class HeartbeatObserver;


/*

0 Class ConnectionInfo

This class represents a connection to a remote Secondo Server.

*/

class ConnectionInfo
{
public:
    ConnectionInfo(const std::string& _host,
                   const int _port,
                   const std::string& _config,
                   SecondoInterfaceCS* _si,
                   NestedList* _mynl,
                   const size_t timeout = 0,
                   const int heartbeat=0);

    virtual ~ConnectionInfo();

    bool reconnect(bool showCommands, CommandLog& log, const size_t timeout=0,
                   const int heartbeat=0);

    std::string getHost() const;

    int getPort() const;

    std::string getConfig() const;

    bool check(bool showCommands, CommandLog& commandLog,
               const size_t timeout=0);

    void setId(const int i);

    void simpleCommand(std::string command1,
                       int& err,
                       std::string& result,
                       bool rewrite,
                       double& runtime,
                       bool showCommands,
                       CommandLog& commandLog,
                       bool forceExec = false,
                       const size_t timeout=0);

    std::string getSecondoHome(bool showCommands,
                               CommandLog& commandLog);

    bool cleanUp(bool showCommands,
                 CommandLog& commandLog,
                 const size_t timeout=0);

    bool cleanUp1( const size_t timeout=0);

    bool switchDatabase(const std::string& dbname,
                        bool createifnotexists,
                        bool showCommands,
                        bool forceExec = false,
                        const size_t timeout=0);

    void simpleCommand(const std::string& command1,
                       int& error,
                       std::string& errMsg,
                       std::string& resList,
                       const bool rewrite,
                       double& runtime,
                       bool showCommands,
                       CommandLog& commandLog,
                       bool forceExec = false,
                       const size_t timeout=0);

    void simpleCommandFromList(const std::string& command1,
                               int& error,
                               std::string& errMsg,
                               std::string& resList,
                               const bool rewrite,
                               double& runtime,
                               bool showCommands,
                               CommandLog& commandLog,
                               bool forceExec = false,
                               const int timeout = 0);

    void simpleCommand(const std::string& command1,
                       int& error,
                       std::string& errMsg,
                       ListExpr& resList,
                       const bool rewrite,
                       double& runtime,
                       bool showCommands,
                       CommandLog& commandLog,
                       bool forceExec = false,
                       const size_t timeout=0);

    int serverPid();

    int sendFile(const std::string& local,
                 const std::string& remote,
                 const bool allowOverwrite,
                 const size_t timeout=0);

    int requestFile(const std::string& remote,
                    const std::string& local,
                    const bool allowOverwrite,
                    const size_t timeout=0);

    std::string getRequestFolder();

    std::string getSendFolder();

    std::string getSendPath();

    static ConnectionInfo* createConnection(const std::string& host,
                                            const int port,
                                            std::string& config,
                                            const size_t timeout=0,
                                            const int heartbeat=0);

    bool createOrUpdateObject(const std::string& name,
                              ListExpr typelist,
                              Word& value,
                              bool showCommands,
                              CommandLog& commandLog,
                              bool forceExec = false,
                              const size_t timeout=0);

    bool createOrUpdateRelation(const std::string& name,
                                ListExpr typeList,
                                Word& value,
                                bool showCommands,
                                CommandLog& commandLog,
                                bool forceExec = false,
                                const size_t timeout=0);

    bool createOrUpdateRelationFromBinFile(const std::string& name,
                                           const std::string& filename,
                                           bool showCommands,
                                           CommandLog& commandLog,
                                           const bool allowOverwrite = true,
                                           bool forceExec = false,
                                           const size_t timeout=0);

    bool createOrUpdateAttributeFromBinFile(const std::string& name,
                                            const std::string& filename,
                                            bool showCommands,
                                            CommandLog& commandLog,
                                            const bool allowOverwrite = true,
                                            bool forceExec = false,
                                            const size_t timeout=0);

    bool saveRelationToFile(ListExpr relType,
                            Word& value,
                            const std::string& filename);

    bool saveAttributeToFile(ListExpr type,
                             Word& value,
                             const std::string& filename);

    bool storeObjectToFile(const std::string& objName,
                           Word& value,
                           ListExpr typeList,
                           const std::string& fileName);

    bool retrieve(const std::string& objName,
                  ListExpr& resType,
                  Word& result,
                  bool checkType,
                  bool showCommands,
                  CommandLog& commandLog,
                  bool forceExec = false,
                  const size_t timeout=0);

    bool retrieveRelation(const std::string& objName,
                          ListExpr& resType,
                          Word& result,
                          bool showCommands,
                          CommandLog& commandLog,
                          bool forceExec = false,
                          const size_t timeout=0);

    bool retrieveRelationInFile(const std::string& fileName,
                                ListExpr& resType,
                                Word& result,
                                bool showCommands,
                                CommandLog& commandLog,
                                bool forceExec = false,
                                const size_t timeout=0);

    bool retrieveRelationFile(const std::string& objName,
                              const std::string& fname1,
                              bool showCommands,
                              CommandLog& commandLog,
                              bool forceExec = false,
                              const size_t timeout=0);

    bool retrieveAnyFile(const std::string& remoteName,
                         const std::string& localName,
                         bool showCommands,
                         CommandLog& commandLog,
                         bool forceExec = false,
                         const size_t timeout=0);

    Word createRelationFromFile(const std::string& fname, ListExpr& resType);

    std::ostream& print(std::ostream& o) const;


    void setLogger(CommandLogger* cmdlog){
      this->cmdLog = cmdlog;
    }
    

    CommandLogger* getLogger() const{
      return cmdLog;
    }

    void setNum(const int num){
       this->num = num;
    }

    int getNum() const{
      return num;
    }

    void killConnection();

    void timeout();

    bool setHeartbeat(const int hb1, const int hb2){
        if(hb1<0 || hb2<0) return false;
        if(!si) return false;
        return si->setHeartbeat(hb1,hb2);
    }


private:
    void retrieveSecondoHome(bool showCommands,
                             CommandLog& commandLog);

    void retrieveSecondoHome();

    std::string host;
    int port;
    std::string config;
    SecondoInterfaceCS* si;
    NestedList* mynl;
    int serverPID;
    std::string secondoHome;
    std::string requestFolder;
    std::string requestPath;
    std::string sendFolder;
    std::string sendPath;

    typedef boost::recursive_mutex mutex_type;
    typedef boost::lock_guard<mutex_type> guard_type;

    mutex_type simtx; // mutex for synchronizing 
                                  // access to the interface
    CommandLogger* cmdLog;  // if this is nor null, commands are
                         // written to log instead of sending
                         // to the server
    int num; // some number that can be used to store additional information


    HeartbeatObserver<ConnectionInfo>* hbobserver;
    TimeoutNotifier<ConnectionInfo>* tonotifier;


    void startTimeout(int seconds, bool acceptMessages);
    void stopTimeout(const bool msg);

};

std::ostream& operator<<(std::ostream& o, const ConnectionInfo& sc);

/*
0.16 ~showError~

*/

void showError(const ConnectionInfo* ci, const std::string& command ,
               const int errorCode, const std::string& errorMessage);

void showError(const SecondoInterfaceCS* ci, const std::string& command ,
               const int errorCode, const std::string& errorMessage);

} /* namespace distributed2 */

#endif /* ALGEBRAS_DISTRIBUTED2_CONNECTIONINFO_H_ */
