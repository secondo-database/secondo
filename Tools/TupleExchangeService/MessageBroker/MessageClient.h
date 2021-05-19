/*

*/
#ifndef SECONDO_TESMESSAGECLIENT_H
#define SECONDO_TESMESSAGECLIENT_H


#include "Algebras/Relation-C++/RelationAlgebra.h"

namespace distributed3 {
 class MessageClient {
 public:
  virtual ~MessageClient();
  virtual void sendTuple(const int eid, 
                         const int slot, 
                         Tuple* tuple) /*const*/ = 0; 
  virtual void sendFinishMessage(const int eid) /*const*/ = 0;
 };
}


#endif //SECONDO_TESMESSAGECLIENT_H
