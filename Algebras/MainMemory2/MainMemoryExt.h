/*

----
This file is part of SECONDO.

Copyright (C) 2016, 
University in Hagen, 
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

*/



/*

1 Defines, includes, and constants

*/


#ifndef MAINMEMORY2_H
#define MAINMEMORY2_H

#include "Mem.h"

#include <map>
#include <vector>
#include <string>
#include <limits>
#include <algorithm>

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "OrderedRelationAlgebra.h"
#include "MMRTree.h"
#include "MMMTree.h"
#include "AvlTree.h"
#include "Point.h"
#include "GraphAlgebra.h"
#include "ttree.h"
#include "graph.h"
#include "PointerWrapper.h"
#include "MPointer.h"



namespace mm2algebra{


class MemCatalog;
class Memory;
class MemoryObject;
class MemoryRelObject;
class MemoryAttributeObject;
template<int dim> class MemoryRtreeObject;
class KeyComparator;

class MemoryTTreeObject;
class MemoryORelObject;
class MemoryGraphObject;
class TupleComp;
class AttrComp;


class AttrIdPair{
 public:
  AttrIdPair(Attribute* _attr, size_t _tid): attr(_attr->Copy()),
                                          tid(_tid) {}

  AttrIdPair(): attr(0),tid(0){}

  AttrIdPair(const AttrIdPair& rhs){
    tid = rhs.tid;
    attr = rhs.attr?rhs.attr->Copy():0; 
  }

  ~AttrIdPair(){
    if(attr){
      attr->DeleteIfAllowed();
    }
  }

  AttrIdPair& operator=(const AttrIdPair& rhs){
    if(attr){
      attr->DeleteIfAllowed();
    }
    tid = rhs.tid;
    attr = rhs.attr?rhs.attr->Copy():0; 
    return *this;
  }

  inline const Attribute* getAttr() const{
    return attr;
  }

  inline const size_t getTid() const{
    return tid;
  }

  private:
    Attribute* attr;
    size_t tid;
};



typedef PointerWrap<Tuple> TupleWrap;



class MemCatalog {

    public:
        MemCatalog (){
          memSizeTotal=256;  //main memory size in MB
          usedMemSize=0;     //used main memory size in B
        };
        ~MemCatalog();

        void setMemSizeTotal(size_t size);

        size_t getMemSizeTotal();

        unsigned long getUsedMemSize(); //in Byte

        void addToUsedMemSize(int i){
            usedMemSize = usedMemSize + i;
        }

        unsigned long getAvailableMemSize();  //in Byte

        std::map<std::string,MemoryObject*>* getMemContent();

        bool insert (const std::string& name, MemoryObject* obj);

        bool deleteObject (const std::string& name, const bool erase=true);

        void clear ();

        bool isMMObject(const std::string& objectName);

        //*Precondition*: "isMMObject( objectName ) == true"
        MemoryObject* getMMObject(const std::string& objectName);

        ListExpr getMMObjectTypeExpr(const std::string& oN);

        bool isAccessible(const std::string& name);



    private:
        unsigned long usedMemSize;  //in Byte
        size_t memSizeTotal; //in MB
        std::map<std::string,MemoryObject*> memContents;
};


class MemoryObject {
    public:
    MemoryObject(){
        flob = false;
        database="";
        memSize=0;
        objectTypeExpr="";
    }

    MemoryObject(bool _flob, const std::string& _database,
                    const std::string& _type) : memSize(0), 
                    objectTypeExpr(_type), flob(_flob),
                    database(_database) {}

    virtual ~MemoryObject();

    unsigned long getMemSize ();

    std::string getObjectTypeExpr();
    void setObjectTypeExpr(std::string _oTE);
    std::string getDatabase();
    bool hasflob();
    ListExpr getType() const{
      ListExpr res;
      if(!nl->ReadFromString(objectTypeExpr,res)){
        return nl->TheEmptyList();
      } else {
        return res;
      }
    }


