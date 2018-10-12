/*
Temporal2Algebra - developed for bachelor thesis by Simon Jacobi in 2018
type mpoint2 has optional in-memory representation of data
operator appendpositions adds date to mpoint2's in-memory data (quicker update)
operators streamvalve/streamnext, barrier enable synchronization of test runs
operators enterwormhole/leavewormhole work around crash issue:
  intermediate commits of appendpositions crashes feed operator
other operators are "copied" from TemporalAlgebra to demonstrate re-use

Now just include all the separate Operators and initialize the algebra

*/
#include "Algebra.h"

#include "TemporalAlgebraOperators.h"

#include "MemStorageManager.h"

#include "MPoint2.h"

#include "OpStreamNext.h"
#include "OpStreamValve.h"
#include "OpM2MM.h"
#include "OpMM2M.h"
#include "OpBarrier.h"
#include "OpAppendPositions.h"
#include "OpWormHole.h"

namespace temporal2algebra{

class Temporal2Algebra : public Algebra
{
public:
    Temporal2Algebra() : Algebra()
{
        // There is no callback available on database open/close
        // so, to allow cleanup of MemStorageManager bind its lifetime
        // to lifetime of the algebra
        MemStorageManager::createInstance();

        AddTypeConstructor( getMPoint2TypePtr(), true );

        AddOperator( getStreamValveOpPtr(), true );
        AddOperator( getStreamNextOpPtr(), true );
        AddOperator( getM2MMOpPtr(), true );
        AddOperator( getMM2MOpPtr(), true );
        AddOperator( getBarrierOpPtr(), true );
        AddOperator( getAppendPositionsOpPtr(), true );

        AddOperator( getEnterWormHoleOpPtr(), true );
        AddOperator( getLeaveWormHoleOpPtr(), true );

        // Sample Operators copied from TemporalAlgebra:
        // demonstrating the refactoring approach for operator reuse

        AddOperator( getAtperiodsOpPtr(), true );
        AddOperator( getTrajectoryOpPtr(), true );
        AddOperator( getBBoxOpPtr(), true );
        AddOperator(getTranslateappendSOpPtr(), true );

}
    ~Temporal2Algebra() {
        MemStorageManager::deleteInstance();
    }
};
} // end of namespace temporal2algebra


// just forward declare NestedList/QueryProcessor
// we don't need them here anyway
class NestedList;
class QueryProcessor;

extern "C"
Algebra*
InitializeTemporal2Algebra(NestedList *nl, QueryProcessor *qp)
{
    return (new temporal2algebra::Temporal2Algebra());
}
