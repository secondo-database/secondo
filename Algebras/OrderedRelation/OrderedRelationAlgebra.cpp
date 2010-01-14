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

// #define DEBUG_OREL

#ifdef DEBUG_OREL
#define DEBUG_OREL2
#endif

/*
3 Implementation
3.1 OrderedRelationIterator

*/
OrderedRelationIterator::OrderedRelationIterator(const OrderedRelation* orel,
                                                TupleType* newType /*=0*/,
                                                const CompositeKey& from,
                                                const CompositeKey& to):
              tupleType(orel->tupleType), outtype(newType),
              tupleFile(orel->tupleFile), lobFileId(orel->lobFileId),
              tupleId(-1) {
#ifdef DEBUG_OREL
cout << "Konstruktor_OrelIter" << endl;
#endif
  tupleType->IncReference();
  if(outtype!=0) outtype->IncReference();
  endOfScan = true;
  if(from.IsDefined() || to.IsDefined()) {
    if(from.IsDefined()) {
      if(to.IsDefined()) {
        it = orel->tupleFile->SelectRangePrefetched(from.GetSmiKey(),
                                                    to.GetSmiKey());
      } else {
        it = orel->tupleFile->SelectRightRangePrefetched(from.GetSmiKey());
      }
    } else {
      it = orel->tupleFile->SelectLeftRangePrefetched(to.GetSmiKey());
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
  if(t->OpenOrel(lobFileId, it, tupleId)) {
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
  if(t->OpenPartialOrel(outtype, attrList, lobFileId, it, tupleId)) {
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
  return tupleId;
}

const CompositeKey& OrderedRelationIterator::GetKey() const {
#ifdef DEBUG_OREL
cout << "GetKey" << endl;
#endif
  return key;
}

bool OrderedRelationIterator::Advance() {
  if(endOfScan) return false;
  if(!it->Next()) {
    key = CompositeKey();
    endOfScan = true;
    return false;
  }
  SmiKey k;
  it->CurrentKey(k);
  key = CompositeKey(k);
  
  tupleId = key.GetAppendix();
  return (tupleId>=0);
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
  GetKeyStructure(typeInfo, keyElement, keyElemType);
  if(createFiles) {
    tupleFile = new SmiBtreeFile(SmiKey::Composite);
    tupleFile->Create();
    tupleFileId = tupleFile->GetFileId();
    hasLobs = false;
    lobFileId = 0;
    if (tupleType->NumOfFlobs() > 0) {
      hasLobs = true;
      SmiRecordFile* lobFile = SecondoSystem::GetFLOBCache()->CreateFile(false);
      lobFileId = lobFile->GetFileId();
    }
  }
  noTuples = 0;
  maxId = 0;
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
  }
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
    SecondoSystem::GetFLOBCache()->Drop( orel->lobFileId, false );
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
  valueRecord.Read(orel->maxId);
  
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
  
  orel->tupleFile = new SmiBtreeFile(SmiKey::Composite);
  orel->tupleFile->Open(orel->tupleFileId);
  
  orel->hasLobs = (orel->lobFileId!=0);

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
  valueRecord.Write(orel->maxId);
  
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
    clone->AppendTuple(t);
    t->DeleteIfAllowed();
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
  if ( listutils::isOrelDescription(typeInfo) ) {
    return true;
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
  tupleFile->Truncate();
  if(hasLobs) {
    SecondoSystem::GetFLOBCache()->Truncate( lobFileId, false );
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
  SmiRecord record;
  TupleId extension = maxId++;
  bool rc = tupleFile->InsertRecord(GetKey(t, true, extension).GetSmiKey(),
                                    record);
  assert(rc==true);
  t->SaveOrel(&record, lobFileId, totalExtSize, totalSize, attrExtSize,
              attrSize, false, extension);
  record.Finish();
  noTuples++;
}


Tuple* OrderedRelation::GetTuple(const TupleId& id) const {
#ifdef DEBUG_OREL
cout << "GetTuple_Orel" << endl;
#endif
  return 0;
}


Tuple* OrderedRelation::GetTuple(const TupleId& id, const int attrIndex,
                const vector<pair<int, int> >& intervals) const {
  return GetTuple(id);
}


Tuple* OrderedRelation::GetTuple(const CompositeKey& key) const {
  Tuple* t = 0;
  SmiKeyedFileIterator iter;
  SmiRecord record;
  if(tupleFile->SelectRecord(key.GetSmiKey(), record)) {
      t = new Tuple(tupleType);
      t->OpenOrel(lobFileId, record, key.GetAppendix());
  }
  return t;
}


Tuple* OrderedRelation::GetTuple(const CompositeKey& key, const int attrIndex,
                const vector<pair<int, int> >& intervals) const {
  Tuple* t = 0;
  if((t=GetTuple(key))!=0)
    t->GetAttribute(attrIndex)->Restrict(intervals);
  return t;
}

bool OrderedRelation::DeleteTuple(Tuple* t) {
  GetKey(t, true, t->GetTupleId());
  return false;
}

void OrderedRelation::UpdateTuple( Tuple *tuple,
                                   const vector<int>& changedIndices,
                                   const vector<Attribute *>& newAttrs ) {
  tuple->UpdateAttributes(changedIndices, newAttrs,
                          relDesc.totalExtSize,
                          relDesc.totalSize,
                          relDesc.attrExtSize,
                          relDesc.attrSize );

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

GenericRelationIterator* OrderedRelation::MakeRangeScan(
                                                  const CompositeKey& from,
                                                  const CompositeKey& to) const{
#ifdef DEBUG_OREL
cout << "MakeRangeScan_Orel" << endl;
#endif
  return new OrderedRelationIterator(this, 0, from, to);
}


GenericRelationIterator* OrderedRelation::MakeRangeScan(TupleType* tt,
                                                  const CompositeKey& from,
                                                  const CompositeKey& to) const{
#ifdef DEBUG_OREL
cout << "MakeRangeScan(tt)_Orel" << endl;
#endif
  return new OrderedRelationIterator(this, tt, from, to);
}


bool OrderedRelation::GetTupleFileStats(SmiStatResultType& result) {
#ifdef DEBUG_OREL
cout << "GetTupleFileStats_Orel" << endl;
#endif
  result = tupleFile->GetFileStatistics(SMI_STATS_EAGER);
  std::stringstream fileid;
  fileid << tupleFileId;
  result.push_back(pair<string,string>("FilePurpose",
            "OrderedRelationTupleCoreFile"));
  result.push_back(pair<string,string>("FileId",fileid.str()));
  return true;
}


bool OrderedRelation::GetLOBFileStats(SmiStatResultType& result) {
#ifdef DEBUG_OREL
cout << "GetLOBFileStats_Orel" << endl;
#endif
  if( !hasLobs ){
    return true;
  }
  SmiRecordFile lobFile(false);
  if( !lobFile.Open( lobFileId ) ){
    return false;
  } else {
    result = lobFile.GetFileStatistics(SMI_STATS_EAGER);
    result.push_back(pair<string,string>("FilePurpose",
                                         "OrderedRelationTupleLOBFile"));
    std::stringstream fileid;
    fileid << lobFileId;
    result.push_back(pair<string,string>("FileId",fileid.str()));
  }
  if( !lobFile.Close() )
    return false;
  return true;
}

CompositeKey OrderedRelation::GetKey(const Tuple* t, const bool appendNumber,
                                     const TupleId appendix) {
  return CompositeKey(t, keyElement, keyElemType, appendNumber, appendix);
}

CompositeKey OrderedRelation::GetRangeKey(Word& arg, int length, bool upper) {
  Word val;
  Supplier son;
  vector<void*> attributes(length);
  vector<SmiKey::KeyDataType> attrTypes(length);
  for(SmiSize i = 0; (i < (SmiSize)length); i++) {
    son = qp->GetSupplier(arg.addr, i);
    qp->Request(son, val);
    attributes[i] = val.addr;
    attrTypes[i] = keyElemType[i];
  }
  return CompositeKey(attributes, attrTypes, upper);
}

CompositeKey OrderedRelation::GetLowerRangeKey(Word& arg, int length) {
#ifdef DEBUG_OREL
cout << "GetLowerRangeKey" << endl;
#endif
  return GetRangeKey(arg, length, false);
}

CompositeKey OrderedRelation::GetUpperRangeKey(Word& arg, int length) {
#ifdef DEBUG_OREL
cout << "GetUpperRangeKey" << endl;
#endif
  return GetRangeKey(arg, length, true);
}

bool OrderedRelation::GetKeyStructure(ListExpr typeInfo,
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
  for(int count = 0; count < keyCount; count++) {

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
  return true;
}

OrderedRelation::OrderedRelation() :
    tupleType(new TupleType(nl->TheEmptyList())) {
#ifdef DEBUG_OREL
cout << "Konstruktor_Orel" << endl;
#endif
  noTuples = 0;
  maxId = 0;
}

enum RangeKind {
  LeftRange,
  RightRange,
  Range
};
template<RangeKind rk> ListExpr ORangeTypeMap(ListExpr args) {
#ifdef DEBUG_OREL
cout << "RangeTypeMap" << endl;
#endif
#ifdef DEBUG_OREL2
cout << nl->ToString(args) << endl;
#endif
  string op= rk==LeftRange?"oleftrange":(rk==RightRange?"orightrange":"orange");
  int length = rk==Range?3:2;
  string typelist = "(orel (tuple (a1:t1...an:tn)) (ai1...ain)) x (ti1...tik)";
  if(rk==Range) typelist += " x (ti1...til)";
  if(nl->ListLength(args) != length) {
    ErrorReporter::ReportError("Operator " + op + " expects a list of type "
                                + typelist);
    return nl->TypeError();
  }
  if(!listutils::isOrelDescription(nl->First(args))) {
    ErrorReporter::ReportError("Operator " + op + " expects " + OREL + 
                                " as first argument");
    return nl->TypeError();
  }
  bool ok = true;
  ListExpr keyList = nl->Third(nl->First(args));
  int t_size = nl->ListLength(keyList);
  vector<string> keyTypes(t_size);
  int count = 0;
  while (!nl->IsEmpty(keyList)) {
    ListExpr attrType;
    ListExpr current = nl->First(keyList);
    keyList = nl->Rest(keyList);
    listutils::findAttribute(nl->Second(nl->Second(nl->First(args))),
                              nl->SymbolValue(current), attrType);
    keyTypes[count++] = nl->SymbolValue(attrType);
  }
  ListExpr keyInfo = nl->Second(args);
  if(nl->IsAtom(keyInfo))
    return nl->TypeError();
  length = nl->ListLength(keyInfo);
  if((length==0) || (length > t_size)) {
    ErrorReporter::ReportError("Zero length or too long key list!");
    return nl->TypeError();
  }
  for (int i=0;ok && i<length;i++) {
    ListExpr current = nl->First(keyInfo);
    keyInfo = nl->Rest(keyInfo);
    if(!nl->IsAtom(current))
      return nl->TypeError();
    ok = ok && (keyTypes[i] == nl->SymbolValue(current));
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
    if(nl->IsAtom(keyInfo))
      return nl->TypeError();
    length = nl->ListLength(keyInfo);
    if((length==0) || (length > t_size)) {
      ErrorReporter::ReportError("Zero length or too long key list!");
      return nl->TypeError();
    }
    for (int i=0;ok && i<length;i++) {
      ListExpr current = nl->First(keyInfo);
      keyInfo = nl->Rest(keyInfo);
      if(!nl->IsAtom(current))
        return nl->TypeError();
      ok = ok && (keyTypes[i] == nl->SymbolValue(current));
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
      CompositeKey fromKey;
      CompositeKey toKey;
      int l1 = 0;
      int l2 = 0;
      if(rk==Range) {
        l1 = ((CcInt*)args[3].addr)->GetIntval();
        l2 = ((CcInt*)args[4].addr)->GetIntval();
      } else {
        l1 = ((CcInt*)args[2].addr)->GetIntval();
      }
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

ListExpr CompKeyProperty() {
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"no listrepresentation");
  ListExpr listrep = nl->TextAtom();
  nl->AppendText(listrep, "no listrepresentation");
  return nl->TwoElemList(
      nl->FourElemList( nl->StringAtom("Signature"),
                        nl->StringAtom("ExampleTypeList"),
                        nl->StringAtom("List Rep"),
                        nl->StringAtom("Example List")),
      nl->FourElemList( nl->StringAtom("-> DATA"),
                        nl->StringAtom("compkey"),
                        listrep,
                        examplelist));
}



TypeConstructor cppcompkey( "compkey", CompKeyProperty,
                          CompositeKey::Out, CompositeKey::In,
                          0, 0,
                          CompositeKey::Create, CompositeKey::Delete,
                          CompositeKey::Open, CompositeKey::Save,
                          CompositeKey::Close, CompositeKey::Clone,
                          CompositeKey::Cast, CompositeKey::SizeOf,
                          CompositeKey::CheckKind);


                          
class OrderedRelationAlgebra : public Algebra {
  public:
    OrderedRelationAlgebra() : Algebra() {
      AddTypeConstructor(&cpporel);
      cpporel.AssociateKind("REL");
      
      AddTypeConstructor(&cppcompkey);
      cppcompkey.AssociateKind("DATA");
      
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