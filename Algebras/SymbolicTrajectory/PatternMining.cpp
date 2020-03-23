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
  noOccurrences = 0;
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

void AggEntry::clear() {
  for (auto it : occurrences) {
    it.second->DeleteIfAllowed();
  }
  occurrences.clear();
  noOccurrences = 0;
  duration.SetType(datetime::durationtype);
  duration.ReadFrom((int64_t)0);
}
  
unsigned int AggEntry::getNoOccurrences(const TupleId& id) const {
  auto it = occurrences.find(id);
  if (it == occurrences.end()) {
    return 0;
  }
  return it->second->GetNoComponents();
}

void AggEntry::computeCommonTimeInterval(SecInterval& iv) const {
  iv.start.SetType(instanttype);
  iv.end.SetType(instanttype);
  iv.start.ToMaximum();
  iv.end.ToMinimum();
  Instant first(1.0), last(1.0);
  for (auto it : occurrences) {
    if (it.second->IsDefined()) {
      if (!it.second->IsEmpty()) {
        it.second->Minimum(first);
        it.second->Maximum(last);
        if (first < iv.start) {
          iv.start = first;
        }
        if (last > iv.end) {
          iv.end = last;
        }
      }
    }
  }
  if (iv.start.IsMinimum() || iv.end.IsMaximum()) {
    iv.SetDefined(false);
  }
}

void AggEntry::computeSemanticTimeSpec(string& semanticTimeSpec) const {
  semanticTimeSpec.clear();
  int month = 1; // {1, ..., 12}
  int weekday = 0; // {0, ..., 6}
  int daytime = 0; // {0, ..., 3}
  Instant first(1.0), last(1.0);
  for (map<TupleId, Periods*>::const_iterator it = occurrences.begin();
       it != occurrences.end(); it++) {
    if (it->second->IsDefined()) {
      if (!it->second->IsEmpty()) {
        it->second->Minimum(first);
        it->second->Maximum(last);
        if (it == occurrences.begin() && first.GetMonth() == last.GetMonth()) {
          month = first.GetMonth();
        }
        else if (month != first.GetMonth() 
              || first.GetMonth() != last.GetMonth()) {
          month = -1;
        }
        if (it == occurrences.begin() 
         && first.GetWeekday() == last.GetWeekday()) {
          weekday = first.GetWeekday();
        }
        else if (weekday != first.GetWeekday()
              || first.GetWeekday() != last.GetWeekday()) {
          weekday = -1;
        }
        if (it == occurrences.begin()
   && Tools::getDaytime(first.GetHour()) == Tools::getDaytime(last.GetHour())) {
          daytime = Tools::getDaytime(first.GetHour());
        }
        else if (daytime != Tools::getDaytime(first.GetHour())
              || Tools::getDaytime(first.GetMonth()) != 
                 Tools::getDaytime(last.GetMonth())) {
          daytime = -1;
        }
      }
    }
  }
  if (month > -1) {
    semanticTimeSpec = Tools::getMonthStr(month - 1);
  }
  if (weekday > -1) {
    semanticTimeSpec += (semanticTimeSpec.empty() ? "" : ", ")
                        + Tools::getWeekdayStr(weekday);
  }
  if (daytime > -1) {
    semanticTimeSpec += (semanticTimeSpec.empty() ? "" : ", ")
                        + Tools::getDaytimeStr(daytime);
  }
}

std::string AggEntry::print(const TupleId& id /* = 0 */) const {
  std::stringstream result;
  if (id == 0) { // print everything
    for (auto it : occurrences) {
      result << "   TID " << it.first << ": " << it.second->GetNoComponents() 
             << " occurrences" << endl << "      " << *(it.second) << endl;
    }
  }
  else {
    auto it = occurrences.find(id);
    if (it == occurrences.end()) { // id not found
      result << "   TID " << id << " not found" << endl;
    }
    else {
      result << "   TID " << id << ": " << *(it->second) << endl;
    }
  }
  return result.str();
}

void RelAgg::clear() {
  for (auto it : contents) {
    it.second.clear();
  }
  contents.clear();
  sortedContents.clear();
}

/*
  Class ~RelAgg~, Function ~insert~
  
  Insert new labels into map structure, update structure for existing labels
 
*/

