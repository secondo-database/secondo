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

 //paragraph [1] title: [{\Large \bf ] [}]
 //[->] [$\rightarrow$]



 [10] ContinuousUpdateAlgebra

 May 2015 JWH

 This algebra allows distributed query processing by connecting stream
 sockets.

*/

#include "ContinuousUpdate.h"
#include "signal.h"

extern NestedList *nl;
extern QueryProcessor *qp;

using namespace std;
using namespace temporalalgebra;

/*
 Mutex used for locking of nl->WriteStringTo() against nl->ReadFromString()
 which internally use the same memory structures.
 Not using the mutex "could" lead to a crash when
 using providemessages and receivenlstream in the same query.

*/
static std::mutex g_nl_lock;

/*
 ----
 1 Operator ipointstoupoint
 ----

*/
class ipointstoupointLocalInfo {
private:
    Word stream;
    ListExpr tupleType;
    int mergeAttributeIndex;
    int ipointIndex;
    string mergeAttribute;
    queue<Tuple*> mergedQueue;
    vector<Tuple*> mergeCandidates;

    /**
     * Merges the two tuples while constructing a new upoint from two ipoints
     * and using the attributes of the new tuple

    */
    Tuple* mergeTuples(Tuple* oldT, Tuple* newT) {
        Tuple* t = new Tuple(tupleType);

        for (int i = 0; i < t->GetNoAttributes(); i++) {
            if (i == ipointIndex) {
                double oldInstant =
                    ((IPoint*) oldT->GetAttribute(i))->instant.ToDouble();
                double newInstant =
                    ((IPoint*) newT->GetAttribute(i))->instant.ToDouble();
                Point oldPosition =
                    ((IPoint*) oldT->GetAttribute(i)->Clone())->value;
                Point newPosition =
                    ((IPoint*) newT->GetAttribute(i)->Clone())->value;
                UPoint * upoint = new UPoint(
                    Interval<Instant>(oldInstant, newInstant,
                    true, true), oldPosition, newPosition);
                t->PutAttribute(i, upoint);

            } else {
                t->PutAttribute(i, newT->GetAttribute(i)->Clone());
            }
        }

        return t;
    }

    /**
     * Searches for the tuple to merge it

    */
    void proposeMerge(Tuple* tuple) {

        for (vector<Tuple*>::iterator it = mergeCandidates.begin();
            it != mergeCandidates.end(); ++it) {

            if ((*it)->GetAttribute(mergeAttributeIndex)->Compare(
                tuple->GetAttribute(mergeAttributeIndex)) == 0) {
                mergedQueue.push(mergeTuples(*it, tuple));
                (*it)->DeleteIfAllowed();
                it = mergeCandidates.erase(it);
                break;
            }
        }

        tuple->IncReference();
        mergeCandidates.push_back(tuple);

    }

    /**
     * Returns true if a newly constructed tuple is available

    */
    bool tupleAvailable() {
        return mergedQueue.size() > 0 ? true : false;
    }

    /**
     * Get the next tuple from the Queue

    */
    Tuple* getMergedTuple() {

        Tuple* t = mergedQueue.front();
        mergedQueue.pop();
        return t;
    }

public:

    ipointstoupointLocalInfo(Word& pstream, string& pmergeAttribute,
        int& pipointIndex, ListExpr & pstreamType) {
        qp->Open(pstream.addr);
        stream = pstream;
        mergeAttribute = pmergeAttribute;
        tupleType = nl->Second(pstreamType);
        ipointIndex = pipointIndex;

        ListExpr search = AntiNumericType2(tupleType);

        //Determine index of the merge attribute
        for (int i = 1; i <= nl->ListLength(nl->Second(search));
            i++) {

            if (nl->ToString(
                nl->First(nl->Nth(i, nl->Second(search)))).compare(
                mergeAttribute) == 0) {
                mergeAttributeIndex = i - 1;
                break;
            }
        }
    }

    ~ipointstoupointLocalInfo() {
    }

    /**
     * Try to get a newly constructed tuple
     * otherwise wait until a new tuple is available

    */
    Word next() {
        Word tupleWord;
        Tuple * tuple;

        while (!tupleAvailable()) {
            qp->Request(stream.addr, tupleWord);
            if (qp->Received(stream.addr)) {
                tuple = (Tuple*) tupleWord.addr;
                proposeMerge(tuple);
            } else {
                return SetWord((void*) 0);
            }
        }

        return SetWord((void*) getMergedTuple());

    }
};

