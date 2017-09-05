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

using std::map;
using std::ostream;
using std::string;
using std::vector;
using distributed2::DArrayElement;

namespace distributed4 {
  DStruct::DStruct() {}

  DStruct::DStruct(const DStruct& src): partitioning{src.partitioning},
    allocation{src.allocation}, workers{src.workers},
    slotbasename{src.slotbasename} {}

  DStruct::DStruct(const map<double,size_t>& p, const vector<size_t>& a, const
      vector<DArrayElement>& w, const string& n): partitioning{p},
    allocation{a}, workers{w}, slotbasename{n} {}

  DStruct::DStruct(const map<double,size_t>& p, const distributed2::DArray&
      da): partitioning{p}, workers{da.getWorkers()},
    slotbasename{da.getName()} {
      //allocation{da.getMap()};  //TODO
    }

  DStruct::DStruct(ListExpr instance) {
    NList l{instance};
    NList pl{l.first()};
    NList al{l.second()};
    NList wl{l.third()};
    NList e;

/*
Make sure the nested list has the expected length to rule out certain
structural errors.

*/
    if(l.length() != 4)
      throw std::runtime_error("The nested list length does not fit.");
/*
Read the partitioning map from the nested list.

*/
    for(size_t i{0}; i < pl.length(); ++i) {
      e = pl.elem(i);
      partitioning.emplace(e.first().realval(), e.second().intval());
    }
/*
Read the allocation vector from the nested list.

*/
    for(size_t i{0}; i < al.length(); ++i)
      allocation.push_back(al.elem(i).intval());
/*
Read the workers vector from the nested list.

*/
    for(size_t i{0}; i < wl.length(); ++i) {
      e = wl.elem(i);
      workers.emplace_back(e.first().str(), e.second().intval(),
          e.third().intval(), e.fourth().str());
    }
/*
Read the slotbasename string from the nested list.

*/
    slotbasename = l.fourth().str();
  }

  ListExpr DStruct::listExpr() {
    NList pl, al, wl;
    for(auto it{partitioning.begin()}; it != partitioning.end(); ++it)
      pl.append(NList(NList(it->first), NList(it->second)));
    for(auto it{allocation.begin()}; it != allocation.end(); ++it)
      al.append(*it);
    for(auto it{workers.begin()}; it != workers.end(); ++it)
      wl.append(it->toListExpr());
    return NList(pl, al, wl, slotbasename).listExpr();
  }

  void DStruct::addWorker(const string& host, int port, const string& conf) {
    for(DArrayElement w: workers)
      if(w.getHost() == host && w.getPort() == port)
        throw std::runtime_error("Worker already exists.");
    auto num{workers.back().getNum() + 1};
    workers.emplace_back(host, port, num, conf);
  }

  void DStruct::removeWorker() {}

  size_t DStruct::slot(double key) const {
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

  distributed2::DArray DStruct::getDArray() const {
    return distributed2::DArray(0);
    //return distributed2::DArray(allocation, slotbasename, workers);  //TODO
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

  bool DStruct::checkType(ListExpr type, ListExpr&) {
    NList t{type};
    cout << "debug: " << t << endl;
    if(t.length() != 2 || t.first().str() != DStruct::BasicType())
      return false;
    if(t.second().isList()) {
      return false;
      //TODO: check for attr,rel(tuple) and that attr is in rel(tuple)
    }
    return true;
  }

  ostream& operator<<(ostream& os, const map<double,size_t>& m) {
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
