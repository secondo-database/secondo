/*
operator appendpositions: insert incoming position updates to
in-memory data structure of mpoint2
allows for intermediate commits with transactionmode 1

limitations:
only working for mpoint2 in persistent relations

*/

#ifndef ALGEBRAS_TEMPORAL2_OPAPPENDPOSITIONS_H_
#define ALGEBRAS_TEMPORAL2_OPAPPENDPOSITIONS_H_

class Operator;

namespace temporal2algebra{
Operator* getAppendPositionsOpPtr();
}

#endif /* ALGEBRAS_TEMPORAL2_OPAPPENDPOSITIONS_H_ */
