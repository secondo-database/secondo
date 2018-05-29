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
    typedef long TransactionId;
    typedef temporalalgebra::UPoint Unit;

    enum LogOperation { LogOp_memCreateId,
        LogOp_memAppend, LogOp_memGet, LogOp_memClear };

    struct logData {
        logData(const MemStorageId st,
                const TransactionId tr,
                const Unit unit,
                LogOperation operation)
            : storageId(st),
              transactionId(tr),
              unit(unit),
              operation(operation){};

        logData(const MemStorageId st,
                const TransactionId tr,
                LogOperation operation)
            : storageId(st),
              transactionId(tr),
              operation(operation){};

        MemStorageId storageId;
        TransactionId transactionId;

        Unit    unit;

        LogOperation operation;
    };
    std::ostream &operator<<(std::ostream &os, LogOperation const &l);
    std::ostream &operator<<(std::ostream &os, logData const &l);

} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_TYPES_H_ */
