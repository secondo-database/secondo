
//This file is part of SECONDO.

// Copyright (C) 2004-2007, University in Hagen,
// Faculty  of Mathematics and  Computer Science, 
// Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
package gui;

import java.util.Vector;
import tools.Reporter;

// class storing sets of strings
public class Trie{

/** creates an empty trie **/
public Trie(){
   root = null;
}

/** inserts a string to the current trie. **/
public void insert(String word){
  if(root==null){
     root = new TrieNode();
  }
  root.insert(word);
}

/** deletes an entry **/
public void deleteEntry(String word){
   root = TrieNode.deleteEntry(root,word);
}

/** returns all stored extensions of the 
  * given prefix.
  **/
public Vector getExtensions(String prefix){
  Vector result = new Vector();
  if(root!=null){
     root.insertExtensions(prefix, result);
  }
  return result;
}

public String commonPrefix(){
   if(root==null){
      return "";
   } else{
      return root.commonPrefix();
   }

}

private TrieNode root;

private static class TrieNode{
 /** creates an trienode with empty sons **/
  public TrieNode(){
     sons = new TrieNode[size]; 
     for(int i=0;i<size;i++){
        sons[i] = null;
     }
     stop = false;
  }

 /** inserts  a new 'string' **/
  public void insert(String word){
     if(word.length()==0){
       stop = true;
       return;
     }
     char first = word.charAt(0);
     String rest = word.substring(1);
     int pos = (int) first;
     if((pos<0) || (pos>=size)){
       Reporter.writeError("try to insert a char with code" + 
                            pos+ " into a trie, allowed range is [0, "+
                            size + "]");
       return;
     }
     if(sons[pos]==null){
        sons[pos]= new TrieNode();
     }
     sons[pos].insert(rest);
  }

 /** deletes a entry **/
 public static TrieNode deleteEntry(TrieNode orig, String word){
    if(orig ==null){
       return null;
    }    
    if(word.length()==0){ // delete this entry
       orig.stop = false; // delete
       if(orig.isLeaf()){
          return null;
       } else{
          return orig;
       }
    }
    // go down the tree
    char first = word.charAt(0);
    int pos = (int) first;
    if(pos<0 || pos>=size){
      return orig;
    }
    orig.sons[pos] = deleteEntry(orig.sons[pos],word.substring(1));
    if(orig.stop || orig.sons[pos]!=null){
        return orig;
    } else {
        if(orig.isLeaf()){
          return null;
        } else {
          return orig;
        }
    }
 }

 private boolean isLeaf(){
   for(int i=0;i<size;i++){
      if(sons[i]!=null){
         return false;
      }
   } 
   return true;
 } 


  /** returns a prefix which is common to all stored words **/
  public String commonPrefix(){
     String p = "";
     TrieNode root = this;
     boolean done = false;
     while(!done){
        if(root.stop){  
           return p;
        }
        int pos = -1;
        for(int i=0;i<size;i++){
           if(root.sons[i]!=null){
              if(pos<0){
                pos=i;
              } else { // ambigious way found
                 return p;
              }
           }
        }
        if(pos<0){ // leaf reached
           done=true; 
        } else {
           p += (char) pos;
           root = root.sons[pos];
        }
     }     
     return p;
  } 





  /** inserts all stored words beginning with the given prefix into 
    * result.
    **/
   public void insertExtensions(String prefix, Vector result){
      // go down to the prefix if exists
      String rest = prefix;
      TrieNode root = this;
      while(rest.length()>0){
         char first = rest.charAt(0);
         rest = rest.substring(1);
         int pos = (int) first;
         if(pos<0 || pos>=size){ // can't be stored
            return;
         }
         if(root.sons[pos]==null){ // prefix not stored
            return;
         }
         root = root.sons[pos];
      }
      insertStoredWords(root, result, prefix);
   }

 /** Determines recursive all stored string within this node.**/
  private static void insertStoredWords(TrieNode root, Vector res, String path){
      if(root==null){
         return;
      }
      if(root.stop){
          res.add(path);
      }
      for(int i=0;i< size; i++){
         insertStoredWords(root.sons[i], res, path+(char)i);
      }
 }
 



  private TrieNode[] sons;
  boolean stop; // mark the end of a stored word
  private static final int size = 128; // used characters
  
}

}
