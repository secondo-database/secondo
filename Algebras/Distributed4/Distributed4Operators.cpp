/*
----
This file is part of SECONDO.

Copyright (C) 2017, Faculty of Mathematics and Computer Science, Database
Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

SECONDO is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
SECONDO; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [10] title: [{\Large \bf] [}]
//characters [1] tt: [\texttt{] [}]
//[secondo] [{\sc Secondo}]

[10] Operator Implementation of Algebra Distributed4

2017-08-14: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 Preliminary Setup

*/
#include "DPartition.h"
#include "ManagedMutex.h"
#include "../Distributed2/DArray.h"
#include "../Distributed2/ConnectionInfo.h"
#include "../Relation-C++/RelationAlgebra.h"
#include <vector>

namespace distributed4 {
  using distributed2::CommandLog;
  using distributed2::ConnectionInfo;
  using distributed2::DArray;
  using distributed2::DArrayBase;
  using distributed2::DFArray;
  using std::cerr;  // debug
  using std::string;
  using std::vector;
  using std::runtime_error;

  ListExpr peerCommand(ConnectionInfo* ci, const string& cmd) {
    int err;
    string msg;
    ListExpr res;
    double rt;
    CommandLog log;
    ci->simpleCommand(cmd, err, msg, res, false, rt, true, false, log);
    if(err)
      throw runtime_error(msg);
    return res;
  }
/*
2 Type Mapping Functions

These functions evaluate a "ListExpr"[1] to see if it matches the expected
input type of the corresponding operator. They return a "ListExpr"[1] of the
operator's result data type.

Each Type Mapping Function has the same basic two-step structure: (1) Verify
the number of arguments. (2) Verify the type of arguments.

2.1 ~lock~ Type Mapping

*/
  ListExpr lockTM(ListExpr args) {
    NList l{args};
    cerr << "debug(lockTM): " << l << endl;
    string err = "string x bool expected";
    if(!l.checkLength(2, err) || !CcString::checkType(l.first().listExpr()) ||
        !CcBool::checkType(l.second().listExpr()))
      return NList::typeError(err);
    return NList{CcBool::BasicType()}.listExpr();
  }
/*
2.2 ~unlock~ Type Mapping

*/
  ListExpr unlockTM(ListExpr args) {
    NList l{args};
    cerr << "debug(unlockTM): " << l << endl;
    string err = "string expected";
    if(!l.checkLength(1, err) || !CcString::checkType(l.first().listExpr()))
      return NList::typeError(err);
    return NList{CcBool::BasicType()}.listExpr();
  }
/*
2.3 ~addworker~ Type Mapping

*/
  ListExpr addworkerTM(ListExpr args) {
    NList l{args};
    cerr << "debug(addworkerTM): " << l << endl;
    string err = "d[f]array x string x int x string x rel(tuple([Host: "
      "string, Port: int, Config: string])) expected";
    if(!l.checkLength(5, err) ||
        (!DArray::checkType(l.first().first().listExpr()) &&
         !DFArray::checkType(l.first().first().listExpr())) ||
        !CcString::checkType(l.second().first().listExpr()) ||
        !CcInt::checkType(l.third().first().listExpr()) ||
        !CcString::checkType(l.fourth().first().listExpr()) ||
        !Relation::checkType(l.fifth().first().listExpr()))
      return NList::typeError(err);
    NList attrs{l.fifth().first().second().second()};
    if(attrs.length() != 3 || attrs.first().first().str() != "Host" ||
        !CcString::checkType(attrs.first().second().listExpr()) ||
        attrs.second().first().str() != "Port" ||
        !CcInt::checkType(attrs.second().second().listExpr()) ||
        attrs.third().first().str() != "Config" ||
        !CcString::checkType(attrs.third().second().listExpr()))
      return NList::typeError(err);
    return NList{NList{Symbol::APPEND()},
      NList{l.first().second().toStringAtom(),
        NList{l.first().first().convertToString(), true, true}},
      NList{CcBool::BasicType()}}.listExpr();
  }
/*
3 Value Mapping Functions

These functions do the productive work of the corresponding operator. They
process input data ("args"[1]) and post the result ("result"[1]).

3.1 ~lock~ Value Mapping

*/
  int lockVM(Word* args, Word& result, int, Word&, Supplier s) {
    // Convert passed values.
    string name{static_cast<CcString*>(args[0].addr)->GetValue()};
    bool exclusive{static_cast<CcBool*>(args[1].addr)->GetValue()};
    cerr << "debug(lockVM): name = " << name << ", " << "exclusive = " <<
      exclusive << endl;
/*
For the case that this function ends in failure, "result"[1] is set to
"false"[1].

*/
    result = qp->ResultStorage(s);
    CcBool* res{static_cast<CcBool*>(result.addr)};
    res->Set(true, false);
/*
Check to make sure the passed "name"[1] refers to a real database object.

*/
    if(!SecondoSystem::GetCatalog()->IsObjectName(name)) {
      cmsg.otherError("No object exists by that name.");
      return 0;
    }
/*
Acquire the lock. Here the "exclusive"[1] argument indicates whether the lock
is to be exclusive or not (in that case it is sharable).

*/
    ManagedMutex mutex{name, exclusive};
/*
The lock has been sucessfully acquired. Indicate success.

*/
    res->Set(true, true);
    return 0;
  }
/*
3.2 ~unlock~ Value Mapping

*/
  int unlockVM(Word* args, Word& result, int, Word&, Supplier s) {
    // Convert passed values.
    string name{static_cast<CcString*>(args[0].addr)->GetValue()};
    cerr << "debug(unlockVM): name = " << name << endl;
/*
For the case that this function ends in failure, "result"[1] is set to
"false"[1].

*/
    result = qp->ResultStorage(s);
    CcBool* res{static_cast<CcBool*>(result.addr)};
    res->Set(true, false);
/*
Check to make sure the passed "name"[1] refers to a real database object.

*/
    if(!SecondoSystem::GetCatalog()->IsObjectName(name)) {
      cmsg.otherError("No object exists by that name.");
      return 0;
    }
/*
Release the lock.

*/
    ManagedMutex::unlock(name);
/*
The lock has been sucessfully released. Indicate success.

*/
    res->Set(true, true);
    return 0;
  }
/*
3.3 ~addworker~ Value Mapping

*/
  int addworkerVM(Word* args, Word& result, int, Word&, Supplier s) {
    // Convert passed values.
    DArrayBase* da{static_cast<DArrayBase*>(args[0].addr)};
    string host{static_cast<CcString*>(args[1].addr)->GetValue()};
    int port{static_cast<CcInt*>(args[2].addr)->GetValue()};
    string conf{static_cast<CcString*>(args[3].addr)->GetValue()};
    Relation* peers{static_cast<Relation*>(args[4].addr)};
    string daname{static_cast<CcString*>(args[5].addr)->GetValue()};
    string datype{static_cast<FText*>(args[6].addr)->GetValue()};
    cerr << "debug(addworkerVM): da = " << *da << endl;
    cerr << "debug(addworkerVM): host = " << host << ", port = " << port <<
      ", conf = " << conf << endl;
    cerr << "debug(addworkerVM): peers = "; peers->Print(cerr);
    cerr << "debug(addworkerVM): daname = " << daname << endl;
    cerr << "debug(addworkerVM): datype = " << datype << endl;
/*
For the case that this function ends in failure, "result"[1] is set to
"false"[1].

*/
    result = qp->ResultStorage(s);
    CcBool* res{static_cast<CcBool*>(result.addr)};
    res->Set(true, false);
/*
Before attempting to add the specified worker to "da"[1], a few checks are
performed. The list of workers in "da"[1] is checked to make sure the specified
worker isn't already there.

*/
    auto workers{da->getWorkers()};
    for(auto w: workers)
      if(w.getHost() == host && w.getPort() == port) {
        cmsg.otherError("The specified worker already exists.");
        return 0;
      }
/*
A connection attempt is made to the new worker to make sure communication is
possible.

*/
    ConnectionInfo* conn{ConnectionInfo::createConnection(host, port, conf)};
    if(conn == 0) {
      cmsg.otherError("The specified worker could not be contacted.");
      return 0;
    }
    delete conn;
/*
The following steps cause a synchronized update of "da"[1] locally and on all
"peers"[1]:

  * lock "da"[1] for exclusive access (implicitly ensuring "da"[1] exists)

  * begin a transaction and update "da"[1]

  * commit the transaction

Lock "da"[1] locally and then connect to "peers"[1] one at a time and lock
"da"[1]. In case of error, an attempt is made to unlock "da"[1].

*/
    ManagedMutex mutex{daname, true};
    string dbname{SecondoSystem::GetInstance()->GetDatabaseName()};
    vector<ConnectionInfo*> peer_conns;
    auto rit{peers->MakeScan()};
    for(Tuple* t{rit->GetNextTuple()}; !rit->EndOfScan(); t =
        rit->GetNextTuple())
    {
      string peer_host{static_cast<CcString*>(t->GetAttribute(0))->GetValue()};
      int peer_port{static_cast<CcInt*>(t->GetAttribute(1))->GetValue()};
      string peer_conf{static_cast<CcString*>(t->GetAttribute(2))->GetValue()};
      t->DeleteIfAllowed();
      ConnectionInfo* ci{ConnectionInfo::createConnection(peer_host, peer_port,
          peer_conf)};
      try {
        peerCommand(ci, "open database " + dbname);
        peerCommand(ci, "query lock(" + daname + ")");
      } catch(const runtime_error& e) {
        cmsg.otherError(e.what());
        for(auto ci: peer_conns) {
          try {
            peerCommand(ci, "query unlock(" + daname + ")");
          } catch(const runtime_error&) {}
          delete ci;
        }
        mutex.unlock();
        return 0;
      }
      peer_conns.push_back(ci);
    }
/*
Now that all locks are acquired for "da"[1], a temporary "DArray"[1] is created
with the updated information.

*/
    auto num{workers.back().getNum() + 1};
    workers.emplace_back(host, port, num, conf);
    DArray da_new{0};
    da_new.set(da->getMap(), da->getName(), workers);
/*
On all "peers"[1], a transaction is started and "da"[1] updated. In case of
error, an attempt is made to abort the transaction and unlock "da"[1].

*/
    for(auto ci: peer_conns) {
      try {
        peerCommand(ci, "begin transaction");
        peerCommand(ci, "update " + daname + ":= [const " + datype + " value "
            + nl->ToString(da_new.toListExpr()) + "]");
      } catch(const runtime_error& e) {
        cmsg.otherError(e.what());
        for(auto ci: peer_conns) {
          try {
            peerCommand(ci, "abort transaction");
            peerCommand(ci, "query unlock(" + daname + ")");
          } catch(const runtime_error&) {}
          delete ci;
        }
        return 0;
      }
    }
/*
Now, "da"[1] is updated locally as well.

*/
    da->set(da->getMap(), da->getName(), workers);
    qp->SetModified(qp->GetSon(s, 0));
/*
At this point, "da"[1] has been updated successfully everywhere. The
transaction is commited. In case of error, a warning is printed, but execution
continues, as a rollback is no longer safely possible.

*/
    for(auto ci: peer_conns) {
      try {
        peerCommand(ci, "commit transaction");
      } catch(const runtime_error& e) {
        cmsg.warning() << "There was a problem during transaction commit on "
          "peer " << ci->getHost() << ":" << ci->getPort() << ". The value of "
          << daname << " might not be updated!" << endl;
        cmsg.warning() << e.what() << endl;
        cmsg.send();
      }
    }
/*
Finally, the locks are released on all "peers"[1] and then locally.

*/
    for(auto ci: peer_conns) {
      try {
        peerCommand(ci, "query unlock(" + daname + ")");
      } catch(const runtime_error& e) {
        cmsg.warning() << "There was a problem unlocking " << daname << " on "
          "peer " << ci->getHost() << ":" << ci->getPort() << "." << endl;
        cmsg.warning() << e.what() << endl;
        cmsg.send();
      }
    }
    mutex.unlock();
/*
The new worker has been sucessfully added. Indicate success.

*/
    res->Set(true, true);
    return 0;
  }
/*
4 Selection Functions

For overloaded operators, it is necessary to choose one out of an array of
possible value mapping functions. This is accomplished by listing all possible
value mapping functions in a value mapping array and providing a selection
function that returns an index into that array, thus selecting the appropriate
value mapping function to be used. I chose to use a vector instead of an array
and pass the underlying array to the operator instance below. This way, if the
value mapping functions change, I don't have to worry about updating how many
entries are in the array in all the right places.

4.1 ~addworker~ Selection

*/
  //std::vector<ValueMapping> addworkerArray{
  //  addworkerVM<CcString,CcString>,
  //  addworkerVM<CcString,FText>,
  //  addworkerVM<FText,CcString>,
  //  addworkerVM<FText,FText>,
  //};

