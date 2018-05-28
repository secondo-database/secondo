/*
----
This file is part of SECONDO.

Copyright (C) 2018,
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

#include "Tools/DFS/dfs/remotedfs.h"
#include "Tools/DFS/dfs/dfs.h"
#include "QueryProcessor.h"
#include "NestedList.h"
#include "StandardTypes.h"
#include "ListUtils.h"

#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Standard-C++/LongInt.h"


#include <string>




using namespace std;


extern QueryProcessor* qp;
extern NestedList* nl;

namespace dfsalg{


#define REMOTE_FILESYSTEM

// pointer to the currently used file system
// may be this will be changed later to an uri
#ifdef REMOTE_FILESYSTEM
   static dfs::remote::RemoteFilesystem* filesystem = 0;
#else
   static dfs::Filesystem* filesystem = 0;
#endif 

static dfs::log::Logger* logger = new dfs::log::DefaultOutLogger();


/*
2 Operators

*/

/*
2.1 ~connectDFS~

*/
ListExpr connectDFSTM(ListExpr args){
   if(!nl->HasLength(args,2)){
     return listutils::typeError("two arguments expected");
   }
   ListExpr host = nl->First(args);
   if(!CcString::checkType(host) && !FText::checkType(host)){
     return listutils::typeError("first argument must be of type "
                                 "string or text");
   }
   if(!CcInt::checkType(nl->Second(args))){
     return listutils::typeError("second argument must be of type int");
   }
   return listutils::basicSymbol<CcBool>();
}

template<class T>
int connectDFSVMT( Word* args,
                  Word& result, int message,
                  Word& local,
                  Supplier  s ){

   // prepare result
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr; 

   // delete existing remote filesystem
   if(filesystem) {
      delete filesystem;
      filesystem = 0;   
   }

   // check arguments
   T* host = (T*) args[0].addr;
   CcInt* port = (CcInt*) args[1].addr;

   if(!host->IsDefined() || !port->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   stringstream ss;
   ss << "dfs-index://"<<host->GetValue()<<":"<<port->GetValue();
   URI uri = URI::fromString(ss.str().c_str());
   filesystem = new dfs::remote::RemoteFilesystem(uri,logger);
   // check whether connection is successful
   bool success = false;
   try{
      // if there is no connection, some exception will 
      // be thrown by the next command
      filesystem->countFiles();
      success = true;
   } catch(...){
      success = false;
   }
   res->Set(true,success);
   return 0;
}

ValueMapping connectDFSVM[] = {
   connectDFSVMT<CcString>,
   connectDFSVMT<FText>
};

int connectDFSSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec connectDFSSpec(
   "string x int -> bool",
   "connectDFS(_)",
   "Connects tto a distributed file system. Arguments are "
   " hostname and port of the index node.",
   " query connectDFS('localhost', 4444)"
);

Operator connectDFSOp(
   "connectDFS",
   connectDFSSpec.getStr(),
   2,
   connectDFSVM,
   connectDFSSelect,
   connectDFSTM
);

/*
2.1 disconnectDFS

*/
ListExpr disconnectDFSTM(ListExpr args){
  if(!nl->IsEmpty(args)){
    return listutils::typeError("no arguments expected");
  }
  return listutils::basicSymbol<CcBool>();
}


int disconnectDFSVM( Word* args,
                  Word& result, int message,
                  Word& local,
                  Supplier  s ){

  result = qp->ResultStorage(s);
  ((CcBool*) result.addr)->Set(true, filesystem!=0);
  if(filesystem){
    delete filesystem;
    filesystem = 0;
  }
  return 0;
}

OperatorSpec disconnectDFSSpec(
  "-> bool",
  "disconnectDFS()",
  "Disconnects a distributed filesystem. Returns true "
  " if there was a distributed filesystem before",
  "query disconnectDFS() "
);

Operator disconnectDFSOp(
  "disconnectDFS",
   disconnectDFSSpec.getStr(),
   disconnectDFSVM,
   Operator::SimpleSelect,
   disconnectDFSTM
);


/*
2.2 ~deleteDFSFile~

*/
ListExpr deleteDFSFileTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  ListExpr arg1 = nl->First(args);
  if(!CcString::checkType(arg1)&&!FText::checkType(arg1)){
    return listutils::typeError("string or text required");
  }
  return listutils::basicSymbol<CcBool>();
}

