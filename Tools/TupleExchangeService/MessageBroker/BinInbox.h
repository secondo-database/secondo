/*

*/

#ifndef SECONDO_BININBOX_H
#define SECONDO_BININBOX_H

#include <ostream>
#include <queue>
#include <map>
#include <functional>
#include "Message.h"
#include "../typedefs.h"
#include <boost/thread.hpp>

namespace distributed3 {
 
class BinInbox {
public:
  BinInbox();
  //~Inbox(); no destructor: nothing to destroy
  // TODO template für Tuple?
  void pushTuple(const int, const int, char*);
  std::queue<char*>* getSwappedInbox(const int eid, const int slot);
  /**
    Destroys the queues associated with eid and slot and 
    clears the inboxes if no queues are left for eid
  */
  void clear(const int eid, const int slot); // besser am 
  // Schluss alles aufräumen?
  
private:
  //std::queue<Tuple*>* getInbox(const int eid, const int slot);
  // TODO template für Tuple/char?
  std::map<int, std::map<int, std::queue<char*>*>> inboxes; // queue auf 
  // heap wg.Speicherplatz?
  std::map<int, std::map<int, std::queue<char*>*>> inboxes2;
  
  std::map<int, std::map<int, boost::mutex>> inboxLocks;
 };
}


#endif //SECONDO_BININBOX_H
