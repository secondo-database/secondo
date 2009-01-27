/*

STPatternAlgebra.h

Created on: Jan 6, 2009
Author: m.attia

*/

#ifndef STPATTERNALGEBRA_H_
#define STPATTERNALGEBRA_H_
#include "Algebra.h"

#include "NestedList.h"

#include "QueryProcessor.h"

#include "StandardTypes.h"
#include "LogMsg.h"
#include "NList.h"
#include "RelationAlgebra.h"
#include "TemporalAlgebra.h"

using namespace datetime;
typedef DateTime Instant;


/*

use the following to switch between different
designs for the stpattern operator. make sure
that only one of the following is uncommented

design\_stream: the first design for stpattern
the operator takes stream(tuple(x)) and filters it

design\_tuple: a modified version of the operator.
This is the version integrated with Secondo Optimizer.
The operator takes tuple(x) and returns boolean.
It can be invoked as a condition inside the filter
operator.

*/





#define design_tuple
// #define design_stream


#endif /* STPATTERNALGEBRA_H_ */