ListExpr ipointstoupointTM(ListExpr inargs) {

    NList args(inargs);

    if (!args.hasLength(2))
        return args.typeError("Expected 1 argument");

    NList stream_desc = args.first();

    if (!stream_desc.hasLength(2)
        || !stream_desc.first().first().isSymbol(sym.STREAM()))
        return args.typeError("Input is no stream");

    NList tupleDesc = stream_desc.first().second();

    if (!nl->IsEqual(tupleDesc.first().listExpr(), Tuple::BasicType())
        || tupleDesc.length() != 2)
        return args.typeError("No Tuple Description found in stream");

    NList mergeAttribute = args.second();

    if (!CcString::checkType(mergeAttribute.first().listExpr())) {
        return listutils::typeError(
            "second argument must be a string");
    }

    string mergeAttributeName = mergeAttribute.second().str();

    NList attributeList = tupleDesc.second();

    //Check if ther is an attribute with the same name as specified by
    //the parameter
    bool found = false;
    while (!attributeList.isEmpty()) {
        if (mergeAttributeName.compare(
            attributeList.first().first().str()) == 0) {
            found = true;
            break;
        }
        attributeList.rest();
    }

    if (!found)
        return listutils::typeError(
            "second argument must be the name of an attribute");

    // Create result type
    found = false;
    int ipointIndex = 0;
    ListExpr inputAttr = nl->Second(nl->Second(nl->First(nl->First(inargs))));

    // Pointer to the head and the tail of the result list
    ListExpr resultAttr = nl->TheEmptyList();
    ListExpr lastResultAttr = resultAttr;

    while(! (nl->IsEmpty(inputAttr))) {
        ListExpr attr = nl->First(inputAttr);
       inputAttr = nl->Rest(inputAttr);

        // Change first ipoint to upoint
       if(! found) {
           if(nl->IsEqual(nl->Second(attr), IPoint::BasicType())) {
             attr = nl->TwoElemList(
                        nl->First(attr),
                        nl->SymbolAtom(UPoint::BasicType()));

              ipointIndex = nl->ListLength(resultAttr);
              found = true;
          }
       }

        // First call, create new NListExpr.
        // Otherwise append attr to existing list
       if( nl -> IsEmpty(resultAttr) ) {
          resultAttr = nl->OneElemList(attr);
          lastResultAttr = resultAttr;
       } else {
           lastResultAttr = nl->Append(lastResultAttr, attr);
       }
    }

    ListExpr resultType = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
         nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), resultAttr));

    if (!found) {
        return listutils::typeError(
            "One of the attributes of the input stream must be an ipoint");
    }

    return nl->ThreeElemList(
        nl->SymbolAtom(Symbol::APPEND()),
        nl->TwoElemList(nl->StringAtom(mergeAttribute.first().str()),
            nl->IntAtom(ipointIndex)),
       resultType);
}

int ipointstoupointVM(Word* args, Word& result, int message,
    Word& local, Supplier s) {

    ipointstoupointLocalInfo* li =
        (ipointstoupointLocalInfo*) local.addr;

    switch (message) {
    case OPEN: {
        ListExpr tupleType;

        if (li) {
            delete li;
        }

        // Get the expected tupleType
        tupleType = GetTupleResultType(s);

        string mergeAttribute =
            (string) (char*) ((CcString*) args[1].addr)->GetStringval();

        int ipointAttributeIndex =
            ((CcInt*) args[3].addr)->GetIntval();

        li = new ipointstoupointLocalInfo(args[0], mergeAttribute,
            ipointAttributeIndex, tupleType);

        local.addr = li;
        return 0;
    }
    case REQUEST: {
        result.addr = li ? li->next().addr : 0;
        return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
        if (li) {
            delete li;
            local.addr = 0;
        }
        return 0;
    }
    default:
        assert(false);
        return 0;
    }
}

OperatorSpec ipointstoupointSpec("stream -> stream",
    "ipointstoupoint",
    "Merges two tuples while converting one attribute"
    " of the type ipoint to a upoint",
    "query Orte feed ipointstoupoint count");

