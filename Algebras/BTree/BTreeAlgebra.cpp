/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of BTree Algebra

[TOC]

2 Auxilary Functions

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

/*

1.3 Macro CHECK\_COND

This macro makes reporting errors in type mapping functions more convenient.

*/
#define CHECK_COND(cond, msg) \
  if(!(cond)) \
  {\
    ErrorReporter::ReportError(msg);\
    return nl->SymbolAtom("typeerror");\
  };

static int simpleSelect (ListExpr args) { return 0; }

ListExpr BTreeProp ()
{
  NestedList* nl = SecondoSystem::GetNestedList();

  return
   nl->TwoElemList(
     nl->TwoElemList(
       nl->SymbolAtom("TUPLE"),
       nl->SymbolAtom("DATA")),
     nl->SymbolAtom("BTREE"));
}


void AttrToKey(
  StandardAttribute* attr,
  SmiKey::KeyDataType keyType,
  SmiKey& key)
{
  float floatval;
  int intval;
  string strval;

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

void KeyToAttr(
  StandardAttribute* attr,
  SmiKey::KeyDataType keyType,
  SmiKey& key)
{
  double floatval;
  long intval;
  string strval;

  //assert(key.GetType() == keyType);
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

bool WriteRecordId(SmiRecord& record, SmiRecordId id)
{
  SmiSize bytesWritten;
  SmiSize idSize = sizeof(SmiRecordId);

  bytesWritten = record.Write(&id, idSize);
  return bytesWritten == idSize;
}

bool ReadRecordId(SmiRecord& record, SmiRecordId& id)
{
  SmiSize bytesRead;
  SmiRecordId ids[2];
  SmiSize idSize = sizeof(SmiRecordId);

  bytesRead = record.Read(ids, 2 * idSize);
  id = ids[0];
  return bytesRead == idSize;
}

class BTreeIterator
{
  SmiKeyedFileIterator* fileIter;
  StandardAttribute* key;
  SmiRecordId id;
  SmiKey::KeyDataType keyType;
  SmiKey smiKey;
  SmiRecord record;

public:
  BTreeIterator(SmiKey::KeyDataType smiKeyType, SmiKeyedFileIterator* iter)
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
    bool received = fileIter->Next(smiKey, record);
    if(received)
    {
      assert(smiKey.GetType() == keyType);
      KeyToAttr(key, keyType, smiKey);
      return ReadRecordId(record, id);
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
        file->Create();
      }
      else
      {
        isTemporary = false;
        this->file = file;
      }
    }
    else
    {
      this->file = 0;
    }
    this->keyType = keyType;
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
      this->file = file;
      this->keyType = keyType;
    }
    else
    {
      this->file = 0;
      this->keyType = SmiKey::Unknown;
    }

  }

  bool WriteTo(SmiRecord& record, SmiKey::KeyDataType keyType)
  {
    assert(file != 0);

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
    file->Create();
    return true;
  }

  void Truncate()
  {
    assert(file != 0);
    SmiKeyedFileIterator iter;
    SmiRecord record;

    file->SelectAll(iter, SmiFile::Update, true);
    while(iter.Next(record))
    {
      file->DeleteRecord(record.GetKey());
    }
  }

  void DeleteDeep()
  {
    assert(file != 0);
    file->Drop();
    delete file;
    file = 0;
    delete this;
  }

  bool Append(StandardAttribute* attr, SmiRecordId id)
  {
    assert(file != 0);
    SmiRecord record;
    SmiKey key;
    bool success;

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
    SmiKeyedFileIterator* iter;

    AttrToKey(key, keyType, smiKey);
    iter = new SmiKeyedFileIterator(true);
    if(!file->SelectRange(smiKey, smiKey, *iter, SmiFile::ReadOnly, true))
    {
      delete iter;
      return 0;
    }

    return new BTreeIterator(keyType, iter);
  }

  BTreeIterator* LeftRange(StandardAttribute* key)
  {
    SmiKey smiKey;
    SmiKeyedFileIterator* iter;

    AttrToKey(key, keyType, smiKey);
    iter = new SmiKeyedFileIterator(true);
    if(!file->SelectLeftRange(smiKey, *iter, SmiFile::ReadOnly, true))
    {
      delete iter;
      return 0;
    }

    return new BTreeIterator(keyType, iter);
  }

  BTreeIterator* RightRange(StandardAttribute* key)
  {
    SmiKey smiKey;
    SmiKeyedFileIterator* iter;

    AttrToKey(key, keyType, smiKey);
    iter = new SmiKeyedFileIterator(true);
    if(!file->SelectRightRange(smiKey, *iter, SmiFile::ReadOnly, true))
    {
      delete iter;
      return 0;
    }

    return new BTreeIterator(keyType, iter);
  }

  BTreeIterator* Range(StandardAttribute* left, StandardAttribute* right)
  {
    SmiKey leftSmiKey;
    SmiKey rightSmiKey;
    SmiKeyedFileIterator* iter;

    AttrToKey(left, keyType, leftSmiKey);
    AttrToKey(right, keyType, rightSmiKey);
    iter = new SmiKeyedFileIterator(true);
    if(!file->SelectRange(leftSmiKey, rightSmiKey, *iter, SmiFile::ReadOnly, true))
    {
      delete iter;
      return 0;
    }

    return new BTreeIterator(keyType, iter);
  }

  BTreeIterator* SelectAll()
  {
    SmiKeyedFileIterator* iter;

    iter = new SmiKeyedFileIterator(true);
    if(!file->SelectAll(*iter, SmiFile::ReadOnly, true))
    {
      delete iter;
      return 0;
    }

    return new BTreeIterator(keyType, iter);
  }
};

