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

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hybrid~). Only the executable
level remains. Models are also removed from type constructors.

[1] Implementation of BTree Algebra

[TOC]

1 Includes and Defines

*/
#include <iostream>
#include <string>
#include <deque>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <typeinfo>

#include "Algebra.h"
#include "SecondoSystem.h"
#include "SecondoCatalog.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DateTime.h"
#include "TupleIdentifier.h"
#include "Progress.h"
#include "FTextAlgebra.h"
#include "ListUtils.h"
#include "NList.h"
#include "LongInt.h"

using namespace std;

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

/*
2 Auxiliary Functions

2.1 Type property of type constructor ~btree~

*/
static ListExpr
BTreeProp()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"<relation> createbtree [<attrname>] where "
  "<attrname> is the key");

  return (nl->TwoElemList(
            nl->TwoElemList(nl->StringAtom("Creation"),
           nl->StringAtom("Example Creation")),
            nl->TwoElemList(examplelist,
           nl->StringAtom("(let mybtree = ten "
           "createbtree [no])"))));
}

/*
2.2 Function ~AttrToKey~

Converts a ~Attribute~ to a ~SmiKey~.

*/
void AttrToKey(
  Attribute* attr,
  SmiKey& key,
  SmiKey::KeyDataType keyType)
{

  assert(attr->IsDefined());
  switch(keyType)
  {
    case SmiKey::Integer:
      key = SmiKey( ((CcInt*)attr)->GetValue());
      break;
    
    case SmiKey::Longint:
      key = SmiKey( ((LongInt*)attr)->GetValue());
      break;

    case SmiKey::Float:
      key = SmiKey( ((CcReal*)attr)->GetValue());
      break;

    case SmiKey::String: {
      char* strval = (char*)((CcString*)attr)->GetStringval();
      key = SmiKey(strval);
      break;
    }
    case SmiKey::Composite:
      key = SmiKey((IndexableAttribute*)attr);
      break;

    default:
      assert(false /* should not reach this */);
      break;
  }
  assert(key.GetType() == keyType);
}

/*
2.3 Function ~KeyToAttr~

Converts a ~SmiKey~ to a ~Attribute~.

*/
void KeyToAttr(
  Attribute* attr,
  SmiKey& key,
  SmiKey::KeyDataType keyType)
{
  double floatval;
  int32_t intval;
  int64_t longval;
  string strval;

  assert(key.GetType() == keyType);
  switch(keyType)
  {
    case SmiKey::Integer:
      key.GetKey(intval);
      ((CcInt*)attr)->Set(true, (int)intval);
      break;

    case SmiKey::Longint:
      key.GetKey(longval);
      ((LongInt*)attr)->SetValue(longval);
      break;

    case SmiKey::Float:
      key.GetKey(floatval);
      ((CcReal*)attr)->Set(true, floatval);
      break;

    case SmiKey::String:
      key.GetKey(strval);
      ((CcString*)attr)->Set(true, (STRING_T*)strval.c_str());
      break;

    case SmiKey::Composite:
      key.GetKey((IndexableAttribute*)attr);
      break;

    default:
      assert(false /* should not reach this */);
      break;
  }
}

/*
2.4 Function ~ExtractKeyTypeFromTypeInfo~

Extracts the key data type from the type info.

*/
SmiKey::KeyDataType
ExtractKeyTypeFromTypeInfo( ListExpr typeInfo )
{
  int algId = nl->IntValue( nl->First( nl->Third( typeInfo ) ) ),
      typeId = nl->IntValue( nl->Second( nl->Third( typeInfo ) ) );

  string keyTypeString = am->GetTC( algId, typeId )->Name();
  if( (keyTypeString == CcInt::BasicType())
    || (keyTypeString == TupleIdentifier::BasicType()) )
  {
    return SmiKey::Integer;
  } else if(keyTypeString == LongInt::BasicType()){
    return SmiKey::Longint;
  } else if( keyTypeString == CcString::BasicType() )
  {
    return SmiKey::String;
  }
  else if( keyTypeString == CcReal::BasicType() )
  {
    return SmiKey::Float;
  }

  return SmiKey::Composite;
}

/*
2.5 Function ~WriteRecordId~

Writes a ~SmiRecordId~ to a ~SmiRecord~.

*/
bool WriteRecordId(SmiRecord& record, SmiRecordId id)
{
  SmiSize bytesWritten;
  SmiSize idSize = sizeof(SmiRecordId);

  bytesWritten = record.Write(&id, idSize);
  return bytesWritten == idSize;
}

/*
2.6 Function ~ReadRecordId~

Reads a ~SmiRecordId~ from a ~SmiRecord~.

*/
bool ReadRecordId(SmiRecord& record, SmiRecordId& id)
{
  SmiSize bytesRead;
  SmiRecordId ids[2];
  SmiSize idSize = sizeof(SmiRecordId);

  bytesRead = record.Read(ids, 2 * idSize);
  id = ids[0];
  return bytesRead == idSize;
}

/*
2.7 Function ~ReadRecordId~

Reads a ~SmiRecordId~ from a ~PrefetchingIterator~.

*/
bool ReadRecordId(PrefetchingIterator* iter, SmiRecordId& id)
{
  SmiSize bytesRead;
  SmiRecordId ids[2];
  SmiSize idSize = sizeof(SmiRecordId);

  bytesRead = iter->ReadCurrentData(ids, 2 * idSize);
  id = ids[0];
  return bytesRead == idSize;
}

#ifdef BTREE_PREFETCH
typedef PrefetchingIterator BTreeFileIteratorT;
#else
typedef SmiKeyedFileIterator BTreeFileIteratorT;
#endif /* BTREE_PREFETCH */

/*

3 Class ~BTreeIterator~

Used to iterate over all record ids fulfilling a certain condition.

*/
BTreeIterator::BTreeIterator(BTreeFileIteratorT* iter)
  : fileIter(iter)
{
}

BTreeIterator::~BTreeIterator()
{
  delete fileIter;
}

bool BTreeIterator::Next()
{
  bool received;

#ifdef BTREE_PREFETCH
  received = fileIter->Next();
#else
  received = fileIter->Next(smiKey, record);
#endif /* BTREE_PREFETCH */

  if(received)
  {

#ifdef BTREE_PREFETCH
    fileIter->CurrentKey(smiKey);
#endif /* BTREE_PREFETCH */

#ifdef BTREE_PREFETCH
    return ReadRecordId(fileIter, id);
#else
    return ReadRecordId(record, id);
#endif /* BTREE_PREFETCH */
  }
  else
  {
    return false;
  }
}

const SmiKey *BTreeIterator::GetKey() const
{
  return &smiKey;
}

SmiRecordId BTreeIterator::GetId() const
{
  return id;
}

/*

4 Class ~BTree~

The key attribute of a btree can be an ~int~, a ~string~, ~real~, or a
composite one.

*/
BTree::BTree( SmiKey::KeyDataType keyType, bool temporary ):
temporary( temporary ),
file( 0 ),
opened( false )
{
  if( keyType != SmiKey::Unknown )
  {
    file = new SmiBtreeFile( keyType, false, temporary );
    if( file->Create() )
      opened = true;
    else
    {
      delete file; file = 0;
    }
  }
}

BTree::BTree( SmiKey::KeyDataType keyType, SmiRecord& record,
              size_t& offset):
temporary( false ),
file( 0 ),
opened( false )
{
  SmiFileId fileId;
  if( record.Read( &fileId, sizeof(SmiFileId), offset ) != sizeof(SmiFileId) )
    return;

  this->file = new SmiBtreeFile( keyType, false );
  if( file->Open( fileId ) )
  {
    opened = true;
    offset += sizeof(SmiFileId);
  }
  else
  {
    delete file; file = 0;
  }
}

BTree::BTree( SmiKey::KeyDataType keyType, SmiFileId fileId ):
temporary( false ),
file( 0 ),
opened( false )
{
  if( keyType != SmiKey::Unknown )
  {
    file = new SmiBtreeFile( keyType );
    if( file->Open( fileId ) )
      opened = true;
    else
    {
      delete file; file = 0;
    }
  }
}

BTree::~BTree()
{
  if( opened )
  {
    assert( file != 0 );
    if( temporary )
      file->Drop();
    else
      file->Close();
    delete file;
  }
}

bool BTree::Truncate()
{
  if( opened )
    return file->Truncate();
  return true;
}

void BTree::DeleteFile()
{
  if( opened )
  {
    assert( file != 0 );
    file->Close();
    file->Drop();
    delete file; file = 0;
    opened = false;
  }
}