template<class T>
int deleteDFSFileVMT( Word* args,
                      Word& result, int message,
                      Word& local,
                      Supplier  s ){
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   T* id = (T*) args[0].addr;
   if(!id->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   if(!filesystem){
      res->Set(true,false);
      return 0;
   }
   try{
      string name = id->GetValue();
      if(!filesystem->hasFile(name.c_str())){
        res->Set(true,false);
        return 0;
      }
      filesystem->deleteFile(name.c_str());
      res->Set(true,true);
   } catch(...){
      res->Set(true,false);
   }
   return 0;
}

ValueMapping deleteDFSFileVM[] = {
   deleteDFSFileVMT<CcString>,
   deleteDFSFileVMT<FText>
};

int deleteDFSFileSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec deleteDFSFileSpec(
   "{string,text} -> bool",
   "deleteDFSFile(_)",
   "removes a file from connected dfs and returns success.",
   "query deleteDFSFile('myfile')"
);

Operator deleteDFSFileOp(
   "deleteDFSFile",
   deleteDFSFileSpec.getStr(),
   2,
   deleteDFSFileVM,
   deleteDFSFileSelect,
   deleteDFSFileTM
);

/*
2.3 Operator ~storeTextAsDFSFile~

*/
ListExpr storeTextAsDFSFileTM(ListExpr args){
   // text x {string,text} [ x string ] -> bool
   if(!nl->HasLength(args,2) && !nl->HasLength(args,3)){
     return listutils::typeError("2 or 3 arguments required");
   }
   if(!FText::checkType(nl->First(args))){
     return listutils::typeError("fisrt arg is not a text");
   }
   ListExpr arg2 = nl->Second(args);
   if(!CcString::checkType(arg2) && !FText::checkType(arg2)){
     return listutils::typeError("second arg must be a string or a text");
   } 
   if(nl->HasLength(args,3)){
      if(!CcString::checkType(nl->Third(args))){
         return listutils::typeError("third arg must be a string");
      }
   }
   return listutils::basicSymbol<CcBool>();
}



template<class T>
int storeTextAsDFSFileVMT( Word* args,
                      Word& result, int message,
                      Word& local,
                      Supplier  s ){
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  FText* content = (FText*) args[0].addr;
  T* name = (T*) args[1].addr;
  if(!content->IsDefined() || !name->IsDefined()){
    res->SetDefined(false);
    return  0;
  }
  string contentstr = content->GetValue();
  string namestr = name->GetValue();
  string catstr;
  if(qp->GetNoSons(s)==3){
     CcString* cat = (CcString*) args[2].addr;
     if(!cat->IsDefined()) {
        res->SetDefined(false);
        return 0;
     }
     catstr = cat->GetValue();
  }
  if(!filesystem){
    res->Set(true,false);
    return 0;
  }
  try{
    if(catstr.length()>0){
       filesystem->storeFile(namestr.c_str(),contentstr.c_str(), 
                             contentstr.length(), catstr.c_str());
    } else {
       filesystem->storeFile(namestr.c_str(),contentstr.c_str(), 
                             contentstr.length());
    }
    res->Set(true,true);
  } catch(...){
    res->Set(true,false);
  }
  return 0;
}

ValueMapping storeTextAsDFSFileVM[] = {
   storeTextAsDFSFileVMT<CcString>,
   storeTextAsDFSFileVMT<FText>
};

int storeTextAsDFSFileSelect(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}

OperatorSpec storeTextAsDFSFileSpec(
  "text x {string,text} [x string] -> bool",
  " storeTextAsDFSFile(content, filename [,category]) ",
  "Stores the content of a text into the currently "
  " connected dfs. ",
  "query storeTextAsDFSFile('My content', 'myfirstfile')"
);

Operator storeTextAsDFSFileOp(
  "storeTextAsDFSFile",
  storeTextAsDFSFileSpec.getStr(),
  2,
  storeTextAsDFSFileVM,
  storeTextAsDFSFileSelect,
  storeTextAsDFSFileTM
);


/*
2.4 Operator ~storeLocalFileToDFS~

*/
ListExpr storeLocalFileToDFSTM(ListExpr args){
  if(!nl->HasLength(args,2) && !nl->HasLength(args,3)){
    return listutils::typeError("2 or 3 arguments expected");
  }
  ListExpr arg = nl->First(args);
  if(!CcString::checkType(arg) && !FText::checkType(arg)){
    return listutils::typeError("string or text as 1st arg expected");
  }
  arg = nl->Second(args);
  if(!CcString::checkType(arg) && !FText::checkType(arg)){
    return listutils::typeError("string or text as 2nd arg expected");
  }
  if(nl->HasLength(args,3)){
    if(!CcString::checkType(nl->Third(args))){
      return listutils::typeError("3rd arg not of type string");
    }
  }
  return listutils::basicSymbol<CcBool>();
}


template<class L, class R>
int storeLocalFileToDFSVMT( Word* args,
                      Word& result, int message,
                      Word& local,
                      Supplier  s ){
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  L* localName = (L*) args[0].addr;
  R* remoteName = (R*) args[1].addr;
  if(!localName->IsDefined() || !remoteName->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  string cat;
  if(qp->GetNoSons(s)==3){
    CcString* Cat = (CcString*) args[2].addr;
    if(!Cat->IsDefined()){
      res->SetDefined(false);
      return 0;
    }
    cat = Cat->GetValue();
  }
  string localN = localName->GetValue();
  string remoteN = remoteName->GetValue();
  if(!filesystem){ // no dfs connected
    res->Set(true,false);
    return 0;
  }

  // arguments are checked
  // the file file be transmitted in several parts
  ifstream in(localN.c_str(), ios::binary);
  if(!in){ // local file could ot be opened
    res->Set(true,false);
    return 0;
  }
  in.close();
  try{
     if(cat.length()>0){
       filesystem->storeFileFromLocal(remoteN.c_str(),
                                      localN.c_str(),
                                      cat.c_str());
     } else {
       filesystem->storeFileFromLocal(remoteN.c_str(), localN.c_str());
     }
     res->Set(true,true);
  } catch(...){
     res->Set(true,false);
  }
  return 0; 
}

ValueMapping storeLocalFileToDFSVM[] = {
  storeLocalFileToDFSVMT<CcString, CcString>,
  storeLocalFileToDFSVMT<CcString, FText>,
  storeLocalFileToDFSVMT<FText, CcString>,
  storeLocalFileToDFSVMT<FText, FText>
};

int storeLocalFileToDFSSelect(ListExpr args){
   int n1 = CcString::checkType(nl->First(args))?0:2;
   int n2 = CcString::checkType(nl->Second(args))?0:1;
   return n1+n2;
}

OperatorSpec storeLocalFileToDFSSpec(
   "{string,text} x {string, text} [ x string] -> bool",
   "storeLocalFileToDFS( localName, remoteName [,category]",
   "Brings a local file into the dfs. ",
   " query storeLocalFileToDFS('berlintest', 'berlintest')"
);

Operator storeLocalFileToDFSOp(
  "storeLocalFileToDFS",
   storeLocalFileToDFSSpec.getStr(),
   4,
   storeLocalFileToDFSVM,
   storeLocalFileToDFSSelect,
   storeLocalFileToDFSTM
);

/*
2.5 Operator ~storeDFSFile~

*/
ListExpr  storeDFSFileTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("2 args expected");
  }
  ListExpr arg = nl->First(args);
  if(!CcString::checkType(arg) && !FText::checkType(arg)){
    return listutils::typeError("1st arg not of type string or text");
  }
  arg = nl->Second(args);
  if(!CcString::checkType(arg) && !FText::checkType(arg)){
    return listutils::typeError("2nd arg not of type string or text");
  }
  return listutils::basicSymbol<CcBool>();
}


