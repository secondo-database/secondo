/*
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

*/
#include "DStruct.h"
#include "FTextAlgebra.h"

using std::map;
using std::ostream;
using std::runtime_error;
using std::string;
using std::to_string;
using std::vector;
using distributed2::DArray;
using distributed2::DArrayElement;

namespace distributed4 {
  DStruct::DStruct() {}

  DStruct::DStruct(const DStruct& src): partitioning{src.partitioning},
    allocation{src.allocation}, workers{src.workers},
    slotbasename{src.slotbasename} {}

  DStruct::DStruct(const map<double,uint32_t>& p, const vector<uint32_t>& a,
      const vector<DArrayElement>& w, const string& n): partitioning{p},
    allocation{a}, workers{w}, slotbasename{n} {}

  DStruct::DStruct(const map<double,uint32_t>& p, const DArray& da):
    partitioning{p}, allocation{da.getMap()}, workers{da.getWorkers()},
    slotbasename{da.getName()} {}

  DStruct::DStruct(ListExpr instance) {
    NList e;
/*
Validate the top-level structure of the nested list. The rest of the structure
will be validated further along.

*/
    NList l{instance};
    if(l.length() != 4)
      throw runtime_error("The passed value needs to be a nested list with 4 "
          "elements.");
/*
Validate and read the partitioning map from the nested list.

*/
    NList pl{l.first()};
    if(!pl.isList())
      throw runtime_error("The first element in the passed nested list needs "
          "to be a nested list containing pairs.");
    for(Cardinal i{1}; i <= pl.length(); ++i) {
      e = pl.elem(i);
      if(e.length() != 2 || !(e.first().isReal() || e.first().isInt()) ||
          !e.second().isInt() || e.second().intval() <= 0)
        // The second number may not be 0 (or negative) because slot 0 is
        // inferred when there is no match in the map.
        throw runtime_error("Element " + to_string(i) + " of the first "
            "sublist needs to be a list containing a pair of atoms. The first "
            "atom needs to be any real or integer value, and the second a "
            "positive integer.");
      partitioning.emplace(e.first().isReal() ? e.first().realval() :
          e.first().intval(), e.second().intval());
    }
/*
Validate and read the allocation vector from the nested list.

*/
    NList al{l.second()};
    if(!al.isList())
      throw runtime_error("The second element in the passed nested list needs "
          "to be a list of non-negative integer atoms.");
    for(Cardinal i{1}; i <= al.length(); ++i) {
      e = al.elem(i);
      if(!e.isInt() || e.intval() < 0)
        throw runtime_error("Element " + to_string(i) + " of the second "
            "sublist needs to be a non-negative integer atom.");
      allocation.push_back(e.intval());
    }
/*
Validate and read the workers vector from the nested list.

*/
    NList wl{l.third()};
    if(!wl.isList())
      throw runtime_error("The third element in the passed nested list needs "
          "to be a nested list of workers (host, port, cfgfile).");
    for(Cardinal i{1}; i <= wl.length(); ++i) {
      e = wl.elem(i);
      DArrayElement w{"", 0, 0, ""};
      if(!distributed2::InDArrayElement(e.listExpr(), w))
        throw runtime_error("Element " + to_string(i) + " of the third "
            "sublist needs to be a triple describing a worker (host, port, "
            "cfgfile).");
      w.setNum(i);
      workers.push_back(w);
    }
/*
Validate and read the slotbasename string from the nested list.

*/
    NList sl{l.fourth()};
    if(!(sl.isString() || sl.isText()))
      throw runtime_error("The fourth and last element in the passed nested "
          "list needs to be a string or text atom.");
    slotbasename = l.fourth().str();
  }

  ListExpr DStruct::listExpr() {
    NList pl, al, wl, sl;
    for(auto it{partitioning.begin()}; it != partitioning.end(); ++it)
      pl.append(NList(NList(it->first), NList(static_cast<int>(it->second))));
    for(auto it{allocation.begin()}; it != allocation.end(); ++it)
      al.append(static_cast<int>(*it));
    for(auto it{workers.begin()}; it != workers.end(); ++it)
      wl.append(it->toListExpr());
    sl = NList().stringAtom(slotbasename);
    return NList(pl, al, wl, sl).listExpr();
  }

  void DStruct::addWorker(const string& host, int port, const string& conf) {
    for(DArrayElement w: workers)
      if(w.getHost() == host && w.getPort() == port)
        throw runtime_error("Worker already exists.");
    auto num{workers.back().getNum() + 1};
    workers.emplace_back(host, port, num, conf);
  }

  void DStruct::removeWorker() {}

