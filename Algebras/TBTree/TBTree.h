

/*

Implementation of the trajectory bundle tree.

*/

#ifndef TBTREE_H
#define TBTREE_H

#include "TupleIdentifier.h"
#include "RectangleAlgebra.h"
#include "SecondoSMI.h"
#include "TemporalAlgebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include <cassert>
#include <vector>
#include <iostream>

namespace tbtree{


bool CheckTBTree(ListExpr type, ListExpr& ErrorInfo);

/*
1 Class LeafInfo

This class contains the information of an leaf entry in the tree.

*/
class TBLeafInfo{
  public:

/*
~Constructors~

*/
     TBLeafInfo():tid(0){}
     TBLeafInfo(const TupleId& id): tid(id) {}
     TBLeafInfo(const TBLeafInfo& li): tid(li.tid) {}
/*
~ Assignment operator~

*/
     TBLeafInfo& operator=(const TBLeafInfo& li){
       this->tid = li.tid;
       return *this;
     }

     bool operator==(const TBLeafInfo& li) const{
         return (this->tid == li.tid);
     }
/*
~ Destructor~

*/
     ~TBLeafInfo() {}

/*
~Getter~

*/
     inline TupleId getTupleId() const{
       return tid;
     }
/*
Writing into a buffer.

*/
     inline void writeTo(char* buffer,unsigned  int& offset) const{
        memcpy(buffer+offset, &tid, sizeof(TupleId));
        offset += sizeof(TupleId);
     }

/*
Reading from a buffer.


*/
     inline void readFrom(const char* buffer, unsigned int& offset) {
        memcpy(&tid, buffer+offset, sizeof(TupleId));
        offset += sizeof(TupleId);
     }
/*
Size required for a buffer.

*/
     static int bufferSize() {
       return sizeof(TupleId);
     }

/*
  prints out this object

*/
 ostream& print(ostream& os){
   return os <<  tid ;
 }

  private:
     TupleId  tid;
};

/*
2 class InnerInfo

This class contains the information of an entry of an inner node in the tree.

*/
class InnerInfo{
  public:
/*
Constructors

*/
     InnerInfo(): pointer(0){}
     InnerInfo(const SmiRecordId& p): pointer(p) {}
     InnerInfo(const InnerInfo& ii): pointer(ii.pointer) {}

/*
Assignment operator

*/
     InnerInfo& operator=(const InnerInfo& ii){
        pointer = ii.pointer;
        return *this;
     }

     inline bool operator==(const InnerInfo& ii) const{
        return this->pointer == ii.pointer;
     }

/*
Destructor

*/
     ~InnerInfo(){}

/*
Getter

*/
     inline SmiRecordId getPointer() const{
       return pointer;
     }

/*
Writing into a buffer.

*/
     inline void writeTo(char* buffer, unsigned int& offset) const{
        memcpy(buffer + offset, &pointer, sizeof(SmiRecordId));
        offset += sizeof(SmiRecordId);
     }


/*
Reading from a buffer.

*/
     inline void readFrom(const char* buffer,unsigned  int& offset) {
        memcpy(&pointer, buffer + offset, sizeof(SmiRecordId));
        offset += sizeof(SmiRecordId);
     }
/*
Required buffersize to store it.

*/
     static int bufferSize() {
       return sizeof(SmiRecordId);
     }

/*
prints out the pointer

*/
  ostream& print(ostream& os){
    return os << pointer;
  }

  private:
    SmiRecordId pointer;
};



/*
Function copying a defined rectangle into a buffer

*/
template<int Dim>
inline void writeRectangle(const Rectangle<Dim>& r,
                           char* buffer,
                           unsigned int& offset){
   assert(r.IsDefined());
   for(int i = 0; i< Dim; i++){
      double d = r.MinD(i);
      memcpy(buffer+offset, &d, sizeof(double));
      offset += sizeof(double);
      d = r.MaxD(i);
      memcpy(buffer+offset, &d, sizeof(double));
      offset += sizeof(double);
   }
}

/*
Function reading a rectangle from a buffer

*/
template<int Dim>
inline void readRectangle(Rectangle<Dim>& r,
                          const char* buffer,
                          unsigned int& offset){
   double mi[Dim];
   double ma[Dim];
   double d;
   for(int i = 0; i< Dim; i++){
      memcpy(&d, buffer+offset, sizeof(double));
      offset += sizeof(double);
      mi[i] = d;
      memcpy(&d, buffer+offset, sizeof(double));
      offset += sizeof(double);
      ma[i] = d;
   }
   r.Set(true, mi, ma);
}


/*
Size required to store a rectangle.

*/
template<int Dim>
inline int rectBufferSize(){
   return 2 * Dim * sizeof(double);
}


/*
3 Class Entry

This class realized an entry within the tree. It should be
intantiated with InnerInfo or LeafInfo.

*/
template<int Dim, class Info>
class Entry{
  public:
/*
Constructors

*/
    Entry() {}
    Entry(const Rectangle<Dim>& b,const Info& i): box(b), info(i) {}
    Entry(const Entry<Dim, Info>& src): box(src.box), info(src.info) {}
/*
Assignment operator

*/

    Entry<Dim, Info>& operator=(const Entry<Dim, Info>& e){
      this->box = e.box;
      this->info = e.info;
       return *this;
    }

    inline bool operator==(const Entry<Dim, Info>& e) const{
      return (this->box == e.box) && (this->info==e.info);
    }


/*
Destructor

*/
    ~Entry() {}

/*
Getter

*/
    inline const Rectangle<Dim> getBox() const{
      return box;
    }

    inline const Info& getInfo() const{
      return info;
    }

/*
 Writing into a buffer

*/
    inline void writeTo(char* buffer, unsigned int& offset) const{
      writeRectangle<Dim>(box, buffer, offset);
      info.writeTo(buffer, offset);
    }

/*
 Reading from a buffer

*/
    inline void readFrom(const char* buffer,unsigned  int& offset) {
      readRectangle<Dim>(box, buffer, offset);
      info.readFrom(buffer, offset);
    }

/*
Required BufferSize

*/
    inline static int bufferSize(){
      return rectBufferSize<Dim>() + Info::bufferSize();
    }

/*
Let the box be the union of the original box and b;

*/
   inline void addBox(const Rectangle<Dim>& b){
      box = box.Union(b);
   }

/*
Prints entry to stream;

*/
  ostream& print(ostream& os){
     os << "(";
     box.Print(os);
     os << ", ";
     info.print(os);
     os << ")";
     return os;
  }

