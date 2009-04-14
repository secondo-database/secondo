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
#include <map>
using namespace datetime;
typedef DateTime Instant;


//the "0" in the end of the array is used for iterating over the array
//if you want to add more connectors, add them in the begining or the
//middle of the array
string connector[]= {"then","later","meanwhile","immediately","follows","0"};
map<string, Interval<Instant> > label;



#endif /* STPATTERNALGEBRA_H_ */
