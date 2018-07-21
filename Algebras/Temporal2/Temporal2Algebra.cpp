/*
Just include all the separate Operators and initialize the algebra

*/
#include "Algebra.h"
#include "Temporal2Algebra.h"

#include "TemporalAlgebraOperators.h"

#include "MemStorageManager.h"

#include "MPoint2.h"

#include "OpAppendTo.h"
#include "OpStreamNext.h"
#include "OpStreamValve.h"
#include "OpM2MM.h"
#include "OpMM2M.h"
#include "OpBarrier.h"
#include "OpAppendPositions.h"
#include "OpStorageAction.h"
#include "OpWormHole.h"

namespace temporal2algebra{

class Temporal2Algebra : public Algebra
{
  public:
    Temporal2Algebra() : Algebra()
    {
        MemStorageManager::createInstance();

        AddTypeConstructor( getMPoint2TypePtr(), true );

        AddOperator( getAppendToOpPtr(), true );
        AddOperator( getStreamValveOpPtr(), true );
        AddOperator( getStreamNextOpPtr(), true );
        AddOperator( getM2MMOpPtr(), true );
        AddOperator( getMM2MOpPtr(), true );
        AddOperator( getBarrierOpPtr(), true );
        AddOperator( getAppendPositionsOpPtr(), true );
        AddOperator( getStorageActionOpPtr(), true );

        AddOperator( getEnterWormHoleOpPtr(), true );
        AddOperator( getLeaveWormHoleOpPtr(), true );

        // Operators copied from TemporalAlgebra:

//        AddOperator( getIsemptyOpPtr(), true );
//        AddOperator( getEqualOpPtr(), true );
//        AddOperator( getNotequalOpPtr(), true );
//        AddOperator( getEqual2OpPtr(), true );
//        AddOperator( getNotequal2OpPtr(), true );
//        AddOperator( getLessOpPtr(), true );
//        AddOperator( getLessequalOpPtr(), true );
//        AddOperator( getGreaterOpPtr(), true );
//        AddOperator( getGreaterequalOpPtr(), true );
//        AddOperator( getIntersectsOpPtr(), true );
//        AddOperator( getInsideOpPtr(), true );
//        AddOperator( getBeforeOpPtr(), true );
//        AddOperator( getIntersectionOpPtr(), true );
//        AddOperator( getUnionOpPtr(), true );
//        AddOperator( getMinusOpPtr(), true );
//        AddOperator( getMinOpPtr(), true );
//        AddOperator( getMaxOpPtr(), true );
//        AddOperator( getNocomponentsOpPtr(), true );
//
//        AddOperator( getInstOpPtr(), true );
//        AddOperator( getValOpPtr(), true );
//        AddOperator( getAtinstantOpPtr(), true );
        AddOperator( getAtperiodsOpPtr(), true );
//        AddOperator( getWhenOpPtr(), true );
//        AddOperator( getDeftimeOpPtr(), true );
        AddOperator( getTrajectoryOpPtr(), true );
//        AddOperator( getPresentOpPtr(), true );
//        AddOperator( getPassesOpPtr(), true );
//        AddOperator( getInitialOpPtr(), true );
//        AddOperator( getFinalOpPtr(), true );
//        AddOperator( getUnitsOpPtr(), true );
//        AddOperator( getGetunitOpPtr(), true );
//        AddOperator( getGetpositionOpPtr(), true );
        AddOperator( getBBoxOpPtr(), true );
//        AddOperator( getMbrangeOpPtr(), true );
//        AddOperator( getBbox2dOpPtr(), true );
//        AddOperator( getBboxoldOpPtr(), true );
//
//        AddOperator( getAtOpPtr(), true );
//        AddOperator( getDistanceOpPtr(), true );
//        AddOperator( getSimplifyOpPtr(), true );
//        AddOperator( getIntegrateOpPtr(), true );
//        AddOperator( getLinearizeOpPtr(), true );
//        AddOperator( getLinearize2OpPtr(), true );
//        AddOperator( getApproximateOpPtr(), true );
//        AddOperator( getMinimumOpPtr(), true );
//        AddOperator( getMaximumOpPtr(), true );
//        AddOperator( getBreakpointsOpPtr(), true );
//        AddOperator( getBreaksOpPtr(), true );
//        AddOperator( getGkOpPtr(), true );
//        AddOperator( getVerticesOpPtr(), true );
//        AddOperator( getTranslateOpPtr(), true );
//
//        AddOperator( getTheyearOpPtr(), true );
//        AddOperator( getThemonthOpPtr(), true );
//        AddOperator( getThedayOpPtr(), true );
//        AddOperator( getThehourOpPtr(), true );
//        AddOperator( getTheminuteOpPtr(), true );
//        AddOperator( getThesecondOpPtr(), true );
//        AddOperator( getTheperiodOpPtr(), true );
//        AddOperator( getTherangeOpPtr(), true );
//
//        AddOperator(getBox3dOpPtr(), true );
//        AddOperator(getBox2dOpPtr(), true );
//        AddOperator(getMbool2mintOpPtr(), true );
//        AddOperator(getMint2mboolOpPtr(), true );
//        AddOperator(getMint2mrealOpPtr(), true );
//        AddOperator(getExtdeftimeOpPtr(), true );
//        AddOperator(getTranslateappendOpPtr(), true );
        AddOperator(getTranslateappendSOpPtr(), true );
//        AddOperator(getReverseOpPtr(), true );
//        AddOperator(getSamplempointOpPtr(), true );
//        AddOperator(getGpsOpPtr(), true );
//        AddOperator(getDisturbOpPtr(), true );
//        AddOperator(getLengthOpPtr(), true );
//        AddOperator(getEqualizeUOpPtr(), true );
//        AddOperator(getHatOpPtr(), true );
//        AddOperator(getRestrictOpPtr(), true );
//        AddOperator(getSpeedupOpPtr(), true );
//        AddOperator(getAvg_speedOpPtr(), true );
//        AddOperator(getSubmoveOpPtr(), true );
//        AddOperator(getUvalOpPtr(), true );
//        AddOperator(getMp2onempOpPtr(), true );
//        AddOperator(getP2mpOpPtr(), true );
//        AddOperator(getDelayOpPtr(), true);
//        AddOperator(getDistancetraversedOpPtr(), true);
//        AddOperator(getTurnsOpPtr(), true );
//        AddOperator(getMappingtimeshiftOpPtr(), true );
//        AddOperator(getGridcelleventsOpPtr(), true );
//        AddOperator(getSquareddistanceOpPtr(), true );
//        AddOperator(getGetrefinementpartitionOpPtr(), true );
//
//        AddOperator(getCreateCellGrid2DOpPtr(), true );
//        AddOperator(getCreateCellGrid3DOpPtr(), true );
//
//        AddOperator(getAtRectOpPtr(), true );
//        AddOperator(getMoveToOpPtr(), true );
//        AddOperator(getFillGapsOpPtr(), true );
//        AddOperator(getRemoveShortOpPtr(), true );
//
//        AddOperator(getGetIntervalsOpPtr(), true );
//        AddOperator(getComponentsOpPtr(), true );
//
//        AddOperator(getTrajectory3OpPtr(), true );
//
//        AddOperator(getCreatePeriodsOpPtr(), true);
//        AddOperator(getCt::containsOpPtr(), true);
//        AddOperator(getCt::replaceOpPtr(), true);
//        AddOperator(getCt::removeOpPtr(), true);
//        AddOperator(getCt::getGKZoneOpPtr(), true);


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
