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

The Ordered Relational Algebra implements the type constructor ~orel~.

For more information see the OrderedRelation.h header file.

2 Defines, includes, and constants

*/
#include "OrderedRelationAlgebra.h"
#include "StandardTypes.h"
#include "ListUtils.h"

#define DEBUG_OREL

#ifdef DEBUG_OREL
#define DEBUG_OREL2
#endif

/*
3 Implementation
3.1 OrderedRelationIterator

*/
OrderedRelationIterator::OrderedRelationIterator(const OrderedRelation* orel,
                                                TupleType* newType /*=0*/,
                                                const SmiKey& from/*=SmiKey()*/,
                                                const SmiKey& to /*=SmiKey()*/):
              tupleType(orel->tupleType), outtype(newType),
              tupleFile(orel->tupleFile), lobFileId(orel->lobFileId),
              fromKey(from), toKey(to) {
#ifdef DEBUG_OREL
cout << "Konstruktor_OrelIter" << endl;
#endif
  tupleType->IncReference();
  if(outtype!=0) outtype->IncReference();
  SmiKey::KeyDataType t1 = fromKey.GetType();
  SmiKey::KeyDataType t2 = toKey.GetType();
  endOfScan = true;
  if(t1 != SmiKey::Unknown || t2 != SmiKey::Unknown) {
    if(t1!=SmiKey::Unknown) {
      if(t2!=SmiKey::Unknown) {
        it = orel->tupleFile->SelectRangePrefetched(fromKey, toKey);
      } else {
        it = orel->tupleFile->SelectRightRangePrefetched(fromKey);
      }
    } else {
      it = orel->tupleFile->SelectLeftRangePrefetched(toKey);
    }
  } else {
    it = orel->tupleFile->SelectAllPrefetched();
  }
  if (it!=0) {
    endOfScan = false;
  }
}

OrderedRelationIterator::~OrderedRelationIterator() {
  tupleType->DeleteIfAllowed();
  if(outtype!=0) outtype->DeleteIfAllowed();
  if(it!=0) delete it;
  return;
}

Tuple* OrderedRelationIterator::GetNextTuple() {
#ifdef DEBUG_OREL
cout << "GetNextTuple_OrelIter" << endl;
#endif
  if(!Advance()) {
    return 0;
  }
  Tuple* t = new Tuple(tupleType);
  if(t->OpenNoId(0, lobFileId, it)) {
    return t;
  } else {
    delete t;
    return 0;
  }
}

Tuple* OrderedRelationIterator::GetNextTuple(const list<int>& attrList) {
  if(!Advance()) {
    return 0;
  }
  Tuple* t = new Tuple(tupleType);
  if(t->OpenPartialNoId(outtype, attrList, 0, lobFileId, it)) {
    return t;
  } else {
    delete t;
    return 0;
  }
}

TupleId OrderedRelationIterator::GetTupleId() const {
#ifdef DEBUG_OREL
cout << "GetTupleId_OrelIter" << endl;
#endif
assert(false);
  return -1;
}

SmiKey OrderedRelationIterator::GetKey() const {
#ifdef DEBUG_OREL
cout << "GetKey" << endl;
#endif
  return key;
}

bool OrderedRelationIterator::Advance() {
  if(endOfScan) return false;
  if(!it->Next()) {
    key = SmiKey();
    endOfScan = true;
    return false;
  }
  it->CurrentKey(key);
  return true;
}

/*
3.2 OrderedRelation

*/
OrderedRelation::OrderedRelation(ListExpr typeInfo, bool createFiles/*=true*/):
  tupleType(new TupleType(nl->Second(typeInfo))),
  attrExtSize(tupleType->GetNoAttributes()),
  attrSize(tupleType->GetNoAttributes()) {
#ifdef DEBUG_OREL
cout << "Konstruktor_Orel(typeInfo)" << endl;
#endif
  GetKeytype(typeInfo, keyType, keyElement, keyElemType);
  if(createFiles) {
    tupleFile = new SmiBtreeFile(keyType, false);
    tupleFile->Create();
    tupleFileId = tupleFile->GetFileId();
    hasLobs = false;
    lobFileId = 0;
    if (tupleType->NumOfFlobs() > 0) {
      hasLobs = true;
      lobFile = new SmiRecordFile(false);
      lobFile->Create();
      lobFileId = lobFile->GetFileId();
    }
  }
  noTuples = 0;
}

OrderedRelation::~OrderedRelation() {
#ifdef DEBUG_OREL
cout << "Destruktor_Orel" << endl;
#endif
  tupleType->DeleteIfAllowed();
  if(tupleFile!=0) {
    tupleFile->Close();
    delete tupleFile;
  }
  if (hasLobs && (lobFile!=0)) {
    lobFile->Close();
    delete lobFile;
  }
}

