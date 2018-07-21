/*
MemStorageManager.h
Created on: 22.05.2018
Author: simon

*/

#ifndef ALGEBRAS_TEMPORAL2_MEMSTORAGEMANAGER_H_
#define ALGEBRAS_TEMPORAL2_MEMSTORAGEMANAGER_H_

#include <tr1/memory> // std::tr1::shared_ptr
#include <string>
#include <boost/interprocess/sync/named_mutex.hpp>
// This should be obsolete once we create a template param..
#include "Types.h"

namespace temporal2algebra {

// The MemUpdateStorage does the actual allocation etc.
// We need to wrap it in order to correctly handle closing DB's
// We can detect if we still have the same DB open, but do not get notified
class MemUpdateStorage;
typedef std::tr1::shared_ptr<MemUpdateStorage> MemUpdateStoragePtr;

class DbUpdateLogger;
typedef std::tr1::shared_ptr<DbUpdateLogger> DbUpdateLoggerPtr;

class MemStorageManager {
public:
    // we need to Access the StorageManager from MPoint2 (and other classes)
    // But we cannot pass it directly... so rely on singleton.
    static MemStorageManager* getInstance(); // not thread safe!

    // Dedicated construction and deletion of instance
    // (To allow controlled creation/deletion of instance during algebra
    //  construction/destruction)
    static void createInstance();
    static void deleteInstance();

    virtual ~MemStorageManager();
    const MemStorageId createId();
    void setBackRef(const MemStorageId& id,
            const BackReference& backRef, const Unit& finalUnit);
    bool hasMemoryUnits(const MemStorageId id);
//    Units get(const MemStorageId id); /*const*/
    Unit Get(const MemStorageId id, size_t memIndex) ; /*const*/
    Unit getFinalUnit (const MemStorageId id); /*const*/
    MemStorageId getId(const BackReference& backRef);
    int Size(const MemStorageId id);
    void append(const MemStorageId id, const Unit& unit);

    void clear (const MemStorageId id);
    int pushToFlobs();
    int printLog();
    int printMem();

    void applyLog (const LogData& log);



protected:
    MemStorageManager();

private:
    // helper to make sure we still have the correct Storage
    // if not: cleanup old Storage and connect/create to new Storage
    void ensureStorageConnection();
//    MemStorageIds getIds();

private:
    // single instance to handle all client access
    static MemStorageManager* instance;
    // keep track of the previously used DB.
    // If we still have the same DB the MemUpdateStoragePtr remains valid
    std::string lastUsedDatabase;
    // synchronize creation of MemUpdateStorages:
    boost::interprocess::named_mutex storage_create_guard;
    // Pointer to MemUpdateStorage for currentDatabase"
    MemUpdateStoragePtr memUpdateStoragePtr;
    // Handle for transaction log in database
    DbUpdateLoggerPtr dbUpdateLoggerPtr;

    std::string smiLogFileName;
};

} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_MEMSTORAGEMANAGER_H_ */
