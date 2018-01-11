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

[10] Implementation of Class ADist

Adaptive Distribution

2017-11-13: Sebastian J. Bronner $<$sebastian@bronner.name$>$
2018-01-11: SJB: fixed a bug in "moveslot"[1]

\tableofcontents

1 Preliminary Setup

*/
#include "ADist.h"
#include "../Distributed2/CommandLogger.h"
#include "../Distributed2/ConnectionInfo.h"
#include "../Distributed2/Distributed2Algebra.h"
#include <functional>

namespace distributed4 {
  using distributed2::ConnectionInfo;
  using distributed2::DArrayBase;
  using distributed2::Distributed2Algebra;
  using std::function;
  using std::get;
  using std::numeric_limits;
  using std::ostream;
  using std::runtime_error;
  using std::string;
  using std::to_string;
  using std::tuple;
  using std::unique_ptr;
  using std::vector;
/*
2 Constructors

2.1 "ADist()"[1] (for "Create"[1])

The default constructor creates an instance of "ADist"[1] with no link to
a ~dpartition~ object or any other data. That is an invalid state. This
constructor is provided for use by the "Create"[1] function that the query
processor uses when instantiating a query tree. It is protected in the class
declaration to avoid accidental misuse.

*/
  ADist::ADist() {}
/*
2.2 "ADist(vector, string, size\_t)"[1]

The passed "vector"[1], "string"[1], and "size\_t"[1] are copied to the
respective member variables of the newly constructed "ADist"[1].

*/
  ADist::ADist(const string& d, size_t s, const peers_t& p):
    dpartition_name{d}, slotsize{s}, peers{p} {}
/*
2.3 "ADist(NList)"[1]

The value of the database object is passed in "list"[1]. "list"[1] is
rigorously checked and its contents applied to the proper member variables of
the newly constructed "ADist"[1].

*/
  ADist::ADist(const NList& list) {
/*
Validate the top-level structure of the nested list.

*/
    if(list.length() != 3)
      throw runtime_error{"The adist value needs to be a list of three items: "
        "a symbol atom with the referenced dpartition, a slot size, and a "
          "list of peers."};
/*
Validate and read the name of the referenced ~dpartition~ from the nested list.

*/
    NList l{list.first()};
    if(!l.isSymbol())
      throw runtime_error{"The first element, \"" + l.convertToString() +
        "\", needs to be a symbol atom."};
    dpartition_name = l.str();
/*
Make sure "dpartition\_name"[1] really refers to a ~dpartition~.

*/
    SecondoCatalog* ctlg{SecondoSystem::GetCatalog()};
    NList dpartition_type{ctlg->GetObjectTypeExpr(dpartition_name)};
    if(!DPartition::checkType(dpartition_type.listExpr()))
      throw runtime_error{"The named database object, \"" + dpartition_name +
        "\", is not a dpartition."};
/*
Validate and read the slot size.

*/
    l = list.second();
    if(!l.isInt())
      throw runtime_error{"The second element, \"" + l.convertToString() +
        "\", needs to be an integer."};
    slotsize = l.intval();
/*
Validate and read the list of peers from "list"[1].

*/
    l = list.third();
    if(l.isAtom())
      throw runtime_error{"The third element, \"" + l.convertToString() +
        "\", needs to be a list of peers."};
    peers.reserve(l.length());
    for(Cardinal i{1}; i <= l.length(); ++i) {
      NList e{l.elem(i)};
      if(!(e.length() == 3 && e.first().isString() && e.second().isInt() &&
            e.third().isString()))
        throw runtime_error{"Element " + to_string(i) + " of the third "
          "sublist, \"" + e.convertToString() + "\", needs to contain "
            "hostname, port, and configuration file of a peer."};
      peers.emplace_back(e.first().str(), e.second().intval(),
          e.third().str());
    }
/*
Store the ~dpartition~ named in "dpartition\_name"[1] and its referenced
~d[f]array~ in "dpartition"[1] and "darray"[1].

*/
    dpartition.reset(readDPartition());
    darray.reset(readDArray());
  }
/*
2.4 "ADist(ADist)"[1]

The member values "dpartition\_name"[1], "slotsize"[1], and "peers"[1] are
copied from the passed "ADist"[1] object. This differs from the default copy
constructor in that all the member variables described as state variables are
left empty. They are handled automatically by corresponding member functions.

*/
  ADist::ADist(const ADist& ad): dpartition_name{ad.dpartition_name},
    slotsize{ad.slotsize}, peers{ad.peers} {}
/*
3 Read-Only Member Functions

3.1 "listExpr"[1]

The "ADist"[1] instance is converted into its nested list representation.

*/
  ListExpr ADist::listExpr() const {
    NList l;
    l.append(dpartition_name);
    l.append(static_cast<int>(slotsize));
    NList p;
    for(auto it{peers.begin()}; it != peers.end(); ++it)
      p.append(NList{NList{get<0>(*it), true}, NList{get<1>(*it)},
          NList{get<2>(*it), true}});
    l.append(p);
    return l.listExpr();
  }
/*
3.2 "print"[1]

For easier debugging and/or output of the contents of a ADist, push all
details to a passed "ostream"[1]. This makes it usable by the "operator<<"[1]
defined below.

*/
  void ADist::print(ostream& os) const {
    os << "ADist with " << dpartition_name << " having slot size " << slotsize
      << " on " << peers;
  }
/*
3.3 "dpartitionName"[1]

*/
  string ADist::dpartitionName() const {
    return dpartition_name;
  }
/*
3.4 "darrayName"[1]

*/
  string ADist::darrayName() const {
    return dpartition->darrayName();
  }
/*
4 Modifying Member Functions

4.1 "exec"[1]

*/
  void ADist::exec(const function<void(const unique_ptr<ConnectionSession>&)>&
      f) {
/*
Be sure a connection exists to all peers and initialize the session.

*/
    if(sessions.empty()) {
      sessions.reserve(peers.size());
      for(auto p: peers)
        sessions.emplace_back(new ConnectionSession{get<0>(p), get<1>(p),
            get<2>(p)});
    }
/*
Perform "f"[1] on each peer in turn. "f"[1] is usually a "lambda"[1] calling
one of the member functions of "ConnectionSession"[1]. It is completely free to
do anything else, though. The only constraint is that it must take an element
from "sessions"[1] as its lone argument.

*/
    for_each(sessions.begin(), sessions.end(), f);
  }
/*
4.2 "addPeer"[1]

Add a new peer to "peers"[1] and distribute the referenced ~dpartition~ and
~d[f]array~ to it. If "sessions"[1] is active, keep the just used session
there.

*/
  void ADist::addPeer(const string& host, int port, const string& config) {
/*
Check the list of "peers"[1] to make sure the new peer isn't already there.

*/
    for(auto p: peers)
      if(get<0>(p) == host && get<1>(p) == port)
        throw runtime_error{"The specified peer, " + host + ":" +
          to_string(port) + ", already exists."};
/*
Begin distributing "dpartition"[1] and "darray"[1] to the new peer.

*/
    unique_ptr<ConnectionSession> cs{new ConnectionSession{host, port,
      config}};
    cs->beginTransaction();
    cs->letObject(darrayName(), darray.get());
    cs->letObject(dpartition_name, dpartition.get());
/*
Record the new peer in "peers"[1], keeping the active session in "sessions"[1]
if that has previously been initialized. Then commit the changes on the new
peer.

*/
    peers.emplace_back(host, port, config);
    if(!sessions.empty()) {
      sessions.push_back(move(cs));
      sessions.back()->commitTransaction();
    } else {
      cs->commitTransaction();
    }
  }
/*
4.3 "removePeer"[1] (by index)

Remove an existing peer from "peers"[1] and make sure its instances of the
referenced ~dpartition~ and ~d[f]array~ are removed from it, as well. If
"sessions"[1] is active, remove the session there, as well.

*/
  void ADist::removePeer(size_t index) {
    if(index >= peers.size())
      throw runtime_error{"No peer with index " + to_string(index) +
        " exists."};
/*
Establish a connection with that peer.

*/
    auto pit{peers.begin() + index};
    auto sit{sessions.begin() + index};
    unique_ptr<ConnectionSession> cs_holder;
    ConnectionSession* cs;
    if(sessions.empty()) {
      cs_holder.reset(new ConnectionSession{get<0>(*pit), get<1>(*pit),
          get<2>(*pit)});
      cs = cs_holder.get();
    } else {
      cs = sit->get();
    }
/*
Remove the referenced ~dpartition~ and ~d[f]array~ from the peer.

*/
    cs->beginTransaction();
    cs->deleteObject(dpartition_name);
    cs->deleteObject(darrayName());
    cs->commitTransaction();
/*
Remove the peer from "peers"[1] and "sessions"[1].

*/
    peers.erase(pit);
    if(!sessions.empty())
      sessions.erase(sit);
  }
/*
4.4 "removePeer"[1] (by hostname and port)

*/
  void ADist::removePeer(const string& host, int port) {
    auto pit{peers.begin()};
    while(pit != peers.end()) {
      if(get<0>(*pit) == host && get<1>(*pit) == port)
        break;
      ++pit;
    }
    if(pit == peers.end())
      throw runtime_error{"The peer given by " + host + ":" + to_string(port) +
        " was not found."};
    removePeer(pit - peers.begin());
  }
/*
4.5 "addWorker"[1]

*/
  void ADist::addWorker(const string& host, int port, const string& conf) {
/*
Before proceeding, the following checks are performed on the input values and
database state:

  * The list of workers in "darray"[1] is checked to make sure the specified
    worker isn't already there.

*/
    auto workers{darray->getWorkers()};
    for(auto w: workers)
      if(w.getHost() == host && w.getPort() == port)
        throw runtime_error{"The specified worker already exists."};
/*
  * A connection attempt is made to the new worker to make sure communication
    is possible.

*/
    string c{conf};
    ConnectionInfo* ci{ConnectionInfo::createConnection(host, port, c)};
    if(!ci)
      throw runtime_error{"The specified worker could not be contacted."};
    delete ci;
/*
The following steps cause a synchronized update of "darray"[1] locally and on
all peers:

  * Add the new worker to the in-memory instance "darray"[1].

  * Lock "darray"[1] for exclusive access on all peers (implicitly ensuring
    "darray"[1] exists there).

  * Begin a transaction and update "darray"[1].

  * Save the changes to the in-memory instance "darray"[1] to the persistent
    database object.

  * Commit the transaction.

  * Unlock "darray"[1] on all peers.

*/
    auto num{workers.back().getNum() + 1};
    workers.emplace_back(host, port, num, conf);
    darray->set(darray->getMap(), darray->getName(), workers);
    exec([&](auto& p){ p->lockObject(this->darrayName(), true); });
    exec([&](auto& p){
        p->beginTransaction();
        p->updateObject(this->darrayName(), darray.get());
        });
    if(!SecondoSystem::GetCatalog()->UpdateObject(darrayName(),
          Word{darray.get()}))
      throw runtime_error{"Could not update " + darrayName() + "."};
    darray.release();  // UpdateObject uses the pointer after query is done
    exec([](auto& p){ p->commitTransaction(); });
    exec([&](auto& p){ p->unlockObject(this->darrayName()); });
  }
/*
4.6 "removeWorker"[1] (by index)

*/
  void ADist::removeWorker(size_t index) {
/*
Before proceeding, the following checks are performed on the input values and
database state:

  * Make sure a worker at "index"[1] exists in "darray"[1].

*/
    auto workers{darray->getWorkers()};
    if(index >= workers.size())
      throw runtime_error{"The specified worker index is out of range."};
/*
  * Make sure "index"[1] is not responsible for any slots.

*/
    auto map{darray->getMap()};
    for(auto m: map)
      if(m == index)
        throw runtime_error{"The specified worker still has slots mapped to "
          "it."};
/*
Next, any existing connections for that worker are closed.

*/
    static_cast<Distributed2Algebra*>(SecondoSystem::GetAlgebraManager()->
        getAlgebra("Distributed2Algebra"))->closeWorker(workers[index]);
/*
The following steps cause a synchronized update of "darray"[1] locally and on
all peers:

  * Remove "index"[1] from the in-memory instance "darray"[1]. The
    slot-to-worker mapping for all slots mapped to subsequent workers must be
    decremented to stay in sync with the decremented worker indices.

  * Lock "darray"[1] for exclusive access on all peers (implicitly ensuring
    "darray"[1] exists there).

  * Begin a transaction and update "darray"[1].

  * Save the changes to the in-memory instance "darray"[1] to the persistent
    database object.

  * Commit the transaction.

  * Unlock "darray"[1] on all peers.

*/
    for(auto it{map.begin()}; it != map.end(); ++it)
      if(*it > index)
        --(*it);
    workers.erase(workers.begin() + index);
    darray->set(map, darray->getName(), workers);
    exec([&](auto& p){ p->lockObject(this->darrayName(), true); });
    exec([&](auto& p){
        p->beginTransaction();
        p->updateObject(this->darrayName(), darray.get());
        });
    if(!SecondoSystem::GetCatalog()->UpdateObject(darrayName(),
          Word{darray.get()}))
      throw runtime_error{"Could not update " + darrayName() + "."};
    darray.release();  // UpdateObject uses the pointer after query is done
    exec([](auto& p){ p->commitTransaction(); });
    exec([&](auto& p){ p->unlockObject(this->darrayName()); });
  }
/*
4.7 "removeWorker"[1] (by hostname and port)

*/
  void ADist::removeWorker(const string& host, int port) {
    auto workers{darray->getWorkers()};
    auto it{workers.begin()};
    while(it != workers.end()) {
      if(it->getHost() == host && it->getPort() == port)
        break;
      ++it;
    }
    if(it == workers.end())
      throw runtime_error{"The worker given by " + host + ":" + to_string(port)
        + " was not found."};
    removeWorker(it - workers.begin());
  }
/*
4.8 "moveSlot"[1] (by worker index)

*/
  void ADist::moveSlot(uint32_t slot, uint32_t index) {
/*
Before proceeding, the following checks are performed on the input values and
database state:

  * Make sure "slot"[1] and the worker "index"[1] exist in "darray"[1].

*/
    if(slot >= darray->getSize())
      throw runtime_error{"The specified slot is out of range in \"" +
        darrayName() + "\"."};
    auto workers{darray->getWorkers()};
    if(index >= workers.size())
      throw runtime_error{"The specified worker index is out of range in \"" +
        darrayName() + "\"."};
/*
  * Make sure "index"[1] is not the worker "slot"[1] is already on.

*/
    if(index == darray->getWorkerIndexForSlot(slot))
      throw runtime_error{"The specified slot is to be moved to the worker it "
        "is already on. Aborting."};
/*
  * Determine if "slot"[1] is in use. This is needed for decisions made by the
    steps below. If the slot is in use, its value mapping is determined as a
    side effect.

*/
    double partition_start;
    bool inuse;
    try {
      partition_start = dpartition->partition(slot);
      inuse = true;
    } catch(const runtime_error&) {
      inuse = false;
    }
/*
Moving "slot"[1] will proceed using the following steps. It is possible that
the slot to be moved does not have any value range mapped to it. Such a slot is
considered not to be in use. It can simply be copied. It is not necessary (or
possible) to adjust the value mapping for inserts to reach it during transfer
(all steps but the copy step, step 3, will be skipped). Any intricacies or
possible error conditions pertaining to the individual steps will be documented
with the implementation below:

  1 Allocate a (new or recycled) slot as "temp"[1] and update "darray"[1] on
    all peers.

  2 Set "temp"[1] as the target of the value mapping currently used for
    "slot"[1] and mark "slot"[1] as an additional temporary target to be used
    when *reading* values. Update "dpartition"[1] on all peers.

  3 Copy "slot"[1] as a unit to the worker at "index"[1] and update "darray"[1]
    on all peers. Then remove "slot"[1] from its worker of origin.

  4 Return "slot"[1] to its original value mapping and mark "temp"[1] as an
    additional temporary target to be used when *reading* values. Update
    "dpartition"[1] on all peers.

  5 Move the contents of "temp"[1] to "slot"[1].

  6 Remove the temporary value mapping for "temp"[1] and update "dpartition"[1]
    on all peers.

  7 If "temp"[1] happens to be the last slot in "darray"[1], remove it
    completely.

Step 1: Allocate a (new or recycled) slot as "temp"[1] and update "darray"[1]
on all peers.

This requires an exclusive global lock on "darray"[1]. An existing slot may be
recycled, or a new slot created. In the case of a recycled slot, it will be
placed on and assigned to "index"[1] and removed from wherever it was before.

*/
    uint32_t temp;
    if(inuse) {
      exec([&](auto& p){ p->lockObject(this->darrayName(), true); });
      temp = dpartition->allocateSlot(index, darray.get());
      exec([&](auto& p){ p->updateObject(this->darrayName(), darray.get()); });
      exec([&](auto& p){
          p->clearRollback();
          p->unlockObject(this->darrayName());
          });
/*
Step 2: Set "temp"[1] as the target of the value mapping currently used for
"slot"[1] and mark "slot"[1] as an additional temporary target to be used when
*reading* values. Update "dpartition"[1] on all peers.

This requires an exclusive global lock on "dpartition"[1]. Since "temp"[1]
replaces "slot"[1], any new data will be inserted in "temp"[1] instead of
"slot"[1]. If that weren't the case, step 3 would be copying data out of a slot
that is still receiving data. Having "slot"[1] marked as an additional
temporary target for its previous value mapping allows normal access to the
data it holds.

*/
      exec([&](auto& p){ p->lockObject(dpartition_name, true); });
      dpartition->setBufferPartition(partition_start, slot);
      dpartition->resetPartition(partition_start, temp);
      exec([&](auto& p){
          p->updateObject(dpartition_name, dpartition.get());
          });
      exec([&](auto& p){
          p->clearRollback();
          p->unlockObject(dpartition_name);
          });
    }
/*
Step 3: Copy "slot"[1] as a unit to the worker at "index"[1] and update
"darray"[1] on all peers. Then remove "slot"[1] from its worker of origin.

This requires an exclusive global lock on "darray"[1].

*/
    Distributed2Algebra* d2{static_cast<Distributed2Algebra*>(SecondoSystem::
        GetAlgebraManager()->getAlgebra("Distributed2Algebra"))};
    const string dbname{SecondoSystem::GetInstance()->GetDatabaseName()};
    ConnectionSession sess{d2->getWorkerConnection(
        darray->getWorkerForSlot(slot), dbname)};
    string slot_name{darray->getObjectNameForSlot(slot)};
    exec([&](auto& p){ p->lockObject(this->darrayName(), true); });
    sess.run("query share(\"" + slot_name + "\", FALSE, [const "
        "rel(tuple([Host: string, Port: int, Config: string])) value ((\"" +
        workers[index].getHost() + "\" " + to_string(workers[index].getPort())
        + " \"" + workers[index].getConfig() + "\"))])");
    darray->setResponsible(slot, index);
    exec([&](auto& p){ p->updateObject(this->darrayName(), darray.get()); });
    exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(this->darrayName());
        });
    sess.run("delete " + slot_name);
    sess.clearRollback();
