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

November 30, 2002 RHG Introduced a function ~RelPersistValue~ instead of
~DefaultPersistValue~ which keeps relations that have been built in memory in a
small cache, so that they need not be rebuilt from then on.

March 2003 Victor Almeida created the new Relational Algebra organization

November 2004 M. Spiekermann. The declarations of the PrivateRelation have been
moved to the files RelationPersistent.h and RelationMainMemory.h. This was
necessary to implement some little functions as inline functions.

June 2005 M. Spiekermann. The tuple's size information will now be stored in
member variables and only recomputed after attributes were changed. Changes in
class ~TupleBuffer~ which allow to store tuples as "free" or "non-free" tuples
in it.


[TOC]

1 Overview

The Relational Algebra basically implements two type constructors, namely ~tuple~ and ~rel~.
More information about the Relational Algebra can be found in the RelationAlgebra.h header
file.

This file contains the implementation of the Persistent Relational Algebra, where the
type constructors ~tuple~ and ~rel~ are kept in secondary memory.

A relation has two files: the tuple file and the LOB file, for storing tuples and LOBs
respectively.

The tuples can be in two states, namely ~fresh~ and ~solid~. A fresh tuple is a tuple created
by the In function of the ~tuple~ type constructor. It is stored in memory and looks like
the tuple in the Main Memory Relational Algebra. An example schema of such tuple can be viewed
in Figure 1.

                Figure 1: Example schema of a fresh tuple. [FreshTuple.eps]

A solid tuple is created when a fresh tuple is saved on disk, or when a tuple is read from disk.
The tuple representation on disk has two basic parts: the attributes and the tuple extension
for small FLOBs. Large FLOBs are written in the LOB file of the relation. An example schema of
two tuples of the same type as the one in Figure 1 can be viewed in Figure 2. The first tuple
contains a small region, and the second contains a big one, which is saved separately on the
LOB file.

                Figure 2: Example schema of two solid tuples with a small FLOB (a) and with a big one (b).[SolidTuple.eps]

The state diagram of tuples can be seen in the Figure 3.

                Figure 3: State diagram of tuples.[TupleStateDiagram.eps]

2 Includes, Constants, Globals, Enumerations

*/
#ifdef RELALG_PERSISTENT
/*
This ~RELALG\_PERSISTENT~ defines which kind of relational algebra is to be compiled.
If it is set, the persistent version of the relational algebra will be compiled, and
otherwise, the main memory version will be compiled.

*/

using namespace std;

#include "RelationAlgebra.h"
#include "SecondoSystem.h"
#include "SecondoSMI.h"
#include "FLOB.h"
#include "RelationPersistent.h"
#include "LogMsg.h"

extern NestedList *nl;

/*
3 Type constructor ~tuple~

3.2 Struct ~PrivateTuple~

This struct contains the private attributes of the class ~Tuple~.

*/

#ifdef DONT_COMPILE_THIS
const int PrivateTuple::Save( ostream& *tempFile ) {

 // ToDo: It would be nice to have a method like this which should save memory
 // representations of tuples in a flat file to avoid the complexity of the SMI
 // and Berkeley-DB. This could improve performance for materializing streams
 // for example in the sortby operator. 

}
#endif

