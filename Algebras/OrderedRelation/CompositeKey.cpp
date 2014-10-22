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
#include "CompositeKey.h"
#include "RelationAlgebra.h"
#include "LongInt.h"

CompositeKey::CompositeKey(const Tuple* t, const vector<int>& keyElements,
                           const vector<SmiKey::KeyDataType>& keyElemTypes,
                           const bool append, const TupleId appendix):
                           IndexableAttribute(false),data(0) {

  if((!append) xor (false)) {
    return;
  }

  if(keyElements.size() != keyElemTypes.size())
    return;

  vector<void*> attributes(keyElements.size());
  for(SmiSize i=0; i<keyElements.size(); i++) {
    attributes[i] = t->GetAttribute(keyElements[i]);
  }

  init(attributes, keyElemTypes, (append?appendNumber:none), appendix);
}

CompositeKey::CompositeKey(const vector<void*>& attributes,
                           const vector<SmiKey::KeyDataType>& attrTypes,
                           const bool upper):
                           IndexableAttribute(false),data(0) {
  if(attributes.size() != attrTypes.size())
    return;
  init(attributes, attrTypes, (upper?upperRange:none));
}

CompositeKey::CompositeKey(SmiKey& key):
  IndexableAttribute(false),data(0) {
  kdt = key.GetType();
  if(kdt == SmiKey::Unknown)
    return;

  int32_t iVal;
  int32_t lVal;
  double dVal;
  string sVal;
  switch(kdt) {
    case SmiKey::Integer:
      key.GetKey(iVal);
      charsize = sizeof(iVal);
      data = malloc(charsize);
      memcpy(data, &iVal, charsize);
      break;
    case SmiKey::Longint:
      key.GetKey(lVal);
      charsize = sizeof(lVal);
      data = malloc(charsize);
      memcpy(data, &lVal, charsize);
      break;
    case SmiKey::Float:
      key.GetKey(dVal);
      charsize = sizeof(dVal);
      data = malloc(charsize);
      memcpy(data, &dVal, charsize);
      break;
    case SmiKey::String:
      key.GetKey(sVal);
      charsize = sVal.length();
      data = malloc(charsize);
      sVal.copy((char*)data, charsize);
      break;
    case SmiKey::Composite:
      charsize = key.GetLength();
      key.GetKey(this);
      return;
    default: assert(false);
  }
  SetDefined(true);
}

CompositeKey::CompositeKey(SmiRecord& record):
  IndexableAttribute(true),data(0) {
  SmiSize offset = 0;
  init(record, offset);
}

CompositeKey::CompositeKey(SmiRecord& record, SmiSize& offset):
  IndexableAttribute(true),data(0) {
  init(record, offset);
}

CompositeKey::CompositeKey(PrefetchingIterator* iter):
  IndexableAttribute(true),data(0) {
  SmiSize offset = 0;
  init(iter, offset);
}

CompositeKey::CompositeKey(PrefetchingIterator* iter, SmiSize& offset):
  IndexableAttribute(true),data(0) {
  init(iter, offset);
}

CompositeKey::CompositeKey() :
  IndexableAttribute(false),data(0){
}

CompositeKey::CompositeKey(const CompositeKey& src): IndexableAttribute(src),
  kdt(src.kdt),charsize(src.charsize),data(0){
   if(src.data){
     data = malloc(charsize);
     memcpy(data,src.data,charsize);
   }
}

CompositeKey& CompositeKey::operator=(const CompositeKey& src){
  IndexableAttribute::operator=(src);
  kdt = src.kdt;
  charsize = src.charsize;
  if(src.data){
    if(data){
       data = realloc(data,charsize);
    } else {
       data = malloc(charsize);
    }
    memcpy(data,src.data,charsize);
  } else {
    if(data){
     free(data);
     data = 0;
    }
  }
  return *this;
}





void CompositeKey::WriteTo(char* dest) const {
  if(IsDefined())
    memcpy(dest, data, charsize);
}

