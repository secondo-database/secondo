/*
Types.h
Created on: 26.05.2018
Author: simon

*/

#ifndef ALGEBRAS_TEMPORAL2_TYPES_H_
#define ALGEBRAS_TEMPORAL2_TYPES_H_


#include "Algebras/Temporal/TemporalAlgebra.h"

namespace temporal2algebra {

    typedef long MemStorageId;
    typedef std::vector<MemStorageId> MemStorageIds;
    typedef long TransactionId;
    typedef temporalalgebra::UPoint Unit; //make this a template param
    typedef temporalalgebra::IPoint Intime; //make this a template param
    typedef std::vector<Unit> Units;

    const size_t MaxRelNameLen = MAX_STRINGSIZE;

    struct BackReference {
        BackReference(
            std::string relName,
            const TupleId tupleId,
            const int attrPosition) :
                tupleId(tupleId),
                attrPos(attrPosition) {
            initRelationName(relName);
        };
        BackReference() :
                tupleId(0),
                attrPos(0) {
            initRelationName("");
        };
        char relationName[MaxRelNameLen + 1];
        TupleId tupleId;
        int attrPos;

        bool operator==(const BackReference& rhs) {
            return (tupleId == rhs.tupleId
                    && attrPos == rhs.attrPos
                    && (strcmp(relationName, rhs.relationName) == 0)
            );
        }

    private:
        void initRelationName(const std::string relNameString) {
            strncpy(relationName, relNameString.c_str(), MaxRelNameLen + 1);
            if (relationName[MaxRelNameLen] != '\0') {
                relationName[MaxRelNameLen] = '\0';
                cout << "relationName '" << relNameString
                     << "' is too long. Would truncate to '"
                     << relationName << "'\n";
                assert (false);
            }
        }
    };

    enum LogOperation { LogOp_memCreateId, LogOp_memSetBackRef,
          LogOp_memAppend, LogOp_memClear,  LogOp_pushToFlobs};

    struct LogData {
        LogData(const MemStorageId st,
                const TransactionId tr,
                const Unit unit,
                LogOperation operation)
            : storageId(st),
              transactionId(tr),
              backReference(),
              unit(unit),
              operation(operation){};

        LogData(const MemStorageId st,
                const TransactionId tr,
                LogOperation operation)
            : storageId(st),
              transactionId(tr),
              backReference(),
              operation(operation) {};

        LogData(const MemStorageId st,
                const TransactionId tr,
                const BackReference& backRef,
                const Unit& finalUnit,
                LogOperation operation)
            : storageId(st),
              transactionId(tr),
              backReference(backRef),
              unit(finalUnit),
              operation(operation) {};

        LogData(const TransactionId tr,
                const BackReference& backRef,
                const Intime intime,
                LogOperation operation)
            : storageId(0),
              transactionId(tr),
              backReference(backRef),
              intime(intime),
              operation(operation) {};

        MemStorageId  storageId;
        TransactionId transactionId;
        BackReference backReference;
        Unit          unit;
        Intime        intime;
        LogOperation  operation;
    };

    struct QueueData2 {
        bool lastElement;
        TupleIdentifier tid;
        Intime intime;
    };

    std::ostream &operator<<(std::ostream &os, BackReference const &l);
    std::ostream &operator<<(std::ostream &os, LogOperation const &l);
    std::ostream &operator<<(std::ostream &os, LogData const &l);
    std::ostream &operator<<(std::ostream &os, QueueData2 const &l);

} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_TYPES_H_ */
