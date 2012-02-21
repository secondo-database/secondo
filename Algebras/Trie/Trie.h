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
#include <stack>




#ifndef TRIE_H
#define TRIE_H

static const unsigned int CHARS = 256;

template<typename I>
class TrieNode{
  public:


/*
~Constructor~

This constructor creates a new Node without any successors.

*/    
    TrieNode():content(0){
       memset(links, 0, CHARS*sizeof(SmiRecordId));
    }

/*
~Constructor~

This constructor creates a trie node from the record stored
if file at position recID.

*/
    TrieNode(SmiRecordFile* file, const SmiRecordId& recID){
       readFrom(file, recID);
    }


/*
~readFrom~

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
       readFrom((unsigned char*)data);
       free(data);
       return true;
    }


/*
~insert~

This fucntion inserts a string from postion __pos__ to this node.
The return value indicates changes at this node.  

*/
   bool insert(const string& str, 
                const unsigned int pos,
                const I& cont,
                SmiRecordFile* file,
                bool newEntry){


        if(str.length()-1 == pos){ // end of string reached
           if(content==0){
               newEntry = true;
           }
           content = cont;
           return true; // content changed
        }
        unsigned char c = str[pos];
        if(links[c] != 0){
            // path exists
            TrieNode son (file, links[c]);
            bool changed = son.insert(str,pos+1,cont,file, newEntry);
            if(changed){
               son.writeToFile(file, links[c]);
            }
            return false;
        }        
        // path does not exist
        TrieNode son; // create an empty node
        son.insert(str,pos+1,cont,file, newEntry);
        SmiRecordId id =  son.appendToFile(file);
        links[c] = id;
        return true;
    }
    
  
    I search(const std::string& str, const unsigned int pos, SmiRecordFile* f){

        if(str.length()-1 == pos){
           return content;
        }
        unsigned char c = str[pos];
        if(links[c] == 0){
          return 0;
        }

        TrieNode son(f,links[c]);
        return son.search(str, pos+1,f);
    }


    void writeToFile(SmiRecordFile* f, SmiRecordId rid){
       char* buffer = getBuffer();
       SmiSize written;
       f->Write(rid, buffer, getBufferLength(),0, written);
       assert(written = getBufferLength());
       delete[] buffer;
    }

    SmiRecordId appendToFile(SmiRecordFile* file){
     
      SmiRecord record;
      char* buffer = getBuffer();
      SmiRecordId id;
      file->AppendRecord(id, record);
      record.Write(buffer, getBufferLength()); 
      delete[] buffer;
      return id;
    }


    SmiRecordId copyFromTo(SmiRecordFile* src, SmiRecordFile* dest){
       TrieNode<I> copy;
       copy.content = content;
       for(unsigned int i=0; i<CHARS;i++){
          if(links[i]!=0){
             TrieNode<I> sc(src,links[i]);
             copy.links[i] = copyFromTo(src,dest); 
          }
       }
       return copy.appendToFile(dest);   
    }

       
    ostream& print(ostream& o){
      o << "[ " << content << ", " << " ( " <<  links[0];
      for(unsigned int i=0;i<CHARS;i++){
        o << ", " << links[i] ; 
      }
      o << ") ]" ;
      return o;
    }          

    SmiRecordId getNext(unsigned char pos) const {
       return links[pos];
    }

    void setNext( const unsigned char pos, const SmiRecordId id){
       links[pos] = id;  
    }

    I getContent(){ 
      return content;
    }

    void setContent(const I& c){
       content = c;
    }

    void clear(){
       memset(links, 0, CHARS*sizeof(SmiRecordId));
       content=0;
    }
 

  private:
    SmiRecordId links[CHARS]; 
    I content;
    


  void inline readFrom( const unsigned char* buffer){
     memcpy(links,buffer, CHARS*sizeof(SmiRecordId));
     memcpy(&content, (void*) (buffer + CHARS*sizeof(SmiRecordId)), sizeof(I));
  }

  void writeTo(char* buffer) const{
     memcpy(buffer, links, CHARS*sizeof(SmiRecordId));
     memcpy((void*) (buffer + CHARS*sizeof(SmiRecordId)), &content, sizeof(I));
  }

  char* getBuffer() const{
     char* res = new char[getBufferLength()];
     writeTo(res);
     return res;
  }