void RelAgg::insert(const std::string& label, const TupleId& id,
                    const temporalalgebra::SecInterval& iv) {
//   cout << "insert (" << label << ", " << id << ", " << iv << ")" << endl;
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
      contents[label].occurrences[id]->Add(iv);
    }
  }
  contents[label].noOccurrences++;
  contents[label].duration += iv.end - iv.start;
}

/*
  Class ~RelAgg~, Function ~compute~
  
  Scan relation, call ~insert~ for all labels of mlabel attribute
 
*/

void RelAgg::compute(Relation *rel, const NewPair<int, int> attrPos) {
  string label;
  SecInterval iv(true);
  noTuples = rel->GetNoTuples();
  GenericRelationIterator* it = rel->MakeScan();
  Tuple *tuple = 0;
  while ((tuple = it->GetNextTuple())) {
    MLabel *ml = (MLabel*)(tuple->GetAttribute(attrPos.first));
    for (int j = 0; j < ml->GetNoComponents(); j++) {
      ml->GetValue(j, label);
      ml->GetInterval(j, iv);
      insert(label, tuple->GetTupleId(), iv);
    }
    tuple->DeleteIfAllowed();
  }
  delete it;
}

/*
  Class ~RelAgg~, Function ~sort~
  
  Sort contents by comparison function ~compareEntries~; keep only entries with
  supp >= minSupp; all others are removed from ~contents~;
 
*/

void RelAgg::sort(const double ms) {
  minSupp = ms;
  double supp = 1.0;
  // scan ~contents~; push frequent items into ~sortedContents~
  for (auto it : contents) {
    supp = double(it.second.occurrences.size()) / noTuples;
    if (supp >= minSupp) {
      sortedContents.push_back(it);
//       for (auto it2 : it.second.occurrences) {
//         // store frequent labels with tuple id
//         frequentLabels[it2.first].push_back(it.first);
//       }
    }
  }
  // sort ~sortedContents~ by comparison function
  std::sort(sortedContents.begin(), sortedContents.end(), compareEntries());
  // clean ~contents~
  contents.clear();
  for (auto it : sortedContents) {
    contents[it.first] = it.second;
  }
}

/*
  Class ~RelAgg~, Function ~buildAtomWithSupp~
  
  Build a string representing a pattern atom from an entry of ~sortedContents~
  and compute its support
 
*/

void RelAgg::buildAtom(pair<string, AggEntry>& sortedContentsEntry, 
                       string& atom) {
  SecInterval iv(true);
  string timeSpec, semanticTimeSpec;
  string label = sortedContentsEntry.first;
  sortedContentsEntry.second.computeCommonTimeInterval(iv);
  sortedContentsEntry.second.computeSemanticTimeSpec(semanticTimeSpec);
  if (!semanticTimeSpec.empty()) {
    if (iv.start.IsDefined() && iv.end.IsDefined()) {
      timeSpec = "{" + iv.start.ToString() + "~" + iv.end.ToString() + ", "
                + semanticTimeSpec + "}";
    }
    else {
      timeSpec = "{" + semanticTimeSpec + "}";
    }
  }
  else {
    if (iv.IsDefined()) {
      timeSpec = iv.start.ToString() + "~" + iv.end.ToString();
    }
    else {
      timeSpec = "_";
    }
  }
  atom = "(" + timeSpec + " \"" + label + "\")";
}

/*
  Class ~RelAgg~, Function ~retrieveLabelSets~
  
  Computes all label combinations of size ~size~ for a tuple
  
*/
void RelAgg::retrieveLabelCombs(const unsigned int size, vector<string>& source,
                                set<vector<string > >& result) {
  vector<string> labelVec;
  Tools::subset(source, size, 0, labelVec, result);
}

