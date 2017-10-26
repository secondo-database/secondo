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
#include "Operators.h"
#include "Algebra.h"
#include "ManagedMutex.h"
#include "DPartition.h"
#include "Peers.h"
#include "../Distributed2/Distributed2Algebra.h"

namespace distributed4 {
  using distributed2::ConnectionInfo;
  using distributed2::DArray;
  using distributed2::DArrayBase;
  using distributed2::DFArray;
  using distributed2::Distributed2Algebra;
  using std::runtime_error;
  using std::string;
  using std::to_string;
  using std::unique_ptr;
/*
2 Helper Functions

2.1 "is\_common\_type"[1]

Check the type in "l"[1] for patterns commonly needed by operators of the
Distributed4 Algebra.

*/
  bool is_common_type(NList& l, int len, string& err) {
    if(!(l.checkLength(len, err) &&
          Relation::checkType(l.elem(len).first().listExpr())))
      return false;
    NList attrs{l.elem(len).first().second().second()};
    if(!(attrs.length() == 3 && attrs.first().first().str() == "Host" &&
          CcString::checkType(attrs.first().second().listExpr()) &&
          attrs.second().first().str() == "Port" &&
          CcInt::checkType(attrs.second().second().listExpr()) &&
          attrs.third().first().str() == "Config" &&
          CcString::checkType(attrs.third().second().listExpr())))
      return false;
    return true;
  }
/*
2.2 "extended\_return\_type"[1]

Create an extended return type from "l"[1] that includes the name and the type
of the first argument.

*/
  ListExpr extended_return_type(const string& type, const NList& l, const
      NList& more = NList{}) {
    NList append{l.first().second().toStringAtom().enclose()};
    append.concat(more);
    return NList{NList{Symbol::APPEND()}, append, NList{type}}.listExpr();
  }
/*
3 Type Mapping Functions

These functions evaluate a "ListExpr"[1] to see if it matches the expected
input type of the corresponding operator. They return a "ListExpr"[1] of the
operator's result data type.

Each Type Mapping Function has the same basic two-step structure: (1) Verify
the number of arguments. (2) Verify the type of arguments.

3.1 ~lock~ Type Mapping (also used by ~trylock~)

*/
  ListExpr lockTM(ListExpr args) {
    NList l{args};
    string err = "string x bool expected";
    if(!l.checkLength(2, err) || !CcString::checkType(l.first().listExpr()) ||
        !CcBool::checkType(l.second().listExpr()))
      return NList::typeError(err);
    return NList{CcBool::BasicType()}.listExpr();
  }
/*
3.2 ~unlock~ Type Mapping

*/
  ListExpr unlockTM(ListExpr args) {
    NList l{args};
    string err = "string expected";
    if(!l.checkLength(1, err) || !CcString::checkType(l.first().listExpr()))
      return NList::typeError(err);
    return NList{CcBool::BasicType()}.listExpr();
  }
/*
3.3 ~addworker~ Type Mapping

*/
  ListExpr addworkerTM(ListExpr args) {
    NList l{args};
    string err = "d[f]array x string x int x string x rel(tuple([Host: "
      "string, Port: int, Config: string])) expected";
    if(!(is_common_type(l, 5, err) &&
          (DArray::checkType(l.first().first().listExpr()) ||
           DFArray::checkType(l.first().first().listExpr())) &&
          CcString::checkType(l.second().first().listExpr()) &&
          CcInt::checkType(l.third().first().listExpr()) &&
          CcString::checkType(l.fourth().first().listExpr())))
      return NList::typeError(err);
    return extended_return_type(CcBool::BasicType(), l);
  }
/*
3.4 ~removeworker~ Type Mapping

*/
  ListExpr removeworkerTM(ListExpr args) {
    NList l{args};
    string err = "d[f]array x int x rel(tuple([Host: string, Port: int, "
      "Config: string])) expected";
    if(!(is_common_type(l, 3, err) &&
          (DArray::checkType(l.first().first().listExpr()) ||
           DFArray::checkType(l.first().first().listExpr())) &&
          CcInt::checkType(l.second().first().listExpr())))
      return NList::typeError(err);
    return extended_return_type(CcBool::BasicType(), l);
  }
/*
3.5 ~moveslot~ Type Mapping

*/
  ListExpr moveslotTM(ListExpr args) {
    NList l{args};
    string err = "dpartition x int x int x rel(tuple([Host: string, Port: "
      "int, Config: string])) expected";
    if(!(is_common_type(l, 4, err) &&
          DPartition::checkType(l.first().first().listExpr()) &&
          CcInt::checkType(l.second().first().listExpr()) &&
          CcInt::checkType(l.third().first().listExpr())))
      return NList::typeError(err);
    return extended_return_type(CcInt::BasicType(), l);
  }
/*
3.6 ~splitslot~ Type Mapping

*/
  ListExpr splitslotTM(ListExpr args) {
    NList l{args};
    string err = "dpartition x int x rel(tuple([Host: string, Port: int, "
      "Config: string])) expected";
    if(!(is_common_type(l, 3, err) &&
          DPartition::checkType(l.first().first().listExpr()) &&
          CcInt::checkType(l.second().first().listExpr())))
      return NList::typeError(err);
    return extended_return_type(CcInt::BasicType(), l,
        l.first().first().second().first().toStringAtom());
  }
/*
3.7 ~getworkerindex~ Type Mapping

*/
  ListExpr getworkerindexTM(ListExpr args) {
    NList l{args};
    string err = "d[f]array x string x int expected";
    if(!(l.checkLength(3, err) && (DArray::checkType(l.first().listExpr()) ||
            DFArray::checkType(l.first().listExpr())) &&
          CcString::checkType(l.second().listExpr()) &&
          CcInt::checkType(l.third().listExpr())))
      return NList::typeError(err);
    return NList{CcInt::BasicType()}.listExpr();
  }
/*
4 Value Mapping Functions

These functions do the productive work of the corresponding operator. They
process input data ("args"[1]) and post the result ("result"[1]).

4.1 ~lock~ Value Mapping

*/
  int lockVM(Word* args, Word& result, int, Word&, Supplier s) {
    CcBool* res{&qp->template ResultStorage<CcBool>(result, s)};
    string name{static_cast<CcString*>(args[0].addr)->GetValue()};
    bool exclusive{static_cast<CcBool*>(args[1].addr)->GetValue()};
    ManagedMutex mutex{name, exclusive};
    mutex.noautounlock();
    res->Set(true, true);
    return 0;
  }
/*
4.2 ~trylock~ Value Mapping

*/
  int trylockVM(Word* args, Word& result, int, Word&, Supplier s) {
    CcBool* res{&qp->template ResultStorage<CcBool>(result, s)};
    string name{static_cast<CcString*>(args[0].addr)->GetValue()};
    bool exclusive{static_cast<CcBool*>(args[1].addr)->GetValue()};
    ManagedMutex mutex{name, exclusive, false};
    mutex.noautounlock();
    res->Set(true, true);
    return 0;
  }
/*
4.3 ~unlock~ Value Mapping

*/
  int unlockVM(Word* args, Word& result, int, Word&, Supplier s) {
    CcBool* res{&qp->template ResultStorage<CcBool>(result, s)};
    string name{static_cast<CcString*>(args[0].addr)->GetValue()};
    ManagedMutex mutex{name};
    mutex.unlock();
    res->Set(true, true);
    return 0;
  }
/*
4.4 ~addworker~ Value Mapping

*/
  int addworkerVM(Word* args, Word& result, int, Word&, Supplier s) {
    CcBool* res{&qp->template ResultStorage<CcBool>(result, s)};
/*
Lock "daname"[1]. This must be done as soon as possible to avoid race
conditions between the checks and the update.

*/
    string daname{static_cast<CcString*>(args[5].addr)->GetValue()};
    ManagedMutex mutex{daname, true};
/*
Before proceeding, the following checks are performed on the input values and
database state:

  * The list of workers in "da"[1] is checked to make sure the specified
    worker isn't already there.

*/
    DArrayBase* da{static_cast<DArrayBase*>(args[0].addr)};
    string host{static_cast<CcString*>(args[1].addr)->GetValue()};
    int port{static_cast<CcInt*>(args[2].addr)->GetValue()};
    auto workers{da->getWorkers()};
    for(auto w: workers)
      if(w.getHost() == host && w.getPort() == port)
        throw runtime_error{"The specified worker already exists."};
/*
  * A connection attempt is made to the new worker to make sure communication
    is possible.

*/
    string conf{static_cast<CcString*>(args[3].addr)->GetValue()};
    ConnectionInfo* conn{ConnectionInfo::createConnection(host, port, conf)};
    if(!conn)
      throw runtime_error{"The specified worker could not be contacted."};
    delete conn;
/*
The following steps cause a synchronized update of "da"[1] locally and on all
"peers"[1]:

  * Lock "da"[1] for exclusive access (implicitly ensuring "da"[1] exists).

  * Begin a transaction and update "da"[1].

  * Commit the transaction.

Connect to "peers"[1] one at a time and lock "da"[1].

*/
    Peers peers{static_cast<Relation*>(args[4].addr)};
    peers.exec([&](auto& p){ p->lockObject(daname, true); });
/*
Now that all locks are acquired for "da"[1], the in-memory-copy is updated.
This is then used to update "da"[1] on all "peers"[1] within a not-yet-commited
transaction.

*/
    auto num{workers.back().getNum() + 1};
    workers.emplace_back(host, port, num, conf);
    da->set(da->getMap(), da->getName(), workers);
    peers.exec([&](auto& p){
        p->beginTransaction();
        p->updateObject(daname, da);
        });
/*
Next, the changes to "da"[1] are committed first locally then on the
"peers"[1].

*/
    qp->SetModified(qp->GetSon(s, 0));
    peers.exec([](auto& p){
        p->clearRollback();
        p->commitTransaction();
        });
/*
Finally, the locks are released on all "peers"[1]. The local lock will be
released automatically when the function returns.

*/
    peers.exec([&](auto& p){ p->unlockObject(daname); });
/*
The new worker has been sucessfully added. Indicate success.

*/
    res->Set(true, true);
    return 0;
  }
/*
4.5 ~removeworker~ Value Mapping

*/
  int removeworkerVM(Word* args, Word& result, int, Word&, Supplier s) {
    CcBool* res{&qp->template ResultStorage<CcBool>(result, s)};
/*
Lock "daname"[1]. This must be done as soon as possible to avoid race
conditions between the checks and the update.

*/
    string daname{static_cast<CcString*>(args[3].addr)->GetValue()};
    ManagedMutex mutex{daname, true};
/*
Before proceeding, the following checks are performed on the input values and
database state:

  * Make sure the worker number was passed as a positive value and put it in
    "worker"[1].

*/
    int arg{static_cast<CcInt*>(args[1].addr)->GetValue()};
    if(arg < 0)
      throw runtime_error{"A negative number was given for the worker."};
    uint32_t worker{static_cast<uint32_t>(arg)};
/*
  * Make sure "worker"[1] exists in "da"[1].

*/
    DArrayBase* da{static_cast<DArrayBase*>(args[0].addr)};
    auto workers{da->getWorkers()};
    if(worker >= workers.size())
      throw runtime_error{"The specified worker is out of range."};
/*
  * Make sure "worker"[1] is not responsible for any slots.

*/
    auto map{da->getMap()};
    for(auto m: map)
      if(m == worker)
        throw runtime_error{"The specified worker still has slots mapped to "
          "it."};
/*
Next, any existing connections for that worker are closed.

*/
    static_cast<Distributed2Algebra*>(SecondoSystem::GetAlgebraManager()->
        getAlgebra("Distributed2Algebra"))->closeWorker(workers[worker]);
/*
The following steps cause a synchronized update of "da"[1] locally and on all
"peers"[1]:

  * Lock "da"[1] for exclusive access (implicitly ensuring "da"[1] exists).

  * Begin a transaction and update "da"[1].

  * Commit the transaction.

Connect to "peers"[1] one at a time and lock "da"[1].

*/
    Peers peers{static_cast<Relation*>(args[2].addr)};
    peers.exec([&](auto& p){ p->lockObject(daname, true); });
/*
Now that all locks are acquired for "da"[1], the specified worker is removed
and the slot-to-worker mapping for subsequent workers decremented in the
in-memory copy. This is then used to update "da"[1] on all "peers"[1] within a
not-yet-commited transaction.

*/
    for(auto it{map.begin()}; it != map.end(); ++it)
      if(*it > worker)
        --(*it);
    workers.erase(workers.begin() + worker);
    da->set(map, da->getName(), workers);
    peers.exec([&](auto& p){
        p->beginTransaction();
        p->updateObject(daname, da);
        });
/*
Next, the changes to "da"[1] are committed first locally then on the
"peers"[1].

*/
    qp->SetModified(qp->GetSon(s, 0));
    peers.exec([](auto& p){
        p->clearRollback();
        p->commitTransaction();
        });
/*
Finally, the locks are released on all "peers"[1]. The local lock will be
released automatically when the function returns.

*/
    peers.exec([&](auto& p){ p->unlockObject(daname); });
/*
The new worker has been sucessfully removed. Indicate success.

*/
    res->Set(true, true);
    return 0;
  }
/*
4.6 ~moveslot~ Value Mapping

*/
  int moveslotVM(Word* args, Word& result, int, Word&, Supplier s) {
    CcInt* res{&qp->template ResultStorage<CcInt>(result, s)};
/*
Lock "dpname"[1] and the referenced ~d[f]array~. This must be done as soon as
possible to avoid race conditions between the checks and updates.

*/
    string dpname{static_cast<CcString*>(args[4].addr)->GetValue()};
    ManagedMutex mutex_dp{dpname, true};
    DPartition* dp{static_cast<DPartition*>(args[0].addr)};
    ManagedMutex mutex_da{dp->getDArrayName(), true};
/*
Before proceeding, the following checks are performed on the input values and
database state:

  * Make sure the slot and worker numbers were passed as positive values and
    put them in "src"[1] and "worker"[1], respectively.

*/
    int arg{static_cast<CcInt*>(args[1].addr)->GetValue()};
    if(arg < 0)
      throw runtime_error{"A negative number was given for the slot to be "
        "moved."};
    uint32_t src{static_cast<uint32_t>(arg)};
    arg = static_cast<CcInt*>(args[2].addr)->GetValue();
    if(arg < 0)
      throw runtime_error{"A negative number was given for the worker."};
    uint32_t worker{static_cast<uint32_t>(arg)};
/*
  * Make sure the ~d[f]array~ referenced in "dp"[1] can be read and put it in
    "da"[1].

*/
    unique_ptr<DArrayBase> da{dp->getDArray()};
/*
  * Make sure "src"[1] and "worker"[1] exist in "da"[1].

*/
    if(src >= da->getSize())
      throw runtime_error{"The specified slot is out of range in \"" +
        dp->getDArrayName() + "\"."};
    auto workers{da->getWorkers()};
    if(worker >= workers.size())
      throw runtime_error{"The specified worker is out of range in \"" +
        dp->getDArrayName() + "\"."};
/*
  * Make sure "worker"[1] is not the worker "src"[1] is already on.

*/
    if(worker == da->getWorkerIndexForSlot(src))
      throw runtime_error{"The specified slot is to be moved to the worker it "
        "is already on. Aborting."};
/*
  * Make sure "src"[1] is in use by "dp"[1]. This is done by retrieving its
    value-to-slot mapping (throws if no mapping to "src"[1] exists).

*/
    double partition_start{dp->getPartition(src)};
/*
Moving "src"[1] will proceed using the following steps. Any intricacies
and error conditions will be documented with the implementation below:

  1 Allocate a (new or recycled) slot as "dst"[1] and update "da"[1] on all
    peers.

  2 Replace "src"[1] with "dst"[1] as the value mapping target and mark
    "src"[1] as an additional temporary target to be used when *reading*
    values. Update "dp"[1] on all peers.

  3 Copy the contents of "src"[1] to "dst"[1].

  4 Remove the additional temporary value mapping target status for "src"[1]
    and update "dp"[1] on all peers.

Step 1: Allocate a (new or recycled) slot as "dst"[1] and update "da"[1] on all
peers.

This requires an exclusive global lock on "da"[1]. An existing slot may be
recycled, or a new slot created. In the case of a recycled slot, it will be
placed on and assigned to "worker"[1] and removed from wherever it was before.

*/
    Peers peers{static_cast<Relation*>(args[3].addr)};
    peers.exec([&](auto& p){ p->lockObject(dp->getDArrayName(), true); });
    uint32_t dst{dp->allocateSlot(worker, da.get())};
    peers.exec([&](auto& p){
        p->updateObject(dp->getDArrayName(), da.get());
        });
    peers.exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(dp->getDArrayName());
        });
