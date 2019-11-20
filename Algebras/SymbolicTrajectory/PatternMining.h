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

#include "Algorithms.h"

namespace stj {



struct AggEntry {
  AggEntry();
  AggEntry(const TupleId id, const temporalalgebra::SecInterval& iv);

  unsigned int getNoOccurrences(const TupleId& id) const;
  datetime::DateTime getDuration() const {return duration;}
  void computeAggregation() const;
  std::string print(const TupleId& id = 0) const;
  
  
  std::map<TupleId, temporalalgebra::Periods*> occurrences;
  unsigned int noOccurrences;
  datetime::DateTime duration;
};

/*

The original mlabel objects are transformed into a map from a label onto a set
of tuple ids (one for every tuple containing the label) together with a time
period (the time period in which the label occurs for this tuple), the number
of occurrences inside the tuple, and the total duration of its occurrences.

*/

struct RelAgg {
  RelAgg() {}
  
  void clear() {contents.clear();}
  void insert(const std::string& label, const TupleId& id, 
              const temporalalgebra::SecInterval& iv);
  void compute(Relation *rel, const NewPair<int, int> indexes);
  std::string print(const std::string& label = "");
  
  
  std::map<std::string, AggEntry> contents;
};

struct GetPatternsLI {
  GetPatternsLI(Relation *r, const NewPair<int, int> i, double ms, int ma);
  
  Tuple *getNextResult();
  
  
  Relation *rel;
  NewPair<int, int> indexes; // first: textual, second: spatial
  int minSupp;
  int minNoAtoms;
  RelAgg agg;
};
  
  
}
