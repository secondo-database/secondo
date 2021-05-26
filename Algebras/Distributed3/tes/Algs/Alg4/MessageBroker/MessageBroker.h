/*

*/
#ifndef SECONDO_TES_MESSAGEBROKER_H
#define SECONDO_TES_MESSAGEBROKER_H

#include <list>
#include <functional>
#include <map>
#include "MessageServer.h"
#include "MessageClient.h"
//#include "QueueSupplier.h"
#include "LoopbackProxy.h"
#include "../Helpers/RemoteEndpoint.h"
#include "../typedefs.h"
#include <boost/thread.hpp>

namespace distributed2 {
 class MessageBroker {
 
 public:
  MessageBroker();

  ~MessageBroker();

  static MessageBroker& get();
  
  void addServer(const int eid, const int slot, MessageServer* server); // 5
  void wakeup(const int eid, const int slot); // 5
 
  Tuple* getTuple(const int eid, const int slot); // 5
  void pushTuple(const int eid, const int slot, Tuple* tuple); // 5
  void incrementFinished(const int eid, const int slot); // 5
  void setNumberOfSlots(const int eid, const int slots); // 5
  int getNumberOfSlots(const int eid); // 5

  void sendTuple(const int eid, const int slot, const int workerNumber, 
                 Tuple* tuple);
  void broadcastFinishMessage(const int eid);
  
  bool startTcpListener(const int port); // in StartTESServer

  bool startClient(const int slot, const RemoteEndpoint host);

  bool startLoopbackProxy(const int slot);

  bool tcpListenerRunning(); // in StartTESServer

  void stopServers();

  void stopClients();

  void reset();

  private:
  static MessageBroker broker;
  std::shared_ptr<boost::thread> tcpListener = nullptr; 

  std::shared_ptr<Socket> globalSocket = nullptr; 

  std::list<std::shared_ptr<MessageServer> > servers 
                    = std::list< std::shared_ptr<MessageServer> >();
  
  using mt = std::map<int, std::shared_ptr<MessageClient> >;
  mt workerToClient = mt();
  
  template<typename T>
  void clear(std::map<int,std::map<int, std::queue<T>*>>& inbox); 
  
  void removeMessageServer(const int eid,const int slot); // 5
  bool allfinished(const int eid, const int slot);  // 5
  
  std::map<int, std::map<int, boost::mutex>> servers2mtx; // 5
  std::map<int, std::map<int, std::list<MessageServer*>>> servers2; // 5
  std::map<int, std::map<int, std::list<MessageServer*>::iterator>>
   serversIterator; // 5
  std::map<int, std::map<int, boost::condition_variable>> readycond; // 5
  
  std::map<int, std::map<int, std::queue<Tuple*>* > > loopInboxes; // 5
  std::map<int, std::map<int, unsigned int>> finished; // 5
    
  std::map<int,int> slotCountMap; // 5
  inline void initializeInboxes(const int eid, const int slots);
  
  
  void acceptConnections(int port);
 };
}


#endif //SECONDO_MESSAGEBROKER_H
