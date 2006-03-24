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

[1] Implementation of the Module Update Relation Algebra for persistent 
storage

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

Updates the tuple by replacing the old attributes at the positions given 
by 'changedIndices' with the new ones from 'newAttrs'. If the old attribute 
had FLOBs they are destroyed so that there is no garbage left on the disk.

*/
void Tuple::UpdateAttributes( const vector<int>& changedIndices, 
                              const vector<Attribute*>& newAttrs )
{
  int index;
  for ( size_t i = 0; i < changedIndices.size(); i++)
  {
    index = changedIndices[i];
    assert( index >= 0 && index < GetNoAttributes() );
    assert( privateTuple->attributes[index] != 0 );
    for (int j = 0; 
         j < privateTuple->attributes[index]->NumOfFLOBs(); 
         j++)
    {
      FLOB *tmpFLOB = privateTuple->attributes[index]->GetFLOB(j);
      tmpFLOB->Destroy();
    }
    if( privateTuple->state == Fresh )
    {
      privateTuple->attributes[index]->DeleteIfAllowed();
      privateTuple->attributes[index] = newAttrs[i];
    }
    else
    { 
      memcpy( privateTuple->attributes[index], newAttrs[i], 
              privateTuple->tupleType->GetAttributeType(index).size );
    }
  }
  privateTuple->UpdateSave( changedIndices );

  recomputeExtSize = true;
  recomputeSize = true;
}

/*
2 Class ~PrivateTuple~

Saves the updated tuple to disk. Only for the new attributes the real 
LOBs are saved to the lobfile. The memorytuple and extensiontuple are 
again computed and saved to the corresponding tuplerecord.

*/
void PrivateTuple::UpdateSave( const vector<int>& changedIndices )
{
  int tupleSize = tupleType->GetTotalSize(), extensionSize = 0;
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
      tupleSize += tmpFLOB->Size();
      if( !tmpFLOB->IsLob() )
        extensionSize += tmpFLOB->Size();
      else
      {
        tmpFLOB->BringToMemory();
        tmpFLOB->SaveToLob( lobFileId );
      }
    }
  }

  if( state == Fresh )
  {
    // Move FLOB data to extension tuple.
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
            tmpFLOB->SaveToExtensionTuple( extensionPtr );
            extensionPtr += tmpFLOB->Size();
          }
        }
      }
    }

    // Move external attributes to memory tuple
    assert( memoryTuple == 0 );
    memoryTuple = (char*)malloc( tupleType->GetTotalSize() );
    int offset = 0;
    for( int i = 0; i < tupleType->GetNoAttributes(); i++)
    {
      memcpy( &memoryTuple[offset], attributes[i],
              tupleType->GetAttributeType(i).size );
      attributes[i]->DeleteIfAllowed();
      attributes[i] = (Attribute*)
        (*(am->Cast(tupleType->GetAttributeType(i).algId,
                    tupleType->GetAttributeType(i).typeId)))
          (&memoryTuple[offset]);
      offset += tupleType->GetAttributeType(i).size;
    }
  }

  SmiRecord *tupleRecord = new SmiRecord();
  bool ok = tupleFile->SelectRecord( tupleId, *tupleRecord,
                                     SmiFile::Update );
  if (! ok)
  {
    cout << "UpdateSave: there was no record for the tuple with "
         << "tupleId: " << tupleId << " found" << endl;
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
  if( extensionSize > 0 )
    rc = tupleRecord->Write( extensionTuple, 
                             extensionSize, 
                             tupleType->GetTotalSize() ) && rc;

  // The record must be truncated in case the size of a small 
  // FLOB has decreased.
  if( newRecordSize < oldRecordSize )
    tupleRecord->Truncate( newRecordSize );

  tupleRecord->Finish();
  delete tupleRecord;

  state = Solid;
}

/*
4 Class ~Relation~

Updates the tuple by deleting the old attributes at the 
positions given by 'changedIndices' and puts the new attributres 
from 'newAttrs' into their places. These changes are made persistent.

*/
void Relation::UpdateTuple( Tuple *tuple, 
                            const vector<int>& changedIndices,
                            const vector<Attribute *>& newAttrs )
{
  long oldExtSize = tuple->GetExtSize(),
       oldSize = tuple->GetSize();
  tuple->UpdateAttributes(changedIndices, newAttrs);
  privateRelation->relDesc.totalExtSize = 
    privateRelation->relDesc.totalExtSize - 
    oldExtSize + tuple->GetExtSize();
  privateRelation->relDesc.totalSize = 
    privateRelation->relDesc.totalSize - 
    oldSize + tuple->GetSize();
}

/*
Deletes the tuple from the relation, FLOBs and SMIRecord are deleted
and the size of the relation is adjusted.

*/
bool Relation::DeleteTuple( Tuple *tuple )
{
  Attribute* nextAttr;
  FLOB* nextFLOB;
  long tupleExtSize = tuple->GetExtSize(),
       tupleSize = tuple->GetSize();
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
    privateRelation->relDesc.totalExtSize -= tupleExtSize;
    privateRelation->relDesc.totalSize -= tupleSize;
    privateRelation->relDesc.noTuples -= 1;
    return true;
  }
  else
    return false;
}

#endif
