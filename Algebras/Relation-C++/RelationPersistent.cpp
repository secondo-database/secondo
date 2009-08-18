/*
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty for Mathematics
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

#include "LogMsg.h"
#include "QueryProcessor.h"
#include "NestedList.h"
#include "SecondoSystem.h"
#include "SecondoSMI.h"
#include "FLOB.h"
#include "FLOBCache.h"
#include "RelationAlgebra.h"
#include "WinUnix.h"
#include "Symbols.h"
#include "StandardTypes.h"
#include "Serialize.h"

using namespace std;
using namespace symbols;

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

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

  Counter::reportValue(CTR_ATTR_BASIC_OPS, show);
  Counter::reportValue(CTR_ATTR_HASH_OPS, show);
  Counter::reportValue(CTR_ATTR_COMPARE_OPS, show);
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
    DEBUG(this, "Destructor called.")
    // delete all attributes if no further references exist
    for( int i = 0; i < noAttributes; i++ ){
      if( attributes[i] != 0 )
      {
        DEBUG(this, "call attributes[" << i << "]->DeleteIfAllowed() with"
                    << " del.refs = " << (int)attributes[i]->del.refs)

        attributes[i]->DeleteIfAllowed();
        attributes[i] = 0;
      }
      else {
        DEBUG(this, "attributes[" << i << "] == 0")
      }
    }
    tupleType->DeleteIfAllowed();
    tupleType = 0;

    tuplesDeleted++;
    tuplesInMemory--;
    if (noAttributes > MAX_NUM_OF_ATTR){
      delete [] attributes;
    }
  }
/*
The destructor.

*/

#ifdef USE_SERIALIZATION

void Tuple::Save( SmiRecordFile *tuplefile,
                         SmiFileId& lobFileId,
                         double& extSize, double& size,
                         vector<double>& attrExtSize, vector<double>& attrSize,
                         bool ignorePersistentLOBs /*=false*/)
{
  TRACE_ENTER

#ifdef TRACE_ON
  static   long& saveCtr = Counter::getRef("RA:Tuple::Save");
  static  long& flobCtr = Counter::getRef("RA:Tuple::Save.FLOBs");
  static   long& lobCtr  = Counter::getRef("RA:Tuple::Save.LOBs");

  saveCtr++;
#endif

  // Calculate the size of the small FLOB data which will be
  // saved together with the tuple attributes and save the LOBs
  // in the lobFile.
  extSize += tupleType->GetCoreSize();
  size += tupleType->GetCoreSize();

  this->tupleFile = tuplefile;
  this->lobFileId = lobFileId;

  size_t coreSize = 0;

  size_t extensionSize
             = CalculateBlockSize( coreSize, extSize,
			           size, attrExtSize,
				   attrSize, ignorePersistentLOBs );

  lobFileId = this->lobFileId;

  char* data = WriteToBlock(coreSize, extensionSize);

  // Write the data
  DEBUG(this, "Writing tuple to disk!")
  tupleFile = tuplefile;
  SmiRecord *tupleRecord = new SmiRecord();
  tupleId = 0;

  SHOW(coreSize)
  SHOW(extensionSize)

  uint16_t rootSize = coreSize + extensionSize;
  const size_t rootSizeLen = sizeof(rootSize);
  bool rc = tupleFile->AppendRecord( tupleId, *tupleRecord );
  assert(rc == true);
  rc = tupleRecord->Write(data, rootSizeLen + rootSize, 0);
  assert(rc == true);

  tupleRecord->Finish();
  delete tupleRecord;

  // free the allocated memory block
  free(data);
  TRACE_LEAVE
}

#endif



/*

Saves the updated tuple to disk. Only for the new attributes the real
LOBs are saved to the lobfile.

*/
void Tuple::UpdateSave( const vector<int>& changedIndices,
                               double& extSize, double& size,
                               vector<double>& attrExtSize,
                               vector<double>& attrSize )
{
    size_t attrSizes = 0;

    size_t extensionSize
                = CalculateBlockSize( attrSizes, extSize,
				      size, attrExtSize, attrSize );

    char* data = WriteToBlock( attrSizes, extensionSize );


    SmiRecord *tupleRecord = new SmiRecord();
    bool ok = tupleFile->SelectRecord( tupleId, *tupleRecord,
                                      SmiFile::Update );
    if (! ok)
    {
       DEBUG(this, "No record found")
       DEBUG2(this, tupleId)
       assert (false);
    }
    int oldRecordSize = tupleRecord->Size();

#ifdef USE_SERIALIZATION

  uint16_t rootSize = attrSizes + extensionSize;
  const size_t rootSizeLen = sizeof(rootSize);

  int newRecordSize = sizeof(int) + rootSizeLen + rootSize;

  SHOW(rootSize)
  size_t bufSize = rootSizeLen + rootSize;
  //char* newBuf = (char*) malloc( bufSize );
  //memcpy(newBuf, &rootSize, rootSizeLen );
  //memcpy(newBuf + rootSizeLen, data, rootSize);

  SmiSize len = 0;
  len = tupleRecord->Write(data, bufSize);
  assert(len == bufSize);

  //free( newBuf );

#else
    int newRecordSize = sizeof(int) +
                      tupleType->GetTotalSize() +
                      extensionSize;

    // Now write the attributes
    bool rc = tupleRecord->Write( data,
                                  tupleType->GetTotalSize() + extensionSize,
                                  0 );

    assert(rc == true);

#endif

    // The record must be truncated in case the size of a small
    // FLOB was decreased.
    SHOW(newRecordSize)
    SHOW(oldRecordSize)
    if( newRecordSize < oldRecordSize ){
      tupleRecord->Truncate( newRecordSize );
    }
    tupleRecord->Finish();
    delete tupleRecord;

    free( data );
}


#ifdef USE_SERIALIZATION

char* Tuple::WriteToBlock( size_t coreSize,
		           size_t extensionSize )

