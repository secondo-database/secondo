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

  void clear();
  unsigned int getNoOccurrences(const TupleId& id) const;
  datetime::DateTime getDuration() const {return duration;}
  void computeCommonTimeInterval(temporalalgebra::SecInterval& iv) const;
  void computeSemanticTimeSpec(std::string& semanticTimeSpec) const;
  std::string print(const TupleId& id = 0) const;
  
  
  std::map<TupleId, temporalalgebra::Periods*> occurrences;
  unsigned int noOccurrences; // over all tuples
  datetime::DateTime duration; // over all tuples
};

/*

Comparison function; sort by

  * number of tuples with at least one occurrence (support * noTuples)
  * total duration
  * total number of occurrences (>= support * noTuples)
  * lexicographical order of labels

*/


struct compareEntries {
  bool operator()(std::pair<const std::string, AggEntry> const& left,
                  std::pair<const std::string, AggEntry> const& right) const {
      if (left.second.occurrences.size() != right.second.occurrences.size()) {
        return left.second.occurrences.size() > right.second.occurrences.size();
      }
      if (left.second.duration != right.second.duration) {
        return left.second.duration > right.second.duration;
      }
      if (left.second.noOccurrences != right.second.noOccurrences) {
        return left.second.noOccurrences > right.second.noOccurrences;
      }
      return left.first < right.first;
    }
};

/*

The original mlabel objects are transformed into a map from a label onto a set
of tuple ids (one for every tuple containing the label) together with a time
period (the time period in which the label occurs for this tuple), the number
of occurrences inside the tuple, and the total duration of its occurrences.

*/

struct RelAgg {
  RelAgg() {}
  
  void clear();
  void insert(const std::string& label, const TupleId& id, 
              const temporalalgebra::SecInterval& iv);
  void compute(Relation *rel, const NewPair<int, int> attrPos);
  void sort(const double ms);
  void buildAtom(std::pair<std::string, AggEntry>& sortedContentsEntry,
                 std::string& atom);
  void retrieveLabelCombs(const unsigned int size, 
                          std::vector<std::string>& source, 
                          std::set<std::vector<std::string > >& result);
  bool canIntersectionBeFrequent(std::vector<std::string>& labelSeq,
                                 std::set<TupleId>& intersection);
  double sequenceSupp(std::vector<std::string>& labelSeq,
                      std::set<TupleId>& intersection);
  void derivePatterns(const int ma, Relation *rel);
  std::string print(const std::vector<std::pair<std::string, AggEntry> >&
                                                          sortedContents) const;
  std::string print(const std::map<TupleId, std::vector<std::string> >& 
                                                          frequentLabels) const;
  std::string print(const std::set<std::vector<std::string> >& labelCombs) 
                                                                          const;
  std::string print(const std::set<TupleId>& tidSet) const;
  std::string print(const std::string& label = "") const;
  
  unsigned int noTuples;
  std::map<std::string, AggEntry> contents;
  std::vector<std::pair<std::string, AggEntry> > sortedContents;
  std::list<std::pair<std::string, double> > results;
  double minSupp;
  int minNoAtoms;
};

struct GetPatternsLI {
  GetPatternsLI(Relation *r, const NewPair<int, int> ap, double ms, int ma);
  ~GetPatternsLI();
  
  TupleType *getTupleType() const;
  Tuple *getNextResult();
  
  
  Relation *rel;
  TupleType *tupleType;
  NewPair<int, int> attrPos; // first: textual, second: spatial
  RelAgg agg;
};
  
  
}
