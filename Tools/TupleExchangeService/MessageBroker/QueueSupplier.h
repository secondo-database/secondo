/*

*/

#ifndef SECONDO_INBOXSUPPLIER_H
#define SECONDO_INBOXSUPPLIER_H


#include <queue>

#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "../typedefs.h"

namespace distributed3 {
  // TODO besser InboxSupplier? Inbox ist allerdings mehr als nur eine queue.
  class QueueSupplier { // queue ist nur die interne Repräsentation
  public:
    virtual ~QueueSupplier();
    // TODO getInbox
    
    virtual std::queue<Tuple*>* getQueue(const int eid, const int slot) = 0;
    virtual bool hasTuples(const int eid, const int slot) = 0;
    virtual supplier<Tuple> getTupleSupplier(const int eid, const int slot) = 0;
    virtual bool isFinished(const int eid) = 0;
    virtual void clearInbox(const int eid, const int slot) = 0;
  
  };
}
#endif //SECONDO_QUEUESUPPLIER_H
