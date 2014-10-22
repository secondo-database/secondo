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
#include "FTextAlgebra.h"
#include "ListUtils.h"

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

using namespace std;

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
temporary( false ),
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

HashIterator* Hash::ExactMatch( Attribute* key )
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

bool Hash::getFileStats( SmiStatResultType &result )
{
  if ( (file == 0) || !opened ){
    return false;
  }
  result = file->GetFileStatistics(SMI_STATS_EAGER);
  std::stringstream fileid;
  fileid << file->GetFileId();
  result.push_back(pair<string,string>("FilePurpose",
            "SecondaryHashIndexFile"));
  result.push_back(pair<string,string>("FileId",fileid.str()));
  return true;
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
    && nl->Equal(nl->First(type), nl->SymbolAtom(Hash::BasicType())))
  {
    return
      am->CheckKind(Kind::TUPLE(), nl->Second(type), errorInfo)
      && am->CheckKind(Kind::DATA(), nl->Third(type), errorInfo);
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
cpphash( Hash::BasicType(),         HashProp,
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

  string err = "rel(tuple) x attr_name or stream(tuple) x attr_name expected";
  if(nl->ListLength(args)!=2){
    return listutils::typeError(err + "( wrong number of arguments)");
  }

  ListExpr first = nl->First(args);
  ListExpr nameL = nl->Second(args);
  if(!listutils::isRelDescription(first) &&
     !listutils::isTupleStream(first)){
    return listutils::typeError(err);
  }
  if(!listutils::isSymbol(nameL)){
    return listutils::typeError(err + "( invalid attr name)");
  }

  string attrName = nl->SymbolValue(nameL);
  ListExpr tupleDescription = nl->Second(first);
  ListExpr attrlist = nl->Second(tupleDescription);

  ListExpr attrType;
  int attrIndex = listutils::findAttribute(attrlist, attrName, attrType);
  if(attrIndex==0){
   return listutils::typeError(err + "( attribute not found )");
  }


  if(!listutils::isBDBIndexableType(attrType)){
    return listutils::typeError(" attribute not indexable");
  }

  if( nl->IsEqual(nl->First(first), Relation::BasicType()) ) {
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbol::APPEND()),
        nl->OneElemList(
          nl->IntAtom(attrIndex)),
        nl->ThreeElemList(
          nl->SymbolAtom(Hash::BasicType()),
          tupleDescription,
          attrType));
  } else { // nl->IsEqual(nl->First(first), Symbol::STREAM())
    string name;
    int tidIndex = listutils::findType(attrlist,
                            nl->SymbolAtom(TupleIdentifier::BasicType()), name);
    if(tidIndex==0){
      return listutils::typeError("stream must contain a tid attribute");
    }
    string name2;
    int tidpos2 = listutils::findType(attrlist,
                                  nl->SymbolAtom(TupleIdentifier::BasicType()),
                                  name, tidIndex+1);
    if(tidpos2!=0){
      return listutils::typeError("multiple tid attributes found ");
    }
    set<string> k;
    k.insert(name);
    ListExpr newAttrList;
    ListExpr last;
    if(listutils::removeAttributes(attrlist, k, newAttrList,last)!=1){
      return listutils::typeError("error in removing tid attribute");
    }
    return nl->ThreeElemList(
        nl->SymbolAtom(Symbol::APPEND()),
        nl->TwoElemList(
          nl->IntAtom(attrIndex),
          nl->IntAtom(tidIndex)),
        nl->ThreeElemList(
          nl->SymbolAtom(Hash::BasicType()),
          nl->TwoElemList(
            nl->SymbolAtom(Tuple::BasicType()),
            newAttrList),
          attrType));
  }
}