/*
Step 4: Return "slot"[1] to its original value mapping and mark "temp"[1] as an
additional temporary target to be used when *reading* values. Update
"dpartition"[1] on all peers.

This requires an exclusive global lock on "dpartition"[1].

*/
    if(inuse) {
      exec([&](auto& p){ p->lockObject(dpartition_name, true); });
      dpartition->clearBufferPartition();
      dpartition->setBufferPartition(partition_start, temp);
      dpartition->resetPartition(partition_start, slot);
      exec([&](auto& p){ p->updateObject(dpartition_name, dpartition.get()); });
      exec([&](auto& p){
          p->clearRollback();
          p->unlockObject(dpartition_name);
          });
/*
Step 5: Move the contents of "temp"[1] to "slot"[1].

This requires a shared global lock on "darray"[1]. Both slots "slot"[1] and
"temp"[1] may be accessed for read access by other processes during this
operation. Care should be taken because the copied tuples will exist in both
"slot"[1] and "temp"[1] until "temp"[1] is cleared. All write accesses (except
independent inserts, which require no lock) are blocked. It is absolutely vital
that "temp"[1] be empty at the end of this operation. Otherwise future attempts
to reuse it will fail.

*/
      sess = ConnectionSession{d2->getWorkerConnection(
          darray->getWorker(index), dbname)};
      string temp_name{darray->getObjectNameForSlot(temp)};
      exec([&](auto& p){ p->lockObject(this->darrayName(), false); });
      sess.run("query " + temp_name + " feed " + slot_name + " insert count");
      sess.run("update " + temp_name + " := " + temp_name +
          " feed head[0] consume");
      exec([&](auto& p){
          p->clearRollback();
          p->unlockObject(this->darrayName());
          });
      sess.clearRollback();
/*
Step 6: Remove the temporary value mapping for "temp"[1] and update
"dpartition"[1] on all peers.

*/
      exec([&](auto& p){ p->lockObject(dpartition_name, true); });
      dpartition->clearBufferPartition();
      exec([&](auto& p){ p->updateObject(dpartition_name, dpartition.get()); });
      exec([&](auto& p){
          p->clearRollback();
          p->unlockObject(dpartition_name);
          });
/*
Step 7: If "temp"[1] happens to be the last slot in "darray"[1], remove it
completely.

This requires an exclusive global lock on "darray"[1]. Be sure to update
"darray"[1] on all peers.

*/
      if(temp == darray->getSize() - 1) {
        auto map{darray->getMap()};
        map.pop_back();
        darray->set(map, darray->getName(), workers);
        exec([&](auto& p){ p->lockObject(this->darrayName(), true); });
        exec([&](auto& p){
            p->beginTransaction();
            p->updateObject(this->darrayName(), darray.get());
            });
        sess.beginTransaction();
        sess.run("delete " + temp_name);
        exec([](auto& p){ p->commitTransaction(); });
        sess.commitTransaction();
        exec([&](auto& p){ p->unlockObject(this->darrayName()); });
      }
    }