ListExpr OrderedRelation::Out(const ListExpr typeInfo, Word value) {
#ifdef DEBUG_OREL
cout << "Out_Orel" << endl;
#endif
  OrderedRelation* orel = static_cast<OrderedRelation*>(value.addr);
#ifdef DEBUG_OREL2
cout << orel->noTuples << endl;
#endif
  ListExpr result = nl->TheEmptyList();
  ListExpr last = result, tupleList = result;

  GenericRelationIterator* rit = orel->MakeScan();
  Tuple* t = 0;
  ListExpr tupleTypeInfo = nl->TwoElemList(
      nl->Second(typeInfo),
      nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));

#ifdef DEBUG_OREL2
cout << "Out:\tvor Iteration" << endl;
cout << nl->ToString(tupleTypeInfo) << endl;
int count = 0;
#endif
  while ((t = rit->GetNextTuple()) !=0) {
    tupleList = t->Out(tupleTypeInfo);
    t->DeleteIfAllowed();
#ifdef DEBUG_OREL2
cout << "Tuple No. " << count++ << endl;
cout << nl->ToString(tupleList) << endl;
#endif
    if (result == nl->TheEmptyList()) {
      result = nl->Cons(tupleList, nl->TheEmptyList());
      last = result;
    } else {
      last = nl->Append(last,tupleList);
    }
  }
  delete rit;
  return result;
}

Word OrderedRelation::In(const ListExpr typeInfo, const ListExpr value,
                    const int errorPos, ListExpr& errorInfo, bool& correct) {
#ifdef DEBUG_OREL
cout << "In_Orel" << endl;
#endif
#ifdef DEBUG_OREL2
cout << nl->ToString(typeInfo) << endl;
#endif
  OrderedRelation* orel = new OrderedRelation(typeInfo);
  int tupleno = 0;
  Tuple* t;
  ListExpr list = value;
  ListExpr first;
  correct=true;
  
  ListExpr tupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
      nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));

  while(!nl->IsEmpty(list)) {
    first = nl->First(list);
    list = nl->Rest(list);
    tupleno++;
    t = Tuple::In(tupleTypeInfo, first, tupleno, errorInfo, correct);
    if(correct) {
      orel->AppendTuple(t);
      t->DeleteIfAllowed();
    } else {
      delete orel;
      return SetWord(Address(0));
    }
#ifdef DEBUG_OREL2
cout << "noTuples:" << '\t' << orel->noTuples << endl;
#endif
  }
#ifdef DEBUG_OREL2
cout << nl->ToString(OrderedRelation::Out(typeInfo, SetWord(orel)));
#endif
  return SetWord(orel);
}

ListExpr OrderedRelation::SaveToList(const ListExpr typeInfo, const Word value){
#ifdef DEBUG_OREL
cout << "Save_Orel" << endl;
#endif
  return Out(typeInfo, value);
}

Word OrderedRelation::RestoreFromList(const ListExpr typeInfo,
                                      const ListExpr value, const int errorPos,
                                      ListExpr& errorInfo, bool& correct) {
#ifdef DEBUG_OREL
cout << "Restore_Orel" << endl;
#endif
  return In(typeInfo, value, errorPos, errorInfo, correct);
}

Word OrderedRelation::Create(const ListExpr typeInfo) {
#ifdef DEBUG_OREL
cout << "Create_Orel" << endl;
cout << nl->ToString(typeInfo) << endl;
#endif
  return SetWord(new OrderedRelation(typeInfo));
}

void OrderedRelation::Delete(const ListExpr typeInfo, Word& value) {
#ifdef DEBUG_OREL
cout << "Delete_Orel" << endl;
#endif
  OrderedRelation* orel = static_cast<OrderedRelation*>(value.addr);
  orel->tupleFile->Close();
  orel->tupleFile->Drop();
  delete orel->tupleFile;
  orel->tupleFile = 0; //to prevent ~OrderedRelation from closing tupleFile
  if (orel->hasLobs) {
    orel->lobFile->Close();
    orel->lobFile->Drop();
    delete orel->lobFile;
    orel->lobFile = 0;
  }
  delete orel;
  value.addr = 0;
}

bool OrderedRelation::Open(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value) {
#ifdef DEBUG_OREL
cout << "Open_Orel" << endl;
#endif
//STUB
  OrderedRelation* orel = new OrderedRelation(typeInfo, false);
  
  valueRecord.SetPos(offset);
  
  valueRecord.Read(orel->noTuples);
  valueRecord.Read(orel->tupleFileId);
  valueRecord.Read(orel->lobFileId);
  
  valueRecord.Read(orel->totalExtSize);
  valueRecord.Read(orel->totalSize);
  
  for(int i=0;i<orel->tupleType->GetNoAttributes();i++) {
    valueRecord.Read(orel->attrExtSize[i]);
  }
  for(int i=0;i<orel->tupleType->GetNoAttributes();i++) {
    valueRecord.Read(orel->attrSize[i]);
  }
  
  orel->tupleFile = new SmiBtreeFile(orel->keyType,false);
  orel->tupleFile->Open(orel->tupleFileId);
  
  orel->hasLobs = (orel->lobFileId!=0);
  if (orel->hasLobs) {
    orel->lobFile = new SmiRecordFile(false);
    orel->lobFile->Open(orel->lobFileId);
  }
  
  offset = valueRecord.GetPos();
  value = SetWord(orel);
  return true;
}


