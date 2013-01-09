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

This class is a collection of two interfaces: first, the secondo-specific
functions (like Open,Save,Create,etc.) are declared here as static methods.
Second, this class serves as an interface for the BTree2Impl class,
allowing access to methods without the burden of having to instantiate
keytype and valuetype.

*/

#define FULL_FEATURED
/*
Undefine FULL\_FEATURED for testing purposes: compile time is dramatically
reduced, but only int/tid trees are possible.

*/

#include "BTree2.h"

#include "BTree2Impl.h"
#include "BTree2Node.h"
#include "BTree2Iterator.h"

#include "TypeConstructor.h"
#include "StandardTypes.h"

#include "TupleIdentifier.h"
#include "ListUtils.h"
#include "Symbols.h"

namespace BTree2Algebra {

/*
2.1 Creation of BTree Objects

*/

// Default parameter

int BTree2::defaultMaxKeysize = 16;
int BTree2::defaultMaxValuesize = 16;

bool BTree2::debugPrintTree = false;
bool BTree2::debugPrintCache = false;
bool BTree2::debugPrintNodeLoading = false;

TypeConstructor BTree2::typeConstructor(
  "btree2",                        // name of the type in SECONDO
  BTree2::Property,                // property function describing signature
  BTree2::Out, BTree2::In,         // Out and In functions
  BTree2::SaveToList,
  BTree2::RestoreFromList,         // SaveToList, RestoreFromList functions
  BTree2::Instantiate,
  BTree2::Delete,                  // object creation and deletion
  BTree2::Open, BTree2::Save,       // object open, save
  BTree2::Close, BTree2::Clone,    // close, and clone
  BTree2::Cast,                    // cast function
  BTree2::SizeOf,               // sizeof function
  BTree2::KindCheck );             // kind checking function // TODO!

BTree2::BTree2() {
               header.treeHeight = 0;
               header.multiplicity = multiple;
               header.leafNodeCount = 0;
               header.internalNodeCount = 0;
               header.leafEntryCount = 0;
               header.internalEntryCount = 0;
               header.nodesVisitedCounter = 0;
               header.maxKeysize = defaultMaxKeysize;
               header.maxValuesize = defaultMaxValuesize;

               keytype="";
               valuetype="";
               recordSize=0;
               file=0;
               headerId=0;
               maxNodesInMemory=0;
               extendedKeysFile=0;
               extendedValuesFile=0;
               keyTypeListExpr = nl->TheEmptyList();
               valueTypeListExpr= nl->TheEmptyList();
             }

BTree2* BTree2::Factory(const string& keyTypeString,
                        const string& valueTypeString) {
  BTree2* btree;
  if (keyTypeString == CcInt::BasicType()) {
    if (valueTypeString == TupleIdentifier::BasicType()) {
      btree = new BTree2Impl<int,TupleId>(keyTypeString,valueTypeString);
    }
#ifdef FULL_FEATURED
      else if (valueTypeString == "none") {
      btree = new BTree2Impl<int,NoneType>(keyTypeString,valueTypeString);
    } else if (valueTypeString == "double") {
    btree = new BTree2Impl<int,double>(keyTypeString,valueTypeString);
    } else if (valueTypeString == CcInt::BasicType()) {
    btree = new BTree2Impl<int,int>(keyTypeString,valueTypeString);
    } else if (valueTypeString == CcString::BasicType()) {
    btree = new BTree2Impl<int,string>(keyTypeString,valueTypeString);
    }
#endif
      else {
      btree = new BTree2Impl<int,Attribute*>(keyTypeString,valueTypeString);
    }
  }
#ifdef FULL_FEATURED
    else if (keyTypeString == CcReal::BasicType()) {
    if (valueTypeString == TupleIdentifier::BasicType()) {
    btree = new BTree2Impl<double,TupleId>(keyTypeString,valueTypeString);
    } else if (valueTypeString == "none") {
    btree = new BTree2Impl<double,NoneType>(keyTypeString,valueTypeString);
    } else if (valueTypeString == "double") {
    btree = new BTree2Impl<double,double>(keyTypeString,valueTypeString);
    } else if (valueTypeString == CcInt::BasicType()) {
    btree = new BTree2Impl<double,int>(keyTypeString,valueTypeString);
    } else if (valueTypeString == CcString::BasicType()) {
    btree = new BTree2Impl<double,string>(keyTypeString,valueTypeString);
    } else {
   btree = new BTree2Impl<double,Attribute*>(keyTypeString,valueTypeString);
    }
  } else if (keyTypeString == CcString::BasicType()) {
    if (valueTypeString == "none") {
    btree = new BTree2Impl<string,NoneType>(keyTypeString,valueTypeString);
    } else if (valueTypeString == TupleIdentifier::BasicType()) {
    btree = new BTree2Impl<string,TupleId>(keyTypeString,valueTypeString);
    } else if (valueTypeString == "double") {
    btree = new BTree2Impl<string,double>(keyTypeString,valueTypeString);
    } else if (valueTypeString == CcInt::BasicType()) {
    btree = new BTree2Impl<string,int>(keyTypeString,valueTypeString);
    } else if (valueTypeString == CcString::BasicType()) {
    btree = new BTree2Impl<string,string>(keyTypeString,valueTypeString);
    } else {
   btree = new BTree2Impl<string,Attribute*>(keyTypeString,valueTypeString);
    }
  } else if (keyTypeString == CcBool::BasicType()) {
    if (valueTypeString == "none") {
      btree = new BTree2Impl<bool,NoneType>(keyTypeString,valueTypeString);
    } else if (valueTypeString == TupleIdentifier::BasicType()) {
      btree = new BTree2Impl<bool,TupleId>(keyTypeString,valueTypeString);
    } else if (valueTypeString == "double") {
    btree = new BTree2Impl<bool,double>(keyTypeString,valueTypeString);
    } else if (valueTypeString == CcInt::BasicType()) {
    btree = new BTree2Impl<bool,int>(keyTypeString,valueTypeString);
    } else if (valueTypeString == CcString::BasicType()) {
    btree = new BTree2Impl<bool,string>(keyTypeString,valueTypeString);
    } else {
      btree = new BTree2Impl<bool,Attribute*>(keyTypeString,valueTypeString);
    }
  }
#endif
    else {
    if (valueTypeString == TupleIdentifier::BasicType()) {
      btree = new BTree2Impl<IndexableAttribute*,TupleId>(
                       keyTypeString,valueTypeString);
#ifdef FULL_FEATURED
    } else if (valueTypeString == "none") {
      btree = new BTree2Impl<IndexableAttribute*,NoneType>(
                       keyTypeString,valueTypeString);
    } else if (valueTypeString == "double") {
    btree = new BTree2Impl<IndexableAttribute*,double>(
                               keyTypeString,valueTypeString);
    } else if (valueTypeString == CcInt::BasicType()) {
    btree = new BTree2Impl<IndexableAttribute*,int>(
                               keyTypeString,valueTypeString);
    } else if (valueTypeString == CcString::BasicType()) {
    btree = new BTree2Impl<IndexableAttribute*,string>(
                               keyTypeString,valueTypeString);
#endif
    } else {
      btree = new BTree2Impl<IndexableAttribute*,Attribute*>(
                       keyTypeString,valueTypeString);
    }
  }
  return btree;
}

/*
2.2 The secondo specific part

*/

ListExpr BTree2::Property() {
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,
    "<relation> createbtree2 [<keyattr>,{<f>,<n>}]\n"
    "<stream> createbtree2 [<keyattr>,{<f>,<n>}]\n"
    "<stream> createbtree2 [<f>,<n>,<keyattr>,<valueattr>,<multipl.>]\n"
    "  where <keyattr> is the key, \n"
    "        <f> is the minimum fill factor (optional in case 1+2)\n"
    "        <n> is the record size (optional in case 1+2)\n"
    "        <valueattr> is data directly stored, \n"
    "        <multipl> is 'multiple','uniqueKeys' or 'uniqueKeysMultiData\n");

