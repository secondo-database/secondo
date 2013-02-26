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

[1] Auxiliary functions of the Symbolic Trajectory Algebra

Started March 2012, Fabio Vald\'{e}s

*/
#include "SymbolicTrajectoryTools.h"
#include "SecondoCatalog.h"
#include "RelationAlgebra.h"
#include "FTextAlgebra.h"

extern QueryProcessor *qp;

using namespace std;

string int2String(int i) {
  stringstream result;
  result << i;
  return result.str();
}

int str2Int(string const &text) {
  int result;
  stringstream ss(text);
  if((ss >> result).fail())
    result = 0;
  return result;
}

void deleteSpaces(string &text) {
  size_t pos = 0;
  while ((pos = text.find(' ', pos)) != string::npos)
    text.erase(pos, 1);
}

/*
\subsection{Function ~convert~}

Converts a string into a char[*].

*/
char* convert(string arg) {
  return strdup(arg.c_str());
}

/*
\subsection{Function ~eraseQM~}

Deletes enclosing quotation marks from a string.

*/
string eraseQM(string arg) {
  if (arg.at(0) == '"') {
    return arg.substr(1, arg.length() - 2);
  }
  return arg;
}

string addQM(string arg) {
  if (arg.at(0) == '"') {
    return arg;
  }
  arg.insert(0, "\"");
  arg.append("\"");
  return arg;
}

/*
\subsection{Function ~stringToSet~}

Splits a string {a, b, c, ...} into a set of strings a, b, c, ...

*/
set<string> stringToSet(string input) {
  string element, contents(input);
  int limitpos;
  set<string> result;
  contents.erase(0, input.find_first_not_of(" "));
  if (!contents.compare("_")) {
    return result;
  }
  if (contents.at(0) == '{') {
    contents.assign(contents.substr(1, contents.length() - 2));
  }
  while (!contents.empty()) {
    contents.erase(0, contents.find_first_not_of(", "));
    if (contents.at(0) == '\"') {
      limitpos = contents.find('\"', 1);
      element = contents.substr(1, limitpos - 1);
      contents.erase(0, limitpos + 1);
    }
    else {
      limitpos = contents.find_first_of(",");
      element = contents.substr(0, limitpos);
      contents.erase(0, limitpos);
    }
    result.insert(element);
//     cout << "element " << element << " added" << endl;
  }
  return result;
}

/*
function ~setToString~

*/
string setToString(set<string> input) {
  set<string>::iterator i;
  stringstream result;
  if (input.size() == 1) {
    result << *(input.begin());
  }
  else if (input.size() > 1) {
    result << "{";
    for (i = input.begin(); i != input.end(); i++) {
      if (i != input.begin()) {
        result << ", ";
      }
      result << *i;
    }
    result << "}";
  }
  return result.str();
}
/*
function ~prefixCount~

Returns the number of strings from which ~str~ is a prefix. This is needed in
MLabel::buildIndex.

*/
int prefixCount(string str, set<string> strings) {
  set<string>::iterator it;
  int result = 0;
  for (it = strings.begin(); it != strings.end(); it++) {
    if ((it->substr(0, str.length()) == str) && (*it != str)) {
      result++;
    }
  }
  return result;
}

vector<string> splitPattern(string input) {
  vector<string> result;
  if (!input.size()) {
    return result;
  }
  size_t pos = input.find('{');
  if (pos == 0) { // ({2012, 2013}, ...)
    result.push_back(input.substr(0, input.find('}') + 1));
    if (input.find('{', 1) != string::npos) { // ({2012, 2013}, {a, b, c})
      result.push_back(input.substr(input.find('{', 1)));
    }
    else { // ({2012, 2013} a)
      result.push_back(input.substr(input.find('}') + 1));
    }
  }
  else if (pos == string::npos) { // no set
    if (input.find(' ') == string::npos) { // * or +
      result.push_back(input);
    }
    else { // (2012 a)
      result.push_back(input.substr(0, input.find(' ')));
      result.push_back(input.substr(input.find(' ')));
    }
  }
  else { // (2012 {a, b, c})
    result.push_back(input.substr(0, input.find(' ')));
    result.push_back(input.substr(input.find(' ')));
  }
  return result;
}

