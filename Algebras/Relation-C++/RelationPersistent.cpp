/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Relation Algebra

[1] Separate part of persistent data representation

June 1996 Claudia Freundorfer

May 2002 Frank Hoffmann port to C++

November 7, 2002 RHG Corrected the type mapping of ~tcount~.

November 30, 2002 RHG Introduced a function ~RelPersistValue~
instead of ~DefaultPersistValue~ which keeps relations that have
been built in memory in a small cache, so that they need not be
rebuilt from then on.

March 2003 Victor Almeida created the new Relational Algebra
organization

November 2004 M. Spiekermann. The declarations of the
PrivateRelation have been moved to the files RelationPersistent.h
and RelationMainMemory.h. This was necessary to implement some
little functions as inline functions.

June 2005 M. Spiekermann. The tuple's size information will now be i
stored in member variables and only recomputed after attributes
were changed. Changes in class ~TupleBuffer~ which allow to store
tuples as "free" or "non-free" tuples in it.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006 Victor Almeida replaced the ~free~ tuples concept to
reference counters. There are reference counters on tuples and also
on attributes. Some assertions were removed, since the code is
stable.

April 2006, M. Spiekermann. Introduction of a new function ~clearAll~ in class
~PrivateTupleBuffer~. This function calls the tuples' member functions
~DecReference~ and ~DeleteIfAllowed~ instead of deleting the tuple pointer
directly and is called by the destructor.

January 2007, M. Spiekermann. A memory leak in ~PrivateTupleBuffer~ has been
fixed.

April 2007, T Behr. The concept of solid tuples has been removed.

May 2007, M. Spiekermann. From now on the function TupleBuffer::Clear()
deletes a the pointer to a relation object and marks the buffer's state
as memory only.

June 2009, S.Jungnickel. Added implementation for classes ~TupleFile~ and
~TupleFileIterator~. Added implementation for new methods ~Save~ and ~Open~
of class ~Tuple~.

October 2009, S.Jungnickel. Constructor of TupleFileIterator now rewinds
read/write position of a TupleFile. Solved some problems when temporary closing
and reopening a ~TupleFile~.


[TOC]

1 Overview

The Relational Algebra basically implements two type constructors,
namely ~tuple~ and ~rel~.

More information about the Relational Algebra can be found in the
RelationAlgebra.h header file.

This file contains the implementation of the Persistent Relational
Algebra, where the type constructors ~tuple~ and ~rel~ are kept in
secondary memory.

A relation has two files: the tuple file and the LOB file, for
storing tuples and LOBs respectively.


2 Includes, Constants, Globals, Enumerations

*/


#include <string.h>

#include "RelationAlgebra.h"
#include "StringUtils.h"

#include "LogMsg.h"
#include "QueryProcessor.h"
#include "NestedList.h"
#include "SecondoSystem.h"
#include "SecondoSMI.h"
#include "../Tools/Flob/Flob.h"
//#include "FLOBCache.h"
#include "WinUnix.h"
#include "Symbols.h"
#include "StandardTypes.h"
#include "Serialize.h"
#include "FileSystem.h"

#include "Base64.h"

using namespace std;

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;


static Base64 B64;


/*
Output funtions

*/
ostream& operator<<(ostream& o, AttributeType& at){
  o << "[ algId =" << at.algId
      << ", typeId =" <<  at.typeId
      << ", size =" <<  at.size
      << ", offset =" <<  at.offset
      << "]";
  return o;
}

ostream& operator<<(ostream& o, const TupleType& tt){
  o << " noAttributes " << tt.noAttributes
      << " totalSize " <<  tt.totalSize
      << " coreSize " <<  tt.coreSize
      << " refs " << tt.refs
      << " AttrTypes ";
  for(int i=0 ; i< tt.noAttributes; i++){
    o << "[" << i << "]" << tt.attrTypeArray[i] << " , ";
  }
  return o;
}

ostream& operator<<(ostream& o, const RelationDescriptor& rd){
  o << "[ " "tupleType " << (*rd.tupleType) << ",  "
      << "noTuples " << rd.noTuples << ", "
      << "totalExtSize " << rd.totalExtSize<< ", "
      << "totalSize "  << rd.totalSize << ", "
      << "tupleFileId " << rd.tupleFileId << ", "
      << "lobFileId " << rd.lobFileId  << ", ";

  o << "attrExtSize [";
  for(unsigned int i=0; i< rd.attrExtSize.size(); i++){
    o << rd.attrExtSize[i] << ", ";
  }
  o << "]";

  o << "attrSize [";
  for(unsigned int i=0; i< rd.attrSize.size(); i++){
    o << rd.attrSize[i] << ", ";
  }
  o << "]";

  o << "]";

  return o;
}



char Tuple::tupleData[MAX_TUPLESIZE];

void
Attribute::counters(bool reset, bool show)
{
  StdTypes::UpdateBasicOps(reset);

  Counter::reportValue(Symbol::CTR_ATTR_BASIC_OPS(), show);
  Counter::reportValue(Symbol::CTR_ATTR_HASH_OPS(), show);
  Counter::reportValue(Symbol::CTR_ATTR_COMPARE_OPS(), show);
}

void
Attribute::InitCounters(bool show) {
  counters(true, show);
}

void
Attribute::SetCounterValues(bool show) {
  counters(false, show);
}


/*
3 Type constructor ~tuple~

3.2 Struct ~PrivateTuple~

This struct contains the private attributes of the class ~Tuple~.

*/

  Tuple::~Tuple()
  {
    DEBUG_MSG("Destructor called.")
    // delete all attributes if no further references exist
    for( int i = 0; i < noAttributes; i++ ){
      if( attributes[i] != 0 )
      {
        DEBUG_MSG("call attributes[" << i << "]->DeleteIfAllowed() with"
                    << " del.refs = " << (int)attributes[i]->del.refs)

        attributes[i]->DeleteIfAllowed();
        attributes[i] = 0;
      }
      else {
        DEBUG_MSG("attributes[" << i << "] == 0")
      }
    }
    if(noAttributes > MAX_NUM_OF_ATTR){
       delete[] attributes;
    }
    attributes=0;
    tupleType->DeleteIfAllowed();
    tupleType = 0;
    // do not delete the tuple file

    tuplesDeleted++;
    tuplesInMemory--;
  }
/*
The destructor.

*/


void Tuple::Save( SmiRecordFile* file,
                         const SmiFileId& fid,
                         double& extSize, double& size,
                         vector<double>& attrExtSize, vector<double>& attrSize,
                         const bool ignoreLobs /*=false*/)
{
  TRACE_ENTER

  this->tupleFile = file;
  this->lobFileId = fid;

#ifdef TRACE_ON
  static   long& saveCtr = Counter::getRef("RA:Tuple::Save");
  saveCtr++;
#endif

  // Calculate the size of the small FLOB data which will be
  // saved together with the tuple attributes and save the LOBs
  // in the lobFile.
  extSize += tupleType->GetCoreSize();
  size += tupleType->GetCoreSize();

  size_t coreSize = 0;

  // create a new record for the tuple
  DEBUG_MSG("Appending tuple record!")
  SmiRecord *tupleRecord = new SmiRecord();
//  SmiRecordId tupleId;
  bool rc = file->AppendRecord( tupleId, *tupleRecord );
  assert(rc == true);

  size_t extensionSize
             = CalculateBlockSize( coreSize, extSize,
                                   size, attrExtSize,
                                   attrSize);

  char* data = WriteToBlock(coreSize, extensionSize, ignoreLobs, file, fid);

  // Write data into the record
  DEBUG_MSG("Writing tuple record!")
  SHOW(coreSize)
  SHOW(extensionSize)

  uint16_t rootSize = coreSize + extensionSize;
  const size_t rootSizeLen = sizeof(rootSize);
  rc = tupleRecord->Write(data, rootSizeLen + rootSize, 0);
  assert(rc == true);

  // free allocated resources
  tupleRecord->Finish();
  delete tupleRecord;

  free(data);
  TRACE_LEAVE
}


void Tuple::Save(TupleFile& tuplefile)
{
  PinAttributes();
  double extSize = 0;
  double size = 0;
  vector<double> attrExtSize(tupleType->GetNoAttributes());
  vector<double> attrSize(tupleType->GetNoAttributes());

  // Calculate the size of the small FLOB data which will be
  // saved together with the tuple attributes and save the LOBs
  // in the lobFile.
  extSize += tupleType->GetCoreSize();
  size += tupleType->GetCoreSize();
  size_t coreSize = 0;
  size_t extensionSize = CalculateBlockSize( coreSize, extSize,
                                             size, attrExtSize,
                                             attrSize);

  // Put core and extension part into a single memory block
  // Note: this memory block contains already the block size
  // as uint16_t value
  char* data = WriteToBlock(coreSize, extensionSize, true, 0,0);


  // Append data to temporary tuple file
  tuplefile.Append(data, coreSize, extensionSize);

  // Free the allocated memory block
  free(data);
}


/*
~UpdateSave~ saves updated tuple data to disk.

*/
void Tuple::UpdateSave( const vector<int>& changedIndices,
                               double& extSize, double& size,
                               vector<double>& attrExtSize,
                               vector<double>& attrSize )
{
  size_t attrSizes = 0;
  size_t extensionSize
         = CalculateBlockSize( attrSizes, extSize,
                               size, attrExtSize, attrSize);

  char* data = WriteToBlock( attrSizes, extensionSize, false,
                             tupleFile, lobFileId );

  SmiRecord *tupleRecord = new SmiRecord();
  bool ok = tupleFile->SelectRecord( tupleId, *tupleRecord,
                                     SmiFile::Update );
  if (!ok)
  {
     cerr << "Fatal Error: Record for updated tuple not found!" << endl;
     cerr << "tupleId = " << tupleId << endl;
     assert (false);
  }
  const size_t oldRecordSize = tupleRecord->Size();

  uint16_t rootSize = attrSizes + extensionSize;
  const size_t rootSizeLen = sizeof(rootSize);
  const size_t bufSize = rootSizeLen + rootSize;

  SHOW(rootSize)
  SHOW(bufSize)
  SHOW(oldRecordSize)

  // The record must be truncated if its size decreased.
  if( bufSize < oldRecordSize ){
    tupleRecord->Truncate( bufSize );
  }

  // write the new data
  SmiSize len = 0;
  len = tupleRecord->Write(data, bufSize);
  assert(len == bufSize);

  tupleRecord->Finish();
  delete tupleRecord;

  free( data );
}



char* Tuple::WriteToBlock( size_t coreSize,
                           size_t extensionSize,
                           bool ignoreLobs,
                           SmiRecordFile* tuplefile,
                           const SmiFileId& lobFileId) const
{
 TRACE_ENTER
  // create a single block able to pick up the roots of the
  // attributes and all small FLOBs

  const uint16_t dataSize = coreSize + extensionSize;

  const size_t recordSizeLen = sizeof(dataSize);
  const size_t blockSize = dataSize + recordSizeLen;


  char* data = (char*) malloc( blockSize );

  WriteToBlock(data, coreSize, extensionSize,
      ignoreLobs, tuplefile, lobFileId);

  return data;
}