bool BTree::Append( const SmiKey& smiKey, SmiRecordId id )
{
  if( opened )
  {
    assert( file != 0 );
    assert( smiKey.GetType() == GetKeyType() );

    SmiRecord record;
    if( file->InsertRecord( smiKey, record ) )
    {
      if( WriteRecordId( record, id ) )
        return true;
      file->DeleteRecord(smiKey);
    }
  }
  return false;
}

bool BTree::Delete( const SmiKey& smiKey, const SmiRecordId id )
{
  if( opened )
  {
    assert(file != 0);
    return file->DeleteRecord( smiKey, false, id );
  }
  return false;
}

SmiBtreeFile* BTree::GetFile() const
{
  return this->file;
}

SmiFileId BTree::GetFileId() const
{
  return this->file->GetFileId();
}

SmiKey::KeyDataType BTree::GetKeyType() const
{
  return this->file->GetKeyType();
}

BTreeIterator* BTree::ExactMatch( Attribute* key )
{
  if( !opened )
    return 0;

  assert( file != 0 );

  SmiKey smiKey;
  BTreeFileIteratorT* iter;

  AttrToKey( key, smiKey, file->GetKeyType() );

#ifdef BTREE_PREFETCH
  iter = file->SelectRangePrefetched( smiKey, smiKey );
  if( iter == 0 )
    return 0;
#else
  iter = new SmiKeyedFileIterator( true );
  if( !file->SelectRange( smiKey, smiKey, *iter, SmiFile::ReadOnly, true ) )
  {
    delete iter;
    return 0;
  }
#endif /* BTREE_PREFETCH */

  return new BTreeIterator( iter );
}

BTreeIterator* BTree::LeftRange( Attribute* key )
{
  if( !opened )
    return 0;

  assert( file != 0 );
  SmiKey smiKey;
  BTreeFileIteratorT* iter;

  AttrToKey( key, smiKey, file->GetKeyType() );

#ifdef BTREE_PREFETCH
  iter = file->SelectLeftRangePrefetched( smiKey );
  if(iter == 0)
    return 0;
#else
  iter = new SmiKeyedFileIterator( true );
  if( !file->SelectLeftRange( smiKey, *iter, SmiFile::ReadOnly, true ) )
  {
    delete iter;
    return 0;
  }
#endif /* BTREE_PREFETCH */

  return new BTreeIterator( iter );
}

BTreeIterator* BTree::RightRange( Attribute* key )
{
  if( !opened )
    return 0;

  assert( file != 0 );
  SmiKey smiKey;
  BTreeFileIteratorT* iter;

  AttrToKey( key, smiKey, file->GetKeyType() );

#ifdef BTREE_PREFETCH
  iter = file->SelectRightRangePrefetched( smiKey );
  if( iter == 0 )
    return 0;
#else
  iter = new SmiKeyedFileIterator( true );
  if(!file->SelectRightRange( smiKey, *iter, SmiFile::ReadOnly, true ) )
  {
    delete iter;
    return 0;
  }
#endif /* BTREE_PREFETCH */

  return new BTreeIterator( iter );
}

BTreeIterator* BTree::Range( Attribute* left, Attribute* right)
{
  if( !opened )
    return 0;

  assert( file != 0 );
  SmiKey leftSmiKey;
  SmiKey rightSmiKey;
  BTreeFileIteratorT* iter;

  AttrToKey( left, leftSmiKey, file->GetKeyType() );
  AttrToKey( right, rightSmiKey, file->GetKeyType() );

#ifdef BTREE_PREFETCH
  iter = file->SelectRangePrefetched( leftSmiKey, rightSmiKey );
  if( iter == 0 )
    return 0;
#else
  iter = new SmiKeyedFileIterator( true );
  if( !file->SelectRange( leftSmiKey, rightSmiKey,
                          *iter, SmiFile::ReadOnly, true))
  {
    delete iter;
    return 0;
  }
#endif /* BTREE_PREFETCH */

  return new BTreeIterator( iter );
}

BTreeIterator* BTree::SelectAll()
{
  BTreeFileIteratorT* iter;

#ifdef BTREE_PREFETCH
  iter = file->SelectAllPrefetched();
  if(iter == 0)
    return 0;
#else
  iter = new SmiKeyedFileIterator( true );
  if( !file->SelectAll( *iter, SmiFile::ReadOnly, true ) )
  {
    delete iter;
    return 0;
  }
#endif /* BTREE_PREFETCH */

  return new BTreeIterator( iter );
}


bool BTree::getFileStats( SmiStatResultType &result )
{
  if ( (file == 0) || !opened ){
    return false;
  }
  result = file->GetFileStatistics(SMI_STATS_EAGER);
  std::stringstream fileid;
  fileid << file->GetFileId();
  result.push_back(pair<string,string>("FilePurpose",
            "SecondaryBtreeIndexFile"));
  result.push_back(pair<string,string>("FileId",fileid.str()));
  return true;
}

/*

5 Type constructor ~btree~

5.1 Function ~OutBTree~

A btree does not make any sense as an independent value since
the record ids stored in it become obsolete as soon as
the underlying relation is deleted. Therefore this function
outputs an empty list.

*/
ListExpr OutBTree(ListExpr typeInfo, Word  value)
{
  return nl->TheEmptyList();
}

ListExpr SaveToListBTree(ListExpr typeInfo, Word  value)
{
  BTree *btree = (BTree*)value.addr;

  return nl->IntAtom( btree->GetFileId() );
}

/*

5.2 Function ~CreateBTree~

*/
Word CreateBTree(const ListExpr typeInfo)
{
  return SetWord( new BTree( ExtractKeyTypeFromTypeInfo( typeInfo ) ) );
}

/*

5.3 Function ~InBTree~

Reading a btree from a list does not make sense because a btree
is not an independent value. Therefore calling this function leads
to program abort.

*/
Word InBTree(ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct)
{
  correct = false;
  return SetWord(Address(0));
}

Word RestoreFromListBTree( ListExpr typeInfo, ListExpr value,
                           int errorPos, ListExpr& errorInfo, bool& correct)
{
  SmiKey::KeyDataType keyType = ExtractKeyTypeFromTypeInfo( typeInfo );
  SmiFileId fileId = nl->IntValue(value);

  BTree *btree = new BTree( keyType, fileId );
  if( btree->IsOpened() )
    return SetWord( btree );
  else
  {
    delete btree;
    return SetWord( Address( 0 ) );
  }
}

/*

5.4 Function ~CloseBTree~

*/
void CloseBTree(const ListExpr typeInfo, Word& w)
{
  BTree* btree = (BTree*)w.addr;
  delete btree;
}
/*

5.5 Function ~CloneBTree~

*/
Word CloneBTree(const ListExpr typeInfo, const Word& w)
{
  BTree *btree = (BTree *)w.addr,
        *clone = new BTree( btree->GetKeyType() );

  if( !clone->IsOpened() )
    return SetWord( Address(0) );

  BTreeIterator *iter = btree->SelectAll();
  while( iter->Next())
  {
    if( !clone->Append( *iter->GetKey(), iter->GetId() ) )
      return SetWord( Address( 0 ) );
  }
  delete iter;

  return SetWord( clone );
}

/*

5.6 Function ~DeleteBTree~

*/
void DeleteBTree(const ListExpr typeInfo, Word& w)
{
  BTree* btree = (BTree*)w.addr;
  btree->DeleteFile();
  delete btree;
}

/*

5.7 ~Check~-function of type constructor ~btree~

*/
bool CheckBTree(ListExpr type, ListExpr& errorInfo)
{
  if((!nl->IsAtom(type))
    && (nl->ListLength(type) == 3)
    && nl->Equal(nl->First(type), nl->SymbolAtom(BTree::BasicType())))
  {
    return
      am->CheckKind(Kind::TUPLE(), nl->Second(type), errorInfo)
      && am->CheckKind(Kind::DATA(), nl->Third(type), errorInfo);
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("BTREE"), type));
    return false;
  }
  return true;
}

void* CastBTree(void* addr)
{
  return ( 0 );
}

int SizeOfBTree()
{
  return 0;
}

/*

5.8 ~OpenFunction~ of type constructor ~btree~

*/
bool
OpenBTree( SmiRecord& valueRecord,
           size_t& offset,
           const ListExpr typeInfo,
           Word& value )
{
  value = SetWord( BTree::Open( valueRecord, offset, typeInfo ) );
  bool rc  = value.addr != 0;
  return rc;
}

BTree *BTree::Open( SmiRecord& valueRecord, size_t& offset,
                    const ListExpr typeInfo )
{
  return new BTree( ExtractKeyTypeFromTypeInfo( typeInfo ), valueRecord,
                    offset );
}

