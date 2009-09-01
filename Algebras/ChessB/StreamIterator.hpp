#ifndef SECONDO_STREAM_ITERATOR_HPP
#define SECONDO_STREAM_ITERATOR_HPP

#include "QueryProcessor.h"

template< typename T >
struct StreamIterator
{
    StreamIterator( void* arg ) : valid_(false), element_(0), handle_(arg)
    {
        request();
    }

    bool valid() const { return valid_; }

    T* operator*() const
    {
        return element_;
    }

    StreamIterator& operator++()
    {
        request();
        return *this;
    }

private:
    void request()
    {
        if ( handle_ )
        {
            element_ = static_cast< T* >( qp->Request( handle_ ).addr );
            valid_ = qp->Received( handle_ );
        }
    }

    bool valid_;
    T* element_;
    void* handle_;
};

#endif // SECONDO_STREAM_ITERATOR_HPP
