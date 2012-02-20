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

#include "Trie.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "FTextAlgebra.h"
#include "Stream.h"


extern NestedList* nl;
extern QueryProcessor* qp;


namespace trie{



/*
1 Type constructors



1.1 Type Constructor Trie

*/


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
                nl->TextAtom("trie"),
                nl->TextAtom("( (a 1))"),
                nl->TextAtom("test type constructor"))
         );   
}


bool CheckTrie(ListExpr type, ListExpr& ErrorInfo){
   return listutils::isSymbol(type, Trie::BasicType());
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
   res.addr = new Trie();
   return res;
}

void DeleteTrie( const ListExpr typeInfo, Word& w ){
  Trie* t = (Trie*) w.addr;
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
  Trie* tree = new Trie(fileid, rid);
  value.setAddr(tree);
  return true;
}


void CloseTrie( const ListExpr typeInfo, Word& w ){
  Trie* t = (Trie*) w.addr;
  delete t;
  w.addr = 0;
}

bool SaveTrie( SmiRecord& valueRecord,
               size_t& offset,
               const ListExpr typeInfo,
               Word& value ){
   Trie* t = static_cast<Trie*>(value.addr);
   SmiFileId fileId = t->getFileId();
   valueRecord.Write( &fileId, sizeof( SmiFileId ), offset );
   offset += sizeof( SmiFileId );
   SmiRecordId rootId = t->getRootId();
   valueRecord.Write(&rootId, sizeof(SmiRecordId), offset);
   offset += sizeof( SmiRecordId );
   return true;
}


Word CloneTrie(const ListExpr typeInfo, const Word& value){
  Trie* src = (Trie*) value.addr;
  return src->clone(); 
}

void* CastTrie( void* addr) {
   return (Trie*) addr;
}

int SizeOfTrie(){
  return sizeof(Trie);
}


TypeConstructor trietc( Trie::BasicType(),
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
2 Operator


2.1 Operator ~createmptytrie~

This operator creates a new empty trie.


2.1.1 Type Mapping for createemptytrie


 

*/

ListExpr createemptytrieTM(ListExpr args){
  if(!nl->IsEmpty(args)){
    return listutils::typeError("no arguments expected");
  }
  return nl->SymbolAtom(Trie::BasicType());
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
  if(!Trie::checkType(nl->First(args)) ||
     !CcString::checkType(nl->Second(args)) ||
     !TupleIdentifier::checkType(nl->Third(args))){
     return listutils::typeError(err);
  }
  return nl->SymbolAtom(CcBool::BasicType());
}


int insert2trieVM(Word* args, Word& result, int message,
                   Word& local, Supplier s){

   Trie* trie = (Trie*) args[0].addr;
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
  if(!Trie::checkType(nl->First(args)) ||
     !CcString::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return nl->SymbolAtom(TupleIdentifier::BasicType()); 
}

int searchtrieVM(Word* args, Word& result, int message,
                    Word& local, Supplier s){

    Trie* trie = (Trie*) args[0].addr;
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
  if(!Trie::checkType(nl->First(args)) ||
     !CcString::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return nl->SymbolAtom(CcBool::BasicType()); 
}


template<bool acceptPrefix>
int containsVM(Word* args, Word& result, int message,
                    Word& local, Supplier s){

    Trie* trie = (Trie*) args[0].addr;
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
  if(!Trie::checkType(nl->First(args)) ||
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

  TrieIterator<TupleId>* li = (TrieIterator<TupleId>*) local.addr;
  switch(message){
     case OPEN : {
                   if(li){
                     delete li;
                     local.addr=0;
                   }
                   Trie* trie = (Trie*)args[0].addr;
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
                  bool ok = li->next(r);
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

} // end of namespace trie




class TrieAlgebra : public Algebra {
  public:
   TrieAlgebra() : Algebra() {
     AddTypeConstructor( &trie::trietc );

     AddOperator(&trie::createemptytrie);
     AddOperator(&trie::insert2trie);
     AddOperator(&trie::searchtrie);
     AddOperator(&trie::contains);
     AddOperator(&trie::containsPrefix);
     AddOperator(&trie::trieEntries);
   }
};


extern "C"
Algebra*
  InitializeTrieAlgebra( NestedList* nlRef, QueryProcessor* qpRef ) {
     nl = nlRef;
     qp = qpRef;
     return (new TrieAlgebra);
}