/*

5.9 ~SaveFunction~ of type constructor ~btree~

*/
bool
SaveBTree( SmiRecord& valueRecord,
           size_t& offset,
           const ListExpr typeInfo,
           Word& value )
{
  BTree *btree = (BTree*)value.addr;
  return btree->Save( valueRecord, offset, typeInfo );
}

bool BTree::Save(SmiRecord& record, size_t& offset, const ListExpr typeInfo)
{
  if( !opened )
    return false;

  assert( file != 0 );
  const size_t n = sizeof(SmiFileId);
  SmiFileId fileId = file->GetFileId();
  if( record.Write( &fileId, n, offset) != n )
    return false;

  offset += n;
  return true;
}

/*

5.11 Type Constructor object for type constructor ~btree~

*/
TypeConstructor
cppbtree( BTree::BasicType(),         BTreeProp,
          OutBTree,        InBTree,
          SaveToListBTree, RestoreFromListBTree,
          CreateBTree,     DeleteBTree,
          OpenBTree,       SaveBTree,
          CloseBTree,      CloneBTree,
          CastBTree,       SizeOfBTree,
          CheckBTree );

/*

6 Operators of the btree algebra

6.1 Operator ~createbtree~

6.1.1 Type Mapping of operator ~createbtree~

*/
ListExpr CreateBTreeTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=2){
    return listutils::typeError("two arguments expected");
  }
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  if(!listutils::isRelDescription(first) &&
     !listutils::isTupleStream(first)){
     return listutils::typeError("first arg is not a rel or a tuple stream");
  }

  if(nl->AtomType(second)!=SymbolType){
    return listutils::typeError("second argument is not a valid attr name");
  }

  string name = nl->SymbolValue(second);

  int attrIndex;
  ListExpr attrType;
  attrIndex = listutils::findAttribute(nl->Second(nl->Second(first)),
                                       name,attrType);
  if(attrIndex==0){
    return listutils::typeError("attr name " + name + "not found in attr list");
  }

  if(!listutils::isBDBIndexableType(attrType)){
    return listutils::typeError("selected attribute not an indexable key");
  }

  ListExpr tupleDescription = nl->Second(first);


  if( listutils::isSymbol(nl->First(first),Relation::BasicType()) ) {
    return nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
                              nl->OneElemList( nl->IntAtom(attrIndex)),
                              nl->ThreeElemList(
                              nl->SymbolAtom(BTree::BasicType()),
                              tupleDescription,
                              attrType));
  } else { // nl->IsEqual(nl->First(first), Symbol::STREAM())
    // Find the attribute with type tid
    string name;

    int tidIndex =listutils::findType(nl->Second(tupleDescription),
                                  nl->SymbolAtom(TupleIdentifier::BasicType()),
                                    name);
    if(tidIndex ==0){
     return listutils::typeError("attr list does not contain a tid attribute");
    }
    string name2;
    if(listutils::findType(nl->Second(tupleDescription),
                           nl->SymbolAtom(TupleIdentifier::BasicType()),
                           name2,
                           tidIndex+1)>0){
      return listutils::typeError("multiple tid attributes found");
    }
    set<string> a;
    a.insert(name);
    ListExpr newAttrList;
    ListExpr lastAttrList;
    if(listutils::removeAttributes(nl->Second(tupleDescription),
                                   a, newAttrList, lastAttrList)!=1){
     return listutils::typeError("problem in removing attribute " + name);
    }

    if(nl->IsEmpty(newAttrList)){
      return listutils::typeError("the resulting attr list would be empty");
    }

    ListExpr res =  nl->ThreeElemList(
        nl->SymbolAtom(Symbol::APPEND()),
        nl->TwoElemList(
          nl->IntAtom(attrIndex),
          nl->IntAtom(tidIndex)),
        nl->ThreeElemList(
          nl->SymbolAtom(BTree::BasicType()),
          nl->TwoElemList(
            nl->SymbolAtom(Tuple::BasicType()),
            newAttrList),
          attrType));
    cout << "out = " << nl->ToString(res) << endl;
    return res;
  }
}

/*
6.1.2 Selection function of operator ~createbtree~

*/
int CreateBTreeSelect( ListExpr args )
{
  if( nl->IsEqual(nl->First(nl->First(args)), Relation::BasicType()) )
    return 0;
  if( nl->IsEqual(nl->First(nl->First(args)), Symbol::STREAM()) )
    return 1;
  return -1;
}

/*
6.1.3 Value mapping function of operator ~createbtree~

*/
int
CreateBTreeValueMapping_Rel(Word* args, Word& result, int message,
                            Word& local, Supplier s)
{
  result = qp->ResultStorage(s);

  Relation* relation = (Relation*)args[0].addr;
  BTree* btree = (BTree*)result.addr;
  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

  assert(btree != 0);
  assert(relation != 0);

  if( !btree->IsOpened() )
    return CANCEL;
  btree->Truncate();

  GenericRelationIterator *iter = relation->MakeScan();
  Tuple* tuple;
  while( (tuple = iter->GetNextTuple()) != 0 )
  {
    SmiKey key;

    if( (Attribute *)tuple->GetAttribute(attrIndex)->IsDefined() )
    {
      AttrToKey( (Attribute *)tuple->GetAttribute(attrIndex),
                 key, btree->GetKeyType() );
      btree->Append( key, iter->GetTupleId() );
    }
    tuple->DeleteIfAllowed();
  }
  delete iter;

  return 0;
}

int
CreateBTreeValueMapping_Stream(Word* args, Word& result, int message,
                               Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BTree* btree = (BTree*)result.addr;
  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1,
      tidIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;
  Word wTuple;

  assert(btree != 0);
  if( !btree->IsOpened() )
    return CANCEL;
  btree->Truncate();

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);
  while (qp->Received(args[0].addr))
  {
    Tuple* tuple = (Tuple*)wTuple.addr;
    SmiKey key;
    if( (Attribute *)tuple->GetAttribute(attrIndex)->IsDefined() &&
        (Attribute *)tuple->GetAttribute(tidIndex)->IsDefined() )
    {
      AttrToKey( (Attribute *)tuple->GetAttribute(attrIndex),
                 key, btree->GetKeyType() );
      btree->Append( key,
                 ((TupleIdentifier *)tuple->GetAttribute(tidIndex))->GetTid());
    }
    tuple->DeleteIfAllowed();

    qp->Request(args[0].addr, wTuple);
  }
  qp->Close(args[0].addr);

  return 0;
}

/*

6.1.4 Specification of operator ~createbtree~

*/
const string CreateBTreeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>(((rel (tuple (x1 t1)...(xn tn)))) xi)"
    " -> (btree (tuple ((x1 t1)...(xn tn))) ti)\n"
    "((stream (tuple (x1 t1)...(xn tn) (id tid))) xi)"
    " -> (btree (tuple ((x1 t1)...(xn tn))) ti)</text--->"
    "<text>_ createbtree [ _ ]</text--->"
    "<text>Creates a btree. The key type ti must "
    "be either string or int or real or to implement the "
    "kind INDEXABLE. The operator only accepts input tuple types "
    "containing 0 (for a relation) or 1 (for a stream) "
    "attribute of type tid. The naming of this attribute is of "
    "no concern.</text--->"
    "<text>let mybtree = ten createbtree [nr];\n"
    "let mybtree = ten feed extend[id: tupleid(.)] "
    "sortby[no asc] createbtree[no]</text--->"
    ") )";

/*

6.1.5 Definition of operator ~createbtree~

*/
ValueMapping createbtreemap[] = { CreateBTreeValueMapping_Rel,
                                  CreateBTreeValueMapping_Stream };

Operator createbtree (
          "createbtree",             // name
          CreateBTreeSpec,           // specification
          2,                         // number of overloaded functions
          createbtreemap,            // value mapping
          CreateBTreeSelect,         // trivial selection function
          CreateBTreeTypeMap         // type mapping
);

/*

6.2 Operators ~range~, ~leftrange~, ~rightrange~ and ~exactmatch~

*/

const int LEFTRANGE = 0;
const int RIGHTRANGE = 1;
const int RANGE = 2;
const int EXACTMATCH = 3;
const int KEYRANGE = 4;

int nKeyArguments(int opid)
{
  assert(opid >= LEFTRANGE && opid <= KEYRANGE);
  return opid == RANGE ? 2 : 1;
}

