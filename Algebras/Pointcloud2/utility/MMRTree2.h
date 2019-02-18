/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 Preparations

This section contains  __includes__,
                      __namespaces__, and
                      __forward declarations__.

*/

#pragma once
#include <string.h>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <utility>
#include <stack>
#include <functional>

#include "Rectangle2.h"
#include "MathUtils.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"


namespace pointcloud2 {



// max = maximum number of entries within a node >= 2
template<unsigned dim, class T, unsigned max> class Node;


/*
2 Declaration of class RTree2

This class was created from the class RtreeT and Rtree in the includes
section (MMRTree.h) and enhanced by an iterator that accepts predicates.

*/

template<unsigned dim, class T, unsigned max>
class RtreeT2 {
    using NodeT = Node<dim,T,max>;

public:

/*
2.1 Constructor

This constructor creates a new empty RTRee2.

*/
    RtreeT2();

/*
2.2 Destructor

*/
    ~RtreeT2(){
        if (root) {
            root->destroy();
            delete root;
            root = 0;
        }
    }

    Rectangle2<dim> getBBox() const;


/*
2.2 ~insert~

Inserts a box together with an id into the R-Tree.

*/
    void insert(const Rectangle2<dim>& box, const T& id);

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
~usedMem~

Returns the memory used by this structure in bytes

*/
    size_t usedMem() const;


/*
1.1 An iterator class

This class can be used to traverse the tree without collecting all
results intersecting a given rectangle.

*/
    class iterator{
        enum IterationMode {
            all,
            bbox,
            pred
        };

    public:

/*
1.1.1 Constructor

This constructor should only be used by the RTreeT class.

*/
        iterator(NodeT* root, const Rectangle2<dim>& r)
    : mode(IterationMode::bbox), path(), box(r), predicate(0) {
            if(root!=0 && accept(root)){
                init(root);
            }
        }

        iterator(NodeT* root)
        : mode(IterationMode::all), path(), box(), predicate(0) {
            if(root!=0){
                box = root->box;
                init(root);
            }
        }

        iterator(NodeT* root,
                std::function<bool(const Rectangle2<dim>&)>* predicate)
        : mode(IterationMode::pred), path(), box(), predicate(predicate) {
            if(root!=0 && accept(root)){
                box = root->box;
                this->predicate = predicate;
                init(root);
            }
        }

/*
Returns the next element from this tree. If no more elements are
available, 0 is returned. The caller must not destroy the
pointer returned.

*/
        T const * next() {
            while(!path.empty()){
                std::pair< NodeT*, int> t = path.top();
                path.pop();
                NodeT* node = t.first;
                int pos  = t.second;
                pos++;
                // try to find an intersecting entry
                while(pos < node->count){
                    if(accept(node->sons[pos])){
                        t.second = pos;
                        path.push(t);
                        return &(node->sons[pos]->id);
                    } else {
                        pos++;
                    }
                }
                computeNext();
            }
            return 0;

        }

/*
Returns an id for the node of the element last retrieved.

*/
        uintptr_t getNodeId() {
            return path.empty() ? 0 :
                    reinterpret_cast<uintptr_t>(path.top().first);
        }

    private:
        // the iteration mode
        const IterationMode mode;
        // the current stack
        std::stack<std::pair<NodeT*, int> > path;
        // the search rectangle
        Rectangle2<dim> box;
        // a predicate to determine intersection
        const std::function<bool(const Rectangle2<dim>&)>* predicate;

        bool accept(const NodeT* node) const {
            switch(mode) {
            case all:
                return true;
            case bbox:
                return node->box.Intersects(this->box);
            case pred:
                return (*(this->predicate))(node->box);
            default:
                assert (false);
                return true;
            }
        }

