/*
wrapper around shared memory structure holding updates for MPoint2
each secondo-process has its own MemUpdateStorage to access the shared mem
for each open database exists a seperate shared mem

*/


#ifndef ALGEBRAS_TEMPORAL2_MEMUPDATESTORAGE_H_
#define ALGEBRAS_TEMPORAL2_MEMUPDATESTORAGE_H_

#include <map>
#include <vector>
#include "Algebras/Temporal/TemporalAlgebra.h"

#include "Types.h"

#include <boost/interprocess/sync/named_upgradable_mutex.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

#include <boost/interprocess/ipc/message_queue.hpp>


namespace temporal2algebra {

// change to adapte size of in-memory storage
const int storage_size = 65536;

typedef boost::interprocess::allocator
        <FlatUnit, boost::interprocess::managed_shared_memory::segment_manager>
UnitAllocator;

typedef boost::container::vector<FlatUnit, UnitAllocator> SharedUnits;

typedef boost::interprocess::allocator
        <void, boost::interprocess::managed_shared_memory::segment_manager>
VoidAllocator;

struct SharedData {
    SharedData(const BackReference& backref,
            const Unit& finalUnit,
            const VoidAllocator& alloc) :
                backReference(backref),
                finalUnit(finalUnit),
                units(alloc) {}

    BackReference backReference;
    FlatUnit finalUnit;
    SharedUnits units;
};

typedef std::pair<const MemStorageId, SharedData> SharedDataMapValue;

typedef boost::interprocess::allocator
        <SharedDataMapValue,
        boost::interprocess::managed_shared_memory::segment_manager>
MapValueAllocator;

typedef boost::container::map
        <MemStorageId, SharedData, std::less<MemStorageId>, MapValueAllocator>
SharedDataMap;

typedef boost::interprocess::allocator
        <SharedDataMap,
        boost::interprocess::managed_shared_memory::segment_manager>
MapAllocator;

struct SharedMemStorage {
    int numOfUsers;
    MemStorageId lastKnownStorageId;
    SharedDataMap dataMap;

    SharedMemStorage(const VoidAllocator& alloc):
        numOfUsers(0),
        lastKnownStorageId(0),
        dataMap(alloc){};
};

std::ostream &operator<<(std::ostream &os, SharedData const &l);
std::ostream &operator<<(std::ostream &os, SharedDataMapValue const &l);
std::ostream &operator<<(std::ostream &os, SharedMemStorage const &l);


class MemUpdateStorage {
public:
    MemUpdateStorage(std::string database);
    virtual ~MemUpdateStorage();

    //only use during logReplay
    void memCreateId(MemStorageId id);

    // these methods are backed up by log:
    const MemStorageId memCreateId();
    void memSetBackRef(const MemStorageId& id,
            const BackReference& backRef, const Unit& finalUnit);
    bool memHasMemoryUnits(const MemStorageId id) const;

    Unit memGet(const MemStorageId id, size_t memIndex);
    Unit memGetFinalUnit(const MemStorageId id);
    MemStorageId memGetId(const BackReference& backRef) const;
    int memSize(const MemStorageId id);
    void memAppend(const MemStorageId id, const Unit& unit);

    void memClear (const MemStorageId id);

    MemStorageIds getIdsToPush() const;
    const BackReference getBackReference(const MemStorageId id);

    int memPushToFlobs(const MemStorageId idToPush, bool keep_reference );

    bool isNewlyCreated() const;

private:
    void assertSameDb() const;
    void initMem();

    boost::interprocess::managed_shared_memory  segment;
    SharedMemStorage* memdata;

    boost::interprocess::message_queue* memqueue;

    // Name of currently open DB - used in identifier for mem
    std::string currentDbName;
    std::string memStorageName;
    std::string shMemDataName;
    std::string memQueueName;
    bool newly_created;
};

} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_MEMUPDATESTORAGE_H_ */
