/*
Shared memory barrier, used by OpBarrier.h/.cpp

*/

#include "Barrier.h"
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>

namespace temporal2algebra {

using boost::interprocess::scoped_lock;
using boost::interprocess::named_mutex;
using namespace boost::interprocess;
using namespace std;

Barrier::Barrier(std::string id, int num_of_procs) :
                      barrier_create_guard(boost::interprocess::open_or_create,
                                "mtx_Barrier_CreateGuard"),
                                shm_name("temporal2_barrier_" + id){

    scoped_lock<named_mutex>(barrier_create_guard);
    try {
        shm_obj = shared_memory_object(create_only,
                shm_name.c_str(),
                read_write );

        shm_obj.truncate(sizeof(ShmBarrier));

        region = mapped_region(shm_obj, read_write);
        void* addr = region.get_address();
        shmBarrierPtr = new (addr) ShmBarrier(num_of_procs);
    } catch (const boost::interprocess::interprocess_exception ex) {
        cout << "couldn't create shm_obj - someone else did. Try to open.\n";
        cout << ex.what() << endl;

        shm_obj = shared_memory_object(open_only,
                shm_name.c_str(),
                read_write );
        region = mapped_region(shm_obj, read_write);
        void * addr = region.get_address();
        shmBarrierPtr = static_cast<ShmBarrier*>(addr);

        if (shmBarrierPtr->num_procs_to_wait_for != num_of_procs) {
            cout << "Found existing Barrier with num_of_procs: "
                    << shmBarrierPtr->num_procs_to_wait_for
                    << ", but requested num_of_procs: "
                    << num_of_procs << endl;
        }
    } catch (const exception ex) {
        cout << "Ups: MemUpdateStorage::initialize()"
                " - unhandled exeption:\n"
                << ex.what() << endl;
        throw;
    }
    shmBarrierPtr->numOfUsers++;
}

Barrier::~Barrier() {
    scoped_lock<named_mutex>(barrier_create_guard);
    shmBarrierPtr->numOfUsers--;
    if (shmBarrierPtr->numOfUsers == 0) {
        cout << "we are the last remaining user, cleaning up...\n";
        bool shm_remove = shared_memory_object::remove(shm_name.c_str());
        std::cout << "shm_remove = " << shm_remove << std::endl;
    } else {
        cout << "there are other users - skip cleanup...\n";
    }
}

int Barrier::wait() {
    cout << "Barrier::wait()\n";
    scoped_lock<interprocess_mutex> lock(shmBarrierPtr->mutex);
    shmBarrierPtr->num_waiting++;
    // the lock is single use: always let pass,
    // once num_procs_to_wait_for has been reached
    if (shmBarrierPtr->num_waiting >= shmBarrierPtr->num_procs_to_wait_for) {
        shmBarrierPtr->cond_everyone_arrived.notify_all();
    } else {
        shmBarrierPtr->cond_everyone_arrived.wait(lock);
    }
    return shmBarrierPtr->num_waiting;
}

} /* namespace temporal2algebra */