{
  // create a single block able to pick up the roots of the
  // attributes and all small FLOBs

  uint16_t blockSize = coreSize + extensionSize;

  assert( blockSize < MAX_TUPLESIZE );

  const size_t blockSizeLen = sizeof(blockSize);

  char* data = (char*) malloc( blockSize + blockSizeLen );

          DEBUG(this, "blockSizeLen = " << blockSizeLen)
          DEBUG(this, "coreSize = " << coreSize)
          DEBUG(this, "extensionSize = " << extensionSize)
          DEBUG(this, "blockSize = " << blockSize)

  // write data into the memory block
  size_t offset = 0;
  WriteVar<uint16_t>( blockSize, data, offset );


  //data = block + sizeof(uint16_t);
  //offset = 0;


  uint32_t extOffset = offset + coreSize;

     DEBUG(this, "offset = " << offset)
     DEBUG(this, "extOffset = " << extOffset)

  uint32_t currentSize = 0;
  uint32_t currentExtSize = 0;

  // collect all attributes into the memory block
  for( int i = 0; i < noAttributes; i++)
  {
    DEBUG(this, "defined = " << attributes[i]->IsDefined())

    Attribute::StorageType st =  attributes[i]->GetStorageType();

    if (st == Attribute::Default) {

      currentSize = tupleType->GetAttributeType(i).size;
      attributes[i]->Serialize( data, currentSize, offset );
      offset += currentSize;

      DEBUG(this, "numFlobs = " << numFlobs)


      // iterate the FLOBs
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
      {

          WriteVar<uint32_t>( extOffset, data, offset );
          char *extensionPtr = &data[extOffset];

	  FLOB *tmpFLOB = attributes[i]->GetFLOB(j);
	  size_t offset2 = tmpFLOB->SerializeFD(extensionPtr);
	  extOffset += offset2;
	  DEBUG(this, "offset2-FD = " << offset2)

	  extensionPtr += offset2;
	  if( !tmpFLOB->IsLob() )
	  {
	    offset2 = tmpFLOB->WriteTo(extensionPtr);
	    DEBUG(this, "offset2-WriteTo = " << offset2)
	    extOffset += offset2;
	    extensionPtr += offset2;
	  }

	  DEBUG(this, "offset = " << offset)
	  DEBUG(this, "extOffset = " << extOffset)
	  //DEBUG(this, "extensionPtr = " << (void*)extensionPtr)
	  //DEBUG(this, "&data[extOffset] = " << (void*)&data[extOffset])

      }

    }
    else if (st == Attribute::Core) {

      currentSize = attributes[i]->SerializedSize();
      attributes[i]->Serialize( data, currentSize, offset );
      offset += currentSize;
    }
    else if ( st == Attribute::Extension ) {

      currentSize = sizeof(uint32_t);

      WriteVar<uint32_t>( extOffset, data, offset );
      DEBUG(this, "Attribute::Core, extOffset = " << extOffset)

      currentExtSize = attributes[i]->SerializedSize();
      attributes[i]->Serialize(data, currentExtSize, extOffset);
      //cerr << Array2HexStr(data, currentExtSize, extOffset) << endl;
      extOffset += currentExtSize;
    }
    else {
      cerr << "ERROR: unknown storage type for attribute No "
	   << i << endl;
      assert(false);
    }


     DEBUG(this, "currentSize = " << currentSize)
     DEBUG(this, "offset = " << offset)
     DEBUG(this, "currentExtSize = " << currentExtSize)
     DEBUG(this, "extOffset = " << extOffset)

#ifdef TRACE_ON
     cerr << "Attr(" << i << ")" << *attributes[i] << endl;
     //cerr << Array2HexStr((char*)attributes[i], sz, 0) << endl;
     //cerr << Array2HexStr(data, sz, offset) << endl;
#endif
   }

  return data;
}

#endif



#ifdef USE_SERIALIZATION

size_t Tuple::CalculateBlockSize( size_t& coreSize,
                                  double& extSize,
                                  double& size,
                                  vector<double>& attrExtSize,
                                  vector<double>& attrSize,
                                  bool ignorePersistentLOBs /*= false*/ )
{
  assert( attrExtSize.size() == attrSize.size() );
  assert( (size_t)tupleType->GetNoAttributes() >= attrSize.size() );

  //cerr << "this->lobFileId = " << this->lobFileId << endl;


  coreSize = tupleType->GetCoreSize();
  uint32_t extensionSize = 0;

  // Calculate the size of the small FLOB data which will be
  // saved together with the tuple attributes and save the LOBs
  // in the lobFile.
  for( int i = 0; i < tupleType->GetNoAttributes(); i++)
  {
    Attribute::StorageType st =  attributes[i]->GetStorageType();

    uint16_t currentSize = 0;

    if (st == Attribute::Default) {
      currentSize = tupleType->GetAttributeType(i).size;
    }
    else if (st == Attribute::Core) {
      currentSize = attributes[i]->SerializedSize();
    }
    else if ( st == Attribute::Extension ) {
      currentSize = sizeof(uint32_t);
      size_t ss = attributes[i]->SerializedSize();

      extensionSize += ss;
      attrExtSize[i] += ss;
      extSize += ss;
      size += ss;
      attrSize[i] += ss;
    }
    else {
      cerr << "ERROR: unknown storage type for attribute No "
	   << i << endl;
      assert(false);
    }

    attrExtSize[i] += currentSize;
    attrSize[i] += currentSize;

    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
    {
      FLOB *tmpFLOB = attributes[i]->GetFLOB(j);

      //assert( i >= 0 && (size_t)i < attrSize.size() );
      const size_t tmpSize = tmpFLOB->Size();

      attrExtSize[i] += sizeof(uint32_t); // needed in the core part
                                          // to store offset
      attrSize[i] += sizeof(uint32_t);
      attrSize[i] += tmpSize;
      size += tmpSize;

      if( !tmpFLOB->IsLob() )
      {
        //assert( i >= 0 && (size_t)i < attrExtSize.size() );
        attrExtSize[i] += tmpSize;
        extSize += tmpSize;

        extensionSize += tmpFLOB->MetaSize();
        extensionSize += tmpSize;
      }
      else
      {
        //lobCtr++;

        /* Note: lobFileId is a class member which is *
	 * passed by reference to SaveToLob(...)      */

        if(ignorePersistentLOBs) {
          if (!tmpFLOB->IsPersistentLob()){
            tmpFLOB->SaveToLob( lobFileId );
          }
        } else {
           tmpFLOB->SaveToLob( lobFileId );
        }
        extensionSize += tmpFLOB->MetaSize();
      }
    }
  }

  return extensionSize;
}

