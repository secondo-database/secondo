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
#include "GraphAlgebra.h"


ostream& operator<< (ostream& o, const verticesStruct& m) {
   
   o << "[" << m.pos << ", " << m.succ << ", " 
     << m.inDeg << ", " << m.outDeg << "]";
   return o;
}


ostream& operator<< (ostream& o, const adjStruct& m) {
   
   o << "[" << m.cost << ", " << m.keyInt << "]";
   return o;
}


template<class T>
ostream& operator <<( ostream& o, const AVLNode<T>& n ) {

   if (n.free)
     o << "undefined";
   else
     o << "Node " << n.key << "->" << n.elem 
       << " : Left[" << n.leftDepth << "]: " << n.left 
       << " / Right[" << n.rightDepth << "]: " << n.right 
       << " / Balance = " << n.Balance();
   return o;
}



template<class T>
inline string Print(DBArray<AVLNode<T> >& tree) {

   string s;
   const AVLNode<T>* node;
   
   s = "--------------------------------------\n";
   for (int i=0;i<tree.Size();i++) {
     tree.Get(i,node);
     s = s + (string)i + ". " + *node + "\n"; 
   }
   s += "--------------------------------------\n";
   
   return s;
}



template<class T>
inline int AVLNode<T>::Balance() const {
   
   return rightDepth-leftDepth;
}


template<class T>
AVLNode<T> AVLTree<T>::NewNode (const int key, const T elem) {
   
   AVLNode<T> n;
   n.key = key;
   n.elem = elem;
   n.left = -1;
   n.right = -1;
   n.leftDepth = 0;
   n.rightDepth = 0;
   n.free = false;
   return n;
}

template<class T>
AVLNode<T> AVLTree<T>::NewNode (const AVLNode<T>* n) {
   
   AVLNode<T> node;
   node.key = n->key;
   node.elem = n->elem;
   node.left = n->left;
   node.right = n->right;
   node.leftDepth = n->leftDepth;
   node.rightDepth = n->rightDepth;
   node.free = n->free;
   return node;
}


template<class T>
inline int AVLTree<T>::BalancedTree(DBArray<AVLNode<T> >& tree, 
                             const AVLNode<T>& node, const int root) {

   assert((0 <= root) && (root < tree.Size()));
   
   return (abs(node.Balance())>1) ? AVLTree<T>::Rebalance(tree,root) : root;
}


template<class T>
inline int AVLTree<T>::NumNodes(const DBArray<AVLNode<T> >& tree) {

   return tree.Size();
}


template<class T>
bool AVLTree<T>::DeleteNode(DBArray<AVLNode<T> >& tree, 
                      const int index, const AVLNode<T>* n) {
   
   assert((index >= 0) && (index < tree.Size()));
   
   AVLNode<T> root;
   
   // no node parameter?
   if (n==0) {
     const AVLNode<T>* node;
     tree.Get(index,node);
     root = AVLTree<T>::NewNode(node);
   }
   else
     root = AVLTree<T>::NewNode(n);
   
   // node already deleted?
   if (root.free)
     return false;
     
   root.free = true;
   tree.Put(index,root);
   
   return true;
}


template<class T>
void AVLTree<T>::UpdateNode(DBArray<AVLNode<T> >& tree, 
                                const int index, const T& elem) {
  
   assert((index >= 0) && (index < tree.Size()));
   
   const AVLNode<T>* node;
   tree.Get(index,node);
   
   AVLNode<T> root = AVLTree<T>::NewNode(node);
   root.elem = elem;
   tree.Put(index,root);
}


template<class T>
bool AVLTree<T>::HasKey(const DBArray<AVLNode<T> >& tree, 
                               const int key, const int index) {
   
   assert((index >= -1) && (index < tree.Size()));
   
   // key not found?
   if (index == -1)
     return false;
     
   const AVLNode<T>* node;
   tree.Get(index,node);
   
   // key found?
   if (key == node->key)
     return true;
   
   // key in left tree?
   if (key < node->key)
     // continue search in left tree
     return AVLTree<T>::HasKey(tree,key,node->left);
   
   // continue search in right tree
   return AVLTree<T>::HasKey(tree,key,node->right);
}


