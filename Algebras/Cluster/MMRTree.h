
/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen,
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


[1]  A Main Memory based R-Tree implementation

[TOC]

*/

#ifndef MYRTREE_H
#define MYRTREE_H


#include <string.h>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>

#include "RectangleAlgebra.h"


namespace myrtree {



/*
1 The Class ~Node~


This class represents a single node within an R-tree.
There is no distinction between nodes holding the objects ids,
leafs nodes and inner nodes. All nodes are represented by this 
class.

*/


template<unsigned dim> class Rtree;

template<unsigned dim>
class Node{
friend class Rtree<dim>;
public:
/*
1.1 Constructors


This constructor creates a leaf node.

*/
  Node(Rectangle<dim> abox, long id):
    min(-1), max(-1),count(id), box(abox), sons(0){ }  


/*
Copy constructor.

*/
  Node(const Node& src):min(src.min), max(src.max), 
    count(src.count),box(src.box){
    if(src.sons){
       sons = new Node[max+1];
       for(int i=0;i<count;i++){
          sons[i] = src.sons[i];
       }
    } else {
      sons = 0;
    }
  }

/*
1.2 The assignment operator.

*/

  Node& operator=(const Node& src){
    this->min = src.min;
    this.max = src.max;
    this.count = src.count;
    this.box = src.box;
    if(src.sons){
       sons = new Node[max+1];
       for(int i=0;i<count;i++){
          sons[i] = src.sons[i];
       }
    } else {
      sons = 0;
    }
  }

/*
1.3 Destructor

The destructor does not remove any sons of the node. Just the 
array managing the sons are removed.

*/
  
  ~Node(){
      if(sons){
         delete [] sons;
      }
   }

/*
1.4 getLabel

*/
  string getLabel() const{
    stringstream s; 
    if(max <0){
      s << "' o: " << count << "'";  
    } else {
      s << "' (";
      for(unsigned int i=0;i<dim;i++){
        if(i!=0){
           s << ", ";
        }
        s << box.MinD(i);
      }
      s << " ) -> (";
      for(unsigned int i=0;i<dim;i++){
        if(i!=0){
           s << ", ";
        }
        s << box.MaxD(i);
      }
      s << ") '";
    }
    return s.str();
  }


   

private:

/*
1.4 Data members

*/

  int min;            // minimum count of entries
  int max;            // maximum count of entries <0 for "object nodes"
  long count;         // current count of entries
                      // used as id for objects nodes 
  Rectangle<dim> box; // the bounding box
  Node<dim>** sons;   // array of sons, 0 for object nodes

/*
1.5 Private functions


~Constructor~

This constructor constructs an empty inner node.

*/