  size_t getBufferLength()const{
    return CHARS*sizeof(SmiRecordId) + sizeof(I);
  }
};




template<class I>
class StackEntry{

public:
  StackEntry(const TrieNode<I>& _node, unsigned int _pos, const string& _str):
     node(_node),pos(_pos),str(_str){}
  TrieNode<I> node;
  unsigned int pos;
  string  str;   
};






template<class I>
class TrieIterator{
public:
   TrieIterator(SmiRecordFile* _file, 
                const SmiRecordId& rid,
                const string& _str):
    file(_file), st(){
      if(rid!=0){
        TrieNode<I> s(_file,rid);
        StackEntry<I>* entry = new StackEntry<I>(s,0,_str);
        st.push(entry);
      }
   }


   ~TrieIterator(){
      while(!st.empty()){
         StackEntry<I>* victim = st.top();
         st.pop();
         delete victim;
      }
    }

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
          TrieNode<I> s(file,c);
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





class Trie{

  public:
     Trie(): file(true, CHARS*sizeof(SmiRecordId) + sizeof(TupleId)), rootId(0){
         file.Create();
      }

     Trie(const Trie& src):file(src.file), rootId(src.rootId) {
     }

     Trie(SmiFileId fid, SmiRecordId rid) : file(true), rootId(rid){
        file.Open(fid);
     }


     ~Trie(){
       if(file.IsOpen()){
          file.Close();
       }
     }


     void insert(const string& s, const TupleId tid){
        if(tid==0){
            return;
        }
        if(rootId==0){
           TrieNode<TupleId> son;
           bool newEntry = false;
           son.insert(s,0,tid,&file, newEntry);
           rootId = son.appendToFile(&file);
        } else {
           TrieNode<TupleId> son(&file,rootId);
           bool newEntry = false;
           bool changed = son.insert(s,0,tid,&file, newEntry);
           if(changed){
             son.writeToFile(&file, rootId);
           }
        }
     }


     void deleteFile(){
       if(file.IsOpen()){
         file.Close();
       }
       file.Drop();
     }

     TupleId search(const string& str){
         if(rootId==0){
            return 0;
         }
         TrieNode<TupleId> root(&file, rootId);
         return root.search(str,0,&file);    
     }

     bool contains(const string& str, const bool acceptPrefix){
         unsigned int pos=0;
         SmiRecordId id = rootId;
         
         TrieNode<TupleId> son;
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


     Trie* clone(){
        Trie * res = new Trie();
        if(rootId!=0){
           TrieNode<TupleId> son(&file,rootId);
           res->rootId = son.copyFromTo( &file, &res->file);
        }
        return res;
     }

     void copyFrom( Trie& src){
        if(rootId!=0){
          file.ReCreate();
          rootId = 0;
        }
        if(src.rootId!=0){
          TrieNode<TupleId> srcSon(&src.file , src.rootId);
          rootId = srcSon.copyFromTo(&src.file, &file);
        }
     }


     static string BasicType(){ 
        return "trie";
     }
  
     static bool checkType(ListExpr t){
       return listutils::isSymbol(t,BasicType());
     }


     SmiFileId getFileId() {
       return file.GetFileId();
     }     
     SmiRecordId getRootId()const {
         return rootId;
     }

     TrieIterator<TupleId> * getEntries(const string& prefix){
        unsigned int pos=0;
        SmiRecordId id = rootId;

        while(pos<prefix.length() && (id!=0)){
           TrieNode<TupleId> s(&file,id);
           id = s.getNext(prefix[pos]);
           pos++;
        } 
        return new TrieIterator<TupleId>(&file,id,prefix);
     }


 protected:
    SmiRecordFile file;
    SmiRecordId   rootId;




    bool getInsertNode(const string& str, 
                       TrieNode<TupleId>& node, 
                       SmiRecordId& nodeId){


         SmiRecordId lastId = 0;
         SmiRecordId id = rootId;
         size_t pos = 0;
         TrieNode<TupleId> son;
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

         // the string is not member of the trie,
         // we extend the trie
         id = lastId; 

         if(rootId == 0 ){
             rootId = son.appendToFile(&file);
             id = rootId;
         }


         TrieNode<TupleId> newNode;
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

};


#endif


