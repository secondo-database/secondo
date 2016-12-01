/*

----
This file is part of SECONDO.

Copyright (C) 2016, 
University in Hagen, 
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

*/

#ifndef TTREE_H
#define TTREE_H

#include <assert.h>
#include <cstdlib> 
#include <utility>
#include <iostream>
#include <stack>
#include <queue>
#include <vector>
#include <stdlib.h>
#include <algorithm>

#include "MainMemoryExt.h"



namespace ttree{

  
/*
0. Forward declaration of the TTree class

*/
template <class U, class Comp>
class TTree;


/*
1 class TTreeNode

The class TTreeNode is a class for the nodes of an t-tree.

*/
template<class T, class Comparator>
class TTreeNode{

  template<class U,class Comp> friend class TTree;
    public:
  

/*
1.1 Constructor

This constructor initializes all members

*/
      // creates a new T-Tree node 
      TTreeNode(const size_t _minEntries, const size_t _maxEntries): 
         
        minEntries(_minEntries), maxEntries(_maxEntries), 
        left(0), right(0), count(0), height(0) {
          
          objects = new T*[TTreeNode<T,Comparator>::maxEntries];
          for(size_t i=0;i<TTreeNode<T,Comparator>::maxEntries;i++){
            objects[i] = 0;
          }
        }
        
/*
1.2 Copy constructor

Creates a depth copy of this node.

*/
    TTreeNode(const TTreeNode<T,Comparator>& source):
      objects(source.objects), left(0), right(0), height(source.height){
      this->left = 
        source.left==NULL?NULL:new TTreeNode<T,Comparator>(*source.left);
      this->right = 
        source.right==NULL?NULL:new TTreeNode<T,Comparator>(*source.right);
    }

/*
1.3 Assignment Operator

*/
  TTreeNode<T,Comparator>& operator=(const TTreeNode<T,Comparator>&  source) {
     this->objects = source.objects;
     this->left = source.left
                  ? new TTreeNode<T,Comparator>(source.left)
                  :NULL;
     this->right = source.right
                   ? new TTreeNode<T,Comparator>(source.right)
                   :NULL;
     this->height = source.height;
     return *this;
  }
      
/*
1.4 Destructor

Destroy the subtree rooted by this node

*/
     
      ~TTreeNode() {

        for(size_t i=0; i<count; i++) {
          delete objects[i];
        }
        delete[] objects;
        if(left) delete left;
        if(right) delete right;
        
        left = NULL;
        right = NULL;
      }

      
      
/*
1.5 Methods

~isLeaf~

This function returns true if this node represents a leaf.

*/
  
      inline bool isLeaf() const {
        if(left || right)
          return false;
        return true;
      }

/*
~memSize~

Returns the amount of memory occupied by the subtree represented by this node.

*/

      size_t memSize() const {
        size_t res = sizeof(*this) + 
                     sizeof(void*) * TTreeNode<T,Comparator>::maxEntries;
        
        for(int i=0;i<count;i++){
          res += sizeof(* objects[i]);
        }
        
        if(left)
          res += left->memSize();
        if(right)
          res += right->memSize();
        return res;  
      }

      

/*
~getObject~

Returns the content of this node at position i.

*/
      T* getObject(const int i) const {
        return objects[i];
      }
      

/*
~getMinEntries~

Return the minimum number of entries of this node.

*/
      inline size_t getMinEntries() const{
        return minEntries;
      }
   

/*
~getMaxEntries~

Returns the capacity of this node.

*/
      inline int getMaxEntries() const{
        return maxEntries;
      }

     
/*
~getCount~

Returns the number of entries of this node.

*/
      inline size_t getCount() const{
        return count;  
      }


/*
~getLeftSon~

Returns the parent of this node or 0 if this node is the root of the tree.

*/
      inline TTreeNode<T,Comparator>* getLeftSon(){
        return left;
      }
      

/*
~getRightSon~

Returns the parent of this node or 0 if this node is the root of the tree.

*/
      inline TTreeNode<T,Comparator>* getRightSon(){
        return right;
      }
      

/*
~clear~

Removes the entries from this node. If deleteContent is set to be
true, the sons of this node are destroyed.

*/
      void clear(const bool deleteContent) {
        for(size_t i=0;i<count;i++) {
          if(deleteContent){
            delete objects[i];
          }
          objects[i] = 0;
        }
        if(deleteContent) {
          if(left)
            delete left;
          if(right)
            delete right;
        }
        left = 0;
        right = 0;
        height=0;
        count = 0;
      }

/*
~updateHeight~

Computes the height of this node from the height of
the sons. This method should be called whenever the 
sons are changed. Note that only the direct descendants
are used to compute the new height.

*/
      void updateHeight() {
        int h1 = left==NULL?0:left->height+1;
        int h2 = right==NULL?0:right->height+1;
        height = std::max(h1,h2);  
      }


/*
~balance~

Computes the balance of this node.

*/
      int balance()const{
        int h1 = 0;
        int h2 = 0;
        if(left) 
          h1 = left?left->height+1:0;
        
        if(right)
          h2 = right?right->height+1:0;
        
        return h1-h2;
      }


/*
~print~

write a textual representation for this node to ~out~.

*/
  