void CompositeKey::ReadFrom(const char* src) {
  if(charsize > SMI_MAX_KEYLEN)
    charsize = SMI_MAX_KEYLEN;
  if(!data){
     data = malloc(charsize);
  } else {
     data = realloc(data, charsize);
  }
  memcpy(data,src,charsize);
  SetDefined(true);
}

SmiSize CompositeKey::SizeOfChars() const {
  if(IsDefined())
    return charsize;
  return 0;
}



size_t CompositeKey::Sizeof() const {
  return sizeof(this);
}

int CompositeKey::Compare(const Attribute* attr) const {
  const CompositeKey* other = static_cast<const CompositeKey*>(attr);
  if(!IsDefined() && !other->IsDefined()) return 0;
  if(!IsDefined()) return -1;
  if(!other->IsDefined()) return 1;
  if(other->charsize == charsize) {
    return memcmp(data,other->data,charsize);
  }
  else if(other->charsize < charsize) {
    if(memcmp(data,other->data,other->charsize)>=0) {
      return 1;
    } else {
      return -1;
    }
  } else {
    if(memcmp(data,other->data,charsize)<=0) {
      return -1;
    } else {
      return 1;
    }
  }
}

inline bool CompositeKey::Adjacent(const Attribute* attr) const {
  return false;
}

inline CompositeKey* CompositeKey::Clone() const {
  CompositeKey* result = new CompositeKey();
  result->CopyFrom(this);
  return result;
}

inline size_t CompositeKey::HashValue() const {
  return charsize;
}

void CompositeKey::CopyFrom(const Attribute* attr) {
cout << "Composite::CopyFrom" << endl;
  const CompositeKey* other = (CompositeKey*)attr;
  if(data){
    free(data);
    data = 0;
  }
  SetDefined(other->IsDefined());
  if(IsDefined()) {
    kdt = other->kdt;
    charsize = other->charsize;
    data = malloc(charsize);
    memcpy(data,other->data,charsize);
  } else {
    data = 0;
  }
}

SmiKey::KeyDataType CompositeKey::GetType() const {
  if(!IsDefined())
    return SmiKey::Unknown;
  return kdt;
}

bool CompositeKey::WriteToRecord(SmiRecord& record, SmiSize& offset) const {
  if(!IsDefined())
    return false;

  if(record.Write(&kdt, sizeof(kdt), offset) != sizeof(kdt))
    return false;
  offset += sizeof(kdt);

  if((kdt == SmiKey::Composite) || (kdt == SmiKey::String)) {
    if(record.Write(&charsize, sizeof(charsize), offset) != sizeof(charsize))
      return false;
    offset += sizeof(charsize);
  }

  if(record.Write(data, charsize, offset) != charsize)
    return false;
  offset += charsize;
  return true;
}

SmiKey CompositeKey::GetSmiKey() const {
  if(!IsDefined())
    return SmiKey();

  int32_t iVal;
  int64_t lVal;
  double dVal;
  string sVal;
  switch(kdt) {
    case SmiKey::Integer:
      if(charsize != sizeof(iVal))
        break;
      memcpy(&iVal, data, charsize);
      return SmiKey(iVal);
    case SmiKey::Longint:
      if(charsize != sizeof(lVal))
        break;
      memcpy(&lVal, data, charsize);
      return SmiKey(lVal);
    case SmiKey::Float:
      if(charsize != sizeof(dVal))
        break;
      memcpy(&dVal, data, charsize);
      return SmiKey(dVal);
    case SmiKey::String:
      sVal.assign((char*)data, charsize);
      return SmiKey(sVal);
    case SmiKey::Composite:
      return SmiKey(this);
    default: assert(false);
  }
  return SmiKey();
}

TupleId CompositeKey::GetAppendix() const {
  TupleId lVal;
  SmiKey::Unmap((void*)((char*)(data)+charsize-sizeof(lVal)), lVal);
  return lVal;
}

CompositeKey::~CompositeKey() {
   if(data){
      free(data);
      data = 0;
   }
}

ListExpr CompositeKey::Out(const ListExpr typeInfo, Word value) {
  return nl->TheEmptyList();
};

Word CompositeKey::In(const ListExpr typeInfo, const ListExpr value,
                const int errorPos, ListExpr& errorInfo, bool& correct) {
// cout << "Composite::IN" << endl;
  correct = false;
  return SetWord((void*)0);
}

