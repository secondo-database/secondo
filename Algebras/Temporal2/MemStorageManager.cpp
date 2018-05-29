/*
MemStorageManager.cpp
Created on: 22.05.2018
Author: simon

*/

#include "MemStorageManager.h"
#include "MemUpdateStorage.h"
#include "DbUpdateLogger.h"
#include "SecondoSMI.h" // SmiEnvironment::IsDatabaseOpen()
#include <boost/interprocess/sync/scoped_lock.hpp>

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


const MemStorageId MemStorageManager::createId(){
    ensureStorageConnection();
    MemStorageId newId = memUpdateStoragePtr->createMem();
    dbUpdateLoggerPtr->logCreateId(newId);
    return newId;
}

Units MemStorageManager::get(const MemStorageId id)/*const*/ {
    ensureStorageConnection();
    Units result = memUpdateStoragePtr->memGet(id);
    dbUpdateLoggerPtr->logGet(id);
    return result;
}

void MemStorageManager::append(const MemStorageId id, const Unit& unit){
    ensureStorageConnection();
    memUpdateStoragePtr->memAppend(id, unit);
    dbUpdateLoggerPtr->logAppend(id, unit);
}

void MemStorageManager::clear (const MemStorageId id){
    ensureStorageConnection();
    memUpdateStoragePtr->memClear(id);
    dbUpdateLoggerPtr->logClear(id);
}



void MemStorageManager::applyLog (const logData& log){
    cout << "MemStorageManager::applyLog("
            << log << ")\n";
    switch (log.operation) {
        case LogOp_memCreateId:
            memUpdateStoragePtr->createMem(log.storageId);
            break;
        case LogOp_memAppend:
            memUpdateStoragePtr->memAppend(log.storageId, log.unit);
            break;
        case LogOp_memGet:
            // nothing to do
            break;
        case LogOp_memClear:
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

        dbUpdateLoggerPtr->replayLog(*this);
    }
}

} /* namespace temporal2algebra */
