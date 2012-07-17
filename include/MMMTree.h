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

#include <assert.h>
#include "distances.h"
#include <cstdlib> 
#include <utility>
#include <iostream>
#include <stack>
#include <queue>


// implementation of a main memory based M-tree

/*
1 class MTreeNode

The class MTreeNode is an abstract super class for the nodes 
of an m-tree.

*/
template<class T>
class MTreeNode{
    public:
  

/*
1.1 Destructor

Destroy the subtree rooted by this node

*/
      virtual ~MTreeNode(){}    

/*
1.2 Methods

~isLeaf~

This function returns true if this node represents a leaf.

*/
  
      virtual bool isLeaf() const = 0;

/*
1.3 ~getRoutingObject~

This funtion returns the routingObject (center) of a node.

*/
      const T* getRoutingObject() const {
         return &routingObject;
      }

/*
~getRadius~

Returns the radius of this node.

*/
      inline double getRadius() const{
         return radius;
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
~isOverflow~

Returns true if the node has more entries than allowed.
Note that a node temporarly can hold one node more than
the limit. In this case, the node must be splitted.

*/
      inline bool isOverflow() const{
        return count == maxEntries;
      }


/*
~getCount~

Returns the number of entries (sons or objects) of this node.

*/
      inline int getCount() const{
         return count;
      }

/*
~centerDist~

Returns the distance between the routing object of this node and the
argument.

*/
      inline double centerDist(const T& o) const{
        return ::distance(routingObject,o);
      }
      

/*
~minDist~

Returns the distance between the area repreented by this node and __o__.
If __o__ is inside the circle, 0 is returned.

*/
      inline double minDist(const T& o){
        double dist = ::distance(routingObject,o) - radius;
        return dist>=0?dist:0;
      }

/*
~getParent~

Returns the parent of this node or 0 if this node is the root of the tree.

*/
      inline MTreeNode<T>* getParent(){
        return parent;
      }

/*
~setParent~

Sets the father node.

*/
      inline void setParent(MTreeNode<T>* p){
         parent = p;
         distanceToParent = distance(p);
      }

/*
~distance~

Returns the distance between the routing objects of this node and
the routing objectt of the other one.

*/
      inline double distance(MTreeNode<T>* node) const{
        return ::distance(routingObject,node->routingObject);
      }

/*
~clear~

Removes the entries from this node. If deleteContent is set to be
true, the sons of this node are destroyed.

*/
      virtual void clear(const bool deleteContent) =0;

/*
~print~

Writes a textual representation of this node to ~out~.

*/

      virtual std::ostream& print(std::ostream& out) const = 0;
      
      virtual std::ostream& print(std::ostream& out, 
                                  const bool printSubTrees) const = 0;


/*
~getNoEntries~

Returns the number of entries stored in the tree rooted by this node.

*/

      virtual int getNoEntries() const = 0;
      
/*
~getNoLeafs~

Returns the number of leafs within this subtree.

*/
      virtual int getNoLeafs() const = 0;
      
/*
~getNoNodes~

Returns, how many nodes are within the subtree rooted by this.

*/
      virtual int getNoNodes() const = 0;


    protected:
/*
1.3 Constructor

This constructor initializes all members

*/
      MTreeNode(const T& Or, double rad, double dist, 
                const int _minEntries, const int _maxEntries): 
        routingObject(Or), radius(rad), distanceToParent(dist),
        minEntries(_minEntries), maxEntries(_maxEntries), parent(0),count(0){}


/*
1.4 Member variables

*/
      T routingObject;
      double radius;
      double distanceToParent;
      int minEntries;
      int maxEntries;
      MTreeNode<T>* parent;
      int count;
};



/*
2 Class ~MTreeInnerNode~

This class represents an inner node of an m-tree.

*/

template<class T>
class MTreeInnerNode: public MTreeNode<T>{
   public:

/*
2.1 Constructor

This constructor creates a new inner node of an m-tree able to to __maxEntries__
sons. The routing object is given by ~Or~.

*/
      MTreeInnerNode(const int _minEntries, const int _maxEntries, const T Or):
        MTreeNode<T>(Or,0,0,_minEntries,_maxEntries)
      {
         sons = new MTreeNode<T>*[_maxEntries+1];
         for(int i=0;i<_maxEntries+1;i++){
            sons[i]=0;
         }
      }

/*
2.2 Destructor

Destroys the tree rooted by this node.

*/
      virtual ~MTreeInnerNode(){
        for(int i=0;i<MTreeNode<T>::maxEntries+1;i++){
          if(sons[i]){
            delete sons[i];
          }
        }
        delete[] sons;
      }

/*
2.3 Methods

~clear~

Removes all subtress from this node. If ~deleteContent~ is true,
all subtrees are destroyed.

*/      
      virtual void clear(const bool deleteContent){
         for(int i=0;i<MTreeNode<T>::count;i++){
            if(deleteContent){
               delete sons[i];
            }
            sons[i] = 0;
         }
         MTreeNode<T>::count = 0;
      }

/*
~isLeaf~

Implements the pure virtual function of the super class by 
returning false.

*/
   virtual bool isLeaf() const{
     return false;
   }


/*
~getBestSon~

 returns the son of this node which is the best for
 inserting o

*/
   MTreeNode<T>* getBestSon(const T& o) const{
     MTreeNode<T>* res = 0;
     double currentDist=0; 

     MTreeNode<T>* secondRes = 0;
     double secondDist=0;

     for(int i=0;i<MTreeNode<T>::count;i++){
        const T* ro = (sons[i])->getRoutingObject();
        double dist = distance(o,*ro);
        if(dist < sons[i]->getRadius()){ // o fits into the circle of son
            if(!res || dist < currentDist){  // first fitting son
               res = sons[i];
               currentDist = dist;
            }  
        } 
        if(!res){
           if(!secondRes || dist - sons[i]->getRadius() < secondDist){
              secondRes = sons[i];
              secondDist = dist - sons[i]->getRadius();
           }
        }
     }
     return res?res:secondRes;
   }

