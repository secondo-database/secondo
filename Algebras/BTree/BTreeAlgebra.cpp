/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of BTree Algebra

[TOC]

2 Auxiliary Functions

*/

#include "Algebra.h"
#include "AlgebraManager.h"
#include "SecondoSystem.h"
#include "SecondoCatalog.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "Tuple.h"

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

namespace {

static NestedList* nl;

/* 

Commenting out the following line prevents the usage
of prefetching iterators in the btree algebra.

*/
#define BTREE_PREFETCH

/*

2.1 Macro CHECK\_COND

This macro makes reporting errors in type mapping functions more convenient.

*/
#define CHECK_COND(cond, msg) \
  if(!(cond)) \
  {\
    ErrorReporter::ReportError(msg);\
    return nl->SymbolAtom("typeerror");\
  };

/*

2.2 Function ~simpleSelect~

Trivial selection function.

*/
static int simpleSelect (ListExpr args) { return 0; }

/*

2.2 Type property of type constructor ~btree~

*/
ListExpr BTreeProp ()
{

  return
   nl->TwoElemList(
     nl->TwoElemList(
       nl->SymbolAtom("TUPLE"),
       nl->SymbolAtom("DATA")),
     nl->SymbolAtom("BTREE"));
}

/*

2.3 Function ~AttrToKey~

Converts a ~StandardAttribute~ to a ~SmiKey~.

*/
void AttrToKey(
  StandardAttribute* attr,
  SmiKey::KeyDataType keyType,
  SmiKey& key)
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

    default:
      assert(false /* should not reach this */);
      break;
  }
  assert(key.GetType() == keyType);
}

/*

2.4 Function ~KeyToAttr~

Converts a ~SmiKey~ to a ~StandardAttribute~.

*/
void KeyToAttr(
  StandardAttribute* attr,
  SmiKey::KeyDataType keyType,
  SmiKey& key)
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

    default:
      assert(false /* should not reach this */);
      break;
  }
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
class BTreeIterator
{
  BTreeFileIteratorT* fileIter;
  StandardAttribute* key;
  SmiRecordId id;
  SmiKey::KeyDataType keyType;
  SmiKey smiKey;
  SmiRecord record;

public:
  BTreeIterator(SmiKey::KeyDataType smiKeyType, BTreeFileIteratorT* iter)
    : fileIter(iter), keyType(smiKeyType)
  {
    assert(
      keyType == SmiKey::Integer
      || keyType == SmiKey::String
      || keyType == SmiKey::Float);

    switch(keyType)
    {
      case SmiKey::Integer:
        key = new CcInt();
        break;

      case SmiKey::Float:
        key = new CcReal();
        break;

      case SmiKey::String:
        key = new CcString();
        break;

      default:
        assert(false /* should not reach this */);
        break;
    }
  }

  ~BTreeIterator()
  {
    delete key;
    delete fileIter;
  }

  bool Next()
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

      assert(smiKey.GetType() == keyType);
      KeyToAttr(key, keyType, smiKey);

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

  StandardAttribute* GetKey()
  {
    return key;
  }

  SmiRecordId GetId()
  {
    return id;
  }
};

/*

4 Class ~BTree~

The key attribute of a btree can be an ~int~, a ~string~, or a ~real~.

*/
class BTree
{
private:
  SmiRecordId id;
  bool isTemporary;
  SmiKey::KeyDataType keyType;
  SmiKeyedFile* file;

public:
  BTree(SmiKey::KeyDataType keyType = SmiKey::Unknown, SmiKeyedFile* file = 0)
  {
    if(keyType != SmiKey::Unknown)
    {
      if(file == 0)
      {
        isTemporary = true;
        file = new SmiKeyedFile(keyType, false);
        if(!file->Create())
        {
          delete file;
          this->file = 0;
          this->keyType = SmiKey::Unknown;
        }
      }
      else
      {
        isTemporary = false;
        this->file = file;
        this->keyType = keyType;
      }
    }
    else
    {
      this->file = 0;
      this->keyType = keyType;
    }
    
  }

  ~BTree()
  {
    if(file != 0)
    {
      if(isTemporary)
      {
        file->Drop();
      }
      else
      {
        file->Close();
      }
    }
    delete file;
  }