      std::ostream& print(std::ostream& out) const{
         out << "(";
      for(int i=0; i<count; i++) {
        
        out << i <<  ":" << " '" << *objects[i] <<"' ";
        
      }
      out << ")";
         return out;
      }


/*

~printtree~

Print the tree rooted by this in tree format.


*/
    
      void printtree(std::ostream& out) const{
          printtree(this,out);
      }


/*
~getMinValue~

Returns the minimum value of a non-empty node.

*/
      inline const T& getMinValue()const{ 
         assert(count>0);
         return *(objects[0]);
      }

/*
~getMaxValue~

Returns the maximum value of a non-empty node.

*/

      inline const T& getMaxValue()const{ 
        assert(count>0);
        return *(objects[count-1]);
      }

/*
~hasSpace~

The function checks whether the node can include a further entry.

*/
    inline bool hasSpace() const{
      return count < maxEntries-1;
    }    
  
/*
~insert~

Inserts a new entry into a node having enough space.
The new entry should be created exclusively for the tree,
this method does not create a copy from it.

*/
    void insert(T* value, std::vector<int>* attrPos){
      assert(hasSpace());
      // search insertion position
      size_t pos = 0; 
      while(     (pos < count) 
            &&!Comparator::greater( *objects[pos],*value,attrPos)){
          pos++;
      }    
      for(size_t i = count ; i > pos; i--){
        objects[i] = objects[i-1];
      }    
      objects[pos] = value;
      count++;
    }   


/*
~replaceGreatest~

Replaces the last element of this node with a new value. The old
last element is returned. If the node is empty, null is returned.

*/
  T* replaceGreatest(T* newValue, std::vector<int>* attrPos){
    T* res;
    if(count==0){
        res = 0;
    } else {
      // remove the greatest object
      res = objects[count-1];
      objects[count-1] = 0;
      count--;
    }
    insert(newValue,attrPos);
    return res;
  }


/*
~checkCount~

This is a pure debugging function checking whether each node in the
tree rooted by this contains a valid number of entries.

*/
   bool checkCount(std::ostream& out) const{
       if(count==0){
           out << "found node without elements" << endl;
           return false;
       }
       if(count > maxEntries){
          out << "found node having more entries than allowed" << endl;
       }
       if(left && right){ // inner node
          if(count < minEntries){
             out << "found inner node having too less entries" << endl;
             return false;
          }
       }

       bool ok = true;
       if(left){
         ok = ok && left->checkCount(out);
       }
       if(right){
         ok = ok && right->checkCount(out);
       }
       return ok;
   }

/*
~checkOrder~

checks whether the tree rooted by this has a valid ordering.

*/

