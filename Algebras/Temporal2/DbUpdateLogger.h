/*
logger used by MemStorageManager to log in-memory updates
uses SMI-File in database for logging.

*/

#ifndef ALGEBRAS_TEMPORAL2_DBUPDATELOGGER_H_
#define ALGEBRAS_TEMPORAL2_DBUPDATELOGGER_H_

#include <string>
#include "Types.h"
#include "SecondoSMI.h"

// required for replay log
// bad design: refactor to use callback interface/ allow registering callback
#include "MemStorageManager.h"

namespace temporal2algebra {

class DbUpdateLogger {
public:
    DbUpdateLogger(std::string database, std::string smiLogFileName);
    virtual ~DbUpdateLogger();

    void logCreateId(const MemStorageId id);
    void logSetBackRef(const MemStorageId& id,
            const BackReference& backRef, const Unit& finalUnit);

    void logAppend(const MemStorageId id, const Unit& unit);
    void logEnque(const BackReference& backRef, const Intime& intime);
    void logClear (const MemStorageId id);

    void logPushToFlobs(const MemStorageId id);

    void replayLog(MemStorageManager& manager);
    void truncateLog();
    int printLog();

private:
    void writeLogRecord(const LogData& log);

    std::string database;
    std::string smiLogFileName;
    SmiRecordFile* smifile;

};

} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_DBUPDATELOGGER_H_ */
