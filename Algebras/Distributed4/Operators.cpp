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

[10] Implementation of Operators in Algebra Distributed4

2017-11-13: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 Preliminary Setup

*/
#include "Operators.h"
#include "ManagedMutex.h"
#include "ADist.h"

namespace distributed4 {
  using std::runtime_error;
  using std::string;
  using std::to_string;
/*
2 Type Mapping Functions

These functions evaluate a "ListExpr"[1] to see if it matches the expected
input type of the corresponding operator. They return a "ListExpr"[1] of the
operator's result data type.

Each Type Mapping Function has the same basic two-step structure: (1) Verify
the number of arguments. (2) Verify the type of arguments.

2.1 ~lock~ Type Mapping (also used by ~trylock~)

*/
  ListExpr lockTM(ListExpr args) {
    NList l{args};
    string err = "string x bool expected";
    if(!(l.checkLength(2, err) && CcString::checkType(l.first().listExpr()) &&
          CcBool::checkType(l.second().listExpr())))
      return NList::typeError(err);
    return NList{CcBool::BasicType()}.listExpr();
  }
/*
2.2 ~unlock~ Type Mapping

*/
  ListExpr unlockTM(ListExpr args) {
    NList l{args};
    string err = "string expected";
    if(!(l.checkLength(1, err) && CcString::checkType(l.first().listExpr())))
      return NList::typeError(err);
    return NList{CcBool::BasicType()}.listExpr();
  }
/*
2.3 ~addhost~ Type Mapping (for d[f]array workers or adist peers)

*/
  ListExpr addhostTM(ListExpr args) {
    NList l{args};
    string err = "adist x string x int x string expected";
    if(!(l.checkLength(4, err) && ADist::checkType(l.first()) &&
          CcString::checkType(l.second().listExpr()) &&
          CcInt::checkType(l.third().listExpr()) &&
          CcString::checkType(l.fourth().listExpr())))
      return NList::typeError(err);
    return NList{CcBool::BasicType()}.listExpr();
  }
/*
2.4 ~removehost~ Type Mapping (for d[f]array workers or adist peers)

*/
  ListExpr removehostTM(ListExpr args) {
    NList l{args};
    string err = "adist x {int, string x int} expected";
    if(l.length() < 2 || l.length() > 3 || !ADist::checkType(l.first()) ||
        (l.length() == 2 && !CcInt::checkType(l.second().listExpr())) ||
        (l.length() == 3 && (!CcString::checkType(l.second().listExpr()) ||
                             !CcInt::checkType(l.third().listExpr()))))
      return NList::typeError(err);
    return NList{CcBool::BasicType()}.listExpr();
  }
/*
2.5 ~moveslot~ Type Mapping

*/
  ListExpr moveslotTM(ListExpr args) {
    NList l{args};
    string err = "adist x int x {int, string x int} expected";
    if(l.length() < 3 || l.length() > 4 || !ADist::checkType(l.first()) ||
        !CcInt::checkType(l.second().listExpr()) || (l.length() == 3 &&
          !CcInt::checkType(l.third().listExpr())) || (l.length() == 4 &&
          (!CcString::checkType(l.third().listExpr()) ||
           !CcInt::checkType(l.fourth().listExpr()))))
      return NList::typeError(err);
    return NList{CcBool::BasicType()}.listExpr();
  }
/*
2.6 ~splitslot~ Type Mapping

*/
  ListExpr splitslotTM(ListExpr args) {
    NList l{args};
    string err = "adist x int expected";
    if(!(l.checkLength(2, err) && ADist::checkType(l.first()) &&
          CcInt::checkType(l.second().listExpr())))
      return NList::typeError(err);
    return NList{CcInt::BasicType()}.listExpr();
  }
/*
2.7 ~mergeslots~ Type Mapping

*/
  ListExpr mergeslotsTM(ListExpr args) {
    NList l{args};
    string err = "adist x int x int expected";
    if(!(l.checkLength(3, err) && ADist::checkType(l.first()) &&
          CcInt::checkType(l.second().listExpr()) &&
          CcInt::checkType(l.third().listExpr())))
      return NList::typeError(err);
    return NList{CcInt::BasicType()}.listExpr();
  }
/*
3 Selection Functions

3.1 "removehostSel"[1] (for d[f]array workers or adist peers)

~removeworker~ and ~removepeer~ may be called either given an index for the
host to be removed or a hostname and port in addition to the object from which
that host is to be removed. The selection is made simply by the number of
arguments.

*/
  int removehostSel(ListExpr args) {
    // obj x int
    // obj x string x int
    return NList{args}.length() - 2;
  }
/*
3.2 "moveslotSel"[1]

*/
  int moveslotSel(ListExpr args) {
    // adist x int x int
    // adist x int x string x int
    return NList{args}.length() - 3;
  }
/*
4 Value Mapping Functions/Arrays

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
4.4 ~addpeer~ Value Mapping

*/
  int addpeerVM(Word* args, Word& result, int, Word&, Supplier s) {
    CcBool* res{&qp->template ResultStorage<CcBool>(result, s)};
    ADist* ad{static_cast<ADist*>(args[0].addr)};
    string host{static_cast<CcString*>(args[1].addr)->GetValue()};
    int port{static_cast<CcInt*>(args[2].addr)->GetValue()};
    string conf{static_cast<CcString*>(args[3].addr)->GetValue()};
    ad->addPeer(host, port, conf);
    qp->SetModified(qp->GetSon(s, 0));
    res->Set(true, true);
    return 0;
  }
/*
4.5 ~removepeer~ Value Mapping (overloaded)

4.5.1 "removepeerVM\_Index"[1]

*/
  int removepeerVM_Index(Word* args, Word& result, int, Word&, Supplier s) {
    CcBool* res{&qp->template ResultStorage<CcBool>(result, s)};
    ADist* ad{static_cast<ADist*>(args[0].addr)};
    int index{static_cast<CcInt*>(args[1].addr)->GetValue()};
    if(index < 0)
      throw runtime_error{"A negative index was given for the peer."};
    ad->removePeer(index);
    qp->SetModified(qp->GetSon(s, 0));
    res->Set(true, true);
    return 0;
  }
/*
4.5.2 "removepeerVM\_HostPort"[1]

*/
  int removepeerVM_HostPort(Word* args, Word& result, int, Word&, Supplier s) {
    CcBool* res{&qp->template ResultStorage<CcBool>(result, s)};
    ADist* ad{static_cast<ADist*>(args[0].addr)};
    string host{static_cast<CcString*>(args[1].addr)->GetValue()};
    int port{static_cast<CcInt*>(args[2].addr)->GetValue()};
    ad->removePeer(host, port);
    qp->SetModified(qp->GetSon(s, 0));
    res->Set(true, true);
    return 0;
  }
/*
4.6 ~addworker~ Value Mapping

*/
  int addworkerVM(Word* args, Word& result, int, Word&, Supplier s) {
    CcBool* res{&qp->template ResultStorage<CcBool>(result, s)};
    ADist* ad{static_cast<ADist*>(args[0].addr)};
    ManagedMutex dplock{ad->dpartitionName(), true};
    ManagedMutex dalock{ad->darrayName(), true};
    string host{static_cast<CcString*>(args[1].addr)->GetValue()};
    int port{static_cast<CcInt*>(args[2].addr)->GetValue()};
    string conf{static_cast<CcString*>(args[3].addr)->GetValue()};
    ad->addWorker(host, port, conf);
    res->Set(true, true);
    return 0;
  }
/*
4.7 ~removeworker~ Value Mapping (overloaded)

4.7.1 "removeworkerVM\_Index"[1]

*/
  int removeworkerVM_Index(Word* args, Word& result, int, Word&, Supplier s) {
    CcBool* res{&qp->template ResultStorage<CcBool>(result, s)};
    ADist* ad{static_cast<ADist*>(args[0].addr)};
    ManagedMutex dplock{ad->dpartitionName(), true};
    ManagedMutex dalock{ad->darrayName(), true};
    int index{static_cast<CcInt*>(args[1].addr)->GetValue()};
    if(index < 0)
      throw runtime_error{"A negative index was given for the worker."};
    ad->removeWorker(index);
    res->Set(true, true);
    return 0;
  }
/*
4.7.2 "removeworkerVM\_HostPort"[1]

*/
  int removeworkerVM_HostPort(Word* args, Word& result, int, Word&, Supplier s)
  {
    CcBool* res{&qp->template ResultStorage<CcBool>(result, s)};
    ADist* ad{static_cast<ADist*>(args[0].addr)};
    ManagedMutex dplock{ad->dpartitionName(), true};
    ManagedMutex dalock{ad->darrayName(), true};
    string host{static_cast<CcString*>(args[1].addr)->GetValue()};
    int port{static_cast<CcInt*>(args[2].addr)->GetValue()};
    ad->removeWorker(host, port);
    res->Set(true, true);
    return 0;
  }
/*
4.8 ~moveslot~ Value Mapping (overloaded)

4.8.1 "moveslotVM\_Index"[1]

*/
  int moveslotVM_Index(Word* args, Word& result, int, Word&, Supplier s) {
    CcBool* res{&qp->template ResultStorage<CcBool>(result, s)};
    ADist* ad{static_cast<ADist*>(args[0].addr)};
    ManagedMutex dplock{ad->dpartitionName(), true};
    ManagedMutex dalock{ad->darrayName(), true};
    int arg{static_cast<CcInt*>(args[1].addr)->GetValue()};
    if(arg < 0)
      throw runtime_error{"A negative number was given for the slot to be "
        "moved."};
    uint32_t slot{static_cast<uint32_t>(arg)};
    arg = static_cast<CcInt*>(args[2].addr)->GetValue();
    if(arg < 0)
      throw runtime_error{"A negative index was given for the worker."};
    uint32_t index{static_cast<uint32_t>(arg)};
    ad->moveSlot(slot, index);
    res->Set(true, true);
    return 0;
  }
/*
4.8.2 "moveslotVM\_HostPort"[1]

*/
  int moveslotVM_HostPort(Word* args, Word& result, int, Word&, Supplier s) {
    CcBool* res{&qp->template ResultStorage<CcBool>(result, s)};
    ADist* ad{static_cast<ADist*>(args[0].addr)};
    ManagedMutex dplock{ad->dpartitionName(), true};
    ManagedMutex dalock{ad->darrayName(), true};
    int arg{static_cast<CcInt*>(args[1].addr)->GetValue()};
    if(arg < 0)
      throw runtime_error{"A negative number was given for the slot to be "
        "moved."};
    uint32_t slot{static_cast<uint32_t>(arg)};
    string host{static_cast<CcString*>(args[2].addr)->GetValue()};
    int port{static_cast<CcInt*>(args[3].addr)->GetValue()};
    ad->moveSlot(slot, host, port);
    res->Set(true, true);
    return 0;
  }
/*
4.9 ~splitslot~ Value Mapping

*/
  int splitslotVM(Word* args, Word& result, int, Word&, Supplier s) {
    CcInt* res{&qp->template ResultStorage<CcInt>(result, s)};
    ADist* ad{static_cast<ADist*>(args[0].addr)};
    ManagedMutex dplock{ad->dpartitionName(), true};
    ManagedMutex dalock{ad->darrayName(), true};
    int arg{static_cast<CcInt*>(args[1].addr)->GetValue()};
    if(arg < 0)
      throw runtime_error{"A negative number was given for the slot to be "
        "split."};
    uint32_t slot{static_cast<uint32_t>(arg)};
    res->Set(ad->splitSlot(slot));
    return 0;
  }
/*
4.10 ~mergeslots~ Value Mapping

*/
  int mergeslotsVM(Word* args, Word& result, int, Word&, Supplier s) {
    CcInt* res{&qp->template ResultStorage<CcInt>(result, s)};
    ADist* ad{static_cast<ADist*>(args[0].addr)};
    ManagedMutex dplock{ad->dpartitionName(), true};
    ManagedMutex dalock{ad->darrayName(), true};
    int arg{static_cast<CcInt*>(args[1].addr)->GetValue()};
    if(arg < 0)
      throw runtime_error{"A negative number (" + to_string(arg) + ") was "
        "given for the first slot to be merged."};
    uint32_t slot1{static_cast<uint32_t>(arg)};
    arg = static_cast<CcInt*>(args[2].addr)->GetValue();
    if(arg < 0)
      throw runtime_error{"A negative number (" + to_string(arg) + ") was "
        "given for the second slot to be merged."};
    uint32_t slot2{static_cast<uint32_t>(arg)};
    res->Set(ad->mergeSlots(slot1, slot2));
    return 0;
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
5.4 ~addpeer~ Description

*/
  addpeerInfo::addpeerInfo() {
    name = "addpeer";
    signature = ADist::BasicType() + " x " + CcString::BasicType() + " x " +
      CcInt::BasicType() + " x " + CcString::BasicType() + " -> " +
      CcBool::BasicType();
    syntax = "addpeer(ad, host, port, conffile)";
    meaning = "Adds a peer given by \"host\", \"port\", and \"conffile\" to "
      "\"ad\". The dpartition and d[f]array referenced from \"ad\" are "
      "synchronized to the new peer.";
    example = "query addpeer(ad, \"snew\", 1234, \"cfg.ini\")";
  }
/*
5.5 ~removepeer~ Description

*/
  removepeerInfo::removepeerInfo() {
    name = "removepeer";
    signature = ADist::BasicType() + " x {" + CcInt::BasicType() + ", " +
      CcString::BasicType() + " x " + CcInt::BasicType() + "} -> " +
      CcBool::BasicType();
    syntax = "removepeer(ad, peer)";
    meaning = "Removes the peer specified either by index or by hostname and "
      "port from \"ad\".";
    example = "query removepeer(ad, 2)";
  }
/*
5.6 ~addworker~ Description

*/
  addworkerInfo::addworkerInfo() {
    name = "addworker";
    signature = ADist::BasicType() + " x " + CcString::BasicType() + " x " +
      CcInt::BasicType() + " x " + CcString::BasicType() + " x " + " -> " +
      CcBool::BasicType();
    syntax = "addworker(ad, host, port, conffile)";
    meaning = "Adds a worker given by \"host\", \"port\", and \"conffile\" to "
      "the d[f]array referenced from \"ad\". No slots are mapped to the new "
      "worker at this point.";
    example = "query addworker(ad, \"snew\", 1234, \"cfg.ini\")";
  }
/*
5.7 ~removeworker~ Description

*/
  removeworkerInfo::removeworkerInfo() {
    name = "removeworker";
    signature = ADist::BasicType() + " x {" + CcInt::BasicType() + ", " +
      CcString::BasicType() + " x " + CcInt::BasicType() + "} -> " +
      CcBool::BasicType();
    syntax = "removeworker(ad, worker)";
    meaning = "Removes the worker specified either by index or by hostname "
      "and port from the d[f]array referenced from \"ad\". The worker may not "
      "have any slots mapped to it.";
    example = "query removeworker(ad, 2)";
  }
/*
5.8 ~moveslot~ Description

*/
  moveslotInfo::moveslotInfo() {
    name = "moveslot";
    signature = ADist::BasicType() + " x " + CcInt::BasicType() + " x " +
      CcInt::BasicType() + " x " + " -> " + CcInt::BasicType();
    syntax = "moveslot(ad, slot, worker)";
    meaning = "Moves the contents of slot number \"slot\" to the worker at "
      "index \"worker\" in the d[f]array referenced from \"ad\". The result "
      "is that slot number \"slot\" will be empty and taken out of use and "
      "the contents will be in the slot given by the return value.";
    example = "query moveslot(ad, 7, 2)";
  }
/*
5.9 ~splitslot~ Description

*/
  splitslotInfo::splitslotInfo() {
    name = "splitslot";
    signature = ADist::BasicType() + " x " + CcInt::BasicType() + " -> " +
      CcInt::BasicType();
    syntax = "splitslot(ad, slot)";
    meaning = "Splits the slot so that half of the containing elements remain "
      "and the other half are moved to a new slot on the same worker. The "
      "number of the new slot is returned.";
    example = "query splitslot(ad, 7)";
  }
/*
5.10 ~mergeslots~ Description

*/
  mergeslotsInfo::mergeslotsInfo() {
    name = "mergeslots";
    signature = ADist::BasicType() + " x " + CcInt::BasicType() + " x " +
      CcInt::BasicType() + " -> " + CcInt::BasicType();
    syntax = "mergeslots(ad, slot1, slot2)";
    meaning = "Merges \"slot1\" and \"slot2\" together so that the data of "
      "both slots ends up in one of them. The slots must have contiguous "
      "values mapped to them in the dpartition referenced from \"ad\" and "
      "must reside on the same worker. The slot already holding most of the "
      "data is chosen as the final slot, the other slot is freed. The number "
      "of the slot ending up with all the data is returned.";
    example = "query mergeslots(ad, 7, 2)";
  }
}
