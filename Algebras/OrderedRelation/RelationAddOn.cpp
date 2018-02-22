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
#include "Algebras/Relation-C++/RelationAlgebra.h"

using namespace std;

void Tuple::SaveOrel(SmiRecord* record, SmiFileId& lobFileId, double& extSize,
                 double& size, vector<double>& attrExtSize,
                 vector<double>& attrSize, bool ignoreFlobs,
                 TupleId tupleId) {
  extSize += tupleType->GetCoreSize();
  size += tupleType->GetCoreSize();
  size_t coreSize = 0;
  size_t extensionSize = CalculateBlockSize(coreSize, extSize, size,
                                            attrExtSize, attrSize);
  char* data = WriteToBlock(coreSize, extensionSize, ignoreFlobs, 
                            tupleFile, lobFileId);
  bool rc = record->Write(data, sizeof(uint16_t)+coreSize+extensionSize, 0);
  assert(rc==true);
  free(data);
}

bool Tuple::OpenOrel(SmiFileId fileId,SmiFileId lobfileId,
                      SmiRecord& record, TupleId tupleId )
{
  this->tupleId = tupleId;
  this->tupleFile = 0;
  this->lobFileId = lobfileId;
  return ReadFrom( fileId, record );
}

bool Tuple::OpenOrel(SmiFileId fileId, SmiFileId lobfileId,
                  PrefetchingIterator *iter, TupleId tupleId )
{
  TRACE_ENTER
  DEBUG_MSG("Open::Prefetch")
  
  this->tupleFile = 0;
  this->lobFileId = lobfileId;
  this->tupleId = tupleId;
  
  uint16_t rootSize = 0;
  char* data = GetSMIBufferData(iter, rootSize);
  
  if (data) {
    InitializeAttributes(fileId, data);
    free ( data );
    return true;
  }
  else {
    return false;
  }
  
  TRACE_LEAVE
}


bool Tuple::OpenPartialOrel( SmiFileId fileId,
                         TupleType* newtype, const list<int>& attrIdList,
                         SmiFileId lobfileId,
                         PrefetchingIterator *iter,
                         TupleId tupleId)
{
  TRACE_ENTER
  DEBUG_MSG("OpenPartial using PrefetchingIterator")

  this->tupleId = tupleId;
  this->tupleFile = 0;
  this->lobFileId = lobfileId;

  uint16_t rootSize = 0;
  char* data = GetSMIBufferData(iter, rootSize);

  if (data) {
   InitializeSomeAttributes(fileId, attrIdList, data);
   ChangeTupleType(newtype, attrIdList);
   free ( data );
   return true;
  }
  else {
   return false;
  }

  TRACE_LEAVE
}

void Tuple::UpdateAttributesOrel( const vector<int>& changedIndices,
                                  const vector<Attribute*>& newAttrs )
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

      tmpFlob->destroy();
    }

    attributes[index]->DeleteIfAllowed();
    attributes[index] = newAttrs[i];
  }
}