/*
 ----
 1 Operator sendmessages
 ----

*/
class sendmessagesLocalInfo {
private:
    Word stream;
    ListExpr tupleType;
    MessageCenter* msg;
    public:

    sendmessagesLocalInfo(Word& pstream, ListExpr & pstreamType) {
        qp->Open(pstream.addr);
        msg = MessageCenter::GetInstance();
        stream = pstream;
        tupleType = nl->OneElemList(nl->Second(pstreamType));
    }

    ~sendmessagesLocalInfo() {
    }

    /**
     * Get the next tuple from the QP
     * and send it to the client by using the MessageCenter

    */
    Word next() {
        Word tupleWord;

        qp->Request(stream.addr, tupleWord);
        if (qp->Received(stream.addr)) {

            // Convert the received tuple to its ListExpr
            Tuple * tuple = (Tuple*) tupleWord.addr;
            ListExpr tuple_list = tuple->Out(tupleType);

            // Send the ListExpr of the Tuple to the client
            msg->Send(nl, tuple_list);
            msg->Flush();

            return SetWord((void*) tuple);
        } else {
            return SetWord((void*) 0);
        }
    }
};

ListExpr sendmessagesTM(ListExpr inargs) {

    NList args(inargs);

    if (!args.hasLength(1))
        return args.typeError("Expected 1 argument");

    NList stream_desc = args.first();

    if (!stream_desc.hasLength(2)
        || !stream_desc.first().first().isSymbol(sym.STREAM()))
        return args.typeError("Input is no stream");

    ListExpr tuple_desc = stream_desc.first().second().listExpr();

    if (!nl->IsEqual(nl->First(tuple_desc), Tuple::BasicType())
        || nl->ListLength(tuple_desc) != 2)
        return args.typeError("No Tuple Description found in stream");

    return stream_desc.first().listExpr();
}

int sendmessagesVM(Word* args, Word& result, int message, Word& local,
    Supplier s) {

    sendmessagesLocalInfo* li = (sendmessagesLocalInfo*) local.addr;

    switch (message) {
    case OPEN: {
        ListExpr tupleType;

        if (li) {
            delete li;
        }

        // Get the expected tupleType
        tupleType = GetTupleResultType(s);

        li = new sendmessagesLocalInfo(args[0], tupleType);

        local.addr = li;
        return 0;
    }
    case REQUEST: {
        result.addr = li ? li->next().addr : 0;
        return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
        if (li) {
            delete li;
            local.addr = 0;
        }
        return 0;
    }
    default:
        assert(false);
        return 0;
    }
}

OperatorSpec sendmessagesSpec("stream -> stream", "sendmessages",
    "Send the tuples to the client using the messagecenter",
    "query Orte feed sendmessages count");

/*
 ----
 1 Operator providemessages
 ----

*/
ListExpr providemessagesTM(ListExpr inargs) {

    NList args(inargs);

    if (!args.hasLength(2))
        return args.typeError("Expected 2 argument");

    NList stream_desc = args.first();
    NList port = args.second();

    if (!stream_desc.hasLength(2)
        || !stream_desc.first().first().isSymbol(sym.STREAM()))
        return args.typeError("Input is no stream");

    ListExpr tuple_desc = stream_desc.first().second().listExpr();

    if (!nl->IsEqual(nl->First(tuple_desc), Tuple::BasicType())
        || nl->ListLength(tuple_desc) != 2)
        return args.typeError("No Tuple Description found in stream");

    if (!CcInt::checkType(port.first().listExpr())) {
        return listutils::typeError(
            "second argument must be a portnumber");
    }

    return stream_desc.first().listExpr();
}

/**
 Handels the Client-Connection after it was established with the Server

*/
class providemessageHandler {
private:
    ListExpr streamType, tupleType;
    std::thread thread;
    Socket* client;
    std::atomic<bool> keepRunning;
    moodycamel::ConcurrentQueue<Tuple*> queue;

