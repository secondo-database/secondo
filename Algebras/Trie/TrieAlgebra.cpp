/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen
Faculty of Mathematic and Computer Science,
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

*/

#include "VTrie2.h"
#include "InvertedFile.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "FTextAlgebra.h"
#include "Stream.h"
#include "Progress.h"


extern NestedList* nl;
extern QueryProcessor* qp;



namespace triealg{



/*
1 Type constructors



1.1 Type Constructor Trie

*/

typedef vtrie::VTrie<TupleId> TrieTypeAlg;
typedef vtrie::VTrieIterator<TupleId> TrieIteratorTypeAlg;



ListExpr TrieProperty(){
  return nl->TwoElemList(
             nl->FiveElemList(
                 nl->StringAtom("Signature"),
                 nl->StringAtom("Example Type List"),
                 nl->StringAtom("List Rep"),
                 nl->StringAtom("Example List"),
                 nl->StringAtom("Remarks")),
             nl->FiveElemList(
                nl->TextAtom(" -> SIMPLE"),
                nl->TextAtom("trie"),
                nl->TextAtom("()"),
                nl->TextAtom("( (a 1))"),
                nl->TextAtom("test type constructor"))
         );   
}


bool CheckTrie(ListExpr type, ListExpr& ErrorInfo){
   return listutils::isSymbol(type, TrieTypeAlg::BasicType());
}

ListExpr OutTrie(ListExpr typeInfo, Word value){
   return nl->TextAtom("A trie");
}

Word InTrie(ListExpr typeInfo, ListExpr value,
               int errorPos, ListExpr& errorInfo, bool& correct){
   Word w;
   w.addr = 0;
   correct = false;
   return w;
}

Word CreateTrie(const ListExpr typeInfo){
   Word  res;
   res.addr = new TrieTypeAlg();
   return res;
}

void DeleteTrie( const ListExpr typeInfo, Word& w ){
  TrieTypeAlg* t = (TrieTypeAlg*) w.addr;
  t->deleteFile();
  delete t;
  w.addr = 0;
}

bool OpenTrie( SmiRecord& valueRecord,
                 size_t& offset,
                 const ListExpr typeInfo,
                 Word& value ){
  SmiFileId fileid;
  valueRecord.Read( &fileid, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );
  SmiRecordId rid;
  valueRecord.Read( &rid, sizeof( SmiRecordId ), offset );
  offset += sizeof( SmiRecordId );
  TrieTypeAlg* tree = new TrieTypeAlg(fileid, rid);
  value.setAddr(tree);
  return true;
}


void CloseTrie( const ListExpr typeInfo, Word& w ){
  TrieTypeAlg* t = (TrieTypeAlg*) w.addr;
  delete t;
  w.addr = 0;
}

bool SaveTrie( SmiRecord& valueRecord,
               size_t& offset,
               const ListExpr typeInfo,
               Word& value ){
   TrieTypeAlg* t = static_cast<TrieTypeAlg*>(value.addr);
   SmiFileId fileId = t->getFileId();
   valueRecord.Write( &fileId, sizeof( SmiFileId ), offset );
   offset += sizeof( SmiFileId );
   SmiRecordId rootId = t->getRootId();
   valueRecord.Write(&rootId, sizeof(SmiRecordId), offset);
   offset += sizeof( SmiRecordId );
   return true;
}


Word CloneTrie(const ListExpr typeInfo, const Word& value){
  TrieTypeAlg* src = (TrieTypeAlg*) value.addr;
  return src->clone(); 
}

void* CastTrie( void* addr) {
   return (TrieTypeAlg*) addr;
}

int SizeOfTrie(){
  return sizeof(TrieTypeAlg);
}


TypeConstructor trietc( TrieTypeAlg::BasicType(),
                        TrieProperty,
                        OutTrie,
                        InTrie,
                        0,
                        0,
                        CreateTrie,
                        DeleteTrie,
                        OpenTrie,
                        SaveTrie,
                        CloseTrie,
                        CloneTrie,
                        CastTrie,
                        SizeOfTrie,
                        CheckTrie );


/*

1.2 Type Constructor InvFile

*/


ListExpr InvfileProperty(){
  return nl->TwoElemList(
             nl->FiveElemList(
                 nl->StringAtom("Signature"),
                 nl->StringAtom("Example Type List"),
                 nl->StringAtom("List Rep"),
                 nl->StringAtom("Example List"),
                 nl->StringAtom("Remarks")),
             nl->FiveElemList(
                nl->TextAtom(" -> SIMPLE"),
                nl->TextAtom("invfile"),
                nl->TextAtom("invfile"),
                nl->TextAtom("( (a 1))"),
                nl->TextAtom("test type constructor"))
         );   
}


bool CheckInvfile(ListExpr type, ListExpr& ErrorInfo){
   return InvertedFile::checkType(type);
}

ListExpr OutInvfile(ListExpr typeInfo, Word value){
   return nl->TextAtom("An invfile");
}

Word InInvfile(ListExpr typeInfo, ListExpr value,
               int errorPos, ListExpr& errorInfo, bool& correct){
   Word w;
   w.addr = 0;
   correct = false;
   return w;
}

Word CreateInvfile(const ListExpr typeInfo){
   Word  res;
   res.addr = new InvertedFile();
   return res;
}

void DeleteInvfile( const ListExpr typeInfo, Word& w ){
  InvertedFile* t = (InvertedFile*) w.addr;
  t->deleteFiles();
  delete t;
  w.addr = 0;
}

bool OpenInvfile( SmiRecord& valueRecord,
                 size_t& offset,
                 const ListExpr typeInfo,
                 Word& value ){
  SmiFileId triefileid;
  valueRecord.Read( &triefileid, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );
  SmiRecordId trierid;
  valueRecord.Read( &trierid, sizeof( SmiRecordId ), offset );
  offset += sizeof( SmiRecordId );
  SmiFileId listfileid;
  valueRecord.Read( &listfileid, sizeof( SmiFileId ), offset );
  offset += sizeof(SmiFileId);  
  InvertedFile* invFile = new InvertedFile(triefileid, trierid, listfileid);
  value.setAddr(invFile);
  return true;
}


void CloseInvfile( const ListExpr typeInfo, Word& w ){
  InvertedFile* t = (InvertedFile*) w.addr;
  delete t;
  w.addr = 0;
}

bool SaveInvfile( SmiRecord& valueRecord,
               size_t& offset,
               const ListExpr typeInfo,
               Word& value ){
   InvertedFile* t = static_cast<InvertedFile*>(value.addr);
   SmiFileId triefileId = t->getFileId();
   valueRecord.Write( &triefileId, sizeof( SmiFileId ), offset );
   offset += sizeof( SmiFileId );
   SmiRecordId rootId = t->getRootId();
   valueRecord.Write(&rootId, sizeof(SmiRecordId), offset);
   offset += sizeof( SmiRecordId );
   SmiFileId listFileId = t->getListFileId();
   valueRecord.Write(&listFileId, sizeof(SmiFileId), offset);
   offset += sizeof(SmiFileId);
   return true;
}


Word CloneInvfile(const ListExpr typeInfo, const Word& value){
  InvertedFile* src = (InvertedFile*) value.addr;
  return src->clone(); 
}

void* CastInvfile( void* addr) {
   return (InvertedFile*) addr;
}

int SizeOfInvfile(){
  return sizeof(InvertedFile);
}


TypeConstructor invfiletc( InvertedFile::BasicType(),
                        InvfileProperty,
                        OutInvfile,
                        InInvfile,
                        0,
                        0,
                        CreateInvfile,
                        DeleteInvfile,
                        OpenInvfile,
                        SaveInvfile,
                        CloseInvfile,
                        CloneInvfile,
                        CastInvfile,
                        SizeOfInvfile,
                        CheckInvfile );


/*
2 Operator


2.1 Operator ~createmptytrie~

This operator creates a new empty trie.


2.1.1 Type Mapping for createemptytrie


 

*/

ListExpr createemptytrieTM(ListExpr args){
  if(!nl->IsEmpty(args)){
    return listutils::typeError("no arguments expected");
  }
  return nl->SymbolAtom(TrieTypeAlg::BasicType());
}


int createemptytrieVM(Word* args, Word& result, int message,
                   Word& local, Supplier s){
   result = qp->ResultStorage(s);
   return 0;
}

const string createemptytrieSpec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text> -> trie </text--->"
    "<text> createemptytrie()</text--->"
    "<text> creates an empty trie structure<text--->"
    "<text>query createemptytrie()</text--->"
    "<text></text--->"
    ") )";


Operator createemptytrie (
         "createemptytrie",            // name
          createemptytrieSpec,          // specification
          createemptytrieVM,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          createemptytrieTM);



/*
2.2 Operator ~insert2trie~

2.2.1 Type Mapping

*/

ListExpr insert2trieTM(ListExpr args){
  string err = "trie x string x tupleid required";
  if(!nl->HasLength(args,3)){
    return listutils::typeError(err);
  }
  if(!TrieTypeAlg::checkType(nl->First(args)) ||
     !CcString::checkType(nl->Second(args)) ||
     !TupleIdentifier::checkType(nl->Third(args))){
     return listutils::typeError(err);
  }
  return nl->SymbolAtom(CcBool::BasicType());
}


int insert2trieVM(Word* args, Word& result, int message,
                   Word& local, Supplier s){

   TrieTypeAlg* trie = (TrieTypeAlg*) args[0].addr;
   CcString* str = (CcString*) args[1].addr;
   TupleIdentifier* tid = (TupleIdentifier*) args[2].addr;
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   if(!str->IsDefined() || ! tid->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   res->Set(true,true);

   string str2 = str->GetValue();
   TupleId t = tid->GetTid();
   trie->insert(str2,t);

   qp->SetModified(qp->GetSon(s,0));

   return 0;
}


const string insert2trieSpec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text> trie x string x TID </text--->"
    "<text> insert2trie(_,_,_)</text--->"
    "<text> inserts or overwrites an entry in a trie<text--->"
    "<text>query  inserttrie(createemptytrie(), \"Hello\",int2tid(1))</text--->"
    "<text></text--->"
    ") )";

Operator insert2trie (
         "insert2trie",            // name
          insert2trieSpec,          // specification
          insert2trieVM,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          insert2trieTM);




/*
2.3 Operator search

2.3.1 Type Mapping

*/
ListExpr searchtrieTM(ListExpr args){
  string err = "trie x string expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  if(!TrieTypeAlg::checkType(nl->First(args)) ||
     !CcString::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return nl->SymbolAtom(TupleIdentifier::BasicType()); 
}

int searchtrieVM(Word* args, Word& result, int message,
                    Word& local, Supplier s){

    TrieTypeAlg* trie = (TrieTypeAlg*) args[0].addr;
    CcString* pattern = (CcString*) args[1].addr;
    result = qp->ResultStorage(s);
    TupleIdentifier* tid = (TupleIdentifier*) result.addr;
    if(!pattern->IsDefined()){
       tid->SetDefined(false);
    } else {
       tid->Set(true,trie->search(pattern->GetValue()));
    }
    return 0;  

}


const string searchtrieSpec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text> trie x string -> TID </text--->"
    "<text> searchtrie(_,_) </text--->"
    "<text> retrieves a stored information in a trie<text--->"
    "<text>query  searchtrie(createemptytrie(), \"hello\")</text--->"
    "<text></text--->"
    ") )";

Operator searchtrie (
         "searchtrie" ,           // name
          searchtrieSpec,          // specification
          searchtrieVM,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          searchtrieTM);

/*
2.3 Operator contains 

2.3.1 Type Mapping

*/
ListExpr containsTM(ListExpr args){
  string err = "trie x string expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  if(!TrieTypeAlg::checkType(nl->First(args)) ||
     !CcString::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return nl->SymbolAtom(CcBool::BasicType()); 
}


template<bool acceptPrefix>
int containsVM(Word* args, Word& result, int message,
                    Word& local, Supplier s){

    TrieTypeAlg* trie = (TrieTypeAlg*) args[0].addr;
    CcString* pattern = (CcString*) args[1].addr;
    result = qp->ResultStorage(s);
    CcBool* res = (CcBool*) result.addr;
    if(!pattern->IsDefined()){
       res->SetDefined(false);
    } else {
       res->Set(true,trie->contains(pattern->GetValue(),acceptPrefix));
    }
    return 0;  

}


const string containsSpec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text> trie x string -> bool </text--->"
    "<text> _ contains _  </text--->"
    "<text> ichecks whether a trie contains a given string<text--->"
    "<text>query  createemptytrie() contains \"hello\"</text--->"
    "<text></text--->"
    ") )";


const string containsPrefixSpec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text> trie x string -> bool </text--->"
    "<text> _ containsPrefix _  </text--->"
    "<text> checks whether a trie contains a given prefix<text--->"
    "<text>query  createemptytrie() containsPrefix \"hello\"</text--->"
    "<text></text--->"
    ") )";

Operator contains (
         "contains" ,           // name
          containsSpec,          // specification
          containsVM<false>,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          containsTM);

Operator containsPrefix (
         "containsPrefix" ,           // name
          containsPrefixSpec,          // specification
          containsVM<true>,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          containsTM);
/*
2.4 Operator trieEntries

This operator returns all entries within a trie with a given prefix

*/
ListExpr trieEntriesTM(ListExpr args){
  string err = "trie x string  expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  if(!TrieTypeAlg::checkType(nl->First(args)) ||
     !CcString::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  
  ListExpr res =  nl->TwoElemList( 
                      nl->SymbolAtom(Stream<CcString>::BasicType()),
                      nl->SymbolAtom(CcString::BasicType()));

  return res;
}



int trieEntriesVM(Word* args, Word& result, int message,
                  Word& local, Supplier s){

  TrieIteratorTypeAlg* li = (TrieIteratorTypeAlg*) local.addr;
  switch(message){
     case OPEN : {
                   if(li){
                     delete li;
                     local.addr=0;
                   }
                   TrieTypeAlg* trie = (TrieTypeAlg*)args[0].addr;
                   CcString* str = (CcString*) args[1].addr;
                   if(str->IsDefined() ){
                        local.addr = trie->getEntries(str->GetValue());
                   }
                   return 0;
                 }
      case REQUEST : {
                  if(!li){
                    return CANCEL;
                  }
                  string r;
                  TupleId id;
                  bool ok = li->next(r, id);
                  if(ok){
                     result.addr = new CcString(true,r);
                  } else {
                     result.addr=0;
                  }
                  return result.addr?YIELD:CANCEL;
               }
       case CLOSE: {
                   if(li){
                       delete li;
                       local.addr = 0;
                   }
                }

  }
  return 0;
}


const string trieEntriesSpec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text> trie x string  -> stream(string) </text--->"
    "<text> _ trieEntries[_]  </text--->"
    "<text>returns the elements stored in a trie starting with a given prefix"
    "</text--->"
    "<text>query  tr trieEntries[\"sec\"] count</text--->"
    "<text></text--->"
    ") )";

Operator trieEntries (
         "trieEntries" ,           // name
          trieEntriesSpec,          // specification
          trieEntriesVM,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          trieEntriesTM);




/*
2.5 Operator createInvFile

2.5.1 Type Mapping

Signature is stream(tuple) x a1 x a2 -> invfile

a1 must be of type text
a2 must be of type tid

*/
ListExpr createInvFileTM(ListExpr args){
  string err = "stream(tuple) x a_i x a_j expected";
  if(!nl->HasLength(args,3)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
    return listutils::typeError(err + " (first arg is not a tuple stream)");
  }
  if(!listutils::isSymbol(nl->Second(args)) ||
     !listutils::isSymbol(nl->Third(args))){
    return listutils::typeError(err + 
                  " (one of the attribute names is not valid)");
  }
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  string a1 = nl->SymbolValue(nl->Second(args));
  string a2 = nl->SymbolValue(nl->Third(args));
  ListExpr t1;
  ListExpr t2;
  int i1 = listutils::findAttribute(attrList,a1,t1);
  if(i1==0){
    return listutils::typeError("Attribute " + a1 + 
                                " not known in the tuple");
  }
  int i2 = listutils::findAttribute(attrList,a2,t2);
  if(i2==0){
    return listutils::typeError("Attribute " + a2 + 
                                " not known in the tuple");
  }

  if(!FText::checkType(t1)){
    return listutils::typeError(a1 + " not of type text");
  } 

  if(!TupleIdentifier::checkType(t2)){
    return listutils::typeError(a2 + " not of type " + 
                             TupleIdentifier::BasicType());
  }

  ListExpr appendList = nl->TwoElemList( nl->IntAtom(i1-1),
                                         nl->IntAtom(i2-1));

 
  return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                            appendList,
                            nl->SymbolAtom(InvertedFile::BasicType()));

}


/*
2.5.2 Value Mapping

*/

int createInvFileVM(Word* args, Word& result, int message,
                  Word& local, Supplier s){

  switch(message){

    case OPEN:
    case CLOSE:
    case REQUEST: {      
       Stream<Tuple> stream(args[0]);
       int textIndex = ((CcInt*)args[3].addr)->GetValue();
       int tidIndex  = ((CcInt*)args[4].addr)->GetValue();
       result = qp->ResultStorage(s);
       InvertedFile* invFile = (InvertedFile*) result.addr;

       stream.open();
       Tuple* tuple;
     
       size_t maxMem = qp->GetMemorySize(s) * 1024*1024;

       size_t trieCacheSize = maxMem / 20;
       if(trieCacheSize < 4096){
          trieCacheSize = 4096;
       }
       size_t invFileCacheSize;
       if(trieCacheSize + 4096 > maxMem){
            invFileCacheSize = 4096;
       } else {
            invFileCacheSize = maxMem - trieCacheSize;
       }


       appendcache::RecordAppendCache* cache = 
                            invFile->createAppendCache(invFileCacheSize);

       TrieNodeCacheType* trieCache = 
                            invFile->createTrieCache(trieCacheSize);

       while( (tuple = stream.request())!=0){
          FText* text = (FText*) tuple->GetAttribute(textIndex);
          TupleIdentifier* tid = (TupleIdentifier*) 
                                  tuple->GetAttribute(tidIndex);

          if(text->IsDefined() && tid->IsDefined()){
             invFile->insertText(tid->GetTid() , text->GetValue(),
                                 cache, trieCache);
          }
          tuple->DeleteIfAllowed();
       }   
       stream.close();
       delete cache;
       delete trieCache;
       return 0;
     }
   case REQUESTPROGRESS: {
     ProgressInfo p1;
     ProgressInfo* pRes;

     pRes = (ProgressInfo*) result.addr;

     if ( qp->RequestProgress(args[0].addr, &p1) ) {    
        pRes->Copy(p1);
        return YIELD;
     } else {
        return CANCEL;
      }    
   }
   case CLOSEPROGRESS: {
      return 0;
   }  
  }
  return 0;
}


/*
2.5.3 Specification

*/

const string createInvFileSpec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text> stream(tuple(...) x a_i x a_j -> invfile </text--->"
    "<text> _ createInvFile[_,_]  </text--->"
    "<text>creates an inverted file from a stream. "
    " a_i must be of type text, a_j must be of type tid."
    "</text--->"
    "<text>query SEC2OPERATORINFO feed addid "
    "createInvFile[Signature, TID] </text--->"
    "<text></text--->"
    ") )";


Operator createInvFile (
         "createInvFile" ,           // name
          createInvFileSpec,          // specification
          createInvFileVM,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          createInvFileTM);


/*
2.6 Operator searchWord

2.6.1 Type Mapping

Signature : invfile x string -> 
            stream(tuple([Tid : tid, WordPos : int, CharPos : int]))

*/
ListExpr searchWordTM(ListExpr args){
   string err = "invfile x string expected" ;
   if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
   }
   if(!InvertedFile::checkType(nl->First(args)) ||
      !CcString::checkType(nl->Second(args))){
     return listutils::typeError(err);
   }   
   ListExpr attrList = nl->ThreeElemList(
                    nl->TwoElemList( nl->SymbolAtom("Tid"), 
                    nl->SymbolAtom(TupleIdentifier::BasicType())),
                    nl->TwoElemList( nl->SymbolAtom("WordPos"),  
                                     nl->SymbolAtom(CcInt::BasicType())),
                    nl->TwoElemList( nl->SymbolAtom("CharPos"),  
                                     nl->SymbolAtom(CcInt::BasicType()))
                   );
   return nl->TwoElemList( nl->SymbolAtom(Stream<Tuple>::BasicType()),
                 nl->TwoElemList( nl->SymbolAtom(Tuple::BasicType()),
                     attrList));                                
}


/*
2.6.1 LocalInfo

*/

class searchWordLocalInfo{
   public:
      searchWordLocalInfo( InvertedFile* inv, string word, 
                           ListExpr typeList, size_t mem){
         tt = new TupleType(typeList);
         it = inv->getExactIterator(word, mem);
      } 
      ~searchWordLocalInfo(){
         tt->DeleteIfAllowed();
         delete it;
      }