/*
function ~extractVar~

Takes an assignment string like ~time=Z.time~ and returns the variable string.

*/
string extractVar(string input) {
  int posEq = input.find('=');
  int posDot = input.find('.');
  return input.substr(posEq + 1, posDot - posEq - 1);
}

int getKey(string type) {
  if (type == "label")  return 0;
  if (type == "time")   return 1;
  if (type == "start")  return 2;
  if (type == "end")    return 3;
  if (type == "card")   return 4;
  if (type == "labels") return 5;
  else return -1; // should not occur
}

/*
function ~extendDate~

Takes a datetime string and extends it to the format YYYY-MM-DD-HH:MM:SS.MMM,
the variable ~start~ decides on the values.

*/
string extendDate(string input, const bool start) {
  string result, mask;
  int month, daysInMonth, year;
  if (start) {
    mask.assign("-01-01-00:00:00.000");
  }
  else {
    mask.assign("-12-31-23:59:59.999");
  }
  if (input[input.size() - 1] == '-') { // handle case 2011-04-02-
    input.resize(input.size() - 1);
  }
  result.assign(input);
  int pos = 1;
  if ((pos = input.find('-', pos)) == string::npos) {
    result.append(mask.substr(0));
    return result;
  }
  pos++;
  if ((pos = input.find('-', pos)) == string::npos) {
    if (!start) {
      stringstream yearStream(input.substr(0, input.find('-')));
      stringstream monthStream(input.substr(input.find('-') + 1));
      stringstream dayStream;
      if ((monthStream >> month).fail()) {
        cout << "month stringstream error" << endl;
        return input + mask.substr(3);
      }
      else {
        switch (month){
          case 1:
          case 3:
          case 5:
          case 7:
          case 8:
          case 10:
          case 12:
            daysInMonth = 31;
            break;
          case 4:
          case 6:
          case 9:
          case 11:
            daysInMonth = 30;
            break;
          case 2:
            if ((yearStream >> year).fail()) {
              cout << "year stringstream error" << endl;
              return input + mask.substr(3);
            }
            else {
              if (((year % 4 == 0) && (year % 100 != 0))
                || (year % 400 == 0)) {
                daysInMonth = 29;
              }
              else {
                daysInMonth = 28;
              }
            }
            break;
          default: // should not occur
            cout << "month " << month << " does not exist" << endl;
            return input + mask.substr(3);
        }
      }
    dayStream << "-" << daysInMonth;
    result.append(dayStream.str());
    result.append(mask.substr(6));
    return result;
    }
  }
  pos++;
  if ((pos = input.find('-', pos)) == string::npos) {
    result.append(mask.substr(6));
    return result;
  }
  pos++;
  if ((pos = input.find(':', pos)) == string::npos) {
    result.append(mask.substr(9));
    return result;
  }
  pos++;
  if ((pos = input.find(':', pos)) == string::npos) {
    result.append(mask.substr(12));
    return result;
  }
  pos++;
  if ((pos = input.find('.', pos)) == string::npos) {
    result.append(mask.substr(15));
    return result;
  }
  return input;
}

