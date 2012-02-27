
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


/*
1 Foreword

In this file several classes realizing an inverted File index are 
defined.


*/

typedef SmiRecordId TrieContentType; // content of trie entries
typedef uint32_t wordPosType;        // type representing word positions
typedef uint32_t charPosType;        // type representing character positions



namespace appendcache{

/*

2 AppendCache

When building an enverted file index a lot of append operations to existing
records within the file containing the inverted lists are performed. To 
accelerate these appens, the class appendCache can be used.


*/

/*
2.1 Class CacheEntry


This class represents the non persistent part of a record. 

*/

class CacheEntry{

  public:

/*
2.1.1 Constructor

This constructor creates a cache entry for the given record id. 
The size of this record is specified in ~currentRecordSize~. 
The size of the buffer corresponds to the ~slotSize~ argument.

The content of the buffer will be swapped to disk if the buffer is
full or the function ~bringToDisk~ is called. 

*/
    CacheEntry(const SmiRecordId _id, 
               const size_t currentRecordSize,
               const size_t _slotSize): 
                  id(_id), offset(currentRecordSize), 
                  length(0), slotSize(_slotSize) {
          buffer = new char[slotSize];
    }

/*
2.1.2 Destructor

*/
    ~CacheEntry(){
        delete[] buffer;
    }

/*
2.1.3 bringToDisk

Writes the used part of the buffer to disk and empties the 
buffer for further append calls.

*/
    void bringToDisk(SmiRecordFile* file){

       SmiRecord record;
       file->SelectRecord(id, record, SmiFile::Update);
       record.Write(buffer, length, offset);
       offset += length;
       length = 0;

       record.Finish();
    }


/*
2.1.4 append

Appends data to this entry. In case of an overflow, the
buffer is written to disk and emptied for further append calls.

*/
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

/*
2.2 RecordAppendCache

This class realizes an cache for Records only suporting append operation
to these records. The cache uses the LRU strategy for writing record contents
to disk.


*/
class RecordAppendCache{

  public:

/*
2.2.1 Constructor

Creates a new cache for the underlying file, with a maximum 
memory consumptions of ~maxMem~ and a buffer size of ~slotSize~
for each record.

*/
     RecordAppendCache(SmiRecordFile* _file,  
                       const size_t _maxMem, 
                       const size_t _slotSize): 

              file(_file), 
              lru(_maxMem / (_slotSize + sizeof(appendcache::CacheEntry))),
              slotSize(_slotSize){

     }


/*
2.2.2 Destructor

Empties the cache writing all in memory data to disk.

*/
     ~RecordAppendCache(){
         clear();
     }


/*
2.2.3 append

Appends data to the specified record.

*/
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


/*
2.2.4 clear

Removes all entries from the cache writing the buffers to disk.

*/
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
4 Class InvertedFile



*/


class InvertedFile: public Trie<TrieContentType> {

  public:
/*
~Standard Constructor~

*/
     InvertedFile(): Trie<TrieContentType>(), listFile(false,0,false){
        listFile.Create();
     }

/*
~Copy Constructor~

*/
     InvertedFile(const InvertedFile& src): Trie<TrieContentType>(src), 
                                            listFile(src.listFile) 
                                            {
      }

/*
~Constructor~

*/
    InvertedFile(SmiFileId& _trieFileId, SmiRecordId& _trieRootId,
                 SmiFileId& _listFileId): 
        Trie<TrieContentType>(_trieFileId, _trieRootId), 
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
       Trie<TrieContentType>::deleteFile();
       if(listFile.IsOpen()){
         listFile.Close();          
       }
       listFile.Drop();
    }

/*
~clone~

Creates a depth copy of this objects.

Not implemented yet.

*/

   InvertedFile* clone(){
      assert(false);
      return 0;
   }



/*
~BasicType~

Returns Secondo's type description of an inverted file.  

*/
   static string BasicType(){
     return "invfile";
   }


/*
~checktype~

Checkes whether type is Secondo's type description of an inverted file.

*/
  static bool checkType(ListExpr type){
    return listutils::isSymbol(type,BasicType());
  }


/*
~createAppendCache~

Creates an appendCache for this inverted file. The caller is responsible for
deleting the created object. This Cache can be used within the insert 
function.

*/

