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
(~executable~, ~descriptive~, and ~hibrid~). Only the executable 
level remains. Models are also removed from type constructors.

[1] Implementation of BTree Algebra

[TOC]

2 Includes and Defines

*/
using namespace std;

#include "Algebra.h"
#include "AlgebraManager.h"
#include "SecondoSystem.h"
#include "SecondoCatalog.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DateTime.h"
#include "TupleIdentifier.h"

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

extern NestedList* nl;
extern QueryProcessor *qp;

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

Converts a ~StandardAttribute~ to a ~SmiKey~.

*/
void AttrToKey(
  StandardAttribute* attr,
  SmiKey& key,
  SmiKey::KeyDataType keyType)
{
  float floatval;
  int intval;
  string strval;

  assert(attr->IsDefined());
  switch(keyType)
  {
    case SmiKey::Integer:
      intval = ((CcInt*)attr)->GetIntval();
      key = SmiKey((long)intval);
      break;

    case SmiKey::Float:
      floatval = ((CcReal*)attr)->GetRealval();
      key = SmiKey((double)floatval);
      break;

    case SmiKey::String:
      strval = (char*)((CcString*)attr)->GetStringval();
      key = SmiKey(strval);
      break;

    case SmiKey::Composite:
      key = SmiKey((IndexableStandardAttribute*)attr);
      break;

    default:
      assert(false /* should not reach this */);
      break;
  }
  assert(key.GetType() == keyType);
}