/*
Step 2: Replace "src"[1] with "dst"[1] as the value mapping target and mark
"src"[1] as an additional temporary target to be used when *reading* values.
Update "dp"[1] on all peers.

This requires an exclusive global lock on "dp"[1]. Since "dst"[1] replaces
"src"[1] at this point (rather than in step 4), any new data will be inserted
in "dst"[1] instead of "src"[1]. If that weren't the case, step 3 would be
copying data out of a slot that is still receiving data. Having "src"[1] marked
as an additional temporary target for its previous value mapping allows normal
access to the data it holds.

*/
    peers.exec([&](auto& p){ p->lockObject(dpname, true); });
    dp->setBufferPartition(partition_start, src);
    dp->resetPartition(partition_start, dst);
    qp->SetModified(qp->GetSon(s, 0));
    peers.exec([&](auto& p){ p->updateObject(dpname, dp); });
    peers.exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(dpname);
        });
/*
Step 3: Copy the contents of "src"[1] to "dst"[1].

This requires a shared global lock on "da"[1]. Both slots "src"[1] and "dst"[1]
may be accessed for read access by other processes during this operation. As
write access requires an exclusive lock (except pure inserts, which require no
lock), that is blocked for the duration of the copy procedure. The copied
tuples exist in both "src"[1] and "dst"[1] until the procedure is complete. It
is absolutely vital that "src"[1] be empty at the end of this operation.
Otherwise future attempts to reuse it will fail.

  * Lock "da"[1] on all "peers"[1], determine values for "src\_name"[1],
    "dst\_name"[1], and "filename"[1], and establish sessions "src\_s"[1] and
    "dst\_s"[1].

*/
    Distributed2Algebra* d2{static_cast<Distributed2Algebra*>(SecondoSystem::
        GetAlgebraManager()->getAlgebra("Distributed2Algebra"))};
    const string dbname{SecondoSystem::GetInstance()->GetDatabaseName()};
    peers.exec([&](auto& p){ p->lockObject(dp->getDArrayName(), false); });
    string src_name{da->getObjectNameForSlot(src)};
    string dst_name{da->getObjectNameForSlot(dst)};
    ConnectionSession src_s{d2->getWorkerConnection(da->getWorkerForSlot(src),
        dbname)};
    ConnectionSession dst_s{d2->getWorkerConnection(da->getWorkerForSlot(dst),
        dbname)};
    string filename{src_name + ".bin"};
