/*

*/
#include <iostream>
#include "MessageServer.h"
#include "MessageBroker.h"
#include <boost/log/trivial.hpp>
#include "../TESContext.h"

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
}

 void MessageServer::run() {
  try {
   while (!boost::this_thread::interruption_requested()) { // throws nothing.
    boost::this_thread::interruption_point();
    processMessage();
   }
  } catch (boost::thread_interrupted &e) {
   BOOST_LOG_TRIVIAL(error) << "catching interrupted";
   return;
  }
 }

  void MessageServer::interrupt() {
    thread->interrupt();
  }


 void MessageServer::processMessage() {
  //boost::this_thread::interruption_point();
  // Checks whether the socket is correctly initialized and ready 
  // for operation (SocketIO.h)
  if(!socket->IsOk()) { 
    BOOST_LOG_TRIVIAL(error) << "BOOST processMessage Anfang: " 
    << socket->GetErrorText();
    interrupt();
    return;
  }
  
  char headerBuffer[Message::HEADER_SIZE];
  memset(headerBuffer,0,Message::HEADER_SIZE); // notwendig? 
  // void * memset ( void * ptr, int value, size_t num );
  // setzt die ersten Message::HEADER_SIZE bytes auf 0.

  int lengthRead = 0; // socket->Read liefert int.
  //size_t lengthRead = 0; // socket->Read liefert bei Fehler neg.int. 
  // -> Konvertierung nach size_t gibt sehr hohe positive Zahl.

  char* offset = headerBuffer;

  while ((size_t)lengthRead < Message::HEADER_SIZE) {
    // int Read( void* buf, size_t minSize, size_t maxSize, 
    // time_t timeout = WAIT_FOREVER )
    lengthRead = socket->Read((void *) offset, Message::HEADER_SIZE,
                             Message::HEADER_SIZE, 
                             1); // wait forever führt dazu, dass von 
                             // insges. 3212 nur 649 Tupel gelesen werden 
                             // timeout bedeutet, dass Datenstrom für 
                             // max n Sekunden unterbrochen sein darf.
    // !!!!! bool Read( void* buf, size_t size ); 
    // !!!!! lengthRead = socket->Read((void *) offset, Message::HEADER_SIZE)
    //  true -> 1 !!!
    if (lengthRead < 0) { // When the return value is less than zero, 
    // an error has occurred. Usually it
                // means that the socket has disconnected (SocketIO.h)
      BOOST_LOG_TRIVIAL(error) << "Lesen von Header fehlgeschlagen: " 
      << socket->GetErrorText();
      BOOST_LOG_TRIVIAL(warning) << "buffer after:  " <<
                               std::string(headerBuffer,
                                           Message::HEADER_SIZE).c_str();
                               
      interrupt(); // socket disconnected. Weiter hat keinen Sinn mehr.
      return;
    }
    // When the return value is less than ~minSize~, 
    // a time out has occurred (SocketIO.h)
    // noch nie passiert
    if ((size_t)lengthRead < Message::HEADER_SIZE && lengthRead > 0) {
      BOOST_LOG_TRIVIAL(warning) << "header partially received (Read "
                               << lengthRead << "B / "
                               << Message::HEADER_SIZE << "B)";
      BOOST_LOG_TRIVIAL(warning) << "buffer after:  " <<
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
    return;
   case Message::MessageType::FINISH:
    handleFinishedMessage(header.eid);
    return;
   default:
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
    delete[] bodyBuffer;
    interrupt();
    return;
  }
  bool read = socket->Read((void *) bodyBuffer,
               bodySize); //blocking, but awaiting message body already
               
  if (!read)  { // noch nie passiert
    BOOST_LOG_TRIVIAL(error) << "Fehler beim Lesen des Tupels: " 
    << socket->GetErrorText();
    delete[] bodyBuffer;
    interrupt();
    return;
  }
  MessageBroker::get().pushTuple(header.eid,header.slot, bodyBuffer);
}

 void MessageServer::handleFinishedMessage(const int eid) {
  MessageBroker::get().pushFinished(eid);
 }

}
