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

[1] Implementation of DistributeStreamWorker.

[toc]

1 DistributeStreamWorker class implementation
For detailed information refer to ~DistributeStreamWorker.cpp~.

*/

#ifndef _DISTRIBUTE_STREAM_WORKER_H_
#define _DISTRIBUTE_STREAM_WORKER_H_

#include <queue>
#include <boost/thread.hpp>
#include "Queue.h"

class Socket;
class TupleDescr;

namespace cstream {

class DistributeStreamWorker {

    public:
        DistributeStreamWorker(Socket* serverConnection, 
                const bool providedFormat);

        ~DistributeStreamWorker();

        void pushTuple(VTuple* tuple);

        bool run();

        bool isConnected();

    private:
        int communicate();

        void sendTuple(std::iostream& io, Tuple* tuple);

        Tuple* projectTuple(VTuple* vt);

        static bool filterTuple(Tuple* tuple, QueryProcessor* qp, OpTree& tree, 
                                ArgVectorPointer& funargs);

        void sendQueue(QueryProcessor* qp, OpTree& tree, 
                       ArgVectorPointer& funargs, std::iostream& io);

        void sendQueue(std::iostream& io);

/*
2 Member Definitions

2.1 selTD

Stores the TupleDescr, a client chooses. All tuples fit to this
TupleDescr have to be send to a conneceted client.

*/
        TupleDescr* _selTD;
/*
2.2 serverConnection

Stores current connection to the client.

*/
        Socket* _serverConnection;
/*
2.3 providedFormat

Stores the format that is provided by the server. The client can only request 
the provided format. In the case the client request another format the 
DistirbuteStreamWorkter closes the connection.

*/
        const bool _providedFormat;
/*
2.4 providedFormat

This is the worker thread which is usesd to communicate with the client 

*/
        boost::thread* _worker;
/*
2.4 queue

Is a queue of tuples to send to the client, if a tuple fit to 
the selection creteria and is projectable to the the selected 
TupleDescr.

*/
        Queue<VTuple> _queue;
/*
2.5 newTupleGueard

Is used to wait for new tuples in the queue.

*/
        boost::mutex _newTupleGuard;
/*
2.5 newTupleCV

Is used to wait for new tuples in the queue.

*/
        boost::condition_variable _newTupleCV;

/*
2.6 tupleType

Stores the requested type in nested list format

*/
        ListExpr _tupleType;
};

} /* namespace cstream */

#endif // _DISTRIBUTE_STREAM_WORKER_H_