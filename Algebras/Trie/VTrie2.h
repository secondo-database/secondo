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
#include "LRU.h"
#include <string>
#include <stdlib.h>
#include "NestedList.h"
#include "ListUtils.h"
#include "StringUtils.h"
#include <stack>




#ifndef VTRIE_H
#define VTRIE_H

namespace vtrie{


static const unsigned int CHARS = 256;

/*
1 Class VTrieNode

This class represents a single node within a Vtrie. Besides the content of type I,
it contains an array of 256 SmiRecordIds for referring the sons.

*/


template<typename I>
class VTrieNode{
  public:


/*
~Constructor~

This constructor creates a new Node without any successors.

*/    
    VTrieNode():content(0){
       memset(links, 0, CHARS*sizeof(SmiRecordId));
    }

/*
~Constructor~

This constructor creates a Vtrie node from the record stored
if file at position recID.

*/
    VTrieNode(SmiRecordFile* file, const SmiRecordId& recID){
       readFrom(file, recID);
    }


/*
~readFrom~

The functions read this node from the specifified record. It overwrites
the content of this node. If an error occurs, the return value of this function
will be false.

*/
    bool readFrom(SmiRecordFile* file, const SmiRecordId& id){
       if(id==0){
         return false;
       }
       SmiSize length;
       char* data = file->GetData(id, length, true);
       if(data==0){
         return false;
       }
       readFrom((unsigned char*)data,length);
       free(data);
       return true;
    }


/*
~writeToFile~

Writes the content of this node to a specified record.

*/
    void writeToFile(SmiRecordFile* f, SmiRecordId rid){
       SmiSize bufferLength;
       char* buffer = getBuffer(bufferLength);
       SmiSize written;
       f->Write(rid, buffer, bufferLength,0, written);
       assert(written == bufferLength);
       delete[] buffer;
    }

/*
~appendToFile~

This function appends this node to a file and returns the 
record id where this node was stored.

*/

    SmiRecordId appendToFile(SmiRecordFile* file){
     
      SmiRecord record;
      SmiSize bufferLength;
      char* buffer = getBuffer(bufferLength);
      SmiRecordId id;
      file->AppendRecord(id, record);
      record.Write(buffer, bufferLength); 
      delete[] buffer;
      return id;
    }


/*
~copyFromTo~

This function copyies the subtree rooted by this node into the file ~dest~.

*/
    SmiRecordId copyFromTo(SmiRecordFile* src, SmiRecordFile* dest){
       VTrieNode<I> copy;
       copy.content = content;
       for(unsigned int i=0; i<CHARS;i++){
          if(links[i]!=0){
             VTrieNode<I> sc(src,links[i]);
             copy.links[i] = copyFromTo(src,dest); 
          }
       }
       return copy.appendToFile(dest);   
    }


/*
~print~

Prints a textual representation of this node to ~o~.

*/
       
    ostream& print(ostream& o){
      o << "[ " << content << ", " << " ( " ;
      bool first = true;
      for(unsigned int i=0;i<CHARS;i++){
        if(links[i]!=0){
          if(!first){
              o << ", " ;
          }
          o << (char)i  << " :  " << links[i] ; 
        }
      }
      o << ") ]" ;
      return o;
    }          


/*
~next~

Returns the record Id for the son at the specified character index.

*/
    SmiRecordId getNext(unsigned char pos) const {
       return links[pos];
    }


/*
~setNext~

Sets the son at position pos to id.

*/
    void setNext( const unsigned char pos, const SmiRecordId id){
       links[pos] = id;  
    }


/*
~getContent~

Returns the content belonging to this node.

*/
    I getContent(){ 
      return content;
    }

/*
~setContent~

Sets the content for this node.

*/
    void setContent(const I& c){
       content = c;
    }

/*
~clear~

Sets all sons and the content of this node to 0.

*/
    void clear(){
       memset(links, 0, CHARS*sizeof(SmiRecordId));
       content=0;
    }
 