  BTree(SmiRecord& record, SmiKey::KeyDataType keyType)
  {
    SmiSize bytesRead;
    SmiFileId fileId[2];
    SmiSize buflen = sizeof(SmiFileId);
    SmiKeyedFile* file = 0;
    bool success;

    bytesRead = record.Read((void*)fileId, 2 * buflen);
    if(bytesRead == buflen)
    {
      file = new SmiKeyedFile(keyType);
      success = file->Open(fileId[0]);
      if(success)
      {
        this->file = file;
        this->keyType = keyType;
      }
      else
      {
        delete file;
        this->file = 0;
        this->keyType = SmiKey::Unknown;
      }
    }
    else
    {
      this->file = 0;
      this->keyType = SmiKey::Unknown;
    }
  }

  bool IsInitialized()
  {
    return file != 0 && keyType != SmiKey::Unknown;
  }
  
  bool WriteTo(SmiRecord& record, SmiKey::KeyDataType keyType)
  {
    assert(file != 0);
    assert(this->keyType != SmiKey::Unknown);

    SmiSize bytesWritten;
    SmiFileId id;
    SmiSize fileIdLength = sizeof(SmiFileId);

    id = file->GetFileId();
    bytesWritten = record.Write(&id, fileIdLength);
    if(bytesWritten == fileIdLength)
    {
      isTemporary = false;
      return true;
    }
    return false;
  }

  bool SetTypeAndCreate(SmiKey::KeyDataType keyType)
  {
    assert(
      keyType == SmiKey::Integer
      || keyType == SmiKey::String
      || keyType == SmiKey::Float);
    assert(this->keyType == SmiKey::Unknown);
    assert(file == 0);

    this->keyType = keyType;
    isTemporary = true;
    file = new SmiKeyedFile(keyType, false);
    if(!file->Create())
    {
      delete file;
      file = 0;
      this->keyType = SmiKey::Unknown;
      return false;
    }
    return true;
  }

  bool Truncate()
  {
    assert(file != 0);
    SmiKeyedFileIterator iter;
    SmiRecord record;

    if(!file->SelectAll(iter, SmiFile::Update, true))
    {
      return false;
    }
    while(iter.Next(record))
    { 
      if(!file->DeleteRecord(record.GetKey()))
      {
        return false;
      }
    }
    return true;
  }

  void DeleteDeep()
  {
    assert(file != 0);
    file->Drop();
    delete file;
    file = 0;
    delete this;
  }

  void DeleteFile()
  {
    if ( file != 0 )
    {
      file->Close();
      file->Drop();
      delete file;
      file = 0;
    }
  }

  bool Append(StandardAttribute* attr, SmiRecordId id)
  {
    assert(file != 0);
    SmiRecord record;
    SmiKey key;
    bool success;

    if(!attr->IsDefined())
    {
      return false;
    }

    AttrToKey(attr, keyType, key);
    if(file->InsertRecord(key, record))
    {
      success = WriteRecordId(record, id);
      if(!success)
      {
        file->DeleteRecord(key);
      }
      return success;
    }
    return false;
  }

  SmiKeyedFile* GetFile()
  {
    return this->file;
  }

  BTreeIterator* ExactMatch(StandardAttribute* key)
  {
    SmiKey smiKey;
    BTreeFileIteratorT* iter;

    AttrToKey(key, keyType, smiKey);

#ifdef BTREE_PREFETCH
    iter = file->SelectRangePrefetched(smiKey, smiKey);
    if(iter == 0)
    {
      return 0;
    }
#else
    iter = new SmiKeyedFileIterator(true);
    if(!file->SelectRange(smiKey, smiKey, *iter, SmiFile::ReadOnly, true))
    {
      delete iter;
      return 0;
    }
#endif /* BTREE_PREFETCH */

    return new BTreeIterator(keyType, iter);
  }

  BTreeIterator* LeftRange(StandardAttribute* key)
  {
    SmiKey smiKey;
    BTreeFileIteratorT* iter;

    AttrToKey(key, keyType, smiKey);

#ifdef BTREE_PREFETCH
    iter = file->SelectLeftRangePrefetched(smiKey);
    if(iter == 0)
    {
      return 0;
    }
#else
    iter = new SmiKeyedFileIterator(true);
    if(!file->SelectLeftRange(smiKey, *iter, SmiFile::ReadOnly, true))
    {
      delete iter;
      return 0;
    }
#endif /* BTREE_PREFETCH */

    return new BTreeIterator(keyType, iter);
  }