    /*
     Main thread method

    */
    void run() {
        Tuple * tuple;
        keepRunning = true;

        initalize();

        // Create the iostream and prepare it for exception usage
        iostream & stream = client->GetSocketStream();
        stream.exceptions(
            ifstream::failbit | ifstream::badbit | ifstream::eofbit);

        while (keepRunning) {

            //Get the next tuple from the queue
            if (queue.try_dequeue(tuple)) {
                sendTuple(tuple, stream);

            } else {
                // No tuple could be received,
                //sleep for a short while
                std::this_thread::sleep_for(std::chrono::milliseconds(
                PROVIDEMESSAGES_HANDLER_SLEEP_MS));
            }

        }
    }

    /**
     Converts the provided Tuple to ist ListExpr-Representation
     and sends it over the socket
     A lock on g_nl_lock secures the interaction with nl.

    */
    void sendTuple(Tuple* tuple, iostream & stream) {
        try {
            //The following Block ist used for the lock_guard
            {
                string s;
                std::lock_guard<std::mutex> lock(g_nl_lock);
                ListExpr l = tuple->Out(tupleType);
                //Cannot write directly to the
                //stream because of error handling...
                nl->WriteToString(s, l);
                stream << s << endl;
            }
        } catch (ios_base::failure & e) {
            keepRunning = false;
        }

        tuple->DeleteIfAllowed();
    }

    /**
     * Initalizes the client connection according to the defined protocol

    */
    void initalize() {
        iostream & stream = client->GetSocketStream();
        stream.exceptions(
            ifstream::failbit | ifstream::badbit | ifstream::eofbit);
        string in;

        try {
            getline(stream, in);
            if (in.compare("<GET_TYPE>") == 0) {
                stream << "<TYPE>" << endl;
                stream << nl->ToString(AntiNumericType2(streamType))
                    << endl;
                stream << "</TYPE>" << endl;
            } else {
                throw 20;
            }

            getline(stream, in);
            if (in.compare("</GET_TYPE>") != 0) {
                throw 30;
            }
        } catch (ios_base::failure &e) {
            keepRunning = false;
        } catch (int i) {
            keepRunning = false;
        }
    }
public:
    providemessageHandler(Socket* psocket, ListExpr pstreamType,
        ListExpr ptupleType) {
        queue = moodycamel::ConcurrentQueue<Tuple*>();
        streamType = pstreamType;
        tupleType = ptupleType;
        client = psocket;
    }

    ~providemessageHandler() {
    }

    /**
     Appends the Tuple to the queue

    */
    void appendTuple(Tuple * tuple) {
        tuple->IncReference();
        queue.enqueue(tuple);
    }

    /**
     Start the thread

    */
    void start() {
        thread = std::thread(&providemessageHandler::run, this);
    }

    /**
     Returns true if the handler should be done
     handling the client connection

    */
    bool finished() {
        return !keepRunning;
    }

    /**
     Stop the thread

    */
    void stop() {
        keepRunning = false;
        if (thread.joinable()) {
            thread.join();
        }
    }
};

/*
 Accepts Client-Connections
 After establishing a connection with the client
 a providemessageHandler is created to handle the transfer

*/
class providemessageServer {
private:
    Socket * serverSocket;
    ListExpr streamType, tupleType;
    std::atomic<bool> keepRunning;
    vector<providemessageHandler*> handlers;
    public:
    providemessageServer(int pport, ListExpr pstreamType,
        ListExpr ptupleType) {
        streamType = pstreamType;
        tupleType = ptupleType;
        serverSocket = Socket::CreateGlobal("localhost",
            int2string(pport));
        handlers = vector<providemessageHandler*>();
    }

    ~providemessageServer() {
        serverSocket->Close();
        delete serverSocket;

        for (std::vector<providemessageHandler*>::iterator it =
            handlers.begin(); it != handlers.end(); ++it) {
            delete *it;
        }
        handlers.clear();
    }

    /*
     Wait for new clients and spawn a thread if a client connects

    */
    void acceptConnections() {
        keepRunning = true;
        while (keepRunning) {
            signal(SIGPIPE, SIG_IGN);
            Socket* client = serverSocket->Accept();
            if (client && client->IsOk()) {
                providemessageHandler * newHandler =
                    new providemessageHandler(client, streamType,
                        tupleType);
                handlers.push_back(newHandler);
                newHandler->start();
            }

        }
    }

    /*
     Stop the server

    */
    void stop() {
        keepRunning = false;
        serverSocket->CancelAccept();
        for (std::vector<providemessageHandler*>::iterator it =
            handlers.begin(); it != handlers.end(); ++it) {
            (*it)->stop();
        }
    }