/*
  * Have "src\_s"[1] store the relation in a file and send it to "dst\_s"[1].

*/
    src_s.run("query " + src_name + " writeRel['" + filename + "']",
        "query removeFile('" + filename + "')");
    src_s.run("query connect('" + dst_s.getHost() + "', " +
        to_string(dst_s.getPort()) + ", '" + dst_s.getConfig() + "')");
    string dstsendfolder{src_s.queryString("getSendFolder(checkConnections() "
        "count - 1)")};
    if(dstsendfolder.empty())
      throw runtime_error{"The source instance, " + src_s.getHost() + ":" +
        to_string(src_s.getPort()) + ", did not receive the path of the "
          "SendFolder from the destination instance, " + dst_s.getHost() + ":"
          + to_string(dst_s.getPort()) + "."};
    string dsthome{dst_s.queryString("secondoHome()")};
    string dstfilepath{dsthome + "/" + dstsendfolder + "/" + filename};
    src_s.run("query sendFile(0, '" + filename + "', '" + filename +
        "', TRUE)",
        "query rquery(0, 'query removeFile('" + dstfilepath + "')')");
/*
  * Have "dst\_s"[1] access the received file as an ~frel~ and insert the
    tuples into "dst\_name"[1].

*/
    dst_s.run("let " + src_name + " = createFrel('" + dstfilepath + "')",
        "delete " + src_name);
    dst_s.run("query " + src_name + " feed " + dst_name + " insert count");
