/*
MemUpdateStorage.cpp
Created on: 06.05.2018
Author: simon

*/



#include "MemUpdateStorage.h"
#include "MPoint2.h"

#include "SecondoSMI.h"


namespace temporal2algebra {
using namespace boost::interprocess;


std::ostream &operator<<(std::ostream &os, SharedData const &l) {
    os << "[SharedData: backReference: " << l.backReference <<
            ", finalUnit:" << l.finalUnit <<"]";
    SharedUnits::const_iterator it;
    for (it = l.units.begin(); it != l.units.end(); ++it) {
        os << endl << *it;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, SharedDataMapValue const &l) {
   return os << "[SharedDataMapValue: id: " << l.first << " ]\n"
            << l.second;
}

std::ostream &operator<<(std::ostream &os, SharedMemStorage const &l) {
    os << "[SharedMemStorage: numOfUsers: " << l.numOfUsers
       << ", lastKnownStorageId: " << l.lastKnownStorageId << "]";
    SharedDataMap::const_iterator it;
    for (it = l.dataMap.begin(); it != l.dataMap.end(); ++it) {
      os << "\n" << *it;
    }
    return os;
}

//Try to find or create the shared_memory.
//If the caller is interested if the shared mem was found or created
//she can use the isNewlyCreated() method to find out


void MemUpdateStorage::initMem() {
    // assume either there is no shared_memory at all
    // then initialize from log
    // or
    // the shared memory has been completely constructed
    // then do nothing

    try {
        segment = boost::interprocess::managed_shared_memory
                (boost::interprocess::create_only,
                        memStorageName.c_str(),
                        65536);
        VoidAllocator allocator(segment.get_segment_manager());
        cout << "creating SharedMemStorage:" << shMemDataName.c_str() << "\n";

        memdata = segment.construct<SharedMemStorage>
            (shMemDataName.c_str())/*object name*/
            (allocator/*allocator as last ctor arg*/);

        newly_created=true;
    }
    catch (const boost::interprocess::interprocess_exception ex) {
        cout << "couldn't create shm - someone else did. Try to open.\n";
        cout << ex.what() << endl;;
        segment = boost::interprocess::managed_shared_memory
                (boost::interprocess::open_only,
                        memStorageName.c_str());

        cout << "finding MemData: " << shMemDataName.c_str() << "\n";
        memdata = segment.find<SharedMemStorage>(shMemDataName.c_str()).first;
        cout << "memdata->dataMap.size(): "
                << memdata->dataMap.size() << endl;

        return;
    } catch (const exception ex) {
        cout << "Ups: MemUpdateStorage::initMem() - unhandled exeption:\n"
                << ex.what() << endl;
        throw;
    }
}

bool MemUpdateStorage::isNewlyCreated() const {
    return newly_created;
}

void MemUpdateStorage::assertSameDb() const {
    assert (SmiEnvironment::IsDatabaseOpen());
    assert (currentDbName == SmiEnvironment::CurrentDatabase());
}

MemUpdateStorage::~MemUpdateStorage() {
    cout <<  "MemUpdateStorage::~MemUpdateStorage()\n";

    if (!memdata) {
        cout << "no memdata to clean up.\n";
        return;
    }
    cout << "contents of SharedMemStorage:\n";
    cout << *memdata << endl;

    memdata->numOfUsers--;

    if (memdata->numOfUsers == 0) {
        cout << "we are the last remaining user, cleaning up...\n";
        segment.destroy<SharedMemStorage>(shMemDataName.c_str());
        cout << "SharedMemStorage destroyed\n";
        boost::interprocess::shared_memory_object::remove(
                memStorageName.c_str());
        cout << "SharedMemory destroyed\n";
    } else {
        cout << "there are other users - skip cleanup...\n";
    }
}

MemUpdateStorage::MemUpdateStorage(std::string database) :
                currentDbName(database),
                newly_created(false)
{
    cout << "MemUpdateStorage::MemUpdateStorage(\""
            << database << "\")\n";

    memStorageName = "MemUpdateStorage_SHM_" + currentDbName + "_" +"MPoint2";
    shMemDataName  = "data";

    memQueueName = "MemUpdateStorage_Queue_" + currentDbName + "_" +"MPoint2";

    initMem();
    memdata->numOfUsers++;
}

const MemStorageId MemUpdateStorage::memCreateId() {
    cout << "MemUpdateStorage::memCreateId()\n";
    assertSameDb();
    MemStorageId newId = ++(memdata->lastKnownStorageId);
    cout << "MemUpdateStorage::memCreateId()\n: " << newId << endl;
    return newId;
}

void MemUpdateStorage::memCreateId(MemStorageId id) {
    cout << "MemUpdateStorage::memCreateId("<< id << ")\n";
    assertSameDb();
    assert (id > memdata->lastKnownStorageId);
    memdata->lastKnownStorageId = id;
    cout << "MemUpdateStorage::memCreateId("<< id << ")\n";
}

void MemUpdateStorage::memSetBackRef(const MemStorageId& id,
        const BackReference& backRef, const Unit& finalUnit) {
    cout << "MemUpdateStorage::memSetBackRef(id: " << id
         << ", backRef: " << backRef << ")\n";
    assertSameDb();

    // id should not have a backref:
    assert(!memHasMemoryUnits(id));

    VoidAllocator allocator(segment.get_segment_manager());
    memdata->dataMap.insert(
            SharedDataMapValue(id, SharedData(backRef, finalUnit, allocator)));
    cout << "createMem():" << backRef << endl;
}

bool MemUpdateStorage::memHasMemoryUnits(const MemStorageId id) const {
    return (memdata->dataMap.count(id) > 0);
}

Unit MemUpdateStorage::memGet(const MemStorageId id, size_t memIndex) {
        cout << "MemUpdateStorage::memGet(id: " << id
                << ", memIndex: " << memIndex << ")\n";
        assert (id > 0);
        assertSameDb();
        SharedDataMap::iterator it = memdata->dataMap.find(id);
        assert(it != memdata->dataMap.end());
        assert(memIndex < it->second.units.size());
        return it->second.units.at(memIndex);
}

Unit MemUpdateStorage::memGetFinalUnit(const MemStorageId id) {
        cout << "MemUpdateStorage::memGet(id: " << id << ")\n";
        assert (id > 0);
        assertSameDb();
        SharedDataMap::iterator it = memdata->dataMap.find(id);
        assert(it != memdata->dataMap.end());
        return it->second.finalUnit;
}

MemStorageId MemUpdateStorage::memGetId(const BackReference& backRef) const {
    SharedDataMap::iterator it;
    for (it =  memdata->dataMap.begin();
            it !=  memdata->dataMap.end();
            ++it) {
        if (it->second.backReference == backRef) {
            return it->first;
        }
    }
    return -1;
}

int MemUpdateStorage::memSize(const MemStorageId id) {
        cout << "MemUpdateStorage::memSize(id: " << id << ")\n";
        assert (id > 0);
        assertSameDb();
        SharedDataMap::iterator it = memdata->dataMap.find(id);
        assert(it != memdata->dataMap.end());
        return (int) it->second.units.size();
}

void MemUpdateStorage::memAppend
(MemStorageId id, const temporalalgebra::UPoint& unit) {
    cout << "MemUpdateStorage::memAppend(" << id << ", " << unit << ")\n";
    assert (id > 0);
    assertSameDb();

    SharedDataMap::iterator it = memdata->dataMap.find(id);
    if (it == memdata->dataMap.end()) {
        cout << "no Storage for id :" << id << endl;
        assert(false);
    }
    it->second.units.push_back(unit);
    it->second.finalUnit = unit;
}

void MemUpdateStorage::memClear (const MemStorageId id) {
    cout << "MemUpdateStorage::memClear(" << id << ")\n";
    assert (id>0);
    assertSameDb(); // probably can be removed if we check for id=0;

    SharedDataMap::iterator it = memdata->dataMap.find(id);
    if (it != memdata->dataMap.end()) {
        memdata->dataMap.erase(it);
    }
}

MemStorageIds MemUpdateStorage::getIdsToPush() const {
    cout << "MemUpdateStorage::getIdsToPush()\n";
        assertSameDb();
        MemStorageIds res(0);

        SharedDataMap::iterator it;
        for (it =  memdata->dataMap.begin();
                it !=  memdata->dataMap.end();
                ++it) {
            res.push_back(it->first);
        }

        cout << "found " << res.size() << " entries\n";
        return res;
    }


int MemUpdateStorage::memPushToFlobs(const MemStorageId idToPush) {
    cout << "MemUpdateStorage::memPushToFlobs(idToPush: "
         << idToPush   << ")\n";

    assert (idToPush > 0);

    if (!memdata) {
        cout << "no MemStorage\n";
        assert (false);
        return 0;
    }

    SharedDataMap::iterator it = memdata->dataMap.find(idToPush);
    if (it == memdata->dataMap.end()) {
        cout << "no MemDate with idToPush: " << idToPush << endl;
        assert (false);
        return 0;
    }


    cout << "finding Backreference:\n";
    BackReference* backref_it = &(it->second.backReference);


    std::string relationName = backref_it->relationName;
    SecondoCatalog* catalog = SecondoSystem::GetCatalog();
    if (!catalog->IsObjectName(relationName)) {
        cout << "no secondo object with name '" << relationName
                        << "' found\n";
        return 0;
    }

    Word relationWord;
    bool defined;
    catalog->GetObject(relationName, relationWord, defined);
    Relation* relation = static_cast<Relation*>(relationWord.addr);
    if (!relation) {
        cout << "couldn't get Relation* for '"
                << relationName << "'\n";
        return 0;
    }

    cout << "retrieving tuple with TID " << backref_it->tupleId << endl;
    Tuple*  tuple = relation->GetTuple(backref_it->tupleId, true);

    if (!tuple) {
        cout << "no tuple found\n";
        return 0;
    }

    cout << "found target tuple: " << *tuple << endl;

    // GetAttribute just gives us a pointer to the existing Attr
    // read: do not delete, do not deleteIfAllowed; better not touch
    MPoint2* moving = static_cast<MPoint2*>(
            tuple->GetAttribute(backref_it->attrPos));

    if (moving->getMemId() != idToPush) {
        cout << "mismatch between storageIds! "
                << "moving: " << moving->getMemId()
                << "idToPush: " << idToPush << endl;
        tuple->DeleteIfAllowed();
        return 0;
    }

    // this implicitly creates a new moving with all data
    // pushed to the flobs and empty backreference
    MPoint2* moving_for_update = moving->Clone();

    std::vector<int> changedIndices(1);
    changedIndices[0] = backref_it->attrPos;
    std::vector<Attribute*> newAttrs(1);
    newAttrs[0] = moving_for_update;
    // it seems that UpdateTuple does not incr ref count
    // for newAttrs -> so do not delete(IfAllowed) moving.
    relation->UpdateTuple(tuple,
            changedIndices,
            newAttrs);

    catalog->ModifyObject(backref_it->relationName,
            relationWord);
    catalog->CleanUp(false, false);

    relation->Close();
    relation = 0;

    memClear(idToPush);

    tuple->DeleteIfAllowed();

    return 1;
}

int MemUpdateStorage::printMem() const {
    if (memdata) {
            cout << *memdata << endl;
            return 1;
    } else {
        cout << "no MemData present\n";
        return 0;
    }
}

} /* namespace temporal2algebra */
