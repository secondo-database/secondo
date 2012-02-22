
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

#include "LRU.h"


namespace appendcache{

  // debug function
  /*
void print(const char* buffer){
  SmiRecordId id;
  size_t wc;
  size_t cc;
  size_t offset = 0;
  memcpy(&id, buffer, sizeof(SmiRecordId));
  offset+=sizeof(SmiRecordId); 
  memcpy(&wc, buffer + offset, sizeof(size_t));
  offset+=sizeof(size_t); 
  memcpy(&cc, buffer + offset, sizeof(size_t));
  offset+=sizeof(size_t); 
  cout << id << ", " << wc << ", " << cc;
}
  */


class CacheEntry{

  public:
    CacheEntry(const SmiRecordId _id, 
               const size_t currentRecordSize,
               const size_t _slotSize): 
                  id(_id), offset(currentRecordSize), 
                  length(0), slotSize(_slotSize) {
          buffer = new char[slotSize];
    }

    ~CacheEntry(){
        delete[] buffer;
    }

    void bringToDisk(SmiRecordFile* file){

       SmiRecord record;
       file->SelectRecord(id, record, SmiFile::Update);
       record.Write(buffer, length, offset);
       offset += length;
       length = 0;

       record.Finish();
    }


    void append(SmiRecordFile* file, const char* buffer, const size_t length){
       // the buffer cannot be appended in memory
       size_t offset = 0; // offset in buffer
       while(this->length + length - offset > slotSize){
          size_t toWrite = slotSize-this->length;
          memcpy(this->buffer + this->length, buffer + offset, toWrite);
          this->length += toWrite; 
          offset += toWrite;

          bringToDisk(file);
       }

       size_t toWrite = length - offset;

       memcpy(this->buffer+this->length, buffer, toWrite);
       this->length += toWrite;

    }

  private:
    SmiRecordId id;
    size_t offset;
    size_t length;
    size_t slotSize;
    char* buffer;

};


class RecordAppendCache{

  public:
     RecordAppendCache(SmiRecordFile* _file,  
                       const size_t _maxMem, 
                       const size_t _slotSize): 

              file(_file), 
              lru(_maxMem / (_slotSize + sizeof(appendcache::CacheEntry))),
              slotSize(_slotSize){

     }

     ~RecordAppendCache(){
         clear();
     }

     void append(SmiRecordId id, const char* buffer, const size_t length){
        appendcache::CacheEntry** entry = lru.get(id);
        appendcache::CacheEntry* ce=0;
        if(entry==0){ // not cached
          SmiRecord record;
          file->SelectRecord(id, record);
          ce = new appendcache::CacheEntry(id, record.Size(), slotSize);
          record.Finish();
          LRUEntry<SmiRecordId, appendcache::CacheEntry*>* e2 = lru.use(id, ce);
          if(e2!=0){
             e2->value->bringToDisk(file);
              
             delete e2->value;
             delete e2;

          }
          entry = lru.get(id);
          assert(entry!=0);
        }
 
        ce = *entry;
        ce->append(file, buffer, length);
     }

    void clear(){
         LRUEntry<SmiRecordId, appendcache::CacheEntry*>* victim;
         while( (victim = lru.deleteLast())!=0){
            victim->value->bringToDisk(file);
            delete victim->value;
            delete victim; 
         }
    }