const int PrivateTuple::Save( SmiRecordFile *tuplefile, SmiRecordFile *lobfile )
{
  int tupleSize = tupleType.GetTotalSize(),
      extensionSize = 0;
  char *extensionTuple = 0;

  lobFile = lobfile;

  if( state == Solid )
  {
    assert( lobFile != 0 && tupleFile != 0 && memoryTuple != 0 );

    if( tupleRecord != 0 )
      delete tupleRecord;

    // Calculate the size of the small FLOB data which will be saved together
    // with the tuple attributes and save the LOBs in the lobFile.
    for( int i = 0; i < tupleType.GetNoAttributes(); i++)
    {
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
      {
        FLOB *tmpFLOB = attributes[i]->GetFLOB(j);
        tupleSize += tmpFLOB->Size();
        if( !tmpFLOB->IsLob() )
          extensionSize += tmpFLOB->Size();
        else
        {
          tmpFLOB->BringToMemory();
          tmpFLOB->SaveToLob( *lobFile );
        }
      }
    }

    // Move FLOB data to extension tuple.
    if( extensionSize > 0 )
    {
      extensionTuple = (char *)malloc(extensionSize);
      char *extensionPtr = extensionTuple;
      for( int i = 0; i < tupleType.GetNoAttributes(); i++)
      {
        for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
        {
          FLOB *tmpFLOB = attributes[i]->GetFLOB(j);
          if( !tmpFLOB->IsLob() )
          {
            tmpFLOB->SaveToExtensionTuple( extensionPtr );
            extensionPtr += tmpFLOB->Size();
          }
        }
      }
    }
  }
  else
  {
    assert( tupleRecord == 0 && memoryTuple == 0 );

    // Calculate the size of the small FLOB data which will be saved together
    // with the tuple attributes and save the LOBs in the lobFile.
    for( int i = 0; i < tupleType.GetNoAttributes(); i++)
    {
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
      {
        FLOB *tmpFLOB = attributes[i]->GetFLOB(j);
        tupleSize += tmpFLOB->Size();
        if( !tmpFLOB->IsLob() )
          extensionSize += tmpFLOB->Size();
        else
          tmpFLOB->SaveToLob( *lobFile );
      }
    }

    // Move FLOB data to extension tuple.
    if( extensionSize > 0 )
    {
      extensionTuple = (char *)malloc(extensionSize);
      char *extensionPtr = extensionTuple;
      for( int i = 0; i < tupleType.GetNoAttributes(); i++)
      {
        for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
        {
          FLOB *tmpFLOB = attributes[i]->GetFLOB(j);
          if( !tmpFLOB->IsLob() )
          {
            tmpFLOB->SaveToExtensionTuple( extensionPtr );
            extensionPtr += tmpFLOB->Size();
          }
        }
      }
    }

    // Move external attributes to memory tuple
    memoryTuple = (char *)malloc( tupleType.GetTotalSize() );
    int offset = 0;
    for( int i = 0; i < tupleType.GetNoAttributes(); i++)
    {
      memcpy( &memoryTuple[offset], attributes[i], tupleType.GetAttributeType(i).size );
      offset += tupleType.GetAttributeType(i).size;
    }
  }

  assert( extensionSize == 0 && extensionTuple == 0 ||
          extensionSize > 0 && extensionTuple != 0 );
  tupleFile = tuplefile;
  tupleRecord = new SmiRecord();
  tupleId = 0;
  bool rc = tupleFile->AppendRecord( tupleId, *tupleRecord );
  rc = tupleRecord->Write( memoryTuple, tupleType.GetTotalSize(), 0) && rc;
  if( extensionSize > 0 )
    rc = tupleRecord->Write( extensionTuple, extensionSize, tupleType.GetTotalSize() ) && rc;

  state = Solid;

  if( extensionSize > 0 )
  {
    free( extensionTuple ); extensionTuple = 0;
  }

  if( !rc )
    return 0;

  return tupleSize;
}

/*
Saves the updated tuple to disk. Only for the new attributes the real LOBs are saved to the lobfile.
The memorytuple and extensiontuple are again computed and saved to the corresponding tuplerecord. 

*/
const int PrivateTuple::UpdateSave(const vector<int>& changedIndices){
	
  cout << "PrivateTuple::UpdateSave" << endl;
	
	int tupleSize = tupleType.GetTotalSize(), extensionSize = 0;
	assert( (state == Solid) && (lobFile != 0) && (tupleFile != 0) && (memoryTuple != 0) );
	
  	/*Calculate the size of the small FLOB data which will be saved together
     with the tuple attributes and save the LOBs of the new attribute in the lobFile.*/
    for( int i = 0; i < tupleType.GetNoAttributes(); i++)
    {
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
      {
        FLOB *tmpFLOB = attributes[i]->GetFLOB(j);
        tupleSize += tmpFLOB->Size();
        if( !tmpFLOB->IsLob() )
          extensionSize += tmpFLOB->Size();
        else{
        	for (size_t k = 0; k < changedIndices.size(); k++) {
          		if (i == changedIndices[k]){
          			tmpFLOB->SaveToLob( *lobFile );
          			break;
          		}
        	}
        }
      }
    }
    char* newExtensionTuple;

    // Move FLOB data to extension tuple.
    if( extensionSize > 0 )
    {
      newExtensionTuple = (char *)malloc(extensionSize);
      char *extensionPtr = newExtensionTuple;
      for( int i = 0; i < tupleType.GetNoAttributes(); i++)
      {
        for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
        {
          FLOB *tmpFLOB = attributes[i]->GetFLOB(j);
          if( !tmpFLOB->IsLob() )
          {
            tmpFLOB->SaveToExtensionTuple( extensionPtr );
            // Set FLOBs to old state
            //tmpFLOB->Restore( extensionPtr );
            extensionPtr += tmpFLOB->Size();
          }
        }
      }
    }
	char* newMemoryTuple ;
    // Move external attributes to memory tuple
    newMemoryTuple = (char *)malloc( tupleType.GetTotalSize() );
    int offset = 0;
    for( int i = 0; i < tupleType.GetNoAttributes(); i++)
    {
      memcpy( &newMemoryTuple[offset], attributes[i], tupleType.GetAttributeType(i).size );
      offset += tupleType.GetAttributeType(i).size;
    }   
    tupleRecord = new SmiRecord();
  	bool ok = tupleFile->SelectRecord( tupleId, *tupleRecord ,SmiFile::Update);
  	if (! ok){
  		cout << "UpdateSave: there was no record for the tuple with tupleId: " << tupleId << " found" << endl;
  		assert (false);
  	}   
    int oldRecordSize = tupleRecord->Size();
    int newRecordSize = sizeof(int) + tupleType.GetTotalSize() + extensionSize;
  	bool rc = true;
  	//rc = tupleRecord->Write( &extensionSize, sizeof(int), 0) && rc;
	// Now only write new attributes  
  	offset = 0;
  	for( int j = 0; j < tupleType.GetNoAttributes(); j++){
    	// Only overwrite new attributes
  		for (size_t i = 0; i < changedIndices.size(); i++){
  			if ( j == changedIndices[i]){
    			rc = tupleRecord->Write( &newMemoryTuple[offset],tupleType.GetAttributeType(j).size, offset) && rc;
  				break;
  			}
  		}
  		offset += tupleType.GetAttributeType(j).size;
  	}
    // The whole extensiontuple must be rewritten in case the size of a small FLOB has changed
  	if( extensionSize > 0 )
    	rc = tupleRecord->Write( newExtensionTuple, extensionSize, tupleType.GetTotalSize() ) && rc;
    if(newRecordSize < oldRecordSize)
		tupleRecord->Truncate(newRecordSize);
	if( extensionSize > 0 )
  	{
    	free( newExtensionTuple ); 
  	}
  	free( newMemoryTuple ); 
  	delete tupleRecord;
  	tupleRecord = 0;
  	// Reset lobFile for all saved LOBs
    for (size_t k = 0; k < changedIndices.size(); k++) {
    	for( int j = 0; j < attributes[changedIndices[k]]->NumOfFLOBs(); j++){
    		FLOB *tmpFLOB = attributes[changedIndices[k]]->GetFLOB(j);
    		if( tmpFLOB->IsLob() ){
    			tmpFLOB->SetLobFile( lobFile );
    			//tmpFLOB->BringToMemory();
    		}
    	}
    }        		
  	if( !rc )
    	return 0;

  	return tupleSize;
}