template<class R, class L, bool partially>
int storeDFSFileVMT( Word* args,
                     Word& result, int message,
                     Word& local,
                     Supplier  s ){
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  R* remoteName = (R*) args[0].addr;
  L* localName = (L*) args[1].addr;
  if(!remoteName->IsDefined() || !localName->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  if(!filesystem){
    res->Set(true,false);
    return 0;
  }
  string remoteN = remoteName->GetValue();
  string localN = localName->GetValue();
  
  char* buffer = 0;
  try{
     if(!filesystem->hasFile(remoteN.c_str())){
        res->Set(true,false);
        return 0;
     }
     if(!partially){
        filesystem->receiveFileToLocal(remoteN.c_str(), localN.c_str());
        res->Set(true,true);
     } else {
        size_t buffersize = 1024*1024; 
        buffer = new char[buffersize];
        size_t toRead = filesystem->fileSize(remoteN.c_str());
        ofstream out(localN.c_str(), ios::binary| ios::trunc);
        if(!out){
          res->Set(true,false);
          return 0;
        }
        size_t pos = 0;
        while(toRead>0){
           size_t r = buffersize<toRead?buffersize:toRead;
           filesystem->receiveFilePartially(remoteN.c_str(),pos, r, buffer);
           out.write(buffer,r);
           toRead -= r;
           pos += r;
        }
        out.close();
        res->Set(true,true);
     }
  } catch(...){
     res->Set(true,false);
  }
  if(buffer){
    delete[] buffer;
  }
  return 0;
}

ValueMapping storeDFSFileVM[] = {
  storeDFSFileVMT<CcString,CcString,false>,
  storeDFSFileVMT<CcString,FText,false>,
  storeDFSFileVMT<FText,CcString,false>,
  storeDFSFileVMT<FText,FText,false>
};


ValueMapping storeDFSFile2VM[] = {
  storeDFSFileVMT<CcString,CcString,true>,
  storeDFSFileVMT<CcString,FText,true>,
  storeDFSFileVMT<FText,CcString,true>,
  storeDFSFileVMT<FText,FText,true>
};



int storeDFSFileSelect(ListExpr args){
   int n1 = CcString::checkType(nl->First(args))?0:2;
   int n2 = CcString::checkType(nl->Second(args))?0:1;
   return n1+n2;
}

OperatorSpec storeDFSFileSpec(
  "{string,text} x {string,text} -> bool",
  "storeDFSFile(remoteName, localName)",
  "Copies a file from a connected dfs to the local file system.",
  "query storeDFSFile('myRemoteFile', 'myLocalFile')"
);

OperatorSpec storeDFSFile2Spec(
  "{string,text} x {string,text} -> bool",
  "storeDFSFile2(remoteName, localName)",
  "Copies a file from a connected dfs to the local file system.",
  "query storeDFSFile2('myRemoteFile', 'myLocalFile')"
);

Operator storeDFSFileOp(
  "storeDFSFile",
  storeDFSFileSpec.getStr(),
  4,
  storeDFSFileVM,
  storeDFSFileSelect,
  storeDFSFileTM
);

Operator storeDFSFile2Op(
  "storeDFSFile2",
  storeDFSFile2Spec.getStr(),
  4,
  storeDFSFile2VM,
  storeDFSFileSelect,
  storeDFSFileTM
);

/*
2.6 Operator ~isDFSFile~

*/
ListExpr isDFSFileTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  ListExpr arg = nl->First(args);
  if(!CcString::checkType(arg) && !FText::checkType(arg)){
    return listutils::typeError("string or text expected");
  }
  return listutils::basicSymbol<CcBool>();
}


