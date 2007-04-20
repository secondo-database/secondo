/*  
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

April 2006, M. Spiekermann. Intorduced a bew function ~clearAll~ 
in ~PrivateTupleBuffer~. This function will now call the tuples' 
member functions ~DecReference~ and ~DeleteIfAllowed~ instead of 
deleting the tuple pointer directly. 

January 2007, M. Spiekermann. Fixed a memory leak in ~PrivateTupleBuffer~.

April 2007, T Behr, Concept of solid tuples has been removed.


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

using namespace std;

#include <string.h>

#include "QueryProcessor.h"
#include "NestedList.h"
#include "SecondoSystem.h"
#include "SecondoSMI.h"
#include "FLOB.h"
#include "FLOBCache.h"
#include "RelationAlgebra.h"
#include "LogMsg.h"

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

bool PrivateTuple::debug = false;

#undef TRACE
#define TRACE(ptr, msg) { \
	    cerr << (void*)this \
	         << " " << __FUNCTION__ << ": " \
	         << msg << endl; }

#undef DEBUG
#define DEBUG(ptr, msg) if (debug) TRACE(ptr, msg)


/*
3 Type constructor ~tuple~

3.2 Struct ~PrivateTuple~

This struct contains the private attributes of the class ~Tuple~.

*/


/*
The first constructor. It creates a tuple from a ~tupleType~.

*/
PrivateTuple::PrivateTuple(TupleType *tupleType):
    tupleId( 0 ),
    tupleType( tupleType ),
    attributes( 0 ),
    lobFileId( 0 ),
    tupleFile( 0 ),
    NumOfAttr(tupleType->GetNoAttributes())
    {
      DEBUG(this, "Constructor PrivateTuple(TupleType *tupleType) called.")
      tupleType->IncReference();
    }
/*
The second constructor. It creates tuple from a ~typeInfo~.

*/
  PrivateTuple::PrivateTuple( const ListExpr typeInfo ):
    tupleId( 0 ),
    tupleType( new TupleType( typeInfo ) ),
    attributes( 0 ),
    lobFileId( 0 ),
    tupleFile( 0 ),
    NumOfAttr(tupleType->GetNoAttributes())
    {
      DEBUG(this, "Constructor PrivateTuple(const ListExpr typeInfo) called.")
    }


  PrivateTuple::~PrivateTuple() 
  {
    DEBUG(this, "Destructor called.")
    // delete all attributes if no further references exist 
    for( int i = 0; i < NumOfAttr; i++ ){
      if( attributes[i] != 0 )
      {
	DEBUG(this, "call attributes[" << i << "]->DeleteIfAllowed() with"
                    << " del.refs = " << (int)attributes[i]->del.refs)

        attributes[i]->DeleteIfAllowed();
      } 
      else {
	DEBUG(this, "attributes[" << i << "] == 0")
      }	      
    }
    tupleType->DeleteIfAllowed();
  }
/*
The destructor.

*/