const bool PrivateTuple::Open( SmiRecordFile *tuplefile, SmiRecordFile *lobfile, SmiRecordId rid )
{
  assert( state == Fresh &&
          lobFile == 0 && tupleFile == 0 && tupleRecord == 0 &&
          memoryTuple == 0 );

  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();

  tupleId = rid;
  tupleRecord = new SmiRecord();
  this->tupleFile = tuplefile;
  this->lobFile = lobfile;
  if( !tupleFile->SelectRecord( tupleId, *tupleRecord ) )
    return false;

  size_t offset = 0;
  memoryTuple = (char *)malloc( tupleType.GetTotalSize() );
  if( (int)tupleRecord->Read( memoryTuple, tupleType.GetTotalSize(), offset ) != tupleType.GetTotalSize() )
    return false;
  offset += tupleType.GetTotalSize();

  // Read attribute values from memoryTuple.
  char *valuePtr = memoryTuple;
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    int algId = tupleType.GetAttributeType(i).algId,
        typeId = tupleType.GetAttributeType(i).typeId;
    attributes[i] = (TupleElement*)(*(algM->Cast(algId, typeId)))(valuePtr);
    valuePtr += tupleType.GetAttributeType(i).size;
  }

  // Reads the small FLOBs. The read of LOBs is postponed to its usage.
  // Sets the lobFile for all LOBs.
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
    {
      FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
      if( !tmpFLOB->IsLob() )
      {
        tmpFLOB->ReadFromExtensionTuple( *tupleRecord, offset );
        offset += tmpFLOB->Size();
      }
      else
        tmpFLOB->SetLobFile( lobFile );
    }
  }

  // Call the Initialize function for every attribute
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    attributes[i]->Initialize();
  }

  state = Solid;
  return true;
}

const bool PrivateTuple::Open( SmiRecordFile *tuplefile, SmiRecordFile *lobfile, PrefetchingIterator *iter )
{
  assert( state == Fresh && lobFile == 0 && tupleFile == 0 && tupleRecord == 0 );
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();

  iter->ReadCurrentRecordNumber( tupleId );
  this->tupleFile = tuplefile;
  this->lobFile = lobfile;

  size_t offset = 0;
  memoryTuple = (char *)malloc( tupleType.GetTotalSize() );
  if( (int)iter->ReadCurrentData( memoryTuple, tupleType.GetTotalSize(), offset ) != tupleType.GetTotalSize() )
    return false;
  offset += tupleType.GetTotalSize();

  // Read attribute values from memoryTuple.
  char *valuePtr = memoryTuple;
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    int algId = tupleType.GetAttributeType(i).algId,
        typeId = tupleType.GetAttributeType(i).typeId;
    attributes[i] = (TupleElement*)(*(algM->Cast(algId, typeId)))(valuePtr);
    valuePtr += tupleType.GetAttributeType(i).size;
  }

  // Reads the small FLOBs. The read of LOBs is postponed to its usage.
  // Sets the lobFile for all LOBs.
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
    {
      FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
      if( !tmpFLOB->IsLob() )
      {
        tmpFLOB->ReadFromExtensionTuple( *iter, offset );
        offset += tmpFLOB->Size();
      }
      else
        tmpFLOB->SetLobFile( lobFile );
    }
  }

  // Call the Initialize function for every attribute
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    attributes[i]->Initialize();
  }

  state = Solid;
  return true;
}

