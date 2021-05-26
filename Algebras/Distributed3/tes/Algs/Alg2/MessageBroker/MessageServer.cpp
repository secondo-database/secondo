/*

*/
#include <iostream>
#include "MessageServer.h"
#include "MessageBroker.h"
#include <boost/log/trivial.hpp>
//#include "../TESContext.h"

namespace distributed2 {

MessageServer::MessageServer(std::shared_ptr<Socket>  _socket) 
                                                    : socket{_socket} {
  thread = new boost::thread(boost::bind(&MessageServer::run, this));
}

MessageServer::~MessageServer() {
  socket->Close(); 
  thread->interrupt();
  if(thread->joinable()){
     thread->join();
  }
  delete thread;
  clear(inboxes);
}

 template<typename T>
 void MessageServer::clear(std::map<int,std::map<int, std::queue<T>*>>& inbox) {
   for (auto it1 = inbox.begin(); it1 != inbox.end(); ++it1) {
    for (auto it2 = (*it1).second.begin(); it2 != (*it1).second.end(); ++it2) {
      delete (*it2).second;
      (*it2).second = nullptr;
    }
  }
 }
 
 void MessageServer::run() {
  try {
   while (!boost::this_thread::interruption_requested()) { // throws nothing.
    boost::this_thread::interruption_point();
    processMessage();
   }
  } catch (boost::thread_interrupted &e) {
   cout << "\nMessageServer::run unterbrochen";
   BOOST_LOG_TRIVIAL(error) << "catching interrupted";
   return;
  }
 }

  void MessageServer::interrupt() {
    thread->interrupt();
  }


 void MessageServer::processMessage() {
  //boost::this_thread::interruption_point();
  // Checks whether the socket is correctly initialized and 
  // ready for operation (SocketIO.h)
  if(!socket->IsOk()) { 
    BOOST_LOG_TRIVIAL(error) << "BOOST processMessage Anfang: " 
    << socket->GetErrorText();
    cout << "\ncout processMessage Anfang: " << socket->GetErrorText();
    interrupt();
    return;
  }
  //auto header = readHeader2();
  
  char headerBuffer[Message::HEADER_SIZE];
  memset(headerBuffer,0,Message::HEADER_SIZE); // notwendig? 
  //void * memset ( void * ptr, int value, size_t num );
  // setzt die ersten Message::HEADER_SIZE bytes auf 0.

  int lengthRead = 0; // socket->Read liefert int.
  //size_t lengthRead = 0; // socket->Read liefert bei Fehler 
  // neg.int. -> Konvertierung nach size_t gibt sehr hohe positive Zahl.

  char* offset = headerBuffer;
  // wozu dieser Teil?
  while ((size_t)lengthRead < Message::HEADER_SIZE) {
    // int Read( void* buf, size_t minSize, size_t maxSize, 
    // time_t timeout = WAIT_FOREVER )
    lengthRead = socket->Read((void *) offset, Message::HEADER_SIZE,
                             Message::HEADER_SIZE, 
                             1); // wait forever führt dazu, dass von 
                             //insges. 3212 nur 649 Tupel gelesen werden 
                             // timeout bedeutet, dass Datenstrom für 
                             //max n Sekunden unterbrochen sein darf.
    // !!!!! bool Read( void* buf, size_t size ); 
    // !!!!! lengthRead = socket->Read((void *) offset, 
    // Message::HEADER_SIZE) true -> 1 !!!
    if (lengthRead < 0) { // When the return value is less than zero, 
    // an error has occurred. Usually it
                          // means that the socket has disconnected (SocketIO.h)
      BOOST_LOG_TRIVIAL(error) << "Lesen von Header fehlgeschlagen: " 
      << socket->GetErrorText();
      BOOST_LOG_TRIVIAL(warning) << "buffer after:  " <<
                               std::string(headerBuffer,
                                           Message::HEADER_SIZE).c_str();
      cout << "\ncout Lesen von Header fehlgeschlagen: " 
      << socket->GetErrorText();  
      cout << "\ncout buffer after:  " 
      << std::string(headerBuffer,Message::HEADER_SIZE).c_str();
      interrupt(); // socket disconnected. Weiter hat keinen Sinn mehr.
      return;
    }
    // When the return value is less than ~minSize~, a 
    // time out has occurred (SocketIO.h)
    // noch nie passiert
    if ((size_t)lengthRead < Message::HEADER_SIZE && lengthRead > 0) {
      BOOST_LOG_TRIVIAL(warning) << "header partially received (Read "
                               << lengthRead << "B / "
                               << Message::HEADER_SIZE << "B)";
      BOOST_LOG_TRIVIAL(warning) << "buffer after:  " <<
                               std::string(headerBuffer,
                                           Message::HEADER_SIZE).c_str();
      cout << "\ncout header partially received (Read "
                               << lengthRead << "B / "
                               << Message::HEADER_SIZE << "B)";
      cout << "\ncout buffer after:  " <<
                               std::string(headerBuffer,
                                           Message::HEADER_SIZE).c_str();
     }
     offset += lengthRead;
  }
  
  auto header = Message::Header::fromBin(headerBuffer);
  
  const Message::MessageType messageType = header.type;

  switch (messageType) {
   case Message::MessageType::DATA: // Normalfall, wird zuerst geprüft
    handleDataMessage(header);
    //RECEIVED_MESSAGE
    return;
   case Message::MessageType::FINISH:
    cout << "\nhandle finish message: " << header;
    //handleFinishedMessage(header.eid);
    /* Tuple From Server Begin */
    handleFinishedMessage(header.eid);
    /* Tuple From Server End */
    
    return;
   default:
    cout << "\ncout Received message of unknown type: " << header; 
    BOOST_LOG_TRIVIAL(error) << "BOOST Received message of unknown type: " 
    << header; 
    assert(false);
    return;
  }
 }

