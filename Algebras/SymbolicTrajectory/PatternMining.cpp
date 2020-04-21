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
#include <chrono>

using namespace std;
using namespace datetime;
using namespace temporalalgebra;
using namespace std::chrono; 

namespace stj {

AggEntry::AggEntry() {
  occs.clear();
  occsPos.clear();
  noOccs = 0;
  duration.SetType(datetime::durationtype);
  duration.ReadFrom((int64_t)0);
}
  
AggEntry::AggEntry(const TupleId id, const temporalalgebra::SecInterval& iv,
                   Rect& rect, const unsigned int noTuples) {
  occs.clear();
  occsPos.clear();
  occsPos.resize(noTuples + 1, UINT_MAX);
  Periods *per = new Periods(1);
  per->Add(iv);
  occsPos[id] = occs.size();
  occs.push_back(make_tuple(id, per, rect));
  noOccs = 0;
  duration.SetType(datetime::durationtype);
  duration.ReadFrom((int64_t)0); // durations are computed at the end
}

void AggEntry::clear() {
  for (auto it : occs) {
    get<1>(it)->DeleteIfAllowed();
  }
  occs.clear();
  occsPos.clear();
  noOccs = 0;
  duration.SetType(datetime::durationtype);
  duration.ReadFrom((int64_t)0);
}

void AggEntry::deletePeriods() {
  for (auto it : occs) {
    (get<1>(it))->DeleteIfAllowed();
  }
}

ListExpr AggEntry::toListExpr() {
  ListExpr occsList(nl->Empty()), occList(nl->Empty());
  Word perWord, rectWord;
  TupleId id;
  if (occs.size() >= 1) {
    id = get<0>(occs[0]);
    perWord.addr = get<1>(occs[0]);
    rectWord.addr = &(get<2>(occs[0]));
    occsList = nl->OneElemList(nl->ThreeElemList(nl->IntAtom(id),
                           OutRange<Instant, OutDateTime>(nl->Empty(), perWord),
                           OutRectangle<2>(nl->Empty(), rectWord)));
    occList = occsList;
  }
  for (unsigned int i = 1; i < occs.size(); i++) {
    id = get<0>(occs[i]);
    perWord.addr = get<1>(occs[i]);
    rectWord.addr = &(get<2>(occs[i]));
    occList = nl->Append(occList, nl->ThreeElemList(nl->IntAtom(id),
                           OutRange<Instant, OutDateTime>(nl->Empty(), perWord),
                           OutRectangle<2>(nl->Empty(), rectWord)));
  }
  return nl->ThreeElemList(nl->IntAtom(noOccs), duration.ToListExpr(false), 
                           occsList);
}
  
unsigned int AggEntry::getNoOccs(const TupleId& id) const {
  return get<1>(occs[occsPos[id]])->GetNoComponents();
}

void AggEntry::computeCommonTimeInterval(const set<TupleId>& commonTupleIds,
                                         SecInterval& iv) {
  iv.start.SetType(instanttype);
  iv.end.SetType(instanttype);
  iv.start.ToMaximum();
  iv.end.ToMinimum();
  Instant first(1.0), last(1.0);
  if (commonTupleIds.empty()) { // use all occurrences
    for (auto it : occs) {
      if (get<1>(it)->IsDefined()) {
        if (!get<1>(it)->IsEmpty()) {
          get<1>(it)->Minimum(first);
          get<1>(it)->Maximum(last);
          if (first < iv.start) {
            iv.start = first;
          }
          if (last > iv.end) {
            iv.end = last;
          }
        }
      }
    }
  }
  else { // use only entries occurring in the set
    for (auto it : commonTupleIds) {
      if (occsPos[it] != UINT_MAX) {
        Periods* per = get<1>(occs[occsPos[it]]);
        if (per->IsDefined()) {
          if (!per->IsEmpty()) {
            per->Minimum(first);
            per->Maximum(last);
            if (first < iv.start) {
              iv.start = first;
            }
            if (last > iv.end) {
              iv.end = last;
            }
          }
        }
      }
    }
  }
  if (iv.start.IsMinimum() || iv.end.IsMaximum()) {
    iv.SetDefined(false);
  }
}

void AggEntry::computeCommonRect(const SecInterval& iv,
                 const set<TupleId>& commonTupleIds, Geoid *geoid, Rect &rect) {
  if (commonTupleIds.empty()) { // use all occurrences (for 1-patterns)
    if (occs.empty()) {
      return;
    }
    if (!get<2>(occs[0]).IsDefined()) {
      rect.SetDefined(false);
      return;
    }
    Rect tempRect = get<2>(occs[0]);
    for (unsigned int i = 1; i < occs.size(); i++) {
      if (!get<2>(occs[i]).IsDefined()) {
        rect.SetDefined(false);
        return;
      }
      tempRect = get<2>(occs[i]).Union(tempRect);
    }
    rect = tempRect;
  }
  else {
    set<TupleId>::iterator it = commonTupleIds.begin();
    if (!get<2>(occs[occsPos[*it]]).IsDefined()) {
      rect.SetDefined(false);
      return;
    }
    Rect tempRect = get<2>(occs[occsPos[*it]]);
    while (it != commonTupleIds.end()) {
      if (!get<2>(occs[occsPos[*it]]).IsDefined()) {
        rect.SetDefined(false);
        return;
      }
      tempRect = get<2>(occs[occsPos[*it]]).Union(tempRect);
      it++;
    }
    rect = tempRect;
  }
}

void AggEntry::computeSemanticTimeSpec(const set<TupleId>& commonTupleIds,
                                       string& semanticTimeSpec) const {
  semanticTimeSpec.clear();
  int month = 1; // {1, ..., 12}
  int weekday = 0; // {0, ..., 6}
  int daytime = 0; // {0, ..., 3}
  Instant first(1.0), last(1.0);
  Periods *per = 0;
  for (set<TupleId>::const_iterator it = commonTupleIds.begin(); 
                                             it != commonTupleIds.end(); it++) {
    per = get<1>(occs[occsPos[*it]]);
    if (!per->IsDefined()) {
      semanticTimeSpec = "";
      return;
    }
    if (per->IsEmpty()) {
      semanticTimeSpec = "";
      return;
    }
    per->Minimum(first);
    per->Maximum(last);
    if (it == commonTupleIds.begin() && first.GetMonth() == last.GetMonth()) {
      month = first.GetMonth();
    }
    else if (month != first.GetMonth() 
          || first.GetMonth() != last.GetMonth()) {
      month = -1;
    }
    if (it == commonTupleIds.begin() 
      && first.GetWeekday() == last.GetWeekday()) {
      weekday = first.GetWeekday();
    }
    else if (weekday != first.GetWeekday()
          || first.GetWeekday() != last.GetWeekday()) {
      weekday = -1;
    }
    if (it == commonTupleIds.begin()
   && Tools::getDaytime(first.GetHour()) == Tools::getDaytime(last.GetHour())) {
      daytime = Tools::getDaytime(first.GetHour());
    }
    else if (daytime != Tools::getDaytime(first.GetHour())
          || Tools::getDaytime(first.GetMonth()) != 
              Tools::getDaytime(last.GetMonth())) {
      daytime = -1;
    }
    if (month == -1 && weekday == -1 && daytime == -1) {
      semanticTimeSpec = "";
      return;
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
    for (auto it : occs) {
      result << "   TID " << get<0>(it) << ": " << get<1>(it)->GetNoComponents()
             << " occs:  " << *(get<1>(it)) << get<2>(it) << endl;
    }
  }
  else {
    if (occsPos[id] == UINT_MAX) { // id not found
      result << "   TID " << id << " not found" << endl;
    }
    else {
      result << "   TID " << id << ": " 
             << get<1>(occs[occsPos[id]])->GetNoComponents() << " occs:  " 
             << *(get<1>(occs[occsPos[id]])) << get<2>(occs[occsPos[id]]) 
             << endl;
    }
  }
  return result.str();
}

std::string AggEntry::print(const Rect& rect) const {
  std::stringstream result;
  if (!rect.IsDefined()) {
    return "_";
  }
  result << "[" << rect.MinD(0) << " " << rect.MaxD(0) << " " << rect.MinD(1)
         << " " << rect.MaxD(1) << "]";
  return result.str();
}

/*
  Class ~RelAgg~, Constructor
  
*/
RelAgg::RelAgg() : noTuples(0), minNoAtoms(0), maxNoAtoms(0), minSupp(0.0), 
                   geoid(0), rel(0) {}

/*
  Class ~RelAgg~, Function ~clear~
  
  Deletes the periods values
 
*/

void RelAgg::clear() {
  for (auto it : entriesMap) {
    it.second.clear();
  }
}

void RelAgg::clearEntries() {
  for (auto it : entries) {
    it.deletePeriods();
  }
  for (auto it : checkedSeqs) {
    for (auto it2 : it) {
      it2.clear();
    }
    it.clear();
  }
  checkedSeqs.clear();
  for (auto it : freqSets) {
    for (auto it2 : it) {
      it2.clear();
    }
    it.clear();
  }
  freqSets.clear();
  for (auto it : nonfreqSets) {
    for (auto it2 : it) {
      it2.clear();
    }
    it.clear();
  }
  nonfreqSets.clear();
}

/*
  Class ~RelAgg~, Function ~insertLabel~
  
  Insert new labels into map structure, update structure for existing labels
 
*/

void RelAgg::insertLabelAndBbox(const std::string& label, const TupleId& id,
                           const temporalalgebra::SecInterval& iv, Rect& rect) {
//   cout << "insert (" << label << ", " << id << ", " << iv << ")" << endl;
  if (label == "undefined") {
    return;
  }
  auto aggIt = entriesMap.find(label);
  if (aggIt == entriesMap.end()) { // new label
    AggEntry entry(id, iv, rect, rel->GetNoTuples());
    entriesMap.insert(make_pair(label, entry));
  }
  else { // label already present
    if (aggIt->second.occsPos[id] == UINT_MAX) { // new id for label
      Periods *per = new Periods(1);
      per->Add(iv);
      entriesMap[label].occsPos[id] = entriesMap[label].occs.size();
      entriesMap[label].occs.push_back(make_tuple(id, per, rect));
    }
    else { // id already present for label
      get<1>(entriesMap[label].occs[entriesMap[label].occsPos[id]])->
                                                                   MergeAdd(iv);
      if (rect.IsDefined()) {
        get<2>(entriesMap[label].occs[entriesMap[label].occsPos[id]]) = 
      get<2>(entriesMap[label].occs[entriesMap[label].occsPos[id]]).Union(rect);
      }
    }
  }
  entriesMap[label].noOccs++;
  entriesMap[label].duration += iv.end - iv.start;
}

/*
  Class ~RelAgg~, Function ~scanRelation~
  
  Scan relation, call ~insert~ for all labels of mlabel attribute
 
*/

void RelAgg::scanRelation(Relation *r, const NewPair<int, int> ap, Geoid *g) {
  rel = r;
  attrPos = ap;
  geoid = g;
  string label;
  SecInterval iv(true);
  noTuples = rel->GetNoTuples();
  GenericRelationIterator* it = rel->MakeScan();
  MLabel *ml = 0;
  MPoint *mp = 0;
  MPoint mpPart(true);
  Tuple *tuple = 0;
  Periods per(true);
  Rect rect(true);
  while ((tuple = it->GetNextTuple())) {
    ml = (MLabel*)(tuple->GetAttribute(attrPos.first));
    mp = (MPoint*)(tuple->GetAttribute(attrPos.second));
    for (int j = 0; j < ml->GetNoComponents(); j++) {
      ml->GetValue(j, label);
      ml->GetInterval(j, iv);
      per.Add(iv);
      mp->AtPeriods(per, mpPart);
      rect = mpPart.BoundingBoxSpatial();
      per.Clear();
      insertLabelAndBbox(label, tuple->GetTupleId(), iv, rect);
    }
    tuple->DeleteIfAllowed();
  }
  delete it;
}

/*
  Class ~RelAgg~, Function ~filter~
  
  Filter contents; keep only labels with supp >= minSupp
 
*/

void RelAgg::filter(const double ms, const size_t memSize) {
  minSupp = ms;
  double supp = 1.0;
  // scan ~entriesMap~; push entries for frequent labels into ~entries~ and
  //   store every label and its entry's position inside ~entries~ in ~inv~
  for (auto it : entriesMap) {
    supp = double(it.second.occs.size()) / noTuples;
    if (supp >= minSupp) {
//       cout << "INSERTED: \"" << it.first << "\", POS " << entries.size()
//            << " " << it.second.print() << endl;
      entries.push_back(it.second);
      labelPos.insert(make_pair(it.first, freqLabels.size()));
      freqLabels.push_back(it.first);
    }
    else {
      it.second.deletePeriods();
    }
  }
//   for (unsigned int i = 0; i < freqLabels.size(); i++) {
//     cout << "<" << i << " : " << freqLabels[i] << ">   ";
//   }
//   cout << endl << endl;
//   for (auto it : labelPos) {
//     cout << it.first << " |---> " << it.second << "       ";
//   }
//   cout << endl;
}

/*
  Class ~RelAgg~, Function ~buildAtom~
  
  Build a string representing a pattern atom from an entry of ~contents~ and
  compute its support
 
*/

bool RelAgg::buildAtom(unsigned int label, AggEntry entry,
                       const set<TupleId>& commonTupleIds, string& atom) {
//   cout << "bA: \"" << label << "\", " << entry.occurrences.size() << " occs" 
//        << endl;
  SecInterval iv(true);
  string timeSpec, semanticTimeSpec;
  entry.computeCommonTimeInterval(commonTupleIds, iv);
  entry.computeSemanticTimeSpec(commonTupleIds, semanticTimeSpec);
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
    if (iv.start.IsDefined() && iv.end.IsDefined()) {
      timeSpec = iv.start.ToString() + "~" + iv.end.ToString();
    }
    else {
      atom.clear();
      return false;
    }
  }
  Rect rect(true);
  entry.computeCommonRect(iv, commonTupleIds, geoid, rect);
  atom = "(" + timeSpec + " \"" + freqLabels[label] + "\" " + entry.print(rect)
         + ")";
  return true;
}

void RelAgg::subsetperm(vector<unsigned int> source, int left, int index,
           vector<unsigned int>& labelVec, set<vector<unsigned int> >& result) {
  if (left == 0) {
    do {
      result.insert(labelVec);
    } while (std::next_permutation(labelVec.begin(), labelVec.end()));
    return;
  }
  for (unsigned int i = index; i < source.size(); i++) {
    labelVec.push_back(source[i]);
    subsetperm(source, left - 1, i + 1, labelVec, result);
    labelVec.pop_back();
  }
}

void RelAgg::subset(vector<unsigned int> source, int left, int index,
           vector<unsigned int>& labelVec, set<vector<unsigned int> >& result) {
  if (left == 0) {
//     if (nonfreqSets[labelVec.size()].find(labelVec) == 
//                                         nonfreqSets[labelVec.size()].end()) {
      result.insert(labelVec);
//     }
  }
  for (unsigned int i = index; i < source.size(); i++) {
    labelVec.push_back(source[i]);
    subset(source, left - 1, i + 1, labelVec, result);
    labelVec.pop_back();
  }
}

/*
  Class ~RelAgg~, Function ~retrieveLabelSets~
  
  Computes all label combinations of size ~size~ for a tuple
  
*/
void RelAgg::retrieveLabelCombs(const unsigned int size, 
             vector<unsigned int>& source, set<vector<unsigned int> >& result) {
  result.clear();
  vector<unsigned int> labelVec;
  subsetperm(source, size, 0, labelVec, result);
}

/*
  Class ~RelAgg~, Function ~retrieveLabelSubsets~

*/
void RelAgg::retrieveLabelSubsets(const unsigned int size, 
             vector<unsigned int>& source, set<vector<unsigned int> >& result) {
  result.clear();
  vector<unsigned int> labelVec;
  subset(source, size, 0, labelVec, result);
}

double RelAgg::getSupp(unsigned int label) {
  return double(entries[label].occs.size()) / noTuples;
}

/*
  Class ~RelAgg~, Function ~canIntersectionBeFrequent~
  
  Computes the fraction of tuples in which all strings of ~labelSeq~ occur
  
*/
bool RelAgg::canLabelsBeFrequent(vector<unsigned int>& labelSeq,
                                 set<TupleId>& intersection) {
  intersection.clear();
  if (labelSeq.size() < 2) {
    cout << "sequence has only " << labelSeq.size() << " component(s)" << endl;
    return false;
  }
  set<TupleId> intersection_temp;
  vector<set<TupleId> > allOccs;
  allOccs.resize(labelSeq.size());
  // retrieve occurrences for every label
//   cout << "check sequence " << print(labelSeq) << endl;
  for (unsigned int pos = 0; pos < labelSeq.size(); pos++) {
    for (auto occ : entries[labelSeq[pos]].occs) {
      allOccs[pos].insert(get<0>(occ));
    }
  }
  // compute intersection of all id sets
  set_intersection(allOccs[0].begin(), allOccs[0].end(), allOccs[1].begin(),
                allOccs[1].end(), inserter(intersection, intersection.begin()));
  for (unsigned int pos = 2; pos < labelSeq.size(); pos++) {
    set_intersection(intersection.begin(), intersection.end(), 
                     allOccs[pos].begin(), allOccs[pos].end(), 
                     inserter(intersection_temp, intersection_temp.begin()));
    intersection = intersection_temp;
    intersection_temp.clear();
  }
  // check support of intersection; if it is below ~minSupp~, there is no chance
  // for a frequent (k+1)-pattern
//   cout << "support of " << print(intersection) << " equals "
//       << double(intersection.size()) / noTuples << " ==> "
//       << (double(intersection.size()) / noTuples >= minSupp) << endl;
  return (double(intersection.size()) / noTuples >= minSupp);
}

/*
  Class ~RelAgg~, Function ~sequenceSupp~
  
  Computes the support for a given sequence of labels
  
*/

double RelAgg::sequenceSupp(vector<unsigned int> labelSeq, 
                            set<TupleId> intersection) {
  if (labelSeq.empty()) {
    return 0.0;
  }
  Instant start(instanttype), end(instanttype);
  int noOccurrences = 0;
  AggEntry *entry;
  for (auto id : intersection) {
    // try to find all labels
    start.ToMinimum();
    end.ToMaximum();
    bool sequenceFound = true;
    unsigned int pos = 0;
    while (sequenceFound && (pos < labelSeq.size())) {
      entry = &(entries[labelSeq[pos]]);
      if (entry->occsPos[id] < UINT_MAX) {
        get<1>(entry->occs[entry->occsPos[id]])->Maximum(end);
        if (start < end) { // label found, correct order
          // set start instant to begin of periods for current label
          get<1>(entry->occs[entry->occsPos[id]])->Minimum(start);
        }
        else { // label found, but not in expected order
//           cout << "WRONG ORDER: id " << id << ", \"" << labelSeq[pos]
//                << "\" NOT after \"" << labelSeq[pos-1] << "\"" << endl;
          sequenceFound = false;
        }
      }
      else { // label not found in corresponding tuple
//         cout << "NOT FOUND: id " << id << ", \"" << labelSeq[pos] << endl;
        sequenceFound = false;
      }
      pos++;
    }
    if (sequenceFound) {
//       cout << "SEQUENCE FOUND: id " << id << ", " << print(labelSeq) << endl;
      noOccurrences++;
    }
  }
  return double(noOccurrences) / noTuples;
}

/*
  Class ~RelAgg~, Function ~combineApriori~
  
  Combine sets of $k$ frequent labels to sets of $k+1$ labels, similarly to
  Apriori algorithm, e.g. {a,b,c} combined with {a,b,d} yields {a,b,c,d},
  or {a,b,c} combined with {e,c,b} yields {
 
*/

void RelAgg::combineApriori(set<vector<unsigned int> >& frequentLabelCombs,
                            set<vector<unsigned int> >& labelCombs) {
  if (frequentLabelCombs.empty()) {
    return;
  }
  if ((frequentLabelCombs.begin())->empty()) {
    return;
  }
//   cout << "Frequent label combs for k = 2:" << endl;
//   for (auto it : frequentLabelCombs) {
//     cout << print(it) << endl;
//   }
//   cout << endl;
  unsigned int k = (frequentLabelCombs.begin())->size();
  set<vector<unsigned int> >::iterator it2;
  set<unsigned int> union_k_inc;
  for (set<vector<unsigned int> >::iterator it1 = frequentLabelCombs.begin();
       it1 != frequentLabelCombs.end(); it1++) {
    it2 = it1;
    it2++;
    while (it2 != frequentLabelCombs.end()) {
      set_union(it1->begin(), it1->end(), it2->begin(), it2->end(), 
                inserter(union_k_inc, union_k_inc.begin()));
      if (union_k_inc.size() == k+1) {
//         cout << print(*it1) << " and " << print(*it2) << " united to "
//              << print(union_k_inc) << endl;
        vector<unsigned int> unionvec(union_k_inc.begin(), union_k_inc.end());
        labelCombs.insert(labelCombs.end(), unionvec);
      }
      union_k_inc.clear();
      it2++;
    }
    union_k_inc.clear();
  }
}

void RelAgg::retrievePermutations(vector<unsigned int>& labelComb,
                                  set<vector<unsigned int> >& labelPerms) {
  labelPerms.clear();
  vector<unsigned int> labels = labelComb;
  std::sort(labels.begin(), labels.end());
  do {
    labelPerms.insert(labelPerms.end(), labels);
  } while (std::next_permutation(labels.begin(), labels.end()));
}

/*
  Class ~RelAgg~, Function ~derivePatterns~
  
  Scan sorted representation in order to retrieve patterns
 
*/

void RelAgg::derivePatterns(const int mina, const int maxa) {
  minNoAtoms = mina;
  maxNoAtoms = maxa;
  // retrieve patterns with one atom, ~entries~ guaranteed to fulfill minSupp
  string pattern, atom;
  set<TupleId> commonTupleIds;
  double supp = 1.0;
  for (unsigned int label = 0; label < entries.size(); label++) {
    buildAtom(label, entries[label], commonTupleIds, atom);
    if (minNoAtoms == 1) {
      supp = double(entries[label].occs.size()) / noTuples;
      results.push_back(NewPair<string, double>(atom, supp));
    }
  }
  cout << freqLabels.size() << " frequent 1-patterns found" << endl;
  // retrieve patterns with two atoms
  if (maxNoAtoms < 2) {
    return;
  }
  set<vector<unsigned int> > labelCombs, frequentLabelCombs, labelPerms;
  SecInterval iv(true);
  // scan ~contents~; only atoms whose corresponding 1-patterns fulfill
  // ~minSupp~ can be part of a frequent 2-pattern
  vector<unsigned int> frequentLabels;
  for (unsigned int label = 0; label < freqLabels.size(); label++) {
    frequentLabels.push_back(label);
  }
  retrieveLabelCombs(2, frequentLabels, labelPerms);
  // check all combinations for their support
  bool correct = false;
  for (auto labelPerm : labelPerms) {
    if (canLabelsBeFrequent(labelPerm, commonTupleIds)) {
      supp = sequenceSupp(labelPerm, commonTupleIds);
      if (supp >= minSupp) {
        if (minNoAtoms <= 2) {
          pattern.clear();
          // build complete 2-pattern
          correct = buildAtom(labelPerm[0], entries[labelPerm[0]], 
                              commonTupleIds, atom);
          pattern += atom + " ";
          correct = correct && buildAtom(labelPerm[1], entries[labelPerm[1]], 
                                         commonTupleIds, atom);
          pattern += atom;
          if (correct) {
            results.push_back(NewPair<string, double>(pattern, supp));
          }
        }
        frequentLabelCombs.insert(frequentLabelCombs.end(), labelPerm);
      }
    }
  }
  cout << frequentLabelCombs.size() << " frequent 2-patterns found" << endl;
  // retrieve patterns with three or more atoms
  unsigned int k = 3;
  while (k <= maxNoAtoms && !frequentLabelCombs.empty()) { // no frequent k-pat
    map<vector<unsigned int>, set<TupleId> > labelPermsWithCommonIds;
    labelCombs.clear();
    combineApriori(frequentLabelCombs, labelCombs);
    frequentLabelCombs.clear();
    for (auto labelComb : labelCombs) {
      if (canLabelsBeFrequent(labelComb, commonTupleIds)) {
//           cout << print(labelComb) << " can be frequent; occurs in " 
//                << print(commonTupleIds) << endl;
        retrievePermutations(labelComb, labelPerms);
        for (auto labelPerm : labelPerms) {
          labelPermsWithCommonIds[labelPerm] = commonTupleIds;
        }
      }
    }
    for (auto it : labelPermsWithCommonIds) {
  //       cout << print(it.first) << " occurs in " << print(it.second);
      supp = sequenceSupp(it.first, it.second);
  //       cout << ", supp is " << supp << endl;
      if (supp >= minSupp) {
        if (k >= minNoAtoms) {
          pattern.clear();
          correct = true;
          unsigned int pos = 0;
          while (correct && (pos < it.first.size())) {
            correct = correct && buildAtom(it.first[pos], 
                                       entries[it.first[pos]], it.second, atom);
            pattern += atom + " ";
            pos++;
          }
          if (correct) {
            results.push_back(NewPair<string, double>(pattern, supp));
          }
        }
        frequentLabelCombs.insert(frequentLabelCombs.end(), it.first);
//         cout << "k = " << k << "; sequence " << print(it.first) 
//              << " inserted" << endl;
      }
    }
    cout << frequentLabelCombs.size() << " frequent " << k 
         << "-patterns found" << endl;
    k++;
  }
  std::sort(results.begin(), results.end(), comparePMResults());
}

/*
  Class ~RelAgg~, Function ~computeEntriesSize~
  
  Compute storage space in bytes for all entries:
  
  constant: noOccs, (noTuples + 1) * tid, noOccs, duration
  in every occ: tid, per->noComponents * (start, lc, end, rc), 
                min(0,1), max(0,1), isdefined

*/
unsigned long long int RelAgg::computeEntriesSize() const {
  unsigned long long int constEntrySize = sizeof(unsigned int) + 
                         (noTuples + 1) * sizeof(unsigned int) + 
                         sizeof(unsigned int) + sizeof(double);
  cout << "const entry size is " << constEntrySize << endl;
  unsigned long long int result = entries.size() * constEntrySize;
  for (auto entry : entries) {
    for (auto occ : entry.occs) {
      result += sizeof(unsigned int) + sizeof(unsigned int) +
          get<1>(occ)->GetNoComponents() * (2 * (sizeof(double) + sizeof(bool)))
          + 4 * sizeof(double) + sizeof(bool);
    }
  }
  return result;
}

/*
  Class ~RelAgg~, Function ~computeFreqLabelsSize~
  
  Compute storage space in bytes for ~freqLabels~:
  
  size + size * (wordlength + word)

*/
unsigned long long int RelAgg::computeFreqLabelsSize() const {
  unsigned long long int result = sizeof(unsigned int);
  for (auto label : freqLabels) {
    result += label.size() + 1 + sizeof(unsigned int);
  }
  return result;
}

string RelAgg::print(const map<unsigned int, AggEntry>& contents) const {
  stringstream result;
  for (auto it : contents) {
    result << "\"" << freqLabels[it.first] << "\" occurs " 
           << it.second.noOccs << " times with a total duration of " 
           << it.second.duration << endl << "   " << it.second.print()
           << endl << "-----------------------------------------------" << endl;
  }
  return result.str();
}

string RelAgg::print(const map<TupleId,vector<unsigned int> >& frequentLabels) 
                                                                         const {
  stringstream result;
  for (auto it : frequentLabels) {
    result << "TID " << it.first << ": ";
    for (auto it2 : it.second) {
      result << "\"" << freqLabels[it2] << "\" ";
    }
    result << endl;
  }
  return result.str();
}

string RelAgg::print(const set<vector<unsigned int> >& labelCombs) const {
  stringstream result;
  result << "{" << endl;
  for (auto it : labelCombs) {
    result << "  " << print(it);
  }
  result << "}" << endl;
  return result.str();
}

string RelAgg::print(const unsigned int label /* = UINT_MAX */) {
  stringstream result;
  if (label == UINT_MAX) { // print everything
    for (unsigned int l = 0; l < freqLabels.size(); l++) {
      result << "\"" << freqLabels[label] << "\" :" << entries[label].print() 
             << endl << "---------------------------------------------" << endl;
    }
  }
  else {
    if (entries[label].occs.empty()) { // label not found
      result << "Label \"" << freqLabels[label] << "\" not found" << endl;
    }
    else {
      result << "\"" << freqLabels[label] << "\" :" << entries[label].print() 
             << endl;
    }
  }
  return result.str();
}

/*
  Class ~FPNode~, Function ~toListExpr~

*/
ListExpr FPNode::toListExpr(vector<string>& freqLabels) const {
  ListExpr childrenList = nl->Empty();
  if (!children.empty()) {
    childrenList = nl->OneElemList(nl->IntAtom(children[0]));
  }
  ListExpr childList = childrenList;
  for (unsigned int i = 1; i < children.size(); i++) {
    childList = nl->Append(childList, nl->IntAtom(children[i]));
  }
  string lb = (label < UINT_MAX ? freqLabels[label] : "<ROOT>");
  return nl->FiveElemList(nl->SymbolAtom(lb), 
                          nl->IntAtom(frequency), childrenList, 
                          nl->IntAtom(nodeLink), nl->IntAtom(ancestor));
}

/*
  Class ~FPTree~, Function ~isChildOf~

*/
bool FPTree::isChildOf(unsigned int label, unsigned int pos, 
                                                        unsigned int& nextPos) {
  for (auto it : nodes[pos].children) {
    if (nodes[it].label == label) {
      nextPos = it;
      return true;
    }
  }
  nextPos = UINT_MAX;
  return false;
}

/*
  Class ~FPTree~, Function ~updateNodeLink~

*/
void FPTree::updateNodeLink(unsigned int label, unsigned int targetPos) {
  map<unsigned int, unsigned int>::iterator it = nodeLinks.find(label);
  if (it == nodeLinks.end()) { // no existing node link
    nodeLinks.insert(make_pair(label, targetPos));
  }
  else { // node link for label exists
    unsigned int link = it->second;
    unsigned int currentPos = 0;
    while (link != 0) { // find end of node link
      currentPos = link;
      link = nodes[link].nodeLink;
    }
    nodes[currentPos].nodeLink = targetPos;
  }
}

/*
  Class ~FPTree~, Function ~insertLabelVector~

*/
void FPTree::insertLabelVector(const vector<unsigned int>& labelsOrdered,
                               const unsigned int freq) {
//   cout << "insert: | ";
//   for (auto it : labelsOrdered) {
//     cout << it << " | ";
//   }
//   cout << endl;
  unsigned int nodePos(0), nextPos(0);
  for (auto label : labelsOrdered) {
    if (isChildOf(label, nodePos, nextPos)) {
      nodes[nextPos].frequency += freq;
//       cout << "  \"" << label << "\" is child of \"" << nodes[nodePos].label 
//            << "\", frequency = " << nodes[nextPos].frequency << endl;
      nodePos = nextPos;
    }
    else {
      FPNode node(label, freq, nodePos);
      nodes.push_back(node);
      nodes[nodePos].children.push_back(nodes.size() - 1);
      updateNodeLink(label, nodes.size() - 1);
//       cout << "  new node for \"" << label << "\" at pos " 
//            << nodes.size() - 1 << ", now child of " << nodePos << endl;
      nodePos = nodes.size() - 1;
    }
  }
//   cout << "   ... SUCCESSFULLY inserted" << endl;
}

/*
  Class ~FPTree~, Function ~construct~

*/
void FPTree::construct() {
  GenericRelationIterator* it = agg->rel->MakeScan();
  MLabel *ml = 0;
  Tuple *tuple = 0;
  string label;
  set<NewPair<unsigned int, double>, compareLabelsWithSupp> labelsWithSupp;
  vector<unsigned int> labelsOrdered;
  while ((tuple = it->GetNextTuple())) {
    labelsWithSupp.clear();
    labelsOrdered.clear();
    ml = (MLabel*)(tuple->GetAttribute(agg->attrPos.first));
    for (int j = 0; j < ml->GetNoComponents(); j++) {
      ml->GetValue(j, label);
      unsigned labelPos = agg->labelPos[label];
      NewPair<unsigned int, double> labelWithSupp(labelPos, 
                                                  agg->getSupp(labelPos));
      if (labelWithSupp.second >= minSupp) {
        labelsWithSupp.insert(labelWithSupp);
      }
    }
    for (auto itl : labelsWithSupp) {
      labelsOrdered.push_back(itl.first);
    }
    insertLabelVector(labelsOrdered, 1);
    tuple->DeleteIfAllowed();
  }
  delete it;
}

/*
  Class ~FPTree~, Function ~initialize~

*/
void FPTree::initialize(const double ms, RelAgg *ra) {
  minSupp = ms;
  FPNode node(UINT_MAX, 0, 0);
  nodes.push_back(node); // create dummy node for root
  agg = ra;
  minSuppCnt = (unsigned int)std::ceil(minSupp * ra->noTuples);
  agg->checkedSeqs.resize(agg->maxNoAtoms + 1);
}

/*
  Class ~FPTree~, Function ~isOnePathTree~

*/
bool FPTree::isOnePathTree() {
  for (unsigned int i = 0; i < getNoNodes(); i++) {
    if (i < getNoNodes() - 1) { // inner node
      if (nodes[i].children.size() != 1) {
        return false; // inner nodes have more or less than one child
      }
      if (nodes[i].children[0] != i+1) {
        return false; // wrong position of child
      }
      if (nodes[i].nodeLink != 0) {
        return false; // node link exists
      }
    }
    if (i == getNoNodes() - 1 && nodes[i].children.size() > 0) {
      return false; // leaf node has childs (ERROR)
    }
  }
  return true;
}

/*
  Class ~FPTree~, Function ~sortNodeLinks~

*/
void FPTree::sortNodeLinks(vector<unsigned int>& result) {
  result.clear();
  set<NewPair<unsigned int, double>, compareLabelsWithSupp> labelsWithSupp;
  for (auto it : nodeLinks) {
    labelsWithSupp.insert(NewPair<unsigned int, double>(it.first, 
                                                       agg->getSupp(it.first)));
  }
  for (set<NewPair<unsigned int, double>,
          compareLabelsWithSupp>::reverse_iterator it = labelsWithSupp.rbegin();
                                            it != labelsWithSupp.rend(); ++it) {
    result.push_back(it->first);
  }
}



/*
  Class ~FPTree~, Function ~collectPatternsFromSeq~

*/
void FPTree::collectPatternsFromSeq(vector<unsigned int>& labelSeq,
                 const unsigned int minNoAtoms, const unsigned int maxNoAtoms) {
  set<vector<unsigned int> > labelSubsets, labelPerms;
  unsigned int minSetSize = max(minNoAtoms, (unsigned int)2);
  set<TupleId> commonTupleIds;
  string atom, pattern;
  double supp;
  // find all subsets of label sequence, having a suitable number of elements
  unsigned int setSize = minSetSize;
  unsigned oldResultSize = 0;
  bool freqkPatFound = true;
  while (setSize <= maxNoAtoms && freqkPatFound) {
    agg->retrieveLabelSubsets(setSize, labelSeq, labelSubsets);
    for (auto subset : labelSubsets) {
      if (agg->nonfreqSets[setSize].find(subset) == 
                                              agg->nonfreqSets[setSize].end()) {
        bool isSetFreq = agg->freqSets[setSize].find(subset) != 
                                                   agg->freqSets[setSize].end();
        if (!isSetFreq) {
          isSetFreq = agg->canLabelsBeFrequent(subset, commonTupleIds);
        }
        if (isSetFreq) {
          agg->freqSets[setSize].insert(subset);
          labelPerms.clear();
          do { // process all unchecked permutations of ~subset~
            if (agg->checkedSeqs[setSize].find(subset) == 
                                              agg->checkedSeqs[setSize].end()) {
              supp = agg->sequenceSupp(subset, commonTupleIds);
              if (supp >= minSupp) {
                for (unsigned int i = 0; i < subset.size(); i++) {
                  if (!agg->buildAtom(subset[i], agg->entries[subset[i]], 
                                                        commonTupleIds, atom)) {
                    cout << "Error in buildAtom for " << subset[i] << endl;
                    return;
                  }
                  pattern += atom + " ";
                }
                agg->results.push_back(NewPair<string, double>(pattern, supp));
                pattern.clear();
              }
              agg->checkedSeqs[setSize].insert(subset);
            }
          } while (std::next_permutation(subset.begin(), subset.end()));
        }
        else {
          agg->nonfreqSets[setSize].insert(subset);
        }
      }
    }
    freqkPatFound = (agg->results.size() > oldResultSize);
    oldResultSize = agg->results.size();
    setSize++;
  }
}

/*
  Class ~FPTree~, Function ~computeReducedCondBase~

*/
void FPTree::computeCondPatternBase(vector<unsigned int>& labelSeq, 
                 vector<NewPair<vector<unsigned int>, unsigned int> >& result) {
  result.clear();
  NewPair<vector<unsigned int>, unsigned int> labelPathWithSuppCnt;
  unsigned int link = nodeLinks[*(labelSeq.rbegin())];
  unsigned int anc, freq;
  while (link != 0) {
    anc = nodes[link].ancestor;
    freq = nodes[link].frequency;
    while (anc != 0) { // retrieve whole branch above ~label~ node
      labelPathWithSuppCnt.first.push_back(nodes[anc].label);
      anc = nodes[anc].ancestor;
    }
    labelPathWithSuppCnt.second = freq;
    if (!labelPathWithSuppCnt.first.empty()) {
      std::reverse(labelPathWithSuppCnt.first.begin(), 
                   labelPathWithSuppCnt.first.end());
      result.push_back(labelPathWithSuppCnt);
      labelPathWithSuppCnt.first.clear();
    }
    link = nodes[link].nodeLink;
  }
}

/*
  Class ~FPTree~, Function ~constructCondTree~

*/
FPTree* FPTree::constructCondTree(
                 vector<NewPair<vector<unsigned int>, unsigned int> >& condPB) {
  if (condPB.empty()) {
    return 0;
  }
  FPTree *condFPTree = new FPTree();
//   cout << "new tree created... " << endl;
  condFPTree->initialize(minSupp, agg);
  map<unsigned int, unsigned int> labelsToSuppCnt;
  map<unsigned int, unsigned int>::iterator mapIt;
  // build map: label --> suppCnt
  for (auto labelSeqWithSuppCnt : condPB) {
    for (auto label : labelSeqWithSuppCnt.first) {
      mapIt = labelsToSuppCnt.find(label);
      if (mapIt != labelsToSuppCnt.end()) { // label found; increase suppCnt
        mapIt->second += labelSeqWithSuppCnt.second;
      }
      else { // label not found; insert
        labelsToSuppCnt[label] = labelSeqWithSuppCnt.second;
      }
    }
  }
  // keep only labels having suppCnt >= minSuppCnt
  vector<NewPair<vector<unsigned int>, unsigned int> > freqCondPB;
  vector<unsigned int> labelSeq;
  for (auto labelSeqWithSuppCnt : condPB) {
    for (auto label : labelSeqWithSuppCnt.first) {
      if (labelsToSuppCnt[label] >= condFPTree->minSuppCnt) {
        labelSeq.push_back(label);
      }
    }
    if (!labelSeq.empty()) {
      freqCondPB.push_back(NewPair<vector<unsigned int>, unsigned int>(labelSeq,
                                                   labelSeqWithSuppCnt.second));
      labelSeq.clear();
    }
  }
  if (freqCondPB.empty()) {
    delete condFPTree;
    return 0;
  }
  for (auto it : freqCondPB) {
    condFPTree->insertLabelVector(it.first, it.second);
  }
//   cout << " ... filled, " << condFPTree->getNoNodes() << " nodes" << endl;
  return condFPTree;
}

/*
  Class ~FPTree~, Function ~mineTree~

*/
void FPTree::mineTree(vector<unsigned int>& initLabels, 
                 const unsigned int minNoAtoms, const unsigned int maxNoAtoms) {
  if (!hasNodes()) {
    return;
  }
  if (isOnePathTree()) {
//     cout << "  tree has ONE path, " << nodes.size() - 1 << " node(s) : <";
//     for (auto it : initLabels) {
//       cout << agg->freqLabels[it] << ", ";
//     }
//     cout << "| ";
    set<unsigned int> freqLabels(initLabels.begin(), initLabels.end());
    for (unsigned int i = 1; i < nodes.size(); i++) {
      freqLabels.insert(nodes[i].label);
//       cout << agg->freqLabels[nodes[i].label] << ", ";
    }
//     cout << ">" << endl;
    vector<unsigned int> labels(freqLabels.begin(), freqLabels.end());
    collectPatternsFromSeq(labels, minNoAtoms, maxNoAtoms);
//     cout << "  ... all patterns collected" << endl;
  }
  else { // tree has more than one path
//     cout << "tree has SEVERAL paths" << endl;
    vector<unsigned int> labelsSortedByFrequency, 
                   labelSeq(initLabels.begin(), initLabels.end());
    sortNodeLinks(labelsSortedByFrequency);
    vector<NewPair<vector<unsigned int>, unsigned int> > condPatBase;
    for (auto label : labelsSortedByFrequency) {
      labelSeq.push_back(label);
      if (labelSeq.size() > 1) {
        collectPatternsFromSeq(labelSeq, minNoAtoms, maxNoAtoms);
      }
      computeCondPatternBase(labelSeq, condPatBase);
//       cout << "rPB for " << agg->print(labelSeq) << " has " 
//            << condPatBase.size() << " elems: ";
//       for (auto it : condPatBase) {
//         cout << "-" << agg->print(it.first) << ",freq=" << it.second << endl;
//       }
      FPTree *condFPTree = constructCondTree(condPatBase);
      if (condFPTree != 0) {
//         Word fptval;
//         fptval.addr = condFPTree;
//         SecondoCatalog* sc = SecondoSystem::GetCatalog();
//         cout << nl->ToString(FPTree::Out(sc->NumericType(nl->SymbolAtom(
//                                           BasicType())), fptval)) << endl;
        condFPTree->mineTree(labelSeq, minNoAtoms, maxNoAtoms);
        delete condFPTree;
      }
      labelSeq.pop_back();
    }
  }
}

/*
  Class ~FPTree~, Function ~retrievePatterns~

*/
void FPTree::retrievePatterns(const unsigned int minNoAtoms, 
                              const unsigned int maxNoAtoms) {
  if (minNoAtoms == 1) {
    string pattern, atom;
    set<TupleId> commonTupleIds;
    vector<unsigned int> frequentLabels;
    double supp = 1.0;
    for (unsigned int l = 0; l < agg->entries.size(); l++) { // retrieve 1-pats
      agg->buildAtom(l, agg->entries[l], commonTupleIds, atom);
      supp = double(agg->entries[l].occs.size()) / agg->noTuples;
      agg->results.push_back(NewPair<string, double>(atom, supp));
      frequentLabels.push_back(l);
    }
    cout << frequentLabels.size() << " frequent 1-patterns found" << endl;
  }
  vector<unsigned int> initialLabels;
  agg->checkedSeqs.resize(maxNoAtoms + 1);
  agg->freqSets.resize(maxNoAtoms + 1);
  agg->nonfreqSets.resize(maxNoAtoms + 1);
  mineTree(initialLabels, minNoAtoms, maxNoAtoms);
  std::sort(agg->results.begin(), agg->results.end(), comparePMResults());
}

/*
  Class ~FPTree~, Function ~computeNodesSize~
  
  Compute storage space in bytes for ~Nodes~:
  
  noNodes + noNodes * (label + freq + noChildren + noChildren * child + nodeLink
          + anc)

*/
unsigned long long int FPTree::computeNodesSize() const {
  unsigned long long int result = sizeof(unsigned int);
  for (auto node : nodes) {
    result += (5 + node.children.size()) * sizeof(unsigned int);
  }
  return result;
}

/*
  Class ~FPTree~, Function ~computeNodeLinksSize~
  
  Compute storage space in bytes for ~NodeLinks~:
  
  noNL + noNL * (label + nodePos)

*/
unsigned long long int FPTree::computeNodeLinksSize() const {
  unsigned long long int result = sizeof(unsigned int) +
                                    nodeLinks.size() * 2 * sizeof(unsigned int);
  return result;
}

/*
  Class ~FPTree~, functions for secondo data type

*/
ListExpr FPTree::Property() {
  return (nl->TwoElemList(
    nl->FourElemList(
      nl->StringAtom("Signature"), nl->StringAtom("Example Type List"),
      nl->StringAtom("List Rep"), nl->StringAtom("Example List")),
    nl->FourElemList (
      nl->StringAtom("-> SIMPLE"), 
      nl->StringAtom(FPTree::BasicType()),
      nl->StringAtom("no list representation"),
      nl->StringAtom(""))));
}

Word FPTree::In(const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct) {
  correct = false;
  return SetWord(Address(0));
}

ListExpr FPTree::getNodeLinksList(unsigned int label) {
  unsigned int link = nodeLinks[label];
  ListExpr result = nl->OneElemList(nl->IntAtom(link));
  ListExpr nodeLinkList = result;
  link = nodes[link].nodeLink;
  while (link != 0) {
    nodeLinkList = nl->Append(nodeLinkList, nl->IntAtom(link));
    link = nodes[link].nodeLink;
  }
  return result;
}

ListExpr FPTree::Out(ListExpr typeInfo, Word value) {
  FPTree *tree = (FPTree*)value.addr;
  ListExpr nodesList(nl->Empty()), nodeList(nl->Empty()),
           nodeLinksList(nl->Empty()), nodeLinkList(nl->Empty()); 
//            relAggList(nl->Empty()), relAggsList(nl->Empty());
  ListExpr noTuplesList = nl->TwoElemList(nl->SymbolAtom("noTuples"),
                                          nl->IntAtom(tree->agg->noTuples));
  ListExpr minSuppList = nl->TwoElemList(nl->SymbolAtom("minSupp"),
                                         nl->RealAtom(tree->minSupp));
  if (tree->hasNodes()) {
    nodesList = nl->OneElemList(tree->nodes[0].toListExpr
                                                       (tree->agg->freqLabels));
    nodeList = nodesList;
  }
  for (unsigned int i = 1; i < tree->nodes.size(); i++) {
    nodeList = nl->Append(nodeList, tree->nodes[i].toListExpr
                                                       (tree->agg->freqLabels));
  }
  map<unsigned int, unsigned int>::iterator it = tree->nodeLinks.begin();
  if (tree->hasNodeLinks()) {
    nodeLinksList = nl->OneElemList(nl->TwoElemList(nl->SymbolAtom
        (tree->agg->freqLabels[it->first]), tree->getNodeLinksList(it->first)));
    nodeLinkList = nodeLinksList;
  }
  it++;
  while (it != tree->nodeLinks.end()) {
    nodeLinkList = nl->Append(nodeLinkList, 
               nl->TwoElemList(nl->SymbolAtom(tree->agg->freqLabels[it->first]),
                                            tree->getNodeLinksList(it->first)));
    it++;
  }
  return nl->FourElemList(noTuplesList, minSuppList, nodesList, nodeLinksList);
//   if (tree->hasAggEntries()) {
//     relAggsList = nl->OneElemList(nl->TwoElemList(
//                                  nl->SymbolAtom(tree->agg->entries[0].first),
//                                  tree->agg->entries[0].second.toListExpr()));
//     relAggList = relAggsList;
//   }
//   for (unsigned int i = 1; i < tree->agg->entries.size(); i++) {
//     relAggList = nl->Append(relAggList, nl->TwoElemList(
//                                  nl->SymbolAtom(tree->agg->entries[i].first),
//                                  tree->agg->entries[i].second.toListExpr()));
//   }
//   return nl->FiveElemList(noTuplesList, minSuppList, nodesList,nodeLinksList,
//                           relAggsList);
}

Word FPTree::Create(const ListExpr typeInfo) {
  Word w;
  w.addr = (new FPTree());
  return w;
}

void FPTree::Delete(const ListExpr typeInfo, Word& w) {
  FPTree *tree = (FPTree*)w.addr;
  delete tree;
  w.addr = 0;
}

bool FPTree::Save(SmiRecord& valueRecord, size_t& offset,
                  const ListExpr typeInfo, Word& value) {
  FPTree *tree = (FPTree*)value.addr;
  // store minSupp
  if (!valueRecord.Write(&tree->minSupp, sizeof(double), offset)) {
    return false;
  }
  offset += sizeof(double);
  // store noNodes
  unsigned int noNodes = tree->getNoNodes();
  if (!valueRecord.Write(&noNodes, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  // store nodes
  string label;
  unsigned int labelLength, numLabel, frequency, noChildren, child, nodeLink, 
               noOccs, ancestor, tid, noComponents;
  double durD, start, end;
  SecInterval iv(true);
  for (unsigned int i = 0; i < noNodes; i++) { // store nodes
    numLabel = tree->nodes[i].label;
    if (!valueRecord.Write(&numLabel, sizeof(unsigned int), offset)) {
      return false;
    }
//     if (numLabel != UINT_MAX) {
//       cout << "node with label " << numLabel << " <--> " 
//            << tree->agg->freqLabels[numLabel] << " written" << endl;
//     }
    offset += sizeof(unsigned int);
    frequency = tree->nodes[i].frequency;
    if (!valueRecord.Write(&frequency, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    noChildren = tree->nodes[i].children.size();
    if (!valueRecord.Write(&noChildren, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    for (unsigned int j = 0; j < tree->nodes[i].children.size(); j++) {//childr.
      child = tree->nodes[i].children[j];
      if (!valueRecord.Write(&child, sizeof(unsigned int), offset)) {
        return false;
      }
      offset += sizeof(unsigned int);
    }
    nodeLink = tree->nodes[i].nodeLink;
    if (!valueRecord.Write(&nodeLink, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    ancestor = tree->nodes[i].ancestor;
    if (!valueRecord.Write(&ancestor, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
  }
  // store noNodeLinks
  unsigned int noNodeLinks = tree->getNoNodeLinks();
  if (!valueRecord.Write(&noNodeLinks, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  // store nodeLinks
  for (map<unsigned int, unsigned int>::iterator it = tree->nodeLinks.begin();
                         it != tree->nodeLinks.end(); it++) { // store nodeLinks
    numLabel = it->first;
    if (!valueRecord.Write(&numLabel, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    nodeLink = it->second;
    if (!valueRecord.Write(&nodeLink, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
  }
  
  // store ~noTuples~, ~entries~ and ~freqLabels~ from relAgg
  if (!valueRecord.Write(&tree->agg->noTuples, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  unsigned long long int entriesSize = tree->agg->computeEntriesSize();
  cout << "size of entries is " << entriesSize << endl;
  if (!valueRecord.Write(&entriesSize, sizeof(unsigned long long int), offset)){
    return false;
  }
  offset += sizeof(unsigned long long int);
  unsigned int noAggEntries = tree->agg->entries.size();
  if (!valueRecord.Write(&noAggEntries, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  for (unsigned int i = 0; i < noAggEntries; i++) {
    if (!valueRecord.Write(&tree->agg->entries[i].noOccs, sizeof(unsigned int), 
                                                                      offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    durD = tree->agg->entries[i].duration.ToDouble();
    if (!valueRecord.Write(&durD, sizeof(double), offset)) {
      return false;
    }
    offset += sizeof(double);
    noOccs = tree->agg->entries[i].occs.size();
    if (!valueRecord.Write(&noOccs, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    for (auto occ : tree->agg->entries[i].occs) {
      tid = get<0>(occ);
      if (!valueRecord.Write(&tid, sizeof(unsigned int), offset)) {
        return false;
      }
      offset += sizeof(unsigned int);
      noComponents = get<1>(occ)->GetNoComponents();
      if (!valueRecord.Write(&noComponents, sizeof(unsigned int), offset)) {
        return false;
      }
      offset += sizeof(unsigned int);
      for (unsigned int j = 0; j < noComponents; j++) {
        get<1>(occ)->Get(j, iv);
        start = iv.start.ToDouble();
        end = iv.end.ToDouble();
        if (!valueRecord.Write(&start, sizeof(double), offset)) {
          return false;
        }
        offset += sizeof(double);
        if (!valueRecord.Write(&iv.lc, sizeof(bool), offset)) {
          return false;
        }
        offset += sizeof(bool);
        if (!valueRecord.Write(&end, sizeof(double), offset)) {
          return false;
        }
        offset += sizeof(double);
        if (!valueRecord.Write(&iv.rc, sizeof(bool), offset)) {
          return false;
        }
        offset += sizeof(bool);
      }
      double coords[] = {get<2>(occ).MinD(0), get<2>(occ).MinD(1),
                         get<2>(occ).MaxD(0), get<2>(occ).MaxD(1)};
      for (int c = 0; c < 4; c++) {
        if (!valueRecord.Write(&coords[c], sizeof(double), offset)) {
          return false;
        }
        offset += sizeof(double);
      }
      bool isdefined = get<2>(occ).IsDefined();
      if (!valueRecord.Write(&isdefined, sizeof(bool), offset)) {
        return false;
      }
      offset += sizeof(bool);
    }
    for (auto occPos : tree->agg->entries[i].occsPos) {
      tid = occPos;
      if (!valueRecord.Write(&tid, sizeof(unsigned int), offset)) {
        return false;
      }
      offset += sizeof(unsigned int);
    }
  }
  for (unsigned int i = 0; i < noAggEntries; i++) {
    label = tree->agg->freqLabels[i];
    labelLength = label.length();
    if (!valueRecord.Write(&labelLength, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    char labelArray[labelLength + 1];
    strcpy(labelArray, label.c_str());
    if (!valueRecord.Write(&labelArray, labelLength + 1, offset)) {
      return false;
    }
    offset += labelLength + 1;
  }
  return true;
}

bool FPTree::Open(SmiRecord& valueRecord, size_t& offset,
                  const ListExpr typeInfo, Word& value) {
  FPTree *tree = new FPTree();
  // read minSupp
  if (!valueRecord.Read(&tree->minSupp, sizeof(double), offset)) {
    return false;
  }
  offset += sizeof(double);
  double durD, start, end;
  Periods *per;
  unsigned int labelLength, numLabel, noNodes, frequency, noChildren, child, 
       nodeLink, ancestor, noNodeLinks, noOccs, noAggEntries, occPos,
       noComponents;
  SecInterval iv(true);
  iv.start.SetType(instanttype);
  iv.end.SetType(instanttype);
  // read nodes
  if (!valueRecord.Read(&noNodes, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  for (unsigned int i = 0; i < noNodes; i++) { // read nodes
    if (!valueRecord.Read(&numLabel, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    if (!valueRecord.Read(&frequency, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    if (!valueRecord.Read(&noChildren, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    vector<unsigned int> children;
    for (unsigned int j = 0; j < noChildren; j++) {
      if (!valueRecord.Read(&child, sizeof(unsigned int), offset)) {
        return false;
      }
      offset += sizeof(unsigned int);
      children.push_back(child);
    }
    if (!valueRecord.Read(&nodeLink, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    if (!valueRecord.Read(&ancestor, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
//     cout << "create node: " << label << ", " << frequency << ", " 
//          << children.size() << ", " << nodeLink << ", " << ancestor << endl;
    FPNode node(numLabel, frequency, children, nodeLink, ancestor);
    tree->nodes.push_back(node);
  }
  // read nodeLinks
  if (!valueRecord.Read(&noNodeLinks, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  for (unsigned int i = 0; i < noNodeLinks; i++) {
    if (!valueRecord.Read(&numLabel, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    if (!valueRecord.Read(&nodeLink, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
//     cout << "create nodeLink: " << label << " --> " << nodeLink << endl;
    tree->nodeLinks.insert(tree->nodeLinks.begin(), 
                           make_pair(numLabel, nodeLink));
  }
  tree->agg = new RelAgg();
  // read ~noTuples~
  if (!valueRecord.Read(&tree->agg->noTuples, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  unsigned long long int entriesSize;
  if (!valueRecord.Read(&entriesSize, sizeof(unsigned long long int), offset)) {
    return false;
  }
  offset += sizeof(unsigned long long int);
  cout << "size of entries is " << entriesSize << endl;
  char* entriesChars = new char[entriesSize];
  // read ~entries~
  if (!valueRecord.Read(&noAggEntries, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  if (!valueRecord.Read(entriesChars, entriesSize, offset)) {
    return false;
  }
  offset += entriesSize;
  size_t offsetEntries = 0;
  for (unsigned int i = 0; i < noAggEntries; i++) {
//     auto measureStart = high_resolution_clock::now(); 
    AggEntry entry;
    memcpy(&entry.noOccs, entriesChars + offsetEntries, sizeof(unsigned int));
    offsetEntries += sizeof(unsigned int);
//     cout << "noOccs = " << entry.noOccs << endl;
    memcpy(&durD, entriesChars + offsetEntries, sizeof(double));
    offsetEntries += sizeof(double);
//     offsetEntries += sizeof(double);
    entry.duration.ReadFrom(durD);
//     cout << "duration is " << entry.duration << endl;
    memcpy(&noOccs, entriesChars + offsetEntries, sizeof(unsigned int));
    offsetEntries += sizeof(unsigned int);
    TupleId tid;
    for (unsigned int j = 0; j < noOccs; j++) {
      memcpy(&tid, entriesChars + offsetEntries, sizeof(unsigned int));
      offsetEntries += sizeof(unsigned int);
      memcpy(&noComponents, entriesChars + offsetEntries, sizeof(unsigned int));
      offsetEntries += sizeof(unsigned int);
      per = new Periods(true);
      for (unsigned int k = 0; k < noComponents; k++) {
        memcpy(&start, entriesChars + offsetEntries, sizeof(double));
        offsetEntries += sizeof(double);
        iv.start.ReadFrom(start);
        memcpy(&iv.lc, entriesChars + offsetEntries, sizeof(bool));
        offsetEntries += sizeof(bool);
        memcpy(&end, entriesChars + offsetEntries, sizeof(double));
        offsetEntries += sizeof(double);
        iv.end.ReadFrom(end);
        memcpy(&iv.rc, entriesChars + offsetEntries, sizeof(bool));
        offsetEntries += sizeof(bool);
        per->MergeAdd(iv);
      }
      double *min = new double[2];
      for (int c = 0; c < 2; c++) {
        memcpy(&min[c], entriesChars + offsetEntries, sizeof(double));
        offsetEntries += sizeof(double);
      }
      double *max = new double[2];
      for (int c = 0; c < 2; c++) {
        memcpy(&max[c], entriesChars + offsetEntries, sizeof(double));
        offsetEntries += sizeof(double);
      }
      bool isdefined;
      memcpy(&isdefined, entriesChars + offsetEntries, sizeof(bool));
      offsetEntries += sizeof(bool);
      Rect rect(isdefined, min, max);
      delete[] min;
      delete[] max;
//       cout << "ENTRY: " << tid << " " << *per << rect << endl;
      entry.occs.push_back(make_tuple(tid, per, rect));
    }
    for (unsigned int j = 0; j <= tree->agg->noTuples; j++) {
      memcpy(&occPos, entriesChars + offsetEntries, sizeof(unsigned int));
      offsetEntries += sizeof(unsigned int);
      entry.occsPos.push_back(occPos);
//       cout << "... pushed back occPos " << occPos << endl;
    }
    tree->agg->entries.push_back(entry);
//     auto measureStop = high_resolution_clock::now();
//     double ms = 
//    (double)(duration_cast<milliseconds>(measureStop - measureStart).count());
//     cout << "entry with " << entry.occs.size() << " occs read, took " << ms
//          << " ms, equals " << ms / entry.noOccs << " ms per occ" << endl;
  }
  delete[] entriesChars;
  // read ~freqLabels~
  for (unsigned int i = 0; i < noAggEntries; i++) {
    if (!valueRecord.Read(&labelLength, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    char labelArray[labelLength + 1];
    if (!valueRecord.Read(&labelArray, labelLength + 1, offset)) {
      return false;
    }
    offset += labelLength + 1;
    string label(labelArray);
    tree->agg->freqLabels.push_back(label);
  }
  value.setAddr(tree);
  return true;
}

void FPTree::Close(const ListExpr typeInfo, Word& w) {
  FPTree *tree = (FPTree*)w.addr;
  delete tree;
  w.addr = 0;
}

Word FPTree::Clone(const ListExpr typeInfo, const Word& w) {
  FPTree *tree = (FPTree*)w.addr;
  Word res;
  res.addr = new FPTree(*tree);
  return res;
}

int FPTree::SizeOfObj() {
  return sizeof(FPTree);
}

bool FPTree::TypeCheck(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, BasicType());
}


/*
  Type constructor for secondo type ~fptree~
 
*/

TypeConstructor fptreeTC(
  FPTree::BasicType(),
  FPTree::Property,
  FPTree::Out,
  FPTree::In,
  0, 0,
  FPTree::Create,
  FPTree::Delete,
  FPTree::Open,
  FPTree::Save,
  FPTree::Close,
  FPTree::Clone,
  0,
  FPTree::SizeOfObj,
  FPTree::TypeCheck);

GetPatternsLI::GetPatternsLI(Relation *r, const NewPair<int, int> ap, double ms,
                             int mina, int maxa, Geoid *g, const size_t mem) {
  tupleType = getTupleType();
  agg.clear();
  agg.scanRelation(r, ap, g);
  agg.filter(ms, mem);
  agg.derivePatterns(mina, maxa);
}

GetPatternsLI::~GetPatternsLI() {
  tupleType->DeleteIfAllowed();
}

TupleType* GetPatternsLI::getTupleType() {
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

Tuple* GetPatternsLI::getNextResult(RelAgg& agg, TupleType *tt) {
  if (agg.results.empty()) {
    return 0;
  }
  Tuple *tuple = new Tuple(tt);
  NewPair<string, double> result;
  result = agg.results.back();
  agg.results.pop_back();
  FText *pattern = new FText(true, result.first);
  tuple->PutAttribute(0, pattern);
  CcReal *support = new CcReal(true, result.second);
  tuple->PutAttribute(1, support);
  return tuple;
}

MineFPTreeLI::MineFPTreeLI(FPTree *t, int mina, int maxa) : 
                                   tree(t), minNoAtoms(mina), maxNoAtoms(maxa) {
  tupleType = GetPatternsLI::getTupleType();
  tree->retrievePatterns(minNoAtoms, maxNoAtoms);
}

MineFPTreeLI::~MineFPTreeLI() {
//   tree->agg->clearEntries();
  delete tree->agg;
  tupleType->DeleteIfAllowed();
}

}