const bool PrivateTuple::Open( SmiRecordFile *tuplefile, SmiRecordFile *lobfile, SmiRecord *record )
{
  assert( state == Fresh &&
          lobFile == 0 && tupleFile == 0 && tupleRecord == 0 &&
          memoryTuple == 0 );

  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();

  SmiKey key;
  key = record->GetKey();
  key.GetKey( tupleId );
  this->tupleRecord = record;
  this->tupleFile = tuplefile;
  this->lobFile = lobfile;
  if( !tupleFile->SelectRecord( tupleId, *tupleRecord ) )
    return false;;

  size_t offset = 0;
  memoryTuple = (char *)malloc( tupleType.GetTotalSize() );
  if( (int)tupleRecord->Read( memoryTuple, tupleType.GetTotalSize(), offset ) != tupleType.GetTotalSize() )
    return false;
  offset += tupleType.GetTotalSize();

  // Read attribute values from memoryTuple.
  char *valuePtr = memoryTuple;
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    int algId = tupleType.GetAttributeType(i).algId,
        typeId = tupleType.GetAttributeType(i).typeId;
    attributes[i] = (TupleElement*)(*(algM->Cast(algId, typeId)))(valuePtr);
    valuePtr += tupleType.GetAttributeType(i).size;
  }

  // Reads the small FLOBs. The read of LOBs is postponed to its usage.
  // Sets the lobFile for all LOBs.
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
    {
      FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
      if( !tmpFLOB->IsLob() )
      {
        tmpFLOB->ReadFromExtensionTuple( *tupleRecord, offset );
        offset += tmpFLOB->Size();
      }
      else
        tmpFLOB->SetLobFile( lobFile );
    }
  }

  // Call the Initialize function for every attribute
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    attributes[i]->Initialize();
  }

  state = Solid;
  return true;
}

/*
3.3 Implementation of the class ~Tuple~

This class implements the persistent representation of the type constructor ~tuple~.
A tuple contains a pointer to a ~TMTuple~ from the Tuple Manager. For more information
about tuples in the TupleManager see the file TMTuple.h.

*/

Tuple::Tuple( const TupleType& tupleType, const bool isFreeVal ):
  privateTuple( new PrivateTuple( tupleType, isFreeVal ) )
  {
    Init(isFreeVal, tupleType.GetNoAttributes(), privateTuple );
  }

Tuple::Tuple( const ListExpr typeInfo, const bool isFreeVal ):
  privateTuple( new PrivateTuple( typeInfo, isFreeVal ) )
  {
    Init(isFreeVal, (privateTuple->tupleType).GetNoAttributes(), privateTuple);
  }

Tuple::~Tuple()
{
  tuplesDeleted++;
  tuplesInMemory--;
  delete privateTuple;
}

Tuple *Tuple::RestoreFromList( ListExpr typeInfo, ListExpr value, 
                               int errorPos, ListExpr& errorInfo, bool& correct )
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

const int Tuple::GetMemorySize() 
{
  if ( !recomputeMemSize ) {
    return tupleMemSize;
  }

  int extensionSize = 0;

  for( int i = 0; i < privateTuple->tupleType.GetNoAttributes(); i++)
  {
    for( int j = 0; j < privateTuple->attributes[i]->NumOfFLOBs(); j++)
    {
      FLOB *tmpFLOB = privateTuple->attributes[i]->GetFLOB(j);
      if( !tmpFLOB->IsLob() )
        extensionSize += tmpFLOB->Size();
    }
  }
  tupleMemSize = privateTuple->tupleType.GetTotalSize() + extensionSize;
  recomputeMemSize = false;
  return tupleMemSize;
}

const int Tuple::GetTotalSize()
{
  if ( !recomputeTotalSize ) {
    return tupleTotalSize;
  }

  int totalSize = privateTuple->tupleType.GetTotalSize();

  for( int i = 0; i < privateTuple->tupleType.GetNoAttributes(); i++)
  {
    for( int j = 0; j < privateTuple->attributes[i]->NumOfFLOBs(); j++)
    {
      totalSize += privateTuple->attributes[i]->GetFLOB(j)->Size();
    }
  }
  tupleTotalSize = totalSize;
  recomputeTotalSize = false;
  return tupleTotalSize; 
}

void Tuple::PutAttribute( const int index, Attribute* attr )
{
  assert( index >= 0 && index < GetNoAttributes() );

  if( privateTuple->attributes[index] != 0 )
    delete privateTuple->attributes[index];
  privateTuple->attributes[index] = attr;

  recomputeMemSize = true;
  recomputeTotalSize = true;
}

/*
Updates the tuple by replacing the old attributes at the positions given by 'changedIndices'
with the new ones from 'newAttrs'. If the old attribute had FLOBs they are destroyed so that there
is no garbage left on the disk. 
   
*/