/*
function ~checkSemanticDate~
Checks whether ~text~ is a valid semantic date string or a valid database object
of type ~Periods~ and contains the time interval defined by ~uIv~. If the
parameter ~resultNeeded~ is ~false~, the function will only check the validity
of ~text~.

*/
bool checkSemanticDate(const string text, const SecInterval uIv,
                       const bool eval) {
  string weekdays[7] = {"monday", "tuesday", "wednesday", "thursday", "friday",
                        "saturday", "sunday"};
  string months[12] = {"january", "february", "march", "april", "may", "june",
                       "july", "august", "september", "october", "november",
                       "december"};
  string daytimes[4] = {"morning", "afternoon", "evening", "night"};
  Instant uStart = uIv.start;
  Instant uEnd = uIv.end;
  bool isSemanticDate = false;
  for (int i = 0; i < 12; i++) {
    if (!text.compare(months[i])) {
      isSemanticDate = true;
    }
    else if (i < 7) {
      if (!text.compare(weekdays[i])) {
        isSemanticDate = true;
      }
      else if (i < 4) {
        if (!text.compare(daytimes[i])) {
          isSemanticDate = true;
        }
      }
    }
  }
  if (isSemanticDate && !eval) {
    return true;
  }
  else if (isSemanticDate && eval) {
    if ((uStart.GetYear() == uEnd.GetYear()) // year and month of start and end
         && (uStart.GetMonth() == uEnd.GetMonth())) { //must coincode for match
      for (int i = 0; i < 12; i++) { // handle months
        if (!text.compare(months[i])) {
          return (i == uStart.GetMonth() - 1);
        }
      } // for weekdays and daytimes, start and end day have to coincide
      if (uStart.GetGregDay() == uEnd.GetGregDay()) {
        for (int i = 0; i < 7; i++) { // handle weekdays
          if (!text.compare(weekdays[i])) {
            return (i == uStart.GetWeekday());
          }
        }
        for (int i = 0; i < 4; i++) { // handle daytimes
          if (!text.compare(daytimes[i])) {
            switch (i) {
              case 0:
                return (uStart.GetHour() >= 0) && (uEnd.GetHour() <= 11);
              case 1:
                return (uStart.GetHour() >= 12) && (uEnd.GetHour() <= 16);
              case 2:
                return (uStart.GetHour() >= 17) && (uEnd.GetHour() <= 20);
              case 3:
                return (uStart.GetHour() >= 21) && (uEnd.GetHour() <= 23);
              default: // cannot occur
                cout << "daytime error" << endl;
                return false;
            }
          }
        }
      }
    } // different months => match impossible
  }
  else {
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    if (!Periods::checkType(sc->GetObjectTypeExpr(text))) {
      cout << text << " is not a periods object." << endl;
      return false;
    }
    else {
      const int errPos = 0;
      ListExpr errInfo;
      bool ok;
      Word pWord = sc->InObject(sc->GetObjectTypeExpr(text),
                                sc->GetObjectValue(text), errPos, errInfo, ok);
      if (!ok) {
        cout << "Error: InObject failed." << endl;
        return false;
      }
      else {
        Periods* period = static_cast<Periods*>(pWord.addr);
        return (eval ? period->Contains(uIv) : true);
      }
    }
  }
  return false;
}

/*
function ~checkDaytime~

Checks whether the daytime interval resulting from text (e.g., 0:00[~]8:00)
contains the one from the unit label.

*/
bool checkDaytime(const string text, const SecInterval uIv) {
  if ((uIv.start.GetYear() == uIv.end.GetYear())
       && (uIv.start.GetMonth() == uIv.end.GetMonth())
       && (uIv.start.GetGregDay() == uIv.end.GetGregDay())) {
    string startString, endString;
    stringstream startStream;
    startStream << uIv.start.GetYear() << "-" << uIv.start.GetMonth() << "-"
                << uIv.start.GetGregDay() << "-";
    startString.assign(startStream.str());
    endString.assign(startString);
    startString.append(text.substr(0, text.find('~')));
    endString.append(text.substr(text.find('~') + 1));
    Instant *pStart = new DateTime(instanttype);
    Instant *pEnd = new DateTime(instanttype);
    if (!pStart->ReadFrom(extendDate(startString, true))) {
      cout << "error: ReadFrom " << extendDate(startString, true) << endl;
      return false;
    }
    if (!pEnd->ReadFrom(extendDate(endString, false))) {
      cout << "error: ReadFrom " << extendDate(endString, false) << endl;
      return false;
    }
    SecInterval *pInterval = new SecInterval(*pStart, *pEnd, true, true);
    bool result = pInterval->Contains(uIv);
    delete pInterval;
    delete pStart;
    delete pEnd;
    return result;
  } // no match possible in case of different days
  return false;
}

