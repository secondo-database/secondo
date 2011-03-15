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
#include "FTextAlgebra.h"
#include "RTuple.h"
#include "FileSystem.h"
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include "../Array/ArrayAlgebra.h"
#ifdef SECONDO_WIN32
#include "Win32Socket.h"
#endif

const string rmDefaultPath = ":secondo/bin/parallel/";
const int MAX_COPYTIMES = 5;
const int MAX_FILEHANDLENUM = 200;

string
tranStr(const string& s, const string& from, const string& to);
string
getFilePath(string path, const string fileName);

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
      Array* machines = 0, int servIndex = -1, int att = -1)
  {
    //Fetch binary file from remote machine.
    bool fileFound = false;
    bool isLocal = false;
    if (!machines)
      isLocal = true;

    string hostName, targetName = "";
    if (!isLocal)
    {
      char buf[255];
      memset(buf, '\0', sizeof(buf));
      if (0 != gethostname(buf, sizeof(buf) - 1 ))
      {
        cerr << "Error: Can't get localhost name" << endl;
        return false;
      }
      hostName.assign(buf);
    }

    cerr << "isLocal: " << (isLocal ? "true" : "false") << endl;

    while(!fileFound && (isLocal || (att-- > 0)))
    {
      string servName, rFileName;
      if (!isLocal)
      {
        int aSize = machines->getSize();
        servName =
            ((CcString*)(machines->
             getElement((servIndex++) % aSize)).addr)->GetValue();
        if (0 == targetName.length())
          targetName = servName;

        rFileName = relName;
        if (targetName.compare(hostName) != 0 &&
            targetName.compare(servName) != 0)
          rFileName += ("_" + targetName);
      }

      cerr << "rFileName: " << rFileName << endl;
      cerr << "hostName : " << hostName << endl;
      cerr << "servName : " << servName << endl;

      //Find the file before reading it.
      int findTimes = MAX_COPYTIMES;
      while (!fileFound && (findTimes-- > 0))
      {
        FILE *fs;
        char qBuf[1024];
        memset(qBuf, '\0', sizeof(qBuf));
        string qStr;
        if (isLocal || (0 == servName.compare(hostName)))
          qStr = "ls " + filePath + " 2>/dev/null";
        else
          qStr = "ssh " + servName +
                 " ls " + rmDefaultPath.substr(1) + rFileName +
                 " 2>/dev/null";
        cerr << "qStr: " << qStr << endl;
        fs = popen(qStr.c_str(), "r");
        if (fgets(qBuf, sizeof(qBuf), fs) != NULL)
          fileFound = true;
        pclose(fs);
      }

      if (!fileFound)
      {
        if (!isLocal)
        {
          //Attempt the next possible candidate node if have
//          att--;
//          servIndex++;
          continue;
        }
        else
        {
          //Specify to fetch the local file, but it's not exist.
          cerr << "Error: Local file: " << filePath
               << " is NOT exist!\n";
          return false;
        }
      }

      //Found the file on the local or remote node
      if (!isLocal && (0 != servName.compare(hostName)))
      {
/*
Find the file on the remote node, attempt to copy it to local node.
The file that ~filePath~ point to is assumed as an old file,
and is deleted before the copy starts.
If the copy doesn't work for MAX\_COPYTIMES times,
then try the next possible candidate node.

*/
        int copyTimes = MAX_COPYTIMES;
        filePath += ("_" + targetName);
        FileSystem::DeleteFileOrFolder(filePath);
        do
        {
          system(("scp " + servName + rmDefaultPath + rFileName +
                  " " + filePath).c_str());
        }while ((--copyTimes > 0) &&
            !FileSystem::FileOrFolderExists(filePath));
        if (0 > copyTimes)
          fileFound = false;
      }
    }

    if (!fileFound)
    {
      cerr << "Error: File '" << filePath
           << "' doesn't exist!\n" << endl;
      return false;
    }
    tupleBlockFile = new ifstream(filePath.c_str(), ios::binary);
    if (!tupleBlockFile->good())
    {
      cerr << "Error accessing file '" << filePath << "'\n\n";
      tupleBlockFile = 0;
    }

    if (tupleBlockFile)
    {
      //get the description list
      u_int32_t descSize;
      size_t fileLength;
      tupleBlockFile->seekg(0, ios::end);
      fileLength = tupleBlockFile->tellg();
      tupleBlockFile->seekg(
          (fileLength - sizeof(descSize)), ios::beg);
      tupleBlockFile->read((char*)&descSize, sizeof(descSize));

      char descStr[descSize];
      tupleBlockFile->seekg(
          (fileLength - (descSize + sizeof(descSize))), ios::beg);
      tupleBlockFile->read(descStr, descSize);
      tupleBlockFile->seekg(0, ios::beg);

      NList descList = NList(binDecode(string(descStr)));

      //Initialize the sizes of progress local info
      noAttrs = tupleType->GetNoAttributes();
      total = descList.first().intval();
      attrSize = new double[noAttrs];
      attrSizeExt = new double[noAttrs];
      for(int i = 0; i < noAttrs; i++)
      {
        attrSizeExt[i] =
            descList.elem(4 + i*2).realval() / total;
        attrSize[i] =
            descList.elem(4 + (i*2 + 1)).realval() / total;

        SizeExt += attrSizeExt[i]; //average sizeExt of a tuple
        Size += attrSize[i];
      }

      sizesInitialized = true;
      sizesChanged = true;
    }

    return true;
  }

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
  fileInfo(size_t _hv, string _fp, size_t _an):
  cnt(0), totalExtSize(0),totalSize(0),
  lastTupleIndex(0), fileOpen(false)
  {
    blockFileName = _fp + "_" + int2string(_hv);
    attrExtSize.resize(_an);
    attrSize.resize(_an);
  }

  bool openFile()
  {
    ios_base::openmode mode = ios::binary;
    if (lastTupleIndex == 0)
      mode |= ios::app;
    blockFile.open(blockFileName.c_str(), mode);
    fileOpen = true;
    return blockFile.good();
  }

  void closeFile()
  {
    if (isFileOpen())
      blockFile.close();
    fileOpen = false;
  }

  bool writeTuple(Tuple* tuple, size_t tupleIndex,
       int attrIndex, TupleType* exTupleType)
  {
    size_t coreSize = 0;
    size_t extensionSize = 0;
    size_t flobSize = 0;

    //The tuple written to the file need remove the key attribute
    Tuple* newTuple = new Tuple(exTupleType);
    int j = 0;
    for (int i = 0; i < tuple->GetNoAttributes(); i++)
    {
      if (i != attrIndex)
        newTuple->CopyAttribute(i, tuple, j++);
    }

    size_t tupleBlockSize =
        newTuple->GetBlockSize(coreSize, extensionSize, flobSize,
                        &attrExtSize, &attrSize);
    totalSize += (coreSize + extensionSize + flobSize);
    totalExtSize += (coreSize + extensionSize);

    char* tBlock = (char*)malloc(tupleBlockSize);
    newTuple->WriteToBin(tBlock, coreSize, extensionSize, flobSize);
    blockFile.write(tBlock, tupleBlockSize);
    free(tBlock);
    lastTupleIndex = tupleIndex;
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
    int attrNum = attrExtSize.size();
    for(int i = 0; i < attrNum; i++)
    {
      descList.append(NList(attrExtSize[i]));
      descList.append(NList(attrSize[i]));
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
    attrExtSize.erase(attrExtSize.begin(), attrExtSize.end());
    attrSize.erase(attrSize.begin(), attrSize.end());
  }

  string getFileName() {
    return blockFileName;
  }
  bool isFileOpen()
  {  return fileOpen;  }
  size_t getLastTupleIndex()
  {  return lastTupleIndex;  }
private:
  string blockFileName;
  ofstream blockFile;

  int cnt;
  int totalExtSize;
  int totalSize;
  vector<double> attrExtSize;
  vector<double> attrSize;

  size_t lastTupleIndex;
  bool fileOpen;
};

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

  string filePath;
  map<size_t, fileInfo*> fileList;
  map<size_t, fileInfo*>::iterator fit;

  size_t nBuckets;
  int attrIndex;
  TupleType *resultTupleType, *exportTupleType;
  int openFilesNum;
  size_t tupleCounter;

