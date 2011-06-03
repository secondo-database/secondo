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

1 Auxiliary Classes in HadoopParallelAlgebra

This file claims all relevant classes and methods that is
used in HadoopParallelAlgebra. Includes follow classes:

  * ~deLocalInfo~. Assists ~doubleexport~ operator.

  * ~phjLocalInfo~. Assists ~parahashjoin~ operator.

And includes one method:

  * ~renameList~. Assists ~parahashjoin~ operator

*/
#ifndef HADOOPPARALLELALGEBRA_H_
#define HADOOPPARALLELALGEBRA_H_

#define MAX_WAITING_TIME 10

#include <cstdlib>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include "RTuple.h"
#include "FTextAlgebra.h"
#ifdef SECONDO_WIN32
#include <winsock2.h>
#endif
#include "TemporalAlgebra.h"
#include "Progress.h"
#include "FileSystem.h"
#include "../Array/ArrayAlgebra.h"
#include "Profiles.h"

const int MAX_COPYTIMES = 5;
const size_t MAX_FILEHANDLENUM = 100;

string
tranStr(const string& s, const string& from, const string& to);
string
getFilePath(string path, const string fileName, bool extendPath = true);
string addFileIndex(string fileName, int index);

/*
1.1 deLocalInfo Class

Assists ~doubleexport~ operator.
First traverse tuples from streamA to the end, then traverse streamB.

*/
class deLocalInfo
{
private:

  Word streamA;
  Word streamB;
  bool isAEnd;
  //Check out whether gets to the end of streamA.

  int attrIndexA;
  int attrIndexB;

  ListExpr tupTypeA;
  ListExpr tupTypeB;

  TupleType *resultTupleType;

  Tuple* makeTuple(Word stream, int index, int sig);

public:
  deLocalInfo(Word _streamA, Word wAttrIndexA,
              Word _streamB, Word wAttrIndexB,
              Supplier s);

  ~deLocalInfo()
  {
    //delete resultTupleType;
    if (resultTupleType != 0)
      resultTupleType->DeleteIfAllowed();
  }

  Tuple* nextResultTuple();
};

/*
1.2 renameList Method

Assists ~parahashjoin~ operator.
Append an ~appendName~ after each attribute name of ~oldTupleList~,
to avoid the situation that joined relations have a same attribute name.

*/
ListExpr renameList(ListExpr oldTupleList, string appendName);

/*
1.3 binEncode Method

Assist ~doubleexport~, ~add0Tuple~ operator.
Transform a nestedList into a transportable Base 64 binary string

*/
string binEncode(ListExpr nestList);

/*
1.4 binDecode Method

Assist ~parahashjoin~, ~parajoin~ opertor.
To decode a Base 64 binary string back to nestedList efficiently.

*/
ListExpr binDecode(string binStr);

/*
1.3 phjLocalInfo Class

Assists ~parahashjoin~ operator.
~getNewProducts~ collects tuples in each buckets,
and make products if the tuples come from different sources.
Put the product results in the TupleBuffer ~joinedTuples~.


*/
class phjLocalInfo
{
private:

  Word mixStream;
  ListExpr aTypeInfo;
  ListExpr bTypeInfo;
  TupleType *resultTupleType;
  TupleType *tupleTypeA, *tupleTypeB;

  TupleBuffer *joinedTuples;
  GenericRelationIterator *tupleIterator;

  GenericRelationIterator* getNewProducts();

public:
  phjLocalInfo(Word _stream, Supplier s,
               ListExpr ttA, ListExpr ttB);

  ~phjLocalInfo()
  {
    if (joinedTuples != 0)
      delete joinedTuples;
    joinedTuples = 0;

    if (resultTupleType != 0)
      resultTupleType->DeleteIfAllowed();
    resultTupleType = 0;
    if (tupleTypeA != 0)
      tupleTypeA->DeleteIfAllowed();
    tupleTypeA = 0;
    if (tupleTypeB != 0)
      tupleTypeB->DeleteIfAllowed();
    tupleTypeB = 0;

    if (tupleIterator != 0)
      delete tupleIterator;
    tupleIterator = 0;
  }

  Word nextJoinTuple();
};


/*
1.4 pjLocalInfo Class

Assists ~parajoin~ operator.

*/
typedef enum { tupBufferA, tupBufferB } tupleBufferType;