#endif



#ifdef USE_SERIALIZATION

bool Tuple::ReadFrom(SmiRecord& record) {

  TRACE_ENTER
  DEBUG(this, "ReadFrom")

  uint16_t rootSize = 0;
  char* data = GetSMIBufferData(record, rootSize);

  if (data) {
    InitializeAttributes(data, rootSize);
    free ( data );
    return true;
  }
  else {
    return false;
  }
}

#endif




bool Tuple::Open( SmiRecordFile* tuplefile,
                  SmiFileId lobfileId,
                  SmiRecordId rid )
{
  tupleId = rid;
  SmiRecord record;
  this->tupleFile = tuplefile;
  this->lobFileId = lobfileId;
  if( !tupleFile->SelectRecord( tupleId, record ) )
  {
    record.Finish();
    return false;
  }

  bool ok = ReadFrom(record);
  record.Finish();
  return ok;
}



bool Tuple::Open( SmiRecordFile* tuplefile,
                  SmiFileId lobfileId,
                  SmiRecord* record )
{
  SmiKey key;
  key = record->GetKey();
  key.GetKey( tupleId );
  this->tupleFile = tuplefile;
  this->lobFileId = lobfileId;
  if( !tupleFile->SelectRecord( tupleId, *record ) )
  {
    record->Finish();
    return false;
  }

  return ReadFrom( *record );
}


char* Tuple::GetSMIBufferData(SmiRecord& record, uint16_t& rootSize)
{
  // read size
  const size_t rootSizeLen = sizeof(rootSize);

  if( record.Read(&rootSize, rootSizeLen, 0) != rootSizeLen )
  {
    record.Finish();
    cerr << "ERROR: record.Read(...) failed!" << endl;
    return 0;
  }

  DEBUG(this, "rootSizeLen = " << rootSizeLen)
  DEBUG(this, "rootSize = " << rootSize)

  // read all attributes
  assert(rootSize < MAX_TUPLESIZE);
  char* data = (char*) malloc ( sizeof(rootSize) + rootSize );
  char* ptr = data + sizeof(rootSize);

  if( record.Read( ptr, rootSize, rootSizeLen) != rootSize )
  {
    record.Finish();
    cerr << "ERROR: record.Read(...) failed!" << endl;
    return 0;
  }

  return data;
}


char* Tuple::GetSMIBufferData(PrefetchingIterator* iter, uint16_t& rootSize)
{
  // read size
  const size_t rootSizeLen = sizeof(rootSize);

  if( (size_t)iter->ReadCurrentData(&rootSize, rootSizeLen, 0) != rootSizeLen )
  {
    cerr << "ERROR: iter->ReadCurrentData(...) failed!" << endl;
    return 0;
  }

  DEBUG(this, "rootSizeLen = " << rootSizeLen)
  DEBUG(this, "rootSize = " << rootSize)

  // read all attributes
  assert(rootSize < MAX_TUPLESIZE);
  char* data = (char*) malloc ( sizeof(rootSize) + rootSize );
  char* ptr = data + sizeof(rootSize);


  if( (uint16_t)iter->ReadCurrentData( ptr,
                                       rootSize,
                                       rootSizeLen) != rootSize )
  {
    cerr << "ERROR: iter->ReadCurrentData(...) failed!" << endl;
    return 0;
  }

  return data;
}



void Tuple::InitializeAttributes(char* src, uint16_t rootSize)
{
  TRACE_ENTER

  size_t offset = sizeof(rootSize);
  //uint32_t extOffset = 0; //offset + tupleType->GetCoreSize();
  int i = 0;

  DEBUG(this, "rootSize = " << rootSize)
  DEBUG(this, "offset = " << offset)
  DEBUG(this, "extOffset = " << extOffset)

  while( i < noAttributes )
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
      uint32_t attrExtOffset = 0;
      ReadVar<uint32_t>( attrExtOffset, src, offset );
      DEBUG(this, "attrExtOffset = " << attrExtOffset)
      attributes[i] = Attribute::Create( attr, &src[attrExtOffset],
		                                            0, algId, typeId);

      DEBUG(this, "Attr(" << i << ")" << *attributes[i])
      //extOffset += attrExtOffset;
    }
    else
    {
      attributes[i] = Attribute::Create( attr, &src[offset], sz, algId, typeId);
      int numFlobs = attributes[i]->NumOfFLOBs();
      DEBUG(this, "numOfFLOBS = " << numFlobs)

      int serialSize = attributes[i]->SerializedSize();
      int readData = (serialSize > 0) ? serialSize : sz;

      DEBUG(this, "readData = " << readData)
      DEBUG(this, Array2HexStr(src, readData, offset) );
      offset += readData;

      // Read the small FLOBs. The read of LOBs is postponed to its
      // usage.

      for( int j = 0; j < numFlobs; j++ )
      {
        uint32_t flobOffset = 0;
        ReadVar<uint32_t>( flobOffset, src, offset );
	//extOffset += flobOffset;
	//DEBUG(this, "extOffset = " << extOffset)
	DEBUG(this, "flobOffset = " << flobOffset)

	FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
	//cerr <<"data = " << Array2HexStr(data, 12, offset) << endl;
	/*offset += */ tmpFLOB->RestoreFD(&src[flobOffset]);
	//cerr << "offset-FD = " << offset << endl;
	//
	if( !tmpFLOB->IsLob()){

	  int size = tmpFLOB->Size();

	    DEBUG( this, "not tmpFlob[" << i << ","  << j << "].IsLob()" )
	    DEBUG(this, "tmpFLOB.size = " << size)

	  void* ptr = tmpFLOB->Malloc();
	  if(ptr){
	     memcpy( ptr, &src[flobOffset], size );

	     //offset += size;
	     //cerr << "offset-memcpy = " << offset << endl;
	  }
	} else{
	  DEBUG( this, "tmpFlob[" << i << ","  << j << "].IsLob()" )
	  //tmpFLOB->SetLobFileId( lobFileId );
	}
      }

      DEBUG(this, "Attr(" << i << ")" << *attributes[i])


    }

    DEBUG(this, "offset = " << offset)
    DEBUG(this, "extOffset = " << extOffset)
    DEBUG(this, "Defined = " << attributes[i]->IsDefined() )



    // Call the Initialize function for every attribute
    // and initialize the reference counter
    attributes[i]->Initialize();
    attributes[i]->InitRefs();

    i++; // next Attribute
  }

  TRACE_LEAVE

}