/*
  * Have "src\_s"[1] empty "src\_name"[1]. At this point, the slot data is
    successfully moved. Any further errors should not result in a rollback on
    "src\_s"[1] or "dst\_s"[1].

*/
    src_s.run("update " + src_name + " := " + src_name +
        " feed head[0] consume");
    src_s.clearRollback();
    dst_s.clearRollback();
/*
  * Unlock the ~d[f]array~.

*/
    peers.exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(dp->getDArrayName());
        });
/*
  * Clean up data remnants.

*/
    dst_s.run("delete " + src_name);
    dst_s.run("query removeFile('" + dstfilepath + "')");
    src_s.run("query removeFile('" + filename + "')");
/*
Step 4: Remove the additional temporary value mapping target status for
"src"[1] and update "dp"[1] on all peers.

*/
    peers.exec([&](auto& p){ p->lockObject(dpname, true); });
    dp->clearBufferPartition();
    peers.exec([&](auto& p){ p->updateObject(dpname, dp); });
    peers.exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(dpname);
        });
/*
The contents and mapping of "src"[1] have now been successfully and completely
moved. Save the changes made to "da"[1] and return the new slot number,
"dst"[1].

*/
    if(!SecondoSystem::GetCatalog()->UpdateObject(dp->getDArrayName(),
          Word{da.get()}))
      throw runtime_error{"Could not update " + dp->getDArrayName() + "."};
    da.release();  // UpdateObject uses the pointer after this scope ends
    res->Set(dst);
    return 0;
  }
