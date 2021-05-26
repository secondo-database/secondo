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
  
  Tuple* getTuple(const int eid, const int slot);
  void addServer(const int eid, const int slot, MessageServer* server);
  void wakeup(const int eid, const int slot);

  void pushTuple(const int eid, const int slot, Tuple* tuple);
  
  void incrementFinished(const int eid);
  void sendTuple(const int eid, const int slot, const int workerNumber, 
                                                Tuple* tuple);
  void broadcastFinishMessage(const int eid);
  
  void setNumberOfSlots(const int eid, const int slots); 
  int getNumberOfSlots(const int eid); 
  
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


  bool allfinished(const int eid);
  std::map<int, std::map<int, std::queue<Tuple*>* > > loopInboxes;

  template<typename T>
  void clear(std::map<int,std::map<int, std::queue<T>*>>& inbox); 
  
  std::map<int, std::map<int, boost::mutex>> inboxmutex;

  std::map<int, unsigned int> finished;
  std::map<int, boost::mutex> finishedMutex;
  
  std::map<int, std::map<int, boost::mutex>> servers2mtx;
  std::map<int, std::map<int, std::list<MessageServer*>>> servers2;
  std::map<int, std::map<int, std::list<MessageServer*>::iterator>>
                                                           serversIterator;
  std::map<int, std::map<int, boost::condition_variable>> readycond;

  std::map<int,int> slotCountMap; 
  inline void initializeInboxes(const int eid, const int slots);
  
  void removeMessageServer(const int eid, const int slot);
  
  void acceptConnections(int port);
 };
}


#endif //SECONDO_MESSAGEBROKER_H
