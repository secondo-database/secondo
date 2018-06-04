/*
MPoint2.h
Created on: 03.05.2018
Author: simon

*/

#include "MPoint2.h"
#include "SecondoSMI.h"

namespace temporal2algebra {

bool CheckMPoint2( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, MPoint2::BasicType() ));
}

ListExpr MPoint2Property() {
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mpoint2) "),
                             nl->StringAtom("( u1 ... un ) "),
        nl->StringAtom("(((i1 i2 TRUE FALSE) (1.0 2.2 2.5 2.1)) ...)"))));
}

//TypeConstructor* getMPoint2TypePtr() {
//    TypeConstructor* type = new TypeConstructor (
//            MPoint2::BasicType(),   //name
//            MPoint2Property,        //property function describing signature
//            OutMapping<MPoint, UPoint, OutUPoint>,
//            InMapping<MPoint, UPoint, InUPoint>,//Out and In functions
//            0,
//            0,                 //SaveToList and RestoreFromList functions
//            CreateMapping<MPoint>,
//            DeleteMapping<MPoint>,     //object creation and deletion
//            OpenAttribute<MPoint>,
//            SaveAttribute<MPoint>,      // object open and save
//            CloseMapping<MPoint>,
//            CloneMapping<MPoint>, //object close and clone
//            CastMapping<MPoint>,    //cast function
//            SizeOfMapping<MPoint>, //sizeof function
//            CheckMPoint2 );
//    type->AssociateKind( Kind::TEMPORAL() );
//    type->AssociateKind( Kind::DATA() );
//    return type;
//}


//void MPoint2::CopyFrom(const Attribute* arg) {
//
//    const MPoint2* other = static_cast<const MPoint2*>(arg);
//    cout << "MPoint2::CopyFrom other:" << other->id << endl;
//
//    MemStorageManager* storage = MemStorageManager::getInstance();
//    id = storage->getNextId();
//
//        const std::vector<temporalalgebra::UPoint>& memUnits =
//                storage->get(other->id);
//        std::vector<temporalalgebra::UPoint>::const_iterator it;
//        for (it = memUnits.begin(); it != memUnits.end(); ++it) {
//            this->memAdd(*it);
//        }
//
//}

//Attribute* MPoint2::Clone() const {
//
//    MemStorageManager* storage = MemStorageManager::getInstance();
//    MemStorageId nextId = storage->getNextId();
//    cout << "MPoint2::Clone nextId:" << nextId << endl;
//
//    MPoint2* res = new MPoint2(this->GetNoComponents(), nextId);
//
//    const std::vector<temporalalgebra::UPoint>& memUnits =
//            storage->get(this->id);
//    std::vector<temporalalgebra::UPoint>::const_iterator it;
//    for (it = memUnits.begin(); it != memUnits.end(); ++it) {
//        res->memAdd(*it);
//    }
//
//    return res;
//}

void MPoint2::memClear() {
    cout << "MPoint2::memClear["<< id << "]()\n";
    MemStorageManager* storage = MemStorageManager::getInstance();
    storage->clear(id);
}

void MPoint2::memAppend(const temporalalgebra::UPoint& upoint) {
    cout << "MPoint2::memAppend[" << id << "](" << upoint << ")\n";
    MemStorageManager* storage = MemStorageManager::getInstance();
    if (id == 0) {
        id = storage->createId();
    }
    storage->append(id, upoint);
}

std::vector<temporalalgebra::UPoint> MPoint2::memGet() const {
    cout << "MPoint2::memRead[" << id << "]()\n";
    MemStorageManager* storage = MemStorageManager::getInstance();
    return storage->get(id);
}


} /* temporal2algebra */


namespace gentc {
template<>
void Delete<temporal2algebra::MPoint2>(const ListExpr typeInfo,Word &w){
    cout << "gentc::Delete<MPoint2>\n";
    cout << "IsDatabaseOpen()="
         << (SmiEnvironment::IsDatabaseOpen()?"true\n":"false\n")
         << "CurrentDatabase()="
         << SmiEnvironment::CurrentDatabase() << endl;

    temporal2algebra::MPoint2* B =
            static_cast<temporal2algebra::MPoint2*>(w.addr);
  int nof = B->NumOfFLOBs();
  for(int i=0;i< nof; i++){
    (B->GetFLOB(i))->destroy();
  }
  B->memClear();
  delete B;
  B = NULL;
  w.addr=0;
}

template<>
void Close<temporal2algebra::MPoint2>(const ListExpr typeInfo, Word& w ){
  cout << "genttc::Close<MPoint2>\n";
  cout << "IsDatabaseOpen()="
       << (SmiEnvironment::IsDatabaseOpen()?"true\n":"false\n")
       << "CurrentDatabase()="
       << SmiEnvironment::CurrentDatabase() << endl;
  delete static_cast<temporal2algebra::MPoint2*>(w.addr);
  w.addr = 0;
}
}