  private:
      SmiRecordFile* file;
      LRU<SmiRecordId, appendcache::CacheEntry*> lru;
      size_t slotSize;
};

} // end of namespace appendcache




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

   appendcache::RecordAppendCache* createAppendCache(const size_t maxMem){
     return new appendcache::RecordAppendCache(&listFile, maxMem, 1024);
   }


   void insertText(TupleId tid, const string& text, 
                   appendcache::RecordAppendCache* cache=0 ){

       stringutils::StringTokenizer st(text," \t\n\r.,;:-+*!?()");
     //  stringutils::StringTokenizer st(text," \t\n\r.,;:-+*!?()<>\"'");
       size_t wc = 0;
       size_t pos = 0;
       while(st.hasNextToken()){
          pos = st.getPos(); 
          string token = st.nextToken();
          if(token.length()>0){
            insert(token, tid, wc, pos, cache);
            wc++;
          }
       }
   }



  SmiFileId getListFileId()  {
     return listFile.GetFileId();
  }



  class exactIterator {
     friend class InvertedFile;
     public:
       bool next(TupleId& id, size_t& wc, size_t& cc){
         if(!record){ // no record available
           return false;
         }
         if(done){ // finished
           return false;
         }

         if(slotPos >= slotsInMem){ // buffer exhausted
           done = readPartition();
         }         

         if(done){ // no further slots 
            return false;
         }

         size_t offset = slotSize*slotPos;
         memcpy(&id,buffer+offset, sizeof(TupleId));
         offset += sizeof(TupleId);
         memcpy(&wc,buffer+offset, sizeof(size_t));
         offset += sizeof(size_t);
         memcpy(&cc,buffer+offset, sizeof(TupleId));
         slotPos++;
         count++;
         return true;
       }

       ~exactIterator(){
          record->Finish();
          if(record){
             delete record;
          } 
          if(buffer){
             delete[] buffer;
          }
        }

     private:
       size_t part;           // partition within the record
       size_t slotPos;        // position within the slot
       SmiRecord* record;     // record containing the values
       size_t slotsInMem;     // currently available slots in memory 
       size_t maxSlotsInMem;  // maximum number of slots in memory
       size_t slotsInRecord;  // slots available in record
       char* buffer;          // memory buffer
       bool done;             // true if record is exhausted
       size_t slotSize;       // size of a single slot
       size_t count;          // number of returned results

       exactIterator(){
         done = true;
         buffer = 0;
         record = 0;       
       }

       exactIterator(SmiRecordFile* f, SmiRecordId id, 
                     const size_t _mem): part(0), slotPos(0), record(0){

          count = 0;
          slotSize = sizeof(TupleId) + sizeof(size_t) + sizeof(size_t); 
          maxSlotsInMem = _mem / slotSize; 
          if(maxSlotsInMem<1){
             maxSlotsInMem = 1;
          } 
          if(id!=0){
             record=new SmiRecord();
             f->SelectRecord(id, *record);
          } else {
             buffer = 0;
             return;
          }

          slotsInRecord = record->Size() / slotSize;

          size_t buffersize = maxSlotsInMem * slotSize;
          buffer = new char[buffersize];
          slotsInMem = 0;
          done = readPartition(); 
       }

      // transfer data from record to memory
      bool readPartition(){
         int64_t processed = part * maxSlotsInMem;
         int64_t  availableSlots = (int64_t)slotsInRecord - processed;
         if(availableSlots <= 0){
            return true; 
         }
         
         size_t readSlots = min((size_t) availableSlots, maxSlotsInMem);

         record->Read(buffer, readSlots*slotSize , part*maxSlotsInMem*slotSize);
         part++;
         slotsInMem = readSlots;
         slotPos = 0; // nothing read from current mem
         return false;
      }

  };

 
  exactIterator* getExactIterator(const string& str, const size_t mem){
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
       return new exactIterator(&listFile, tid, mem);
    }
    return new exactIterator();
  }

  exactIterator* getExactIterator(const SmiRecordId& rid, const size_t mem){
    return new exactIterator(&listFile, rid, mem);
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
                exactIt = inv->getExactIterator(id, 1024);
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
  

/*
~insert~

inserts a new element into this inverted file

*/
 
   void insert(const string& word, const TupleId tid, 
                const size_t wordCount, const size_t pos, 
                appendcache::RecordAppendCache* cache){


       SmiRecordId listId;
       SmiRecord record;     // record containing the list
       SmiRecordId recordId; // id of the record
       TrieNode<TupleId> insertNode;
       SmiRecordId insertId;

       bool isNew = Trie::getInsertNode(word, insertNode, insertId);

       if(insertNode.getContent()==0){
          listFile.AppendRecord(listId, record);
          insertNode.setContent(listId);
          insertNode.writeToFile(&file, insertId);   
          recordId = listId; 
       } else {
          assert(!isNew);
          if(cache==0){
            listFile.SelectRecord(insertNode.getContent(),
                                record, SmiFile::Update); 
          }
          recordId = insertNode.getContent();
       }

       size_t buffersize = sizeof(TupleId) + sizeof(size_t) + sizeof(size_t);
       char buffer[buffersize];
       size_t offset=0;
       memcpy(buffer,&tid, sizeof(TupleId));
       offset += sizeof(TupleId);
       memcpy(buffer + offset, &wordCount, sizeof(size_t));
       offset += sizeof(size_t);
       memcpy(buffer + offset, &pos, sizeof(size_t));
       if(cache==0){
          size_t recordOffset = record.Size();
          record.Write(buffer, buffersize, recordOffset);
       } else {
          record.Finish();
          cache->append(recordId, buffer, buffersize);
       }
  }





};