void PrivateTuple::Save( SmiRecordFile *tuplefile, 
                         SmiFileId& lobFileId,
                         double& extSize, double& size,
                         vector<double>& attrExtSize, vector<double>& attrSize,
                         bool ignoreLOBs /*=false*/)
{
  long extensionSize = 0;

// static   long& saveCtr = Counter::getRef("RA:PrivateTuple::Save");
// static  long& flobCtr = Counter::getRef("RA:PrivateTuple::Save.FLOBs");
// static   long& lobCtr  = Counter::getRef("RA:PrivateTuple::Save.LOBs");

//  saveCtr++;

  // Calculate the size of the small FLOB data which will be 
  // saved together with the tuple attributes and save the LOBs 
  // in the lobFile.
  extSize += tupleType->GetTotalSize();
  size += tupleType->GetTotalSize();

  for( int i = 0; i < NumOfAttr; i++)
  {
    assert( i >= 0 && (size_t)i < attrExtSize.size() );
    attrExtSize[i] += tupleType->GetAttributeType(i).size;
    assert( i >= 0 && (size_t)i < attrSize.size() );
    attrSize[i] += tupleType->GetAttributeType(i).size;

    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
    {
      FLOB *tmpFLOB = attributes[i]->GetFLOB(j);

      attrSize[i] += tmpFLOB->Size();
      size += tmpFLOB->Size();

      if( !tmpFLOB->IsLob() )
      {
        // flobCtr++;      
        attrExtSize[i] += tmpFLOB->Size();
        extSize += tmpFLOB->Size();

        extensionSize += tmpFLOB->Size();
      }
      else
      {
        //lobCtr++;  
        if(!ignoreLOBs){
           tmpFLOB->SaveToLob( lobFileId );
        }
      }
    }
  }

  

  // create a single block able to pick up the roots of the
  // attributes and all small FLOBs
  
  char* tupleData = (char*) malloc( tupleType->GetTotalSize() +
                                    extensionSize);
  int offset = 0;

  // collect all attributes into the memory block
  for( int i = 0; i < NumOfAttr; i++)
  {
     memcpy( &tupleData[offset], attributes[i], 
              tupleType->GetAttributeType(i).size );
      offset += tupleType->GetAttributeType(i).size;
  }
  
  DEBUG(this, "offset = " << offset 
	      << "totalsize = " << tupleType->GetTotalSize()) 
  DEBUG(this, "Writing tuple to disk!")
  

  // Copy FLOB data to behind the attributes
  if( extensionSize>0) // there are small FLOBs
  {
     char *extensionPtr = &tupleData[offset];
     for( int i = 0; i < NumOfAttr; i++)
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

  // Write the data 
  tupleFile = tuplefile;
  SmiRecord *tupleRecord = new SmiRecord();
  tupleId = 0;
  bool rc = tupleFile->AppendRecord( tupleId, *tupleRecord );
  assert(rc == true);
  rc = tupleRecord->Write(tupleData, 
                          tupleType->GetTotalSize()+extensionSize,
                          0);
  assert(rc == true);
  tupleRecord->Finish();
  delete tupleRecord;
  // free the block
  free(tupleData);
}

/*

Saves the updated tuple to disk. Only for the new attributes the real 
LOBs are saved to the lobfile.

*/
void PrivateTuple::UpdateSave( const vector<int>& changedIndices,
                               double& extSize, double& size,
                               vector<double>& attrExtSize,
                               vector<double>& attrSize )
{

  long extensionSize = 0;
  bool hasFLOBs = false;

  // Calculate the size of the small FLOB data which will be
  // saved together with the tuple attributes and save the LOBs
  // in the lobFile.
  for( int i = 0; i < tupleType->GetNoAttributes(); i++)
  {
    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
    {
      hasFLOBs = true;
      FLOB *tmpFLOB = attributes[i]->GetFLOB(j);

      assert( i >= 0 && (size_t)i < attrSize.size() );
      attrSize[i] += tmpFLOB->Size();
      size += tmpFLOB->Size();

      if( !tmpFLOB->IsLob() )
      {
        assert( i >= 0 && (size_t)i < attrExtSize.size() );
        attrExtSize[i] += tmpFLOB->Size();
        extSize += tmpFLOB->Size();

        extensionSize += tmpFLOB->Size();
      }
      else
      {
        tmpFLOB->SaveToLob( lobFileId );
      }
    }
  }

  // Copy external attributes to memory tuple
  char* memoryTuple;
  memoryTuple = (char*)malloc( tupleType->GetTotalSize() );
  int offset = 0;
  for( int i = 0; i < tupleType->GetNoAttributes(); i++)
  {
     memcpy( &memoryTuple[offset], attributes[i],
              tupleType->GetAttributeType(i).size );
     offset += tupleType->GetAttributeType(i).size;
   }

   // Copy FLOB data to extension tuple.
   char* extensionTuple=0; 
   if( hasFLOBs )
   {
     if( extensionSize > 0 )
        extensionTuple = (char *)malloc(extensionSize);

      char *extensionPtr = extensionTuple;
      for( int i = 0; i < tupleType->GetNoAttributes(); i++)
      {
        for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
        {
          FLOB *tmpFLOB = attributes[i]->GetFLOB(j);
          if( !tmpFLOB->IsLob() )
          {
            extensionPtr += tmpFLOB->WriteTo( extensionPtr );
          }
        }
      }
    }

    SmiRecord *tupleRecord = new SmiRecord();
    bool ok = tupleFile->SelectRecord( tupleId, *tupleRecord,
                                      SmiFile::Update );
    if (! ok)
    {
       TRACE(this, "There was no record for the tuple with "
                       << "tupleId: " << tupleId << " found" )
       assert (false);
    }
    int oldRecordSize = tupleRecord->Size();
    int newRecordSize = sizeof(int) + 
                      tupleType->GetTotalSize() + 
                      extensionSize;
    bool rc = true;

    // Now write the attributes
    rc = tupleRecord->Write( memoryTuple, 
                             tupleType->GetTotalSize(), 
                              0 ) && rc;

    // The whole extension tuple must be rewritten.
    if( extensionSize > 0 ){
        rc = tupleRecord->Write( extensionTuple, 
                             extensionSize, 
                             tupleType->GetTotalSize() ) && rc;
    }

    // The record must be truncated in case the size of a small 
    // FLOB has decreased.
    if( newRecordSize < oldRecordSize ){
      tupleRecord->Truncate( newRecordSize );
    }
    tupleRecord->Finish();
    delete tupleRecord;
    free(memoryTuple);
    if(extensionTuple){
       free(extensionTuple);
    }

}



bool PrivateTuple::Open( SmiRecordFile *tuplefile, 
                         SmiFileId lobfileId, SmiRecordId rid )
{
  tupleId = rid;
  SmiRecord *tupleRecord = new SmiRecord();
  this->tupleFile = tuplefile;
  this->lobFileId = lobfileId;
  if( !tupleFile->SelectRecord( tupleId, *tupleRecord ) )
  {
    delete tupleRecord;
    return false;
  }

  size_t offset = 0;


  // we read all attributes within a single block to
  // avoid frequently getting of small data sizes
  char* memoryTuple=0;
  memoryTuple = (char *)malloc( tupleType->GetTotalSize() );

  if( (int)tupleRecord->Read( memoryTuple, tupleType->GetTotalSize(),
                              offset ) != tupleType->GetTotalSize() )
  {
    tupleRecord->Finish();
    delete tupleRecord;
    free( memoryTuple ); memoryTuple = 0;
    return false;
  }
  offset += tupleType->GetTotalSize();

  // Read attribute values from memoryTuple.
  // Calculate the size of the extension tuple.
  // Set the lobFile for all LOBs.
  size_t extensionSize = 0;
  char *valuePtr = memoryTuple;
  for( int i = 0; i < NumOfAttr; i++ )
  {
    int algId  = tupleType->GetAttributeType(i).algId;
    int typeId = tupleType->GetAttributeType(i).typeId;
    // cast the attribute
    attributes[i] = (Attribute*)(*(am->Cast(algId, typeId)))(valuePtr);
    attributes[i] = (Attribute*)malloc(tupleType->GetAttributeType(i).size);
    memcpy(attributes[i], valuePtr, tupleType->GetAttributeType(i).size);
    attributes[i]->SetFreeAttr(); // mark as created using malloc
    valuePtr += tupleType->GetAttributeType(i).size;

    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
    {
      FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
      if( !tmpFLOB->IsLob() )
        extensionSize += tmpFLOB->Size();
      else
        tmpFLOB->SetLobFileId( lobFileId );
    }
  }

  free(memoryTuple);

  // Read the small FLOBs. The read of LOBs is postponed to its 
  // usage.
  // as for the memorytuple read all small FLOBS into a 
  // single block and than distribute them free in memory
 
  if( extensionSize > 0 )
  {
    char* extensionTuple = (char*)malloc( extensionSize );
    if( tupleRecord->Read( extensionTuple, extensionSize,
                           offset ) != extensionSize )
    {
      tupleRecord->Finish();
      delete tupleRecord;
      free( memoryTuple ); memoryTuple = 0;
      free( extensionTuple ); extensionTuple = 0;
      return false;
    }

    char *extensionPtr = extensionTuple;
    for( int i = 0; i < NumOfAttr; i++ )
    {
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
      {
        FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
        if( !tmpFLOB->IsLob() )
        {
          tmpFLOB->ReadFrom( extensionPtr );
          extensionPtr += tmpFLOB->Size();
        }
      }
    }
    free(extensionTuple);
  }

  // Call the Initialize function for every attribute
  // and sets the references to 1
  for( int i = 0; i < NumOfAttr; i++ ){
    attributes[i]->Initialize();
    attributes[i]->InitRefs();
  }

  tupleRecord->Finish();
  delete tupleRecord;

  return true;
}

bool PrivateTuple::Open( SmiRecordFile *tuplefile, 
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

  for( int i = 0; i < NumOfAttr; i++ )
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
  
  for(int i=0; i< NumOfAttr; i++){
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
             for(int k=0;k<NumOfAttr;k++){
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
        tmpFLOB->SetLobFileId( lobFileId );
     }
    }
  }  
  // Call the Initialize function for every attribute
  // and initialize the reference counter
  for( int i = 0; i < NumOfAttr; i++ ){
    attributes[i]->Initialize();
    attributes[i]->InitRefs();
  }

  return true;
}

