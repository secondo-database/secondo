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

[1] Implementation of class DistributeStreamWorker.

[toc]

1 DistributeStreamWorker implementation

The DistributeStreamWorker gets a client connection and answer with a 
requested stream of tuples. For this a clients needs to send a correct 
request. The DistributeStreamWorker is able to project an filter the 
tuples if requested by the clients. The protocol of the communication is 
described in ~DistributeStreamProtocoll.cpp~. The DistributeStreamWorker 
can send the tuples in binary and in text format.
The vtuples are stored by the DistributeStreamWorker in a queue.

2 Includes

*/
#include "SocketIO.h"
#include "Algebras/DBService/CommunicationUtils.hpp"

#include "DistributeStreamProtocol.h"
#include "VTHelpers.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "TupleDescr.h"
#include "VTuple.h"

#include "DistributeStreamWorker.h"
#include "Queue.h"

using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;

namespace cstream {

/*
1.1 Constructor

Creates a new object of the DistributeStreamWorker with an existing 
connection and a providedFormat.

*/
DistributeStreamWorker::DistributeStreamWorker(Socket* serverConnection, 
        const bool providedFormat) :
    _serverConnection(serverConnection), 
    _providedFormat(providedFormat), 
    _worker(0) {
    LOG << "DistributeStreamWorker::DistributeStreamWorker" << ENDL;
    LOG << "Initializing DistributeStreamWorker" << ENDL;
}

/*
1.2 Destructor

The worker thread will be terminated and the connection will be closed.

*/
DistributeStreamWorker::~DistributeStreamWorker() {
    LOG << "DistributeStreamWorker::~DistributeStreamWorker" << ENDL;
    if(_worker) {
        _worker->interrupt();
        _worker->join();
    }
    if(isConnected())
        _serverConnection->ShutDown();
}

/*
1.3 Function Definitions

The functions provided by the DistributeStreamWorker class are explained
below.

1.3.1 run

This function starts the worker thread. The thread is used to handle the 
communication with the client. Only one worker thread can be started for 
a DistributeStreamWorker.

*/

bool DistributeStreamWorker::run() {
    // start a new thread
    if(!_worker) {
        _worker = new boost::thread(
            boost::bind(&DistributeStreamWorker::communicate, this));
        return true;
    }

    if(!_worker->joinable()) {
        delete _worker;
        _worker = new boost::thread(
            boost::bind(&DistributeStreamWorker::communicate, this));
        return true;
    }

    return false;
}

/*
1.3.2 communicate

This function is running in a separate thread to handle the communication 
with a connected client. The communication protocol is documented in 
~DistributeStreamProtocol.cpp~. In this function the worker thread get the 
vtuples of the queue, filter and project them as requested by the client 
and sends the filtered and projected tuples to the client in the requested 
format. If the request of the client has an error, the worker closes the 
connection.
If the queue is empty, the worker thread waits for new vtuple and has to be 
notified.

*/
int DistributeStreamWorker::communicate() {
    LOG << "DistributeStreamServer::communicate" << ENDL;
    iostream& io = _serverConnection->GetSocketStream();
    string request;

    DBService::CommunicationUtils::receiveLine(io, request);
    LOG << "Query to the server: " << ENDL;
    LOG << request << ENDL;

    // If the client request the supported types, the client can choose one
    if(request == DistributeStreamProtocol::requestSupportedTypes()) {
        DBService::CommunicationUtils::sendLine(io, 
            DistributeStreamProtocol::sendSupportedTypes(_providedFormat));

        DBService::CommunicationUtils::receiveLine(io, request);
    }

    string tupledesc;
    ListExpr funList;
    bool format;
    if(DistributeStreamProtocol::requestStream(
        request, tupledesc, funList, format) 
        && format == _providedFormat) {

        bool filter = !(funList == nl->TheEmptyList());

        // the TupleDescr to project the stream
        try {
            _selTD = new TupleDescr(tupledesc);
            _tupleType = nl->OneElemList(
                        SecondoSystem::GetCatalog()->NumericType(
                        nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                            _selTD->GetTupleTypeExpr())));
        }
        catch(SecondoException&) {
            DBService::CommunicationUtils::sendLine(io, 
                        DistributeStreamProtocol::confirmStream(
                            "Requested TupleDescr was not OK"));
            LOG << "Requested TupleDescr was not OK" << ENDL;
            _serverConnection->ShutDown();

            return 0;
        }                

        if(filter) {
            OpTree tree = 0;
            QueryProcessor *qp = new QueryProcessor(nl,
                SecondoSystem::GetAlgebraManager());

            // Construct the operator tree of the function
            // LOG << "Contruct" << ENDL;
            bool correct        = false;
            bool evaluable      = false;
            bool defined        = false;
            bool isFunction     = false;
            ListExpr resultType;
            try {
                qp->Construct( funList,
                            correct,
                            evaluable,
                            defined,
                            isFunction,
                            tree,
                            resultType );
            }
            catch(SI_Error ERR_IN_QUERY_EXPR) {
                 DBService::CommunicationUtils::sendLine(io, 
                        DistributeStreamProtocol::confirmStream(
                            "Error in the filter function"));
                _serverConnection->ShutDown();
                delete qp;

                return 0;
            }

            if(!correct || !defined || !isFunction) {
                DBService::CommunicationUtils::sendLine(io, 
                        DistributeStreamProtocol::confirmStream(
                            "Filterfunction was not OK"));
                _serverConnection->ShutDown();
                qp->Destroy(tree, true);
                delete qp;

                return 0;
            }

            ArgVectorPointer funargs = qp->Argument(tree);
            
            DBService::CommunicationUtils::sendLine(io, 
                        DistributeStreamProtocol::confirmStreamOK());

            _queue.setReady(true);
            
            while(true) {
                try {
                    boost::unique_lock<boost::mutex> lock(_newTupleGuard);
                    _newTupleCV.wait(lock);

                    sendQueue(qp, tree, funargs, io);
                }
                catch (boost::thread_interrupted) {
                    sendQueue(qp, tree, funargs, io);

                    DBService::CommunicationUtils::sendLine(io, 
                        DistributeStreamProtocol::streamDone());

                    qp->Destroy(tree, true);
                    delete qp;

                    return 0;
                }
            }
        }
        else {
            DBService::CommunicationUtils::sendLine(io, 
                        DistributeStreamProtocol::confirmStreamOK());

            _queue.setReady(true);
            
            while(true) {
                try {
                    boost::unique_lock<boost::mutex> lock(_newTupleGuard);
                    _newTupleCV.wait(lock);

                    sendQueue(io);
                }
                catch (boost::thread_interrupted) {
                    sendQueue(io);

                    DBService::CommunicationUtils::sendLine(io, 
                        DistributeStreamProtocol::streamDone());

                    return 0;
                }
            }
        }
    }
    else {
        DBService::CommunicationUtils::sendLine(io, 
                    DistributeStreamProtocol::confirmStream(
                        "Request was not OK"));
        LOG << "Request not OK" << ENDL;
        _serverConnection->ShutDown();
    }
    