  private:
    Rectangle<Dim> box;
    Info           info;
};

/*
class BasicNode

This is the super class for all types of nodes.

*/
template<unsigned Dim>
class BasicNode{
  public:

/*
1 Constructors

*/
   BasicNode(uint16_t min1, uint16_t max1):
      min(min1), max(max1), current(0) {}
   BasicNode(const BasicNode<Dim>& src):
      min(src.min), max(src.max), current(src.current) {}
/*
2 Assignment operator

*/
   BasicNode<Dim>& operator=(const BasicNode<Dim>& src){
       this->min = src.min;
       this->max = src.max;
       this->current = src.current;
       return *this;
   }

/*
3 Comparison

*/
   inline bool operator==(const BasicNode<Dim>& bn) const{
       return this->min == bn.min &&
              this->max == bn.max &&
              this->current == bn.current;
   }

/*
4 some pure virtual functions.

*/
   virtual ~BasicNode() {}

   virtual void writeTo(char* buffer, unsigned int& offset) const = 0;
   virtual void readFrom(const char* buffer,unsigned  int& offset) = 0;
   virtual Rectangle<Dim> getBox() const = 0;
   virtual const bool isLeaf()const = 0;
   virtual int bufferSize() const =0;
   virtual ostream& print(ostream& os ) const=0;
   virtual BasicNode<Dim>* clone() const=0;

/*
5   some Getter


*/

   uint16_t entryCount() const{
     return current;
   }

   inline uint16_t getMin() const{
      return min;
   }

   inline virtual uint16_t getMax() const{
      return max;
   }


  protected:
    uint16_t min;      // minimum entrycount
    uint16_t max;      // maximum entrycount
    uint16_t current;  // current entrycount
};

/*
4 Class Node

This is the super class of inner nodes and leaf nodes.

*/
template<unsigned Dim, class Info>
class Node : public BasicNode<Dim>{
 public:
/*
4.1 Constructors

*/

   Node(int min1, int max1): BasicNode<Dim>(min1,max1){
      entries = new Entry<Dim, Info>*[BasicNode<Dim>::max +1];
      for(int i=0;i<BasicNode<Dim>::max+1; i++){
        entries[i] = 0;
      }
   }

   Node(const Node<Dim, Info>& src) : BasicNode<Dim>(src){
       entries = new Entry<Dim,Info>*[BasicNode<Dim>::max +1];
       for(int i=0;i< src.entryCount(); i++){
         this->entries[i] = new Entry<Dim, Info>(*(src.entries[i]));
       }
       for(int i=src.entryCount(); i<BasicNode<Dim>::max; i++){
         this->entries[i] = 0;
       }
    }

/*
4.2 Assignment operator

*/
    Node& operator=(const Node<Dim, Info>& src) {
       // first, delete all included entries
       for(int i=0; i< BasicNode<Dim>::current; i++){
          delete entries[i];
          entries[i] = 0;
       }
       // if the size is different, adopt it
       if(BasicNode<Dim>::max!=src.max){
         delete[] entries;
         entries = new Entry<Dim, Info>*[src.max+1];
       }
       BasicNode<Dim>::operator=(src);
       for(int i=0;i< BasicNode<Dim>::current; i++){
         entries[i] = new Entry<Dim, Info>(*(src.entries[i]));
       }
    }

/*
4.3 Comparision

*/

    bool operator==(const Node<Dim, Info>& n) const{
      if(! (BasicNode<Dim>::operator==(n))){
         return false;
      }

      for(int i=0;i<BasicNode<Dim>::current; i++){
        if(!(  *(this->entries[i]) == *(n.entries[i]))){
          return false;
        }
      }
      return true;
    }


/*
4.4 Destructor

*/
   virtual ~Node(){
      for(int i=0;i<BasicNode<Dim>::current;i++){
       delete entries[i];
      }
      BasicNode<Dim>::current = 0;
      delete[] entries;
      entries = 0;
    }
/*
4.5 ~deleteEntries~

Removes all entries from this  node.

*/
   inline void deleteEntries() {
      for(int i=0;i<BasicNode<Dim>::current;i++){
       delete entries[i];
      }
      BasicNode<Dim>::current = 0;
    }

/*
4.6 ~getEntry~

Returns the entry at the given position.

*/
    inline const Entry<Dim, Info>* getEntry(int index) const{
       assert(index<BasicNode<Dim>::current);
       return entries[index];
    }

/*
4.7 ~getBox~

Returns the union of all boxes of all entries.

*/
    inline virtual Rectangle<Dim> getBox() const{
       if(BasicNode<Dim>::current==0){
          Rectangle<Dim> r(false);
          return r;
       } else {
         Rectangle<Dim> res(entries[0]->getBox());
         for(int i=1; i< BasicNode<Dim>::current; i++){
            res = res.Union(entries[i]->getBox());
         }
         return res;
       }
    }

/*
4.8 ~writeTo~

Writes this node to an existing buffer.

*/

   inline virtual void writeTo(char* buffer, unsigned int& offset) const{
      uint16_t c = BasicNode<Dim>::current;
      memcpy(buffer+offset, &c, sizeof(uint16_t));
      offset += sizeof(uint16_t);
      for(int i=0; i< BasicNode<Dim>::current; i++){
         entries[i]->writeTo(buffer, offset);
      }
   }

/*
4.9 ~readFrom~

Reconstructs the node from a buffer.

*/
   inline virtual void readFrom(const char* buffer,unsigned  int& offset){
      // delete old entries
      for(int i=0;i<BasicNode<Dim>::current; i++){
        delete entries[i];
        entries[i] = 0;
      }
      uint16_t c;
      memcpy(&c, buffer+offset, sizeof(uint16_t));
      BasicNode<Dim>::current = c;
      offset += sizeof(uint16_t);
      for(int i=0; i< BasicNode<Dim>::current; i++){
         entries[i] = new Entry<Dim, Info>();
         entries[i]->readFrom(buffer, offset);
      }
   }

/*
4.10 ~bufferSize~

Returns the number of bytes required to store this node.

*/
   inline virtual int bufferSize() const{
     return sizeof(uint16_t) +
            BasicNode<Dim>::current*Entry<Dim, Info>::bufferSize();
   }

/*
4.11 ~getMax~

Return the maximum count of nodes which can be placed in a buffer with
given size.

*/
   static uint32_t getMax(const uint32_t buffersize){
       uint32_t ls = buffersize - sizeof(uint16_t); // current
       return ls / Entry<Dim, Info>::bufferSize();
   }


/*
4.12 ~isFull~

Returns true if no more elements can be inserted into this node.

*/
   inline bool isFull() const{
      return BasicNode<Dim>::current == BasicNode<Dim>::max -1;
   }

/*
4.12 ~insert~

Appends a new entry. If there is an overflow, the result will be true.

*/

   inline bool insert(const Entry<Dim, Info>& e){
     assert(BasicNode<Dim>::current <= BasicNode<Dim>::max);


     entries[BasicNode<Dim>::current] = new Entry<Dim, Info>(e);
     BasicNode<Dim>::current++;
     return BasicNode<Dim>::current==BasicNode<Dim>::max+1;
   }

/*
4.14 ~addBoxAt~

Builds the uion of the given box and the box of element at position pos
and stores the result at posiotion pos.

*/