  bool checkOrder(std::vector<int>* attrPos, std::ostream& out)const{
    return checkOrder(0,0,attrPos, out);
  }


/*
~checkHalfLeaf~

Checks whether this has exactly one son.

*/
   inline bool isHalfLeaf() const{
     return ((left==0) && (right!=0)) || ((left!=0) && (right==0));
   }


/*
~hasSpaceFor~

Checks whether this has enough space for including the
entries of rhs.

*/
   inline bool hasSpaceFor(TTreeNode<T,Comparator>* rhs) const{
     assert(rhs);
     assert(this);
     return count + rhs->count <= maxEntries;
   }



/*
~merge~

Moves the entries from right to left. The pointers of the parameters
will be destroyed by this function. This is only possible, if 
one of the arguments is a leaf and the only son of the other parameter.

*/
   static TTreeNode<T,Comparator>* merge(TTreeNode<T,Comparator>* left, 
                                         TTreeNode<T,Comparator>* right,
                                         std::vector<int>* attrPos){
       assert(left);
       assert(right);
       assert(left!=right);

       if(left == right->left){
         assert(!right->right); 
         assert(left->isLeaf());
         right->decouple();; // disconnect nodes
       } else if(right == left->right){
         assert(!left->left);
         assert(right->isLeaf());   
         left->decouple();
       } else {
         assert(false);
       } 
       assert(left->hasSpaceFor(right));

       for(size_t i=0;i<right->count; i++){
          left->objects[left->count] = right->objects[i];
          left->count++;
          right->objects[i] = 0;
       }
       TTreeNode<T,Comparator>* res = left;
       left = 0;
       right->count = 0;
       delete right;
       right = 0;
       return res;
   }

/*
~remove~

Removes an entry from this node. If the entry is not present,
false is returned. The removed object will be returned in
parameter object.

*/
   bool remove(T& value, std::vector<int>* attrPos){
      int index = find(value,attrPos);
      if(!Comparator::equal(value, *objects[index],attrPos)){
         return false;
      }
      delete objects[index];
      count--;
      for(size_t i=index; i<count; i++){
        objects[i] = objects[i+1];
      }      
      objects[count] = 0;
      return true;
   } 


/*
~deleteMin~

Removes the minimum element of a node. The element is returned, not destroyed.

*/

   T* deleteMin(){
     assert(count>0);
     T* res = objects[0];
     count--;
     for(size_t i=0;i<count; i++){
       objects[i] = objects[i+1];
     } 
     objects[count] = 0;
     return res;
   }



      
    protected:

/*
1.6 Member variables

*/
      size_t minEntries;
      size_t maxEntries;
      TTreeNode<T,Comparator>* left;
      TTreeNode<T,Comparator>* right;
      size_t count;
      size_t height;
      T** objects;  


/*
~find~

Searches for a specified element within this node using a binary search. 
Returns the index where the element is or should be.

*/
      int find(const T& value, const std::vector<int>* attrPos)const{
         int min = 0;
         int max = count-1;
         while(min < max){
            int mid = (min + max) / 2;
            if(Comparator::equal(value,*objects[mid],attrPos)){
               return mid;
            }
            if(Comparator::smaller(value,*objects[mid], attrPos)){
              max = mid - 1;
            } else {
              min = mid + 1;
            }
         }
         return min;
      }


/*
~decouple~

Decouples any child from this node.

*/
      void decouple(){
         left=0;
         right=0;
         height=0;
      }

/*
~printtree~

Support function.

*/

      static void printtree(const TTreeNode<T,Comparator>* root, 
                            std::ostream& out, bool isRoot = true){
        if(isRoot){
          out << "( tree ";
        }
        if(!root){
           out << "' '" << endl;
        } else {
          out << "(";
          out << "'";
          for(size_t i=0;i<root->getCount();i++){
             if(i>0) out << ", ";
             out << *(root->getObject(i));
          }
          out << "'  ";
          printtree(root->left,out,false);
          printtree(root->right,out, false);
          out << " )";
        }
        if(isRoot){
          out << ")";
          out << endl;
        }
    }
   


/*
~checkOrder~

Support function.

*/
   bool checkOrder(const T* min, const T* max, std::vector<int>* attrPos,
                   std::ostream& out) const{
       // check internal order
       for(size_t i=0;i<count-1; i++){
          if(Comparator::greater(*objects[i], *objects[i+1],attrPos)){
             out << "found invalid order within a single node" << endl;
             return false;
          }
       }
       if(min && Comparator::smaller(getMinValue(),*min, attrPos) ){
          out << "found value of a node smaller than allowed" << endl;
          return false; 
       }
       if(max && Comparator::greater(getMaxValue(), *max, attrPos)){
          out << "found value of a node greater than allowed" << endl;
          return false; 
       }
       bool ok = true;
       if(left){
          ok = ok && left->checkOrder(min, objects[0], attrPos,out);
       }
       if(right){
          ok = ok && right->checkOrder(objects[count-1], max, attrPos,out);
       }
       return ok;
   }


};




/*
2 Class Iterator

This inner class provides an iterator for sorted iterating 
trough the tree.

*/
template<class T, class Comparator>
class Iterator{

  public:
 

