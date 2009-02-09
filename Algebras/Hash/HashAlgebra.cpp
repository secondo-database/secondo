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

[1] Implementation of Hash Algebra

[TOC]

1 Includes and Defines

*/
using namespace std;

#include "Algebra.h"
#include "SecondoSystem.h"
#include "SecondoCatalog.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "HashAlgebra.h"
#include "DateTime.h"
#include "TupleIdentifier.h"
#include "Progress.h"
#include "../BTree/BTreeAlgebra.h"

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
extern AlgebraManager *am;


/*

3 Class ~HashIterator~

Used to iterate over all record ids fulfilling a certain condition.

*/
HashIterator::HashIterator(SmiKeyedFileIterator* iter)
  : fileIter(iter)
{
}

HashIterator::~HashIterator()
{
  delete fileIter;
}

bool HashIterator::Next()
{
  bool received = fileIter->Next(smiKey, record);

  if(received)
    return ReadRecordId(record, id);
  else
    return false;
}

const SmiKey *HashIterator::GetKey() const
{
  return &smiKey;
}

SmiRecordId HashIterator::GetId() const
{
  return id;
}

/*

4 Class ~Hash~

The key attribute of a hash can be an ~int~, a ~string~, ~real~, or a 
composite one.

*/
Hash::Hash( SmiKey::KeyDataType keyType, bool temporary ):
temporary( temporary ),
file( 0 ),
opened( false )
{
  if( keyType != SmiKey::Unknown )
  {
    file = new SmiHashFile( keyType, false, temporary );
    if( file->Create() )
      opened = true;
    else
    {
      delete file; file = 0;
    }
  }
}

Hash::Hash( SmiKey::KeyDataType keyType, SmiRecord& record,
              size_t& offset):
temporary( false ),
file( 0 ),
opened( false )
{
  SmiFileId fileId;
  if( record.Read( &fileId, sizeof(SmiFileId), offset ) != sizeof(SmiFileId) )
    return;
    
  this->file = new SmiHashFile( keyType, false, false );
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

Hash::Hash( SmiKey::KeyDataType keyType, SmiFileId fileId ):
temporary( true ),
file( 0 ),
opened( false )
{
  if( keyType != SmiKey::Unknown )
  {
    file = new SmiHashFile( keyType, false, false );
    if( file->Open( fileId ) )
      opened = true;
    else
    {
      delete file; file = 0;
    }
  }  
}

Hash::~Hash()
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

bool Hash::Truncate()
{
  if( opened )
    return file->Truncate();
  return true;
}

void Hash::DeleteFile()
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

bool Hash::Append( const SmiKey& smiKey, SmiRecordId id )
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

bool Hash::Delete( const SmiKey& smiKey, const SmiRecordId id )
{
  if( opened )
  {
    assert(file != 0);
    return file->DeleteRecord( smiKey, false, id );
  }
  return false;
}

SmiHashFile* Hash::GetFile() const
{
  return this->file;
}

SmiFileId Hash::GetFileId() const
{
  return this->file->GetFileId();
}

SmiKey::KeyDataType Hash::GetKeyType() const
{
  return this->file->GetKeyType();
}

HashIterator* Hash::ExactMatch( StandardAttribute* key )
{
  if( !opened )
    return 0;

  assert( file != 0 );

  SmiKey smiKey;
  SmiKeyedFileIterator* iter;

  AttrToKey( key, smiKey, file->GetKeyType() );

  iter = new SmiKeyedFileIterator( true );
  if( !file->SelectRecord( smiKey, *iter ) )
  {
    delete iter;
    return 0;
  }
  return new HashIterator( iter );
}

/*

5 Type constructor ~hash~

5.1 Type property of type constructor ~hash~

*/
static ListExpr
HashProp()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"<relation> createhash [<attrname>] where "
  "<attrname> is the key");

  return (nl->TwoElemList(
            nl->TwoElemList(nl->StringAtom("Creation"),
           nl->StringAtom("Example Creation")),
            nl->TwoElemList(examplelist,
           nl->StringAtom("(let myhash = ten "
           "createhash [no])"))));
}

/*
5.1 Function ~OutHash~

A hash does not make any sense as an independent value since
the record ids stored in it become obsolete as soon as
the underlying relation is deleted. Therefore this function
outputs an empty list.

*/
ListExpr OutHash(ListExpr typeInfo, Word  value)
{
  return nl->TheEmptyList();
}

/*
5.3 Funcion ~SaveToListHash~

*/
ListExpr SaveToListHash(ListExpr typeInfo, Word  value)
{
  Hash *hash = (Hash*)value.addr;

  return nl->IntAtom( hash->GetFileId() );
}

/*

5.2 Function ~CreateHash~

*/
Word CreateHash(const ListExpr typeInfo)
{
  return SetWord( new Hash( ExtractKeyTypeFromTypeInfo( typeInfo ) ) );
}

/*

5.3 Function ~InHash~

Reading a hash from a list does not make sense because a hash
is not an independent value. Therefore calling this function leads
to program abort.

*/
Word InHash(ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct)
{
  correct = false;
  return SetWord(Address(0));
}

/*
5.3 Funcion ~RestoreFromListHash~

*/
Word RestoreFromListHash( ListExpr typeInfo, ListExpr value,
                           int errorPos, ListExpr& errorInfo, bool& correct)
{
  SmiKey::KeyDataType keyType = ExtractKeyTypeFromTypeInfo( typeInfo );
  SmiFileId fileId = nl->IntValue(value);

  Hash *hash = new Hash( keyType, fileId );
  if( hash->IsOpened() )
    return SetWord( hash );
  else
  {
    delete hash;
    return SetWord( Address( 0 ) );
  }
}