  BTreeIterator* RightRange(StandardAttribute* key)
  {
    SmiKey smiKey;
    BTreeFileIteratorT* iter;

    AttrToKey(key, keyType, smiKey);

#ifdef BTREE_PREFETCH
    iter = file->SelectRightRangePrefetched(smiKey);
    if(iter == 0)
    {
      return 0;
    }
#else
    iter = new SmiKeyedFileIterator(true);
    if(!file->SelectRightRange(smiKey, *iter, SmiFile::ReadOnly, true))
    {
      delete iter;
      return 0;
    }
#endif /* BTREE_PREFETCH */

    return new BTreeIterator(keyType, iter);
  }

  BTreeIterator* Range(StandardAttribute* left, StandardAttribute* right)
  {
    SmiKey leftSmiKey;
    SmiKey rightSmiKey;
    BTreeFileIteratorT* iter;

    AttrToKey(left, keyType, leftSmiKey);
    AttrToKey(right, keyType, rightSmiKey);

#ifdef BTREE_PREFETCH
    iter = file->SelectRangePrefetched(leftSmiKey, rightSmiKey);
    if(iter == 0)
    {
      return 0;
    }
#else
    iter = new SmiKeyedFileIterator(true);
    if(!file->SelectRange(leftSmiKey, rightSmiKey, 
      *iter, SmiFile::ReadOnly, true))
    {
      delete iter;
      return 0;
    }
#endif /* BTREE_PREFETCH */

    return new BTreeIterator(keyType, iter);
  }

  BTreeIterator* SelectAll()
  {
    BTreeFileIteratorT* iter;

#ifdef BTREE_PREFETCH
    iter = file->SelectAllPrefetched();
    if(iter == 0)
    {
      return 0;
    }
#else
    iter = new SmiKeyedFileIterator(true);
    if(!file->SelectAll(*iter, SmiFile::ReadOnly, true))
    {
      delete iter;
      return 0;
    }
#endif /* BTREE_PREFETCH */

    return new BTreeIterator(keyType, iter);
  }
};

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

/*

5.2 Function ~CreateBTree~

*/
Word CreateBTree(const ListExpr typeInfo)
{
  return SetWord(new BTree());
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
  assert(false);
  return SetWord(0);
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
  return SetWord( Address(0) );
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
  AlgebraManager* algMgr;

  if((!nl->IsAtom(type))
    && (nl->ListLength(type) == 3)
    && nl->Equal(nl->First(type), nl->SymbolAtom("rel")))
  {
    algMgr = SecondoSystem::GetAlgebraManager();
    return
      algMgr->CheckKind("TUPLE", nl->Second(type), errorInfo)
      && algMgr->CheckKind("DATA", nl->Third(type), errorInfo);
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("REL"), type));
    return false;
  }
  return true;
}

void* CastBTree(void* addr)
{
  return ( 0 );
}

/*

5.8 ~OpenFunction~ of type constructor ~btree~

*/
bool
OpenBTree( SmiRecord& valueRecord,
           const ListExpr typeInfo,
           Word& value )
{
  AlgebraManager* alg = SecondoSystem::GetAlgebraManager();
  BTree* btree;

  ListExpr first;
  SmiKey::KeyDataType keyType;
  ListExpr keyTypeLE;
  ListExpr algNoLE;
  ListExpr typeNoLE;
  int algNumber;
  int typeNumber;
  string keyTypeString;

  /* find out key type */
  assert(!nl->IsAtom(typeInfo));
  assert(!nl->IsEmpty(typeInfo));
  assert(nl->ListLength(typeInfo) == 1);

  first = nl->First(typeInfo);

  assert(!nl->IsAtom(first));
  assert(!nl->IsEmpty(first));
  assert(nl->ListLength(first) == 3);

  keyTypeLE = nl->Third(first);
  assert(!nl->IsAtom(keyTypeLE));
  assert(!nl->IsEmpty(keyTypeLE));
  assert(nl->ListLength(keyTypeLE) == 2);
  algNoLE = nl->First(keyTypeLE);
  typeNoLE = nl->Second(keyTypeLE);

  assert(nl->IsAtom(algNoLE));
  assert(nl->IsAtom(typeNoLE));
  assert(nl->AtomType(algNoLE) == IntType);
  assert(nl->AtomType(typeNoLE) == IntType);

  algNumber = nl->IntValue(algNoLE);
  typeNumber = nl->IntValue(typeNoLE);

  keyTypeString = alg->Constrs(algNumber, typeNumber);

  if(keyTypeString == "int")
  {
    keyType = SmiKey::Integer;
  }
  else if(keyTypeString == "string")
  {
    keyType = SmiKey::String;
  }
  else if(keyTypeString == "real")
  {
    keyType = SmiKey::Float;
  }
  else
  {
    assert(false /* no proper key type given */);
  }

  btree = new BTree(valueRecord, keyType);
  if(btree->IsInitialized())
  {
    value = SetWord(btree);
    return true;
  }
  else
  {
    delete btree;
    return false;
  }
}