      Tuple* next(){
         TupleId id;
         wordPosType wp;
         charPosType cp;
         if(it->next(id,wp,cp)){
            Tuple* res = new Tuple(tt);
            res->PutAttribute(0, new TupleIdentifier(true,id));
            res->PutAttribute(1, new CcInt(true,wp));
            res->PutAttribute(2, new CcInt(true, cp));
            return res;
         }
         return 0;
      }


   private:
      TupleType* tt;
      InvertedFile::exactIterator* it; 
};

/*
2.6.2 Value Mapping

*/

int searchWordVM(Word* args, Word& result, int message,
                  Word& local, Supplier s){

   searchWordLocalInfo* li = (searchWordLocalInfo*) local.addr;
   switch(message){
      case OPEN : {
                   if(li){
                      delete li;
                   }
                   InvertedFile* iv = (InvertedFile*) args[0].addr;
                   CcString* cstr = (CcString*) args[1].addr;
                   ListExpr type = nl->Second(GetTupleResultType(s));
                   size_t memBuffer = 4096;
                   if(cstr->IsDefined()){
                       local.addr = new searchWordLocalInfo(iv, 
                                         cstr->GetValue(), type, memBuffer);
                   }
                   return 0;
                   }
     case REQUEST : {
                      if(!li){
                        return CANCEL;
                      }
                      result.addr=li->next();
                      return result.addr?YIELD:CANCEL;
                    }  
     case CLOSE  : {
                     if(li){
                       delete li;
                       local.addr = 0;
                     }
                   }                 

   }
   return -1;
}

/*
2.6.3 Specification

*/

const string searchWordSpec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text> invfile x string -> stream(tuple([TID : tid, "
    "WordPos : int, CharPos : int)) </text--->"
    "<text> _ searchWord [_]  </text--->"
    "<text>Retrives the information stored in an inverted file "
    " for the give string"
    "</text--->"
    "<text>query SEC2OPERATORINFO feed addid createInvFile[Signature, TID] "
    " searchWord[\"string\"] count"
     "</text--->"
    "<text></text--->"
    ") )";


Operator searchWord (
         "searchWord" ,           // name
          searchWordSpec,          // specification
          searchWordVM,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          searchWordTM);


/*
2.6 Operator searchPrefix

2.6.1 Type Mapping

Signature : invfile x string -> 
          stream(tuple([Word : text, Tid : tid, WordPos : int, CharPos : int]))

*/
ListExpr searchPrefixTM(ListExpr args){
   string err = "invfile x string expected" ;
   if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
   }
   if(!InvertedFile::checkType(nl->First(args)) ||
      !CcString::checkType(nl->Second(args))){
     return listutils::typeError(err);
   }   
   ListExpr attrList = nl->FourElemList(
                     nl->TwoElemList( nl->SymbolAtom("Word"), 
                                 nl->SymbolAtom(FText::BasicType())),
                     nl->TwoElemList( nl->SymbolAtom("Tid"), 
                                 nl->SymbolAtom(TupleIdentifier::BasicType())),
                     nl->TwoElemList( nl->SymbolAtom("WordPos"),  
                                 nl->SymbolAtom(CcInt::BasicType())),
                     nl->TwoElemList( nl->SymbolAtom("CharPos"),  
                                 nl->SymbolAtom(CcInt::BasicType()))
                       );
   return nl->TwoElemList( nl->SymbolAtom(Stream<Tuple>::BasicType()),
                 nl->TwoElemList( nl->SymbolAtom(Tuple::BasicType()),
                     attrList));                                
}


/*
2.6.1 LocalInfo

*/

class searchPrefixLocalInfo{
   public:
      searchPrefixLocalInfo( InvertedFile* inv, string word, ListExpr typeList){
         tt = new TupleType(typeList);
         it = inv->getPrefixIterator(word);
      } 
      ~searchPrefixLocalInfo(){
         tt->DeleteIfAllowed();
         delete it;
      }

      Tuple* next(){
         string word;
         TupleId id;
         wordPosType wp;
         charPosType cp;
         if(it->next(word,id,wp,cp)){
            Tuple* res = new Tuple(tt);
            res->PutAttribute(0, new FText(true,word));
            res->PutAttribute(1, new TupleIdentifier(true,id));
            res->PutAttribute(2, new CcInt(true,wp));
            res->PutAttribute(3, new CcInt(true, cp));
            return res;
         }
         return 0;
      }


   private:
      TupleType* tt;
      InvertedFile::prefixIterator* it;  
};

/*
2.6.2 Value Mapping

*/

int searchPrefixVM(Word* args, Word& result, int message,
                  Word& local, Supplier s){

   searchPrefixLocalInfo* li = (searchPrefixLocalInfo*) local.addr;
   switch(message){
      case OPEN : {
                   if(li){
                      delete li;
                   }
                   InvertedFile* iv = (InvertedFile*) args[0].addr;
                   CcString* cstr = (CcString*) args[1].addr;
                   ListExpr type = nl->Second(GetTupleResultType(s));
                   if(cstr->IsDefined()){
                       local.addr = new searchPrefixLocalInfo(iv, 
                                             cstr->GetValue(), type);
                   }
                   return 0;
                   }
     case REQUEST : {
                      if(!li){
                        return CANCEL;
                      }
                      result.addr=li->next();
                      return result.addr?YIELD:CANCEL;
                    }  
     case CLOSE  : {
                     if(li){
                       delete li;
                       local.addr = 0;
                     }
                   }                 

   }
   return -1;
}

/*
2.6.3 Specification

*/

const string searchPrefixSpec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text> invfile x string -> stream(tuple([ Word : string, TID : tid, "
    "WordPos : int, CharPos : int)) </text--->"
    "<text> _ searchPrefix [_]  </text--->"
    "<text>Retrieves the information stored in an inverted file "
    " for the given prefix"
    "</text--->"
    "<text>query SEC2OPERATORINFO feed addid createInvFile[Signature, TID] "
    " searchPrefix[\"stri\"] count"
     "</text--->"
    "<text></text--->"
    ") )";


Operator searchPrefix (
         "searchPrefix" ,           // name
          searchPrefixSpec,          // specification
          searchPrefixVM,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          searchPrefixTM);

/*
2.7 Operator getFileInfo

2.7.1 Type Mapping

The Signature is {trie, invfile} -> text 

*/
ListExpr getFileInfoTM(ListExpr args){
  string err = " trie or invfile expected " ;
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  ListExpr arg = nl->First(args);
  if(!TrieTypeAlg::checkType(arg) &&
     !InvertedFile::checkType(arg)){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<FText>();

}

/*
2.7.2 ValueMapping

*/
template<class T>
int getFileInfoVM1(Word* args, Word& result, int message,
                  Word& local, Supplier s){

   T* t = (T*) args[0].addr;
   result = qp->ResultStorage(s);
   FText* res = (FText*) result.addr;
   SmiStatResultType r;
   t->getFileInfo(r);
   stringstream ss;
   for(unsigned int i=0; i< r.size() ; i++){
      ss << r[i].first << " : " << r[i].second << endl;
   }
   res->Set(true,ss.str());
   return 0;
}

/*
2.7.3 Value Mapping Array and Selection Function

*/

ValueMapping getFileInfoVM[] = {
  getFileInfoVM1<TrieTypeAlg >,
  getFileInfoVM1<InvertedFile>
};

int getFileInfoSelect(ListExpr args){
  return TrieTypeAlg::checkType(nl->First(args))?0:1;
}

/*
2.7.4 Specification

*/
OperatorSpec getFileInfoSpec(
           "{trie|invfile} -> text",
           "getFileInfo(_)",
           "Returns information about the underlying files of"
           " an index structure",
           " query getFileInfo(iv1) " );

/*
2.7.5 Operator Instance

*/  


Operator getFileInfo
  (
  "getFileInfo",             //name
   getFileInfoSpec.getStr(),         //specification
   2,                           // no of VM functions
   getFileInfoVM,        //value mapping
   getFileInfoSelect,   //trivial selection function
   getFileInfoTM        //type mapping
  );



/*
2.8 wordCount

This operator returns the amount of a certain word within the
whole indexed relation.

2.8.1 typeMapping

Signature : invfile x string -> int

*/

ListExpr wordCountTM(ListExpr args){
  string err ="invfile x string expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  if(!InvertedFile::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(!CcString::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcInt>();  
}


/*
2.8.2 Value Mapping

*/

int wordCountVM(Word* args, Word& result, int message,
                  Word& local, Supplier s){

   InvertedFile* iv = (InvertedFile*) args[0].addr;
   CcString* str = (CcString*) args[1].addr;   
   result = qp->ResultStorage(s);
   CcInt* res = (CcInt*) result.addr;
   if(!str->IsDefined()){
      res->SetDefined(false);
   } else {
     res->Set(true, iv->wordCount(str->GetValue()));
  }
  return 0;
}


/*
2.8.3 Specification

*/

OperatorSpec wordCountSpec(
           "  invfile x string -> int",
           " _ wordCount[_]",
           " Returns how ofter a word is indexed.",
           " query iv wordCount[\"secondo\" " );

/*
2.8.4 Operator Instance

*/

Operator wordCount (
         "wordCount" ,           // name
          wordCountSpec.getStr(),          // specification
          wordCountVM,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          wordCountTM);


/*
2.9 prefixCount

2.9.1 Type Mapping

Signature :  invfile x string -> stream(tuple([Word : text, Count : int]))

*/
ListExpr prefixCountTM(ListExpr args){
  string err ="invfile x string expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  if(!InvertedFile::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(!CcString::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }

  ListExpr attrList = nl->TwoElemList(
                             nl->TwoElemList(
                                    nl->SymbolAtom("Word"),
                                    listutils::basicSymbol<FText>()),
                             nl->TwoElemList(
                                    nl->SymbolAtom("Count"),
                                    listutils::basicSymbol<CcInt>()));

  return  nl->TwoElemList(  
                 listutils::basicSymbol<Stream<Tuple> >(),
                 nl->TwoElemList(
                      listutils::basicSymbol<Tuple>(),
                      attrList)); 
                           
} 

/*
2.9.2 Value Mapping

*/

class prefixCountLI{
  public:
     prefixCountLI(InvertedFile* iv, CcString* str, ListExpr type): tt(0),it(0){
        if(str->IsDefined()){
           it = iv->getCountPrefixIterator(str->GetValue());
           tt = new TupleType(type);
        }
     }

    ~prefixCountLI(){
         if(it){
           delete it;
         } 
         if(tt){
           tt->DeleteIfAllowed();;
         }
     }

     Tuple* next(){
        string s;
        size_t c = 0;
        if(!it->next(s,c)){
           return 0;
        }
        Tuple* res = new Tuple(tt);
        res->PutAttribute(0, new FText(true,s));
        res->PutAttribute(1, new CcInt(true,c));
        return res; 
     }

  private:
     TupleType* tt;
     InvertedFile::countPrefixIterator* it;    

};

int prefixCountVM(Word* args, Word& result, int message,
                  Word& local, Supplier s){


   prefixCountLI* li = (prefixCountLI*) local.addr;
   switch(message){
      case OPEN : {
                      if(li){
                         delete li;
                      }
                      InvertedFile* iv = (InvertedFile*) args[0].addr;
                      CcString* str = (CcString*) args[1].addr;
                      local.addr = new prefixCountLI(iv,str, 
                                   nl->Second(GetTupleResultType(s)));
                      return 0;
                  } 
     case REQUEST : {
                      if(!li){
                        return CANCEL;
                      }
                      result.addr = li->next();
                      return result.addr?YIELD:CANCEL;
                   }

     case CLOSE : {
                      if(li){
                         delete li;
                         local.addr = 0;
                      }
                      return 0;
                  }
   }
   return -1;
}

/*
9.2.3 Specification

*/

OperatorSpec prefixCountSpec(
           "  invfile x string -> stream(tuple([Word : text, Count : int]))",
           " _ prefixCount[_]",
           " Returns how often word starting with a prefix are indexed.",
           " query iv prefixCountCount[\"secondo\" ] tconsume " );


/*
9.2.4 Operator instance

*/

Operator prefixCount (
         "prefixCount" ,           // name
          prefixCountSpec.getStr(),          // specification
          prefixCountVM,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          prefixCountTM);

} // end of namespace triealg




class TrieAlgebra : public Algebra {
  public:
     TrieAlgebra() : Algebra() {
     AddTypeConstructor( &triealg::trietc );
     AddTypeConstructor( &triealg::invfiletc );

     AddOperator(&triealg::createemptytrie);
     AddOperator(&triealg::insert2trie);
     AddOperator(&triealg::searchtrie);
     AddOperator(&triealg::contains);
     AddOperator(&triealg::containsPrefix);
     AddOperator(&triealg::trieEntries);
     
     AddOperator(&triealg::createInvFile);
     triealg::createInvFile.SetUsesMemory();

     AddOperator(&triealg::getFileInfo);
     AddOperator(&triealg::wordCount);
     AddOperator(&triealg::prefixCount);
     AddOperator(&triealg::searchWord);
     AddOperator(&triealg::searchPrefix);


#ifdef USE_PROGRESS
     triealg::createInvFile.EnableProgress();
#endif
   }
};


extern "C"
Algebra*
  InitializeTrieAlgebra( NestedList* nlRef, QueryProcessor* qpRef ) {
     nl = nlRef;
     qp = qpRef;
     return (new TrieAlgebra);
}