void Tuple::WriteToBlock(char* buf,
                         size_t coreSize,
                         size_t extensionSize,
                         bool ignoreLobs,
                         SmiRecordFile* file,
                         const SmiFileId& lobFileId,
                         bool containLOBs/* = false*/) const
{
  TRACE_ENTER
  assert(buf);

  const uint16_t dataSize = coreSize + extensionSize;

  char* data = buf;
  char* ext = data + sizeof(dataSize) + coreSize;

  char* lob = 0;
  SmiSize lobOffset = 0;
  if (containLOBs)
  {
    lobOffset = sizeof(dataSize) + coreSize + extensionSize;
    lob = data + lobOffset;
  }

  // current position in the core part
  SmiSize offset = 0;
  WriteVar<uint16_t>( dataSize, data, offset );

  // current position in the extension
  SmiSize extOffset = offset + coreSize;

     SHOW(offset)
     SHOW(extOffset)

  uint32_t currentSize = 0;
  uint32_t currentExtSize = 0;

  // collect all attributes into the memory block
  for( int i = 0; i < noAttributes; i++)
  {
    SHOW(attributes[i]->IsDefined())

    Attribute::StorageType st =  attributes[i]->GetStorageType();

    if (Attribute::Default == st)
    {
      //Write Flob data and adjust FlobIds if necessary

      // vector to remember old flob states
      vector<Flob> attrFlobs;

      for (int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
      {
        Flob *tmpFlob = attributes[i]->GetFLOB(j);
        SmiSize flobsz = tmpFlob->getSize();
        attrFlobs.push_back(*tmpFlob);

        if(!ignoreLobs && (flobsz >= extensionLimit))
        {
          if (!containLOBs)
          {
            DEBUG_MSG("tmpFlob->saveToFile");
            tmpFlob->saveToFile( lobFileId,(char)0, *tmpFlob );
          }
          else
          {
            //Write big Flobs to the flob area,
            //located in the end of the memory block
            assert(lob);

            // write data to flob area
            tmpFlob->read(lob, flobsz);

            SmiFileId fid(0);
            bool isTemp = true;
            if(tupleFile){
               fid = tupleFile->GetFileId();
               isTemp = tupleFile->IsTemp();
            }
            char mode = isTemp?1:0;
            Flob newFlob = Flob::createFrom(fid, tupleId,
                lobOffset, mode, flobsz);

            // change flob header
            *tmpFlob = newFlob;

            // update lob offset
            lobOffset += flobsz;
            lob += flobsz;
          }
        }
        else if (flobsz < extensionLimit)
        { // put small flobs to memory block too

          // write data to extension
          tmpFlob->read(ext, flobsz);

          SmiFileId fid(0);
          bool isTemp = true;
          if(tupleFile){
             fid = tupleFile->GetFileId();
             isTemp = tupleFile->IsTemp();
          }
          char mode = isTemp?1:0;
          Flob newFlob = Flob::createFrom( fid, tupleId,
                         extOffset, mode, flobsz );

          // change flob header
          *tmpFlob = newFlob;

          // update ext offset
          extOffset += flobsz;
          ext += flobsz;
        }

        SHOW(extOffset)
      }

      //Write attribute data
      currentSize = tupleType->GetAttributeType(i).size;
      SHOW(currentSize)

      DEBUG_MSG( Array2HexStr( (char*)attributes[i], currentSize) )
      attributes[i]->Serialize( data, currentSize, offset );
      offset += currentSize;

      // restore old flob data
      assert((size_t)attributes[i]->NumOfFLOBs() == attrFlobs.size());
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++) {
        Flob *tmpFlob = attributes[i]->GetFLOB(j);
        *tmpFlob = attrFlobs[j];
      }
      attrFlobs.clear();
    }
    else if (Attribute::Core == st)
    {
      assert( attributes[i]->NumOfFLOBs() == 0 );

      currentSize = attributes[i]->SerializedSize();
      attributes[i]->Serialize( data, currentSize, offset );
      offset += currentSize;
    }
    else if (Attribute::Extension == st)
    {
      assert( attributes[i]->NumOfFLOBs() == 0 );

      currentSize = sizeof(uint32_t);

      WriteVar<uint32_t>( extOffset, data, offset );
      SHOW(extOffset)

      currentExtSize = attributes[i]->SerializedSize();
      attributes[i]->Serialize(ext, currentExtSize, 0);
      DEBUG_MSG( Array2HexStr(ext, currentExtSize) )

      extOffset += currentExtSize;
      ext += currentExtSize;
    }
    else
    {
      cerr << "ERROR: unknown storage type for attribute No " << i << endl;
      assert(false);
    }

    SHOW(currentSize)
    SHOW(currentExtSize)

    SHOW(offset)
    SHOW(extOffset)
  }

  SHOW((void*)data)
  DEBUG_MSG( Array2HexStr(data, blockSize) )

  TRACE_LEAVE
}

void Tuple::WriteToDivBlock(char* buf, size_t coreSize,
    size_t extensionSize, size_t flobSize,
    SmiFileId flobFileId, SmiRecordId sourceDS, SmiSize& flobBlockOffset,
    map<pair<SmiFileId, SmiRecordId>, SmiSize>& flobIdCache) const
{
  TRACE_ENTER
  assert(buf);

  //Write block size (uint32)
  const uint16_t dataSize = coreSize + extensionSize;
  uint32_t blockSize = sizeof(uint32_t) + sizeof(u_int16_t) + dataSize;

  SmiSize offset = 0;
  WriteVar<uint32_t>(blockSize, buf, offset);
  char* data = buf + offset;

  //Write tuple size (uint16)
  offset = 0;
  WriteVar<uint16_t>(dataSize, data, offset);

  SmiSize extOffset = sizeof(dataSize) + coreSize;
  char* ext = data + extOffset;
  //The flob offset within the current block
  SmiSize lobOffset = extOffset + extensionSize;
  char* lob = data + lobOffset;

  uint32_t currentSize = 0;
  uint32_t currentExtSize = 0;
  char mode = 3;
  char smode = 1; //mode for small Flob data
  SmiRecordId rcdId = sourceDS;

  for (int i = 0; i < noAttributes; i++)
  {
    SHOW(attributes[i]->IsDefined())

    Attribute::StorageType st =  attributes[i]->GetStorageType();

    if (Attribute::Default == st)
    {
      vector<Flob> attrFlobs;

      for (int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
      {
        Flob *tmpFlob = attributes[i]->GetFLOB(j);
        SmiSize flobsz = tmpFlob->getSize();
        attrFlobs.push_back(*tmpFlob);

        if (flobsz >= extensionLimit)
        {
          // put big flobs to the flob part
          tmpFlob->read(lob, flobsz);

          SmiSize curFlobOffset = flobBlockOffset;
          bool writeCurrentFlob = true;
          if (tmpFlob->getMode() == 0){
            map<pair<SmiFileId, SmiRecordId>, SmiSize>::iterator lobId
              = flobIdCache.find(
                  make_pair(tmpFlob->getFileId(), tmpFlob->getRecordId()));

            if (lobId != flobIdCache.end()){
              curFlobOffset = lobId->second;
              writeCurrentFlob = false;
            } else {
              flobIdCache.insert(make_pair(
                  make_pair(tmpFlob->getFileId(), tmpFlob->getRecordId()),
                  curFlobOffset));
            }
          }

          // change the flob header which is exported into the block
          Flob newFlob = Flob::createFrom(
              flobFileId, rcdId, curFlobOffset, mode, flobsz);
          *tmpFlob = newFlob;
/*
The offset in the flob header is set differently, based on containsLob value.
If we export the whole tuple into one block, then the flob is always accessed
within the block, hence the offset is set as a relative position.
Or else, the flob is accessed separately in the flob file,
hence the offset should be set as a absolute position.

*/
          if (writeCurrentFlob){
            flobBlockOffset += flobsz;
            lob += flobsz;
          }
        }
        else if (flobsz < extensionLimit)
        {
          // put small flobs to the extension part
          tmpFlob->read(ext, flobsz);

          Flob newFlob = Flob::createFrom(
              flobFileId, rcdId, extOffset, smode, flobsz);
          *tmpFlob = newFlob;

          extOffset += flobsz;
          ext += flobsz;
        }
      }

      currentSize = tupleType->GetAttributeType(i).size;
      attributes[i]->Serialize(data, currentSize, offset);
      offset += currentSize;

      //restore old flob data
      assert((size_t)attributes[i]->NumOfFLOBs() == attrFlobs.size());
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++) {
        Flob *tmpFlob = attributes[i]->GetFLOB(j);
        *tmpFlob = attrFlobs[j];
      }
      attrFlobs.clear();
    }
    else if (Attribute::Core == st)
    {
      assert( attributes[i]->NumOfFLOBs() == 0 );

      currentSize = attributes[i]->SerializedSize();
      attributes[i]->Serialize( data, currentSize, offset );
      offset += currentSize;
    }
    else if (Attribute::Extension == st)
    {
      assert( attributes[i]->NumOfFLOBs() == 0 );

      currentSize = sizeof(uint32_t);

      WriteVar<uint32_t>( extOffset, data, offset );
      SHOW(extOffset)

      currentExtSize = attributes[i]->SerializedSize();
      attributes[i]->Serialize(ext, currentExtSize, 0);
      DEBUG_MSG( Array2HexStr(ext, currentExtSize) )

      extOffset += currentExtSize;
      ext += currentExtSize;
    }
    else
    {
      cerr << "ERROR: unknown storage type for attribute No " << i << endl;
      assert(false);
    }
  }

  TRACE_LEAVE
}

