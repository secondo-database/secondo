

/*



*/



#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>    
#include <boost/thread/lock_types.hpp>


class semaphore
{
    //The current semaphore count.
    unsigned int count_;

    //mutex_ protects count_.
    //Any code that reads or writes the count_ data must hold a lock on
    //the mutex.
    boost::mutex mutex_;

    //Code that increments count_ must notify the condition variable.
    boost::condition_variable condition_;

public:
    explicit semaphore(unsigned int initial_count) 
       : count_(initial_count),
         mutex_(), 
         condition_()
    {
    }

    unsigned int get_count() //for debugging/testing only
    {
        //The "lock" object locks the mutex when it's constructed,
        //and unlocks it when it's destroyed.
        boost::unique_lock<boost::mutex> lock(mutex_);
        return count_;
    }

    void signal() //called "release" in Java
    {
        boost::unique_lock<boost::mutex> lock(mutex_);

        ++count_;

        //Wake up any waiting threads. 
        //Always do this, even if count_ wasn't 0 on entry. 
        //Otherwise, we might not wake up enough waiting threads if we 
        //get a number of signal() calls in a row.
        condition_.notify_one(); 
    }

    void wait() //called "acquire" in Java
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        while (count_ == 0)
        {
             condition_.wait(lock);
        }
        --count_;
    }

};