/*
6.2.1 Type mapping function of operators ~range~, ~leftrange~,
~rightrange~ and ~exactmatch~

*/
template<int operatorId>
ListExpr IndexQueryTypeMap(ListExpr args)
{
  int nKeys = nKeyArguments(operatorId);

  int expLen = nKeys +2;
  if(nl->ListLength(args)!=expLen){
    return listutils::typeError("wrong number of arguments");
  }
  /* Split argument in three parts */
  ListExpr btreeDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr keyDescription = nl->Third(args);
  ListExpr secondKeyDescription = nl->TheEmptyList();
  if(nKeys == 2)
  {
    secondKeyDescription = nl->Fourth(args);
  }

  /* find out type of key */
  if(!listutils::isSymbol(keyDescription)){
    return listutils::typeError("invalid key");
  }
  /* find out type of second key (if any) */
  if(nKeys == 2) {
    if(!nl->Equal(keyDescription, secondKeyDescription)){
       return listutils::typeError("different key types");
    }
  }

  /* handle btree part of argument */
  if(!listutils::isBTreeDescription(btreeDescription)){
    return listutils::typeError("not a btree");
  }

  ListExpr btreeKeyType = nl->Third(btreeDescription);

  /* check that the type of given key equals the btree key type */
  if(!nl->Equal(keyDescription, btreeKeyType)){
    return listutils::typeError("key and btree key are different");
  }

  if(!listutils::isRelDescription(relDescription)){
    return listutils::typeError("not a relation");
  }

  /* check that btree and rel have the same associated tuple type */
  ListExpr trel = nl->Second(relDescription);
  ListExpr rtree = nl->Second(btreeDescription);
  if(!nl->Equal(trel, rtree)){
    return listutils::typeError("different types for relation and btree");
  }

  if (operatorId == KEYRANGE) {
    NList a1(NList("Less"), NList(CcReal::BasicType()));
    NList a2(NList("Equal"), NList(CcReal::BasicType()));
    NList a3(NList("Greater"), NList(CcReal::BasicType()));
    NList a4(NList("NumOfKeys"), NList(CcInt::BasicType()));

    NList result(NList(Symbol::STREAM()), NList(NList(Tuple::BasicType()),
                                                NList(a1,a2,a3,a4)));

    return result.listExpr();
  }

  ListExpr resultType =
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      trel);

  return resultType;
}



/*

6.2.2 Value mapping function of operators ~range~, ~leftrange~,
~rightrange~ and ~exactmatch~

*/

#ifndef USE_PROGRESS

// standard version


struct IndexQueryLocalInfo
{
  Relation* relation;
  BTreeIterator* iter;
};


template<int operatorId>
int
IndexQuery(Word* args, Word& result, int message, Word& local, Supplier s)
{
  BTree* btree;
  Attribute* key;
  Attribute* secondKey;
  Tuple* tuple;
  SmiRecordId id;
  IndexQueryLocalInfo* localInfo;

  switch (message)
  {
    case OPEN :
      localInfo = new IndexQueryLocalInfo;
      btree = (BTree*)args[0].addr;
      localInfo->relation = (Relation*)args[1].addr;
      key = (Attribute*)args[2].addr;
      if(operatorId == RANGE)
      {
        secondKey = (Attribute*)args[3].addr;
        assert(secondKey != 0);
      }

      assert(btree != 0);
      assert(localInfo->relation != 0);
      assert(key != 0);

      switch(operatorId)
      {
        case EXACTMATCH:
          localInfo->iter = btree->ExactMatch(key);
          break;
        case RANGE:
          localInfo->iter = btree->Range(key, secondKey);
          break;
        case LEFTRANGE:
          localInfo->iter = btree->LeftRange(key);
          break;
        case RIGHTRANGE:
          localInfo->iter = btree->RightRange(key);
          break;
        default:
          assert(false);
      }

      if(localInfo->iter == 0)
      {
        delete localInfo;
        return -1;
      }

      local = SetWord(localInfo);
      return 0;

    case REQUEST :
      localInfo = (IndexQueryLocalInfo*)local.addr;
      assert(localInfo != 0);

      if(localInfo->iter->Next())
      {
        id = localInfo->iter->GetId();
        tuple = localInfo->relation->GetTuple( id );
        if(tuple == 0)
        {
          cerr << "Could not find tuple for the given tuple id. "
               << "Maybe the given btree and the given relation "
               << "do not match." << endl;
          assert(false);
        }

        result = SetWord(tuple);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }

    case CLOSE :
      if(local.addr)
      {
        localInfo = (IndexQueryLocalInfo*)local.addr;
        delete localInfo->iter;
        delete localInfo;
        local = SetWord(Address(0));
      }
      return 0;
  }
  return 0;
}



# else

// progress version



class IndexQueryLocalInfo: public ProgressLocalInfo
{

public:

  Relation* relation;
  BTreeIterator* iter;
  bool first;
  int completeCalls;
  int completeReturned;
  double less, equal, range, equal2, greater;
};



template<int operatorId>
int
IndexQuery(Word* args, Word& result, int message, Word& local, Supplier s)
{
  BTree* btree;
  Attribute* key;
  Attribute* secondKey;
  Tuple* tuple;
  SmiRecordId id;
  IndexQueryLocalInfo* ili;
  SmiKeyRange factors, secondFactors;
  SmiKey sk;

  ili = (IndexQueryLocalInfo*)local.addr;

  switch (message)
  {
    case OPEN :

      //local info kept over many calls of OPEN!
      //useful for loopjoin

      if ( !ili ) //first time
      {
        ili = new IndexQueryLocalInfo;
        ili->completeCalls = 0;
        ili->completeReturned = 0;

        ili->sizesInitialized = false;
        ili->sizesChanged = false;

        ili->relation = (Relation*)args[1].addr;
        local = SetWord(ili);
      }

      btree = (BTree*)args[0].addr;
      key = (Attribute*)args[2].addr;
      if(operatorId == RANGE)
      {
        secondKey = (Attribute*)args[3].addr;
        assert(secondKey != 0);
      }

      assert(btree != 0);
      assert(ili->relation != 0);
      assert(key != 0);

      //create iterator
      switch(operatorId)
      {
        case EXACTMATCH: {
          ili->iter = btree->ExactMatch(key);
          break;
			 }
        case RANGE:
          ili->iter = btree->Range(key, secondKey);
          break;
        case LEFTRANGE:
          ili->iter = btree->LeftRange(key);
          break;
        case RIGHTRANGE:
          ili->iter = btree->RightRange(key);
          break;
        default:
          assert(false);
      }

      if(ili->iter == 0)
      {
        delete ili;
        return -1;
      }

      //determine selectivities for progress estimation,
      //but only for single or first call (not many times in loopjoin)

      if ( ili->completeCalls == 0 )
      {

        AttrToKey( key, sk, btree->GetFile()->GetKeyType() );
        btree->GetFile()->KeyRange(sk, factors);
        if(operatorId == RANGE)
        {
          AttrToKey( secondKey, sk, btree->GetFile()->GetKeyType() );
          btree->GetFile()->KeyRange(sk, secondFactors);
        }
        ili->less = factors.less;
        ili->equal = factors.equal;
        ili->greater = factors.greater;
        if(operatorId == RANGE)
        {
          ili->range = secondFactors.less - (factors.less + factors.equal);
          ili->equal2 = secondFactors.equal;
          ili->greater = secondFactors.greater;
        }
      }
      return 0;

    case REQUEST :

      assert(ili != 0);

      if(ili->iter->Next())
      {
        id = ili->iter->GetId();
        tuple = ili->relation->GetTuple( id , false);
        if(tuple == 0)
        {
          cerr << "Could not find tuple for the given tuple id. "
               << "Maybe the given btree and the given relation "
               << "do not match." << endl;
          assert(false);
        }

        result = SetWord(tuple);

        ili->returned++;

        return YIELD;
      }
      else
      {
        return CANCEL;
      }

    case CLOSE :
      if( ili )
      {
        delete ili->iter;
        ili->completeCalls++;
        ili->completeReturned += ili->returned;
        ili->returned = 0;
      }

      //Do not delete ili data structure in CLOSE! To be kept over several
      //calls for correct progress estimation when embedded in a loopjoin.
      //Is deleted at the end of the query in CLOSEPROGRESS.

      return 0;


    case CLOSEPROGRESS:
      if ( ili )
      {
        delete ili;
        local = SetWord(Address(0));
      }
      return 0;


    case REQUESTPROGRESS :
      ProgressInfo *pRes;
      pRes = (ProgressInfo*) result.addr;

      const double uIndexQuery = 0.15;  //ms per search
      const double vIndexQuery = 0.018; //ms per result tuple

      if ( !ili ) return CANCEL;
      else
      {
        ili->sizesChanged = false;

        if (!ili->sizesInitialized )
        {
          ili->total = ili->relation->GetNoTuples();
          ili->defaultValue = 50;
          ili->Size = 0;
          ili->SizeExt = 0;
          ili->noAttrs =
            nl->ListLength(nl->Second(nl->Second(qp->GetType(s))));
          ili->attrSize = new double[ili->noAttrs];
          ili->attrSizeExt = new double[ili->noAttrs];
          for ( int i = 0;  i < ili->noAttrs; i++)
          {
            ili->attrSize[i] = ili->relation->GetTotalSize(i)
                                  / (ili->total + 0.001);
            ili->attrSizeExt[i] = ili->relation->GetTotalExtSize(i)
                                  / (ili->total + 0.001);

            ili->Size += ili->attrSize[i];
            ili->SizeExt += ili->attrSizeExt[i];
          }
          ili->sizesInitialized = true;
          ili->sizesChanged = true;
        }

        pRes->CopySizes(ili);


        if ( ili->completeCalls > 0 )     //called in a loopjoin
        {
          pRes->Card =
            (double) ili->completeReturned / (double) ili->completeCalls;
        }
        else  //single or first call
        {
          switch(operatorId)
          {
            case EXACTMATCH:
              pRes->Card = ili->total * ili->equal;
              break;
            case RANGE:
              pRes->Card = ili->total * (ili->equal + ili->range + ili->equal2);
              break;
            case LEFTRANGE:
               pRes->Card = ili->total * (ili->less + ili->equal);
              break;
            case RIGHTRANGE:
                pRes->Card = ili->total * (ili->equal + ili->greater);
              break;
            default:
              assert(false);
          }

          if ( (double) ili->returned > pRes->Card )   // there are more tuples
            pRes->Card = (double) ili->returned * 1.1;   // than expected

          if ( !ili->iter )  // btree has been finished
            pRes->Card = (double) ili->returned;

          if ( pRes->Card > (double) ili->total ) // more than all cannot be
            pRes->Card = (double) ili->total;

        }

        pRes->Time = uIndexQuery + pRes->Card * vIndexQuery;

        pRes->Progress =
          (uIndexQuery + (double) ili->returned * vIndexQuery)
          / pRes->Time;

        return YIELD;
      }

  }
  return 0;
}