class pjLocalInfo
{
private:

  Word mixedStream;
  Supplier JNfun;
  ListExpr aTypeInfo;
  ListExpr bTypeInfo;
  TupleType *tupleTypeA, *tupleTypeB;

  TupleBuffer *tbA;
  GenericRelationIterator *itrA;
  TupleBuffer *tbB;
  GenericRelationIterator *itrB;
  const int maxMem;
  bool endOfStream;
  bool isBufferFilled;

  int bucketNum;
  int tupNum;


  //Get the tuples within one bucket,
  //and fill them into tupleBuffers
  void loadTuples();
public:
  pjLocalInfo(Word inputStream, Supplier fun, Supplier s,
      ListExpr ttA, ListExpr ttB,
      int mem) :
    mixedStream(inputStream),
    aTypeInfo(ttA),
    bTypeInfo(ttB),
    tbA(0),
    itrA(0),
    tbB(0),
    itrB(0),
//    tpIndex_A(-1),
//    tpIndex_B(-1),
    maxMem(mem),
    endOfStream(false),
    isBufferFilled(false)
  {
    //Indicator both arguments of the function accept stream
    JNfun = fun;
    qp->SetupStreamArg(JNfun, 1, s);
    qp->SetupStreamArg(JNfun, 2, s);
    qp->Open(JNfun);

    tupleTypeA = new TupleType(ttA);
    tupleTypeB = new TupleType(ttB);
  }

  ~pjLocalInfo()
  {
    if (tbA != 0)
      delete tbA;
    tbA = 0;

    if (tbB != 0)
      delete tbB;
    tbB = 0;

    if (tupleTypeA != 0)
      tupleTypeA->DeleteIfAllowed();
    tupleTypeA = 0;
    if (tupleTypeB != 0)
      tupleTypeB->DeleteIfAllowed();
    tupleTypeB = 0;
  }

  // Get the next tuple from tupleBuffer A or tuppleBuffer B.
  Tuple* getNextInputTuple(tupleBufferType tbt);

  // Get the next result tuple
  void* getNextTuple();
};

/*
1.4 pj2LocalInfo Class

Assists ~parajoin2~ operator.

*/

class pj2LocalInfo
{
private:
  Word streamA, streamB;
  Supplier pf;  //parameter function
  int keyAIndex, keyBIndex;

  TupleBuffer *tba, *tbb;
  GenericRelationIterator *ita, *itb;
  RTuple cta, ctb;  //Cached tuple for the next bucket

  int maxMem;
  bool endOfStream;
  bool moreTuples;
  //any stream is end, then the operation is over

  bool LoadTuples();
  //find both streams' closest buckets
  //having the same key attribute value

  int CompareTuples(Tuple* ta, int kai,
                    Tuple* tb, int kbi);

  inline Tuple* NextTuple(Word stream)
  {
    bool yield = false;
    Word result( Address(0) );

    if(stream.addr)
    {
      qp->Request(stream.addr, result);
      yield = qp->Received(stream.addr);

      if(yield)
      {
        return static_cast<Tuple*> (result.addr);
      }
    }

    result.addr = 0;
    return static_cast<Tuple*>( result.addr );
  }

public:
  pj2LocalInfo(Word _sa, Word _sb, Word _kai, Word _kbi,
               Word _fun, Supplier s)
  : streamA(_sa), streamB(_sb),
    tba(0), tbb(0), ita(0), itb(0),
    cta(0), ctb(0),
    endOfStream(false), moreTuples(true)
  {
    keyAIndex = StdTypes::GetInt( _kai ) - 1;
    keyBIndex = StdTypes::GetInt( _kbi ) - 1;

    pf = _fun.addr;
    qp->SetupStreamArg(pf, 1, s);
    qp->SetupStreamArg(pf, 2, s);
    qp->Open(pf);

    maxMem = qp->MemoryAvailableForOperator();
  }

  ~pj2LocalInfo()
  {
    if (ita)
      delete ita; ita = 0;
    if (tba)
      delete tba; tba = 0;

    if (itb)
      delete itb; itb = 0;
    if (tbb)
      delete tbb; tbb = 0;
  }

  Tuple* getNextTuple();

