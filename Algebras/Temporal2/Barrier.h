/*
Shared memory barrier, used by OpBarrier.h/.cpp

*/

#ifndef ALGEBRAS_TEMPORAL2_BARRIER_H_
#define ALGEBRAS_TEMPORAL2_BARRIER_H_

#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace temporal2algebra {

struct ShmBarrier{
    ShmBarrier(int num_of_procs) :
        numOfUsers(0),
        num_waiting(0),
        num_procs_to_wait_for(num_of_procs)
    {};
    int numOfUsers; // use to determine if shared mem can be destroyed

    int num_waiting; // currently waiting processes
    int num_procs_to_wait_for; // number of procs to join
    boost::interprocess::interprocess_mutex mutex;
    boost::interprocess::interprocess_condition cond_everyone_arrived;
};

class Barrier {
public:
    Barrier(std::string id, int num_of_procs);
    virtual ~Barrier();
    int wait();
private:
    boost::interprocess::named_mutex barrier_create_guard;
    std::string shm_name;
    boost::interprocess::shared_memory_object shm_obj;
    ShmBarrier* shmBarrierPtr;
    boost::interprocess::mapped_region region;
};

} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_BARRIER_H_ */