    return 0;
}

/*

1.3.3 pushTuple

Push a tuple to the queue and notifies the waiting thread.

*/
void DistributeStreamWorker::pushTuple(VTuple* tuple) {
    LOG << "DistributeStreamWorker::pushTuple" << ENDL;
    if(_queue.isReady()) {
        _queue.push(tuple);
        _newTupleCV.notify_all();
    }
}

/*
1.3.4 projectTuple

This function projects a given vtuple to a tuple matching to the given 
tupledescr. If the vtuple cannot be projected the result is a nullpointer. 

*/
Tuple* DistributeStreamWorker::projectTuple(VTuple* vt) {
    bool projected;
    Tuple* tuple = _selTD->ProjectTuple(vt, projected);

    return tuple;
}

/*
1.3.5 filterTuple

This function checks wether the filterfunction is true for a given vtuple.
True is returned when the result of the function is true.

*/
bool DistributeStreamWorker::filterTuple(Tuple* tuple, 
                                         QueryProcessor* qp, 
                                         OpTree& tree, 
                                         ArgVectorPointer& funargs) {
    (*funargs)[0] = tuple;
    Word result;
    qp->EvalS(tree, result, REQUEST);
    if (((Attribute*)result.addr)->IsDefined()) {
        return ((CcBool*)result.addr)->GetBoolval();
    }
    return false;
}

/*
1.3.6 sendTuple

This function send a given tuple into the given stream. The function checks 
the requested format. The possible formats are binary and text.

*/
void DistributeStreamWorker::sendTuple(iostream& io, Tuple* tuple) {
    string message;
    if(_providedFormat) {
        message = tuple->WriteToBinStr();
        message = "(" + message + ")";
    }
    else {
        ListExpr tupleValue;
        tupleValue = tuple->Out(_tupleType);
        nl->WriteToString(message, tupleValue);
    }

    DBService::CommunicationUtils::sendLine(io, 
        DistributeStreamProtocol::tupleMessage() + message);
}

/*
1.3.7 isConnected

Tests the connection to the client.

*/
bool DistributeStreamWorker::isConnected() {
    return _serverConnection && _serverConnection->IsOk();
}

/*
1.3.8 sendQueue

Sends the Queue with filtering and projection

*/
void DistributeStreamWorker::sendQueue(QueryProcessor* qp, 
                                       OpTree& tree, 
                                       ArgVectorPointer& funargs,
                                       iostream& io) {
    while(!_queue.empty()) {
        VTuple* vt = _queue.pop_front();

        Tuple* tuple;
        if((tuple = projectTuple(vt))) {
            if(filterTuple(tuple, qp, tree, funargs)) {
                sendTuple(io, tuple);
            }
        }

        vt->DeleteIfAllowed();
    }
}

/*
1.3.9 sendQueue

Sends the Queue with projection but without filtering

*/
void DistributeStreamWorker::sendQueue(iostream& io) {
    while(!_queue.empty()) {
        VTuple* vt = _queue.pop_front();

        Tuple* tuple;
        if((tuple = projectTuple(vt))) {
            sendTuple(io, tuple);
        }

        vt->DeleteIfAllowed();
    }
}

};