/*
"slot"[1] has now been successfully and completely moved. Save the changes made
to "darray"[1]. All changes made to "dpartition"[1] were removed again, so it
does not need to be written out to secondary storage.

*/
    SecondoCatalog* ctlg{SecondoSystem::GetCatalog()};
    if(!ctlg->UpdateObject(darrayName(), Word{darray.get()}))
      throw runtime_error{"Could not update " + darrayName() + "."};
    darray.release();  // UpdateObject uses the pointer after query is done
  }
/*
4.9 "moveSlot"[1] (by worker hostname and port)

*/
  void ADist::moveSlot(uint32_t slot, const string& host, int port) {
    auto workers{darray->getWorkers()};
    auto it{workers.begin()};
    while(it != workers.end()) {
      if(it->getHost() == host && it->getPort() == port)
        break;
      ++it;
    }
    if(it == workers.end())
      throw runtime_error{"The worker given by " + host + ":" + to_string(port)
        + " was not found."};
    moveSlot(slot, it - workers.begin());
  }
/*
4.10 "splitSlot"[1]

*/
  uint32_t ADist::splitSlot(uint32_t src) {
/*
Before proceeding, the following checks are performed on the input values and
database state:

  * Make sure "src"[1] exists in "darray"[1].

*/
    if(src >= darray->getSize())
      throw runtime_error{"The specified slot is out of range in \"" +
        darrayName() + "\"."};
/*
  * Make sure "src"[1] is in use by "dpartition"[1]. This is done by retrieving
    its value-to-slot mapping (throws if no mapping to "src"[1] exists).

*/
    dpartition->partition(src);
/*
Splitting "src"[1] will proceed using the following steps. Any intricacies
and error conditions will be documented with the implementation below:

  1 Allocate a (new or recycled) slot as "dst"[1] and update "darray"[1] on all
    peers.

  2 Determine the value of the first (sorted) tuple or element past the
    midpoint of the slot.

  3 Add the value mapping for "dst"[1] and mark "src"[1] as an additional
    temporary target to be used when *reading* values. Update "dpartition"[1]
    on all peers.

  4 Copy the matching contents of "src"[1] to "dst"[1], deleting them from
    "src"[1].

  5 Remove the additional temporary value mapping target status for "src"[1]
    and update "dpartition"[1] on all peers.

Step 1: Allocate a (new or recycled) slot as "dst"[1] and update "darray"[1] on
all peers.

This requires an exclusive global lock on "darray"[1]. An existing slot may be
recycled, or a new slot created. In the case of a recycled slot, it will be
placed on and assigned to the worker containing "src"[1] and removed from
wherever it was before.

*/
    exec([&](auto& p){ p->lockObject(this->darrayName(), true); });
    uint32_t worker{static_cast<uint32_t>(darray->getWorkerIndexForSlot(src))};
    uint32_t dst{dpartition->allocateSlot(worker, darray.get())};
    exec([&](auto& p){
        p->updateObject(this->darrayName(), darray.get());
        });
    exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(this->darrayName());
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
    string src_name{darray->getObjectNameForSlot(src)};
    string dst_name{darray->getObjectNameForSlot(dst)};
    SecondoCatalog* ctlg{SecondoSystem::GetCatalog()};
    string attr_name{NList{ctlg->GetObjectTypeExpr(dpartition_name)}.
      second().first().str()};
    ConnectionSession sess{d2->getWorkerConnection(
        darray->getWorkerForSlot(src), dbname)};
    double partition_start{sess.queryNumeric(src_name +
        " feed kbiggest[real2int(" + src_name + " count / 2); " + attr_name +
        "] tail[1] extract[" + attr_name + "]")};
/*
Step 3: Add the value mapping for "dst"[1] and mark "src"[1] as an additional
temporary target to be used when *reading* values. Update "dpartition"[1] on
all peers.

This requires an exclusive global lock on "dpartition"[1]. Since "dst"[1]
effectively takes over part of the value range that was mapped to "src"[1] so
far, any new data in that range will be inserted in "dst"[1] instead of
"src"[1].  Having "src"[1] marked as an additional temporary target for the
same value mapping as "dst"[1] allows normal access to the data it holds.

*/
    exec([&](auto& p){ p->lockObject(dpartition_name, true); });
    dpartition->setBufferPartition(partition_start, src);
    dpartition->insertPartition(partition_start, dst);
    exec([&](auto& p){ p->updateObject(dpartition_name, dpartition.get()); });
    exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(dpartition_name);
        });
