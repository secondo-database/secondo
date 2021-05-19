/*

*/

#ifndef SECONDO_INBOX_H
#define SECONDO_INBOX_H

#include <ostream>
#include <queue>
#include <map>
#include <functional>
#include "Message.h"
#include "../typedefs.h"
#include <boost/thread.hpp>

namespace distributed3 {

template<typename T>
class Inbox {
public:
  Inbox();
  //~Inbox(); no destructor: nothing to destroy
  void pushTuple(const int, const int, T*);
  std::queue<T*>* getSwappedInbox(const int eid, const int slot);
  /**
    Destroys the queues associated with eid and slot and 
    clears the inboxes if no queues are left for eid
  */
  void clear(const int eid, const int slot); // besser am Schluss 
  // alles aufräumen?
  
private:
  //std::queue<Tuple*>* getInbox(const int eid, const int slot);
  std::map<int, std::map<int, std::queue<T*>*>> inboxes; // queue auf 
  // heap wg.Speicherplatz?
  std::map<int, std::map<int, std::queue<T*>*>> inboxes2;
  // std::map<int, std::map<int, unique_ptr<std::queue<Tuple*>>>>
  //boost::mutex inboxLock;
  std::map<int, std::map<int, boost::mutex>> inboxLocks;
 };
}


#endif //SECONDO_SWITCHABLEBUFFER_H