   appendcache::RecordAppendCache* createAppendCache(const size_t maxMem){
     return new appendcache::RecordAppendCache(&listFile, maxMem, 2048);
   }

/*
~createTrieCache~

Creates an cache for the nodes of the trie of this inverted file. 
The caller is responsible for deleting the created object. This 
cache can be used within the insert function.

*/
   TrieNodeCache<TrieContentType>* createTrieCache(const size_t maxMem){
     return new TrieNodeCache<TrieContentType>(maxMem,&file);
   }


/*
~insertText~

Inserts the words contained within ~text~ into this inverted file.

*/
   void insertText(TupleId tid, const string& text, 
                   appendcache::RecordAppendCache* cache=0,
                   TrieNodeCache<TrieContentType>* triecache = 0 ){

       stringutils::StringTokenizer 
                 st(text," \t\n\r.,;:-+*!?()<>\"$§&/[]{}=´`@€~'#|");
       wordPosType wc = 0;
       charPosType pos = 0;
       while(st.hasNextToken()){
          pos = st.getPos(); 
          string token = st.nextToken();
          if(token.length()>0){
            insert(token, tid, wc, pos, cache, triecache);
            wc++;
          }
       }
   }


/*
~getListFileId~

Returns the fileId of the file containing the inverted lists.


*/