/*
Step 4: Copy the matching contents of "src"[1] to "dst"[1], deleting them from
"src"[1].

This requires a shared global lock on "darray"[1]. Both slots "src"[1] and
"dst"[1] may be accessed for read access by other processes during this
operation. As write access requires an exclusive lock (except pure inserts,
which require no lock), that is blocked for the duration of the copy procedure.
The copied tuples exist in both "src"[1] and "dst"[1] until the procedure is
complete. It is absolutely vital that "src"[1] be empty at the end of this
operation.  Otherwise future attempts to reuse it will fail.

*/
    exec([&](auto& p){ p->lockObject(this->darrayName(), false); });
    sess.run("query " + src_name + " feed filter[." + attr_name + " >= " +
        to_string(partition_start) + "] " + src_name +
        " deletedirect remove[TID] " + dst_name + " insert count");
    exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(this->darrayName());
        });
/*
Step 5: Remove the additional temporary value mapping target status for
"src"[1] and update "dpartition"[1] on all peers.

*/
    exec([&](auto& p){ p->lockObject(dpartition_name, true); });
    dpartition->clearBufferPartition();
    exec([&](auto& p){ p->updateObject(dpartition_name, dpartition.get()); });
    exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(dpartition_name);
        });
/*
The contents and mapping of "src"[1] have now been successfully and completely
split. Save the changes made to "darray"[1] and "dpartition"[1] and return the
new slot number, "dst"[1].

*/
    if(!ctlg->UpdateObject(darrayName(), Word{darray.get()}))
      throw runtime_error{"Could not update " + darrayName() + "."};
    darray.release();  // UpdateObject uses the pointer after query is done
    if(!ctlg->UpdateObject(dpartition_name, Word{dpartition.get()}))
      throw runtime_error{"Could not update " + dpartition_name + "."};
    dpartition.release();  // UpdateObject uses the pointer after query is done
    return dst;
  }