size_t Tuple::CalculateBlockSize( size_t& coreSize,
                                  double& extSize,
                                  double& size,
                                  vector<double>& attrExtSize,
                                  vector<double>& attrSize
                                 ) const
{
/*
These size values represent aggregate values for a single specific tuple.

----
coreSize = tupleType's size.
extensionSize = sum of each attribute's value size + (smallFlob sizes)
lobSize = sum of each attribute's bigFlob size
extSize += extensionSize
size += extensionSize + lobSize
return extensionSize
----

*/

  TRACE_ENTER
  int numOfAttr = tupleType->GetNoAttributes();
  assert( attrExtSize.size() == attrSize.size() );
  assert( (size_t)numOfAttr >= attrSize.size() );

  coreSize = tupleType->GetCoreSize();
  uint32_t extensionSize = 0;
  double   lobSize = 0.0;

  // Calculate the size of the small FLOB data which will be
  // saved together with the tuple attributes and save the LOBs
  // in the lobFile.
  for( int i = 0; i < numOfAttr; i++) {
    Attribute::StorageType st =  attributes[i]->GetStorageType();

    uint16_t currentSize = 0;
    size_t currentExtSize = 0;

    // check storage type
    if (st == Attribute::Default) {
      currentSize = tupleType->GetAttributeType(i).coreSize;
    }
    else if (st == Attribute::Core) {
      currentSize = attributes[i]->SerializedSize();
    }
    else if ( st == Attribute::Extension ) {
      currentSize = sizeof(uint32_t);
      currentExtSize = attributes[i]->SerializedSize();
    }
    else {
      cerr << "ERROR: unknown storage type for attribute No "
           << i << endl;
      assert(false);
    }

    extensionSize  += currentExtSize;
    attrExtSize[i] += (currentSize + currentExtSize);
    attrSize[i]    += (currentSize + currentExtSize);

         SHOW(currentSize)
         SHOW(currentExtSize)
         SHOW(extensionSize)

    // handle Flobs
    double attrLobSize = 0.0;
    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++) {
      Flob* tmpFlob = attributes[i]->GetFLOB(j);

      //assert( i >= 0 && (size_t)i < attrSize.size() );
      const SmiSize tmpFlobSize = tmpFlob->getSize();
      SHOW(tmpFlobSize)
      if (tmpFlobSize < extensionLimit){ // small Flobs
        extensionSize += tmpFlobSize;
        attrExtSize[i] += tmpFlobSize;
      } else { // big Flobs
        attrLobSize += tmpFlobSize;
      }
      attrSize[i] += tmpFlobSize;
    }
    lobSize += attrLobSize;
  } // for each attribute

  extSize += extensionSize;
  size += (extensionSize + lobSize);

  TRACE_LEAVE
  return extensionSize;
}



bool Tuple::Open( SmiRecordFile* tuplefile,
                  SmiFileId lobfileId,
                  SmiRecordId rid,
                  const bool dontReportError )
{
  tupleId = rid;
  this->tupleFile = tuplefile;
  this->lobFileId = lobfileId;
  SmiSize size;
  char* data = tupleFile->GetData(rid, size, dontReportError);
  if(!data){
    return false;
  }
  // the first two bytes in data contain the rootsize
  InitializeAttributes(data);
  free(data);
  return true;
}


bool Tuple::ReadFrom(SmiRecord& record){
  SmiSize size;
  char* data = record.GetData( size);
  if(!data){
    return false;
  }
  InitializeAttributes(data);
  free(data);
  return true;
}


bool Tuple::Open( SmiRecordFile* tuplefile,
                  SmiFileId lobfileId,
                  SmiRecord* record,
                  const bool dontReportError ) {
  SmiKey key;
  key = record->GetKey();
  key.GetKey( tupleId );
  return Open(tuplefile, lobFileId, tupleId, dontReportError);
}


bool Tuple::Open(TupleFileIterator *iter)
{
  if ( iter->MoreTuples() )
  {
    size_t rootSize;

    char* data = iter->ReadNextTuple(rootSize);

    if (data)
    {
      assert(rootSize < MAX_TUPLESIZE);
      InitializeAttributes(data);
      free(data);
      return true;
    }
  }

  return false;
}

#define FATAL_ERROR cerr << __FILE__ << "@" << __LINE__ << " - ERROR: "


char* Tuple::GetSMIBufferData(PrefetchingIterator* iter, uint16_t& rootSize)
{
  // read size
  const size_t rootSizeLen = sizeof(rootSize);

  if( (size_t)iter->ReadCurrentData(&rootSize, rootSizeLen, 0) != rootSizeLen )
  {
    FATAL_ERROR << "iter->ReadCurrentData(...) failed!" << endl;
    return 0;
  }

//#undef SHOW
//#define SHOW(a) {cerr << "  " << #a << " = " << a << endl;}
  SHOW(rootSizeLen)
  SHOW(rootSize)
//#define SHOW(a)

  // read all attributes
  assert(rootSize < MAX_TUPLESIZE);
  uint16_t wholesize = sizeof(rootSize) + rootSize;
  char* data = (char*) malloc ( wholesize );
  char* ptr = data + sizeof(rootSize);
  memcpy(data,&rootSize,sizeof(rootSize));

  uint16_t k = (uint16_t)iter->ReadCurrentData( ptr,
                                       rootSize,
                                       rootSizeLen);

  if( k != rootSize )
  {
    FATAL_ERROR << "iter->ReadCurrentData(...) failed!" << endl;
    cerr << " k of " << rootSize << " bytes read." << endl;
    return 0;
  }

  return data;
}



void Tuple::InitializeAttributes(char* src, bool containLOBs/* = false*/)
{
  TRACE_ENTER

  size_t offset = 0;

  // indicate the area holding tuple's big flobs
  size_t lobOffset = 0;
  if(containLOBs)
  {
    u_int16_t tupleSize;
    ReadVar<u_int16_t>(tupleSize, src, offset);
    lobOffset = sizeof(u_int16_t) + tupleSize;
  }
  offset = sizeof(u_int16_t);

    SHOW(offset)

  for(int i=0;i<noAttributes;i++){

    int algId = tupleType->GetAttributeType(i).algId;
    int typeId = tupleType->GetAttributeType(i).typeId;
    int sz = tupleType->GetAttributeType(i).size;

    // create an instance of the specified type, which gives
    // us an instance of a subclass of class Attribute.
    Attribute* attr =
              static_cast<Attribute*>( am->CreateObj(algId, typeId)(0).addr );

    if ( attr->GetStorageType() == Attribute::Extension )
    {
      // extension serialization
      uint32_t attrExtOffset = 0;
      ReadVar<uint32_t>( attrExtOffset, src, offset );
      SHOW(attrExtOffset)
      attributes[i] = Attribute::Create( attr, &src[attrExtOffset],
                                                            0, algId, typeId);

      SHOW(*attributes[i])
    }
    else
    {
      // serialization within root block
      attributes[i] = Attribute::Create( attr, &src[offset], sz, algId, typeId);

      int serialSize = attributes[i]->SerializedSize();
      int readData = (serialSize > 0) ? serialSize : sz;

      /*
        //debug
       if(offset+readData >wholesize + sizeof(uint16_t)){
         cout << "try to read " << sz << " bytes at offset  "
              << offset << " within a block of size " << wholesize
              << endl;
         assert(false);
      }
      */

      offset += readData;

    }

    SHOW(offset)
    SHOW(attributes[i]->IsDefined() )



    // create uncontrollable Flobs
    for(int k=0; k< attributes[i]->NumOfFLOBs();k++){
      Flob* flob = attributes[i]->GetFLOB(k);
      if(flob->getSize() < extensionLimit){
         Flob::createFromBlock(*flob, src + flob->getOffset(),
                               flob->getSize(), true);
      }
      else if(containLOBs)
      {
        assert(lobOffset > 0);
        Flob::createFromBlock(*flob, src + lobOffset,
                              flob->getSize(), true);
        flob->resize(flob->getSize());  //swap flob into a limited memory cache
        //resize function will make the ftext attribute be empty
        lobOffset += flob->getSize();
      }
    }
    // Call the Initialize function for every attribute
    // and initialize the reference counter
    attributes[i]->Initialize();
    attributes[i]->InitRefs();
  } // end for

  TRACE_LEAVE

}

void Tuple::InitializeSomeAttributes( const list<int>& aIds,
                                      char* src )
{
  TRACE_ENTER

  list<int>::const_iterator iter = aIds.begin();
  for( ; iter != aIds.end(); iter++ )
  {
    int i = *iter;
    DEBUG_MSG( "Attribute #" << i << endl )

    size_t offset = tupleType->GetAttributeType(i).offset;
    int algId = tupleType->GetAttributeType(i).algId;
    int typeId = tupleType->GetAttributeType(i).typeId;
    int sz = tupleType->GetAttributeType(i).size;

        SHOW(offset)
        SHOW(sz)

    // create an instance of the specified type, which gives
    // us an instance of a subclass of class Attribute.
    Attribute* attr =
              static_cast<Attribute*>( am->CreateObj(algId, typeId)(0).addr );

    if ( attr->GetStorageType() == Attribute::Extension )
    {
      uint32_t attrExtOffset = 0;
      ReadVar<uint32_t>( attrExtOffset, src, offset );

           SHOW(attrExtOffset)

      attributes[i] = Attribute::Create( attr, &src[attrExtOffset],
                                                            0, algId, typeId);
    }
    else
    {
      attributes[i] = Attribute::Create( attr, &src[offset], sz, algId, typeId);

      int serialSize = attributes[i]->SerializedSize();
      int readData = (serialSize > 0) ? serialSize : sz;

          SHOW(serialSize)
          SHOW(readData)
          DEBUG_MSG( Array2HexStr(src, readData, offset) )

      offset += readData;

      // Note: No special treatment for Flobs needed!
    }

    SHOW( attributes[i]->IsDefined() )

    // Call the Initialize function for every attribute
    // and initialize the reference counter
    attributes[i]->Initialize();
    attributes[i]->InitRefs();

  }

  TRACE_LEAVE
}

/*
Here the block contains only the data for the tuple,
without Flob and the length header that is a int16.
Therefore, the flobOffset should be reduced accordingly.

*/
void Tuple::InitializeNoFlobAttributes(char* src,
  string flobFile/* = ""*/, size_t flobOffset/* = 0*/)
{
  TRACE_ENTER

  size_t offset = 0;

  for (int i = 0; i < noAttributes; i++)
  {
    int algId = tupleType->GetAttributeType(i).algId;
    int typeId = tupleType->GetAttributeType(i).typeId;
    int sz = tupleType->GetAttributeType(i).size;

    // create an instance of the specified type, which gives
    // us an instance of a subclass of class Attribute.
    Attribute* attr =
              static_cast<Attribute*>( am->CreateObj(algId, typeId)(0).addr );

    if ( attr->GetStorageType() == Attribute::Extension )
    {
      // extension serialization
      uint32_t attrExtOffset = 0;
      ReadVar<uint32_t>( attrExtOffset, src, offset );
      SHOW(attrExtOffset)
      attrExtOffset -= sizeof(u_int16_t);
      attributes[i] = Attribute::Create( attr, &src[attrExtOffset],
                                                            0, algId, typeId);
    }
    else
    {
      // serialization within root block
      attributes[i] = Attribute::Create( attr, &src[offset], sz, algId, typeId);

      int serialSize = attributes[i]->SerializedSize();
      int readData = (serialSize > 0) ? serialSize : sz;
      offset += readData;
    }

    //process FLOBS, set their FlobID, instead of reading them out
    for (int k = 0; k < attributes[i]->NumOfFLOBs(); k++){
      Flob* flob = attributes[i]->GetFLOB(k);
      if(flob->getSize() < extensionLimit){
        Flob::createFromBlock(*flob,
                              src + flob->getOffset() - sizeof(u_int16_t),
                              flob->getSize(), true);
      }
      else{
        if (!flobFile.empty()){
          Flob::setExFile(*flob, flobFile, flob->getSize(), flobOffset);
        }
      }
    }
    // Call the Initialize function for every attribute
    // and initialize the reference counter
    attributes[i]->Initialize();
    attributes[i]->InitRefs();
  } // end for

  TRACE_LEAVE
}