bool PrivateTuple::Open( SmiRecordFile *tuplefile, 
                         SmiFileId lobfileId, 
                         SmiRecord *record )
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

  size_t offset = 0;
  // read a single block and copy the data to be faster
  char* memoryTuple = (char *)malloc( tupleType->GetTotalSize() );
  if( (int)record->Read( memoryTuple, 
                              tupleType->GetTotalSize(), offset ) != 
      tupleType->GetTotalSize() )
  {
    free( memoryTuple ); memoryTuple = 0;
    record->Finish();
    return false;
  }
  offset += tupleType->GetTotalSize();

  // Read attribute values from memoryTuple.
  // Calculate the size of the extension tuple.
  // Set the lobFile for all LOBs.
  size_t extensionSize = 0;
  char *valuePtr = memoryTuple;
  for( int i = 0; i < NumOfAttr; i++ )
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
      if( !tmpFLOB->IsLob() )
        extensionSize += tmpFLOB->Size();
      else
        tmpFLOB->SetLobFileId( lobFileId );
    }
  }

  free(memoryTuple);


  // Read the small FLOBs. The read of LOBs is postponed to its 
  // usage.
  // as for the memorytuple read all small FLOBS into a 
  // single block and than distribute them free in memory
 
  if( extensionSize > 0 )
  {
    char* extensionTuple = (char*)malloc( extensionSize );
   if( record->Read( extensionTuple, extensionSize,
                      offset ) != extensionSize )
    {
      record->Finish();
      free( extensionTuple ); extensionTuple = 0;
      return false;
    }
    char *extensionPtr = extensionTuple;
    for( int i = 0; i < NumOfAttr; i++ )
    {
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
      {
        FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
        if( !tmpFLOB->IsLob() )
        {
          tmpFLOB->ReadFrom( extensionPtr );
          extensionPtr += tmpFLOB->Size();
        }
      }
    }
    free(extensionTuple);
  }


  // Call the Initialize function for every attribute
  // and initialize the reference counter
  for( int i = 0; i < NumOfAttr; i++ ){
    attributes[i]->Initialize();
    attributes[i]->InitRefs();
  }
  return true;
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
  return (TupleId&)privateTuple->tupleId;
}

