/*
header for including sample operators from TemporalAlgebra

*/
#ifndef ALGEBRAS_TEMPORAL2_TEMPORALALGEBRAOPERATORS_H_
#define ALGEBRAS_TEMPORAL2_TEMPORALALGEBRAOPERATORS_H_

class Operator;

namespace temporal2algebra {
Operator* getAtperiodsOpPtr();
Operator* getTrajectoryOpPtr();
Operator* getBBoxOpPtr();
Operator* getTranslateappendSOpPtr();
} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_TEMPORALALGEBRAOPERATORS_H_ */