void Tuple::readLocalFlobFile(const string flobFilePath)
{
  TRACE_ENTER

  for (int i = 0; i < noAttributes; i++)
  {
    for (int k = 0; k < attributes[i]->NumOfFLOBs(); k++)
    {
      Flob* flob = attributes[i]->GetFLOB(k);
      SmiSize bsize = flob->getSize();
      if (bsize >= extensionLimit){
        std::stringstream fileId;
        fileId << flob->getFileId();
        string flobFile = flobFilePath + fileId.str();

        Flob::readExFile(*flob, flobFile, bsize, flob->getOffset());
      }
    }
  }

  TRACE_LEAVE
}


bool Tuple::Open( SmiRecordFile *tuplefile,
                  SmiFileId lobfileId,
                  PrefetchingIterator *iter )
{
  TRACE_ENTER
  DEBUG_MSG("Open::Prefetch")

  iter->ReadCurrentRecordNumber( tupleId );
  this->tupleFile = tuplefile;
  this->lobFileId = lobfileId;

  uint16_t rootSize = 0;
  char* data = GetSMIBufferData(iter, rootSize);

  if (data) {
    InitializeAttributes(data);
    free ( data );
    return true;
  }
  else {
    return false;
  }

  TRACE_LEAVE
}



bool Tuple::OpenPartial( TupleType* newtype, const list<int>& attrIdList,
                         SmiRecordFile *tuplefile,
                         SmiFileId lobfileId,
                         PrefetchingIterator *iter )
{
  TRACE_ENTER
  DEBUG_MSG("OpenPartial using PrefetchingIterator")

  iter->ReadCurrentRecordNumber( tupleId );
  this->tupleFile = tuplefile;
  this->lobFileId = lobfileId;

  uint16_t rootSize = 0;
  char* data = GetSMIBufferData(iter, rootSize);

  if (data) {
    InitializeSomeAttributes(attrIdList, data);
    ChangeTupleType(newtype, attrIdList);
    free ( data );
    return true;
  }
  else {
    return false;
  }

  TRACE_LEAVE
}






/*
3.3 Implementation of the class ~Tuple~

This class implements the persistent representation of the type
constructor ~tuple~.

*/
Tuple *Tuple::RestoreFromList( ListExpr typeInfo, ListExpr value,
                               int errorPos, ListExpr& errorInfo,
                               bool& correct )
{
  return 0;
}

ListExpr Tuple::SaveToList( ListExpr typeInfo )
{
  return nl->TheEmptyList();
}

const TupleId& Tuple::GetTupleId() const
{
  return (TupleId&)tupleId;
}

void Tuple::SetTupleId( const TupleId& id )
{
  tupleId = (SmiRecordId)id;
}

void Tuple::PutAttribute( const int index, Attribute* attr )
{
  if( attributes[index] != 0 )
    attributes[index]->DeleteIfAllowed();
  attributes[index] = attr;

  recomputeExtSize = true;
  recomputeSize = true;
}

/*
The next function updates the tuple by replacing the old attributes at the positions given
by 'changedIndices' with the new ones from 'newAttrs'. If the old attribute
had FLOBs they are destroyed so that there is no garbage left on the disk.

*/
void Tuple::UpdateAttributes( const vector<int>& changedIndices,
                              const vector<Attribute*>& newAttrs,
                              double& extSize, double& size,
                              vector<double>& attrExtSize,
                              vector<double>& attrSize )
{
  int index;
  for ( size_t i = 0; i < changedIndices.size(); i++)
  {
    index = changedIndices[i];
    assert( index >= 0 && index < GetNoAttributes() );
    assert( attributes[index] != 0 );
    for (int j = 0;
         j < attributes[index]->NumOfFLOBs();
         j++)
    {
      Flob *tmpFlob = attributes[index]->GetFLOB(j);

      assert( index >= 0 && (size_t)index < attrSize.size() );
      SmiSize fsz = tmpFlob->getSize();

      attrSize[index] -=  fsz;
      size -= fsz;

      if( fsz < extensionLimit )
      {
        assert( index >= 0 && (size_t)index < attrExtSize.size() );
        attrExtSize[index] -= fsz;
        extSize -= fsz;
      }
      tmpFlob->destroy();
    }

    attributes[index]->DeleteIfAllowed();
    attributes[index] = newAttrs[i];
  }
  UpdateSave( changedIndices,
                            extSize, size,
                            attrExtSize, attrSize );

  recomputeExtSize = true;
  recomputeSize = true;
}

/*
For transporting complete tuples between different Secondo system
without change, transfer the tuple value into a transportable
string follow Base 64. Then the cost of the ~Out~ function
can be avoided.

In the contrast, construct a tuple object by reading this string,
can avoid the ~In~ function cost.

*/
string Tuple::WriteToBinStr()
{
  size_t coreSize = 0;
  size_t extensionSize = 0;
  size_t flobSize = 0;
  size_t blockSize = GetBlockSize(coreSize, extensionSize, flobSize);

  //The tuple block need two values to indicate the whole block size
  //and only the tuple's core and extension length.
  //blockSize += sizeof(blockSize) + sizeof(u_int16_t);
  char data[blockSize];
  WriteToBin(data, coreSize, extensionSize, flobSize);

  Base64 b64;
  string binStr;
  b64.encode(data, blockSize, binStr);

  return stringutils::replaceAll(binStr, "\n", "");
}

void Tuple::ReadFromBinStr(string binStr)
{
  Base64 b64;
  int sizeDecoded = b64.sizeDecoded(binStr.size());
  char bytes[sizeDecoded];
  int result = b64.decode( binStr, bytes );
  assert( result <= sizeDecoded );

  ReadFromBin(bytes);
}

/*
Put complete tuple's binary data into an allocated memory buffer,
including its big Flobs.
The head value of the block named ~blockSize~ is used to
indicate the whole block's size

----
blockSize = sizeof(blockSize) + tupleSize + flobSize
tupleSize = sizeof(tupleSize) + coreSize + extensionSize
----

*/
void Tuple::WriteToBin(char* buf,
            size_t coreSize, size_t extensionSize, size_t flobSize)
{
  u_int32_t blockSize =
      sizeof(u_int32_t) + sizeof(u_int16_t)
      + coreSize + extensionSize + flobSize;
  memcpy(buf, &blockSize, sizeof(blockSize));

  WriteToBlock(buf + sizeof(u_int32_t),
      coreSize, extensionSize,
      false, 0, 0, true);
}

/*
Put the tuple except its big flob data into an allocated memory buffer.
However, it still follows the format of the block that contains the flob,
therefore, the head value ~blockSize~ here is:

----
blockSize = sizeof(blockSize) + tupleSize
tupleSize = sizeof(tupleSize) + coreSize + extensionSize
----

*/
void Tuple::WriteTupleToBin(char* buf,
      size_t coreSize, size_t extensionSize)
{
  u_int32_t blockSize =
      sizeof(u_int32_t) + sizeof(u_int16_t)
      + coreSize + extensionSize;
  memcpy(buf, &blockSize, sizeof(blockSize));

  WriteToBlock(buf + sizeof(u_int32_t),
      coreSize, extensionSize,
      true, 0, 0, false);
}

size_t Tuple::GetBlockSize( size_t& coreSize,
                               size_t& extensionSize,
                               size_t& flobSize,
                               vector<double>* aes/* = 0*/,
                               vector<double>* as/* = 0*/) const
{
  // Get the memory block size of a single tuple, includes its big Flobs,
  // and head size values.

  double extSize = 0.0;
  double size = 0.0;
  vector<double> *attrExtSize = aes;
  vector<double> *attrSize = as;
  if (!attrExtSize)
    attrExtSize = new vector<double>(tupleType->GetNoAttributes());
  if (!attrSize)
    attrSize = new vector<double>(tupleType->GetNoAttributes());

//  vector<double> attrExtSize(tupleType->GetNoAttributes());
//  vector<double> attrSize(tupleType->GetNoAttributes());

  extSize = tupleType->GetCoreSize();
  size = tupleType->GetCoreSize();
  extensionSize = CalculateBlockSize( coreSize, extSize,
                                      size, *attrExtSize,
                                      *attrSize);

  if (!aes) delete attrExtSize;
  if (!as)  delete attrSize;

  flobSize = (size_t)size - extensionSize - tupleType->GetCoreSize();
  //size = tupleType's core size + extensionSize + flobSize (big flobs)
  return (size_t)size + sizeof(u_int32_t) + sizeof(u_int16_t);
}

/*
Read a tuple from a binary block, created by ~WriteToBin~ function.
The block contains a tuple's complete data, including its big Flobs.

*/
u_int32_t Tuple::ReadFromBin(char* buf, u_int32_t bSize/* = 0*/)
{
  assert(buf);

  u_int32_t blockSize;
  if (bSize == 0)
  {
    memcpy(&blockSize, buf, sizeof(blockSize));
    buf += sizeof(blockSize);

    InitializeAttributes(buf, true);
    return blockSize;
  }
  else
  {
    blockSize = bSize;
    u_int16_t tupleSize;
    size_t offset = 0;
    ReadVar<u_int16_t>(tupleSize, buf, offset);
    bool containLOBs =
        ( (u_int32_t)(tupleSize + sizeof(tupleSize)) != blockSize);

    InitializeAttributes(buf, containLOBs);
    return blockSize;
  }
}

/*
Prepare this function for ~ffeed2~ operator

Read the tuple from a binary block created by ~WriteToBin~.
Although the file data contains the whole tuple data including the Flob,
this function reads only its root data and leaves the Flob untouched.
Consequently, the created tuple sets the Flob mode to 2 and
links the Flob reference to the flobFile.

*/
u_int32_t Tuple::ReadTupleFromBin(char* buf, u_int32_t bSize,
    string flobFile, size_t flobOffset)
{
  assert(buf);
  InitializeNoFlobAttributes(buf, flobFile, flobOffset);
  return bSize;
}

void Tuple::ReadTupleFromBin(char* buf){
  assert(buf);
  InitializeNoFlobAttributes(buf);
}


TupleFileIterator::TupleFileIterator(TupleFile& f)
: tupleFile(f)
, data(0)
, size(0)
{
  // First close stream if it still open
  tupleFile.Close();

  // Open stream for reading
  tupleFile.stream = fopen((char*)tupleFile.pathName.c_str(), "rb");

  if( !tupleFile.stream )
  {
    cerr << "TupleFileIterator: Cannot open file '"
         << tupleFile.pathName << "' for binary reading!\n" << endl;
    return;
  }

  // reposition stream to beginning of file
  rewind(tupleFile.stream);

  if ( tupleFile.traceMode )
  {
    cmsg.info() << "TupleFile " << tupleFile.pathName
                << " opened for reading." << endl;
    cmsg.send();
  }

  // Read data of first tuple
  data = readData(size);
}

TupleFileIterator::~TupleFileIterator()
{
  tupleFile.Close();
}

