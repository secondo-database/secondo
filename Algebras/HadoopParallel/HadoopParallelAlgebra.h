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

const string rmDefaultPath = /*$HOME/*/":secondo/bin/parallel/";

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

//  Tuple* makeTuple(Word stream, int index, ListExpr typeInfo, int sig);
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
  phjLocalInfo(Word _stream, Supplier s, ListExpr ttA, ListExpr ttB);

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


  //Get the tuples within one bucket, and fill them into tupleBuffers
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

    qp->Request(stream.addr, result);
    yield = qp->Received(stream.addr);

    if(yield)
    {
      return static_cast<Tuple*>( result.addr );
    }
    else
    {
      result.addr = 0;
      return static_cast<Tuple*>( result.addr );
    }
  }

public:
  pj2LocalInfo(Word _sa, Word _sb, Word _kai, Word _kbi,
               Word _fun, Supplier s)
  : streamA(_sa), streamB(_sb),
    tba(0), tbb(0), ita(0), itb(0),
    cta(0), ctb(0),
    endOfStream(false)
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
    if (machines && att > 0)
    {
      char hostName[255];
      int aSize = machines->getSize();

      do
      {
        string servName =
            ((CcString*)(machines->
                getElement((servIndex++) % aSize)).addr)->GetValue();

        //If the file is on local machine, don't delete it.
        memset(hostName, '\0', sizeof(hostName));
        if (0 == gethostname(hostName, sizeof(hostName)))
        {
          if (0 != strcmp(hostName, servName.c_str()))
          {
            FileSystem::DeleteFileOrFolder(filePath);
            system(("scp " + servName + rmDefaultPath + relName
                + " " + filePath).c_str());
          }
          else
            break;
        }
      }while((--att) > 0
          && !FileSystem::FileOrFolderExists(filePath));
    }

    if (!FileSystem::FileOrFolderExists(filePath))
    {
      cerr << "Error: File '" << filePath
           << "' doesn't exist!\n" << endl;
      return false;
    }
    else
    {
      tupleBlockFile = new ifstream(filePath.c_str(), ios::binary);
      if (!tupleBlockFile->good())
      {
        cerr << "Error accessing file '" << filePath << "'\n\n";
        tupleBlockFile = 0;
      }
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

#endif /* HADOOPPARALLELALGEBRA_H_ */