/*
4.7 ~splitslot~ Value Mapping

*/
  int splitslotVM(Word* args, Word& result, int, Word&, Supplier s) {
    CcInt* res{&qp->template ResultStorage<CcInt>(result, s)};
/*
Lock "dpname"[1] and the referenced ~d[f]array~. This must be done as soon as
possible to avoid race conditions between the checks and updates.

*/
    string dpname{static_cast<CcString*>(args[3].addr)->GetValue()};
    ManagedMutex mutex_dp{dpname, true};
    DPartition* dp{static_cast<DPartition*>(args[0].addr)};
    ManagedMutex mutex_da{dp->getDArrayName(), true};
/*
Before proceeding, the following checks are performed on the input values and
database state:

  * Make sure the slot number was passed as a positive value and put it in
    "src"[1].

*/
    int arg{static_cast<CcInt*>(args[1].addr)->GetValue()};
    if(arg < 0)
      throw runtime_error{"A negative number was given for the slot to be "
        "split."};
    uint32_t src{static_cast<uint32_t>(arg)};
/*
  * Make sure the ~d[f]array~ referenced in "dp"[1] can be read and put it in
    "da"[1].

*/
    unique_ptr<DArrayBase> da{dp->getDArray()};
/*
  * Make sure "src"[1] exists in "da"[1].

*/
    if(src >= da->getSize())
      throw runtime_error{"The specified slot is out of range in \"" +
        dp->getDArrayName() + "\"."};
/*
  * Make sure "src"[1] is in use by "dp"[1]. This is done by retrieving its
    value-to-slot mapping (throws if no mapping to "src"[1] exists).

*/
    dp->getPartition(src);
/*
Splitting "src"[1] will proceed using the following steps. Any intricacies
and error conditions will be documented with the implementation below:

  1 Allocate a (new or recycled) slot as "dst"[1] and update "da"[1] on all
    peers.

  2 Determine the value of the first (sorted) tuple or element past the
    midpoint of the slot.

  3 Add the value mapping for "dst"[1] and mark "src"[1] as an additional
    temporary target to be used when *reading* values. Update "dp"[1] on all
    peers.

  4 Copy the matching contents of "src"[1] to "dst"[1], deleting them from
    "src"[1].

  5 Remove the additional temporary value mapping target status for "src"[1]
    and update "dp"[1] on all peers.

Step 1: Allocate a (new or recycled) slot as "dst"[1] and update "da"[1] on all
peers.

This requires an exclusive global lock on "da"[1]. An existing slot may be
recycled, or a new slot created. In the case of a recycled slot, it will be
placed on and assigned to the worker containing "src"[1] and removed from
wherever it was before.

*/
    Peers peers{static_cast<Relation*>(args[2].addr)};
    peers.exec([&](auto& p){ p->lockObject(dp->getDArrayName(), true); });
    uint32_t worker{static_cast<uint32_t>(da->getWorkerIndexForSlot(src))};
    uint32_t dst{dp->allocateSlot(worker, da.get())};
    peers.exec([&](auto& p){
        p->updateObject(dp->getDArrayName(), da.get());
        });
    peers.exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(dp->getDArrayName());
        });
