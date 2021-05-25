/*

*/

#include "BinInbox.h"
#include <boost/log/trivial.hpp>

using namespace std;

namespace distributed3 {

 BinInbox::BinInbox() : inboxes{}, inboxes2{} {}
   
  // sollen wir nicht besser am Schluss alles aufräumen?
  void BinInbox::clear(const int eid, const int slot) {
    delete inboxes[eid][slot];
    delete inboxes2[eid][slot];
    if (inboxes[eid].empty()) {
      inboxes[eid].clear();
      inboxes2[eid].clear();
      inboxLocks[eid].clear();
    }
  }
  /*
  void Inbox::clear(const int eid) {
    for (auto it = inboxes[eid].begin(); it != inboxes[eid].end(); ++it) {
      delete (*it).second;
    }
    inboxes[eid].clear();
    for (auto it = inboxes2[eid].begin(); it != inboxes2[eid].end(); ++it) {
      delete (*it).second;
    }
    inboxes2[eid].clear();
    inboxLocks[eid].clear();
  }
  */
  void BinInbox::pushTuple(const int eid, const int slot, char* tuple) {
    boost::lock_guard<boost::mutex> guard(inboxLocks[eid][slot]); 
    // wenn sichergestellt werden könnte, dass die queue schon 
    // existiert, könnte direkt darauf zugegriffen werden
    // und dadurch das Lock feiner sein. Es müsste also nur diese 
    // queue gesperrt werden.
    // Dann bräuchte es einen mutex für jede queue, der am Ende 
    // wieder zerstört werden müsste.
    //auto queue = inboxes[eid][slot];
    //auto queue = getInbox(eid,slot);
    //if (!queue) {
    //  queue = new std::queue<Tuple*> {};
    //  inboxes[eid][slot] = queue;
    //}
    //queue->push(tuple);
    
    if (!inboxes[eid][slot]) { 
      inboxes[eid][slot] = new std::queue<char*> {}; // braucht es 
      //die Initialisierung mit {} ?
    }
    inboxes[eid][slot]->push(tuple);  
  }
  // TODO besser getInbox. this is not the one and only queue, 
  // but we needn't tell.
std::queue<char*>* BinInbox::getSwappedInbox(const int eid, const int slot) {
   /*
   boost::lock_guard<boost::mutex> guard(inboxLock); // inboxLock 
   // könnte feingranulärer sein.
   
   auto result = inboxes[eid][slot];
   
   inboxes[eid][slot] = new std::queue<Tuple*>{};
   if (!result) { // why can this happen?
     return new queue<Tuple*>{};
   }
   cout << "\nGröße der Inbox: " << result->size();
   return result;
   */
   /* Alternative Implementierung, bei der nicht 
      immer neue queues erzeugt werden.
      Am Ende des Exchange müssen die queues gelöscht werden. 
      Dazu ist eid erforderlich.
      inboxes[eid].begin() -> pair<const key_type, mapped_type>
      inboxes2[eid].begin() -> pair<const key_type, mapped_type>
      Wie teuer sind immer wieder neue queues?
      Am Ende des Exchange müssen die queues zerstört werden. 
      Wo kann das initiiert werden?
      Wo liegen die dazu nötigen Informationen? 
      Es gibt inboxes für die slots 1,6,11,16,...
   */
   boost::lock_guard<boost::mutex> guard(inboxLocks[eid][slot]);
   auto result = inboxes[eid][slot]; // can be nullptr if 
   //there has never been pushed a tuple.
  
   inboxes[eid][slot] = new std::queue<char*>{};
   
   if (!result) {
     return new std::queue<char*>{};
   }
   return result;
 }
 
 // used by an old version of push
 /*
 std::queue<Tuple*>* Inbox::getInbox(const int eid, const int slot) {
   if (not inboxes[eid][slot]) { // used by push. 
   //Initially the queue does not yet exist
     inboxes[eid][slot] = new std::queue<Tuple*> {};
   }
   return inboxes[eid][slot];
 }
 */
  /*
  void Inbox::push(std::shared_ptr<Message> message) {
    int eid = message->getEID();
    int slot = message->getSlot();
    boost::lock_guard<boost::mutex> guard(inboxLock); // lock erst hier nötig
    auto queue = getInbox(eid,slot);
    // eid, slot, workerNumber sind redundant und werden 
    // hier nicht mehr benötigt
    queue->push(message->getBody());
  }
 */

}
