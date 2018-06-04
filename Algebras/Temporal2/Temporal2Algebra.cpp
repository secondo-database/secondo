/*
Just include all the separate Operators and initialize the algebra

Limitations and ToDos:
- check if MemUpdateStorage (de-)construction can move to type constructor

*/
#include "Algebra.h"
#include "Temporal2Algebra.h"

#include "MemUpdateStorage.h"
#include "MemStorageManager.h"

#include "MPoint2.h"

#include "OpAppendTo.h"
#include "OpStreamNext.h"
#include "OpStreamValve.h"
#include "OpM2MM.h"
#include "OpMM2M.h"
#include "OpBarrier.h"

#include "GenericTC.h"

namespace temporal2algebra{

GenTC<MPoint2> mpoint2;

class Temporal2Algebra : public Algebra
{
  public:
    Temporal2Algebra() : Algebra()
    {
        MemStorageManager::createInstance();

        //AddTypeConstructor( getMPoint2TypePtr(), true );
        AddTypeConstructor( & mpoint2 );
        mpoint2.AssociateKind( Kind::TEMPORAL() );
        mpoint2.AssociateKind( Kind::DATA() );

        AddOperator( getAppendToOpPtr(), true );
        AddOperator( getStreamValveOpPtr(), true );
        AddOperator( getStreamNextOpPtr(), true );
        AddOperator( getM2MMOpPtr(), true );
        AddOperator( getMM2MOpPtr(), true );
        AddOperator( getBarrierOpPtr(), true );

    }
    ~Temporal2Algebra() {
        MemStorageManager::deleteInstance();
    }
  private:


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