void Tuple::UpdateAttributes(const vector<int>& changedIndices, const vector<Attribute*>& newAttrs){
	int index;
	Attribute* newAttr;
	for ( size_t i = 0; i < changedIndices.size(); i++){
		index = changedIndices[i];
		assert( index >= 0 && index < GetNoAttributes() );
		assert( privateTuple->attributes[index] != 0 );
		newAttr = newAttrs[i]->Clone();
   		for (int i = 0; i < privateTuple->attributes[index]->NumOfFLOBs(); i++){
   			FLOB *tmpFLOB = privateTuple->attributes[index]->GetFLOB(i);
   			tmpFLOB->Destroy();
   		}
  		privateTuple->attributes[index] = newAttr;
  		newAttr->Initialize();
	}
  	privateTuple->UpdateSave(changedIndices);

}


/*
3.9 Class ~TupleBuffer~

This class is used to collect tuples for sorting, for example, or
to do a cartesian product. In this persistent version, if the buffer
is small it will be stored in memory and if it is large, it will be
stored into a disk file.

3.9.1 Struct ~PrivateTupleBuffer~

*/
struct PrivateTupleBuffer
{
  PrivateTupleBuffer( const size_t maxMemorySize ):
    MAX_MEMORY_SIZE( maxMemorySize ),
    diskBuffer( 0 ),
    inMemory( true ),
    totalSize( 0 )
    {}
/*
The constructor.

*/
  ~PrivateTupleBuffer()
  {
    if( !inMemory )
      diskBuffer->Delete();
    else
    {
      for( size_t i = 0; i < memoryBuffer.size(); i++ )
        delete memoryBuffer[i];
    }
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
  double totalSize;
/*
The total size occupied by the tuples in the buffer.

*/
};

/*
3.9.2 Implementation of the class ~TupleBuffer~

*/
TupleBuffer::TupleBuffer( const size_t maxMemorySize, bool isFree /* = false */ ):
privateTupleBuffer( new PrivateTupleBuffer( maxMemorySize ) )
{
  isFreeStorageType = isFree;
  if (RTFlag::isActive("RA:TupleBufferInfo")) {
    cmsg.info() << "New Instance of TupleBuffer with size " 
                << maxMemorySize/1024 
                << " kb, isFreeStorageType = " << isFreeStorageType
                << " address = " << (void*)this << endl;
    cmsg.send();
  }
}

TupleBuffer::~TupleBuffer()
{
  delete privateTupleBuffer;
}

const int TupleBuffer::GetNoTuples() const
{
  if( privateTupleBuffer->inMemory )
    return privateTupleBuffer->memoryBuffer.size();
  else
    return privateTupleBuffer->diskBuffer->GetNoTuples();
}

const double TupleBuffer::GetTotalSize() const
{
  if( privateTupleBuffer->inMemory )
    return privateTupleBuffer->totalSize;
  else
    return privateTupleBuffer->diskBuffer->GetTotalSize();
}

const bool TupleBuffer::IsEmpty() const
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
    for( size_t i = 0; i < privateTupleBuffer->memoryBuffer.size(); i++ )
      delete privateTupleBuffer->memoryBuffer[i];
    privateTupleBuffer->memoryBuffer.clear();
    privateTupleBuffer->totalSize = 0;
  }
  else
  {
    privateTupleBuffer->diskBuffer->Clear();
  }
}

void TupleBuffer::AppendTuple( Tuple *t )
{
  t->SetFree( true );
  if( privateTupleBuffer->inMemory )
  {
    if( privateTupleBuffer->totalSize + t->GetTotalSize() <= privateTupleBuffer->MAX_MEMORY_SIZE )
    {
      t->SetFree( isFreeStorageType );
      privateTupleBuffer->memoryBuffer.push_back( t );
      privateTupleBuffer->totalSize += t->GetTotalSize();
    }
    else
    {
      if (RTFlag::isActive("RA:TupleBufferInfo")) {

        cmsg.info() << "Changing TupleBuffer's state from inMemory -> !inMemory" << endl;
        cmsg.send();
        privateTupleBuffer->diskBuffer = new Relation( t->GetTupleType() );
      }

      vector<Tuple*>::iterator iter = privateTupleBuffer->memoryBuffer.begin();
      while( iter != privateTupleBuffer->memoryBuffer.end() )
      {
        privateTupleBuffer->diskBuffer->AppendTuple( *iter );
        delete *iter;
        iter++;
      }

      privateTupleBuffer->memoryBuffer.clear();
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
	assert( id >= 0 && id < (TupleId)privateTupleBuffer->memoryBuffer.size() );
	return privateTupleBuffer->memoryBuffer[id];
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
    diskIterator( tupleBuffer.privateTupleBuffer->inMemory ? 0 : tupleBuffer.privateTupleBuffer->diskBuffer->MakeScan() )
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
TupleBufferIterator::TupleBufferIterator( const TupleBuffer& tupleBuffer ):
  privateTupleBufferIterator( new PrivateTupleBufferIterator( tupleBuffer ) )
  { isFreeStorageType = tupleBuffer.isFreeStorageType; }

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
        privateTupleBufferIterator->tupleBuffer.privateTupleBuffer->memoryBuffer.size() )
      return 0;

    Tuple *result = 
      privateTupleBufferIterator->tupleBuffer.privateTupleBuffer->memoryBuffer[privateTupleBufferIterator->currentTuple];

    privateTupleBufferIterator->currentTuple++;
    if( result->IsFree() != isFreeStorageType ) {
      cerr << "TupleBuffer [" 
        << (void*)&(privateTupleBufferIterator->tupleBuffer) << "]" << endl;
      cerr << "Problem with tuple " << privateTupleBufferIterator->currentTuple << endl;
      cerr << "result->isFree : " <<  result->IsFree() << endl;
      cerr << "isFreeStorage: " << isFreeStorageType << endl;
      cerr << "tuple value:" << *result << endl;
      assert(false);
    };
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
    assert( privateTupleBufferIterator->currentTuple > 0 );
    return privateTupleBufferIterator->currentTuple-1;
  }
}

