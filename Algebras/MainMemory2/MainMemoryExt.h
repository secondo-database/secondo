/*

1 Defines, includes, and constants

*/


#ifndef MAINMEMORY2_H
#define MAINMEMORY2_H

#include <map>
#include <vector>
#include <string>
#include <limits>
#include <algorithm>

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "MMRTree.h"
#include "MMMTree.h"
#include "AvlTree.h"
#include "Point.h"
#include "TTree.h"
#include "Graph.h"
#include "GraphAlgebra.h"


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
class Comparator;
template <class T> class AttComparator;


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
        virtual ~MemoryObject();

        unsigned long getMemSize ();

        std::string getObjectTypeExpr();
        void setObjectTypeExpr(std::string _oTE);
        std::string getDatabase();
        bool hasflob();


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

        bool relToVector(GenericRelation* r, ListExpr le,
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

    private:
        std::vector<Tuple*>* mmrel;

};


class Comparator {
  
  public:
     static bool smaller(Tuple* a, Tuple* b, std::vector<int>* attrPos) {
        
       bool smaller/* = true*/;
       for(size_t i=0; i<attrPos->size(); i++) {
          int cmp = ((Attribute*)a->GetAttribute(attrPos->at(i)-1))->Compare(
                    ((Attribute*)b->GetAttribute(attrPos->at(i)-1)));
//           std::cout << "Comparator smaller cmp: " << cmp << std::endl;
//           std::cout << "Comparator smaller a: ";
//           a->GetAttribute(attrPos->at(i)-1)->Print(std::cout);
//           std::cout <<" b: ";
//           b->GetAttribute(attrPos->at(i)-1)->Print(std::cout);
//           std::cout << std::endl;
          if(cmp<0)
            return true;
          else if(cmp>0)
            return false;
          else
            smaller = false;
        }
        
        return smaller;
     }
     
     static bool equal(Tuple* a, const Tuple* b, std::vector<int>* attrPos) {
       //std::cout << "Comparator equal" << std::endl;
       bool equal = false;
       for(size_t i=0; i<attrPos->size(); i++) {
          int cmp = ((Attribute*)a->GetAttribute(attrPos->at(i)-1))->Compare(
                    ((Attribute*)b->GetAttribute(attrPos->at(i)-1)));
//           std::cout << "Comparator equal cmp: " << cmp << std::endl;
//           std::cout << "Comparator equal a: ";
//           a->GetAttribute(attrPos->at(i)-1)->Print(std::cout);
//           std::cout <<" b: ";
//           b->GetAttribute(attrPos->at(i)-1)->Print(std::cout);
//           std::cout << std::endl;
          if(cmp==0)
            equal = true;
          else 
            return false;
        }
        //std::cout << "Comparator" << std::endl;
        return equal;
     }
     
     static bool greater(Tuple* a, const Tuple* b, std::vector<int>* attrPos) {
       //std::cout << "Comparator greater" << std::endl;
       bool greater/* = true*/;
       for(size_t i=0; i<attrPos->size(); i++) {
          int cmp = ((Attribute*)a->GetAttribute(attrPos->at(i)-1))->Compare(
                    ((Attribute*)b->GetAttribute(attrPos->at(i)-1)));
//           std::cout << "Comparator greater cmp: " << cmp << std::endl;
//           std::cout << "Comparator greater a: ";
//           a->GetAttribute(attrPos->at(i)-1)->Print(std::cout);
//           std::cout <<" b: ";
//           b->GetAttribute(attrPos->at(i)-1)->Print(std::cout);
//           std::cout << std::endl;
          if(cmp>0)
            return true;
          else if(cmp<0)
            return false;
          else
            greater = false;
        }
        //std::cout << "Comparator" << std::endl;
        return greater;
     }
};



class MemoryORelObject : public MemoryObject {
        public:

        MemoryORelObject();

        MemoryORelObject(ttree::TTree<Tuple*,Comparator>* _mmorel,
                         std::vector<int>* _pos,
                         unsigned long _memSize, std::string _objectTypeExpr, 
                         bool _flob, std::string _database);

        MemoryORelObject (std::string _objectTypeExpr);

        ~MemoryORelObject();

        ttree::TTree<Tuple*,Comparator>* getmmorel();
        
        std::vector<int>* getAttrPos();
        
        void setAttrPos(int attrPos);
        
        void setAttrPos();
        
        void addTuple(Tuple* tup);

        bool relToTree(GenericRelation* r, ListExpr le,
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

        static const std::string BasicType() { return "memoryORelObject"; }

        static const bool checkType(const ListExpr type);
        
        std::ostream& print(std::ostream& out) const;
        
        
    private:
        ttree::TTree<Tuple*,Comparator>* mmorel;
        std::vector<int>* pos;  
};

struct QueueEntry {
  
  QueueEntry() {}
  
