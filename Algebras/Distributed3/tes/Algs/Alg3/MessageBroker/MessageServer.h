/*

*/
#ifndef SECONDO_TES_MESSAGESERVER_H
#define SECONDO_TES_MESSAGESERVER_H

#include <string>
#include <functional>
#include <SocketIO.h>
#include <memory>
#include <queue>
#include <boost/thread.hpp>
//#include "Inbox.h"
#include "Message.h"

#include "../typedefs.h"

namespace distributed2 {
  class MessageServer {

  public:
  explicit MessageServer(std::shared_ptr<Socket> socket );
  ~MessageServer();
  void run();
  void interrupt();
  
  private:
  boost::thread* thread = nullptr;
  std::shared_ptr<Socket> socket;

  void processMessage();
  void handleDataMessage(Message::Header& header);
  void handleFinishedMessage(const int eid);
  
  };
}

#endif //SECONDO_MESSAGESERVER_H