/*
4.11 "mergeSlots"[1]

*/
  uint32_t ADist::mergeSlots(uint32_t slot1, uint32_t slot2) {
/*
Before proceeding, the following checks are performed on the input values and
database state:

  * Make sure "slot1"[1] and "slot2"[1] are not the same slot and that they
    exist in "darray"[1].

*/
    if(slot1 == slot2)
      throw runtime_error{"The two slots to be merged (" + to_string(slot1) +
        " and " + to_string(slot2) + " are identical."};
    if(slot1 >= darray->getSize())
      throw runtime_error{"Slot " + to_string(slot1) + " is out of range in \""
        + darrayName() + "\"."};
    if(slot2 >= darray->getSize())
      throw runtime_error{"Slot " + to_string(slot2) + " is out of range in \""
        + darrayName() + "\"."};
/*
  * Make sure the values mapped to "slot1"[1] and "slot2"[1] are contiguous.
    Store their value mappings in "slot1\_start"[1] and "slot2\_start"[1],
    respectively.

*/
    double slot1_start{dpartition->partition(slot1)};
    double slot2_start{dpartition->partition(slot2)};
    dpartition->checkContiguous(slot1_start, slot2_start);
/*
  * Make sure that "slot1"[1] and "slot2"[1] are both on the same worker.

*/
    if(darray->getWorkerIndexForSlot(slot1) !=
        darray->getWorkerIndexForSlot(slot2))
      throw runtime_error{"Slots " + to_string(slot1) + " and " +
        to_string(slot2) + " don't currently reside on the same worker."};
/*
Merging "slot1"[1] with "slot2"[1] will proceed using the following steps. Any
intricacies and error conditions will be documented with the implementation
below:

  1 Determine which slot will hold the combined data and which slot will be
    freed. Assign them to "src"[1] and "dst"[1].

  2 Remove the value mapping for "src"[1] and mark it as an additional
    temporary target to be used when *reading* values. Update "dpartition"[1]
    on all peers.

  3 Copy the contents of "src"[1] to "dst"[1].

  4 Remove the additional temporary value mapping target status for "src"[1]
    and update "dpartition"[1] on all peers.

  5 If "src"[1] happens to be the last slot in "darray"[1], remove it
    completely.

Step 1: Determine which slot will hold the combined data and which slot will be
freed. Assign them to "src"[1] and "dst"[1].

The slot with more data already in it will be selected as "dst"[1]. If both
slots have the same amount of data, the slot with the lower number will be
selected as "dst"[1]. No lock is required for this. If the slots are so close
in size, that parallel modifications might change the outcome of this
algorithm, the difference is negligible, anyway.

*/
    Distributed2Algebra* d2{static_cast<Distributed2Algebra*>(SecondoSystem::
        GetAlgebraManager()->getAlgebra("Distributed2Algebra"))};
    const string dbname{SecondoSystem::GetInstance()->GetDatabaseName()};
    string slot1_name{darray->getObjectNameForSlot(slot1)};
    string slot2_name{darray->getObjectNameForSlot(slot2)};
    ConnectionSession sess{d2->getWorkerConnection(
        darray->getWorkerForSlot(slot1), dbname)};
    int slot1_count{sess.queryInt(slot1_name + " count")};
    int slot2_count{sess.queryInt(slot2_name + " count")};
    bool toslot1{slot1_count > slot2_count || (slot1_count == slot2_count &&
        slot1 < slot2)};
    uint32_t src{toslot1 ? slot2 : slot1};
    uint32_t dst{toslot1 ? slot1 : slot2};
    double src_start{toslot1 ? slot2_start : slot1_start};
    double dst_start{toslot1 ? slot1_start : slot2_start};
    string src_name{toslot1 ? slot2_name : slot1_name};
    string dst_name{toslot1 ? slot1_name : slot2_name};
/*
Step 2: Set the value mapping for "dst"[1] to encompass the values formerly
mapped to both "src"[1] and "dst"[1] and remove the value mapping for "src"[1].
Also mark "src"[1] as an additional temporary target to be used when *reading*
values. Update "dpartition"[1] on all peers.

This requires an exclusive global lock on "dpartition"[1]. Since "dst"[1] takes
over the value range that was mapped to "src"[1], any new data in that range
will be inserted in "dst"[1] instead of "src"[1]. Having "src"[1] marked as an
additional temporary target for its value mapping allows normal access to the
data it holds.

*/
    exec([&](auto& p){ p->lockObject(dpartition_name, true); });
    dpartition->setBufferPartition(src_start, src);
    if(dst_start < src_start) {
      dpartition->removePartition(src_start);
    } else {
      dpartition->resetPartition(src_start, dst);
      dpartition->removePartition(dst_start);
    }
    exec([&](auto& p){ p->updateObject(dpartition_name, dpartition.get()); });
    exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(dpartition_name);
        });
