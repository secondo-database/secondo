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

[1] Source File of the Symbolic Trajectory Algebra

Started March 2012, Fabio Vald\'{e}s

Some basic implementations were done by Frank Panse.

[TOC]

\section{Overview}
This algebra includes the operators ~matches~ and ~rewrite~.

\section{Defines and Includes}

*/

#include "SymbolicTrajectoryAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;

namespace stj {

/*
\section{Implementation of class ~Label~}

\subsection{Constructor}

*/
Label::Label(const Label& rhs) : Attribute(rhs.IsDefined()), value(0) {
  CopyFrom(&rhs);
}

/*
\subsection{Function ~GetValue~}

*/
void Label::GetValue(string& text) const {
  assert(IsDefined());
  SmiSize sz = value.getSize();
  char *bytes;
  if (sz == 0) {
    bytes = new char[1];
    bytes[0] = 0;
  }
  else {
    bytes = new char[sz];
    bool ok = value.read(bytes, sz);
    assert(ok);
  }
  text = bytes;
  delete[] bytes;
}

/*
\subsection{Function ~buildValue~}

*/
void Label::buildValue(const string& text, const unitelem& unit, base& result) {
  result = text;
}

/*
\subsection{Function ~SetValue~}

*/
void Label::SetValue(const string &text) {
  const char *bytes = text.c_str();
  SmiSize sz = strlen(bytes) + 1;
  if (sz > 0) {
    assert(bytes[sz - 1] == 0);
    value.write(bytes, sz);
  }
  else {
    char d = 0;
    value.write(&d, 1);
  }
}

/*
\subsection{Operator ~==~}

*/
bool Label::operator==(const Label& lb) const {
  if (!IsDefined() && !lb.IsDefined()) {
    return true;
  }
  if (IsDefined() != lb.IsDefined()) {
    return false;
  }
  string str1, str2;
  GetValue(str1);
  lb.GetValue(str2);
  return (str1 == str2);
}

/*
\subsection{Operator ~==~}

*/
bool Label::operator==(const string& text) const {
  if (!IsDefined()) {
    return false;
  }
  string str;
  GetValue(str);
  return (str == text);
}

/*
\subsection{Function ~readValueFrom~}

*/
bool Label::readValueFrom(ListExpr LE, string& text, unitelem& unit) {
  if (nl->IsAtom(LE)) {
    nl->WriteToString(text, LE);
    return true;
  }
  return false;
}

/*
\subsection{Function ~ReadFrom~}

*/
bool Label::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  if (nl->IsAtom(LE)) {
    string text;
    nl->WriteToString(text, LE);
    Set(true, text.substr(1, text.length() - 2));
    return true;
  }
  SetDefined(false);
  return false;
}

/*
\subsection{Function ~ToListExpr~}

*/
ListExpr Label::ToListExpr(ListExpr typeInfo) {
  string text;
  GetValue(text);
  return nl->TextAtom(text);
}

/*
\subsection{Function ~CheckKind~}

*/
bool Label::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(type, Label::BasicType()));
}

/*
\subsection{Function ~Property~}

*/
ListExpr Label::Property() {
  return gentc::GenProperty("-> DATA", BasicType(), "(<text>)", "\'Dortmund\'");
}

/*
\subsection{Function ~checkType~}

*/
const bool Label::checkType(const ListExpr type) {
  return listutils::isSymbol(type, BasicType());
}

/*
\subsection{Function ~CopyFrom~}

*/
void Label::CopyFrom(const Attribute* right) {
  SetDefined(right->IsDefined());
  if (IsDefined()) {
    string text;
    ((Label*)right)->GetValue(text);
    SetValue(text);
  }
}

/*
\subsection{Function ~Compare~}

*/
int Label::Compare(const Attribute* arg) const {
  string str1, str2;
  GetValue(str1);
  ((Label*)arg)->GetValue(str2);
  return str1.compare(str2);
}

/*
\subsection{Function ~Print~}

*/
ostream& Label::Print(ostream& os) const {
  string text;
  GetValue(text);
  return os << text;
}

/*
\subsection{Operator ~<<~}

*/
ostream& operator<<(ostream& os, const Label& lb) {
  string text;
  lb.GetValue(text);
  os << "\'" << text << "\'";
  return os;
}

/*
\subsection{Type Constructor}

*/
GenTC<Label> label;

/*
\section{Implementation of class ~MLabels~}

\subsection{Function ~compress~}

If there are subsequent ULabels with the same Label, this function squeezes
them to one ULabel.

*/
void MLabel::compress(MLabel& result) const {
  result.Clear();
  if(!IsDefined()) {
    result.SetDefined(false);
    return;
  }
  ULabel ul(1);
  for (int i = 0; i < GetNoComponents(); i++) {
    Get(i, ul);
    result.MergeAdd(ul);
  }
}

/*
\subsection{Function ~create~}

Creates an MLabel of a certain size for testing purposes. The labels will be
contain only numbers between 1 and size[*]rate; rate being the number of
different labels divided by the size.

*/
void MLabel::createML(int size, bool text, double rate = 1.0) {
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
      vector<string> trajectory = createTrajectory(size);
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
        ul.constValue.Set(true, int2String(max - (i % max)));
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
  ULabel ul(true);
  for (int i = 0; i < source.GetNoComponents(); i++) {
    source.Get(i, us);
    ul.timeInterval = us.timeInterval;
    ul.constValue.SetValue(us.constValue.GetValue());
    Add(ul);
  }
  
}

/*
\subsection{Functions supporting ~rewrite~}

*/
void Assign::setLabelPtr(unsigned int pos, string value) {
  if (pos < pointers[0].size()) {
    ((CcString*)pointers[0][pos])->Set(true, value);
  }
}

void Assign::setTimePtr(unsigned int pos, SecInterval value) {
  if (pos < pointers[1].size()) {
    *((SecInterval*)pointers[1][pos]) = value;
  }
}

void Assign::setStartPtr(unsigned int pos, Instant value) {
  if (pos < pointers[2].size()) {
    *((Instant*)pointers[2][pos]) = value;
  }
}

void Assign::setEndPtr(unsigned int pos, Instant value) {
  if (pos < pointers[3].size()) {
    *((Instant*)pointers[3][pos]) = value;
  }
}

void Assign::setLeftclosedPtr(unsigned int pos, bool value) {
  if (pos < pointers[4].size()) {
    ((CcBool*)pointers[4][pos])->Set(true, value);
  }
}

void Assign::setRightclosedPtr(unsigned int pos, bool value) {
  if (pos < pointers[5].size()) {
    ((CcBool*)pointers[5][pos])->Set(true, value);
  }
}

/*
\subsection{Function ~rewrite~}

Rewrites a moving label using another moving label.

*/
MLabel* MLabel::rewrite(map<string, pair<unsigned int, unsigned int> > binding,
                        vector<Assign> &assigns) const {
//   cout << "rewrite called with binding ";
//   for (map<string, pair<unsigned int, unsigned int> >::iterator i =
//                                                              binding.begin();
//     i != binding.end(); i++) {
//     cout << i->first << "--> [" << i->second.first << ","
//          << i->second.second << "]  ";
//   }
//   cout << endl;
  MLabel *result = new MLabel(1);
  Word qResult;
  string label(""), var("");
  Instant start(instanttype), end(instanttype);
  SecInterval iv(1);
  bool lc(false), rc(false);
  ULabel ul(1), uls(1);
  pair<unsigned int, unsigned int> segment;
  for (unsigned int i = 0; i < assigns.size(); i++) {
    for (int j = 0; j < 6; j++) {
      if (!assigns[i].getText(j).empty()) {
        for (int k = 0; k < assigns[i].getRightSize(j); k++) {
          if (binding.count(assigns[i].getRightVar(j, k))) {
            segment = binding[assigns[i].getRightVar(j, k)];
            switch (assigns[i].getRightKey(j, k)) {
              case 0: { // label
                Get(segment.first, ul);
                string text;
                ul.constValue.GetValue(text);
                assigns[i].setLabelPtr(k, text);
                break;
              }
              case 1: { // time
                Get(segment.first, ul);
                iv.start = ul.timeInterval.start;
                iv.lc = ul.timeInterval.lc;
                Get(segment.second, ul);
                iv.end = ul.timeInterval.end;
                iv.rc = ul.timeInterval.rc;
                assigns[i].setTimePtr(k, iv);
                break;
              }
              case 2: { // start
                Get(segment.first, ul);
                if (j == 2) {
                  assigns[i].setStartPtr(k, ul.timeInterval.start);
                }
                else {
                  assigns[i].setEndPtr(k, ul.timeInterval.start);
                }
                break;
              }
              case 3: { // end
                Get(segment.second, ul);
                if (j == 2) {
                  assigns[i].setStartPtr(k, ul.timeInterval.end);
                }
                else {
                  assigns[i].setEndPtr(k, ul.timeInterval.end);
                }
                break;
              }
              case 4: { // leftclosed
                Get(segment.first, ul);
                if (j == 4) {
                  assigns[i].setLeftclosedPtr(k, ul.timeInterval.lc);
                }
                else {
                  assigns[i].setRightclosedPtr(k, ul.timeInterval.lc);
                }
                break;
              }
              case 5: { // rightclosed
                Get(segment.second, ul);
                if (j == 4) {
                  assigns[i].setLeftclosedPtr(k, ul.timeInterval.rc);
                }
                else {
                  assigns[i].setRightclosedPtr(k, ul.timeInterval.rc);
                }
                break;
              }
              default: { // cannot occur
                cout << "Error: assigns[" << i << "].getRightKey(" << j << ", "
                     << k << ") = " << assigns[i].getRightKey(j, k) << endl;
                result->SetDefined(false);
                return result;
              }
            }
          }
          else { // variable from right size unbound
            result->SetDefined(false);
            return result;
          }
        }
      }
    } // all pointers are set now
    if (!assigns[i].getText(0).empty()) {
      assigns[i].getQP(0)->EvalS(assigns[i].getOpTree(0), qResult, OPEN);
      label = ((CcString*)qResult.addr)->GetValue();
    }
    if (!assigns[i].getText(1).empty()) {
      assigns[i].getQP(1)->EvalS(assigns[i].getOpTree(1), qResult, OPEN);
      iv = *((SecInterval*)qResult.addr);
    }
    if (!assigns[i].getText(2).empty()) {
      assigns[i].getQP(2)->EvalS(assigns[i].getOpTree(2), qResult, OPEN);
      start = *((Instant*)qResult.addr);
    }
    if (!assigns[i].getText(3).empty()) {
      assigns[i].getQP(3)->EvalS(assigns[i].getOpTree(3), qResult, OPEN);
      end = *((Instant*)qResult.addr);
    }
    if (!assigns[i].getText(4).empty()) {
      assigns[i].getQP(4)->EvalS(assigns[i].getOpTree(4), qResult, OPEN);
      lc = ((CcBool*)qResult.addr)->GetValue();
    }
    if (!assigns[i].getText(5).empty()) {
      assigns[i].getQP(5)->EvalS(assigns[i].getOpTree(5), qResult, OPEN);
      rc = ((CcBool*)qResult.addr)->GetValue();
    }
     // information from assignment i collected
    if (binding.count(assigns[i].getV())) { // variable occurs in binding
      segment = binding[assigns[i].getV()];
      if (segment.second == segment.first) { // 1 source ul
        Get(segment.first, uls);
        if (!assigns[i].getText(0).empty()) {
          uls.constValue.Set(true, label);
        }
        if (!assigns[i].getText(1).empty()) {
          uls.timeInterval = iv;
        }
        if (!assigns[i].getText(2).empty()) {
          uls.timeInterval.start = start;
        }
        if (!assigns[i].getText(3).empty()) {
          uls.timeInterval.end = end;
        }
        if (!assigns[i].getText(4).empty()) {
          uls.timeInterval.lc = lc;
        }
        if (!assigns[i].getText(5).empty()) {
          uls.timeInterval.rc = rc;
        }
        if (!uls.timeInterval.IsValid()) {
          uls.timeInterval.Print(cout);
          cout << " is an invalid interval" << endl;
          result->SetDefined(false);
          return result;
        }
        result->Add(uls);
      }
      else { // arbitrary many source uls
        for (unsigned int m = segment.first; m <= segment.second; m++) {
          Get(m, uls);
          if (!assigns[i].getText(0).empty()) {
            uls.constValue.Set(true, label);
          }
          if ((m == segment.first) && // first unit label
            (!assigns[i].getText(1).empty() || !assigns[i].getText(2).empty())){
            uls.timeInterval.start = start;
            if (!uls.timeInterval.IsValid()) {
              uls.timeInterval.Print(cout);
              cout << " is an invalid interval" << endl;
              result->SetDefined(false);
              return result;
            }
          }
          if ((m == segment.second) && // last unit label
            (!assigns[i].getText(1).empty() || !assigns[i].getText(3).empty())){
            uls.timeInterval.end = end;
            if (!uls.timeInterval.IsValid()) {
              uls.timeInterval.Print(cout);
              cout << " is an invalid interval" << endl;
              result->SetDefined(false);
              return result;
            }
          }
          if ((m == segment.first) && !assigns[i].getText(4).empty()) {
            uls.timeInterval.lc = lc;
          }
          if ((m == segment.second) && !assigns[i].getText(5).empty()) {
            uls.timeInterval.rc = rc;
          }
          result->Add(uls);
        }
      }
    }
    else { // variable does not occur in binding
      if (!assigns[i].occurs()) { // and not in pattern
        uls.constValue.Set(true, label);
        if (!assigns[i].getText(1).empty()) {
          uls.timeInterval = iv;
        }
        else {
          uls.timeInterval.start = start;
          uls.timeInterval.end = end;
        }
        if (!assigns[i].getText(4).empty()) {
          uls.timeInterval.lc = lc;
        }
        if (!assigns[i].getText(5).empty()) {
          uls.timeInterval.rc = rc;
        }
        result->Add(uls);
      }
    }
  }
  result->SetDefined(result->IsValid());
  return result;
}

/*
\section{Implementation of class ~Labels~}

\subsection{Constructors}

*/
Labels::Labels(const Labels& src, const bool sort /* = false */) :
  Attribute(src.IsDefined()), values(src.GetLength()), pos(src.GetNoValues()) {
  if (sort) {
    set<string> labels;
    for (int i = 0; i < src.GetNoValues(); i++) {
      string text;
      src.GetValue(i, text);
      labels.insert(text); // automatic sorting
    }
    for (set<string>::iterator it = labels.begin(); it != labels.end(); it++) {
      Append(*it); // append in alphabetical order
    }
  }
  else {
    CopyFrom(&src); // keep original order
  }
}

/*
\subsection{Operator ~=~}

*/
Labels& Labels::operator=(const Labels& src) {
  Attribute::operator=(src);
  values.copyFrom(src.values);
  pos.copyFrom(src.pos);
  return *this;
}

/*
\subsection{Operator ~==~}

*/
bool Labels::operator==(const Labels& src) const {
  if (!IsDefined() && !src.IsDefined()) {
    return true;
  }
  if (IsDefined() != src.IsDefined()) {
    return false;
  }
  if ((GetNoValues() != src.GetNoValues()) || (GetLength() != src.GetLength())){
    return false;
  }
  string str1, str2;
  for (int i = 0; i < GetNoValues(); i++) {
    GetValue(i, str1);
    src.GetValue(i, str2);
    if (str1 != str2) {
      return false;
    }
  }
  return true;
}

/*
\subsection{Functions ~Append~}

*/
void Labels::Append(const Label& lb) {
  string text;
  lb.GetValue(text);
  Append(text);
}

void Labels::Append(const string& text) {
  pos.Append(values.getSize());
  const char *bytes = text.c_str();
  SmiSize sz = strlen(bytes) + 1;
  if (sz > 0) {
    assert(bytes[sz - 1] == 0);
    values.write(bytes, sz, values.getSize());
  }
  else {
    char d = 0;
    values.write(&d, 1, values.getSize());
  }
}

/*
\subsection{Function ~Get~}

*/
void Labels::Get(int i, Label& lb) const {
  lb.SetDefined(IsDefined());
  assert((0 <= i) && (i < GetNoValues()));
  if (IsDefined()) {
    string text;
    GetValue(i, text);
    lb.Set(true, text);
  }
}

/*
\subsection{Function ~GetValue~}

*/
void Labels::GetValue(int i, string& text) const {
  assert((0 <= i) && (i < GetNoValues()) && IsDefined());
  unsigned int cur(-1), next(-1);
  pos.Get(i, cur);
  if (i == GetNoValues() - 1) { // last value
    next = values.getSize();
  }
  else {
    pos.Get(i + 1, next);
  }
  char *bytes = new char[next - cur];
  values.read(bytes, next - cur, cur);
  text = bytes;
  delete[] bytes;
}

/*
\subsection{Function ~getRefToLastElem~}

*/
void Labels::getRefToLastElem(const int size, unsigned int& result) {
  result = size;
}

/*
\subsection{Function ~getFlobPos~}

*/
void Labels::getFlobPos(const pair<unsigned int, unsigned int> elems, 
                        pair<size_t, size_t>& result) {
  result.first = elems.first;
  result.second = elems.second;
}

/*
\subsection{Function ~valuesToListExpr~}

*/
void Labels::valuesToListExpr(const vector<string>& values, ListExpr& result) {
  if (values.empty()) {
    result = nl->Empty();
    return;
  }
  result = nl->OneElemList(nl->TextAtom(values[0]));
  ListExpr last = result;
  for (unsigned int i = 1; i < values.size(); i++) {
    last = nl->Append(last, nl->TextAtom(values[i]));
  }
}

/*
\subsection{Function ~getString~}

*/
void Labels::getString(const ListExpr& list, string& result) {
  nl->WriteToString(result, list);
}

/*
\subsection{Function ~getElemFromList~}

*/
void Labels::getElemFromList(const ListExpr& list, const unsigned int size, 
                             unsigned int& result) {
  result = size;
}

/*
\subsection{Function ~buildValue~}

*/
void Labels::buildValue(const string& text, const unsigned int pos,
                        string& result) {
  result = text;
}

/*
\subsection{Function ~Contains~}

*/
bool Labels::Contains(string& text) const {
  string str;
  for (int i = 0; i < GetNoValues(); i++) {
    GetValue(i, str);
    if (str == text) {
      return true;
    }
  }
  return false;
}

/*
\subsection{Operator ~<<~}

*/
ostream& operator<<(ostream& os, const Labels& lbs) {
  Label lb(true);
  string text;
  for(int i = 0; i < lbs.GetNoValues() - 1; i++) {
    lbs.GetValue(i, text);
    os << text << " ";
  }
  lbs.GetValue(lbs.GetNoValues() - 1, text);
  os << text;
  return os;
}

/*
\subsection{Function ~GetFLOB~}

*/
Flob *Labels::GetFLOB(const int i) {
  assert(i >= 0 && i < NumOfFLOBs());
  return (i == 0 ? &values : &pos);
}


/*
\subsection{Function ~Compare~}

*/
int Labels::Compare(const Attribute* arg) const {
  if (GetNoValues() > ((Labels*)arg)->GetNoValues()) {
    return 1;
  }
  if (GetNoValues() < ((Labels*)arg)->GetNoValues()) {
    return -1;
  }
  string str1, str2;
  for (int i = 0; i < GetNoValues(); i++) { // equal size; compare labels
    GetValue(i, str1);
    ((Labels*)arg)->GetValue(i, str2);
    int comp = str1.compare(str2);
    if (comp != 0) {
      return comp;
    }
  }
  return 0;
}

/*
\subsection{Function ~Property~}

*/
ListExpr Labels::Property() {
  return gentc::GenProperty("-> DATA", BasicType(), "(<label> <label> ...)",
    "(\'Dortmund\' \'Mailand\')");
}

/*
\subsection{Function ~CheckKind~}

*/
bool Labels::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, Labels::BasicType());
}

/*
\subsection{Function ~ToListExpr~}

*/
ListExpr Labels::ToListExpr(ListExpr typeInfo) {
  if (!IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  if (IsEmpty()) {
    return nl->Empty();
  }
  else {
    string text;
    GetValue(0, text);
    ListExpr res = nl->OneElemList(nl->TextAtom(text));
    ListExpr last = res;
    for (int i = 1; i < GetNoValues(); i++) {
      GetValue(i, text);
      last = nl->Append(last, nl->TextAtom(text));
    }
    return res;
  }
}

/*
\subsection{Function ~ReadFrom~}

*/
bool Labels::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  Clean();
  if (listutils::isSymbolUndefined(LE)) {
    SetDefined(false);
    return true;
  }
  SetDefined(true);
  ListExpr rest = LE;
  string text;
  while (!nl->IsEmpty(rest)) {
    if (!nl->IsAtom(nl->First(rest))) {
      SetDefined(false);
      return false;
    }
    nl->WriteToString(text, nl->First(rest));
    Append(text.substr(1, text.length() - 2));
    rest = nl->Rest(rest);
  }
  return true;
}

/*
\subsection{Function ~HashValue~}

*/
size_t Labels::HashValue() const {
  if (IsEmpty()) {
    return 0;
  }
  size_t result = 1;
  for (int i = 1; i < pos.Size(); i++) {
    unsigned int factor;
    pos.Get(i, factor);
    result *= factor;
  }
  return GetNoValues() * result;
}

/*
\subsection{Type Constructor}

*/
GenTC<Labels> labels;

/*
\section{Implementation of class ~MBasics~}

\subsection{Constructors}

*/
template<class B>
MBasics<B>::MBasics(const MBasics &mbs) : Attribute(mbs.IsDefined()), 
  values(0), units(mbs.GetNoComponents()), pos(0) {
  values.copyFrom(mbs.values);
  units.copyFrom(mbs.units);
  pos.copyFrom(mbs.pos);
}

/*
\subsection{Function ~Property~}

*/
template<class B>
ListExpr MBasics<B>::Property() {
  if (B::BasicType() == "labels") {
    return gentc::GenProperty("-> DATA", BasicType(),
      "((<interval> <labels>) (<interval> <labels>) ...)",
      "(((\"2014-01-29\" \"2014-01-30\" TRUE FALSE) (\"home\" \"Dortmund\")))");
  }
  if (B::BasicType() == "places") {
    return gentc::GenProperty("-> DATA", BasicType(),
      "((<interval> <places>) (<interval> <places>) ...)",
      "(((\"2014-01-29\" \"2014-01-30\" TRUE FALSE) ((\"home\" 2012) "
      "(\"Dortmund\" 1909))))");
  }
  return gentc::GenProperty("-> DATA", BasicType(), "Error: invalid type.", "");
}

/*
\subsection{Function ~CheckKind~}

*/
template<class B>
bool MBasics<B>::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, MBasics<B>::BasicType());
}

/*
\subsection{Function ~checkType~}

*/
template<class B>
bool MBasics<B>::checkType(ListExpr t) {
  return listutils::isSymbol(t, MBasics<B>::BasicType());
}

/*
\subsection{Function ~GetFLOB~}

*/
template<class B>
Flob* MBasics<B>::GetFLOB(const int i) {
  assert((i >= 0) && (i < NumOfFLOBs()));
  if (i == 0) {
    return &values;
  }
  if (i == 1) {
    return &units;
  }
  return &pos;
}

/*
\subsection{Function ~Compare~}

*/
template<class B>
int MBasics<B>::Compare(const Attribute* arg) const {
  if (GetNoComponents() == ((MBasics<B>*)arg)->GetNoComponents()) {
    if (GetNoValues() == ((MBasics<B>*)arg)->GetNoValues()) {
      return 0;
    }
    return (GetNoValues() > ((MBasics<B>*)arg)->GetNoValues() ? 1 : -1);
  }
  else {
    return (GetNoComponents() > ((MBasics<B>*)arg)->GetNoComponents() ? 
            1 : -1);
  }
}

/*
\subsection{Function ~Clone~}

*/
template<class B>
Attribute* MBasics<B>::Clone() const {
  return new (MBasics<B>)(*(MBasics<B>*)this);
}

/*
\subsection{Function ~getEndPos~}

*/
template<class B>
int MBasics<B>::getUnitEndPos(const int i) const {
  if (i < GetNoComponents() - 1) {
    SymbolicUnit nextUnit;
    units.Get(i + 1, nextUnit);
    return nextUnit.pos - 1;
  }
  else { // last unit
    return pos.Size() - 1;
  }
}

/*
\subsection{Function ~unitToListExpr~}

*/
template<class B>
ListExpr MBasics<B>::unitToListExpr(int i) {
  assert ((i >= 0) && (i < GetNoComponents()));
  SymbolicUnit unit;
  units.Get(i, unit);
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  vector<typename B::base> values;
  GetValues(i, values);
  ListExpr valuelist;
  B::valuesToListExpr(values, valuelist);
  return nl->TwoElemList(unit.iv.ToListExpr(sc->GetTypeExpr(
                                            SecInterval::BasicType())),
                         valuelist);
}

/*
\subsection{Function ~ToListExpr~}

*/
template<class B>
ListExpr MBasics<B>::ToListExpr(ListExpr typeInfo) {
  if (!IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  if (IsEmpty()) {
    return nl->Empty();
  }
  ListExpr result = nl->OneElemList(unitToListExpr(0));
  ListExpr last = result;
  for (int i = 1; i < GetNoComponents(); i++) {
    last = nl->Append(last, unitToListExpr(i));
  }
  return result;
}

/*
\subsection{Function ~readValues~}

*/
template<class B>
bool MBasics<B>::readValues(ListExpr valuelist) {
  ListExpr rest = valuelist;
  string text;
  typename B::arrayelem elem, testelem;
  while (!nl->IsEmpty(rest)) {
    if (listutils::isSymbolUndefined(nl->First(rest))) {
      return false;
    }
    B::getString(nl->First(rest), text);
    B::getElemFromList(nl->First(rest), values.getSize(), elem);
    pos.Append(elem);
    pos.Get(pos.Size() - 1, testelem);
    const char *bytes = (text.substr(1, text.length() - 2)).c_str();
    SmiSize sz = strlen(bytes) + 1;
    if (sz > 0) {
      assert(bytes[sz - 1] == 0);
      values.write(bytes, sz, values.getSize());
    }
    else {
      char d = 0;
      values.write(&d, 1, values.getSize());
    }
    rest = nl->Rest(rest);
  }
  return true;
}

/*
\subsection{Function ~readUnit~}

*/
template<class B>
bool MBasics<B>::readUnit(ListExpr unitlist) {
  if (!nl->HasLength(unitlist, 2)) {
    return false;
  }
  SymbolicUnit unit(GetNoValues());
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  if (!unit.iv.ReadFrom(nl->First(unitlist), 
                        sc->GetTypeExpr(SecInterval::BasicType()))) {
    return false;
  }
  if (!readValues(nl->Second(unitlist))) {
    return false;
  }
  if (GetNoComponents() > 0) {
    SymbolicUnit prevUnit;
    units.Get(GetNoComponents() - 1, prevUnit);
    if (!(prevUnit.iv.Before(unit.iv))) { // check time intervals
      return false;
    }
  }
  units.Append(unit);
  return true;
}

/*
\subsection{Function ~ReadFrom~}

*/
template<class B>
bool MBasics<B>::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  values.clean();
  units.clean();
  pos.clean();
  if (listutils::isSymbolUndefined(LE)) {
    SetDefined(false);
    return true;
  }
  ListExpr rest = LE;
  while (!nl->IsEmpty(rest)) {
    if (!readUnit(nl->First(rest))) {
      SetDefined(false);
      return true;
    }
    rest = nl->Rest(rest);
  }
  SetDefined(true);
  return true;
}

/*
\subsection{Operator ~<<~}

*/
template<class B>
ostream& operator<<(ostream& o, const MBasics<B>& mbs) {
  o << nl->ToString(mbs.ToListExpr());
  return o;
}