   MTreeNode<T>* getSon(const int i){
       return sons[i];
   }


/*
~store~

Adds a new subtree to this node.

*/
   void store(MTreeNode<T>* node){
        sons[MTreeNode<T>::count] = node;
        MTreeNode<T>::count++; 
        double dist = this->distance(node);
        if(dist + max(this->getRadius(),node->getRadius()) > 
           MTreeNode<T>::radius){
           MTreeNode<T>::radius = dist + 
                                 max(node->getRadius(),this->getRadius());
        }  
        node->setParent(this);
   }


/*
~search~

Searches a son by scanning all entries.

*/
   int search(const MTreeNode<T>* son) const{
     for(int i=0;i<MTreeNode<T>::count;i++){
       if(sons[i]==son){
         return i;
       }
     } 
     return -1;
   }


/*
~replace~

Replaces the entry at position ~index~ by ~replacement~.

*/
   void replace(MTreeNode<T>* replacement, int index){
      sons[index] = replacement;
      double dist = this->distance(replacement);
      if(dist + max(replacement->getRadius(),this->getRadius()) > 
         MTreeNode<T>::radius){
         MTreeNode<T>::radius = dist + 
                       max(this->getRadius(),replacement->getRadius()); 
      } 
      replacement->setParent(this); 
   } 


/*
~print~

Writes a textual representation of this node to ~out~.

*/
   virtual std::ostream& print(std::ostream& out) const {
     return print(out,true);
   }
   