  QueueEntry(int _nodeNumber, int _prev, double _dist, double _priority)
    : nodeNumber(_nodeNumber),prev(_prev),dist(_dist),priority(_priority) {}
    
  ~QueueEntry() {}
  
  
  void print(std::ostream& out) {
    out << std::endl << "QUEUEENTRY:" << std::endl;
    
    out << "NodeNumber: " << this->nodeNumber;
    out << " previous: " << this->prev;
    out << " Distance: " << this->dist;
    out << " Priority: " << this->priority << std::endl;
    
  }
  
  int nodeNumber;
  int prev;
  double dist;
  double priority;
};


class EntryComp {
  public:
    
    bool operator() (const QueueEntry* lhs, const QueueEntry* rhs) const {
      if(lhs->priority > rhs->priority) {
        return true;
      }
      return false;
    }
    
     static bool smaller(QueueEntry* a, QueueEntry* b, std::vector<int>* pos) {
       if(a->nodeNumber < b->nodeNumber)
         return true;
       return false;
     }
     
     static bool greater(QueueEntry* a, QueueEntry* b, std::vector<int>* pos) {
        if(a->nodeNumber > b->nodeNumber)
         return true;
       return false;
     }
     
     static bool equal(QueueEntry* a, QueueEntry* b, std::vector<int>* pos) {
        if(a->nodeNumber == b->nodeNumber)
         return true;
       return false;
     }
};

struct Queue {
  
  
  Queue() {
    queue = new std::vector<QueueEntry*>();    
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
  
  QueueEntry* top() {
    return queue->front();
  }
  
  void push(QueueEntry* entry) {
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
      out << "NodeNumber: " << queue->at(i)->nodeNumber;
      out << " previous: " << queue->at(i)->prev;
      out << " Distance: " << queue->at(i)->dist;
      out << " Priority: " << queue->at(i)->priority << std::endl;
    }
  }
  
private:
  std::vector<QueueEntry*>* queue;
  std::greater<QueueEntry*> comp;
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
                        std::string _objectTypeExpr, bool _flob,
                        std::string _database){


                        rtree = _rtree;
                        memSize = _memSize;
                        objectTypeExpr =_objectTypeExpr;
                        flob = _flob;
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


template <class T> 
class AttComparator{

    public:
        static bool smaller(const T& o1,
                            const T& o2, std::vector<int>* pos){

            Attribute* thisAttr = o1.first;
            Attribute* rhs = o2.first;
            int res  = thisAttr->Compare(rhs);
            if (res < 0 ) {
                return true;
            }
            if( res > 0){
              return false;
            }
            return o1.second < o2.second;
        }


        static bool equal(const T& o1,
                          const T& o2, std::vector<int>* pos) {
          
          Attribute* thisAttr = o1.first;
          Attribute* rhs = o2.first;
          int res = thisAttr->Compare(rhs);
          return (res == 0) && (o1.second == o2.second);
        }


        static bool greater(const T& o1,
                            const T& o2, std::vector<int>* pos) {
          
         Attribute* thisAttr = o1.first;
         Attribute* rhs = o2.first;
         int res = thisAttr->Compare(rhs);
         if(res > 0){
           return true;
         }
         if(res < 0){
            return false;
         }
         return o1.second > o2.second;
     }

};

typedef std::pair<Attribute*, TupleId> ttreePair;
typedef ttree::TTree<ttreePair,AttComparator<ttreePair> > memttree;

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
            std::cout << "~MemoryTTreeObject()" << std::endl;
            ttree->destroy();
            delete ttree;
          }
        }
        
        

        memttree* getttree() {
          return ttree;
        };

        static std::string BasicType() { 
          return "ttree"; 
        }
        
        static bool checkType(ListExpr type){
            return    nl->HasLength(type,2) 
                   && listutils::isSymbol(nl->First(type),BasicType());
        }


    private:
         memttree* ttree;
};

// TODO anpassen
class MemoryGraphObject : public MemoryObject {

    public:
        
        MemoryGraphObject();
        
        MemoryGraphObject(graph::Graph* _graph,
                          size_t _memSize, 
                          const std::string& _objectTypeExpr, 
                          bool _flob,
                          const std::string& _database);
          
        ~MemoryGraphObject();

        graph::Graph* getgraph();
        
        bool relToGraph(MemoryRelObject* r, 
                        std::string _database, 
                        bool _flob);

        
        void addTuple(Tuple* tup);

        
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

        static std::string BasicType()  { 
          return "graph"; 
        }

        static const bool checkType(const ListExpr type) {
          return nl->HasLength(type,2) 
                  && listutils::isSymbol(nl->First(type),BasicType());
        }
        
        int getPosSource() { return posSource; }
        int getPosDest() { return posDest; }
        int getPosP1() { return posP1; }
        int getPosP2() { return posP2; }