void Tuple::SetTupleId( const TupleId& tupleId )
{
  privateTuple->tupleId = (SmiRecordId)tupleId;
}

void Tuple::PutAttribute( const int index, Attribute* attr )
{
  if( privateTuple->attributes[index] != 0 )
    privateTuple->attributes[index]->DeleteIfAllowed();
  privateTuple->attributes[index] = attr;

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

3.9.1 Struct ~PrivateTupleBuffer~

*/
struct PrivateTupleBuffer
{
  typedef vector<Tuple*> TupleVec;
 
  PrivateTupleBuffer( const size_t maxMemorySize ):
    MAX_MEMORY_SIZE( maxMemorySize ),
    diskBuffer( 0 ),
    inMemory( true ),
    totalExtSize( 0 ),
    totalSize( 0 )
    {}
/*
The constructor.

*/
  ~PrivateTupleBuffer()
  {
    clearAll();
    if( !inMemory )
      diskBuffer->Delete();
  }

  void clearAll()
  {
     for( TupleVec::iterator it = memoryBuffer.begin(); 
          it != memoryBuffer.end(); it++ )
     {
       //cout << (void*) *it << " - " << (*it)->GetNumOfRefs() << endl; 
       (*it)->DecReference();
       (*it)->DeleteIfAllowed();
     }  
     memoryBuffer.clear();
     totalSize=0;
  }
  
/*
The destructor.

*/
  const size_t MAX_MEMORY_SIZE;
/*
The maximum size of the memory in bytes. 32 MBytes being used.

*/
  vector<Tuple*> memoryBuffer;
/*
The memory buffer which is a ~vector~ from STL.

*/
  Relation* diskBuffer;
/*
The buffer stored on disk.

*/
  bool inMemory;
/*
A flag that tells if the buffer fit in memory or not.

*/
  double totalExtSize;
/*
The total size occupied by the tuples in the buffer,
taking into account the small FLOBs.

*/
  double totalSize;
/*
The total size occupied by the tuples in the buffer,
taking into account the FLOBs.

*/
};

/*
3.9.2 Implementation of the class ~TupleBuffer~

*/
TupleBuffer::TupleBuffer( const size_t maxMemorySize ):
privateTupleBuffer( new PrivateTupleBuffer( maxMemorySize ) )
{
  if (RTFlag::isActive("RA:TupleBufferInfo")) 
  {
    cmsg.info() << "New Instance of TupleBuffer with size " 
                << maxMemorySize/1024 
                << " address = " << (void*)this << endl;
    cmsg.send();
  }
}

TupleBuffer::~TupleBuffer()
{
  delete privateTupleBuffer;
}

int TupleBuffer::GetNoTuples() const
{
  if( privateTupleBuffer->inMemory )
    return privateTupleBuffer->memoryBuffer.size();
  else
    return privateTupleBuffer->diskBuffer->GetNoTuples();
}

double TupleBuffer::GetTotalRootSize() const
{
  if( IsEmpty() )
    return 0;

  return GetNoTuples() * 
    ( privateTupleBuffer->inMemory ?
      privateTupleBuffer->memoryBuffer[0]->GetRootSize() :
      privateTupleBuffer->diskBuffer->GetTupleType()->GetTotalSize() ); 
}

double TupleBuffer::GetTotalExtSize() const
{
  if( privateTupleBuffer->inMemory )
    return privateTupleBuffer->totalExtSize;
  else
    return privateTupleBuffer->diskBuffer->GetTotalExtSize();
}

double TupleBuffer::GetTotalSize() const
{
  if( privateTupleBuffer->inMemory )
    return privateTupleBuffer->totalSize;
  else
    return privateTupleBuffer->diskBuffer->GetTotalSize();
}

bool TupleBuffer::IsEmpty() const
{
  if( privateTupleBuffer->inMemory )
    return privateTupleBuffer->memoryBuffer.empty();
  else
    return false;
}

void TupleBuffer::Clear()
{
  if( privateTupleBuffer->inMemory )
  {
    privateTupleBuffer->clearAll();
  }
  else
  {
    privateTupleBuffer->diskBuffer->Clear();
  }
}

void TupleBuffer::AppendTuple( Tuple *t )
{
  if( privateTupleBuffer->inMemory )
  {
    if( privateTupleBuffer->totalExtSize + t->GetExtSize() <= 
        privateTupleBuffer->MAX_MEMORY_SIZE )
    {
      t->IncReference();
      privateTupleBuffer->memoryBuffer.push_back( t );
      privateTupleBuffer->totalExtSize += t->GetExtSize();
      privateTupleBuffer->totalSize += t->GetSize();
    }
    else
    {
      if (RTFlag::isActive("RA:TupleBufferInfo")) 
      {
        cmsg.info() << "Changing TupleBuffer's state from inMemory "
                    << "-> !inMemory" << endl;
        cmsg.send();
      }
      privateTupleBuffer->diskBuffer = 
        new Relation( t->GetTupleType(), true );

      vector<Tuple*>::iterator iter = 
        privateTupleBuffer->memoryBuffer.begin();
      while( iter != privateTupleBuffer->memoryBuffer.end() )
      {
        privateTupleBuffer->diskBuffer->AppendTuple( *iter );
        (*iter)->DecReference();
        (*iter)->DeleteIfAllowed();
        iter++;
      }
      privateTupleBuffer->memoryBuffer.clear();
      privateTupleBuffer->totalExtSize = 0;
      privateTupleBuffer->totalSize = 0;
      privateTupleBuffer->diskBuffer->AppendTuple( t );
      privateTupleBuffer->inMemory = false;
    }
  }
  else
  {
    return privateTupleBuffer->diskBuffer->AppendTuple( t );
  }
}

Tuple *TupleBuffer::GetTuple( const TupleId& id ) const
{
  if( privateTupleBuffer->inMemory )
  {
    if( id >= 0 && 
        id < (TupleId)privateTupleBuffer->memoryBuffer.size() &&
        privateTupleBuffer->memoryBuffer[id] != 0 )
      return privateTupleBuffer->memoryBuffer[id];
    return 0;
  }
  else
    return privateTupleBuffer->diskBuffer->GetTuple( id );
}

TupleBufferIterator *TupleBuffer::MakeScan() const
{
  return new TupleBufferIterator( *this );
}

/*
3.9.3 Struct ~PrivateTupleBufferIterator~

*/
struct PrivateTupleBufferIterator
{
  PrivateTupleBufferIterator( const TupleBuffer& tupleBuffer ):
    tupleBuffer( tupleBuffer ),
    currentTuple( 0 ),
    diskIterator( 
      tupleBuffer.privateTupleBuffer->inMemory ?  
        0 : 
        tupleBuffer.privateTupleBuffer->diskBuffer->MakeScan() )
    {
    }
/*
The constructor.

*/
  ~PrivateTupleBufferIterator()
  {
    delete diskIterator;
  }
/*
The destructor.

*/
  const TupleBuffer& tupleBuffer;
/*
A pointer to the tuple buffer.

*/
  size_t currentTuple;
/*
The current tuple if it is in memory.

*/
  RelationIterator *diskIterator;
/*
The iterator if it is not in memory.

*/
};

/*
3.9.3 Implementation of the class ~TupleBufferIterator~

*/
TupleBufferIterator::
TupleBufferIterator( const TupleBuffer& tupleBuffer ):
  privateTupleBufferIterator( 
    new PrivateTupleBufferIterator( tupleBuffer ) )
  { 
  }

TupleBufferIterator::~TupleBufferIterator()
{
  delete privateTupleBufferIterator;
}

Tuple *TupleBufferIterator::GetNextTuple()
{
  if( privateTupleBufferIterator->diskIterator )
  {
    return privateTupleBufferIterator->diskIterator->GetNextTuple();
  }
  else
  {
    if( privateTupleBufferIterator->currentTuple == 
        privateTupleBufferIterator->
          tupleBuffer.privateTupleBuffer->memoryBuffer.size() )
      return 0;

    Tuple *result = 
      privateTupleBufferIterator->tupleBuffer.privateTupleBuffer->
        memoryBuffer[privateTupleBufferIterator->currentTuple];
    privateTupleBufferIterator->currentTuple++;

    return result;
  }
}

TupleId TupleBufferIterator::GetTupleId() const
{
  if( privateTupleBufferIterator->diskIterator )
  {
    return privateTupleBufferIterator->diskIterator->GetTupleId();
  }
  else
  {
    return privateTupleBufferIterator->currentTuple-1;
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

void Relation::Close()
{
  if( pointerTable.find( privateRelation->relDesc ) !=
                         pointerTable.end() )
    pointerTable.erase( privateRelation->relDesc );  
  delete this;
}

void Relation::Delete()
{
  privateRelation->tupleFile.Close();
  privateRelation->tupleFile.Drop();

  SecondoSystem::GetFLOBCache()->Drop( 
    privateRelation->relDesc.lobFileId, 
    privateRelation->isTemp );

  if( pointerTable.find( privateRelation->relDesc ) !=
                         pointerTable.end() )
    pointerTable.erase( privateRelation->relDesc );  

  delete this;
}

Relation *Relation::Clone()
{
  Relation *r = new Relation( privateRelation->relDesc.tupleType );

  Tuple *t;
  RelationIterator *iter = MakeScan();
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
  tuple->GetPrivateTuple()->Save( 
    &privateRelation->tupleFile, 
    privateRelation->relDesc.lobFileId,
    privateRelation->relDesc.totalExtSize,
    privateRelation->relDesc.totalSize,
    privateRelation->relDesc.attrExtSize,
    privateRelation->relDesc.attrSize );

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
  if( result->GetPrivateTuple()->Open( 
        &privateRelation->tupleFile,
        privateRelation->relDesc.lobFileId,
        id ) )
    return result;

  delete result;
  return 0;
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
         privateRelation->relDesc.tupleType->GetTotalSize();
}

double Relation::GetTotalRootSize( int i ) const
{
  return privateRelation->relDesc.noTuples *
         privateRelation->relDesc.tupleType->GetAttributeType(i).size;
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

RelationIterator *Relation::MakeScan() const
{
  return new RelationIterator( *this );
}

RandomRelationIterator *Relation::MakeRandomScan() const
{
  return new RandomRelationIterator( *this );
}

#ifdef _PREFETCHING_
/*
4.3 Struct ~PrivateRelationIterator~ (using ~PrefetchingIterator~)

This struct contains the private attributes of the class 
~RelationIterator~.

*/
struct PrivateRelationIterator
{
  PrivateRelationIterator( const Relation& rel ):
    iterator( rel.privateRelation->tupleFile.SelectAllPrefetched() ),
    relation( rel ),
    endOfScan( false ),
    currentTupleId( -1 )
    {
    }
/*
The constructor.

*/
  ~PrivateRelationIterator()
  {
    delete iterator;
  }
/*
The destructor.

*/
  PrefetchingIterator *iterator;
/*
The iterator.

*/
  const Relation& relation;
/*
A reference to the relation.

*/
  bool endOfScan;
/*
Stores the state of the iterator.

*/
  TupleId currentTupleId;
/*
Stores the identification of the current tuple.

*/
};

/*
4.4 Implementation of the class ~RelationIterator~ 
(using ~PrefetchingIterator~)

This class is used for scanning (iterating through) relations.

*/
RelationIterator::RelationIterator( const Relation& relation ):
  privateRelationIterator( new PrivateRelationIterator( relation ) )
  {}

RelationIterator::~RelationIterator()
{
  delete privateRelationIterator;
}

Tuple* RelationIterator::GetNextTuple()
{
//#define TRACE_ON
//  NTRACE(10000, "GetNextTuple()")
//#undef TRACE_ON 
  if( !privateRelationIterator->iterator->Next() )
  {
    privateRelationIterator->endOfScan = true;
    privateRelationIterator->currentTupleId = -1;
    return 0;
  }

  Tuple *result = new Tuple( 
    privateRelationIterator->relation.privateRelation->relDesc.tupleType );
  result->GetPrivateTuple()->Open( 
    &privateRelationIterator->relation.privateRelation->tupleFile,
    privateRelationIterator->relation.
      privateRelation->relDesc.lobFileId,
    privateRelationIterator->iterator );
  privateRelationIterator->currentTupleId = result->GetTupleId();
  return result;
}

TupleId RelationIterator::GetTupleId() const
{
  return privateRelationIterator->currentTupleId;
}

bool RelationIterator::EndOfScan()
{
  return privateRelationIterator->endOfScan;
}


/*
4.5 Implementation of the class ~RandomRelationIterator~ 
(using ~PrefetchingIterator~)

This class is used for scanning (iterating through) relations.

*/
RandomRelationIterator::RandomRelationIterator( const Relation& relation ):
  privateRelationIterator( new PrivateRelationIterator( relation ) )
  {}

RandomRelationIterator::~RandomRelationIterator()
{
  delete privateRelationIterator;
}

Tuple* RandomRelationIterator::GetNextTuple(int step/*=1*/)
{
//#define TRACE_ON
//  NTRACE(10000, "GetNextTuple()")
//#undef TRACE_ON 
  for (; step > 0; step--) {
  if( !privateRelationIterator->iterator->Next() )
  {
    privateRelationIterator->endOfScan = true;
    privateRelationIterator->currentTupleId = -1;
    return 0;
  }
  }

  Tuple *result = new Tuple( 
    privateRelationIterator->relation.privateRelation->relDesc.tupleType );
  result->GetPrivateTuple()->Open( 
    &privateRelationIterator->relation.privateRelation->tupleFile,
    privateRelationIterator->relation.
      privateRelation->relDesc.lobFileId,
    privateRelationIterator->iterator );
  privateRelationIterator->currentTupleId = result->GetTupleId();
  return result;
}

TupleId RandomRelationIterator::GetTupleId() const
{
  return privateRelationIterator->currentTupleId;
}

bool RandomRelationIterator::EndOfScan()
{
  return privateRelationIterator->endOfScan;
}

#else
/*
4.5 Struct ~PrivateRelationIterator~ (using ~SmiRecordFileIterator~)

This struct contains the private attributes of the class 
~RelationIterator~.

*/
struct PrivateRelationIterator
{
  PrivateRelationIterator( const Relation& rel ):
    iterator(),
    relation( rel ), currentTupleId( -1 )
    {
      rel.privateRelation->tupleFile.SelectAll( iterator );
    }
/*
The constructor.

*/
  ~PrivateRelationIterator()
  {}
/*
The destructor.

*/
  SmiRecordFileIterator iterator;
/*
The iterator.

*/
  const Relation& relation;
/*
A reference to the relation.

*/

  TupleId currentTupleId;
/*
Stores the identification of the current tuple.

*/

};

/*
4.6 Implementation of the class ~RelationIterator~ 
(using ~SmiRecordFileIterator~)

This class is used for scanning (iterating through) relations.

*/
RelationIterator::RelationIterator( const Relation& relation ):
  privateRelationIterator( new PrivateRelationIterator( relation ) )
  {}

RelationIterator::~RelationIterator()
{
  delete privateRelationIterator;
}

Tuple* RelationIterator::GetNextTuple()
{
  SmiRecord *record = new SmiRecord();
  privateRelationIterator->iterator.Next( *record );

  if( EndOfScan() )
  {
    privateRelationIterator->currentTupleId = -1;
    return 0;
  }

  Tuple *result = new Tuple( 
    privateRelationIterator->relation.privateRelation->tupleType );
  result->GetPrivateTuple()->Open( 
    &privateRelationIterator->relation.privateRelation->tupleFile,
    &privateRelationIterator->relation.privateRelation->lobFile,
    record );
  privateRelationIterator->currentTupleId = result->GetTupleId();
  return result;
}

const bool RelationIterator::EndOfScan()
{
  return privateRelationIterator->iterator.EndOfScan();
}

TupleId RelationIterator::GetTupleId() const
{
  return privateRelationIterator->currentTupleId;
}

#endif // _PREFETCHING_

/*
5 Auxiliary functions

5.1 Function ~Concat~

Copies the attribute values of two tuples
(words) ~r~ and ~s~ into tuple (word) ~t~.

*/
void Concat( Tuple *r, Tuple *s, Tuple *t )
{
  int rnoattrs, snoattrs, tnoattrs;

  rnoattrs = r->GetNoAttributes();
  snoattrs = s->GetNoAttributes();
  tnoattrs = rnoattrs + snoattrs;

  for( int i = 0; i < rnoattrs; i++)
    t->CopyAttribute( i, r, i );
  for (int j = 0; j < snoattrs; j++)
    t->CopyAttribute( j, s, rnoattrs + j );
}
