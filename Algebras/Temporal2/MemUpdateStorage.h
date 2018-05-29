/*
MemUpdateStorage.h
Created on: 06.05.2018
Author: simon

Limitations and ToDos:
* Implement
* How to Handle Lifecycle:
-> within a single process - shared-ptr
--> If possible create once at secondo startup and destruct on secondo exit
--> How to avoid Memory leak (how/when destruct Singleton)
-> of shared Mem structure between processes?
--> shared memory shared-ptr available?
--> Implement?!

*/



/*
Problematic lifecycle/missing hooks:
- Start Secondo
- Call MMPoint ctor -> no DB open yet...
? Can we ensure to always have other ctor called with open DB??
- Open DB
? is there really no hook available ??
* Here we need to recover from the logs:
- a) Query/Update MMPoint -> Read from DB
- b) Create MMPoint -> Write to DB
- c) Delete MMPoint (but would first read from DB)
- Some more queries/updates/inserts/deletes
- Close DB
? Once again: a hook would be nice, e.g. distribute log to MMPoints
* But might do this during the next recovery?!

*/

#ifndef ALGEBRAS_TEMPORAL2_MEMUPDATESTORAGE_H_
#define ALGEBRAS_TEMPORAL2_MEMUPDATESTORAGE_H_

#include <map>
#include <vector>
#include "Algebras/Temporal/TemporalAlgebra.h"


#include "Types.h"

//#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_upgradable_mutex.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
//#include <boost/interprocess/sync/interprocess_mutex.hpp>
//#include <boost/interprocess/sync/interprocess_condition.hpp>
//#include <boost/interprocess/shared_memory_object.hpp>
//#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

namespace temporal2algebra {



typedef std::vector<Unit> Units;



struct memData {
    memData(const MemStorageId st,
            const TransactionId tr,
            bool valid,
            const Unit unit,
            LogOperation operation) :
                storageId(st),
                transactionId(tr),
                isValid(valid),
                unit(unit),
                operation(operation){};
    memData(const logData log) :
        storageId(log.storageId),
        transactionId(log.transactionId),
        isValid(true),
        unit(log.unit),
        operation(log.operation){};

    MemStorageId storageId;
    TransactionId transactionId;
    bool    isValid; // allow to mark entries as obsolete
    Unit    unit;
    LogOperation operation;
};

std::ostream &operator<<(std::ostream &os, memData const &l);

typedef boost::interprocess::allocator
        <memData, boost::interprocess::managed_shared_memory::segment_manager>
        ShmemAllocator;
typedef boost::container::vector<memData, ShmemAllocator> SharedDataVector;

struct MemData {
    // number of users
    // access only in ctor/dtor, synchronize access externally
    int numOfUsers;

    MemStorageId lastKnownStorageId;
    SharedDataVector data;
    MemData(const ShmemAllocator& alloc):
        numOfUsers(0),
        lastKnownStorageId(0),
        data(alloc) {};
};


class MemUpdateStorage {
public:
    MemUpdateStorage(std::string database);

    virtual ~MemUpdateStorage();

    // these 4 methods are backed up by log:
    const MemStorageId createMem();
    void createMem(const MemStorageId id); //only use during logReplay
    Units memGet(const MemStorageId id) /*const*/;
    void memAppend(const MemStorageId id, const Unit& unit);
    void memClear (const MemStorageId id);

private:
    void assertSameDb();
    void initMem();

    boost::interprocess::managed_shared_memory  segment;

    MemData* memdata;

    // Name of currently open DB - used in identifier for mem
    std::string currentDbName;
    std::string memStorageName;
    std::string shMemDataName;
};

} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_MEMUPDATESTORAGE_H_ */