template<class T>
bool AVLTree<T>::UpdateKey(DBArray<AVLNode<T> >& tree, const int key, 
                                            const T elem, const int index) {

   assert((index >= -1) && (index < tree.Size()));
   
   // key not found?
   if (index == -1)
     return false;
   
   const AVLNode<T>* node;
   tree.Get(index,node);
   
   // key found?
   if (key == node->key) {
     
     // update node
     AVLNode<T> root = AVLTree<T>::NewNode(node);
     root.elem = elem;
     tree.Put(index,root);
     
     return true;
   }
   
   // key in left tree?
   if (key < node->key)
     // continue search in left tree
     return AVLTree<T>::UpdateKey(tree,key,elem,node->left);
   
   // continue search in right tree
   return AVLTree<T>::UpdateKey(tree,key,elem,node->right);
}


template<class T>
int AVLTree<T>::Rebalance(DBArray<AVLNode<T> >& tree, const int index) {
   
   assert((index >= 0) && (index < tree.Size()));
   
   const AVLNode<T>* bNode;
   const AVLNode<T>* son1;
   const AVLNode<T>* son2;
   
   tree.Get(index,bNode);
   AVLNode<T> rootNode = AVLTree<T>::NewNode(bNode);
   
   // node really unbalanced ?
   assert(abs(bNode->Balance())==2);
   
   int root = index;
   int sonIndex;
   
   if (bNode->Balance()==2) {
     
     // too much nodes on the right hand side
     sonIndex = bNode->right;
     tree.Get(sonIndex,son1);
     AVLNode<T> rootSon = AVLTree<T>::NewNode(son1);
     
     // too much nodes on the right hand side of the son?
     if (son1->Balance()==1) {
       
       // start rotation       
       rootNode.right = son1->left;
       rootNode.rightDepth = son1->leftDepth;
       rootSon.left = index;
       rootSon.leftDepth = max(bNode->leftDepth,son1->leftDepth)+1;
       root = sonIndex;
       
       // update nodes
       tree.Put(index,rootNode);
       tree.Put(root,rootSon);
     }
     else {
       
       // start double rotation
       tree.Get(son1->left,son2);
       AVLNode<T> newRoot = AVLTree<T>::NewNode(son2);
       
       root = son1->left;
       newRoot.left = index;
       newRoot.leftDepth = max(bNode->leftDepth,son2->leftDepth)+1;
       newRoot.right = sonIndex;
       newRoot.rightDepth = max(son1->rightDepth,son2->rightDepth)+1;
       rootNode.right = son2->left;
       rootNode.rightDepth = son2->leftDepth;
       rootSon.left = son2->right;
       rootSon.leftDepth = son2->rightDepth;
       
       // update nodes
       tree.Put(index,rootNode);
       tree.Put(root,newRoot);
       tree.Put(sonIndex,rootSon);
     }
   }
   else {
   
     // too much nodes on the left hand side
     sonIndex = bNode->left;
     tree.Get(sonIndex,son1);
     AVLNode<T> rootSon = AVLTree<T>::NewNode(son1);
     
     // too much nodes on the left hand side of the son?
     if (son1->Balance()==-1) {
       
       // start rotation       
       rootNode.left = son1->right;
       rootNode.leftDepth = son1->rightDepth;
       rootSon.right = index;
       rootSon.rightDepth = max(bNode->rightDepth,son1->rightDepth)+1;
       root = sonIndex;
       
       // update nodes
       tree.Put(index,rootNode);
       tree.Put(root,rootSon);
     }
     else {
       
       // start double rotation
       tree.Get(son1->right,son2);
       AVLNode<T> newRoot = AVLTree<T>::NewNode(son2);
       
       root = son1->right;
       newRoot.right = index;
       newRoot.rightDepth = max(bNode->rightDepth,son2->rightDepth)+1;
       newRoot.left = sonIndex;
       newRoot.leftDepth = max(son1->leftDepth,son2->leftDepth)+1;
       rootNode.left = son2->right;
       rootNode.leftDepth = son2->rightDepth;
       rootSon.right = son2->left;
       rootSon.rightDepth = son2->leftDepth;
       
       // update nodes
       tree.Put(index,rootNode);
       tree.Put(root,newRoot);
       tree.Put(sonIndex,rootSon);
     }
   }
   return root;
}