Tuple* TupleFileIterator::GetNextTuple()
{
  if ( data )
  {
    Tuple* t = new Tuple((TupleType*)tupleFile.tupleType);

    // Read tuple data from disk buffer
    if ( t->Open(this) )
    {
      return t;
    }
    else
    {
      delete t;
      return 0;
    }
  }

  return 0;
}

char* TupleFileIterator::ReadNextTuple(size_t& size)
{
  if ( data )
  {
    char* tmp = data;

    size = this->size;

    // Read data of next tuple
    data = readData(this->size);

    return tmp;
  }
  else
  {
    size = 0;
    return 0;
  }
}

char* TupleFileIterator::readData(size_t& size)
{
  if ( tupleFile.stream )
  {
    uint16_t blockSize;

    // Read the size of the next data block
    size_t rc = fread(&blockSize, 1, sizeof(blockSize), tupleFile.stream);

    if ( feof(tupleFile.stream) )
    {
      size = 0;
      return 0;
    }
    else if ( rc < sizeof(blockSize) )
    {
      cerr << "TupleFileIterator::ReadNextTuple: error "
           << "reading data block size in file '"
           << tupleFile.pathName << "'!\n" << endl;
      size = 0;
      return 0;
    }

    size = sizeof(blockSize) + blockSize;

    // Allocate a single memory block for data block size and tuple data
    char* data = (char*)malloc(size);

    // Store block size in memory block
    memcpy(data,&blockSize,sizeof(blockSize));

    // Read tuple data into memory block
    rc = fread(data + sizeof(blockSize), 1, blockSize, tupleFile.stream);


    if ( rc < blockSize )
    {
      cerr << "TupleFileIterator::ReadNextTuple: error "
           << "reading tuple data block in file '"
           << tupleFile.pathName << "'!\n" << endl;
      size = 0;
      return 0;
    }

    return data;
  }
  else
  {
    size = 0;
    return 0;
  }
}

bool TupleFile::traceMode = false;

TupleFile::TupleFile( TupleType* tupleType,
                        const size_t bufferSize )
: tupleType(tupleType)
, bufferSize(bufferSize)
, stream(0)
, tupleCount(0)
, totalSize(0)
, totalExtSize(0)
{
  this->tupleType->IncReference();

  // create a unique temporary filename
  string str = FileSystem::GetCurrentFolder();
  FileSystem::AppendItem(str, "tmp");
  FileSystem::AppendItem(str, "TF");
  pathName = FileSystem::MakeTemp(str);

  if ( traceMode )
  {
    cmsg.info() << "TupleFile " << pathName << " created." << endl;
    cmsg.send();
  }
}

TupleFile::TupleFile( TupleType* tupleType,
                        string pathName,
                        const size_t bufferSize )
: tupleType(tupleType)
, pathName(pathName)
, bufferSize(bufferSize)
, stream(0)
, tupleCount(0)
, totalSize(0)
, totalExtSize(0)
{
  this->tupleType->IncReference();

  if ( pathName == "" )
  {
    // create a unique temporary filename
    string str = FileSystem::GetCurrentFolder();
    FileSystem::AppendItem(str, "tmp");
    FileSystem::AppendItem(str, "TF");
    pathName = FileSystem::MakeTemp(str);
  }

  if ( traceMode )
  {
    cmsg.info() << "TupleFile " << pathName << " created." << endl;
    cmsg.send();
  }
}

TupleFile::~TupleFile()
{
  // close stream if still open
  Close();

  // delete reference to tuple type
  tupleType->DeleteIfAllowed();

  // delete temporary disk file
  FileSystem::DeleteFileOrFolder(pathName);

  if ( traceMode )
  {
    cmsg.info() << "TupleFile " << pathName << " deleted." << endl;
    cmsg.send();
  }
}

bool TupleFile::Open()
{
  if ( tupleCount == 0 )
  {
    // Open fresh binary stream
    stream = fopen(pathName.c_str(), "w+b" );

    if ( traceMode )
    {
      cmsg.info() << "TupleFile opened (w+b)." << endl;
      cmsg.send();
    }
  }
  else
  {
    // Append to existing file if tupleCount > 0
    stream = fopen(pathName.c_str(), "a+b" );

    if ( traceMode )
    {
      cmsg.info() << "TupleFile opened (a+b)" << endl;
      cmsg.send();
    }
  }

  if( !stream )
  {
    cerr << "TupleFile::Open(): Cannot open file '"
         << pathName << "'!\n" << endl;
    cerr << "TupleCount: " << tupleCount << endl;
    return false;
  }

  // Set stream buffer size
  if ( setvbuf(stream, 0, bufferSize >= 2 ? _IOFBF : _IONBF, bufferSize) != 0 )
  {
    cerr << "TupleFile::Open(): illegal buffer type or size specified '"
         << bufferSize << "'!\n" << endl;
    return false;
  }

  if ( traceMode )
  {
    cmsg.info() << "TupleFile " << pathName << " opened for writing." << endl;
    cmsg.send();
  }

  return true;
}

void TupleFile::Close()
{
  if ( stream != 0 )
  {
    if ( fclose(stream) == EOF )
    {
      cerr << "TupleFile::Close(): error while closing file '"
           << pathName << "'!\n" << endl;
    }
    stream = 0;

    if ( traceMode )
    {
      cmsg.info() << "TupleFile " << pathName << " closed." << endl;
      cmsg.send();
    }
  }
}

void TupleFile::Append(Tuple* t)
{
  t->Save(*this);
}

void TupleFile::Append(char *data, size_t core, size_t ext)
{
  if ( stream == 0 )
  {
    this->Open();
  }

  uint16_t size = core + ext;


  size_t rc = fwrite(data, 1, sizeof(size) + size, stream);

  // check the number of written
  if ( rc < sizeof(size) + size )
  {
    cerr << "TupleFile::Append(): error writing to file '"
         << pathName << "'!\n" << endl;
    cerr << "(" << sizeof(size) + size
         << ") bytes should be written, but only ("
         << rc << ") bytes were written" "!\n" << endl;
    return;
  }

  fflush(stream);

  tupleCount++;
  totalSize += size;
  totalExtSize += ext;

  return;
}

TupleFileIterator* TupleFile::MakeScan()
{
  return new TupleFileIterator(*this);
}

/*
3.9 Class ~TupleBuffer~

This class is used to collect tuples for sorting, for example, or
to do a cartesian product. In this persistent version, if the buffer
is small it will be stored in memory and if it exceeds the allowed size,
the current buffer contents will be flushed to disk.

The Iterator will first retrieve the tuples on disk and afterwards the remaining
tuples which reside in memory.

*/

TupleBuffer::TupleBuffer( const size_t maxMemorySize ):
  MAX_MEMORY_SIZE( maxMemorySize ),
  diskBuffer( 0 ),
  inMemory( true ),
  traceFlag( RTFlag::isActive("RA:TupleBufferInfo") ),
  totalMemSize( 0 ),
  totalExtSize( 0),
  totalSize( 0 )
  {
    if (traceFlag)
    {
      cmsg.info() << "New Instance of TupleBuffer with size "
                  << maxMemorySize / 1024 << "kb "
                  << " address = " << (void*)this << endl;
      cmsg.send();
    }
  }

/*
The constructor.

*/

TupleBuffer::~TupleBuffer()
{
  updateDataStatistics();
  clearAll();
  if( !inMemory ) {
    diskBuffer->DeleteAndTruncate();
    diskBuffer = 0;
  }
}

void TupleBuffer::clearAll()
{
   for( TupleVec::iterator it = memoryBuffer.begin();
        it != memoryBuffer.end(); it++ )
   {
     //cout << (void*) *it << " - " << (*it)->GetNumOfRefs() << endl;
     //if (*it != 0) {
       (*it)->DeleteIfAllowed();
     //}
   }
   memoryBuffer.clear();
   totalSize=0;
}

/*
The destructor.

*/

int TupleBuffer::GetNoTuples() const
{
  if( inMemory )
    return memoryBuffer.size();
  else
    return diskBuffer->GetNoTuples();
}

size_t TupleBuffer::FreeBytes() const
{
  if (MAX_MEMORY_SIZE > totalMemSize) {
    return MAX_MEMORY_SIZE - static_cast<size_t>( ceil(totalMemSize) );
  } else {
    return 0;
  }
}

double TupleBuffer::GetTotalRootSize() const
{
  if( IsEmpty() )
    return 0;

  if (inMemory)
    return GetNoTuples() * memoryBuffer[0]->GetRootSize();
  else
    return diskBuffer->GetTupleType()->GetTotalSize();
}


double TupleBuffer::GetTotalRootSize(int i) const
{
  if( IsEmpty() )
    return 0;

  if (inMemory)
    return memoryBuffer[0]->GetRootSize(i);
  else
    return diskBuffer->GetTupleType()->GetTotalSize();
}

double TupleBuffer::GetTotalExtSize() const
{
  if( inMemory )
    return totalExtSize;
  else
    return diskBuffer->GetTotalExtSize();
}

double TupleBuffer::GetTotalExtSize(int i) const
{
  if( IsEmpty() )
    return 0;

  if( inMemory )
    return memoryBuffer[0]->GetExtSize(i);
  else
    return diskBuffer->GetTotalExtSize(i);
}


double TupleBuffer::GetTotalSize() const
{
  if( inMemory )
    return totalSize;
  else
    return diskBuffer->GetTotalSize();
}

double TupleBuffer::GetTotalSize(int i) const
{
  if( IsEmpty() )
    return 0;

  if( inMemory )
    return memoryBuffer[0]->GetSize(i);
  else
    return diskBuffer->GetTotalSize(i);
}

void
TupleBuffer::updateDataStatistics() {

  static long& writtenData_Bytes = Counter::getRef(Symbol::CTR_TBUF_BYTES_W());
  static long& writtenData_Pages = Counter::getRef(Symbol::CTR_TBUF_PAGES_W());

  if (diskBuffer)
    writtenData_Bytes += (long)ceil( diskBuffer->GetTotalExtSize() );

  writtenData_Pages = writtenData_Bytes / WinUnix::getPageSize();
}

bool TupleBuffer::IsEmpty() const
{
  if( inMemory )
    return memoryBuffer.empty();
  else
    return false;
}

void TupleBuffer::Clear()
{
  updateDataStatistics();
  if( inMemory )
  {
    clearAll();
  }
  else
  {
    diskBuffer->DeleteAndTruncate();
    diskBuffer = 0;
    inMemory = true;
  }
}

