/*
TemporalAlgebraFunctions.h
Created on: 03.07.2018
Author: simon

class Methods of temporalalgebra::MPoint refactored to free functions

*/

#ifndef ALGEBRAS_TEMPORAL2_TEMPORALALGEBRAFUNCTIONS_H_
#define ALGEBRAS_TEMPORAL2_TEMPORALALGEBRAFUNCTIONS_H_

#include "Algebras/Rectangle/RectangleAlgebra.h" //Rectangle<>
#include "Algebras/Spatial/SpatialAlgebra.h" //Line

namespace temporalalgebra {
    class Periods;
}
namespace datetime {
    class DateTime;
}

class Geoid;

namespace temporal2algebra {

class MPoint2;

bool IsValid(const MPoint2& mp);

void AtPeriods( const MPoint2& mp,
        const temporalalgebra::Periods& p,
        MPoint2& result );

Rectangle<3> BoundingBox( const MPoint2& mp, const Geoid* geoid = 0);

void Trajectory( const MPoint2& mp, Line& line );

// special case: non-const in TemporalAlgebra:
//void MPoint::TranslateAppend(const MPoint& mp, const datetime::DateTime& dur);
void TranslateAppend(const MPoint2& mp,
        const datetime::DateTime& dur,
        MPoint2& destMp);

} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_TEMPORALALGEBRAFUNCTIONS_H_ */