void Tuple::InitializeSomeAttributes( const list<int>& aIds,
		                      char* src, uint16_t rootSize )
{
  TRACE_ENTER
  DEBUG(this, "rootSize = " << rootSize)

  list<int>::const_iterator iter = aIds.begin();
  for( ; iter != aIds.end(); iter++ )
  {
    int i = *iter;
    DEBUG2(this, i)

    size_t offset = tupleType->GetAttributeType(i).offset;
    int algId = tupleType->GetAttributeType(i).algId;
    int typeId = tupleType->GetAttributeType(i).typeId;
    int sz = tupleType->GetAttributeType(i).size;

        DEBUG2(this, offset)
        DEBUG2(this, sz)

    // create an instance of the specified type, which gives
    // us an instance of a subclass of class Attribute.
    Attribute* attr =
	      static_cast<Attribute*>( am->CreateObj(algId, typeId)(0).addr );

    if ( attr->GetStorageType() == Attribute::Extension )
    {
      uint32_t attrExtOffset = 0;
      ReadVar<uint32_t>( attrExtOffset, src, offset );

           DEBUG(this, "attrExtOffset = " << attrExtOffset)

      attributes[i] = Attribute::Create( attr, &src[attrExtOffset],
		                                            0, algId, typeId);
    }
    else
    {
      attributes[i] = Attribute::Create( attr, &src[offset], sz, algId, typeId);

      int serialSize = attributes[i]->SerializedSize();
      int readData = (serialSize > 0) ? serialSize : sz;

          DEBUG(this, "readData = " << readData)
          DEBUG(this, Array2HexStr(src, readData, offset) );

      offset += readData;

      // Read the small FLOBs. The read of LOBs is postponed to its
      // usage.

      int numFlobs = attributes[i]->NumOfFLOBs();

          DEBUG(this, "numOfFLOBS = " << numFlobs)

      for( int j = 0; j < numFlobs; j++ )
      {
        uint32_t flobOffset = 0;
        ReadVar<uint32_t>( flobOffset, src, offset );

	FLOB *tmpFLOB = attributes[i]->GetFLOB( j );

	    DEBUG(this, "flobOffset = " << flobOffset)
	    //cerr <<"data = " << Array2HexStr(data, 12, offset) << endl;

	tmpFLOB->RestoreFD( &src[flobOffset] );
	if( !tmpFLOB->IsLob() )
	{
	  int size = tmpFLOB->Size();

	      DEBUG( this, "not tmpFlob[" << i << ","  << j << "].IsLob()" )
	      DEBUG(this, "tmpFLOB.size = " << size)

	  void* ptr = tmpFLOB->Malloc();
	  if( ptr ){
	    memcpy( ptr, &src[flobOffset], size );
	  }
	}
	else
	{
	  DEBUG( this, "tmpFlob[" << i << ","  << j << "].IsLob()" )
	  //tmpFLOB->SetLobFileId( lobFileId );
	}
      }
    }

    DEBUG(this, "Attr(" << i << ")" << *attributes[i])
    DEBUG(this, "Defined = " << attributes[i]->IsDefined() )

    // Call the Initialize function for every attribute
    // and initialize the reference counter
    attributes[i]->Initialize();
    attributes[i]->InitRefs();

  }

  TRACE_LEAVE
}


#ifdef USE_SERIALIZATION

bool Tuple::Open( SmiRecordFile *tuplefile,
                  SmiFileId lobfileId,
                  PrefetchingIterator *iter )
{
  TRACE_ENTER
  DEBUG(this, "Open::Prefetch")

  iter->ReadCurrentRecordNumber( tupleId );
  this->tupleFile = tuplefile;
  this->lobFileId = lobfileId;

  uint16_t rootSize = 0;
  char* data = GetSMIBufferData(iter, rootSize);

  if (data) {
    InitializeAttributes(data, rootSize);
    free ( data );
    return true;
  }
  else {
    return false;
  }

  TRACE_LEAVE
}

#endif


bool Tuple::OpenPartial( TupleType* newtype, const list<int>& attrIdList,
		         SmiRecordFile *tuplefile,
                         SmiFileId lobfileId,
                         PrefetchingIterator *iter )
{
  TRACE_ENTER
  DEBUG(this, "OpenPartial using PrefetchingIterator")

  iter->ReadCurrentRecordNumber( tupleId );
  this->tupleFile = tuplefile;
  this->lobFileId = lobfileId;

  uint16_t rootSize = 0;
  char* data = GetSMIBufferData(iter, rootSize);

  if (data) {
    InitializeSomeAttributes(attrIdList, data, rootSize);
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
      FLOB *tmpFLOB = attributes[index]->GetFLOB(j);

      assert( index >= 0 && (size_t)index < attrSize.size() );
      attrSize[index] -= tmpFLOB->Size();
      size -= tmpFLOB->Size();

      if( !tmpFLOB->IsLob() )
      {
        assert( index >= 0 && (size_t)index < attrExtSize.size() );
        attrExtSize[index] -= tmpFLOB->Size();
        extSize -= tmpFLOB->Size();
      }
      tmpFLOB->Destroy();
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

  static long& writtenData_Bytes = Counter::getRef(CTR_TBUF_BYTES_W);
  static long& writtenData_Pages = Counter::getRef(CTR_TBUF_PAGES_W);

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
        diskBuffer->AppendTupleNoLOBs( tuple );
        tuple->DeleteIfAllowed();
        iter++;
      }
      memoryBuffer.clear();
      totalMemSize = 0;
      totalExtSize = 0;
      totalSize = 0;
      diskBuffer->AppendTupleNoLOBs( t );
      inMemory = false;
    }
  }
  else
  {
    return diskBuffer->AppendTupleNoLOBs( t );
  }
}

