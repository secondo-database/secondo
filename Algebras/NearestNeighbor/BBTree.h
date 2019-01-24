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

1 The BBTree class

The BBTree is a data structure which can be used to determine the bounding box
of a moving point for a determines time interval.

*/

#ifndef BBTREE_H
#define BBTREE_H


#include "DateTime.h"
#include "Algebras/Temporal/TemporalAlgebra.h" 
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include <iostream>


template<class timeType>
class BBTreeNode;

template<class timeType>
class BBTree{
  public:

/*
~Constructor~

This constructor creates a BBTree for a given temporalalgebra::MPoint;

*/
    BBTree(const temporalalgebra::MPoint& p);

/*
~Copy constructor~

*/
    BBTree(const BBTree& t);
/*
~Assignment Operator~

*/
    BBTree& operator=(const BBTree& src);
/*
~Destructor~

*/
    ~BBTree();

/*
~getBox~

This Operator returns the bounding box of the mpoint
from which created it this tree when it is restricted to the
given time interval.

*/
    Rectangle<2> getBox(
        const temporalalgebra::Interval<timeType>& interval)const;

/*
~noLeafs~

Returns the number of leaves within that tree

*/
    int noLeaves()const;

/*
~noNodes~

Returns the number of nodes within this tree.

*/
    int noNodes() const;
    
/*
~height~

Returns the height of the tree.

*/
    int height() const;

/*
~print~

Writes the tree to o;

*/
  std::ostream& print(std::ostream& o) const;

  private:

/*
~root~

Represents the root of this tree.

*/
    BBTreeNode<timeType>* root;

/*
~createFromMPoint~

Builds the tree for a given mpoint.


*/
    void createFromMPoint(const temporalalgebra::MPoint& p);

};


/*
 Class BBTreeNode

An instance of that class represents a single  node within a BBTree.

*/
template<class timeType>
static temporalalgebra::Interval<timeType> tottInterval(
  const temporalalgebra::Interval<Instant>& iv){
  temporalalgebra::Interval<timeType> result(iv.start.ToDouble(), 
                            iv.end.ToDouble(), 
                            iv.lc, iv.rc);
  return result;
}

inline void toMinimum(double& s){
  s = std::numeric_limits<double>::min();
}

inline void toMaximum(double& s){
  s = std::numeric_limits<double>::max();
}

inline void toMinimum(datetime::DateTime& s){
  s.ToMinimum();
}

inline void toMaximum(datetime::DateTime& s){
  s.ToMaximum();
}



template<class timeType>
class BBTreeNode{
public:
  
/*
2.1 Constructors

2.1.1 Constructor for a leaf

*/  
  BBTreeNode(const temporalalgebra::UPoint& unit):
  unit(new temporalalgebra::UPoint(unit)),
  box(unit.BoundingBoxSpatial()),
  left(0), right(0),
  interval(tottInterval<timeType>(unit.timeInterval))
  { } 

/*
2.1.2 Constructor for inner nodes

*/

