/*

1 Defines, includes, and constants

*/


#ifndef MAINMEMORY_H
#define MAINMEMORY_H

#include <map>
#include <vector>
#include <string>
#include <limits>

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "MMRTree.h"
#include "MMMTree.h"
#include "AvlTree.h"
#include "Point.h"



namespace mmalgebra{


class MemCatalog;
class Memory;
class MemoryObject;
class MemoryRelObject;
class MemoryAttributeObject;
template<int dim> class MemoryRtreeObject;
class KeyComparator;



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
        bool hasflob(); // returns true if either no flob attribute
                        // is contained or the flobs are in main 
                        // memory


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



} //ende namespace mmalgebra

#endif