/*
4 Type constructor ~rel~

4.2 Struct ~RelationDescriptor~

This struct contains necessary information for opening a relation.

*/
struct RelationDescriptor
{
  RelationDescriptor( const int noTuples, const double totalSize,
                      const SmiFileId tId, const SmiFileId lId ):
    noTuples( noTuples ),
    totalSize( totalSize ),
    tupleFileId( tId ),
    lobFileId( lId )
    {}
/*
The first constructor.

*/
  RelationDescriptor( const RelationDescriptor& desc ):
    noTuples( desc.noTuples ),
    totalSize( desc.totalSize ),
    tupleFileId( desc.tupleFileId ),
    lobFileId( desc.lobFileId )
    {}
/*
The copy constructor.

*/
  inline RelationDescriptor& operator=( const RelationDescriptor& d )
  {
    noTuples = d.noTuples;
    totalSize = d.totalSize;
    tupleFileId = d.tupleFileId;
    lobFileId = d.lobFileId;
    return *this;
  }
/*
Redefinition of the assignement operator.

*/
  
  int noTuples;
/*
The quantity of tuples inside the relation.

*/
  double totalSize;
/*
The total size occupied by the tuples in the relation.

*/
  SmiFileId tupleFileId;
/*
The tuple's file identification.

*/
  SmiFileId lobFileId;
/*
The LOB's file identification.

*/
};

/*
4.2 Class ~RelationDescriptorCompare~

*/
class RelationDescriptorCompare
{
  public:
    inline const bool operator()( const RelationDescriptor& d1, const RelationDescriptor d2 )
    {
      if( d1.tupleFileId < d2.tupleFileId )
        return true;
      else if( d1.tupleFileId == d2.tupleFileId &&
               d1.lobFileId == d2.lobFileId )
        return true;
      else 
        return false;
    }
};

/*
4.1 Struct ~PrivateRelation~

This struct contains the private attributes of the class ~Relation~.

*/
struct PrivateRelation
{
  PrivateRelation( const ListExpr typeInfo, const bool isTemporary ):
    noTuples( 0 ),
    totalSize( 0 ),
    tupleType( nl->Second( typeInfo ) ),
    tupleFile( false, 0, isTemporary ),
    lobFile( false, 0, isTemporary )
    {
      if( !tupleFile.Create() )
      {
        string error;
        SmiEnvironment::GetLastErrorCode( error );
        cout << error << endl;
        assert( false );
      }
      if( !lobFile.Create() )
      {
        string error;
        SmiEnvironment::GetLastErrorCode( error );
        cout << error << endl;
        assert( false );
      }
    }
/*
The first constructor. Creates an empty relation from a ~typeInfo~.

*/
  PrivateRelation( const TupleType& tupleType, const bool isTemporary ):
    noTuples( 0 ),
    totalSize( 0 ),
    tupleType( tupleType ),
    tupleFile( false, 0, isTemporary ),
    lobFile( false, 0, isTemporary )
    {
      assert( tupleFile.Create() );
      assert( lobFile.Create() );
    }
/*
The second constructor. Creates an empty relation from a ~tupleType~.

*/
  PrivateRelation( const TupleType& tupleType, const RelationDescriptor& relDesc, const bool isTemporary ):
    noTuples( relDesc.noTuples ),
    totalSize( relDesc.totalSize ),
    tupleType( tupleType ),
    tupleFile( false, 0, isTemporary ),
    lobFile( false, 0, isTemporary )
    {
      assert( tupleFile.Open( relDesc.tupleFileId ) );
      assert( lobFile.Open( relDesc.lobFileId ) );
    }
/*
The third constructor. Opens a previously created relation.

*/
  PrivateRelation( const ListExpr typeInfo, const RelationDescriptor& relDesc, const bool isTemporary ):
    noTuples( relDesc.noTuples ),
    totalSize( relDesc.totalSize ),
    tupleType( nl->Second( typeInfo ) ),
    tupleFile( false, 0 ),
    lobFile( false, 0 )
    {
      assert( tupleFile.Open( relDesc.tupleFileId ) );
      assert( lobFile.Open( relDesc.lobFileId ) );
    }
/*
The fourth constructor. It opens a previously created relation using the ~typeInfo~ instead of
the ~tupleType~.

*/
  ~PrivateRelation()
  {
    assert( tupleFile.Close() );
    assert( lobFile.Close() );
  }
/*
The destuctor. 

*/
  int noTuples;
/*
Contains the number of tuples in the relation.

*/
  double totalSize;
/*
Stores the total size occupied by the tuples in the relation.

*/
  TupleType tupleType;
/*
Stores the tuple type for every tuple of this relation.

*/
  SmiRecordFile tupleFile;
/*
The file to store tuples.

*/
  SmiRecordFile lobFile;
/*
The file to store FLOBs

*/
};

