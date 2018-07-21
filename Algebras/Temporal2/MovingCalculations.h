/*
MovingCalculations.h
Created on: 10.06.2018
Author: simon

*/

#ifndef ALGEBRAS_TEMPORAL2_MOVINGCALCULATIONS_H_
#define ALGEBRAS_TEMPORAL2_MOVINGCALCULATIONS_H_

#include "DateTime.h" //Instant
#include "Algebras/Temporal/TemporalAlgebra.h" //Interval

namespace temporal2algebra {
using temporalalgebra::Interval;

template<class TiAlpha, class TuAlpha>
void CreateStartExtensionUnit(const TiAlpha* iAlpha, TuAlpha* result) {
    Instant instant = iAlpha->instant;
    Interval<Instant> interval(instant, instant, true, true);
    TuAlpha resUnit(interval, iAlpha->value, iAlpha->value);
    *result = resUnit;
    return;
}

template<class TuAlpha, class TiAlpha>
void CreateRegularExtensionUnit(const TiAlpha* final_intime_mAlpha,
        const TiAlpha* iAlpha,
        TuAlpha* result ) {

    // TODO: corner case: previous unit is right open but same instant.
    if (iAlpha->instant > final_intime_mAlpha->instant) {
        Interval<Instant> res_interval(
                final_intime_mAlpha->instant, iAlpha->instant, false, true);
        TuAlpha extension_unit(
                res_interval, final_intime_mAlpha->value, iAlpha->value);
        *result = extension_unit;
        return;
    }
    result->SetDefined(false);
    return;
}

// calculate the next Unit based on the an m(alpha) for a given i(alpha)
template<class TiAlpha, class TuAlpha>
void CreateExtensionUnit(
        const TuAlpha* uAlpha,
        const TiAlpha* iAlpha,
        TuAlpha* res_uAlpha) {

    if (!iAlpha->IsDefined()) {
        // nothing to append => nothing to do
        *res_uAlpha = TuAlpha(false);
        return;
    }

    if (!uAlpha->IsDefined()) {
        // we have no "starting point" => create a single "point" unit
        TuAlpha result;
        CreateStartExtensionUnit(iAlpha, &result);
        *res_uAlpha = result;
        return;
    }

    TiAlpha final_instant(true);
    uAlpha->TemporalFunction(
            uAlpha->timeInterval.end,
            final_instant.value, true );
    final_instant.instant.CopyFrom( &uAlpha->timeInterval.end );

    TuAlpha result;
    CreateRegularExtensionUnit(&final_instant, iAlpha, &result );
    *res_uAlpha = result;
    return;
}

} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_MOVINGCALCULATIONS_H_ */