bool OrderedRelation::Save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value) {
#ifdef DEBUG_OREL
cout << "Save_Orel" << endl;
#endif
  OrderedRelation* orel = static_cast<OrderedRelation*>(value.addr);
  
  valueRecord.SetPos(offset);
  valueRecord.Write(orel->noTuples);
  
  valueRecord.Write(orel->tupleFileId);
  valueRecord.Write(orel->lobFileId);
  
  valueRecord.Write(orel->totalExtSize);
  valueRecord.Write(orel->totalSize);
  
  for(int i=0;i<orel->tupleType->GetNoAttributes();i++) {
    valueRecord.Write(orel->attrExtSize[i]);
  }
  for(int i=0;i<orel->tupleType->GetNoAttributes();i++) {
    valueRecord.Write(orel->attrSize[i]);
  }
  
  offset = valueRecord.GetPos();
  return true;
}


void OrderedRelation::Close(const ListExpr typeInfo, Word& value) {
#ifdef DEBUG_OREL
cout << "Close_Orel" << endl;
#endif
   delete (static_cast<OrderedRelation*>(value.addr));
}


Word OrderedRelation::Clone(const ListExpr typeInfo, const Word& value) {
#ifdef DEBUG_OREL
cout << "Clone_Orel" << endl;
#endif
  OrderedRelation* clone = new OrderedRelation(typeInfo);
  OrderedRelation* orel = static_cast<OrderedRelation*>(value.addr);
  Tuple* t;
  OrderedRelationIterator* iter = (OrderedRelationIterator*)orel->MakeScan();
  while((t = iter->GetNextTuple()) != 0) {
#ifdef DEBUG_OREL2
cout << "GotNextTuple" << endl;
#endif
    iter->GetKey();
    SmiKey k = iter->GetKey();
#ifdef DEBUG_OREL2
cout << "GotKey" << endl;
#endif
    clone->AppendTuple(t, k);
#ifdef DEBUG_OREL2
cout << "Appended" << endl;
#endif
    t->DeleteIfAllowed();
#ifdef DEBUG_OREL2
cout << "nextTuple" << endl;
#endif
  }
  delete iter;
  return SetWord(clone);
}


void* OrderedRelation::Cast(void* addr) {
#ifdef DEBUG_OREL
cout << "Cast_Orel" << endl;
#endif
  return 0;
}


bool OrderedRelation::CheckKind(const ListExpr typeInfo,
                                        ListExpr& errorInfo) {
#ifdef DEBUG_OREL
cout << "CheckKind_Orel" << endl;
#endif
#ifdef DEBUG_OREL2
cout << nl->ToString(typeInfo) << endl;
cout << nl->SymbolValue(nl->First(typeInfo)) << endl;
#endif
  if ((nl->ListLength(typeInfo) == 3) &&
    nl->IsEqual(nl->First(typeInfo), OREL)) {
    return (am->CheckKind("TUPLE", nl->Second(typeInfo), errorInfo) &&
            ValidKeyElements(nl->Second(typeInfo), nl->Third(typeInfo)));
  }
  else {
    errorInfo = nl->Append(errorInfo,
                           nl->ThreeElemList(nl->IntAtom(80),
                                             nl->SymbolAtom(OREL),
                                             typeInfo));
    return false;
  }
}


int OrderedRelation::GetNoTuples() const {
#ifdef DEBUG_OREL
cout << "GetNoTuples_Orel" << endl;
#endif
  return noTuples;
}


double OrderedRelation::GetTotalRootSize() const {
#ifdef DEBUG_OREL
cout << "GetTotalRootSize_Orel" << endl;
#endif
  return 12;
}


double OrderedRelation::GetTotalRootSize(int i) const {
#ifdef DEBUG_OREL
cout << "GetTotalRootSize[i]_Orel" << endl;
#endif
  return 3;
}


double OrderedRelation::GetTotalExtSize() const {
#ifdef DEBUG_OREL
cout << "GetTotalExtSize_Orel" << endl;
#endif
  return totalExtSize;
}


double OrderedRelation::GetTotalExtSize(int i) const {
#ifdef DEBUG_OREL
cout << "GetTotalExtSize[i]_Orel" << endl;
#endif
  return attrExtSize[i];
}


double OrderedRelation::GetTotalSize() const {
#ifdef DEBUG_OREL
cout << "GetTotalSize_Orel" << endl;
#endif
  return totalSize;
}


double OrderedRelation::GetTotalSize(int i) const {
#ifdef DEBUG_OREL
cout << "GetTotalSize[i]_Orel" << endl;
#endif
  return attrSize[i];
}


