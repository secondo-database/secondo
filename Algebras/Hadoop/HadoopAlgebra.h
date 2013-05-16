/*
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

[1] Definition of Auxiliary Classes in HadoopParallelAlgebra

April 2010 Jiamin Lu

[newpage]

1 Auxiliary Classes in HadoopAlgebra

This file claims all relevant classes and methods that is
used in HadoopAlgebra.


*/

#ifndef HADOOPALGEBRA_H_
#define HADOOPALGEBRA_H_

/*
Select different collect option.

  * SEQCOPY: Copy files one by one, read after all files are got

  * PIPECOPY: Copy files with parallel threads, read when one file is available

*/

//#define SEQCOPY
#define PIPECOPY

#include "../HadoopParallel/HadoopParallelAlgebra.h"
#include <pthread.h>

bool isFListStreamDescription(const NList& typeInfo);
ListExpr replaceDLOF(ListExpr createQuery, string listName, fList* listObject,
    vector<string>& DLF_NameList, vector<string>& DLF_fileLocList,
    vector<string>& DLO_NameList, vector<string>& DLO_locList,
    bool ua, bool& ok, int argIndex = 0);  //Replace DLO and DLF
ListExpr replaceParaOp(
    ListExpr createQuery, vector<string>& flistParaList,
    vector<fList*>& flistObjList, bool& ok);
ListExpr replaceSecObj(ListExpr createQuery);

pthread_mutex_t CLI_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
1.7 fList class

At 20th Aug.
Increase a new parameter to flist class, named UEMapQuery (UnExecuted Map Query).
Since it is difficult to adjust the QueryProcessor to permit setting up
a sequence of argument functions in an operator,
e.g using 3 argument functions , A, B and C, in a same operator,
and the result of A and B can be used as the input parameter of the function C.
Regarding this issue, we have to extend the data type ~flist~ with an intermediate status,
by extending a new member variable, UEMapQuery.

This new parameter describes a MapQuery, which can create a flist.
However, this query is not yet processed, hence this flist stays in the intermediate status.
When this flist is delivered to a hadoopReduce or hadoopReduce2 operator,
then the created Hadoop job should process this Map Query at first in its Map stage,
before its Reduce stage.

This paramemeter is a nested-list, composed by three elements.
(MapQuery, DLO-List, DLF-List)
The second and third list describes the distribution of involved DLO and DLF objects
inside this MapQuery.

Since the UEMapQuery need to be executed in another operator,
its output must be DLF type, since this is the only type that can be
shuffled in the cluster.

*/
typedef enum { UNDEF, DLO, DLF } fListKind;
class fList
{
public:

  fList(string objectName, NList typeList,
      clusterInfo *ci,NList fileLocList,
      size_t dupTime,
      bool isDistributed = false,
      fListKind kind = UNDEF,
      size_t maxRNum = 0,
      size_t maxCNum = 0,
      NList ueMapQuery = NList());  //NEW
  fList(fList& rhg);
  ~fList()
  {
    if (interCluster)
      delete interCluster;
  }

  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& errorInfo,
                 bool& correct);
  static ListExpr Out(ListExpr typeInfo,
                      Word value);

  //Status methods
  static bool Open(SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value);
  static bool Save(SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& w);