/*
2.3 Function ~KeyToAttr~

Converts a ~SmiKey~ to a ~StandardAttribute~.

*/
void KeyToAttr(
  StandardAttribute* attr,
  SmiKey& key,
  SmiKey::KeyDataType keyType)
{
  double floatval;
  long intval;
  string strval;

  assert(key.GetType() == keyType);
  switch(keyType)
  {
    case SmiKey::Integer:
      key.GetKey(intval);
      ((CcInt*)attr)->Set(true, (int)intval);
      break;

    case SmiKey::Float:
      key.GetKey(floatval);
      ((CcReal*)attr)->Set(true, (float)floatval);
      break;

    case SmiKey::String:
      key.GetKey(strval);
      ((CcString*)attr)->Set(true, (STRING*)strval.c_str());
      break;

    case SmiKey::Composite:
      key.GetKey((IndexableStandardAttribute*)attr);
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
  AlgebraManager* alg = SecondoSystem::GetAlgebraManager();

  int algId = nl->IntValue( nl->First( nl->Third( typeInfo ) ) ),
      typeId = nl->IntValue( nl->Second( nl->Third( typeInfo ) ) );

  string keyTypeString = alg->Constrs( algId, typeId );
  if( keyTypeString == "int" )
  {
    return SmiKey::Integer;
  }
  else if( keyTypeString == "string" )
  {
    return SmiKey::String;
  }
  else if( keyTypeString == "real" )
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

The key attribute of a btree can be an ~int~, a ~string~, ~real~, or a composite one.

*/
BTree::BTree( SmiKey::KeyDataType keyType, bool temporary ):
temporary( temporary ),
file( 0 ),
opened( false )
{
  if( keyType != SmiKey::Unknown )
  {
    file = new SmiKeyedFile( keyType,false, temporary );
    if( file->Create() )
      opened = true;
    else
    {
      delete file; file = 0;
    }
  }
}

BTree::BTree( SmiKey::KeyDataType keyType, SmiRecord& record ):
temporary( false ),
file( 0 ),
opened( false )
{
  SmiFileId fileId;
  if( record.Read( &fileId, sizeof(SmiFileId) ) != sizeof(SmiFileId) )
    return;
 
  this->file = new SmiKeyedFile( keyType,false );
  if( file->Open( fileId ) )
    opened = true;
  else 
  {
    delete file; file = 0;
  }
}

BTree::BTree( SmiKey::KeyDataType keyType, SmiFileId fileId ):
temporary( true ),
file( 0 ),
opened( false )
{
  if( keyType != SmiKey::Unknown )
  {
    file = new SmiKeyedFile( keyType );
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

SmiKeyedFile* BTree::GetFile() const
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

BTreeIterator* BTree::ExactMatch( StandardAttribute* key )
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

BTreeIterator* BTree::LeftRange( StandardAttribute* key )
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

BTreeIterator* BTree::RightRange( StandardAttribute* key )
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

BTreeIterator* BTree::Range( StandardAttribute* left, StandardAttribute* right )
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
  return SetWord(0);
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
void CloseBTree(Word& w)
{
  BTree* btree = (BTree*)w.addr;
  delete btree;
}
/*

5.5 Function ~CloneBTree~

*/
Word CloneBTree(const Word& w)
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
void DeleteBTree(Word& w)
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
    && nl->Equal(nl->First(type), nl->SymbolAtom("btree")))
  {
    AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
    return
      algMgr->CheckKind("TUPLE", nl->Second(type), errorInfo)
      && algMgr->CheckKind("DATA", nl->Third(type), errorInfo);
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
  return value.addr != 0;
}

BTree *BTree::Open( SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo )
{
  return new BTree( ExtractKeyTypeFromTypeInfo( typeInfo ), valueRecord );
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
  SmiFileId fileId = file->GetFileId();
  if( record.Write( &fileId, sizeof(SmiFileId) ) != sizeof(SmiFileId) )
    return false;

  return true;
}

/*

5.11 Type Constructor object for type constructor ~btree~

*/
TypeConstructor cppbtree( "btree",				BTreeProp,
                          OutBTree,				InBTree,
                          SaveToListBTree,      RestoreFromListBTree,
                          CreateBTree,			DeleteBTree,
						              OpenBTree,			SaveBTree,
            						  CloseBTree,			CloneBTree,
						              CastBTree,   			SizeOfBTree,
                          CheckBTree );

/*

6 Operators of the btree algebra

6.1 Type Mapping of operator ~createbtree~

*/
ListExpr CreateBTreeTypeMap(ListExpr args)
{
  string argstr;

  CHECK_COND(nl->ListLength(args) == 2, 
             "Operator createbtree expects a list of length two");

  ListExpr first = nl->First(args);

  nl->WriteToString(argstr, first);
  CHECK_COND((nl->IsEqual(nl->First(first), "rel") && IsRelDescription(first)) ||
             (nl->IsEqual(nl->First(first), "stream") && IsTupleDescription(nl->Second(nl->Second(first)))),
             "Operator createbtree expects as first argument a list with structure\n"
             "rel(tuple ((a1 t1)...(an tn))) or stream (tuple ((a1 t1)...(an tn)))\n"
             "Operator createbtree gets a list with structure '" + argstr + "'."); 

  ListExpr second = nl->Second(args);
  nl->WriteToString(argstr, second);
  
  CHECK_COND(nl->IsAtom(second) && nl->AtomType(second) == SymbolType, 
             "Operator createbtree expects as second argument an attribute name\n"
             "bug gets '" + argstr + "'.");

  string attrName = nl->SymbolValue(second);
  ListExpr tupleDescription = nl->Second(first),
           attrList = nl->Second(tupleDescription);

  nl->WriteToString(argstr, attrList);
  int attrIndex;
  ListExpr attrType;
  CHECK_COND((attrIndex = FindAttribute(attrList, attrName, attrType)) > 0, 
             "Operator createbtree expects as a second argument an attribute name\n"
             "Attribute name '" + attrName + "' is not known.\n"
             "Known Attribute(s): " + argstr);

  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  AlgebraManager* alg = SecondoSystem::GetAlgebraManager();
  nl->WriteToString(argstr, attrType);
  CHECK_COND(nl->SymbolValue(attrType) == "string" ||
             nl->SymbolValue(attrType) == "int" ||
             nl->SymbolValue(attrType) == "real" ||
             alg->CheckKind("INDEXABLE", attrType, errorInfo), 
             "Operator createbtree expects as a second argument an attribute of types\n"
             "int, real, string, or any attribute that implements the kind INDEXABLE\n"
             "but gets '" + argstr + "'.");

  if( nl->IsEqual(nl->First(first), "rel") )
  {
    return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->OneElemList(
          nl->IntAtom(attrIndex)),
        nl->ThreeElemList(
          nl->SymbolAtom("btree"),
          tupleDescription,
          attrType));
  }
  else // nl->IsEqual(nl->First(first), "stream")
  {
    // Find the attribute with type tid
    ListExpr first, rest, newAttrList, lastNewAttrList;
    int j, tidIndex = 0;
    string type;
    bool firstcall = true;

    rest = attrList;
    j = 1;
    while (!nl->IsEmpty(rest))
    {
      first = nl->First(rest);
      rest = nl->Rest(rest);

      type = nl->SymbolValue(nl->Second(first));
      if (type == "tid")
      {
        nl->WriteToString(argstr, attrList);
        CHECK_COND(tidIndex == 0,
                   "Operator createbtree expects as first argument a stream with\n"
                   "one and only one attribute of type tid but gets\n'" + argstr + "'.");
        tidIndex = j;
      }
      else
      {
        if (firstcall)
        {
          firstcall = false;
          newAttrList = nl->OneElemList(first);
          lastNewAttrList = newAttrList;
        }
        else
        {
          lastNewAttrList = nl->Append(lastNewAttrList, first);
        }
      }
      j++;
    }
    CHECK_COND( tidIndex != 0,
                "Operator createbtree expects as first argument a stream with\n"
                "one and only one attribute of type tid but gets\n'" + argstr + "'.");

    return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->TwoElemList(
          nl->IntAtom(attrIndex),
          nl->IntAtom(tidIndex)),
        nl->ThreeElemList(
          nl->SymbolAtom("btree"),
          nl->TwoElemList(
            nl->SymbolAtom("tuple"),
            newAttrList),
          attrType));
  }
}

/*
6.2 Selection function of operator ~createbtree~

*/
int CreateBTreeSelect( ListExpr args )
{
  if( nl->IsEqual(nl->First(nl->First(args)), "rel") )
    return 0;
  if( nl->IsEqual(nl->First(nl->First(args)), "stream") )
    return 1; 
  return -1;
}

/*
6.2 Value mapping function of operator ~createbtree~

*/
int
CreateBTreeValueMapping_Rel(Word* args, Word& result, int message, Word& local, Supplier s)
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

  RelationIterator *iter = relation->MakeScan();
  Tuple* tuple;
  while( (tuple = iter->GetNextTuple()) != 0 )
  {
    SmiKey key;

    if( (StandardAttribute *)tuple->GetAttribute(attrIndex)->IsDefined() )
    {
      AttrToKey( (StandardAttribute *)tuple->GetAttribute(attrIndex), key, btree->GetKeyType() );
      btree->Append( key, iter->GetTupleId() );
    }
    tuple->DeleteIfAllowed();
  }
  delete iter;

  return 0;
}

