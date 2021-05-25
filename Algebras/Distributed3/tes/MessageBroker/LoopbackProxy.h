/*

*/

#ifndef SECONDO_TESCLIENTLOOPBACKPROXY_H
#define SECONDO_TESCLIENTLOOPBACKPROXY_H

#include "MessageClient.h"
//#include "Inbox.h"
//#include "QueueSupplier.h"
//#include "MessageBroker.h"
//#include "../Helpers/RemoteEndpoint.h"
#include "../typedefs.h"
#include <functional>

namespace distributed3 {
 class LoopbackProxy : public MessageClient {
 
 public:
  LoopbackProxy();
  ~LoopbackProxy() override; // TODO nothing to destroy
    
  void sendTuple(const int eid, 
                 const int slot, Tuple* tuple) /*const*/ override;
  void sendFinishMessage(const int eid) /*const*/ override;
  
 };
}


#endif //SECONDO_TESCLIENTLOOPBACKPROXY_H