/*
Step 3: Copy the contents of "src"[1] to "dst"[1].

This requires a shared global lock on "darray"[1]. Both slots "src"[1] and
"dst"[1] may be accessed for read access by other processes during this
operation. As write access requires an exclusive lock (except pure inserts,
which require no lock), that is blocked for the duration of the copy procedure.
The copied tuples exist in both "src"[1] and "dst"[1] until the procedure is
complete. It is absolutely vital that "src"[1] be empty at the end of this
operation.  Otherwise future attempts to reuse it will fail.

*/
    exec([&](auto& p){ p->lockObject(this->darrayName(), false); });
    sess.run("query " + src_name + " feed " + dst_name + " insert count");
    sess.run("update " + src_name + " := " + src_name +
        " feed head[0] consume");
    sess.clearRollback();
    exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(this->darrayName());
        });
/*
Step 4: Remove the additional temporary value mapping target status for
"src"[1] and update "dpartition"[1] on all peers.

*/
    exec([&](auto& p){ p->lockObject(dpartition_name, true); });
    dpartition->clearBufferPartition();
    exec([&](auto& p){ p->updateObject(dpartition_name, dpartition.get()); });
    exec([&](auto& p){
        p->clearRollback();
        p->unlockObject(dpartition_name);
        });
/*
Step 5: If "src"[1] happens to be the last slot in "darray"[1], remove it
completely. 

This requires an exclusive global lock on "darray"[1]. Be sure to update
"darray"[1] on all peers and save any changes made locally.

*/
    SecondoCatalog* ctlg{SecondoSystem::GetCatalog()};
    if(src == darray->getSize() - 1) {
      auto map{darray->getMap()};
      map.pop_back();
      darray->set(map, darray->getName(), darray->getWorkers());
      exec([&](auto& p){ p->lockObject(this->darrayName(), true); });
      exec([&](auto& p){
          p->beginTransaction();
          p->updateObject(this->darrayName(), darray.get());
          });
      sess.beginTransaction();
      sess.run("delete " + src_name);
      exec([](auto& p){ p->commitTransaction(); });
      sess.commitTransaction();
      exec([&](auto& p){ p->unlockObject(this->darrayName()); });
      if(!ctlg->UpdateObject(darrayName(), Word{darray.get()}))
        throw runtime_error{"Could not update " + darrayName() + "."};
      darray.release();  // UpdateObject uses the pointer after query is done
    }