int
CreateBTreeValueMapping_Stream(Word* args, Word& result, int message, Word& local, Supplier s)
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
    if( (StandardAttribute *)tuple->GetAttribute(attrIndex)->IsDefined() &&
        (StandardAttribute *)tuple->GetAttribute(tidIndex)->IsDefined() )
    {
      AttrToKey( (StandardAttribute *)tuple->GetAttribute(attrIndex), key, btree->GetKeyType() );
      btree->Append( key, ((TupleIdentifier *)tuple->GetAttribute(tidIndex))->GetTid() );
    }
    tuple->DeleteIfAllowed();
   
    qp->Request(args[0].addr, wTuple); 
  }
  qp->Close(args[0].addr);

  return 0;
}

/*

6.3 Specification of operator ~createbtree~

*/
const string CreateBTreeSpec  = "( ( \"1st Signature\" \"2nd Signature\" "
                                "\"Syntax\" \"Meaning\" \"1st Example\" "
                                "\"2nd Example\" ) "
                                "( <text>((rel (tuple (x1 t1)...(xn tn)))) xi)"
								                " -> (btree (tuple ((x1 t1)...(xn tn))) ti)</text--->"
                                "<text>((stream (tuple (x1 t1)...(xn tn) (id tid))) xi)"
								                " -> (btree (tuple ((x1 t1)...(xn tn))) ti)</text--->"
						                    "<text>_ createbtree [ _ ]</text--->"
             						        "<text>Creates a btree. The key type ti must"
						                		" be either string or int or real or to implement the"
                                " kind INDEXABLE.</text--->"
            						        "<text>let mybtree = ten createbtree [nr]</text--->"
                                "<text>let mybtree = ten feed extend[id: tupleid(.)] "
                                "sortby[no asc] createbtree[no]</text--->"
             						        ") )";

/*

6.4 Definition of operator ~createbtree~

*/
ValueMapping createbtreemap[] = { CreateBTreeValueMapping_Rel, 
                                  CreateBTreeValueMapping_Stream };

Operator createbtree (
          "createbtree",             // name
          CreateBTreeSpec,           // specification
          2,	                       // number of overloaded functions
          createbtreemap,            // value mapping
          CreateBTreeSelect,         // trivial selection function
          CreateBTreeTypeMap         // type mapping
);

const int LEFTRANGE = 0;
const int RIGHTRANGE = 1;
const int RANGE = 2;
const int EXACTMATCH = 3;

int nKeyArguments(int opid)
{
  assert(opid >= LEFTRANGE && opid <= EXACTMATCH);
  return opid == RANGE ? 2 : 1;
}

char* errorMessages[] =
  { "Incorrect input for operator leftrange.",
    "Incorrect input for operator rightrange.",
    "Incorrect input for operator range.",
    "Incorrect input for operator exactmatch." };
