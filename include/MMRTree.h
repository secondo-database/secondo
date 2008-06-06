
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

/*
1 Preparations

This section contains  __includes__, __namespaces__, and __forward declarations__.


*/


#include <string.h>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>

#include "RectangleAlgebra.h"


namespace mmrtree {



template<unsigned dim> class Node;


/*
2 Declaration of class RTree

*/

template<unsigned dim> class Rtree{

public:
/*
2.1 Constructor

This constructor creates a new empty RTRee with ~min~ and ~max~ as
parameters for the minimum/ maximum number of entries within a node.

*/
   Rtree(int min, int max);

/*
2.2 Destructor 

*/
  ~Rtree(){
      if(root){
         root->destroy();
         delete root;
         root = 0;
      }
   }


/*
2.2 ~insert~

Inserts a box together with an id into the R-Tree.

*/
   void insert(const Rectangle<dim>& box, long id);

/*
2.3 ~findAll~

Returns all object's ids stored in the tree where the box 
intersects ~box~. 

*/
  void findAll(const Rectangle<dim>& box, set<long>& res)const;

/*
2.4 ~findAllExact~

Returns all object ids stored within the tree whose corresponding 
rectangle is AlmostEqual to ~box~

*/
  void findAllExact(const Rectangle<dim>& box, set<long>& res)const;



/*
2.4 ~erase~

Erases an  entry of ~id~ found at positions intersecting by box.
If the object id is multiple stored, only the first instance will
be removed from the tree. If an object was deleted, the result 
will be __true__; otherwise (i.e.
if no object with given id was present at the specified position),
the result will be __false__.


*/
  bool erase(const Rectangle<dim>& box, const long id);

/*
2.4 ~printStats~

Prints some statistical information about this tree to ~o~.

*/
  ostream& printStats(ostream& o)const;

/*
2.5 ~printAsRel~

This function writes the nodes of the tree into ~o~
as a relation in nested list format. 

*/
  void printAsRel(ostream& o)const;

/*
2.6 ~printAsTree~

This function writes the content of the tree into ~o~.
The result can be read from Secondo's Javagui.

*/
void printAsTree(ostream& o) const;

/*
~noNodes~

Computes the number of non-object-nodes within the tree.

*/
   int noNodes() const;

  
/*
~noLeaves~

Returns the number of leaves within the tree.

*/
   int noLeaves() const;

/*
~noObjects~

Computes the number of stored objects. If an objects is
multiple stored, each instance is count.

*/
   int noObjects() const;

/*
~height~

Computes the height of the tree rooted by ~root~.

*/
   int height() const;

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
   string getListString(const Rectangle<dim>& rect) const ;

/*
~printAsRelRec~

Prints the content of the subtree given by root as relation to 
~o~.

*/

   void printAsRelRec(Node<dim>* root, ostream& o,
                      const int level, int& id, const int father) const;


/*
~printAsTreeRec~

Function supporting the public function ~printAsTree~.


*/

void printAsTreeRec(const Node<dim>* root, ostream& o)const;

/*
~noNodes~

Computes the number of non-object-nodes within the tree.

*/
   int noNodes(const Node<dim>* root) const;

  
/*
~noLeaves~

Returns the number of leaves within the tree rooted by ~root~.

*/
   int noLeaves(const Node<dim>* root) const;

/*
~noObjects~

Computes the number of stored objects. If an object is
multiple stored, each instance is count.

*/
   int noObjects(const Node<dim>* root) const;

/*
~height~

Computes the height of the tree rooted by ~root~.

*/
   int height(const Node<dim>* root) const;

/*
~insert~

Inserts a  set of subtrees at the specified levels.
This function supports the ~erase~ function. 

*/
void insert(const set<pair < int , Node<dim>* > >&  Q);

/*
Inserts a node at the specified level. If the tree grows, true is 
returned. If a leaf should be inserted, just set level to -1.

*/
bool insertNodeAtLevel(int level, Node<dim>* node);

/*
~insertRecAtLevel~

Function supporting the ~insertNodeAtLevel~ function.

*/
pair<Node<dim>*, Node<dim>* >* 
  insertRecAtLevel(Node<dim>*& root,  Node<dim>* node, 
                   const int targetLevel, const int currentLevel);


/*
~findAllRec~

Searches in the subtree given by root for objects whose
bounding box intersects ~box~ and collect them in ~res~.

*/   
   void findAllRec(const Node<dim>* root,
                   const Rectangle<dim>& box,
                   set<long>& res)const;

/*
~findAllRecExact~

Searches in the subtree given by root for objects whose
bounding box contains ~box~ and collect the object ids
whose corresponding rectangle is equals to ~box~.

*/   
   void findAllRecExact(const Node<dim>* root,
                   const Rectangle<dim>& box,
                   set<long>& res)const;

/* 
Erases one occurence of id.

If ~root~ is 0, nothing is do. Otherwise, if root is underflowed by
erasing the entry, root is deleted and the remaining subtrees are
inserted into ~Q~. An exception is the root of the tree which is not
removed even if  0 entries  left in the root.

*/
bool eraseRec(Node<dim>*& root,const Rectangle<dim>& box, const long id,
              set<pair < int, Node<dim>*> >& Q, int level);


/*
~checkTree~

This function checks the complete tree for RTree properties. 
 - all leaves are on the same level
 - all nodes have at most max sons
 - all nodes without the root have at least min nodes 
 - the bounding box a each node is the union of the boxes of its sons


*/
bool checkTree(const bool print = true)const;

bool checkBox(const Node<dim>* root, const bool print,const int level)const;

bool checkSonNumber(const Node<dim>* root,  
                    const bool print,
                    const int level) const;

bool checkLeafLevel(const Node<dim>* root, const bool print) const;

int getHeight(const Node<dim>* root)const;

}; 

/*

1 Definition of the Class ~Node~


This class represents a single node within an R-tree.
There is no distinction between nodes holding the objects ids,
leafs nodes and inner nodes. All nodes are represented by this 
class. This class should only be used within the R-tree class.
For this reason, all members are private and the R-Tree class
is declared as a friend of this class.


*/

template<unsigned dim> class Node{

friend class Rtree<dim>;
private:
/*
1.1 Constructors


This constructor creates a leaf node.

*/
  Node(const Rectangle<dim>& abox, long id):
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
array managing the sons is removed. If the complete subtree should
be removed, use a combination of destroy and delete.

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

/*
1.4 ~destroy~

Deletes the subtrees rooted by this node.

*/

  void destroy(){
     if(sons){
        for(int i=0;i<count; i++){
           sons[i]->destroy();
           delete sons[i];
        }
        delete[] sons;
        count = 0;
        sons = 0;
     }
  }


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

/*
~recomputeBox~

Sets the box to the union of all boxes of the sons.

*/
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
unsigned int selectFittestSon(const Rectangle<dim>& box)const{
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
   sons[count-1] = 0;
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
2 Implementation of the class RTree

2.1 Constructor

This constructor creates a new empty RTRee with ~min~ and ~max~ as
parameters for the minimum/ maximum number of entries within a node.

*/
template<unsigned dim>
Rtree<dim>::Rtree(const int min, const int max){
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
template<unsigned dim>
void Rtree<dim>::insert(const Rectangle<dim>& box, long id){
   if(!root){
     root = new Node<dim>(min,max);
   } 
   Node<dim>* obj = new Node<dim>(box,id);
   insertNodeAtLevel(-1,obj);
}

/*
2.3 ~findAll~

Returns all object's ids stored in the tree where the box 
intersects ~box~. 

*/
template<unsigned dim>
void Rtree<dim>::findAll(const Rectangle<dim>& box, set<long>& res)const{
  res.clear();
  findAllRec(root,box,res);
} 

/*
2.3 ~findAllExact~

Returns all object's ids stored in the tree where the box 
is equals to ~box~. 

*/
template<unsigned dim>
void Rtree<dim>::findAllExact(const Rectangle<dim>& box, set<long>& res)const{
  res.clear();
  findAllRecExact(root,box,res);
} 

/*
2.4 ~erase~

Erases the entries of ~id~ found at positions intersecting by box.

*/
template<unsigned dim>
bool Rtree<dim>::erase(const Rectangle<dim>& box, const long id){
  set<pair < int , Node<dim>*> > Q;
  Q.clear();
  if(eraseRec(root,box,id, Q,0)){
     if(root->count==0  && root->isLeaf()){ // last entry removed
        delete root;
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
     return true;
  } else {
     return false;
  }
}

/*
2.4 ~printStats~

Prints some statistical information about this tree to ~o~.

*/
template<unsigned dim>
ostream& Rtree<dim>::printStats(ostream& o)const{
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

This function writes the nested list representation of this tree to ~o~.

*/
template<unsigned dim>
void Rtree<dim>::printAsRel(ostream& o) const{
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
~getListString~

This function produces a string according to the nested list
representation of a rectangle.

*/
template<unsigned dim>
string Rtree<dim>::getListString(const Rectangle<dim>& rect)const{
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

template<unsigned dim>
void Rtree<dim>::printAsRelRec(Node<dim>* root, ostream& o,
                      const int level, int& id, const int father) const{
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

template<unsigned dim>
void Rtree<dim>::printAsTree(ostream& o) const {
   o << "( tree " << endl;
   printAsTreeRec(root,o);
   o << ")"; 
}


template<unsigned dim>
void Rtree<dim>::printAsTreeRec(const Node<dim>* root, ostream& o)const{
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
template<unsigned dim>
int Rtree<dim>::noNodes()const{
   return noNodes(root);
}

template<unsigned dim>
int Rtree<dim>::noNodes(const Node<dim>* root) const{
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

template<unsigned dim>
int Rtree<dim>::noLeaves()const {
  return noLeaves(root);
}

template<unsigned dim>
int Rtree<dim>::noLeaves(const Node<dim>* root)const{
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

Computes the number of stored objects. If an object is
multiple stored, each instance is count.

*/

template<unsigned dim>
int Rtree<dim>::noObjects()const{
   return noObjects(root);
}

template<unsigned dim>
int Rtree<dim>::noObjects(const Node<dim>* root)const{
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
template<unsigned dim>
int Rtree<dim>::height() const{
  return height(root);
}


template<unsigned dim>
int Rtree<dim>::height(const Node<dim>* root) const{
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

/*
~insert~

Inserts a  set of subtrees at the specified levels. This function supports the
~erase~ function. 

*/
template<unsigned dim>
void Rtree<dim>::insert(const set<pair < int , Node<dim>* > >&  Q){
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
template<unsigned dim>
bool Rtree<dim>::insertNodeAtLevel(int level, Node<dim>* node){
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


template<unsigned dim>
pair<Node<dim>*, Node<dim>* >* 
  Rtree<dim>::insertRecAtLevel(Node<dim>*& root,  Node<dim>* node, 
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

Searches in the subtree given by root for objects whose
bounding box intersects ~box~ and collect them in ~res~.

*/   
template<unsigned dim>
void Rtree<dim>::findAllRec(const Node<dim>* root,
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
~findAllRecExact~

Searches in the subtree given by root for objects whose
bounding box is equals to ~box~ and collect them in ~res~.

*/   
template<unsigned dim>
void Rtree<dim>::findAllRecExact(const Node<dim>* root,
                       const Rectangle<dim>& box,
                       set<long>& res)const{
  if(!root){
     return;
  } else if(root->isLeaf()){
    for(int i = 0; i<root->count; i++){
       if(root->sons[i]->box.AlmostEqual(box)){
         res.insert(root->sons[i]->count);
       }
    }
  } else {
    for(int i =0; i < root->count; i++){
       if(root->sons[i]->box.Contains(box)){
         findAllRec(root->sons[i],box,res);
       }
    }
  }
}

/* 
Erases one occurence of ~id~.

If ~root~ is 0, nothing is do. Otherwise, if root is underflowed by
erasing the entry, root is deleted and the remaining subtrees are
inserted into Q. An exception is the root of the tree which is not
removed even if  0 entries  left in the root.

*/
template<unsigned dim>
bool Rtree<dim>::eraseRec(Node<dim>*& root,
                     const Rectangle<dim>& box, 
                     const long id,
                     set<pair < int, Node<dim>*> >& Q, 
                     int level){

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
    Node<dim>* victim = root->sons[index];
    bool under = !root->remove(index,true);
    delete victim;
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
template<unsigned dim>
bool Rtree<dim>::checkTree(const bool print/* = true*/)const{
  return checkLeafLevel(root, print) &&
         checkSonNumber (root,print,0 ) &&
         checkBox(root,print,0);
}

template<unsigned dim>
bool Rtree<dim>::checkBox(const Node<dim>* root, 
                     const bool print,
                     const int level)const{
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

template<unsigned dim>
bool Rtree<dim>::checkSonNumber(const Node<dim>* root,  
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




template<unsigned dim>
bool Rtree<dim>::checkLeafLevel(const Node<dim>* root, const bool print) const{
  if(!root) { // empty tree
    return true;
  } else if(root->isLeaf()){ // a leave has correct height
    return true;
  } else { // not a leaf

    int h = height(root->sons[0]);
    bool ok = true;
    for(int i=1; i<root->count && ok ; i++){
       ok = h == height(root->sons[i]);
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




} // end of namespace

#endif
