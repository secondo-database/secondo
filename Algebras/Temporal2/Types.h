/*
some types used in different places of the temporal2algebra

*/

#ifndef ALGEBRAS_TEMPORAL2_TYPES_H_
#define ALGEBRAS_TEMPORAL2_TYPES_H_


#include "Algebras/Temporal/TemporalAlgebra.h"

namespace temporal2algebra {

typedef long MemStorageId;
typedef std::vector<MemStorageId> MemStorageIds;
typedef long TransactionId;
typedef temporalalgebra::UPoint Unit; //make this a template param

struct FlatUnit {
    FlatUnit(const Unit& unit) {
        defined = unit.IsDefined();
        if (!defined)
            return;
        x0 = unit.p0.GetX();
        y0 = unit.p0.GetY();
        x1 = unit.p1.GetX();
        y1 = unit.p1.GetY();
        t0 = unit.timeInterval.start.ToDouble();
        t1 = unit.timeInterval.end.ToDouble();
        lc = unit.timeInterval.lc;
        rc = unit.timeInterval.rc;
    }

    void createUnit(Unit* result) const {
        if (!defined) {
            *result = Unit(0);
            result->SetDefined(false);
            return;
        }
        temporalalgebra::Interval<Instant> i (
                Instant(t0), Instant(t1), lc, rc);

        *result = Unit(i ,x0, y0, x1, y1);
        result->SetDefined(true);
    }

    bool defined;
    double x0;
    double y0;
    double x1;
    double y1;
    double t0;
    double t1;
    bool lc;
    bool rc;
};

typedef temporalalgebra::IPoint Intime;
typedef std::vector<FlatUnit> Units;

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
    : operation(operation),
      storageId(st),
      transactionId(tr),
      backReference(),
      flatUnit(unit) {};

    LogData(const MemStorageId st,
            const TransactionId tr,
            LogOperation operation)
    : operation(operation),
      storageId(st),
      transactionId(tr),
      backReference(),
      flatUnit(Unit(0)) {};

    LogData(const MemStorageId st,
            const TransactionId tr,
            const BackReference& backRef,
            const Unit& finalUnit,
            LogOperation operation)
    : operation(operation),
      storageId(st),
      transactionId(tr),
      backReference(backRef),
      flatUnit(finalUnit){};

    LogData(const TransactionId tr,
            const BackReference& backRef,
            LogOperation operation)
    : operation(operation),
      storageId(0),
      transactionId(tr),
      backReference(backRef),
      flatUnit(Unit(0)) {};

    LogData()
    : operation(LogOp_pushToFlobs),
      storageId(0),
      transactionId(0),
      backReference(),
      flatUnit(Unit(0)){};

    LogOperation  operation;
    MemStorageId  storageId;
    TransactionId transactionId;
    BackReference backReference;
    FlatUnit flatUnit;

    void createUnit(Unit* result) const {
        flatUnit.createUnit(result);
    }
};

struct QueueData2 {
    bool lastElement;
    TupleId tid;
    double x;
    double y;
    double t;
};

std::ostream &operator<<(std::ostream &os, BackReference const &l);
std::ostream &operator<<(std::ostream &os, LogOperation const &l);
std::ostream &operator<<(std::ostream &os, LogData const &l);
std::ostream &operator<<(std::ostream &os, QueueData2 const &l);

} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_TYPES_H_ */