Word CompositeKey::Create(const ListExpr typeInfo) {
// cout << "Composite::Create" << endl;
  return SetWord(new CompositeKey());
}

void CompositeKey::Delete(const ListExpr typeInfo, Word& value) {
// cout << "Composite::Delete" << endl;
  delete (CompositeKey*)(value.addr);
  value.addr = 0;
}

bool CompositeKey::Open(SmiRecord& valueRecord, size_t& offset,
                        const ListExpr typeInfo, Word& value) {
// cout << "Composite::Open" << endl;
  CompositeKey* comp = new CompositeKey();
  bool defined = comp->IsDefined();
  valueRecord.Read(&(defined), sizeof(defined), offset);
  offset += sizeof(defined);
  if(defined) {
    valueRecord.Read(&(comp->kdt), sizeof(comp->kdt), offset);
    offset += sizeof(comp->kdt);
    valueRecord.Read(&(comp->charsize), sizeof(comp->charsize), offset);
    offset += sizeof(comp->charsize);
    comp->data = malloc(comp->charsize);
    valueRecord.Read(comp->data, comp->charsize, offset);
    offset += comp->charsize;
  }
  value = SetWord(comp);
  return (comp!=0);
}

bool CompositeKey::Save(SmiRecord& valueRecord, size_t& offset,
                        const ListExpr typeInfo, Word& value) {
// cout << "Composite::Save" << endl;
  CompositeKey* comp = (CompositeKey*)value.addr;
  bool defined = comp->IsDefined();
  valueRecord.Write(&defined, sizeof(defined), offset);
  offset += sizeof(defined);
  if(defined) {
    valueRecord.Write(&(comp->kdt), sizeof(comp->kdt), offset);
    offset += sizeof(comp->kdt);
    valueRecord.Write(&(comp->charsize), sizeof(comp->charsize), offset);
    offset += sizeof(comp->charsize);
    comp->data = malloc(comp->charsize);
    valueRecord.Write(comp->data, comp->charsize, offset);
    offset += comp->charsize;
  }
  return true;
}

void CompositeKey::Close(const ListExpr typeInfo, Word& value) {
// cout << "Composite::Close" << endl;
  delete (CompositeKey*)(value.addr);
  value.addr = 0;
}

Word CompositeKey::Clone(const ListExpr typeInfo, const Word& value) {
// cout << "Composite::Clone" << endl;
  return SetWord(((CompositeKey*)(value.addr))->Clone());
}

void* CompositeKey::Cast(void* addr) {
// cout << "Composite::Cast" << endl;
  return new (addr) CompositeKey;
}

int CompositeKey::SizeOf() {
// cout << "Composite::SizeOf" << endl;
  return sizeof(CompositeKey);
}

bool CompositeKey::CheckKind(const ListExpr typeInfo, ListExpr& errorInfo) {
  return (nl->IsAtom(typeInfo) &&
          nl->AtomType(typeInfo)==SymbolType &&
          nl->SymbolValue(typeInfo) == "compkey");
}

const bool CompositeKey::operator==(const CompositeKey& other) const {
  return (Compare(&other) == 0);
}