/*
\subsection{Function ~Position~}

*/
template<class B>
int MBasics<B>::Position(const Instant& inst) const {
  assert(IsDefined());
  assert(inst.IsDefined());
  int first = 0, last = GetNoComponents() - 1;
  Instant t1 = inst;
  while (first <= last) {
    int mid = (first + last) / 2;
    if ((mid < 0) || (mid >= GetNoComponents())) {
      return -1;
    }
    SymbolicUnit unit;
    units.Get(mid, unit);
    if (((Interval<Instant>)(unit.iv)).Contains(t1)) {
      return mid;
    }
    else { // not contained
      if ((t1 > unit.iv.end) || (t1 == unit.iv.end)) {
        first = mid + 1;
      }
      else if ((t1 < unit.iv.start) || (t1 == unit.iv.start)) {
        last = mid - 1;
      }
      else {
        return -1; // should never be reached.
      }
    }
  }
  return -1;
}

/*
\subsection{Function ~Get~}

*/
template<class B>
void MBasics<B>::Get(const int i, UBasics<B>& result) const {
  assert(IsDefined());
  assert((i >= 0) && (i < GetNoComponents()));
  result.SetDefined(true);
  result.constValue.Clean();
  vector<typename B::base> values;
  GetValues(i, values);
  for (unsigned int j = 0; j < values.size(); j++) {
    result.constValue.Append(values[j]);
  }
  SecInterval iv;
  GetInterval(i, iv);
  result.timeInterval = iv;
}

/*
\subsection{Function ~GetValues~}

*/
template<class B>
void MBasics<B>::GetBasics(const int i, B& result) const {
  assert(IsDefined());
  assert((i >= 0) && (i < GetNoComponents()));
  result.Clean();
  vector<typename B::base> values;
  GetValues(i, values);
  for (unsigned int j = 0; j < values.size(); j++) {
    result.Append(values[j]);
  }
}

/*
\subsection{Function ~GetValues~}

*/
template<class B>
void MBasics<B>::GetValues(const int i, vector<typename B::base>& result) const{
  assert (IsDefined() && (i >= 0) && (i < GetNoComponents()));
  result.clear();
  SymbolicUnit unit;
  units.Get(i, unit);
  typename B::base val;
  pair<typename B::arrayelem, typename B::arrayelem> elems;
  pair<size_t, size_t> flobpos;
  for (int j = unit.pos; j <= getUnitEndPos(i); j++) {
    pos.Get(j, elems.first);
    if (j < pos.Size() - 1) { // not the final entry
      pos.Get(j + 1, elems.second);
    }
    else {
      B::getRefToLastElem(values.getSize() - 1, elems.second);
    }
    B::getFlobPos(elems, flobpos);
    char *bytes = new char[flobpos.second - flobpos.first + 1];
    values.read(bytes, flobpos.second - flobpos.first + 1, flobpos.first);
    string text = bytes;
    B::buildValue(text, elems.first, val);
    result.push_back(val);
    delete[] bytes;
  }
}

/*
\subsection{Function ~GetInterval~}

*/
template<class B>
void MBasics<B>::GetInterval(const int i, SecInterval& result) const {
  assert((i >= 0) && (i < GetNoComponents()));
  result.SetDefined(true);
  SymbolicUnit unit;
  units.Get(i, unit);
  result = unit.iv;
}

/*
\subsection{Function ~Clear~}

*/
template<class B>
void MBasics<B>::Clear() {
  units.clean();
  values.clean();
  pos.clean();
}

/*
\subsection{Function ~EndBulkLoad~}

*/
template<class B>
void MBasics<B>::EndBulkLoad(const bool sort /*= true*/, 
                             const bool checkvalid /*= true*/) {
  if (!IsDefined()) {
    units.clean();
    values.clean();
    pos.clean();
  }
  units.TrimToSize();
  pos.TrimToSize();
}

/*
\subsection{Function ~Add~}

*/
template<class B>
void MBasics<B>::Add(const UBasics<B>& ut) {
  assert(IsDefined() && ut.IsDefined());
  UBasics<B> ut2(ut);
  if (!readUnit(ut2.ToListExpr(nl->Empty()))) {
    SetDefined(false);
  }
}

template<class B>
void MBasics<B>::Add(const SecInterval& iv, const B& values) {
  assert(IsDefined() && iv.IsDefined() && values.IsDefined());
  B values2(values);
  ListExpr unitlist = nl->TwoElemList(iv.ToListExpr(nl->Empty()),
                                      values2.ToListExpr(nl->Empty()));
  if (!readUnit(unitlist)) {
    SetDefined(false);
  }
}

/*
\subsection{Function ~MergeAdd~}

*/
template<class B>
void MBasics<B>::MergeAdd(const SecInterval& iv, const B& values) {
  assert(IsDefined() && iv.IsDefined() && values.IsDefined());
  if (!iv.IsDefined() || !iv.IsValid()) {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
      << " MergeAdd(Unit): Unit is undefined or invalid:";
    iv.Print(cout); cout << endl;
    assert(false);
  }
  if (GetNoComponents() > 0) {
    SymbolicUnit prevUnit;
    units.Get(GetNoComponents() - 1, prevUnit);
    int numOfValues = getUnitEndPos(GetNoComponents() - 1) - prevUnit.pos + 1;
    bool equal = (values.GetNoValues() == numOfValues);
    vector<typename B::base> mvalues;
    GetValues(GetNoComponents() - 1, mvalues);
    for (unsigned int i = 0; ((i < mvalues.size()) && equal); i++) {
      typename B::base value;
      values.GetValue(i, value);
      if (!(mvalues[i] == value)) {
        equal = false;
      }
    }
    if (equal && (prevUnit.iv.end == iv.start) && (prevUnit.iv.rc || iv.lc)) {
      prevUnit.iv.end = iv.end; // adjacent invervals \& equal labels
      prevUnit.iv.rc = iv.rc;
      units.Put(GetNoComponents() - 1, prevUnit);
    }
    else if (prevUnit.iv.Before(iv)) {
      Add(iv, values);
    }
  }
  else { // first unit
    Add(iv, values);
  }
}

template<class B>
void MBasics<B>::MergeAdd(const UBasics<B>& ub) {
  MergeAdd(ub.timeInterval, ub.constValue);
}

/*
\subsection{Function ~Passes~}

*/
template<class B>
bool MBasics<B>::Passes(const typename B::single& sg) const {
  vector<typename B::base> vals;
  typename B::base val;
  sg.GetValue(val);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetValues(i, vals);
    for (unsigned int j = 0; j < vals.size(); j++) {
      if (vals[j] == val) {
        return true;
      }
    }
  }
  return false;
}

/*
\subsection{Function ~Passes~}

*/
template<class B>
bool MBasics<B>::Passes(const B& bs) const {
  B basics(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetBasics(i, basics);
    if (basics == bs) {
      return true;
    }
  }
  return false;
}

/*
\subsection{Function ~At~}

*/
template<class B>
void MBasics<B>::At(const typename B::single& sg, MBasics<B>& result) const {
  result.Clear();
  if (!IsDefined()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    B basics(true);
    GetBasics(i, basics);
    bool found = false;
    int j = 0;
    typename B::base val;
    while ((j < basics.GetNoValues()) && !found) {
      basics.GetValue(j, val);
      if (sg == val) {
        found = true;
      }
      j++;
    }
    if (found) {
      SecInterval iv;
      GetInterval(i, iv);
      UBasics<B> ut(iv, basics);
      result.Add(ut);
    }
  }
}

/*
\subsection{Function ~At~}

*/
template<class B>
void MBasics<B>::At(const B& bs, MBasics<B>& result) const {
  result.Clear();
  if (!IsDefined()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    B basics(true);
    GetBasics(i, basics);
    if (basics == bs) {
      SecInterval iv;
      GetInterval(i, iv);
      result.Add(iv, basics);
    }
  }
}

/*
\subsection{Function ~DefTime~}

*/
template<class B>
void MBasics<B>::DefTime(Periods& per) const {
  per.Clear();
  per.SetDefined(IsDefined());
  if (IsDefined()) {
    SymbolicUnit unit;
    for (int i = 0; i < GetNoComponents(); i++) {
      units.Get(i, unit);
      per.MergeAdd(unit.iv);
    }
  }
}

/*
\subsection{Function ~Atinstant~}

*/
template<class B>
void MBasics<B>::Atinstant(const Instant& inst, 
                           IBasics<B>& result) const {
  if(!IsDefined() || !inst.IsDefined()) {
    result.SetDefined(false);
    return;
  }
  int pos = Position(inst);
  if (pos == -1) {
    result.SetDefined(false);
  }
  else {
    GetBasics(pos, result.value);
    result.SetDefined(true);
    result.instant = inst;
  }
}

/*
\subsection{Function ~Initial~}

*/
template<class B>
void MBasics<B>::Initial(IBasics<B>& result) const {
  if (!IsDefined() || !GetNoComponents()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  SecInterval iv;
  GetInterval(0, iv);
  result.instant = iv.start;
  GetBasics(0, result.value);
}

/*
\subsection{Function ~Final~}

*/
template<class B>
void MBasics<B>::Final(IBasics<B>& result) const {
  if (!IsDefined() || !GetNoComponents()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  SecInterval iv;
  GetInterval(GetNoComponents() - 1, iv);
  result.instant = iv.end;
  GetBasics(GetNoComponents() - 1, result.value);
}

/*
\section{Type Constructors}

*/
GenTC<MBasics<Labels> > mlabels;
GenTC<MBasics<Places> > mplaces;

/*
\section{Implementation of class ~Place~}

\subsection{Function ~Set~}

*/
void Place::Set(const bool defined, const pair<string, unsigned int>& value) {
  Label::Set(defined, value.first);
  SetRef(value.second);
}

/*
\subsection{Function ~SetValue~}

*/
void Place::SetValue(const pair<string, unsigned int>& value) {
  Label::SetValue(value.first);
  SetRef(value.second);
}

/*
\subsection{Function ~GetValue~}

*/
void Place::GetValue(pair<string, unsigned int>& value) const {
  Label::GetValue(value.first);
  value.second = GetRef();
}

/*
\subsection{Function ~buildValue~}

*/
void Place::buildValue(const string& text, const unitelem& unit, base& result) {
  result.first = text;
  result.second = unit.ref;
}

/*
\subsection{Function ~getList~}

*/
ListExpr Place::getList(const base& value) {
  return nl->TwoElemList(nl->TextAtom(value.first), nl->IntAtom(value.second));
}

/*
\subsection{Operator ~=~}

*/
Place& Place::operator=(const Place& p) {
  pair<string, unsigned int> value;
  p.GetValue(value);
  Set(p.IsDefined(), value);
  return *this;
}

/*
\subsection{Operator ~==~}

*/
bool Place::operator==(const Place& p) const {
  if (!IsDefined() && !p.IsDefined()) { // both undefined
    return true;
  }
  if (IsDefined() != p.IsDefined()) { // one defined, one undefined
    return false;
  }
  pair<string, unsigned int> value;
  p.GetValue(value);
  return operator==(value);
}

bool Place::operator==(const pair<string, unsigned int>& value) const {
  if (!IsDefined()) {
    return false;
  }
  pair<string, unsigned int> val;
  GetValue(val);
  return (val == value);
}

/*
\subsection{Function ~Property~}

*/
ListExpr Place::Property() {
  return gentc::GenProperty("-> DATA", BasicType(), "(<label> <int>)",
    "(\'Dortmund\' 1909)");
}

/*
\subsection{Function ~CheckKind~}

*/
bool Place::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, Place::BasicType());
}

/*
\subsection{Function ~Compare~}

*/
int Place::Compare(const Attribute* arg) const {
  int cmp = Label::Compare(arg);
  if (cmp != 0) {
    return cmp;
  }
  if (ref > ((Place*)arg)->GetRef()) {
    return 1;
  }
  if (ref < ((Place*)arg)->GetRef()) {
    return -1;
  }
  return 0;
}

/*
\subsection{Function ~ToListExpr~}

*/
ListExpr Place::ToListExpr(ListExpr typeInfo) {
  if (!IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  base val;
  GetValue(val);
  return nl->TwoElemList(nl->TextAtom(val.first), nl->IntAtom(val.second));
}

/*
\subsection{Function ~readValueFrom~}

*/
bool Place::readValueFrom(ListExpr LE, string& text, unitelem& unit) {
  if (nl->HasLength(LE, 2)) {
    if (nl->IsAtom(nl->First(LE)) && nl->IsAtom(nl->Second(LE))) {
      nl->WriteToString(text, nl->First(LE));
      unit.ref = nl->IntValue(nl->Second(LE));
      return true;
    }
  }
  return false;
}

/*
\subsection{Function ~ReadFrom~}

*/
bool Place::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  if (listutils::isSymbolUndefined(LE)) {
    SetDefined(false);
    return true;
  }
  if (!nl->HasLength(LE, 2)) {
    SetDefined(false);
    return false;
  }
  string text;
  nl->WriteToString(text, nl->First(LE));
  Set(true, make_pair(text.substr(1, text.length() - 2), 
                      nl->IntValue(nl->Second(LE))));
  return true;
}

/*
\subsection{Type Constructor}

*/
GenTC<Place> place;

/*
\section{Implementation of class ~Places~}

\subsection{Constructor}

*/
Places::Places(const Places& rhs,  const bool sort /* = false */) :
Attribute(rhs.IsDefined()), values(rhs.GetLength()), posref(rhs.GetNoValues()) {
  if (sort) {
    set<base> vals;
    for (int i = 0; i < rhs.GetNoValues(); i++) {
      base val;
      rhs.GetValue(i, val);
      vals.insert(val); // automatic sorting
    }
    set<base>::iterator it;
    for (it = vals.begin(); it != vals.end(); it++) {
      Append(*it); // append in automatic order
    }
  }
  else {
    CopyFrom(&rhs); // keep original order
  }
}

/*
\subsection{Function ~Append~}

*/
void Places::Append(const base& val) {
  NewPair<unsigned int, unsigned int> pr;
  pr.first = values.getSize();
  pr.second = val.second;
  posref.Append(pr);
  const char *bytes = val.first.c_str();
  SmiSize sz = strlen(bytes) + 1;
  if (sz > 0) {
    assert(bytes[sz - 1] == 0);
    values.write(bytes, sz, values.getSize());
  }
  else {
    char d = 0;
    values.write(&d, 1, values.getSize());
  }
}

void Places::Append(const Place& pl) {
  pair<string, unsigned int> val;
  pl.GetValue(val);
  Append(val);
}

/*
\subsection{Function ~GetPlace~}

*/
void Places::Get(const int i, Place& result) const {
  assert((0 <= i) && (i < GetNoValues()));
  base val;
  GetValue(i, val);
  result.Set(true, val);
}

/*
\subsection{Function ~GetValue~}

*/
void Places::GetValue(const int i, base& result) const {
  assert(IsDefined() && (0 <= i) && (i < GetNoValues()));
  NewPair<unsigned int, unsigned int> cur, next;
  unsigned int nextPos;
  posref.Get(i, cur);
  if (i == GetNoValues() - 1) { // last value
    nextPos = values.getSize();
  }
  else {
    posref.Get(i + 1, next);
    nextPos = next.first;
  }
  SmiSize sz = nextPos - cur.first;
  char *bytes;
  if (sz == 0) {
    bytes = new char[1];
    bytes[0] = 0;
  }
  else {
    bytes = new char[sz];
    bool ok = values.read(bytes, sz, cur.first);
    assert(ok);
  }
  result.first = bytes;
  result.second = cur.second;
  delete[] bytes;
}

/*
\subsection{Function ~Contains~}

*/
bool Places::Contains(const base& val) const {
  base value;
  for (int i = 0; i < GetNoValues(); i++) {
    GetValue(i, value);
    if (value == val) {
      return true;
    }
  }
  return false;
}

/*
\subsection{Function ~getRefToLastElem~}

*/
void Places::getRefToLastElem(const int size, arrayelem& result) {
  result.first = size;
}

/*
\subsection{Function ~getFlobPos~}

*/
void Places::getFlobPos(const pair<arrayelem, arrayelem>& elems, 
                        pair<size_t, size_t>& result) {
  result.first = elems.first.first;
  result.second = elems.second.first;
}

/*
\subsection{Function ~valuesToListExpr~}

*/
void Places::valuesToListExpr(const vector<base>& values, ListExpr& result) {
  if (values.empty()) {
    result = nl->Empty();
    return;
  }
  result = nl->OneElemList(nl->TwoElemList(nl->TextAtom(values[0].first),
                                           nl->IntAtom(values[0].second)));
  ListExpr last = result;
  for (unsigned int i = 1; i < values.size(); i++) {
    last = nl->Append(last, nl->TwoElemList(nl->TextAtom(values[i].first),
                                            nl->IntAtom(values[i].second)));
  }
}

/*
\subsection{Function ~getString~}

*/
void Places::getString(const ListExpr& list, string& result) {
  nl->WriteToString(result, nl->First(list));
}

/*
\subsection{Function ~getElemFromList~}

*/
void Places::getElemFromList(const ListExpr& list, const unsigned int size, 
                             arrayelem& result) {
  result.first = size;
  result.second = nl->IntValue(nl->Second(list));
}

/*
\subsection{Function ~buildValue~}

*/
void Places::buildValue(const string& text, const arrayelem pos, base& result) {
  result.first = text;
  result.second = pos.second;
}

/*
\subsection{Operator ~=~}

*/
void Places::operator=(const Places& p) {
  SetDefined(p.IsDefined());
  if (IsDefined()) {
    values.copyFrom(p.values);
    posref.copyFrom(p.posref);
  }
}

/*
\subsection{Operator ~==~}

*/
bool Places::operator==(const Places& p) const {
  if (!IsDefined() && !p.IsDefined()) {
    return true;
  }
  if (IsDefined() != p.IsDefined()) {
    return false;
  }
  if ((GetNoValues() == p.GetNoValues()) && (GetLength() == p.GetLength())) {
    base val1, val2;
    for (int i = 0; i < GetNoValues(); i++) {
      GetValue(i, val1);
      p.GetValue(i, val2);
      if (val1 != val2) {
        return false;
      }
    }
    return true;
  }
  return false;
}

/*
\subsection{Function ~Property~}

*/
ListExpr Places::Property() {
  return gentc::GenProperty("-> DATA", BasicType(),
    "((<label> <int>) (<label> <int>) ...)",
    "((\'Dortmund\' 1909) (\'Mailand\' 1899))");
}

/*
\subsection{Function ~CheckKind~}

*/
bool Places::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, Places::BasicType());
}

/*
\subsection{Function ~GetFLOB~}

*/
Flob* Places::GetFLOB(const int i) {
  assert((i >= 0) && (i < 2));
  return (i == 0 ? &values : &posref);
}

/*
\subsection{Function ~Compare~}

*/
int Places::Compare(const Attribute* arg) const {
  if (GetNoValues() == ((Places*)arg)->GetNoValues()) {
    base val1, val2;
    for (int i = 0; i < GetNoValues(); i++) {
      GetValue(i, val1);
      ((Places*)arg)->GetValue(i, val2);
      int comp = val1.first.compare(val2.first);
      if (comp == 0) { // equal strings
        if (val1.second > val2.second) {
          return 1;
        }
        if (val1.second < val2.second) {
          return -1;
        }
      }
      return (comp > 0 ? 1 : -1);
    }
    return 0;
  }
  if (GetNoValues() > ((Places*)arg)->GetNoValues()) {
    return 1;
  }
  return -1;
}

/*
\subsection{Function ~HashValue~}

*/
size_t Places::HashValue() const {
  if (IsEmpty() || !IsDefined()) {
    return 0;
  }
  size_t result = 1;
  base value;
  for (int i = 0; i < GetNoValues(); i++) {
    GetValue(i, value);
    result *= value.first.length() * value.second;
  }
  return result;
}

/*
\subsection{Function ~ToListExpr~}

*/
ListExpr Places::ToListExpr(ListExpr typeInfo) {
  if (!IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  if (IsEmpty()) {
    return nl->Empty();
  }
  base val;
  GetValue(0, val);
  ListExpr res = nl->OneElemList(nl->TwoElemList(nl->TextAtom(val.first),
                                                 nl->IntAtom(val.second)));
  ListExpr last = res;
  for (int i = 1; i < GetNoValues(); i++) {
    GetValue(i, val);
    last = nl->Append(last, nl->TwoElemList(nl->TextAtom(val.first),
                                            nl->IntAtom(val.second)));
  }
  return res;
}

/*
\subsection{Function ~ReadFrom~}

*/
bool Places::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  Clean();
  if (listutils::isSymbolUndefined(LE)) {
    SetDefined(false);
    return true;
  }
  SetDefined(true);
  ListExpr rest = LE;
  while (!nl->IsEmpty(rest)) {
    if (!nl->HasLength(nl->First(rest), 2)) {
      SetDefined(false);
      return false;
    }
    string text;
    nl->WriteToString(text, nl->First(nl->First(rest)));
    Append(make_pair(text.substr(1, text.length() - 2),
                     nl->IntValue(nl->Second(nl->First(rest)))));
    rest = nl->Rest(rest);
  }
  return true;
}

/*
\subsection{Type Constructor}

*/
GenTC<Places> places;

/*
\section{Implementation of Class ~IBasic~}

\subsection{Constructors}

*/
template<class B>
IBasic<B>::IBasic(const Instant& inst, const B& val) : Intime<B>(inst, val) {
  SetDefined(inst.IsDefined() && val.IsDefined());
}

template<class B>
IBasic<B>::IBasic(const IBasic& rhs) : Intime<B>(rhs.instant, rhs.value) {
  SetDefined(rhs.IsDefined());
}

/*
\subsection{Function ~Property~}

*/
template<class B>
ListExpr IBasic<B>::Property() {
  if (BasicType() == "ilabel") {
    return gentc::GenProperty("-> DATA", BasicType(), "(<instant> <label>)",
                              "(\"2014-02-26\" \'Dortmund\')");
  }
  if (BasicType() == "iplace") {
    return gentc::GenProperty("-> DATA", BasicType(), "(<instant> <place>)",
                              "(\"2014-02-18\" (\'Dortmund\' 1909))");
  }
  return gentc::GenProperty("-> DATA", BasicType(), "Error: invalid type.", "");
}

/*
\subsection{Function ~CheckKind~}

*/
template<class B>
bool IBasic<B>::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, IBasic<B>::BasicType());
}

/*
\subsection{Function ~ToListExpr~}

*/
template<class B>
ListExpr IBasic<B>::ToListExpr(ListExpr typeInfo) {
  if (!Intime<B>::IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  return nl->TwoElemList(this->instant.ToListExpr(false), 
                         this->value.ToListExpr(nl->Empty()));
}

/*
\subsection{Function ~ReadFrom~}

*/
template<class B>
bool IBasic<B>::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  this->SetDefined(false);
  if (listutils::isSymbolUndefined(LE)) {
    return true;
  }
  if (!nl->HasLength(LE, 2)) {
    return false;
  }
  if (this->instant.ReadFrom(nl->First(LE), nl->Empty()) &&
      this->value.ReadFrom(nl->Second(LE), nl->Empty())) {
    this->SetDefined(this->instant.IsDefined() && this->value.IsDefined());
    return true;
  }
  return false;
}

/*
\subsection{Function ~Val~}

*/
template<class B>
void IBasic<B>::Val(B& result) const {
  if (!this->IsDefined()) {
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(&(this->value));
}

/*
\subsection{Type Constructor}

*/
GenTC<IBasic<Label> > ilabel;
GenTC<IBasic<Place> > iplace;

/*
\section{Implementation of Class ~IBasics~}

\subsection{Constructors}

*/
template<class B>
IBasics<B>::IBasics(const Instant& inst, const B& values) : 
                                                     Intime<B>(inst, values) {}

/*
\subsection{Function ~Property~}

*/
template<class B>
ListExpr IBasics<B>::Property() {
  if (B::BasicType() == Labels::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(), "(<instant> <labels>)",
      "(\"2014-02-18\" (\"Dortmund\" \"Mailand\"))");
  }
  if (B::BasicType() == Places::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(), "(<instant> <places>)",
      "(\"2014-02-18\" ((\"Dortmund\" 1909) (\"Mailand\" 1899)))");
  }
  return gentc::GenProperty("-> DATA", BasicType(), "Error: invalid type.", "");
}

/*
\subsection{Function ~CheckKind~}

*/
template<class B>
bool IBasics<B>::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, IBasics<B>::BasicType());
}

/*
\subsection{Function ~ToListExpr~}

*/
template<class B>
ListExpr IBasics<B>::ToListExpr(ListExpr typeInfo) {
  if (!this->IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  return nl->TwoElemList(this->instant.ToListExpr(false), 
                         this->value.ToListExpr(nl->Empty()));
}

/*
\subsection{Function ~ReadFrom~}

*/
template<class B>
bool IBasics<B>::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  this->SetDefined(false);
  if (listutils::isSymbolUndefined(LE)) {
    return true;
  }
  if (!nl->HasLength(LE, 2)) {
    return false;
  }
  if (this->instant.ReadFrom(nl->First(LE), nl->Empty()) &&
      this->value.ReadFrom(nl->Second(LE), nl->Empty())) {
    this->SetDefined(this->instant.IsDefined() && this->value.IsDefined());
    return true;
  }
  return false;
}

/*
\subsection{Function ~Val~}

*/
template<class B>
void IBasics<B>::Val(B& result) const {
  if (!this->IsDefined()) {
    result.SetDefined(false);
    return;
  }
  result = this->value;
}

/*
\subsection{Type Constructor}

*/
GenTC<IBasics<Labels> > ilabels;
GenTC<IBasics<Places> > iplaces;

/*
\section{Implementation of class ~UBasic~}

\subsection{Constructors}

*/
template<class B>
UBasic<B>::UBasic(const SecInterval &iv, const B& val)
                                            : ConstTemporalUnit<B>(iv, val) {
  SetDefined(iv.IsDefined() && val.IsDefined());
}

template<class B>
UBasic<B>::UBasic(const UBasic<B>& ub) : ConstTemporalUnit<B>(ub) {
  this->SetDefined(ub.IsDefined());
}

/*
\subsection{Operator ~==~}

*/
template<class B>
bool UBasic<B>::operator==(const UBasic<B>& rhs) const {
  return Compare(&rhs) == 0;
}

/*
\subsection{Function ~Property~}

*/
template<class B>
ListExpr UBasic<B>::Property() {
  if (B::BasicType() == Label::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(), "(<interval> <label>)",
      "((\"2014-02-18\" \"2014-02-19-13:00\" TRUE FALSE) \"Dortmund\")");
  }
  if (B::BasicType() == Place::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(), "(<interval> <place>)",
      "((\"2014-02-18\" \"2014-02-19-13:00\" TRUE FALSE) (\"Dortmund\" 1909))");
  }
  return gentc::GenProperty("-> DATA", BasicType(), "Error: invalid type.", "");
}

/*
\subsection{Function ~CheckKind~}

*/
template<class B>
bool UBasic<B>::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, UBasic<B>::BasicType());
}

/*
\subsection{Function ~ToListExpr~}

*/
template<class B>
ListExpr UBasic<B>::ToListExpr(ListExpr typeInfo) {
  if (!this->IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  return nl->TwoElemList(((SecInterval)this->timeInterval).ToListExpr
          (nl->Empty()), this->constValue.ToListExpr(nl->Empty()));
}

/*
\subsection{Function ~ReadFrom~}

*/
template<class B>
bool UBasic<B>::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  this->SetDefined(false);
  if (listutils::isSymbolUndefined(LE)) {
    return true;
  }
  if (!nl->HasLength(LE, 2)) {
    return false;
  }
  SecInterval iv(true);
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  if (iv.ReadFrom(nl->First(LE), sc->GetTypeExpr(SecInterval::BasicType())) &&
      this->constValue.ReadFrom(nl->Second(LE), nl->Empty())) {
    this->timeInterval = iv;
    this->SetDefined(this->timeInterval.IsDefined() && 
                     this->constValue.IsDefined());
    return true;
  }
  return false;
}

