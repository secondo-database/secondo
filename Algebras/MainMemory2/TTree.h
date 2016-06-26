/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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
      TTreeNode(const int _minEntries, const int _maxEntries): 
         
        minEntries(_minEntries), maxEntries(_maxEntries), 
        left(0), right(0), count(0), height(0) {
          
          objects = new T*[TTreeNode<T,Comparator>::maxEntries];
          for(int i=0;i<TTreeNode<T,Comparator>::maxEntries;i++){
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
        
        for(size_t i=0; i<maxEntries; i++) {
          if(objects[i]) {
            delete objects[i];
          }
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
  
      bool isLeaf() const {
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
        
        for(int i=0;i<TTreeNode<T,Comparator>::count;i++){
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
      inline int getMinEntries() const{
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
      inline int getCount() const{
        return count;	
      }

/*
~getHeight~

Returns, the height of the tree.

*/
//       inline int getHeight() const {
//         return height;	
//       }
      
      
     

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
~quickSort~

Quicksorts the elements in this node.

*/
      void quickSort(TTreeNode<T,Comparator>* node, 
                     int left, int right, std::vector<int>* attrPos) {
        
        int i = left, j = right;
        T* tmp;
        T* pivot = node->objects[(left + right) / 2];
        
        /* partition */
        while (i <= j) {
          while (Comparator::smaller(*node->objects[i],*pivot,attrPos))
            i++;
          while (Comparator::greater(*node->objects[j],*pivot,attrPos))
            j--;
          
          if (i <= j) {
            tmp = node->objects[i];
            node->objects[i] = node->objects[j];
            node->objects[j] = tmp;
            
            i++;
            j--;
          }
        }

        /* recursion */
        if (left < j)
          quickSort(node, left, j, attrPos);
        if (i < right)
          quickSort(node, i, right, attrPos);
      }
      
      
      
      
      void quickSort(TTreeNode<T,Comparator>* node, 
                     int left, int right) {
        
        int i = left, j = right;
        T* tmp;
        T* pivot = node->objects[(left + right) / 2];
        
        /* partition */
        while (i <= j) {
          while (Comparator::smaller(*node->objects[i],*pivot))
            i++;
          while (Comparator::greater(*node->objects[j],*pivot))
            j--;
          
          if (i <= j) {
            tmp = node->objects[i];
            node->objects[i] = node->objects[j];
            node->objects[j] = tmp;
            
            i++;
            j--;
          }
        }

        /* recursion */
        if (left < j)
          quickSort(node, left, j);
        if (i < right)
          quickSort(node, i, right);
      }


/*
~updateNode~


*/
    void updateNode(std::vector<int>* attrPos) {
      if(count > 1) 
        quickSort(this,0,count-1,attrPos);

      minValue = *objects[0];
      maxValue = *objects[count-1];
    }
    
    void updateNode() {
      minValue = *objects[0];
      maxValue = *objects[count-1];
    }
    
    void updateNode(bool b) {
      if(count > 1) 
        quickSort(this,0,count-1);
      
      minValue = *objects[0];
      maxValue = *objects[count-1];
    }


/*
~clear~

Removes the entries from this node. If deleteContent is set to be
true, the sons of this node are destroyed.

*/
      
      // TODO überprüfen
      void clear(const bool deleteContent) {
        if(this==0)
          return;
        for(int i=0;i<TTreeNode<T,Comparator>::count;i++) {
          if(deleteContent)
            delete objects[i];
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
         TTreeNode<T,Comparator>::count = 0;
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

    protected:


/*
1.6 Member variables

*/
      T** objects;  
      T minValue;
      T maxValue;
      int minEntries;
      int maxEntries;
      int count;
      int height;
      TTreeNode<T,Comparator>* parent;  // TODO überprüfen
      TTreeNode<T,Comparator>* left;
      TTreeNode<T,Comparator>* right;
};




/*
2 Class Iterator

This inner class provides an iterator for sorted iterating 
trough the tree.

*/
template<class T, class Comparator>
class Iterator{

  public:
 

    Iterator():stack(),count(0){}

      
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
      count = 0;
    }

    
/*
Creates a new iterator for root. The position is set to the first
entry within the tree which is equals to or greater than min.

*/
    Iterator(const TTreeNode<T,Comparator>* root, const T min){
      if(root) {
          tail(root,min);
      }
    }   

    
/*
2.2 Copy Contructor~

*/    
    Iterator(const Iterator& it){
       this->stack = it.stack;
       this->count = it.count;         
    }

    
/*
2.2 Assignment Operator

*/ 
    Iterator& operator=(const Iterator& it){
       this->stack = it.stack;
       this->count = it.count;
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
      return get(count);
    } 
    
    
/*
2.5 Methods

~get~

The ~get~ function returns the value currently under this 
iterator at position i.

*/
     T get(int i) const {
        if(stack.empty()){
          return 0;
        } 
        else {
          const TTreeNode<T,Comparator>* elem = stack.top();
          return *elem->getObject(i);
        }
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
     if(count < elem->getCount()-1) {
       count++;
       return true;
     }
     // go to first element in next node
     TTreeNode<T,Comparator>* son;
     if((son = elem->getRightSon())) {
        stack.pop(); // the current node is processed
        stack.push(son);
        while((son = son->getLeftSon())){
          stack.push(son);
        }
        count = 0;
        return true;
     } else { // there is no right son
        stack.pop();
        count = 0;
        return !stack.empty();
     }
   }
  

/*
~hasNext~

This function checks whether after the current element
further elements exist.

*/
    
    bool hasNext(){
      int size = stack.size();
      if(size==0){ // no elements
        return false;
      } 
      if(size>=1){ 
        return true;
      }
      return (stack.top()->getRightSon()!=0);
    }

    
/*
~end~

The ~end~ function checks whether the iterator does not have
any elements, i.e. whether Get or [*] would return NULL. 

*/
    bool end(){
      return stack.empty();
    }

    
/*
~tail~

 This function supports the contructor having a mimimum argument.

*/
    // TODO überprüfen
    const TTreeNode<T,Comparator>* tail(
                        const TTreeNode<T,Comparator>* root, 
                        const T& min) {
      assert(root);
      if(Comparator::equal(root->content,min)) {
          stack.push(root);
          return root;
      } else if(Comparator::smaller(root->content,min)){ 
          const TTreeNode<T,Comparator>* son = root->getRightSon();
          if(!son){
            return 0;
          } else {
            return tail(son,min);
          }
      } else { // root.content > min 
          stack.push(root);
          const TTreeNode<T,Comparator>* son = root->getLeftSon();
          if(son){
            const TTreeNode<T,Comparator>* best = tail(son,min);
            return best?best:root;
          } else {
            return 0;
          }
      }
    } 

  private:
 
     std::stack<TTreeNode<T,Comparator>*> stack;
     size_t count;     
};


/*
3 class TTree

This is the main class of this file. It implements a mainmemory-based TTree.

*/
template<class T, class Comparator>
class TTree {
  public:

/*
3.1 Constructor

Creates an empty tree.

*/
    TTree(int _minEntries, int _maxEntries): 
         minEntries(_minEntries), maxEntries(_maxEntries), root(0) {}

/*
3.2 Destructor

Destroys this tree.

*/
    ~TTree(){
       if(root!=NULL) {
         delete root;
         root = NULL;
       }
     }


/*
3.3 Methods

~begin~

This function creates an iterator which is positioned to the
first element in the tree .

*/

    Iterator<T,Comparator> begin () {
      Iterator<T,Comparator> it(root);
      return it;
    }
    
    
/*
~tail~

This function creates an iterator which is positioned to the
first element within the tree which is equals to or greater than
the given minimum.

*/
    Iterator<T,Comparator> tail(T min) {
      Iterator<T,Comparator> it(root,min);
      return it;
    }
    
    
    
/*
~getHeight~

Returns the height of this tree.

*/
    int getHeight() const{
      return root->getHeight();
    }

    
/*
~noEntries~

Returns the height of this tree.

*/
    int noEntries() {
      int count;
      noEntries(root,count);
      return count;
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

Returns the number of nodes of this tree.

*/
    size_t memSize() const {
       size_t res = sizeof(*this);  // size of empty tree
       if(root){
         res += root->memSize();
       }
       return res;
    }


/*
~getMinValue~

This function returns the minimum value stored in the tree.
If the tree is empty, NULL is returned.

*/

    T getMinValue() {
      return getMinValue(root);
    }
    
    
/*
~getMin~

This function returns the minimum value stored in the tree.
If the tree is empty, NULL is returned.

*/
    TTreeNode<T,Comparator>* getMin() {
      return getMin(root);
    }

    
    
/*
2.8 GetMax

This returns a pointer to the maximum value stored in the tree or NULL
if the tree is empty.

*/
    TTreeNode<T,Comparator>* getMax() {
      return getMax(root);
    }



/*
~print~

Prints this tree to the give ostream. 
The format is understood by the tree viewer of Secondo's Javagui.

*/
    // TODO merge
//     void print(std::ostream& out) const {
//       out << "( tree (" << std::endl;
//       if(root) 
//         print(root, out);
//       else
//         out << "empty";
//       out << "))" << std::endl;
//       
//     } 
    
    void print(std::ostream& out, bool& isTree) const {
      out << "( tree (" << std::endl;
      if(root) 
        print(root, isTree, out);
      else
        out << "empty";
      out << "))" << std::endl;
      
    } 
    
   
    
/*
~update~


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
    
    bool insert(T& value) {
      std::vector<int>* attrPos = new std::vector<int>();
      attrPos->push_back(1);
      bool success = insert(value,attrPos);
      delete attrPos;
      return success;
    }
    
    bool insert(T& value, int i) {
      std::vector<int>* attrPos = new std::vector<int>();
      attrPos->push_back(i);
      bool success = insert(value,attrPos);
      delete attrPos;
      return success;
    }
    
    bool insert(T& value,bool noDuplicate) {
      bool success;
      root = insert(root,value,success,noDuplicate);
      root->updateHeight();
      return success;
    }

    
/*
~remove~

Removes object ~o~ ftom this tree.

*/
    bool remove(T& value, std::vector<int>* attrPos){
      bool success;
      root = remove(root,value,attrPos,success);
      if(root)
        root->updateHeight();
      return success;
    }
    
    
/*
~clear~


*/
    // TODO überprüfen, erzeugt memory Fehler
    void clear(const bool deleteContent) {
      root->clear(deleteContent);
    }

    
   private:
     int minEntries;
     int maxEntries;
     TTreeNode<T,Comparator>* root;  

     
     
/*
~noEntries~

Computes the height of this node from the height of
the sons. This method should be called whenever the 
sons are changed. Note that only the direct descendants
are used to compute the new height.

*/
    int noEntries(TTreeNode<T,Comparator>* root, int &count) {
      if(root == 0)
        return 0;
      
      if(root->left) {
        noEntries(root->left,count);
      }
      if(root->right) {
        noEntries(root->right,count);
      }
      count += root->count;
      return count;
    }

       
     
/*
~insert~

This function inserts __content__ into the subtree given by root.

It returns the root of the new tree.

*/

// TODO zusammenfügen
// TODO Methode für Rotation
    TTreeNode<T,Comparator>* insert(TTreeNode<T,Comparator>* root, 
                                    T& value,
                                    std::vector<int>* attrPos,
                                    bool& success) {
      
      // leaf reached
      if(root == NULL){ 
        root = new TTreeNode<T,Comparator>(minEntries,maxEntries);
        root->objects[0] = new T(value);   // TODO memory leak
        root->count = 1;
        root->updateHeight();
        root->updateNode();
        success = true;
        return root;
      }
      
      // node bounds value and still has room
      if(root->count < root->maxEntries) {
        root->objects[root->count] = new T(value);  // TODO memory leak
        root->count++;      
        root->updateHeight();
        root->updateNode(attrPos);
        success = true;
        return root;
      }
      
      // TODO check the sorting
      // bounding node found, but it is full
      else if(Comparator::greater(value,root->minValue,attrPos) &&
              Comparator::smaller(value,root->maxValue,attrPos)) {

        T* tmp = root->objects[0];     
        root->objects[0] = new T(value); 
        root->minValue = value;
        root->left = insert(root->left,*tmp,attrPos,success);
        root->updateHeight();
        root->updateNode(attrPos);
        // rotation or double rotation required
        if(abs(root->balance()) > 1) { 
          
          // single rotation is sufficient
          if(root->left->balance() > 0) { 
            return rotateRight(root); 
          }
          // left-right rotation is required
          if(root->left->balance() < 0) {
//             std::cout << "rotateLeftRight required" << std::endl;  
            return rotateLeftRight(root,attrPos);
          } 
          assert(false); // should never be reached
          return NULL; 
        } 
        // root remains balanced
        else { 
          return root;
        }
      }
      

      // search in the left subtree
      if(Comparator::smaller(value,root->minValue,attrPos) ||
         Comparator::equal(value,root->minValue,attrPos)) { 
        root->left = insert(root->left,value,attrPos,success);
        root->updateHeight();
        root->updateNode();
        
        // rotation or double rotation required
        if(abs(root->balance()) > 1) { 
          
          // single rotation is sufficient
          if(root->left->balance() > 0) { 
            return rotateRight(root); 
          }
          // left-right rotation is required
          if(root->left->balance() < 0) {
            /*std::cout << "rotateLeftRight required" << std::endl;*/  
            return rotateLeftRight(root,attrPos);
          } 
//           std::cout << "insert Z.1241" << std::endl;
          assert(false); // should never be reached
          return NULL; 
        } 
        // root remains balanced
        else { 
          return root;
        }
      }
      
      // search right subtree
      else if(Comparator::greater(value,root->maxValue,attrPos) ||
              Comparator::equal(value,root->maxValue,attrPos)) {

        root->right = insert(root->right,value,attrPos,success);
        root->updateHeight();
        root->updateNode();
        
        // rotation or double rotation required
        if(abs(root->balance())>1) {
          // single left rotation sufficient
          if(root->right->balance() < 0) { 
            return rotateLeft(root);
          }
          // right-left rotation required
          if(root->right->balance() > 0){
//             std::cout << "rotateRightLeft required" << std::endl; 
            return rotateRightLeft(root,attrPos);
          }
          assert(false); // should never be reached
          return NULL;
        } 
        // no rotation required
        else { 
          return root;
        }
      }
      return 0;  // should never be reached
    }


    TTreeNode<T,Comparator>* insert(TTreeNode<T,Comparator>* root, 
                                    T& value,
                                    bool& success, 
                                    bool noDuplicate) {
      
      
      // leaf reached
      if(root == NULL){ 
        root = new TTreeNode<T,Comparator>(minEntries,maxEntries);
        root->objects[0] = new T(value);   // TODO memory leak
        root->count = 1;
        root->updateHeight();
        root->updateNode();
        success = true;
        return root;
      }
      
      // node bounds value and still has room
      if(root->count < root->maxEntries) {
        root->objects[root->count] = new T(value);  // TODO memory leak
        root->count++;      
        root->updateHeight();
        root->updateNode(true);
        success = true;
        return root;
      }
      
      // bounding node found, but it is full
      else if(Comparator::greater(value,root->minValue) &&
              Comparator::smaller(value,root->maxValue)) {

        T* tmp = root->objects[0];     
        root->objects[0] = new T(value); 
        root->minValue = value;
        root->left = insert(root->left,*tmp,success,true);
        root->updateHeight();
        root->updateNode(true);
        // rotation or double rotation required
        if(abs(root->balance()) > 1) { 
          
          // single rotation is sufficient
          if(root->left->balance() > 0) { 
            return rotateRight(root); 
          }
          // left-right rotation is required
          if(root->left->balance() < 0) {
//             std::cout << "rotateLeftRight required" << std::endl;  
            return rotateLeftRight(root);
          } 
          assert(false); // should never be reached
          return NULL; 
        } 
        // root remains balanced
        else { 
          return root;
        }
      }
      

      // search in the left subtree
      if(Comparator::smaller(value,root->minValue) ||
         Comparator::equal(value,root->minValue)) { 
        root->left = insert(root->left,value,success,true);
        root->updateHeight();
        root->updateNode();
        
        // rotation or double rotation required
        if(abs(root->balance()) > 1) { 
          
          // single rotation is sufficient
          if(root->left->balance() > 0) { 
            return rotateRight(root); 
          }
          // left-right rotation is required
          if(root->left->balance() < 0) {
            /*std::cout << "rotateLeftRight required" << std::endl; */ 
            return rotateLeftRight(root);
          } 
//           std::cout << "insert Z.1241" << std::endl;
          assert(false); // should never be reached
          return NULL; 
        } 
        // root remains balanced
        else { 
          return root;
        }
      }
      
      // search right subtree
      else if(Comparator::greater(value,root->maxValue) ||
              Comparator::equal(value,root->maxValue)) {

        root->right = insert(root->right,value,success,true);
        root->updateHeight();
        root->updateNode();
        
        // rotation or double rotation required
        if(abs(root->balance())>1) {
          // single left rotation sufficient
          if(root->right->balance() < 0) { 
            return rotateLeft(root);
          }
          // right-left rotation required
          if(root->right->balance() > 0){
//             std::cout << "rotateRightLeft required" << std::endl; 
            return rotateRightLeft(root);
          }
          assert(false); // should never be reached
          return NULL;
        } 
        // no rotation required
        else { 
          return root;
        }
      }
      return 0;  // should never be reached
    }
    
    
    
/*
~remove~

*/
    // TODO überprüfen
    TTreeNode<T,Comparator>* remove(TTreeNode<T,Comparator>* root, 
                                    T& value,
                                    std::vector<int>* attrPos,
                                    bool& success) {
      
      // search in the left subtree
      if(Comparator::smaller(value,root->minValue,attrPos)) { 
        root->left = remove(root->left,value,attrPos,success);
        root->updateHeight();
        root->updateNode(attrPos);
        
        // rotation or double rotation required
        if(abs(root->balance()) > 1) { 
          
          // single rotation is sufficient
          if(root->left->balance() > 0) { 
            return rotateRight(root); 
          }
          // left-right rotation is required
          if(root->left->balance() < 0) {
            /*std::cout << "rotateLeftRight required" << std::endl; */ 
            return rotateLeftRight(root,attrPos);
          } 
          assert(false); // should never be reached
          return NULL; 
        } 
        // root remains balanced
        else { 
          return root;
        }
      }
      
      // search right subtree
      else if(Comparator::greater(value,root->maxValue,attrPos)) {
        root->right = remove(root->right,value,attrPos,success);
        root->updateHeight();
        root->updateNode(attrPos);
        
        // rotation or double rotation required
        if(abs(root->balance())>1) {
          // single left rotation sufficient
          if(root->right->balance() < 0) { 
            return rotateLeft(root);
          }
          // right-left rotation required
          if(root->right->balance() > 0){
//             std::cout << "rotateRightLeft required" << std::endl; 
            return rotateRightLeft(root,attrPos);
          }
          assert(false); // should never be reached
          return NULL;
        } 
        // no rotation required
        else { 
          return root;
        }
      }
      
      // bounding node found, searching for value
      else {
        // TODO fertigstellen
        if(root->count == 1 && root->isLeaf()) {
          //root->left->print(std::cout);
          //root->print(std::cout);
          
          //root->parent->left = 0;
          //root->parent->right = 0;
          //TTreeNode<T,Comparator>* node = root;
          //root->objects[root->count-1] = 0;
          //delete[] root->objects;
          root = root->parent;
          root->left = 0;
          root->updateNode(attrPos);
          root->updateHeight();
          
          if(abs(root->balance())>1) {
            if(root->right->balance() < 0) { 
              std::cout << "rotateLeft required" << std::endl; 
              //return rotateLeft(root);
            }
            else if(root->right->balance() > 0){
//               std::cout << "rotateRightLeft required" << std::endl; 
              root = rotateRightLeft(root,attrPos);
            }
          }
          //root->right = 0;
          //printNodeInfo(root);
          //printNodeInfo(root->right);
          //printNodeInfo(root->left);
          //remove(root->left,0);
          //root->left->objects[0] = 0;
          //root->left = 0;
          //root = node;
          //root->updateHeight();
          success = true;
          return root;
          
        }
        
        if(root->count > root->minEntries || root->isLeaf()) {
          
          for(int i=0; i<root->count; i++) {
            if(Comparator::equal(value,*root->objects[i],attrPos)) {
              // swap value to be deleted with last element
              swap(root,i,root,root->count-1);
              // delete last element
              remove(root,root->count-1);
              root->updateNode(attrPos);
              success = true;
            }
          }
          return root;
        }
        else { // count < minEntries
          for(int i=0; i<root->count; i++) {
            if(Comparator::equal(value,*root->objects[i],attrPos)) {
              
              TTreeNode<T,Comparator>* node = getMax(root->left);
              root->objects[i] = node->objects[node->count-1];    
              root->left->parent = root;
              // TODO: Pointer überprüfen
              root->left = remove(root->left,*node->objects[node->count-1],
                                  attrPos,success);
              root->updateNode(attrPos);
              node->updateNode(attrPos);
              success = true;
              return root;
            }
          }
        }
      }
      return 0;  // should never be reached
    }
    
    
/*
~update~

*/
    TTreeNode<T,Comparator>* update(TTreeNode<T,Comparator>* root, 
                                    T& value,
                                    T& newValue,
                                    std::vector<int>* attrPos,
                                    bool& success) {
      
      root = remove(root,value,attrPos,success);
      root = insert(root,newValue,attrPos,success);
      
      return root;  
    }
    
    
/*
~swap~

*/      
    void swap(TTreeNode<T,Comparator>* root, int i, 
              TTreeNode<T,Comparator>* node, int j) {
      T* tmp = root->objects[i];     
      root->objects[i] = node->objects[j];     
      node->objects[j] = tmp;
    }
    
    
/*
~remove~

*/    
    void remove(TTreeNode<T,Comparator>* root, int i) {
      
      root->objects[i] = 0;
      delete root->objects[i];
      root->count--;
      
      if(root->count < 1) 
        delete root;
      
    }
   
   
/*
~rotateRight~

*/    
    TTreeNode<T,Comparator>* rotateRight(TTreeNode<T,Comparator>* root) {
      TTreeNode<T,Comparator>* y = root->left;
      TTreeNode<T,Comparator>* B = y->right;
      TTreeNode<T,Comparator>* C = root->right;
      root->left=B;
      root->right=C;
      root->updateHeight();
      y->right=root;
      y->updateHeight();
      return y;
    }

    
/*
~rotateRightLeft~

*/    
// TODO zusammenfügen
    TTreeNode<T,Comparator>* rotateRightLeft(TTreeNode<T,Comparator>* root, 
                                             std::vector<int>* attrPos) {
      TTreeNode<T,Comparator>* x = root;
      TTreeNode<T,Comparator>* z = x->right;
      TTreeNode<T,Comparator>* y = z->left;
      TTreeNode<T,Comparator>* B1 = y->left;
      TTreeNode<T,Comparator>* B2 = y->right;
      
      //special case:
      //if y is leaf, move entries from z to y till y is full node
      if(y->isLeaf()) {
        while(y->count < y->maxEntries) {
          // TODO: reverseSort
          T* tmp = z->objects[z->count-1];
          y->objects[y->count] = tmp;
          z->objects[z->count-1] = 0;
          z->count--;
          y->count++;
          y->updateNode(attrPos);
        }
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
    
    TTreeNode<T,Comparator>* rotateRightLeft(TTreeNode<T,Comparator>* root) {
      TTreeNode<T,Comparator>* x = root;
      TTreeNode<T,Comparator>* z = x->right;
      TTreeNode<T,Comparator>* y = z->left;
      TTreeNode<T,Comparator>* B1 = y->left;
      TTreeNode<T,Comparator>* B2 = y->right;
      
      //special case:
      //if y is leaf, move entries from z to y till y is full node
      if(y->isLeaf()) {
        while(y->count < y->maxEntries) {
          // TODO: reverseSort
          T* tmp = z->objects[z->count-1];
          y->objects[y->count] = tmp;
          z->objects[z->count-1] = 0;
          z->count--;
          y->count++;
          y->updateNode(true);
        }
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

*/    
    TTreeNode<T,Comparator>* rotateLeft(TTreeNode<T,Comparator>* root) {
      TTreeNode<T,Comparator>* y = root->right;
      TTreeNode<T,Comparator>* B = y->left;
      root->right = B;
      root->updateHeight();
      y->left=root;
      y->updateHeight();
      return y;
    }

    
/*
~rotateLeftRight~

*/     
// TODO zusammenfügen
    TTreeNode<T,Comparator>* rotateLeftRight(TTreeNode<T,Comparator>* root, 
                                             std::vector<int>* attrPos) {
      TTreeNode<T,Comparator>* x = root;
      TTreeNode<T,Comparator>* z = root->left;
      TTreeNode<T,Comparator>* y = z->right;
      TTreeNode<T,Comparator>* A = z->left;
      TTreeNode<T,Comparator>* B = y->left;
      TTreeNode<T,Comparator>* C = y->right;
      
      // special case:
      // if y is leaf, move entries from z to y till y is full node
      if(y->isLeaf()) {
        while(y->count < y->maxEntries) {
          T* tmp = z->objects[z->count-1];
          y->objects[y->count] = tmp;
          z->objects[z->count-1] = 0;
          z->count--;
          y->count++;
          y->updateNode(attrPos);
        }
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
    
    TTreeNode<T,Comparator>* rotateLeftRight(TTreeNode<T,Comparator>* root) {
      TTreeNode<T,Comparator>* x = root;
      TTreeNode<T,Comparator>* z = root->left;
      TTreeNode<T,Comparator>* y = z->right;
      TTreeNode<T,Comparator>* A = z->left;
      TTreeNode<T,Comparator>* B = y->left;
      TTreeNode<T,Comparator>* C = y->right;
      
      // special case:
      // if y is leaf, move entries from z to y till y is full node
      if(y->isLeaf()) {
        while(y->count < y->maxEntries) {
          T* tmp = z->objects[z->count-1];
          y->objects[y->count] = tmp;
          z->objects[z->count-1] = 0;
          z->count--;
          y->count++;
          y->updateNode(true);
        }
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

*/   
    // TODO merge
//   void print(const TTreeNode<T,Comparator>* root, std::ostream& out) const {
//       if(!root){
//         out << " 'empty' ";
//         return;
//       }
//       
//       out << " -->ROOT(";
//       for(int i=0; i<root->count; i++) {
//         out << i <<  ":" << " '" << **root->objects[i] <<"' ";
//       }
//       out << ") \n";
//       
//       out << " ----->LEFT(";
//       print(root->left,out);
//       out << ") \n";
//       
//       out << "         ------->RIGHT(";
//       print(root->right,out);
//       out << ") \n";
//       
//     }
    
    
    void print(const TTreeNode<T,Comparator>* root, 
               bool& isTree, std::ostream& out) const {
      if(!root){
        out << " 'empty' ";
        return;
      }
      
      out << " root(";
      if(isTree) {
        for(int i=0; i<root->count; i++) {
          out << i <<  ":" << " '" << *root->objects[i] <<"' ";
        }
      }
      else {
        for(int i=0; i<root->count; i++) {
          out << i <<  ":" << " '" << **root->objects[i] <<"' ";
        }
      }
      out << ") \n";
      
      out << " left(";
      print(root->left,isTree,out);
      out << ") ";
      
      out << " right(";
      print(root->right,isTree,out);
      out << ")";
      
    }
 


/*
~getMax~

Returns a pointer to the maximum node or NULL if the tree is empty.

*/

    TTreeNode<T,Comparator>* getMax(TTreeNode<T,Comparator>* root) {
      if(root==NULL) 
        return NULL;
      
      TTreeNode<T,Comparator>* tmp = root; 
      
      while(tmp->right){
          tmp = tmp->right;
      }
      return tmp;
    }

/*
~getMin~

Returns a pointer to the minimum node in the tree or NULL if the tree
is empty.

*/
    TTreeNode<T,Comparator>* getMin(TTreeNode<T,Comparator>* root) {
      if(root==NULL) 
        return 0;
      
      TTreeNode<T,Comparator>* tmp = root; 
      while(tmp->left){
          tmp = tmp->left;
      }
      return tmp;
    }
    
    
/*
~getMinValue~

Returns a pointer to the minimum entry in the tree or NULL if the tree
is empty.

*/    
    T getMinValue(TTreeNode<T,Comparator>* root) {
      if(root==NULL) 
        return 0;
      
      TTreeNode<T,Comparator>* tmp = root; 
      while(tmp->left){
          tmp = tmp->left;
      }
      return tmp->minValue;
    }



};


} // end namespace

#endif