  inline Tuple* getNextInputTuple(tupleBufferType tbt)
  {
    if (ita && tupBufferA == tbt)
      return ita->GetNextTuple();
    else if (itb && tupBufferB == tbt)
      return itb->GetNextTuple();
    else
      return 0;
  }
};

/*
1.5 a0tLocalInfo

Assists ~add0Tuple~ operator.

*/
struct a0tLocalInfo
{
  string key;
  TupleType *resultTupleType;
  Tuple *cachedTuple;
  bool needInsert;
  string sepTupleStr;

  inline a0tLocalInfo(ListExpr rtNL):
      key(""), cachedTuple(0),
      needInsert(false)
  {
    resultTupleType = new TupleType(nl->Second(rtNL));
  }

  inline ~a0tLocalInfo()
  {
    if (resultTupleType != 0)
      resultTupleType->DeleteIfAllowed();
    resultTupleType = 0;

    if ( cachedTuple != 0)
      cachedTuple->DeleteIfAllowed();
    cachedTuple = 0;

  }
};

/*
1.6 clusterInfo

We define two list files in all nodes involved in a parallel Secondo.
These files lists all nodes' ip adresses, cell file locations,
and Secondo Monitor's access ports.

These two files are separated to master and slaves list.
The master list only have one line, and the slave list have lines of
as same as the disks of the slaves.
These two files are specified in nodes, by setting their
PARALLEL\_SECONDO\_MASTER and PARALLEL\_SECONDO\_SLAVES
environment virables.

This class is used to verify these two files.

*/
class clusterInfo
{
public:
  clusterInfo(bool _isMaster = false);

  string getPath(size_t loc, bool round = false, bool withIP = true);
  string getIP(size_t loc, bool round = false);

  inline int getLocalNode(){
    if (localNode < 0)
      localNode = searchLocalNode();
    return localNode;
  }

  inline string getLocalIP(){
    string confPath = string(getenv("SECONDO_CONFIG"));
    return SmiProfile::GetParameter("ParallelSecondo",
          "localIP","", confPath);
  }

/*
If the Parallel Secondo system is involved,
then the default path cannot be used anymore.

*/
  inline string getLocalPath()
  {
    string confPath = string(getenv("SECONDO_CONFIG"));
    return SmiProfile::GetParameter("ParallelSecondo",
          "SecondoFilePath","", confPath);
  }

  inline bool isOK(){  return ok;  }
  inline int getLines(){
    if (disks)
      return disks->size();
    else
      return 0;
  }

  void print();

private:
  string ps_master;
  string ps_slaves;
  bool isMaster;
  string fileName;
  typedef pair<string, pair<string, int> > diskDesc;
  vector<diskDesc> *disks;
  bool ok;
  int localNode;

  int searchLocalNode();
};

/*
1.4 FFeedLocalInfo Class

Assists ~ffeed~ operator.
Support progress estimation.

*/
class FFeedLocalInfo: public ProgressLocalInfo
{
public:
  FFeedLocalInfo( ListExpr streamTypeList)
  : tupleBlockFile(0)
  {

    tupleType = new TupleType(SecondoSystem::GetCatalog()
                    ->NumericType(nl->Second(streamTypeList)));
  }

  ~FFeedLocalInfo() {
    if (tupleBlockFile)
    {
      delete tupleBlockFile;
      tupleBlockFile = 0;
    }
    if (tupleType)
    {
      tupleType->DeleteIfAllowed();
    }
  }

  bool fetchBlockFile(string relName, string filePath,
       int pdi = -1, int tgi = -1, int att = -1);

  Tuple* getNextTuple(){
    if (0 == tupleBlockFile )
      return 0;

    Tuple* t = 0;
    u_int32_t blockSize;
    tupleBlockFile->read(
        reinterpret_cast<char*>(&blockSize),
        sizeof(blockSize));
    if (!tupleBlockFile->eof() && (blockSize > 0))
    {
      blockSize -= sizeof(blockSize);
      char tupleBlock[blockSize];
      tupleBlockFile->read(tupleBlock, blockSize);
      t = new Tuple(tupleType);
      t->ReadFromBin(tupleBlock, blockSize);
    }

    return t;
  }

  ifstream *tupleBlockFile;
  TupleType* tupleType;

private:
  bool isLocalFileExist(string filePath);

};