        // initializes the stack
        void init(NodeT* root){
            if(root->isLeaf()){
                std::pair<NodeT*, int>  p(root,-1);
                path.push(p);
            } else {
                std::pair<NodeT*, int>  p(root,-1);
                path.push(p);
                computeNext();
            }
        }



/*
~computeNext~

This function searches the next leaf having an entry with a box intersecting
the searchbox or fulfilling the predicate respectively.

*/
        void computeNext(){
            bool found = false;
            while(!path.empty() && !found){
                // get the topmost entry
                std::pair<NodeT*, int> t = path.top();
                path.pop(); // topmost element (must be updated)
                NodeT* n = t.first;
                int pos = t.second;
                // use  the next pos
                pos++;
                bool changed = false;
                while(pos < n->count && !changed){
                    NodeT* son = n->sons[pos];
                    if(accept(son)){ // try this path
                        std::pair<NodeT*,int> p(son,-1);
                        changed = true;
                        t.second = pos; // save new state
                        path.push(t);
                        path.push(p);
                        found = son->isLeaf();
                    } else {
                        pos++;
                    }
                }
            }
        }

    };


/*
~find~

This function returns an iterator which returns all entries with a
 box intersecting ~r~.  The caller is responsible to destroy the
iterator after using it. Whenever the tree is changed (inserting
or deleting objects), this iterator is invalid and will crash when
used anyway.

*/
    iterator* find(const Rectangle2<dim>& r) const{
        return new iterator(root, r);
    }


/*
~find~

This function returns an iterator which returns all entries with
bounding boxes for which the predicate returns true. The caller is
responsible to destroy the iterator after using it. Whenever the tree is
changed (inserting or deleting objects), this iterator is invalid and will
crash when used anyway.

*/
    iterator* find(std::function<bool(const Rectangle2<dim>&)>* predicate)
            const {
        return new iterator(root, predicate);
    }


/*
~entries~

Returns an iterator traversing all entries of the tree.

*/
    iterator* entries() const {
        return new iterator(root);
    }

    void printStats(size_t pointCount, size_t pointSize = 0) const;

/*
2.6 private part

*/

private:

/*
~Data Members~

*/
    NodeT* root; // root node

/*
~noNodes~

Computes the number of non-object-nodes within the tree.

*/
    int noNodes(const NodeT* root) const;


/*
~noLeaves~

Returns the number of leaves within the tree rooted by ~root~.

*/
    int noLeaves(const NodeT* root) const;

/*
~noObjects~

Computes the number of stored objects. If an object is
multiple stored, each instance is count.

*/
    int noObjects(const NodeT* root) const;

/*
~height~

Computes the height of the tree rooted by ~root~.

*/
    int height(const NodeT* root) const;

/*
~insert~

Inserts a  set of subtrees at the specified levels.
This function supports the ~erase~ function.

*/
    void insert(const std::set<std::pair < int , NodeT* > >&  Q);

/*
Inserts a node at the specified level. If the tree grows, true is
returned. If a leaf should be inserted, just set level to -1.

*/
    bool insertNodeAtLevel(int level, NodeT* node);

/*
~insertRecAtLevel~

Function supporting the ~insertNodeAtLevel~ function.

*/
    std::pair<NodeT*, NodeT* >*
    insertRecAtLevel(NodeT*& root,  NodeT* node,
            const int targetLevel, const int currentLevel);


    int getHeight(const NodeT* root) const;
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
template<unsigned dim, class T, unsigned max>
class Node{

    friend class RtreeT2<dim,T,max>;
    // friend class RtreeT2<dim,T,max>::iterator;
private:
/*
1.1 Constructors

This constructor creates a leaf node.

*/
    Node(const Rectangle2<dim>& abox, const T& _id):
        box(abox), count(-1), id(_id), sons(0){ }


/*
Copy constructor.

*/
    Node(const Node& src):
        box(src.box), count(src.count), id(src.id) {
        if(src.sons){
            sons = new Node[max+1];
            for(int i = 0; i < count; ++i)
                sons[i] = src.sons[i];
        } else {
            sons = 0;
        }
    }

/*
1.2 The assignment operator.

*/
    Node& operator=(const Node& src){
        this->box = src.box;
        this->count = src.count;
        this->id = src.id;
        if(src.sons) {
            sons = new Node[max+1];
            for(int i = 0; i < count; ++i)
                sons[i] = src.sons[i];
        } else {
            sons = 0;
        }
        return *this;
    }

/*
1.3 Destructor

The destructor does not remove any sons of the node. Just the
array managing the sons is removed. If the complete subtree should
be removed, use a combination of destroy and delete.

*/

