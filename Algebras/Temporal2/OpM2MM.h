/*
OpM2MM.h
Created on: 08.04.2018
Author: simon

Helper Operators to externally control a stream:
insert a named StreamValve into any Secondo Stream to block that stream
using the M2MM Operator will allow the specified number of items through the stream

*/

#ifndef ALGEBRAS_TEMPORAL2_OPM2MM_H_
#define ALGEBRAS_TEMPORAL2_OPM2MM_H_

class Operator;

namespace temporal2algebra{
  Operator* getM2MMOpPtr();
}

#endif /* ALGEBRAS_TEMPORAL2_OPM2MM_H_ */
