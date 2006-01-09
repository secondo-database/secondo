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

[1] Implementation of the Module Update Relation Algebra for Persistent storage

June 2005 Matthias Zielke

January 2006, Victor Almeida separated this algebra from the Extended
Relational Algebra and fixed some memory leaks.

[TOC]

1 Includes and defines

*/
#ifdef RELALG_PERSISTENT

#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "QueryProcessor.h"
#include "SecondoInterface.h"
#include "RelationPersistent.h"

#include <vector>

extern NestedList* nl;
extern QueryProcessor* qp;

/*
2 Class ~Tuple~

Updates the tuple by replacing the old attributes at the positions given by 'changedIndices'
with the new ones from 'newAttrs'. If the old attribute had FLOBs they are destroyed so that there
is no garbage left on the disk.

*/
void Tuple::UpdateAttributes(const vector<int>& changedIndices, const vector<Attribute*>& newAttrs)
{
  int index;
  for ( size_t i = 0; i < changedIndices.size(); i++)
  {
    index = changedIndices[i];
    assert( index >= 0 && index < GetNoAttributes() );
    assert( privateTuple->attributes[index] != 0 );
    for (int j = 0; j < privateTuple->attributes[index]->NumOfFLOBs(); j++)
    {
      FLOB *tmpFLOB = privateTuple->attributes[index]->GetFLOB(j);
      tmpFLOB->Destroy();
    }
    privateTuple->attributes[index]->DeleteIfAllowed();
    privateTuple->attributes[index] = newAttrs[i];
  }
  privateTuple->UpdateSave(changedIndices);
}

/*
2 Class ~PrivateTuple~

Saves the updated tuple to disk. Only for the new attributes the real LOBs are saved to the lobfile.
The memorytuple and extensiontuple are again computed and saved to the corresponding tuplerecord.

*/
int PrivateTuple::UpdateSave(const vector<int>& changedIndices)
{
  int tupleSize = tupleType.GetTotalSize(), extensionSize = 0;
  assert( state == Solid && lobFileId != 0 && tupleFile != 0 );

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
      else
      {
        for (size_t k = 0; k < changedIndices.size(); k++)
        {
          if (i == changedIndices[k])
          {
            tmpFLOB->SaveToLob( lobFileId );
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
  SmiRecord *tupleRecord = new SmiRecord();
  bool ok = tupleFile->SelectRecord( tupleId, *tupleRecord,
                                     SmiFile::Update );
  if (! ok)
  {
    cout << "UpdateSave: there was no record for the tuple with tupleId: " << tupleId << " found" << endl;
    assert (false);
  }
  int oldRecordSize = tupleRecord->Size();
  int newRecordSize = sizeof(int) + tupleType.GetTotalSize() + extensionSize;
  bool rc = true;
    //rc = tupleRecord->Write( &extensionSize, sizeof(int), 0) && rc;

  // Now only write new attributes
  offset = 0;
  for( int j = 0; j < tupleType.GetNoAttributes(); j++)
  {
    // Only overwrite new attributes
    for (size_t i = 0; i < changedIndices.size(); i++)
    {
      if ( j == changedIndices[i])
      {
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
  tupleRecord->Finish();
  delete tupleRecord;

  // Reset lobFile for all saved LOBs
  for (size_t k = 0; k < changedIndices.size(); k++)
  {
    for( int j = 0; j < attributes[changedIndices[k]]->NumOfFLOBs(); j++)
    {
      FLOB *tmpFLOB = attributes[changedIndices[k]]->GetFLOB(j);
      if( tmpFLOB->IsLob() )
      {
        tmpFLOB->SetLobFileId( lobFileId );
        //tmpFLOB->BringToMemory();
      }
    }
  }
  if( !rc )
    return 0;

  return tupleSize;
}

/*
4 Class ~Relation~

Updates the tuple by deleting the old attributes at the positions given by 'changedIndices'
and puts the new attributres from 'newAttrs' into their places. These changes are made persistent.

*/
void Relation::UpdateTuple( Tuple *tuple, 
                            const vector<int>& changedIndices,
                            const vector<Attribute *>& newAttrs )
{
  int oldSize = tuple->GetTotalSize();
  tuple->UpdateAttributes(changedIndices, newAttrs);
  int newSize = tuple->GetTotalSize();
  privateRelation->totalSize -= oldSize;
  privateRelation->totalSize += newSize;
}

/*
Deletes the tuple from the relation, FLOBs and SMIRecord are deleted
and the size of the relation is adjusted.

*/
bool Relation::DeleteTuple( Tuple *tuple )
{
  Attribute* nextAttr;
  FLOB* nextFLOB;
  int tupleSize = tuple->GetTotalSize();
  for (int i = 0; i < tuple->GetNoAttributes(); i++)
  {
    nextAttr = tuple->GetAttribute(i);
    nextAttr->Finalize();
    for (int j = 0; j < nextAttr->NumOfFLOBs(); j++)
    {
      nextFLOB = nextAttr->GetFLOB(j);
      nextFLOB->Destroy();
    }
  }
  if (privateRelation->tupleFile.DeleteRecord(tuple->GetTupleId()))
  {
    privateRelation->totalSize -= tupleSize;
    privateRelation->noTuples -= 1;
    return true;
  }
  else
    return false;
}

#endif
