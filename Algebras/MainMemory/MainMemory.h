/*

1 Defines, includes, and constants

*/


#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include <string>
#include <map>
#include "RelationAlgebra.h"
#include <vector>

using namespace std;


namespace mmalgebra{

class MemCatalog;
class Memory;
class MemoryObject;
class MemoryRelObject;
class MemoryAttributeObject;

class MemCatalog {

    public:
        //Konstruktor;
        MemCatalog (){
            memSizeTotal=1;  //die Hauptspeichergroesse in MB
            usedMemSize=0;     //die benutzte Speichergroesse in B
            };

        void setMemSizeTotal(size_t size) {
            memSizeTotal = size;
        }
        size_t getMemSizeTotal(){
            return memSizeTotal;
        }
        size_t getUsedMemSize() {
            return usedMemSize;
        }
        void setUsedMemSize(size_t size) {
            usedMemSize = size;
        }

    // was muss ich das wirklich privat machen,
        size_t memSizeTotal; //in MB
        size_t usedMemSize;  //in Byte
        map<string,MemoryObject*> memContents;



};



class MemoryObject {
    public:
        //virtual damit erste späte Bindung darüber entscheidet
        // welcher Destruktor verwendet werden soll
        virtual ~MemoryObject(){
        }

        void setExtStorage(bool sES);
        size_t getObjectSize(); //die Gesamtgröße des Objekts
                                //(extStorageSize+memSize)
        void setMemSize(size_t i);
        void setExtStorageSize(size_t i);
        size_t getExtStorageSize();
        string getObjectTypeExpr();
        void setObjectTypeExpr(string oTE);
        void toStringOut(){
            cout<<"MemoryObject und die Membervariablen lauten: "<<endl;
            cout<<"1. extStorage: "<<extStorage<<endl;
            cout<<"2. memsize: "<<memSize<<endl;
            cout<<"3. extStorageSize: "<<extStorageSize<<endl;
            cout<<"4. objectTypeExpr: "<<objectTypeExpr<<endl;
        }
        //noch eine virtuelle Funktion = 0, um die Klasse abstrakt zu machen??
    protected:
        bool extStorage;                // ganzes Objekt im HS
        size_t memSize;                 // Größe die das Objekt im HS belegt
        size_t extStorageSize;          // Größe die das Objekt auf HD belegt
        string objectTypeExpr;          // typeExpr des zu ladenden Objekts,
                                        // bei Relation die Tupelbeschreibung,
                                        // sonst die Attributbeschreibung

};

void MemoryObject::setExtStorage(bool sES){
    extStorage = sES;
}
size_t MemoryObject::getObjectSize(){
    return memSize + extStorageSize;
}
void MemoryObject::setMemSize(size_t i){
    memSize = i;
}
void MemoryObject::setExtStorageSize(size_t i){
    extStorageSize = i;
}
size_t MemoryObject::getExtStorageSize(){
   return extStorageSize;
}
string MemoryObject::getObjectTypeExpr(){
    return objectTypeExpr;
}
void MemoryObject::setObjectTypeExpr(string oTE){
    objectTypeExpr=oTE;
};





class MemoryRelObject : public MemoryObject {

    public:

        //MemoryRelObject(){};

        ~MemoryRelObject(){
            if(mmrelDiskpart){
                delete mmrelDiskpart;
                mmrelDiskpart=0;
            }
        }

        vector<Tuple*>* getmmrel();
        void setmmrel(vector<Tuple*>* _mmrel);
        Relation* getmmrelDiskpart();
        void setmmrelDiskpart(Relation* _mmrelDiskpart);

        void toStringOut(){
            cout<<"MemoryRelObject und die Membervariablen lauten: "<<endl;
            cout<<"1. extStorage: "<<extStorage<<endl;
            cout<<"2. memsize: "<<memSize<<endl;
            cout<<"3. extStorageSize: "<<extStorageSize<<endl;
            cout<<"4. objectTypeExpr: "<<objectTypeExpr<<endl;
            cout<<"5. Adresse des TupleVektors ist: "<<&mmrel<<endl;
        }



        /*
Below the mandatory set of algebra support functions is declared.
Note that these functions need to be static member functions of the class.
Their implementations do nothing which depends on the state of an instance.

*/
//        static Word In( const ListExpr typeInfo, const ListExpr instance,
//                        const int errorPos, ListExpr& errorInfo,
//                        bool& correct );