/*
4.2 Implementation of the class ~Relation~

This class implements the persistent representation of the type constructor ~rel~.
A relation is stored into two files: one for the tuples and another for the large
objects (FLOBs) of the tuples.

*/
map<RelationDescriptor, Relation*, RelationDescriptorCompare> Relation::pointerTable;

Relation::Relation( const ListExpr typeInfo, const bool isTemporary ):
privateRelation( new PrivateRelation( typeInfo, isTemporary ) )
{
  RelationDescriptor d( privateRelation->noTuples,
                        privateRelation->totalSize,
                        privateRelation->tupleFile.GetFileId(),
                        privateRelation->lobFile.GetFileId() );
  if( pointerTable.find( d ) == pointerTable.end() )
    pointerTable.insert( make_pair( d, this ) );
}

Relation::Relation( const TupleType& tupleType, const bool isTemporary ):
privateRelation( new PrivateRelation( tupleType, isTemporary ) )
{
  RelationDescriptor d( privateRelation->noTuples,
                        privateRelation->totalSize,
                        privateRelation->tupleFile.GetFileId(),
                        privateRelation->lobFile.GetFileId() );
  if( pointerTable.find( d ) == pointerTable.end() )
    pointerTable.insert( make_pair( d, this ) );
}

Relation::Relation( const TupleType& tupleType, const RelationDescriptor& relDesc, const bool isTemporary ):
privateRelation( new PrivateRelation( tupleType, relDesc, isTemporary ) )
{
  if( pointerTable.find( relDesc ) == pointerTable.end() )
    pointerTable.insert( make_pair( relDesc, this ) );
}

Relation::Relation( const ListExpr typeInfo, const RelationDescriptor& relDesc, const bool isTemporary ):
privateRelation( new PrivateRelation( typeInfo, relDesc, isTemporary ) )
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
  map<RelationDescriptor, Relation*>::iterator i = pointerTable.find( d );
  if( i == pointerTable.end() )
    return 0;
  else
    return i->second;
}

Relation *Relation::RestoreFromList( ListExpr typeInfo, ListExpr value, int errorPos, ListExpr& errorInfo, bool& correct )
{
  RelationDescriptor relDesc( nl->IntValue( nl->First( value ) ),
                              nl->RealValue( nl->Second( value ) ),
                              nl->IntValue( nl->Third( value ) ),
                              nl->IntValue( nl->Fourth( value ) ));
  return new Relation( typeInfo, relDesc );
}

ListExpr Relation::SaveToList( ListExpr typeInfo )
{
  return nl->FourElemList( nl->IntAtom( privateRelation->noTuples ),
                           nl->RealAtom( privateRelation->totalSize ),
                           nl->IntAtom( privateRelation->tupleFile.GetFileId() ),
                           nl->IntAtom( privateRelation->lobFile.GetFileId() ) );
}

Relation *Relation::Open( SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo )
{
  SmiFileId tupleId, lobId;
  int noTuples;
  double totalSize;
  valueRecord.Read( &tupleId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );
  valueRecord.Read( &lobId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );
  valueRecord.Read( &noTuples, sizeof( int ), offset );
  offset += sizeof( int );
  valueRecord.Read( &totalSize, sizeof( double ), offset );
  offset += sizeof( double );

  RelationDescriptor relDesc( noTuples, totalSize, tupleId, lobId );
  return new Relation( typeInfo, relDesc );
}

bool Relation::Save( SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo )
{
  SmiFileId tupleId = privateRelation->tupleFile.GetFileId(),
            lobId = privateRelation->lobFile.GetFileId();
  valueRecord.Write( &tupleId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );
  valueRecord.Write( &lobId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );
  valueRecord.Write( &(privateRelation->noTuples), sizeof( int ), offset );
  offset += sizeof( int );
  valueRecord.Write( &(privateRelation->totalSize), sizeof( double ), offset );
  offset += sizeof( double );

  return true;
}

void Relation::Close()
{
  delete this;
}

void Relation::Delete()
{
  privateRelation->tupleFile.Close();
  privateRelation->tupleFile.Drop();
  privateRelation->lobFile.Close();
  privateRelation->lobFile.Drop();
  delete this;
}

Relation *Relation::Clone()
{
  Relation *r = new Relation( privateRelation->tupleType );

  Tuple *t;
  RelationIterator *iter = MakeScan();
  while( (t = iter->GetNextTuple()) != 0 )
  {
    Tuple *insT = t->CloneIfNecessary();
    if( t != insT )
      t->DeleteIfAllowed();
    r->AppendTuple( insT );
    insT->DeleteIfAllowed();
  }
  delete iter;

  return r;
}