/*

5.4 Function ~CloseHash~

*/
void CloseHash(const ListExpr typeInfo, Word& w)
{
  Hash* hash = (Hash*)w.addr;
  delete hash;
}

/*

5.5 Function ~CloneHash~

It is not possible to clone a ~hash~ type, since it is not possible
to iterate through all records in a hash file.

*/
Word CloneHash(const ListExpr typeInfo, const Word& w)
{
  return SetWord(Address(0));
}

/*
5.6 Function ~DeleteHash~

*/
void DeleteHash(const ListExpr typeInfo, Word& w)
{
  Hash* hash = (Hash*)w.addr;
  hash->DeleteFile();
  delete hash;
}

/*
5.7 ~Check~-function of type constructor ~hash~

*/
bool CheckHash(ListExpr type, ListExpr& errorInfo)
{
  if((!nl->IsAtom(type))
    && (nl->ListLength(type) == 3)
    && nl->Equal(nl->First(type), nl->SymbolAtom("hash")))
  {
    return
      am->CheckKind("TUPLE", nl->Second(type), errorInfo)
      && am->CheckKind("DATA", nl->Third(type), errorInfo);
  }
  errorInfo = nl->Append(errorInfo,
    nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("HASH"), type));
  return false;
}

void* CastHash(void* addr)
{
  return ( 0 );
}

int SizeOfHash()
{
  return 0;
}

/*
5.8 ~OpenFunction~ of type constructor ~hash~

*/
bool
OpenHash( SmiRecord& valueRecord,
           size_t& offset,
           const ListExpr typeInfo,
           Word& value )
{
  value = SetWord( Hash::Open( valueRecord, offset, typeInfo ) );
  bool rc  = value.addr != 0;
  return rc;  
}

Hash *Hash::Open( SmiRecord& valueRecord, size_t& offset, 
                    const ListExpr typeInfo )
{
  return new Hash( ExtractKeyTypeFromTypeInfo( typeInfo ), valueRecord,
                    offset );
}

/*
5.9 ~SaveFunction~ of type constructor ~hash~

*/
bool
SaveHash( SmiRecord& valueRecord,
           size_t& offset, 
           const ListExpr typeInfo,
           Word& value )
{
  Hash *hash = (Hash*)value.addr;
  return hash->Save( valueRecord, offset, typeInfo );
}

bool Hash::Save(SmiRecord& record, size_t& offset, const ListExpr typeInfo)
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
5.11 Type Constructor object for type constructor ~hash~

*/
TypeConstructor 
cpphash( "hash",         HashProp,
          OutHash,        InHash,
          SaveToListHash, RestoreFromListHash,
          CreateHash,     DeleteHash,
          OpenHash,       SaveHash,
          CloseHash,      CloneHash,
          CastHash,       SizeOfHash,
          CheckHash );

/*

6 Operators of the hash algebra

6.1 Operator ~createhash~

6.1.1 Type Mapping of operator ~createhash~

*/
ListExpr CreateHashTypeMap(ListExpr args)
{
  string argstr;

  CHECK_COND(nl->ListLength(args) == 2, 
             "Operator createhash expects a list of length two");

  ListExpr first = nl->First(args);

  nl->WriteToString(argstr, first);
  CHECK_COND( (   (!nl->IsAtom(first) &&
             nl->IsEqual(nl->First(first), "rel") && 
             IsRelDescription(first)
                  ) 
                ||
      (!nl->IsAtom(first) &&
             nl->IsEqual(nl->First(first), "stream") &&
             (nl->ListLength(first) == 2) && 
                   !nl->IsAtom(nl->Second(first)) &&
       (nl->ListLength(nl->Second(first)) == 2) &&
       nl->IsEqual(nl->First(nl->Second(first)), "tuple") &&
             IsTupleDescription(nl->Second(nl->Second(first)))
                  ) 
              ),
    "Operator createhash expects as first argument a list with structure\n"
    "rel(tuple ((a1 t1)...(an tn))) or stream (tuple ((a1 t1)...(an tn)))\n"
    "Operator createhash gets a list with structure '" + argstr + "'."); 

  ListExpr second = nl->Second(args);
  nl->WriteToString(argstr, second);
  
  CHECK_COND(nl->IsAtom(second) && nl->AtomType(second) == SymbolType, 
    "Operator createhash expects as second argument an attribute name\n"
    "bug gets '" + argstr + "'.");

  string attrName = nl->SymbolValue(second);
  ListExpr tupleDescription = nl->Second(first),
           attrList = nl->Second(tupleDescription);

  nl->WriteToString(argstr, attrList);
  int attrIndex;
  ListExpr attrType;
  CHECK_COND((attrIndex = FindAttribute(attrList, attrName, attrType)) > 0, 
    "Operator createhash expects as a second argument an attribute name\n"
    "Attribute name '" + attrName + "' is not known.\n"
    "Known Attribute(s): " + argstr);

  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  nl->WriteToString(argstr, attrType);
  CHECK_COND(nl->SymbolValue(attrType) == "string" ||
             nl->SymbolValue(attrType) == "int" ||
             nl->SymbolValue(attrType) == "real" ||
             am->CheckKind("INDEXABLE", attrType, errorInfo), 
    "Operator createhash expects as a second argument an attribute of types\n"
    "int, real, string, or any attribute that implements the kind INDEXABLE\n"
    "but gets '" + argstr + "'.");

  if( nl->IsEqual(nl->First(first), "rel") )
  {
    return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->OneElemList(
          nl->IntAtom(attrIndex)),
        nl->ThreeElemList(
          nl->SymbolAtom("hash"),
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
          "Operator createhash expects as first argument a stream with\n"
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
      "Operator createhash expects as first argument a stream with\n"
      "one and only one attribute of type tid but gets\n'" + argstr + "'.");

    return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->TwoElemList(
          nl->IntAtom(attrIndex),
          nl->IntAtom(tidIndex)),
        nl->ThreeElemList(
          nl->SymbolAtom("hash"),
          nl->TwoElemList(
            nl->SymbolAtom("tuple"),
            newAttrList),
          attrType));
  }
}