template<class T>
int AVLTree<T>::InsertKey(DBArray<AVLNode<T> >& tree, 
                          const int key, const T elem, 
                          const int index) {
  
   assert((index >= -1) && (index < tree.Size()));
   
   const AVLNode<T>* node;
   const AVLNode<T>* son;
      
   // tree empty?
   if (index == -1) {
      tree.Append(AVLTree<T>::NewNode(key,elem));  
      return tree.Size()-1;
   }
   
   tree.Get(index,node);
      
   // key already in tree?
   if (key == node->key)
     return -1;
     
   AVLNode<T> thisNode = AVLTree<T>::NewNode(node);
   int newSon;
   int newDepth;
   
   // continue in left son?
   if (key < node->key) {
     
     // no left son?
     if (node->left == -1) {
       
       // insert key as new left son
       tree.Append(AVLTree<T>::NewNode(key,elem));
       thisNode.left = tree.Size()-1;
       thisNode.leftDepth = 1;
       
       tree.Put(index,thisNode);
       return index;
     }
     else {
       
       // try to insert key in left tree
       newSon = AVLTree<T>::InsertKey(tree,key,elem,node->left);
       
       // insert failed?
       if (newSon == -1)
         return -1;
         
       // check left son
       tree.Get(newSon,son);
       newDepth = max(son->rightDepth,son->leftDepth)+1;
       
       // update required?  
       if ((newSon != thisNode.left) || (newDepth != thisNode.leftDepth)) {
         thisNode.leftDepth = newDepth;
         thisNode.left = newSon;
         tree.Put(index,thisNode);
         
         // Node not balanced?
         return AVLTree<T>::BalancedTree(tree,thisNode,index);
       }
       
       return index;
     }
   }
   
   // continue in right son
   else {
     
     // no right son?
     if (node->right == -1) {
       
       // insert key as new right son
       tree.Append(AVLTree<T>::NewNode(key,elem));
       thisNode.right = tree.Size()-1;
       thisNode.rightDepth = 1;
       
       tree.Put(index,thisNode);
       return index;
     }
     else {
       
       // try to insert key in right tree
       newSon = AVLTree<T>::InsertKey(tree,key,elem,node->right);
       
       // insert failed?
       if (newSon == -1)
         return -1;
         
       // check right son
       tree.Get(newSon,son);
       newDepth = max(son->rightDepth,son->leftDepth)+1;
       
       // update required?  
       if ((newSon != thisNode.right) || (newDepth != thisNode.rightDepth)) {
         thisNode.rightDepth = newDepth;
         thisNode.right = newSon;
         tree.Put(index,thisNode);
         
         // Node not balanced?
         return AVLTree<T>::BalancedTree(tree,thisNode,index);
       }
       
       return index;
     }
   
   }
 
}


template<class T>
int AVLTree<T>::DeleteMinKey (DBArray<AVLNode<T> >& tree, 
                             const int index, AVLNode<T>& minNode) {
   
   assert((index >= 0) && (index < tree.Size()));
   
   const AVLNode<T>* node;
   tree.Get(index,node);
   AVLNode<T> root = AVLTree<T>::NewNode(node);
   
   // minimum in current node?
   if (node->left == -1) {
     
     // save and delete the minimum node
     minNode = root;
     AVLTree<T>::DeleteNode(tree,index,&root);
     
     return minNode.right;
   } 
   
   // continue in left son
   root.left = AVLTree<T>::DeleteMinKey(tree,node->left,minNode);
   
   // left son not deleted?
   if (root.left != -1) {
     
     tree.Get(root.left,node);
     root.leftDepth = max(node->leftDepth,node->rightDepth)+1;
   }
   else
     root.leftDepth = 0;
     
   // update current node
   tree.Put(index,root);
   
   return AVLTree<T>::BalancedTree(tree,root,index);   
}