  return (nl->TwoElemList(
            nl->TwoElemList(nl->StringAtom("Creation"),
           nl->StringAtom("Example Creation")),
            nl->TwoElemList(examplelist,
           nl->StringAtom("(let mybtree = ten "
           "createbtree2 [no])"))));
}

ListExpr BTree2::Out(ListExpr typeInfo, Word  value)
{
  return nl->TheEmptyList();
}

Word BTree2::In(ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct)
{
  correct = false;
  return SetWord(Address(0));
}

ListExpr BTree2::SaveToList(ListExpr typeInfo, Word value)
{
  // TODO: check RestoreFromList
  BTree2 *btree = (BTree2*)value.addr;

  return nl->ThreeElemList(nl->IntAtom(btree->GetBTreeFileId()),
                           nl->IntAtom(btree->GetHeaderId()),
                           nl->IntAtom(btree->GetRecordSize()));
}

Word BTree2::Instantiate(const ListExpr typeInfo)
{
  int keyAlgId = nl->IntValue( nl->First( nl->Second( typeInfo ) ) );
  int keyTypeId = nl->IntValue( nl->Second( nl->Second( typeInfo ) ) );

  string keyTypeString = am->GetTC(keyAlgId,keyTypeId)->Name();

  string valueTypeString;
  if (listutils::isSymbol(nl->Third(typeInfo),"none")) {
    valueTypeString = "none";
  } else {
    int valueAlgId = nl->IntValue( nl->First( nl->Third( typeInfo ) ) );
    int valueTypeId = nl->IntValue( nl->Second( nl->Third( typeInfo ) ) );

    valueTypeString = am->GetTC(valueAlgId,valueTypeId)->Name();
  }

  BTree2* btree = BTree2::Factory(keyTypeString, valueTypeString);

  return SetWord(btree);
}