ListExpr OutBTree(ListExpr typeInfo, Word  value)
{
  NestedList* nl = SecondoSystem::GetNestedList();
  return nl->TheEmptyList();
}

Word CreateBTree(int Size)
{
  return SetWord(new BTree());
}

Word InBTree(ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct)
{
  assert(false /* reading a btree from a list expression
    does not make sense currently */);
  return SetWord(0);
}

void DeleteBTree(Word& w)
{
  BTree* btree = (BTree*)w.addr;
  delete btree;
}

/*

4.3.8 ~Check~-function of type constructor ~rel~

Checks the specification:

----    TUPLE   -> REL          rel
----

Hence the type expression must have the form

----    (rel x)
----

and ~x~ must be a type of kind TUPLE.

*/
bool CheckBTree(ListExpr type, ListExpr& errorInfo)
{
  /*
  AlgebraManager* algMgr;

  if ((nl->ListLength(type) == 2) && nl->IsEqual(nl->First(type), "rel"))
  {
    algMgr = SecondoSystem::GetAlgebraManager();
    return (algMgr->CheckKind("TUPLE", nl->Second(type), errorInfo));
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("REL"), type));
    return false;
  }
  */
  return true;
}

void* CastBTree(void* addr)
{
  return ( 0 );
}
/*

3.2.5 ~PersistFunction~ of type constructor ~rel~

This is a slightly modified version of the function ~DefaultPersistValue~ (from
~Algebra~) which creates the relation from the SmiRecord only if it does not
yet exist.

The idea is to maintain a cache containing the relation representations that
have been built in memory. The cache basically stores pairs (recordId, relation
value). If the record Id passed to this function is found, the cached relation
value is returned instead of building a new one.

*/
bool
BTreePersistValue( const PersistDirection dir,
    SmiRecord& valueRecord,
    const ListExpr typeInfo,
    Word& value )
{
  NestedList* nl = SecondoSystem::GetNestedList();
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


  //nl->WriteListExpr(typeInfo, cerr);
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
  //cerr << "key type : " << keyTypeString << endl;

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

  if(dir == ReadFrom)
  {
    btree = new BTree(valueRecord, keyType);
    value = SetWord(btree);
    return btree != 0;
  }
  else
  {
    btree = (BTree*)value.addr;
    success = btree->WriteTo(valueRecord, keyType);
    return success;
  }
}
/*

3.2.5 ~Model~-functions of type constructor ~btree~

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

TypeConstructor cppbtree( "btree",           BTreeProp,
                        OutBTree,          InBTree,   CreateBTree,
                        DeleteBTree,       CastBTree,   CheckBTree,
			BTreePersistValue, 0,
			BTreeInModel,      BTreeOutModel,
			BTreeValueToModel, BTreeValueListToModel );

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

  //nl->WriteListExpr(dbgList, cout);
  return resultType;
}
/*

4.1.2 Value mapping function of operator ~feed~

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
  iter = relation->MakeNewScan();
  while(!iter->EndOfScan())
  {
    tuple = iter->GetTuple();
    iter->Next();

    btree->Append((StandardAttribute*)tuple->Get(attrIndex), tuple->GetId());
    tuple->DeleteIfAllowed();
  }

  delete iter;
  return 0;
}
/*

4.1.3 Specification of operator ~feed~

*/
const string CreateBTreeSpec =
  "(<text>(rel x) -> (stream x)</text---><text>Produces a stream from a "
  "relation by scanning the relation tuple by tuple.</text--->)";