    ~Node(){
        if (sons)
            delete[] sons;
    }

/*
1.4 ~destroy~

Deletes the subtrees rooted by this node.

*/
    void destroy(){
        if (sons) {
            for(int i = 0; i < count; i++){
                sons[i]->destroy();
                delete sons[i];
            }
            delete[] sons;
            count = 0;
            sons = 0;
        }
    }

/*
~usedMem~

Returns the memory allocated for this subtree;

*/
    ssize_t usedMem() const{
        if (sons == 0)
            return sizeof(*this);
        size_t sum = 0;
        for (int i = 0; i < count; ++i)
            sum += sons[i]->usedMem();
        return sum + sizeof(*this) + sizeof(sons);
    }


/*
1.4 Data members

*/
    Rectangle2<dim> box; // the bounding box
    int count;           // current count of entries
    const T id;          // content
    Node<dim,T,max>** sons;   // array of sons, 0 for object nodes

/*
1.5 Private functions

~Constructor~

This constructor constructs an empty inner node.

*/

    Node():
        box(), count(0), id(0) {
        sons = new Node<dim,T,max>*[max+1];
    }


/*
~append~

Appends a new entry to this node. If the node overflows,
the result will be false. Not usable for "object nodes".

*/
    bool append(Node* entry){
        assert(count <= max);
        sons[count] = entry;
        if(count == 0)
            this->box = entry->box;
        else
            this->box = this->box.Union(entry->box);
        ++count;
        return count <= max;
    }

/*
~selectFittestSon~

Returns the index of the son which is the best one for
searching further the leaf for including ~box~.

*/
    unsigned int selectFittestSon(const Rectangle2<dim>& box)const{
        assert(sons != 0 && count > 0);
        // initialize best fit index to be 0
        double area = sons[0]->box.Area();
        double extend = sons[0]->box.UnionArea(box) - area;
        int index = 0;
        for(int i = 1; i < count; ++i){
            double area2 = sons[i]->box.Area();
            double extend2 =sons[i]->box.UnionArea(box) - area2;
            if (extend2 < extend){
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
    std::pair<unsigned int, unsigned int> pickSeeds() const{
        std::pair<unsigned int, unsigned int> res;
        double d = 0;
        for (unsigned int i = 0; i < count; ++i) {
            for (unsigned int j = i + 1; j < count; ++j) {
                double d2 = sons[i]->box.UnionArea(sons[j]->box) -
                        (sons[i]->box.Area() + sons[j]->box.Area());
                if (d2 > d) {
                    res.first = i;
                    res.second = j;
                    d = d2;
                }
            }
        }
        if (d == 0) { // all boxes are the same
            res.first = 0;
            res.second = unsigned(count - 1);
        }
        return res;
    }


/*
~pickNext~

returns the next index for the quadratic split algorithm.

*/
    std::pair<unsigned int, unsigned int>
    pickNext(const Node* grpone,const Node* grptwo) const{
        assert(count > 0);
        double d1 = sons[0]->box.UnionArea(grpone->box);
        double d2 = sons[0]->box.UnionArea(grptwo->box);
        unsigned int index = 0;
        unsigned int bestgrp = -1;
        double d = std::abs(d1-d2);
        for(int i = 1; i < count; ++i){
            d1 = sons[i]->box.UnionArea(grpone->box);
            d2 = sons[i]->box.UnionArea(grptwo->box);
            double d3 = std::abs(d1-d2);
            if (d3 > d) {
                d = d3;
                index = i;
                double a1 = grpone->box.Area();
                double a2 = grptwo->box.Area();
                d1 = d1 - a1;
                d2 = d2 - a2;
                if (d1 != d2)
                    bestgrp = d1 < d2 ? 1 : 2;
                else if(a1!=a2)
                    bestgrp = a1 < a2 ? 1 : 2;
                else if(grpone->count != grptwo->count)
                    bestgrp = grpone->count < grptwo->count ? 1 : 2;
                else // all criteria failed
                    bestgrp = 1;
            }
        }
        std::pair<unsigned int, unsigned int> res;
        res.first = index;
        res.second = bestgrp;
        return res;
    }

/*
~Remove~

Removes the entry at position entry from this node.
If there is an underflow, the result will be false.

*/
    bool remove(unsigned int index) {
        for (unsigned int i = index; i < count - 1; i++)
            sons[i] = sons[i + 1];
        sons[count - 1] = 0;
        --count;
        return count >= (max / 2);
    }

/*
~Split~

Splits this node into two ones. After calling this function, this node will
be empty and all contained elements was distributed to the new nodes.

*/
    std::pair<Node*, Node*> split(){
        std::pair<Node<dim,T,max>*, Node<dim,T,max>* > res =
                quadraticSplit();
        return res;
    }


/*
~isLeaf~

This function checks whether this node is a leaf, i.e. whether the sons
are objects nodes.

*/
    bool isLeaf() const{
        assert(sons != 0);
        return (count == 0) || (sons[0]->sons == 0);
    }


/*
~quadraticSplit~

Implementation of the quadratic split algorithm.

*/
    std::pair<Node*, Node*> quadraticSplit(){
        std::pair<unsigned int, unsigned int> seeds = pickSeeds();
        unsigned int index1 = seeds.first;
        unsigned int index2 = seeds.second;
        Node* node1 = new Node();
        Node* node2 = new Node();
        node1->append(sons[index1]);
        node2->append(sons[index2]);
        this->remove(std::max(index1, index2)); // max must be removed first!
        this->remove(std::min(index1, index2));
        unsigned min = max / 2;
        while (count > 0) {
            if (count + node1->count == min) { // all entries to node1
                for (int i = 0; i < count; ++i)
                    node1->append(sons[i]);
                count = 0;
            } else if (count + node2->count == min) { // all entries to node2
                for (int i = 0; i < count; ++i)
                    node2->append(sons[i]);
                count = 0;
            } else {
                std::pair<unsigned int, unsigned int> next =
                        pickNext(node1,node2);
                if (next.second == 1)
                    node1->append(sons[next.first]);
                else
                    node2->append(sons[next.first]);
                remove(next.first);
            }
        }
        return std::make_pair(node1,node2);
    } // end of quadraticsSplit


}; // end of class Node


/*
2 Implementation of the class RTree2

2.1 Constructor

This constructor creates a new empty RTRee2.

*/
template<unsigned dim, class T, unsigned max>
RtreeT2<dim,T,max>::RtreeT2(){
    root = 0;
}

/*
2.2 ~insert~

Inserts a box together with an id into the R-Tree.

*/
template<unsigned dim, class T, unsigned max>
void RtreeT2<dim,T,max>::insert(const Rectangle2<dim>& box, const T& id){
    using NodeT = Node<dim,T,max>;

    if(!root)
        root = new NodeT();
    NodeT* obj = new NodeT(box, id);
    insertNodeAtLevel(-1, obj);
}

/*
~noNodes~

Computes the number of non-object-nodes within the tree.

*/
template<unsigned dim, class T, unsigned max>
int RtreeT2<dim,T,max>::noNodes() const{
    return noNodes(root);
}

template<unsigned dim, class T, unsigned max>
int RtreeT2<dim,T,max>::noNodes(const Node<dim,T,max>* root) const{
    if (!root)
        return 0;
    if (root->isLeaf())
        return 1;

    int sum = 1;
    for(int i = 0; i < root->count; ++i)
        sum += noNodes(root->sons[i]);
    return sum;
}

/*
~noLeaves~

Returns the number of leaves within the tree.

*/
template<unsigned dim, class T, unsigned max>
int RtreeT2<dim,T,max>::noLeaves() const {
    return noLeaves(root);
}

template<unsigned dim, class T, unsigned max>
int RtreeT2<dim,T,max>::noLeaves(const Node<dim,T,max>* root) const{
    if (!root)
        return 0;
    if (root->isLeaf())
        return 1;

    int sum = 0;
    for (int i = 0; i < root->count; ++i)
        sum += noLeaves(root->sons[i]);
    return sum;
}

/*
~noObjects~

Computes the number of stored objects. If an object is
multiple stored, each instance is count.

*/

template<unsigned dim, class T, unsigned max>
int RtreeT2<dim,T,max>::noObjects() const{
    return noObjects(root);
}

template<unsigned dim, class T, unsigned max>
int RtreeT2<dim,T,max>::noObjects(const Node<dim,T,max>* root)const{
    if(!root)
        return 0;
    if (root->isLeaf())
        return root->count;

    int sum = 0;
    for (int i = 0; i < root->count; i++)
        sum += noObjects(root->sons[i]);
    return sum;
}

/*
~height~

Computes the height of the tree.

*/
template<unsigned dim, class T, unsigned max>
int RtreeT2<dim,T,max>::height() const{
    return height(root);
}


template<unsigned dim, class T, unsigned max>
int RtreeT2<dim,T,max>::height(const Node<dim,T,max>* root) const{
    if (!root)
        return -1;

    int h = 0;
    const Node<dim,T,max>* node =  root;
    while(!node->isLeaf()) {
        h++;
        node = node->sons[0];
    }
    return h;
}

/*
~usedMem~

*/
template<unsigned dim, class T, unsigned max>
size_t RtreeT2<dim,T,max>::usedMem() const{
    if (!root)
        return sizeof(*this);
    else
        return sizeof(*this) + root->usedMem();
}


/*
~insert~

Inserts a  set of subtrees at the specified levels. This function supports the
~erase~ function.

*/
template<unsigned dim, class T, unsigned max>
void RtreeT2<dim,T,max>::insert(
        const std::set<std::pair< int, Node<dim,T,max>* > >&  Q){

    int levelDiff = 0; // store the grow of the tree
    typename std::set<std::pair < int , NodeT*> >::iterator it;
    for(it = Q.begin(); it != Q.end(); it++){
        std::pair<int, NodeT*> node = *it;
        int level = node.first < 0 ? node.first : node.first + levelDiff;
        if (insertNodeAtLevel(level, node.second))
            levelDiff++;
    }
}

/*
Inserts a node at the specified level. If the tree grows, true is
returned.

*/
template<unsigned dim, class T, unsigned max>
bool RtreeT2<dim,T,max>::insertNodeAtLevel(int level,
        Node<dim,T,max>* node){
    using NodeT = Node<dim,T,max>;

    if (!root) {
        root = new NodeT();
    }
    std::pair<NodeT*, NodeT* >* res = insertRecAtLevel(root,node, level,0);
    if (!res) { // tree does not grow
        return false;
    } else {
        delete root;
        root = new NodeT();
        root->append(res->first);
        root->append(res->second);
        delete res;
        return true;
    }
}

template<unsigned dim, class T, unsigned max>
std::pair<Node<dim,T,max>*, Node<dim,T,max>* >*
RtreeT2<dim,T,max>::insertRecAtLevel(
        Node<dim,T,max>*& root,
        Node<dim,T,max>* node,
        const int targetLevel,
        const int currentLevel) {
    using NodeT = Node<dim,T,max>;

    if (root->isLeaf() || (targetLevel == currentLevel) ){
        if (root->append(node)){ // no overflow
            return 0;
        } else { // overflow
            std::pair<NodeT*, NodeT*> res
            = root->split();
            delete root;
            root = 0;
            return new std::pair<NodeT*,
                    NodeT*>(res);
        }
    } else { // not the target node
        int index = root->selectFittestSon(node->box);
        std::pair<NodeT*, NodeT*>* res;
        res  = insertRecAtLevel(root->sons[index], node,
                targetLevel, currentLevel+1);
        if (!res){ // son was not split
            root->box = root->box.Union(node->box);
            return 0;
        } else {
            root->sons[index] = res->first; // replace old son by a split node
            root->box = root->box.Union(res->first->box);
            if (root->append(res->second)) { // no overflow
                delete res;
                return 0;
            } else {
                delete res;
                res = new std::pair<NodeT*, NodeT*>( root->split() );
                delete root;
                root = 0;
                return res;
            }
        }
    }
}


template<unsigned dim, class T, unsigned max>
Rectangle2<dim> RtreeT2<dim,T,max>::getBBox() const{
    return (root) ? root->box : Rectangle2<dim>();
}


template<unsigned dim, class T, unsigned max>
void RtreeT2<dim,T,max>::printStats(size_t pointCount,
        size_t pointSize /* = 0 */) const {
    size_t entryCount = noObjects();
    size_t leafCount = noLeaves();
    size_t nodeCount = noNodes();
    size_t memMmRTree = usedMem();
    size_t memPoints = pointCount * pointSize;
    size_t memTotal = memMmRTree + memPoints;
    cout << "Memory used for MMRTree: " <<
            formatInt(memMmRTree) << " bytes for "
            << formatInt(entryCount) << " entries in " <<
            formatInt(leafCount) << " leaves + "
            << formatInt(nodeCount - leafCount) << " inner nodes" << endl;
    if (memPoints > 0) {
        cout << "Memory used for points : " << formatInt(memPoints)
                << " bytes = " << formatInt(pointCount) << " points * "
                << pointSize << " bytes" << endl;
    }
    cout << "Total memory used      : " << formatInt(memTotal) << " bytes = "
            << formatInt(pointCount) << " points * "
            << formatInt((int)std::round(memTotal / (double)pointCount))
            << " bytes" << endl << endl;
}
} // end of namespace