Word BTree2::RestoreFromList( ListExpr typeInfo, ListExpr value,
                           int errorPos, ListExpr& errorInfo, bool& correct)
{
  int keyAlgId = nl->IntValue(nl->First(nl->Third( typeInfo ))),
      keyTypeId = nl->IntValue(nl->Second(nl->Third( typeInfo)));

  string keyTypeString = am->GetTC(keyAlgId,keyTypeId)->Name();

  int valueAlgId = nl->IntValue( nl->First( nl->Fourth( typeInfo))),
      valueTypeId = nl->IntValue( nl->Second( nl->Fourth( typeInfo)));

  string valueTypeString = am->GetTC(valueAlgId,valueTypeId)->Name();

  SmiFileId fileid = nl->IntValue(nl->First(value));
  SmiRecordId headerId = nl->IntValue(nl->Second(value));
  int recordsize = nl->IntValue(nl->Third(value));

  BTree2* btree = BTree2::Factory(keyTypeString, valueTypeString);

  btree->Open(fileid, headerId, recordsize);

  return SetWord(btree);
}

void BTree2::Close(const ListExpr typeInfo, Word& w)
{
  BTree2* btree = (BTree2*)w.addr;
  btree->WriteHeader();
  delete btree;
}

Word BTree2::Clone(const ListExpr typeInfo, const Word& w)
{
  BTree2 *btree = (BTree2*)w.addr;
  BTree2 *clone = BTree2::Factory(btree->GetKeyType(), btree->GetValueType());
  // btree->SetValueTypeListExpr(nl->Fourth(typeInfo));
  clone->CreateNewBTree(btree->GetFill(), btree->GetRecordSize(),
                         btree->GetMultiplicity(), false);

  if ((!clone->hasValidBTreeFile()) || (!clone->GetBTreeFile()->IsOpen())) {
    return SetWord( Address(0) );
  }

  for (BTree2Iterator iter = btree->begin();
       iter != btree->end();
       ++iter) {
    if (!clone->AppendGeneric(iter.key(),*iter)) {
       return SetWord(Address(0));
    }
  }

  return SetWord(clone);
}

