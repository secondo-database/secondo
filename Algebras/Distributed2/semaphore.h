/*

'Semaphore' for threads.

*/

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>    
#include <boost/thread/lock_types.hpp>


class semaphore
{
    unsigned int count_;
    boost::mutex mutex_;
    boost::condition_variable condition_;

public:
    explicit semaphore(unsigned int initial_count) 
       : count_(initial_count),
         mutex_(), 
         condition_()
    {
    }

    void up() {
        boost::unique_lock<boost::mutex> lock(mutex_);
        ++count_;
        condition_.notify_one(); 
    }

    void down() {
        boost::unique_lock<boost::mutex> lock(mutex_);
        while (count_ == 0)
        {
             condition_.wait(lock);
        }
        --count_;
    }
};

#endif
