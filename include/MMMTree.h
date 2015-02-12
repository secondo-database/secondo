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

#ifndef MMMTREE_H
#define MMMTREE_H

#include <assert.h>
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
template<class T, class DistComp>
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
~incraseRadius~

Increases the radius of this node in that way to include the 
argument objects.

*/
     inline void increaseRadius(const T& o, DistComp& dc){
         double d = dc(routingObject,o);
         if(d>radius){
           radius = d;
         }
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
      inline double centerDist(const T& o, DistComp& di) const{
        return di(routingObject,o);
      }
      

/*
~minDist~

Returns the distance between the area repreented by this node and __o__.
If __o__ is inside the circle, 0 is returned.

*/
      inline double minDist(const T& o, DistComp& di){
        double dist = di(routingObject,o) - radius;
        return dist>=0?dist:0;
      }

/*
~getParent~

Returns the parent of this node or 0 if this node is the root of the tree.

*/
      inline MTreeNode<T,DistComp>* getParent(){
        return parent;
      }

/*
~setParent~

Sets the father node.

*/
      inline void setParent(MTreeNode<T,DistComp>* p, DistComp& di){
         parent = p;
         distanceToParent = distance(p, di);
      }

/*
~distance~

Returns the distance between the routing objects of this node and
the routing objectt of the other one.

*/
      inline double distance(MTreeNode<T,DistComp>* node, DistComp& di) const{
        return di(routingObject,node->routingObject);
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

      virtual std::ostream& print(std::ostream& out, DistComp& di) const = 0; 
      
      virtual std::ostream& print(std::ostream& out, 
                                  const bool printSubTrees,
                                  DistComp& di) const =0;


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
      MTreeNode<T,DistComp>* parent;
      int count;
};



/*
2 Class ~MTreeInnerNode~

This class represents an inner node of an m-tree.

*/

template<class T,class DistComp>
class MTreeInnerNode: public MTreeNode<T,DistComp>{
   public:

/*
2.1 Constructor

This constructor creates a new inner node of an m-tree able to to __maxEntries__
sons. The routing object is given by ~Or~.

*/
      MTreeInnerNode(const int _minEntries, const int _maxEntries, const T Or):
        MTreeNode<T,DistComp>(Or,0,0,_minEntries,_maxEntries)
      {
         sons = new MTreeNode<T,DistComp>*[_maxEntries+1];
         for(int i=0;i<_maxEntries+1;i++){
            sons[i]=0;
         }
      }

/*
2.2 Destructor

Destroys the tree rooted by this node.

*/
      virtual ~MTreeInnerNode(){
        for(int i=0;i<MTreeNode<T,DistComp>::maxEntries+1;i++){
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
         for(int i=0;i<MTreeNode<T,DistComp>::count;i++){
            if(deleteContent){
               delete sons[i];
            }
            sons[i] = 0;
         }
         MTreeNode<T,DistComp>::count = 0;
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
   MTreeNode<T,DistComp>* getBestSon(const T& o, DistComp& di) const{
     MTreeNode<T,DistComp>* res = 0;
     double currentDist=0; 

     MTreeNode<T,DistComp>* secondRes = 0;
     double secondDist=0;

     for(int i=0;i<MTreeNode<T,DistComp>::count;i++){
        const T* ro = (sons[i])->getRoutingObject();
        double dist = di(o,*ro);
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

   MTreeNode<T,DistComp>* getSon(const int i){
       return sons[i];
   }


/*
~store~

Adds a new subtree to this node.

*/
   void store(MTreeNode<T,DistComp>* node, DistComp& di){

       //cout << "store called" << endl;
       //cout << "Routing Object is "; 
       //di.print(MTreeNode<T,DistComp>::routingObject,cout) << endl;
       //cout << "radius is " << MTreeNode<T,DistComp>::radius << endl;

       //cout << "noderoutingobject is "; 
       // di.print(*node->getRoutingObject(),cout) << endl;
       //cout << "node radius is " << node->getRadius() << endl;


       // compute new radius from this and the new node
       double dist = di(MTreeNode<T,DistComp>::routingObject, 
                        *node->getRoutingObject());;

       //cout << " dist = " << dist << endl;

       double rad = max(this->getRadius() , dist + node->getRadius());


       //cout << "new radius is " << rad << endl;

       MTreeNode<T,DistComp>::radius = rad;
       sons[MTreeNode<T,DistComp>::count] = node;
       MTreeNode<T,DistComp>::count++; 
       node->setParent(this, di);
   }


/*
~search~

Searches a son by scanning all entries.

*/
   int search(const MTreeNode<T,DistComp>* son) const{
     for(int i=0;i<MTreeNode<T,DistComp>::count;i++){
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
   void replace(MTreeNode<T,DistComp>* replacement, int index, DistComp& di){
      sons[index] = replacement;
      double dist = this->distance(replacement, di);
      double rad = max(this->getRadius(), dist + replacement->getRadius());
      MTreeNode<T,DistComp>::radius =  rad;
      replacement->setParent(this, di); 
   } 


/*
~print~

Writes a textual representation of this node to ~out~.

*/
  
   virtual std::ostream& print(std::ostream& out, DistComp& di) const {
     return print(out,true, di);
   }
   
   virtual std::ostream& print(std::ostream& out,
                               const bool printSubtrees,
                               DistComp& di ) const {
       out << "( \"o = " ;
       di.print( MTreeNode<T,DistComp>::routingObject,out) << ", rad = " 
           << MTreeNode<T,DistComp>::radius << "\"";
       if(printSubtrees){
         out <<  " (";
         for(int i=0;i<MTreeNode<T,DistComp>::count;i++){
            sons[i]->print(out,di);
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
     for(int i=0; i<MTreeNode<T,DistComp>::count;i++){
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
     for(int i=0; i<MTreeNode<T,DistComp>::count;i++){
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
     for(int i=0; i< MTreeNode<T,DistComp>::count;i++){
       sum += sons[i]->getNoNodes();
     }
     return sum + 1;
   }

   private:
/*
2.4 Member variables

*/
     MTreeNode<T,DistComp>** sons;
};



/*
3 Class MTreeLeafNode

This class represents a leaf node of an m-tree.

*/

template<class T,class DistComp>
class MTreeLeafNode: public MTreeNode<T,DistComp>{
   public:

/*
3.1 Constructor

*/
     MTreeLeafNode(const int _minEntries, const int _maxEntries, const T Or):
        MTreeNode<T,DistComp>(Or,0,0,_minEntries,_maxEntries){
         Objects = new T*[MTreeNode<T,DistComp>::maxEntries+1];
         for(int i=0;i<MTreeNode<T,DistComp>::maxEntries+1;i++){
            Objects[i] = 0;
         }
     }

/*
3.2 Destructor

*/
     virtual ~MTreeLeafNode(){
         for(int i=0;i<MTreeNode<T,DistComp>::maxEntries+1;i++){
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
        return MTreeNode<T,DistComp>::count;
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
     void store(const T& o, DistComp& di){
        Objects[MTreeNode<T,DistComp>::count] = new T(o);
        double dist = di(MTreeNode<T,DistComp>::routingObject,o);
        if(MTreeNode<T,DistComp>::radius<dist){
          MTreeNode<T,DistComp>::radius=dist;
        }
        MTreeNode<T,DistComp>::count++;
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
         for(int i=0;i<MTreeNode<T,DistComp>::count;i++){
            if(deleteContent){
               delete Objects[i];
            }
            Objects[i] = 0;
         }
         MTreeNode<T,DistComp>::count = 0;
      }

/*
~print~

write a textual representation fo this leaf to ~out~.

*/
  
      std::ostream& print(std::ostream& out, DistComp& di) const{
         out << "[ o = ";
         di.print(MTreeNode<T,DistComp>::routingObject,out) << ", rad = " 
             << MTreeNode<T,DistComp>::radius << " , content = \"";
         for(int i=0;i<MTreeNode<T,DistComp>::count;i++){
            if(i>0) out << ", ";
            di.print(*Objects[i],out) ;
         }
         out << "\"]";
         return out;
      }

      std::ostream& print(std::ostream& out, 
                         const bool printSubtrees,
                         DistComp &di) const{
         return print(out, di);
      }

  
   private:

/*
3.4 Member variables.

*/
     T** Objects;
};




template <class T, class DistComp>
class RangeIterator{

   public:
      RangeIterator(const MTreeNode<T,DistComp>* root, const T& _q, 
                    const double _range, const DistComp& _di):
            s(),q(_q), range(_range), di(_di) {
         if(!root){
           return;
         }
         di.reset();
         if((root->centerDist(q, di) - range <= root->getRadius() )){
             s.push(std::pair<const MTreeNode<T,DistComp>*,int>(root,-1));
             findNext();
         } 
      }

      bool hasNext(){
          return !s.empty();
      }
  
      const T* next(){
       std::pair<const MTreeNode<T,DistComp>*,int> top = s.top();
       findNext();
       return ((MTreeLeafNode<T,DistComp>*)top.first)->getObject(top.second);
      }

      size_t noComparisons(){
          return di.getCount();
      }


   private:
      std::stack<std::pair<const MTreeNode<T,DistComp>*, int> > s;
      T q;
      double range;
      DistComp di;

      void findNext(){
         while(!s.empty()){
           std::pair<const MTreeNode<T,DistComp>*, int> top = s.top();
           s.pop();
           top.second++; // ignore current result
           if(top.second < top.first->getCount() ){
             if(top.first->isLeaf()){
                MTreeLeafNode<T,DistComp>* leaf = 
                                 (MTreeLeafNode<T,DistComp>*) top.first;
                while(top.second < leaf->getCount()){
                    double dist = di(*(leaf->getObject(top.second)),q);
                    if(dist<=range){
                        s.push(top);
                        return;
                    } else {
                       top.second++;
                    }
                }
             }   else { // an inner node
                MTreeInnerNode<T,DistComp>* inner = 
                                (MTreeInnerNode<T,DistComp>*) top.first;
                s.push(top);
                std::pair<MTreeNode<T,DistComp>*, int> 
                            cand(inner->getSon(top.second),-1);
                if(cand.first->minDist(q,di) <= range){
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
template<class T,class DistComp>
class NNContent{

   public:
      NNContent(const double _dist, MTreeNode<T,DistComp>* _node):
         dist(_dist), node(_node), obj(0){ }
      
      NNContent(const double _dist, T* _obj):
         dist(_dist), node(0), obj(_obj) {}

      T* getObject() {
         return obj;
      }


      MTreeNode<T,DistComp>* getNode() {
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
      MTreeNode<T,DistComp>* node;
      T* obj;
};

/*
4.2 NNContentComparator

This auxiliary class implements the less operator for two 
NNContent objects.

*/
template<class T, class DistComp>
class NNContentComparator{
  public:
      bool operator()(const NNContent<T,DistComp>& a, 
                      const NNContent<T,DistComp>& b){
         return a < b;
      }
};


template<class T, class DistComp>
class NNIterator{
   public:
/*
4.1 Constructor

*/ 
   NNIterator(MTreeNode<T,DistComp>* root, T& _ref, DistComp _di ):
      ref(_ref), q(), di(_di){
      if(root!=0){
        double dist = root->minDist(ref, di);
        q.push(NNContent<T,DistComp>(dist,root));
      }
   }

   bool hasNext() const{
      return !q.empty();
   }

   T* next(){
      if(q.empty()){
         return 0;
      }
      NNContent<T,DistComp> top = q.top();
      q.pop();
      // for nodes, push the sons/objects to q
      while(top.getObject()==0){
          MTreeNode<T,DistComp>* node = top.getNode();
          if(node->isLeaf()){
            MTreeLeafNode<T,DistComp>* leaf = (MTreeLeafNode<T,DistComp>*) node;
            for(int i=0;i<leaf->getCount();i++){
               T* o = leaf->getObject(i);
               double dist = di( *o,ref);
               q.push(NNContent<T,DistComp>(dist,o));
            }
          } else {
             MTreeInnerNode<T,DistComp>* inner = 
                                            (MTreeInnerNode<T,DistComp>*) node;
             for(int i=0;i<inner->getCount(); i++){
                MTreeNode<T,DistComp>* son = inner->getSon(i);
                double dist = son->minDist(ref, di);
                q.push(NNContent<T,DistComp>(dist, son));
             }
          }
          top = q.top();
          q.pop();
      }
      return top.getObject(); 
      
   }
  

   private:
      T ref;
      std::priority_queue<NNContent<T,DistComp>,
                          std::vector<NNContent<T,DistComp> >, 
                          std::greater<NNContent<T,DistComp> > > q;
      DistComp di;

};




/*
4 Class MMTree

This is the main class of this file. It implements a main memory based m-tree.

*/
template<class T, class DistComp>
class MMMTree{
  public:

/*
4.1 Constructor

Creates an empty tree.

*/
    MMMTree(int _minEntries, int _maxEntries, DistComp& _di): 
         minEntries(_minEntries), maxEntries(_maxEntries), root(0), di(_di){}

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


    size_t noComparisons(){
       return di.getCount(); 
    }


/*
~insert~

Adds ~o~ to this tree.

*/
    void insert(T & o){
       if(!root){
         root=new MTreeLeafNode<T,DistComp>(minEntries,maxEntries,o);
       }
       root = insert(root,o,di);
    }


/*
~print~

Writes a textual representation of this tree to ~out~.

*/
    std::ostream& print(std::ostream& out){
       if(root==0){
           out << "empty";
       } else {
           root->print(out,di);
       }
       return out;
    }

/*
~rangeSearch~

Returns a range iterator iterating over all Elements with distance 
to q smaller or equals to range.

*/
   RangeIterator<T,DistComp>* rangeSearch(const T& q, double range) const {
      return new RangeIterator<T,DistComp>(root,q,range, di);
   }


/*
~nnSearch~

Returns an iterator returning the elements of this tree in
increasing order to the reference object.

*/
   NNIterator<T,DistComp>* nnSearch(T& ref){
     return new NNIterator<T,DistComp>(root,ref,di);
   }

   const DistComp& getDistComp(){
      return di;
   }


   private:
     int minEntries;
     int maxEntries;
     MTreeNode<T,DistComp>* root;  
     DistComp di;
/*
~insert~

Inserts a new entry to the tree rooted by root, returns the root of the
tree that contains all old entries and o (may be the same root as the 
parameter.

*/
   static MTreeNode<T,DistComp>* insert(MTreeNode<T,DistComp>* root, const T& o,
                                       DistComp& di){
     MTreeNode<T,DistComp>* son = root;
      while(!son->isLeaf()){
         son->increaseRadius(o,di);
         son = ((MTreeInnerNode<T,DistComp>*)son)->getBestSon(o, di);
      }
      ((MTreeLeafNode<T,DistComp>*)son)->store(o,di);
      return split(root,son, di);
   }

/*
~selectSeeds~

 selects two nodes from the given node ahich are the seeds for 
 splitting.

*/

   static std::pair<int,int> selectSeeds(MTreeNode<T,DistComp>* node,
                                         DistComp& dc){
     //variants
     // 1: use two random numbers
     // 2: use one random number and the entry with maximum distance to it
     // 3: build a sample from this node, take the two nodes form the sample 
           // with minium overlapping area

     // temporarly use the first variant
     //int seed1 = std::rand()%node->getCount();
     //int seed2 = std::rand()%(node->getCount()-1);
     //if(seed2>=seed1){
     //  seed2++;
     //}     
     //return std::pair<int,int>(seed1,seed2);

     // second variant
     int seed1 = std::rand()%node->getCount();
     int seed2 = seed1;   
     double dist = 0;

     if(node->isLeaf()){
        MTreeLeafNode<T,DistComp>* node1 = (MTreeLeafNode<T,DistComp>*) node;
        const T* ro = node1->getObject(seed1);
        for(int i=0;i<node1->getCount();i++){
           if(i!=seed1){
              const T* ro2 = node1->getObject(i); 
              double d2 = dc(*ro,*ro2);
              if((seed1==seed2) || d2>dist){
                  dist = d2;
                  seed2 = i;
              }
           }
        }
     } else {
        MTreeInnerNode<T,DistComp>* node1 = (MTreeInnerNode<T,DistComp>*) node;
        const T* ro = node1->getSon(seed1)->getRoutingObject();
        for(int i=0;i<node1->getCount();i++){
           if(i!=seed1){
              const T* ro2 = node1->getSon(i)->getRoutingObject(); 
              double d2 = dc(*ro,*ro2);
              if((seed1==seed2) || d2>dist){
                  dist = d2;
                  seed2 = i;
              }
           }
        }
     }
     pair<int,int> res(seed1,seed2);
     return res;
     


   }

/*
~Create partition~

Distributes the content of a node two two new nodes using ~firstSeed~ and
~secondSeed~ for the seeds.

*/
   static std::pair<MTreeNode<T,DistComp>*,MTreeNode<T,DistComp>*> 
   createPartition(
         const MTreeNode<T,DistComp>* node, 
         const int firstSeed, 
         const int secondSeed,
         DistComp& di) {

       if(node->isLeaf()){
          MTreeLeafNode<T,DistComp>* nodeL = (MTreeLeafNode<T,DistComp>*) node;
          T* obj1 = nodeL->getObject(firstSeed);
          T* obj2 = nodeL->getObject(secondSeed); 
          MTreeLeafNode<T,DistComp>* first = 
                           new MTreeLeafNode<T,DistComp>(node->getMinEntries(), 
                                                         node->getMaxEntries(), 
                                                         *obj1);
          MTreeLeafNode<T,DistComp>* second = 
                       new MTreeLeafNode<T,DistComp>(node->getMinEntries(), 
                                            node->getMaxEntries(),
                                            *obj2);
          MTreeLeafNode<T,DistComp>* in;
          for(int i=0;i<node->getCount();i++){
             T* obj = nodeL->getObject(i);
             double dist1 = first->centerDist(*obj,di);
             double dist2 = second->centerDist(*obj,di);
             if(dist1==dist2){
                in = first->getCount()<second->getCount()?first:second;
             } else {
                in = dist1<dist2?first:second;
             }
             in->store(*obj,di);
          }
          return std::pair<MTreeNode<T,DistComp>*,MTreeNode<T,DistComp>*>
                     (first,second);
       } else { // process inner node
          MTreeInnerNode<T,DistComp>* nodeI= (MTreeInnerNode<T,DistComp>*) node;
          const T* obj1 = nodeI->getSon(firstSeed)->getRoutingObject();  
          const T* obj2 = nodeI->getSon(secondSeed)->getRoutingObject();  
          MTreeInnerNode<T,DistComp>* first = 
                      new MTreeInnerNode<T,DistComp>(node->getMinEntries(),
                                            node->getMaxEntries(), 
                                            *obj1);
          MTreeInnerNode<T,DistComp>* second = 
                      new MTreeInnerNode<T,DistComp>(node->getMinEntries(), 
                                            node->getMaxEntries(),
                                            *obj2);
          MTreeInnerNode<T,DistComp>* in;
          for(int i=0;i<node->getCount();i++){
              MTreeNode<T,DistComp>* son = nodeI->getSon(i);
              double dist1 = first->distance(son,di);
              double dist2 = second->distance(son,di);
              if(dist1==dist2){
                in = first->getCount()<second->getCount()?first:second;
              } else {
                in = dist1<dist2?first:second;
              }
              in->store(son,di);
          }
          return std::pair<MTreeNode<T,DistComp>*,MTreeNode<T,DistComp>*>
                   (first,second);
       }
   }

/*
~split~

Performs a split up to the root of this tree.

*/
   static MTreeNode<T,DistComp>* split(MTreeNode<T,DistComp>* root,
                                       MTreeNode<T,DistComp>* currentNode, 
                                       DistComp& di){

       while((currentNode!=0) && currentNode->isOverflow()){
          std::pair<int,int> seeds = selectSeeds(currentNode,di);
          std::pair<MTreeNode<T,DistComp>*,MTreeNode<T,DistComp>*> part = 
                         createPartition(currentNode,seeds.first, 
                                         seeds.second, di);
          
          if(currentNode->getParent()==0){ // splitted root
              const T* ro = rand()%1>0?part.first->getRoutingObject()
                                :part.second->getRoutingObject();
              MTreeInnerNode<T,DistComp>* root2 = 
                          new MTreeInnerNode<T,DistComp>(
                                   currentNode->getMinEntries(), 
                                   currentNode->getMaxEntries(),
                                     *ro);
              currentNode->clear(currentNode->isLeaf());
              delete currentNode;
              currentNode = 0;
              root2->store(part.first,di);
              root2->store(part.second, di); 
              root = root2;
          } else {
              
              MTreeInnerNode<T,DistComp>* parent =
                      (MTreeInnerNode<T,DistComp>*) currentNode->getParent();
              int index = parent->search(currentNode);
              assert(index >=0);
              parent->replace(part.first,index,di);
              parent->store(part.second,di);
              currentNode->clear(currentNode->isLeaf()); 
              delete currentNode;
              currentNode = parent;
          }
       }
       return root;
   } 
   
};



#endif


