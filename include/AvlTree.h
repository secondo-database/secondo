/*
0 An AVL Tree class

This file contains the implementation of a standard AVL tree.
Additionally some specialized functions are provided.


*/




#ifndef AVLTREE_H
#define AVLTREE_H

  /*
#define __AVL_TRACE__ cout << __FILE__ << "@" << __LINE__ << ":" << \
        __PRETTY_FUNCTION__ << endl;
  */
//#define __AVL_TRACE__ cout << __FUNCTION__ << endl;

#define __AVL_TRACE__

#include <iostream>
#include <assert.h>
#include <stack>

using namespace std;


/*
0 Forward declaration of the AVLTree class

*/
template <class contenttype>
class AVLTree;


/*
1 class Node

This class represents a single node of 
an AVL tree.

*/
template<class contenttype>
class AvlNode{
friend class AVLTree<contenttype>;
public:

/*
1.0 Constructor

This constructor creates a new leaf with the given content.

*/
   AvlNode(const contenttype& content){
      __AVL_TRACE__
      height=0;
      left=NULL;
      right = NULL;
      this->content = content;
   }
/*
1.0 Copy constructor

Creates a depth copy of this node.

*/
  AvlNode(const AvlNode<contenttype>& source){
     __AVL_TRACE__
     this->content = source.content;
     this->left = source.left==NULL?NULL:new AvlNode(*source.left);
     this->right = source.right==NULL?NULL:new AvlNode(*source.right);
     this->height = source.height;
   }

/*
1.0 Assignment Operator



*/
  AvlNode<contenttype>& operator=(AvlNode<contenttype>&  source){
     __AVL_TRACE__
     this->content = source.content;
     this->left = source.left?new AvlNode<contenttype>(source.left):NULL;
     this->right = source.right?new AvlNode<contenttype>(source.right):NULL;
     this->heigth = source.height;
     return *this;
  }

/*
1.0 Destructor

Deletes the subtree represented by this tree.

*/
  ~AvlNode(){
      __AVL_TRACE__
      if(left) delete left;
      if(right) delete right;
   }    

/*
1.1 Cut

Sets the left and the right son of this node to NULL without
deleting them. This can be useful in deleting a single node
but keeping the subtrees in the memory

*/
   void cut(){
      __AVL_TRACE__
      this->left = NULL;
      this->right = NULL; 
   }


/*
1.1 getLeftSon

returns the left subtree

*/
   AvlNode<contenttype> const* getLeftSon()const{
      __AVL_TRACE__
      return left;
   }

/*
1.1 getRightSon

Returns the right subtree.

*/
   AvlNode<contenttype> const* getRightSon()const{
      __AVL_TRACE__
      return right;
   }

/*
1.1 balance

Computes the balance of this node.

*/
 int balance()const{
      __AVL_TRACE__
      int h1 = left?left->height+1:0;
      int h2 = right?right->height+1:0;
      return h1-h2;
 }


/*
1.1 updateHeight

Computes the height of this node from the height of
the sons. This method should be called whenever the 
sons are changed. Note that only the direct descendants
are used to compute the new height.

*/
void updateHeight(){
   __AVL_TRACE__
   int h1 = left==NULL?0:left->height+1;
   int h2 = right==NULL?0:right->height+1;
   height = max(h1,h2);  
}

/* 
1.1 isLeaf

Returns true when this node have no sons 

*/
bool isLeaf()const{
   __AVL_TRACE__
   return left==NULL && right==NULL;
}

private:
/*
1.2 Data Members

*/
   contenttype content;
   AvlNode<contenttype>* left;
   AvlNode<contenttype>* right;
   int height; // required for the balance computation
};


/*
2 AVL Tree

This class provides an AVL Tree.

*/