template<class T>
int AVLTree<T>::DeleteKey (DBArray<AVLNode<T> >& tree, 
                                     const int key, const int index) {
   
   assert((index >= -1) && (index < tree.Size()));
   
   // tree empty?
   if (index == -1)
     return -2;
   
   const AVLNode<T>* node;
   const AVLNode<T>* son;
   int newSon;
   int newDepth;
   
   tree.Get(index,node);
   AVLNode<T> root = AVLTree<T>::NewNode(node);
  
   // key not in current node?
   if (key != node->key) {
     
     // continue search in left tree?
     if ((key < node->key) && (node->left != -1)) {
       
       newSon = AVLTree<T>::DeleteKey(tree,key,node->left);
       
       // key not found?
       if (newSon == -2)
         return -2;
       
       // son not deleted?
       if (newSon != -1) {
         tree.Get(newSon,son);
         newDepth = max(son->leftDepth,son->rightDepth)+1;
       }
       else
         newDepth = 0;
         
       // update required?
       if ((newDepth != node->leftDepth) || (newSon != node->left)) {
       
         root.leftDepth = newDepth;
         root.left = newSon;
         
         tree.Put(index,root);  
          
         // node not balanced?
         return AVLTree<T>::BalancedTree(tree,root,index);
       }
       
       return index;
     }
     
     // continue search in right tree?
     if ((key > node->key) && (node->right != -1)) {
     
       newSon = AVLTree<T>::DeleteKey(tree,key,node->right);
       
       // key not found?
       if (newSon == -2)
         return -2;
         
       // son not deleted?
       if (newSon != -1) {
         tree.Get(newSon,son);
         newDepth = max(son->rightDepth,son->leftDepth)+1;
       }
       else
         newDepth = 0;
         
       // update required?
       if ((newDepth != node->rightDepth) || (newSon != node->right)) {
       
         root.rightDepth = newDepth;
         root.right = newSon;
         
         tree.Put(index,root);  
          
         // node not balanced?
         return AVLTree<T>::BalancedTree(tree,root,index);
       }
       
       return index;
     }
     
     // error: key not found
     return -2; 
   }
   
   // key found in current node
      
   // node is a leaf?
   if ((node->left == -1) && (node->right == -1)) {
   
      // delete leaf
      AVLTree<T>::DeleteNode(tree,index,&root);
      
      return -1;
   }
   
   // node has 2 sons?
   if ((node->left != -1) && (node->right != -1)) {
   
      AVLNode<T> min;
      tree.Get(node->right,son);
      
      root.right = AVLTree<T>::DeleteMinKey(tree,node->right,min);
      
      // right son not deleted by DeleteMinKey?
      if (root.right != -1) {
        tree.Get(root.right, son);
        root.rightDepth = max(son->leftDepth,son->rightDepth)+1;
      }
      else
        root.rightDepth = 0;
        
      root.key = min.key;
      root.elem = min.elem;
      tree.Put(index,root);      

      return index;
   }
   
   // node has only one son
   AVLTree<T>::DeleteNode(tree,index,&root);
   
   return ((node->left != -1) ? node->left : node->right);
}


template<class T>
void AVLTree<T>::DeleteKeys(DBArray<AVLNode<T> >& tree, const int index) {

   assert((index >= -1) && (index < tree.Size()));   

   // tree empty?
   if (index == -1)
     return;
     
   const AVLNode<T>* node;
   tree.Get(index,node);
   AVLNode<T> root = AVLTree<T>::NewNode(node);
   
   // nodes in left tree?
   if (node->left != -1)
     AVLTree<T>::DeleteKeys(tree,node->left);
     
   // nodes in right tree?
   if (node->right != -1)
     AVLTree<T>::DeleteKeys(tree,node->right);
   
   // delete current node
   AVLTree<T>::DeleteNode(tree,index,&root);
}