void TupleBuffer::AppendTuple( Tuple *t )
{

  static long& appendCalls = Counter::getRef("RA:TupleBuf:diskBufferAppend");
  if( inMemory )
  {
    if( totalMemSize + t->GetMemSize() <=
        MAX_MEMORY_SIZE )
    {
      t->IncReference();
      memoryBuffer.push_back( t );
      totalMemSize += t->GetMemSize();
      totalSize += t->GetSize();
      totalExtSize += t->GetExtSize();
    }
    else
    {
      if (traceFlag)
      {
        cmsg.info() << "Changing TupleBuffer's state from inMemory "
                    << "-> !inMemory" << endl;
        cmsg.send();
      }
      diskBuffer =
        new Relation( t->GetTupleType(), true );

      vector<Tuple*>::iterator iter =
        memoryBuffer.begin();
      while( iter != memoryBuffer.end() )
      {
        Tuple* tuple = *iter;
        tuple->PinAttributes();
        diskBuffer->AppendTupleNoLOBs( tuple );
        appendCalls++;
        tuple->DeleteIfAllowed();
        iter++;
      }
      memoryBuffer.clear();
      totalMemSize = 0;
      totalExtSize = 0;
      totalSize = 0;
      t->PinAttributes();
      diskBuffer->AppendTupleNoLOBs( t );
      appendCalls++;
      inMemory = false;
    }
  }
  else
  {
    t->PinAttributes();
    return diskBuffer->AppendTupleNoLOBs( t );
  }
}

Tuple *TupleBuffer::GetTuple( const TupleId& id,
                              const bool dontReportError ) const
{
  if( inMemory )
  {
    Tuple* res =  GetTupleAtPos( id );
    res->IncReference();
    return res;
  }
  else
  {
    return diskBuffer->GetTuple( id+1, dontReportError );
  }
}

Tuple* TupleBuffer::GetTupleAtPos( const size_t pos ) const
{
  if( inMemory )
  {
    if( pos >= 0
        && pos < memoryBuffer.size()
        && memoryBuffer[pos] != 0 )
    {
      return memoryBuffer[pos];
    }
    return 0;
  }
  return 0;
}

bool TupleBuffer::SetTupleAtPos( const size_t pos, Tuple* t)
{
  if( inMemory )
  {
    if( pos >= 0 && pos < memoryBuffer.size() )
    {
      memoryBuffer[pos] = t;
    }
    return true;
  }
  return false;
}

GenericRelationIterator *TupleBuffer::MakeScan() const
{
  return new TupleBufferIterator( *this );
}

GenericRelationIterator *TupleBuffer::MakeScan(TupleType* tt) const
{
  return new TupleBufferIterator( *this, tt);
}

bool TupleBuffer::GetTupleFileStats( SmiStatResultType &result )
{
  if( !inMemory && diskBuffer->GetTupleFileStats(result) ){
    result.push_back(pair<string,string>("FilePurpose",
              "TupleBufferTupleCoreFile"));
  }
  return true;
}


bool TupleBuffer::GetLOBFileStats( SmiStatResultType &result )
{
  if( !inMemory && diskBuffer->GetLOBFileStats(result) ){
    result.push_back(pair<string,string>("FilePurpose",
              "TupleBufferTupleLOBFile"));
  }
  return true;
}



/*
3.9 Class ~CircularTupleBuffer~

This class is used to collect tuples for sliding window operators. In this
persistent version, it is decided during the buffer construction, whether it
fits into main memory or it will be stored in disk.

*/

CircularTupleBuffer::CircularTupleBuffer(
    bool _inMemory, TupleType* tT, unsigned int noTuples):
    inMemory(_inMemory),
    diskBuffer( 0 ),
    overWrite(false),
    maxNoTuples(noTuples),
    totalExtSize( 0),
    totalSize( 0 ),
    totalMemSize( 0 )
{
  if(! _inMemory)
  {
    diskBuffer = new Relation( tT, true );
    overWrite= false;
  }
}
/*
The constructor.

*/

CircularTupleBuffer::~CircularTupleBuffer()
{
  clearMemory();
  if( !inMemory ) {
    diskBuffer->DeleteAndTruncate();
    diskBuffer = 0;
  }
}

void CircularTupleBuffer::clearMemory()
{
   for( deque<Tuple*>::iterator it = memoryBuffer.begin();
        it != memoryBuffer.end(); it++ )
   {
     //cout << (void*) *it << " - " << (*it)->GetNumOfRefs() << endl;
     //if (*it != 0) {
       (*it)->DeleteIfAllowed();
     //}
   }
   memoryBuffer.clear();
   totalSize=0;
}

/*
The destructor.

*/

int CircularTupleBuffer::GetNoTuples() const
{
  if( inMemory )
    return memoryBuffer.size();
  else
    return diskBuffer->GetNoTuples();
}

double CircularTupleBuffer::GetTotalRootSize() const
{
  if( GetNoTuples() == 0 )
    return 0;

  if (inMemory)
    return GetNoTuples() * memoryBuffer[0]->GetRootSize();
  else
    return diskBuffer->GetTupleType()->GetTotalSize();
}


double CircularTupleBuffer::GetTotalRootSize(int i) const
{
  if( GetNoTuples() == 0 )
    return 0;

  if (inMemory)
    return memoryBuffer[0]->GetRootSize(i);
  else
    return diskBuffer->GetTupleType()->GetTotalSize();
}

double CircularTupleBuffer::GetTotalExtSize() const
{
  if( inMemory )
    return totalExtSize;
  else
    return diskBuffer->GetTotalExtSize();
}

double CircularTupleBuffer::GetTotalExtSize(int i) const
{
  if( GetNoTuples() == 0 )
    return 0;

  if( inMemory )
    return memoryBuffer[0]->GetExtSize(i);
  else
    return diskBuffer->GetTotalExtSize(i);
}


double CircularTupleBuffer::GetTotalSize() const
{
  if( inMemory )
    return totalSize;
  else
    return diskBuffer->GetTotalSize();
}

double CircularTupleBuffer::GetTotalSize(int i) const
{
  if( GetNoTuples() == 0 )
    return 0;

  if( inMemory )
    return memoryBuffer[0]->GetSize(i);
  else
    return diskBuffer->GetTotalSize(i);
}

void CircularTupleBuffer::Clear()
{
  if( inMemory )
  {
    clearMemory();
  }
  else
  {
    diskBuffer->DeleteAndTruncate();
    diskBuffer = 0;
  }
}

void CircularTupleBuffer::AppendTuple( Tuple *t )
{
  if( inMemory )
  {
    t->IncReference();
    if(memoryBuffer.size() < maxNoTuples)
      memoryBuffer.push_back( t );
    else
    {
      memoryBuffer.front()->DeleteIfAllowed();
      memoryBuffer.pop_front();
      memoryBuffer.push_back( t );
    }
    totalMemSize += t->GetMemSize();
    totalSize += t->GetSize();
    totalExtSize += t->GetExtSize();

  }
  else
  {
//    t->PinAttributes();
//    if(! overWrite && (GetNoTuples() < maxNoTuples))
//      diskBuffer->AppendTupleNoLOBs( t );
//    else if(! overWrite && (GetNoTuples() == maxNoTuples))
//    {
//      overWrite= true;
//      writeIterator= diskBuffer->MakeScan();
////      readIterator= diskBuffer->MakeScan();
////      readIterator->GetNextTuple();
//    }
//
//    if(overWrite)
//    {
//      //Advance the writeIterator
//      Tuple* victim= writeIterator->GetNextTuple();
//      if(writeIterator->EndOfScan())
//      {
//        delete writeIterator;
//        writeIterator= diskBuffer->MakeScan();
//        victim= writeIterator->GetNextTuple();
//      }
//    }
  }
}

Tuple *CircularTupleBuffer::GetTuple( const TupleId& id,
                              const bool dontReportError ) const
{
  if( inMemory )
  {
    Tuple* res =  GetTupleAtPos( id );
    res->IncReference();
    return res;
  }
  else
  {

  }
  return 0;
}

Tuple *CircularTupleBuffer::GetTuple( const TupleId& id,
                              const int attrIndex,
                              const vector< pair<int, int> >& intervals,
                              const bool dontReportError ) const
{
  Tuple *t = 0;
  if( (t = GetTuple( id, dontReportError )) != 0 )
    t->GetAttribute( attrIndex )->Restrict( intervals );
  return t;
}

Tuple* CircularTupleBuffer::GetTupleAtPos( const size_t pos ) const
{
  if( inMemory )
  {
    if( pos >= 0
        && pos < memoryBuffer.size()
        && memoryBuffer[pos] != 0 )
    {
      return memoryBuffer[pos];
    }
    return 0;
  }
  return 0;
}

bool CircularTupleBuffer::SetTupleAtPos( const size_t pos, Tuple* t)
{
  if( inMemory )
  {
    if( pos >= 0 && pos < memoryBuffer.size() )
    {
      memoryBuffer[pos] = t;
    }
    return true;
  }
  return false;
}

GenericRelationIterator *CircularTupleBuffer::MakeScan() const
{
  return new CircularTupleBufferIterator( *this );
}

GenericRelationIterator *CircularTupleBuffer::MakeScan(TupleType* tt) const
{
  return new CircularTupleBufferIterator( *this, tt);
}

bool CircularTupleBuffer::GetTupleFileStats( SmiStatResultType &result )
{
  return true;
}


bool CircularTupleBuffer::GetLOBFileStats( SmiStatResultType &result )
{
  return true;
}


bool CircularTupleBuffer::DeleteTuple(Tuple* t)
{
  //Not implemented
  return false;
}
/*
The function that deletes the given Tuple from the relation

*/
void CircularTupleBuffer::UpdateTuple( Tuple *tuple,
                        const vector<int>& changedIndices,
                        const vector<Attribute *>& newAttrs )
{
  //Not implemented
  bool Implemented= false;
  assert(Implemented);
}


/*
3.9.3 ~TupleBufferIterator~

*/
TupleBufferIterator::TupleBufferIterator( const TupleBuffer& tupleBuffer ):
  readData_Bytes( Counter::getRef(Symbol::CTR_TBUF_BYTES_R()) ),
  readData_Pages( Counter::getRef(Symbol::CTR_TBUF_PAGES_R()) ),
  tupleBuffer( tupleBuffer ),
  currentTuple( 0 ),
  diskIterator(
    tupleBuffer.inMemory ?
      0 :
      tupleBuffer.diskBuffer->MakeScan() )
  {}

TupleBufferIterator::TupleBufferIterator(
    const TupleBuffer& tupleBuffer, TupleType* tt ):
    readData_Bytes( Counter::getRef(Symbol::CTR_TBUF_BYTES_R()) ),
    readData_Pages( Counter::getRef(Symbol::CTR_TBUF_PAGES_R()) ),
  tupleBuffer( tupleBuffer ),
  currentTuple( 0 ),
  diskIterator(
    tupleBuffer.inMemory ?
      0 :
      tupleBuffer.diskBuffer->MakeScan() ),
  outtype( tt )
  {}
/*
The constructor.

*/
TupleBufferIterator::~TupleBufferIterator()
{
  readData_Pages = readData_Bytes / WinUnix::getPageSize();
  delete diskIterator;
}
/*
The destructor.

*/

