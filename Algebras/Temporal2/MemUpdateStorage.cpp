/*
MemUpdateStorage.cpp
Created on: 06.05.2018
Author: simon

*/



#include "MemUpdateStorage.h"

#include "SecondoSMI.h"


namespace temporal2algebra {
using namespace boost::interprocess;

std::ostream &operator<<(std::ostream &os, memData const &l) {
     return os << "[store: " << l.storageId
             << ", trans: " << l.transactionId
             << ", valid: " << (l.isValid?"true":"false")
             << ", " << l.unit << "]\n";
}




//Try to find or created the shared_memory.
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
        cout << "creating SharedDataVector\n";
        ShmemAllocator allocator (segment.get_segment_manager());
        cout << "creating MemData:" << shMemDataName.c_str() << "\n";
        memdata = segment.construct<MemData>
            (shMemDataName.c_str())/*object name*/
            (allocator /*allocator as last ctor arg*/);
        newly_created=true;
    }
    catch (const boost::interprocess::interprocess_exception ex) {
        cout << "couldn't create shm - someone else did. Try to open.\n";
        cout << ex.what() << endl;;
        segment = boost::interprocess::managed_shared_memory
                (boost::interprocess::open_only,
                        memStorageName.c_str());

        cout << "finding MemData:" << shMemDataName.c_str() << "\n";
        memdata = segment.find<MemData>(shMemDataName.c_str()).first;
        cout << "memdata->data.size(): "
                << memdata->data.size() << endl;
        return;
    } catch (const exception ex) {
        cout << "Ups: MemUpdateStorage::initialize() - unhandled exeption:\n"
                << ex.what() << endl;
        throw;
    }
}

bool MemUpdateStorage::isNewlyCreated() const {
    return newly_created;
}

void MemUpdateStorage::assertSameDb() {
    assert (SmiEnvironment::IsDatabaseOpen());
    assert (currentDbName == SmiEnvironment::CurrentDatabase());
}

MemUpdateStorage::~MemUpdateStorage() {
    cout <<  "MemUpdateStorage::~MemUpdateStorage()\n";

    cout << "contents of SharedDataVector:\n";
    if (memdata) {
        SharedDataVector::iterator it;
        for (it=memdata->data.begin(); it != memdata->data.end(); ++it) {
            cout << *it << endl;
        }
    }

    memdata->numOfUsers--;

    if (memdata->numOfUsers == 0) {
        cout << "we are the last remaining user, cleaning up...\n";
        segment.destroy<MemData>(shMemDataName.c_str());
        cout << "SharedDataVector destroyed\n";
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

    initMem();
    memdata->numOfUsers++;
}



const MemStorageId MemUpdateStorage::createMem() {
    cout << "MemUpdateStorage::createMem()\n";
    assertSameDb();
    // TODO: log handing of ID (as we may not always memAppend with it)
    MemStorageId newId = ++(memdata->lastKnownStorageId);
    cout << "createMem():" << newId << endl;
    return newId;
}

void MemUpdateStorage::createMem(const MemStorageId id) {
    cout << "MemUpdateStorage::createMem(" << id << ")\n";
    assertSameDb();
    assert (id > memdata->lastKnownStorageId);
    memdata->lastKnownStorageId = id;
}

Units
MemUpdateStorage::memGet (const MemStorageId id) /*const*/ {
    cout << "MemUpdateStorage::memGet(" << id << ")\n";
    assertSameDb();
    Units res;
    cout << "search shared memory\n";
    SharedDataVector::iterator it;
    for (it =  memdata->data.begin(); it !=  memdata->data.end(); ++it) {
        if (it->storageId == id && it->isValid) {
            res.push_back(it->unit);
        }
    }
    cout << "found " << res.size() << " entries\n";

    return res;
}

void MemUpdateStorage::memAppend
(MemStorageId id, const temporalalgebra::UPoint& unit) {
    cout << "MemUpdateStorage::memAppend(" << id << ", " << unit << ")\n";
    assertSameDb();
    memdata->data.push_back (memData(id, 0, true, unit, LogOp_memAppend));
}

void MemUpdateStorage::memClear (const MemStorageId id) {
    cout << "MemUpdateStorage::memClear(" << id << ")\n";
    assertSameDb(); // probably can be removed if we check for id=0;
    SharedDataVector::iterator it;
    for (it =  memdata->data.begin(); it !=  memdata->data.end(); ++it) {
        if (it->storageId == id) {
            it->isValid = false;
        }
    }
}



} /* namespace temporal2algebra */
