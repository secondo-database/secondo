/*

*/
#include <assert.h>
#include "LoopbackProxy.h"
#include "MessageBroker.h"
#include "../typedefs.h"
#include <boost/log/trivial.hpp>
#include <iostream>

namespace distributed3 {

LoopbackProxy::~LoopbackProxy() {
  std::cout << "~LoopbackProxy aufgerufen";
}

LoopbackProxy::LoopbackProxy()  {}

void LoopbackProxy::sendTuple(const int eid, const int slot, Tuple* tuple) {
  MessageBroker::get().pushTuple(eid,slot,tuple);
}
void LoopbackProxy::sendFinishMessage(const int eid) /*const*/ {}

} // namespace distributed3