  Node(int min1, int max1): 
     min(min1), max(max1), count(0), box(false) {
     sons = new Node<dim>*[max+1]; 
  }


/*
~append~

Appends a new entry to this node. If the node overflows,
the result will be false.

*/
  bool append(Node* entry){
   assert(count <= max);

   assert(entry->box.IsDefined());
   sons[count] = entry;
   if(count == 0){
      this->box = entry->box;
   } else {
      this->box = this->box.Union(entry->box);
   }
   count++;
   return  count <= max;
}

/*
~checkBox~

This function is for debugging purposes. It checks whether the
box stored within this node corresponds to the union of  all
boxes of all sons.

*/
bool checkBox(bool print = true) const{
  if(max<0){ // an object node
     bool res = box.IsDefined();
     if(print && !res){
        cout << "undefined object node" << endl;
     }
     return res;
  }
  if(count == 0){
     bool res = !box.IsDefined();
     if(print && !res){
        cout << "box defined but count == 0" << endl; 
     }
     return  res;
  }
  if(count < 0){
     if(print){
        cout << "count < 0 " << endl;
     }
     return false;
  }
  if(!box.IsDefined()){
     if(print){
       cout << "count >0 but box is undefined" << endl;
     }
     return false;
  }
  Rectangle<dim> abox = sons[0]->box;
  for(int i=0;i<count;i++){
     abox = abox.Union(sons[i]->box);
  }
  bool res =  abox == box; 
  if(print && !res){
     cout << "boxes differ";
     cout << "computed box = "; abox.Print(cout); cout << endl;
     cout << "stored box = "; box.Print(cout) ; cout  << endl;
  }
  return res;
}


void recomputeBox() {
  if(max<0){ // an object node
     return;
  }
  if(count == 0){
    box.SetDefined(false);
  } else {
    box = sons[0]->box;
    for(int i=0;i<count;i++){
      box =  box.Union(sons[i]->box);
    }
  }
}

/*
~selectFittestSon~


Returns the index of the son which is the best one for
searching further the leaf for including ~box~.

*/
unsigned int selectFittestSon(const Rectangle<dim> box)const{
  assert(max>0 && count >0);
  // initialize best fit index to be 0
  double area = sons[0]->box.Area();
  Rectangle<dim> b = sons[0]->box;
  double extend = b.Union(box).Area() - area;
  int index = 0;
  for(int i=1;i<count;i++){
      double area2 = sons[i]->box.Area();
      Rectangle<dim> b2 = sons[i]->box;
      double extend2 = b2.Union(box).Area() - area2;
      if(extend2 < extend){
        extend = extend2;
        area = area2;
        index = i;
      } else if((extend2 == extend) && (area2 < area)){
        extend = extend2;
        area = area2;
        index = i;
      } else if((extend2 == extend) && (area2 == area) &&
                (sons[i]->count < sons[index]->count)){
        extend = extend2;
        area = area2;
        index = i;
      }
  }
  return index;
}


/*

~pickSeeds~

Returns the indexes for the seeds using quadratic split.

*/
pair<unsigned int,unsigned  int> pickSeeds() const{
  pair<int, int> res;
  double d = 0;
  for(int i=0;i<count;i++){
    for(int j=i+1;j<count;j++){
        double d2 = (sons[i]->box.Union(sons[j]->box)).Area() -
                    (sons[i]->box.Area() + sons[j]->box.Area());
        if(d2>d){
         res.first = i;
         res.second = j;
         d = d2;
        }
    }
  }
  if(d==0){ // all boxes are the same
     res.first = 0;
     res.second = count-1;
  }
  return res;
}


/*
~pickNext~

returns the next index for the quadratic split algorithm.

*/
pair<unsigned int,unsigned int>
 pickNext(const Node* grp1,const Node* grp2) const{
    assert(count>0);
    double d1 = sons[0]->box.Union(grp1->box).Area();
    double d2 = sons[0]->box.Union(grp2->box).Area();
    unsigned int index = 0;
    unsigned int bestgrp = -1;
    double d = abs(d1-d2);
    for(int i=1;i<count;i++){
        d1 = sons[i]->box.Union(grp1->box).Area();
        d2 = sons[i]->box.Union(grp2->box).Area();
        double d3 = abs(d1-d2);
        if(d3>d){
           d = d3;
           index = i;
           double a1 = grp1->box.Area();
           double a2 = grp2->box.Area();
           d1 = d1 - a1;
           d2 = d2 - a2;
           if(d1!=d2){
             bestgrp = d1<d2?1:2;
           } else if(a1!=a2){
             bestgrp = a1<a2?1:2;
           } else if(grp1->count!=grp2->count){
             bestgrp = grp1->count<grp2->count? 1:2;
           } else { // all criterions failed
             bestgrp = 1;
           }
        }
    }
    pair<unsigned int , unsigned int> res;
    res.first = index;
    res.second = bestgrp;
    return res;
}

/*
~Remove~

Removes the entry at position entry from this node.
If there is an underflow, the result will be false.

*/
bool remove(int index, bool updateBox = false){
   for(int i=index;i<count-1;i++){
     sons[i] = sons[i+1];
   }
   count--;
   if(updateBox){
      if(count==0){
         box.SetDefined(false);
      } else {
         box = sons[0]->box;
         for(int i=1;i < count; i++){
            box = box.Union(sons[i]->box);
         }
      }
   }
   return count >= min;
}

/*
~Split~

Splits this node into two ones. After calling this function, this node will
be empty and all contained elements was distributed to the new nodes. 

*/
pair<Node*, Node*> split(){
  pair<Node<dim>*, Node<dim>* > res =  quadraticSplit();
  return res;
}


/*
~isLeaf~

This function checks whether this node is a leaf, i.e. whether the sons
are objects nodes.

*/
bool isLeaf() const{
  assert(max>0);
  if(count==0){
    return true;
  }
  return sons[0]->max < 0; 
}


/*
~quadraticSplit~

Implementation of the quadratic split algorithm. 

*/
pair<Node*, Node*> quadraticSplit(){
  pair<int, int> seeds = pickSeeds();
  int index1 = seeds.first;
  int index2 = seeds.second;
  Node* node1 = new Node(min,max);
  Node* node2 = new Node(min,max);
  node1->append(sons[index1]);
  node2->append(sons[index2]);
  this->remove(std::max(index1,index2));
  this->remove(std::min(index1,index2));

  while(count > 0){
    if(count + node1->count == min){ // all entries to node1
      for(int i=0;i<count;i++){
        node1->append(sons[i]);
      }
      count = 0;
    }else if(count + node2->count == min){ // all entries to node2
      for(int i=0;i<count;i++){
        node2->append(sons[i]);
      }
      count = 0;
    } else {
      pair<unsigned int, unsigned int> next = pickNext(node1,node2);
      if(next.second == 1){
         node1->append(sons[next.first]);
      } else {
         node2->append(sons[next.first]);
      }
      remove(next.first);
    }
  }
  return make_pair(node1,node2);
} // end of quadraticsSplit


/*
~Print~

Prints information about this node to ~o~.

*/
void Print( ostream& o){
   o << "[min = " << min  
     << ", max = " << max
     << ", count = " << count
     << ", box = "   << box
     << ", sons = " << sons;
}


}; // end of class Node


/*
2 The class RTree

*/

template<unsigned dim>
class Rtree{

public:
/*
2.1 Constructor

This coinstructor creates a new empty RTRee with ~min~ and ~max~ as
parameters for the minimum/ maximum number of entries within a node.

*/
   Rtree(int min, int max){
      assert(max>=2);
      assert(min>0);
      assert(min<=max/2);
      this->min=min;
      this->max = max;
      root = 0;
   }

/*
2.2 ~insert~

Inserts a box together with an id into the R-Tree.

*/
   void insert(Rectangle<dim> box, long id){
      if(!root){
        root = new Node<dim>(min,max);
      } 
      Node<dim>* obj = new Node<dim>(box,id);
      pair<Node<dim>*, Node<dim>* >* res = insertRec(root,obj);
      if(res){
         root = new Node<dim>(min,max);
         root->append(res->first);
         root->append(res->second);
         delete res;
      }    
   }

/*
2.3 ~findAll~

Returns all object's ids stored in the tree where the box 
intersects ~box~. 

*/
  void findAll(const Rectangle<dim> box, set<long>& res)const{
     res.clear();
     findAllRec(root,box,res);
  } 


/*
2.4 ~erase~

Erases the entries of ~id~ found at positions intersecting by box.

*/
  void erase(const Rectangle<dim> box, long id){

     set<pair < int , Node<dim>*> > Q;
     Q.clear();
     if(eraseRec(root,box,id, Q,0)){
        if(root->count==0  && root->isLeaf()){ // last entry removed
           root = 0;
        }


        if(Q.size() > 0){
           insert(Q);
        }
        if((root!=0) && (root->count == 1) &&(!root->isLeaf())){
           Node<dim>* victim = root;
           root = root->sons[0];
           delete victim;
           
        } 
     }
  }




/*
2.4 ~printStats~

Prints some statistical information about this tree to ~o~.

*/
  ostream& printStats(ostream& o){
     o << "Tree[" << endl
       << "  min = " << min << endl
       << "  max = " << max << endl
       << "  nodes = " << noNodes(root) << endl
       << "  leafs = " << noLeafs(root) << endl
       << " height = " << height(root) << endl
       << " objects = " << noObjects(root) << endl
       << " ] ";
       return o;
  }

/*
2.5 ~printAsRel~

This function writes the nested List representation of this tree to ~o~.

*/
  void printAsRel(ostream& o){
     o << "  ( (rel (tuple ("
       << "      ( level int )"
       << "      ( box rectangle) "               
       << "      ( id  int )"
       << "      ( father int) "
       << "      )))"
       << " ( ";
       int level = 0;
       int id = 0;
       int father = -1;
       printAsRelRec(root,o,level, id, father);
       o << " ))";
  }

/*
2.6 private part

*/

private:
/*
~Data Members~

*/