/*
The contents and mapping of "src"[1] have now been successfully and completely
merged with "dst"[1]. Save the changes made to "dpartition"[1] and return the
slot number of "dst"[1].

*/
    if(!ctlg->UpdateObject(dpartition_name, Word{dpartition.get()}))
      throw runtime_error{"Could not update " + dpartition_name + "."};
    dpartition.release();  // UpdateObject uses the pointer after query is done
    return dst;
  }
/*
5 Protected Member Functions

5.1 "readDPartition"[1]

*/
  DPartition* ADist::readDPartition() {
    if(dpartition_name.empty())
      return nullptr;

    Word ref;
    bool isdefined;
    if(!SecondoSystem::GetCatalog()->GetObject(dpartition_name, ref,
          isdefined))
      throw runtime_error{"The dpartition " + dpartition_name +
          " couldn't be opened."};
    if(!isdefined)
      throw runtime_error{"The dpartition " + dpartition_name +
        " is undefined."};
    return static_cast<DPartition*>(ref.addr);
  }
/*
5.2 "readDArray"[1]

*/
  DArrayBase* ADist::readDArray() {
    if(!dpartition)
      return nullptr;
    return dpartition->darray();
  }
/*
5 Static Member Functions

5.1 "BasicType"[1]

"BasicType"[1] is required by [secondo]. It returns [secondo]'s basic type for
this class.

*/
  string ADist::BasicType() {
    return "adist";
  }