/*

4.1.3 Definition of operator ~feed~

Non-overloaded operators are defined by constructing a new instance of
class ~Operator~, passing all operator functions as constructor arguments.

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

template<int operatorId>
ListExpr IndexQueryTypeMap(ListExpr args)
{
  char* errmsg = errorMessages[operatorId];
  NestedList* nl = SecondoSystem::GetNestedList();
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

  nl->WriteListExpr(resultType, cout);
  return resultType;
}

template<int operatorId>
static int
IndexQuery(Word* args, Word& result, int message, Word& local, Supplier s)
{
  QueryProcessor* qp = SecondoSystem::GetQueryProcessor();

  BTree* btree;
  CcRel* relation;
  StandardAttribute* key;
  StandardAttribute* secondKey;
  BTreeIterator* iter;
  CcTuple* tuple;
  SmiRecordId id;

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

      btree = (BTree*)btreeWord.addr;
      relation = (CcRel*)relWord.addr;
      key = (StandardAttribute*)keyWord.addr;
      if(operatorId == RANGE)
      {
        secondKey = (StandardAttribute*)secondKeyWord.addr;
        assert(secondKey != 0);
      }

      assert(btree != 0);
      assert(relation != 0);
      assert(key != 0);

      switch(operatorId)
      {
        case EXACTMATCH:
          iter = btree->ExactMatch(key);
          break;
        case RANGE:
          iter = btree->Range(key, secondKey);
          break;
        case LEFTRANGE:
          iter = btree->LeftRange(key);
          break;
        case RIGHTRANGE:
          iter = btree->RightRange(key);
          break;
        default:
          assert(false);
      }

      if(iter == 0)
      {
        return 1;
      }
      local = SetWord(iter);
      return 0;

    case REQUEST :
      iter = (BTreeIterator*)local.addr;
      assert(iter != 0);

      if(iter->Next())
      {
        id = iter->GetId();
        tuple = relation->GetTupleById(id);
        result = SetWord(tuple);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }

    case CLOSE :
      iter = (BTreeIterator*)local.addr;
      delete iter;
      return 0;
  }
  return 0;
}

const string ExactMatchSpec =
  "(<text>(stream x) -> (rel x)</text---><text>Collects objects from a stream "
  "into a relation.</text--->)";

Operator exactmatch (
         "exactmatch",            // name
	 ExactMatchSpec,          // specification
	 IndexQuery<EXACTMATCH>,              // value mapping
	 Operator::DummyModel, // dummy model mapping, defines in Algebra.h
	 simpleSelect,         // trivial selection function
	 IndexQueryTypeMap<EXACTMATCH>        // type mapping
);

const string RangeSpec =
  "(<text>(stream x) -> (rel x)</text---><text>Collects objects from a stream "
  "into a relation.</text--->)";

Operator cpprange (
         "range",            // name
	 RangeSpec,          // specification
	 IndexQuery<RANGE>,              // value mapping
	 Operator::DummyModel, // dummy model mapping, defines in Algebra.h
	 simpleSelect,         // trivial selection function
	 IndexQueryTypeMap<RANGE>        // type mapping
);

const string LeftRangeSpec =
  "(<text>(stream x) -> (rel x)</text---><text>Collects objects from a stream "
  "into a relation.</text--->)";

Operator leftrange (
         "leftrange",            // name
	 LeftRangeSpec,          // specification
	 IndexQuery<LEFTRANGE>,              // value mapping
	 Operator::DummyModel, // dummy model mapping, defines in Algebra.h
	 simpleSelect,         // trivial selection function
	 IndexQueryTypeMap<LEFTRANGE>        // type mapping
);

const string RightRangeSpec =
  "(<text>(stream x) -> (rel x)</text---><text>Collects objects from a stream "
  "into a relation.</text--->)";

Operator rightrange (
         "rightrange",            // name
	 RightRangeSpec,          // specification
	 IndexQuery<RIGHTRANGE>,              // value mapping
	 Operator::DummyModel, // dummy model mapping, defines in Algebra.h
	 simpleSelect,         // trivial selection function
	 IndexQueryTypeMap<RIGHTRANGE>        // type mapping
);

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
  return (&btreealgebra);
}