   int min; // minimum number of entries within a node
   int max; // maximum number of entries within a node
   Node<dim>* root; // root node

/*
~getListString~

This function produces a string according to the nested list
representation of a rectangle.

*/
   string getListString(const Rectangle<dim>& rect){
      stringstream res;
      res <<   "(" ;
      for(unsigned int i=0;i<dim; i++){
         res << " " << rect.MinD(i) << " " << rect.MaxD(i);
      }
      res << ")";
      return res.str();
   }

/*
~printAsRelRec~

Prints the content of the subtree given by root as relation to o.

*/

   void printAsRelRec(Node<dim>* root, ostream& o,
                      int level, int& id, int father){
       if(!root){
          return;
       } else if (root->max<0){
         o << "("
           << level << " "
           << getListString(root->box) << " "
           << id++ << " " 
           << father 
           << " )" << endl;
       } else {
          int myId = id;
          id++;
          o << "("
            << level << " "
            << getListString(root->box) << " "
            << myId  << " "
            << father
            << " )" << endl;
          for(int i=0;i<root->count; i++){
             printAsRelRec( root->sons[i],o, level+1, id, myId);
          }
       }

   } 

/*
~printAsTree~



*/

void printAsTree(ostream& o) const {
   o << "( tree " << endl;
   printAsTreeRec(root,o);
   o << ")"; 
}


void printAsTreeRec(const Node<dim>* root, ostream& o)const {
   if(!root){
      o << "()" << endl;
   } else {
      o << "(" 
        << root->getLabel() << "  " ;
     if(root->max >0){ // not an object node
        o << "(";
        for(int i=0;i<root->count ; i++){
          printAsTreeRec(root->sons[i],o);
        }
        o << ")"; 
     } else {
       o << "()" ; 
     }
     o << ")" << endl;
   }
}



/*
~noNodes~

Computes the number of non-object-nodes within the tree.

*/
   int noNodes(Node<dim>* root){
       if(!root){
         return 0;
       }
       if(root->isLeaf()){
         return 1;
       }else {
          int sum = 1;
          for(int i=0;i<root->count;i++){
             sum += noNodes(root->sons[i]);
          }
         return sum;
       }
   }
  
/*
~noLeaves~

Returns the number of leaves within the tree.

*/
   int noLeaves(Node<dim>* root){
       if(!root){
          return 0;
       }
       if(root->isLeaf()){
         return 1;
       }else {
          int sum = 0;
          for(int i=0;i<root->count;i++){
             sum += noLeafs(root->sons[i]);
          }
         return sum;
       }
   }

/*
~noObjects~

Computes the number of stored objects. If an objects is
multiple stored, each instance is count.

*/
   int noObjects(Node<dim>* root){
       if(!root){
          return 0; 
       }
       if(root->isLeaf()){
         return root->count;
       }else {
          int sum = 0;
          for(int i=0;i<root->count;i++){
             sum += noObjects(root->sons[i]);
          }
         return sum;
       }
   }

/*
~height~

Computes the height of the tree.

*/
   int height(Node<dim>* root){
       if(!root){
          return -1;
       }
       if(root->isLeaf()){
         return 0;
       } else {
         return 1 + height(root->sons[0]);
       }
   }

/*
~insertRec~

Inserts the objects node ~obj~ into the subtree given by ~root~.
If ~root~ must be split, it is destroyed and the split parts are
returned as result. In the other case, the result will be 0.

*/

