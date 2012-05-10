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

#include "../HadoopParallel/HadoopParallelAlgebra.h"

bool isFListStreamDescription(const NList& typeInfo);
ListExpr replaceDLOF(ListExpr createQuery, string listName, fList* listObject,
    vector<string>& DLF_NameList, vector<string>& DLF_fileLocList,
    vector<string>& DLO_NameList, vector<string>& DLO_locList,
    bool& ok, int argIndex = 0);  //Replace DLO and DLF
ListExpr replaceParaOp(
    ListExpr createQuery, vector<string>& flistParaList,
    vector<fList*>& flistObjList, bool& ok);
ListExpr replaceSecObj(ListExpr createQuery);


/*
1.7 fList class

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
      size_t maxCNum = 0);
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
  NList  getColumnList(size_t row);

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

  inline static string tempName(const bool isDB){
    stringstream ss;
    if (isDB){
      ss << "SubXXXDB_" <<
          SecondoSystem::GetInstance()->GetDatabaseName();
    }
    else{
      ss << "SubXXXFL_" << clock()
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

  bool setLocList(NList fllist);
  bool verifyLocList();

  friend class ConstructorFunctions<fList>;
};

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
  }

  bool fetchAllPartFiles();

  Tuple* getNextTuple();



private:
  fList* fileList;
  size_t row, column;

  TupleType* resultType;
  vector<string> partFiles;
  ifstream *inputFile;

  bool partFileOpened();
};


#endif /* HADOOPALGEBRA_H_ */
