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

[10] Implementation of Class DPartition

2017-08-14: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 Preliminary Setup

*/
#include "DPartition.h"
#include "ConnectionSession.h"
#include "../Distributed2/Distributed2Algebra.h"
#include <unordered_set>
#include <memory>

namespace distributed4 {
  using distributed2::DArray;
  using distributed2::DArrayBase;
  using distributed2::DFArray;
  using distributed2::Distributed2Algebra;
  using std::isnan;
  using std::map;
  using std::numeric_limits;
  using std::ostream;
  using std::out_of_range;
  using std::pair;
  using std::runtime_error;
  using std::set;
  using std::string;
  using std::to_string;
  using std::unique_ptr;
  using std::unordered_set;
/*
2 Helper Functions

2.1 "partition2nlist"[1]

*/
  NList partition2nlist(const pair<double,uint32_t>& p) {
    if(p.first != -numeric_limits<double>::infinity())
      return NList{NList{p.first}, NList{static_cast<int>(p.second)}};
    else
      return NList{static_cast<int>(p.second)}.enclose();
  }
/*
2.2 "nlist2partition"[1]

*/
  pair<double,uint32_t> nlist2partition(const NList& l) {
    double v;
    int s;
    switch(l.length()) {
      case 1:
        if(!l.first().isInt())
          throw runtime_error{"The lone value needs to be an integer."};
        v = -numeric_limits<double>::infinity();
        s = l.first().intval();
        break;
      case 2:
        if(!l.first().isReal() && !l.first().isInt())
          throw runtime_error{"The first value needs to be a real or an "
            "integer."};
        if(!l.second().isInt())
          throw runtime_error{"The second value needs to be an integer."};
        v = l.first().isReal() ? l.first().realval() : l.first().intval();
        s = l.second().intval();
        break;
      default:
        throw runtime_error{"The element needs to be a value-to-slot "
          "partitioning pair. If a partition is to take values beginning from "
            "negative infinity, the pair is reduced to just the slot number."};
    }
    if(s < 0)
      throw runtime_error{"The slot number needs to be non-negative."};
    return {v, static_cast<uint32_t>(s)};
  }
/*
3 Constructors

3.1 "DPartition()"[1] (for "Create"[1])

The default constructor creates an instance of "DPartition"[1] with no link to
a ~d[f]array~ object. That is an invalid state. This constructor is provided
for use by the "Create"[1] function that the query processor uses when
instantiating a query tree. It is protected in the class declaration to avoid
accidental misuse.

*/
  DPartition::DPartition() {}
/*
3.2 "DPartition(map, string)"[1]

The passed "map"[1] and "string"[1] are copied to the respective member
variables of the newly constructed "DPartition"[1].

*/
  DPartition::DPartition(const map<double,uint32_t>& p, const string& n):
    partitioning{p}, darrayname{n} {}
/*
3.3 "DPartition(NList, NList)"[1]

The value of the database object is passed in "list"[1]. The subtype, which
must match the referenced ~d[f]array~, is passed in "subtype"[1].

*/
  DPartition::DPartition(const NList& list, const NList& subtype) {
/*
Validate the top-level structure of the nested list.

*/
    if(list.length() < 2 || list.length() > 3)
      throw runtime_error{"The dpartition value needs to be a list of two or "
        "three items: a list of value-to-slot partitioning pairs, a symbol "
          "atom with the referenced d[f]array, and optionally a value-to-slot "
          "buffer partitioning pair."};
/*
Validate and read the partitioning map from the nested list.

*/
    NList l{list.first()};
    if(l.isAtom())
      throw runtime_error{"The first element, \"" + l.convertToString() +
        "\", needs to be a list of value-to-slot partitioning pairs."};
    for(Cardinal i{1}; i <= l.length(); ++i) {
      try {
        partitioning.insert(nlist2partition(l.elem(i)));
      } catch(const runtime_error& e) {
        throw runtime_error{"A problem was determined in the first sublist "
          "with element " + to_string(i) + ", \"" + l.elem(i).convertToString()
            + "\": " + e.what()};
      }
    }
/*
Validate and read the name of the referenced ~d[f]array~ from the nested list.

*/
    l = list.second();
    if(!l.isSymbol())
      throw runtime_error{"The second element, \"" + l.convertToString() +
        "\", needs to be a symbol atom."};
    darrayname = l.str();
/*
Make sure "darrayname"[1] really refers to a ~d[f]array~.

*/
    SecondoCatalog* ctlg{SecondoSystem::GetCatalog()};
    NList darraytype{ctlg->GetObjectTypeExpr(darrayname)};
    if(!DArray::checkType(darraytype.listExpr()) &&
        !DFArray::checkType(darraytype.listExpr()))
      throw runtime_error{"The named database object, \"" + darrayname +
        "\", is not a d[f]array."};
/*
Validate "darraytype"[1] if possible ("subtype"[1] is not empty). If
"subtype"[1] is an attribute name and type, make sure the ~d[f]array~ is over a
"rel(tuple)"[1] and that it has a matching attribute. If, alternately, the
subtype represents a simple type, make sure the ~d[f]array~ is over a container
containing that type.

*/
    if(!subtype.isEmpty()) {
      NList dasubtype{darraytype.second()};
      if(subtype.isList()) {
        if(!Relation::checkType(dasubtype.listExpr()))
          throw runtime_error{"The subtype of the referenced d[f]array needs "
            "to be rel(tuple))."};
        NList attrlist{dasubtype.second().second()};
        Cardinal i{1};
        while(i <= attrlist.length() && subtype != attrlist.elem(i))
          ++i;
        if(i > attrlist.length())
          throw runtime_error{"The attribute list of the d[f]array's "
            "rel(tuple) needs to contain " + subtype.convertToString() + "."};
      } else {
        if(dasubtype.length() != 2 || !dasubtype.second().isSymbol())
          throw runtime_error{"The subtype of the referenced d[f]array needs "
            "to be a collection of a simple type."};
        if(dasubtype.second() != subtype)
          throw runtime_error{"The type of the d[f]array's collection needs "
            "to be " + subtype.convertToString() + "."};
      }
    }
/*
Check for and read the optional partitioning pair if present.

*/
    if(list.length() != 3)
      return;
    try {
      bufpartition = nlist2partition(list.third());
    } catch(const runtime_error& e) {
      throw runtime_error{"A problem was determined with the third sublist, \""
        + list.third().convertToString() + "\": " + e.what()};
    }
  }
/*
4 Member Functions

4.1 "slot"[1]

The "partitioning"[1] map is applied to the passed key to determine to which
slot that key corresponds.

*/
  uint32_t DPartition::slot(double key) const {
/*
"upper\_bound"[1] determines the next slot after the one being looked for, as
it looks for a key in the map that is greater than the key passed.

*/
    auto it{partitioning.upper_bound(key)};
/*
If the slot found happens to be the slot at the beginning of the map, that
means that the value passed in key is below all keys in the map.  That is all
slots whose left borders are defined. That means that the passed key is
somewhere between negative infinity and the first defined slot border. That is
always slot 0. In all other cases, the slot for the value passed in key is the
slot one before the slot found.

*/
    if(it == partitioning.begin()) {
      return 0;
    } else {
      return (--it)->second;
    }
  }
/*
4.2 "listExpr"[1]

The "DPartition" instance is converted into its nested list representation.

*/
  ListExpr DPartition::listExpr() const {
    NList l;
    for(auto it{partitioning.begin()}; it != partitioning.end(); ++it)
      l.append(partition2nlist(*it));
    l.enclose().append(darrayname);
    if(!isnan(bufpartition.first))
      l.append(partition2nlist(bufpartition));
    return l.listExpr();
  }
/*
4.3 "print"[1]

For easier debugging and/or output of the contents of a DPartition, push all
details to a passed ostream. This makes it usable by "operator<<"[1], which
defined below.

*/
  void DPartition::print(ostream& os) const {
    os << "DPartition " << partitioning << " with " << darrayname;
  }
/*
4.4 "getPartition"[1]

Retrieve the beginning of the value range mapped to "slot"[1].

*/
  double DPartition::getPartition(uint32_t slot) const {
    for(auto p: partitioning)
      if(p.second == slot)
        return p.first;
    throw runtime_error{"Slot " + to_string(slot) + " has no associated "
      "partitioning value."};
  }
/*
4.5 "getDArrayName"[1]

*/
  string DPartition::getDArrayName() const {
    return darrayname;
  }
/*
4.6 "getDArray"[1]

Retrieve the ~d[f]array~ named in "darrayname"[1]. The caller is responsible
for deleting the instantiated object.

*/
  DArrayBase* DPartition::getDArray() const {
    Word ref;
    bool isdefined;
    if(!SecondoSystem::GetCatalog()->GetObject(darrayname, ref, isdefined))
      throw runtime_error{"The d[f]array " + darrayname +
          " couldn't be opened."};
    if(!isdefined)
      throw runtime_error{"The d[f]array " + darrayname + " is undefined."};
    return static_cast<DArrayBase*>(ref.addr);
  }
/*
4.7 "allocateSlot"[1]

There are several different occasions when a new or recycled slot in a
~d[f]array~ is needed. The most direct examples are the operators ~moveslot~
and ~splitslot~. It is quite possible that slots exist in the ~d[f]array~ that
aren't being used. This can happen as the result of ~moveslot~ and ~mergeslots~
when the unused slot is numerically in between other slots in the ~d[f]array~.

This function won't directly modify the referenced ~d[f]array~ database object.
Instead, it will modify an in-memory instance of that object. A pointer to this
instance is passed as "da"[1].

The procedure for allocating a new slot mirrors the complexity mentioned above:

*/
  uint32_t DPartition::allocateSlot(uint32_t worker, DArrayBase* da) {
/*
  * Find the lowest slot number not currently in use ("slot"[1]). That is the
    case when "partitioning"[1] doesn't contain a value-to-slot partitioning
    for a slot. This may come up with a slot number that already exists
    (recycled slot), or with one past the highest existing slot number (new
    slot).

*/
    set<uint32_t> inuse;
    for(auto it{partitioning.begin()}; it != partitioning.end(); ++it)
      inuse.insert(it->second);
    uint32_t slot{0};
    for(auto n: inuse) {
      if(n != slot)
        break;
      ++slot;
    }
/*
  * Retrieve several values needed for further processing: "slotname"[1] is the
    name of the database object holding the slot on the responsible worker.
    "exists"[1] tells whether the slot is new ("exists == false"[1]) or
    recycled ("exists == true"[1]).  "dbname"[1] holds the name of the
    currently open database.  "d2"[1] is a pointer to the
    "Distributed2Algebra"[1] object.

*/
    string slotname{da->getObjectNameForSlot(slot)};
    bool exists{slot < da->getSize()};
    const string dbname{SecondoSystem::GetInstance()->GetDatabaseName()};
    Distributed2Algebra* d2{static_cast<Distributed2Algebra*>(SecondoSystem::
        GetAlgebraManager()->getAlgebra("Distributed2Algebra"))};
/*
  * If "slot"[1] already exists in the ~d[f]array~ named in "darrayname"[1],
    make sure it is empty and then remove it. If the slot still does contain
    data, there appears to be some inconsistency.  In that case, the procedure
    is aborted with an exception.

*/
    if(exists) {
      ConnectionSession s{d2->getWorkerConnection(da->getWorkerForSlot(slot),
          dbname)};
      NList result{s.run("query " + slotname + " count")};
      if(!(result.length() == 2 && result.first().isSymbol() &&
            result.first().str() == CcInt::BasicType() &&
            result.second().isInt()))
        throw runtime_error{"Received an invalid record count for slot " +
          to_string(slot) + ": \"" + result.convertToString() + "\"."};
      int count{result.second().intval()};
      if(count > 0)
        throw runtime_error{"The slot " + to_string(slot) + " contains " +
          to_string(count) + " records. It should be empty. The procedure "
            "cannot continue."};
      s.run("delete " + slotname);
    }
/*
  * Create "slot"[1] on "worker"[1].

*/
    NList rel_type{NList{SecondoSystem::GetCatalog()->GetObjectTypeExpr(
        darrayname)}.second()};
    ConnectionSession s{d2->getWorkerConnection(da->getWorker(worker),
        dbname)};
    s.createEmpty(slotname, rel_type);
/*
  * Assign "slot"[1] to "worker"[1] in the ~d[f]array~ named in
    "darrayname"[1], unless it was already so assigned.

*/
    if(!exists) {
      auto map{da->getMap()};
      map.push_back(worker);
      da->set(map, da->getName(), da->getWorkers());
    } else if(worker != da->getWorkerIndexForSlot(slot)) {
      da->setResponsible(slot, worker);
    }
/*
Return the slot number that was allocated.

*/
    s.clearRollback();
    return slot;
  }
/*
4.8 "setPartition"[1]

Set the target "slot"[1] of the existing partition beginning at "start"[1]. The
former target slot is discarded.

*/
  void DPartition::setPartition(double start, uint32_t slot) {
    try {
      partitioning.at(start) = slot;
    } catch(const out_of_range&) {
      throw runtime_error{"No partition exists here that begins at " +
        to_string(start) + "."};
    }
  }
/*
4.9 "setBufferPartition"[1]

Set the so-called buffer partition to map "start"[1] to "slot"[1]. The buffer
partition is a side storage area designed for use while the DPartition is being
restructured. It may only be used by one process at a time. This member
function assumes that the calling process holds an exclusive lock on the
dpartition to avoid race conditions.

*/
  void DPartition::setBufferPartition(double start, uint32_t slot) {
    if(!isnan(bufpartition.first))
      throw runtime_error{"The buffer partition is already in use. It appears "
        "that this dpartition is already being restructured with respect to "
          "the partition (" + to_string(bufpartition.first) + " -> " +
          to_string(bufpartition.second) + ")."};
    bufpartition = {start, slot};
  }
/*
4.10 "clearBufferPartition"[1]

Clear the so-called buffer partition. This operation is unprotected and will
succeed no matter whether the buffer partition was previously in use or not.

*/
  void DPartition::clearBufferPartition() {
    bufpartition = {numeric_limits<double>::quiet_NaN(), 0};
  }
/*
5 Static Member Functions

5.1 "BasicType"[1]

"BasicType"[1] is required by [secondo]. It returns [secondo]'s basic type for
this class.

*/
  string DPartition::BasicType() {
    return "dpartition";
  }
/*
5.2 "Out"[1]

The Out function takes a nested list type expression and a "Word"[1] containing
a pointer to an instance of that type as arguments. It returns the value
represented by the instance as a nested list value expression. This is done in
the form of a "ListExpr"[1]. The passed type expression is commonly ignored.

*/
  ListExpr DPartition::Out(ListExpr, Word value) {
    return static_cast<DPartition*>(value.addr)->listExpr();
  }
/*
5.3 "In"[1]

The In function takes a nested list type expression, a nested list value
expression, and an integer position. It passes back a nested list error
expression, and a boolean correctness value. It returns a "Word"[1] containing
a pointer to an object instantiated based on the passed value expression.

*/
  Word DPartition::In(ListExpr typeInfo, ListExpr instance, int, ListExpr&,
      bool& correct) {
    NList ti{typeInfo}, inst{instance};
    correct = false;
/*
"typeInfo"[1] is a bit enigmatic. When the object is created from a "[const ...
value ...]"[1] expression or with a "restore from ..."[1] command, it contains
both the basic type and any subtypes. This information is checked by checkType
before being processed here. When the object is opened from the database,
however, there will be no subtype, just the basic type. In all cases, both the
basic type and any subtypes are passed in a modified form: all mentioned type
symbols ("dpartition"[1], "int"[1], "real"[1], etc.) are converted to pairs of
algebra and type ID.

Thus, the following cases need to be considered: "typeInfo"[1] is a pair of ...

  * ... "int"[1]s. Only the basic type is given. The object is being
    opened from the database. The subtype can't be checked as it isn't passed.

  * ... lists. The basic type and the subtype are given. The subtype is
    converted back to its original form and used when instantiating the object
    so that the constructor can check the type of the referenced ~d[f]array~.

*/
    NList subtype;
    if(ti.second().isList()) {
      SecondoCatalog* ctlg{SecondoSystem::GetCatalog()};
      if(ti.second().second().isList()) {
        subtype = NList{ti.second().first(),
          ctlg->GetTypeName(ti.second().second().first().intval(),
              ti.second().second().second().intval())};
      } else {
        subtype = NList{ctlg->GetTypeName(ti.second().first().intval(),
            ti.second().second().intval())};
      }
    }
/*
Create the object in memory.

*/
    DPartition* addr{new DPartition{inst, subtype}};
    correct = true;
    return Word{addr};
  }
/*
5.4 "checkType"[1]

"checkType"[1] is required by [secondo] as well. It checks the passed ListExpr
to make sure that it is a valid type description for this class. It doesn't
only check the basic type (~dpartition~) but the complete type expression.

*/
  bool DPartition::checkType(ListExpr type) {
    NList t{type};
    const unordered_set<string> supported{CcInt::BasicType(),
      CcReal::BasicType(), CcString::BasicType(), FText::BasicType()};
    string partitioning_type;
/*
A dpartition type contains two elements: the basic type and a subtype.

*/
    if(t.length() != 2 || !t.first().isSymbol() || t.first().str() !=
        DPartition::BasicType())
      throw runtime_error{"The type expression needs to be a nested list "
        "beginning with the symbol " + DPartition::BasicType() + " followed "
          "by a symbol or a pair of symbols."};
/*
A ~dpartition~ for a ~d[f]array~ over "rel(tuple)"[1] specifies its subtype as
a pair indicating the attribute of the relation that is used in partitioning
and its type, i.e. "(dpartition (Code int))"[1]. A dpartition for a ~d[f]array~
over a container of simple types specifies its subtype as the type that is
contained in the ~d[f]array~, i.e. "(dpartition int)"[1].

*/
    NList subtype{t.second()};
    if(subtype.isList()) {
      if(subtype.length() != 2 || !subtype.first().isSymbol() ||
          !subtype.second().isSymbol())
        throw runtime_error{"The type expression looks like a " +
          DPartition::BasicType() + " for a d[f]array over a " +
            Relation::BasicType() + "(" + Tuple::BasicType() + "). In this "
            "case, the type expression needs to have an attribute of that " +
            Relation::BasicType() + "(" + Tuple::BasicType() + ") as its "
            "second element."};
      partitioning_type = subtype.second().str();
    } else {
      if(!subtype.isSymbol())
        throw runtime_error{"The type expression looks like a " +
          DPartition::BasicType() + " for a d[f]array over a container of "
            "simple types. In this case, the type expression needs to have "
            "that type as its second element."};
      partitioning_type = subtype.str();
    }
/*
The type or attribute that determines partitioning must be supported by
DPartition.

*/
    if(!supported.count(partitioning_type))
      throw runtime_error{"The type " + partitioning_type + " is not "
        "supported by " + DPartition::BasicType() + "."};
    return true;
  }
/*
5.5 "checkType"[1] (wrapper)

Call the above "checkType"[1] function. It is used by the type constructor and
takes a reference that is superfluous in this case.

*/
  bool DPartition::checkType(ListExpr type, ListExpr&) {
    return DPartition::checkType(type);
  }
/*
6 Member Structs

6.1 "Info"[1]

This struct inherits from "ConstructorInfo"[1] and is responsible for
formatting information about the type constructor for the database user.

*/
  DPartition::Info::Info() {
    name = BasicType();
    signature = "-> SIMPLE";
    typeExample = "(dpartition int)";
    listRep = "(<partitioning map> <d[f]arrayname> [<partitioning pair>])";
    valueExample = "(((0) (-0.3 2) (0.1 1)) da)";
    remarks = "This type provides partitioning of values into slots found in "
      "a matching d[f]array. This makes it possible to use value-oriented "
      "database operations on distributed data (insert, update, delete, "
      "etc.).";
  }
/*
6.2 "Functions"[1]

This struct inherits from "ConstructorFunctions"[1] and defines the functions
needed by a type constructor.

*/
  DPartition::Functions::Functions() {
    out = DPartition::Out;
    in = DPartition::In;
    open = save = 0;
    kindCheck = DPartition::checkType;
  }
/*
7 Stream Operators Used by DPartition

7.1 "operator<<"[1] (map)

This operator allows formatting the pairs in a map for a character stream.

*/
  ostream& operator<<(ostream& os, const map<double,uint32_t>& m) {
    os << "{";
    for(auto it = m.begin(); it != m.end(); ++it) {
      if(it != m.begin()) os << ", ";
      os << it->first << " -> " << it->second;
    }
    os << "}";
    return os;
  }
/*
7.2 "operator<<"[1] (DPartition)

This operator allows something like the following:

---- DPartition dp;
     cerr << "debug: " << dp;
----

*/
  ostream& operator<<(ostream& os, const DPartition& dp) {
    dp.print(os);
    return os;
  }
}