void Relation::AppendTuple( Tuple *tuple )
{
  tuple->SetFree( true );
  privateRelation->totalSize +=
    tuple->GetPrivateTuple()->Save( &privateRelation->tupleFile, &privateRelation->lobFile );
  privateRelation->noTuples += 1;
}
 
/*
Deletes the tuple from the relation, FLOBs and SMIRecord are deleted and the size of the
relation is adjusted.

*/
bool Relation::DeleteTuple( Tuple *tuple )
{
  Attribute* nextAttr;
  FLOB* nextFLOB;
  int tupleSize = tuple->GetTotalSize();
  tuple->SetFree( true );
  for (int i = 0; i < tuple->GetNoAttributes(); i++){
  	nextAttr = tuple->GetAttribute(i);
  	for (int j = 0; j < nextAttr->NumOfFLOBs(); j++){
  		nextFLOB = nextAttr->GetFLOB(j);
  		nextFLOB->Destroy();
  	}
  }
  if (privateRelation->tupleFile.DeleteRecord(tuple->GetTupleId())){
  	privateRelation->totalSize -= tupleSize;
  	privateRelation->noTuples -= 1;
  	return true;
  }
  else
  	return false;
  	
}


/*
Updates the tuple by deleting the old attributes at the positions given by 'changedIndices'
and puts the new attributres from 'newAttrs' into their places. These changes are made persistent.

*/


void Relation::UpdateTuple( Tuple *tuple, const vector<int>& changedIndices,const vector<Attribute *>& newAttrs ){
	int oldSize = tuple->GetTotalSize();
	tuple->UpdateAttributes(changedIndices, newAttrs);
	int newSize = tuple->GetTotalSize();
	privateRelation->totalSize -= oldSize;
	privateRelation->totalSize += newSize;
	
}

void Relation::Clear()
{
  privateRelation->noTuples = 0;
  privateRelation->totalSize = 0;
  assert( privateRelation->tupleFile.Truncate() );
  assert( privateRelation->lobFile.Truncate() );
}

Tuple *Relation::GetTuple( const TupleId& id ) const
{
  Tuple *result = new Tuple( privateRelation->tupleType );
  result->GetPrivateTuple()->Open( &privateRelation->tupleFile,
                                   &privateRelation->lobFile,
                                   id );
  return result;
}

const int Relation::GetNoTuples() const
{
  return privateRelation->noTuples;
}

const TupleType& Relation::GetTupleType() const
{
  return privateRelation->tupleType;
}

const double Relation::GetTotalSize() const
{
  return privateRelation->totalSize;
}

RelationIterator *Relation::MakeScan() const
{
  return new RelationIterator( *this );
}

#ifdef _PREFETCHING_
/*
4.3 Struct ~PrivateRelationIterator~ (using ~PrefetchingIterator~)

This struct contains the private attributes of the class ~RelationIterator~.

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
4.4 Implementation of the class ~RelationIterator~ (using ~PrefetchingIterator~)

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
  if( !privateRelationIterator->iterator->Next() )
  {
    privateRelationIterator->endOfScan = true;
    privateRelationIterator->currentTupleId = -1;
    return 0;
  }

  Tuple *result = new Tuple( privateRelationIterator->relation.privateRelation->tupleType );
  result->GetPrivateTuple()->Open( &privateRelationIterator->relation.privateRelation->tupleFile,
                                   &privateRelationIterator->relation.privateRelation->lobFile,
                                   privateRelationIterator->iterator );
  privateRelationIterator->currentTupleId = result->GetTupleId();
  result->SetFree( true );
  return result;
}

TupleId RelationIterator::GetTupleId() const
{
  assert( privateRelationIterator->currentTupleId != -1 );
  return privateRelationIterator->currentTupleId;
}

const bool RelationIterator::EndOfScan()
{
  return privateRelationIterator->endOfScan;
}
#else
/*
4.5 Struct ~PrivateRelationIterator~ (using ~SmiRecordFileIterator~)

This struct contains the private attributes of the class ~RelationIterator~.

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
4.6 Implementation of the class ~RelationIterator~ (using ~SmiRecordFileIterator~)

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

  if( EndOfScan() ){
  	privateRelationIterator->currentTupleId = -1;
    return 0;
  }

  Tuple *result = new Tuple( privateRelationIterator->relation.privateRelation->tupleType );
  result->GetPrivateTuple()->Open( &privateRelationIterator->relation.privateRelation->tupleFile,
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
  assert( privateRelationIterator->currentTupleId != -1 );
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
  Attribute* attr;

  rnoattrs = r->GetNoAttributes();
  snoattrs = s->GetNoAttributes();
  tnoattrs = rnoattrs + snoattrs;
  

  assert( t->GetNoAttributes() == tnoattrs );

  for( int i = 0; i < rnoattrs; i++)
  {
    attr = r->GetAttribute( i )->Clone();
    t->PutAttribute( i, attr );
  }
  for (int j = 0; j < snoattrs; j++)
  {
    attr = s->GetAttribute( j )->Clone();
    t->PutAttribute( rnoattrs + j, attr );
  }
}

#endif // RELALG_PERSISTENT
