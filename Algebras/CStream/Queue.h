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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of datatype Queue and operators.

[toc]

1 Queue class implementation
For detailed information refer to ~Queue.cpp~. This queue is thread safe.

2 Defines and includes


*/

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <queue>
#include <boost/thread.hpp>

namespace cstream {

/*

1.1 Queue

The Queue is thread save. Elements can be added and removed without problems
while running in different threads.

*/

template<class T>
class Queue {

    public:
    
/*
1.1.1 Function Definitions

The functions provided by the Queue class are explained
below.

1.1.1.1 Constructor

*/
        Queue() : _ready(false) {
            
        }

/*
1.1.1.2 Destructor

*/
        ~Queue() {
            boost::unique_lock<boost::mutex> lock(_queueGuard);
            while(!_queue.empty()) {
                _queue.front()->DeleteIfAllowed();
                _queue.pop();
            }
        }

/*
1.1.1.3 push

*/
        void push(T* data) {
            boost::unique_lock<boost::mutex> lock(_queueGuard);
            _queue.push(data);
        }

/*
1.1.1.4 pop

*/
        void pop() {
            boost::unique_lock<boost::mutex> lock(_queueGuard);
            _queue.pop();
        }

/*
1.1.1.5 front

*/
        T* front() {
            boost::unique_lock<boost::mutex> lock(_queueGuard);
            return _queue.front();
        }

/*
1.1.1.6 empty

*/
        bool empty() {
            boost::unique_lock<boost::mutex> lock(_queueGuard);
            return _queue.empty();
        }

/*
1.1.1.7 popfront

*/
        T* pop_front() {
            boost::unique_lock<boost::mutex> lock(_queueGuard);
            T* elem = _queue.front();
            _queue.pop();
            return elem;
        }

/*
1.1.1.8 setReady

*/
        void setReady(bool ready) {
            boost::unique_lock<boost::mutex> lock(_queueReadyGuard);
            _ready = ready;
        }

/*
1.1.1.9 ready

*/
        bool isReady() {
            boost::unique_lock<boost::mutex> lock(_queueReadyGuard);
            return _ready;
        }

    private:

/*

1.1.2 Member Definitions

1.1.2.1 queueGuard

*/
        boost::mutex _queueGuard;

/*
1.1.2.1 queueGuard

*/
        boost::mutex _queueReadyGuard;

/*
1.1.2.2 queue

*/
        std::queue<T*> _queue;

/*
1.1.2.3 ready

*/
        bool _ready;


};

} /* namespace cstream */

#endif // _QUEUE_H_