void BTree2::Delete(const ListExpr typeInfo, Word& w)
{
  BTree2* btree = (BTree2*)w.addr;
  if (btree->hasValidBTreeFile() && btree->GetBTreeFile()->IsOpen()) {
    btree->DropCacheFile();
    btree->GetBTreeFile()->Close();
    if (btree->GetExtendedKeysFile() != 0) {
       btree->GetExtendedKeysFile()->Close();
    }
    if (btree->GetExtendedValuesFile() != 0) {
       btree->GetExtendedValuesFile()->Close();
    }

    btree->GetBTreeFile()->Drop();
    if (btree->GetExtendedKeysFile() != 0) {
      btree->GetExtendedKeysFile()->Drop();
    }
    if (btree->GetExtendedValuesFile() != 0) {
      btree->GetExtendedValuesFile()->Drop();
    }
  }

  delete btree;
}

bool BTree2::Open( SmiRecord& valueRecord,
           size_t& offset,
           const ListExpr typeInfo,
           Word& value )
{
  SmiFileId fileid;
  SmiRecordId headerId;
  int recordsize;
  int typeSize;
  char* typeStr;
  int valueSize;
  char* valueStr;
  valueRecord.Read( &typeSize, sizeof(int), offset);
  offset += sizeof(int);

  typeStr = new char[typeSize+1];

  valueRecord.Read( typeStr, typeSize, offset);
  offset += typeSize;
  typeStr[typeSize] = '\0';

  valueRecord.Read( &valueSize, sizeof(int), offset);
  offset += sizeof(int);

  valueStr = new char[valueSize+1];

  valueRecord.Read( valueStr, valueSize, offset);
  offset += valueSize;
  valueStr[valueSize] = '\0';

  valueRecord.Read( &fileid, sizeof(SmiFileId), offset);
  offset += sizeof(SmiFileId);

  valueRecord.Read( &recordsize, sizeof(int), offset);
  offset += sizeof(int);

  valueRecord.Read( &headerId, sizeof(SmiRecordId), offset);
  offset += sizeof(SmiRecordId);

  BTree2 *btree = BTree2::Factory(typeStr,valueStr);
  // btree->SetValueTypeListExpr(nl->Fourth(typeInfo));

  btree->Open(fileid, headerId, recordsize);

  value = SetWord(btree);
  delete [] typeStr;
  delete [] valueStr;
  return true;
}

bool BTree2::Save( SmiRecord& valueRecord,
           size_t& offset,
           const ListExpr typeInfo,
           Word& value )
{
  BTree2 *btree = (BTree2*)value.addr;
  SmiFileId fileid = 0;
  if (btree->hasValidBTreeFile()) {
    fileid = btree->GetBTreeFileId() ;
  }
  int recordsize = btree->GetRecordSize();
  SmiRecordId headerid = btree->GetHeaderId();
  int typesize = btree->GetKeyType().size();
  valueRecord.Write( &typesize, sizeof(int), offset);
  offset += sizeof(int);

  valueRecord.Write( btree->GetKeyType().c_str(), typesize, offset);
  offset += typesize;

  int valuesize = btree->GetValueType().size();
  valueRecord.Write( &valuesize, sizeof(int), offset);
  offset += sizeof(int);

  valueRecord.Write( btree->GetValueType().c_str(), valuesize, offset);
  offset += valuesize;

  valueRecord.Write( &fileid, sizeof(SmiFileId), offset);
  offset += sizeof(SmiFileId);

  valueRecord.Write( &recordsize, sizeof(int), offset);
  offset += sizeof(int);

  valueRecord.Write( &headerid, sizeof(SmiRecordId), offset);
  offset += sizeof(SmiRecordId);
  btree->WriteHeader();

  return true;

}

bool BTree2::KindCheck(ListExpr type, ListExpr& errorInfo)
{
  string str;
  nl->WriteToString(str, type);
  if((!nl->IsAtom(type))
    && (nl->ListLength(type) == 4)
    && nl->Equal(nl->First(type), nl->SymbolAtom("btree2")))
  {
    return true;
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("BTREE2"), type));
    return false;
  }
  return true;
}

