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

bool Tuple::Open(SmiRecordFile* tuplefile,
                 SmiFileId lobfileId,
                 SmiRecord& record) {
  this->tupleFile = tuplefile;
  this->lobFileId = lobfileId;
  return ReadFrom(record);
}

bool Tuple::OpenPartial( TupleType* newtype, const list<int>& attrIdList,
                         SmiRecordFile *tuplefile,
                         SmiFileId lobfileId,
                         SmiRecord& record) {
  TRACE_ENTER
  DEBUG(this, "OpenPartial using PrefetchingIterator")

  this->tupleFile = tuplefile;
  this->lobFileId = lobfileId;

  uint16_t rootSize = 0;
  char* data = GetSMIBufferData(record, rootSize);

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