/*
function ~checkAndPrintSeq~

Checks a rewrite sequence and prints it

*/
bool checkRewriteSeq(pair<vector<size_t>, vector<size_t> > seq, size_t maxSize,
                     bool print) {
  for (int i = 0; i < (int)seq.second.size(); i = i + 2) {
    if ((seq.second[i] < 0) || (seq.second[i] > maxSize)
     || (seq.second[i] >= seq.second[i + 1])) {
      cout << "Error: empty assignment sequence" << endl;
      return false;
    }
  }
  for (int i = 0; i < (int)seq.first.size(); i = i + 2) {
    if ((seq.first[i] < 0) || (seq.first[i] > maxSize)
     || (seq.first[i] > seq.first[i + 1])) {
       cout << "Error: " << seq.first[i] << ", " << seq.first[i + 1] << endl;
       return false;
    }
  }
  if (print) {
    cout << "seq=";
    for (int i = 0; i < (int)seq.first.size(); i++) {
      cout << seq.first[i] << "|";
    }
    cout << endl << "assignedSeq=";
    for (int i = 0; i < (int)seq.second.size(); i++) {
      cout << seq.second[i] << "|";
    }
    cout << endl;
  }
  return true;
}

/*
\subsection{Function ~evaluate~}

The string ~input~ is evaluated by Secondo. The result is returned as a Word.

*/
Word evaluate(string input) {
  SecParser qParser;
  string query, queryStr;
  ListExpr queryList;
  Word queryResult;
  input.insert(0, "query ");
  if (!qParser.Text2List(input, queryStr)) {
    if (nl->ReadFromString(queryStr, queryList)) {
      if (!nl->IsEmpty(nl->Rest(queryList))) {
        query = nl->ToString(nl->First(nl->Rest(queryList)));
        if (qp->ExecuteQuery(query, queryResult)) {
          return queryResult;
        }
      }
    }
  }
  return queryResult;
}

/*
\subsection{Function ~createTrajectory~}

Creates a vector of string containing districts of Dortmund in a random but
sensible order.

*/
vector<string> createTrajectory(int size) {
  vector<string> result;
  string districts[12] = {"Aplerbeck", "Brackel", "Eving", "Hörde", "Hombruch",
                          "Huckarde", "Innenstadt-Nord", "Innenstadt-Ost",
                          "Innenstadt-West", "Lütgendortmund", "Mengede",
                          "Scharnhorst"};
  map<int, set<int> > transitions;
  set<int>::iterator it;
  transitions[0].insert(3);
  transitions[0].insert(7);
  transitions[0].insert(1);
  transitions[1].insert(6);
  transitions[1].insert(7);
  transitions[1].insert(11);
  transitions[2].insert(5);
  transitions[2].insert(6);
  transitions[2].insert(10);
  transitions[2].insert(11);
  transitions[3].insert(4);
  transitions[3].insert(7);
  transitions[4].insert(7);
  transitions[4].insert(8);
  transitions[4].insert(9);
  transitions[5].insert(6);
  transitions[5].insert(8);
  transitions[5].insert(9);
  transitions[5].insert(10);
  transitions[6].insert(7);
  transitions[6].insert(8);
  transitions[6].insert(11);
  transitions[7].insert(8);
  transitions[8].insert(9);
  for (int i = 0; i < 12; i++) {
    for (it = transitions[i].begin(); it != transitions[i].end(); it++) {
      transitions[*it].insert(i); // add symmetric transitions
    }
    transitions[i].insert(i); // remaining in the same area is possible
  }
  int choice, prevPos;
  prevPos = (int)rand() % 12;
  for (int i = 0; i < size; i++) {
    choice = (int)rand() % transitions[prevPos].size();
    it = transitions[prevPos].begin();
    advance(it, choice);
    result.push_back(districts[*it]);
    prevPos = *it;
  }
  return result;
}

void fillML(const MString& source, MString& result, DateTime* duration) {
//   result.Clear();
  if (!source.IsDefined()) {
    result.SetDefined(false);
    return;
  }
  if (!source.GetNoComponents()) {
    return;
  }
  UString last, next;
  source.Get(0, last);
  for (int i = 1; i < source.GetNoComponents(); i++) {
    source.Get(i, next);
    if ((last.constValue == next.constValue) && (!duration ||
        (next.timeInterval.start - last.timeInterval.end <= *duration))) {
      last.timeInterval.end = next.timeInterval.end;
      last.timeInterval.rc = next.timeInterval.rc;
    }
    else {
      result.MergeAdd(last);
      last = next;
    }
  }
  result.MergeAdd(last);
}

MLabelIndex::MLabelIndex(DbArray<NodeRef> n, DbArray<NodeLink> nL,
                     DbArray<size_t> lI) {
  nodes = n;
  nodeLinks = nL;
  labelIndex = lI;
  root = 0;
}