//  static ListExpr SaveToList(ListExpr typeInfo, Word value);
  static Word RestoreFromList(
      const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct );

  static void Close(const ListExpr typeInfo, Word& w);
  static Word Create(const ListExpr typeInfo);
  static void Delete(const ListExpr typeInfo, Word& w);
  static Word Clone(const ListExpr typeInfo,
                    const Word& w);
  static const string BasicType(){
    return "flist";
  }

  void appendFileLocList(NList elem);

  void inline setDistributed() {
    isDistributed = true;
  }

  //Auxiliary methods
  int SizeOfObj();
  inline bool isAvailable() {
    return ((objKind != UNDEF) && isDistributed);
  }
  inline bool isCollectable(NList currentCluster)
  {
    if (isAvailable() && objKind == DLF){
      return interCluster->covers(currentCluster);
    }
    return false;
  }

  size_t getPartitionFileLoc(size_t row, vector<string>& locations);
  ListExpr getColumnList(size_t row);

  inline string getSubName(){ return subName; }
  inline int getMtxRowNum() { return mrNum; }
  inline int getMtxColNum() { return mcNum; }
  inline int getDupTimes() { return dupTimes; }
  inline NList getInterTypeList() {
    NList inType = objectType.second();
    if (inType.isAtom()){
      inType = inType.enclose();
    }
    return inType;
  }
  inline NList getNodeList() {
    return interCluster->toNestedList();
  }
  inline NList getLocList() { return fileLocList; }
  inline bool isInDB() { return (isAvailable() && (objKind == DLO));}
  inline fListKind getKind() { return objKind; }
  //NEW
  inline NList getUEMapQuery() { return UEMapQuery; }
  inline static string tempName(const bool isDB){
    stringstream ss;
    if (isDB){
// A short prefix allows user to set longer database name on the master.
      ss << "PS" <<
          SecondoSystem::GetInstance()->GetDatabaseName();
    }
    else{
/*
Sometimes, it happens that two parallel operators are used in a same query,
and both get a same name for their sub-objects, if we use clock function to
set names. Hence I picked up the rand function to substitute it.

*/
      ss << "SubXXXFL_" << WinUnix::rand()
          << "_" << WinUnix::getpid();
    }

    return ss.str();
  }

private:
  fList() {}

  string subName;
  NList objectType;
  clusterInfo *interCluster;

  NList fileLocList;
  size_t dupTimes;  // duplicate times
  size_t  mrNum,    // matrix row number
          mcNum;    // matrix column number
  bool isDistributed;
  fListKind objKind;
  //NEW
  NList UEMapQuery;  // UnExecuted Map Query

  bool setLocList(NList fllist);
  bool verifyLocList();

  friend class ConstructorFunctions<fList>;
};

/*
1.6 RowInLocList structure

It describes one row in the flistLoc list.

*/

typedef struct
{
  int dsIndex, column;
  string filePath;

}rowInLocRel;

bool static rowRelInfo(pair<int, rowInLocRel> r1, pair<int, rowInLocRel> r2)
{
  return r1.first < r2.first;
}

/*
1.7 SpreadLocalInfo class

*/

class SpreadLocalInfo{
public:
  SpreadLocalInfo(string fileName, string filePath, int dupTimes,
             int attrIndex1, int rowNum, bool keepAI,
             int attrIndex2, int colNum, bool keepAJ);

  bool insertTuple(Word wTuple);
  bool closeAllPartFiles();
  inline bool isAvailable(){ return (!(resultList == 0)); }

  ~SpreadLocalInfo(){

    // clean all file handles.
    openFileList.clear();
    map<size_t, rowFile*>::iterator mit = matrixRel.begin();
    while (mit != matrixRel.end()){
      rowFile::iterator rit = mit->second->begin();
      while ( rit!= mit->second->end()){
        fileInfo* fp = rit->second;
        delete fp;
        rit++;
      }
      mit++;
    }

    if (ci){
      delete ci;
      ci = 0;
    }
    if (resultList){
      delete resultList;
      resultList = 0;
    }
    if (exportTupleType){
      exportTupleType->DeleteIfAllowed();
    }
  }

  fList* getResultList(){
    if (done)
      return resultList;
    else
      return 0;
  }

private:
  typedef map<size_t, fileInfo*> rowFile;
  map<size_t, rowFile*> matrixRel;
  clusterInfo *ci;
  fList* resultList;

  string partFileName, partFilePath;
  int attrIndex1,attrIndex2,rowAmount,colAmount;
  bool keepA1, keepA2;
  bool done;
  size_t tupleCounter;
  size_t dupTimes;

  TupleType *exportTupleType;
  //~openFileList~ keeps at most MAX_FILEHANDLENUM file handles.
  vector<fileInfo*> openFileList;


