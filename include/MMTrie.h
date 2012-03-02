

/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen,
Faculty of Mathematics and Computer Science,
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
//[_] [\_]


[1]  A Main Memory based Trie implementation

[TOC]

*/

#ifndef MMTRIE_H
#define MMTRIE_H

#include <string>
#include <stdlib.h>

namespace mmtrie{

const size_t CHARS = 256;

class TrieNode{
  public: 
   TrieNode(): contained(false){
      memset(sons,0,sizeof(TrieNode*)*CHARS);
   }

  void  destroySubTrees(){
      for(size_t i=0;i<CHARS;i++){
        if(sons[i]!=0){
           sons[i]->destroySubTrees();
           delete sons[i];
           sons[i]=0;
        }
      }
  }

  TrieNode* getSon(unsigned char x) const{
     return sons[x];
  }

   void writeStrings(stringstream& ss, string str){
      if(contained){
         ss << "str" << " ";
      } 
      for(unsigned int i=0;i<CHARS; i++){
        if(sons[i]){
           char c = (char)i;
           string sn = str.append(&c,1);
           sons[i]->writeStrings(ss, sn);     
        }
      }
   }

   bool contained;
   TrieNode* sons[CHARS]; 
};


class Trie{
  public:
     Trie() : root(0) {}

     ~Trie() {
         if(root){
           root->destroySubTrees();
           delete root;
         }
      }

      bool contains(const string& str) const{

         size_t pos = 0;
         TrieNode* node = root;
         while(node!=0 && pos<str.length()){
            char x = str[pos];
            node = node->getSon(x);
            pos++;
         }
         if(node){
           return node->contained;
         } else {
           return false;
         }
      }

      void insert(const string& str){
          size_t size = str.length();
          if(size==0){
            return;
          }
          if(root==0){
             root = new TrieNode();
          } 
          size_t pos = 0;
          TrieNode* node= root;
          while(pos<size){
             unsigned char x = str[pos];
             pos++;
             if(node->sons[x]==0){
               node->sons[x] = new TrieNode();
             }
             node = node->sons[x];
          }
          node->contained = true;
      }

      string concatStrings() const{
         if(!root){
            return "";
         } else {
            stringstream ss;
            root->writeStrings(ss,"");
            return ss.str();
         }
      }     

  private:
      TrieNode* root;

};




} // end of namespace mmtrie

#endif