/*

5.9 ~SaveFunction~ of type constructor ~btree~

*/
bool
SaveBTree( SmiRecord& valueRecord,
           const ListExpr typeInfo,
           Word& value )
{
  AlgebraManager* alg = SecondoSystem::GetAlgebraManager();

  bool success;
  BTree* btree;

  ListExpr first;
  SmiKey::KeyDataType keyType;
  ListExpr keyTypeLE;
  ListExpr algNoLE;
  ListExpr typeNoLE;
  int algNumber;
  int typeNumber;
  string keyTypeString;

  /* find out key type */
  assert(!nl->IsAtom(typeInfo));
  assert(!nl->IsEmpty(typeInfo));
  assert(nl->ListLength(typeInfo) == 1);

  first = nl->First(typeInfo);

  assert(!nl->IsAtom(first));
  assert(!nl->IsEmpty(first));
  assert(nl->ListLength(first) == 3);

  keyTypeLE = nl->Third(first);
  assert(!nl->IsAtom(keyTypeLE));
  assert(!nl->IsEmpty(keyTypeLE));
  assert(nl->ListLength(keyTypeLE) == 2);
  algNoLE = nl->First(keyTypeLE);
  typeNoLE = nl->Second(keyTypeLE);

  assert(nl->IsAtom(algNoLE));
  assert(nl->IsAtom(typeNoLE));
  assert(nl->AtomType(algNoLE) == IntType);
  assert(nl->AtomType(typeNoLE) == IntType);

  algNumber = nl->IntValue(algNoLE);
  typeNumber = nl->IntValue(typeNoLE);

  keyTypeString = alg->Constrs(algNumber, typeNumber);

  if(keyTypeString == "int")
  {
    keyType = SmiKey::Integer;
  }
  else if(keyTypeString == "string")
  {
    keyType = SmiKey::String;
  }
  else if(keyTypeString == "real")
  {
    keyType = SmiKey::Float;
  }
  else
  {
    assert(false /* no proper key type given */);
  }

  btree = (BTree*)value.addr;
  success = btree->WriteTo(valueRecord, keyType);
  return success;
}

/*

5.10 ~Model~-functions of type constructor ~btree~

*/
Word BTreeInModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  return (SetWord( Address( 0 ) ));
}

ListExpr BTreeOutModel( ListExpr typeExpr, Word model )
{
  return (0);
}

Word BTreeValueToModel( ListExpr typeExpr, Word value )
{
  return (SetWord( Address( 0 ) ));
}

Word BTreeValueListToModel( const ListExpr typeExpr, const ListExpr valueList,
                       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  return (SetWord( Address( 0 ) ));
}

/*

5.11 Type Constructor object for type constructor ~btree~

*/
TypeConstructor cppbtree( "btree",		BTreeProp,
                          OutBTree,		InBTree,   
                          CreateBTree,		DeleteBTree,
			  OpenBTree,		SaveBTree,
			  CloseBTree,		CloneBTree,
			  CastBTree,   		CheckBTree,
			  0,
			  BTreeInModel,		BTreeOutModel,
			  BTreeValueToModel,	BTreeValueListToModel );

