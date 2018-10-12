/*
implementation of output methods for most of our types

*/

#include "Types.h"

namespace temporal2algebra {

std::ostream &operator<<(std::ostream &os, BackReference const &l) {
    return os << "[rel: " << l.relationName
            << ", tup: " << l.tupleId
            << ", pos: " << l.attrPos << "]";
}

std::ostream &operator<<(std::ostream &os, LogOperation const &l) {
    switch (l) {
    case LogOp_memCreateId:
        return os << "LogOp_memCreateId";
    case LogOp_memSetBackRef:
        return os << "LogOp_memSetBackRef";
    case LogOp_memAppend:
        return os << "LogOp_memAppend";
    case LogOp_memClear:
        return os << "LogOp_memClear";
    case LogOp_pushToFlobs:
        return os << "LogOp_pushToFlobs";
    default:
        return os << "LogOperation: no name for value '"
                << (int)l << "'";
    }
}

std::ostream &operator<<(std::ostream &os, LogData const &l) {
    Unit unit;
    l.flatUnit.createUnit(&unit);
    return os << "[op: " << l.operation
            << ", store: " << l.storageId
            << ", trans: " << l.transactionId
            << ", backref: " << l.backReference
            << ", unit: " << unit << "]";
}

std::ostream &operator<<(std::ostream &os, QueueData2 const &l) {
    return os << "[lastElement: " << l.lastElement
            << ", tid: " << l.tid
            << ", intime: " << l.t << ", " << l.x << ", " << l.y << "]";
}

} /* namespace temporal2algebra */