  //int addworkerSelect(ListExpr args) {
  //  NList l{args};
  //  cerr << "debug(addworkerSelect): " << l << endl;
  //  int i{0};
  //  if(FText::checkType(l.second().listExpr())) i += 2;
  //  if(FText::checkType(l.fourth().listExpr())) i += 1;
  //  return i;
  //}
/*
5 Operator Specifications

The "OperatorSpec"[1] class formats human-readable specification for the
operators in a consistent way. The constructor expects four or five strings in
the following order:

  * signature

  * syntax

  * meaning

  * example

  * remark (optional)

5.1 ~lock~ Specification

*/
  OperatorSpec lockSpec{
    CcString::BasicType() + " x " + CcBool::BasicType() + " -> " +
      CcBool::BasicType(),
    "lock(objname, exclusive)",
    "Acquires a lock for objname. The ownership of the lock is exclusive or "
      "sharable depending on whether exclusive is given as TRUE or FALSE.",
    "query lock(\"Da\", TRUE)",
  };
/*
5.2 ~unlock~ Specification

*/
  OperatorSpec unlockSpec{
    CcString::BasicType() + " -> " + CcBool::BasicType(),
    "unlock(objname)",
    "Releases a lock for objname. Whether the held lock is exclusive or "
      "sharable is handled internally.",
    "query unlock(\"Da\")",
  };
/*
5.3 ~addworker~ Specification

*/
  OperatorSpec addworkerSpec{
    "d[f]array x " + CcString::BasicType() + " x " + CcInt::BasicType() + " x "
      + CcString::BasicType() + " -> " + CcBool::BasicType(),
    "addworker(da, host, port, conffile, peers)",
    "Adds a worker given by host, port, and conffile to da. No slots are "
      "mapped to the new worker at this point.",
    "query addworker(Da, \"snew\", 1234, \"cfg.ini\", Peers)",
  };
/*
6 Operator Instances

The "Operator"[1] class takes care of putting together operator instances given
data for a simple or an overloaded operator. The constructor expects the
following elements:

  * name of operator ("string"[1])

  * specification of operator ("string"[1], returned by the "getStr"[1] method
    of "OperatorSpec"[1])

  * value mapping function ("ValueMapping"[1], simple operators only)

  * number of value mapping functions ("int"[1], overloaded operators only)

  * array of value mapping functions ("ValueMapping[]"[1], overloaded operators
    only)

  * selection function ("SelectFunction"[1], simple operators use
    "Operator::SimpleSelect"[1])

  * type mapping function ("TypeMapping"[1])

6.1 ~lock~ Instance

*/
  Operator lockOp{
    "lock",
    lockSpec.getStr(),
    lockVM,
    Operator::SimpleSelect,
    lockTM,
  };
/*
6.2 ~unlock~ Instance

*/
  Operator unlockOp{
    "unlock",
    unlockSpec.getStr(),
    unlockVM,
    Operator::SimpleSelect,
    unlockTM,
  };
/*
6.3 ~addworker~ Instance

*/
  //Operator addworkerOp{
  //  "addworker",
  //  addworkerSpec.getStr(),
  //  static_cast<int>(addworkerArray.size()),
  //  addworkerArray.data(),
  //  addworkerSelect,
  //  addworkerTM,
  //};
  Operator addworkerOp{
    "addworker",
    addworkerSpec.getStr(),
    addworkerVM,
    Operator::SimpleSelect,
    addworkerTM,
  };
}

//TODO: remove commented remains of addWorker.
