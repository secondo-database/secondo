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

[1] Implementation of Module Ordered Relation Algebra

Winter 2009 Nicolai Voget

[TOC]

1 Overview

This file provides some extensions for the class Tuple which is defined in
RelationAlgebra.h.

2 Defines, includes, and constants

*/
#include "RelationAlgebra.h"

void Tuple::Save(SmiRecord* record, SmiFileId& lobFileId, double& extSize,
                 double& size, vector<double>& attrExtSize,
                 vector<double>& attrSize, bool ignorePersistentLOBs) {
  this->lobFileId = lobFileId;
  extSize += tupleType->GetTotalSize();
  size += tupleType->GetTotalSize();
  size_t attrSizes = tupleType->GetTotalSize();
  size_t extensionSize = CalculateBlockSize(attrSizes, extSize, size,
                                            attrExtSize, attrSize,
                                            ignorePersistentLOBs);
  char* data = WriteToBlock(attrSizes,extensionSize);
  bool rc = record->Write(data, sizeof(uint16_t)+attrSizes+extensionSize, 0);
  assert(rc==true);
  free(data);
}

#ifdef USE_SERIALIZATION

bool Tuple::OpenNoId( SmiRecordFile *tuplefile,
                  SmiFileId lobfileId,
                  PrefetchingIterator *iter )
{
  TRACE_ENTER
  DEBUG(this, "Open::Prefetch")
  
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

#else

bool Tuple::OpenNoId( SmiRecordFile *tuplefile,
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

#endif


bool Tuple::OpenPartialNoId( TupleType* newtype, const list<int>& attrIdList,
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