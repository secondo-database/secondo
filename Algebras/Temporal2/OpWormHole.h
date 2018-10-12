/*
Helper operators to workaround crashes using feed-operator
and intermediate commits from appendpositions
operator enterwormhole accepts stream and copies it into a memory queue
operator leavewormhole reads from that queue creates a new stream

Limitations:
- only accepts tuple(ipoint, tid), everything else might cause serious trouble
- no type checking on stream/tuple

*/

#ifndef ALGEBRAS_TEMPORAL2_OPWORMHOLE_H_
#define ALGEBRAS_TEMPORAL2_OPWORMHOLE_H_


class Operator;

namespace temporal2algebra{
Operator* getEnterWormHoleOpPtr();
Operator* getLeaveWormHoleOpPtr();
}

#endif /* ALGEBRAS_TEMPORAL2_OPWORMHOLE_H_ */