    protected:
        unsigned long memSize;      // object size in main memory in byte
        std::string objectTypeExpr;  // the tuple description for relations,
                                     // or the attribute description

        bool flob;
        std::string database;

};



class MemoryRelObject : public MemoryObject {

    public:

        MemoryRelObject();

        MemoryRelObject(std::vector<Tuple*>* _mmrel,
                    unsigned long _memSize, 
                    std::string _objectTypeExpr, bool _flob,
                    std::string _database);

        MemoryRelObject (std::string _objectTypeExpr);

        ~MemoryRelObject();

        std::vector<Tuple*>* getmmrel();

        void addTuple(Tuple* tup);
        
        Tuple* getTuple(TupleId id);

        bool relToVector(GenericRelation* r, ListExpr le,
                        std::string _database, bool _flob);
        
        bool mmrelToVector(std::vector<Tuple*>* r, ListExpr le,
                        std::string _database, bool _flob);

        bool tupleStreamToRel (Word arg, ListExpr le,
                        std::string _database, bool _flob);

        ListExpr toListExpr();

        static Word In( const ListExpr typeInfo, const ListExpr instance,
                        const int errorPos, ListExpr& errorInfo,
                        bool& correct );

        static ListExpr Out( ListExpr typeInfo, Word value );

        static bool KindCheck( ListExpr type, ListExpr& errorInfo );

        static Word create(const ListExpr typeInfo);

        static bool Save(SmiRecord& valueRecord, size_t& offset,
                            const ListExpr typeInfo, Word& value);

        static bool Open (SmiRecord& valueRecord, size_t& offset,
                            const ListExpr typeInfo, Word& value);

        static void Close (const ListExpr typeInfo, Word& w);

        static Word Clone (const ListExpr typeInfo, const Word& w);

        static void* Cast (void* addr);

        static int SizeOfObj();

        static void Delete(const ListExpr typeInfo, Word& w);

        static ListExpr Property();

        static const std::string BasicType() { return "memoryRelObject"; }

        static const bool checkType(const ListExpr type);

        inline size_t getSize()const{
           return mmrel?mmrel->size():0;
        }

    private:
        std::vector<Tuple*>* mmrel;

};



class TupleComp {
  
  public:
     static bool smaller(const TupleWrap& aw, 
                         const TupleWrap& bw, 
                         const std::vector<int>* attrPos) {
        
       bool smaller;
       Tuple* a = aw.getPointer();
       Tuple* b = bw.getPointer();
       if( (a==0) && (b == 0)){
         return false;
       }
       if( (a==0) || (b==0)){
         return a==0;
       }

       for(size_t i=0; i<attrPos->size(); i++) {
          int cmp = ((Attribute*)a->GetAttribute(attrPos->at(i)-1))->Compare(
                    ((Attribute*)b->GetAttribute(attrPos->at(i)-1)));
          if(cmp<0)
            return true;
          else if(cmp>0)
            return false;
          else
            smaller = false;
        }        
        return smaller;
     }
     
     static bool equal(const TupleWrap& aw, 
                       const TupleWrap& bw, 
                       const std::vector<int>* attrPos) {
       bool equal = false;
       Tuple* a = aw.getPointer();
       Tuple* b = bw.getPointer();
       if( (a==0) && (b == 0)){
         return true;
       }
       if( (a==0) || (b==0)){
         return false;
       }
       for(size_t i=0; i<attrPos->size(); i++) {
          int cmp = ((Attribute*)a->GetAttribute(attrPos->at(i)-1))->Compare(
                    ((Attribute*)b->GetAttribute(attrPos->at(i)-1)));
          if(cmp==0)
            equal = true;
          else 
            return false;
        }
        return equal;
     }
     