    /*
     Update all connected clients with the new tuple
     Removes handler if it is no longer in use

    */
    void update(Tuple * tuple) {
        std::vector<providemessageHandler*>::iterator it;

        //Provide all handlers with the new tuple
        for (it = handlers.begin(); it != handlers.end();) {

            if ((*it)->finished()) {
                //If a handler has stopped/failed,
                //remove it from the vector
                (*it)->stop();
                delete (*it);
                it = handlers.erase(it);
            } else {
                (*it)->appendTuple(tuple);
                it++;
            }
        }
    }
};

class providemessagesLocalInfo {
private:
    int port;
    ListExpr tupleType, streamType;
    Word stream;
    providemessageServer * server;
    thread localThread;
    public:

    providemessagesLocalInfo(Word& pstream, int& pport,
        ListExpr pstreamType) {
        qp->Open(pstream.addr);
        streamType = pstreamType;
        tupleType = nl->OneElemList(nl->Second(streamType));
        port = pport;
        stream = pstream;
        server = new providemessageServer(port, streamType,
            tupleType);
        localThread = thread(&providemessageServer::acceptConnections,
            server);
    }

    ~providemessagesLocalInfo() {
        server->stop();
        if (localThread.joinable()) {
            localThread.join();
        }
        delete server;
    }

    /*
     Get the next tuple from the QP, and hand it to
     the server for further processing

    */
    Word next() {
        Word tupleWord;
        qp->Request(stream.addr, tupleWord);
        if (qp->Received(stream.addr)) {

            Tuple * tuple = (Tuple*) tupleWord.addr;

            server->update(tuple);

            return SetWord((void*) tuple);
        } else {
            return SetWord((void*) 0);
        }
    }
};

int providemessagesVM(Word* args, Word& result, int message,
    Word& local, Supplier s) {

    providemessagesLocalInfo* li =
        (providemessagesLocalInfo*) local.addr;

    ListExpr resultTupleNL;

    switch (message) {
    case OPEN: {
        if (li) {
            delete li;
        }

        resultTupleNL = GetTupleResultType(s);

        int port = ((CcInt*) args[1].addr)->GetIntval();

        li = new providemessagesLocalInfo(args[0], port,
            resultTupleNL);

        local.addr = li;
        return 0;
    }
    case REQUEST: {
        result.addr = li ? li->next().addr : 0;
        return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
        if (li) {
            delete li;
            local.addr = 0;
        }
        return 0;
    }
    default: {
        assert(false);
        return 0;
    }
    }
}

OperatorSpec providemessagesSpec("stream x int -> stream",
    "_ providemessages [ port ]",
    "Receives a Tuple-Stream and provides a"
        "multithreaded server to send the tuples"
        "to clients who are interested",
    "query Orte feed providemessages[9000] count");

/*
 ----
 1 Operator owntransactioninsert
 ----

*/
ListExpr owntransactioninsertTM(ListExpr inargs) {

    NList args(inargs);

    if (!args.hasLength(2)) {
        return listutils::typeError("expected 2 arguments");
    }

    NList stream_desc = args.first();
    NList relation = args.second();

    if (!stream_desc.hasLength(2)
        || !stream_desc.first().first().isSymbol(sym.STREAM()))
        return listutils::typeError("Input is no stream");

    ListExpr tuple_desc = stream_desc.first().second().listExpr();

    if (!nl->IsEqual(nl->First(tuple_desc), Tuple::BasicType())
        || nl->ListLength(tuple_desc) != 2)
        return listutils::typeError(
            "No Tuple Description found in stream");

    if (!CcString::checkType(relation.first().listExpr())) {
        return listutils::typeError(
            "second argument must be the name of a relation");
    }

//Check if the received tupleType fits the relation
    string relName = relation.second().str();
    SecondoCatalog* catalog = SecondoSystem::GetCatalog();

    if (!catalog->IsObjectName(relName)) {
        return listutils::typeError(relName + " is no valid Relation");
    }

    ListExpr relType;
    Word word;
    string typeName;
    bool defined;
    bool hasTypeName;

    catalog->GetObjectExpr(relName, typeName, relType, word, defined,
        hasTypeName);

    if (!nl->Equal(stream_desc.first().second().listExpr(),
        nl->Second(relType))) {
        return listutils::typeError(
            "Stream-Type and Relation-Type are not equal!");
    }

    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
        nl->OneElemList(nl->StringAtom(relName)),
        stream_desc.first().listExpr());
}

