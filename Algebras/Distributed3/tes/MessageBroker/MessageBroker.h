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

namespace distributed3 {
 class MessageBroker {
 
 public:
  MessageBroker();

  ~MessageBroker();

  static MessageBroker& get();
  
  /* Tuple From Server Begin */
  Tuple* getTuple(const int eid, const int slot);
  void addServer(const int eid, const int slot, MessageServer* server);
  void wakeup(const int eid, const int slot);
  /* Tuple From Server End */
  
  //Tuple* getTuple(const int eid, const int slot);
  //void pushTuple(const int eid, const int slot, char* tuple);
  void pushTuple(const int eid, const int slot, Tuple* tuple);
  
  void incrementFinished(const int eid);
  void sendTuple(const int eid, const int slot, 
                 const int workerNumber, Tuple* tuple);
  void broadcastFinishMessage(const int eid);
  
  void setNumberOfSlots(const int eid, const int slots); // 5
  int getNumberOfSlots(const int eid); // 5
  
  bool startTcpListener(const int port); // in StartTESServer

  bool startClient(const int slot, const RemoteEndpoint host);

  bool startLoopbackProxy(const int slot);

  bool tcpListenerRunning(); // in StartTESServer
  /*
  void pauseServers();
  
  void startServers();
  */
  void stopServers();

  void stopClients();

  void reset();

  private:
  static MessageBroker broker;
  std::shared_ptr<boost::thread> tcpListener = nullptr; 

  std::shared_ptr<Socket> globalSocket = nullptr; 
  
  std::list<std::shared_ptr<MessageServer> > servers 
                    = std::list< std::shared_ptr<MessageServer> >();
  
  //std::shared_ptr<LoopbackProxy> loopbackProxy;
  
  using mt = std::map<int, std::shared_ptr<MessageClient> >;
  mt workerToClient = mt();


  bool allfinished(const int eid);
  std::map<int, std::map<int, std::queue<Tuple*>* > > loopInboxes;
  //std::map<int, std::map<int, std::queue<char*>* > > inboxes;
  template<typename T>
  void clear(std::map<int,std::map<int, std::queue<T>*>>& inbox); 
  
  std::map<int, std::map<int, boost::mutex>> inboxmutex;
  //std::map<int, std::map<int, boost::condition_variable>> inboxcond;
  
  //std::map<int, std::map<int, boost::mutex>> readymtx;
  
  //std::map<int, std::map<int, bool>> ready;

  std::map<int, unsigned int> finished;
  std::map<int, boost::mutex> finishedMutex;
  
  std::map<int, std::map<int, boost::mutex>> servers2mtx;
  std::map<int, std::map<int, std::list<MessageServer*>>> servers2;
  std::map<int, std::map<int, std::list<MessageServer*>::iterator>>
                                                              serversIterator;
  std::map<int, std::map<int, boost::condition_variable>> readycond;
  
  // uses 
  // bool allfinished(const int eid);
  
  std::map<int,int> slotCountMap; // 5
  inline void initializeInboxes(const int eid, const int slots);
  
  void removeMessageServer(const int eid, const int slot);
  
  void acceptConnections(int port);
 };
}


#endif //SECONDO_MESSAGEBROKER_H