void OrderedRelation::Clear() {
#ifdef DEBUG_OREL
cout << "Clear_Orel" << endl;
#endif
  noTuples=0;
  tupleFile->Close();
  tupleFile->Remove();
  tupleFile = new SmiBtreeFile(keyType,false);
  tupleFile->Create();
  tupleFileId = tupleFile->GetFileId();
  if(hasLobs) {
    lobFile->Close();
    lobFile->Remove();
    lobFile = new SmiRecordFile(false);
    lobFile->Create();
    lobFileId = lobFile->GetFileId();
  }
  for(int i;i<tupleType->GetNoAttributes();i++) {
    attrSize[i] = 0.0;
    attrExtSize[i] = 0.0;
  }
  totalExtSize = 0.0;
  totalSize = 0.0;
}


void OrderedRelation::AppendTuple(Tuple* t) {
#ifdef DEBUG_OREL
cout << "AppendTuple_Orel" << endl;
#endif
  SmiKey k;
  GetKey(t, k, keyType, keyElement, keyElemType);
  return AppendTuple(t,k);
#ifdef DEBUG_OREL2
cout << "EndAppendTuple_Orel" << endl;
#endif
}


void OrderedRelation::AppendTuple(Tuple* t, SmiKey& k) {
  SmiRecord record;
  bool rc = tupleFile->InsertRecord(k, record);
  assert(rc==true);
  SmiFileId lobId = 0;//lobFile->GetFileId();
  t->Save(&record, lobId,totalExtSize, totalSize,attrExtSize,attrSize,false);
  record.Finish();
  noTuples++;
}


Tuple* OrderedRelation::GetTuple(const TupleId& id) const {
#ifdef DEBUG_OREL
cout << "GetTuple_Orel" << endl;
#endif
  return new Tuple(nl->TheEmptyList());
}


Tuple* OrderedRelation::GetTuple(const TupleId& id, const int attrIndex,
                const vector<pair<int, int> >& intervals) const {
  return GetTuple(id);
}

GenericRelationIterator* OrderedRelation::MakeScan() const {
#ifdef DEBUG_OREL
cout << "MakeScan_Orel" << endl;
#endif
  return new OrderedRelationIterator(this);
}

GenericRelationIterator* OrderedRelation::MakeScan(TupleType* tt) const {
#ifdef DEBUG_OREL
cout << "MakeScan(tt)_Orel" << endl;
#endif
  return new OrderedRelationIterator(this, tt);
}

GenericRelationIterator* OrderedRelation::MakeRangeScan(const SmiKey& from,
                                                      const SmiKey& to) const {
#ifdef DEBUG_OREL
cout << "MakeRangeScan_Orel" << endl;
#endif
  return new OrderedRelationIterator(this, 0, from, to);
}


GenericRelationIterator* OrderedRelation::MakeRangeScan(TupleType* tt,
                                                        const SmiKey& from,
                                                        const SmiKey& to) const{
#ifdef DEBUG_OREL
cout << "MakeRangeScan(tt)_Orel" << endl;
#endif
  return new OrderedRelationIterator(this, tt, from, to);
}


bool OrderedRelation::GetTupleFileStats(SmiStatResultType&) {
#ifdef DEBUG_OREL
cout << "GetTupleFileStats_Orel" << endl;
#endif
  return false;
}


bool OrderedRelation::GetLOBFileStats(SmiStatResultType&) {
#ifdef DEBUG_OREL
cout << "GetLOBFileStats_Orel" << endl;
#endif
  return false;
}


void OrderedRelation::GetKey(Tuple* t, SmiKey& key,
                             const SmiKey::KeyDataType& keyType,
                             const vector<int>& keyElement,
                             const vector<SmiKey::KeyDataType>& keyElemType){
#ifdef DEBUG_OREL
cout << "GetKey_Orel" << endl;
#endif
  switch (keyType) {
    case SmiKey::Integer:
      key = SmiKey((long int)
      (static_cast<CcInt*>(t->GetAttribute(keyElement[0]))->GetValue()));
      return;
    case SmiKey::String:
      key = SmiKey((string)
      (static_cast<CcString*>(t->GetAttribute(keyElement[0]))->GetValue()));
      return;
    case SmiKey::Float:
      key = SmiKey((double)
      (static_cast<CcReal*>(t->GetAttribute(keyElement[0]))->GetValue()));
      return;
    default:
      size_t maxSize = SMI_MAX_KEYLEN;
      char data[maxSize];
      size_t offset = 0;
      long lValue;
      double fValue;
      string sValue;
      IndexableAttribute* attr;
      size_t tmpSize = 0;
      void* tmpData = 0;
      for(size_t i=0;i<keyElement.size() && offset < maxSize;i++) {
        switch (keyElemType[i]) {
          case SmiKey::Integer:
            lValue = (long int)
            (static_cast<CcInt*>(t->GetAttribute(keyElement[i]))->GetValue());
            tmpSize = sizeof(lValue);
            tmpData = malloc(tmpSize);
            SmiKey::Map(lValue, tmpData);
            break;
          case SmiKey::Float:
            fValue = (double)
            (static_cast<CcReal*>(t->GetAttribute(keyElement[i]))->GetValue());
            tmpSize = sizeof(fValue);
            tmpData = malloc(tmpSize);
            SmiKey::Map(fValue, tmpData);
            break;
          case SmiKey::String:
            sValue = (string)
              (static_cast<CcString*>(t->GetAttribute(keyElement[i]))
              ->GetValue());
            tmpSize = sValue.length() + 1;
            tmpData = malloc(tmpSize);
            sValue.copy((char*)tmpData, sValue.length());
            ((char*)tmpData)[sValue.length()] = 0;
            break;
          default:
            attr = 
              static_cast<IndexableAttribute*>(t->GetAttribute(keyElement[i]));
            tmpSize = attr->SizeOfChars();
            tmpData = malloc(tmpSize);
            attr->WriteTo((char*)tmpData);
        }
        if(offset+tmpSize>maxSize){
          tmpSize = maxSize-offset;
        }
        memcpy(data+offset, tmpData, tmpSize);
        free(tmpData);
        offset += tmpSize;
      }
      CompositeKey comp(data,offset);
      key = SmiKey(&comp);
  }
}

