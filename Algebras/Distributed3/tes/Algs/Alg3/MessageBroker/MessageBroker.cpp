/*

*/

#include <iostream>
#include <memory>
#include "MessageBroker.h"
#include "Message.h"
#include "NetworkedClient.h"
#include <pthread.h>
#include <boost/log/trivial.hpp>
#include "../TESContext.h"
#include "../TESManager.h"
#include <StandardTypes.h>

namespace distributed2 {
 MessageBroker MessageBroker::broker;

 MessageBroker::MessageBroker() {}

 MessageBroker::~MessageBroker() {
   reset();
 }
 template<typename T>
 void MessageBroker::clear(std::map<int,std::map<int, std::queue<T>*>>& inbox) {
   for (auto it1 = inbox.begin(); it1 != inbox.end(); ++it1) {
    for (auto it2 = (*it1).second.begin(); it2 != (*it1).second.end(); ++it2) {
      delete (*it2).second;
      (*it2).second = nullptr;
    }
  }
 }
 MessageBroker& MessageBroker::get() {
  return broker;
 }
 
 
 Tuple* MessageBroker::getTuple(const int eid, const int slot) { 
   // read loopInbox first
   if (!loopInboxes[eid][slot]->empty()) {
     Tuple* tuple = loopInboxes[eid][slot]->front(); 
     loopInboxes[eid][slot]->pop();
     return tuple;
   }
   // read inboxes
   while(true) {
     boost::unique_lock<boost::mutex> lock {inboxmutex[eid][slot]}; 
     // inboxcond[eid][slot].wait(lock) needs a unique_lock
     //if (inboxes[eid][slot]->empty()) {
       while (inboxes[eid][slot]->empty() ) {
         inboxcond[eid][slot].wait(lock);
       }
     //}
   // inboxes[eid][slot] not empty
     char* front = inboxes[eid][slot]->front();
     inboxes[eid][slot]->pop();
     boost::mutex* m = lock.release(); // release lock as soon as possible
     m->unlock();
     if (front == nullptr) { // Finish message
       incrementFinished(eid,slot);
       if (allfinished(eid,slot)) { 
         return nullptr;
       }
       continue;
     }
     Tuple* tuple = Message::deserialize(eid, front);
     delete front;
     return tuple;
   }
 }
 
 void MessageBroker::pushTuple(const int eid, const int slot, char* tuple) {
   boost::unique_lock<boost::mutex> lock {inboxmutex[eid][slot]};
   inboxes[eid][slot]->push(tuple);
   inboxcond[eid][slot].notify_one();
 }
 
 void MessageBroker::pushTuple(const int eid, const int slot, char* tuple, 
                                                              int dummy) {
   boost::unique_lock<boost::mutex> lock {inboxmutex[eid][slot]};
   inboxes[eid][slot]->push(tuple);
   inboxcond[eid][slot].notify_one();   
 }
 
 void MessageBroker::pushTuple(const int eid, const int slot, Tuple* tuple) {
   // there is only one LoopbackProxy. And inserting tuples is finished 
   // before tuples are fetched. So no lock is neede
   loopInboxes[eid][slot]->push(tuple);
   // no notification needed. All tuples are inserted before any 
   // tuple is fetched
 }
 void MessageBroker::incrementFinished(const int eid, const int slot) {
   finished[eid][slot]++;
 }

 void MessageBroker::pushFinished(const int eid) {
   for (int slot = 0; slot < getNumberOfSlots(eid); ++slot) {
     if (!inboxes[eid][slot]) {
       assert(inboxes[eid][slot]);
       inboxes[eid][slot] = new std::queue<char*>;
     }
     pushTuple(eid,slot,nullptr,5);
   }
   /*
   for (auto pair : inboxes[eid]) {
     pushTuple(eid, pair.first, nullptr, 5);
   }
   int numberOfSlots = TESContext::get().getNumberOfSlots(eid);
   for (int slot=0; slot<numberOfSlots; ++slot) {
     if (!inboxes[eid][slot]) {
       inboxes[eid][slot] = new std::queue<char*>;
     }
     pushTuple(eid,slot,nullptr,5);
   }
   */
 }
 bool MessageBroker::allfinished(const int eid, const int slot) {
   return servers.size() == finished[eid][slot]; 
   // LoopbackProxy can be spared out
 }
 
