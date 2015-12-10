/*

1 Defines, includes, and constants

*/


#ifndef MAINMEMORY_H
#define MAINMEMORY_H


#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include <string>
#include <map>
#include "RelationAlgebra.h"
#include <vector>
#include "MMRTree.h"
#include "AvlTree.h"



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

        unsigned long getAvailabeMemSize();  //in Byte

        std::map<std::string,MemoryObject*>* getMemContent();

        bool insert (const std::string& name, MemoryObject* obj);

        bool deleteObject (const std::string& name);

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




class MemoryRelType {

    public:
    MemoryRelType();
    MemoryRelType(bool _defined, std::string _value);
    ~MemoryRelType();


    bool IsDefined();

    void SetDefined(bool _defined);

    void Set( const bool d, const std::string& v );

    std::string GetValue();

    static Word In( const ListExpr typeInfo, const ListExpr value,
                        const int errorPos, ListExpr& errorInfo,
                        bool& correct );

    static ListExpr Out( ListExpr typeInfo, Word value );

    static bool KindCheck( ListExpr type, ListExpr& errorInfo );

    static Word create(const ListExpr typeInfo);

    static void Close (const ListExpr typeInfo, Word& w);

    static Word Clone (const ListExpr typeInfo, const Word& w);

    static void* Cast (void* addr);

    static int SizeOfObj();

    static void Delete(const ListExpr typeInfo, Word& w);

    static const std::string BasicType();

    static const bool checkType(const ListExpr type);


    static ListExpr Property();


    private:
    std::string value;
    bool defined;


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



class MemoryAVLObject : public MemoryObject {

    public:
        MemoryAVLObject();
        MemoryAVLObject( avltree::AVLTree< std::pair<Attribute*,size_t>,
            KeyComparator >* tree, size_t _memSize, std::string _objectTypeExpr,
            std::string _keyType, bool _flob, std::string _database );
        ~MemoryAVLObject();

        avltree::AVLTree< std::pair<Attribute*,size_t>,KeyComparator >* 
                    getAVLtree();

        std::string getKeyType();

    private:
         avltree::AVLTree< std::pair<Attribute*,size_t>,KeyComparator >* tree;
         std::string keyType;
};

class KeyComparator{

    public:
        static bool smaller(const std::pair<Attribute*,size_t>& o1,
                                const std::pair<Attribute*,size_t>& o2){


            Attribute* thisAttr = o1.first;
            Attribute* rhs = o2.first;
            int ergebnis = thisAttr->Compare(rhs);

            if (ergebnis == -1) {

                return true;
            }

            if (ergebnis == 0 && (o1.second < o2.second)){

                return true;
            }

            return false;
        }


        static bool equal(const std::pair<Attribute*,size_t>& o1,
                                const std::pair<Attribute*,size_t>& o2){

                    Attribute* thisAttr = o1.first;
                    Attribute* rhs = o2.first;
                    int ergebnis = thisAttr->Compare(rhs);



                    if (ergebnis == 0 && (o1.second == o2.second)){
                        return true;
                    }

                    return false;
        }


        static bool greater(const std::pair<Attribute*,size_t>& o1,
                                const std::pair<Attribute*,size_t>& o2){

                    Attribute* thisAttr = o1.first;
                    Attribute* rhs = o2.first;
                    int ergebnis = thisAttr->Compare(rhs);

                    if (ergebnis == 1) {

                        return true;
                    }

                    if (ergebnis == 0 && (o1.second > o2.second)){

                        return true;
                    }

                    return false;

     }

};


} //ende namespace mmalgebra

#endif