/**
 Implements a thread which caches a bunch op tuples in a queue,
 and periodically inserts them into the given relation

*/
class asyncInsertThread {
private:
    moodycamel::ConcurrentQueue<Tuple*> queue;
    std::atomic<bool> keepRunning;
    SecondoCatalog* catalog;
    Relation* rel;
    string relationName;
    int sleepCounter, tupleCounter;
    Word relationWord;

    /**
     Append the tuple to the queue

    */
    void insertTuples(Relation* r) {
        Tuple * tuple;

        while (queue.try_dequeue(tuple)) {
            r->AppendTuple(tuple);
            tuple->DeleteIfAllowed();
        }
    }

    /**
     Opens the relation

    */
    Relation* openRelation() {
        if (catalog->IsObjectName(relationName)) {
            bool defined;
            Word word;
            catalog->GetObject(relationName, word, defined);
            relationWord = word;
            return (Relation*) word.addr;
        }
        return 0;
    }

    /**
     Ends the current transaction and starts a new one

    */
    void commitAndBegin() {
        SmiEnvironment::CommitTransaction();

        // Begin a new Transaction so the
        // surrounding Query can commit at its end
        SmiEnvironment::BeginTransaction();
    }

    /**
     Inserts the tuples into the relation

    */
    void insertAndCommit(bool commit = true) {

        insertTuples(rel);

        //Needed to update the relation record
        catalog->ModifyObject(relationName, relationWord);
        catalog->CleanUp(false);

        tupleCounter = 0;

        if (commit) {
            commitAndBegin();
        }
    }
public:
    asyncInsertThread(string pRelation) {
        queue = moodycamel::ConcurrentQueue<Tuple*>();
        catalog = SecondoSystem::GetCatalog();
        keepRunning = true;
        relationName = pRelation;
        rel = openRelation();
        sleepCounter = tupleCounter = 0;
    }

    ~asyncInsertThread() {
        rel->Close();
    }

    /**
     Appends the tuple to the queue to insert it later

    */
    void pushTuple(Tuple * t) {
        t->IncReference();
        queue.enqueue(t);
        tupleCounter++;
    }

    /**
     Stop the thread

    */
    void quit() {
        keepRunning = false;
    }

    /**
     Start the thread

    */
    void startInserts() {

        while (keepRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(
            OWNTRANSACTIONINSERT_SLEEP_MS));
            sleepCounter++;
            if ((tupleCounter >
            OWNTRANSACTIONINSERT_COMMIT_TUPLE_COUNT)
                || (sleepCounter % OWNTRANSACTIONINSERT_SLEEP_COUNT)
                    == 0) {

                // Insert the tuples into the relation an commit
                insertAndCommit();
            }
        }

        //Empty the queue, without a commit!
        insertAndCommit(false);
    }
};

class NLStreamOwntransactioninsertLocalInfo {
private:
    string relationName;
    ListExpr tupleType;
    asyncInsertThread * inserter;
    thread insertThread;
    Word stream;

public:

    NLStreamOwntransactioninsertLocalInfo(Word& pstream,
        ListExpr & ptupleType, string& prelationName) {
        qp->Open(pstream.addr);
        stream = pstream;
        tupleType = nl->OneElemList(ptupleType);
        relationName = prelationName;

        // Create and start the inserter
        inserter = new asyncInsertThread(relationName);
        insertThread = thread(&asyncInsertThread::startInserts,
            inserter);
    }

    ~NLStreamOwntransactioninsertLocalInfo() {
        if (insertThread.joinable()) {
            inserter->quit();
            insertThread.join();
            delete inserter;
        }
    }

    /**
     Return the next tuple and send it to the inserter thread

    */
    Word next() {
        Word tupleWord;

        qp->Request(stream.addr, tupleWord);
        if (qp->Received(stream.addr)) {

            Tuple * tuple = (Tuple*) tupleWord.addr;
            inserter->pushTuple(tuple);

            return SetWord((void*) tuple);
        } else {
            return SetWord((void*) 0);
        }
    }
};