SmiKey OrderedRelation::GetRangeKey(Word& arg, int length, bool lower) {
  Word val;
  Supplier son = qp->GetSupplier(arg.addr, 0);
  qp->Request(son, val);
  switch(keyType) {
    case SmiKey::Integer:
      return SmiKey((long int)
        (static_cast<CcInt*>(val.addr)->GetValue()));
    case SmiKey::String:
      return SmiKey((string)
        (static_cast<CcString*>(val.addr)->GetValue()));
    case SmiKey::Float:
      return SmiKey((double)
        (static_cast<CcReal*>(val.addr)->GetValue()));
    default:
      size_t maxSize = SMI_MAX_KEYLEN;
      char data[maxSize];
      size_t offset = 0;
      size_t tmpSize = 0;
      void* tmpData = 0;
      long lValue;
      double fValue;
      string sValue;
      IndexableAttribute* attr;
      for(size_t i = 0; (i < length) && (offset < maxSize); i++) {
        if(i>0) {
          son = qp->GetSupplier(arg.addr, i);
          qp->Request(son, val);
        }
        switch (keyElemType[i]) {
          case SmiKey::Integer:
            lValue = (long int)
              (static_cast<CcInt*>(val.addr)->GetValue());
            tmpSize = sizeof(lValue);
            tmpData = malloc(tmpSize);
            SmiKey::Map(lValue, tmpData);
            break;
          case SmiKey::Float:
            fValue = (double)
              (static_cast<CcReal*>(val.addr)->GetValue());
            tmpSize = sizeof(fValue);
            tmpData = malloc(tmpSize);
            SmiKey::Map(fValue, tmpData);
            break;
          case SmiKey::String:
            sValue = (string)
              (static_cast<CcString*>(val.addr)->GetValue());
            tmpSize = sValue.length() + 1;
            tmpData = malloc(tmpSize);
            sValue.copy((char*)tmpData, sValue.length());
            ((char*)tmpData)[sValue.length()] = 0;
            break;
          default:
            attr = static_cast<IndexableAttribute*>(val.addr);
            tmpSize = attr->SizeOfChars();
            tmpData = malloc(tmpSize);
            attr->WriteTo((char*)tmpData);
        }
        if(offset+tmpSize>maxSize){
          tmpSize = maxSize-offset;
        }
        memcpy(data+offset, tmpData, tmpSize);
        free(tmpData);
        offset += tmpSize;
      }
      if(offset<maxSize && !lower) {
        data[offset] = 255;
        offset += 1;
      }
      CompositeKey comp(data,offset);
      return SmiKey(&comp);
  }
}

SmiKey OrderedRelation::GetLowerRangeKey(Word& arg, int length) {
#ifdef DEBUG_OREL
cout << "GetLowerRangeKey" << endl;
#endif
  return GetRangeKey(arg, length, true);
}

SmiKey OrderedRelation::GetUpperRangeKey(Word& arg, int length) {
#ifdef DEBUG_OREL
cout << "GetUpperRangeKey" << endl;
#endif
  return GetRangeKey(arg, length, false);
}