#endif



/*

6.2.3 Specification of operator ~exactmatch~

*/
const string ExactMatchSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>((btree (tuple ((x1 t1)...(xn tn)))"
  " ti)(rel (tuple ((x1 t1)...(xn tn)))) ti) ->"
  " (stream (tuple ((x1 t1)...(xn tn))))"
  "</text--->"
  "<text>_ _ exactmatch [ _ ]</text--->"
  "<text>Uses the given btree to find all tuples"
  " in the given relation with .xi = argument "
  "value.</text--->"
  "<text>query citiesNameInd cities exactmatch"
  " [\"Berlin\"] consume; where citiesNameInd "
  "is e.g. created with 'let citiesNameInd = "
  "cities createbtree [name]'</text--->"
  ") )";
/*

6.2.4 Definition of operator ~exactmatch~

*/
Operator exactmatch (
         "exactmatch",                  // name
   ExactMatchSpec,                // specification
   IndexQuery<EXACTMATCH>,        // value mapping
   Operator::SimpleSelect,        // trivial selection function
   IndexQueryTypeMap<EXACTMATCH>  // type mapping
);

/*

6.2.5 Specification of operator ~range~

*/
const string RangeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>((btree (tuple ((x1 t1)...(xn tn)))"
  " ti)(rel (tuple ((x1 t1)...(xn tn)))) ti ti)"
  " -> (stream (tuple ((x1 t1)...(xn tn))))"
  "</text--->"
  "<text>_ _ range [ _ , _ ]</text--->"
  "<text>Uses the given btree to find all tuples "
  "in the given relation with .xi >= argument value 1"
  " and .xi <= argument value 2.</text--->"
  "<text>query tenNoInd ten range[5, 7] consume"
  "</text--->"
  "   ) )";

/*

6.2.6 Definition of operator ~range~

*/
Operator cpprange (
         "range",                        // name
   RangeSpec,                      // specification
   IndexQuery<RANGE>,              // value mapping
   Operator::SimpleSelect,         // trivial selection function
   IndexQueryTypeMap<RANGE>        // type mapping
);

/*

6.2.7 Specification of operator ~leftrange~

*/
const string LeftRangeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>((btree (tuple ((x1 t1)...(xn tn))) ti)"
  "(rel (tuple ((x1 t1)...(xn tn)))) ti) -> "
  "(stream"
  " (tuple ((x1 t1)...(xn tn))))</text--->"
  "<text>_ _ leftrange [ _ ]</text--->"
  "<text>Uses the given btree to find all tuples"
  " in the given relation with .xi <= argument "
  "value</text--->"
  "<text>query citiesNameInd cities leftrange"
  "[\"Hagen\"]"
  " consume; where citiesNameInd is e.g. created "
  "with "
  "'let citiesNameInd = cities createbtree [name]'"
  "</text--->"
  ") )";
/*

6.2.8 Definition of operator ~leftrange~

*/
Operator leftrange (
         "leftrange",                    // name
   LeftRangeSpec,                  // specification
   IndexQuery<LEFTRANGE>,          // value mapping
   Operator::SimpleSelect,         // trivial selection function
   IndexQueryTypeMap<LEFTRANGE>    // type mapping
);

/*

6.2.9 Specification of operator ~rightrange~

*/
const string RightRangeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>((btree (tuple ((x1 t1)...(xn tn)))"
  " ti)"
  "(rel (tuple ((x1 t1)...(xn tn)))) ti) -> "
  "(stream"
  " (tuple ((x1 t1)...(xn tn))))</text--->"
  "<text>_ _ rightrange [ _ ]</text--->"
  "<text>Uses the given btree to find all tuples"
  " in the"
  " given relation with .xi >= argument value."
  "</text--->"
  "<text>query citiesNameInd cities rightrange"
  "[ 6 ] "
  "consume; where citiesNameInd is e.g. created"
  " with "
  "'let citiesNameInd = cities createbtree "
  "[name]'"
  "</text--->"
  ") )";
/*

6.2.10 Definition of operator ~rightrange~

*/
Operator rightrange (
         "rightrange",            // name
         RightRangeSpec,          // specification
         IndexQuery<RIGHTRANGE>,              // value mapping
         Operator::SimpleSelect,         // trivial selection function
         IndexQueryTypeMap<RIGHTRANGE>        // type mapping
);

/*

6.3 Operators ~rangeS~, ~leftrangeS~, ~rightrangeS~ and ~exactmatchS~

These operators are variants on the normal btree query operators, but
return a stream(tuple((id tid))) rather than the Indexed tuple.
Therefore, they do not need a relation argument.

*/


/*
6.3.1 Type mapping function of operators ~rangeS~, ~leftrangeS~,
~rightrangeS~ and ~exactmatchS~

*/
template<int operatorId>
ListExpr IndexQuerySTypeMap(ListExpr args)
{
  int nKeys = nKeyArguments(operatorId);
  int expLength = nKeys + 1;
  if(nl->ListLength(args)!=expLength){
    return listutils::typeError("wrong number of arguments");
  }

  /* Split argument in two/three parts */
  ListExpr btreeDescription = nl->First(args);
  ListExpr keyDescription = nl->Second(args);
  ListExpr secondKeyDescription = nl->TheEmptyList();
  if(nKeys == 2)
  {
    secondKeyDescription = nl->Third(args);
  }

  /* find out type of key */
  if(!listutils::isSymbol(keyDescription)){
    return listutils::typeError("invalid key");
  }

  /* find out type of second key (if any) */
  if(nKeys == 2)
  {
    if(!listutils::isSymbol(secondKeyDescription)){
      return listutils::typeError("second key is invalid");
    }
    if(!nl->Equal(keyDescription, secondKeyDescription)){
      return listutils::typeError("different keys found");
    }
  }

  /* handle btree part of argument */
  if(!listutils::isBTreeDescription(btreeDescription)){
    return listutils::typeError("first argument is not a btree");
  }
  ListExpr btreeKeyType = nl->Third(btreeDescription);

  if(!nl->Equal(btreeKeyType, keyDescription)){
    return listutils::typeError("btree has another keytype than given");
  }

  /* return result type */

  return nl->TwoElemList(
          nl->SymbolAtom(Symbol::STREAM()),
          nl->TwoElemList(
            nl->SymbolAtom(Tuple::BasicType()),
            nl->OneElemList(
              nl->TwoElemList(
                nl->SymbolAtom("Id"),
                nl->SymbolAtom(TupleIdentifier::BasicType())))));

}

/*

6.3.2 Value mapping function of operators ~rangeS~, ~leftrangeS~,
~rightrangeS~ and ~exactmatchS~

*/

struct IndexQuerySLocalInfo
{
  BTreeIterator* iter;
  TupleType *resultTupleType;
};

template<int operatorId>
int
IndexQueryS(Word* args, Word& result, int message, Word& local, Supplier s)
{
  BTree* btree;
  Attribute* key;
  Attribute* secondKey;
  SmiRecordId id;
  IndexQuerySLocalInfo* localInfo;

  switch (message)
  {
    case OPEN :
      localInfo = new IndexQuerySLocalInfo;
      btree = (BTree*)args[0].addr;

      key = (Attribute*)args[1].addr;
      if(operatorId == RANGE)
      {
        secondKey = (Attribute*)args[2].addr;
        assert(secondKey != 0);
      }

      assert(btree != 0);
      assert(key != 0);

      switch(operatorId)
      {
        case EXACTMATCH:
          localInfo->iter = btree->ExactMatch(key);
          break;
        case RANGE:
          localInfo->iter = btree->Range(key, secondKey);
          break;
        case LEFTRANGE:
          localInfo->iter = btree->LeftRange(key);
          break;
        case RIGHTRANGE:
          localInfo->iter = btree->RightRange(key);
          break;
        default:
          assert(false);
      }

      if(localInfo->iter == 0)
      {
        delete localInfo;
        return -1;
      }

      localInfo->resultTupleType =
             new TupleType(nl->Second(GetTupleResultType(s)));

      local = SetWord(localInfo);
      return 0;

    case REQUEST :
      localInfo = (IndexQuerySLocalInfo*)local.addr;
      assert(localInfo != 0);

      if(localInfo->iter->Next())
      {
        id = localInfo->iter->GetId();
        Tuple *tuple = new Tuple( localInfo->resultTupleType );
        tuple->PutAttribute(0, new TupleIdentifier(true, id));
        result = SetWord(tuple);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }

    case CLOSE :
      if(local.addr)
      {
        localInfo = (IndexQuerySLocalInfo*)local.addr;
        delete localInfo->iter;
        localInfo->resultTupleType->DeleteIfAllowed();
        delete localInfo;
        local = SetWord(Address(0));
      }
      return 0;
  }
  return 0;
}

/*

6.3.3 Specification of operator ~exactmatchS~

*/
const string ExactMatchSSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(btree( (tuple(X))) ti) ti->)"
  "(stream((id tid)))</text--->"
  "<text> _ exactmatchS [ _ ]</text--->"
  "<text>Uses the given btree to find all tuple "
  "identifiers where the key matches argument "
  "value ti.</text--->"
  "<text>query citiesNameInd exactmatchS"
  " [\"Berlin\"] cities gettuples consume; "
  "where citiesNameInd is e.g. created with "
  "'let citiesNameInd = cities createbtree "
  "[name]'</text--->"
  ") )";
/*

6.3.4 Definition of operator ~exactmatchS~

*/
Operator exactmatchs (
         "exactmatchS",                   // name
         ExactMatchSSpec,                 // specification
         IndexQueryS<EXACTMATCH>,         // value mapping
         Operator::SimpleSelect,          // trivial selection function
         IndexQuerySTypeMap<EXACTMATCH>   // type mapping
);

/*

6.3.5 Specification of operator ~rangeS~

*/
const string RangeSSpec  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text>((btree (tuple (X)) ti) ti ti)"
        " -> (stream (tuple ((ti tid))))"
        "</text--->"
        "<text> _ rangeS [ _ , _ ]</text--->"
        "<text>Uses the given btree to find all stored "
        "tuple identifiers referencing tuples "
        "with key >= argument value 1"
        " and key <= argument value 2.</text--->"
        "<text>query tenNoInd rangeS[5, 7] ten gettuples "
        "consume </text--->"
        "   ) )";

/*

6.3.6 Definition of operator ~rangeS~

*/
Operator cppranges (
         "rangeS",                       // name
         RangeSSpec,                     // specification
         IndexQueryS<RANGE>,             // value mapping
         Operator::SimpleSelect,         // trivial selection function
         IndexQuerySTypeMap<RANGE>       // type mapping
);

/*

6.3.7 Specification of operator ~leftrangeS~

*/
const string LeftRangeSSpec  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
        "( <text>((btree (tuple (X)) ti) ti)"
        " -> (stream (tuple ((ti tid))))</text--->"
        "<text>_ leftrangeS [ _ ]</text--->"
        "<text>Uses the given btree to find all tuple "
        "identifiers referencing tuples with "
        ".xi <= argument value</text--->"
        "<text>query citiesNameInd leftrangeS"
        "[\"Hagen\"] cities gettuples "
        "consume; where citiesNameInd is e.g. created "
        "with "
        "'let citiesNameInd = cities createbtree [name]'"
        "</text--->"
        ") )";
/*

6.3.8 Definition of operator ~leftrangeS~

*/
Operator leftranges (
         "leftrangeS",                   // name
         LeftRangeSSpec,                 // specification
         IndexQueryS<LEFTRANGE>,         // value mapping
         Operator::SimpleSelect,         // trivial selection function
         IndexQuerySTypeMap<LEFTRANGE>   // type mapping
);

/*

6.3.9 Specification of operator ~rightrangeS~

*/
const string RightRangeSSpec  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
        "( <text>((btree (tuple (X))"
        " ti) ti) -> (stream"
        " (tuple ((id tid))))</text--->"
        "<text>_ rightrangeS [ _ ]</text--->"
        "<text>Uses the given btree to find all tuple "
        "identifiers associated with keys "
        ".xi >= argument value."
        "</text--->"
        "<text>query citiesNameInd rightrangeS"
        "[ 6 ] cities gettuples"
        "consume; where citiesNameInd is e.g. created"
        " with "
        "'let citiesNameInd = cities createbtree "
        "[name]'"
        "</text--->"
        ") )";
/*

6.3.10 Definition of operator ~rightrangeS~

*/
Operator rightranges (
         "rightrangeS",                  // name
         RightRangeSSpec,                // specification
         IndexQueryS<RIGHTRANGE>,        // value mapping
         Operator::SimpleSelect,         // trivial selection function
         IndexQuerySTypeMap<RIGHTRANGE>  // type mapping
);




/*
6.4 Operator ~insertbtree~

For each tuple of the inputstream inserts an entry into the btree. The entry
is built from the attribute
of the tuple over which the tree is built and the tuple-identifier of the
inserted tuple which is extracted
as the last attribute of the tuple of the inputstream.


6.4.0 General Type mapping function of operators ~insertbtree~, ~deletebtree~
and ~updatebtree~


Type mapping ~insertbtree~ and ~deletebtree~

----     (stream (tuple ((a1 x1) ... (an xn) (TID tid)))) (btree X ti)) ai

        -> (stream (tuple ((a1 x1) ... (an xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----

Type mapping ~updatebtree~

----
     (stream (tuple ((a1 x1) ... (an xn)
                     (a1_old x1) ... (an_old xn) (TID tid)))) (btree X ti)) ai

  -> (stream (tuple ((a1 x1) ... (an xn)
                     (a1_old x1) ... (an_old xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----


*/

ListExpr allUpdatesBTreeTypeMap( const ListExpr& args, string opName )
{

  if(!nl->HasLength(args,3)){
     ErrorReporter::ReportError("3 arguments expected");
     return nl->TypeError();
  }
  /* Split argument in three parts */
  ListExpr streamDescription = nl->First(args);
  ListExpr btreeDescription = nl->Second(args);
  ListExpr nameOfKeyAttribute = nl->Third(args);


  if(!listutils::isTupleStream(streamDescription)){
   return listutils::typeError("first arguments must be a tuple stream");
  }

  // Proceed to last attribute of stream-tuples
  ListExpr rest = nl->Second(nl->Second(streamDescription));
  ListExpr next;
  while (!(nl->IsEmpty(rest)))
  {
    next = nl->First(rest);
    rest = nl->Rest(rest);
  }

  if(!listutils::isSymbol(nl->Second(next),TupleIdentifier::BasicType())){
   return listutils::typeError("last attribut must be of type tid");
  }

  if(!listutils::isBTreeDescription(btreeDescription)){
    return listutils::typeError("second argument is not a valid btree");
  }

  if(!listutils::isSymbol(nl->Third(btreeDescription))){
    return listutils::typeError("invalid key Type in btree");
  }

  if(!listutils::isSymbol(nameOfKeyAttribute)){
    return listutils::typeError("invalid attribute name");
  }


  rest = nl->Second(nl->Second(streamDescription)); // get attrlist

  if(nl->ListLength(rest)<2){
    return listutils::typeError("stream must contains at least 2 attributes");
  }

  ListExpr btreeAttrList = nl->Second(nl->Second(btreeDescription));

  ListExpr listn;
  ListExpr lastlistn;
  ListExpr restBTreeAttrs;
  if (opName == "updatebtree")
  {
    listn = nl->OneElemList(nl->First(rest));
    lastlistn = listn;
    rest = nl->Rest(rest);
    // Compare first part of the streamdescription
    while (nl->ListLength(rest) > nl->ListLength(btreeAttrList)+1)
    {
      lastlistn = nl->Append(lastlistn,nl->First(rest));
      rest = nl->Rest(rest);
    }
    if(!nl->Equal(listn, btreeAttrList)){
      return listutils::typeError("diffent type in btree and tuple stream");
    }

    restBTreeAttrs = btreeAttrList;
    while (nl->ListLength(rest) >  1) {
      string oldName;
      nl->WriteToString(oldName,
                        nl->First(nl->First(restBTreeAttrs)));
      oldName += "_old";
      ListExpr oldAttribute = nl->TwoElemList(nl->SymbolAtom(oldName),
                                     nl->Second(nl->First(restBTreeAttrs)));
      if(!nl->Equal(oldAttribute, nl->First(rest))){
         return listutils::typeError("Second part of the "
                   "tupledescription of the stream without the last "
                   "attribute has to be the same as the tuple"
                   "description of the btree except for that the "
                   "attributenames carry an additional '_old.'");
      }
      rest = nl->Rest(rest);
      restBTreeAttrs = nl->Rest(restBTreeAttrs);
    }
  }
  // For insert and delete check whether tupledescription of the stream
  // without the last attribute is the same as the tupledescription
  // of the btree
  else {
    listn = nl->OneElemList(nl->First(rest));
    lastlistn = listn;
    rest = nl->Rest(rest);
    while (nl->ListLength(rest) > 1) {
      lastlistn = nl->Append(lastlistn,nl->First(rest));
      rest = nl->Rest(rest);
    }
    if(!nl->Equal(listn, btreeAttrList)){
      return listutils::typeError( "tupledescription of the stream "
                   "without the last attribute has to be the same as the "
                   "tupledescription of the btree");

    }
  }


  // Test if attributename of the third argument exists as a name in the
  // attributlist of the streamtuples
  string attrname = nl->SymbolValue(nameOfKeyAttribute);
  ListExpr attrtype;
  int j = listutils::findAttribute(listn,attrname,attrtype);
  if(j==0){
    return listutils::typeError("attribute name not found");
  }
  ListExpr btreeKeyType = nl->Third(btreeDescription);
  if(!nl->Equal(attrtype, btreeKeyType)){
    return listutils::typeError("key attribute type "
                                "different from indexed type");
  }
  ListExpr outList = nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                          nl->OneElemList(nl->IntAtom(j)),streamDescription);
  return outList;
}

/*
6.4.1 TypeMapping of operator ~insertbtree~

*/
ListExpr insertBTreeTypeMap(ListExpr args)
{
  return allUpdatesBTreeTypeMap(args, "insertbtree");
}



/*
6.4.2 ValueMapping of operator ~insertbtree~

*/

int insertBTreeValueMap(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  Word t;
  Tuple* tup;
  BTree* btree;
  CcInt* indexp;
  int index;
  Attribute* keyAttr;
  Attribute* tidAttr;
  TupleId oldTid;
  SmiKey key;



  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      indexp = ((CcInt*)args[3].addr);
      local = SetWord(indexp );
      return 0;

    case REQUEST :
      index = ((CcInt*) local.addr)->GetIntval();
      btree = (BTree*)(args[1].addr);
      assert(btree != 0);
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        keyAttr = tup->GetAttribute(index - 1);
        tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
        oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
        AttrToKey((Attribute*)keyAttr, key, btree->GetKeyType());
        btree->Append(key,oldTid);
        result = SetWord(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s,1));
      local = SetWord(Address(0));
      return 0;
  }
  return 0;
}

/*
6.4.3 Specification of operator ~insertbtree~

*/
const string insertBTreeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>stream(tuple(x@[TID tid])) x (btree(tuple(x) ti) xi)"
  " -> stream(tuple(x@[TID tid]))] "
  "</text--->"
  "<text>_ _ insertbtree [_]</text--->"
  "<text>Inserts references to the tuples with TupleId 'tid' "
  "into the btree.</text--->"
  "<text>query neueStaedte feed staedte insert staedte_Name "
  " insertbtree [Name] count "
  "</text--->"
  ") )";

/*
6.4.4 Definition of operator ~insertbtree~

*/
Operator insertbtree (
         "insertbtree",              // name
         insertBTreeSpec,            // specification
         insertBTreeValueMap,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         insertBTreeTypeMap          // type mapping
);


/*
6.5 Operator ~deletebtree~

For each tuple of the inputstream deletes the corresponding entry from the
btree. The entry is built from the attribute of the tuple over which the tree
is built and the tuple-identifier of the deleted tuple which is extracted
as the last attribute of the tuple of the inputstream.


6.5.1 TypeMapping of operator ~deletebtree~

*/

ListExpr deleteBTreeTypeMap(ListExpr args)
{
  return allUpdatesBTreeTypeMap(args, "deletebtree");
}

/*
6.5.2 ValueMapping of operator ~deletebtree~

*/

int deleteBTreeValueMap(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  Word t;
  Tuple* tup;
  BTree* btree;
  CcInt* indexp;
  int index;
  Attribute* keyAttr;
  Attribute* tidAttr;
  TupleId oldTid;
  SmiKey key;


  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      indexp = ((CcInt*)args[3].addr);
      local = SetWord(indexp );
      return 0;

    case REQUEST :
      index = ((CcInt*) local.addr)->GetIntval();
      btree = (BTree*)(args[1].addr);
      assert(btree != 0);
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        keyAttr = tup->GetAttribute(index - 1);
        tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
        oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
        AttrToKey((Attribute*)keyAttr, key, btree->GetKeyType());
        btree->Delete(key,oldTid);
        result = SetWord(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s,1));
      local = SetWord(Address(0));
      return 0;
  }
  return 0;
}

/*
6.5.3 Specification of operator ~deletebtree~

*/
const string deleteBTreeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>stream(tuple(x@[TID tid])) x (btree(tuple(x) ti) xi)"
  " -> stream(tuple(x@[TID tid]))] "
  "</text--->"
  "<text>_ _ deletebtree [_]</text--->"
  "<text>Deletes the references to the tuples with TupleId 'tid' "
  "from the btree.</text--->"
  "<text>query alteStaedte feed staedte deletesearch staedte_Name "
  " deletebtree [Name] count "
  "</text--->"
  ") )";

/*
6.5.4 Definition of operator ~deletebtree~

*/
Operator deletebtree (
         "deletebtree",              // name
         deleteBTreeSpec,            // specification
         deleteBTreeValueMap,        // value mapping
         Operator::SimpleSelect,     // trivial selection function
         deleteBTreeTypeMap          // type mapping
);


/*
6.6 Operator ~updatebtree~

For each tuple of the inputstream updates the entry in the btree. The entry is
built from the attribute of the tuple over which the tree is built and the
tuple-identifier of the updated tuple which is extracted as the last attribute
of the tuple of the inputstream.

6.6.1 TypeMapping of operator ~updatebtree~

*/

ListExpr updateBTreeTypeMap(ListExpr args)
{
  return allUpdatesBTreeTypeMap(args, "updatebtree");
}

/*

6.6.2 ValueMapping of operator ~updatebtree~

*/

int updateBTreeValueMap(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  Word t;
  Tuple* tup;
  BTree* btree;
  CcInt* indexp;
  int index;
  Attribute* keyAttr;
  Attribute* oldKeyAttr;
  Attribute* tidAttr;
  TupleId oldTid;
  SmiKey key, oldKey;


  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      indexp = ((CcInt*)args[3].addr);
      local = SetWord(indexp );
      return 0;

    case REQUEST :
      index = ((CcInt*) local.addr)->GetIntval();
      btree = (BTree*)(args[1].addr);
      assert(btree != 0);
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        keyAttr = tup->GetAttribute(index - 1);
        oldKeyAttr = tup->GetAttribute((tup->GetNoAttributes()-1)/2+index-1);
        tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
        oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
        AttrToKey((Attribute*)keyAttr, key, btree->GetKeyType());
        AttrToKey((Attribute*)oldKeyAttr, oldKey, btree->GetKeyType());
        // Only update if key has changed
        if ((key > oldKey) || (oldKey > key))
        {
          btree->Delete(oldKey,oldTid);
          btree->Append(key,oldTid);
        }
        result = SetWord(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s,1));
      local = SetWord(Address(0));
      return 0;
  }
  return 0;
}


