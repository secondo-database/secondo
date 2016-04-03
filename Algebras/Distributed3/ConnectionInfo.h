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

#ifndef CONNECTIONINFO_H
#define CONNECTIONINFO_H

//#include "semaphore.h"

#include "fsrel.h"
#include "DArray.h"
#include "frel.h"
#include <iostream>
#include <vector>
#include <list>

#include "SecondoInterface.h"
#include "SecondoInterfaceCS.h"
#include "FileSystem.h"
#include "Algebra.h"
#include "NestedList.h"
#include "StandardTypes.h"
#include "FTextAlgebra.h"
#include "ListUtils.h"
#include "StringUtils.h"
#include "RelationAlgebra.h"
#include "Stream.h"
#include "NList.h"
#include "ArrayAlgebra.h"
#include "SocketIO.h"
#include "StopWatch.h"

#include "Bash.h"
#include "DebugWriter.h"


#include "FileRelations.h"
#include "FileAttribute.h"


  // use boost for thread handling

#include <boost/thread.hpp>
#include <boost/date_time.hpp>


using namespace std;

namespace distributed3 {

class ConnectionInfo{

   public:
     ConnectionInfo(const string& _host, const int _port,
                    const string& _config, SecondoInterfaceCS* _si,
                    NestedList* _mynl);
      
      ~ConnectionInfo();
      
       string getHost() const;

       int getPort() const;
       
       string getConfig() const;
    
       bool check();

       void setId(const int i);

       void simpleCommand(string command1, int& err, string& result, 
                          bool rewrite, double& runtime);

       string getSecondoHome();

       bool cleanUp();
      
       bool switchDatabase(const string& dbname, bool createifnotexists);
       
       void simpleCommand(const string& command1, int& error, string& errMsg, 
                          string& resList, const bool rewrite, double& runtime);
       
       void simpleCommandFromList(const string& command1, int& error, 
                                  string& errMsg, string& resList, 
                                  const bool rewrite, double& runtime);
       
       void simpleCommand(const string& command1, int& error, string& errMsg, 
                          ListExpr& resList, const bool rewrite,
                          double& runtime);

       int serverPid();

        int sendFile( const string& local, const string& remote, 
                      const bool allowOverwrite);

        int requestFile( const string& remote, const string& local,
                         const bool allowOverwrite);

        string getRequestFolder();
        
        string getSendFolder();
        
        string getSendPath();
        
        static ConnectionInfo* createConnection(const string& host, 
                                   const int port, string& config);

         bool createOrUpdateObject(const string& name, 
                                   ListExpr typelist, Word& value);
         
         bool createOrUpdateRelation(const string& name, ListExpr typeList,
                                     Word& value);

         bool createOrUpdateRelationFromBinFile(const string& name, 
                                                const string& filename,
                                                const bool allowOverwrite=true);

         bool createOrUpdateAttributeFromBinFile(const string& name, 
                                                const string& filename,
                                                const bool allowOverwrite=true);

         bool saveRelationToFile(ListExpr relType, Word& value, 
                                 const string& filename);

         bool saveAttributeToFile(ListExpr type, Word& value, 
                                 const string& filename);
         bool storeObjectToFile( const string& objName, Word& value, 
                                 ListExpr typeList, 
                                  const string& fileName);

          bool retrieve(const string& objName, ListExpr& resType, Word& result, 
                        bool checkType);

          bool retrieveRelation(const string& objName, 
                                 ListExpr& resType, Word& result
                                );
          
          bool retrieveRelationInFile(const string& fileName,
                                      ListExpr& resType, 
                                      Word& result);
          
          bool retrieveRelationFile(const string& objName,
                                    const string& fname1);

          bool retrieveAnyFile(const string& remoteName,
                            const string& localName);
          
          Word createRelationFromFile(const string& fname, ListExpr& resType);
  
  private:
    string host;
    int port;
    string config;
    SecondoInterfaceCS* si;
    NestedList* mynl;
    int serverPID;
    string secondoHome;
    string requestFolder;
    string sendFolder;
    string sendPath;

    boost::recursive_mutex simtx; 
                // mutex for synchronizing access to the interface



    void retrieveSecondoHome(){
       string cmd ="query secondoHome()";
       int err;
       string errmsg;
       ListExpr result; 
       double rt;
       simpleCommand(cmd,err,errmsg, result,false,rt);
       if(err!=0){
            cerr << "command " << cmd << " failed" << endl;
            cerr << err << " : " << errmsg << endl;
            return;
       }
       if(    !nl->HasLength(result,2) 
            || nl->AtomType(nl->Second(result))!=TextType){
          cerr << "invalid result for secondoHome() query " << endl;
          cerr << nl->ToString(result);
          return;
       }
       secondoHome =  nl->Text2String(nl->Second(result));
    }
};
}

#endif