    Iterator():stack(),pos(0){}

      
/*
2.1 Constructors

Creates an iterator for the given tree. The position 
is set to the smallest entry in the tree.

*/    
    Iterator(TTreeNode<T,Comparator>* root) {
      TTreeNode<T,Comparator>* son = root;
      while(son){
        stack.push(son);
        son = son->getLeftSon();   
      }
      pos = 0;
    }

/*

2.2 Tail constructor

Creates a constructor for a tree using a 
minimum value.

*/
    Iterator(TTreeNode<T,Comparator>* root,
             const T& minV){
       TTreeNode<T,Comparator>* son = root;
       pos = 0; 
       while(son){
         if(Comparator::smaller(minV,son->getMinValue(),0)){
            // value smaller than all entries
            stack.push(son);
            son = son->getLeftSon();
         } else if(Comparator::greater(minV,son->getMaxValue(),0)){
           // value greater than all entries
           son = son->getRightSon();
         } else {
           // value is inside the node
           stack.push(son);
           while(pos<son->getCount()){
             if(!Comparator::smaller(*(son->getObject(pos)),minV,0)){
                return;
             }    
             pos++;
           }    
           // should never be reached
         }    
       }    
   }


    
/*
2.2 Copy Contructor

*/    
    Iterator(const Iterator& it){
       this->stack = it.stack;
       this->pos = it.pos;         
    }

    
/*
2.2 Assignment Operator

*/ 
    Iterator& operator=(const Iterator& it){
       this->stack = it.stack;
       this->pos = it.pos;
       return *this; 
    }

/*
2.3 Increment operator

This operator sets this iterator to the next position 
in the tree.

*/
    Iterator& operator++(int){
      next();
      return *this;
    }

/*
2.4 Dereference operator

This operator returns the element currently under the 
iterator's cursor. If no more elements exist, NULL
is returned.

*/  
    T operator*() {
       assert(!stack.empty());
       const TTreeNode<T,Comparator>* elem = stack.top();
       assert(elem);
       assert(pos < elem->getCount());
       return *(elem->getObject(pos));
    } 
    
     
/*
~next~

The ~next~ function sets the iterator to the next element. 
If no more elements are available, the result will be __false__
otherwise __true__.

*/
   bool next(){
     if(stack.empty()){
        return false;
     }
     // there are elemnts in the node left
     TTreeNode<T,Comparator>* elem = stack.top();
     if(pos < elem->getCount()-1) {
       pos++;
       return true;
     }

     stack.pop(); // the current node is processed
     pos = 0;     // try to find an unprocessed node
     // go to first element in next node
     TTreeNode<T,Comparator>* son;
     if((son = elem->getRightSon())) {
        stack.push(son);
        while((son = son->getLeftSon())){
          stack.push(son);
        }
        return true;
     } else { // there is no right son
        return !stack.empty();
     }
   }
  

/*
~hasNext~

This function checks whether after the current element
further elements exist.

*/
    
    bool hasNext() const{
      int size = stack.size();
      if(size==0){ // no elements
        return false;
      } 
      if(size>1){ // at least one nn processed node 
        return true;
      }
      TTreeNode<T,Comparator>* elem = stack.top();
      if(pos < elem->getCount()-1){ // node has unprocessed  elements
        return true;
      } 
      return (elem->getRightSon()!=0);
    }

    
/*
~end~

The ~end~ function checks whether the iterator does not have
any elements, i.e. whether Get or [*] would return NULL. 

*/
    bool end() const{
      return stack.empty();
    }


  private:
 
     std::stack<TTreeNode<T,Comparator>*> stack;       
     size_t pos;     
};


/*
3 class TTree

This is the main class of this file. It implements a mainmemory-based TTree.

*/
template<class T, class Comparator>
class TTree {

  typedef TTreeNode<T, Comparator> Node;


  public:

/*
3.1 Constructor

Creates an empty tree.

*/
    TTree(int _minEntries, int _maxEntries): 
         minEntries(_minEntries), maxEntries(_maxEntries), 
         root(0), entryCount(0) {}

/*
3.2 Destructor

Destroys this tree.

*/
    ~TTree(){
       if(root!=NULL) {
         delete root;
         root = NULL;
         entryCount = 0;
       }
     }


/*
3.3 Methods

~begin~

This function creates an iterator which is positioned to the
first element in the tree .

*/

    Iterator<T,Comparator> begin() {
      Iterator<T,Comparator> it(root);
      return it;
    }
  

/*
~tail~

This operator returns an iterator starting at a specified element
instead of the begin.


*/ 
    Iterator<T, Comparator> tail(const T& minV){
      Iterator<T,Comparator> it(root,minV);
      return it;
    }
 
   

    const T* GetNearestSmallerOrEqual(const T& v, 
                                      const std::vector<int>* attrPos)const{
        return getNearestSmallerOrEqual(root,v, attrPos);
    }

 

/*
~noEntries~

Returns the number od entries of this tree.

*/
   size_t noEntries() const{
     return entryCount;
   } 


