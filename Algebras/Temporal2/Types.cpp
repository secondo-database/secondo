/*
Types.cpp
Created on: 26.05.2018
Author: simon

*/

#include "Types.h"

namespace temporal2algebra {

std::ostream &operator<<(std::ostream &os, LogOperation const &l) {
    switch (l) {
    case LogOp_memCreateId:
        return os << "LogOp_memCreateId";
    case LogOp_memAppend:
        return os << "LogOp_memAppend";
    case LogOp_memGet:
        return os << "LogOp_memGet";
    case LogOp_memClear:
        return os << "LogOp_memClear";
    default:
        return os << "LogOperation: no name for value '"
                << (int)l << "'";
    }
}

std::ostream &operator<<(std::ostream &os, logData const &l) {
    return os << "[op: " << l.operation
            << ", store: " << l.storageId
            << ", trans: " << l.transactionId
            << ", " << l.unit << "]";
}



} /* namespace temporal2algebra */