/*
6.6.3 Specification of operator ~updatebtree~

*/
const string updateBTreeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>stream(tuple(x@[(a1_old x1)...(an_old xn)(TID tid)])) x "
  "(btree(tuple(x) ti) xi)"
  " -> stream(tuple(x@[(a1_old x1)...(an_old xn)(TID tid)]))] "
  "</text--->"
  "<text>_ _ updatebtree [_]</text--->"
  "<text>Updates references to the tuples with TupleId 'tid' "
  "in the btree.</text--->"
  "<text>query staedte feed filter [.Name = 'Hargen'] staedte "
  "updatedirect [Name: "
  "'Hagen'] staedte_Name updatebtree [Name] count "
  "</text--->"
  ") )";

/*
6.6.4 Definition of operator ~updatebtree~

*/
Operator updatebtree (
         "updatebtree",              // name
         updateBTreeSpec,            // specification
         updateBTreeValueMap,        // value mapping
         Operator::SimpleSelect,     // trivial selection function
         updateBTreeTypeMap          // type mapping
);




/*
6.6 Operator ~getFileInfo~

Returns a text object with statistical information on all files used by the
btree.

The result has format ~file\_stat\_result~:

----
file_stat_result --> (file_stat_list)
file_stat_list   -->   epsilon
                     | file_statistics file_stat_list
file_statistics  -->   epsilon
                     | file_stat_field file_statistics
file_stat_field  --> ((keyname) (value))
keyname          --> string
value            --> string
----

6.6.1 TypeMapping of operator ~getFileInfo~

*/

