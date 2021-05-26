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
      std::cout << "\ndelete " << (*it2).first;
      if ((*it2).second) {
        delete (*it2).second;
        (*it2).second = nullptr;
      }
    }
  }
 }
 MessageBroker& MessageBroker::get() {
  return broker;
 }
 
 Tuple* MessageBroker::getTuple(const int eid, const int slot) {
   // read LoopInbox first until empty
   if (!loopInboxes[eid][slot]->empty()) {
     Tuple* tuple = loopInboxes[eid][slot]->front(); 
     loopInboxes[eid][slot]->pop();
     return tuple;
   }
   // else: getTuple from MessageServer
   boost::unique_lock<boost::mutex> lock1 {servers2mtx[eid][slot]};  
       //while (!ready[eid][slot] ) {
       while (servers2[eid][slot].empty() ) {
         readycond[eid][slot].wait(lock1);
       }
       boost::mutex* m = lock1.release();
       m->unlock();

   /* // only needed for alternative in addServer
   
     // serversIterator[eid][slot] must be initialized correctly
     // servers2[eid][slot] can't be empty here
     serversIterator[eid][slot] = servers2[eid][slot].begin();
   }
   */ // alternative end
   
   while(true) {
     assert(!servers2[eid][slot].empty()); // addServer has been called earlier
     // make it circular and ensure that the serversIterator does 
     // not point to the end. 
     // serversIterator[eid][slot] is not modified in the 
     // state !servers2[eid][slot].empty() by addServer(). So we need no lock.
     if (serversIterator[eid][slot] == servers2[eid][slot].end() ) { // make 
     // it circular
       serversIterator[eid][slot] = servers2[eid][slot].begin();
     }
     MessageServer* ms = *(serversIterator[eid][slot]);
  
     if (ms->hasTuple(eid,slot)) {
       (serversIterator[eid][slot])++; // it is not possible to move 
       // it to the beginning of the while-loop.
       return ms->getTuple(eid,slot);  // Explanation follows later
     }
     removeMessageServer(eid,slot);  
     if (allfinished(eid)) {
       if (servers2[eid][slot].empty()) {
         return nullptr;
       }
       continue; // try next MessageServer
     }
     // else: not allfinished(eid)
     boost::unique_lock<boost::mutex> lock2 {servers2mtx[eid][slot]};
     while (servers2[eid][slot].empty() ) {
       readycond[eid][slot].wait(lock2);
     } 
     // List not empty -> try next MessageServer
   }
 }
 
 
 void MessageBroker::removeMessageServer(const int eid, const int slot) {
   boost::unique_lock<boost::mutex> lock {servers2mtx[eid][slot]};
   std::list<MessageServer*>::iterator remove = serversIterator[eid][slot];
   (serversIterator[eid][slot])++; 
   servers2[eid][slot].erase(remove);
 }
 
 void MessageBroker::addServer(const int eid, const int slot, 
                                                MessageServer* server) { 
  
   // alternative: if getTuple() uses a MessageServer who has at 
   // least one tuple, we possibly avoid erasing a MessageServer 
   // from servers2[eid][slot] who has not yet received its next tuple.
   boost::unique_lock<boost::mutex> lock {servers2mtx[eid][slot]};
   if (servers2[eid][slot].empty()) { // initialized on the fly
     servers2[eid][slot].push_back(server); // first elem must be pushed. 
     // An iterator to an empty list points to the end
     serversIterator[eid][slot] = servers2[eid][slot].begin(); // now 
     // the iterator points to an elem
   }
   auto insertit = serversIterator[eid][slot];
   insertit++; // might be the end
   servers2[eid][slot].insert(insertit,server);
   // thesis one end
   
   // thesis two: in the naive implementation servers[eid][slot] is 
   // locked for the shortest possible duration.
   //             getTuple() can be called sooner than in the first solution
   //boost::unique_lock<boost::mutex> lock {servers2mtx[eid][slot]};
   //servers2[eid][slot].push_back(server);
   // thesis two end
 }
 void MessageBroker::wakeup(const int eid, const int slot) {
   readycond[eid][slot].notify_one();
 }

 void MessageBroker::pushTuple(const int eid, const int slot, Tuple* tuple) {
   // there is only one LoopbackProxy. And inserting tuples is finished 
   // before tuples are fetched. So no lock is needed
   if (!loopInboxes[eid][slot]) { 
     loopInboxes[eid][slot] = new std::queue<Tuple*>; // braucht es 
     // die Initialisierung mit {} ?
   }
   loopInboxes[eid][slot]->push(tuple);
   // no notification needed. All tuples are inserted before any tuple 
   // is fetched
 }
 void MessageBroker::incrementFinished(const int eid) {
   boost::lock_guard<boost::mutex> {finishedMutex[eid]};
   finished[eid]++;
 }
 bool MessageBroker::allfinished(const int eid) {
   return servers.size() == finished[eid]; // LoopbackProxy can be spared out
 }
 void MessageBroker::setNumberOfSlots(const int eid, const int slots) {
    slotCountMap[eid] = slots;
    initializeInboxes(eid,slots);
  }
  int MessageBroker::getNumberOfSlots(const int eid) {
    assert (slotCountMap[eid] > 0);
    return slotCountMap[eid];
  }
  void MessageBroker::initializeInboxes(const int eid, const int slots) {
    for (int i = 0; i < slots; ++i) {
      loopInboxes[eid][i] = new std::queue<Tuple*>;
    }
    for (auto server : servers) {
      server->initializeInboxes(eid, slots);
    }
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
 
}

