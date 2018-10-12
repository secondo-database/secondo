/*
operator streamvalve:
insert into any second stream to externally control its flow.
use operator streamnext in other secondo process to let elements pass.

*/

#ifndef ALGEBRAS_TEMPORAL2_OPSTREAMVALVE_H_
#define ALGEBRAS_TEMPORAL2_OPSTREAMVALVE_H_


class Operator;

namespace temporal2algebra{
Operator* getStreamValveOpPtr();
}

#endif /* ALGEBRAS_TEMPORAL2_OPSTREAMVALVE_H_ */
