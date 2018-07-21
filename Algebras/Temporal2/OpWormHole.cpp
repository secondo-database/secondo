/*
OpEnterWormHole.cpp
Created on: 08.04.2018
Author: simon

Limitations and ToDos:
- Handle failures for RemoteEnterWormHole in a sensible way
    (Default to NoneStremValve? "throw" secondo error?)
- Refactoring: Move TypeMapping Checks and ValueMapping to EnterWormHole
- Refactoring: Inroduce Factory for EnterWormHoles

*/

#include "OpWormHole.h"
#include <tr1/memory>

#include "Operator.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Algebras/Stream/Stream.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"

#include "TypeMapUtils.h"
#include "Symbols.h"
#include "ListUtils.h"

#include <sstream>

#include <boost/interprocess/ipc/message_queue.hpp>
#include "Types.h"

extern NestedList* nl;
extern QueryProcessor* qp;


using namespace std;
using std::tr1::shared_ptr;
namespace temporal2algebra{

struct EnterWormHoleInfo : OperatorInfo {
  EnterWormHoleInfo() : OperatorInfo() {
    name =      "enterwormhole";
    signature = "stream(alpha) x string -> int)";
    syntax =    "<incoming_stream> enterwormhole [<MyId>]";
    meaning =   "Redirects a stream to a different process"
            " used to workaround transaction limitations.\n"
            "Elements in the remote process can be retrieved by the"
            " leavewormhole operator.";
  }
};

ListExpr EnterWormHole_tm( ListExpr args ) {
    cout << "EnterWormHole_tm(" << nl->ToString(args) << ")\n";

    int numArgs = nl->ListLength(args);
    if (numArgs != 2 ) {
            stringstream s;
            s << "expected 2 arguments, but got " << numArgs;
            return listutils::typeError(s.str());
    }

    ListExpr streamType = nl->First(args);

    if (!listutils::isStream(streamType)) {
        return listutils::typeError(
                "expected stream as first argument (incoming stream), but got "
                + nl->ToString(streamType));
    }

    ListExpr wormHoleId = nl->Second(args);
    if (!FText::checkType(wormHoleId)) {
            return listutils::typeError("expected " + FText::BasicType()
            + " as second argument (WormHole Id), but got "
            + nl->ToString(wormHoleId));
        }

    return NList(CcInt::BasicType()).listExpr();

}


int EnterWormHole_sf( ListExpr args ) {
    cout << "EnterWormHole_sf(" << nl->ToString(args) << ")\n";
  return 0;
}

int EnterWormHole_vm( Word* args,
        Word& result,
        int message,
        Word& local,
        Supplier s ) {

    std::string wormHoleId = (static_cast<FText*>(args[1].addr))->GetValue();
    std::string memQueueName = "Temporal2_OpWormHole_Queue_" + wormHoleId;

    boost::interprocess::message_queue* memqueue = 0;
    if (wormHoleId != "") { // use empty id for testing...
        try {
            cout << "creating MemQueue: " << memQueueName.c_str() << "\n";
            memqueue = new boost::interprocess::message_queue(
                    boost::interprocess::create_only,
                    memQueueName.c_str(),
                    1000,
                    sizeof(QueueData2)
            );
        } catch (const boost::interprocess::interprocess_exception ex) {
            cout << "MemQueue not created\n";
            cout << ex.what();
            memqueue = 0;
        }
    }

    result = qp->ResultStorage(s);

    int count = 0;
    qp->Open(args[0].addr); // open the stream

    Word elem;
    qp->Request(args[0].addr, elem);
    while ( qp->Received(args[0].addr) ){
        ++count;
        if (memqueue) {
            int ipointPos=0;
            int tidPos = 1;

            Tuple* tuple = static_cast<Tuple*>(elem.addr);
            Intime* intime =
                    static_cast<Intime*>(tuple->GetAttribute(ipointPos));
            TupleIdentifier* tid =
                    static_cast<TupleIdentifier*>(tuple->GetAttribute(tidPos));

            QueueData2 qdata;
            qdata.intime = *intime;
            qdata.tid = *tid;
            qdata.lastElement = false;
            cout << qdata << endl;
            memqueue->send(&qdata, sizeof(QueueData2), 0);

            tuple->DeleteIfAllowed();
        }
       // elem->DeleteIfAllowed();
        qp->Request(args[0].addr, elem);
    }

    if (memqueue) {
        QueueData2 qdata;
        qdata.lastElement = true;
        memqueue->send(&qdata, sizeof(QueueData2), 0);

        delete memqueue; // the queue will be removed in the receiving proc
    }

    CcInt* res = static_cast<CcInt*>(result.addr) ;
    res->Set(true , count);
    qp->Close(args[0].addr);
    return 0;
}


ValueMapping EnterWormHole_vms[] =
{
  EnterWormHole_vm
};

Operator* getEnterWormHoleOpPtr() {
    Operator* op =  new Operator(
            EnterWormHoleInfo(),
            EnterWormHole_vms,
            EnterWormHole_sf,
            EnterWormHole_tm
           );
  //  op->SetUsesArgsInTypeMapping();
    return op;
}

struct LeaveWormHoleInfo : OperatorInfo {
  LeaveWormHoleInfo() : OperatorInfo() {
    name =      "leavewormhole";
    signature = "string -> stream(alpha)";
    syntax =    "leavewormhole [<MyValveId>]";
    meaning =   "provides a stream from another secondo"
            " process (via enterwormhole).";
  }
};

ListExpr LeaveWormHole_tm( ListExpr args ) {
    cout << "LeaveWormHole_tm(" << nl->ToString(args) << ")\n";

       int numArgs = nl->ListLength(args);
       if (numArgs != 1 ) {
               stringstream s;
               s << "expected 1 argument, but got " << numArgs;
               return listutils::typeError(s.str());
       }

       ListExpr wormHoleIdType = nl->First(nl->First(args));
       if (!FText::checkType(wormHoleIdType)) {
               return listutils::typeError("expected " + FText::BasicType()
               + " as first argument (WormHole Id), but got "
               + nl->ToString(wormHoleIdType));
           }

       ListExpr wormHoleIdName = nl->Second(nl->First(args));
           if (nl->AtomType(wormHoleIdName) != TextType) {
               return listutils::typeError(
                       "expected TextType contents in second Argument, but got "
                       + nl->ToString(wormHoleIdName));
           }

       std::string wormHoleId = nl->Text2String(wormHoleIdName);
       cout << wormHoleId;

       // connect to wormhole, read stream type and return that

       NList tid_attr("DestTid", TupleIdentifier::BasicType());
       NList intime_attr("IPos", Intime::BasicType());
       NList attrs(tid_attr, intime_attr);
       return NList().tupleStreamOf( attrs ).listExpr();

       //return NList(CcInt::BasicType()).listExpr();

   }


int LeaveWormHole_sf( ListExpr args ) {
    cout << "LeaveWormHole_sf(" << nl->ToString(args) << ")\n";
  return 0;
}

int LeaveWormHole_vm( Word* args,
        Word& result,
        int message,
        Word& local,
        Supplier s ) {

    struct LocalInfo {
        std::string qName;
        boost::interprocess::message_queue* queue;

        LocalInfo(std::string& memQueueName) :
            qName(memQueueName),
            queue(0) {
            cout << "opening MemQueue: "
                    << qName.c_str() << "\n";
            try {
                queue = new boost::interprocess::message_queue(
                        boost::interprocess::open_only,
                        qName.c_str()
                );
            } catch (const boost::interprocess::interprocess_exception ex) {
                cout << "couldn't open MemQueue:\n";
                cout << ex.what() << endl;
                queue = 0;
            }
        }

        ~LocalInfo() {
            if (queue) {
                delete queue;
                boost::interprocess::message_queue::remove(
                        qName.c_str());
            }
        }
    };

    LocalInfo* localInfo = static_cast<LocalInfo*>(local.addr);

    result = qp->ResultStorage(s);

    switch (message) {
    case OPEN:
    {
        if (localInfo) {
            delete localInfo;
        }

        std::string wormHoleId =
                (static_cast<FText*>(args[0].addr))->GetValue();
        std::string memQueueName = "Temporal2_OpWormHole_Queue_" + wormHoleId;

        localInfo = new LocalInfo(memQueueName);
        local.setAddr(localInfo);

        if (!localInfo->queue) {
            return CANCEL;
        }

        return 0;
        break;
    }
    case REQUEST:
    {
        if (!localInfo->queue) {
            cout << "no MemQueue with Data available\n";
            return CANCEL;
        }

        size_t dummy_rcvd_size;
        unsigned int dummy_prio;
        QueueData2 qdata;
        localInfo->queue->receive(
                &qdata,
                sizeof(QueueData2),
                dummy_rcvd_size,
                dummy_prio);

        if (!qdata.lastElement) {

            ListExpr tupleType = nl->Second(GetTupleResultType(s));
            Tuple* res_tuple = new Tuple(tupleType);

            res_tuple->PutAttribute(0, new TupleIdentifier(qdata.tid));
            res_tuple->PutAttribute(1, new Intime(qdata.intime));

            result.setAddr(res_tuple);

            return YIELD;
        }

        delete localInfo;
        localInfo = 0;
        local.setAddr(localInfo);

        return CANCEL;
        break;
    }
    case CLOSE:
        if (localInfo) {
            delete localInfo;
            localInfo = 0;
            local.setAddr(localInfo);
        }
        return 0;
        break;
    }

    assert(false); //shouldn't get here. Unhandled message type
    return -1;
}

//      struct LeaveWormHole_LocalInfo{
//        LeaveWormHole_LocalInfo(LeaveWormHolePtr valve): pValve(valve) {
//        };
//        LeaveWormHolePtr pValve;
//      };
//
//      Word elem;
//      LeaveWormHole_LocalInfo *localInfo =
//              static_cast<LeaveWormHole_LocalInfo*>(local.addr);
//
//      switch( message ){
//        case OPEN:{
//          FText* valveId = static_cast<FText*>(args[1].addr);
//          const std::string valve_id = valveId->GetValue();
//          LeaveWormHolePtr valvePtr;
//          if (valve_id == OpenValveId) {
//              valvePtr = LeaveWormHolePtr(new OpenLeaveWormHole() );
//          } else if (valve_id == RandomValveId) {
//              CcInt* min_wait = static_cast<CcInt*>(args[2].addr);
//              CcInt* max_wait = static_cast<CcInt*>(args[3].addr);
//              valvePtr = LeaveWormHolePtr (
//                 new RandomLeaveWormHole(min_wait->GetValue(),
//                              max_wait->GetValue()) );
//          } else if (valve_id == RemoteValveId) {
//              FText* remote_valve = static_cast<FText*>(args[2].addr);
//              valvePtr = LeaveWormHolePtr (
//                 RemoteLeaveWormHole::create(remote_valve->GetValue()) );
//          } else {
//              cout << "Unknown valveId: " << valve_id << endl;
//              assert(false);
//          }
//
//          localInfo = new LeaveWormHole_LocalInfo(valvePtr);
//
//          local.setAddr(localInfo);
//          qp->Open(args[0].addr);
//          return 0;
//        }
//        case REQUEST:{
//          if(!localInfo){ return CANCEL; }
//          localInfo->pValve->waitForSignal();
//          qp->Request(args[0].addr, elem);
//          while ( qp->Received(args[0].addr) ) {
//            result = elem;
//            localInfo->pValve->sendHasReadSucceeded(true);
//            return YIELD;
//          }
//          localInfo->pValve->sendHasReadSucceeded(false);
//          return CANCEL;
//        }
//        case CLOSE:{
//          qp->Close(args[0].addr);
//          if(localInfo){
//            delete localInfo;
//            local.setAddr(0);
//          }
//          return 0;
//        }
//      } // switch
//      assert (false); // should not happen: unhandled message type
// }


ValueMapping LeaveWormHole_vms[] =
{
  LeaveWormHole_vm
};

Operator* getLeaveWormHoleOpPtr() {
    Operator* op =  new Operator(
            LeaveWormHoleInfo(),
            LeaveWormHole_vms,
            LeaveWormHole_sf,
            LeaveWormHole_tm
           );
    op->SetUsesArgsInTypeMapping();
    return op;
}

} // end of namespace temporal2algebra


