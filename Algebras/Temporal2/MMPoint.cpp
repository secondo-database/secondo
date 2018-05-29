/*
MMPoint.h
Created on: 03.05.2018
Author: simon

*/

#include "MMPoint.h"
#include "SecondoSMI.h"

namespace temporal2algebra {

bool CheckMMPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, MMPoint::BasicType() ));
}

ListExpr MMPointProperty() {
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mmpoint) "),
                             nl->StringAtom("( u1 ... un ) "),
        nl->StringAtom("(((i1 i2 TRUE FALSE) (1.0 2.2 2.5 2.1)) ...)"))));
}

//TypeConstructor* getMMPointTypePtr() {
//    TypeConstructor* type = new TypeConstructor (
//            MMPoint::BasicType(),   //name
//            MMPointProperty,        //property function describing signature
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
//            CheckMMPoint );
//    type->AssociateKind( Kind::TEMPORAL() );
//    type->AssociateKind( Kind::DATA() );
//    return type;
//}


//void MMPoint::CopyFrom(const Attribute* arg) {
//
//    const MMPoint* other = static_cast<const MMPoint*>(arg);
//    cout << "MMPoint::CopyFrom other:" << other->id << endl;
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

//Attribute* MMPoint::Clone() const {
//
//    MemStorageManager* storage = MemStorageManager::getInstance();
//    MemStorageId nextId = storage->getNextId();
//    cout << "MMPoint::Clone nextId:" << nextId << endl;
//
//    MMPoint* res = new MMPoint(this->GetNoComponents(), nextId);
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

void MMPoint::memClear() {
    cout << "MMPoint::memClear["<< id << "]()\n";
    MemStorageManager* storage = MemStorageManager::getInstance();
    storage->clear(id);
}

void MMPoint::memAppend(const temporalalgebra::UPoint& upoint) {
    cout << "MMPoint::memAppend[" << id << "](" << upoint << ")\n";
    MemStorageManager* storage = MemStorageManager::getInstance();
    if (id == 0) {
        id = storage->createId();
    }
    storage->append(id, upoint);
}

std::vector<temporalalgebra::UPoint> MMPoint::memGet() const {
    cout << "MMPoint::memRead[" << id << "]()\n";
    MemStorageManager* storage = MemStorageManager::getInstance();
    return storage->get(id);
}


} /* temporal2algebra */


namespace gentc {
template<>
void Delete<temporal2algebra::MMPoint>(const ListExpr typeInfo,Word &w){
    cout << "gentc::Delete<MMPoint>\n";
    cout << "IsDatabaseOpen()="
         << (SmiEnvironment::IsDatabaseOpen()?"true\n":"false\n")
         << "CurrentDatabase()="
         << SmiEnvironment::CurrentDatabase() << endl;

    temporal2algebra::MMPoint* B =
            static_cast<temporal2algebra::MMPoint*>(w.addr);
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
void Close<temporal2algebra::MMPoint>(const ListExpr typeInfo, Word& w ){
  cout << "genttc::Close<MMPoint>\n";
  cout << "IsDatabaseOpen()="
       << (SmiEnvironment::IsDatabaseOpen()?"true\n":"false\n")
       << "CurrentDatabase()="
       << SmiEnvironment::CurrentDatabase() << endl;
  delete static_cast<temporal2algebra::MMPoint*>(w.addr);
  w.addr = 0;
}
}