Tuple *TupleBuffer::GetTuple( const TupleId& id ) const
{
  if( inMemory )
  {
    Tuple* res =  GetTupleAtPos( id );
    res->IncReference();
    return res;
  }
  else
  {
    return diskBuffer->GetTuple( id+1 );
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
3.9.3 ~TupleBufferIterator~

*/
TupleBufferIterator::TupleBufferIterator( const TupleBuffer& tupleBuffer ):
  readData_Bytes( Counter::getRef(CTR_TBUF_BYTES_R) ),
  readData_Pages( Counter::getRef(CTR_TBUF_PAGES_R) ),
  tupleBuffer( tupleBuffer ),
  currentTuple( 0 ),
  diskIterator(
    tupleBuffer.inMemory ?
      0 :
      tupleBuffer.diskBuffer->MakeScan() )
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
map<RelationDescriptor, Relation*, RelationDescriptorCompare>
Relation::pointerTable;

Relation::Relation( const ListExpr typeInfo, bool isTemp ):
privateRelation( new PrivateRelation( typeInfo, isTemp ) )
{
  if( pointerTable.find( privateRelation->relDesc ) ==
                         pointerTable.end() )
    pointerTable.insert( make_pair( privateRelation->relDesc,
                                    this ) );
}

Relation::Relation( TupleType *tupleType, bool isTemp ):
privateRelation( new PrivateRelation( tupleType, isTemp ) )
{
  if( pointerTable.find( privateRelation->relDesc ) ==
                         pointerTable.end() )
    pointerTable.insert( make_pair( privateRelation->relDesc,
                                    this ) );
}

Relation::Relation( const RelationDescriptor& relDesc,
                    bool isTemp ):
privateRelation( new PrivateRelation( relDesc, isTemp ) )
{
  if( pointerTable.find( relDesc ) == pointerTable.end() )
    pointerTable.insert( make_pair( relDesc, this ) );
}

Relation::~Relation()
{
  delete privateRelation;
}

Relation *Relation::GetRelation( const RelationDescriptor& d )
{
  map<RelationDescriptor, Relation*>::iterator
    i = pointerTable.find( d );
  if( i == pointerTable.end() )
    return 0;
  else
    return i->second;
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
    nl->OneElemList( nl->IntAtom( privateRelation->relDesc.noTuples ) ),
           last = result;

  last =
    nl->Append( last, nl->RealAtom( privateRelation->relDesc.totalExtSize ) );
  last =
    nl->Append( last, nl->RealAtom( privateRelation->relDesc.totalSize ) );

  for( int i = 0;
       i < privateRelation->relDesc.tupleType->GetNoAttributes();
       i++ )
  {
    last =
      nl->Append( last,
                  nl->RealAtom( privateRelation->relDesc.attrExtSize[i] ) );
    last =
      nl->Append( last,
                  nl->RealAtom( privateRelation->relDesc.attrSize[i] ) );
  }

  last =
    nl->Append( last, nl->IntAtom( privateRelation->relDesc.tupleFileId ) );
  last =
    nl->Append( last, nl->IntAtom( privateRelation->relDesc.lobFileId ) );

  return result;
}

Relation *
Relation::Open( SmiRecord& valueRecord, size_t& offset,
                const ListExpr typeInfo )
{
  RelationDescriptor relDesc( typeInfo );

  valueRecord.Read( &relDesc.noTuples, sizeof( int ), offset );
  offset += sizeof( int );

  valueRecord.Read( &relDesc.totalExtSize, sizeof( double ), offset );
  offset += sizeof( double );

  valueRecord.Read( &relDesc.totalSize, sizeof( double ), offset );
  offset += sizeof( double );

  for( int i = 0; i < relDesc.tupleType->GetNoAttributes(); i++ )
  {
    double d;
    valueRecord.Read( &d, sizeof( double ), offset );
    relDesc.attrExtSize[i] = d;
    offset += sizeof( double );
  }

  for( int i = 0; i < relDesc.tupleType->GetNoAttributes(); i++ )
  {
    double d;
    valueRecord.Read( &d, sizeof( double ), offset );
    relDesc.attrSize[i] = d;
    offset += sizeof( double );
  }

  valueRecord.Read( &relDesc.tupleFileId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );

  valueRecord.Read( &relDesc.lobFileId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );

  return new Relation( relDesc );
}

bool
Relation::Save( SmiRecord& valueRecord, size_t& offset,
                const ListExpr typeInfo )
{
  const RelationDescriptor& relDesc = privateRelation->relDesc;

  valueRecord.Write( &relDesc.noTuples, sizeof( int ), offset );
  offset += sizeof( int );

  valueRecord.Write( &relDesc.totalExtSize, sizeof( double ), offset );
  offset += sizeof( double );

  valueRecord.Write( &relDesc.totalSize, sizeof( double ), offset );
  offset += sizeof( double );

  for( int i = 0; i < relDesc.tupleType->GetNoAttributes(); i++ )
  {
    valueRecord.Write( &relDesc.attrExtSize[i], sizeof( double ), offset );
    offset += sizeof( double );
  }

  for( int i = 0; i < relDesc.tupleType->GetNoAttributes(); i++ )
  {
    valueRecord.Write( &relDesc.attrSize[i], sizeof( double ), offset );
    offset += sizeof( double );
  }

  valueRecord.Write( &relDesc.tupleFileId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );

  valueRecord.Write( &relDesc.lobFileId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );

  return true;
}

void Relation::ErasePointer()
{
  if( pointerTable.find( privateRelation->relDesc ) !=
                         pointerTable.end() )
  {
    pointerTable.erase( privateRelation->relDesc );
  }
}

void Relation::Close()
{
  ErasePointer();
  delete this;
}

void Relation::Delete()
{
  privateRelation->tupleFile.Close();
  privateRelation->tupleFile.Drop();

  SecondoSystem::GetFLOBCache()->Drop(
    privateRelation->relDesc.lobFileId,
    privateRelation->isTemp );

  ErasePointer();

  delete this;
}

void Relation::DeleteAndTruncate()
{
  privateRelation->tupleFile.Remove();
  privateRelation->tupleFile.Drop();

  SecondoSystem::GetFLOBCache()->Drop(
    privateRelation->relDesc.lobFileId,
    privateRelation->isTemp );

  ErasePointer();

  delete this;
}



Relation *Relation::Clone()
{
  Relation *r = new Relation( privateRelation->relDesc.tupleType );

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
  tuple->Save(
    &privateRelation->tupleFile,
    privateRelation->relDesc.lobFileId,
    privateRelation->relDesc.totalExtSize,
    privateRelation->relDesc.totalSize,
    privateRelation->relDesc.attrExtSize,
    privateRelation->relDesc.attrSize );

  privateRelation->relDesc.noTuples += 1;
}


void Relation::AppendTupleNoLOBs( Tuple *tuple )
{
  tuple->Save(
    &privateRelation->tupleFile,
    privateRelation->relDesc.lobFileId,
    privateRelation->relDesc.totalExtSize,
    privateRelation->relDesc.totalSize,
    privateRelation->relDesc.attrExtSize,
    privateRelation->relDesc.attrSize, true );

  privateRelation->relDesc.noTuples += 1;
}

void Relation::Clear()
{
  privateRelation->relDesc.noTuples = 0;
  privateRelation->relDesc.totalExtSize = 0;
  privateRelation->relDesc.totalSize = 0;
  privateRelation->tupleFile.Truncate();
  SecondoSystem::GetFLOBCache()->Truncate(
    privateRelation->relDesc.lobFileId,
    privateRelation->isTemp );
}

Tuple *Relation::GetTuple( const TupleId& id ) const
{
  Tuple *result = new Tuple( privateRelation->relDesc.tupleType );
  if( result->Open(
        &privateRelation->tupleFile,
        privateRelation->relDesc.lobFileId,
        id ) )
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
    privateRelation->relDesc.totalExtSize,
    privateRelation->relDesc.totalSize,
    privateRelation->relDesc.attrExtSize,
    privateRelation->relDesc.attrSize );
}

/*
Deletes the tuple from the relation, FLOBs and SMIRecord are deleted
and the size of the relation is adjusted.

*/
bool Relation::DeleteTuple( Tuple *tuple )
{
  if( privateRelation->tupleFile.DeleteRecord( tuple->GetTupleId() ) )
  {
    Attribute* nextAttr;
    FLOB* nextFLOB;

    privateRelation->relDesc.noTuples -= 1;
    privateRelation->relDesc.totalExtSize -= tuple->GetRootSize();
    privateRelation->relDesc.totalSize -= tuple->GetRootSize();

    for (int i = 0; i < tuple->GetNoAttributes(); i++)
    {
      nextAttr = tuple->GetAttribute(i);
      nextAttr->Finalize();
      for (int j = 0; j < nextAttr->NumOfFLOBs(); j++)
      {
        nextFLOB = nextAttr->GetFLOB(j);

        assert( i >= 0 &&
                (size_t)i < privateRelation->relDesc.attrSize.size() );
        privateRelation->relDesc.attrSize[i] -= nextFLOB->Size();
        privateRelation->relDesc.totalSize -= nextFLOB->Size();

        if( !nextFLOB->IsLob() )
        {
          assert( i >= 0 &&
                  (size_t)i < privateRelation->relDesc.attrExtSize.size() );
          privateRelation->relDesc.attrExtSize[i] -= nextFLOB->Size();
          privateRelation->relDesc.totalExtSize -= nextFLOB->Size();
        }

        nextFLOB->Destroy();
      }
    }
    return true;
  }

  return false;
}

int Relation::GetNoTuples() const
{
  return privateRelation->relDesc.noTuples;
}

TupleType *Relation::GetTupleType() const
{
  return privateRelation->relDesc.tupleType;
}

double Relation::GetTotalRootSize() const
{
  return privateRelation->relDesc.noTuples *
         privateRelation->relDesc.tupleType->GetCoreSize();
}

double Relation::GetTotalRootSize( int i ) const
{
  return privateRelation->relDesc.noTuples *
         privateRelation->relDesc.tupleType->GetAttributeType(i).coreSize;
}

double Relation::GetTotalExtSize() const
{
  return privateRelation->relDesc.totalExtSize;
}

double Relation::GetTotalExtSize( int i ) const
{
  assert( i >= 0 &&
          (size_t)i < privateRelation->relDesc.attrExtSize.size() );
  return privateRelation->relDesc.attrExtSize[i];
}

double Relation::GetTotalSize() const
{
  return privateRelation->relDesc.totalSize;
}

double Relation::GetTotalSize( int i ) const
{
  assert( i >= 0 &&
          (size_t)i < privateRelation->relDesc.attrSize.size() );
  return privateRelation->relDesc.attrSize[i];
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
  result = privateRelation->tupleFile.GetFileStatistics(SMI_STATS_EAGER);
  std::stringstream fileid;
  fileid << privateRelation->tupleFile.GetFileId();
  result.push_back(pair<string,string>("FilePurpose",
            "RelationTupleCoreFile"));
  result.push_back(pair<string,string>("FileId",fileid.str()));
  return true;
}


bool Relation::GetLOBFileStats( SmiStatResultType &result )
{
//  cerr << "Relation::GetLOBFileStats( SmiStatResultType &result ) still "
//       << "lacks implementation!" << endl;
  SmiFileId  lobFileId = privateRelation->relDesc.lobFileId;
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
   
   map<RelationDescriptor, Relation*>::iterator it;
   for (it=pointerTable.begin(); it != pointerTable.end(); it++)
   {
       if (((*it).first.tupleFileId == fileId))
         return (*it).second;
   }
   return 0;
}



/*
4.4 Implementation of the class ~RelationIterator~
(using ~PrefetchingIterator~)

This class is used for scanning (iterating through) relations.

*/
RelationIterator::RelationIterator( const Relation& rel, TupleType* tt /*=0*/ ):
  iterator( rel.privateRelation->tupleFile.SelectAllPrefetched() ),
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

  Tuple *result = new Tuple( relation.privateRelation->relDesc.tupleType );

  result->Open( &relation.privateRelation->tupleFile,
                relation.privateRelation->relDesc.lobFileId,
                iterator );

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
    new Tuple( relation.privateRelation->relDesc.tupleType );
  } */

  DEBUG2(this, *relation.privateRelation->relDesc.tupleType )
  DEBUG2(this, *outtype )

  result = new Tuple( relation.privateRelation->relDesc.tupleType );



  result->OpenPartial( outtype, attrList,
		       &relation.privateRelation->tupleFile,
                       relation.privateRelation->relDesc.lobFileId,
                       iterator );

  currentTupleId = result->GetTupleId();
  return result;
}