     static bool greater(const TupleWrap& aw, 
                         const TupleWrap& bw, 
                         const std::vector<int>* attrPos) {
       bool greater;
       Tuple* a = aw.getPointer();
       Tuple* b = bw.getPointer();
       if( (a==0) && (b == 0)){
         return false;
       }
       if( (a==0) || (b==0)){
         return b==0;
       }
       for(size_t i=0; i<attrPos->size(); i++) {
          int cmp = ((Attribute*)a->GetAttribute(attrPos->at(i)-1))->Compare(
                    ((Attribute*)b->GetAttribute(attrPos->at(i)-1)));
          if(cmp>0)
            return true;
          else if(cmp<0)
            return false;
          else
            greater = false;
        }
        return greater;
     }
     

     static bool smaller(const Tuple* a, 
                         const Tuple* b, 
                         std::vector<int>* attrPos) {
        
       bool smaller;
       if( (a==0) && (b == 0)){
         return false;
       }
       if( (a==0) || (b==0)){
         return a==0;
       }

       for(size_t i=0; i<attrPos->size(); i++) {
          int cmp = ((Attribute*)a->GetAttribute(attrPos->at(i)-1))->Compare(
                    ((Attribute*)b->GetAttribute(attrPos->at(i)-1)));
          if(cmp<0)
            return true;
          else if(cmp>0)
            return false;
          else
            smaller = false;
        }        
        return smaller;
     }
     
     static bool equal(const Tuple* a, 
                       const Tuple* b, 
                       std::vector<int>* attrPos) {
       bool equal = false;
       if( (a==0) && (b == 0)){
         return true;
       }
       if( (a==0) || (b==0)){
         return false;
       }
       for(size_t i=0; i<attrPos->size(); i++) {
          int cmp = ((Attribute*)a->GetAttribute(attrPos->at(i)-1))->Compare(
                    ((Attribute*)b->GetAttribute(attrPos->at(i)-1)));
          if(cmp==0)
            equal = true;
          else 
            return false;
        }
        return equal;
     }
     
     static bool greater(const Tuple* a, 
                         const Tuple* b, 
                         std::vector<int>* attrPos) {
       bool greater;
       if( (a==0) && (b == 0)){
         return false;
       }
       if( (a==0) || (b==0)){
         return b==0;
       }
       for(size_t i=0; i<attrPos->size(); i++) {
          int cmp = ((Attribute*)a->GetAttribute(attrPos->at(i)-1))->Compare(
                    ((Attribute*)b->GetAttribute(attrPos->at(i)-1)));
          if(cmp>0)
            return true;
          else if(cmp<0)
            return false;
          else
            greater = false;
        }
        return greater;
     }
};



class MemoryORelObject : public MemoryObject {
        public:

        MemoryORelObject();

        MemoryORelObject(ttree::TTree<TupleWrap,TupleComp>* _mmorel,
                         std::vector<int>* _pos,
                         unsigned long _memSize, std::string _objectTypeExpr, 
                         bool _flob, std::string _database);

        MemoryORelObject (std::string _objectTypeExpr);

        ~MemoryORelObject();

        ttree::TTree<TupleWrap,TupleComp>* getmmorel();
        
        std::vector<int>* getAttrPos();
        
        void setAttrPos(int attrPos, bool keep);
        
        void setAttrPos();
        
        void addTuple(Tuple* tup);

        bool relToTree(GenericRelation* r, ListExpr le,
                       std::string _database, bool _flob);

        bool tupleStreamToORel(Word arg, ListExpr le, ListExpr type,
                               std::string _database, bool _flob);

        ListExpr toListExpr();

        static Word In( const ListExpr typeInfo, const ListExpr instance,
                        const int errorPos, ListExpr& errorInfo,
                        bool& correct );

        static ListExpr Out( ListExpr typeInfo, Word value );

        static bool KindCheck( ListExpr type, ListExpr& errorInfo );

        static Word create(const ListExpr typeInfo);

        static bool Save(SmiRecord& valueRecord, size_t& offset,
                            const ListExpr typeInfo, Word& value);

        static bool Open (SmiRecord& valueRecord, size_t& offset,
                            const ListExpr typeInfo, Word& value);

        static void Close (const ListExpr typeInfo, Word& w);

