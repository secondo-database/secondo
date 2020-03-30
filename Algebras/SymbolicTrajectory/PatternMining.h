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
  AggEntry(const TupleId id, const temporalalgebra::SecInterval& iv,
           Rect& rect);

  void clear();
  unsigned int getNoOccurrences(const TupleId& id) const;
  datetime::DateTime getDuration() const {return duration;}
  void computeCommonTimeInterval(const std::set<TupleId>& commonTupleIds,
                                 temporalalgebra::SecInterval& iv);
  void computeCommonRect(const temporalalgebra::SecInterval& iv,
             const std::set<TupleId>& commonTupleIds, Geoid *geoid, Rect &rect);
  void computeSemanticTimeSpec(std::string& semanticTimeSpec) const;
  std::string print(const TupleId& id = 0) const;
  std::string print(const Rect& rect) const;
  
  std::map<TupleId, NewPair<temporalalgebra::Periods*, Rect> > occurrences;
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


// struct compareEntries {
//   bool operator()(NewPair<const std::string, AggEntry> const& l,
//                   NewPair<const std::string, AggEntry> const& r) const {
//       if (l.second.occurrences.size() != r.second.occurrences.size()) {
//         return l.second.occurrences.size() > r.second.occurrences.size();
//       }
//       if (l.second.duration != r.second.duration) {
//         return l.second.duration > r.second.duration;
//       }
//       if (left.second.noOccurrences != r.second.noOccurrences) {
//         return l.second.noOccurrences > r.second.noOccurrences;
//       }
//       return l.first < r.first;
//     }
// };

struct comparePatternMiningResults {
  bool operator()(NewPair<std::string, double> res1,
                  NewPair<std::string, double> res2) {
    if (res1.second == res2.second) {
      return (res1.first.length() > res2.first.length());
    }
    return (res1.second < res2.second); // ascending order
  }
};

// struct compareFrequentLabelCombs { // lexicographical order
//   bool operator()(std::vector<std::string> flc1,
//                   std::vector<std::string> flc2) {
//     if (flc1.size() != flc2.size()) {
//       return (flc1.size() < flc2.size());
//     }
//     unsigned int i = 0;
//     while (i < flc1.size()) {
//       if (flc1[i] != flc2[i]) {
//         return (flc1[i] < flc2[i]);
//       }
//       i++;
//     }
//     return false;
//   }
// };

/*

The original mlabel objects are transformed into a map from a label onto a set
of tuple ids (one for every tuple containing the label) together with a time
period (the time period in which the label occurs for this tuple), the number
of occurrences inside the tuple, and the total duration of its occurrences.

*/

struct RelAgg {
  RelAgg() {}
  
  void clear(const bool deleteInv);
  void initializeInv();
  void insertLabelAndBbox(const std::string& label, const TupleId& id, 
                          const temporalalgebra::SecInterval& iv, Rect& rect);
  void scanRelation(Relation *rel, const NewPair<int, int> attrPos, Geoid *g);
  void filter(const double ms, const size_t memSize);
  bool buildAtom(std::string label, AggEntry entry,
                 const std::set<TupleId>& commonTupleIds, std::string& atom);
  void retrieveLabelCombs(const unsigned int size, 
                          std::vector<std::string>& source, 
                          std::set<std::vector<std::string > >& result);
  AggEntry* getLabelEntry(std::string label);
  bool canLabelsBeFrequent(std::vector<std::string>& labelSeq,
                                 std::set<TupleId>& intersection);
  double sequenceSupp(std::vector<std::string> labelSeq,
                      std::set<TupleId> intersection);
  void combineApriori(std::set<std::vector<std::string > >& frequentLabelCombs,
                      std::set<std::vector<std::string > >& labelCombs);
  void retrievePermutations(std::vector<std::string>& labelComb,
                            std::set<std::vector<std::string > >& labelPerms);
  void derivePatterns(const int mina, const int maxa);
  std::string print(const std::map<std::string, AggEntry>& contents) const;
  std::string print(const std::map<TupleId, std::vector<std::string> >& 
                                                          frequentLabels) const;
  std::string print(const std::vector<std::string>& labelComb) const;
  std::string print(const std::set<std::vector<std::string> >& labelCombs) 
                                                                          const;
  std::string print(const std::string& label = "");
  
  template<class T>
  std::string print(const std::set<T>& anySet) const {
    std::stringstream result;
    result << "{";
    bool first = true;
    for (auto it : anySet) {
      if (!first) {
        result << ", ";
      }
      first = false;
      result << it;
    }
    result << "}";
    return result.str();
  }
  
  unsigned int noTuples, minNoAtoms, maxNoAtoms;
  std::map<std::string, AggEntry> entriesMap; // only for initial insertions
  std::vector<std::pair<std::string, AggEntry> > entries;
  InvertedFile *inv; // leaves contain positions of vector ~entries~
  std::vector<NewPair<std::string, double> > results;
  double minSupp;
  Geoid *geoid;
  Relation *rel;
  NewPair<int, int> attrPos; // textual, spatial
};

struct GetPatternsLI {
  GetPatternsLI(Relation *r, const NewPair<int, int> ap, double ms, int mina,
                int maxa, Geoid *g, const size_t mem);
  ~GetPatternsLI();
  
  TupleType *getTupleType() const;
  Tuple *getNextResult();
  
  
  TupleType *tupleType;
  RelAgg agg;
};
  
  
}