   inline void addBoxAt(const int pos, const Rectangle<Dim>& r){
      entries[pos]->addBox(r);
   }

/*
4.15 ~remove~

Removes the entrie at position pos from this node.

*/
   inline void remove(const int index){
      assert(index < BasicNode<Dim>::current);
      delete entries[index];
      BasicNode<Dim>::current--;
      for(int i=index; i<BasicNode<Dim>::current; i++){
         entries[i] = entries[i+1];
      }
      entries[BasicNode<Dim>::current] = 0;
   }


/*
4.16 getEmptyNode()

pure virtual.


*/
   virtual Node* getEmptyNode()const = 0;


/*
~updateEntry~

Replaces the entry at the given position by the new one.

*/
   inline void updateEntry(int pos, Entry<Dim, Info> e){
     assert(pos < BasicNode<Dim>::current);
     *(entries[pos]) = e;
   }

/*
~isLeaf~

pure virtual.

*/
   virtual const bool isLeaf() const = 0;


/*
~print~

*/
   ostream& print(ostream& os) const{
       os << "(node - ";
       os << (isLeaf() ? "leaf" : "inner");
       os << "[" << BasicNode<Dim>::min <<", "
          << BasicNode<Dim>::max << ", "
          << BasicNode<Dim>::current << "] ::{"  ;
       for(int i=0;i<BasicNode<Dim>::current;i++){
         entries[i]->print(os);
       }
       os << "}])";
       return os;
   }

  protected:
    Entry<Dim, Info>** entries;
};


/*
Class ~QNodeSplitter~

This class provides some functions to split a node into 2 new ones
using quadratic split algorithm.

*/
template<int Dim, class Info>
class QNodeSplitter{
  public:
   pair<Node<Dim, Info>*, Node<Dim, Info>* >
     splitNode(Node<Dim, Info>& node){
       pair<int, int> seeds = pickSeeds(node);
       int index1 = seeds.first;
       int index2 = seeds.second;
       Node<Dim, Info>*  node1 = node.getEmptyNode();
       Node<Dim, Info>*  node2 = node.getEmptyNode();
       node1->insert(*(node.getEntry(index1)));
       node2->insert(*(node.getEntry(index2)));
       node.remove(std::max(index1,index2));
       node.remove(std::min(index1,index2));
       int min = node.getMin();

       while(node.entryCount() > 0){
         if(node.entryCount() + node1->entryCount() == min){
           for(int i=0;i<node.entryCount();i++){
              node1->insert(*node.getEntry(i));
           }
           node.deleteEntries();
         }else if(node.entryCount() + node2->entryCount() == min){
           for(int i=0;i<node.entryCount();i++){
              node2->insert(*(node.getEntry(i)));
           }
           node.deleteEntries();
         } else {
           pair<unsigned int, unsigned int> next = pickNext(&node,node1,node2);
           if(next.second == 1){
             node1->insert(*(node.getEntry(next.first)));
           } else {
             node2->insert(*node.getEntry(next.first));
           }
           node.remove(next.first);
         }
       }
       return make_pair(node1,node2);
     } // end of splitNode

  private:

  typedef Node<Dim,Info> node_type;

  pair<unsigned int,unsigned  int> pickSeeds(const node_type& n) const{
     pair<int, int> res;
     double d = 0;
     for(int i=0;i<n.entryCount();i++){
       for(int j=i+1;j<n.entryCount();j++){
         Rectangle<Dim> r1(n.getEntry(i)->getBox());
         Rectangle<Dim> r2(n.getEntry(j)->getBox());
         double d2 = 2 * r1.Union(r2).Area()  - (r1.Area() + r2.Area());
         if(d2>d){
           res.first = i;
           res.second = j;
           d = d2;
         }
       }
     }
     if(d==0){ // all boxes are the same
       res.first = 0;
       res.second = n.entryCount()-1;
     }
     return res;
   }

