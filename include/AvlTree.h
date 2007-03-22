/*
0 An AVL Tree class

This file contains the implementation of a standard AVL tree.
Additionally some specialized search functions are provided.


*/




#ifndef AVLTREE_H
#define AVLTREE_H


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
     this->content = source.content;
     this->left = source.left==NULL?NULL:new AvlNode(source.left);
     this->right = source.right==NULL?NULL:new AvlNode(source.right);
     this->heigth = source.height;
   }

/*
1.0 Assignment Operator



*/
  AvlNode<contenttype>& operator=(AvlNode<contenttype>&  source){
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
      this->left = NULL;
      this->right = NULL; 
   }


/*
1.1 getLeftSon

returns the left subtree

*/
   AvlNode<contenttype> const* getLeftSon()const{
      return left;
   }

/*
1.1 getRightSon

Returns the right subtree.

*/
   AvlNode<contenttype> const* getRightSon()const{
      return right;
   }

/*
1.1 balance

Computes the balance of this node.

*/
 int balance()const{
      int h1 = left!=NULL?left->height+1:0;
      int h2 = right!=NULL?right->height+1:0;
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
   int h1 = left==NULL?0:left->height+1;
   int h2 = right==NULL?0:right->height+1;
   height = max(h1,h2);  
}

/* 
1.1 isLeaf

Returns true when this node have no sons 

*/
bool isLeaf()const{
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
     root = NULL;
   }

/*
2.2 Copy Constructor

Creates a depth copy of the argument.

*/
   AVLTree(const AVLTree<contenttype>& T){
      if(T.root==NULL)
        root = NULL;
      else
        root = new AvlNode<contenttype>(T.root);
   }

/*
2.3 Destructor

*/
   ~AVLTree(){
      delete root;
   }

/*
2.4 Assignment operator

*/
    AVLTree<contenttype>& operator=(const AVLTree<contenttype> source){
       delete root;
       source.root?new AvlNode<contenttype>(source.root) : NULL;
       return  *this;
    }


/*
2.5 Check for emptyness

*/
    bool IsEmpty()const{
       return root==NULL;
    }

/*
2.6 Number of Elements 

*/
    int Size() const{
       return Size(root);
    }

/*
2.7 Height of the tree

*/
   int GetHeight() const{
     return root->height;
   }

/* 
2.1 Insert

Inserts an object into this tree.

*/
void insert(const contenttype& x){
   root = insert(root,x);
   root->updateHeight();
}

/*
2.2 Remove

Deletes x from the tree. 

*/
void remove(const contenttype& x){
  root = remove(root,x);
  if(root!=NULL){
     root->updateHeight();
  }
}

/*
2.3 member

Checks whether x is contained in the tree 

*/
bool member(const contenttype& x) const{
   return member(root,x);
}

/*
2.4 GetNearestSmallerOrEqual

This function returns a pointer to the biggest object
store din the tree which is smaller or equal to __pattern__.

*/
contenttype const * GetNearestSmallerOrEqual(const contenttype& pattern)const {
   return GetNearestSmallerOrEqual(root,pattern);
}
 
/*
2.5 GetNearestGreaterOrEqual

This function works similar to the GetNearestSmaller function but
returns the nearest entry to pattern which is greater or equal to it.

*/

contenttype const* GetNearestGreaterOrEqual(const contenttype& pattern)const{
   return GetNearestGreaterOrEqual(root,pattern);
}


/*
2.6 GetMin

This function returns the minimum value stored in the tree.
If the tree is empty, NULL is returned.

*/
contenttype const* GetMin()const{
   return GetMin(root);
}

/*
2.8 GetMax

This returns a pointer to the maximum value stored in the tree or NULL
if the tree is empty.

*/
contenttype const* GetMax()const{
    return GetMax(root);
}

/*
2.9 GetNearestSmaller

Returns a pointer to the biggest entry in the tree which is smaller 
than the argument or NULL if there is no such object.

*/
contenttype const* GetNearestSmaller(const contenttype& pattern) const{
   return GetNearestSmaller(root,pattern);
}

/*
2.10 GetNearestGreater

This function works symmetrically to the ~GetNearestSmaller~ function.

*/
contenttype const* GetNearestGreater(const contenttype& pattern) const{
   return GetNearestGreater(root,pattern);
}



private:

/*
2.11 rotateRight

Rotates a node right and returns the new root.
Note there is no check wether the sons required for this
operations exists

*/
static AvlNode<contenttype>* rotateRight(AvlNode<contenttype>* root){
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
                                  const contenttype& content){
   if(root==NULL){ // leaf reached
      return new AvlNode<contenttype>(content);
   }
   contenttype c = root->content;
   if(c==content){ // an AVL tree represents a set, do nothing
      return root;
   } else if(content<c){ // perform the left subtree
      root->left = insert(root->left,content);
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
      root->right = insert(root->right,content);
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
2.1 delete

Deletes the Node holding the value __content__ from the subtree
rooted by __root__.
It returns the new root of the created tree.

*/
static AvlNode<contenttype>* remove(AvlNode<contenttype>* root,
                                 const contenttype& x){
   if(root==NULL){ // nothing found
      return NULL;
   }
   contenttype  value = root->content;
   if(x<value){
      root->left = remove(root->left,x);
      root->updateHeight();
      if(root->right==NULL){ // because we have deleted  
                            // in the left part, we cannot 
                           // get any unbalanced subtree when right is null
         return root; 
      }
      if((root->balance())>1){ // we have to rotate
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
       root->right = remove(root->right,x);
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
2.1 deleteMin

Deletes the minimum value in root  storing its value in value 
the result is the root of the tree with the deleted minimum.

**/
static AvlNode<contenttype>* deletemin(AvlNode<contenttype>* root,
                                    contenttype& value){
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
  if(root==NULL) return false;
  if(root->content==content) return true;
  return   content<root->content
          ? member(root->left,content) 
          : member(root->right,content);
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
    if(root==NULL) return 0;
    return Size(root->left) + Size(root->right) + 1;
  }


/** 
3 Data Members

3.1 The root of this tree

*/
AvlNode<contenttype>*  root;


} ;


#endif