/*
5.2 "Out"[1]

The Out function takes a nested list type expression and a "Word"[1] containing
a pointer to an instance of that type as arguments. It returns the value
represented by the instance as a nested list value expression. This is done in
the form of a "ListExpr"[1]. The passed type expression is commonly ignored.

*/
  ListExpr ADist::Out(ListExpr, Word value) {
    return static_cast<ADist*>(value.addr)->listExpr();
  }
/*
5.3 "In"[1]

The In function takes a nested list type expression, a nested list value
expression, and an integer position. It passes back a nested list error
expression, and a boolean correctness value. It returns a "Word"[1] containing
a pointer to an object instantiated based on the passed value expression.

*/
  Word ADist::In(ListExpr, ListExpr instance, int, ListExpr&,
      bool& correct) {
    correct = false;
    ADist* addr{new ADist{NList{instance}}};
    correct = true;
    return Word{addr};
  }
/*
5.4 "checkType"[1]

"checkType"[1] is required by [secondo] as well. It checks the passed ListExpr
to make sure that it is a valid type description for this class.

*/
  bool ADist::checkType(const NList& type) {
    if(!(type.isAtom() && type.str() == BasicType()))
      throw runtime_error{"The type expression needs to be a single atom with "
        "the value \"" + BasicType() + "\"."};
    return true;
  }
/*
5.5 "checkType"[1] (wrapper for consistency with other types)

*/
  bool ADist::checkType(ListExpr type) {
    return checkType(NList{type});
  }
/*
5.6 "checkType"[1] (wrapper for "Functions"[1])

*/
  bool ADist::checkType(ListExpr type, ListExpr&) {
    return checkType(NList{type});
  }
/*
6 Member Structs

6.1 "Info"[1]

This struct inherits from "ConstructorInfo"[1] and is responsible for
formatting information about the type constructor for the database user.

*/
  ADist::Info::Info() {
    name = BasicType();
    signature = "-> SIMPLE";
    typeExample = "adist";
    listRep = "(<dpartition> <slot size> <peers>)";
    valueExample = "(dp 1000 ((\"s1\" 1234 \"cfg\") (\"s2\" 1234 \"cfg\")))";
    remarks = "This type provides the means by which <dpartition> can be "
      "restructured on several <peers> in a synchronized fashion. This "
      "enables both scaling and load-based re-distribution of data.";
  }
/*
6.2 "Functions"[1]

This struct inherits from "ConstructorFunctions"[1] and defines the functions
needed by a type constructor.

*/
  ADist::Functions::Functions() {
    out = ADist::Out;
    in = ADist::In;
    open = save = 0;
    kindCheck = ADist::checkType;
  }
/*
7 Stream Operators Used by ADist

7.1 "operator<<"[1] (map)

This operator allows formatting the pairs in a map for a character stream.

*/
  ostream& operator<<(ostream& os, const peers_t& p) {
    os << "[";
    for(auto it = p.begin(); it != p.end(); ++it) {
      if(it != p.begin()) os << ", ";
      os << "(" << get<0>(*it) << ", " << get<1>(*it) << ", " << get<2>(*it) <<
        ")";
    }
    os << "]";
    return os;
  }
/*
7.2 "operator<<"[1] (ADist)

*/
  ostream& operator<<(ostream& os, const ADist& ad) {
    ad.print(os);
    return os;
  }
}
