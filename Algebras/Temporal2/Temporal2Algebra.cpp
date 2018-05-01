/*
Just include all the separate Operators and initialize the algebra

*/

#include "Algebra.h"
#include "Temporal2Algebra.h"

#include "OpExtendWith.h"
#include "OpStreamNext.h"
#include "OpStreamValve.h"

namespace temporal2algebra{

class Temporal2Algebra : public Algebra
{
  public:
    Temporal2Algebra() : Algebra()
    {
        AddOperator( getExtendWithOpPtr(), true );
        AddOperator( getStreamValveOpPtr(), true );
        AddOperator( getStreamNextOpPtr(), true );
    }
    ~Temporal2Algebra() {}
};

} // end of namespace temporal2algebra


// just forward declare NestedList/QueryProcessor
// we don't need them here anyway
class NestedList;
class QueryProcessor;

extern "C"
Algebra*
InitializeTemporal2Algebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  return (new temporal2algebra::Temporal2Algebra());
}
