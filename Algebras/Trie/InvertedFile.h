
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


#include "SecondoSMI.h"
#include "TupleIdentifier.h"
#include <string>
#include <stdlib.h>
#include "NestedList.h"
#include "ListUtils.h"
#include "StringUtils.h"
#include "Trie.h"


/*
1 Class InvertedFile



*/


class InvertedFile: public Trie {

  public:
/*
~Standard Constructor~

*/
     InvertedFile(): Trie(), listFile(false,0,false){
        listFile.Create();
     }

/*
~Copy Constructor~

*/
     InvertedFile(const InvertedFile& src): Trie(src), 
                                            listFile(src.listFile) 
                                            {
      }

/*
~Constructor~

*/
    InvertedFile(SmiFileId& _trieFileId, SmiRecordId& _trieRootId,
                 SmiFileId& _listFileId): 
        Trie(_trieFileId, _trieRootId), 
        listFile(false){
         listFile.Open(_listFileId);

    }

/*
~Destructor~

*/
    ~InvertedFile(){
       if(listFile.IsOpen()){
           listFile.Close();
       }
     }

/*
~deleteFiles~

Destroys all underlying files.

*/
    void deleteFiles(){
       Trie::deleteFile();
       if(listFile.IsOpen()){
         listFile.Close();          
       }
       listFile.Drop();
    }

/*
~clone~

Creates a depth copy of this objects.

*/

   InvertedFile* clone(){
      assert(false);
      return 0;
   }

   static string BasicType(){
     return "invfile";
   }

  static bool checkType(ListExpr type){
    return listutils::isSymbol(type,BasicType());
  }



   void insertText(TupleId tid, const string& text){
       stringutils::StringTokenizer st(text," \t\n\r.,;:-+*!?()");
       size_t wc = 0;
       size_t pos = 0;
       while(st.hasNextToken()){
          pos = st.getPos(); 
          string token = st.nextToken();
          insert(token, tid, wc, pos);
          wc++;
       }
   }



/*
~printEntries~

Debug function

*/   
  void printEntries(const string& prefix){

     TrieIterator<TupleId>* it = Trie::getEntries(prefix);

     string word;
     SmiRecordId id; 
     while(it->next(word,id)){
        SmiRecord record;
        listFile.SelectRecord(id, record);

        TupleId tid;
        size_t wc;
        size_t pos;
        size_t offset=0;
        while(offset <  record.Size()){
          record.Read((char*) &tid, sizeof(TupleId), offset);
          offset += sizeof(TupleId);
          record.Read((char*) &wc, sizeof(size_t), offset);
          offset += sizeof(size_t);
          record.Read((char*) &pos, sizeof(size_t), offset);
          offset += sizeof(size_t);
          cout << "(" << tid << ", " << wc << ", " << pos << ")" << endl;
        }
        cout << " --- " << endl;
     }
     delete it;

  }

  SmiFileId getListFileId()  {
     return listFile.GetFileId();
  }



  class exactIterator {
     friend class InvertedFile;
     public:
       bool next(TupleId& id, size_t& wc, size_t& cc){
         if(!record){
           return false;
         }
         if(pos<record->Size()){
             record->Read(&id, sizeof(TupleId), pos);
             pos+= sizeof(TupleId);
             record->Read(&wc, sizeof(size_t), pos);
             pos += sizeof(size_t);
             record->Read(&cc , sizeof(size_t), pos);
             pos += sizeof(size_t);
             return true;
         }
         return false;
       }
       ~exactIterator(){
          record->Finish();
          delete record;
        }

     private:
       size_t pos;
       SmiRecord* record;

       exactIterator(SmiRecordFile* f, SmiRecordId id): pos(0),record(0){
          if(id!=0){
             record=new SmiRecord();
             f->SelectRecord(id, *record);
          }
       }
  };

 
  exactIterator* getExactIterator(const string& str){
    // find the node for str
    SmiRecordId id = rootId;
    size_t pos = 0;
    while((id!=0) && (pos <str.length())) {
       TrieNode<TupleId> node(&file,id);
       id = node.getNext(str[pos]);
       pos++; 
    }
    if(id!=0){
       TrieNode<TupleId> node(&file,id);
       TupleId tid = node.getContent();
       return new exactIterator(&listFile, tid);
    }
    return new exactIterator(0,0);
  }

  exactIterator* getExactIterator(const SmiRecordId& rid){
    return new exactIterator(&listFile, rid);
  } 

  
  class prefixIterator{
     friend class InvertedFile;
     public:
       bool next(string& word, TupleId& tid, size_t& wc, size_t& cc){
          while(true){
            if(exactIt==0){
                // create a new exactIterator
                if(!it->next(str,id)){
                   return false;
                }
                exactIt = inv->getExactIterator(id);
            } 
            if(exactIt){
               if(!exactIt->next(tid,wc,cc)){
                  delete exactIt;
                  exactIt=0;
               } else {
                  word = str;
                  return true;
               }
            }

          }
       }

     private:
       InvertedFile* inv;
       TrieIterator<TupleId>* it;
       exactIterator* exactIt;
       SmiRecordId id; 
       string str;
       
       prefixIterator(InvertedFile* _inv, const string& prefix): inv(_inv){
          it = inv->getEntries(prefix);
          exactIt = 0;
          id = 0;
      }
   };

   prefixIterator* getPrefixIterator(const string& str){
     return new prefixIterator(this, str);
   }



  private:
     SmiRecordFile listFile;
   
   void insert(const string& word, const TupleId tid, 
                const size_t wordCount, const size_t pos){

 
       if(word.length()==0){ // do not allow empty strings here
          return; 
       }

       SmiRecordId listId;
       SmiRecord record;
       TrieNode<TupleId> insertNode;
       SmiRecordId insertId;

       bool isNew = Trie::getInsertNode(word, insertNode, insertId);

       if(insertNode.getContent()==0){
          listFile.AppendRecord(listId, record);
          insertNode.setContent(listId);
          insertNode.writeToFile(&file, insertId);    
       } else {
          assert(!isNew);
          listFile.SelectRecord(insertNode.getContent(),
                                record, SmiFile::Update); 
       }

       size_t buffersize = sizeof(TupleId) + sizeof(size_t) + sizeof(size_t);
       char buffer[buffersize];
       size_t offset=0;
       memcpy(buffer,&tid, sizeof(TupleId));
       offset += sizeof(TupleId);
       memcpy(buffer + offset, &wordCount, sizeof(size_t));
       offset += sizeof(size_t);
       memcpy(buffer + offset, &pos, sizeof(size_t));

       size_t recordOffset = record.Size();
       record.Write(buffer, buffersize, recordOffset);
  }



};