    size_t countEntries() const {
      return countEntries(root);
    }

    
/*
~isEmpty~

Returns true if this tree is empty.

*/    
    bool isEmpty() const {
       return root==NULL;
    }


/* 
~memSize~

Returns the memory size of this tree.

*/   
    size_t memSize() const {
       size_t res = sizeof(*this);  // size of empty tree
       if(root){
         res += root->memSize();
       }
       return res;
    }



/*
~print~

Prints this tree to the give ostream. 
The format is understood by the tree viewer of Secondo's Javagui.

*/

    void Print(std::ostream& out) const {
      out << "( tree (" << std::endl;
      if(root) 
        printttree(root, out);
      else
        out << "empty";
      out << "))" << std::endl;
      
    } 

    void printorel(std::ostream& out) const {
      out << "( tree (" << std::endl;
      if(root) 
        printorel(root, out);
      else
        out << "empty";
      out << "))" << std::endl;
      
    } 
    

    
/*
~update~

Updates a given value in this tree with the second value.

*/
    bool update(T& value, T& newValue, std::vector<int>* attrPos){
      bool success;
      root = update(root,value,newValue,attrPos,success);
      return success;
    }
    
    bool update(T& value, T& newValue) {
      std::vector<int>* attrPos = new std::vector<int>();
      attrPos->push_back(1);
      attrPos->push_back(2);
      bool success = update(root,value,newValue,attrPos,success);
      delete attrPos;
      return success;
    }
    
    
/*
~insert~

Adds object ~o~ to this tree.

*/
    bool insert(T& value, std::vector<int>* attrPos) {
      bool success;
      root = insert(root,value,attrPos,success);
      root->updateHeight();
      return success;
    }
    
    bool insert(T& value, int i) {
      std::vector<int>* attrPos = new std::vector<int>();
      attrPos->push_back(i);
      bool success = insert(value,attrPos);
      attrPos->clear();
      delete attrPos;
      return success;
    }
    
    bool insert(T& value) {
      std::vector<int>* attrPos = new std::vector<int>();
      attrPos->push_back(1);
      bool success = insert(value,attrPos);
      attrPos->clear();
      delete attrPos;
      return success;
    }
    

/*
~remove~

Removes object ~o~ from this tree.

*/
    bool remove(T& value, std::vector<int>* attrPos){
      bool success;
      if(!root)
        return false;

      root = remove(root,value,attrPos,success);
      if(root) {
        root->updateHeight();
      }
      return success;
    }
    
    bool remove(T& value){
      bool success;
      if(!root) {
        return false;
      }
      std::vector<int>* attrPos = new std::vector<int>();
      attrPos->push_back(1);
      root = remove(root,value,attrPos,success);
      if(root) {
        root->updateHeight();
      }
      delete attrPos;
      return success;
    }
    
    
/*
~clear~


*/
    void clear(const bool deleteContent) {
      if(root){
         root->clear(deleteContent);
         entryCount = 0;
      }
    }


/*
~check~

Checks the structure of the tree

*/  

    bool Check(std::ostream& out){
       if(!root) return true;
       return root->checkCount(out) && root->checkOrder(0,out);
    }

  
    size_t Size(){
      return Size(root);
    }
 
    
   private:
     int minEntries;
     int maxEntries;
     Node* root; 
     size_t entryCount; 

     

     size_t Size(Node* node){
        if(!node) return 0;
        return Size(node->left) + Size(node->right) + 1;
     }

     
/*
~noEntries~

Computes the height of this node from the height of
the sons. This method should be called whenever the 
sons are changed. Note that only the direct descendants
are used to compute the new height.

*/
    size_t countEntries(Node* root) {
      if(root == 0)
        return 0;
      
      return   countEntries(root->left)
             + countEntries(root->right)
             + root->count;
    }

       
     
/*
~insert~

This function inserts __value__ into the subtree given by root.
Duplicates are allowed

It returns the root of the new tree.

*/

    Node* insert(Node* root, 
                 T& value,
                 std::vector<int>* attrPos,
                 bool& success) {
       T* v = new T(value);

       Node* res = insert(root,v,attrPos,success);
       if(!success){
          delete v;
       }

       return res;

    }
    

