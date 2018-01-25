
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

Started March 2012, Fabio Vald\'{e}s

[TOC]

\section{Overview}
This is the implementation of the Symbolic Trajectory Algebra.

\section{Defines and Includes}

*/

#include "Algorithms.h"

using namespace temporalalgebra;
using namespace std;


namespace stj{

/*
\subsection{Functions supporting ~rewrite~}

*/
void Assign::setLabelPtr(unsigned int pos, const string& value) {
  if (pos < pointers[0].size()) {
    ((Label*)pointers[0][pos])->Set(true, value);
  }
}

void Assign::setPlacePtr(unsigned int pos, 
                         const pair<string, unsigned int>& value) {
  if (pos < pointers[1].size()) {
    ((Place*)pointers[1][pos])->Set(true, value);
  }
}

void Assign::setTimePtr(unsigned int pos, const SecInterval& value) {
  if (pos < pointers[2].size()) {
    *((SecInterval*)pointers[2][pos]) = value;
  }
}

void Assign::setStartPtr(unsigned int pos, const Instant& value) {
  if (pos < pointers[3].size()) {
    *((Instant*)pointers[3][pos]) = value;
  }
}

void Assign::setEndPtr(unsigned int pos, const Instant& value) {
  if (pos < pointers[4].size()) {
    *((Instant*)pointers[4][pos]) = value;
  }
}

void Assign::setLeftclosedPtr(unsigned int pos, bool value) {
  if (pos < pointers[5].size()) {
    ((CcBool*)pointers[5][pos])->Set(true, value);
  }
}

void Assign::setRightclosedPtr(unsigned int pos, bool value) {
  if (pos < pointers[6].size()) {
    ((CcBool*)pointers[6][pos])->Set(true, value);
  }
}

void Assign::cleanLabelsPtr(unsigned int pos) {
  if (pos < pointers[8].size()) {
    ((Labels*)pointers[8][pos])->Clean();
  }
}

void Assign::appendToLabelsPtr(unsigned int pos, const string& value) {
  if (pos < pointers[8].size()) {
    ((Labels*)pointers[8][pos])->Append(value);
  }
}

void Assign::appendToLabelsPtr(unsigned int pos, const set<string>& values) {
  if (pos < pointers[8].size()) {
    ((Labels*)pointers[8][pos])->Append(values);
  }
}

void Assign::cleanPlacesPtr(unsigned int pos) {
  if (pos < pointers[9].size()) {
    ((Places*)pointers[9][pos])->Clean();
  }
}
  
void Assign::appendToPlacesPtr(unsigned int pos, 
                               const pair<string, unsigned int>& value) {
  if (pos < pointers[9].size()) {
    ((Places*)pointers[9][pos])->Append(value);
  }
}

void Assign::appendToPlacesPtr(unsigned int pos, 
                               const set<pair<string, unsigned int> >& values) {
  if (pos < pointers[9].size()) {
    ((Places*)pointers[9][pos])->Append(values);
  }
}


/*
\section{Implementation of class ~MLabels~}

\subsection{Function ~createML~}

*/
void MLabel::createML(const int size, const int number, vector<string>& labels){
  if (size > 0) {
    ULabel ul(1);
    Instant start(1.0);
    start.Set(2015, 1, 1);
    time_t t;
    time(&t);
    srand((unsigned int)t);
    Instant end(start);
    SecInterval iv(true);
    int labelStartPos = size * number;
    for (int i = 0; i < size; i++) {
      datetime::DateTime dur(0, rand() % 86400000 + 3600000, 
                             datetime::durationtype);
      end.Add(&dur);
      ul.constValue.Set(true, labels[labelStartPos + i]);
      iv.Set(start, end, true, false);
      ul.timeInterval = iv;
      Add(ul);
      start = end;
    }
    units.TrimToSize();
  }
  else {
    cout << "Invalid parameters for creation." << endl;
    SetDefined(false);
  }
}

/*
\subsection{Function ~convertFromMString~}

*/
void MLabel::convertFromMString(const MString& source) {
  Clear();
  if (!IsDefined()) {
    return;
  }
  SetDefined(true);
  UString us(true);
  for (int i = 0; i < source.GetNoComponents(); i++) {
    source.Get(i, us);
    ULabel ul(us.timeInterval, us.constValue.GetValue());
    Add(ul);
  }
  units.TrimToSize();
}

/*
\subsection{Function ~GetText~}

Returns the pattern text as specified by the user.

*/
string Pattern::GetText() const {
  return text.substr(0, text.length() - 1);
}

/*
\subsection{Function ~isValid~}

Decides whether the pattern is suitable for the type M.

*/
bool Pattern::isValid(const string& type) const {
  for (int i = 0; i < getSize(); i++) {
    if (type.length() < 6) {
      return false;
    }
  }
  for (unsigned int i = 0; i < conds.size(); i++) {
    for (int j = 0; j < conds[i].getVarKeysSize(); j++) {
      switch (conds[i].getKey(j)) {
        case 0: { // label
          if (type != MLabel::BasicType()) {
            return false;
          }
          break;
        }
        case 1: { // place
          if (type != MPlace::BasicType()) {
            return false;
          }
          break;
        }
        case 8: { // labels
          if (type.substr(1, 5) != Label::BasicType()) {
            return false;
          }
          break;
        }
        case 9: { // places
          if (type.substr(1, 5) != Place::BasicType()) {
            return false;
          }
          break;
        }
        default: {}
      }
    }
  }
  for (unsigned int i = 0; i < assigns.size(); i++) {
    if ((!assigns[i].getText(0).empty() && (type != MLabel::BasicType())) ||
        (!assigns[i].getText(1).empty() && (type != MPlace::BasicType())) ||
        (!assigns[i].getText(8).empty() && (type != MLabels::BasicType())) ||
        (!assigns[i].getText(9).empty() && (type != MPlaces::BasicType()))) {
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~isCompatible~}

Checks whether an extended pattern is compatible to a certain tuple type. The
attribute number refers to the master attribute, whose type has to be a symbolic
trajectory.
For indextmatches2, majorAttrNo and majorValueNo are irrelevant.

*/
bool Pattern::isCompatible(TupleType *ttype, const int majorAttrNo,
      vector<pair<int, string> >& relevantAttrs, int& majorValueNo) {
  relevantAttrs = Tools::getRelevantAttrs(ttype, majorAttrNo, majorValueNo);
//   cout << "attrs: ";
//   for (unsigned int i = 0; i < relevantAttrs.size(); i++) {
//     cout << "(" << relevantAttrs[i].first << ": " << relevantAttrs[i].second
//          << ") ";
//   }
//   cout << endl;
  vector<pair<Word, SetRel> > values;
  for (int i = 0; i < getSize(); i++) {
    values = elems[i].getValues();
    if (values.size() > 0) { // atom has values
      if (values.size() != relevantAttrs.size()) {
        cout << "wrong number of values (" << values.size() << " instead of "
             << relevantAttrs.size() << ") in atom " << i << endl;
        return false;
      }
      for (unsigned int j = 0; j < values.size(); j++) {
        if (!Tools::checkAttrType(relevantAttrs[j].second, values[j].first)) {
          cout << "wrong type (atom " << i << ", specification " << j << ")"
               << endl;
          return false;
        }
      }
    }
  }
  return true;
}

/*
\subsection{Function ~getPattern~}

Calls the parser.

*/
Pattern* Pattern::getPattern(string input, bool classify, Tuple *tuple /* =0 */,
                             ListExpr ttype /* = 0 */) {
  if (input.find('\n') == string::npos) {
    input.append("\n");
  }
  const char *patternChar = input.c_str();
  return parseString(patternChar, classify, tuple, ttype);
}

bool Pattern::containsFinalState(set<int> &states) {
  set<int> result;
  set_intersection(states.begin(), states.end(), finalStates.begin(),
                   finalStates.end(), std::inserter(result, result.begin()));
  return !result.empty();
}

bool Pattern::parseNFA() {
  IntNfa* intNfa = 0;
  if (parsePatternRegEx(regEx.c_str(), &intNfa) != 0) {
    return false;
  }
  intNfa->nfa.makeDeterministic();
  intNfa->nfa.minimize();
  intNfa->nfa.bringStartStateToTop();
  map<int, set<int> >::iterator it;
  for (unsigned int i = 0; i < intNfa->nfa.numOfStates(); i++) {
    map<int, set<int> > transitions = intNfa->nfa.getState(i).getTransitions();
    map<int, int> newTrans;
    for (it = transitions.begin(); it != transitions.end(); it++) {
      newTrans[it->first] = *(it->second.begin());
    }
    nfa.push_back(newTrans);
    if (intNfa->nfa.isFinalState(i)) {
      finalStates.insert(i);
    }
  }
//   printNfa(nfa, finalStates);
  delete intNfa;
  return true;
}

/*
\subsection{Function ~simplifyNFA~}

*/
void Pattern::simplifyNFA(vector<map<int, int> >& result) {
  result.clear();
  string ptext = GetText().substr(0, GetText().find("=>"));
  ptext = ptext.substr(0, ptext.find("//")); //remove assignments and conditions
  for (unsigned int i = 1; i < ptext.length(); i++) {
    if ((ptext[i - 1] == ']') && ((ptext[i] == '*') || (ptext[i] == '+'))) {
      ptext[i] = ' '; // eliminate repetition of regular expressions
    }
  }
  Pattern *pnew = Pattern::getPattern(ptext, false);
  vector<map<int, int> > oldNFA;
  pnew->getNFA(oldNFA);
  delete pnew;
  result.resize(oldNFA.size());
  map<int, int>::iterator im;
  for (int i = 0; i < (int)oldNFA.size(); i++) {
    for (im = oldNFA[i].begin(); im != oldNFA[i].end(); im++) {
      if (im->second != i) { // eliminate * and + loops
        result[i][im->first] = im->second;
      }
    }
  }
  // TODO: do it more elegantly!
  set<int> foundFinalStates;
  int maxState = 0;
  for (unsigned int i = 0; i < result.size(); i++) {
    for (im = result[i].begin(); im != result[i].end(); im++) {
      if (finalStates.find(im->second) != finalStates.end()) {
        foundFinalStates.insert(im->second);
      }
      if (im->second > maxState) {
        maxState = im->second;
      }
    }
  }
  if (finalStates != foundFinalStates) {
    finalStates.clear();
    finalStates.insert(maxState);
//     cout << "final state is now " << maxState << endl;
  }
//   Tools::printNfa(result, finalStates);
}

/*
\subsection{Function ~findNFApaths~}

*/
void Pattern::findNFApaths(vector<map<int, int> >& simpleNFA, 
                           set<pair<set<int>, int> >& result) {
  result.clear();
  set<pair<set<int>, int> > newPaths;
  set<int> elems;
  set<pair<set<int>, int> >::iterator is;
  map<int, int>::iterator im;
  result.insert(make_pair(elems, 0));
  bool proceed = true;
  while (proceed) {
    for (is = result.begin(); is != result.end(); is++) {
      map<int, int> trans = simpleNFA[is->second];
      for (im = trans.begin(); im != trans.end(); im++) {
        elems = is->first; // get old transition set
        elems.insert(im->first); // add new transition
        newPaths.insert(make_pair(elems, im->second)); // and new state
      }
      if (simpleNFA[is->second].empty()) { // no transition available
        newPaths.insert(*is); // keep path
      }
    }
    result = newPaths;
    newPaths.clear();
    proceed = false;
    for (is = result.begin(); is != result.end(); is++) {
      if (finalStates.find(is->second) == finalStates.end()) { // no final state
        proceed = true; // continue pathfinding
      }
    }
  }
}

/*
\subsection{Function ~getCrucialElems~}

*/
void Pattern::getCrucialElems(const set<pair<set<int>, int> >& paths,
                              set<int>& result) {
  set<int> elems, temp;
  set<pair<set<int>, int> >::iterator is;
  set<int>::iterator it;
  is = paths.begin();
  elems = is->first;
  is++;
  while (is != paths.end()) { // collect crucial elems
//     cout << "{";
//     for (it = is->first.begin(); it != is->first.end(); it++) {
//       cout << *it << " ";
//     }
//     cout << "} {";
//     for (it = elems.begin(); it != elems.end(); it++) {
//       cout << *it << " ";
//     }    
//     cout << "}";
    set_intersection(is->first.begin(), is->first.end(), elems.begin(), 
                     elems.end(), std::inserter(temp, temp.begin()));
//     cout << "  -->  {";
//     for (it = temp.begin(); it != temp.end(); it++) {
//       cout << *it << " ";
//     }  
//     cout << "}" << endl;
    elems = temp;
    temp.clear();
    is++;
  }
//   cout << "crucial elements: {";
  for (it = elems.begin(); it != elems.end(); it++) {
//     cout << *it << " ";
    result.insert(*it);
  }
//   cout << "}" << endl;
}

/*
\subsection{Function ~tmatches~}

Computes whether a tuple with several trajectories of different types matches
the pattern.

*/
ExtBool Pattern::tmatches(Tuple *tuple, const int attrno, ListExpr ttype) {
  ExtBool result = ST_UNDEF;
  vector<pair<int, string> > relevantAttrs;
  int majorValueNo = -1;
  if (isCompatible(tuple->GetTupleType(), attrno, relevantAttrs, majorValueNo)){
    if (initEasyCondOpTrees(true, tuple, ttype) && !isNFAempty()) {
      TMatch tmatch(this, tuple, ttype, attrno, relevantAttrs, majorValueNo);
      result = tmatch.matches();
    }
  }
  deleteAtomValues(relevantAttrs);
  return result;
}

/*
function ~getConstValue~

Extracts a constant value from a moving attribute at a certain instant, e.g.,
for an mpoint attribute, the function yields a point result.

*/
template<class Moving, class Intime, class Constant>
void Condition::getConstValue(Attribute *src, const Instant& inst,
                              Attribute*& result) {
  if (src != 0) {
    Intime it(true);
    ((Moving*)src)->AtInstant(inst, it);
    result->CopyFrom((Constant*)&(it.value));
  }
  else {
    result = new Constant(true);
  }
}


void Condition::getConstValue(Attribute *src, const std::string& type,
                              const Instant& inst, Attribute*& result) {
  if (type == "mlabel") {
    getConstValue<MLabel, ILabel, Label>(src, inst, result);
  }
  else if (type == "mlabels") {
    getConstValue<MLabels, ILabels, Labels>(src, inst, result);
  }
  else if (type == "mplace") {
    getConstValue<MPlace, IPlace, Place>(src, inst, result);
  }
  else if (type == "mplaces") {
    getConstValue<MPlaces, IPlaces, Places>(src, inst, result);
  }
  else if (type == "mbool") {
    getConstValue<MBool, IBool, CcBool>(src, inst, result);
  }
  else if (type == "mint") {
    getConstValue<MInt, IInt, CcInt>(src, inst, result);
  }
  else if (type == "mreal") {
    getConstValue<MReal, IReal, CcReal>(src, inst, result);
  }
  else if (type == "mpoint") {
    getConstValue<MPoint, IPoint, Point>(src, inst, result);
  }
  else if (type == "mregion") {
    getConstValue<MRegion, IRegion, Region>(src, inst, result);
  }
  else {
    cout << "INVALID type " << type << endl;
    result = 0;
  }
}

string Condition::getType(const int t, const string& var,
                          Tuple *tuple /* = 0 */, ListExpr ttype /* = 0 */) {
  if (t > 99) {
    if (tuple && ttype) {
      if (var.empty()) {
        return nl->ToString(nl->First(nl->Nth(t - 99, nl->Second(ttype))));
      }
      return "." + nl->ToString(nl->First(nl->Nth(t - 99, nl->Second(ttype))));
    }
    else {
      return ".ERROR";
    }
  }
  switch (t) {
    case 0: return ".label";
    case 1: return ".place";
    case 2: return ".time";
    case 3: return ".start";
    case 4: return ".end";
    case 5: return ".leftclosed";
    case 6: return ".rightclosed";
    case 7: return ".card";
    case 8: return ".labels";
    case 9: return ".places";
    default: return ".ERROR";
  }
}

string Condition::toString() const {
  stringstream result;
  result << text << endl;
  for (unsigned int j = 0; j < varKeys.size(); j++) {
    result << j << ": " << varKeys[j].first << "." << varKeys[j].second << endl;
  }
  return result.str();
}

void Condition::setValuePtr(unsigned int pos, string& value) {
  if (pos < pointers.size()) {
    ((Label*)pointers[pos])->Set(true, value);
  }
}

void Condition::setValuePtr(unsigned int pos, pair<string,unsigned int>& value){
  if (pos < pointers.size()) {
    ((Place*)pointers[pos])->Set(true, value);
  }
}

void Condition::clearTimePtr(unsigned int pos) {
  if (pos < pointers.size()) {
    if (((Periods*)pointers[pos])->IsDefined()) {
      ((Periods*)pointers[pos])->Clear();
    }
  }
}

void Condition::mergeAddTimePtr(unsigned int pos, Interval<Instant>& value) {
  if (pos < pointers.size()) {
    ((Periods*)pointers[pos])->MergeAdd(value);
  }
}

void Condition::setTimePtr(unsigned int pos, const Periods& per) {
  if (pos < pointers.size()) {
    ((Periods*)pointers[pos])->CopyFrom(&per);
  }
}

void Condition::setStartEndPtr(unsigned int pos, Instant& value) {
  if (pos < pointers.size()) {
    *((Instant*)pointers[pos]) = value;
  }
}

void Condition::setCardPtr(unsigned int pos, int value) {
  if (pos < pointers.size()) {
    ((CcInt*)pointers[pos])->Set(true, value);
  }
}

void Condition::cleanLabelsPtr(unsigned int pos) {
  if (pos < pointers.size()) {
    ((Labels*)pointers[pos])->Clean();
  }
}

void Condition::appendToLabelsPtr(unsigned int pos, string& value) {
  if (pos < pointers.size()) {
    ((Labels*)pointers[pos])->Append(value);
  }
}

void Condition::appendToLabelsPtr(unsigned int pos, set<string>& values) {
  if (pos < pointers.size()) {
    ((Labels*)pointers[pos])->Append(values);
  }
}

void Condition::cleanPlacesPtr(unsigned int pos) {
  if (pos < pointers.size()) {
    ((Places*)pointers[pos])->Clean();
  }
}

void Condition::appendToPlacesPtr(unsigned int pos, 
                                  pair<string, unsigned int>& value) {
  if (pos < pointers.size()) {
    ((Places*)pointers[pos])->Append(value);
  }
}

void Condition::appendToPlacesPtr(unsigned int pos, 
                                  set<pair<string, unsigned int> >& values) {
  if (pos < pointers.size()) {
    ((Places*)pointers[pos])->Append(values);
  }
}

void Condition::setLeftRightclosedPtr(unsigned int pos, bool value) {
  if (pos < pointers.size()) {
    ((CcBool*)pointers[pos])->Set(true, value);
  }
}

/*
\subsection{Function ~getPointer~}

Static function invoked by ~initCondOpTrees~ or ~initAssignOpTrees~

*/
pair<string, Attribute*> Pattern::getPointer(const int key, 
                   const bool isInterval /* = true */, Tuple *tuple /* = 0 */) {
  pair<string, Attribute*> result;
  if (key > 99) { // attribute name
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    AttributeType attrType = tuple->GetTupleType()->GetAttributeType(key - 100);
    string type = sc->GetTypeName(attrType.algId, attrType.typeId);
//     cout << "TYPE is " << type << endl;
    if (isInterval) {
      if (type == "mplace" || type == "mplaces") {
        result.second = new Region(1);
        result.first = "[const region pointer "
                    + nl->ToString(listutils::getPtrList(result.second)) + "]";
      }
      else {
        result.second = tuple->GetAttribute(key - 100)->Clone();
        result.first = "[const " + type + " pointer "
                    + nl->ToString(listutils::getPtrList(result.second)) + "]";
      }
    }
    else {
      if (Tools::isRelevantAttr(type)) { // moving attribute
        Instant inst(datetime::instanttype);
        Condition::getConstValue(0, type, inst, result.second);
        type = type.substr(1);
      }
      else { // constant attribute type
        result.second = tuple->GetAttribute(key - 100)->Clone();
      }
      result.first = "[const " + type + " pointer "
                   + nl->ToString(listutils::getPtrList(result.second)) + "]";
//       cout << result.first << endl;
    }
  }
  else {
    if (isInterval) {
      switch (key) {
        case 0: { // label, type Label
          result.second = new Label(true);
          result.first = "[const label pointer "
                     + nl->ToString(listutils::getPtrList(result.second)) + "]";
          break;
        }
        case 1: { // place, type Place
          result.second = new Place(true);
          result.first = "[const place pointer "
                     + nl->ToString(listutils::getPtrList(result.second)) + "]";
          break;
        }
        case 2: { // time, type Periods
          result.second = new Periods(1);
          result.first = "[const periods pointer "
                     + nl->ToString(listutils::getPtrList(result.second)) + "]";
          break;
        }
        case 3:   // start, type Instant
        case 4: { // end, type Instant
          result.second = new datetime::DateTime(datetime::instanttype);
          result.first = "[const instant pointer "
                     + nl->ToString(listutils::getPtrList(result.second)) + "]";
          break;
        }
        case 5:   // leftclosed, type CcBool
        case 6: { // rightclosed, type CcBool
          result.second = new CcBool(false);
          result.first = "[const bool pointer "
                     + nl->ToString(listutils::getPtrList(result.second)) + "]";
          break;
        }
        case 7: { // card, type CcInt
          result.second = new CcInt(false);
          result.first = "[const int pointer "
                     + nl->ToString(listutils::getPtrList(result.second)) + "]";
          break;
        }
        case 8: { // labels, type Labels
          result.second = new Labels(true);
          result.first = "[const labels pointer "
                     + nl->ToString(listutils::getPtrList(result.second)) + "]";
          break;
        }
        default: { // places, type Places
          result.second = new Places(true);
          result.first = "[const places pointer "
                     + nl->ToString(listutils::getPtrList(result.second)) + "]";
        }
      }
    }
    else { // used for indextmatches2
      if (key >= 2 && key <= 4) { // time, start, end: type instant
        result.second = new datetime::DateTime(datetime::instanttype);
        result.first = "[const instant pointer "
                     + nl->ToString(listutils::getPtrList(result.second)) + "]";
      }
    }
  }
  return result;
}

/*
\subsection{Function ~initCondOpTrees~}

For a pattern with conditions, an operator tree structure is prepared.

*/
bool Pattern::initCondOpTrees(Tuple *tuple /* = 0 */, ListExpr ttype /* = 0 */,
                              const bool mainAttr) {
  for (unsigned int i = 0; i < conds.size(); i++) { // opTrees for conditions
//     if (!mainAttr) {
//       conds[i].collectInstantVars(varToElem);
//     }
    if (!conds[i].initOpTree(tuple, ttype)) {
      cout << "Operator tree for condition " << i << " uninitialized" << endl;
      return false;
    }
  }
  return true;
}

bool Condition::initOpTree(Tuple *tuple /* = 0 */, ListExpr ttype /* = 0 */) {
  string q(""), part, toReplace("");
  pair<string, Attribute*> strAttr;
  vector<Attribute*> ptrs;
  if (!isTreeOk()) {
    q = "query " + text;
    for (unsigned int i = 0; i < varKeys.size(); i++) { // init pointers
//       cout << "|| " << varKeys[i].first << "|" << varKeys[i].second 
//            << "|" << endl;
//    bool isInterval = instantVars.find(varKeys[i].first) == instantVars.end();
      bool isInterval = true;
      strAttr = Pattern::getPointer(getKey(i), isInterval, tuple);
      ptrs.push_back(strAttr.second);
      toReplace = getVar(i) + getType(getKey(i), getVar(i), tuple, ttype);
      q.replace(q.find(toReplace), toReplace.length(), strAttr.first);
//       cout << "init op tree for | " << q << endl;
    }
    pair<QueryProcessor*, OpTree> qp_optree = Tools::processQueryStr(q, -1);
    if (!qp_optree.first) {
      return false;
    }
    setOpTree(qp_optree);
    setPointers(ptrs);
    ptrs.clear();
    setTreeOk(true);
  }
  return true;
}

/*
\subsection{Function ~initEasyCondOpTrees~}

For a pattern with conditions, an operator tree structure is prepared.

*/
bool Pattern::initEasyCondOpTrees(const bool mainAttr, Tuple *tuple /* = 0 */, 
                                  ListExpr ttype /* = 0 */) {
  string q(""), part, toReplace("");
  pair<string, Attribute*> strAttr;
  vector<Attribute*> ptrs;
  for (unsigned int i = 0; i < easyConds.size(); i++) {
    if (!easyConds[i].isTreeOk()) {
      q = "query " + easyConds[i].getText();
      for (int j = 0; j < easyConds[i].getVarKeysSize(); j++) { // init pointers
//        cout << "|" << easyConds[i].getVar(j) << "|" << easyConds[i].getKey(j)
//             << "|" << varPos[easyConds[i].getVar(j)].first << " " 
//             << varPos[easyConds[i].getVar(j)].second << endl;
        strAttr = getPointer(easyConds[i].getKey(j), true, tuple);
        ptrs.push_back(strAttr.second);
        string var = easyConds[i].getVar(j);
        toReplace = var 
                + Condition::getType(easyConds[i].getKey(j), var, tuple, ttype);
        q.replace(q.find(toReplace), toReplace.length(), strAttr.first);
//         cout << "# " << q << endl;
      }
      pair<QueryProcessor*, OpTree> qp_optree = Tools::processQueryStr(q, -1);
      if (!qp_optree.first) {
        cout << "Op tree for easy condition " << i << " uninitialized" << endl;
        return false;
      }
      easyConds[i].setOpTree(qp_optree);
      easyConds[i].setPointers(ptrs);
      ptrs.clear();
      easyConds[i].setTreeOk(true);
    }
  }
  return true;
}

/*
\subsection{Function ~deleteCondOpTrees~}

Removes the corresponding structures.

*/
void Pattern::deleteCondOpTrees() {
  for (unsigned int i = 0; i < conds.size(); i++) {
    if (conds[i].isTreeOk()) {
      conds[i].deleteOpTree();
    }
  }
}

/*
\subsection{Function ~deleteEasyCondOpTrees~}

Removes the corresponding structures.

*/
void Pattern::deleteEasyCondOpTrees() {
  for (unsigned int i = 0; i < easyConds.size(); i++) {
    if (easyConds[i].isTreeOk()) {
      easyConds[i].deleteOpTree();
    }
  }
}

/*
\section{Functions for class ~TMatch~}

\subsection{Constructor}

*/
TMatch::TMatch(Pattern *pat, Tuple *tuple, ListExpr tt, const int _attrno, 
               vector<pair<int, string> >& _relevantAttrs, const int _valueno) {
  p = pat;
  t = tuple;
  ttype = tt;
  attrno = _attrno;
  valueno = _valueno;
  matching = 0;
  type = Tools::getDataType(t->GetTupleType(), attrno);
  relevantAttrs = _relevantAttrs;
}

/*
\subsection{Function ~matches~}

*/
ExtBool TMatch::matches() {
  set<int> states;
  states.insert(0); // initial state
  int noMainComponents = GetNoMainComponents();
  if (!p->hasConds() && !p->hasAssigns()) {
    for (int i = 0; i < noMainComponents; i++) {
      if (!performTransitions(i, states)) {
        return ST_FALSE;
      }
    }
    if (p->containsFinalState(states)) {
      return ST_TRUE;
    }
    else {
      return ST_FALSE;
    }
  }
  pathMatrix = Tools::createSetMatrix(noMainComponents, p->elemToVar.size());
  for (int i = 0; i < noMainComponents; i++) {
    if (!performTransitionsWithMatrix(i, states)) {
      Tools::deleteSetMatrix(pathMatrix, noMainComponents);
      return ST_FALSE;
    }
  }
  if (!p->containsFinalState(states)) {
    Tools::deleteSetMatrix(pathMatrix, noMainComponents);
    return ST_FALSE;
  }
  if (!p->initCondOpTrees(t, ttype)) {
    Tools::deleteSetMatrix(pathMatrix, noMainComponents);
    return ST_FALSE;
  }
  if (!p->hasAssigns()) {
    bool result = findMatchingBinding(0);
    Tools::deleteSetMatrix(pathMatrix, noMainComponents);
    return (result ? ST_TRUE : ST_FALSE);
  }
  return ST_TRUE;
}

/*
\subsection{Function ~GetNoMainComponents~}

*/
int TMatch::GetNoMainComponents() const {
  switch (type) {
    case MLABEL: {
      return ((MLabel*)t->GetAttribute(attrno))->GetNoComponents();
    }
    case MLABELS: {
      return ((MLabels*)t->GetAttribute(attrno))->GetNoComponents();
    }
    case MPLACE: {
      return ((MPlace*)t->GetAttribute(attrno))->GetNoComponents();
    }
    case MPLACES: {
      return ((MPlaces*)t->GetAttribute(attrno))->GetNoComponents();
    }
    default: { // cannot occur
      return -1;
    }
  }  
}

/*
\subsection{Function ~GetInterval~}

*/
void TMatch::GetInterval(const int u, SecInterval& iv) const {
  switch (type) {
    case MLABEL: {
      ((MLabel*)t->GetAttribute(attrno))->GetInterval(u, iv);
      return;
    }
    case MLABELS: {
      ((MLabels*)t->GetAttribute(attrno))->GetInterval(u, iv);
      return;
    }
    case MPLACE: {
      ((MPlace*)t->GetAttribute(attrno))->GetInterval(u, iv);
      return;
    }
    case MPLACES: {
      ((MPlaces*)t->GetAttribute(attrno))->GetInterval(u, iv);
      return;
    }
  }
}

/*
\subsection{Function ~labelsMatch~}

*/
bool TMatch::labelsMatch(const set<string>& tlabels, const int atom, 
                         const int pos) {
  set<string> plabels;
  pair<Word, SetRel> values = p->elems[atom].values[pos];
  if (values.first.addr == 0) {
    return true;
  }
  ((Labels*)values.first.addr)->GetValues(plabels);
  return Tools::relationHolds(tlabels, plabels, values.second);
}

/*
\subsection{Function ~placesMatch~}

*/
bool TMatch::placesMatch(const set<pair<string, unsigned int> >& tplaces, 
                         const int atom, const int pos) {
  set<string> pplaces;
  pair<Word, SetRel> values = p->elems[atom].values[pos];
  if (values.first.addr == 0) {
    return true;
  }
  string type = p->elems[atom].types[pos];
  if (type == Labels::BasicType()) {
    if (((Labels*)values.first.addr)->IsDefined()) {
      ((Labels*)values.first.addr)->GetValues(pplaces);
      return Tools::relationHolds(tplaces, pplaces, values.second);
    }
    else {
      cout << "ERROR: type/value mismatch" << endl;
      return false;
    }
  }
  else if (type == Rectangle<2>::BasicType()) {
    return true; // no further test required
  }
  else if (type == Region::BasicType()) {
    Region reg(*((Region*)(values.first.addr)));
    cout << "CHECK region ";
    return Tools::relationHolds(tplaces, reg, values.second);
  }
  else {
    cout << "ERROR: invalid type " << type << endl;
    return false;
  }
}

/*
\subsection{Function ~mainValuesMatch~}

*/
bool TMatch::mainValuesMatch(const int u, const int atom) {
  switch (type) {
    case MLABEL: {
      if (u < 0 || u >= ((MLabel*)t->GetAttribute(attrno))->GetNoComponents()) {
        return false;
      }
      set<string> labels;
      string label;
      ((MLabel*)t->GetAttribute(attrno))->GetValue(u, label);
      labels.insert(label);
      return labelsMatch(labels, atom, valueno);
    }
    case MLABELS: {
      if (u < 0 || u >= ((MLabels*)t->GetAttribute(attrno))->GetNoComponents()){
        return false;
      }
      set<string> labels;
      ((MLabels*)t->GetAttribute(attrno))->GetValues(u, labels);
      return labelsMatch(labels, atom, valueno);
    }
    case MPLACE: {
      if (u < 0 || u >= ((MPlace*)t->GetAttribute(attrno))->GetNoComponents()) {
        return false;
      }
      set<pair<string, unsigned int> > places;
      pair<string, unsigned int> place;
      ((MPlace*)t->GetAttribute(attrno))->GetValue(u, place);
      places.insert(place);
      return placesMatch(places, atom, valueno);
    }
    case MPLACES: {
      if (u < 0 || u >= ((MPlaces*)t->GetAttribute(attrno))->GetNoComponents()){
        return false;
      }
      set<pair<string, unsigned int> > places;
      ((MPlaces*)t->GetAttribute(attrno))->GetValues(u, places);
      return placesMatch(places, atom, valueno);
    }
    default: { // cannot occur
      return false;
    }
  }
}

/*
\subsection{Function ~otherValuesMatch~}

*/
bool TMatch::otherValuesMatch(const int pos, const SecInterval& iv, 
                              const int atom) {
  pair<Word, SetRel> values = p->elems[atom].values[pos];
  if (values.first.addr == 0) {
    return true;
  }
  pair<int, string> attrInfo = relevantAttrs[pos];
  if (attrInfo.first == attrno) {
    return true;
  }
  int start(-1), end(-1);
  if (attrInfo.second == "mlabel") {
    MLabel *mlabel = (MLabel*)t->GetAttribute(attrInfo.first);
    start = mlabel->Position(iv.start);
    end = mlabel->Position(iv.end);
    if (start == -1 || end == -1) {
      return false;
    }
    temporalalgebra::SecInterval iv1, iv2;
    mlabel->GetInterval(start, iv1);
    mlabel->GetInterval(end, iv2);
    if (!iv.lc && iv.start == iv1.end) {
      start++;
    }
    if (!iv.rc && iv.end == iv2.start) {
      end--;
    }
    set<string> tlabels, plabels, labels;
    string label;
    for (int i = start; i <= end; i++) {
      mlabel->GetValue(i, label);
      tlabels.insert(label);
    }
    ((Labels*)values.first.addr)->GetValues(plabels);
    return Tools::relationHolds(tlabels, plabels, values.second);
  }
  else if (attrInfo.second == "mlabels") {
    MLabels *mlabels = (MLabels*)t->GetAttribute(attrInfo.first);
    start = mlabels->Position(iv.start);
    end = mlabels->Position(iv.end);
    if (start == -1 || end == -1) {
      return false;
    }
    set<string> tlabels, plabels, labels;
    string label;
    for (int i = start; i <= end; i++) {
      mlabels->GetValues(i, labels);
      tlabels.insert(labels.begin(), labels.end());
    }
    ((Labels*)values.first.addr)->GetValues(plabels);
    return Tools::relationHolds(tlabels, plabels, values.second);
  }
  else if (attrInfo.second == "mplace") {
    // TODO.
  }
  else if (attrInfo.second == "mplaces") {
    // TODO.
  }
  else if (attrInfo.second == "mpoint") {
    MPoint *mpoint = (MPoint*)t->GetAttribute(attrInfo.first);
    Periods per(true);
    per.Add(iv);
    MPoint mpAtPer(true);
    mpoint->AtPeriods(per, mpAtPer);
    if (mpAtPer.IsEmpty()) {
      return false;
    }
    return Tools::relationHolds(mpAtPer, *((Region*)values.first.addr), 
                                values.second);
  }
  else if (attrInfo.second == "mregion") {
    MRegion *mreg = (MRegion*)t->GetAttribute(attrInfo.first);
    Periods per(true);
    per.Add(iv);
    MRegion mrAtPer(true);
    mreg->AtPeriods(&per, &mrAtPer);
    if (mrAtPer.IsEmpty()) {
      return false;
    }
    return Tools::relationHolds(mrAtPer, *((Region*)values.first.addr), 
                                values.second);
  }
  else if (attrInfo.second == "mbool") {
    MBool *mbool = (MBool*)t->GetAttribute(attrInfo.first);
    start = mbool->Position(iv.start);
    end = mbool->Position(iv.end);
    if (start == -1 || end == -1) {
      return false;
    }
    set<bool> boolSet;
    UBool ub(true);
    for (int i = start; i <= end; i++) {
      mbool->Get(i, ub);
      boolSet.insert(ub.constValue.GetValue());
    }
    return Tools::relationHolds(boolSet, 
                       ((CcBool*)values.first.addr)->GetValue(), values.second);
  }
  else if (attrInfo.second == "mint") {
    Range<CcInt> trange(true), prange(true);
    MInt *mint = (MInt*)t->GetAttribute(attrInfo.first);
    UInt unit(true);
    int firstpos(mint->Position(iv.start)), lastpos(mint->Position(iv.end));
    if (firstpos >= 0 && lastpos >= firstpos &&
        lastpos < mint->GetNoComponents()) {
      set<int> intSet;
      for (int i = firstpos; i <= lastpos; i++) {
        mint->Get(i, unit);
        intSet.insert(unit.constValue.GetValue());
      }
      return Tools::relationHolds(*((Range<CcReal>*)values.first.addr), intSet, 
                                  values.second);
    }
    return false;
  }
  else if (attrInfo.second == "mreal") {
    Range<CcReal> trange(true);
    MReal *mreal = (MReal*)t->GetAttribute(attrInfo.first);
    UReal ureal(true), urealTemp(true);
    int firstpos(mreal->Position(iv.start)), lastpos(mreal->Position(iv.end));
    if (firstpos >= 0 && lastpos >= firstpos &&
        lastpos < mreal->GetNoComponents()) {
      bool correct = true;
      CcReal ccmin(0.0, true), ccmax(0.0, true);
      Interval<CcReal> minMax(ccmin, ccmax, true, true);
      set<Interval<CcReal>, ivCmp> ivSet;
      for (int i = firstpos; i <= lastpos; i++) {
        mreal->Get(i, ureal);
        minMax.start.Set(ureal.Min(correct));
        minMax.end.Set(ureal.Max(correct));
        ivSet.insert(minMax);
      }
      for (set<Interval<CcReal>, ivCmp>::iterator it = ivSet.begin();
           it != ivSet.end(); it++) {
        trange.MergeAdd(*it);
      }
      return Tools::relationHolds<CcReal>(trange,
                           *((Range<CcReal>*)values.first.addr), values.second);
    }
    return false;
  }
  else { // error
    cout << "TYPE is " << attrInfo.second << endl;
    return false;
  }
  return true;
}

/*
\subsection{Function ~valuesMatch~}

*/
bool TMatch::valuesMatch(const int u, const int atom, const bool checkMain) {
  SecInterval iv(true);
  if (checkMain) {
    if (!mainValuesMatch(u, atom)) {
      return false;
    }
  }
  GetInterval(u, iv);
  for (unsigned int pos = 0; pos < relevantAttrs.size(); pos++) {
    if (!otherValuesMatch(pos, iv, atom)) {
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~easyCondsMatch~}

*/
bool TMatch::easyCondsMatch(const int u, const int atom) {
  set<int> pos = p->getEasyCondPos(atom);
  if (p->elems[atom].getW() || pos.empty() || p->easyConds.empty()) {
    return true;
  }
  IndexMatchInfo imi(p->getElemFromAtom(atom), u);
  std::map<std::string, int> vte = p->getVarToElem();
  for (set<int>::iterator it = pos.begin(); it != pos.end(); it++) {
    switch (type) {
      case MLABEL: {
        MLabel *traj = (MLabel*)t->GetAttribute(attrno);
        if (!p->easyConds[*it].evaluate<MLabel>(imi, traj, vte, t, ttype)) {
          return false;
        }
        break;
      }
      case MLABELS: {
        MLabels *traj = (MLabels*)t->GetAttribute(attrno);
        if (!p->easyConds[*it].evaluate<MLabels>(imi, traj, vte, t, ttype)) {
          return false;
        }
        break;
      }
      case MPLACE: {
        MPlace *traj = (MPlace*)t->GetAttribute(attrno);
        if (!p->easyConds[*it].evaluate<MPlace>(imi, traj, vte, t, ttype)) {
          return false;
        }
        break;
      }
      case MPLACES: {
        MPlaces *traj = (MPlaces*)t->GetAttribute(attrno);
        if (!p->easyConds[*it].evaluate<MPlaces>(imi, traj, vte, t, ttype)) {
          return false;
        }
        break;
      }
    }
    
  }
  return true;
}

/*
\subsection{Function ~performTransitionsWithMatrix~}

*/
bool TMatch::performTransitionsWithMatrix(const int u, set<int>& states) {
  map<int, int> transitions;
  for (set<int>::iterator it = states.begin(); it != states.end(); it++) {
    map<int, int> trans = p->nfa[*it]; // collect possible transitions
    transitions.insert(trans.begin(), trans.end());
  }
  if (transitions.empty()) { // no possible transition available
    return false;
  }
  states.clear();
  map<int, int>::iterator it, itn;
  set<unsigned int>::iterator itu;
  if (u < GetNoMainComponents() - 1) { // usual case
    for (it = transitions.begin(); it != transitions.end(); it++) {
      if (p->elems[it->first].getW() != NO) {
        states.insert(states.end(), it->second);
        map<int, int> nextTrans = p->nfa[it->second];
        for (itn = nextTrans.begin(); itn != nextTrans.end(); itn++) {
          itu = pathMatrix[u][p->atomicToElem[it->first]].end();
          pathMatrix[u][p->atomicToElem[it->first]].insert
                                             (itu, p->atomicToElem[itn->first]);
        }
      }
      else {
        set<string> ivs;
        SecInterval iv;
        GetInterval(u, iv);
        p->elems[it->first].getI(ivs);
        if (Tools::timesMatch(iv, ivs) && valuesMatch(u, it->first, true)
            && easyCondsMatch(u, it->first)) {
          states.insert(states.end(), it->second);
          map<int, int> nextTrans = p->nfa[it->second];
          for (itn = nextTrans.begin(); itn != nextTrans.end(); itn++) {
            itu = pathMatrix[u][p->atomicToElem[it->first]].end();
            pathMatrix[u][p->atomicToElem[it->first]].insert
                                             (itu, p->atomicToElem[itn->first]);
          }
        }
      }
    }
  }
  else { // last row; mark final states with -1
    for (it = transitions.begin(); it != transitions.end(); it++) {
      if (p->elems[it->first].getW() != NO) {
        states.insert(states.end(), it->second);
        if (p->finalStates.count(it->second)) { // store last matching
          pathMatrix[u][p->atomicToElem[it->first]].insert(UINT_MAX);
        }
      }
      else {
        set<string> ivs;
        SecInterval iv;
        GetInterval(u, iv);
        p->elems[it->first].getI(ivs);
        if (Tools::timesMatch(iv, ivs) && valuesMatch(u, it->first, true)
            && easyCondsMatch(u, it->first)) {
          states.insert(states.end(), it->second);
          if (p->finalStates.count(it->second)) { // store last matching
            pathMatrix[u][p->atomicToElem[it->first]].insert(UINT_MAX);
          }
        }
      }
    }
  }
  return !states.empty();
}

/*
\subsection{Function ~performTransitions~}

*/
bool TMatch::performTransitions(const int u, set<int>& states) {
  map<int, int> transitions;
  for (set<int>::iterator it = states.begin(); it != states.end(); it++) {
    map<int, int> trans = p->nfa[*it]; // collect possible transitions
    transitions.insert(trans.begin(), trans.end());
  }
  if (transitions.empty()) { // no possible transition available
    return false;
  }
  states.clear();
  map<int, int>::iterator it;
  for (it = transitions.begin(); it != transitions.end(); it++) {
    if (p->elems[it->first].getW() != NO) {
      states.insert(states.end(), it->second);
    }
    else {
      set<string> ivs;
      SecInterval iv;
      GetInterval(u, iv);
      p->elems[it->first].getI(ivs);
      if (Tools::timesMatch(iv, ivs) && valuesMatch(u, it->first, true)
          && easyCondsMatch(u, it->first)) {
        states.insert(states.end(), it->second);
      }
    }
  }
  return !states.empty();
}

/*
\subsection{Function ~conditionsMatch~}

*/
bool TMatch::conditionsMatch(const IndexMatchInfo &imi) {
  Attribute *attr = t->GetAttribute(attrno);
  std::map<std::string, int> varToElem = p->getVarToElem();
  for (unsigned int i = 0; i < p->conds.size(); i++) {
    switch (type) {
      case MLABEL: {
        if (!p->conds[i].evaluate(imi, (MLabel*)attr, varToElem, t, ttype)) {
//           cout << "False cond " << p->conds[i].getText() << endl;
          return false;
        }
        break;
      }
      case MLABELS: {
        if (!p->conds[i].evaluate(imi, (MLabels*)attr, varToElem, t, ttype)) {
          return false;
        }
        break;
      }
      case MPLACE: {
        if (!p->conds[i].evaluate(imi, (MPlace*)attr, varToElem, t, ttype)) {
          return false;
        }
        break;
      }
      case MPLACES: {
        if (!p->conds[i].evaluate(imi, (MPlaces*)attr, varToElem, t, ttype)) {
          return false;
        }
        break;
      }
      default: { // cannot occur
        return false;
      }
    }
  }
//   cout << "True conditions!!!" << endl;
  return true;
}

/*
\subsection{Function ~findBinding~}

Recursively finds all bindings in the matching set matrix and checks whether
they fulfill every condition, stopping immediately after the first success.

*/
bool TMatch::findBinding(const int u, const int elem, IndexMatchInfo& imi) {
  bool inserted = imi.extendOrInsert(elem, u);
  if (*(pathMatrix[u][elem].begin()) == UINT_MAX) { // complete match
    if (conditionsMatch(imi)) {
      return true;
    }
  }
  else {
    for (set<unsigned int>::reverse_iterator it = pathMatrix[u][elem].rbegin();
         it != pathMatrix[u][elem].rend(); it++) {
      if (findBinding(u + 1, *it, imi)) {
        return true;
      }
    }
  }
  imi.reduceOrErase(elem, inserted);
  return false;
}


/*
\subsection{Function ~findMatchingBinding~}

Searches for a binding which fulfills every condition.

*/
bool TMatch::findMatchingBinding(const int startState) {
  if ((startState < 0) || (startState > (int)p->nfa.size() - 1)) {
    return false; // illegal start state
  }
  if (p->conds.empty()) {
    return true;
  }
  map<int, int> transitions = p->nfa[startState];
  IndexMatchInfo imi;
  map<int, int>::reverse_iterator itm;
  for (itm = transitions.rbegin(); itm != transitions.rend(); itm++) {
    if (findBinding(0, p->atomicToElem[itm->first], imi)) {
      return true;
    }
  }
  return false;
}

TypeConstructor tupleindexTC(
  TupleIndex<UnitPos, UnitPos>::BasicType(),
  TupleIndex<UnitPos, UnitPos>::Property,
  TupleIndex<UnitPos, UnitPos>::Out, TupleIndex<UnitPos, UnitPos>::In,
  0, 0,
  TupleIndex<UnitPos, UnitPos>::Create, TupleIndex<UnitPos,UnitPos>::Delete,
  TupleIndex<UnitPos, UnitPos>::Open, TupleIndex<UnitPos, UnitPos>::Save,
  TupleIndex<UnitPos, UnitPos>::Close, TupleIndex<UnitPos, UnitPos>::Clone,
  0,
  TupleIndex<UnitPos, UnitPos>::SizeOfObj,
  TupleIndex<UnitPos, UnitPos>::TypeCheck);

TypeConstructor tupleindex2TC(
  TupleIndex<NewInterval, UnitPos>::BasicType(),
  TupleIndex<NewInterval, UnitPos>::Property,
  TupleIndex<NewInterval, UnitPos>::Out,
  TupleIndex<NewInterval, UnitPos>::In,
  0, 0,
  TupleIndex<NewInterval, UnitPos>::Create,
  TupleIndex<NewInterval, UnitPos>::Delete,
  TupleIndex<NewInterval, UnitPos>::Open,
  TupleIndex<NewInterval, UnitPos>::Save,
  TupleIndex<NewInterval, UnitPos>::Close,
  TupleIndex<NewInterval, UnitPos>::Clone,
  0,
  TupleIndex<NewInterval, UnitPos>::SizeOfObj,
  TupleIndex<NewInterval, UnitPos>::TypeCheck);

/*
\section{Functions for class ~TMatchIndexLI~}

\subsection{Constructor for indextmatches}

*/
TMatchIndexLI::TMatchIndexLI(Relation *r, ListExpr tt, 
                             TupleIndex<UnitPos, UnitPos> *t, int a, 
                             Pattern *pat, int majorValueNo, DataType type) :
  IndexMatchSuper(r, pat, a, type), ttList(tt), ti(t), ti2(0),
                                            valueNo(majorValueNo), firstEnd(0) {
    TupleType *tupletype = rel->GetTupleType();
    relevantAttrs = Tools::getRelevantAttrs(tupletype, a, majorValueNo);
//     tupletype->DeleteIfAllowed();
}

/*
\subsection{Constructor for indextmatches2}

*/
TMatchIndexLI::TMatchIndexLI(Relation *r, ListExpr tt, 
                             TupleIndex<NewInterval, UnitPos> *t, Pattern *p) :
  IndexMatchSuper(r, p), ttList(tt), ti(0), ti2(t), valueNo(-1), firstEnd(0) {
    TupleType *tupletype = rel->GetTupleType();
    int dummy = INT_MIN;
    relevantAttrs = Tools::getRelevantAttrs(tupletype, -1, dummy);
    
//     tupletype->DeleteIfAllowed();
}

/*
\subsection{Destructor}

*/
TMatchIndexLI::~TMatchIndexLI() {
  firstEnd = 0;
}

/*
\subsection{Function ~tiCompatibleToRel~}

*/
bool TMatchIndexLI::tiCompatibleToRel() {
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  TupleType *ttype = rel->GetTupleType();
  IndexType indexType;
  unsigned int pos;
  pair<AttributeType, string> attrType;
  for (int i = 0; i < ttype->GetNoAttributes(); i++) {
    attrType.first = ttype->GetAttributeType(i);
    attrType.second = sc->GetTypeName(attrType.first.algId,
                                      attrType.first.typeId);
    indexType = (ti!= 0 ? ti->attrToIndex[i].first : ti2->attrToIndex[i].first);
    pos = (ti != 0 ? ti->attrToIndex[i].second : ti2->attrToIndex[i].second);
    switch (indexType) {
      case TRIE: {
        if (attrType.second != "mlabel" && attrType.second != "mplace" &&
            attrType.second != "mlabels" && attrType.second != "mplaces") {
          return false;
        }
        if (pos >= (ti != 0 ? ti->tries.size() : ti2->tries.size())) {
          return false;
        }
        break;
      }
      case BTREE: {
        if (attrType.second != "mint") {
          return false;
        }
        if (pos >= (ti != 0 ? ti->btrees.size() : ti2->btrees.size())) {
          return false;
        }
        break;
      }
      case RTREE1: {
        if (attrType.second != "mreal") {
          return false;
        }
        if (pos >= (ti != 0 ? ti->rtrees1.size() : ti2->rtrees1.size())) {
          return false;
        }
        break;
      }
      case RTREE2: {
        if (attrType.second != "mpoint" && attrType.second != "mregion") {
          return false;
        }
        if (pos >= (ti != 0 ? ti->rtrees2.size() : ti2->rtrees2.size())) {
          return false;
        }
        break;
      }
      default: { // case NONE
        break;
      }
    }
  }
  return true;
}

/*
\subsection{Function ~getNoComponents~}

*/
int TMatchIndexLI::getNoComponents(const TupleId tId, const int attrNo) {
  string type = nl->ToString(nl->Second(nl->Nth(attrNo+1, nl->Second(ttList))));
  if (type == "mlabel" || type == "mlabels" || type == "mplace" ||
    type ==  "mplaces") {
    return getTrajSize(tId, Tools::getDataType(type));
  }
  return Tools::getNoComponents(rel, tId, type, attrNo);
}

/*
\subsection{Function ~getFirstEnd~}

*/
Instant TMatchIndexLI::getFirstEnd(const TupleId id) {
  Instant temp(datetime::instanttype), result(datetime::instanttype);
  result.ToMaximum();
  Tuple *t = rel->GetTuple(id, false);
  for (unsigned int i = 0; i < relevantAttrs.size(); i++) {
    if (relevantAttrs[i].second == "mlabel") {
      ((MLabel*)t->GetAttribute(relevantAttrs[i].first))->FinalInstant(temp);
    }
    else if (relevantAttrs[i].second == "mlabels") {
      ((MLabels*)t->GetAttribute(relevantAttrs[i].first))->FinalInstant(temp);
    }
    else if (relevantAttrs[i].second == "mplace") {
      ((MPlace*)t->GetAttribute(relevantAttrs[i].first))->FinalInstant(temp);
    }
    else if (relevantAttrs[i].second == "mplaces") {
      ((MPlaces*)t->GetAttribute(relevantAttrs[i].first))->FinalInstant(temp);
    }
    else if (relevantAttrs[i].second == "mpoint") {
      ((MPoint*)t->GetAttribute(relevantAttrs[i].first))->FinalInstant(temp);
    }
    else if (relevantAttrs[i].second == "mregion") {
      ((MRegion*)t->GetAttribute(relevantAttrs[i].first))->FinalInstant(temp);
    }
    else if (relevantAttrs[i].second == "mbool") {
      ((MBool*)t->GetAttribute(relevantAttrs[i].first))->FinalInstant(temp);
    }
    else if (relevantAttrs[i].second == "mint") {
      ((MInt*)t->GetAttribute(relevantAttrs[i].first))->FinalInstant(temp);
    }
    else if (relevantAttrs[i].second == "mreal") {
      ((MReal*)t->GetAttribute(relevantAttrs[i].first))->FinalInstant(temp);
    }
    else {
      cout << "invalid type " << relevantAttrs[i].second << endl;
      return result;
    }
    if (temp < result) { // find minimum
      result = temp;
    }
  }
  t->DeleteIfAllowed();
  return result;
}

/*
\subsection{Function ~getResultForAtomPart~}

*/
void TMatchIndexLI::getResultForAtomPart(pair<int, pair<IndexType, int> >
      indexInfo, pair<Word, SetRel> values, string type, vector<Periods*> &prev,
      vector<Periods*> &result, const int prevCrucial, const bool mainAttr, 
      bool checkPrev /* = false */) {
  vector<set<int> > temp1, temp2;
  vector<Periods*> tempp2;
  if (mainAttr) {
    temp1.resize(rel->GetNoTuples() + 1);
    temp2.resize(rel->GetNoTuples() + 1);
  }
  else {
    tempp2.resize(rel->GetNoTuples() + 1, 0);
  }
  result.resize(rel->GetNoTuples() + 1);
  int valueNo = 0;
  bool proceed = false;
  if (mainAttr) {
    proceed = getSingleIndexResult(indexInfo, values, type, valueNo, temp1);
  }
  else {
    proceed = getSingleIndexResult(indexInfo, values, type, valueNo, result);
  }
  set<int> tmp;
  if (!proceed && !mainAttr) { // 
    
  }
  while (proceed) {
    valueNo++;
    if (mainAttr) {
      proceed = getSingleIndexResult(indexInfo, values, type, valueNo, temp2);
    }
    else {
      proceed = getSingleIndexResult(indexInfo, values, type, valueNo, tempp2);
    }
    switch (values.second) {
      case STANDARD:
      case INTERSECT: // unite intermediate results
        if (mainAttr) {
          for (int i = 1; i <= rel->GetNoTuples(); i++) {
            temp1[i].insert(temp2[i].begin(), temp2[i].end());
          }
        }
        else {
          Periods pertmp(true);
          for (int i = 1; i <= rel->GetNoTuples(); i++) {
            if (result[i] != 0 && tempp2[i] != 0) {
              if (!result[i]->IsOrdered()) {
                result[i]->EndBulkLoad(true);
              }
              if (!tempp2[i]->IsEmpty() && !tempp2[i]->IsOrdered()) {
                tempp2[i]->EndBulkLoad(true);
              }
              result[i]->Union(*(tempp2[i]), pertmp);
//               result[i]->CopyFrom(&pertmp);
              result[i]->DeleteIfAllowed();
              result[i] = (Periods*)pertmp.compress();
            }  
            else if (result[i] == 0 && tempp2[i] != 0) {
              if (!tempp2[i]->IsOrdered()) {
                tempp2[i]->EndBulkLoad(true);
              }
//               result[i] = new Periods(*(tempp2[i]));
              result[i] = (Periods*)(tempp2[i])->compress();
            }
          }
        }
        break;
      case SUPERSET:
      case EQUAL: // intersect intermediate results
        if (mainAttr) {
          for (int i = 1; i <= rel->GetNoTuples(); i++) {
            std::set_intersection(temp1[i].begin(), temp1[i].end(),
             temp2[i].begin(), temp2[i].end(), std::inserter(tmp, tmp.begin()));
            temp1[i] = tmp;
            tmp.clear();
          }
        }
        else {
          Periods pertmp(true);
          for (int i = 1; i <= rel->GetNoTuples(); i++) {
            if (result[i] != 0 && tempp2[i] != 0) {
              result[i]->EndBulkLoad(true);
              tempp2[i]->EndBulkLoad(true);
              result[i]->Intersection(*(tempp2[i]), pertmp);
              result[i]->CopyFrom(&pertmp);
            }
            else if (result[i] != 0) {
              result[i]->DeleteIfAllowed();
              result[i] = 0;
            }
          }
        }
        break;
      default: { // case DISJOINT: ignore
        break;
      }
    }
    if (mainAttr) {
      temp2.clear();
      temp2.resize(rel->GetNoTuples() + 1);
    }
    else {
      for (int i = 1; i <= rel->GetNoTuples(); i++) {
        if (tempp2[i] != 0) {
          tempp2[i]->DeleteIfAllowed();
          tempp2[i] = 0;
        }
      }
    }
  }
  if (mainAttr) {
    if (prevCrucial == -1) { // standard version
      if (!checkPrev) {
        for (int i = 1; i <= rel->GetNoTuples(); i++) {
          if (!temp1[i].empty()) {
            result[i] = new Periods(1);
            unitsToPeriods(temp1[i], i, indexInfo.first, result[i]);
          }
        }
      }
      else { // create periods iff necessary
        for (int i = 1; i <= rel->GetNoTuples(); i++) {
          if (!temp1[i].empty() && prev[i]) {
            result[i] = new Periods(1);
            unitsToPeriods(temp1[i], i, indexInfo.first, result[i]);
          }
        }
      }
    }
    else {
//     if (mainAttr) {
      if (!checkPrev) {
        for (int i = 1; i <= rel->GetNoTuples(); i++) {
          if (!temp1[i].empty() && indexResult[prevCrucial][i] != 0) {
            result[i] = new Periods(1);
            unitsToPeriods(temp1[i], i, indexInfo.first, result[i]);
          }
        }
      }
      else { // create periods iff necessary
        for (int i = 1; i <= rel->GetNoTuples(); i++) {
          if (!temp1[i].empty() && prev[i] && indexResult[prevCrucial][i] != 0){
            result[i] = new Periods(1);
            unitsToPeriods(temp1[i], i, indexInfo.first, result[i]);
          }
        }
      }
    }
//     else {
//       if (!checkPrev) {
//         for (int i = 1; i <= rel->GetNoTuples(); i++) {
//           if (!temp1[i].empty() && indexResult2[prevCrucial][i] != 0) {
//             result[i] = new Periods(1);
//             unitsToPeriods(temp1[i], i, indexInfo.first, result[i]);
//           }
//         }
//       }
//       else { // create periods iff necessary
//         for (int i = 1; i <= rel->GetNoTuples(); i++) {
//           if (!temp1[i].empty() && prev[i] && 
//               indexResult2[prevCrucial][i] != 0) {
//             result[i] = new Periods(1);
//             unitsToPeriods(temp1[i], i, indexInfo.first, result[i]);
//           }
//         }
//       }
//     }
  }
}

/*
\subsection{Function ~getResultForAtomTime~}

*/
bool TMatchIndexLI::getResultForAtomTime(const int atomNo,
                              const int prevCrucial, vector<Periods*> &result) {
  PatElem atom;
  p->getElem(atomNo, atom);
  set<string> ivs;
  atom.getI(ivs);
  if (ivs.empty()) {
    return false;
  }
  for (set<string>::iterator it = ivs.begin(); it != ivs.end(); it++) {
    if (!Tools::isInterval(*it)) {
      return false;
    }
  }
  SecInterval ivInst(true);
  CcReal start(true, 0);
  CcReal end(true, 0);
  vector<set<int> > temp1, temp2;
  temp1.resize(rel->GetNoTuples() + 1);
  temp2.resize(rel->GetNoTuples() + 1);
  set<int> tmp;
  bool first = true;
  bool indexApplied = false;
  for (set<string>::iterator it = ivs.begin(); it != ivs.end(); it++) {
    Tools::stringToInterval(*it, ivInst);
    start.Set(true, ivInst.start.ToDouble());
    end.Set(true, ivInst.end.ToDouble());
    if (!(start == end) || start.GetValue() != 0.0) {
      Interval<CcReal> iv(start, end, true, false);
      if (first) {
        Tools::queryRtree1(ti->timeIndex, iv, temp1);
        first = false;
      }
      else {
        Tools::queryRtree1(ti->timeIndex, iv, temp2);
        for (int i = 1; i <= rel->GetNoTuples(); i++) {
          std::set_intersection(temp1[i].begin(), temp1[i].end(),
             temp2[i].begin(), temp2[i].end(), std::inserter(tmp, tmp.begin()));
          temp1[i] = tmp;
          tmp.clear();
        }
      }
      indexApplied = true;
    }
  }
  if (!indexApplied) {
    return false;
  }
  if (prevCrucial == -1) {
    for (int i = 1; i <= rel->GetNoTuples(); i++) {
      if (!temp1[i].empty()) {
        result[i] = new Periods(0);
        unitsToPeriods(temp1[i], i, attrNo, result[i]);
      }
    }
  }
  else {
    for (int i = 1; i <= rel->GetNoTuples(); i++) {
      if (!temp1[i].empty() && indexResult[prevCrucial][i] != 0) {
        result[i] = new Periods(0);
        unitsToPeriods(temp1[i], i, attrNo, result[i]);
      }
    }
  }
  return true;
}

/*
\subsection{Function ~storeIndexResult~}

*/
void TMatchIndexLI::storeIndexResult(const int atomNo, const int prevCrucial,
                                     const bool mainAttr, int &noResults) {
  if (!mainAttr && indexResult2[atomNo] != 0) {
    return;
  }
  noResults = 0;
  PatElem atom;
  p->getElem(atomNo, atom);
  map<int, pair<IndexType, int> >::iterator it
               = (ti != 0 ? ti->attrToIndex.begin() : ti2->attrToIndex.begin());
  int noRelevantAttrs = 0;
  while (it != (ti != 0 ? ti->attrToIndex.end() : ti2->attrToIndex.end())) {
    if (it->second.first != NONE) {
      noRelevantAttrs++;
    }
    it++;
  }
  if (noRelevantAttrs != atom.getNoValues()) {
    cout << "Error: " << noRelevantAttrs << " != " << atom.getNoValues()<< endl;
    return;
  }
  vector<set<int> > result;
  vector<Periods*> periods, temp;
  if (mainAttr) {
    result.resize(rel->GetNoTuples() + 1);
  }
  periods.resize(rel->GetNoTuples() + 1, 0);
  temp.resize(rel->GetNoTuples() + 1, 0);
  int pos(0), pred(0);
  bool intersect = mainAttr ? 
                   getResultForAtomTime(atomNo, prevCrucial, periods) : false;
  Periods tmp(0);
  if (mainAttr) {
    if (indexResult[atomNo] == 0) {
      indexResult[atomNo] = new IndexRetrieval*[rel->GetNoTuples() + 1];
      memset(indexResult[atomNo], 0, (rel->GetNoTuples() + 1)*sizeof(void*));
    }
    indexResult[atomNo][0] = new IndexRetrieval(0, 0);
  }
  else {
    if (indexResult2[atomNo] == 0) {
      indexResult2[atomNo] = new IndexRetrieval2*[rel->GetNoTuples() + 1];
      memset(indexResult2[atomNo], 0, (rel->GetNoTuples() + 1)*sizeof(void*));
    }
    indexResult2[atomNo][0] = new IndexRetrieval2(0, 0);
  }
//   for (it = ti->attrToIndex.begin(); it != ti->attrToIndex.end(); it++) {
//     cout << it->first << " ---> " << it->second.second << endl;
//   }
  it = (ti != 0 ? ti->attrToIndex.begin() : ti2->attrToIndex.begin());
  while (it != (ti != 0 ? ti->attrToIndex.end() : ti2->attrToIndex.end())) {
    if (!intersect && it->second.second != -1 && 
        atom.values[pos].first.addr != 0) {
//       cout << "call gRFAP " << it->first << endl;
      getResultForAtomPart(*it, atom.values[pos], atom.types[pos], temp, 
                           periods, prevCrucial, mainAttr);
      intersect = true;
//       cout << atom.values.size() << "values, after " << it->first << ", pos "
//            << pos << ": ";
//       for (int i = 1; i <= rel->GetNoTuples(); i++) {
//         cout << result[i].size() << " ";
//       }
//       cout << endl;
    }
    else {
      if (it->second.second != -1 && atom.values[pos].first.addr != 0) {
//         cout << "call xgRFAP for attr " << it->first << endl;
        getResultForAtomPart(*it, atom.values[pos], atom.types[pos], periods,
                             temp, prevCrucial, mainAttr, true);
        for (int i = 1; i <= rel->GetNoTuples(); i++) {
          if (periods[i] && temp[i]) {
            if (!periods[i]->IsOrdered()) {
              periods[i]->EndBulkLoad(true);
            }
            if (!temp[i]->IsOrdered()) {
              temp[i]->EndBulkLoad(true);
            }
            periods[i]->Intersection(*temp[i], tmp);
            periods[i]->CopyFrom(&tmp);
            tmp.Clear();
            temp[i]->DeleteIfAllowed();
            temp[i] = 0;
          }
          else if (periods[i]) {
            periods[i]->DeleteIfAllowed();
            periods[i] = 0;
          }
          else if (temp[i]) {
            temp[i]->DeleteIfAllowed();
            temp[i] = 0;
          }
        }
        temp.clear();
        temp.resize(rel->GetNoTuples() + 1);
      }
    }
    if (it->second.first != NONE) {
      pos++;
    }
    it++;
  } // index information collected and intersected
  if (mainAttr) { // version for indextmatches
    if (prevCrucial == -1) { // standard version
      for (int i = 1; i <= rel->GetNoTuples(); i++) {
        periodsToUnits(periods[i], i, result[i]);
        if (periods[i]) {
          periods[i]->DeleteIfAllowed();
        }
        if (!result[i].empty()) {
          indexResult[atomNo][pred]->succ = i; // refresh succ of pred
          if (indexResult[atomNo][i] == 0) {
            indexResult[atomNo][i] = new IndexRetrieval(pred, i, result[i]);
            memset(indexResult[atomNo][i], 0, sizeof(void*));
            noResults++;
          }
          pred = i;
        }
      }
    }
    else { // process periods only if tuple id is still active and relevant
      for (int i = 1; i <= rel->GetNoTuples(); i++) {
        if (indexResult[prevCrucial][i] != 0) { // previous index result exists
          periodsToUnits(periods[i], i, result[i]);
          if (periods[i]) {
            periods[i]->DeleteIfAllowed();
          }
          if (!result[i].empty()) {
            indexResult[atomNo][pred]->succ = i; // refresh succ of pred
            if (indexResult[atomNo][i] == 0) {
              indexResult[atomNo][i] = new IndexRetrieval(pred, i, result[i]);
              memset(indexResult[atomNo][i], 0, sizeof(void*));
              noResults++;
            }
            pred = i;
          }
        }
        else if (periods[i]) {
          periods[i]->DeleteIfAllowed();
        }
      }
    }
  }
  else { // version for indextmatches2
    set<string> ivsAtom;
    atom.getI(ivsAtom);
    set<string>::iterator it = ivsAtom.begin();
    Periods perAtom(false), perSpec(false), perTemp(true);
    if (ivsAtom.empty()) {
      perAtom.SetDefined(false);
    }
    else {
      perAtom.SetDefined(true);
    }
    while (it != ivsAtom.end()) {
//       cout << "process |" << *it << "|" << endl;
      Tools::specToPeriods(*it, ti2->timeLimits, perSpec);
      if (it == ivsAtom.begin()) { // first specification
        perAtom.CopyFrom(&perSpec);
//         cout << "1st RESULT: " << perSpec << endl;
      }
      else {
        perAtom.Intersection(perSpec, perTemp);
        perAtom.CopyFrom(&perTemp);
        perTemp.Clear();
//         cout << "intermediate RESULT: " << perAtom << endl;
      }
      it++;
    }
    if (prevCrucial == -1) {
      for (int i = 1; i <= rel->GetNoTuples(); i++) {
        if (periods[i]) {
          if (!periods[i]->IsEmpty()) { // index result exists
            indexResult2[atomNo][pred]->succ = i;
            if (perAtom.IsDefined()) {
              if (!periods[i]->IsOrdered()) {
                periods[i]->EndBulkLoad(true, false);
              }
              periods[i]->Intersection(perAtom, perTemp);
              periods[i]->CopyFrom(&perTemp);
            }
            indexResult2[atomNo][i] = new IndexRetrieval2(pred, 0, periods[i]);
            noResults++;
            pred = i;
          }
          else { // remove empty results
            periods[i]->DeleteIfAllowed();
          }
        }
      }
    }
    else {
      for (int i = 1; i <= rel->GetNoTuples(); i++) {
        if (indexResult2[prevCrucial][i] != 0 && periods[i] != 0) {
          if (!periods[i]->IsEmpty()) { // prev. index result exists
            indexResult2[atomNo][pred]->succ = i; // refresh succ of pred
            if (perAtom.IsDefined()) {
              if (!periods[i]->IsOrdered()) {
                periods[i]->EndBulkLoad(true, false);
              }
              periods[i]->Intersection(perAtom, perTemp);
              periods[i]->CopyFrom(&perTemp);
            }
            indexResult2[atomNo][i] = new IndexRetrieval2(pred, 0, periods[i]);
            noResults++;
            pred = i;
          }
          else { // remove empty results
            periods[i]->DeleteIfAllowed();
          }
        }
        else if (periods[i] != 0) {
          periods[i]->DeleteIfAllowed();
        }
      }
    }
  }
  if (mainAttr) {
    indexResult[atomNo][pred]->succ = 0; // set successor to 0 for final id
  }
  else {
    indexResult2[atomNo][pred]->succ = 0;
  }
//   if (!mainAttr) {
//     cout << "index result for atom " << atomNo << ":" << endl;
//     for (int i = 1; i <= rel->GetNoTuples(); i++) {
//       if (indexResult2[atomNo][i] != 0) {
//         cout << "tuple " << i << ": " << indexResult2[atomNo][i]->pred
//             << ", " << indexResult2[atomNo][i]->succ << ", "
//             << *(indexResult2[atomNo][i]->per) << endl;
//       }
//     }
//   }
}

/*
\subsection{Function ~initMatchInfo~}

*/
void TMatchIndexLI::initMatchInfo(const bool mainAttr) {
  PatElem atom;
  for (set<int>::iterator it = crucialAtoms.begin(); it != crucialAtoms.end();
       it++) {
//     cout << "removed ids ";
    p->getElem(*it, atom);
    int oldId = 0;
    if (atom.isRelevantForTupleIndex()) {
      int id = mainAttr ? indexResult[*it][0]->succ : 
                          indexResult2[*it][0]->succ;
//       if (id == 0) { // no index result
//         cout << "crucial atom " << *it << ": no index result; EXIT" << endl;
//         return;
//       }
      while (id > 0) {
        for (int i = oldId + 1; i < id; i++) {
          removeIdFromIndexResult(i, mainAttr);
          active[i] = false;
        }
        oldId = id;
        id = mainAttr ? indexResult[*it][oldId]->succ : 
                        indexResult2[*it][oldId]->succ;
      }
      for (int i = oldId + 1; i <= rel->GetNoTuples(); i++) {
        removeIdFromIndexResult(i, mainAttr);
        active[i] = false;
//         cout << i << ",";
      }
    }
//     cout << " because of crucial atom " << *it << endl;
  }
  unsigned int pred = 0;
  DataType type;
  if (mainAttr) { // version for indextmatches
    matchInfo = new IndexMatchSlot**[p->getNFAsize()];
    memset(matchInfo, 0, p->getNFAsize() * sizeof(void*));
    newMatchInfo = new IndexMatchSlot**[p->getNFAsize()];
    memset(newMatchInfo, 0, p->getNFAsize() * sizeof(void*));
    for (int s = 0; s < p->getNFAsize(); s++) {
      matchInfo[s] = new IndexMatchSlot*[rel->GetNoTuples() + 1];
      memset(matchInfo[s], 0, (rel->GetNoTuples() + 1) * sizeof(void*));
      newMatchInfo[s] = new IndexMatchSlot*[rel->GetNoTuples() + 1];
      memset(newMatchInfo[s], 0, (rel->GetNoTuples() + 1) * sizeof(void*));
      matchInfo[s][0] = new IndexMatchSlot();
      newMatchInfo[s][0] = new IndexMatchSlot();
    }
    type = Tools::getDataType(rel->GetTupleType(), attrNo);
    for (TupleId id = 1; id <= (TupleId)rel->GetNoTuples(); id++) {
      if (active[id]) {
        activeTuples++;
        IndexMatchInfo imi(false);
        if (loopStates.find(0) != loopStates.end()) {
          imi.range = true;
        }
        for (int s = 0; s < p->getNFAsize(); s++) {
          matchInfo[s][id] = new IndexMatchSlot();
          matchInfo[s][id]->pred = pred;
          matchInfo[s][pred]->succ = id;
          newMatchInfo[s][id] = new IndexMatchSlot();
          newMatchInfo[s][id]->pred = pred;
          newMatchInfo[s][pred]->succ = id;
        }
        matchInfo[0][id]->imis.push_back(imi);
        pred = id;
        trajSize[id] = getTrajSize(id, type);
      }
    }
    for (int s = 0; s < p->getNFAsize(); s++) {
      matchInfo[s][pred]->succ = 0; // set succ of last active tid to 0
      newMatchInfo[s][pred]->succ = 0;
    }
  }
  else { // version for indextmatches2
    matchInfo2 = new IndexMatchSlot2**[p->getNFAsize()];
    memset(matchInfo2, 0, p->getNFAsize() * sizeof(void*));
    newMatchInfo2 = new IndexMatchSlot2**[p->getNFAsize()];
    memset(newMatchInfo2, 0, p->getNFAsize() * sizeof(void*));
    for (int s = 0; s < p->getNFAsize(); s++) {
      matchInfo2[s] = new IndexMatchSlot2*[rel->GetNoTuples() + 1];
      memset(matchInfo2[s], 0, (rel->GetNoTuples() + 1) * sizeof(void*));
      newMatchInfo2[s] = new IndexMatchSlot2*[rel->GetNoTuples() + 1];
      memset(newMatchInfo2[s], 0, (rel->GetNoTuples() + 1) * sizeof(void*));
      matchInfo2[s][0] = new IndexMatchSlot2();
      newMatchInfo2[s][0] = new IndexMatchSlot2();
    }
    for (TupleId id = 1; id <= (TupleId)rel->GetNoTuples(); id++) {
      if (active[id]) {
        activeTuples++;
        IndexMatchInfo2 imi;
        for (int s = 0; s < p->getNFAsize(); s++) {
          matchInfo2[s][id] = new IndexMatchSlot2();
          matchInfo2[s][id]->pred = pred;
          matchInfo2[s][pred]->succ = id;
          newMatchInfo2[s][id] = new IndexMatchSlot2();
          newMatchInfo2[s][id]->pred = pred;
          newMatchInfo2[s][pred]->succ = id;
        }
        matchInfo2[0][id]->imis.push_back(imi);
//         cout << "pushed back imi for id " << id << endl;
        pred = id;
      }
    }
    for (int s = 0; s < p->getNFAsize(); s++) {
      matchInfo2[s][pred]->succ = 0; // set succ of last active tid to 0
      newMatchInfo2[s][pred]->succ = 0;
    }
  }
}

/*
\subsection{Function ~atomMatch~}

*/
bool TMatchIndexLI::atomMatch(int state, pair<int, int> trans, 
                              const bool rewrite /* = false */) {
//   cout << "atomMatch(" << state << ", " << trans.first << ", " 
//        << trans.second << ") called" << endl;
  PatElem atom;
  p->getElem(trans.first, atom);
  set<string> ivs;
  atom.getI(ivs);
  SecInterval iv;
  vector<TupleId> toRemove;
  p->getElem(trans.first, atom);
  bool transition = false;
  IndexMatchInfo *imiPtr = 0;
  if (atom.isRelevantForTupleIndex()) {
    if (indexResult[trans.first] == 0) {
      int noResults;
      storeIndexResult(trans.first, -1, true, noResults);
    }
    TupleId id = indexResult[trans.first][0]->succ;
    int minUnit = INT_MAX;
    while (id > 0) {
      bool totalMatch = false;
      set<int>::reverse_iterator it = 
                                   indexResult[trans.first][id]->units.rbegin();
      while (active[id] && it != indexResult[trans.first][id]->units.rend()) {
        unsigned int numOfIMIs = (matchInfo[state][id] != 0 ?
                                  matchInfo[state][id]->imis.size() : 0);
        for (unsigned int i = 0; i < numOfIMIs; i++) {
          imiPtr = &matchInfo[state][id]->imis[i];
          bool ok = (imiPtr->range ? imiPtr->next <= *it : imiPtr->next == *it);
          if (ok) {
            Tuple *t = rel->GetTuple(id, false);
            TMatch tmatch(p, t, ttList, attrNo, relevantAttrs, valueNo);
            bool match = false;
            ExtBool matchRec = ST_UNDEF;
            if (matchRecord[trans.first][id] != 0) {
              matchRec = matchRecord[trans.first][id]->getMatchRecord(*it);
              if (matchRec == ST_TRUE) {
                match = true;
              }
            }
            else {
              matchRecord[trans.first][id] = new DoubleUnitSet();
            }
            if (matchRec == ST_UNDEF) {
              getInterval(id, *it, iv);
              match = tmatch.valuesMatch(*it, trans.first, false) &&
                      Tools::timesMatch(iv, ivs) &&
                      tmatch.easyCondsMatch(*it, trans.first);
              matchRecord[trans.first][id]->addMatchRecord(*it, match);
            }
            if (match) {
              transition = true;
              IndexMatchInfo newIMI(false, *it + 1, imiPtr->binding,
                                    imiPtr->prevElem);
              if (loopStates.find(trans.second) != loopStates.end()) {
                newIMI.range = true;
              }
              totalMatch = p->isFinalState(trans.second) &&
                           newIMI.finished(trajSize[id]);
              if (p->hasConds() || rewrite) {
                extendBinding(newIMI, trans.first, false, totalMatch ? id : 0);
                if (totalMatch && p->hasConds()) {
                  ExtBool condRec = ST_UNDEF;
                  if (condRecord[id] != 0) {
                    condRec = condRecord[id]->getCondRecord(newIMI);
                  }
                  else {
                    condRecord[id] = new DoubleBindingSet();
                  }
                  if (condRec != ST_UNDEF) {
                    totalMatch = (condRec == ST_TRUE ? true : false);
                  }
                  else {
                    TMatch tmatch(p, t, ttList, attrNo, relevantAttrs, valueNo);
                    totalMatch = tmatch.conditionsMatch(newIMI);
                    condRecord[id]->addCondRecord(newIMI, totalMatch);
                  }
                }
              }
              if (totalMatch) {
                if (rewrite) {
                  NewPair<TupleId, IndexMatchInfo> res(id, newIMI);
                  matchesR.push_back(res);
//                   cout << "Result: " << id << "; ";
//                   newIMI.print(true);
                }
                else {
                  matches.push_back(id); // complete match
  //                 cout << "MATCH for id " << id << endl;
                  toRemove.push_back(id);
      //         cout << id << " removed (wild match) " << activeTuples 
      //              << " active tuples" << endl;
                  i = numOfIMIs; // stop processing this tuple id
                  active[id] = false;
                }
              }
              else if (!newIMI.exhausted(trajSize[id])) { // continue
                newMatchInfo[trans.second][id]->imis.push_back(newIMI);
//                 cout << "Pushed back imi for id " << id << ", range="
//                      << (newIMI.range ? "TRUE" : "FALSE") << endl;
              }
              if (*it < minUnit) {
                minUnit = *it;
              }
            }
            else {
              if (imiPtr->range && canBeDeactivated(id, state, trans.first)) {
//                 cout << "*Deactivate id " << id << endl;
                toRemove.push_back(id);
              }
            }
            t->DeleteIfAllowed();
          }
        }
        it++;
      }
      TupleId oldId = id;
      id = indexResult[trans.first][id]->succ;
      if (canBeDeactivated(oldId, state, trans.first, true)) {
//         cout << "#### deactivate " << oldId << endl;
        toRemove.push_back(oldId);
      }
      remove(toRemove, true);
    }
    if (minUnit > unitCtr && minUnit < INT_MAX) {
      unitCtr = minUnit; //set counter to minimum of index results
    }
  }
  else { // consider all existing imi instances
//     cout << "not relevant for index; consider all instances; " 
//          << (*matchInfoPtr)[state][0].succ << " " 
//          <<  (*newMatchInfoPtr)[state][0].succ << endl;
    TupleId id = matchInfo[state][0]->succ; // first active tuple id
    while (id > 0) {
//        cout << ": " << matchInfo[state][id]->imis.size() 
//             << " IMIs, state=" << state << ", id=" << id << endl;
      unsigned int numOfIMIs = (matchInfo[state][id] != 0 ? 
                                matchInfo[state][id]->imis.size() : 0);
      unsigned int i = 0;
      while (active[id] && i < numOfIMIs) {
        bool totalMatch = false;
        imiPtr = &(matchInfo[state][id]->imis[i]);
        if (imiPtr->next + 1 >= unitCtr) {
          Tuple *t = rel->GetTuple(id, false);
          if (atom.getW() == NO) { // () or disjoint{...} or (monday _); no wc
            TMatch tmatch(p, t, ttList, attrNo, relevantAttrs, valueNo);
            if (imiPtr->range == false) { // exact matching required
              bool match = false;
              ExtBool matchRec = ST_UNDEF;
              if (matchRecord[trans.first][id] != 0) {
                matchRec = matchRecord[trans.first][id]->
                                                   getMatchRecord(imiPtr->next);
                if (matchRec == ST_TRUE) {
                  match = true;
                }
              }
              else {
                matchRecord[trans.first][id] = new DoubleUnitSet();
              }
              if (matchRec == ST_UNDEF) {
                getInterval(id, imiPtr->next, iv);
                match = tmatch.valuesMatch(imiPtr->next, trans.first, false) &&
                        Tools::timesMatch(iv, ivs) &&
                        tmatch.easyCondsMatch(imiPtr->next, trans.first);
                matchRecord[trans.first][id]->
                                            addMatchRecord(imiPtr->next, match);
              }
              if (match) {
                transition = true;
                IndexMatchInfo newIMI(false, imiPtr->next + 1, imiPtr->binding,
                                      imiPtr->prevElem);
                totalMatch = p->isFinalState(trans.second) &&
                             newIMI.finished(trajSize[id]);
                if (p->hasConds() || rewrite) {
                  extendBinding(newIMI, trans.first, false,totalMatch ? id : 0);
                  if (totalMatch && p->hasConds()) {
                    ExtBool condRec = ST_UNDEF;
                    if (condRecord[id] != 0) {
                      condRec = condRecord[id]->getCondRecord(newIMI);
                    }
                    else {
                      condRecord[id] = new DoubleBindingSet();
                    }
                    if (condRec != ST_UNDEF) {
                      totalMatch = (condRec == ST_TRUE ? true : false);
                    }
                    else {
                      TMatch tmatch(p, t, ttList, attrNo, relevantAttrs, 
                                    valueNo);
                      totalMatch = tmatch.conditionsMatch(newIMI);
                      condRecord[id]->addCondRecord(newIMI, totalMatch);
                    }
                  }
                }
                if (totalMatch) {
                  if (rewrite) {
                    NewPair<TupleId, IndexMatchInfo > res(id, newIMI);
                    matchesR.push_back(res);
//                     cout << "Result: " << id << "; ";
//                     newIMI.print(true);
                  }
                  else {
                    matches.push_back(id); // complete match
                    toRemove.push_back(id);
        //         cout << id << " removed (wild match) " << activeTuples 
        //              << " active tuples" << endl;
                    i = numOfIMIs; // stop processing this tuple id
                    active[id] = false;
                  }
                }
                else if (!newIMI.exhausted(trajSize[id])) { // continue
                  newMatchInfo[trans.second][id]->imis.push_back(newIMI);
                }
              }
              else {
                if (imiPtr->range && canBeDeactivated(id, state, trans.first)) {
//                   cout << "*Deactivate id " << id << endl;
                  toRemove.push_back(id);
                }
              }
            }
            else { // wildcard before this transition; check all successors
              int k = imiPtr->next;
              while (k < trajSize[id]) {
                bool match = false;
                ExtBool matchRec = ST_UNDEF;
                if (matchRecord[trans.first][id] != 0) {
                  matchRec = matchRecord[trans.first][id]->
                                                   getMatchRecord(k);
                  if (matchRec == ST_TRUE) {
                    match = true;
                  }
                }
                else {
                  matchRecord[trans.first][id] = new DoubleUnitSet();
                }
                if (matchRec == ST_UNDEF) {
                  getInterval(id, k, iv);
                  match = tmatch.valuesMatch(k, trans.first, false) &&
                          Tools::timesMatch(iv, ivs) &&
                          tmatch.easyCondsMatch(k, trans.first);
                  matchRecord[trans.first][id]-> addMatchRecord(k, match);
                }
                if (match) {
                  transition = true;
                  IndexMatchInfo newIMI(false, k + 1, imiPtr->binding,
                                        imiPtr->prevElem);
                  totalMatch = p->isFinalState(trans.second) &&
                               newIMI.finished(trajSize[id]);
                  if (p->hasConds() || rewrite) {
                    extendBinding(newIMI, trans.first, false, totalMatch?id:0);
                    if (totalMatch && p->hasConds()) {
                      ExtBool condRec = ST_UNDEF;
                      if (condRecord[id] != 0) {
                        condRec = condRecord[id]->getCondRecord(newIMI);
                      }
                      else {
                        condRecord[id] = new DoubleBindingSet();
                      }
                      if (condRec != ST_UNDEF) {
                        totalMatch = (condRec == ST_TRUE ? true : false);
                      }
                      else {
                        TMatch tmatch(p, t, ttList, attrNo, relevantAttrs, 
                                      valueNo);
                        totalMatch = tmatch.conditionsMatch(newIMI);
                        condRecord[id]->addCondRecord(newIMI, totalMatch);
                      }
                    }
                  }
                  if (totalMatch) {
                    if (rewrite) {
                      NewPair<TupleId, IndexMatchInfo> res(id, newIMI);
                      matchesR.push_back(res);
//                       cout << "Result: " << id << "; ";
//                       newIMI.print(true);
                    }
                    else {
                      matches.push_back(id); // complete match
  //                     cout << "MATCH for id " << id << " !!" << endl;
                      toRemove.push_back(id);
        //         cout << id << " removed (wild match) " << activeTuples 
        //              << " active tuples" << endl;
                      k = trajSize[id];
                      i = numOfIMIs; // stop processing this tuple id
                      active[id] = false;
                    }
                  }
                  else if (!newIMI.exhausted(trajSize[id])) { // continue
                    newMatchInfo[trans.second][id]->imis.push_back(newIMI);
                  }
                  else {
                    if (canBeDeactivated(id, state, trans.first)) {
//                     cout << "Deactivate id " << id << endl;
                      toRemove.push_back(id);
                    }
                  }
                  k++;
                }
              }
            }
          }
          else { // wildcard
            transition = true;
            IndexMatchInfo newIMI(true, imiPtr->next + 1, imiPtr->binding,
                                  imiPtr->prevElem);
            totalMatch = p->isFinalState(trans.second) &&
                         newIMI.finished(trajSize[id]);
            if (p->hasConds() || rewrite) {
              extendBinding(newIMI, trans.first, true, totalMatch ? id : 0);
              if (totalMatch && p->hasConds()) {
                ExtBool condRec = ST_UNDEF;
                if (condRecord[id] != 0) {
                  condRec = condRecord[id]->getCondRecord(newIMI);
                }
                else {
                  condRecord[id] = new DoubleBindingSet();
                }
                if (condRec != ST_UNDEF) {
                  totalMatch = (condRec == ST_TRUE ? true : false);
                }
                else {
                  TMatch tmatch(p, t, ttList, attrNo, relevantAttrs, valueNo);
                  totalMatch = tmatch.conditionsMatch(newIMI);
                  condRecord[id]->addCondRecord(newIMI, totalMatch);
                }
              }
            }
            if (totalMatch) {
              if (rewrite) {
                NewPair<TupleId, IndexMatchInfo> res(id, newIMI);
                matchesR.push_back(res);
//                 cout << "Result: " << id << "; ";
//                 newIMI.print(true);
              }
              else {
                matches.push_back(id); // complete match
  //               cout << "complete match for id " << id << endl;
                toRemove.push_back(id);
        //         cout << id << " removed (wild match) " << activeTuples 
        //              << " active tuples" << endl;
                i = numOfIMIs; // stop processing this tuple id
                active[id] = false;
              }
            }
            else if (!newIMI.exhausted(trajSize[id])) { // continue
              newMatchInfo[trans.second][id]->imis.push_back(newIMI);
            }
          }
          t->DeleteIfAllowed();
        }
        i++;
      }
      if (active[id]) {
        if (!hasIdIMIs(id, true, state)) { // no IMIs for id and state
          if (!hasIdIMIs(id, true)) { // no IMIs at all for id
            toRemove.push_back(id);
    //         cout << id << " removed (wild mismatch) " << activeTuples 
    //              << " active tuples" << endl;
          }
        }
      }
      id = matchInfo[state][id]->succ;
      remove(toRemove, true);
    }
  }
//   cout << "........ return " << (transition ? "TRUE" : "FALSE") << endl;
  return transition;
}

/*
\subsection{Function ~extendBinding2~}

*/
void TMatchIndexLI::extendBinding2(Periods *per, const int elem, 
                                  const bool totalMatch, IndexMatchInfo2& imi) {
  if (imi.binding.find(elem) == imi.binding.end()) { // elem not bound yet
    imi.binding.insert(make_pair(elem, *per));
    Periods inter(true);
    Instant start(datetime::instanttype), end(datetime::instanttype);
    if (elem == 0) { // *X Y (__)* ...
      start.ToMinimum();
      per->Minimum(end);
      SecInterval iv(start, end, false, false);
      inter.Add(iv);
    }
    else { // ... *Y (__)* ...
      Periods lastBinding(true), unionResult(true);
      imi.getBinding(elem - 1, lastBinding);
      lastBinding.Minimum(start);
      per->Maximum(end);
      SecInterval iv(start, end, false, false);
      inter.Add(iv);
      if (!lastBinding.IsOrdered()) {
        cout << "sort now" << endl;
        lastBinding.EndBulkLoad(true, false);
        cout << lastBinding.IsOrdered() << endl;
      }
      lastBinding.Union(*per, unionResult);
      inter.Minus(unionResult); // [p_B.start, p_D.end] \ (p_B U p_D)
    } // bind intervals between elements
    imi.binding.insert(make_pair(-1 * elem - 1, inter));
    if (totalMatch && elem == p->getNoElems() - 1) { // ... *X (__)* Y
      Periods after(true);
      bool leftclosed = false;
      per->Maximum(start);
      end.ToMaximum();
      if (!per->IsEmpty()) {
        SecInterval previousIv(start, end, false, false);
        per->Get(per->GetNoComponents() - 1, previousIv);
        leftclosed = !previousIv.rc;
      }
      SecInterval iv(start, end, leftclosed, false);
      after.Add(iv);
      imi.binding.insert(make_pair(-1 * elem - 2, after));
    }
  }
  else { // extend existing binding; ... X [(__) | (__) (__) *(__)*] ...
    Periods unionResult(true);
    imi.binding[elem].Union(*per, imi.binding[elem]);
  }
//   cout << "elem " << elem << ": "; imi.print();
}

/*
\subsection{Function ~canBeDeactivated2~}

*/
bool TMatchIndexLI::canBeDeactivated2(const TupleId id, const int state,
                                      const int atom) {
  if (crucialAtoms.find(atom) != crucialAtoms.end()) {
    for (int i = state + 1; i < p->getNFAsize(); i++) {
      if (matchInfo2[i] != 0) {
        if (matchInfo2[i][id] != 0) {
          if (!matchInfo2[i][id]->imis.empty()) {
//             cout << "------------DO NOT deactivate id " << id 
//                  << ", has mI[" << i << "]" << endl;
            return false;
          }
        }
      }
      if (newMatchInfo2[i] != 0) {
        if (newMatchInfo2[i][id] != 0) {
          if (!newMatchInfo2[i][id]->imis.empty()) {
//             cout << "-----------DO NOT deactivate id " << id 
//                  << ", has nMI[" << i << "]" << endl;
            return false;
          }
        }
      }
    }
  }
  return true;
}

/*
\subsection{Function ~geoMatch~}

Checks whether geometric specifications from a pattern atom match the
corresponding information from the tuple. Applied for indextmatches2

*/
bool TMatchIndexLI::geoMatch(const int atomNo, Tuple *t, Periods *per) {
  PatElem atom;
  for (unsigned int i = 0; i < relevantAttrs.size(); i++) {
    if (relevantAttrs[i].second == "mpoint") {
      p->getElem(atomNo, atom);
      pair<Word, SetRel> values = atom.values[i];
      if (values.first.addr != 0) {
        MPoint *mpoint = (MPoint*)t->GetAttribute(relevantAttrs[i].first);
        MPoint mpAtPer(true);
        mpoint->AtPeriods(*per, mpAtPer);
        if (mpAtPer.IsEmpty()) {
          return false;
        }
        if (!((Region*)values.first.addr)->IsEmpty()) {
          if (!Tools::relationHolds(mpAtPer, *((Region*)values.first.addr), 
                                    values.second)) {
            return false;
          }
        }
      }
    }
    else if (relevantAttrs[i].second == "mregion") {
      p->getElem(atomNo, atom);
      pair<Word, SetRel> values = atom.values[i];
      if (values.first.addr != 0) {
        MRegion *mreg = (MRegion*)t->GetAttribute(relevantAttrs[i].first);
        MRegion mrAtPer(true);
        mreg->AtPeriods(per, &mrAtPer);
        if (mrAtPer.IsEmpty()) {
          return false;
        }
        if (!((Region*)values.first.addr)->IsEmpty()) {
          if (!Tools::relationHolds(mrAtPer, *((Region*)values.first.addr), 
                                    values.second)) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

/*
\subsection{Function ~evaluateInstant~}

applied for indextmatches2

*/
bool Condition::evaluateInstant(const ListExpr tt, Tuple *t, 
                                IndexMatchInfo2& imi) {
  Word qResult;
  for (int i = 0; i < getVarKeysSize(); i++) {
    int key = getKey(i);
    if (key > 99) { // reference to attribute of tuple, e.g., X.Trip
      if (Tools::isMovingAttr(tt, key - 99)) {
        string type = nl->ToString(nl->Second(nl->Nth(key-99, nl->Second(tt))));
        getConstValue(t->GetAttribute(key - 100), type, imi.inst, pointers[i]);
      }
      else { 
        pointers[i]->CopyFrom(t->GetAttribute(key - 100));
      }
    }
    else { // reference to instant; X.time
      ((Periods*)pointers[i])->DeleteIfAllowed();
      pointers[i] = new datetime::DateTime(datetime::instanttype);
      *((Instant*)pointers[i]) = imi.inst;
    }
  }
//   cout << nl->ToString(qp->ListOfTree(getOpTree(), cout)) << endl;
  getQP()->EvalS(getOpTree(), qResult, OPEN);
//   cout << "result for |" << text << "| is "
//        << (((CcBool*)qResult.addr)->GetValue() ? "TRUE" : "FALSE") << endl;
  return ((CcBool*)qResult.addr)->GetValue();
}

/*
\subsection{Function ~evaluatePeriods~}

*/
bool Condition::evaluatePeriods(const ListExpr tt, Tuple *t, Periods *per) {
  Word qResult;
//   cout << "evaluate cond |" << text << "| ;" << *per << endl;
  for (int i = 0; i < getVarKeysSize(); i++) {
    int key = getKey(i);
    if (key > 99) { // reference to attribute of tuple, e.g., X.Trip
      if (Tools::isMovingAttr(tt, key - 99)) {
        string type = nl->ToString(nl->Second(nl->Nth(key-99, nl->Second(tt))));
        copyAndRestrictPtr(i, t, tt, key - 99, *per);
      }
      else { // constant attribute, e.g., X.Id
        pointers[i]->CopyFrom(t->GetAttribute(key - 100));
      }
    }
    else {
      Interval<Instant> iv(true);
      switch (key) {
        case 2: {
          ((Periods*)pointers[i])->CopyFrom(per);
          break;
        }
        case 3: {
          per->Minimum(*((Instant*)pointers[i]));
          break;
        }
        case 4: {
          per->Maximum(*((Instant*)pointers[i]));
          break;
        }
        case 5: {
          if (per->IsEmpty()) {
            ((CcBool*)pointers[i])->SetDefined(false);
          }
          else {
            per->Get(0, iv);
            ((CcBool*)pointers[i])->Set(true, iv.lc);
          }
          break;
        }
        case 6: {
          if (per->IsEmpty()) {
            ((CcBool*)pointers[i])->SetDefined(false);
          }
          else {
            per->Get(per->GetNoComponents() - 1, iv);
            ((CcBool*)pointers[i])->Set(true, iv.rc);
          }
          break;
        }
        default: {
          cout << "Error: invalid key " << key << " in condition" << endl;
          return false;
        }
      }
    }
  }
  return true;
//   getQP()->EvalS(getOpTree(), qResult, OPEN);
//   cout << "result for |" << text << "| is "
//        << (((CcBool*)qResult.addr)->GetValue() ? "TRUE" : "FALSE") << endl;
//   return ((CcBool*)qResult.addr)->GetValue();
}

/*
\subsection{Function ~easyCondsMatch~}

applied for indextmatches2

*/
bool TMatchIndexLI::easyCondsMatch(const int atomNo, Tuple *t, Periods *per) {
  set<int> pos = p->getEasyCondPos(atomNo);
  if (pos.empty() || p->easyConds.empty()) {
    return true;
  }
  MBool mbool(true);
  Periods tempPer(true), interResult(true);
  Word qResult;
  for (set<int>::iterator it = pos.begin(); it != pos.end(); it++) {
    if (!p->easyConds[*it].evaluatePeriods(ttList, t, per)) {
      return false;
    }
    p->easyConds[*it].getQP()->EvalS(p->easyConds[*it].getOpTree(), qResult, 
                                     OPEN);
    if (((MBool*)qResult.addr)->IsDefined() && 
        ((MBool*)qResult.addr)->IsOrdered()) {
      CcBool *cctrue = new CcBool(true, true);
      ((MBool*)qResult.addr)->At(*cctrue, mbool);
      cctrue->DeleteIfAllowed();
      if (mbool.IsEmpty()) {
        return false;
      }
      else {
        if (interResult.IsEmpty()) { // no intermediate result existing
          mbool.DefTime(interResult);
        }
        else { // compute intersection with previous result
          mbool.DefTime(tempPer);
          interResult.Intersection(tempPer, interResult);
        }
        if (interResult.IsEmpty()) {
//           cout << "empty intersection result after cond #" << *it << endl;
          return false;
        }
//         cout << "intersection after easy cond #" << *it << ": "
//              << interResult << endl;
      }
    }
    else if (((CcBool*)qResult.addr)->IsDefined()) {
      if (!((CcBool*)qResult.addr)->GetValue()) {
//         cout << "CcBool is defined and false" << endl;
        return false;
      }
    }
    else {
      cout << "invalid result type for condition #" << *it << endl;
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~copyPtrFromAttr~}

Used for indextmatches2.

*/
bool Condition::copyPtrFromAttr(const int pos, Attribute *attr) {
  if (pos < 0 || pos >= (int)pointers.size()) {
    return false;
  }
  pointers[pos]->CopyFrom(attr);
  return true;
}

/*
\subsection{Function ~copyAndRestrictPtr~}

Used for indextmatches2.

*/
void Condition::copyAndRestrictPtr(const int pos, Tuple *tuple, 
                      const ListExpr ttype, const int key, const Periods& per) {
  string attrtype = nl->ToString(nl->Second(nl->Nth(key, 
                                        nl->Second(ttype))));
  if (instantVars.find(getVar(pos)) == instantVars.end()) { // interval variable
    if (attrtype == "mbool") {
      ((MBool*)tuple->GetAttribute(key - 1))->AtPeriods(per,
                                                      *((MBool*)pointers[pos]));
    }
    else if (attrtype == "mint") {
      ((MInt*)tuple->GetAttribute(key - 1))->AtPeriods(per,
                                                       *((MInt*)pointers[pos]));
    }
    else if (attrtype == "mlabel") {
      ((MLabel*)tuple->GetAttribute(key - 1))->AtPeriods(per, 
                                                     *((MLabel*)pointers[pos]));
    }
    else if (attrtype == "mlabels") {
      ((MLabels*)tuple->GetAttribute(key - 1))->AtPeriods(per, 
                                                    *((MLabels*)pointers[pos]));
    }
    else if (attrtype == "mplace") {
      MPlace mp(true);
      Place::base value;
      Region tmp(true);
      std::string type;
      Word geo;
      ((MPlace*)tuple->GetAttribute(key - 1))->AtPeriods(per, mp);
      for (int i = 0; i < mp.GetNoComponents(); i++) {
        mp.GetValue(i, value);
        if (value.second > 0) {
          Tools::getGeoFromORel("Places", value.second, false, geo, type);
          if (type == "point") {
            tmp.Union(*((Point*)geo.addr), (*((Region*)pointers[pos])));
          }
          else if (type == "line") {
            tmp.Union(*((Line*)geo.addr), (*((Region*)pointers[pos])));
          }
          else if (type == "region") {
            tmp.Union(*((Region*)geo.addr), (*((Region*)pointers[pos])));
          }
          else {
            cout << "ERROR: type is " << type << endl; // cannot occur
          }
        }
      }
    }
    else if (attrtype == "mplaces") {
      ((MPlaces*)tuple->GetAttribute(key - 1))->AtPeriods(per, 
                                                    *((MPlaces*)pointers[pos]));
    }
    else if (attrtype == "mpoint") {
      ((temporalalgebra::MPoint*)tuple->GetAttribute(key - 1))->AtPeriods(per, 
                                    *((temporalalgebra::MPoint*)pointers[pos]));
    }
    else if (attrtype == "mreal") {
      ((temporalalgebra::MReal*)tuple->GetAttribute(key - 1))->AtPeriods(per, 
                                  *((temporalalgebra::MReal*)pointers[pos]));
    }
    else if (attrtype == "mregion") {
      ((temporalalgebra::MRegion*)tuple->GetAttribute(key - 1))->AtPeriods(&per,
                                      (temporalalgebra::MRegion*)pointers[pos]);
    }
  }
  else { // variable refers to an instant; delete old pointer
    Instant inst;
    per.Minimum(inst);
    getConstValue(tuple->GetAttribute(key - 1), attrtype, inst, pointers[pos]);
  }
}

/*
\subsection{Function ~setPtrToTimeValue~}

Used for indextmatches2.

*/
void Condition::setPtrToTimeValue(const int pos, const Periods& per) {
  SecInterval iv(true);
  Instant inst(datetime::instanttype);
  if (!per.IsDefined()) {
    clearTimePtr(pos);
    return;
  }
  if (per.IsEmpty()) {
    clearTimePtr(pos);
    return;
  }
  switch (getKey(pos)) {
    case 2: { // time
      clearTimePtr(pos);
      setTimePtr(pos, per);
      break;
    }
    case 3: { // start
      per.Minimum(inst);
      setStartEndPtr(pos, inst);
      break;
    }
    case 4: { // end
      per.Maximum(inst);
      setStartEndPtr(pos, inst);
      break;
    }
    case 5: { // leftclosed
      Interval<Instant> iv(true);
      per.Get(0, iv);
      setLeftRightclosedPtr(pos, iv.lc);
      break;
    }
    case 6: { // rightclosed
      Interval<Instant> iv(true);
      per.Get(per.GetNoComponents() - 1, iv);
      setLeftRightclosedPtr(pos, iv.rc);
      break;
    }
    default: {
      cout << "ERROR: key " << getKey(pos) << " is invalid" << endl;
    }
  }
}

/*
\subsection{Function ~condsMatch~}

Used for indextmatches2.

*/
bool TMatchIndexLI::condsMatch(Tuple *t, const IndexMatchInfo2& imi) {
  if (!p->hasConds()) {
    return true;
  }
  Word qResult;
  vector<Condition> *conds = p->getConds();
  Periods bindPer(true), interResult(true), tempPer(true);
  interResult.Clear();
  MBool mbool(true);
  for (unsigned int i = 0; i < conds->size(); i++) {
//     cout << "evaluate cond " << i << " % " << conds->at(i).getText() << endl;
//     conds->at(i).evaluate(t, imi);
    for (int j = 0; j < conds->at(i).getVarKeysSize(); j++) {
      int elem = p->getElemFromVar(conds->at(i).getVar(j));
      if (!imi.getBinding(elem, bindPer)) {
        cout << "ERROR, binding for elem " << elem << " does not exist" << endl;
        return false;
      }
      int key = conds->at(i).getKey(j);
//       cout << conds->at(i).getVar(j) << " $ " << key << " $ "
//            << elem << " $ " << iv << endl;
      if (key > 99) { // reference to attribute, e.g., X.Pos
        if (!t) {
          cout << "tuple not found" << endl;
          return false;
        }
        if (Tools::isMovingAttr(ttList, key - 99)) {
//           cout << "restrict ptr to " << bindPer << endl;
          conds->at(i).copyAndRestrictPtr(j, t, ttList, key - 99, bindPer);
        }
        else { // constant attribute type
          if (!conds->at(i).copyPtrFromAttr(j, t->GetAttribute(key - 100))) {
            cout << "ERROR: attribute " << key - 100 << " not copied" << endl;
            return false;
          }
        }
      }
      else { // X.time, X.start, X.end, X.leftclosed, X.rightclosed
        conds->at(i).setPtrToTimeValue(j, bindPer);
      }
    }
    conds->at(i).getQP()->EvalS(conds->at(i).getOpTree(), qResult, OPEN);
    if (((MBool*)qResult.addr)->IsDefined() && 
        ((MBool*)qResult.addr)->IsOrdered()) {
      CcBool *cctrue = new CcBool(true, true);
      ((MBool*)qResult.addr)->At(*cctrue, mbool);
      cctrue->DeleteIfAllowed();
      if (interResult.IsEmpty()) { // no intermediate result existing
        mbool.DefTime(interResult);
      }
      else { // compute intersection with previous result
        mbool.DefTime(tempPer);
        interResult.Intersection(tempPer, interResult);
      }
      if (interResult.IsEmpty()) {
//         cout << "empty intersection result after cond #" << i << endl;
        return false;
      }
//      cout << "intersection after cond #" << i << ": " << interResult << endl;
    }
    else if (((CcBool*)qResult.addr)->IsDefined()) {
      if (!((CcBool*)qResult.addr)->GetValue()) {
//         cout << "CcBool is defined and false" << endl;
        return false;
      }
    }
    else {
      cout << "invalid result type for condition #" << i << endl;
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~atomMatch2~}

*/
bool TMatchIndexLI::atomMatch2(const int state, std::pair<int, int> trans) {
//     cout << "atomMatch(" << state << ", " << trans.first << ", " 
//        << trans.second << ") called" << endl;
  PatElem atom;
  set<string> ivs;
  atom.getI(ivs);
  SecInterval iv;
  vector<TupleId> toRemove;
  p->getElem(trans.first, atom);
  bool transition = false;
  IndexMatchInfo2 *imiPtr = 0;
  Periods *per = 0;
//   if (atom.isRelevantForTupleIndex()) {
    if (indexResult2[trans.first] == 0) { // no index result found yet
      int noResults;
      storeIndexResult(trans.first, -1, false, noResults);
    }
    TupleId id = indexResult2[trans.first][0]->succ; // id of first index result
    Instant minInst(datetime::instanttype);
    minInst.ToMaximum();
    while (id > 0) {
      bool totalMatch = false;
      per = indexResult2[trans.first][id]->per; // retrive periods from index
      if (active[id]) { // process only active tuples
        unsigned int numOfIMIs = (matchInfo2[state][id] != 0 ?
                                  matchInfo2[state][id]->imis.size() : 0);
        for (unsigned int i = 0; i < numOfIMIs; i++) {
          imiPtr = &matchInfo2[state][id]->imis[i];
          if (imiPtr->checkPeriods(per)) {
            Tuple *t = rel->GetTuple(id, false);
            IndexMatchInfo2 newIMI(imiPtr, per);
            bool match = geoMatch(trans.first, t, per) &&
                         easyCondsMatch(trans.first, t, per);
            if (match) {
              transition = true;
              totalMatch = p->isFinalState(trans.second);
//               if (p->hasConds()) {
                extendBinding2(per, p->getElemFromAtom(trans.first), totalMatch,
                               newIMI);
                if (totalMatch && p->hasConds()) {
                  totalMatch = condsMatch(t, newIMI);
                }
//               }
              if (totalMatch) {
                matches.push_back(id); // complete match
//                 cout << "MATCH for id " << id << endl;
                toRemove.push_back(id);
    //         cout << id << " removed (wild match) " << activeTuples 
    //              << " active tuples" << endl;
                i = numOfIMIs; // stop processing this tuple id
                active[id] = false;
              }
              else if (!newIMI.exhausted((*firstEnd)[id])) { // continue
                newMatchInfo2[trans.second][id]->imis.push_back(newIMI);
//                 cout << "Pushed back imi for id " << id << ", range="
//                      << (newIMI.range ? "TRUE" : "FALSE") << endl;
              }
              if (newIMI.inst < minInst) {
                minInst = newIMI.inst;
              }
            }
            else {
              if (canBeDeactivated2(id, state, trans.first)) {
//                 cout << "*Deactivate id " << id << endl;
                toRemove.push_back(id);
              }
            }
            t->DeleteIfAllowed();
          }
        }
      }
      TupleId oldId = id;
      id = indexResult2[trans.first][id]->succ;
      if (canBeDeactivated2(oldId, state, trans.first)) {
//         cout << "#### deactivate " << oldId << endl;
        toRemove.push_back(oldId);
      }
      remove(toRemove, false);
    }
    if (minInst.millisecondsToNull() > (*firstEnd)[0] && !minInst.IsMaximum()) {
      (*firstEnd)[0] = minInst.millisecondsToNull();
    } // set global limit to minimum of index results
//   }
//   else { // consider all existing imi instances
//     TupleId id = matchInfo2[state][0]->succ; // first active tuple id
//     while (id > 0) {
// //        cout << ": " << matchInfo[state][id]->imis.size() 
// //             << " IMIs, state=" << state << ", id=" << id << endl;
//       unsigned int numOfIMIs = (matchInfo2[state][id] != 0 ? 
//                                 matchInfo2[state][id]->imis.size() : 0);
//       unsigned int i = 0;
//       while (active[id] && i < numOfIMIs) {
//         bool totalMatch = false;
//         imiPtr = &(matchInfo2[state][id]->imis[i]);
//         Tuple *t = rel->GetTuple(id, false);
//         // TODO: check symbolic time specs and easy conditions
// //         TMatch tmatch(p, t, ttList, attrNo, relevantAttrs, valueNo);
//         bool match = true;
// //         match = tmatch.valuesMatch(imiPtr->next, trans.first, false) &&
// //                 Tools::timesMatch(iv, ivs) &&
// //                 tmatch.easyCondsMatch(imiPtr->next, trans.first);
//         if (match) {
//           transition = true;
//           IndexMatchInfo2 newIMI(imiPtr);
//           totalMatch = p->isFinalState(trans.second);
//           if (p->hasConds()) {
//             extendBinding2(per, p->getElemFromAtom(trans.first), totalMatch, 
//                            newIMI);
//             if (totalMatch && p->hasConds()) {
//               totalMatch = condsMatch(t, newIMI);
//             }
//           }
//           if (totalMatch) {
//             matches.push_back(id); // complete match
//             toRemove.push_back(id);
//   //         cout << id << " removed (wild match) " << activeTuples 
//   //              << " active tuples" << endl;
//             i = numOfIMIs; // stop processing this tuple id
//             active[id] = false;
//           }
//           else if (!newIMI.exhausted((*firstEnd)[id])) { // continue
//             newMatchInfo2[trans.second][id]->imis.push_back(newIMI);
//           }
//         }
//         else {
//           if (canBeDeactivated2(id, state, trans.first)) {
// //                   cout << "*Deactivate id " << id << endl;
//             toRemove.push_back(id);
//           }
//         }
//         t->DeleteIfAllowed();
//         i++;
//       }
//       if (active[id]) {
//         if (!hasIdIMIs(id, false, state)) { // no IMIs for id and state
//           if (!hasIdIMIs(id, false)) { // no IMIs at all for id
//             toRemove.push_back(id);
//     //         cout << id << " removed (wild mismatch) " << activeTuples 
//     //              << " active tuples" << endl;
//           }
//         }
//       }
//       id = matchInfo2[state][id]->succ;
//       remove(toRemove, false);
//     }
//   }
//   cout << "........ return " << (transition ? "TRUE" : "FALSE") << endl;
  return transition;
}

/*
\subsection{Function ~applyNFA~}

*/
void TMatchIndexLI::applyNFA(const bool mainAttr, 
                             const bool rewrite /* = false */) {
  set<int> states, newStates;
  states.insert(0);
  set<int>::reverse_iterator is;
  map<int, int>::reverse_iterator im;
  if (mainAttr) {
    while ((activeTuples > 0) && !states.empty()) {
//       cout << "WHILE loop: activeTuples=" << activeTuples << "; " 
//           << matches.size() - 1 << " matches" << endl;
      for (is = states.rbegin(); is != states.rend(); is++) {
        map<int, int> trans = p->getTransitions(*is);
        for (im = trans.rbegin(); im != trans.rend() && activeTuples > 0; im++){
          if (atomMatch(*is, *im, rewrite)) {
            newStates.insert(im->second);
          }
        }
      }
      states.clear();
      states.swap(newStates);
  //     cout << "Current states: ";
      clearMatchInfo();
      unitCtr++;
  //     for (set<int>::iterator it = states.begin(); it != states.end(); it++){
  //       cout << *it << ",";
  //     }
  //     cout << "unitCtr set to " << unitCtr << ";   activeTuples: " 
  //          << activeTuples << endl;
    }
  }
  else {
    while ((activeTuples > 0) && !states.empty()) {
//       cout << "WHILE loop: activeTuples=" << activeTuples << "; " 
//            << matches.size() - 1 << " matches" << endl;
      for (is = states.rbegin(); is != states.rend(); is++) {
        map<int, int> trans = p->getTransitions(*is);
        for (im = trans.rbegin(); im != trans.rend() && activeTuples > 0; im++){
          if (atomMatch2(*is, *im)) {
            newStates.insert(im->second);
          }
        }
      }
      states.clear();
      states.swap(newStates);
  //     cout << "Current states: ";
      clearMatchInfo2();
      unitCtr++;
  //     for (set<int>::iterator it = states.begin(); it != states.end(); it++){
  //       cout << *it << ",";
  //     }
  //     cout << "unitCtr set to " << unitCtr << ";   activeTuples: " 
  //          << activeTuples << endl;
    }
  }
}

/*
\subsection{Function ~initialize~}

In the version with main attribute, index results are stored as sets of unit
positions. Otherwise, we apply periods.

*/
bool TMatchIndexLI::initialize(const bool mainAttr, 
                               const bool rewrite /* = false */) {
  if (!tiCompatibleToRel()) {
    cout << "Error: tuple index is not compatible with relation" << endl;
    return false;
  }
  vector<map<int, int> > simpleNFA;
  p->simplifyNFA(simpleNFA);
  set<pair<set<int>, int> > paths;
  p->findNFApaths(simpleNFA, paths);
  set<int> cruElems;
  p->getCrucialElems(paths, crucialAtoms);
  for (int state = 0; state < p->getNFAsize(); state++) {
    if (p->nfaHasLoop(state)) {
      loopStates.insert(state);
    }
  }
  if (mainAttr) {
    indexResult = new IndexRetrieval**[p->getSize()];
    memset(indexResult, 0, p->getSize() * sizeof(void*));
    trajSize = new int[rel->GetNoTuples() + 1];
    memset(trajSize, 0, (rel->GetNoTuples() + 1) * sizeof(int));
    matchRecord = new DoubleUnitSet**[p->getSize()];
    memset(matchRecord, 0, p->getSize() * sizeof(void*));
    for (int i = 0; i < p->getSize(); i++) {
      matchRecord[i] = new DoubleUnitSet*[rel->GetNoTuples() + 1];
      memset(matchRecord[i], 0, (rel->GetNoTuples() + 1) * sizeof(void*));
    }
    if (p->hasConds()) {
      condRecord = new DoubleBindingSet*[rel->GetNoTuples() + 1];
      memset(condRecord, 0, (rel->GetNoTuples() + 1) * sizeof(void*));
    }
  }
  else {
    indexResult2 = new IndexRetrieval2**[p->getSize()];
    memset(indexResult2, 0, p->getSize() * sizeof(void*));
//     firstEnd = new Instant*[rel->GetNoTuples() + 1];
//     memset(firstEnd, 0, (rel->GetNoTuples() + 1) * sizeof(void*));
//     for (int id = 1; id <= rel->GetNoTuples(); id++) {
//       firstEnd[id] = new Instant(getFirstEnd(id));
//     }
    firstEnd = &(ti2->firstEnd);
    (*firstEnd)[0] = 0;
  }
  active = new bool[rel->GetNoTuples() + 1];
  memset(active, 1, (rel->GetNoTuples() + 1) * sizeof(bool));
  if (rewrite) {
    NewPair<TupleId, IndexMatchInfo> firstEntry(1, IndexMatchInfo());
    matchesR.push_back(firstEntry);
  }
  else {
    matches.push_back(1);
  }
  int minResultPos = -1;
  int minResults(INT_MAX), curResults(0), lastCrucialRelevant(-1);
  PatElem atom;
  set<int> relevantForTupleIndex;
  for (int i = 0; i < p->getSize(); i++) {
    p->getElem(i, atom);
    if (atom.isRelevantForTupleIndex()) {
      relevantForTupleIndex.insert(i);
    }
  }
  for (set<int>::iterator it = crucialAtoms.begin(); it != crucialAtoms.end(); 
       it++) {
    storeIndexResult(*it, lastCrucialRelevant, mainAttr, curResults);
    if (curResults < minResults) {
      minResults = curResults;
      minResultPos = *it;
    }
    if (relevantForTupleIndex.find(*it) != relevantForTupleIndex.end()) {//found
      lastCrucialRelevant = *it;
    }
    else {
      lastCrucialRelevant = -1;
    }
//     int successor = indexResult[*it][0]->succ;
//     int noResults = 0;
//     while (successor != 0) {
//       noResults++;
//       cout << "[" << successor << ": (";
//       for (set<int>::iterator t = indexResult[*it][successor]->units.begin();
//            t != indexResult[*it][successor]->units.end(); t++) {
//         cout << *t << " ";
//       }
//       cout << ")] ";
//       successor = indexResult[*it][successor]->succ;
//     }
//     cout << endl << endl;
  }
  if (minResultPos > -1) { // atom with smallest number of index result tuples
    int removed = 0;
    for (set<int>::iterator it = crucialAtoms.begin(); it != crucialAtoms.end();
         it++) {
      PatElem atom;
      p->getElem(*it, atom);
      if (*it != minResultPos && atom.isRelevantForTupleIndex()) {
        TupleId oldId = 0;
        if (mainAttr) {
          TupleId id = indexResult[*it][0]->succ;
          while (id != 0) {
            if (indexResult[minResultPos][id] == 0) {
              oldId = id;
              id = indexResult[*it][id]->succ;
              removeIdFromIndexResult(oldId, mainAttr);
              active[oldId] = false;
              removed++;
            }
            else {
              id = indexResult[*it][id]->succ;
            }
          }
        }
        else {
          TupleId id = indexResult2[*it][0]->succ;
          while (id != 0) {
            if (indexResult2[minResultPos][id] == 0) {
              oldId = id;
              id = indexResult2[*it][id]->succ;
              removeIdFromIndexResult(oldId, mainAttr);
              active[oldId] = false;
              removed++;
            }
            else {
              id = indexResult2[*it][id]->succ;
            }
          }
        }
      }
    }
//     cout << removed << " tuples removed due to crucial atom " 
//          << minResultPos << endl;

  }
  Tuple *firstTuple = rel->GetTuple(1, false);
  if (!p->initEasyCondOpTrees(mainAttr, firstTuple, ttList)) {
    return false;
  }
  if (!p->initCondOpTrees(firstTuple, ttList, mainAttr)) {
    return false;
  }
  firstTuple->DeleteIfAllowed();
  initMatchInfo(mainAttr);
  applyNFA(mainAttr, rewrite);
  return true;
}

/*
\subsection{Function ~nextTuple~}

*/
Tuple* TMatchIndexLI::nextTuple() {
  if (matches[0] == 0 || matches[0] >= matches.size() || matches.size() <= 1) {
    return 0;
  }
//   cout << "size: " << matches.size() << "; elem 0: " << matches[0] << " : " 
//        << matches[matches[0]] << endl;
  Tuple *result = rel->GetTuple(matches[matches[0]], false);
  matches[0]++;
  return result;
}

/*
\section{Functions for class ~ClassifyLI~} 

\subsection{Destructor}

*/
ClassifyLI::~ClassifyLI() {
  if (classifyTT) {
    delete classifyTT;
    classifyTT = 0;
  }
  vector<Pattern*>::iterator it;
  for (it = pats.begin(); it != pats.end(); it++) {
    delete (*it);
  }
  pats.clear();
}

/*
\subsection{Function ~getTupleType~}

*/
TupleType* ClassifyLI::getTupleType() {
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  ListExpr resultTupleType = nl->TwoElemList(
    nl->SymbolAtom(Tuple::BasicType()),
    nl->TwoElemList(nl->TwoElemList(nl->SymbolAtom("Description"),
                                    nl->SymbolAtom(FText::BasicType())),
                    nl->TwoElemList(nl->SymbolAtom("Trajectory"),
                                    nl->SymbolAtom(MLabel::BasicType()))));
  ListExpr numResultTupleType = sc->NumericType(resultTupleType);
  return new TupleType(numResultTupleType);
}

/*
\subsection{Function ~nextResultText~}

This function is used for the operator ~classify~.

*/
FText* ClassifyLI::nextResultText() {
  if (!pats.size()) {
    return 0;
  }  
  if (!matchingPats.empty()) {
    set<int>::iterator it = matchingPats.begin();
    FText* result = new FText(true, pats[*it]->getDescr());
    matchingPats.erase(it);
    return result;
  }
  return 0;
}

/*
\section{Functions for class ~Classifier~}

\subsection{List Representation}

The list representation of a classifier is

----    ((desc_1 pat_1) (desc_2 pat_2) ...)
----

\subsection{Constructors}

*/
Classifier::Classifier(const Classifier& src) {
  charpos = src.charpos;
  chars = src.chars;
  delta = src.delta;
  s2p = src.s2p;
  defined = src.defined;
}

string Classifier::getDesc(int pos) {
  int chpos = -1;
  int chposnext = -1;
  charpos.Get(pos * 2, chpos);
  charpos.Get(pos * 2 + 1, chposnext);
  string result = "";
  char ch;
  for (int i = chpos; i < chposnext; i++) {
    chars.Get(i, ch);
    result += ch;
  }
  return result;
}

string Classifier::getPatText(int pos) {
  int chpos = -1;
  int chposnext = -1;
  charpos.Get(pos * 2 + 1, chpos);
  charpos.Get(pos * 2 + 2, chposnext);
  string result = "";
  char ch;
  for (int i = chpos; i < chposnext; i++) {
    chars.Get(i, ch);
    result += ch;
  }
  return result;
}

void Classifier::getStartStates(set<int> &startStates) {
  startStates.clear();
  int pat = 0;
  startStates.insert(0);
  for (int i = 1; i < s2p.Size(); i++) {
    s2p.Get(i, pat);
    if (pat < 0) {
      startStates.insert(startStates.end(), - pat);
    }
  }
}

/*
\subsection{Function Describing the Signature of the Type Constructor}

*/
ListExpr Classifier::Property() {
  return (nl->TwoElemList(
    nl->FiveElemList(nl->StringAtom("Signature"),
             nl->StringAtom("Example Type List"),
             nl->StringAtom("List Rep"),
             nl->StringAtom("Example List"),
             nl->StringAtom("Remarks")),
    nl->FiveElemList(nl->StringAtom("->" + Kind::DATA() ),
             nl->StringAtom(Classifier::BasicType()),
             nl->StringAtom("((d1:text, p1:text) ((d2:text) (p2:text)) ...)"),
             nl->StringAtom("((home (_ at_home) *))"),
             nl->StringAtom("a collection of pairs (description, pattern)"))));
}

/*
\subsection{~In~ Function}

*/
Word Classifier::In(const ListExpr typeInfo, const ListExpr instance,
                    const int errorPos, ListExpr& errorInfo, bool& correct) {
  Word result = SetWord(Address(0));
  if (nl->IsEmpty(instance)) {
    cmsg.inFunError("Empty list");
    return SetWord(Address(0));
  }
  NList list(instance);
  Classifier* c = new Classifier(0);
  Pattern* p = 0;
  map<int, int> state2Pat; // maps start and final states to their pattern id
  c->SetDefined(true);
  c->appendCharPos(0);
  vector<Pattern*> patterns;
  while (!list.isEmpty()) {
    if ((list.length() % 2 == 0) && !list.isAtom() && list.first().isAtom() &&
     list.first().isText() && list.second().isAtom() && list.second().isText()){
      for (unsigned int i = 0; i < list.first().str().length(); i++) { //descr
        c->appendChar(list.first().str()[i]);
      }
      c->appendCharPos(c->getCharSize());
      for (unsigned int i = 0; i < list.second().str().length(); i++) {//pattern
        c->appendChar(list.second().str()[i]);
      }
      c->appendCharPos(c->getCharSize());
      p = Pattern::getPattern(list.second().str(), true);
      patterns.push_back(p);
    }
    else {
      cmsg.inFunError("Expecting a list of an even number of text atoms!");
      delete c;
      return SetWord(Address(0));
    }
    list.rest();
    list.rest();
  }
  vector<map<int, int> > nfa;
  set<int> finalStates;
  c->buildMultiNFA(patterns, nfa, finalStates, state2Pat);
  for (unsigned int i = 0; i < patterns.size(); i++) {
    delete patterns[i];
  }
  c->setPersistentNFA(nfa, finalStates, state2Pat);
  result.addr = c;
  return result;
}

/*
\subsection{~Out~ Function}

*/
ListExpr Classifier::Out(ListExpr typeInfo, Word value) {
  Classifier* c = static_cast<Classifier*>(value.addr);
  if (!c->IsDefined()) {
    return (NList(Symbol::UNDEFINED())).listExpr();
  }
  if (c->IsEmpty()) {
    return (NList()).listExpr();
  }
  else {
    NList list(c->getDesc(0), true, true);
    list.append(NList(c->getPatText(0), true, true));
    for (int i = 1; i < (c->getCharPosSize() / 2); i++) {
      list.append(NList(c->getDesc(i), true, true));
      list.append(NList(c->getPatText(i), true, true));
    }
    return list.listExpr();
  }
}

/*
\subsection{Kind Checking Function}

This function checks whether the type constructor is applied correctly. Since
type constructor ~classifier~ does not have arguments, this is trivial.

*/
bool Classifier::KindCheck(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(type, Classifier::BasicType()));
}

/*

\subsection{~Create~-function}

*/
Word Classifier::Create(const ListExpr typeInfo) {
  Classifier* c = new Classifier(0);
  return (SetWord(c));
}

/*
\subsection{~Delete~-function}

*/
void Classifier::Delete(const ListExpr typeInfo, Word& w) {
  Classifier* c = (Classifier*)w.addr;
  delete c;
}

/*
\subsection{~Open~-function}

*/
bool Classifier::Open(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value) {
  Classifier *c = (Classifier*)Attribute::Open(valueRecord, offset, typeInfo);
  value.setAddr(c);
  return true;
}

/*
\subsection{~Save~-function}

*/
bool Classifier::Save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value) {
  Classifier *c = (Classifier*)value.addr;
  Attribute::Save(valueRecord, offset, typeInfo, c);
  return true;
}

/*
\subsection{~Close~-function}

*/
void Classifier::Close(const ListExpr typeInfo, Word& w) {
  Classifier* c = (Classifier*)w.addr;
  delete c;
}

/*
\subsection{~Clone~-function}

*/
Word Classifier::Clone(const ListExpr typeInfo, const Word& w) {
  return SetWord(((Classifier*)w.addr)->Clone());
}

/*
\subsection{~SizeOf~-function}

*/
int Classifier::SizeOfObj() {
  return sizeof(Classifier);
}

/*
\subsection{~Cast~-function}

*/
void* Classifier::Cast(void* addr) {
  return (new (addr)Classifier);
}

/*
\subsection{Compare}

*/
int Classifier::Compare(const Attribute* arg) const {
  if (getCharPosSize() > ((Classifier*)arg)->getCharPosSize()) {
    return 1;
  }
  else if (getCharPosSize() < ((Classifier*)arg)->getCharPosSize()) {
    return -1;
  }
  else {
    if (getCharSize() > ((Classifier*)arg)->getCharPosSize()) {
      return 1;
    }
    else if (getCharSize() < ((Classifier*)arg)->getCharPosSize()) {
      return -1;
    }
    else {
      return 0;
    }
  }
}

/*
\subsection{HashValue}

*/
size_t Classifier::HashValue() const {
  return getCharPosSize() * getCharSize();
}

/*
\subsection{Adjacent}

Not implemented.

*/
bool Classifier::Adjacent(const Attribute* arg) const {
  return 0;
}

/*
\subsection{Clone}

Returns a new created element labels (clone) which is a copy of ~this~.

*/
Classifier* Classifier::Clone() const {
  assert(defined);
  Classifier *c = new Classifier(*this);
  return c;
}

/*
\subsection{CopyFrom}

*/
void Classifier::CopyFrom(const Attribute* right) {
  *this = *((Classifier*)right);
}

/*
\subsection{Sizeof}

*/
size_t Classifier::Sizeof() const {
  return sizeof(*this);
}

/*
\subsection{Creation of the Type Constructor Instance}

*/
TypeConstructor classifierTC(
  Classifier::BasicType(),             // name of the type in SECONDO
  Classifier::Property,                // property function describing signature
  Classifier::Out, Classifier::In,     // Out and In functions
  0, 0,                                // SaveToList, RestoreFromList functions
  Classifier::Create, Classifier::Delete, // object creation and deletion
  0, 0,                                // object open, save
  Classifier::Close, Classifier::Clone,// close, and clone
  0,                                   // cast function
  Classifier::SizeOfObj,               // sizeof function
  Classifier::KindCheck );             // kind checking function

/*
\subsection{Function ~buildMultiNFA~}

*/
void Classifier::buildMultiNFA(vector<Pattern*> patterns,
 vector<map<int, int> > &nfa, set<int> &finalStates, map<int, int> &state2Pat) {
  map<int, set<int> >::iterator it;
  unsigned int elemShift = 0;
  for (unsigned int i = 0; i < patterns.size(); i++) {
    unsigned int stateShift = nfa.size();
    IntNfa* intNfa = 0;
    state2Pat[stateShift] = -i;
    if (parsePatternRegEx(patterns[i]->getRegEx().c_str(), &intNfa) != 0) {
      cout << "error while parsing " << patterns[i]->getRegEx() << endl;
      return;
    }
    intNfa->nfa.makeDeterministic();
    intNfa->nfa.minimize();
    intNfa->nfa.bringStartStateToTop();
    map<int, set<int> >::iterator it;
    for (unsigned int j = 0; j < intNfa->nfa.numOfStates(); j++) {
      map<int, set<int> > trans = intNfa->nfa.getState(j).getTransitions();
      map<int, int> newTrans;
      for (it = trans.begin(); it != trans.end(); it++) {
        newTrans[it->first + elemShift] = *(it->second.begin()) + stateShift;
      }
      nfa.push_back(newTrans);
      if (intNfa->nfa.isFinalState(j)) {
        finalStates.insert(j + stateShift);
        state2Pat[j + stateShift] = i;
      }
      else if (j > 0) {
        state2Pat[j + stateShift] = INT_MAX;
      }
    }
    elemShift += patterns[i]->getSize();
    delete intNfa;
  }
}

/*
\subsection{Constructor for class ~IndexMatchesLI~}

*/
IndexMatchesLI::IndexMatchesLI(Relation *_rel, InvertedFile *inv,
            R_Tree<1, NewPair<TupleId, UnitPos> > *rt, int _attrNr, Pattern *_p,
            DataType _t)
    : IndexMatchSuper(_rel, _p, _attrNr, _t), invFile(inv), rtree(rt) {
  unitCtr = 0;
  if (p) {
    if (rel->GetNoTuples() > 0) {
      initialize();
      applyNFA();
      p->deleteEasyCondOpTrees();
      p->deleteCondOpTrees();
    }
  }
}

/*
\subsection{Destructor}

*/
IndexMatchesLI::~IndexMatchesLI() {
  rel = 0;
  if (p) {
    delete p;
  }
}

/*
\subsection{Function ~nextResultTuple~}

*/
Tuple* IndexMatchesLI::nextTuple() {
  if (!matches.empty()) {
    TupleId result = matches.back();
    matches.pop_back();
    return rel->GetTuple(result, false);
  }
  return 0;
}

/*
\subsection{Function ~applyNFA~}

*/
void IndexMatchesLI::applyNFA() {
  set<int> states, newStates;
  PatElem elem;
  states.insert(0);
  set<int>::reverse_iterator is;
  map<int, int>::reverse_iterator im;
  while ((activeTuples > 0) && !states.empty()) {
    unitCtr++;
//     cout << "WHILE loop: activeTuples=" << activeTuples << "; " 
//          << matches.size() << " matches; states:";
    for (is = states.rbegin(); is != states.rend(); is++) {
      map<int, int> trans = p->getTransitions(*is);
      for (im = trans.rbegin(); im != trans.rend() && activeTuples > 0; im++) {
        p->getElem(im->first, elem);
        if (elem.getW() == NO) { // no wildcard
//            cout << "call simpleMatch for transition " << *is << " --" 
//                 << im->first << "--> " << im->second << endl;
          if (simpleMatch(im->first, *is, im->second)) {
            newStates.insert(im->second);
          }
        }
        else { // + or *
//            cout << "call wildcardMatch for transition " << *is << " --" 
//                 << im->first << "--> " << im->second << endl;
          if (wildcardMatch(*is, *im)) {
            newStates.insert(im->second);
          }
        }
      }
    }
    states.clear();
    states.swap(newStates);
    clearMatchInfo();
  }
}


/*
\subsection{Function ~initOpTrees~}

Necessary for the operator ~rewrite~

*/
bool Assign::initOpTrees() {
  if (treesOk && opTree[0].second) {
    return true;
  }
  for (int i = 0; i <= 9; i++) {
    if (pointers[i].size() > 0) { // pointers already initialized
      return true;
    }
  }
  if (!occurs() && (!hasValue() || !hasTime())) {
    cout << "not enough data for variable " << var << endl;
    return false;
  }
  string q(""), part, toReplace("");
  pair<string, Attribute*> strAttr;
  for (int i = 0; i <= 9; i++) {
    if (!text[i].empty()) {
      q = "query " + text[i];
      for (unsigned int j = 0; j < right[i].size(); j++) { // loop through keys
        if (right[i][j].second == 2) { // interval instead of periods
          deleteIfAllowed(strAttr.second);
          strAttr.second = new SecInterval(true);
          strAttr.first = "[const interval pointer "
                   + nl->ToString(listutils::getPtrList(strAttr.second)) + "]";
        }
        else {
          strAttr = Pattern::getPointer(right[i][j].second, true);
        }
        pointers[i].push_back(strAttr.second);
        toReplace = right[i][j].first 
                  + Condition::getType(right[i][j].second, right[i][j].first);
        q.replace(q.find(toReplace), toReplace.length(), strAttr.first);
      }
      opTree[i] = Tools::processQueryStr(q, i);
      if (!opTree[i].first) {
        cout << "pointer not initialized" << endl;
        return false;
      }
      if (!opTree[i].second) {
        cout << "opTree not initialized" << endl;
        return false;
      }
    }
  }
  treesOk = true;
  return true;
}

void Assign::clear() {
  resultPos = -1;
  for (int i = 0; i < 6; i++) {
    text[i].clear();
    right[i].clear();
  }
  right[6].clear();
  deleteOpTrees();
}

void Assign::deleteOpTrees() {
  if (treesOk) {
    for (int i = 0; i < 6; i++) {
      if (opTree[i].first) {
        opTree[i].first->Destroy(opTree[i].second, true);
        delete opTree[i].first;
      }
      for (unsigned int j = 0; j < pointers[i].size(); j++) {
        if (pointers[i][j]) {
          deleteIfAllowed(pointers[i][j]);
          pointers[i][j] = 0;
        }
      }
      pointers[i].clear();
    }
  }
  treesOk = false;
}

void Condition::deleteOpTree() {
  if (treeOk) {
    if (opTree.first) {
//       cout << "destroy second" << endl;
      opTree.first->Destroy(opTree.second, true);
//       cout << "delete first" << endl;
      delete opTree.first;
      for (unsigned int i = 0; i < pointers.size(); i++) {
//         cout << "delete ptr " << i;
        deleteIfAllowed(pointers[i]);
//         cout << " ...........ok" << endl;
      }
    }
    treeOk = false;
  }
}

/*
\subsection{Function ~initAssignOpTrees~}

Invoked by the value mapping of the operator ~rewrite~.

*/
bool Pattern::initAssignOpTrees() {
  for (unsigned int i = 0; i < assigns.size(); i++) {
    if (!assigns[i].initOpTrees()) {
      cout << "Error at assignment #" << i << endl;
      return false;
    }
  }
  return true;
}

void Pattern::deleteAssignOpTrees(bool deleteConds) {
  for (unsigned int i = 0; i < assigns.size(); i++) {
    assigns[i].deleteOpTrees();
  }
  if (deleteConds) {
    for (unsigned int i = 0; i < conds.size(); i++) {
      conds[i].deleteOpTree();
    }
  }
}

/*
\subsection{Function ~deleteAtomValues~}

*/
void Pattern::deleteAtomValues(vector<pair<int, string> > &relevantAttrs) {
  for (int i = 0; i < getSize(); i++) {
    elems[i].deleteValues(relevantAttrs);
  }
}

bool Pattern::startsWithAsterisk() const {
  if (getSize() == 0) {
    return false;
  }
  PatElem atom;
  getElem(0, atom);
  if (atom.getW() == STAR) {
    if (!regEx.empty()) {
      return (regEx[0] != '(');
    }
  }
  return false;
}

bool Pattern::startsWithPlus() const {
  if (getSize() == 0) {
    return false;
  }
  PatElem atom;
  getElem(0, atom);
  if (atom.getW() == PLUS) {
    if (!regEx.empty()) {
      return (regEx[0] != '(');
    }
  }
  return false;
}

bool Pattern::nfaHasLoop(const int state) const {
  if ((int)(nfa.size()) <= state) {
    return false;
  }
  for (map<int, int>::const_iterator it = nfa[state].begin(); 
       it != nfa[state].end(); it++) {
    if (it->second == state) {
      return true;
    }
  }
  return false;
}

bool PatElem::hasValuesWithContent() const {
  for (unsigned int i = 0; i < values.size(); i++) {
    if (values[i].first.addr != 0) {
      return true;
    }
  }
  return false;
}

bool PatElem::isRelevantForTupleIndex() const {
  if (hasRealInterval()) {
    return true;
  }
  if (!hasValuesWithContent()) {
    return false;
  }
  for (int i = 0; i < getNoValues(); i++) {
    if (values[i].second != DISJOINT) {
      return true;
    }
  }
  return false;
}

void PatElem::getInterval(SecInterval& result) const {
  if (!hasInterval()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  std::set<std::string> ivsAtom;
  getI(ivsAtom);
  SecInterval ivtmp1(true), ivtmp2(true);
  std::set<std::string>::iterator it = ivsAtom.begin();
  bool first = true;
  while (it != ivsAtom.end()) {
    Tools::stringToInterval(*it, ivtmp1);
    if (ivtmp1.IsDefined()) {
      if (first) {
        ivtmp2 = ivtmp1;
        first = false;
      }
      else {
        ivtmp1.Intersection(ivtmp2, result);
        ivtmp2 = result;
      }
    }
    it++;
  }
}

void PatElem::deleteValues(vector<pair<int, string> > &relevantAttrs) {
  if (wc != NO) {
    return;
  }
  for (unsigned int i = 0; i < values.size(); i++) {
    Tools::deleteValue(values[i].first, relevantAttrs[i].second);
  }
}

/*
\subsection{Constructor for class ~IndexClassifyLI~}

This constructor is used for the operator ~indexclassify~.

*/
IndexClassifyLI::IndexClassifyLI(Relation *rel, InvertedFile *inv,
  R_Tree<1, NewPair<TupleId, UnitPos> > *rt, Word _classifier, int _attrNr, 
  DataType type) : 
         IndexMatchesLI::IndexMatchesLI(rel, inv, rt, _attrNr, 0, type),
         classifyTT(0), c(0) {
  if (rel->GetNoTuples() > 0) {
    c = (Classifier*)_classifier.addr;
    for (int i = 0; i < c->getNumOfP(); i++) { // check patterns
      Pattern *p = Pattern::getPattern(c->getPatText(i), false);
      bool ok = false;
      if (p) {
        switch (type) {
          case MLABEL: {
            ok = p->isValid("label");
            break;
          }
          case MLABELS: {
            ok = p->isValid("labels");
            break;
          }
          case MPLACE: {
            ok = p->isValid("place");
            break;
          }
          case MPLACES: {
            ok = p->isValid("places");
            break;
          }
        }
        delete p;
      }
      if (!ok) {
        c = 0;
        return;
      }
    }
  }
  classifyTT = ClassifyLI::getTupleType();
  currentPat = 0;
}

/*
\subsection{Destructor for class ~IndexClassifyLI~}

*/
IndexClassifyLI::~IndexClassifyLI() {
  if (classifyTT) {
    classifyTT->DeleteIfAllowed();
    classifyTT = 0;
  }
}

/*
\subsection{Function ~retrieveValue~}

*/
void IndexMatchesLI::retrieveValue(vector<set<int> >& oldPart, 
     vector<set<int> >& newPart, SetRel rel, bool first, const string& label, 
     unsigned int ref /* = UINT_MAX */) {
  InvertedFile::exactIterator* eit = 0;
  TupleId id;
  uint32_t wc, cc;
  eit = invFile->getExactIterator(label, 16777216);
  if ((mtype == MLABEL) || (mtype == MLABELS)) {
    while (eit->next(id, wc, cc)) {
      if (!deactivated[id]) {
        if (first) { // ignore oldPart
          newPart[id].insert(wc);
        }
        else {
          if ((rel == STANDARD) || (rel == EQUAL)) { // intersect sets
            set<int>::iterator it = oldPart[id].find(wc);
            if (it != oldPart[id].end()) { // found in oldPart
              newPart[id].insert(it, wc);
            }
          }
          else { // SUPERSET or INTERSECT => unite sets
            oldPart[id].insert(wc);
          }
        }
      }
    }
  }
  else { // ref is used
    while (eit->next(id, wc, cc)) {
      if (!deactivated[id]) {
        if (ref == cc) { // cc represents the reference of the place
          if (first) { // ignore oldPart
            newPart[id].insert(wc);
          }
          else {
            if ((rel == STANDARD) || (rel == EQUAL)) { // intersect sets
              set<int>::iterator it = oldPart[id].find(wc);
              if (it != oldPart[id].end()) {
                newPart[id].insert(it, wc);
              }
            }
            else { // SUPERSET or INTERSECT => unite sets
              oldPart[id].insert(wc);
            }
          }
        }
      }
    }
  }
  if (first || (rel == STANDARD) || (rel == EQUAL)) {
    oldPart.swap(newPart);
  }
//   for (int i = 0; i < oldPart.size(); i++) {
//     cout << oldPart[i].size() << " ";
//   }
//   cout << endl;
//   for (int i = 0; i < newPart.size(); i++) {
//     cout << newPart[i].size() << " ";
//   }
//   cout << endl;
  delete eit;
}

/*
\subsection{Function ~retrieveTime~}

*/
void IndexMatchesLI::retrieveTime(vector<set<int> >& oldPart,
                              vector<set<int> >& newPart, const string& ivstr) {
  bool init = false;
  if (oldPart.empty()) {
    oldPart.resize(rel->GetNoTuples() + 1);
    newPart.resize(rel->GetNoTuples() + 1);
    init = true;
  }
  R_TreeLeafEntry<1, NewPair<TupleId, UnitPos> > leaf;
  double min[1], max[1];
  SecInterval iv(true);
  Tools::stringToInterval(ivstr, iv);
  min[0] = iv.start.ToDouble();
  max[0] = iv.end.ToDouble();
  Rect1 rect1(true, min, max);
  if (rtree->First(rect1, leaf)) {
    if ((init || !oldPart[leaf.info.first].empty()) &&
        !deactivated[leaf.info.first]) {
      newPart[leaf.info.first].insert(newPart[leaf.info.first].end(), 
                                      leaf.info.second.pos);
    }
    while (rtree->Next(leaf)) {
      if ((init || !oldPart[leaf.info.first].empty()) &&
          !deactivated[leaf.info.first]) {
        newPart[leaf.info.first].insert(newPart[leaf.info.first].end(),
                                        leaf.info.second.pos);
      }
    }
  }
  newPart.swap(oldPart);
}

/*
\subsection{Function ~removeIdFromMatchInfo~}

*/
void IndexMatchesLI::removeIdFromMatchInfo(const TupleId id) {
  bool removed = false;
  for (int s = 0; s < p->getNFAsize(); s++) {
    if (newMatchInfo[s] != 0) {
//         cout << "Elem " << i << ", Tuple " << id << ":" << endl;
      newMatchInfo[s][newMatchInfo[s][id]->pred]->succ = 
                                                      newMatchInfo[s][id]->succ;
//         cout << "   succ of " << matchInfo[i][id].pred << " set to " 
//              << matchInfo[i][id].succ << endl;
      newMatchInfo[s][newMatchInfo[s][id]->succ]->pred = 
                                                      newMatchInfo[s][id]->pred;
//         cout << "   pred of " << matchInfo[i][id].succ << " set to " 
//              << matchInfo[i][id].pred << endl;
//       (*newMatchInfoPtr)[s][id].imis.clear();
    }
    if (matchInfo[s] != 0) {
      if (matchInfo[s][matchInfo[s][id]->pred]->succ == id) {
//         cout << "Elem " << i << ", Tuple " << id << ":" << endl;
        matchInfo[s][matchInfo[s][id]->pred]->succ = matchInfo[s][id]->succ;
//         cout << "   succ of " << matchInfo[i][id].pred << " set to " 
//              << matchInfo[i][id].succ << endl;
        matchInfo[s][matchInfo[s][id]->succ]->pred = matchInfo[s][id]->pred;
        removed = true;
//         cout << "   pred of " << matchInfo[i][id].succ << " set to " 
//              << matchInfo[i][id].pred << endl;
//       (*matchInfoPtr)[s][id].imis.clear();
      }
    }
  }
  deactivated[id] = true;
  activeTuples -= (removed ? 1 : 0);
}

/*
\subsection{Function ~storeIndexResult~}

*/
void IndexMatchesLI::storeIndexResult(const int e) {
  if (indexResult[e] != 0) {
    return;
  }
  PatElem elem;
  p->getElem(e, elem);
  if (!elem.hasIndexableContents()) {
    return;
  }
  set<string> ivs, lbs, pls;
  vector<set<int> > part, part2;
//   vector<bool> time, time2;
//   time.resize(mRel->GetNoTuples() + 1, true);
//   time2.resize(mRel->GetNoTuples() + 1, true);
  if (elem.getSetRel() != DISJOINT) {
    if ((mtype == MLABEL) || (mtype == MLABELS)) {
      elem.getL(lbs);
      set<string>::iterator is = lbs.begin();
      if (!lbs.empty()) {
        part.resize(rel->GetNoTuples() + 1);
        part2.resize(rel->GetNoTuples() + 1);
        retrieveValue(part, part2, elem.getSetRel(), true, *is);
        is++;
      }
      while (is != lbs.end()) {
        retrieveValue(part, part2, elem.getSetRel(), false, *is);
        is++;
      }
    }
    else { // PLACE or PLACES
      elem.getL(pls);
      set<string>::iterator ip = pls.begin();
      if (!pls.empty()) {
        part.resize(rel->GetNoTuples() + 1);
        part2.resize(rel->GetNoTuples() + 1);
        retrieveValue(part, part2, elem.getSetRel(), true, *ip);
        ip++;
      }
      while (ip != pls.end()) {
        retrieveValue(part, part2, elem.getSetRel(), false, *ip);
        ip++;
      }
    }  
  } // continue with time intervals
  if (!part.empty() || (elem.getSetRel() == DISJOINT) || !elem.hasLabel()) {
    elem.getI(ivs);
    set<string>::iterator is = ivs.begin();
    while (is != ivs.end()) {
      if (Tools::isInterval(*is)) {
        retrieveTime(part, part2, *is);
      }
      is++;
    }
  }
  indexResult[e] = new IndexRetrieval*[rel->GetNoTuples() + 1];
  memset(indexResult[e], 0, (rel->GetNoTuples() + 1)*sizeof(void*));
  unsigned int pred = 0;
  indexResult[e][0] = new IndexRetrieval(pred, 0);
  for (unsigned int i = 1; i < part.size(); i++) { // collect results
    if (!part[i].empty()) {
      indexResult[e][pred]->succ = i; // update successor of predecessor
      indexResult[e][i] = new IndexRetrieval(pred, 0, part[i]);
      pred = i;
    }
  }
//   if (part.empty() && ((elem.getSetRel() == DISJOINT) ||
//       (!elem.hasLabel() && !elem.hasPlace()))) {
//     for (unsigned int i = 1; i < time.size(); i++) {
//       if (time[i]) {
//         indexResult[e][pred].succ = i; // update successor of predecessor
//         IndexRetrieval ir(pred);
//         indexResult[e][i] = ir;
//         pred = i;
//       }
//     }
//   }
  if ((indexResult[e][0]->succ == 0) && elem.hasIndexableContents() &&
      (elem.getSetRel() != DISJOINT)) {
    indexMismatch.insert(e);
  }
}

/*
\subsection{Function ~initMatchInfo~}

*/
void IndexMatchesLI::initMatchInfo(const set<int>& cruElems) {
  matchInfo = new IndexMatchSlot**[p->getNFAsize()];
  memset(matchInfo, 0, p->getNFAsize() * sizeof(void*));
  newMatchInfo = new IndexMatchSlot**[p->getNFAsize()];
  memset(newMatchInfo, 0, p->getNFAsize() * sizeof(void*)); 
  trajSize = new int[rel->GetNoTuples() + 1];
  memset(trajSize, 0, (rel->GetNoTuples() + 1 * sizeof(int)));
  vector<bool> trajInfo, newTrajInfo;
  trajInfo.assign(rel->GetNoTuples() + 1, true);
  PatElem elem;
  for (set<int>::iterator it = cruElems.begin(); it != cruElems.end(); it++) {
    p->getElem(*it, elem);
    if (elem.hasIndexableContents()) {
      if (indexMismatch.find(*it) != indexMismatch.end()) {
        cout << "index mismatch at crucial element " << *it << endl;
        return;
      }
      newTrajInfo.assign(rel->GetNoTuples() + 1, false);
      TupleId id = indexResult[*it][0]->succ; // first active tuple id
      while (id > 0) {
        if (trajInfo[id]) {
          newTrajInfo[id] = true;
        }
        else {
          removeIdFromIndexResult(id, true);
        }
        id = indexResult[*it][id]->succ;
      }
      trajInfo.swap(newTrajInfo);
    }
  }
  for (int i = 0; i < p->getNFAsize(); i++) {
    matchInfo[i] = new IndexMatchSlot*[rel->GetNoTuples() + 1];
    memset(matchInfo[i], 0, (rel->GetNoTuples() + 1) * sizeof(void*));
    newMatchInfo[i] = new IndexMatchSlot*[rel->GetNoTuples() + 1];
    memset(newMatchInfo[i], 0, (rel->GetNoTuples() + 1) * sizeof(void*));
  }
  unsigned int pred = 0;
  for (unsigned int id = 1; id < trajInfo.size(); id++) {
    if (trajInfo[id]) {
      trajSize[id] = getTrajSize(id, mtype);
      IndexMatchInfo imi(false);
      deactivated[id] = false;
      for (int s = 0; s < p->getNFAsize(); s++) {
        matchInfo[s][id] = new IndexMatchSlot();
        matchInfo[s][pred]->succ = id;
        matchInfo[s][id]->pred = pred;
        newMatchInfo[s][id] = new IndexMatchSlot();
        newMatchInfo[s][pred]->succ = id;
        newMatchInfo[s][id]->pred = pred;
        if (indexResult[s] != 0) {
          for (unsigned int i = pred + 1; i < id; i++) { // deactivate indexRes.
            indexResult[s][i]->pred = 0;
            indexResult[s][i]->succ = 0;
          }
          indexResult[s][pred]->succ = id;
        }
      }
      matchInfo[0][id]->imis.push_back(imi);
      activeTuples++;
      pred = id;
    }
  }
  matchInfo[0][pred]->succ = 0;
  newMatchInfo[0][pred]->succ = 0;
  for (int s = 0; s < p->getSize(); s++) {
    if (indexResult[s] != 0) {
      indexResult[s][pred]->succ = 0;
    }
  }
}

/*
\subsection{Function ~initialize~}

Collects the tuple ids of the trajectories that could match the pattern
according to the index information.

*/
void IndexMatchesLI::initialize() {
  vector<map<int, int> > simpleNFA;
  p->simplifyNFA(simpleNFA);
  set<pair<set<int>, int> > paths;
  p->findNFApaths(simpleNFA, paths);
  set<int> cruElems;
  p->getCrucialElems(paths, cruElems);
  indexResult = new IndexRetrieval**[p->getSize()];
  memset(indexResult, 0, p->getSize() * sizeof(void*));
  deactivated.resize(rel->GetNoTuples() + 1, false);
  for (set<int>::iterator it = cruElems.begin(); it != cruElems.end(); it++) {
    storeIndexResult(*it);
  }
  deactivated.resize(rel->GetNoTuples() + 1, true);
  p->initEasyCondOpTrees(true);
  p->initCondOpTrees();
  activeTuples = 0;
  initMatchInfo(cruElems);
}

/*
\subsection{Destructor}

*/
IndexMatchSuper::~IndexMatchSuper() {
  if (p) {
    int pred(0), id(0);
    if (indexResult && matchInfo) { // for indextmatches
      for (int i = 0; i < p->getSize(); i++) {
        if (indexResult[i] != 0) {
          id = indexResult[i][0]->succ;
          while (id != 0 && indexResult[i][id] != 0) {
            pred = id;
            id = indexResult[i][id]->succ;
            delete indexResult[i][pred];
          }
          delete indexResult[i][0];
          delete[] indexResult[i];
        }
        if (matchRecord[i] != 0) {
          for (int j = 0; j <= rel->GetNoTuples(); j++) {
            if (matchRecord[i][j] != 0) {
              delete matchRecord[i][j];
            }
          }
          delete[] matchRecord[i];
        }
      }
      for (int i = 0; i < p->getNFAsize(); i++) {
        if (matchInfo != 0) {
          if (matchInfo[i] != 0) {
            id = matchInfo[i][0]->succ;
            while (id != 0 && matchInfo[i][id] != 0) {
              pred = id;
              id = matchInfo[i][id]->succ;
              delete matchInfo[i][pred];
            }
            delete matchInfo[i][0];
            delete[] matchInfo[i];
          }
        }
        if (newMatchInfo != 0) {
          if (newMatchInfo[i] != 0) {
            id = newMatchInfo[i][0]->succ;
            while (id != 0 && newMatchInfo[i][id] != 0) {
              pred = id;
              id = newMatchInfo[i][id]->succ;
              delete newMatchInfo[i][pred];
            }
            delete newMatchInfo[i][0];
            delete[] newMatchInfo[i];
          }
        }
      }
      delete[] indexResult;
      delete[] matchRecord;
      delete[] matchInfo;
      delete[] newMatchInfo;
      delete[] trajSize;
    }
    if (condRecord != 0) {
      for (int i = 0; i <= rel->GetNoTuples(); i++) {
        if (condRecord[i]) {
          delete condRecord[i];
        }
      }
      delete[] condRecord;
    }
    if (indexResult2 && matchInfo2) { // for indextmatches2
      for (int i = 0; i < p->getSize(); i++) {
        if (indexResult2[i] != 0) {
          id = indexResult2[i][0]->succ;
          while (id != 0 && indexResult2[i][id] != 0) {
            pred = id;
            id = indexResult2[i][id]->succ;
            delete indexResult2[i][pred];
            indexResult2[i][pred] = 0;
          }
          delete indexResult2[i][0];
          delete[] indexResult2[i];
        }
      }
      for (int i = 0; i < p->getNFAsize(); i++) {
        if (matchInfo2 != 0) {
          if (matchInfo2[i] != 0) {
            id = matchInfo2[i][0]->succ;
            while (id != 0 && matchInfo2[i][id] != 0) {
              pred = id;
              id = matchInfo2[i][id]->succ;
              delete matchInfo2[i][pred];
            }
            delete matchInfo2[i][0];
            delete[] matchInfo2[i];
          }
        }
        if (newMatchInfo2 != 0) {
          if (newMatchInfo2[i] != 0) {
            id = newMatchInfo2[i][0]->succ;
            while (id != 0 && newMatchInfo2[i][id] != 0) {
              pred = id;
              id = newMatchInfo2[i][id]->succ;
              delete newMatchInfo2[i][pred];
            }
            delete newMatchInfo2[i][0];
            delete[] newMatchInfo2[i];
          }
        }
      }
      delete[] indexResult2;
      delete[] matchInfo2;
      delete[] newMatchInfo2;
    }
    delete[] active;
    deletePattern();
  }
}

/*
\subsection{Function ~getMsize~}

*/
int IndexMatchSuper::getTrajSize(const TupleId tId, const DataType type) {
  Tuple* tuple = rel->GetTuple(tId, false);
  int result = -1;
  switch (type) {
    case MLABEL: {
      result = ((MLabel*)tuple->GetAttribute(attrNo))->GetNoComponents();
      break;
    }
    case MLABELS: {
      result = ((MLabels*)tuple->GetAttribute(attrNo))->GetNoComponents();
      break;
    }
    case MPLACE: {
      result = ((MPlace*)tuple->GetAttribute(attrNo))->GetNoComponents();
      break;
    }
    default: { // places
      result = ((MPlaces*)tuple->GetAttribute(attrNo))->GetNoComponents();
      break;
    }
  }
  deleteIfAllowed(tuple);
  return result;
}

/*
\subsection{Function ~getInterval~}

*/
void IndexMatchSuper::getInterval(const TupleId tId, const int pos, 
                                  SecInterval& iv) {
  Tuple *tuple = rel->GetTuple(tId, false);
  switch (mtype) {
    case MLABEL: {
      ((MLabel*)tuple->GetAttribute(attrNo))->GetInterval(pos, iv);
      break;
    }
    case MLABELS: {
      ((MLabels*)tuple->GetAttribute(attrNo))->GetInterval(pos, iv);
      break;
    }
    case MPLACE: {
      ((MPlace*)tuple->GetAttribute(attrNo))->GetInterval(pos, iv);
      break;
    }
    case MPLACES: {
      ((MPlaces*)tuple->GetAttribute(attrNo))->GetInterval(pos, iv);
      break;
    }
    default: {
      break;
    }
  }
  tuple->DeleteIfAllowed();
}

/*
\subsection{Function ~periodsToUnits~}

*/
void IndexMatchSuper::periodsToUnits(const Periods *per, const TupleId tId,
                                     set<int> &units) {
  if (!per) {
    return;
  }
  if (!per->IsDefined()) {
    cout << "undefined periods!" << endl;
    return;
  }
  switch (mtype) {
    case MLABEL: {
      periodsToUnits<MLabel>(per, tId, units);
      break;
    }
    case MLABELS: {
      periodsToUnits<MLabels>(per, tId, units);
      break;
    }
    case MPLACE: {
      periodsToUnits<MPlace>(per, tId, units);
      break;
    }
    case MPLACES: {
      periodsToUnits<MPlaces>(per, tId, units);
      break;
    }
  }
}

/*
\subsection{Function ~unitsToPeriods~}

*/
void TMatchIndexLI::unitsToPeriods(const set<int> &units, const TupleId tId, 
                                   const int attr, Periods *per) {
  per->SetDefined(true);
  Tuple *t = rel->GetTuple(tId, false);
  Attribute *traj = t->GetAttribute(attr);
  ListExpr attrList = nl->Second(nl->Nth(attr + 1, nl->Second(ttList)));
  if (MLabel::checkType(attrList)) {
    unitsToPeriods<MLabel, ULabel>(traj, units, per);
  }
  else if (MLabels::checkType(attrList)) {
    unitsToPeriods<MLabels, ULabels>(traj, units, per);
  }
  else if (MPlace::checkType(attrList)) {
    unitsToPeriods<MPlace, UPlace>(traj, units, per);
  }
  else if (MPlaces::checkType(attrList)) {
    unitsToPeriods<MPlaces, UPlaces>(traj, units, per);
  }
  else if (MPoint::checkType(attrList)) {
    unitsToPeriods<MPoint, UPoint>(traj, units, per);
  }
  else if (MRegion::checkType(attrList)) {
    unitsToPeriods<MRegion, URegionEmb>(traj, units, per);
  }
  else if (MBool::checkType(attrList)) {
    unitsToPeriods<MBool, UBool>(traj, units, per);
  }
  else if (MInt::checkType(attrList)) {
    unitsToPeriods<MInt, UInt>(traj, units, per);
  }
  else if (MReal::checkType(attrList)) {
    unitsToPeriods<MReal, UReal>(traj, units, per);
  }
  else if (MString::checkType(attrList)) {
    unitsToPeriods<MString, UString>(traj, units, per);
  }
  t->DeleteIfAllowed();
}

/*
\subsection{Function ~remove~}

*/
void IndexMatchSuper::remove(std::vector<TupleId> &toRemove, 
                             const bool mainAttr) {
  for (unsigned int i = 0; i < toRemove.size(); i++) {
    if (active[toRemove[i]]) {
      removeIdFromMatchInfo(toRemove[i], mainAttr);
      removeIdFromIndexResult(toRemove[i], mainAttr);
    }
  }
  toRemove.clear();
}

/*
\subsection{Function ~removeIdFromIndexResult~}

*/
void IndexMatchSuper::removeIdFromIndexResult(const TupleId id, 
                                              const bool mainAttr) {
  if (mainAttr) {
    for (int i = 0; i < p->getSize(); i++) {
      if (indexResult[i] != 0) {
        if (indexResult[i][id] != 0) {
          if (indexResult[i][indexResult[i][id]->pred]->succ == id) {
            indexResult[i][indexResult[i][id]->pred]->succ = 
                                                       indexResult[i][id]->succ;
            indexResult[i][indexResult[i][id]->succ]->pred = 
                                                       indexResult[i][id]->pred;
            delete indexResult[i][id];
            indexResult[i][id] = 0;
          }
        }
      }
    }
  }
  else {
    for (int i = 0; i < p->getSize(); i++) {
      if (indexResult2[i] != 0) {
        if (indexResult2[i][id] != 0) {
          if (indexResult2[i][indexResult2[i][id]->pred]->succ == id) {
            indexResult2[i][indexResult2[i][id]->pred]->succ = 
                                                      indexResult2[i][id]->succ;
            indexResult2[i][indexResult2[i][id]->succ]->pred = 
                                                      indexResult2[i][id]->pred;
            delete indexResult2[i][id];
            indexResult2[i][id] = 0;
          }
        }
      }
    }
  }
}

/*
\subsection{Function ~removeIdFromMatchInfo~}

*/
void IndexMatchSuper::removeIdFromMatchInfo(const TupleId id, 
                                            const bool mainAttr) {
  bool removed = false;
  if (mainAttr) {
    for (int s = 0; s < p->getNFAsize(); s++) {
      if (newMatchInfo[s] != 0) {
        if (newMatchInfo[s][id]) {
    //         cout << "Elem " << i << ", Tuple " << id << ":" << endl;
          newMatchInfo[s][newMatchInfo[s][id]->pred]->succ = 
                                                      newMatchInfo[s][id]->succ;
    //         cout << "   succ of " << matchInfo[i][id].pred << " set to " 
    //              << matchInfo[i][id].succ << endl;
          newMatchInfo[s][newMatchInfo[s][id]->succ]->pred = 
                                                      newMatchInfo[s][id]->pred;
    //         cout << "   pred of " << matchInfo[i][id].succ << " set to " 
    //              << matchInfo[i][id].pred << endl;
          delete newMatchInfo[s][id];
          newMatchInfo[s][id] = 0;
          removed = true;
        }
      }
    }
  }
//     if (matchInfo[s] != 0) {
//       if (matchInfo[s][id] != 0) {
//         if (matchInfo[s][matchInfo[s][id]->pred]->succ == id) {
//   //         cout << "Elem " << i << ", Tuple " << id << ":" << endl;
//          matchInfo[s][matchInfo[s][id]->pred]->succ = matchInfo[s][id]->succ;
//   //         cout << "   succ of " << matchInfo[i][id].pred << " set to " 
//   //              << matchInfo[i][id].succ << endl;
//          matchInfo[s][matchInfo[s][id]->succ]->pred = matchInfo[s][id]->pred;
//           removed = true;
//   //         cout << "   pred of " << matchInfo[i][id].succ << " set to " 
//   //              << matchInfo[i][id].pred << endl;
//           delete matchInfo[s][id];
//           matchInfo[s][id] = 0;
//         }
//       }
//     }
  else {
    for (int s = 0; s < p->getNFAsize(); s++) {
      if (newMatchInfo2[s] != 0) {
        if (newMatchInfo2[s][id]) {
    //         cout << "Elem " << i << ", Tuple " << id << ":" << endl;
          newMatchInfo2[s][newMatchInfo2[s][id]->pred]->succ = 
                                                     newMatchInfo2[s][id]->succ;
    //         cout << "   succ of " << matchInfo[i][id].pred << " set to " 
    //              << matchInfo[i][id].succ << endl;
          newMatchInfo2[s][newMatchInfo2[s][id]->succ]->pred = 
                                                     newMatchInfo2[s][id]->pred;
    //         cout << "   pred of " << matchInfo[i][id].succ << " set to " 
    //              << matchInfo[i][id].pred << endl;
          delete newMatchInfo2[s][id];
          newMatchInfo2[s][id] = 0;
          removed = true;
        }
      }
    }
  }
  active[id] = false;
  activeTuples -= (removed ? 1 : 0);
//   cout << "ID " << id << " removed; activeTuples=" << activeTuples << endl;
}

/*
\subsection{Function ~canBeDeactivated~}

*/
bool IndexMatchSuper::canBeDeactivated(const TupleId id, const int state,
                    const int atom, const bool checkRange /* = false */) const {
  if (crucialAtoms.find(atom) != crucialAtoms.end()) {
    for (int i = state + 1; i < p->getNFAsize(); i++) {
      if (matchInfo[i] != 0) {
        if (matchInfo[i][id] != 0) {
          if (!matchInfo[i][id]->imis.empty()) {
//             cout << "------------DO NOT deactivate id " << id 
//                  << ", has mI[" << i << "]" << endl;
            return false;
          }
        }
      }
      if (newMatchInfo[i] != 0) {
        if (newMatchInfo[i][id] != 0) {
          if (!newMatchInfo[i][id]->imis.empty()) {
//             cout << "-----------DO NOT deactivate id " << id 
//                  << ", has nMI[" << i << "]" << endl;
            return false;
          }
        }
      }
    }
  }
  if (checkRange) {
    if (matchInfo[state][id] != 0) {
      for (unsigned int i = 0; i < matchInfo[state][id]->imis.size(); i++) {
        if (!matchInfo[state][id]->imis[i].range) {
//           cout << "-----------DO NOT deactivate id " << id 
//                << ", range=FALSE" << endl;
          return false;
        }
      }
    }
  }
  return true;
}

/*
\subsection{Function ~clearMatchInfo~}

Copy the contents of newMatchInfo into matchInfo, and clear newMatchInfo.

*/
void IndexMatchSuper::clearMatchInfo() {
  TupleId pred(0), pred2(0);
  for (int i = 0; i < p->getNFAsize(); i++) {
    if (newMatchInfo[i] == 0) { // nothing found, clear matchInfo
      if (matchInfo[i] != 0) {
        TupleId id = matchInfo[i][0]->succ;
        while (id > 0) {
          pred = id;
          id = matchInfo[i][id]->succ;
          if (matchInfo[i][pred] != 0) {
            matchInfo[i][pred]->imis.clear();
          }
        }
      }
    }
    else { // copy
      TupleId id2 = matchInfo[i][0]->succ;
      TupleId id = newMatchInfo[i][0]->succ;
      while (id2 < id) {
        pred2 = id2;
        id2 = matchInfo[i][id2]->succ;
        matchInfo[i][matchInfo[i][pred2]->pred]->succ = id2;
        matchInfo[i][id2]->pred = matchInfo[i][pred2]->pred;
        delete matchInfo[i][pred2];
        matchInfo[i][pred2] = 0;
      }
      while (id > 0) {
        matchInfo[i][id]->imis = newMatchInfo[i][id]->imis;
        pred = id;
        id = newMatchInfo[i][id]->succ;
        newMatchInfo[i][pred]->imis.clear();
      }
    }
    if (pred > 0 && newMatchInfo[i][pred] != 0) {
      newMatchInfo[i][pred]->imis.clear();
    }
  }
//   cout << "after clearMatchInfo: " << (*matchInfoPtr)[0][0].succ << " "
//        << (*matchInfoPtr)[1][0].succ << " " << (*newMatchInfoPtr)[0][0].succ
//        << " " << (*newMatchInfoPtr)[1][0].succ << endl;
}

/*
\subsection{Function ~clearMatchInfo2~}

Copy the contents of newMatchInfo2 into matchInfo2, and clear newMatchInfo2.

*/
void IndexMatchSuper::clearMatchInfo2() {
  TupleId pred(0), pred2(0);
  for (int i = 0; i < p->getNFAsize(); i++) {
    if (newMatchInfo2[i] == 0) { // nothing found, clear matchInfo
      if (matchInfo2[i] != 0) {
        TupleId id = matchInfo2[i][0]->succ;
        while (id > 0) {
          pred = id;
          id = matchInfo2[i][id]->succ;
          if (matchInfo2[i][pred] != 0) {
            matchInfo2[i][pred]->imis.clear();
          }
        }
      }
    }
    else { // copy
      TupleId id2 = matchInfo2[i][0]->succ;
      TupleId id = newMatchInfo2[i][0]->succ;
      while (id2 < id) {
        pred2 = id2;
        id2 = matchInfo2[i][id2]->succ;
        matchInfo2[i][matchInfo2[i][pred2]->pred]->succ = id2;
        matchInfo2[i][id2]->pred = matchInfo2[i][pred2]->pred;
        delete matchInfo2[i][pred2];
        matchInfo2[i][pred2] = 0;
      }
      while (id > 0) {
        matchInfo2[i][id]->imis = newMatchInfo2[i][id]->imis;
        pred = id;
        id = newMatchInfo2[i][id]->succ;
        newMatchInfo2[i][pred]->imis.clear();
      }
    }
    if (pred > 0 && newMatchInfo2[i][pred] != 0) {
      newMatchInfo2[i][pred]->imis.clear();
    }
  }
//   cout << "after clearMatchInfo: " << (*matchInfoPtr)[0][0].succ << " "
//        << (*matchInfoPtr)[1][0].succ << " " << (*newMatchInfoPtr)[0][0].succ
//        << " " << (*newMatchInfoPtr)[1][0].succ << endl;
}

/*
\subsection{Function ~hasIdIMIs~}

*/
bool IndexMatchSuper::hasIdIMIs(const TupleId id, const bool mainAttr,
                                const int state /* = -1 */) {
  if (mainAttr) {
    if (state == -1) { // check all states
      for (int s = 0; s < p->getNFAsize(); s++) {
        if (matchInfo[s][id] != 0) {
          if (!matchInfo[s][id]->imis.empty()) {
            return true;
          }
        }
        if (newMatchInfo[s][id] != 0) {
          if (!newMatchInfo[s][id]->imis.empty()) {
            return true;
          }
        }
      }
      return false;
    }
    else {
      if (matchInfo[state][id] != 0) {
        if (!matchInfo[state][id]->imis.empty()) {
          return true;
        }
      }
      if (newMatchInfo[state][id] != 0) {
        if (!newMatchInfo[state][id]->imis.empty()) {
          return true;
        }
      }
      return false;
    }
  }
  else {
    if (state == -1) { // check all states
      for (int s = 0; s < p->getNFAsize(); s++) {
        if (matchInfo2[s][id] != 0) {
          if (!matchInfo2[s][id]->imis.empty()) {
            return true;
          }
        }
        if (newMatchInfo2[s][id] != 0) {
          if (!newMatchInfo2[s][id]->imis.empty()) {
            return true;
          }
        }
      }
      return false;
    }
    else {
      if (matchInfo2[state][id] != 0) {
        if (!matchInfo2[state][id]->imis.empty()) {
          return true;
        }
      }
      if (newMatchInfo2[state][id] != 0) {
        if (!newMatchInfo2[state][id]->imis.empty()) {
          return true;
        }
      }
      return false;
    }
  }
}

/*
\subsection{Function ~extendBinding~}

*/
void IndexMatchSuper::extendBinding(IndexMatchInfo& imi, const int atom,
                           const bool wc, const TupleId id /* = 0 */) {
  if (!p->hasConds() && !p->hasAssigns()) {
    return;
  }
  int elem = p->getElemFromAtom(atom);
  imi.set(elem, imi.next - 1);
//   cout << elem << " set! to " << imi.next - 1 << endl;
  PatElem prevE;
  bool prevW = false;
  if (elem > 0) {
    p->getElem(elem - 1, prevE);
    prevW = (prevE.getW() != NO);
  }
  if (!wc && prevW) {
    imi.set(elem - 1, imi.next - 2);
//     cout << elem - 1 << " set* to " << imi.next - 2 << endl;
  }
  if (id > 0) { // totalMatch
    int lastElem = p->getElemFromAtom(p->getSize() - 1);
    for (int i = elem + 1; i < lastElem; i++) {
      imi.set(i, -1);
//       cout << i << " set% to " << -1 << endl;
    }
    imi.set(lastElem, trajSize[id] - 1);
    if (lastElem > 0) {
      if (imi.binding[lastElem - 1] == trajSize[id] - 1) {
        imi.set(lastElem, -1);
      }
    }
  }
  imi.prevElem = p->getElemFromAtom(atom);
//   imi.print(true);
}

/*
\subsection{Function ~deletePattern~}

*/
void IndexMatchSuper::deletePattern() {
  if (p) {
    p->deleteAtomValues(relevantAttrs);
    delete p;
    p = 0;
  }
}

/*
\subsection{Function ~wildcardMatch~}

*/
bool IndexMatchesLI::wildcardMatch(const int state, pair<int, int> trans) {
  IndexMatchInfo *imiPtr;
  bool ok = false;
  int oldEnd = -1;
  TupleId id = matchInfo[state][0]->succ; // first active tuple id
  while (id > 0) {
//      cout << "wildcardMatch: " << (*matchInfoPtr)[state][id].imis.size() 
//           << " IMIs, state=" << state << ", id=" << id << endl;
    unsigned int numOfIMIs = (matchInfo[state][id] != 0 ?
                              matchInfo[state][id]->imis.size() : 0);
    unsigned int i = 0;
    while (!deactivated[id] && (i < numOfIMIs)) {
      imiPtr = &(matchInfo[state][id]->imis[i]);
//       cout << imiPtr->next + 1 << " | " << unitCtr << endl;
      if (imiPtr->next + 1 >= unitCtr) {
        bool match = false;
        IndexMatchInfo newIMI(true, imiPtr->next + 1, imiPtr->binding, 
                              imiPtr->prevElem);
        if (p->hasConds()) { // extend binding for a complete match
          extendBinding(newIMI, trans.first, true);
          if (p->isFinalState(trans.second) && imiPtr->finished(trajSize[id])) {
            oldEnd = newIMI.getTo(p->getElemFromAtom(trans.first));
            match = checkConditions(id, newIMI);
            if (!match) { // reset unsuccessful binding
              newIMI.reset(oldEnd);
            }
          }
        }
        else { // no conditions
//          cout << "   " << p.isFinalState(trans.second) << " *** "
//               << newIMI.finished(trajSize[id]) << endl;
          match = p->isFinalState(trans.second) &&newIMI.finished(trajSize[id]);
        }
        if (match) {
          matches.push_back(id); // complete match
          ok = true;
          removeIdFromMatchInfo(id);
          removeIdFromIndexResult(id, true);
  //         cout << id << " removed (wild match) " << activeTuples 
  //              << " active tuples" << endl;
          i = UINT_MAX - 1;
        }
        else if (!newIMI.exhausted(trajSize[id])) { // continue
          newMatchInfo[trans.second][id]->imis.push_back(newIMI);
  //         cout << "   imi pushed back from wildcardMatch, id " << id << endl;
          ok = true;
        }
      }
      i++;
    }
    if (!ok && !hasIdIMIs(id, state)) { // no IMIs for id and state
      if (!hasIdIMIs(id, true)) { // no IMIs at all for id
        removeIdFromMatchInfo(id);
        removeIdFromIndexResult(id, true);
//         cout << id << " removed (wild mismatch) " << activeTuples 
//              << " active tuples" << endl;
      }
    }
    id = matchInfo[state][id]->succ;
  }
  return ok;
  cout << "return ok" << endl;
}

/*
\subsection{Function ~valuesMatch~}

*/
bool IndexMatchesLI::valuesMatch(const int e, const TupleId id, 
                      IndexMatchInfo& imi, const int newState, const int unit) {
  if (imi.next >= trajSize[id]) {
    return false;
  }
  Tuple *tuple = rel->GetTuple(id, false);
  bool result = false;
  switch (mtype) {
    case MLABEL: {
      MLabel *ml = (MLabel*)(tuple->GetAttribute(attrNo));
      Match<MLabel> match(0, ml);
      result = imiMatch(match, e, id, imi, unit, newState);
      break;
    }
    case MLABELS: {
      MLabels *mls = (MLabels*)(tuple->GetAttribute(attrNo));
      Match<MLabels> match(0, mls);
      result = imiMatch(match, e, id, imi, unit, newState);
      break;
    }
    case MPLACE: {
      MPlace *mp = (MPlace*)(tuple->GetAttribute(attrNo));
      Match<MPlace> match(0, mp);
      result = imiMatch(match, e, id, imi, unit, newState);
      break;
    }
    default: { // PLACES
      MPlaces *mps = (MPlaces*)(tuple->GetAttribute(attrNo));
      Match<MPlaces> match(0, mps);
      result = imiMatch(match, e, id, imi, unit, newState);
      break;
    }
  }
  tuple->DeleteIfAllowed();
  return result;
}

/*
\subsection{Function ~applySetRel~}

*/
void IndexMatchesLI::applySetRel(const SetRel setRel, 
                                 vector<set<pair<TupleId, int> > >& valuePosVec,
                                 set<pair<TupleId, int> >*& result) {
  switch (setRel) {
    case STANDARD: {
      Tools::uniteLastPairs(valuePosVec.size(), valuePosVec);
      result = &(valuePosVec[0]);
      break;
    }
    case DISJOINT: { // will not happen
      break;
    }
    case SUPERSET: {}
    case EQUAL: {
      Tools::intersectPairs(valuePosVec, result);
      break;
    }
    default: { // INTERSECT
      Tools::uniteLastPairs(valuePosVec.size(), valuePosVec);
      result = &(valuePosVec[0]);
      break;
    }
  }
}

/*
\subsection{Function ~simpleMatch~}

*/
bool IndexMatchesLI::simpleMatch(const int e, const int state,
                                  const int newState) {
  bool transition = false;
  storeIndexResult(e);
//   cout << e << " " << indexResult[e].size() << " " << indexResult[e][0].succ 
//        << endl;
  if (indexResult[e] != 0) { // contents found in at least one index
    TupleId id = indexResult[e][0]->succ;
    while (id > 0) {
      if (!deactivated[id]) {
  //       cout << "state " << state << "; elem " << e << "; next id is " << id 
  //            << endl;
  //       cout << "simpleMatch: " << (*matchInfoPtr)[state][id].imis.size()
  //            << " IMIs, state=" << state << ", id=" << id << endl;
        if ((indexResult[e][indexResult[e][id]->pred]->succ == id) &&
            !indexResult[e][id]->units.empty()) {
          unsigned int numOfIMIs = (matchInfo[state][id] != 0 ?
                                    matchInfo[state][id]->imis.size() : 0);
          for (unsigned int i = 0; i < numOfIMIs; i++) {
            set<int>::iterator it = indexResult[e][id]->units.begin();
            while (!deactivated[id] && (it != indexResult[e][id]->units.end())){
              if (valuesMatch(e, id, matchInfo[state][id]->imis[i], 
                  newState, *it)) {
                transition = true;
                if (deactivated[id]) {
                  i = numOfIMIs;
                }
              }
              it++;
            }
          }
        }
        if (!hasIdIMIs(id, true, state)) { // no IMIs
          if (!hasIdIMIs(id, true)) { // no IMIs at all for id
            removeIdFromMatchInfo(id);
            removeIdFromIndexResult(id, true);
  //           cout << id << " removed (index) " << activeTuples 
  //                << " active tuples" << endl;
          }
        }
      }
      id = indexResult[e][id]->succ;
    }
  }
  else { // no result from index
    if (indexMismatch.find(e) != indexMismatch.end()) { // mismatch
      return false;
    }
    else { // disjoint or () or only semantic time information
      PatElem elem;
      TupleId id = matchInfo[e][0]->succ; // first active tuple id
      while (id > 0) {
        if (!deactivated[id]) {
          unsigned int numOfIMIs = (matchInfo[state][id] != 0 ?
                                    matchInfo[state][id]->imis.size() : 0);
          for (unsigned int i = 0; i < numOfIMIs; i++) {
            if (valuesMatch(e, id, matchInfo[state][id]->imis[i], newState,-1)){
              transition = true;
              if (deactivated[id]) {
                i = numOfIMIs;
              }
            }
          }
          if (!hasIdIMIs(id, true, state)) { // no IMIs for id and state
            if (!hasIdIMIs(id, true)) { // no IMIs at all for id
              removeIdFromMatchInfo(id);
              removeIdFromIndexResult(id, true);
  //             cout << id << " removed (no index) " << activeTuples #
  //                  << " active tuples"  << endl;
            }
          }
        }
        id = matchInfo[e][id]->succ;
      }
    }
  }
  return transition;
}

/*
\subsection{Function ~timesMatch~}

*/
bool IndexMatchesLI::timesMatch(const TupleId id, const unsigned int unit,
                                 const PatElem& elem) {
  set<string> ivs;
  elem.getI(ivs);
  SecInterval iv(true);
  getInterval(id, unit, iv);
  return Tools::timesMatch(iv, ivs);
}

/*
\subsection{Function ~checkConditions~}

*/
bool IndexMatchesLI::checkConditions(const TupleId id, IndexMatchInfo& imi) {
  if (!p->hasConds()) {
    return true;
  }
  Tuple *tuple = rel->GetTuple(id, false);
//   cout << "checkConditions: size = " << getMsize(id) 
//        << "; binding(lastVar) = (" << imi.binding[imi.prevVar].first << ", " 
//        << imi.binding[imi.prevVar].second << ")" << endl;
  bool result = false;
  switch (mtype) {
    case MLABEL: {
      MLabel *ml = (MLabel*)(tuple->GetAttribute(attrNo));
      Match<MLabel> match(p, ml);
      result = match.conditionsMatch(*(p->getConds()), imi);
      break;
    }
    case MLABELS: {
      MLabels *mls = (MLabels*)(tuple->GetAttribute(attrNo));
      Match<MLabels> match(p, mls);
      result = match.conditionsMatch(*(p->getConds()), imi);
      break;
    }
    case MPLACE: {
      MPlace *mp = (MPlace*)(tuple->GetAttribute(attrNo));
      Match<MPlace> match(p, mp);
      result = match.conditionsMatch(*(p->getConds()), imi);
      break;
    }
    default: { // PLACES
      MPlaces *mps = (MPlaces*)(tuple->GetAttribute(attrNo));
      Match<MPlaces> match(p, mps);
      result = match.conditionsMatch(*(p->getConds()), imi);
      break;
    }
  }
  tuple->DeleteIfAllowed();
  return result;
}

void IndexMatchInfo::print(const bool printBinding) {
  if (range) {
    cout << "range from " << next << endl;
  }
  else {
    cout << "next: {" << next << "}" << endl;
  }
  if (printBinding) {
    cout << binding.size() << ": ";
    for (unsigned int i = 0; i < binding.size(); i++) {
      cout << i << " ---> [" << getFrom(i) << ", " << getTo(i) << "],   ";
    }
    cout << endl;
  }
}

}