template<class T>
int AVLTree<T>::ReadKeys (const DBArray<AVLNode<T> >& tree,
                          vector<int>* v, const int index) {

   
   assert((index >= -1) && (index < tree.Size()));
   
   // tree empty?
   if (index == -1)
     return 0;
     
   const AVLNode<T>* node;
   tree.Get(index,node);
   
   int n = 0;
   if (node->left != -1)
     n += AVLTree<T>::ReadKeys(tree,v,node->left);
   v->push_back(node->key);
   n++;
   if (node->right != -1)
     n += AVLTree<T>::ReadKeys(tree,v,node->right);
     
   return n;
}


template<class T>
void AVLTree<T>::MapKeys (DBArray<AVLNode<T> >& tree, 
                             int& num, vector<int>& v, 
                             const int index) {
   
   assert((index >= -1) && (index < tree.Size()) && (num >= 0));
   
   // tree empty?
   if (index == -1)
     return;
     
   const AVLNode<T>* node;   
   tree.Get(index,node);
   
   if (node->left != -1)
     AVLTree<T>::MapKeys(tree,num,v,node->left);
   
   AVLNode<T> n = AVLTree<T>::NewNode(node);
   v.push_back(n.key);
   n.key = num;
   tree.Put(index,n);
   
   num++;
   
   if (node->right != -1)
     AVLTree<T>::MapKeys(tree,num,v,node->right);
}


template<class T>
int AVLTree<T>::ReadOptNodes(const DBArray<AVLNode<T> >& tree, 
                             vector<AVLNode<T> >& v, const int root) {

   assert((root >= -1) && (root < tree.Size()));
   
   // tree empty?
   if (root == -1)
     return 0;
     
   const AVLNode<T>* node;
   vector<int> source(0);
   vector<int> target(0);
   
   target.push_back(root);
   
   int n = 0;
   
   // any nodes on next level?
   while (target.size()>0) {
     
     source = target;
     target.clear();
     
     // add all sons of source nodes to target
     for (unsigned int i = 0;i<source.size();i++) {
       
       tree.Get(source[i],node);
       v.push_back(AVLTree<T>::NewNode(node));
       n++;
       
       // left son?
       if (node->left != -1)
         target.push_back(node->left);
       
       // right son?
       if (node->right != -1)
         target.push_back(node->right);
     }
   }
   return n;
}


template<class T>
int AVLTree<T>::ReadNodes (const DBArray<AVLNode<T> >& tree, 
                            vector<AVLNode<T> >& v, const int index) {
   
   assert((index >= -1) && (index < tree.Size()));
   
   // tree empty?
   if (index == -1)
     return 0;
     
   const AVLNode<T>* node;
   tree.Get(index,node);
   
   int n = 0;
   if (node->left != -1)
     n += AVLTree<T>::ReadNodes(tree,v,node->left);
   v.push_back(AVLTree<T>::NewNode(node));
   n++;
   if (node->right != -1)
     n += AVLTree<T>::ReadNodes(tree,v,node->right);
     
   return n;
}


template<class T>
int AVLTree<T>::ReadNode(const DBArray<AVLNode<T> >& tree, AVLNode<T>& n, 
                         const int key, const int index) {

   assert((index >= -1) && (index < tree.Size()));
   
   // key not found?
   if (index == -1)
     return -1;
   
   const AVLNode<T>* node;
   tree.Get(index,node);
   
   // key found?
   if (key == node->key) {
      
      // copy node and return index
      n = *node;
      return index;
   }
   
   // key in left tree?
   if (key < node->key)
     // continue search in left tree
     return AVLTree<T>::ReadNode(tree,n,key,node->left);
     
   // continue search in right tree
   return AVLTree<T>::ReadNode(tree,n,key,node->right);
}


template<class T>
int AVLTree<T>::ReplaceKey(DBArray<AVLNode<T> >& tree, 
                           const int key, const int newKey, const int root) {

   assert((root >= -1) && (root < tree.Size()));
   
   AVLNode<T> node;
   
   // key and newKey ok?
   if (AVLTree<T>::HasKey(tree,newKey,root) || 
       !AVLTree<T>::ReadNode(tree,node,key,root))
     return -2;
   
   int newRoot = AVLTree<T>::DeleteKey(tree,key,root);
   
   // delete failed?
   if (newRoot == -2)
     return -2;
     
   return AVLTree<T>::InsertKey(tree,newKey,node.elem,root);
}

