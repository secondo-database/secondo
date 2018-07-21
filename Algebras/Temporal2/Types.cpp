/*
Types.cpp
Created on: 26.05.2018
Author: simon

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
    return os << "[op: " << l.operation
            << ", store: " << l.storageId
            << ", trans: " << l.transactionId
            << ", backref: " << l.backReference
            << ", unit: " << l.unit
            << ", intime: " << l.intime << "]";
}

std::ostream &operator<<(std::ostream &os, QueueData2 const &l) {
    return os << "[lastElement: " << l.lastElement
            << ", tid: " << l.tid
            << ", intime: " << l.intime << "]";
}

} /* namespace temporal2algebra */
