/*

*/

#include "NetworkedClient.h"

namespace distributed3 {
  
NetworkedClient::NetworkedClient(const RemoteEndpoint &host) noexcept(false) {
    auto socket = Socket::Connect(host.host, std::to_string(host.port),
                                Socket::SockAnyDomain, 10, 1);
  if (socket == nullptr) {
   BOOST_LOG_TRIVIAL(error) << "Couldn't connect to server.";
   throw std::exception();
  }
  if (not socket->IsOk()) {
   delete socket;
   BOOST_LOG_TRIVIAL(error) << "Connection to server is NOT ok.";
   throw std::exception();
  }

  BOOST_LOG_TRIVIAL(info) << "Successfully connected to host: " << host.host
                          << ":" << host.port;
  this->socket = socket;
  
  this->binOutbox = new std::queue<std::pair<size_t,char*>>;
  
  sendThread = new boost::thread(
    boost::bind(&NetworkedClient::send, this)
   );
  
   
}

NetworkedClient::~NetworkedClient() {
  std::cout << "\n~NetworkedClient() aufgerufen";
  if (socket != nullptr) {
   socket->Close();
   delete socket;
  }
  
  sendThread->interrupt();
  if(sendThread->joinable()){
     sendThread->join();
  }
  delete sendThread;
  
}
 
void NetworkedClient::sendTuple(const int eid, const int slot, Tuple* tuple){
  //sendMessage(Message::fromTuple1(eid,slot,tuple)); 
  sendMessageAsync(Message::fromTuple1(eid,slot,tuple));
  
  
}

void NetworkedClient::sendFinishMessage(const int eid) /*const*/ {
  //sendMessage(Message::constructFinishMessage1(eid));
  sendMessageAsync(Message::constructFinishMessage1(eid));
}

void NetworkedClient::sendMessageAsync(Message* message) { // is inlined 
  char* buffer = nullptr;
  size_t size = message->serialize(buffer);
  delete message;
  boost::unique_lock<boost::mutex> lock {outboxmtx};
  binOutbox->push(std::make_pair(size,buffer));
  outboxcond.notify_one();
}

void NetworkedClient::send() {
  while (true) {
    boost::unique_lock<boost::mutex> lock {outboxmtx};
    while (binOutbox->empty() ) { 
      outboxcond.wait(lock);
    }
    auto pair = binOutbox->front();
    binOutbox->pop();
    boost::mutex* m = lock.release();
    m->unlock();
    outboxcond.notify_one();
    
    sendMessage(pair.first, pair.second);
  }
}

void NetworkedClient::sendMessage(Message* message) {
  char* buffer = nullptr;
  size_t size = message->serialize(buffer); 
  delete message;
  sendMessage(size, buffer);
}
 
void NetworkedClient::sendMessage(size_t size, char* buffer) {
  if (not socket->IsOk()) {
    BOOST_LOG_TRIVIAL(error) 
    << "NetworkedClient::sendMessage, vor socket->Write Abbruch: "
    << socket->GetErrorText();
    cout <<"\ncout NetworkedClient::sendMessage,vor socket->Write Abbruch: "
    << socket->GetErrorText();
    delete[] buffer;
    throw std::exception();
  }
  bool written = socket->Write(buffer, size); 
  // virtual bool Write( void const* buf, size_t size ) = 0;
  
  if (!written) {
    cout << "\n!written: " << "" << " !written";
    BOOST_LOG_TRIVIAL(error) 
    << "NetworkedClient::sendMessage,nach socket->Write Abbruch: " 
    << socket->GetErrorText();
    cout 
    << "\ncout NetworkedClient::sendMessage,nach socket->Write Abbruch: " 
    << socket->GetErrorText();
    delete[] buffer;
    throw std::exception();
  }

  delete[] buffer;
 
}


 
}