template<class contenttype>
class AVLTree{
public:

/*
2.1 Constructor

Creates an empty avl tree.   

*/
   AVLTree(){
     __AVL_TRACE__
     root = NULL;
   }
/*
2.2 Empty

Removes all entries from the tree.

*/
 void  Empty(){
      if(root){
        delete root;
      }
      root=NULL;
   }


/*
2.2 Copy Constructor

Creates a depth copy of the argument.

*/
   AVLTree(const AVLTree<contenttype>& T){
      __AVL_TRACE__
      if(T.root==NULL)
        root = NULL;
      else
        root = new AvlNode<contenttype>(*T.root);
   }

/*
2.3 Destructor

*/
   ~AVLTree(){
      __AVL_TRACE__
      delete root;
   }

/*
2.4 Assignment operator

*/
    AVLTree<contenttype>& operator=(const AVLTree<contenttype> source){
       __AVL_TRACE__
       delete root;
       source.root?new AvlNode<contenttype>(source.root) : NULL;
       return  *this;
    }


/*
2.5 Check for emptyness

*/
    bool IsEmpty()const{
       __AVL_TRACE__
       return root==NULL;
    }

/*
2.6 Number of Elements 

*/
    int Size() const{
       __AVL_TRACE__
       return Size(root);
    }

/*
2.7 Height of the tree

*/
   int GetHeight() const{
     __AVL_TRACE__
     return root->height;
   }


/*
2.8 check

This function is for debugging only. 
Error messages are written to out.

*/
   bool Check(ostream& out)const{
     return Check(root,out);
   }



/* 
2.1 Insert

Inserts an object into this tree.

*/
bool insert(const contenttype& x){
   __AVL_TRACE__
   bool success;
   root = insert(root,x,success);
   root->updateHeight();
   return success;
}

/*
2.1 insertN

This insert functions inserts the element provided as first parameter
into the tree. The other parameters are set to the direct neighbour 
in the tree. This means __left__ points to the greatest element in the
tree which is smaller than __x__ and __right__ points to the smallest 
element in the  tree greater than __x__. If such element(s) not exist(s),
the corresponding parameter is set to 0. 

*/
bool insertN(const contenttype& x,      // in
             const contenttype*& left,   // out
             const contenttype*& right){ // out
   __AVL_TRACE__
   bool success;
   left  = 0;
   right = 0;
   root = insertN(root,x,success,left,right);
   root->updateHeight();
   return success;
}



/*
2.2 remove

Deletes __x__ from the tree. 

*/
bool remove(const contenttype& x){
  __AVL_TRACE__
  bool found = false;
  root = remove(root,x,found);
  if(root!=NULL){
     root->updateHeight();
  }
  return found;
}

/*
2.1 removeN

Deletes __x__ from the tree. __left__ and __right__ are set to the 
neighbours __x__ in the tree. If the neighbours not exist or __x__
is not an element of the tree, the corresponding parameter is set to 
be NULL.

*/
bool removeN(const contenttype& x, 
             const contenttype*& left, 
             const contenttype*& right){
  __AVL_TRACE__
  bool found = false;
  left  = 0;
  right = 0;
  root = removeN(root,x,found,left,right);
  if(root){
    root->updateHeight();
  }
  return found;
}


/*
2.3 member

Checks whether __x__ is contained in the tree. 

*/
bool member(const contenttype& x) const{
   __AVL_TRACE__
   return member(root,x);
}


const contenttype* getMember(const contenttype& x) const{
   __AVL_TRACE__
   return getMember(root,x);
}

/*
2.4 GetNearestSmallerOrEqual

This function returns a pointer to the biggest object
store din the tree which is smaller or equal to __pattern__.

*/
contenttype const * GetNearestSmallerOrEqual(const contenttype& pattern)const {
   __AVL_TRACE__
   return GetNearestSmallerOrEqual(root,pattern);
}
 
/*
2.5 GetNearestGreaterOrEqual

This function works similar to the GetNearestSmaller function but
returns the nearest entry to pattern which is greater or equal to it.

*/

contenttype const* GetNearestGreaterOrEqual(const contenttype& pattern)const{
   __AVL_TRACE__
   return GetNearestGreaterOrEqual(root,pattern);
}


/*
2.6 GetMin

This function returns the minimum value stored in the tree.
If the tree is empty, NULL is returned.

*/
contenttype const* GetMin()const{
   __AVL_TRACE__
   return GetMin(root);
}

/*
2.8 GetMax

This returns a pointer to the maximum value stored in the tree or NULL
if the tree is empty.

*/
contenttype const* GetMax()const{
   __AVL_TRACE__
    return GetMax(root);
}

/*
2.9 GetNearestSmaller

Returns a pointer to the biggest entry in the tree which is smaller 
than the argument or NULL if there is no such object.

*/
contenttype const* GetNearestSmaller(const contenttype& pattern) const{
   __AVL_TRACE__
   return GetNearestSmaller(root,pattern);
}

/*
2.10 GetNearestGreater

This function works symmetrically to the ~GetNearestSmaller~ function.

*/
contenttype const* GetNearestGreater(const contenttype& pattern) const{
   __AVL_TRACE__
   return GetNearestGreater(root,pattern);
}


/*
~print~

Prints this tree to the give ostream. 
The format is understand by the tree viewer of Secondo's Javagui.

*/
void Print(ostream& out)const{
   out << "( tree (" << endl;
   Print(root, out);
   out << "))" << endl;
} 


class iterator;
/*
~begin~

This function returns an iterator over this tree.

*/
iterator begin(){
   iterator it(root);
   return it;
}


/*
~tail~

This function creates an iterator which is positioned to the
fisrt element within the tree which is equals to or greater than
the given minimum.

*/
iterator tail(contenttype min){
   iterator it(root,min);
   return it;
}


/*
~iterator~

This inner class provides an iterator for sorted iterating 
trough the tree.

*/
class iterator{

