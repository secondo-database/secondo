/*
DbUpdateLogger.h
Created on: 26.05.2018
Author: simon

*/

#ifndef ALGEBRAS_TEMPORAL2_DBUPDATELOGGER_H_
#define ALGEBRAS_TEMPORAL2_DBUPDATELOGGER_H_

#include <string>
#include "Types.h"
#include "SecondoSMI.h"

// required for replay log
// bad design: refactor to use callback!
#include "MemStorageManager.h"

namespace temporal2algebra {

class DbUpdateLogger {
public:
    DbUpdateLogger(std::string database, std::string smiLogFileName);
    virtual ~DbUpdateLogger();

    void logCreateId(const MemStorageId id);
    void logGet(const MemStorageId id) /*const*/;
    void logAppend(const MemStorageId id, const Unit& unit);
    void logClear (const MemStorageId id);

    void replayLog(MemStorageManager& manager);


private:
    void writeLogRecord(const logData& log);

    std::string database;
    std::string smiLogFileName;
    SmiRecordFile* smifile;

};

} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_DBUPDATELOGGER_H_ */
