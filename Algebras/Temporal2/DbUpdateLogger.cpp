/*
DbUpdateLogger.cpp
Created on: 26.05.2018
Author: simon

*/

#include "DbUpdateLogger.h"


namespace temporal2algebra {

DbUpdateLogger::DbUpdateLogger(std::string database,
        std::string smiLogFileName) :
                database(database),
                smiLogFileName(smiLogFileName){
    smifile = new SmiRecordFile (true, sizeof(logData));
}

DbUpdateLogger::~DbUpdateLogger() {
    delete smifile;
}


void DbUpdateLogger::logCreateId(const MemStorageId id) {
    logData log(id, 0, LogOp_memCreateId);
    writeLogRecord(log);
}

void DbUpdateLogger::logGet(const MemStorageId id) {
    logData log(id, 0, LogOp_memGet);
    writeLogRecord(log);
}

void DbUpdateLogger::logAppend(const MemStorageId id, const Unit& unit) {
    logData log(id, 0, unit, LogOp_memAppend);
    writeLogRecord(log);
}

void DbUpdateLogger::logClear (const MemStorageId id) {
   logData log(id, 0, LogOp_memClear);
   writeLogRecord(log);
}


void DbUpdateLogger::replayLog(MemStorageManager& manager) {
    cout << "DbUpdateLogger::replayLog()\n";
    smifile->Open(smiLogFileName);
    SmiRecordFileIterator iterator;
    smifile->SelectAll(iterator);

    SmiRecord rec;
    while (iterator.Next(rec)) {
        SmiSize sz;
        logData* myLog = (logData*)(rec.GetData(sz));
        cout << *myLog << endl;
        manager.applyLog(*myLog);
        delete myLog;
    }
    smifile->Close();
}

void DbUpdateLogger::writeLogRecord(const logData& val) {
    smifile->Open(smiLogFileName);
    SmiRecord record;
    SmiRecordId rec_id;
    smifile->AppendRecord(rec_id, record);

    size_t offset = 0;
    record.Write(&val,sizeof(logData),offset);
    offset += sizeof(logData);

    record.Finish();
    // Is there a better way? Is closing a problem anyway?
    smifile->Close();
}

} /* namespace temporal2algebra */
