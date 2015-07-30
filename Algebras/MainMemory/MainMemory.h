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

using namespace std;



namespace mmalgebra{

class MemCatalog;
class Memory;
class MemoryObject;
class MemoryRelObject;
class MemoryAttributeObject;
class MemoryRtreeObject;


class MemCatalog {

    public:
        MemCatalog (){
            memSizeTotal=256;  //main memory size in MB
            usedMemSize=0;     //used main memory size in B
            };
        ~MemCatalog();

        void setMemSizeTotal(size_t size);

        size_t getMemSizeTotal();

        size_t getUsedMemSize();

        size_t getAvailabeMemSize();

        //only used in memgetcatalog
        map<string,MemoryObject*>* getMemContent();


        bool insert (const string& name, MemoryObject* obj);

        bool deleteObject (const string& name);

        void clear ();

        bool isMMObject(const string& objectName);

        //*Precondition*: "isMMObject( objectName ) == true"
        MemoryObject* getMMObject(const string& objectName);

        ListExpr getMMObjectTypeExpr(const string& oN);


    private:
        size_t usedMemSize;  //in Byte
        size_t memSizeTotal; //in MB
        map<string,MemoryObject*> memContents;
};



class MemoryObject {
    public:
        virtual ~MemoryObject();

        void setMemSize(size_t i);
        size_t getMemSize ();

        string getObjectTypeExpr();
       // void setObjectTypeExpr(string oTE);
        void toStringOut(){
            cout<<"MemoryObject und die Membervariablen lauten: "<<endl;
            cout<<"2. memsize: "<<memSize<<endl;
            cout<<"4. objectTypeExpr: "<<objectTypeExpr<<endl;
        }

        static const string BasicType() { return "memoryObject"; }

    protected:
        size_t memSize;              // object size in main memory in byte
        string objectTypeExpr;       // the tuple description for relations,
                                     // or the attribute description


};






class MemoryRelObject : public MemoryObject {

    public:

        MemoryRelObject();
        MemoryRelObject(vector<Tuple*>* _mmrel,
                    size_t _memSize, string _objectTypeExpr);
        MemoryRelObject (string _objectTypeExpr);
        ~MemoryRelObject();

        vector<Tuple*>* getmmrel();
        void setmmrel(vector<Tuple*>* _mmrel);

        void addTuple(Tuple* tup);

        void toStringOut(){
            cout<<"MemoryRelObject und die Membervariablen lauten: "<<endl;
            cout<<"2. memsize: "<<memSize<<endl;
            cout<<"4. objectTypeExpr: "<<objectTypeExpr<<endl;
            cout<<"5. Adresse des TupleVektors ist: "<<&mmrel<<endl;
        }

        ListExpr toListExpr();

        static Word In( const ListExpr typeInfo, const ListExpr instance,
                        const int errorPos, ListExpr& errorInfo,
                        bool& correct );

        static ListExpr Out( ListExpr typeInfo, Word value );

        static bool KindCheck( ListExpr type, ListExpr& errorInfo );

        static Word create(const ListExpr typeInfo);

        static int SizeOfObj();

        static void deleteMemoryRelObject(const ListExpr typeInfo, Word& w);

        static ListExpr Property();

        static const string BasicType() { return "memoryRelObject"; }

        static const bool checkType(const ListExpr type);

    private:
        vector<Tuple*>* mmrel;

};


class MemoryAttributeObject : public MemoryObject {

    public:
        MemoryAttributeObject(Attribute* _attr,
                size_t _memSize, string _objectTypeExpr);
        ~MemoryAttributeObject();

        void setAttributeObject(Attribute* attr);
        Attribute* getAttributeObject();

        void toStringOut(){
            cout<<"MemoryAttributeObject, Membervariablen lauten: "<<endl;
            cout<<"2. memsize: "<<memSize<<endl;
            cout<<"4. objectTypeExpr: "<<objectTypeExpr<<endl;
            cout<<"5. Adresse des Attributs ist: "<<&attributeObject<<endl;}

        static const string BasicType() { return "memoryAttributeObject"; }

    private:
         Attribute* attributeObject;

};



class MemoryRtreeObject : public MemoryObject {

    public:
        MemoryRtreeObject();
        MemoryRtreeObject(mmrtree::RtreeT<2, size_t>* _rtree,
                        size_t _memSize, string _objectTypeExpr);
        ~MemoryRtreeObject();

        void setRtree (mmrtree::RtreeT<2, size_t>* _rtree);

        mmrtree::RtreeT<2, size_t>* getrtree();

        void toStringOut(){
            cout<<"MemoryRtreeObject, Membervariablen lauten: "<<endl;
            cout<<"2. memsize: "<<memSize<<endl;
            cout<<"4. objectTypeExpr: "<<objectTypeExpr<<endl;
            cout<<"5. Die Adresse des Indexes ist: "<<&rtree<<endl;}

        static const string BasicType() { return "memoryRtreeObject"; }

        static const bool checkType(const ListExpr type){
            return (nl->ToString(type)==BasicType());
        }


    private:
         mmrtree::RtreeT<2, size_t>* rtree;

};



class MemoryAVLObject : public MemoryObject {

    public:
        MemoryAVLObject();
        ~MemoryAVLObject();

        void toStringOut(){
            cout<<"MemoryAVLObject, Membervariablen lauten: "<<endl;
            cout<<"2. memsize: "<<memSize<<endl;
            cout<<"4. objectTypeExpr: "<<objectTypeExpr<<endl;
            cout<<"5. Die Adresse des Indexes ist: "<<&tree<<endl;}

        static const string BasicType() { return "memoryAVLObject"; }

        static const bool checkType(const ListExpr type){
            return (nl->ToString(type)==BasicType());
        }


    private:
         avltree::AVLTree< pair<Attribute*,size_t> > tree;

};




} //ende namespace mmalgebra

#endif