public:
  FDistributeLocalInfo(string baseName, string path,
                       int nBuckets, int attrIndex,
                       ListExpr resultTupleType,
                       ListExpr inputTupleType);

  bool insertTuple(Word tuple);
  void startCloseFiles();
  Tuple* closeOneFile();
  ~FDistributeLocalInfo(){
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
  fList(NList typeList, string fileName);
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
                   Word& value);  //Unnecessary
  static bool Save(SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& w);

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

  //Get the next possible location of a cell file
  int getNextLoc(int rowNum, int columnNum);
private:
  fList() {}

  NList *objectType;
  vector< vector< vector<int> > > *fileLocList;

  //partition number, maximum node number, maximum candidate number
  int partNum, mrNum, mnNum, mcNum;
  string fileName;
  bool isAvailable;

  bool setLocList(NList fllist);

  inline string getFileName(){
    return fileName;
  }
  inline int getPartNum() { return partNum; }
  inline int getMaxRowNum() { return mrNum; }
  inline int getMaxNodeNum() { return mnNum; }
  inline int getMaxCandidateNum() { return mcNum; }
  inline vector< vector< vector<int> > >* getLocList(){
    return fileLocList;
  }


  friend class ConstructorFunctions<fList>;
};


#endif /* HADOOPPARALLELALGEBRA_H_ */
