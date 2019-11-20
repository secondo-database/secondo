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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

Started November 2019, Fabio Vald\'{e}s

*/

#include "PatternMining.h"

using namespace std;
using namespace datetime;
using namespace temporalalgebra;

namespace stj {

AggEntry::AggEntry() {
  occurrences.clear();
  duration.SetType(datetime::durationtype);
  duration.ReadFrom((int64_t)0);
}
  
AggEntry::AggEntry(const TupleId id, const temporalalgebra::SecInterval& iv) {
  occurrences.clear();
  Periods *per = new Periods(1);
  per->Add(iv);
  occurrences[id] = per;
  noOccurrences = 0;
  duration.SetType(datetime::durationtype);
  duration.ReadFrom((int64_t)0); // durations are computed at the end
}
  
unsigned int AggEntry::getNoOccurrences(const TupleId& id) const {
  auto it = occurrences.find(id);
  if (it == occurrences.end()) {
    return 0;
  }
  return it->second->GetNoComponents();
}

void AggEntry::computeAggregation() const {
  
  
}
  
std::string AggEntry::print(const TupleId& id /* = 0 */) const {
  std::stringstream result;
  if (id == 0) { // print everything
    for (auto it : occurrences) {
      result << "   TID " << it.first << ": " << *(it.second) << std::endl;
    }
  }
  else {
    auto it = occurrences.find(id);
    if (it == occurrences.end()) { // id not found
      result << "   TID " << id << " not found" << std::endl;
    }
    else {
      result << "   TID " << id << ": " << *(it->second) << std::endl;
    }
  }
  return result.str();
}

void RelAgg::insert(const std::string& label, const TupleId& id,
                    const temporalalgebra::SecInterval& iv) {
  cout << "insert (" << label << ", " << id << ", " << iv << ")" << endl;
  auto aggIt = contents.find(label);
  if (aggIt == contents.end()) { // new label
    AggEntry entry(id, iv);
    contents.insert(make_pair(label, entry));
  }
  else { // label already present
    auto entryIt = aggIt->second.occurrences.find(id);
    if (entryIt == aggIt->second.occurrences.end()) { // new id for label
      Periods *per = new Periods(1);
      per->Add(iv);
      contents[label].occurrences.insert(make_pair(id, per));
    }
    else { // id already present for label
      
    }
  }
}

void RelAgg::compute(Relation *rel, const NewPair<int, int> indexes) {
  string label;
  SecInterval iv;
  GenericRelationIterator* it = rel->MakeScan();
  Tuple *tuple = 0;
  while ((tuple = it->GetNextTuple())) {
    MLabel *ml = (MLabel*)(tuple->GetAttribute(indexes.first));
    for (int j = 0; j < ml->GetNoComponents(); j++) {
      ml->GetValue(j, label);
      ml->GetInterval(j, iv);
      insert(label, tuple->GetTupleId(), iv);
    }
    tuple->DeleteIfAllowed();
  }
}

std::string RelAgg::print(const std::string& label /* = "" */) {
  std::stringstream result;
  if (label == "") { // print everything
    for (auto it : contents) {
      result << "\"" << it.first << "\" :" << it.second.print() << endl;
    }
  }
  else {
    auto it = contents.find(label);
    if (it == contents.end()) { // label not found
      result << "Label \"" << label << "\" not found" << endl;
    }
    else {
      result << "\"" << label << "\" :" << it->second.print() << endl;
    }
  }
  return result.str();
}

GetPatternsLI::GetPatternsLI(Relation *r, const NewPair<int,int> i, double ms, 
                             int ma)
  : rel(r), indexes(i), minSupp(ms), minNoAtoms(ma) {
  agg.clear();
  agg.compute(rel, indexes);
  agg.print();
}


Tuple* GetPatternsLI::getNextResult() {
  return 0; // TODO: a lot
}


}