Tuple *TupleBufferIterator::GetNextTuple()
{
  if( diskIterator )
  {
    Tuple* t = diskIterator->GetNextTuple();
    if (t)
      readData_Bytes += t->GetExtSize();
    return t;
  }
  else
  {
    if( currentTuple == tupleBuffer.memoryBuffer.size() )
      return 0;
    Tuple *result =
      tupleBuffer.memoryBuffer[currentTuple];
    result->IncReference();
    currentTuple++;
    return result;
  }
}

Tuple *TupleBufferIterator::GetNextTuple(const list<int>& attrList)
{
  if( diskIterator )
  {
    Tuple* t = diskIterator->GetNextTuple(attrList);
    if (t)
      readData_Bytes += t->GetExtSize();
    return t;
  }
  else
  {
    if( currentTuple == tupleBuffer.memoryBuffer.size() )
      return 0;

    Tuple *t =
      tupleBuffer.memoryBuffer[currentTuple];

    Tuple *result = new Tuple( outtype );
    list<int>::const_iterator iter = attrList.begin();
    for(int i=0 ; iter != attrList.end(); ++iter, ++i )
      result->CopyAttribute(*iter, t, i);
    currentTuple++;
    return result;
  }
}

TupleId TupleBufferIterator::GetTupleId() const
{
  if( diskIterator )
  {
    return diskIterator->GetTupleId();
  }
  else
  {
    return currentTuple-1;
  }
}


/*
3.9.3 ~CircularTupleBufferIterator~

*/
CircularTupleBufferIterator::CircularTupleBufferIterator(
    const CircularTupleBuffer& _tupleBuffer ):
  tupleBuffer( _tupleBuffer )
  {
    currentTuple= tupleBuffer.memoryBuffer.begin() ;
  }

CircularTupleBufferIterator::CircularTupleBufferIterator(
    const CircularTupleBuffer& _tupleBuffer, TupleType* tt ):
  tupleBuffer( _tupleBuffer ),
  outtype( tt )
{
  currentTuple= tupleBuffer.memoryBuffer.begin();
}
/*
The constructor.

*/
CircularTupleBufferIterator::~CircularTupleBufferIterator()
{

}
/*
The destructor.

*/

Tuple *CircularTupleBufferIterator::GetNextTuple()
{
  if( currentTuple == tupleBuffer.memoryBuffer.end() )
      return 0;

  Tuple *result = *currentTuple;
  result->IncReference();
  ++currentTuple;
  return result;
}

Tuple *CircularTupleBufferIterator::GetNextTuple(const list<int>& attrList)
{
  if( currentTuple == tupleBuffer.memoryBuffer.end() )
    return 0;

  Tuple *t = *currentTuple;

  Tuple *result = new Tuple( outtype );
  list<int>::const_iterator iter = attrList.begin();
  for(int i=0 ; iter != attrList.end(); ++iter, ++i )
    result->CopyAttribute(*iter, t, i);
  ++currentTuple;
  return result;
}

TupleId CircularTupleBufferIterator::GetTupleId() const
{
  return (*currentTuple)->GetTupleId();
}


RandomTBuf::RandomTBuf(size_t setSize /*= default*/)
  : subsetSize(setSize),
    streamPos(0),
    run(0),
    trace(false),
    memBuf(subsetSize,0)
{}


Tuple* RandomTBuf::ReplacedByRandom(Tuple* s, size_t& i, bool& replaced)
{
    Tuple* t = 0;
    replaced = false;

    i = streamPos % subsetSize;
    if ( i == 0 )
    {
     run++;

     if (trace) {

       cerr << endl
            << "subsetSize: " << subsetSize
            << ", run: " << run
            << ", replaced slots:"
            << endl;
     }
    }

    if (run > 0)
    {
      int r = WinUnix::rand(run);
      if (trace) {
        cerr << ", r = " << r;
      }

      assert( (r >= 1) && (r <= run) );

      //cout << "s:" << *s << endl;
      if ( r == run )
      {
        if (trace) {
          cerr << ", i = " << i;
        }

        // tuple s will be selected for the front part
        // of the output stream. The currently stored tuple
        // t at slot i will be released.
        replaced = true;

        t = memBuf[i];
        if (t != 0) {
          //cout << "t:" << *t << endl;
          t->DeleteIfAllowed();
        }
        s->IncReference();
        memBuf[i] = s;
      }
    } // end of run > 0

    streamPos++;
    return t;
}

void RandomTBuf::copy2TupleBuf(TupleBuffer& tb)
{
    for(iterator it = begin(); it != end(); it++) {
      if (*it != 0) {
        (*it)->DeleteIfAllowed();
        tb.AppendTuple(*it);
      }
    }
}


/*
4 Type constructor ~rel~

4.2 Implementation of the class ~Relation~

This class implements the persistent representation of the type
constructor ~rel~.

*/
map<SmiFileId,Relation*> Relation::pointerTable;


void
Relation::InitFiles( bool open /*= false */) {

  bool rc = false;
  if (open) {
    rc = tupleFile.Open(relDesc.tupleFileId);
  } else {
    rc = tupleFile.Create();
  }

  if( !rc )
  {
    string error;
    SmiEnvironment::GetLastErrorCode( error );
    cerr << error << endl;
    assert( false );
  }
  relDesc.tupleFileId = tupleFile.GetFileId();


  if( pointerTable.find( relDesc.tupleFileId ) ==
                         pointerTable.end() )
    pointerTable.insert( make_pair( relDesc.tupleFileId,
                                    this ) );

  // init LOB File
  if (relDesc.tupleType->NumOfFlobs() > 0 && relDesc.lobFileId == 0)
  {
    SmiRecordFile rf(false,0, relDesc.isTemp);
    if(!rf.Create()){
      assert(false);
    }
    relDesc.lobFileId = rf.GetFileId();
    rf.Close();
  }
}


Relation::Relation( const ListExpr typeInfo, bool isTemp ):
  relDesc( typeInfo, isTemp ),
  tupleFile( false, 0, isTemp )
{
  Relation::InitFiles();
}

Relation::Relation( TupleType *tupleType, bool isTemp ):
  relDesc( tupleType, isTemp ),
  tupleFile( false, 0, isTemp )
{
  Relation::InitFiles();
}

Relation::Relation( const RelationDescriptor& relDesc ):
  relDesc( relDesc ),
  tupleFile( false, 0, relDesc.isTemp )
{
  Relation::InitFiles(true);
}

Relation::~Relation()
{
  //delete privateRelation;
  tupleFile.Close();
}


Relation *
Relation::RestoreFromList( ListExpr typeInfo, ListExpr value,
                           int errorPos, ListExpr& errorInfo,
                           bool& correct )
{
  ListExpr rest = value;
  for( int i = 0; i < 3; i++ )
    rest = nl->Rest( rest );

  int n = (nl->ListLength( rest )-2)/2;
  vector<double> attrExtSize( n ),
                 attrSize( n );

  for( int i = 0; i < n; i++  )
  {
    attrExtSize[i] = nl->RealValue( nl->First( rest ) );
    attrSize[i] = nl->RealValue( nl->Second( rest ) );
    rest = nl->Rest( rest );
    rest = nl->Rest( rest );
  }

  RelationDescriptor relDesc( typeInfo,
                              nl->IntValue( nl->First( value ) ),
                              nl->RealValue( nl->Second( value ) ),
                              nl->RealValue( nl->Third( value ) ),
                              attrExtSize, attrSize,
                              nl->IntValue( nl->First( rest ) ),
                              nl->IntValue( nl->Second( rest ) ) );

  return new Relation( relDesc );
}

ListExpr
Relation::SaveToList( ListExpr typeInfo )
{
  ListExpr result =
    nl->OneElemList( nl->IntAtom( relDesc.noTuples ) ),
           last = result;

  last =
    nl->Append( last, nl->RealAtom( relDesc.totalExtSize ) );
  last =
    nl->Append( last, nl->RealAtom( relDesc.totalSize ) );

  for( int i = 0;
       i < relDesc.tupleType->GetNoAttributes();
       i++ )
  {
    last =
      nl->Append( last,
                  nl->RealAtom( relDesc.attrExtSize[i] ) );
    last =
      nl->Append( last,
                  nl->RealAtom( relDesc.attrSize[i] ) );
  }

  last =
    nl->Append( last, nl->IntAtom( relDesc.tupleFileId ) );
  last =
    nl->Append( last, nl->IntAtom( relDesc.lobFileId ) );

  return result;
}

Relation *
Relation::Open( SmiRecord& valueRecord, size_t& offset,
                const ListExpr typeInfo )
{
  RelationDescriptor relDesc( typeInfo );

  valueRecord.SetPos(offset);
  valueRecord.Read( relDesc.noTuples );
  valueRecord.Read( relDesc.totalExtSize );
  valueRecord.Read( relDesc.totalSize );

  for( int i = 0; i < relDesc.tupleType->GetNoAttributes(); i++ )
  {
    double d;
    valueRecord.Read( d );
    relDesc.attrExtSize[i] = d;
  }

  for( int i = 0; i < relDesc.tupleType->GetNoAttributes(); i++ )
  {
    double d;
    valueRecord.Read( d );
    relDesc.attrSize[i] = d;
  }

  valueRecord.Read( relDesc.tupleFileId );
  valueRecord.Read( relDesc.lobFileId );

  offset = valueRecord.GetPos();

  return new Relation( relDesc );
}

bool
Relation::Save( SmiRecord& valueRecord, size_t& offset,
                const ListExpr typeInfo )
{
  valueRecord.SetPos(offset);
  valueRecord.Write( relDesc.noTuples);
  valueRecord.Write( relDesc.totalExtSize );
  valueRecord.Write( relDesc.totalSize );

  for( int i = 0; i < relDesc.tupleType->GetNoAttributes(); i++ )
  {
    valueRecord.Write( relDesc.attrExtSize[i] );
  }

  for( int i = 0; i < relDesc.tupleType->GetNoAttributes(); i++ )
  {
    valueRecord.Write( relDesc.attrSize[i] );
  }

  valueRecord.Write( relDesc.tupleFileId );
  valueRecord.Write( relDesc.lobFileId );

  offset = valueRecord.GetPos();
  return true;
}

void Relation::ErasePointer()
{
  if( pointerTable.find( relDesc.tupleFileId ) !=
                         pointerTable.end() )
  { 
    pointerTable.erase( relDesc.tupleFileId );
  } else {
    map<SmiFileId, Relation*>::iterator it;
    for (it=pointerTable.begin(); it != pointerTable.end(); it++) {
       //cout << "ErasePointer hotfix" << endl;
       if(it->second == this){
           pointerTable.erase(it);
           return;
       }
    }
  }
  
}

void Relation::Close()
{
  ErasePointer();
  delete this;
}

void Relation::Delete()
{
  tupleFile.Close();
  tupleFile.Drop();
  if(relDesc.lobFileId){
    char mode = relDesc.isTemp?1:0;
    Flob::dropFile(relDesc.lobFileId, mode);
    SmiRecordFile rf(false,0, relDesc.isTemp);
    rf.Open(relDesc.lobFileId);
    rf.Close();
    rf.Drop();
  }
  ErasePointer();

  delete this;
}

