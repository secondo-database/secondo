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
  bool ignoreCase;
  valueRecord.Read(&ignoreCase, sizeof(bool), offset);
  offset += sizeof(bool);
  uint32_t minWordLength;
  valueRecord.Read(&minWordLength, sizeof(uint32_t), offset);
  offset += sizeof(uint32_t);
  SmiRecordId stopWordsId;
  valueRecord.Read(&stopWordsId, sizeof(SmiRecordId), offset);
  offset += sizeof(SmiRecordId);
  InvertedFile* invFile = new InvertedFile(triefileid, trierid, listfileid, 
                                      ignoreCase, minWordLength, stopWordsId);
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
   bool ignoreCase = t->getIgnoreCase(); 
   valueRecord.Write(&ignoreCase, sizeof(bool),offset);
   offset += sizeof(bool);
   uint32_t minWordLength = t->getMinWordLength();
   valueRecord.Write(&minWordLength, sizeof(uint32_t), offset);
   offset += sizeof(uint32_t);
   SmiRecordId stopWordsId = t->getStopWordsId();
   valueRecord.Write(&stopWordsId, sizeof(SmiRecordId), offset);
   offset += sizeof(SmiRecordId); 
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
2.5 Operator createInvFile

2.5.1 Type Mapping

Signature is stream(tuple) x a1 x a2 -> invfile

a1 must be of type text
a2 must be of type tid

*/
ListExpr createInvFileTM(ListExpr args){
  string err = "stream(tuple) x a_i x a_j [ x bool x int x text ]expected";
  if(!nl->HasLength(args,3) && !nl->HasLength(args,6)){
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

  
  if(nl->HasLength(args,3)){
     ListExpr defaultParamList = nl->ThreeElemList(
                                nl->BoolAtom(false),
                                nl->IntAtom(1),
                                nl->TextAtom(""));
      appendList = listutils::concat(defaultParamList, appendList);
  } else {
       ListExpr ignoreCase = nl->Fourth(args);
       ListExpr minLength = nl->Fifth(args);
       ListExpr stopWords = nl->Sixth(args);
       if(!CcBool::checkType(ignoreCase) ||
          !CcInt::checkType(minLength) ||
          !FText::checkType(stopWords)){
          return listutils::typeError(err);
       }
  }
 
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
       int textIndex = ((CcInt*)args[6].addr)->GetValue();
       int tidIndex  = ((CcInt*)args[7].addr)->GetValue();
       result = qp->ResultStorage(s);
       InvertedFile* invFile = (InvertedFile*) result.addr;

       bool ignoreCase = false;
       CcBool* ic = (CcBool*) args[3].addr;
       if(ic->IsDefined()){
          ignoreCase = ic->GetValue();
       }
       int minWL = 1;
       CcInt* wl = (CcInt*) args[4].addr;
       if(wl->IsDefined() && wl->GetValue()>0){
           minWL = wl->GetValue();
       }
       string stopWords = "";
       FText* sw = (FText*) args[5].addr;
       if(sw->IsDefined()){
         stopWords = sw->GetValue();
       }

       invFile->setParams(ignoreCase, minWL, stopWords);

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
    "(<text> stream(tuple(...) x a_i x a_j "
    "[ignoreCase, minWordLength, stopWords]-> invfile </text--->"
    "<text> _ createInvFile[_, _, _, _ , _] </text--->"
    "<text>creates an inverted file from a stream. "
    " a_i must be of type text, a_j must be of type tid."
    " The last three arguments are optionally."
    " If ignoreCase is set to true, upper and lower case is ignored."
    " minWordLength is of type int and descibes the minimum word length"
    " for indexing (default 1)."
    "Stopwords is a text containing words which not should be indexed."
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
  string err = " invfile expected " ;
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  ListExpr arg = nl->First(args);
  if(!InvertedFile::checkType(arg)){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<FText>();

}

/*
2.7.2 ValueMapping

*/
template<class T>
int getFileInfoVM(Word* args, Word& result, int message,
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
2.7.4 Specification

*/
OperatorSpec getFileInfoSpec(
           "{invfile} -> text",
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
   getFileInfoVM<InvertedFile>,        //value mapping
   Operator::SimpleSelect,   //trivial selection function
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


/*
9.3 Operator ~getInvFileSeparators~

Returns all characters used for tokenize the string within the 
InvFile data type.

*/
ListExpr getInvFileSeparatorsTM(ListExpr args){
  if(!nl->IsEmpty(args)){
    return listutils::typeError("no arguments expected");
  }
  return listutils::basicSymbol<CcString>();
}

int getInvFileSeparatorsVM(Word* args, Word& result, int message,
                  Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcString* res = (CcString*) result.addr;
   res->Set(true, InvertedFile::getSeparatorsString());
   return 0;
}

OperatorSpec getInvFileSeparatorsSpec(
           "  -> string",
           " getInvFileSeparators()",
           " Returns all characters used by Invered Files to "
          "tokenize texts as a single string",
           " query getInvFileSeparators()" );




Operator getInvFileSeparators (
         "getInvFileSeparators" ,           // name
          getInvFileSeparatorsSpec.getStr(),          // specification
          getInvFileSeparatorsVM,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          getInvFileSeparatorsTM);




} // end of namespace triealg




class TrieAlgebra : public Algebra {
  public:
     TrieAlgebra() : Algebra() {
     AddTypeConstructor( &triealg::invfiletc );

     
     AddOperator(&triealg::createInvFile);
     triealg::createInvFile.SetUsesMemory();

     AddOperator(&triealg::getFileInfo);
     AddOperator(&triealg::wordCount);
     AddOperator(&triealg::prefixCount);
     AddOperator(&triealg::searchWord);
     AddOperator(&triealg::searchPrefix);
     AddOperator(&triealg::getInvFileSeparators);


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