/*

6 Operators of the btree algebra

6.1 Type Mapping of operator ~createbtree~

*/
static ListExpr CreateBTreeTypeMap(ListExpr args)
{
  string attrName;
  char* errmsg = "Incorrect input for operator createbtree.";
  NestedList* nl = SecondoSystem::GetNestedList();
  int attrIndex;
  ListExpr attrType;

  CHECK_COND(!nl->IsEmpty(args), errmsg);
  CHECK_COND(nl->ListLength(args) == 2, errmsg);

  ListExpr relDescription = nl->First(args);
  ListExpr attrNameLE = nl->Second(args);

  CHECK_COND(nl->IsAtom(attrNameLE), errmsg);
  CHECK_COND(nl->AtomType(attrNameLE) == SymbolType, errmsg);
  attrName = nl->SymbolValue(attrNameLE);

  CHECK_COND(!nl->IsEmpty(relDescription), errmsg);
  CHECK_COND(nl->ListLength(relDescription) == 2, errmsg);

  ListExpr relSymbol = nl->First(relDescription);;
  ListExpr tupleDescription = nl->Second(relDescription);

  CHECK_COND(nl->IsAtom(relSymbol), errmsg);
  CHECK_COND(nl->AtomType(relSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(relSymbol) == "rel", errmsg);

  CHECK_COND(nl->ListLength(tupleDescription) == 2, errmsg);
  ListExpr tupleSymbol = nl->First(tupleDescription);;
  ListExpr attrList = nl->Second(tupleDescription);

  CHECK_COND(nl->IsAtom(tupleSymbol), errmsg);
  CHECK_COND(nl->AtomType(tupleSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(tupleSymbol) == "tuple", errmsg);
  CHECK_COND(IsTupleDescription(attrList, nl), errmsg);
  CHECK_COND((attrIndex = findattr(attrList, attrName, attrType, nl)) > 0, errmsg);

  assert
    (nl->SymbolValue(attrType) == "string"
    || nl->SymbolValue(attrType) == "int"
    || nl->SymbolValue(attrType) == "real");


  ListExpr resultType =
    nl->ThreeElemList(
      nl->SymbolAtom("APPEND"),
      nl->TwoElemList(
        nl->IntAtom(attrIndex),
        nl->StringAtom(nl->SymbolValue(attrType))),
      nl->ThreeElemList(
        nl->SymbolAtom("btree"),
        tupleDescription,
        attrType));

  return resultType;
}

/*

6.2 Value mapping function of operator ~createbtree~

*/
static int
CreateBTreeValueMapping(Word* args, Word& result, int message, Word& local, Supplier s)
{
  QueryProcessor* qp = SecondoSystem::GetQueryProcessor();
  result = qp->ResultStorage(s);

  CcRel* relation;
  BTree* btree;
  CcInt* attrIndexCcInt;
  int attrIndex;
  CcString* attrTypeStr;
  CcRelIT* iter;
  CcTuple* tuple;
  SmiKey::KeyDataType dataType;
  bool appendHasNotWorked = false;

  btree = (BTree*)qp->ResultStorage(s).addr;
  relation = (CcRel*)args[0].addr;
  attrIndexCcInt = (CcInt*)args[2].addr;
  attrTypeStr = (CcString*)args[3].addr;

  assert(btree != 0);
  assert(relation != 0);
  assert(attrIndexCcInt != 0);
  assert(attrTypeStr != 0);

  attrIndex = attrIndexCcInt->GetIntval() - 1;
  char* attrType = (char*)attrTypeStr->GetStringval();
  if(strcmp(attrType, "int") == 0)
  {
    dataType = SmiKey::Integer;
  }
  else if(strcmp(attrType, "string") == 0)
  {
    dataType = SmiKey::String;
  }
  else if(strcmp(attrType, "real") == 0)
  {
    dataType = SmiKey::Float;
  }
  else
  {
    assert(false /* this should not happen */);
  }
  btree->SetTypeAndCreate(dataType);
  if(!btree->IsInitialized())
  {
    return -1;
  }
  
  iter = relation->MakeNewScan();
  while(!iter->EndOfScan())
  {
    tuple = iter->GetTuple();
    iter->Next();

    appendHasNotWorked = appendHasNotWorked ||
      !btree->Append((StandardAttribute*)tuple->Get(attrIndex), tuple->GetId());
    tuple->DeleteIfAllowed();
  }

  if(appendHasNotWorked)
  {
    cerr << "Warning, not all tuples could be inserted into btree." << endl;
  }
  
  delete iter;
  return 0;
}
/*

6.3 Specification of operator ~createbtree~

*/
const string CreateBTreeSpec =
"(<text>((rel (tuple ((x1 t1)...(xn tn)))) xi) -> "
"(btree (tuple ((x1 t1)...(xn tn))) ti)</text--->"
"<text>Creates a btree. The key type ti must be "
"either string or int or real.</text--->)";

/*

6.4 Definition of operator ~createbtree~

*/
Operator createbtree (
          "createbtree",                // name
          CreateBTreeSpec,              // specification
          CreateBTreeValueMapping,                  // value mapping
          Operator::DummyModel, // dummy model mapping, defines in Algebra.h
          simpleSelect,         // trivial selection function
          CreateBTreeTypeMap           // type mapping
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

  ListExpr secondKeyDescription;
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
  CHECK_COND(IsTupleDescription(btreeAttrList, nl), errmsg);

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
  CHECK_COND(IsTupleDescription(attrList, nl), errmsg);

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
  CcRel* relation;
  BTreeIterator* iter;
};

/*

6.6 Value mapping function of operators ~range~, ~leftrange~,
~rightrange~ and ~exactmatch~

*/
template<int operatorId>
static int
IndexQuery(Word* args, Word& result, int message, Word& local, Supplier s)
{
  QueryProcessor* qp = SecondoSystem::GetQueryProcessor();

  BTree* btree;
  StandardAttribute* key;
  StandardAttribute* secondKey;
  CcTuple* tuple;
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
      localInfo->relation = (CcRel*)relWord.addr;
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
        tuple = localInfo->relation->GetTupleById(id);
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
const string ExactMatchSpec =
"(<text>((btree (tuple ((x1 t1)...(xn tn))) ti)"
" (rel (tuple ((x1 t1)...(xn tn)))) ti) -> "
"(stream (tuple ((x1 t1)...(xn tn))))</text--->"
"<text>Uses the given btree to find all tuples in the given "
"relation with .xi = argument value.</text--->)";

/*

6.8 Definition of operator ~exactmatch~

*/
Operator exactmatch (
         "exactmatch",            // name
	 ExactMatchSpec,          // specification
	 IndexQuery<EXACTMATCH>,              // value mapping
	 Operator::DummyModel, // dummy model mapping, defines in Algebra.h
	 simpleSelect,         // trivial selection function
	 IndexQueryTypeMap<EXACTMATCH>        // type mapping
);

/*

6.9 Specification of operator ~range~

*/
const string RangeSpec =
"(<text>((btree (tuple ((x1 t1)...(xn tn))) ti)"
" (rel (tuple ((x1 t1)...(xn tn)))) ti ti) -> "
"(stream (tuple ((x1 t1)...(xn tn))))</text--->"
"<text>Uses the given btree to find all tuples in the given "
"relation with .xi >= argument value 1 and .xi <= argument value 2.</text--->)";

/*

6.10 Definition of operator ~range~

*/
Operator cpprange (
         "range",            // name
	 RangeSpec,          // specification
	 IndexQuery<RANGE>,              // value mapping
	 Operator::DummyModel, // dummy model mapping, defines in Algebra.h
	 simpleSelect,         // trivial selection function
	 IndexQueryTypeMap<RANGE>        // type mapping
);

/*

6.11 Specification of operator ~leftrange~

*/
const string LeftRangeSpec =
"(<text>((btree (tuple ((x1 t1)...(xn tn))) ti)"
" (rel (tuple ((x1 t1)...(xn tn)))) ti) -> "
"(stream (tuple ((x1 t1)...(xn tn))))</text--->"
"<text>Uses the given btree to find all tuples in the given "
"relation with .xi <= argument value.</text--->)";

/*

6.12 Definition of operator ~leftrange~

*/
Operator leftrange (
         "leftrange",            // name
	 LeftRangeSpec,          // specification
	 IndexQuery<LEFTRANGE>,              // value mapping
	 Operator::DummyModel, // dummy model mapping, defines in Algebra.h
	 simpleSelect,         // trivial selection function
	 IndexQueryTypeMap<LEFTRANGE>        // type mapping
);

/*

6.13 Specification of operator ~rightrange~

*/
const string RightRangeSpec =
"(<text>((btree (tuple ((x1 t1)...(xn tn))) ti)"
" (rel (tuple ((x1 t1)...(xn tn)))) ti) -> "
"(stream (tuple ((x1 t1)...(xn tn))))</text--->"
"<text>Uses the given btree to find all tuples in the given "
"relation with .xi >= argument value.</text--->)";

/*

6.14 Definition of operator ~rightrange~

*/
Operator rightrange (
         "rightrange",            // name
	 RightRangeSpec,          // specification
	 IndexQuery<RIGHTRANGE>,              // value mapping
	 Operator::DummyModel, // dummy model mapping, defines in Algebra.h
	 simpleSelect,         // trivial selection function
	 IndexQueryTypeMap<RIGHTRANGE>        // type mapping
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
  }
  ~BTreeAlgebra() {};
};

BTreeAlgebra btreealgebra;


extern "C"
Algebra*
InitializeBTreeAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  return (&btreealgebra);
}

}