/*
\subsection{Function ~Initial~}

*/
template<class B>
void UBasic<B>::Initial(IBasic<B>& result) const {
  if (this->IsDefined()) {
    result.instant = this->timeInterval.start;
    result.value.CopyFrom(&(this->constValue));
    result.SetDefined(true);
  }
  else {
    result.SetDefined(false);
  }
}

/*
\subsection{Function ~Final~}

*/
template<class B>
void UBasic<B>::Final(IBasic<B>& result) const {
  if (this->IsDefined()) {
    result.instant = this->timeInterval.end;
    result.value.CopyFrom(&(this->constValue));
    result.SetDefined(true);
  }
  else {
    result.SetDefined(false);
  }
}

/*
\subsection{Type Constructor}

*/
GenTC<UBasic<Label> > ulabel;
GenTC<UBasic<Place> > uplace;

/*
\section{Implementation of class ~UPlace~}

\subsection{Constructors}

*/
template<class B>
UBasics<B>::UBasics(const SecInterval &iv, const B& values)
                                           : ConstTemporalUnit<B>(iv, values) {}

/*
\subsection{Function ~Property~}

*/
template<class B>
ListExpr UBasics<B>::Property() {
  if (B::BasicType() == Labels::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(), "(<interval> <labels>)",
      "((\"2014-02-18\" \"2014-02-19-13:00\" TRUE FALSE) (\"Dortmund\" "
      "\"Mailand\"))");
  }
  if (B::BasicType() == Places::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(), "(<interval> <places>)",
      "((\"2014-02-18\" \"2014-02-19-13:00\" TRUE FALSE) ((\"Dortmund\" 1909) "
      "(\"Mailand\" 1899)))");
  }
  return gentc::GenProperty("-> DATA", BasicType(), "Error: invalid type.", "");
}

/*
\subsection{Function ~CheckKind~}

*/
template<class B>
bool UBasics<B>::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, UBasics<B>::BasicType());
}

/*
\subsection{Function ~ToListExpr~}

*/
template<class B>
ListExpr UBasics<B>::ToListExpr(ListExpr typeInfo) {
  if (!this->IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  return nl->TwoElemList(((SecInterval)this->timeInterval).ToListExpr
          (nl->Empty()), this->constValue.ToListExpr(nl->Empty()));
}

/*
\subsection{Function ~ReadFrom~}

*/
template<class B>
bool UBasics<B>::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  this->SetDefined(false);
  if (listutils::isSymbolUndefined(LE)) {
    return true;
  }
  if (!nl->HasLength(LE, 2)) {
    return false;
  }
  SecInterval iv(true);
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  if (iv.ReadFrom(nl->First(LE), sc->GetTypeExpr(SecInterval::BasicType())) &&
      this->constValue.ReadFrom(nl->Second(LE), nl->Empty())) {
    this->timeInterval = iv;
    this->SetDefined(this->timeInterval.IsDefined() &&
                     this->constValue.IsDefined());
    return true;
  }
  return false;
}

/*
\subsection{Function ~Initial~}

*/
template<class B>
void UBasics<B>::Initial(IBasics<B>& result) const {
  if (this->IsDefined()) {
    result.instant = this->timeInterval.start;
    result.value = this->constValue;
    result.SetDefined(true);
  }
  else {
    result.SetDefined(false);
  }
}

/*
\subsection{Function ~Final~}

*/
template<class B>
void UBasics<B>::Final(IBasics<B>& result) const {
  if (this->IsDefined()) {
    result.instant = this->timeInterval.end;
    result.value = this->constValue;
    result.SetDefined(true);
  }
  else {
    result.SetDefined(false);
  }
}

/*
\subsection{Type Constructor}

*/
GenTC<UBasics<Labels> > ulabels;
GenTC<UBasics<Places> > uplaces;

/*
\section{Implementation of class ~MBasic~}

\subsection{Constructors}

*/
template<class B>
MBasic<B>::MBasic(const MBasic &mb) : Attribute(mb.IsDefined()) {
  if (IsDefined()) {
    values.copyFrom(mb.values);
    units.copyFrom(mb.units);
  }
}

/*
\subsection{Function ~Property~}

*/
template<class B>
ListExpr MBasic<B>::Property() {
  if (B::BasicType() == Label::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(),
      "((<interval> <label>) (<interval> <label>) ...)",
      "(((\"2014-02-27\" \"2014-02-27-09:48\" TRUE FALSE) \"Dortmund\") "
      "((\"2014-05-17\" \"2014-05-17-22:00\" TRUE FALSE) \"Berlin\"))");
  }
  if (B::BasicType() == Place::BasicType()) {
    return gentc::GenProperty("-> DATA", BasicType(),
      "((<interval> <place>) (<interval> <place>) ...)",
      "(((\"2014-02-27\" \"2014-02-27-09:48\" TRUE FALSE) (\"Dortmund\" 1909)) "
      "((\"2014-05-17\" \"2014-05-17-22:00\" TRUE FALSE) (\"Berlin\" 4)))");
  }
  return gentc::GenProperty("-> DATA", BasicType(), "Error: invalid type.", "");
}

/*
\subsection{Function ~GetFLOB~}

*/
template<class B>
Flob* MBasic<B>::GetFLOB(const int i) {
  assert((i >= 0) && (i < NumOfFLOBs()));
  return (i == 0 ? &values : &units);
}

/*
\subsection{Function ~KindCheck~}

*/
template<class B>
bool MBasic<B>::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, MBasic<B>::BasicType());
}

/*
\subsection{Function ~Compare~}

*/
template<class B>
int MBasic<B>::Compare(const Attribute* arg) const {
  if (GetNoComponents() > ((MBasic<B>*)arg)->GetNoComponents()) {
    return 1;
  }
  if (GetNoComponents() < ((MBasic<B>*)arg)->GetNoComponents()) {
    return -1;
  }
  UBasic<B> ub1(true), ub2(true);
  for (int i = 0; i < GetNoComponents(); i++) {
    Get(i, ub1);
    ((MBasic<B>*)arg)->Get(i, ub2);
    int comp = ub1.Compare(&ub2);
    if (comp != 0) {
      return comp;
    }
  }
  return 0;
}

/*
\subsection{Function ~HashValue~}

*/
template<class B>
size_t MBasic<B>::HashValue() const {
  if (!IsDefined() || IsEmpty()) {
    return 0;
  }
  typename B::unitelem unit;
  units.Get(0, unit);
  return values.getSize() * unit.iv.HashValue();
}

/*
\subsection{Function ~CopyFrom~}

*/
template<class B>
void MBasic<B>::CopyFrom(const Attribute *arg) {
  if (!arg->IsDefined()) {
    SetDefined(false);
    return;
  }
  SetDefined(true);
  values.copyFrom(((MBasic<B>*)arg)->values);
  units.copyFrom(((MBasic<B>*)arg)->units);
}

/*
\subsection{Function ~unitToListExpr~}

*/
template<class B>
ListExpr MBasic<B>::unitToListExpr(const int i) {
  assert(i < GetNoComponents());
  typename B::base value;
  GetValue(i, value);
  typename B::unitelem unit;
  units.Get(i, unit);
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  return nl->TwoElemList(unit.iv.ToListExpr(sc->GetTypeExpr(
                                                     SecInterval::BasicType())),
                         B::getList(value));
}

/*
\subsection{Function ~ToListExpr~}

*/
template<class B>
ListExpr MBasic<B>::ToListExpr(ListExpr typeInfo) {
  if (!IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  if (IsEmpty()) {
    return nl->Empty();
  }
  ListExpr result = nl->OneElemList(unitToListExpr(0));
  ListExpr last = result;
  for (int i = 1; i < GetNoComponents(); i++) {
    last = nl->Append(last, unitToListExpr(i));
  }
  return result;
}

/*
\subsection{Function ~ReadFrom~}

*/
template<class B>
bool MBasic<B>::readUnitFrom(ListExpr LE) {
  if (!nl->HasLength(LE, 2)) {
    return false;
  }
  SecInterval iv;
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  if (!iv.ReadFrom(nl->First(LE), sc->GetTypeExpr(SecInterval::BasicType()))) {
    return false;
  }
  typename B::unitelem unit(values.getSize());
  unit.iv = iv;
  string text;
  if (!B::readValueFrom(nl->Second(LE), text, unit)) {
    return false;
  }
  units.Append(unit);
  const char *bytes = (text.substr(1, text.length() - 2)).c_str();
  SmiSize sz = strlen(bytes) + 1;
  if (sz > 0) {
    assert(bytes[sz - 1] == 0);
    values.write(bytes, sz, values.getSize());
  }
  else {
    char d = 0;
    values.write(&d, 1, values.getSize());
  }
  return true;
}

/*
\subsection{Function ~ReadFrom~}

*/
template<class B>
bool MBasic<B>::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  SetDefined(false);
  Clear();
  if (listutils::isSymbolUndefined(LE)) {
    return true;
  }
  ListExpr rest = LE;
  while (!nl->IsEmpty(rest)) {
    if (!readUnitFrom(nl->First(rest))) {
      return false;
    }
    rest = nl->Rest(rest);
  }
  SetDefined(true);
  return true;
}

/*
\subsection{Function ~Position~}

*/
template<class B>
int MBasic<B>::Position(const Instant& inst) const {
  assert(IsDefined());
  assert(inst.IsDefined());
  int first = 0, last = GetNoComponents() - 1;
  Instant t1 = inst;
  while (first <= last) {
    int mid = (first + last) / 2;
    if ((mid < 0) || (mid >= GetNoComponents())) {
      return -1;
    }
    typename B::unitelem unit;
    units.Get(mid, unit);
    if (((Interval<Instant>)(unit.iv)).Contains(t1)) {
      return mid;
    }
    else { // not contained
      if ((t1 > unit.iv.end) || (t1 == unit.iv.end)) {
        first = mid + 1;
      }
      else if ((t1 < unit.iv.start) || (t1 == unit.iv.start)) {
        last = mid - 1;
      }
      else {
        return -1; // should never be reached.
      }
    }
  }
  return -1;
}

/*
\subsection{Function ~Get~}

*/
template<class B>
void MBasic<B>::Get(const int i, UBasic<B>& result) const {
  assert((i >= 0) && (i < GetNoComponents()));
  result.SetDefined(IsDefined());
  if (IsDefined()) {
    typename B::unitelem unit;
    units.Get(i, unit);
    result.timeInterval = unit.iv;
    GetBasic(i, result.constValue);
  }
}

/*
\subsection{Function ~GetBasic~}

*/
template<class B>
void MBasic<B>::GetBasic(const int i, B& result) const {
  assert((i >= 0) && (i < GetNoComponents()));
  result.SetDefined(IsDefined());
  if (IsDefined()) {
    typename B::base value;
    GetValue(i, value);
    result.SetValue(value);
  }
}

/*
\subsection{Function ~getUnitEndPos~}

*/
template<class B>
unsigned int MBasic<B>::getUnitEndPos(const int i) const {
  assert((i >= 0) && (i < GetNoComponents()));
  if (i == GetNoComponents() - 1) {
    return values.getSize() - 1;
  }
  typename B::unitelem nextUnit;
  units.Get(i + 1, nextUnit);
  return nextUnit.pos - 1;
}

/*
\subsection{Function ~GetValue~}

*/
template<class B>
void MBasic<B>::GetValue(const int i, typename B::base& result) const {
  assert((i >= 0) && (i < GetNoComponents()));
  typename B::unitelem unit;
  units.Get(i, unit);
  unsigned int endPos = getUnitEndPos(i);
  char* bytes = new char[endPos - unit.pos + 1];
  values.read(bytes, endPos - unit.pos + 1, unit.pos);
  string text = bytes;
  B::buildValue(text, unit, result);
  delete[] bytes;
}

/*
\subsection{Function ~IsValid~}

*/
template<class B>
bool MBasic<B>::IsValid() const {
  if (!IsDefined() || IsEmpty()) {
    return true;
  }
  typename B::unitelem lastUnit, unit;
  units.Get(0, lastUnit);
  if (!lastUnit.iv.IsValid()) {
    return false;
  }
  if (GetNoComponents() == 1) {
    return true;
  }
  for (int i = 1; i < GetNoComponents(); i++) {
    units.Get(i, unit);
    if (!unit.iv.IsValid()) {
      return false;
    }
    if (lastUnit.iv.end > unit.iv.start) {
      return false;
    }
    if (!lastUnit.iv.Disjoint(unit.iv)) {
      return false;
    }
    lastUnit = unit;
  }
  return true;
}

/*
\subsection{Function ~EndBulkLoad~}

*/
template<class B>
void MBasic<B>::EndBulkLoad(const bool sort, const bool checkvalid) {
  if (!IsDefined()) {
    units.clean();
  }
  units.TrimToSize();
}

/*
\subsection{Function ~Add~}

*/
template<class B>
void MBasic<B>::Add(const UBasic<B>& ub) {
  assert(ub.IsDefined());
  assert(ub.IsValid());
  UBasic<B> ub2(ub);
  if (IsDefined()) {
    readUnitFrom(ub2.ToListExpr(nl->Empty()));
  }
}

/*
\subsection{Function ~MergeAdd~}

*/
template<class B>
void MBasic<B>::MergeAdd(const UBasic<B>& ub) {
  assert(IsDefined());
  if (!ub.IsDefined() || !ub.IsValid()) {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
      << " MergeAdd(Unit): Unit is undefined or invalid:";
    ub.Print(cout); cout << endl;
    assert(false);
  }
  if (!IsEmpty()) {
    bool extend = false;
    typename B::unitelem lastUnit;
    units.Get(GetNoComponents() - 1, lastUnit);
    if ((lastUnit.iv.end == ub.timeInterval.start) &&
        (lastUnit.iv.rc || ub.timeInterval.lc)) { // adjacent intervals
      typename B::base value1, value2;
      GetValue(GetNoComponents() - 1, value1);
      ub.constValue.GetValue(value2);
      if (value1 == value2) {
        lastUnit.iv.end = ub.timeInterval.end; // extend last interval
        lastUnit.iv.rc = ub.timeInterval.rc;
        units.Put(GetNoComponents() - 1, lastUnit);
        extend = true;
      }
    }
    if (!extend && lastUnit.iv.Before(ub.timeInterval)) {
      Add(ub);
    }
  }
  else { // first unit
    Add(ub);
  }
}

/*
\subsection{Function ~Passes~}

*/
template<class B>
bool MBasic<B>::Passes(const B& basic) const {
  if (!IsDefined() || !basic.IsDefined()) {
    return false;
  }
  typename B::base value1, value2;
  basic.GetValue(value2);
  for (int i = 0; i < GetNoComponents(); i++) {
    GetValue(i, value1);
    if (value1 == value2) {
      return true;
    }
  }
  return false;
}