ListExpr getFileInfoBtreeTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=1){
    return NList::typeError("expeced one argument");
  }

  ListExpr btreeDescription = nl->First(args);

  if(  nl->IsAtom(btreeDescription)
      || (nl->ListLength(btreeDescription) != 3)
    ) {
    return NList::typeError("1st argument is not a btree.");
  }
  ListExpr btreeSymbol = nl->First(btreeDescription);;
  if(    !nl->IsAtom(btreeSymbol)
      || (nl->AtomType(btreeSymbol) != SymbolType)
      || (nl->SymbolValue(btreeSymbol) != BTree::BasicType())
    ){
    return NList::typeError("1st argument is not a btree.");
  }
  return NList(FText::BasicType()).listExpr();
}

/*

6.6.2 ValueMapping of operator ~getFileInfo~

*/

int getFileInfoBtreeValueMap(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  FText* restext = (FText*)(result.addr);
  BTree* btree   = (BTree*)(args[0].addr);
  SmiStatResultType resVector(0);

  if ( (btree != 0) && btree->getFileStats(resVector) ){
    string resString = "[[\n";
    for(SmiStatResultType::iterator i = resVector.begin();
        i != resVector.end(); ){
      resString += "\t[['" + i->first + "'],['" + i->second + "']]";
      if(++i != resVector.end()){
        resString += ",\n";
      } else {
        resString += "\n";
      }
    }
    resString += "]]";
    restext->Set(true,resString);
  } else {
    restext->Set(false,"");
  }
  return 0;
}