/*
1.5 structure ~fconsumeLocalInfo~

Assist ~fconsume~ operator.

*/
struct fconsumeLocalInfo
{
  int state;
  int current;
};

/*
1.6 hdpJoinLocalInfo class

*/

class hdpJoinLocalInfo
{
private:
  vector<pair<int, int> > resultList;
  vector<pair<int, int> >::iterator it;
  TupleType *resultTupleType;

public:
  hdpJoinLocalInfo(Supplier s){

    ListExpr resultTTList = GetTupleResultType(s);
    resultTupleType = new TupleType(nl->Second(resultTTList));
  }

  void insertPair(pair<int, int> elem){
    resultList.push_back(elem);
  }

  void setIterator(){
    it = resultList.begin();
  }

  Tuple* getTuple()
  {
    if (it == resultList.end())
      return 0;
    else {
      Tuple *t = new Tuple(resultTupleType);
      t->PutAttribute(0, new CcInt((*it).first));
      t->PutAttribute(1, new CcInt((*it).second));

      it++;
      return t;
    }
  }
};


/*
1.7 fDistributeLocalInfo class

*/
class fileInfo{
public:
  fileInfo(size_t _hv, string _fp, string _fn, size_t _an):
  cnt(0), totalExtSize(0),totalSize(0),
  lastTupleIndex(0), fileOpen(false)
  {
    blockFileName = _fn + "_" + int2string(_hv);
    blockFilePath = _fp;
    FileSystem::AppendItem(blockFilePath, blockFileName);

    attrExtSize = new vector<double>(_an);
    attrSize = new vector<double>(_an);
  }

  bool openFile()
  {
    ios_base::openmode mode = ios::binary;
    if (lastTupleIndex > 0)
      mode |= ios::app;
    blockFile.open(blockFilePath.c_str(), mode);
    fileOpen = true;
    return blockFile.good();
  }

  void closeFile()
  {
    if (fileOpen)
      blockFile.close();
    fileOpen = false;
  }

  bool writeTuple(Tuple* tuple, size_t tupleIndex,
       int attrIndex, TupleType* exTupleType, bool kpa)
  {
    size_t coreSize = 0;
    size_t extensionSize = 0;
    size_t flobSize = 0;

    //The tuple written to the file need remove the key attribute
    Tuple* newTuple;
    if (kpa)
      newTuple = tuple;
    else
    {
      newTuple = new Tuple(exTupleType);
      int j = 0;
      for (int i = 0; i < tuple->GetNoAttributes(); i++)
      {
        if (i != attrIndex)
          newTuple->CopyAttribute(i, tuple, j++);
      }
    }
    size_t tupleBlockSize =
        newTuple->GetBlockSize(coreSize, extensionSize, flobSize,
                        attrExtSize, attrSize);
//                        &attrExtSize, &attrSize);
    totalSize += (coreSize + extensionSize + flobSize);
    totalExtSize += (coreSize + extensionSize);

    char* tBlock = (char*)malloc(tupleBlockSize);
    newTuple->WriteToBin(tBlock, coreSize,
                         extensionSize, flobSize);
    blockFile.write(tBlock, tupleBlockSize);
    free(tBlock);
    if (!kpa)
      newTuple->DeleteIfAllowed();
    lastTupleIndex = tupleIndex + 1;
    cnt++;

    return true;
  }

  int writeLastDscr()
  {
    // write a zero after all tuples to indicate the end.
    u_int32_t endMark = 0;
    blockFile.write((char*)&endMark, sizeof(endMark));

    // build a description list of output tuples
    NList descList;
    descList.append(NList(cnt));
    descList.append(NList(totalExtSize));
    descList.append(NList(totalSize));
    int attrNum = attrExtSize->size();
    for(int i = 0; i < attrNum; i++)
    {
      descList.append(NList((*attrExtSize)[i]));
      descList.append(NList((*attrSize)[i]));
    }

    //put the base64 code of the description list to the file end.
    string descStr = binEncode(descList.listExpr());
    u_int32_t descSize = descStr.size() + 1;
    blockFile.write(descStr.c_str(), descSize);
    blockFile.write((char*)&descSize, sizeof(descSize));

    return cnt;
  }
  ~fileInfo()
  {
    if (fileOpen)
      blockFile.close();
    attrExtSize->clear();
    delete attrExtSize;
    attrSize->clear();
    delete attrSize;
  }