/*
Step 2: Determine the value of the first (sorted) tuple or element past the
midpoint of the slot.

No lock is required for this. It makes no difference if parallel modifications
to the slot push the midpoint to a neighboring value.

*/
    Distributed2Algebra* d2{static_cast<Distributed2Algebra*>(SecondoSystem::
        GetAlgebraManager()->getAlgebra("Distributed2Algebra"))};
    const string dbname{SecondoSystem::GetInstance()->GetDatabaseName()};
    string src_name{da->getObjectNameForSlot(src)};
    string dst_name{da->getObjectNameForSlot(dst)};
    string attr_name{static_cast<CcString*>(args[4].addr)->GetValue()};
    ConnectionSession sess{d2->getWorkerConnection(da->getWorkerForSlot(src),
        dbname)};
    double partition_start{sess.queryNum(src_name + " feed kbiggest[real2int("
        + src_name + " count / 2); " + attr_name + "] tail[1] extract[" +
        attr_name + "]")};
/*
Step 3: Add the value mapping for "dst"[1] and mark "src"[1] as an additional
temporary target to be used when *reading* values. Update "dp"[1] on all peers.

This requires an exclusive global lock on "dp"[1]. Since "dst"[1] effectively
takes over part of the value range that was mapped to "src"[1] so far, any new
data in that range will be inserted in "dst"[1] instead of "src"[1].  Having
"src"[1] marked as an additional temporary target for the same value mapping as
"dst"[1] allows normal access to the data it holds.

*/
    peers.exec([&](auto& p){ p->lockObject(dpname, true); });
    dp->setBufferPartition(partition_start, src);
    dp->insertPartition(partition_start, dst);
    qp->SetModified(qp->GetSon(s, 0));
    peers.exec([&](auto& p){ p->updateObject(dpname, dp); });
    peers.exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(dpname);
        });
