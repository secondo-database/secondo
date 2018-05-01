/*
OpStreamValve.h
Created on: 08.04.2018
Author: simon

Helper Operators to externally control a stream:
insert a named StreamValve into any Secondo Stream to block that stream
using the StreamValve Operator will allow the specified number of items through the stream

*/

#ifndef ALGEBRAS_TEMPORAL2_OPSTREAMVALVE_H_
#define ALGEBRAS_TEMPORAL2_OPSTREAMVALVE_H_


class Operator;

namespace temporal2algebra{
  Operator* getStreamValveOpPtr();
}

#endif /* ALGEBRAS_TEMPORAL2_OPSTREAMVALVE_H_ */