template<class R>
int isDFSFileVMT( Word* args,
                      Word& result, int message,
                      Word& local,
                      Supplier  s ){
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   R* Id = (R*) args[0].addr;
   if(!Id->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   string id = Id->GetValue();
   if(!filesystem){
     res->Set(true,false);
     return 0;
   }
   try{
     res->Set(true,filesystem->hasFile(id.c_str()));
   } catch(...){
     res->Set(true,false);
   }
   return 0;
}

ValueMapping isDFSFileVM[] = {
  isDFSFileVMT<CcString>,
  isDFSFileVMT<FText>
};

int isDFSFileSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec isDFSFileSpec(
  "{string,text} -> bool",
  "isDFSFile(name)",
  "Check whether the file is part of the connected dfs",
  "query isDFSFile('myRemoteFile')"
);

Operator isDFSFileOp(
  "isDFSFile",
  isDFSFileSpec.getStr(),
  2,
  isDFSFileVM,
  isDFSFileSelect,
  isDFSFileTM
);

/*
2.7 Operator ~nextDFSWritePosition~

*/
ListExpr nextDFSWritePositionTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  ListExpr arg = nl->First(args);
  if(!CcString::checkType(arg) && !FText::checkType(arg)){
    return listutils::typeError("string or text expected");
  }
  return listutils::basicSymbol<LongInt>();
}