TupleId RelationIterator::GetTupleId() const
{
  return currentTupleId;
}

bool RelationIterator::EndOfScan()
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

  Tuple *result = new Tuple( relation.privateRelation->relDesc.tupleType );

  result->Open( &relation.privateRelation->tupleFile,
                relation.privateRelation->relDesc.lobFileId,
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



/*
6 Old Implementation of Tuple Management without Serialization and direct Attribute Access

for an easy comparison of old and new code the old one is kept here, so one can inspect it without
using CVS to retrieve old code versions.

*/


#ifndef USE_SERIALIZATION

void Tuple::Save( SmiRecordFile *tuplefile,
                         SmiFileId& lobFileId,
                         double& extSize, double& size,
                         vector<double>& attrExtSize, vector<double>& attrSize,
                         bool ignorePersistentLOBs /*=false*/)
{
  TRACE_ENTER

// static   long& saveCtr = Counter::getRef("RA:Tuple::Save");
// static  long& flobCtr = Counter::getRef("RA:Tuple::Save.FLOBs");
// static   long& lobCtr  = Counter::getRef("RA:Tuple::Save.LOBs");

//  saveCtr++;

  // Calculate the size of the small FLOB data which will be
  // saved together with the tuple attributes and save the LOBs
  // in the lobFile.


  //cerr << "lobFileId = " << lobFileId << endl;

  this->tupleFile = tuplefile;
  this->lobFileId = lobFileId;

  extSize += tupleType->GetTotalSize();
  size += tupleType->GetTotalSize();

  size_t attrSizes = tupleType->GetTotalSize();
  size_t extensionSize
             = CalculateBlockSize( attrSizes, extSize, size,
                                   attrExtSize, attrSize,
                                   ignorePersistentLOBs );

  lobFileId = this->lobFileId;
  DEBUG(this, "attrSizes = " << attrSizes)
  DEBUG(this, "extSize = " << extensionSize)

  char* data = WriteToBlock(attrSizes, extensionSize);

  // Write the data
  tupleFile = tuplefile;
  SmiRecord *tupleRecord = new SmiRecord();
  tupleId = 0;

  bool rc = tupleFile->AppendRecord( tupleId, *tupleRecord );
  assert(rc == true);
  rc = tupleRecord->Write(data, sizeof(uint16_t) + attrSizes + extensionSize,
                          0);
  assert(rc == true);

  tupleRecord->Finish();
  delete tupleRecord;

  free( data );

  TRACE_LEAVE
}


bool Tuple::ReadFrom(SmiRecord& record) {

  size_t offset = 0;

  // we read all attributes within a single block to
  // avoid frequently fetching small data sizes
  if( (int)
      record.Read( tupleData,
                   tupleType->GetTotalSize(),
                   offset )                    !=  tupleType->GetTotalSize() )
  {
    record.Finish();
    return false;
  }

  offset += tupleType->GetTotalSize();

  // Read attribute values from memoryTuple.
  // Calculate the size of the extension tuple.
  // Set the lobFile for all LOBs.
  size_t extensionSize = 0;
  char *valuePtr = tupleData;

  for( int i = 0; i < noAttributes; i++ )
  {
    int algId  = tupleType->GetAttributeType(i).algId;
    int typeId = tupleType->GetAttributeType(i).typeId;
    // cast the attribute
    attributes[i] = (Attribute*)(*(am->Cast(algId, typeId)))(valuePtr);

    attributes[i] = (Attribute*)malloc(tupleType->GetAttributeType(i).size);
    memcpy(attributes[i], valuePtr, tupleType->GetAttributeType(i).size);
    attributes[i]->SetFreeAttr();
    valuePtr += tupleType->GetAttributeType(i).size;

    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
    {
      FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
      if( !tmpFLOB->IsLob() ){
        extensionSize += tmpFLOB->Size();
      } else {
       // tmpFLOB->SetLobFileId( lobFileId );
      }
    }
  }

  // Read the small FLOBs. The read of LOBs is postponed to its usage. As for
  // the memorytuple read all small FLOBS into a single block and than
  // distribute them free in memory

  if( extensionSize > 0 )
  {
   char* extensionTuple = &tupleData[offset];
   if( record.Read( extensionTuple, extensionSize,
                      offset ) != extensionSize )
    {
      record.Finish();
      return false;
    }
    char *extensionPtr = extensionTuple;
    for( int i = 0; i < noAttributes; i++ )
    {
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
      {
        FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
        if( !tmpFLOB->IsLob() )
        {
          tmpFLOB->ReadFrom( extensionPtr );
          extensionPtr += tmpFLOB->Size();
        }
        /*} else {
          tmpFLOB->SetLobFileId( lobFileId );
        }*/
      }
    }
  }

  // Call the Initialize function for every attribute
  // and initialize the reference counter
  for( int i = 0; i < noAttributes; i++ ){
    attributes[i]->Initialize();
    attributes[i]->InitRefs();
  }
  return true;
}


bool Tuple::Open( SmiRecordFile *tuplefile,
                         SmiFileId lobfileId,
                         PrefetchingIterator *iter )
{
  iter->ReadCurrentRecordNumber( tupleId );
  this->tupleFile = tuplefile;
  this->lobFileId = lobfileId;


  /* In case of the prefetching iterator, we don't need
   *  to read the data first into a single block.
   *  Indeed, we can read each attribute by a single call.
   **/

  size_t offset = 0;

  for( int i = 0; i < noAttributes; i++ )
  {
    int algId = tupleType->GetAttributeType(i).algId;
    int typeId = tupleType->GetAttributeType(i).typeId;
    int size = tupleType->GetAttributeType(i).size;

    attributes[i] = (Attribute*)malloc(size);
    if( (int)iter->ReadCurrentData(attributes[i],
                              size, offset)!=size){
       // problem in reading, delete all attributes allocated
       // before
       for(int k=0;k<=i;k++){
          free(attributes[k]);
       }
       return false;
    }
    offset += size;
    // all fine, cast the attribute
    attributes[i] = (Attribute*)(*(am->Cast(algId, typeId)))(attributes[i]);
    attributes[i]->SetFreeAttr();

  }

  // Read the small FLOBs. The read of LOBs is postponed to its
  // usage.

  for(int i=0; i< noAttributes; i++){
    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
    {
      FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
      if( !tmpFLOB->IsLob()){
        void* ptr = tmpFLOB->Malloc();
        if(ptr){
           int size = tmpFLOB->Size();
           if((int)iter->ReadCurrentData( ptr,
                                          size,
                                          offset)!=size){
             // error in getting the data
             // free all mallocs
             for(int k=0;k<noAttributes;k++){
                for(int m=0;m<attributes[i]->NumOfFLOBs();m++){
                  FLOB* victim = attributes[k]->GetFLOB(m);
                  if(k<=i || m<=j){ // FLOB buffer already allocated
                     delete victim;
                  }
                }
                free(attributes[k]);
             }
             return false;
           }
           offset += size;
        }
     } else{
        //tmpFLOB->SetLobFileId( lobFileId );
     }
    }
  }
  // Call the Initialize function for every attribute
  // and initialize the reference counter
  for( int i = 0; i < noAttributes; i++ ){
    attributes[i]->Initialize();
    attributes[i]->InitRefs();
  }

  return true;

}


size_t Tuple::CalculateBlockSize( size_t& coreSize,
                                  double& extSize,
                                  double& size,
                                  vector<double>& attrExtSize,
                                  vector<double>& attrSize,
                                  bool ignorePersistentLOBs /*= false*/ )
{
  assert( attrExtSize.size() == attrSize.size() );
  assert( (size_t)tupleType->GetNoAttributes() >= attrSize.size() );

  //cerr << "this->lobFileId = " << this->lobFileId << endl;


  coreSize = tupleType->GetCoreSize();
  uint32_t extensionSize = 0;

  // Calculate the size of the small FLOB data which will be
  // saved together with the tuple attributes and save the LOBs
  // in the lobFile.
  for( int i = 0; i < tupleType->GetNoAttributes(); i++)
  {
    attrExtSize[i] += tupleType->GetAttributeType(i).size;
    attrSize[i] += tupleType->GetAttributeType(i).size;

    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
    {
      FLOB *tmpFLOB = attributes[i]->GetFLOB(j);

      //assert( i >= 0 && (size_t)i < attrSize.size() );
      const size_t tmpSize = tmpFLOB->Size();

      attrSize[i] += tmpSize;
      size += tmpSize;

      if( !tmpFLOB->IsLob() )
      {
        //assert( i >= 0 && (size_t)i < attrExtSize.size() );
        attrExtSize[i] += tmpSize;
        extSize += tmpSize;

        extensionSize += tmpFLOB->MetaSize();
        extensionSize += tmpSize;
      }
      else
      {
        //lobCtr++;

        /* Note: lobFileId is a class member which is *
	 * passed by reference to SaveToLob(...)      */

        if(ignorePersistentLOBs) {
          if (!tmpFLOB->IsPersistentLob()){
            tmpFLOB->SaveToLob( lobFileId );
          }
        } else {
           tmpFLOB->SaveToLob( lobFileId );
        }
        extensionSize += tmpFLOB->MetaSize();
      }
    }
  }

  return extensionSize;
}


char* Tuple::WriteToBlock(size_t attrSizes, size_t extensionSize) {

  // create a single block able to pick up the roots of the
  // attributes and all small FLOBs

  size_t blockSize = tupleType->GetTotalSize() + extensionSize;

  assert( blockSize < MAX_TUPLESIZE );

  char* data = (char*) malloc( blockSize );

  int offset = 0;

  // collect all attributes into the memory block
  for( int i = 0; i < noAttributes; i++)
  {
     memcpy( &data[offset], attributes[i],
              tupleType->GetAttributeType(i).size );
      offset += tupleType->GetAttributeType(i).size;
  }

  DEBUG(this, "offset = " << offset
              << "totalsize = " << tupleType->GetTotalSize())
  DEBUG(this, "Writing tuple to disk!")


  // Copy FLOB data to behind the attributes
  if( extensionSize>0) // there are small FLOBs
  {
     char *extensionPtr = &data[offset];
     for( int i = 0; i < noAttributes; i++)
     {
       for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
       {
         FLOB *tmpFLOB = attributes[i]->GetFLOB(j);
         if( !tmpFLOB->IsLob() )
         {
           extensionPtr += tmpFLOB->WriteTo(extensionPtr);
         }
       }
     }
     DEBUG(this, "extPtr = " << (void*) extensionPtr
                   << " extSize = " << extensionSize )
 }

 return data;

}



#endif