        static Word Clone (const ListExpr typeInfo, const Word& w);

        static void* Cast (void* addr);

        static int SizeOfObj();

        static void Delete(const ListExpr typeInfo, Word& w);

        static ListExpr Property();

        static const std::string BasicType() { return "memoryORelObject"; }

        static const bool checkType(const ListExpr type);
        
        std::ostream& print(std::ostream& out) const;

        size_t getSize()const{
          return mmorel?mmorel->noEntries():0;
        }
        
        
    private:
        ttree::TTree<TupleWrap,TupleComp>* mmorel;
        std::vector<int>* pos;  
};

class QueueEntry {
  

  public:
  
  QueueEntry(int _nodeNumber, int _prev, double _dist, 
             double _priority)
    : nodeNumber(_nodeNumber),prev(_prev),dist(_dist),
      priority(_priority), visited(false), norefs(1) {
    
  }
    
  ~QueueEntry() {
    assert(norefs==0);
  }
  
 
  void DeleteIfAllowed(){
    assert(norefs>0);
    norefs--;
    if(norefs==0){
      delete this;
    }
  }

  void IncReference(){
    norefs++;
  }
 
  void print(std::ostream& out) {
    out << std::endl << "QUEUEENTRY:" << std::endl;
    
    out << "NodeNumber: " << this->nodeNumber;
    out << " previous: " << this->prev;
    out << " Distance: " << this->dist;
    out << " Priority: " << this->priority;
    out << " visited: " << this->visited << std::endl;
    
  }
 
  int nodeNumber;
  int prev;
  double dist;
  double priority;
  bool visited;
  size_t norefs;
};


typedef PointerWrap<QueueEntry> QueueEntryWrap;




class EntryComp {
  public:
    
    bool operator() (const QueueEntryWrap& lhs, 
                     const QueueEntryWrap& rhs) const {
      if(lhs.getPointer()->priority > rhs.getPointer()->priority) {
        return true;
      }
      return false;
    }
    
     static bool smaller(const QueueEntryWrap& a, 
                         const QueueEntryWrap& b, 
                         const std::vector<int>* pos) {
       if(a.getPointer()->nodeNumber < b.getPointer()->nodeNumber)
         return true;
       return false;
     }
     
     static bool greater(const QueueEntryWrap& a, 
                         const QueueEntryWrap& b, 
                         const std::vector<int>* pos) {
        if(a.getPointer()->nodeNumber > b.getPointer()->nodeNumber)
         return true;
       return false;
     }
     
     static bool equal(const QueueEntryWrap& a, 
                       const QueueEntryWrap& b, 
                       const std::vector<int>* pos) {
        if(a.getPointer()->nodeNumber == b.getPointer()->nodeNumber)
         return true;
       return false;
     }
};

struct Queue {
  
  Queue() {
    queue = new std::vector<QueueEntryWrap>();    
    std::make_heap(queue->begin(),queue->end(),EntryComp());
  }
  
  ~Queue() {
    queue->clear();
    delete queue;
  }
  
  bool empty() {
    return queue->empty();
  }
  
  size_t size() {
    return queue->size();
  }
  
  QueueEntryWrap& top() {
    return queue->front();
  }
  
  void push(QueueEntryWrap& entry) {
    queue->push_back(entry);
    std::push_heap(queue->begin(),queue->end(),EntryComp());
  }
  
  void pop() {
    std::pop_heap(queue->begin(),queue->end(),EntryComp());
    queue->pop_back();
  }
  
  void print(std::ostream& out) {
    out << std::endl << "QUEUE:" << std::endl;
    for(size_t i=0; i<queue->size(); i++) {
      out << "NodeNumber: " << queue->at(i).getPointer()->nodeNumber;
      out << " previous: " << queue->at(i).getPointer()->prev;
      out << " Distance: " << queue->at(i).getPointer()->dist;
      out << " Priority: " << queue->at(i).getPointer()->priority << std::endl;
    }
  }
  
private:
  std::vector<QueueEntryWrap>* queue;
};




class MemoryAttributeObject : public MemoryObject {