int owntransactioninsertVM(Word* args, Word& result, int message,
    Word& local, Supplier s) {

    NLStreamOwntransactioninsertLocalInfo* li =
        (NLStreamOwntransactioninsertLocalInfo*) local.addr;

    switch (message) {
    case OPEN: {
        string relationName;
        ListExpr tupleType;

        if (li) {
            delete li;
        }

        // Get the expected tupleType
        tupleType = nl->Second(GetTupleResultType(s));

        relationName =
            (string) (char*) ((CcString*) args[1].addr)->GetStringval();

        // Start the receiver
        li = new NLStreamOwntransactioninsertLocalInfo(args[0],
            tupleType, relationName);

        local.addr = li;
        return 0;
    }
    case REQUEST: {
        result.addr = li ? li->next().addr : 0;
        return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
        if (li) {
            delete li;
            local.addr = 0;
        }
        return 0;
    }
    default:
        assert(false);
        return 0;
    }
}

OperatorSpec owntransactioninsertSpec("stream x string -> stream",
    "owntransactioninsert [relation]",
    "Inserts the Tuples in a new transaction",
    "query Orte feed owntransactioninsert[\"Orte\"] count");

/*
 ----
 1 Operator receivenlstream
 ----

*/

ListExpr receivenlstreamTM(ListExpr args) {

    if (!nl->HasLength(args, 2)) {
        return listutils::typeError("expected 2 arguments");
    }

    ListExpr host = nl->First(args);
    ListExpr port = nl->Second(args);

    if (!CcString::checkType(nl->First(host))) {
        return listutils::typeError(
            "first argument must be an hostname or ip-address");
    }
    if (!CcInt::checkType(nl->First(port))) {
        return listutils::typeError(
            "second argument must be an portnumber");
    }

    Socket *client = Socket::Connect(
        nl->StringValue(nl->Second(host)),
        int2string(nl->IntValue(nl->Second(port))),
        Socket::SockGlobalDomain);

    if (!client || !client->IsOk())
        return listutils::typeError("unable to connect.");

// Connection initialization procol:
// Client (NLStream):             <GET_TYPE>
// Server (Provides Stream):    <TYPE>
// Server: TupleInfo as NL-String (ex. stream(tuple((Id string))) )
// Server:                        </TYPE>
// Client:                        </GET_TYPE>
    string tupleInfo, msg;

// Initiate Contact
    client->GetSocketStream() << "<GET_TYPE>" << endl;
    getline(client->GetSocketStream(), msg);
    if (msg != "<TYPE>") {
        client->Close();
        return listutils::typeError(
            "unable getting handshake message <TYPE> from the server.");
    }

// Get TupleInfo and parse it
    getline(client->GetSocketStream(), tupleInfo);

    ListExpr streamType;
    if (!nl->ReadFromString(tupleInfo, streamType)) {
        client->Close();
        return listutils::typeError(
            "TupleInfo is no valid NestedList-Expression");
    }

// Finalize initialization
    getline(client->GetSocketStream(), msg);
    if (msg != "</TYPE>") {
        client->Close();
        return listutils::typeError(
            "unable getting handshake-completion "
            "message </TYPE> from the server.");
    }

    client->GetSocketStream() << "</GET_TYPE>" << endl;

    streamType = AntiNumericType2(streamType);

    //Client-Descriptor will not deleted!
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
        nl->TwoElemList(nl->IntAtom(client->GetDescriptor()),
            nl->TextAtom(nl->ToString(streamType))), streamType);
}

class NLStreamReceiveStreamLocalInfo {
private:
    Socket* client;
    string buffer;
    ListExpr tupleType;

public:

    NLStreamReceiveStreamLocalInfo(int& sd, ListExpr & ptupleType) {
        tupleType = nl->OneElemList(ptupleType);
        client = Socket::CreateClient(sd);
    }

    ~NLStreamReceiveStreamLocalInfo() {
        if (client) {
            client->Close();
            delete client;
        }
    }