/*

6.5 Type mapping function of operators ~range~, ~leftrange~,
~rightrange~ and ~exactmatch~

*/
template<int operatorId>
ListExpr IndexQueryTypeMap(ListExpr args)
{
  char* errmsg = errorMessages[operatorId];
  int nKeys = nKeyArguments(operatorId);

  CHECK_COND(!nl->IsEmpty(args), errmsg);
  CHECK_COND(!nl->IsAtom(args), errmsg);
  CHECK_COND(nl->ListLength(args) == nKeys + 2, errmsg);

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
  CHECK_COND(nl->IsAtom(keyDescription), errmsg);
  CHECK_COND(nl->AtomType(keyDescription) == SymbolType, errmsg);

  /* find out type of second key (if any) */
  if(nKeys == 2)
  {
    CHECK_COND(nl->IsAtom(secondKeyDescription), errmsg);
    CHECK_COND(nl->AtomType(secondKeyDescription) == SymbolType, errmsg);
    CHECK_COND(nl->Equal(keyDescription, secondKeyDescription), errmsg);
  }

  /* handle btree part of argument */
  CHECK_COND(!nl->IsEmpty(btreeDescription), errmsg);
  CHECK_COND(!nl->IsAtom(btreeDescription), errmsg);
  CHECK_COND(nl->ListLength(btreeDescription) == 3, errmsg);

  ListExpr btreeSymbol = nl->First(btreeDescription);;
  ListExpr btreeTupleDescription = nl->Second(btreeDescription);
  ListExpr btreeKeyType = nl->Third(btreeDescription);

  /* check that the type of given key equals the btree key type */
  CHECK_COND(nl->Equal(keyDescription, btreeKeyType), errmsg);

  /* handle btree type constructor */
  CHECK_COND(nl->IsAtom(btreeSymbol), errmsg);
  CHECK_COND(nl->AtomType(btreeSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(btreeSymbol) == "btree", errmsg);

  CHECK_COND(!nl->IsEmpty(btreeTupleDescription), errmsg);
  CHECK_COND(!nl->IsAtom(btreeTupleDescription), errmsg);
  CHECK_COND(nl->ListLength(btreeTupleDescription) == 2, errmsg);
  ListExpr btreeTupleSymbol = nl->First(btreeTupleDescription);;
  ListExpr btreeAttrList = nl->Second(btreeTupleDescription);

  CHECK_COND(nl->IsAtom(btreeTupleSymbol), errmsg);
  CHECK_COND(nl->AtomType(btreeTupleSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(btreeTupleSymbol) == "tuple", errmsg);
  CHECK_COND(IsTupleDescription(btreeAttrList), errmsg);

  /* handle rel part of argument */
  CHECK_COND(!nl->IsEmpty(relDescription), errmsg);
  CHECK_COND(!nl->IsAtom(relDescription), errmsg);
  CHECK_COND(nl->ListLength(relDescription) == 2, errmsg);

  ListExpr relSymbol = nl->First(relDescription);;
  ListExpr tupleDescription = nl->Second(relDescription);

  CHECK_COND(nl->IsAtom(relSymbol), errmsg);
  CHECK_COND(nl->AtomType(relSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(relSymbol) == "rel", errmsg);

  CHECK_COND(!nl->IsEmpty(tupleDescription), errmsg);
  CHECK_COND(!nl->IsAtom(tupleDescription), errmsg);
  CHECK_COND(nl->ListLength(tupleDescription) == 2, errmsg);
  ListExpr tupleSymbol = nl->First(tupleDescription);
  ListExpr attrList = nl->Second(tupleDescription);

  CHECK_COND(nl->IsAtom(tupleSymbol), errmsg);
  CHECK_COND(nl->AtomType(tupleSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(tupleSymbol) == "tuple", errmsg);
  CHECK_COND(IsTupleDescription(attrList), errmsg);

  /* check that btree and rel have the same associated tuple type */
  CHECK_COND(nl->Equal(attrList, btreeAttrList), errmsg);

  ListExpr resultType =
    nl->TwoElemList(
      nl->SymbolAtom("stream"),
      tupleDescription);

  return resultType;
}

struct IndexQueryLocalInfo
{
  Relation* relation;
  BTreeIterator* iter;
};

/*

6.6 Value mapping function of operators ~range~, ~leftrange~,
~rightrange~ and ~exactmatch~

*/
template<int operatorId>
int
IndexQuery(Word* args, Word& result, int message, Word& local, Supplier s)
{
  BTree* btree;
  StandardAttribute* key;
  StandardAttribute* secondKey;
  Tuple* tuple;
  SmiRecordId id;
  IndexQueryLocalInfo* localInfo;

  Word relWord;
  Word btreeWord;
  Word keyWord;
  Word secondKeyWord;

  switch (message)
  {
    case OPEN :
      qp->Request(args[0].addr, btreeWord);
      qp->Request(args[1].addr, relWord);
      qp->Request(args[2].addr, keyWord);
      if(operatorId == RANGE)
      {
        qp->Request(args[3].addr, secondKeyWord);
      }

      localInfo = new IndexQueryLocalInfo;
      btree = (BTree*)btreeWord.addr;
      localInfo->relation = (Relation*)relWord.addr;
      key = (StandardAttribute*)keyWord.addr;
      if(operatorId == RANGE)
      {
        secondKey = (StandardAttribute*)secondKeyWord.addr;
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
      localInfo = (IndexQueryLocalInfo*)local.addr;
      delete localInfo->iter;
      delete localInfo;
      return 0;
  }
  return 0;
}

/*

6.7 Specification of operator ~exactmatch~

*/
const string ExactMatchSpec  = 	"( ( \"Signature\" \"Syntax\" \"Meaning\""
                               	" \"Example\" )"
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

6.8 Definition of operator ~exactmatch~

*/
Operator exactmatch (
         "exactmatch",            // name
	 ExactMatchSpec,          // specification
	 IndexQuery<EXACTMATCH>,              // value mapping
	 Operator::SimpleSelect,         // trivial selection function
	 IndexQueryTypeMap<EXACTMATCH>        // type mapping
);

/*

6.9 Specification of operator ~range~

*/
const string RangeSpec  = 	"( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          	"\"Example\" )"
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

6.10 Definition of operator ~range~

*/
Operator cpprange (
         "range",            // name
	 RangeSpec,          // specification
	 IndexQuery<RANGE>,              // value mapping
	 Operator::SimpleSelect,         // trivial selection function
	 IndexQueryTypeMap<RANGE>        // type mapping
);

/*

6.11 Specification of operator ~leftrange~

*/
const string LeftRangeSpec  = 	"( ( \"Signature\" \"Syntax\" \"Meaning\" "
                  	            "\"Example\" ) "
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

6.12 Definition of operator ~leftrange~

*/
Operator leftrange (
         "leftrange",            // name
	 LeftRangeSpec,          // specification
	 IndexQuery<LEFTRANGE>,              // value mapping
	 Operator::SimpleSelect,         // trivial selection function
	 IndexQueryTypeMap<LEFTRANGE>        // type mapping
);

/*

6.13 Specification of operator ~rightrange~

*/
const string RightRangeSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" ) "
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

6.14 Definition of operator ~rightrange~

*/
Operator rightrange (
         "rightrange",            // name
	 RightRangeSpec,          // specification
	 IndexQuery<RIGHTRANGE>,              // value mapping
	 Operator::SimpleSelect,         // trivial selection function
	 IndexQueryTypeMap<RIGHTRANGE>        // type mapping
);

/*
6.15 Operator ~insertbtree~ 

For each tuple of the inputstream inserts an entry into the btree. The entry is built from the attribute 
of the tuple over which the tree is built and the tuple-identifier of the inserted tuple which is extracted
as the last attribute of the tuple of the inputstream.


6.15.0 General Type mapping function of operators ~insertbtree~, ~deletebtree~ and ~updatebtree~


Type mapping ~insertbtree~ and ~deletebtree~ 

----     (stream (tuple ((a1 x1) ... (an xn) (TID tid)))) (btree X ti)) ai

        -> (stream (tuple ((a1 x1) ... (an xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----

Type mapping ~updatebtree~

----     (stream (tuple ((a1 x1) ... (an xn) (a1_old x1) ... (an_old xn) (TID tid)))) (btree X ti)) ai

        -> (stream (tuple ((a1 x1) ... (an xn) (a1_old x1) ... (an_old xn)(TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----


*/

ListExpr allUpdatesBTreeTypeMap( const ListExpr& args, string opName )
{
  	ListExpr  rest,next,listn,lastlistn,restBTreeAttrs,oldAttribute, outList;
  	string argstr, argstr2, oldName;
  

   	/* Split argument in three parts */
  	ListExpr streamDescription = nl->First(args);
  	ListExpr btreeDescription = nl->Second(args);
  	ListExpr nameOfKeyAttribute = nl->Third(args);

	// Test stream
  	nl->WriteToString(argstr, streamDescription);
  	CHECK_COND(nl->ListLength(streamDescription) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(streamDescription)) == stream) &&
             	(nl->ListLength(nl->Second(streamDescription)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(streamDescription))) == tuple) &&
       			(nl->ListLength(nl->Second(streamDescription)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(streamDescription)))),
    			"Operator " + opName + " expects as first argument a list with structure "
    			"(stream (tuple ((a1 t1)...(an tn)(TID tid)))\n "
    			"Operator " + opName + " gets as first argument '" + argstr + "'." );
    
  	// Proceed to last attribute of stream-tuples
  	rest = nl->Second(nl->Second(streamDescription));
  	while (!(nl->IsEmpty(rest)))
  	{
     	next = nl->First(rest);
     	rest = nl->Rest(rest);
  	}
  	
  	CHECK_COND(!(nl->IsAtom(next)) &&
             	(nl->IsAtom(nl->Second(next)))&&
             	(nl->AtomType(nl->Second(next)) == SymbolType) &&
             	 (nl->SymbolValue(nl->Second(next)) == "tid") ,
    			"Operator " + opName + ": Type of last attribute of tuples of the inputstream must be tid" );
 	// Test btree

  	/* handle btree part of argument */
  	CHECK_COND(!nl->IsEmpty(btreeDescription), "Operator " + opName + ": Description for the btree may not be empty");
  	CHECK_COND(!nl->IsAtom(btreeDescription), "Operator " + opName + ": Description for the btree may not be an atom");
  	CHECK_COND(nl->ListLength(btreeDescription) == 3, "Operator " + opName + ": Description for the btree must consist of three parts");

  	ListExpr btreeSymbol = nl->First(btreeDescription);;
  	ListExpr btreeTupleDescription = nl->Second(btreeDescription);
  	ListExpr btreeKeyType = nl->Third(btreeDescription);

  	/* handle btree type constructor */
  	CHECK_COND(nl->IsAtom(btreeSymbol), "Operator " + opName + ": First part of the btree-description has to be 'btree'");
  	CHECK_COND(nl->AtomType(btreeSymbol) == SymbolType, "Operator " + opName + ": First part of the btree-description has to be 'bree' ");
  	CHECK_COND(nl->SymbolValue(btreeSymbol) == "btree","Operator " + opName + ": First part of the btree-description has to be 'bree' ");

  	/* handle btree tuple description */
  	CHECK_COND(!nl->IsEmpty(btreeTupleDescription), "Operator " + opName + ": Second part of the btree-description has to be a tuple-description ");
  	CHECK_COND(!nl->IsAtom(btreeTupleDescription), "Operator " + opName + ": Second part of the btree-description has to be a tuple-description ");
  	CHECK_COND(nl->ListLength(btreeTupleDescription) == 2, "Operator " + opName + ": Second part of the btree-description has to be a tuple-description ");
  	ListExpr btreeTupleSymbol = nl->First(btreeTupleDescription);;
  	ListExpr btreeAttrList = nl->Second(btreeTupleDescription);

  	CHECK_COND(nl->IsAtom(btreeTupleSymbol),"Operator " + opName + ": Second part of the btree-description has to be a tuple-description ");
  	CHECK_COND(nl->AtomType(btreeTupleSymbol) == SymbolType, "Operator " + opName + ": Second part of the btree-description has to be a tuple-description ");
  	CHECK_COND(nl->SymbolValue(btreeTupleSymbol) == "tuple", "Operator " + opName + ": Second part of the btree-description has to be a tuple-description ");
  	CHECK_COND(IsTupleDescription(btreeAttrList), "Operator " + opName + ": Second part of the btree-description has to be a tuple-description ");
  
   	/* Handle key-part of btreedescription */
  	CHECK_COND(nl->IsAtom(btreeKeyType), "Operator " + opName + ": Key of the btree has to be an atom");
  	CHECK_COND(nl->AtomType(btreeKeyType) == SymbolType,"Operator " + opName + ": Key of the btree has to be an atom");
  
  	// Handle third argument which shall be the name of the attribute of the streamtuples
  	// that serves as the key for the btree
  	// Later on it is checked if this name is an attributename of the inputtuples
  	CHECK_COND(nl->IsAtom(nameOfKeyAttribute), "Operator " + opName + ": Name of the key-attribute of the streamtuples has to be an atom");
  	CHECK_COND(nl->AtomType(nameOfKeyAttribute) == SymbolType, "Operator " + opName + ": Name of the key-attribute of the streamtuples has to be an atom");

  	//Test if stream-tupledescription fits to btree-tupledescription
  	rest = nl->Second(nl->Second(streamDescription));
  	CHECK_COND(nl->ListLength(rest) > 1 , "Operator " + opName + ": There must be at least two attributes in the tuples of the tuple-stream");
  	// For updates the inputtuples need to carry the old attributevalues after the
  	// new values but their names with an additional _old at the end
  	if (opName == "updatebtree"){
  		listn = nl->OneElemList(nl->First(rest));
  		lastlistn = listn;
  		rest = nl->Rest(rest);
  		// Compare first part of the streamdescription
  		while (nl->ListLength(rest) > nl->ListLength(btreeAttrList) + 1)
  		{
     		lastlistn = nl->Append(lastlistn,nl->First(rest));
     		rest = nl->Rest(rest);
  		}
  		CHECK_COND(nl->Equal(listn,btreeAttrList), "Operator " + opName + ":  First part of the tupledescription of the stream "
  					"has to be the same as the tupledescription of the btree");
  		// Compare second part of the streamdescription
  		restBTreeAttrs = btreeAttrList;
  		while (nl->ListLength(rest) >  1)
  		{
  	 		nl->WriteToString(oldName, nl->First(nl->First(restBTreeAttrs)));
  	 		oldName += "_old";
  	 		oldAttribute = nl->TwoElemList(nl->SymbolAtom(oldName),nl->Second(nl->First(restBTreeAttrs)));
  	 		CHECK_COND(nl->Equal(oldAttribute,nl->First(rest)), "Operator " + opName + ":  Second part of the tupledescription of the stream "
     					"without the last attribute has to be the same as the tupledescription of the btree except for that"
     					" the attributenames carry an additional '_old.'");
     		rest = nl->Rest(rest);
     		restBTreeAttrs = nl->Rest(restBTreeAttrs);
  		}
  	}
  	// For insert and delete check whether tupledescription of the stream without the last 
  	//attribute is the same as the tupledescription of the btree 
  	else{
  		listn = nl->OneElemList(nl->First(rest));
  		lastlistn = listn;
  		rest = nl->Rest(rest);
  		while (nl->ListLength(rest) > 1)
  		{
     		lastlistn = nl->Append(lastlistn,nl->First(rest));
     		rest = nl->Rest(rest);
  		}
  		CHECK_COND(nl->Equal(listn,btreeAttrList), "Operator " + opName + ": tupledescription of the stream without the"
  					"last attribute has to be the same as the tupledescription of the btree");
  	}
  
  
	// Test if attributename of the third argument exists as a name in the attributlist of the streamtuples
	string attrname = nl->SymbolValue(nameOfKeyAttribute);
	ListExpr attrtype;
	int j = FindAttribute(listn,attrname,attrtype);
	CHECK_COND(j != 0, "Operator " + opName + ": Name of the attribute that shall contain the keyvalue for the"
	 			"btree was not found as a name of the attributes of the tuples of the inputstream"); 
	//Test if type of the attriubte which shall be taken as a key is the same as the keytype of the btree
	CHECK_COND(nl->Equal(attrtype,btreeKeyType), "Operator " + opName + ": Type of the attribute that shall contain the keyvalue for the"
	 			"btree is not the same as the keytype of the btree");
	//Append the index of the attribute over which the btree is built to the resultlist. 
	outList = nl->ThreeElemList(nl->SymbolAtom("APPEND"), nl->OneElemList(nl->IntAtom(j)),streamDescription);
  	return outList;
}

/*
6.15.1 TypeMapping of operator ~insertbtree~

*/
ListExpr insertBTreeTypeMap(ListExpr args){
	return allUpdatesBTreeTypeMap(args, "insertbtree");
}



/*
6.15.2 ValueMapping of operator ~insertbtree~

*/

int insertBTreeValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, argBTree, attrPos;
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
      qp->Request(args[3].addr,attrPos);
      indexp = ((CcInt*)attrPos.addr);
      local = SetWord(indexp );
      return 0;

    case REQUEST :
      index = ((CcInt*) local.addr)->GetIntval();
      qp->Request(args[1].addr, argBTree);
      btree = (BTree*)(argBTree.addr);    
      assert(btree != 0);
      qp->Request(args[0].addr,t);     
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
       	keyAttr = tup->GetAttribute(index - 1);
       	tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
       	oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
       	AttrToKey((StandardAttribute*)keyAttr, key, btree->GetKeyType());
       	btree->Append(key,oldTid);
        result = SetWord(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
    	qp->Close(args[0].addr);
    	qp->SetModified(args[1].addr);
        return 0;
  }
  return 0;
}