   pair<Node<dim>*, Node<dim>* >* 
     insertRec(Node<dim>*& root, Node<dim>* obj){

      if(root->isLeaf()){
         if(root->append(obj)){ // no overflow
            return 0;
         } else { // overflow
            pair<Node<dim>*, Node<dim>*> res = root->split();
            delete root;
            root = 0;
            return new pair<Node<dim>*, Node<dim>*>(res);
         }
      } else { // not a leaf node
         int index = root->selectFittestSon(obj->box);
         pair<Node<dim>*, Node<dim>*>* res = insertRec(root->sons[index],obj);
         if(!res){ // son was not split
           root->box = root->box.Union(obj->box);
           return 0;   
         }else {
           root->sons[index] = res->first; // replace old son by a split node
           root->box = root->box.Union(res->first->box); 
           if(root->append(res->second)){ // no overflow
              delete res;
              return 0;
           } else { 
              delete res;
              res = new pair<Node<dim>*, Node<dim>*>(root->split());
              delete root;
              root = 0;
              return res;
           }
         }
      }
   }

/*
~insert~

Inserts a  set of subtrees at the specified levels. This function supports the
~erase~ function. 

*/
void insert(const set<pair < int , Node<dim>* > >&  Q){
   int levelDiff = 0; // store the grow of the tree
   typename set<pair < int , Node<dim>*> >::iterator it;
   for(it = Q.begin(); it!=Q.end(); it++){
      pair<int, Node<dim>*> node = *it;     
      int level = node.first<0?node.first:node.first+levelDiff;
      if(insertNodeAtLevel(level, node.second)){
        levelDiff++;
      }
   }
}

/*
Inserts a node at the specified level. If the tree grows, true is 
returned.

*/
bool insertNodeAtLevel(int level, Node<dim>* node){
  if(!root){
     root = new Node<dim>(min,max);
  }
  pair<Node<dim>*, Node<dim>* >* res = insertRecAtLevel(root,node, level,0); 
  if(!res){ // tree does not grow
     return false;
  } else {
    delete root;
    root = new Node<dim>(min,max);
    root->append(res->first);
    root->append(res->second);
    delete res;
    return true;
  }
}


pair<Node<dim>*, Node<dim>* >* 
  insertRecAtLevel(Node<dim>*& root,  Node<dim>* node, 
                   const int targetLevel, const int currentLevel){
    if(root->isLeaf() || (targetLevel == currentLevel) ){
      if(root->append(node)){ // no overflow
        return 0;
      } else { // overflow
        pair<Node<dim>*, Node<dim>*> res = root->split();
        delete root;
        root = 0;
        return new pair<Node<dim>*, Node<dim>*>(res);
      }
    } else { // not the target node
      int index = root->selectFittestSon(node->box);
      pair<Node<dim>*, Node<dim>*>* res;
      res  = insertRecAtLevel(root->sons[index], node,
                              targetLevel, currentLevel+1);
      if(!res){ // son was not split
        root->box = root->box.Union(node->box);
        return 0;   
      }else {
        root->sons[index] = res->first; // replace old son by a split node
        root->box = root->box.Union(res->first->box); 
        if(root->append(res->second)){ // no overflow
           delete res;
           return 0;
        } else { 
           delete res;
           res = new pair<Node<dim>*, Node<dim>*>(root->split());
           delete root;
           root = 0;
           return res;
        }
      }
    }
 }


/*
~findAllRec~

Searches in the subtree givben by root for objects whose
bounding box intersects ~box~ and collect them in ~res~.

*/   
   void findAllRec(const Node<dim>* root,
                   const Rectangle<dim>& box,
                   set<long>& res)const{
     if(!root){
        return;
     } else if(root->isLeaf()){
       for(int i = 0; i<root->count; i++){
          if(root->sons[i]->box.Intersects(box)){
            res.insert(root->sons[i]->count);
          }
       }
     } else {
       for(int i =0; i < root->count; i++){
          if(root->sons[i]->box.Intersects(box)){
            findAllRec(root->sons[i],box,res);
          }
       }
     }
   }


/* 
Erases one occurence of id.

If ~root~ is 0, nothing is doed. Otherwise, if root is underflowed by
erasing the entry, root is deleted and the remaining subtrees are
inserted into Q. An exception is the root of the tree which is not
removed even if  0 entries  left in the root.

*/
bool eraseRec(Node<dim>*& root,const Rectangle<dim>& box, const long id,
              set<pair < int, Node<dim>*> >& Q, int level){

    if(!root){ // tree is empty, doe nothing
       return false;
    } else if(root->isLeaf()){
       // try find the object node having the corresponding id
       int index = -1;
       for(int i=0;i<root->count && index < 0;i++){
          if(root->sons[i]->count == id){
              index = i;
          }
       }
       if(index < 0){  // id not found within this node
          return false;
       }
       bool under = !root->remove(index,true);
       if( under && (root != this->root)){ // an underflow
          // insert all remaining leaves into Q
          for(int i=0;i<root->count; i++){
             Q.insert(make_pair(-1, root->sons[i]));
          }
          delete root; // delete the underflowed node
          root = 0;
       }
       return true;  // deletion successful
    } else { // root is an inner node
      int index = -1;
      for(int i=0;i<root->count && index <0 ; i++){
         if(root->sons[i]->box.Intersects(box)){
            if(eraseRec(root->sons[i], box, id, Q, level + 1)){
               index = i;
            }
         }
      } 
      if(index < 0){ // id not found in this subtree
        return false;  
      }
      if(root->sons[index]){ // no underflow
         root->recomputeBox();
         return true;
      }
      bool under = !root->remove(index,true);
      if( !under || (root == this->root)){ // no underflow 
         return true;
      }
      // an underflow in root because deletion of son[index]
      // insert remaining subtrees into Q
      for(int i=0;i<root->count;i++){
         Q.insert(make_pair(level, root->sons[i]));
      }
      delete root;
      root = 0;
      return true;
    }
}


/*
~checkTree~

This function checks the complete tree for RTree properties. 
 - all leaves are on the same level
 - all nodes have at most max sons
 - all nodes without the root have at least min nodes 
 - the bounding box a each node is the union of the boxes of its sons


*/
bool checkTree(const bool print = true)const{
  return checkLeafLevel(root, print) &&
         checkSonNumber (root,print,0 ) &&
         checkBox(root,print,0);
}

bool checkBox(const Node<dim>* root, const bool print,const int level)const{
   if(!root) {
     return true;
   } else {
     if(!root->checkBox(print)){
        if(print){
           cout << "Wrong boxes at level " << level << endl;
           cout << "There are " << root->count << "entries" << endl;
        }
        return false;
     } 
     if(!root->isLeaf()){
        bool wrong = true;
        for(int i=0;i<root->count && wrong;i++){
           wrong = checkBox(root->sons[i],print,level+1);
        } 
        return wrong;
     }
     return true;
   }
}

bool checkSonNumber(const Node<dim>* root,  
                    const bool print,
                    const int level) const{
  if(!root){ // empty tree all ok
    return true;
  } else {

    if(root->count > max){ // too much entries
      if(print){
          cout << "Node with more than " << max << " entries found " << endl;
          cout << " problem at level " << level << endl;
      }
      return false;
    }
    if((level>0) ){ // not the root node
      if((root->count) < (this->min)  ){ // too less entries
        if(print){
          cout << "Node with less than " << min << " entries found " << endl;
          cout << "Problem at Level " << level << endl;
        }
        return false;
      }
    } 
    if(!root->isLeaf()){
       bool ok = true;
       for(int i=0;i<root->count && ok; i++){
          ok = checkSonNumber(root->sons[i],print,level + 1);
       }
       return ok;
    }
    return true;
  }
}




bool checkLeafLevel(const Node<dim>* root, const bool print) const{
  if(!root) { // empty tree
    return true;
  } else if(root->isLeaf()){ // a leave has correct height
    return true;
  } else { // not a leaf

    int h = getHeight(root->sons[0]);
    bool ok = true;
    for(int i=1; i<root->count && ok ; i++){
       ok = h == getHeight(root->sons[i]);
    }
    for(int i=0;i<root->count && ok; i++){
       ok = checkLeafLevel(root->sons[i],print);
    }
    if(!ok && print){
       cout << "Leaves on different levels found " << endl;
    }
    return ok;
  }
}

int getHeight(const Node<dim>* root)const{
   if(!root){
     return -1;
   }
   int h = 0;
   const Node<dim>* node =  root;
   if(node->isLeaf()){
      h++;
      node = node->sons[0];
   }
   return h;
}




}; 






} // end of namespace

#endif