  bool duplicateOneRow(rowFile *row);
  size_t hashValue(Tuple *t, int attrIndex, int scale);
//  size_t rowHashValue(Tuple* t);
//  size_t columnHashValue(Tuple* t);
  bool openFile(fileInfo *fp);
};


/*
1.7 CollectLocalInfo class

*/
class CLI_Thread;
class CollectLocalInfo{
public:
  CollectLocalInfo(fList* valueList, size_t row, size_t column);

  ~CollectLocalInfo(){
    if (resultType){
      resultType->DeleteIfAllowed();
      resultType = 0;
    }
    if (inputFile){
      delete inputFile;
      inputFile = 0;
    }
#ifdef PIPECOPY
    pthread_mutex_destroy(&CLI_OWN_Mutex);
#endif
  }

  bool fetchAllPartFiles();
  //Copy files in sequential order
  static void* fetchAllPartFiles2(void* ptr);
  //Copy files in pipeline, with an independent thread

  Tuple* getNextTuple();    //Sequence Copy
  Tuple* getNextTuple2();   //Parallel Copy

  size_t getRow(){ return row; }
  size_t getColumn(){ return column; }
  fList* getFileList(){ return fileList;}

private:
  fList* fileList;
  size_t row, column;

  TupleType* resultType;
  vector<string> partFiles;
  ifstream *inputFile;

#ifdef PIPECOPY
  static const int PipeWidth = 10;
  pthread_t threadID[PipeWidth];
  bool tokenPass[PipeWidth];
  bool *fileStatus;
  pthread_t faf_TID;
  size_t fIdx;
  static pthread_mutex_t CLI_OWN_Mutex;
  vector<CLI_Thread*> fileTasks;
  static void* tCopyFile(void* ptr);
#else
  bool partFileOpened();
#endif

};

class CLI_Thread
{
public:
  CLI_Thread(CollectLocalInfo* _ci, string _sn, int _r, ListExpr _clst)
    : cli(_ci), subName(_sn), row(_r)
  {
    columnList = _clst;
    remotePaths = new vector<string>();
    allColumns = new vector<int>();
    token = 0;
  }

  void setToken(int _t)
  {
    token = _t;
  }


  CollectLocalInfo* cli;
  string subName;
  int row;
  int token;
  ListExpr columnList;
  vector<string>* remotePaths;
  vector<int>* allColumns;
};

class PFFeedLocalInfo{
public:

  PFFeedLocalInfo(Supplier s, Word inputStream,
      int rp, int cp, int dp,
      string fileName, string filePath,
      int attTimes);
  ~PFFeedLocalInfo(){

  }

  static void* fetchAllFiles(void* ptr);
  //Thread to copy all files
  Tuple* getNextTuple();

  string getFilePrefixName(){return fileName;}
  string getLocalFilePath(){return localFilePath;}
  int getAttemptTimes(){return attTimes;}
private:
  string fileName, localFilePath;
  TupleType* resultType;
  clusterInfo *interCluster;
  vector<string> partFiles;
  ifstream *inputFile;
  pthread_t faf_TID;
  int attTimes;

  static const int PipeWidth = 10;
  bool tokenPass[PipeWidth];
  static void* tCopyFile(void* ptr);
  //Thread to copy one involved file

  size_t fIdx;
  string curFileName;
  ifstream *curFilePt;
};


class PLI_FAF_Thread
{
public:
  PLI_FAF_Thread(PFFeedLocalInfo* _pi, Word _is,
      int _rp, int _cp, int _dp)
  : pli(_pi), inputStream(_is)
  {
    attrPos[0] = _rp;
    attrPos[1] = _cp;
    attrPos[2] = _dp;
  }

  PFFeedLocalInfo* pli;
  Word inputStream;
  int attrPos[3];  //Row, Column, Dest
};

class PLI_CF_Thread
{
public:
  PLI_CF_Thread(PFFeedLocalInfo* _pi, int _r, int _c,
      int _d, int _t)
    : pli(_pi), row(_r), column(_c), dest(_d), token(_t){}

  PFFeedLocalInfo* pli;
  int row, column, dest, token;
};

#endif /* HADOOPALGEBRA_H_ */