/*
6.15.3 Specification of operator ~insertbtree~

*/
const string insertBTreeSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
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
6.15.4 Definition of operator ~insertbtree~

*/
Operator insertbtree (
         "insertbtree",              // name
         insertBTreeSpec,            // specification
         insertBTreeValueMap,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         insertBTreeTypeMap          // type mapping
);


/*
6.16 Operator ~deletebtree~ 

For each tuple of the inputstream deletes the corresponding entry from the btree. The entry is built from the attribute 
of the tuple over which the tree is built and the tuple-identifier of the deleted tuple which is extracted
as the last attribute of the tuple of the inputstream.


6.16.1 TypeMapping of operator ~deletebtree~

*/

ListExpr deleteBTreeTypeMap(ListExpr args){
	return allUpdatesBTreeTypeMap(args, "deletebtree");
}

/*
6.16.1 ValueMapping of operator ~deletebtree~

*/

int deleteBTreeValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, argBTree, attrPos;
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
      qp->Request(args[3].addr,attrPos);
      indexp = ((CcInt*)attrPos.addr);
      local = SetWord(indexp );
      return 0;

    case REQUEST :
      index = ((CcInt*) local.addr)->GetIntval();
      qp->Request(args[1].addr, argBTree);
      btree = (BTree*)(argBTree.addr);    
      assert(btree != 0);
      qp->Request(args[0].addr,t);    
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
       	keyAttr = tup->GetAttribute(index - 1);
       	tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
       	oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
       	AttrToKey((StandardAttribute*)keyAttr, key, btree->GetKeyType());
       	btree->Delete(key,oldTid);
        result = SetWord(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(args[1].addr);
      return 0;
  }
  return 0;
}