   virtual std::ostream& print(std::ostream& out,
                               const bool printSubtrees ) const {
       out << "( \"o = " << MTreeNode<T>::routingObject << ", rad = " 
           << MTreeNode<T>::radius << "\"";
       if(printSubtrees){
         out <<  " (";
         for(int i=0;i<MTreeNode<T>::count;i++){
            sons[i]->print(out);
         }
         out << " )";
       }
       out << ")";
       return out;
   }

/*
~getNoLeafs~

Returns the number of leafs for this subtree.

*/
   virtual int getNoLeafs() const{
     int sum = 0;
     for(int i=0; i<MTreeNode<T>::count;i++){
       sum += sons[i]->getNoLeafs();
     }
     return sum;
   }
  
/*
~getNoEntries~

Returns the number of entries in this subtree.

*/ 
   virtual int getNoEntries() const{
     int sum = 0;
     for(int i=0; i<MTreeNode<T>::count;i++){
       sum += sons[i]->getNoEntries();
     }
     return sum;
   }


/*
~getNoNodes~

Returns the number of nodes of  this subtree.

*/
   virtual int getNoNodes() const{
     int sum = 0;
     for(int i=0; i< MTreeNode<T>::count;i++){
       sum += sons[i]->getNoNodes();
     }
     return sum + 1;
   }

   private:
/*
2.4 Member variables

*/
     MTreeNode<T>** sons;
};



/*
3 Class MTreeLeafNode

This class represents a leaf node of an m-tree.

*/

template<class T>
class MTreeLeafNode: public MTreeNode<T>{
   public:

/*
3.1 Constructor

*/
     MTreeLeafNode(const int _minEntries, const int _maxEntries, const T Or):
        MTreeNode<T>(Or,0,0,_minEntries,_maxEntries){
         Objects = new T*[MTreeNode<T>::maxEntries+1];
         for(int i=0;i<MTreeNode<T>::maxEntries+1;i++){
            Objects[i] = 0;
         }
     }

/*
3.2 Destructor

*/
     virtual ~MTreeLeafNode(){
         for(int i=0;i<MTreeNode<T>::maxEntries+1;i++){
           if(Objects[i]){
              delete Objects[i];
           }
         }
        delete[] Objects;
     }
         
/*
3.3 Methods

~isLeaf~

Of course, this function returns true.

*/
     virtual bool isLeaf() const { return true; }
   
/*
~getNoLeafs~

Returns 1.

*/
     virtual int getNoLeafs() const{
        return 1;
     }

/*
~getNoEntries~

Returns the number of entries stored within this leaf.

*/
     virtual int getNoEntries() const{
        return MTreeNode<T>::count;
     }


/*
~getNoNodes~

Returns 1.

*/
     virtual int getNoNodes() const {
       return 1;
     }

/*

~store~

Adds an entry to this leaf.


*/
     void store(const T& o){
        Objects[MTreeNode<T>::count] = new T(o);
        double dist = distance(MTreeNode<T>::routingObject,o);
        if(MTreeNode<T>::radius<dist){
          MTreeNode<T>::radius=dist;
        }
        MTreeNode<T>::count++;
     }

/*
~getObject~

Returns the object stored at index ~i~.

*/
     T* getObject(const int i){
        return Objects[i];
     }


/*
~clear~

Removes all entries of this node. If ~deleteContent~ is true,
all contained objects are destroyed.

*/
     virtual void clear(const bool deleteContent){
         for(int i=0;i<MTreeNode<T>::count;i++){
            if(deleteContent){
               delete Objects[i];
            }
            Objects[i] = 0;
         }
         MTreeNode<T>::count = 0;
      }

/*
~print~

write a textual representation fo this leaf to ~out~.

*/
      std::ostream& print(std::ostream& out) const{
         out << "[ o = " << MTreeNode<T>::routingObject << ", rad = " 
             << MTreeNode<T>::radius << " , content = \"";
         for(int i=0;i<MTreeNode<T>::count;i++){
            if(i>0) out << ", ";
            out << *Objects[i] ;
         }
         out << "\"]";
         return out;
      }

      std::ostream& print(std::ostream& out, 
                         const bool printSubtrees) const{
         return print(out);
      }


