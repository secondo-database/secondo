/*
StreamValve.h
Created on: 21.04.2018
Author: simon

*/

#ifndef ALGEBRAS_TEMPORAL2_STREAMVALVE_H_
#define ALGEBRAS_TEMPORAL2_STREAMVALVE_H_

#include <string>
#include <tr1/memory>

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace temporal2algebra {

const static std::string OpenValveId   ="Open";
const static std::string RandomValveId ="Random";
const static std::string RemoteValveId ="Remote";

class StreamValve;
typedef std::tr1::shared_ptr<StreamValve> StreamValvePtr;

class StreamValve {
public:
    virtual void waitForSignal();
    virtual void sendHasReadSucceeded(bool successful);
    virtual ~StreamValve();
protected:
    StreamValve();
};

class OpenStreamValve : public StreamValve {
};

class RandomStreamValve : public StreamValve{
public:
    RandomStreamValve(int min_wait_sec, int max_wait_sec);
    virtual ~RandomStreamValve();
    virtual void waitForSignal();
private:
    int min_wait_sec;
    int max_wait_sec;
};

struct RemoteValveControl{
    RemoteValveControl() :
        num_request(0),
        request_processed(false),
        num_done(0) {};
    // how many items should the valve let pass
    int num_request;
    // indicates that a request has been processed (as num_done may be 0)
    bool request_processed;
    // how many items were actually let through?
    //any value less than the requested number indicates end of stream
    int num_done;

    boost::interprocess::interprocess_mutex mutex;
    // the valve will wait on this condition to start processing
    boost::interprocess::interprocess_condition cond_wait_for_signal;
    // the next op will wait here after a request has been made
    boost::interprocess::interprocess_condition cond_wait_for_read_result;
};

class RemoteStreamValve : public StreamValve{
public:
    static RemoteStreamValve* create(std::string controllerId);
    virtual ~RemoteStreamValve();

    static int advance(const std::string& controllerId, int advanceCount);
    virtual void waitForSignal();
    virtual void sendHasReadSucceeded(bool successful);
private:
    RemoteStreamValve(std::string controllerId);
    std::string controllerId;
    boost::interprocess::named_mutex mutex;
    boost::interprocess::shared_memory_object shm;
    boost::interprocess::mapped_region region;
    RemoteValveControl* controlPtr;
};

} /* namespace temporal2algebra */

#endif /* ALGEBRAS_TEMPORAL2_STREAMVALVE_H_ */