/*
6.1.2 Selection function of operator ~createhash~

*/
int CreateHashSelect( ListExpr args )
{
  if( nl->IsEqual(nl->First(nl->First(args)), "rel") )
    return 0;
  if( nl->IsEqual(nl->First(nl->First(args)), "stream") )
    return 1; 
  return -1;
}

/*
6.1.3 Value mapping function of operator ~createhash~

*/
int
CreateHashValueMapping_Rel(Word* args, Word& result, int message, 
                            Word& local, Supplier s)
{
  result = qp->ResultStorage(s);

  Relation* relation = (Relation*)args[0].addr;
  Hash* hash = (Hash*)result.addr;
  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

  assert(hash != 0);
  assert(relation != 0);

  if( !hash->IsOpened() )
    return CANCEL;
  hash->Truncate();

  GenericRelationIterator *iter = relation->MakeScan();
  Tuple* tuple;
  while( (tuple = iter->GetNextTuple()) != 0 )
  {
    SmiKey key;

    if( (StandardAttribute *)tuple->GetAttribute(attrIndex)->IsDefined() )
    {
      AttrToKey( (StandardAttribute *)tuple->GetAttribute(attrIndex), 
                 key, hash->GetKeyType() );
      hash->Append( key, iter->GetTupleId() );
    }
    tuple->DeleteIfAllowed();
  }
  delete iter;

  return 0;
}

int
CreateHashValueMapping_Stream(Word* args, Word& result, int message, 
                               Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  Hash* hash = (Hash*)result.addr;
  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1,
      tidIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;
  Word wTuple;

  assert(hash != 0);
  if( !hash->IsOpened() )
    return CANCEL;
  hash->Truncate();

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);
  while (qp->Received(args[0].addr))
  {
    Tuple* tuple = (Tuple*)wTuple.addr;
    SmiKey key;
    if( (StandardAttribute *)tuple->GetAttribute(attrIndex)->IsDefined() &&
        (StandardAttribute *)tuple->GetAttribute(tidIndex)->IsDefined() )
    {
      AttrToKey( (StandardAttribute *)tuple->GetAttribute(attrIndex), 
                 key, hash->GetKeyType() );
      hash->Append( key, 
                 ((TupleIdentifier *)tuple->GetAttribute(tidIndex))->GetTid());
    }
    tuple->DeleteIfAllowed();
   
    qp->Request(args[0].addr, wTuple); 
  }
  qp->Close(args[0].addr);

  return 0;
}

/*

6.1.4 Specification of operator ~createhash~

*/
const string CreateHashSpec  = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>(((rel (tuple (x1 t1)...(xn tn)))) xi)"
    " -> (hash (tuple ((x1 t1)...(xn tn))) ti)\n"
    "((stream (tuple (x1 t1)...(xn tn) (id tid))) xi)"
    " -> (hash (tuple ((x1 t1)...(xn tn))) ti)</text--->"
    "<text>_ createhash [ _ ]</text--->"
    "<text>Creates a hash. The key type ti must "
    "be either string or int or real or to implement the "
    "kind INDEXABLE. The operator only accepts input tuple types "
    "containing 0 (for a relation) or 1 (for a stream) "
    "attribute of type tid. The naming of this attribute is of "
    "no concern.</text--->"
    "<text>let myhash = ten createhash [nr];\n"
    "let myhash = ten feed extend[id: tupleid(.)] "
    "sortby[no asc] createhash[no]</text--->"
    ") )";

/*

6.1.5 Definition of operator ~createhash~

*/
ValueMapping createhashmap[] = { CreateHashValueMapping_Rel, 
                                  CreateHashValueMapping_Stream };

Operator createhash (
          "createhash",             // name
          CreateHashSpec,           // specification
          2,                         // number of overloaded functions
          createhashmap,            // value mapping
          CreateHashSelect,         // trivial selection function
          CreateHashTypeMap         // type mapping
);