/*
\subsection{Function ~At~}

*/
template<class B>
void MBasic<B>::At(const B& basic, MBasic<B>& result) const {
  result.Clear();
  if (!IsDefined() || !basic.IsDefined()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  typename B::base value1, value2;
  basic.GetValue(value2);
  typename B::unitelem unit;
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  for (int i = 0; i < GetNoComponents(); i++) {
    GetValue(i, value1);
    if (value1 == value2) {
      units.Get(i, unit);
      result.readUnitFrom(nl->TwoElemList(unit.iv.ToListExpr(sc->GetTypeExpr(
                                                     SecInterval::BasicType())),
                                          B::getList(value1)));
    }
  }
}

/*
\subsection{Function ~DefTime~}

*/
template<class B>
void MBasic<B>::DefTime(Periods& per) const {
  per.Clear();
  per.SetDefined(IsDefined());
  if (IsDefined()) {
    typename B::unitelem unit;
    for (int i = 0; i < GetNoComponents(); i++) {
      units.Get(i, unit);
      per.MergeAdd(unit.iv);
    }
  }
}

/*
\subsection{Function ~Atinstant~}

*/
template<class B>
void MBasic<B>::Atinstant(const Instant& inst, IBasic<B>& result) const {
  if(!IsDefined() || !inst.IsDefined()) {
    result.SetDefined(false);
    return;
  }
  int pos = this->Position(inst);
  if (pos == -1) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.value.SetDefined(true);
  typename B::unitelem unit;
  units.Get(pos, unit);
  GetBasic(pos, result.value);
  result.instant = unit.iv.start;
}

/*
\subsection{Function ~Initial~}

*/
template<class B>
void MBasic<B>::Initial(IBasic<B>& result) const {
  if (!IsDefined() || IsEmpty()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  typename B::unitelem unit;
  units.Get(0, unit);
  GetBasic(0, result.value);
  result.instant = unit.iv.start;
}

/*
\subsection{Function ~Final~}

*/
template<class B>
void MBasic<B>::Final(IBasic<B>& result) const {
  if (!IsDefined() || IsEmpty()) {
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  typename B::unitelem unit;
  units.Get(GetNoComponents() - 1, unit);
  GetBasic(GetNoComponents() - 1, result.value);
  result.instant = unit.iv.end;
}

/*
\subsection{Function ~Inside~}

*/
template<class B>
void MBasic<B>::Inside(const typename B::coll& coll, MBool& result) const {
  result.Clear();
  if (!IsDefined() || !coll.IsDefined()) {
    result.SetDefined(false);
    return;
  }
  typename B::unitelem unit;
  typename B::base value;
  for (int i = 0; i < GetNoComponents(); i++) {
    units.Get(i, unit);
    GetValue(i, value);
    CcBool res(true, coll.Contains(value));
    UBool ub(unit.iv, res);
    result.Add(ub);
  }
}

/*
\subsection{Function ~Fill~}

*/
template<class B>
void MBasic<B>::Fill(MBasic<B>& result, DateTime& dur) const {
  result.Clear();
  result.SetDefined(true);
  if (!IsDefined()) {
    result.SetDefined(false);
    return;
  }
  if (IsEmpty()) {
    return;
  }
  UBasic<B> unit, lastUnit;
  Get(0, lastUnit);
  for (int i = 1; i < GetNoComponents(); i++) {
    Get(i, unit);
    if ((lastUnit.constValue == unit.constValue) && 
        (unit.timeInterval.start - lastUnit.timeInterval.end <= dur)) {
      lastUnit.timeInterval.end = unit.timeInterval.end;
      lastUnit.timeInterval.rc = unit.timeInterval.rc;
    }
    else {
      result.MergeAdd(lastUnit);
      lastUnit = unit;
    }
  }
  result.MergeAdd(lastUnit);
}

/*
\subsection{Type Constructors}

*/
GenTC<MLabel> mlabel;
GenTC<MBasic<Place> > mplace;

/*
\section{Operator ~tolabel~}

\subsection{Type Mapping}

*/
ListExpr tolabelTM(ListExpr args) {
  if (nl->ListLength(args) == 1) {
    if (FText::checkType(nl->First(args)) ||
        CcString::checkType(nl->First(args))) {
      return nl->SymbolAtom(Label::BasicType());
    }
  }
  return NList::typeError("Expecting a text or a string.");
}

/*
\subsection{Selection Function}

*/
int tolabelSelect(ListExpr args) {
  return (FText::checkType(nl->First(args)) ? 0 : 1);
}

/*
\subsection{Value Mapping}

*/
template<class T>
int tolabelVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  result = qp->ResultStorage(s);
  T* src = static_cast<T*>(args[0].addr);
  Label* res = static_cast<Label*>(result.addr);
  if (src->IsDefined()) {
    res->Set(true, src->GetValue());
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct tolabelInfo : OperatorInfo {
  tolabelInfo() {
    name      = "tolabel";
    signature = "text -> label";
    appendSignature("string -> label");
    syntax    = "tolabel( _ );";
    meaning   = "Creates a label from a text or string.";
  }
};

/*
\section{Operator ~tostring~}

\subsection{Type Mapping}

*/
ListExpr tostringTM(ListExpr args) {
  if (nl->ListLength(args) == 1) {
    if (Label::checkType(nl->First(args))) {
      return nl->SymbolAtom(CcString::BasicType());
    }
  }
  return NList::typeError("Expecting a label.");
}

/*
\subsection{Value Mapping}

*/
int tostringVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  result = qp->ResultStorage(s);
  Label* source = static_cast<Label*>(args[0].addr);
  CcString* res = static_cast<CcString*>(result.addr);
  if (source->IsDefined()) {
    string text;
    source->GetValue(text),
    res->Set(true, text);
  }
  result.addr = res;
  return 0;
}

/*
\subsection{Operator Info}

*/
struct tostringInfo : OperatorInfo {
  tostringInfo() {
    name      = "tostring";
    signature = "label -> string";
    syntax    = "tostring ( _ )";
    meaning   = "Converts a label into a string.";
  }
};

/*
\section{Operator ~totext~}

\subsection{Type Mapping}

*/
ListExpr totextTM(ListExpr args) {
  if (nl->ListLength(args) == 1) {
    if (Label::checkType(nl->First(args))) {
      return nl->SymbolAtom(FText::BasicType());
    }
  }
  return NList::typeError("Expecting a label.");
}

/*
\subsection{Value Mapping}

*/
int totextVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  result = qp->ResultStorage(s);
  Label* source = static_cast<Label*>(args[0].addr);
  FText* res = static_cast<FText*>(result.addr);
  if (source->IsDefined()) {
    string text;
    source->GetValue(text);
    res->Set(true, text);
  }
  result.addr = res;
  return 0;
}

/*
\subsection{Operator Info}

*/
struct totextInfo : OperatorInfo {
  totextInfo() {
    name      = "totext";
    signature = "label -> text";
    syntax    = "totext ( _ )";
    meaning   = "Converts a label into a text.";
  }
};

/*
\section{Operator ~mstringtomlabel~}

\subsection{Type Mapping}

*/
ListExpr mstringtomlabelTM(ListExpr args) {
  if (nl->ListLength(args) == 1) {
    if (MString::checkType(nl->First(args))) {
      return nl->SymbolAtom(MLabel::BasicType());
    }
  }
  return NList::typeError("Expecting a mstring.");
}

/*
\subsection{Value Mapping}

*/
int mstringtomlabelVM(Word* args, Word& result, int message, Word& local,
                      Supplier s) {
  result = qp->ResultStorage(s);
  MString *source = static_cast<MString*>(args[0].addr);
  MLabel* res = static_cast<MLabel*>(result.addr);
  res->convertFromMString(*source);
  result.addr = res;
  return 0;
}

/*
\subsection{Operator Info}

*/
struct mstringtomlabelInfo : OperatorInfo {
  mstringtomlabelInfo() {
    name      = "mstringtomlabel";
    signature = "mstring -> mlabel";
    syntax    = "mstringtomlabel(_)";
    meaning   = "Converts a mstring into a mlabel.";
  }
};

/*
\section{Operator ~contains~}

contains: labels x label -> bool
contains: places x place -> bool

\subsection{Type Mapping}

*/
ListExpr containsTM(ListExpr args) {
  const string errMsg = "Expecting labels x label or places x place.";
  if (nl->ListLength(args) != 2) {
    return listutils::typeError("Two arguments expected.");
  }
  if ((Labels::checkType(nl->First(args)) && Label::checkType(nl->Second(args)))
 || (Places::checkType(nl->First(args)) && Place::checkType(nl->Second(args)))){
    return nl->SymbolAtom(CcBool::BasicType());
  }
  return NList::typeError(errMsg);
}

/*
\subsection{Selection Function}

*/
int containsSelect(ListExpr args) {
  if (Labels::checkType(nl->First(args))) return 0;
  if (Places::checkType(nl->First(args))) return 1;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class Collection, class Value>
int containsVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  Collection *coll = static_cast<Collection*>(args[0].addr);
  Value* val = static_cast<Value*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool* ccbool = static_cast<CcBool*>(result.addr);
  if (coll->IsDefined() && val->IsDefined()) {
    typename Value::base value;
    val->GetValue(value);
    ccbool->Set(true, coll->Contains(value));
  }
  else {
    ccbool->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct containsInfo : OperatorInfo {
  containsInfo() {
    name      = "contains";
    signature = "labels x label -> bool";
    appendSignature("places x place -> bool");
    syntax    = "_ contains _;";
    meaning   = "Checks whether a labels/places object contains a label/place.";
  }
};

/*
\section{Operator ~toplace~}

toplace: string x int -> place

\subsection{Type Mapping}

*/
ListExpr toplaceTM(ListExpr args) {
  if (nl->ListLength(args) == 2) {
    if ((CcString::checkType(nl->First(args)) || 
         FText::checkType(nl->First(args))) && 
        CcInt::checkType(nl->Second(args))) {
      return nl->SymbolAtom(Place::BasicType());
    }
  }
  return NList::typeError("Expecting ([string | text] x int).");
}

/*
\subsection{Selection Function}

*/
int toplaceSelect(ListExpr args) {
  if (CcString::checkType(nl->First(args))) return 0; 
  if (FText::checkType(nl->First(args))) return 1;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class T>
int toplaceVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  T *name = static_cast<T*>(args[0].addr);
  CcInt *ref = static_cast<CcInt*>(args[1].addr);
  result = qp->ResultStorage(s);
  Place *res = static_cast<Place*>(result.addr);
  if (name->IsDefined() && ref->IsDefined()) {
    res->Set(true, make_pair(name->GetValue(), ref->GetIntval()));
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct toplaceInfo : OperatorInfo {
  toplaceInfo() {
    name      = "toplace";
    signature = "[string | text] x int -> place";
    syntax    = "toplace( _ );";
    meaning   = "Converts a string/text and an int into a place.";
  }
};

/*
\section{Operator ~name~}

\subsection{Type Mapping}

*/
ListExpr nameTM(ListExpr args) {
  if (nl->ListLength(args) != 1) {
    return listutils::typeError("One argument expected.");
  }
  if (Place::checkType(nl->First(args))) {
    return nl->SymbolAtom(FText::BasicType());
  }
  return NList::typeError("Expecting a place.");
}

/*
\subsection{Value Mapping}

*/
int nameVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  Place *src = static_cast<Place*>(args[0].addr);
  result = qp->ResultStorage(s);
  FText* res = static_cast<FText*>(result.addr);
  if (src->IsDefined()) {
    pair<string, unsigned int> value;
    src->GetValue(value);
    res->Set(true, value.first);
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct nameInfo : OperatorInfo {
  nameInfo() {
    name      = "name";
    signature = "place -> text";
    syntax    = "name( _ );";
    meaning   = "Returns the name from a place object.";
  }
};

/*
\section{Operator ~name~}

\subsection{Type Mapping}

*/
ListExpr refTM(ListExpr args) {
  if (nl->ListLength(args) != 1) {
    return listutils::typeError("One argument expected.");
  }
  if (Place::checkType(nl->First(args))) {
    return nl->SymbolAtom(CcInt::BasicType());
  }
  return NList::typeError("Expecting a place.");
}

/*
\subsection{Value Mapping}

*/
int refVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  Place *src = static_cast<Place*>(args[0].addr);
  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*>(result.addr);
  if (src->IsDefined()) {
    res->Set(true, src->GetRef());
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct refInfo : OperatorInfo {
  refInfo() {
    name      = "ref";
    signature = "place -> int";
    syntax    = "ref( _ );";
    meaning   = "Returns the reference from a place object.";
  }
};

/*
\section{Implementation of class ~Pattern~}

\subsection{Function ~GetText~}

Returns the pattern text as specified by the user.

*/
string Pattern::GetText() const {
  return text.substr(0, text.length() - 1);
}

/*
\subsection{Function ~BasicType~}

*/
const string Pattern::BasicType() {
  return "pattern";
}

/*
\subsection{Function ~checkType~}

*/
const bool Pattern::checkType(const ListExpr type){
  return listutils::isSymbol(type, BasicType());
}

/*
\subsection{Function ~In~}

*/
Word Pattern::In(const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr& errorInfo, bool& correct) {
  Word result = SetWord(Address(0));
  correct = false;
  NList list(instance);
  if (list.isAtom()) {
    if (list.isText()) {
      string text = list.str();
      Pattern *pattern = getPattern(text);
      if (pattern) {
        correct = true;
        result.addr = pattern;
      }
      else {
        correct = false;
        cmsg.inFunError("Parsing error.");
      }
    }
    else {
      correct = false;
      cmsg.inFunError("Expecting a text!");
    }
  }
  else {
    correct = false;
    cmsg.inFunError("Expecting one text atom!");
  }
  return result;
}

/*
\subsection{Function ~Out~}

*/
ListExpr Pattern::Out(ListExpr typeInfo, Word value) {
  Pattern* pattern = static_cast<Pattern*>(value.addr);
  NList element(pattern->GetText(), true, true);
  return element.listExpr();
}

/*
\subsection{Function ~Create~}

*/
Word Pattern::Create(const ListExpr typeInfo) {
  return (SetWord(new Pattern()));
}

/*
\subsection{Function ~Delete~}

*/
void Pattern::Delete(const ListExpr typeInfo, Word& w) {
//   delete static_cast<Pattern*>(w.addr);
  w.addr = 0;
}

/*
\subsection{Function ~Close~}

*/
void Pattern::Close(const ListExpr typeInfo, Word& w) {
  delete static_cast<Pattern*>(w.addr);
  w.addr = 0;
}

/*
\subsection{Function ~Clone~}

*/
Word Pattern::Clone(const ListExpr typeInfo, const Word& w) {
  Pattern* pattern = static_cast<Pattern*>(w.addr);
  return SetWord(new Pattern(*pattern));
}

/*
\subsection{Function ~Open~}

*/
bool Pattern::Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value) {
  Pattern *p = (Pattern*)Attribute::Open(valueRecord, offset, typeInfo);
  value.setAddr(p);
  return true;
}

/*
\subsection{Function ~Save~}

*/
bool Pattern::Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value) {
  Pattern *p = (Pattern*)value.addr;
  Attribute::Save(valueRecord, offset, typeInfo, (Attribute*)p);
  return true;
}

/*
\subsection{Function ~SizeOfObj~}

*/
int Pattern::SizeOfObj() {
  return sizeof(Pattern);
}

/*
\subsection{Function ~KindCheck~}

*/
bool Pattern::KindCheck(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(type, Pattern::BasicType()));
}

/*
\subsection{Function ~Property~}

Describes the signature of the type constructor.

*/
ListExpr Pattern::Property() {
  return (nl->TwoElemList(
    nl->FiveElemList(nl->StringAtom("Signature"),
       nl->StringAtom("Example Type List"),
       nl->StringAtom("List Rep"),
       nl->StringAtom("Example List"),
       nl->StringAtom("Remarks")),
    nl->FiveElemList(nl->StringAtom("-> DATA"),
       nl->StringAtom(Pattern::BasicType()),
       nl->StringAtom("<pattern>"),
       nl->TextAtom("\' (monday at_home) X () // X.start = 2011-01-01 \'"),
       nl->StringAtom("<pattern> must be a text."))));
}

/*
\subsection{Creation of the Type Constructor Instance}

*/
TypeConstructor patternTC(
  Pattern::BasicType(),               // name of the type in SECONDO
  Pattern::Property,                // property function describing signature
  Pattern::Out, Pattern::In,         // Out and In functions
  0, 0,                            // SaveToList, RestoreFromList functions
  Pattern::Create, Pattern::Delete,  // object creation and deletion
  0, 0,                            // object open, save
  Pattern::Close, Pattern::Clone,    // close, and clone
  0,                               // cast function
  Pattern::SizeOfObj,               // sizeof function
  Pattern::KindCheck );             // kind checking function

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

/*
\subsection{Function ~matches~}

Checks the pattern and the condition and (if no problem occurs) invokes the NFA
construction and the matching procedure.

*/
ExtBool Pattern::matches(MLabel *ml) {
/*  cout << nfa2String() << endl;*/
  Match *match = new Match(this, ml);
  ExtBool result = UNDEF;
  if (initEasyCondOpTrees()) {
    result = match->matches();
  }
  delete match;
  return result;
}

/*
\subsection{Function ~filterTransitions~}

If this function is called with a second parameter, first ~nfaSimple~ is built
from it. Then, for each transition of the NFA from p, the function checks
whether it is viable according to the mlabel index. If not, it is erased in
both automata.

This function is only called if the mlabel provides an index.

*/
// void Match::filterTransitions(vector<map<int, int> > &nfaSimple,
//                               string regExSimple /* = "" */) {
//   if (!ml->hasIndex()) {
//     return;
//   }
//   if (!regExSimple.empty()) {
//     map<int, int>::iterator im;
//     IntNfa* intNfa = 0;
//     if (parsePatternRegEx(regExSimple.c_str(), &intNfa) != 0) {
//       return;
//     }
//     intNfa->nfa.makeDeterministic();
//     intNfa->nfa.minimize();
//     intNfa->nfa.bringStartStateToTop();
//     map<int, set<int> >::iterator it;
//     for (unsigned int i = 0; i < intNfa->nfa.numOfStates(); i++) {
//    map<int,set<int> > transitions = intNfa->nfa.getState(i).getTransitions();
//       map<int, int> newTrans;
//       for (it = transitions.begin(); it != transitions.end(); it++) {
//         newTrans[it->first] = *(it->second.begin());
//       }
//       nfaSimple.push_back(newTrans);
//     }
//     delete intNfa;
//   }
//   vector<map<int, int> >* nfaP = p->getNFA();
//   map<int, int>::iterator im;
//   TrieNode *ptr = 0;
//   for (unsigned int i = 0; i < nfaP->size(); i++) {
//     set<int> toErase;
//     for (im = (*nfaP)[i].begin(); im != (*nfaP)[i].end(); im++) {
//       set<string> labels = p->getElem(im->first).getL();
//       bool found = labels.empty();
//       set<string>::iterator is = labels.begin();
//       while ((is != labels.end()) && !found) {
//         if (!ml->index.find(ptr, *is).empty()) { // label occurs in mlabel
//           found = true;
//         }
//         is++;
//       }
//       if (!found) { // label does not occur in mlabel
//         toErase.insert(im->first);
//       }
//     }
//    for (set<int>::iterator it = toErase.begin(); it != toErase.end(); it++) {
//       p->eraseTransition(i, *it);
//       if (i < nfaSimple.size()) {
//         nfaSimple[i].erase(*it);
//       }
//     }
//   }
// }

/*
\subsection{Function ~reachesFinalState~}

Checks whether ~nfa~ has a path which reaches a final state.

*/
// bool Match::reachesFinalState(vector<map<int, int> > &nfa) {
//   set<int> finalStates = p->getFinalStates();
//   printNfa(*(p->getNFA()), finalStates);
// //   cout << "==================================================" << endl;
// //   printNfa(nfa, finalStates);
//   set<int> states;
//   states.insert(0);
//   map<int, int>::iterator im;
//   while (!states.empty()) {
//     set<int> newStates;
//     for (set<int>::iterator it = states.begin(); it != states.end(); it++) {
//       map<int, int> trans = nfa[*it];
//       for (im = trans.begin(); im != trans.end(); im++) {
//         if (finalStates.count(im->second)) {
//        cout << "final state " << im->second << " reached => Viable." << endl;
//           return true;
//         }
//         newStates.insert(im->second);
//       }
//     }
//     states = newStates;
//   }
//   cout << "final state(s) inaccessible" << endl;
//   return false;
// }

/*
\subsection{Function ~isViable~}

Checks whether a final state can be reached in a NFA, according to a mlabel
index.

*/
// bool Match::nfaIsViable() {
//   string regExSimple = p->getRegEx();
//   simplifyRegEx(regExSimple);
//   vector<map<int, int> > nfaSimple;
//   filterTransitions(nfaSimple, regExSimple);
//   return reachesFinalState(nfaSimple);
// }

/*
\subsection{Function ~match~}

Loops through the MLabel calling updateStates() for every ULabel. True is
returned if and only if the final state is an element of currentStates after
the loop.

*/
ExtBool Match::matches() {
  if (p->getNFA()->empty()) {
    return UNDEF;
  }
  set<int> states;
  states.insert(0);
  if (!p->hasConds() && !p->hasAssigns()) {
    for (int i = 0; i < ml->GetNoComponents(); i++) {
      if (!updateStates(i, p->nfa, p->elems, p->finalStates, states,
                        p->easyConds, p->easyCondPos, p->atomicToElem)) {
//           cout << "mismatch at unit label " << i << endl;
        return FALSE;
      }
    }
    if (!p->containsFinalState(states)) {
//         cout << "no final state is active" << endl;
      return FALSE;
    }
  }
  else {
    createSetMatrix(ml->GetNoComponents(), p->elemToVar.size());
    for (int i = 0; i < ml->GetNoComponents(); i++) {
      if (!updateStates(i, p->nfa, p->elems, p->finalStates, states,
                        p->easyConds, p->easyCondPos, p->atomicToElem, true)){
//           cout << "mismatch at unit label " << i << endl;
        ::deleteSetMatrix(matching, ml->GetNoComponents());
        return FALSE;
      }
    }
    if (!p->containsFinalState(states)) {
//         cout << "no final state is active" << endl;
      ::deleteSetMatrix(matching, ml->GetNoComponents());
      return FALSE;
    }
    if (!p->initCondOpTrees()) {
      ::deleteSetMatrix(matching, ml->GetNoComponents());
      return UNDEF;
    }
    if (!p->hasAssigns()) {
      bool result = findMatchingBinding(p->nfa, 0, p->elems, p->conds, 
                                        p->atomicToElem, p->elemToVar);
      ::deleteSetMatrix(matching, ml->GetNoComponents());
      return (result ? TRUE : FALSE);
    }
    return TRUE; // happens iff rewrite is called
  }
  return TRUE;
}

/*
\subsection{Function ~indexMatch~}

Recursively decides whether a pattern matches a mlabel with an index.

TODO: A LOT!

*/
// bool Match::indexMatch(StateWithULs swuOld) {
//   cout << "indexMatch invoked with state " << swuOld.state
//     << " range start " << swuOld.rangeStart << " and " << swuOld.items.size()
//        << " items" << endl;
//   map<int, int> transitions = p->getTransitions(swuOld.state);
//   if (transitions.empty() || swuOld.mismatch(ml->GetNoComponents())) {
//     return false;
//   }
//   map<int, int>::reverse_iterator im;
//   set<size_t>::iterator it;
//   ULabel ul(0);
//   StateWithULs swu;
//   TrieNode *ptr = 0;
//   for (im = transitions.rbegin(); im != transitions.rend(); im++) {
//     swu.clear();
//     Wildcard w = p->elems[im->first].getW();
//     set<string> ivs = p->elems[im->first].getI();
//     set<string> labels = p->elems[im->first].getL();
//     cout << "pattern element " << im->first << " has " << labels.size()
//          << " labels" << endl;
//    or (set<string>::iterator st = labels.begin(); st != labels.end(); st++) {
//       set<size_t> labelPos = ml->index.find(ptr, *st); // (... ...)
//       for (it = labelPos.begin(); it != labelPos.end(); it++) {
//         if (swuOld.isActive((unsigned int)*it)) {
//           ml->Get(*it, ul);
//           if (timesMatch(&ul.timeInterval, ivs) /*&& easyCondsMatch(...)*/) {
//             swu.items.insert(*it + 1);
//             swu.state = im->second;
//             cout << "ul " << *it + 1 << " and state " << im->second
//                  << " are active now" << endl;
//           }
//         }
//       }
//     }
//     if (labels.empty()) {
//       if (w == NO) { // (... _)
//         if (ivs.empty()) { // no time information
//           swu.activateNextItems(swuOld);
//         }
//         else { // time information exists
//           if (swuOld.rangeStart != UINT_MAX) {
//             for (int i = swuOld.rangeStart; i < ml->GetNoComponents(); i++) {
//               ml->Get(i, ul);
//               if (timesMatch(&ul.timeInterval, ivs)) {
//                 swu.items.insert(i + 1);
//               }
//             }
//           }
//           set<unsigned int>::iterator it;
//           for (it = swuOld.items.begin(); it != swuOld.items.end(); it++) {
//             ml->Get(*it, ul);
//             if (timesMatch(&ul.timeInterval, ivs)) {
//               swu.items.insert(*it + 1);
//             }
//           }
//         }
//       }
//       else { // either + or *
// 
//       }
//     }
//    if (swu.match(p->getFinalStates(), (unsigned int)ml->GetNoComponents())) {
//       return true;
//     }
//     if (indexMatch(swu)) {
//       return true;
//     }
//   }
//   return false;
// }

/*
\subsection{Function ~states2Str~}

Writes the set of currently active states into a string.

*/
string Match::states2Str(int ulId, set<int> &states) {
  stringstream result;
  if (!states.empty()) {
    set<int>::iterator it = states.begin();
    result << "after ULabel # " << ulId << ", active states are {" << *it;
    it++;
    while (it != states.end()) {
      result << ", " << *it;
      it++;
    }
    result << "}" << endl;
  }
  else {
    result << "after ULabel # " << ulId << ", there is no active state" << endl;
  }
  return result.str();
}

/*
\subsection{Function ~matchings2Str~}

Writes the matching table into a string.

*/
string Match::matchings2Str(unsigned int dim1, unsigned int dim2) {
  stringstream result;
  for (unsigned int i = 0; i < dim1; i++) {
    for (unsigned int j = 0; j < dim2; j++) {
      if (matching[i][j].empty()) {
        result << "                    ";
      }
      else {
        string cell;
        set<unsigned int>::iterator it, it2;
        for (it = matching[i][j].begin(); it != matching[i][j].end(); it++) {
          it2 = it;
          it2++;
          cell += int2Str(*it) + (it2 != matching[i][j].end() ? "," : "");
        }
        result << cell;
        for (unsigned int k = 20; k > cell.size(); k--) {
          result << " ";
        }
      }
    }
    result << endl;
  }
  return result.str();
}

/*
\subsection{Function ~updateStates~}

Applies the NFA. Each valid transaction is processed. If ~store~ is true,
each matching is stored.

*/

bool Match::updateStates(int ulId, vector<map<int, int> > &nfa,
             vector<PatElem> &elems, set<int> &finalStates, set<int> &states,
             vector<Condition> &easyConds, map<int, set<int> > &easyCondPos,
             map<int, int> &atomicToElem, bool store /* = false */) {
  set<int>::iterator its;
  set<unsigned int>::iterator itu;
  map<int, int> transitions;
  for (its = states.begin(); its != states.end(); its++) { // collect possible
    map<int, int> trans = nfa[*its];                       // transitions
    transitions.insert(trans.begin(), trans.end());
  }
  if (transitions.empty()) {
    return false;
  }
  states.clear();
  map<int, int>::iterator itm, itn;
  ULabel ul(true);
  ml->Get(ulId, ul);
  string text;
  ul.constValue.GetValue(text);
  if (store) {
    if (ulId < ml->GetNoComponents() - 1) { // usual case
      for (itm = transitions.begin(); itm != transitions.end(); itm++) {
        if (labelsMatch(text, elems[itm->first].getL())
         && timesMatch(&ul.timeInterval, elems[itm->first].getI())
         && easyCondsMatch(ulId, itm->first, elems[itm->first], easyConds,
                           easyCondPos[itm->first])) {
          states.insert(states.end(), itm->second);
          map<int, int> nextTrans = nfa[itm->second];
          for (itn = nextTrans.begin(); itn != nextTrans.end(); itn++) {
            itu = matching[ulId][atomicToElem[itm->first]].end();
            matching[ulId][atomicToElem[itm->first]].insert
                               (itu, atomicToElem[itn->first]);// store matching
          }
        }
      }
    }
    else { // last row; mark final states with -1
      for (itm = transitions.begin(); itm != transitions.end(); itm++) {
        if (labelsMatch(text, elems[itm->first].getL())
         && timesMatch(&ul.timeInterval, elems[itm->first].getI())
         && easyCondsMatch(ulId, itm->first, elems[itm->first], easyConds,
                           easyCondPos[itm->first])) {
          states.insert(states.end(), itm->second);
          if (finalStates.count(itm->second)) { // store last matching
            matching[ulId][atomicToElem[itm->first]].insert(UINT_MAX);
          }
        }
      }
    }
  }
  else {
    for (itm = transitions.begin(); itm != transitions.end(); itm++) {
      if (labelsMatch(text, elems[itm->first].getL())
       && timesMatch(&ul.timeInterval, elems[itm->first].getI())
       && easyCondsMatch(ulId, itm->first, elems[itm->first], easyConds,
                         easyCondPos[itm->first])){
        states.insert(states.end(), itm->second);
      }
    }
  }
  return !states.empty();
}

/*
\subsection{Function ~cleanPaths~}

Deletes all paths inside ~matching~ which do not end at a final state.

*/
void Match::cleanPaths(map<int, int> &atomicToElem) {
  map<int, int> transitions = p->getTransitions(0);
  map<int, int>::reverse_iterator itm;
  for (itm = transitions.rbegin(); itm != transitions.rend(); itm++) {
    cleanPath(0, atomicToElem[itm->first]);
  }
}

/*
\subsection{Function ~findMatchingBinding~}

Searches for a binding which fulfills every condition.

*/
bool Match::findMatchingBinding(vector<map<int, int> > &nfa, int startState,
                                vector<PatElem> &elems,vector<Condition> &conds,
                                map<int, int> &atomicToElem,
                                map<int, string> &elemToVar) {
  if ((startState < 0) || (startState > (int)nfa.size() - 1)) {
    return false; // illegal start state
  }
  if (conds.empty()) {
    return true;
  }
  map<int, int> transitions = nfa[startState];
  map<string, pair<unsigned int, unsigned int> > binding;
  map<int, int>::reverse_iterator itm;
  for (itm = transitions.rbegin(); itm != transitions.rend(); itm++) {
    if (findBinding(0, atomicToElem[itm->first], elems, conds, elemToVar,
                    binding)) {
      return true;
    }
  }
  return false;
}

/*
\subsection{Function ~findBinding~}

Recursively finds all bindings in the matching set matrix and checks whether
they fulfill every condition, stopping immediately after the first success.

*/
bool Match::findBinding(unsigned int ulId, unsigned int pId,
                        vector<PatElem> &elems, vector<Condition> &conds,
                        map<int, string> &elemToVar,
                      map<string, pair<unsigned int, unsigned int> > &binding) {
  string var = elemToVar[pId];
  bool inserted = false;
  if (!var.empty()) {
    if (binding.count(var)) { // extend existing binding
      binding[var].second++;
    }
    else { // add new variable
      binding[var] = make_pair(ulId, ulId);
      inserted = true;
    }
  }
  if (*(matching[ulId][pId].begin()) == UINT_MAX) { // complete match
    if (conditionsMatch(conds, binding)) {
      return true;
    }
  }
  else {
    for (set<unsigned int>::reverse_iterator it = matching[ulId][pId].rbegin();
         it != matching[ulId][pId].rend(); it++) {
      if (findBinding(ulId + 1, *it, elems, conds, elemToVar, binding)) {
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
\subsection{Function ~cleanPath~}

Recursively deletes all paths starting from (ulId, pId) that do not end at a
final state.

*/
bool Match::cleanPath(unsigned int ulId, unsigned int pId) {
//   cout << "cleanPaths called, ul " << ulId << ", pE " << pId << endl;
  if (matching[ulId][pId].empty()) {
    return false;
  }
  if (*(matching[ulId][pId].begin()) == UINT_MAX) {
    return true;
  }
  bool result = false;
  set<unsigned int>::iterator it;
  vector<unsigned int> toDelete;
  for (it = matching[ulId][pId].begin(); it != matching[ulId][pId].end(); it++){
    if (cleanPath(ulId + 1, *it)) {
      result = true;
    }
    else {
      toDelete.push_back(*it);
    }
  }
  for (unsigned int i = 0; i < toDelete.size(); i++) {
    matching[ulId][pId].erase(toDelete[i]);
  }
  return result;
}

void Match::printBinding(map<string, pair<unsigned int, unsigned int> > &b) {
  map<string, pair<unsigned int, unsigned int> >::iterator it;
  for (it = b.begin(); it != b.end(); it++) {
    cout << it->first << " --> [" << it->second.first << ","
         << it->second.second << "]  ";
  }
  cout << endl;
}

/*
\subsection{Function ~easyCondsMatch~}

*/
bool Match::easyCondsMatch(int ulId, int pId, PatElem const &up,
                           vector<Condition> &easyConds, set<int> &pos) {
  if (up.getW() || pos.empty()) {
    return true;
  }
  map<string, pair<unsigned int, unsigned int> > binding;
  binding[up.getV()] = make_pair(ulId, ulId);
  for (set<int>::iterator it = pos.begin(); it != pos.end(); it++) {
    if (!evaluateCond(easyConds[*it], binding)) {
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~conditionsMatch~}

Checks whether the specified conditions are fulfilled. The result is true if
and only if there is (at least) one binding that matches every condition.

*/
bool Match::conditionsMatch(vector<Condition> &conds,
                const map<string, pair<unsigned int, unsigned int> > &binding) {
  if (!ml->GetNoComponents()) { // empty MLabel
    return evaluateEmptyML();
  }
  for (unsigned int i = 0; i < conds.size(); i++) {
    map<string, pair<unsigned int, unsigned int> > b = binding;
    if (!evaluateCond(conds[i], binding)) {
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~evaluateEmptyML~}

This function is invoked in case of an empty moving label (i.e., with 0
components). A match is possible for a pattern like 'X [*] Y [*]' and conditions
X.card = 0, X.card = Y.card [*] 7. Time or label constraints are invalid.

*/
bool Match::evaluateEmptyML() {
  Word res;
  for (unsigned int i = 0; i < p->conds.size(); i++) {
    for (int j = 0; j < p->conds[i].getVarKeysSize(); j++) {
      if (p->conds[i].getKey(j) != 4) { // only card conditions possible
        cout << "Error: Only cardinality conditions allowed" << endl;
        return false;
      }
      p->conds[i].setCardPtr(j, 0);
    }
    p->conds[i].getQP()->EvalS(p->conds[i].getOpTree(), res, OPEN);
    if (!((CcBool*)res.addr)->IsDefined() || !((CcBool*)res.addr)->GetValue()) {
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~evaluateCond~}

This function is invoked by ~conditionsMatch~ and checks whether a binding
matches a certain condition.

*/
bool Match::evaluateCond(Condition &cond,
                const map<string, pair<unsigned int, unsigned int> > &binding) {
  Word qResult;
  ULabel ul;
  unsigned int from, to;
  for (int i = 0; i < cond.getVarKeysSize(); i++) {
    string var = cond.getVar(i);
    if (binding.count(var)) {
      from = binding.find(var)->second.first;
      to = binding.find(var)->second.second;
      string text;
      switch (cond.getKey(i)) {
        case 0: { // label
          ml->Get(from, ul);
          ul.constValue.GetValue(text);
          cond.setLabelPtr(i, text);
          break;
        }
        case 1: { // time
          cond.clearTimePtr(i);
          for (unsigned int j = from; j <= to; j++) {
            ml->Get(j, ul);
            cond.mergeAddTimePtr(i, ul.timeInterval);
          }
          break;
        }
        case 2: { // start
          ml->Get(from, ul);
          cond.setStartEndPtr(i, ul.timeInterval.start);
          break;
        }
        case 3: { // end
          ml->Get(to, ul);
          cond.setStartEndPtr(i, ul.timeInterval.end);
          break;
        }
        case 4: { // leftclosed
          ml->Get(from, ul);
          cond.setLeftRightclosedPtr(i, ul.timeInterval.lc);
          break;
        }
        case 5: { // rightclosed
          ml->Get(to, ul);
          cond.setLeftRightclosedPtr(i, ul.timeInterval.rc);
          break;
        }
        case 6: { // card
          cond.setCardPtr(i, to - from + 1);
          break;
        }
        default: { // labels
          cond.cleanLabelsPtr(i);
          for (unsigned int j = from; j <= to; j++) {
            Label label(true);
            ml->GetBasic(j, label);
            cond.appendToLabelsPtr(i, label);
          }
        }
      }
    }
    else { // variable bound to empty sequence
      switch (cond.getKey(i)) {
        case 6: {
          cond.setCardPtr(i, 0);
          break;
        }
        case 7: {
          cond.cleanLabelsPtr(i);
          break;
        }
        default: { // no other attributes allowed
          return false;
        }
      }
    }
  }
  cond.getQP()->EvalS(cond.getOpTree(), qResult, OPEN);
  return ((CcBool*)qResult.addr)->GetValue();
}

string Condition::getType(int t) {
  switch (t) {
    case 0: return ".label";
    case 1: return ".time";
    case 2: return ".start";
    case 3: return ".end";
    case 4: return ".leftclosed";
    case 5: return ".rightclosed";
    case 6: return ".card";
    case 7: return ".labels";
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

void Condition::setLabelPtr(unsigned int pos, string value) {
  if (pos < pointers.size()) {
    ((CcString*)pointers[pos])->Set(true, value);
  }
}

void Condition::clearTimePtr(unsigned int pos) {
  if (pos < pointers.size()) {
    if (((Periods*)pointers[pos])->IsDefined()) {
      ((Periods*)pointers[pos])->Clear();
    }
  }
}

void Condition::mergeAddTimePtr(unsigned int pos, SecInterval value) {
  if (pos < pointers.size()) {
    ((Periods*)pointers[pos])->MergeAdd(value);
  }
}

void Condition::setStartEndPtr(unsigned int pos, Instant value) {
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

void Condition::appendToLabelsPtr(unsigned int pos, Label value) {
  if (pos < pointers.size()) {
    ((Labels*)pointers[pos])->Append(value);
  }
}

void Condition::setLeftRightclosedPtr(unsigned int pos, bool value) {
  if (pos < pointers.size()) {
    ((CcBool*)pointers[pos])->Set(true, value);
  }
}

string Assign::getDataType(int key) {
  switch (key) {
    case -1: return CcBool::BasicType();
    case 0: return CcString::BasicType();
    case 1: return SecInterval::BasicType();
    case 2: 
    case 3: return Instant::BasicType();
    case 4:
    case 5: return CcBool::BasicType();
    default: return "error";
  }
}

/*
\subsection{Function ~getPointer~}

Static function invoked by ~initCondOpTrees~ or ~initAssignOpTrees~

*/
pair<string, Attribute*> Pattern::getPointer(int key) {
  pair<string, Attribute*> result;
  switch (key) {
    case 0: { // label, type CcString
      result.second = new CcString(false, "");
      result.first = "[const string pointer "
                   + nl->ToString(listutils::getPtrList(result.second)) + "]";
      break;
    }
    case 1: { // time, type Periods
      result.second = new Periods(1);
      result.first = "[const periods pointer "
                   + nl->ToString(listutils::getPtrList(result.second)) + "]";
      break;
    }
    case 2:   // start, type Instant
    case 3: { // end, type Instant
      result.second = new DateTime(instanttype);
      result.first = "[const instant pointer "
                   + nl->ToString(listutils::getPtrList(result.second)) + "]";
      break;
    }
    case 4:   // leftclosed, type CcBool
    case 5: { // rightclosed, type CcBool
      result.second = new CcBool(false);
      result.first = "[const bool pointer "
                   + nl->ToString(listutils::getPtrList(result.second)) + "]";
      break;
    }
    case 6: { // card, type CcInt
      result.second = new CcInt(false);
      result.first = "[const int pointer "
                   + nl->ToString(listutils::getPtrList(result.second)) + "]";
      break;
    }
    default: { // labels, type Labels
      result.second = new Labels(true);
      result.first = "[const labels pointer "
                   + nl->ToString(listutils::getPtrList(result.second)) + "]";
      break;
    }
    
  }
  return result;
}

/*
\subsection{Function ~processQueryStr~}

Invoked by ~initOpTrees~

*/
pair<QueryProcessor*, OpTree> Match::processQueryStr(string query, int type) {
  pair<QueryProcessor*, OpTree> result;
  result.first = 0;
  result.second = 0;
  SecParser parser;
  string qParsed;
  ListExpr qList, rType;
  bool correct(false), evaluable(false), defined(false), isFunction(false);
  if (parser.Text2List(query, qParsed)) {
    cout << "Text2List(" << query << ") failed" << endl;
    return result;
  }
  if (!nl->ReadFromString(qParsed, qList)) {
    cout << "ReadFromString(" << qParsed << ") failed" << endl;
    return result;
  }
  result.first = new QueryProcessor(nl, am);
  try {
    result.first->Construct(nl->Second(qList), correct, evaluable, defined,
                            isFunction, result.second, rType);
  }
  catch (...) {
    delete result.first;
    result.first = 0;
    result.second = 0;
    return result;
  }
  if (!correct || !evaluable || !defined) {
    cout << "correct:   " << (correct ? "TRUE" : "FALSE") << endl
         << "evaluable: " << (evaluable ? "TRUE" : "FALSE") << endl
         << "defined:   " << (correct ? "TRUE" : "FALSE") << endl;
    delete result.first;
    result.first = 0;
    result.second = 0;
    return result;
  }
  if (nl->ToString(rType) != Assign::getDataType(type)) {
    cout << "incorrect result type: " << nl->ToString(rType) << endl;
    delete result.first;
    result.first = 0;
    result.second = 0;
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
    pair<QueryProcessor*, OpTree> qp_optree = Match::processQueryStr(q, -1);
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
      pair<QueryProcessor*, OpTree> qp_optree = Match::processQueryStr(q, -1);
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
\section{Generic operators for ~[i|m|u] [label|place] [s]?~}

\subsection{Operator ~the\_unit~}

the\_unit: T x instant x instant x bool x bool -> uT,   with T in 
{label, labels, place, places}

\subsubsection{Type Mapping}

*/
ListExpr the_unitSymbolicTM(ListExpr args) {
  if (nl->Equal(nl->Rest(args), nl->FourElemList(
    nl->SymbolAtom(Instant::BasicType()), nl->SymbolAtom(Instant::BasicType()),
    nl->SymbolAtom(CcBool::BasicType()), nl->SymbolAtom(CcBool::BasicType())))){
    if (Label::checkType(nl->First(args))) {
      return nl->SymbolAtom(ULabel::BasicType());
    }
    if (Labels::checkType(nl->First(args))) {
      return nl->SymbolAtom(ULabels::BasicType());
    }
    if (Place::checkType(nl->First(args))) {
      return nl->SymbolAtom(UPlace::BasicType());
    }
    if (Places::checkType(nl->First(args))) {
      return nl->SymbolAtom(UPlaces::BasicType());
    }
  }
  return listutils::typeError(
    "Operator 'the_unit' expects a list with structure\n"
    "'(point point instant instant bool bool)', or \n"
    "'(ipoint ipoint bool bool)', or \n"
    "'(real real real bool instant instant bool bool)', or\n"
    "'(T instant instant bool bool)', or \n"
    "'(iT duration bool bool)'\n for T in "
    "{bool, int, string, label, labels, place, places}.");
}

/*
\subsubsection{Selection Function}

*/
int the_unitSymbolicSelect(ListExpr args) {
  if (Label::checkType(nl->First(args))) return 0;
  if (Labels::checkType(nl->First(args))) return 1;
  if (Place::checkType(nl->First(args))) return 2;
  if (Places::checkType(nl->First(args))) return 3;
  return -1;
}

/*
\subsubsection{Value Mapping}

*/
template<class Value, class Unit>
int the_unitSymbolicVM(Word* args, Word& result, 
                       int message, Word& local, Supplier s) {
  result = (qp->ResultStorage(s));
  Unit *res = static_cast<Unit*>(result.addr);
  Value *value = static_cast<Value*>(args[0].addr);
  Instant *i1 = static_cast<DateTime*>(args[1].addr);
  Instant *i2 = static_cast<DateTime*>(args[2].addr);
  CcBool *cl = static_cast<CcBool*>(args[3].addr);
  CcBool *cr = static_cast<CcBool*>(args[4].addr);
  bool clb, crb;
  if (!value->IsDefined() || !i1->IsDefined() || !i2->IsDefined() ||
      !cl->IsDefined() || !cr->IsDefined()) {
    res->SetDefined(false);
    return 0;
  }
  clb = cl->GetBoolval();
  crb = cr->GetBoolval();
  if (((*i1 == *i2) && (!clb || !crb)) || (i1->Adjacent(i2) && !(clb || crb))) {
    res->SetDefined(false); // illegal interval setting
    return 0;
  }
  if (*i1 < *i2) { // sorted instants
    Interval<Instant> iv(*i1, *i2, clb, crb);
    res->timeInterval = iv;
  }
  else {
    Interval<Instant> iv(*i2, *i1, clb, crb);
    res->timeInterval = iv;
  }
  res->constValue = *value;
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct the_unitSymbolicInfo : OperatorInfo {
  the_unitSymbolicInfo() {
    name      = "the_unit";
    signature = "label x instant x instant x bool x bool -> ulabel";
    appendSignature("labels x instant x instant x bool x bool -> ulabels");
    appendSignature("place x instant x instant x bool x bool -> uplace");
    appendSignature("places x instant x instant x bool x bool -> uplaces");
    syntax    = "the_unit( _ _ _ _ _ )";
    meaning   = "Creates a ulabel(s) / uplace(s) from its components.";
  }
};

/*
\subsection{Operator ~makemvalue~}

makemvalue: stream (tuple ((x1 t1)...(xi uT)...(xn tn))) xi -> mT,   with T in
{label, labels, place, places}

\subsubsection{Type Mapping}

*/
ListExpr makemvalueSymbolic_TM(ListExpr args) {
  ListExpr first, second, rest, listn,
           lastlistn, first2, second2, firstr, listfull, attrtype;
  int j;
  string argstr, argstr2, attrname, inputtype, inputname, fulllist;
  if (nl->ListLength(args) != 2) {
    return listutils::typeError("two arguments expected");
  }
  first = nl->First(args);
  nl->WriteToString(argstr, first);
  if (!listutils::isTupleStream(first)) {
    ErrorReporter::ReportError("Operator makemvalue expects a tuplestream as "
    "first argument, but gets '" + argstr + "'.");
    return nl->TypeError();
  }
  second  = nl->Second(args);
  nl->WriteToString(argstr, second);
  if(argstr == Symbol::TYPEERROR()){
    return listutils::typeError("invalid attrname" + argstr);
  }
  nl->WriteToString(inputname, second);
  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  firstr = nl->First(rest);
  rest = nl->Rest(rest);
  first2 = nl->First(firstr);
  second2 = nl->Second(firstr);
  nl->WriteToString(attrname, first2);
  nl->WriteToString(argstr2, second2);
  if (attrname == inputname) {
    inputtype = argstr2;
  }
  while (!(nl->IsEmpty(rest))) {
    lastlistn = nl->Append(lastlistn,nl->First(rest));
    firstr = nl->First(rest);
    rest = nl->Rest(rest);
    first2 = nl->First(firstr);
    second2 = nl->Second(firstr);
    nl->WriteToString(attrname, first2);
    nl->WriteToString(argstr2, second2);
    if (attrname == inputname) {
      inputtype = argstr2;
    }
  }
  rest = second;
  listfull = listn;
  nl->WriteToString(fulllist, listfull);
  if (inputtype == "") {
    return listutils::typeError("attribute not found");
  }
  if ((inputtype != ULabel::BasicType()) && (inputtype != ULabels::BasicType())
  && (inputtype != UPlace::BasicType()) && (inputtype != UPlaces::BasicType())){
    return listutils::typeError("attribute type not in {ulabel, ulabels, uplace"
                                ", uplaces}");
  }
  attrname = nl->SymbolValue(second);
  j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);
  assert(j != 0);
  if (inputtype == ULabel::BasicType()) {
    attrtype = nl->SymbolAtom(MLabel::BasicType());
  }
  else if (inputtype == ULabels::BasicType()) {
    attrtype = nl->SymbolAtom(MLabels::BasicType());
  }
  else if (inputtype == UPlace::BasicType()) {
    attrtype = nl->SymbolAtom(MPlace::BasicType());
  }
  else if (inputtype == UPlaces::BasicType()) {
    attrtype = nl->SymbolAtom(MPlaces::BasicType());
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
         nl->TwoElemList(nl->IntAtom(j),
         nl->StringAtom(nl->SymbolValue(attrtype))), attrtype);
}

/*
\subsubsection{Selection Function}

*/
int makemvalueSymbolicSelect(ListExpr args) {
  ListExpr arg = nl->Second(nl->First(nl->Second(nl->Second(nl->First(args)))));
  if (ULabel::checkType(arg)) return 0;
  if (ULabels::checkType(arg)) return 1;
  if (UPlace::checkType(arg)) return 2;
  if (UPlaces::checkType(arg)) return 3;
  return -1;
}

/*
\subsubsection{Value Mapping}

*/
template<class Unit, class Mapping>
int makemvalueSymbolicVM(Word* args, Word& result, int message,
                         Word& local, Supplier s) {
  Mapping* m;
  Unit* unit;
  Word curTupleWord;
  assert(args[2].addr != 0);
  assert(args[3].addr != 0);
  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, curTupleWord);
  result = qp->ResultStorage(s);
  m = (Mapping*)result.addr;
  m->Clear();
  m->SetDefined(true);
  m->StartBulkLoad();
  while (qp->Received(args[0].addr)) { // get all tuples
    Tuple* curTuple = (Tuple*)curTupleWord.addr;
    Attribute* curAttr = (Attribute*)curTuple->GetAttribute(attrIndex);
    if (curAttr == 0) {
      cout << endl << "ERROR in " << __PRETTY_FUNCTION__
           << ": received Nullpointer!" << endl;
      assert(false);
    }
    else if (curAttr->IsDefined()) {
      unit = static_cast<Unit*>(curAttr);
      m->MergeAdd(*unit);
    }
    else {
      cerr << endl << __PRETTY_FUNCTION__ << ": Dropping undef unit. " << endl;
    }
    curTuple->DeleteIfAllowed();
    qp->Request(args[0].addr, curTupleWord);
  }
  m->EndBulkLoad(true, true); // force Mapping to sort the units
  qp->Close(args[0].addr);    // and mark invalid Mapping as undefined
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct makemvalueSymbolicInfo : OperatorInfo {
  makemvalueSymbolicInfo() {
    name      = "makemvalue";
    signature = "stream (tuple ((x1 t1)...(xi uT)...(xn tn))) xi -> mT,   with"
                "T in {label, labels, place, places}";
    syntax    = "_ makemvalue[ _ ]";
    meaning   = "Creates a moving object from a (not necessarily sorted) tuple "
                "stream containing a ulabel(s) or uplace attribute. No two unit"
                " timeintervals may overlap. Undefined units are allowed and "
                "will be ignored. A stream without defined units will result in"
                " an \'empty\' moving object, not in an \'undef\'.";
  }
};

/*
\subsection{Operator ~passes~}

passes: mlabel x label -> bool
passes: mlabels x label -> bool
passes: mlabels x labels -> bool
passes: mplace x place -> bool
passes: mplaces x place -> bool
passes: mplaces x places -> bool

\subsubsection{Type Mapping}

*/
ListExpr passesSymbolicTM(ListExpr args) {
  if (nl->HasLength(args, 2)) {
    ListExpr first(nl->First(args)), second(nl->Second(args));
    if ((MLabel::checkType(first) && Label::checkType(second)) ||
        (MLabels::checkType(first) && Label::checkType(second)) ||
        (MLabels::checkType(first) && Labels::checkType(second)) ||
        (MPlace::checkType(first) && Place::checkType(second)) ||
        (MPlaces::checkType(first) && Place::checkType(second)) ||
        (MPlaces::checkType(first) && Places::checkType(second))) {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }
  return listutils::typeError("Correct signatures:  mlabel x label -> bool,   "
    "mlabels x label -> bool,   mlabels x labels -> bool,   "
    "mplace x place -> bool,   mplaces x place -> bool,   "
    "mplaces x places -> bool");
}

/*
\subsubsection{Selection Function}

*/
int atPassesSymbolicSelect(ListExpr args) {
  if (MLabel::checkType(nl->First(args))) return 0;
  if (Label::checkType(nl->Second(args))) return 1;
  if (Labels::checkType(nl->Second(args))) return 2;
  if (MPlace::checkType(nl->First(args))) return 3;
  if (Place::checkType(nl->Second(args))) return 4;
  if (Places::checkType(nl->Second(args))) return 5;
  return -1;
}

/*
\subsubsection{Value Mapping}

*/
template<class Mapping, class Value>
int passesSymbolicVM(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  Mapping *m = static_cast<Mapping*>(args[0].addr);
  Value *val = static_cast<Value*>(args[1].addr);
  if (m->IsDefined() && val->IsDefined()) {
    res->Set(true, m->Passes(*val));
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct passesSymbolicInfo : OperatorInfo {
  passesSymbolicInfo() {
    name      = "passes";
    signature = "mlabel x label -> bool";
    appendSignature("mlabels x label -> bool");
    appendSignature("mlabels x labels -> bool");
    appendSignature("mplace x place -> bool");
    appendSignature("mplaces x place -> bool");
    appendSignature("mplaces x places -> bool");
    syntax    = "_ passes _ ";
    meaning   = "Returns TRUE if and only if the label(s) / place(s) occur(s) "
                "at least once in the mlabel(s) / mplace(s).";
  }
};

/*
\subsection{Operator ~at~}

at: mlabel x label -> mlabel
at: mlabels x label -> mlabels
at: mlabels x labels -> mlabels
at: mplace x place -> mplace
at: mplaces x place -> mplaces
at: mplaces x places -> mplace

\subsubsection{Type Mapping}

*/
ListExpr atSymbolicTM(ListExpr args) {
  if (nl->HasLength(args, 2)) {
    ListExpr first(nl->First(args)), second(nl->Second(args));
    if (MLabel::checkType(first) && Label::checkType(second)) {
      return nl->SymbolAtom(MLabel::BasicType());
    }
    if ((MLabels::checkType(first) && Label::checkType(second)) ||
        (MLabels::checkType(first) && Labels::checkType(second))) {
      return nl->SymbolAtom(MLabels::BasicType());
    }
    if (MPlace::checkType(first) && Place::checkType(second)) {
      return nl->SymbolAtom(MPlace::BasicType());
    }
    if ((MPlaces::checkType(first) && Place::checkType(second)) ||
        (MPlaces::checkType(first) && Places::checkType(second))) {
      return nl->SymbolAtom(MPlaces::BasicType());
    }
  }
  return listutils::typeError("Correct signatures: mlabel x label -> mlabel,   "
    "mlabels x label -> mlabels,   mlabels x labels -> mlabels,   "
    "mplace x place -> mplace,   mplaces x place -> mplaces,   "
    "mplaces x places -> mplaces");
}

/*
\subsubsection{Value Mapping}

*/
template<class Mapping, class Value>
int atSymbolicVM(Word* args, Word& result, int message, Word& local,Supplier s){
  result = qp->ResultStorage(s);
  Mapping *src = static_cast<Mapping*>(args[0].addr);
  Value *val = static_cast<Value*>(args[1].addr);
  Mapping *res = static_cast<Mapping*>(result.addr);
  src->At(*val, *res);
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct atSymbolicInfo : OperatorInfo {
  atSymbolicInfo() {
    name      = "at";
    signature = "mlabel x label -> mlabel";
    appendSignature("mlabels x label -> mlabels");
    appendSignature("mlabels x labels -> mlabels");
    appendSignature("mplace x place -> mplace");
    appendSignature("mplaces x place -> mplace");
    appendSignature("mplaces x places -> mplaces");
    syntax    = "_ at _ ";
    meaning   = "Reduces the mlabel(s) / mplace(s) to those units whose "
                "label(s) / place(s) equals the label(s) / place(s).";
  }
};

/*
\subsection{Operator ~deftime~}

deftime: mlabel -> periods
deftime: mlabels -> periods
deftime: mplace -> periods
deftime: mplaces -> periods

\subsubsection{Type Mapping}

*/
ListExpr deftimeSymbolicTM(ListExpr args) {
  if (nl->HasLength(args, 1)) {
    ListExpr first = nl->First(args);
    if (MLabel::checkType(first) || MLabels::checkType(first) ||
        MPlace::checkType(first) || MPlaces::checkType(first)) {
      return nl->SymbolAtom(Periods::BasicType());
    }
  }
  return listutils::typeError("Correct signature: mlabel -> periods,    "
    "mlabels -> periods,   mplace -> periods,   mplaces -> periods");
}

/*
\subsubsection{Selection Function}

*/
int deftimeUnitsAtinstantSymbolicSelect(ListExpr args) {
  if (MLabel::checkType(nl->First(args))) return 0;
  if (MLabels::checkType(nl->First(args))) return 1;
  if (MPlace::checkType(nl->First(args))) return 2;
  if (MPlaces::checkType(nl->First(args))) return 3;
  return -1;
}

/*
\subsubsection{Value Mapping}

*/
template<class Mapping>
int deftimeSymbolicVM(Word* args, Word& result, int message, Word& local,
                      Supplier s) {
  result = qp->ResultStorage(s);
  Periods* res = static_cast<Periods*>(result.addr);
  Mapping *src = static_cast<Mapping*>(args[0].addr);
  src->DefTime(*res);
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct deftimeSymbolicInfo : OperatorInfo {
  deftimeSymbolicInfo() {
    name      = "deftime";
    signature = "mlabel -> periods";
    appendSignature("mlabels -> periods");
    appendSignature("mplace -> periods");
    appendSignature("mplaces -> periods");
    syntax    = "deftime ( _ )";
    meaning   = "Returns the periods containing the time intervals during which"
                "the mlabel(s) / mplace(s) is defined.";
  }
};

/*
\subsection{Operator ~atinstant~}

atinstant: mlabel x instant -> ilabel
atinstant: mlabels x instant -> ilabels
atinstant: mplace x instant -> iplace
atinstant: mplaces x instant -> iplaces

\subsubsection{Type Mapping}

*/
ListExpr atinstantSymbolicTM(ListExpr args) {
  if (nl->HasLength(args, 2)) {
    if (Instant::checkType(nl->Second(args))) {
      if (MLabel::checkType(nl->First(args))) {
        return nl->SymbolAtom(ILabel::BasicType());
      }
      if (MLabels::checkType(nl->First(args))) {
        return nl->SymbolAtom(ILabels::BasicType());
      }
      if (MPlace::checkType(nl->First(args))) {
        return nl->SymbolAtom(IPlace::BasicType());
      }
      if (MPlaces::checkType(nl->First(args))) {
        return nl->SymbolAtom(IPlaces::BasicType());
      }
    }
  }
  return listutils::typeError("Correct signature: mT x instant -> iT,   with "
    "T in {label(s), place(s)}");
}

/*
\subsubsection{Value Mapping}

*/
template<class Mapping, class Intime>
int atinstantSymbolicVM(Word* args, Word& result, int message, Word& local,
                        Supplier s) {
  result = qp->ResultStorage(s);
  Mapping *src = static_cast<Mapping*>(args[0].addr);
  Instant *inst = static_cast<Instant*>(args[1].addr);
  Intime *res = static_cast<Intime*>(result.addr);
  src->Atinstant(*inst, *res);
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct atinstantSymbolicInfo : OperatorInfo {
  atinstantSymbolicInfo() {
    name      = "atinstant";
    signature = "mlabel x instant -> ilabel";
    appendSignature("mlabels x instant -> ilabels");
    appendSignature("mplace x instant -> iplace");
    appendSignature("mplaces x instant -> iplaces");
    syntax    = "_ atinstant _";
    meaning   = "Gets the intime value from a moving object corresponding to "
                "the temporal value at the given instant.";
  }
};

/*
\subsection{Operator ~units~}

units: mlabel -> (stream ulabel)
units: mlabels -> (stream ulabels)
units: mplace -> (stream uplace)
units: mplaces -> (stream uplaces)

\subsubsection{Type Mapping}

*/
ListExpr unitsSymbolicTM(ListExpr args) {
  if (nl->HasLength(args, 1)) {
    if (MLabel::checkType(nl->First(args))) {
      return nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                             nl->SymbolAtom(ULabel::BasicType()));
    }
    if (MLabels::checkType(nl->First(args))) {
      return nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                             nl->SymbolAtom(ULabels::BasicType()));
    }
    if (MPlace::checkType(nl->First(args))) {
      return nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                             nl->SymbolAtom(UPlace::BasicType()));
    }
    if (MPlaces::checkType(nl->First(args))) {
      return nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                             nl->SymbolAtom(UPlaces::BasicType()));
    }
  }
  return listutils::typeError("Correct signatures: mT -> (stream uT), with "
    "T in {label(s), place(s)}");
}

/*
\subsubsection{Value Mapping}

*/
template<class Mapping, class Unit>
int unitsSymbolicVM(Word* args, Word& result, int message, Word& local,
                    Supplier s) {
  Mapping *source = static_cast<Mapping*>(args[0].addr);
  UnitsLI *li = static_cast<UnitsLI*>(local.addr);
  switch (message) {
    case OPEN: {
      if (li) {
        li = 0;
      }
      li = new UnitsLI();
      local.addr = li;
      return 0;
    }
    case REQUEST: {
      if (!local.addr) {
        result.addr = 0;
        return CANCEL;
      }
      li = (UnitsLI*)local.addr;
      if (li->index < source->GetNoComponents()) {
        Unit unit(true);
        source->Get(li->index, unit);
        li->index++;
        result = SetWord(unit.Clone());
        return YIELD;
      }
      else {
        return CANCEL;
      }
    }
    case CLOSE: {
      if (local.addr) {
        li = (UnitsLI*)local.addr;
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct unitsSymbolicInfo : OperatorInfo {
  unitsSymbolicInfo() {
    name      = "units";
    signature = "mT -> (stream uT), with T in {label(s), place(s)}";
    syntax    = "units ( _ )";
    meaning = "Splits a mlabel(s) / mplace(s) into its units and returns them "
              "as a stream.";
  }
};

/*
\subsection{Operator ~initial~}

initial: XT -> iT   with X in {u, m} and T in {label(s), place(s)}

\subsubsection{Type Mapping}

*/
ListExpr initialFinalSymbolicTM(ListExpr args) {
  if (nl->HasLength(args, 1)) {
    ListExpr first = nl->First(args);
    if (ULabel::checkType(first) || MLabel::checkType(first)) {
      return nl->SymbolAtom(ILabel::BasicType());
    }
    if (ULabels::checkType(first) || MLabels::checkType(first)) {
      return nl->SymbolAtom(ILabels::BasicType());
    }
    if (UPlace::checkType(first) || MPlace::checkType(first)) {
      return nl->SymbolAtom(IPlace::BasicType());
    }
    if (UPlaces::checkType(first) || MPlaces::checkType(first)) {
      return nl->SymbolAtom(IPlaces::BasicType());
    }
  }
  return listutils::typeError("Correct signature:  XT -> iT   with X in {u, m} "
    "and T in {label(s), place(s)}");
}

/*
\subsubsection{Selection Function}

*/
int initialFinalSymbolicSelect(ListExpr args) {
  if (ULabel::checkType(nl->First(args))) return 0;
  if (ULabels::checkType(nl->First(args))) return 1;
  if (MLabel::checkType(nl->First(args))) return 2;
  if (MLabels::checkType(nl->First(args))) return 3;
  if (UPlace::checkType(nl->First(args))) return 4;
  if (UPlaces::checkType(nl->First(args))) return 5;
  if (MPlace::checkType(nl->First(args))) return 6;
  if (MPlaces::checkType(nl->First(args))) return 7;
  return -1;
}

/*
\subsubsection{Value Mapping}

*/
template<class UnitMapping, class Intime>
int initialSymbolicVM(Word* args, Word& result, int message, Word& local,
                      Supplier s) {
  result = qp->ResultStorage(s);
  Intime* res = static_cast<Intime*>(result.addr);
  UnitMapping *src = static_cast<UnitMapping*>(args[0].addr);
  src->Initial(*res);
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct initialSymbolicInfo : OperatorInfo {
  initialSymbolicInfo() {
    name      = "initial";
    signature = "XT -> iT   with X in {u, m} and T in {label(s), place(s)}";
    syntax    = "initial ( _ )";
    meaning   = "Returns the ilabel(s) belonging to the initial instant of the "
                "ulabel(s) / mlabel(s) / uplace(s) / mplace(s).";
  }
};

/*
\subsection{Operator ~final~}

final: XT -> iT   with X in {u, m} and T in {label(s), place(s)}

\subsubsection{Value Mapping}

*/
template<class UnitMapping, class Intime>
int finalSymbolicVM(Word* args, Word& result, int message, Word& local, 
                    Supplier s) {
  result = qp->ResultStorage(s);
  Intime* res = static_cast<Intime*>(result.addr);
  UnitMapping *src = static_cast<UnitMapping*>(args[0].addr);
  src->Final(*res);
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct finalSymbolicInfo : OperatorInfo {
  finalSymbolicInfo() {
    name      = "final";
    signature = "XT -> iT   with X in {u, m} and T in {label(s), place(s)}";
    syntax    = "final ( _ )";
    meaning   = "Returns the ilabel(s) belonging to the final instant of the "
                "ulabel(s) / mlabel(s) / uplace(s) / mplace(s).";
  }
};

/*
\subsection{Operator ~val~}

val: ilabel -> label
val: ilabels -> labels
val: iplace -> place
val: iplaces -> places

\subsubsection{Type Mapping}

*/
ListExpr valSymbolicTM(ListExpr args) {
  if (nl->HasLength(args, 1)) {
    if (ILabel::checkType(nl->First(args))) {
      return nl->SymbolAtom(Label::BasicType());
    }
    if (ILabels::checkType(nl->First(args))) {
      return nl->SymbolAtom(Labels::BasicType());
    }
    if (IPlace::checkType(nl->First(args))) {
      return nl->SymbolAtom(Place::BasicType());
    }
    if (IPlaces::checkType(nl->First(args))) {
      return nl->SymbolAtom(Places::BasicType());
    }
  }
  return listutils::typeError("Correct signature:  ilabel -> label,   "
    "ilabels -> labels,   iplace -> place,   iplaces -> places");
}

/*
\subsubsection{Selection Function}

*/
int valInstSymbolicSelect(ListExpr args) {
  if (ILabel::checkType(nl->First(args))) return 0;
  if (ILabels::checkType(nl->First(args))) return 1;
  if (IPlace::checkType(nl->First(args))) return 2;
  if (IPlaces::checkType(nl->First(args))) return 3;
  return -1;
}

/*
\subsubsection{Value Mapping}

*/
template<class Intime, class L>
int valSymbolicVM(Word* args, Word& result, int message, Word& local, 
                  Supplier s) {
  result = qp->ResultStorage(s);
  L* res = static_cast<L*>(result.addr);
  Intime *src = static_cast<Intime*>(args[0].addr);
  src->Val(*res);
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct valSymbolicInfo : OperatorInfo {
  valSymbolicInfo() {
    name      = "val";
    signature = "ilabel -> label";
    appendSignature("ilabels -> labels");
    appendSignature("iplace -> place");
    appendSignature("iplaces -> places");
    syntax    = "val ( _ )";
    meaning   = "Returns the value of the ilabel(s) or the iplace(s).";
  }
};

/*
\subsection{Operator ~inst~}

inst: ilabel -> instant
inst: ilabels -> instant
inst: iplace -> instant
inst: iplaces -> instant

\subsubsection{Type Mapping}

*/
ListExpr instSymbolicTM(ListExpr args) {
  if (nl->HasLength(args, 1)) {
    if (ILabel::checkType(nl->First(args)) || 
        ILabels::checkType(nl->First(args)) ||
        IPlace::checkType(nl->First(args)) ||
        IPlaces::checkType(nl->First(args))) {
      return nl->SymbolAtom(Instant::BasicType());
    }
  }
  return listutils::typeError("Correct signature:  ilabel -> instant,   "
    "ilabels -> instant,   iplace -> instant,   iplaces -> instant");
}

/*
\subsubsection{Value Mapping}

*/
template<class Intime>
int instSymbolicVM(Word* args, Word& result, int message, Word& local, 
                   Supplier s) {
  result = qp->ResultStorage(s);
  Instant* res = static_cast<Instant*>(result.addr);
  Intime *src = static_cast<Intime*>(args[0].addr);
  if (src->IsDefined()) {
    res->CopyFrom(&(src->instant));
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct instSymbolicInfo : OperatorInfo {
  instSymbolicInfo() {
    name      = "inst";
    signature = "ilabel -> instant";
    appendSignature("ilabels -> instant");
    appendSignature("iplace -> instant");
    appendSignature("iplaces -> instant");
    syntax    = "inst ( _ )";
    meaning   = "Returns the instant of the ilabel(s) or the iplace(s).";
  }
};

/*
\subsection{Operator ~inside~}

inside: mlabel x labels -> mbool
inside: mplace x places -> mbool

\subsubsection{Type Mapping}

*/
ListExpr insideSymbolicTM(ListExpr args) {
  if (nl->HasLength(args, 2)) {
    ListExpr first(nl->First(args)), second(nl->Second(args));
    if ((MLabel::checkType(first) && Labels::checkType(second)) ||
        (MPlace::checkType(first) && Places::checkType(second))) {
      return nl->SymbolAtom(MBool::BasicType());
    }
  }
  return listutils::typeError("Correct signatures: mlabel x labels -> mbool, "
    "  mplace x places -> mbool");
}

/*
\subsubsection{Selection Function}

*/
int insideSymbolicSelect(ListExpr args) {
  if (MLabel::checkType(nl->First(args))) return 0;
  if (MPlace::checkType(nl->First(args))) return 1;
  return -1;
}

/*
\subsubsection{Value Mapping}

*/
template<class Mapping, class Collection>
int insideSymbolicVM(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  result = qp->ResultStorage(s);
  MBool* res = static_cast<MBool*>(result.addr);
  Mapping *src = static_cast<Mapping*>(args[0].addr);
  Collection *coll = static_cast<Collection*>(args[1].addr);
  src->Inside(*coll, *res);
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct insideSymbolicInfo : OperatorInfo {
  insideSymbolicInfo() {
    name      = "inside";
    signature = "mlabel x labels -> mbool";
    appendSignature("mplace x places -> mbool");
    syntax    = "_ inside _";
    meaning   = "Returns a mbool with the same time intervals as the mlabel / "
                "mplace. A unit\'s value is TRUE if and only if the label / "
                "place is an element of the labels / places.";
  }
};

/*
\section{Operator ~topattern~}

\subsection{Type Mapping}

*/
ListExpr topatternTM(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError("one argument expected");
  }
  NList type(args);
  if (type.first() == NList(FText::BasicType())) {
    return NList(Pattern::BasicType()).listExpr();
  }
  return NList::typeError("Expecting a text!");
}

/*
\subsection{Value Mapping}

*/
int topatternVM(Word* args, Word& result, int message, Word& local,
                Supplier s) {
  FText* patternText = static_cast<FText*>(args[0].addr);
  result = qp->ResultStorage(s);
  Pattern* p = static_cast<Pattern*>(result.addr);
  Pattern* pattern = 0;
  if (patternText->IsDefined()) {
    pattern = Pattern::getPattern(patternText->toText());
  }
  else {
    cout << "undefined text" << endl;
    return 0;
  }
  if (pattern) {
    (*p) = (*pattern);
    delete pattern;
  }
  else {
    cout << "invalid pattern" << endl;
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct topatternInfo : OperatorInfo {
  topatternInfo() {
    name      = "topattern";
    signature = " Text -> " + Pattern::BasicType();
    syntax    = "_ topattern";
    meaning   = "Creates a Pattern from a Text.";
  }
};

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
\section{Operator ~toclassifier~}

\subsection{Type Mapping}

*/
ListExpr toclassifierTM(ListExpr args) {
  const string errMsg = "Expecting stream(tuple(s: text, t: text))";
  if (nl->HasLength(args, 1)) {
    if (Stream<Tuple>::checkType(nl->First(args))) {
      ListExpr dType, pType;
      if (nl->ListLength(nl->Second(nl->Second(nl->First(args)))) != 2) {
        return listutils::typeError("Tuple must have exactly 2 attributes");
      }
      dType = nl->Second(nl->First(nl->Second(nl->Second(nl->First(args)))));
      pType = nl->Second(nl->Second(nl->Second(nl->Second(nl->First(args)))));
      if (FText::checkType(dType) && FText::checkType(pType)) {
        return nl->SymbolAtom(Classifier::BasicType());
      }
    }
  }
  return listutils::typeError(errMsg);
}

/*
\subsection{Value Mapping}

*/
int toclassifierVM(Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  Stream<Tuple> stream = static_cast<Stream<Tuple> >(args[0].addr);
  stream.open();
  result = qp->ResultStorage(s);
  Classifier* c = static_cast<Classifier*>(result.addr);
  Tuple* tuple = stream.request();
  FText *desc, *ptext;
  map<int, int> final2Pat;
  Pattern* p = 0;
  vector<string> texts;
  vector<Pattern*> patterns;
  while (tuple) {
    desc = (FText*)tuple->GetAttribute(0);
    if (!desc->IsDefined()) {
      cout << "Undefined description" << endl;
    }
    else {
      ptext = (FText*)tuple->GetAttribute(1);
      if (!ptext->IsDefined()) {
        cout << "Undefined pattern text" << endl;
      }
      else {
        p = Pattern::getPattern(ptext->GetValue(), true); // do not build NFA
        if (!p) {
          cout << "invalid pattern" << endl;
        }
        else {
          texts.push_back(desc->GetValue());
          texts.push_back(ptext->GetValue());
          patterns.push_back(p);
        }
      }
    }
    tuple->DeleteIfAllowed();
    tuple = stream.request();
  }
  stream.close();
  c->appendCharPos(0);
  for (unsigned int i = 0; i < texts.size(); i++) {
    for (unsigned int j = 0; j < texts[i].length(); j++) { //store desc&pattern
      c->appendChar(texts[i][j]);
    }
    c->appendCharPos(c->getCharSize());
  }
  vector<map<int, int> > nfa;
  set<int> finalStates;
  c->buildMultiNFA(patterns, nfa, finalStates, final2Pat);
  for (unsigned int i = 0; i < patterns.size(); i++) {
    delete patterns[i];
  }
  c->setPersistentNFA(nfa, finalStates, final2Pat);
  return 0;
}

/*
\subsection{Operator Info}

*/
struct toclassifierInfo : OperatorInfo {
  toclassifierInfo() {
    name      = "toclassifier";
    signature = "stream(tuple(description: text, pattern: text)) -> classifier";
    syntax    = "_ toclassifier";
    meaning   = "creates a classifier from a stream(tuple(s: text, t: text))";
  }
};

/*
\section{Operator ~matches~}

\subsection{Type Mapping}

This type mapping checks whether the second argument (i.e., the pattern) is
constant or not and passes that information to the value mapping.

*/
ListExpr matchesTM(ListExpr args) {
  if (!nl->HasLength(args, 2)) {
    return NList::typeError("Two arguments expected");
  }
  if (!nl->HasLength(nl->First(args),2) || !nl->HasLength(nl->Second(args),2)) {
    return NList::typeError("Two arguments expected for each sublist");
  }
  if (!MLabel::checkType(nl->First(nl->First(args))) ||
      (!FText::checkType(nl->First(nl->Second(args))) &&
       !Pattern::checkType(nl->First(nl->Second(args))))) {
    return NList::typeError("Expecting a mlabel and a text/pattern");
  }
  string query = nl->ToString(nl->Second(nl->Second(args)));
  Word res;
  bool isConst =  QueryProcessor::ExecuteQuery(query, res);
  if (isConst) {
    if(FText::checkType(nl->First(nl->Second(args)))) {
      ((FText*)res.addr)->DeleteIfAllowed();
    } 
    else {
      delete (Pattern*)res.addr;
    }
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           nl->OneElemList( nl->BoolAtom(isConst)),
                           nl->SymbolAtom(CcBool::BasicType()));
}

/*
\subsection{Selection Function}

*/
int matchesSelect(ListExpr args) {
  return (Pattern::checkType(nl->Second(args))) ? 1 : 0;
}

/*
\subsection{Value Mapping (for a Pattern)}

*/
int matchesVM_P(Word* args, Word& result, int message, Word& local, Supplier s){
  MLabel* ml = static_cast<MLabel*>(args[0].addr);
  Pattern* p = static_cast<Pattern*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool* b = static_cast<CcBool*>(result.addr);
  ExtBool match = p->matches(ml);
  switch (match) {
    case FALSE: {
      b->Set(true, false);
      break;
    }
    case TRUE: {
      b->Set(true, true);
      break;
    }
    default: {
      b->SetDefined(false);
    }
  }
  return 0;
}

/*
\subsection{Value Mapping (for a text)}

*/
int matchesVM_T(Word* args, Word& result, int message, Word& local, Supplier s){
  MLabel* ml = static_cast<MLabel*>(args[0].addr);
  FText* pText = static_cast<FText*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool* b = static_cast<CcBool*>(result.addr);
  Pattern *p = 0;
  if (message != CLOSE) {
    if ((static_cast<CcBool*>(args[2].addr))->GetValue()) { //2nd argument const
      if (!local.addr) {
        if (pText->IsDefined() && ml->IsDefined()) {
          local.addr = Pattern::getPattern(pText->toText());
        }
        else {
          cout << "Undefined pattern text or mlabel." << endl;
          b->SetDefined(false);
          return 0;
        }
      }
      p = (Pattern*)local.addr;
      if (!p) {
        b->SetDefined(false);
      }
      else if (p->hasAssigns()) {
        cout << "No assignments allowed for matches" << endl;
        b->SetDefined(false);
      }
      else {
        ExtBool res = p->matches(ml);
        switch (res) {
          case FALSE: {
            b->Set(true, false);
            break;
          }
          case TRUE: {
            b->Set(true, true);
            break;
          }
          default: {
            b->SetDefined(false);
          }
        }
      }
    }
    else { // second argument non-constant
      if (pText->IsDefined()) {
        p = Pattern::getPattern(pText->toText());
      }
      else {
        cout << "Undefined pattern text." << endl;
        b->SetDefined(false);
        return 0;
      }
      if (!p) {
        b->SetDefined(false);
      }
      else if (p->hasAssigns()) {
        cout << "No assignments allowed for matches" << endl;
        b->SetDefined(false);
      }
      else {
        ExtBool res = p->matches(ml);
        switch (res) {
          case FALSE: {
            b->Set(true, false);
            break;
          }
          case TRUE: {
            b->Set(true, true);
            break;
          }
          default: {
            b->SetDefined(false);
          }
        }
      }
      if (p) {
        delete p;
      }
    }
  }
  else {
    if (local.addr) {
      delete (Pattern*)local.addr;
      local.addr = 0;
    }
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
const string matchesSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> mlabel x {pattern|text} -> bool </text--->"
  "<text> ML matches P </text--->"
  "<text> Checks whether ML matches P.\n"
  "<text> query michael matches '(_ at_home) * (_ at_home)' </text--->) )";

ValueMapping matchesVMs[] = {matchesVM_T, matchesVM_P};

Operator matches("matches", matchesSpec, 2, matchesVMs, matchesSelect,
                 matchesTM);

/*
\section{Operator ~indexmatches~}

\subsection{Type Mapping}

*/
ListExpr indexmatchesTM(ListExpr args) {
  const string errMsg = "Expecting a relation, the name of a mlabel"
             " attribute of that relation, an invfile, and a pattern/text";
  if (nl->HasLength(args, 4)) {
    if (FText::checkType(nl->Fourth(args)) || 
        Pattern::checkType(nl->Fourth(args))) {
      if (Relation::checkType(nl->First(args))) {
        ListExpr tupleList = nl->First(nl->Rest(nl->First(args)));
        if (Tuple::checkType(tupleList)
         && listutils::isSymbol(nl->Second(args))) {
          ListExpr attrType;
          ListExpr attrList = nl->Second(tupleList);
          string attrName = nl->SymbolValue(nl->Second(args));
          int i = listutils::findAttribute(attrList, attrName, attrType);
          if (i == 0) {
            return listutils::typeError(attrName + " not found");
          }
          if (!MLabel::checkType(attrType)) {
            return listutils::typeError
                   ("type " + nl->ToString(attrType) + " is an invalid type");
          }
          if (InvertedFile::checkType(nl->Third(args))) {
            return nl->ThreeElemList(
              nl->SymbolAtom(Symbol::APPEND()),
              nl->OneElemList(nl->IntAtom(i - 1)),
              nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), tupleList));
          }
        }
      }
    }
  }
  return listutils::typeError(errMsg);
}

/*
\subsection{Constructor for class ~IndexMatchesLI~}

*/
IndexMatchesLI::IndexMatchesLI(Word _mlrel, InvertedFile *inv, int _attrNr,
             Pattern *p, bool deleteP) : IndexClassifyLI(_mlrel, inv, _attrNr) {
  if (p) {
    applyPattern(p);
    if (deleteP) {
      delete p;
    }
  }
}

/*
\subsection{Function ~nextResultTuple~}

*/
Tuple* IndexMatchesLI::nextTuple() {
  if (!classification.empty()) {
    pair<string, TupleId> matched = classification.front();
    classification.pop();
    return mlRel->GetTuple(matched.second, false);
  }
  return 0;
}

/*
\subsection{Selection Function}

*/
int indexmatchesSelect(ListExpr args) {
  return (FText::checkType(nl->Fourth(args)) ? 0 : 1);
}

/*
\subsection{Value Mapping (type text)}

*/
int indexmatchesVM_T(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  IndexMatchesLI *li = (IndexMatchesLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      FText *pText = static_cast<FText*>(args[3].addr);
      CcInt *attr = static_cast<CcInt*>(args[4].addr);
      InvertedFile *inv = static_cast<InvertedFile*>(args[2].addr);
      Pattern* p = 0;
      if (pText->IsDefined() && attr->IsDefined()) {
        p = Pattern::getPattern(pText->GetValue());
        if (p) {
          if (p->hasConds() || p->containsRegEx()) {
            cout << "pattern is not simple" << endl;
            local.addr = 0;
          }
          else {
            local.addr = new IndexMatchesLI(args[0], inv, attr->GetIntval(), p, 
                                            true);
          }
        }
      }
      else {
        cout << "undefined parameter(s)" << endl;
        local.addr = 0;
      }
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->nextTuple() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

/*
\subsection{Value Mapping (type pattern)}

*/
int indexmatchesVM_P(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  IndexMatchesLI *li = (IndexMatchesLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      Pattern *p = static_cast<Pattern*>(args[3].addr);
      CcInt *attr = static_cast<CcInt*>(args[4].addr);
      InvertedFile *inv = static_cast<InvertedFile*>(args[2].addr);
      if (p->IsDefined() && attr->IsDefined()) {
        if (p->hasConds() || p->containsRegEx()) {
          cout << "pattern is not simple" << endl;
          local.addr = 0;
        }
        else {
          local.addr = new IndexMatchesLI(args[0], inv, attr->GetIntval(), p,
                                          false);
        }
      }
      else {
        cout << "undefined parameter(s)" << endl;
        local.addr = 0;
      }
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->nextTuple() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct indexmatchesInfo : OperatorInfo {
  indexmatchesInfo() {
    name      = "indexmatches";
    signature = "rel(tuple(X)) x IDENT x invfile x text -> stream(tuple(X))";
    syntax    = "_ indexmatches [ _ , _ , _ ]";
    meaning   = "Filters a relation containing a mlabel attribute, applying a "
                "trajectory relation index and passing only those trajectories "
                "matching the pattern on to the output stream.";
  }
};

/*
\section{Operator ~filtermatches~}

\subsection{Type Mapping}

*/
ListExpr filtermatchesTM(ListExpr args) {
  string err = "the expected syntax is: stream(tuple(X)) x attrname x text"
               "or stream(tuple(X)) x attrname x pattern";
  if (!nl->HasLength(args, 3)) {
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  ListExpr stream = nl->First(args);
  ListExpr anlist = nl->Second(args);
  if (!Stream<Tuple>::checkType(stream) || !listutils::isSymbol(anlist) ||
  (!FText::checkType(nl->Third(args)) && !Pattern::checkType(nl->Third(args)))){
    return listutils::typeError(err);
  }
  string name = nl->SymbolValue(anlist);
  ListExpr type;
  int index = listutils::findAttribute(nl->Second(nl->Second(stream)),
                                       name, type);
  if (!index) {
    return listutils::typeError("attribute " + name + " not found in tuple");
  }
  if (!MLabel::checkType(type)) {
    return listutils::typeError("wrong type " + nl->ToString(type)
                                + " of attritube " + name);
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->OneElemList(nl->IntAtom(index - 1)), stream);
}

/*
\subsection{Constructors for class ~FilterMatchesLI~}

*/
FilterMatchesLI::FilterMatchesLI(Word _stream, int _attrIndex, FText* text) :
                 stream(_stream), attrIndex(_attrIndex), match(0), 
                 streamOpen(false), deleteP(true) {
  Pattern *p = Pattern::getPattern(text->GetValue());
  if (p) {
    match = new Match(p, 0);
    stream.open();
    streamOpen = true;
  }
}

FilterMatchesLI::FilterMatchesLI(Word _stream, int _attrIndex, Pattern* p):
                 stream(_stream), attrIndex(_attrIndex), match(0), 
                 streamOpen(false), deleteP(false) {
  if (p) {
    match = new Match(p, 0);
    stream.open();
    streamOpen = true;
  }
}

/*
\subsection{Destructor for class ~FilterMatchesLI~}

*/
FilterMatchesLI::~FilterMatchesLI() {
  if (match) {
    if (deleteP) {
      match->deletePattern();
    }
    delete match;
    match = 0;
  }
  if (streamOpen) {
    stream.close();
  }
}

/*
\subsection{Function ~getNextResult~}

*/
Tuple* FilterMatchesLI::getNextResult() {
  if (!match) {
    return 0;
  }
  Tuple* cand = stream.request();
  while (cand) {
    match->setML((MLabel*)cand->GetAttribute(attrIndex));
    if (match->matches() == TRUE) {
      return cand;
    }
    cand->DeleteIfAllowed();
    cand = stream.request();
  }
  return 0;
}

/*
\subsection{Selection Function}

*/
int filtermatchesSelect(ListExpr args) {
  return (FText::checkType(nl->Third(args)) ? 0 : 1);
}

/*
\subsection{Value Mapping for a Text}

*/
int filtermatchesVM_T(Word* args, Word& result, int message, Word& local,
                      Supplier s) {
  FilterMatchesLI* li = (FilterMatchesLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      CcInt* ccint = (CcInt*)args[3].addr;
      FText* ftext = (FText*)args[2].addr;
      if (ftext->IsDefined() && ccint->IsDefined()) {
        local.addr = new FilterMatchesLI(args[0], ccint->GetValue(), ftext);
      }
      else {
        cout << "undefined argument(s)" << endl;
      }
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->getNextResult() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return  0;
}

/*
\subsection{Value Mapping for a Pattern}

*/
int filtermatchesVM_P(Word* args, Word& result, int message, Word& local, 
                      Supplier s) {
  FilterMatchesLI* li = (FilterMatchesLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      CcInt* ccint = (CcInt*)args[3].addr;
      Pattern* p = (Pattern*)args[2].addr;
      if (ccint->IsDefined()) {
        local.addr = new FilterMatchesLI(args[0], ccint->GetValue(), p);
      }
      else {
        cout << "undefined argument(s)" << endl;
      }
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->getNextResult() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return  0;
}

/*
\subsection{Operator Info}

*/
struct filtermatchesInfo : OperatorInfo {
  filtermatchesInfo() {
    name      = "filtermatches";
    signature = "stream(tuple(X)) x IDENT x text -> stream(tuple(X))";
    syntax    = "_ filtermatches [ _ , _ ]";
    meaning   = "Filters a stream containing moving labels, passing "
                "exactly the tuples whose moving labels match the pattern on "
                "to the output stream.";
  }
};


/*
\section{Operator ~rewrite~}

\subsection{Type Mapping}

*/
ListExpr rewriteTM(ListExpr args) {
  NList type(args);
  const string errMsg = "Expecting a mlabel and a pattern/text"
                        " or a stream<mlabel> and a stream<text>";
  if ((type == NList(MLabel::BasicType(), Pattern::BasicType()))
   || (type == NList(MLabel::BasicType(), FText::BasicType()))) {
    return nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                           nl->SymbolAtom(MLabel::BasicType()));
  }
  if (nl->HasLength(args, 2)) {
    if (Stream<MLabel>::checkType(nl->First(args))
     && Stream<FText>::checkType(nl->Second(args))) {
       return nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                              nl->SymbolAtom(MLabel::BasicType()));
    }
  }
  return NList::typeError(errMsg);
}

/*
\subsection{Selection Function}

*/
int rewriteSelect(ListExpr args) {
  if (Stream<FText>::checkType(nl->Second(args))) {
    return 2;
  }
  return (Pattern::checkType(nl->Second(args)) ? 1 : 0);
}

/*
\subsection{Function ~initOpTrees~}

Necessary for the operator ~rewrite~

*/
bool Assign::initOpTrees() {
  if (treesOk && opTree[0].second) {
    return true;
  }
  for (int i = 0; i < 6; i++) {
    if (pointers[i].size() > 0) { // pointers already initialized
      return true;
    }
  }
  if (!occurs() && (text[0].empty() || (text[1].empty() &&
     (text[2].empty() || text[3].empty())))) {
    cout << "not enough data for variable " << var << endl;
    return false;
  }
  string q(""), part, toReplace("");
  pair<string, Attribute*> strAttr;
  for (int i = 0; i < 6; i++) { // label, time, start, end, leftclosed, rightcl.
    if (!text[i].empty()) {
      q = "query " + text[i];
      for (unsigned int j = 0; j < right[i].size(); j++) { // loop through keys
        strAttr = Pattern::getPointer(right[i][j].second);
        if (right[i][j].second == 1) {
          deleteIfAllowed(strAttr.second);
          strAttr.second = new SecInterval(1);
          strAttr.first = "[const interval pointer "
                   + nl->ToString(listutils::getPtrList(strAttr.second)) + "]";
        }
        pointers[i].push_back(strAttr.second);
        toReplace = right[i][j].first + Condition::getType(right[i][j].second);
        q.replace(q.find(toReplace), toReplace.length(), strAttr.first);
      }
      opTree[i] = Match::processQueryStr(q, i);
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

RewriteLI::RewriteLI(MLabel *src, Pattern *pat) {
  match = new Match(pat, src);
  if (match->matches()) {
    match->initCondOpTrees();
    if (!src->GetNoComponents()) {
      BindingStackElem dummy(0, 0);
      bindingStack.push(dummy);
    }
    else {
      map<int, int> transitions = pat->getTransitions(0);
      for (map<int, int>::iterator itm = transitions.begin();
                                   itm != transitions.end(); itm++) {
        BindingStackElem bE(0, itm->first); // init stack
  //       cout << "push (0, " << itm->first << ") to stack" << endl;
        bindingStack.push(bE);
      }
    }
  }
  else {
    match->deletePattern();
    delete match;
    match = 0;
  }
}

MLabel* RewriteLI::getNextResult() {
  if (!match) {
    return 0;
  }
  if (!match->ml->GetNoComponents()) { // empty mlabel
    if (bindingStack.empty()) {
      return 0;
    }
    bindingStack.pop();
    vector<Condition> *conds = match->p->getConds();
    if (match->conditionsMatch(*conds, binding)) {
      MLabel *source = match->ml;
      return source->rewrite(binding, match->p->getAssigns());
    }
    return 0;
  }
  else { // non-empty mlabel
    BindingStackElem bE(0, 0);
    while (!bindingStack.empty()) {
      bE = bindingStack.top();
  //    cout << "take (" << bE.ulId << ", " << bE.pId << ") from stack" << endl;
      bindingStack.pop();
      resetBinding(bE.ulId);
      if (findNextBinding(bE.ulId, bE.peId, match->p, 0)) {
//         match->printBinding(binding);
        if (!rewBindings.count(binding)) {
          rewBindings.insert(binding);
          MLabel *source = match->ml;
          return source->rewrite(binding, match->p->getAssigns());
        }
      }
    }
  //   cout << "stack is empty" << endl;
    match->deletePattern();
    match->deleteSetMatrix();
    delete match;
    return 0;
  }
}

void RewriteLI::resetBinding(unsigned int limit) {
  vector<string> toDelete;
  map<string, pair<unsigned int, unsigned int> >::iterator it;
  for (it = binding.begin(); it != binding.end(); it++) {
    if (it->second.first >= limit) {
      toDelete.push_back(it->first);
    }
    else if (it->second.second >= limit) {
      it->second.second = limit - 1;
    }
  }
  for (unsigned int i = 0; i < toDelete.size(); i++) {
    binding.erase(toDelete[i]);
  }
}

bool RewriteLI::findNextBinding(unsigned int ulId, unsigned int peId,
                                Pattern *p, int offset) {
//   cout << "findNextBinding(" << ulId << ", " << peId << ", " << offset
//        << ") called" << endl;
  string var = p->getVarFromElem(peId - offset);
  if (!var.empty() && p->isRelevant(var)) {
    if (binding.count(var)) { // extend existing binding
      binding[var].second++;
    }
    else { // add new variable
      binding[var] = make_pair(ulId, ulId);
    }
  }
  if (*(match->matching[ulId][peId].begin()) == UINT_MAX) { // complete match
    vector<Condition> *conds = p->getConds();
    return match->conditionsMatch(*conds, binding);
  }
  if (match->matching[ulId][peId].empty()) {
    return false;
  }
  else { // push all elements except the first one to stack; process first elem
    set<unsigned int>::reverse_iterator it, it2;
    it2 = match->matching[ulId][peId].rbegin();
    it2++;
    for (it = match->matching[ulId][peId].rbegin();
         it2 != match->matching[ulId][peId].rend(); it++) {
      it2++;
      BindingStackElem bE(ulId + 1, *it);
//       cout << "push (" << ulId + 1 << ", " << *it << ") to stack" << endl;
      bindingStack.push(bE);
    }
    return findNextBinding(ulId + 1, *(match->matching[ulId][peId].begin()), p,
                           offset);
  }
}
/*
\subsection{Value Mapping (for a text)}

*/
int rewriteVM_T(Word* args, Word& result, int message, Word& local, Supplier s){
  MLabel *source = 0;
  FText* pText = 0;
  Pattern *p = 0;
  RewriteLI *rewriteLI = 0;
  switch (message) {
    case OPEN: {
      source = static_cast<MLabel*>(args[0].addr);
      pText = static_cast<FText*>(args[1].addr);
      if (!pText->IsDefined()) {
        cout << "Error: undefined pattern text." << endl;
        return 0;
      }
      if (!source->IsDefined()) {
        cout << "Error: undefined mlabel." << endl;
        return 0;
      }
      p = Pattern::getPattern(pText->toText());
      if (!p) {
        cout << "Error: pattern not initialized." << endl;
      }
      else {
        if (!p->hasAssigns()) {
          cout << "No result specified." << endl;
        }
        else {
          if (p->initAssignOpTrees() && p->initEasyCondOpTrees()) {
            rewriteLI = new RewriteLI(source, p);
          }
        }
      }
      local.addr = rewriteLI;
      return 0;
    }
    case REQUEST: {
      if (!local.addr) {
        result.addr = 0;
        return CANCEL;
      }
      rewriteLI = ((RewriteLI*)local.addr);
      result.addr = rewriteLI->getNextResult();
      return (result.addr ? YIELD : CANCEL);
    }
    case CLOSE: {
      if (local.addr) {
        rewriteLI = ((RewriteLI*)local.addr);
        delete rewriteLI;
        local.addr=0;
      }
      return 0;
    }
    default:
      return -1;
  }
}

/*
\subsection{Value Mapping (for a Pattern)}

*/
int rewriteVM_P(Word* args, Word& result, int message, Word& local, Supplier s){
  MLabel *source(0);
  Pattern *p = 0;
  RewriteLI *rewriteLI = 0;
  switch (message) {
    case OPEN: {
      source = static_cast<MLabel*>(args[0].addr);
      if (!source->IsDefined()) {
        cout << "Error: undefined mlabel." << endl;
        return 0;
      }
      p = static_cast<Pattern*>(args[1].addr);
      if (!p) {
        cout << "Error: pattern not initialized." << endl;
      }
      else {
        if (!p->hasAssigns()) {
          cout << "No result specified." << endl;
        }
        else {
          if (p->initAssignOpTrees() && p->initEasyCondOpTrees()) {
            rewriteLI = new RewriteLI(source, p);
          }
        }
      }
      local.addr = rewriteLI;
      return 0;
    }
    case REQUEST: {
      if (!local.addr) {
        result.addr = 0;
        return CANCEL;
      }
      rewriteLI = ((RewriteLI*)local.addr);
      result.addr = rewriteLI->getNextResult();
      return (result.addr ? YIELD : CANCEL);
    }
    case CLOSE: {
      if (local.addr) {
        rewriteLI = ((RewriteLI*)local.addr);
        delete rewriteLI;
        local.addr=0;
      }
      return 0;
    }
    default:
      return -1;
  }
}

/*
\subsection{Value Mapping (for a stream of patterns)}

*/
int rewriteVM_Stream(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  MultiRewriteLI *li = (MultiRewriteLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      local.addr = new MultiRewriteLI(args[0], args[1]);
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->nextResultML() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct rewriteInfo : OperatorInfo {
  rewriteInfo() {
    name      = "rewrite";
    signature = "mlabel x text -> stream(mlabel)";
    appendSignature("mlabel x pattern -> + stream(mlabel)");
    appendSignature("stream(mlabel) x stream(text) -> stream(mlabel)");
    syntax    = "rewrite (_, _)";
    meaning   = "Rewrite a mlabel or a stream of them.";
  }
};

/*
\section{Operator ~classify~}

\subsection{Type Mapping}

*/
ListExpr classifyTM(ListExpr args) {
  const string errMsg = "Expecting an mlabel and a classifier.";
  if (nl->HasLength(args, 2)) {
    if (MLabel::checkType(nl->First(args))
     && Classifier::checkType(nl->Second(args))) {
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(FText::BasicType()));
    }
  }
  return listutils::typeError(errMsg);
}

/*
\subsection{Constructor for class ~ClassifyLI~}

This constructor is used for the operator ~classify~.

*/
ClassifyLI::ClassifyLI(MLabel *ml, Word _classifier) : classifyTT(0) {
  Classifier *c = static_cast<Classifier*>(_classifier.addr);
  int startState(0), pat(0);
  set<unsigned int> emptyset;
  set<int> states, finalStates, matchCands;
  set<int>::iterator it;
  Pattern *p = 0;
  vector<PatElem> patElems;
  vector<Condition> easyConds;
  vector<int> startStates;
  map<int, set<int> > easyCondPos;
  map<int, int> atomicToElem; // TODO: use this sensibly
  map<int, string> elemToVar; // TODO: use this sensibly
  bool condsOccur = false;
  for (int i = 0; i < c->getCharPosSize() / 2; i++) {
    states.insert(states.end(), startState);
    startStates.push_back(startState);
    p = Pattern::getPattern(c->getPatText(i), true); // single NFA are not built
    if (p) {
      p->setDescr(c->getDesc(i));
      if (p->hasConds()) {
        condsOccur = true;
      }
      pats.push_back(p);
      map<int, set<int> > easyOld = p->getEasyCondPos();
      for (map<int, set<int> >::iterator im = easyOld.begin();
                                         im != easyOld.end(); im++) {
        for (it = im->second.begin(); it != im->second.end(); it++) {
          easyCondPos[im->first+patElems.size()].insert(*it + easyConds.size());
        }
      }
      for (unsigned int j = 0; j < p->getEasyConds().size(); j++) {
        easyConds.push_back(p->getEasyCond(j));
        easyConds.back().initOpTree();
      }
      for (int j = 0; j < p->getSize(); j++) {
        patElems.push_back(p->getElem(j));
      }
      do { // get start state
        startState++;
        c->s2p.Get(startState, pat);
      } while ((i < c->getCharPosSize() / 2 - 1) && (pat >= 0));
    }
    else {
      cout << "pattern could not be parsed" << endl;
    }
  }
  if (!pats.size()) {
    cout << "no classification data specified" << endl;
    return;
  }
  vector<map<int, int> > nfa;
  createNFAfromPersistent(c->delta, c->s2p, nfa, finalStates);
  Match *match = new Match(0, ml);
  if (condsOccur) {
    match->createSetMatrix(ml->GetNoComponents(), patElems.size());
  }
  for (int i = 0; i < ml->GetNoComponents(); i++) {
    if (!match->updateStates(i, nfa, patElems, finalStates, states, easyConds,
                             easyCondPos, atomicToElem, condsOccur)){
      for (unsigned int j = 0; j < easyConds.size(); j++) {
        easyConds[j].deleteOpTree();
      }
      return;
    }
  }
  for (unsigned int j = 0; j < easyConds.size(); j++) {
    easyConds[j].deleteOpTree();
  }
  for (it = states.begin(); it != states.end(); it++) { // active states final?
    c->s2p.Get(*it, pat);
    if ((*it > 0) && (pat != INT_MAX) && (pat >= 0)) {
      matchCands.insert(matchCands.end(), pat);
//       cout << "pattern " << pat << " matches" << endl;
    }
  }
  for (it = matchCands.begin(); it != matchCands.end(); it++) { // check conds
    pats[*it]->initCondOpTrees();
    vector<Condition>* conds = pats[*it]->getConds();
//     cout << "call fMB(nfa, " << startStates[*it] << ", " << patElems.size()
//          << ", " << conds->size() << ")" << endl;
    if (match->findMatchingBinding(nfa, startStates[*it], patElems, *conds,
                                   atomicToElem, elemToVar)) {
      matchingPats.insert(*it);
//       cout << "p " << *it << " matches after condition check" << endl;
    }
    else {
//       cout << "p " << *it << " has non-matching conditions" << endl;
    }
    pats[*it]->deleteCondOpTrees();
  }
  match->deleteSetMatrix();
  delete match;
}

/*
\subsection{Constructor for class ~MultiRewriteLI~}

This constructor is used for the operator ~rewrite~.

*/
MultiRewriteLI::MultiRewriteLI(Word _mlstream, Word _pstream) : ClassifyLI(0),
             RewriteLI(0), mlStream(_mlstream), streamOpen(false), ml(0), c(0) {
  Stream<FText> pStream(_pstream);
  pStream.open();
  FText* inputText = pStream.request();
  Pattern *p = 0;
  set<int>::iterator it;
  map<int, set<int> >::iterator im;
  int elemCount(0);
  while (inputText) {
    if (!inputText->IsDefined()) {
      cout << "undefined input is ignored" << endl;
    }
    else {
      p = Pattern::getPattern(inputText->GetValue(), true); // no single NFA
      if (p) {
        if (!p->hasAssigns()) {
          cout << "pattern without rewrite part ignored" << endl;
        }
        else {
          if (p->initCondOpTrees()) {
            pats.push_back(p);
            for (int i = 0; i < p->getSize(); i++) {
              atomicToElem[patElems.size()] = elemCount + p->getElemFromAtom(i);
              elemToVar[elemCount+p->getElemFromAtom(i)] = p->getElem(i).getV();
              patElems.push_back(p->getElem(i));
              patOffset[elemCount + p->getElemFromAtom(i)] =
                                          make_pair(pats.size() - 1, elemCount);
            }
            elemCount += p->getElemFromAtom(p->getSize() - 1) + 1;
            map<int, set<int> > easyOld = p->getEasyCondPos();
            for (im = easyOld.begin(); im != easyOld.end(); im++) {
              for (it = im->second.begin(); it != im->second.end(); it++) {
                easyCondPos[im->first + patElems.size()].insert
                                                       (*it + easyConds.size());
              }
            }
            for (unsigned int j = 0; j < p->getEasyConds().size(); j++) {
              easyConds.push_back(p->getEasyCond(j));
              easyConds.back().initOpTree();
            }
          }
        }
      }
    }
    inputText->DeleteIfAllowed();
    inputText = pStream.request();
  }
  pStream.close();
  if (!pats.size()) {
    cout << "no classification data specified" << endl;
  }
  else {
    mlStream.open();
    streamOpen = true;
    Classifier *c = new Classifier(0);
    c->buildMultiNFA(pats, nfa, finalStates, state2Pat);
    c->getStartStates(states);
    match = new Match(0, ml);
    c->DeleteIfAllowed();
  }
}


/*
\subsection{Function ~nextResultML~}

This function is used for the operator ~rewrite~.

*/
MLabel* MultiRewriteLI::nextResultML() {
  if (!pats.size()) {
    return 0;
  }
  set<int> startStates;
  set<int>::iterator it;
  while (!bindingStack.empty()) {
    BindingStackElem bE(0, 0);
    bE = bindingStack.top();
//     cout << "take (" << bE.ulId << ", " << bE.pId << ") from stack" << endl;
    bindingStack.pop();
    resetBinding(bE.ulId);
    pair<int, int> patNo = patOffset[bE.peId];
    if (findNextBinding(bE.ulId, bE.peId, pats[patNo.first], patNo.second)) {
      return ml->rewrite(binding, pats[patNo.first]->getAssigns());
    }
  }
  while (bindingStack.empty()) { // new ML from stream necessary
    match->deleteSetMatrix();
    delete match;
    match = 0;
    deleteIfAllowed(ml);
    ml = (MLabel*)mlStream.request();
    if (!ml) {
      return 0;
    }
    match = new Match(0, ml);
    match->createSetMatrix(ml->GetNoComponents(), patElems.size());
    getStartStates(startStates);
    states = startStates;
    matchCands.clear();
    int i = 0;
    while (!states.empty() && (i < ml->GetNoComponents())) { // loop through ml
      match->updateStates(i, nfa, patElems, finalStates, states, easyConds,
                          easyCondPos, atomicToElem, true);
      i++;
    }
    for (it = states.begin(); it != states.end(); it++) { //active states final?
      if (finalStates.count(*it)) {
        matchCands.insert(matchCands.end(), state2Pat[*it]);
      }
    }
    initStack(startStates);
    while (!bindingStack.empty()) {
      BindingStackElem bE(0, 0);
      bE = bindingStack.top();
//      cout << "take (" << bE.ulId << ", " << bE.pId << ") from stack" << endl;
      bindingStack.pop();
      resetBinding(bE.ulId);
      pair<int, int> patNo = patOffset[bE.peId];
      if (findNextBinding(bE.ulId, bE.peId, pats[patNo.first], patNo.second)) {
        return ml->rewrite(binding, pats[patNo.first]->getAssigns());
      }
    }
  }
  cout << "SHOULD NOT OCCUR" << endl;
  return 0;
}

/*
\subsection{Function ~initStack~}

Determines the start states of the match candidate patterns and pushes the
corresponding initial transitions onto the stack.

*/
void MultiRewriteLI::initStack(set<int> &startStates) {
  set<int>::iterator it;
  map<int, int>::iterator itm;
  for (it = startStates.begin(); it != startStates.end(); it++) {
    if (matchCands.count(-state2Pat[*it])) {
      map<int, int> transitions = nfa[*it];
      for (itm = transitions.begin(); itm != transitions.end(); itm++) {
        BindingStackElem bE(0, atomicToElem[itm->first]);
        bindingStack.push(bE);
//         cout << "(0, " << itm->first << ") pushed onto stack" << endl;
      }
    }
  }
}

/*
\subsection{Function ~getStartStates~}

*/
void MultiRewriteLI::getStartStates(set<int> &states) {
  states.clear();
  states.insert(0);
  map<int, int>::iterator it;
  for (it = state2Pat.begin(); it != state2Pat.end(); it++) {
    if (it->second < 0) {
      states.insert(it->first);
    }
  }
}

/*
\subsection{Destructor for class ~MultiRewriteLI~}

*/
MultiRewriteLI::~MultiRewriteLI() {
  if (match) {
    match->deleteSetMatrix();
    delete match;
  }
  match = 0;
  if (ml) {
    deleteIfAllowed(ml);
  }
  ml = 0;
  if (streamOpen) {
    mlStream.close();
  }
  for (unsigned int i = 0; i < easyConds.size(); i++) {
    easyConds[i].deleteOpTree();
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
\subsection{Value Mapping without index}

*/
int classifyVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  ClassifyLI *li = (ClassifyLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      MLabel* source = static_cast<MLabel*>(args[0].addr);
      if (source) {
        if (source->IsDefined()) {
          local.addr = new ClassifyLI(source, args[1]);
        }
      }
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->nextResultText() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct classifyInfo : OperatorInfo {
  classifyInfo() {
    name      = "classify";
    signature = "mlabel x classifier -> stream(text)";
    syntax    = "classify(_ , _)";
    meaning   = "Classifies a trajectory according to a classifier";
  }
};

/*
\section{Operator ~indexclassify~}

\subsection{Type Mapping}

*/
ListExpr indexclassifyTM(ListExpr args) {
  const string errMsg = "Expecting a relation, the name of an mlabel"
             " attribute of that relation, an invfile, and a classifier";
  if (nl->HasLength(args, 4)) {
    if (Classifier::checkType(nl->Fourth(args))) {
      if (Relation::checkType(nl->First(args))) {
        if (Tuple::checkType(nl->First(nl->Rest(nl->First(args))))
         && listutils::isSymbol(nl->Second(args))) {
          ListExpr attrType;
          ListExpr attrList = nl->Second(nl->First(nl->Rest(nl->First(args))));
          string attrName = nl->SymbolValue(nl->Second(args));
          int i = listutils::findAttribute(attrList, attrName, attrType);
          if (i == 0) {
            return listutils::typeError("Attribute " + attrName + " not found");
          }
          if (!MLabel::checkType(attrType)) {
            return listutils::typeError
                   ("Type " + nl->ToString(attrType) + " is invalid");
          }
          if (InvertedFile::checkType(nl->Third(args))) {
            ListExpr outputAttrs = nl->TwoElemList(
                       nl->TwoElemList(nl->SymbolAtom("Description"),
                                       nl->SymbolAtom(FText::BasicType())),
                       nl->TwoElemList(nl->SymbolAtom("Trajectory"), attrType));
            return nl->ThreeElemList(
              nl->SymbolAtom(Symbol::APPEND()),
              nl->OneElemList(nl->IntAtom(i - 1)),
              nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                          nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                          outputAttrs)));
          }
        }
      }
    }
  }
  return listutils::typeError(errMsg);
}

/*
\subsection{Constructor for class ~IndexClassifyLI~}

This constructor is used for the operator ~indexclassify~.

*/
IndexClassifyLI::IndexClassifyLI(Word _mlrel, InvertedFile *inv,
 Word _classifier, int _attrNr) : invFile(inv), attrNr(_attrNr), processedP(0) {
  mlRel = (Relation*)_mlrel.addr;
  c = (Classifier*)_classifier.addr;
  classifyTT = ClassifyLI::getTupleType();
}

/*
\subsection{Constructor for class ~IndexClassifyLI~}

This constructor is used for the operator ~indexmatches~.

*/
IndexClassifyLI::IndexClassifyLI(Word _mlrel, InvertedFile *inv, int _attrNr) :
                            c(0), invFile(inv), attrNr(_attrNr), processedP(0) {
  mlRel = (Relation*)_mlrel.addr;
  classifyTT = ClassifyLI::getTupleType();
}

/*
\subsection{Destructor for class ~IndexClassifyLI~}

*/
IndexClassifyLI::~IndexClassifyLI() {
  if (classifyTT) {
    delete classifyTT;
    classifyTT = 0;
  }
}

/*
\subsection{Function ~timesMatch~}

*/
bool IndexClassifyLI::timesMatch(TupleId tId, unsigned int ulId,
                                 set<string> ivs) {
  if (ivs.empty()) {
    return true;
  }
  ULabel ul = getUL(tId, ulId);
  return ::timesMatch(&ul.timeInterval, ivs);
}

/*
\subsection{Function ~applyPattern~}

*/
void IndexClassifyLI::applyPattern(Pattern *p) {
  InvertedFile::exactIterator* eit = 0;
  TupleId id;
  wordPosType wc;
  charPosType cc;
  vector<IndexMatchInfo> matchInfo;
  for (int i = 0; i < mlRel->GetNoTuples(); i++) {
    IndexMatchInfo imi(getMLsize(i + 1));
    matchInfo.push_back(imi);
  }
  int pSize = p->getSize();
  for (int i = 0; i < pSize; i++) { // iterate over pattern elements
    if (p->getElem(i).getW() == NO) {
      set<string> labels = p->getElem(i).getL();
      set<string>::iterator is;
      for (is = labels.begin(); is != labels.end(); is++) {
        eit = invFile->getExactIterator(*is, 4096);
        set<int> foundULs;
        TupleId lastId = UINT_MAX;
        while (eit->next(id, wc, cc)) {
          if (matchInfo[id - 1].isActive(i)) {
            if (timesMatch(id, wc, p->getElem(i).getI())) {
              if ((id == lastId) || (lastId == UINT_MAX)) {
                foundULs.insert(foundULs.end(), wc); // collect unit label ids
              }
              else { // new tuple
                matchInfo[lastId - 1].processSimple(foundULs);
                foundULs.clear();
                foundULs.insert(wc);
              }
              lastId = id;
            }
          }
        }
        if (!foundULs.empty()) {
          matchInfo[lastId - 1].processSimple(foundULs);
        }
        if (eit) {
          delete eit;
        }
      }
      if (labels.empty()) { // index cannot be exploited
        if (p->getElem(i).getI().empty()) { // empty unit pattern
          for (unsigned j = 0; j < matchInfo.size(); j++) {
            if (matchInfo[j].isActive(i)) {
              matchInfo[j].processSimple();
            }
          }
        }
        else { // only time information exists
          for (unsigned j = 0; j < matchInfo.size(); j++) {
            if (matchInfo[j].isActive(i)) {
              if (matchInfo[j].range) {
                matchInfo[j].range = false;
                matchInfo[j].items.clear();
                for (int k = matchInfo[j].start; k < matchInfo[j].size; k++) {
                  if (timesMatch(j + 1, k, p->getElem(i).getI())) {
                    matchInfo[j].insert(k + 1);
                  }
                }
              }
              else {
                for (set<int>::iterator it = matchInfo[j].items.begin();
                     it != matchInfo[j].items.end(); it++) {
                  if (timesMatch(j + 1, *it, p->getElem(i).getI())) {
                    matchInfo[j].insert(*it + 1);
                  }
                }
              }
              matchInfo[j].processed++;
            }
          }
        }
      }
    }
    else { // PLUS or STAR
      for (unsigned j = 0; j < matchInfo.size(); j++) {
        matchInfo[j].processWildcard(p->getElem(i).getW(), i);
      }
    }
//     for (unsigned j = 0; j < matchInfo.size(); j++) {
//       matchInfo[j].print(j, i);
//     }
//     if (i == pSize - 1) {
//       cout << endl;
//     }
  }
  for (unsigned j = 1; j <= matchInfo.size(); j++) {
    if (matchInfo[j - 1].matches(pSize)) {
      classification.push(make_pair(p->getDescr(), j));
//       cout << "pushed back (" << p->getDescr() << ", " << j << ")" << endl;
    }
  }
}

/*
\subsection{Function ~getMLsize~}

*/
int IndexClassifyLI::getMLsize(TupleId tId) {
  Tuple* tuple = mlRel->GetTuple(tId, false);
  int result = ((MLabel*)tuple->GetAttribute(attrNr))->GetNoComponents();
  deleteIfAllowed(tuple);
  return result;
}

/*
\subsection{Function ~getUL~}

*/
ULabel IndexClassifyLI::getUL(TupleId tId, unsigned int ulId) {
  ULabel result(1);
  Tuple* tuple = mlRel->GetTuple(tId, false);
  ((MLabel*)tuple->GetAttribute(attrNr))->Get(ulId, result);
  deleteIfAllowed(tuple);
  return result;
}

/*
\subsection{Function ~nextResultTuple~}

This function is used for the operators ~indexclassify~.

*/
Tuple* IndexClassifyLI::nextResultTuple() {
  if (!mlRel->GetNoTuples()) { // no mlabel => no result
    return 0;
  }
  pair<string, TupleId> resultPair;
  Pattern* p = 0;
  while (processedP <= c->getNumOfP()) {
    if (!classification.empty()) { // convert matched mlabel into result
      pair<string, TupleId> resultPair = classification.front();
      classification.pop();
      Tuple* tuple = mlRel->GetTuple(resultPair.second, false);
      MLabel* ml = (MLabel*)tuple->GetAttribute(attrNr)->Copy();
      Tuple *result = new Tuple(classifyTT);
      result->PutAttribute(0, new FText(true, resultPair.first));
      result->PutAttribute(1, ml);
      deleteIfAllowed(tuple);
      return result;
    }
    if (processedP == c->getNumOfP()) { // all patterns processed
      return 0;
    }
    p = Pattern::getPattern(c->getPatText(processedP), true);
    if (p) {
      if (p->hasConds() || p->containsRegEx()) {
        p->parseNFA();
        for (int i = 1; i <= mlRel->GetNoTuples(); i++) {
          Tuple *t = mlRel->GetTuple(i, false);
          MLabel *source = (MLabel*)t->GetAttribute(attrNr)->Copy();
          Match *match = new Match(p, source);
          if (match->matches() == TRUE) {
            classification.push(make_pair(p->getDescr(), i));
          }
        }
      }
      else {
        p->setDescr(c->getDesc(processedP));
        applyPattern(p);
      }
    }
    processedP++;
    if (p) {
      delete p;
      p = 0;
    }
  }
  return 0;
}

/*
\subsection{Value Mapping with index}

*/
int indexclassifyVM(Word* args, Word& result, int message, Word& local,
                    Supplier s){
  IndexClassifyLI *li = (IndexClassifyLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      InvertedFile *inv = static_cast<InvertedFile*>(args[2].addr);
      CcInt *attr = static_cast<CcInt*>(args[4].addr);
      if (!attr->IsDefined()) {
        cout << "undefined parameter(s)" << endl;
        local.addr = 0;
        return 0;
      }
      local.addr = new IndexClassifyLI(args[0], inv,args[3], attr->GetIntval());
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->nextResultTuple() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

struct indexclassifyInfo : OperatorInfo {
  indexclassifyInfo() {
    name      = "indexclassify";
    signature = "rel(tuple(..., mlabel, ...)) x attrname x invfile x classifier"
                " -> stream(tuple(string, mlabel))";
    syntax    = "_ indexclassify [_ , _ , _]";
    meaning   = "Classifies an indexed relation of trajectories according to a "
                " classifier";
  }
};

/*
\subsection{Implementation of struct ~IndexMatchInfo~}

*/
IndexMatchInfo::IndexMatchInfo(int s) : range(false), start(INT_MAX),
                                        processed(0), size(s) {
  items.insert(0);
}

void IndexMatchInfo::print(TupleId tId, int pE) {
  if (!isActive(pE)) {
    cout << "pE " << pE << " tId " << tId << ": inactive" << endl;
    return;
  }
  if (range) {
    cout << "pE " << pE << " tId " << tId << ": active from " << start << endl;
  }
  else {
    cout << "pE " << pE << " tId " << tId << ": active: {";
    for (set<int>::iterator i = items.begin(); i != items.end(); i++) {
      cout << *i << ",";
    }
    cout << "}" << endl;
  }
}

bool IndexMatchInfo::isActive(int patElem) {
  if (!range && items.empty()) {
    return false;
  }
  return processed >= patElem;
}

void IndexMatchInfo::processWildcard(Wildcard w, int patElem) {
  if (!isActive(patElem)) return;
  if (range) {
    start += (w == PLUS ? 1 : 0);
  }
  else {
    range = true;
    start = *(items.begin()) + (w == PLUS ? 1 : 0);
    items.clear();
  }
  processed++;
}

void IndexMatchInfo::processSimple(set<int> found) {
  if (range) {
    items.clear();
    for (set<int>::iterator it = found.begin(); it != found.end(); it++) {
      if (start <= *it) {
        items.insert(items.end(), *it + 1);
      }
    }
    range = false;
  }
  else {
    set<int> newItems;
    for (set<int>::iterator it = found.begin(); it != found.end(); it++) {
      if (items.count(*it)) {
        newItems.insert(newItems.end(), *it + 1);
      }
    }
    items = newItems;
  }
  processed++;
}

void IndexMatchInfo::processSimple() {
  if (range) {
    start++;
  }
  else {
    set<int> newItems;
    set<int>::iterator it, it2(newItems.begin());
    for (it = items.begin(); it != items.end(); it++) {
      it2 = newItems.insert(it2, *it + 1);
    }
    items = newItems;
  }
  processed++;
}

bool IndexMatchInfo::matches(int patSize) {
  if (!isActive(patSize)) {
    return false;
  }
  if (range) {
    return (start <= size);
  }
  return items.count(size);
}



/*
\section{Operator ~compress~}

\subsection{Type Mapping}

*/
ListExpr compressTM(ListExpr args) {
  const string errMsg = "Expecting mlabel or stream(mlabel).";
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError(errMsg);
  }
  ListExpr arg = nl->First(args);
  if (MLabel::checkType(arg) || Stream<MLabel>::checkType(arg)) {
    return arg;
  }
  return listutils::typeError(errMsg);
}

/*
\subsection{Selection Function}

*/
int compressSelect(ListExpr args) {
  ListExpr arg = nl->First(args);
  if (MLabel::checkType(arg)) return 0;
  if (Stream<MLabel>::checkType(arg)) return 1;
  return -1;
}

/*
\subsection{Value Mapping (for a single MLabel)}

*/
template<class T>
int compressVM_1(Word* args, Word& result, int message, Word& local,
                  Supplier s){
  T* source = static_cast<T*>(args[0].addr);
  result = qp->ResultStorage(s);
  T* res = (T*)result.addr;
  source->compress(*res);
  return  0;
}

/*
\subsection{Value Mapping (for a stream of MLabels)}

*/
template<class T>
int compressVM_Str(Word* args, Word& result, int message, Word& local,
                  Supplier s){
  switch (message) {
    case OPEN: {
      qp->Open(args[0].addr);
      return 0;
    }  
    case REQUEST: {
      Word arg;
      qp->Request(args[0].addr,arg);
      if (qp->Received(args[0].addr)) {
        T* mlabel =(T*) arg.addr;
        T* res = new T(true);
        mlabel->compress(*res);
        result.addr = res;
        return YIELD;
      }
      else {
        return CANCEL;
      }
    }
    case CLOSE:{
      qp->Close(args[0].addr); 
      return 0;
    }
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct compressInfo : OperatorInfo {
  compressInfo() {
    name      = "compress";
    signature = "mlabel -> mlabel";
    appendSignature("mstring -> mstring");
    appendSignature("stream(mlabel) -> stream(mlabel)");
    appendSignature("stream(mstring) -> stream(mstring)");
    syntax    = "compress(_)";
    meaning   = "Unites temporally subsequent units with the same label.";
  }
};


/*
\section{Operator ~fillgaps~}

\subsection{Type Mapping}

*/
ListExpr fillgapsTM(ListExpr args) {
  string errMsg = "Expecting one argument of type mlabel or mstring and one of"
                  " type integer.";
  if (nl->ListLength(args) != 2) {
    return listutils::typeError("Two arguments expected.");
  }
  ListExpr arg = nl->First(args);
  if ((MLabel::checkType(arg) || MPlace::checkType(arg)
    || Stream<MLabel>::checkType(arg) || Stream<MPlace>::checkType(arg))
    && CcInt::checkType(nl->Second(args))) {
    return arg;
  }
  else {
    return listutils::typeError(errMsg);
  }
}

/*
\subsection{Selection Function}

*/
int fillgapsSelect(ListExpr args) {
  ListExpr arg = nl->First(args);
  if (MLabel::checkType(arg)) return 0;
  if (MPlace::checkType(arg)) return 1;
  if (Stream<MLabel>::checkType(arg)) return 2;
  if (Stream<MPlace>::checkType(arg)) return 3;
  return -1;
}

/*
\subsection{Value Mapping (for MLabel or MString)}

*/
template<class T>
int fillgapsVM_1(Word* args, Word& result, int message, Word& local,
                  Supplier s) {
  T* source = (T*)(args[0].addr);
  CcInt* ccDur = (CcInt*)(args[1].addr);
  result = qp->ResultStorage(s);
  T* res = (T*)result.addr;
  DateTime dur(0, ccDur->GetValue(), durationtype);
  source->Fill(*res, dur);
  return 0;
}

/*
\subsection{Value Mapping (for a stream of MLabels)}

*/
template<class T>
int fillgapsVM_Str(Word* args, Word& result, int message, Word& local,
                    Supplier s){
  CcInt* ccInt = 0;
  switch (message) {
    case OPEN: {
      qp->Open(args[0].addr);
      ccInt = (CcInt*)(args[1].addr);
      local.addr = ccInt;
      return 0;
    }
    case REQUEST: {
      if (!local.addr) {
        result.addr = 0;
        return CANCEL;
      }
      Word arg;
      qp->Request(args[0].addr, arg);
      if (qp->Received(args[0].addr)) {
        ccInt = (CcInt*)local.addr;
        T* source =(T*)arg.addr;
        T* res = new T(1);
        DateTime dur(0, ccInt->GetValue(), durationtype);
        source->Fill(*res, dur);
        result.addr = res;
        return YIELD;
      }
      else {
        return CANCEL;
      }
    }
    case CLOSE:{
      qp->Close(args[0].addr);
      local.addr=0;
      return 0;
    }
  }
  return 0;
}

/*
\subsection{Operator Info for operator ~fillgaps~}

*/
struct fillgapsInfo : OperatorInfo {
  fillgapsInfo() {
    name      = "fillgaps";
    signature = "mlabel -> mlabel";
    appendSignature("mstring -> mstring");
    appendSignature("stream(mlabel) -> stream(mlabel)");
    appendSignature("stream(mstring) -> stream(mstring)");
    syntax    = "fillgaps(_)";
    meaning   = "Fills temporal gaps between two (not temporally) subsequent "
                "units inside the moving label if the labels coincide";
  }
};


/*
\section{operator ~createml~}

\subsection{Type Mapping}

*/
ListExpr createmlTM(ListExpr args) {
  const string errMsg = "Expecting an integer and a real.";
  if (nl->ListLength(args) != 2) {
    return listutils::typeError("Two arguments expected.");
  }
  if (nl->IsEqual(nl->First(args), CcInt::BasicType())
   && (nl->IsEqual(nl->Second(args), CcReal::BasicType())
    || CcInt::checkType(nl->Second(args)))) {
    return nl->SymbolAtom(MLabel::BasicType());
  }
  return NList::typeError(errMsg);
}

/*
\subsection{Value Mapping}

*/
int createmlVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  result = qp->ResultStorage(s);
  CcInt* ccint = static_cast<CcInt*>(args[0].addr);
  CcReal* ccreal = static_cast<CcReal*>(args[1].addr);
  MLabel* ml = static_cast<MLabel*>(result.addr);
  if (ccint->IsDefined() && ccreal->IsDefined()) {
    int size = ccint->GetValue();
    double rate = ccreal->GetValue();
    ml->createML(size, false, rate);
    ml->SetDefined(true);
  }
  else {
//     cout << "Error: undefined value." << endl;
    ml->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct createmlInfo : OperatorInfo {
  createmlInfo() {
    name      = "createml";
    signature = "int x real -> mlabel";
    syntax    = "createml(_,_)";
    meaning   = "Creates an MLabel, the size being determined by the first"
                "parameter. The second one is the rate of different entries.";
  }
};

/*
\section{Operator ~createmlrelation~}

\subsection{Type Mapping}

*/
ListExpr createmlrelationTM(ListExpr args) {
  if (nl->ListLength(args) != 3) {
    return listutils::typeError("Three arguments expected.");
  }
  if (nl->IsEqual(nl->First(args), CcInt::BasicType())
   && nl->IsEqual(nl->Second(args), CcInt::BasicType())
   && nl->IsEqual(nl->Third(args), CcString::BasicType())) {
    return nl->SymbolAtom(CcBool::BasicType());
  }
  return NList::typeError("Expecting two integers and a string.");
}

/*
\subsection{Value Mapping}

*/
int createmlrelationVM(Word* args, Word& result, int message, Word& local,
                        Supplier s) {
  CcInt* ccint1 = static_cast<CcInt*>(args[0].addr);
  CcInt* ccint2 = static_cast<CcInt*>(args[1].addr);
  CcString* ccstring = static_cast<CcString*>(args[2].addr);
  int number, size;
  string relName, errMsg;
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*)result.addr;
  if (ccstring->IsDefined() && ccint1->IsDefined() && ccint2->IsDefined()) {
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    relName = ccstring->GetValue();
    if (!sc->IsValidIdentifier(relName, errMsg, true)) { // check relation name
      cout << "Invalid relation name \"" << relName << "\"; " << errMsg << endl;
      res->Set(true, false);
      return 0;
    }
    number = ccint1->GetValue();
    size = ccint2->GetValue();
    ListExpr typeInfo = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
        nl->TwoElemList(nl->TwoElemList(nl->SymbolAtom("No"),
                                        nl->SymbolAtom(CcInt::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("Trajectory"),
                                       nl->SymbolAtom(MLabel::BasicType()))));
    ListExpr numTypeInfo = sc->NumericType(typeInfo);
    TupleType* type = new TupleType(numTypeInfo);
    Relation* rel = new Relation(type, false);
    Tuple* tuple;
    MLabel* ml;
    srand(time(0));
    for (int i = 0; i < number; i++) {
      tuple = new Tuple(type);
      ml = new MLabel(1);
      ml->createML(size, true);
      tuple->PutAttribute(0, new CcInt(true, i));
      tuple->PutAttribute(1, ml);
      rel->AppendTuple(tuple);
      tuple = 0;
      ml = 0;
    }
    Word relWord;
    relWord.setAddr(rel);
    sc->InsertObject(relName, "", nl->TwoElemList
            (nl->SymbolAtom(Relation::BasicType()), typeInfo), relWord, true);
    res->Set(true, true);
    type->DeleteIfAllowed();
  }
  else {
    cout << "Error: undefined value." << endl;
    res->Set(true, false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct createmlrelationInfo : OperatorInfo {
  createmlrelationInfo() {
    name      = "createmlrelation";
    signature = "int x int x string -> bool";
    syntax    = "createmlrelation(_ , _ , _)";
    meaning   = "Creates a relation containing arbitrary many synthetic moving"
                "labels of arbitrary size and stores it into the database.";
  }
};

// /*
// \section{Operator ~index~}
// 
// \subsection{Type Mapping}
// 
// */
// ListExpr createindexTM(ListExpr args) {
//   if (nl->ListLength(args) != 1) {
//     return listutils::typeError("One argument expected.");
//   }
//   if (MLabel::checkType(nl->First(args))) {
//     return nl->SymbolAtom(MLabel::BasicType());
//   }
//   return listutils::typeError("Argument type must be mlabel.");
// }
// 
// /*
// \subsection{Value Mapping}
// 
// */
// int createindexVM(Word* args, Word& result, int message, Word& local,
//                   Supplier s) {
//   MLabel* res = new MLabel(1);
//   ULabel ul(1);
//   set<size_t> positions;
//   set<string> labels;
//   MLabel* source = (MLabel*)(args[0].addr);
//   TrieNode* ptr = 0;
//   if (source->IsDefined()) {
//     for (int i = 0; i < source->GetNoComponents(); i++) {
//       source->Get(i, ul);
//       positions.insert(i);
//       labels.insert(ul.constValue.GetValue());
//       res->index.insert(ptr, ul.constValue.GetValue(), positions);
//       positions.clear();
//       res->Add(ul);
//     }
//   }
//   res->index.makePersistent(ptr);
//   res->index.removeTrie(ptr);
//   res->index.printDbArrays();
//   res->index.printContents(ptr, labels);
//   result.addr = res;
//   return 0;
// }
// 
// /*
// \subsection{Operator Info}
// 
// */
// struct createindexInfo : OperatorInfo {
//   createindexInfo() {
//     name      = "createindex";
//     signature = "mlabel -> mlabel";
//     syntax    = "createindex(_)";
//     meaning   = "Builds an index for a moving label.";
//   }
// };

/*
\section{Operator ~createtrie~}

\subsection{Type Mapping}

*/
ListExpr createtrieTM(ListExpr args) {
  if (nl->ListLength(args) != 2) {
    return listutils::typeError("Two arguments expected.");
  }
  if (Relation::checkType(nl->First(args))) {
    if (Tuple::checkType(nl->First(nl->Rest(nl->First(args))))) {
      ListExpr attrList =
               nl->First(nl->Rest(nl->First(nl->Rest(nl->First(args)))));
      ListExpr attrType;
      string attrName = nl->SymbolValue(nl->Second(args));
      int i = listutils::findAttribute(attrList, attrName, attrType);
      if (MLabel::checkType(attrType) || MString::checkType(attrType)) {
        bool ml = MLabel::checkType(attrType);
        return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                                 nl->TwoElemList(nl->IntAtom(i),
                                                 nl->BoolAtom(ml)),
                                 nl->SymbolAtom(InvertedFile::BasicType()));
      }
    }
  }
  return listutils::typeError
             ("Argument types must be rel(tuple(..., mlabel, ...)) x attrname");
}

/*
\subsection{Value Mapping}

*/
int createtrieVM(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  Relation *rel = (Relation*)(args[0].addr);
  Tuple *tuple = 0;
  MLabel *ml = 0;
  MString *ms = 0;
  result = qp->ResultStorage(s);
  InvertedFile* inv = (InvertedFile*)result.addr;
  inv->setParams(false, 1, "");
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
  appendcache::RecordAppendCache* cache = inv->createAppendCache(invCacheSize);
  TrieNodeCacheType* trieCache = inv->createTrieCache(trieCacheSize);
  for (int i = 0; i < rel->GetNoTuples(); i++) {
    tuple = rel->GetTuple(i + 1, false);
    if (((CcBool*)args[3].addr)->GetBoolval()) {
      ml = (MLabel*)tuple->GetAttribute(((CcInt*)args[2].addr)->GetIntval()-1);
      ULabel ul(1);
      string text;
      for (int j = 0; j < ml->GetNoComponents(); j++) {
        ml->Get(j, ul);
        ul.constValue.GetValue(text);
        inv->insertString(tuple->GetTupleId(), text, j, 0, cache, trieCache);
      }
    }
    else {
      ms = (MString*)tuple->GetAttribute(((CcInt*)args[2].addr)->GetIntval()-1);
      UString us(1);
      for (int j = 0; j < ms->GetNoComponents(); j++) {
        ms->Get(j, us);
        inv->insertString(tuple->GetTupleId(), us.constValue.GetValue(), j, 0,
                          cache, trieCache);
      }
    }
    tuple->DeleteIfAllowed();
  }
  delete trieCache;
  delete cache;
  return 0;
}

/*
\subsection{Operator Info}

*/
struct createtrieInfo : OperatorInfo {
  createtrieInfo() {
    name      = "createtrie";
    signature = "rel(tuple(..., mlabel, ...)) x attrname -> invfile";
    syntax    = "_ createtrie [ _ ]";
    meaning   = "Builds an index for a relation of numbered moving labels.";
  }
};

/*
\section{Operator ~triptompoint~}

\subsection{Type Mapping}

*/
ListExpr triptompointTM(ListExpr args) {
  if (nl->ListLength(args) != 1) {
    return listutils::typeError("One argument expected.");
  }
  if (!Stream<Tuple>::checkType(nl->First(args))) {
    return listutils::typeError("Argument is not a stream of tuples.");
  }
  ListExpr attrlist = nl->Second(nl->Second(nl->First(args)));
  string attrs[] = {"int", "int", "instant", "instant",
                    "real", "real", "real", "real"};
  int pos = 0;
  while (!nl->IsEmpty(attrlist)) {
    ListExpr first = nl->First(attrlist);
    attrlist = nl->Rest(attrlist);
    if (!listutils::isSymbol(nl->Second(first), attrs[pos])) {
      return listutils::typeError("Wrong attribute type at pos " + pos);
    }
    pos++;
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
\subsection{Value Mapping}

*/
int triptompointVM(Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*)result.addr;
  Stream<Tuple> src = static_cast<Stream<Tuple>* >(args[0].addr);
  src.open();
  Tuple *tuple = src.request();
  int moId(-1), tripId(-1);
  MPoint *mp = 0;
  ListExpr typeinfo = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
    nl->ThreeElemList(nl->TwoElemList(nl->SymbolAtom("Moid"),
                                      nl->SymbolAtom(CcInt::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Tripid"),
                                      nl->SymbolAtom(CcInt::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Trip"),
                                      nl->SymbolAtom(MPoint::BasicType()))));
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  ListExpr numtypeinfo = sc->NumericType(typeinfo);
  TupleType *tupletype = new TupleType(numtypeinfo);
  Relation *tripsrel = new Relation(tupletype, false);
  Tuple *resTuple = 0;
  int counter = 0;
  while (tuple) {
    counter++;
    if ((moId != ((CcInt*)(tuple->GetAttribute(0)))->GetValue()) ||
        (tripId != ((CcInt*)(tuple->GetAttribute(1)))->GetValue())) {
      if (resTuple) {
        resTuple->PutAttribute(0, new CcInt(true, moId));
        resTuple->PutAttribute(1, new CcInt(true, tripId));
        resTuple->PutAttribute(2, mp);
        tripsrel->AppendTuple(resTuple);
//         cout << "Tuple finished" << endl;
//         resTuple->Print(cout);
      }
      moId = ((CcInt*)(tuple->GetAttribute(0)))->GetValue();
      tripId = ((CcInt*)(tuple->GetAttribute(1)))->GetValue();
      mp = new MPoint(0);
      resTuple = new Tuple(tupletype);
    }
    SecInterval iv(*((Instant*)(tuple->GetAttribute(2))),
                   *((Instant*)(tuple->GetAttribute(3))));
    Point p1(true, ((CcReal*)(tuple->GetAttribute(4)))->GetValue(),
                   ((CcReal*)(tuple->GetAttribute(5)))->GetValue());
    Point p2(true, ((CcReal*)(tuple->GetAttribute(6)))->GetValue(),
                   ((CcReal*)(tuple->GetAttribute(7)))->GetValue());
    UPoint up(iv, p1, p2);
    mp->Add(up);
    deleteIfAllowed(tuple);
    tuple = src.request();
  }
  resTuple->PutAttribute(0, new CcInt(true, moId));
  resTuple->PutAttribute(1, new CcInt(true, tripId));
  resTuple->PutAttribute(2, mp);
  tripsrel->AppendTuple(resTuple);
//   cout << "last tuple appended" << endl;
//   resTuple->Print(cout);
  src.close();
  ListExpr reltype = nl->TwoElemList(nl->SymbolAtom(Relation::BasicType()),
                                     typeinfo);
  Word relWord;
  relWord.setAddr(tripsrel);
  sc->InsertObject("Trips", "", reltype, relWord, true);
  res->Set(true, true);
  return 0;
}

/*
\subsection{Operator Info}

*/
struct triptompointInfo : OperatorInfo {
  triptompointInfo() {
    name      = "triptompoint";
    signature = "stream(tuple(Moid: int, Tripid: int, Tstart: instant, Tend: "
                "instant, Xstart: real, Ystart: real, Xend: real, Yend: real"
                ")) -> bool";
    syntax    = "_ triptompoint";
    meaning   = "Builds a relation containing one mpoint for each trip.";
  }
};

/*
\section{Class ~SymbolicTrajectoryAlgebra~}

*/
  
class SymbolicTrajectoryAlgebra : public Algebra {
 public:
  SymbolicTrajectoryAlgebra() : Algebra() {

  AddTypeConstructor(&label);
  AddTypeConstructor(&ilabel);
  AddTypeConstructor(&ulabel);
  AddTypeConstructor(&mlabel);
  AddTypeConstructor(&labels);
  AddTypeConstructor(&ilabels);
  AddTypeConstructor(&ulabels);
  AddTypeConstructor(&mlabels);
  
  AddTypeConstructor(&place);
  AddTypeConstructor(&places);
  AddTypeConstructor(&iplace);
  AddTypeConstructor(&iplaces);
  AddTypeConstructor(&uplace);
  AddTypeConstructor(&uplaces);
  AddTypeConstructor(&mplace);
  AddTypeConstructor(&mplaces);

  label.AssociateKind(Kind::DATA());
  ilabel.AssociateKind(Kind::DATA());
  ilabel.AssociateKind(Kind::TEMPORAL());
  ulabel.AssociateKind(Kind::DATA());
  ulabel.AssociateKind(Kind::TEMPORAL());
  mlabel.AssociateKind(Kind::DATA());
  mlabel.AssociateKind(Kind::TEMPORAL());
  
  labels.AssociateKind(Kind::DATA());
  ilabels.AssociateKind(Kind::DATA());
  ilabels.AssociateKind(Kind::TEMPORAL());
  ulabels.AssociateKind(Kind::DATA());
  ulabels.AssociateKind(Kind::TEMPORAL());
  mlabels.AssociateKind(Kind::DATA());
  mlabels.AssociateKind(Kind::TEMPORAL());
  
  place.AssociateKind(Kind::DATA());    
  iplace.AssociateKind(Kind::DATA());
  iplace.AssociateKind(Kind::TEMPORAL());
  uplace.AssociateKind(Kind::DATA());
  uplace.AssociateKind(Kind::TEMPORAL());
  mplace.AssociateKind(Kind::DATA());
  mplace.AssociateKind(Kind::TEMPORAL());
  
  places.AssociateKind(Kind::DATA());
  iplaces.AssociateKind(Kind::DATA());
  iplaces.AssociateKind(Kind::TEMPORAL());
  uplaces.AssociateKind(Kind::DATA());
  uplaces.AssociateKind(Kind::TEMPORAL());
  mplaces.AssociateKind(Kind::DATA());
  mplaces.AssociateKind(Kind::TEMPORAL());

  AddTypeConstructor(&patternTC);
  AddTypeConstructor(&classifierTC);

  ValueMapping tolabelVMs[] = {tolabelVM<FText>, tolabelVM<CcString>, 0};
  
  AddOperator(tolabelInfo(), tolabelVMs, tolabelSelect, tolabelTM);

  AddOperator(tostringInfo(), tostringVM, tostringTM);

  AddOperator(totextInfo(), totextVM, totextTM);
  
  AddOperator(mstringtomlabelInfo(), mstringtomlabelVM, mstringtomlabelTM);

  ValueMapping containsVMs[] = {containsVM<Labels, Label>,
                                containsVM<Places, Place>, 0};
  AddOperator(containsInfo(), containsVMs, containsSelect, containsTM);
  
  ValueMapping toplaceVMs[] = {toplaceVM<CcString>, toplaceVM<FText>, 0};
  AddOperator(toplaceInfo(), toplaceVMs, toplaceSelect, toplaceTM);
  
  AddOperator(nameInfo(), nameVM, nameTM);
  
  AddOperator(refInfo(), refVM, refTM);

  ValueMapping the_unitSymbolicVMs[] = {the_unitSymbolicVM<Label, ULabel>,
    the_unitSymbolicVM<Labels, ULabels>, the_unitSymbolicVM<Place, UPlace>,
    the_unitSymbolicVM<Places, UPlaces>, 0};
  AddOperator(the_unitSymbolicInfo(), the_unitSymbolicVMs,
              the_unitSymbolicSelect, the_unitSymbolicTM);
  
  ValueMapping makemvalueSymbolicVMs[] = {makemvalueSymbolicVM<ULabel, MLabel>,
    makemvalueSymbolicVM<ULabels, MLabels>, makemvalueSymbolicVM<UPlace,MPlace>,
    makemvalueSymbolicVM<UPlaces, MPlaces>, 0};
  AddOperator(makemvalueSymbolicInfo(), makemvalueSymbolicVMs,
              makemvalueSymbolicSelect, makemvalueSymbolic_TM);
  
  ValueMapping passesSymbolicVMs[] = {passesSymbolicVM<MLabel, Label>,
    passesSymbolicVM<MLabels, Label>, passesSymbolicVM<MLabels, Labels>,
    passesSymbolicVM<MPlace, Place>, passesSymbolicVM<MPlaces, Places>, 0};
  AddOperator(passesSymbolicInfo(), passesSymbolicVMs, atPassesSymbolicSelect,
              passesSymbolicTM);
  
  ValueMapping atSymbolicVMs[] = {atSymbolicVM<MLabel, Label>,
    atSymbolicVM<MLabels, Label>, atSymbolicVM<MLabels, Labels>,
    atSymbolicVM<MPlace, Place>, atSymbolicVM<MPlaces, Place>,
    atSymbolicVM<MPlaces, Places>, 0};
  AddOperator(atSymbolicInfo(), atSymbolicVMs, atPassesSymbolicSelect, 
              atSymbolicTM);
  
  ValueMapping deftimeSymbolicVMs[] = {deftimeSymbolicVM<MLabel>,
    deftimeSymbolicVM<MLabels>, deftimeSymbolicVM<MPlace>,
    deftimeSymbolicVM<MPlaces>, 0};
  AddOperator(deftimeSymbolicInfo(), deftimeSymbolicVMs, 
              deftimeUnitsAtinstantSymbolicSelect, deftimeSymbolicTM);
  
  ValueMapping atinstantSymbolicVMs[] = {atinstantSymbolicVM<MLabel, ILabel>,
    atinstantSymbolicVM<MLabels, ILabels>, atinstantSymbolicVM<MPlace, IPlace>,
    atinstantSymbolicVM<MPlaces, IPlaces>, 0};
  AddOperator(atinstantSymbolicInfo(), atinstantSymbolicVMs,
              deftimeUnitsAtinstantSymbolicSelect, atinstantSymbolicTM);
  
  ValueMapping unitsSymbolicVMs[] = {unitsSymbolicVM<MLabel, ULabel>,
    unitsSymbolicVM<MLabels, ULabels>, unitsSymbolicVM<MPlace, UPlace>, 
    unitsSymbolicVM<MPlaces, UPlaces>, 0};
  AddOperator(unitsSymbolicInfo(), unitsSymbolicVMs, 
              deftimeUnitsAtinstantSymbolicSelect, unitsSymbolicTM);
  
  ValueMapping initialSymbolicVMs[] = {initialSymbolicVM<ULabel, ILabel>,
    initialSymbolicVM<ULabels, ILabels>, initialSymbolicVM<MLabel, ILabel>,
    initialSymbolicVM<MLabels, ILabels>, initialSymbolicVM<UPlace, IPlace>,
    initialSymbolicVM<UPlaces, IPlaces>, initialSymbolicVM<MPlace, IPlace>,
    initialSymbolicVM<MPlaces, IPlaces>, 0};
  AddOperator(initialSymbolicInfo(), initialSymbolicVMs, 
              initialFinalSymbolicSelect, initialFinalSymbolicTM);
  
  ValueMapping finalSymbolicVMs[] = {finalSymbolicVM<ULabel, ILabel>,
    finalSymbolicVM<ULabels, ILabels>, finalSymbolicVM<MLabel, ILabel>,
    finalSymbolicVM<MLabels, ILabels>, finalSymbolicVM<UPlace, IPlace>,
    finalSymbolicVM<UPlaces, IPlaces>, finalSymbolicVM<MPlace, IPlace>,
    finalSymbolicVM<MPlaces, IPlaces>, 0};
  AddOperator(finalSymbolicInfo(), finalSymbolicVMs, 
              initialFinalSymbolicSelect, initialFinalSymbolicTM);
  
  ValueMapping valSymbolicVMs[] = {valSymbolicVM<ILabel, Label>,
    valSymbolicVM<ILabels, Labels>, valSymbolicVM<IPlace, Place>,
    valSymbolicVM<IPlaces, Places>, 0};
  AddOperator(valSymbolicInfo(), valSymbolicVMs, valInstSymbolicSelect,
              valSymbolicTM);
  
  ValueMapping instSymbolicVMs[] = {instSymbolicVM<ILabel>,
    instSymbolicVM<ILabels>, instSymbolicVM<IPlace>, instSymbolicVM<IPlaces>,
    0};
  AddOperator(instSymbolicInfo(), instSymbolicVMs, valInstSymbolicSelect,
              instSymbolicTM);
  
  ValueMapping insideSymbolicVMs[] = {insideSymbolicVM<MLabel, Labels>,
    insideSymbolicVM<MPlace, Places>, 0};
  AddOperator(insideSymbolicInfo(), insideSymbolicVMs, insideSymbolicSelect,
              insideSymbolicTM);
      
  AddOperator(topatternInfo(), topatternVM, topatternTM);

  AddOperator(toclassifierInfo(), toclassifierVM, toclassifierTM);

  AddOperator(&matches);
  matches.SetUsesArgsInTypeMapping();
  
  ValueMapping indexmatchesVMs[] = {indexmatchesVM_T, indexmatchesVM_P, 0};
  AddOperator(indexmatchesInfo(), indexmatchesVMs, indexmatchesSelect,
              indexmatchesTM);

  ValueMapping filtermatchesVMs[] = {filtermatchesVM_T,
                                      filtermatchesVM_P, 0};
  AddOperator(filtermatchesInfo(), filtermatchesVMs, filtermatchesSelect,
              filtermatchesTM);
  
  ValueMapping rewriteVMs[] = {rewriteVM_T, rewriteVM_P,
                                rewriteVM_Stream, 0};
  AddOperator(rewriteInfo(), rewriteVMs, rewriteSelect, rewriteTM);

  AddOperator(classifyInfo(), classifyVM, classifyTM);

  AddOperator(indexclassifyInfo(), indexclassifyVM, indexclassifyTM);

  ValueMapping compressVMs[] = {compressVM_1<MLabel>, compressVM_Str<MLabel>,0};
  AddOperator(compressInfo(), compressVMs, compressSelect, compressTM);

  ValueMapping fillgapsVMs[] = {fillgapsVM_1<MLabel>, fillgapsVM_1<MPlace>,
                             fillgapsVM_Str<MLabel>, fillgapsVM_Str<MPlace>, 0};
  AddOperator(fillgapsInfo(), fillgapsVMs, fillgapsSelect, fillgapsTM);

  AddOperator(createmlInfo(), createmlVM, createmlTM);

  AddOperator(createmlrelationInfo(), createmlrelationVM,
              createmlrelationTM);

//       AddOperator(createindexInfo(), createindexVM, createindexTM);

  AddOperator(createtrieInfo(), createtrieVM, createtrieTM);

//       AddOperator(triptompointInfo(), triptompointVM, triptompointTM);

  }
  
  ~SymbolicTrajectoryAlgebra() {}
};

// SymbolicTrajectoryAlgebra SymbolicTrajectoryAlgebra;

} // end of namespace ~stj~

extern "C"
Algebra* InitializeSymbolicTrajectoryAlgebra(NestedList *nlRef,
                                             QueryProcessor *qpRef) {
  return new stj::SymbolicTrajectoryAlgebra;
}
