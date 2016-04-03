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


#include "ConnectionInfo.h"


  // use boost for thread handling

#include <boost/thread.hpp>
#include <boost/date_time.hpp>



using namespace std;

namespace distributed3 {


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

boost::mutex createRelMut;
boost::mutex copylistmutex;
bool showCommands;
boost::mutex showCommandMtx;
boost::mutex nlparsemtx;

void showCommand(SecondoInterfaceCS* src, const string& host, const int port, 
                 const string& cmd, bool start){

   if(showCommands){
      dwriter.write(showCommands,cout, src, src->getPid(), "= " + host + ":" 
              + stringutils::int2str(port)+ ":" + (start?"start ":"finish ") 
              + cmd);
   }

}


ConnectionInfo::ConnectionInfo(const string& _host, const int _port,
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

ConnectionInfo::~ConnectionInfo(){
  {
      boost::lock_guard<boost::recursive_mutex> guard(simtx);
      si->Terminate();
  }
  delete si;
  si = 0;
  delete mynl;
}

string ConnectionInfo::getHost() const{
  return host;
}

int ConnectionInfo::getPort() const{
  return port;
}
string ConnectionInfo::getConfig() const{
  return config;
}

bool ConnectionInfo::check() {
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

void ConnectionInfo::setId(const int i){
    if(si){
        si->setId(i);;
    }
}


void ConnectionInfo::simpleCommand(string command1, int& err, string& result, 
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

string ConnectionInfo::getSecondoHome(){
  if(secondoHome.length()==0){
    retrieveSecondoHome();
  }
  return secondoHome;
}



bool ConnectionInfo::cleanUp() {
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



bool ConnectionInfo::switchDatabase(
  const string& dbname, bool createifnotexists){
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


void ConnectionInfo::simpleCommand(
  const string& command1, int& error, string& errMsg, 
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

void ConnectionInfo::simpleCommandFromList(const string& command1, int& error, 
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


void ConnectionInfo::simpleCommand(const string& command1, int& error, 
                string& errMsg,ListExpr& resList, const bool rewrite,
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

int ConnectionInfo::serverPid(){
  boost::lock_guard<boost::recursive_mutex> guard(simtx);
  if(serverPID==0 && si != 0){
    serverPID = si->getPid(); 
  }
  return serverPID;
}


int ConnectionInfo::sendFile( const string& local, const string& remote, 
            const bool allowOverwrite){
boost::lock_guard<boost::recursive_mutex> guard(simtx);   
int res =  si->sendFile(local,remote, allowOverwrite);
return res;
}


int ConnectionInfo::requestFile( const string& remote, const string& local,
                const bool allowOverwrite){
boost::lock_guard<boost::recursive_mutex> guard(simtx);   
int res =  si->requestFile(remote, local, allowOverwrite);
return res;
}

string ConnectionInfo::getRequestFolder(){
if(requestFolder.length()==0){
    boost::lock_guard<boost::recursive_mutex> guard(simtx);   
    requestFolder =  si->getRequestFileFolder();
}
return requestFolder;
}

string ConnectionInfo::getSendFolder(){
if(sendFolder.length()==0){
    boost::lock_guard<boost::recursive_mutex> guard(simtx);   
    sendFolder =  si->getSendFileFolder();
}
return sendFolder;
}

string ConnectionInfo::getSendPath(){
if(sendPath.length()==0){
    boost::lock_guard<boost::recursive_mutex> guard(simtx);   
    sendPath =  si->getSendFilePath();
}
return sendPath;
}

ConnectionInfo* ConnectionInfo::createConnection(const string& host, 
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

bool ConnectionInfo::createOrUpdateObject(const string& name, 
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


bool ConnectionInfo::createOrUpdateRelation(const string& name, 
                     ListExpr typeList,Word& value){

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

bool ConnectionInfo::createOrUpdateRelationFromBinFile(const string& name, 
                                      const string& filename,
                                      const bool allowOverwrite){
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


bool ConnectionInfo::createOrUpdateAttributeFromBinFile(const string& name, 
                                      const string& filename,
                                      const bool allowOverwrite){
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

bool ConnectionInfo::saveRelationToFile(ListExpr relType, Word& value, 
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

bool ConnectionInfo::saveAttributeToFile(ListExpr type, Word& value, 
                        const string& filename){
  Attribute* attr = (Attribute*) value.addr;
  return FileAttribute::saveAttribute(type, attr, filename);
}

bool ConnectionInfo::storeObjectToFile( const string& objName, Word& value, 
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


bool ConnectionInfo::retrieve(const string& objName, 
              ListExpr& resType, Word& result,bool checkType){
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

bool ConnectionInfo::retrieveRelation(const string& objName, 
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

bool ConnectionInfo::retrieveRelationInFile(const string& fileName,
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



bool ConnectionInfo::retrieveRelationFile(const string& objName,
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


bool ConnectionInfo::retrieveAnyFile(const string& remoteName,
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


Word ConnectionInfo::createRelationFromFile(
            const string& fname, ListExpr& resType){
  
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

}