   pair<unsigned int,unsigned int>
   pickNext(const node_type* source,
            const node_type* grpone,
            const node_type* grptwo) const{
    double d1 = source->getEntry(0)->getBox().Union(grpone->getBox()).Area();
    double d2 = source->getEntry(0)->getBox().Union(grptwo->getBox()).Area();
    unsigned int index = 0;
    unsigned int bestgrp = -1;
    double d = abs(d1-d2);
    Rectangle<Dim> b1 = grpone->getBox();
    Rectangle<Dim> b2 = grptwo->getBox();
    double a1 = b1.Area();
    double a2 = b2.Area();

    for(int i=1;i<source->entryCount();i++){
        d1 = source->getEntry(i)->getBox().Union(b1).Area();
        d2 = source->getEntry(i)->getBox().Union(b2).Area();
        double d3 = abs(d1-d2);
        if(d3>d){
           d = d3;
           index = i;
           d1 = d1 - a1;
           d2 = d2 - a2;
           if(d1!=d2){
             bestgrp = d1<d2?1:2;
           } else if(a1!=a2){
             bestgrp = a1<a2?1:2;
           } else if(grpone->entryCount()!=grptwo->entryCount()){
             bestgrp = grpone->entryCount()<grptwo->entryCount()? 1:2;
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




};




/*
5 class TBLeafNode

*/
template<int Dim, class Info>
class TBLeafNode : public Node<Dim, Info>{
  public:
/*
5.1 Constructors

*/

    TBLeafNode() {}
    TBLeafNode(int min, int max, int trjid1):
              Node<Dim, Info>(min,max), next(0), trjid(trjid1){}

    TBLeafNode(const TBLeafNode& src): Node<Dim, Info>(src),
                                   next(src.next),
                                   trjid(src.trjid){ }
/*
5.2 Assignment Operator

*/

    inline TBLeafNode& operator=(const TBLeafNode<Dim,Info>& src){
       Node<Dim, Info>::operator=(src);
       this->next = src.next;
       this->trjid = src.trjid;
       return *this;
    }

/*
5.3 Comparison

*/
    inline bool operator==(const TBLeafNode<Dim, Info>& ln)const{
       return Node<Dim, Info>::operator==(ln) &&
              this->next == ln.next &&
              this->trjid == ln.trjid;
    }

/*
5.4 Destructor

*/
    virtual ~TBLeafNode(){
       Node<Dim, Info>::deleteEntries();
    }
/*
5.5 isLeaf

Returns allways true.

*/
   inline virtual const bool isLeaf() const{
      return true;
   }

/*
5.6 Import/Export to a buffer

*/
   inline virtual void writeTo(char* buffer, unsigned int& offset) const{
      Node<Dim, Info>::writeTo(buffer, offset);
      memcpy(buffer + offset, &next, sizeof(SmiRecordId));
      offset += sizeof(SmiRecordId);
      memcpy(buffer + offset, &trjid, sizeof(int));
      offset += sizeof(int);
   }

   inline virtual void readFrom(const char* buffer,unsigned  int& offset){
      Node<Dim, Info>::readFrom(buffer, offset);
      memcpy(&next, buffer + offset, sizeof(SmiRecordId));
      offset += sizeof(SmiRecordId);
      memcpy(&trjid, buffer + offset, sizeof(int));
      offset += sizeof(int);
   }

   inline int bufferSize() const{
      return Node<Dim, Info>::bufferSize() + sizeof(SmiRecordId)+sizeof(int);
   }

   inline uint16_t getMax(){
      return BasicNode<3>::getMax();
   }



/*
~getMax~

Returns the maximum number of nodes whiach can be placed into a buffer with
given size.

*/
   static uint32_t getMax(const uint32_t buffersize){
       uint32_t ls = buffersize - (sizeof(SmiRecordId) + sizeof(int));
       return Node<Dim,Info>::getMax(ls);
   }


/*
Getter / Setter

*/
   inline SmiRecordId getNext() const{
      return next;
   }

   inline void setNext(const SmiRecordId id){
     next = id;
   }

   inline int getTrjId() const{
     return trjid;
   }

   inline void setTrjId(const int trjid){
     this->trjid = trjid;
   }

/*
Returns a node wiout any entries.

*/
   inline Node<Dim, Info>* getEmptyNode() const{
       return new TBLeafNode<Dim, Info>(BasicNode<Dim>::min,
                                      BasicNode<Dim>::max, 0);
   }

/*
Prints out this node.

*/

  virtual ostream& print(ostream& os)const{
     return Node<Dim, Info>::print(os) << next << endl;
  }

/*
Returns a clone of this node.

*/
  inline virtual BasicNode<Dim>* clone() const{
     return new TBLeafNode<Dim, Info>(*this);
  }

  private:
    SmiRecordId next;
    int trjid;
};


/*
5 class InnerNode

*/
template<int Dim, class Info>
class InnerNode : public Node<Dim, Info>{
  public:
/*
5.1 Constructors

*/
    InnerNode(const int min1, const int max1): Node<Dim, Info>(min1, max1) {}

    InnerNode(const InnerNode<Dim, Info>& src): Node<Dim, Info>(src) {}

/*
5.2 Assignment operator

*/
    inline InnerNode& operator=(const InnerNode<Dim, Info>& src) {
      return Node<Dim, Info>::operator=(src);
    }

/*
5.3 Comparison

*/
    inline bool operator==(const InnerNode<Dim, Info>& in) const{
        return Node<Dim, Info>::operator==(in);
    }

/*
5.4 Destructor

*/
    virtual ~InnerNode(){
       Node<Dim, Info>::deleteEntries();
    }

/*
~isLeaf~

returns allways false.

*/
    inline virtual const bool isLeaf() const{
       return false;
    }

/*
~selectBestNode~

for details see Gutmans paper about the r-tree.

*/
    int selectBestNode(const Rectangle<Dim>& r) const{
      assert(  (Node<Dim, Info>::current)>0);
      int res = 0;
      Rectangle<Dim> r1(Node<Dim, Info>::entries[0]->getBox());
      double diff = r1.Union(r).Area() - r1.Area();
      for(int i=1; i< (Node<Dim, Info>::current); i++){
         r1 = Node<Dim, Info>::entries[i]->getBox();
         double diff2 = r1.Union(r).Area() - r1.Area();
         if(diff2<diff){
            res = i;
            diff = diff2;
         }
      }
      return res;
    }

/*
~getEmptyNode~

Returns a node without any entries.

*/
    inline Node<Dim, Info>* getEmptyNode() const{
       return new InnerNode<Dim, Info>(BasicNode<Dim>::min,
                                       BasicNode<Dim>::max);
    }
/*
~clone~

Returns a clone of this node.

*/
    inline virtual BasicNode<Dim>* clone() const{
        return new InnerNode<Dim, Info>(*this);
    }

/*
~getMax~

Returns the maximum number of inner nodes which can be stored
into a buffer of given size.

*/
    inline static uint32_t getMax(uint32_t buffersize){
       return Node<Dim,Info>::getMax(buffersize);
    }


};


/*
5 Class PathEntry

This class supports the searchNode function of a tbtree.

*/

template<int Dim>
struct pathEntry{
 public:
  pathEntry(const SmiRecordId id1, BasicNode<Dim>* node1, const int pos1):
    id(id1), node(node1), pos(pos1){}
  pathEntry(const pathEntry& pe):id(pe.id), node(pe.node), pos(pe.pos){}

  inline pathEntry& operator=(const pathEntry& s){
     this->id = s.id;
     this->node = s.node;
     this->pos = s.pos;
     return *this;
  }

  inline bool operator==(const pathEntry& s)const{
    return this->id == s.id &&
           this->node == s.node  &&
           this->pos == s.pos;
  }

  ~pathEntry(){}

  SmiRecordId    id;   // the record id
  BasicNode<Dim>*  node; // the node corresponding to that id
  int            pos;  // position of the next entry

};

/*
7 class TBTree


This is the implementation of a tbtree. In contrast to the
original paper, we don't store the direction in each leaf entry.
Instead we store an trip id in each leaf node.


*/

class TBTree{
  public:

/*
 Creates an empty TBTree

*/
  TBTree(const int pageSize): file(true, pageSize-64),rootId(0){
     file.Create();
     int size = file.GetRecordLength();
     recordLength = size;
     leafMax = TBLeafNode<3, TBLeafInfo>::getMax(size);
     leafMin = leafMax / 4;

     innerMax = InnerNode<3, InnerInfo>::getMax(size);

     innerMin = innerMax / 4;
     noEntries = 0;
     noNodes = 0;
     noLeafNodes = 0;
     level = 0;
     Rectangle<3> b(false);
     box = b;
     SmiRecord dummy;
     headerId =0;
     saveHeader();
  }

/*
Opens an existing TBTRee from file

*/

  TBTree(const SmiFileId id, SmiRecordId hid): file(true), headerId(hid){
     file.Open(id);
     readHeader();
  }


/*
~ Destructor ~

*/
  ~TBTree(){
     if(file.IsOpen()){
       saveHeader();
       file.Close();
     }
   }


/*
Returns the fileId of the underlying file.

*/

  inline SmiFileId getFileId() {
    return file.GetFileId();
  }


/*
Returns the record id of the record storing the header of the tree.

*/
  inline SmiRecordId getHeaderId() const{
    return headerId;
  }


/*
Returns the record id storing the root of the tree. If the tree is emty, the
result will be 0.

*/
  inline SmiRecordId getRootId() const{
     return rootId;
  }

/*
Returns the node stored at the given id.

*/
  inline BasicNode<3>* getNode(const SmiRecordId id){
     return readNode(id);
  }


/*
Stores a new entry.

*/
  void insert(const UPoint& up, const int trjid, const TupleId tid){
     if(!up.IsDefined()){
        return;
     }
     noEntries++;
     TBLeafInfo li(tid);
     Entry<3, TBLeafInfo> e(up.BoundingBox(), li);
     if(!rootId){ // the tree is empty
        TBLeafNode<3, TBLeafInfo> node(leafMin, leafMax, trjid);
        node.insert(e);
        rootId = saveNode(node);
        level++;
        box = e.getBox();
        return;
     } else {
        box = box.Union(e.getBox());
        vector<pathEntry<3> > path;
        TBLeafNode<3, TBLeafInfo>* newLeaf(0);
        SmiRecordId id(0);
        if(searchNode(rootId, path, e, trjid)) {
              TBLeafNode<3, TBLeafInfo>* leaf =
                   dynamic_cast<TBLeafNode<3, TBLeafInfo>* >
                        (path[path.size()-1].node);
           if(!leaf->isFull()) {
              leaf->insert(e);
              updateNode(path[path.size()-1].id, *leaf);
              Rectangle<3> b = leaf->getBox();
              updatePath(path, b);
           } else {
              assert(! leaf->getNext());
              newLeaf = new TBLeafNode<3, TBLeafInfo>(leafMin, leafMax, trjid);
              newLeaf->insert(e);
              id = saveNode(*newLeaf);
              leaf->setNext(id);
              updateNode(path[path.size()-1].id, *leaf); // update next
           }
        } else { // no predecessor was found -> create  a new leaf
           newLeaf = new TBLeafNode<3, TBLeafInfo>(leafMin, leafMax,trjid);
           newLeaf->insert(e);
           id = saveNode(*newLeaf);
        }


        // path processed or needless, delete the entries
        for(unsigned int i=0;i<path.size(); i++){
          assert(path[i].node); // dont allow an empty entry
          delete path[i].node;
          path[i].node = 0;
        }
        while(!path.empty()){
          path.pop_back();
        }
        if(newLeaf){ // there is new leaf which has to
                     // inserted into the tree structure
            SmiRecordId newRootId = insertLeaf(id, e.getBox());
            if(newRootId!=rootId){
              level++;
            }
            rootId = newRootId;
            delete newLeaf;
        }
     }
  }

/*
~insertLeaf~

Stores a new Leaf in the tree. To be used in bulk loading.
Returns the id of the used record

*/
  SmiRecordId insertLeaf(const TBLeafNode<3, TBLeafInfo>& leaf,
                         const SmiRecordId predecessor = 0){
     assert(leaf.entryCount() >0);

     noEntries += leaf.entryCount();
     SmiRecordId id = saveNode(leaf); // save the leaf
     if(!rootId){ // tree was empty
       level++;
       box = leaf.getBox();
       rootId = id;
       return id;
     }

     // there is a predecessor
     if(predecessor){
        TBLeafNode<3, TBLeafInfo>* pred =
             dynamic_cast<TBLeafNode<3, TBLeafInfo>*>(readNode(predecessor));
        pred->setNext(id);
        updateNode(predecessor, * pred);
        delete pred;
     }
     // update bounding box
     box = box.Union(leaf.getBox());
     // insert the new leaf into the structure
     SmiRecordId newRootId = insertLeaf(id, leaf.getBox());
     if(newRootId!=rootId){
       level++;
       rootId = newRootId;
     }
     return id;
  }

/*
Returns a new empty leaf node

*/
 TBLeafNode<3, TBLeafInfo>* getEmptyLeaf(const TupleId trjid){
       return new TBLeafNode<3, TBLeafInfo>(leafMin, leafMax, trjid);
 }


/*
  Deletes the underlying file. After that operator, the TBTree is not longer
  usable.

*/

  void deleteFile(){
     file.Close();
     file.Drop();
  }

    string toString(){

       ostringstream os;
       os << "[ -- tbtree -- " << endl
          << "  fileId       : " << file.GetFileId() << endl
          << "  rootId       : " << rootId << endl
          << "  headerId     : " << headerId << endl
          << "  leafMin      : " << leafMin << endl
          << "  leafMax      : " << leafMax << endl
          << "  innerMin     : " << innerMin << endl
          << "  innerMax     : " << innerMax << endl
          << "  entries      : " << noEntries << endl
          << "  nodes        : " << noNodes  << endl
          << "  leafNodes    : " << noLeafNodes << endl
          << "  level        : " << level << endl
          << "  recordLength : " << recordLength << endl
          << "  box          : ";
       if(!box.IsDefined()){
          os << "undef";
       } else {
          os << box;
       }
       os << endl << "]";
       return os.str();

    }
/*
Some Getter

*/
  int entryCount() const{
    return noEntries;
  }
  int nodeCount() const{
    return noNodes;
  }
  int leafnodeCount() const{
    return noLeafNodes;
  }

  int getHeight() const{
    return level;
  }

  Rectangle<3> getBox()const{
     return box;
  }

  bool getFileStats( SmiStatResultType &result ) {
      result = file.GetFileStatistics(SMI_STATS_EAGER);
      std::stringstream fileid;
      fileid << file.GetFileId();
      result.push_back(pair<string,string>("FilePurpose",
            "SecondaryTBTreetreeIndexFile"));
      result.push_back(pair<string,string>("FileId",fileid.str()));
      return true;
  }

  static const string BasicType(){
     return "tbtree";
  }

  static const bool checkType(ListExpr type){
    ListExpr errorInfo = listutils::emptyErrorInfo();
    return CheckTBTree(type, errorInfo);
  }


/*
calculate number of different trajectories in this node

*/
  int getcoverage(BasicNode<3>* n){
    if(n->isLeaf())
      return 1;
    InnerNode<3,InnerInfo>* innernode =
                  dynamic_cast<InnerNode<3, InnerInfo>* >(n);
    vector<int> trajid;
    vector<Entry<3,InnerInfo> > list1;
    vector<Entry<3,InnerInfo> > list2;
    for(unsigned int i = 0;i < innernode->entryCount();i++){
      list1.push_back(*(innernode->getEntry(i)));
    }
    SmiRecordId adr;
    while(!list1.empty()){
      for(unsigned int i = 0;i < list1.size();i++){
        adr = list1[i].getInfo().getPointer();
        BasicNode<3>* node = getNode(adr);
        if(node->isLeaf()){ //get trajid
          TBLeafNode<3,TBLeafInfo>* leafnode =
                  dynamic_cast<TBLeafNode<3, TBLeafInfo>* >(node);
          trajid.push_back(leafnode->getTrjId());
        }
        else{
          innernode = dynamic_cast<InnerNode<3, InnerInfo>* >(node);
          for(unsigned int j = 0;j < innernode->entryCount();j++)
            list2.push_back(*(innernode->getEntry(j)));
        }
        delete node;
      }
      list1.clear();
      list1.swap(list2);
      list2.clear();
    }
    stable_sort(trajid.begin(),trajid.end());
    vector<int>::iterator end;
    end = unique(trajid.begin(),trajid.end());
    vector<int>::iterator begin = trajid.begin();
    int count = 0;
    while(begin != end){
      begin++;
      count++;
    }
    return count;
  }

  // for CloneTBtree
  void Clone(TBTree*);
  SmiRecordId DFVisit_TBtree(TBTree*,tbtree::BasicNode<3>*);

  private:
    SmiRecordFile file;
    SmiRecordId rootId;
    SmiRecordId headerId;
    uint16_t leafMin;
    uint16_t leafMax;
    uint16_t innerMin;
    uint16_t innerMax;

    uint32_t noEntries;  // numer of stored entries
    uint32_t noNodes;    // number of nodes in the tree
    uint32_t level;      // height of the tree
    uint32_t noLeafNodes;  // number of leaf nodes
    Rectangle<3> box;    // bounding boc of the whole tree
    uint32_t recordLength;

    void saveHeader() {

       //unsigned int size = 4*sizeof(uint16_t) + sizeof(SmiRecordId) +
       //                    4*sizeof(uint32_t) + sizeof(char) +
       //                    rectBufferSize<3>();
       unsigned int size = recordLength;
       char buffer[size];
       memset(buffer,0,size);
       unsigned int offset = 0;
       // rootid
       memcpy(buffer+offset, &rootId, sizeof(SmiRecordId));
       offset += sizeof(SmiRecordId);
       // min, max stuff
       memcpy(buffer+offset, &leafMin, sizeof(uint16_t));
       offset += sizeof(uint16_t);
       memcpy(buffer+offset, &leafMax, sizeof(uint16_t));
       offset += sizeof(uint16_t);
       memcpy(buffer+offset, &innerMin, sizeof(uint16_t));
       offset += sizeof(uint16_t);
       memcpy(buffer+offset, &innerMax, sizeof(uint16_t));
       offset += sizeof(uint16_t);
       // counters
       memcpy(buffer+offset, &noEntries, sizeof(uint32_t));
       offset += sizeof(uint32_t);
       memcpy(buffer+offset, &noNodes, sizeof(uint32_t));
       offset += sizeof(uint32_t);
       memcpy(buffer+offset, &noLeafNodes, sizeof(uint32_t));
       offset += sizeof(uint32_t);
       memcpy(buffer+offset, &level, sizeof(uint32_t));
       offset += sizeof(uint32_t);
       // bounding box
       if(box.IsDefined()){
          char c = 1;
          memcpy(buffer+offset,&c,sizeof(char));
          offset += sizeof(char);
          writeRectangle<3>(box, buffer, offset);
       } else {
          char c = 0;
          memcpy(buffer+offset,&c,sizeof(char));
          offset += sizeof(char);
          double d = -1;
          for(int i=0;i<2*3; i++){
             memcpy(buffer+offset,&d, sizeof(double));
             offset += sizeof(double);
          }
       }

       if(offset>size){
           cout << "offset = " << offset << "  , size = " << size << endl;
       }
       assert(offset<=size);

       // store the buffer
       SmiRecord record;
       if(headerId != 0){
           file.SelectRecord(headerId, record, SmiFile::Update);
           SmiSize hss = record.Write(buffer,size,0);
           assert(hss == size);
       } else {
           file.AppendRecord(headerId, record);
           assert(headerId !=0);
           SmiSize hss = record.Write(buffer,size,0);
           assert(hss == size);
       }
       record.Finish();
    }

    void readHeader(){
       // read buffer
       unsigned int size = 4*sizeof(uint16_t) + sizeof(SmiRecordId) +
                           4*sizeof(uint32_t) + sizeof(char) +
                           rectBufferSize<3>() ;
       char buffer[size];
       SmiRecord record;
       file.SelectRecord(headerId, record);
       recordLength = record.Size();
       SmiSize hs = record.Read(buffer,size,0);
       unsigned int offset = 0;
       assert(hs == size);
       memcpy(&rootId, buffer+offset, sizeof(SmiRecordId));
       offset += sizeof(SmiRecordId);
       memcpy(&leafMin, buffer+offset, sizeof(uint16_t));
       offset += sizeof(uint16_t);
       memcpy(&leafMax, buffer+offset, sizeof(uint16_t));
       offset += sizeof(uint16_t);
       memcpy(& innerMin, buffer+offset, sizeof(uint16_t));
       offset += sizeof(uint16_t);
       memcpy(&innerMax, buffer+offset, sizeof(uint16_t));
       offset += sizeof(uint16_t);
       // counters
       memcpy(&noEntries, buffer+offset, sizeof(uint32_t));
       offset += sizeof(uint32_t);
       memcpy(&noNodes, buffer+offset, sizeof(uint32_t));
       offset += sizeof(uint32_t);
       memcpy(&noLeafNodes, buffer+offset, sizeof(uint32_t));
       offset += sizeof(uint32_t);
       memcpy(&level, buffer+offset, sizeof(uint32_t));
       offset += sizeof(uint32_t);
       // bounding box
       char c;
       memcpy(&c, buffer+offset, sizeof(char));
       offset += sizeof(char);
       if(c==0){
          box.SetDefined(false);
       } else {
         readRectangle<3>(box, buffer, offset);
       }
    }

    static bool EqualNodes(const BasicNode<3>* n1, const BasicNode<3>* n2) {
       if(n1->isLeaf()!=n2->isLeaf()){
          return false;
       }
       if(n1->isLeaf()){
          const TBLeafNode<3, TBLeafInfo>* ln1 =
                dynamic_cast<const TBLeafNode<3, TBLeafInfo>*>(n1);
          const TBLeafNode<3, TBLeafInfo>* ln2 =
                dynamic_cast<const TBLeafNode<3, TBLeafInfo>*>(n2);
          return  *ln1 == *ln2;
       } else {
          const InnerNode<3, InnerInfo>* in1 =
                dynamic_cast<const InnerNode<3, InnerInfo>*>(n1);
          const InnerNode<3, InnerInfo>* in2 =
                dynamic_cast<const InnerNode<3, InnerInfo>*>(n2);
          return *in1 == *in2;
       }
    }

    SmiRecordId saveNode(const BasicNode<3>& n){
       noNodes++;
       if(n.isLeaf()){
         noLeafNodes++;
       }
       assert(n.entryCount()>0); // never save an empty node
       unsigned int size = recordLength;
       char buffer[size];
       memset(buffer,0,size);
       char leaf = n.isLeaf()?1:0;
       unsigned int offset = 0;
       memcpy(buffer , &leaf, sizeof(char));
       offset += sizeof(char);
       n.writeTo(buffer, offset);
       SmiRecordId id = 0;
       SmiRecord record;
       file.AppendRecord(id, record);
       SmiSize os = 0; // offset
       SmiSize rss = record.Write(buffer, size, os);
       assert(rss == size);
       record.Finish();
       return id;
    }


    void updateNode(const SmiRecordId id, const BasicNode<3>& n){
       assert(n.entryCount() > 0);
       SmiRecord record;
       file.SelectRecord(id, record, SmiFile::Update);
       // unsigned int size = n.bufferSize()+sizeof(char);
       unsigned int size = recordLength;
       char buffer[size];
       memset(buffer,0,size);
       unsigned int offset = 0;
       char isLeaf = n.isLeaf()?1:0;
       memcpy(buffer,&isLeaf,sizeof(char));
       offset += sizeof(char);
       n.writeTo(buffer, offset);


       SmiSize os = 0; // offset
       SmiSize rss = record.Write(buffer, size, os);
       assert(rss == size);
       if(record.Size()>size){
           record.Truncate(size);
       }
       record.Finish();
    }


    BasicNode<3>* readNode(const SmiRecordId id) {
       SmiRecord record;
       file.SelectRecord(id, record);
       SmiSize size = record.Size();
       assert(size>1);

       char buffer[size];
       SmiSize bs = record.Read(buffer, size, 0); // read record completely
       assert(bs == size);
       unsigned int  offset = 0;
       char leaf;
       memcpy(&leaf, buffer, sizeof(char));
       offset += sizeof(char);
       BasicNode<3>* res(0);
       if(leaf==1){
          res =  createLeafNodeFrom(buffer, offset);
       } else {
          res =  createInnerNodeFrom(buffer, offset);
       }
       assert(offset<=size);
       return res;
    }

    TBLeafNode<3, TBLeafInfo>* createLeafNodeFrom(const char* buffer,
                                                unsigned int& offset){
         TBLeafNode<3, TBLeafInfo>* res =
                     new TBLeafNode<3, TBLeafInfo>(leafMin, leafMax, 0);
         res->readFrom(buffer, offset);
         return res;
    }

   InnerNode<3, InnerInfo>* createInnerNodeFrom(const char* buffer,
                                                unsigned int& offset){
       InnerNode<3, InnerInfo>* res =
                    new InnerNode<3, InnerInfo>(innerMin, innerMax);
       res->readFrom(buffer, offset);
       return res;
   }



    bool searchNode(const SmiRecordId id, vector<pathEntry<3> >& path,
                    const Entry<3, TBLeafInfo>& li, const int trjid )  {

        BasicNode<3>*  bn = readNode(id);
        assert(bn->entryCount()>0);


        if(bn->isLeaf()){
           TBLeafNode<3, TBLeafInfo>* leaf =
                       dynamic_cast<TBLeafNode<3, TBLeafInfo>*> (bn);
           if(leaf->getTrjId()==trjid && leaf->getNext()==0){
                  path.push_back(pathEntry<3>(id,leaf, -1));
                  return true;
           } else {
             delete leaf;
             return false;
           }
        } else {  // an inner node
           InnerNode<3, InnerInfo>* node =
                        dynamic_cast<InnerNode<3, InnerInfo>* >(bn);
           const Entry<3, InnerInfo>* entry;
           uint16_t c = node->entryCount();
           for(uint16_t i=0; i < c; i++){

              entry = node->getEntry(i);
              if(entry->getBox().Intersects(li.getBox())){
                 path.push_back(pathEntry<3>(id, node,i));
                 if(searchNode(entry->getInfo().getPointer(), path, li, trjid)){
                    return true;
                 } else {
                   path.pop_back();
                 }
              }
           }
           delete node;
           return false;
        }
    }



    void updatePath(const vector<pathEntry<3> >& path, const Rectangle<3>& b ) {
       // a new entry was inserted at the end of the path.
       // we have to update the bounding boxes of all nodes in a path
       for(int i=path.size()-2; i>=0;i--){
          InnerNode<3, InnerInfo>* node =
                       dynamic_cast<InnerNode<3, InnerInfo>* >(path[i].node);
          int pos = path[i].pos;
          Rectangle<3> r = node->getEntry(pos)->getBox();
          if(r.Contains(b)){
             return;
          } else {
             node->addBoxAt(pos, b);
             updateNode(path[i].id, *node);
          }
       }
    }


/*
~InsertLeaf~

This function embedds the leaf with given id and box into the tree structure.

*/
    SmiRecordId insertLeaf(const SmiRecordId& id, const Rectangle<3>& rect){
       pair<SmiRecordId, SmiRecordId> res =  insertLeafRec(rootId, id, rect, 0);
       if(res.second==0){
          return res.first;
       } else {
          BasicNode<3>* s1 = readNode(res.first);
          BasicNode<3>* s2 = readNode(res.second);
          Entry<3, InnerInfo> e1(s1->getBox(), res.first);
          Entry<3, InnerInfo> e2(s2->getBox(), res.second);
          InnerNode<3, InnerInfo> n(innerMin, innerMax);
          n.insert(e1);
          n.insert(e2);
          delete s1;
          delete s2;
          return saveNode(n);
       }
    }


    pair<SmiRecordId, SmiRecordId>
    insertLeafRec(const SmiRecordId rootId,
                  const SmiRecordId& id,
                  const Rectangle<3>& rect,
                  const int level){
      BasicNode<3>* root = readNode(rootId);
      if(root->isLeaf()){
         pair<SmiRecordId, SmiRecordId> res(rootId, id);
         delete root;
         return res;
      } else {
         InnerNode<3, InnerInfo>* iroot =
                      dynamic_cast<InnerNode<3, InnerInfo>*>(root);
         int sonIndex = iroot->selectBestNode(rect);
         SmiRecordId sonId = iroot->getEntry(sonIndex)->getInfo().getPointer();
         pair<SmiRecordId, SmiRecordId> sonRes =
               insertLeafRec(sonId, id, rect, level+1);
         if(sonRes.second == 0 ){ // no split required
            Rectangle<3>  b(root->getBox());
            if(!b.Contains(rect)){ // bounding box update required
               b = b.Union(rect);
               Entry<3, InnerInfo> newSon(b, sonId);
               iroot->updateEntry(sonIndex, newSon);
               updateNode(rootId, *iroot);
            }
            delete iroot;
            sonRes.first = rootId;
            return sonRes;
         } else { // son was split into two nodes
            // update first son
            BasicNode<3>* firstSon = readNode(sonRes.first);
            Entry<3, InnerInfo> newSon1(firstSon->getBox(), sonRes.first);
            BasicNode<3>* secondSon = readNode(sonRes.second);
            Entry<3, InnerInfo> newSon2(secondSon->getBox(), sonRes.second);
            delete firstSon;
            delete secondSon;
            iroot->updateEntry(sonIndex, newSon1);
            if(!iroot->insert(newSon2)){ // root has space enough
              updateNode(rootId, *iroot);
              sonRes.first = rootId;
              sonRes.second = 0;
              delete iroot;
              return sonRes;
            } else {
              QNodeSplitter<3, InnerInfo> ns;
              pair<Node<3, InnerInfo>*, Node<3, InnerInfo>* > nodes =
                    ns.splitNode(*iroot);
              delete iroot;
              updateNode(rootId, *(nodes.first));
              sonRes.first = rootId;
              sonRes.second = saveNode(*(nodes.second));
              delete nodes.first;
              delete nodes.second;
              return sonRes;
            }
         }
      }

    }

};

/*

9 classes for a tree traversal


9.1 Iterator element


Using an iterator implementation, instances of obkjects of this class
will be returned as return values.

*/



template<int Dim>
class IteratorElement{
  public:
/*
~Constructors~


*/
    IteratorElement(): ownId(0),parentId(0), node(0), level(0){}

    IteratorElement(SmiRecordId ownId1,
                    SmiRecordId parentId1,
                    BasicNode<Dim>* node1,
                    int level1): ownId(ownId1), parentId(parentId1),
                                 node(node1), level(level1) {}

    IteratorElement( const IteratorElement<Dim>& s):
         ownId(s.ownId), parentId(s.parentId),
         node(s.node), level(s.level) {}
/*
Assignment operator

*/
    IteratorElement<Dim>& operator=(const IteratorElement<Dim>& s){
       ownId = s.ownId;
       parentId = s.parentId;
       node = s.node;
       level = s.level;
       return *this;
    }

/*
~Destructor~

*/
    ~IteratorElement(){}

     void deleteNode(){
        if(node){
          delete node;
          node = 0;
        }
     }

/*

~Getter~

*/
     SmiRecordId getOwnId() const{ return ownId; }
     SmiRecordId getParentId() const {return parentId;}
     BasicNode<Dim>* getNode() const { return node; }
     int getLevel() const { return level; }

/*
~print~

*/
     ostream& print(ostream& os){
        return os << "ownId = " << ownId << ", parentId = "
                  << parentId << ", level = " << level;
     }


  private:
     SmiRecordId ownId;    // id of the node
     SmiRecordId parentId; // parent id, 0 if root
     BasicNode<Dim>* node; // the node
     int  level;           // level (0 = root)
};



/*

Class PathElement

For internal use within the DepthSearch class.

*/
template<int Dim>
class PathElement: public IteratorElement<Dim>{
  public:
    PathElement(SmiRecordId ownId,
                SmiRecordId parentId,
                BasicNode<Dim>* node,
                int level): IteratorElement<Dim>(ownId, parentId, node, level),
                            pos(0), used(false) {}

   PathElement(const PathElement<Dim>& p): IteratorElement<Dim>(p),
                                           pos(p.pos), used(p.used){}

   PathElement<Dim>& operator=(const PathElement<Dim>& s){
       IteratorElement<Dim>::operator=(s);
       pos = s.pos;
       used = s.used;
       return *this;
   }

   ostream& print(ostream& os){
     return IteratorElement<Dim>::print(os) << " , pos = "
                                            << pos << ", used = " << used;
   }

   int pos;
   bool used;
};


/*
Class NoPruner

Prunes no entry.

*/

template<class InnerNode>
class NoPruner{
 public:
   inline bool  prune(const InnerNode& node, const int pos) const{
       return false;
   }
};

/*
Class IntersectsPruner

Prunes all entries whose bounding box does not intersect the
rectangle given in construction of this class.

*/

template<unsigned Dim, class InnerNode>
class IntersectsPruner{
  public:
   IntersectsPruner(const Rectangle<Dim> rect1): rect(rect1){}

   inline bool prune(const InnerNode& node, const int pos) const{
      return !rect.Intersects(node.getEntry(pos)->getBox());
   }

   Rectangle<Dim> getBox() const{
     return rect;
   }

 private:
    Rectangle<Dim> rect;

};


/*
Class DepthSearc


This is an iterator class which performs a depth search on a tree.
By setting Select and Prune classes this class can be used in a
flexible way.


*/

template<class Tree, class InnerNode, int Dim, class Select, class Pruner>
class DepthSearch{
  public:
/*
~Constructor~

*/
    DepthSearch(Tree* tree1, const Select& select1, const Pruner pruner1):
         tree(tree1), select(select1), pruner(pruner1){
     SmiRecordId rid = tree->getRootId();
     if(rid){
       init(rid);
     }
    }

/*
~Next~

Returns the next node.

*/
    bool next(IteratorElement<Dim>& result){
      if(path.empty()){
        return false;
      }
      return  next1(result);
    }


/*
~finish~

Destroys local variables.

*/
    void finish(){
       while(!path.empty()){
          PathElement<Dim> p = path.top();
          delete p.getNode();
          path.pop();
       }
    }
    stack<PathElement<Dim> > getPath()
    {
      return path;
    }

  private:
    Tree* tree;
    stack<PathElement<Dim> > path;
    Select select;
    Pruner pruner;

    /* puts the root of the tree to the path */
    void init(SmiRecordId rid){
       BasicNode<Dim>* root = tree->getNode(rid);
       PathElement<Dim> p(rid, 0, root , 0);
       path.push(p);
    }

    /* gets the next objects */
    bool next1(IteratorElement<Dim>& result){

    while(!path.empty()){

       PathElement<Dim> top = path.top();

       if(!(top.used) ){
          path.pop();
          top.used = true;
          path.push(top);
          if(select(top.getNode())){
             IteratorElement<Dim> r(top.getOwnId(), top.getParentId(),
                                top.getNode()->clone(),top.getLevel());
             result = r;
             return true;
          }
       }

       if(top.getNode()->isLeaf()){ // used leaf found
          path.pop();
          delete top.getNode();
          if(path.empty()){
            return false;
          } else {
            top = path.top();
            top.pos++;
            path.pop();
            path.push(top);
          }
       }

       // p contains an inner node
       while(top.pos == top.getNode()->entryCount()){ // no more sons available
          path.pop();
          delete top.getNode();
          if(path.empty()){
             return false;
          } else {
             top = path.top();
             top.pos++;
             path.pop();
             path.push(top);
          }
       }

       InnerNode* in = dynamic_cast<InnerNode*>(top.getNode());
       if(!pruner.prune(*in, top.pos)){
          SmiRecordId sonId = in->getEntry(top.pos)->getInfo().getPointer();
          BasicNode<Dim>* son = tree->getNode(sonId);
          PathElement<Dim> p(sonId, top.getOwnId(), son , top.getLevel()+1);
          path.push(p);
       } else {
          path.pop();
          top.pos++;
          path.push(top);
       }
     }
     return false;

    }
};



} // end of namespace tbtree


#endif