bool OrderedRelation::GetKeytype(ListExpr typeInfo,
                                 SmiKey::KeyDataType& keyType,
                                 vector<int>& keyElement,
                                 vector<SmiKey::KeyDataType>& keyElemType) {
#ifdef DEBUG_OREL
cout << "GetKeytype_Orel" << endl;
cout << nl->ToString(typeInfo) << endl;
#endif

  if(nl->ListLength(typeInfo)!=3 || nl->IsAtom(nl->Second(typeInfo))
      || nl->ListLength(nl->Second(typeInfo))!=2) return false;
  //(orel(tuple((a1 t1)...(an tn)) (ai1 ai2 ai3))) or
  //(orel(tuple((a1 t1)...(an tn)) ai)) expected
  
  ListExpr tupleInfo = nl->Second(nl->Second(typeInfo));
  ListExpr keyInfo = nl->Third(typeInfo);
  int keyCount = 0;
  if(nl->IsAtom(keyInfo)) {
    keyCount = 1;
  } else {
    keyCount = nl->ListLength(keyInfo);
  }
  keyElement.resize(keyCount);
  keyElemType.resize(keyCount);
  int algId, typeId;
  for(int count =0;count<keyCount;count++) {

#ifdef DEBUG_OREL2
cout << nl->IsAtom(keyInfo) << endl;
cout << nl->ToString(keyInfo) << endl;
if(nl->IsAtom(keyInfo)) {
cout << nl->SymbolValue(keyInfo) << endl;
}
#endif

    string id;
    if(nl->IsAtom(keyInfo)) {
      id = nl->SymbolValue(keyInfo);
    } else {
      id = nl->SymbolValue(nl->First(keyInfo));
      keyInfo = nl->Rest(keyInfo);
    }
    ListExpr tempInfo = tupleInfo;
    bool found = false;
    for(int i=0;!(nl->IsEmpty(tempInfo)||found);i++) {
      ListExpr current = nl->First(tempInfo);
      if(nl->SymbolValue(nl->First(current)) == id) {
        found = true;
        
#ifdef DEBUG_OREL2
cout << count << '\t' << id << '\t' << i << endl;
cout << nl->ToString(current) << endl;
#endif

        keyElement[count] = i;
        algId = nl->IntValue(nl->First(nl->Second(current)));
        typeId = nl->IntValue(nl->Second(nl->Second(current)));
        string keyTypeString = am->GetTC(algId, typeId)->Name();
        
        if (keyTypeString == "int") {
          keyElemType[count] = SmiKey::Integer;
        } else if(keyTypeString == "string") {
          keyElemType[count] = SmiKey::String;
        } else if(keyTypeString == "real") {
          keyElemType[count] = SmiKey::Float;
        } else {
          keyElemType[count] = SmiKey::Composite;
        }
      }
      tempInfo = nl->Rest(tempInfo);
    }
    if(!found) {
      return false;
    }
  }
  if(keyCount > 1) {
#ifdef DEBUG_OREL2
cout << "more than one" << endl;
#endif
    keyType = SmiKey::Composite;
  } else {
    keyType = keyElemType[0];
#ifdef DEBUG_OREL2
cout << "keyType:\t" << keyType << endl;
#endif
  }
  return true;
}


bool OrderedRelation::ValidKeyElements(const ListExpr tupleInfo,
                                        const ListExpr keyInfo,
                                        vector<string>* types) {
#ifdef DEBUG_OREL
cout << "ValidKeyElements" << endl;
#endif
  int keyCount = 0;
  if(nl->IsAtom(keyInfo)) {
    keyCount = 1;
  } else {
    keyCount = nl->ListLength(keyInfo);
  }
  if(types!=0) {
    types->resize(keyCount);
  }
  ListExpr restInfo = keyInfo;
  for(int count=0; count<keyCount; count++) {
#ifdef DEBUG_OREL2
cout << nl->IsAtom(restInfo) << endl;
cout << nl->ToString(restInfo) << endl;
if(nl->IsAtom(restInfo)) {
  cout << nl->SymbolValue(restInfo) << endl;
} else {
  cout << nl->SymbolValue(nl->First(restInfo)) << endl;
}
#endif
    string id;
    if(nl->IsAtom(restInfo)) {
      id = nl->SymbolValue(restInfo);
    } else {
      id = nl->SymbolValue(nl->First(restInfo));
      restInfo = nl->Rest(restInfo);
    }
    ListExpr tempInfo = nl->Second(tupleInfo);
    ListExpr attrType;
    int attrIndex = listutils::findAttribute(tempInfo, id, attrType);
#ifdef DEBUG_OREL2
cout << count << '\t' << id << '\t' << attrIndex << endl;
#endif
    if(attrIndex==0){
      return false;
    }
#ifdef DEBUG_OREL2
cout << nl->ToString(attrType) << endl;
#endif

    if(!listutils::isSymbol(attrType,"string") &&
      !listutils::isSymbol(attrType,"int") &&
      !listutils::isSymbol(attrType,"real") &&
      !listutils::isKind(attrType,"INDEXABLE")){
      return false;
    }
    if(types!=0) {
      (*types)[count] = nl->SymbolValue(attrType);
    }
  }
  return true;
}


OrderedRelation::OrderedRelation() :
    tupleType(new TupleType(nl->TheEmptyList())) {
#ifdef DEBUG_OREL
cout << "Konstruktor_Orel" << endl;
#endif
  noTuples=0;
}