/*
6.1.2 Selection function of operator ~createhash~

*/
int CreateHashSelect( ListExpr args )
{
  if( nl->IsEqual(nl->First(nl->First(args)), Relation::BasicType()) )
    return 0;
  if( nl->IsEqual(nl->First(nl->First(args)), Symbol::STREAM()) )
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
  CcInt::inttype attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

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

    if( (Attribute *)tuple->GetAttribute(attrIndex)->IsDefined() )
    {
      AttrToKey( (Attribute *)tuple->GetAttribute(attrIndex),
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
  CcInt::inttype attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1,
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
    if( (Attribute *)tuple->GetAttribute(attrIndex)->IsDefined() &&
        (Attribute *)tuple->GetAttribute(tidIndex)->IsDefined() )
    {
      AttrToKey( (Attribute *)tuple->GetAttribute(attrIndex),
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
  if(nl->ListLength(args)!=3){
    return listutils::typeError("three arguments expected");
  }
  ListExpr hashDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr keyDescription = nl->Third(args);

  string err = "hash x rel x key expected";
  if(!listutils::isHashDescription(hashDescription) ||
     !listutils::isRelDescription(relDescription) ||
     !listutils::isSymbol(keyDescription)){
    return listutils::typeError(err);
  }

  ListExpr hashKeyType = nl->Third(hashDescription);
  if(!nl->Equal(hashKeyType, keyDescription)){
    return listutils::typeError("type conflict between keys");
  }

  ListExpr tupleDescription = nl->Second(relDescription);
  ListExpr relAttrList = nl->Second(nl->Second(relDescription));
  ListExpr hashAttrList = nl->Second(nl->Second(hashDescription));
  if(!nl->Equal(relAttrList, hashAttrList)){
    return listutils::typeError("type conflicts between hash and relation");
  }

  ListExpr resultType =
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
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
  Attribute* key;
  Attribute* secondKey;
  Tuple* tuple;
  SmiRecordId id;
  HashExactMatchLocalInfo* localInfo;

  switch (message)
  {
    case OPEN :
      localInfo = new HashExactMatchLocalInfo;
      hash = (Hash*)args[0].addr;
      localInfo->relation = (Relation*)args[1].addr;
      key = (Attribute*)args[2].addr;

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
      if(localInfo){
         delete localInfo->iter;
         delete localInfo;
         local.addr=0;
      }
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
  bool progressInitialized;
};



int
HashExactMatch(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Hash* hash;
  Attribute* key;
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
      key = (Attribute*)args[2].addr;

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
        tuple = ili->relation->GetTuple( id, false );
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
      if(ili){
        delete ili->iter;

        ili->completeCalls++;
        ili->completeReturned += ili->returned;
        ili->returned = 0;
      }

      return 0;


    case CLOSEPROGRESS:
      if ( ili ) delete ili;
      local.addr=0;
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

  if(nl->ListLength(args)!=2){
    return listutils::typeError("tweo arguments expected");
  }
  string err = "hash x key expected";
  ListExpr hash = nl->First(args);
  ListExpr key = nl->Second(args);

  if(!listutils::isSymbol(key)){
    return listutils::typeError(err + ": invalid key type");
  }

  if(!listutils::isHashDescription(hash)){
    return listutils::typeError(err);
  }

  if(!nl->Equal(nl->Third(hash),key)){
   return listutils::typeError("type mismatch");
  }

  return nl->TwoElemList(
          nl->SymbolAtom(Symbol::STREAM()),
          nl->TwoElemList(
            nl->SymbolAtom(Tuple::BasicType()),
            nl->OneElemList(
              nl->TwoElemList(
                nl->SymbolAtom("id"),
                nl->SymbolAtom(TupleIdentifier::BasicType())))));

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
  Attribute* key;
  SmiRecordId id;
  HashExactMatchSLocalInfo* localInfo;

  switch (message)
  {
    case OPEN :
      localInfo = new HashExactMatchSLocalInfo;
      hash = (Hash*)args[0].addr;

      key = (Attribute*)args[1].addr;

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
      if(localInfo){
         delete localInfo->iter;
         localInfo->resultTupleType->DeleteIfAllowed();
         delete localInfo;
         local.addr=0;
      }
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
  if(nl->ListLength(args)!=3){
    return listutils::typeError("three arguments expected");
  }

  /* Split argument in three parts */
  ListExpr streamDescription = nl->First(args);
  ListExpr hashDescription = nl->Second(args);
  ListExpr nameOfKeyAttribute = nl->Third(args);

  string err = "stream(tuple) x hash x attrname expected";

  if(!listutils::isTupleStream(streamDescription) ||
     !listutils::isHashDescription(hashDescription) ||
     !listutils::isSymbol(nameOfKeyAttribute)){
    return listutils::typeError(err);
  }

  // Proceed to last attribute of stream-tuples
  ListExpr next;
  ListExpr rest = nl->Second(nl->Second(streamDescription));
  while (!(nl->IsEmpty(rest)))
  {
    next = nl->First(rest);
    rest = nl->Rest(rest);
  }

  if(!listutils::isSymbol(nl->Second(next),TupleIdentifier::BasicType())){
    return listutils::typeError("last attribute must be of type tid");
  }

  if(!listutils::isSymbol(nl->Third(hashDescription))){
    return listutils::typeError("key of hash must be an atom");
  }

  //Test if stream-tupledescription fits to hash-tupledescription
  rest = nl->Second(nl->Second(streamDescription));
  ListExpr hashAttrList = nl->Second(nl->Second(hashDescription));

  if(nl->ListLength(rest) < 2){
    return listutils::typeError("tuple stream must have at leat 2 attributes");
  }


  ListExpr listn;
  ListExpr lastlistn;
  ListExpr restHashAttrs;
  if (opName == "updatehash") {
    listn = nl->OneElemList(nl->First(rest));
    lastlistn = listn;
    rest = nl->Rest(rest);
    // Compare first part of the streamdescription
    while (nl->ListLength(rest) > nl->ListLength(hashAttrList)+1)
    {
      lastlistn = nl->Append(lastlistn,nl->First(rest));
      rest = nl->Rest(rest);
    }
    if(!nl->Equal(listn, hashAttrList)){
       return listutils::typeError("hash attr list and "
                                   "tuple attr list differ");
    }

    // Compare second part of the streamdescription
    restHashAttrs = hashAttrList;
    while (nl->ListLength(rest) >  1)
    {
      string oldName = nl->SymbolValue(nl->First(nl->First(restHashAttrs)));
      oldName += "_old";
      ListExpr oldAttribute = nl->TwoElemList(nl->SymbolAtom(oldName),
                                     nl->Second(nl->First(restHashAttrs)));
      if(!nl->Equal(oldAttribute, nl->First(rest))){
         return listutils::typeError("Second part of the "
                 "tupledescription of the stream without the last "
                 "attribute has to be the same as the tuple"
                 "description of the hash except for that the "
                 "attributenames carry an additional '_old.'");
      }
      rest = nl->Rest(rest);
      restHashAttrs = nl->Rest(restHashAttrs);
    }
  } else {
    // For insert and delete check whether tupledescription of the stream
    // without the last attribute is the same as the tupledescription
    // of the hash
    listn = nl->OneElemList(nl->First(rest));
    lastlistn = listn;
    rest = nl->Rest(rest);
    while (nl->ListLength(rest) > 1)
    {
      lastlistn = nl->Append(lastlistn,nl->First(rest));
      rest = nl->Rest(rest);
    }
    if(!nl->Equal(listn, hashAttrList)){
       return listutils::typeError("types of hash and tuple differ");
    }
  }


  // Test if attributename of the third argument exists as a name in the
  // attributlist of the streamtuples
  string attrname = nl->SymbolValue(nameOfKeyAttribute);
  ListExpr attrtype;
  int j = listutils::findAttribute(listn,attrname,attrtype);
  if(j==0){
    return listutils::typeError("key " + attrname + "not found in attrlist");
  }
  ListExpr hashKeyType = nl->Third(hashDescription);
  if(!nl->Equal(attrtype, hashKeyType)){
    return listutils::typeError("types for hash key and given key differ");
  }

  //Append the index of the attribute over which the hash is built to
  //the resultlist.
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                          nl->OneElemList(nl->IntAtom(j)),streamDescription);
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
  CcInt::inttype index;
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
        AttrToKey((Attribute*)keyAttr, key, hash->GetKeyType());
        hash->Append(key,oldTid);
        result = SetWord(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s,1));
      local.addr=0;
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
  CcInt::inttype index;
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
        AttrToKey((Attribute*)keyAttr, key, hash->GetKeyType());
        hash->Delete(key,oldTid);
        result = SetWord(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s,1));
      local.addr=0;
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
  CcInt::inttype index;
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
        AttrToKey((Attribute*)keyAttr, key, hash->GetKeyType());
        AttrToKey((Attribute*)oldKeyAttr, oldKey, hash->GetKeyType());
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
      local.addr=0;
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
6.6 Operator ~getFileInfo~

Returns a text object with statistical information on all files used by the
hash table.

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

ListExpr getFileInfoHashTypeMap(ListExpr args)
{

  if(nl->ListLength(args)!=1){
    return NList::typeError("1 arguiment expected.");
  }

  ListExpr btreeDescription = nl->First(args);

  if( nl->IsAtom(btreeDescription)
      || (nl->ListLength(btreeDescription) != 3)
    ) {
    return NList::typeError("1st argument is not a hash table.");
  }
  ListExpr hashSymbol = nl->First(btreeDescription);;
  if(    !nl->IsAtom(hashSymbol)
      || (nl->AtomType(hashSymbol) != SymbolType)
      || (nl->SymbolValue(hashSymbol) != Hash::BasicType())
    ){
    return NList::typeError("1st argument is not a hash table.");
  }
  return NList(FText::BasicType()).listExpr();
}

/*

6.6.2 ValueMapping of operator ~getFileInfo~

*/

int getFileInfoHashValueMap(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  FText* restext = (FText*)(result.addr);
  Hash* hash   = (Hash*)(args[0].addr);
  SmiStatResultType resVector(0);

  if ( (hash != 0) && hash->getFileStats(resVector) ){
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
const string getFileInfoHashSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(hash(tuple(x) ti) xi -> text)</text--->"
  "<text>getFileInfo( _ )</text--->"
  "<text>Retrieve statistical infomation on the file(s) used by the hash table "
  "instance.</text--->"
  "<text>query getFileInfo(plz_PLZ_hash)</text--->"
  ") )";


/*
6.7.4 Definition of operator ~getFileInfo~

*/
Operator getfileinfohash (
         "getFileInfo",              // name
         getFileInfoHashSpec,        // specification
         getFileInfoHashValueMap,    // value mapping
         Operator::SimpleSelect,     // trivial selection function
         getFileInfoHashTypeMap      // type mapping
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
    AddOperator(&getfileinfohash);

#else

    AddOperator(&createhash);
    AddOperator(&hashexactmatch);
    hashexactmatch.EnableProgress();
    AddOperator(&hashexactmatchs);
    AddOperator(&inserthash);
    AddOperator(&deletehash);
    AddOperator(&updatehash);
    AddOperator(&getfileinfohash);

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
