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
  
AggEntry::AggEntry(const TupleId id, const temporalalgebra::SecInterval& iv,
                   Rect& rect) {
  occurrences.clear();
  Periods *per = new Periods(1);
  per->Add(iv);
  occurrences[id].first = per;
  occurrences[id].second = rect;
  noOccurrences = 0;
  duration.SetType(datetime::durationtype);
  duration.ReadFrom((int64_t)0); // durations are computed at the end
}

void AggEntry::clear() {
  for (auto it : occurrences) {
    it.second.first->DeleteIfAllowed();
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
  return it->second.first->GetNoComponents();
}

void AggEntry::computeCommonTimeInterval(const set<TupleId>& commonTupleIds,
                                         SecInterval& iv) {
  iv.start.SetType(instanttype);
  iv.end.SetType(instanttype);
  iv.start.ToMaximum();
  iv.end.ToMinimum();
  Instant first(1.0), last(1.0);
  if (commonTupleIds.empty()) { // use all occurrences
    for (auto it : occurrences) {
      if (it.second.first->IsDefined()) {
        if (!it.second.first->IsEmpty()) {
          it.second.first->Minimum(first);
          it.second.first->Maximum(last);
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
      if (occurrences.find(it) != occurrences.end()) {
        Periods* per = occurrences[it].first;
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
    if (occurrences.empty()) {
      return;
    }
    map<TupleId, NewPair<Periods*, Rect> >::iterator it = occurrences.begin();
    Rect tempRect = it->second.second;
    it++;
    while (it != occurrences.end()) {
      tempRect = it->second.second.Union(tempRect);
      it++;
    }
    rect = tempRect;
  }
  else {
    set<TupleId>::iterator it = commonTupleIds.begin();
    Rect tempRect = occurrences[*it].second;
    it++;
    while (it != commonTupleIds.end()) {
      tempRect = occurrences[*it].second.Union(tempRect);
      it++;
    }
    rect = tempRect;
  }
}

void AggEntry::computeSemanticTimeSpec(string& semanticTimeSpec) const {
  semanticTimeSpec.clear();
  int month = 1; // {1, ..., 12}
  int weekday = 0; // {0, ..., 6}
  int daytime = 0; // {0, ..., 3}
  Instant first(1.0), last(1.0);
  for (map<TupleId, NewPair<Periods*, Rect> >::const_iterator it = 
       occurrences.begin(); it != occurrences.end(); it++) {
    if (it->second.first->IsDefined()) {
      if (!it->second.first->IsEmpty()) {
        it->second.first->Minimum(first);
        it->second.first->Maximum(last);
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
      result << "   TID " << it.first << ": " 
             << it.second.first->GetNoComponents() 
             << " occurrences:  " << *(it.second.first) << it.second.second
             << endl;
    }
  }
  else {
    auto it = occurrences.find(id);
    if (it == occurrences.end()) { // id not found
      result << "   TID " << id << " not found" << endl;
    }
    else {
      result << "   TID " << id << ": " << *(it->second.first) << endl;
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
  Class ~FPNode~, Function ~toListExpr~

*/
ListExpr FPNode::toListExpr() const {
  ListExpr childrenList = nl->Empty();
  if (!children.empty()) {
    childrenList = nl->OneElemList(nl->IntAtom(children[0]));
  }
  ListExpr childList = childrenList;
  for (unsigned int i = 1; i < children.size(); i++) {
    childList = nl->Append(childList, nl->IntAtom(children[i]));
  }
  ListExpr result = nl->FourElemList(nl->SymbolAtom(label), 
                   nl->IntAtom(frequency), childrenList, nl->IntAtom(nodeLink));
  return result;
}

/*
  Class ~FPTree~, Function ~isChildOf~

*/
bool FPTree::isChildOf(string& label, unsigned int pos, unsigned int& nextPos) {
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
void FPTree::updateNodeLink(string& label, unsigned int targetPos) {
  map<string, unsigned int>::iterator it = nodeLinks.find(label);
  if (it == nodeLinks.end()) { // no existing node link
    nodeLinks.insert(make_pair(label, targetPos));
  }
  else { // node link for label exists
    unsigned int link = it->second;
    unsigned int currentPos = 0;
    while (link != UINT_MAX) { // find end of node link
      currentPos = link;
      link = nodes[link].nodeLink;
    }
    nodes[currentPos].nodeLink = targetPos;
  }
}

/*
  Class ~FPTree~, Function ~insertLabelVector~

*/
void FPTree::insertLabelVector(const vector<string>& labelsOrdered) {
  cout << "insert: | ";
  for (auto it : labelsOrdered) {
    cout << it << " | ";
  }
  cout << endl;
  unsigned int nodePos(0), nextPos(0);
  for (auto it : labelsOrdered) {
    if (isChildOf(it, nodePos, nextPos)) {
      nodes[nextPos].frequency++;
      cout << "  \"" << it << "\" is child of \"" << nodes[nodePos].label 
           << "\", frequency = " << nodes[nextPos].frequency << endl;
      nodePos = nextPos;
    }
    else {
      FPNode node(it);
      nodes.push_back(node);
      nodes[nodePos].children.push_back(nodes.size() - 1);
      updateNodeLink(it, nodes.size() - 1);
      cout << "  new node for \"" << it << "\" at pos " << nodes.size() - 1
           << ", now child of " << nodePos << endl;
      nodePos = nodes.size() - 1;
    }
  }
  cout << ">>> SUCCESSFULLY inserted" << endl;
}

/*
  Class ~FPTree~, Function ~construct~

*/
void FPTree::construct(RelAgg *agg) {
  GenericRelationIterator* it = agg->rel->MakeScan();
  MLabel *ml = 0;
  Tuple *tuple = 0;
  string label;
  set<NewPair<string, double>, compareLabelsWithSupp> labelsWithSupp;
  vector<string> labelsOrdered;
  while ((tuple = it->GetNextTuple())) {
    labelsWithSupp.clear();
    labelsOrdered.clear();
    ml = (MLabel*)(tuple->GetAttribute(agg->attrPos.first));
    for (int j = 0; j < ml->GetNoComponents(); j++) {
      ml->GetValue(j, label);
      NewPair<string, double> labelWithSupp(label, 
                                            agg->getSuppForFreqLabel(label));
      if (labelWithSupp.second >= agg->minSupp) {
        labelsWithSupp.insert(labelWithSupp);
      }
    }
    for (auto it : labelsWithSupp) {
      labelsOrdered.push_back(it.first);
    }
    insertLabelVector(labelsOrdered);
    tuple->DeleteIfAllowed();
  }
  delete it;
}

/*
  Class ~FPTree~, Function ~initialize~

*/
void FPTree::initialize() {
  FPNode node("<ROOT>");
  node.frequency = 0;
  nodes.push_back(node); // create dummy node for root
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

ListExpr FPTree::getNodeLinksList(string label) {
  unsigned int link = nodeLinks[label];
  ListExpr result = nl->OneElemList(nl->IntAtom(link));
  ListExpr nodeLinkList = result;
  link = nodes[link].nodeLink;
  while (link != UINT_MAX) {
    nodeLinkList = nl->Append(nodeLinkList, nl->IntAtom(link));
    link = nodes[link].nodeLink;
  }
  return result;
}

ListExpr FPTree::Out(ListExpr typeInfo, Word value) {
  FPTree *tree = (FPTree*)value.addr;
  ListExpr nodesList(nl->Empty()), nodeList, nodeLinksList(nl->Empty()),
           nodeLinkList;
  if (tree->hasNodes()) {
    nodesList = nl->OneElemList(tree->nodes[0].toListExpr());
    nodeList = nodesList;
  }
  for (unsigned int i = 1; i < tree->nodes.size(); i++) {
    nodeList = nl->Append(nodeList, tree->nodes[i].toListExpr());
  }
  map<string, unsigned int>::iterator it = tree->nodeLinks.begin();
  if (tree->hasNodeLinks()) {
    nodeLinksList = nl->OneElemList(nl->TwoElemList(nl->SymbolAtom(it->first),
                                            tree->getNodeLinksList(it->first)));
    nodeLinkList = nodeLinksList;
  }
  it++;
  while (it != tree->nodeLinks.end()) {
    nodeLinkList = nl->Append(nodeLinkList, 
                              nl->TwoElemList(nl->SymbolAtom(it->first), 
                                            tree->getNodeLinksList(it->first)));
    it++;
  }
  return nl->TwoElemList(nodesList, nodeLinksList);
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
  unsigned int noNodes = tree->getNoNodes();
  if (!valueRecord.Write(&noNodes, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  string label;
  unsigned int labelLength, frequency, noChildren, child, nodeLink;
  for (unsigned int i = 0; i < noNodes; i++) { // store nodes
    label = tree->nodes[i].label;
    labelLength = label.length();
    if (!valueRecord.Write(&labelLength, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    if (!valueRecord.Write(&label, labelLength, offset)) {
      return false;
    }
    offset += labelLength;
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
  }
  unsigned int noNodeLinks = tree->getNoNodeLinks();
  if (!valueRecord.Write(&noNodeLinks, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  for (map<string, unsigned int>::iterator it = tree->nodeLinks.begin();
       it != tree->nodeLinks.end(); it++) { // store nodeLinks
    label = it->first;
    labelLength = label.length();
    if (!valueRecord.Write(&labelLength, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += labelLength;
    if (!valueRecord.Write(&label, sizeof(label), offset)) {
      return false;
    }
    offset += sizeof(label);
    nodeLink = it->second;
    if (!valueRecord.Write(&nodeLink, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
  }
  return true;
}

bool FPTree::Open(SmiRecord& valueRecord, size_t& offset,
                  const ListExpr typeInfo, Word& value) {
  FPTree *tree = new FPTree();
  string label;
  unsigned int labelLength, noNodes, frequency, noChildren, child, nodeLink,
               noNodeLinks;
  if (!valueRecord.Read(&noNodes, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  for (unsigned int i = 0; i < noNodes; i++) { // read nodes
    if (!valueRecord.Read(&labelLength, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    if (!valueRecord.Read(&label, labelLength, offset)) {
      return false;
    }
    offset += labelLength;
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
    cout << "create node: " << label << ", " << frequency << ", " 
         << children.size() << ", " << nodeLink << endl;
    FPNode node(label, frequency, children, nodeLink);
    tree->nodes.push_back(node);
  }
  if (!valueRecord.Read(&noNodeLinks, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  for (unsigned int i = 0; i < noNodeLinks; i++) {
    if (!valueRecord.Read(&labelLength, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    if (!valueRecord.Read(&label, labelLength, offset)) {
      return false;
    }
    offset += labelLength;
    if (!valueRecord.Read(&nodeLink, sizeof(unsigned int), offset)) {
      return false;
    }
    offset += sizeof(unsigned int);
    tree->nodeLinks.insert(tree->nodeLinks.begin(), make_pair(label, nodeLink));
  }
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

/*
  Class ~RelAgg~, Function ~clear~
  
  Deletes the periods values and, maybe, the inverted file
 
*/

void RelAgg::clear(const bool deleteInv) {
  for (auto it : entriesMap) {
    it.second.clear();
  }
  if (deleteInv) {
    delete inv;
  }
}

/*
  Class ~RelAgg~, Function ~initialize~
 
*/
void RelAgg::initializeInv() {
  inv = new InvertedFile();
  inv->setParams(false, 1, "");
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
    AggEntry entry(id, iv, rect);
    entriesMap.insert(make_pair(label, entry));
  }
  else { // label already present
    auto entryIt = aggIt->second.occurrences.find(id);
    if (entryIt == aggIt->second.occurrences.end()) { // new id for label
      Periods *per = new Periods(1);
      per->Add(iv);
      entriesMap[label].occurrences.insert(
                             make_pair(id, NewPair<Periods*, Rect>(per, rect)));
    }
    else { // id already present for label
      entriesMap[label].occurrences[id].first->MergeAdd(iv);
      if (rect.IsDefined()) {
        entriesMap[label].occurrences[id].second = 
                           entriesMap[label].occurrences[id].second.Union(rect);
      }
    }
  }
  entriesMap[label].noOccurrences++;
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
  size_t maxMem = memSize * 16 * 1024 * 1024;
  size_t trieCacheSize = maxMem / 20;
  if (trieCacheSize < 4096) {
    trieCacheSize = 4096;
  }
  size_t invCacheSize;
  if (trieCacheSize + 4096 > maxMem) {
    invCacheSize = 4096;
  }
  else {
    invCacheSize = maxMem - trieCacheSize;
  }
  appendcache::RecordAppendCache* cache = inv->createAppendCache(invCacheSize);
  TrieNodeCacheType* trieCache = inv->createTrieCache(trieCacheSize);
  for (auto it : entriesMap) {
    supp = double(it.second.occurrences.size()) / noTuples;
    if (supp >= minSupp) {
      inv->insertString(entries.size(), it.first, 0, 0, cache, trieCache);
//       cout << "INSERTED: \"" << it.first << "\", POS " << entries.size()
//            << " " << it.second.print() << endl;
      entries.push_back(it);
    }
  }
  delete trieCache;
  delete cache;
}

/*
  Class ~RelAgg~, Function ~buildAtom~
  
  Build a string representing a pattern atom from an entry of ~contents~ and
  compute its support
 
*/

bool RelAgg::buildAtom(string label, AggEntry entry,
                       const set<TupleId>& commonTupleIds, string& atom) {
//   cout << "bA: \"" << label << "\", " << entry.occurrences.size() << " occs" 
//        << endl;
  SecInterval iv(true);
  string timeSpec, semanticTimeSpec;
  entry.computeCommonTimeInterval(commonTupleIds, iv);
  entry.computeSemanticTimeSpec(semanticTimeSpec);
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
  atom = "(" + timeSpec + " \"" + label + "\" " + entry.print(rect) + ")";
  return true;
}

/*
  Class ~RelAgg~, Function ~retrieveLabelSets~
  
  Computes all label combinations of size ~size~ for a tuple
  
*/
void RelAgg::retrieveLabelCombs(const unsigned int size, vector<string>& source,
                                set<vector<string > >& result) {
  result.clear();
  vector<string> labelVec;
  Tools::subset(source, size, 0, labelVec, result);
}

/*
  Class ~RelAgg~, Function ~getLabelEntry~
  
  Retrieve the corresponding AggEntry instance for a label, using ~inv~
  
*/

AggEntry* RelAgg::getLabelEntry(string label) {
  uint32_t pos, wc, cc;
  InvertedFile::exactIterator* eit = inv->getExactIterator(label, 16777216);
  if (eit->next(pos, wc, cc)) {
    if (pos >= entries.size()) {
      cout << "pos " << pos << " does not exist; entries.size = " 
           << entries.size() << endl;
    }
    else {
      delete eit;
//       cout << "FOUND label " << label << ", entry: " 
//            << entries[pos].second.print() << endl;
      return &(entries[pos].second);
    }
  }
  delete eit;
//   cout << "NOTHING FOUND for label \"" << label << "\"" << endl;
  return 0;
}

double RelAgg::getSuppForFreqLabel(string& label) {
  AggEntry *entry = getLabelEntry(label);
//   cout << "|" << label << "|" << double(entry->occurrences.size()) / noTuples
//        << endl;
  if (entry == 0) {
    return 0;
  }
  return double(entry->occurrences.size()) / noTuples;
}

/*
  Class ~RelAgg~, Function ~canIntersectionBeFrequent~
  
  Computes the fraction of tuples in which all strings of ~labelSeq~ occur
  
*/
bool RelAgg::canLabelsBeFrequent(vector<string>& labelSeq,
                                 set<TupleId>& intersection) {
  intersection.clear();
  if (labelSeq.size() < 2) {
    cout << "sequence has only " << labelSeq.size() << " component(s)" << endl;
    return false;
  }
  set<TupleId> intersection_temp;
  vector<set<TupleId> > occs;
  occs.resize(labelSeq.size());
  AggEntry *entry = 0;
  // retrieve occurrences for every label
//   cout << "check sequence " << print(labelSeq) << endl;
  for (unsigned int pos = 0; pos < labelSeq.size(); pos++) {
    entry = getLabelEntry(labelSeq[pos]);
    for (auto occ : entry->occurrences) {
      occs[pos].insert(occ.first);
    }
  }
  // compute intersection of all id sets
  set_intersection(occs[0].begin(), occs[0].end(), occs[1].begin(),    
                   occs[1].end(), inserter(intersection, intersection.begin()));
  for (unsigned int pos = 2; pos < labelSeq.size(); pos++) {
    set_intersection(intersection.begin(), intersection.end(), 
                     occs[pos].begin(), occs[pos].end(), 
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

double RelAgg::sequenceSupp(vector<string> labelSeq, 
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
      entry = getLabelEntry(labelSeq[pos]);
      if (entry->occurrences.find(id) != entry->occurrences.end()) {
        entry->occurrences[id].first->Maximum(end);
        if (start < end) { // label found, correct order
          // set start instant to begin of periods for current label
          entry->occurrences[id].first->Minimum(start);
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

void RelAgg::combineApriori(set<vector<string > >& frequentLabelCombs,
                            set<vector<string > >& labelCombs) {
  if (frequentLabelCombs.empty()) {
    return;
  }
  if ((frequentLabelCombs.begin())->empty()) {
    return;
  }
  unsigned int k = (frequentLabelCombs.begin())->size();
  set<vector<string > >::iterator it2;
  set<string> union_k_inc;
  for (set<vector<string > >::iterator it1 = frequentLabelCombs.begin();
       it1 != frequentLabelCombs.end(); it1++) {
    it2 = it1;
    it2++;
    while (it2 != frequentLabelCombs.end()) {
      set_union(it1->begin(), it1->end(), it2->begin(), it2->end(), 
                inserter(union_k_inc, union_k_inc.begin()));
      if (union_k_inc.size() == k+1) {
//         cout << print(*it1) << " and " << print(*it2) << " united to "
//              << print(union_k_inc) << endl;
        vector<string> unionvec(union_k_inc.begin(), union_k_inc.end());
        labelCombs.insert(labelCombs.end(), unionvec);
      }
      it2++;
    }
    union_k_inc.clear();
  }
}

void RelAgg::retrievePermutations(vector<string>& labelComb,
                                  set<vector<string > >& labelPerms) {
  labelPerms.clear();
  vector<string> labels = labelComb;
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
  vector<string> frequentLabels;
  double supp = 1.0;
  for (auto it : entries) {
    buildAtom(it.first, it.second, commonTupleIds, atom);
    if (minNoAtoms == 1) {
      supp = double(it.second.occurrences.size()) / noTuples;
      results.push_back(NewPair<string, double>(atom, supp));
    }
    frequentLabels.push_back(it.first);
  }
  cout << frequentLabels.size() << " frequent 1-patterns found" << endl;
  // retrieve patterns with two atoms
  if (maxNoAtoms < 2) {
    return;
  }
  string label;
  set<vector<string > > labelCombs, frequentLabelCombs, labelPerms;
  SecInterval iv(true);
  AggEntry *entry;
  // scan ~contents~; only atoms whose corresponding 1-patterns fulfill
  // ~minSupp~ can be part of a frequent 2-pattern
  retrieveLabelCombs(2, frequentLabels, labelCombs);
  // check all combinations for their support
  bool correct = false;
  for (auto labelComb : labelCombs) {
    if (canLabelsBeFrequent(labelComb, commonTupleIds)) {
      supp = sequenceSupp(labelComb, commonTupleIds);
      if (supp >= minSupp) {
        if (minNoAtoms <= 2) {
          pattern.clear();
          // build complete 2-pattern
          entry = getLabelEntry(labelComb[0]);
          correct = buildAtom(labelComb[0], *entry, commonTupleIds, atom);
          pattern += atom + " ";
          entry = getLabelEntry(labelComb[1]);
          correct = correct && buildAtom(labelComb[1], *entry, commonTupleIds, 
                                         atom);
          pattern += atom;
          if (correct) {
            results.push_back(NewPair<string, double>(pattern, supp));
          }
        }
        frequentLabelCombs.insert(frequentLabelCombs.end(), labelComb);
      }
    }
  }
  cout << frequentLabelCombs.size() << " frequent 2-patterns found" << endl;
  // retrieve patterns with three or more atoms
  unsigned int k = 3;
  while (k <= maxNoAtoms && !frequentLabelCombs.empty()) { // no frequent k-pat
    map<vector<string>, set<TupleId> > labelPermsWithCommonIds;
    labelCombs.clear();
    combineApriori(frequentLabelCombs, labelCombs);
    frequentLabelCombs.clear();
    for (auto labelComb : labelCombs) {
      if (canLabelsBeFrequent(labelComb, commonTupleIds)) {
  //         cout << print(labelComb) << " can be frequent; occurs in " 
  //              << print(commonTupleIds) << endl;
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
            entry = getLabelEntry(it.first[pos]);
            correct = correct && buildAtom(it.first[pos], *entry, it.second, 
                                           atom);
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
  std::sort(results.begin(), results.end(), comparePatternMiningResults());
}

string RelAgg::print(const map<string, AggEntry>& entriesMap) const {
  stringstream result;
  for (auto it : entriesMap) {
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

string RelAgg::print(const vector<string>& labelComb) const {
  stringstream result;
  bool first = true;
  result << "{";
  for (auto it : labelComb) {
    if (!first) {
      result << ", ";
    }
    first = false;
    result << "\"" << it << "\"";
  }
  result << "}" << endl;
  return result.str();
}

string RelAgg::print(const set<vector<string> >& labelCombs) const {
  stringstream result;
  result << "{" << endl;
  for (auto it : labelCombs) {
    result << "  " << print(it);
  }
  result << "}" << endl;
  return result.str();
}

string RelAgg::print(const string& label /* = "" */) {
  stringstream result;
  if (label == "") { // print everything
    for (auto it : entriesMap) {
      result << "\"" << it.first << "\" :" << it.second.print() << endl
             << "-----------------------------------------------------" << endl;
    }
  }
  else {
    AggEntry *entry = getLabelEntry(label);
    if (entry->occurrences.empty()) { // label not found
      result << "Label \"" << label << "\" not found" << endl;
    }
    else {
      result << "\"" << label << "\" :" << entry->print() << endl;
    }
  }
  return result.str();
}

GetPatternsLI::GetPatternsLI(Relation *r, const NewPair<int, int> ap, double ms,
                             int mina, int maxa, Geoid *g, const size_t mem) {
  tupleType = getTupleType();
  agg.clear(false);
  agg.initializeInv();
  agg.scanRelation(r, ap, g);
  agg.filter(ms, mem);
  agg.derivePatterns(mina, maxa);
}

GetPatternsLI::~GetPatternsLI() {
  tupleType->DeleteIfAllowed();
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
  NewPair<string, double> result;
  result = agg.results.back();
  agg.results.pop_back();
  FText *pattern = new FText(true, result.first);
  tuple->PutAttribute(0, pattern);
  CcReal *support = new CcReal(true, result.second);
  tuple->PutAttribute(1, support);
  return tuple;
}


}
