/*
OpStorageAction.h
Created on: 08.04.2018
Author: simon

Allow direct access to MemStorageActions

*/

#ifndef ALGEBRAS_TEMPORAL2_OPSTORAGEACTION_H_
#define ALGEBRAS_TEMPORAL2_OPSTORAGEACTION_H_

class Operator;

namespace temporal2algebra{
  Operator* getStorageActionOpPtr();
}

#endif /* ALGEBRAS_TEMPORAL2_OPSTORAGEACTION_H_ */