template<class R>
int nextDFSWritePositionVMT( Word* args,
                      Word& result, int message,
                      Word& local,
                      Supplier  s ){
   result = qp->ResultStorage(s);
   LongInt* res = (LongInt*) result.addr;
   R* Id = (R*) args[0].addr;
   if(!Id->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   string id = Id->GetValue();
   if(!filesystem){
     res->Set(true,-1);
     return 0;
   }
   try{
     res->Set(true,filesystem->nextWritePosition(id.c_str()));
   } catch(...){
     res->Set(true,-1);
   }
   return 0;
}

ValueMapping nextDFSWritePositionVM[] = {
  nextDFSWritePositionVMT<CcString>,
  nextDFSWritePositionVMT<FText>
};

int nextDFSWritePositionSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec nextDFSWritePositionSpec(
  "{string,text} -> bool",
  "nextDFSWritePosition(name)",
  "Returns the append position of a dfs file (its length).",
  "query nextDFSWritePosition('myRemoteFile')"
);

Operator nextDFSWritePositionOp(
  "nextDFSWritePosition",
  nextDFSWritePositionSpec.getStr(),
  2,
  nextDFSWritePositionVM,
  nextDFSWritePositionSelect,
  nextDFSWritePositionTM
);

/*
2.8 Operator ~getDFSFileAsText~

*/
ListExpr getDFSFileAsTextTM(ListExpr args){
   if(!nl->HasLength(args,1) && !nl->HasLength(args,3)){
     return listutils::typeError("one or three args expected");
   }
   ListExpr arg1 = nl->First(args);
   if(!CcString::checkType(arg1) && !FText::checkType(arg1)){
     return listutils::typeError("first arg must be of type string or text");
   }
   if(nl->HasLength(args,3)){
     if(!CcInt::checkType(nl->Second(args)) || 
        !CcInt::checkType(nl->Third(args))){
       return listutils::typeError("2nd and 3rd arg must be of type int");
     }
   }
   return listutils::basicSymbol<FText>();
}

template<class R>
int getDFSFileAsTextVMT( Word* args,
                         Word& result, int message,
                         Word& local,
                         Supplier  s ){
   result=qp->ResultStorage(s);
   FText* res = (FText*) result.addr;
   R* Fname = (R*) args[0].addr;
   if(!Fname->IsDefined() || filesystem==0){
     res->SetDefined(false);
     return 0;
   }
   string fname = Fname->GetValue();
   if(!filesystem->hasFile(fname.c_str())){
     res->SetDefined(false);
     return 0;
   }
   int min=0;
   int max=filesystem->fileSize(fname.c_str());
   if(qp->GetNoSons(s)==3){
     CcInt* Min = (CcInt*) args[1].addr;
     CcInt* Max = (CcInt*) args[2].addr;
     if(!Min->IsDefined() || !Max->IsDefined()){
       res->SetDefined(false);
       return 0;    
     }
     min = Min->GetValue();
     int m = Max->GetValue();
     if(m<max){
        max = m;
     }
     if(min<0 || min > max){
       res->SetDefined(false);
       return 0; 
     }
   }
   size_t bufferlength = max-min;
   char* buffer = new char[bufferlength];
   try{
      filesystem->receiveFilePartially(fname.c_str(),
                                       min, bufferlength,buffer);
      string bufferstr(buffer, bufferlength);
      res->Set(true,bufferstr);
   } catch(...){
      res->SetDefined(false);
   }
   delete[] buffer;
   return 0;
}

ValueMapping getDFSFileAsTextVM[] = {
   getDFSFileAsTextVMT<CcString>,
   getDFSFileAsTextVMT<FText>
};

int getDFSFileAsTextSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec getDFSFileAsTextSpec(
  "{string,text} [x int x int] -> text",
  "getDFSFileAsText(filename, min, max) ",
  "Receives a portion of a file from connected remote "
  "file system. If the min and max is missing, the whole "
  " file is taken.",
  "query getDFSFileAsText('myRemoteFile')"
);

Operator getDFSFileAsTextOp(
  "getDFSFileAsText",
  getDFSFileAsTextSpec.getStr(),
  2,
  getDFSFileAsTextVM,
  getDFSFileAsTextSelect,
  getDFSFileAsTextTM
);


/*
2.9 Operator ~deleteAllDFSFiles~

*/
ListExpr deleteAllDFSFilesTM(ListExpr args){
   if(!nl->IsEmpty(args)){
      return listutils::typeError("don't bother me with arguments");
   }
   return listutils::basicSymbol<CcBool>();
}

int deleteAllDFSFilesVM( Word* args,
                         Word& result, int message,
                         Word& local,
                         Supplier  s ){
   result=qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   if(!filesystem){
      res->SetDefined(false);
      return 0;
   } 
   try{
       filesystem->deleteAllFiles();
       res->Set(true,true); 
   } catch(...){
      res->Set(true,false);
   }
   return 0;
}

OperatorSpec deleteAllDFSFilesSpec(
  "-> bool",
  "deleteAllDFSFiles()",
  "Removes all files from a connected dfs",
  "query deleteAllDFSFiles() "
);

Operator deleteAllDFSFilesOp(
  "deleteAllDFSFiles",
   deleteAllDFSFilesSpec.getStr(),
   deleteAllDFSFilesVM,
   Operator::SimpleSelect,
   deleteAllDFSFilesTM
);


/*
2.10 Operator ~deleteAllDFSFilesWithCat~

*/
ListExpr deleteAllDFSFilesWithCatTM(ListExpr args){
   if(!nl->HasLength(args,1)){
     return listutils::typeError("one arg expected");
   }
   if(!CcString::checkType(nl->First(args))){
     return listutils::typeError("string expected");
   }
   return listutils::basicSymbol<CcBool>();
}


int deleteAllDFSFilesWithCatVM( Word* args,
                         Word& result, int message,
                         Word& local,
                         Supplier  s ){
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   CcString* Cat = (CcString*) args[0].addr;
   if(!Cat->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   string cat = Cat->GetValue();
   if(!filesystem){
     res->Set(true,false);
     return 0;
   }
   try{
     filesystem->deleteAllFilesOfCategory(cat.c_str());
     res->Set(true,true);
   } catch(...){
     res->Set(true,false);
   }
   return 0;
}

OperatorSpec deleteAllDFSFilesWithCatSpec(
   "string -> bool",
   "deleteAllDFSFilesWithCat(category)",
   "Removes all files belonging to a certain category "
   " from the connected dfs",
   " query deleteAllDFSFilesWithCat(\"deprecated\")"
);

Operator deleteAllDFSFilesWithCatOp(
   "deleteAllDFSFilesWithCat",
   deleteAllDFSFilesWithCatSpec.getStr(),
   deleteAllDFSFilesWithCatVM,
   Operator::SimpleSelect,
   deleteAllDFSFilesWithCatTM
);

/*
2.11 Operator ~countDFSFiles~

*/
ListExpr countDFSFilesTM(ListExpr args){
  if(!nl->IsEmpty(args)){
     return listutils::typeError("what should i do with an argument?");
  }
  return listutils::basicSymbol<CcInt>();
}


int countDFSFilesVM( Word* args,
                         Word& result, int message,
                         Word& local,
                         Supplier  s ){
   result = qp->ResultStorage(s);
   CcInt* res = (CcInt*) result.addr;
   if(!filesystem){
      res->SetDefined(false);
      return 0;
   }
   try{
      res->Set(true, filesystem->countFiles());
   } catch(...){
      res->SetDefined(false);
   }
   return 0;
}

OperatorSpec countDFSFilesSpec(
  "-> int",
  "countDFSFiles()",
  "Returns the number of files in the connected dfs.",
  " query countDFSFiles()"
);

Operator countDFSFilesOp(
  "countDFSFiles",
  countDFSFilesSpec.getStr(),
  countDFSFilesVM,
  Operator::SimpleSelect,
  countDFSFilesTM
);

/*
2.12 Operator ~totalDFSSize~

*/
ListExpr totalDFSSizeTM(ListExpr args){
  if(!nl->IsEmpty(args)){
     return listutils::typeError("pfui, an argument. put it away!");
  }
  return listutils::basicSymbol<LongInt>();
}


int totalDFSSizeVM( Word* args,
                         Word& result, int message,
                         Word& local,
                         Supplier  s ){
   result = qp->ResultStorage(s);
   LongInt* res = (LongInt*) result.addr;
   if(!filesystem){
      res->SetDefined(false);
      return 0;
   }
   try{
      res->Set(true, filesystem->totalSize());
   } catch(...){
      res->SetDefined(false);
   }
   return 0;
}

OperatorSpec totalDFSSizeSpec(
  "-> int",
  "totalDFSSize()",
  "Returns the amount of hdd space aquired by the dfs.",
  "query totalDFSSize()"
);

Operator totalDFSSizeOp(
  "totalDFSSize",
  totalDFSSizeSpec.getStr(),
  totalDFSSizeVM,
  Operator::SimpleSelect,
  totalDFSSizeTM
);


/*
2.13 Operator ~dfsFileSize~

*/
ListExpr dfsFileSizeTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  ListExpr arg = nl->First(args);
  if(!CcString::checkType(arg) && !FText::checkType(arg)){
    return listutils::typeError("string or text expected");
  }
  return listutils::basicSymbol<LongInt>();
}


template<class R>
int dfsFileSizeVMT( Word* args,
                      Word& result, int message,
                      Word& local,
                      Supplier  s ){
   result = qp->ResultStorage(s);
   LongInt* res = (LongInt*) result.addr;
   R* Id = (R*) args[0].addr;
   if(!Id->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   string id = Id->GetValue();
   if(!filesystem){
     res->Set(true,-1);
     return 0;
   }
   try{
     res->Set(true,filesystem->fileSize(id.c_str()));
   } catch(...){
     res->Set(true,-1);
   }
   return 0;
}

ValueMapping dfsFileSizeVM[] = {
  dfsFileSizeVMT<CcString>,
  dfsFileSizeVMT<FText>
};

int dfsFileSizeSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec dfsFileSizeSpec(
  "{string,text} -> bool",
  "dfsFileSize(name)",
  "Returns the size of a file stored in a connected dfs.",
  "query dfsFileSize('myRemoteFile')"
);

Operator dfsFileSizeOp(
  "dfsFileSize",
  dfsFileSizeSpec.getStr(),
  2,
  dfsFileSizeVM,
  dfsFileSizeSelect,
  dfsFileSizeTM
);

/*
2.14 Operator ~dfsFileNames~

*/

ListExpr dfsFileNamesTM(ListExpr args){

  if(!nl->IsEmpty(args)){
    return listutils::typeError("you are crazy to give me an argument?");
  }
  return nl->TwoElemList(nl->SymbolAtom(Symbols::STREAM()),
                         listutils::basicSymbol<FText>());
}

class dfsFileNamesInfo{

  public:
      dfsFileNamesInfo(dfs::Filesystem* fs): pos(0){
         if(fs){
           try{
             names = fs->listFileNames();
           } catch(...){

           }
         }
      }
      FText* next(){
         FText* res = pos>=names.size()?0:new FText(true,names[pos]);
         pos++;
         return res;
      }
  private:
     size_t pos;
     vector<string> names;

};

int dfsFileNamesVM( Word* args,
                      Word& result, int message,
                      Word& local,
                      Supplier  s ){

   dfsFileNamesInfo* li = (dfsFileNamesInfo*) local.addr;
   switch(message){
      case OPEN: if(li) delete li;
                 local.addr = new dfsFileNamesInfo(filesystem);
                 return 0;
      case REQUEST: result.addr = li?li->next():0;
                    return result.addr?YIELD:CANCEL;
      case CLOSE : if(li){
                     delete li;
                     local.addr = 0;
                   }
                   return 0;

   }
   return -1;
}

OperatorSpec dfsFileNamesSpec(
   " -> stream(text)",
   " dfsFileNames() ",
   " Feeds the names of all stored files into a stream of text.",
   " query dfsFileNames() count"
);

Operator dfsFileNamesOp(
  "dfsFileNames",
  dfsFileNamesSpec.getStr(),
  dfsFileNamesVM,
  Operator::SimpleSelect,
  dfsFileNamesTM
);

/*
2.15 Operator ~changeDFSChunkSize~


*/
ListExpr changeDFSChunkSizeTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  if(!CcInt::checkType(nl->First(args))){
    return listutils::typeError("int expected");
  }
  return listutils::basicSymbol<CcBool>();
}


int changeDFSChunkSizeVM( Word* args,
                      Word& result, int message,
                      Word& local,
                      Supplier  s ){
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  CcInt* newSize = (CcInt*) args[0].addr;
  if(!newSize->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  int ns = newSize->GetValue();
  if(ns < 1){
    res->SetDefined(false);
    return 0;
  }
  if(!filesystem){
    res->Set(true,false);
    return 0;    
  }

  try{
    filesystem->changeChunkSize(ns);
    res->Set(true,true);      
  } catch(...){
    res->Set(true,false);
  }
  return 0;
}

OperatorSpec changeDFSChunkSizeSpec(
  "int -> bool",
  "changeDFSChunkSize(newSize)",
  "Changes the chunk size in a connected dfs.",
  " query changeDFSChunkSize(4096)"
);

Operator changeDFSChunkSizeOp(
  "changeDFSChunkSize",
  changeDFSChunkSizeSpec.getStr(),
  changeDFSChunkSizeVM,
  Operator::SimpleSelect,
  changeDFSChunkSizeTM
);



#ifdef REMOTE_FILESYSTEM

/*
2.16 Operator ~registerDFSDataNode~

*/
ListExpr registerDFSDataNodeTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("2 args required");
  }
  ListExpr arg1 = nl->First(args);
  if(!CcString::checkType(arg1) && !FText::checkType(arg1)){
    return listutils::typeError("string or text expected as the first arg");
  }
  if(!CcInt::checkType(nl->Second(args))){
    return listutils::typeError("second arguemnt must be an int");
  }
  return listutils::basicSymbol<CcBool>();
}