  private:
    SmiRecordId links[CHARS]; 
    I content;
    

/*
~readFrom~

Sets this node to the content of the buffer.

*/
  void inline readFrom( const unsigned char* buffer, SmiSize bufferSize){
      memset(links, 0, CHARS*sizeof(SmiRecordId));
      size_t entriesSize = bufferSize -  sizeof(content);
      size_t esize = sizeof(unsigned char) + sizeof(SmiRecordId);
      size_t noEntries = entriesSize / esize;
      size_t offset = 0;
      unsigned char c;
      SmiRecordId id;
      for(unsigned int i=0;i<noEntries;i++){
         memcpy(&c,buffer+offset,sizeof(unsigned char));
         offset += sizeof(unsigned char);
         memcpy(&id,buffer+offset,sizeof(SmiRecordId));
         offset+=sizeof(SmiRecordId);
         links[c] = id;
      } 
      memcpy(&content, buffer+offset, sizeof(I));
  }

/*
~writeTo~

Copyies this node to a buffer.

*/
  void writeTo(char* buffer) const{
     size_t offset =0;
     unsigned char c;
     SmiRecordId id;

     for(unsigned int i=0;i<CHARS;i++){
         if( (id = links[i])!=0){
            c = (unsigned char) i;
            memcpy(buffer+offset,&c, sizeof(unsigned char));
            offset += sizeof(unsigned char);
            memcpy(buffer+offset,&id, sizeof(SmiRecordId));
            offset += sizeof(SmiRecordId);
         }
     }
     memcpy(buffer+offset, &content, sizeof(I));
  }


/*
~getBuffer~

Creates a buffer containing this node.

*/
  char* getBuffer( SmiSize& bufferLength) const{
     bufferLength = getBufferLength();
     char* res = new char[bufferLength];
     writeTo(res);
     return res;
  }

/*
~getBufferLength~

Return the length of a character buffer able to store this node.

*/
  SmiSize getBufferLength()const{
    SmiSize bl = sizeof(I);
    for(unsigned int i=0;i<CHARS;i++){
      if(links[i]){
         bl += sizeof(unsigned char) + sizeof(SmiRecordId);
      }
    }
    return bl;
  }
};


/*
2 Cache for VTrieNodes

This class represents an LRU cache for ionstances of class VTrieNode<T>.

*/
template<class T>
class VTrieNodeCache{
  public:

/*
2.1 Contructor

Creates a cache with size ~maxMem~ for file ~file~.

*/
      VTrieNodeCache(size_t _maxMem, SmiRecordFile* _file): 
                   file(_file), lru(_maxMem / sizeof(VTrieNode<T>)){

      }

/*
~Destructir~

Clears the cache and writes back all contained nodes.

*/
      ~VTrieNodeCache(){
         clear();
       }


/*
~getNode~

Returns the node specified by it's record id.

*/
      VTrieNode<T>* getNode(const SmiRecordId id ){
         VTrieNode<T>** n = lru.get(id);
         if(n!=0){
           return *n;
         }

         VTrieNode<T>* node = new VTrieNode<T>(file,id);
         LRUEntry<SmiRecordId, VTrieNode<T>*>* victim = lru.use(id, node);
         if(victim!=0){
             victim->value->writeToFile(file, victim->key);
             delete victim->value;
             delete victim;
         }
         return node;
      }


/*
~clear~

Removes all entries form this cache and writes back the contained nodes.

*/
      void clear(){
         LRUEntry<SmiRecordId, VTrieNode<T>*>* victim;
         while( (victim  = lru.deleteLast())!=0){
             victim->value->writeToFile(file, victim->key);
             delete victim->value;
             delete victim;
         }
      }


/*
~appendBlankNode~

Creates a new node, appends it to the underlying file and inserts this node into the cache.

*/
      VTrieNode<T>*  appendBlankNode( SmiRecordId& id){
          VTrieNode<T>*  node = new VTrieNode<T>();
          id = node->appendToFile(file);
          LRUEntry<SmiRecordId, VTrieNode<T>*>* victim = lru.use(id, node);
          if(victim!=0){
             victim->value->writeToFile(file, victim->key);
             delete victim->value;
             delete victim;
         }
         return node;          
      }

  private:
     SmiRecordFile* file;
     LRU<SmiRecordId, VTrieNode<T>*> lru;
     

};




/*
3 Class StackEntry

A helper class for the VTrieIterator.

*/

template<class I>
class StackEntry{

public:
  StackEntry(const VTrieNode<I>& _node, unsigned int _pos, const string& _str):
     node(_node),pos(_pos),str(_str){}

  VTrieNode<I> node;
  unsigned int pos;
  string  str;   
};



/*
4 Class VTrieIterator


*/
template<class I>
class VTrieIterator{
public:

/*
4.1 Constructor

*/