/*
Step 4: Copy the matching contents of "src"[1] to "dst"[1], deleting them from
"src"[1].

No lock is required for this. The copy-and-delete procedure is handled in a
single operator tree and should be consistent enough only using SMI
transactions.

*/
    sess.run("query " + src_name + " feed filter[." + attr_name + " >= " +
        to_string(partition_start) + "] " + src_name +
        " deletedirect remove[TID] " + dst_name + " insert count");
/*
Step 5: Remove the additional temporary value mapping target status for
"src"[1] and update "dp"[1] on all peers.

*/
    peers.exec([&](auto& p){ p->lockObject(dpname, true); });
    dp->clearBufferPartition();
    peers.exec([&](auto& p){ p->updateObject(dpname, dp); });
    peers.exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(dpname);
        });
/*
The contents and mapping of "src"[1] have now been successfully and completely
split. Save the changes made to "da"[1] and return the new slot number,
"dst"[1].

*/
    if(!SecondoSystem::GetCatalog()->UpdateObject(dp->getDArrayName(),
          Word{da.get()}))
      throw runtime_error{"Could not update " + dp->getDArrayName() + "."};
    da.release();  // UpdateObject uses the pointer after this scope ends
    res->Set(dst);
    return 0;
  }
/*
4.8 ~getworkerindex~ Value Mapping

*/
  int getworkerindexVM(Word* args, Word& result, int, Word&, Supplier s) {
    CcInt* res{&qp->template ResultStorage<CcInt>(result, s)};
    DArrayBase* da{static_cast<DArrayBase*>(args[0].addr)};
    string host{static_cast<CcString*>(args[1].addr)->GetValue()};
    int port{static_cast<CcInt*>(args[2].addr)->GetValue()};
    auto workers{da->getWorkers()};
    for(uint32_t i{0}; i < workers.size(); ++i)
      if(host == workers[i].getHost() && port == workers[i].getPort()) {
        res->Set(i);
        return 0;
      }
    throw runtime_error{"The worker at " + host + ":" + to_string(port) +
      " is not part of the specified d[f]array."};
  }
/*
5 Operator Descriptions

The "OperatorInfo"[1] class provides the details needed by a user. It is
derived here to keep "Distributed4Algebra.cpp"[1] clean.

5.1 ~lock~ Description

*/
  lockInfo::lockInfo() {
    name = "lock";
    signature = CcString::BasicType() + " x " + CcBool::BasicType() + " -> " +
      CcBool::BasicType();
    syntax = "lock(objname, exclusive)";
    meaning = "Acquires a lock for \"objname\". The ownership of the lock is "
      "exclusive or sharable depending on whether \"exclusive\" is given as "
      "TRUE or FALSE.";
    example = "query lock(\"da\", TRUE)";
  }
/*
5.2 ~trylock~ Description

*/
  trylockInfo::trylockInfo() {
    name = "trylock";
    signature = CcString::BasicType() + " x " + CcBool::BasicType() + " -> " +
      CcBool::BasicType();
    syntax = "trylock(objname, exclusive)";
    meaning = "Tries to acquire a lock for \"objname\". If the lock can't be "
      "acquired immediately, the operation is aborted with an error. The "
      "ownership of the lock is exclusive or sharable depending on whether "
      "\"exclusive\" is given as TRUE or FALSE.";
    example = "query trylock(\"da\", TRUE)";
  }