  public:

    iterator(const iterator& it){
       this->thestack = it.thestack;
    }

    iterator& operator=(const iterator& it){
       this->thestack = it.thestack;
       return *this; 
    }

/*
~Get~

The ~Get~ function returns the value currently under this iterator.

*/
     contenttype const* Get() const{
        if(thestack.empty()){
          return 0;
        } else {
          const AvlNode<contenttype>* elem = thestack.top();
          return &(elem->content);
        }
     }
/*
~Next~

The ~Next~ function sets the iterator to the next element. 
If no more elements are available, the result will be __false__
otherwise   __true__.

*/
   bool Next(){
     if(thestack.empty()){
        return false;
     }
     const AvlNode<contenttype>* elem = thestack.top();
     const AvlNode<contenttype>* son;
     if( (son = elem->getRightSon())){
        thestack.push(son);
        while((son = son->getLeftSon())){
          thestack.push(son);
        }
        return true;
     } else { // there is no right son
        // go up until the stack is empty or the entry is
        // greater than the current element
        contenttype content = elem->content;
        thestack.pop();
        while(!thestack.empty()){
           const AvlNode<contenttype>* father = thestack.top();
           if(father->content > content){
              return true;
           } else {
              thestack.pop();
           }
        }
        return false;
     }
   }

/*
~Constructor~

Creates an iterator for the given tree. The position 
is set to the smallest entry in the tree.

*/
  iterator(const AvlNode<contenttype>* root){
     const AvlNode<contenttype>* son = root;
     while(son){
       thestack.push(son);
       son = son->getLeftSon();   
     }
  }

/*
~Contructor~

Creates a new iterator for root. The position is set to the first
entry within the tree which is equals to or greater than min.

*/
  iterator(const AvlNode<contenttype>* root, const contenttype min){
     if(root){
        tail(root,min);
     }
  }

/*
~Increment operator~

This operator sets this iterator to the next position 
in the tree.

*/
  iterator& operator++(int){
      Next();
      return *this;
  }

/*
~Dereference operator~

This operator returns the element currently under the 
iterator's cursor. If no more elements exist, NULL
is returned.

*/  
  const contenttype* operator*(){
     return Get();
  } 


/*
~hasNext~

This function checks whether after the current element
further elements exist.

*/
  bool hasNext(){
    int size = thestack.size();
    if(size==0){ // no elements
      return false;
    } 
    if(size>1){ 
       return true;
    }
    return (thestack.top()->getRightSon()!=0);
  }

/*
~onEnd~

The ~onEnd~ function checks whether the iterator does not have
any elements, i.e. whether Get or [*] would return NULL. 

*/
  bool onEnd(){
   return thestack.empty();
  }



