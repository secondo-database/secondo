
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
      DateTime dur(0, rand() % 86400000 + 3600000, durationtype); // duration
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
    if ((elems[i].hasLabel() && (type.substr(1, 5) == Place::BasicType())) ||
        (elems[i].hasPlace() && (type.substr(1, 5) == Label::BasicType()))) {
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
  ExtBool result = UNDEF;
  vector<pair<int, string> > relevantAttrs;
  int majorValueNo = -1;
  if (isCompatible(tuple->GetTupleType(), attrno, relevantAttrs, majorValueNo)){
    if (initEasyCondOpTrees(tuple, ttype) && !isNFAempty()) {
      TMatch tmatch(this, tuple, ttype, attrno, relevantAttrs, majorValueNo);
      result = tmatch.matches();
    }
  }
  deleteAtomValues(relevantAttrs);
  return result;
}

string Condition::getType(int t, Tuple *tuple /* = 0 */, 
                          ListExpr ttype /* = 0 */) {
  if (t > 99) {
    if (tuple && ttype) {
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
pair<string, Attribute*> Pattern::getPointer(int key, Tuple *tuple /* = 0 */) {
  pair<string, Attribute*> result;
  if (key > 99) { // attribute name
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    AttributeType attrType = tuple->GetTupleType()->GetAttributeType(key - 100);
    string type = sc->GetTypeName(attrType.algId, attrType.typeId);
    result.second = tuple->GetAttribute(key - 100)->Clone();
    result.first = "[const " + type + " pointer "
                 + nl->ToString(listutils::getPtrList(result.second)) + "]";
  }
  else {
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
        result.second = new DateTime(instanttype);
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
  return result;
}

/*
\subsection{Function ~initCondOpTrees~}

For a pattern with conditions, an operator tree structure is prepared.

*/
bool Pattern::initCondOpTrees(Tuple *tuple /* = 0 */, ListExpr ttype /* = 0 */){
  for (unsigned int i = 0; i < conds.size(); i++) { // opTrees for conditions
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
    for (unsigned int j = 0; j < varKeys.size(); j++) { // init pointers
      strAttr = Pattern::getPointer(getKey(j), tuple);
      ptrs.push_back(strAttr.second);
      toReplace = getVar(j) + getType(getKey(j), tuple, ttype);
      q.replace(q.find(toReplace), toReplace.length(), strAttr.first);
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
bool Pattern::initEasyCondOpTrees(Tuple *tuple /* = 0 */, 
                                  ListExpr ttype /* = 0 */) {
  string q(""), part, toReplace("");
  pair<string, Attribute*> strAttr;
  vector<Attribute*> ptrs;
  for (unsigned int i = 0; i < easyConds.size(); i++) {
    if (!easyConds[i].isTreeOk()) {
      q = "query " + easyConds[i].getText();
      for (int j = 0; j < easyConds[i].getVarKeysSize(); j++) { // init pointers
        strAttr = getPointer(easyConds[i].getKey(j), tuple);
        ptrs.push_back(strAttr.second);
        toReplace = easyConds[i].getVar(j)
                  + Condition::getType(easyConds[i].getKey(j), tuple, ttype);
        q.replace(q.find(toReplace), toReplace.length(), strAttr.first);
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
        return FALSE;
      }
    }
    if (p->containsFinalState(states)) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
  pathMatrix = Tools::createSetMatrix(noMainComponents, p->elemToVar.size());
  for (int i = 0; i < noMainComponents; i++) {
    if (!performTransitionsWithMatrix(i, states)) {
      Tools::deleteSetMatrix(pathMatrix, noMainComponents);
      return FALSE;
    }
  }
  if (!p->containsFinalState(states)) {
    Tools::deleteSetMatrix(pathMatrix, noMainComponents);
    return FALSE;
  }
  if (!p->initCondOpTrees(t, ttype)) {
    Tools::deleteSetMatrix(pathMatrix, noMainComponents);
    return FALSE;
  }
  if (!p->hasAssigns()) {
    bool result = findMatchingBinding(0);
    Tools::deleteSetMatrix(pathMatrix, noMainComponents);
    return (result ? TRUE : FALSE);
  }
  return TRUE;
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
  set<string> plabels, plabelsTemp;
  pair<Word, SetRel> values = p->elems[atom].values[pos];
  if (values.first.addr == 0) {
    return true;
  }
  ((Labels*)values.first.addr)->GetValues(plabels);
  return Tools::relationHolds<string>(tlabels, plabels, values.second);
}

/*
\subsection{Function ~placesMatch~}

*/
bool TMatch::placesMatch(const set<pair<string, unsigned int> >& tplaces, 
                         const int atom, const int pos) {
  set<pair<string, unsigned int> > pplaces, pplacesTemp;
  pair<Word, SetRel> values = p->elems[atom].values[pos];
  if (values.first.addr == 0) {
    return true;
  }
  ((Places*)values.first.addr)->GetValues(pplaces);
  return Tools::relationHolds<pair<string, unsigned int> >(tplaces, pplaces, 
                                                           values.second);
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
  int start(-1), end(-1);
  if (attrInfo.second == "mlabel") {
    MLabel *mlabel = (MLabel*)t->GetAttribute(attrInfo.first);
    start = mlabel->Position(iv.start);
    end = mlabel->Position(iv.end);
    if (start == -1 || end == -1) {
      return false;
    }
    set<string> tlabels, plabels, labels;
    string label;
    for (int i = start; i <= end; i++) {
      mlabel->GetValue(i, label);
      tlabels.insert(label);
    }
    ((Labels*)values.first.addr)->GetValues(plabels);
    return Tools::relationHolds<string>(tlabels, plabels, values.second);
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
    return Tools::relationHolds<string>(tlabels, plabels, values.second);
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
    Range<CcReal> trange(true), prange(true);
    MReal *mreal = (MReal*)t->GetAttribute(attrInfo.first);
    UReal ureal(true), urealTemp(true);
    int firstpos(mreal->Position(iv.start)), lastpos(mreal->Position(iv.end));
    if (firstpos >= 0 && lastpos >= firstpos &&
        lastpos < mreal->GetNoComponents()) {
      mreal->Get(firstpos, ureal);
      ureal.AtInterval(iv, urealTemp);
      bool correct = true;
      CcReal ccmin(urealTemp.Min(correct)), ccmax(urealTemp.Max(correct));
      Interval<CcReal> minMax(ccmin, ccmax, true, true);
      set<Interval<CcReal>, ivCmp> ivSet;
      ivSet.insert(minMax);
      for (int i = firstpos + 1; i < lastpos; i++) {
        mreal->Get(i, ureal);
        minMax.start.Set(ureal.Min(correct));
        minMax.end.Set(ureal.Max(correct));
        ivSet.insert(minMax);
      }
      mreal->Get(mreal->Position(iv.end), ureal);
      ureal.AtInterval(iv, urealTemp);
      minMax.start.Set(urealTemp.Min(correct));
      minMax.end.Set(urealTemp.Max(correct));
      ivSet.insert(minMax);
      for (set<Interval<CcReal>, ivCmp>::iterator it = ivSet.begin();
           it != ivSet.end(); it++) {
        trange.MergeAdd(*it);
      }
      return Tools::relationHolds<CcReal>(trange,
                           *((Range<CcReal>*)values.first.addr), values.second);
    }
    return false;
  }
  else {
    cout << "TYPE is " << attrInfo.second << endl;
  }
  
  return true;
}

/*
\subsection{Function ~valuesMatch~}

*/
bool TMatch::valuesMatch(const int u, const int atom) {
  SecInterval iv(true);
  if (!mainValuesMatch(u, atom)) {
    return false;
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
  map<string, pair<int, int> > binding;
  binding[p->elems[atom].var] = make_pair(u, u);
  for (set<int>::iterator it = pos.begin(); it != pos.end(); it++) {
    switch (type) {
      case MLABEL: {
        MLabel *traj = (MLabel*)t->GetAttribute(attrno);
        if (!p->easyConds[*it].evaluate<MLabel>(binding, traj, t, ttype)) {
          return false;
        }
        break;
      }
      case MLABELS: {
        MLabels *traj = (MLabels*)t->GetAttribute(attrno);
        if (!p->easyConds[*it].evaluate<MLabels>(binding, traj, t, ttype)) {
          return false;
        }
        break;
      }
      case MPLACE: {
        MPlace *traj = (MPlace*)t->GetAttribute(attrno);
        if (!p->easyConds[*it].evaluate<MPlace>(binding, traj, t, ttype)) {
          return false;
        }
        break;
      }
      case MPLACES: {
        MPlaces *traj = (MPlaces*)t->GetAttribute(attrno);
        if (!p->easyConds[*it].evaluate<MPlaces>(binding, traj, t, ttype)) {
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
        if (Tools::timesMatch(iv, ivs) && valuesMatch(u, it->first)
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
        if (Tools::timesMatch(iv, ivs) && valuesMatch(u, it->first)
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
      if (Tools::timesMatch(iv, ivs) && valuesMatch(u, it->first)
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
bool TMatch::conditionsMatch(const map<string, pair<int, int> > &binding) {
  for (unsigned int i = 0; i < p->conds.size(); i++) {
    switch (type) {
      case MLABEL: {
        if (!p->conds[i].evaluate(binding, (MLabel*)t->GetAttribute(attrno), t,
                                  ttype)) {
//           cout << "False cond " << p->conds[i].getText() << endl;
          return false;
        }
        break;
      }
      case MLABELS: {
        if (!p->conds[i].evaluate(binding, (MLabels*)t->GetAttribute(attrno), t,
                                  ttype)){
          return false;
        }
        break;
      }
      case MPLACE: {
        if (!p->conds[i].evaluate(binding, (MPlace*)t->GetAttribute(attrno), t,
                                  ttype)) {
          return false;
        }
        break;
      }
      case MPLACES: {
        if (!p->conds[i].evaluate(binding,(MPlaces*)t->GetAttribute(attrno), t,
                                  ttype)) {
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
bool TMatch::findBinding(const int u, const int elem,
                         map<string, pair<int, int> > &binding) {
  string var = p->elemToVar[elem];
  bool inserted = false;
  if (!var.empty()) {
    if (binding.count(var)) { // extend existing binding
      binding[var].second++;
    }
    else { // add new variable
      binding[var] = make_pair(u, u);
      inserted = true;
    }
  }
  if (*(pathMatrix[u][elem].begin()) == UINT_MAX) { // complete match
    if (conditionsMatch(binding)) {
      return true;
    }
  }
  else {
    for (set<unsigned int>::reverse_iterator it = pathMatrix[u][elem].rbegin();
         it != pathMatrix[u][elem].rend(); it++) {
      if (findBinding(u + 1, *it, binding)) {
        return true;
      }
    }
  }
  if (!var.empty()) { // unsuccessful: reset binding
    if (inserted) {
      binding.erase(var);
    }
    else {
      binding[var].second--;
    }
  }
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
  map<string, pair<int, int> > binding;
  map<int, int>::reverse_iterator itm;
  for (itm = transitions.rbegin(); itm != transitions.rend(); itm++) {
    if (findBinding(0, p->atomicToElem[itm->first], binding)) {
      return true;
    }
  }
  return false;
}

/*
\section{Functions for class ~TupleIndex~}

\subsection{auxiliary Functions for Secondo support}

*/
TupleIndex::TupleIndex(vector<InvertedFile*> t, vector<BTree*> b, 
  vector<RTree1TLLI*> r1, vector<RTree2TLLI*> r2, RTree1TLLI *tI, 
  map<int, pair<IndexType,int> > aI, map<pair<IndexType,int>, int> iA, int mA) {
  tries = t;
  btrees = b;
  rtrees1 = r1;
  rtrees2 = r2;
  timeIndex = tI;
  attrToIndex = aI;
  indexToAttr = iA;
  mainAttr = mA;
}

TupleIndex::TupleIndex(TupleIndex &src) {
  assert(false);
  tries = src.tries;
  btrees = src.btrees;
  rtrees1 = src.rtrees1;
  rtrees2 = src.rtrees2;
  timeIndex = src.timeIndex;
  attrToIndex = src.attrToIndex;
  indexToAttr = src.indexToAttr;
  mainAttr = src.mainAttr;
}

bool TupleIndex::checkType(const ListExpr list) {
  return listutils::isSymbol(list, BasicType());
}

ListExpr TupleIndex::Property() {
  return (nl->TwoElemList(
    nl->FourElemList(
      nl->StringAtom("Signature"), nl->StringAtom("Example Type List"),
      nl->StringAtom("List Rep"), nl->StringAtom("Example List")),
    nl->FourElemList (
      nl->StringAtom("-> SIMPLE"), nl->StringAtom(TupleIndex::BasicType()),
      nl->StringAtom("no list representation"),
      nl->StringAtom(""))));
}

Word TupleIndex::In(const ListExpr typeInfo, const ListExpr instance,
                    const int errorPos, ListExpr& errorInfo, bool& correct) {
  correct = false;
  return SetWord(Address(0));
}

ListExpr TupleIndex::Out(ListExpr typeInfo, Word value) {
  ListExpr overviewlist, rtree1list, rtree2list, last1, last2;
  TupleIndex *ti = (TupleIndex*)value.addr;
  stringstream overview;
  Word val;
  overview << ti->tries.size() << " Tries," << endl << ti->btrees.size()
           << " BTrees," << endl << ti->rtrees1.size() << " 1-dim RTrees," 
           << endl << ti->rtrees2.size() << " 2-dim RTrees. More:" << endl;
    overviewlist = nl->TextAtom(overview.str());
  cout << overview.str() << endl;
  if (ti->rtrees1.size() > 0) {
    val.addr = ti->rtrees1[0];
    rtree1list = nl->OneElemList(OutRTree<1>(nl->FourElemList(nl->Empty(),
                nl->Empty(), nl->Empty(), nl->BoolAtom(true)), val));
    last1 = rtree1list;
  }
  for (unsigned int i = 1; i < ti->rtrees1.size(); i++) {
    val.addr = ti->rtrees1[i];
    last1 = nl->Append(last1, OutRTree<1>(nl->FourElemList(nl->Empty(),
                nl->Empty(), nl->Empty(), nl->BoolAtom(true)), val));
  }
  if (ti->rtrees2.size() > 0) {
    val.addr = ti->rtrees2[0];
    rtree2list = nl->OneElemList(OutRTree<2>(nl->FourElemList(nl->Empty(),
                nl->Empty(), nl->Empty(), nl->BoolAtom(true)), val));
    last2 = rtree2list;
  }
  for (unsigned int i = 1; i < ti->rtrees2.size(); i++) {
    val.addr = ti->rtrees2[i];
    last2 = nl->Append(last2, OutRTree<2>(nl->FourElemList(nl->Empty(),
                nl->Empty(), nl->Empty(), nl->BoolAtom(true)), val));
  }  
  return nl->ThreeElemList(overviewlist, rtree1list, rtree2list);
}

Word TupleIndex::Create(const ListExpr typeInfo) {
  Word w;
  w.addr = (new TupleIndex(true));
  return w;
}

void TupleIndex::Delete(const ListExpr typeInfo, Word& w) {
  TupleIndex *ti = (TupleIndex*)w.addr;
  delete ti;
  w.addr = 0;
}

bool TupleIndex::Save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value) {
  SecondoCatalog *sc = SecondoSystem::GetCatalog();
  TupleIndex *ti = static_cast<TupleIndex*>(value.addr);
  unsigned int noComponents = ti->tries.size();
  if (!valueRecord.Write(&noComponents, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  Word val;
  ListExpr tList = sc->NumericType(nl->SymbolAtom(InvertedFile::BasicType()));
  for (unsigned int i = 0; i < noComponents; i++) {
    val.addr = ti->tries[i];
    cout << "save trie # " << i + 1 << " of " << noComponents << endl;
    if (!triealg::SaveInvfile(valueRecord, offset, tList, val)) {
      cout << "error saving trie " << i << endl;
      return false;
    }
  }
  noComponents = ti->btrees.size();
  if (!valueRecord.Write(&noComponents, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  tList = sc->NumericType(nl->SymbolAtom(BTree::BasicType()));
  for (unsigned int i = 0; i < noComponents; i++) {
    cout << "save btree # " << i + 1 << " of " << noComponents << endl;
    if (!ti->btrees[i]->Save(valueRecord, offset, tList)) {
      cout << "error saving btree " << i << endl;
      return false;
    }
  }
  noComponents = ti->rtrees1.size();
  if (!valueRecord.Write(&noComponents, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  tList = nl->FourElemList(nl->Empty(), nl->Empty(), nl->Empty(), 
                           nl->BoolAtom(true));
  for (unsigned int i = 0; i < noComponents; i++) {
    val.addr = ti->rtrees1[i];
    cout << "save rtree1 # " << i + 1 << " of " << noComponents << endl;
    if (!SaveRTree<1>(valueRecord, offset, tList, val)) {
      cout << "error saving rtree1 " << i << endl;
      return false;
    }
  }
  noComponents = ti->rtrees2.size();
  if (!valueRecord.Write(&noComponents, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  for (unsigned int i = 0; i < noComponents; i++) {
    val.addr = ti->rtrees2[i];
    cout << "save rtree2 # " << i + 1 << " of " << noComponents << endl;
    if (!SaveRTree<2>(valueRecord, offset, tList, val)) {
      cout << "error saving rtree2 " << i << endl;
      return false;
    }
  }
  val.addr = ti->timeIndex;
  cout << "save time index" << endl;
  if (!SaveRTree<1>(valueRecord, offset, tList, val)) {
    cout << "error saving timeIndex" << endl;
    return false;
  }
  noComponents = ti->attrToIndex.size();
  if (!valueRecord.Write(&noComponents, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  map<int, pair<IndexType, int> >::iterator it1 = ti->attrToIndex.begin();
  while (it1 != ti->attrToIndex.end()) {
    if (!valueRecord.Write(&it1->first, sizeof(int), offset)) {
      return false;
    }
    offset += sizeof(int);
    if (!valueRecord.Write(&it1->second.first, sizeof(IndexType), offset)) {
      return false;
    }
    offset += sizeof(IndexType);
    if (!valueRecord.Write(&it1->second.second, sizeof(int), offset)) {
      return false;
    }
    offset += sizeof(int);
    it1++;
  }
  cout << "attrToIndex saved" << endl;
  noComponents = ti->indexToAttr.size();
  if (!valueRecord.Write(&noComponents, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  map<pair<IndexType, int>, int>::iterator it2 = ti->indexToAttr.begin();
  while (it2 != ti->indexToAttr.end()) {
    if (!valueRecord.Write(&it2->first.first, sizeof(IndexType), offset)) {
      return false;
    }
    offset += sizeof(IndexType);
    if (!valueRecord.Write(&it2->first.second, sizeof(int), offset)) {
      return false;
    }
    offset += sizeof(int);
    if (!valueRecord.Write(&it2->second, sizeof(int), offset)) {
      return false;
    }
    offset += sizeof(int);
    it2++;
  }
  cout << "indexToAttr saved" << endl;
  if (!valueRecord.Write(&(ti->mainAttr), sizeof(int), offset)) {
    return false;
  }
  cout << "mainAttr = " << ti->mainAttr << " saved" << endl;
  offset += sizeof(int);
  return true;
}

bool TupleIndex::Open(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value) {
  SecondoCatalog *sc = SecondoSystem::GetCatalog();
  TupleIndex *ti = new TupleIndex();
  Word val;
  ListExpr tList = sc->NumericType(nl->SymbolAtom(InvertedFile::BasicType()));
  unsigned int noComponents;
  if (!valueRecord.Read(&noComponents, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
//   cout << "There are " << noComponents << " tries, ";
  for (unsigned int i = 0; i < noComponents; i++) {
    if (!triealg::OpenInvfile(valueRecord, offset, tList, val)) {
      cout << "error opening trie" << endl;
      return false;
    }
    ti->tries.push_back((InvertedFile*)val.addr);
  }
  if (!valueRecord.Read(&noComponents, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
//   cout << "There are " << noComponents << " btrees, ";
  tList = sc->NumericType(nl->SymbolAtom(BTree::BasicType()));
  for (unsigned int i = 0; i < noComponents; i++) {
    if (!((BTree*)(val.addr))->Open(valueRecord, offset, tList)) {
      cout << "error opening btree" << endl;
      return false;
    }
    ti->btrees.push_back((BTree*)val.addr);
  }
  if (!valueRecord.Read(&noComponents, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
//   cout << "There are " << noComponents << " rtree1s, ";
  tList = nl->FourElemList(nl->Empty(), nl->Empty(), nl->Empty(), 
                           nl->BoolAtom(true));
  for (unsigned int i = 0; i < noComponents; i++) {
    if (!OpenRTree<1>(valueRecord, offset, tList, val)) {
      cout << "error opening rtree1" << endl;
      return false;
    }
    ti->rtrees1.push_back((RTree1TLLI*)val.addr);
  }
  if (!valueRecord.Read(&noComponents, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
//   cout << "There are " << noComponents << " rtree2s, ";
  for (unsigned int i = 0; i < noComponents; i++) {
    if (!OpenRTree<2>(valueRecord, offset, tList, val)) {
      cout << "error opening rtree2" << endl;
      return false;
    }
    ti->rtrees2.push_back((RTree2TLLI*)val.addr);
  }
  if (!OpenRTree<1>(valueRecord, offset, tList, val)) {
    cout << "error opening time index" << endl;
    return false;
  }
  ti->timeIndex = (RTree1TLLI*)val.addr;
//   cout << "Time index opened, ";
  if (!valueRecord.Read(&noComponents, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  int attr;
  pair<IndexType, int> indexPos;
  for (unsigned int i = 0; i < noComponents; i++) {
    if (!valueRecord.Read(&attr, sizeof(int), offset)) {
      return false;
    }
    offset += sizeof(int);
    if (!valueRecord.Read(&indexPos.first, sizeof(IndexType), offset)) {
      return false;
    }
    offset += sizeof(IndexType);
    if (!valueRecord.Read(&indexPos.second, sizeof(int), offset)) {
      return false;
    }
    offset += sizeof(int);
    ti->attrToIndex[attr] = indexPos;
  }
  if (!valueRecord.Read(&noComponents, sizeof(unsigned int), offset)) {
    return false;
  }
  offset += sizeof(unsigned int);
  for (unsigned int i = 0; i < noComponents; i++) {
    if (!valueRecord.Read(&(indexPos.first), sizeof(IndexType), offset)) {
      return false;
    }
    offset += sizeof(IndexType);
    if (!valueRecord.Read(&(indexPos.second), sizeof(int), offset)) {
      return false;
    }
    offset += sizeof(int);
    if (!valueRecord.Read(&attr, sizeof(int), offset)) {
      return false;
    }
    offset += sizeof(int);
    ti->indexToAttr[indexPos] = attr;
  }
  if (!valueRecord.Read(&(ti->mainAttr), sizeof(int), offset)) {
    return false;
  }
  offset += sizeof(int);
  value = SetWord(ti);
//   cout << "TupleIndex opened succesfully" << endl;
  return true;
}

void TupleIndex::Close(const ListExpr typeInfo, Word& w) {
  TupleIndex *ti = (TupleIndex*)w.addr;
  delete ti;
  w.addr = 0;
}

Word TupleIndex::Clone(const ListExpr typeInfo, const Word& w) {
  TupleIndex *ti = (TupleIndex*)w.addr;
  Word res;
  res.addr = new TupleIndex(*ti);
  return res;
}

int TupleIndex::SizeOfObj() {
  return sizeof(TupleIndex);
}

bool TupleIndex::TypeCheck(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, BasicType());
}

TypeConstructor tupleindexTC(
  TupleIndex::BasicType(),
  TupleIndex::Property,
  TupleIndex::Out, TupleIndex::In,
  0, 0,
  TupleIndex::Create, TupleIndex::Delete,
  TupleIndex::Open, TupleIndex::Save,
  TupleIndex::Close, TupleIndex::Clone,
  0,
  TupleIndex::SizeOfObj,
  TupleIndex::TypeCheck);

/*
\subsection{Function ~initialize~}

*/
void TupleIndex::initialize(TupleType *ttype, int _mainAttr) {
  mainAttr = _mainAttr;
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  ListExpr typeInfo = nl->FourElemList(nl->Empty(), nl->Empty(), 
                                       nl->Empty(), nl->BoolAtom(true));
  timeIndex = (RTree1TLLI*)((CreateRTree<1>(typeInfo)).addr);
  cout << "RTree1 for time intervals created" << endl;
  for (int i = 0; i < ttype->GetNoAttributes(); i++) {
    AttributeType atype = ttype->GetAttributeType(i);
    string name = sc->GetTypeName(atype.algId, atype.typeId);
    if (name == "mlabel" || name == "mlabels" || name == "mplace" || 
        name == "mplaces") {
      attrToIndex[i] = make_pair(TRIE, (int)tries.size());
      indexToAttr[make_pair(TRIE, (int)tries.size())] = i;
      ListExpr tInfo = sc->NumericType(nl->SymbolAtom(InvertedFile
                                                                ::BasicType()));
//       InvertedFile *inv = new InvertedFile();
      Word value = triealg::CreateInvfile(tInfo);
      InvertedFile *inv = (InvertedFile*)value.addr;
      inv->setParams(false, 1, "");
      tries.push_back(inv);
      cout << "Trie for attr " << i << " created and appended" << endl;
    }
    else if (name == "mint") {
      attrToIndex[i] = make_pair(BTREE, (int)btrees.size());
      indexToAttr[make_pair(BTREE, (int)btrees.size())] = i;
      BTree *btree = new BTree(SmiKey::Integer);
      btrees.push_back(btree);
      cout << "BTree for attr " << i << " created and appended" << endl;
    }
    else if (name == "mreal") {
      attrToIndex[i] = make_pair(RTREE1, (int)rtrees1.size());
      indexToAttr[make_pair(RTREE1, (int)rtrees1.size())] = i;
//       R_Tree<1, NewPair<TupleId, int> > *rtree1 = 
//                   new R_Tree<1, NewPair<TupleId, int> >(4096);
      typeInfo = nl->FourElemList(nl->Empty(), nl->Empty(), nl->Empty(), 
                                  nl->BoolAtom(true));
      RTree1TLLI *rtree1 = (RTree1TLLI*)((CreateRTree<1>(typeInfo)).addr);
      rtrees1.push_back(rtree1);
      cout << "RTree1 for attr " << i << " created and appended" << endl;
    }
    else if (name == "mpoint" || name == "mregion") {
      attrToIndex[i] = make_pair(RTREE2, (int)rtrees2.size());
      indexToAttr[make_pair(RTREE2, (int)rtrees2.size())] = i;
//       R_Tree<2, NewPair<TupleId, int> > *rtree2 = 
//                   new R_Tree<2, NewPair<TupleId, int> >(4096);
      typeInfo = nl->FourElemList(nl->Empty(), nl->Empty(), nl->Empty(), 
                                  nl->BoolAtom(false));
      RTree2TLLI *rtree2 = (RTree2TLLI*)((CreateRTree<2>(typeInfo)).addr);
      rtrees2.push_back(rtree2);
      cout << "RTree2 for attr " << i << " created and appended" << endl;
    }
    else {
      attrToIndex[i] = make_pair(NONE, -1);
    }
  }
}

/*
\subsection{Function ~insertIntoTrie~}

*/
void TupleIndex::insertIntoTrie(InvertedFile *inv, TupleId tid, Attribute *traj,
                           DataType type, appendcache::RecordAppendCache* cache,
                                TrieNodeCacheType* trieCache) {
  switch (type) {
    case MLABEL: {
      string value;
      for (int j = 0; j < ((MLabel*)traj)->GetNoComponents(); j++) {
        ((MLabel*)traj)->GetValue(j, value);
        inv->insertString(tid, value, j, 0, cache, trieCache);
//         cout << "inserted " << tid << " " << value << " " << j << endl;
      }
      break;
    }
    case MPLACE: {
      pair<string, charPosType> value;
      for (int j = 0; j < ((MPlace*)traj)->GetNoComponents(); j++) {
        ((MPlace*)traj)->GetValue(j, value);
        inv->insertString(tid, value.first, j, value.second, cache, trieCache);
      }
      break;
    }
    case MLABELS: {
      set<string> values;
      for (int j = 0; j < ((MLabels*)traj)->GetNoComponents(); j++) {
        ((MLabels*)traj)->GetValues(j, values);
        for (set<string>::iterator it = values.begin();it != values.end();it++){
          inv->insertString(tid, *it, j, 0, cache, trieCache);
        }
      }
      break;
    }
    default: { // mplaces
      set<pair<string, charPosType> > values;
      for (int j = 0; j < ((MPlaces*)traj)->GetNoComponents(); j++) {
        ((MPlaces*)traj)->GetValues(j, values);
        for (set<pair<string, charPosType> >::iterator it = values.begin();
                                                     it != values.end(); it++) {
          inv->insertString(tid, it->first, j, it->second, cache, trieCache);
        }
      }
    }
  }
}

/*
\subsection{Function ~fillTimeIndex~}

*/
bool TupleIndex::fillTimeIndex(RTree1TLLI* rt, TupleId tid, Attribute *traj,
                               DataType type) {
  SecInterval iv(true);
  double start[1], end[1];
  int noComponents = -1;
  switch (type) {
    case MLABEL: {
      noComponents = ((MLabel*)traj)->GetNoComponents();
      break;
    }
    case MLABELS: {
      noComponents = ((MLabels*)traj)->GetNoComponents();
      break;
    }
    case MPLACE: {
      noComponents = ((MPlace*)traj)->GetNoComponents();
      break;
    }
    default: { // MPLACES
      noComponents = ((MPlaces*)traj)->GetNoComponents();
      break;
    }
  }
  for (int i = 0; i < noComponents; i++) {
    switch (type) {
      case MLABEL: {
        ((MLabel*)traj)->GetInterval(i, iv);
        break;
      }
      case MLABELS: {
        ((MLabels*)traj)->GetInterval(i, iv);
        break;
      }
      case MPLACE: {
        ((MPlace*)traj)->GetInterval(i, iv);
        break;
      }
      default: { // MPLACES
        ((MPlaces*)traj)->GetInterval(i, iv);
        break;
      }
    }
    start[0] = iv.start.ToDouble();
    end[0] = iv.end.ToDouble();
    Rectangle<1> doubleIv(true, start, end);
    if (doubleIv.IsDefined()) {
      TwoLayerLeafInfo position(tid, i, i);
      R_TreeLeafEntry<1, TwoLayerLeafInfo> entry(doubleIv, position);
      rt->Insert(entry);
//       cout << "Inserted (" << start[0] << ", " << end[0] << ")" << endl;
    }
  }
  return true;
}

/*
\subsection{Function ~insertIntoBTree~}

*/
void TupleIndex::insertIntoBTree(BTree *bt, TupleId tid, MInt *mint) {
  UInt unit(true);
  for (int i = 0; i < mint->GetNoComponents(); i++) {
    mint->Get(i, unit);
    bt->Append(unit.constValue.GetValue(), tid);
  }
}

/*
\subsection{Function ~insertIntoRTree1~}

*/
bool TupleIndex::insertIntoRTree1(RTree1TLLI *rt, TupleId tid, Attribute *m) {
  double start[1], end[1];
  UReal unit(true);
  bool correct1(true), correct2(true);
  for (int i = 0; i < ((MReal*)m)->GetNoComponents(); i++) {
    ((MReal*)m)->Get(i, unit);
    start[0] = unit.Min(correct1);
    end[0] = unit.Max(correct2);
    if (!correct1 || !correct2) {
      cout << "Error at unit " << i << ", tuple " << tid << endl;
      return false;
    }
    Rectangle<1> doubleIv(true, start, end);
    TwoLayerLeafInfo position(tid, i, i);
    R_TreeLeafEntry<1, TwoLayerLeafInfo> entry(doubleIv, position);
    rt->Insert(entry);
  }
  return true;
}

/*
\subsection{Function ~insertIntoRTree2~}

*/
bool TupleIndex::insertIntoRTree2(RTree2TLLI *rt, TupleId tid, Attribute *m,
                                  string type) {
  if (type == "mpoint") {
    UPoint up(true);
    for (int i = 0; i < ((MPoint*)m)->GetNoComponents(); i++) {
      ((MPoint*)m)->Get(i, up);
      if (up.IsDefined()) {
        TwoLayerLeafInfo position(tid, i, i);
        R_TreeLeafEntry<2, TwoLayerLeafInfo> entry(up.BoundingBoxSpatial(), 
                                                   position);
        rt->Insert(entry);
      }
    }
  }
  else if (type == "mregion") {
    URegionEmb ur(true);
    double min[2], max[2];
    for (int i = 0; i < ((MRegion*)m)->GetNoComponents(); i++) {
      ((MRegion*)m)->Get(i, ur);
      if (ur.IsDefined()) {
        TwoLayerLeafInfo position(tid, i, i);
        Rectangle<2> bbox2d(true);
        min[0] = ur.BoundingBox().MinD(0);
        max[0] = ur.BoundingBox().MaxD(0);
        min[1] = ur.BoundingBox().MinD(1);
        max[1] = ur.BoundingBox().MaxD(1);
        bbox2d.Set(true, min, max);
        R_TreeLeafEntry<2, TwoLayerLeafInfo> entry(bbox2d, position);
        rt->Insert(entry);
      }
    }
  }
  else {
    cout << "Invalid type " << type << endl;
    return false;
  }
  return true;
}

/*
\subsection{Function ~addTuple~}

*/
bool TupleIndex::addTuple(Tuple *tuple) {
  for (int i = 0; i < tuple->GetNoAttributes(); i++) {
    pair<IndexType, int> indexPos = attrToIndex[i];
    if (indexPos.second > -1) {
      if (indexPos.first == TRIE) {
        size_t maxMem = 0;/*qp->GetMemorySize(s) * 1024 * 1024*/
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
        InvertedFile *inv = tries[indexPos.second];
        appendcache::RecordAppendCache* cache = 
                                           inv->createAppendCache(invCacheSize);
        TrieNodeCacheType* trieCache = inv->createTrieCache(trieCacheSize);
//         cout << "INSERT INTO TRIE " << indexPos.second << endl;
        insertIntoTrie(inv, tuple->GetTupleId(), tuple->GetAttribute(i),
                Tools::getDataType(tuple->GetTupleType(), i), cache, trieCache);
        delete trieCache;
        delete cache;
        if (i == mainAttr) {
//           cout << "FILL TIME INDEX" << endl;
          if (!fillTimeIndex(timeIndex, tuple->GetTupleId(), 
        tuple->GetAttribute(i), Tools::getDataType(tuple->GetTupleType(), i))) {
            cout << "Error adding tuple " << tuple->GetTupleId() << endl;
            return false;
          }
        }
      }
      else if (indexPos.first == BTREE) {
//         cout << "INSERT INTO BTREE" << endl;
        insertIntoBTree(btrees[indexPos.second], tuple->GetTupleId(),
          (MInt*)(tuple->GetAttribute(i)));
      }
      else if (indexPos.first == RTREE1) {
//         cout << "INSERT INTO RTREE1 " << indexPos.second << endl;
        if (!insertIntoRTree1(rtrees1[indexPos.second], tuple->GetTupleId(),
                              tuple->GetAttribute(i))) {
          cout << "Error adding tuple " << tuple->GetTupleId() << endl;
          return false;
        }
      }
      else if (indexPos.first == RTREE2) {
//         cout << "INSERT INTO RTREE2" << endl;
        if (!insertIntoRTree2(rtrees2[indexPos.second], tuple->GetTupleId(),
        tuple->GetAttribute(i), Tools::getTypeName(tuple->GetTupleType(), i))) {
          cout << "Error adding tuple " << tuple->GetTupleId() << endl;
          return false;
        }
      }
    }
  }
  return true;
}

/*
\subsection{Function ~deleteIndexes~}

*/
void TupleIndex::deleteIndexes() {
  for (unsigned int i = 0; i < tries.size(); i++) {
    delete tries[i];
  }
  for (unsigned int i = 0; i < btrees.size(); i++) {
    delete btrees[i];
  }
  for (unsigned int i = 0; i < rtrees1.size(); i++) {
    delete rtrees1[i];
  }
  for (unsigned int i = 0; i < rtrees2.size(); i++) {
    delete rtrees2[i];
  }
  delete timeIndex;
}

/*
\section{Functions for class ~TMatchIndexLI~}

\subsection{Constructor}

*/
TMatchIndexLI::TMatchIndexLI(Relation *r, ListExpr tt, TupleIndex *t, int a, 
                             Pattern *pat, int majorValueNo, DataType type) :
  IndexMatchSuper(r, pat, a, type), ttList(tt), ti(t), valueNo(majorValueNo) {} 

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
    indexType = ti->attrToIndex[i].first;
    pos = ti->attrToIndex[i].second;
    switch (indexType) {
      case TRIE: {
        if (attrType.second != "mlabel" && attrType.second != "mplace" &&
            attrType.second != "mlabels" && attrType.second != "mplaces") {
          return false;
        }
        if (pos >= ti->tries.size()) {
          return false;
        }
        break;
      }
      case BTREE: {
        if (attrType.second != "mint") {
          return false;
        }
        if (pos >= ti->btrees.size()) {
          return false;
        }
        break;
      }
      case RTREE1: {
        if (attrType.second != "mreal") {
          return false;
        }
        if (pos >= ti->rtrees1.size()) {
          return false;
        }
        break;
      }
      case RTREE2: {
        if (attrType.second != "mpoint" && attrType.second != "mregion") {
          return false;
        }
        if (pos >= ti->rtrees2.size()) {
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
\subsection{Function ~getSingleIndexResult~}

*/
bool TMatchIndexLI::getSingleIndexResult(pair<int, pair<IndexType, int> > 
 indexInfo, pair<Word, SetRel> values, int valueNo, vector<set<int> > &result) {
  string type = nl->ToString(nl->Second(nl->Nth(indexInfo.first + 1, 
                                                nl->Second(ttList))));
  switch (indexInfo.second.first) {
    case TRIE: {
      if (values.first.addr == 0) {
        return false; // no content
      }
      if (type.substr(0, 6) == "mlabel") { // mlabel or mlabels
        set<string> lbs;
        ((Labels*)(values.first.addr))->GetValues(lbs);
        if (valueNo == 0 && lbs.empty()) {
          return false; // first access unsuccessful
        }
        set<string>::iterator it = lbs.begin();
        for (int i = 0; i < valueNo; i++) {
          it++;
        }
        Tools::queryTrie(ti->tries[indexInfo.second.second], *it, result);
        return (int)(lbs.size()) > valueNo + 1; // TRUE iff there is a successor
      }
      else if (type.substr(0, 6) == "mplace") {
        set<pair<string, unsigned int> > pls;
        ((Places*)(values.first.addr))->GetValues(pls);
        if (valueNo == 0 && pls.empty()) {
          return false; // first access unsuccessful
        }
        set<pair<string, unsigned int> >::iterator it = pls.begin();
        for (int i = 0; i < valueNo; i++) {
          it++;
        }
        Tools::queryTrie(ti->tries[indexInfo.second.second], *it, result);
//         for (unsigned int i = 0; i < result.size(); i++) {
//           cout << "|" << i << ": " << result[i].size() << " ";
//         }
//         cout << endl;
        return (int)(pls.size()) > valueNo + 1; // TRUE iff there is a successor
      }
      break;
    }
    case BTREE: {
//       cout << "BTREE, type " << type << ", pos " << indexInfo.second.second 
//            << endl;
      if (values.first.addr == 0) {
        return false; // no content
      }
      Range<CcReal> *range = (Range<CcReal>*)(values.first.addr);
      Interval<CcReal> iv;
      if (valueNo == 0 && range->IsEmpty()) {
        return false; // first access unsuccessful
      }
      range->Get(valueNo, iv);
     // TODO: Tools::queryBtree(ti->btree[indexInfo.second.second], iv, result);
//       for (unsigned int i = 0; i < result.size(); i++) {
//         cout << "|" << i << ": " << result[i].size() << " ";
//       }
//       cout << endl;
      return range->GetNoComponents() > valueNo + 1;
      
    }
    case RTREE1: {
//       cout << "RTREE1, type " << type << ", pos " << indexInfo.second.second 
//            << endl;
      if (values.first.addr == 0) {
        return false; // no content
      }
      Range<CcReal> *range = (Range<CcReal>*)(values.first.addr);
      Interval<CcReal> iv;
      if (valueNo == 0 && range->IsEmpty()) {
        return false; // first access unsuccessful
      }
      range->Get(valueNo, iv);
      Tools::queryRtree1(ti->rtrees1[indexInfo.second.second], iv, result);
//       for (unsigned int i = 0; i < result.size(); i++) {
//         cout << "|" << i << ": " << result[i].size() << " ";
//       }
//       cout << endl;
      return range->GetNoComponents() > valueNo + 1;
    }
    case RTREE2: {
//       cout << "RTREE2, type " << type << ", pos " << indexInfo.second.second 
//            << endl;
      if (values.first.addr == 0) {
        return false; // no content
      }
      Rectangle<2> rect = ((Region*)(values.first.addr))->BoundingBox();
      if (rect.IsEmpty()) {
        return false; // first access unsuccessful
      }
      Tools::queryRtree2(ti->rtrees2[indexInfo.second.second], rect, result);
//       for (unsigned int i = 0; i < result.size(); i++) {
//         cout << "|" << i << ": " << result[i].size() << " ";
//       }
//       cout << endl;
      return false;
    }
    default: { // case NONE
      break;
    }
  }
  return false;
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
\subsection{Function ~getResultForAtomPart~}

*/
void TMatchIndexLI::getResultForAtomPart(pair<int, pair<IndexType, int> >
              indexInfo, pair<Word, SetRel> values, vector<Periods*> &result) {
  vector<set<int> > temp1, temp2;
  temp1.resize(rel->GetNoTuples() + 1);
  temp2.resize(rel->GetNoTuples() + 1);
  result.resize(rel->GetNoTuples() + 1);
  int valueNo = 0;
//   vector<set<int> > *ptr(&temp), *ptr2(0);
  bool proceed = getSingleIndexResult(indexInfo, values, valueNo, temp1);
//   for (unsigned int i = 0; i < result.size(); i++) {
//     cout << result[i].size() << " ";
//   }
  set<int> tmp;
  while (proceed) {
    valueNo++;
    proceed = getSingleIndexResult(indexInfo, values, valueNo, temp2);
//     for (unsigned int i = 0; i < temp.size(); i++) {
//       cout << temp[i].size() << " ";
//     }
    switch (values.second) {
      case STANDARD:
      case INTERSECT: // unite intermediate results
        for (int i = 1; i <= rel->GetNoTuples(); i++) {
          temp1[i].insert(temp2[i].begin(), temp2[i].end());
        }
        break;
      case SUPERSET:
      case EQUAL: // intersect intermediate results
        for (int i = 1; i <= rel->GetNoTuples(); i++) {
          std::set_intersection(temp1[i].begin(), temp1[i].end(),
             temp2[i].begin(), temp2[i].end(), std::inserter(tmp, tmp.begin()));
          temp1[i] = tmp;
          tmp.clear();
        }
        break;
      default: { // case DISJOINT: ignore
        break;
      }
    }
    temp2.clear();
    temp2.resize(rel->GetNoTuples() + 1);
  }
  for (int i = 1; i <= rel->GetNoTuples(); i++) {
    unitsToPeriods(temp1[i], i, indexInfo.first, result[i]);
  }
}

/*
\subsection{Function ~getResultForAtomTime~}

*/
bool TMatchIndexLI::getResultForAtomTime(const int atomNo,
                                         vector<Periods*> &result) {
  PatElem atom;
  p->getElem(atomNo, atom);
  set<string> ivs;
  atom.getI(ivs);
  if (ivs.empty()) {
    return false;
  }
  SecInterval ivInst(true);
  CcReal start(true, 0);
  CcReal end(true, 0);
  vector<set<int> > temp1, temp2;
  temp1.resize(rel->GetNoTuples() + 1);
  temp2.resize(rel->GetNoTuples() + 1);
  set<int> tmp;
  bool first = true;
  for (set<string>::iterator it = ivs.begin(); it != ivs.end(); it++) {
    Tools::stringToInterval(*it, ivInst);
    start.Set(true, ivInst.start.ToDouble());
    end.Set(true, ivInst.end.ToDouble());
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
  }
  for (int i = 1; i <= rel->GetNoTuples(); i++) {
    unitsToPeriods(temp1[i], i, attrNo, result[i]);
  }
//   for (unsigned int i = 0; i < result.size(); i++) {
//     cout << "|" << i << ": " << result[i].size() << " ";
//   }
//   cout << endl;
  return true;
}

/*
\subsection{Function ~storeIndexResult~}

*/
void TMatchIndexLI::storeIndexResult(int atomNo) {
  PatElem atom;
  p->getElem(atomNo, atom);
  map<int, pair<IndexType, int> >::iterator it;
  int noRelevantAttrs = 0;
  for (it = ti->attrToIndex.begin(); it != ti->attrToIndex.end(); it++) {
    if (it->second.first != NONE) {
      noRelevantAttrs++;
    }
  }
  if (noRelevantAttrs != atom.getNoValues()) {
    cout << "Error: " << noRelevantAttrs << " != " << atom.getNoValues()<< endl;
    return;
  }
  vector<set<int> > result;
  vector<Periods*> periods, temp;
  result.resize(rel->GetNoTuples() + 1);
  periods.resize(rel->GetNoTuples() + 1);
  temp.resize(rel->GetNoTuples() + 1);
  int pos(0), pred(0);
  bool intersect = getResultForAtomTime(atomNo, periods);
  Periods tmp(0);
  indexResult[atomNo].resize(rel->GetNoTuples() + 1);
//   for (it = ti->attrToIndex.begin(); it != ti->attrToIndex.end(); it++) {
//     cout << it->first << " ---> " << it->second.second << endl;
//   }
  for (it = ti->attrToIndex.begin(); it != ti->attrToIndex.end(); it++) {
    if (!intersect && it->second.second != -1 && 
        atom.values[pos].first.addr != 0) {
      getResultForAtomPart(*it, atom.values[pos], periods);
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
        getResultForAtomPart(*it, atom.values[pos], temp);
        for (int i = 1; i <= rel->GetNoTuples(); i++) {
          periods[i]->Intersection(*temp[i], tmp);
//           std::set_intersection(result[i].begin(), result[i].end(),
//             temp[i].begin(), temp[i].end(), std::inserter(tmp, tmp.begin()));
	  periods[i]->Clear();
          periods[i]->CopyFrom(&tmp);
          tmp.Clear();
  //      cout << "Still " << result[i].size() << " elems for id " << i << endl;
        }
        temp.clear();
        temp.resize(rel->GetNoTuples() + 1);
      }
    }
    if (it->second.first != NONE) {
      pos++;
    }
  } // index information collected into result
  for (int i = 1; i <= rel->GetNoTuples(); i++) {
    periodsToUnits(periods[i], i, result[i]);
    if (periods[i]) {
      periods[i]->DeleteIfAllowed();
    }
    if (!result[i].empty()) {
      indexResult[atomNo][pred].succ = i; // refresh successor of predecessor
      indexResult[atomNo][i].units.insert(result[i].begin(), result[i].end());
      indexResult[atomNo][i].pred = pred;
      pred = i;
    }
  }
  indexResult[atomNo][pred].succ = 0; // set successor to 0 for final id
}

/*
\subsection{Function ~initMatchInfo~}

*/
void TMatchIndexLI::initMatchInfo() {
  PatElem atom;
  for (set<int>::iterator it = crucialAtoms.begin(); it != crucialAtoms.end();
       it++) {
//     cout << "removed ids ";
    p->getElem(*it, atom);
    TupleId oldId = 0;
    if (atom.isRelevantForTupleIndex()) {
      TupleId id = indexResult[*it][0].succ; // first active tuple id
      if (id == 0) { // no index result
        cout << "no index result for crucial atom, EXIT " << *it << endl;
        return;
      }
      while (id > 0) {
        for (TupleId i = oldId + 1; i < id; i++) {
          removeIdFromIndexResult(i);
          active[i] = false;
//           cout << i << ",";
        }
        oldId = id;
        id = indexResult[*it][id].succ;
      }
      for (int i = oldId + 1; i <= rel->GetNoTuples(); i++) {
        removeIdFromIndexResult(i);
        active[i] = false;
//         cout << i << ",";
      }
    }
//     cout << " because of crucial atom " << *it << endl;
  }
  matchInfo.resize(p->getNFAsize());
  matchInfoPtr = &matchInfo;
  newMatchInfo.resize(p->getNFAsize());
  newMatchInfoPtr = &newMatchInfo;
  for (int i = 0; i < p->getNFAsize(); i++) {
    (*matchInfoPtr)[i].resize(rel->GetNoTuples() + 1);
    (*newMatchInfoPtr)[i].resize(rel->GetNoTuples() + 1);
  }
  DataType type = Tools::getDataType(rel->GetTupleType(), attrNo);
  unsigned int pred = 0;
//   cout << "pushed back imi for ids ";
  for (TupleId id = 1; id <= (TupleId)rel->GetNoTuples(); id++) {
    if (active[id]) {
      activeTuples++;
      IndexMatchInfo imi(false);
      (*matchInfoPtr)[0][id].imis.push_back(imi);
      (*newMatchInfoPtr)[0][id].imis.push_back(imi);
      for (int s = 0; s < p->getNFAsize(); s++) {
        (*matchInfoPtr)[s][id].pred = pred;
        (*matchInfoPtr)[s][pred].succ = id;
        (*newMatchInfoPtr)[s][id].pred = pred;
        (*newMatchInfoPtr)[s][pred].succ = id;
      }
//       cout << id << ",";
      pred = id;
      trajSize[id] = getTrajSize(id, type);
    }
  }
//   cout << endl;
  for (int s = 0; s < p->getNFAsize(); s++) {
    (*matchInfoPtr)[s][pred].succ = 0; // set succ of last active tid to 0
    (*newMatchInfoPtr)[s][pred].succ = 0;
  }
}

/*
\subsection{Function ~removeIdFromMatchInfo~}

*/
void TMatchIndexLI::removeIdFromMatchInfo(const TupleId id) {
//   cout << "remove tuple " << id << endl;
  bool removed = false;
  for (int s = 0; s < p->getNFAsize(); s++) {
    if ((*newMatchInfoPtr)[s].size() > 0) {
//         cout << "Elem " << i << ", Tuple " << id << ":" << endl;
      (*newMatchInfoPtr)[s][(*newMatchInfoPtr)[s][id].pred].succ = 
           (*newMatchInfoPtr)[s][id].succ;
//         cout << "   succ of " << matchInfo[i][id].pred << " set to " 
//              << matchInfo[i][id].succ << endl;
      (*newMatchInfoPtr)[s][(*newMatchInfoPtr)[s][id].succ].pred = 
           (*newMatchInfoPtr)[s][id].pred;
//         cout << "   pred of " << matchInfo[i][id].succ << " set to " 
//              << matchInfo[i][id].pred << endl;
      (*newMatchInfoPtr)[s][id].imis.clear();
    }
    if ((*matchInfoPtr)[s].size() > 0) {
      if ((*matchInfoPtr)[s][(*matchInfoPtr)[s][id].pred].succ == id) {
//         cout << "Elem " << i << ", Tuple " << id << ":" << endl;
        (*matchInfoPtr)[s][(*matchInfoPtr)[s][id].pred].succ = 
             (*matchInfoPtr)[s][id].succ;
//         cout << "   succ of " << matchInfo[i][id].pred << " set to " 
//              << matchInfo[i][id].succ << endl;
        (*matchInfoPtr)[s][(*matchInfoPtr)[s][id].succ].pred = 
             (*matchInfoPtr)[s][id].pred;
        removed = true;
//         cout << "   pred of " << matchInfo[i][id].succ << " set to " 
//              << matchInfo[i][id].pred << endl;
        (*matchInfoPtr)[s][id].imis.clear();
      }
    }
  }
  active[id] = false;
  activeTuples -= (removed ? 1 : 0);
//   cout << "ID " << id << " removed; activeTuples=" << activeTuples << endl;
}

/*
\subsection{Function ~atomMatch~}

*/
bool TMatchIndexLI::atomMatch(int state, pair<int, int> trans) {
//   cout << "atomMatch(" << state << ", " << trans.first << ", " 
//        << trans.second << ") called" << endl
  PatElem atom;
  set<string> ivs;
  atom.getI(ivs);
  SecInterval iv;
  vector<pair<int, string> > relevantAttrs;
  p->getElem(trans.first, atom);
  bool transition = false;
  IndexMatchInfo *imiPtr = 0;
  if (atom.isRelevantForTupleIndex()) {
    TupleId id = indexResult[trans.first][0].succ;
//     cout << "first id for atom " << trans.first << " is " << id << " with "
//          << indexResult[trans.first][id].units.size() << " units" << endl;
    while (id > 0) {
      bool totalMatch = false;
      set<int>::reverse_iterator it = 
                                    indexResult[trans.first][id].units.rbegin();
      while (active[id] && it != indexResult[trans.first][id].units.rend()) {
        unsigned int numOfIMIs = (*matchInfoPtr)[state][id].imis.size();
//         cout << "  while loop: consider unit " << *it << " and " << numOfIMIs
//              << " imis" << endl;
        for (unsigned int i = 0; i < numOfIMIs; i++) {
          Tuple *t = rel->GetTuple(id, false);
          TMatch tmatch(p, t, ttList, attrNo, relevantAttrs, valueNo);
          imiPtr = &(*matchInfoPtr)[state][id].imis[i];
          bool ok = imiPtr->range ? imiPtr->next <= *it : imiPtr->next == *it;
          if (ok) {
            if (tmatch.valuesMatch(*it, trans.first) &&
                Tools::timesMatch(iv, ivs) &&
                tmatch.easyCondsMatch(*it, trans.first)) {
              transition = true;
              IndexMatchInfo newIMI(false, *it + 1, imiPtr->binding,
                                    imiPtr->prevElem);
              if (p->hasConds()) { // extend binding for a complete match
                extendBinding(newIMI, trans.first);
                if (p->isFinalState(trans.second) && 
                    newIMI.finished(trajSize[id])) {
                  string var = p->getVarFromElem
                                              (p->getElemFromAtom(trans.first));
                  int oldEnd = newIMI.binding[var].second;
                  newIMI.binding[var].second = trajSize[id] - 1;
                  TMatch tmatch(p, t, ttList, attrNo, relevantAttrs, valueNo);
                  totalMatch = tmatch.conditionsMatch(newIMI.binding);
                  if (!totalMatch) { // reset unsuccessful binding
                    newIMI.binding[var].second = oldEnd;
                    newIMI.prevElem = imiPtr->prevElem;
                  }
                }
              }
              else {
                totalMatch = p->isFinalState(trans.second) &&
                             newIMI.finished(trajSize[id]);
              }
              if (totalMatch) {
                matches.push_back(id); // complete match
                removeIdFromMatchInfo(id);
                removeIdFromIndexResult(id);
    //         cout << id << " removed (wild match) " << activeTuples 
    //              << " active tuples" << endl;
                i = UINT_MAX - 1; // stop processing this tuple id
              }
              else if (!newIMI.exhausted(trajSize[id])) { // continue
                (*newMatchInfoPtr)[trans.second][id].imis.push_back(newIMI);
              }
            }
            unitCtr = *it; //set counter to minimum of index results
          }
          t->DeleteIfAllowed();
        }
        it++;
      }
      id = indexResult[trans.first][id].succ;
    }
  }
  else { // consider all existing imi instances
//     cout << "not relevant for index; consider all instances; " 
//          << (*matchInfoPtr)[state][0].succ << " " 
//          <<  (*newMatchInfoPtr)[state][0].succ << endl;
    TupleId id = (*matchInfoPtr)[state][0].succ; // first active tuple id
    while (id > 0) {
  //      cout << ": " << (*matchInfoPtr)[state][id].imis.size() 
  //           << " IMIs, state=" << state << ", id=" << id << endl;
      unsigned int numOfIMIs = (*matchInfoPtr)[state][id].imis.size();
      unsigned int i = 0;
      while (active[id] && i < numOfIMIs) {
        bool totalMatch = false;
        imiPtr = &((*matchInfoPtr)[state][id].imis[i]);
        if (imiPtr->next + 1 >= unitCtr) {
          Tuple *t = rel->GetTuple(id, false);
          if (atom.getW() == NO) { // () or disjoint{...} or (monday _); no wc
            TMatch tmatch(p, t, ttList, attrNo, relevantAttrs, valueNo);
            if (imiPtr->range == false) { // exact matching required
              if (!ivs.empty()) {
                getInterval(id, imiPtr->next, iv);
              }
              if (tmatch.valuesMatch(imiPtr->next, trans.first) &&
                  Tools::timesMatch(iv, ivs) &&
                  tmatch.easyCondsMatch(imiPtr->next, trans.first)) {
                transition = true;
                IndexMatchInfo newIMI(false, imiPtr->next + 1, imiPtr->binding,
                                      imiPtr->prevElem);
                if (p->hasConds()) { // extend binding for a complete match
                  extendBinding(newIMI, trans.first);
                  if (p->isFinalState(trans.second) && 
                      newIMI.finished(trajSize[id])) {
                    string var = p->getVarFromElem
                                              (p->getElemFromAtom(trans.first));
                    int oldEnd = newIMI.binding[var].second;
                    newIMI.binding[var].second = trajSize[id] - 1;
                    TMatch tmatch(p, t, ttList, attrNo, relevantAttrs, valueNo);
                    totalMatch = tmatch.conditionsMatch(newIMI.binding);
                    if (!totalMatch) { // reset unsuccessful binding
                      newIMI.binding[var].second = oldEnd;
                      newIMI.prevElem = imiPtr->prevElem;
                    }
                  }
                }
                else {
                  totalMatch = p->isFinalState(trans.second) &&
                               newIMI.finished(trajSize[id]);
                }
                if (totalMatch) {
                  matches.push_back(id); // complete match
                  removeIdFromMatchInfo(id);
                  removeIdFromIndexResult(id);
      //         cout << id << " removed (wild match) " << activeTuples 
      //              << " active tuples" << endl;
                  i = UINT_MAX - 1; // stop processing this tuple id
                }
                else if (!newIMI.exhausted(trajSize[id])) { // continue
                  (*newMatchInfoPtr)[trans.second][id].imis.push_back(newIMI);
                }
              }
            }
            else { // wildcard before this transition; check all successors
              int k = imiPtr->next;
              while (k < trajSize[id]) {
                getInterval(id, k, iv);
                if (tmatch.valuesMatch(k, trans.first) &&
                    Tools::timesMatch(iv, ivs) &&
                    tmatch.easyCondsMatch(k, trans.first)) {
                  transition = true;
                  IndexMatchInfo newIMI(false, k + 1, imiPtr->binding,
                                        imiPtr->prevElem);
                  if (p->hasConds()) { // extend binding for a complete match
                    extendBinding(newIMI, trans.first);
                    if (p->isFinalState(trans.second) && 
                        newIMI.finished(trajSize[id])) {
                      string var = p->getVarFromElem
                                              (p->getElemFromAtom(trans.first));
                      int oldEnd = newIMI.binding[var].second;
                      newIMI.binding[var].second = trajSize[id] - 1;
                      TMatch tmatch(p, t, ttList, attrNo, relevantAttrs, 
                                    valueNo);
                      totalMatch = tmatch.conditionsMatch(newIMI.binding);
                      if (!totalMatch) { // reset unsuccessful binding
                        newIMI.binding[var].second = oldEnd;
                        newIMI.prevElem = imiPtr->prevElem;
                      }
                    }
                  }
                  totalMatch = p->isFinalState(trans.second) &&
                               newIMI.finished(trajSize[id]);
                  if (totalMatch) {
                    matches.push_back(id); // complete match
                    removeIdFromMatchInfo(id);
                    removeIdFromIndexResult(id);
      //         cout << id << " removed (wild match) " << activeTuples 
      //              << " active tuples" << endl;
                    k = trajSize[id];
                    i = UINT_MAX - 1; // stop processing this tuple id
                  }
                  else if (!newIMI.exhausted(trajSize[id])) { // continue
                    (*newMatchInfoPtr)[trans.second][id].imis.push_back(newIMI);
                  }
                }
                k++;
              }
            }
          }
          else { // wildcard
            transition = true;
            IndexMatchInfo newIMI(true, imiPtr->next + 1, imiPtr->binding,
                                  imiPtr->prevElem);
            if (p->hasConds()) { // extend binding for a complete match
              extendBinding(newIMI, trans.first);
              if (p->isFinalState(trans.second) && 
                  newIMI.finished(trajSize[id])) {
                string var = p->getVarFromElem(p->getElemFromAtom(trans.first));
                int oldEnd = newIMI.binding[var].second;
                newIMI.binding[var].second = trajSize[id] - 1;
                TMatch tmatch(p, t, ttList, attrNo, relevantAttrs, valueNo);
                totalMatch = tmatch.conditionsMatch(newIMI.binding);
                if (!totalMatch) { // reset unsuccessful binding
                  newIMI.binding[var].second = oldEnd;
                  newIMI.prevElem = imiPtr->prevElem;
                }
              }
            }
            else { // no conditions
              totalMatch = p->isFinalState(trans.second) &&
                           newIMI.finished(trajSize[id]);
            }
            if (totalMatch) {
              matches.push_back(id); // complete match
//            cout << "complete match for id " << id << ", pushed into matches";
              removeIdFromMatchInfo(id);
              removeIdFromIndexResult(id);
      //         cout << id << " removed (wild match) " << activeTuples 
      //              << " active tuples" << endl;
              i = UINT_MAX - 1; // stop processing this tuple id
            }
            else if (!newIMI.exhausted(trajSize[id])) { // continue
              (*newMatchInfoPtr)[trans.second][id].imis.push_back(newIMI);
//               cout << "  imi pushed back from wc, id " << id << ", state " 
//                    << trans.second << ", next " << newIMI.next << endl;
            }
          }
          t->DeleteIfAllowed();
        }
        i++;
      }
      if (active[id]) {
        if (!hasIdIMIs(id, state)) { // no IMIs for id and state
          if (!hasIdIMIs(id)) { // no IMIs at all for id
            removeIdFromMatchInfo(id);
            removeIdFromIndexResult(id);
    //         cout << id << " removed (wild mismatch) " << activeTuples 
    //              << " active tuples" << endl;
          }
        }
      }
      id = (*matchInfoPtr)[state][id].succ;
    }
  }
//   cout << "return " << (transition ? "TRUE" : "FALSE") << endl;
  return transition;
}

/*
\subsection{Function ~applyNFA~}

*/
void TMatchIndexLI::applyNFA() {
  set<int> states, newStates;
  states.insert(0);
  set<int>::reverse_iterator is;
  map<int, int>::reverse_iterator im;
  while ((activeTuples > 0) && !states.empty()) {
//     cout << "WHILE loop: activeTuples=" << activeTuples << "; " 
//          << matches.size() << " matches; states:";
    for (is = states.rbegin(); is != states.rend(); is++) {
      map<int, int> trans = p->getTransitions(*is);
      for (im = trans.rbegin(); im != trans.rend() && activeTuples > 0; im++) {
    // TODO: invoke atomMatch iff newStates.find(im->second) != newStates.end()
        if (atomMatch(*is, *im)) {
          newStates.insert(im->second);
        }
      }
    }
    states.clear();
    states.swap(newStates);
//     cout << "Current states: ";
//     for (set<int>::iterator it = states.begin(); it != states.end(); it++) {
//       cout << *it << ",";
//     }
//     cout << "   " << !newStates.empty() << endl;
    clearMatchInfo();
    unitCtr++;
//     cout << "unitCtr set to " << unitCtr << endl;
  }
}

/*
\subsection{Function ~initialize~}

*/
bool TMatchIndexLI::initialize() {
  if (!tiCompatibleToRel()) {
    cout << "Error: Tuple index is not compatible with relation" << endl;
    return false;
  }
  vector<map<int, int> > simpleNFA;
  p->simplifyNFA(simpleNFA);
  set<pair<set<int>, int> > paths;
  p->findNFApaths(simpleNFA, paths);
  set<int> cruElems;
  p->getCrucialElems(paths, crucialAtoms);
  indexResult.resize(p->getSize());
  active.resize(rel->GetNoTuples() + 1, true);
  matches.push_back(1);
  trajSize.resize(rel->GetNoTuples() + 1, 0);
  for (set<int>::iterator it = crucialAtoms.begin(); it != crucialAtoms.end(); 
       it++) {
    storeIndexResult(*it);
  }
  Tuple *firstTuple = rel->GetTuple(1, false);
  if (!p->initEasyCondOpTrees(firstTuple, ttList)) {
    return false;
  }
  if (!p->initCondOpTrees(firstTuple, ttList)) {
    return false;
  }
  firstTuple->DeleteIfAllowed();
  initMatchInfo();
  applyNFA();
  return true;
}

/*
\subsection{Function ~nextTuple~}

*/
Tuple* TMatchIndexLI::nextTuple() {
  if (matches[0] == 0 || matches[0] >= matches.size() || matches.size() <= 1) {
    return 0;
  }
  Tuple *result = rel->GetTuple(matches[matches[0]], false);
  matches[0]++;
  return result;
}

/*
\subsection{Function ~deletePattern~}

*/
void TMatchIndexLI::deletePattern() {
  int majorAttrNo;
  vector<pair<int, string> > relevantAttrs = Tools::getRelevantAttrs(
                         rel->GetTupleType(), attrNo, majorAttrNo);
  p->deleteAtomValues(relevantAttrs);
  delete p;
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
 R_Tree<1, NewPair<TupleId, int> > *rt, int _attrNr, Pattern *_p, DataType _t)
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
          strAttr = Pattern::getPointer(right[i][j].second);
        }
        pointers[i].push_back(strAttr.second);
        toReplace = right[i][j].first + Condition::getType(right[i][j].second);
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
  R_Tree<1, NewPair<TupleId, int> > *rt, Word _classifier, int _attrNr, 
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
  wordPosType wc;
  charPosType cc;
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
  R_TreeLeafEntry<1, NewPair<TupleId, int> > leaf;
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
                                      leaf.info.second);
    }
    while (rtree->Next(leaf)) {
      if ((init || !oldPart[leaf.info.first].empty()) &&
          !deactivated[leaf.info.first]) {
        newPart[leaf.info.first].insert(newPart[leaf.info.first].end(),
                                        leaf.info.second);
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
    if ((*newMatchInfoPtr)[s].size() > 0) {
//         cout << "Elem " << i << ", Tuple " << id << ":" << endl;
      (*newMatchInfoPtr)[s][(*newMatchInfoPtr)[s][id].pred].succ = 
           (*newMatchInfoPtr)[s][id].succ;
//         cout << "   succ of " << matchInfo[i][id].pred << " set to " 
//              << matchInfo[i][id].succ << endl;
      (*newMatchInfoPtr)[s][(*newMatchInfoPtr)[s][id].succ].pred = 
           (*newMatchInfoPtr)[s][id].pred;
//         cout << "   pred of " << matchInfo[i][id].succ << " set to " 
//              << matchInfo[i][id].pred << endl;
//       (*newMatchInfoPtr)[s][id].imis.clear();
    }
    if ((*matchInfoPtr)[s].size() > 0) {
      if ((*matchInfoPtr)[s][(*matchInfoPtr)[s][id].pred].succ == id) {
//         cout << "Elem " << i << ", Tuple " << id << ":" << endl;
        (*matchInfoPtr)[s][(*matchInfoPtr)[s][id].pred].succ = 
             (*matchInfoPtr)[s][id].succ;
//         cout << "   succ of " << matchInfo[i][id].pred << " set to " 
//              << matchInfo[i][id].succ << endl;
        (*matchInfoPtr)[s][(*matchInfoPtr)[s][id].succ].pred = 
             (*matchInfoPtr)[s][id].pred;
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
  if (!indexResult[e].empty()) {
    return;
  }
  PatElem elem;
  p->getElem(e, elem);
  if (!elem.hasIndexableContents()) {
    return;
  }
  set<string> ivs, lbs;
  set<pair<string, unsigned int> > pls;
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
      elem.getP(pls);
      set<pair<string, unsigned int> >::iterator ip = pls.begin();
      if (!pls.empty()) {
        part.resize(rel->GetNoTuples() + 1);
        part2.resize(rel->GetNoTuples() + 1);
        retrieveValue(part, part2, elem.getSetRel(), true, 
                      ip->first, ip->second);
        ip++;
      }
      while (ip != pls.end()) {
        retrieveValue(part, part2, elem.getSetRel(), false,
                      ip->first, ip->second);
        ip++;
      }
    }  
  }
  if (!part.empty() || (elem.getSetRel() == DISJOINT) ||
      (!elem.hasLabel() && !elem.hasPlace())) { // continue with time intervals
    elem.getI(ivs);
    set<string>::iterator is = ivs.begin();
    while (is != ivs.end()) {
      if (Tools::isInterval(*is)) {
        retrieveTime(part, part2, *is);
      }
      is++;
    }
  }
  indexResult[e].resize(rel->GetNoTuples() + 1);
  unsigned int pred = 0;
  IndexRetrieval ir(pred);
  indexResult[e][0] = ir; // first entry points to first active entry
  for (unsigned int i = 1; i < part.size(); i++) { // collect results
    if (!part[i].empty()) {
      indexResult[e][pred].succ = i; // update successor of predecessor
      IndexRetrieval ir(pred, 0, part[i]);
      indexResult[e][i] = ir;
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
  if (indexResult[e].empty() && elem.hasIndexableContents() &&
      (elem.getSetRel() != DISJOINT)) {
    indexMismatch.insert(e);
  }
}

/*
\subsection{Function ~initMatchInfo~}

*/
void IndexMatchesLI::initMatchInfo(const set<int>& cruElems) {
  matchInfo.clear();
  matchInfo.resize(p->getNFAsize());
  matchInfoPtr = &matchInfo;
  newMatchInfo.clear();
  newMatchInfo.resize(p->getNFAsize());
  newMatchInfoPtr = &newMatchInfo;
  trajSize.resize(rel->GetNoTuples() + 1, 0);
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
      TupleId id = indexResult[*it][0].succ; // first active tuple id
      while (id > 0) {
        if (trajInfo[id]) {
          newTrajInfo[id] = true;
        }
        else {
          removeIdFromIndexResult(id);
        }
        id = indexResult[*it][id].succ;
      }
      trajInfo.swap(newTrajInfo);
    }
  }
  for (int i = 0; i < p->getNFAsize(); i++) {
    (*matchInfoPtr)[i].resize(rel->GetNoTuples() + 1);
    (*newMatchInfoPtr)[i].resize(rel->GetNoTuples() + 1);
  }
  unsigned int pred = 0;
  for (unsigned int id = 1; id < trajInfo.size(); id++) {
    if (trajInfo[id]) {
      trajSize[id] = getTrajSize(id, mtype);
      IndexMatchInfo imi(false);
      (*matchInfoPtr)[0][id].imis.push_back(imi);
      deactivated[id] = false;
      for (int s = 0; s < p->getNFAsize(); s++) {
        (*matchInfoPtr)[s][pred].succ = id;
        (*matchInfoPtr)[s][id].pred = pred;
        (*newMatchInfoPtr)[s][pred].succ = id;
        (*newMatchInfoPtr)[s][id].pred = pred;
        if ((int)indexResult[s].size() == rel->GetNoTuples() + 1) {
          for (unsigned int i = pred + 1; i < id; i++) { // deactivate indexRes.
            indexResult[s][i].pred = 0;
            indexResult[s][i].succ = 0;
          }
          indexResult[s][pred].succ = id;
        }
      }
      activeTuples++;
      pred = id;
    }
  }
  (*matchInfoPtr)[0][pred].succ = 0;
  (*newMatchInfoPtr)[0][pred].succ = 0;
  for (int s = 0; s < p->getSize(); s++) {
    if ((int)indexResult[s].size() == rel->GetNoTuples() + 1) {
      indexResult[s][pred].succ = 0;
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
  indexResult.resize(p->getSize());
  deactivated.resize(rel->GetNoTuples() + 1, false);
  for (set<int>::iterator it = cruElems.begin(); it != cruElems.end(); it++) {
    cout << "storeIndexResult(" << *it << ")" << endl;
    storeIndexResult(*it);
  }
  deactivated.resize(rel->GetNoTuples() + 1, true);
  p->initEasyCondOpTrees();
  p->initCondOpTrees();
  matchInfoPtr = &matchInfo;
  newMatchInfoPtr = &newMatchInfo;
  activeTuples = 0;
  initMatchInfo(cruElems);
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
  if (!per->IsDefined()) {
    cout << "undefined periods!" << endl;
    return;
  }
  Interval<Instant> iv;
  Tuple *t = rel->GetTuple(tId, false);
  int start, end;
  switch (mtype) {
    case MLABEL: {
      MLabel *traj = (MLabel*)t->GetAttribute(attrNo);
      for (int i = 0; i < per->GetNoComponents(); i++) {
        per->Get(i, iv);
        start = traj->Position(iv.start);
        end = traj->Position(iv.end);
        for (int j = start; j <= end; j++) {
          units.insert(j);
        }
      }
      traj->DeleteIfAllowed();
      break;
    }
    case MLABELS: {
      MLabels *traj = (MLabels*)t->GetAttribute(attrNo);
      for (int i = 0; i < per->GetNoComponents(); i++) {
        per->Get(i, iv);
        start = traj->Position(iv.start);
        end = traj->Position(iv.end);
        for (int j = start; j <= end; j++) {
          units.insert(j);
        }
      }
      traj->DeleteIfAllowed();
      break;
    }
    case MPLACE: {
      MPlace *traj = (MPlace*)t->GetAttribute(attrNo);
      for (int i = 0; i < per->GetNoComponents(); i++) {
        per->Get(i, iv);
        start = traj->Position(iv.start);
        end = traj->Position(iv.end);
        for (int j = start; j <= end; j++) {
          units.insert(j);
        }
      }
      traj->DeleteIfAllowed();
      break;
    }
    case MPLACES: {
      MPlaces *traj = (MPlaces*)t->GetAttribute(attrNo);
      for (int i = 0; i < per->GetNoComponents(); i++) {
        per->Get(i, iv);
        start = traj->Position(iv.start);
        end = traj->Position(iv.end);
        for (int j = start; j <= end; j++) {
          units.insert(j);
        }
      }
      traj->DeleteIfAllowed();
      break;
    }
  }
  t->DeleteIfAllowed();
}

/*
\subsection{Function ~unitsToPeriods~}

*/
void IndexMatchSuper::unitsToPeriods(const set<int> &units, const TupleId tId, 
                                     const int attr, Periods *per) {
  if (!per) {
    per = new Periods(0);
  }
  per->SetDefined(true);
  per->Clear();
  Tuple *t = rel->GetTuple(tId, false);
  Attribute *traj = t->GetAttribute(attr);
  TupleType *tt = t->GetTupleType();
  string type = Tools::getTypeName(tt, attr);
  if (type == "mlabel") {
    unitsToPeriods<MLabel, ULabel>(traj, units, per);
  }
  if (type == "mlabels") {
    unitsToPeriods<MLabels, ULabels>(traj, units, per);
  }
  if (type == "mplace") {
    unitsToPeriods<MPlace, UPlace>(traj, units, per);
  }
  if (type == "mplaces") {
    unitsToPeriods<MPlaces, UPlaces>(traj, units, per);
  }
  if (type == "mpoint") {
    unitsToPeriods<MPoint, UPoint>(traj, units, per);
  }
  if (type == "mregion") {
    unitsToPeriods<MRegion, URegionEmb>(traj, units, per);
  }
  if (type == "mbool") {
    unitsToPeriods<MBool, UBool>(traj, units, per);
  }
  if (type == "mint") {
    unitsToPeriods<MInt, UInt>(traj, units, per);
  }
  if (type == "mreal") {
    unitsToPeriods<MReal, UReal>(traj, units, per);
  }
  if (type == "mstring") {
    unitsToPeriods<MString, UString>(traj, units, per);
  }
  tt->DeleteIfAllowed();
  t->DeleteIfAllowed();
}

/*
\subsection{Function ~removeIdFromIndexResult~}

*/
void IndexMatchSuper::removeIdFromIndexResult(const TupleId id) {
  for (int i = 0; i < p->getSize(); i++) {
    if (indexResult[i].size() > 0) {
      if (indexResult[i][indexResult[i][id].pred].succ == id) {
        indexResult[i][indexResult[i][id].pred].succ = indexResult[i][id].succ;
        indexResult[i][indexResult[i][id].succ].pred = indexResult[i][id].pred;
      }
    }
  }
}

/*
\subsection{Function ~clearMatchInfo~}

Copy the contents of newMatchInfo into matchInfo, and clear newMatchInfo.

*/
void IndexMatchSuper::clearMatchInfo() {
//   for (int i = 0; i < p->getNFAsize(); i++) {
// //     cout << "size of (*matchInfoPtr)[" << i << "] is " 
// //          << (*matchInfoPtr)[i].size() << endl;
//     TupleId id = (*matchInfoPtr)[i][0].succ;
//     while (id > 0) {
//       (*matchInfoPtr)[i][id].imis.clear();
//       id = (*matchInfoPtr)[i][id].succ;
//     }
//   }
//   vector<vector<IndexMatchSlot> >* temp = newMatchInfoPtr;
//   newMatchInfoPtr = matchInfoPtr;
//   matchInfoPtr = temp;
  
  
  for (int i = 0; i < p->getNFAsize(); i++) {
    TupleId id = (*newMatchInfoPtr)[i][0].succ;
    while (id > 0) {
      (*matchInfoPtr)[i][id].imis = (*newMatchInfoPtr)[i][id].imis;
      (*newMatchInfoPtr)[i][id].imis.clear();
      id = (*newMatchInfoPtr)[i][id].succ;
    }
  }  
//   cout << "after clearMatchInfo: " << (*matchInfoPtr)[0][0].succ << " "
//        << (*matchInfoPtr)[1][0].succ << " " << (*newMatchInfoPtr)[0][0].succ
//        << " " << (*newMatchInfoPtr)[1][0].succ << endl;
}

/*
\subsection{Function ~hasIdIMIs~}

*/
bool IndexMatchSuper::hasIdIMIs(const TupleId id, const int state /* = -1 */) {
  if (state == -1) { // check all states
    for (int s = 0; s < p->getNFAsize(); s++) {
      if (!(*matchInfoPtr)[s][id].imis.empty() || 
          !(*newMatchInfoPtr)[s][id].imis.empty()) {
        return true;
      }
    }
    return false;
  }
  else {
    return !(*matchInfoPtr)[state][id].imis.empty();
  }
}

/*
\subsection{Function ~extendBinding~}

*/
void IndexMatchSuper::extendBinding(IndexMatchInfo& imi, const int e) {
  if (!p->hasConds()) {
    return;
  }
  PatElem elem, prevElem;
  string var, prevVar;
  p->getElem(e, elem);
  if (imi.prevElem > -1) {
    p->getElem(imi.prevElem, prevElem);
    prevElem.getV(prevVar);
  }
  elem.getV(var);
//   cout << "extendBinding called: " << imi.next << " " << imi.prevElem << " " 
//        << prevVar << " " << var << " " << e << " " 
//        << imi.binding[prevVar].second << " | ";
  if (e < imi.prevElem) { // possible for repeated regex
    if (var == prevVar) { // valid case
      if (var != "") { // X [() * ()]+
        imi.binding[var].second = imi.next - 1;
//         cout << "right limit of " << var << " is " << imi.next - 1 << endl;
      }
      else { // [() ()]+
//         cout << "no variable for atom " << e << " => no changes" << endl;
      }
    }
    else { // invalid case
//       cout << "ERROR: " << e << ", " << var << " after " << imi.prevElem
//            << ", " << prevVar << endl;
    }
  }
  else if (e == imi.prevElem) { // same elem (and same variable) as before
    if (var != "") { // X * or X [...]+
      imi.binding[var].second = imi.next - 1;
//       cout << "upper limit of " << var << " set to " << imi.next - 1 << endl;
    }
    else { // no variable
//       cout << "no variable for atom " << e << " ===> no changes" << endl;
    }
  }
  else { // different elems
    if (var == prevVar) { // X [() +]
      if (var != "") {
        imi.binding[var].second = imi.next - 1;
//         cout << "ceiling of " << var << " set to " << imi.next - 1 << endl;
      }
      else { // no variable
//         cout << "no variable for atom " << e << " ====>> no changes" << endl;
      }
    }
    else { // different variables
      if ((var != "") && (prevVar == "")) { // () X ()
        if (imi.next == 0) { // first atom
          imi.binding[var] = make_pair(0, 0);
//           cout << "new var " << var << " bound to 0" << endl;
        }
        else {
          imi.binding[var] = make_pair(imi.next - 1, imi.next - 1);
//           cout << "new var " << var << " bound to " << imi.next - 1 << endl;
        }
      }
      else if (var == "") { // X () ()
        if (prevElem.getW() != NO) {
          imi.binding[prevVar].second = imi.next - 2;
//           cout << "prevVar " << prevVar << " finishes at " << imi.next - 2
//                << endl;
        }
      }
      else { // X ... Y ...
        if (elem.getW() == NO) { // X * Y () or X () Y ()
          imi.binding[prevVar].second = imi.next - 2;
//           cout << "prevVar " << prevVar << " extended to " << imi.next - 2
//                << endl;
          imi.binding[var] = make_pair(imi.next - 1, imi.next - 1);
        }
        else { // wildcard
          if (prevElem.getW() != NO) { // X * Y *
            imi.binding[prevVar].second = imi.next - 2;
//             cout << "prev " << prevVar << " extended to " << imi.next - 2 
//                  << endl;
          }
          else { // X () Y *
            imi.binding[var] = make_pair(imi.binding[prevVar].second + 1, 
                                         imi.next - 1);
            imi.binding[prevVar].second = imi.binding[prevVar].first;
          }
        }
      }
    }
    if (prevVar != "" && prevElem.getW() != NO) {
      imi.binding[prevVar].second = imi.next - 2;
    }
  }
  imi.prevElem = e;
//   Tools::printBinding(imi.binding);
}

/*
\subsection{Function ~wildcardMatch~}

*/
bool IndexMatchesLI::wildcardMatch(const int state, pair<int, int> trans) {
  IndexMatchInfo *imiPtr;
  bool ok = false;
  TupleId id = (*matchInfoPtr)[state][0].succ; // first active tuple id
  while (id > 0) {
//      cout << "wildcardMatch: " << (*matchInfoPtr)[state][id].imis.size() 
//           << " IMIs, state=" << state << ", id=" << id << endl;
    unsigned int numOfIMIs = (*matchInfoPtr)[state][id].imis.size();
    unsigned int i = 0;
    while (!deactivated[id] && (i < numOfIMIs)) {
      imiPtr = &((*matchInfoPtr)[state][id].imis[i]);
//       cout << imiPtr->next + 1 << " | " << unitCtr << endl;
      if (imiPtr->next + 1 >= unitCtr) {
        bool match = false;
        IndexMatchInfo newIMI(true, imiPtr->next + 1, imiPtr->binding, 
                              imiPtr->prevElem);
        if (p->hasConds()) { // extend binding for a complete match
          extendBinding(newIMI, trans.first);
          if (p->isFinalState(trans.second) && imiPtr->finished(trajSize[id])) {
            string var = p->getVarFromElem(p->getElemFromAtom(trans.first));
            int oldEnd = newIMI.binding[var].second;
            newIMI.binding[var].second = trajSize[id] - 1;
            match = checkConditions(id, newIMI);
            if (!match) { // reset unsuccessful binding
              newIMI.binding[var].second = oldEnd;
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
          removeIdFromIndexResult(id);
  //         cout << id << " removed (wild match) " << activeTuples 
  //              << " active tuples" << endl;
          i = UINT_MAX - 1;
        }
        else if (!newIMI.exhausted(trajSize[id])) { // continue
          (*newMatchInfoPtr)[trans.second][id].imis.push_back(newIMI);
  //         cout << "   imi pushed back from wildcardMatch, id " << id << endl;
          ok = true;
        }
      }
      i++;
    }
    if (!ok && !hasIdIMIs(id, state)) { // no IMIs for id and state
      if (!hasIdIMIs(id)) { // no IMIs at all for id
        removeIdFromMatchInfo(id);
        removeIdFromIndexResult(id);
//         cout << id << " removed (wild mismatch) " << activeTuples 
//              << " active tuples" << endl;
      }
    }
    id = (*matchInfoPtr)[state][id].succ;
  }
  return ok;
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
  if (!indexResult[e].empty()) { // contents found in at least one index
    TupleId id = indexResult[e][0].succ;
    while (id > 0) {
      if (!deactivated[id]) {
  //       cout << "state " << state << "; elem " << e << "; next id is " << id 
  //            << endl;
  //       cout << "simpleMatch: " << (*matchInfoPtr)[state][id].imis.size()
  //            << " IMIs, state=" << state << ", id=" << id << endl;
        if ((indexResult[e][indexResult[e][id].pred].succ == id) &&
            !indexResult[e][id].units.empty()) {
          unsigned int numOfIMIs = (*matchInfoPtr)[state][id].imis.size();
          for (unsigned int i = 0; i < numOfIMIs; i++) {
            set<int>::iterator it = indexResult[e][id].units.begin();
            while (!deactivated[id] && (it != indexResult[e][id].units.end())) {
              if (valuesMatch(e, id, (*matchInfoPtr)[state][id].imis[i], 
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
        if (!hasIdIMIs(id, state)) { // no IMIs
          if (!hasIdIMIs(id)) { // no IMIs at all for id
            removeIdFromMatchInfo(id);
            removeIdFromIndexResult(id);
  //           cout << id << " removed (index) " << activeTuples 
  //                << " active tuples" << endl;
          }
        }
      }
      id = indexResult[e][id].succ;
    }
  }
  else { // no result from index
    if (indexMismatch.find(e) != indexMismatch.end()) { // mismatch
      return false;
    }
    else { // disjoint or () or only semantic time information
      PatElem elem;
      TupleId id = matchInfo[e][0].succ; // first active tuple id
      while (id > 0) {
        if (!deactivated[id]) {
          unsigned int numOfIMIs = (*matchInfoPtr)[state][id].imis.size();
          for (unsigned int i = 0; i < numOfIMIs; i++) {
            if (valuesMatch(e, id, (*matchInfoPtr)[state][id].imis[i], 
                newState, -1)) {
              transition = true;
              if (deactivated[id]) {
                i = numOfIMIs;
              }
            }
          }
          if (!hasIdIMIs(id, state)) { // no IMIs for id and state
            if (!hasIdIMIs(id)) { // no IMIs at all for id
              removeIdFromMatchInfo(id);
              removeIdFromIndexResult(id);
  //             cout << id << " removed (no index) " << activeTuples #
  //                  << " active tuples"  << endl;
            }
          }
        }
        id = matchInfo[e][id].succ;
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
      result = match.conditionsMatch(*(p->getConds()), imi.binding);
      break;
    }
    case MLABELS: {
      MLabels *mls = (MLabels*)(tuple->GetAttribute(attrNo));
      Match<MLabels> match(p, mls);
      result = match.conditionsMatch(*(p->getConds()), imi.binding);
      break;
    }
    case MPLACE: {
      MPlace *mp = (MPlace*)(tuple->GetAttribute(attrNo));
      Match<MPlace> match(p, mp);
      result = match.conditionsMatch(*(p->getConds()), imi.binding);
      break;
    }
    default: { // PLACES
      MPlaces *mps = (MPlaces*)(tuple->GetAttribute(attrNo));
      Match<MPlaces> match(p, mps);
      result = match.conditionsMatch(*(p->getConds()), imi.binding);
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
    Tools::printBinding(binding);
  }
}

}