void* BTree2::Cast(void* addr)
{
  return ( 0 );
}

int BTree2::SizeOf()
{
  return 0;
}

void BTree2::ReadHeader() {
  SmiRecord record;
  int RecordSelected = file->SelectRecord( headerId, record,
                                              SmiFile::ReadOnly);
  assert( RecordSelected );
  int offset = 0;
  int written = record.Read(&header, sizeof(headerS), 0);
  assert(written == sizeof(headerS));
  assert(sizeof(headerS) <= (unsigned) recordSize);
  offset += sizeof(headerS);
}

void BTree2::WriteHeader() {
  SmiRecord record;
  int RecordSelected = file->SelectRecord( headerId, record,
                                              SmiFile::Update );
  assert( RecordSelected );
  assert(sizeof(headerS) <= (unsigned) recordSize);
  int offset = 0;
  int written = record.Write(&header, sizeof(headerS), 0);
  assert(written == sizeof(headerS));

  offset += sizeof(headerS);
}

/*
2.3 Methods for getting information on the BTree

*/

bool BTree2::GetFileStats( FileEnum fe, SmiStatResultType &result )
{
  if (fe == FILE_CORE) {
    result = file->GetFileStatistics(SMI_STATS_EAGER);
    std::stringstream fileid;
    fileid << file->GetFileId();
    result.push_back(pair<string,string>("FilePurpose",
              "BTree2IndexFile - Core"));
    result.push_back(pair<string,string>("FileId",fileid.str()));
    return true;
  } else if (fe == FILE_EXTENDED_KEYS) {
    if (extendedKeysFile == 0) {
      return false;
    } else {
      result = extendedKeysFile->GetFileStatistics(SMI_STATS_EAGER);
      std::stringstream fileid;
      fileid << extendedKeysFile->GetFileId();
      result.push_back(pair<string,string>("FilePurpose",
                "BTree2IndexFile - Extended Keys File"));
      result.push_back(pair<string,string>("FileId",fileid.str()));
      return true;
    }
  } else if (fe == FILE_EXTENDED_VALUES) {
    if (extendedValuesFile == 0) {
      return false;
    } else {
      result = extendedValuesFile->GetFileStatistics(SMI_STATS_EAGER);
      std::stringstream fileid;
      fileid << extendedValuesFile->GetFileId();
      result.push_back(pair<string,string>("FilePurpose",
                "BTree2IndexFile - Extended Values File"));
      result.push_back(pair<string,string>("FileId",fileid.str()));
      return true;
    }
  } else if (fe == FILE_CACHE) {
      bool temporary = false; // ??? Might be wrong here
      SmiRecordFile* f = new SmiRecordFile(false,0,temporary);
      f->Open(header.cacheFileId);
      result = f->GetFileStatistics(SMI_STATS_EAGER);
      std::stringstream fileid;
      fileid << f->GetFileId();
      result.push_back(pair<string,string>("FilePurpose",
                "BTree2IndexFile - Cache Parameter"));
      result.push_back(pair<string,string>("FileId",fileid.str()));
      f->Close();
      delete f;
      return true;
  }
  return false;
}

/*
2.4 Content related queries

*/

bool BTree2::ProbeIsInternal(NodeId nid) {
  // ProbeIsInternal is static method in BTreeNode. As only
  // one byte is loaded we do not have to instantiate the
  // node with the correct type, so we choose int,int
  return BTreeNode<int,int>::ProbeIsInternal(file, nid);
}

/*
2.7 Global parameter settings

*/

/*static*/
bool BTree2::SetDefaultMaxKeysize(int i) {
  if ((i > 1) && (i < 255)) {
    defaultMaxKeysize = i;
    return true;
  }
  return false;
}

/*static*/
bool BTree2::SetDefaultMaxValuesize(int i) {
  if ((i > 1) && (i < 255)) {
    defaultMaxValuesize = i;
    return true;
  }
  return false;
}

BTree2Iterator BTree2::end() { return BTree2Iterator(this,0,-1); }
} // end namespace std

