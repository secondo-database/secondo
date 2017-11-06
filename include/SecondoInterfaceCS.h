
/*
----
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, 
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


#ifndef SECONDO_INTERFACE_CS_H
#define SECONDO_INTERFACE_CS_H

#include "SecondoInterface.h"
#include "Messages.h"


class SecondoInterfaceCS : public SecondoInterface , MessageHandler {
  
  
public:
  SecondoInterfaceCS(bool isServer, NestedList* _nl, 
                     bool verbose);
  
  ~SecondoInterfaceCS();
  
  virtual bool Initialize( const std::string& user, const std::string& pswd,
                   const std::string& host, const std::string& port,
                   std::string& profile,
                   std::string& errorMsg,
                   const bool multiUser = false );
  
  virtual void Terminate();

  bool reconnect(std::string& errMsg){
    try{
        Terminate();
    } catch(...) {
      std::cerr << "################### " << std::endl;
      std::cerr << "Exception occured during terminate " << std::endl;
      initialized = false;
    }
    return Initialize(user, pswd, secHost, secPort, secConfig, 
                      errMsg, multiUser);
  }
  
  virtual void Secondo( const std::string& commandText,
                const ListExpr commandLE,
                const int commandType,
                const bool commandAsText,
                const bool resultAsText,
                ListExpr& resultList,
                int& errorCode,
                int& errorPos,
                std::string& errorMessage,
                const std::string& resultFileName =
                                "SecondoResult",
                const bool isApplicationLevelCommand = true);
  
  virtual ListExpr NumericTypeExpr( const ListExpr type );
  
  virtual bool GetTypeId( const std::string& name,
                  int& algebraId, int& typeId );
  
  virtual bool LookUpTypeExpr( ListExpr type, std::string& name,
                       int& algebraId, int& typeId );
  
  
  //virtual ListExpr DerivedObjValueExpr();
  
  virtual void SetDebugLevel( const int level );

  bool Secondo( const std::string& cmdText,
                 ListExpr& resultList,
                 SecErrInfo& err,
                 const std::string& resultFileName = "SecondoResult",
                 const bool isApplicationLevelCommand = true        ){
   return SecondoInterface::Secondo(cmdText, resultList, err, 
                                    resultFileName, isApplicationLevelCommand);
  }

  bool Secondo( const ListExpr cmdList,
                 ListExpr& resultList,
                 SecErrInfo& err,
                 const std::string& resultFileName = "SecondoResult",
                 const bool isApplicationLevelCommand = true        ){
     return SecondoInterface::Secondo(cmdList, resultList,err,resultFileName, 
                                      isApplicationLevelCommand);
  }


  
  virtual  bool getOperatorIndexes(
         const std::string OpName,
         ListExpr argList,
         ListExpr& resList,
         int& algId,
         int& opId,
         int& funId,
         NestedList* listStorage);
  
  virtual bool getCosts(const int algId,
              const int opId,
              const int funId,
              const size_t noTuples,
              const size_t sizeOfTuple,
              const size_t noAttributes,
              const double selectivity,
              const size_t memoryMB,
              size_t& costs);
  
  virtual bool getCosts(const int algId,
              const int opId,
              const int funId,
              const size_t noTuples1,
              const size_t sizeOfTuple1,
              const size_t noAttributes1,
              const size_t noTuples2,
              const size_t sizeOfTuple2,
              const size_t noAttributes2,
              const double selectivity,
              const size_t memoryMB,
              size_t& costs);
  
  virtual bool getLinearParams( const int algId,
                      const int opId,
                      const int funId,
                      const size_t noTuples1,
                      const size_t sizeOfTuple1,
                      const size_t noAttributes1,
                      const double selectivity,
                      double& sufficientMemory,
                      double& timeAtSuffMemory,
                      double& timeAt16MB);
  
  virtual bool getLinearParams( const int algId,
                      const int opId,
                      const int funId,
                      const size_t noTuples1,
                      const size_t sizeOfTuple1,
                      const size_t noAttributes1,
                      const size_t noTuples2,
                      const size_t sizeOfTuple2,
                      const size_t noAttributes2,
                      const double selectivity,
                      double& sufficientMemory,
                      double& timeAtSuffMemory,
                      double& timeAt16MB);
  
   virtual bool getFunction(const int algId,
                 const int opId,
                 const int funId,
                 const size_t noTuples,
                 const size_t sizeOfTuple,
                 const size_t noAttributes,
                 const double selectivity,
                 int& funType,
                 double& sufficientMemory,
                 double& timeAtSuffMemory,
                 double& timeAt16MB,
                 double& a, double& b, double&c, double& d);
   
   virtual bool getFunction(const int algId,
                 const int opId,
                 const int funId,
                 const size_t noTuples1,
                 const size_t sizeOfTuple1,
                 const size_t noAttributes1,
                 const size_t noTuples2,
                 const size_t sizeOfTuple2,
                 const size_t noAttributes2,
                 const double selectivity,
                 int& funType,
                 double& sufficientMemory,
                 double& timeAtSuffMemory,
                 double& timeAt16MB,
                 double& a, double& b, double&c, double& d);

   void setMaxAttempts(int a);

   void setTimeout(int t);

   int sendFile(const std::string& localFileName,
                const std::string& serverFileName,
                const bool allowOverwrite);

   int requestFile( const std::string& serverFileName,
                    const std::string& localFileName,
                    const bool allowOverwrite );

   std::string getSendFileFolder();
   std::string getSendFilePath();

   std::string getRequestFileFolder();
   std::string getRequestFilePath();

   bool setHeartbeat(const int heart1, const int heart2);


   virtual int getPid();
   virtual bool cancelQuery(int pid);

   std::string getHome();

   std::string getHost() const;

   std::string getConnectionInfo() const;

   bool connectionOK() const;


   void killConnection();

   void addMessageHandler(MessageHandler* msgH){
      if(!msgH) return;
      for(size_t i=0;i<messageListener.size();i++){
         if(messageListener[i]==msgH){
           return;
         }
      }
      messageListener.push_back(msgH);
   }

   void removeMessageHandler(MessageHandler* msgH){
      std::vector<MessageHandler*>::iterator it;
      for(it=messageListener.begin(); it!=messageListener.end();it++){
         if(*it == msgH){
             messageListener.erase(it);
             return;
         }
      }
   }

   bool handleMsg(NestedList* nl, ListExpr msg, int source);


 protected:
     Socket*     server;     // used in C/S version only
     CSProtocol* csp;

  private:
     int  maxAttempts; // maximum number of attemps when connecting
     int  timeout; // timeout in second per connection attemp 
     int  server_pid;
     bool debugSecondoMethod;
     bool verbose;
     std::string secHost;
     std::string secPort;
     std::string secConfig;
     std::string user;
     std::string pswd;
     bool multiUser;
     std::vector<MessageHandler*> messageListener;

     SecondoInterfaceCS() ;

};

#endif


