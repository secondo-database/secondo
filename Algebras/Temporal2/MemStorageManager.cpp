/*
implementation of MemStorageManager

*/

#include "MemStorageManager.h"
#include "MemUpdateStorage.h"
#include "DbUpdateLogger.h"
#include "SecondoSMI.h" // SmiEnvironment::IsDatabaseOpen()
#include <boost/interprocess/sync/scoped_lock.hpp>

#include "MPoint2.h" // TODO: remove dependency - see applyLog

namespace temporal2algebra {
using boost::interprocess::scoped_lock;
using boost::interprocess::named_mutex;

MemStorageManager::MemStorageManager() :
                      lastUsedDatabase (""),
                      storage_create_guard(boost::interprocess::open_or_create,
                                "mtx_MemStorageManager_CreateGuard"),
                                smiLogFileName("logfile")
{
    cout << "MemStorageManager::MemStorageManager()\n";
}

MemStorageManager::~MemStorageManager() {
    cout << "MemStorageManager::~MemStorageManager()\n";
}

//static
MemStorageManager* MemStorageManager::instance = 0;

//static
MemStorageManager* MemStorageManager::getInstance() {
    cout << "MemStorageManager::getInstance()\n";
    assert(instance);
    return instance;
}

//static
void MemStorageManager::createInstance() {
    cout << "MemStorageManager::createInstance()\n";
    assert(!instance);
    instance = new MemStorageManager();
}

//static
void MemStorageManager::deleteInstance() {
    cout << "MemStorageManager::deleteInstance()\n";
    assert(instance);
    delete instance;
}

const MemStorageId MemStorageManager::createId() {
    ensureStorageConnection();
    MemStorageId storageId = memUpdateStoragePtr->memCreateId();
    dbUpdateLoggerPtr->logCreateId(storageId);
    return storageId;
}

void MemStorageManager::setBackRef(const MemStorageId& id,
        const BackReference& backRef, const Unit& finalUnit) {
    ensureStorageConnection();

    try {
        memUpdateStoragePtr->memSetBackRef(id, backRef, finalUnit);
    }
    catch (const boost::interprocess::interprocess_exception ex) {
        // Memory full. Clean up then try again...
        cout << "MemStorageManager::setBackRef - Memory full. Pushing.\n";
        cout << ex.what() << endl;
        pushToFlobs(0);
        memUpdateStoragePtr->memSetBackRef(id, backRef, finalUnit);
    }

    dbUpdateLoggerPtr->logSetBackRef(id, backRef, finalUnit);
}

bool MemStorageManager::hasMemoryUnits(const MemStorageId id) {
    ensureStorageConnection();
    return memUpdateStoragePtr->memHasMemoryUnits(id);
}

Unit MemStorageManager::Get(const MemStorageId id, const size_t memIndex)
/*const*/
{
    ensureStorageConnection();
    Unit result = memUpdateStoragePtr->memGet(id, memIndex);
    return result;
}

Unit MemStorageManager::getFinalUnit (const MemStorageId id) {
    ensureStorageConnection();
    Unit result = memUpdateStoragePtr->memGetFinalUnit(id);
    return result;
}

MemStorageId MemStorageManager::getId(const BackReference& backRef)
{
    ensureStorageConnection();
    return memUpdateStoragePtr->memGetId(backRef);
}

int MemStorageManager::Size(const MemStorageId id) /*const*/
{
    ensureStorageConnection();
    //narrowing cast!
    int result = memUpdateStoragePtr->memSize(id);
    return result;
}

void MemStorageManager::append(const MemStorageId id, const Unit& unit){
    ensureStorageConnection();
    try {
        memUpdateStoragePtr->memAppend(id, unit);
    }
    catch (const boost::interprocess::interprocess_exception ex) {
        // Memory full. Clean up then try again...
        cout << "MemStorageManager::append - Memory full. Pushing.\n";
        cout << ex.what() << endl;
        BackReference backRef = memUpdateStoragePtr->getBackReference(id);
        pushToFlobs(id);
        setBackRef(id, backRef, unit);
        memUpdateStoragePtr->memAppend(id, unit);
    }


    dbUpdateLoggerPtr->logAppend(id, unit);
}

void MemStorageManager::clear (const MemStorageId id){
    ensureStorageConnection();
    memUpdateStoragePtr->memClear(id);
    dbUpdateLoggerPtr->logClear(id);
}

int MemStorageManager::pushToFlobs(MemStorageId id_to_keep) {
    cout << "MemStorageManager::pushToFlobs()\n";
    ensureStorageConnection();
    MemStorageIds idsToPush = memUpdateStoragePtr->getIdsToPush();
    MemStorageIds::iterator id_it;
    int countPushed(0);
    for (id_it = idsToPush.begin(); id_it != idsToPush.end(); ++id_it) {
        cout << "pushing id: " << *id_it << endl;
        bool keep_id = (id_to_keep == *id_it)?true:false;
        countPushed += memUpdateStoragePtr->memPushToFlobs(*id_it, keep_id);
        dbUpdateLoggerPtr->logPushToFlobs(*id_it);
    }
    // hack: just create the log, so we have something to delete
    dbUpdateLoggerPtr->logCreateId(0);

    dbUpdateLoggerPtr->truncateLog();

    // log latest known id +1 to avoid conflict with existing ids
    MemStorageId dummyId = memUpdateStoragePtr->memCreateId();
    dbUpdateLoggerPtr->logCreateId(dummyId);

    return countPushed;
}

void MemStorageManager::applyLog (const LogData& log){
    cout << "MemStorageManager::applyLog("
            << log << ")\n";
    Unit unit;
    log.createUnit(&unit);
    switch (log.operation) {
    case LogOp_memCreateId:
        memUpdateStoragePtr->memCreateId(log.storageId);
        break;
    case LogOp_memSetBackRef:
        memUpdateStoragePtr->memSetBackRef(log.storageId,
                log.backReference, unit );
        break;
    case LogOp_memAppend:
        memUpdateStoragePtr->memAppend(log.storageId, unit);
        break;
    case LogOp_memClear:
        memUpdateStoragePtr->memClear(log.storageId);
        break;
    case LogOp_pushToFlobs:
        memUpdateStoragePtr->memClear(log.storageId);
        break;
    default:
        cout << "unhandled operation\n";
        assert(false);
    }
}

void MemStorageManager::ensureStorageConnection() {
    cout << "MemStorageManager::ensureStorageConnection()\n";
    if (!SmiEnvironment::IsDatabaseOpen()) {
        cout << "Didn't expect this: No Database Open!\n";
        assert(false); // can't read the log...
    }
    std::string currentDatabase = SmiEnvironment::CurrentDatabase();
    if (lastUsedDatabase == currentDatabase) {
        // The memUpdateStoragePtr should still be valid, nothing to do.
        return;
    }
    lastUsedDatabase = currentDatabase;
    {
        scoped_lock<named_mutex>(storage_create_guard);

        dbUpdateLoggerPtr = DbUpdateLoggerPtr(
                new DbUpdateLogger(lastUsedDatabase, smiLogFileName));
        memUpdateStoragePtr = MemUpdateStoragePtr(
                new MemUpdateStorage(lastUsedDatabase));
        if (memUpdateStoragePtr->isNewlyCreated()) {
            cout << "MemUpdateStorage has not been initialized. Read log!\n";
            dbUpdateLoggerPtr->replayLog(*this);
        } else{
            cout << "MemUpdateStorage should be fine. Go on.\n";
        }
    }
}

} /* namespace temporal2algebra */