/*
  Class ~RelAgg~, Function ~canIntersectionBeFrequent~
  
  Computes the fraction of tuples in which all strings of ~labelSeq~ occur
  
*/
bool RelAgg::canIntersectionBeFrequent(vector<string>& labelSeq,
                                       set<TupleId>& intersection) {
  set<TupleId> firstOccs, secondOccs;
  for (auto it : contents[labelSeq[0]].occurrences) {
    firstOccs.insert(it.first);
  }
  for (auto it : contents[labelSeq[1]].occurrences) {
    secondOccs.insert(it.first);
  }
  set_intersection(firstOccs.begin(), firstOccs.end(), secondOccs.begin(), 
                secondOccs.end(), inserter(intersection, intersection.begin()));
  // check support of intersection; if it is below ~minSupp~, there is no chance
  // for a frequent 2-pattern
//   cout << "support of " << print(intersection) << " equals "
//        << double(intersection.size()) / noTuples << " ==> "
//        << (double(intersection.size()) / noTuples >= minSupp) << endl;
  return (double(intersection.size()) / noTuples >= minSupp);
}

/*
  Class ~RelAgg~, Function ~sequenceSupp~
  
  Computes the support for a given sequence of two labels
  
*/

double RelAgg::sequenceSupp(vector<string>& labelSeq, 
                            set<TupleId>& intersection) {
  Instant firstStart, secondEnd;
  int counter = 0;
  for (auto itInt : intersection) {
    if (contents[labelSeq[0]].occurrences.find(itInt) != 
        contents[labelSeq[0]].occurrences.end()
     && contents[labelSeq[1]].occurrences.find(itInt) != 
        contents[labelSeq[1]].occurrences.end()) { // both labels found
//     cout << "Compute Minimum for " << labelSeq[0] << ", id " << itInt << ": "
//          << *(contents[labelSeq[0]].occurrences[itInt]) << endl;
      contents[labelSeq[0]].occurrences[itInt]->Minimum(firstStart);
//       contents[labelSeq[0]].occurrences[itInt]->Maximum(firstEnd);
//     cout << "compute minimum for " << labelSeq[1] << ", id " << itInt << ": "
//         << *(contents[labelSeq[1]].occurrences[itInt]) << endl;
//       contents[labelSeq[1]].occurrences[itInt]->Minimum(secondStart);
      contents[labelSeq[1]].occurrences[itInt]->Maximum(secondEnd);
      if (firstStart < secondEnd) {
        counter++;
      }
//       if (secondStart < firstEnd) {
//         counter[1]++;
//       }
    }
//     if (contents[labelSeq[0]].occurrences.find(itInt) == 
//         contents[labelSeq[0]].occurrences.end()) {
//       cout << "  No Periods for " << labelSeq[0] << ", id " << itInt << endl;
//     }
//     if (contents[labelSeq[1]].occurrences.find(itInt) == 
//         contents[labelSeq[1]].occurrences.end()) {
//       cout << "  no periods for " << labelSeq[1] << ", id " << itInt << endl;
//     }
  }
//   cout << "support of sequence <" << labelSeq[0] << ", " << labelSeq[1] 
//        << "> is " << counter[0] << " / " << noTuples << endl;
  return double(counter) / noTuples;
}

/*
  Class ~RelAgg~, Function ~combineApriori~
  
  Combine sets of $k$ frequent labels to sets of $k+1$ labels, similarly to
  Apriori algorithm, e.g. {a,b,c} combined with {a,b,d} yields {a,b,c,d}
 
*/

void RelAgg::combineApriori(set<vector<string > > frequentLabelCombs,
                            set<vector<string > > labelCombs) {
  for (auto it : frequentLabelCombs) {
    
  }
}

/*
  Class ~RelAgg~, Function ~derivePatterns~
  
  Scan sorted representation in order to retrieve patterns
 
*/

void RelAgg::derivePatterns(const int ma, Relation *rel) {
  minNoAtoms = ma;
  // retrieve patterns with one atom, guaranteed to fulfill minSupp
  string pattern, atom;
  map<string, string> label2atom;
  double supp = 1.0;
  for (auto it : sortedContents) {
    buildAtom(it, atom);
    pattern = atom;
    if (minNoAtoms == 1) {
      supp = double(it.second.occurrences.size()) / noTuples;
      results.push_back(make_pair(pattern, supp));
    }
    label2atom.insert(make_pair(it.first, atom));
  }
  // retrieve patterns with two atoms
  string label;
  set<vector<string > > labelCombs, frequentLabelCombs;
  SecInterval iv(true);
  vector<string> frequentLabels;
  // scan ~contents~; only atoms whose corresponding 1-patterns fulfill
  // ~minSupp~ can be part of a frequent 2-pattern
  for (auto it : contents) {
    frequentLabels.push_back(it.first);
  }
  retrieveLabelCombs(2, frequentLabels, labelCombs);
//   cout << print(labelCombs) << endl;
  // check all combinations for their support
  string atom1, atom2;
  set<TupleId> intersection;
  for (auto labelComb : labelCombs) {
    if (canIntersectionBeFrequent(labelComb, intersection)) {
      supp = sequenceSupp(labelComb, intersection);
      if (supp >= minSupp) {
        // build complete 2-pattern
        pattern = label2atom[labelComb[0]] + " " + label2atom[labelComb[1]];
        if (minNoAtoms <= 2) {
          results.push_back(make_pair(pattern, supp));
        }
        frequentLabelCombs.insert(frequentLabelCombs.end(), labelComb);
      }
    }
  }
  // retrieve patterns with three atoms
  if (!frequentLabelCombs.empty()) {
    labelCombs.clear();
    combineApriori(frequentLabelCombs, labelCombs);
  }
}

