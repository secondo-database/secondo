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
    smifile = new SmiRecordFile (true, sizeof(LogData));
}

DbUpdateLogger::~DbUpdateLogger() {
    delete smifile;
}

void DbUpdateLogger::logCreateId(const MemStorageId id) {
    LogData log(id, 0, LogOp_memCreateId);
    writeLogRecord(log);
}

void DbUpdateLogger::logSetBackRef(const MemStorageId& id,
        const BackReference& backRef,
        const Unit& finalUnit) {
    LogData log(id, 0, backRef, finalUnit, LogOp_memSetBackRef);
    writeLogRecord(log);
}

void DbUpdateLogger::logAppend(const MemStorageId id, const Unit& unit) {
    LogData log(id, 0, unit, LogOp_memAppend);
    writeLogRecord(log);
}

void DbUpdateLogger::logClear (const MemStorageId id) {
   LogData log(id, 0, LogOp_memClear);
   writeLogRecord(log);
}

void DbUpdateLogger::logPushToFlobs(const MemStorageId id){
    LogData log(id, 0, LogOp_pushToFlobs);
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
        LogData* myLog = (LogData*)(rec.GetData(sz));
        cout << *myLog << endl;
        manager.applyLog(*myLog);
        delete myLog;
    }
    smifile->Close();
}

void DbUpdateLogger::truncateLog() {

    cout << "DbUpdateLogger::truncateLog()\n";
    bool res = smifile->Truncate();
    cout << "res: " << res << endl;
}

int DbUpdateLogger::printLog() {
    cout << "DbUpdateLogger::printLog()\n";
    smifile->Open(smiLogFileName);
    SmiRecordFileIterator iterator;
    smifile->SelectAll(iterator);

    int count(0);
    SmiRecord rec;
    while (iterator.Next(rec)) {
        SmiSize sz;
        LogData* myLog = (LogData*)(rec.GetData(sz));
        cout << *myLog << endl;
        delete myLog;
        ++count;
    }
    smifile->Close();
    return count;
}

void DbUpdateLogger::writeLogRecord(const LogData& val) {
    smifile->Open(smiLogFileName);
    SmiRecord record;
    SmiRecordId rec_id;
    smifile->AppendRecord(rec_id, record);

    size_t offset = 0;
    record.Write(&val,sizeof(LogData),offset);
    offset += sizeof(LogData);

    record.Finish();
    // Is there a better way? Is closing a problem anyway?
    smifile->Close();
}

} /* namespace temporal2algebra */