    /**
     Get the next tuple from the Server

    */
    Tuple* getNextTuple() {
        string input = "";
        ListExpr output, errInfo;
        int errPos = 0;
        bool status = false;
        Tuple* tuple = 0;

        if (readLine(client->GetDescriptor(), input, buffer)) {
            //Parse the line for the NestedList
            std::lock_guard<std::mutex> lock(g_nl_lock);
            nl->ReadFromString(input, output);

            //Create the Tuple
            tuple = Tuple::In(tupleType, output, errPos, errInfo,
                status);
        }

        return tuple;
    }

    /**
     Return the next tuple and send it to the inserter thread

    */
    Word next() {
        // Get the next Tuple
        Tuple* t = getNextTuple();

        // Provide the Tuple if it was received
        if (t) {
            return SetWord((void*) t);
        } else {
            return SetWord((void*) 0);
        }
    }
};

int receivenlstreamVM(Word* args, Word& result, int message,
    Word& local, Supplier s) {

    NLStreamReceiveStreamLocalInfo* receiver =
        (NLStreamReceiveStreamLocalInfo*) local.addr;

    switch (message) {
    case OPEN: {
        SocketDescriptor clientSocketDescriptor;
        ListExpr tupleType;

        if (receiver) {
            delete receiver;
        }

        // Get the expected tupleType
        nl->ReadFromString(((FText*) args[3].addr)->GetValue(),
            tupleType);

        tupleType = SecondoSystem::GetCatalog()->NumericType(
            nl->Second(tupleType));

        // Get the appended Clinet-SD
        clientSocketDescriptor = ((CcInt*) args[2].addr)->GetIntval();

        // Start the receiver
        receiver = new NLStreamReceiveStreamLocalInfo(
            clientSocketDescriptor, tupleType);

        local.addr = receiver;
        return 0;
    }
    case REQUEST: {
        result.addr = receiver ? receiver->next().addr : 0;
        return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
        if (receiver) {
            delete receiver;
            local.addr = 0;
        }
        return 0;
    }
    default:
        assert(false);
        return 0;
    }
}

OperatorSpec receivenlstreamSpec("string x int -> stream",
    "receivenlstream ( host, port )",
    "Receives a Tuple-Stream from the specified server",
    "query receivenlstream(\"localhost\", 9000) count");

Operator sendmessagesOP("sendmessages", sendmessagesSpec.getStr(),
    sendmessagesVM, Operator::SimpleSelect, sendmessagesTM);

Operator providemessagesOP("providemessages",
    providemessagesSpec.getStr(), providemessagesVM,
    Operator::SimpleSelect, providemessagesTM);

Operator receivenlstreamOP("receivenlstream",
    receivenlstreamSpec.getStr(), receivenlstreamVM,
    Operator::SimpleSelect, receivenlstreamTM);

Operator owntransactioninsertOP("owntransactioninsert",
    owntransactioninsertSpec.getStr(), owntransactioninsertVM,
    Operator::SimpleSelect, owntransactioninsertTM);

Operator ipointstoupointOP("ipointstoupoint",
    ipointstoupointSpec.getStr(), ipointstoupointVM,
    Operator::SimpleSelect, ipointstoupointTM);

class ContinuousUpdateAlgebra: public Algebra {
public:
    ContinuousUpdateAlgebra() :
        Algebra() {
        AddOperator(&sendmessagesOP);
        sendmessagesOP.SetUsesMemory();
        sendmessagesOP.SetUsesArgsInTypeMapping();
        AddOperator(&providemessagesOP);
        providemessagesOP.SetUsesMemory();
        providemessagesOP.SetUsesArgsInTypeMapping();
        AddOperator(&receivenlstreamOP);
        receivenlstreamOP.SetUsesArgsInTypeMapping();
        receivenlstreamOP.SetUsesMemory();
        AddOperator(&owntransactioninsertOP);
        owntransactioninsertOP.SetUsesArgsInTypeMapping();
        owntransactioninsertOP.SetUsesMemory();
        AddOperator(&ipointstoupointOP);
        ipointstoupointOP.SetUsesArgsInTypeMapping();
        ipointstoupointOP.SetUsesMemory();

    }
    ~ContinuousUpdateAlgebra() {
    }
    ;
};

extern "C" Algebra*
InitializeContinuousUpdateAlgebra(NestedList* nlRef,
    QueryProcessor* qpRef) {
    nl = nlRef;
    qp = qpRef;
    return (new ContinuousUpdateAlgebra());
}