   VTrieIterator(SmiRecordFile* _file, 
                const SmiRecordId& rid,
                const string& _str):
    file(_file), st(){
      if(rid!=0){
        VTrieNode<I> s(_file,rid);
        StackEntry<I>* entry = new StackEntry<I>(s,0,_str);
        st.push(entry);
      }
   }

/*
4.2 Destructor

Destroys the underlying data structure.

*/
   ~VTrieIterator(){
      while(!st.empty()){
         StackEntry<I>* victim = st.top();
         st.pop();
         delete victim;
      }
    }


/*
4.3 ~next~

If there are more entries starting with the prefix specified in the constructor, this
function will return true and set ~str~ to the complete word and set content to the
content of the corresponding TrieNode.

*/
    bool next(string& str, I& content){
      while(!st.empty()){
        StackEntry<I>* top = st.top();
        st.pop();
        unsigned int pos = top->pos;
        while(pos<CHARS && top->node.getNext(pos)==0){
          pos++;
        }
        if(pos >= CHARS){
           if(top->node.getContent()!=0){
              str = top->str;
              content = top->node.getContent(); 
              delete top;
              return true;
           } else {
              delete top;
           }
        } else {
          SmiRecordId c = top->node.getNext(pos);
          VTrieNode<I> s(file,c);
          stringstream ss;
          ss << top->str;
          ss << (char) pos;
          StackEntry<I>* entry = new StackEntry<I>(s,0,ss.str());
          top->pos = pos+1;
          st.push(top);
          st.push(entry);
        }
      }
      return false;
    }

  private:
     SmiRecordFile* file;
     stack<StackEntry<I>*> st;
};


/*
5 Class VTrie

This class represents a prefix tree.

*/

template<class T>
class VTrie{

  public:

/*
5.1 Constructor


*/
     VTrie(): file(false,0,false), rootId(0){
         file.Create();
      }

/*
5.2 Copy Constructor.

This constructor creates a flat copy  for src.

*/
     VTrie(const VTrie& src):file(src.file), rootId(src.rootId) {
     }


/*
5.3 Constructor

Creates a Vtrie with given root.

*/
     VTrie(SmiFileId fid, SmiRecordId rid) : file(false,0,false), rootId(rid){
        file.Open(fid);
     }


/*
5.4 Destructor

*/
     ~VTrie(){
       if(file.IsOpen()){
          file.Close();
       }
     }


/*
5.5 insert

Inserts id at the node specified by s. If there is already an entry, this
entry will be overwritten by id. 

*/
     void insert(const string& s, const T id){
        if(id==0){
            return;
        }
        if(rootId==0){
           VTrieNode<T> son;
           bool newEntry = false;
           son.insert(s,0,id,&file, newEntry);
           rootId = son.appendToFile(&file);
        } else {
           VTrieNode<T> son(&file,rootId);
           bool newEntry = false;
           bool changed = son.insert(s,0,id,&file, newEntry);
           if(changed){
             son.writeToFile(&file, rootId);
           }
        }
     }

/*
5.6 deleteFile

Removes ths underlying file from disk.

*/
     void deleteFile(){
       if(file.IsOpen()){
         file.Close();
       }
       file.Drop();
     }


/*
5.7 search

Returns the content stored under the specified prefix.

*/
     T search(const string& str){
          SmiRecordId id = rootId;
          size_t pos = 0;
          while((id!=0) && (pos <str.length())) {
              VTrieNode<T> node(&file,id);
              id = node.getNext(str[pos]);
              pos++; 
          }
          if(id==0){
            return 0;
          }
          return VTrieNode<T>(&file,id).getContent();
          
     }


/*
5.8 contains

Checks whether str is a member of this Vtrie. If acceptPrefix is true,
the return value of this function will also be true, if str is a 
prefix of a stored word.

*/
     bool contains(const string& str, const bool acceptPrefix){
         unsigned int pos=0;
         SmiRecordId id = rootId;
         
         VTrieNode<T> son;
         while( (id!=0) && pos < str.length()){
            son.readFrom(&file,id);
            id = son.getNext(str[pos]);
            pos++;
         }
         if(id==0){
            return false; 
         }
         son.readFrom(&file,id);
         return acceptPrefix || (son.getContent()!=0);
     }

/*
5.9 clone

Creates a depth copy of this VTrie.

*/
     VTrie* clone(){
        VTrie * res = new VTrie();
        if(rootId!=0){
           VTrieNode<T> son(&file,rootId);
           res->rootId = son.copyFromTo( &file, &res->file);
        }
        return res;
     }

/*
5.10 copyFrom

Sets this Vtrie to be identically to src.

*/
     void copyFrom( VTrie& src){
        if(rootId!=0){
          file.ReCreate();
          rootId = 0;
        }
        if(src.rootId!=0){
          VTrieNode<T> srcSon(&src.file , src.rootId);
          rootId = srcSon.copyFromTo(&src.file, &file);
        }
     }


/*
5.11 BasicType

Returns Secondo's type description for this class.

*/
     static string BasicType(){ 
        return "Vtrie";
     }
  

/*
5.12 checkType

Checks whether ~t~ corresponds the Secondo's type description of this class.

*/
     static bool checkType(ListExpr t){
       return listutils::isSymbol(t,BasicType());
     }

/*
5.13 getFileId

Returns the file id of the underlying file.

*/
     SmiFileId getFileId() {
       return file.GetFileId();
     }    


/*
5.14 Returns the record id referring to the root of this Vtrie.

*/ 
     SmiRecordId getRootId()const {
         return rootId;
     }


/*
5.15 getEntries

Returns an iterator iterating over all entries with ~prefix~ as prefix. The caller
of this function is responsible to delete the returned instance.

*/
     VTrieIterator<T> * getEntries(const string& prefix){
        unsigned int pos=0;
        SmiRecordId id = rootId;

        while(pos<prefix.length() && (id!=0)){
           VTrieNode<T> s(&file,id);
           id = s.getNext(prefix[pos]);
           pos++;
        } 
        return new VTrieIterator<T>(&file,id,prefix);
     }

/*
5.16 getFileInfo

Returns statistical information about the underlying file.

*/
    void  getFileInfo( SmiStatResultType& result){
          result = file.GetFileStatistics(SMI_STATS_LAZY);
          result.push_back( pair<string,string>("FilePurpose", 
                                                "Secondary VTrie Index"));
     }



