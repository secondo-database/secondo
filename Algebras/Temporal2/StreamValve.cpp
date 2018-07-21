/*
StreamValve.cpp
Created on: 21.04.2018
Author: simon

Limitations and Todos:
Refactor:
- Provide Methods for TypeMapping checks
- Create Factory to create StreamValves
RemoteStreamValve:
- Handle multi streamnext clients for the same ID correctly
    (currently all except one will remain blocked)
- Handle multi streamvalves for the same ID in a sensible way
    (Producer/Consumer pattern? First one wins, others throw?)
- Handle shm cleanup/recovery
- Avoid pollution of shm with individual Ids / ensure cleanup
- (optional) Handle multi-database case

*/

#include <iostream>
#include "StreamValve.h"

#include <cstdlib>
#include <unistd.h>

using namespace std;
using namespace boost::interprocess;

namespace temporal2algebra {

StreamValve::StreamValve() {
    std::cout << "StreamValve()\n";
}

StreamValve::~StreamValve() {
    std::cout << "~StreamValve()\n";
}

void StreamValve::waitForSignal() {
    std::cout << "StreamValve::waitForSignal()\n";
}

void StreamValve::sendHasReadSucceeded(bool success) {
    std::cout << "StreamValve::sendHasReadSucceeded("<< success << ")\n";
}


RandomStreamValve::RandomStreamValve(int min_wait_sec, int max_wait_sec)
    : min_wait_sec(min_wait_sec),
      max_wait_sec(max_wait_sec) {
    std::cout << "RandomStreamValve ("
            << min_wait_sec << ", "
            << max_wait_sec << ")\n";
};

RandomStreamValve::~RandomStreamValve() {
    std::cout << "~RandomStreamValve()\n";
}

void RandomStreamValve::waitForSignal() {
    std::cout << "RandomStreamValve::waitForSignal()\n";

    int random_integer = rand() %
            (max_wait_sec - min_wait_sec + 1) + min_wait_sec;

    std::cout << "will sleep for " << random_integer << "secs\n";
    sleep(random_integer);
    std::cout << "done sleeping  " << random_integer << "secs\n";
}




RemoteStreamValve* RemoteStreamValve::create(std::string controllerId) {
    std::cout << "RemoteStreamValve::create(" << controllerId << ")\n";

    // if we have a dangling named_mutex this will remove it, but:
    // if there is another process actually using the same,
    // we'll be in trouble
    bool res_remove = named_mutex::remove(("mutex_" + controllerId).c_str());
    std::cout << "res_remove = " << res_remove << std::endl;

    bool shm_remove = shared_memory_object::remove(
            ("shm_" + controllerId).c_str());

    std::cout << "shm_remove = " << shm_remove << std::endl;

    return new RemoteStreamValve(controllerId);

}

// This may throw!
RemoteStreamValve::RemoteStreamValve(std::string controllerId) :
        controllerId(controllerId),
        mutex(create_only, ("mutex_" + controllerId).c_str()),
        shm(create_only         //only create
              ,("shm_" + controllerId).c_str()         //name
              ,read_write       //read-write mode
              )
{
    std::cout << "RemoteStreamValve (" << controllerId << ")\n";
    //try...
    shm.truncate(sizeof(RemoteValveControl));
    region = mapped_region(shm, read_write);
    void* addr = region.get_address();
    controlPtr = new (addr) RemoteValveControl;
};

RemoteStreamValve::~RemoteStreamValve() {
    std::cout << "~RemoteStreamValve()\n";
    bool res_remove = named_mutex::remove(("mutex_" + controllerId).c_str());
    std::cout << "res_remove = " << res_remove << std::endl;

    bool shm_remove = shared_memory_object::remove(
            ("shm_" + controllerId).c_str());

    std::cout << "shm_remove = " << shm_remove << std::endl;
}


int RemoteStreamValve::advance(
        const std::string& controllerId,
        int advanceCount) {
    std::cout << "RemoteStreamValve::advance("
            << controllerId << ", "
            << advanceCount << ")\n";

    int num_read = 0;
    try{
        // code duplication ahead (c.f. StreamValve.cpp/.h)
      shared_memory_object shm
         (open_only
         ,("shm_" + controllerId).c_str()
         ,read_write
         );

      mapped_region region
              (shm        //What to map
              ,read_write //Map it as read-write
              );

      void * addr       = region.get_address();

      RemoteValveControl* control = static_cast<RemoteValveControl*>(addr);

      cout << "StreamNext_vm: control->num_request: "
              << control->num_request << "\n"
              << "StreamNext_vm: control->num_done:    "
              << control->num_done << "\n";

      {
          scoped_lock<interprocess_mutex> lock(control->mutex);
          while (1) {
              // There's work left in the queue,
              // wait until processed and get actual read items
              if (control->num_request) {
                  control->cond_wait_for_read_result.wait(lock);
                  continue;
              }

              // Empty queue, but no result -> put some work in the queue
              if (!control->request_processed) {
                  control->num_request = advanceCount;
                  control->num_done = 0; // should be zero anyway
                  control->cond_wait_for_signal.notify_all();
                  control->cond_wait_for_read_result.wait(lock);
                  continue;
              }

              // empty request queue but result received:
              control->request_processed = false;
              num_read = control->num_done;
              control->num_done = 0;
              break;
          }
          control->cond_wait_for_signal.notify_all();
      } // end scope for lock
    }
   catch(interprocess_exception &ex){
        std::cout << ex.what() << std::endl;
        num_read = -1;
   }

   return num_read;
}


void RemoteStreamValve::waitForSignal() {
    std::cout << "RemoteStreamValve::waitForSignal()\n";

    std::cout << "ToDo: wait for remote Controller "
            << controllerId
            << ". For now just sleep a bit\n";
   // sleep(1);
    { // scope for lock
        scoped_lock<interprocess_mutex> lock(controlPtr->mutex);
        while (1) {
            if (!controlPtr->num_request) {
                controlPtr->cond_wait_for_signal.wait(lock);
                continue;
            }
            break;
        }
    }
    std::cout << "done sleeping\n";
}

void RemoteStreamValve::sendHasReadSucceeded(bool success) {
    std::cout << "RemoteStreamValve::sendHasReadSucceeded("
            << success << ")\n";
    { // scope for lock
            scoped_lock<interprocess_mutex> lock(controlPtr->mutex);
            if (success) {
                controlPtr->num_request--;
                controlPtr->num_done++;
            } else {
                controlPtr->num_request=0;
            }

            if (controlPtr->num_request <= 0) {
                controlPtr->request_processed = true;
            }
            controlPtr->cond_wait_for_read_result.notify_all();
    }
}


} /* namespace temporal2algebra */