  void MessageServer::handleDataMessage(Message::Header& header) {
  const size_t bodySize = header.length;
  char* bodyBuffer = new char[bodySize];
  if (!socket->IsOk()) {
    BOOST_LOG_TRIVIAL(error) << "processMessage unten: " 
    << socket->GetErrorText();
    cout << "\ncout processMessage unten: " << socket->GetErrorText();
    delete[] bodyBuffer;
    interrupt();
    return;
  }
  bool read = socket->Read((void *) bodyBuffer,
               bodySize); //blocking, but awaiting message body already
               
  if (!read)  { // noch nie passiert
    BOOST_LOG_TRIVIAL(error) << "Fehler beim Lesen des Tupels: " 
    << socket->GetErrorText();
    cout << "\ncout Fehler beim Lesen des Tupels: " << socket->GetErrorText();
    delete[] bodyBuffer;
    interrupt();
    return;
  }
  pushTuple(header.eid, header.slot, bodyBuffer);
}

 void MessageServer::handleFinishedMessage(const int eid) {
   MessageBroker& broker = MessageBroker::get();
   broker.incrementFinished(eid);
   for (int slot = 0; slot < MessageBroker::get().getNumberOfSlots(eid); 
         ++slot) {
      broker.addServer(eid, slot, this);
      broker.wakeup(eid,slot);
    }
 }
 
  void MessageServer::pushTuple(const int eid, const int slot, char* tuple) {
   boost::unique_lock<boost::mutex> lock {inboxmutex[eid][slot]};
   bool empty = inboxes[eid][slot]->empty();
   inboxes[eid][slot]->push(tuple);
   boost::mutex* m = lock.release();
   m->unlock();
   if (empty) {
     MessageBroker& broker = MessageBroker::get();
     broker.addServer(eid, slot, this);
     broker.wakeup(eid, slot);
   }
   
   //inboxcond[eid][slot].notify_one();
 }
 bool MessageServer::hasTuple(const int eid, const int slot) {
   //boost::unique_lock<boost::mutex> lock {inboxmutex[eid][slot]};
   // if (!inboxes[eid][slot]) return false;
   assert (inboxes[eid][slot]);
   return !(inboxes[eid][slot]->empty());
 }
 Tuple* MessageServer::getTuple(const int eid, const int slot) {
   boost::unique_lock<boost::mutex> lock {inboxmutex[eid][slot]}; 
   char* front = inboxes[eid][slot]->front();
   inboxes[eid][slot]->pop();
   // release lock as soon as possible. Messages can be pushed 
   // while processing the popped message
   boost::mutex* m = lock.release(); 
   m->unlock();
   Tuple* tuple = Message::deserialize(eid, front);
   delete front;
   return tuple;
 }
 void MessageServer::initializeInboxes(const int eid, const int numberOfSlots) {
    for (int i = 0; i < numberOfSlots; ++i) {
      inboxes[eid][i] = new std::queue<char*>;
    }
  }
/* Tuple From Server End */

}