 protected:
    SmiRecordFile file;
    SmiRecordId   rootId;



/*
5.17 ~getInsertNode~

Returns the node corresponding to str. ~node~ and ~nodeId~ will be
set to these values. If the node was not present before calling this 
function, the return value will be true.

*/
    bool getInsertNode(const string& str, 
                       VTrieNode<T>& node, 
                       SmiRecordId& nodeId){


         SmiRecordId lastId = 0;
         SmiRecordId id = rootId;
         size_t pos = 0;
         VTrieNode<T> son;
         while(id != 0 && pos < str.length()){
            son.readFrom(&file,id);
            lastId = id;
            id = son.getNext(str[pos]);
            if(id!=0){
              pos++;
            }
         }

         if(id!=0){
            node.readFrom(&file,id);
            nodeId = id;
            return false;
         }   

         // the string is not member of the Vtrie,
         // we extend the Vtrie
         id = lastId; 

         if(rootId == 0 ){
             rootId = son.appendToFile(&file);
             id = rootId;
         }


         VTrieNode<T> newNode;
         SmiRecordId newId = 0;
         while(pos<str.length()){
            newNode.clear();
            newId = newNode.appendToFile(&file);
            son.setNext(str[pos],newId);
            son.writeToFile(&file, id);
            son = newNode;
            id = newId; 
            pos++;
         }

         node = son;
         nodeId = id; 
         return true;
    }
    
/*
5.17 getInsertNode

This is a variant of ~getInsertNOde~ using a cache for faster
access to the nodes. This is helpful in bulkload insertions but 
cannot be used within a transactional environment.

*/
    bool getInsertNode(const string& str, 
                       VTrieNode<T>& node, 
                       SmiRecordId& nodeId,
                       VTrieNodeCache<T>* cache){


         SmiRecordId lastId = 0;
         SmiRecordId id = rootId;
         size_t pos = 0;
         VTrieNode<T>* son=0;
         while(id != 0 && pos < str.length()){
            son = cache->getNode(id);
            lastId = id;
            id = son->getNext(str[pos]);
            if(id!=0){
              pos++;
            }
         }

         if(id!=0){
            node = *(cache->getNode(id));
            nodeId = id;
            return false;
         }   

         // the string is not member of the Vtrie,
         // we extend the Vtrie
         id = lastId; 

         if(rootId == 0 ){
             son = cache->appendBlankNode(rootId);
             id = rootId;
         }


         SmiRecordId newId = 0;
         while(pos<str.length()){
            VTrieNode<T>* newNode = cache->appendBlankNode(newId);
            son->setNext(str[pos],newId);
            son = newNode;
            id = newId; 
            pos++;
         }

         node = *son;
         nodeId = id; 
         return true;
    }

};


} // end of namespace Vtrie

#endif


