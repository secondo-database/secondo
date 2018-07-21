/*
OpWormHole.h
Created on: 08.04.2018
Author: simon

Helper Operators to externally control a stream:
insert a named EnterWormHole into any Secondo Stream to block that stream
using the EnterWormHole Operator will allow the specified number of items through the stream

*/

#ifndef ALGEBRAS_TEMPORAL2_OPWORMHOLE_H_
#define ALGEBRAS_TEMPORAL2_OPWORMHOLE_H_


class Operator;

namespace temporal2algebra{
  Operator* getEnterWormHoleOpPtr();
  Operator* getLeaveWormHoleOpPtr();
}

#endif /* ALGEBRAS_TEMPORAL2_OPWORMHOLE_H_ */
