 
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

\subsection{Function ~create~}

Creates an MLabel of a certain size for testing purposes. The labels will be
contain only numbers between 1 and size[*]rate; rate being the number of
different labels divided by the size.

*/
void MLabel::createML(int size, bool text, double rate /* = 1.0 */) {
  if ((size > 0) && (rate > 0) && (rate <= 1)) {
    int max = size * rate;
    ULabel ul(1);
    DateTime start(instanttype);
    time_t t;
    time(&t);
    srand(((unsigned int)t) % 86400000);
    DateTime dur(0, rand(), durationtype); // duration
    start.Set(2014, 1, 1);
    Instant end(start);
    end.Add(&dur);
    SecInterval iv(start, end, true, false);
    if (text) {
      vector<string> trajectory;
      Tools::createTrajectory(size, trajectory);
      for (int i = 0; i < size; i++) {
        time_t tm;
        time(&tm);
        srand(((unsigned int)tm) % 86400000);
        DateTime dur(0, rand(), durationtype); // duration
        ul.constValue.Set(true, trajectory[i]);
        iv.Set(start, end, true, false);
        ul.timeInterval = iv;
        Add(ul);
        start.Add(&dur);
        end.Add(&dur);
      }
    }
    else {
      for (int i = 0; i < size; i++) {
        time_t tm;
        time(&tm);
        srand(((unsigned int)tm) % 86400000);
        DateTime dur(0, rand(), durationtype); // duration
        ul.constValue.Set(true, Tools::int2String(max - (i % max)));
        start.Add(&dur);
        end.Add(&dur);
        iv.Set(start, end, true, false);
        ul.timeInterval = iv;
        Add(ul);
      }
    }
    units.TrimToSize();
  }
  else {
    cout << "Invalid parameters for creation." << endl;
    Clear();
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
\subsection{Function ~getPattern~}

Calls the parser.

*/
Pattern* Pattern::getPattern(string input, bool classify) {
  if (input.find('\n') == string::npos) {
    input.append("\n");
  }
  const char *patternChar = input.c_str();
  return parseString(patternChar, classify);
}

bool Pattern::containsFinalState(set<int> &states) {
  for (set<int>::iterator i = states.begin(); i != states.end(); i++) {
    if (finalStates.count(*i)) {
      return true;
    }
  }
  return false;
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


string Condition::getType(int t) {
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
pair<string, Attribute*> Pattern::getPointer(int key) {
  pair<string, Attribute*> result;
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
  return result;
}

/*
\subsection{Function ~initCondOpTrees~}

For a pattern with conditions, an operator tree structure is prepared.

*/
bool Pattern::initCondOpTrees() {
  for (unsigned int i = 0; i < conds.size(); i++) { // opTrees for conditions
    if (!conds[i].initOpTree()) {
      cout << "Operator tree for condition " << i << " uninitialized" << endl;
      return false;
    }
  }
  return true;
}

bool Condition::initOpTree() {
  string q(""), part, toReplace("");
  pair<string, Attribute*> strAttr;
  vector<Attribute*> ptrs;
  if (!isTreeOk()) {
    q = "query " + text;
    for (unsigned int j = 0; j < varKeys.size(); j++) { // init pointers
      strAttr = Pattern::getPointer(getKey(j));
      ptrs.push_back(strAttr.second);
      toReplace = getVar(j) + getType(getKey(j));
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
bool Pattern::initEasyCondOpTrees() {
  string q(""), part, toReplace("");
  pair<string, Attribute*> strAttr;
  vector<Attribute*> ptrs;
  for (unsigned int i = 0; i < easyConds.size(); i++) {
    if (!easyConds[i].isTreeOk()) {
      q = "query " + easyConds[i].getText();
      for (int j = 0; j < easyConds[i].getVarKeysSize(); j++) { // init pointers
        strAttr = getPointer(easyConds[i].getKey(j));
        ptrs.push_back(strAttr.second);
        toReplace = easyConds[i].getVar(j)
                  + Condition::getType(easyConds[i].getKey(j));
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
\subsection{Destructor for class ~ClassifyLI~}

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
IndexMatchesLI::IndexMatchesLI(Relation *rel, InvertedFile *inv,
 R_Tree<1, NewPair<TupleId, int> > *rt, int _attrNr, Pattern *_p, bool deleteP, 
   DataType type):
 mRel(rel), counter(0), invFile(inv), rtree(rt), attrNr(_attrNr), mtype(type) {
  if (_p) {
    if (mRel->GetNoTuples() > 0) {
      p = *_p;
      initialize();
      applyNFA();
      p.deleteEasyCondOpTrees();
      p.deleteCondOpTrees();
      if (deleteP) {
        delete _p;
      }
    }
  }
}

/*
\subsection{Function ~nextResultTuple~}

*/
Tuple* IndexMatchesLI::nextTuple() {
  if (!matches.empty()) {
    TupleId result = matches.back();
    matches.pop_back();
    return mRel->GetTuple(result, false);
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
    counter++;
//     cout << "WHILE loop: activeTuples=" << activeTuples << "; " 
//          << matches.size() << " matches; states:";
    for (is = states.rbegin(); is != states.rend(); is++) {
      map<int, int> trans = p.getTransitions(*is);
      for (im = trans.rbegin(); im != trans.rend() && activeTuples > 0; im++) {
        p.getElem(im->first, elem);
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
      opTree.first->Destroy(opTree.second, true);
      delete opTree.first;
      for (unsigned int i = 0; i < pointers.size(); i++) {
        deleteIfAllowed(pointers[i]);
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
\subsection{Constructor for class ~IndexClassifyLI~}

This constructor is used for the operator ~indexclassify~.

*/
IndexClassifyLI::IndexClassifyLI(Relation *rel, InvertedFile *inv,
  R_Tree<1, NewPair<TupleId, int> > *rt, Word _classifier, int _attrNr, 
  DataType type) : 
         IndexMatchesLI::IndexMatchesLI(rel, inv, rt, _attrNr, 0, false, type),
         classifyTT(0), c(0) {
  if (mRel->GetNoTuples() > 0) {
    c = (Classifier*)_classifier.addr;
    for (int i = 0; i < c->getNumOfP(); i++) { // check patterns
      Pattern *p = Pattern::getPattern(c->getPatText(i));
      bool ok = false;
      if (p) {
        switch (type) {
          case LABEL: {
            ok = p->isValid("label");
            break;
          }
          case LABELS: {
            ok = p->isValid("labels");
            break;
          }
          case PLACE: {
            ok = p->isValid("place");
            break;
          }
          case PLACES: {
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
\subsection{Function ~clearMatchInfo~}

*/
void IndexMatchesLI::clearMatchInfo() {
  for (int i = 0; i < p.getNFAsize(); i++) {
//     cout << "size of (*matchInfoPtr)[" << i << "] is " 
//          << (*matchInfoPtr)[i].size() << endl;
    TupleId id = (*matchInfoPtr)[i][0].succ;
    while (id > 0) {
      (*matchInfoPtr)[i][id].imis.clear();
      id = (*matchInfoPtr)[i][id].succ;
    }
  }
  vector<vector<IndexMatchSlot> >* temp = newMatchInfoPtr;
  newMatchInfoPtr = matchInfoPtr;
  matchInfoPtr = temp;
}

/*
\subsection{Function ~getInterval~}

*/
void IndexMatchesLI::getInterval(const TupleId tId, const int pos, 
                                  SecInterval& iv) {
  Tuple *tuple = mRel->GetTuple(tId, false);
  switch (mtype) {
    case LABEL: {
      ((MLabel*)tuple->GetAttribute(attrNr))->GetInterval(pos, iv);
      break;
    }
    case LABELS: {
      ((MLabels*)tuple->GetAttribute(attrNr))->GetInterval(pos, iv);
      break;
    }
    case PLACE: {
      ((MPlace*)tuple->GetAttribute(attrNr))->GetInterval(pos, iv);
      break;
    }
    case PLACES: {
      ((MPlaces*)tuple->GetAttribute(attrNr))->GetInterval(pos, iv);
      break;
    }
    default: {
      break;
    }
  }
  tuple->DeleteIfAllowed();
}

/*
\subsection{Function ~simplifyNFA~}

*/
void IndexMatchesLI::simplifyNFA(vector<map<int, int> >& result) {
  result.clear();
  string ptext = p.GetText();
  for (unsigned int i = 1; i < ptext.length(); i++) {
    if ((ptext[i-1] == ']') && ((ptext[i] == '*') || (ptext[i] == '+'))) {
      ptext[i] = ' '; // eliminate repetition of regular expressions
    }
  }
  Pattern *pnew = Pattern::getPattern(ptext);
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
void IndexMatchesLI::findNFApaths(const vector<map<int, int> >& nfa, 
               const set<int>& finalStates, set<pair<set<int>, int> >& result) {
  result.clear();
  set<pair<set<int>, int> > newPaths;
  set<int> elems;
  set<pair<set<int>, int> >::iterator is;
  map<int, int>::iterator im;
  result.insert(make_pair(elems, 0));
  bool proceed = true;
  while (proceed) {
    for (is = result.begin(); is != result.end(); is++) {
      map<int, int> trans = nfa[is->second];
      for (im = trans.begin(); im != trans.end(); im++) {
        elems = is->first; // get old transition set
        elems.insert(im->first); // add new transition
        newPaths.insert(make_pair(elems, im->second)); // and new state
      }
      if (nfa[is->second].empty()) { // no transition available
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
void IndexMatchesLI::getCrucialElems(const set<pair<set<int>, int> >& paths,
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
  if ((mtype == LABEL) || (mtype == LABELS)) {
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
    oldPart.resize(mRel->GetNoTuples() + 1);
    newPart.resize(mRel->GetNoTuples() + 1);
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
\subsection{Function ~removeIdFromIndexResult~}

*/
void IndexMatchesLI::removeIdFromIndexResult(const TupleId id) {
  for (int i = 0; i < p.getSize(); i++) {
    if (indexResult[i].size() > 0) {
      if (!indexResult[i][id].units.empty() &&
          (indexResult[i][indexResult[i][id].pred].succ == id)) {
        indexResult[i][indexResult[i][id].pred].succ = indexResult[i][id].succ;
        indexResult[i][indexResult[i][id].succ].pred = indexResult[i][id].pred;
      }
    }
  }
}

/*
\subsection{Function ~removeIdFromMatchInfo~}

*/
void IndexMatchesLI::removeIdFromMatchInfo(const TupleId id) {
  bool removed = false;
  for (int s = 0; s < p.getNFAsize(); s++) {
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
  p.getElem(e, elem);
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
    if ((mtype == LABEL) || (mtype == LABELS)) {
      elem.getL(lbs);
      set<string>::iterator is = lbs.begin();
      if (!lbs.empty()) {
        part.resize(mRel->GetNoTuples() + 1);
        part2.resize(mRel->GetNoTuples() + 1);
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
        part.resize(mRel->GetNoTuples() + 1);
        part2.resize(mRel->GetNoTuples() + 1);
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
  indexResult[e].resize(mRel->GetNoTuples() + 1);
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
  matchInfo.resize(p.getNFAsize());
  matchInfoPtr = &matchInfo;
  newMatchInfo.clear();
  newMatchInfo.resize(p.getNFAsize());
  newMatchInfoPtr = &newMatchInfo;
  trajSize.resize(mRel->GetNoTuples() + 1, 0);
  vector<bool> trajInfo, newTrajInfo;
  trajInfo.assign(mRel->GetNoTuples() + 1, true);
  PatElem elem;
  for (set<int>::iterator it = cruElems.begin(); it != cruElems.end(); it++) {
    p.getElem(*it, elem);
    if (elem.hasIndexableContents()) {
      if (indexMismatch.find(*it) != indexMismatch.end()) {
        cout << "index mismatch at crucial element " << *it << endl;
        return;
      }
      newTrajInfo.assign(mRel->GetNoTuples() + 1, false);
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
  for (int i = 0; i < p.getNFAsize(); i++) {
    (*matchInfoPtr)[i].resize(mRel->GetNoTuples() + 1);
    (*newMatchInfoPtr)[i].resize(mRel->GetNoTuples() + 1);
  }
  unsigned int pred = 0;
  for (unsigned int id = 1; id < trajInfo.size(); id++) {
    if (trajInfo[id]) {
      trajSize[id] = getMsize(id);
      IndexMatchInfo imi(false);
      (*matchInfoPtr)[0][id].imis.push_back(imi);
      deactivated[id] = false;
      for (int s = 0; s < p.getNFAsize(); s++) {
        (*matchInfoPtr)[s][pred].succ = id;
        (*matchInfoPtr)[s][id].pred = pred;
        (*newMatchInfoPtr)[s][pred].succ = id;
        (*newMatchInfoPtr)[s][id].pred = pred;
        if ((int)indexResult[s].size() == mRel->GetNoTuples() + 1) {
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
  for (int s = 0; s < p.getSize(); s++) {
    if ((int)indexResult[s].size() == mRel->GetNoTuples() + 1) {
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
  simplifyNFA(simpleNFA);
  set<int> finalStates = p.getFinalStates();
  set<pair<set<int>, int> > paths;
  findNFApaths(simpleNFA, finalStates, paths);
  set<int> cruElems;
  getCrucialElems(paths, cruElems);
  indexResult.resize(p.getSize());
  deactivated.resize(mRel->GetNoTuples() + 1, false);
  for (set<int>::iterator it = cruElems.begin(); it != cruElems.end(); it++) {
    storeIndexResult(*it);
  }
  deactivated.resize(mRel->GetNoTuples() + 1, true);
  p.initEasyCondOpTrees();
  p.initCondOpTrees();
  matchInfoPtr = &matchInfo;
  newMatchInfoPtr = &newMatchInfo;
  activeTuples = 0;
  initMatchInfo(cruElems);
}

/*
\subsection{Function ~getMsize~}

*/
int IndexMatchesLI::getMsize(TupleId tId) {
  Tuple* tuple = mRel->GetTuple(tId, false);
  int result = -1;
  switch (mtype) {
    case LABEL: {
      result = ((MLabel*)tuple->GetAttribute(attrNr))->GetNoComponents();
      break;
    }
    case LABELS: {
      result = ((MLabels*)tuple->GetAttribute(attrNr))->GetNoComponents();
      break;
    }
    case PLACE: {
      result = ((MPlace*)tuple->GetAttribute(attrNr))->GetNoComponents();
      break;
    }
    default: { // places
      result = ((MPlaces*)tuple->GetAttribute(attrNr))->GetNoComponents();
      break;
    }
  }
  deleteIfAllowed(tuple);
  return result;
}

/*
\subsection{Function ~hasIdIMIs~}

*/
bool IndexMatchesLI::hasIdIMIs(const TupleId id, const int state /* = -1 */) {
  if (state == -1) { // check all states
    for (int s = 0; s < p.getNFAsize(); s++) {
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
//       cout << imiPtr->next + 1 << " | " << counter << endl;
      if (imiPtr->next + 1 >= counter) {
        bool match = false;
        IndexMatchInfo newIMI(true, imiPtr->next, imiPtr->binding, 
                              imiPtr->prevElem);
        if (p.hasConds()) { // extend binding for a complete match
          extendBinding(newIMI, trans.first);
          if (p.isFinalState(trans.second) && imiPtr->finished(trajSize[id])) {
            string var = p.getVarFromElem(trans.first);
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
          match = p.isFinalState(trans.second) && newIMI.finished(trajSize[id]);
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
\subsection{Function ~extendBinding~}

*/
void IndexMatchesLI::extendBinding(IndexMatchInfo& imi, const int e) {
  if (!p.hasConds()) {
    return;
  }
  PatElem elem, prevElem;
  string var, prevVar;
  p.getElem(e, elem);
  if (imi.prevElem > -1) {
    p.getElem(imi.prevElem, prevElem);
    prevElem.getV(prevVar);
  }
  elem.getV(var);
  if (e < imi.prevElem) { // possible for repeated regex
    if (var == prevVar) { // valid case
      if (!var.empty()) { // X [() * ()]+
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
  else if (e == imi.prevElem) { // same atom (and same variable) as before
    if (!var.empty()) { // X * or X [...]+
      imi.binding[var].second = imi.next - 1;
//       cout << "upper limit of " << var << " set to " << imi.next - 1 << endl;
    }
    else { // no variable
//       cout << "no variable for atom " << e << " ===> no changes" << endl;
    }
  }
  else { // different atoms
    if (var == prevVar) { // X [() +]
      if (!var.empty()) {
        imi.binding[var].second = imi.next - 1;
//         cout << "ceiling of " << var << " set to " << imi.next - 1 << endl;
      }
      else { // no variable
//         cout << "no variable for atom " << e << " ====>> no changes" << endl;
      }
    }
    else { // different variables
      if (prevVar.empty()) { // () X ()
        imi.binding[var] = make_pair(imi.next - 1, imi.next - 1);
//         cout << "new var " << var << " bound to " << imi.next - 1 << endl;
      }
      else if (var.empty()) { // X () ()
        if (prevElem.getW() != NO) {
          imi.binding[prevVar].second = imi.next - 2;
//           cout << "prevVar " << prevVar << " finishes at " << imi.next - 2
//                << endl;
        }
      }
      else { // X ... Y ...
        if (elem.getW() == NO) {
          if (prevElem.getW() != NO) { // X * Y ()
            imi.binding[prevVar].second = imi.next - 2;
//             cout << "prevVar " << prevVar << " extended to " << imi.next - 2
//                  << endl;
          }
        }
        else { // wildcard
          if (prevElem.getW() != NO) { // X * Y *
            imi.binding[prevVar].second = imi.next - 2;
//             cout << "prev " << prevVar << " extended to " << imi.next - 2 
//                  << endl;
          }
        }
        imi.binding[var] = make_pair(imi.next - 1, imi.next - 1);
//         cout << "new var " << var << " receives " << imi.next - 1 << endl;
      }
    }
    imi.binding[prevVar].second = imi.next - 2;
  }
  imi.prevElem = e;
//   Tools::printBinding(imi.binding);
}

/*
\subsection{Function ~valuesMatch~}

*/
bool IndexMatchesLI::valuesMatch(const int e, const TupleId id, 
                      IndexMatchInfo& imi, const int newState, const int unit) {
  if (imi.next >= trajSize[id]) {
    return false;
  }
  Tuple *tuple = mRel->GetTuple(id, false);
  bool result = false;
  switch (mtype) {
    case LABEL: {
      MLabel *ml = (MLabel*)(tuple->GetAttribute(attrNr));
      Match<MLabel> match(0, ml);
      result = imiMatch(match, e, id, imi, unit, newState);
      break;
    }
    case LABELS: {
      MLabels *mls = (MLabels*)(tuple->GetAttribute(attrNr));
      Match<MLabels> match(0, mls);
      result = imiMatch(match, e, id, imi, unit, newState);
      break;
    }
    case PLACE: {
      MPlace *mp = (MPlace*)(tuple->GetAttribute(attrNr));
      Match<MPlace> match(0, mp);
      result = imiMatch(match, e, id, imi, unit, newState);
      break;
    }
    default: { // PLACES
      MPlaces *mps = (MPlaces*)(tuple->GetAttribute(attrNr));
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
  if (!p.hasConds()) {
    return true;
  }
  Tuple *tuple = mRel->GetTuple(id, false);
//   cout << "checkConditions: size = " << getMsize(id) 
//        << "; binding(lastVar) = (" << imi.binding[imi.prevVar].first << ", " 
//        << imi.binding[imi.prevVar].second << ")" << endl;
  bool result = false;
  switch (mtype) {
    case LABEL: {
      MLabel *ml = (MLabel*)(tuple->GetAttribute(attrNr));
      Match<MLabel> match(&p, ml);
      result = match.conditionsMatch(*(p.getConds()), imi.binding);
      break;
    }
    case LABELS: {
      MLabels *mls = (MLabels*)(tuple->GetAttribute(attrNr));
      Match<MLabels> match(&p, mls);
      result = match.conditionsMatch(*(p.getConds()), imi.binding);
      break;
    }
    case PLACE: {
      MPlace *mp = (MPlace*)(tuple->GetAttribute(attrNr));
      Match<MPlace> match(&p, mp);
      result = match.conditionsMatch(*(p.getConds()), imi.binding);
      break;
    }
    default: { // PLACES
      MPlaces *mps = (MPlaces*)(tuple->GetAttribute(attrNr));
      Match<MPlaces> match(&p, mps);
      result = match.conditionsMatch(*(p.getConds()), imi.binding);
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