void MLabelIndex::insert(string label, set<size_t> pos) {
  if (!root) {
    root = new TrieNode();
  }
  TrieNode *ptr = root;
  unsigned char c;
  unsigned int i = 0;
  while (i <= label.length()) {
    c = label[i];
    if (!ptr->content[c].nextNode) { // successor node has to be created
      if (i == label.length()) { // after last character
        ptr->positions.insert(pos.begin(), pos.end());
      }
      else {
        ptr->content[c].nextNode = new TrieNode();
        ptr = ptr->content[c].nextNode;
      }
    }
    else { // path exists
      ptr = ptr->content[c].nextNode;
    }
    i++;
  }
}

set<size_t> MLabelIndex::find(string label) {
  set<size_t> result;
  if (!root) {
    result = findInDbArrays(label);
    return result;
  }
  if (!label.length()) { // empty label
    return root->positions;
  }
  TrieNode *ptr = root;
  unsigned int i = 0;
  unsigned char c;
  while (i <= label.length()) {
    if (i == label.length()) { // last character
      return ptr->positions;
    }
    c = label[i];
    if (ptr->content[c].nextNode) { // path exists
      ptr = ptr->content[c].nextNode;
    }
    else { // path does not exist
      result = findInDbArrays(label);
      return result;
    }
    i++;
  }
  return result; // should not occur
}

set<size_t> MLabelIndex::findInDbArrays(string label) {
  set<size_t> result;
  if (!nodes.Size()) {
    return result;
  }
  NodeRef nRef = getNodeRef(0);
  unsigned int i(0);
  int nodeRefPos(0);
  bool found = false;
  while (i < label.length()) {
    if ((nRef.firstCont == -1) || (nRef.lastCont == -1)) {
      return result;
    }
    found = false;
    if (nRef.lastCont < nodeLinks.Size()) {
      for (int j = nRef.firstCont; (j <= nRef.lastCont && !found); j++) {
        if (getNodeLink(j).character == label[i]) {
          found = true;
          nodeRefPos = getNodeLink(j).nextNode;
        }
      }
      if (!found) {
        return result;
      }
    }
    else {
      cout << "NL Error:" << nRef.lastCont << ">=" << nodeLinks.Size() << endl;
      return result;
    }
    if (nodeRefPos < nodes.Size()) {
      nRef = getNodeRef(nodeRefPos);
    }
    else {
      cout << "Nodes Error: " << nodeRefPos << " >= " << nodes.Size() << endl;
      return result;
    }
    i++;
  }
  if ((nRef.firstIndex > -1) && (nRef.lastIndex > -1)) {
    if (nRef.lastIndex < labelIndex.Size()) {
      for (int k = nRef.firstIndex; k <= nRef.lastIndex; k++) {
        result.insert(getLabelIndex(k));
      }
    }
    else {
      cout << "LI Err:" << nRef.lastIndex << ">=" << labelIndex.Size() << endl;
      return result;
    }
  }
  insert(label, result);
  return result;
}

void MLabelIndex::makePersistent() {
  if (!root) {
  }
  else {
    stack<unsigned int> nodeIndexes;
    makePersistent(root, nodeIndexes);
  }
}