        static ListExpr Out( ListExpr typeInfo, Word value );


        //static Word     Create( const ListExpr typeInfo );

        //static void     Delete( const ListExpr typeInfo, Word& w );

        //static void     Close( const ListExpr typeInfo, Word& w );

        //static Word     Clone( const ListExpr typeInfo, const Word& w );

        static bool     KindCheck(  ListExpr type, ListExpr& errorInfo  );

        static int      SizeOfObj();

        static ListExpr Property();

/*
The following function defines the name of the type constructor, resp. the name
Secondo uses for this type.

*/
        static const string BasicType() { return "memoryRelObject"; }
                                            //oder nur memoryObject???


        static const bool checkType(const ListExpr type){
            return listutils::isSymbol(type, BasicType());
        }

    private:
        vector<Tuple*>* mmrel;
        Relation* mmrelDiskpart;

};


vector<Tuple*>* MemoryRelObject::getmmrel(){
    return mmrel;
    };

void MemoryRelObject::setmmrel(vector<Tuple*>* _mmrel){
    mmrel = _mmrel;
    };

Relation* MemoryRelObject::getmmrelDiskpart(){
    return mmrelDiskpart;
    };

void MemoryRelObject::setmmrelDiskpart(Relation* _mmrelDiskpart){
    mmrelDiskpart = _mmrelDiskpart;
    };

//
// Word MemoryRelObject::In( const ListExpr typeInfo, const ListExpr instance,
//                       const int errorPos, ListExpr& errorInfo,
//                        bool& correct ){
//
//  correct = false;
//    Word result = SetWord(Address(0));
//    const string errMsg = "Leider ist in noch nicht implementiert";
//    return result;
//
//}

ListExpr MemoryRelObject::Out( ListExpr typeInfo, Word value ){
/*

Mach es!!!!!!!!!!!!!!!!!!!!!


*/





ListExpr li = nl->IntAtom(23);
return li;
//                return l;
};


bool MemoryRelObject::KindCheck( ListExpr type, ListExpr& errorInfo ){

    return (nl->IsEqual(type, MemoryRelObject::BasicType()));

}

int MemoryRelObject::SizeOfObj(){

int i = 78;
return i;

}



//nochmal!!!
ListExpr MemoryRelObject::Property(){
    return (nl->TwoElemList (
        nl->FourElemList (
            nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
            nl->StringAtom("List Rep"),
            nl->StringAtom("Example List")),
        nl->FourElemList (
            nl->StringAtom("-> SIMPLE"), //nicht doch eher rel -> ja nach was??
            nl->StringAtom(MemoryRelObject::BasicType()),
            nl->StringAtom("??A List of tuples"),
            nl->StringAtom(("Meyer, 7"),("Muller, 5"))
            )));
}



TypeConstructor MemoryRelObjectTC(
    MemoryRelObject::BasicType(),     // name of the type in SECONDO
    MemoryRelObject::Property,        // property function describing signature
    MemoryRelObject::Out, 0,          // out und in functions
    0, 0,                             // SaveToList, RestoreFromList functions
    0,0,                             // object creation and deletion
    0, 0,                            // object open, save
    0,0,                             // close and clone
    0,                                // cast function
    MemoryRelObject::SizeOfObj,      // sizeof function
    MemoryRelObject::KindCheck);      // kind checking function





class MemoryAttributeObject : public MemoryObject {

    public:

        void setAttributeObject(Attribute* attr);

        void toStringOut(){
            cout<<"MemoryAttributeObject, Membervariablen lauten: "<<endl;
            cout<<"1. extStorage: "<<extStorage<<endl;
            cout<<"2. memsize: "<<memSize<<endl;
            cout<<"3. extStorageSize: "<<extStorageSize<<endl;
            cout<<"4. objectTypeExpr: "<<objectTypeExpr<<endl;
            cout<<"5. Adresse des Attributs ist: "<<&attributeObject<<endl;}
    private:
         Attribute* attributeObject;

};

 void MemoryAttributeObject::setAttributeObject(Attribute* attr){
            attributeObject=attr;
        }



} //ende namespace mmalgebra