  BBTreeNode(BBTreeNode<timeType>* left, BBTreeNode<timeType>* right){
    unit = 0;
    if(left){
      if(right){
        box = left->box.Union(right->box);
      } else {
        box = left->box;
      }
    } else {
      if(right){
        box = right->box;
      }
    }
    this->left = left;
    this->right = right;
    timeType  min(0.0);
    timeType  max(0.0);
    toMinimum(min);
    toMaximum(max);
    
    if(left){
        min=left->interval.start;
        max=left->interval.end;   
    }
    if(right){
       if(right->interval.start < min){
          min = right->interval.start;
       }
       if(right->interval.end > max){
          max = right->interval.end;
       }
    } 
    interval.start = min;
    interval.end = max;
    interval.lc = true;
    interval.rc = true;
  }

/*
2.1. Copy Constructor 

performs a deepth copy

*/
BBTreeNode(const BBTreeNode<timeType>& src){
   if(src.unit){
     unit = new temporalalgebra::UPoint(*src.unit);
   } else {
     unit = 0;
   }
   box = src.box;
   if(src.left){
     left = new BBTreeNode<timeType>(*src.left);
   } else {
     left = 0;
   } 
   if(src.right){
     right = new BBTreeNode<timeType>(*src.right);
   } else {
     right = 0;
   } 
   interval.start = src.interval.start;
   interval.end = src.interval.end;
   interval.lc  = src.interval.lc;
   interval.rc = src.interval.rc;
}

/*
2.2 Assignment Operator

performs a deep copy

*/
  BBTreeNode<timeType>& operator=(const BBTreeNode<timeType>& src){
   if(src.unit){
     unit = new temporalalgebra::UPoint(*src.unit);
   } else {
     unit = 0;
   }
   box = src.box;
   if(src.left){
     left = new BBTreeNode<timeType>(*src.left);
   } else {
     left = 0;
   } 
   if(src.right){
     right = new BBTreeNode<timeType>(*src.right);
   } else {
     right = 0;
   } 
   interval.start = src.interval.start;
   interval.end = src.interval.end;
   interval.lc  = src.interval.lc;
   interval.rc = src.interval.rc;
   return *this;
  }

/*
2. Destructor

*/
  ~BBTreeNode(){
      if(unit){
         delete unit;
         unit=0;
      }
      if(left){
        delete left;
        left = 0;
      }
      if(right){
        delete right;
        right = 0;
      }
   }


/*
2.4 ~getBox~

*/

inline bool AlmostEqual(const Instant& i1, const Instant& i2)const{
   return i1 == i2;
}

inline bool ivInside(const temporalalgebra::Interval<timeType>& iv1, 
                     const temporalalgebra::Interval<timeType>& iv2) const{

  const timeType& e1 = iv1.end;
  const timeType& e2 = iv2.end;
  return( ( (iv1.start > iv2.start) || 
            (AlmostEqual(iv1.start, iv2.start) && ( !iv1.lc || iv2.lc ) ) ) &&
          ( (e1 < e2) || 
            ( AlmostEqual(iv1.end, iv2.end) && ( !iv1.rc || iv2.rc ) ) ) );
}

inline bool ivDisjoint(const temporalalgebra::Interval<timeType>& iv1, 
                       const temporalalgebra::Interval<timeType>& iv2) const{
  const timeType& e1 = iv1.end;
  return( ( (e1 < iv2.start) || 
            ( AlmostEqual(iv1.end, iv2.start) && ( !iv1.lc || !iv2.lc ) ) ) ||
          ( (iv1.start > iv2.end) || 
            ( AlmostEqual(iv1.start, iv2.end) && ( !iv1.rc || !iv2.rc ) ) ) );
}


Rectangle<2> getBox(const temporalalgebra::Interval<timeType>& interval) const{

  //disjoint intervals -> return undef
  if(ivDisjoint(interval,this->interval)){
    Rectangle<2> res(false);
    return res;
  }
  if(ivInside(this->interval, interval)){
    return box;
  }

  if(unit){ // a leaf node
     timeType mind;
     if(interval.start<this->interval.start){
        mind = this->interval.start;
     } else {
        mind = interval.start;
     }  
     timeType maxd;
     const timeType& ie = interval.end;
     const timeType& te = this->interval.end;
     if(ie<te){
        maxd = interval.end;
     } else {
        maxd = this->interval.end;
     }
     Point p0,p1;
     unit->TemporalFunction(datetime::DateTime(mind),p0,true);
     unit->TemporalFunction(datetime::DateTime(maxd),p1,true);
     assert(p0.IsDefined());
     assert(p1.IsDefined());
     return p0.BoundingBox().Union(p1.BoundingBox()); 
  } else { // an inner node
     Rectangle<2> Lres(false);
     if(left){
        Lres  = left->getBox(interval);
     }
     Rectangle<2> Rres(false);
     if(right){
        Rres = right->getBox(interval);
     }
     if(!Lres.IsDefined()){
        return Rres; 
     } else {
       if(!Rres.IsDefined()){
         return Lres;
       } else { // both are defined
         return Lres.Union(Rres);
       }
     }
  }
}

int noLeaves() const{
  if(unit){
     return 1;
  } else {
    int l = left?left->noLeaves():0;
    int r = right?right->noLeaves():0;
    return l+r;
  }
}

int noNodes() const{
    int l = left?left->noNodes():0;
    int r = right?right->noNodes():0;
    return l+r+1;
}

int height() const{
   if(unit) return 0;
   int l = left?left->height():0;
   int r = right?right->height():0;
   return std::max(l,r) +1;  
}

std::ostream& print(std::ostream& o) const{

  if(unit){
     o << "\"" << "U" << "\"";
     return o;
  } else {
     o << "(";
     o << "\"" << "I" << "\"";
     if(left){
       left->print(o);
     } else {
       o << "()";
     }
     if(right){
       right->print(o);
     } else {
       o << "()";
     }
     o << ")";
     return o;
  }
}


private:
  temporalalgebra::UPoint* unit;
  Rectangle<2> box;
  BBTreeNode<timeType>* left;
  BBTreeNode<timeType>* right;
  temporalalgebra::Interval<timeType> interval;
};