 bool MessageBroker::startTcpListener(const int port) {
  try {
   BOOST_LOG_TRIVIAL(info) << "start message server on port " << port;
   tcpListener = std::make_shared<boost::thread>(
    boost::bind(&MessageBroker::acceptConnections, this, port)
   );
   return true;
  } catch (std::exception &e) {
   BOOST_LOG_TRIVIAL(error) << e.what();
   return false;
  } catch (boost::exception &e) {
   BOOST_LOG_TRIVIAL(error) << "Caught boost exception";
   return false;
  }
 }

 void MessageBroker::acceptConnections(const int port) {
  try {
   globalSocket = std::shared_ptr<Socket>(Socket::CreateGlobal("localhost", 
                                                       std::to_string(port)));

   while (!boost::this_thread::interruption_requested()) {
    std::shared_ptr<Socket> serverSocket(globalSocket->Accept());
    boost::this_thread::interruption_point();
    if (boost::this_thread::interruption_requested() ||
        serverSocket == nullptr || !serverSocket->IsOk()) {
     continue;
    }

    auto messageServer = std::make_shared<MessageServer>(serverSocket);
    servers.push_back(messageServer);
    
    boost::this_thread::interruption_point();
   }
  } catch (boost::thread_interrupted &interrupted) {
   BOOST_LOG_TRIVIAL(debug) << "Interrupted. Return";
  }
  if (globalSocket != nullptr) {
   globalSocket->Close();
  }
 }
 
 bool MessageBroker::startClient(const int worker, const RemoteEndpoint host) {
  try {
   auto client = std::make_shared<NetworkedClient>(host);
   workerToClient.insert(std::make_pair(worker, client));
   return true;
  } catch (std::exception &e) {
   return false;
  }
 }
 
 bool MessageBroker::startLoopbackProxy(const int worker) {
    auto loopbackProxy = std::make_shared<LoopbackProxy>(); 
    workerToClient.insert(std::make_pair(worker, loopbackProxy));
    return true;
 }

 void MessageBroker::reset() {
  stopServers();
  stopClients();
  clear(inboxes);
  clear(loopInboxes);
 }
 
 void MessageBroker::stopServers() {
  if (tcpListenerRunning()) {
   tcpListener->interrupt();
   tcpListener = nullptr;
   globalSocket = nullptr;
  }
  for (auto server : servers) {
    server->interrupt();
  }
  servers.clear();
 }
 
 void MessageBroker::stopClients() {
  workerToClient.clear();
 }

 void MessageBroker::sendTuple(const int eid, const int slot, 
                               const int workerNumber, Tuple* tuple) {
  if (workerToClient.find(workerNumber) == workerToClient.end()) {
   BOOST_LOG_TRIVIAL(warning) << "no client set up for worker "
                              << workerNumber << ". Will delete message";
   return;
  }
  std::shared_ptr<MessageClient> client = workerToClient.at(workerNumber);
  client->sendTuple(eid,slot,tuple);
 }

 void MessageBroker::broadcastFinishMessage(const int eid) {
  for (auto it = workerToClient.begin(); it != workerToClient.end(); ++it) {
   auto client = (*it).second;
   client->sendFinishMessage(eid);   
  }
 }

 bool MessageBroker::tcpListenerRunning() {
  if (tcpListener == nullptr) {
   return false;
  }
  if (!tcpListener->joinable()) {
   return false;
  }
  if (tcpListener->try_join_for(boost::chrono::nanoseconds(1))) {
   return false;
  }
  return true;
 }
  
  void MessageBroker::setNumberOfSlots(const int eid, const int slots) {
    initializeInboxes(eid,slots);
    slotCountMap[eid] = slots;
  }
  int MessageBroker::getNumberOfSlots(const int eid) {
    assert (slotCountMap[eid] > 0);
    return slotCountMap[eid];
  }
  void MessageBroker::initializeInboxes(const int eid, const int slots) {
    for (int i = 0; i < slots; ++i) {
      loopInboxes[eid][i] = new std::queue<Tuple*>;
    }
    for (int i = 0; i < slots; ++i) {
      inboxes[eid][i] = new std::queue<char*>;
    }
     /*
    for (auto server : servers) {
      server->initializeInboxes(eid, slots);
    }
    */
  }
 
}

