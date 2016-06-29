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


#include "semaphore.h"

#include "fsrel.h"
#include "DArray.h"
#include "frel.h"
#include "fobj.h"
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


extern DebugWriter dwriter;

extern boost::mutex nlparsemtx;


using namespace std;

namespace distributed2 {

/*
0 Workaround to the nested list parser

The nested list parser implementation used a lot of global variables.
This makes it impossible to create a nested list from a file or a 
string in parallel. Thus, we introduce a global mutex which is locked
always if a nested list is parsed.

*/
boost::mutex createRelMut;
boost::mutex copylistmutex;




bool showCommands;
boost::mutex showCommandMtx;

void showCommand(SecondoInterfaceCS* src, const string& host, const int port, 
                 const string& cmd, bool start){

   if(showCommands){
      dwriter.write(showCommands,cout, src, src->getPid(), "= " + host + ":" 
              + stringutils::int2str(port)+ ":" + (start?"start ":"finish ") 
              + cmd);
   }

}

class ConnectionInfo;
void showError(const ConnectionInfo* ci, const string& command , 
               const int errorCode, const string& errorMessage);

void showError(const SecondoInterfaceCS* ci, const string& command , 
               const int errorCode, const string& errorMessage);

/*
Some Helper functions.

*/

string getUDRelType(ListExpr r){

  assert(Relation::checkType(r) || frel::checkType(r));
  ListExpr attrList = nl->Second(nl->Second(r));
  string rt = nl->SymbolValue(nl->First(r));
  string res = rt+"(tuple([";
  bool first = true;
  while(!nl->IsEmpty(attrList)){
    if(!first){
      res += ", ";
    } else {
      first = false;
    }
    ListExpr attr = nl->First(attrList);
    attrList = nl->Rest(attrList);
    res += nl->SymbolValue(nl->First(attr));
    res += " : " + nl->ToString(nl->Second(attr));
  }
  res +="]))";
  return res;
}




/*
1.0.1 ~rewriteQuery~

This function replaces occurences of [$]<Ident> within the string orig
by corresponding const expressions. If one Ident does not represent
a valid object name, the result will be false and the string is
unchanged.

*/

string rewriteRelType(ListExpr relType){
  if(!Relation::checkType(relType)){
     return nl->ToString(relType);
  }

  ListExpr attrList = nl->Second(nl->Second(relType));

  stringstream ss;
  ss << "rel(tuple([";
  bool first = true;
  while(!nl->IsEmpty(attrList)){
     if(!first){
       ss << ", ";
     } else {
        first = false;
     }
     ListExpr attr = nl->First(attrList);
     attrList = nl->Rest(attrList);
     ss << nl->SymbolValue(nl->First(attr));
     ss << " : ";
     ss << nl->ToString(nl->Second(attr));
  }
  ss << "]))";
  return ss.str();
}

/*
The next function returns a constant expression for an 
database object. 

*/

bool getConstEx(const string& objName, string& result){

   SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
   if(!ctlg->IsObjectName(objName)){
       return false;
   }
   result = " [const " + rewriteRelType(ctlg->GetObjectTypeExpr(objName))
          + " value " + nl->ToString(ctlg->GetObjectValue(objName)) + "] ";

   return true;
}


template<class T>
void print(vector<T>& v, ostream& out){
  for(size_t i=0;i<v.size();i++){
     out << v[i] << " ";
  }
}



/*
rewrites a query. Replaces dollar signs by numbers.

*/
bool rewriteQuery(const string& orig, string& result){

  stringstream out;
  size_t pos = 0;
  size_t f = orig.find_first_of("$", pos);
  map<string,string> used;
  while(f!=string::npos){
     size_t f2 = f +1;
     if(f2 < orig.length()){
        if(stringutils::isLetter(orig[f2])){
           if(f>pos){
              out << orig.substr(pos , f - pos);
           }
           stringstream ident;
           ident << orig[f2];
           f2++;
           while(f2<orig.length() && (stringutils::isLetter(orig[f2]) || 
                 stringutils::isDigit(orig[f2] || (orig[f2]=='_')))){
              ident  << orig[f2];
              f2++;
           }
           string constEx;
           if(used.find(ident.str())!=used.end()){
             constEx = used[ident.str()];
           }  else {
              if(!getConstEx(ident.str(),constEx)){
                  result =  orig;
                  return false;
              }
           }
           out << constEx; 
           pos = f2;
        } else if(orig[f2]=='$'){ // masked dollar
            out <<  orig.substr(pos,f2-pos);
            pos = f2+1;
        } else { // not a indent after $
            out <<  orig.substr(pos,f2+1-pos);
            pos = f2;
           pos++;
        } 
     } else { // end of 
       out << orig.substr(pos,string::npos);
       pos = f2;
     }
     f = orig.find_first_of("$", pos);
  }
   out << orig.substr(pos,string::npos);
   result = out.str();
  return true;
}

/*
1 Class Connection

This class represents a connection to a remote Secondo Server.

*/

class ConnectionInfo{

   public:

/*
1.1 Constructor

Creates a new connection instance. 

*/
     ConnectionInfo(const string& _host, const int _port,
                    const string& _config, SecondoInterfaceCS* _si,
                    NestedList* _mynl):
      host(_host), port(_port), config(_config), si(_si){
          mynl = _mynl;
          serverPID = 0;
          secondoHome = "";
          requestFolder ="";
          sendFolder="";
          sendPath="";
      }


/*
1.2 Destructor

Terminates the connection and destoys the object.

*/
      ~ConnectionInfo(){
          {
             boost::lock_guard<boost::recursive_mutex> guard(simtx);
             si->Terminate();
          }
          delete si;
          si = 0;
          delete mynl;
       }

/*
1.3 ~getHost~

returns the remote host.

*/
       string getHost() const{
          return host;
       }

/*
1.4 getPort

*/
       int getPort() const{
          return port;
       }

/*
1.5 getConfig

Returns the configuration file used for the client. 
Has nothing to do with the configuration file used by the
remove monitor.

*/
       string getConfig() const{
          return config;
       }
    
/*
1.6 check

Checks whether the remote server is working by sending a simple command.

*/
       bool check() {
          ListExpr res;
          string cmd = "list databases";
          SecErrInfo err;
          {
             boost::lock_guard<boost::recursive_mutex> guard(simtx);
             showCommand(si,host,port,cmd, true);
             si->Secondo(cmd,res,err);
             showCommand(si,host,port,cmd, false);
          }  
          return err.code==0;
       }

/*
1.7 setId

Sets the id.

*/
        void setId(const int i){
            if(si){
               si->setId(i);;
            }
        }


/*
1.8 simpleCommand

Performs a command on the remote server. The result is stored as a string.


*/
       void simpleCommand(string command1, int& err, string& result, 
                          bool rewrite, double& runtime){
          string command;
          if(rewrite){
            rewriteQuery(command1,command);
          } else {
            command = command1;
          }
          SecErrInfo serr;
          ListExpr resList;
          {
             boost::lock_guard<boost::recursive_mutex> guard(simtx);
             StopWatch sw;
             showCommand(si,host,port,command, true);
             si->Secondo(command, resList, serr);
             showCommand(si,host,port,command, false);
             runtime = sw.diffSecondsReal();
             err = serr.code;
             if(err==0){
                result = mynl->ToString(resList);
             } else {
                result = si->GetErrorMessage(err);
             }
          }
       }


/*
1.9 getSecondoHome

Returns the path of the secondo databases of the remote server.

*/
       string getSecondoHome(){
         if(secondoHome.length()==0){
            retrieveSecondoHome();
         }
         return secondoHome;
       }

/*
1.10 cleanUp

This command removes temporal objects on the remote server.
Such objects having names starting with TMP[_]. Furthermore,
files starting with TMP are removed within the dfarray directory
of the remote server.

*/

      bool cleanUp() {
          // first step : remove database objects
          string command = "query getcatalog() "
                           "filter[.ObjectName startsWith \"TMP_\"] "
                           "extend[OK : deleteObject(.ObjectName)] "
                           " count";
          int err;
          string res;
          double rt;
          simpleCommand(command, err, res, false, rt);
          bool result = err==0;
          string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
          string path = getSecondoHome() +"/dfarrays/" + dbname + "/";
          command = "query getDirectory('" + path +"') "
                    "filter[basename(.) startsWith \"TMP_\"] "
                    "namedtransformstream[F] "
                    "extend[ OK : removeDirectory(.F, TRUE)] count";
          simpleCommand(command, err, res, false, rt);
          result = result && (err==0);
          return result;
      }


/*
1.11 switchDatabase

Switches the remote server to another database.

*/
       bool switchDatabase(const string& dbname, bool createifnotexists){
          boost::lock_guard<boost::recursive_mutex> guard(simtx);;
          // close database ignore errors
          SecErrInfo serr;
          ListExpr resList;
          string cmd = "close database";
          showCommand(si,host,port,cmd, true);
          si->Secondo(cmd, resList, serr);
          showCommand(si,host,port,cmd, false);
          // create database ignore errors
          if(createifnotexists){
              cmd = "create database " + dbname;
              showCommand(si,host,port,cmd, true);
              si->Secondo(cmd, resList, serr);
              showCommand(si,host,port,cmd, false);
          }
          // open database 
          cmd = "open database "+ dbname;
          showCommand(si,host,port,cmd, true);
          si->Secondo(cmd, resList, serr);
          showCommand(si,host,port,cmd, false);
          bool res = serr.code==0;   
          return res;
       }


/*
1.12 simpleCommand

This variant of ~simpleCommand~ returns the result as a list.


*/
       void simpleCommand(const string& command1, int& error, string& errMsg, 
                          string& resList, const bool rewrite, double& runtime){

          string command;
          if(rewrite){
            rewriteQuery(command1,command);
          } else {
            command = command1;
          }
          {
             boost::lock_guard<boost::recursive_mutex> guard(simtx);;
             SecErrInfo serr;
             ListExpr myResList = mynl->TheEmptyList();
             StopWatch sw;
             showCommand(si,host,port,command,true);
             si->Secondo(command, myResList, serr);
             showCommand(si,host,port,command,false);
             runtime = sw.diffSecondsReal();
             error = serr.code;
             errMsg = serr.msg;

             resList = mynl->ToString(myResList);
             mynl->Destroy(myResList);
          }
       }
       

/*
1.13 simpleCommandFromList

Performs a command that is given in nested list format.

*/
       void simpleCommandFromList(const string& command1, int& error, 
                                  string& errMsg, string& resList, 
                                  const bool rewrite, double& runtime){

          string command;
          if(rewrite){
            rewriteQuery(command1,command);
          } else {
            command = command1;
          }
          boost::lock_guard<boost::recursive_mutex> guard(simtx);
          ListExpr cmd = mynl->TheEmptyList();
          {
              boost::lock_guard<boost::mutex> guard(nlparsemtx);
              if(! mynl->ReadFromString(command,cmd)){
                 error = 3;
                 errMsg = "error in parsing list";
                 return;
              }
          }
          SecErrInfo serr;
          ListExpr myResList = mynl->TheEmptyList();
          StopWatch sw;
          showCommand(si,host,port,command, true);
          si->Secondo(cmd, myResList, serr);
          showCommand(si,host,port,command, false);
          runtime = sw.diffSecondsReal();
          error = serr.code;
          errMsg = serr.msg;

          resList = mynl->ToString(myResList);
          if(mynl->AtomType(cmd)!=NoAtom && !mynl->IsEmpty(cmd)){
              mynl->Destroy(cmd);
          }
          if(mynl->AtomType(myResList)!=NoAtom && !mynl->IsEmpty(myResList)){
              mynl->Destroy(myResList);
          }
          
          
       }


/*
1.13 simpleCommand

This variant provides the result as a list.

*/

       void simpleCommand(const string& command1, int& error, string& errMsg, 
                          ListExpr& resList, const bool rewrite,
                          double& runtime){


          string command;
          if(rewrite){
            rewriteQuery(command1,command);
          } else {
            command = command1;
          }
          boost::lock_guard<boost::recursive_mutex> guard(simtx);;
          SecErrInfo serr;
          ListExpr myResList = mynl->TheEmptyList();
          StopWatch sw;
          showCommand(si,host,port,command, true);
          si->Secondo(command, myResList, serr);
          showCommand(si,host,port,command, false);
          runtime = sw.diffSecondsReal();
          error = serr.code;
          errMsg = serr.msg;
          // copy resultlist from local nested list to global nested list
          {
              boost::lock_guard<boost::mutex> guard(copylistmutex);   
              assert(mynl!=nl);
              resList =  mynl->CopyList(myResList, nl);
              mynl->Destroy(myResList);
          }
       }


/*
1.14 serverPid

returns the process id of the remote server

*/
        int serverPid(){
           boost::lock_guard<boost::recursive_mutex> guard(simtx);
           if(serverPID==0){
             serverPID = si->getPid(); 
           }
           return serverPID;
        }


/*
1.15 sendFile

transfers a locally stored file to the remote server. 
It returns an error code.

*/
        int sendFile( const string& local, const string& remote, 
                      const bool allowOverwrite){
          boost::lock_guard<boost::recursive_mutex> guard(simtx);   
          int res =  si->sendFile(local,remote, allowOverwrite);
          return res;
        }
        

/*
1.16 requestFile

Transfers a remotely stored file to the local file system.

*/
        int requestFile( const string& remote, const string& local,
                         const bool allowOverwrite){
          boost::lock_guard<boost::recursive_mutex> guard(simtx);   
          int res =  si->requestFile(remote, local, allowOverwrite);
          return res;
        }

/*
1.17 getRequestFolder

Returns the path on remote machine for requesting files.

*/
        string getRequestFolder(){
          if(requestFolder.length()==0){
             boost::lock_guard<boost::recursive_mutex> guard(simtx);   
             requestFolder =  si->getRequestFileFolder();
          }
          return requestFolder;
        }
        

/*
1.18 getSendFolder

returns the folder the remote machine where files are stored.
This folder is a subdirectory of the request folder.

*/
        string getSendFolder(){
          if(sendFolder.length()==0){
             boost::lock_guard<boost::recursive_mutex> guard(simtx);   
             sendFolder =  si->getSendFileFolder();
          }
          return sendFolder;
        }

/*
1.19 getSendPath

Returns the complete path where files are stored.

*/
        
        string getSendPath(){
          if(sendPath.length()==0){
             boost::lock_guard<boost::recursive_mutex> guard(simtx);   
             sendPath =  si->getSendFilePath();
          }
          return sendPath;
        }


/*
1.20 Factory function

*/
        static ConnectionInfo* createConnection(const string& host, 
                                   const int port, string& config){

              NestedList* mynl = new NestedList();
              SecondoInterfaceCS* si = new SecondoInterfaceCS(true,mynl,true);
              string user="";
              string passwd = "";
              string errMsg;
              si->setMaxAttempts(4);
              si->setTimeout(1);
              if(! si->Initialize(user, passwd, host, 
                       stringutils::int2str(port), config, 
                       errMsg, true)){
                  delete si;
                  si = 0;
                  delete mynl;
                  return 0;
             } else {
                  return  new ConnectionInfo(host, port, config, si, mynl);
             }
         }

/*
1.21 createOrUpdateObject

creates a new object or updates an existing one on remote server.

*/
         bool createOrUpdateObject(const string& name, 
                                   ListExpr typelist, Word& value){
              if(Relation::checkType(typelist)){
                 return createOrUpdateRelation(name, typelist, value);
              }

              boost::lock_guard<boost::recursive_mutex> guard(simtx);
              SecErrInfo serr;
              ListExpr resList;
              string cmd = "delete " + name;
              showCommand(si,host,port,cmd,true);
              si->Secondo(cmd, resList, serr);
              showCommand(si,host,port,cmd,false);
              // ignore error (object must not exist)
              string filename = name + "_" + 
                                stringutils::int2str(WinUnix::getpid())
                                 + ".obj";
              storeObjectToFile(name, value, typelist, filename);
              cmd = "restore " + name + " from '" + filename + "'";
              showCommand(si,host,port,cmd,true);
              si->Secondo(cmd, resList, serr);
              showCommand(si,host,port,cmd,false);
              FileSystem::DeleteFileOrFolder(filename);
              return serr.code==0;
         }

/*
1.22 createOrUpdateRelation

Accelerated version for relations.

*/
         bool createOrUpdateRelation(const string& name, ListExpr typeList,
                                     Word& value){

             // write relation to a file
             string filename = name + "_" 
                               + stringutils::int2str(WinUnix::getpid()) 
                               + ".bin";
             if(!saveRelationToFile(typeList, value, filename)){
                return false;
             }
             // restore remote relation from local file
             bool ok = createOrUpdateRelationFromBinFile(name,filename); 
             // remove temporarly file
             FileSystem::DeleteFileOrFolder(filename);
             return ok;
         }

/*
1.23 createOrUpdateRelationFromBinFile

Creates a new relation or updates an existing one on remote server from
local file.

*/

         bool createOrUpdateRelationFromBinFile(const string& name, 
                                                const string& filename,
                                                const bool allowOverwrite=true){
             boost::lock_guard<boost::recursive_mutex> guard(simtx);

             SecErrInfo serr;
             ListExpr resList;
             // transfer file to remote server
             int error = si->sendFile(filename,filename,true);
             if(error!=0){
                return false;
             } 

             // retrieve folder from which the filename can be read
             string sendFolder = si->getSendFilePath();
             
             string rfilename = sendFolder+"/"+filename;
             // delete existing object

             string cmd = "delete " + name;
             if(allowOverwrite){
                showCommand(si,host,port,cmd,true);
                si->Secondo(cmd, resList, serr);
                showCommand(si,host,port,cmd,false);
             }
       
             cmd = "let " + name + " =  '" + rfilename +"' ffeed5 consume ";

             showCommand(si,host,port,cmd,true);
             si->Secondo(cmd, resList, serr);
             showCommand(si,host,port,cmd,false);

             bool ok = serr.code == 0;


             cmd = "query removeFile('"+rfilename+"')";

             si->Secondo (cmd,resList,serr);


             return ok;
         }
         
/*
1.24 createOrUpdateAttributeFromBinFile

Creates an new attribute object or updates an existing one on remote server 
from a local file.

*/
         bool createOrUpdateAttributeFromBinFile(const string& name, 
                                                const string& filename,
                                                const bool allowOverwrite=true){
             boost::lock_guard<boost::recursive_mutex> guard(simtx);

             SecErrInfo serr;
             ListExpr resList;
             // transfer file to remote server
             int error = si->sendFile(filename,filename,true);
             if(error!=0){
                return false;
             } 

             // retrieve folder from which the filename can be read
             string sendFolder = si->getSendFilePath();
             
             string rfilename = sendFolder+"/"+filename;
             // delete existing object

             string cmd = "delete " + name;
             if(allowOverwrite){
                showCommand(si,host,port,cmd,true);
                si->Secondo(cmd, resList, serr);
                showCommand(si,host,port,cmd,false);
             }
       
             cmd = "let " + name + " =  loadAttr('" 
                          + rfilename + "') ";

             showCommand(si,host,port,cmd,true);
             si->Secondo(cmd, resList, serr);
             showCommand(si,host,port,cmd,false);

             bool ok = serr.code == 0;

             cmd = "query removeFile('"+rfilename+"')";

             si->Secondo (cmd,resList,serr);

             return ok;
         }


/*
1.26 saveRelationToFile

Stores a local relation info a binary local file.

*/
         bool saveRelationToFile(ListExpr relType, Word& value, 
                                 const string& filename){
            ofstream out(filename.c_str(),ios::out|ios::binary);
            char* buffer = new char[FILE_BUFFER_SIZE];
            out.rdbuf()->pubsetbuf(buffer, FILE_BUFFER_SIZE);
            if(!out.good()){
               delete[] buffer;
               return false;
            }
            if(!BinRelWriter::writeHeader(out,relType)){
               delete[] buffer;
               return false;
            }
            Relation* rel = (Relation*) value.addr;
            GenericRelationIterator* it = rel->MakeScan();
            bool ok = true;
            Tuple* tuple=0;
            while(ok && ((tuple=it->GetNextTuple())!=0)){
               ok = BinRelWriter::writeNextTuple(out,tuple);
               tuple->DeleteIfAllowed();
            }
            delete it;
            BinRelWriter::finish(out);
            out.close();
            delete[] buffer;
            return ok;
         }

/*
1.27 saveAttributeToFile

saves an attribute to a local file.

*/
         bool saveAttributeToFile(ListExpr type, Word& value, 
                                 const string& filename){
            Attribute* attr = (Attribute*) value.addr;
            return FileAttribute::saveAttribute(type, attr, filename);
         }

/*
1.28 storeObjectToFile

Stores any object to a file using its nested list represnetation.

*/

         bool storeObjectToFile( const string& objName, Word& value, 
                                 ListExpr typeList, 
                                  const string& fileName){
               SecondoCatalog* ctl = SecondoSystem::GetCatalog();
               ListExpr valueList = ctl->OutObject(typeList, value);
               ListExpr objList = nl->FiveElemList(
                                     nl->SymbolAtom("OBJECT"),
                                     nl->SymbolAtom(objName),
                                     nl->TheEmptyList(),
                                     typeList,
                                     valueList);
              return nl->WriteToFile(fileName, objList);
         }        


/*
1.29 retrieve

Retrieves a remote object.

*/

          bool retrieve(const string& objName, ListExpr& resType, Word& result, 
                        bool checkType){
              boost::lock_guard<boost::recursive_mutex> guard(simtx);
              if(Relation::checkType(resType)){
                  if(retrieveRelation(objName, resType, result)){
                     return true;
                  } 
                  cerr << "Could not use fast retrieval for a "
                       << " relation, failback" << endl;
              }

              SecErrInfo serr;
              ListExpr myResList;
              string cmd = "query " + objName;
              showCommand(si,host,port,cmd,true);
              si->Secondo(cmd, myResList, serr);
              showCommand(si,host,port,cmd,false);
              SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
              if(serr.code!=0){
                 return false;
              }
              if(!mynl->HasLength(myResList,2)){
                return false;
              }
              // copy result list into global list memory
              ListExpr resList;
              {
                 boost::lock_guard<boost::mutex> guard(copylistmutex);
                 resList =  mynl->CopyList(myResList, nl);
                 mynl->Destroy(myResList);
              }
              ListExpr resType2 = nl->First(resList);
              if(checkType && !nl->Equal(resType,resType2)){
                 // other type than expected
                 return false;
              }
              ListExpr value = nl->Second(resList);

              int errorPos=0;
              ListExpr errorInfo = listutils::emptyErrorInfo();
              bool correct;
              {
                 boost::lock_guard<boost::mutex> guard(createRelMut);
                 result = ctlg->InObject(resType,value, errorPos, 
                                      errorInfo, correct);
              }
              if(!correct){
                 result.addr = 0;
              }
              return correct;
          }
/*
1.30 retrieveRelation

Special accelerated version for relations

*/
          bool retrieveRelation(const string& objName, 
                                 ListExpr& resType, Word& result
                                ){

             string fname1 = objName+".bin";
             if(!retrieveRelationFile(objName, fname1)){
                return false;
             }
             result = createRelationFromFile(fname1, resType);
             FileSystem::DeleteFileOrFolder(fname1);
             return true;
          }


/*
1.31 retrieveRelationInFile

Retrieves a relation which is on a remotely stored file.

*/
          bool retrieveRelationInFile(const string& fileName,
                                      ListExpr& resType, 
                                      Word& result){
             boost::lock_guard<boost::recursive_mutex> guard(simtx);
             result.addr = 0;
             // get the full path for requesting files 
             string rfpath = si->getRequestFilePath()+"/";
             string base = FileSystem::Basename(fileName);
             if(stringutils::startsWith(fileName,rfpath)){
                 // we can just get the file without copying it
                 if(!si->requestFile(fileName.substr(rfpath.length()),
                                     base+".tmp",true)){
                    return false;
                 }
                 result = createRelationFromFile(base+".tmp",resType);
                 FileSystem::DeleteFileOrFolder(base+".tmp");
                 return true;
             }
             // remote file located in a not accessible folder, send command
             // to copy it into a such folder
             string cmd = "query copyFile('"+fileName+ "', '" + 
                                           rfpath + base +".tmp')";
             SecErrInfo  serr;
             ListExpr resList;
             showCommand(si,host,port,cmd,true);
             si->Secondo(cmd,resList,serr);
             showCommand(si,host,port,cmd,false);

             if(serr.code!=0){
               showError(si, cmd,serr.code,serr.msg);
               return false;
             }
             if(    !mynl->HasLength(resList,2) 
                 || mynl->AtomType(mynl->Second(resList))!=BoolType){
               cerr << "command " << cmd << " returns unexpected result" 
                    << endl;
               cerr << mynl->ToString(resList) << endl;
               return false;
             }
             if(!mynl->BoolValue(mynl->Second(resList))){
                 mynl->Destroy(resList);
                 return false;
             }

             mynl->Destroy(resList);

             // copy the file to local file system
             if( si->requestFile(base +".tmp", base +".tmp", true)!=0){
                cerr << "Requesting file " + base + ".tmp failed" << endl;
                return false;
             }
             result = createRelationFromFile(base+".tmp",resType);

             FileSystem::DeleteFileOrFolder(base+".tmp");
             cmd = "query removeFile('"+rfpath + base +".tmp' )" ;
             showCommand(si,host,port,cmd, true);
             si->Secondo(cmd,resList,serr);
             showCommand(si,host,port,cmd, false);
             if(serr.code != 0){
                showError(si,cmd,serr.code,serr.msg);
             }
             return true; 
          }

/*
1.32 retrieveRelationFile

retrieves a remote relation and stored it into a local file.

*/

          bool retrieveRelationFile(const string& objName,
                                    const string& fname1){
             boost::lock_guard<boost::recursive_mutex> guard(simtx);

             string rfname = si->getRequestFilePath() + "/"+fname1;
             // save the remove relation into a binary file
             SecErrInfo serr;
             ListExpr resList;
             string cmd =   "query createDirectory('"+si->getRequestFilePath()  
                   + "', TRUE) ";
             showCommand(si,host,port,cmd,true);
             si->Secondo(cmd,resList,serr);
             showCommand(si,host,port,cmd,false);
             if(serr.code!=0){
                 cerr << "Creating filetransfer directory failed" << endl;
                 return false;
             }


             cmd = "query " + objName + " feed fconsume5['"
                          +rfname+"'] count";
             showCommand(si,host,port,cmd,true);
             si->Secondo(cmd,resList,serr);
             showCommand(si,host,port,cmd,false);

             if(serr.code!=0){
                 return false;
             }

             if(si->requestFile(fname1, fname1 ,true)!=0){
                 return false;
             }

             // delete remote file             
             cmd = "query removeFile('"+rfname+"')";
             showCommand(si,host,port,cmd,true);
             si->Secondo(cmd,resList,serr);
             showCommand(si,host,port,cmd,false);
             return true;
          }


/*
1.34 retrieveAnyFile

This function can be used to get any file from a remote server
even if the file is located outside the requestFile directory.

*/
          bool retrieveAnyFile(const string& remoteName,
                            const string& localName){
              string rf = getRequestFolder();
              bool copyRequired = !stringutils::startsWith(remoteName,rf);
              string cn;
              int err;
              string errMsg;    
              ListExpr result;
              double rt;
              if(!copyRequired){
                cn = remoteName;
              } else {
                 // create request dir 
                 string cmd = "query createDirectory('"+rf+"', TRUE)";
                 simpleCommand(cmd,err,errMsg,result,false,rt);
                 if(err){
                   showError(this,cmd,err,errMsg);
                   return false;
                 }
                 cn = "tmp_" + stringutils::int2str(serverPid()) + "_" 
                      + FileSystem::Basename(remoteName);
                 string cf =  getSecondoHome()+"/"+ rf + "/"+cn;
                 cmd = "query copyFile('"+remoteName+"','"+cf+"')";
                 simpleCommand(cmd,err,errMsg,result,false,rt);
                 if(err){
                    cerr << "command " << cmd << " failed" << endl;
                    return false;
                 }
                 if(!nl->HasLength(result,2)){
                   cerr << "unexpected result for command " << cmd << endl;
                   cerr << "expected (type value), got " 
                        << nl->ToString(result) << endl;
                   return false;
                 }
                 if(nl->AtomType(nl->Second(result)) != BoolType){
                   cerr << "unexpected result for command " << cmd << endl;
                   cerr << "expected (bool boolatom), got " 
                        << nl->ToString(result) << endl;
                   return false;
                 }
                 if(!nl->BoolValue(nl->Second(result))){
                   cerr << "copying file failed" << endl;
                   return false;
                 }
 
              }
             
              int errres  = requestFile(cn,localName,true);

              if(copyRequired){
                 // remove copy
                 string cmd = "query removeFile('"+cn+"')";
                 simpleCommand(cmd,err,errMsg,result,false,rt);
                 if(err){
                    cerr << "command " << cmd << " failed" << endl;
                 }
              }
              return errres==0;
          }



/*
1.35 creates a relation from its binary file representation.

*/
          Word createRelationFromFile(const string& fname, ListExpr& resType){
            
             boost::lock_guard<boost::recursive_mutex> guard(simtx);

             Word result((void*) 0);
             // create result relation

            
             ListExpr tType = nl->Second(resType);
             tType = SecondoSystem::GetCatalog()->NumericType(resType);
             TupleType* tt = new TupleType(nl->Second(tType));
             ffeed5Info reader(fname,tt);

             if(!reader.isOK()){
               tt->DeleteIfAllowed();
               return result;
             }

             ListExpr typeInFile = reader.getRelType();
             if(!nl->Equal(resType, typeInFile)){
                cerr << "Type conflict between expected type and tyoe in file"
                     << endl;
                cerr << "Expected : " << nl->ToString(resType) << endl;
                cerr << "Type in  File " << nl->ToString(typeInFile) << endl;
                tt->DeleteIfAllowed();
                return result;
             }
             Relation* resultrel;
             {
                 boost::lock_guard<boost::mutex> guard2(createRelMut);
                 resultrel = new Relation(tType); 
             }

             Tuple* tuple;
             while((tuple=reader.next())){
                 boost::lock_guard<boost::mutex> guard2(createRelMut);
                 resultrel->AppendTuple(tuple);
                 tuple->DeleteIfAllowed();
             }
             tt->DeleteIfAllowed();
             result.addr = resultrel;
             return result;
          }


       ostream& print(ostream& o) const{
         o << host << ", " << port << ", " << config;
         return o;
       } 

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


   // retrieves secondoHome from remote server
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


ostream& operator<<(ostream& o, const ConnectionInfo& sc){
    return sc.print(o);
}



/*
2 CommandListener

This is a callback class interface for remote commands.

*/
template<class ResType>
class CommandListener{
 public:

  virtual void jobDone(int id, string& command, size_t no, int error, 
                       string& errMsg, ResType& resList, double runtime)=0;
};









/*
3 Class ProgressInfo

*/

class PProgressInfo{


  public:
     PProgressInfo(){
       lastValue = -1;
       w = new StopWatch(); 
       lastT = 0;
     }
   
    ~PProgressInfo(){
        delete w;
    }

    int getLastValue(){
        return lastValue;
    }
    void setLastValue(int i){
       lastValue = i;
    }

    void reset(){
       delete w;
       w = new StopWatch();
       lastT = 0;
       lastValue = -1;
    }

    double getTime(){
      return  w->diffSecondsReal();
    }

    void setLast(){
       lastT = getTime();
    }
  
    double getLastT(){
       return lastT;
    }

  private:
     int lastValue;
     StopWatch* w;
     double lastT;
};

/*
4 Class ProgressView

*/
class PProgressView: public MessageHandler{
  public:

     PProgressView(){
        infos = 0;
        enabled = true;
     }

     ~PProgressView(){
        killInfos();
     }

     bool handleMsg(NestedList* nl, ListExpr list, int source) {
        if(!enabled) return false;
        if(source <= 0) return false;
        if(source > noServers) return false;


        boost::lock_guard<boost::mutex> guard(mtx);
        if(!nl->HasMinLength(list,2)){
           return false;
        }
        if ( !nl->IsEqual(nl->First(list),"progress") ){
           return false;
        }
        ListExpr second = nl->Second(list);
        if(!nl->HasMinLength(second,2)){
           return false;
        }
        if(nl->AtomType(nl->First(second))!=IntType){
           return false;
        }
        if(nl->AtomType(nl->Second(second))!=IntType){
          return false;
        }

        int actValue = nl->IntValue(nl->First(second));
        int totalValue = nl->IntValue(nl->Second(second));
        write(source, actValue, totalValue);
        return true;
     }

     void killInfos(){
        if(infos){
          for(int i=0;i<noServers;i++){
            if(infos[i]){
              delete infos[i];
            }
          }
          delete[] infos;
        }

     }


     void init(int _noServers){
         if(!enabled) return;
         noServers = _noServers; 
         boost::lock_guard<boost::mutex> guard(mtx);
         killInfos();
         cout << endl << endl;
         infos = new PProgressInfo*[noServers];
         for(int i=0;i<noServers;i++){
             cout << (i) << " \t: " <<  endl;
             infos[i] = new PProgressInfo(); 
         }
         Bash::cursorUp(noServers+1);
     }

     void finish(){
         boost::lock_guard<boost::mutex> guard(mtx);
         Bash::cursorDown(noServers); 
         cout << "\n\n";
     }

     void enable(bool on){
        enabled = on;
     }


   private:
      bool enabled;
      int noServers;
      PProgressInfo** infos;
      boost::mutex mtx;

      void write(int source, int actValue, int totalValue){
         if(!infos[source-1]){
             infos[source-1] = new PProgressInfo();
          }
          PProgressInfo* info = infos[source-1];

          double lastT = info->getLastT(); 
          double t = info->getTime();
          if( (totalValue >=0) &&  (t - lastT < 0.5)){
              return;
          }
          info->setLast();

          Bash::cursorDown(source);
          cout << formatNumber( (source-1),3,' ') << " : ";
          if(actValue >=0){
              if(totalValue<=0){
                 cout << "done, total time: " << (int) t << " seconds" ;
                 info->reset();
              } else {
                 int value = ((actValue * 100 ) / totalValue);
                 int tt = (t * 100) / value; // total time
                 int rt = (tt * (100-value)) / 100; // remaining time
                 cout << progbar(value,20) << "  " << value 
                      << "% remaining time " << formatTime(rt) ;
              }
          }  else {
             info->reset();
             cout << "started";
          }
          Bash::clearRestOfLine();
          cout << "\r"; 
          Bash::cursorUp(source);
          cout.flush();
      }


      string formatTime(int seconds){
          int days = seconds / 86400;
          seconds = seconds % 86400;
          int hours = seconds / 3600;
          seconds = seconds % 3600;
          int minutes = seconds / 60;
          seconds = seconds % 60;

          bool force = false;
          stringstream ss;
          if(days > 0){
             ss << days << " days ";  
             force = true;
          }
          if(force || hours > 0){
              ss << hours << " hours ";
              force = true;
          }
          if( force || minutes > 0){
              ss << minutes << ":" << formatNumber(seconds,2) << " min";
          } else {
              ss << seconds << " seconds";
          }
          return ss.str();
      }

      string formatNumber(int number, int digits, char f = '0'){
          string n = stringutils::int2str(number);
          stringstream ss;
          for(int i=0;i<digits-(int)n.length(); i++){
            ss << f;
          }
          return ss.str() + n;
      }

      string progbar( int percent, int length){
         double full = (length*percent) / 100.0;
         int fulli = (int) (full+0.5);
         stringstream ss;
         char b ='*';
         char e = '-';
         for(int i=0;i<fulli;i++){
             ss << b;
         }
         for(int i=0;i<length-fulli;i++){
             ss << e;
         }
         return ss.str();
      }
};



/*
5 The Distributed2Algebra


Besides the usual Tasks of an algebra, namely providing types and 
operators, this algebra alos manages sets of connections. One set is 
for user defined connections, the other one contains connections used 
in DArrays. 

*/

class Distributed2Algebra: public Algebra{

  public:

/*
1.1 Constructor defined at the end of this file

*/
     Distributed2Algebra();


/*
1.2 Destructor

Closes all open connections and destroys them.

*/
     ~Distributed2Algebra();


/*
~addConnection~

Adds a new connection to the connection pool.

*/
     int addConnection(ConnectionInfo* ci){
       boost::lock_guard<boost::mutex> guard(mtx);
       int res = connections.size();
       connections.push_back(ci);
       if(ci){
           ci->setId(connections.size());
       }
       return res; 
     }

/*
~noConnections~

Returns the number of connections

*/

     size_t noConnections(){
         boost::lock_guard<boost::mutex> guard(mtx);
         size_t res =  connections.size();
         return res;
     }


/*
~getConnection~

Returns a connection

*/

     ConnectionInfo* getConnection(const int i){
         boost::lock_guard<boost::mutex> guard(mtx);
         if(i<0 || ((size_t) i>=connections.size())){
           return 0;
         }
         ConnectionInfo* res =  connections[i];
         res->setId(i);;
         return res;
     }

/*
~showProgress~

*/     
    void showProgress(bool enable){
       if(pprogView){
           pprogView->enable(enable);
           MessageCenter::GetInstance()->RemoveHandler(pprogView);
           if(enable){
               MessageCenter::GetInstance()->AddHandler(pprogView);
           }
       } 
    }



/*
~noValidConnections~

Returns the number of non-null connections.

*/    

   size_t noValidConnections(){
     boost::lock_guard<boost::mutex> guard(mtx);
     size_t count = 0;
     for(size_t i=0;i<connections.size();i++){
       if(connections[i]) count++;
     }
     return count;
   }

/*
~disconnect~

Closes all connections and destroys the instances.
The return value is the number of closed connections.

*/
     int disconnect(){
        int count = 0;
        boost::lock_guard<boost::mutex> guard(mtx);
        for(size_t i=0;i<connections.size();i++){
          if(connections[i]){
             delete connections[i];
             count++;
          }
        }
        connections.clear();
        return count;
     }
     

/*
~disconnect~

Disconnects a specified connection and removes it from the
connection pool. The result is 0 or 1 depending whether the
given argument specifies an existing connection.

*/
     int disconnect( unsigned int position){
        boost::lock_guard<boost::mutex> guard(mtx);
        if( position >= connections.size()){
           return 0;
        }
        if(!connections[position]){
           return 0;
        }
        delete connections[position];
        connections[position] = 0;
        return 1;
     }

/*
~isValidServerNo~

checks whether an given integer points to a server

*/
   bool isValidServerNumber(int no){
      if(no < 0){
        return false;
      }
      boost::lock_guard<boost::mutex> guard(mtx);
      bool res = no < (int) connections.size();
      return res; 
   }

/*
~serverExists~

*/   
  bool serverExists(int s){
     return isValidServerNumber(s) && (connections[s]!=0);
  }

/*
~serverPid~

*/

   int serverPid(int s){
      boost::lock_guard<boost::mutex> guard(mtx);
 
      if(s < 0 || (size_t) s >= connections.size()){
          return 0;
      }
      ConnectionInfo* ci = connections[s];
      return ci?ci->serverPid():0;
   }



/*
~sendFile~

Transfers a local file to a remove server.

*/
    int sendFile( const int con, 
                       const string& local,
                       const string& remote,
                       const bool allowOverwrite){
      if(con < 0 ){
       return -3;
      }
      unsigned int con2 = con;
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con2 >= connections.size()){
           return -3;
         }
         c = connections[con2];
      }
      if(!c){
         return -3;
      }
      return c->sendFile(local,remote, allowOverwrite);
    }
    
/*
~requestFile~

This functions copies a remotely stored file into the local
file system.


*/
    int requestFile( const int con, 
                     const string& remote,
                     const string& local,
                     const bool allowOverwrite){
      if(con < 0 ){
       return -3;
      }
      unsigned int con2 = con;
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con2 >= connections.size()){
            return -3;
         }
         c = connections[con2];
      }
      if(!c){
        return -3;
      }
      return c->requestFile(remote,local, allowOverwrite);
    }


/*
~getRequestFolder~

returns the name of the folder where get requests start.

*/
    string getRequestFolder( int con){
      if(con < 0 ){
       return "";
      }
      unsigned int con2 = con;
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con2 >= connections.size()){
           return "";
         }
         c = connections[con2];
      }
      if(!c){
         return "";
      }
      return c->getRequestFolder(); 
    }


/*
~getSendFolder~

returns the name of the folder where new files are stored on remote side.

*/    
    string getSendFolder( int con){
      if(con < 0 ){
       return "";
      }
      unsigned int con2 = con;
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con2 >= connections.size()){
           return "";
         }
         c = connections[con2];
      }
      if(!c){
        return "";
      }
      return c->getSendFolder(); 
    }


/*
~initProgress~

initializes the progress view.

*/
    void initProgress(){
        pprogView->init(connections.size());
    }


/*
~finishProgress~

mark the end of the progress view

*/
    void finishProgress(){
        pprogView->finish();
    }

/*
~getHost~

Returns the host name of the connection with index con.

*/

    string getHost(int con){
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con < 0 || con >= (int) connections.size()){
            return "";
         }
         c = connections[con];
      }
      if(!c) return "";
      return c->getHost();
    }


/*
~getPort~

Returns the port of the connection with index con.

*/    
    int getPort(int con){
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con < 0 || con >= (int) connections.size()){
            return -3;
         }
         c = connections[con];
      }
      if(!c) return -3;
      return c->getPort();
    }


/*
~getConfig~

Returns the name of the configuration file of the
connection at index ~con~.

*/    
    string getConfig(int con){
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con < 0 || con >= (int) connections.size()){
            return "";
         }
         c = connections[con];
      }
      if(!c) return "";
      return c->getConfig();
    }


/*
~check~

Tests whether the connection at index ~con~ is alive.

*/
    bool check(int con){
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con < 0 || con >= (int) connections.size()){
             return false;
         }
         c = connections[con];
      }
      if(!c) return false;
      return c->check();
    }
    
/*
~simpleCommand~

Performs a command at connection with index ~con~.

*/

    bool simpleCommand(int con, const string& cmd, int& error, 
                       string& errMsg, ListExpr& resList, const bool rewrite,
                       double& runtime){
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con < 0 || con >= (int) connections.size()){
            error = -3;
            errMsg = "invalid connection number";
            resList = nl->TheEmptyList();
            runtime = 0;
            return false;
         }
         c = connections[con];
      }
      if(!c) {
         error = -3;
         errMsg = "invalid connection number";
         resList = nl->TheEmptyList();
         runtime = 0;
         return false;
      }
      c->simpleCommand(cmd,error,errMsg,resList, rewrite, runtime);
      return true;
    }
    
/*
~simpleCommand~

Performs a command returning the result as as string.


*/
    bool simpleCommand(int con, const string& cmd, int& error, 
                       string& errMsg, string& resList, const bool rewrite,
                       double& runtime){
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con < 0 || con >= (int) connections.size()){
            error = -3;
            errMsg = "invalid connection number";
            resList = "()";
            runtime = 0;
           return false;
         }
         c = connections[con];
      }
      if(!c) {
         error = -3;
         errMsg = "invalid connection number";
         resList = "()";
         runtime = 0;
         return false;
      }
      c->simpleCommand(cmd,error,errMsg,resList, rewrite, runtime);
      return true;
    }


/*
The next functions are for interacting with workers, i.e.
connections coming from darray elements.

*/
  ConnectionInfo* getWorkerConnection( const DArrayElement& info,
                                       const string& dbname ){
     boost::lock_guard<boost::mutex> guard(workerMtx);
     map<DArrayElement, pair<string, ConnectionInfo*> >::iterator it;
     it = workerconnections.find(info);
     pair<string,ConnectionInfo*> pr("",0);
     if(it==workerconnections.end()){
        if(!createWorkerConnection(info,pr)){
             return 0;
        }
        workerconnections[info] = pr;
        it = workerconnections.find(info);
     } else {
         pr = it->second;
     }
     string wdbname = dbname;
     if(pr.first!=wdbname){
         if(!pr.second->switchDatabase(wdbname,true)){
            it->second.first="";
            return 0;
         } else {
             it->second.first=dbname;
         }
     }
     return pr.second;
  }

/*
~getDBName~

Returns the database name currently opened at the specified connection.

*/
  string getDBName(const DArrayElement& info){
     boost::lock_guard<boost::mutex> guard(workerMtx);
     map<DArrayElement, pair<string, ConnectionInfo*> >::iterator it;
     it = workerconnections.find(info);
     string db = "";
     if(it!=workerconnections.end()){
        db=it->second.first;         
     }
     return db;
  }


/*
This operator closes all non user defined existing server connections.
It returns the numer of closed workers

*/
 int closeAllWorkers(){
    boost::lock_guard<boost::mutex> guard(workerMtx);
    map<DArrayElement, pair<string,ConnectionInfo*> > ::iterator it;
    int count = 0;
    for(it=workerconnections.begin(); it!=workerconnections.end(); it++){
       ConnectionInfo* ci = it->second.second;
       delete ci;
       count++;
    }
    workerconnections.clear();
    return count;
 } 

/*
The operator ~closeWorker~ closes the connections for a 
specified DArrayElement.

*/ 
  bool closeWorker(const DArrayElement& elem){
    boost::lock_guard<boost::mutex> guard(workerMtx);
     map<DArrayElement, pair<string,ConnectionInfo*> >::iterator it;
     it = workerconnections.find(elem);
     if(it==workerconnections.end()){
        return false;
     }
     ConnectionInfo* info = it->second.second;
     if(info){
        delete info;
        it->second.second = 0;
     }
     workerconnections.erase(it);
     return true;
  }


/*
~workerConnection~

Checks whether the specified connection exists. If so, the currently 
opened database and the ConnectionInfo are returned.

*/
  bool workerConnection(const DArrayElement& elem, string& dbname,
                         ConnectionInfo*& ci){
    boost::lock_guard<boost::mutex> guard(workerMtx);
     map<DArrayElement, pair<string,ConnectionInfo*> >::iterator it;
     it = workerconnections.find(elem);
     if(it == workerconnections.end()){
        return false;
     } 
     dbname = it->second.first;
     ci = it->second.second;
     return true;
  }


/*
workersIterator

returns an iterator over the available connetions of workers.

*/
  map<DArrayElement, pair<string,ConnectionInfo*> >::iterator
  workersIterator(){
    return workerconnections.begin();
  }

/*
~isEnd~

checks for the end of an iterator threadsafe.

*/
  bool isEnd( map<DArrayElement, pair<string,ConnectionInfo*> >::iterator& it){
    boost::lock_guard<boost::mutex> guard(workerMtx);
    return it==workerconnections.end();
  } 


/*
~getTempName~

returns a temporary name for a specified connection.

*/
  string getTempName(int server){
    boost::lock_guard<boost::mutex> guard(mtx);
    if(server < 0 || (size_t)server<connections.size()){
       return "";
    }
    ConnectionInfo* ci = connections[server];
    if(!ci){
       return "";
    }
    stringstream ss;
    ss << "TMP_" << WinUnix::getpid() 
       << "_" << ci->serverPid() 
       << "_" << nextNameNumber();
    return ss.str();
  } 

/*
~getTempName~

returns a temporary name for a specified DArrayElement.

*/

  string getTempName(const DArrayElement& elem){
     boost::lock_guard<boost::mutex> guard(workerMtx);
     map<DArrayElement, pair<string,ConnectionInfo*> >::iterator it;
     it = workerconnections.find(elem);
     if(it==workerconnections.end()){
        return "";
     }
     stringstream ss;
     ss << "TMP_" << WinUnix::getpid() 
        << "_" << it->second.second->serverPid() 
        << "_" << nextNameNumber();
    return ss.str();
  }


/*
~getTempName~

returns some temporary name.

*/
  string getTempName(){
     boost::lock_guard<boost::mutex> guard1(workerMtx);
     boost::lock_guard<boost::mutex> guard2(mtx);
     ConnectionInfo* ci=0;
     for(size_t i=0;i<connections.size() && !ci; i++){
         ci = connections[i];
     }
     map<DArrayElement, pair<string,ConnectionInfo*> >::iterator it;
     for(it = workerconnections.begin(); 
        it !=workerconnections.end() && !ci; 
        it++){
         ci = it->second.second;
     }
     int spid;
     size_t sh;
     if(ci){
        sh = stringutils::hashCode(ci->getHost());
        spid = ci->serverPid();
     } else {
        srand(time(0));
        spid = rand();
        sh = rand();
     }
     stringstream ss;
     ss << "TMP_" << WinUnix::getpid() 
        << "_" << sh 
        << "_" << spid 
        << "_" << nextNameNumber();
    return ss.str();
  }

/*
~cleanUp~

removes temporary files and objects of all opened 
connections.

*/
  void cleanUp(){
    
     set<pair<string,int> > used;
     vector<ConnectionInfo*> unique;

     for(size_t i=0;i<connections.size();i++){
         ConnectionInfo* ci = connections[i];
         if(ci){
             pair<string,int> p(ci->getHost(), ci->getPort());
             if(used.find(p)==used.end()){
                  unique.push_back(ci);
                  used.insert(p);
             }
         }
     }
     map<DArrayElement, pair<string,ConnectionInfo*> >::iterator it;
     for(it = workerconnections.begin();it!=workerconnections.end();it++){
        ConnectionInfo* ci = it->second.second;
        if(ci){
            pair<string,int> p(ci->getHost(), ci->getPort());
            if(used.find(p)==used.end()){
                 unique.push_back(ci);
                 used.insert(p);
            }
        }
     }

     vector<boost::thread*> runners;
     for(size_t i=0;i<unique.size();i++){
        ConnectionInfo* ci = unique[i];
        boost::thread* r = new boost::thread(&ConnectionInfo::cleanUp, ci);
        runners.push_back(r);
     }
     for(size_t i=0;i<runners.size();i++){
        runners[i]->join();
        delete runners[i];
     }
  }


  private:
    // connections managed by the user
    vector<ConnectionInfo*> connections;
    boost::mutex mtx;

    // connections managed automatically 
    // for darray type
    // the key represents the connection information,
    // the string the used database
    // the ConnctionInfo the connection
    map<DArrayElement, pair<string,ConnectionInfo*> > workerconnections;
    boost::mutex workerMtx;

    size_t namecounter;
    boost::mutex namecounteraccess;

    PProgressView* pprogView;


   // returns a unique number
   size_t nextNameNumber(){
      boost::lock_guard<boost::mutex> guard(namecounteraccess);
      namecounter++;
      return namecounter;
   } 

    // tries to create a new connection to a worker
    bool createWorkerConnection(const DArrayElement& worker, pair<string, 
                                ConnectionInfo*>& res){
      string host = worker.getHost();
      int port = worker.getPort();
      string config = worker.getConfig();
      ConnectionInfo* ci = ConnectionInfo::createConnection(host, port, config);
      res.first="";
      res.second = ci;
      return ci!=0;
    }



};


  // Algebra instance
Distributed2Algebra* algInstance;

/*
2. Class ConnectionTask

This class manages the task for a single connection.
In an endless loop it will wait for a new command. If a 
new command is available, the command is executed and
the listener is informed about that. Then, then it waits for
the next command.

*/
template<class ResType>
class ConnectionTask{
  public:

     ConnectionTask( int _connection, 
                     CommandListener<ResType>* _listener):
      connection(_connection), listener(_listener), no(0),
      done(false){
        worker = boost::thread(&ConnectionTask::run, this); 
      } 
     
    ~ConnectionTask(){
          listener = 0;
          {
             boost::lock_guard<boost::mutex> lock(condmtx);
             done = true;
          }
          cond.notify_one();
          worker.join(); // wait until worker is done
      }


     void addCommand(const int id, const string& cmd){
       { 
         boost::lock_guard<boost::mutex> lock(condmtx);
         commandList.push_back(make_pair(id, cmd));
       }
       cond.notify_one();        
     }

     void removeListener(){
        listener = 0;
     }
  
     void stop(){
        done = true;
     }

  private:
    
    void run(){
      pair<int,string> cmd1;
      while(!done){
         {
           boost::unique_lock<boost::mutex> lock(condmtx);
           // wait for available command 
           while(!done && commandList.empty()){
              cond.wait(lock);
           }
           if(done){
             return; 
           }
           // get next command from list
           cmd1 = commandList.front();
           commandList.pop_front();
         }
         no++;
         string cmd = cmd1.second;
         int id = cmd1.first;
         int error;
         string errMsg; 
         ResType resList;
         double runtime;
         algInstance->simpleCommand(connection, cmd, 
                                    error,errMsg, resList, true, runtime);

         if(listener){
            listener->jobDone(id, cmd, no, error, errMsg, resList, runtime);
         }
      }
     }

     int connection;            // server number
     CommandListener<ResType>* listener; // informed if a command is processed
     size_t no;                 // a running number
     bool done;                 // finished state
     list<pair<int,string> > commandList; // list of command

     boost::mutex condmtx; 
     boost::condition_variable cond;
     boost::thread worker;
     
};



/*
0.16 ~showError~

*/
void showError(const ConnectionInfo* ci, const string& command , 
               const int errorCode, const string& errorMessage){

   static boost::mutex mtx;
   boost::lock_guard<boost::mutex> lock(mtx);
   if(errorCode){
      cerr << "command " << command << endl 
           << " failed on server " << (*ci) << endl
           << "with code " << errorCode << " : " << errorMessage << endl;
   }
}

void showError(const SecondoInterfaceCS* ci, const string& command , 
               const int errorCode, const string& errorMessage){

   static boost::mutex mtx;
   boost::lock_guard<boost::mutex> lock(mtx);
   if(errorCode){
      cerr << "command " << command << endl 
           << " failed on server " << (ci->getHost()) << endl
           << "with code " << errorCode << " : " << errorMessage << endl;
   }
}



/*
1 Operators

1.1 Operator  ~connect~

Establishes a connection to a running Secondo Server.
The result of this operator is a boolean indicating the success 
of the operation. 

1.1.1 Type Mapping

This operator gets a hostname, a port and a file for the configuration.

*/
ListExpr connectTM(ListExpr args){
  string err = "{text,string} x int [x {text,string}] = "
               "(host, port[, configfile]) expected";
  if(!nl->HasLength(args,3) && !nl->HasLength(args,2)){
   return listutils::typeError(err);     
  }
  ListExpr first = nl->First(args);
  if(!FText::checkType(first)&& !CcString::checkType(first)){
    return listutils::typeError(err);     
  }
  if(!CcInt::checkType(nl->Second(args))){
    return listutils::typeError(err);     
  }
  if(nl->HasLength(args,3)) {
     ListExpr third = nl->Third(args);
     if(!FText::checkType(third) && !CcString::checkType(third)){
       return listutils::typeError(err);     
     }
     return listutils::basicSymbol<CcBool>();
  }
  return nl->ThreeElemList(
           nl->SymbolAtom(Symbols::APPEND()),
           nl->OneElemList( nl->StringAtom("SecondoConfig.ini")),
           listutils::basicSymbol<CcBool>());
}

/*
1.1.2 Value Mapping

*/
template<class H, class C>
int connectVMT( Word* args, Word& result, int message,
                  Word& local, Supplier s ){
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;

   H* ahost = (H*) args[0].addr;
   CcInt* aport = (CcInt*) args[1].addr;
   C* afile = (C*) args[2].addr;

   if(!ahost->IsDefined() || !aport->IsDefined() || !afile->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   string host = ahost->GetValue();
   int port = aport->GetValue();
   string file = afile->GetValue();
   if(port<=0){
      res->SetDefined(false);
      return 0;
   }
   NestedList* mynl = new NestedList();
   SecondoInterfaceCS* si = new SecondoInterfaceCS(true,mynl, true);
   string user="";
   string passwd = "";
   string errMsg;
   MessageCenter* msgcenter = MessageCenter::GetInstance();
   si->setMaxAttempts(4);
   si->setTimeout(1);
   if(! si->Initialize(user, passwd, host, 
                       stringutils::int2str(port), file, 
                       errMsg, true)){
      msgcenter->Send(nl, nl->TwoElemList( nl->SymbolAtom("simple"),
                                       nl->TextAtom(errMsg)));
     delete si;
     si = 0;
     delete mynl;
     res->Set(true,false);
   } else {
     res->Set(true,true);
     ConnectionInfo* ci= new ConnectionInfo(host, port, file, si, mynl);
     algInstance->addConnection(ci);     
   }
   return 0;
}

/*
1.1.3 Specification

*/
OperatorSpec connectSpec(
     "text x int x text -> bool",
     "connect(host, port, configfile)",
     "Connects to a Secondo Server",
     " query connect('localhost', 1234, 'SecondoConfig.ini')"); 


/*
1.1.4 ValueMapping Array

*/
ValueMapping connectVM[] = {
  connectVMT<CcString,CcString>,
  connectVMT<CcString,FText>,
  connectVMT<FText, CcString>,
  connectVMT<FText,FText>
};

/*
1.1.5 Selection Function

*/
int connectSelect(ListExpr args){
  int first = FText::checkType(nl->First(args))?2:0;
  int second = 0;
  if(nl->HasLength(args,3)){
    second = FText::checkType(nl->Third(args))?1:0;
  }
  return first + second;
}


/*
1.1.6 Operator instance


*/
Operator connectOp (
    "connect",             //name
     connectSpec.getStr(),         //specification
     4,
     connectVM,        //value mapping
     connectSelect,   //trivial selection function
     connectTM        //type mapping
);


/*
1.2 Operator checkConnection

This operator checks available connections.
The current implementation soes it sequentially. In the future,
this will be parallelized using the boost library.

1.2.1 Type Mapping

This operator have no arguments. The result is a stream of tuples 
of the form (id, host, port, configfile, state)

*/
ListExpr  checkConnectionsTM(ListExpr args){
  
  if(!nl->IsEmpty(args)){
    return listutils::typeError("no arguments required");
  }
  ListExpr attrList = nl->SixElemList(
    nl->TwoElemList( nl->SymbolAtom("Id"), 
                     listutils::basicSymbol<CcInt>()),
    nl->TwoElemList( nl->SymbolAtom("Host"), 
                     listutils::basicSymbol<FText>()),
    nl->TwoElemList( nl->SymbolAtom("Port"), 
                     listutils::basicSymbol<CcInt>()),
    nl->TwoElemList( nl->SymbolAtom("ConfigFile"), 
                     listutils::basicSymbol<FText>()),
    nl->TwoElemList( nl->SymbolAtom("OK"), 
                     listutils::basicSymbol<CcBool>()),
    nl->TwoElemList( nl->SymbolAtom("PID"), 
                     listutils::basicSymbol<CcInt>()) 
);
    return nl->TwoElemList(
             listutils::basicSymbol<Stream<Tuple> >(),
             nl->TwoElemList(
                 listutils::basicSymbol<Tuple>(),
                  attrList));
}

/*
1.2.2 Value Mapping

*/

class checkConLocal{
 public:
    checkConLocal(ListExpr resType): pos(0){
      tt = new TupleType(resType);
    }
    ~checkConLocal(){
       tt->DeleteIfAllowed();
    }
    Tuple* next(){
      if(pos >= algInstance->noConnections()){
        return 0;
      }
      Tuple* res = new Tuple(tt);
      res->PutAttribute(0, new CcInt(true,pos));
      string h = algInstance->getHost(pos);
      FText* host = h.empty()?new FText(false,""): new FText(true,h);
      res->PutAttribute(1, host);
      int p = algInstance->getPort(pos);
      CcInt* port = p<0?new CcInt(false,0) : new CcInt(true,p);
      res->PutAttribute(2, port);
      string c = algInstance->getConfig(pos);
      FText* config = c.empty()?new FText(false,""): new FText(true,c);
      res->PutAttribute(3, config);
      res->PutAttribute(4, new CcBool(true, algInstance->check(pos)));
      int pid = algInstance->serverPid(pos);
      CcInt* Pid = pid>0?new CcInt(true,pid): new CcInt(false,0);
      res->PutAttribute(5,Pid);
      pos++;
      return res;
    }
 private:
    size_t pos;
    TupleType* tt;

};


int checkConnectionsVM( Word __attribute__((unused))* args, 
                        Word& result, int message,
                        Word& local, 
                        Supplier __attribute__((unused)) s ){

  checkConLocal* li = (checkConLocal*) local.addr;
  switch(message){
     case OPEN: 
         if(li) delete li;
         local.addr = new checkConLocal(nl->Second(GetTupleResultType(s)));
         return 0;
     case REQUEST: 
         result.addr = li?li->next():0;
         return result.addr?YIELD:CANCEL;
     case CLOSE:
           if(li){
             delete li;
             local.addr = 0;
           }              
           return 0;   
  }
  return -1;

}

/*
1.2.3 Specification

*/
OperatorSpec checkConnectionsSpec(
     "-> stream(tuple((No int)(Host text)(Port int)(Config text)(Ok bool)",
     "checkConnections()",
     "Check connections in the Distributed2Algebra",
     " query checkConnections() consume"); 


/*
1.2.4 Operator instance


*/
Operator checkConnectionsOp (
    "checkConnections",             //name
    checkConnectionsSpec.getStr(),         //specification
    checkConnectionsVM,        //value mapping
    Operator::SimpleSelect,   //trivial selection function
    checkConnectionsTM        //type mapping
);



/*
1.3 Operator ~rcmd~

This operator performs a remote command on a server.
The result is a tuple stream containing a single tuple containing the 
result of the command.

1.3.1 Type Mapping

*/
ListExpr rcmdTM(ListExpr args){
  string err = "int x {string, text} expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  ListExpr cmdt = nl->Second(args);
  if(!FText::checkType(cmdt) && !CcString::checkType(cmdt)){
    return listutils::typeError(err);
  }
  ListExpr attrList = nl->FourElemList(
         nl->TwoElemList( nl->SymbolAtom("Connection"), 
                          listutils::basicSymbol<CcInt>()),
         nl->TwoElemList( nl->SymbolAtom("ErrorCode"), 
                          listutils::basicSymbol<CcInt>()),
         nl->TwoElemList( nl->SymbolAtom("Result"), 
                          listutils::basicSymbol<FText>()),
         nl->TwoElemList( nl->SymbolAtom("Runtime"), 
                          listutils::basicSymbol<CcReal>()));
  return nl->TwoElemList( 
            listutils::basicSymbol<Stream<Tuple> >(),
            nl->TwoElemList(
                listutils::basicSymbol<Tuple>(),
                attrList));
}

/*
1.3.2 Value Mapping

*/
template<class C>
int rcmdVMT( Word* args, Word& result, int message,
                        Word& local, Supplier s ){
   switch(message){
      case OPEN: {
           CcInt* con = (CcInt*) args[0].addr;
           C*  cmd = (C*) args[1].addr;
           if(!con->IsDefined() || !cmd->IsDefined()){
             return 0;
           }
           int conv = con->GetValue();
           int errorCode = 0;
           string result ="";
           string errMsg;
           double runtime;
           bool ok = algInstance->simpleCommand(conv, cmd->GetValue(), 
                                           errorCode, errMsg, result, true,
                                           runtime);
           if(!ok){
             return 0;
           }
           TupleType* tt = new TupleType( nl->Second(GetTupleResultType(s)));
           Tuple* resTuple = new Tuple(tt);
           tt->DeleteIfAllowed();
           resTuple->PutAttribute(0, new CcInt(true,conv));
           resTuple->PutAttribute(1, new CcInt(true,errorCode));
           resTuple->PutAttribute(2, new FText(true, result));
           resTuple->PutAttribute(3, new CcReal(true,runtime));
           local.addr = resTuple;
            return 0;
       }
       case REQUEST: {
           result.addr = local.addr;
           local.addr = 0;
           return result.addr?YIELD:CANCEL; 
       }
       case CLOSE: {
          if(local.addr){
             ((Tuple*)local.addr)->DeleteIfAllowed();
             local.addr = 0; 
          }
          return 0;
       }
   }
   return -1;
}

/*
1.3.4 ValueMapping Array

*/
ValueMapping rcmdVM[] = {
    rcmdVMT<CcString>,
    rcmdVMT<FText>
};

/*
1.3.5 Selection function

*/
int rcmdSelect(ListExpr args){
   ListExpr cmd = nl->Second(args);
   return CcString::checkType(cmd)?0:1;
}

/*
1.3.6 Specification

*/
OperatorSpec rcmdSpec(
     "int x {text,string} -> stream(Tuple(C,E,R,T))",
     "rcmd(Connection, Command)",
     "Performs a remote command at given connection. Small Objects existing "
     "only in the local database, can be used by prefixing the object name "
     " with a dollar sign. ",
     "query rcmd(1,'list algebras') consume"); 

/*
1.3.7 Operator instance

*/
Operator rcmdOp (
    "rcmd",             //name
     rcmdSpec.getStr(),         //specification
     2,
     rcmdVM,        //value mapping
     rcmdSelect,   //trivial selection function
     rcmdTM        //type mapping
);


/*
1.4 Operator disconnect

This operator closes an existing connection. The ID's of 
subsequent connects will be decreased. If no special id
id given, all existing connections will be closed.

1.4.1 Type Mapping

*/
ListExpr disconnectTM(ListExpr args){

  string err = "int or nothing expected";
  if(nl->IsEmpty(args)){
    return listutils::basicSymbol<CcInt>();
  }
  if(!nl->HasLength(args,1)){
     return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->First(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcInt>();
}


int disconnectVM( Word* args, Word& result, int message,
                        Word& local, Supplier s ){
  int resi=0;
  if(qp->GetNoSons(s)==0){
    // remove all commections
    resi = algInstance->disconnect();
  } else {
    // remove special connection
    CcInt* c = (CcInt*) args[0].addr;
    if(c->IsDefined() && c->GetValue()>=0){
       resi = algInstance->disconnect(c->GetValue());
    }
  }
  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;
  res->Set(true,resi);
  return 0;
}

/*
1.4.3 Specification

*/
OperatorSpec disconnectSpec(
     " int -> int , ->int",
     "disconnect(), disconnect(_)",
     "Closes a connection to a remote server. Without any argument"
     " all connections are closed, otherwise only the specified one",
     "query disconnect(0)");

/*
1.4.4 Operator instance

*/
Operator disconnectOp (
    "disconnect",             //name
     disconnectSpec.getStr(),         //specification
     disconnectVM,        //value mapping
     Operator::SimpleSelect,   //trivial selection function
     disconnectTM        //type mapping
);



/*
1..5 Operator rquery

This operator works quite similar to the rcmd command. 
The difference is that the correct type will be produced if possible.
This is done using a query on the specified server within the type mapping.
For this reason. only constants for the server number and the command
can be given. This makes only sense for queries. 

*/
template<class S> bool getValue(ListExpr in, string&out){
   Word res;
   bool success = QueryProcessor::ExecuteQuery(nl->ToString(in),res);
   if(!success){
     out ="";
     return false;
   }
   S* s = (S*) res.addr;
   if(!s->IsDefined()){
      out = "";
      s->DeleteIfAllowed();
      return false;
   }
   out = s->GetValue();
   s->DeleteIfAllowed();
   return true;
}

bool getValue(ListExpr in, int& out){
   Word res;
   bool success = QueryProcessor::ExecuteQuery(nl->ToString(in),res);
   if(!success){
     out = -1;
     return false;
   }
   CcInt* r = (CcInt*) res.addr;
   if(!r->IsDefined()){
     out = -1;
     r->DeleteIfAllowed();
     return false;
   }
   out = r->GetValue();
   r->DeleteIfAllowed();
   return true;
}


ListExpr rqueryTM(ListExpr args){
  string err = "int x {text,string} expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  if(!nl->HasLength(first,2) || !nl->HasLength(second,2)){
     return listutils::typeError("internal Error: usesArgsInTypeMapping");
  }
  if(!CcInt::checkType(nl->First(first))){
     return listutils::typeError(err);
  }
  ListExpr serverNo = nl->Second(first);
  // get the server number from the expression
   Word res;
   bool success = QueryProcessor::ExecuteQuery(nl->ToString(serverNo),res);
   if(!success){
       return listutils::typeError("could not evaluate the value of  " +
                                    nl->ToString(serverNo) );
   }
   CcInt* serverNoV = (CcInt*) res.addr;
   if(!serverNoV->IsDefined()){
       return listutils::typeError("Undefined Server number");
   }
   int sn = serverNoV->GetValue();
   serverNoV->DeleteIfAllowed();
   if(sn < 0 || sn>= (int)(algInstance->noConnections())){
      return listutils::typeError("Invalid server number");
   }
   // ok server number is ok
   string query;
   bool ok;
   if(FText::checkType(nl->First(second))){
      ok = getValue<FText>(nl->Second(second),query);
   } else if(CcString::checkType(nl->First(second))){
      ok = getValue<CcString>(nl->Second(second),query);
   } else {
      return listutils::typeError(err);
   }
   if(!ok){
      return listutils::typeError("Value of second argument invalid");
   }


   stringutils::trim(query);
   if(!stringutils::startsWith(query,"query")){
     return listutils::typeError("command is not a query");
   }
   query = query.substr(5); // remove leading query
   stringutils::trim(query);
   

   query = "query (" + query + ") getTypeNL";


   int error;
   string errMsg;
   ListExpr reslist;
   double runtime;
   ok = algInstance->simpleCommand(sn, query, error, errMsg, reslist, true,
                                   runtime);
   if(!ok){
     return listutils::typeError("found no connection for specified "
                                 "connection number");
   }
   if(error!=0){
     return listutils::typeError("Problem in determining result type : "
                                 + errMsg + "\n" + query);
   }
   if(!nl->HasLength(reslist,2)){
      return listutils::typeError("Invalid result get from remote server");
   }
   if(!FText::checkType(nl->First(reslist))){
      return listutils::typeError("Invalid result type for getTypeNL");
   }
   if(nl->AtomType(nl->Second(reslist))!=TextType){
     return listutils::typeError("getTypeNL returns invalid result");
   }
   string typeList = nl->Text2String(nl->Second(reslist));
   ListExpr resType;
   {
      boost::lock_guard<boost::mutex> guard(nlparsemtx);
      if(!nl->ReadFromString(typeList,resType)){
        return listutils::typeError("getTypeNL returns no "
                                    "valid list expression");
      }   
   }
   if(nl->HasLength(resType,2)){
     first = nl->First(resType);
     if(listutils::isSymbol(first, Stream<Tuple>::BasicType())){
        return listutils::typeError("remote result is a stream");
     }
   }


   return resType; 
}

/*
1.5.2 Value Mapping

*/
void sendMessage(MessageCenter* msg, const string& message){
  ListExpr list = nl->TwoElemList( nl->SymbolAtom("simple"),
                                   nl->TextAtom(message));
  msg->Send(nl,list );
  msg->Flush();
}

void sendMessage(const string& message){
  sendMessage(MessageCenter::GetInstance(), message);
}


template<class T>
int rqueryVMT( Word* args, Word& result, int message,
               Word& local, Supplier s ){

  MessageCenter*  msg = MessageCenter::GetInstance();
  result = qp->ResultStorage(s);
  CcInt* server = (CcInt*) args[0].addr;
  if(!server->IsDefined()){
     sendMessage(msg, "Server ID undefined");
     return 0;
  }
  int serv = server->GetValue();
  if(serv < 0 || serv >= algInstance->noConnections()){
    sendMessage(msg, "Invalid Server ID " );
    return 0;
  }
  T* query = (T*) args[1].addr;
  if(!query->IsDefined()){
    sendMessage(msg,"Undefined query");
    return 0;
  }
  int error; 
  string errMsg;
  ListExpr resList;
  double runtime;
  bool ok = algInstance->simpleCommand(serv, query->GetValue(), error,
                                       errMsg, resList, true, runtime);
  if(!ok){
     sendMessage(msg, "Error during remote command" );
     return 0;
  }
  if(error){
     sendMessage(msg, "Error during remote command" );
     return 0;
  } 
  // Create Object from resList
  if(!nl->HasLength(resList,2)){
     sendMessage(msg,"Invalid result list");
     return 0;
  }
  ListExpr typeList = nl->First(resList);
  ListExpr valueList = nl->Second(resList);
  int algId;
  int typeId;
  string typeName;
  ListExpr nTypeList = SecondoSystem::GetCatalog()->NumericType(typeList);
  ok = SecondoSystem::GetCatalog()->LookUpTypeExpr(typeList, 
                                               typeName, algId, typeId);
  if(!ok){ // could not determine algid and typeid for given type list
     sendMessage(msg, string("could not determine type Id and algebra "
                             "id for ") + nl->ToString(typeList) );
     return 0;
  }
  int errorPos=0;
  ListExpr errorInfo = listutils::emptyErrorInfo();
  bool correct;
  AlgebraManager* am = SecondoSystem::GetAlgebraManager();
  Word res = am->InObj(algId,typeId)(nTypeList, valueList, 
                       errorPos, errorInfo, correct);
  if(!correct){ // nested list could not read in correctly
     sendMessage(msg, "InObject failed" );
     return 0;
  }
  qp->DeleteResultStorage(s);
  qp->ChangeResultStorage(s,res);
  result = qp->ResultStorage(s);
  return 0;
}

/*
1.5.3 ValueMapping Array and Selection Function

*/

ValueMapping rqueryVM[] = {
  rqueryVMT<CcString>,
  rqueryVMT<FText>
};

int rquerySelect(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}

/*
1.5.4  Specification

*/
OperatorSpec rquerySpec(
     " int x {string,text} -> ??",
     " rquery(server, query)",
     " Performs a remote query at a remote server."
     " The server is specified by its Id as the first argument."
     " The second argument is the query. The result type depends"
     " on the type of the query expression",
     " query rquery(0, 'query ten count') ");

/*
1.4.4 Operator instance

*/
Operator rqueryOp (
    "rquery",             //name
     rquerySpec.getStr(),         //specification
     2,
     rqueryVM,        //value mapping
     rquerySelect,   //trivial selection function
     rqueryTM        //type mapping
);


/*
1.5 Operator prcmd

This operator takes a stream of tuples at least containing an 
integer and a text/string attribute.  These attributes are named 
as secondary arguments. It performed the commands get by the
text attribute on the servers given by the integer attribute. 
While on each connection, the processing is done in a serial way,
the processing between the different servers is processen in 
parallel. Each incoming tuple is extended by the errorCode, the 
errorMessage and the result (as text). Note that the order 
of output tuples depends on the running time of the single tasks,
not on in input order.

*/
ListExpr prcmdTM(ListExpr args){
  string err = "stream(tuple) x attrname x attrname expected";
  if(!nl->HasLength(args,3)){
    return listutils::typeError(err);
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
     return listutils::typeError(err + " (first arg is not a tuple stream)");
  }
  if(nl->AtomType(nl->Second(args))!=SymbolType) { 
    return listutils::typeError(err + " 2nd arg is not a "
                                      "valid attribute name");
  }
  if(nl->AtomType(nl->Third(args))!=SymbolType) {
    return listutils::typeError(err + " 3th arg is not a "
                                      "valid attribute name");
  }
  string intAttr = nl->SymbolValue(nl->Second(args));
  string textAttr = nl->SymbolValue(nl->Third(args));
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr type;
  int indexi = listutils::findAttribute(attrList,  intAttr, type);
  if(indexi==0){
    return listutils::typeError("Attribute name " + intAttr + " unknown ");
  }
  if(!CcInt::checkType(type)){
    return listutils::typeError("Attribute " + intAttr + " not of type int");
  }
  
  int indext = listutils::findAttribute(attrList, textAttr, type);
  if(indext==0){
    return listutils::typeError("Attribute name " + textAttr + " unknown ");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
    return listutils::typeError("Attribute " + textAttr + 
                                " not of type string or text");
  }

  ListExpr extAttr = nl->FourElemList(
              nl->TwoElemList( nl->SymbolAtom("ErrorCode"), 
                               listutils::basicSymbol<CcInt>()),
              nl->TwoElemList( nl->SymbolAtom("ErrorMsg") , 
                               listutils::basicSymbol<FText>()),
              nl->TwoElemList( nl->SymbolAtom("ResultList"), 
                               listutils::basicSymbol<FText>()),
              nl->TwoElemList( nl->SymbolAtom("Runtime"),
                                 listutils::basicSymbol<CcReal>()));

  ListExpr resAttrList = listutils::concat(attrList, extAttr);
  if(!listutils::isAttrList(resAttrList)){
     return listutils::typeError("Name conflics in result Type, one of the"
                                 " names ErrorCode, ErrorMsg, or ResultList "
                                 "already present in inpout tuple");
  }
  ListExpr extList = nl->TwoElemList( nl->IntAtom(indexi-1), 
                                      nl->IntAtom(indext-1));
  ListExpr resType = nl->TwoElemList(
                       listutils::basicSymbol<Stream<Tuple> >(),
                       nl->TwoElemList(
                            listutils::basicSymbol<Tuple>(),
                             resAttrList));
  return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                            extList,
                             resType);
}



template<class T>
class prcmdInfo: public CommandListener<string>{

  public:
    prcmdInfo(Word _s, int _intAttrPos, int _qAttrPos, ListExpr _tt):
       inStream(_s), stopped(false), noInTuples(-1), noOutTuples(0),
       serverAttrPos(_intAttrPos), queryAttrPos(_qAttrPos)
    {
      tt = new TupleType(_tt);
      inStream.open();
      runner = boost::thread(&prcmdInfo::run, this);
    }

    virtual ~prcmdInfo(){
       {
          boost::lock_guard<boost::mutex> guard(stopMutex);
          stopped = true;
       }
       // wait until runner is ready
       runner.join();
       // stop and delete running connectionTasks
       map<int, ConnectionTask<string>*>::iterator it1;
       for(it1 = serverTasks.begin(); it1!=serverTasks.end(); it1++){
          delete it1->second;
       }
       // delete non processed input tuples
       map<int,Tuple*>::iterator it2;
       {
          boost::lock_guard<boost::mutex> guard(inputTuplesMutex);
          for(it2 = validInputTuples.begin(); 
              it2!= validInputTuples.end(); it2++){
             it2->second->DeleteIfAllowed();
          } 
       }
       inStream.close();
       tt->DeleteIfAllowed();
    }


    Tuple* next(){

      // wait for next available tuple
       boost::unique_lock<boost::mutex> lock(resultMutex);
       while(pendingResults.empty()){
            cond.wait(lock);
        }

        Tuple* res = pendingResults.front();
        pendingResults.pop_front();
        return res;
    }
    

    void jobDone(int id, string& command, size_t no, int error, 
                 string& errMsg, string& resList, double runtime){

       Tuple* inTuple;
       {
          boost::lock_guard<boost::mutex> guard(inputTuplesMutex);
          if(validInputTuples.find(id)==validInputTuples.end()){
              return;
          }
          inTuple = validInputTuples[id];
          validInputTuples.erase(id);
       }
       createResultTuple(inTuple,error,errMsg, resList, runtime);
    }


  private:
    Stream<Tuple> inStream;
    bool stopped;
    int noInTuples;
    int noOutTuples;
    int serverAttrPos;
    int queryAttrPos;
    TupleType* tt;
    list<Tuple*> pendingResults;
    boost::thread runner;
    
    boost::mutex resultMutex; // waiting for new results
    boost::condition_variable cond; // waiting for new results

    boost::mutex stopMutex; // access to stopped variable

    map<int,ConnectionTask<string> *> serverTasks;
    map<int, Tuple*>  validInputTuples;

    boost::mutex tupleCreationMutex;
    boost::mutex inputTuplesMutex;

    void run(){
      Tuple* inTuple = inStream.request();
      int noTuples = 0;
      stopMutex.lock();
      while(inTuple!=0 && !stopped){
         stopMutex.unlock();
         noTuples++;
         processInputTuple(inTuple, noTuples);
         inTuple = inStream.request();
         stopMutex.lock();
      }
      if(stopped){
        if(inTuple){
           inTuple->DeleteIfAllowed();
        }
      } else {
         ;
      }
      stopMutex.unlock();
      setNoInTuples(noTuples);
    }

    void setNoInTuples(int num){
       tupleCreationMutex.lock();
       noInTuples = num;
       if(noInTuples == noOutTuples){
         tupleCreationMutex.unlock();
          {
            boost::lock_guard<boost::mutex> lock(resultMutex);
            pendingResults.push_back(0);
          }
          cond.notify_one();
       } else {
         tupleCreationMutex.unlock();
       } 
    } 



    void processInputTuple(Tuple* inTuple, int tupleId){
       CcInt* ServerNo = (CcInt*) inTuple->GetAttribute(serverAttrPos);
       T* Query = (T*) inTuple->GetAttribute(queryAttrPos);
       if(!ServerNo->IsDefined() || !Query->IsDefined()){
          createResultTuple(inTuple,-3, "Undefined Argument found", "",0.0);
          return;
       }
       int serverNo = ServerNo->GetValue();
       if(!algInstance->isValidServerNumber(serverNo)){
          createResultTuple(inTuple, -2, "Invalid server number", "",0.0);
          return; 
       }
       // process a valid tuple
       {
          boost::lock_guard<boost::mutex> guard(inputTuplesMutex);
          validInputTuples[tupleId] = inTuple; // store input tuple
       }
       // create a ConnectionTask for server if not present  
       if(serverTasks.find(serverNo)==serverTasks.end()){
          serverTasks[serverNo] = new ConnectionTask<string>(serverNo, this);
       }
       // append the command
       serverTasks[serverNo]->addCommand(tupleId, Query->GetValue());
    }


    void createResultTuple(Tuple* inTuple, int error, const string& errMsg, 
                           const string& resList, double runtime){

       {
          boost::lock_guard<boost::mutex> guard(stopMutex);
          if(stopped){
             inTuple->DeleteIfAllowed();
             return;
          }
       }

       {
          boost::lock_guard<boost::mutex> guard(tupleCreationMutex);
          Tuple* resTuple = new Tuple(tt);
          int noAttr = inTuple->GetNoAttributes();
          for(int i=0;i<noAttr;i++){
            resTuple->CopyAttribute(i,inTuple,i);
          }
          inTuple->DeleteIfAllowed();
          resTuple->PutAttribute(noAttr, new CcInt(error));
          resTuple->PutAttribute(noAttr+1, new FText(true,errMsg));
          resTuple->PutAttribute(noAttr+2, new FText(true,resList));
          resTuple->PutAttribute(noAttr+3, new CcReal(true,runtime));
          noOutTuples++;
          {
           boost::lock_guard<boost::mutex> lock(resultMutex);
            pendingResults.push_back(resTuple);
            if(noInTuples==noOutTuples){
               pendingResults.push_back(0);
            }
          }
       }
       // inform about a new result tuple
       cond.notify_one();
    }
};



/*
1.5.3 Value Mapping

*/

template<class T>
int prcmdVMT( Word* args, Word& result, int message,
               Word& local, Supplier s ){

  prcmdInfo<T>* li = (prcmdInfo<T>*) local.addr;
  switch(message){
    case OPEN: {
       if(li){
         delete li;
       }
       algInstance->initProgress();
       local.addr = new prcmdInfo<T>(args[0], 
                                  ((CcInt*)args[3].addr)->GetValue(),
                                  ((CcInt*)args[4].addr)->GetValue(),
                                  nl->Second(GetTupleResultType(s)));
       return 0;
    }
    case REQUEST:
           result.addr = li?li->next():0;
           return result.addr?YIELD:CANCEL;
    case CLOSE:
         if(li){
        //   boost::thread k(del<prcmdInfo<T> >,li); 
           delete li;
           local.addr =0;
           algInstance->finishProgress();
         }
         return 0;
  } 
  return -1;
};


/*
1.6.3 ValueMapping Array and Selection Function

*/

ValueMapping prcmdVM[] = {
  prcmdVMT<CcString>,
  prcmdVMT<FText>
};

int prcmdSelect(ListExpr args){
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr textType;
  listutils::findAttribute(attrList, 
                    nl->SymbolValue(nl->Third(args)), textType);
  return CcString::checkType(textType)?0:1;
}

/*
1.5.4  Specification

*/
OperatorSpec prcmdSpec(
     " stream(tuple) x attrName x attrName -> stream(tuple + E)",
     " _ prcmd [ _,_]",
     " Performs a a set of remote queries in parallel. Each incoming tuple "
     "is extended by some status information in attributes ErrorCode (int)"
     "and ErrorMsg (text),  the result of the query in form of "
     "a nested list (text attribute) as well as the runtime for processing "
     "the query. If small local stored objects should "
     "be used at the remote machine, the object name can be prefixed with "
     "a dollar sign.",
     " query intstream(0,3) namedtransformstream[S] "
     "extend[ Q : 'query plz feed count'] prcmd[S,Q] consume ");

/*
1.4.4 Operator instance

*/
Operator prcmdOp (
    "prcmd",             //name
     prcmdSpec.getStr(),         //specification
     2,
     prcmdVM,        //value mapping
     prcmdSelect,   //trivial selection function
     prcmdTM        //type mapping
);



/*
1.6 Operator ~sendFile~ 

Copies a local file to a remove server.  This version works  
serial.

1.6.1 Type Mapping

The arguments are the server number, the name of the local file as well as 
the name of the remote file.

*/
ListExpr sendFileTM(ListExpr args){
  string err = "int x {string, text} x {string, text} [ x bool] expected";

  if(!nl->HasLength(args,3) && !nl->HasLength(args,4)){
    return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(    !CcString::checkType(nl->Second(args)) 
      && !FText::checkType(nl->Second(args))){
    return listutils::typeError(err);
  } 
  if(    !CcString::checkType(nl->Third(args)) 
      && !FText::checkType(nl->Third(args))){
    return listutils::typeError(err);
  }

  if(nl->HasLength(args,4)){
     if(!CcBool::checkType(nl->Fourth(args))){
        return listutils::typeError(err);
     }
  }
  return listutils::basicSymbol<CcInt>();
}

template<class L , class R>
int sendFileVMT( Word* args, Word& result, int message,
               Word& local, Supplier s ){


   CcInt* Server = (CcInt*) args[0].addr;
   L* Local = (L*) args[1].addr;
   R* Remote = (R*) args[2].addr;
   CcBool al(true,false);
   CcBool* AllowOverwrite = &al;

   if(qp->GetNoSons(s)==4){
      AllowOverwrite = (CcBool*) args[3].addr;
   }
   result = qp->ResultStorage(s);
   CcInt* res = (CcInt*) result.addr;
   if(!Server->IsDefined() || ! Local->IsDefined() || !Remote->IsDefined()
      || !AllowOverwrite->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   int server = Server->GetValue();
   if(server < 0 || server >= algInstance->noConnections()){
      res->Set(true,-3);
      return 0;
   }
   res->Set(true, algInstance->sendFile(server, 
                                 Local->GetValue(), 
                                 Remote->GetValue(),
                                 AllowOverwrite->GetValue()));
   return 0;
}

/*
1.6.2 Specification

*/

OperatorSpec sendFileSpec(
     " int x {string, text} x {string, text} [x bool] -> int",
     " sendFile( serverNo, localFile, remoteFile) ",
     " Transfers a local file to the remote server. "
     " The local file is searched within the current "
     " directory (normally the bin directory of Secondo)"
     " if not given as an absolute path. The name of "
     " the remote file must be a relative path without "
     " any gobacks (..) inside. The file is stored "
     " on the server below the directory "
     " $SECONDO_HOME/filetransfers/$PID, where $SECONDO_HOME"
     " denotes the used database directory and $PID corresponds "
     "to the process id of the connected server (see also operator"
     " getSendFolder. The optional boolean argument determines whether "
     "an already existing file on the server should be overwritten. "
     "The return value is an error code. If the code is 0, the transfer "
     "was successful ", 
     " query sendFile( 0, 'local.txt', 'remote.txt' ");

/*
1.6.3 Value Mapping Array and Selection

*/

ValueMapping sendFileVM[] = {
  sendFileVMT<CcString, CcString>,
  sendFileVMT<CcString, FText>,
  sendFileVMT<FText, CcString>,
  sendFileVMT<FText, FText>
};

int sendFileSelect(ListExpr args){

  int n1 = CcString::checkType(nl->Second(args))?0:2;
  int n2 = CcString::checkType(nl->Third(args))?0:1;
  return n1+n2; 
}

/*
1.6.4 Operator instance

*/

Operator sendFileOp (
    "sendFile",             //name
     sendFileSpec.getStr(),         //specification
     4,
     sendFileVM,        //value mapping
     sendFileSelect,   //trivial selection function
     sendFileTM        //type mapping
);


/*
1.7 Operator requestFile

This operator request a file from a remote server.

1.7.1 Type Mapping

The arguments for this operator are the number of the server,
the name of the file at server side and the local file name.
The result is a boolean reporting the success of the file 
transfer.

*/

ListExpr requestFileTM(ListExpr args){
  string err = "int x {string, text} x {string, text} [ x bool] expected";

  if(!nl->HasLength(args,4) && !nl->HasLength(args,3) ){
    return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(    !CcString::checkType(nl->Second(args)) 
      && !FText::checkType(nl->Second(args))){
    return listutils::typeError(err);
  } 
  if(    !CcString::checkType(nl->Third(args)) 
      && !FText::checkType(nl->Third(args))){
    return listutils::typeError(err);
  }

  if(nl->HasLength(args,4)){ 
    // full argument set given
     if(!CcBool::checkType(nl->Fourth(args))){
        return listutils::typeError(err);
     }
  }

  return listutils::basicSymbol<CcInt>();
}

/*
1.7.2 Value Mapping

*/

template<class R , class L>
int requestFileVMT( Word* args, Word& result, int message,
               Word& local, Supplier s ){
   CcInt* Server = (CcInt*) args[0].addr;
   R* Remote = (R*) args[1].addr;
   L* Local = (L*) args[2].addr;

   CcBool al(true,false);
   CcBool* AllowOverwrite = &al;
   if(qp->GetNoSons(s)==4){
     AllowOverwrite = (CcBool*) args[3].addr;
   }
   result = qp->ResultStorage(s);
   CcInt* res = (CcInt*) result.addr;
   
   if(!Server->IsDefined() || ! Local->IsDefined() || !Remote->IsDefined()
      || !AllowOverwrite->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   int server = Server->GetValue();
   res->Set(true, algInstance->requestFile(server, 
                                 Remote->GetValue(), 
                                 Local->GetValue(),
                                 AllowOverwrite->GetValue()));
   return 0;
}


OperatorSpec requestFileSpec(
     " int x {string, text} x {string, text} [x bool] -> int",
     " requestFile( serverNo, remoteFile, localFile) ",
     " Transfers a remote file to the local file system. "
     " The local file is stored relative to current directory if"
     " its name is not given as an absolute path. The remote "
     " file is taken relative from $SECONDO_HOME/filetransfers."
     " For security reasons, the remote name cannot be an "
     " absolute path and cannot contains any gobacks (..).  The "
     "optional boolean argument determines whether a already existing "
     "local file should be overwritten by this operation. The return "
     "value is an error code. The transfer was successful if the "
     "return value is 0.",
     " query requestFile( 0, 'remote.txt', 'local.txt' ");

/*
1.6.3 Value Mapping Array and Selection

*/

ValueMapping requestFileVM[] = {
  requestFileVMT<CcString, CcString>,
  requestFileVMT<CcString, FText>,
  requestFileVMT<FText, CcString>,
  requestFileVMT<FText, FText>
};

int requestFileSelect(ListExpr args){

  int n1 = CcString::checkType(nl->Second(args))?0:2;
  int n2 = CcString::checkType(nl->Third(args))?0:1;
  return n1+n2; 
}

/*
1.6.4 Operator instance

*/

Operator requestFileOp (
    "requestFile",             //name
     requestFileSpec.getStr(),         //specification
     4,
     requestFileVM,        //value mapping
     requestFileSelect,   //trivial selection function
     requestFileTM        //type mapping
);



/*
1.7 Operators ~psendFile~ and ~prequestFile~

These operators transfers files between the local
file system and the remote server.

1.7.1 Type Mapping

The operator receives a stream of tuples at least having a number and 
text/string attributes. Furthermore three attribute names are required. 
The first attribute name points to an integer attribute specifing the 
connection where the file should be transferred to. The second attribute 
name points to the local filename within the tuple. This may be a string 
or a text. The third attribute name points to the name of the file which 
should be created at the remote server. 
The last two attribute names may be the same (in this case the local file 
name is equal to the remote file name.

Each input tuple is extened by a boolean and an text attribute. 
The boolean value points to the success of the file transfer and 
the text value will given an error message if fauled.

*/

ListExpr ptransferFileTM(ListExpr args){

  string err = "stream(tuple) x attrname x attrname x attrname expected";

  if(!nl->HasLength(args,4) && !nl->HasLength(args,5) ){
    return listutils::typeError(err + " (invalid number of attributes)");
  }
  ListExpr stream = nl->First(args);
  ListExpr attrs[] = { nl->Second(args), nl->Third(args), nl->Fourth(args)};

  if(!Stream<Tuple>::checkType(stream)){
    return listutils::typeError(err + " ( first arg is not a tuple stream)");
  }

  for(int i=0;i<3;i++){
    if(nl->AtomType(attrs[i])!=SymbolType){
       return listutils::typeError(err + " ( invalid attribute name found )");
    }
  }
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr type;
  string name = nl->SymbolValue(attrs[0]);
  int indexServer = listutils::findAttribute(attrList, name, type);
  if(!indexServer){
    return listutils::typeError(" attribute " + name + " not found");
  }
  if(!CcInt::checkType(type)){
    return listutils::typeError(" attribute " + name + " not an integer");
  }

  name = nl->SymbolValue(attrs[1]);
  int indexLocal = listutils::findAttribute(attrList, name, type);
  if(!indexLocal){
    return listutils::typeError(" attribute " + name + " not found");
  }
  if(!CcString::checkType(type) &&!FText::checkType(type)){
    return listutils::typeError(" attribute " + name + 
                                " is not of type string or text");
  }

  name = nl->SymbolValue(attrs[2]);
  int indexRemote = listutils::findAttribute(attrList, name, type);
  if(!indexRemote){
    return listutils::typeError(" attribute " + name + " not found");
  }
  if(!CcString::checkType(type) &&!FText::checkType(type)){
    return listutils::typeError(" attribute " + name + 
                                " is not of type string or text");
  }

  int indexOver = -1;
  ListExpr appendList;

  if(nl->HasLength(args,5)){ // full parameter set
    ListExpr overname = nl->Fifth(args);
    if(nl->AtomType(overname)!=SymbolType){
      return listutils::typeError("invalid attribute name detected");
    }
    name = nl->SymbolValue(overname);
    indexOver = listutils::findAttribute(attrList, name, type);
    if(!indexOver){
       return listutils::typeError(" attribute " + name + " not found");
    }
    if(!CcBool::checkType(type)){
       return listutils::typeError(" attribute " + name + 
                                   " is not of type bool");
    }
    appendList = nl->FourElemList(
                          nl->IntAtom(indexServer-1),
                          nl->IntAtom(indexLocal-1),
                          nl->IntAtom(indexRemote-1),
                          nl->IntAtom(indexOver-1));

  } else { // optional overwrite omitten
    appendList = nl->FiveElemList(
                          nl->IntAtom(-3),  
                         // dummy filling up missing attribute name
                          nl->IntAtom(indexServer-1),
                          nl->IntAtom(indexLocal-1),
                          nl->IntAtom(indexRemote-1),
                          nl->IntAtom(indexOver));
  }


  // input ok, generate output
  ListExpr extAttr = nl->TwoElemList(
                    nl->TwoElemList(
                            nl->SymbolAtom("ErrorCode"), 
                            listutils::basicSymbol<CcInt>()),
                    nl->TwoElemList(
                            nl->SymbolAtom("ErrMsg"), 
                            listutils::basicSymbol<FText>()) );

  ListExpr newAttrList = listutils::concat(attrList, extAttr);
  if(!listutils::isAttrList(newAttrList)){
    return listutils::typeError("Attribute ErrorCode or ErrMsg "
                                "already present in tuple");
  }
  ListExpr resultList = nl->TwoElemList(
                            listutils::basicSymbol<Stream<Tuple> >(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Tuple>(),
                                newAttrList));

  return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                            appendList,
                            resultList);  
}


/*
1.7.2 Class ~transferFileListener~

This class is an interface for listening running file transfers.

*/

class transferFileListener{

  public:


/*
~transferFinished~

This function is called if a single transfer is finished. 


*/   
    virtual void  transferFinished(Tuple* intuple, 
                               int errorCode, 
                               const string& msg ) =0;

/*
~setInputTupleNumber~

This function is called if the number of input tuples is known, i.e.
the input stream is completely processed or the processing is
canceled.

*/
    virtual void setInputTupleNumber(int no)=0;
};


/*
struct ~transfer~

This is a class holding the information about a single 
file transfer.

*/
struct transfer{
  transfer(Tuple* _tuple, const string& _local, const string& _remote,
           const bool _allowOverwrite):
    inTuple(_tuple), local(_local), remote(_remote), 
    allowOverwrite(_allowOverwrite){}

  transfer(): inTuple(0),local(""),remote(""),allowOverwrite(false){

  }

  Tuple* inTuple;
  string local;
  string remote;
  bool allowOverwrite;
};

/*
class ~FileTransferator~

This class organizes the transfer of files for a single connection.
All file transfers are handles sequentially. It is possible to add
new transfer tasks during a running file transfer. Furthermore, it
is possible to stop the processing. In the case of stop, the last
started transfer must be finished, since the interface does not
allow to interrupt a transfer.

*/


class sendFunctor{
 public:
  int operator()(const int server, const string& source, 
                 const string& target, const bool allowOverwrite){
    return algInstance->sendFile(server, source, target, allowOverwrite);
  }
};

class requestFunctor{
 public:
  int operator()(const int server, const string& source, 
                 const string& target, const bool allowOverwrite){
    return algInstance->requestFile(server, source, target, allowOverwrite);
  }
};


template<class T>
class fileTransferator{
  public:

/*
~Constructor~

This constructs a filetranferator for a given connection (server) with a
single listener.

*/
     fileTransferator(int _server, transferFileListener* _listener) :
         server(_server), listener(_listener), stopped(false){
         runnerThread = boost::thread(
                        boost::bind(&fileTransferator::run, this));
     }


/*
~Destructor~

This will destroy the instance of this class. 
The last running transfer (if there is one) will be finsihed during 
destroying this object.

*/
     ~fileTransferator(){
        stop();
        runnerThread.join();
        {
           boost::lock_guard<boost::mutex> guard(listAccess);
           while(!pendingtransfers.empty()){
              transfer t = pendingtransfers.front();
              t.inTuple->DeleteIfAllowed();
              pendingtransfers.pop_front();
           }
       }
     }

/*
~stop~

This will stop the thread processing pending transfers.
After calling this function, this object cannot be 
restarted. The only thing you can do after calling stop 
it to destroy it.

*/    
     void stop(){
         {
            boost::lock_guard<boost::mutex> lock(condmtx);
            stopped = true;
          }
          cond.notify_one();
     }


/*
~addtransfer~

Adds a new transfer to this list. If there is no other transfer
active, the transfer will start immediately. Otherwise the transfer
will be handled as the last one.

*/

      void addTransfer(Tuple* inTuple, const string& local, 
                       const string& remote, const bool allowOverwrite) {
          transfer newTransfer(inTuple, local, remote, allowOverwrite);
          {
            boost::lock_guard<boost::mutex> lock(listAccess);
            {
                boost::lock_guard<boost::mutex> lock(condmtx);
                pendingtransfers.push_back(newTransfer);
              }
          }
          cond.notify_one();
      }

  private:


/*
~run~

This function works within an own thread. It waits unil new transfer tasks
are available, tankes the first one, and starts the transfer.

*/
      void run(){
          boost::unique_lock<boost::mutex> lock(condmtx);
          while(!stopped){
               // wait for date
               listAccess.lock();
               while(!stopped && pendingtransfers.empty()){
                   listAccess.unlock();
                   cond.wait(lock); 
                   listAccess.lock();
               }
               listAccess.unlock();
               if(!stopped){
                   transfer t;
                   {
                      boost::lock_guard<boost::mutex> guard(listAccess);
                      t = pendingtransfers.front();
                      pendingtransfers.pop_front();   
                   }
                   int ret = functor(server, t.local, t.remote, 
                                     t.allowOverwrite);
                   if(listener){
                       if(ret == -3){ // algebra specific error code
                          listener->transferFinished(t.inTuple, ret, 
                                              "Conection number invalid.");
                       }  else if(ret==0){ // no error
                          listener->transferFinished(t.inTuple, ret, "");
                       } else { // general error
                          listener->transferFinished(t.inTuple, ret, 
                                   SecondoInterface::GetErrorMessage(ret) );
                       }
                   } else {
                      t.inTuple->DeleteIfAllowed();
                   }
               }
          }
      }

    int server;
    transferFileListener* listener;
    bool stopped;
    list<transfer>  pendingtransfers;
    boost::mutex listAccess;
    boost::thread runnerThread;
    boost::mutex condmtx;
    boost::condition_variable cond;
    T functor;
};


/*

Class ~ptransferFileInputProcessor~

This class processes in input tuples of the stream for the
parallel file transfer.

*/

template<class L, class R, class T>
class ptransferFileInputProcessor{
     public:

/*
~Constructors~

*/
        ptransferFileInputProcessor(Word& _stream, 
                                   int _serverIndex, int _localIndex, 
                       int _remoteIndex, int _overwriteIndex,
                       transferFileListener* _listener):
          stream(_stream), serverIndex(_serverIndex), localIndex(_localIndex),
          remoteIndex(_remoteIndex), overwriteIndex(_overwriteIndex),
          listener(_listener), stopped(false) 
       {
          stream.open();
        }



/*
~Destructors~

*/
        ~ptransferFileInputProcessor(){
           stream.close();
           // stop active threads
           typename map<int, fileTransferator<T>*>::iterator it;
           for(it = activeThreads.begin(); it!=activeThreads.end(); it++){
               it->second->stop();
           }
           // delete active threads
           for(it = activeThreads.begin(); it!=activeThreads.end(); it++){
               delete it->second;
           }
         }


/*
~stop~

This will stop the processing of input tuples.

*/
         void stop(){
            stopped=true;
         }


/*
~run~

This function processes the tuples of the input stream.

*/
         void run(){
            Tuple* inTuple;
            inTuple = stream.request();
            int noInTuples = 0; 
            while(inTuple && !stopped) {
               noInTuples++;
               CcInt* Server = (CcInt*) inTuple->GetAttribute(serverIndex);
               L* Local = (L*) inTuple->GetAttribute(localIndex);
               R* Remote = (R*) inTuple->GetAttribute(remoteIndex);
               CcBool* Overwrite = 0;
               CcBool defaultOverwrite(true,false);
               if(overwriteIndex>=0){
                  Overwrite = (CcBool*) inTuple->GetAttribute(overwriteIndex);
               } else {
                  Overwrite = &defaultOverwrite;
               }

               if(!Server->IsDefined() || !Local->IsDefined()
                  || !Remote->IsDefined() || !Overwrite->IsDefined()){
                   processInvalidTuple(inTuple, "undefined value found");
               } else {
                  int server = Server->GetValue(); 
                  string localname = Local->GetValue();
                  string remotename = Remote->GetValue();
                  bool overwrite = Overwrite->GetValue();
                  if(server < 0 || server >= algInstance->noConnections()){
                     processInvalidTuple(inTuple, "invalid server number");
                  } else {
                    processValidTuple(inTuple, server, localname, 
                                      remotename, overwrite);
                  }
               }
               inTuple = stream.request();
            }
            if(inTuple){
                inTuple->DeleteIfAllowed();
            }
            listener->setInputTupleNumber(noInTuples);
          }


    private:
       Stream<Tuple> stream;  // input stream
       int serverIndex;       // where to find server index in input tuple
       int localIndex;        // where to find the local file name
       int remoteIndex;       // where to find the remote file name
       int overwriteIndex;    // where to find the remote file name, 
                              // < 0 if default
       transferFileListener* listener; // reference to the listener object 
       bool stopped;          // flag for activiness
       map<int, fileTransferator<T>*>
                             activeThreads; //threads for each connection


/*
~processInvalidTuple~

This function is called if an invalid tuple is determined in the input stream.
This is the case if one of the required attributes is undefined or has an
invalid value, e.g., a negative server number.

*/
       void processInvalidTuple(Tuple* inTuple, const string& msg){
            if(listener){
               listener->transferFinished(inTuple, -3, msg);
            } else {
                inTuple->DeleteIfAllowed();
            }
       }


/*
~processValidTuple~

This function is called for each valid input tuple. If there not yet
a processing instance for the given server number, a new instance 
is created. The transfer task is added to the running instance for this
server.


*/
       void processValidTuple(Tuple* inTuple, const int server, 
                              const string& local, const string& remote,
                              const bool allowOverwrite){

          if(activeThreads.find(server)==activeThreads.end()){
              fileTransferator<T>* fi =
                               new fileTransferator<T>(server, listener);
              activeThreads[server] = fi; 
          } 
          activeThreads[server]->addTransfer(inTuple, 
                                           local,remote, allowOverwrite);
       }
  };



/*
Class ~ptransferLocal~

This is the local info class for the ptransfer operator. 

*/

template<class L, class R, class T>
class ptransferLocal: public transferFileListener {

  public:


/*
~Constructor~

*/

     ptransferLocal(Word& stream, int _serverindex,
                    int _localindex, int  _remoteindex,
                    int _overwrite, ListExpr _tt) {
        inputTuples = -1; // unknown number of input tuples
        outputTuples = 0; // up to now, there is no output tuple
        tt = new TupleType(_tt);
        inputProcessor = 
              new ptransferFileInputProcessor<L,R,T>(stream,_serverindex, 
                                           _localindex, _remoteindex,
                                           _overwrite, this);
        runner = new boost::thread( boost::bind(
                &ptransferFileInputProcessor<L,R,T>::run, inputProcessor));
     }

/*
~Destructor~

This will wait until all running transfers are processed. This does not
includes instantiates but not yet started transfers. All present but
not used result tuples are deleted also.

*/
     virtual ~ptransferLocal(){
        inputProcessor->stop(); // stop processing
        runner->join(); // wait until started jobs are done
        delete runner;
        delete inputProcessor;

        list<Tuple*>::iterator it;
        // remove already created result tuples
        listAccess.lock();
        for(it = resultList.begin();it!=resultList.end();it++){
            (*it)->DeleteIfAllowed();
        }
        listAccess.unlock();
        tt->DeleteIfAllowed();
     }


/*
~next~

This function provides the next result tuple.
If there is no result tuple available, it waits 
until it is. 

*/

     Tuple* next(){
        boost::unique_lock<boost::mutex> lock(resultMutex);
        listAccess.lock();
        while(resultList.empty()){
            listAccess.unlock();
            resultAvailable.wait(lock);
            listAccess.lock();
        }
        Tuple* res = resultList.front();
        resultList.pop_front();
        listAccess.unlock();
        return res;
     } 


/*
~transferFinished~

This function is called if a transfer has been finished. 
It will created a new result tuple from the input tuple,
the success flag and the error message.

*/

     // callback function for a single finished transfer
     virtual void transferFinished(Tuple* inTuple,int errorCode, 
                                   const string& msg){

        tupleCreationMtx.lock();
        Tuple* resTuple =  new Tuple(tt);
        // copy original attributes
        for(int i=0;i<inTuple->GetNoAttributes(); i++){
           resTuple->CopyAttribute(i,inTuple,i);
        } 
        resTuple->PutAttribute(inTuple->GetNoAttributes(), 
                               new CcInt(true, errorCode));
        resTuple->PutAttribute(inTuple->GetNoAttributes() + 1, 
                               new FText(true, msg));

        inTuple->DeleteIfAllowed();

        {
           boost::lock_guard<boost::mutex> lock(resultMutex);

           listAccess.lock();
           resultList.push_back(resTuple);
           listAccess.unlock();

           // update counter
           outputTuples++;
          if(inputTuples==outputTuples){
             // this was the last tuple to create
             listAccess.lock();
             resultList.push_back(0);
             listAccess.unlock();
           }
        }
        tupleCreationMtx.unlock();
        resultAvailable.notify_one();
     }


/*
~setInputTupleNumber~

This function is called if the number of input tuples is known.


*/   
     virtual void setInputTupleNumber(int i){
        if(i==0){ // we cannot expect any results
           {
             boost::lock_guard<boost::mutex> lock(resultMutex);
             resultList.push_back(0);
           }
           // inform about new tuple.
           resultAvailable.notify_one(); 
        } else {
          assert(i>0);
          countMtx.lock();
          inputTuples = i;
          if(inputTuples == outputTuples){
             // all output tuples was generated
             countMtx.unlock();
             {
               boost::lock_guard<boost::mutex> lock(resultMutex);
               listAccess.lock();
               resultList.push_back(0);
               listAccess.unlock();
             }
             // inform about new tuple.
             resultAvailable.notify_one(); 
          } else {
             countMtx.unlock();
          }
        }
     }


  private:
     TupleType* tt;
     bool finished;
     ptransferFileInputProcessor<L,R,T>* inputProcessor;
     boost::thread* runner;
     list<Tuple*> resultList;
     boost::condition_variable resultAvailable;
     boost::mutex resultMutex;
     boost::mutex listAccess;
     boost::mutex tupleCreationMtx;
     int inputTuples;
     int outputTuples;
     boost::mutex countMtx;
};


/*
~Value Mapping~

*/

template<class L , class R, class T>
int ptransferFileVMT( Word* args, Word& result, int message,
               Word& local, Supplier s ){

   ptransferLocal<L,R,T>* li = (ptransferLocal<L,R,T>*) local.addr;

   switch(message){
      case OPEN: {
               if(li) delete li;
               int serv = ((CcInt*)args[5].addr)->GetValue();
               int loc = ((CcInt*)args[6].addr)->GetValue();
               int rem = ((CcInt*)args[7].addr)->GetValue();
               int over = ((CcInt*) args[8].addr)->GetValue(); 
               ListExpr tt = nl->Second(GetTupleResultType(s));
               local.addr = new ptransferLocal<L,R,T>(args[0], serv, loc, 
                                                     rem, over, tt);
               return 0;
          }
      case REQUEST:
               result.addr = li?li->next():0;
               return result.addr?YIELD:CANCEL;
      case CLOSE:
             if(li){
                delete li;
                local.addr = 0;                  
             }
             return 0;
   }
   return -1;
}

/*
1.7.2 Specification

*/

OperatorSpec psendFileSpec(
     " stream(tuple) x attrName x attrName x attrName [x attrName] "
     "-> stream(Tuple + Ext)",
     " _ psendFile[ ServerAttr, LocalFileAttr, RemoteFileAtt"
     " [, overwriteAttr] ]  ",
     " Transfers local files to remote servers in parallel. The ServerAttr "
     " must point to an integer attribute specifying the server number. "
     " The localFileAttr and remoteFileAttr arguments mark the attribute name"
     " for the local and remote file name, respectively. Both arguments can be"
     " of type text or string. overWriteAttr is the name of a boolean "
     "attribute specifying whether an already existing remote file should be "
     "overwritten.", 
     " query fileRel feed psendFile[0, LocalFile, RemoteFile] consume");


OperatorSpec prequestFileSpec(
     " stream(tuple) x attrName x attrName x attrName [x attrName] "
     "-> stream(Tuple + Ext)",
     " _ prequestFile[ ServerAttr, RemoteFileAttr, "
     "LocalFileAttr[,overwriteAttr] ]  ",
     " Transfers remote files to local file system  in parallel. "
     "The ServerAttr "
     " must point to an integer attribute specifying the server number. "
     " The localFileAttr and remoteFileAttr arguments mark the attribute name"
     " for the local and remote file name, respectively. Both arguments can be"
     " of type text or string. The optional overwriteAttr of type bool " 
     "specifies whether an already existing local file should be overwritten.",
     " query fileRel feed prequestFile[0, LocalFile, RemoteFile] consume");

/*
1.7.3 Value Mapping Array and Selection

*/

ValueMapping psendFileVM[] = {
  ptransferFileVMT<CcString, CcString, sendFunctor>,
  ptransferFileVMT<CcString, FText, sendFunctor>,
  ptransferFileVMT<FText, CcString, sendFunctor>,
  ptransferFileVMT<FText, FText, sendFunctor>
};

ValueMapping prequestFileVM[] = {
  ptransferFileVMT<CcString, CcString, requestFunctor>,
  ptransferFileVMT<CcString, FText, requestFunctor>,
  ptransferFileVMT<FText, CcString, requestFunctor>,
  ptransferFileVMT<FText, FText, requestFunctor>
};



int ptransferFileSelect(ListExpr args){
  string name1 = nl->SymbolValue(nl->Third(args));
  string name2 = nl->SymbolValue(nl->Fourth(args));
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr type1, type2;
  listutils::findAttribute(attrList, name1, type1);
  listutils::findAttribute(attrList, name2, type2);
  int n1 = CcString::checkType(type1)?0:2;
  int n2 = CcString::checkType(type2)?0:1;

  return n1+n2; 
}

/*
1.6.4 Operator instance

*/

Operator psendFileOp (
    "psendFile",             //name
     psendFileSpec.getStr(), //specification
     4,
     psendFileVM,        //value mapping
     ptransferFileSelect,   //trivial selection function
     ptransferFileTM        //type mapping
);

Operator prequestFileOp (
    "prequestFile",             //name
     prequestFileSpec.getStr(), //specification
     4,
     prequestFileVM,        //value mapping
     ptransferFileSelect,   //trivial selection function
     ptransferFileTM        //type mapping
);

/*
1.7 Operators ~getRequestFolder~ and ~getTransferFolder~

These operator are indeed to get the folder for sending and 
receiving files on remote servers.

1.7.1 Type Mapping

*/
ListExpr getFoldersTM(ListExpr args){

  string err = "int expected";

  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->First(args))){
    return listutils::typeError(err);
  } 
  return listutils::basicSymbol<FText>();
}


/*
1.7.2 Value Mapping

*/

template<bool send>
int getFolderVM(Word* args, Word& result, int message,
                Word& local, Supplier s ){

  CcInt* server = (CcInt*) args[0].addr;
  result = qp->ResultStorage(s);
  FText* res = (FText*) result.addr;
  if(!server->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  int ser = server->GetValue();
  if(ser<0 || ser >=algInstance->noConnections()){
     res->SetDefined(false);
     return 0;
  }
  if(send) {
     res->Set(true, algInstance->getSendFolder(ser));
  } else {
     res->Set(true, algInstance->getRequestFolder(ser));
  }
  return 0;
}


OperatorSpec getRequestFolderSpec(
     " int -> text ",
     " getRequestFolder(_)  ",
     " Returns the name of the folder on the server for a given connection"
     " from where files are requested. ",
     " query getRequestFolder(0)");

OperatorSpec getSendFolderSpec(
     " int -> text ",
     " getSendFolder(_)  ",
     " Returns the name of the folder on the server for a given connection"
     " into which files are written. ",
     " query getSendFolder(0)");


Operator getRequestFolderOp (
    "getRequestFolder",             //name
     getRequestFolderSpec.getStr(), //specification
     getFolderVM<false>,        //value mapping
     Operator::SimpleSelect,   //trivial selection function
     getFoldersTM        //type mapping
);

Operator getSendFolderOp (
    "getSendFolder",             //name
     getSendFolderSpec.getStr(), //specification
     getFolderVM<true>,        //value mapping
     Operator::SimpleSelect,   //trivial selection function
     getFoldersTM        //type mapping
);


/*
1.9 Operator ~pconnect~

This operator connects to different Servers in parallel.

1.9.1 Type Mapping

The operator receives an tuple stream and three attribute names.
The first attribute determines the host name and points to a string
or a text attribute. The second attribute name determines the port und
points to an integer attribute. The third attribute name determines the
SecondoConfig.ini file and may be of type text or of type string.

*/

ListExpr pconnectTM(ListExpr args){

  string err = "stream(tuple) x ID x ID x ID  expected";
  if(!nl->HasLength(args,4) && !nl->HasLength(args,5)){
    return listutils::typeError(err);
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
    return listutils::typeError("The first attribute is not a tuple stream");
  }
  ListExpr a1 = nl->Second(args);
  ListExpr a2 = nl->Third(args);
  ListExpr a3 = nl->Fourth(args);
  if(nl->AtomType(a1) != SymbolType || 
     nl->AtomType(a2) != SymbolType ||
     nl->AtomType(a3) != SymbolType){
    return listutils::typeError(err);
  }

  string n1 = nl->SymbolValue(a1);
  string n2 = nl->SymbolValue(a2);
  string n3 = nl->SymbolValue(a3);

  ListExpr type;
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));

  int hostIndex = listutils::findAttribute(attrList, n1, type);
  if(!hostIndex){
    return listutils::typeError("Attribute " + n1 + " not found");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + n1 + 
                                 " not of type string or type text");
  }
  int portIndex = listutils::findAttribute(attrList, n2, type);
  if(!portIndex){
    return listutils::typeError("Attribute " + n2 + " not found");
  }
  if(!CcInt::checkType(type) ){
     return listutils::typeError("Attribute " + n2 + 
                                 " not of type int");
  }
  int configIndex = listutils::findAttribute(attrList, n3, type);
  if(!configIndex){
    return listutils::typeError("Attribute " + n3 + " not found");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + n3 + 
                                 " not of type string or type text");
  }

  if(listutils::findAttribute(attrList, "CNo",type)){
    return listutils::typeError("incoming tuple stream already "
                                "contains CNo attribute");
  }

  ListExpr resAttrList = listutils::concat(attrList, 
                              nl->OneElemList(nl->TwoElemList(
                                      nl->SymbolAtom("CNo"), 
                                     listutils::basicSymbol<CcInt>())));

  ListExpr appendList;
  if(nl->HasLength(args,5)){
     if(!CcBool::checkType(nl->Fifth(args))){
       return listutils::typeError("5th argument must be of type bool");
     }
     appendList = nl->ThreeElemList(
                           nl->IntAtom(hostIndex -1),
                           nl->IntAtom(portIndex -1),
                           nl->IntAtom(configIndex -1));
  } else {
     appendList = nl->FourElemList(
                           nl->BoolAtom(false),
                           nl->IntAtom(hostIndex -1),
                           nl->IntAtom(portIndex -1),
                           nl->IntAtom(configIndex -1));

  }

  ListExpr resList = nl->TwoElemList(
                      listutils::basicSymbol<Stream<Tuple> >(),
                      nl->TwoElemList(
                           listutils::basicSymbol<Tuple>(),
                           resAttrList));
  return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                            appendList,
                            resList); 

}


/*
1.7.2 auxiliary classes


class ~ConnectionListener~

This class contains a callback function that is called when
a attempt for a connection is done. If the attempt was
successful, no is the connection number, other wise no
is negative.

*/
class ConnectionListener{

  public:
    virtual void connectionDone(Tuple* inputTuple, 
                                int no, ConnectionInfo* ci ) =0;

};


/*
Class ~Connector~

This class implements a single attempt to connect 
with a remote server. Directly after calling the constructor,
a thread is start doing the connection attempt.

*/

class Connector{

  public:
     Connector(Tuple* _inTuple, const int _inTupleNo,
               const string& _host, const int _port, 
               const string& _config, ConnectionListener* _listener):
       inTuple(_inTuple), inTupleNo(_inTupleNo), host(_host), port(_port),
       config(_config), 
       listener(_listener){
       mythread = boost::thread(
                      boost::bind(&Connector::run, this));
     }

     ~Connector(){
         mythread.join();
      }

  private:
     Tuple* inTuple;
     int inTupleNo;
     string host;
     int port;
     string config;
     ConnectionListener* listener;
     boost::thread mythread;


     void run(){
        if(port<0 || host.empty() || config.empty()){
           listener->connectionDone(inTuple, inTupleNo, 0);
           return;
        }

        NestedList* mynl = new NestedList();
        SecondoInterfaceCS* si = new SecondoInterfaceCS(false, mynl,true);
        si->setMaxAttempts(4);
        si->setTimeout(1);
        string errMsg;
        if(!si->Initialize( "", "", host, stringutils::int2str(port),
                           config,errMsg, true)){
           listener->connectionDone(inTuple, inTupleNo, 0);
           delete si;
           si = 0;
           delete mynl;
        } else {
           ConnectionInfo* ci = new ConnectionInfo(host,port,config,si,mynl);
           listener->connectionDone(inTuple, inTupleNo, ci);
        }
     }
};


class ConnectInfoElem{

public:

  ConnectInfoElem(Tuple* _inTuple, int _inTupleNo, ConnectionInfo* _ci):
   inTuple(_inTuple), inTupleNo(_inTupleNo), ci(_ci){}


   bool operator<(const ConnectInfoElem& rhs)const{
      return inTupleNo < rhs.inTupleNo;   
   }
   
   bool operator==(const ConnectInfoElem& rhs) const{
      return inTupleNo == rhs.inTupleNo;   
   }

   bool operator>(const ConnectInfoElem& rhs)const{
      return inTupleNo > rhs.inTupleNo;   
   }

  Tuple* inTuple;
  int inTupleNo;
  ConnectionInfo* ci;

};


template<class H, class C>
class pconnectLocal : public ConnectionListener{
  public:
    pconnectLocal(Word& _stream, int _hostIndex, int _portIndex,
                  int _configIndex, ListExpr _tt, bool _det):
       stream(_stream), hostIndex(_hostIndex), portIndex(_portIndex), 
       configIndex(_configIndex), inputTuples(-1), outputTuples(0), 
       stopped(false), det(_det) {
        tt = new TupleType(_tt);
        stream.open(); 
        runnerThread = boost::thread(
                        boost::bind(&pconnectLocal::run, this));
    }

    virtual ~pconnectLocal(){
       stop();
       runnerThread.join();
       stream.close();
       for(size_t i=0;i<connectors.size(); i++){
         delete connectors[i];
       }
       tt->DeleteIfAllowed(); 
       cachemut.lock();
       while(!cache.empty()){
         ConnectInfoElem elem = cache.top();
         cache.pop();
         elem.inTuple->DeleteIfAllowed();
         delete elem.ci;
       }
       cachemut.unlock();
    }


    Tuple* next(){
       boost::unique_lock<boost::mutex> lock(waitmut);
       while(resultTuples.empty()){
          cond.wait(lock);
       }
       Tuple* res = resultTuples.front();
       resultTuples.pop_front();
       return res;
    } 

    void stop(){
      stopped = true;
    }


    virtual void connectionDone(Tuple* inTuple, int inTupleNo, 
                                ConnectionInfo* ci){
       if(!det){
          connectionDoneNonDet(inTuple,inTupleNo, ci);
       } else {
          connectionDoneDet(inTuple, inTupleNo, ci);      
       }
    }

       

  private:
     Stream<Tuple> stream;
     int hostIndex;
     int portIndex;
     int configIndex;
     int inputTuples;
     int outputTuples;
     bool stopped;
     bool det;
     TupleType* tt;
     boost::mutex mut;
     boost::mutex waitmut;
     boost::mutex outmut;
     boost::condition_variable cond;
     list<Tuple*> resultTuples;
     boost::thread runnerThread;
     vector<Connector*> connectors;
     // cache for deterministic variant
     priority_queue<ConnectInfoElem, 
                    vector<ConnectInfoElem>, 
                    greater<ConnectInfoElem> > cache;
     boost::mutex cachemut;
     

     void run(){
        Tuple* inTuple;
        int noTuples = 0;
        inTuple = stream.request();
        while(!stopped && inTuple!=0){
           noTuples++;
           H* Host = (H*) inTuple->GetAttribute(hostIndex);
           CcInt* Port = (CcInt*) inTuple->GetAttribute(portIndex);
           C* Config = (C*) inTuple->GetAttribute(configIndex);
           if(   !Host->IsDefined() || !Port->IsDefined() 
              || !Config->IsDefined()){
              Connector* connector = new Connector(inTuple, noTuples,
                                                    "",-1,"",this);
              connectors.push_back(connector);
           } else {
              Connector* connector = new Connector(inTuple, 
                                        noTuples, Host->GetValue(), 
                                  Port->GetValue(), Config->GetValue(), this);
              connectors.push_back(connector);
           }
           inTuple = stream.request();
        }
        if(inTuple){
          inTuple->DeleteIfAllowed();
        }
        outmut.lock();
        inputTuples = noTuples;
        outmut.unlock();
        if(inputTuples == outputTuples){
          connectionDone(0,noTuples+1,0);
        }    
     }

/*
~connectionDoneNonDet~

In this variant, the order of output tuples as well as the order 
of servernumbers are not deterministic, i.e. the first connection which 
is established will be the
first connection and the first output tuple.

*/
    void connectionDoneNonDet(Tuple* inTuple, int inTupleNo, 
                              ConnectionInfo* ci){
        if(!inTuple){
            {
              boost::lock_guard<boost::mutex> lock(waitmut);
              resultTuples.push_back(0);
            }
            cond.notify_one();
            return;
        }
        mut.lock();
        if(stopped){ // do not produce more output tuples
           inTuple->DeleteIfAllowed();
           mut.unlock();
           return; 
        }
        mut.unlock();
        outmut.lock();
        Tuple* resTuple = new Tuple(tt);
        // copy attributes
        int noa = inTuple->GetNoAttributes();
        for(int i=0;i<noa ; i++){
           resTuple->CopyAttribute(i,inTuple,i);   
        }
        inTuple->DeleteIfAllowed();
        int no = algInstance->addConnection(ci);
        resTuple->PutAttribute(noa, new CcInt(no));
        {
          boost::lock_guard<boost::mutex> lock(waitmut);
          resultTuples.push_back(resTuple);
          outputTuples++;
          if(inputTuples==outputTuples){
             stop();
             resultTuples.push_back(0); // add end marker
          }
        }
        outmut.unlock();
        cond.notify_one();
    }

/*
~connectionDoneDet~

Deterministic variant. Here, the output tuples as well as the 
server numbers are determined by the 

*/
    void connectionDoneDet(Tuple* inTuple, int inTupleNo, 
                              ConnectionInfo* ci){


       cachemut.lock();
       cache.push(ConnectInfoElem(inTuple,inTupleNo,ci));
       ConnectInfoElem elem = cache.top();
       bool produce = elem.inTupleNo == outputTuples+1;
       while( produce && !stopped){
           cache.pop();
           connectionDoneNonDet(elem.inTuple,elem.inTupleNo, elem.ci);
           elem = cache.top();
           produce = elem.inTupleNo == outputTuples+1;
       }
       cachemut.unlock(); 
    }

};


/*
1.7.3 Value Mapping

*/
template<class H, class C>
int pconnectVMT(Word* args, Word& result, int message,
                Word& local, Supplier s ){

   pconnectLocal<H,C>* li = (pconnectLocal<H,C>*) local.addr;
   switch(message){
     case OPEN: {
            if(li) delete li;
            int hostIndex = ((CcInt*)args[5].addr)->GetValue();
            int portIndex = ((CcInt*)args[6].addr)->GetValue();
            int configIndex = ((CcInt*)args[7].addr)->GetValue();
            CcBool* det = (CcBool*) args[4].addr;
            if(!det->IsDefined()){
               local.addr = 0;
               return 0;
            }
            ListExpr tt = nl->Second(GetTupleResultType(s));
            local.addr  = new pconnectLocal<H,C>( args[0], hostIndex,  
                                  portIndex, configIndex, tt, det->GetValue());
            return 0;
          }
     case REQUEST:
            result.addr = li?li->next():0;
            return result.addr?YIELD:CANCEL;
     case CLOSE:
            if(li){
              delete li;
              local.addr = 0;
            }
            return 0;
   }
   return -1;
}

/*
1.7.3 Value Mapping Array and Selection

*/
ValueMapping pconnectVM[] = {
  pconnectVMT<CcString, CcString>,
  pconnectVMT<CcString, FText>,
  pconnectVMT<FText, CcString>,
  pconnectVMT<FText, FText>
};



int pconnectSelect(ListExpr args){
  string name1 = nl->SymbolValue(nl->Second(args));// hostattr
  string name2 = nl->SymbolValue(nl->Fourth(args));// configattr
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr type1, type2;
  listutils::findAttribute(attrList, name1, type1);
  listutils::findAttribute(attrList, name2, type2);
  int n1 = CcString::checkType(type1)?0:2;
  int n2 = CcString::checkType(type2)?0:1;
  return n1+n2; 
}

/*
1.7.4 Specification

*/

OperatorSpec pconnectSpec(
     " stream(tuple(X)) x attrName x attrName x attrName "
     "-> stream(tuple(X + (CNo int))) ",
     " _ pconnect[_,_,_]  ",
     " Creates connection to Secondo servers in parallel.  "
     " The first argument is a stream of tuples. The parameters are "
     " the attribute name for the host (string or text), the attribute "
     " name for the port (int), and the attribute name for the "
     " local configuration file (string or text). By this operator, the "
     " input tuples are extended by the connection number. If the number is"
     " negative, no connection could be built up for this input tuple. "
     " The order of the output tuples depends on the connection speeds.",
     " query ConTable feed pconnect[Host, Port, Config] consume");

/*
1.7.5 Operator instance

*/

Operator pconnectOp (
    "pconnect",             //name
     pconnectSpec.getStr(),         //specification
     4,
     pconnectVM,        //value mapping
     pconnectSelect,   //trivial selection function
     pconnectTM        //type mapping
);


/*
1.8 Operator prquery

This operators performs the same query on a set of servers. The result type 
of the query is asked by a specified server. It's assumed that each server 
returns the same type and this type must be in kind DATA.

1.8.1 Type Mapping

The type mapping is

stream(int) x {string.text} x {int} -> stream(tuple(Server : int, Resul : DATA))

This operator uses argument evaluation in type mapping.

*/

bool  getQueryType(const string& query1, int serverNo, 
                   ListExpr& result, string& errMsg){
   string query = query1; 

   stringutils::trim(query);
   if(!stringutils::startsWith(query,"query")){
     return false;
   }
   query = query.substr(5); // remove leading query
   stringutils::trim(query);
   query = "query (" + query + ") getTypeNL";

   int error;
   ListExpr reslist;
   double runtime;
   bool ok = algInstance->simpleCommand(serverNo, query, 
                                 error, errMsg, reslist, true, runtime);
   if(!ok){
      errMsg = " determining type command failed (invalid server number)";
      return false;
   }
   if(error!=0){
      errMsg = " error in type detecting command (" + errMsg+")";
      return false;
   }
   if(!nl->HasLength(reslist,2)){
      errMsg = "Invalid result get from remote server";
      return false;
   }
   if(!FText::checkType(nl->First(reslist))){
      errMsg = "Invalid result type for getTypeNL";
      return false;
   }
   if(nl->AtomType(nl->Second(reslist))!=TextType){
     errMsg = "getTypeNL returns invalid/undefined result";
     return false;
   }
   string typeList = nl->Text2String(nl->Second(reslist));
   ListExpr resType;
   {
      boost::lock_guard<boost::mutex> guard(nlparsemtx);
      if(!nl->ReadFromString(typeList,resType)){
         errMsg = "getTypeNL returns no valid list expression";
         return false;
      }
   }   
   if(nl->HasLength(resType,2)){
     ListExpr first = nl->First(resType);
     if(listutils::isSymbol(first, Stream<Tuple>::BasicType())){
        errMsg = "remote result is a stream";
        return false;
     }
   }
   result = resType;
   errMsg = "";
   return true;
}

ListExpr prqueryTM(ListExpr args){
  string err="stream(int) x {string.text} x {int} expected";
  if(!nl->HasLength(args,3)){
   return listutils::typeError(err + "( wrong number of args)" );
  }
  ListExpr stream = nl->First(nl->First(args)); // ignore query part
  if(!Stream<CcInt>::checkType(stream)){
    return listutils::typeError(err);
  }

  ListExpr second = nl->Second(args);

  // determine query
  string query;
  bool ok;
  if(FText::checkType(nl->First(second))){
      ok = getValue<FText>(nl->Second(second),query);
  } else if(CcString::checkType(nl->First(second))){
     ok = getValue<CcString>(nl->Second(second),query);
  } else {
      return listutils::typeError(err);
   }
   if(!ok){
      return listutils::typeError("Value of second argument invalid");
   }

   // determine type server
   ListExpr third = nl->Third(args);
   if(!CcInt::checkType(nl->First(third))){
      return listutils::typeError(err);
   }

   int server;
   if(!getValue(nl->Second(third),server)){
      return listutils::typeError("Could not determine server "
                                  "providing the type");
   }

   string errMsg;
   ListExpr resType;
   if(!getQueryType(query,server,resType,errMsg)){
      return listutils::typeError(errMsg);
   }


   // check whether resType is in kind data
   if(!listutils::isDATA(resType)){
      return listutils::typeError("Query result " + nl->ToString(resType) + 
                                  "  not in kind data");
   }

   ListExpr attrList = nl->TwoElemList(
                            nl->TwoElemList(
                               nl->SymbolAtom("ServerNo"),
                               listutils::basicSymbol<CcInt>() ),
                            nl->TwoElemList(
                               nl->SymbolAtom("Result"),
                               resType));

   ListExpr resList = nl->TwoElemList(
             listutils::basicSymbol<Stream<Tuple> >(),
             nl->TwoElemList( listutils::basicSymbol<Tuple>(), attrList));


    return resList; 


}


/*
1.8.2 LocalInfoClass

*/


class PRQueryInfo : public CommandListener<ListExpr>{
  public:
    PRQueryInfo(Word& _stream , const string& _query,  ListExpr _tt) : 
      stream(_stream), query(_query){
       tt = new TupleType(_tt);
       stream.open();
       stop = false;
       noInputs=-1;
       noOutputs = 0;
       // run processing thread
       nTypeList =nl->Second(nl->Second(nl->Second(_tt)));
       int algId;
       int typeId;
       string typeName;

       bool done = false;
       while(!done){
          if(nl->ListLength(nTypeList)<2){
             resultList.push_back(0);
             return;
          }
          if(nl->AtomType(nl->First(nTypeList))!=IntType){
               nTypeList = nl->First(nTypeList);
          } else {
             if(nl->AtomType(nl->Second(nTypeList))!=IntType){
                resultList.push_back(0);
                return;
             } else {
                algId = nl->IntValue(nl->First(nTypeList));
                typeId = nl->IntValue(nl->Second(nTypeList));
                done = true;
             }
          }
       } 

       AlgebraManager* am = SecondoSystem::GetAlgebraManager();
       inObject = am->InObj(algId,typeId);
       createObject = am->CreateObj(algId,typeId);

       runner = boost::thread(boost::bind(&PRQueryInfo::run,this));
    }

    virtual ~PRQueryInfo(){
        stop = true;
        runner.join();

        map<int,ConnectionTask<ListExpr>*>::iterator it;
        for(it = connectors.begin(); it!=connectors.end(); it++){
           it->second->stop();
        }
        for(it = connectors.begin(); it!=connectors.end(); it++){
           delete it->second;
        }
        while(!resultList.empty()){
          Tuple* t = resultList.front();
          resultList.pop_front();
          t->DeleteIfAllowed();
        }
        stream.close();
        tt->DeleteIfAllowed();
    }

    Tuple* next(){
       boost::unique_lock<boost::mutex> lock(resmtx);
       while(resultList.empty()){
          cond.wait(lock);
       }
       Tuple* res = resultList.front();
       resultList.pop_front();
       return res;
    }

    virtual void jobDone(int id, string& command, size_t no, int error, 
                       string& errMsg, ListExpr& valueList, double runtime){

       {
      
          boost::lock_guard<boost::mutex> guard(resmtx);
          bool correct;    
          Word res;
          Tuple* resTuple = new Tuple(tt);
          resTuple->PutAttribute(0, new CcInt(id));
          int errorPos = 0;
          ListExpr errorInfo = listutils::emptyErrorInfo();
          if(nl->HasLength(valueList,2)){
              res = inObject(nTypeList, nl->Second(valueList), 
                         errorPos, errorInfo, correct);
          } else {
             correct = false;
          }
      
          if(correct){ // nested list could not read in correctly
             resTuple->PutAttribute(1, (Attribute*) res.addr); 
          } else {
             Attribute* attr = (Attribute*) createObject(nTypeList).addr;
             attr->SetDefined(false);
             resTuple->PutAttribute(1,attr); 
          }
          resultList.push_back(resTuple);
          noOutputs++;
          if(noInputs==noOutputs){
           resultList.push_back(0);
          }
       }
       cond.notify_one();
    }

  private:
    Stream<CcInt> stream;
    ListExpr nTypeList;
    TupleType* tt;
    string query;
    int noInputs;
    int noOutputs;
    list<Tuple*> resultList;
    boost::mutex resmtx;
    boost::thread runner;
    bool stop;
    map<int,ConnectionTask<ListExpr>*> connectors;
    boost::mutex mapmtx;
    boost::condition_variable cond;
    InObject inObject;
    ObjectCreation createObject; 
    

    
    void run(){
       CcInt* inInt;
       inInt = stream.request();
       int count =0;
       while(inInt && !stop){
          if(inInt->IsDefined()){
             int s = inInt->GetValue();
             if(algInstance->serverExists(s)){
               count++;
               startQuery(s);
             }
          }
          inInt->DeleteIfAllowed();
          inInt = stream.request();
       }
       if(inInt){
          inInt->DeleteIfAllowed();
       }
       noInputs = count;
       if(noInputs==noOutputs){
           {  boost::unique_lock<boost::mutex> lock(resmtx);
              resultList.push_back(0);
           }
           cond.notify_one();
       }
    }

    void startQuery(int s){
       boost::lock_guard<boost::mutex> guard(mapmtx);
       if(connectors.find(s)==connectors.end()){
          connectors[s] = new ConnectionTask<ListExpr>(s,this);
       }
       connectors[s]->addCommand(s,query);
     }
};



/*
1.8.3 Value Mapping

*/
template<class T>
int prqueryVMT(Word* args, Word& result, int message,
                Word& local, Supplier s ){

   PRQueryInfo* li = (PRQueryInfo*) local.addr;
   switch(message){
     case OPEN:{
           if(li){
               delete li;
               local.addr = 0;
           }
           ListExpr tt = nl->Second(GetTupleResultType(s));
           T* query = (T*) args[1].addr;
           if(!query->IsDefined()){
              return 0;
           }
           algInstance->initProgress();
           local.addr = new PRQueryInfo(args[0], query->GetValue(), tt);
           return 0;
       }
     case REQUEST:
             result.addr = li?li->next():0;
             return result.addr?YIELD:CANCEL;
     case CLOSE:
            if(li){
               delete li;
               local.addr = 0;
               algInstance->finishProgress();
            }
            return 0;
   }
   return -1;
}

/*
1.8.4 Value Mapping and selection

*/

ValueMapping prqueryVM[] = {
  prqueryVMT<CcString>,
  prqueryVMT<FText>
};

int prquerySelect(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}

/*
1.8.5 Sepcification

*/
OperatorSpec prquerySpec(
     " stream(int) x {text, string} x int -> stream(tuple(int, DATA))",
     " _ prquery[_,_]  ",
     " Performs the same query on a set of remote servers. "
     "The first argument is a stream of integer specifying the servers. "
     "The second argument is the query to execute. The result of this "
     "query has to be in kind DATA. The third argument is a server number "
     "for determining the exact result type of the query. The result is "
     "a stream of tuples consisting of the server number and the result "
     "of this server.",
     "query intstream(0,3) prquery['query ten count', 0] sum[Result]");

/*
1.8.6

Operator instance

*/
Operator prqueryOp (
    "prquery",             //name
     prquerySpec.getStr(),         //specification
     2,
     prqueryVM,        //value mapping
     prquerySelect,   //trivial selection function
     prqueryTM        //type mapping
);


/*
1.9 Operator ~prquery2~

This operator works quite similar as the prquery operator. The difference is
that the first argument is a tuple stream with at least two integer values.
The first value corresponds to the server number (like in prquery), the 
second number is a parameter for the remote query. The remote query contains 
somewhere the keywords SERVER and PART which are replaced by the 
corresponding values. For determining the result type, some
default parameters for part and server are given.


1.9.1 Type Mapping

*/

ListExpr prquery2TM(ListExpr args){

  string err = "stream(tuple) x {string,text} x AttrName x AttrName "
               "x int x int expected";
  if(!nl->HasLength(args,6)){
    return listutils::typeError(err + " (invalid number of args)");
  }

  ListExpr stream = nl->First(args);
  if(!nl->HasLength(stream,2)){
     return listutils::typeError("internal error");
  }
  stream = nl->First(stream);
  if(!Stream<Tuple>::checkType(stream)){
    return listutils::typeError(err + " (first arg is not a tuple stream)");
  }
  ListExpr query = nl->Second(args);
  if(!nl->HasLength(query,2)){
    return listutils::typeError("internal error");
  }
  query = nl->First(query);
  if(!CcString::checkType(query) && !FText::checkType(query)){
     return listutils::typeError(err + " (second arg is not of string or "
                                 "text)");
  }
  ListExpr serverNo = nl->Third(args);
  if(!nl->HasLength(serverNo,2)){
     return listutils::typeError("internal error");
  }
  serverNo = nl->First(serverNo);
  if(nl->AtomType(serverNo)!=SymbolType){
     return listutils::typeError(err +" (third arg is not an attr name)");
  }
  ListExpr partNo = nl->Fourth(args);
  if(!nl->HasLength(partNo,2)){
     return listutils::typeError("internal error");
  }
  partNo = nl->First(partNo);
  if(nl->AtomType(partNo)!=SymbolType){
     return listutils::typeError(err +" (fourth arg is not an attr name)");
  }
  if(!CcInt::checkType(nl->First(nl->Fifth(args)))){
     return listutils::typeError(err +" (fifth arg is not an int)");
  }
  if(!CcInt::checkType(nl->First(nl->Sixth(args)))){
     return listutils::typeError(err +" (sixth arg is not an int)");
  }
  // check for presence and int type of attributes in tuple stream
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr attrType;
  string sp = nl->SymbolValue(serverNo);
  int serverPos = listutils::findAttribute(attrList, sp,  attrType);
  if(!serverPos){
     return listutils::typeError("attribute " + sp + " not found");
  }
  if(!CcInt::checkType(attrType)){
    return listutils::typeError("attribute " + sp + " not of type int");
  }
  string pp = nl->SymbolValue(partNo);
  int partPos = listutils::findAttribute(attrList, pp, attrType);
  if(!partPos){
     return listutils::typeError("attribute " + pp + " not found");
  }
  if(!CcInt::checkType(attrType)){
     return listutils::typeError("attribute " + pp + " not of type int");
  }

  // get the query as well as the default numbers
  string q;
  bool ok;
  if(CcString::checkType(nl->First(nl->Second(args)))){
    ok = getValue<CcString>(nl->Second(nl->Second(args)),q);
  } else {
    ok = getValue<FText>(nl->Second(nl->Second(args)),q);
  }
  if(!ok){
    return listutils::typeError("problem in determining query text");
  }
  int defaultPart;
  if(!getValue(nl->Second(nl->Fifth(args)),defaultPart)){
    return listutils::typeError("cannot determine default part");
  }
  int defaultServer;
  if(!getValue(nl->Second(nl->Sixth(args)),defaultServer)){
    return listutils::typeError("cannot determine default server");
  }

  // manipulate
  q = stringutils::replaceAll(q,"SERVER",stringutils::int2str(defaultServer));
  q = stringutils::replaceAll(q,"PART",stringutils::int2str(defaultPart));

  
  string errMsg;
  ListExpr resType;

  if(!getQueryType(q,defaultServer,resType, errMsg)){
    return listutils::typeError(errMsg);
  }
  if(!listutils::isDATA(resType)){
     return listutils::typeError("result type not in kind DATA");
  }
  ListExpr resAttrList = listutils::concat(attrList, 
                              nl->OneElemList( nl->TwoElemList( 
                                        nl->SymbolAtom("Result"),resType))); 
  if(!listutils::isAttrList(resAttrList)){
    return listutils::typeError("Result already part of tuple ");
  }

  return nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               nl->TwoElemList( nl->IntAtom(serverPos-1), 
                                nl->IntAtom(partPos-1)),
               nl->TwoElemList(
                     listutils::basicSymbol<Stream<Tuple> >(),
                      nl->TwoElemList(
                           listutils::basicSymbol<Tuple>(),
                           resAttrList)));
}

/*
1.9.2 LocalInfo Class

*/
class PRQuery2Info : public CommandListener<ListExpr>{
  public:
    PRQuery2Info(Word& _stream , const string& _query,  
                int _serverAttr, int _partAttr, ListExpr _tt) : 
      stream(_stream), serverAttr(_serverAttr), 
      partAttr(_partAttr), query(_query) {
    
       stream.open();
       stop = false;
       noInputs=-1;
       noOutputs = 0;
       // run processing thread
       string typeName;

       // determine result type
       ListExpr attrList = nl->Second(nl->Second(_tt));
       while(!nl->HasLength(attrList,1)){
          attrList = nl->Rest(attrList);
       } 
       resType = nl->Second(nl->First(attrList)); 

       tt = new TupleType(
              SecondoSystem::GetCatalog()->NumericType(nl->Second(_tt)));


       string tname;
       int algId, typeId;
       SecondoSystem::GetCatalog()->LookUpTypeExpr(resType, tname, 
                                                   algId, typeId);


       AlgebraManager* am = SecondoSystem::GetAlgebraManager();
       inObject = am->InObj(algId,typeId);
       createObject = am->CreateObj(algId,typeId);
       runner = boost::thread(boost::bind(&PRQuery2Info::run,this));
    }

    virtual ~PRQuery2Info(){
        stop = true;
        runner.join();

        map<int,ConnectionTask<ListExpr>*>::iterator it;
        for(it = connectors.begin(); it!=connectors.end(); it++){
           it->second->stop();
        }
        for(it = connectors.begin(); it!=connectors.end(); it++){
           delete it->second;
        }
        while(!resultList.empty()){
          Tuple* t = resultList.front();
          resultList.pop_front();
          t->DeleteIfAllowed();
        }
        stream.close();
        tt->DeleteIfAllowed();
        // delete non processed input tuples
        boost::lock_guard<boost::mutex> gurad(mapmtx);
        map<int,Tuple*>::iterator it2;
        for(it2=inTuples.begin();it2!=inTuples.end();it2++){
           it2->second->DeleteIfAllowed();
        }
    }

    Tuple* next(){
       boost::unique_lock<boost::mutex> lock(resmtx);
       while(resultList.empty()){
          cond.wait(lock);
       }
       Tuple* res = resultList.front();
       resultList.pop_front();
       return res;
    }

    virtual void jobDone(int id, string& command, size_t no, int error, 
                       string& errMsg, ListExpr& valueList, double runtime){

       {


          boost::lock_guard<boost::mutex> guard(resmtx);
          bool correct;    
          Word res;
          Tuple* resTuple = new Tuple(tt);

          Tuple* inTuple = 0;
          {
             boost::lock_guard<boost::mutex> guard(mapmtx);
             map<int,Tuple*>::iterator it = inTuples.find(id);
             if(it == inTuples.end()){
                 cerr << "Task with id " << id 
                      << "finshed, but no input tuple is found" << endl;
                 return;
             }
             inTuple = it->second;
             inTuples.erase(it);
          }
          // copy original tuples
          int noAttributes = inTuple->GetNoAttributes();
          for(int i=0;i<noAttributes;i++){
            resTuple->CopyAttribute(i,inTuple,i);
          }
          inTuple->DeleteIfAllowed();

          int errorPos = 0;
          ListExpr errorInfo = listutils::emptyErrorInfo();
          Attribute* resAttr;

          if(nl->HasLength(valueList,2)){
              res = inObject(resType, nl->Second(valueList), 
                         errorPos, errorInfo, correct);
          } else {
             correct = false;
          }
      
          if(correct){ // nested list could not read in correctly
             resAttr = (Attribute*) res.addr; 
          } else {
             Attribute* attr = (Attribute*) createObject(resType).addr;
             attr->SetDefined(false);
             resAttr = attr;; 
          }
          resTuple->PutAttribute(noAttributes,resAttr);
          resultList.push_back(resTuple);
          noOutputs++;
          if(noInputs==noOutputs){
           resultList.push_back(0);
          }
       }
       cond.notify_one();
    }

  private:
    Stream<Tuple> stream;
    int serverAttr;
    int partAttr;
    string query;
    TupleType* tt;
    int noInputs;
    int noOutputs;
    ListExpr resType;
    list<Tuple*> resultList;
    map<int, Tuple*> inTuples;
    boost::mutex resmtx;
    boost::thread runner;
    bool stop;
    map<int,ConnectionTask<ListExpr>*> connectors;
    boost::mutex mapmtx;
    boost::condition_variable cond;
    InObject inObject;
    ObjectCreation createObject; 
    

    
    void run(){
       Tuple* inTuple = stream.request();
       int count =0;
       while(inTuple && !stop){
          CcInt* serverNo = (CcInt*) inTuple->GetAttribute(serverAttr);
          CcInt* partNo = (CcInt*) inTuple->GetAttribute(partAttr);



          if(serverNo->IsDefined() && partNo->IsDefined()){
              int snum = serverNo->GetValue();
              int pnum = partNo->GetValue();
              if(algInstance->serverExists(snum)){
                  startQuery(inTuple, snum,pnum, count);
              } else {
                 inTuple->DeleteIfAllowed();
              } 
          }  
          inTuple = stream.request();
       }
       if(inTuple){
          inTuple->DeleteIfAllowed();
       }
       noInputs = count;
       if(noInputs==noOutputs){
           {  boost::unique_lock<boost::mutex> lock(resmtx);
              resultList.push_back(0);
           }
           cond.notify_one();
       }
    }

    void startQuery(Tuple* inTuple,int s, int p, int &id){
       boost::lock_guard<boost::mutex> guard(mapmtx);
       inTuples[id] = inTuple;
       if(connectors.find(s)==connectors.end()){
          connectors[s] = new ConnectionTask<ListExpr>(s,this);
       }
       string q = stringutils::replaceAll(query,"PART",stringutils::int2str(p));
       q = stringutils::replaceAll(q,"SERVER",stringutils::int2str(s));
       connectors[s]->addCommand(id,q);
       id++;
     }
};


/*
1.9.3 Value Mapping

*/

template<class T>
int prquery2VMT(Word* args, Word& result, int message,
                Word& local, Supplier s ){

   PRQuery2Info* li = (PRQuery2Info*) local.addr;

   switch(message){
     case OPEN : {
                   if(li) {
                      delete li;
                      local.addr = 0;
                   }
                   T* q = (T*) args[1].addr;
                   if(!q->IsDefined()){
                     return 0;
                   }
                   algInstance->initProgress();
                   local.addr = new PRQuery2Info(args[0], q->GetValue(),
                                 ((CcInt*) args[6].addr)->GetValue(),
                                 ((CcInt*) args[7].addr)->GetValue(),
                                 qp->GetType(s));
                   return 0;
                 }
      case REQUEST: 
                 result.addr=li?li->next():0;
                 return result.addr?YIELD:CANCEL;
      case CLOSE:
                 if(li){
                    delete li;
                    local.addr =0;
                    algInstance->finishProgress();
                 }
                 return 0;
   }
   return -1; 
}


/*
1.9.4 Value Mapping and selection

*/

ValueMapping prquery2VM[] = {
  prquery2VMT<CcString>,
  prquery2VMT<FText>
};

int prquery2Select(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}

/*
1.8.5 Sepcification

*/
OperatorSpec prquery2Spec(
     " stream(tuple(...)) x {text, string} x AttrName x AttrName x "
     "int x int -> stream(tuple)",
     " _ prquery2[_,_,_,_,_]  ",
     " Performs a slightly different  query on a set of remote servers.  "
     " The first argument is a stream of tuples containing the server numbers"
     " as well as the modification number."
     " The second argument is the query to execute. The parts SERVER and PART "
     " are replaced by the corresponding numbers of the stream. "
     " The third argument specifies the Attribute name of the server number,"
     " the fourth argument specifies the attribute name for the part component."
     " The last two arguments are integer numbers used in typemapping as "
     " default values for server and part.",
     " query serverparts feed  prquery2['query ten_SERVER_PART count', Server, "
     "PART,0.5] sum[Result]");

/*
1.9.6

Operator instance

*/
Operator prquery2Op (
    "prquery2",             //name
     prquery2Spec.getStr(),         //specification
     2,
     prquery2VM,        //value mapping
     prquerySelect,   //trivial selection function
     prquery2TM        //type mapping
);




/*
2 Implementation of the DistributedAlgebra Functionality

Here, the functionality of the Distributed algebra is implemented using the
parts from before.

2.1 Type ~darray~

A ~darray~ contains the remote type as well as a server configuration 
consiting of the name of the server, the used port, and the used 
configuration file.  When accessing a  ~darray~ element, it is checked 
whether there is already a connection to the remote server. To avoid
 conflicts between user defined connections and connections used by 
darray values, a second structure is hold within the algebra
which opens and closes connections automatically.

*/




/*
2.1.2.1 Property function

*/
ListExpr DArrayProperty(){
   return nl->TwoElemList(
            nl->FourElemList(
                 nl->StringAtom("Signature"),
                 nl->StringAtom("Example Type List"), 
                 nl->StringAtom("List Rep"),
                 nl->StringAtom("Example List")),
            nl->FourElemList(
                 nl->StringAtom(" -> SIMPLE"),
                 nl->StringAtom(" (darray <basictype>)"),
                 nl->TextAtom("(name <map> ( s1 s2  ...)) where "
                                "s_i =(server port config)"),
                 nl->TextAtom(" ( mydarray (0 1 0 1) ('localhost' 1234 "
                              "'config.ini')"
                              " ('localhost'  1235 'config.ini'))")));
}

ListExpr DFArrayProperty(){
   return nl->TwoElemList(
            nl->FourElemList(
                 nl->StringAtom("Signature"),
                 nl->StringAtom("Example Type List"), 
                 nl->StringAtom("List Rep"),
                 nl->StringAtom("Example List")),
            nl->FourElemList(
                 nl->StringAtom(" -> SIMPLE"),
                 nl->StringAtom(" (dfarray <basictype>)"),
                 nl->TextAtom("(name <map> ( s1 s2  ...)) where "
                                "s_i =(server port config)"),
                 nl->TextAtom(" ( mydfarray (0 1 0 1 1) ('localhost' 1234 "
                              "'config.ini')"
                              " ('localhost'  1235 'config.ini'))")));
}


ListExpr DFMatrixProperty(){
   return nl->TwoElemList(
            nl->FourElemList(
                 nl->StringAtom("Signature"),
                 nl->StringAtom("Example Type List"), 
                 nl->StringAtom("List Rep"),
                 nl->StringAtom("Example List")),
            nl->FourElemList(
                 nl->StringAtom(" -> SIMPLE"),
                 nl->StringAtom(" (dfmatrix <basictype>)"),
                 nl->TextAtom("(name size ( s1 s2  ...)) where "
                                "s_i =(server port config)"),
                 nl->TextAtom(" ( mymatrix 42  ('localhost' 1234 'config.ini')"
                              " ('localhost'  1235 'config.ini'))")));
}
/*
2.1.2.2 IN function

*/

template<class A>
Word InDArray(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct){

   Word res((void*)0);
   res.addr = A::readFrom(instance);
   correct = res.addr!=0;
   return res;
}

/*

2.1.2.3 Out function

*/
template<class A>
ListExpr OutDArray(ListExpr typeInfo, Word value){
   A* da = (A*) value.addr;
   return da->toListExpr();
}

/*

2.1.2.4 Create function

*/
template <class A>
Word CreateDArray(const ListExpr typeInfo){

  Word w;
  vector<uint32_t> m;
  w.addr = new A(m,"");
  return w;
}

Word CreateDFMatrix(const ListExpr typeInfo){
  Word w;
  w.addr = new DFMatrix(0,"");
  return w;
}

/*

2.1.2.4 Delete function

*/
template <class A>
void DeleteDArray(const ListExpr typeInfo, Word& w){
  A* a = (A*) w.addr;
  delete a;
  w.addr = 0;
}


/*

2.1.2.4 Open function

*/

template<class A>
void CloseDArray(const ListExpr typeInfo, Word& w){
  A* a = (A*) w.addr;
  delete a;
  w.addr = 0;
}

template<class A>
Word CloneDArray(const ListExpr typeInfo, const Word& w){
    A* a = (A*) w.addr;
    Word res;
    res.addr = new A(*a);
    return res;
}

template<class A>
void* CastDArray(void* addr){
   return (new (addr) A(0));   
}

template<class A>
bool DistTypeBaseCheck(ListExpr type, ListExpr& errorInfo){
    return A::checkType(type);
}


int SizeOfDArray(){
  return 42; // a magic number
}


TypeConstructor DArrayTC(
  DArray::BasicType(),
  DArrayProperty,
  OutDArray<DArray>, InDArray<DArray>,
  0,0,
  CreateDArray<DArray>, DeleteDArray<DArray>,
  DArray::open<DArray>, DArray::save,
  CloseDArray<DArray>, CloneDArray<DArray>,
  CastDArray<DArray>,
  SizeOfDArray,
  DistTypeBaseCheck<DArray> );


TypeConstructor DFArrayTC(
  DFArray::BasicType(),
  DFArrayProperty,
  OutDArray<DFArray>, InDArray<DFArray>,
  0,0,
  CreateDArray<DFArray>, DeleteDArray<DFArray>,
  DFArray::open<DFArray>, DFArray::save,
  CloseDArray<DFArray>, CloneDArray<DFArray>,
  CastDArray<DFArray>,
  SizeOfDArray,
  DistTypeBaseCheck<DFArray> );



TypeConstructor DFMatrixTC(
  DFMatrix::BasicType(),
  DFMatrixProperty,
  OutDArray<DFMatrix>, InDArray<DFMatrix>,
  0,0,
  CreateDFMatrix, DeleteDArray<DFMatrix>,
  DFMatrix::open, DFMatrix::save,
  CloseDArray<DFMatrix>, CloneDArray<DFMatrix>,
  CastDArray<DFMatrix>,
  SizeOfDArray,
  DistTypeBaseCheck<DFMatrix> );


TypeConstructor FRelTC(
  frel::BasicType(),
  frel::Property,
  frel::Out, frel::In,
  0,0,
  frel::Create, frel::Delete,
  frel::Open, frel::Save,
  frel::Close, frel::Clone, 
  frel::Cast, 
  frel::SizeOf,
  frel::TypeCheck );

TypeConstructor FSRelTC(
  fsrel::BasicType(),
  fsrel::Property,
  fsrel::Out, fsrel::In,
  0,0,
  fsrel::Create, fsrel::Delete,
  fsrel::Open, fsrel::Save,
  fsrel::Close, fsrel::Clone, 
  fsrel::Cast, 
  fsrel::SizeOf,
  fsrel::TypeCheck );


TypeConstructor FObjTC(
  fobj::BasicType(),
  fobj::Property,
  fobj::Out, fobj::In,
  0,0,
  fobj::Create, fobj::Delete,
  fobj::Open, fobj::Save,
  fobj::Close, fobj::Clone, 
  fobj::Cast, 
  fobj::SizeOf,
  fobj::TypeCheck );

/*
3. DArray Operators

3.1 Operator ~put~

This operator puts a new value into a DArray. It returns a clone of
the argument array. If there is a problem during storing the value,
the resulting array will be undefined. 

3.1.1 Type Mapping

the signature is darray(t) x int x t -> darray(t)

*/
ListExpr putTM(ListExpr args){
   string err = "{darray(t), dfarray(t)}  x int x t expected";

   if(!nl->HasLength(args,3)){
     return listutils::typeError(err + " (invalid number of args)");
   } 
   if( !DArray::checkType(nl->First(args))&&
       !DFArray::checkType(nl->First(args))){
     return listutils::typeError(err + " ( first arg is not a "
                                       "darray or dfarray)");
   }
   if( !CcInt::checkType(nl->Second(args))){
     return listutils::typeError(err + " ( second arg is not an int)");
   }


   ListExpr subtype = nl->Second(nl->First(args));
   if(!nl->Equal(subtype, nl->Third(args))){
    return listutils::typeError("type conflict between darray "
                                 "and argument type");
   }
   return nl->First(args);
}

/*
3.1.2 ~put~ Value Mapping

Note: the current implementation uses just nested list 
even for relations, here, a special threatment for big objects
should be implemented for transferring big objects to the
remote server;

*/

int putVMA(Word* args, Word& result, int message,
                Word& local, Supplier s ){


  DArray* array = (DArray*) args[0].addr;
  CcInt* index = (CcInt*) args[1].addr;
  result = qp->ResultStorage(s);
  DArray* res = (DArray*) result.addr;
  if(!array->IsDefined() || !index->IsDefined()){
     res->makeUndefined();
     return 0;
  } 

  int i = index->GetValue();

  if(i < 0 || i >= (int) array->getSize()){
     res->makeUndefined();
     return 0;
  }
  if(array->numOfWorkers() < 1){
       res->makeUndefined();
  }


  // retrieve the current database name on the master

  string dbname = SecondoSystem::GetInstance()->GetDatabaseName();

  
  DArrayElement elem = array->getWorkerForSlot(i);

  ConnectionInfo* ci = algInstance->getWorkerConnection(elem, dbname);

  if(!ci){ // problem with connection the monitor or opening the database
     res->makeUndefined();
     return 0;
  }

  if(! ci->createOrUpdateObject(  array->getName() + "_" 
                                + stringutils::int2str(i),
                                nl->Second(qp->GetType(s)),
                                args[2])){
      res->makeUndefined();
      return 0;
  }
  (*res) = *array;
  return 0;                           
}


int putVMFA(Word* args, Word& result, int message,
                Word& local, Supplier s ){


  result = qp->ResultStorage(s);
  DFArray* res = (DFArray*) result.addr; 
  DFArray* array = (DFArray*) args[0].addr;
  (*res) = (*array);
  if(!array->IsDefined()){ // never put something intop an undefined array
    return 0;
  }
  CcInt* ccIndex = (CcInt*) args[1].addr;
  if(!ccIndex->IsDefined()){
     return 0;
  }
  int index = ccIndex->GetValue();
  if(index<0 || (size_t) index >= array->getSize()){
     return 0;
  }
  // step 1 write relation to local file
  Relation* rel = (Relation*) args[2].addr;
  string fname = array->getName() + "_" + stringutils::int2str(index)+".bin";
  ListExpr relType = nl->Second(qp->GetType(s));
  if(!BinRelWriter::writeRelationToFile(rel,relType, fname)){
     cerr << "error in writing relation" << endl;
     res->makeUndefined();
     return 0;
  }
  // get ConnectionInfo
  string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
  ConnectionInfo* ci = algInstance->getWorkerConnection( 
                      array->getWorkerForSlot(index),dbname);
  if(!ci){
     cerr << "could not get connection to server" << endl;
     res->makeUndefined();
     return 0;
  }
  // send file
  if(ci->sendFile(fname,fname,true)!=0){
     cerr << "error in sending file" << fname << endl;
     res->makeUndefined();
     return 0;
  }

  FileSystem::DeleteFileOrFolder(fname);
  // move File to target position

  string targetDir = ci->getSecondoHome()+"/dfarrays/"+dbname
                     + "/"+array->getName()+"/";
  string cmd = "query createDirectory('"+targetDir+"', TRUE)";
  int err;
  string errMsg;
  ListExpr resList;
  double runtime;
  ci->simpleCommand(cmd,err,errMsg,resList,false, runtime);
  if(err){
    cerr << "creating targetdirectory " << targetDir << " failed" << endl;
    cerr << "cmd : " << cmd << endl;
    cerr << "error " << errMsg << endl;
    res->makeUndefined();
    return 0;
  }  
  string f1 = ci->getSendPath()+"/"+fname;
  string f2 = targetDir + fname;
  cmd = "query moveFile('"+f1+"', '"+ f2 +"')";

  ci->simpleCommand(cmd,err,errMsg,resList,false, runtime);
  if(err!=0){
     cerr << "error in command " << cmd << endl;
     cerr << " error code : " << err << endl;
     cerr << "error Messahe " << errMsg << endl;
     res->makeUndefined();
     return 0;
  }
  if(    !nl->HasLength(resList,2) 
      || (nl->AtomType(nl->Second(resList))!=BoolType)){
    cerr << "command " << cmd << " returns unexpected result" << endl;
    cerr << nl->ToString(resList) << endl;
    res->makeUndefined();
    return 0;
  }

  if(!nl->BoolValue(nl->Second(resList))){
     cerr << "move file failed" << endl;
     res->makeUndefined();
  }
  return 0;
}


/*
3.1.3 Specification

*/

OperatorSpec putSpec(
     " d[f]array(T) x int x T -> d[f]array(T) ",
     " put(_,_,_)",
     "Puts an element at a specific position of a d[f]array. "
     "If there is some problem, the result is undefined; otherwise "
     "the result represents the same array as before with "
     "a changed element.",
     "query put([const darray(int) value"
     "           (da1 4 ((\"host1\" 1234 \"Config1.ini\")"
     "           (\"host2\" 1234 \"Config2.ini\")))] , 1, 27)"
     );


ValueMapping putVM[] ={ putVMA, putVMFA };

int putSelect(ListExpr args){
  return DArray::checkType(nl->First(args))?0:1;
}



/*
3.1.4 Operator Instance

*/

Operator putOp(
           "put",
           putSpec.getStr(),
           2,
           putVM,
           putSelect,
           putTM);


/*
3.2 Operator get

This operator retrieves an remove object from a server

3.2.1 Type Mapping

Signature is darray(T) x int -> T

*/
ListExpr getTM(ListExpr args){

  string err = "{darray(T), dfarray(T)} x int expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err + " (invalid number of args)");
  }
  if(    !DArray::checkType(nl->First(args)) 
      && !DFArray::checkType(nl->First(args))){
     return listutils::typeError(err+
                          " ( first is not a darray or a dfarray)");
  }
  if(!CcInt::checkType(nl->Second(args))){
     return listutils::typeError(err+"(second arg is not of type int)");
  }
  return nl->Second(nl->First(args));
}

/*
3.2.2 Value Mapping

*/
int getVMDA(Word* args, Word& result, int message,
          Word& local, Supplier s ){

  DArray* array = (DArray*) args[0].addr;
  CcInt* index = (CcInt*) args[1].addr;
  result = qp->ResultStorage(s);
  bool isData = listutils::isDATA(qp->GetType(s));
  if(!array->IsDefined() || !index->IsDefined()){
     cerr << " undefined argument" << endl;
     if(isData){
        ((Attribute*) result.addr)->SetDefined(false);
     } else {
        cerr << "Undefined element found" << endl;
     }
     return 0;
  } 

  int i = index->GetValue();
  if(i<0 || i>=(int)array->getSize()){
     if(isData){
        ((Attribute*) result.addr)->SetDefined(false);
     } else {
        cerr << "Undefined element found" << endl;
     }
     return 0;
  }
  if(array->numOfWorkers() <1){
     if(isData){
        ((Attribute*) result.addr)->SetDefined(false);
     } else {
        cerr << "Undefined element found" << endl;
     }
     return 0;
  }


  DArrayElement elem = array->getWorkerForSlot(i);
  string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
  ConnectionInfo* ci = algInstance->getWorkerConnection(elem, dbname);
  if(!ci){ // problem with connection the monitor or opening the databaseA
     if(isData){
        ((Attribute*) result.addr)->SetDefined(false);
     } else {
        cerr << "Undefined element found" << endl;
     }
     return 0;
  }

  Word r;
  ListExpr resType = qp->GetType(s);
  bool ok =  ci->retrieve(array->getName() + "_"
                                + stringutils::int2str(i),
                                resType, r, true);

  if(!ok){ // in case of an error, res.addr point to 0A
     if(isData){
        ((Attribute*) result.addr)->SetDefined(false);
     } else {
        cerr << "Undefined element found" << endl;
     }
     return 0;
  }
  if(!nl->Equal(resType, qp->GetType(s))){
     // result type  differs from expected type
     cerr << "Types in darray differs from type in remote db" << endl;
     string typeName; 
     int algebraId;
     int typeId;
     SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
     AlgebraManager* am = SecondoSystem::GetAlgebraManager();
     if( ctlg->LookUpTypeExpr( resType, typeName, algebraId, typeId ) ){
         am->DeleteObj(algebraId, typeId)(resType,r);    

     } else {
        cerr << "internal error leads to memory leak" << endl;
     }
     return 0;
  }


  // in case of no error, r.addr points to the result
  qp->DeleteResultStorage(s);
  qp->ChangeResultStorage(s,r);
  result = qp->ResultStorage(s);
  return 0;
}


int getVMDFA(Word* args, Word& result, int message,
             Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  DFArray* array = (DFArray*) args[0].addr;
  CcInt* ccpos = (CcInt*) args[1].addr;

  if(!array->IsDefined() || !ccpos->IsDefined()){
    return 0;
  }
  int pos = ccpos->GetValue();
  if(pos < 0 || (size_t) pos >= array->getSize()){
     return 0;
  }
  DArrayElement elem = array->getWorkerForSlot(pos);
  string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
  ConnectionInfo* ci = algInstance->getWorkerConnection(elem, dbname);
  if(!ci){
     return 0;
  }
  ListExpr resType = qp->GetType(s);;
  string fileName = ci->getSecondoHome() + "/dfarrays/"+dbname+"/"
                    + array->getName() + "/"
                    + array->getName()+"_"+stringutils::int2str(pos)+".bin";  
  Word relResult;
  ci->retrieveRelationInFile(fileName, resType, relResult);
  if(relResult.addr == 0){
     return 0;
  }
  if(!nl->Equal(resType, qp->GetType(s))){
    cerr << "Type of remote object does not fit the sub type of dfarray." 
         << endl;
    string typeName; 
    int algebraId;
    int typeId;
    SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
    AlgebraManager* am = SecondoSystem::GetAlgebraManager();
    if( ctlg->LookUpTypeExpr( resType, typeName, algebraId, typeId ) ){
        am->DeleteObj(algebraId, typeId)(resType,relResult);    
    } else {
       cerr << "internal error leads to memory leak" << endl;
    }
    return 0;
  }
  qp->DeleteResultStorage(s);
  qp->ChangeResultStorage(s,relResult);
  result = qp->ResultStorage(s);
  return 0;
}



OperatorSpec getSpec(
     " {darray(T), dfarray(T)}  x int -> T ",
     " get(_,_)",
     " Retrieves an element at a specific position "
     "of a darray or a dfarray.",
     " query get([const darray(int) value"
     "           (da1 4 ((\"host1\" 1234 \"Config1.ini\")"
     "           (\"host2\" 1234 \"Config2.ini\")))] , 1, )"
     );

ValueMapping getVM[] = {
  getVMDA,
  getVMDFA
};

int getSelect(ListExpr args){
 return DArray::checkType(nl->First(args))?0:1;
}



Operator getOp(
           "get",
           getSpec.getStr(),
           2,
           getVM,
           getSelect,
           getTM);


/*
3.4.4 Operator size

This operator just asks for the size of a darray instance.

*/
ListExpr sizeTM(ListExpr args){
  string err  = "darray  or dfarrayexpected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  if(!DArray::checkType(nl->First(args))
    && !DFArray::checkType(nl->First(args))
    && !DFMatrix::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcInt>();
}

template<class A>
int sizeVMT(Word* args, Word& result, int message,
          Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;

  A* arg  = (A*) args[0].addr;
  if(!arg->IsDefined()){
    res->SetDefined(false);
  } else {
    res->Set(true,arg->getSize());
  }
  return 0;
}

OperatorSpec sizeSpec(
     " d[f]array(T) -> int",
     " size(_)",
     " Returns the number of slots of a d[f]array.",
     " query size([const darray(int) value"
     "           (da1 4 ((\"host1\" 1234 \"Config1.ini\")"
     "           (\"host2\" 1234 \"Config2.ini\")))] )"
     );

int sizeSelect(ListExpr args){
  ListExpr a = nl->First(args);
  if(DArray::checkType(a)) return 0;
  if(DFArray::checkType(a)) return 1;
  if(DFMatrix::checkType(a)) return 2;
  return -1;
}

ValueMapping sizeVM[] = {
  sizeVMT<DArray>,
  sizeVMT<DFArray>,
  sizeVMT<DFMatrix>
};


Operator sizeOp(
           "size",
           sizeSpec.getStr(),
           3,
           sizeVM,
           sizeSelect,
           sizeTM);

/*
Operator getWorkers

*/
ListExpr getWorkersTM(ListExpr args){
  string err = "darray  or dfarray  expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  if(!DArray::checkType(nl->First(args))
     && !DFArray::checkType(nl->First(args))){
    return listutils::typeError(err);
  }

  ListExpr attrList = nl->FourElemList(
        nl->TwoElemList(
           nl->SymbolAtom("Host"),
           listutils::basicSymbol<FText>()),
        nl->TwoElemList(
           nl->SymbolAtom("Port"),
           listutils::basicSymbol<CcInt>()),
        nl->TwoElemList(
           nl->SymbolAtom("Config"),
           listutils::basicSymbol<FText>()),
        nl->TwoElemList(
           nl->SymbolAtom("No"),
           listutils::basicSymbol<CcInt>())
      );
  return nl->TwoElemList(
      listutils::basicSymbol<Stream<Tuple> >(),
      nl->TwoElemList(
         listutils::basicSymbol<Tuple>(),
         attrList));
}

template<class A>
class getWorkersInfo{
  public:
     getWorkersInfo(A* _array, ListExpr _tt):
        array(_array), pos(0){
         tt = new TupleType(_tt);
     }

     ~getWorkersInfo(){
         tt->DeleteIfAllowed();
      }

     Tuple* next(){
         if(pos >= array->numOfWorkers()){
            return 0;
         }
         DArrayElement e = array->getWorker(pos);
         pos++;
         Tuple* tuple = new Tuple(tt);
         tuple->PutAttribute(0, new FText(true,e.getHost()));
         tuple->PutAttribute(1, new CcInt(e.getPort()));
         tuple->PutAttribute(2, new FText(true,e.getConfig()));
         tuple->PutAttribute(3, new CcInt(e.getNum()));
         return tuple;
     }

  private:
     A* array;
     size_t pos;
     TupleType* tt;

};

template<class A>
int getWorkersVMT(Word* args, Word& result, int message,
          Word& local, Supplier s ){

   getWorkersInfo<A>* li = (getWorkersInfo<A>*) local.addr;
   switch(message){
      case OPEN:{
            if(li){
               delete li;
               local.addr = 0;
            }
            A* arg = (A*) args[0].addr;
            if(arg->IsDefined()){
              local.addr = new getWorkersInfo<A>(arg, 
                              nl->Second(GetTupleResultType(s)));
            }
            return 0;
         }   
       case REQUEST:
              result.addr = li?li->next():0;
              return result.addr?YIELD:CANCEL;
       case CLOSE:
              if(li){
                 delete li;
                 local.addr = 0;
              } 
              return 0;
   }
   return -1;
}


OperatorSpec getWorkersSpec(
     " d[f]array(T) -> stream(tuple(...)) ",
     " getWorkers(_)",
     " Returns information about workers in a d[f]array.",
     " query getWorkers([const darray(int) value"
     "           (da1 4 ((\"host1\" 1234 \"Config1.ini\")"
     "           (\"host2\" 1234 \"Config2.ini\")))] ) count"
     );

ValueMapping getWorkersVM[] = {
  getWorkersVMT<DArray>,
  getWorkersVMT<DFArray>
};

int getWorkersSelect(ListExpr args){
  return DArray::checkType(nl->First(args))?0:1;

}


Operator getWorkersOp(
           "getWorkers",
           getWorkersSpec.getStr(),
           2,
           getWorkersVM,
           getWorkersSelect,
           getWorkersTM);


/*
1.4 Operator ~fconsume5~

This operator stores a tuple stream into a single binary file.
The incoming tuple stream is also the output stream.

1.4.1 Type Mapping

*/
ListExpr fconsume5TM(ListExpr args){
  string err = "stream(TUPLE) x {string.text} [x bool]  expected";

  if(!nl->HasLength(args,2) && !nl->HasLength(args,3)){
    return listutils::typeError(err);
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  ListExpr fn = nl->Second(args);
  if(!CcString::checkType(fn) && !FText::checkType(fn)){
    return listutils::typeError(err);
  }
  if(nl->HasLength(args,2)){
     return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           nl->OneElemList(nl->BoolAtom(true)),
                           nl->First(args));
  }
  if(!CcBool::checkType(nl->Third(args))){
    return listutils::typeError(err + "( third arg is not a bool)");
  }
  return nl->First(args);
}

/*
1.4.2 Local Info class

*/
class fconsume5Info{
  public:

    fconsume5Info(Word& _stream,
                 const string& filename, 
                 const ListExpr typeList):
       in(_stream){
       out.open(filename.c_str(),ios::out|ios::binary);
       ok = out.good();
       buffer = new char[FILE_BUFFER_SIZE];
       out.rdbuf()->pubsetbuf(buffer, FILE_BUFFER_SIZE);
       if(ok){
         BinRelWriter::writeHeader(out,typeList);
       } 
       ok = out.good();
       in.open();
       firstError = true;
    }

    fconsume5Info(Word& _stream,
                 const ListExpr typeList):
       in(_stream){
       ok = false;
       firstError = true;
    }

    ~fconsume5Info(){
        if(out.is_open()){
           BinRelWriter::finish(out);
           out.close();
        }
        in.close();
        delete[] buffer;
     }

     Tuple* next(){
       Tuple* tuple = in.request();
       if(!tuple){
         return 0;
       }
       if(ok){
         if(!BinRelWriter::writeNextTuple(out,tuple)){
            ok = false;
            if(firstError){
              cerr << "Problem in writing tuple" << endl;
              firstError = false;
            }
         }
       }
       return tuple;
     }
     
  private:
     Stream<Tuple> in;
     ofstream out;
     char* buffer;
     bool ok;
     bool firstError;
};

/*
1.4.3 Value Mapping Template

*/

template<class T>
int fconsume5VMT(Word* args, Word& result, int message,
          Word& local, Supplier s ){

    fconsume5Info* li = (fconsume5Info*) local.addr;

    switch(message){
      case OPEN : {
         ListExpr type = qp->GetType(s);
         T* fn = (T*) args[1].addr;
         if(li){
            delete li;
            local.addr = 0;
         }
         if(!fn->IsDefined()){
             local.addr = new fconsume5Info(args[0],type);
             return 0;
         }
         string fns = fn->GetValue();
         type = nl->TwoElemList( listutils::basicSymbol<Relation>(),
                                 nl->Second(type));

         CcBool* overwrite = (CcBool*) args[2].addr;
         if(overwrite->IsDefined() && overwrite->GetValue()){
           string folder = FileSystem::GetParentFolder(fns);
           if(!folder.empty()){
              FileSystem::CreateFolderEx(folder);
           }
         }
         local.addr = new fconsume5Info(args[0],fns,type);
         return 0;
      }
      case REQUEST:
             result.addr = li?li->next():0;
             return result.addr?YIELD:CANCEL;
      case CLOSE:{
            if(li) {
               delete li;
               local.addr = 0;
            }
            return 0;
      }   
    }
    return -1;

}

/*
1.4.4 Value Mapping Array and Selection

*/
int fconsume5Select(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}

ValueMapping fconsume5VM[] = {
  fconsume5VMT<CcString>,
  fconsume5VMT<FText>
};


/*
1.4.5 Specification

*/

OperatorSpec fconsume5Spec(
     " stream(TUPLE) x {string, text} -> stream(TUPLE) ",
     " _ fconsume5[_]",
     " Stores a tuple stream into a binary file. ",
     " query ten feed fconsume5['ten.bin'] count"
     );

/*
1.4.6 Instance

*/
Operator fconsume5Op(
   "fconsume5",
   fconsume5Spec.getStr(),
   2,
   fconsume5VM,
   fconsume5Select,
   fconsume5TM
);


/*
1.5 Operator ~ffeed5~

This operator produces a stream of tuples from a file.

*/
ListExpr ffeed5TM(ListExpr args){
  string err = "string, text, frel, or fsrel expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  ListExpr argwt = nl->First(args);
  if(!nl->HasLength(argwt,2)){
     return listutils::typeError("internal error");
  }
  ListExpr arg = nl->First(argwt);

  // accept frel
  if(frel::checkType(arg)){
    return nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                            nl->Second(arg));
  }
  // accept fsrel
  if(fsrel::checkType(arg)){
    return nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                            nl->Second(arg));
  }

  // accept string or text, retrieve type from file
  if(!CcString::checkType(arg) && !FText::checkType(arg)){
    return listutils::typeError(err);
  }

  string filename;
  bool ok;
  ListExpr query = nl->Second(argwt);

  if(CcString::checkType(arg)){
    ok = getValue<CcString>(query, filename);
  } else {
    ok = getValue<FText>(query, filename);
  }

  if(!ok){
    return listutils::typeError("could not extract filename from query");
  }

  ifstream in(filename.c_str(), ios::in | ios::binary);
  if(!in.good()){
     return listutils::typeError("could not open file " + filename);
  }
  char marker[4];
  in.read(marker,4);
  if(!in.good()){
     in.close();
     return listutils::typeError("problem in reading from file " 
                                 + filename);
  }
  string m(marker,4);

  if(m!="srel"){
    in.close();
    return listutils::typeError("not a binary relation file");
  }
  uint32_t typeLength;
  in.read((char*) &typeLength, sizeof(uint32_t));
  char* buffer = new char[typeLength];
  in.read(buffer,typeLength);
  if(!in.good()){
    in.close();
    return listutils::typeError("problem in reading from file");
  }  
  string typeS(buffer, typeLength);
  delete [] buffer;
  ListExpr relType;
  {
     boost::lock_guard<boost::mutex> guard(nlparsemtx);
     if(!nl->ReadFromString(typeS, relType)){
         in.close();
         return listutils::typeError("problem in determining rel type");
      } 
  }

  if(!Relation::checkType(relType)){
     in.close();
     return listutils::typeError("not a valid relation type " 
                    + nl->ToString(relType) );
  }


  in.close();
  return nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                          nl->Second(relType));
}


/*
1.5.3 LocalInfo

is defined before

*/


template<class T>
int ffeed5VMT(Word* args, Word& result, int message,
          Word& local, Supplier s ){

   ffeed5Info* li = (ffeed5Info*) local.addr;
   switch(message){
      case OPEN: {
          if(li){
             delete li;
             local.addr = 0;
          }
          T* fn = (T*) args[0].addr;
          if(!fn->IsDefined()){
             return 0;
          }
          local.addr = new ffeed5Info( fn->GetValue(),
                                       nl->Second(GetTupleResultType(s)));
          return 0;
      }
      case REQUEST:{
           result.addr = li?li->next():0;
           return result.addr?YIELD:CANCEL;
      }
      case CLOSE: {
           if(li){
              delete li;
              local.addr = 0;
           }
           return 0;
      }
   }
   return -1;
}


class ffeed5fsrelInfo{

  public:
      ffeed5fsrelInfo(fsrel* _fn, ListExpr _tt):
        fn(_fn), tt(new TupleType(_tt)), pos(0), 
        li(0) {}

      Tuple* next(){
         while(pos <= fn->size()){
            if(!li){
              retrieveNextLi();
            }
            if(!li){
               return 0;
            }
            Tuple* res = li->next();
            if(res){
               return res;
            } 
            delete li;
            li = 0; 
         }
         return 0;
      }

      ~ffeed5fsrelInfo(){
           tt->DeleteIfAllowed();
           if(li){
              delete li;
              li = 0;
           }
       }


  private:
     fsrel* fn;
     TupleType* tt;
     size_t pos;
     ffeed5Info* li;

     void retrieveNextLi(){
         assert(!li);
         while(!li){
           if(pos >= fn->size()){
             return;
           }
           string f = (*fn)[pos];
           pos++;
           li = new ffeed5Info( f, tt);
           if(!li->isOK()){
              delete li;
              li = 0;
           }
         }
     }


};


int ffeed5VMfsrel(Word* args, Word& result, int message,
          Word& local, Supplier s ){

  ffeed5fsrelInfo* li = (ffeed5fsrelInfo*) local.addr;
  switch(message){
     case OPEN : {
         if(li){
           delete li;
         }
         local.addr = new ffeed5fsrelInfo( (fsrel*) args[0].addr,
                                         nl->Second(GetTupleResultType(s)));
         return 0;
     }  
     case REQUEST: {
          result.addr = li?li->next():0;
          return result.addr?YIELD:CANCEL;
     }
     case CLOSE: {
          if(li){
             delete li;
             local.addr = 0;
          }
          return 0;
     }
  }
  return -1;
}

/*
1.4.4 Value Mapping Array and Selection

*/
int ffeed5Select(ListExpr args){
  ListExpr arg = nl->First(args);
  if(CcString::checkType(arg)) { return 0;}
  if(FText::checkType(arg)){ return 1; }
  if(frel::checkType(arg)) { return 2; }
  if(fsrel::checkType(arg)) { return 3; }
  return -1;
}

ValueMapping ffeed5VM[] = {
  ffeed5VMT<CcString>,
  ffeed5VMT<FText>,
  ffeed5VMT<frel>,
  ffeed5VMfsrel
};


/*
1.4.5 Specification

*/

OperatorSpec ffeed5Spec(
  " {string, text} -> stream(TUPLE)  | f[s]rel(tuple(X)) -> stream(tuple(X))",
  " _ ffeed5",
  " Restores  a tuple stream from a binary file. ",
  " query 'ten.bin' ffeed5 count "
 );

OperatorSpec feedSpec(
   " {string, text} -> stream(TUPLE)  | f[s]rel(tuple(X)) -> stream(tuple(X))",
   " _ feed",
   " Restores  a tuple stream from a binary file. ",
   " query 'ten.bin' feed count "
);

/*
1.4.6 Instance

*/
Operator ffeed5Op(
   "ffeed5",
   ffeed5Spec.getStr(),
   4,
   ffeed5VM,
   ffeed5Select,
   ffeed5TM
);

Operator feedOp(
   "feed",
   feedSpec.getStr(),
   4,
   ffeed5VM,
   ffeed5Select,
   ffeed5TM
);

/*
1.5 Operator create darray

This operator creates a darray from a stream specifying the workers. 
As well as a template type.

1.5.1 Type Mapping 

*/ 
ListExpr createDArrayTM(ListExpr args){
   string err = "stream(tuple) x string x int x any [x bool] expected";
   if(!nl->HasLength(args,4) && !nl->HasLength(args,5)){
      return listutils::typeError(err);
   }
   ListExpr stream = nl->First(args);
   if(!Stream<Tuple>::checkType(stream)){
     return listutils::typeError("First argument must be a tuple stream");
   }

   if(!CcString::checkType(nl->Second(args))){
     return listutils::typeError("Second argument must be a string");
   }
   if(!CcInt::checkType(nl->Third(args))){
     return listutils::typeError("Third argument must be an int");
   }
   ListExpr attrList = nl->Second(nl->Second(stream));


   // check for attributes Host, Port, and Config in stream
   string hn = "Host";
   ListExpr ht;
   int hp;
   hp = listutils::findAttribute(attrList, hn, ht);
   if(!hp){
      return listutils::typeError("Attribute Host not found");
   }
   // fo the same for the port
   ListExpr pt;
   int pp; 
   string pn = "Port";
   pp = listutils::findAttribute(attrList, pn, pt);
   if(!pp){
      return listutils::typeError("Attribute " + pn + " not found");
   }
   // and for the configuration
   ListExpr ct;
   int cp; 
   string cn = "Config";
   cp = listutils::findAttribute(attrList, cn, ct);
   if(!cp){
      return listutils::typeError("Attribute " + cn + " not found");
   }
    
   // check correct types of these attributes
   if(!CcInt::checkType(pt)){
      return listutils::typeError(  "port attribute " + pn 
                                  + " not of type int");
   }

   if(!CcString::checkType(ht) && !FText::checkType(ht)){
      return listutils::typeError( "host attribute " + hn 
                               + " must be of type text or string");

   }
   if(!CcString::checkType(ct) && !FText::checkType(ct)){
      return listutils::typeError( "Config attribute " + cn 
                               + " must be of type text or string");
   }

   ListExpr resType = nl->TwoElemList(
                           listutils::basicSymbol<DArray>(),
                           nl->Fourth(args));

   if(!DArray::checkType(resType)){
      return listutils::typeError("the fourth element does not "
                                  "describe a valid type");
   } 

   if(nl->HasLength(args,4)){
      return nl->ThreeElemList(
                 nl->SymbolAtom(Symbols::APPEND()),
                 nl->FourElemList( nl->BoolAtom(false),
                                   nl->IntAtom(hp-1),
                                   nl->IntAtom(pp-1),
                                   nl->IntAtom(cp-1)),
                 resType);
   }
   if(!CcBool::checkType(nl->Fifth(args))){
      return listutils::typeError("fifth arg not of typoe bool");
   }
   return nl->ThreeElemList(
                 nl->SymbolAtom(Symbols::APPEND()),
                 nl->ThreeElemList( nl->IntAtom(hp-1),
                                    nl->IntAtom(pp-1),
                                    nl->IntAtom(cp-1)),
                 resType);
}

/*
1.5.3 Value Mapping Template

Groups elements of a darray-type to those having the same IP and
the same secondo home directory. Each group will be element of the 
result.

*/
template<class T>
vector<vector< pair<DArrayElement, size_t> > > group(T& array){

   typedef map<pair<string,string>, 
               vector< pair<DArrayElement, size_t> > > groupt;

   groupt groups;

   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   for(size_t i=0;i<array->numOfWorkers();i++){
     DArrayElement elem = array->getWorker(i);
     ConnectionInfo* ci = algInstance->getWorkerConnection(elem, dbname);
     if(!ci){
       throw new SecondoException("worker cannot be reached");
     }
     string home = ci->getSecondoHome();
     pair<string,string> p(ci->getHost(), home);
     if(groups.find(p)==groups.end()){
       vector< pair<DArrayElement, size_t> > v;
       v.push_back(make_pair(elem, i));
       groups[p] = v;
     } else{
       groups[p].push_back(make_pair(elem,i));
     }
   }
   groupt::iterator it;
   vector<vector<pair<DArrayElement, size_t> > > res;
   for(it = groups.begin();it!=groups.end();it++){
      res.push_back(it->second);
   }
   return res;
}

template<class H, class C>
int createDArrayVMT(Word* args, Word& result, int message,
          Word& local, Supplier s ){

   CcBool* cctrace = (CcBool*) args[4].addr;
   bool trace = cctrace->IsDefined()?cctrace->GetValue():false;
   int host = ((CcInt*) args[5].addr)->GetValue();
   int port = ((CcInt*) args[6].addr)->GetValue();
   int config = ((CcInt*) args[7].addr)->GetValue();



   result = qp->ResultStorage(s);
   DArray* res = (DArray*) result.addr;

   CcString* name = (CcString*) args[1].addr;
   CcInt* size = (CcInt*) args[2].addr;
   if(!size->IsDefined() || !name->IsDefined()){
      res->makeUndefined();
      if(trace){
        sendMessage("createDArray: size or name not defined");
      }
      return 0; 
   }
   int si = size->GetValue();
   string n = name->GetValue();
   if(si<=0 || !stringutils::isIdent(n)){
      res->makeUndefined();
      if(trace){
        sendMessage("createDArray: invalid value for size or name");
      }
      return 0; 
   }
   vector<DArrayElement> v;
   Stream<Tuple> stream(args[0]);
   stream.open();
   Tuple* tuple;
   int count = 0;
   while((tuple=stream.request())){
     H* h = (H*)  tuple->GetAttribute(host); 
     CcInt* p = (CcInt*) tuple->GetAttribute(port);
     C* c = (C*) tuple->GetAttribute(config);
     if(h->IsDefined() && p->IsDefined() && c->IsDefined()){
        string ho = h->GetValue();
        int po = p->GetValue();
        string co = c->GetValue();
        if(po>0){
           DArrayElement elem(ho,po,count,co);
            count++;
            v.push_back(elem);
        }
     }
     tuple->DeleteIfAllowed();
   }
   stream.close();
   res->set(si,n,v);

   // step 1:  Build groups of workers working on the same SecondoHome
   try{
     vector<vector<pair<DArrayElement, size_t> > > groups = group(res);


     // step 2: for each of these groups connect to one instance from this
     vector<vector<string> > groupobjects;
     string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
     vector<int> themap;
     for(size_t i=0;i<si;i++){
       themap.push_back(-1);
     }
     for(size_t i=0;i<groups.size();i++){
       vector<pair<DArrayElement,size_t> >& g = groups[i];
       DArrayElement elem = g[0].first;
       ConnectionInfo* ci = algInstance->getWorkerConnection(elem, dbname);
       if(!ci){
         throw new SecondoException("worker cannot be reached");
       }
       string re = n + "_"+"[0-9]+"; 
       string query =   "query getcatalog() filter[regexmatches(.ObjectName,"
                        " [const regex value '"
                      + re 
                      + "' ])] project[ObjectName, TypeExpr] consume";
       ListExpr reslist;
       int err;
       string errMsg;
       double runtime; 
       ci->simpleCommand(query, err, errMsg, reslist, false, runtime);
       if(err!=0){
          if(trace){
            sendMessage("createDArray: command " + query + " failed");
          }
          res->makeUndefined();
          return 0;
       } 
       ListExpr tuplelist = nl->Second(reslist);
       vector<int> numbers;
       ListExpr expType = nl->Second(qp->GetType(s));
       while(!nl->IsEmpty(tuplelist)){
         ListExpr tuple = nl->First(tuplelist);
         tuplelist = nl->Rest(tuplelist);
         // shoud not be necessarcy to check .. be who knows it?
         if(   nl->HasLength(tuple,2)  
            && (nl->AtomType(nl->First(tuple))==StringType)
            && (nl->AtomType(nl->Second(tuple))==TextType)){
           string ObjectName = nl->StringValue(nl->First(tuple));
           string typeDescr = nl->Text2String(nl->Second(tuple));
           ListExpr typelist;
           if(nl->ReadFromString(typeDescr, typelist)){
             if(nl->Equal(typelist, expType)){
                bool ok;
                string nu = ObjectName.substr(n.length()+1);
                int num = stringutils::str2int<int>(nu,ok);
                assert(ok);
                if(num < si){
                   numbers.push_back(num);
                }
             } else {
                if(trace){
                  sendMessage("createDArray: type of stored object ("
                             + ObjectName + ") does not fit the array type");
                }
                res->makeUndefined();
                return 0;
             }
   
           }
         }
       } // processing tuple list
       for(size_t i=0;i<numbers.size();i++){
          if(themap[numbers[i]] >= 0){
             if(trace){
                sendMessage("slot number " + stringutils::int2str(numbers[i])
                             +  " found on several workers: "); 
             }
             res->makeUndefined();
             return 0;
          } else {
             themap[numbers[i] ] = g[i % g.size()].second;
          }
       }
     } // for all groups
  
     // check whether all slots where found
     for( size_t i=0;i<themap.size();i++){
        if(themap[i] <0){
           if(trace){
             sendMessage( "createDArray: no worker found storing slot " 
                         + stringutils::int2str(i));
           }
           res->makeUndefined();
           return 0;
        } else {
           res->setResponsible(i,themap[i]);
        }
     }
  
     return 0;
   } catch(SecondoException e){
     if(trace){
        sendMessage(string("createDArray: Exception coourced") + e.what());
     }
     res->makeUndefined();
     return 0;
   } catch(...) {
     if(trace){
        sendMessage("createDArray: unknown exception occured");
     }
     res->makeUndefined();
     return 0;
   }
}

/*
1.4.5 Value Mapping Array and Selection Function

*/
ValueMapping createDArrayVM[] = {
   createDArrayVMT<CcString,CcString>,
   createDArrayVMT<CcString,FText>,
   createDArrayVMT<FText,CcString>,
   createDArrayVMT<FText,FText>
};

int createDArraySelect(ListExpr args){
  ListExpr ht;
  ListExpr ct;
  ListExpr s = nl->Second(nl->Second(nl->First(args)));
  listutils::findAttribute(s,"Host",ht);
  listutils::findAttribute(s,"Config",ct);

  int n1 = CcString::checkType(ht)?0:1; 
  int n2 = CcString::checkType(ct)?0:1; 
  return n2 + n1*2;
}

/*
1.4.6 Specification

*/
OperatorSpec createDArraySpec(
     " stream<TUPLE> x string x int x ANY [x bool]  -> darray",
     " _ createDArray[name, size, type template [, verbose] ]",
     "Creates a darray. The workers are given by the input stream. "
     "If the boolean argument is present and true, in case of an error,"
     " messages will be send.",
     " query workers feed createDArray[\"obj\", 6, streets, TRUE] "
     );

/*
1.4.7 Operator instance

*/
Operator createDArrayOp(
  "createDArray",
  createDArraySpec.getStr(),
  4,
  createDArrayVM,
  createDArraySelect,
  createDArrayTM
);


/*
1.5 Operator pput

This operator sets some values of the darray in parallel.


*/
ListExpr pputTM(ListExpr args){
  string err = "darray(T) x (int x T)+ expected";
  if(nl->ListLength(args) <2){
    return listutils::typeError(err);
  }
  ListExpr darray = nl->First(args);
  ListExpr pairs = nl->Rest(args);
  if(!DArray::checkType(darray)){
    return listutils::typeError(err);
  }
  ListExpr subType = nl->Second(darray);
  bool index = true;
  while(nl->IsEmpty(pairs)){
    if(index){
       if(!CcInt::checkType(nl->First(pairs))){
          return listutils::typeError("array index not an int");
       }
    } else {
       if(!nl->Equal(subType,nl->First(pairs))){
         return listutils::typeError("expression does not fit darray subtype");
       }
    }
    pairs = nl->Rest(pairs);
    index = !index; 
  }
  if(!index){
    return listutils::typeError("Missing expression for index");
  }
  return darray;
}


class SinglePutter{

  public:
    SinglePutter(ListExpr _type, DArray* _array, 
                 int _arrayIndex, Word& _value):
     type(_type), array(_array), arrayIndex(_arrayIndex), value(_value){
       runner = boost::thread(&SinglePutter::run,this);
    }

    ~SinglePutter(){
        runner.join();
    }
 

  private:
     ListExpr type;
     DArray* array;
     int arrayIndex;
     Word value;
     boost::thread runner;


    void run(){
      string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
      DArrayElement elem = 
                    array->getWorkerForSlot(arrayIndex);
      ConnectionInfo* ci = algInstance->getWorkerConnection(elem, dbname);
      if(!ci){
           cerr << "could not connect to server " << elem << endl;
           return;
      } 
      string objname = array->getName() + "_"+stringutils::int2str(arrayIndex); 
      ci->createOrUpdateObject(objname, type, value);
    }


};

class PPutter{
  public:

    PPutter(ListExpr _type,
            DArray* _arg, vector<pair<int,Word> >& _values){
      this->type = _type;
      this->source = _arg;
      set<int> used;
      vector<pair<int,Word> >::iterator it;
      for(it = _values.begin(); it!=_values.end(); it++){
            pair<int,Word> p = *it;
            int index = p.first;
            if( (index >= 0) && ( (size_t)index < source->getSize())
                 && (used.find(index)==used.end())){
              values.push_back(p);
            }
      }
      started = false;
    }

    void start(){
       boost::lock_guard<boost::mutex> guard(mtx);
       if(!started){
         started=true;
         for(size_t i=0;i<values.size();i++){
            SinglePutter* sp = new SinglePutter(type,source,values[i].first, 
                                                values[i].second);
            putters.push_back(sp);
         }
       }
    }  

    ~PPutter(){
         for(size_t i=0;i<putters.size();i++){
            delete putters[i];
         }
      
    } 

  private:
    ListExpr type;
    DArray* source;
    vector<pair<int,Word> > values;
    vector<SinglePutter*> putters;
    bool started;
    boost::mutex mtx;

};

/*
1.4.2 Value Mapping

*/

int pputVM(Word* args, Word& result, int message,
           Word& local, Supplier s ){

   result = qp->ResultStorage(s);
   DArray* res = (DArray*) result.addr;

   DArray* arg = (DArray*) args[0].addr;

   // collect the value to set into an vector
   // ignoring undefined indexes or indexes 
   // out of range, if an index is specified
   // more than once, the first one wins
   int sons = qp->GetNoSons(s);
   int i=1;
   set<int> used;

   vector<pair<int, Word> > pairs;

   while(i<sons){
     CcInt* Index = (CcInt*) args[i].addr;
     i++;
     Word w = args[i];
     i++;
     if(Index->IsDefined()){
       int index = Index->GetValue();
       if( (index>=0 )&& 
           ((size_t) index < arg->getSize()) && 
           (used.find(index)==used.end())){
           pair<int,Word> p(index,w);
           pairs.push_back(p);
           used.insert(index);
       }
     }
   }

   
   PPutter* pput = new PPutter(nl->Second(qp->GetType(s)),arg, pairs);
   pput->start(); 
   delete pput;
   *res = *arg;
   return 0;
}

/*
1.4.3 Specification

*/


OperatorSpec pputSpec(
     " darray(T) x (int x T)+ -> darray ",
     " _ pput[ index , value , index, value ,...]",
     " Puts elements into a darray in parallel. ",
     " query da pput[0, streets1, 1, streets2]  "
     );


/*
1.4.4 Operator instance

*/

Operator pputOp(
  "pput",
  pputSpec.getStr(),
  pputVM,
  Operator::SimpleSelect,
  pputTM
);


/*
1.6 Operator ddistribute2

1.6.1 Type Mapping

*/

/*
Function ~isWorkerRelDesc~

This function checks wether a relation is a valid description of
a set of workers. It returns true if it is so , false otherwise.
If the list is correct, the result parameter will be a list
with three arguments containing the postions of the attributes
~Host~, ~Port~, and ~Config~. In the false case, the errMsg
parameter will contain an error message. 

*/

bool isWorkerRelDesc(ListExpr rel, ListExpr& positions, ListExpr& types,
                     string& errMsg){

  if(!Relation::checkType(rel)){
     errMsg = " not a relation";
     return false;
  }
  ListExpr attrList = nl->Second(nl->Second(rel));

  ListExpr htype;

  int hostPos = listutils::findAttribute(attrList,"Host",htype);
  if(!hostPos){
     errMsg = "Attribute Host not present in relation";
     return false;
  }
  if(!CcString::checkType(htype) && !FText::checkType(htype)){
     errMsg = "Attribute Host not of type text or string";
     return false;
  }
  hostPos--;

  ListExpr ptype;
  int portPos = listutils::findAttribute(attrList,"Port",ptype);
  if(!portPos){
    errMsg = "Attribute Port not present in relation";
    return false;
  }
  if(!CcInt::checkType(ptype)){
     errMsg = "Attribute Port not of type int";
     return false;
  }
  portPos--;
  ListExpr ctype;
  int configPos = listutils::findAttribute(attrList, "Config", ctype);
  if(!configPos){
    errMsg = "Attrribute Config not present in relation";
    return false;
  }
  if(!CcString::checkType(ctype) && !FText::checkType(ctype)){
     errMsg = "Attribute Config not of type text or string";
     return false;
  }
  configPos--;
  positions = nl->ThreeElemList(
               nl->IntAtom(hostPos),
               nl->IntAtom(portPos),
               nl->IntAtom(configPos));
  types = nl->ThreeElemList(htype, ptype,ctype);

  return true;
}

template<class R>
ListExpr ddistribute2TMT(ListExpr args){

  string err = "stream(tuple(X)) x string x ident x int x rel expected";

  if(!nl->HasLength(args,5)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  // check for correct types
  if(!Stream<Tuple>::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(!CcString::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  if(nl->AtomType(nl->Third(args))!=SymbolType){
    return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->Fourth(args))){
    return listutils::typeError(err);
  }
  if(!Relation::checkType(nl->Fifth(args))){
    return listutils::typeError(err);
  }

  // retrieve position of the attribute for distributing the relation

  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  string ident = nl->SymbolValue(nl->Third(args));
  ListExpr dType;
  int pos = listutils::findAttribute(attrList, ident ,dType);
  if(!pos){
     return listutils::typeError("Attribute " + ident + " unknown");
  }
  if(!CcInt::checkType(dType)){
     return listutils::typeError("Attribute " + ident + " not of type int");
  }
  //
  ListExpr workerAttrPositions;
  ListExpr workerAttrTypes;
  string errMsg;
  if(!isWorkerRelDesc(nl->Fifth(args), workerAttrPositions, 
                      workerAttrTypes,errMsg)){
     return listutils::typeError("Fifth arg does not describe a valid "
                                 "worker relation: " + errMsg);
  }

  ListExpr appendList = listutils::concat(workerAttrPositions,
                                   nl->OneElemList( nl->IntAtom(pos-1)));

  ListExpr res = nl->TwoElemList(
                    listutils::basicSymbol<R>(),
                    nl->TwoElemList(
                        listutils::basicSymbol<Relation>(),
                        nl->Second(nl->First(args))));

  return nl->ThreeElemList(
                  nl->SymbolAtom(Symbols::APPEND()),
                  appendList,
                  res);
}

/*
1.6.2 Auxiliary class ~RelFileRestore~

This class restores a relation stored within a binary file
on a remote server within an own thread.

*/

class RelFileRestorer{

 public:
   RelFileRestorer(const string& _objName,
                   DArray* _array, int _arrayIndex, 
                   const string& _filename):
    objNames(), array(_array), 
    arrayIndex(_arrayIndex), filenames(),
    started(false){
    objNames.push_back(_objName);
    filenames.push_back(_filename);
    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    ci = algInstance->getWorkerConnection(
                         array->getWorkerForSlot(arrayIndex),dbname);
   }

   RelFileRestorer(const vector<string>& _objNames,
                   DArray* _array, int _arrayIndex, 
                   const vector<string>& _filenames):
    objNames(_objNames), array(_array), 
    arrayIndex(_arrayIndex), filenames(_filenames),
    started(false){
    assert(objNames.size() == filenames.size());
    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    ci = algInstance->getWorkerConnection(
                         array->getWorkerForSlot(arrayIndex),dbname);
   }


   ~RelFileRestorer(){
     boost::lock_guard<boost::mutex> guard(mtx);
     runner.join();
    }

   void start(){
     boost::lock_guard<boost::mutex> guard(mtx);
     if(!started && ci ){
        started = true;
        res = true;
        checkServerFree();
        runner = boost::thread(&RelFileRestorer::run, this);
     }
   }

   static void cleanUp(){
       map<pair<string,int>,boost::mutex*>::iterator it;
       for(it = serializer.begin();it!=serializer.end();it++){
             delete it->second;
       }
       serializer.clear();
   }


 private:
    //ListExpr relType;
    vector<string> objNames;
    DArray* array;
    int arrayIndex;
    vector<string> filenames;
    bool started;
    boost::mutex mtx;
    boost::thread runner;
    bool res;
    ConnectionInfo* ci;

    static map<pair<string,int>,boost::mutex*> serializer;
    static boost::mutex sermtx;


    void run(){
      if(ci){
        for(size_t i=0;i<objNames.size();i++){
           string& objName = objNames[i];
           string& filename = filenames[i];
           res = ci->createOrUpdateRelationFromBinFile(objName,filename);
           if(!res){
             cerr << "createorUpdateObject failed" << endl;
           }
           sermtx.lock();
           pair<string,int> p(ci->getHost(), ci->getPort());
           serializer[p]->unlock();
           sermtx.unlock();
       }
      } else {
        cerr << "connection failed" << endl;
      }
    }

    void checkServerFree(){
       sermtx.lock();
       map<pair<string,int>,boost::mutex*>::iterator it;
       pair<string,int> p(ci->getHost(), ci->getPort());
       it = serializer.find(p);
       if(it==serializer.end()){
          boost::mutex* mtx = new boost::mutex();
          serializer[p] = mtx;
       }        
       boost::mutex* m = serializer[p];
       sermtx.unlock();
       m->lock();
    }

};



class FRelCopy{

  public:

     FRelCopy(const string& _objName,
              DFArray* _array, int _arrayIndex, 
              const string& _filename):
     names(), slot(_arrayIndex), array(_array), started(false),ci(0)
     {
        names.push_back(_filename);
        dbname = SecondoSystem::GetInstance()->GetDatabaseName();
        ci = algInstance->getWorkerConnection(
                                      array->getWorkerForSlot(slot), dbname);

     }
     

     FRelCopy(const vector<string>& _objNames,
              DFArray* _array, int _arrayIndex, 
              const vector<string>& _filenames):
     names(_filenames), slot(_arrayIndex), array(_array), started(false),ci(0)
     {
        dbname = SecondoSystem::GetInstance()->GetDatabaseName();
        ci = algInstance->getWorkerConnection(
                                      array->getWorkerForSlot(slot), dbname);

     }

   ~FRelCopy(){
     boost::lock_guard<boost::mutex> guard(mtx);
     runner.join();
    }

   void start(){
     boost::lock_guard<boost::mutex> guard(mtx);
     if(!started && ci ){
        started = true;
        runner = boost::thread(&FRelCopy::run, this);
     }
   }

   static void cleanUp(){}


  private:
     vector<string> names;   // filename
     int slot;
     DFArray* array; 
     bool started;
     ConnectionInfo* ci;
     string dbname;
     boost::mutex mtx;
     boost::thread runner;

     void run(){
        if(!ci){
          cerr << "Connection failed" << endl;
          return;
        }
        for(size_t i=0;i<names.size();i++){
          string& name = names[i];
          // send file to remote server
          ci->sendFile(name,name,true);
          // get target directory
          int err;
          string errmsg;
          ListExpr result; 
          double runtime;
          string home = ci->getSecondoHome();

          string targetDir = home +"/dfarrays/"+dbname+ "/" + array->getName();

          // result will be false, if directory already exists
          // hence ignore result here
          string sendDir = ci->getSendFolder();
          string src = home+'/'+sendDir + "/"+name;
          string target = targetDir+"/"+name;
          string cmd = "query moveFile('"+src+"','"+target+"', TRUE)";
          ci->simpleCommand(cmd,err,errmsg, result,false, runtime);
          if(err!=0){
              cerr << "command " << cmd << " failed" << endl;
              cerr << err << " : " << errmsg << endl;
          }
          if(!nl->HasLength(result,2) || 
              nl->AtomType(nl->Second(result))!=BoolType){
              cerr << "moveFile returns unexpected result: " 
                   << nl->ToString(result) << endl;
          }

          if(!nl->BoolValue(nl->Second(result))){
             cerr << "moving file from: " << src << endl
                  << "to " << target << endl
                  << "failed" << endl;   
        
          } 
      }
     }
};

map<pair<string,int>,boost::mutex*> RelFileRestorer::serializer;
boost::mutex RelFileRestorer::sermtx;

/*
1.6.3 Value Mapping

*/
template<class AType, class DType, class HostType, class ConfigType>
int ddistribute2VMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){

   result = qp->ResultStorage(s);
   AType* res = (AType*) result.addr; 


   CcInt* Size = (CcInt*) args[3].addr;
   CcString* Name = (CcString*) args[1].addr;

   if(!Size->IsDefined() || !Name->IsDefined()){
      res->makeUndefined();
      return 0;
   }
   int isize = Size->GetValue();
   string name = Name->GetValue();

   if(name.size()==0){
      name = algInstance->getTempName();
   } 

   if( (isize<=0) || !stringutils::isIdent(name)){
      res->makeUndefined();
      return 0;
   }
   size_t size = (size_t) isize;


   Relation* rel = (Relation*) args[4].addr;
   int hostPos = ((CcInt*) args[5].addr)->GetValue();
   int portPos = ((CcInt*) args[6].addr)->GetValue();
   int configPos = ((CcInt*) args[7].addr)->GetValue();

   res->copyFrom(
          DArrayBase::createFromRel<HostType,ConfigType, AType>(rel,size,name,
                                       hostPos,portPos,configPos));

   if(res->numOfWorkers()==0){
      res->makeUndefined();
      return 0;
   }

   int pos = ((CcInt*) args[8].addr)->GetValue();

   // distribute the incoming tuple stream to a set of files
   // if the distribution number is not defined, the tuple is
   // treated as for number 0
   map<int , pair<string, pair<ofstream*, char*> > > files;
   Stream<Tuple> stream(args[0]);
   stream.open();
   Tuple* tuple;

   ListExpr relType = nl->Second(qp->GetType(s));

   size_t bufsize = max((size_t)4096u, FILE_BUFFER_SIZE*10/size);

   while((tuple=stream.request())){
      CcInt* D = (CcInt*) tuple->GetAttribute(pos);
      int index = D->IsDefined()?D->GetValue():0;
      index = index % res->getSize(); // adopt to array size
      string fn = name + "_" + stringutils::int2str(index)+".bin";
      if(files.find(index)==files.end()){
          ofstream* out = new ofstream(fn.c_str(), ios::out | ios::binary);
          char* buffer = new char[bufsize];
          out->rdbuf()->pubsetbuf(buffer, bufsize); 
          pair<ofstream*, char*> p1(out,buffer);
          pair<string, pair<ofstream*, char* > > p(fn,p1);
          files[index] = p;
          BinRelWriter::writeHeader(*out,relType); 
      }
      BinRelWriter::writeNextTuple(*(files[index].second.first),tuple);
      tuple->DeleteIfAllowed();
   }
   stream.close();
   // finalize files


   vector<DType*> restorers;

   
   typename map<int, pair<string,pair<ofstream*, char*> > >::iterator it;

   for(int i=0;i<res->getSize();i++){
      it = files.find(i); 
      string objName = res->getName()+"_"+stringutils::int2str(i);
      string fn;
      if(it==files.end()){
         // create empty file
         fn = name + "_" + stringutils::int2str(i)+".bin";
         ofstream out(fn.c_str(), ios::out | ios::binary);
         BinRelWriter::writeHeader(out,relType);
         BinRelWriter::finish(out);
         out.close();
      } else {
         BinRelWriter::finish(*(it->second.second.first));
         fn = it->second.first;
         // close and delete ofstream
         it->second.second.first->close();
         delete it->second.second.first;
         delete[] it->second.second.second;
      }
      restorers.push_back(new DType( objName,res, i, fn));
   }


   // distribute the files to the workers and restore relations
   for(size_t i=0;i<restorers.size();i++){
     restorers[i]->start();
   } 
   // wait for finishing restore

   
   for(size_t i=0;i<restorers.size();i++){
     delete restorers[i];
   } 
   for(int i=0;i<res->getSize();i++){
      FileSystem::DeleteFileOrFolder(name + "_" 
                                + stringutils::int2str(i)+".bin");
   }

   DType::cleanUp();

   return 0;   
}

/*
1.6.4 Specification

*/
OperatorSpec ddistribute2Spec(
     " stream(tuple(X)) x string x ident x int x rel -> darray(X) ",
     " _ ddistribute2[ _, _,_,_]",
     "Distributes a locally stored relation into a darray. "
     "The first argument is the tuple stream to distribute. The second "
     "Argument is the name for the resulting darray. If the name is "
     "an empty string, a name is choosen automatically. "
     "The third argument is an attribute within the stream of type int. "
     "This attribute controls in which slot of the resulting array "
     "is the corresponding tuple inserted. The ifourth argument specifies "
     "the size of the resulting array. The relation argument specifies "
     "the workers for this array. It must be a relation having attributes "
     "Host, Port, and Config. Host and Config must be of type string or text, "
     "the Port attribute must be of type int. ", 
     " query strassen feed addcounter[No,1] ddistribute2[\"dstrassen\", No, 5,"
     " workers, "
     );



ValueMapping ddistribute2VM[] = {
  ddistribute2VMT<DArray,RelFileRestorer, CcString,CcString>,
  ddistribute2VMT<DArray,RelFileRestorer, CcString,FText>,
  ddistribute2VMT<DArray,RelFileRestorer, FText,CcString>,
  ddistribute2VMT<DArray,RelFileRestorer, FText,FText>,
};




int distribute2Select(ListExpr args){

  ListExpr rel = nl->Fifth(args);
  ListExpr attrList = nl->Second(nl->Second(rel));
  ListExpr hostType, configType;
  listutils::findAttribute(attrList,"Host",hostType);
  listutils::findAttribute(attrList,"Config", configType);
  int n1 = CcString::checkType(hostType)?0:2;
  int n2 = CcString::checkType(configType)?0:1;
  return n1 + n2;
}


/*
1.6.5 Operator instance

*/
Operator ddistribute2Op(
  "ddistribute2",
  ddistribute2Spec.getStr(),
  4,
  ddistribute2VM,
  distribute2Select,
  ddistribute2TMT<DArray>
);



/*
1.7 Operator ~ddistribute3~

Similar to the ddistribute2 operator, this operator distributes a tuple strean
into a darray object. The difference is how the tuples of the incoming stream
are distributed.  While the ddistribute2 operator requires an integer attribute
signaling to which slot of the array the tuple should be stored, this operator
get a partion size of the size of the array. The variant (meaning of the int
argument) is chosen by a boolean argument.

1.7.1 Type Mapping

*/
template<class R>
ListExpr distribute3TM(ListExpr args){
  string err = "stream(tuple) x string x int x bool x rel expected";
  if(!nl->HasLength(args,5)){
    return listutils::typeError(err+": wrong number of args");
  }

  if(   !Stream<Tuple>::checkType(nl->First(args))
     || !CcString::checkType(nl->Second(args))
     || !CcInt::checkType(nl->Third(args))
     || !CcBool::checkType(nl->Fourth(args)) ){
    return listutils::typeError(err);
  }
  ListExpr positions;
  string errMsg;
  ListExpr types;
  if(!isWorkerRelDesc(nl->Fifth(args),positions,types, errMsg)){
     return listutils::typeError("fourth argument is not a worker relation:" 
                                 + errMsg);
  }

  ListExpr resType = nl->TwoElemList(
                          listutils::basicSymbol<R>(), 
                          nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second(nl->First(args))));
  return nl->ThreeElemList(
                 nl->SymbolAtom(Symbols::APPEND()),
                 positions,
                 resType);
}



/*
1.7.3 Value Mapping

Template arguments are:

  AType : resulting array type
  DType : class distributing the created files
  HType : type of Host in workers relation
  CType : type of Config in workers relation

*/

template<class AType, class DType, class HType,class CType>
int distribute3VMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){

   result = qp->ResultStorage(s);
   AType* res = (AType*) result.addr;

   CcString* n = (CcString*) args[1].addr;
   CcInt* size = (CcInt*) args[2].addr;
   CcBool* method = (CcBool*) args[3].addr;
   Relation* workers = (Relation*) args[4].addr;

   if(!size->IsDefined() || !method->IsDefined() 
      || !n->IsDefined()){
     res->makeUndefined();
     return 0;
   }

   int sizei = size->GetValue();
   if(sizei<=0){
     res->makeUndefined();
     return 0;
   }
   string name = n->GetValue();
  
   if(name.size()==0){
      name = algInstance->getTempName();
   }

   if(!stringutils::isIdent(name)){
     res->makeUndefined();
     return 0;
   }

   int hostPos = ((CcInt*) args[5].addr)->GetValue();
   int portPos = ((CcInt*) args[6].addr)->GetValue();
   int configPos = ((CcInt*) args[7].addr)->GetValue();

   res->copyFrom(DArrayBase::createFromRel<HType,CType,AType>(workers, 1, 
                 name, hostPos, portPos, configPos));
   

   if(res->numOfWorkers() < 1 ){
     // no valid workers found in relation
     res->makeUndefined();
     return 0;
   }


   // distribute incoming tuple stream 
   // to files

   bool methodb = method->GetValue();

   vector< pair<ofstream*, char*> > files;
   Stream<Tuple> stream(args[0]);
   stream.open();
   Tuple* tuple;
   ListExpr relType = nl->Second(qp->GetType(s));
   if(methodb){ // size is the size of the darray
      // circular distribution of 
      res->setStdMap(sizei);
      int index = 0;
      ofstream* current=0;
      size_t bufsize = max(4096, (FILE_BUFFER_SIZE*16) / sizei);

      while((tuple=stream.request())){
         size_t index1 = index % sizei; // adopt to array size
         index++;
         if(index1 >= files.size()){
             string fn = name + "_" + stringutils::int2str(index1)+".bin";
             current = new ofstream(fn.c_str(), ios::out | ios::binary);
             char* buf = new char[bufsize];
             current->rdbuf()->pubsetbuf(buf,bufsize);
             pair<ofstream*, char*> p(current,buf);
             files.push_back(p);
             BinRelWriter::writeHeader(*current,relType); 
          } else {
              current = files[index1].first;
          }
          BinRelWriter::writeNextTuple(*current,tuple);
          tuple->DeleteIfAllowed();
      }
      for(size_t i=0;i<files.size();i++){
         BinRelWriter::finish(*(files[i].first));
         files[i].first->close();
         delete files[i].first;
         delete[] files[i].second;
      }
   } else {
      int written = 0;
      ofstream* current=0;
      char* buf = 0;
      while((tuple=stream.request())){
         if(written==0){
           string fn = name + "_" + stringutils::int2str(files.size())+".bin";
           current = new ofstream(fn.c_str(), ios::out | ios::binary);
           buf = new char[FILE_BUFFER_SIZE];
           current->rdbuf()->pubsetbuf(buf,FILE_BUFFER_SIZE);
           pair<ofstream*, char*> p(current, buf);
           files.push_back(p);
           BinRelWriter::writeHeader(*current,relType); 
         }
         BinRelWriter::writeNextTuple(*current,tuple);
         written++;
         if(written==sizei){
           BinRelWriter::finish(*current);
           current->close();
           delete current;
           delete[] buf;
           current=0;
           written = 0;
         }
         tuple->DeleteIfAllowed();
      }     
      if(current){
        BinRelWriter::finish(*current);
        current->close();
        delete current;
        delete[] buf;
        current=0;
      } 
      res->setStdMap(files.size());
   }
   stream.close();



    // now, all tuples are distributed to files.
    // we have to put the relations stored in these
    // files to the workers

   vector<DType*> restorers;

   for(size_t i = 0; i<files.size(); i++) {
      string objName = res->getName()+"_"+stringutils::int2str(i);     
      string fn = objName + ".bin";
      restorers.push_back(new DType(objName,res, 
                                    i, fn));
   }

   // distribute the files to the workers and restore relations
   for(size_t i=0;i<restorers.size();i++){
     restorers[i]->start();
   } 
   // wait for finishing restore


   for(size_t i=0;i<restorers.size();i++){
     delete restorers[i];
   }

   // delete local files
   for(size_t i=0;i<files.size();i++){
      string fn = res->getName()+"_"+stringutils::int2str(i)+".bin";     
      FileSystem::DeleteFileOrFolder(fn); 
   }
   DType::cleanUp();

   return 0;   
}


/*
1.7.4 Specification

*/
OperatorSpec ddistribute3Spec(
     " stream(tuple(X)) x string x int x bool x rel -> darray(X) ",
     " _ ddistribute3[ _, _,_,_]",
     "Distributes a tuple stream into a darray. The boolean "
     "flag controls the method of distribution. If the flag is set to "
     "true, the integer argument specifies the target size of the "
     "resulting darray and the tuples are distributed in a circular way. "
     "In the other case, this number represents the size of a single "
     "array slot. A slot is filled until the size is reached. After that "
     "a new slot is opened. The string attribute gives the name of the "
     "result. The fifth attribute is a relation with attributes "
     "Host (string,text), Port(int), and Config(string,text) containing "
     "the workers for the resulting array.",
     " query strassen feed ddistribute3[\"da28\",10, TRUE, workers ]  "
     );


/*
1.7.5 Operator instance

*/
ValueMapping ddistribute3VM[] = {
  distribute3VMT<DArray,RelFileRestorer, CcString, CcString>,   
  distribute3VMT<DArray,RelFileRestorer, CcString, FText>,   
  distribute3VMT<DArray,RelFileRestorer, FText, CcString>,   
  distribute3VMT<DArray,RelFileRestorer, FText, FText>   
};

int distribute3Select(ListExpr args){
  ListExpr rel = nl->Fifth(args);
  ListExpr attrList = nl->Second(nl->Second(rel));
  ListExpr hostType, configType;
  listutils::findAttribute(attrList,"Host",hostType);
  listutils::findAttribute(attrList,"Config", configType);
  int n1 = CcString::checkType(hostType)?0:2;
  int n2 = CcString::checkType(configType)?0:1;
  return n1 + n2;
}



Operator ddistribute3Op(
  "ddistribute3",
  ddistribute3Spec.getStr(),
  4,
  ddistribute3VM,
  distribute3Select,
  distribute3TM<DArray>
);


/*
1.8 Operator ~ddistribute4~

This Operator uses a function: tuple -> int for distributing
a tuple stream to an darray.

1.8.1 Type Mapping

*/
template<class R>
ListExpr distribute4TMT(ListExpr args){
  string err ="stream(tuple) x string x (tuple->int) x int x rel expected";
  if(!nl->HasLength(args,5)){
    return listutils::typeError(err + "( wrong number of args)");
  }
  if(    !Stream<Tuple>::checkType(nl->First(args))
      || !CcString::checkType(nl->Second(args))
      || !listutils::isMap<1>(nl->Third(args))
      || !CcInt::checkType(nl->Fourth(args))){
    return listutils::typeError(err);
  }
  string errMsg;
  ListExpr positions;
  ListExpr types;
  if(!isWorkerRelDesc(nl->Fifth(args),positions, types, errMsg)){
     return listutils::typeError("fourth arg is not a worker relation: "
                                 + errMsg);
  }


  ListExpr funargType = nl->Second(nl->Third(args));
  if(!nl->Equal(funargType, nl->Second(nl->First(args)))){
    return listutils::typeError("tuple type and function arg type differ");
  }
  ListExpr funResType = nl->Third(nl->Third(args));
  if(!CcInt::checkType(funResType)){
    return listutils::typeError("result of function not an int");
  }

  ListExpr res =  nl->TwoElemList(
                      listutils::basicSymbol<R>(),
                      nl->TwoElemList(
                          listutils::basicSymbol<Relation>(),
                          nl->Second(nl->First(args))));
  return nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            positions,
            res);    
}


/*
1.8.2 Value Mapping

*/
template<class AType, class DType,class HType, class CType>
int distribute4VMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){

   result = qp->ResultStorage(s);
   AType* res   = (AType*) result.addr;

   CcString* n = (CcString*) args[1].addr;
   CcInt* si = (CcInt*) args[3].addr;
   Relation* rel = (Relation*) args[4].addr;
   
   if(!n->IsDefined() || !si->IsDefined()){
     res->makeUndefined();
     return 0;
   }
   string name = n->GetValue();
   if(name.size()==0){
      name = algInstance->getTempName();
   }
   int siz = si->GetValue();
   if(!stringutils::isIdent(name) || (siz<=0)){
      res->makeUndefined();
      return 0;
   }
   size_t size = (size_t) siz;
   int hostPos = ((CcInt*) args[5].addr)->GetValue();
   int portPos = ((CcInt*) args[6].addr)->GetValue();
   int configPos = ((CcInt*) args[7].addr)->GetValue();

   (*res) = DArrayBase::createFromRel<HType,CType,AType>(rel, size,
                               name, hostPos, portPos, configPos);

   if(!res->IsDefined() || (res->numOfWorkers()<1) || (res->getSize() < 1)){
      res->makeUndefined();
      return 0;
   }

   // distribute the incoming tuple stream to a set of files
   // if the distribution number is not defined, the tuple is
   // treated as for number 0
   map<int , pair<string,pair<ofstream*, char*> > > files;
   Stream<Tuple> stream(args[0]);
   stream.open();
   Tuple* tuple;

   ListExpr relType = nl->Second(qp->GetType(s));

   ArgVectorPointer funargs = qp->Argument(args[2].addr);
   Word funres;

   size_t bufsize = max((size_t)4096, (FILE_BUFFER_SIZE*16) / res->getSize());

   while((tuple=stream.request())){
      (* funargs[0]) = tuple;
      qp->Request(args[2].addr, funres);

      CcInt* fr = (CcInt*) funres.addr;
      int index = fr->IsDefined()?fr->GetValue():0;
      index = index % res->getSize();
      

      string fn = name + "_" + stringutils::int2str(index)+".bin";
      if(files.find(index)==files.end()){
          ofstream* out = new ofstream(fn.c_str(), ios::out | ios::binary);
          char* buf = new char[bufsize];
          out->rdbuf()->pubsetbuf(buf,bufsize);
          pair<string, pair<ofstream*, char*> >
                        p(fn,pair<ofstream*,char*>(out,buf));
          files[index] = p;
          BinRelWriter::writeHeader(*out,relType); 
      }
      BinRelWriter::writeNextTuple(*(files[index].second.first),tuple);
      tuple->DeleteIfAllowed();
   }
   stream.close();
   // finalize files

   vector<DType*> restorers;

   
   typename map<int, pair<string, pair<ofstream*, char*> > >::iterator it;

   for(int i=0;i<res->getSize();i++){
      it = files.find(i); 
      string objName = res->getName()+"_"+stringutils::int2str(i);
      string fn;
      if(it==files.end()){
         // create empty file
         fn = name + "_" + stringutils::int2str(i)+".bin";
         ofstream out(fn.c_str(), ios::out | ios::binary);
         BinRelWriter::writeHeader(out,relType);
         BinRelWriter::finish(out);
         out.close();
      } else {
         BinRelWriter::finish(*(it->second.second.first));
         fn = it->second.first;
         // close and delete ofstream
         it->second.second.first->close();
         delete it->second.second.first;
         delete[] it->second.second.second;
      }
      restorers.push_back(new DType(objName,res, i, fn));
   }


   // distribute the files to the workers and restore relations
   for(size_t i=0;i<restorers.size();i++){
     restorers[i]->start();
   } 
   // wait for finishing restore

   
   for(size_t i=0;i<restorers.size();i++){
     delete restorers[i];
   } 
   // delete local files
   for(size_t i=0;i<files.size();i++){
      string fn = res->getName()+"_"+stringutils::int2str(i)+".bin";     
      FileSystem::DeleteFileOrFolder(fn); 
   }

   DType::cleanUp();

   return 0;   
}

OperatorSpec ddistribute4Spec(
     " stream(tuple(X)) x string x (tuple->int) x int x rel -> darray(X) ",
     " stream  ddistribute4[ name, fun, size, workers ]",
     " Distributes a locally stored relation into a darray. If the name is "
     "an empty string, a name will be automatically chosen. The function "
     "determines the slot where the tuple will be stored. ",
     " query strassen feed  ddistribute4[\"s200}\", hashvalue(.Name,2000),"
     " 5, workers]  "
     );

int distribute4Select(ListExpr args){
  ListExpr rel = nl->Fifth(args);
  ListExpr attrList = nl->Second(nl->Second(rel));
  ListExpr hostType, configType;
  listutils::findAttribute(attrList,"Host",hostType);
  listutils::findAttribute(attrList,"Config", configType);
  int n1 = CcString::checkType(hostType)?0:2;
  int n2 = CcString::checkType(configType)?0:1;
  return n1 + n2;
}

ValueMapping ddistribute4VM[] = {
    distribute4VMT<DArray, RelFileRestorer, CcString, CcString>,
    distribute4VMT<DArray, RelFileRestorer, CcString, FText>,
    distribute4VMT<DArray, RelFileRestorer, FText, CcString>,
    distribute4VMT<DArray, RelFileRestorer, FText, FText>,
};


/*
1.7.5 Operator instance

*/

Operator ddistribute4Op(
  "ddistribute4",
  ddistribute4Spec.getStr(),
  4,
  ddistribute4VM,
  distribute4Select,
  distribute4TMT<DArray>
);




/*
1.7 fdistribute

*/
ListExpr fdistribute5TM(ListExpr args){
  string err = "stream(tuple) x string x int x attrName expected";
  // stream <base file name> <number of files> <attrName for distribute>
  if(!nl->HasLength(args,4)){
    return listutils::typeError(err);
  }
  if( !Stream<Tuple>::checkType(nl->First(args))
     || (   !CcString::checkType(nl->Second(args)) 
         && !FText::checkType(nl->Second(args)))
     || !CcInt::checkType(nl->Third(args))
     || (nl->AtomType(nl->Fourth(args))!=SymbolType)){
    return listutils::typeError(err);
  }
  string attrName = nl->SymbolValue(nl->Fourth(args));
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr type;
  int pos = listutils::findAttribute(attrList,attrName,type);
  if(!pos){
     return listutils::typeError("AttrName " + attrName + " not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("AttrName " + attrName + " not of type int");
  }
  return nl->ThreeElemList(
                nl->SymbolAtom(Symbols::APPEND()),
                nl->OneElemList(nl->IntAtom(pos-1)),
                nl->First(args));

}

template<class T>
class fdistribute5Info{
  public:
     fdistribute5Info(Word& _stream, CcInt* _size, T* _name,int _pos, 
                      ListExpr relType)
       : stream(_stream){
        write = true;
        if(!_size->IsDefined() || (_size->GetValue() <=0)){
          write = false;
        } else {
          this->size = _size->GetValue();
          bufsize = max(( FILE_BUFFER_SIZE * 10 ) / this->size, 4096);
        }
        if(!_name->IsDefined()){
           write = false;
        } else {
           basename = _name->GetValue();
        }
        this->pos = _pos;
        this->relType = relType;
        stream.open();
     }

     Tuple* next(){
        Tuple* tuple = stream.request();
        if(write && tuple){
            writeNextTuple(tuple);
        }
        return tuple;
     }

     ~fdistribute5Info(){
        map<int, pair<ofstream*,char*> >::iterator it;
        for(it = writers.begin();it!=writers.end() ; it++){
           BinRelWriter::finish(*(it->second.first));
           delete it->second.first;
           delete [] it->second.second;
        }
        stream.close();
     }


  private:
     Stream<Tuple> stream;
     bool write;
     string basename;
     int pos;
     int size;
     ListExpr relType;
     map<int, pair< ofstream*, char*> > writers;
     size_t bufsize;

    
     void writeNextTuple(Tuple* tuple){
        CcInt* num = (CcInt*) tuple->GetAttribute(pos);
        int f =0;
        if(num->IsDefined()){
            f = num->GetValue() % size;
        }
        map<int, pair<ofstream*, char*> >::iterator it = writers.find(f);
        ofstream* out;
        if(it==writers.end()){
           out = new ofstream((basename +"_"+stringutils::int2str(f) 
                               + ".bin").c_str(),
                              ios::out | ios::binary);
           char* buf = new char[bufsize];
           out->rdbuf()->pubsetbuf(buf, bufsize);
           pair<ofstream*, char*> p(out,buf);
           writers[f] = p;
           BinRelWriter::writeHeader(*out,relType);
        } else {
           out = it->second.first;
        }
        BinRelWriter::writeNextTuple(*out, tuple);
     }

};


template<class T>
int fdistribute5VMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){
 
  fdistribute5Info<T>* li = (fdistribute5Info<T>*) local.addr;
  switch(message){
    case OPEN: { 
      T* fname = (T*) args[1].addr;
      CcInt* size = (CcInt*) args[2].addr;
      int attrPos = ((CcInt*)args[4].addr)->GetValue();
      if(li){
         delete li;
      }
      ListExpr relType = nl->TwoElemList(
                            listutils::basicSymbol<Relation>(),
                            nl->Second(qp->GetType(s)));
      local.addr = new fdistribute5Info<T>(args[0],size, fname, attrPos,
                                           relType);
      return 0;
    }
   case REQUEST:
           result.addr = li?li->next():0;
           return result.addr?YIELD:CANCEL;
    case CLOSE:
          if(li){
             delete li;
             local.addr = 0;
          }
          return 0;
   }

    
  return -1;

}

/*
1.7.3 Selction and value mapping

*/

int fdistribute5Select(ListExpr args){
 return CcString::checkType(nl->Second(args))?0:1;
}

ValueMapping fdistribute5VM[] = {
   fdistribute5VMT<CcString>,
   fdistribute5VMT<FText>
};

/*
1.7.4 Specification

*/
OperatorSpec fdistribute5Spec(
     " stream(tuple(X)) x string,text} x int x attrName ",
     " _ ddistribute2[ _, _,_]",
     " Distributes a locally stored relation into a set of files. "
     "The file names are given by the second attribute extended by "
     "an index and file extension .bin." 
     " The integer argument determines how many files are  "
     "created. The attribute given by attrName terminates the file in "
     "that the tuple is written.",
     " query strassen feed addcounter[No,1] ifdistribute5['strassen',10,No]"
     "  count"
     );

/*
1.7.5 Operator Instance

*/
Operator fdistribute5Op(
   "fdistribute5",
   fdistribute5Spec.getStr(),
   2,
   fdistribute5VM,
   fdistribute5Select,
   fdistribute5TM
);




/*
1.7 Operator closeWorkers

This operator closes the worker connections either dor a spoecified
darray instance or all existing connections.

*/
ListExpr closeWorkersTM(ListExpr args){
  string err = " no argument or d[f]array expected";
  if(!nl->IsEmpty(args) && !nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  if(nl->HasLength(args,1)){
    if(   !DArray::checkType(nl->First(args))
       && !DFArray::checkType(nl->First(args))){
       return listutils::typeError(err);
    }
  }
  return listutils::basicSymbol<CcInt>();
}


template<class A>
int closeWorkersVMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){

    result = qp->ResultStorage(s);
    CcInt* res = (CcInt*) result.addr;
    if(qp->GetNoSons(s)==0){
       res->Set(true, algInstance->closeAllWorkers());  
    } else {
       A* arg = (A*) args[0].addr;
       if(!arg->IsDefined()){
         res->SetDefined(false);
         return 0;
       }
       int count = 0;
       for(size_t i=0;i<arg->numOfWorkers();i++){
          bool closed = algInstance->closeWorker(arg->getWorker(i));
          if(closed){
             count++;
          }
       }
       res->Set(true,count);
    }
    return 0;
}

ValueMapping closeWorkersVM[] = {
  closeWorkersVMT<DArray>,
  closeWorkersVMT<DFArray>
};

int closeWorkersSelect(ListExpr args){
  if(nl->IsEmpty(args)){
     return 0;
  }
  return DArray::checkType(nl->First(args))?0:1;
}


OperatorSpec closeWorkersSpec(
     " -> int, d[f]array -> int ",
     " closeWorkers(_)",
     " Closes either all connections to workers (no argument)"
     ", or connections of a specified darray instance.",
     " query closeWorkerConnections()  "
     );

Operator closeWorkersOp(
  "closeWorkers",
  closeWorkersSpec.getStr(),
  2,
  closeWorkersVM,
  closeWorkersSelect,
  closeWorkersTM
);


/*
1.8 Operator ~showWorkers~

This operator shows the information about existing worker connections.
If the optional argument is given, only open conections of this array
are shown, otherwise infos about all existing workers.

*/

ListExpr showWorkersTM(ListExpr args){
  string err = "nothing or darray or dfarray expected" ;
  if(!nl->IsEmpty(args) && !nl->HasLength(args,1)){
     return listutils::typeError(err);
  }
  if(nl->HasLength(args,1) && !DArray::checkType(nl->First(args))
    && !DFArray::checkType(nl->First(args))){
     return listutils::typeError(err);
  }
  ListExpr attrList = 
      nl->Cons(
         nl->TwoElemList( nl->SymbolAtom("Host"), 
                     listutils::basicSymbol<FText>()),
         nl->SixElemList(
             nl->TwoElemList( nl->SymbolAtom("Port"), 
                      listutils::basicSymbol<CcInt>()),
         nl->TwoElemList( nl->SymbolAtom("ConfigFile"), 
                     listutils::basicSymbol<FText>()),
         nl->TwoElemList( nl->SymbolAtom("Num"),
                     listutils::basicSymbol<CcInt>()),
         nl->TwoElemList( nl->SymbolAtom("DBName"),
                      listutils::basicSymbol<CcString>()),
         nl->TwoElemList( nl->SymbolAtom("PID"),
                     listutils::basicSymbol<CcInt>()),
         nl->TwoElemList( nl->SymbolAtom("OK"), 
                     listutils::basicSymbol<CcBool>()))
      );
    return nl->TwoElemList(
             listutils::basicSymbol<Stream<Tuple> >(),
             nl->TwoElemList(
                 listutils::basicSymbol<Tuple>(),
                  attrList));

}


template<class A>
class showWorkersInfo{

  public:
    showWorkersInfo(ListExpr resType) : array(0){
       iter = algInstance->workersIterator();
       tt = new TupleType(resType);
    }
    showWorkersInfo(A* _array, ListExpr resType): array(_array), pos(0){
       tt = new TupleType(resType);
    }
    ~showWorkersInfo(){
       tt->DeleteIfAllowed();
    }

    Tuple* next(){
      if(array){
        return nextFromArray();
      } else {
        return nextFromAll();
      }
    }


  private:
    A* array;
    size_t pos;
    typename map<DArrayElement, pair<string, ConnectionInfo*> >::iterator iter;
    TupleType* tt;

    Tuple* nextFromArray(){
      if(!array->IsDefined()){
         return 0;
      }
      while(pos < array->numOfWorkers()){
        DArrayElement elem = array->getWorker(pos);
        pos++;
        string dbname = algInstance->getDBName(elem);
        ConnectionInfo* connectionInfo;
        if(algInstance->workerConnection(elem, dbname, connectionInfo)){
           return createTuple(elem, dbname, connectionInfo);
        }
     }
     return 0;
    }

    Tuple* nextFromAll(){
      if(algInstance->isEnd(iter)){
         return 0;
      }
      Tuple* res = createTuple(iter->first, iter->second.first, 
                               iter->second.second);
      iter++;
      return res;
    }

    Tuple* createTuple(const DArrayElement& elem, string& dbname,
                        ConnectionInfo* ci){
      Tuple* res = new Tuple(tt);
      res->PutAttribute(0, new FText(true, elem.getHost()));
      res->PutAttribute(1, new CcInt(true, elem.getPort()));
      res->PutAttribute(2, new FText(true, elem.getConfig()));
      res->PutAttribute(3, new CcInt(true, elem.getNum()));
      res->PutAttribute(4, new CcString(true, dbname));
      res->PutAttribute(5, new CcInt(true, ci->serverPid()));
      bool ok = ci?ci->check():false;
      res->PutAttribute(6, new CcBool(true,ok));
      return res;
    }

};


template<class A>
int showWorkersVMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){

  showWorkersInfo<A>* li = (showWorkersInfo<A>*) local.addr;
  switch(message){
    case OPEN: {

                 if(li){
                    delete li;
                    local.addr =0;
                 }
                 ListExpr tt = nl->Second(GetTupleResultType(s));
                 if(qp->GetNoSons(s)==0){
                   local.addr = new showWorkersInfo<A>(tt);
                 } else {
                   local.addr = new showWorkersInfo<A>((A*) args[0].addr,tt);
                 }
                 return 0;
    }
    case REQUEST:
                 result.addr = li?li->next():0;
                 return result.addr?YIELD:CANCEL;
    case CLOSE:
           if(li){
             delete li;
             local.addr = 0;
           }
           return 0;
  }
  return -1; 
}

OperatorSpec showWorkersSpec(
     " -> stream(tuple), {darray,dfarray} -> stream(tuple) ",
     " showWorkers([_])",
     "This operator shows information about either all connections to workers "
     "(no argument),  "
     "or connections of a specified darray instance.",
     " query showWorkers()  consume "
     );

ValueMapping showWorkersVM[]={
  showWorkersVMT<DArray>,
  showWorkersVMT<DFArray>
};

int showWorkersSelect(ListExpr args){

  return nl->IsEmpty(args) || DArray::checkType(nl->First(args)) ?0:1;

}



Operator showWorkersOp(
  "showWorkers",
  showWorkersSpec.getStr(),
  2,
  showWorkersVM,
  showWorkersSelect,
  showWorkersTM
);


/*
1.9 Operator dloop

This operator performs a function over all entries of a darray.

1.9.1 Type Mpping

Signature: darray(X) x string x (X->Y) -> darray(Y)

*/

ListExpr dloopTM(ListExpr args){


  string err = "d[f]array(X) x string x fun: X -> Y  expected";
  if(!nl->HasLength(args,3) ){
    return listutils::typeError(err + "(wrong number of args)");
  }

  ListExpr temp = args;
  while(!nl->IsEmpty(temp)){
     if(!nl->HasLength(nl->First(temp),2)){
        return listutils::typeError("internal Error");
     }
     temp = nl->Rest(temp);
  }

  ListExpr darray = nl->First(args);
  ListExpr fun;

  if(!CcString::checkType(nl->First(nl->Second(args)))){
     return listutils::typeError("Second arg not of type string");
  }
  fun = nl->Third(args);

  ListExpr funType = nl->First(fun);
  ListExpr arrayType = nl->First(darray);

  if(!DArray::checkType(arrayType) && !DFArray::checkType(arrayType)){
    return listutils::typeError(err + ": first arg not a d[f]array");
  }
  if(!listutils::isMap<1>(funType)){
    return listutils::typeError(err + ": last arg is not a unary function");
  }

  if(DArray::checkType(arrayType)){
    if(!nl->Equal(nl->Second(arrayType), nl->Second(funType))){
      return listutils::typeError("type mismatch between darray and "
                                  "function arg");
     }
  } else {
     if(!Relation::checkType(nl->Second(arrayType))){
        return listutils::typeError("sub type of dfarray is not a relation");
     }
     if(!frel::checkType(nl->Second(funType))){
        return listutils::typeError("funarg is not an frel");
     }
         if(!nl->Equal(nl->Second(nl->Second(arrayType)), 
                    nl->Second(nl->Second(funType)))){
        return listutils::typeError("type mismatch between array "
                                    "type and function arg");
        
      }

  }

  ListExpr result = nl->TwoElemList(listutils::basicSymbol<DArray>(),
                                    nl->Third(funType));

  if(!DArray::checkType(result)){
    return listutils::typeError("Invalid function result");
  }

  ListExpr funquery = nl->Second(fun);
  
  ListExpr funargs = nl->Second(funquery);

  ListExpr dat = nl->Second(arrayType);

  if(DFArray::checkType(arrayType)){
      dat = nl->TwoElemList(
                   listutils::basicSymbol<frel>(),
                   nl->Second(dat));
  }


  ListExpr rfunargs = nl->TwoElemList(
                        nl->First(funargs),
                        dat);
  ListExpr rfun = nl->ThreeElemList(
                        nl->First(funquery),
                        rfunargs,
                        nl->Third(funquery));   


  ListExpr res =  nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               nl->OneElemList(nl->TextAtom(nl->ToString(rfun))),
               result);

  return res;   


}



class dloopInfo{

  public:
   dloopInfo(DArrayBase* _array, int _index, string& _resName,string& _fun,
             ListExpr srcType) : 
     index(_index), resName(_resName), fun(_fun), elem("",0,0,""){
     elem = _array->getWorkerForSlot(index);
     array = _array; 
     sourceType = srcType;     
     runner = boost::thread(&dloopInfo::run, this);
  }

  ~dloopInfo(){
     runner.join();
  }


  private:
    int index;
    string resName;
    string fun;
    DArrayElement elem;
    DArrayBase* array; 
    ListExpr sourceType;
    
    boost::thread runner;

    void run(){
       string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
       ConnectionInfo* ci = algInstance->getWorkerConnection(elem,dbname);
       if(!ci){
          sendMessage("Cannot find connection ");
          return;
       }
       string bn = resName + "_" + stringutils::int2str(index);
       int err;
       string strres;
       double runtime;
       string errMsg;
       // delete old array content
       string funarg;
       if(array->getType()==DARRAY){
         funarg = array->getObjectNameForSlot(index); 
       } else {
          string home = ci->getSecondoHome();
          ListExpr ft = nl->TwoElemList( listutils::basicSymbol<frel>(),
                                     nl->Second(nl->Second(sourceType)));
          string fn = array->getFilePath(home, dbname,  index);
          funarg ="(" + nl->ToString(ft) + " '"+fn+"')";
       }
       ci->simpleCommand("delete "+ bn, err,strres, false, runtime);
       // create new object by applying the function
       string cmd =   "(let "+bn+" = ("+fun+" " +  funarg +"))";
       ci->simpleCommandFromList(cmd, err,errMsg, strres, false, runtime);
       if(err!=0){ 
           sendMessage(" Problem in command " + cmd + ":" + errMsg);
       }
    }

};

template<class A>
int dloopVMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){

   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   A* array = (A*) args[0].addr;
   result = qp->ResultStorage(s);
   DArray* res = (DArray*) result.addr;
   
  if(!array->IsDefined()){
      res->makeUndefined();
      return 0;
   }  

   string n;
   FText* fun = 0;
   CcString* name = (CcString*) args[1].addr;
   if(!name->IsDefined() || (name->GetValue().length()==0)){
      algInstance->getWorkerConnection(
                          array->getWorker(0),dbname);
      n = algInstance->getTempName();
   } else {
      n = name->GetValue();
   }
   fun = (FText*) args[3].addr;

   if(!stringutils::isIdent(n)){
      res->makeUndefined();
      return 0;
   }
   (*res) = (*array);
   res->setName(n);

   vector<dloopInfo*> runners;
   string f = fun->GetValue();

   for(size_t i=0; i<array->getSize(); i++){
     dloopInfo* r = new dloopInfo(array,i,n,f,qp->GetType(qp->GetSon(s,0)));
     runners.push_back(r);
   }

   for(size_t i=0;i<runners.size();i++){
     delete runners[i];
   }
   return 0; 
}

ValueMapping dloopVM[] = {
   dloopVMT<DArray>,
   dloopVMT<DFArray>
};

int dloopSelect(ListExpr args){
   return DArray::checkType(nl->First(args))?0:1;
}



OperatorSpec dloopSpec(
     " darray(X) x string x  (X->Y) -> darray(Y)",
     " _ dloop[_,_]",
     "Performs a function on each element of a darray instance."
     "The string argument specifies the name of the result. If the name"
     " is undefined or an empty string, a name is generated automatically.",
     "query da2 dloop[\"da3\", . count"
     );

Operator dloopOp(
  "dloop",
  dloopSpec.getStr(),
  2,
  dloopVM,
  dloopSelect,
  dloopTM
);


ListExpr
DARRAYELEMTM( ListExpr args )
{

  if(!nl->HasMinLength(args,1)){
    return listutils::typeError("at least one argument required");
  }
  ListExpr first = nl->First(args);
  if(!DArray::checkType(first)){
    return listutils::typeError("darray expected");
  }
  return nl->Second(first);
}

OperatorSpec DARRAYELEMSpec(
     "darray(X) -> X ",
     "DARRAYELEM(_)",
     "Type Mapping Operator. Extract the type of a darray.",
     "query da2 dloop[\"da3\", . count"
     );

Operator DARRAYELEMOp (
      "DARRAYELEM",
      DARRAYELEMSpec.getStr(),
      0,   
      Operator::SimpleSelect,
      DARRAYELEMTM );


ListExpr
DARRAYELEM2TM( ListExpr args )
{
  if(!nl->HasMinLength(args,2)){
    return listutils::typeError("at least one argument required");
  }
  ListExpr second = nl->Second(args);
  if(!DArray::checkType(second)){
    return listutils::typeError("darray expected");
  }
  ListExpr res =  nl->Second(second);
  return res;
}

OperatorSpec DARRAYELEM2Spec(
     "T x darray(Y) x ... -> Y ",
     "DARRAYELEM2(_)",
     "Type Mapping Operator. Extract the type of a darray.",
     "query da2 da3 dloop[\"da3\", .. count"
     );

Operator DARRAYELEM2Op (
      "DARRAYELEM2",
      DARRAYELEM2Spec.getStr(),
      0,   
      Operator::SimpleSelect,
      DARRAYELEM2TM );

/*
1.8 Operator ~dsummarize~

This operator puts all parts of an darray instance into a single stream.
If relations are stored within the darray, each tuple is a stream element.
If attributes are stored, each element corresponds to a single token in 
the resulting stream. Other subtypes are not supported.

*/
ListExpr dsummarizeTM(ListExpr args){

  string err="d[f]array(X) with X in rel or DATA expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  ListExpr arg = nl->First(args);
  if(!DArray::checkType(arg) && !DFArray::checkType(arg)){
    return listutils::typeError(err);
  }
  ListExpr subtype = nl->Second(arg);
  if(!Relation::checkType(subtype) && !listutils::isDATA(subtype)){
    return listutils::typeError("subtype not supported");
  }
  ListExpr streamtype;
  if(Relation::checkType(subtype)){
     streamtype = nl->Second(subtype);
  } else {
     streamtype = subtype;
  }
  return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                         streamtype);
  
}

/*
1.8.2 LocalInfo for Attributes

*/
class successListener{
  public:
     virtual void jobDone(int id, bool success)=0;
};

class dsummarizeAttrInfoRunner{

  public:
     dsummarizeAttrInfoRunner(DArrayElement& _elem, const string& _objName,
                               ListExpr _resType,
                               const string& _db, int _index,
                               successListener* _listener): 
       elem(_elem), objName(_objName), resType(_resType),db(_db),
       index(_index), listener(_listener), 
       attribute(0){

     }

     void start(){
       runner = boost::thread(&dsummarizeAttrInfoRunner::run,this);
     }
      

     ~dsummarizeAttrInfoRunner(){
          runner.join();
          if(attribute){
              attribute->DeleteIfAllowed();
          }
      }

      Attribute* getAttribute(){
         Attribute* res = attribute;
         attribute = 0;
         return res;
      }

  private:
     DArrayElement elem;
     string objName;
     ListExpr resType;
     string db;
     int index;
     successListener* listener;
     Attribute* attribute;
     boost::thread runner;

     void run(){
         ConnectionInfo* ci = algInstance->getWorkerConnection(elem,db);
         if(!ci){
            listener->jobDone(index, false);
            return;
         } 
         Word result;
         bool ok = ci->retrieve(objName,resType, result,true);
         if(!ok){
           listener->jobDone(index,false);
         } else {
           attribute = (Attribute*) result.addr;
           listener->jobDone(index,true);
         }
     }
};

class dsummarizeAttrInfo : public successListener{

   public:
      dsummarizeAttrInfo(DArray* _array, ListExpr _resType) : 
        array(_array), resType(_resType),stopped(false), pos(0){
        runner = boost::thread(&dsummarizeAttrInfo::run, this);       
      }

      virtual ~dsummarizeAttrInfo(){
         stopped = true;
         runner.join();
         for(size_t i=0;i<runners.size();i++){
             if(runners[i].first){
                 delete runners[i].first;
             }
         }
      }

      Attribute* next(){
        while(pos < array->getSize()){
           boost::unique_lock<boost::mutex> lock(mtx);
           while(runners.size() <= pos || !runners[pos].second){
               cond.wait(lock);
           }
           Attribute* result = runners[pos].first->getAttribute();
           delete runners[pos].first;
           runners[pos].first = 0;
           pos++;
           if(result){
              return result;
           }
        }
        return 0;
      }

      void jobDone(int index, bool success){
         {
            boost::lock_guard<boost::mutex> lock(mtx);
            runners[index].second = true;
         }
         cond.notify_one();
      } 
   
   private:
      DArray* array;
      ListExpr resType;
      bool stopped;
      size_t pos;
      vector<pair<dsummarizeAttrInfoRunner*,bool> > runners;
      boost::thread runner;
      boost::condition_variable cond;
      boost::mutex mtx;
      
      void run(){
           string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
           string aname = array->getName();
           while(!stopped && runners.size() < array->getSize()){
              DArrayElement elem = array->getWorkerForSlot(runners.size());
              string objname = aname + "_" + 
                               stringutils::int2str(runners.size());
              dsummarizeAttrInfoRunner* r = new 
                      dsummarizeAttrInfoRunner( elem, 
                             objname, resType, dbname, runners.size(), this);
              runners.push_back(make_pair(r,false));
              r->start();
           }
      }
};


/*
1.8.3 LocalInfo for Relations

*/
class dsummarizeRelListener{
  public:
     virtual void connectionFailed(int id) = 0;
     virtual void fileAvailable(int id, const string& fname)=0;
};


template<class A>
class RelationFileGetter{
  public:
     RelationFileGetter(A* _array, int _index, 
                        dsummarizeRelListener* _listener):
      array(_array), index(_index), listener(_listener){
     }

     void operator()(){
       // get the connection
       dbname = SecondoSystem::GetInstance()->GetDatabaseName();
       DArrayElement elem = array->getWorkerForSlot(index);
       ConnectionInfo* ci = algInstance->getWorkerConnection(elem,dbname);
       if(!ci){ // connection failed
          listener->connectionFailed(index);
          return;
       }
       switch(array->getType()){
         case DARRAY : retrieveFileFromObject(ci); break;
         case DFARRAY : retrieveFileFromFile(ci); break;
         default: assert(false);
       }
     }

     

   private:
     A* array;
     int      index;
     dsummarizeRelListener* listener;
     string dbname;


     void retrieveFileFromObject( ConnectionInfo* ci){
       string objName = array->getName()+"_"+stringutils::int2str(index);
       string fname = objName+".bin";
       if(!ci->retrieveRelationFile(objName,fname)){
          listener->connectionFailed(index);
          return;
       }
       listener->fileAvailable(index, fname);
     }

     void retrieveFileFromFile(ConnectionInfo* ci){
       string rname = array->getName()+"_"+stringutils::int2str(index)+".bin";
       string path = ci->getSecondoHome() + "/dfarrays/" + dbname + "/"
                     + array->getName() + "/" + rname;

       string lname = rname;
       if(!ci->retrieveAnyFile(path,lname)){
          listener->connectionFailed(index);
          return;
       }
       listener->fileAvailable(index, lname);
        
     }


};





template<class A>
class dsummarizeRelInfo: public dsummarizeRelListener{

  public:
     dsummarizeRelInfo(A* _array, ListExpr _resType):
         array(_array), currentIndex(0), currentFeeder(0), resType(_resType)
    {
        start();
     }

     Tuple* next() { 
        while(true){
           if(currentFeeder){
              Tuple* res = currentFeeder->next();
              if(res){
                 return res;
              } else {
                 delete currentFeeder;
                 currentFeeder = 0;
                 currentIndex ++;
              }
           }
           if(currentIndex >= array->getSize()){
             return 0;
           }  
           boost::unique_lock<boost::mutex> lock(mtx);
           while(getters[currentIndex]){
               cond.wait(lock);
           }
           if(filenames[currentIndex].length()>0){
              ListExpr tType = 
                  SecondoSystem::GetCatalog()->NumericType(resType);
              currentFeeder = new ffeed5Info(filenames[currentIndex],
                     tType);
           } else { // retrieving file failed
              currentIndex++;
           }
         }
     }
 
     virtual ~dsummarizeRelInfo(){
        for(size_t i=0;i<runners.size();i++){
           if(runners[i]){
              runners[i]->join();
              delete runners[i];
           }
           if(getters[i]){
              delete getters[i];
           }
        }
        if(currentFeeder){
           delete currentFeeder;
        }

     } 

     void connectionFailed(int index){
       { boost::lock_guard<boost::mutex> guard(mtx);
        delete getters[index];
        getters[index] = 0;
       }
       cond.notify_one();
     }

     void fileAvailable(int index, const string& filename){
       { boost::lock_guard<boost::mutex> guard(mtx);
         filenames[index] = filename;
         delete getters[index];
         getters[index] = 0;
        }
        cond.notify_one();
     }




  private:
     A* array;
     size_t currentIndex; 
     ffeed5Info* currentFeeder;
     ListExpr resType;
     vector<boost::thread*> runners;
     vector<RelationFileGetter<A>*> getters;
     vector<string> filenames;
     boost::mutex mtx;
     boost::condition_variable cond;


     void start(){

        for(size_t i=0;i< array->getSize();i++){
           RelationFileGetter<A>* getter = 
                          new RelationFileGetter<A>(array,i,this);
           getters.push_back(getter);
           filenames.push_back("");
           runners.push_back(new boost::thread(*getter));
        }
     }
};


template<class T, class A>
int dsummarizeVMT(Word* args, Word& result, int message,
           Word& local, Supplier s ){

   T* li = (T*) local.addr;
   switch(message){
     case OPEN: if(li) delete li;
                local.addr = new T((A*) args[0].addr, 
                                    nl->Second(qp->GetType(s)));
                return 0;
     case REQUEST:
                 result.addr = li?li->next():0;
                 return result.addr?YIELD:CANCEL;
     case CLOSE:
            if(li){
              delete li;
              local.addr = 0;
            }
   }
   return -1;
}

/*
1.8.4 Selection and Value Mapping

*/

ValueMapping dsummarizeVM[] = {
    dsummarizeVMT<dsummarizeRelInfo<DArray>, DArray >,
    dsummarizeVMT<dsummarizeRelInfo<DFArray>, DFArray >,
    dsummarizeVMT<dsummarizeAttrInfo, DArray>
};


int dsummarizeSelect(ListExpr args){
  int res = -1;
  if(!Relation::checkType(nl->Second(nl->First(args)))){
     res =  2;
  } else {
     res = DArray::checkType(nl->First(args))?0:1;
  }
  return res;
}

/*
1.8.5 Specification

*/

OperatorSpec dsummarizeSpec(
     "darray(DATA) -> stream(DATA) , d[f]array(rel(X)) -> stream(X)",
     "_ dsummarize",
     "Produces a stream of the darray elements.",
     "query da2 dsummarize count"
     );

/*
1.8.6 Operator instance

*/


Operator dsummarizeOp(
  "dsummarize",
  dsummarizeSpec.getStr(),
  3,
  dsummarizeVM,
  dsummarizeSelect,
  dsummarizeTM
);


/*
1.10 Operator ~getValue~

The getValue operator converts a distributed array into a 
normal array from the ArrayAlgebra.

*/

ListExpr getValueTM(ListExpr args){
  string err ="darray or dfarray expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr a1 = nl->First(args);
  if(!DArray::checkType(a1) && !DFArray::checkType(a1)){
    return listutils::typeError(err);
  }
  return nl->TwoElemList( 
              listutils::basicSymbol<arrayalgebra::Array>(),
              nl->Second(a1));

}

class getValueListener{
  public:
    virtual void  jobDone(int index, Word result)=0;

};

/*
1.10.2 Class retrieving a single slot from a darray

*/
class getValueGetter{
  public:
    getValueGetter(DArray* _array, int _index, getValueListener* _listener, 
      ListExpr _resType):
       array(_array), index(_index), resType(_resType),listener(_listener){}
       

   void operator()(){

      string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
      ConnectionInfo* ci = algInstance->getWorkerConnection(
                              array->getWorkerForSlot(index),dbname);
      Word res((void*)0);
      if(!ci){
          cerr << "workerconnection not found";
          listener->jobDone(index,res); 
          return;
      }
      string name = array->getName()+"_"+stringutils::int2str(index);
      ci->retrieve(name, resType, res,true);
      listener->jobDone(index,res); 
   }

  private:
    DArray* array;
    int index;
    ListExpr resType;
    getValueListener* listener;

};


/*
1.10.3 Class retrieving a single slot from a dfarray

*/
class getValueFGetter{
  public:
    getValueFGetter(DFArray* _array, int _index, getValueListener* _listener, 
      ListExpr _resType):
       array(_array), index(_index), resType(_resType),listener(_listener){}
       

   void operator()(){

      string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
      ConnectionInfo* ci = algInstance->getWorkerConnection(
                              array->getWorkerForSlot(index),dbname);
      Word res((void*)0);
      if(!ci){
          cerr << "workerconnection not found";
          listener->jobDone(index,res); 
          return;
      }
      string name = array->getName()+"_"+stringutils::int2str(index);
      string home = ci->getSecondoHome();
      string fname = home +"/dfarrays/"+dbname+"/"+array->getName()
                     + "/" + name + ".bin"; 
      ci->retrieveRelationInFile(fname, resType, res);
      listener->jobDone(index,res); 
   }

  private:
    DFArray* array;
    int index;
    ListExpr resType;
    getValueListener* listener;

};



template<class AType, class GType>
class getValueInfo : public getValueListener{

  public:
   getValueInfo(AType* _arg, arrayalgebra::Array* _result, ListExpr _resType):
    arg(_arg), result(_result), resType(_resType), algId(0),
    typeId(0),n(-1),values(0){
      isData = listutils::isDATA(nl->Second(_resType));
   }

   virtual ~getValueInfo(){
      delete[] values;
   }


   void convert(){
     if(!init()){
        return;
     }
    vector<GType*> getters;
    vector<boost::thread*> threads;
    for(int i=0;i<n;i++){
      GType* getter = new GType(arg,i,this, nl->Second(resType));
      getters.push_back(getter);
      boost::thread* t = new boost::thread(*getter); 
      threads.push_back(t);
    }
    // for for finishing retrieving elements
    for(size_t i =0 ;i<threads.size();i++){
        threads[i]->join();
    }
    for(size_t i =0 ;i<threads.size();i++){
        delete threads[i];
    }
    for(size_t i =0 ;i<getters.size();i++){
       delete getters[i];  
    }
    result->initialize(algId,typeId,n,values);
  } 

  void jobDone(int id, Word value){
    if(value.addr==0){ // problem in getting element
       // set some default value 
       boost::lock_guard<boost::mutex> guard(createRelMut);
       ListExpr numresType = SecondoSystem::GetCatalog()->NumericType(resType);
       value = am->CreateObj(algId,typeId)(nl->Second(numresType));
       if(isData){
          ((Attribute*)value.addr)->SetDefined(false);
       }
    }

    values[id] = value;

  }

  private:
    AType* arg;
    arrayalgebra::Array* result;
    ListExpr resType;
    int algId;
    int typeId;
    int n;
    Word* values;
    bool isData;
    
    bool init(){
       if(!arg->IsDefined()){
         return false;
       }
       string typeName;
       if(!SecondoSystem::GetCatalog()->LookUpTypeExpr(nl->Second(resType), 
                                                 typeName, algId, typeId)){
          cerr << "internal error, could not determine algid and typeId"
               << " for " << nl->ToString(nl->Second( resType)) << endl;
          return false;
       }
       n = arg->getSize();
       Word std((void*)0);
       values = new Word[n];
       for(int i=0;i<n;i++){
         values[i] = std;
       }
       return true;
    }
};


template<class AType, class GType>
int getValueVMT(Word* args, Word& result, int message,
            Word& local, Supplier s ){

   result = qp->ResultStorage(s);
   AType* array = (AType*) args[0].addr;
   arrayalgebra::Array* res = (arrayalgebra::Array*) result.addr;
   if(array->IsDefined()){
      getValueInfo<AType,GType> info(array,res, qp->GetType(s)); 
      info.convert();
   } else {
      res->setUndefined();
   }
   return 0;
}


int getValueSelect(ListExpr args){
  return DArray::checkType(nl->First(args))?0:1;
}

ValueMapping getValueVM[] = {
  getValueVMT<DArray,getValueGetter>,
  getValueVMT<DFArray,getValueFGetter>
};

OperatorSpec getValueSpec(
     "{darray(T),dfarray(T)} -> array(T)",
     "getValue(_)",
     "Converts a distributed array into a normal one.",
     "query getValue(da2)"
     );


Operator getValueOp(
  "getValue",
  getValueSpec.getStr(),
  2,
  getValueVM,
  getValueSelect,
  getValueTM
);


/*
1.10 OPerator ~dloop2~

This operator performs a function on the elements of two darray values.
The workerlist of both darray instances must be the same. The smaller
darray instance determines the size of the result. Because the workerlist
must be equal, the single elements of the arrays are on the same worker
and no data must be tramsferred.

1.10.1 Type Mapping

darray(X) x darray(Y) x string x (fun: X x Y [->] Z) [->] darray(Z)

*/
ListExpr dloop2TM(ListExpr args){
   string err ="darray(X) x darray(Y) x string x (X x Y ->Z) expected";
   if(!nl->HasLength(args,4)){
      return listutils::typeError(err);
   } 
   // check wether each element consists of two elements
   // uses args in type mapping
   ListExpr args2 = args;
   while(!nl->IsEmpty(args2)){
      if(!nl->HasLength(nl->First(args2),2)){
        return listutils::typeError("internal error");
      }
      args2 = nl->Rest(args2);
   }
   
   if(    !DArray::checkType(nl->First(nl->First(args))) 
       || !DArray::checkType(nl->First(nl->Second(args)))
       || !CcString::checkType(nl->First(nl->Third(args)))
       || !listutils::isMap<2>(nl->First(nl->Fourth(args)))){
      return listutils::typeError(err);    
   }

   ListExpr fa1 = nl->Second(nl->First(nl->First(args)));
   ListExpr fa2 = nl->Second(nl->First(nl->Second(args)));
   ListExpr a1 = nl->Second(nl->First(nl->Fourth(args)));
   ListExpr a2 = nl->Third(nl->First(nl->Fourth(args)));
   if(   !nl->Equal(fa1,a1)
      || !nl->Equal(fa2,a2)){
     return listutils::typeError("function arguments does not fit "
                                 "to the darray subtypes");
   }

   ListExpr funRes = nl->Fourth(nl->First(nl->Fourth(args)));
   ListExpr resType = nl->TwoElemList(
                        listutils::basicSymbol<DArray>(),
                        funRes);
   ListExpr funQuery = nl->Second(nl->Fourth(args));

 
   // replace eventually type mapping operators by the
   // resulting types

   ListExpr fa1o = nl->Second(funQuery);
   fa1 = nl->TwoElemList(
                       nl->First(fa1o),
                       nl->Second(nl->First(nl->First(args))));
   ListExpr fa2o = nl->Third(funQuery);
   fa2 = nl->TwoElemList(
                       nl->First(fa2o),
                       nl->Second(nl->First(nl->Second(args))));
   funQuery = nl->FourElemList(
                   nl->First(funQuery),
                   fa1, fa2, nl->Fourth(funQuery));
 

   string funstr = nl->ToString(funQuery);
 
   return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                            nl->OneElemList(
                               nl->TextAtom(funstr)),
                            resType);
      
}

class dloop2Runner{
  public:

     dloop2Runner(DArray* _a1, DArray* _a2, int _index, 
                   const string& _fun, const string& _rname):
       a1(_a1), a2(_a2), index(_index),fun(_fun), rname(_rname){
     }

     void operator()(){
      string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
      ConnectionInfo* ci = algInstance->getWorkerConnection(
                              a1->getWorkerForSlot(index),dbname);
      if(!ci){
         sendMessage("could not open connection to "
                     +  a1->getWorkerForSlot(index).getHost()
                     + "@" 
                     + stringutils::int2str(
                            a1->getWorkerForSlot(index).getPort()));
         return;
      }
      int err;
      string errMsg;
      string strres;
      double runtime;
      string tname = rname + "_" + stringutils::int2str(index);
      // remove old stuff
      ci->simpleCommand("delete " + tname , err,errMsg, strres, false, runtime);
      // create new one
      string a1o = a1->getName()+"_"+stringutils::int2str(index);
      string a2o = a2->getName()+"_"+stringutils::int2str(index);
      string cmd ="(let " + tname + " = ( " + fun + " " + a1o + " " + a2o +"))";
      ci->simpleCommandFromList(cmd, err,errMsg, strres, false, runtime);
      if(err!=0){
         cerr << "error in creating result via " << cmd << endl;
         cerr << errMsg << endl;
         return;
      }
     }

  private:
      DArray* a1;
      DArray* a2;
      int index;
      string fun;
      string rname;
};



int dloop2VM(Word* args, Word& result, int message,
            Word& local, Supplier s ){

  DArray* a1 = (DArray*) args[0].addr;
  DArray* a2 = (DArray*) args[1].addr;
  result = qp->ResultStorage(s);
  DArray* res = (DArray*) result.addr;
  CcString* name = (CcString*) args[2].addr;
  FText* funQuery = (FText*) args[4].addr;
  string dbname = SecondoSystem::GetInstance()->GetDatabaseName();

  if(!a1->IsDefined() || !a2->IsDefined() ){
      res->makeUndefined();
      return 0;
  }
  string n;
  if(!name->IsDefined() || (name->GetValue().length()==0)){
      algInstance->getWorkerConnection(
                          a1->getWorker(0),dbname);
      n = algInstance->getTempName();
  } else {
      n = name->GetValue();
  }
  if(!stringutils::isIdent(n)){
      res->makeUndefined();
      return 0;
  }


  // check whether workers of both arrays are the same
  if(a1->numOfWorkers()!=a2->numOfWorkers()){
    sendMessage("dloop2 : Different workers ");
    return 0;
  } 

  vector<DArrayElement> workers;

  for(size_t i=0;i<a1->numOfWorkers();i++){
    if(a1->getWorker(i) != a2->getWorker(i)){
        sendMessage("dloop2: Different workers");
        res->makeUndefined();
        return 0;
    }
    workers.push_back(a1->getWorker(i));
  } 

  int max = min(a1->getSize(), a2->getSize());

  if(!a1->equalMapping(*a2,true)){
     sendMessage("dloop2: Different mappings");
     res->makeUndefined();
     return 0;
  }

  res->set(max,n,workers);

  vector<dloop2Runner*> runners;
  vector<boost::thread*> threads;
  string funstr = funQuery->GetValue();


  for(int i=0;i<max;i++){
      dloop2Runner* runner = new dloop2Runner(a1,a2,i,funstr, n);
      runners.push_back(runner);
      boost::thread* thread = new boost::thread(*runner);
      threads.push_back(thread);
  }

  for(size_t i=0;i<threads.size(); i++){
     threads[i]->join();
     delete threads[i];
     delete runners[i];
  }

  return 0;
}

/*
11.3 Specification

*/
OperatorSpec dloop2Spec(
     "darray(X) x darray(Y) x string x (fun : X x Y -> Z) -> darray(Z)",
     "_ _ dloop2[_,_]",
     "Performs a function on the elements of two darray instances. The "
     "string argument specifies the name of the resulting darray." 
     " If the string is undefined or empty, a name is generated "
     "automatically.",
     "query da1 da2 dloop2[\"newName\", fun(i1 : int, i2 : int) i1 + i2) "
     );
/*
11.4 Operator instance

*/


Operator dloop2Op(
  "dloop2",
  dloop2Spec.getStr(),
  dloop2VM,
  Operator::SimpleSelect,
  dloop2TM
);





/*
1.11 Operator fdistribute6

1.11.1 TRype Mapping

 stream(tuple) x {string,text} x int -> stream(tuple)


*/

ListExpr fdistribute6TM(ListExpr args){
  string err = "stream(tuple) x {string,text} x int expected";
  if(!nl->HasLength(args,3)){
    return listutils::typeError(err);
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(!CcString::checkType(nl->Second(args)) &&
     !FText::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->Third(args))){
    return listutils::typeError(err);
  }
  return nl->First(args);
}

/*
1.11.2 LocalInfo


*/
template<class T>
class fdistribute6Info{
  public:
    fdistribute6Info(Word _stream, T* _fname, CcInt* _max, 
                     ListExpr _resultType): 
      stream(_stream){
       counter = 0;
       fileCounter = 0;
       out = 0;
       buffer = 0;
       if(!_fname->IsDefined() || !_max->IsDefined()){
            c = 0;
            return;
       }
       bname = ( _fname->GetValue());
       stringutils::trim(bname);
       if(bname.length()==0){
          c=0;
          return;
       } 
       c = _max->GetValue();
       if(c<0){
          c = 0;
       }
       relType = nl->TwoElemList(listutils::basicSymbol<Relation>(),
                                 nl->Second(_resultType));
       stream.open();
    }
    
    ~fdistribute6Info(){
      finishCurrentFile();
      stream.close();
    }


    Tuple* next(){
      Tuple* in = stream.request();
      if(c==0){
         return in;
      }
      if(!c){
        finishCurrentFile();
      } else {
        storeTuple(in);
      }
      return in;
    }



  private:
    Stream<Tuple> stream;
    string bname;
    int c;
    ListExpr relType;
    int counter;
    int fileCounter;
    ofstream* out;
    char* buffer;


    void finishCurrentFile(){
       if(out){
          BinRelWriter::finish(*out);
          out->close();
          delete out;
          delete[] buffer;
          out=0;
          buffer = 0;
       }
    }

    void storeTuple(Tuple* tuple){
       if(!tuple){
          return;
       }
       if(!out){
          createNextOutputFile();
       }
       BinRelWriter::writeNextTuple(*out,tuple);
       counter++;
       if(counter==c){
         finishCurrentFile();
         counter = 0;
       }
    } 

    void createNextOutputFile(){
      string fname = bname +"_"+stringutils::int2str(fileCounter) + ".bin";
      fileCounter++;
      out = new ofstream(fname.c_str(), ios::binary | ios::out);
      buffer = new char[FILE_BUFFER_SIZE];
      out->rdbuf()->pubsetbuf(buffer, FILE_BUFFER_SIZE);
      BinRelWriter::writeHeader(*out,relType);
    }
};


template<class T>
int fdistribute6VMT(Word* args, Word& result, int message,
            Word& local, Supplier s ){

  fdistribute6Info<T>* li = (fdistribute6Info<T>*) local.addr;

  switch(message){
     case OPEN:
            if(li) delete li;
            local.addr = new fdistribute6Info<T>(args[0], (T*) args[1].addr, 
                                             (CcInt*) args[2].addr, 
                                              qp->GetType(s));
            return 0;
     case REQUEST:
            result.addr = li?li->next():0;
            return result.addr?YIELD:CANCEL;
     case CLOSE:
           if(li){
              delete li;
              local.addr = 0;
           }
           return 0;
  }
  return -1; 
}

/*
11.3 Value Mapping and Selection

*/

int fdistribute6Select(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}

ValueMapping fdistribute6VM[] = {
   fdistribute6VMT<CcString>,
   fdistribute6VMT<FText>
};

/*
11.4 Specification

*/

OperatorSpec fdistribute6Spec(
     " stream(tuple> x {string,text} x int -> stream(tuple)",
     "_ fdistribute6[<filename>, <numElemsPerFile>]",
     "Distributes a tuple stream into a set of files. " 
     " The first <numOfElemsPerFile> element of the stream "
     " are stored into the first file and so on. "
     "The given basic filename is extended by an underscore, "
     " a running number, and the file extension .bin." ,
     "query strassen feed fdistribute6['strassen',1000] count"
     );

/*
11.5 Operator instance

*/
Operator fdistribute6Op(
  "fdistribute6",
  fdistribute6Spec.getStr(),
  2,
  fdistribute6VM,
  fdistribute6Select,
  fdistribute6TM
);



/*
12 Operator ~fdistribute7~

This operator distributes a tuple stream according to a function from
tuple to int and a maximum size.

*/
ListExpr fdistribute7TM(ListExpr args){

  string err = "stream(tuple) x {string,text} x  (tuple->int) x int  "
               "x bool expected";
  if(!nl->HasLength(args,5)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(    !Stream<Tuple>::checkType(nl->First(args) )
      || (    !CcString::checkType(nl->Second(args))
           && !FText::checkType(nl->Second(args)))
      || !listutils::isMap<1>(nl->Third(args)) 
      || !CcInt::checkType(nl->Fourth(args))
      || !CcBool::checkType(nl->Fifth(args))){
    return listutils::typeError(err);
  }

  if(!nl->Equal(nl->Second(nl->First(args)),
                nl->Second(nl->Third(args)))){
    return listutils::typeError("type mismatch between tuple type "
                                "and function arg");
  }
  if(!CcInt::checkType(nl->Third(nl->Third(args)))) {
    return listutils::typeError("function does not results in an integer");
  }
  return nl->First(args);
}

template<class T>
class fdistribute7Info{
  public:
  fdistribute7Info(Word _stream, Supplier _fun, CcInt* _max, T* _fn,
                   CcBool* _createEmpty,ListExpr _relType):
       stream(_stream), fun(_fun){

     bufsize = 1014*1024*64; // 64 MB

     if(!_max->IsDefined() || !_fn->IsDefined()){
         max = -1;
         fn = "";
     } else {
        max = _max->GetValue();
        fn = _fn->GetValue();
     }
     stream.open();
     relType = _relType;
     if(   _createEmpty->IsDefined()  
        && _createEmpty->GetValue() 
        && (max > 0)){
       for(int i=0;i<max;i++){
          getStream(i);
       }
     }
     funArgs = qp->Argument(fun);


  }


  ~fdistribute7Info(){
      typename map<int,pair<ofstream*,char*>* >::iterator it;
      for(it = files.begin();it!=files.end();it++){
         if(it->second){
            BinRelWriter::finish(*(it->second->first));
            it->second->first->close();
            delete it->second->first;
            delete[] it->second->second;
            delete it->second;
         }
      }
      stream.close();
   }

   Tuple* next(){
     Tuple* t = stream.request();
     if(!t || max <=0){
        return t;
     }
     (*funArgs)[0] = t;
     Word r;
     qp->Request(fun,r);
     CcInt* res = (CcInt*) r.addr;
     int resi = res->IsDefined()?res->GetValue():0;
     pair<ofstream*,char*>*  outp = getStream(resi % max);
     if(outp){
        BinRelWriter::writeNextTuple(*(outp->first),t);
     }
     return t;
   }



  private:
     Stream<Tuple> stream;
     Supplier fun;
     ArgVectorPointer funArgs;
     string fn;
     int max;
     map<int,pair<ofstream*, char*>*> files;
     ListExpr relType;
     size_t bufsize;


    pair<ofstream*,char*>* getStream(int i){
       typename map<int,pair<ofstream*,char*>*>::iterator it;
       it=files.find(i);
       if(it!=files.end()){
          return it->second;
       }
       string name = fn +"_" + stringutils::int2str(i)+".bin";
       ofstream* out = new ofstream(name.c_str(),ios::binary|ios::trunc);
       char* buffer = 0;
       if(!out->good()){
          delete out;
          out=0;
       } else {
           buffer = new char[bufsize];
           out->rdbuf()->pubsetbuf(buffer, bufsize); 
           BinRelWriter::writeHeader(*out,relType);
       }
       pair<ofstream*, char*>* res = out
                                     ?new pair<ofstream*,char*>(out,buffer)
                                     :0;
       files[i] = res;
       return res;
    }
};


template<class T>
int fdistribute7VMT(Word* args, Word& result, int message,
            Word& local, Supplier s ){

  fdistribute7Info<T>* li = (fdistribute7Info<T>*) local.addr;
  switch(message){
    case OPEN :{
         if(li){
           delete li;
         }
         local.addr = new fdistribute7Info<T>(
                            args[0],
                            qp->GetSon(s,2),
                            (CcInt*) args[3].addr,
                            (T*) args[1].addr,
                            (CcBool*) args[4].addr,
                            nl->TwoElemList(
                               listutils::basicSymbol<Relation>(),
                               nl->Second(qp->GetType(s))));
    }
    case REQUEST:
       result.addr = li?li->next():0;
       return result.addr?YIELD:CANCEL;
    case CLOSE:
        if(li){
          delete li;
          local.addr = 0;
        }
        return 0;
  }
  return -1;
}



ValueMapping fdistribute7VM[] = {
   fdistribute7VMT<CcString>,
   fdistribute7VMT<FText>
};

int fdistribute7Select(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}



OperatorSpec fdistribute7Spec(
     " stream(tuple> x {string,text} x (tuple->int) x int x "
     "bool -> stream(tuple)",
     "_ fdistribute7[filename, fun, maxfiles,  createEmpty]",
     "Distributes a tuple stream into a set of files. " 
     "The tuples are distributed according a function given "
     "by the user. The used slot for a tuple is the result of this "
     "function modulo maxfiles. The tuples are written in files "
     "with name prefixed by the user given name followed by an "
     "underscore and the slotnumber. "
     "If the boolean argument is set to TRUE, also empty relations are "
     "stored into files, otherwise empty relations are omitted.",
     "query strassen feed fdistribute7['strassen',hashvalue(.Name), 12, "
     "TRUE] count"
 );


Operator fdistribute7Op(
   "fdistribute7",
   fdistribute7Spec.getStr(),
   2,
   fdistribute7VM,
   fdistribute7Select,
   fdistribute7TM
);





/*
12 Operator ~deleteRemoteObjects~

12.1 Type Mapping

This operator gets a darray instance and optionally an integer 
value. If the integer value is given, only the object for the 
specified index is deleted otherwise all objects handled by this
darray object. The return value is the number of successfully
deleted objects. A distributed matrix can be removed only 
completely.

*/
ListExpr deleteRemoteObjectsTM(ListExpr args){
  string err = " {darray,dfarray} [x int] or dfmatrix expected";
  if(!nl->HasLength(args,1) && !nl->HasLength(args,2)){
    return listutils::typeError(err + ": invalid number of args" );
  }
  
  if(nl->HasLength(args,1) && DFMatrix::checkType(nl->First(args))){
     return listutils::basicSymbol<CcInt>();
  } 


  if(!DArray::checkType(nl->First(args))
     && !DFArray::checkType(nl->First(args))){
    return listutils::typeError(err + ": first arg not a darray "
                                "or a dfarray");
  }
  if(nl->HasLength(args,2)){
    if(!CcInt::checkType(nl->Second(args))){
       return listutils::typeError(err+  ": second arg is not an int");
    }
  }
  return listutils::basicSymbol<CcInt>();
}


class Object_Del{

   public:

      Object_Del(DArray* _array, int _index):
       array(_array), farray(0), index(_index), del(0){
      }
      
      Object_Del(DFArray* _array, int _index):
       array(0), farray(_array), index(_index), del(0){
      }

      Object_Del(Object_Del& src): array(src.array),farray(src.farray),
         index(src.index), del(src.del){}
      

      void operator()(){
         if(array){
           deleteArray();
         } else {
           deleteFArray();
         }
      }


      int getNumber(){
        return del;
      }

   private:
      DArray* array;
      DFArray* farray;
      int index;
      int del;

    void deleteArray(){
        string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
        ConnectionInfo* ci = algInstance->getWorkerConnection(
                              array->getWorkerForSlot(index),dbname);
        if(!ci){
           sendMessage("could not open connection to "
                     +  array->getWorkerForSlot(index).getHost()
                     + "@" 
                     + stringutils::int2str(
                             array->getWorkerForSlot(index).getPort()));
           return;
        }
        string objName = array->getName() +"_" + stringutils::int2str(index);
        int err;
        string errMsg;
        string resstr;
        double runtime;
        ci->simpleCommand("delete " + objName, err, errMsg, resstr, 
                          false, runtime);
        if(err==0){
          del = 1;
        } 
     }

    void deleteFArray(){
        string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
        ConnectionInfo* ci = algInstance->getWorkerConnection(
                              farray->getWorkerForSlot(index),dbname);
        if(!ci){
           sendMessage("could not open connection to "
                     +  farray->getWorkerForSlot(index).getHost()
                     + "@" + stringutils::int2str(
                               farray->getWorkerForSlot(index).getPort()));
           return;
        }
         
        string fileName = ci->getSecondoHome()+ "/dfarrays/"+dbname+"/"
                  + farray->getName() + "/"
                  + farray->getName() +"_" + stringutils::int2str(index)
                  + ".bin";
        int err;
        string errMsg;
        string resstr;
        double runtime;
        ci->simpleCommand("query removeFile('"+fileName+"')", err, 
                          errMsg, resstr, false, runtime);
        if(err==0){
          del = 1;
        } 
    }

};



template<class A>
int deleteRemoteObjectsVMT(Word* args, Word& result, int message,
            Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;
  A* array = (A*) args[0].addr;
  if(!array->IsDefined()){
     res->Set(true,0);
     return 0;
  }
  bool all = true;
  int index = 0;
  if(qp->GetNoSons(s)==2){
     all = false;
     CcInt* i = (CcInt*) args[1].addr;
     if(!i->IsDefined()){
       res->Set(true,0);
       return 0;
     }
     index = i->GetValue();
     if(index < 0 || index >= (int) array->getSize()){
        res->Set(true,0);
        return 0;
     }
  }

  vector<Object_Del*> deleters;
  vector<boost::thread*> threads;
  int count = 0;
  if(!all){
    Object_Del* del = new Object_Del(array,index);
    deleters.push_back(del);
    boost::thread* thread = new boost::thread(&Object_Del::operator(),del);
    threads.push_back(thread);
  } else {
    for(size_t i=0;i<array->getSize();i++){
       Object_Del* del = new Object_Del(array,i);
       deleters.push_back(del);
       boost::thread* thread = new boost::thread(&Object_Del::operator(),del);
       threads.push_back(thread);
    }
  }

  for(size_t i=0;i<threads.size();i++){
     threads[i]->join();
     delete threads[i];
     count += deleters[i]->getNumber();
     delete deleters[i];
  }
  res->Set(true,count);
  return 0;
}


class MatrixKiller{
  public:

    MatrixKiller(ConnectionInfo* _ci, const string& _dbname, 
                 const string& _mname):
       ci(_ci), dbname(_dbname), mname(_mname){
      runner = new boost::thread(&MatrixKiller::run,this);
    }

    ~MatrixKiller(){
       runner->join();
       delete runner;
    }

    private:
       ConnectionInfo* ci;
       string dbname;
       string mname;
       boost::thread* runner;

       void run(){
          string dir = ci->getSecondoHome()+"/dfarrays/"+dbname+"/"+mname;
          string cmd = "query removeDirectory('"+dir+"', TRUE)";
          int err;
          string errMsg;
          double runtime;
          string res;
          ci->simpleCommand(cmd, err, errMsg, res, false, runtime);
          if(err){
            cerr << "command failed: " << cmd << endl;
            cerr << errMsg << endl;
          }
       }
};


int deleteRemoteObjectsVM_Matrix(Word* args, Word& result, int message,
            Word& local, Supplier s ){

   result = qp->ResultStorage(s);
   CcInt* res = (CcInt*) result.addr;
   DFMatrix* matrix = (DFMatrix*) args[0].addr;
   set<string> usedHosts;
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   vector<MatrixKiller*> killers;

   for(size_t i=0;i<matrix->numOfWorkers();i++){
     ConnectionInfo* ci = 
            algInstance->getWorkerConnection(matrix->getWorker(i),dbname);
     string host = ci->getHost();
     if(usedHosts.find(host)==usedHosts.end()){
         usedHosts.insert(host);
         MatrixKiller* killer = new MatrixKiller(ci,dbname, matrix->getName());
         killers.push_back(killer);        
     }
   }
   for(size_t i=0;i<killers.size();i++){
      delete killers[i];
   }
   res->Set(true,matrix->getSize());
   return 0;
}



OperatorSpec deleteRemoteObjectsSpec(
     " {darray, dfarray} [x int] | dfmatrix -> int",
     "deleteRemoteObjects(_,_)",
     "Deletes the remote objects managed by a darray  or a dfarray object. "
     "If the optionally integer argument is given, only the "
     "object at the specified index is deleted. For a dfmatrix object, "
     " only the deletion of all slots is possible.",
     "query deleteRemoteObjects(da2)"
 );

ValueMapping deleteRemoteObjectsVM[] = {
  deleteRemoteObjectsVMT<DArray>,
  deleteRemoteObjectsVMT<DFArray>,
  deleteRemoteObjectsVM_Matrix
};

int deleteRemoteObjectsSelect(ListExpr args){
  if(DArray::checkType(nl->First(args))) {
    return 0;
  }
  if(DFArray::checkType(nl->First(args))){
    return 1;
  }
  if(DFMatrix::checkType(nl->First(args))){
    return 2;
  }
  return  -1; 
}


Operator deleteRemoteObjectsOp(
  "deleteRemoteObjects",
  deleteRemoteObjectsSpec.getStr(),
  3,
  deleteRemoteObjectsVM,
  deleteRemoteObjectsSelect,
  deleteRemoteObjectsTM
);


/*
12 Operator ~clone~

This operator gets a darray and a string. It produces a new darray 
as a clone of the first argument having a new name.

12.1 Type Mapping

*/
ListExpr cloneTM(ListExpr args){
  string err = "{darray, dfarray} x string expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err + ": invalid number of args");
  }
  if(   (    !DArray::checkType(nl->First(args)) 
          && !DFArray::checkType(nl->First(args))) 
     || !CcString::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return nl->First(args);
}

/*
12.2 Class for a single task

*/
class cloneTask{

  public:

     cloneTask(DArray* _array, int _index, const string& _name) :
        darray(_array), farray(0), index(_index), name(_name){}
     
     cloneTask(DFArray* _array, int _index, const string& _name) :
        darray(0), farray(_array), index(_index), name(_name){}

     void run(){
         if(darray){
            runDArray();
         } else {
            runFArray();
         }
     }

  private:
      DArray* darray;
      DFArray* farray;
      int index;
      string name;

      void runDArray(){
        string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
        ConnectionInfo* ci = algInstance->getWorkerConnection(
                              darray->getWorkerForSlot(index),dbname);
        if(!ci){
           sendMessage("could not open connection to "
                     +  darray->getWorkerForSlot(index).getHost()
                     + "@" + stringutils::int2str(
                                darray->getWorkerForSlot(index).getPort()));
           return;
        }
        string objName = darray->getName() +"_" + stringutils::int2str(index);
        string newName = name + "_" + stringutils::int2str(index);
        int err;
        string errMsg;
        string resstr;
        double runtime;
        ci->simpleCommand("let " + newName + " = " + objName , 
                          err, errMsg, resstr, false, runtime);
     }

     void runFArray(){
        string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
        ConnectionInfo* ci = algInstance->getWorkerConnection(
                              farray->getWorkerForSlot(index),dbname);
        if(!ci){
           sendMessage("could not open connection to "
                     +  farray->getWorkerForSlot(index).getHost()
                     + "@" 
                     + stringutils::int2str(
                           farray->getWorkerForSlot(index).getPort()));
           return;
        }
        string objName = farray->getName() +"_" + stringutils::int2str(index)
                         + ".bin";
        string newName = name + "_" + stringutils::int2str(index) + ".bin";
        string spath = ci->getSecondoHome()+"/dfarrays/"+dbname+"/" 
                       + farray->getName() + "/";
        string tpath = ci->getSecondoHome()+"/dfarrays/"+dbname+"/" 
                       + name + "/";
        int err;
        string errMsg;
        string resstr;
        double runtime;
        string cmd = "query createDirectory('"+tpath+"', TRUE)";
        ci->simpleCommand(cmd,  err, errMsg, resstr, false, runtime);
        if(err){
           cerr << "error in creating directory " + tpath << endl;
           cerr << "by command " << cmd << endl;
           cerr << errMsg << endl;
        }

        cmd = "query copyFile('"+spath+objName+"', '"+tpath+newName+"')";
        ci->simpleCommand(cmd,  err, errMsg, resstr, false, runtime);
        if(err!=0){
           showError(ci,cmd,err,errMsg);
        }

     }


};


/*
12.3 Value Mapping Function

*/
template<class A>
int cloneVMT(Word* args, Word& result, int message,
            Word& local, Supplier s ){

   A* array = (A*) args[0].addr;
   CcString* name = (CcString*) args[1].addr;
   result = qp->ResultStorage(s);
   A* res = (A*) result.addr;
   (*res) = (*array);

   if(!array->IsDefined() || !name->IsDefined()){
      res->makeUndefined();
      return 0;
   }
   string n = name->GetValue();

   if(!stringutils::isIdent(n)){
      res->makeUndefined();
      return 0;
   }
   res->setName(n);

   vector<cloneTask*> cloner;
   vector<boost::thread*> threads;

   for(size_t i=0; i<array->getSize();i++){
      cloneTask* ct = new cloneTask(array,i,n);
      cloner.push_back(ct);
      boost::thread* t = new boost::thread(&cloneTask::run, ct);
      threads.push_back(t);
   }

   for(size_t i=0;i<array->getSize(); i++){
       threads[i]->join();
       delete threads[i];
       delete cloner[i];
   }
   return 0;
}



/*
12.4 Specification

*/
OperatorSpec cloneSpec(
     " darray x string -> darray",
     " _ clone[_]",
     "Creates a copy of a darray with a new name. " ,
     "let da9 = da8 clone[\"da9\"] "
 );


ValueMapping cloneVM[] = {
  cloneVMT<DArray>,
  cloneVMT<DFArray>
};

int cloneSelect(ListExpr args){
  return DArray::checkType(nl->First(args))?0:1;
}


/*
12.5 Operator instance

*/
Operator cloneOp(
  "clone",
  cloneSpec.getStr(),
  2,
  cloneVM,
  cloneSelect,
  cloneTM
);


/*
13 Operator ~share~

This operator stores a local object to all connections either of a given darray
or all user defined connections.

*/

ListExpr shareTM(ListExpr args){
  string err = "string x bool {[x d[f]array], rel} expected";
  if(!nl->HasLength(args,2) && !nl->HasLength(args,3)){
    return listutils::typeError(err);
  }
  if(!CcString::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(!CcBool::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  if(nl->HasLength(args,3)){
    if(   !DArray::checkType(nl->Third(args))
       && !DFArray::checkType(nl->Third(args))){
       ListExpr positions;
       string errmsg;
       ListExpr types;
       if(!isWorkerRelDesc(nl->Third(args),positions,types,errmsg)){
           return listutils::typeError(err);
       } else {
           ListExpr appendList = nl->FiveElemList(
                                   nl->First(positions),
                                   nl->Second(positions),
                                   nl->Third(positions),
                                   nl->BoolAtom(
                                           CcString::checkType(
                                                 nl->First(types))),
                                   nl->BoolAtom(
                                          CcString::checkType(
                                                 nl->Third(types))));
           return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                                    appendList,
                                    listutils::basicSymbol<FText>());
       }
    }
  }
  return listutils::basicSymbol<FText>(); 
}


class shareRunner{

 public:
    shareRunner(ConnectionInfo* _ci, const string& _objName, 
                const string& _fileName, const bool _relation,
                const bool _attribute,  
                int _id, bool _allowOverwrite, successListener* _listener):
            ci(_ci), objName(_objName), fileName(_fileName),relation(_relation),
            attribute(_attribute),
            id(_id), allowOverwrite(_allowOverwrite), listener(_listener) {}
 

    void operator()(){
      if(relation){
         bool r = ci->createOrUpdateRelationFromBinFile(objName, 
                                                 fileName, allowOverwrite);
         listener->jobDone(id,r);
      } else if(attribute){
         bool r = ci->createOrUpdateAttributeFromBinFile(objName, 
                                                 fileName, allowOverwrite);
         listener->jobDone(id,r);
      } else {
          int err;
          string res;
          string cmd = "delete " + objName;
          double runtime;
          if(allowOverwrite){
             ci->simpleCommand(cmd,err,res, false, runtime);
          }
          cmd = "restore " +  objName + " from '" + fileName +"'";           
          ci->simpleCommand(cmd,err,res,false, runtime);
          listener->jobDone(id, err==0);
      }
    }

 private:
     ConnectionInfo* ci;
     string objName;
     string fileName;
     bool relation;
     bool attribute;
     int id;
     bool allowOverwrite;
     successListener* listener;

};


template<class AType>
class shareInfo: public successListener{

  public:
    shareInfo(const string& _name, const bool _allowOverwrite,
              AType* _array, FText* _result): name(_name),
              allowOverwrite(_allowOverwrite), array(_array),
              result(_result), fileCreated(false) {
      failed = 0;
      success = 0;
      value.addr = 0;
   }

   ~shareInfo(){
      if(value.addr){
         SecondoSystem::GetCatalog()->CloseObject(typeList, value);
         value.addr = 0;
      }
   }


    void share(){
       SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
       string tn;
       bool defined;
       bool hasTypeName;
       if(!ctlg->GetObjectExpr(name, tn, typeList, value, 
                               defined, hasTypeName)){
          result->Set(true, "Name " + name + " is not on object");
          return;
       }
       if(!defined){
         result->Set("Undefined objects cannot be shared");
         return;
       }

       if(array){
         shareArray();
       } else {
         shareUser();
       }
   
       
       for(size_t i=0;i<runners.size();i++){
         runners[i]->join();
         delete runners[i];
       }
       string t = "success : " + stringutils::int2str(success)+ ", failed "
                  + stringutils::int2str(failed);
       result->Set(true,t);
       if(filename.size()>0){
          FileSystem::DeleteFileOrFolder(filename); 
       }
    }

    void jobDone(int id, bool success){
       if(success){
          this->success++;
       } else {
          this->failed++;
       }
    }


  private:
    string name;
    bool allowOverwrite;
    AType* array;
    FText* result;
    bool fileCreated;
    Word value;
    ListExpr typeList;
    set<pair <string, int> > cons;
    string filename;
    bool isRelation;
    bool isAttribute;
    boost::mutex createFileMutex;
    vector<boost::thread*> runners;
    int failed;
    int success;

    void shareArray() {
       string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
       for(size_t i=0;i<array->numOfWorkers();i++){
          ConnectionInfo* ci = algInstance->getWorkerConnection(
                              array->getWorker(i),dbname);
          share(ci);
       }
    }

    void shareUser() {
       string dbname = SecondoSystem::GetInstance()->GetDatabaseName();

       for(size_t i=0; i< algInstance->noConnections(); i++){
           ConnectionInfo* ci = algInstance->getConnection(i);
           if(ci){
              ci->switchDatabase(dbname, true);
              share(ci); 
           }
       } 
    }

    void share(ConnectionInfo* ci){
      pair<string,int> con(ci->getHost(), ci->getPort());
      if(cons.find(con)==cons.end()){
         cons.insert(con);
         createFile(ci); 
         shareRunner runner(ci,name,filename, isRelation, isAttribute,
                             cons.size(), allowOverwrite, this);
         boost::thread * t = new boost::thread(runner);
         runners.push_back(t);
      }
    }

    void createFile(ConnectionInfo* ci){
      boost::lock_guard<boost::mutex> guard(createFileMutex);
      if(!fileCreated){
          isAttribute = false;
          isRelation = Relation::checkType(typeList);
          filename = name + "_" + stringutils::int2str(WinUnix::getpid()) 
                     + ".bin";
          if(isRelation){
             ci->saveRelationToFile(typeList, value, filename);
          } else {
             isAttribute = Attribute::checkType(typeList);
             if(isAttribute){
                ci->saveAttributeToFile(typeList, value, filename);
             } else {
                ci->storeObjectToFile(name, value, typeList, filename); 
             }
          }
          fileCreated=true;
      }      
    }

};

template<class A>
int shareVMT(Word* args, Word& result, int message,
            Word& local, Supplier s ){

  CcString* objName = (CcString*) args[0].addr;
  CcBool* overwrite = (CcBool*) args[1].addr;

  int sons = qp->GetNoSons(s);
  A* array = 0;
  A a(137);

  if(sons == 2 ){ 
    // without third argument
    array = 0;
  } else if(sons ==3){
    // array given by user
    array = (A*) args[2].addr;
  } else {
    // workers given in relation

    int hostPos = ((CcInt*) args[3].addr)->GetValue();
    int portPos = ((CcInt*) args[4].addr)->GetValue();
    int confPos = ((CcInt*) args[5].addr)->GetValue();
    bool hostStr = ((CcBool*) args[6].addr)->GetValue();
    bool confStr = ((CcBool*) args[7].addr)->GetValue();

    if(hostStr && confStr){
       a = DArrayBase::createFromRel<CcString, CcString,A>(
               (Relation*) args[2].addr, 400, "tmp",
                hostPos, portPos, confPos);
    } else if(hostStr && !confStr){
       a = DArrayBase::createFromRel<CcString, FText,A>(
               (Relation*) args[2].addr, 400, "tmp",
                hostPos, portPos, confPos);
    } else if(!hostStr && confStr){
       a = DArrayBase::createFromRel<FText, CcString,A>(
               (Relation*) args[2].addr, 400, "tmp",
                hostPos, portPos, confPos);
    } else if(!hostStr && !confStr){
       a = DArrayBase::createFromRel<FText, FText,A>(
               (Relation*) args[2].addr, 400, "tmp",
                hostPos, portPos, confPos);
    }
    array = &a;
  }


  result = qp->ResultStorage(s);
  FText* res = (FText*) result.addr;
  if(array && !array->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  if(!objName->IsDefined() || !overwrite->IsDefined()){
     res->SetDefined(false);
     return 0;
  }

  shareInfo<A> info(objName->GetValue(), overwrite->GetValue(), array, res);
  info.share();
  return 0;
}


OperatorSpec shareSpec(
     "string x bool [x d[f]array] -> text",
     "share(ObjectName, allowOverwrite, workerArray)",
     "distributes an object from local database to the workers."
     "The allowOverwrite flag controls whether existing objects "
     "with the same name should be overwritten. Id the optional "
     "darray argument is given, the object is stored at all "
     "workers contained within this array. If this argument is "
     "missing the user defined connections are used as workers.",
     "query share(\"ten\", TRUE,da8) "
 );


int shareSelect(ListExpr args){
   if(!nl->HasLength(args,3)){
      return 0; 
   }
   return DArray::checkType(nl->Third(args))?0:1;
}

ValueMapping shareVM[] = {
   shareVMT<DArray>,
   shareVMT<DFArray>
};


/*
12.5 Operator instance

*/
Operator shareOp(
  "share",
  shareSpec.getStr(),
  2,
  shareVM,
  shareSelect,
  shareTM
);


/*
13 Operator ~cleanUp~

This operators removes all temporarly objects from remote server
either from a given darray object or all connected objects (including
worker connections).

13.1 Type Mapping

*/
ListExpr cleanUpTM(ListExpr args){
  string err= " no argument or d[f]array expected";
  if(!nl->HasLength(args,1) && !nl->IsEmpty(args)){
     return listutils::typeError(err);
  }
  if(nl->HasLength(args,1)){
     if(!DArray::checkType(nl->First(args))
        && !DFArray::checkType(nl->First(args))){
        return listutils::typeError(err);
     }
  }
  return listutils::basicSymbol<CcBool>();
}


/*
13.2 Value Mapping

*/
template<class A>
int cleanUpVMT(Word* args, Word& result, int message,
            Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  res->Set(true,true);

  if(qp->GetNoSons(s)==0){
    algInstance->cleanUp();
  } else {
    set<pair<string,int> > used;
    A* arg = (A*) args[0].addr;
    if(!arg->IsDefined()){
      res->SetDefined(false);
      return 0;
    }
    
    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    vector<boost::thread*> runners;
    for(size_t i=0;i<arg->numOfWorkers();i++){
      DArrayElement e = arg->getWorker(i);
      pair<string,int> p(e.getHost(),e.getPort());
      if(used.find(p)==used.end()){
         used.insert(p);
         ConnectionInfo* ci = algInstance->getWorkerConnection(e,dbname);
         if(ci){
             boost::thread* r = new boost::thread(&ConnectionInfo::cleanUp,ci);
             runners.push_back(r);
         }
      }
    }
    for(size_t i=0;i<runners.size();i++){
        runners[i]->join();
        delete runners[i];
    }
  }
  return 0;
}


/*
13.3 Specification

*/
OperatorSpec cleanUpSpec(
     "-> bool , d[f]array -> bool",
     "cleanUp(_)",
     "Removes temporary objects, i.e. objects whose name starts with TMP_, "
     "from remote servers. If no argument is given, all open connections to "
     "servers are used for removing objects. If the darray argument is "
     "present, only the workers specified by this argument are used.",
     "query cleanUp() "
 );

int cleanUpSelect(ListExpr args){
  if(nl->IsEmpty(args)){
     return 0;
  }
  return DArray::checkType(nl->First(args))?0:1;
}

ValueMapping cleanUpVM[] = {
   cleanUpVMT<DArray>,
   cleanUpVMT<DFArray>
};


/*
13.4 Operator instance

*/

Operator cleanUpOp(
  "cleanUp",
  cleanUpSpec.getStr(),
  2,
  cleanUpVM,
  cleanUpSelect,
  cleanUpTM
);

/*
14 Operators acting on dfarrays

14.1 ~dfdistribute2~

*/

OperatorSpec dfdistribute2Spec(
     " stream(tuple(X)) x string x ident x int x rel -> dfarray(X) ",
     " _ dfdistribute2[name, attrname, size, workers]",
     "Distributes a locally stored relation into a dfarray. "
     "The first argument is the tuple stream to distribute. The second "
     "argument is the name of the resulting dfarray. If an empty string "
     "is given, a name is choosen automatically. The Third argument is "
     "an attribute within the tuple stream of type int. "
     "This attribute controls in which slot of the resulting array "
     "is inserted the corresponding tuple. The fourth argument specifies "
     "the size of the resulting array. The relation argument specifies "
     "the workers for this array. It must be a relation having attributes "
     "Host, Port, and Config. Host and Config must be of type string or text, "
     "the Port attribute must be of type int. " 
     "The fifth attribute specifies the name of the resulting dfarray.",
     " query strassen feed addcounter[No,1] dfdistribute2[\"fstrassen\", "
     "No, 5, workers]  "
 );

/*
13.4 Operator instnace

*/
ValueMapping fdistribute2VM [] = {
   ddistribute2VMT<DFArray, FRelCopy, CcString,CcString>,
   ddistribute2VMT<DFArray, FRelCopy, CcString,FText>,
   ddistribute2VMT<DFArray, FRelCopy, FText,CcString>,
   ddistribute2VMT<DFArray, FRelCopy, FText,FText>
};


Operator dfdistribute2Op(
  "dfdistribute2",
  dfdistribute2Spec.getStr(),
  4, 
  fdistribute2VM,
  distribute2Select,
  ddistribute2TMT<DFArray>
);


/*
15 Operator dfdistribute3

Destributes a tuple stream to an fdarray. The dsitribution to the slots
of the array is done using the same mechanism as for the ddistribute3
operator.

*/


OperatorSpec dfdistribute3Spec(
     " stream(tuple(X)) x string x int x bool x rel -> dfarray(X) ",
     " _ dfdistribute3[ name, size, <meaning of size>, workers]",
     "Distributes a tuple stream into a dfarray. The boolean "
     "flag controls the method of distribution. If the flag is set to "
     "true, the integer argument specifies the target size of the "
     "resulting dfarray and the tuples are distributed in a circular way. "
     "In the other case, this number represents the size of a single "
     "array slot. A slot is filled until the size is reached. After that "
     "a new slot is opened. The string attribute gives the name of the "
     "result. The relation specifies the workers and has to contain "
     "attributes Host(string or text), Port(int), and Config(string or text).",
     " query strassen feed dfdistribute3[\"dfa20\",10, TRUE, workers ]  "
     );

ValueMapping dfdistribute3VM[] = {
  distribute3VMT<DFArray, FRelCopy, CcString, CcString>,
  distribute3VMT<DFArray, FRelCopy, CcString, FText>,
  distribute3VMT<DFArray, FRelCopy, FText, CcString>,
  distribute3VMT<DFArray, FRelCopy, FText, FText>
};

Operator dfdistribute3Op(
  "dfdistribute3",
  dfdistribute3Spec.getStr(),
  4, 
  dfdistribute3VM,
  distribute3Select,
  distribute3TM<DFArray>
);

/*
Operator fdistribute4

*/
OperatorSpec dfdistribute4Spec(
     " stream(tuple(X)) x (tuple->int) x int x rel x string-> dfarray(X) ",
     " stream dfdistribute4[ fun, size, workers, name ]",
     " Distributes a locally stored relation into a dfarray ",
     " query strassen feed  dfdistribute4[ hashvalue(.Name,2000),"
     " 8, workers, \"df8\"]  "
     );

ValueMapping dfdistribute4VM[] = {
   distribute4VMT<DFArray, FRelCopy, CcString, CcString>,
   distribute4VMT<DFArray, FRelCopy, CcString, FText>,
   distribute4VMT<DFArray, FRelCopy, FText, CcString>,
   distribute4VMT<DFArray, FRelCopy, FText, FText>
};

Operator dfdistribute4Op(
  "dfdistribute4",
  dfdistribute4Spec.getStr(),
  4,
  dfdistribute4VM,
  distribute4Select,
  distribute4TMT<DFArray>
);



/*
14 Operator ~convertDArray~

This operator converts a darray into a dfarray and vice versa.

*/
ListExpr convertdarrayTM(ListExpr args){

 string err = "darray or dfarray expected";

 if(!nl->HasLength(args,1)){
    return listutils::typeError(err + " ( wrong number of args)");
 }
 ListExpr arg = nl->First(args);
 if(DFArray::checkType(arg)){
   return nl->TwoElemList(
               listutils::basicSymbol<DArray>(),
               nl->Second(arg));
 }
 if(!DArray::checkType(arg)){
    return listutils::typeError(err + " ( wrong number of args)");
 }
 ListExpr subtype = nl->Second(arg);
 if(!Relation::checkType(subtype)){
    return listutils::typeError("subtype must be a relation");
 }
 return nl->TwoElemList( listutils::basicSymbol<DFArray>(),
                         subtype);
}

template<class A, class R>
int convertdarrayVMT(Word* args, Word& result, int message,
            Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  A* arg = (A*) args[0].addr;
  R* res = (R*) result.addr;
  (*res) = (*arg);
  return 0;
}

int convertdarraySelect(ListExpr args){
  return DArray::checkType(nl->First(args))?0:1;
}

ValueMapping convertdarrayVM[] = {
   convertdarrayVMT<DArray,DFArray>,
   convertdarrayVMT<DFArray,DArray>
};

OperatorSpec convertdarraySpec(
     "darray -> dfarray, dfarray -> darray ",
     "convertdarray(_)",
     "Converts a darray into a dfarray and vice versa. "
     "Note that this only converts the root of the array, "
     "i.e. the slots of the result are empty even if the "
     "slots of the argument are not.",
     "query convertdarray(da8) "
 );


Operator convertdarrayOp(
  "convertdarray",
  convertdarraySpec.getStr(),
  2,
  convertdarrayVM,
  convertdarraySelect,
  convertdarrayTM
);



/*
15 Operator ~gettuples~

Retrieves tuples from a relation file created with fconsume5

*/

ListExpr gettuplesTM(ListExpr args){
  string err = "stream(Tuple) x {string,text} expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of args)");
  }
  // check for usesArginTypeMapping
  if(    !nl->HasLength(nl->First(args),2) 
      || !nl->HasLength(nl->Second(args),2)){
    return listutils::typeError("internal error");
  }
  ListExpr arg1Type = nl->First(nl->First(args)); 

  if(!Stream<Tuple>::checkType(arg1Type)){
    return listutils::typeError(err + " (first arg is not a tuple stream)");
  }
  ListExpr arg2Type = nl->First(nl->Second(args));
  if(!CcString::checkType(arg2Type) && !FText::checkType(arg2Type)){
    return listutils::typeError(err + " (second arg not a string "
                                "and not a text)");
  }
  string name;
  ListExpr tid = listutils::basicSymbol<TupleIdentifier>();
  ListExpr attrList = nl->Second(nl->Second(arg1Type));
  int pos = listutils::findType( attrList, tid, name);
  if(!pos){
    return listutils::typeError("tuple stream does not contain "
                                "a tid attribute");
  }
  ListExpr arg2q = nl->Second(nl->Second(args));
  string filename;
  if(CcString::checkType(arg2Type)){
    if(!getValue<CcString>(arg2q,filename)){
      return listutils::typeError("could not get filename from " 
                                  + nl->ToString(arg2q));
    }
  } else {
    if(!getValue<FText>(arg2q,filename)){
      return listutils::typeError("could not get filename from " 
                                  + nl->ToString(arg2q));
    }
  }

  ffeed5Info info(filename);
  if(!info.isOK()){
    return listutils::typeError("file " + filename + 
                           " not present or not a binary relation file");
  }
  ListExpr relType = info.getRelType(); 
 
  ListExpr conAttrList;

  if(nl->HasLength(attrList,1)){ // only a tid attribute
     conAttrList = nl->Second(nl->Second(relType));
  } else {
    ListExpr head;
    ListExpr last;
    set<string> names;
    names.insert(name);
    listutils::removeAttributes(attrList,names,head,last);
    ListExpr fattrList = nl->Second(nl->Second(relType));
    while(!nl->IsEmpty(fattrList)){
       ListExpr first = nl->First(fattrList);
       fattrList = nl->Rest(fattrList);
       last = nl->Append(last,first);
    }
    if(!listutils::isAttrList(head)){
       return listutils::typeError("name conflicts in attributes");
    }
    conAttrList = head;
  }
  ListExpr resType = nl->TwoElemList(
                          listutils::basicSymbol<Stream<Tuple> >(),
                          nl->TwoElemList(
                             listutils::basicSymbol<Tuple>(),
                             conAttrList));
  return nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            nl->OneElemList( nl->IntAtom(pos-1)),
            resType);
}


class gettuplesInfo{

public:
  gettuplesInfo(Word _stream, int _tidPos, const string& _filename, 
               ListExpr _tt):
    stream(_stream), tidPos(_tidPos), filename(_filename) ,tt(0),
    reader(_filename){
     tt = new TupleType(_tt);
     stream.open();
  }


  ~gettuplesInfo(){
     stream.close();
     tt->DeleteIfAllowed();
   }

  Tuple* next(){
     if(!reader.isOK()){
        cerr << "problem in reading file";
        return 0;
     }
     Tuple* inTuple = stream.request();
     Tuple* resTuple;
     if(inTuple){
       resTuple = createResTuple(inTuple);
       inTuple->DeleteIfAllowed();
     } else {
       resTuple = 0;
     }
     return resTuple;
  }

private:
   Stream<Tuple> stream;
   int tidPos;
   string filename;
   TupleType* tt;
   ffeed5Info reader;


   Tuple* createResTuple(Tuple* inTuple){
      TupleIdentifier* Tid = (TupleIdentifier*) inTuple->GetAttribute(tidPos);
      TupleId id = Tid->GetTid();
      reader.changePosition(id);
      if(!reader.isOK()){
         cerr << "problem in repositioning file pointer" << endl;
         return 0;
      }
      Tuple* t = reader.next();
      if(inTuple->GetNoAttributes()==1){
         return t;
      }
      Tuple* res = new Tuple(tt);
      int p = 0;
      // copy attributes of inTuple to res
      for(int i=0;i<inTuple->GetNoAttributes(); i++){
          if(i!=tidPos){
            res->CopyAttribute(i,inTuple,p);
            p++;
          }
      }
      // copy attributes of t to res
      for(int i=0;i<t->GetNoAttributes();i++){
          res->CopyAttribute(i,t,i+p);
      }
      t->DeleteIfAllowed();
      return res;
   }
};


template<class T>
int gettuplesVMT(Word* args, Word& result, int message,
            Word& local, Supplier s ){

  gettuplesInfo* li = (gettuplesInfo*) local.addr;
  switch(message){
     case OPEN: {
        if(li){
           delete li;
        }
        int pos = ((CcInt*)args[2].addr)->GetValue();
        string filename = ((T*) args[1].addr)->GetValue();
        ListExpr tt = nl->Second(GetTupleResultType(s));
        local.addr = new gettuplesInfo(args[0], pos, filename, tt);
        return 0;
     }
     case REQUEST:
         result.addr = li?li->next():0;
         return result.addr?YIELD:CANCEL;
     case CLOSE:
          if(li){
             delete li;
             local.addr = 0;
          }
          return 0;
  }
  return -1;
}


ValueMapping gettuplesVM[] = {
   gettuplesVMT<CcString>,
   gettuplesVMT<FText>
};

int gettuplesSelect(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}

OperatorSpec gettuplesSpec(
  "stream(tuple(X)) x {string,text} -> stream(tuple(Y))",
  "_ _ getTuples",
  "Retrieves tuples from a binary relation file.",
  "query tree exactmatch[3] 'file' gettuples consume"
);

Operator gettuplesOp(
  "gettuples",
  gettuplesSpec.getStr(),
  2,
  gettuplesVM,
  gettuplesSelect,
  gettuplesTM  
);



/*
16 Operator ~dmap~

This operator maps the content of a dfarray to another value.
Depending on the result of the function, the result is a 
dfarray (result is a relation) or a darray (result is 
something other).

*/

ListExpr dmapTM(ListExpr args){
  string err = "d[f]array(X)  x string x fun expected";
  if(!nl->HasLength(args,3)){
    return  listutils::typeError(err + " (wrong number of args)");
  }
  // check for internal correctness
  if(   !nl->HasLength(nl->First(args),2)
      ||!nl->HasLength(nl->Second(args),2)
      ||!nl->HasLength(nl->Third(args),2)){
    return listutils::typeError("internal error");
  }
  ListExpr arg1Type = nl->First(nl->First(args));
  ListExpr arg2Type = nl->First(nl->Second(args));
  ListExpr arg3Type = nl->First(nl->Third(args));

  if(  (!DFArray::checkType(arg1Type) && !DArray::checkType(arg1Type))
     ||!CcString::checkType(arg2Type)
     ||!listutils::isMap<1>(arg3Type)){
    return listutils::typeError(err);
  }

  if(   DArray::checkType(arg1Type) 
     && !Relation::checkType(nl->Second(arg1Type))){
     return listutils::typeError("subtype of darray is not a relation");
  }


  ListExpr frelt = nl->TwoElemList(
                     listutils::basicSymbol<frel>(),
                     nl->Second(nl->Second(arg1Type)));

  ListExpr funArg = nl->Second(arg3Type);

  ListExpr expFunArg = DArray::checkType(arg1Type)
                      ? nl->Second(arg1Type)
                      : frelt; 


  if(!nl->Equal(expFunArg,funArg)){
     stringstream ss;
     ss << "type mismatch between function argument and "
        << " subtype of dfarray" << endl
        << "subtype is " << nl->ToString(expFunArg) << endl
        << "funarg is " << nl->ToString(funArg) << endl;
    
     return listutils::typeError(ss.str());
  }


  ListExpr funq = nl->Second(nl->Third(args));

  ListExpr funargs = nl->Second(funq);
  ListExpr rfunargs = nl->TwoElemList(
                         nl->First(funargs),
                          funArg);

  ListExpr rfun = nl->ThreeElemList(
                    nl->First(funq),
                    rfunargs,
                    nl->Third(funq)
                  );

  ListExpr funRes = nl->Third(arg3Type);
  
  bool isRel = Relation::checkType(funRes);
  bool isStream = Stream<Tuple>::checkType(funRes);

  if(listutils::isStream(funRes) && !isStream){
    return listutils::typeError("function produces a stream of non-tuples.");
  }

  if(isStream){
    funRes = nl->TwoElemList(
               listutils::basicSymbol<Relation>(),
               nl->Second(funRes));
  }
  

  ListExpr resType = nl->TwoElemList(
              isRel||isStream?listutils::basicSymbol<DFArray>()
                   :listutils::basicSymbol<DArray>(),
               funRes);

  return nl->ThreeElemList(
          nl->SymbolAtom(Symbols::APPEND()),
          nl->ThreeElemList( nl->TextAtom(nl->ToString(rfun)),
                            nl->BoolAtom(isRel),
                            nl->BoolAtom(isStream)),
          resType
       ); 
}

template<class A>
class Mapper{
 public: 
   Mapper(A* _array, ListExpr _aType, CcString* _name, FText* _funText ,
          bool _isRel, bool _isStream, void* res):
          array(_array), aType(_aType), ccname(_name), 
          funText(_funText), isRel(_isRel),
          isStream(_isStream) {
       if(isRel || isStream){
         dfarray = (DFArray*) res;
         darray = 0; 
       } else {
         dfarray = 0;
         darray = (DArray*) res;
       }
   }

   void start(){
       if(dfarray){
          startDFArray();
       } else {
          startDArray();
       }
    }

 private:
    A* array;
    ListExpr aType;
    CcString* ccname;
    FText* funText;
    bool isRel;
    bool isStream;
    DFArray* dfarray;
    DArray* darray;
    string name;


    void startDArray(){
       if(!array->IsDefined()){
         darray->makeUndefined();
       }
       *darray = (*array);
       if(array->numOfWorkers()<1){
         darray->makeUndefined();
       }
       string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
       if(!ccname->IsDefined() || ccname->GetValue().length()==0){
          algInstance->getWorkerConnection(array->getWorker(0),dbname);
          name = algInstance->getTempName();            
       } else {
          name = ccname->GetValue();
       }
       if(!stringutils::isIdent(name)){
           darray->makeUndefined();
           return;
       }
       darray->setName(name);
       if(array->getSize()<1){
          // no slots -> nothing to do
          return;
       }
       vector<dRun*> w;
       vector<boost::thread*> runners;

       for( size_t i=0;i< array->getSize();i++){
          ConnectionInfo* ci = algInstance->getWorkerConnection(
                                 array->getWorkerForSlot(i), dbname);
          dRun* r = new dRun(ci,dbname, i, this);
          w.push_back(r);
          boost::thread* runner = new boost::thread(&dRun::run, r);
          runners.push_back(runner);
       }

       for( size_t i=0;i< array->getSize();i++){
          runners[i]->join();
          delete runners[i];
          delete w[i];
       }
       
    }


    void startDFArray(){
       if(!array->IsDefined()){
           dfarray->makeUndefined();
           return;
       }
      *dfarray = *array;
       
       if(array->numOfWorkers()<1){
          dfarray->makeUndefined();
          return;
       }

       string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
       if(!ccname->IsDefined() || ccname->GetValue().length()==0){
          algInstance->getWorkerConnection(array->getWorker(0),dbname);
          name = algInstance->getTempName();            
       } else {
          name = ccname->GetValue();
       }


       if(!stringutils::isIdent(name)){
           dfarray->makeUndefined();
           return;
       }
       dfarray->setName(name);
       if(array->getSize()<1){
          return;
       }

       vector<fRun*> w;
       vector<boost::thread*> runners;
       for( size_t i=0;i< array->getSize();i++){
          ConnectionInfo* ci = algInstance->getWorkerConnection(
                                 array->getWorkerForSlot(i), dbname);
          fRun* r = new fRun(ci,dbname, i, this);
          w.push_back(r);
          boost::thread* runner = new boost::thread(&fRun::run, r);
          runners.push_back(runner);
       }
       for( size_t i=0;i< array->getSize();i++){
          runners[i]->join();
          delete runners[i];
          delete w[i];
       }
    }

   
    class fRun{
      public:
        fRun(ConnectionInfo* _ci, const string& _dbname, int _nr, 
             Mapper* _mapper):
           ci(_ci), dbname(_dbname), nr(_nr), mapper(_mapper) {}

        void run(){
           if(!ci){
             return;
           }

           // create temporal function
           string funName = "tmpfun_"+stringutils::int2str(ci->serverPid())
                             + "_"+ stringutils::int2str(nr);

           string fun = mapper->funText->GetValue();

           string cmd = "(let " + funName + " = " + fun +")";
           int err; string errMsg; string r;
           double runtime;
           ci->simpleCommand("delete "+funName,err,errMsg,r,false, runtime);
           ci->simpleCommandFromList(cmd,err,errMsg,r,true, runtime);
           if(err!=0){
             cerr << "problem in command " << cmd;
             cerr << "code : " << err << endl;
             cerr << "msg : " << errMsg << endl;
             return;
           } 
           string n = mapper->array->getName()+"_"+stringutils::int2str(nr);
           if(mapper->array->getType()==DFARRAY){
               string fname1 = ci->getSecondoHome()+"/dfarrays/"+dbname+"/"
                   + mapper->array->getName() + "/"
                   + n + ".bin";
               ListExpr frelType = nl->TwoElemList(
                                     listutils::basicSymbol<frel>(),
                                     nl->Second(nl->Second(mapper->aType))); 
               string funarg = "[ const " + getUDRelType(frelType) 
                               + " value  '" + fname1 +"' ]"; 
               cmd = "query "+ funName +"( "+funarg +" )";
           } else {
               cmd = "query " + funName+"( " + n +" )";
           }
            
           string targetDir = ci->getSecondoHome()+"/dfarrays/"+dbname+"/"
                              + mapper->name + "/" ;

           string cd = "query createDirectory('"+targetDir+"', TRUE)";
           ci->simpleCommand(cd, err, errMsg,r, false, runtime);
           if(err){
             cerr << "creating directory failed, cmd = " << cd << endl;
             cerr << errMsg << endl;
           }

           string fname2 =   targetDir
                           + mapper->name + "_" + stringutils::int2str(nr)
                           + ".bin";

           if(mapper->isRel) {
             cmd += " feed fconsume5['"+fname2+"'] count";
           } else {
             cmd += "fconsume5['"+fname2+"'] count";
           }

           ci->simpleCommand(cmd,err,errMsg,r,false, runtime);
           if((err!=0) ){ 
              showError(ci,cmd,err,errMsg);
           }
           ci->simpleCommand("delete "+funName,err,errMsg,r,false, runtime);

        }

      private:
        ConnectionInfo* ci;
        string dbname;
        size_t nr;
        Mapper* mapper;

    };

 
    class dRun{
      public:
        dRun(ConnectionInfo* _ci, const string& _dbname, int _nr, 
             Mapper* _mapper):
           ci(_ci), dbname(_dbname), nr(_nr), mapper(_mapper) {}

        void run(){
          if(!ci){
             return;
          }
             // create temporal function
          string funName = "tmpfun_"+stringutils::int2str(ci->serverPid())
                           + "_" + stringutils::int2str(nr);
          string cmd = "(let " + funName + " = " 
                     + mapper->funText->GetValue() +")";
          int err; string errMsg; string r;
          double runtime;
          ci->simpleCommand("delete "+funName,err,errMsg,r,false, runtime);
          ci->simpleCommandFromList(cmd,err,errMsg,r,true, runtime);
          if(err!=0){
            showError(ci,"delete "+funName,err,errMsg);
           return;
          } 
             
          string funarg;
          string n = mapper->array->getName()+"_"+stringutils::int2str(nr);

          if(mapper->array->getType()==DFARRAY){  
              string fname1 = ci->getSecondoHome()+"/dfarrays/"+dbname+"/"
                              + mapper->array->getName() + "/"
                              + n + ".bin";
               ListExpr frelType = nl->TwoElemList(
                                     listutils::basicSymbol<frel>(),
                                     nl->Second(nl->Second(mapper->aType))); 
               funarg = "[ const " + getUDRelType(frelType) 
                               + " value  '" + fname1 +"' ]"; 
          } else {
               funarg = n + " ";
          }

          string name2 = mapper->name + "_" + stringutils::int2str(nr);
          cmd = "let "+ name2 +" = " + funName +"( " + funarg + ")";

          ci->simpleCommand(cmd,err,errMsg,r,false, runtime);
          if((err!=0)  ){ // ignore type map errors
                                    // because reason is a missing file
             showError(ci,cmd,err,errMsg);
          }
          ci->simpleCommand("delete "+funName,err,errMsg,r,false, runtime);
        }

      private:
        ConnectionInfo* ci;
        string dbname;
        size_t nr;
        Mapper* mapper;

    };
};

template<class A>
int dmapVMT(Word* args, Word& result, int message,
            Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  A* array = (A*) args[0].addr;
  CcString* name = (CcString*) args[1].addr;
   // ignore original fun at args[2];
  FText* funText = (FText*) args[3].addr;
  bool isRel = ((CcBool*) args[4].addr)->GetValue();
  bool isStream = ((CcBool*) args[5].addr)->GetValue();
  Mapper<A> mapper(array, qp->GetType(qp->GetSon(s,0)), name, funText, 
                   isRel, isStream, result.addr);
  mapper.start();
  return 0;
}


ValueMapping dmapVM[] = {
   dmapVMT<DArray>,
   dmapVMT<DFArray>
};

int dmapSelect(ListExpr args){

  return DArray::checkType(nl->First(args))?0:1;

}


OperatorSpec dmapSpec(
  "d[f]array x string x fun -> d[f]array",
  "_ dmap[_,_]",
  "Performs a function on a distributed file array. "
  "If the string argument is empty or undefined, a name for "
  "the result is chosen automatically. If not, the string "
  "specifies the name. The result is of type dfarray if "
  "the function produces a tuple stream or a relationi; "
  "otherwise the result is a darray.",
  "query dfa8 dmap[\"\", . head[23]] "
);




Operator dmapOp(
  "dmap",
  dmapSpec.getStr(),
  2,
  dmapVM,
  dmapSelect,
  dmapTM
);

/*
19 TypeMapOperator DFARRAYSTREAM

*/

ListExpr DFARRAYSTREAMTM(ListExpr args){


  if(!nl->HasMinLength(args,1)){
    return listutils::typeError("too less elements");
  }
  ListExpr arg = nl->First(args);
  if(!DFArray::checkType(arg)){
    return listutils::typeError("dfarray expected");
  }
  ListExpr res =  nl->TwoElemList(
                      listutils::basicSymbol<Stream<Tuple> >(),
                      nl->Second(nl->Second(arg)));
  return res;

}


OperatorSpec DFARRAYSTREAMSPEC(
  "DFARRAYSTREAMSPEC",
  "dfarray(rel(X)) x ... -> stream(X) ",
  "Type map operator.",
  "query da8 dmap[\"\", . feed head[23]]"
);

Operator DFARRAYSTREAMOP(
  "DFARRAYSTREAM",
   DFARRAYSTREAMSPEC.getStr(),
   0,
   Operator::SimpleSelect,
   DFARRAYSTREAMTM
);

class transferatorStarter{
  public:
  transferatorStarter(ConnectionInfo* _ci, int _port): ci(_ci), port(_port){
     runner = new boost::thread(&transferatorStarter::run, this);  
  }

  ~transferatorStarter(){
       runner->join();
       delete runner;
   }

  private: 
     ConnectionInfo* ci;
     int port;
     boost::thread* runner;

     void run(){
        
        string cmd = "query staticFileTransferator("
                     + stringutils::int2str(port) + ",10)";
        int err;
        string errMsg; 
        string res;
        double runtime;
        ci->simpleCommand(cmd, err,errMsg, res, false, runtime);
        if(err!=0){
           showError(ci,cmd,err,errMsg);
        }
     }
};


template<class AT>
bool startFileTransferators( AT* array, int port){
   assert(port>0);
   set<string> usedIPs;
   vector<transferatorStarter*> starters;
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   for(size_t i=0;i<array->numOfWorkers();i++){
     DArrayElement e = array->getWorker(i);
     string host = e.getHost();
     if(usedIPs.find(host)==usedIPs.end()){
        ConnectionInfo* ci = algInstance->getWorkerConnection(e,dbname);
        if(ci){
           usedIPs.insert(host);
           starters.push_back(new transferatorStarter(ci,port));
        } else {
          return false;
        }
     }
   }
   for(size_t i=0;i<starters.size();i++){
      delete starters[i];
   }
   starters.clear();
   return true;

}


bool startFileTransferators( vector<DArrayElement>&  array, int port){
 assert(port>0);
 set<string> usedIPs;
 vector<transferatorStarter*> starters;
 string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
 for(size_t i=0;i<array.size();i++){
     DArrayElement e = array[i];
     string host = e.getHost();
     if(usedIPs.find(host)==usedIPs.end()){
      ConnectionInfo* ci = algInstance->getWorkerConnection(e,dbname);
            if(ci){
                 usedIPs.insert(host);
                 starters.push_back(new transferatorStarter(ci,port));
            } else {
                return false;
            }
     }
 }
 for(size_t i=0;i<starters.size();i++){
        delete starters[i];
 }
 starters.clear();
 return true;
}


bool startFileTransferators( vector<ConnectionInfo*>&  array, int port){
 assert(port>0);
 set<string> usedIPs;
 vector<transferatorStarter*> starters;
 for(size_t i=0;i<array.size();i++){
     ConnectionInfo* info = array[i];
     string host = info->getHost();
     if(usedIPs.find(host)==usedIPs.end()){
         usedIPs.insert(host);
         starters.push_back(new transferatorStarter(info,port));
     }
 }
 for(size_t i=0;i<starters.size();i++){
        delete starters[i];
 }
 starters.clear();
 return true;
}


/*
20 Operator dmapX

20.1 Type Mapping

*/
ListExpr dmapXTM(ListExpr args){

 
 int x = nl->ListLength(args) -3;
 string err = "d[f]array(X_i) ^" + stringutils::int2str(x) + " x string x "
              "fun : (X_0 x X_1 x ...X_" + stringutils::int2str(x)+
              " -> Z) x int expected";

 if(x < 1){
    return listutils::typeError("too few argumnets");
 }
  

 ListExpr ac = args;

 // check uses types in type mapping
 while(!nl->IsEmpty(ac)){
    if(!nl->HasLength(nl->First(ac),2)){
       return listutils::typeError("internal error");
    }
    ac = nl->Rest(ac);
 }
 // copy arguments into partitions
 ListExpr arrays[x];
 ListExpr Name;
 ListExpr name;
 ListExpr Fun;
 ListExpr fun;
 ListExpr Port;
 ListExpr port;

 ac = args;
 for(int i=0;i<x;i++){
    arrays[i] = nl->First(nl->First(ac));
    ac = nl->Rest(ac);
 } 
 Name = nl->First(ac);
 name = nl->First(Name);
 ac = nl->Rest(ac);
 Fun = nl->First(ac);
 fun = nl->First(Fun);
 ac = nl->Rest(ac);
 Port = nl->First(ac);
 port = nl->First(Port);
 ac = nl->Rest(ac);
 assert(nl->IsEmpty(ac));

 for(int i=0;i<x;i++){
    if(     !DArray::checkType(arrays[i]) 
         && !DFArray::checkType(arrays[i])){
      return listutils::typeError(err + "( first " + stringutils::int2str(x) 
                                      + " args must be of type d[f]array)");
    }
 }

 if(!CcString::checkType(name)){
   return  listutils::typeError(err + " (name (string) not found at "
                                      "expected position)");
 }
 if(!listutils::isMapX(x,fun)){
   return listutils::typeError(err + " (function not found at "
                                        "expected position)");
 }
 if(!CcInt::checkType(port)){
   return listutils::typeError(err + " (last argument not of type int)");
 }


 // buils an array of expected function arguments
 // for a DFarray, we use an frel as argument, for a 
 // DArray the darray's subtype is used 
 ListExpr efunargs[x];
 for(int i=0;i<x;i++){
   efunargs[i] = DFArray::checkType(arrays[i])
                    ? nl->TwoElemList( listutils::basicSymbol<frel>(),
                                       nl->Second(nl->Second(arrays[i])))
                    : nl->Second(arrays[i]);
 }

 // check whether function arguments fit to the expected one from the arrays
 ListExpr funargs = fun;
 funargs = nl->Rest(funargs);
 for(int i=0;i<x;i++){
    if(!nl->Equal(efunargs[i], nl->First(funargs))){
      return listutils::typeError("type mismatch between darray subtype and "
                                  "function argument at position "
                                  + stringutils::int2str(i+1) );
    }
    funargs = nl->Rest(funargs);
 }
 assert(nl->HasLength(funargs,1));
 ListExpr funres = nl->First(funargs);
 bool streamRes = false;
 ListExpr resType;
 if(listutils::isStream(funres)){
    if(!Stream<Tuple>::checkType(funres)){
      return listutils::typeError("function produces a stream of non tuples");
    }
    streamRes = true;
    resType = nl->TwoElemList(
                   listutils::basicSymbol<DFArray>(),
                   nl->TwoElemList(
                         listutils::basicSymbol<Relation>(),
                         nl->Second(funres)));
 } else {
    resType =   Relation::checkType(funres)
              ? nl->TwoElemList( listutils::basicSymbol<DFArray>(),
                                 funres)
              : nl->TwoElemList( listutils::basicSymbol<DArray>(),
                                 funres); 
 }

 ListExpr funQ = nl->Second(Fun); 

 // rewrite function arguments
 // this is required to replace type mapping operators by the 
 // corresponding types

 ListExpr rfun = nl->OneElemList(nl->First(funQ));
 ListExpr last = rfun;

 funQ = nl->Rest(funQ);
 for(int i=0;i<x;i++){
    last = nl->Append(last,
                nl->TwoElemList(
                   nl->First(nl->First(funQ)),
                   efunargs[i]
                ));
     funQ = nl->Rest(funQ);
 }
 assert(nl->HasLength(funQ,1));

 last = nl->Append(last, nl->First(funQ));

 ListExpr finalRes =  nl->ThreeElemList(
           nl->SymbolAtom(Symbols::APPEND()),
           nl->TwoElemList(
                 nl->BoolAtom(streamRes),
                 nl->TextAtom( nl->ToString(rfun))),
           resType);
  return finalRes;
}


template<int x>
ListExpr dmapXTMT(ListExpr args){

  if(!nl->HasLength(args,x+3)){
      return listutils::typeError("wrong number of arguments");
  }
  return dmapXTM(args);
}


/*
7 ~transferRequired~

checks whether a transfer is required from source to target if source has
a specified array type.

*/
 bool transferRequired(DArrayElement& target, DArrayElement& source, 
                     arrayType T, const string& dbname){
   if(source.getHost() != target.getHost()){
      // data are on different hosts
      return true;
   }
   // data are on the same host
   // we assume, the whole file system is accessible from each worker
   if(T==DFARRAY){
      return false;
   }
   ConnectionInfo* cs = algInstance->getWorkerConnection(source, dbname);
   ConnectionInfo* ct = algInstance->getWorkerConnection(target, dbname);
   if(!cs || !ct){
      return false;
   }
   bool res =  cs->getSecondoHome() != ct->getSecondoHome();
   return res;
}

/*
7 ~retrieveData~

This function transfers argument data to the target worker if
required. Additionally, one entry is added to both, the sourceNames and the
funargs.

*/

 bool retrieveData(DArrayBase* target, DArrayBase* source, int slot,
                         const string& dbname, const int port, bool isRelation,
                         vector<pair<bool,string> >& sourceNames,
                         vector<string>& funargs,
                         ListExpr argType, bool createObjectIfNecessary=true){
          DArrayBase* a0 = target; // note: array 0 determines the 
                                               // distribution of the result
          DArrayBase* ai = source;
          DArrayElement elem0 = a0->getWorkerForSlot(slot);
          DArrayElement elemi = ai->getWorkerForSlot(slot);
          ConnectionInfo* ci = algInstance->getWorkerConnection(elemi,dbname);
          ConnectionInfo* c0 = algInstance->getWorkerConnection(elem0,dbname);
          int errorCode;
          string errMsg;
          string resList;
          double rt;
          bool formerObject;


          if(transferRequired(elem0,elemi, ai->getType(),dbname)){

             // this code will be processed if the ip's  are different or
             // 
 

            // 1 create temporal file on remote server 
            //if required, (not a dfarray)
             string fileNameOnI1;
             string fileNameOnI;
             bool moved = false;
             if(ai->getType() != DFARRAY){
               // darray, we have to store the object into a temporal file
                fileNameOnI1 =  "darrays/TMP_"
                               + dbname+ "_" + ai->getName() +"_"
                               + stringutils::int2str(slot) + "_"
                               + stringutils::int2str(WinUnix::getpid()) +"_"
                               + stringutils::int2str(ci->serverPid()) + "_"
                               + ".bobj";
                 string odir = ci->getSecondoHome() + "/dfarrays/" 
                               + dbname+"/";
                 fileNameOnI = odir + fileNameOnI1;
                 string oname = ai->getObjectNameForSlot(slot);

                 string cmd = "query " + oname + " saveObjectToFile['" 
                              + fileNameOnI+ "']";
              
                 ci->simpleCommand(cmd, errorCode, errMsg, resList, false, rt);
                 if(errorCode){
                    cerr << __FILE__ << "@"  << __LINE__ << endl;
                    showError(ci,cmd,errorCode,errMsg);
                    return false;
                 } 
                 formerObject = true; 
             } else { // already a file object on ci, hust set file names 
                fileNameOnI = ai->getFilePath(ci->getSecondoHome(), 
                                             dbname, slot);
                fileNameOnI1 = ai->getFilePath("", 
                                             dbname, slot);
                formerObject = false;
             }

             // 1.2 transfer file from i to 0
             string fileNameOn0 = source->getFilePath(c0->getSecondoHome(),
                                                       dbname, slot); 
             string cmd;
             if(c0->getHost() != ci->getHost()){ // use TCP transfer or 
                                                 // disc utils
                   cmd =    "query getFileTCP('" + fileNameOnI + "', '" 
                            + ci->getHost() 
                            + "', " + stringutils::int2str(port) 
                            + ", TRUE, '" + fileNameOn0 +"')";

              } else {
                   string op = formerObject?"moveFile":"copyFile";
                   moved = formerObject;
                   cmd =   "query " + op + "('" + fileNameOnI + "','"
                         + fileNameOn0 + "', TRUE)";
              }
              c0->simpleCommand(cmd, errorCode, errMsg, resList, false, rt);
              if(errorCode){
                 cerr << __FILE__ << "@"  << __LINE__ << endl;
                 showError(c0, cmd, errorCode, errMsg);
                 return false;
             } 

             // remove temp file from original server
             if(formerObject && !moved){
                 string cmd = "query removeFile('" + fileNameOnI+ "')";
                 ci->simpleCommand(cmd, errorCode, errMsg, resList, false, rt);
                 if(errorCode){
                    cerr << __FILE__ << "@"  << __LINE__ << endl;
                    showError(ci,cmd,errorCode, errMsg);
                    return false;
                 } 
             }

             // step 2: create object if required
             if(formerObject && createObjectIfNecessary){
                string oname = ai->getObjectNameForSlot(slot);
                string end = isRelation?"consume":"";
                string cmd = "let " + oname + " =  '"+fileNameOn0
                              +"' getObjectFromFile " + end;
                c0->simpleCommand(cmd, errorCode, errMsg, resList, false, rt);
                if(errorCode){
                    cerr << __FILE__ << "@"  << __LINE__ << endl;
                    showError(c0,cmd,errorCode,errMsg);
                   // return false; //ignore, may be object already there
                 }
                 cmd = "query removeFile('" + fileNameOn0+ "')";
                 c0->simpleCommand(cmd, errorCode, errMsg, resList, false, rt);
                 if(errorCode){
                    cerr << __FILE__ << "@"  << __LINE__ << endl;
                    showError(c0,cmd,errorCode,errMsg);
                    return false;
                 } 
                 sourceNames.push_back(pair<bool,string>(true,oname));
                 funargs.push_back(oname);
                 return true;
             }  else {
                string fname = fileNameOn0;
                string type ="(frel " + nl->ToString(nl->Second(
                              nl->Second(argType))) + ")";
                string fa = "("+ type + "  '"+fname+"')"; 
                funargs.push_back(fa);
                sourceNames.push_back(pair<bool,string>(true,fname)); 
                return true;
             }
          } else { // file or object already on the server
              // create entries in funargs and sourceNames
              if(ai->getType()==DARRAY){
                string oname = ai->getObjectNameForSlot(slot);
                funargs.push_back(oname);
                sourceNames.push_back(pair<bool,string>(false,oname));     
              } else {
                string fname = ai->getFilePath(ci->getSecondoHome(), 
                                               dbname, slot);
                string type ="(frel " + nl->ToString(nl->Second(
                              nl->Second(argType))) + ")";
                string fa = "( "+ type + " '"+fname+"')";
                funargs.push_back(fa);
                sourceNames.push_back(pair<bool,string>(false,fname)); 
              }
              return true;             
          }
       }


/*
8.1 Class ~DMapXInfo~

This class will handle mapping involving any number of d[f]arrays.


*/
class dmapXInfo{

 public:

/*
8.1.1 Constructions

*/
    dmapXInfo( vector<DArrayBase*>& _arguments, vector<bool>& _isRelation, 
               vector<ListExpr> _argTypes,const string& _name, 
               const string& _funText, 
               const bool _isStreamRes, const int _port, DArrayBase* _res): 
               arguments(_arguments),
               isRelation(_isRelation), argTypes(_argTypes),
               name(_name), funText(_funText), 
               isStreamRes(_isStreamRes), port(_port), res(_res) {}


/*
8.1.2 ~start~

This is the main function of this class. It will do some checks
and start a new thread to compute the result for  each slot of 
the result array.

If the distribution of the involved arrays differs, the first array
determines the distribution of the result array.

*/

     void start(){
          // determines whether all arrays are defined and the
          // maximum processing slot size (minimum of alle involved arrays)
          // if ok, for each slot a single thread is started
          if(arguments.size()<1){
             // no arguments found
             res->makeUndefined();
             return;
          }
          if(!arguments[0]->IsDefined()){
             // main array undefined
             res->makeUndefined();
             return;
          }
          minSlots = arguments[0]->getSize();
          for(size_t i=0;i<arguments.size();i++){
              if(!arguments[i]->IsDefined()){
                 // one of the arrays is not defined
                 res->makeUndefined();
                 return;
              }
              if(arguments[i]->getSize() < minSlots){
                 minSlots = arguments[i]->getSize();
              }
          }

          if(minSlots<1){
             // at least one array is empty
             res->makeUndefined();
             return;
          }

          // create map for result array
          vector<uint32_t> map;
          for(size_t i=0;i<minSlots;i++){
             map.push_back(arguments[0]->getWorkerIndexForSlot(i));
          }
          res->set(map, name, arguments[0]->getWorkers());
          
          dbname = SecondoSystem::GetInstance()->GetDatabaseName();

          // the local result is complete, lets start FileTransferators
          // on Servers if neccesary
          if(!startFileTransferators()){
              res->makeUndefined();
              return;
          }
          // start computation of the result for each slot
          if(!buildRemoteObjects()){
              res->makeUndefined();
              return;
          }

     } 

 private:
     vector<DArrayBase*> arguments;
     vector<bool> isRelation;   // for each argument, flag whether 
                                //it is a relation
     vector<ListExpr> argTypes; // the type of each arg
     string name;            // name of the result
     string funText;         // the text of the function
     bool isStreamRes;       // result of the function is a stream ?
     int port;               // port for file transfer
     DArrayBase* res;        // the result array
     size_t minSlots;        // number of slots within the result
     string dbname;          // name of the currently opened database


/*
8.7 ~startFileTranaferators~

This starts a file transferator on each server from which data are required.

*/
     bool startFileTransferators(){
        // collect all servers
        vector<DArrayElement> servers;
        for(size_t i=1;i<arguments.size(); i++){
            if(!getFileTransferators(i, servers)){
              return false;
            }
        }
        // remove duplicates
        servers = reduceFileTransferServers(servers);
        // start the transferator, if failed on a single server,
        // break
        for(size_t i=0;i<servers.size();i++){
           if(!startFileTransferatorOnServer(servers[i])){
             return false;
           }
        }
        return true;
     }


/*
8.8 ~getFileTransferators~

This function inserts  the array elements for which a 
filetransfer is required into the result vector.

*/
     bool getFileTransferators(size_t arg, vector<DArrayElement>& result){
         DArrayBase* array = arguments[arg];
         for(size_t i=0;i<minSlots;i++){
             DArrayElement a0 = arguments[0]->getWorkerForSlot(i);
             DArrayElement ai = arguments[arg]->getWorkerForSlot(i);
             if(transferRequired(a0,ai,array->getType(),dbname)){
                 result.push_back(ai);
             }  
         }
         return true;
     }



/*
8.10 ~buildRemoteObjects~

This functions creates a Runner object for each slot. These objects will create 
the result for their slot.

*/
     bool buildRemoteObjects(){

        if(res->getType() == DFARRAY){
           // we have to build the result directory
           for(size_t i=0;i<minSlots; i++){
               DArrayElement elem = res->getWorkerForSlot(i);
               ConnectionInfo* ci = algInstance->getWorkerConnection(elem,
                                                                     dbname);
               string dirname = res->getPath(ci->getSecondoHome(), dbname);
               string q = "query createDirectory('"+dirname+"', TRUE)";
               int err;
               string errMsg;
               double rt;
               string result;
               ci->simpleCommand(q, err, errMsg, result, false, rt);
               if(err){
                    cerr << __FILE__ << "@" << __LINE__ << endl;
                    showError(ci,q,err,errMsg);
                    res->makeUndefined();
                    return false;
                }
           } 

        }

        vector<dmapXRunner*> runners;


        for(size_t i=0; i<minSlots; i++){
            dmapXRunner* runner = new dmapXRunner(i, this);
            runners.push_back(runner); 
        }
        for(size_t i=0; i<minSlots; i++){
           delete runners[i]; 
        }
        return true; 
     }



/*
8.11 ~reduceFileTransferServers~

This function removes duplicates within the argument list. 
The remainder is one DArray per host name.

*/
     vector<DArrayElement> reduceFileTransferServers(
               vector<DArrayElement> src){
         // it's sufficient to start one server per host
         set<string> hosts;
         vector<DArrayElement> result;
         for(size_t i=0;i<src.size();i++){
             string h = src[i].getHost();
             if(hosts.find(h)==hosts.end()){
               result.push_back(src[i]);
               hosts.insert(h);
             }
         }
         return result;
     }

/*
8.12 ~startFileTransferatorOnServer~

Does what the name says.

*/
     bool startFileTransferatorOnServer(DArrayElement& elem){
         ConnectionInfo* ci = algInstance->getWorkerConnection(elem, dbname);
         if(!ci){
           return false;
         }
         string cmd = "query staticFileTransferator(" + 
                      stringutils::int2str(port) +", 10)";
         int err;
         string errMsg;
         double rt;
         ListExpr result;
         ci->simpleCommand(cmd, err, errMsg, result, false, rt);
         if(err){
            cerr << __FILE__ << "@"  << __LINE__ << endl;
            showError(ci,cmd,err,errMsg);
            return false;
         }
         if(!nl->HasLength(result,2)){
             cerr << "command " << cmd << " returns unexpected result" << endl;
             return false;
         }
         if(nl->AtomType(nl->Second(result))!=BoolType){
             cerr << "command " << cmd << " returns unexpected result" << endl;
             return false;
         }
         if(!nl->BoolValue(nl->Second(result))){
            cerr << "failed to start file transfer server" << endl;
            return false;
         }
         return true;
     }


/*
8.13 class ~dmapXRunner~

An instance of this class is responsible for the computation of the result for
a single slot.

*/
  class dmapXRunner{
    public:

/*
8.13.1 ~Construvtor~

Overtakes the given values and starts a new thread.

*/
       dmapXRunner(size_t _slot, dmapXInfo* _info): slot(_slot), info(_info) {
           t = new boost::thread(&dmapXRunner::run, this);
       }


/*
8.13.2 ~Destructor~

Waits for finishing the local thread and deletes local objects.

*/
       ~dmapXRunner(){
           t->join();
           delete t;
        }
      

    private:
       size_t slot;         // slot to process
       dmapXInfo* info;     // hold some 'global' variables
       boost::thread* t;    // internal thread

       vector<pair<bool, string> > sourceNames;
              // for each involved array
              // stores whether the object is temporal 
              // and the name of the object or the file
       vector<string>  funargs; // contains the function argument for each array


/*
8.13.3 ~run~

This is the main function of this class.
In a first step, all data are collected from servers. If the data are already
on the correct server, no transfer is started. In the same time, the names of
the sources and the correspnding function arguments are created.

*/
       void run(){
           for(size_t i=0;i <info->arguments.size(); i++){
               distributed2::retrieveData(info->arguments[0], 
                            info->arguments[i], slot, 
                            info->dbname, info->port, info->isRelation[i],
                            sourceNames, funargs, info->argTypes[i]);
           }

           // get the info for the worker belonging to the result.
           ConnectionInfo* c0 = algInstance->getWorkerConnection(
                                   info->arguments[0]->getWorkerForSlot(slot),
                                   info->dbname);
           // create the command for result computation
           string cmd = createCommand(c0);
           // process the command
           int err;
           string errMsg;
           double rt;
           string result;
           // if the result is an object, delete existings objects before
           if(info->res->getType()==DARRAY){
               string n = info->res->getObjectNameForSlot(slot);
               c0->simpleCommand("delete " + n,err,errMsg, result, false, rt);
               // ignore error, may be there is no such an object
           }
 
           c0->simpleCommandFromList(cmd, err, errMsg, result, false, rt);
           if(err){
              cerr << __FILE__ << "@"  << __LINE__ << endl;
              cerr << "could not compute result for slot " << slot << endl;
              showError(c0,cmd,err,errMsg);
           }
           // remove temporarly created objects.
           for(size_t i=0;i <info->arguments.size(); i++){
               removeTempData(i, c0);
           } 
       }


/*
8.13.5 ~createCommand~

Creates the command for computing the result for this slot.

*/
       string createCommand(ConnectionInfo* ci){
          string funText = info->funText;
          string funCall = "( " + funText;
          for(size_t i=0;i<funargs.size(); i++){
            funCall = funCall + " " + funargs[i];
          }
          funCall += " )";

          arrayType resType = info->res->getType();
          if(resType == DARRAY){
              // command creates an object (let)
              if(info->isStreamRes){
                 funCall ="( consume " + funCall +")";
              }
              string resultName = info->res->getObjectNameForSlot(slot);
              string res = "( let " + resultName + " = " + funCall +")";
              return res;

          } else {
              // command creates a file (query + fconsume5 + count)
              if(!info->isStreamRes){
                funCall = "( feed " + funCall + ")"; // ensure a stream
              }
              // get result file name
              string resFileName = info->res->getFilePath(ci->getSecondoHome(),
                                           info->dbname, slot); 
              string res ="(query ( count ( fconsume5 "+ funCall+ 
                          "'"+resFileName+"'"+" )))";

              return res;
         }
       }


/*
8.13.6 ~removeTempData~

Removes temporarly created data, i.e. data that was created by file transfers
from other than the result worker.

*/
       void removeTempData(size_t arg, ConnectionInfo* ci){
          if(sourceNames.size()<= arg){
             cerr << "too less source names for arg " << arg << endl;
             return;
          }
          pair<bool,string> toDelete = sourceNames[arg];
          if(!toDelete.first){
             // the source is original, not a good idea to delete it
             return; 
          }
          bool isFile = info->arguments[arg]->getType() == DFARRAY;
          string cmd;
          if(isFile){
             cmd = "query removeFile('" + toDelete.second + "')";
          } else {
             cmd = "delete " + toDelete.second;
          }
          int err;
          string errMsg;
          double rt;
          string result;
          ci->simpleCommand(cmd, err, errMsg, result, false, rt);
          if(err){
            cerr << "could not remove temporal object of argument " << arg 
                 << " for slot " << slot << endl;
            cerr << "because the following command failed: " << cmd  
                 << "with code " << err << endl << errMsg << endl << endl;
          }
       }
  };
};



int dmapXVM(Word* args, Word& result, int message,
            Word& local, Supplier s ){

    result = qp->ResultStorage(s);
    // we have x arrays followed by
    // a name
    // a fun (ignored)
    // a port
    // a boolean value whether the result is a stream
    //     the type mapping esures that is a tuple stream 
    //     in this case
    // the function as a text in nested list format

    int x = qp->GetNoSons(s) - 5; // index until the name


    // collect all d[f]arrays into a vector
    vector<DArrayBase*> arrays;
    vector<bool> isRelation; 
    for(int i=0;i<x;i++){
      arrays.push_back((DArrayBase*) args[i].addr);
      isRelation.push_back(Relation::checkType(nl->Second(
                                        qp->GetType(qp->GetSon(s,i)))));
    }

    CcString* objName = (CcString*) args[x].addr;
    // args[3] is the original fun and is not used here
    CcInt* Port = (CcInt*) args[x+2].addr;
    bool streamRes =  ((CcBool*)args[x+3].addr)->GetValue();
    string funtext = ((FText*) args[x+4].addr)->GetValue();
    string name;
    int port = 0;
    if(Port->IsDefined()){
       port = Port->GetValue();
    }
    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    if(!objName->IsDefined() || objName->GetValue().length()==0){
      algInstance->getWorkerConnection(arrays[0]->getWorker(0),dbname);
      name= algInstance->getTempName();            
    } else {
      name = objName->GetValue();
    }

    if(!stringutils::isIdent(name) || port <= 0){
        ((DArrayBase*) result.addr)->makeUndefined();
        return 0;
    }
    vector<ListExpr> argTypes;    
    for(int i=0;i<x;i++){
        argTypes.push_back(qp->GetType(qp->GetSon(s,i)));
    }

    dmapXInfo info(arrays, isRelation, argTypes,  name, funtext, streamRes, 
                   port, (DArrayBase*) result.addr);
    info.start();

    return 0;
}






/*
5 Operator dmapX

*/
OperatorSpec dmapXSpec(
  "d[f]array^x x string x fun -> d[f]array",
  "dmapX[_,_,_]",
  "Joins the slots of x  distributed arrays",
  "query mapX(df1i, df2, \"df3\" . .. product]"
);

Operator dmap2Op(
  "dmap2",
  dmapXSpec.getStr(),
  dmapXVM,
  Operator::SimpleSelect,
  dmapXTMT<2>
);


Operator dmap3Op(
  "dmap3",
  dmapXSpec.getStr(),
  dmapXVM,
  Operator::SimpleSelect,
  dmapXTMT<3>
);


Operator dmap4Op(
  "dmap4",
  dmapXSpec.getStr(),
  dmapXVM,
  Operator::SimpleSelect,
  dmapXTMT<4>
);


Operator dmap5Op(
  "dmap5",
  dmapXSpec.getStr(),
  dmapXVM,
  Operator::SimpleSelect,
  dmapXTMT<5>
);

Operator dmap6Op(
  "dmap6",
  dmapXSpec.getStr(),
  dmapXVM,
  Operator::SimpleSelect,
  dmapXTMT<6>
);

Operator dmap7Op(
  "dmap7",
  dmapXSpec.getStr(),
  dmapXVM,
  Operator::SimpleSelect,
  dmapXTMT<7>
);

Operator dmap8Op(
  "dmap8",
  dmapXSpec.getStr(),
  dmapXVM,
  Operator::SimpleSelect,
  dmapXTMT<8>
);



/*
3.21 Type Map Operators ~DPRODUCTARG1~ and ~DPRODUCTARG2~

This operator is a special implementation for handling 
dproduct arguments. Depending on the position  of the argument 
as well as the type of the last argument, it will produce
different result types.

case 1: pos is 1, we check whether the first arg is a d[f] array
        and return the subtype or an frel conversion

case 2: pos is 2, we check whether the second argument is a d[f]array
       and the subtype is a relation and return an frel for both types

*/
template<int pos>
ListExpr dproductargT(ListExpr args){
   assert( (pos==1) || (pos==2) );
   if(!nl->HasMinLength(args,pos)){
      return listutils::typeError("too less arguments");
   }
   for(int i=1; i<pos ; i++){
     args = nl->Rest(args);
   }
   ListExpr arg = nl->First(args);
   if(!DArray::checkType(arg) && !DFArray::checkType(arg)){
     return listutils::typeError(  "the argument at position " 
                                 + stringutils::int2str(pos) 
                                 + "is not of type  d[f]array");
   }
   if(pos == 1){
      if(DArray::checkType(arg)){
         return nl->Second(arg);
      }
      return nl->TwoElemList( listutils::basicSymbol<frel>(),
                              nl->Second(nl->Second(arg)));
   }
   // pos == 2
   ListExpr subtype = nl->Second(arg);
   if(!Relation::checkType(subtype)){
      return listutils::typeError("subtype type of second d[f]array "
                                  "is not a relation");
   }

   return nl->TwoElemList( listutils::basicSymbol<fsrel>(),
                           nl->Second(subtype));

}

OperatorSpec dproductarg1Spec (
  " darray(X) x ... -> X | dfarray(rel(x)) -> frel(X) ",
  " op(_,_,_)",
  "A type map operator",
  "query dplz1 dplz2 dproduct[\"\", .. feed . feed concat , 1234]"
);

OperatorSpec dproductarg2Spec (
  "  ? x d[f]array(rel(X)) x ... -> fsrel(X) ",
  " op(_,_,_)",
  "A type map operator",
  "query dplz1 dplz2 dproduct[\"\", .. feed . feed concat , 1234]"
);

Operator dproductarg1Op(
  "DPRODUCTARG1",
  dproductarg1Spec.getStr(),
  0,
  Operator::SimpleSelect,
  dproductargT<1>
);

Operator dproductarg2Op(
  "DPRODUCTARG2",
  dproductarg2Spec.getStr(),
  0,
  Operator::SimpleSelect,
  dproductargT<2>
);


/*
3.16 Operator ~dproduct~

This operator connects all slots of the fisrt argument with all slots of
the second argument using a parameter function. 

*/


ListExpr dproductTM(ListExpr args){
  if(!nl->HasLength(args,5)){
    return listutils::typeError("5 arguments expected");
  }
  string err =" d[f]array(rel T1)) x d[f]array(rel T2) x string x "
              "(stream(T1) x stream(T2) -> stream(T3)) x int expected";

  ListExpr c = args;
  // check for args in type mapping
  while(!nl->IsEmpty(c)){
    if(!nl->HasLength(nl->First(c),2)){
      return listutils::typeError("internal error");
    }
    c = nl->Rest(c);
  }

  if(    !DArray::checkType(nl->First(nl->First(args))) 
      && !DFArray::checkType(nl->First(nl->First(args)))){
    return listutils::typeError(err + " (first arg is not a d[f]array)");
  }
  if(    !DArray::checkType(nl->First(nl->Second(args))) 
      && !DFArray::checkType(nl->First(nl->Second(args)))){
    return listutils::typeError(err + " (2nd arg is not a d[f]array)");
  }
  if(!CcString::checkType(nl->First(nl->Third(args)))){
    return listutils::typeError(err + " (third arg is not a string)");
  }
  if(!listutils::isMap<2>(nl->First(nl->Fourth(args)))){
    return listutils::typeError(err + " (4th arg is not a binary function)");
  }
  if(!CcInt::checkType(nl->First(nl->Fifth(args)))){
    return listutils::typeError(err + " (5th arg is not an integer)");
  }
  ListExpr a1 = nl->First(nl->First(args));
  ListExpr a2 = nl->First(nl->Second(args));
  if(   !Relation::checkType(nl->Second(a1))
     || !Relation::checkType(nl->Second(a2))){
    return listutils::typeError(err + " ( only relationa are allowed for the "
                                      "subtypes of the d[f]arrays)");
  }

  ListExpr s1 =   DFArray::checkType(a1)
                ? nl->TwoElemList(listutils::basicSymbol<frel> (),
                                  nl->Second(nl->Second(a1)))
                : nl->Second(a1);
   
  ListExpr s2 = nl->TwoElemList(listutils::basicSymbol<fsrel>(),
                       nl->Second(nl->Second(a2)));

  ListExpr fun = nl->First(nl->Fourth(args));
  if(    !nl->Equal(s1, nl->Second(fun))
     ||  !nl->Equal(s2, nl->Third(fun))){
     return listutils::typeError(err + " (function arguments does not "
                                       "fit the d[f]array types");
  }
  // check the resulting type of the function. if it is a stream, it has to be a
  // tuple stream 
  ListExpr funres = nl->Fourth(fun);
  bool isStream = false;
  if(listutils::isStream(funres)){
     isStream= true;
     if(!Stream<Tuple>::checkType(funres)){
       return listutils::typeError("invalid function result, stream, "
                                   "but not a tuple stream");
     }
  }

  // replace function arguments
  ListExpr funQuery = nl->Second(nl->Fourth(args));
  ListExpr rfunQuery = nl->FourElemList(
                          nl->First(funQuery),
                          nl->TwoElemList(nl->First(nl->Second(funQuery)), s1),
                          nl->TwoElemList(nl->First(nl->Third(funQuery)), s2),
                          nl->Fourth(funQuery));
                    
   ListExpr resType = isStream? nl->TwoElemList( 
                                      listutils::basicSymbol<Relation>(), 
                                      nl->Second(funres))
                              : funres;

   if(isStream || Relation::checkType(funres)){
      resType = nl->TwoElemList(listutils::basicSymbol<DFArray>(), resType);
   } else {
      resType = nl->TwoElemList(listutils::basicSymbol<DArray>(), resType);
   }

   ListExpr appendList =
        nl->TwoElemList( nl->TextAtom(nl->ToString(rfunQuery)), 
                         nl->BoolAtom(isStream));


   return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                             appendList,
                             resType);
}



void performCommand(ConnectionInfo* ci, const string& cmd){
     int errorCode;
     string errorMsg;
     double rt;
     string res;
     ci->simpleCommand(cmd, errorCode, errorMsg, res, false, rt);
     if(errorCode){
           showError(ci,cmd, errorCode, errorMsg);
     }
}


void performListCommand(ConnectionInfo* ci, const string& cmd){
     int errorCode;
     string errorMsg;
     double rt;
     string res;
     ci->simpleCommandFromList(cmd, errorCode, errorMsg, res, false, rt);
     if(errorCode){
           showError(ci,cmd, errorCode, errorMsg);
     }
}


class dproductInfo{
 public:

    dproductInfo(DArrayBase* _arg0, DArrayBase* _arg1, DArrayBase* _result,
                const string& _funText,ListExpr _a0Type, ListExpr _a1Type,
                int _transferPort, bool _streamRes ): arg0(_arg0), arg1(_arg1),
                result(_result), funText(_funText), a0Type(_a0Type), 
                a1Type(_a1Type), port(_transferPort), streamRes(_streamRes) {
                    dbname = SecondoSystem::GetInstance()->GetDatabaseName();
                }

    void run(){
       if(!arg0->IsDefined() || !arg1->IsDefined()){
          result->makeUndefined();
          return;
       }
       copyHelper* ch = copySlots(); // copy missing slots to the workers
      
       // after copying, all slots of the second arg are accessible on
       // each worker of the first argument.
       // if the slot was already there, it can be accessed by the 
       // original access method (object or frel), otherwise, it can be 
       // accessed  only via frel
       computeResult(ch); // perform the function to compute the result
       removeTemp(ch); // removes temporarly files
       delete ch;
    }





 private:
     DArrayBase* arg0;
     DArrayBase* arg1;
     DArrayBase* result;
     string funText;
     ListExpr a0Type;
     ListExpr a1Type;
     int port;
     bool streamRes;
     string dbname;
     map<string,string> Ip2Home;
     vector<pair<DArrayElement, vector<string> > > temps; 


     string getHome(string IP){
        return Ip2Home[IP];
     }


   class copyHelper{
      public:
         copyHelper() {}
         typedef  pair< vector<ConnectionInfo*>,   
                               vector< pair<bool, string> > > ipinfo;
              // set of involved workers
             // for each slot a pair whether file creation  or
             // file transfer is required
         typedef map<string, ipinfo > content; // meaning ip-> ipinfo
         typedef typename content::iterator iter;

         void add(ConnectionInfo* info, DArrayBase* source, string& dbname){
            string ip = info->getHost();
            string home = info->getSecondoHome();
            iter it = c.find(ip);
            if(it!=c.end()){
              // just insert into existing structure
              it->second.first.push_back(info);
            } else {
                // create new entry
                vector<ConnectionInfo*> infos;
                infos.push_back(info);
                vector<pair<bool, string> > pos;

                for(size_t i= 0; i<source->getSize(); i++){
                   DArrayElement elem = source->getWorkerForSlot(i);
                   ConnectionInfo* sourceInfo = 
                              algInstance->getWorkerConnection(elem, dbname);
                   string sourceHome = sourceInfo->getSecondoHome();
                   if(ip!=sourceInfo->getHost()){
                      pos.push_back(make_pair(true, home) );  
                   } else {
                      if(source->getType()==DFARRAY){//no transfer, just access
                         pos.push_back(make_pair(false,sourceHome));
                      } else {  
                         pos.push_back(make_pair(false, home));
                      }
                   }
                }
                pair<vector<ConnectionInfo*>, 
                     vector<pair<bool, string> > > p(infos,pos);
                c[ip] = p;
            }
         }

         void addAdditional(ConnectionInfo* info){
           string ip = info->getHost();
           iter it = c.find(ip);
           if(it==c.end()){
               return;
           }
           vector<ConnectionInfo*>& infos = it->second.first;
           bool found = false;
           for(size_t i=0;i<infos.size() && !found; i++){
              found = infos[i] == info;
           }
           if(!found){
             it->second.first.push_back(info);
           }
         }

         void getWorkerSelection(vector<ConnectionInfo*>& result){
              result.clear();
              iter it;
              for(it = c.begin(); it!=c.end(); it++){
                 result.push_back(it->second.first[0]);
              }
         }


         ipinfo* getInfo(const string& ip){
            iter it = c.find(ip);
            if(it==c.end()){
               return 0;
            } else {
               return &(it->second);
            }
         }
         inline iter scan(){
           return c.begin();
         } 
         inline iter end(){
           return c.end();
         }

         void print(ostream& out){
            iter it;
            for(it = c.begin();it!=c.end();it++){
               print(it, out);
            }
         }

      private:
        content c;

        void print(iter& it, ostream& out){
          out << "info for ip adress " << it->first << endl;
          ipinfo& info = it->second;
          vector<ConnectionInfo*>& ws = info.first;
          out << "involved workers " << endl;
          for(size_t i=0;i<ws.size();i++){
            out << "   " << *(ws[i]) << endl;
          }
          vector<pair<bool, string> >& homes = info.second;
          out << "map: slot -> transfer , home" << endl;
          for(size_t i=0;i<homes.size();i++){
             out << "  " << i << " : " << homes[i].first << ", " 
                 << homes[i].second << endl;
          }
        }


   };

   class copyRunner{
     public:
        copyRunner(copyHelper::ipinfo& inf , dproductInfo* _pi): 
            pi(_pi){
            // determine which worker is responsible for which slot
            vector<ConnectionInfo*>& workers = inf.first;
            slots = inf.second;
            assert(workers.size()>0);
            int currentWorker = 0;

            string host = workers[0]->getHost();
            // create empty tasks
            map<ConnectionInfo*, size_t> pos;

            for(size_t i=0;i<workers.size();i++){
                vector<pair<bool, size_t> > s;
                ConnectionInfo* w = workers[i];
                pos[w] = i;
                responsible.push_back(make_pair(w,s)); 
            }
            
            for(size_t i=0;i<slots.size(); i++){
               if(slots[i].first){ // slot missing
                   DArrayElement elem = pi->arg1->getWorkerForSlot(i);
                   if(host!=elem.getHost()){ // round robin, real copying
                      responsible[currentWorker].second.push_back(
                                                          make_pair(true,i));
                      currentWorker = (currentWorker+1) % workers.size();
                   }  else { // only file creation
                      ConnectionInfo* info = 
                              algInstance->getWorkerConnection(elem,pi->dbname);
                      assert(pos.find(info)!=pos.end());
                      int rw = pos[info];
                      responsible[rw].second.push_back(make_pair(false,i));
                   }
               }
            }

            vector<retrieveClass*> runners;
            for(size_t i=0;i<responsible.size();i++){
                runners.push_back(new retrieveClass(i, this));
            }       

            for(size_t i=0;i<runners.size();i++){
                delete runners[i];
            }
        }


     private:
        dproductInfo* pi;
        vector<pair<ConnectionInfo*,vector<pair<bool,size_t > > > > responsible;
        vector<pair<bool, string> > slots;
      

        class retrieveClass{ // class doing the real data transfer
                            //  for a single worker
         public:
           retrieveClass(int _index, copyRunner* _parent): 
                index(_index), parent(_parent){
                runner = new boost::thread(&retrieveClass::run, this);
           }
           ~retrieveClass(){
              runner->join();
              delete runner;
           }

         private:
           int index;
           copyRunner* parent;
           boost::thread* runner;

           void run(){
              pair<ConnectionInfo*, 
                  vector<pair<bool,size_t > > >& p = parent->responsible[index];
              ConnectionInfo* target = p.first;
              vector< pair<bool,size_t> >& slots = p.second;
              for(size_t i=0;i<slots.size(); i++){
                 if(slots[i].first){
                    transferFile(target, slots[i].second);
                 }
              } 
           }

           void transferFile(ConnectionInfo* target, int slot){
               DArrayBase* a = parent->pi->arg1;
               DArrayElement elem = a->getWorkerForSlot(slot);
               ConnectionInfo* source = 
                        algInstance->getWorkerConnection(elem, 
                                                       parent->pi->dbname);
               string sourceName = a->getFilePath(source->getSecondoHome(),
                                           parent->pi->dbname, slot);
               string targetName = a->getFilePath(parent->slots[slot].second, 
                                           parent->pi->dbname, slot);

               string cmd =    "query getFileTCP('" + sourceName+"', '" 
                             + source->getHost()+"', " 
                             + stringutils::int2str(parent->pi->port)
                             + ", FALSE, '" + targetName+"')";
               performCommand(target, cmd);
           } 



        }; 


   };


   class fileCreator{
     public:
        fileCreator(DArrayBase* _a, ConnectionInfo* _ci,
                    vector<int>* _s, string& _db): 
            a(_a), ci(_ci), s(_s), dbname(_db){
            runner = new boost::thread(&fileCreator::run, this);
        }
        ~fileCreator(){
            runner->join();
            delete runner;
        }
      private:
           DArrayBase* a;
           ConnectionInfo* ci;
           vector<int>* s;
           string dbname;
           boost::thread* runner;

            void run(){
               for(size_t i=0;i<s->size();i++){
                  createFile((*s)[i]);
               }
            }

            void createFile(int slot){
               string oname = a->getObjectNameForSlot(slot);
               string fname = a->getFilePath(ci->getSecondoHome(),dbname,slot);
               string cmd = "query " + oname + " saveObjectToFile['"+fname+"']";
               int errCode;
               string errMsg;
               double rt;
               string res;
               ci->simpleCommand(cmd, errCode, errMsg, res, false, rt);
               if(errCode){
                  showError(ci, cmd, errCode, errMsg);
               }
            }

   };

   void createFilesFromDArray(DArrayBase* array){
      assert(array->getType()==DARRAY);
      map<ConnectionInfo*, vector<int> > resp;
      for(size_t i = 0; i< array->getSize(); i++){
         DArrayElement  elem= array->getWorkerForSlot(i);
         ConnectionInfo* ci = algInstance->getWorkerConnection(elem, dbname);
         typename map<ConnectionInfo*, vector<int> >::iterator it;
         it = resp.find(ci);
         if(it!=resp.end()){
            it->second.push_back(i);
         } else {
            vector<int> v;
            v.push_back(i);
            resp[ci] = v;
         }
      }
      vector<fileCreator*> runners;
      typename map<ConnectionInfo*, vector<int> >::iterator it;
      for(it = resp.begin(); it!=resp.end(); it++){
         runners.push_back(new fileCreator(arg1, it->first,
                                            &(it->second), dbname));          
      }
      for(size_t i=0;i<runners.size();i++){
         delete runners[i];
      }
   }

     copyHelper* copySlots(){
       

       // we assume that du to the  parallel access to the
       //  single slots of argument 2
       // it would be better to convert all slots into files
       if(arg1->getType()==DARRAY){
          createFilesFromDArray(arg1);
       }

       copyHelper* ch = new copyHelper();   
       // collect different ip adresses into a copyHelper object
       for(unsigned int i = 0; i< arg0->numOfWorkers(); i++){
           DArrayElement elem = arg0->getWorker(i);
           ConnectionInfo* target = 
                      algInstance->getWorkerConnection(elem, dbname);
           ch->add(target, arg1, dbname);
       }
       for(unsigned int i=0; i< arg1->numOfWorkers(); i++){
           DArrayElement elem = arg1->getWorker(i);
           ConnectionInfo* source = 
                       algInstance->getWorkerConnection(elem, dbname);
           ch->addAdditional(source);
       }

       vector<ConnectionInfo*>  workerSel;
       ch->getWorkerSelection(workerSel); // one worker for each ip
       startFileTransferators(workerSel, port);

      
       // transfer data in parallel
       copyHelper::iter it;
       vector<copyRunner*> cr;
       for(it  = ch->scan(); it!=ch->end(); it++){
          copyHelper::ipinfo inf = it->second;
          cr.push_back(new copyRunner(inf, this));   
       }
       for(size_t i=0;i<cr.size();i++){
          delete cr[i];
       }
       return ch;
    }

    void computeResult( copyHelper* ch ){
       // we start an resultrunner for each slot and wait for 
       // finishing of all of them


       vector<resultRunner*> runners;
       for(size_t i=0;i<arg0->getSize(); i++){
            resultRunner* runner = new resultRunner(this,i, ch);
            runners.push_back(runner); 
       }
       for(size_t i=0;i<runners.size();i++){
          delete runners[i];
       }
    }

    void removeTemp(copyHelper* ch){
       // temporary file are create for the second argument
       // if the second argument is a darray, we can  
       // remove the complete corresponding directory on the workers
       // otherwise we have to select the files which have been copied
       // from other IP adresses
       if(arg1->getType()==DARRAY){
           removeDirs(ch);
       } else {
           removeFiles(ch);
       }
    }

    void removeDirs(copyHelper* ch){
       // we build a vector of pairs consisting of the ConnectionInfo
       // and the directory which is to remove
       copyHelper::iter it;
       vector<pair<ConnectionInfo*, string> > v;
       for(it = ch->scan(); it!=ch->end(); it++){ // go through ip adresses
           set<string> usedHomes;
           vector<ConnectionInfo*>& cis = it->second.first;
           for(size_t i=0;i<cis.size();i++){
              ConnectionInfo* ci = cis[i];
              string home = ci->getSecondoHome();
              if(usedHomes.find(home)==usedHomes.end()){
                  usedHomes.insert(home);
                  string s0 = arg1->getFilePath(home, dbname, 0);
                  v.push_back(make_pair(ci, FileSystem::GetParentFolder(s0)));
              }
           }    
       }
       vector<dirRemover*> runners;
       for(size_t i=0;i<v.size();i++){
          runners.push_back(new dirRemover(v[i].first, v[i].second));
       }
       for(size_t i=0;i<v.size();i++){
          delete runners[i];
       }
    }

    void removeFiles(copyHelper* ch){
       vector< pair<ConnectionInfo*, vector<string> > > files;
       // we collect the files which should be removed   
       copyHelper::iter it;
       for(it = ch->scan(); it!=ch->end(); it++){ //go through ip adresses
           vector<ConnectionInfo*>& cis = it->second.first;
           vector<string> filenames;
           vector< pair<bool, string> >& homes = it->second.second;
           for(size_t i=0;i<homes.size(); i++){
               if(homes[i].first){
                  string f = arg1->getFilePath(homes[i].second,dbname,i);
                  filenames.push_back(f);
               }
           }
           // assign to each connection info in cis a set of filenames
           size_t number = filenames.size()/cis.size(); 
           number = number>0?number:1;
           size_t pos = 0;
           for( size_t i=0; i< cis.size() && pos < filenames.size(); i++){
               vector<string> v;
               size_t cp = pos;
               while(cp < filenames.size() && cp < pos + number){
                   v.push_back(filenames[cp]);
                   cp++;
               }
               pos = cp; 
               files.push_back(make_pair(cis[i],v));
           }
          // assert(pos == filenames.size()); 
         //  TODO: think about why this is wrong
       }


       // send command for removing files
       vector<fileRemover*> runners;
       for(size_t i=0; i< files.size(); i++){
           runners.push_back(new fileRemover(files[i].first, files[i].second));
       }
       for(size_t i=0; i< files.size(); i++){
            delete runners[i];
       }
    }

    class fileRemover{
       public:
           fileRemover(ConnectionInfo* _ci, vector<string>& _files):
               ci(_ci), files(_files){
              runner = new boost::thread(&fileRemover::run, this);
           }
           ~fileRemover(){
               runner->join();
               delete runner;
            }
       private:
           ConnectionInfo* ci;
           vector<string> files;
           boost::thread* runner;

           void run(){
              for(size_t i= 0; i< files.size(); i++){
                 string cmd = "query removeFile('"+files[i]+"')";
                 performCommand(ci, cmd);
              }
           }

    };


    class dirRemover{
       public:
          dirRemover(ConnectionInfo* _ci, string& _dir): ci(_ci), dir(_dir){
             runner = new boost::thread(&dirRemover::run, this);
             cout << "remove " << dir << " on " << ci->getHost() << endl;
          }
          ~dirRemover(){
              runner->join();
              delete runner;
          }

       private:
           ConnectionInfo* ci;
           string dir;
           boost::thread* runner;
           void run(){
              string cmd = "query removeDirectory('"+dir+"', TRUE)";
              performCommand(ci,cmd); 
           }
    };



    class resultRunner{
        public:
           resultRunner(dproductInfo* _pi, int _slot, copyHelper* _ch): 
             pi(_pi), slot(_slot), ch(_ch){
             runner = new boost::thread(&resultRunner::run, this);
           }

           ~resultRunner(){
               runner->join();
               delete runner;
           }
        private:
           dproductInfo* pi;
           int slot;
           copyHelper* ch;
           boost::thread* runner;


           void run(){
              string funarg1;
              DArrayElement w0 = pi->arg0->getWorkerForSlot(slot);
              ConnectionInfo* c0;
              c0 = algInstance->getWorkerConnection(w0, pi->dbname);
              if(pi->arg0->getType()==DARRAY){
                 funarg1 =  pi->arg0->getObjectNameForSlot(slot);
              } else {
                 funarg1 = "(" +
                           nl->ToString(
                               nl->TwoElemList(listutils::basicSymbol<frel>(),
                               nl->Second(nl->Second(pi->a0Type))))
                           + "'" 
                           + pi->arg0->getFilePath(c0->getSecondoHome(), 
                                                   pi->dbname, slot) 
                           + "')";
                                        
              }
              string funarg2;
              // funarg2 consists of the concatenation of all slots of argument1
              funarg2 = "("   + nl->ToString( nl->TwoElemList(
                                     listutils::basicSymbol<fsrel>(),
                                     nl->Second(nl->Second(pi->a1Type))) )
                       +"( ";
              for(size_t i=0;i<pi->arg1->getSize(); i++){
                 string slotFile = getSlotFile(pi->arg0->getWorkerForSlot(slot),
                                            i, c0, ch);
                   funarg2 += " " + slotFile;
              }
              funarg2 += "))";

              string funcall = "( " + pi->funText+ " " + funarg1 + " " 
                               + funarg2 + ")";

              string cmd;
              if(pi->result->getType()==DARRAY){
                 cmd =   "(let " + pi->result->getObjectNameForSlot(slot)
                       + " =  " + funcall + " )";
              }  else {
                 cmd =   "(query (count (fconsume5 " + funcall + "'" 
                      + pi->result->getFilePath(c0->getSecondoHome(), 
                                                pi->dbname, slot) 
                      + "' )))";
              }
              int ec;
              string errMsg;
              double rt;
              string resList;
              c0->simpleCommandFromList(cmd, ec, errMsg, resList, false, rt);
              if(ec){
                  cout << __FILE__ << "@" << __LINE__ << endl;
                  showError(c0,cmd,ec,errMsg);
              }
           }

           string getSlotFile(DArrayElement target, size_t slot, 
                             ConnectionInfo* c0, copyHelper* sh ){
              string dir = ch->getInfo(target.getHost())->second[slot].second;
              DArrayElement source = pi->arg1->getWorkerForSlot(slot);
              return  "'" + pi->arg1->getFilePath(dir , pi->dbname, slot) + "'";
           }


    };




};


template<class A1, class A2, class R>
int dproductVMT( Word* args, Word& result, int message,
                  Word& local, Supplier s ){
  
   A1* a1 = (A1*) args[0].addr;
   A2* a2 = (A2*) args[1].addr;
   CcString* ccname = (CcString*) args[2].addr;
   // ignore fun, use funtext
   CcInt* ccport = (CcInt*) args[4].addr; 
   string fun  = ((FText*) args[5].addr)->GetValue();
   bool isStream = ((CcBool*) args[6].addr)->GetValue();

   result = qp->ResultStorage(s);
   R* res = (R*) result.addr;
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();

   if(!ccport->IsDefined()){
      res->makeUndefined();
      return 0;
   }
   int port = ccport->GetValue();
   if(port <=0){
      res->makeUndefined();
      return 0;
   }
   string name = "";
   if(ccname->IsDefined()){
     name = ccname->GetValue();
   }
   if(a1->numOfWorkers()<1 || a2->numOfWorkers() < 1){
      res->makeUndefined();
      return 0;
   }
   if(name==""){
      algInstance->getWorkerConnection(a1->getWorker(0),dbname);
      name = algInstance->getTempName();            
   }
   if(!stringutils::isIdent(name)){
      // name is invalid
      res->makeUndefined();
      return 0;
   }
   (*res) = (*a1);
   res->setName(name);

   dproductInfo info(a1, a2, res, fun, qp->GetType(qp->GetSon(s,0)),
                     qp->GetType(qp->GetSon(s,1)), port, isStream);

   info.run();
  
   return 0; 

}


ValueMapping dproductVM[] = {
   dproductVMT<DArray,DArray,DArray>,
   dproductVMT<DArray,DArray,DFArray>,
   dproductVMT<DArray,DFArray,DArray>,
   dproductVMT<DArray,DFArray,DFArray>,
   dproductVMT<DFArray,DArray,DArray>,
   dproductVMT<DFArray,DArray,DFArray>,
   dproductVMT<DFArray,DFArray,DArray>,
   dproductVMT<DFArray,DFArray,DFArray>,
};

int dproductSelect(ListExpr args){

  int a1 = DArray::checkType(nl->First(args))?0:4;
  int a2 = DArray::checkType(nl->Second(args))?0:2;
  ListExpr fr = nl->Fourth(nl->Fourth(args));
  int a3 = Relation::checkType(fr) || listutils::isStream(fr)?1:0;
  return a1+a2+a3;

}


OperatorSpec dproductSpec(
  "d[f]array(rel T1)  x d[f]array(rel T2) x string x "
  "( stream(T1) x stream(T2) -> K) x int -> d[f]array(K)",
  "_ _ dproduct[_,_,_] ",
  "Connects the slots of the first argument with all slots of the "
  "second argument using the specified function." 
  "The string argument is the name of the result, if empty or undefined, "
  "a name is generated automatically. "
  "The int argument specifies a port that is used for transferring data "
  "between workers.",
  "query dplz1 dplz2[ \"\", . .. hashjoin[P1,P2] , 1238] "
);


Operator dproductOp(
  "dproduct",
  dproductSpec.getStr(),
  8,
  dproductVM,
  dproductSelect,
  dproductTM
);


/*
Operator arraytypestream

A type mapping operator.

*/


template<int pos>
ListExpr arraytypeStreamTMT(ListExpr args){
  if(!nl->HasMinLength(args,pos)){
    return listutils::typeError("too less arguments");
  }
  for(int i=1;i<pos;i++){
     args = nl->Rest(args);
  }
  ListExpr arg = nl->First(args);
  if(!DArray::checkType(arg) && !DFArray::checkType(arg)){
    return listutils::typeError("Argument number " + stringutils::int2str(pos) 
                                 + " has to be a d[f]array");
  } 
  ListExpr subtype = nl->Second(arg);
  if(!Relation::checkType(subtype)){
    return listutils::typeError("subtype of d[f]array is not a relation");
  }
  return nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                          nl->Second(subtype));

}

OperatorSpec arraytypeStream1Spec(
  "d[f]array(rel X) x ... -> stream(X)",
  "arraytypeStream1(_)",
  "Type Mapping operator",
  "query dfa1 dfa2 dproductspec[\"\", . . product , 1238" 
);


OperatorSpec arraytypeStream2Spec(
  " . x d[f]array(rel X) x ... -> stream(X)",
  "arraytypeStream2(_)",
  "Type Mapping operator",
  "query dfa1 dfa2 dproductspec[\"\", . . product , 1238" 
);

Operator arraytypeStream1Op(
  "arraytypeStream1",
  arraytypeStream1Spec.getStr(),
  0,
  Operator::SimpleSelect,
  arraytypeStreamTMT<1>
);

Operator arraytypeStream2Op(
  "arraytypeStream2",
  arraytypeStream1Spec.getStr(),
  0,
  Operator::SimpleSelect,
  arraytypeStreamTMT<2>
);


/*
Type Map Operator subtype

*/
template<int pos1, int pos2 >
ListExpr SUBTYPETM(ListExpr args){

   if(!nl->HasMinLength(args,pos1)){
     return listutils::typeError("too less arguments");
   }
   for(int i=1;i<pos1;i++){
     args = nl->Rest(args);
   }
   ListExpr arg = nl->First(args);

   for(int i=0;i<pos2;i++){
      if(!nl->HasLength(arg,2)){
         return listutils::typeError("no subtype found");
      }
      arg = nl->Second(arg);
   }
   return arg; 
  
}


OperatorSpec SUBTYPE1SPEC(
  " t(st) x ... -> st",
  "SUBTYPE1SPEC(_)",
  "Type mapping operator.",
  "not for direct usage"
);

OperatorSpec SUBSUBTYPE1SPEC(
  " t(st1(st2)) x ... -> st2",
  "SUBSUBTYPE1SPEC(_)",
  "Type mapping operator.",
  "not for direct usage"
);

OperatorSpec SUBTYPE2SPEC(
  " a x t(st) x ... -> st",
  "SUBTYPE2SPEC(_,_)",
  "Type mapping operator.",
  "not for direct usage"
);

Operator SUBTYPE1OP(
  "SUBTYPE1",
   SUBTYPE1SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   SUBTYPETM<1,1>
);

Operator SUBSUBTYPE1OP(
  "SUBSUBTYPE1",
   SUBTYPE1SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   SUBTYPETM<1,2>
);

Operator SUBTYPE2OP(
  "SUBTYPE2",
   SUBTYPE2SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   SUBTYPETM<2,1>
);


/*
TypeMapOperators ARRAYFUNARG1, ARRAYFUNARG2 up to ARRAYFUNARG8

*/
template<int pos, bool makeFS>
ListExpr ARRAYFUNARG(ListExpr args){

  if(!nl->HasMinLength(args,pos)){
    return listutils::typeError("too less arguments");
  }
  for(int i=1;i<pos;i++){
    args = nl->Rest(args);
  }
  ListExpr arg = nl->First(args);

  if(DArray::checkType(arg)){
     return nl->Second(arg);
  }

  if(DFArray::checkType(arg) ||
     DFMatrix::checkType(arg)){
     ListExpr res;
     if(makeFS){
       res  = nl->TwoElemList(
               listutils::basicSymbol<fsrel>(),
               nl->Second(nl->Second(arg)));
      } else {
       res  = nl->TwoElemList(
               listutils::basicSymbol<frel>(),
               nl->Second(nl->Second(arg)));
      }
      return res;

  }
  return listutils::typeError("Invalid type found");
}

OperatorSpec ARRAYFUNARG1SPEC(
  "darray(X) x ... -> X, dfarray(rel(X)) x ... -> frel(X)",
  "ARRAYFUNARG1(_)",
  "Type mapping operator.",
  "query df1 dmap [\"df3\" . count]"
);

Operator ARRAYFUNARG1OP(
  "ARRAYFUNARG1",
   ARRAYFUNARG1SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   ARRAYFUNARG<1, false>
);

OperatorSpec ARRAYFUNARG2SPEC(
  " any x darray(X) x ... -> X, any x dfarray(rel(X)) x ... -> frel(X)",
  " ARRAYFUNARG2(_)",
  "Type mapping operator.",
  "query df1 df2 dmap2 [\"df3\" . feed  .. feed  product, 1238]"
);
Operator ARRAYFUNARG2OP(
  "ARRAYFUNARG2",
   ARRAYFUNARG2SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   ARRAYFUNARG<2, false>
);

OperatorSpec ARRAYFUNARG3SPEC(
  "darray(X) x ... -> X, dfarray(rel(X)) x ... -> frel(X)",
  "ARRAYFUNARG3(_,_,_)",
  "Type mapping operator.",
  "query df1 df2 df3 dmap3 [\"df9\" . count]"
);

Operator ARRAYFUNARG3OP(
  "ARRAYFUNARG3",
   ARRAYFUNARG3SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   ARRAYFUNARG<3, false>
);
OperatorSpec ARRAYFUNARG4SPEC(
  "darray(X) x ... -> X, dfarray(rel(X)) x ... -> frel(X)",
  "ARRAYFUNARG4(_,_,_,_)",
  "Type mapping operator.",
  "query df1 df2 df3 df4 dmap4 [\"df9\" . count]"
);

Operator ARRAYFUNARG4OP(
  "ARRAYFUNARG4",
   ARRAYFUNARG4SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   ARRAYFUNARG<4, false>
);
OperatorSpec ARRAYFUNARG5SPEC(
  "darray(X) x ... -> X, dfarray(rel(X)) x ... -> frel(X)",
  "ARRAYFUNARG5(_,_,_,_,_)",
  "Type mapping operator.",
  "query df1 df2 df3 df4 df5 dmap5 [\"df9\" . count]"
);

Operator ARRAYFUNARG5OP(
  "ARRAYFUNARG5",
   ARRAYFUNARG5SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   ARRAYFUNARG<5, false>
);

OperatorSpec ARRAYFUNARG6SPEC(
  "darray(X) x ... -> X, dfarray(rel(X)) x ... -> frel(X)",
  "ARRAYFUNARG6(_,_,_,_,_,_)",
  "Type mapping operator.",
  "query df1 df2 df3 df4 df5 df5 dmap6 [\"df9\" . count]"
);

Operator ARRAYFUNARG6OP(
  "ARRAYFUNARG6",
   ARRAYFUNARG6SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   ARRAYFUNARG<6, false>
);

OperatorSpec ARRAYFUNARG7SPEC(
  "darray(X) x ... -> X, dfarray(rel(X)) x ... -> frel(X)",
  "ARRAYFUNARG7(_,_,_,_,_,_,_)",
  "Type mapping operator.",
  "query df1 df2 df3 df4 df5 df6 df7 dmap7 [\"df9\" . count]"
);

Operator ARRAYFUNARG7OP(
  "ARRAYFUNARG7",
   ARRAYFUNARG7SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   ARRAYFUNARG<7, false>
);

OperatorSpec ARRAYFUNARG8SPEC(
  "darray(X) x ... -> X, dfarray(rel(X)) x ... -> frel(X)",
  "ARRAYFUNARG8(_)",
  "Type mapping operator.",
  "query df1 df2 df3 df4 df5 df6 df7 df8 dmap8 [\"df3\" . count]"
);

Operator ARRAYFUNARG8OP(
  "ARRAYFUNARG8",
   ARRAYFUNARG8SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   ARRAYFUNARG<8, false>
);

OperatorSpec AREDUCEARG1SPEC(
  "darray(X) x ... -> X, dfarray(rel(X)) x ... -> stream(X)",
  "AREDUCEARG1(_)",
  "Type mapping operator.",
  "query df1  areduce[\"d3\" . count , 1238]"
);

Operator AREDUCEARG1OP(
  "AREDUCEARG1",
   AREDUCEARG1SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   ARRAYFUNARG<1, true>
);

OperatorSpec AREDUCEARG2SPEC(
  " any x darray(X) x ... -> X, any x dfarray(rel(X)) x ... -> stream(X)",
  " AREDUCEARG2(_)",
  "Type mapping operator.",
  "query df1 df2 areduce[\"df3\" . .. product, 1238]"
);

Operator AREDUCEARG2OP(
  "AREDUCEARG2",
   AREDUCEARG2SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   ARRAYFUNARG<2, true>
);

ListExpr DFARRAYTUPLETM(ListExpr args){

  if(!nl->HasMinLength(args,1)){
    return listutils::typeError("at least one arg required");
  }
  if(!DFArray::checkType(nl->First(args))){
    return listutils::typeError("first arg must be of type dfarray");
  }
  return nl->Second(nl->Second(nl->First(args)));
}


OperatorSpec DFARRAYTUPLESPEC(
  "dfarray((rel(X)) x ... -> X",
  "DFARRAYTUPLE1(_)",
  "Type mapping operator.",
  "query s7 partition[hashvlaue(.,23], \"m7\"]"
);


Operator DFARRAYTUPLEOP(
  "DFARRAYTUPLE",
   DFARRAYTUPLESPEC.getStr(),
   0,
   Operator::SimpleSelect,
   DFARRAYTUPLETM
);

/*
4. File Transfer between workers

The following code implements the filetransfer between different workers 
without using the master. For that purpose, an operator is provided which
creates a server and waits for the request of a client. If a client is 
connected, the client may either send a file to this server or request a
file from the server. The client itselfs is also created by an operator.

In the first implementation, the server allowes only a single client to 
connect. After transferring a single file, the connection is terminated.

If the overhead for creating a server is big, this will be canged in the
future.

*/

class FileTransferKeywords{
 public:
 static string FileTransferServer(){ return "<FILETRANSFERSERVER>";}
 static string FileTransferClient(){ return "<FILETRANSFERCLIENT>";}
 static string SendFile(){ return "<SENDFILE>";}
 static string ReceiveFile(){ return "<RECEIVEFILE>";}
 static string Data(){ return "<DATA>";}
 static string EndData(){ return "</DATA>";}
 static string FileNotFound(){ return "<FILENOTFOUND>";}
 static string Cancel(){ return "<CANCEL>";}
 static string OK(){ return "<OK>";}

 static bool isBool(string s){
     stringutils::toLower(s);
     return (s=="true") || (s=="false");
 }

 static bool getBool(string s){
     stringutils::toLower(s);
     return s=="true";
 }

};


class FileTransferServer{

  public:
    FileTransferServer(int _port): port(_port), listener(0), server(0){}

    ~FileTransferServer(){
       if(server){
         server->ShutDown();
         delete server;
       }
       if(listener){
         listener->ShutDown();
         delete listener;
       }
     }

    int start(){
       listener = Socket::CreateGlobal("localhost", stringutils::int2str(port));
       if(!listener->IsOk()){
            return 1;
       }
       server = listener->Accept();
       if(!server->IsOk()){
          return 2;
       }
       return communicate();
    }

  private:
     int port;
     Socket* listener;
     Socket* server;

     int communicate(){
       try{
          iostream& io = server->GetSocketStream(); 
          io << FileTransferKeywords::FileTransferServer() << endl;
          io.flush();
          string line;
          getline(io,line);
          if(line==FileTransferKeywords::Cancel()){
             return true;
          }
          if(line!=FileTransferKeywords::FileTransferClient()){ 
            cerr << "Protocol error" << endl;
            return 3;
          }
          getline(io, line);
          if(line==FileTransferKeywords::SendFile()){
            return sendFile(io);
          } else if(line==FileTransferKeywords::ReceiveFile()){
            return receiveFile(io);
          } else {
             cerr << "protocol error" << endl;
             return 4;
          }
       }catch(...){
          cerr << "Exception in server occured during communination" << endl;
          return 5;
       }
     }

     int sendFile(iostream& io) {
        // client ask for a file
        string filename;
        getline(io,filename);
        ifstream in(filename.c_str(), ios::binary);
        if(!in){
           io << FileTransferKeywords::FileNotFound() << endl;
           io.flush();
           return 6;
        }
        in.seekg(0,in.end);
        size_t length = in.tellg();
        in.seekg(0,in.beg);
        io << FileTransferKeywords::Data() << endl;
        io << stringutils::any2str(length) << endl;
        io.flush();
        size_t bufsize = 1048576;
        char buffer[bufsize];
        while(!in.eof() && in.good()){
            in.read(buffer, bufsize);
            size_t r = in.gcount();
            io.write(buffer, r);
        }
        in.close();
        io << FileTransferKeywords::EndData() << endl;
        io.flush();
        return 0;
     }

     bool receiveFile(iostream& io){
        // not implemented yet
        return 7;
     }

};


class FileTransferClient{

  public:

     FileTransferClient(string& _server, int _port, bool _receive, 
                        string& _localName, string& _remoteName):
        server(_server), port(_port), receive(_receive), localName(_localName),
        remoteName(_remoteName){
     }

     ~FileTransferClient(){
         if(socket){
           socket->ShutDown();
           delete socket;
         }
      }
     int start(){
        socket = Socket::Connect(server, stringutils::int2str(port), 
                                 Socket::SockGlobalDomain, 3, 1);
        if(!socket){
           return 8;
        }
        if(!socket->IsOk()){
          return 9;
        }
        if(receive){
           return receiveFile();
        } else {
           return sendFile();
        }
     }

    private:
      string server;
      int port;
      bool receive;
      string localName;
      string remoteName;
      Socket* socket;
    

      int sendFile(){
          return 10; // not implemented yet
      }

      int receiveFile(){
         iostream& io = socket->GetSocketStream();
         string line;
         getline(io,line);
         if(line!=FileTransferKeywords::FileTransferServer()){
            return 11;
         }
         io << FileTransferKeywords::FileTransferClient() << endl;
         io << FileTransferKeywords::SendFile() << endl;
         io << remoteName << endl;
         io.flush();
         getline(io,line);
         if(line==FileTransferKeywords::FileNotFound()){
            return 12;
         }
         if(line!=FileTransferKeywords::Data()){
            return 13;
         }
         getline(io,line);
         bool ok;
         size_t t = stringutils::str2int<size_t>(line,ok);
   
         if(!ok || t<1){
           return 14;
         }

         size_t bufsize = 4096;
         char buffer[bufsize]; 

         // read in data
         ofstream out(localName.c_str(),ios::binary|ios::trunc);
         while(t>0){
             size_t s = min(t,bufsize);
             if(!io.read(buffer, s)){
                return 15;
             }
             t -= s;
             out.write(buffer,s);
         }
         out.close();
         getline(io,line);
         if(line!=FileTransferKeywords::EndData()){
             return 16;
         } 
         return 0;
      }
};

/*
4.1 Operator ~fileTransferServer~

*/

ListExpr fileTransferServerTM(ListExpr args){

  string err = "int expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(!CcInt::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcBool>();
}


int fileTransferServerVM(Word* args, Word& result, int message,
            Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  CcInt* Port = (CcInt*) args[0].addr;
  CcBool* res = (CcBool*) result.addr;
  if(!Port->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  int port = Port->GetValue();
  if(port<=0){
     res->Set(true,false);
     return 0;
  }
  FileTransferServer server(port);
  res->Set(true,server.start()==0);
  return 0;
}


OperatorSpec fileTransferServerSpec(
  " int -> bool)",
  " fileTransferServer(_)",
  "Starts a server waiting for a client requesting a file from "
  "this server. See also operator receiveFileClient. These operators "
  "are used internally for the transferFile operator. There is no "
  "requirement to use it directly.",
  "query fileTransferServer(1238)"
);


Operator fileTransferServerOP(
  "fileTransferServer",
   fileTransferServerSpec.getStr(),
   fileTransferServerVM,
   Operator::SimpleSelect,
   fileTransferServerTM
);

/*
4.2 operator ~receiveFileClient~

*/

bool isTextOrString(ListExpr a){
  return CcString::checkType(a) || FText::checkType(a);
}

ListExpr receiveFileClientTM(ListExpr args){
  string err = "{text,string} x int x {text,string} x {text,string} expected";
  if(!nl->HasLength(args,4)){
   return listutils::typeError(err + " (wrong number of arguments)");
  }
  ListExpr server = nl->First(args);
  ListExpr port = nl->Second(args);
  ListExpr remote = nl->Third(args);
  ListExpr local = nl->Fourth(args);

  if(    !isTextOrString(server)
      || !CcInt::checkType(port) 
      || !isTextOrString(remote)
      || !isTextOrString(local)){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcBool>();
}


template<class S, class R, class L>
int receiveFileClientVMT(Word* args, Word& result, int message,
            Word& local, Supplier s ){

   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   S* Server = (S*) args[0].addr;
   CcInt* Port = (CcInt*) args[1].addr;
   R* Remote = (R*) args[2].addr;
   L* Local = (L*) args[3].addr;
 
   if(     !Server->IsDefined() || !Port->IsDefined() 
        || !Remote->IsDefined() || !Local->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   string server = Server->GetValue();
   int port = Port->GetValue();
   string remoteName = Remote->GetValue();
   string localName = Local->GetValue();

   if(port <=0){
      res->SetDefined(false);
      return 0;
   }
 
  FileTransferClient client(server, port,true,  localName, remoteName);
  int code = client.start();

  res->Set(true,code==0); 
  return 0;
}


ValueMapping receiveFileClientVM[] = {
   receiveFileClientVMT<CcString, CcString, CcString>,
   receiveFileClientVMT<CcString, CcString, FText>,
   receiveFileClientVMT<CcString, FText, CcString>,
   receiveFileClientVMT<CcString, FText, FText>,
   receiveFileClientVMT<FText, CcString, CcString>,
   receiveFileClientVMT<FText, CcString, FText>,
   receiveFileClientVMT<FText, FText, CcString>,
   receiveFileClientVMT<FText, FText, FText>
  };

int receiveFileClientSelect(ListExpr args){

  int s = CcString::checkType(nl->First(args))?0:4;
  int r = CcString::checkType(nl->Third(args))?0:2;
  int l = CcString::checkType(nl->Fourth(args))?0:1;
  return s+r+l;
}


OperatorSpec receiveFileClientSpec(
  " {string, text} x int x {string,text} x {string,text} -> bool",
  "receiveFileClient(server, port, remoteFile, localFile)",
  "Copies a file from a remote server to the local file system. "
  "On the remote host, a query using transferFileServer should be "
  "started. The operator is for intern usage, e.g., for the transferFile "
  "operator.",
  "query recieveFileClient('server', 1238,'remote.txt', 'local.txt')"
);

Operator recieveFileClientOP(
  "receiveFileClient",
  receiveFileClientSpec.getStr(),
  8,
  receiveFileClientVM,
  receiveFileClientSelect,
  receiveFileClientTM
);


/*
4.4 Operator ~transferFile~

This operator copies a file from one connected worker to another
one using a tcp connection. 

*/

ListExpr transferFileTM(ListExpr args){
  string err ="int x int x int x {string,text} x {string,text} expected";
  // worker1 worker2 port <file on w1> <file on w2>
  if(!nl->HasLength(args,5)){
    return listutils::typeError(err + " (wrong number of argumnents)");
  }
  if(   !CcInt::checkType(nl->First(args))
     || !CcInt::checkType(nl->Second(args))
     || !CcInt::checkType(nl->Third(args))
     || !isTextOrString(nl->Fourth(args))
     || !isTextOrString(nl->Fifth(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcBool>();
}


class FileTransferServerStarter{

 public:
    FileTransferServerStarter(ConnectionInfo* _ci,
                int _port): ci(_ci),port(_port)//,error(0)
               {} 


    ~FileTransferServerStarter(){
        cancel();
        runner.join();
     }


    void start(){
       runner = boost::thread(&FileTransferServerStarter::run,this);
    }
     


 private:
    ConnectionInfo* ci;
    int port;
    //int error;
    boost::thread runner;


    void run(){
        string serverQuery = "query fileTransferServer("
                             +stringutils::int2str(port)+")";
        int err;
        string errMsg;
        string result;
        double runtime;
        ci->simpleCommand(serverQuery,err,errMsg,result,false, runtime);
    }

    void cancel(){
       if(!runner.timed_join(boost::posix_time::seconds(0))){
          sendCancel();
       }
    }

    void sendCancel(){
        Socket* socket = Socket::Connect(ci->getHost(), 
                                 stringutils::int2str(port), 
                                 Socket::SockGlobalDomain, 3, 1);
        if(!socket){
           return;
        }
        if(!socket->IsOk()){
          return;
        }
        iostream& io = socket->GetSocketStream();
        string line;
        getline(io,line); // should be <FILETRANSFERSERVER>
        io << FileTransferKeywords::Cancel() << endl;
        socket->ShutDown();
        delete socket;
    }    

};



template<class S, class T>
int transferFileVMT(Word* args, Word& result, int message,
            Word& local, Supplier s ){

  result=qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  CcInt* Server1 = (CcInt*) args[0].addr;
  CcInt* Server2 =  (CcInt*) args[1].addr;
  CcInt* Port = (CcInt*) args[2].addr;
  S* SrcFile = (S*) args[3].addr;
  T* TargetFile = (T*) args[4].addr;

  if(    !Server1->IsDefined() || !Server2->IsDefined() || !Port->IsDefined()
      || !SrcFile->IsDefined() || !TargetFile->IsDefined()){
    res->SetDefined(false);
    return 0;
  }  

  int server1 = Server1->GetValue();
  int server2 = Server2->GetValue();
  int port = Port->GetValue();
  string srcFile = SrcFile->GetValue();
  string targetFile = TargetFile->GetValue();

  if(    !algInstance->serverExists(server1)
      || !algInstance->serverExists(server2)
      || (port <=0)){
     res->Set(true,false);
     return 0;
  } 

  ConnectionInfo* ci1 = algInstance->getConnection(server1);
  ConnectionInfo* ci2 = algInstance->getConnection(server2); 

  assert(ci1);
  assert(ci2);


  if(server1==server2){
      // copying on a single server
      string q = "query copyFile('"+srcFile+"','"+targetFile+"')";
      int err;
      string errMsg;
      ListExpr resList;
      double runtime;
      ci1->simpleCommand(q,err,errMsg,resList,false, runtime);
      if(err!=0){
          cerr << __FILE__ << "@"  << __LINE__ << endl;
          showError(ci1,q,err,errMsg);
          res->Set(true,false);
          return 0;
      }
      if(   !nl->HasLength(resList,2) 
         || (nl->AtomType(nl->Second(resList))!=BoolType)){
         cerr << "unexpected result in " << q << endl;
         cerr << nl->ToString(resList) << endl;
         res->Set(true,false);
         return 0;
      }
      res->Set(true, nl->BoolValue(nl->Second(resList)));
      return 0;
  } 
 
  string clientQuery = "query receiveFileClient( '" + ci1->getHost() +"', "
                       + stringutils::int2str(port) +", '"
                       + srcFile +"', '" + targetFile+"')";

  FileTransferServerStarter starter(ci1,port);
  starter.start();

  int err;
  string errMsg;
  ListExpr resList;
  double runtime;
  ci2->simpleCommand(clientQuery, err,errMsg, resList, false, runtime);  
  if(err!=0){
     cerr << __FILE__ << "@"  << __LINE__ << endl;
     showError(ci2,clientQuery, err, errMsg);
     res->Set(true,false);
     return 0;
  } 
  if(    !nl->HasLength(resList,2) 
      || ((nl->AtomType(nl->Second(resList))!=BoolType))){
     cerr << "command " << clientQuery << " has unexpected result" << endl;
     cerr << nl->ToString(resList) << endl;
     res->Set(true,false);
     return 0;
  }
  res->Set(true, nl->BoolValue(nl->Second(resList)));
  return 0;
}


ValueMapping transferFileVM[] = {
   transferFileVMT<CcString, CcString>,
   transferFileVMT<CcString, FText>,
   transferFileVMT<FText, CcString>,
   transferFileVMT<FText, FText>
};

int transferFileSelect(ListExpr args){
  int r1 = CcString::checkType(nl->Fourth(args))?0:2;
  int r2 = CcString::checkType(nl->Fifth(args))?0:1;
  return r1+r2;
}

OperatorSpec transferFileSpec(
  "int x int x int x {string,text} x {string,text} -> bool",
  "transferFile( serverno1, serverno2, port, srcfile, targetfile)",
  "Transfers a file from a server to another one. "
  "The result shows the success of the operation. The servers must be "
  "connected before using connect or similar.",
  "query transferFile(0,1,1238,'Staedte.txt','Staedte3.txt')"
);

Operator transferFileOP(
   "transferFile",
   transferFileSpec.getStr(),
   4,
   transferFileVM,
   transferFileSelect,
   transferFileTM
);


/*
Operator ~traceCommands~

*/
ListExpr traceCommandsTM(ListExpr args){
  if(nl->HasLength(args,1) && CcBool::checkType(nl->First(args))){
    return listutils::basicSymbol<CcBool>();
  }
  return listutils::typeError("bool expected");
}


int traceCommandsVM(Word* args, Word& result, int message,
                    Word& local, Supplier s ){
   result = qp->ResultStorage(s);
   CcBool* arg = (CcBool*) args[0].addr;
   CcBool* res = (CcBool*) result.addr;
   if(arg->IsDefined()){
     showCommands = arg->GetValue();
     res->Set(true,true);
   } else {
     res->SetDefined(false);
   }
   return 0;
}

OperatorSpec traceCommandsSpec(
  "bool -> bool",
  "traceCommands(_)",
  "Enables or disables tracing of remote commands.",
  "query traceCommands(false)"
);


Operator traceCommandsOp(
  "traceCommands",
  traceCommandsSpec.getStr(),
  traceCommandsVM,
  Operator::SimpleSelect,
  traceCommandsTM
);

/*
Operator ~showProgress~

*/
ListExpr showProgressTM(ListExpr args){
  if(nl->HasLength(args,1) && CcBool::checkType(nl->First(args))){
    return listutils::basicSymbol<CcBool>();
  }
  return listutils::typeError("bool expected");
}

int showProgressVM(Word* args, Word& result, int message,
                    Word& local, Supplier s ){
   result = qp->ResultStorage(s);
   CcBool* arg = (CcBool*) args[0].addr;
   CcBool* res = (CcBool*) result.addr;
   if(arg->IsDefined()){
     algInstance->showProgress(arg->GetValue());
     res->Set(true,true);
   } else {
     res->SetDefined(false);
   }
   return 0;
}


OperatorSpec showProgressSpec(
  "bool -> bool",
  "showProgress(_)",
  "Enables or disables progress view for the prcmd command.",
  "query showProgress(false)"
);


Operator showProgressOp(
  "showProgress",
  showProgressSpec.getStr(),
  showProgressVM,
  Operator::SimpleSelect,
  showProgressTM
);


/*
3 static filetransfer

The following code can be used for creating a 
file reciever. This is a server accpting to receive a file 
from a client. After the server is startet. it works in 
parallel to running secondo queries. A server can be killed 
by another query. If so, no new commections are accepted. If the
last running transfer is done, the server is shut down.



*/
  
  // forward declaration

  

class staticFileTransferator;

map<int, staticFileTransferator*>  staticFileTransferators;


class staticFileTransferator{


   public:
      static staticFileTransferator* getInstance(int port, int& noTransfers){
          map<int, staticFileTransferator*>::iterator it;
          it = staticFileTransferators.find(port);
          if(   staticFileTransferators.find(port) 
             != staticFileTransferators.end()){
                staticFileTransferator* res = it->second;
                noTransfers = res->maxTransfers;
                return res;
          }
          staticFileTransferator* res = 
                   new staticFileTransferator(port,noTransfers);
          if(res->running){
             // able to listen at the specified port
             staticFileTransferators[port] = res;
             return res;
          }
          // some problems, e.g., security problems
          delete res;
          return 0;
      }

      static bool finishInstance(int port) {
         map<int,staticFileTransferator*>::iterator it;
         it = staticFileTransferators.find(port);
         if(it==staticFileTransferators.end()){ // there is no such receiver
            return false;
         }
         staticFileTransferator* k = it->second;
         staticFileTransferators.erase(it);
         delete k;
         return true;
      }

      ~staticFileTransferator(){
         running = false;

         set<transferator*>::iterator it;
         for(it = activeTransfers.begin();it!=activeTransfers.end();it++){
            delete *it;
         }

         listener->CancelAccept();
         listthread->join();
         delete listthread;
         
         delete listener;
         deleteFinishedTransfers();
            
      }

      void deleteFinishedTransfers(){
         boost::lock_guard<boost::recursive_mutex> guard(mtx);
         set<transferator*>::iterator it;
         for(it=finishedTransfers.begin(); it!=finishedTransfers.end();
             it++){
            delete *it;
         }
         finishedTransfers.clear();
      }


   private:
      class transferator;

      bool running;      // flag for activity
      int maxTransfers;  // maximum number of parallel transfers
      Socket* listener;  // the listener object
      boost::thread* listthread; // thread for listener
      vector<Socket*> connections; // active connections
      set<transferator*> activeTransfers;
      set<transferator*> finishedTransfers;
      boost::recursive_mutex mtx;  // synchronize access to transfer sets 


      staticFileTransferator(int port, int maxTransfers){
         listener = Socket::CreateGlobal("localhost", 
                                         stringutils::int2str(port));
         running = listener->IsOk();
         listthread = new boost::thread(&staticFileTransferator::listen , this);
      }


      void listen(){
        while(running){
           Socket* socket = listener->Accept();
           if(socket){
               if(!socket->IsOk()){
                  delete socket;
               } else{
                  addTransfer(socket);
               }
           }
        }
      }

      
      void addTransfer( Socket* socket){
         boost::lock_guard<boost::recursive_mutex> guard(mtx);
         connections.push_back(socket);
         activeTransfers.insert(new transferator(socket, 
                             connections.size()-1,this));
      }

      void finish(transferator* t, bool success){
          if(running){
             boost::lock_guard<boost::recursive_mutex> guard(mtx);
             activeTransfers.erase(t);
             finishedTransfers.insert(t);
          }
          if(!success){
             cerr << "File Transfer failed" << endl;
          } else {
             cerr << "File received correctly" << endl;
          }
      }

     class transferator{
       public:
          transferator(Socket* _socket, int _index, 
                       staticFileTransferator* _listener){
            socket = _socket;
            index = _index;
            listener = _listener;
            runner = new boost::thread(&transferator::transfer, this);
          }

          ~transferator(){
              runner->join();
              delete runner;
              delete socket;
          }


       private:
          Socket* socket;
          int index;
          staticFileTransferator* listener;
          boost::thread* runner;


          void join(){
             runner->join();
          }
          
          void transfer(){
            bool ok = true;
            try{
              iostream& io = socket->GetSocketStream();
              string line;
              getline(io,line);

              if(line!=FileTransferKeywords::ReceiveFile()
                 && line!=FileTransferKeywords::SendFile()){
                 ok = false; // protocol error
                 socket->Close();
                 listener->finish(this,false);
                  return;
              }
              if(line==FileTransferKeywords::ReceiveFile()){
                 receive(io,ok);
              } else {
                 send(io,ok); // not implemented yet
              }
              io.flush();
            } catch(...){
               ok = false;
            }
            socket->Close();
            listener->finish(this,ok);
          }

          void send(iostream& io, bool& ok){
              string filename;
              string line;
              getline(io,filename);
              ifstream in(filename.c_str(), ios::binary|ios::in);
              if(!in.good()){
                 io << FileTransferKeywords::Cancel() << endl;
                 ok = false;
                 return;  
              }
              io << FileTransferKeywords::OK() << endl;
              in.seekg(0,in.end);
              size_t length = in.tellg();
              in.seekg(0,in.beg);
              io << length << endl;
              io << FileTransferKeywords::Data() << endl;
              size_t bs = 1024*16;
              char buffer[bs];
              while(length>0 && io.good() && in.good()){
                 if(bs>length){
                     bs = length;
                 }
                 in.read(buffer, bs);
                 size_t r = in.gcount();
                 io.write(buffer,r);
                 length -=r;
              }
              ok = io.good();
              io << FileTransferKeywords::EndData() << endl;
          }

          void receive(iostream& io, bool& ok){
              string filename;
              string line;
              getline(io,filename);
              getline(io,line);
              if(!FileTransferKeywords::isBool(line)){
                ok = false;
                return;
              }
              bool overwrite = FileTransferKeywords::getBool(line);
              if(!overwrite){
                 if(FileSystem::FileOrFolderExists(filename)){
                    io << FileTransferKeywords::Cancel() << endl;
                    ok = false;
                    return;
                 }
              }
              ofstream out(filename.c_str(), ios::binary| ios::trunc);
              if(out.good()){
                 io << FileTransferKeywords::OK() << endl;
              }  else {
                 io << FileTransferKeywords::Cancel() << endl;
                 ok = false;
                 return;
               }
               size_t length=0;
               getline(io,line);
               length = stringutils::str2int<size_t>(line,ok);
               if(!ok) {
                 return;
               }
               getline(io,line);
               if(line!=FileTransferKeywords::Data()){
                    ok = false; // protocol error
                    return;
               }
               // get data
               size_t bs = 1024;
               char buffer[bs];
               while(length>0 &&!io.eof()){
                  if(bs > length){
                     bs = length;
                   } 
                   io.read(buffer,bs);
                   size_t r = io.gcount();
                   out.write(buffer,r);
                   length -= r;
                }
                ok = io.good() && out.good();
                out.close();
                getline(io,line);
                ok = line == FileTransferKeywords::EndData(); 
                if(ok){
                   io << FileTransferKeywords::OK() << endl;
                } else {
                   io << FileTransferKeywords::Cancel() << endl;
                }
                io.flush();
          }
     };
};


ListExpr staticFileTransferatorTM(ListExpr args){
  string err = "int x int expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcBool>();
}


int staticFileTransferatorVM(Word* args, Word& result, int message,
                    Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*)result.addr;
  CcInt*  port = (CcInt*)args[0].addr;
  CcInt*  count = (CcInt*) args[1].addr;
  if(!port->IsDefined() || !count->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  int p = port->GetValue();
  int c = count->GetValue();
  res->Set(true, staticFileTransferator::getInstance(p,c )!=0);
  return 0;
}



OperatorSpec staticFileTransferatorSpec(
  "int x int -> bool",
  "staticFileTransferator(port, maxConnections)",
  "Creates a server running in parallel to the rest "
  "of the SecondoSystem. It can handle several connections "
  "in parallel. If more than the specified number of connections"
  " is required, some connections go into a queue until a new "
  "slot is available. Aided by this server, files can be tranferred "
  "to and from this host.",
  "query staticFileTransferator(1238,10)"
);


Operator staticFileTransferatorOp(
  "staticFileTransferator",
  staticFileTransferatorSpec.getStr(),
  staticFileTransferatorVM,
  Operator::SimpleSelect,
  staticFileTransferatorTM
);





ListExpr killStaticFileTransferatorTM(ListExpr args){
  string err = "int expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcBool>();
}


int killStaticFileTransferatorVM(Word* args, Word& result, int message,
                    Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  CcInt* port = (CcInt*) args[0].addr;
  if(!port->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  res->Set(true, staticFileTransferator::finishInstance(port->GetValue()));  
  return 0;
}


OperatorSpec killStaticFileTransferatorSpec(
  "int -> bool",
  "killStaticFileTransferator(port)",
  "Finishes a running file transferator instance. "
  "Pending file transfers are finished before "
  "terminating the server. ",
  "query killStaticFileTransferator(1238)"
);
  

Operator killStaticFileTransferatorOp(
  "killStaticFileTransferator",
  killStaticFileTransferatorSpec.getStr(),
  killStaticFileTransferatorVM,
  Operator::SimpleSelect,
  killStaticFileTransferatorTM
);


/*
12.1 Operators ~putFileTCP~ amd ~getFileTCP~

This operator copies a file to a remote server on which a
staticFileTransferator is started before.

*/

ListExpr putOrGetFileTCPTM(ListExpr args){

   // localFile adress port overwrite remoteFile -> success
   string err = "{string,text} x {string,text} x int x bool x {string, text} "
                "expected";
   if(!nl->HasLength(args,5)){
      return listutils::typeError(err);
   }
   if(   !isTextOrString(nl->First(args))
      || !isTextOrString(nl->Second(args))
      || !CcInt::checkType(nl->Third(args))
      || !CcBool::checkType(nl->Fourth(args))
      || !isTextOrString(nl->Fifth(args))){
     return listutils::typeError(err);
   }
   return listutils::basicSymbol<CcBool>();
}


class fileCopy{
  public:
    fileCopy(const string& _local, const string& _host, 
             const int _port, const bool _overwrite,
              const string& _remote): local(_local),
             host(_host), port(_port), overwrite(_overwrite),
             remote(_remote){}


    bool send(string& errMsg){
      ifstream in(local.c_str(),ios::binary|ios::in);
      if(!in){
        errMsg = "Could not find local file";
        return false;
      }
      // determine length of the input stream
      in.seekg(0,in.end);
      size_t length = in.tellg();
      in.seekg(0,in.beg);
      

      // connect to server
      Socket* socket = Socket::Connect(host, stringutils::int2str(port), 
                              Socket::SockGlobalDomain, 3, 1);
      if(!socket){
        errMsg = "could not open connection";
        return false;
      }
      if(!socket->IsOk()){
        errMsg = "could not open connection";
        delete socket;
        return false;
      }
      iostream& io = socket->GetSocketStream();
      io << FileTransferKeywords::ReceiveFile() << endl;
      io << remote << endl;
      io << (overwrite?"true":"false") << endl;
      io.flush();
      string line;
      getline(io,line);
      if(line!=FileTransferKeywords::OK()){
         socket->Close();
         delete socket;
         errMsg = "Problem in creating remote file";
         in.close();
         return false;
      }
      io << length << endl;
      io << FileTransferKeywords::Data() << endl;
      size_t bufsize = 1048576;
      char buffer[bufsize];
      while(!in.eof() && in.good()){
          in.read(buffer, bufsize);
          size_t r = in.gcount();
          io.write(buffer, r);
      }
      in.close();
      io << FileTransferKeywords::EndData() << endl;
      io.flush();
      getline(io,line);
      in.close();
      if(line!=FileTransferKeywords::OK()){
         errMsg = "Problem during file transfer";
         return false;
      }
      socket->Close();
      delete socket;
      return true;

    }
    

   bool get(string& errMsg){
      if(!overwrite && FileSystem::FileOrFolderExists(local)){
         errMsg = "local file already exists";
         return false;
      }
      // create directory if not exist
      string pf = FileSystem::GetParentFolder(local);
      if(!FileSystem::CreateFolderEx(pf)){
         cerr <<  "could not create directory "  << pf;
      }

      ofstream out(local.c_str(), ios::binary|ios::trunc);
      if(!out.good()){
         errMsg = "could not create local file";
         return false;
      }
      // connect to server
      Socket* socket = Socket::Connect(host, stringutils::int2str(port), 
                              Socket::SockGlobalDomain, 3, 1);

      if(!socket){
           out.close();
           
           errMsg = "could not connect";
           return false;
      }
      if(!socket->IsOk()){
         socket->Close();
         delete socket;       
         errMsg = "could not connect";
         return false;
      }
      iostream& io = socket->GetSocketStream();
      io << FileTransferKeywords::SendFile() << endl;
      io << remote << endl;
      io.flush();
      string line;
      getline(io,line);
      if(line != FileTransferKeywords::OK()){
        if(line==FileTransferKeywords::Cancel()){
          errMsg = "remote file not found";
        } else {
            errMsg = "protocol error";
        }
        socket->Close();
        delete socket;       
        return false;
      }
      getline(io,line);
      bool ok;
      size_t length = stringutils::str2int<size_t>(line,ok);
      if(!ok){
        errMsg = "protocol error";
        socket->Close();
        delete socket;       
        return false;
      }
      getline(io,line);
      if(line!=FileTransferKeywords::Data()){
        errMsg = "protocol error";
        socket->Close();
        delete socket;       
        return false;
      }
      size_t bs = 1024 * 16;
      char buffer[bs];
      while(length>0 && io.good() && out.good()){
           if(bs > length){
             bs = length;
           }
           io.read(buffer,bs);
           size_t r = io.gcount();
           out.write(buffer,r);
           length -= r;  
      }
      out.close();
      getline(io,line);
      socket->Close();
      delete socket;
      if(line != FileTransferKeywords::EndData()){
         errMsg = "protocol error";
         return false;
      }
      errMsg = "";
      return true;
   }

  private:
     string local;
     string host;
     int port;
     bool overwrite;
     string remote;

};


template<class S, class H, class T, bool send>
int putOrGetFileTCPVMT(Word* args, Word& result, int message,
                    Word& local, Supplier s ){

   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   S* Source = (S*) args[0].addr;
   H* Host  = (H*) args[1].addr;
   CcInt* Port = (CcInt*) args[2].addr;
   CcBool* Over = (CcBool*) args[3].addr;
   T* Target = (T*) args[4].addr; 
   if(   !Source->IsDefined() || !Host->IsDefined() || !Port->IsDefined() 
      || !Target->IsDefined() || !Over->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   string errmsg="";
   bool success;
   if(send){
     fileCopy fc(Source->GetValue(), Host->GetValue(), Port->GetValue(), 
               Over->GetValue(),Target->GetValue());
      success = fc.send(errmsg);
   } else {
     fileCopy fc(Target->GetValue(), Host->GetValue(), Port->GetValue(), 
               Over->GetValue(),Source->GetValue());
      success = fc.get(errmsg);
   }
   if(!success){
      cerr << "Error : " << errmsg << endl;
   }
   res->Set(true, success);  
   return 0;
}


ValueMapping putFileTCPVM[] = {
    putOrGetFileTCPVMT<CcString, CcString, CcString,true>,
    putOrGetFileTCPVMT<CcString, CcString, FText,true>,
    putOrGetFileTCPVMT<CcString, FText, CcString,true>,
    putOrGetFileTCPVMT<CcString, FText, FText,true>,
    putOrGetFileTCPVMT<FText, CcString, CcString,true>,
    putOrGetFileTCPVMT<FText, CcString, FText,true>,
    putOrGetFileTCPVMT<FText, FText, CcString,true>,
    putOrGetFileTCPVMT<FText, FText, FText,true>
 };

int putOrGetFileTCPSelect(ListExpr args){
   ListExpr l = nl->First(args);
   ListExpr h = nl->Second(args);
   ListExpr r = nl->Fifth(args);
   int l1 = CcString::checkType(l)?0:4; 
   int h1 = CcString::checkType(h)?0:2; 
   int r1 = CcString::checkType(r)?0:1; 
   return l1 + h1 + r1;
}



OperatorSpec putFileTCPSpec(
  "{string,text} x {string,text} x int x bool x {string,text} -> bool",
  "putFileTCP(local, host, port, overwrite, remote)",
  "Copies a file to a remote server via TCP. On the remote server, "
  "a staticFileTransferator has to run. The result shows the success of "
  "the transfer.",
  "query putFileTCP('berlintest', Host, 1239, TRUE, 'berlintestCopy')"
);

Operator putFileTCPOp(
  "putFileTCP",
  putFileTCPSpec.getStr(),
  8,
  putFileTCPVM,
  putOrGetFileTCPSelect,
  putOrGetFileTCPTM
);


ValueMapping getFileTCPVM[] = {
    putOrGetFileTCPVMT<CcString, CcString, CcString,false>,
    putOrGetFileTCPVMT<CcString, CcString, FText,false>,
    putOrGetFileTCPVMT<CcString, FText, CcString,false>,
    putOrGetFileTCPVMT<CcString, FText, FText,false>,
    putOrGetFileTCPVMT<FText, CcString, CcString,false>,
    putOrGetFileTCPVMT<FText, CcString, FText,false>,
    putOrGetFileTCPVMT<FText, FText, CcString,false>,
    putOrGetFileTCPVMT<FText, FText, FText,false>
 };


OperatorSpec getFileTCPSpec(
  "{string,text} x {string,text} x int x bool x {string,text} -> bool",
  "copyFileTCP(remote, host, port, overwrite, ilocal)",
  "Gets a file from a remote server. On the remote server, "
  "a staticFileTransferator has to run. The result shows the success of "
  "the transfer.",
  "query getFileTCP('berlintest_copy3', Host, 1239, TRUE, 'berlintest')"
);

Operator getFileTCPOp(
  "getFileTCP",
  getFileTCPSpec.getStr(),
  8,
  getFileTCPVM,
  putOrGetFileTCPSelect,
  putOrGetFileTCPTM
);


/*
13 Operator ~fsfeed5~

13.1 Type Mapping

*/
ListExpr fsfeed5TM(ListExpr args){

  string err = "stream({text,string}) x {rel, text, string} expected";

  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + "( wrong number of args)");
  }
  ListExpr s1 = nl->First(args);
  if(!nl->HasLength(s1,2)){ // uses args in tm
    return listutils::typeError("internal error");
  }
  ListExpr s = nl->First(s1);
  if(     !Stream<FText>::checkType(s) 
      &&  !Stream<CcString>::checkType(s)){
    return listutils::typeError(err);
  }
  ListExpr t1 = nl->Second(args);
  if(!nl->HasLength(t1,2)){
     return listutils::typeError("internal error");
  }
  ListExpr t = nl->First(t1);
  if(   !Relation::checkType(t) && !FText::checkType(t) 
     && !CcString::checkType(t)){
    return listutils::typeError(err + "( wrong number of args");
  }
  if(Relation::checkType(t)){ // template function given
     return nl->TwoElemList(
                   listutils::basicSymbol<Stream<Tuple> >(),
                   nl->Second(t));
  }

  // filename given for template
  string filename;
  ListExpr t2 = nl->Second(t1);
  if(FText::checkType(t)){
     if(!getValue<FText>(t2,filename)){
       return listutils::typeError("could not extract filename");
     }
  } else {
     if(!getValue<CcString>(t2,filename)){
       return listutils::typeError("could not extract filename");
     }
  }
  // get the type stored in file
  ffeed5Info fi(filename);
  if(!fi.isOK()){
    return listutils::typeError("could not determine type from file " 
                                + filename);
  }
  ListExpr relType = fi.getRelType();
  return nl->TwoElemList(
                listutils::basicSymbol<Stream<Tuple> >(),
                nl->Second(relType));
}

template<class T>
class fsfeed5Info{
   public:
      fsfeed5Info(Word& _stream, ListExpr _tt): stream(_stream){
         tt = new TupleType(_tt);
         stream.open();
         fi = 0;
      }

      ~fsfeed5Info(){
         stream.close();
         tt->DeleteIfAllowed();
         if(fi){
           delete fi;
         }
       }

       Tuple* next(){
           while(true){
              if(!fi){
                openNextFile();
              }
              if(!fi){
                 return 0;
              }
              Tuple* res = fi->next();
              if(!res){
                 delete fi;
                 fi = 0;
              } else {
                 return res;
              }
           }
       }

   private:
      Stream<T> stream;
      TupleType* tt;
      ffeed5Info* fi;
      
      void openNextFile(){
          while(!fi){
              T* fn = stream.request();
              if(!fn){
                 return;
              }
              if(fn->IsDefined()){
                 fi = new ffeed5Info(fn->GetValue(),tt);
                 if(!fi->isOK()){
                     cerr << "ignore file" << fn->GetValue() << endl;
                     delete fi;
                     fi = 0;
                 }
              }
              fn->DeleteIfAllowed();
          }
      }
};


template<class T>
int fsfeed5VMT(Word* args, Word& result, int message,
                    Word& local, Supplier s ){

  fsfeed5Info<T>* li = (fsfeed5Info<T>*) local.addr;
  switch(message){
     case OPEN : if(li){
                   delete li;
                 }
                 local.addr = new fsfeed5Info<T>(args[0], 
                      nl->Second(GetTupleResultType(s)));
                 return 0;
     case REQUEST:
                result.addr = li?li->next():0;
                return result.addr?YIELD:CANCEL;
     case CLOSE:{
              if(li){
                delete li;
                local.addr = 0;
              }
              return 0;
     }
  } 
  return -1;
}



ValueMapping fsfeed5VM[] = {
  fsfeed5VMT<CcString>,
  fsfeed5VMT<FText>
};

int fsfeed5Select(ListExpr args){
  return Stream<CcString>::checkType(nl->First(args))?0:1;
}



OperatorSpec fsfeed5Spec(
  "stream({string,text}) x {rel,text,string} -> stream(tuple)",
  "_ fsfeed5[_]",
  "Creates a tuple stream from relations stored in binary files. "
  "The first argument contains the file names of the file from which "
  "the tuples should be extracted. Non existing files, files having "
  "wrong format or a different relation scheme are ignored. "
  "The second argument specifies the tuple type expected in the files. "
  "This may be a relation (the content of the relation is ignored) or "
  "a text/string specifiying a filename of a binary stored relation.",
  "query getDirectory(\".\") filter[ . startsWith 'strassen_'] "
  "fsfeed5[strassen] count"
);

Operator fsfeed5Op(
  "fsfeed5",
  fsfeed5Spec.getStr(),
  2,
  fsfeed5VM,
  fsfeed5Select,
  fsfeed5TM
);



/*
14 Operator ~partition~

This operator partioned the slots of a d[f]array on the clusters.

*/

ListExpr partitionTM(ListExpr args){

 string err ="d[f]array(rel(tuple)) x string x (tuple -> int) x int expected";

 if(!nl->HasLength(args,4)){
   return listutils::typeError(err + "(wrong number of args)");
 }


 ListExpr a0 = nl->First(args);  // the array
 ListExpr n0 = nl->Second(args); // the name
 ListExpr f0 = nl->Third(args);  // the function
 ListExpr s0 = nl->Fourth(args); // the (new) size
 
 if(   !nl->HasLength(a0,2) || !nl->HasLength(f0,2) 
    || !nl->HasLength(n0,2) || !nl->HasLength(s0,2)){
   return listutils::typeError("internal error");
 }

 ListExpr a1 = nl->First(a0);
 ListExpr f1 = nl->First(f0);
 ListExpr n1 = nl->First(n0);
 ListExpr s1 = nl->First(s0);

 if(!DFArray::checkType(a1) && !DArray::checkType(a1)){
   return listutils::typeError(err + "(first attr of wrong type)");
 }

 ListExpr r = nl->Second(a1);
 if(!Relation::checkType(r)){
   return listutils::typeError(err + "(array subtype is not a relation)");
 }
 if(!listutils::isMap<1>(f1)){
   return listutils::typeError(err + "(third arg is not a fun)");
 } 
 if(!CcString::checkType(n1)){
   return listutils::typeError(err + "(second arg is not a string)");
 }
 if(!CcInt::checkType(s1)){
   return listutils::typeError(err + " (fourth arg is nit an int)");
 }

 if(!CcInt::checkType(nl->Third(f1))){
    return listutils::typeError(err + "(fun result is not an int");
 }

 if(!nl->Equal(nl->Second(r), nl->Second(f1))){
    return listutils::typeError(" (function arg type does not fit"
                                " the relation type");
 }

 ListExpr funquery = nl->Second(f0);
  
 ListExpr funargs = nl->Second(funquery);

 ListExpr dat = nl->Second(r);

 ListExpr rfunargs = nl->TwoElemList(
                       nl->First(funargs),
                       dat);
 ListExpr rfun = nl->ThreeElemList(
                        nl->First(funquery),
                        rfunargs,
                        nl->Third(funquery));   


 ListExpr append = nl->HasLength(args,4) 
                  ? nl->OneElemList(nl->TextAtom(nl->ToString(rfun)))
                  : nl->TwoElemList(
                         nl->IntAtom(0),
                         nl->TextAtom(nl->ToString(rfun)));

  ListExpr res =  nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               append,
               nl->TwoElemList( listutils::basicSymbol<DFMatrix>(),
                                 r));
  return res;
}




template<class A>
class partitionInfo{

  public:

    partitionInfo(A* _array,int _resSize, size_t _wnum,
                  ConnectionInfo* _ci, const string& _sfun, 
                  const string& _dfun,
                 string& _tname, ListExpr _relType,
                 const string& _dbname):
        array(_array), resSize(_resSize), workerNumber(_wnum),
        ci(_ci), sfun(_sfun), dfun(_dfun),tname(_tname), 
        sname(array->getName()), relType(_relType),dbname(_dbname),
        runner(0){
        runner = new boost::thread(&partitionInfo::run,this);
    }

    ~partitionInfo(){
        runner->join();
        delete runner;
     }

  private:
     A* array;
     int resSize;
     size_t workerNumber;
     ConnectionInfo* ci;
     string sfun;
     string dfun;
     string tname;
     string sname;
     ListExpr relType;
     string dbname;
     boost::thread* runner;
      
     void run(){
        if(!ci){
           return;
        }
        // construct target directory for the matrix on ci
        string targetDir = ci->getSecondoHome() + "/dfarrays/" + dbname 
                           + "/" + tname + "/"
                           + stringutils::int2str(workerNumber) + "/";
        int err;
        string errMsg;
        double runtime;
        ListExpr resList;
        ci->simpleCommand("query createDirectory('"+targetDir+"', TRUE)", 
                          err, errMsg, 
                          resList, false,runtime);
        if(err!=0){
           cerr << __FILE__ << "@"  << __LINE__ << endl;
           showError(ci, "query createDirectory('"+targetDir+"', TRUE)",
                     err,errMsg);
           return;
        }
        if(!nl->HasLength(resList,2)){
           cerr << "unexpected result during creation of directory" << endl;
           return;
        } 
        if(nl->AtomType(nl->Second(resList))!=BoolType){
           cerr << "unexpected result during creation of directory" << endl;
           return;
        }
        if(!nl->BoolValue(nl->Second(resList))){
            //cerr << "creating directory '" << targetDir << "'failed" << endl;
            //cerr << "resList is " << nl->ToString(resList) << endl;
            // may be directory already exists, TODO: check if it is a directory
        }
        string cmd = constructQuery(targetDir);
        if(cmd==""){
           cerr << "worker " << workerNumber 
                << " does not contain any slot" << endl;
           return;
        }
        string res;
        ci->simpleCommandFromList(cmd, err,errMsg, res, false, runtime);
        if(err!=0){
           cerr << __FILE__ << "@"  << __LINE__ << endl;
           showError(ci,cmd,err,errMsg);
        }
     }



     string constructQuery(const string& targetDir){
        switch(array->getType()){
           case DARRAY : return constructQueryD(targetDir);
           case DFARRAY : return constructQueryDF(targetDir);
           case DFMATRIXXX : assert(false);
        }
        return "";
     }



     string constructQueryD(const string& targetDir){

        // create relation containing the objectnames of the source
        // for this worker
        stringstream ss;
        ss << "(" ; // open value

        for(size_t i=0;i<array->getSize();i++){
           if(array->getWorkerIndexForSlot(i)==workerNumber){
              ss << " (\"" << array->getName() << "_" << i << "\" )" << endl;
           }
        }
        ss << ")"; // end of value list



        string rel = " ( (rel(tuple((T string)))) " + ss.str() + ")";

        // the query in user syntax would be
        // query rel feed projecttransformstream[T] fdistribute7[
        // fun, resSize, dir+"/"+tname, TRUE] count


        string stream1 = "(projecttransformstream (feed " + rel + ") T )";
        string stream2 =   "(feedS " + stream1 
                         + "("+nl->ToString(relType) 
                         + " ()))"; 
        string stream3 = stream2;
        if(!sfun.empty()){
          stream3 = "("+ sfun + "(consume "+  stream2 + "))";
        }


       
        stringstream query;


        query << "(query "
              << " (count "
              << "   ( fdistribute7 "
              <<       stream3
              <<       " '" << targetDir <<"/" << tname <<"' "
              <<       " " << dfun << " "
              <<        resSize
              <<       " TRUE "
              <<       " )))";
        return query.str();
     }


     string constructQueryDF(const string& dir){
        // construct query in nested list form,

        string sourceDir = ci->getSecondoHome() + "/dfarrays/"
                           + dbname + "/" + sname + "/";

        // create a relation containing the filenames
        
        ListExpr relTemp = nl->TwoElemList( relType, nl->TheEmptyList());

        ListExpr fnrelType = nl->TwoElemList(
                                 listutils::basicSymbol<Relation>(),
                                 nl->TwoElemList(
                                    listutils::basicSymbol<Tuple>(),
                                    nl->OneElemList(
                                       nl->TwoElemList(
                                            nl->SymbolAtom("T"),
                                            listutils::basicSymbol<FText>()))));

        // build all texts
        bool first = true;
        ListExpr value = nl->TheEmptyList();
        ListExpr last = value;
        for(size_t i=0;i<array->getSize();i++){
            if(array->getWorkerIndexForSlot(i)==workerNumber){
                 ListExpr F  = nl->OneElemList(
                                   nl->TextAtom(sourceDir+sname + "_"
                                   + stringutils::int2str(i) + ".bin"));
                 if(first){
                     value = nl->OneElemList(F );
                     last = value;
                     first = false;
                 } else {
                     last = nl->Append(last,F);
                 }
            }
        }
        if(nl->IsEmpty(value)){
           return "";
        }
        ListExpr fnrel = nl->TwoElemList(fnrelType, value);
        ListExpr feed = nl->TwoElemList(nl->SymbolAtom("feed"), fnrel);

        ListExpr stream = nl->ThreeElemList(
                              nl->SymbolAtom("projecttransformstream"),
                              feed,
                              nl->SymbolAtom("T"));

        string streamer = sfun.empty()?"fsfeed5":"createFSrel";

        ListExpr fsfeed = nl->ThreeElemList(
                                nl->SymbolAtom(streamer),
                                stream,
                                relTemp);

        ListExpr streamFun = fsfeed;
        if(!sfun.empty()){
           ListExpr sfunl;
           { boost::lock_guard<boost::mutex> guard(nlparsemtx);
             bool ok = nl->ReadFromString(sfun, sfunl);
             if(!ok){
               cerr << "error in parsing list: " << sfun << endl;
               return "";
             }
           }
           streamFun = nl->TwoElemList( sfunl, fsfeed);
        }


        ListExpr dfunl;
        
        { boost::lock_guard<boost::mutex> guard(nlparsemtx);
          bool ok = nl->ReadFromString(dfun, dfunl); 
          if(!ok){
              cerr << "problem in parsing function" << endl;
              return "";
          }
        }

        ListExpr fdistribute = nl->SixElemList(
                                     nl->SymbolAtom("fdistribute7"),
                                     streamFun,
                                     nl->TextAtom(dir+"/"+tname),
                                     dfunl,
                                     nl->IntAtom(resSize),
                                     nl->BoolAtom(true)
                                   );

        ListExpr query =
                  nl->TwoElemList(
                    nl->SymbolAtom("query"),
                    nl->TwoElemList(
                       nl->SymbolAtom("count"),
                       fdistribute));

        return nl->ToString(query);
     }

};




template<class A>
int partitionVMT(Word* args, Word& result, int message,
                    Word& local, Supplier s ){

   A* array = (A*) args[0].addr;

   CcString* name;
   CcInt* newSize;
   string funText="";
   string dfunText="";

   if(qp->GetNoSons(s)==5){
      // without additional function
      name = (CcString*) args[1].addr;
      // third arg is the function, we get the text
      newSize = (CcInt*) args[3].addr;
      dfunText = ((FText*) args[4].addr)->GetValue();
   } else if(qp->GetNoSons(s)==7){
      name =  (CcString*) args[1].addr;
      // args[2] and args[3] are the functions
      newSize = (CcInt*) args[4].addr;
      funText = ((FText*) args[5].addr)->GetValue();
      dfunText = ((FText*) args[6].addr)->GetValue();
   } else {
      assert(false); // invalid number of arguments
      name = 0;
      newSize = 0;
   }

   int size = array->getSize();

   if(newSize->IsDefined() &&
      newSize->GetValue() > 0){
      size = newSize->GetValue();
   }

   result = qp->ResultStorage(s);
   DFMatrix* res = (DFMatrix*) result.addr;

   if(!array->IsDefined() || !name->IsDefined()){
       res->makeUndefined();
       return 0;
   }

   string tname = name->GetValue();
   if(tname.size()==0){
      tname = algInstance->getTempName();
   }

   if(!stringutils::isIdent(tname)){
     res->makeUndefined();
     return 0;
   }

   if(tname == array->getName()){
     res->makeUndefined();
     return 0;
   }

   res->copyFrom(*array);

   res->setName(tname);
   res->setSize(size);
   
   vector<partitionInfo<A>*> infos;

   ListExpr relType = nl->Second(qp->GetType(qp->GetSon(s,0)));
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();

   for(size_t i=0;i<array->numOfWorkers();i++){
      DArrayElement de = array->getWorker(i);
      ConnectionInfo* ci = algInstance->getWorkerConnection(de,dbname);
      if(ci){
         partitionInfo<A>* info = new partitionInfo<A>(array, size, i, ci,
                                             funText, dfunText, tname, 
                                             relType, dbname);
         infos.push_back(info);
      }
   }

   for(size_t i=0;i<infos.size();i++){
       delete infos[i];
   }
   return 0;
}


OperatorSpec partitionSpec(
  "d[f]array(rel(tuple)) x string x (tuple->int) x int-> dfmatrix",
  "_ partition[_,_,_]",
  "Redistributes the contents of a dfarray value. "
  "The new slot contents are kept on the worker where "
  "the values were stored before redistributing them. "
  "The last argument (int) determines the number of slots "
  "of the redistribution. If this value is smaller or equal to "
  "zero, the number of slots is overtaken from the array argument.",
  "query da2 partition[ hashvalue(.Name,2000) + 23, \"dm2\",0]"
);

int partitionSelect(ListExpr args){
   return DFArray::checkType(nl->First(args))?0:1; 
}

ValueMapping partitionVM[] = {
  partitionVMT<DFArray>,
  partitionVMT<DArray>
};


Operator partitionOp(
  "partition",
  partitionSpec.getStr(),
  2,
  partitionVM,
  partitionSelect,
  partitionTM
);



/*
17 Operator ~partitionF~

This operator works similar as the partition operator. The difference is that
the ~partitionF~ operator applies a function before redistributing the array.

*/
ListExpr partitionFTM(ListExpr args){

  string err = "expected: d[f]array(rel(tuple(X))) x string x "
               "([fs]rel(tuple(X)) -> stream(tuple(Y))) x "
               "(tuple(Y)->int) x int";

  if(!nl->HasLength(args,5)){
    return listutils::typeError(err + " (wrong number of args)");
  }
  // check UsesTypes in Type Mapping
  ListExpr tmp = args;
  while(!nl->IsEmpty(tmp)){
     if(!nl->HasLength(nl->First(tmp),2)){
       return listutils::typeError("internal error");
     }
     tmp = nl->Rest(tmp);
  }
  ListExpr a = nl->First(args);  // array
  ListExpr n = nl->Second(args); // name of the result
  ListExpr f = nl->Third(args); // function
  ListExpr d = nl->Fourth(args);  // redistribution function
  ListExpr s = nl->Fifth(args);  // size of the result

  if(   !nl->HasLength(a,2) || !nl->HasLength(n,2) || !nl->HasLength(f,2)
     || !nl->HasLength(d,2) || !nl->HasLength(s,2)){
    return listutils::typeError("internal error");
  } 

  // check Types
  ListExpr a1 = nl->First(a);
  ListExpr n1 = nl->First(n);
  ListExpr f1 = nl->First(f);
  ListExpr d1 = nl->First(d);
  ListExpr s1 = nl->First(s);

  if(!DArray::checkType(a1) && !DFArray::checkType(a1)){
     return listutils::typeError(err + " (first arg is not a d[f]array)");
  }
  ListExpr subtype = nl->Second(a1);
  if(!Relation::checkType(subtype)){
     return listutils::typeError(err + " (array subtype is not a relation)");
  }
  if(!CcString::checkType(n1)){
     return listutils::typeError(err + " (second arg is not a string)");
  }
  if(!listutils::isMap<1>(f1) && !listutils::isMap<2>(f1)){
     return listutils::typeError(err + " (third arg is not a function)");
  }
  if(!listutils::isMap<1>(d1) && !listutils::isMap<2>(d1)){
     return listutils::typeError(err + " (fourth arg is not a function)");
  }
  if(!CcInt::checkType(s1)){
     return listutils::typeError(err + " (fifth arg is not an int)");
  }
  // check function arguments and results
  ListExpr expFunArg =   DArray::checkType(a1)
                        ?subtype
                        : nl->TwoElemList(
                               listutils::basicSymbol<fsrel>(),
                               nl->Second(subtype));

  if(!nl->Equal(expFunArg, nl->Second(f1))){
     return listutils::typeError(" argument of function does not "
                                 "fit the array type.");
  }

  // extract result of function
  ListExpr f1Res= f1;
  while(!nl->HasLength(f1Res,1)){
     f1Res = nl->Rest(f1Res);
  }
  f1Res = nl->First(f1Res);

  // the result of the first function must be a tuple stream
  if(!Stream<Tuple>::checkType(f1Res)){
    return listutils::typeError("function result is not a tuple stream");
  }

  // extract result of d1
  ListExpr d1Res = d1;
  while(!nl->HasLength(d1Res,1)){
    d1Res = nl->Rest(d1Res);
  }
  d1Res = nl->First(d1Res);

  // the result of d1 must be int
  if(!CcInt::checkType(d1Res)){
    return listutils::typeError("result for distribution function is "
                                "not an int");
  }

  // extract used argument in d1 (the last argument)
  ListExpr d1UsedArg = d1;
  while(!nl->HasLength(d1UsedArg,2)){
    d1UsedArg = nl->Rest(d1UsedArg);
  }
  d1UsedArg = nl->First(d1UsedArg);

  if(!Tuple::checkType(d1UsedArg)){
    return listutils::typeError("argument of the distribution function is "
                                "not a tuple");
  }

  if(!nl->Equal(nl->Second(f1Res),d1UsedArg)){
    return listutils::typeError("type mismatch between result of the funciton "
                            "and the argument of the distribution function");
  }

  ListExpr funDef = nl->Second(f);

  // if f1 is defined to have two arguments, we ensure that the 
  // second argument is unused
  // within the whole function definition

  if(listutils::isMap<2>(f1)){
     string arg2Name = nl->SymbolValue(nl->First(nl->Third(funDef)));
     if(listutils::containsSymbol(nl->Fourth(funDef), arg2Name)){
        return listutils::typeError("Usage of the second argument in "
                                    "function is not allowed");
     }
  }


  ListExpr fdarg = nl->Second(funDef);
  ListExpr dfcomp = nl->HasLength(funDef,3)
                   ?nl->Third(funDef)
                   :nl->Fourth(funDef);

  // rewrite function
  ListExpr rfunDef = nl->ThreeElemList(
                       nl->First(funDef),
                       nl->TwoElemList(
                             nl->First(fdarg),
                             expFunArg),
                       dfcomp
                     );

   ListExpr dfunDef = nl->Second(d);
   if(listutils::isMap<2>(d1)){
     string arg1Name = nl->SymbolValue(nl->First(nl->Second(dfunDef)));
     if(listutils::containsSymbol(nl->Fourth(dfunDef),arg1Name)){
        return listutils::typeError("Usage of the first argument is not "
                                    "allowed within the distribution function");
     }
   }

   ListExpr ddarg = nl->HasLength(dfunDef,3)
                    ?nl->Second(dfunDef)
                    :nl->Third(dfunDef);
   ListExpr ddcomp = nl->HasLength(dfunDef,3)
                     ?nl->Third(dfunDef)
                     :nl->Fourth(dfunDef);

   ListExpr rdfunDef = nl->ThreeElemList(
                          nl->First(dfunDef),
                          nl->TwoElemList(
                              nl->First(ddarg),
                              nl->Second(f1Res)),
                        ddcomp
                      );


  ListExpr appendList = nl->TwoElemList(
                     nl->TextAtom( nl->ToString(rfunDef)),
                     nl->TextAtom( nl->ToString(rdfunDef)) );

  ListExpr resType = nl->TwoElemList(
                          listutils::basicSymbol<DFMatrix>(),
                          nl->TwoElemList(
                               listutils::basicSymbol<Relation>(),
                               nl->Second(f1Res)));

  return nl->ThreeElemList(
                   nl->SymbolAtom(Symbols::APPEND()),
                   appendList,
                   resType
               );

  // create result type
  return listutils::typeError("Type Mapping not completely implemented yet.");
}


/*
17 TypeMapOperator ~FFR~

*/
ListExpr FFRTM(ListExpr args){

  if(!nl->HasLength(args,2) && !nl->HasLength(args,3)){
    return listutils::typeError("2 or 3 arguments expected") ;
  }

  if(nl->HasLength(args,2)){
    ListExpr arg = nl->First(args);
    if(DArray::checkType(arg) ){
       if(!Relation::checkType(nl->Second(arg))){
          return listutils::typeError("darray's subtype muts be a relation");
       }
       return nl->Second(arg); // return the relation
    } 
    if(DFArray::checkType(arg)){
       return nl->TwoElemList( listutils::basicSymbol<fsrel>(),
                               nl->Second(nl->Second(arg)));
    }
    return listutils::typeError("first arg is not a d[f]array");
  }
  // three arguments
  ListExpr arg = nl->Third(args);
  if(!listutils::isMap<1>(arg) && !listutils::isMap<2>(arg)){
    return listutils::typeError("third arg is not a function");
  } 
  // extract function result 
  while(!nl->HasLength(arg,1)){
    arg = nl->Rest(arg);
  }
  arg = nl->First(arg);
  if(!Stream<Tuple>::checkType(arg)){
    return listutils::typeError("function result is not a tuple stream");
  }
  ListExpr res =  nl->Second(arg);
  return res;
}


OperatorSpec FFRSpec(
  "d[f]array(rela(tuple(X))) x A -> frel(tuple(X)) or  "
  "B x C x (frel(tuple(D) -> stream(tuple(E)) ) -> tuple(E)",
  " FFR(_,_,_)",
  "Type Map Operator",
  "query FFR(test) getTypeNL"
);


Operator FFROp(
  "FFR",
  FFRSpec.getStr(),
  0,
  Operator::SimpleSelect,
  FFRTM
);

OperatorSpec partitionFSpec(
  "d[f]array(rel(X)) x string x ([fs]rel(X)->stream(Y)) x (Y -> int) x "
  "int -> dfmatrix(rel(Y)) ",
  "_ partitionL[_,_,_,_] ",
  "Repartitions a distributed [file] array. Before repartition, a "
  "function is applied to the slots.",
  "query a1 partitionL[ \"name\", . feed head[12], hashvalue(.Attr,23), 0]" 
);


/*
We don't need selection and value mappings because we reuse the one of the
partition operator is able to handle even the version with additionally 
function.

*/

Operator partitionFOp(
  "partitionF",
  partitionFSpec.getStr(),
  2,
  partitionVM,
  partitionSelect,
  partitionFTM
);




/*
18 Operator ~areduce~

*/

ListExpr areduceTM(ListExpr args){

  string err = "dmatrix(rel(t)) x string x (fsrel(t)-Y) x int expected";
  if(!nl->HasLength(args,4)){
   return listutils::typeError(err + " (wrong number of args)");
  }

  // check SetUsesArgsInTypeMapping
  if(   !nl->HasLength(nl->First(args),2)
     || !nl->HasLength(nl->Second(args),2)
     || !nl->HasLength(nl->Third(args),2)
     || !nl->HasLength(nl->Fourth(args),2)){
    return listutils::typeError(err);
  }

  ListExpr m = nl->First(args);
  ListExpr n = nl->Second(args);
  ListExpr f = nl->Third(args);
  ListExpr p = nl->Fourth(args);

  ListExpr m1 = nl->First(m);
  ListExpr n1 = nl->First(n);
  ListExpr f1 = nl->First(f);
  ListExpr p1 = nl->First(p);



  if(   !DFMatrix::checkType(m1)
     || !CcString::checkType(n1)
     || !listutils::isMap<1>(f1)
     || !CcInt::checkType(p1)){
    return listutils::typeError(err);
  }

  // check function argument
  ListExpr tupleType = nl->Second(nl->Second(m1));
  
   ListExpr fsrelt = nl->TwoElemList(
                             listutils::basicSymbol<fsrel >(),
                             tupleType);

  ListExpr funArg = nl->Second(f1);

  if(!nl->Equal(fsrelt,funArg)){
     return listutils::typeError("function arg does not fit to"
                                 " the dmatrix subtype");
  }

  // check funres for allowed type
  // allowed are non-stream objects and stream(tuple)
  ListExpr funres = nl->Third(f1);

  bool isF = false;
  bool isStream=false;
  if(    nl->HasLength(funres,2) 
      && nl->IsEqual(nl->First(funres),Stream<Tuple>::BasicType())){

     // funtion result is s stream, allow only tuple stream to be
     // the result
     if(!Stream<Tuple>::checkType(funres)){
        return listutils::typeError("function result is a stream "
                                    "of non-tuples");
     }
     isF = true; // can be stored within a dfarray
     isStream = true;
  }
  if(!isF){
    isF = Relation::checkType(funres);
  }
  
  ListExpr funquery = nl->Second(f);
  
  ListExpr funargs = nl->Second(funquery);

  ListExpr dat = fsrelt;

  ListExpr rfunargs = nl->TwoElemList(
                       nl->First(funargs),
                       dat);
  
  ListExpr rfun = nl->ThreeElemList(
                        nl->First(funquery),
                        rfunargs,
                        nl->Third(funquery));

  ListExpr res;
  if(!isF){
    res = nl->TwoElemList( listutils::basicSymbol<DArray>(),
                            funres);
  } else {
    if(!isStream){
       res = nl->TwoElemList( listutils::basicSymbol<DFArray>(),
                              funres);
    } else {
       res = nl->TwoElemList( listutils::basicSymbol<DFArray>(),
                              nl->TwoElemList(
                                listutils::basicSymbol<Relation>(),
                                nl->Second(funres)));
    }
  }

  
   ListExpr fres =  nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            nl->TwoElemList( nl->TextAtom(nl->ToString(rfun)),
                             nl->BoolAtom(isStream)),
            res);

   return fres;

}



class AReduceListener{

  public:
     virtual void ready(int slot, int worker, boost::thread* runner)=0;

};



template<class R>
class AReduceTask{

  public:
     AReduceTask(DFMatrix* _matrix1, DFMatrix* _matrix2, int _worker, 
                 R* _result, int _port, string& _funtext, ListExpr _relType1,
                 ListExpr _relType2, bool _isStream, 
                 AReduceListener* _listener):
          matrix1(_matrix1), matrix2(_matrix2), worker(_worker), 
          result(_result), port(_port), funtext(_funtext), relType1(_relType1),
          relType2(_relType2), isStream(_isStream), listener(_listener)
      {
         dbname = SecondoSystem::GetInstance()->GetDatabaseName();
         // matrix2 is either null or has the same worker specification 
         // as matrix1
         // for this reason, we can always use the worker of matrix1
         ci = algInstance->getWorkerConnection(
                                          matrix1->getWorker(worker),dbname); 
         runner = 0;
         running = false;
      }


      ~AReduceTask(){
          if(runner){
             runner->join();
             delete runner;
          }
       }

      

     void process(int slot){
          if(runner){
             runner->join();
             delete runner;
          }
          currentSlot = slot;
          runner = new boost::thread(&AReduceTask::run, this);
     }


  private:
     DFMatrix* matrix1;
     DFMatrix* matrix2;
     int worker;
     R* result;
     int port;
     string funtext;
     ListExpr relType1;
     ListExpr relType2;
     bool isStream;
     AReduceListener* listener;

     ConnectionInfo* ci;
     string dbname;

     int currentSlot;
     boost::thread* runner;
     bool running;


     void run(){
          stringstream ss;
          ss << "process slot " << currentSlot << " on worker " 
             << worker << endl;
          result->setResponsible(currentSlot,worker);
          cout << ss.str() ;


          // step 1: create a temporal directory for the slot
          string dir =   "tmp/w_" + stringutils::int2str(worker) + "_s_"
                       + stringutils::int2str(currentSlot);
          int err;
          string errMsg;
          string res;
          double runtime;
          string cmd="query createDirectory('"+dir+"', TRUE)";
          ci->simpleCommand(cmd, err, errMsg, res, false, runtime);
          if(err){
               cerr << "creating directory " << dir << " on worker " 
                    << worker << "failed" << endl;
               listener->ready(currentSlot,worker, runner);
               return;
          }

          // step 2: built a relation containing the 
          // remote file R, the ip adress IP, and the local File
          // name L  for each worker
          
          string rt = "rel(tuple([ R : text, IP: text, L : text, M : int ]))";
          string nlrt ="(rel(tuple(( R text)(IP text)(L text)(M int))))";

          string rv = "(";
          for(size_t i=0;i<matrix1->numOfWorkers();i++){
             ConnectionInfo* wi = algInstance->getWorkerConnection(
                                               matrix1->getWorker(i), dbname);
             string remote1 =   wi->getSecondoHome() + "/dfarrays/" + dbname 
                             + "/" 
                             + matrix1->getName()+"/" + stringutils::int2str(i)
                             + "/" + matrix1->getName() + "_"  
                             + stringutils::int2str(currentSlot) + ".bin";
             string local1 =   dir + "/" + matrix1->getName() + "_"
                            + stringutils::int2str(i) + ".bin";
             rv += "( '" + remote1 +"' '" + wi->getHost() +"' '" + local1
                   + "' 1 )";
             // also insert the files for matrix2 if necessary
             if(matrix2 && (matrix2->getName()!=matrix1->getName())){
                string remote2 =   wi->getSecondoHome() + "/dfarrays/" + dbname 
                                 + "/" 
                                 + matrix2->getName()+"/" 
                                 + stringutils::int2str(i)
                                 + "/" + matrix2->getName() + "_"  
                                 + stringutils::int2str(currentSlot) + ".bin";
                 string local2 =   dir + "/" + matrix2->getName() + "_"
                            + stringutils::int2str(i) + ".bin";
                 rv += "( '" + remote2 +"' '" + wi->getHost() +"' '" + local2
                       + "' 2 )";

             }
          }
          rv += ")";


          string rel = "[const " + rt + " value " + rv+ "]";
          string nlrel="(" + nlrt + " " + rv +")";
          cmd =   "query " + rel + " feed extend[ OK : getFileTCP( .R, .IP, " 
                + stringutils::int2str(port) + ", TRUE, .L)] count";
          ci->simpleCommand(cmd, err, errMsg, res, false, runtime);
          if(err){
               cerr << "command  " << cmd  << " on worker " 
                    << worker << "failed" << endl;
               cerr << errMsg << endl;
               listener->ready(currentSlot,worker, runner);
               return;
          }

          // now, all required files are on the worker workernum

          // produce query  doing the actually work, the query 
          // must be given in nested list syntax 
          // because the function is in nl

          string tuplestream0 = "(feed " +nlrel + ")";
          string tuplestream0_1 = tuplestream0;
          string tuplestream0_2 = "";
          if(matrix2){
               if(matrix1->getName()!=matrix2->getName()){
                    tuplestream0_1 = "(filter "+ tuplestream0 
                                        + " (fun (streamelem1 STREAMELEM) "
                                          "(= (attr streamelem1 M) 1)))";
                     tuplestream0_2 = "(filter "+ tuplestream0 
                                        + " (fun (streamelem2 STREAMELEM) "
                                          "(= (attr streamelem2 M) 2)))";
               }  else {
                  tuplestream0_2 = tuplestream0;
               }
          }

          string tuplestream1 = " ( createFSrel (projecttransformstream" 
                                + tuplestream0_1 + " L )(" 
                                + nl->ToString(relType1) + "()) )";

          string tuplestream2 = ""; // no tuple stream
          if(matrix2){
             tuplestream2 =   "( createFSrel (projecttransformstream" 
                            + tuplestream0_2
                            + " L )(" + nl->ToString(relType2) + "())) ";
          }



          if(result->getType()==DARRAY){ // a usual let command

              string objname =    result->getName() + "_" 
                                + stringutils::int2str(currentSlot);
              ci->simpleCommand("delete " + objname, err, errMsg, res, false, 
                                runtime);
              cmd =   "( let " + objname+"  =  (" + funtext + " "
                    + tuplestream1 +  tuplestream2 + "))";
          } else {

              string feed1 =isStream?"":" feed (";
              string feed2 =isStream?"":" ) ";

              string tdir =   ci->getSecondoHome() + "/dfarrays/" + dbname
                            + "/" + result->getName()+"/";
              string filename =  tdir + result->getName() + "_"
                                + stringutils::int2str(currentSlot) + ".bin";
              cmd = "query createDirectory('"+tdir+"', TRUE)";
              ci->simpleCommand(cmd, err, errMsg, res, false, runtime);
              if(err){
                      cerr << "command  " << cmd  << " on worker " 
                           << worker << "failed" << endl;
                      cerr << errMsg << endl;
              }
              cmd =   "( query ( count ( fconsume5 (" + feed1 +  "" + funtext 
                    + " " + tuplestream1 + tuplestream2 + " )" + feed2 + "'" 
                    + filename + "')))";
          }
          ci->simpleCommandFromList(cmd,err,errMsg, res, false, runtime);
          if(err){
              cerr << __FILE__ << "@"  << __LINE__ << endl;
              showError(ci,cmd,err,errMsg);
          } 
         
          cmd="query removeDirectory('"+dir+"', TRUE)";
          ci->simpleCommand(cmd, err, errMsg, res, false, runtime);
          if(err){
               cerr << "removing directory " << dir << " on worker " 
                    << worker << "failed" << endl;
               cerr << errMsg << endl;
          }

          listener->ready(currentSlot,worker, runner);
          return;

     }

};


template<class R>
class AReducer: public AReduceListener{
  public:
    AReducer(DFMatrix* _matrix1, DFMatrix* _matrix2, R* _result, int _port, 
            const string& _funtext, bool _isStream, ListExpr _relType1,
            ListExpr _relType2):
      matrix1(_matrix1), matrix2(_matrix2), result(_result), port(_port), 
      funtext(_funtext),isStream(_isStream), relType1(_relType1), 
      relType2(_relType2) {
      targetName = result->getName();
      isFile = result->getType()==DFARRAY; 

   }

   void reduce(){

       // if matrix2 is not null, the matriox sizes are equal       
       // hence we can use always matrix1 to get the number of workers.

       // create Reduce Objects for each worker
       for(size_t i=0;i<matrix1->numOfWorkers();i++){
          tasks.push_back(new AReduceTask<R>(matrix1, matrix2,i,result,
                          port, funtext, relType1, relType2, isStream,this));
       } 

       // start a task on each worker (one by one, slot, worker)
       slot = matrix1->numOfWorkers();
       for(size_t i=0; i<matrix1->numOfWorkers();i++){
         tasks[i]->process(i);  
       }

       size_t all = matrix1->getSize(); 
       if(matrix2){
          if(matrix2->getSize() < all){
            all = matrix2->getSize();
          }
       }

       while(slot < all){ // not all slots are processed
           boost::unique_lock<boost::mutex> lock(mtx);
           while(freeWorkers.empty()){
              cond.wait(lock);
           }
           // there is a new free worker
           int w = freeWorkers.front();
           freeWorkers.pop();
           tasks[w]->process(slot);
           slot++;
       }

       // for all slots, a task was started
       // wait for finish

       for(size_t i=0;i<tasks.size();i++){
          delete tasks[i];
       }
   }

   void ready(int slot, int worker, boost::thread* theThread){
       {
          boost::lock_guard<boost::mutex> lock(mtx);
          freeWorkers.push(worker); 
       }
       cond.notify_one();
   }


  private:
    DFMatrix* matrix1;
    DFMatrix* matrix2;
    R*        result;
    int       port;
    string    funtext;
    bool      isStream;
    ListExpr  relType1;
    ListExpr  relType2;
    string    targetName;
    bool      isFile;
    int       slot;
    queue<int> freeWorkers; 
    vector<AReduceTask<R>*> tasks;
    boost::condition_variable cond;
    boost::mutex mtx;

};


template<class R>
int areduceVMT(Word* args, Word& result, int message,
             Word& local, Supplier s ){

   result = qp->ResultStorage(s);
   R* res = (R*) result.addr;

   DFMatrix* matrix1 = (DFMatrix*) args[0].addr;
   if(!matrix1->IsDefined()){
      //cout << "Matrix is undefined" << endl;
      res->makeUndefined();
      return 0;
   }



   DFMatrix* matrix2 = 0;
   int o = 0;

   int resSize = matrix1->getSize();

   ListExpr relType2 = nl->TheEmptyList();

   if(qp->GetNoSons(s)==7){
      matrix2 = (DFMatrix*) args[1].addr;
      if(!matrix2->IsDefined()){
         //cout << "matrix2 is undefined" << endl;
         res->makeUndefined();
         return 0;
      }

      if(!matrix1->equalWorkers(*matrix2)){
         //cout << "different worker specification" << endl;
         res->makeUndefined();
         return 0;
      }

      if(matrix2->getSize() < matrix1->getSize()){
          resSize = matrix2->getSize();
      }

      relType2 = nl->Second( qp->GetType(qp->GetSon(s,1)));
      o = 1;
   }




   CcString* ResName = (CcString*) args[o+1].addr;
   CcInt* Port = (CcInt*) args[o+3].addr;


   if(!Port->IsDefined() || !ResName->IsDefined()){
      res->makeUndefined();
      return 0;
   }

   int port = Port->GetValue();
   if(port<=0){
      res->makeUndefined();
      return 0;
   }

   string resName = ResName->GetValue();
   if(resName.size()==0){
      resName = algInstance->getTempName();
   }




   if(!stringutils::isIdent(resName)){
      res->makeUndefined();
      return 0;
   }

   res->copyFrom(*matrix1);
   res->setName(resName);
   res->setStdMap(resSize);

   
   if(!startFileTransferators(matrix1, port)){
      res->makeUndefined();
      return 0;
   }

   // now, on each worker host, a file transferator is listen

   string funtext = ((FText*) args[o+4].addr)->GetValue();
   bool isStream = ((CcBool*) args[o+5].addr)->GetValue();

   ListExpr relType1 = nl->Second( qp->GetType(qp->GetSon(s,0)));
   
   AReducer<R> reducer(matrix1, matrix2,  res, port, funtext, 
                       isStream, relType1, relType2);

   reducer.reduce();


   return 0;
}


OperatorSpec areduceSpec(
   "dfmatrix(rel(t)) x string x (fsrel(t)->Y) x int -> d[f]array(Y)",
   "matrix areduce[newname, function, port]",
   "Performs a function on the distributed slots of an array. "
   "The task distribution is dynamically, meaning that a fast "
   "worker will handle more slots than a slower one. "
   "The result type depends on the result of the function. "
   "For a relation or a tuple stream, a dfarray will be created. "
   "For other non-stream results, a darray is the resulting type.",
   "The integer argument specifies the port for transferring files.",
   "query m8 areduce[ . feed count, 1237]"
);

ValueMapping areduceVM [] = {
   areduceVMT<DFArray>,
   areduceVMT<DArray>
};

int areduceSelect(ListExpr args){

  

  ListExpr funRes;

  funRes =  nl->HasLength(args,4)
           ?nl->Third(nl->Third(args))
           :nl->Fourth(nl->Fourth(args));

  if(Stream<Tuple>::checkType(funRes) ||
     Relation::checkType(funRes)){
    return 0;
  }
  return 1;
}


Operator areduceOp(
  "areduce",
  areduceSpec.getStr(),
  2,
  areduceVM,
  areduceSelect,
  areduceTM
);


/*
28 Operator ~areduce2~

This operator merges two dfmatrices into a single darray.
If the result is a tuple stream or a relation the result will
be a dfarray. Merging of the matrices will be done slot by slot.

The operator will fail if the worker definition of the involved matrices
are different. As the areduce operator, this operator works in an
adoprive way, meaning if a worker is done processing a single, the next 
unpreocessed slot is assigned to this worker.

*/
ListExpr areduce2TM(ListExpr args){  

  string err = "dfmatrix(rel(X)) x dfmatrix(rel(Y)) x string x (fsrel(X) x "
               "fsrel(Y) -> Z) x int expected";

  // check length
  if(!nl->HasLength(args,5)){
    return listutils::typeError(err + " (wrong number of args)");
  }

  // check for usesArgsInTypeMapping
  ListExpr t = args;
  while(!nl->IsEmpty(t)){
     if(!nl->HasLength(nl->First(t),2)){
        return listutils::typeError("internal error");
     }
     t = nl->Rest(t);
  }
  ListExpr a1 = nl->First(args);
  ListExpr a2 = nl->Second(args);
  ListExpr a3 = nl->Third(args);
  ListExpr a4 = nl->Fourth(args);
  ListExpr a5 = nl->Fifth(args);

  ListExpr a1t = nl->First(a1);
  ListExpr a2t = nl->First(a2);
  ListExpr a3t = nl->First(a3);
  ListExpr a4t = nl->First(a4);
  ListExpr a5t = nl->First(a5);

  // check types
  if(   !DFMatrix::checkType(a1t)
     || !DFMatrix::checkType(a2t)
     || !CcString::checkType(a3t)
     || !listutils::isMap<2>(a4t)
     || !CcInt::checkType(a5t)){
    return listutils::typeError(err);
  }

  // Check function arguments
  ListExpr fa1 = nl->Second(a4t);
  ListExpr fa2 = nl->Third(a4t);

  ListExpr s1 = nl->TwoElemList(
       listutils::basicSymbol<fsrel>(),
       nl->Second(nl->Second(a1t))
  );
 
  ListExpr s2 = nl->TwoElemList(
       listutils::basicSymbol<fsrel>(),
       nl->Second(nl->Second(a2t))
  );

  if(   !nl->Equal(s1,fa1)
     || !nl->Equal(s2,fa2)){
    return listutils::typeError("Type mismatch between dfmatrix subtype "
                                "and function argument");
  }

  // determine result type

  ListExpr funRes =  nl->Fourth(a4t);

  ListExpr resType;
  
  bool isStream = false; 



 
  if(!listutils::isStream(funRes)){
     if(!Relation::checkType(funRes)){
        resType = nl->TwoElemList(
                      listutils::basicSymbol<DArray>(),
                      funRes);
     } else {
        resType =  nl->TwoElemList(
                        listutils::basicSymbol<DFArray>(),
                        funRes);
     }
  } else { // funresult is a stream
    isStream = true;
    if(!Stream<Tuple>::checkType(funRes)){
       return listutils::typeError("funtion result is a stream of non-tuples");
    }
    resType = nl->TwoElemList(
               listutils::basicSymbol<DFArray>(),
                  nl->TwoElemList(
                     listutils::basicSymbol<Relation>(),
                     nl->Second(funRes)
                  )
       );
  }

  ListExpr fundef = nl->Second(a4);

  // rewrite function argument types
  ListExpr rfundef = nl->FourElemList(
               nl->First(fundef), // just fun
               nl->TwoElemList(
                    nl->First(nl->Second(fundef)),
                    fa1),
               nl->TwoElemList(
                    nl->First(nl->Third(fundef)),
                    fa2),
               nl->Fourth(fundef)
           );

  ListExpr appendList = nl->TwoElemList(
         nl->TextAtom(nl->ToString(rfundef)),
         nl->BoolAtom(isStream)
  );

  return nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               appendList,
               resType);


}


/*
Specification

*/
OperatorSpec areduce2Spec(
  "dfmatrix(rel(X)) x dfmatrix(rel(Y) x string x (fsrel(X) x "
  "fsrel(Y) -> Z) x int -> d[f]array(Y)",
  "_ _ areduce2[_,_,_] ",
  "Performs areduce function to two dfmatrices. The result type depends "
  "on the return type of the operation. If the result is a stream of tuples "
  "or a relation, the rseult type will be a dfarray, otherwise a darray. "
  "Streams of non-tuples are not alowed as the function result type. "
  "The string argument specified the name of the result array. "
  "The integer specified a port for transferring files between the workers. "
  "On this port automatically a staticFileTransferator is started.",
  "query dfm1 dfm2 areduce2[ \"molly\", . feed  .. feed product , 1236]"
);

Operator areduce2Op(
  "areduce2",
  areduceSpec.getStr(),
  2,
  areduceVM,
  areduceSelect,
  areduce2TM
);

/*
29 collect2

*/



ListExpr collect2TM(ListExpr args){
  string err = "dfmatrix x string x int expected";
  if(!nl->HasLength(args,3)){
    return listutils::typeError(err+" (wrong number of args)");
  }
  if(   !DFMatrix::checkType(nl->First(args))
     || !CcString::checkType(nl->Second(args))
     || !CcInt::checkType(nl->Third(args))){
     return  listutils::typeError(err);
  }

  return nl->TwoElemList( 
              listutils::basicSymbol<DFArray>(),
              nl->Second(nl->First(args)));
}


class slotGetter{

  public:
    slotGetter(int _myNumber, string& _sname, string& _tname,
               int _size, 
               const vector<ConnectionInfo*>& _workers, int _port,
               string _constrel ):
       myNumber(_myNumber), sname(_sname), tname(_tname),
       size(_size), workers(_workers), port(_port), constrel(_constrel){
      
       runner = new boost::thread(&slotGetter::run, this);
     }


     ~slotGetter(){
        runner->join();
        delete runner;
     }



  private:
     int   myNumber; // workers number
     string sname;
     string tname;
     int   size;     // size of the array
     const vector<ConnectionInfo*>& workers; // all workers
     int port; // where listen the transferators
     boost::thread* runner;
     string constrel;




     void run(){
       // firstly, create a temp directory for all of my slots
       ConnectionInfo* ci = workers[myNumber];
       // temoporal directory for partitined slots
       string dir = "tmp/"+tname+"/"+stringutils::int2str(myNumber)+"/";
       // final directory for dfarray
       string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
       string tdir = ci->getSecondoHome()+"/dfarrays/"+dbname+"/"+tname+"/";
       int err;
       string errMsg;
       double runtime;
       string res;
       string cmd = "query createDirectory('"+dir+"',TRUE)";
       ci->simpleCommand(cmd, err, errMsg, res, false, runtime);
       if(err!=0){
          cerr << "error during cmd " << cmd << endl;
          return;
       }
       cmd = "query createDirectory('"+tdir+"',TRUE)";
       ci->simpleCommand(cmd, err, errMsg, res, false, runtime);
       if(err!=0){
          cerr << "error during cmd " << cmd << endl;
          return;
       }

       // get all slots managed by worker with mynumber  from all workers
       for(size_t w=0;w<workers.size();w++){
          getFilesFromWorker(w, dir, ci);
       } 

       // create slots from distributed slots
       int slot = myNumber;
       while(slot < size){
          createSlot(slot, dir,tdir, ci);
          slot += workers.size(); 
       }

       cmd = "query  removeDirectory('"+dir+"', TRUE)";
       ci->simpleCommand(cmd, err, errMsg, res, false, runtime);
       if(err){
          cerr << "Error in command " << cmd << endl;
          cerr << errMsg;
       }

     }


     void getFilesFromWorker(size_t worker, const string& myDir, 
                             ConnectionInfo* ci){

           
         int err;
         string errMsg;
         double runtime;
         string res;
         string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
         
         ConnectionInfo* w = workers[worker];
         string sbasename = w->getSecondoHome() + "/dfarrays/" + dbname + "/"
                            + sname + "/" + stringutils::int2str(worker) + "/"
                            + sname+"_";
         string frel = "[const rel(tuple([S : text, T:text])) value (";
    
         int slot = myNumber;
         string ws = stringutils::int2str(worker);

         while(slot<size){
            string n = stringutils::int2str(slot);
            frel += "( '"+ sbasename + n+".bin'  '"+myDir+"s_"+n+"_"+ws+"')";
            slot += workers.size();
         }
         frel += ")]";
 
         string cmd = "query  "+ frel + " feed extend[ OK : getFileTCP(.S, '" 
                       + w->getHost() + "', " + stringutils::int2str(port) 
                       + ", TRUE, .T )] count";

         
         ci->simpleCommand(cmd, err, errMsg, res, false, runtime);
         if(err){
            cerr << "Error in command " << cmd << endl;
            cerr << errMsg;
         }
     } 


     void createSlot(int num, const string& sdir, const string& tdir, 
                     ConnectionInfo* ci){

        string cmd = " query getDirectory('" + sdir + "') "
                     + "filter[basename(.) startsWith \"s_" 
                     + stringutils::int2str(num) + "_\"]"
                     + " fsfeed5[" + constrel + "] fconsume5['" + tdir 
                     + tname+"_" + stringutils::int2str(num) + ".bin'] count";
       int err;
       string errMsg;
       double runtime;
       string res;
       ci->simpleCommand(cmd, err, errMsg, res, false, runtime);
       if(err!=0){
          cerr << __FILE__ << "@"  << __LINE__ << endl;
          showError(ci,cmd,err,errMsg);
       }
     
     }
};


int collect2VM(Word* args, Word& result, int message,
             Word& local, Supplier s ){

   result = qp->ResultStorage(s);
   DFMatrix* matrix = (DFMatrix*) args[0].addr;
   CcString* name   = (CcString*) args[1].addr;
   CcInt* port = (CcInt*) args[2].addr;
   DFArray* res = (DFArray*) result.addr;

   if(!matrix->IsDefined() || !name->IsDefined() || !port->IsDefined()){
      res->makeUndefined();
      return 0;
   }
   string n = name->GetValue();
   int p = port->GetValue();

   if(n==""){
      n = algInstance->getTempName();
   }

   if(!stringutils::isIdent(n) || (port<=0)){
     res->makeUndefined();
     return 0;
   }
   res->copyFrom(*matrix);
   res->setName(n);

   ListExpr relType = nl->Second(qp->GetType(s));

   string constRel = "[ const " + getUDRelType(relType)+" value ()]";


   // step 1 create base for file transfer 
   startFileTransferators(matrix,p);
   // copy slots parts from other workers to target worker
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   vector<ConnectionInfo*> cis;
   for(size_t i=0;i<matrix->numOfWorkers();i++){
       ConnectionInfo* ci  = 
               algInstance->getWorkerConnection(matrix->getWorker(i), dbname);
       if(ci){
          cis.push_back(ci);
       }
   }

   vector<slotGetter*> getters;
   string sname = matrix->getName();
    
   for(size_t i=0;i<cis.size();i++){
      slotGetter* getter = new slotGetter(i, sname,n, matrix->getSize(), 
                                          cis, p, constRel);
      getters.push_back(getter);
   }

   for(size_t i=0;i<cis.size();i++){
      delete getters[i];
   }

   res->setStdMap(matrix->getSize());

   return 0;
}

OperatorSpec collect2Spec(
  "dfmatrix x string x int -> dfarray",
  " _ collect2[ _ , _] ",
  "Collects the slots of a matrix into a "
  " dfarray. The string is the name of the "
  "resulting array, the int value specified a "
  "port for file transfer. The port value can be any "
  "port usable on all workers. A corresponding file transfer "
  "server is started automatically.",
  "query m8 collect2[\"a8\",1238]"
);

Operator collect2Op(
  "collect2",
  collect2Spec.getStr(),
  collect2VM,
  Operator::SimpleSelect,
  collect2TM
);


/*
10.29 Operator saveAttr

*/

ListExpr saveAttrTM(ListExpr args){
  string err = "attr x {string,text} expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of args)");
  }
  if(   !Attribute::checkType(nl->First(args))
     || (  !CcString::checkType(nl->Second(args))
         &&!FText::checkType(nl->Second(args)))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcBool>();
}

template<class T>
int saveAttrVMT(Word* args, Word& result, int message,
             Word& local, Supplier s ){

   Attribute* attr = (Attribute*) args[0].addr;
   T* filename = (T*) args[1].addr;
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   if(!filename->IsDefined()){
     res->SetDefined(false);
     return 0;
   }

   res->Set(true,  FileAttribute::saveAttribute(
                                qp->GetType(qp->GetSon(s,0)),
                                attr, filename->GetValue()));
   return 0;
}

ValueMapping saveAttrVM[]={
   saveAttrVMT<CcString>,
   saveAttrVMT<FText>
};

int saveAttrSelect(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}

OperatorSpec saveAttrSpec(
  "DATA x {string,text} -> bool",
  " _ saveAttr[_] ",
  "saves an attribute in binary format "
  "to a file",
  "query 6 saveAttr['six.bin']"
);

Operator saveAttrOp(
  "saveAttr",
  saveAttrSpec.getStr(),
  2,
  saveAttrVM,
  saveAttrSelect,
  saveAttrTM

);


/*
2.31 Operator ~loadAttr~

*/
ListExpr loadAttrTM(ListExpr args){

  string err = " string or text expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err +" ( wrong number of args)");
  }
  ListExpr arg = nl->First(args);
  if(!nl->HasLength(arg,2)){
    return listutils::typeError("internal error");
  }
  ListExpr a = nl->First(arg);
  if(!CcString::checkType(a) && !FText::checkType(a)){
    return listutils::typeError(err);
  }
  string fname;
  bool ok;
  if(CcString::checkType(a)){
    ok = getValue<CcString>(nl->Second(arg),fname);
  } else {
    ok = getValue<FText>(nl->Second(arg),fname);
  }
  if(!ok){
    return listutils::typeError("could not evaluate filename in Type Mapping");
  }
  ListExpr res = FileAttribute::getType(fname);
  if(nl->IsEmpty(res)){
     return listutils::typeError("could not extract type from file");
  }
  if(!listutils::isDATA(res)){
     return listutils::typeError("not an attribute in file");
  }
  return res;
}


template<class T>
int loadAttrVMT(Word* args, Word& result, int message,
             Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  Attribute* res = (Attribute*) result.addr;

  T* fn = (T*) args[0].addr;
  if(!fn->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  ListExpr type;
  Attribute* a = FileAttribute::restoreAttribute(type,fn->GetValue());
  if(!a){
     res->SetDefined(false);
     return 0;
  }
  res->CopyFrom(a);
  a->DeleteIfAllowed();
  return 0;
}

ValueMapping loadAttrVM[] = {
  loadAttrVMT<CcString>,
  loadAttrVMT<FText>
};

int loadAttrSelect(ListExpr args){
 return (CcString::checkType(nl->First(args)))?0:1;
}

OperatorSpec loadAttrSpec(
  "{string,text} -> DATA ",
  "loadAttr(_)",
  "loads an attribute from a binary file",
  "query loadAttribute('six.bin')"
);

Operator loadAttrOp(
  "loadAttr",
  loadAttrSpec.getStr(),
  2,
  loadAttrVM,
  loadAttrSelect,
  loadAttrTM
);


/*

2.32 Operator ~createFrel~

*/

ListExpr createFrelTM(ListExpr args){

 string err = "{string,text} [ x rel [ x bool ]] expected";
 if(!nl->HasLength(args,1) && !nl->HasLength(args,2) 
    && !nl->HasLength(args,3)){
   return listutils::typeError(err + " (wrong number of args)");
 }
 // check usesArgs in TypeMapping
 ListExpr t = args;
 while(!nl->IsEmpty(t)){
   if(!nl->HasLength(nl->First(t),2)){
      return listutils::typeError("internal Error");
   }
   t = nl->Rest(t);
 }

 ListExpr arg1t = nl->First(nl->First(args));


 if(    !CcString::checkType(arg1t) && !FText::checkType(arg1t)){
   return listutils::typeError(err + 
                      " (first arg not of type string or text");
 }
 if(nl->HasLength(args,1)){
   // only the name is given, retrieve type from file
   string filename;
   bool ok;
   if(CcString::checkType(arg1t)){
      ok = getValue<CcString>(nl->Second(nl->First(args)), filename);
   } else {
      ok = getValue<FText>(nl->Second(nl->First(args)), filename);
   }
   if(!ok){
     return listutils::typeError("could not retrieve filename from " 
                                 + nl->ToString(nl->Second(nl->First(args))));
   }
   ffeed5Info fi(filename);
   if(!fi.isOK()){
     return listutils::typeError("file " + filename 
                                 + " not present or contains no relation");
   }
   ListExpr relType = fi.getRelType();
   return nl->TwoElemList( listutils::basicSymbol<frel>(),
                           nl->Second(relType));
 }
 // at least two arguments
 ListExpr relType = nl->First(nl->Second(args));
 if(!Relation::checkType(relType)){
    return listutils::typeError(err + " (second arg is not a relation)");
 }
 if(nl->HasLength(args,3)){
    if(!CcBool::checkType(nl->First(nl->Third(args)))){
        return listutils::typeError(err + " ( third arg not a bool )");
    }
 }
 return nl->TwoElemList( listutils::basicSymbol<frel>(),
                          nl->Second(relType));   

}



template<class T>
int createFrelVMT(Word* args, Word& result, int message,
             Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  frel* res = (frel*) result.addr;

  T* fname = (T*) args[0].addr;
  if(!fname->IsDefined()){
    res->SetDefined(false);
    return 0;
  }  
  string fn = fname->GetValue();
  res->set(fn);
  if(qp->GetNoSons(s)==1 || qp->GetNoSons(s)==2){
    return 0;
  }
  assert(qp->GetNoSons(s)==3);
 
  CcBool* over = (CcBool*) args[2].addr;
  if(!over->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  // create relation 
  if(!over->GetValue()){ // do not allow overwrite
     fstream in (fn.c_str(), ios::in);
     if(in.good()){
       res->SetDefined(false);
       return 0;
     }  
  }
  ListExpr relType = nl->TwoElemList( listutils::basicSymbol<Relation>(),
                                      nl->Second(qp->GetType(s))); 

  bool ok = BinRelWriter::writeRelationToFile( (Relation*) args[1].addr,
                                               relType, fn);

  if(!ok){
    res->SetDefined(false);
  }
  return 0;
}


ValueMapping createFrelVM[] = {
   createFrelVMT<CcString>,
   createFrelVMT<FText>
};

int createFrelSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec createFrelSpec(
  " {string,text} [ x rel(tuple(X)) [ x bool ] ] -> frel(tuple(x))",
  " createFRel(filename,relation,overwrite)",
  "This operator creates an frel instance. "
  "If only the first argument is given, the relation type is extracted "
  "in type mapping from the file. " 
  "If the filename and the relation are given, the rekation type is "
  "taken from the relation. If all argument are given, the file is created"
  " from the relation. The overwrite argument controls whether an "
  "existing file should be overwritten.",
  " query createFrel('ten.bin', ten, TRUE)"
);

Operator createFrelOP(
 "createFrel",
 createFrelSpec.getStr(),
 2,
 createFrelVM,  
 createFrelSelect,
 createFrelTM
);



/*

2.32 Operator ~createFrel~

*/
ListExpr createFSrelTM(ListExpr args){
  string err = " stream({text,string}) x rel expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of args)");
  }
  if(   !Stream<FText>::checkType(nl->First(args))
     && !Stream<CcString>::checkType(nl->First(args))){
    return listutils::typeError(err + " (first arg has invalid type)");
  }
  if(!Relation::checkType(nl->Second(args))){
     return listutils::typeError(err + " ( second arg is not a relation)");
  }

  return nl->TwoElemList(
                listutils::basicSymbol<fsrel>(),
                nl->Second(nl->Second(args)));
}


template<class T>
int createFSrelVMT(Word* args, Word& result, int message,
             Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  fsrel* res = (fsrel*) result.addr;
  res->clear();
  Stream<T> stream(args[0]);
  stream.open();
  T* name;
  while( (name = stream.request())!=0){
     if(name->IsDefined()){
        res->append(name->GetValue());
     }
     name->DeleteIfAllowed();
  }
  stream.close();
  return 0;
}

ValueMapping createFSrelVM[] = {
   createFSrelVMT<CcString>,
   createFSrelVMT<FText>
};

int createFSrelSelect(ListExpr args){
  return Stream<CcString>::checkType(nl->First(args))?0:1;
}

OperatorSpec createFSrelSpec(
  " stream({string,text}) x rel(x) -> fsrel(x)",
  " _ createFSRel[_]",
  "This operator creates an fsrel object. "
  "The relation is a type template, it's value is not used."
  "All defined elements in the stream are collected within the "
  "result. There is no duplicate elemination.",
  " query names feed namedtransformstream[File] createfsrel[ten]"
);

Operator createFSrelOP(
 "createFSrel",
 createFSrelSpec.getStr(),
 2,
 createFSrelVM,  
 createFrelSelect,
 createFSrelTM
);


/*
3.98 Operator saveObjectToFile

This operator gets the name of a database object and stores it into a file.
If the object is a relation or in kind DATA, a special binary file is created,
in all other cases, the file will contain a binary nested list.

*/

ListExpr saveObjectToFileTM(ListExpr args){
  string err = " O x {text, string} expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError("two args expected");
  }

  ListExpr a1 = nl->First(args);
  if(nl->HasLength(a1,2) && listutils::isStream(nl->First(a1))){
     if(!Stream<Tuple>::checkType(a1)){
       return listutils::typeError("only streams or normal objects are "
                                   "allowed as the first argument");
     }
     a1 = nl->TwoElemList( listutils::basicSymbol<Relation>(),
                           nl->Second(a1));
  }
  ListExpr a2 = nl->Second(args);
  if(!CcString::checkType(a2) && !FText::checkType(a2)){
    return listutils::typeError(err + " (invalid type for 2nd argument)");
  }
  
  return nl->TwoElemList( listutils::basicSymbol<fobj>(),
                          a1);
}


/*
3.98.1 

*/
template<class T>
int saveObjectToFileVMT(Word* args, Word& result, int message,
             Word& local, Supplier s ){
   ListExpr targetlist = nl->Second(qp->GetType(s));
   result = qp->ResultStorage(s);
   fobj* res = (fobj*) result.addr;
   T* name = (T*) args[1].addr;
   if(!name->IsDefined()){
      res->SetDefined(false);
      return 0; 
   }
   string n = name->GetValue();
   res->set(n);
   ListExpr st = qp->GetType(qp->GetSon(s,0));

   // create target directory
   string dir = FileSystem::GetParentFolder(n);
   FileSystem::CreateFolderEx(dir);


   if(Stream<Tuple>::checkType(st)){
     ofstream out(n.c_str(),ios::out|ios::binary);
     if(!out){
      res->SetDefined(false);
      return 0; 
     }
     BinRelWriter::writeHeader(out,nl->Second(targetlist));
     Stream<Tuple> stream(args[0]);
     stream.open();
     Tuple* t;
     while((t=stream.request())){
         BinRelWriter::writeNextTuple(out,t);
         t->DeleteIfAllowed();
     }
     stream.close();
     out.close();
     return 0;
   } else if(Relation::checkType(st)){
     Relation* rel = (Relation*) args[0].addr;
     if(!BinRelWriter::writeRelationToFile(rel,st,n)){
        res->SetDefined(false);
     }
     return 0;
   } else if(Attribute::checkType(st)){
      if(!FileAttribute::saveAttribute(st,(Attribute*) args[0].addr, n)){
          res->SetDefined(false);
      }
      return 0;
   } else {
      SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
      int algId;
      int typeId; 
      string sn;
      if(!ctlg->LookUpTypeExpr(st,sn,algId, typeId)){
         res->SetDefined(false);
         return 0;
      }
      OutObject o = am->OutObj(algId,typeId);
      ListExpr vlist = o(st,args[0]);
      ListExpr clist = nl->TwoElemList(st,vlist);
      ofstream out(n.c_str(),ios::out|ios::binary);
      if(!out){
         res->SetDefined(false);
         return 0; 
      }
      nl->WriteBinaryTo(clist, out);
      out.close();
      return 0;
   }
}


ValueMapping  saveObjectToFileVM[] = {
  saveObjectToFileVMT<CcString>,
  saveObjectToFileVMT<FText>
};

int saveObjectToFileSelect(ListExpr args){
 return CcString::checkType(nl->Second(args))?0:1;
}

OperatorSpec saveObjectToFileSpec(
  "D x {string, text} -> fobj(D)",
  " _ saveObjectToFile[_]",
  "Saves an object into a binary file.",
  "query strassen saveObjectToFile['strassen.bin']"
);

Operator saveObjectToFileOp(
  "saveObjectToFile",
  saveObjectToFileSpec.getStr(),
  2,
  saveObjectToFileVM,
  saveObjectToFileSelect,
  saveObjectToFileTM
);


/*
4.44 ~getObjectFromFile~

*/

ListExpr getFileType(const string& fn, string& errorMsg){
  if(!FileSystem::FileOrFolderExists(fn)){
     errorMsg = "file " + fn + " does not exist";
     return nl->TheEmptyList();
  }

  // first try: a relation
  ffeed5Info info(fn);
  if(info.isOK()){
     return info.getRelType();
  }
  ListExpr r = FileAttribute::getType(fn);
  if(!nl->IsEmpty(r)){
     return r;
  }
  // last possibility: a binary stored nested list
  // TODD: avoid complete reading of the list, support in 
  // nested list module required
  ifstream in(fn.c_str(), ios::in | ios::binary);
  if(!in){
    return nl->TheEmptyList();
  }
  nl->ReadBinaryFrom(in, r);
  if(!nl->HasLength(r,2)){
     errorMsg = "file " + fn + " has an unknown format";
     return nl->TheEmptyList();
  }
  return nl->First(r);
}

ListExpr getObjectFromFileTM(ListExpr args){
  bool isstream = false;
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  ListExpr a1t = nl->First(nl->First(args));
  if(   !CcString::checkType(a1t)
     && !FText::checkType(a1t)
     && !fobj::checkType(a1t)){
    return listutils::typeError("string, text, or fobj expected");
  }
  if(fobj::checkType(a1t)){
    ListExpr subtype = nl->Second(a1t);
    if(Relation::checkType(subtype)){
       subtype = nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                                 nl->Second(subtype));
       isstream = true;
    } 
    return nl->ThreeElemList( 
                 nl->SymbolAtom(Symbols::APPEND()),
                 nl->OneElemList(nl->BoolAtom(isstream)),
                 subtype);
  }
  ListExpr a1v = nl->Second(nl->First(args));
  int at = nl->AtomType(a1v);
  if(at!=StringType && at!=TextType){
    return listutils::typeError("only constant expressions are allowed");
  }
  string fn = at==StringType?nl->StringValue(a1v):nl->Text2String(a1v);
  if(!FileSystem::FileOrFolderExists(fn)){
    return listutils::typeError("file " + fn + " does not exist");
  }
  string msg;
  ListExpr r = getFileType(fn,msg);
  if(nl->IsEmpty(r)){
    return listutils::typeError(msg);
  } 
  if(Relation::checkType(r)){
     r = nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                                 nl->Second(r));
       isstream = true;
  } 
  return nl->ThreeElemList( 
                 nl->SymbolAtom(Symbols::APPEND()),
                 nl->OneElemList(nl->BoolAtom(isstream)),
                  r);
}



template<class T>
int getObjectFromFileVM_Stream(Word* args, Word& result, int message,
             Word& local, Supplier s ){

     ffeed5Info* li = (ffeed5Info*) local.addr;
     switch(message){
       case OPEN:{
              if(li){
                 delete li;
                 local.addr = 0;
              }
              T* fN = (T*) args[0].addr;
              if(!fN->IsDefined()){
                return 0;
              }
              string n = fN->GetValue();
              li = new ffeed5Info(n);
              if(!li->isOK()){
                 cout << "could not create ffeed4Infro from " << n << endl;
                 delete li;
                 return 0;
              }
              ListExpr filetype = li->getRelType();
              if(!Relation::checkType(filetype)){
                 delete li;
                 return 0;
              }
              if(!nl->Equal(nl->Second(filetype), nl->Second(qp->GetType(s)))){
                 delete li;
                 return 0;
              }
              local.addr = li;
              return 0;
       }

       case REQUEST:
             result.addr = li?li->next():0;
             return result.addr?YIELD:CANCEL;

       case CLOSE:
             if(li){
               delete li;
               local.addr = 0;
             }
             return 0;
     } 
     return -1; 
}

template<class T>
int getObjectFromFileVM_Single(Word* args, Word& result, int message,
             Word& local, Supplier s ){

    ListExpr t = qp->GetType(s);
    bool isDATA = Attribute::checkType(t);
    T* fN = (T*) args[0].addr;
    result = qp->ResultStorage(s);


    if(fN->IsDefined()){
       if(isDATA){
          string n = fN->GetValue();
          ListExpr t2;         
          Attribute* a = FileAttribute::restoreAttribute(t2, n);
          if(a && !nl->Equal(t,t2)){
            delete a;
            a=0;
          }
          if(!a){
            cout << "could not restore attribute from bin file " << n << endl;
            ((Attribute*) result.addr)->SetDefined(false);
          }  else {
            ((Attribute*) result.addr)->CopyFrom(a);
            delete a;
          }
       } else {
          ListExpr list;
          ifstream in(fN->GetValue().c_str(), ios::in | ios::binary);
          if(!in){
            cout << "could note create input stream from file " << fN << endl;
            return 0;
          }
          if(!nl->ReadBinaryFrom(in, list)){
            cout << "could parse binary file " << fN << endl;
            in.close();
            return 0;
          }
          in.close();
          int algId, typeId;
          SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
          string dummy;
          if(!ctlg->LookUpTypeExpr(qp->GetType(s), dummy, algId, typeId)){
             return 0;
          }
          InObject infn = am->InObj(algId, typeId);
          int errorPos=0;
          ListExpr errorInfo = listutils::emptyErrorInfo();
          bool correct;
          Word k = infn(qp->GetType(s), list, errorPos, errorInfo, correct);
          if(!correct){
            return 0;
          }
          qp->ChangeResultStorage(s,k);
          return 0;
       }
    } else {
       if(isDATA){
        ((Attribute*) result.addr)->SetDefined(false);
       }
       return 0;
    }
    return -1;
}


template<class T>
int getObjectFromFileVMT(Word* args, Word& result, int message,
             Word& local, Supplier s ){

   if(((CcBool*)args[1].addr)->GetValue()){
      return getObjectFromFileVM_Stream<T>(args, result, message, local, s);
   } else {
      return getObjectFromFileVM_Single<T>(args, result, message, local, s);

   }
}

ValueMapping getObjectFromFileVM[] = {
   getObjectFromFileVMT<CcString>,
   getObjectFromFileVMT<FText>,
   getObjectFromFileVMT<fobj>
};

int getObjectFromFileSelect(ListExpr args){
 
  ListExpr a = nl->First(args);
  return CcString::checkType(a)?0:
            (FText::checkType(a)?1:2);
}



OperatorSpec getObjectFromFileSpec(
  " {string, text, fobj(X) -> [stream] X",
  " _ getObjectFromFile ",
  "Retrieves an object stored within a binary file. "
  "If the file contains a relation, the result will be "
  " a tuple stream. " ,
  " query \"strassen.bin\" getObjectFromFile count"
);

Operator getObjectFromFileOp(
  "getObjectFromFile",
  getObjectFromFileSpec.getStr(),
  3,
  getObjectFromFileVM,
  getObjectFromFileSelect,
  getObjectFromFileTM
);


/*
98 Operator ~ddistribute8~

This operator partitions a tuple stream into a array(darray)
corresponding to two functions.

*/
ListExpr ddistribute8TM(ListExpr args){
    // tuples , name, size1 , size2, fun1 , fun2, workers
    string err = "stream(tuple(X)) x string x int x int x "
                 "(X -> int) x (X->int) x rel expected";
   if(!nl->HasLength(args,7)){
     return listutils::typeError(err + " (wrong number of args)");
   }
   if(!Stream<Tuple>::checkType(nl->First(args))){
     return listutils::typeError(err + " (first arg is not a tuple stream)");
   }
   if(! CcString::checkType(nl->Second(args))){
      return listutils::typeError(err+ " (second arg is not a string)");
   }
   if(!listutils::isMap<1>(nl->Third(args))){
      return listutils::typeError(err+ " (3th arg is not an unary function");
   }
   if(!listutils::isMap<1>(nl->Fourth(args))){
      return listutils::typeError(err+ " (4th arg is not an unary function");
   }
   if(!CcInt::checkType(nl->Fifth(args))){
      return listutils::typeError(err+ " (third arg is not an int)");
   }
   if(!CcInt::checkType(nl->Sixth(args))){
      return listutils::typeError(err+ " (fourth arg is not an int)");
   }
   ListExpr workerPositions;
   ListExpr workerTypes;
   string errMsg;
   if(!isWorkerRelDesc(nl->Sixth(nl->Rest(args)), workerPositions, 
                       workerTypes, errMsg)){
      return listutils::typeError(err+ " (7th arg is not a worker relation: "
                                  + errMsg);
   }

   // check function arguments and result
   ListExpr tupleType = nl->Second(nl->First(args));
   ListExpr fun1Arg = nl->Second(nl->Third(args));
   ListExpr fun1Res = nl->Third(nl->Third(args));
   if(!nl->Equal(tupleType,fun1Arg)){
     return listutils::typeError(err + " (fun arg 1 does in unequal "
                                       "to the tuple type)");
   } 
   if(!CcInt::checkType(fun1Res)){
      return listutils::typeError(err + " (fun res 1 is not an int) ");
   }

   ListExpr fun2Arg = nl->Second(nl->Fourth(args));
   ListExpr fun2Res = nl->Third(nl->Fourth(args));
   if(!nl->Equal(tupleType,fun2Arg)){
     return listutils::typeError(err + " (fun arg 2 does in unequal "
                                       "to the tuple type)");
   } 
   if(!CcInt::checkType(fun2Res)){
      return listutils::typeError(err + " (fun res 2 is not an int) ");
   }
   
   ListExpr appendList = listutils::concat(workerPositions, 
                         nl->TwoElemList( 
                             nl->BoolAtom(
                                 CcString::checkType(nl->First(workerTypes))),
                             nl->BoolAtom(
                                 CcString::checkType(nl->Third(workerTypes)))));
    
   ListExpr subType = nl->TwoElemList(
                         listutils::basicSymbol<Relation>(),
                         nl->Second(nl->First(args)));

   ListExpr daType = nl->TwoElemList(
                            listutils::basicSymbol<DArray>(),
                            subType);
   ListExpr resType = nl->TwoElemList(
                          listutils::basicSymbol<arrayalgebra::Array>(),
                          daType);
   return nl->ThreeElemList(
             nl->SymbolAtom(Symbols::APPEND()),
             appendList,
             resType
          );
                                              
}



template<class AType, class HType, class CType, class  DType>
int ddistribute8VMT(Word* args, Word& result, int message,
             Word& local, Supplier s ){

  result=qp->ResultStorage(s);
  arrayalgebra::Array* res = (arrayalgebra::Array*) result.addr;
  CcString* ccName = (CcString*) args[1].addr;
  CcInt* ccSize1 = (CcInt*) args[4].addr;
  CcInt* ccSize2 = (CcInt*) args[5].addr;
  int hostPos = ((CcInt*) args[7].addr)->GetValue();
  int portPos = ((CcInt*) args[8].addr)->GetValue();
  int configPos = ((CcInt*) args[9].addr)->GetValue();

  if(!ccName->IsDefined() || !ccSize1->IsDefined() || !ccSize2->IsDefined()){
     res->setUndefined();
     return 0;
  }

  string name = ccName->GetValue();
  int size1 = ccSize1->GetValue(); // size of the outer array
  int size2 = ccSize2->GetValue(); // sizes of the inner distributed arrays
  if(size1<1 || size2<1 || !stringutils::isIdent(name)){
     res->setUndefined();
     return 0;
  }

  // create a darray template for filling up all array slots
  AType temp = DArrayBase::createFromRel<HType, CType, AType>(
               (Relation*) args[6].addr, 
                size2,"blah", hostPos, portPos, configPos );  
  
  if(temp.numOfWorkers()< 1){
     res->setUndefined();
     return 0;
  } 

  // fill up the result array 
  ListExpr at = listutils::basicSymbol<AType>();
  string tname;
  SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
  int algId;
  int typeId;
  if(!ctlg->LookUpTypeExpr(at, tname, algId, typeId)){
    assert(false);
  }
  Word darrays[size1];
  for(int i=0;i< size1; i++){
     string slotname = name +"_" + stringutils::int2str(i);
     AType* a = new AType(temp);
     a->setName(slotname);
     darrays[i].addr = a;    
  }
  res->initialize(algId, typeId, size1, darrays);


  // result is ready, now distribute the data

  // first step of distribution: distribute to files
  Stream<Tuple> stream(args[0]);
  stream.open();
  Tuple* tuple;
  ArgVectorPointer fun1arg = qp->Argument(args[2].addr);
  Word fun1result;
  ArgVectorPointer fun2arg = qp->Argument(args[3].addr);
  Word fun2result;

  int nstreams = size1*size2;
  ostream* outputs[nstreams];
  // create empty file to write in data
  ListExpr relType = nl->Second(nl->Second(qp->GetType(s)));

  for(int i=0;i<nstreams;i++){
    int array_num = i / size2;
    int darray_num = i % size2;
    string fname = name + "_" + stringutils::int2str(array_num) 
                   + "_" + stringutils::int2str(darray_num);
    outputs[i] = new ofstream(fname.c_str(),ios::out|ios::binary);
    BinRelWriter::writeHeader(*outputs[i],relType);
  }



  while( (tuple = stream.request())!=0){
     (*fun1arg)[0].addr = tuple;
     (*fun2arg)[0].addr = tuple;
      qp->Request(args[2].addr, fun1result);
      qp->Request(args[3].addr, fun2result);
      CcInt* f1res = (CcInt*) fun1result.addr;
      CcInt* f2res = (CcInt*) fun2result.addr;
      int f1 = f1res->IsDefined()?f1res->GetValue():0;
      int f2 = f2res->IsDefined()?f2res->GetValue():0;
      f1 = f1 % size1;
      f2 = f2 % size2;
      int pos = f1*size2 + f2; 
      BinRelWriter::writeNextTuple(*outputs[pos], tuple);
      tuple->DeleteIfAllowed();
  }

  // finish the files
  for(int i=0;i<nstreams;i++){
    BinRelWriter::finish(*outputs[i]);
    delete outputs[i]; 
  }
  // next step: transfer the files to the target computers
 
  // collect vectors of object and file names (which are the same)
  vector<vector<string> > names;
  for(size_t islots=0;islots<size2;islots++){
     vector<string> snames;
     for(size_t oslots=0;oslots<size1;oslots++){
         snames.push_back(name + "_" + stringutils::int2str(oslots)
                               + "_" + stringutils::int2str(islots));
     }
     names.push_back(snames);
  }
  vector<DType*> transfers;
  for(size_t i=0;i<names.size();i++){
     DType* transfer = new DType(names[i], (AType*) darrays[0].addr, 
                                 i, names[i]);
     transfer->start();
     transfers.push_back(transfer);
  }
  for(size_t i=0;i<transfers.size();i++){
     delete transfers[i];
  }

  return 0;
}


ValueMapping ddistribute8VM[] = {
  ddistribute8VMT<DArray, CcString, CcString, RelFileRestorer>,
  ddistribute8VMT<DArray, CcString, FText, RelFileRestorer>,
  ddistribute8VMT<DArray, FText, CcString, RelFileRestorer>,
  ddistribute8VMT<DArray, FText, FText, RelFileRestorer>
};


int ddistribute8Select(ListExpr args){
   ListExpr workerPositions;
   ListExpr workerTypes;
   string errMsg;
   if(!isWorkerRelDesc(nl->Sixth(nl->Rest(args)), workerPositions, 
                       workerTypes, errMsg)){
      assert(false); // type map and selection are not compatible
   }
   int n1 = CcString::checkType(nl->First(workerTypes))?0:2;
   int n2 = CcString::checkType(nl->Third(workerTypes))?0:1;
   return n1 + n2;
}

OperatorSpec ddistribute8Spec(
  "stream(tuple) x string x (tuple->int) x (tuple->int) x int x int x rel -> "
  "array(darray(rel(tuple)))",
  "_ ddistribute8[_,_,_,_,_,_]",
  "Distributes a tuple stream into an array of darrays."
  "The first int argument  specifies the size of the main array, the second"
  " int "
  "specifies the number of slots of the contained darrays. "
  "The first function determines the slot within  the main array, whereas the "
  "second function determines the slot within the distributed array.",
  "query plz feed ddistribute8[\"plzd8\", .PLZ, hashvalue(.Ort,58], "
  "4, 12 ,workers] "
);

Operator ddistribute8Op(
  "ddistribute8",
  ddistribute8Spec.getStr(),
  4,
  ddistribute8VM,
  ddistribute8Select,
  ddistribute8TM
);


ValueMapping dfdistribute8VM[] = {
  ddistribute8VMT<DFArray, CcString, CcString, FRelCopy>,
  ddistribute8VMT<DFArray, CcString, FText, FRelCopy>,
  ddistribute8VMT<DFArray, FText, CcString, FRelCopy>,
  ddistribute8VMT<DFArray, FText, FText, FRelCopy>
};

OperatorSpec dfdistribute8Spec(
  "stream(tuple) x string x (tuple->int) x (tuple->int) x int x int x rel -> "
  "array(dfarray(rel(tuple)))",
  "_ dfdistribute8[_,_,_,_,_,_]",
  "Distributes a tuple stream into an array of dfarrays."
  "The first iut specifies the size of the array, the second int "
  "specifies the number of slots of the contained dfarrays. "
  "The first function determines the slot in the main arrays, whereas the "
  "second function determines the slot within the distributed array.",
  "query plz feed ddistribute8[\"plzd8\", .PLZ, hashvalue(.Ort,58], "
  "4, 12 ,workers] "
);

Operator dfdistribute8Op(
  "dfdistribute8",
  dfdistribute8Spec.getStr(),
  4,
  dfdistribute8VM,
  ddistribute8Select,
  ddistribute8TM
);




/*
97 operator partition8Local

This operator gets a tuple stream, two functions, two strings and 
two integer values and distributes the tuple stream according to
the functions and the sizes into a subdirectory given by one string
treated as home according to  dfmatrixes with names.

*/

ListExpr partition8LocalTM(ListExpr args){
  if(!nl->HasLength(args,8)){
     return listutils::typeError("8 arguments expected");
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
     return listutils::typeError("first arg is not a tuple stream");
  }
  if(!listutils::isMap<1>(nl->Second(args))){
     return listutils::typeError("second arg is not an unary function");
  }
  if(!listutils::isMap<1>(nl->Third(args))){
     return listutils::typeError("Third arg is not an unary function");
  }
  if(!FText::checkType(nl->Fourth(args))){
     return listutils::typeError("fourth arg is not a text");
  }
  if(!CcString::checkType(nl->Fifth(args))){
     return listutils::typeError("fifth arg is not a string");
  }
  if(!CcInt::checkType(nl->Sixth(args))){
     return listutils::typeError("sixth arg is not an int");
  }
  if(!CcInt::checkType(nl->Sixth(nl->Rest(args)))){
     return listutils::typeError("seventh arg is not an int");
  } 
  if(!CcInt::checkType(nl->Sixth(nl->Rest(nl->Rest(args))))){
     return listutils::typeError(" eight arg is not an int");
  } 
  // check whether the function arguments corresponds to the
  // tuple type of the tuple stream
  ListExpr tt = nl->Second(nl->First(args));
  ListExpr fun1 = nl->Second(args);
  ListExpr fun2 = nl->Third(args);
  if(!nl->Equal(tt, nl->Second(fun1))){
     return listutils::typeError("argument of function one is not equal "
                                 "to the tuple type of the stream");
  } 
  if(!nl->Equal(tt, nl->Second(fun2))){
     return listutils::typeError("argument of function two is not equal "
                                 "to the tuple type of the stream");
  } 
  if(!CcInt::checkType(nl->Third(fun1))){
    return listutils::typeError("result of the first function is not an int");
  }
  if(!CcInt::checkType(nl->Third(fun2))){
    return listutils::typeError("result of the 2nd function is not an int");
  }
  return nl->First(args); 
}

class partition8LocalInfo{

  public:
     partition8LocalInfo(Word _stream, Word _fun1, Word _fun2,
                    const string& _home, const string& _name, int _size1,
                    int _size2, int _wnum, ListExpr streamType ): 
                    stream(_stream), fun1(_fun1.addr),
                    fun2(_fun2.addr), home(_home), name(_name), size1(_size1),
                    size2(_size2), wnum(_wnum), 
                    outputFiles((size_t)size1*size2, 0){
          fun1Arg = qp->Argument(fun1);
          fun2Arg = qp->Argument(fun2);
          stream.open();
          dbname = SecondoSystem::GetInstance()->GetDatabaseName();
          createDirectories();
          relType = nl->TwoElemList( listutils::basicSymbol<Relation>(),
                                     nl->Second(streamType));
   }

   ~partition8LocalInfo(){
      stream.close();
      for(size_t i=0;i< outputFiles.size();i++){
         if(outputFiles[i]){
           BinRelWriter::finish(*outputFiles[i]);
           outputFiles[i]->close();
           delete outputFiles[i];
         } 
      }
    }

    Tuple* getNext(){
       Tuple* res = stream.request();
       if(res){
          storeTuple(res);
       }
       return res;
    }



  private:
      Stream<Tuple> stream;
      void* fun1;
      void* fun2;
      string home;
      string name;
      int size1;
      int size2;
      int wnum;
      ListExpr relType;
      vector<ofstream*> outputFiles;
      ArgVectorPointer fun1Arg; 
      ArgVectorPointer fun2Arg; 
      string dbname;


      void createDirectories(){
         // matrix directories are build as
         // home/dfarrays/dbname/name/worker/name_slot.bin
         // because we have to distribute to size_1 matrices,
         // name is name_0 ... name_{size1-1}
         for(int i=0;i<size1; i++){
            FileSystem::CreateFolderEx( getDirName(i));
         }
      }

      string getDirName(int i){
            string n = getName(i);
            return home +"/dfarrays/"+ dbname + "/" + n + "/"
                             + stringutils::int2str(wnum)+"/";
      }

      string getName(int i){
          return name + "_" + stringutils::int2str(i);
      }


      void storeTuple(Tuple* tuple){
          Word result;
          (* fun1Arg[0]) = tuple;
          qp->Request(fun1, result);
          CcInt* CcRes1 = (CcInt*) result.addr;
          int res1 = CcRes1->IsDefined()?CcRes1->GetValue():0;
          res1 = res1 % size1;
        
          (* fun2Arg[0]) = tuple;
          qp->Request(fun2, result);
          CcInt* CcRes2 = (CcInt*) result.addr;
          int res2 = CcRes2->IsDefined()?CcRes2->GetValue():0;
          res2 = res2 % size2;

          int index = size1*res2 + res1;
          if(!outputFiles[index]){
              string filename =   getDirName(res1) + getName(res1) 
                                + "_" + stringutils::int2str(res2) +".bin";
              outputFiles[index] = new ofstream(filename.c_str(),  
                                                ios::out | ios::binary);
              
              BinRelWriter::writeHeader(*outputFiles[index], relType); 
          }
          BinRelWriter::writeNextTuple(*outputFiles[index], tuple);
      }
};



int partition8LocalVM(Word* args, Word& result, int message,
             Word& local, Supplier s ){

   partition8LocalInfo* li = (partition8LocalInfo*) local.addr;
   switch(message){
      case OPEN : {
              if(li){
                 delete li;
                 local.addr = 0;
              }
              // 1: stream, 2: fun1, 3 : fun2, 
              // 4: home , 5 : name,
              // 6: size1, 7:  size2 , 8 : worker number
              FText* home = (FText*) args[3].addr;
              CcString* name = (CcString*) args[4].addr;
              CcInt* size1 = (CcInt*) args[5].addr;
              CcInt* size2 = (CcInt*) args[6].addr;
              CcInt* wnum  = (CcInt*) args[7].addr;
              if(!home->IsDefined() || !name->IsDefined()
                 || !size1->IsDefined() || !size2->IsDefined()){
                 return 0;
              }
              if(   size1->GetValue()<1 || size2->GetValue() < 1 
                 ||  wnum->GetValue()<0 || name->GetValue().empty()
                 || home->GetValue().empty()){
                 return 0;
              }
              local.addr = new partition8LocalInfo( args[0], args[1], args[2],
                                    home->GetValue(),
                                    name->GetValue(),  size1->GetValue(),
                                    size2->GetValue(), wnum->GetValue(),
                                    qp->GetType(qp->GetSon(s,0)));
               return 0;
          }
      case  REQUEST:
               result.addr = li?li->getNext():0;
               return result.addr?YIELD:CANCEL;
      case CLOSE:
              if(li){
                 delete li;
                 local.addr = 0;
              }
              return 0;
   }
   return 0;

}


OperatorSpec partition8LocalSpec(
  " stream(Tuple) x (tuple->int) x tuple->int x text x "
  "string x int x int x int -> stream(tuple)",
  " stream partition8Local[ arrayFun , darrayFun, home, "
  "name, arraySize, darraySize, workerNumber] ",
  "Distributes a tuple stream according to the results of 2 functions "
  "to a set of files.\n "
  "ONLY FOR INTERNAL USE.",
  "query plz feed partition8Local[ .PLZ , hashvalue(.Ort,31)," 
  "     '/home/user/secondo-databases', \"myName\", 8, 4, 1] count",
  "Only for internal usage."
);


Operator partition8LocalOp(
  "partition8Local",
  partition8LocalSpec.getStr(),
  partition8LocalVM,
  Operator::SimpleSelect,
  partition8LocalTM
);



/*
98 Operator ~partitionF8~

This operator wokrs similar to the partitionF operator. It creates an array of
dfmatrixes using two partition functions. As already implemented for the 
~partitionF~ operator, a furthre function can be applied firstly to filter or
to manipulate the incoming stream.

*/
ListExpr partitionP8TM(ListExpr args){
  if(!nl->HasLength(args,7)){
    return listutils::typeError("7 arguments expected");
  }
  // check for correct usesArgsInTypeMapping
  ListExpr tmp = args;
  while(!nl->IsEmpty(tmp)){
    if(!nl->HasLength(nl->First(tmp),2)){
      return listutils::typeError("internal error");
    }
    tmp = nl->Rest(tmp); 
  }
  ListExpr at = nl->First(nl->First(args));   // d[f]array
  ListExpr nt = nl->First(nl->Second(args));  // name
  ListExpr f1t = nl->First(nl->Third(args));  // first fun
  ListExpr f2t = nl->First(nl->Fourth(args)); // array distribution
  ListExpr f3t = nl->First(nl->Fifth(args));  // darray distribution
  ListExpr ast  = nl->First(nl->Sixth(args));  // array size
  ListExpr mst  = nl->First(nl->Sixth(nl->Rest(args)));  // matrix size

  if(!DArray::checkType(at) && !DFArray::checkType(at)){
    return listutils::typeError("first argument must be a d[f]array");
  }
  if(!Relation::checkType(nl->Second(at))){
    return listutils::typeError("subtype of the darray must be a relation");
  }

  if(!CcString::checkType(nt)){
    return listutils::typeError("Second arguent must be a string");
  }
  if(!listutils::isMap<1>(f1t)){
    return listutils::typeError("third arg must be a unary function");
  }
  if(!listutils::isMap<1>(f2t)){
    return listutils::typeError("fourth arg must be a unary function");
  }
  if(!listutils::isMap<1>(f3t)){
    return listutils::typeError("fifth arg must be a unary function");
  }

  if(!CcInt::checkType(ast)){
    return listutils::typeError("sixth argument must be of type int");
  }
  if(!CcInt::checkType(mst)){
    return listutils::typeError("seventh argument must be of type int");
  }
  

  // check arguments of the functions

  ListExpr f1a = nl->Second(f1t);
  ListExpr f2a = nl->Second(f2t);
  ListExpr f3a = nl->Second(f3t);

  ListExpr rel = nl->Second(at);
  ListExpr stream = nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                                     nl->Second(rel));


  if(!nl->Equal(f1a,nl->Second(at)) &&
     !nl->Equal(f1a, stream)){
    return listutils::typeError("Argument type of the first function must be "
                                "equal to the darray's subtype");
  }
  ListExpr f1r = nl->Third(f1t);
  if(!Stream<Tuple>::checkType(f1r)){
     return listutils::typeError("result of the first function must be "
                                 "a stream of tuples");
  }
  ListExpr tuplet = nl->Second(f1r);
  if(!nl->Equal(f2a,tuplet)){
     return listutils::typeError("argument type of the second function must be"
                                 " equal to the tuple type of the result of the"
                                 " first function");
  }  
  if(!nl->Equal(f3a,tuplet)){
     return listutils::typeError("argument type of the third function must be"
                                 " equal to the tuple type of the result of the"
                                 " first function");
  }  


  // result of the last two functions must be int
  if(!CcInt::checkType(nl->Third(f2t))){
    return listutils::typeError("result of the second function mut be of "
                                "type int");
  }

  if(!CcInt::checkType(nl->Third(f3t))){
     return listutils::typeError("result of the third function must be of "
                                  "type int");
  }

  // get the function definitions
  ListExpr f1def = nl->Second(nl->Third(args)); 
  ListExpr f2def = nl->Second(nl->Fourth(args));
  ListExpr f3def = nl->Second(nl->Fifth(args));

  // replace the function types that can be just a type map operator name
  f1def = nl->ThreeElemList( 
                  nl->First(f1def),
                  nl->TwoElemList( nl->First(nl->Second(f1def)),
                                   stream),
                  nl->Third(f1def));

   f2def = nl->ThreeElemList(
                   nl->First(f2def),
                   nl->TwoElemList( nl->First(nl->Second(f2def)),
                                    tuplet),
                   nl->Third(f2def));

   f3def = nl->ThreeElemList(
                   nl->First(f3def),
                   nl->TwoElemList( nl->First(nl->Second(f3def)),
                                    tuplet),
                   nl->Third(f3def));
 


    // now we can build the result
    ListExpr restype = nl->TwoElemList(
                          listutils::basicSymbol<arrayalgebra::Array>(),
                          nl->TwoElemList(
                             listutils::basicSymbol<DFMatrix>(),
                             nl->Second(at)));

    ListExpr appendList = nl->ThreeElemList(
                             nl->TextAtom(nl->ToString(f1def)),
                             nl->TextAtom(nl->ToString(f2def)),
                             nl->TextAtom(nl->ToString(f3def)));      

    return nl->ThreeElemList(
                   nl->SymbolAtom(Symbols::APPEND()),
                   appendList,
                   restype);

}


template<class AType>
class partitionF8Runner{
  public:
     partitionF8Runner(AType* _array, ListExpr  _atype,
                   int  _workerNumber,
                   int  _asize, int _msize, const string& _fun1,
                   const string&  _fun2, const string& _fun3,
                   const string _name,
                   const vector<int>& _slots): array(_array), atype(_atype),
                   workerNumber(_workerNumber), asize(_asize), msize(_msize),
                   fun1(_fun1), fun2(_fun2), fun3(_fun3), 
                   name(_name), slots(_slots){
                   runner = new boost::thread(&partitionF8Runner::run, this); 
     }

     ~partitionF8Runner(){
        runner->join();
        delete runner;
     }


   private:
      AType* array;
      ListExpr atype;
      int workerNumber;
      int asize;
      int msize;
      string fun1;
      string fun2;
      string fun3;
      string name;
      vector<int> slots;
      boost::thread* runner;
      ConnectionInfo* ci;
      string home;
      string dbname;


      void run(){
         dbname = SecondoSystem::GetInstance()->GetDatabaseName();
         const DArrayElement& elem = array->getWorker(workerNumber);
         ci = algInstance->getWorkerConnection(elem,dbname);
         if(ci){
            home = ci->getSecondoHome();
            string query = createQuery();
            performListCommand(ci, query);
         } else {
            cerr << "could not connet to " 
                 << elem.getHost() << "@" 
                 << elem.getPort() << endl;
         }
      }


       string createQuery(){

          string res =   string("( count (partition8Local ")
                  + "(" + fun1 + getStream() + " ) " + " " + fun2 + " " + fun3 
                  + " '"+home+"' \"" + name +"\" " 
                  + stringutils::int2str(asize) + " " 
                  + stringutils::int2str(msize) + " " 
                  + stringutils::int2str(workerNumber) + "))";
          return "(query " + res + ")";  
       }

       string getStream(){
         if(array->getType() == DFARRAY){
            string res = string("( feed (  (fsrel " )
                       + nl->ToString(nl->Second(nl->Second(atype))) + ")";
            for(size_t i=0; i< slots.size(); i++){
                string fn =  array->getFilePath(home, dbname, slots[i]);
                res += " '"+fn+"'";
            }
            res += ") )";
            return res;
         }
         assert(array->getType() == DARRAY);
         string res = "(feed  " + array->getObjectNameForSlot(slots[0]) + ")";
         for( size_t i=1; i< slots.size(); i++){
            res = "(concat " + res + "( feed "
                + array->getObjectNameForSlot(slots[i]) + "))";
          }
          return res;
       }

};





template<class AType>
int partitionF8VMT(Word* args, Word& result, int message,
             Word& local, Supplier s ){


   result = qp->ResultStorage(s);
   arrayalgebra::Array* res = (arrayalgebra::Array*) result.addr;
   AType* array = (AType*) args[0].addr;
   CcString* CcName = (CcString*) args[1].addr;
   CcInt* CcASize = (CcInt*) args[5].addr;
   CcInt* CcMSize = (CcInt*) args[6].addr;

   if(!array->IsDefined()){
     res->setUndefined();
     return 0;
   }
   if(array->numOfWorkers()<1){
     res->setUndefined();
     return 0;
   }

   if(!CcASize->IsDefined()){
     res->setUndefined();
     return 0;
   }

   int asize = CcASize->GetValue();

   if(asize < 1){
      res->setUndefined();
      return 0;
   }


   if(!CcMSize->IsDefined()){
      res->setUndefined();
      return 0;
   }
   
   int msize = CcMSize->GetValue();
   if(msize <1){
      msize = array->getSize(); // overtake old value
   } 

   string name = "";
   if(CcName->IsDefined()){
      name = CcName->GetValue();
   }
   if(name==""){
     name = algInstance->getTempName(array->getWorker(0));
   }
   if(!stringutils::isIdent(name)){
     res->setUndefined();
     return 0;
   }

   // create array entries
   Word elements[asize];
   const vector<DArrayElement>& workers = array->getWorkers();
   int algId;
   int typeId;
   string tname;
   SecondoSystem::GetCatalog()->LookUpTypeExpr(nl->Second(qp->GetType(s)),
                                               tname, algId, typeId);


   for(int i=0;i<asize;i++){
     string n = name +"_" + stringutils::int2str(i);
     elements[i].addr = new DFMatrix(msize, n, workers);
   }
   res->initialize(algId, typeId, asize, elements);   

   



   // get the function defintions as texts (appended by tm)
   string fun1 = ((FText*) args[7].addr)->GetValue(); 
   string fun2 = ((FText*) args[8].addr)->GetValue(); 
   string fun3 = ((FText*) args[9].addr)->GetValue(); 

   




   // now, create the remote content

   // for each worker, we collect all slots for that the worker
   // is responsible
   vector<vector<int> > responsible;
   // fill with empty vectors
   for(size_t i=0;i<array->numOfWorkers();i++){
      vector<int> v;
      responsible.push_back(v);
   }
   // fill with  slots
   for(size_t i=0;i<array->getSize();i++){
      int w = array->getWorkerIndexForSlot(i);
      responsible[w].push_back(i);
   }

   // start for each worker a runner instance
   vector<partitionF8Runner<AType>*> runners;
   ListExpr atype = qp->GetType(qp->GetSon(s,0));
   for(size_t i=0;i<array->numOfWorkers(); i++){
       if(!responsible[i].empty()){
          partitionF8Runner<AType>* run = 
             new partitionF8Runner<AType>(array, atype, i, asize, 
                                          msize, fun1, fun2, fun3,
                                          name, 
                                          responsible[i]);
          runners.push_back(run); 
       }
   }
   for(size_t i=0;i<runners.size(); i++){
     delete runners[i];
   }
   return 0;
}


ValueMapping partitionF8VM[] = {
   partitionF8VMT<DArray>,
   partitionF8VMT<DFArray>
};


int partitionF8Select(ListExpr args){
  return DArray::checkType(nl->First(args))?0:1;
}

OperatorSpec partitionF8Spec(
  "d[f]array(rel(tuple(X))) x string x (rel(tuple(X)) -> stream(tuple(Y))) "
  " x (tuple(Y) -> int) x (tuple(Y) -> int) x int x int "
  "-> array(dfmatrx(rel(tuple(Y)))",
  " array partitionF8[ name, prefunction, funA, funD , sizeA , sizeD] ",
  "Redistributes a d[f]array into an array of dfmatrix. "
  " The string argument is uses to determine a base name for the "
  "dfmatrices within the containing array. Each dfmatrix's name is "
  " extended by the slot number of the array that contains the matrix. "
  "The very fisrt function is used to generate a tuple stream from the "
  " relation conatained in the array elements." 
  "The second function determines the slot of a tuple within the main array. "
  "The third function determines the slot of a tuple within the dfmatrix."
  "size1 gives the number of slots in the main  array. " 
  "size2 determines the number of slots of the dfmatrices. If this "
  "value is smaller "
  "then one, the size of the original array will be used. ",
  "query array partitionF8[\"mynewname\",  . feed addcouunter[B, 16],"
  " .PLZ , .B , 8, 16]"
);


Operator partitionF8Op(
  "partitionF8",
   partitionF8Spec.getStr(),
   2,
   partitionF8VM,
   partitionF8Select,
   partitionP8TM
);


/*
TypeMap for the partition F8 operator.

*/
ListExpr P8TM(ListExpr args){
  int len = nl->ListLength(args);
  if(len<2 || len>4){
    return listutils::typeError("between 2 and 4 arguments expected");
  }  
  ListExpr first = nl->First(args);
  if(!DArray::checkType(first) && !DFArray::checkType(first)){
    return listutils::typeError("first arg must be of thy d[f]array.");
  }
  ListExpr rel = nl->Second(first);
  if(!Relation::checkType(rel)){
    return listutils::typeError("sub type of the d[f]array is not a relation");
  }
 
  ListExpr stream = nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                           nl->Second(rel));

  if(len==2){
    return stream;
  }
  ListExpr fun1=nl->Third(args);
  if(!listutils::isMap<1>(fun1)){
    return listutils::typeError("third arg is not an unary function");
  } 
  // check whether the function arg corresponds to rel
  if(!nl->Equal(rel, nl->Second(fun1)) && 
     !nl->Equal(stream, nl->Second(fun1))){
     return listutils::typeError("funtion argument and relation/stream differ");
  }
  ListExpr funRes = nl->Third(fun1);
  if(!Stream<Tuple>::checkType(funRes)){
    return listutils::typeError("result of the first function is "
                                "not a tuple stream");
  }
  return nl->Second(funRes);
}

OperatorSpec P8TMSpec(
  "d[f]array(rel) x X -> rel | "
  "d[f]array(rel) x X x rel -> stream(tuple) x ... -> tuple",
  " P8TM(_,_,_) ",
  "Type Map operator.",
  "query array partitionF8[\"mynewname\",  . feed addcouunter[B, 16],"
  " .PLZ , .B , 8, 16]"
);


Operator P8TMOp(
  "P8TM",
  P8TMSpec.getStr(),
  0,
  Operator::SimpleSelect,
  P8TM
);




/*
3 Implementation of the Algebra

*/
Distributed2Algebra::Distributed2Algebra(){
   namecounter = 0;

   AddTypeConstructor(&DArrayTC);
   DArrayTC.AssociateKind(Kind::SIMPLE());
   AddTypeConstructor(&DFArrayTC);
   DFArrayTC.AssociateKind(Kind::SIMPLE());
   AddTypeConstructor(&DFMatrixTC);
   DFMatrixTC.AssociateKind(Kind::SIMPLE());
   AddTypeConstructor(&FRelTC);
   FRelTC.AssociateKind(Kind::SIMPLE());
   AddTypeConstructor(&FSRelTC);
   FSRelTC.AssociateKind(Kind::SIMPLE());
   
   AddTypeConstructor(&FObjTC);
   FObjTC.AssociateKind(Kind::SIMPLE());


   AddOperator(&connectOp);
   AddOperator(&checkConnectionsOp);
   AddOperator(&rcmdOp);
   AddOperator(&disconnectOp);
   AddOperator(&rqueryOp);
   rqueryOp.SetUsesArgsInTypeMapping();
   AddOperator(&prcmdOp);
   AddOperator(&sendFileOp);
   AddOperator(&requestFileOp);
   AddOperator(&psendFileOp);
   AddOperator(&prequestFileOp);
   AddOperator(&getRequestFolderOp);
   AddOperator(&getSendFolderOp);
   AddOperator(&pconnectOp);
   AddOperator(&prqueryOp);
   prqueryOp.SetUsesArgsInTypeMapping();
   AddOperator(&prquery2Op);
   prquery2Op.SetUsesArgsInTypeMapping();

   AddOperator(&putOp);
   AddOperator(&getOp);
   AddOperator(&sizeOp);
   AddOperator(&getWorkersOp);

   AddOperator(&fconsume5Op);
   AddOperator(&ffeed5Op);
   ffeed5Op.SetUsesArgsInTypeMapping();
   AddOperator(&feedOp);
   feedOp.SetUsesArgsInTypeMapping();

   AddOperator(&createDArrayOp);

   AddOperator(&pputOp);
   AddOperator(&ddistribute2Op);
   AddOperator(&ddistribute3Op);
   AddOperator(&ddistribute4Op);
   AddOperator(&fdistribute5Op);
   AddOperator(&fdistribute6Op);
   AddOperator(&closeWorkersOp);
   AddOperator(&showWorkersOp);
   AddOperator(&dloopOp);
   dloopOp.SetUsesArgsInTypeMapping();
   AddOperator(&dloop2Op);
   dloop2Op.SetUsesArgsInTypeMapping();
   AddOperator(&DARRAYELEMOp);
   AddOperator(&DARRAYELEM2Op);

   AddOperator(&dsummarizeOp);
   AddOperator(&getValueOp);
   AddOperator(&deleteRemoteObjectsOp);
   AddOperator(&cloneOp);
   AddOperator(&shareOp);
   AddOperator(&cleanUpOp);

   AddOperator(&dfdistribute2Op);
   AddOperator(&dfdistribute3Op);
   AddOperator(&dfdistribute4Op);

   AddOperator(&convertdarrayOp);

   AddOperator(&gettuplesOp);
   gettuplesOp.SetUsesArgsInTypeMapping();

   
   AddOperator(&dmapOp);
   dmapOp.SetUsesArgsInTypeMapping();
   AddOperator(&DFARRAYSTREAMOP);


   AddOperator(&dmap2Op);
   dmap2Op.SetUsesArgsInTypeMapping();

   AddOperator(&dmap3Op);
   dmap3Op.SetUsesArgsInTypeMapping();
   AddOperator(&dmap4Op);
   dmap4Op.SetUsesArgsInTypeMapping();
   AddOperator(&dmap5Op);
   dmap5Op.SetUsesArgsInTypeMapping();
   AddOperator(&dmap6Op);
   dmap6Op.SetUsesArgsInTypeMapping();
   AddOperator(&dmap7Op);
   dmap7Op.SetUsesArgsInTypeMapping();
   AddOperator(&dmap8Op);
   dmap8Op.SetUsesArgsInTypeMapping();

   AddOperator(&ARRAYFUNARG1OP);
   AddOperator(&ARRAYFUNARG2OP);
   AddOperator(&ARRAYFUNARG3OP);
   AddOperator(&ARRAYFUNARG4OP);
   AddOperator(&ARRAYFUNARG5OP);
   AddOperator(&ARRAYFUNARG6OP);
   AddOperator(&ARRAYFUNARG7OP);
   AddOperator(&ARRAYFUNARG8OP);

   AddOperator(&AREDUCEARG1OP);
   AddOperator(&AREDUCEARG2OP);


   AddOperator(&fileTransferServerOP);
   AddOperator(&recieveFileClientOP);
   AddOperator(&transferFileOP);


   AddOperator(&traceCommandsOp);
   AddOperator(&showProgressOp);


   AddOperator(&staticFileTransferatorOp);
   AddOperator(&killStaticFileTransferatorOp);
   AddOperator(&putFileTCPOp);
   AddOperator(&getFileTCPOp);

   AddOperator (&fsfeed5Op);
   fsfeed5Op.SetUsesArgsInTypeMapping();

   AddOperator(&fdistribute7Op);

   AddOperator(&partitionOp);
   partitionOp.SetUsesArgsInTypeMapping();
   AddOperator(&DFARRAYTUPLEOP);

   AddOperator (&areduceOp);
   areduceOp.SetUsesArgsInTypeMapping();


   AddOperator (&areduce2Op);
   areduce2Op.SetUsesArgsInTypeMapping();


   AddOperator(&collect2Op);


   AddOperator(&SUBTYPE1OP);
   AddOperator(&SUBSUBTYPE1OP);
   //AddOperator(&SUBTYPE2OP);


   AddOperator(&partitionFOp);
   partitionFOp.SetUsesArgsInTypeMapping();

   AddOperator(&FFROp);


   AddOperator(&saveAttrOp);
   AddOperator(&loadAttrOp);
   loadAttrOp.SetUsesArgsInTypeMapping();


   AddOperator(&createFrelOP);
   createFrelOP.SetUsesArgsInTypeMapping();

   AddOperator(&createFSrelOP);

   AddOperator(&saveObjectToFileOp);
   AddOperator(&getObjectFromFileOp);
   getObjectFromFileOp.SetUsesArgsInTypeMapping();


   AddOperator(&dproductOp);   
   dproductOp.SetUsesArgsInTypeMapping();

   AddOperator(&arraytypeStream1Op);   
   AddOperator(&arraytypeStream2Op);   


   AddOperator(&ddistribute8Op);   
   AddOperator(&dfdistribute8Op);   

   AddOperator(&dproductarg1Op);
   AddOperator(&dproductarg2Op);
   

   AddOperator(&partition8LocalOp);
   AddOperator(&partitionF8Op);
   partitionF8Op.SetUsesArgsInTypeMapping();
   AddOperator(&P8TMOp);


   pprogView = new PProgressView();
   MessageCenter::GetInstance()->AddHandler(pprogView);



   


}


Distributed2Algebra::~Distributed2Algebra(){
   map<int,staticFileTransferator*>::iterator it;
   for(it=staticFileTransferators.begin();
       it!=staticFileTransferators.end();it++){
     delete it->second;
   }

   boost::lock_guard<boost::mutex> guard(mtx);
   for(size_t i=0;i<connections.size();i++){
      delete connections[i];
   }
   connections.clear();
   closeAllWorkers();
   if(pprogView){
      MessageCenter::GetInstance()->RemoveHandler(pprogView);
      delete pprogView;
   }
}

} // end of namespace distributed2

extern "C"
Algebra*
   InitializeDistributed2Algebra( NestedList* nlRef,
                             QueryProcessor* qpRef,
                             AlgebraManager* amRef ) {

   distributed2::algInstance = new distributed2::Distributed2Algebra();
   distributed2::showCommands = false;   
   return distributed2::algInstance;
}