template<class timeType>
BBTree<timeType>::BBTree(const temporalalgebra::MPoint& p):root(0){
   createFromMPoint(p);
}

template<class timeType>
BBTree<timeType>::BBTree(const BBTree<timeType>& t):root(0){
  if(t.root){
     root = new BBTreeNode<timeType>(*t.root);
  } else {
     root = 0;
  }
}

template<class timeType>
BBTree<timeType>& BBTree<timeType>::operator=(const BBTree<timeType>& src){
  if(src.root){
    root = new BBTreeNode<timeType>(*src.root);
  } else {
    root = 0;
  }
  return *this;
}

template<class timeType>
BBTree<timeType>::~BBTree(){
  if(root){
    delete root;
    root = 0;
  }
}

template<class timeType>
Rectangle<2> BBTree<timeType>::getBox(
   const temporalalgebra::Interval<timeType>& interval)const{
   if(root){
     return root->getBox(interval);
   } else {
     Rectangle<2> res(false);
     return res;
   }
}

template<class timeType>
int BBTree<timeType>::noLeaves() const{
  if(root) {
    return root->noLeaves();
  } else {
    return 0;
  }
}

template<class timeType>
int BBTree<timeType>::noNodes() const{
  if(root) {
    return root->noNodes();
   } else {
    return 0;
   }
}
    
template<class timeType>
int BBTree<timeType>::height() const{
  if(root){ 
    return root->height();
  } else {
    return -1;
  }
}

template<class timeType>
void BBTree<timeType>::createFromMPoint(const temporalalgebra::MPoint& p){
   int size = p.GetNoComponents();
   if(size==0){
     root = 0;
     return;
   }
   std::stack<std::pair<int, BBTreeNode<timeType>*> > astack;

   for(int i=0; i< size; i++){
      temporalalgebra::UPoint unit;
      p.Get(i,unit);
      BBTreeNode<timeType>* newNode = new BBTreeNode<timeType>(unit);
      std::pair<int, BBTreeNode<timeType>*> entry(0, newNode);
      if(astack.size()==0){ // first entry
         astack.push(entry);
      } else {
         std::pair<int, BBTreeNode<timeType>*> top = astack.top();
         bool done = false;
         while(!done && (top.first == entry.first)){
           BBTreeNode<timeType>* next = 
               new  BBTreeNode<timeType>(top.second,entry.second);
           astack.pop();
           entry = std::pair<int, BBTreeNode<timeType>*> (top.first+1,next);
           done = astack.empty();
           if(!done){
               top = astack.top();
           }
         }
         astack.push(entry);
      } 
   }
   std::pair<int, BBTreeNode<timeType>*> top = astack.top();
   BBTreeNode<timeType>* r = top.second;
   astack.pop();
   while(!astack.empty()){
      top = astack.top();
      astack.pop();
      r = new BBTreeNode<timeType>(top.second,r);
   }
   root = r;
}

template<class timeType>
std::ostream& BBTree<timeType>::print(std::ostream& o) const{
 if(root){
    o << "( tree ";
    root->print(o);
    o << ")";
    return o;
 } else {
   o << "(tree ())";
   return o;
 }
}


#endif