   private:

/*
3.4 Member variables.

*/
     T** Objects;
};




template <class T>
class RangeIterator{

   public:
      RangeIterator(const MTreeNode<T>* root, const T& _q, 
                    const double _range):s(),q(_q), range(_range) {
         if(!root){
           return;
         }
         if((root->centerDist(q) + range <= root->getRadius() )){
             s.push(std::pair<const MTreeNode<T>*,int>(root,-1));
             findNext();
         } 
      }

      bool hasNext(){
          return !s.empty();
      }
  
      const T* next(){
       std::pair<const MTreeNode<T>*,int> top = s.top();
       findNext();
       return ((MTreeLeafNode<T>*)top.first)->getObject(top.second);
      }



   private:
      std::stack<std::pair<const MTreeNode<T>*, int> > s;
      T q;
      double range;

      void findNext(){
         while(!s.empty()){
           std::pair<const MTreeNode<T>*, int> top = s.top();
           s.pop();
           top.second++; // ignore current result
           if(top.second < top.first->getCount() ){
             if(top.first->isLeaf()){
                MTreeLeafNode<T>* leaf = (MTreeLeafNode<T>*) top.first;
                while(top.second < leaf->getCount()){
                    double dist = distance(*(leaf->getObject(top.second)),q);
                    if(dist<=range){
                        s.push(top);
                        return;
                    } else {
                       top.second++;
                    }
                }
             }   else { // an inner node
                MTreeInnerNode<T>* inner = (MTreeInnerNode<T>*) top.first;
                s.push(top);
                std::pair<MTreeNode<T>*, int> 
                            cand(inner->getSon(top.second),-1);
                if(cand.first->minDist(q) <= range){
                   s.push(cand);
                } 
              }
           }
         }

      }
};

/*
4 Class NNIterator

This iterator returns the content of an m-tree with increasing distance.

4.1 Auxiliary Class 

The class NNContent encapsulates the entries of a priority queue.

*/
template<class T>
class NNContent{

   public:
      NNContent(const double _dist, MTreeNode<T>* _node):
         dist(_dist), node(_node), obj(0){ }
      
      NNContent(const double _dist, T* _obj):
         dist(_dist), node(0), obj(_obj) {}

      T* getObject() {
         return obj;
      }


      MTreeNode<T>* getNode() {
        return node;
      }

      bool operator<(const NNContent& b) const{
          if(dist != b.dist){
             return dist < b.dist;
          }  
          if(node!=b.node) {
             return node < b.node;
          }
          return obj < b.obj;
      }
      
      bool operator>(const NNContent& b) const{
          if(dist != b.dist){
             return dist > b.dist;
          }  
          if(node!=b.node) {
             return node > b.node;
          }
          return obj > b.obj;
      }


   private:  
      double dist;
      MTreeNode<T>* node;
      T* obj;
};

/*
4.2 NNContentComparator

This auxiliary class implements the less operator for two 
NNContent objects.

*/
template<class T>
class NNContentComparator{
  public:
      bool operator()(const NNContent<T>& a, const NNContent<T>& b){
         return a < b;
      }
};


template<class T>
class NNIterator{
   public:
/*
4.1 Constructor

*/ 
   NNIterator(MTreeNode<T>* root, T& _ref ):ref(_ref), q(){
      if(root!=0){
        double dist = root->minDist(ref);
        q.push(NNContent<T>(dist,root));
      }
   }

   bool hasNext() const{
      return !q.empty();
   }

