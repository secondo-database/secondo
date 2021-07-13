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
  

  typedef MTreeNode<T,DistComp> node_t;

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
~memSize~

Returns the amount of memory occupied by the subtree represented by this node.

*/
      virtual size_t memSize() const = 0;


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
      inline node_t* getParent(){
        return parent;
      }

/*
~setParent~

Sets the father node.

*/
      inline void setParent(node_t* p, DistComp& di){
         parent = p;
         distanceToParent = distance(p, di);
      }

/*
~distance~

Returns the distance between the routing objects of this node and
the routing objectt of the other one.

*/
      inline double distance(node_t* node, DistComp& di) const{
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


      virtual node_t* clone() = 0;


      void setParent(node_t* p){
          parent = p;
      }

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
      node_t* parent;
      int count;
};



/*
2 Class ~MTreeInnerNode~

This class represents an inner node of an m-tree.

*/

template<class T,class DistComp>
class MTreeInnerNode: public MTreeNode<T,DistComp>{
   public:
    typedef MTreeInnerNode<T,DistComp> innernode_t;
    typedef MTreeNode<T,DistComp> node_t;
/*
2.1 Constructor

This constructor creates a new inner node of an m-tree able to to __maxEntries__
sons. The routing object is given by ~Or~.

*/
      MTreeInnerNode(const int _minEntries, const int _maxEntries, const T Or):
        node_t(Or,0,0,_minEntries,_maxEntries)
      {
         sons = new node_t*[_maxEntries+1];
         for(int i=0;i<_maxEntries+1;i++){
            sons[i]=0;
         }
      }

/*
2.2 Destructor

Destroys the tree rooted by this node.

*/
      virtual ~MTreeInnerNode(){
        for(int i=0;i<node_t::maxEntries+1;i++){
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
         for(int i=0;i<node_t::count;i++){
            if(deleteContent){
               delete sons[i];
            }
            sons[i] = 0;
         }
         node_t::count = 0;
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
   node_t* getBestSon(const T& o, DistComp& di) const{
     node_t* res = 0;
     double currentDist=0; 

     node_t* secondRes = 0;
     double secondDist=0;

     for(int i=0;i<node_t::count;i++){
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

   node_t* getSon(const int i){
       return sons[i];
   }


/*
~store~

Adds a new subtree to this node.

*/
   void store(node_t* node, DistComp& di){

       //cout << "store called" << endl;
       //cout << "Routing Object is "; 
       //di.print(MTreeNode<T,DistComp>::routingObject,cout) << endl;
       //cout << "radius is " << MTreeNode<T,DistComp>::radius << endl;

       //cout << "noderoutingobject is "; 
       // di.print(*node->getRoutingObject(),cout) << endl;
       //cout << "node radius is " << node->getRadius() << endl;


       // compute new radius from this and the new node
       double dist = di(node_t::routingObject,*node->getRoutingObject());

       //cout << " dist = " << dist << endl;

       double rad = std::max(this->getRadius() , dist + node->getRadius());


       //cout << "new radius is " << rad << endl;

       node_t::radius = rad;
       sons[node_t::count] = node;
       node_t::count++; 
       node->setParent(this, di);
   }


/*
~search~

Searches a son by scanning all entries.

*/
   int search(const node_t* son) const{
     for(int i=0;i<node_t::count;i++){
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
   void replace(node_t* replacement, int index, DistComp& di){
      sons[index] = replacement;
      double dist = this->distance(replacement, di);
      double rad = std::max(this->getRadius(), dist + replacement->getRadius());
      node_t::radius =  rad;
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
       di.print( node_t::routingObject,out) << ", rad = " 
           << node_t::radius << "\"";
       if(printSubtrees){
         out <<  " (";
         for(int i=0;i<node_t::count;i++){
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
     for(int i=0; i<node_t::count;i++){
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
     for(int i=0; i<node_t::count;i++){
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
     for(int i=0; i< node_t::count;i++){
       sum += sons[i]->getNoNodes();
     }
     return sum + 1;
   }


   size_t memSize() const{
     size_t res = sizeof(*this) + 
                  sizeof(void*) * node_t::maxEntries+1;
     for(int i=0;i<node_t::count;i++){
         res += sons[i]->memSize();
     }
     return res;
   }


   innernode_t* clone(){
       innernode_t* res;
       res = new innernode_t(node_t::minEntries, 
                             node_t::maxEntries,
                             node_t::routingObject);
       res->radius = node_t::radius;
       res->distanceToParent = node_t::distanceToParent;
       res->count = node_t::count;
       for(int i=0;i<node_t::count;i++){
          res->sons[i] = sons[i]->clone();
          res->sons[i]->setParent(res);
       }  
       return res;
   }


   private:
/*
2.4 Member variables

*/
     node_t** sons;
};



/*
3 Class MTreeLeafNode

This class represents a leaf node of an m-tree.

*/

template<class T, class DistComp>
class MTreeLeafNode: public MTreeNode<T,DistComp>{
   public:

    typedef MTreeLeafNode<T,DistComp> leafnode_t;
    typedef MTreeNode<T,DistComp> node_t;

/*
3.1 Constructor

*/
     MTreeLeafNode(const int _minEntries, const int _maxEntries, const T& Or):
        node_t(Or,0,0,_minEntries,_maxEntries){
         Objects = new T*[node_t::maxEntries+1];
         for(int i=0;i<node_t::maxEntries+1;i++){
            Objects[i] = 0;
         }
     }

/*
3.2 Destructor

*/
     virtual ~MTreeLeafNode(){
         for(int i=0;i<node_t::maxEntries+1;i++){
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
        return node_t::count;
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
        Objects[node_t::count] = new T(o);
        double dist = di(node_t::routingObject,o);
        if(node_t::radius<dist){
          node_t::radius=dist;
        }
        node_t::count++;
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
         for(int i=0;i<node_t::count;i++){
            if(deleteContent){
               delete Objects[i];
            }
            Objects[i] = 0;
         }
         node_t::count = 0;
      }

/*
~print~

write a textual representation fo this leaf to ~out~.

*/
  
      std::ostream& print(std::ostream& out, DistComp& di) const{
         out << "[ o = ";
         di.print(node_t::routingObject,out) << ", rad = " 
             << node_t::radius << " , content = \"";
         for(int i=0;i<node_t::count;i++){
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

      size_t memSize() const{
        size_t res = sizeof(*this) +
                     node_t::maxEntries * sizeof(void*);
        for(int i=0;i<node_t::count;i++){
          res += sizeof(* Objects[i]);
        }
        return res;
      }

   leafnode_t* clone(){
       leafnode_t* res;
       res = new leafnode_t(node_t::minEntries, 
                            node_t::maxEntries,
                            node_t::routingObject);
       res->radius = node_t::radius;
       res->distanceToParent = node_t::distanceToParent;
       res->count = node_t::count;
       for(int i=0;i<node_t::count;i++){
          res->Objects[i] = new T(*Objects[i]);
       }  
       return res;
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

   typedef RangeIterator<T,DistComp> rangeiterator_t;
   typedef MTreeNode<T,DistComp> node_t;
   typedef MTreeLeafNode<T,DistComp> leafnode_t;
   typedef MTreeInnerNode<T,DistComp> innernode_t;

      RangeIterator(const node_t* root, const T& _q, 
                    const double _range, const DistComp& _di):
            s(),q(_q), range(_range), di(_di) {
         if(!root){
           return;
         }
         di.reset();
         if((root->centerDist(q, di) - range <= root->getRadius() )){
             s.push(std::pair<const node_t*,int>(root,-1));
             findNext();
         } 
      }

      bool hasNext(){
          return !s.empty();
      }
  
      const T* next(){
       if(s.empty()){
          return 0;
       }
       std::pair<const node_t*,int> top = s.top();
       findNext();
       return ((leafnode_t*)top.first)->getObject(top.second);
      }

      size_t noComparisons() const{
          return di.getCount();
      }
      
      int getNoDistFunCalls() const {
        return di.getNoDistFunCalls();
      }


   private:
      std::stack<std::pair<const node_t*, int> > s;
      T q;
      double range;
      DistComp di;

      void findNext(){
         while(!s.empty()){
           std::pair<const node_t*, int> top = s.top();
           s.pop();
           top.second++; // ignore current result
           if(top.second < top.first->getCount() ){
             if(top.first->isLeaf()){
                leafnode_t* leaf = 
                                 (leafnode_t*) top.first;
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
                innernode_t* inner = (innernode_t*) top.first;
                s.push(top);
                std::pair<node_t*, int> 
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
      typedef MTreeNode<T,DistComp> node_t;
      

      NNContent(const double _dist, node_t* _node):
         dist(_dist), node(_node), obj(0){ }
      
      NNContent(const double _dist, T* _obj):
         dist(_dist), node(0), obj(_obj) {}

      T* getObject() {
         return obj;
      }


      node_t* getNode() {
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
      node_t* node;
      T* obj;
};

/*
4.2 NNContentComparator

This auxiliary class implements the less operator for two 
NNContent objects.

*/
template<class T, class DistComp >
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
    typedef MTreeNode<T,DistComp> node_t;
    typedef MTreeLeafNode<T,DistComp> leafnode_t;
    typedef MTreeInnerNode<T,DistComp> innernode_t;
    typedef NNContent<T,DistComp> nncontent_t;
/*
4.1 Constructor

*/ 
   NNIterator(node_t* root, T& _ref, DistComp _di ):
      ref(_ref), q(), di(_di) {
      if(root!=0){
        double dist = root->minDist(ref, di);
        q.push(nncontent_t(dist,root));
      }
   }

   bool hasNext() const{
      return !q.empty();
   }

   T* next(){
      if(q.empty()){
         return 0;
      }
      nncontent_t top = q.top();
      q.pop();
      // for nodes, push the sons/objects to q
      while(top.getObject()==0){
          node_t* node = top.getNode();
          if(node->isLeaf()){
            leafnode_t* leaf = (leafnode_t*) node;
            for(int i=0;i<leaf->getCount();i++){
               T* o = leaf->getObject(i);
               double dist = di( *o,ref);
               q.push(nncontent_t(dist,o));
            }
          } else {
             innernode_t* inner = (innernode_t*) node;
             for(int i=0;i<inner->getCount(); i++){
                node_t* son = inner->getSon(i);
                double dist = son->minDist(ref, di);
                q.push(nncontent_t(dist, son));
             }
          }
          top = q.top();
          q.pop();
      }
      return top.getObject(); 
   }
  
   int getNoDistFunCalls() const {
     return di.getNoDistFunCalls();
   }
   
   size_t distStorageSize() const {
     return di.distStorageSize();
   }

   private:
      T ref;
      std::priority_queue<nncontent_t,
                          std::vector<nncontent_t >, 
                          std::greater<nncontent_t > > q;
      DistComp di;
};




/*
4 Class MMTree

This is the main class of this file. It implements a main memory based m-tree.

*/
template<class T, class DistComp>
class MMMTree{
  public:

    typedef MTreeLeafNode<T,DistComp> leafnode_t;
    typedef RangeIterator<T,DistComp>  rangeiterator_t;
    typedef NNIterator<T,DistComp> nniterator_t;
    typedef MTreeNode<T, DistComp> node_t;
    typedef MMMTree<T,DistComp> mmmtree_t;
    typedef MTreeInnerNode<T,DistComp> innernode_t;

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


    size_t noComparisons() const{
       return di.getCount(); 
    }

    size_t memSize() const {
       size_t res = sizeof(*this);
       if(root){
         res += root->memSize();
       }
       return res;
    }


/*
~insert~

Adds ~o~ to this tree.

*/
    void insert(T & o){
       if(!root){
         root=new leafnode_t(minEntries,maxEntries,o);
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
   rangeiterator_t* rangeSearch(const T& q, double range) const {
      return new rangeiterator_t(root,q,range, di);
   }


/*
~nnSearch~

Returns an iterator returning the elements of this tree in
increasing order to the reference object.

*/
   nniterator_t* nnSearch(T& ref){
     return new nniterator_t(root,ref,di);
   }

   const DistComp& getDistComp(){
      return di;
   }
   
   node_t* getRoot() {
     return root;
   }


   mmmtree_t* clone(){
     mmmtree_t* res = new mmmtree_t(minEntries, maxEntries, di);
     if(root){
        res->root = root->clone();
     }
     return res;
   }


   private:
     int minEntries;
     int maxEntries;
     node_t* root;  
     DistComp di;
/*
~insert~

Inserts a new entry to the tree rooted by root, returns the root of the
tree that contains all old entries and o (may be the same root as the 
parameter.

*/
   static node_t* insert(node_t* root, const T& o, DistComp& di){
      node_t* son = root;
      while(!son->isLeaf()){
         son->increaseRadius(o,di);
         son = ((innernode_t*)son)->getBestSon(o, di);
      }
      ((leafnode_t*)son)->store(o,di);
      return split(root,son, di);
   }

/*
~selectSeeds~

 selects two nodes from the given node ahich are the seeds for 
 splitting.

*/

   static std::pair<int,int> selectSeeds(node_t* node,
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
        leafnode_t* node1 = (leafnode_t*) node;
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
        innernode_t* node1 = (innernode_t*) node;
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
     std::pair<int,int> res(seed1,seed2);
     return res;
     


   }

/*
~Create partition~

Distributes the content of a node two two new nodes using ~firstSeed~ and
~secondSeed~ for the seeds.

*/
   static std::pair<node_t*,node_t*> 
   createPartition(
         const node_t* node, 
         const int firstSeed, 
         const int secondSeed,
         DistComp& di) {

       if(node->isLeaf()){
          leafnode_t* nodeL = (leafnode_t*) node;
          T* obj1 = nodeL->getObject(firstSeed);
          T* obj2 = nodeL->getObject(secondSeed); 
          leafnode_t* first = 
                           new leafnode_t(node->getMinEntries(), 
                                                         node->getMaxEntries(), 
                                                         *obj1);
          leafnode_t* second = 
                       new leafnode_t(node->getMinEntries(), 
                                            node->getMaxEntries(),
                                            *obj2);
          leafnode_t* in;
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
          return std::pair<node_t*,node_t*>
                     (first,second);
       } else { // process inner node
          innernode_t* nodeI= (innernode_t*) node;
          const T* obj1 = nodeI->getSon(firstSeed)->getRoutingObject();  
          const T* obj2 = nodeI->getSon(secondSeed)->getRoutingObject();  
          innernode_t* first = 
                      new innernode_t(node->getMinEntries(),
                                      node->getMaxEntries(), 
                                      *obj1);
          innernode_t* second = 
                      new innernode_t(node->getMinEntries(), 
                                      node->getMaxEntries(),
                                      *obj2);
          innernode_t* in;
          for(int i=0;i<node->getCount();i++){
              node_t* son = nodeI->getSon(i);
              double dist1 = first->distance(son,di);
              double dist2 = second->distance(son,di);
              if(dist1==dist2){
                in = first->getCount()<second->getCount()?first:second;
              } else {
                in = dist1<dist2?first:second;
              }
              in->store(son,di);
          }
          return std::pair<node_t*,node_t*>(first,second);
       }
   }

/*
~split~

Performs a split up to the root of this tree.

*/
   static node_t* split(node_t* root,
                        node_t* currentNode, 
                        DistComp& di){

       while((currentNode!=0) && currentNode->isOverflow()){
          std::pair<int,int> seeds = selectSeeds(currentNode,di);
          std::pair<node_t*,node_t*> part = 
                         createPartition(currentNode,seeds.first, 
                                         seeds.second, di);
          
          if(currentNode->getParent()==0){ // splitted root
              const T* ro = rand()%1>0?part.first->getRoutingObject()
                                :part.second->getRoutingObject();
              innernode_t* root2 = 
                          new innernode_t(
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
              innernode_t* parent = (innernode_t*) currentNode->getParent();
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