void CompositeKey::init(const vector<void*>& attributes,
                        const vector<SmiKey::KeyDataType>& attrTypes,
                        const Mode mode, const TupleId appendix) {

  SmiSize maxSize = SMI_MAX_KEYLEN-1;
  if(data==0){
     data = malloc(maxSize);
  } else {
     data = realloc(data,maxSize);
  }
  if(mode == appendNumber) {
    maxSize -= sizeof(TupleId);
  }

  charsize = 0;
  int32_t iVal;
  int64_t lVal;
  double dVal;
  string sVal;
  IndexableAttribute* attr;
  SmiSize tmpSize = 0;
  void* tmpData = 0;
  for(SmiSize i=0; (i < attributes.size()) && (charsize < maxSize); i++) {
    switch (attrTypes[i]) {
      case SmiKey::Integer:
        iVal = (int32_t)(static_cast<CcInt*>(attributes[i])->GetValue());
        tmpSize = sizeof(iVal);
        tmpData = malloc(tmpSize);
        SmiKey::Map(iVal, tmpData);
        break;
      case SmiKey::Longint:
        lVal = (int64_t)(static_cast<LongInt*>(attributes[i])->GetValue());
        tmpSize = sizeof(lVal);
        tmpData = malloc(tmpSize);
        SmiKey::Map(lVal, tmpData);
        break;
      case SmiKey::Float:
        dVal = (double)(static_cast<CcReal*>(attributes[i])->GetValue());
        tmpSize = sizeof(dVal);
        tmpData = malloc(tmpSize);
        SmiKey::Map(dVal, tmpData);
        break;
      case SmiKey::String:
        sVal = (string)(static_cast<CcString*>(attributes[i])->GetValue());
        tmpSize = sVal.length() + 1;
        tmpData = malloc(tmpSize);
        sVal.copy((char*)tmpData, tmpSize-1);
        ((char*)tmpData)[tmpSize-1] = 0;
        break;
      default:
        attr = static_cast<IndexableAttribute*>(attributes[i]);
        tmpSize = attr->SizeOfChars();
        tmpData = malloc(tmpSize);
        attr->WriteTo((char*)tmpData);
    }
    if(charsize+tmpSize>maxSize){
      tmpSize = maxSize-charsize;
    }
    memcpy((void*)(((char*)data)+charsize), tmpData, tmpSize);
    free(tmpData);
    tmpData = 0;
    charsize += tmpSize;
  }
  if(mode == appendNumber) {
    SmiKey::Map((TupleId)appendix, (void*)(((char*)data)+charsize));
    charsize += sizeof(TupleId);
    maxSize += sizeof(TupleId);
  } else if(mode == upperRange) {
    while(charsize<maxSize) {
      ((char*)data)[charsize++] = 255;
    }
  }
  if(charsize != maxSize)
    data = realloc(data, charsize);
  kdt = SmiKey::Composite;
  SetDefined(true);
}

void CompositeKey::init(SmiRecord& record, SmiSize& offset) {
  SetDefined(false);
  if(record.Read(&kdt, sizeof(kdt), offset)!=sizeof(kdt))
    return;
  offset += sizeof(kdt);
  switch(kdt) {
    case SmiKey::Integer:
      charsize = sizeof(int32_t);
      break;
    case SmiKey::Longint:
      charsize = sizeof(int64_t);
      break;
    case SmiKey::Float:
      charsize = sizeof(double);
      break;
    case SmiKey::String:
    case SmiKey::Composite:
      if(record.Read(&charsize, sizeof(charsize), offset) != sizeof(charsize))
        return;
      offset += sizeof(charsize);
      break;
    default:
      return;
  }
  if(!data){
     data = malloc(charsize);
  } else {
     data = realloc(data,charsize);
  }
  if(record.Read(data, charsize, offset) != charsize) {
    free(data);
    data = 0;
    return;
  }
  offset += charsize;
  SetDefined( true);
}

void CompositeKey::init(PrefetchingIterator* iter, SmiSize& offset) {
  SetDefined(false);
  if(iter->ReadCurrentData(&kdt, sizeof(kdt), offset) != sizeof(kdt))
    return;
  offset += sizeof(kdt);

  switch(kdt) {
    case SmiKey::Integer:
      charsize = sizeof(int32_t);
      break;
    case SmiKey::Longint:
      charsize = sizeof(int64_t);
      break;
    case SmiKey::Float:
      charsize = sizeof(double);
      break;
    case SmiKey::String:
    case SmiKey::Composite:
      if(iter->ReadCurrentData(&charsize, sizeof(charsize), offset)
                    != sizeof(charsize))
        return;
      offset += sizeof(charsize);
      break;
    default:
      return;
  }
  if(!data){
     data = malloc(charsize);
  } else {
     data = realloc(data,charsize);
  }
  if(iter->ReadCurrentData(data, charsize, offset) != charsize) {
    free(data);
    data = 0;
    return;
  }
  offset += charsize;
  SetDefined(true);
}