  uint32_t DStruct::slot(double key) const {
    // The upper_bound method determines the next slot after the one being
    // looked for, as it looks for a key in the map that is greater than the
    // key passed.
    auto it{partitioning.upper_bound(key)};

    // If the slot found happens to be the slot at the beginning of the map,
    // that means that the value passed in key is below all keys in the map.
    // That is all slots whose left borders are defined. That means that the
    // passed key is somewhere between negative infinity and the first defined
    // slot border. That is always slot 0. In all other cases, the slot for the
    // value passed in key is the slot one before the slot found.
    if(it == partitioning.begin()) {
      return 0;
    } else {
      return (--it)->second;
    }
  }

  DArray DStruct::getDArray() const {
    DArray da{DArray(allocation, slotbasename)};
    da.set(slotbasename, workers);
    return da;
  }

  void DStruct::print(ostream& os) const {
    os << "DStruct" << endl;
    os << "partitioning: " << partitioning << endl;
    os << "allocation: " << allocation << endl;
    os << "workers: " << workers << endl;
    os << "slotbasename: " << slotbasename << endl;
  }

  string DStruct::BasicType() {
    return "dstruct";
  }

  bool DStruct::checkType(ListExpr type, ListExpr& errorInfo) {
    NList t{type};
    const vector<string> supported{CcInt::BasicType(), CcReal::BasicType(),
      CcString::BasicType(), FText::BasicType()};
    string partitioning_type;
/*
Identity Check

*/
    if(t.length() < 1 || !t.first().isSymbol() || t.first().str() !=
        DStruct::BasicType()) {
      cmsg.typeError("The type expression needs to be a nested list beginning "
          "with the symbol " + DStruct::BasicType() + ".");
      return false;
    }
/*
A dstruct of a simple type contains two elements, i.e. (dstruct int). A dstruct
of a rel(tuple) contains three elements, i.e. (dstruct (Code int) (rel (tuple
((Osm\_id string) (Code int) (Fclass string))))). Anything else is invalid.

*/
    if(t.length() == 2) {
      if(!t.second().isSymbol()) {
        cmsg.typeError("The type expression looks like a " +
            DStruct::BasicType() + " of a simple type. In this case, the type "
            "expression needs to have a symbol as its second element.");
        return false;
      }
      partitioning_type = t.second().str();
    } else if(t.length() == 3) {
      NList attr{t.second()};
      if(attr.length() != 2 || !attr.first().isSymbol() ||
          !attr.second().isSymbol() || !am->CheckKind(Kind::REL(),
            t.third().listExpr(), errorInfo)) {
        cmsg.typeError("The type expression looks like a " +
            DStruct::BasicType() + " of a " + Relation::BasicType() + "(" +
            Tuple::BasicType() + "). In this case, the type expression needs "
            "to have an attribute of that " + Relation::BasicType() + "(" +
            Tuple::BasicType() + ") as its second element and the "
            "specification of the " + Relation::BasicType() + "(" +
            Tuple::BasicType() + ") as its third element.");
        return false;
      }
      partitioning_type = attr.second().str();
      NList attrlist{t.third().second().second()};
      Cardinal i{1};
      while(i <= t.third().length() && attr != attrlist.elem(i))
        ++i;
      if(i > t.third().length()) {
        cmsg.typeError("The second element of the type expression must be an "
          "attribute in the third.");
        return false;
      }
    } else {
      cmsg.typeError("The type expression must be a nested list of two or "
          "three elements, depending on whether the " + DStruct::BasicType() +
          " is of a simple type or of a " + Relation::BasicType() + "(" +
          Tuple::BasicType() + ").");
      return false;
    }
/*
The type or attribute that determines partitioning must be supported by
DStruct.

*/
    for(auto it{supported.begin()}; it != supported.end(); ++it)
      if(partitioning_type == *it)
        return true;
    cmsg.typeError("The type " + partitioning_type + " is not supported by " +
        DStruct::BasicType() + ".");
    return false;
  }

  ostream& operator<<(ostream& os, const map<double,uint32_t>& m) {
    os << "{";
    for(auto it = m.begin(); it != m.end(); ++it) {
      if(it != m.begin()) os << ", ";
      os << it->first << " -> " << it->second;
    }
    os << "}";
    return os;
  }

  template<typename T> ostream& operator<<(ostream& os, const vector<T>& v) {
    os << "[";
    for(auto it = v.begin(); it != v.end(); ++it) {
      if(it != v.begin()) os << ", ";
      os << *it;
    }
    os << "]";
    return os;
  }

  ostream& operator<<(ostream& os, const DStruct& ds) {
    ds.print(os);
    return os;
  }
}