   T* next(){
      if(q.empty()){
         return 0;
      }
      NNContent<T> top = q.top();
      q.pop();
      // for nodes, push the sons/objects to q
      while(top.getObject()==0){
          MTreeNode<T>* node = top.getNode();
          if(node->isLeaf()){
            MTreeLeafNode<T>* leaf = (MTreeLeafNode<T>*) node;
            for(int i=0;i<leaf->getCount();i++){
               T* o = leaf->getObject(i);
               double dist = ::distance( *o,ref);
               q.push(NNContent<T>(dist,o));
            }
          } else {
             MTreeInnerNode<T>* inner = (MTreeInnerNode<T>*) node;
             for(int i=0;i<inner->getCount(); i++){
                MTreeNode<T>* son = inner->getSon(i);
                double dist = son->minDist(ref);
                q.push(NNContent<T>(dist, son));
             }
          }
          top = q.top();
          q.pop();
      }
      return top.getObject(); 
      
   }
  

   private:
      T ref;
      std::priority_queue<NNContent<T>,
                          std::vector<NNContent<T> >, 
                          std::greater<NNContent<T> > > q;

};




/*
4 Class MMTree

This is the main class of this file. It implements a main memory based m-tree.

*/
template<class T>
class MMMTree{
  public:

/*
4.1 Constructor

Creates an empty tree.

*/
    MMMTree(int _minEntries, int _maxEntries): 
         minEntries(_minEntries), maxEntries(_maxEntries), root(0){}

/*
4.2 Destructor

Destroyes this tree.

*/
    ~MMMTree(){
       if(root){
          delete root;
       }
     }


/*
4.3 Methods

~getNoLeafs~


Returns the number of leafs of this tree.

*/
    int getNoLeafs() const{
       if(!root){
          return 0;
       } else {
          return root->getNoLeafs();
       }
    }

/*
~getNoEntries~

Returns the number of stored objects.

*/
    int getNoEntries() const{
       if(!root){
          return 0;
       } else {
          return root->getNoEntries();
       }

    }

/*
~getNoNodes~

Returns the number of nodes of this tree.

*/
    int getNoNodes() const{
       if(!root){
          return 0;
       } else {
          return root->getNoNodes();
       }

    }


/*
~insert~

Adds ~o~ to this tree.

*/
    void insert(T & o){
       if(!root){
         root=new MTreeLeafNode<T>(minEntries,maxEntries,o);
       }
       root = insert(root,o);
    }


/*
~print~

Writes a textual representation of this tree to ~out~.

*/
    std::ostream& print(std::ostream& out){
       if(root==0){
           out << "empty";
       } else {
           root->print(out);
       }
       return out;
    }

/*
~rangeSearch~

Returns a range iterator iterating over all Elements with distance 
to q smaller or equals to range.

*/
   RangeIterator<T>* rangeSearch(const T& q, double range) const {
      return new RangeIterator<T>(root,q,range);
   }


/*
~nnSearch~

Returns an iterator returning the elements of this tree in
increasing order to the reference object.

*/
   NNIterator<T>* nnSearch(T& ref){
     return new NNIterator<T>(root,ref);
   }



   private:
     int minEntries;
     int maxEntries;
     MTreeNode<T>* root;  

/*
~insert~

Inserts a new entry to the tree rooted by root, returns the root of the
tree that contains all old entries and o (may be the same root as the 
parameter.

*/
   static MTreeNode<T>* insert(MTreeNode<T>* root, const T& o){
     MTreeNode<T>* son = root;
      while(!son->isLeaf()){
         son = ((MTreeInnerNode<T>*)son)->getBestSon(o);
      }
      ((MTreeLeafNode<T>*)son)->store(o);
      return split(root,son);
   }

/*
~selectSeeds~

 selects two nodes from the given node ahich are the seeds for 
 splitting.

*/

