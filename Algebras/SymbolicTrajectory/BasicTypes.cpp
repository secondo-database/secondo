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

Started July 2014, Fabio Vald\'{e}s

*/
#include "BasicTypes.h"

namespace stj {
  
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
double Label::Distance(const Label& lb) const {
  if (!IsDefined() && !lb.IsDefined()) {
    return 0;
  }
  if (!IsDefined() || !lb.IsDefined()) {
    return 1;
  }
  string str1, str2;
  GetValue(str1);
  lb.GetValue(str2);
  // TODO: return Tools::distance(str1, str2) and delete remainder
  return double(stringutils::ld(str1,str2)) / max(str1.length(), str2.length());
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
\subsection{Function ~Contains~}

*/
double Labels::Distance(const Labels& lbs) const {
  if (!IsDefined() && !lbs.IsDefined()) {
    return 0;
  }
  if (!IsDefined() || !lbs.IsDefined()) {
    return 1;
  }
  set<string> values1, values2;
  GetValues(values1);
  lbs.GetValues(values2);
  // TODO: return Tools::distance(values1, values2) and delete remainder
  if (values1.empty() && values2.empty()) {
    return 0;
  }
  if (values1.empty() || values2.empty()) {
    return 1;
  }
  set<string>::iterator i1, i2;
  multiset<double> dist;
  for (i1 = values1.begin(); i1 != values1.end(); i1++) {
    for (i2 = values2.begin(); i2 != values2.end(); i2++) {
      dist.insert(double(stringutils::ld(*i1, *i2)) /
                  max(i1->length(), i2->length()));
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
\subsection{Function ~Distance~}

*/
double Place::Distance(const Place& p) const {
  if (!IsDefined() && !p.IsDefined()) {
    return 0;
  }
  if (!IsDefined() || !p.IsDefined()) {
    return 1;
  }
  // TODO: return Tools::distance(value1, value2) and delete remainder
  return Label::Distance(p) / 2 + (GetRef() == p.GetRef() ? 0 : 0.5);
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
double Places::Distance(const Places& p) const {
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
 
}