/*
6.16.3 Specification of operator ~deletebtree~

*/
const string deleteBTreeSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
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
6.16.4 Definition of operator ~deletebtree~

*/
Operator deletebtree (
         "deletebtree",              // name
         deleteBTreeSpec,            // specification
         deleteBTreeValueMap,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         deleteBTreeTypeMap          // type mapping
);


/*
2.17 Operator ~updatebtree~ 

For each tuple of the inputstream updates the entry in the btree. The entry is built from the attribute 
of the tuple over which the tree is built and the tuple-identifier of the updated tuple which is extracted
as the last attribute of the tuple of the inputstream.

2.17.1 TypeMapping of operator ~updatebtree~

*/

ListExpr updateBTreeTypeMap(ListExpr args){
	return allUpdatesBTreeTypeMap(args, "updatebtree");
}

/*

2.17.2 ValueMapping of operator ~updatebtree~

*/

int updateBTreeValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, argBTree, attrPos;
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
      qp->Request(args[3].addr,attrPos);
      indexp = ((CcInt*)attrPos.addr);
      local = SetWord(indexp );
      return 0;

    case REQUEST :
      index = ((CcInt*) local.addr)->GetIntval();
      qp->Request(args[1].addr, argBTree);
      btree = (BTree*)(argBTree.addr);    
      assert(btree != 0);
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
       	keyAttr = tup->GetAttribute(index - 1);
       	oldKeyAttr = tup->GetAttribute((tup->GetNoAttributes()-1)/2 + index -1);
       	tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
       	oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
       	AttrToKey((StandardAttribute*)keyAttr, key, btree->GetKeyType());
       	AttrToKey((StandardAttribute*)oldKeyAttr, oldKey, btree->GetKeyType());
       	// Only update if key has changed
       	if ((key > oldKey) || (oldKey > key)){
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
      qp->SetModified(args[1].addr);
      return 0;
  }
  return 0;
}

