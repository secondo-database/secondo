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


extern DebugWriter dwriter;

extern boost::mutex nlparsemtx;

namespace distributed3 {

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



/*
Some Helper functions.

*/

template<class T>
bool writeVar(const T& value, SmiRecord& record, size_t& offset){
  bool res = record.Write(&value, sizeof(T), offset) == sizeof(T);
  offset += sizeof(T);
  return res;
}

template<>
bool writeVar<string>(
  const string& value, SmiRecord& record, size_t& offset){

  // write the size of the sting
  size_t len = value.length();
  if(!writeVar<size_t>(len,record,offset)){
     return false;
  }
  if(len==0){
     return true;
  }
  if(record.Write(value.c_str(),len, offset)!=len){
     return false;
  }
  offset+= len;
  return true;
}

template<class T>
bool readVar(T& value, SmiRecord& record, size_t& offset){
  bool res = record.Read(&value, sizeof(T), offset) == sizeof(T);
  offset += sizeof(T);
  return res;
}

template<>
bool readVar<string>(string& value, SmiRecord& record, size_t& offset){
  size_t len;
  if(!readVar<size_t>(len,record,offset)){
    return false;
  }
  //assert(len<=48);

  if(len==0){
     value = "";
     return true;
  }
  char* cstr = new char[len];
  bool res = record.Read(cstr, len, offset) == len;
  offset += len;
  value.assign(cstr,len);
  delete[] cstr;
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

bool getConstEx(const string& objName, string& result){

   SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
   if(!ctlg->IsObjectName(objName)){
       return false;
   }
   result = " [const " + rewriteRelType(ctlg->GetObjectTypeExpr(objName))
          + " value " + nl->ToString(ctlg->GetObjectValue(objName)) + "] ";

   return true;
}


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

      ~ConnectionInfo(){
          {
             boost::lock_guard<boost::recursive_mutex> guard(simtx);
             si->Terminate();
          }
          delete si;
          si = 0;
          delete mynl;
       }

       string getHost() const{
          return host;
       }

       int getPort() const{
          return port;
       }
       string getConfig() const{
          return config;
       }
    
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

        void setId(const int i){
            if(si){
               si->setId(i);;
            }
        }


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

       string getSecondoHome(){
         if(secondoHome.length()==0){
            retrieveSecondoHome();
         }
         return secondoHome;
       }



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


       void simpleCommand(const string& command1, int& error, string& errMsg, 
                          string& resList, const bool rewrite, double& runtime){

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
          showCommand(si,host,port,command,true);
          si->Secondo(command, myResList, serr);
          showCommand(si,host,port,command,false);
          runtime = sw.diffSecondsReal();
          error = serr.code;
          errMsg = serr.msg;

          resList = mynl->ToString(myResList);
          mynl->Destroy(myResList);
       }
       
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

        int serverPid(){
           boost::lock_guard<boost::recursive_mutex> guard(simtx);
           if(serverPID==0){
             serverPID = si->getPid(); 
           }
           return serverPID;
        }


        int sendFile( const string& local, const string& remote, 
                      const bool allowOverwrite){
          boost::lock_guard<boost::recursive_mutex> guard(simtx);   
          int res =  si->sendFile(local,remote, allowOverwrite);
          return res;
        }
        

        int requestFile( const string& remote, const string& local,
                         const bool allowOverwrite){
          boost::lock_guard<boost::recursive_mutex> guard(simtx);   
          int res =  si->requestFile(remote, local, allowOverwrite);
          return res;
        }

        string getRequestFolder(){
          if(requestFolder.length()==0){
             boost::lock_guard<boost::recursive_mutex> guard(simtx);   
             requestFolder =  si->getRequestFileFolder();
          }
          return requestFolder;
        }
        
        string getSendFolder(){
          if(sendFolder.length()==0){
             boost::lock_guard<boost::recursive_mutex> guard(simtx);   
             sendFolder =  si->getSendFileFolder();
          }
          return sendFolder;
        }
        
        string getSendPath(){
          if(sendPath.length()==0){
             boost::lock_guard<boost::recursive_mutex> guard(simtx);   
             sendPath =  si->getSendFilePath();
          }
          return sendPath;
        }

        static ConnectionInfo* createConnection(const string& host, 
                                   const int port, string& config){

              NestedList* mynl = new NestedList();
              SecondoInterfaceCS* si = new SecondoInterfaceCS(true,mynl);
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
       
             cmd = "let " + name + " =  ffeed5('" 
                          + rfilename + "') consume ";

             showCommand(si,host,port,cmd,true);
             si->Secondo(cmd, resList, serr);
             showCommand(si,host,port,cmd,false);

             bool ok = serr.code == 0;


             cmd = "query removeFile('"+rfilename+"')";

             si->Secondo (cmd,resList,serr);


             return ok;
         }
         

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

         bool saveAttributeToFile(ListExpr type, Word& value, 
                                 const string& filename){
            Attribute* attr = (Attribute*) value.addr;
            return FileAttribute::saveAttribute(type, attr, filename);
         }

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
               cerr << "Command " << cmd << " failed with code " 
                    << serr.code << endl;
               cerr << serr.msg << endl;
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
                cerr << "command " << cmd << " failed with code " << serr.code;
                cerr << serr.msg;
             }
             return true; 
          }



          bool retrieveRelationFile(const string& objName,
                                    const string& fname1){
             boost::lock_guard<boost::recursive_mutex> guard(simtx);
             string rfname = si->getRequestFilePath() + "/"+fname1;
             // save the remove relation into a binary file
             string cmd = "query " + objName + " feed fconsume5['"
                          +rfname+"'] count";

             SecErrInfo serr;
             ListExpr resList;
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
                   cerr << "command " << cmd << " failed " << endl;
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


template<class ResType>
class CommandListener{
 public:

  virtual void jobDone(int id, string& command, size_t no, int error, 
                       string& errMsg, ResType& resList, double runtime)=0;
};





/*
Forward declaration of class DArrayElement.

2.1.1 Class ~DArrayElement~

This class represents information about a single worker of a DArray.

*/

class DArrayElement{
  public:
     DArrayElement(const string& _server, const int _port,
                    const int _num, const string& _config):
        server(_server), port(_port), num(_num),config(_config) {}

     DArrayElement(const DArrayElement& src):
      server(src.server), port(src.port), num(src.num),
      config(src.config) {}

     DArrayElement& operator=(const DArrayElement& src){
        this->server = src.server;
        this->port = src.port;
        this->num = src.num;
        this->config = src.config;
        return *this;
     }     

     ~DArrayElement() {}


      void setNum(const int num){
         this->num = num;
      }

     void set(const string& server, const int port, 
              const int num, const string& config){
        this->server = server;
        this->port = port;
        this->num = num;
        this->config = config;
     }


     bool operator==(const DArrayElement& other) const{
       return    (this->port == other.port)
              && (this->num == other.num)
              && (this->server == other.server)
              && (this->config == other.config); 
     }
     bool operator!=(const DArrayElement& other) const{
       return   !((*this) == other);
     }

     bool operator<(const DArrayElement& other) const {
        if(this->port < other.port) return true;
        if(this->port > other.port) return false;
        if(this->num < other.num) return true;
        if(this->num > other.num) return false;
        if(this->server < other.server) return true;
        if(this->server > other.server) return false;
        if(this->config < other.config) return true;
        // equal or greater
        return false;
     }

     bool operator>(const DArrayElement& other) const {
        if(this->port > other.port) return true;
        if(this->port < other.port) return false;
        if(this->num > other.num) return true;
        if(this->num < other.num) return false;
        if(this->server > other.server) return true;
        if(this->server < other.server) return false;
        if(this->config > other.config) return true;
        // equal or greater
        return false;
     }
     
     ListExpr toListExpr(){
        return nl->ThreeElemList(
                   nl->TextAtom(server),
                   nl->IntAtom(port),
                   nl->TextAtom(config));   
     }

     bool readFrom(SmiRecord& valueRecord, size_t& offset){
        string s;
        if(!readVar(s,valueRecord,offset)){
            return false;
        }
        uint32_t p;
        if(!readVar(p,valueRecord,offset)){
           return false;
        }
        string c;
        if(!readVar(c,valueRecord,offset)){
           return false;
        }

        server = s;
        port = p;
        num = -1;
        config = c;
        return true;
     }

     bool saveTo(SmiRecord& valueRecord, size_t& offset){
        if(!writeVar(server,valueRecord,offset)){
           return false;
        }
        if(!writeVar(port,valueRecord,offset)){
            return false;
        }
        if(!writeVar(config,valueRecord,offset)){
           return false;
        }
        return true;
     }

     void print(ostream& out)const{
         out << "( S: " << server << ", P : " << port 
             << "Num : " << num
             << ", C : " << config << ")";
     }

     string getHost()const{ return server; }
     int getPort() const {return port; }
     string getConfig() const{ return config; }
     int getNum() const{ return num; }


     template<class H, class C>
     static DArrayElement* createFromTuple(Tuple* tuple, int num, 
                                   int hostPos, int portPos, int configPos){

         if(!tuple || (num < 0) ) {
            return 0;
         }

         H* CcHost = (H*) tuple->GetAttribute(hostPos);
         CcInt* CcPort = (CcInt*) tuple->GetAttribute(portPos);
         C* CcConfig = (C*) tuple->GetAttribute(configPos);

         if(!CcHost->IsDefined() || !CcPort->IsDefined() || 
            !CcConfig->IsDefined()){
             return 0;
         }
         string host = CcHost->GetValue();
         int port = CcPort->GetValue();
         string config = CcConfig->GetValue();
         if(port<=0){
            return 0;
         }
         return new DArrayElement(host,port,num,config);
     }


  private:
     string server;
     uint32_t port;
     uint32_t num;
     string config;
};


ostream& operator<<(ostream& out, const DArrayElement& elem){
  elem.print(out);
  return out;
}



bool InDArrayElement(ListExpr list, DArrayElement& result){
   if(!nl->HasLength(list,3)){
     return false;
   }
   ListExpr e1 = nl->First(list);
   ListExpr e2 = nl->Second(list);
   ListExpr e4 = nl->Third(list);
   string server;
   int port;
   int num;
   string config;

   if(nl->AtomType(e1) == StringType){
      server = nl->StringValue(e1);     
   } else if(nl->AtomType(e1) == TextType){
      server = nl->Text2String(e1);
   } else {
      return false;
   }
   stringutils::trim(server);
   if(server.empty()){
     return false;
   }
   if(nl->AtomType(e2) != IntType){
     return false;
   }
   port = nl->IntValue(e2);
   if(port <=0){
     return false;
   }

   if(nl->AtomType(e4) == StringType){
      config = nl->StringValue(e4);     
   } else if(nl->AtomType(e4) == TextType){
      config = nl->Text2String(e4);
   } else {
      return false;
   }
   stringutils::trim(config);
   if(config.empty()){
      return false;
   }   
   num = -1;
   result.set(server,port,num,config);
   return true;
}


/*
Class ProgressView

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



class Distributed3Algebra: public Algebra{

  public:

/*
1.1 Constructor defined at the end of this file

*/
     Distributed3Algebra();


/*
1.2 Destructor

Closes all open connections and destroys them.

*/
     ~Distributed3Algebra();


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
~getConnections~

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


    void initProgress(){
        pprogView->init(connections.size());
    }

    void finishProgress(){
        pprogView->finish();
    }



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

  map<DArrayElement, pair<string,ConnectionInfo*> >::iterator
  workersIterator(){
    return workerconnections.begin();
  }

  bool isEnd( map<DArrayElement, pair<string,ConnectionInfo*> >::iterator& it){
    boost::lock_guard<boost::mutex> guard(workerMtx);
    return it==workerconnections.end();
  } 


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



   size_t nextNameNumber(){
      boost::lock_guard<boost::mutex> guard(namecounteraccess);
      namecounter++;
      return namecounter;
   } 

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

Distributed3Algebra* algInstance;

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
   SecondoInterfaceCS* si = new SecondoInterfaceCS(true,mynl);
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


int checkConnectionsVM( Word* args, Word& result, int message,
                  Word& local, Supplier s ){

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
     "Check connections in the Distributed3Algebra",
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
template<class S>
bool getValue(ListExpr in, string&out){
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
  ListExpr list = nl->TwoElemList( nl->SymbolAtom("error"),
                                   nl->TextAtom(message));
  msg->Send(nl,list );
  msg->Flush();
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
        SecondoInterfaceCS* si = new SecondoInterfaceCS(false, mynl);
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
1.8 Operator pquery

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

ListExpr pqueryTM(ListExpr args){
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


class PQueryInfo : public CommandListener<ListExpr>{
  public:
    PQueryInfo(Word& _stream , const string& _query,  ListExpr _tt) : 
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

       runner = boost::thread(boost::bind(&PQueryInfo::run,this));
    }

    virtual ~PQueryInfo(){
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
int pqueryVMT(Word* args, Word& result, int message,
                Word& local, Supplier s ){

   PQueryInfo* li = (PQueryInfo*) local.addr;
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
           local.addr = new PQueryInfo(args[0], query->GetValue(), tt);
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

ValueMapping pqueryVM[] = {
  pqueryVMT<CcString>,
  pqueryVMT<FText>
};

int pquerySelect(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}

/*
1.8.5 Sepcification

*/
OperatorSpec pquerySpec(
     " stream(int) x {text, string} x int -> stream(tuple(int, DATA))",
     " _ pquery[_,_]  ",
     " Performs the same query on a set of remote servers. "
     "The first argument is a stream of integer specifying the servers. "
     "The second argument is the query to execute. The result of this "
     "query has to be in kind DATA. The third argument is a server number "
     "for determining the exact result type of the query. The result is "
     "a stream of tuples consisting of the server number and the result "
     "of this server.",
     "query intstream(0,3) pquery['query ten count', 0] sum[Result]");

/*
1.8.6

Operator instance

*/
Operator pqueryOp (
    "pquery",             //name
     pquerySpec.getStr(),         //specification
     2,
     pqueryVM,        //value mapping
     pquerySelect,   //trivial selection function
     pqueryTM        //type mapping
);


/*
1.9 Operator ~pquery2~

This operator works quite similar as the pquery operator. The difference is
that the first argument is a tuple stream with at least two integer values.
The first value corresponds to the server number (like in pquery), the 
second number is a parameter for the remote query. The remote query contains 
somewhere the keywords SERVER and PART which are replaced by the 
corresponding values. For determining the result type, some
default parameters for part and server are given.


1.9.1 Type Mapping

*/

ListExpr pquery2TM(ListExpr args){

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
class PQuery2Info : public CommandListener<ListExpr>{
  public:
    PQuery2Info(Word& _stream , const string& _query,  
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
       runner = boost::thread(boost::bind(&PQuery2Info::run,this));
    }

    virtual ~PQuery2Info(){
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
int pquery2VMT(Word* args, Word& result, int message,
                Word& local, Supplier s ){

   PQuery2Info* li = (PQuery2Info*) local.addr;

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
                   local.addr = new PQuery2Info(args[0], q->GetValue(),
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

ValueMapping pquery2VM[] = {
  pquery2VMT<CcString>,
  pquery2VMT<FText>
};

int pquery2Select(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}

/*
1.8.5 Sepcification

*/
OperatorSpec pquery2Spec(
     " stream(tuple(...)) x {text, string} x AttrName x AttrName x "
     "int x int -> stream(tuple)",
     " _ pquery2[_,_,_,_,_]  ",
     " Performs a slightly different  query on a set of remote servers.  "
     " The first argument is a stream of tuples containing the server numbers"
     " as well as the modification number."
     " The second argument is the query to execute. The parts SERVER and PART "
     " are replaced by the corresponding numbers of the stream. "
     " The third argument specifies the Attribute name of the server number,"
     " the fourth argument specifies the attribute name for the part component."
     " The last two arguments are integer numbers used in typemapping as "
     " default values for server and part.",
     " query serverparts feed  pquery2['query ten_SERVER_PART count', Server, "
     "PART,0.5] sum[Result]");

/*
1.9.6

Operator instance

*/
Operator pquery2Op (
    "pquery2",             //name
     pquery2Spec.getStr(),         //specification
     2,
     pquery2VM,        //value mapping
     pquerySelect,   //trivial selection function
     pquery2TM        //type mapping
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
2.1.2 Class ~DArray~

This class represents the Secondo type ~darray~. It just stores the information
about a connection to a remote server. The actual connections are stored within
the algebra instance.

*/

enum arrayType{DARRAY,DFARRAY,DFMATRIX};

template<arrayType Type>
class DArrayT{
  public:
     DArrayT(const vector<uint32_t>& _map, const string& _name): 
           worker(),map(_map), size(map.size()),name(_name) {
        if(!stringutils::isIdent(name) || map.size() ==0 ){ // invalid
           name = "";
           defined = false;
           map.clear();
           size = 0;
           return;
        }
        defined = true;
     }
     DArrayT(const size_t _size , const string& _name): 
           worker(),map(), size(_size),name(_name) {
        assert(Type == DFMATRIX);
        if(!stringutils::isIdent(name) || size ==0 ){ // invalid
           name = "";
           defined = false;
           map.clear();
           size = 0;
           return;
        }
        defined = true;
     }

     DArrayT(const vector<uint32_t>& _map, const string& _name, 
               const vector<DArrayElement>& _worker): 
         worker(_worker),map(_map), size(map.size),name(_name) {

        if(!stringutils::isIdent(name) || map.size() ==0 || !checkMap()){ 
           // invalid
           name = "";
           defined = false;
           map.clear();
           size = 0;
           return;
        }
        defined = true;
     }

     DArrayT(const size_t _size, const string& _name, 
               const vector<DArrayElement>& _worker): 
         worker(_worker),map(), size(_size),name(_name) {
 
        assert(Type == DFMATRIX);
        if(!stringutils::isIdent(name) || size ==0 ){ 
           // invalid
           name = "";
           defined = false;
           map.clear();
           size = 0;
           return;
        }
        defined = true;
     }
     



     explicit DArrayT(int dummy) {} // only for cast function

 
     DArrayT(const DArrayT<Type>& src): 
             worker(src.worker), map(src.map), name(src.name), 
             defined(src.defined)
       {
       }

     template<arrayType T>
     DArrayT& operator=(const DArrayT<T>& src) {
        this->worker = src.worker;
        this->map = src.map;
        this->size = src.size;
        this->name = src.name;
        this->defined = src.defined;
        return *this;
     }     
 
     ~DArrayT() {
     }


    uint32_t getWorkerNum(uint32_t index){
       assert(Type!=DFMATRIX);
       return map[index];
    }


    arrayType getType(){
       return Type;
    }


    void setSize(size_t newSize){
       assert(newSize > 0);
       assert(Type == DFMATRIX);
       this->size = newSize;
    }


     void set(const size_t size, const string& name, 
              const vector<DArrayElement>& worker){
        if(!stringutils::isIdent(name) || size ==0){ // invalid
           this->name = "";
           this->defined = false;
           return;
        }
        defined = true;
        this->name = name;
        this->size = size;
        if(Type!=DFMATRIX){
             this->map = createStdMap(size,worker.size());
        } else {
             map.clear();
        }
        this->worker = worker;
     }

     template<class AT>
     bool equalMapping(AT& a, bool ignoreSize ){
        if(Type==DFMATRIX){ // mapping does not exist in DFMATRIX
           return true;
        }
        if(!ignoreSize && (map.size()!=a.map.size())){
           return false;
        }
        size_t minV = min(map.size(), a.map.size());
        for(size_t i=0;i<minV;i++){
           if(map[i]!=a.map[i]){
              return false;
           }
        }
        return true;
     }

     

    void set(const vector<uint32_t>& m, const string& name, 
              const vector<DArrayElement>& worker){
        if(!stringutils::isIdent(name) || m.size() ==0){ // invalid
           makeUndefined(); 
           return;
        }
        defined = true;
        this->name = name;
        this->map = m;
        this->size = m.size();
        this->worker = worker;
        if(Type==DFMATRIX){
           map.clear();
        }
        if(!checkMap()){
             makeUndefined();
        }
     }


     bool IsDefined(){
        return defined;
     }

     static const string BasicType() { 
       switch(Type){
          case DARRAY : return "darray";
          case DFARRAY : return "dfarray";
          case DFMATRIX : return "dfmatrix";
       }
       assert(false);
       return "typeerror";
     }

     static const bool checkType(const ListExpr list){
         if(!nl->HasLength(list,2)){
            return false;
         }  
         if(!listutils::isSymbol(nl->First(list), BasicType())){
             return false;
         }
         // for dfarrays and dmatrices, only relations 
         // are allowed as subtype
         if((Type == DFARRAY) || (Type==DFMATRIX)){
           return Relation::checkType(nl->Second(list));
         }

         // for a darray, each type is allowed
         SecondoCatalog* ctl = SecondoSystem::GetCatalog();
         string name;
         int algid, type;
         if(!ctl->LookUpTypeExpr(nl->Second(list), name, algid, type)){
            return false;
         }
         AlgebraManager* am = SecondoSystem::GetAlgebraManager();
         ListExpr errorInfo = listutils::emptyErrorInfo();
         if(!am->TypeCheck(algid,type,nl->Second(list),errorInfo)){
            return false;
         }
         return true;
     }

     size_t numOfWorkers() const{
       return worker.size();
     }
     size_t getSize() const{
        return size;
     }

     void setStdMap(size_t size){
         if(Type!=DFMATRIX){
            map = createStdMap(size, worker.size());
            this->size = size;
         } else {
            this->size = size;
            map.clear();
         }
     }

 
     DArrayElement getWorkerForSlot(int i){
        assert(Type!=DFMATRIX);
        if(i<0 || i>= map.size()){
           assert(false);
        }
        return getWorker(map[i]);
     }

     size_t getWorkerIndexForSlot(int i){
        assert(Type!=DFMATRIX);
        if(i<0 || i>= map.size()){
           assert(false);
        }
        return map[i];
     }
 

     DArrayElement getWorker(int i){
        if(i< 0 || i >= (int) worker.size()){
            assert(false);
           // throw "Invalid worker number";
        }
        return worker[i];
     }

     void setResponsible(size_t slot, size_t _worker){

       assert(size == map.size());
       assert(slot < size);
       assert(_worker < worker.size());
       assert(Type!=DFMATRIX);
       map[slot] = _worker;
     }


     ListExpr toListExpr(){
       if(!defined){
         return listutils::getUndefined();
       }

       ListExpr wl;
       if(worker.empty()){
         wl =  nl->TheEmptyList();
       } else {
           wl = nl->OneElemList(
                        worker[0].toListExpr());
           ListExpr last = wl;
           for(size_t i=1;i<worker.size();i++){
             last = nl->Append(last, worker[i].toListExpr());
           }
       }
       if(isStdMap() || (Type==DFMATRIX)){ 
          return nl->ThreeElemList(nl->SymbolAtom(name), 
                                   nl->IntAtom(size), 
                                   wl); 
       } else if(map.empty()) {
          return nl->ThreeElemList(nl->SymbolAtom(name), 
                                   nl->TheEmptyList(), 
                                   wl);
       } else {
          ListExpr lmap = nl->OneElemList(nl->IntAtom(map[0]));
          ListExpr last = lmap;
          for(size_t i=1;i<map.size();i++){
             last = nl->Append(last, nl->IntAtom(map[i]));
          }
          return nl->ThreeElemList(nl->SymbolAtom(name), lmap, wl);
       }
     }


     static DArrayT<Type>* readFrom(ListExpr list){
        if(listutils::isSymbolUndefined(list)){
           vector<uint32_t> m;
           return new DArrayT<Type>(m,"");
        }
        if(!nl->HasLength(list,3)){
           return 0;
        }
        ListExpr Name = nl->First(list);
        ListExpr Workers = nl->Third(list);
        if(   (nl->AtomType(Name) != SymbolType)
            ||(nl->AtomType(Workers)!=NoAtom)){
           return 0;
        }
        string name = nl->SymbolValue(Name);
        if(!stringutils::isIdent(name)){
           return 0;
        }
        vector<DArrayElement> v;
        int wn = 0;
        while(!nl->IsEmpty(Workers)){
           DArrayElement elem("",0,0,"");
           if(!InDArrayElement(nl->First(Workers), elem)){
              return 0;
           }
           elem.setNum(wn);
           wn++;
           v.push_back(elem);
           Workers = nl->Rest(Workers);
        }
        vector<uint32_t> m;
        if(nl->AtomType(nl->Second(list)==IntType)){
           int size = nl->IntValue(nl->Second(list));
           if(size <=0){
              return 0;
           }
           m = createStdMap(size,v.size());
        } else if(nl->AtomType(nl->Second(list))!=NoAtom){
           return 0;
        } else {
           ListExpr lm = nl->Second(list);
           while(!nl->IsEmpty(lm)){
               ListExpr first = nl->First(lm);
               lm = nl->Rest(lm);
               if(nl->AtomType(first)!=IntType){
                  return 0;
               }
               int mv = nl->IntValue(first);
               if(mv<0 || mv>=v.size()){
                  return 0;
               }
               m.push_back(mv);
           }
        }
        DArrayT<Type>* result = new DArrayT<Type>(m,name);
        swap(result->worker,v);
        result->defined = true;
        return result;
     }

     static bool open(SmiRecord& valueRecord, size_t& offset, 
                      const ListExpr typeInfo, Word& result){
        bool defined;
        result.addr = 0;
        if(!readVar<bool>(defined,valueRecord,offset)){
           return false;
        } 
        if(!defined){
          vector<uint32_t> m;
          result.addr = new DArrayT<Type>(m,"");
          return true;
        }
        // array in smirecord is defined, read size
        size_t size;
        if(!readVar<size_t>(size,valueRecord,offset)){
           return false;
        }
        // read name
        string name;
        if(!readVar<string>(name,valueRecord, offset)){
            return false;
        }

        // read  map
        uint32_t me;
        vector<uint32_t> m;
        DArrayT<Type>* res = 0; 

        if(Type!=DFMATRIX){
           for(size_t i=0;i<size;i++){
               if(!readVar<uint32_t>(me,valueRecord,offset)){
                  return false;
               }
               m.push_back(me);
           }
           res = new DArrayT<Type>(m,name);
        } else {
           res = new DArrayT<Type>(size,name);
        }
        int wn = 0;
        // append workers
        size_t numWorkers;
        if(!readVar<size_t>(numWorkers,valueRecord, offset)){
          return false;
        }
        for(size_t i=0; i< numWorkers; i++){
           DArrayElement elem("",0,0,"");
           if(!elem.readFrom(valueRecord, offset)){
               delete res;
               return false;
           }
           elem.setNum(wn);
           wn++;
           res->worker.push_back(elem);
        }

        result.addr = res;
        return true;
     }

     static bool save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value) {

         DArrayT<Type>* a = (DArrayT<Type>*) value.addr;
         // defined flag
         if(!writeVar(a->defined,valueRecord,offset)){
           return false;
         }
         if(!a->defined){
            return true;
         }
         // size
         size_t s = a->size;
         if(!writeVar(s,valueRecord,offset)){
           return false;
         }
         // name
         if(!writeVar(a->name, valueRecord, offset)){
           return false;
         }
         if(Type!=DFMATRIX){
           // map
           for(size_t i=0;i<a->map.size();i++){
               if(!writeVar(a->map[i], valueRecord,offset)){
                  return false;
               }
           }
         }
         // workers
         if(!writeVar(a->worker.size(), valueRecord, offset)){
           return false;
         }
         for(size_t i=0;i<a->worker.size();i++){
              if(!a->worker[i].saveTo(valueRecord,offset)){
                 return false;
              }
         }
         return true; 
     }


     static vector<uint32_t> createStdMap(const uint32_t size, 
                                          const int numWorkers){
        vector<uint32_t> map;
        if(Type!=DFMATRIX){
           for(uint32_t i=0;i<size;i++){
              map.push_back(i%numWorkers);
           }
        } 
        return map;
     }


     void print(ostream& out){
       if(!defined){
          out << "undefined";
          return;
       }

       out << "Name : " << name <<", size : " << map.size()
           << " workers : [" ;
       for(size_t i =0;i<worker.size();i++){
          if(i>0) out << ", ";
          worker[i].print(out);
       }
       out << "]";
       if(Type != DFMATRIX){
         out << "map = [";
         for(uint32_t i=0;i<map.size();i++){
           if(i>0){
             out << ", ";
           }
           out << i << " -> " << map[i];
         }
         out << "]";
       }
     }

     void makeUndefined(){
        worker.clear();
        map.clear();
        size = 0;
        name = "";
        defined = false;
     }

     string getName() const{
        return name;
     }

      bool setName( const string& n){
        if(!stringutils::isIdent(n)){
           return false;
        }
        name = n;
        return true;
      }

      template<class TE>
      bool equalWorker(const TE& a) const{
         return equalWorker(a.worker);
      }


      template<class H, class C>
      static DArrayT<Type> createFromRel(Relation* rel, int size, string name,
                              int hostPos, int portPos, int configPos){
          vector<uint32_t> m;
          DArrayT<Type> result(m,"");
          if(size<=0){
             result.defined = false;
             return result;
          }
          if(!stringutils::isIdent(name)){
             result.defined = false;
             return result;
          }
          result.defined = true;
          result.name = name;

          GenericRelationIterator* it = rel->MakeScan();
          Tuple* tuple;
          while((tuple = it->GetNextTuple())){
             DArrayElement* elem = 
                    DArrayElement::createFromTuple<H,C>(tuple,
                    result.worker.size(),hostPos, 
                    portPos, configPos);
             tuple->DeleteIfAllowed();
             if(elem){
                result.worker.push_back(*elem);
                delete elem;
             }
          } 
          delete it;
          result.setStdMap(size);
          return result;

      }



  friend class DArrayT<DARRAY>;
  friend class DArrayT<DFARRAY>;
  friend class DArrayT<DFMATRIX>;

  private:
    vector<DArrayElement> worker; // connection information
    vector<uint32_t> map;  // map from index to worker
    size_t  size; // corresponds with map size except map is empty
    string name;  // the basic name used on workers
    bool defined; // defined state of this array


   bool checkMap(){
     if(Type!=DFMATRIX){
        for(int i=0;i<map.size();i++){
           if(map[i] >= worker.size()){
              return false;
           }
        }
        return true;
     }
     return map.empty();
   }

   bool isStdMap(){
     if(Type!=DFMATRIX){
        int s = worker.size();
        for(size_t i=0;i<map.size();i++){
             if(map[i]!= i%s){
                 return false;
             }
        }
        return true;
     }
     return map.empty();
   }



   bool equalWorker(const vector<DArrayElement>& w) const{
      if(worker.size() != w.size()){
          return false;
      }
      for(size_t i=0;i<worker.size();i++){
           if(worker[i] != w[i]){
              return false;
           }
      }
      return true;
   }

};

typedef DArrayT<DARRAY> DArray;
typedef DArrayT<DFARRAY> DFArray;
typedef DArrayT<DFMATRIX> DFMatrix;


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
bool DArrayTypeCheck(ListExpr type, ListExpr& errorInfo){
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
  DArray::open, DArray::save,
  CloseDArray<DArray>, CloneDArray<DArray>,
  CastDArray<DArray>,
  SizeOfDArray,
  DArrayTypeCheck<DArray> );


TypeConstructor DFArrayTC(
  DFArray::BasicType(),
  DFArrayProperty,
  OutDArray<DFArray>, InDArray<DFArray>,
  0,0,
  CreateDArray<DFArray>, DeleteDArray<DFArray>,
  DFArray::open, DFArray::save,
  CloseDArray<DFArray>, CloneDArray<DFArray>,
  CastDArray<DFArray>,
  SizeOfDArray,
  DArrayTypeCheck<DFArray> );



TypeConstructor DFMatrixTC(
  DFMatrix::BasicType(),
  DFMatrixProperty,
  OutDArray<DFMatrix>, InDArray<DFMatrix>,
  0,0,
  CreateDArray<DFMatrix>, DeleteDArray<DFMatrix>,
  DFMatrix::open, DFMatrix::save,
  CloseDArray<DFMatrix>, CloneDArray<DFMatrix>,
  CastDArray<DFMatrix>,
  SizeOfDArray,
  DArrayTypeCheck<DFMatrix> );


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
  string err = "stream(TUPLE) x {string.text} expected";

  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  ListExpr fn = nl->Second(args);
  if(!CcString::checkType(fn) && !FText::checkType(fn)){
    return listutils::typeError(err);
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
  string err = "string or text expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  ListExpr argwt = nl->First(args);
  if(!nl->HasLength(argwt,2)){
     return listutils::typeError("internal error");
  }
  ListExpr arg = nl->First(argwt);
  if(!CcString::checkType(arg) && !FText::checkType(arg)){
    return listutils::typeError(err);
  }

  ListExpr query = nl->Second(argwt);
  Word queryResult;
  string typeString = "";
  string errorString = "";
  bool correct;
  bool evaluable;
  bool defined;
  bool isFunction;
  qp->ExecuteQuery(query, queryResult,
                    typeString, errorString, correct,
                    evaluable, defined, isFunction);
  if(!correct || !evaluable || !defined || isFunction){
     return listutils::typeError("could not extract filename ("+
                                  errorString + ")");
  }
  string filename;
  if(CcString::checkType(arg)){
     CcString* res = (CcString*) queryResult.addr;
     if(!res->IsDefined()){
       res->DeleteIfAllowed();
       return listutils::typeError("undefined filename");
     } else {
        filename = res->GetValue();
        res->DeleteIfAllowed();
     }
  }else {
     FText* res = (FText*) queryResult.addr;
     if(!res->IsDefined()){
       res->DeleteIfAllowed();
       return listutils::typeError("undefined filename");
     } else {
        filename = res->GetValue();
        res->DeleteIfAllowed();
     }
  }
  // access file for extracting the type 
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


/*
1.4.4 Value Mapping Array and Selection

*/
int ffeed5Select(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

ValueMapping ffeed5VM[] = {
  ffeed5VMT<CcString>,
  ffeed5VMT<FText>
};


/*
1.4.5 Specification

*/

OperatorSpec ffeed5Spec(
     " {string, text} -> stream(TUPLE) ",
     " ffeed5(_)",
     " Restores  a tuple stream from a binary file. ",
     " query ffeed5('ten.bin') count "
     );

/*
1.4.6 Instance

*/
Operator ffeed5Op(
   "ffeed5",
   ffeed5Spec.getStr(),
   2,
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
   string err = "stream(tuple) x int x string x any "
                               "x Ident x Ident x Ident expected";
   if(!nl->HasLength(args,7)){
      return listutils::typeError(err);
   }
   ListExpr stream = nl->First(args);
   if(!Stream<Tuple>::checkType(stream)){
     return listutils::typeError("First argument must be a tuple stream");
   }

   if(!CcInt::checkType(nl->Second(args))){
     return listutils::typeError("Second argument must be an int");
   }
   if(!CcString::checkType(nl->Third(args))){
     return listutils::typeError("Third argument must be a string");
   }

   ListExpr ha = nl->Fifth(args);
   ListExpr pa = nl->Sixth(args);
   ListExpr ca = nl->Seventh(args);
   if(    (nl->AtomType(ha) != SymbolType)
        ||(nl->AtomType(pa) != SymbolType)
        ||(nl->AtomType(ca) != SymbolType)){
     return listutils::typeError("One of the last three argument"
                                 " is not an Identifier");
   }
   ListExpr ht; // host type
   int hp; // host position
   string hn = nl->SymbolValue(ha);
   ListExpr attrList = nl->Second(nl->Second(stream));
   hp = listutils::findAttribute(attrList, hn, ht);
   if(!hp){
      return listutils::typeError("Attribute " + hn + " not found");
   }

   // fo the same for the port
   ListExpr pt;
   int pp; 
   string pn = nl->SymbolValue(pa);
   pp = listutils::findAttribute(attrList, pn, pt);
   if(!pp){
      return listutils::typeError("Attribute " + pn + " not found");
   }
   // and for the configuration
   ListExpr ct;
   int cp; 
   string cn = nl->SymbolValue(ca);
   cp = listutils::findAttribute(attrList, cn, ct);
   if(!cp){
      return listutils::typeError("Attribute " + cn + " not found");
   }
    
   // check correct types
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


   return nl->ThreeElemList(
                 nl->SymbolAtom(Symbols::APPEND()),
                 nl->ThreeElemList( nl->IntAtom(hp-1),
                                    nl->IntAtom(pp-1),
                                    nl->IntAtom(cp-1)),
                 resType);
}

/*
1.5.3 Value Mapping Template

*/
template<class H, class C>
int createDArrayVMT(Word* args, Word& result, int message,
          Word& local, Supplier s ){

   int host = ((CcInt*) args[7].addr)->GetValue();
   int port = ((CcInt*) args[8].addr)->GetValue();
   int config = ((CcInt*) args[9].addr)->GetValue();


   result = qp->ResultStorage(s);
   DArray* res = (DArray*) result.addr;

   CcInt* size = (CcInt*) args[1].addr;
   CcString* name = (CcString*) args[2].addr;
   if(!size->IsDefined() || !name->IsDefined()){
      res->makeUndefined();
      return 0; 
   }
   int si = size->GetValue();
   string n = name->GetValue();
   if(si<=0 || !stringutils::isIdent(n)){
      res->makeUndefined();
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
   return 0;
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
  ListExpr H = nl->Fifth(args);
  ListExpr C = nl->Seventh(args);

  ListExpr ht;
  ListExpr ct;
  ListExpr s = nl->Second(nl->Second(nl->First(args)));

  listutils::findAttribute(s,nl->SymbolValue(H),ht);
  listutils::findAttribute(s,nl->SymbolValue(C),ct);

  int n1 = CcString::checkType(ht)?0:1; 
  int n2 = CcString::checkType(ct)?0:1; 
  return n2 + n1*2;
}

/*
1.4.6 Specification

*/
OperatorSpec createDArraySpec(
     " stream<TUPLE> x int x string x ANY x attrName x "
     "attrName x attrName  -> darray",
     " _ createDArray[size, name, type template , HostAttr, "
                     "PortAttr, ConfigAttr]",
     " Creates a darray. The workers are given by the input stream. ",
     " query workers feed createDArray[6,\"obj\",streets, Host, Port, Config] "
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
   RelFileRestorer(ListExpr _relType, const string& _objName,
                   DArray* _array, int _arrayIndex, 
                   const string& _filename):
    //relType(_relType), 
    objName(_objName), array(_array), 
    arrayIndex(_arrayIndex), filename(_filename),
    started(false){
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
    string objName;
    DArray* array;
    int arrayIndex;
    string filename;
    bool started;
    boost::mutex mtx;
    boost::thread runner;
    bool res;
    ConnectionInfo* ci;

    static map<pair<string,int>,boost::mutex*> serializer;
    static boost::mutex sermtx;


    void run(){
      if(ci){
         res = ci->createOrUpdateRelationFromBinFile(objName,filename);
         if(!res){
           cerr << "createorUpdateObject failed" << endl;
         }
         sermtx.lock();
         pair<string,int> p(ci->getHost(), ci->getPort());
         serializer[p]->unlock();
         sermtx.unlock();
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

     FRelCopy(ListExpr _relType, const string& _objName,
              DFArray* _array, int _arrayIndex, 
              const string& _filename):
     name(_filename), slot(_arrayIndex), array(_array), started(false),ci(0)
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
     string name;
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
        // send file to remote server
        ci->sendFile(name,name,true);
        // get target directory
        int err;
        string errmsg;
        ListExpr result; 
        double runtime;
        string home = ci->getSecondoHome();

        string targetDir = home +"/dfarrays/"+dbname+ "/" + array->getName();
        // create target directory
        string cmd = "query createDirectory('"+targetDir+"', TRUE)";

        
        ci->simpleCommand(cmd,err,errmsg, result,false, runtime);
        if(err!=0){
            cerr << "command " << cmd << " failed" << endl;
            cerr << err << " : " << errmsg << endl;
            return;
        }
        // result will be false, if directory already exists
        // hence ignore result here
        string sendDir = ci->getSendFolder();
        string src = home+'/'+sendDir + "/"+name;
        string target = targetDir+"/"+name;
        cmd = "query moveFile('"+src+"','"+target+"')";
        ci->simpleCommand(cmd,err,errmsg, result,false, runtime);
        if(err!=0){
            cerr << "command " << cmd << " failed" << endl;
            cerr << err << " : " << errmsg << endl;
            return;
        }
        if(!nl->HasLength(result,2) || 
            nl->AtomType(nl->Second(result))!=BoolType){
            cerr << "moveFile returns unexpected result: " 
                 << nl->ToString(result) << endl;
            return;
        }

        if(!nl->BoolValue(nl->Second(result))){
           cerr << "moving file from: " << src << endl
                << "to " << target << endl
                << "failed" << endl;   
        
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


   (*res) =  AType::template createFromRel<HostType,ConfigType>(rel,size,name,
                                       hostPos,portPos,configPos);


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
      restorers.push_back(new DType(relType, objName,res, i, fn));
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

   (*res) = AType::template createFromRel<HType,CType>(workers, 1, 
                 name, hostPos, portPos, configPos);
   

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
      restorers.push_back(new DType(relType, objName,res, 
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

   (*res) = AType::template createFromRel<HType,CType>(rel, size,
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
      restorers.push_back(new DType(relType, objName,res, i, fn));
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


  string err = "darray(X) x string x fun: X -> Y  expected";
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

  if(!DArray::checkType(arrayType)){
    return listutils::typeError(err + ": first arg not a darray");
  }
  if(!listutils::isMap<1>(funType)){
    return listutils::typeError(err + ": last arg is not a function");
  }

  if(!nl->Equal(nl->Second(arrayType), nl->Second(funType))){
    return listutils::typeError("type mismatch between darray and "
                                "function arg");
  }

  ListExpr result = nl->TwoElemList(listutils::basicSymbol<DArray>(),
                                    nl->Third(funType));

  if(!DArray::checkType(result)){
    return listutils::typeError("Invalid function result");
  }

  ListExpr funquery = nl->Second(fun);
  
  ListExpr funargs = nl->Second(funquery);

  ListExpr dat = nl->Second(arrayType);

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
   dloopInfo(DArray* _array, int _index, string& _resName,string& _fun) : 
     index(_index), resName(_resName), fun(_fun), elem("",0,0,""){
     elem = _array->getWorkerForSlot(index);
     srcName = _array->getName();
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
    string srcName;
    boost::thread runner;

    void run(){
       string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
       ConnectionInfo* ci = algInstance->getWorkerConnection(elem,dbname);
       if(!ci){
          sendError("Cannot find connection ");
          return;
       }
       string bn = resName + "_" + stringutils::int2str(index);
       int err;
       string strres;
       double runtime;
       string errMsg;
       // delete old array content
       ci->simpleCommand("delete "+ bn, err,strres, false, runtime);
       // create new object by applying the function
       string cmd =   "(let "+bn+" = ("+fun+" " + srcName + "_" 
                    + stringutils::int2str(index) +"))";
       ci->simpleCommandFromList(cmd, err,errMsg, strres, false, runtime);
       if(err!=0){ 
           sendError(" Problem in command " + cmd + ":" + errMsg);
       }
    }
    void sendError(const string&msg){
       cmsg.error() << msg;
       cmsg.send();
    }

};


int dloopVM(Word* args, Word& result, int message,
           Word& local, Supplier s ){

   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   DArray* array = (DArray*) args[0].addr;
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
     dloopInfo* r = new dloopInfo(array,i,n,f);
     runners.push_back(r);
   }

   for(size_t i=0;i<runners.size();i++){
     delete runners[i];
   }
   return 0; 
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
  dloopVM,
  Operator::SimpleSelect,
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
         case DFMATRIX : assert(false); break;
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
         cmsg.error() << "could not open connection" ;
         cmsg.send();
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
    cmsg.error() << "Different workers in dloop2";
    cmsg.send();
    res->makeUndefined();
    return 0;
  } 

  vector<DArrayElement> workers;

  for(size_t i=0;i<a1->numOfWorkers();i++){
    if(a1->getWorker(i) != a2->getWorker(i)){
        cmsg.error() << "Different workers in dloop2";
        cmsg.send();
        res->makeUndefined();
        return 0;
    }
    workers.push_back(a1->getWorker(i));
  } 

  int max = min(a1->getSize(), a2->getSize());

  if(!a1->equalMapping(*a2,true)){
     cmsg.error() << "Different mappings in dloop2";
     cmsg.send();
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
           cmsg.error() << "could not open connection" ;
           cmsg.send();
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
           cmsg.error() << "could not open connection" ;
           cmsg.send();
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
           cmsg.error() << "could not open connection" ;
           cmsg.send();
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
           cmsg.error() << "could not open connection" ;
           cmsg.send();
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
           cerr << "copyFile command failed with code " << err << endl;
           cerr << " Msg " << errMsg << endl;
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
      cout << "create file" << endl;
      if(!fileCreated){
          cout << "first time" << endl;
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
       a = A::template createFromRel<CcString, CcString>(
               (Relation*) args[2].addr, 400, "tmp",
                hostPos, portPos, confPos);
    } else if(hostStr && !confStr){
       a = A::template createFromRel<CcString, FText>(
               (Relation*) args[2].addr, 400, "tmp",
                hostPos, portPos, confPos);
    } else if(!hostStr && confStr){
       a = A::template createFromRel<FText, CcString>(
               (Relation*) args[2].addr, 400, "tmp",
                hostPos, portPos, confPos);
    } else if(!hostStr && !confStr){
       a = A::template createFromRel<FText, FText>(
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
  string err = "dfarray(X)  x string x fun expected";
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


  ListExpr stream = nl->TwoElemList(
                     listutils::basicSymbol<Stream<Tuple> >(),
                     nl->Second(nl->Second(arg1Type)));

  ListExpr funArg = nl->Second(arg3Type);

  ListExpr expFunArg = DArray::checkType(arg1Type)
                      ? nl->Second(arg1Type)
                      : stream; 


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
   Mapper(A* _array, CcString* _name, FText* _funText ,
          bool _isRel, bool _isStream, void* res):
          array(_array), ccname(_name), funText(_funText), isRel(_isRel),
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
               cmd = "query "+ funName +"( ffeed5 ('" + fname1+"'))";
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
              cerr << "command " + cmd << " failed with code " << err 
                   << endl;
              cerr << "message is " << errMsg;  
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
           cerr << "problem in command " << cmd;
           cerr << "code : " << err << endl;
           cerr << "msg : " << errMsg << endl;
           return;
          } 
             
          string stream;
          string n = mapper->array->getName()+"_"+stringutils::int2str(nr);

          if(mapper->array->getType()==DFARRAY){  
              string fname1 = ci->getSecondoHome()+"/dfarrays/"+dbname+"/"
                              + mapper->array->getName() + "/"
                              + n + ".bin";
               stream = "ffeed5 ('"+fname1 + "')";
          } else {
               stream = n + " ";
          }

          string name2 = mapper->name + "_" + stringutils::int2str(nr);
          cmd = "let "+ name2 +" = " + funName +"( " + stream + ")";

          ci->simpleCommand(cmd,err,errMsg,r,false, runtime);
          if((err!=0)  ){ // ignore type map errors
                                    // because reason is a missing file
             cerr << "command " + cmd << " failed with code " << err 
                  << endl;
             cerr << "message is " << errMsg;  
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
  Mapper<A> mapper(array, name, funText, isRel, isStream, result.addr);
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
           cerr << "command " << cmd << " faile with code " << err << endl
                << errMsg << endl;
        }
        // TODO check result

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
        }
     }
   }
   for(size_t i=0;i<starters.size();i++){
      delete starters[i];
   }
   starters.clear();
   return true;

}




/*
20 Operator dmap2

20.1 Type Mapping

*/

ListExpr dmap2TM(ListExpr args){

 string err = "d[f]array(X) x d[f]array[Y] x string x "
              "fun : (X x Y -> Z) x int expected";
 if(!nl->HasLength(args,5)){
   return listutils::typeError(err + " (wrong number of args)");
 }
 ListExpr first = nl->First(args);
 ListExpr second = nl->Second(args);
 ListExpr third = nl->Third(args);
 ListExpr fourth = nl->Fourth(args);
 ListExpr fifth = nl->Fifth(args);

 if(!nl->HasLength(first,2) || !nl->HasLength(second,2) ||
    !nl->HasLength(third,2) || !nl->HasLength(fourth,2) ||
    !nl->HasLength(fifth,2)){
   return listutils::typeError("internal error");
 }
 // extract type information
 first = nl->First(first);
 second = nl->First(second);
 third = nl->First(third);
 fourth = nl->First(fourth);
 fifth = nl->First(fifth);

 
 if(     !DArray::checkType(first) 
     && !DFArray::checkType(first)){
   return listutils::typeError(err + "( firt arg not a d[f]array)");
 }
 if(     !DArray::checkType(second) 
     && !DFArray::checkType(second)){
   return listutils::typeError(err + "( second arg not a d[f]array)");
 }
 if(!CcString::checkType(third)){
   return  listutils::typeError(err + " (third arg is not a string)");
 }
 if(!listutils::isMap<2>(fourth)){
   return listutils::typeError(err + " (fourth arg is not a function)"); 
 }
 if(!CcInt::checkType(fifth)){
   return listutils::typeError(err + " (fifth arg is not an int)");
 }

 ListExpr a1 = DFArray::checkType(first)
               ? nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                                  nl->Second(nl->Second(first)))
               : nl->Second(first);

 ListExpr a2 = DFArray::checkType(second)
               ? nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                                  nl->Second(nl->Second(second)))
               : nl->Second(second);
 
 ListExpr fa1 = nl->Second(fourth);
 ListExpr fa2 = nl->Third(fourth);
 bool streamRes = false;

 if(!nl->Equal(a1,fa1) || !nl->Equal(a2,fa2)){
   return listutils::typeError("function arguments do not fit to the "
                               "darray subtypes");
 }

 ListExpr funres = nl->Fourth(fourth);
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

 ListExpr funQ = nl->Second(nl->Fourth(args)); 
 ListExpr funarg1 = nl->Second(funQ);
 ListExpr funarg2 = nl->Third(funQ);

 ListExpr rfunarg1 = nl->TwoElemList(
                         nl->First(funarg1),
                         a1);
 ListExpr rfunarg2 = nl->TwoElemList(
                         nl->First(funarg2),
                         a2);

 ListExpr rfun = nl->FourElemList(
      nl->First(funQ),
      rfunarg1,
      rfunarg2,
      nl->Fourth(funQ)
   );

 return nl->ThreeElemList(
           nl->SymbolAtom(Symbols::APPEND()),
           nl->TwoElemList(
                 nl->BoolAtom(streamRes),
                 nl->TextAtom( nl->ToString(rfun))),
           resType);
}


template<class A1, class A2, class R>
class dmap2Info{

  public:

    dmap2Info( A1* _array1, A2* _array2, R* _res, 
              const string& _funtext, const string& _objName,
              bool _streamRes, int _port, ListExpr _arg1Type,
              ListExpr _arg2Type) :
        array1(_array1), array2(_array2), res(_res),
        objName(_objName),funtext(_funtext), streamRes(_streamRes), 
        port(_port), arg1Type(_arg1Type), arg2Type(_arg2Type) {
          dbname = SecondoSystem::GetInstance()->GetDatabaseName();
          res->setName(objName) ;
          arg2IsRel = Relation::checkType(nl->Second(arg2Type));
       }


    void start(){

       if(!array1->IsDefined() || !array2->IsDefined()){
         res->makeUndefined();
         return;
       }
       if(array1->numOfWorkers() <1 || array2->numOfWorkers() < 1){
         res->makeUndefined();
         return;
       }


       if(!array1->equalMapping(*array2,true)){
          // transfer of files or objects required
          if(port <=0){
             // but not possible with given portA
             res->makeUndefined();
             return;
          }
          // try to start staticFileTransferators on worker 
          // of array2
          if(!startFileTransferators(array2,port)){
             res->makeUndefined();
             return;
          }
       }


       *res = *array1;
       res->setName(objName);

       size_t resSize = min(array1->getSize(), array2->getSize());
       if(resSize==0){
         res->setStdMap(resSize);
         return;
       }

       vector<Run*> w;
       vector<boost::thread*> runners;
       for(size_t i=0;i< resSize; i++) {
           DArrayElement elem1 = array1->getWorkerForSlot(i);
           DArrayElement elem2 = array2->getWorkerForSlot(i);
           bool transferRequired = false;
           if(elem1.getHost() != elem2.getHost()){
                transferRequired = true;
           }
           if(!transferRequired){
               if(array2->getType()!=DFARRAY){
                 transferRequired = elem1.getPort() != elem2.getPort();
               }
           }

           ConnectionInfo* ci1 = algInstance->getWorkerConnection(
                                    array1->getWorkerForSlot(i), dbname);
           ConnectionInfo* ci2 = algInstance->getWorkerConnection(
                                    array2->getWorkerForSlot(i), dbname);
           if(ci1 && ci2){
              Run* r = new Run(this,i,ci1, transferRequired, ci2);
              w.push_back(r);
              boost::thread* runner = new boost::thread(&Run::run,r);
              runners.push_back(runner); 
           }
       }

       for(size_t i=0;i<w.size();i++){
           runners[i]->join();
           delete runners[i];
           delete w[i];
       }
    }


  private:
    A1* array1;
    A2* array2;
    R* res;
    string objName;
    string funtext;
    bool streamRes;
    int port;
    ListExpr arg1Type;
    ListExpr arg2Type;
    bool arg2IsRel;
    string dbname;

    class Run{
      public:
         Run(dmap2Info* _mi, size_t _i, ConnectionInfo* _ci1, 
             const bool _transferRequired, ConnectionInfo* _ci2):
             mi(_mi),i(_i),ci1(_ci1), transferRequired(_transferRequired), 
             ci2(_ci2), tempObject(""), tempFile(""){ }

      void run(){
           // step 1 create function on server
             string funName = "tmpfun_"+stringutils::int2str(ci1->serverPid())
                              + "_" + stringutils::int2str(i);
             string cmd = "(let " + funName + " = " 
                        + mi->funtext+")";
             int err; string errMsg; string r;
             double runtime;
             ci1->simpleCommand("delete "+funName,err,errMsg,r,false, runtime);
             ci1->simpleCommandFromList(cmd,err,errMsg,r,true, runtime);
             if(err!=0){
               cerr << "problem in command " << cmd;
               cerr << "code : " << err << endl;
               cerr << "msg : " << errMsg << endl;
               return;
             } 
             string fa1 = getFunArg(mi->array1,ci1);
             string fa2 = getFunArg(mi->array2,ci2);



             if(mi->res->getType()!=DFARRAY){
               cmd = "let " + mi->objName + "_"+stringutils::int2str(i)
                     + " = " + funName + "(" + fa1+ ", " + fa2+")";   
             } else {
               string tdir = ci1->getSecondoHome()+"/dfarrays/"+mi->dbname+"/"
                              + mi->res->getName() + "/";
               cmd = "query createDirectory('"+tdir+"', TRUE)";
               ci1->simpleCommand(cmd,err,errMsg,r,true, runtime);
               if(err){
                 cerr << "could not create directory " << tdir << endl;
                 cerr << "by cmd: " << cmd << endl;
                 cerr << errMsg;
               }
               string fname = tdir
                              + mi->res->getName() + "_" 
                              + stringutils::int2str(i) + ".bin";
               cmd ="query " + funName+"("+fa1+", " + fa2+")";

               if(!mi->streamRes){
                 cmd += " feed";
               } 
               cmd += " fconsume5['" + fname+"'] count";
             }

             ci1->simpleCommand(cmd,err,errMsg,r,true, runtime);
             if(err!=0){
                cerr << "command : " << cmd << endl;
                cerr << "command failed with rc " << err << endl;
                cerr << errMsg << endl;
             }
             ci1->simpleCommand("delete " + funName , err,errMsg,r,false,
                          runtime);

             deleteTempObject();
      }

      private:
        dmap2Info* mi;
        size_t i;
        ConnectionInfo* ci1;
        bool transferRequired;
        ConnectionInfo* ci2;
        string tempObject;
        string tempFile;


        string getFunArg(const DArray* array, ConnectionInfo* ci){
           if(transferRequired && (ci==ci2)){
             retrieveObject(array);
             return tempObject;
           }
           return array->getName()+"_" + stringutils::int2str(i);
        }
        
        string getFunArg(const DFArray* array, ConnectionInfo* ci){
          string fname = ci->getSecondoHome()+"/dfarrays/"+mi->dbname+"/"
                       + array->getName() + "/"
                       + array->getName()+"_"+stringutils::int2str(i)
                       + ".bin";
           if(transferRequired && (ci==ci2)){
              retrieveFile(array);
              fname = tempFile; 
           } 
           return "ffeed5('" + fname+"')";
        }


        void deleteTempObject(){
           if(tempFile.size()>0){
              FileSystem::DeleteFileOrFolder(tempFile);
              tempFile="";
           }
           if(tempObject.size()>0){
              int err;
              string errMsg;
              string r;
              double runtime;
              string cmd = "delete " + tempObject;
              ci1->simpleCommand( cmd, err,errMsg,r,false, runtime);
              if(err){
                 cerr << "cmd << "  << cmd << " failed  with code "
                      << err << endl;
                 cerr << errMsg;
              }  
           }
        }


        void retrieveFile(const DFArray* array){

          string remoteName = ci2->getSecondoHome()+"/dfarrays/"+mi->dbname+"/"
                                + array->getName() + "/"
                                + array->getName()+"_"+stringutils::int2str(i)
                                + ".bin";

          stringstream ss;
          ss << "temp_" << array->getName() << "_" << i << "_" 
             <<  ci1->serverPid() 
             << "_" << ci2->serverPid();
          tempFile = ss.str();
          // on ci2 should already run a staticFileTransferator on port port 
          int err;
          string errMsg;
          string r;
          double runTime;
          string cmd =   "query getFileTCP('" + remoteName+"', '" 
                       + ci2->getHost() 
                       + "', " + stringutils::int2str(mi->port) 
                       + ", TRUE, '" + tempFile+"')";
          ci1->simpleCommand( cmd, err,errMsg,r,false, runTime);
          if(err){
              cerr << "cmd << " << cmd << " failed  with code " << err << endl;
              cerr << errMsg;
           }  
        }

        void retrieveObject(const DArray* array){
            string originalName = array->getName() + "_" 
                                  + stringutils::int2str(i);
            stringstream ss;
            ss << "temp_" << array->getName() << "_" << i << "_" 
               <<  ci1->serverPid() 
               << "_" << ci2->serverPid();
            tempObject = ss.str();

            // step 1 create a file containing the object on ci2
            int err;
            string errMsg;
            string r;
            double runTime;
            string cmd;
            if(!mi->arg2IsRel){
                cmd =   "save " + originalName + " to " + tempObject;
            } else {
                cmd = "query " + originalName + " feed fconsume5['" 
                      + tempObject +"'] count";
            }
            ci2->simpleCommand( cmd, err,errMsg,r,false, runTime);
            if(err){
              cerr << "cmd << " << cmd << " failed  with code " << err << endl;
              cerr << errMsg;
              return;
            }  
            // step2 transfer this file to ci1
            cmd =   "query getFileTCP('" + tempObject +"', '" 
                       + ci2->getHost() 
                       + "', " + stringutils::int2str(mi->port) 
                       + ", TRUE, '" + tempObject+"')";
            ci1->simpleCommand( cmd, err,errMsg,r,false, runTime);
            if(err){
              cerr << "cmd << " << cmd << " failed  with code " << err << endl;
              cerr << errMsg;
              return;
            }  
            // step3 create the temporary object from this file
            if(!mi->arg2IsRel){
               cmd = "restore " + tempObject + " from " + tempObject;
            } else {
               cmd = "let " + tempObject + " = ffeed5('" + tempObject
                     + "') consume";
            }
            ci1->simpleCommand( cmd, err,errMsg,r,false, runTime);
            if(err){
              cerr << "cmd << " << cmd << " failed  with code " << err << endl;
              cerr << errMsg;
              return;
            }  
            // step4 delete temp files on ci1 and ci2
            cmd = "query removeFile('" + tempObject+"')";
            ci1->simpleCommand( cmd, err,errMsg,r,false, runTime);
            if(err){
              cerr << "cmd << " << cmd << " failed  with code " << err << endl;
              cerr << errMsg;
            }  
            ci2->simpleCommand( cmd, err,errMsg,r,false, runTime);
            if(err){
              cerr << "cmd << " << cmd << " failed  with code " << err << endl;
              cerr << errMsg;
            }  

              

        }




    };
};


template<class A1, class A2>
int dmap2VMT(Word* args, Word& result, int message,
            Word& local, Supplier s ){

    result = qp->ResultStorage(s);
    A1 * a1 = (A1*) args[0].addr;
    A2* a2 = (A2*) args[1].addr;
    CcString* objName = (CcString*) args[2].addr;
    // args[3] is the original fun and is not used here
    CcInt* Port = (CcInt*) args[4].addr;
    bool streamRes =  ((CcBool*)args[5].addr)->GetValue();
    string funtext = ((FText*) args[6].addr)->GetValue();

    string n;

    int port = 0;
    if(Port->IsDefined()){
       port = Port->GetValue();
    }

    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    if(!objName->IsDefined() || objName->GetValue().length()==0){
      algInstance->getWorkerConnection(a1->getWorker(0),dbname);
      n = algInstance->getTempName();            

    } else {
        n = objName->GetValue();
    }

    bool isFileBased = DFArray::checkType(qp->GetType(s));


    if(!stringutils::isIdent(n)){
        if(isFileBased){
           ((DFArray*)result.addr)->makeUndefined();
        } else {
           ((DArray*)result.addr)->makeUndefined();
        }
        return 0;
    }

    if(isFileBased){
        dmap2Info<A1,A2,DFArray> info(a1,a2,(DFArray*) result.addr, 
                                      funtext, n,streamRes, port, 
                                      qp->GetType(qp->GetSon(s,0)),
                                      qp->GetType(qp->GetSon(s,1)));
        info.start();
    } else {
        dmap2Info<A1,A2,DArray> info(a1,a2,(DArray*) result.addr, 
                                     funtext, n,streamRes, port,
                                     qp->GetType(qp->GetSon(s,0)),
                                     qp->GetType(qp->GetSon(s,1)));
        info.start();
    }
    return 0;

}


int dmap2Select(ListExpr args){

  int n1 = DArray::checkType(nl->First(args))?0:2;
  int n2 = DArray::checkType(nl->Second(args))?0:1;
  return n1 + n2;
}

ValueMapping dmap2VM[] = {
   dmap2VMT<DArray,DArray>,
   dmap2VMT<DArray,DFArray>,
   dmap2VMT<DFArray,DArray>,
   dmap2VMT<DFArray,DFArray>
};

OperatorSpec dmap2Spec(
  "d[f]array x d[f]array x string x fun -> d[f]array",
  "_ _ dmap2[_,_]",
  "Joins the slots of two distributed arrays",
  "query df1 df2 dmap2 [\"df3\" . .. product]"

);


Operator dmap2Op(
  "dmap2",
  dmap2Spec.getStr(),
  4,
  dmap2VM,
  dmap2Select,
  dmap2TM
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
TypeMapOperators ARRAYFUNARG1 and ARRAYFUNARG2

*/
template<int pos>
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
     ListExpr res = nl->TwoElemList(
               listutils::basicSymbol<Stream<Tuple> >(),
               nl->Second(nl->Second(arg)));
     return res;

  }
  return listutils::typeError("Invalid type found");

}

OperatorSpec ARRAYFUNARG1SPEC(
  "darray(X) x ... -> X, dfarray(rel(X)) x ... -> stream(X)",
  "ARRAYFUNARG1(_)",
  "Type mapping operator.",
  "query df1 df2 map [\"df3\" . .. product]"
);

Operator ARRAYFUNARG1OP(
  "ARRAYFUNARG1",
   ARRAYFUNARG1SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   ARRAYFUNARG<1>
);

OperatorSpec ARRAYFUNARG2SPEC(
  " any x darray(X) x ... -> X, any x dfarray(rel(X)) x ... -> stream(X)",
  " ARRAYFUNARG2(_)",
  "Type mapping operator.",
  "query df1 df2 map [\"df3\" . .. product]"
);
Operator ARRAYFUNARG2OP(
  "ARRAYFUNARG2",
   ARRAYFUNARG2SPEC.getStr(),
   0,
   Operator::SimpleSelect,
   ARRAYFUNARG<2>
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
          cerr << "command " << q << " failed with code " << err << endl;
          cerr << errMsg << endl;
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
     cerr << "Client command " << clientQuery << " failed with code " 
          << err << endl;
     cerr << errMsg << endl;
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
      ofstream out(local.c_str(), ios::binary|ios::trunc);
      if(!out.good()){
         errMsg = "could not create local file";
         return false;
      }
      // connect to server
      Socket* socket = Socket::Connect(host, stringutils::int2str(port), 
                              Socket::SockGlobalDomain, 3, 1);

      if(!socket){
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
           cerr << "command for creating directory failed : " << errMsg;
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
           cerr << " Command : " << cmd << " failed " << endl;
           cerr << errMsg << endl;
        }
     }



     string constructQuery(const string& targetDir){
        switch(array->getType()){
           case DARRAY : return constructQueryD(targetDir);
           case DFARRAY : return constructQueryDF(targetDir);
           case DFMATRIX : assert(false); 
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
        string stream2 = "(feedS " + stream1 + "("+nl->ToString(relType) 
                         + " ()))"; 
        string stream3 = stream2;
        if(!sfun.empty()){
          stream3 = "("+ sfun +   stream2 + ")";
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

        ListExpr fsfeed = nl->ThreeElemList(
                                nl->SymbolAtom("fsfeed5"),
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

   (*res) = (*array);

   res->setName(tname);
   res->setSize(size);
   
   vector<partitionInfo<A>*> infos;

   ListExpr relType = nl->Second(qp->GetType(qp->GetSon(s,0)));
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();


   for(size_t i=0;i<array->numOfWorkers();i++){
      DArrayElement de = array->getWorker(i);
      ConnectionInfo* ci = algInstance->getWorkerConnection(de,dbname);
    //  string ip = ci->getHost();
    //  string home = ci->getSecondoHome();
    //  pair<string,string> p(ip,home);
      partitionInfo<A>* info = new partitionInfo<A>(array, size, i, ci,funText,
                                             dfunText, tname, relType, dbname);
      infos.push_back(info);
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
  "query da2 partition[ hashValue(.Name,2000) + 23, \"dm2\",0]"
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
               "(stream(tuple(X)) -> stream(tuple(Y))) x "
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
  ListExpr arrayStream = nl->TwoElemList(
                               listutils::basicSymbol<Stream<Tuple> >(),
                               nl->Second(subtype));

  if(!nl->Equal(arrayStream, nl->Second(f1))){
     return listutils::typeError(" stream argument of function does not "
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
                             arrayStream),
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
    return listutils::typeError("1 or two arguments expected") ;
  }

  if(nl->HasLength(args,2)){
    ListExpr arg = nl->First(args);
    if(!DArray::checkType(arg) && !DFArray::checkType(arg)){
       return listutils::typeError("d[f]array expected");
    }
    if(!Relation::checkType(nl->Second(arg))){
      return listutils::typeError("D[f]array's subtype muts be a relation");
    }
    return nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                            nl->Second(nl->Second(arg)));
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
  "d[f]array(rela(tuple(X))) x A -> stream(tuple(X)) or  "
  "B x C x (stream(tuple(D) -> stream(tuple(E)) ) -> tuple(E)",
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
  "d[f]array(rel(X)) x string x stream(X)->stream(Y)) x (Y -> int) x "
  "int -> dfmatrix(rel(Y)) ",
  "_ partitionL[_,_,_,_] ",
  "Repartitions a distributed [file] array. Before repartition, a "
  "function is applied to the slots.",
  "query a1 partitionL[ \"name\", . head[12], hashvalue(.Attr,23), 0]" 
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

  string err = "dmatrix(rel(t)) x string x (stream(t)-Y) x int expected";
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
  ListExpr tupleStream = nl->TwoElemList(
                             listutils::basicSymbol<Stream<Tuple> >(),
                             tupleType);

  ListExpr funArg = nl->Second(f1);

  if(!nl->Equal(tupleStream,funArg)){
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

  ListExpr dat = tupleStream;

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
          // dummy implementation
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

          string tuplestream1 = " ( fsfeed5 (projecttransformstream" 
                                + tuplestream0_1 + " L )(" 
                                + nl->ToString(relType1) + "()) )";

          string tuplestream2 = ""; // no tuple stream
          if(matrix2){
             tuplestream2 =   "( fsfeed5 (projecttransformstream" 
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
              cerr << " command " << cmd <<  " failed " << endl
                   << errMsg;
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
      cout << "Matrix is undefined" << endl;
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
         cout << "matrix2 is undefined" << endl;
         res->makeUndefined();
         return 0;
      }

      if(!matrix1->equalWorker(*matrix2)){
         cerr << "different worker specification" << endl;
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

   (*res) = *matrix1;
   res->setName(resName);
   res->setStdMap(resSize);

   
   startFileTransferators(matrix1, port); // do not the same for matrix2 
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
   "dfmatrix(rel(t)) x string x (stream(t)->Y) x int -> d[f]array(Y)",
   "matrix areduce[newname, function, port]",
   "Performs a function on the distributed slots of an array. "
   "The task distribution is dynamicalle, meaning that a fast "
   "worker will handle more slots than a slower one. "
   "The result type depends on the result of the function. "
   "For a relation or a tuple stream, a dfarray will be created. "
   "For other non-stream results, a darray is the resulting type.",
   "The integer argument specifies the port for transferring files.",
   "query m8 areduce[ . count, 1237]"
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

  string err = "dfmatrix(rel(X)) x dfmatrix(rel(Y)) x string x (stream(X) x "
               "stream(Y) -> Z) x int expected";

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
       listutils::basicSymbol<Stream<Tuple> >(),
       nl->Second(nl->Second(a1t))
  );
 
  ListExpr s2 = nl->TwoElemList(
       listutils::basicSymbol<Stream<Tuple> >(),
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


  cout << "areduce2TM isStream : " << isStream << endl;

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
  "dfmatrix(rel(X)) x dfmatrix(rel(Y) x string x (stream(X) x "
  "stream(Y) -> Z) x int -> d[f]array(Y)",
  "_ _ areduce2[_,_,_] ",
  "Performs areduce function to two dfmatrices. The result type depends "
  "on the return type of the operation. If the result is a stream of tuples "
  "or a relation, the rseult type will be a dfarray, otherwise a darray. "
  "Streams of non-tuples are not alowed as the function result type. "
  "The string argument specified the name of the result array. "
  "The integer specified a port for transferring files between the workers. "
  "On this port automatically a staticFileTransferator is started.",
  "query dfm1 dfm2 areduce2[ \"molly\", . .. product , 1236]"
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

string getUDRelType(ListExpr r){

  assert(Relation::checkType(r));
  ListExpr attrList = nl->Second(nl->Second(r));
  string res = "rel(tuple([";
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
          cerr << "command failed " << cmd << endl; 
          cerr << errMsg << endl;
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
   (*res) = (*matrix);
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
2.1.1 Class ~taskElement~

This class represents the Elements of Code.

*/
class TaskElement{
 public:
    TaskElement( Word* args, Word& result, Supplier s): pos(0){
      tt = new TupleType(nl->Second(GetTupleResultType(s)));
      
      
      a1Name = ((DArray*) args[0].addr)->getName();
      a2Name = ((DArray*) args[1].addr)->getName();
      funQuery = (FText*) args[4].addr;
      max = min(((DArray*) args[0].addr)->getSize(), 
                ((DArray*) args[1].addr)->getSize());

      resName = ((DArray*) result.addr)->getName();
    }

    TaskElement( size_t _pos):pos(_pos){
      pos =_pos;
      dep = 0;
      a1Name = "";
      a2Name = "";
      part1 = 0;
      part2 = 0;
      funQuery = 0; 
      resName = "";
      resPart = 0;
    }

    TaskElement( size_t _pos, size_t _dep, string _a1Name,string _a2Name,
               size_t  _part1,size_t  _part2,FText* _funQuery,
               string _resArrayName,size_t  _resPart){
      pos =_pos;
      dep = _dep;
      a1Name = _a1Name;
      a2Name = _a2Name;
      part1 = _part1;
      part2 = _part2;
      funQuery = _funQuery; 
      resName = _resArrayName;
      resPart = _resPart;
    }
    
    
    TaskElement(ListExpr list){
      if(!nl->HasLength(list,9)){
        return;
      }
      ListExpr e1 = nl->First(list);
      pos = nl->IntValue(e1);
      list = nl->Rest(list);

      ListExpr e2 = nl->First(list);
      dep = nl->IntValue(e2);
      list = nl->Rest(list);

      ListExpr e3 = nl->First(list);
      a1Name = nl->StringValue(e3);
      list = nl->Rest(list);

      ListExpr e4 = nl->First(list);
      a2Name = nl->StringValue(e4);
      list = nl->Rest(list);

      ListExpr e5 = nl->First(list);
      part1 = nl->IntValue(e5);
      list = nl->Rest(list);

      ListExpr e6 = nl->First(list);
      part2 = nl->IntValue(e6);
      list = nl->Rest(list);

      ListExpr e7 = nl->First(list);
      funQuery = new FText(true, nl->StringValue(e7));
      list = nl->Rest(list);

      ListExpr e8 = nl->First(list);
      resName = nl->StringValue(e8);
      list = nl->Rest(list);

      ListExpr e9 = nl->First(list);
      resPart = nl->IntValue(e9);
      list = nl->Rest(list);
    }

    
    ListExpr toListExpr(){
       ListExpr expr;
       expr = nl->OneElemList(nl->IntAtom(pos));
       expr = nl->Append(expr, nl->IntAtom(dep));
       expr = nl->Append(expr, nl->StringAtom(a1Name));
       expr = nl->Append(expr, nl->StringAtom(a2Name));
       expr = nl->Append(expr, nl->IntAtom(part1));
       expr = nl->Append(expr, nl->IntAtom(part2));
       expr = nl->Append(expr, nl->TextAtom(funQuery->GetValue()));
       expr = nl->Append(expr, nl->StringAtom(resName));
       expr = nl->Append(expr, nl->IntAtom(resPart));
       return expr;
    }


    ~TaskElement(){
       tt->DeleteIfAllowed();
       delete funQuery;
    }
    
    Tuple* next(){
      if(pos >= max){
        return 0;
      }
      Tuple* res = new Tuple(tt);
      res->PutAttribute(0, new CcInt(true,pos));
      res->PutAttribute(1, new CcInt(true,dep));
      res->PutAttribute(2, new CcString(true,a1Name));
      res->PutAttribute(3, new CcString(true,a2Name));
      res->PutAttribute(4, new CcInt(true,part1));
      res->PutAttribute(5, new CcInt(true,part2));
      res->PutAttribute(6, new FText(true,funQuery->GetValue()));
      res->PutAttribute(7, new CcString(true,resName));
      res->PutAttribute(8, new CcInt(true,resPart));

      pos++;
      return res;
    }

    bool readFrom(SmiRecord& valueRecord, size_t& offset){
        if(!readVar(pos,valueRecord,offset)){
            return false;
        }
        if(!readVar(dep,valueRecord,offset)){
           return false;
        }
        if(!readVar(a1Name,valueRecord,offset)){
           return false;
        }
        if(!readVar(a2Name,valueRecord,offset)){
           return false;
        }
        if(!readVar(part1,valueRecord,offset)){
           return false;
        }
        if(!readVar(part2,valueRecord,offset)){
           return false;
        }
        if(!readVar(funQuery,valueRecord,offset)){
           return false;
        }
        if(!readVar(resName,valueRecord,offset)){
           return false;
        }
        if(!readVar(resPart,valueRecord,offset)){
           return false;
        }
        return true;
     }


    bool saveTo(SmiRecord& valueRecord, size_t& offset){
        if(!writeVar(pos,valueRecord,offset)){
            return false;
        }
        if(!writeVar(dep,valueRecord,offset)){
           return false;
        }
        if(!writeVar(a1Name,valueRecord,offset)){
           return false;
        }
        if(!writeVar(a2Name,valueRecord,offset)){
           return false;
        }
        if(!writeVar(part1,valueRecord,offset)){
           return false;
        }
        if(!writeVar(part2,valueRecord,offset)){
           return false;
        }
        if(!writeVar(funQuery,valueRecord,offset)){
           return false;
        }
        if(!writeVar(resName,valueRecord,offset)){
           return false;
        }
        if(!writeVar(resPart,valueRecord,offset)){
           return false;
        }
         return true; 
     }
     
    void print(ostream& out)const{
      out << "( Task: " << pos << ", Dep : " << dep 
        << ", Arg1 : " << a1Name << ", Arg2 : " << a2Name
        << ", Part1 : " << part1 << ", Part2 : " << part2
        << ", Query : " << funQuery->GetValue() 
        << ", Res : " << resName << ", ResPart : " << resPart
        << ")";
    }

    size_t GetPos(){
      return pos;
    }
    
    size_t GetDep(){
      return dep;
    }
    
    string GetA1Name(){
      return a1Name;
    }
    
    string GetA2Name(){
      return a2Name;
    }
    
    size_t  GetPart1(){
      return part1;
    }
    
    size_t  GetPart2(){
      return part2;
    }
    
    FText* GetFunQuery(){
      return funQuery;
    }
    
    string GetResName(){
      return resName;
    }
    
    size_t GetResPart(){
      return resPart;
    }
     
     
 private:
    size_t  max;
    TupleType* tt;
    
    size_t pos;
    size_t dep;
    string a1Name;
    string a2Name;
    size_t  part1;
    size_t  part2;
    FText* funQuery;
    string resName;
    size_t  resPart;

};



/*
2.1.2 Class ~Code~

This class represents the Secondo type ~code~. It just stores the list of tasks.

*/

class Code{
  public:
     Code(const vector<TaskElement*>& _tasks): 
           tasks(_tasks), size(_tasks.size()) {
        if(size == 0){ // invalid
           defined = false;
           return;
        }
        defined = true;
     }

     explicit Code() {} // only for cast function


     Code& operator=(const Code& src) {
        this->tasks = src.tasks;
        this->size = src.size;
        this->defined = src.defined;
        return *this;
     }     
 
     ~Code() {
         size_t s = tasks.size();
         for(size_t i=0;i<s;i++){
           delete tasks[i];
         }
         tasks.clear();
     }

    void set(const vector<TaskElement*>& tasks){
        if(tasks.size() ==0){ // invalid
           makeUndefined(); 
           return;
        }
        defined = true;
        this->size = tasks.size();
        this->tasks = tasks;
     }

    void add(const vector<TaskElement*> tasks){
        if(tasks.size() ==0){ // invalid
           makeUndefined(); 
           return;
        }
        this->size += tasks.size();
        for(size_t i=0;i<tasks.size();i++){
          this->tasks.push_back(tasks[i]);
        }
     }

     bool IsDefined(){
        return defined;
     }

     static const string BasicType() { 
       return "code";
     }

     static const bool checkType(const ListExpr list){
         if(!nl->HasLength(list,3)){
            return false;
         }  
         if(!listutils::isSymbol(nl->First(list), BasicType())){
             return false;
         }
         return true;
     }

     size_t numOfTasks() const{
       return tasks.size();
     }
     
     size_t getSize() const{
        return size;
     }
 
     TaskElement* getTask(size_t i){
        if(i<0 || i>= tasks.size()){
           assert(false);
        }
        return tasks[i];
     } 

     ListExpr toListExpr(){
       if(!defined){
         return listutils::getUndefined();
       }

       ListExpr tl;
       if(tasks.empty()){
         tl =  nl->TheEmptyList();
       } else {
           tl = nl->OneElemList(tasks[0]->toListExpr());
           for(size_t i=0;i<tasks.size();i++){
             tl = nl->Append(tl, tasks[i]->toListExpr());
           }
       }
       
       return nl->ThreeElemList(nl->SymbolAtom( BasicType()), 
                                nl->IntAtom(size), 
                                tl); 
     }


     static Code* readFrom(ListExpr list){
        if(listutils::isSymbolUndefined(list)){
           vector<TaskElement*> tl;
           return new Code(tl);
        }
        if(!nl->HasLength(list,3)){
           return 0;
        }
        ListExpr Tasks = nl->Second(list);

        vector<TaskElement*> tl;
        while(!nl->IsEmpty(Tasks)){
           TaskElement* elem = new TaskElement(nl->First(Tasks));
           tl.push_back(elem);
           Tasks = nl->Rest(Tasks);
        }
        Code* result = new Code(tl);
        result->defined = true;
        return result;
     }
     
     static bool open(SmiRecord& valueRecord, size_t& offset, 
                      const ListExpr typeInfo, Word& result){
        bool defined;
        result.addr = 0;
        if(!readVar<bool>(defined,valueRecord,offset)){
           return false;
        } 
        if(!defined){
          vector<TaskElement*> tasks;
          result.addr = new Code(tasks);
          return true;
        }
        // object in smirecord is defined, read size
        size_t s;
        if(!readVar<size_t>(s,valueRecord,offset)){
           return false;
        }

        // read  vector
        vector<TaskElement*> tasks;
        Code* res= new Code(tasks);

         // append tasks
        for(size_t i=0; i< s; i++){
          size_t s = 0;
           TaskElement*  task = new TaskElement(s);
           if(!task->readFrom(valueRecord, offset)){
               delete res;
               return false;
           }
           res->tasks.push_back(task);
        }

        result.addr = res;
        return true;
     }

     static bool save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value) {

         Code* c = (Code*) value.addr;
         // defined flag
         if(!writeVar(c->defined,valueRecord,offset)){
           return false;
         }
         if(!c->defined){
            return true;
         }
         // size
         size_t s = c->size;
         if(!writeVar(s,valueRecord,offset)){
           return false;
         }
         // tasks
         for(size_t i=0;i<s;i++){
              if(!c->tasks[i]->saveTo(valueRecord,offset)){
                 return false;
              }
         }
         return true; 
     }


     void print(ostream& out){
       if(!defined){
          out << "undefined";
          return;
       }

       out << ", size : " << tasks.size()
           << " tasks : [" ;
       for(size_t i =0;i<tasks.size();i++){
          if(i>0) out << ", ";
          tasks[i]->print(out);
       }
       out << "]";
     }

     void makeUndefined(){
        tasks.clear();
        size = 0;
        defined = false;
     }
     
     size_t GetStartIndexOfLastResult(){
      size_t lastIndex = tasks.size() - 1;
      TaskElement* lastElement = tasks[lastIndex];
      
       for(size_t i =lastIndex - 1; i >= 0;i--){
          if((tasks[i]->GetResName() == lastElement->GetA1Name() &&
            tasks[i]->GetResPart() == lastElement->GetPart1()) ||
            (tasks[i]->GetResName() == lastElement->GetA2Name() &&
            tasks[i]->GetResPart() == lastElement->GetPart2())){
            return i + 1;
          }
       }
       return 0;
     }
     
     void BeginIteration(){
       iter = 0;
     }

     TaskElement* GetNext(){
       if(iter < tasks.size()){
         return tasks[iter++];
       }
       return 0;
     }

  private:
    vector<TaskElement*> tasks; // the Tasks information
    size_t  size; // corresponds with tasks size except tasks is empty
    bool defined; // defined state of this Code Element
    size_t  iter; // the actual index for an interatition
};


/*
2.1.2.1 Property function

*/
ListExpr CodeProperty(){
   return nl->TwoElemList(
            nl->FourElemList(
                 nl->StringAtom("Signature"),
                 nl->StringAtom("Example Type List"), 
                 nl->StringAtom("List Rep"),
                 nl->StringAtom("Example List")),
            nl->FourElemList(
                 nl->StringAtom(" -> SIMPLE"),
                 nl->StringAtom(" (code(darray <basictype>))"),
                 nl->TextAtom("(size ( t1 t2  ...)) where "
                     "t_i =(pos dep arg1 arg2 part1 part2 query res resPart)"),
                 nl->TextAtom("( 1(1 0 'R' 'S' 0 0 " 
                     "'.feed{r} ..feed{s} hashjoin[R_a, S_b] count' 'T' 0)")));
}

/*
2.1.2.2 IN function

*/

Word InCode(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct){

   Word res((void*)0);
   res.addr = Code::readFrom(instance);
   correct = res.addr!=0;
   return res;
}

/*

2.1.2.3 Out function

*/
ListExpr OutCode(ListExpr typeInfo, Word value){
   Code* c = (Code*) value.addr;
   return c->toListExpr();
}

/*

2.1.2.4 Create function

*/
Word CreateCode(const ListExpr typeInfo){

  Word w;
  vector<TaskElement*> t;
  w.addr = new Code(t);
  return w;
}

/*

2.1.2.4 Delete function

*/
void DeleteCode(const ListExpr typeInfo, Word& w){
  Code* c = (Code*) w.addr;
  delete c;
  w.addr = 0;
}

/*

2.1.2.4 Close function

*/
void CloseCode(const ListExpr typeInfo, Word& w){
  Code* c = (Code*) w.addr;
  delete c;
  w.addr = 0;
}

/*

2.1.2.4 Clone function

*/
Word CloneCode(const ListExpr typeInfo, const Word& w){
    Code* c = (Code*) w.addr;
    Word res;
    res.addr = new Code(*c);
    return res;
}

void* CastCode(void* addr){
   const vector<TaskElement*> ts;
   return (new (addr) Code(ts));   
}

bool CodeTypeCheck(ListExpr type, ListExpr& errorInfo){
    return Code::checkType(type);
}


int SizeOfCode(){
  return 42; // a magic number
}


TypeConstructor CodeTC(
  Code::BasicType(),
  CodeProperty,
  OutCode, InCode,
  0,0,
  CreateCode, DeleteCode,
  Code::open, Code::save,
  CloseCode, CloneCode,
  CastCode,
  SizeOfCode,
  CodeTypeCheck );


/*
1.15 Operator ~dloop3~

This operator transforms a function that will be performed 
over all entries of a DArray into a Code object.

1.15.1 Type Mapping

The signature is
darray(X) x string x (X->Y) -> code(darray(Y))
or
Code(darray(X)) x string x (X->Y) -> code(darray(Y))

*/
ListExpr dloop3TM(ListExpr args){
   string err ="darray(X) x string x (X x Y ->Z) or "
       "code(darray(X)) x string x (X x Y ->Z) expected";

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

  if(!DArray::checkType(arrayType)){
    return listutils::typeError(err + ": first arg not a darray");
  }
  if(!listutils::isMap<1>(funType)){
    return listutils::typeError(err + ": last arg is not a function");
  }
  if(!nl->Equal(nl->Second(arrayType), nl->Second(funType))){
    return listutils::typeError("type mismatch between darray and "
                                "function arg");
  }

  ListExpr result = nl->TwoElemList(listutils::basicSymbol<DArray>(),
                                    nl->Third(funType));

  if(!DArray::checkType(result)){
    return listutils::typeError("Invalid function result");
  }
   
   if(    DArray::checkType(nl->First(nl->First(args))) 
       && CcString::checkType(nl->First(nl->Second(args)))
       && listutils::isMap<1>(nl->First(nl->Third(args)))){
      return NList(Code::BasicType()).listExpr(); 
   }

   if(    Code::checkType(nl->First(nl->First(args))) 
       && CcString::checkType(nl->First(nl->Second(args)))
       && listutils::isMap<1>(nl->First(nl->Third(args)))){
      return NList(Code::BasicType()).listExpr(); 
   }
   cout << "wir sind hier!3" << endl;
   return listutils::typeError(err);
}

/*
1.15.2 Selection Function

*/
int loop3Select( ListExpr args )
{
  NList type(args);
  if(type.first().isSymbol(DArray::BasicType())){
    return 1;
  }
  return 0;
}

/*
1.15.2 Selection Function

*/
int dloop3VM_Array(Word* args, Word& result, int message,
           Word& local, Supplier s ){

   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   DArray* array = (DArray*) args[0].addr;
   result = qp->ResultStorage(s);
   Code* res = (Code*) result.addr;
   
   if(! array->IsDefined()){
      res->makeUndefined();
      return 0;
   }  

   string a2Name = 0;
   size_t  part2 = 0;
   string a1Name;
   FText* fun = 0;
   CcString* name = (CcString*) args[1].addr;
   if(name->IsDefined() && !(name->GetValue().length()==0)){
      a1Name = name->GetValue();
   }
   fun = (FText*) args[3].addr;

   if(!stringutils::isIdent(a1Name)){
      res->makeUndefined();
      return 0;
   }

   vector<TaskElement*> tasks;
   int pos = tasks.size();
   TaskElement* element;
   for(size_t i=0; i < array->getSize(); i++){
      pos++;
      element = new TaskElement( pos, 0,a1Name,a2Name,i+1,part2,fun,a1Name,i+1);
      tasks.push_back(element);
   }
   res->set(tasks);
   return 0; 
}

int dloop3VM_Code(Word* args, Word& result, int message,
           Word& local, Supplier s ){
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   Code* code = (Code*) args[0].addr;
   result = qp->ResultStorage(s);
   Code* res = (Code*) result.addr;
   
   if(!code->IsDefined()){
      res->makeUndefined();
      return 0;
   }  

   string a2Name = 0;
   size_t  part2 = 0;
   string a1Name;
   FText* fun = 0;
   CcString* name = (CcString*) args[1].addr;

//   if(!name->IsDefined() || (name->GetValue().length()==0)){
//      algInstance->getWorkerConnection(
//                          array->getWorker(0),dbname);
//      a1Name = algInstance->getTempName();
//   } else {
      a1Name = name->GetValue();
//   }
   fun = (FText*) args[3].addr;
   
   vector<TaskElement*> tasks;
   TaskElement* e;
   TaskElement* element;
   for(size_t i = 0; i < code->GetStartIndexOfLastResult(); i++){
     e = code->getTask(i);
     element = new TaskElement( e->GetPos(),e->GetDep(),e->GetA1Name(),
                          e->GetA2Name(),e->GetPart1(),e->GetPart2(),
                          e->GetFunQuery(),e->GetResName(),e->GetResPart());
     tasks.push_back(element);
   }

   size_t pos = code->getSize();
   for(size_t i = code->GetStartIndexOfLastResult(); i < code->getSize();i++){
     e = code->getTask(i);
     element = new TaskElement( pos, 0,e->GetA1Name(),a2Name,
                          i+1,part2,fun,e->GetA1Name(),i+1);
     tasks.push_back(element);
     pos++;
   }
   res->set(tasks);
   return 0; 
}

ValueMapping dloop3VM[] = {
  dloop3VM_Array,
  dloop3VM_Code
  
};

/*
1.15.3 Specification

*/
OperatorSpec dloop3Spec(
     " {darray(X), code(darray(X))} x string x  (X->Y) -> code(darray(y))",
     " _ dloop3[_,_]",
     "Generates a Code Object that performs a function "
     " on each element of a darray instance."
     "The string argument specifies the name of the result. If the name"
     " is undefined or an empty string, a name is generated automatically.",
     "query da2 dloop3[\"da3\", . toTasks"
     );

/*
1.15.4 Operator instance

*/
Operator dloop3Op(
  "dloop3",
  dloop3Spec.getStr(),
  2,
  dloop3VM,
  loop3Select,
  dloop3TM
);



/*
1.16 Operator totasks

The totasks operator exports all the tasks of a code object into a stream.

1.16.1 Type Mapping

This operator has a code object as arguments. The output is a stream of tasks.

*/
ListExpr  toTasksTM(ListExpr args){
  string err = "Code object  expected";

  if (nl->ListLength(args) != 1)
    return listutils::typeError(err);

  if(!Code::checkType(nl->First(nl->First(args)))){
    return listutils::typeError(err);    
  }

   //The mapping of the  stream(tuple)
   
  ListExpr attr1 = nl->TwoElemList( nl->SymbolAtom("Task"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  ListExpr attrList = nl->OneElemList(attr1);
  
  ListExpr attr2 = nl->TwoElemList( nl->SymbolAtom("Dep"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  attrList = nl->Append(attrList, attr2);
  
  ListExpr attr3 = nl->TwoElemList( nl->SymbolAtom("Arg1"),
                                      nl->SymbolAtom(CcString::BasicType()));
  attrList = nl->Append(attrList, attr3);
  
  ListExpr attr4 = nl->TwoElemList( nl->SymbolAtom("Arg2"),
                                      nl->SymbolAtom(CcString::BasicType()));
  attrList = nl->Append(attrList, attr4);
  
  ListExpr attr5 = nl->TwoElemList( nl->SymbolAtom("Part1"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  attrList = nl->Append(attrList, attr5);
  
  ListExpr attr6 = nl->TwoElemList( nl->SymbolAtom("Part2"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  attrList = nl->Append(attrList, attr6);
  
  ListExpr attr7 = nl->TwoElemList( nl->SymbolAtom("Query"),
                                    listutils::basicSymbol<FText>());
  attrList = nl->Append(attrList, attr7);
  
  ListExpr attr8 = nl->TwoElemList( nl->SymbolAtom("Res"),
                                      nl->SymbolAtom(CcString::BasicType()));
  attrList = nl->Append(attrList, attr8);
  
  ListExpr attr9 = nl->TwoElemList( nl->SymbolAtom("ResPart"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  attrList = nl->Append(attrList, attr9);

  return nl->TwoElemList(
              nl->SymbolAtom(Stream<Tuple>::BasicType()),
              nl->TwoElemList(
                  nl->SymbolAtom(Tuple::BasicType()),
                  attrList)); 
}


/*
1.16.2 Value Mapping

*/

int toTasksVM( Word* args, Word& result, int message,
                  Word& local, Supplier s ){
  Code* c = (Code*)local.addr;
  switch (message) {
    case OPEN: {
      c->BeginIteration();
      return 0;
    }
    case REQUEST: {
      result.addr = c->GetNext();
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      return 0;
    }
  }
  return 0;
}

/*
1.16.3 Specification

*/
OperatorSpec toTasksSpec(
     "Code(darray(X)) -> stream(tuble)",
     "_ toTasks",
     "The elements of a Code object are exported in a stream. ",
     "query c toTasks"
); 


/*
1.16.4 Operator instance


*/
Operator toTasksOp (
    "toTasks",                    //name
    toTasksSpec.getStr(),         //specification
    toTasksVM,                    //value mapping 
    Operator::SimpleSelect,        //trivial selection function
    toTasksTM                     //type mapping
);



/*
1.16 Operator schedule

The schedule operator reads all the tasks to be performed into an internal
data structure. It then distributes tasks to servers. When a server informs 
the scheduler that a task is finished, the scheduler selects another task 
and assigns it to this server.
The scheduler may also listen to heartbeat or progress messages of servers.
The list of tasks described by the incoming tuple stream may contain dependencies
so that a certain task can only be started when a specified other task has been 
finished. In this way the overlapping processing of successive operations 
can be controlled.

1.16.1 Type Mapping

This operator has 2 arguments. Input is a stream of tuples describing tasks
to be performed by Secondo servers. 
The second argument to schedule is the set of available Secondo servers.

*/
ListExpr  scheduleTM(ListExpr args){
  string lenErr = "2 parameters expected";
  string newErr = "stream(tuple) x stream(tuple) expected";
  string mapErr = "It's not a map ";

  if (nl->ListLength(args) != 2)
    return listutils::typeError(lenErr);

  ListExpr stream1 = nl->First(args);
  ListExpr stream2 = nl->Second(args);

  if (!listutils::isTupleStream(stream1))
    return listutils::typeError(newErr);

  if (!listutils::isTupleStream(stream2))
    return listutils::typeError(newErr);

//Stream  1:  
//The Tasklist, that should be scheduled  
//That could be the output of the dloop2schedule operator. 
  string att;
  int index;
  ListExpr type;
  ListExpr attrList = nl->Second(nl->Second(stream1));

  att = "Task";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type int");
  }  

  att = "Dep";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type int");
  }  

  att = "Arg1";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type string");
  }  

  att = "Arg2";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type string");
  }  

  att = "Part1";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type int");
  }  

  att = "Part2";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type int");
  }  

  att = "Query";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type string");
  }  

  att = "Res";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type string");
  }  

  att = "ResPart";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type int");
  }  
  
//Stream2:
//The set of avaiable Secondo server.
//That could be the output of the checkConnection operator. 

  attrList = nl->Second(nl->Second(stream2));
  att = "Id";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the second streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type int");
  }  

  att = "Host";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the second streem not found");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type string");
  }  

  att = "Port";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the second streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type string");
  }  

  att = "ConfigFile";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the second streem not found");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type string");
  }  

  att = "OK";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the second streem not found");
  }
  if(!CcBool::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type bool");
  }  

  att = "PID";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the second streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type int");
  }  
  
  //returns 0 ?
  return nl->SymbolAtom(CcInt::BasicType());
}


/*
1.16.2 Value Mapping

*/



int scheduleVM( Word* args, Word& result, int message,
                  Word& local, Supplier s ){

  qp->Open(args[0].addr); // open the argument stream

  //Straem 1
  Stream<Tuple> streamTasks(args[0]);
  streamTasks.open();

  Tuple* nextTask = streamTasks.request();
  int countTasks =0;
  while(nextTask!=0){
    countTasks++;
    nextTask->DeleteIfAllowed();
    nextTask = streamTasks.request();
  }
  streamTasks.close();

  Stream<Tuple> streamServer(args[0]);
  streamServer.open();

  //Stream 2
  Tuple* nextServer = streamServer.request();
  int countServer =0;
  while(nextServer!=0){
    countServer++;
    nextServer->DeleteIfAllowed();
    nextServer = streamServer.request();
  }
  streamServer.close();
  return 0;
}

/*
1.16.3 Specification

*/
OperatorSpec scheduleSpec(
     "stream(tuble) x stream(tuble) -> darray(Z)",
     "_ _ schedule",
     "The first stream contains the tasks, the second the Server. "
     "The name of the resulting darray is specifyed with the tasks.", 
     "query st1 st2 schedule "
); 


/*
1.16.4 Operator instance


*/
Operator scheduleOp (
    "schedule",                    //name
    scheduleSpec.getStr(),         //specification
    scheduleVM,                    //value mapping 
    Operator::SimpleSelect,        //trivial selection function
    scheduleTM                     //type mapping
);



/*
3 Implementation of the Algebra

*/
Distributed3Algebra::Distributed3Algebra(){

   AddTypeConstructor(&CodeTC);
   CodeTC.AssociateKind(Kind::SIMPLE());
   

   AddOperator(&dloop3Op);
   AddOperator(&toTasksOp);
   //AddOperator(&scheduleOp);

}


Distributed3Algebra::~Distributed3Algebra(){
}


} // end of namespace distributed3

extern "C"
Algebra*
   InitializeDistributed3Algebra( NestedList* nlRef,
                             QueryProcessor* qpRef,
                             AlgebraManager* amRef ) {

   distributed3::algInstance = new distributed3::Distributed3Algebra();
   //distributed3::showCommands = false;   
   return distributed3::algInstance;
}


