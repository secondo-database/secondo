/*

*/

#ifndef SECONDO_NETWORKEDTESCLIENT_H
#define SECONDO_NETWORKEDTESCLIENT_H

#include "MessageClient.h"
#include "../Helpers/RemoteEndpoint.h"
#include "Message.h"
#include "SocketIO.h"

namespace distributed3 {
 class NetworkedClient : public MessageClient {
 public:
  explicit NetworkedClient(const RemoteEndpoint &host) noexcept(false);
  ~NetworkedClient() override;

  void sendTuple(const int eid, const int slot, Tuple* tuple)  override;
  void sendFinishMessage(const int eid) /*const*/ override;

 private:
  void send(); // sendThread takes Messages and sends them over the network
  inline void sendMessageAsync(Message* message);
  //void sendMessage(Message message) /*const*/ ;
  //void sendMessage(std::pair<size_t,char*>) /*const*/ ;
  void sendMessage(size_t,char*) /*const*/ ;
  void sendMessage(Message* message);
 // void sendMessage(std::shared_ptr<Message> message) /*const*/ ;
  mutable Socket *socket = nullptr;
  boost::thread* sendThread = nullptr; // unique_ptr, naked thread ?
  boost::mutex outboxmtx;
  boost::condition_variable outboxcond;
  //std::queue<Message*>*  outbox;  
  std::queue<std::pair<size_t,char*>>*  binOutbox;
 };
}


#endif //SECONDO_NETWORKEDCLIENT_H