    Node* insert(Node* root, 
                 T* value,
                 std::vector<int>* attrPos,
                 bool& success) {
      // leaf reached
      if(root == NULL){ 
        root = new Node(minEntries,maxEntries);
        root->insert(value, attrPos);
        entryCount++;
        success = true;
        return root;
      }
     
      if(root->isLeaf() && root->hasSpace()){
         root->insert(value,attrPos);
         success = true;  
         entryCount++; 
         return root;
      }

 
      if(   !Comparator::smaller(*value,root->getMinValue(),attrPos)
         && !Comparator::greater(*value,root->getMaxValue(), attrPos)){
         // bounding node
         if( root->hasSpace()){
           // we have space, insert here
           root->insert(value,attrPos);
           entryCount++;
           success = true;   
           return root;
         } else {
         // there is no space, replace value with the 
         // rightmost value in the node
           value=root->replaceGreatest(value,attrPos);
        }
      }
      
      

      // search in the left subtree
      if(!Comparator::greater(*value, root->getMinValue(), attrPos)){
        root->left = insert(root->left,value,attrPos,success);
        root->updateHeight();
        // rotation or double rotation required
        if(abs(root->balance()) > 1) { 
          // single rotation is sufficient
          if(root->left->balance() > 0) {
            return rotateRight(root); 
          }
          // left-right rotation is required
          if(root->left->balance() < 0) {
            return rotateLeftRight(root,attrPos);
          } 
          assert(false); // should never be reached
          return NULL; 
        } else { // root remains balanced 
          return root;
        }
        assert(false);
      }
      
      // search right subtree
      if(!Comparator::smaller(*value,root->getMaxValue(),attrPos)) {
        root->right = insert(root->right,value,attrPos,success);
        root->updateHeight();
        // rotation or double rotation required
        if(abs(root->balance())>1) {
          // single left rotation sufficient
          if(root->right->balance() < 0) { 
            return rotateLeft(root);
          }

          // right-left rotation required
          if(root->right->balance() > 0){
            return rotateRightLeft(root,attrPos);
          }
          assert(false); // should never be reached
          success = false;
          return NULL;
        } else { 
          return root;
        }
      }
      assert(false);
      return 0;  // should never be reached
    }

   

/*
~printNodeInfo~

Prints information about a single node.

*/ 
    void printNodeInfo(Node* node) {
      std::cout << std::endl << "-->NODE INFO<---------------" << std::endl;
      node->print(std::cout);
      std::cout << std::endl;
      std::cout << "minValue: " << node->minValue << std::endl;
      std::cout << "maxValue: " << node->maxValue << std::endl;
      std::cout << "count: " << node->count << std::endl;
      if(node->isLeaf())
        std::cout << "isLeaf: " << "yes" << std::endl;
      else
        std::cout << "isLeaf: " << "no" << std::endl;
      std::cout << "------------------>NODE INFO<--" << std::endl << std::endl;
    }
    
    
/*
~remove~

This function removes __value__ from the subtree given by root.
Returns the new root of the tree.


*/
    Node* remove(Node* root, 
                 T& value,
                 std::vector<int>* attrPos,
                 bool& success) {
     


      if(Comparator::smaller(value,root->getMinValue(),attrPos)){
         // delete value within the left son of root
         if(!root->left){ // value cannot be found
            success = false;
            return root;  
         }  
         root->left = remove(root->left,value,attrPos,success);
         if(!success){ // value not found, structure soes not change
            return root;
         }
         root->updateHeight();
         if(root->left && !root->right && 
            root->hasSpaceFor(root->left)){ 
            // a half leaf having left son
            return Node::merge(root->left, root, attrPos); 
         }
         if(abs(root->balance())>1) {
            // single left rotation sufficient
            if(root->right->balance() <= 0) { 
               return rotateLeft(root);
            }
            // right-left rotation required
            if(root->right->balance() > 0){
              return rotateRightLeft(root,attrPos);
            }
            assert(false); // should never be reached
            return 0;
         } else {  // balance is ok
          return root;
        }       
        assert(false);
      }

      if(Comparator::greater(value,root->getMaxValue(),attrPos)){
         // delete value in the right son of root
         if(!root->getRightSon()){
            success = false;
            return root;
         }
         root->right = remove(root->right,value,attrPos,success);
         if(!success){
            return root;
         }
         root->updateHeight();
         if(root->right && !root->left
           && root->hasSpaceFor(root->right)){
           return Node::merge(root,root->right, attrPos);
         }
         // rotation or double rotation required
         if(abs(root->balance()) > 1) { 
            // single rotation is sufficient
            if(root->left->balance() >= 0) { 
              return rotateRight(root); 
            }
            // left-right rotation is required
            if(root->left->balance() < 0) {
              return rotateLeftRight(root,attrPos);
            } 
            assert(false); // should never be reached
            return NULL; 
          } else { 
            return root;
          }
          assert(false);
      } 

      // found bounding node
      success = root->remove(value, attrPos);
      if(!success){ // value not found, no structure change
         return root;
      } else {
         entryCount--;
      }
      

      // handle leaf node
      if(root->isLeaf()){
         if(root->count==0){
            delete root;
            return 0;
         } else {
            return root; 
         }
      }

      // handle half leaf
      if(root->isHalfLeaf()){
         if(root->left){
           if(root->hasSpaceFor(root->left) ){
              return Node::merge(root->left, root, attrPos); 
           }
         } else if(root->hasSpaceFor(root->right) ){
              return Node::merge(root, root->right, attrPos); 
         }
         return root;
      }
  
      // root is an inner node
      if(root->count >= root->minEntries){ // structure is ok
        return root;
      }

      // remove the minimum value from the right subtree 
      T* v = deleteMin(root->right, attrPos);
      root->insert(v, attrPos);
      // because the removed value is inserted again, 
      // the entryCount is not changed

      root->updateHeight();

      // handle possible structure changes in right subtree
      if(abs(root->balance()) > 1) { 
         // single rotation is sufficient
         if(root->left->balance() >= 0) { 
            return rotateRight(root); 
         }
         // left-right rotation is required
         if(root->left->balance() < 0) {
           return rotateLeftRight(root,attrPos);
         } 
         assert(false); // should never be reached
         return 0; 
      } else { 
        // balance of root is ok
        return root;
      }

      assert(false);
      return 0;
    }


/*
~deleteMin~

Removes the smallest value of the subtree rooted by root and returns this
value. root is changed too if necessary.

*/