    private:
         graph::Graph* graph;
         int posSource;
         int posDest;
         int posP1;
         int posP2;
};


typedef std::pair<Attribute*, size_t> avlPair;
class KeyComparator{

    public:
        static bool smaller(const avlPair& o1,
                            const avlPair& o2){


            Attribute* thisAttr = o1.first;
            Attribute* rhs = o2.first;
            int res  = thisAttr->Compare(rhs);
            if (res < 0 ) {
                return true;
            }
            if( res > 0){
              return false;
            }
            return o1.second < o2.second;
        }


        static bool equal(const avlPair& o1,
                          const avlPair& o2){
          Attribute* thisAttr = o1.first;
          Attribute* rhs = o2.first;
          int res = thisAttr->Compare(rhs);
          return (res == 0) && (o1.second == o2.second);
        }


        static bool greater(const avlPair& o1,
                            const avlPair& o2){
         Attribute* thisAttr = o1.first;
         Attribute* rhs = o2.first;
         int res = thisAttr->Compare(rhs);
         if(res > 0){
           return true;
         }
         if(res < 0){
            return false;
         }
         return o1.second > o2.second;
     }

};


typedef avltree::AVLTree<avlPair,KeyComparator> memAVLtree;
typedef typename memAVLtree::iterator avlIterator;

class MemoryAVLObject : public MemoryObject {

    public:
        MemoryAVLObject();
        MemoryAVLObject( memAVLtree* tree, size_t _memSize, 
                         const std::string& _objectTypeExpr, 
                         bool _flob, 
                         const std::string& _database );
      
         ~MemoryAVLObject();

        memAVLtree* getAVLtree();

        static std::string BasicType(){ return "avltree"; }

        static bool checkType(ListExpr type){
            return    nl->HasLength(type,2) 
                   && listutils::isSymbol(nl->First(type),BasicType());
        }


    private:
        memAVLtree* tree;
};





/*
5.20 Type ~mem~

This type encapsulates just a string. 

*/
class Mem: public Attribute{
  public:
     Mem(){}
     Mem(int i) : Attribute(false) {
        strcpy(value,"");
     }

     Mem(const Mem& src): Attribute(src){
        strcpy(value, src.value);
     }

     Mem& operator=(const Mem& src){
        Attribute::operator=(src);
        strcpy(value, src.value);
        return *this;
     }
     
     ~Mem(){}

      void set(const bool def, const std::string& v){
         SetDefined(def);
         if(def){
            strcpy(value,v.c_str());
         }
      }

      std::string GetValue() const{
        assert(IsDefined());
        return std::string(value);
      }

     
     static const std::string BasicType(){ return "mem"; }

     static const bool checkType(ListExpr type){
        if(!nl->HasLength(type,2)){
           return false;
        }
        if(!listutils::isSymbol(nl->First(type), BasicType())){
          return false;
        }
        // TODO: check whether the second element is a valid type
        return true;
     }  

     inline virtual int NumOfFLOBs() const{
        return 0;
     }
     inline virtual Flob* GetFLOB(const int i){
        assert(false);
        return 0;
     }

     int Compare(const Attribute* arg) const{
        if(!IsDefined() && !arg->IsDefined()){
          return 0;
        }
        if(!IsDefined()){
          return -1;
        }
        if(!arg->IsDefined()){
          return 1;
        }
        return strcmp(value,((Mem*)arg)->value);
     }

     bool Adjacent(const Attribute* arg) const{
        return false;
     }
     size_t Sizeof() const{
        return sizeof(*this);
     }

     size_t HashValue() const{
        if(!IsDefined()){
           return 0;
        }
        size_t m = 5;
        size_t l = std::min(m, strlen(value));
        int sum = 0;
        for(size_t i=0;i<l;i++){
          sum = sum*10 + value[i];
        }
        return sum;
     }

     void CopyFrom(const Attribute* arg){
        *this = *((Mem*) arg);
     }

     Attribute* Clone() const{
       return new Mem(*this);
     }

     


     static ListExpr Property() {
          return gentc::GenProperty( "-> DATA",
                                    "("+ BasicType() + " <subtype> )",
                                    "<string>",
                                    "\"testobj\"");
                                    
     }
     static bool CheckKind(ListExpr type, ListExpr& errorInfo){
        return checkType(type);
     }

     bool ReadFrom(ListExpr le, const ListExpr typeInfo){
        if(listutils::isSymbolUndefined(le)){
          SetDefined(false);
          return true;
        }         
        if(nl->AtomType(le)!=StringType){
           return false;
        }
        set(true, nl->StringValue(le));
        return true; 
     }

     ListExpr ToListExpr( const ListExpr typeInfo){
       if(!IsDefined()){
         return listutils::getUndefined();
       }
       return nl->StringAtom(value);
     } 
     
  private:
     STRING_T value;

};



} //ende namespace mm2algebra

#endif