  SmiFileId getListFileId()  {
     return listFile.GetFileId();
  }


/*
5.3.2 Class exactIterator

The class can be used for iterating over a single inverted list.

*/
  class exactIterator {
     friend class InvertedFile;
     public:

/*
Function ~next~

This function returns the next position by setting the arguments.
If no more entries are available, the result of this function is __false__.

*/
       bool next(TrieContentType& id, wordPosType& wc, charPosType& cc){
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
         memcpy(&wc,buffer+offset, sizeof(wordPosType));
         offset += sizeof(wordPosType);
         memcpy(&cc,buffer+offset, sizeof(charPosType));
         slotPos++;
         count++;
         return true;
       }


/*
~Destructor~

*/
       ~exactIterator(){
          if(record){
             record->Finish();
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

/*
~Constructor~

This constructor will create an iterator returning no entry.

*/
       exactIterator(){
         done = true;
         buffer = 0;
         record = 0;       
       }


/*
~Constructor~

This constructor will create an iterator iterating over the entries
of the specified record.

*/
       exactIterator(SmiRecordFile* f, SmiRecordId id, 
                     const size_t _mem): part(0), slotPos(0), record(0){

          count = 0;
          slotSize = sizeof(TrieContentType) + sizeof(wordPosType) + 
                     sizeof(charPosType); 
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


/*
~readPartition~

Reads the next part of the record into the memory buffer.

*/

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

  }; // end of class ExactIterator



/*
~getExactIterator~

This function returns an iterator for a specified word.

*/
 
  exactIterator* getExactIterator(const string& str, const size_t mem){
    // find the node for str
    SmiRecordId id = rootId;
    size_t pos = 0;
    while((id!=0) && (pos <str.length())) {
       TrieNode<TrieContentType> node(&file,id);
       id = node.getNext(str[pos]);
       pos++; 
    }
    if(id!=0){
       TrieNode<TrieContentType> node(&file,id);
       TrieContentType tid = node.getContent();
       return new exactIterator(&listFile, tid, mem);
    }
    return new exactIterator();
  }


/*
~getExactIterator~

This function returns an iterator for a specified record.

*/
  exactIterator* getExactIterator(const SmiRecordId& rid, const size_t mem){
    return new exactIterator(&listFile, rid, mem);
  } 

/*
~wordCount~

This functions returns how ofter ~word~ is stored within this inverted file.

*/   
     size_t wordCount(const string& word){
       return wordCount( Trie<TrieContentType>::search(word));

     }


/*
~wordCount~

This function returns how many entries are stored within a specified record.

*/
     size_t wordCount(const TrieContentType id){
       if(id==0){
         return 0;
       }
       SmiRecord record;
       listFile.SelectRecord(id,record);
       size_t s = record.Size();
       record.Finish();
       return s / entrySize();
     }

/*
~entrySize~

This function returns the size of a single entry within an inverted list.

*/
   size_t entrySize()const{
         return  sizeof(TrieContentType) + sizeof(wordPosType) + 
                 sizeof(charPosType); 
   }


/*
2.3.5 Class PrefixIterator

This iterator iterates over all entries starting with a specified prefix.

*/
  
  class prefixIterator{
     friend class InvertedFile;
     public:

/*
~next~

Returns the next entry of this iterator if present. If not, the arguments keep
unchanged and the return value if false.

*/
       bool next(string& word, 
                 TrieContentType& tid, 
                 wordPosType& wc, 
                 charPosType& cc){
          while(true){
            if(exactIt==0){
                // create a new exactIterator
                if(!it->next(str,id)){
                   return false;
                }
                exactIt = inv->getExactIterator(id, 4096);
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

       ~prefixIterator(){
          if(it){
            delete it;
          } 
          if(exactIt){
            delete exactIt;
          }
       }

     private:
       InvertedFile* inv;
       TrieIterator<TrieContentType>* it;
       exactIterator* exactIt;
       SmiRecordId id; 
       string str;
      
/*
~constructor~

*/ 
       prefixIterator(InvertedFile* _inv, const string& prefix): inv(_inv){
          it = inv->getEntries(prefix);
          exactIt = 0;
          id = 0;
      }
   }; // end of class PrefixIterator


/*
~getPrefixIterator~

Returns a prefixIterator for str. The caller of this functions is responsible to
destroy the iterator after using.

*/
   prefixIterator* getPrefixIterator(const string& str){
     return new prefixIterator(this, str);
   }


/*
Class countPrefixIterator

This iterator returns all words starting with a certain prefix stored in this
structure together with the count of this word.

*/
  class countPrefixIterator{
    friend class InvertedFile;
    public:
       bool next(string& word, size_t& count){
          TrieContentType id;
          if(!it->next(word, id)){
             return false;
          }
          count = inv->wordCount(id);
          return true;
       }
      
       ~countPrefixIterator(){
          delete it;
        }

    private:
       InvertedFile* inv;
       TrieIterator<TrieContentType>* it;


       countPrefixIterator(InvertedFile* _inv, const string& prefix):  
         inv(_inv) {
         it = inv->getEntries(prefix);
       }

  };

/*
~getCountPrefixIterator~

Returns a countPrefixIterator of this for a specified prefix.

*/
  countPrefixIterator* getCountPrefixIterator(const string& str){
    return new countPrefixIterator(this, str);
  }



/*
~getFileInfo~

Returns data about the underlying files.

*/

   void  getFileInfo( SmiStatResultType& result){
         Trie<TrieContentType>::getFileInfo(result); 
         SmiStatResultType listresult = 
                           listFile.GetFileStatistics(SMI_STATS_LAZY);
         listresult.push_back( pair<string,string>("FilePurpose", 
                                                   "Inverted List File"));
         result.push_back(pair<string,string>("---","---"));
         for(unsigned int i=0;i<listresult.size();i++){
           result.push_back(listresult[i]);
         }
    }


   




  private:
     SmiRecordFile listFile;
  

/*
~insert~

inserts a new element into this inverted file

*/
 
   void insert(const string& word, 
               const TupleId tid, 
               const wordPosType wordPos, const charPosType pos, 
               appendcache::RecordAppendCache* cache,
               TrieNodeCache<TrieContentType>* triecache){


       SmiRecordId listId;
       SmiRecord record;     // record containing the list
       SmiRecordId recordId; // id of the record
       TrieNode<TrieContentType> insertNode;
       SmiRecordId insertId;

      
       bool isNew;
       if(triecache){
            isNew  = Trie<TrieContentType>::getInsertNode(word, insertNode, 
                                                       insertId, triecache);
       } else {
            isNew  = Trie<TrieContentType>::getInsertNode(word, insertNode,
                                                           insertId);
       }

       if(insertNode.getContent()==0){
          listFile.AppendRecord(listId, record);
          if(!triecache){
              insertNode.setContent(listId);
              insertNode.writeToFile(&file, insertId);   
          } else {
              triecache->getNode(insertId)->setContent(listId);
          }
          recordId = listId; 
       } else {
          assert(!isNew);
          if(cache==0){
            listFile.SelectRecord(insertNode.getContent(),
                                record, SmiFile::Update); 
          }
          recordId = insertNode.getContent();
       }

       size_t buffersize = sizeof(TupleId) + sizeof(wordPosType) + 
                           sizeof(charPosType);
       char buffer[buffersize];
       size_t offset=0;
       memcpy(buffer,&tid, sizeof(TupleId));
       offset += sizeof(TupleId);
       memcpy(buffer + offset, &wordPos, sizeof(wordPosType));
       offset += sizeof(wordPosType);
       memcpy(buffer + offset, &pos, sizeof(charPosType));
       if(cache==0){
          size_t recordOffset = record.Size();
          record.Write(buffer, buffersize, recordOffset);
       } else {
          record.Finish();
          cache->append(recordId, buffer, buffersize);
       }
  }





};