void Relation::DeleteAndTruncate()
{
  tupleFile.Remove();
  tupleFile.Drop();
  if(relDesc.lobFileId){
    char mode = relDesc.isTemp?1:0;
    Flob::dropFile(relDesc.lobFileId, mode);
    SmiRecordFile rf(false,0, relDesc.isTemp);
    rf.Open(relDesc.lobFileId);
    rf.Close();
    rf.Remove();
    rf.Drop();
  }
//  else {
//    cerr << "Relation has no LOB-file!" << endl;
//  }

  ErasePointer();

  delete this;
}



Relation *Relation::Clone() const
{
  Relation *r = new Relation( relDesc.tupleType );

  Tuple *t;
  GenericRelationIterator *iter = MakeScan();
  while( (t = iter->GetNextTuple()) != 0 )
  {
    r->AppendTuple( t );
    t->DeleteIfAllowed();
  }
  delete iter;

  return r;
}

void Relation::AppendTuple( Tuple *tuple )
{
  tuple->Save( &tupleFile,
    relDesc.lobFileId,
    relDesc.totalExtSize,
    relDesc.totalSize,
    relDesc.attrExtSize,
    relDesc.attrSize );

  relDesc.noTuples += 1;
}


void Relation::AppendTupleNoLOBs( Tuple *tuple )
{
  tuple->Save( &tupleFile,
    relDesc.lobFileId,
    relDesc.totalExtSize,
    relDesc.totalSize,
    relDesc.attrExtSize,
    relDesc.attrSize, true );

  relDesc.noTuples += 1;
}

void Relation::Clear()
{
  relDesc.noTuples = 0;
  relDesc.totalExtSize = 0;
  relDesc.totalSize = 0;
  if(tupleFile.IsTemp()){
    tupleFile.ReCreate();
  } else {
    tupleFile.Truncate();
  }
  if(relDesc.lobFileId){
    SmiRecordFile rf(false,0, relDesc.isTemp);
    rf.Open(relDesc.lobFileId);
    rf.Truncate();
    rf.Close();
  }
}

Tuple *Relation::GetTuple( const TupleId& id,
                           const bool dontReportError ) const
{
  Tuple *result = new Tuple( relDesc.tupleType );
  if( result->Open( &tupleFile, relDesc.lobFileId, id, dontReportError ) )
    return result;

  delete result;
  return 0;
}

/*
The next fcuntion updates the tuple by deleting the old attributes at the
positions given by 'changedIndices' and puts the new attributres
from 'newAttrs' into their places. These changes are made persistent.

*/
void Relation::UpdateTuple( Tuple *tuple,
                            const vector<int>& changedIndices,
                            const vector<Attribute *>& newAttrs )
{
  tuple->UpdateAttributes(
    changedIndices, newAttrs,
    relDesc.totalExtSize,
    relDesc.totalSize,
    relDesc.attrExtSize,
    relDesc.attrSize );
}

/*
Deletes the tuple from the relation, Flobs and SMIRecord are deleted
and the size of the relation is adjusted.

*/
bool Relation::DeleteTuple( Tuple *tuple )
{
  if( tupleFile.DeleteRecord( tuple->GetTupleId() ) )
  {
    Attribute* nextAttr = 0;
    Flob* nextFlob = 0;

    relDesc.noTuples -= 1;
    relDesc.totalExtSize -= tuple->GetRootSize();
    relDesc.totalSize -= tuple->GetRootSize();

    for (int i = 0; i < tuple->GetNoAttributes(); i++)
    {
      nextAttr = tuple->GetAttribute(i);
      nextAttr->Finalize();
      for (int j = 0; j < nextAttr->NumOfFLOBs(); j++)
      {
        nextFlob = nextAttr->GetFLOB(j);
        SmiSize fsz = nextFlob->getSize();

        assert( i >= 0 &&
                (size_t)i < relDesc.attrSize.size() );
        relDesc.attrSize[i] -= fsz;
        relDesc.totalSize -= fsz;

        if( fsz < Tuple::extensionLimit )
        {
          assert( i >= 0 &&
                  (size_t)i < relDesc.attrExtSize.size() );
          relDesc.attrExtSize[i] -= fsz;
          relDesc.totalExtSize -= fsz;
        }

        nextFlob->destroy();
      }
    }
    return true;
  }

  return false;
}

int Relation::GetNoTuples() const
{
  return relDesc.noTuples;
}

TupleType *Relation::GetTupleType() const
{
  return relDesc.tupleType;
}

double Relation::GetTotalRootSize() const
{
  return relDesc.noTuples *
         relDesc.tupleType->GetCoreSize();
}

double Relation::GetTotalRootSize( int i ) const
{
  return relDesc.noTuples *
         relDesc.tupleType->GetAttributeType(i).coreSize;
}

double Relation::GetTotalExtSize() const
{
  return relDesc.totalExtSize;
}

double Relation::GetTotalExtSize( int i ) const
{
  assert( i >= 0 &&
          (size_t)i < relDesc.attrExtSize.size() );
  return relDesc.attrExtSize[i];
}

double Relation::GetTotalSize() const
{
  return relDesc.totalSize;
}

double Relation::GetTotalSize( int i ) const
{
  assert( i >= 0 &&
          (size_t)i < relDesc.attrSize.size() );
  return relDesc.attrSize[i];
}

GenericRelationIterator *Relation::MakeScan() const
{
  return new RelationIterator( *this );
}

GenericRelationIterator *Relation::MakeScan(TupleType* tt) const
{
  return new RelationIterator( *this, tt );
}



RandomRelationIterator *Relation::MakeRandomScan() const
{
  return new RandomRelationIterator( *this );
}


bool Relation::GetTupleFileStats( SmiStatResultType &result )
{
  result = tupleFile.GetFileStatistics(SMI_STATS_EAGER);
  std::stringstream fileid;
  fileid << tupleFile.GetFileId();
  result.push_back(pair<string,string>("FilePurpose",
            "RelationTupleCoreFile"));
  result.push_back(pair<string,string>("FileId",fileid.str()));
  return true;
}


bool Relation::GetLOBFileStats( SmiStatResultType &result )
{
//  cerr << "Relation::GetLOBFileStats( SmiStatResultType &result ) still "
//       << "lacks implementation!" << endl;
  SmiFileId  lobFileId = relDesc.lobFileId;
  SmiRecordFile lobFile(false);
  if( lobFileId == 0 ){
    return true;
  } else if( !lobFile.Open( lobFileId ) ){
    return false;
  } else {
    result = lobFile.GetFileStatistics(SMI_STATS_EAGER);
    result.push_back(pair<string,string>("FilePurpose",
              "RelationTupleLOBFile"));
    std::stringstream fileid;
    fileid << lobFileId;
    result.push_back(pair<string,string>("FileId",fileid.str()));
  };
  if( !lobFile.Close() ){
    return false;
  };
  return true;
}


// SPM: Extension communicated by K. Teufel

Relation *Relation::GetRelation (const SmiFileId fileId )
{
   map<SmiFileId, Relation*>::iterator it = 
      pointerTable.find(fileId);
   if(it!=pointerTable.end()){
      return it->second;
   } else {
      return 0;
   }
}



/*
4.4 Implementation of the class ~RelationIterator~
(using ~PrefetchingIterator~)

This class is used for scanning (iterating through) relations.

*/
RelationIterator::RelationIterator( const Relation& rel, TupleType* tt /*=0*/ ):
  iterator( rel.tupleFile.SelectAllPrefetched() ),
  relation( rel ),
  endOfScan( false ),
  currentTupleId( -1 ),
  outtype( tt )
{}



RelationIterator::~RelationIterator()
{
  delete iterator;
}


Tuple* RelationIterator::GetNextTuple()
{
//#define TRACE_ON
//  NTRACE(10000, "GetNextTuple()")
//#undef TRACE_ON
  if( !iterator->Next() )
  {
    endOfScan = true;
    currentTupleId = -1;
    return 0;
  }

  Tuple *result = new Tuple( relation.relDesc.tupleType );

  bool openOK = result->Open( &relation.tupleFile,
                       relation.relDesc.lobFileId,
                       iterator );
  assert(openOK); // otherwise the prefetching iterators works wrong

  currentTupleId = result->GetTupleId();
  return result;
}

Tuple* RelationIterator::GetNextTuple( const list<int>& attrList )
{
//#define TRACE_ON
//  NTRACE(10000, "GetNextTuple()")
//#undef TRACE_ON
  if( !iterator->Next() )
  {
    endOfScan = true;
    currentTupleId = -1;
    return 0;
  }

  Tuple* result = 0;

  /*
  if (outtype) {
    new Tuple( outtype );
  } else {
    new Tuple( relation.relDesc.tupleType );
  } */

  SHOW( *relation.relDesc.tupleType )
  SHOW( *outtype )

  result = new Tuple( relation.relDesc.tupleType );



  result->OpenPartial( outtype, attrList,
                       &relation.tupleFile,
                       relation.relDesc.lobFileId,
                       iterator );

  currentTupleId = result->GetTupleId();
  return result;
}



TupleId RelationIterator::GetTupleId() const
{
  return currentTupleId;
}

bool RelationIterator::EndOfScan() const
{
  return endOfScan;
}

/*
4.5 Implementation of the class ~RandomRelationIterator~
(using ~PrefetchingIterator~)

This class is used for scanning (iterating through) relations. Currently, the
random iteration is only helpful for creating samples since it is possible to
skip some of the tuples which makes the iteration a litte bit more efficent.
Here we need an implementation which is able to skip some pages of the
underlying record file. This would make it rather efficient,

*/
RandomRelationIterator::RandomRelationIterator( const Relation& relation ):
  RelationIterator( relation )
  {}

RandomRelationIterator::~RandomRelationIterator()
{}

Tuple* RandomRelationIterator::GetNextTuple(int step/*=1*/)
{
//#define TRACE_ON
//  NTRACE(10000, "GetNextTuple()")
//#undef TRACE_ON
  for (; step > 0; step--) { // advance the iterator #step times
    if( !iterator->Next() )
    {
      endOfScan = true;
      currentTupleId = -1;
      return 0;
    }
  }

  Tuple *result = new Tuple( relation.relDesc.tupleType );

  result->Open( &relation.tupleFile,
                relation.relDesc.lobFileId,
                iterator );

  currentTupleId = result->GetTupleId();
  return result;
}

/*
5 Auxiliary functions

5.1 Function ~Concat~

Copies the attribute values of two tuples
(words) ~r~ and ~s~ into tuple (word) ~t~.

*/
void Concat( Tuple *r, Tuple *s, Tuple *t )
{
  int rnoattrs = r->GetNoAttributes();
  int snoattrs = s->GetNoAttributes();
  //int tnoattrs = rnoattrs + snoattrs;

  for( int i = 0; i < rnoattrs; i++) {
    t->CopyAttribute( i, r, i );
  }
  for (int j = 0; j < snoattrs; j++) {
    t->CopyAttribute( j, s, rnoattrs + j );
  }
}