    T* deleteMin(Node*& root, std::vector<int>* attrPos){

       if(root->left){ // there is a left subtree, minimum value is there
         T* res =  deleteMin(root->left, attrPos);
         root->updateHeight();
         if(root->isLeaf()){
           // left subtree is 0 now
           return res;
         }         

         if(root->isHalfLeaf()){
           if(root->left && root->hasSpaceFor(root->left)){
             root = Node::merge(root->left, root,attrPos);
             return res;
           } 
           if(root->right && root->hasSpaceFor(root->right)){
             root=Node::merge(root,root->right, attrPos);
             return res;
           }
         }
         // a inner node, may be a balance is required
         if(abs(root->balance())>1) {
            // single left rotation sufficient
            if(root->right->balance() <= 0) { 
               root =  rotateLeft(root);
               return res;
            }
            // right-left rotation required
            if(root->right->balance() > 0){
              root =  rotateRightLeft(root,attrPos);
              return res;
            }
            assert(false); // should never be reached
            return 0;
         } else {  // balance ok
          return res;
         }       
         assert(false);
         return 0;
       }


       // now root is either a leaf or a half leaf having a 
       // right son
       T* res = root->deleteMin();
       if(root->right && root->hasSpaceFor(root->right) ){
           root = Node::merge(root, root->right, attrPos);
           return res;
       }
       if(root->count==0){ // removed the last entry from a leaf
          delete root;
          root = 0;
       }
       return res;
    }



    
    
/*
~update~

This function updates __value__ in the subtree given by root.

*/
    Node* update(Node* root, 
                 T& value,
                 T& newValue,
                 std::vector<int>* attrPos,
                 bool& success) {
      
      root = remove(root,value,attrPos,success);
      root = insert(root,newValue,attrPos,success);
      return root;  
    }
    
   
/*
~rotateRight~

Performs a right rotation in the subtree given by root.

*/    
    Node* rotateRight(Node* root) {
      Node* y = root->left;
      Node* B = y->right;
      Node* C = root->right;
      root->left=B;
      root->right=C;
      root->updateHeight();
      y->right=root;
      y->updateHeight();
      return y;
    }

    
/*
~rotateRightLeft~

Performs a right-left rotation in the subtree given by root.

*/    
    Node* rotateRightLeft(Node* root, 
                          std::vector<int>* attrPos) {
      
      Node* x = root;
      Node* z = x->right;
      Node* y = z->left;
      Node* B1 = y->left;
      Node* B2 = y->right;
      
      //special case:
      //if y is leaf, move entries from z to y till y has the 
      // minimum number of entries
      // y contains smaller values than z, so we have to
      // move from the beginning of z to the end of y
     

      if(y->isLeaf() && (y->getCount() < y->getMinEntries())){
         size_t entriesToTransfer = y->getMinEntries() - y->getCount();
         assert(z->getCount() > entriesToTransfer);

         // transfer the entries
         for(size_t i=0; i< entriesToTransfer; i++){
            y->objects[y->getCount()+i] = z->objects[i];
         }
         // move entries in z to left
         for(size_t i=0; i< z->getCount() - entriesToTransfer; i++){
            z->objects[i] = z->objects[i+entriesToTransfer];
         } // fill z up with 0
         for(size_t i = z->getCount() - entriesToTransfer ; 
            i< z->getCount(); i++){
            z->objects[i] = 0;
         }
         y->count += entriesToTransfer;
         z->count -= entriesToTransfer;
      }


      x->right=B1;
      x->updateHeight();
      z->left = B2;
      z->updateHeight();
      y->left = x;
      y->right=z;
      y->updateHeight();
      return y;
    }
    
    
/*
~rotateLeft~

Performs a left rotation in the subtree given by root.

*/    
    Node* rotateLeft(Node* root) {
      Node* y = root->right;
      Node* B = y->left;
      root->right = B;
      root->updateHeight();
      y->left=root;
      y->updateHeight();
      return y;
    }

    
/*
~rotateLeftRight~

Performs a left-right rotation in the subtree given by root.

*/     
    Node* rotateLeftRight(Node* root, 
                                             std::vector<int>* attrPos) {
      
      Node* x = root;
      Node* z = root->left;
      Node* y = z->right;
      Node* A = z->left;
      Node* B = y->left;
      Node* C = y->right;
     
      // special case:
      // if y is leaf, move entries from z to y till y is full node
      if(y->isLeaf() && (y->getCount() < y->getMinEntries())) {
        size_t entriesToMove = y->getMinEntries() - y->getCount();
        // create place in the beginning of y
        for(size_t i = y->getCount() ; i>0; i--){
           y->objects[i+entriesToMove -1] = y->objects[i -1];
        }
        // move entries from z to y
        for(size_t i=0;i<entriesToMove; i++){
           size_t z_i = z->getCount()  - entriesToMove + i;
           y->objects[i] = z->objects[z_i];
           z->objects[z_i] = 0;
        }
        y->count += entriesToMove;
        z->count -= entriesToMove;
      }    
      
      z->left=A;
      z->right=B;
      z->updateHeight();
      x->left=C;
      x->updateHeight();
      y->left=z;
      y->right=x;
      y->updateHeight();
      return y;
    }
  
/*
~print~

Prints the subtree given by root to the console.

*/   
    void printttree(const Node* root, 
                    std::ostream& out) const {
      if(!root){
        out << " 'empty' ";
        return;
      }
      
      out << " -->ROOT(";
      for(int i=0; i<root->count; i++) {
        out << i <<  ":" << " '" << *root->objects[i] <<"' ";
      }
      out << ") \n";
      
      out << " ----->LEFT(";
      printttree(root->left,out);
      out << ") \n";
      
      out << "         ------->RIGHT(";
      printttree(root->right,out);
      out << ") \n";
      
    }
    
    
    void printorel(const Node* root, 
                   std::ostream& out) const {
      if(!root){
        out << " 'empty' ";
        return;
      }
      
      out << " -->ROOT(";
      for(int i=0; i<root->count; i++) {
        out << i <<  ":" << " '" << **root->objects[i] <<"' ";
      }
      out << ") \n";
      
      out << " ----->LEFT(";
      printorel(root->left,out);
      out << ") \n";
      
      out << "         ------->RIGHT(";
      printorel(root->right,out);
      out << ") \n";
      
    }


    T const * getNearestSmallerOrEqual(Node* root, const T& v, 
                                const std::vector<int>* attrPos)const{

       T const * cand = 0;
       while(root){
          if(Comparator::smaller(v, root->getMinValue(),attrPos)){
             root = root->getLeftSon();
          } else if(Comparator::greater(v,root->getMaxValue(),attrPos)){
             cand = &root->getMaxValue();
             root = root->getRightSon();
          } else {
             int index = root->find(v, attrPos);
             if(!Comparator::greater(*root->objects[index],v, attrPos)){
               return root->objects[index];
             } else {
               assert(index>0);
               return root->objects[index-1];
             }
          }
       }
       return cand;      
    }


};


} // end namespace

#endif