/*
5.3 ~unlock~ Description

*/
  unlockInfo::unlockInfo() {
    name = "unlock";
    signature = CcString::BasicType() + " -> " + CcBool::BasicType();
    syntax = "unlock(objname)";
    meaning = "Releases a lock for \"objname\". Whether the held lock is "
      "exclusive or sharable is handled internally.";
    example = "query unlock(\"da\")";
  }
/*
5.4 ~addworker~ Description

*/
  addworkerInfo::addworkerInfo() {
    name = "addworker";
    signature = "d[f]array x " + CcString::BasicType() + " x " +
      CcInt::BasicType() + " x " + CcString::BasicType() + " x " +
      Relation::BasicType() + "(" + Tuple::BasicType() + "([Host: " +
      CcString::BasicType() + ", Port: " + CcInt::BasicType() + ", Config: " +
      CcString::BasicType() + "])) -> " + CcBool::BasicType();
    syntax = "addworker(da, host, port, conffile, peers)";
    meaning = "Adds a worker given by \"host\", \"port\", and \"conffile\" to "
      "\"da\". No slots are mapped to the new worker at this point.";
    example = "query addworker(da, \"snew\", 1234, \"cfg.ini\", peers)";
    usesArgsInTypeMapping = true;
  }
/*
5.5 ~removeworker~ Description

*/
  removeworkerInfo::removeworkerInfo() {
    name = "removeworker";
    signature = "d[f]array x " + CcInt::BasicType() + " x " +
      Relation::BasicType() + "(" + Tuple::BasicType() + "([Host: " +
      CcString::BasicType() + ", Port: " + CcInt::BasicType() + ", Config: " +
      CcString::BasicType() + "])) -> " + CcBool::BasicType();
    syntax = "removeworker(da, worker, peers)";
    meaning = "Removes the worker at index \"worker\" from \"da\". The worker "
      "may not have any slots mapped to it.";
    example = "query removeworker(da, 2, peers)";
    usesArgsInTypeMapping = true;
  }
/*
5.6 ~moveslot~ Description

*/
  moveslotInfo::moveslotInfo() {
    name = "moveslot";
    signature = DPartition::BasicType() + " x " + CcInt::BasicType() + " x " +
      CcInt::BasicType() + " x " + Relation::BasicType() + "(" +
      Tuple::BasicType() + "([Host: " + CcString::BasicType() + ", Port: " +
      CcInt::BasicType() + ", Config: " + CcString::BasicType() + "])) -> " +
      CcInt::BasicType();
    syntax = "moveslot(dp, slot, worker, peers)";
    meaning = "Moves the contents of slot number \"slot\" to the worker at "
      "index \"worker\" in the d[f]array referenced from \"dp\". The result "
      "is that slot number \"slot\" will be empty and taken out of use and "
      "the contents will be in the slot given by the return value.";
    example = "query moveslot(dp, 7, 2, peers)";
    usesArgsInTypeMapping = true;
  }
/*
5.7 ~splitslot~ Description

*/
  splitslotInfo::splitslotInfo() {
    name = "splitslot";
    signature = DPartition::BasicType() + " x " + CcInt::BasicType() +
      Relation::BasicType() + "(" + Tuple::BasicType() + "([Host: " +
      CcString::BasicType() + ", Port: " + CcInt::BasicType() + ", Config: " +
      CcString::BasicType() + "])) -> " + CcInt::BasicType();
    syntax = "splitslot(dp, slot, peers)";
    meaning = "Splits the slot so that half of the containing elements remain "
      "and the other half are moved to a new slot on the same worker. The "
      "number of the new slot is returned.";
    example = "query splitslot(dp, 7, peers)";
    usesArgsInTypeMapping = true;
  }
/*
5.8 ~getworkerindex~ Description

*/
  getworkerindexInfo::getworkerindexInfo() {
    name = "getworkerindex";
    signature = "d[f]array x " + CcString::BasicType() + " x " +
      CcInt::BasicType() + " -> " + CcInt::BasicType();
    syntax = "getworkerindex(da, host, port)";
    meaning = "Retrieves the index for the worker specified with \"host\" and "
      "\"port\" as recorded in \"da\". This is useful with operators like "
      "\"removeworker\", \"moveslot\", or \"splitslot\" that expect a worker "
      "index.";
    example = "query getworkerindex(da, \"s1\", 1234)";
  }
}