template<class T>
int registerDFSDataNodeVMT( Word* args,
                           Word& result, int message,
                           Word& local,
                           Supplier  s ){
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   T* Host = (T*) args[0].addr;
   CcInt* Port = (CcInt*) args[1].addr;
   if(!Host->IsDefined() || !Port->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   string host = Host->GetValue();
   int port = Port->GetValue();
   if(port < 1){
     res->SetDefined(false);
     return 0;
   }
   if(!filesystem){
     res->SetDefined(false);
     return 0;
   }
   stringstream ss;
   ss << "dfs-data://"<<host<<":"<<port;
   try{
     URI uri = URI::fromString(ss.str().c_str());
     filesystem->registerDataNode(uri);
     res->Set(true,true);
   } catch(...){
     res->Set(true,false);
   }
   return 0;
}


ValueMapping registerDFSDataNodeVM[] = {
   registerDFSDataNodeVMT<CcString>,
   registerDFSDataNodeVMT<FText>
};

int registerDFSDataNodeSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec registerDFSDataNodeSpec(
  "{string,text} x int -> bool ",
  "registerDFSDataNode(host, port)",
  "Registers a new data node to the currently connected dfs.",
  " query registerDFSDataNode('remotehost', 4445) "
);

Operator registerDFSDataNodeOp(
  "registerDFSDataNode",
  registerDFSDataNodeSpec.getStr(),
  2,
  registerDFSDataNodeVM,
  registerDFSDataNodeSelect,
  registerDFSDataNodeTM
);

#endif

/*
3 Algebra definition

3.1 Algebra class

*/

class DFSAlgebra: public Algebra{
  public:

      DFSAlgebra(){
        AddOperator(&connectDFSOp);
        AddOperator(&deleteDFSFileOp);
        AddOperator(&storeTextAsDFSFileOp);
        AddOperator(&storeLocalFileToDFSOp);
        AddOperator(&storeDFSFileOp);
        AddOperator(&storeDFSFile2Op);
        AddOperator(&isDFSFileOp);
        AddOperator(&nextDFSWritePositionOp);
        AddOperator(&getDFSFileAsTextOp);
        AddOperator(&deleteAllDFSFilesOp);
        AddOperator(&deleteAllDFSFilesWithCatOp);
        AddOperator(&countDFSFilesOp);
        AddOperator(&totalDFSSizeOp);
        AddOperator(&dfsFileSizeOp);
        AddOperator(&dfsFileNamesOp);
        AddOperator(&changeDFSChunkSizeOp);
#ifdef REMOTE_FILESYSTEM
        AddOperator(&registerDFSDataNodeOp);
#endif
        AddOperator(&disconnectDFSOp);
      }

      ~DFSAlgebra(){
         if(filesystem){
           delete filesystem;
           filesystem = 0;
         }
         if(logger){
            delete logger;
            logger = 0;
         }
      }

};

} // end of namespace dfsalg

/*
3.2 Algebra registration

*/

extern "C"
Algebra*
   InitializeDFSAlgebra( NestedList* nlRef,
                             QueryProcessor* qpRef,
                             AlgebraManager* amRef ) {

    
   return new dfsalg::DFSAlgebra();
}

