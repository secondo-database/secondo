/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//characters  [1] verbatim: [\verb|]  [|]

January 2008, V. Ferderer 

February 2008, M. Spiekermann. Minor modifications and documentation.


1 Class StreamIterator 

The template class below can be used to iterate over input streams. It can be
typically used in operator implementations which operate on streams. It
encapsulates calls to the query processor. Since this is done very often in
many operator implementations it is a helpful tool for code reduction.

*/

#ifndef SECONDO_STREAM_ITERATOR_H
#define SECONDO_STREAM_ITERATOR_H

#include "QueryProcessor.h"

template< typename T >
struct StreamIterator
{

/*
 
2 Constructors

The empty Constructor does nothing while the other ones request for the first
element of a stream.

*/	
    StreamIterator() : valid_(false), element_(0), handle_(0)
    {}

    StreamIterator( void* arg ) : valid_(false), element_(0), handle_(arg)
    {
        request();
    }

    StreamIterator( Word& arg ) : valid_(false), element_(0), handle_(arg.addr)
    {
        request();
    }

    void init( Word& arg ) { handle_ = arg.addr; } 

    bool valid() const { return valid_; }

/*
3 Overloaded Operators

The current value of an iterator "it"[1] can be referenced by the prefix star
operator and the next value can be requested by the postfix-"++" operation,
e.g. "it++"; example code for a stream iteration:

----
  StreamIterator<Tuple>( arg[0] );
  while( it.valid() ) {
    cout << *it << endl;
    it++;
  }
----

*/
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

#endif // SECONDO_STREAM_ITERATOR_H