   static std::pair<int,int> selectSeeds(MTreeNode<T>* node){
     //variants
     // 1: use two random numbers
     // 2: use one random number and the entry with maximum distance to it
     // 3: build a sample from this node, take the two nodes form the sample 
           // with minium overlapping area

     // temporarly use the first variant
     int seed1 = std::rand()%node->getCount();
     int seed2 = std::rand()%(node->getCount()-1);
     if(seed2>=seed1){
       seed2++;
     }     
     return std::pair<int,int>(seed1,seed2);
   }

/*
~Create partition~

Distributes the content of a node two two new nodes using ~firstSeed~ and
~secondSeed~ for the seeds.

*/
   static std::pair<MTreeNode<T>*,MTreeNode<T>*> createPartition(
         const MTreeNode<T>* node, 
         const int firstSeed, 
         const int secondSeed) {

       if(node->isLeaf()){
          MTreeLeafNode<T>* nodeL = (MTreeLeafNode<T>*) node;
          T* obj1 = nodeL->getObject(firstSeed);
          T* obj2 = nodeL->getObject(secondSeed); 
          MTreeLeafNode<T>* first = new MTreeLeafNode<T>(node->getMinEntries(), 
                                                         node->getMaxEntries(), 
                                                         *obj1);
          MTreeLeafNode<T>* second = 
                       new MTreeLeafNode<T>(node->getMinEntries(), 
                                            node->getMaxEntries(),
                                            *obj2);
          MTreeLeafNode<T>* in;
          for(int i=0;i<node->getCount();i++){
             T* obj = nodeL->getObject(i);
             double dist1 = first->centerDist(*obj);
             double dist2 = second->centerDist(*obj);
             if(dist1==dist2){
                in = first->getCount()<second->getCount()?first:second;
             } else {
                in = dist1<dist2?first:second;
             }
             in->store(*obj);
          }
          return std::pair<MTreeNode<T>*,MTreeNode<T>*>(first,second);
       } else { // process inner node
          MTreeInnerNode<T>* nodeI = (MTreeInnerNode<T>*) node;
          const T* obj1 = nodeI->getSon(firstSeed)->getRoutingObject();  
          const T* obj2 = nodeI->getSon(secondSeed)->getRoutingObject();  
          MTreeInnerNode<T>* first = 
                      new MTreeInnerNode<T>(node->getMinEntries(),
                                            node->getMaxEntries(), 
                                            *obj1);
          MTreeInnerNode<T>* second = 
                      new MTreeInnerNode<T>(node->getMinEntries(), 
                                            node->getMaxEntries(),
                                            *obj2);
          MTreeInnerNode<T>* in;
          for(int i=0;i<node->getCount();i++){
              MTreeNode<T>* son = nodeI->getSon(i);
              double dist1 = first->distance(son);
              double dist2 = second->distance(son);
              if(dist1==dist2){
                in = first->getCount()<second->getCount()?first:second;
              } else {
                in = dist1<dist2?first:second;
              }
              in->store(son);
          }
          return std::pair<MTreeNode<T>*,MTreeNode<T>*>(first,second);
       }
   }

/*
~split~

Performs a split up to the root of this tree.

*/
   static MTreeNode<T>* split(MTreeNode<T>* root,MTreeNode<T>* currentNode){

       while((currentNode!=0) && currentNode->isOverflow()){
          std::pair<int,int> seeds = selectSeeds(currentNode);
          std::pair<MTreeNode<T>*,MTreeNode<T>*> part = 
                         createPartition(currentNode,seeds.first, 
                                         seeds.second);

          if(currentNode->getParent()==0){ // splitted root

              MTreeInnerNode<T>* root2 = 
                          new MTreeInnerNode<T>(currentNode->getMinEntries(), 
                                     currentNode->getMaxEntries(),
                                     *(currentNode->getRoutingObject()));
              currentNode->clear(currentNode->isLeaf());
              delete currentNode;
              currentNode = 0;
              root2->store(part.first);
              root2->store(part.second); 
              root = root2;
          } else {
              
              MTreeInnerNode<T>* parent = (MTreeInnerNode<T>*) 
                                           currentNode->getParent();
              int index = parent->search(currentNode);
              assert(index >=0);
              parent->replace(part.first,index);
              parent->store(part.second);
              currentNode->clear(currentNode->isLeaf()); 
              delete currentNode;
              currentNode = parent;
          }
       }
       return root;
   } 
   
};