  private:
/*
~Data member~

 We manage stack to be able to go up in the tree.

*/
 
     stack<const AvlNode<contenttype>*> thestack;

/*
~tail~

 This function support the contructor having a mimimum argument.

*/
 const AvlNode<contenttype>* tail(const AvlNode<contenttype>* root, 
                                  const contenttype& min){
   assert(root);
   thestack.push(root);
   if(root->content==min){
       return root;
   } else if(root->content < min){
      const AvlNode<contenttype>* node = root->getRightSon();
      if(node){
        node = tail(node,min);
        if(!node){
           thestack.pop();
           return 0;
        } else {
           return node;
        }
      } else {
        // the subtree specified by root contains only
        // elements smaller than min
        thestack.pop();
        return 0;
      } 
   } else { // root.content > min 
      // in the left subtree may be an element nearer to min
      const AvlNode<contenttype>* node = root->getLeftSon();
      if(!node){ // no better element available
         return root;
      } else {
         node = tail(node,min);
         if(node){ // found a better element
           return node;
         } else {
           return root;
         }
      }
   }
 } 



};





private:

void Print(const AvlNode<contenttype>* root, ostream& out)const{
  if(!root){
     out << " '' ";
     return;
  }
  out << "'"<<root->content <<"'";
  out << "(";
  Print(root->getLeftSon(),out);
  out << ")";
  out << "(";
  Print(root->getRightSon(),out);
  out << ")";
}


/*
2.11 rotateRight

Rotates a node right and returns the new root.
Note there is no check wether the sons required for this
operations exists

*/
static AvlNode<contenttype>* rotateRight(AvlNode<contenttype>* root){
   __AVL_TRACE__
   AvlNode<contenttype>* y = root->left;
   AvlNode<contenttype>* B = y->right;
   AvlNode<contenttype>* C = root->right;
   root->left=B;
   root->right=C;
   root->updateHeight();
   y->right=root;
   y->updateHeight();
   return y;
}

/* 
2.1 rotateLeftRight

Performs a left right double rotation around root

*/
static AvlNode<contenttype>* rotateLeftRight(AvlNode<contenttype>* root){
   __AVL_TRACE__
    AvlNode<contenttype>* x = root;
    AvlNode<contenttype>* z = root->left;
    AvlNode<contenttype>* y = z->right;
    AvlNode<contenttype>* A = z->left;
    AvlNode<contenttype>* B = y->left;
    AvlNode<contenttype>* C = y->right;
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
2.1 rotateLeft

Performs a left rotation around root. 

*/
static AvlNode<contenttype>* rotateLeft(AvlNode<contenttype>* root){
   __AVL_TRACE__
  AvlNode<contenttype>* y = root->right;
  AvlNode<contenttype>* B = y->left;
  root->right = B;
  root->updateHeight();
  y->left=root;
  y->updateHeight();
  return y;
}

/*
2.1 rotateRightLeft

performs a right left double rotation around root 

*/
static AvlNode<contenttype>* rotateRightLeft(AvlNode<contenttype>* root){
   __AVL_TRACE__
   AvlNode<contenttype>* x = root;
   AvlNode<contenttype>* z = x->right;
   AvlNode<contenttype>* y = z->left;
   AvlNode<contenttype>* B1 = y->left;
   AvlNode<contenttype>* B2 = y->right;
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
2.1 insert

This function inserts __content__ into the subtree given by root.

It returns the root of the new tree.

*/
static  AvlNode<contenttype>* insert(AvlNode<contenttype>* root, 
                                  const contenttype& content,bool& success){
   __AVL_TRACE__
   if(root==NULL){ // leaf reached
      success=true;
      return new AvlNode<contenttype>(content);
   }
   contenttype c = root->content;
   if(c==content){ // an AVL tree represents a set, do nothing
      success=false;
      return root;
   } else if(content<c){ // perform the left subtree
      root->left = insert(root->left,content,success);
      root->updateHeight();
      if(abs(root->balance())>1){ // rotation or double rotation required
         // check where the overhang is
         if(root->left->balance()>0){ // single rotation is sufficient
            return rotateRight(root); 
         }
         if(root->left->balance()<0){
            return rotateLeftRight(root);
         } 
         assert(false); // should never be reached
         return NULL; 
      } else{ // root remains balanced
          return root;
      }
   } else{
   // content > c => insert at right 
      root->right = insert(root->right,content,success);
      root->updateHeight();
      if(abs(root->balance())>1){
        if(root->right->balance()<0){ // LeftRotation
           return rotateLeft(root);
        }
        if(root->right->balance()>0){
           return rotateRightLeft(root);
        }
        assert(false); // should never be reached
        return NULL;
      } else{ // no rotation required
           return root;
     }
   }
}


/*
2.1 insertN

This function inserts __content__ into the subtree given by root.

It returns the root of the new tree. Left and right are set to the 
left (right) neighbour of the new element (or NULL) if there are not
present. __left__ and __right__ must be initialized with 0 to ensure a
correct result for them.

*/
static  AvlNode<contenttype>* insertN(AvlNode<contenttype>* root, 
                              const contenttype& content,bool& success,
                              const contenttype*& left,
                              const contenttype*& right){
   __AVL_TRACE__
   if(root==NULL){ // leaf reached
      success=true;
      return new AvlNode<contenttype>(content);
   }

   contenttype c = root->content;
   if(c==content){ // an AVL tree represents a set, do nothing
     // set the neighbours
     if(root->left){
        AvlNode<contenttype>* tmp = root->left;
        while(tmp->right){
          tmp = tmp->right;
        }
        left = &tmp->content;
     }
     if(root->right){
        AvlNode<contenttype>* tmp = root->right;
        while(tmp->left){
          tmp = tmp->left;
        }
        right = &tmp->content;
        
     }

      success=false;
      return root;
   } else if(content<c){ // perform the left subtree
      root->left = insertN(root->left,content,success,left,right);
      if(!right){
        right =  &root->content;
      }
      if(!left && content>root->content){
        left = &root->content;
      }
      root->updateHeight();
      if(abs(root->balance())>1){ // rotation or double rotation required
         // check where the overhang is
         if(root->left->balance()>0){ // single rotation is sufficient
            return rotateRight(root); 
         }
         if(root->left->balance()<0){
            return rotateLeftRight(root);
         } 
         assert(false); // should never be reached
         return NULL; 
      } else{ // root remains balanced
          return root;
      }
   } else{
   // content > c => insert at right
      root->right = insertN(root->right,content,success,left,right);
      if(!left){
        left = &root->content; 
      } 
      if(!right && content<root->content){
        right = &root->content;
      }
      root->updateHeight();
      if(abs(root->balance())>1){
        if(root->right->balance()<0){ // LeftRotation
           return rotateLeft(root);
        }
        if(root->right->balance()>0){
           return rotateRightLeft(root);
        }
        assert(false); // should never be reached
        return NULL;
      } else{ // no rotation required
           return root;
     }
   }
}





/*
2.1 remove

Deletes the Node holding the value __content__ from the subtree
rooted by __root__.
It returns the new root of the created tree.

*/
static AvlNode<contenttype>* remove(AvlNode<contenttype>* root,
                                 const contenttype& x, bool& found){
   __AVL_TRACE__
   if(root==NULL){ // nothing found
      found = false;
      return NULL;
   }
   contenttype  value = root->content;
   if(x<value){
      root->left = remove(root->left,x,found);
      root->updateHeight();
      if(root->right==NULL){ // because we have deleted  
                            // in the left part, we cannot 
                           // get any unbalanced subtree when right is null
         return root; 
      }
      if(abs(root->balance())>1){ // we have to rotate
          // we know that the height of the left son is smaller than before
          int RB = root->right->balance();

          if(RB<=0){ // rotation is sufficient
             return rotateLeft(root);
          } else{
             return rotateRightLeft(root);
          }
      } else{ // balance of root is ok
        return root;
      }
   } else if(x>value){
       root->right = remove(root->right,x,found);
       root->updateHeight();
       if(abs(root->balance())>1){ // we have to rotate
          int LB = root->left->balance();
          if(LB>=0){
             return rotateRight(root);
          } else{
             return rotateLeftRight(root);
          }
       } else {  // root is balanced
         return root;
       }
   } else { // value == x , value found , we have a lot to do
      found = true;
      if(root->isLeaf()){
        delete root; // free the memory
        return NULL; // delete a single leaf
      } 
      if(root->left==NULL){
        AvlNode<contenttype>* res = root->right;
        root->cut();
        delete root;
        return res;
      }
      if(root->right==NULL){
        AvlNode<contenttype>* res = root->left;
        root->cut();
        delete root;
        return res;
      }
      contenttype c(root->content);
      root->right=deletemin(root->right,c);
      root->updateHeight();
      root->content = c;
      if(abs(root->balance())>1){ // we have to rotate
          int LB = root->left->balance();
          if(LB>=0){
             return rotateRight(root);
          } else{
             return rotateLeftRight(root);
          }
       } else{ // root is balanced
         return root;
       }
  }
}


/*
2.1 removeN

Deletes the Node holding the value __content__ from the subtree
rooted by __root__. Set the arguments __left__ and __right__ to the left 
and right neighbour respectively. __left__ and __right__ have to be 
initialized with NULL.
It returns the new root of the created tree.

*/
static AvlNode<contenttype>* removeN(AvlNode<contenttype>* root,
                                 const contenttype& x, bool& found,
                                 const contenttype*& left,
                                 const contenttype*& right){
   __AVL_TRACE__
   if(root==NULL){ // nothing found
      found = false;
      return NULL;
   }
   contenttype  value = root->content;
   if(x<value){
      root->left = removeN(root->left,x,found,left,right);
      if(!right){
        right = &root->content;
      } 
      if(!left && value < x){
         left = &root->content;
      }
      root->updateHeight();
      if(root->right==NULL){ // because we have deleted  
                            // in the left part, we cannot 
                           // get any unbalanced subtree when right is null
         return root; 
      }
      if(abs(root->balance())>1){ // we have to rotate
          // we know that the height of the left son is smaller than before
          int RB = root->right->balance();

          if(RB<=0){ // rotation is sufficient
             return rotateLeft(root);
          } else{
             return rotateRightLeft(root);
          }
      } else{ // balance of root is ok
        return root;
      }
   } else if(x>value){
       root->right = removeN(root->right,x,found,left,right);
       if(!left){
          left = &root->content;
       } 
       if(!right && value > x){
          right = &root->content;
       }
       root->updateHeight();
       if(abs(root->balance())>1){ // we have to rotate
          int LB = root->left->balance();
          if(LB>=0){
             return rotateRight(root);
          } else{
             return rotateLeftRight(root);
          }
       } else {  // root is balanced
         return root;
       }
   } else { // value == x , value found , we have a lot to do
      found = true;
      if(root->isLeaf()){
        delete root; // free the memory
        return NULL; // delete a single leaf
      } 
      if(root->left==NULL){
        // search the content of the right neighbour
        AvlNode<contenttype>* r = root->right;
        while(r->left){
           r = r->left;
        }
        right = &r->content;
        AvlNode<contenttype>* res = root->right;
        root->cut();
        delete root;
        return res;
      }
      if(root->right==NULL){
        // search for the left neighbour
        AvlNode<contenttype>* l = root->left;
        while(l->right){
           l = l->right;
        }
        left = &l->content;
        AvlNode<contenttype>* res = root->left;
        root->cut();
        delete root;
        return res;
      }
      // both sons exist

      AvlNode<contenttype>* tmp;
      tmp = root->left;
      assert(tmp);
      while(tmp->right){
         tmp = tmp->right;
      }
      left = &tmp->content;

      contenttype c(root->content);
      root->right=deletemin(root->right,c);
      root->updateHeight();
      root->content = c;
      right = &root->content;

      if(abs(root->balance())>1){ // we have to rotate
          int LB = root->left->balance();
          if(LB>=0){
             return rotateRight(root);
          } else{
             return rotateLeftRight(root);
          }
       } else{ // root is balanced
         return root;
       }
  }
}



/* 
2.1 deleteMin

Deletes the minimum value in root  storing its value in value 
the result is the root of the tree with the deleted minimum.

**/
static AvlNode<contenttype>* deletemin(AvlNode<contenttype>* root,
                                    contenttype& value){
   __AVL_TRACE__
   if(root->left==NULL){ // this node is to remove
       value = root->content; // save the value
       AvlNode<contenttype>* res = root->right;
       root->cut();
       delete root;  
       return res;
   } else{ // try to delete more left
      root->left=deletemin(root->left,value);
      root->updateHeight();
      if(abs(root->balance())>1){ // we have to rotate
          // we know that the height of the left son is smaller than before
          // we can follow that a right node must exist
          int RB = root->right->balance();
          if(RB<=0){ // rotation is sufficient
             return rotateLeft(root);
          } else{
             return rotateRightLeft(root);
          }
      } else{
          return root;
      }
  }
}

/*
2.1 Member

Checks whether x is contained in the subtree given by __root__. 

*/
static bool member(AvlNode<contenttype>const* const root,
                  const contenttype& content){
   __AVL_TRACE__
  if(root==NULL) return false;
  if(root->content==content) return true;
  return   content<root->content
          ? member(root->left,content) 
          : member(root->right,content);
}


static const contenttype* getMember(AvlNode<contenttype>const* const root,
                  const contenttype& content){
   __AVL_TRACE__
  if(root==NULL) return 0;
  if(root->content==content) return &root->content;
  return   content<root->content
          ? getMember(root->left,content) 
          : getMember(root->right,content);
}


/*
2.1 Check

*/
static bool Check(AvlNode<contenttype>const* const root,
                  ostream& out) {

   if(!root){
     return true;
   } else {
     if(!Check(root->left,out)){
        return false;
     } 
     if(!Check(root->right,out)){
        return false;
     }
     // check whether height is correct
     int h1 = root->left?root->left->height + 1:0;
     int h2 = root->right?root->right->height +1:0;     
     int h = h1>h2?h1:h2;
     if(!h==root->height){
       out << "height not correct";
       return false;
     }
     // check for balance
     int diff = h1>h2?h1-h2:h2-h1;
     if(diff>1){
        out << "Balance is not correct h1 = " << h1 << ", h2 = " << h2;
        return false;
     }
     return  CheckCmp(root->left,root->content,true,out) &&
             CheckCmp(root->right,root->content,false,out); 
     
   }
}
/*
2.1 CheckCmp

This function helps to implement the ~Check~ function.


*/
static bool CheckCmp(AvlNode<contenttype>const* const root,
                     contenttype content,
                     bool smaller, ostream& out){

  if(!root){
    return true;
  }
  if(!CheckCmp(root->left,content,smaller,out)){
      return false;
  }
  if(!CheckCmp(root->right,content,smaller,out)){
      return false;
  }
  if(smaller){
    if(root->content < content){
      return true;
    } else {
      out << root->content << " should be smaller than " << content;
      return false;
    }
  } else {
    if((root->content < content) || (root->content==content)){
      out << root->content << " should be greater than " << content;
      return false;
    } else {
      return true;
    }


  }


}
                  



/*
2.1 GetNearestSmallerOrEqual

Returns a pointer to the biggest object stored in the tree which
is smaller or equal to __pattern__. If there is noc such
object, NULL is returned.

*/

static contenttype const*  GetNearestSmallerOrEqual(
                       AvlNode<contenttype> const* const root,
                       const contenttype& pattern) {
   __AVL_TRACE__
   if(root==NULL) return NULL;
   contenttype value = root->content;   
 
   if(value==pattern){ // the nearest is equal to the pattern
      return &root->content;
   }
   if(value>pattern){ // a value smaller than pattern can only found
                      // in the left subtree
      return GetNearestSmallerOrEqual(root->left,pattern);
   }
   // at this point holds value < pattern
   // root is a candidate for the result but possible we
   // can find an entry clooser to pattern in th right subtree
   contenttype const* tmp = GetNearestSmallerOrEqual(root->right,pattern);
   if(tmp){ // found an closer entry
      return tmp;
   } else{ // all entries are greater than pattern
      return &root->content;
   }
}

/*
2.1 GetNearestGreaterOrEqual

The symmetric function to ~GetSmallerOrEqual~.

*/
static contenttype const* GetNearestGreaterOrEqual(
                             AvlNode<contenttype> const* const root,
                             const contenttype& pattern) {
   __AVL_TRACE__

    if(root==NULL) return NULL;
    contenttype value = root->content;
    if(value==pattern){
       return &root->content;
    }
    if(value>pattern){ // search for a closer object in the left subtree
       contenttype const* tmp = GetNearestGreaterOrEqual(root->left,pattern);
       if(tmp){
           return tmp;
       } else{
           return &root->content;
       }  
    }
    // up to now we are smaller -> search in the right subtree
    return GetNearestGreaterOrEqual(root->right,pattern);
}

/*
2.1 GetNearestSmaller

Returns a pointer to the biggest objectzs which is smaller than the
argument. If no such object is stored in the current tree, NULL is
returned.

*/
static contenttype const* GetNearestSmaller(
                             AvlNode<contenttype> const * const root,
                             const contenttype& pattern) {
   __AVL_TRACE__

  if(root==NULL) return NULL;
  contenttype value = root->content;
  if(value==pattern){
     return GetMax(root->left);
  }
  if(value<pattern){
     // this is a candidate search for closer object in right subtree
     contenttype const* tmp = GetNearestSmaller(root->right,pattern);
     if(tmp){
        return tmp;
     } else{
        return &root->content;
     }
  }
  // search for smaller object
  return GetNearestSmaller(root->left,pattern);
}

/*
2.1 GetNearestGreater

This function returns a pointer to the object stored in the tree which
is the smallest one bigger than __pattern__. If pattern is bigger than all
stored objects (or the tree is empty), NULL is returned.

*/
static contenttype const* GetNearestGreater(
                             AvlNode<contenttype> const * const root,
                             const contenttype& pattern) {
   __AVL_TRACE__
   if(root==NULL) return NULL;
   contenttype value = root->content;
   if(value==pattern){
       return GetMin(root->right);
   }
   if(value<pattern){ // search in the right tree for greater objects
      return GetNearestGreater(root->right,pattern);
   }
   contenttype const* tmp = GetNearestGreater(root->left,pattern);
   if(tmp){
      return tmp;
   } else{
      return &root->content;
   }
}



/*
2.1 GetMax

Returns a pointer to the maximum entry or NULL if the tree is empty.

*/

static contenttype const* GetMax(
                            AvlNode<contenttype> const* const root) {
   __AVL_TRACE__
   if(root==NULL) return NULL;
   AvlNode<contenttype> const* tmp; // create tmp to keep root constant
   tmp = root;
   while(tmp->right){
      tmp = tmp->right;
   }
   return &tmp->content;
}

/*
2.1 GetMin

Returns a pointer to the minimum entry in the tree or NULL if the tree
is empty.

*/
static contenttype const* GetMin(
                             AvlNode<contenttype> const* const root) {
   __AVL_TRACE__
   if(root==NULL) return NULL;
   AvlNode<contenttype> const* tmp; // create tmp to keep root constant
   tmp = root;
   while(tmp->left){
      tmp = tmp->left;
   }
   return &tmp->content;
}

/*
2.1 Size

Computes the number of nodes in the given subtree.

*/
static  int Size(AvlNode<contenttype> const * const root) {
   __AVL_TRACE__
    if(root==NULL) return 0;
    return Size(root->left) + Size(root->right) + 1;
  }


/** 
3 Data Members

3.1 The root of this tree

*/
AvlNode<contenttype>*  root;


} ;

template<class T>
ostream& operator<<(ostream& o,const AVLTree<T>& tree){
    tree.Print(o);
    return o;
}


#endif