enum RangeKind {
  LeftRange,
  RightRange,
  Range
};
template<RangeKind rk> ListExpr ORangeTypeMap(ListExpr args) {
// #ifdef DEBUG_OREL
cout << "RangeTypeMap" << endl;
// #endif
// #ifdef DEBUG_OREL2
cout << nl->ToString(args) << endl;
// #endif
  string op =rk==LeftRange?"oleftrange":(rk==RightRange?"orightrange":"orange");
  int length = rk==Range?3:2;
  string typelist = "(orel (tuple (a1:t1...an:tn)) (ai1...ain)) x (ti1...tik)";
  if(rk==Range) typelist += " x (ti1...til)";
  if(nl->ListLength(args) != length) {
    ErrorReporter::ReportError("Operator " + op + " expects a list of type "
                                + typelist);
    return nl->TypeError();
  }
  ListExpr errorInfo = nl->OneElemList(nl->TheEmptyList());
  ListExpr orelInfo = nl->First(args);
  if(!OrderedRelation::CheckKind(orelInfo, errorInfo)) {
    ErrorReporter::ReportError("Operator " + op + " expects " + OREL + 
                                " as first argument");
    return nl->TypeError();
  }
  vector<string> types;
  OrderedRelation::ValidKeyElements(nl->Second(orelInfo),nl->Third(orelInfo),
                                    &types);
  bool ok = true;
  int t_size = types.size();
  int l1 = 0;
  int l2 = 0;
  ListExpr keyInfo = nl->Second(args);
  if(nl->IsAtom(keyInfo)) {
    return nl->TypeError();
    ok = ok && (types[0]==nl->SymbolValue(keyInfo));
  } else {
    if((nl->ListLength(keyInfo)==0) || (nl->ListLength(keyInfo) > t_size)) {
      ErrorReporter::ReportError("Zero length or too long key list!");
      return nl->TypeError();
    }
    length = nl->ListLength(keyInfo);
    for (int i=0;ok && i<length;i++) {
      ListExpr current = nl->First(keyInfo);
      keyInfo = nl->Rest(keyInfo);
      ok = ok && (types[i] == nl->SymbolValue(current));
    }
  }
  if(!ok) {
    string tmpStr = rk==Range?"first ":"";
    ErrorReporter::ReportError("The " + tmpStr +
                                "range has to follow the typeorder of the " +
                                OREL + " key");
    return nl->TypeError();
  }
  if(rk==Range) {
    keyInfo = nl->Third(args);
    if(nl->IsAtom(keyInfo)) {
      return nl->TypeError();
      ok = ok && (types[0]==nl->SymbolValue(keyInfo));
    } else {
      if((nl->ListLength(keyInfo)==0) || (nl->ListLength(keyInfo) > t_size)) {
        ErrorReporter::ReportError("Zero length or too long key list!");
        return nl->TypeError();
      }
      length = nl->ListLength(keyInfo);
      for (int i=0;ok && i<length;i++) {
        ListExpr current = nl->First(keyInfo);
        keyInfo = nl->Rest(keyInfo);
        ok = ok && (types[i] == nl->SymbolValue(current));
      }
    }
    if(!ok) {
      ErrorReporter::ReportError("The second range has to follow the "
                                  "typeorder of the " + OREL + " key");
      return nl->TypeError();
    }
  }
  ListExpr appList;
  if(rk==Range) {
    appList = nl->TwoElemList(nl->IntAtom(nl->ListLength(nl->Second(args))),
                              nl->IntAtom(nl->ListLength(nl->Third(args))));
  } else {
    appList = nl->OneElemList(nl->IntAtom(nl->ListLength(nl->Second(args))));
  }
  return nl->ThreeElemList(nl->SymbolAtom("APPEND"), appList,
        nl->TwoElemList(nl->SymbolAtom("stream"),nl->Second(nl->First(args))));
}

template<RangeKind rk> int ORangeValueMap(Word* args, Word& result, int message,
                                          Word& local, Supplier s) {
  GenericRelationIterator* rit;
  OrderedRelation* r;
  switch(message) {
    case OPEN: {
      SmiKey fromKey;
      SmiKey toKey;
      int l1 = 0;
      int l2 = 0;
      if(rk==Range) {
        l1 = ((CcInt*)args[3].addr)->GetIntval();
        l2 = ((CcInt*)args[4].addr)->GetIntval();
      } else {
        l1 = ((CcInt*)args[2].addr)->GetIntval();
      }
cout << "ValueMap\t" << l1 << "\t" << l2 << endl;
      r = (OrderedRelation*)args[0].addr;
      if(rk==LeftRange) {
        toKey = r->GetUpperRangeKey(args[1],l1);
      } else if(rk==RightRange) {
        fromKey = r->GetLowerRangeKey(args[1],l1);
      } else if(rk==Range) {
        fromKey = r->GetLowerRangeKey(args[1],l1);
        toKey = r->GetUpperRangeKey(args[2],l2);
      }
      rit = r->MakeRangeScan(fromKey,toKey);
      local.addr = rit;
      return 0;
    }
    case REQUEST:
      rit = (GenericRelationIterator*)local.addr;
      Tuple* t;
      if((t = rit->GetNextTuple())) {
        result.setAddr(t);
        return YIELD;
      }
      return CANCEL;
      break;
    case CLOSE:
      if(local.addr) {
        rit = (GenericRelationIterator*)local.addr;
        delete rit;
      }
      return 0;
  }
  return 0;
}