    public:
        MemoryAttributeObject();
        MemoryAttributeObject(Attribute* _attr,
                unsigned long _memSize, std::string _objectTypeExpr, bool _flob,
                std::string _database);
        ~MemoryAttributeObject();

        Attribute* getAttributeObject();

        bool attrToMM(Attribute* attr,
            ListExpr le, std::string database, bool flob);

    private:
         Attribute* attributeObject;

};


template <int dim>
class MemoryRtreeObject : public MemoryObject {

    public:
        MemoryRtreeObject(){};
        MemoryRtreeObject(mmrtree::RtreeT<dim, size_t>* _rtree,
                        size_t _memSize, 
                        std::string _objectTypeExpr,
                        std::string _database){


                        rtree = _rtree;
                        memSize = _memSize;
                        objectTypeExpr =_objectTypeExpr;
                        flob = true;
                        database = _database;
                      };
        ~MemoryRtreeObject(){
            if (rtree){
                delete rtree;
            }
        };

        mmrtree::RtreeT<dim, size_t>* getrtree(){
            return rtree;
        };

    private:
         mmrtree::RtreeT<dim, size_t>* rtree;

};

template <class T, class DistComp> 
class MemoryMtreeObject : public MemoryObject {

    public:
        MemoryMtreeObject(MMMTree<std::pair<T, TupleId>, DistComp>* _mtree,
                        size_t _memSize, 
                        const std::string& _objectTypeExpr, 
                        bool _flob,
                        const std::string& _database){
                        mtree = _mtree;
                        memSize = _memSize;
                        objectTypeExpr =_objectTypeExpr;
                        flob = _flob;
                        database = _database;
                        };
        ~MemoryMtreeObject(){
            if (mtree){
                delete mtree;
            }
        };

        MMMTree<std::pair<T, TupleId>, DistComp>* getmtree(){
            return mtree;
        };

        static std::string BasicType(){
           return "mtree";
        }

        static bool checkType( ListExpr list){
           if(!nl->HasLength(list,2)){
             return false;
           }
           if(!listutils::isSymbol(nl->First(list),BasicType())){
              return false;
           }
           return T::checkType(nl->Second(list));
        }


    private:
         MMMTree< std::pair<T, TupleId>, DistComp>* mtree;
};

class AttrComp{

    public:
        static bool smaller(const AttrIdPair& p1, 
                            const AttrIdPair& p2, 
                            const std::vector<int>* pos) {

            int res  = p1.getAttr()->Compare(p2.getAttr());
            if(res < 0) {
              return true;
            }
            if(res > 0) {
              return false;
            }
            return p1.getTid() < p2.getTid();
        }


        static bool equal(const AttrIdPair& p1, 
                          const AttrIdPair& p2, 
                          const std::vector<int>* pos) {
          
          int res = p1.getAttr()->Compare(p2.getAttr());
          return (res == 0) && (p1.getTid() == p2.getTid());
        }


        static bool greater(const AttrIdPair& p1, 
                            const AttrIdPair& p2, 
                            const std::vector<int>* pos) {
          
         int res = p1.getAttr()->Compare(p2.getAttr());
         if(res > 0) {
           return true;
         }
         if(res < 0) {
            return false;
         }
         return p1.getTid() > p2.getTid();
     }

};


typedef ttree::TTree<AttrIdPair,AttrComp> memttree;
class MemoryTTreeObject : public MemoryObject {

    public:
        MemoryTTreeObject(memttree* _ttree,
                          size_t _memSize, 
                          const std::string& _objectTypeExpr, 
                          bool _flob,
                          const std::string& _database) {
          
        
            ttree = _ttree;
            memSize = _memSize;
            objectTypeExpr = _objectTypeExpr;
            flob = _flob;
            database = _database;    
          
        };
        
         
        ~MemoryTTreeObject() {
          if(ttree) {
            delete ttree;
          }
        }
           