/*

6.2 Operator ~exactmatch~

6.2.1 Type mapping function

*/
ListExpr HashExactMatchTypeMap(ListExpr args)
{
  const string errmsg = "Incorrect input for operator exactmatch.";
  int nKeys = 1;

  CHECK_COND(!nl->IsEmpty(args), errmsg);
  CHECK_COND(!nl->IsAtom(args), errmsg);
  CHECK_COND(nl->ListLength(args) == nKeys + 2, errmsg);

  /* Split argument in three parts */
  ListExpr hashDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr keyDescription = nl->Third(args);

  /* find out type of key */
  CHECK_COND(nl->IsAtom(keyDescription), errmsg);
  CHECK_COND(nl->AtomType(keyDescription) == SymbolType, errmsg);

  /* handle hash part of argument */
  CHECK_COND(!nl->IsEmpty(hashDescription), errmsg);
  CHECK_COND(!nl->IsAtom(hashDescription), errmsg);
  CHECK_COND(nl->ListLength(hashDescription) == 3, errmsg);

  ListExpr hashSymbol = nl->First(hashDescription);;
  ListExpr hashTupleDescription = nl->Second(hashDescription);
  ListExpr hashKeyType = nl->Third(hashDescription);

  /* check that the type of given key equals the hash key type */
  CHECK_COND(nl->Equal(keyDescription, hashKeyType), errmsg);

  /* handle hash type constructor */
  CHECK_COND(nl->IsAtom(hashSymbol), errmsg);
  CHECK_COND(nl->AtomType(hashSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(hashSymbol) == "hash", errmsg);

  CHECK_COND(!nl->IsEmpty(hashTupleDescription), errmsg);
  CHECK_COND(!nl->IsAtom(hashTupleDescription), errmsg);
  CHECK_COND(nl->ListLength(hashTupleDescription) == 2, errmsg);
  ListExpr hashTupleSymbol = nl->First(hashTupleDescription);;
  ListExpr hashAttrList = nl->Second(hashTupleDescription);

  CHECK_COND(nl->IsAtom(hashTupleSymbol), errmsg);
  CHECK_COND(nl->AtomType(hashTupleSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(hashTupleSymbol) == "tuple", errmsg);
  CHECK_COND(IsTupleDescription(hashAttrList), errmsg);

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

  /* check that hash and rel have the same associated tuple type */
  CHECK_COND(nl->Equal(attrList, hashAttrList), errmsg);

  ListExpr resultType =
    nl->TwoElemList(
      nl->SymbolAtom("stream"),
      tupleDescription);

  return resultType;
}



/*

6.2.2 Value mapping function

*/

#ifndef USE_PROGRESS

// standard version


struct HashExactMatchLocalInfo
{
  Relation* relation;
  HashIterator* iter;
};


int
HashExactMatch(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Hash* hash;
  StandardAttribute* key;
  StandardAttribute* secondKey;
  Tuple* tuple;
  SmiRecordId id;
  HashExactMatchLocalInfo* localInfo;

  switch (message)
  {
    case OPEN :
      localInfo = new HashExactMatchLocalInfo;
      hash = (Hash*)args[0].addr;
      localInfo->relation = (Relation*)args[1].addr;
      key = (StandardAttribute*)args[2].addr;

      assert(hash != 0);
      assert(localInfo->relation != 0);
      assert(key != 0);

      localInfo->iter = hash->ExactMatch(key);
      if(localInfo->iter == 0)
      {
        delete localInfo;
        return -1;
      }
      local = SetWord(localInfo);
      return 0;

    case REQUEST :
      localInfo = (HashExactMatchLocalInfo*)local.addr;
      assert(localInfo != 0);

      if(localInfo->iter->Next())
      {
        id = localInfo->iter->GetId();
        tuple = localInfo->relation->GetTuple( id );
        if(tuple == 0)
        {
          cerr << "Could not find tuple for the given tuple id. "
               << "Maybe the given hash and the given relation "
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
      localInfo = (HashExactMatchLocalInfo*)local.addr;
      delete localInfo->iter;
      delete localInfo;
      return 0;
  }
  return 0;
}



# else

// progress version



class HashExactMatchLocalInfo: public ProgressLocalInfo
{

public:

  Relation* relation;
  HashIterator* iter;
  bool first;
  int completeCalls;
  int completeReturned;
};



int
HashExactMatch(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Hash* hash;
  StandardAttribute* key;
  Tuple* tuple;
  SmiRecordId id;
  HashExactMatchLocalInfo* ili;

  ili = (HashExactMatchLocalInfo*)local.addr;

  switch (message)
  {
    case OPEN :

      //local info kept over many calls of OPEN!
      //useful for loopjoin

      if ( !ili ) //first time
      {
        ili = new HashExactMatchLocalInfo;
        ili->completeCalls = 0;
        ili->completeReturned = 0;

        ili->relation = (Relation*)args[1].addr;
        local = SetWord(ili);
      }

      hash = (Hash*)args[0].addr;
      key = (StandardAttribute*)args[2].addr;

      assert(hash != 0);
      assert(ili->relation != 0);
      assert(key != 0);

      ili->iter = hash->ExactMatch(key);
      if(ili->iter == 0)
      {
        delete ili;
        return -1;
      }

      return 0;

    case REQUEST :

      assert(ili != 0);

      if(ili->iter->Next())
      {
        id = ili->iter->GetId();
        tuple = ili->relation->GetTuple( id );
        if(tuple == 0)
        {
          cerr << "Could not find tuple for the given tuple id. "
               << "Maybe the given hash and the given relation "
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
      delete ili->iter;

      ili->completeCalls++;
      ili->completeReturned += ili->returned;
      ili->returned = 0;

      return 0;


    case CLOSEPROGRESS:
      if ( ili ) delete ili;
      return 0;


    case REQUESTPROGRESS :
      ProgressInfo *pRes;
      pRes = (ProgressInfo*) result.addr;

      const double uHashExactMatch = 0.15;      //ms per search
      const double vHashExactMatch = 0.018;     //ms per result tuple

      if ( !ili ) return CANCEL;
      else
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

        pRes->CopySizes(ili);


        if ( ili->completeCalls > 0 )     //called in a loopjoin
        {
          pRes->Card = 
            (double) ili->completeReturned / (double) ili->completeCalls;
          
        }
        else      //single or first call
        {
          if ( fabs(qp->GetSelectivity(s) - 0.1) < 0.000001 ) // default
            pRes->Card = (double) ili->defaultValue;
          else                                              // annotated
            pRes->Card = ili->total * qp->GetSelectivity(s);
 
        if ( (double) ili->returned > pRes->Card )   // there are more tuples
            pRes->Card = (double) ili->returned * 1.1;   // than expected

        if ( !ili->iter )  // hash has been finished
            pRes->Card = (double) ili->returned; 
 
        if ( pRes->Card > (double) ili->total ) // more than all cannot be
            pRes->Card = (double) ili->total;

        }


        pRes->Time = uHashExactMatch + pRes->Card * vHashExactMatch;

        pRes->Progress = 
          (uHashExactMatch + (double) ili->returned * vHashExactMatch)
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
const string HashExactMatchSpec  =   
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>((hash (tuple ((x1 t1)...(xn tn)))"
  " ti)(rel (tuple ((x1 t1)...(xn tn)))) ti) ->"
  " (stream (tuple ((x1 t1)...(xn tn))))"
  "</text--->"
  "<text>_ _ exactmatch [ _ ]</text--->"
  "<text>Uses the given hash to find all tuples"
  " in the given relation with .xi = argument "
  "value.</text--->"
  "<text>query citiesNameInd cities exactmatch"
  " [\"Berlin\"] consume; where citiesNameInd "
  "is e.g. created with 'let citiesNameInd = "
  "cities createhash [name]'</text--->"
  ") )";
/*

6.2.4 Definition of operator ~exactmatch~

*/
Operator hashexactmatch (
   "exactmatch",                  // name
   HashExactMatchSpec,            // specification
   HashExactMatch,                // value mapping
   Operator::SimpleSelect,        // trivial selection function
   HashExactMatchTypeMap          // type mapping
);


/*
6.3.1 Type mapping function of operator ~exactmatchS~

*/
ListExpr HashExactMatchSTypeMap(ListExpr args)
{
  string errmsg = "Incorrect input for operator exactmatch.";
  int nKeys = 1;

  CHECK_COND(!nl->IsEmpty(args), errmsg);
  CHECK_COND(!nl->IsAtom(args), errmsg);
  CHECK_COND(nl->ListLength(args) == nKeys + 1, errmsg);

  /* Split argument in two/three parts */
  ListExpr hashDescription = nl->First(args);
  ListExpr keyDescription = nl->Second(args);

  /* find out type of key */
  CHECK_COND(nl->IsAtom(keyDescription), errmsg);
  CHECK_COND(nl->AtomType(keyDescription) == SymbolType, errmsg);

  /* handle hash part of argument */
  CHECK_COND(!nl->IsEmpty(hashDescription), errmsg);
  CHECK_COND(!nl->IsAtom(hashDescription), errmsg);
  CHECK_COND(nl->ListLength(hashDescription) == 3, errmsg);

  ListExpr hashSymbol = nl->First(hashDescription);;
  ListExpr hashTupleDescription = nl->Second(hashDescription);
  ListExpr hashKeyType = nl->Third(hashDescription);

  /* check that the type of given key equals the hash key type */
  CHECK_COND(nl->Equal(keyDescription, hashKeyType), errmsg);

  /* handle hash type constructor */
  CHECK_COND(nl->IsAtom(hashSymbol), errmsg);
  CHECK_COND(nl->AtomType(hashSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(hashSymbol) == "hash", errmsg);

  CHECK_COND(!nl->IsEmpty(hashTupleDescription), errmsg);
  CHECK_COND(!nl->IsAtom(hashTupleDescription), errmsg);
  CHECK_COND(nl->ListLength(hashTupleDescription) == 2, errmsg);
  ListExpr hashTupleSymbol = nl->First(hashTupleDescription);;
  ListExpr hashAttrList = nl->Second(hashTupleDescription);

  CHECK_COND(nl->IsAtom(hashTupleSymbol), errmsg);
  CHECK_COND(nl->AtomType(hashTupleSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(hashTupleSymbol) == "tuple", errmsg);
  CHECK_COND(IsTupleDescription(hashAttrList), errmsg);

  /* return result type */

  return nl->TwoElemList(
          nl->SymbolAtom("stream"),
          nl->TwoElemList(
            nl->SymbolAtom("tuple"), 
            nl->OneElemList(
              nl->TwoElemList(
                nl->SymbolAtom("id"),
                nl->SymbolAtom("tid")))));

}

/*

6.3.2 Value mapping function of operator ~exactmatchS~

*/

struct HashExactMatchSLocalInfo
{
  HashIterator* iter;
  TupleType *resultTupleType;
};

int
HashExactMatchS(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Hash* hash;
  StandardAttribute* key;
  SmiRecordId id;
  HashExactMatchSLocalInfo* localInfo;

  switch (message)
  {
    case OPEN :
      localInfo = new HashExactMatchSLocalInfo;
      hash = (Hash*)args[0].addr;

      key = (StandardAttribute*)args[1].addr;

      assert(hash != 0);
      assert(key != 0);

      localInfo->iter = hash->ExactMatch(key);
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
      localInfo = (HashExactMatchSLocalInfo*)local.addr;
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
      localInfo = (HashExactMatchSLocalInfo*)local.addr;
      delete localInfo->iter;
      localInfo->resultTupleType->DeleteIfAllowed();
      delete localInfo;
      return 0;
  }
  return 0;
}

/*

6.3.3 Specification of operator ~exactmatchS~

*/
const string HashExactMatchSSpec  =         
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(hash( (tuple(X))) ti) ti->)"
  "(stream((id tid)))</text--->"
  "<text> _ exactmatchS [ _ ]</text--->"
  "<text>Uses the given hash to find all tuple "
  "identifiers where the key matches argument "
  "value ti.</text--->"
  "<text>query citiesNameInd exactmatchS"
  " [\"Berlin\"] cities gettuples consume; "
  "where citiesNameInd is e.g. created with "
  "'let citiesNameInd = cities createhash "
  "[name]'</text--->"
  ") )";
/*

6.3.4 Definition of operator ~exactmatchS~

*/
Operator hashexactmatchs (
         "exactmatchS",           // name
         HashExactMatchSSpec,     // specification
         HashExactMatchS,         // value mapping
         Operator::SimpleSelect,  // trivial selection function
         HashExactMatchSTypeMap   // type mapping
);

/*
6.4 Operator ~inserthash~ 

For each tuple of the inputstream inserts an entry into the hash. The entry 
is built from the attribute 
of the tuple over which the tree is built and the tuple-identifier of the 
inserted tuple which is extracted
as the last attribute of the tuple of the inputstream.


6.4.0 General Type mapping function of operators ~inserthash~, ~deletehash~ 
and ~updatehash~


Type mapping ~inserthash~ and ~deletehash~ 

----     (stream (tuple ((a1 x1) ... (an xn) (TID tid)))) (hash X ti)) ai

        -> (stream (tuple ((a1 x1) ... (an xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----

Type mapping ~updatehash~

----     
     (stream (tuple ((a1 x1) ... (an xn) 
                     (a1_old x1) ... (an_old xn) (TID tid)))) (hash X ti)) ai

  -> (stream (tuple ((a1 x1) ... (an xn) 
                     (a1_old x1) ... (an_old xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----


*/

ListExpr allUpdatesHashTypeMap( const ListExpr& args, string opName )
{
  ListExpr rest,next,listn,lastlistn,restHashAttrs,oldAttribute,outList;
  string argstr, argstr2, oldName;
  

  /* Split argument in three parts */
  ListExpr streamDescription = nl->First(args);
  ListExpr hashDescription = nl->Second(args);
  ListExpr nameOfKeyAttribute = nl->Third(args);

  // Test stream
  nl->WriteToString(argstr, streamDescription);
        CHECK_COND(nl->ListLength(streamDescription) == 2  &&
          (TypeOfRelAlgSymbol(nl->First(streamDescription)) == stream) &&
          (nl->ListLength(nl->Second(streamDescription)) == 2) &&
          (TypeOfRelAlgSymbol(nl->First(nl->Second(streamDescription)))==tuple)
          &&
          (nl->ListLength(nl->Second(streamDescription)) == 2) &&
          (IsTupleDescription(nl->Second(nl->Second(streamDescription)))),
          "Operator " + opName + 
          " expects as first argument a list with structure "
          "(stream (tuple ((a1 t1)...(an tn)(TID tid)))\n "
          "Operator " + opName + " gets as first argument '" + argstr + "'.");
    
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
      "Operator " + opName + 
          ": Type of last attribute of tuples of the inputstream must be tid");
  // Test hash

  /* handle hash part of argument */
  CHECK_COND(!nl->IsEmpty(hashDescription), 
          "Operator " + opName + 
          ": Description for the hash may not be empty");
  CHECK_COND(!nl->IsAtom(hashDescription), 
          "Operator " + opName + 
          ": Description for the hash may not be an atom");
  CHECK_COND(nl->ListLength(hashDescription) == 3, 
          "Operator " + opName + 
          ": Description for the hash must consist of three parts");

  ListExpr hashSymbol = nl->First(hashDescription);
  ListExpr hashTupleDescription = nl->Second(hashDescription);
  ListExpr hashKeyType = nl->Third(hashDescription);

  /* handle hash type constructor */
  CHECK_COND(nl->IsAtom(hashSymbol), 
          "Operator " + opName + 
          ": First part of the hash-description has to be 'hash'");
  CHECK_COND(nl->AtomType(hashSymbol) == SymbolType, 
          "Operator " + opName + 
          ": First part of the hash-description has to be 'bree' ");
  CHECK_COND(nl->SymbolValue(hashSymbol) == "hash",
          "Operator " + opName + 
          ": First part of the hash-description has to be 'bree' ");

  /* handle hash tuple description */
  CHECK_COND(!nl->IsEmpty(hashTupleDescription), 
          "Operator " + opName + ": Second part of the "
          "hash-description has to be a tuple-description ");
  CHECK_COND(!nl->IsAtom(hashTupleDescription), 
          "Operator " + opName + ": Second part of the "
          "hash-description has to be a tuple-description ");
  CHECK_COND(nl->ListLength(hashTupleDescription) == 2, 
          "Operator " + opName + ": Second part of the "
          "hash-description has to be a tuple-description ");
  ListExpr hashTupleSymbol = nl->First(hashTupleDescription);;
  ListExpr hashAttrList = nl->Second(hashTupleDescription);

  CHECK_COND(nl->IsAtom(hashTupleSymbol),
          "Operator " + opName + ": Second part of the "
          "hash-description has to be a tuple-description ");
  CHECK_COND(nl->AtomType(hashTupleSymbol) == SymbolType, 
          "Operator " + opName + ": Second part of the "
          "hash-description has to be a tuple-description ");
  CHECK_COND(nl->SymbolValue(hashTupleSymbol) == "tuple", 
           "Operator " + opName + ": Second part of the "
           "hash-description has to be a tuple-description ");
  CHECK_COND(IsTupleDescription(hashAttrList), 
           "Operator " + opName + ": Second part of the "
           "hash-description has to be a tuple-description ");
  
  /* Handle key-part of hashdescription */
  CHECK_COND(nl->IsAtom(hashKeyType), 
           "Operator " + opName + ": Key of the hash has to be an atom");
  CHECK_COND(nl->AtomType(hashKeyType) == SymbolType,
           "Operator " + opName + ": Key of the hash has to be an atom");
  
  // Handle third argument which shall be the name of the attribute of 
  // the streamtuples that serves as the key for the hash
  // Later on it is checked if this name is an attributename of the 
  // inputtuples
  CHECK_COND(nl->IsAtom(nameOfKeyAttribute), 
           "Operator " + opName + ": Name of the "
           "key-attribute of the streamtuples has to be an atom");
  CHECK_COND(nl->AtomType(nameOfKeyAttribute) == SymbolType, 
           "Operator " + opName + ": Name of the key-attribute "
           "of the streamtuples has to be an atom");

  //Test if stream-tupledescription fits to hash-tupledescription
  rest = nl->Second(nl->Second(streamDescription));
  CHECK_COND(nl->ListLength(rest) > 1 , 
           "Operator " + opName + ": There must be at least two "
           "attributes in the tuples of the tuple-stream");
  // For updates the inputtuples need to carry the old attributevalues 
  // after the new values but their names with an additional _old at 
  // the end
  if (opName == "updatehash")
  {
    listn = nl->OneElemList(nl->First(rest));
    lastlistn = listn;
    rest = nl->Rest(rest);
    // Compare first part of the streamdescription
    while (nl->ListLength(rest) > nl->ListLength(hashAttrList)+1)
    {
      lastlistn = nl->Append(lastlistn,nl->First(rest));
      rest = nl->Rest(rest);
    }
    CHECK_COND(nl->Equal(listn,hashAttrList), 
                  "Operator " + opName + ":  First part of the "
                  "tupledescription of the stream has to be the same as the "
                  "tupledescription of the hash");
    // Compare second part of the streamdescription
    restHashAttrs = hashAttrList;
    while (nl->ListLength(rest) >  1)
    {
      nl->WriteToString(oldName, 
                        nl->First(nl->First(restHashAttrs)));
      oldName += "_old";
      oldAttribute = nl->TwoElemList(nl->SymbolAtom(oldName),
                                     nl->Second(nl->First(restHashAttrs)));
      CHECK_COND(nl->Equal(oldAttribute,nl->First(rest)), 
        "Operator " + opName + ":  Second part of the "
        "tupledescription of the stream without the last "
        "attribute has to be the same as the tuple"
        "description of the hash except for that the "
        "attributenames carry an additional '_old.'");
      rest = nl->Rest(rest);
      restHashAttrs = nl->Rest(restHashAttrs);
    }
  }
  // For insert and delete check whether tupledescription of the stream
  // without the last attribute is the same as the tupledescription 
  // of the hash 
  else
  {
    listn = nl->OneElemList(nl->First(rest));
    lastlistn = listn;
    rest = nl->Rest(rest);
    while (nl->ListLength(rest) > 1)
    {
      lastlistn = nl->Append(lastlistn,nl->First(rest));
      rest = nl->Rest(rest);
    }
    CHECK_COND(nl->Equal(listn,hashAttrList), 
                  "Operator " + opName + ": tupledescription of the stream "
                  "without the last attribute has to be the same as the "
                  "tupledescription of the hash");
  }
  
  
  // Test if attributename of the third argument exists as a name in the 
  // attributlist of the streamtuples
  string attrname = nl->SymbolValue(nameOfKeyAttribute);
  ListExpr attrtype;
  int j = FindAttribute(listn,attrname,attrtype);
  CHECK_COND(j != 0, "Operator " + opName + 
          ": Name of the attribute that shall contain the keyvalue for the"
          "hash was not found as a name of the attributes of the tuples of "
          "the inputstream"); 
  //Test if type of the attriubte which shall be taken as a key is the 
  //same as the keytype of the hash
  CHECK_COND(nl->Equal(attrtype,hashKeyType), "Operator " + opName + 
          ": Type of the attribute that shall contain the keyvalue for the"
          "hash is not the same as the keytype of the hash");
  //Append the index of the attribute over which the hash is built to 
  //the resultlist. 
  outList = nl->ThreeElemList(nl->SymbolAtom("APPEND"), 
                          nl->OneElemList(nl->IntAtom(j)),streamDescription);
  return outList;
}

/*
6.4.1 TypeMapping of operator ~inserthash~

*/
ListExpr insertHashTypeMap(ListExpr args)
{
  return allUpdatesHashTypeMap(args, "inserthash");
}



/*
6.4.2 ValueMapping of operator ~inserthash~

*/

int insertHashValueMap(Word* args, Word& result, int message, 
                        Word& local, Supplier s)
{
  Word t;
  Tuple* tup;
  Hash* hash;
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
      hash = (Hash*)(args[1].addr);    
      assert(hash != 0);
      qp->Request(args[0].addr,t);     
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        keyAttr = tup->GetAttribute(index - 1);
        tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
        oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
        AttrToKey((StandardAttribute*)keyAttr, key, hash->GetKeyType());
        hash->Append(key,oldTid);
        result = SetWord(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s,1));  
      return 0;
  }
  return 0;
}

/*
6.4.3 Specification of operator ~inserthash~

*/
const string insertHashSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>stream(tuple(x@[TID tid])) x (hash(tuple(x) ti) xi)"
  " -> stream(tuple(x@[TID tid]))] "
  "</text--->"
  "<text>_ _ inserthash [_]</text--->"
  "<text>Inserts references to the tuples with TupleId 'tid' "
  "into the hash.</text--->"
  "<text>query neueStaedte feed staedte insert staedte_Name "
  " inserthash [Name] count "
  "</text--->"
  ") )";

/*
6.4.4 Definition of operator ~inserthash~

*/
Operator inserthash (
         "inserthash",              // name
         insertHashSpec,            // specification
         insertHashValueMap,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         insertHashTypeMap          // type mapping
);


/*
6.5 Operator ~deletehash~ 

For each tuple of the inputstream deletes the corresponding entry from the 
hash. The entry is built from the attribute of the tuple over which the tree 
is built and the tuple-identifier of the deleted tuple which is extracted
as the last attribute of the tuple of the inputstream.


6.5.1 TypeMapping of operator ~deletehash~

*/

ListExpr deleteHashTypeMap(ListExpr args)
{
  return allUpdatesHashTypeMap(args, "deletehash");
}

/*
6.5.2 ValueMapping of operator ~deletehash~

*/

int deleteHashValueMap(Word* args, Word& result, int message, 
                        Word& local, Supplier s)
{
  Word t;
  Tuple* tup;
  Hash* hash;
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
      hash = (Hash*)(args[1].addr);    
      assert(hash != 0);
      qp->Request(args[0].addr,t);    
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        keyAttr = tup->GetAttribute(index - 1);
        tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
        oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
        AttrToKey((StandardAttribute*)keyAttr, key, hash->GetKeyType());
        hash->Delete(key,oldTid);
        result = SetWord(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s,1));
      return 0;
  }
  return 0;
}

/*
6.5.3 Specification of operator ~deletehash~

*/
const string deleteHashSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>stream(tuple(x@[TID tid])) x (hash(tuple(x) ti) xi)"
  " -> stream(tuple(x@[TID tid]))] "
  "</text--->"
  "<text>_ _ deletehash [_]</text--->"
  "<text>Deletes the references to the tuples with TupleId 'tid' "
  "from the hash.</text--->"
  "<text>query alteStaedte feed staedte deletesearch staedte_Name "
  " deletehash [Name] count "
  "</text--->"
  ") )";

/*
6.5.4 Definition of operator ~deletehash~

*/
Operator deletehash (
         "deletehash",              // name
         deleteHashSpec,            // specification
         deleteHashValueMap,        // value mapping
         Operator::SimpleSelect,     // trivial selection function
         deleteHashTypeMap          // type mapping
);


/*
6.6 Operator ~updatehash~ 

For each tuple of the inputstream updates the entry in the hash. The entry is 
built from the attribute of the tuple over which the tree is built and the 
tuple-identifier of the updated tuple which is extracted as the last attribute 
of the tuple of the inputstream.

6.6.1 TypeMapping of operator ~updatehash~

*/

ListExpr updateHashTypeMap(ListExpr args)
{
  return allUpdatesHashTypeMap(args, "updatehash");
}

/*

6.6.2 ValueMapping of operator ~updatehash~

*/

int updateHashValueMap(Word* args, Word& result, int message, 
                        Word& local, Supplier s)
{
  Word t;
  Tuple* tup;
  Hash* hash;
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
      hash = (Hash*)(args[1].addr);    
      assert(hash != 0);
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        keyAttr = tup->GetAttribute(index - 1);
        oldKeyAttr = tup->GetAttribute((tup->GetNoAttributes()-1)/2+index-1);
        tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
        oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
        AttrToKey((StandardAttribute*)keyAttr, key, hash->GetKeyType());
        AttrToKey((StandardAttribute*)oldKeyAttr, oldKey, hash->GetKeyType());
        // Only update if key has changed
        if ((key > oldKey) || (oldKey > key))
        {
          hash->Delete(oldKey,oldTid);
          hash->Append(key,oldTid);
        }
        result = SetWord(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s,1));
      return 0;
  }
  return 0;
}

/*
6.6.3 Specification of operator ~updatehash~

*/
const string updateHashSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>stream(tuple(x@[(a1_old x1)...(an_old xn)(TID tid)])) x "
  "(hash(tuple(x) ti) xi)"
  " -> stream(tuple(x@[(a1_old x1)...(an_old xn)(TID tid)]))] "
  "</text--->"
  "<text>_ _ updatehash [_]</text--->"
  "<text>Updates references to the tuples with TupleId 'tid' "
  "in the hash.</text--->"
  "<text>query staedte feed filter [.Name = 'Hargen'] staedte "
  "updatedirect [Name: "
  "'Hagen'] staedte_Name updatehash [Name] count "
  "</text--->"
  ") )";

/*
6.6.4 Definition of operator ~updatehash~

*/
Operator updatehash (
         "updatehash",              // name
         updateHashSpec,            // specification
         updateHashValueMap,        // value mapping
         Operator::SimpleSelect,     // trivial selection function
         updateHashTypeMap          // type mapping
);



/*

7 Definition and initialization of hash algebra

*/
class HashAlgebra : public Algebra
{
 public:
  HashAlgebra() : Algebra()
  {
    AddTypeConstructor( &cpphash );

#ifndef USE_PROGRESS

    AddOperator(&createhash);
    AddOperator(&hashexactmatch);
    AddOperator(&hashexactmatchs);
    AddOperator(&inserthash);
    AddOperator(&deletehash);
    AddOperator(&updatehash);

#else

    AddOperator(&createhash);
    AddOperator(&hashexactmatch);   
    hashexactmatch.EnableProgress();
    AddOperator(&hashexactmatchs);
    AddOperator(&inserthash);
    AddOperator(&deletehash);
    AddOperator(&updatehash);

#endif
  }
  ~HashAlgebra() {};
};

extern "C"
Algebra*
InitializeHashAlgebra( NestedList* nlRef, 
                        QueryProcessor* qpRef,
                        AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (new HashAlgebra());
}
