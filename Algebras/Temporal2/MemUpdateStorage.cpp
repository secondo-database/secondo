/*
implementation of MemUpdateStorage

*/

#include "MemUpdateStorage.h"
#include "MPoint2.h"

#include "SecondoSMI.h"


namespace temporal2algebra {
using namespace boost::interprocess;


std::ostream &operator<<(std::ostream &os, SharedData const &l) {
    Unit u;
    l.finalUnit.createUnit(&u);
    os << "[SharedData: backReference: " << l.backReference <<
            ", finalUnit:" << u <<"]";
    SharedUnits::const_iterator it;
    for (it = l.units.begin(); it != l.units.end(); ++it) {
        it->createUnit(&u);
        os << std::endl << u;
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
                        storage_size );
        VoidAllocator allocator(segment.get_segment_manager());
        std::cout << "creating SharedMemStorage:" << shMemDataName.c_str()
           << "\n";

        memdata = segment.construct<SharedMemStorage>
        (shMemDataName.c_str())
        (allocator);

        newly_created=true;
    }
    catch (const boost::interprocess::interprocess_exception ex) {
        std::cout << "couldn't create shm - someone else did. Try to open.\n";
        std::cout << ex.what() << std::endl;;
        segment = boost::interprocess::managed_shared_memory
                (boost::interprocess::open_only,
                        memStorageName.c_str());

        std::cout << "finding MemData: " << shMemDataName.c_str() << "\n";
        memdata = segment.find<SharedMemStorage>(shMemDataName.c_str()).first;
        std::cout << "memdata->dataMap.size(): "
                << memdata->dataMap.size() << std::endl;

        return;
    } catch (const std::exception ex) {
        std::cout << "Ups: MemUpdateStorage::initMem() - unhandled exeption:\n"
                << ex.what() << std::endl;
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
    std::cout <<  "MemUpdateStorage::~MemUpdateStorage()\n";

    if (!memdata) {
        std::cout << "no memdata to clean up.\n";
        return;
    }
    std::cout << "contents of SharedMemStorage:\n";
    std::cout << *memdata << std::endl;

    memdata->numOfUsers--;

    if (memdata->numOfUsers == 0) {
        std::cout << "we are the last remaining user, cleaning up...\n";
        segment.destroy<SharedMemStorage>(shMemDataName.c_str());
        std::cout << "SharedMemStorage destroyed\n";
        boost::interprocess::shared_memory_object::remove(
                memStorageName.c_str());
        std::cout << "SharedMemory destroyed\n";
    } else {
        std::cout << "there are other users - skip cleanup...\n";
    }
}

MemUpdateStorage::MemUpdateStorage(std::string database) :
                        currentDbName(database),
                        newly_created(false)
{
    std::cout << "MemUpdateStorage::MemUpdateStorage(\""
            << database << "\")\n";

    memStorageName = "MemUpdateStorage_SHM_" + currentDbName + "_" +"MPoint2";
    shMemDataName  = "data";

    memQueueName = "MemUpdateStorage_Queue_" + currentDbName + "_" +"MPoint2";

    initMem();
    memdata->numOfUsers++;
}

const MemStorageId MemUpdateStorage::memCreateId() {
    std::cout << "MemUpdateStorage::memCreateId()\n";
    assertSameDb();
    MemStorageId newId = ++(memdata->lastKnownStorageId);
    std::cout << "MemUpdateStorage::memCreateId()\n: " << newId << std::endl;
    return newId;
}

void MemUpdateStorage::memCreateId(MemStorageId id) {
    std::cout << "MemUpdateStorage::memCreateId("<< id << ")\n";
    assertSameDb();
    assert (id > memdata->lastKnownStorageId);
    memdata->lastKnownStorageId = id;
    std::cout << "MemUpdateStorage::memCreateId("<< id << ")\n";
}

void MemUpdateStorage::memSetBackRef(const MemStorageId& id,
        const BackReference& backRef, const Unit& finalUnit) {
    std::cout << "MemUpdateStorage::memSetBackRef(id: " << id
            << ", backRef: " << backRef << ")\n";
    assertSameDb();

    // id should not have a backref:
    assert(!memHasMemoryUnits(id));

    VoidAllocator allocator(segment.get_segment_manager());
    memdata->dataMap.insert(
            SharedDataMapValue(id, SharedData(backRef, finalUnit, allocator)));
    std::cout << "createMem():" << backRef << std::endl;
}

bool MemUpdateStorage::memHasMemoryUnits(const MemStorageId id) const {
    return (memdata->dataMap.count(id) > 0);
}

Unit MemUpdateStorage::memGet(const MemStorageId id, size_t memIndex) {
    std::cout << "MemUpdateStorage::memGet(id: " << id
            << ", memIndex: " << memIndex << ")\n";
    assert (id > 0);
    assertSameDb();
    SharedDataMap::iterator it = memdata->dataMap.find(id);
    assert(it != memdata->dataMap.end());
    assert(memIndex < it->second.units.size());
    Unit result;
    it->second.units.at(memIndex).createUnit(&result);
    return result;
}

Unit MemUpdateStorage::memGetFinalUnit(const MemStorageId id) {
    std::cout << "MemUpdateStorage::memGet(id: " << id << ")\n";
    assert (id > 0);
    assertSameDb();
    SharedDataMap::iterator it = memdata->dataMap.find(id);
    assert(it != memdata->dataMap.end());
    Unit result;
    it->second.finalUnit.createUnit(&result);
    return result;
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
    std::cout << "MemUpdateStorage::memSize(id: " << id << ")\n";
    assert (id > 0);
    assertSameDb();
    SharedDataMap::iterator it = memdata->dataMap.find(id);
    assert(it != memdata->dataMap.end());
    return (int) it->second.units.size();
}

void MemUpdateStorage::memAppend
(MemStorageId id, const temporalalgebra::UPoint& unit) {
    std::cout << "MemUpdateStorage::memAppend(" << id << ", " << unit << ")\n";
    assert (id > 0);
    assertSameDb();

    SharedDataMap::iterator it = memdata->dataMap.find(id);
    if (it == memdata->dataMap.end()) {
        std::cout << "no Storage for id :" << id << std::endl;
        assert(false);
    }
    FlatUnit u(unit);
    it->second.units.push_back(u);
    it->second.finalUnit = u;
}

void MemUpdateStorage::memClear (const MemStorageId id) {
    std::cout << "MemUpdateStorage::memClear(" << id << ")\n";
    assert (id>0);
    assertSameDb(); // probably can be removed if we check for id=0;

    SharedDataMap::iterator it = memdata->dataMap.find(id);
    if (it != memdata->dataMap.end()) {
        memdata->dataMap.erase(it);
    }
}

MemStorageIds MemUpdateStorage::getIdsToPush() const {
    std::cout << "MemUpdateStorage::getIdsToPush()\n";
    assertSameDb();
    MemStorageIds res(0);

    SharedDataMap::iterator it;
    for (it =  memdata->dataMap.begin();
            it !=  memdata->dataMap.end();
            ++it) {
        res.push_back(it->first);
    }

    std::cout << "found " << res.size() << " entries\n";
    return res;
}

const BackReference
MemUpdateStorage::getBackReference(const MemStorageId id) {
    std::cout << "MemUpdateStorage::memGetBackReference(" << id << ")\n";
    assert (id>0);
    assertSameDb(); // probably can be removed if we check for id=0;

    SharedDataMap::iterator it = memdata->dataMap.find(id);
    if (it != memdata->dataMap.end()) {
        return it->second.backReference;
    }
    return BackReference();
}


int MemUpdateStorage::memPushToFlobs(
        const MemStorageId idToPush, bool keep_reference ) {
    std::cout << "MemUpdateStorage::memPushToFlobs(idToPush: "
            << idToPush   << ")\n";

    assert (idToPush > 0);

    if (!memdata) {
        std::cout << "no MemStorage\n";
        assert (false);
        return 0;
    }

    SharedDataMap::iterator it = memdata->dataMap.find(idToPush);
    if (it == memdata->dataMap.end()) {
        std::cout << "no MemDate with idToPush: " << idToPush << std::endl;
        assert (false);
        return 0;
    }

// we found some MemData:


    std::cout << "finding Backreference:\n";
    BackReference* backref_it = &(it->second.backReference);


    std::string relationName = backref_it->relationName;
    SecondoCatalog* catalog = SecondoSystem::GetCatalog();
    if (!catalog->IsObjectName(relationName)) {
        std::cout << "no secondo object with name '" << relationName
                << "' found\n";
        memClear(idToPush);
        return 0;
    }

    Word relationWord;
    bool defined;
    catalog->GetObject(relationName, relationWord, defined);
    Relation* relation = static_cast<Relation*>(relationWord.addr);
    if (!relation) {
        std::cout << "couldn't get Relation* for '"
                << relationName << "'\n";
        memClear(idToPush);
        return 0;
    }

    std::cout << "retrieving tuple with TID " << backref_it->tupleId
       << std::endl;
    Tuple*  tuple = relation->GetTuple(backref_it->tupleId, true);

    if (!tuple) {
        std::cout << "no tuple found\n";
        memClear(idToPush);
        return 0;
    }

    std::cout << "found target tuple: " << *tuple << std::endl;

    // GetAttribute just gives us a pointer to the existing Attr
    // read: do not delete, do not deleteIfAllowed; better not touch
    MPoint2* moving = static_cast<MPoint2*>(
            tuple->GetAttribute(backref_it->attrPos));

    if (moving->getMemId() != idToPush) {
        std::cout << "mismatch between storageIds! "
                << "moving: " << moving->getMemId()
                << "idToPush: " << idToPush << std::endl;
        tuple->DeleteIfAllowed();
        memClear(idToPush);
        return 0;
    }

    // this implicitly creates a new moving with all data
    // pushed to the flobs and empty backreference
    MPoint2* moving_for_update = moving->Clone();
    if (keep_reference) {
        moving_for_update->setMemId(moving->getMemId());
    }

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
    catalog->CleanUp(false);

    relation->Close();
    relation = 0;

    memClear(idToPush);

    tuple->DeleteIfAllowed();

    return 1;
}

int MemUpdateStorage::getNumberOfUsers() const {
    return memdata?memdata->numOfUsers:-1;
}

} /* namespace temporal2algebra */