/*
6.7.3 Specification of operator ~getFileInfo~

*/
const string getFileInfoBtreeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(btree(tuple(x) ti) xi -> text)</text--->"
  "<text>getFileInfo( _ )</text--->"
  "<text>Retrieve statistical infomation on the file(s) used by the btree "
  "instance.</text--->"
  "<text>query getFileInfo(plz_PLZ_btree)</text--->"
  ") )";


/*
6.7.4 Definition of operator ~getFileInfo~

*/
Operator getfileinfobtree (
         "getFileInfo",              // name
         getFileInfoBtreeSpec,       // specification
         getFileInfoBtreeValueMap,   // value mapping
         Operator::SimpleSelect,     // trivial selection function
         getFileInfoBtreeTypeMap     // type mapping
);



/*
6.7.5 Definition of operator ~keyrange~

*/


template <typename T>
void init_ptr(T*& p, Word& w)
{ assert(w.addr != 0); p = static_cast<T*>(w.addr); }

template <typename T>
void del_ptr(Word& w)
{ assert(w.addr != 0); delete static_cast<T*>(w.addr); w.addr = 0; }

template <typename T>
void set_val(Word& w, const T& val)
{ (*static_cast<T*>(w.addr)) = val; }


int
keyrange_vm(Word* args, Word& result, int message, Word& local, Supplier s)
{

  switch (message)
  {
    case OPEN : {

      local.addr = new int;
      set_val(local, 1);
      return 0;

    }
    case REQUEST : {

      int* k;
      init_ptr(k, local);
      if(*k == 1)
      {

        BTree* btree;
        Relation* relation;
        Attribute* key;

        init_ptr(btree, args[0]);
        init_ptr(relation, args[1]);
        init_ptr(key, args[2]);

        SmiKeyRange factors;
	SmiKey sk;
        AttrToKey( key, sk, btree->GetFile()->GetKeyType() );
	btree->GetFile()->KeyRange(sk, factors);
	int n = relation->GetNoTuples();

	  /*
	  cout << "btree.less = " << factors.less << endl;
	  cout << "btree.equal = " << factors.equal << endl;
	  cout << "btree.right = " << factors.greater << endl;
	  cout << "relation.tuples = " << n << endl;
	  */

	CcReal* lt = new CcReal(factors.less);
	CcReal* eq = new CcReal(factors.equal);
	CcReal* gt = new CcReal(factors.greater);
	CcInt* nk = new CcInt(n);

	Tuple* t = new Tuple( nl->Second(GetTupleResultType(s)) );
	t->PutAttribute(0, lt);
	t->PutAttribute(1, eq);
	t->PutAttribute(2, gt);
	t->PutAttribute(3, nk);

        result.addr = t;
	*k = 0;
        return YIELD;
      }
      else
      {
	result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE : {

      del_ptr<int>(local);
      local.addr = 0;
      return 0;
    }
  }
  return 0;
}


struct keyrangeInfo : OperatorInfo {

  keyrangeInfo()
  {
    name      = "keyrange";

    signature = "(btree t) x (rel t) x key "
	        " -> " "stream(tuple((Less real)(Equal real)"
		"(Greater real)(NumOfKeys int)";
    syntax    = "_ _ keyrange(_)";
    meaning   = "Retrieves an estimate for the number of tuples which are"
	        " less, equal or grater than the given value";
  }
};


/*

7 Definition and initialization of btree algebra

*/
class BTreeAlgebra : public Algebra
{
 public:
  BTreeAlgebra() : Algebra()
  {
    AddTypeConstructor( &cppbtree );

#ifndef USE_PROGRESS

    AddOperator(&createbtree);
    AddOperator(&exactmatch);
    AddOperator(&leftrange);
    AddOperator(&rightrange);
    AddOperator(&cpprange);
    AddOperator(&exactmatchs);
    AddOperator(&leftranges);
    AddOperator(&rightranges);
    AddOperator(&cppranges);
    AddOperator(&insertbtree);
    AddOperator(&deletebtree);
    AddOperator(&updatebtree);
    AddOperator(&getfileinfobtree);
    AddOperator(keyrangeInfo(), keyrange_vm, IndexQueryTypeMap<KEYRANGE> );

#else

    AddOperator(&createbtree);
    AddOperator(&exactmatch);   exactmatch.EnableProgress();
    AddOperator(&leftrange);    leftrange.EnableProgress();
    AddOperator(&rightrange);   rightrange.EnableProgress();
    AddOperator(&cpprange);     cpprange.EnableProgress();
    AddOperator(&exactmatchs);
    AddOperator(&leftranges);
    AddOperator(&rightranges);
    AddOperator(&cppranges);
    AddOperator(&insertbtree);
    AddOperator(&deletebtree);
    AddOperator(&updatebtree);
    AddOperator(&getfileinfobtree);
    AddOperator(keyrangeInfo(), keyrange_vm, IndexQueryTypeMap<KEYRANGE> );

#endif


  }
  ~BTreeAlgebra() {};
};


extern "C"
Algebra*
InitializeBTreeAlgebra( NestedList* nlRef,
                        QueryProcessor* qpRef,
                        AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (new BTreeAlgebra());
}