void MLabelIndex::makePersistent(TrieNode* ptr,
                                 stack<unsigned int>& nodeIndexes) {
  pair<unsigned int, vector<char> > childs = ptr->getChilds();
  NodeRef nRef;
  nRef.firstCont = (childs.first ? nodeLinks.Size() : -1);
  nRef.lastCont = (childs.first ? nodeLinks.Size() + childs.first - 1 : -1);
  nRef.firstIndex = (ptr->positions.size() ? labelIndex.Size() : -1);
  nRef.lastIndex = (ptr->positions.size() ?
                   labelIndex.Size() + ptr->positions.size() - 1 : -1);
  nodes.Append(nRef);
  NodeLink nLink;
  if (childs.first) {
    nLink.character = childs.second.back();
    nLink.nextNode = nodes.Size();
    nodeLinks.Append(nLink);
    childs.second.pop_back();
    stack<unsigned int> missingIndexes;
    for (int i = nRef.firstCont + 1; i <= nRef.lastCont; i++) {
      NodeLink nLink;
      nLink.character = childs.second.back();
      nodeLinks.Append(nLink);
      missingIndexes.push(nodeLinks.Size() - 1);
      childs.second.pop_back();
    }
    while (missingIndexes.size()) {
      nodeIndexes.push(missingIndexes.top());
      missingIndexes.pop();
    }
  }
  else if (nodeIndexes.size()) {
    nodeLinks.Get(nodeIndexes.top(), nLink);
    nLink.nextNode = nodes.Size();
    nodeLinks.Put(nodeIndexes.top(), nLink);
    nodeIndexes.pop();
  }
  for (set<size_t>::iterator it = ptr->positions.begin();
                                   it != ptr->positions.end(); it++) {
    labelIndex.Append(*it);
  }
  for (int i = 0; i < 256; i++) {
    if (ptr->content[i].nextNode) {
      makePersistent(ptr->content[i].nextNode, nodeIndexes);
    }
  }
}

void MLabelIndex::removeTrie() {
  if (!root) {
    return;
  }
  for (int i = 0; i < 256; i++) {
    if (root->content[i].nextNode) {
      remove(root, i);
    }
  }
  delete root;
  root = 0;
}

void MLabelIndex::remove(TrieNode* ptr1, unsigned char c) {
  TrieNode* ptr2 = ptr1->content[c].nextNode;
  for (int i = 0; i < 256; i++) {
    if (ptr2->content[i].nextNode) { // successor of ~i~ exists
      remove(ptr2, i);
    }
  } // all successors removed now
  delete ptr2;
}

pair<unsigned int, vector<char> > TrieNode::getChilds() {
  unsigned int number = 0;
  vector<char> characters;
  for (unsigned int i = 0; i < 256; i++) {
    if (content[255 - i].nextNode) {
      number++;
      characters.push_back((char)(255 - i));
    }
  }
  return make_pair(number, characters);
}

void MLabelIndex::printDbArrays() {
  stringstream ss;
  ss << "Nodes" << endl << "=====" << endl;
  NodeRef nRef;
  for (int i = 0; i < nodes.Size(); i++) {
    nodes.Get(i, nRef);
    ss << i << "  [" << nRef.firstCont << ", " << nRef.lastCont << "] ["
       << nRef.firstIndex << ", " << nRef.lastIndex << "]" << endl;
  }
  ss << endl << "NodeLinks" << endl << "=========" << endl;
  NodeLink nLink;
  for (int i = 0; i < nodeLinks.Size(); i++) {
    nodeLinks.Get(i, nLink);
    ss << i << "  \'" << nLink.character << "\'  " << nLink.nextNode << endl;
  }
  ss << endl << "LabelIndexes" << endl << "============" << endl;
  size_t index;
  for (int i = 0; i < labelIndex.Size(); i++) {
    labelIndex.Get(i, index);
    ss << i << "  " << index << endl;
  }
  cout << ss.str() << endl;
}

NodeRef MLabelIndex::getNodeRef(int pos) const {
  assert((0 <= pos) && (pos < getNodeRefSize()));
  NodeRef nRef;
  cout << "try to get nodeRef #" << pos;
  nodes.Get(pos, nRef);
  cout << "   .............. successful." << endl;
  return nRef;
}

NodeLink MLabelIndex::getNodeLink(int pos) const {
  assert((0 <= pos) && (pos < getNodeLinkSize()));
  NodeLink nLink;
  nodeLinks.Get(pos, nLink);
  return nLink;
}

size_t MLabelIndex::getLabelIndex(int pos) const {
  assert((0 <= pos) && (pos < getLabelIndexSize()));
  size_t index;
  labelIndex.Get(pos, index);
  return index;
}

void MLabelIndex::destroyDbArrays() {
  nodes.Destroy();
  nodeLinks.Destroy();
  labelIndex.Destroy();
}

void MLabelIndex::copyFrom(const MLabelIndex& source) {
  root = 0;
  nodes.copyFrom(source.getNodeRefs());
  nodeLinks.copyFrom(source.getNodeLinks());
  labelIndex.copyFrom(source.getLabelIndex());
}