string RelAgg::print(const vector<pair<string, AggEntry> >&
                                                         sortedContents) const {
  stringstream result;
  for (auto it : sortedContents) {
    result << "\"" << it.first << "\" occurs " << it.second.noOccurrences
           << " times with a total duration of " << it.second.duration << endl 
           << "   " << it.second.print()
           << endl << "-----------------------------------------------" << endl;
  }
  return result.str();
}

string RelAgg::print(const map<TupleId,vector<string> >& frequentLabels) const {
  stringstream result;
  for (auto it : frequentLabels) {
    result << "TID " << it.first << ": ";
    for (auto it2 : it.second) {
      result << "\"" << it2 << "\" ";
    }
    result << endl;
  }
  return result.str();
}

string RelAgg::print(const set<vector<string> >& labelCombs) const {
  stringstream result;
  result << "{" << endl;
  for (auto it : labelCombs) {
    bool first = true;
    result << "  {";
    for (auto it2 : it) {
      if (!first) {
        result << ", ";
      }
      first = false;
      result << "\"" << it2 << "\"";
    }
    result << "}" << endl;
  }
  result << "}" << endl;
  return result.str();
}

string RelAgg::print(const set<TupleId>& tidSet) const {
  stringstream result;
  result << "{";
  bool first = true;
  for (auto it : tidSet) {
    if (!first) {
      result << ", ";
    }
    first = false;
    result << it;
  }
  result << "}";
  return result.str();
}

string RelAgg::print(const string& label /* = "" */) const {
  stringstream result;
  if (label == "") { // print everything
    for (auto it : contents) {
      result << "\"" << it.first << "\" :" << it.second.print() << endl
             << "-----------------------------------------------------" << endl;
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

GetPatternsLI::GetPatternsLI(Relation *r, const NewPair<int,int> ap, double ms, 
                             int ma)
  : rel(r), attrPos(ap) {
  tupleType = getTupleType();
  agg.clear();
  agg.compute(rel, attrPos);
  agg.sort(ms);
//   cout << agg.print(agg.sortedContents) << endl;
  agg.derivePatterns(ma, rel);
}

GetPatternsLI::~GetPatternsLI() {
  tupleType->DeleteIfAllowed();
  agg.clear();
}

TupleType* GetPatternsLI::getTupleType() const {
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  ListExpr resultTupleType = nl->TwoElemList(
    nl->SymbolAtom(Tuple::BasicType()),
    nl->TwoElemList(nl->TwoElemList(nl->SymbolAtom("Pattern"),
                                    nl->SymbolAtom(FText::BasicType())),
                    nl->TwoElemList(nl->SymbolAtom("Support"),
                                    nl->SymbolAtom(CcReal::BasicType()))));
  ListExpr numResultTupleType = sc->NumericType(resultTupleType);
  return new TupleType(numResultTupleType);
}

Tuple* GetPatternsLI::getNextResult() {
  if (agg.results.empty()) {
    return 0;
  }
  Tuple *tuple = new Tuple(tupleType);
  pair<string, double> result;
  result = agg.results.front();
  agg.results.pop_front();
  FText *pattern = new FText(true, result.first);
  tuple->PutAttribute(0, pattern);
  CcReal *support = new CcReal(true, result.second);
  tuple->PutAttribute(1, support);
  return tuple;
}


}
