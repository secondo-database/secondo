/*

*/
#include "TESClient.h"
#include "MessageBroker/MessageBroker.h"
//#include "MessageBroker/Inbox.h"


namespace distributed3
{
  TESClient TESClient::tesClient;
  
  TESClient::TESClient()  {}
  
  TESClient::~TESClient() {} // TODO nothing to destroy, so why a Destructor
  
  void TESClient::endOfTupleStreamFor(int eid) {
    MessageBroker::get().broadcastFinishMessage(eid);
  }
 
  void TESClient::putTuple(const int eid, const int slot, 
                           const int workerNumber, Tuple* tuple) {
    MessageBroker::get().sendTuple(eid,slot,workerNumber,tuple);
  }

  Tuple* TESClient::getTuple(const int eid, const int slot) {
    return MessageBroker::get().getTuple(eid,slot);
  }
  
  TESClient& TESClient::get() {
    return tesClient;
  }
 
  

} /* namespace distributed3 */