  string getFileName() {
    return blockFileName;
  }

  string getFilePath() {
    return blockFilePath;
  }
  bool isFileOpen()
  {  return fileOpen;  }
  size_t getLastTupleIndex()
  {  return lastTupleIndex;  }

private:
  string blockFilePath, blockFileName;
  ofstream blockFile;

  int cnt;
  int totalExtSize;
  int totalSize;
  vector<double>* attrExtSize;
  vector<double>* attrSize;

  size_t lastTupleIndex;
  bool fileOpen;
};

bool static compFileInfo(fileInfo* fi1, fileInfo* fi2)
{
  return fi1->getLastTupleIndex() > fi2->getLastTupleIndex();
}
class FDistributeLocalInfo
{

private:
  size_t HashTuple(Tuple* tuple)
  {
    size_t hashValue =
        ((Attribute*)tuple->GetAttribute(attrIndex))->HashValue();
    if (nBuckets != 0)
      return hashValue % nBuckets;
    else
      return hashValue;
  }
  bool openFile(fileInfo* tgtFile);

  string fileBaseName;
  string filePath;
  map<size_t, fileInfo*> fileList;
  map<size_t, fileInfo*>::iterator fit;

  //~openFileList~ keeps at most MAX_FILEHANDLENUM file handles.
  vector<fileInfo*> openFileList;
  bool duplicateOneFile(fileInfo* fi);

  size_t nBuckets;
  int attrIndex;
  bool kpa;
  TupleType *resultTupleType, *exportTupleType;
  size_t tupleCounter;

  //data remote variables
  int firstDupTarget, dupTimes, localIndex;
  string cnIP;
  clusterInfo *ci;
  bool* copyList;


public:
  FDistributeLocalInfo(string baseName, string path,
                       int nBuckets, int attrIndex,
                       bool keepKeyAttribute,
                       ListExpr resultTupleType,
                       ListExpr inputTupleType,
                       int dtIndex, int dupTimes);

  bool insertTuple(Word tuple);
  bool startCloseFiles();
  Tuple* closeOneFile();

  ~FDistributeLocalInfo(){
    map<size_t, fileInfo*>::iterator fit;
    for(fit = fileList.begin(); fit != fileList.end(); fit++)
    { delete (*fit).second; }

    openFileList.clear();
    fileList.clear();
    if (resultTupleType)
      resultTupleType->DeleteIfAllowed();
    if (exportTupleType)
      exportTupleType->DeleteIfAllowed();
  }
};

/*
1.7 fList class

*/
class fList
{
public:
  fList(string objectName, NList typeList,
        NList nodeList, NList fileLocList,
        size_t dupTime, bool isAvailable = false);
  fList(fList& rhg);
  ~fList();

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

  static ListExpr SaveToList(ListExpr typeInfo, Word value);
  static Word RestoreFromList(
      const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct );

  static void Close(const ListExpr typeInfo, Word& w);
  static Word Create(const ListExpr typeInfo);
  static void Delete(const ListExpr typeInfo, Word& w);
  static Word Clone(const ListExpr typeInfo,
                    const Word& w);
  static bool CheckFList(ListExpr type, ListExpr& errorInfo)
  {
    return (nl->IsEqual(type, "flist"));
  }


  //Auxiliary methods
  int SizeOfObj();
  inline bool isOK() { return isAvailable; }

  //Get the next possible location of a cell file
  size_t getNextLoc(size_t rowNum, size_t columnNum);
private:
  fList() {}

  string objName;
  NList objectType;
  NList nodesList;
  NList fileLocList;

  size_t dupTimes; // duplicate times
  bool isAvailable;
  int mrNum, // matrix row number,
      mcNum; // matrix column number,

  bool setLocList(NList fllist);
  void verifyLocList();
  inline string getObjName(){ return objName; }
  inline int getNodesNum()  { return nodesList.length(); }
  inline int getMtxRowNum() { return mrNum; }
  inline int getMtxColNum() { return mcNum; }
  inline int getDupTimes() { return dupTimes; }
  inline NList getTypeList() { return objectType; }
  inline NList getNodeList() { return nodesList; }
  inline NList getLocList() { return fileLocList; }

  friend class ConstructorFunctions<fList>;
};


#endif /* HADOOPPARALLELALGEBRA_H_ */