/*
2.17.3 Specification of operator ~updatebtree~

*/
const string updateBTreeSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>stream(tuple(x@[(a1_old x1)...(an_old xn)(TID tid)])) x (btree(tuple(x) ti) xi)"
                           " -> stream(tuple(x@[(a1_old x1)...(an_old xn)(TID tid)]))] "
                           "</text--->"
                           "<text>_ _ updatebtree [_]</text--->"
                           "<text>Updates references to the tuples with TupleId 'tid' "
                           "in the btree.</text--->"
                           "<text>query staedte feed filter [.Name = 'Hargen'] staedte updatedirect [Name: "
                           "'Hagen'] staedte_Name updatebtree [Name] count "
                           "</text--->"
                           ") )";

/*
2.17.4 Definition of operator ~updatebtree~

*/
Operator updatebtree (
         "updatebtree",              // name
         updateBTreeSpec,            // specification
         updateBTreeValueMap,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         updateBTreeTypeMap          // type mapping
);



/*

7 Definition and initialization of btree algebra

*/
class BTreeAlgebra : public Algebra
{
 public:
  BTreeAlgebra() : Algebra()
  {
    AddTypeConstructor( &cppbtree );

    AddOperator(&createbtree);
    AddOperator(&exactmatch);
    AddOperator(&leftrange);
    AddOperator(&rightrange);
    AddOperator(&cpprange);
    AddOperator(&insertbtree);
    AddOperator(&deletebtree);
    AddOperator(&updatebtree);
  }
  ~BTreeAlgebra() {};
};

BTreeAlgebra btreealgebra;

extern "C"
Algebra*
InitializeBTreeAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&btreealgebra);
}
