/*
0 An AVL Tree class

This file contains the implementation of a standard AVL tree.
Additionally some specialized functions are provided.


//[_] [\_]

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

template<typename arg, typename result>
class unary_functor{
  public:
    virtual result operator()(const arg& a) const = 0;
    virtual ~unary_functor(){}
};


template<class Obj>
class StdComp{
   public:
     static bool smaller(const Obj& o1, const Obj& o2){
       return o1 < o2;
     }
     static bool equal(const Obj& o1, const Obj& o2){
       return o1 == o2;
     }
     static bool greater(const Obj& o1, const Obj& o2){
       return o1 > o2;
     }
};



namespace avltree{


/*
0 Forward declaration of the AVLTree class

*/
template <class contenttype, class Comparator = StdComp<contenttype> >
class AVLTree;


/*
1 class Node

This class represents a single node of 
an AVL tree.

*/
template<class contenttype, class Comparator>
class AvlNode{
friend class AVLTree<contenttype, Comparator>;
public:

/*
1.0 Constructor

This constructor creates a new leaf with the given content.

*/
   AvlNode(const contenttype& content1): content(content1),left(0),
                                         right(0),height(0){
      __AVL_TRACE__
   }
/*
1.0 Copy constructor

Creates a depth copy of this node.

*/
  AvlNode(const AvlNode<contenttype, Comparator>& source):
    content(source.content), left(0), right(0),height(source.height){
     __AVL_TRACE__
     this->left = source.left==NULL?NULL:new AvlNode(*source.left);
     this->right = source.right==NULL?NULL:new AvlNode(*source.right);
   }

/*
1.0 Assignment Operator



*/
  AvlNode<contenttype,Comparator>& operator=(
        const AvlNode<contenttype,Comparator>&  source){
     __AVL_TRACE__
     this->content = source.content;
     this->left =   source.left
                  ? new AvlNode<contenttype,Comparator>(source.left)
                  :NULL;
     this->right =   source.right
                   ? new AvlNode<contenttype,Comparator>(source.right)
                   :NULL;
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
      left = NULL;
      right = NULL;
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
   AvlNode<contenttype,Comparator> const* getLeftSon()const{
      __AVL_TRACE__
      return left;
   }

/*
1.1 getRightSon

Returns the right subtree.

*/
   AvlNode<contenttype,Comparator> const* getRightSon()const{
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

const contenttype* getContent() const{
   return &content;
}

private:
/*
1.2 Data Members

*/
   contenttype content;
   AvlNode<contenttype,Comparator>* left;
   AvlNode<contenttype,Comparator>* right;
   int height; // required for the balance computation
};


/*
2 AVL Tree

This class provides an AVL Tree.

*/

template<class contenttype, class Comparator>
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
   AVLTree(const AVLTree<contenttype, Comparator>& T){
      __AVL_TRACE__
      if(T.root==NULL)
        root = NULL;
      else
        root = new AvlNode<contenttype,Comparator>(*T.root);
   }

/*
2.3 Destructor

*/
   ~AVLTree(){
      __AVL_TRACE__
      if(root){
         delete root;
         root = NULL;
      }
   }

/*
2.4 Assignment operator

*/
    AVLTree<contenttype, Comparator>& operator=(
             const AVLTree<contenttype, Comparator> source){
       __AVL_TRACE__
       if(root){
          delete root;
       }
       root =   source.root
              ? new AvlNode<contenttype,Comparator>(source.root) 
              : NULL;
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
~insert2~

inserts x into the tree and returns a pointer to the stored element.
If x is already an element of the tree, a pointer to the stored element
is returned.

*/
const contenttype* insert2(const contenttype& x){
   __AVL_TRACE__
   const contenttype* result=0;
   root = insert2(root,x, result);
   root->updateHeight();
   return result; 
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
2.3 remove[_]smallest

Removes the smallest object stored in the tree fulfilling the
given predicate. The return value indicates whether such an 
entry is found within the tree. The runtime of this operation
is linearly to the count of entries within the tree.

*/
bool remove_smallest(const unary_functor<contenttype, bool>& pred){
  __AVL_TRACE__
  bool found = false;
  root = remove_smallest(root,pred,found);
  if(root!=NULL){
     root->updateHeight();
  }
  return found;
}

/*
2.4 removeAll

Calls removes smallest until no object fulfills the given condition.
The return values is the number of deleted entries.

*/
unsigned int removeAll(const unary_functor<contenttype, bool>& pred){
   unsigned int count = 0;
   while(remove_smallest(pred)) { 
     count++; 
   }
   return count;
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
~GetMember~

Returns a pointer to the entry ~m~ in the tree for which ~x~==~m~ holds.
If the element is not found, the return value is NULL. After calling this 
function ~left~ and ~right~ points to the element stored in the tree which
is the left and right neighbour of x respectively. If a neighbour don't exist,
the corresponding parameter is set to NULL.


*/
const contenttype* getMember(const contenttype& x, 
                             const contenttype*& left,
                             const contenttype*& right) const{
  __AVL_TRACE__
  left = 0;
  right = 0;
  return getMember(root,x,left,right);
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
       __AVL_TRACE__
       this->thestack = it.thestack;
    }

    iterator& operator=(const iterator& it){
       __AVL_TRACE__
       this->thestack = it.thestack;
       return *this; 
    }

/*
~Get~

The ~Get~ function returns the value currently under this iterator.

*/
     contenttype const* Get() const{
        __AVL_TRACE__
        if(thestack.empty()){
          return 0;
        } else {
          const AvlNode<contenttype,Comparator>* elem = thestack.top();
          return (elem->getContent());
        }
     }
/*
~Next~

The ~Next~ function sets the iterator to the next element. 
If no more elements are available, the result will be __false__
otherwise   __true__.

*/
   bool Next(){
     __AVL_TRACE__
     if(thestack.empty()){
        return false;
     }
     const AvlNode<contenttype,Comparator>* elem = thestack.top();
     const AvlNode<contenttype,Comparator>* son;
     if( (son = elem->getRightSon())){
        thestack.pop(); // the current node is processed
        thestack.push(son);
        while((son = son->getLeftSon())){
          thestack.push(son);
        }
        return true;
     } else { // there is no right son
        thestack.pop();
        return !thestack.empty();
     }
   }

/*
~Constructor~

Creates an iterator for the given tree. The position 
is set to the smallest entry in the tree.

*/
  iterator(const AvlNode<contenttype,Comparator>* root){
     __AVL_TRACE__
     const AvlNode<contenttype,Comparator>* son = root;
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
  iterator(const AvlNode<contenttype,Comparator>* root, const contenttype min){
     __AVL_TRACE__
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
     __AVL_TRACE__
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
     __AVL_TRACE__
     return Get();
  } 


/*
~hasNext~

This function checks whether after the current element
further elements exist.

*/
  bool hasNext(){
     __AVL_TRACE__
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
 
     stack<const AvlNode<contenttype,Comparator>*> thestack;

/*
~tail~

 This function support the contructor having a mimimum argument.

*/
 const AvlNode<contenttype,Comparator>* tail(
         const AvlNode<contenttype,Comparator>* root, 
         const contenttype& min){
    __AVL_TRACE__
   assert(root);
   thestack.push(root);
   if(Comparator::equal(root->content,min)){
       return root;
   } else if(Comparator::smaller(root->content , min)){
      const AvlNode<contenttype,Comparator>* node = root->getRightSon();
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
      const AvlNode<contenttype,Comparator>* node = root->getLeftSon();
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

void Print(const AvlNode<contenttype,Comparator>* root, ostream& out)const{
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
static AvlNode<contenttype,Comparator>* rotateRight(
       AvlNode<contenttype,Comparator>* root){
   __AVL_TRACE__
   AvlNode<contenttype,Comparator>* y = root->left;
   AvlNode<contenttype,Comparator>* B = y->right;
   AvlNode<contenttype,Comparator>* C = root->right;
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
static AvlNode<contenttype,Comparator>* rotateLeftRight(
         AvlNode<contenttype,Comparator>* root){
   __AVL_TRACE__
    AvlNode<contenttype,Comparator>* x = root;
    AvlNode<contenttype,Comparator>* z = root->left;
    AvlNode<contenttype,Comparator>* y = z->right;
    AvlNode<contenttype,Comparator>* A = z->left;
    AvlNode<contenttype,Comparator>* B = y->left;
    AvlNode<contenttype,Comparator>* C = y->right;
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
static AvlNode<contenttype,Comparator>* rotateLeft(
         AvlNode<contenttype,Comparator>* root){
   __AVL_TRACE__
  AvlNode<contenttype,Comparator>* y = root->right;
  AvlNode<contenttype,Comparator>* B = y->left;
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
static AvlNode<contenttype,Comparator>* rotateRightLeft(
         AvlNode<contenttype,Comparator>* root){
   __AVL_TRACE__
   AvlNode<contenttype,Comparator>* x = root;
   AvlNode<contenttype,Comparator>* z = x->right;
   AvlNode<contenttype,Comparator>* y = z->left;
   AvlNode<contenttype,Comparator>* B1 = y->left;
   AvlNode<contenttype,Comparator>* B2 = y->right;
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
static  AvlNode<contenttype,Comparator>* insert(
        AvlNode<contenttype,Comparator>* root, 
                                  const contenttype& content,bool& success){
   __AVL_TRACE__
   if(root==NULL){ // leaf reached
      success=true;
      return new AvlNode<contenttype,Comparator>(content);
   }
   contenttype c = root->content;
   if(Comparator::equal(c,content)){ //an AVL tree represents a set, do nothing
      success=false;
      return root;
   } else if(Comparator::smaller(content,c)){ // perform the left subtree
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
static  AvlNode<contenttype,Comparator>* insertN(
                              AvlNode<contenttype,Comparator>* root, 
                              const contenttype& content,bool& success,
                              const contenttype*& left,
                              const contenttype*& right){
   __AVL_TRACE__
   if(root==NULL){ // leaf reached
      success=true;
      return new AvlNode<contenttype,Comparator>(content);
   }

   contenttype c = root->content;
   if(Comparator::equal(c,content)){// an AVL tree represents a set, do nothing
     // set the neighbours
     if(root->left){
        AvlNode<contenttype,Comparator>* tmp = root->left;
        while(tmp->right){
          tmp = tmp->right;
        }
        left = &tmp->content;
     }
     if(root->right){
        AvlNode<contenttype,Comparator>* tmp = root->right;
        while(tmp->left){
          tmp = tmp->left;
        }
        right = &tmp->content;
        
     }

      success=false;
      return root;
   } else if(Comparator::smaller(content,c)){ // perform the left subtree
      root->left = insertN(root->left,content,success,left,right);
      if(!right){
        right =  &root->content;
      }
      if(!left && Comparator::smaller(root->content,content)){
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
      if(!right && Comparator::smaller(content,root->content)){
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


static  AvlNode<contenttype,Comparator>* insert2(
                                  AvlNode<contenttype,Comparator>* root, 
                                  const contenttype& content,
                                  contenttype const*& elem){
   __AVL_TRACE__
   if(root==NULL){ // leaf reached
      AvlNode<contenttype,Comparator>* res =  
                   new AvlNode<contenttype,Comparator>(content);
      elem = &res->content;
      return res;
   }
   contenttype c = root->content;
   if(Comparator::equal(c,content)){ //an AVL tree represents a set, do nothing
      elem = &root->content;
      return root;
   } else if(Comparator::smaller(content,c)){ // perform the left subtree
      root->left = insert2(root->left,content,elem);
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
      root->right = insert2(root->right,content,elem);
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
static AvlNode<contenttype,Comparator>* remove(
                     AvlNode<contenttype,Comparator>* root,
                     const contenttype& x, bool& found){
   __AVL_TRACE__
   if(root==NULL){ // nothing found
      found = false;
      return NULL;
   }
   contenttype  value = root->content;
   if(Comparator::smaller(x,value)){
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
   } else if(Comparator::smaller(value,x)){
       root->right = remove(root->right,x,found);
       root->updateHeight();
       if(root->left==NULL){
         return root;
       }
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
        AvlNode<contenttype,Comparator>* res = root->right;
        root->cut();
        delete root;
        return res;
      }
      if(root->right==NULL){
        AvlNode<contenttype,Comparator>* res = root->left;
        root->cut();
        delete root;
        return res;
      }
      AvlNode<contenttype,Comparator>*  minimum(0);
      root->right=deletemin(root->right,minimum);
      minimum->left = root->left;
      minimum->right = root->right;
      root->cut();
      delete root;
      root = minimum;
      root->updateHeight();
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


static AvlNode<contenttype,Comparator>* remove_smallest(
                 AvlNode<contenttype,Comparator>* root,
                 const unary_functor<contenttype, bool>& pred, 
                 bool& found){
   __AVL_TRACE__
   if(root==NULL || found){ // no tree or already removed
      found = false;
      return NULL;
   }
   // try to delete in the left subtree
   root->left = remove_smallest(root->left,pred,found);
   if(found) { // found the the subtree
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
      assert(false);
   } 
       // not found, try this node
   found = pred(root->content);
   if(found){
      if(root->isLeaf()){
        delete root; // free the memory
        return NULL; // delete a single leaf
      } 
      if(root->left==NULL){
        AvlNode<contenttype,Comparator>* res = root->right;
        root->cut();
        delete root;
        return res;
      }
      if(root->right==NULL){
        AvlNode<contenttype,Comparator>* res = root->left;
        root->cut();
        delete root;
        return res;
      }
      AvlNode<contenttype,Comparator>*  minimum(0);
      root->right=deletemin(root->right,minimum);
      minimum->left = root->left;
      minimum->right = root->right;
      root->cut();
      delete root;
      root = minimum;
      root->updateHeight();
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
     assert(false);
   }

   root->right = remove_smallest(root->right,pred,found);
   if(found){
     root->updateHeight();
     if(root->left==NULL){  
        return root; 
     }
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
     assert(false);
   }
   // predicate could not be evaluated to true in the subtree 
   // represented by root
   // keep tree unchanged
   assert(!found);
   return root; 
}




/*
2.1 removeN

Deletes the Node holding the value __content__ from the subtree
rooted by __root__. Set the arguments __left__ and __right__ to the left 
and right neighbour respectively. __left__ and __right__ have to be 
initialized with NULL.
It returns the new root of the created tree.

*/
static AvlNode<contenttype,Comparator>* removeN(
                                 AvlNode<contenttype,Comparator>* root,
                                 const contenttype& x, bool& found,
                                 const contenttype*& left,
                                 const contenttype*& right){
   __AVL_TRACE__
   if(root==NULL){ // nothing found
      found = false;
      return NULL;
   }
   contenttype  value = root->content;
   if(Comparator::smaller(x,value)){
      root->left = removeN(root->left,x,found,left,right);
      if(!right){
        right = &root->content;
      } 
      if(!left && Comparator::smaller(value , x)){
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
   } else if(Comparator::smaller(value,x)){
       root->right = removeN(root->right,x,found,left,right);
       if(!left){
          left = &root->content;
       } 
       if(!right && Comparator::smaller(x,value)){
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
        AvlNode<contenttype,Comparator>* r = root->right;
        while(r->left){
           r = r->left;
        }
        right = &r->content;
        AvlNode<contenttype,Comparator>* res = root->right;
        root->cut();
        delete root;
        return res;
      }
      if(root->right==NULL){
        // search for the left neighbour
        AvlNode<contenttype,Comparator>* l = root->left;
        while(l->right){
           l = l->right;
        }
        left = &l->content;
        AvlNode<contenttype,Comparator>* res = root->left;
        root->cut();
        delete root;
        return res;
      }
      // both sons exist

      AvlNode<contenttype,Comparator>* tmp;
      tmp = root->left;
      assert(tmp);
      while(tmp->right){
         tmp = tmp->right;
      }
      left = &tmp->content;

      AvlNode<contenttype,Comparator>* minimum(0);
      root->right=deletemin(root->right,minimum);
      minimum->left = root->left;
      minimum->right = root->right;
      root->cut();
      delete root;
      root = minimum;
      root->updateHeight();
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

Removes the node containing the minimum of the subtree 
stored in root. The node itself is not deleted. The minimum
is returned in the appropriate parameter. The left and right pointer
of mininum a set to be null. 

**/
static AvlNode<contenttype,Comparator>* deletemin(
           AvlNode<contenttype,Comparator>* root,
           AvlNode<contenttype,Comparator>*& minimum){
   __AVL_TRACE__
   if(root->left==NULL){ // this node is to remove
       minimum = root; // save the value
       AvlNode<contenttype,Comparator>* res = root->right;
       root->cut();
       return res;
   } else{ // try to delete more left
      root->left=deletemin(root->left,minimum);
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
static bool member(AvlNode<contenttype,Comparator>const* const root,
                  const contenttype& content){
   __AVL_TRACE__
  if(root==NULL) return false;
  if(Comparator::equal(root->content,content)) return true;
  return   Comparator::smaller(content,root->content)
          ? member(root->left,content) 
          : member(root->right,content);
}


static const contenttype* getMember(
             AvlNode<contenttype,Comparator>const* const root,
             const contenttype& content){
   __AVL_TRACE__
  AvlNode<contenttype,Comparator>const* current = root;
  while(current && !( Comparator::equal(current->content,content))){
      if(Comparator::smaller(content , current->content)){
        current = current->left;
      } else {
        current = current->right;   
      }
  }
  if(!current){ // content not found
     return 0;
  } else {
     return &current->content;
  }
}


static const contenttype* getMember(
            AvlNode<contenttype,Comparator>const* const root,
            const contenttype& x,
            const contenttype*& left,
            const contenttype*& right){

  __AVL_TRACE__
  if(root==NULL) return 0;  // member not found
  if(Comparator::equal(root->content,x)){

     if(root->left){ // search the left neighbour
        AvlNode<contenttype,Comparator>* tmp = root->left;
        while(tmp->right){
           tmp = tmp->right;
        }
        left = &tmp->content;
     }
    

     if(root->right){ // search the right neighbour
        AvlNode<contenttype,Comparator>* tmp = root->right;
        while(tmp->left){
           tmp = tmp->left;
        }
        right = &tmp->content;
     }
     return &root->content;
  }
  if(Comparator::smaller(x , root->content)){
     const contenttype* res = getMember(root->left,x,left,right);
     if(!right){
        right = &root->content;
     }
     return res;
  } else { // x > root->content
     const contenttype* res = getMember(root->right,x,left,right);
     if(!left){
        left = &root->content;
     }
     return res;
  }
}



/*
2.1 Check

*/
static bool Check(AvlNode<contenttype,Comparator>const* const root,
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



*/
static bool CheckCmp(AvlNode<contenttype,Comparator>const* const root,
                     const contenttype& content,
                     bool smaller, ostream& out){

  if(!root){
    return true;
  }
  // check the sons
  if(smaller){
    if(! ( Comparator::smaller(root->content , content))){
        cout << root->content << endl <<" is located in the left subtree of "
             << endl
             << content << " but it's not smaller" << endl;
        return false;
    }
  } else {
    if(! (Comparator::smaller(content , root->content))){
        cout << root->content << endl <<" is located in the right subtree of "
             << endl
             << content << " but it's not greater" << endl;
        return false;
    }
  }

 // check the subtrees
 if(!CheckCmp(root->left, content, smaller, out)){
    return false;
  }
 if(!CheckCmp(root->right, content, smaller, out)){
    return false;
  }
  // do the check for the root node
  return  CheckCmp(root->left,root->content,true,out) &&
          CheckCmp(root->right,root->content,false,out);

}

/*
2.1 GetNearestSmallerOrEqual

Returns a pointer to the biggest object stored in the tree which
is smaller or equal to __pattern__. If there is noc such
object, NULL is returned.

*/

static contenttype const*  GetNearestSmallerOrEqual(
                       AvlNode<contenttype,Comparator> const* const root,
                       const contenttype& pattern) {
   __AVL_TRACE__
   if(root==NULL) return NULL;
   contenttype value = root->content;   
 
   if(Comparator::equal(value,pattern)){ // the nearest is equal to the pattern
      return &root->content;
   }
   if(Comparator::smaller(pattern, value)){ 
          // a value smaller than pattern can only found
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
                             AvlNode<contenttype,Comparator> const* const root,
                             const contenttype& pattern) {
   __AVL_TRACE__

    if(root==NULL) return NULL;
    contenttype value = root->content;
    if(Comparator::equal(value,pattern)){
       return &root->content;
    }
    if(Comparator::smaller(pattern, value)){ 
       // search for a closer object in the left subtree
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
                         AvlNode<contenttype,Comparator> const * const root,
                         const contenttype& pattern) {
   __AVL_TRACE__

  if(root==NULL) return NULL;
  contenttype value = root->content;
  if(Comparator::equal(value,pattern)){
     return GetMax(root->left);
  }
  if(Comparator::smaller(value,pattern)){
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
                         AvlNode<contenttype,Comparator> const * const root,
                         const contenttype& pattern) {
   __AVL_TRACE__
   if(root==NULL) return NULL;
   contenttype value = root->content;
   if(Comparator::equal(value,pattern)){
       return GetMin(root->right);
   }
   if(Comparator::smaller(value,pattern)){ 
        // search in the right tree for greater objects
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
                 AvlNode<contenttype,Comparator> const* const root) {
   __AVL_TRACE__
   if(root==NULL) return NULL;
   AvlNode<contenttype,Comparator> const* tmp; 
        // create tmp to keep root constant
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
                     AvlNode<contenttype,Comparator> const* const root) {
   __AVL_TRACE__
   if(root==NULL) return NULL;
   AvlNode<contenttype,Comparator> const* tmp; 
      // create tmp to keep root constant
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
static  int Size(AvlNode<contenttype,Comparator> const * const root) {
   __AVL_TRACE__
    if(root==NULL) return 0;
    return Size(root->left) + Size(root->right) + 1;
  }


/** 
3 Data Members

3.1 The root of this tree

*/
AvlNode<contenttype,Comparator>*  root;


} ;


} // end of namespace avltree

template<class T,class C>
ostream& operator<<(ostream& o,const avltree::AVLTree<T,C>& tree){
    tree.Print(o);
    return o;
}




#endif
