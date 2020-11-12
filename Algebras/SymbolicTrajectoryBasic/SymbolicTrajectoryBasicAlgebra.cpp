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

[TOC]

\section{Overview}
This algebra contains basic types and operations for symbolic trajectories and
has been split from the SymbolicTrajectoryAlgebra in April 2020.

*/
#include "SymbolicTrajectoryBasicAlgebra.h"
#ifdef RECODE
#include <recode.h>
#endif

using namespace std;
using namespace temporalalgebra;
using namespace datetime;

namespace stj {

double BasicDistanceFuns::distance(const string& str1, const string& str2, 
                                   const LabelFunction lf /* = TRIVIAL */) {
  if (lf == TRIVIAL) {
    return (str1 == str2 ? 0.0 : 1.0);
  }
  if (str1.empty() && str2.empty()) {
    return 0.0;
  }
  if (str1.empty() || str2.empty()) {
    return 1.0;
  }
  double ld = 1.0;
  if (lf == EDIT) {
    ld = stringutils::ld(str1, str2);
  }
  return ld / max(str1.length(), str2.length());
}

double BasicDistanceFuns::distance(const pair<string, unsigned int>& val1, 
                                   const pair<string, unsigned int>& val2, 
                                   const LabelFunction lf /* = TRIVIAL */) {
  double ld = BasicDistanceFuns::distance(val1.first, val2.first, lf);
  return ld / 2 + (val1.second == val2.second ? 0 : 0.5);
}

double BasicDistanceFuns::distance(const set<string>& values1, 
                           const set<string>& values2, const LabelFunction lf) {
  if (values1.empty() && values2.empty()) {
    return 0;
  }
  if (values1.empty() || values2.empty()) {
    return 1;
  }
  set<string>::iterator i1(values1.begin()), i2(values2.begin());
  int m(values1.size()), n(values2.size());
  double distsum = 0.0;
  double dist;
  int i1count(0), i2count(0);
  while (i1 != values1.end() && i2 != values2.end()) {
    dist = BasicDistanceFuns::distance(*i1, *i2, lf);
    if (dist < 1) {
//       cout << "  " << *i1 << " = " << *i2 << endl;
      i1++;
      i1count++;
      i2++;
      i2count++;
      distsum += dist;
    }
    else {
      if (*i1 < *i2) {
//         cout << "  " << *i1 << " < " << *i2 << endl;
        i1++;
        i1count++;
      }
      else {
//         cout << "  " << *i1 << " > " << *i2 << endl;
        i2++;
        i2count++;
      }
      distsum += 1.0;
    }
  }
  distsum += std::max(m - i1count, n - i2count);
//   cout << "distsum = " << distsum << "      ";
  return distsum / (m + n);
}

double BasicDistanceFuns::distance(set<pair<string, unsigned int> >& values1, 
                                   set<pair<string, unsigned int> >& values2, 
                                   const LabelFunction lf /* = TRIVIAL */) {
  if (values1.empty() && values2.empty()) {
    return 0;
  }
  if (values1.empty() || values2.empty()) {
    return 1;
  }
  set<pair<string, unsigned int> >::iterator i1, i2;
  double distsum = 0;
  for (i1 = values1.begin(); i1 != values1.end(); i1++) {
    for (i2 = values2.begin(); i2 != values2.end(); i2++) {
      double dist = BasicDistanceFuns::distance(i1->first, i2->first, lf);
      distsum += dist / 2 + (i1->second == i2->second ? 0 : 0.5);
    }
  }
  return distsum / (values1.size() * values2.size());
}

/*
\subsection{Function ~recode~}

*/
#ifdef RECODE
bool RecodeFun::recode(const string &src, const string &from, const string &to,
                      string &result) {
  string rs = trim(from)+".."+trim(to);
  // use recode lib

  RECODE_OUTER outer = recode_new_outer(true);
  RECODE_REQUEST request = recode_new_request(outer);

  bool success = recode_scan_request(request, rs.c_str());
  if (!success) {
    recode_delete_request(request);
    recode_delete_outer(outer);
    result.clear();
    return false;
  }
  char* recoded = recode_string(request, src.c_str());

  // make clean
  recode_delete_request(request);
  recode_delete_outer(outer);
  if (recoded == 0) {
    result.clear();
    return false;
  }
  result = recoded;
  free(recoded);
  return true;
}
#endif

/*
\section{Implementation of class ~Label~}

\subsection{Constructor}

*/
Label::Label(const Label& rhs) : Attribute(rhs.IsDefined()), 
                                 value(rhs.value.getSize()) {
  CopyFrom(&rhs);
}

/*
\subsection{Function ~GetValue~}

*/
void Label::GetValue(string& text) const {
  assert(IsDefined());
  if (value.getSize() > 0) {
    char *bytes = new char[value.getSize()];
    value.read(bytes, value.getSize());
    string text2(bytes, value.getSize());
    delete[] bytes;
    text = text2;
  }
  else {
    text.clear();
  }
}

string Label::GetValue() const {
  assert(IsDefined());
  string result;
  GetValue(result);
  return result;
}

/*
\subsection{Function ~GetValues~}

*/
void Label::GetValues(set<string>& values) const {
  assert(IsDefined());
  string value;
  GetValue(value);
  values.clear();
  values.insert(value);
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
  Clean();
  if (text.length() > 0) {
    const char *bytes = text.c_str();
    value.write(bytes, text.length());
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
\subsection{Function ~Distance~}

*/
double Label::Distance(const Label& lb, const LabelFunction lf /* = EDIT */)
                                                                         const {
  if (!IsDefined() && !lb.IsDefined()) {
    return 0;
  }
  string str1, str2;
  if (!IsDefined() || !lb.IsDefined()) {
    return 1;
  }
  GetValue(str1);
  lb.GetValue(str2);
  return BasicDistanceFuns::distance(str1, str2, lf);
}

/*
\subsection{Function ~InsertLabels~}

*/
void Label::InsertLabels(vector<string>& result) const {
  if (IsDefined()) {
    result.push_back(GetLabel());
  }
}

/*
\subsection{Function ~UpdateFrequencies~}

*/
void Label::UpdateFrequencies(InvertedFile& inv, vector<int>& fv) const {
  if (IsDefined()) {
    InvertedFile::exactIterator* eit = 0;
    TupleId id;
    uint32_t wc, cc;
    eit = inv.getExactIterator(GetLabel(), 16777216);
    if (eit->next(id, wc, cc)) {
      fv[id - 1]++;
    }
    delete eit;
  }
}

/*
\subsection{Function ~readValueFrom~}

*/
bool Label::readValueFrom(ListExpr LE, string& text, unitelem& unit) {
  if (nl->IsAtom(LE)) {
    nl->WriteToString(text, LE);
    if (text.length() == 0) {
      unit.pos = UINT_MAX;
    }
    return true;
  }
  return false;
}

/*
\subsection{Function ~ReadFrom~}

*/
bool Label::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  if (listutils::isSymbolUndefined(LE)) {
    SetDefined(false);
    return true;
  }
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
  if (!IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
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
  set<string> strings1, strings2;
  string str;
  for (int i = 0; i < GetNoValues(); i++) {
    GetValue(i, str);
    strings1.insert(str);
    src.GetValue(i, str);
    strings2.insert(str);
  }
  return strings1 == strings2;
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
  if (!Contains(text)) {
    if (text.length() > 0) {
      pos.Append(values.getSize());
      const char *bytes = text.c_str();
      values.write(bytes, text.length(), values.getSize());
    }
    else {
      pos.Append(UINT_MAX);
    }
  }
}

void Labels::Append(const set<string>& values) {
  for (set<string>::iterator it = values.begin(); it != values.end(); it++) {
    Append(*it);
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
  if (cur != UINT_MAX) {
    int j = i + 1;
    bool finished = false;
    while (!finished && (j < GetNoValues())) {
      pos.Get(j, next);
      if (next != UINT_MAX) {
        finished = true;
      }
      j++;
    }
    if (!finished) {
      next = GetLength();
    }
    char *bytes = new char[next - cur];
    values.read(bytes, next - cur, cur);
    string text2(bytes, next - cur);
    delete[] bytes;
    text = text2;
  }
  else {
    text.clear();
  }
}

/*
\subsection{Function ~GetValues~}

*/
void Labels::GetValues(set<string>& values) const {
  values.clear();
  string value;
  for (int i = 0; i < GetNoValues(); i++) {
    GetValue(i, value);
    values.insert(value);
  }
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
unsigned int Labels::getFlobPos(const arrayelem elem) {
  return elem;
}

/*
\subsection{Function ~valuesToListExpr~}

*/
void Labels::valuesToListExpr(const set<string>& values, ListExpr& result) {
  if (values.empty()) {
    result = nl->Empty();
    return;
  }
  set<string>::iterator it = values.begin();
  result = nl->OneElemList(nl->TextAtom(*it));
  it++;
  ListExpr last = result;
  while (it != values.end()) {
    last = nl->Append(last, nl->TextAtom(*it));
    it++;
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
bool Labels::Contains(const string& text) const {
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
\subsection{Function ~Intersects~}

*/
bool Labels::Intersects(const Labels &lbs) const {
  set<std::string> values1, values2;
  GetValues(values1);
  lbs.GetValues(values2);
  Labels res(true);
  res.Intersection(values1, values2);
  return !res.IsEmpty();
}

/*
\subsection{Function ~Union~}

*/
void Labels::Union(const set<string>& values1, const set<string>& values2) {
  SetDefined(true);
  Clean();
  Append(values1);
  Append(values2);
}

/*
\subsection{Function ~Intersection~}

*/
void Labels::Intersection(const set<string>& values1, 
                          const set<string>& values2) {
  SetDefined(true);
  Clean();
  set<string> intersection;
  set_intersection(values1.begin(), values1.end(), values2.begin(), 
              values2.end(), std::inserter(intersection, intersection.begin()));
  Append(intersection);
}

/*
\subsection{Function ~Minus~}

*/
void Labels::Minus(const set<string>& values1, const set<string>& values2) {
  SetDefined(true);
  Clean();
  set<string> difference;
  set_difference(values1.begin(), values1.end(), values2.begin(), 
                 values2.end(), std::inserter(difference, difference.begin()));
  Append(difference);
}

#ifdef RECODE
/*
\subsection{Function ~Recode~}

*/
bool Labels::Recode(const std::string& from, const std::string& to, 
                    Labels& result) {
  result.SetDefined(IsDefined());
  if (!IsDefined()) {
    return true;
  }
  result.Clean();
  string value, recoded;
  for (int i = 0; i < GetNoValues(); i++) {
    GetValue(i, value);
    if (!RecodeFun::recode(value, from, to, recoded)) {
      result.SetDefined(false);
      return false;
    }
    result.Append(recoded);
  }
  return true;
}
#endif

/*
\subsection{Operator ~<<~}

*/
ostream& operator<<(ostream& os, const Labels& lbs) {
  Label lb(true);
  string text;
  for (int i = 0; i < lbs.GetNoValues() - 1; i++) {
    lbs.GetValue(i, text);
    os << text << " ";
  }
  lbs.GetValue(lbs.GetNoValues() - 1, text);
  os << text;
  return os;
}

/*
\subsection{Function ~Distance~}

*/
double Labels::Distance(const Labels& lbs, const LabelFunction lf /* = EDIT */) 
                                                                         const {
  if (!IsDefined() && !lbs.IsDefined()) {
    return 0;
  }
  set<string> values1, values2;
  if (IsDefined()) {
    GetValues(values1);
  }
  if (lbs.IsDefined()) {
    lbs.GetValues(values1);
  }
  return BasicDistanceFuns::distance(values1, values2, lf);
}

/*
\subsection{Function ~InsertLabels~}

*/
void Labels::InsertLabels(vector<string>& result) const {
  if (IsDefined()) {
    set<std::string> values;
    GetValues(values);
    for (auto it : values) {
      result.push_back(it);
    }
  }
}

/*
\subsection{Function ~UpdateFrequencies~}

*/
void Labels::UpdateFrequencies(InvertedFile& inv, vector<int>& fv) const {
  if (IsDefined()) {
    set<string> values;
    GetValues(values);
    InvertedFile::exactIterator* eit = 0;
    TupleId id;
    uint32_t wc, cc;
    for (auto label : values) {
      eit = inv.getExactIterator(label, 16777216);
      if (eit->next(id, wc, cc)) {
        fv[id - 1]++;
      }
    }
    delete eit;
  }
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
  string text;
  if (nl->IsAtom(LE)) {
    text = nl->ToString(LE);
    Append(text.substr(1, text.length() - 2));
    return true;
  }
  ListExpr rest = LE;
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
\subsection{Function ~GetValues~}

*/
void Place::GetValues(set<pair<string, unsigned int> >& values) const {
  assert(IsDefined());
  pair<string, unsigned int> value;
  GetValue(value);
  values.clear();
  values.insert(value);
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
\subsection{Function ~Distance~}

*/
double Place::Distance(const Place& p, const LabelFunction lf) const {
  if (!IsDefined() && !p.IsDefined()) {
    return 0;
  }
  if (!IsDefined() || !p.IsDefined()) {
    return Label::Distance(p, lf);
  }
  pair<string, unsigned int> val1, val2;
  GetValue(val1);
  p.GetValue(val2);
  return BasicDistanceFuns::distance(val1, val2, lf);
}

/*
\subsection{Operator ~<<~}

*/
ostream& operator<<(ostream& os, const Place& pl) {
  std::pair<std::string, unsigned int> p;
  pl.GetValue(p);
  os << "(\'" << p.first << "\'," << p.second << ")";
  return os;
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
      if (text.length() == 0) {
        unit.pos = UINT_MAX;
      }
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
  if (!Contains(val)) {
    NewPair<unsigned int, unsigned int> pr;
    pr.first = (val.first.length() > 0 ? values.getSize() : UINT_MAX);
    pr.second = val.second;
    posref.Append(pr);
    if (val.first.length() > 0) {
      const char *bytes = val.first.c_str();
      values.write(bytes, val.first.length(), values.getSize());
    }
  }
}

void Places::Append(const Place& pl) {
  pair<string, unsigned int> val;
  pl.GetValue(val);
  Append(val);
}

void Places::Append(const set<base>& values) {
  for (set<base>::iterator it = values.begin(); it != values.end(); it++) {
    Append(*it);
  }
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
  posref.Get(i, cur);
  result.second = cur.second;
  if (cur.first != UINT_MAX) {
    int j = i + 1;
    bool finished = false;
    while (!finished && (j < GetNoValues())) {
      posref.Get(j, next);
      if (next.first != UINT_MAX) {
        finished = true;
      }
      j++;
    }
    if (!finished) {
      next.first = GetLength();
    }
    char *bytes = new char[next.first - cur.first];
    values.read(bytes, next.first - cur.first, cur.first);
    string text(bytes, next.first - cur.first);
    result.first = text;
    delete[] bytes; 
  }
  else {
    result.first.clear();
  }
}


/*
\subsection{Function ~GetValues~}

*/
void Places::GetValues(set<base>& values) const {
  values.clear();
  base value;
  for (int i = 0; i < GetNoValues(); i++) {
    GetValue(i, value);
    values.insert(value);
  }
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
\subsection{Function ~Intersects~}

*/
bool Places::Intersects(const Places &lbs) const {
  set<base> values1, values2;
  GetValues(values1);
  lbs.GetValues(values2);
  Places res(true);
  res.Intersection(values1, values2);
  return !res.IsEmpty();
}

/*
\subsection{Function ~Union~}

*/
void Places::Union(const set<base>& values1, const set<base>& values2) {
  SetDefined(true);
  Clean();
  Append(values1);
  Append(values2);
}

/*
\subsection{Function ~Intersection~}

*/
void Places::Intersection(const set<base>& values1, const set<base>& values2) {
  SetDefined(true);
  Clean();
  set<base> intersection;
  set_intersection(values1.begin(), values1.end(), values2.begin(), 
              values2.end(), std::inserter(intersection, intersection.begin()));
  Append(intersection);
}

/*
\subsection{Function ~Minus~}

*/
void Places::Minus(const set<base>& values1, const set<base>& values2) {
  SetDefined(true);
  Clean();
  set<base> difference;
  set_difference(values1.begin(), values1.end(), values2.begin(), 
                 values2.end(), std::inserter(difference, difference.begin()));
  Append(difference);
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
unsigned int Places::getFlobPos(const arrayelem elem) {
  return elem.first;
}

/*
\subsection{Function ~valuesToListExpr~}

*/
void Places::valuesToListExpr(const set<base>& values, ListExpr& result) {
  if (values.empty()) {
    result = nl->Empty();
    return;
  }
  set<base>::iterator it = values.begin();
  result = nl->OneElemList(nl->TwoElemList(nl->TextAtom(it->first),
                                           nl->IntAtom(it->second)));
  it++;
  ListExpr last = result;
  while (it != values.end()) {
    last = nl->Append(last, nl->TwoElemList(nl->TextAtom(it->first),
                                            nl->IntAtom(it->second)));
    it++;
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
\subsection{Function ~Distance~}

*/
double Places::Distance(const Places& p, const LabelFunction lf /* = EDIT */) 
                                                                         const {
  if (!IsDefined() && !p.IsDefined()) {
    return 0;
  }
  if (!IsDefined() || !p.IsDefined()) {
    return 1;
  }
  set<pair<string, unsigned int> > values1, values2;
  GetValues(values1);
  p.GetValues(values2);
  // TODO: return Tools::distance(values1, values2) and delete remainder
  if (values1.empty() && values2.empty()) {
    return 0;
  }
  if (values1.empty() || values2.empty()) {
    return 1;
  }
  set<pair<string, unsigned int> >::iterator i1, i2;
  multiset<double> dist;
  for (i1 = values1.begin(); i1 != values1.end(); i1++) {
    for (i2 = values2.begin(); i2 != values2.end(); i2++) {
      
      double labelDist = double(stringutils::ld(i1->first, i2->first)) /
                         max(i1->first.length(), i2->first.length());
      dist.insert(labelDist + (i1->second == i2->second ? 0 : 0.5));
    }
  }
  int limit = min(values1.size(), values2.size());
  multiset<double>::iterator it = dist.begin();
  double sum = 0;
  for (int k = 0; k < limit; k++) {
    sum += *it;
    it++;
  }
  return (sum / limit + abs((int64_t)values1.size() - (int64_t)values2.size())/ 
                        max(values1.size(), values2.size())) / 2;
}

/*
\subsection{Function ~InsertLabels~}

*/
void Places::InsertLabels(vector<string>& result) const {
  if (IsDefined()) {
    set<base> values;
    GetValues(values);
    for (auto it : values) {
      result.push_back(it.first);
    }
  }
}

/*
\subsection{Function ~UpdateFrequencies~}

*/
void Places::UpdateFrequencies(InvertedFile& inv, vector<int>& fv) const {
  if (IsDefined()) {
    InvertedFile::exactIterator* eit = 0;
    TupleId id;
    uint32_t wc, cc;
    set<base> values;
    GetValues(values);    
    for (auto place : values) {
      eit = inv.getExactIterator(place.first, 16777216);
      if (eit->next(id, wc, cc)) {
        fv[id - 1]++;
      }
    }
    delete eit;
  }
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
\subsection{Type Constructors}

*/
GenTC<Label> label;
GenTC<Labels> labels;
GenTC<Place> place;
GenTC<Places> places;
GenTC<IBasic<Label> > ilabel;
GenTC<IBasic<Place> > iplace;
GenTC<IBasics<Labels> > ilabels;
GenTC<IBasics<Places> > iplaces;
GenTC<UBasic<Label> > ulabel;
GenTC<UBasic<Place> > uplace;
GenTC<UBasics<Labels> > ulabels;
GenTC<UBasics<Places> > uplaces;
GenTC<MBasics<Labels> > mlabels;
GenTC<MBasics<Places> > mplaces;
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
\section{Operator ~tolabels~}

tolabels: (text)+ -> labels

\subsection{Type Mapping}

*/
ListExpr tolabelsTM(ListExpr args) {
  ListExpr rest = args;
  string firstType;
  while (!nl->IsEmpty(rest)) {
    if (rest == args) { // first value
      firstType = nl->ToString(nl->First(rest));
      if (!FText::checkType(nl->First(rest)) && 
          !CcString::checkType(nl->First(rest))) {
        return NList::typeError("Expecting only text or string elements.");
      }
    }
    else {
      if (nl->ToString(nl->First(rest)) != firstType) {
        return NList::typeError("Expecting only text or only string elements.");
      }
    }
    rest = nl->Rest(rest);
  }
  return nl->SymbolAtom(Labels::BasicType());
}

/*
\subsection{Selection Function}

*/
int tolabelsSelect(ListExpr args) {
  if (nl->IsEmpty(args)) return 0;
  if (FText::checkType(nl->First(args))) return 0;
  if (CcString::checkType(nl->First(args))) return 1;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template <class T>
int tolabelsVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  result = qp->ResultStorage(s);
  Labels* res = static_cast<Labels*>(result.addr);
  res->Clean();
  for (int i = 0; i < qp->GetNoSons(s); i++) {
    T* src = static_cast<T*>(args[i].addr);
    if (src->IsDefined()) {
      if (!src->GetValue().empty()) {
        res->Append(src->GetValue());
      }
    }
  }
  res->SetDefined(true);
  return 0;
}

/*
\subsection{Operator Info}

*/
struct tolabelsInfo : OperatorInfo {
  tolabelsInfo() {
    name      = "tolabels";
    signature = "T x ... x T -> labels, where T in {text, string}";
    syntax    = "tolabels( _ , ..., _ );";
    meaning   = "Creates a labels from a text or string list.";
  }
};

/*
\section{Operator ~toplaces~}

toplaces: text x int x ... x text x int -> places
toplaces: place x ... x place -> places

\subsection{Type Mapping}

*/
ListExpr toplacesTM(ListExpr args) {
  ListExpr rest = args;
  bool place;
  if (!nl->IsEmpty(rest)) {
    place = Place::checkType(nl->First(rest));
  }
  while (!nl->IsEmpty(rest)) {
    if (place) {
      if (!Place::checkType(nl->First(rest))) {
        return NList::typeError("Expecting a list of places.");
      }
    }
    else {
      if (!FText::checkType(nl->First(rest))) {
        return NList::typeError("Expecting a list of text x int pairs.");
      }
      rest = nl->Rest(rest);
      if (nl->IsEmpty(rest)) {
        return NList::typeError("Expecting a list of text x int pairs.");
      }
      if (!CcInt::checkType(nl->First(rest))) {
        return NList::typeError("Expecting a list of text x int pairs.");
      }
    }
    rest = nl->Rest(rest);
  }
  return nl->SymbolAtom(Places::BasicType());
}

/*
\subsection{Selection Function}

*/
int toplacesSelect(ListExpr args) {
  if (nl->IsEmpty(args)) {
    return 0;
  }
  return (Place::checkType(nl->First(args)) ? 0 : 1);
}

/*
\subsection{Value Mapping}

*/
int toplacesVM_P(Word* args, Word& result, int message, Word& local,Supplier s){
  result = qp->ResultStorage(s);
  Places* res = static_cast<Places*>(result.addr);
  res->Clean();
  for (int i = 0; i < qp->GetNoSons(s); i++) {
    Place* src = static_cast<Place*>(args[i].addr);
    if (src->IsDefined()) {
      res->Append(*src);
    }
  }
  res->SetDefined(true);
  return 0;
}

int toplacesVM_T(Word* args, Word& result, int message, Word& local,Supplier s){
  result = qp->ResultStorage(s);
  Places* res = static_cast<Places*>(result.addr);
  res->Clean();
  for (int i = 0; i < qp->GetNoSons(s); i = i + 2) {
    FText *src1 = static_cast<FText*>(args[i].addr);
    CcInt *src2 = static_cast<CcInt*>(args[i + 1].addr);
    if (src1->IsDefined() && src2->IsDefined()) {
      res->Append(make_pair(src1->GetValue(), src2->GetIntval()));
    }
  }
  res->SetDefined(true);
  return 0;
}

/*
\subsection{Operator Info}

*/
struct toplacesInfo : OperatorInfo {
  toplacesInfo() {
    name      = "toplaces";
    signature = "((text x int) ... (text x int)) -> places";
    appendSignature("place x ... x place -> places");
    syntax    = "toplaces( _, ..., _ );";
    meaning   = "Creates a places from a list of (text, int) pairs or places.";
  }
};

/*
\section{Operator ~collect\_labels~}

stream(T) x bool -> labels,   where T in {label, string, text}

\subsection{Type Mapping}

*/
ListExpr collect_labelsTM(ListExpr args) {
  if (!nl->HasLength(args, 2)) {
    return listutils::typeError("Two arguments expected");
  }
  if (!Stream<Label>::checkType(nl->First(args)) &&
      !Stream<CcString>::checkType(nl->First(args)) &&
      !Stream<FText>::checkType(nl->First(args))) {
    return listutils::typeError("First argument must be a stream of label, "
                                "string, or text");
  }
  if (!CcBool::checkType(nl->Second(args))) {
    return listutils::typeError("Second argument must be a bool");
  }
  return nl->SymbolAtom(Labels::BasicType());
}

/*
\subsection{Selection Function}

*/
int collect_labelsSelect(ListExpr args) {
  if (Label::checkType(nl->Second(nl->First(args))))    return 0;
  if (CcString::checkType(nl->Second(nl->First(args)))) return 1;
  if (FText::checkType(nl->Second(nl->First(args))))    return 2;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template <class T>
int collect_labelsVM(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  result = qp->ResultStorage(s);
  Labels* res = static_cast<Labels*>(result.addr);
  res->Clean();
  Stream<T> stream = static_cast<Stream<T> >(args[0].addr);
  CcBool *ccignoreundef = static_cast<CcBool*>(args[1].addr);
  if (!ccignoreundef->IsDefined()) {
    res->SetDefined(false);
    return 0;
  }
  bool ignoreundef = ccignoreundef->GetValue();
  res->SetDefined(true);
  stream.open();
  T *elem = stream.request();
  while (elem != 0) {
    if (!elem->IsDefined() && !ignoreundef) {
      res->SetDefined(false);
      return 0;
    }
    else {
      string elemstr = elem->GetValue();
      res->Append(elemstr);
    }
    elem = stream.request();
  }
  stream.close();
  return 0;
}

/*
\subsection{Operator Info}

*/
struct collect_labelsInfo : OperatorInfo {
  collect_labelsInfo() {
    name      = "collect_labels";
    signature = "stream(T) x bool -> labels, where T in {label, string, text}";
    syntax    = "_ collect_labels[ _ ];";
    meaning   = "Collects the stream elements into a labels value. If the "
                "boolean parameter is true, undefined values are ignored. "
                "Otherwise, the result is defined only if all elements are.";
  }
};

/*
\section{Operator ~contains~}

contains: labels x {label(s), string, text} -> bool
contains: places x place(s) -> bool

\subsection{Type Mapping}

*/
ListExpr containsTM(ListExpr args) {
  const string errMsg = "Expecting labels x label(s) or places x place(s).";
  if (nl->ListLength(args) != 2) {
    return listutils::typeError("Two arguments expected.");
  }
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  if (Labels::checkType(first)) {
    if (Label::checkType(second) || Labels::checkType(second) ||
        CcString::checkType(second) || FText::checkType(second)) {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }
  if (Places::checkType(first)) {
    if (Place::checkType(second) || Places::checkType(second)) {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }
  return NList::typeError(errMsg);
}

/*
\subsection{Selection Function}

*/
int containsSelect(ListExpr args) {
  if (Labels::checkType(nl->First(args))) {
    if (Label::checkType(nl->Second(args))) {
      return 0;
    }
    if (Labels::checkType(nl->Second(args))) {
      return 2;
    }
    if (CcString::checkType(nl->Second(args))) {
      return 4;
    }
    if (FText::checkType(nl->Second(args))) {
      return 5;
    }
  }
  if (Places::checkType(nl->First(args))) {
    return Place::checkType(nl->Second(args)) ? 1 : 3;
  }
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class Collection, class Value>
int containsSingleVM(Word* args, Word& result, int message, Word& local, 
                     Supplier s) {
  Collection *coll = static_cast<Collection*>(args[0].addr);
  Value* val = static_cast<Value*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  if (coll->IsDefined() && val->IsDefined()) {
    typename Value::base value;
    val->GetValue(value);
    res->Set(true, coll->Contains(value));
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

template<class Value>
int containsBasicVM(Word* args, Word& result, int message, Word& local, 
                    Supplier s) {
  Labels *lbs = static_cast<Labels*>(args[0].addr);
  Value* val = static_cast<Value*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  if (lbs->IsDefined() && val->IsDefined()) {
    std::string value = val->GetValue();
    res->Set(true, lbs->Contains(value));
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

template<class Collection, class Values>
int containsMultiVM(Word* args, Word& result, int message, Word& local,
                    Supplier s) {
  Collection *coll = static_cast<Collection*>(args[0].addr);
  Values* vals = static_cast<Values*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  if (coll->IsDefined() && vals->IsDefined()) {
    res->Set(true, true);
    for (int i = 0; i < vals->GetNoValues(); i++) {
      typename Values::base value;
      vals->GetValue(i, value);
      if (!coll->Contains(value)) {
        res->Set(true, false);
        return 0;
      }
    }
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct containsInfo : OperatorInfo {
  containsInfo() {
    name      = "contains";
    signature = "labels x {label(s), string, text} -> bool";
    appendSignature("places x place(s) -> bool");
    syntax    = "_ contains _;";
    meaning   = "Checks whether a labels/places object contains a label/place.";
  }
};

/*
\section{Operator ~intersects~}

intersects: T x T -> bool,   where T in \{labels, places\}

\subsection{Type Mapping}

*/
ListExpr intersectsTM(ListExpr args) {
  const string errMsg = "Expecting labels x labels or places x places.";
  if (!nl->HasLength(args, 2)) {
    return listutils::typeError(errMsg);
  }
  if ((Labels::checkType(nl->First(args)) && 
       Labels::checkType(nl->Second(args))) ||
      (Places::checkType(nl->First(args)) &&
       Places::checkType(nl->Second(args)))) {
    return nl->SymbolAtom(CcBool::BasicType());
  }
  return listutils::typeError(errMsg);
}

/*
\subsection{Selection Function}

*/
int intersectsSelect(ListExpr args) {
  return Labels::checkType(nl->First(args)) ? 0 : 1;
}

/*
\subsection{Value Mapping}

*/
template<class T>
int intersectsVM(Word* args, Word& result, int message, Word& local,
                 Supplier s) {
  T *first = static_cast<T*>(args[0].addr);
  T *second = static_cast<T*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  if (first->IsDefined() && second->IsDefined()) {
    res->Set(true, first->Intersects(*second));
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct intersectsInfo : OperatorInfo {
  intersectsInfo() {
    name      = "intersects";
    signature = "labels x labels -> bool";
    appendSignature("places x places -> bool");
    syntax    = "_ intersects _;";
    meaning   = "Checks whether two labels/places values have a non-empty "
                "intersection";
  }
};


/*
\section{Operator ~toplace~}

toplace: (string | text) x int -> place

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
  return NList::typeError("Expecting (string | text) x int.");
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
\section{Operator ~ref~}

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
\section{Operator ~=~}

=: T x T -> bool,   where T in {place(s), label(s)}

\subsection{Type Mapping}

*/
ListExpr equalsUnequalsTM(ListExpr args) {
  if (nl->ListLength(args) != 2) {
    return NList::typeError("Expecting two arguments.");
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if ((Label::checkType(arg1) && (Label::checkType(arg2) ||
         CcString::checkType(arg2) || FText::checkType(arg2))) ||
      (Label::checkType(arg2) && (Label::checkType(arg1) ||
         CcString::checkType(arg1) || FText::checkType(arg1))) ||
      (Labels::checkType(arg1) && Labels::checkType(arg2)) ||
      (Place::checkType(arg1) && Place::checkType(arg2)) ||
      (Places::checkType(arg1) && Places::checkType(arg2))) {
      return nl->SymbolAtom(CcBool::BasicType());
  }
  return NList::typeError("Expecting T x T, where T in {place(s), label(s)}");
}

/*
\subsection{Selection Function}

*/
int equalsUnequalsSelect(ListExpr args) {
  if (Label::checkType(nl->First(args))) {
    if (Label::checkType(nl->Second(args)))    return 0;
    if (CcString::checkType(nl->Second(args))) return 4;
    if (FText::checkType(nl->Second(args)))    return 5;
  }
  if (Label::checkType(nl->Second(args))) {
    if (CcString::checkType(nl->First(args))) return 6;
    if (FText::checkType(nl->First(args)))    return 7;
  }
  if (Labels::checkType(nl->First(args))) return 1;
  if (Place::checkType(nl->First(args)))  return 2; 
  if (Places::checkType(nl->First(args))) return 3;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class T>
int equalsVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  T *first = static_cast<T*>(args[0].addr);
  T *second = static_cast<T*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool *res = static_cast<CcBool*>(result.addr);
  if (first->IsDefined() && second->IsDefined()) {
    res->Set(true, *first == *second);
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

template<class T, class U>
int equalsVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  T *first = static_cast<T*>(args[0].addr);
  U *second = static_cast<U*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool *res = static_cast<CcBool*>(result.addr);
  cout << T::BasicType() << " " << U::BasicType() << endl;
  cout << second->IsDefined() << endl;
  cout << first->IsDefined() << endl;
  if (first->IsDefined() && second->IsDefined()) {
    cout << second->GetValue() << endl;
    cout << first->GetValue() << endl;
    res->Set(true, first->GetValue() == second->GetValue());
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct equalsInfo : OperatorInfo {
  equalsInfo() {
    name      = "=";
    signature = "T x T -> bool,   where T in {label(s), place(s)}";
    syntax    = "_ = _;";
    meaning   = "Checks whether both objects are equal.";
  }
};

/*
\section{Operator ~\#~}

\#: T x T -> bool,   where T in {place(s), label(s)}

\subsection{Value Mapping}

*/
template<class T>
int unequalsVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  T *first = static_cast<T*>(args[0].addr);
  T *second = static_cast<T*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool *res = static_cast<CcBool*>(result.addr);
  if (first->IsDefined() && second->IsDefined()) {
    res->Set(true, !(*first == *second));
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

template<class T, class U>
int unequalsVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  T *first = static_cast<T*>(args[0].addr);
  U *second = static_cast<U*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool *res = static_cast<CcBool*>(result.addr);
  if (first->IsDefined() && second->IsDefined()) {
    res->Set(true, !(first->GetValue() == second->GetValue()));
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct unequalsInfo : OperatorInfo {
  unequalsInfo() {
    name      = "#";
    signature = "T x T -> bool,   where T in {label(s), place(s)}";
    syntax    = "_ = _;";
    meaning   = "Checks whether both objects are unequal.";
  }
};

/*
\section{Operator ~union~}

union: T x T -> (labels|places),   where T in {place(s), label(s)}

\subsection{Type Mapping}

*/
ListExpr unionTM(ListExpr args) {
  if (nl->ListLength(args) != 2) {
    return NList::typeError("Expecting two arguments.");
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if ((Label::checkType(arg1) && Label::checkType(arg2)) || 
      (Labels::checkType(arg1) && Label::checkType(arg2)) ||
      (Label::checkType(arg1) && Labels::checkType(arg2)) ||
      (Labels::checkType(arg1) && Labels::checkType(arg2))) {
    return nl->SymbolAtom(Labels::BasicType());
  }
  if ((Place::checkType(arg1) && Place::checkType(arg2)) || 
      (Places::checkType(arg1) && Place::checkType(arg2)) ||
      (Place::checkType(arg1) && Places::checkType(arg2)) ||
      (Places::checkType(arg1) && Places::checkType(arg2))) {
    return nl->SymbolAtom(Places::BasicType());
  }
  return NList::typeError("Expecting T x T, where T in {place(s), label(s)}");
}

/*
\subsection{Selection Function}

*/
int unionSelect(ListExpr args) {
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (Label::checkType(arg1) && Label::checkType(arg2))   return 0;
  if (Labels::checkType(arg1) && Label::checkType(arg2))  return 1;
  if (Label::checkType(arg1) && Labels::checkType(arg2))  return 2;
  if (Labels::checkType(arg1) && Labels::checkType(arg2)) return 3;
  if (Place::checkType(arg1) && Place::checkType(arg2))   return 4;
  if (Places::checkType(arg1) && Place::checkType(arg2))  return 5;
  if (Place::checkType(arg1) && Places::checkType(arg2))  return 6;
  if (Places::checkType(arg1) && Places::checkType(arg2)) return 7;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class T, class U, class V>
int unionVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  T *first = static_cast<T*>(args[0].addr);
  U *second = static_cast<U*>(args[1].addr);
  result = qp->ResultStorage(s);
  V *res = static_cast<V*>(result.addr);
  if (first->IsDefined() && second->IsDefined()) {
    set<typename V::base> values1, values2;
    first->GetValues(values1);
    second->GetValues(values2);
    res->Union(values1, values2);
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct unionInfo : OperatorInfo {
  unionInfo() {
    name      = "union";
    signature = "T x T -> (labels|places),   where T in {label(s), place(s)}";
    syntax    = "_ union _;";
    meaning   = "Computes the union of both arguments.";
  }
};

/*
\section{Operator ~intersection~}

intersection: T x T -> T,   where T in {places, labels}

\subsection{Type Mapping}

*/
ListExpr intersectionTM(ListExpr args) {
  if (nl->ListLength(args) != 2) {
    return NList::typeError("Expecting two arguments.");
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (Labels::checkType(arg1) && Labels::checkType(arg2)) {
    return nl->SymbolAtom(Labels::BasicType());
  }
  if (Places::checkType(arg1) && Places::checkType(arg2)) {
    return nl->SymbolAtom(Places::BasicType());
  }
  return NList::typeError("Expecting T x T, where T in {places, labels}");
}

/*
\subsection{Selection Function}

*/
int intersectionSelect(ListExpr args) {
  if (Labels::checkType(nl->First(args))) return 0;
  if (Places::checkType(nl->First(args))) return 1;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class T>
int intersectionVM(Word* args, Word& result, int message, Word& local, 
                   Supplier s) {
  T *first = static_cast<T*>(args[0].addr);
  T *second = static_cast<T*>(args[1].addr);
  result = qp->ResultStorage(s);
  T *res = static_cast<T*>(result.addr);
  if (first->IsDefined() && second->IsDefined()) {
    set<typename T::base> values1, values2;
    first->GetValues(values1);
    second->GetValues(values2);
    res->Intersection(values1, values2);
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct intersectionInfo : OperatorInfo {
  intersectionInfo() {
    name      = "intersection";
    signature = "T x T -> T,   where T in {labels, places}";
    syntax    = "intersection(_ , _);";
    meaning   = "Computes the intersection of both arguments.";
  }
};

/*
\section{Operator ~minus~}

minus: T x U -> T,   where T in {places, labels}, U in {place(s), label(s)}

\subsection{Type Mapping}

*/
ListExpr minusTM(ListExpr args) {
  if (nl->ListLength(args) != 2) {
    return NList::typeError("Expecting two arguments.");
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (Labels::checkType(arg1) &&
     (Label::checkType(arg2) || Labels::checkType(arg2))) {
    return nl->SymbolAtom(Labels::BasicType());
  }
  if (Places::checkType(arg1) && 
     (Place::checkType(arg2) || Places::checkType(arg2))) {
    return nl->SymbolAtom(Places::BasicType());
  }
  return NList::typeError("Expecting T x U, where T in {places, labels} and "
                          "U in {place(s), label(s)}");
}

/*
\subsection{Selection Function}

*/
int minusSelect(ListExpr args) {
  if (Labels::checkType(nl->First(args))) {
    if (Label::checkType(nl->Second(args)))  return 0;
    if (Labels::checkType(nl->Second(args))) return 1;
  }
  if (Places::checkType(nl->First(args))) {
    if (Place::checkType(nl->Second(args)))  return 2;
    if (Places::checkType(nl->Second(args))) return 3;
  }
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class T, class U>
int minusVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  T *first = static_cast<T*>(args[0].addr);
  U *second = static_cast<U*>(args[1].addr);
  result = qp->ResultStorage(s);
  T *res = static_cast<T*>(result.addr);
  if (first->IsDefined() && second->IsDefined()) {
    set<typename T::base> values1, values2;
    first->GetValues(values1);
    second->GetValues(values2);
    res->Minus(values1, values2);
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct minusInfo : OperatorInfo {
  minusInfo() {
    name      = "minus";
    signature = "T x U -> T,   where T in {labels, places}, "
                "U in {label(s), place(s)";
    syntax    = "_ minus _;";
    meaning   = "Computes the difference of both arguments.";
  }
};

#ifdef RECODE
/*
\section{Operator ~recode~}

recode: T x string x string -> T,   where T in {mlabel, mlabels}

\subsection{Type Mapping}

*/
ListExpr recodeTM(ListExpr args){
   string err = " T x string x string -> T,  T in {mlabel, mlabels} expected";
   if (!nl->HasLength(args, 3)) {
     return listutils::typeError(err);
   }
   if (!MLabel::checkType(nl->First(args)) && 
       !MLabels::checkType(nl->First(args))) {
     return listutils::typeError(err);
   }
   if (!CcString::checkType(nl->Second(args)) ||
       !CcString::checkType(nl->Third(args))) {
     return listutils::typeError(err);
   }
   return nl->First(args);
}

/*
\subsection{Selection Function}

*/
int recodeSelect(ListExpr args) {
  if (MLabel::checkType(nl->First(args)))  return 0;
  if (MLabels::checkType(nl->First(args))) return 1;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class T>
int recodeVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  result = qp->ResultStorage(s);
  T *src = static_cast<T*>(args[0].addr);
  CcString *from = static_cast<CcString*>(args[1].addr);
  CcString *to = static_cast<CcString*>(args[2].addr);
  T *res = static_cast<T*>(result.addr);
  if (!src->IsDefined() || !from->IsDefined() || !to->IsDefined()) {
    res->SetDefined(false);
    return 0;
  }
  else {
    src->Recode(from->GetValue(), to->GetValue(), *res);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct recodeInfo : OperatorInfo {
  recodeInfo() {
    name      = "recode";
    signature = "T x string x string -> T,   where T in {mlabel, mlabels}";
    syntax    = "_ recode [ _, _ ];";
    meaning   = "Recodes an mlabel(s) from one charset to another one.";
  }
};
#endif

/*
\section{Generic operators for ~[i|m|u] [label|place] [s]?~}

\subsection{Operator ~the\_unit~}

the\_unit: T x instant x instant x bool x bool -> uT,
           T x interval -> uT,
           with T in {label, labels, place, places}

\subsubsection{Type Mapping}

*/
ListExpr the_unitSymbolicTM(ListExpr args) {
  if(!(nl->HasMinLength(args,1))) 
     return listutils::typeError("Operator requires 1 argument at least!");
  if (nl->Equal(nl->Rest(args), nl->FourElemList(
    nl->SymbolAtom(Instant::BasicType()), nl->SymbolAtom(Instant::BasicType()),
    nl->SymbolAtom(CcBool::BasicType()), nl->SymbolAtom(CcBool::BasicType())))
  || nl->Equal(nl->Rest(args), 
               nl->OneElemList(nl->SymbolAtom(SecInterval::BasicType())))) {
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
    "'(T instant instant bool bool)', or \n"
    "'(T interval)'\n for T in {label, labels, place, places}.");
}

/*
\subsubsection{Selection Function}

*/
int the_unitSymbolicSelect(ListExpr args) {
  if (!SecInterval::checkType(nl->Second(args))) {
    if (Label::checkType(nl->First(args)))  return 0;
    if (Labels::checkType(nl->First(args))) return 1;
    if (Place::checkType(nl->First(args)))  return 2;
    if (Places::checkType(nl->First(args))) return 3;
  }
  if (Label::checkType(nl->First(args)))  return 4;
  if (Labels::checkType(nl->First(args))) return 5;
  if (Place::checkType(nl->First(args)))  return 6;
  if (Places::checkType(nl->First(args))) return 7;
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
  res->SetDefined(true);
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

template<class Value, class Unit>
int the_unitIvSymbolicVM(Word* args, Word& result, 
                       int message, Word& local, Supplier s) {
  result = (qp->ResultStorage(s));
  Unit *res = static_cast<Unit*>(result.addr);
  Value *value = static_cast<Value*>(args[0].addr);
  SecInterval *iv = static_cast<SecInterval*>(args[1].addr);
  if (!value->IsDefined() || !iv->IsDefined()) {
    res->SetDefined(false);
    return 0;
  }
  res->SetDefined(true);
  res->timeInterval = *iv;
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
    appendSignature("label x interval -> ulabel");
    appendSignature("labels x interval -> ulabels");
    appendSignature("place x interval -> uplace");
    appendSignature("places x interval -> uplaces");
    syntax    = "the_unit( _ _ _ _ _ );   the_unit( _ _ )";
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
                                     nl->StringAtom(nl->SymbolValue(attrtype))),
                           attrtype);
}

/*
\subsubsection{Selection Function}

*/
int makemvalueSymbolicSelect(ListExpr args) {
  ListExpr first, second, rest, listn, lastlistn, first2, second2, firstr;
  string argstr, argstr2, attrname, inputtype, inputname;
  first = nl->First(args);
  second = nl->Second(args);
  nl->WriteToString(argstr, first);
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
  if (inputtype == ULabel::BasicType()) return 0;
  if (inputtype == ULabels::BasicType()) return 1;
  if (inputtype == UPlace::BasicType()) return 2;
  if (inputtype == UPlaces::BasicType()) return 3;
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
//   m->StartBulkLoad();
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
//       Unit unit(*((Unit*)curAttr));
      m->MergeAdd(*unit); // in contrast to makemvalue2
    }
//     else {
//     cerr << endl << __PRETTY_FUNCTION__ << ": Dropping undef unit. " << endl;
//     }
    curTuple->DeleteIfAllowed();
    qp->Request(args[0].addr, curTupleWord);
  }
  m->EndBulkLoad(true, true);
  qp->Close(args[0].addr);
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
                "stream containing a ulabel(s) or uplace(s) attribute. No two "
                "unit time intervals may overlap. Undefined units are allowed "
                "and will be ignored. A stream without defined units will "
                "result in an \'empty\' moving object, not in an \'undef\'."
                "Consecutive units with equal values are compressed.";
  }
};

/*
\subsection{Operator ~makemvalue2~}

makemvalue: stream (tuple ((x1 t1)...(xi uT)...(xn tn))) xi -> mT,   with T in
{label, labels, place, places}

*/
/*
\subsubsection{Value Mapping}

*/
template<class Unit, class Mapping>
int makemvalue2SymbolicVM(Word* args, Word& result, int message,
                          Word& local, Supplier s) {
  Mapping* m;
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
      Unit unit(*((Unit*)curAttr));
      m->Add(unit); // in contrast to makemvalue
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
struct makemvalue2SymbolicInfo : OperatorInfo {
  makemvalue2SymbolicInfo() {
    name      = "makemvalue2";
    signature = "stream (tuple ((x1 t1)...(xi uT)...(xn tn))) xi -> mT,   with"
                "T in {label, labels, place, places}";
    syntax    = "_ makemvalue2[ _ ]";
    meaning   = "Creates a moving object from a (not necessarily sorted) tuple "
                "stream containing a ulabel(s) or uplace(s) attribute. No two "
                "unit time intervals may overlap. Undefined units are allowed "
                "and will be ignored. A stream without defined units will "
                "result in an \'empty\' moving object, not in an \'undef\'. "
                "Consecutive units with equal values are NOT compressed.";
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
        (MLabel::checkType(first) && CcString::checkType(second)) ||
        (MLabel::checkType(first) && FText::checkType(second)) ||
        (MLabels::checkType(first) && Label::checkType(second)) ||
        (MLabels::checkType(first) && Labels::checkType(second)) ||
        (MLabels::checkType(first) && CcString::checkType(second)) ||
        (MLabels::checkType(first) && FText::checkType(second)) ||
        (MPlace::checkType(first) && Place::checkType(second)) ||
        (MPlaces::checkType(first) && Place::checkType(second)) ||
        (MPlaces::checkType(first) && Places::checkType(second))) {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }
  return listutils::typeError("Correct signatures:  mlabel x label -> bool,  "
    "mlabel x string -> bool,  mlabel x text -> bool,  mlabels x label -> bool,"
    "  mlabels x labels -> bool,  mlabels x string -> bool,  mlabels x text -> "
    "bool,  mplace x place -> bool,  mplaces x place -> bool,  "
    "mplaces x places -> bool");
}

/*
\subsubsection{Selection Function}

*/
int atPassesSymbolicSelect(ListExpr args) {
  if (MLabel::checkType(nl->First(args))) {
    if (Label::checkType(nl->Second(args)))    return 0;
    if (CcString::checkType(nl->Second(args))) return 1;
    if (FText::checkType(nl->Second(args)))    return 2;
  }
  if (MLabels::checkType(nl->First(args))) {
    if (Label::checkType(nl->Second(args)))    return 3;
    if (Labels::checkType(nl->Second(args)))   return 4;
    if (CcString::checkType(nl->Second(args))) return 5;
    if (FText::checkType(nl->Second(args)))    return 6;
  }
  if (MPlace::checkType(nl->First(args)))  return 7;
  if (Place::checkType(nl->Second(args)))  return 8;
  if (Places::checkType(nl->Second(args))) return 9;
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
    if ((MLabel::checkType(first) && Label::checkType(second)) ||
        (MLabel::checkType(first) && CcString::checkType(second)) ||
        (MLabel::checkType(first) && FText::checkType(second))) {
      return nl->SymbolAtom(MLabel::BasicType());
    }
    if ((MLabels::checkType(first) && Label::checkType(second)) ||
        (MLabels::checkType(first) && Labels::checkType(second)) ||
        (MLabels::checkType(first) && CcString::checkType(second)) ||
        (MLabels::checkType(first) && FText::checkType(second))) {
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
  return listutils::typeError("Correct signatures: mlabel x label -> mlabel,  "
    "mlabel x string -> mlabel,  mlabel x text -> mlabel,  mlabels x label -> "
    "mlabels,  mlabels x labels -> mlabels,  mlabels x string -> mlabel,  "
    "mlabels x text -> mlabel,  mplace x place -> mplace,  mplaces x place -> "
    "mplaces,  mplaces x places -> mplaces");
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
int symbolicSimpleSelect(ListExpr args) {
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
  src->AtInstant(*inst, *res);
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
\subsection{Operator ~atperiods~}

atperiods: mlabel x periods -> mlabel
atperiods: mlabels x periods -> mlabels
atperiods: mplace x periods -> mplace
atperiods: mplaces x periods -> mplaces

\subsubsection{Type Mapping}

*/
ListExpr atperiodsSymbolicTM(ListExpr args) {
  if (nl->HasLength(args, 2)) {
    if (Periods::checkType(nl->Second(args))) {
      ListExpr first = nl->First(args);
      if (MLabel::checkType(first) || MLabels::checkType(first) ||
          MPlace::checkType(first) || MPlaces::checkType(first)) {
        return nl->First(args);
      }
    }
  }
  return listutils::typeError("Correct signature: mT x periods -> mT,   with "
    "T in {label(s), place(s)}");
}

/*
\subsubsection{Value Mapping}

*/
template<class M>
int atperiodsSymbolicVM(Word* args, Word& result, int message, Word& local,
                        Supplier s) {
  result = qp->ResultStorage(s);
  M *src = static_cast<M*>(args[0].addr);
  Periods *per = static_cast<Periods*>(args[1].addr);
  M *res = static_cast<M*>(result.addr);
  src->AtPeriods(*per, *res);
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct atperiodsSymbolicInfo : OperatorInfo {
  atperiodsSymbolicInfo() {
    name      = "atperiods";
    signature = "mlabel x periods -> mlabel";
    appendSignature("mlabels x periods -> mlabels");
    appendSignature("mplace x periods -> mplace");
    appendSignature("mplaces x periods -> mplaces");
    syntax    = "_ atperiods _";
    meaning   = "Restrict the moving object to the given periods.";
  }
};

/*
\subsection{Operator ~no\_components~}

no\_components: mlabel -> int
no\_components: mlabels -> int
no\_components: mplace -> int
no\_components: mplaces -> int
no\_components: labels -> int
no\_components: places -> int

\subsubsection{Type Mapping}

*/
ListExpr nocomponentsSymbolicTM(ListExpr args) {
    if (nl->HasLength(args, 1)) {
    ListExpr first = nl->First(args);
    if (MLabel::checkType(first) || MLabels::checkType(first) ||
        MPlace::checkType(first) || MPlaces::checkType(first) ||
        Labels::checkType(first) || Places::checkType(first)) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }
  return listutils::typeError("Expects a symbolic trajectory or a labels/places"
    "object.");
}

/*
\subsubsection{Selection Function}

*/
int nocomponentsSymbolicSelect(ListExpr args) {
  if (MLabel::checkType(nl->First(args)))  return 0;
  if (MLabels::checkType(nl->First(args))) return 1;
  if (MPlace::checkType(nl->First(args)))  return 2;
  if (MPlaces::checkType(nl->First(args))) return 3;
  if (Labels::checkType(nl->First(args)))  return 4;
  if (Places::checkType(nl->First(args)))  return 5;
  return -1;
}

/*
\subsubsection{Value Mapping}

*/
template<class Coll>
int nocomponentsSymbolicVM(Word* args, Word& result, int message, Word& local,
                           Supplier s) {
  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*>(result.addr);
  Coll *src = static_cast<Coll*>(args[0].addr);
  if (src->IsDefined()) {
    res->Set(true, src->GetNoComponents());
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct nocomponentsSymbolicInfo : OperatorInfo {
  nocomponentsSymbolicInfo() {
    name      = "no_components";
    signature = "mlabel -> int";
    appendSignature("mlabels -> int");
    appendSignature("mplace -> int");
    appendSignature("mplaces -> int");
    appendSignature("labels -> int");
    appendSignature("places -> int");
    syntax    = "no_components ( _ )";
    meaning   = "Returns the number of units of the symbolic trajectory or the "
      "number of components of a labels/places object.";
  }
};

/*
\subsection{Operator ~getInterval~}

This operator returns the interval of a symbolic unit or the bounding interval 
of a symbolic trajectory.

\subsubsection{Type Mapping}

*/
ListExpr getIntervalSymbolicTM(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError("One argument expected.");
  }
  if (!MLabel::checkType(nl->First(args)) &&
      !MLabels::checkType(nl->First(args)) &&
      !MPlace::checkType(nl->First(args)) &&
      !MPlaces::checkType(nl->First(args)) &&
      !ULabel::checkType(nl->First(args)) &&
      !ULabels::checkType(nl->First(args)) &&
      !UPlace::checkType(nl->First(args)) &&
      !UPlaces::checkType(nl->First(args))) {
    return listutils::typeError("Symbolic trajectory or symbolic unit "
                                "expected.");
  }
  return nl->SymbolAtom(SecInterval::BasicType());
}

/*
\subsubsection{Selection Function}

*/
int getIntervalSymbolicSelect(ListExpr args) {
  if (MLabel::checkType(nl->First(args)))  return 0;
  if (MLabels::checkType(nl->First(args))) return 1;
  if (MPlace::checkType(nl->First(args)))  return 2;
  if (MPlaces::checkType(nl->First(args))) return 3;
  if (ULabel::checkType(nl->First(args)))  return 4;
  if (ULabels::checkType(nl->First(args))) return 5;
  if (UPlace::checkType(nl->First(args)))  return 6;
  if (UPlaces::checkType(nl->First(args))) return 7;
  return -1;
}

/*
\subsubsection{Value Mapping}

*/
template<class M>
int getIntervalSymbolicVM(Word* args, Word& result, int message, Word& local,
                          Supplier s) {
  result = qp->ResultStorage(s);
  M *src = static_cast<M*>(args[0].addr);
  SecInterval *res = static_cast<SecInterval*>(result.addr);
  if (src->IsDefined()) {
    src->GetInterval(*res);
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct getIntervalSymbolicInfo : OperatorInfo {
  getIntervalSymbolicInfo() {
    name      = "getInterval";
    signature = "mlabel -> interval";
    appendSignature("mlabels -> interval");
    appendSignature("mplace -> interval");
    appendSignature("mplaces -> interval");
    appendSignature("ulabel -> interval");
    appendSignature("ulabels -> interval");
    appendSignature("uplace -> interval");
    appendSignature("uplaces -> interval");
    syntax    = "getInterval ( _ )";
    meaning   = "This operator returns the interval of a symbolic unit or the "
                "bounding interval of a symbolic trajectory.";
  }
};

/*
\subsection{Operator ~getunit~}

getunit: mlabel x int -> ulabel
getunit: mlabels x int -> ulabels
getunit: mplace x int -> uplace
getunit: mplaces x int -> uplaces

\subsubsection{Type Mapping}

*/
ListExpr getunitSymbolicTM(ListExpr args) {
  if (nl->HasLength(args, 2)) {
    if (CcInt::checkType(nl->Second(args))) {
      ListExpr first = nl->First(args);
      if (MLabel::checkType(first)) {
        return nl->SymbolAtom(ULabel::BasicType());
      } 
      if (MLabels::checkType(first)) {
        return nl->SymbolAtom(ULabels::BasicType());
      }
      if (MPlace::checkType(first)) {
        return nl->SymbolAtom(UPlace::BasicType());
      }
      if (MPlaces::checkType(first)) {
        return nl->SymbolAtom(UPlaces::BasicType());
      }
    }
  }
  return listutils::typeError("Expects a symbolic trajectory and an integer.");
}

/*
\subsubsection{Value Mapping}

*/
template<class M, class U>
int getunitSymbolicVM(Word* args, Word& result, int message, Word& local,
                      Supplier s) {
  result = qp->ResultStorage(s);
  M *src = static_cast<M*>(args[0].addr);
  CcInt* pos = static_cast<CcInt*>(args[1].addr);
  U *res = static_cast<U*>(result.addr);
  if (src->IsDefined() && pos->IsDefined()) {
    if (pos->GetIntval() < src->GetNoComponents() && pos->GetIntval() >= 0) {
      src->Get(pos->GetIntval(), *res);
    }
    else {
      res->SetDefined(false);
    }
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct getunitSymbolicInfo : OperatorInfo {
  getunitSymbolicInfo() {
    name      = "getunit";
    signature = "mlabel x int -> ulabel";
    appendSignature("mlabels x int -> ulabels");
    appendSignature("mplace x int -> uplace");
    appendSignature("mplaces x int -> uplaces");
    syntax    = "getunit ( _, _ )";
    meaning   = "Returns the unit located at a certain position of the symbolic"
                "trajectory.";
  }
};

/*
\subsection{Operator ~getPosition~}

getPosition: mlabel  x instant -> int
getPosition: mlabels x instant -> int
getPosition: mplace  x instant -> int
getPosition: mplaces x instant -> int

\subsubsection{Type Mapping}

*/
ListExpr getPositionSymbolicTM(ListExpr args) {
  if (nl->HasLength(args, 2)) {
    if (Instant::checkType(nl->Second(args))) {
      ListExpr first = nl->First(args);
      if (MLabel::checkType(first) || MLabels::checkType(first) ||
          MPlace::checkType(first) || MPlaces::checkType(first)) {
        return nl->SymbolAtom(CcInt::BasicType());
      }
    }
  }
  return listutils::typeError("Expects a moving(alpha) and an instant.");
}

/*
\subsubsection{Selection Function}

*/
int getPositionSymbolicSelect(ListExpr args) {
  if (MLabel::checkType(nl->First(args)))  return 0;
  if (MLabels::checkType(nl->First(args))) return 1;
  if (MPlace::checkType(nl->First(args)))  return 2;
  if (MPlaces::checkType(nl->First(args))) return 3;
  return -1;
}

/*
\subsubsection{Value Mapping}

*/
template<class M>
int getPositionSymbolicVM(Word* args, Word& result, int message, Word& local,
                          Supplier s) {
  result = qp->ResultStorage(s);
  CcInt *res = static_cast<CcInt*>(result.addr);
  M *m = static_cast<M*>(args[0].addr);
  Instant *inst = static_cast<Instant*>(args[1].addr);
  if (!m->IsDefined() || !inst->IsDefined()) {
    res->SetDefined(false);
    return 0;
  }
  int pos = m->Position(*inst);
  if (pos == -1) {
    res->SetDefined(false);
    return 0;
  }
  res->Set(true, m->Position(*inst));
  return 0;
}

/*
\subsubsection{Operator Info}

*/
struct getPositionSymbolicInfo : OperatorInfo {
  getPositionSymbolicInfo() {
    name      = "getPosition";
    signature = "mlabel x instant -> int";
    appendSignature("mlabels x instant -> int");
    appendSignature("mplace x instant -> int");
    appendSignature("mplaces x instant -> int");
    syntax    = "getPosition ( _, _ )";
    meaning   = "Returns the unit position inside the moving object that "
                "corresponds to the instant";
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
\section{Operator ~concat~}

\subsection{Type Mapping}

*/
ListExpr concatTM(ListExpr args) {
  const string errMsg = "Expecting two symbolic trajectories of the same type.";
  if (!nl->HasLength(args, 2)) {
    return listutils::typeError(errMsg);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if ((MLabel::checkType(arg1) && MLabel::checkType(arg2)) ||
      (MLabels::checkType(arg1) && MLabels::checkType(arg2)) ||
      (MPlace::checkType(arg1) && MPlace::checkType(arg2)) ||
      (MPlaces::checkType(arg1) && MPlaces::checkType(arg2))) {
    return arg1;
  }
  return listutils::typeError(errMsg);
}

/*
\subsection{Value Mapping}

*/
template<class T>
int concatVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  T* src1 = static_cast<T*>(args[0].addr);
  T* src2 = static_cast<T*>(args[1].addr);
  result = qp->ResultStorage(s);
  T* res = (T*)result.addr;
  res->Concat(*src1, *src2);
  return  0;
}

struct concatInfo : OperatorInfo {
  concatInfo() {
    name      = "concat";
    signature = "mT x mT -> mT, where T in {label(s), place(s)}";
    syntax    = "concat _  _";
    meaning   = "Concatenates two symbolic trajectories into one.";
  }
};

/*
\section{Operator ~compress~}

\subsection{Type Mapping}

*/
ListExpr compressTM(ListExpr args) {
  const string errMsg = "Expecting mT, T in {label, labels, place, places}";
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError(errMsg);
  }
  ListExpr arg = nl->First(args);
  if (MLabel::checkType(arg) || MLabels::checkType(arg) || 
      MPlace::checkType(arg) || MPlaces::checkType(arg)) {
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
  if (MLabels::checkType(arg)) return 1;
  if (MPlace::checkType(arg)) return 2;
  if (MPlaces::checkType(arg)) return 3;
  return -1;
}

/*
\subsection{Value Mapping (for a single MLabel)}

*/
template<class T>
int compressVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  T* source = static_cast<T*>(args[0].addr);
  result = qp->ResultStorage(s);
  T* res = (T*)result.addr;
  source->Compress(*res);
  return  0;
}

/*
\subsection{Operator Info}

*/
struct compressInfo : OperatorInfo {
  compressInfo() {
    name      = "compress";
    signature = "mT -> mT,   where T in {label(s), place(s)}";
    syntax    = "compress(_)";
    meaning   = "Unites temporally subsequent units with the same values.";
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
  if ((MLabel::checkType(arg) || MLabels::checkType(arg) ||
       MPlace::checkType(arg) || MPlaces::checkType(arg) || 
       Stream<MLabel>::checkType(arg) || Stream<MLabels>::checkType(arg) ||
       Stream<MPlace>::checkType(arg) || Stream<MPlaces>::checkType(arg))
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
  if (MLabels::checkType(arg)) return 1;
  if (MPlace::checkType(arg)) return 2;
  if (MPlaces::checkType(arg)) return 3;
  if (Stream<MLabel>::checkType(arg)) return 4;
  if (Stream<MLabels>::checkType(arg)) return 5;
  if (Stream<MPlace>::checkType(arg)) return 6;
  if (Stream<MPlaces>::checkType(arg)) return 7;
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
    signature = "mT -> mT,   where T in {label(s), place(s)}";
    appendSignature("stream(mT) -> stream(mT),   where T in {label(s), "
                    "place(s)}");
    syntax    = "fillgaps(_)";
    meaning   = "Fills temporal gaps between two (not temporally) subsequent "
                "units inside the symbolic trajectory if the values coincide";
  }
};


class SymbolicTrajectoryBasicAlgebra : public Algebra {
 public:
  SymbolicTrajectoryBasicAlgebra() : Algebra() {

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
  
  ValueMapping tolabelVMs[] = {tolabelVM<FText>, tolabelVM<CcString>, 0};
  AddOperator(tolabelInfo(), tolabelVMs, tolabelSelect, tolabelTM);

  AddOperator(tostringInfo(), tostringVM, tostringTM);

  AddOperator(totextInfo(), totextVM, totextTM);
  
  AddOperator(mstringtomlabelInfo(), mstringtomlabelVM, mstringtomlabelTM);
  
  ValueMapping tolabelsVMs[] = {tolabelsVM<FText>, tolabelsVM<CcString>, 0};
  AddOperator(tolabelsInfo(), tolabelsVMs, tolabelsSelect, tolabelsTM);
  
  ValueMapping toplacesVMs[] = {toplacesVM_P, toplacesVM_T, 0};
  AddOperator(toplacesInfo(), toplacesVMs, toplacesSelect, toplacesTM);
  
  ValueMapping collect_labelsVMs[] = {collect_labelsVM<Label>, 
    collect_labelsVM<CcString>, collect_labelsVM<FText>, 0};
  AddOperator(collect_labelsInfo(), collect_labelsVMs, collect_labelsSelect,
              collect_labelsTM);

  ValueMapping containsVMs[] = {containsSingleVM<Labels, Label>,
    containsSingleVM<Places, Place>, containsMultiVM<Labels, Labels>, 
    containsMultiVM<Places, Places>, containsBasicVM<CcString>,
    containsBasicVM<FText>, 0};
  AddOperator(containsInfo(), containsVMs, containsSelect, containsTM);
  
  ValueMapping intersectsVMs[] = {intersectsVM<Labels>, intersectsVM<Places>,0};
  AddOperator(intersectsInfo(), intersectsVMs, intersectsSelect, intersectsTM);
  
  ValueMapping toplaceVMs[] = {toplaceVM<CcString>, toplaceVM<FText>, 0};
  AddOperator(toplaceInfo(), toplaceVMs, toplaceSelect, toplaceTM);
  
  AddOperator(nameInfo(), nameVM, nameTM);
  
  AddOperator(refInfo(), refVM, refTM);
  
  ValueMapping equalsVMs[] = {equalsVM<Label>, equalsVM<Labels>, 
    equalsVM<Place>, equalsVM<Places>, equalsVM<Label, CcString>,
    equalsVM<Label, FText>, equalsVM<CcString, Label>, equalsVM<FText, Label>, 
    0};
  AddOperator(equalsInfo(), equalsVMs, equalsUnequalsSelect, equalsUnequalsTM);

  ValueMapping unequalsVMs[] = {unequalsVM<Label>, unequalsVM<Labels>, 
    unequalsVM<Place>, unequalsVM<Places>, unequalsVM<Label, CcString>,
    unequalsVM<Label, FText>, unequalsVM<CcString, Label>, 
    unequalsVM<FText, Label>, 0};
  AddOperator(unequalsInfo(), unequalsVMs, equalsUnequalsSelect,
              equalsUnequalsTM);
  
  ValueMapping unionVMs[] = {unionVM<Label, Label, Labels>,
    unionVM<Labels, Label, Labels>, unionVM<Label, Labels, Labels>,
    unionVM<Labels, Labels, Labels>, unionVM<Place, Place, Places>, 
    unionVM<Places, Place, Places>, unionVM<Place, Places, Places>,
    unionVM<Places, Places, Places>, 0};
  AddOperator(unionInfo(), unionVMs, unionSelect, unionTM);
  
  ValueMapping intersectionVMs[] = {intersectionVM<Labels>,
    intersectionVM<Places>, 0};
  AddOperator(intersectionInfo(), intersectionVMs, intersectionSelect, 
              intersectionTM);
  
  ValueMapping minusVMs[] = {minusVM<Labels, Label>, minusVM<Labels, Labels>,
    minusVM<Places, Place>, minusVM<Places, Places>, 0};
  AddOperator(minusInfo(), minusVMs, minusSelect, minusTM);
  
  #ifdef RECODE
  ValueMapping recodeVMs[] = {recodeVM<MLabel>, recodeVM<MLabels>, 0};
  AddOperator(recodeInfo(), recodeVMs, recodeSelect, recodeTM);
  #endif
  
  ValueMapping the_unitSymbolicVMs[] = {the_unitSymbolicVM<Label, ULabel>,
    the_unitSymbolicVM<Labels, ULabels>, the_unitSymbolicVM<Place, UPlace>,
    the_unitSymbolicVM<Places, UPlaces>, the_unitIvSymbolicVM<Label, ULabel>,
    the_unitIvSymbolicVM<Labels, ULabels>, the_unitIvSymbolicVM<Place, UPlace>,
    the_unitIvSymbolicVM<Places, UPlaces>, 0};
  AddOperator(the_unitSymbolicInfo(), the_unitSymbolicVMs,
              the_unitSymbolicSelect, the_unitSymbolicTM);
  
  ValueMapping makemvalueSymbolicVMs[] = {makemvalueSymbolicVM<ULabel, MLabel>,
    makemvalueSymbolicVM<ULabels, MLabels>, makemvalueSymbolicVM<UPlace,MPlace>,
    makemvalueSymbolicVM<UPlaces, MPlaces>, 0};
  AddOperator(makemvalueSymbolicInfo(), makemvalueSymbolicVMs,
              makemvalueSymbolicSelect, makemvalueSymbolic_TM);

  ValueMapping makemvalue2SymbolicVMs[] = {makemvalue2SymbolicVM<ULabel,MLabel>,
    makemvalue2SymbolicVM<ULabels, MLabels>, 
    makemvalue2SymbolicVM<UPlace, MPlace>,
    makemvalue2SymbolicVM<UPlaces, MPlaces>, 0};
  AddOperator(makemvalue2SymbolicInfo(), makemvalue2SymbolicVMs,
              makemvalueSymbolicSelect, makemvalueSymbolic_TM);
  
  ValueMapping passesSymbolicVMs[] = {passesSymbolicVM<MLabel, Label>,
    passesSymbolicVM<MLabel, CcString>, passesSymbolicVM<MLabel, FText>,
    passesSymbolicVM<MLabels, Label>, passesSymbolicVM<MLabels, Labels>,
    passesSymbolicVM<MLabels, CcString>, passesSymbolicVM<MLabels, FText>,
    passesSymbolicVM<MPlace, Place>, passesSymbolicVM<MPlaces, Place>,
    passesSymbolicVM<MPlaces, Places>, 0};
  AddOperator(passesSymbolicInfo(), passesSymbolicVMs, atPassesSymbolicSelect,
              passesSymbolicTM);
  
  ValueMapping atSymbolicVMs[] = {atSymbolicVM<MLabel, Label>,
    atSymbolicVM<MLabel, CcString>, atSymbolicVM<MLabel, FText>,
    atSymbolicVM<MLabels, Label>, atSymbolicVM<MLabels, Labels>,
    atSymbolicVM<MLabels, CcString>, atSymbolicVM<MLabels, FText>,
    atSymbolicVM<MPlace, Place>, atSymbolicVM<MPlaces, Place>,
    atSymbolicVM<MPlaces, Places>, 0};
  AddOperator(atSymbolicInfo(), atSymbolicVMs, atPassesSymbolicSelect, 
              atSymbolicTM);
  
  ValueMapping deftimeSymbolicVMs[] = {deftimeSymbolicVM<MLabel>,
    deftimeSymbolicVM<MLabels>, deftimeSymbolicVM<MPlace>,
    deftimeSymbolicVM<MPlaces>, 0};
  AddOperator(deftimeSymbolicInfo(), deftimeSymbolicVMs, symbolicSimpleSelect, 
              deftimeSymbolicTM);
  
  ValueMapping atinstantSymbolicVMs[] = {atinstantSymbolicVM<MLabel, ILabel>,
    atinstantSymbolicVM<MLabels, ILabels>, atinstantSymbolicVM<MPlace, IPlace>,
    atinstantSymbolicVM<MPlaces, IPlaces>, 0};
  AddOperator(atinstantSymbolicInfo(), atinstantSymbolicVMs,
              symbolicSimpleSelect, atinstantSymbolicTM);
  
  ValueMapping atperiodsSymbolicVMs[] = {atperiodsSymbolicVM<MLabel>,
    atperiodsSymbolicVM<MLabels>, atperiodsSymbolicVM<MPlace>, 
    atperiodsSymbolicVM<MPlaces>, 0};
  AddOperator(atperiodsSymbolicInfo(), atperiodsSymbolicVMs, 
              symbolicSimpleSelect, atperiodsSymbolicTM);
  
  ValueMapping nocomponentsSymbolicVMs[] = {nocomponentsSymbolicVM<MLabel>,
    nocomponentsSymbolicVM<MLabels>, nocomponentsSymbolicVM<MPlace>,
    nocomponentsSymbolicVM<MPlaces>, nocomponentsSymbolicVM<Labels>,
    nocomponentsSymbolicVM<Places>, 0};
  AddOperator(nocomponentsSymbolicInfo(), nocomponentsSymbolicVMs,
              nocomponentsSymbolicSelect, nocomponentsSymbolicTM);
  
  ValueMapping getIntervalSymbolicVMs[] = {getIntervalSymbolicVM<MLabel>,
    getIntervalSymbolicVM<MLabels>, getIntervalSymbolicVM<MPlace>, 
    getIntervalSymbolicVM<MPlaces>, getIntervalSymbolicVM<ULabel>,
    getIntervalSymbolicVM<ULabels>, getIntervalSymbolicVM<UPlace>, 
    getIntervalSymbolicVM<UPlaces>, 0};
  AddOperator(getIntervalSymbolicInfo(), getIntervalSymbolicVMs, 
              getIntervalSymbolicSelect, getIntervalSymbolicTM);
  
  ValueMapping getunitSymbolicVMs[] = {getunitSymbolicVM<MLabel, ULabel>,
    getunitSymbolicVM<MLabels, ULabels>, getunitSymbolicVM<MPlace, UPlace>, 
    getunitSymbolicVM<MPlaces, UPlaces>, 0};
  AddOperator(getunitSymbolicInfo(), getunitSymbolicVMs, symbolicSimpleSelect,
              getunitSymbolicTM);
  
  ValueMapping getPositionSymbolicVMs[] = {getPositionSymbolicVM<MLabel>,
    getPositionSymbolicVM<MLabels>, getPositionSymbolicVM<MPlace>,
    getPositionSymbolicVM<MPlaces>, 0};
  AddOperator(getPositionSymbolicInfo(), getPositionSymbolicVMs,
              getPositionSymbolicSelect, getPositionSymbolicTM);
  
  ValueMapping unitsSymbolicVMs[] = {unitsSymbolicVM<MLabel, ULabel>,
    unitsSymbolicVM<MLabels, ULabels>, unitsSymbolicVM<MPlace, UPlace>, 
    unitsSymbolicVM<MPlaces, UPlaces>, 0};
  AddOperator(unitsSymbolicInfo(), unitsSymbolicVMs, symbolicSimpleSelect,
              unitsSymbolicTM);
  
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
  
  ValueMapping concatVMs[] = {concatVM<MLabel>, concatVM<MLabels>,
    concatVM<MPlace>, concatVM<MPlaces>, 0};
  AddOperator(concatInfo(), concatVMs, symbolicSimpleSelect, concatTM);

  ValueMapping compressVMs[] = {compressVM<MLabel>, compressVM<MLabels>,
    compressVM<MPlace>, compressVM<MPlaces>, 0};
  AddOperator(compressInfo(), compressVMs, compressSelect, compressTM);

  ValueMapping fillgapsVMs[] = {fillgapsVM_1<MLabel>, fillgapsVM_1<MLabels>,
    fillgapsVM_1<MPlace>, fillgapsVM_1<MPlaces>, fillgapsVM_Str<MLabel>, 
    fillgapsVM_Str<MLabels>, fillgapsVM_Str<MPlace>, fillgapsVM_Str<MPlaces>,0};
  AddOperator(fillgapsInfo(), fillgapsVMs, fillgapsSelect, fillgapsTM);
  }
  
  ~SymbolicTrajectoryBasicAlgebra() {}
};

}

extern "C"
Algebra* InitializeSymbolicTrajectoryBasicAlgebra(NestedList *nlRef,
                                                  QueryProcessor *qpRef) {
  return new stj::SymbolicTrajectoryBasicAlgebra;
}