        memttree* gettree() {
          return ttree;
        }

        static std::string BasicType() { 
          return "ttree"; 
        }
        
        static bool checkType(ListExpr type){
          return nl->HasLength(type,2) && 
                 listutils::isSymbol(nl->First(type),BasicType());
        }


    private:
         memttree* ttree;
};


class MemoryGraphObject : public MemoryObject {

    public:
        
        MemoryGraphObject();
        
        MemoryGraphObject(graph::Graph* _graph, int _source, 
                          int _target, size_t _memSize, 
                          const std::string& _objectTypeExpr, 
                          bool _flob,
                          const std::string& _database);
          
  MemoryGraphObject(std::string _objectTypeExpr);
  
        ~MemoryGraphObject();

        graph::Graph* getgraph();
        
        bool relToGraph(GenericRelation* r, 
                        ListExpr le,
      std::string _database, 
                        bool _flob);
       
        void addTuple(Tuple* tup, double cost, double dist);

        bool tupleStreamToGraph (Word arg, ListExpr le,
                        std::string _database, bool _flob);

        static std::string BasicType()  { 
          return "graph"; 
        }

        static const bool checkType(const ListExpr type);
        
    private:
         graph::Graph* memgraph;
         int source;
         int target;
//          int p1;
//          int p2;
//          double cost;
};




}

std::ostream& operator<<(std::ostream& o, const mm2algebra::AttrIdPair& p);

namespace mm2algebra{

class KeyComparator{

    public:

        static bool smaller(const AttrIdPair& o1,
                            const AttrIdPair& o2){
            const Attribute* thisAttr = o1.getAttr();
            const Attribute* rhs = o2.getAttr();
            int res  = thisAttr->Compare(rhs);
            if (res < 0 ) {
                return true;
            }
            if( res > 0){
              return false;
            }
            bool r =  o1.getTid() < o2.getTid();
            return r;
        }


        static bool equal(const AttrIdPair& o1,
                          const AttrIdPair& o2){

          const Attribute* thisAttr = o1.getAttr();
          const Attribute* rhs = o2.getAttr();
          int res = thisAttr->Compare(rhs);
          bool r =  (res == 0) && (o1.getTid() == o2.getTid());
          return r;   

        }


        static bool greater(const AttrIdPair& o1,
                            const AttrIdPair& o2){

         const Attribute* thisAttr = o1.getAttr();
         const Attribute* rhs = o2.getAttr();
         int res = thisAttr->Compare(rhs);
         if(res > 0){
           return true;
         }
         if(res < 0){
            return false;
         }
         bool r = o1.getTid() > o2.getTid();
         return r;
     }
};


typedef avltree::AVLTree<AttrIdPair,KeyComparator> memAVLtree;
typedef memAVLtree::iterator avlIterator;

class MemoryAVLObject : public MemoryObject {

    public:
        MemoryAVLObject();
        MemoryAVLObject( memAVLtree* tree, size_t _memSize, 
                         const std::string& _objectTypeExpr, 
                         bool _flob, 
                         const std::string& _database );
      
         ~MemoryAVLObject();

        memAVLtree* getAVLtree();
        
        inline memAVLtree* gettree(){
           return tree;
        }

        static std::string BasicType(){ return "avltree"; }

        static bool checkType(ListExpr type){
            return    nl->HasLength(type,2) 
                   && listutils::isSymbol(nl->First(type),BasicType());
        }


    private:
        memAVLtree* tree;
};


bool dijkstra(graph::Graph* graph, Word& arg,
               graph::Vertex* start, graph::Vertex* dest);

bool getMemType(ListExpr type, ListExpr value,
                   ListExpr & result, std::string& error,
                   bool allowMPointer=false);

template<class T>
MemoryGraphObject* getMemGraph(T* aN);

template<>
MemoryGraphObject* getMemGraph(MPointer* a);

} //ende namespace mm2algebra

#endif