const string OLeftRangeSpec = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(orel (tuple(a1:t1 ... an:tn)) (ai1 ai2 ... ain)) x (ti1 ti2) -> "
  "(stream (tuple(a1:t1 ... an:tn)))</text--->"
  "<text>_ oleftrange [key]</text--->"
  "<text>Returns a stream of tuples where each tuple's key is smaller than or "
  "equal as the given key.</text--->"
  "<text>query cities feed oconsume [BevT,Name] oleftrange[100] count</text--->"
  ") )";

const string ORightRangeSpec = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(orel (tuple(a1:t1 ... an:tn)) (ai1 ai2 ... ain)) x (ti1 ti2) -> "
  "(stream (tuple(a1:t1 ... an:tn)))</text--->"
  "<text>_ orightrange [key]</text--->"
  "<text>Returns a stream of tuples where each tuple's key is greater than or "
  "equal as the given key.</text--->"
  "<text>query cities feed oconsume [BevT,Name] orightrange[1000] count"
  "</text--->) )";

const string ORangeSpec = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(orel (tuple(a1:t1 ... an:tn)) (ai1 ai2 ... ain)) x (ti1 ti2) x "
  "(ti1 ti2 ti3) -> (stream (tuple(a1:t1 ... an:tn)))</text--->"
  "<text>_ orange [leftkey, rightkey]</text--->"
  "<text>Returns a stream of tuples where each tuple's key is between the two "
  "given keys.</text--->"
  "<text>query cities feed oconsume [BevT,Name] orange[500,800] count</text--->"
  ") )";      
  
Operator oleftrange (
                      "oleftrange",              // name
                      OLeftRangeSpec,            // specification
                      ORangeValueMap<LeftRange>, // value mapping
                      Operator::SimpleSelect,    // trivial selection function
                      ORangeTypeMap<LeftRange>   // type mapping
                      );


Operator orightrange (
                      "orightrange",              // name
                      ORightRangeSpec,            // specification
                      ORangeValueMap<RightRange>, // value mapping
                      Operator::SimpleSelect,     // trivial selection function
                      ORangeTypeMap<RightRange>   // type mapping
                      );


Operator orange (
                "orange",               // name
                ORangeSpec,             // specification
                ORangeValueMap<Range>,  // value mapping
                Operator::SimpleSelect, // trivial selection function
                ORangeTypeMap<Range>    // type mapping
                );
                        
                        
                        
                          
                          
ListExpr ORelProperty() {
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"((\"Hagen\" 193)(\"Solingen\" 163))");
  ListExpr listrep = nl->TextAtom();
  nl->AppendText(listrep, "(<tuple>*) where <tuple> is "
  "(<attr1> <attr2> <attr3> .."
  ". <attrn>)");
  return nl->TwoElemList(
      nl->FourElemList( nl->StringAtom("Signature"),
                        nl->StringAtom("ExampleTypeList"),
                        nl->StringAtom("List Rep"),
                        nl->StringAtom("Example List")),
      nl->FourElemList( nl->StringAtom("TUPLE -> REL"),
                        nl->StringAtom("("+OREL+" (tuple "
                                         "((city string)(pop int))) city)"),
                        listrep,
                        examplelist));
}



TypeConstructor cpporel( OREL, ORelProperty,
                          OrderedRelation::Out, OrderedRelation::In,
                          OrderedRelation::SaveToList,
                          OrderedRelation::RestoreFromList,
                          OrderedRelation::Create, OrderedRelation::Delete,
                          OrderedRelation::Open, OrderedRelation::Save,
                          OrderedRelation::Close, OrderedRelation::Clone,
                          OrderedRelation::Cast, OrderedRelation::SizeOf,
                          OrderedRelation::CheckKind);



                          
class OrderedRelationAlgebra : public Algebra {
  public:
    OrderedRelationAlgebra() : Algebra() {
      AddTypeConstructor(&cpporel);
      
      cpporel.AssociateKind("REL");
      AddOperator(&oleftrange);
      AddOperator(&orightrange);
      AddOperator(&orange);
    };
    
    ~OrderedRelationAlgebra() {};
};



extern "C" Algebra* InitializeOrderedRelationAlgebra(NestedList* nlRef,
                                                  QueryProcessor* qpRef) {
  nl = nlRef;
  qp = qpRef;
  am = SecondoSystem::GetAlgebraManager();
  return new OrderedRelationAlgebra();
}