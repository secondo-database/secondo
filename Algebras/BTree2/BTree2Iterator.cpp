/*
----
This file is part of SECONDO.

Copyright (C) 2004-2010, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Fre PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

1 Template Header File: BTree2Node

Jan. 2010 M.Klocke, O.Feuer, K.Teufel

1.1 Overview

This class is used for iterating linearly along the leaf nodes.

*/

#include "BTree2Iterator.h"

#include "BTree2.h"

namespace BTree2Algebra {

BTree2Iterator::BTree2Iterator() {
  // Standard initialization
  btree = 0;
  refvalue = 0;
  refkey = 0;
}

BTree2Iterator::BTree2Iterator(BTree2* bt, NodeId ni, int ei, Attribute* ri, 
                                Attribute* ki) {
  btree = bt;
  nodeId = ni;
  entryNumber = ei;
  refvalue = ri;
  refkey = ki;
}

BTree2Iterator::~BTree2Iterator() {
  // If the iterator has fetched Attribute objects, try to free them
  if (refvalue != 0) { 
    refvalue->DeleteIfAllowed();
  }
  if (refkey != 0) { 
    refkey->DeleteIfAllowed();
  }
}

Attribute* BTree2Iterator::operator*() {
  // If we have already fetched an Attribute object, use that
  if (refvalue != 0) {  
    return refvalue;
  }
  // Otherwise ask BTree to give it to us
  refvalue = btree->GetEntryValue(nodeId,entryNumber);
  return refvalue;
}

Attribute* BTree2Iterator::key() {
  // If we have already fetched an Attribute object, use that
  if (refkey != 0) { 
    return refkey;
  }
  // Otherwise ask BTree to give it to us
  refkey = btree->GetEntryKey(nodeId,entryNumber);
  return refkey;
}

BTree2Iterator& BTree2Iterator::operator++() {
  // If the iterator has fetched Attribute objects, try to free them
  if (refvalue != 0) {
    refvalue->DeleteIfAllowed();
    refvalue = 0;
  }
  if (refkey != 0) {
    refkey->DeleteIfAllowed();
    refkey = 0;
  }
  btree->GetNext(nodeId,entryNumber);
  return *this;
}

bool BTree2Iterator::next()
{
  // If the iterator has fetched Attribute objects, try to free them
  if (refvalue != 0) {
    refvalue->DeleteIfAllowed();
    refvalue = 0;
  }
  if (refkey != 0) {
    refkey->DeleteIfAllowed();
    refkey = 0;
  }
  return btree->GetNext(nodeId,entryNumber,refkey,refvalue);
}

bool BTree2Iterator::operator!=(const BTree2Iterator& x) {
  return ((x.btree != btree) || (x.nodeId != nodeId) || 
            (x.entryNumber != entryNumber));
}

} // end namespace
